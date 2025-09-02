/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2021 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software must only be used in or with a processor manufactured by Nordic
 *    Semiconductor ASA, or in or with a processor manufactured by a third party that
 *    is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* PURPOSE: OS and platform depenednt stuff for Unix platform
*/

#ifndef ZB_OSIF_UNIX_H
#define ZB_OSIF_UNIX_H 1

/*! \addtogroup ZB_OSIF_UNIX */
/*! @{ */

#include "zb_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <netinet/in.h>
#ifndef __MINGW32__
#include <poll.h>
#endif

/* Used for fread, fgets, gcc v.7.5 */
#define SUPPRESS_UNUSED_RETURN(x) ((void)!(x))

#define ZB_ABORT abort

#ifndef UNIX_DOMAIN_SOCKETS_DIR
#define UNIX_DOMAIN_SOCKETS_DIR "/tmp/"
#endif

#define ZB_SDCC_XDATA
#define ZB_CALLBACK
#define ZB_SDCC_BANKED
#define ZB_KEIL_REENTRANT

/* Unix-specific trace */


typedef FILE zb_osif_file_t;
typedef zb_uint32_t zb_minimal_vararg_t;

#ifdef ZB_THREADS
void zb_osif_global_lock();
void zb_osif_global_unlock();
#define ZB_OSIF_GLOBAL_LOCK()   TRACE_MSG(TRACE_MACLL3, "lock", (FMT__0)); zb_osif_global_lock()
#define ZB_OSIF_GLOBAL_UNLOCK() TRACE_MSG(TRACE_MACLL3, "unlock", (FMT__0)); zb_osif_global_unlock()
#else
#define ZB_OSIF_GLOBAL_LOCK() (void)0
#define ZB_OSIF_GLOBAL_UNLOCK() (void)0
#endif

#define ZB_XDATA
#define ZB_CODE
#define ZB_STOP_WATCHDOG()

#define DECLARE_SERIAL_INTER_HANDLER()

/* use macros to be able to redefine */
#define ZB_MEMCPY memcpy
#define ZB_MEMMOVE memmove
#define ZB_MEMSET memset
#define ZB_MEMCMP memcmp

#define ZVUNUSED(v) (void)v

#ifdef ZB_STACK_REGRESSION_TESTING_API
#define ZB_ENABLE_ALL_INTER()  ZB_REGRESSION_TESTS_API().osif_interrupts_balance++
#define ZB_DISABLE_ALL_INTER() ZB_REGRESSION_TESTS_API().osif_interrupts_balance--
#else
#define ZB_ENABLE_ALL_INTER()
#define ZB_DISABLE_ALL_INTER()
#endif /* ZB_STACK_REGRESSION_TESTING_API */


#define ZB_VOLATILE

#define ZB_BZERO(s,l) ZB_MEMSET((char*)(s), 0, (l))
#define ZB_BZERO2(s) ZB_BZERO(s, 2)

void zb_unix_platform_init(void);
void zb_osif_init_timer(void);

#define ZB_PLATFORM_INIT() zb_unix_platform_init()

#define ZB_TIMER_INIT() zb_osif_init_timer()
#define ZB_CHECK_TIMER_IS_ON() 1 /*!< always on in linux */
#define ZB_START_HW_TIMER() /*!< nothing to do here */

/* Network routines */

#define OSIF_DECLARE_IPC_WAIT_CONTROL_T(type_name, n) typedef struct pollfd type_name[n]

typedef int osif_ipc_handle_t;
typedef void* osif_wait_control_t;
typedef pid_t osif_pid_t;
typedef int osif_control_pipe_t[2];

#define OSIF_IPC_SIGNAL_RX  1
#define OSIF_IPC_SIGNAL_TX  2
#define OSIF_IPC_SIGNAL_ERR 4
#define OSIF_IPC_SIGNAL_HUP 8

#define OSIF_IPC_INVALID_H -1

typedef void (*osif_io_handler_t)(zb_uint8_t event_mask);

void osif_serial_common_init(int fd);
void osif_serial_common_deinit(int fd);

void osif_transport_init(void);
zb_ret_t osif_transport_add_io_handler(osif_ipc_handle_t handle,
                                       zb_uint8_t event_mask,
                                       osif_io_handler_t handler);
zb_ret_t osif_transport_remove_io_handler(osif_ipc_handle_t handle);

void zb_osif_platform_io_iteration(zb_bool_t block);

#if defined ZB_COMPILE_MAC_MONOLITHIC
#define ZB_TRANSPORT_BLOCK() zb_nsng_io_iteration(ZB_TRUE)
#define ZB_TRANSPORT_NONBLOCK_ITERATION() zb_nsng_io_iteration(ZB_FALSE)
#else
#define ZB_TRANSPORT_BLOCK() zb_osif_platform_io_iteration(ZB_TRUE)
#define ZB_TRANSPORT_NONBLOCK_ITERATION() zb_osif_platform_io_iteration(ZB_FALSE)
#endif /* ZB_COMPILE_MAC_MONOLITHIC */


void zb_unix_platform_init(void);
void zb_osif_serial_deinit(void);
void osif_net_init();
char *osif_getenv(char *key, char *def);
void osif_gettime_us(zb_uint64_t *time_us);
void osif_get_clock_monotonic_raw(struct timespec *t);
void osif_get_clock_realtime(struct timespec *t);
zb_uint32_t osif_timespec_to_seconds(struct timespec *t);
void osif_seconds_to_timespec(zb_uint32_t s, struct timespec *t);
zb_uint32_t osif_get_clock_monotonic_sec();
zb_uint32_t osif_get_clock_realtime_sec();
int osif_timespec_compare(struct timespec *t1, struct timespec *t2);
void osif_timespec_add_sec(struct timespec *t, zb_uint32_t sec);
void osif_timespec_add_ms(struct timespec *t, zb_uint32_t ms);
void osif_timespec_add_us(struct timespec *t, zb_uint32_t us);
void osif_timespec_add_ns(struct timespec *t, zb_uint32_t ns);
zb_uint32_t osif_current_time_to_be();
zb_uint64_t osif_timespec_to_ms(struct timespec *t);
void osif_mseconds_to_timespec(zb_uint64_t ms, struct timespec *t);
zb_uint64_t osif_get_clock_monotonic_ms();
zb_uint64_t osif_timespec_to_us(struct timespec *t);
void osif_useconds_to_timespec(zb_uint64_t us, struct timespec *t);
zb_uint64_t osif_get_clock_monotonic_us();
zb_uint64_t osif_timespec_to_ns(struct timespec *t);
void osif_nseconds_to_timespec(zb_uint64_t ns, struct timespec *t);
zb_uint64_t osif_get_clock_monotonic_ns();
unsigned osif_getuid();
zb_int32_t osif_gethostid();
zb_int_t osif_ipc_create_h(osif_ipc_handle_t *h_p);
zb_int_t osif_tcp_create_h(osif_ipc_handle_t *h_p);
zb_int_t osif_ipc_srv_init(char *key, osif_ipc_handle_t *h_p);
zb_int_t osif_ipc_accept(osif_ipc_handle_t srv_h, osif_ipc_handle_t *h_p);
zb_int_t osif_ipc_connect(char *key, osif_ipc_handle_t h);
zb_int_t osif_tcp_connect(char *host, unsigned port, osif_ipc_handle_t h);
zb_int_t osif_tcp_connect_w_timeout(char *host, unsigned port, osif_ipc_handle_t h, zb_uint8_t timeout_sec);
zb_int_t osif_ipc_write(osif_ipc_handle_t h, const void *buf, zb_int_t len);
zb_int_t osif_ipc_read(osif_ipc_handle_t h, void *buf, zb_int_t len);
void osif_ipc_close(osif_ipc_handle_t h);
zb_int_t osif_ipc_wait_for_io(osif_wait_control_t wait_control, zb_int_t n_handlers, zb_uint_t timeout_us);
osif_ipc_handle_t osif_ipc_get_handler(osif_wait_control_t wait_control, zb_int_t i);
zb_int_t osif_ipc_signaled(osif_wait_control_t wait_control, zb_int_t i, zb_uint_t *mask_p);
void osif_ipc_init_wait(osif_wait_control_t wait_control, zb_int_t nfds);
void osif_ipc_setup_wait(osif_wait_control_t wait_control, zb_int_t i, osif_ipc_handle_t h, zb_uint8_t actions);
void osif_ipc_remove_wait(osif_wait_control_t wait_control, zb_int_t i);

zb_ret_t osif_ctrl_pipe_init(osif_control_pipe_t *ctrl);
zb_ret_t osif_ctrl_pipe_deinit(osif_control_pipe_t ctrl);
zb_ret_t osif_ctrl_pipe_event_set(osif_control_pipe_t ctrl, zb_char_t e);
zb_ret_t osif_ctrl_pipe_exit_event_set(osif_control_pipe_t ctrl);
void osif_ctrl_pipe_event_read(osif_control_pipe_t ctrl, zb_char_t *e);
void osif_ctrl_pipe_event_flush(osif_control_pipe_t ctrl);
osif_ipc_handle_t osif_ctrl_pipe_get_handler(osif_control_pipe_t ctrl);

void osif_usleep(zb_uint_t us);
void zb_osif_wait_ms(zb_uint16_t ms);
osif_pid_t osif_getpid();
void osif_srand(unsigned seed);
int osif_rand();
void osif_terminate_process(osif_pid_t pid);

/* 03/10/2015 CR [MZ] Block start. */
zb_ret_t osif_net_get_local_ip_address(osif_ipc_handle_t *, zb_char_t *);
/* 03/10/2015 CR [MZ] Block end. */
#define MAC_ADDRESS_SIZE 6
/* MAC addr string: 6 hex bytes (12 chars) + str termination ('\0') */
#define MAC_ADDRESS_STRING_SIZE (MAC_ADDRESS_SIZE * 2 + 1)
/* MAC addr string with colons: 6 hex bytes (12 chars) + 5 colons + str termination ('\0') */
#define MAC_ADDRESS_WITH_COLONS_STRING_SIZE (MAC_ADDRESS_SIZE * 2 + 5 + 1)
zb_ret_t osif_net_get_mac_address(unsigned char * mac_address, char *ifname);
zb_ret_t osif_net_get_ip4_address(struct in_addr *ip4_address, char *ifname);
zb_ret_t osif_net_get_mac_address_sys(unsigned char * mac_address, char *ifname);

/* threads */
typedef pthread_cond_t     osif_cond_t;
typedef pthread_condattr_t osif_condattr_t;
typedef pthread_mutex_t    osif_mutex_t;
typedef pthread_t          osif_thread_t;
typedef void *osif_func_ret_t;
typedef void *osif_func_arg_t;
typedef osif_func_ret_t (*osif_func_t)(osif_func_arg_t);
typedef osif_func_ret_t (*osif_tread_detached_func_t)(osif_func_arg_t);

zb_ret_t   osif_start_thread(osif_thread_t *thread, osif_func_t func, void * arg);
zb_ret_t   osif_start_thread_detached(osif_func_t func, void * arg);
void* osif_thread_join(osif_thread_t *thread);
zb_uint_t  osif_get_thread_id(ZB_VOID_ARGLIST);
zb_ret_t   osif_detach_thread(osif_thread_t *handle);

zb_ret_t osif_init_mutex(osif_mutex_t *mutex);
zb_ret_t osif_deinit_mutex(osif_mutex_t *mutex);
zb_ret_t osif_lock_mutex(osif_mutex_t *mutex);
zb_ret_t osif_unlock_mutex(osif_mutex_t *mutex);


zb_ret_t osif_init_cond(osif_cond_t *cond);
zb_ret_t osif_init_cond_with_attr(osif_cond_t *cond, osif_condattr_t *condattr);
zb_ret_t osif_deinit_cond(osif_cond_t *cond);
zb_ret_t osif_init_condattr(osif_condattr_t *condattr);
zb_ret_t osif_deinit_condattr(osif_condattr_t *condattr);
zb_ret_t osif_condattr_setclock(osif_condattr_t *condattr, zb_uint8_t clockid);
zb_ret_t osif_cond_wait(osif_cond_t *cond, osif_mutex_t *mutex);
zb_ret_t osif_cond_wait_with_timeout(osif_cond_t *cond, osif_mutex_t *mutex, unsigned wait_interval_ms);
zb_ret_t osif_cond_wait_with_target(osif_cond_t *cond, osif_mutex_t *mutex, struct timespec *target);
zb_ret_t osif_cond_signal(osif_cond_t *cond);
zb_ret_t osif_cond_broadcast(osif_cond_t *cond);
void osif_sched_yield();

void zb_osif_update_timer(void);

#define ZB_CHECK_TIMER_IS_ON() 1 /*!< always on in linux */
#define ZB_START_HW_TIMER() /*!< nothing to do here */
#define ZB_STOP_HW_TIMER() /*!< nothing to do here */

zb_ret_t osif_net_get_ifname_for_socket(osif_ipc_handle_t h, zb_char_t *buf, zb_uint16_t bufsize);

typedef void (*pipe_notification_cb_t)(zb_char_t e);

#ifdef ZB_MAIN_THREAD_PIPE_NOTIFICATION
void zb_register_pipe_notification_handler(pipe_notification_cb_t cb);
#endif  /* ZB_MAIN_THREAD_PIPE_NOTIFICATION */

/*! @} */

#endif /* ZB_OSIF_UNIX_H */
