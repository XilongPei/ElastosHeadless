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

#ifndef MK_TLS_H
#define MK_TLS_H

#ifndef PTHREAD_TLS  /* Use Compiler Thread Local Storage (TLS) */

/* mk_cache.c */
extern __thread struct mk_iov *mk_tls_cache_iov_header;
extern __thread mk_ptr_t *mk_tls_cache_header_cl;
extern __thread mk_ptr_t *mk_tls_cache_header_lm;
extern __thread struct tm *mk_tls_cache_gmtime;
extern __thread struct mk_gmt_cache *mk_tls_cache_gmtext;

/* mk_vhost.c */
extern __thread struct mk_list *mk_tls_vhost_fdt;

/* mk_scheduler.c */
extern __thread struct rb_root *mk_tls_sched_cs;
extern __thread struct mk_list *mk_tls_sched_cs_incomplete;
extern __thread struct mk_sched_notif *mk_tls_sched_worker_notif;
extern __thread struct mk_sched_worker *mk_tls_sched_worker_node;

/* mk_server.c */
extern __thread struct mk_list *mk_tls_server_listen;
extern __thread struct mk_server_timeout *mk_tls_server_timeout;

/* TLS helper macros */
#define MK_TLS_SET(key, val)        key=val
#define MK_TLS_GET(key)             key
#define MK_TLS_INIT()               do {} while (0)

#else /* Use Posix Thread Keys */

#define __thread "^ ^"

/* mk_cache.c */
pthread_key_t mk_tls_cache_iov_header;
pthread_key_t mk_tls_cache_header_cl;
pthread_key_t mk_tls_cache_header_lm;
pthread_key_t mk_tls_cache_gmtime;
pthread_key_t mk_tls_cache_gmtext;

/* mk_vhost.c */
pthread_key_t mk_tls_vhost_fdt;

/* mk_scheduler.c */
pthread_key_t mk_tls_sched_cs;
pthread_key_t mk_tls_sched_cs_incomplete;
pthread_key_t mk_tls_sched_worker_notif;
pthread_key_t mk_tls_sched_worker_node;

/* mk_server.c */
pthread_key_t mk_tls_server_listen;
pthread_key_t mk_tls_server_timeout;

#define MK_TLS_SET(key, val)  pthread_setspecific(key, (void *) val)
#define MK_TLS_GET(key)       pthread_getspecific(key)
#define MK_TLS_INIT()                                           \
    /* mk_cache.c */                                            \
    pthread_key_create(&mk_tls_cache_iov_header, NULL);         \
    pthread_key_create(&mk_tls_cache_header_cl, NULL);          \
    pthread_key_create(&mk_tls_cache_header_lm, NULL);          \
    pthread_key_create(&mk_tls_cache_gmtime, NULL);             \
    pthread_key_create(&mk_tls_cache_gmtext, NULL);             \
                                                                \
    /* mk_vhost.c */                                            \
    pthread_key_create(&mk_tls_vhost_fdt, NULL);                \
                                                                \
    /* mk_scheduler.c */                                        \
    pthread_key_create(&mk_tls_sched_cs, NULL);                 \
    pthread_key_create(&mk_tls_sched_cs_incomplete, NULL);      \
    pthread_key_create(&mk_tls_sched_worker_notif, NULL);       \
    pthread_key_create(&mk_tls_sched_worker_node, NULL);        \
                                                                \
    /* mk_server.c */                                           \
    pthread_key_create(&mk_tls_server_listen, NULL);            \
    pthread_key_create(&mk_tls_server_timeout, NULL);
#endif

#endif