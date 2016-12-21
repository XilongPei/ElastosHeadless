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

#ifndef MK_CONFIG_H
#define MK_CONFIG_H

#include <unistd.h>

#include <monkey/mk_core.h>

#ifndef O_NOATIME
#define O_NOATIME       01000000
#endif

#define MK_DEFAULT_CONFIG_FILE              "monkey.conf"
#define MK_DEFAULT_MIMES_CONF_FILE          "monkey.mime"
#define MK_DEFAULT_PLUGIN_LOAD_CONF_FILE    "plugins.load"
#define MK_DEFAULT_SITES_CONF_DIR           "sites/"
#define MK_DEFAULT_PLUGINS_CONF_DIR         "plugins/"
#define MK_DEFAULT_LISTEN_ADDR              "0.0.0.0"
#define MK_DEFAULT_LISTEN_PORT              "2001"
#define MK_WORKERS_DEFAULT                  1

/* Core capabilities, used as identifiers to match plugins */
#define MK_CAP_HTTP        1
#define MK_CAP_HTTP2       2
#define MK_CAP_SOCK_PLAIN  4
#define MK_CAP_SOCK_SSL    8

struct mk_config_listener
{
    char *address;                /* address to bind */
    char *port;                   /* TCP port        */
    uint32_t flags;               /* properties: http | http2 | ssl */
    struct mk_list _head;
};

/* Base struct of server */
struct mk_server_config
{
    int server_fd;                /* server socket file descriptor */
    int kernel_features;          /* Hold different server setup status */
    int fd_limit;                 /* Limit of file descriptors */
    unsigned int server_capacity; /* total server capacity */
    short int workers;            /* number of worker threads */
    short int manual_tcp_cork;    /* If enabled it will handle TCP_CORK */

    int8_t fdt;                   /* is FDT enabled ? */
    int8_t is_daemon;
    int8_t is_seteuid;
    int8_t scheduler_mode;        /* Scheduler balancing mode */

    char *serverconf;             /* path to configuration files */
    mk_ptr_t server_software;

    struct mk_list listeners;

    char *one_shot;
    char *port_override;
    char *user;
    char *user_dir;
    char *pid_file_path;        /* pid of server */
    char *path_config;
    char *server_conf_file;
    char *mimes_conf_file;
    char *plugin_load_conf_file;
    char *sites_conf_dir;
    char *plugins_conf_dir;
    char **request_headers_allowed;

    int timeout;                /* max time to wait for a new connection */
    int standard_port;          /* common port used in web servers (80) */
    int pid_status;
    int8_t hideversion;           /* hide version of server to clients ? */
    int8_t resume;                /* Resume (on/off) */
    int8_t symlink;               /* symbolic links */

    /* keep alive */
    int8_t keep_alive;            /* it's a persisten connection ? */
    int max_keep_alive_request; /* max persistent connections to allow */
    int keep_alive_timeout;     /* persistent connection timeout */

    /* counter of threads working */
    int thread_counter;
    /* real user */
    uid_t egid;
    gid_t euid;

    int max_request_size;

    struct mk_list *index_files;

    /* configured host quantity */
    int nhosts;
    struct mk_list hosts;

    mode_t open_flags;
    struct mk_list plugins;

    /* Safe EPOLLOUT event */
    int safe_event_write;

    /*
     * Optional reference to force a specific transport, this one
     * is used when overriding the configuration from some caller
     */
    char *transport_layer;

    /* Define the default mime type when is not possible to find the proper one */
    char *default_mimetype;

    char server_signature[16];
    char server_signature_header[32];
    int  server_signature_header_len;

    /* source configuration */
    struct mk_rconf *config;

    /* FIXME: temporal map of Network Layer plugin */
    struct mk_plugin_network *network;

    /* Direct map to Stage plugins */
    struct mk_list stage10_handler;
    struct mk_list stage20_handler;
    struct mk_list stage30_handler;
    struct mk_list stage40_handler;
    struct mk_list stage50_handler;
};

/* Global context of the server */
struct mk_server_config *mk_config;


/* Functions */
struct mk_server_config *mk_config_init();
void mk_config_start_configure(void);
void mk_config_add_index(char *indexname);
void mk_config_set_init_values(void);

/* config helpers */
void mk_config_error(const char *path, int line, const char *msg);

struct mk_config_listener *mk_config_listener_add(char *address, char *port,
                                                  int flags);

int mk_config_listen_check_busy();
void mk_config_listeners_free();

int mk_config_get_bool(char *value);
void mk_config_read_hosts(char *path);
void mk_config_sanity_check(void);
void mk_config_free_all();

#endif
