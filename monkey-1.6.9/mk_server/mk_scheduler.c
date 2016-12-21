/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Server
 *  ==================
 *  Copyright 2001-2015 Monkey Software LLC <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <monkey/monkey.h>
#include <monkey/mk_core.h>
#include <monkey/mk_vhost.h>
#include <monkey/mk_scheduler.h>
#include <monkey/mk_scheduler_tls.h>
#include <monkey/mk_server.h>
#include <monkey/mk_cache.h>
#include <monkey/mk_config.h>
#include <monkey/mk_clock.h>
#include <monkey/mk_plugin.h>
#include <monkey/mk_utils.h>
#include <monkey/mk_linuxtrace.h>
#include <monkey/mk_server.h>
#include <monkey/mk_plugin_stage.h>

#include <signal.h>
#include <sys/syscall.h>

struct mk_sched_worker *sched_list;
struct mk_sched_handler mk_http_handler;

static pthread_mutex_t mutex_sched_init = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_worker_init = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_worker_exit = PTHREAD_MUTEX_INITIALIZER;

/*
 * Returns the worker id which should take a new incomming connection,
 * it returns the worker id with less active connections. Just used
 * if config->scheduler_mode is MK_SCHEDULER_FAIR_BALANCING.
 */
static inline int _next_target()
{
    int i;
    int target = 0;
    unsigned long long tmp = 0, cur = 0;

    cur = sched_list[0].accepted_connections - sched_list[0].closed_connections;
    if (cur == 0)
        return 0;

    /* Finds the lowest load worker */
    for (i = 1; i < mk_config->workers; i++) {
        tmp = sched_list[i].accepted_connections - sched_list[i].closed_connections;
        if (tmp < cur) {
            target = i;
            cur = tmp;

            if (cur == 0)
                break;
        }
    }

    /*
     * If sched_list[target] worker is full then the whole server too, because
     * it has the lowest load.
     */
    if (mk_unlikely(cur >= mk_config->server_capacity)) {
        MK_TRACE("Too many clients: %i", mk_config->server_capacity);

        /* Instruct to close the connection anyways, we lie, it will die */
        return -1;
    }

    return target;
}

struct mk_sched_worker *mk_sched_next_target()
{
    int t = _next_target();

    if (mk_likely(t != -1))
        return &sched_list[t];
    else
        return NULL;
}

/*
 * This function is invoked when the core triggers a MK_SCHED_SIGNAL_FREE_ALL
 * event through the signal channels, it means the server will stop working
 * so this is the last call to release all memory resources in use. Of course
 * this takes place in a thread context.
 */
void mk_sched_worker_free()
{
    int i;
    pthread_t tid;
    struct mk_sched_worker *sl = NULL;

    pthread_mutex_lock(&mutex_worker_exit);

    /*
     * Fix Me: needs to implement API to make plugins release
     * their resources first at WORKER LEVEL
     */

    /* External */
    mk_plugin_exit_worker();
    mk_vhost_fdt_worker_exit();
    mk_cache_worker_exit();

    /* Scheduler stuff */
    tid = pthread_self();
    for (i = 0; i < mk_config->workers; i++) {
        if (sched_list[i].tid == tid) {
            sl = &sched_list[i];
        }
    }

    mk_bug(!sl);

    /* Free master array (av queue & busy queue) */
    mk_mem_free(MK_TLS_GET(mk_tls_sched_cs));
    mk_mem_free(MK_TLS_GET(mk_tls_sched_cs_incomplete));
    mk_mem_free(MK_TLS_GET(mk_tls_sched_worker_notif));
    pthread_mutex_unlock(&mutex_worker_exit);
}

struct mk_sched_handler *mk_sched_handler_cap(char cap)
{
    if (cap == MK_CAP_HTTP) {
        return &mk_http_handler;
    }

    return NULL;
}

/*
 * Register a new client connection into the scheduler, this call takes place
 * inside the worker/thread context.
 */
struct mk_sched_conn *mk_sched_add_connection(int remote_fd,
                                              struct mk_server_listen *listener,
                                              struct mk_sched_worker *sched)
{
    int ret;
    int size;
    struct mk_sched_handler *handler;
    struct mk_sched_conn *conn;
    struct mk_event *event;

    /* Before to continue, we need to run plugin stage 10 */
    ret = mk_plugin_stage_run_10(remote_fd);

    /* Close connection, otherwise continue */
    if (ret == MK_PLUGIN_RET_CLOSE_CONX) {
        listener->network->network->close(remote_fd);
        MK_LT_SCHED(remote_fd, "PLUGIN_CLOSE");
        return NULL;
    }

    handler = listener->protocol;
    if (handler->sched_extra_size > 0) {
        void *data;
        size = (sizeof(struct mk_sched_conn) + handler->sched_extra_size);
        data = mk_mem_malloc_z(size);
        conn = (struct mk_sched_conn *) data;
    }
    else {
        conn = mk_mem_malloc_z(sizeof(struct mk_sched_conn));
    }

    if (!conn) {
        mk_err("[server] Could not register client");
        return NULL;
    }

    event = &conn->event;
    event->fd           = remote_fd;
    event->type         = MK_EVENT_CONNECTION;
    event->mask         = MK_EVENT_EMPTY;
    event->status       = MK_EVENT_NONE;
    conn->arrive_time   = log_current_utime;
    conn->protocol      = handler;
    conn->net           = listener->network->network;
    conn->is_timeout_on = MK_FALSE;
    conn->server_listen = listener;

    /* Stream channel */
    conn->channel.type  = MK_CHANNEL_SOCKET;    /* channel type     */
    conn->channel.fd    = remote_fd;            /* socket conn      */
    conn->channel.io    = conn->net;            /* network layer    */
    conn->channel.event = event;                /* parent event ref */
    mk_list_init(&conn->channel.streams);

    /* Register the entry in the red-black tree queue for fast lookup */
    struct rb_node **new = &(sched->rb_queue.rb_node);
    struct rb_node *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        struct mk_sched_conn *this = container_of(*new, struct mk_sched_conn, _rb_head);

        parent = *new;
        if (conn->event.fd < this->event.fd)
            new = &((*new)->rb_left);
        else if (conn->event.fd > this->event.fd)
            new = &((*new)->rb_right);
        else {
            mk_bug(1);
            break;
        }
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&conn->_rb_head, parent, new);
    rb_insert_color(&conn->_rb_head, &sched->rb_queue);

    /*
     * Register the connections into the timeout_queue:
     *
     * When a new connection arrives, we cannot assume it contains some data
     * to read, meaning the event loop may not get notifications and the protocol
     * handler will never be called. So in order to avoid DDoS we always register
     * this session in the timeout_queue for further lookup.
     *
     * The protocol handler is in charge to remove the session from the
     * timeout_queue.
     */
    mk_sched_conn_timeout_add(conn, sched);

    /* Linux trace message */
    MK_LT_SCHED(remote_fd, "REGISTERED");

    return conn;
}

static void mk_sched_thread_lists_init()
{
    struct rb_root *sched_cs;
    struct mk_list *sched_cs_incomplete;

    /* mk_tls_sched_cs */
    sched_cs = mk_mem_malloc_z(sizeof(struct rb_root));
    MK_TLS_SET(mk_tls_sched_cs, sched_cs);

    /* mk_tls_sched_cs_incomplete */
    sched_cs_incomplete = mk_mem_malloc(sizeof(struct mk_list));
    mk_list_init(sched_cs_incomplete);
    MK_TLS_SET(mk_tls_sched_cs_incomplete, sched_cs_incomplete);
}

/* Register thread information. The caller thread is the thread information's owner */
static int mk_sched_register_thread()
{
    struct mk_sched_worker *sl;
    static int wid = 0;

    /*
     * If this thread slept inside this section, some other thread may touch wid.
     * So protect it with a mutex, only one thread may handle wid.
     */
    pthread_mutex_lock(&mutex_sched_init);

    sl = &sched_list[wid];
    sl->idx = wid++;
    sl->tid = pthread_self();


#if defined(__linux__)
    /*
     * Under Linux does not exists the difference between process and
     * threads, everything is a thread in the kernel task struct, and each
     * one has it's own numerical identificator: PID .
     *
     * Here we want to know what's the PID associated to this running
     * task (which is different from parent Monkey PID), it can be
     * retrieved with gettid() but Glibc does not export to userspace
     * the syscall, we need to call it directly through syscall(2).
     */
    sl->pid = syscall(__NR_gettid);
#elif defined(__APPLE__)
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    sl->pid = tid;
#else
    sl->pid = 0xdeadbeef;
#endif

    pthread_mutex_unlock(&mutex_sched_init);

    /* Initialize lists */
    sl->rb_queue = RB_ROOT;
    mk_list_init(&sl->timeout_queue);
    sl->request_handler = NULL;

    return sl->idx;
}

static void mk_signal_thread_sigpipe_safe()
{
    sigset_t old;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &old);
}

/* created thread, all these calls are in the thread context */
void *mk_sched_launch_worker_loop(void *thread_conf)
{
    int ret;
    int wid;
    unsigned long len;
    char *thread_name = 0;
    struct mk_sched_worker *sched = NULL;
    struct mk_sched_notif *notif = NULL;

    /* Avoid SIGPIPE signals on this thread */
    mk_signal_thread_sigpipe_safe();

    /* Init specific thread cache */
    mk_sched_thread_lists_init();
    mk_cache_worker_init();

    /* Register working thread */
    wid = mk_sched_register_thread();

    sched = &sched_list[wid];
    sched->loop = mk_event_loop_create(MK_EVENT_QUEUE_SIZE);
    if (!sched->loop) {
        mk_err("Error creating Scheduler loop");
        exit(EXIT_FAILURE);
    }
    sched->mem_pagesize = sysconf(_SC_PAGESIZE);

    /*
     * Create the notification instance and link it to the worker
     * thread-scope list.
     */
    notif = mk_mem_malloc(sizeof(struct mk_sched_notif));
    MK_TLS_SET(mk_tls_sched_worker_notif, notif);

    /* Register the scheduler channel to signal active workers */
    ret = mk_event_channel_create(sched->loop,
                                  &sched->signal_channel_r,
                                  &sched->signal_channel_w,
                                  notif);
    if (ret < 0) {
        exit(EXIT_FAILURE);
    }

    mk_list_init(&sched->event_free_queue);

    /*
     * ULONG_MAX BUG test only
     * =======================
     * to test the workaround we can use the following value:
     *
     *  thinfo->closed_connections = 1000;
     */

    //thinfo->ctx = thconf->ctx;

    mk_mem_free(thread_conf);

    /* Rename worker */
    mk_string_build(&thread_name, &len, "monkey: wrk/%i", sched->idx);
    mk_utils_worker_rename(thread_name);
    mk_mem_free(thread_name);

    /* Export known scheduler node to context thread */
    MK_TLS_SET(mk_tls_sched_worker_node, sched);
    mk_plugin_core_thread();

    if (mk_config->scheduler_mode == MK_SCHEDULER_REUSEPORT) {
        sched->listeners = mk_server_listen_init(mk_config);
        if (!sched->listeners) {
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_lock(&mutex_worker_init);
    sched->initialized = 1;
    pthread_mutex_unlock(&mutex_worker_init);

    /* init server thread loop */
    mk_server_worker_loop();

    return 0;
}

/* Create thread which will be listening for incomings requests */
int mk_sched_launch_thread(int max_events, pthread_t *tout)
{
    pthread_t tid;
    pthread_attr_t attr;
    sched_thread_conf *thconf;
    (void) max_events;

    /* Thread data */
    thconf = mk_mem_malloc_z(sizeof(sched_thread_conf));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (pthread_create(&tid, &attr, mk_sched_launch_worker_loop,
                       (void *) thconf) != 0) {
        mk_libc_error("pthread_create");
        return -1;
    }

    *tout = tid;

    return 0;
}

/*
 * The scheduler nodes are an array of struct mk_sched_worker type,
 * each worker thread belongs to a scheduler node, on this function we
 * allocate a scheduler node per number of workers defined.
 */
void mk_sched_init()
{
    int size;

    size = sizeof(struct mk_sched_worker) * mk_config->workers;
    sched_list = mk_mem_malloc_z(size);
}

void mk_sched_set_request_list(struct rb_root *list)
{
    MK_TLS_SET(mk_tls_sched_cs, list);
}

int mk_sched_remove_client(struct mk_sched_conn *conn,
                           struct mk_sched_worker *sched)
{
    struct mk_event *event;

    /*
     * Close socket and change status: we must invoke mk_epoll_del()
     * because when the socket is closed is cleaned from the queue by
     * the Kernel at its leisure, and we may get false events if we rely
     * on that.
     */
    event = &conn->event;
    MK_TRACE("[FD %i] Scheduler remove", event->fd);

    mk_event_del(sched->loop, event);

    /* Invoke plugins in stage 50 */
    mk_plugin_stage_run_50(event->fd);

    sched->closed_connections++;

    /* Unlink from the red-black tree */
    rb_erase(&conn->_rb_head, &sched->rb_queue);
    mk_sched_conn_timeout_del(conn);

    /* Close at network layer level */
    conn->net->close(event->fd);

    /* Release and return */
    mk_channel_clean(&conn->channel);
    mk_sched_event_free(&conn->event);
    conn->status = MK_SCHED_CONN_CLOSED;

    MK_LT_SCHED(remote_fd, "DELETE_CLIENT");
    return 0;
}

struct mk_sched_conn *mk_sched_get_connection(struct mk_sched_worker *sched,
                                                 int remote_fd)
{
    struct rb_node *node;
    struct mk_sched_conn *this;

    /*
     * In some cases the sched node can be NULL when is a premature close,
     * an example of this situation is when the function mk_sched_add_client()
     * close an incoming connection when invoking the MK_PLUGIN_STAGE_10 stage plugin,
     * so no thread context exists.
     */
    if (!sched) {
        MK_TRACE("[FD %i] No scheduler information", remote_fd);
        close(remote_fd);
        return NULL;
    }

  	node = sched->rb_queue.rb_node;
  	while (node) {
        this = container_of(node, struct mk_sched_conn, _rb_head);
		if (remote_fd < this->event.fd)
  			node = node->rb_left;
		else if (remote_fd > this->event.fd)
  			node = node->rb_right;
		else {
            MK_LT_SCHED(remote_fd, "GET_CONNECTION");
  			return this;
        }
	}

    MK_TRACE("[FD %i] not found in scheduler list", remote_fd);
    MK_LT_SCHED(remote_fd, "GET_FAILED");
    return NULL;
}

/*
 * For a given connection number, remove all resources associated: it can be
 * used on any context such as: timeout, I/O errors, request finished,
 * exceptions, etc.
 */
int mk_sched_drop_connection(struct mk_sched_conn *conn,
                             struct mk_sched_worker *sched)
{
    return mk_sched_remove_client(conn, sched);
}

int mk_sched_check_timeouts(struct mk_sched_worker *sched)
{
    int client_timeout;
    struct mk_sched_conn *conn;
    struct mk_list *head;
    struct mk_list *temp;

    /* PENDING CONN TIMEOUT */
    mk_list_foreach_safe(head, temp, &sched->timeout_queue) {
        conn = mk_list_entry(head, struct mk_sched_conn, timeout_head);
        if (conn->event.type & MK_EVENT_IDLE) {
            continue;
        }

        client_timeout = conn->arrive_time + mk_config->timeout;

        /* Check timeout */
        if (client_timeout <= log_current_utime) {
            MK_TRACE("Scheduler, closing fd %i due TIMEOUT",
                     conn->event.fd);
            MK_LT_SCHED(conn->event.fd, "TIMEOUT_CONN_PENDING");
            conn->protocol->cb_close(conn, sched, MK_SCHED_CONN_TIMEOUT);
            mk_sched_drop_connection(conn, sched);
        }
    }

    return 0;
}

/*
 * Scheduler events handler: lookup for event handler and invoke
 * proper callbacks.
 */
int mk_sched_event_read(struct mk_sched_conn *conn,
                        struct mk_sched_worker *sched)
{
    int ret = 0;
    size_t count = 0;
    size_t total = 0;
    struct mk_event *event;
    uint32_t stop = (MK_CHANNEL_DONE | MK_CHANNEL_ERROR | MK_CHANNEL_EMPTY);

#ifdef TRACE
    MK_TRACE("[FD %i] Connection Handler / read", conn->event.fd);
#endif

    /*
     * When the event loop notify that there is some readable information
     * from the socket, we need to invoke the protocol handler associated
     * to this connection and also pass as a reference the 'read()' function
     * that handle 'read I/O' operations, e.g:
     *
     *  - plain sockets through liana will use just read(2)
     *  - ssl though mbedtls should use mk_mbedtls_read(..)
     */
    ret = conn->protocol->cb_read(conn, sched);
    if (ret == -1) {
        if (errno == EAGAIN) {
            MK_TRACE("EAGAIN: need to read more data");
            return 1;
        }
        return -1;
    }

    /*
     * There is a high probability that the protocol-handler have enqueued
     * some data to write. We will check the Channel, if some Stream is attached,
     * we will try to dispath the data over the socket, we will do this using the
     * following logic:
     *
     *  1. Try to flush a minimum of bytes contained in multiple Streams. The
     *     value is given by _SC_PAGESIZE stored on:
     *
     *      struct mk_sched_worker {
     *          ...
     *          int mem_pagesize;
     *      };
     *
     *  2. If in #1 we receive some -EAGAIN, ask for MK_EVENT_WRITE events on
     *     the socket. On that way we make sure we flush data when the Kernel
     *     tell us this is possible.
     *
     *  3. If after #1 there is still some enqueued data, handle the remaining
     *     ones through a MK_EVENT_WRITE and it proper callback handler.
     */
    do {
        ret = mk_channel_write(&conn->channel, &count);
        total += count;
    } while (total <= sched->mem_pagesize && ((ret & stop) == 0));

    if (ret == MK_CHANNEL_DONE) {
        if (conn->protocol->cb_done) {
            ret = conn->protocol->cb_done(conn, sched);
            if (ret == 1) {
                /* Protocol handler want to send more data */
                event = &conn->event;
                mk_event_add(sched->loop, event->fd,
                             MK_EVENT_CONNECTION,
                             MK_EVENT_WRITE,
                             conn);
                return 0;
            }
            return ret;
        }
    }
    else if (ret & (MK_CHANNEL_FLUSH | MK_CHANNEL_BUSY)) {
        event = &conn->event;
        if ((event->mask & MK_EVENT_WRITE) == 0) {
            mk_event_add(sched->loop, event->fd,
                         MK_EVENT_CONNECTION,
                         MK_EVENT_WRITE,
                         conn);
        }
    }

    return ret;
}

int mk_sched_event_write(struct mk_sched_conn *conn,
                         struct mk_sched_worker *sched)
{
    int ret = -1;
    size_t count;
    struct mk_event *event;

    MK_TRACE("[FD %i] Connection Handler / write", conn->event.fd);


    ret = mk_channel_write(&conn->channel, &count);
    if (ret == MK_CHANNEL_FLUSH || ret == MK_CHANNEL_BUSY) {
        return 0;
    }
    else if (ret == MK_CHANNEL_DONE || ret == MK_CHANNEL_EMPTY) {
        if (conn->protocol->cb_done) {
            ret = conn->protocol->cb_done(conn, sched);
        }
        if (ret == -1) {
            return -1;
        }
        else if (ret == 0) {
            event = &conn->event;
            mk_event_add(sched->loop, event->fd,
                         MK_EVENT_CONNECTION,
                         MK_EVENT_READ,
                         conn);
        }
        return 0;
    }
    else if (ret & MK_CHANNEL_ERROR) {
        return -1;
    }

    /* avoid to make gcc cry :_( */
    return -1;
}

int mk_sched_event_close(struct mk_sched_conn *conn,
                         struct mk_sched_worker *sched,
                         int type)
{
    MK_TRACE("[FD %i] Connection Handler, closed", conn->event.fd);
    mk_event_del(sched->loop, &conn->event);

    if (type != MK_EP_SOCKET_DONE) {
        conn->protocol->cb_close(conn, sched, type);
    }
    /*
     * Remove the socket from the scheduler and make sure
     * to disable all notifications.
     */
    mk_sched_drop_connection(conn, sched);
    return 0;
}

void mk_sched_event_free(struct mk_event *event)
{
    struct mk_sched_worker *sched = mk_sched_get_thread_conf();

    if ((event->type & MK_EVENT_IDLE) != 0) {
        return;
    }

    event->type |= MK_EVENT_IDLE;
    mk_list_add(&event->_head, &sched->event_free_queue);
}
