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
/* PURPOSE: File i/o trace, traffic dump and, maybe, other unix-specific
*/

#define ZB_TRACE_FILE_ID 30010
#include "zb_common.h"
#include <pthread.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <dirent.h>

#ifdef ZB_PLATFORM_LINUX
#include "linux/limits.h"
#include "fcntl.h"
#include <sys/stat.h>
#endif

#ifdef USE_STACK_UNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

//#include "zb_platform.h"

/*! \addtogroup ZB_OSIF */
/*! @{ */

#ifdef UNIX

zb_uint32_t zb_random_seed()
{
#ifdef ZB_PLATFORM_LINUX
  zb_uint32_t random_value = 0xffffffff;

  zb_uint32_t random_number_generator = open("/dev/urandom", O_RDONLY);
  SUPPRESS_UNUSED_RETURN(read(random_number_generator, &random_value, sizeof(zb_uint32_t)));
  close(random_number_generator);

  return random_value;
#else
  time_t t = time(NULL);
  srand((unsigned int)t);

  return rand();
#endif
}


zb_ret_t osif_set_transmit_power(zb_uint8_t channel, zb_int8_t power)
{
  ZVUNUSED(channel);
  ZVUNUSED(power);
  return RET_OK;
}


osif_pid_t osif_getpid()
{
  return getpid();
}


void osif_terminate_process(osif_pid_t pid)
{
  (void)kill(pid, SIGKILL);
}

void osif_srand(unsigned seed)
{
  srand(seed);
}


int osif_rand()
{
  return rand();
}


void osif_close_all_descriptor();

void zb_reset(zb_uint8_t param)
{
  char self_name[PATH_MAX+1];
  ssize_t self_name_len = 0;
#define MAX_N_ARGS 10

  ZVUNUSED(param);

  /* remove any trace to exclude dependence between osif and common */
  TRACE_MSG(TRACE_ERROR, ">zb_reset", (FMT__0));
  setlogmask(LOG_UPTO (LOG_NOTICE));
  openlog("zb_unix", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, "zb_reset");

#ifdef USE_STACK_UNWIND
  {
    unw_context_t context;
    unw_cursor_t  cursor;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0)
    {
      char       tracemsg[512];
      char       sym[256];
      unw_word_t offset, pc;

      unw_get_reg(&cursor, UNW_REG_IP, &pc);

      if (pc == 0)
      {
        break;
      }

      ZB_BZERO(tracemsg, sizeof(tracemsg));

      if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
      {
        snprintf(tracemsg, sizeof(tracemsg),
                 "0x%" PRIx64 ": (%s+0x%" PRIx64 ")", pc, sym, offset);
      } else
      {
        snprintf(tracemsg, sizeof(tracemsg),
                 "0x%" PRIx64 ": -- no symbol name", pc);
      }
      syslog(LOG_NOTICE, "%s\n", tracemsg);
#ifdef ZB_BINARY_TRACE
      TRACE_MSG(TRACE_ERROR, "0x%x", (FMT__D, (zb_uint32_t)pc));
#else
      TRACE_MSG(TRACE_ERROR, "%s", (FMT__P, tracemsg));
#endif
    }
  }
#endif  /* USE_STACK_UNWIND */

#ifdef ZB_RESET_AUTORESTART
  self_name_len = readlink("/proc/self/exe", self_name, PATH_MAX);
  if (self_name_len == -1)
  {
//    TRACE_MSG(TRACE_ERROR, "Cannot get self name", (FMT__0));
     syslog (LOG_NOTICE, "Cannot get self name");
  }
  else
  {
    int n, i;
    char *p;
    char args[PATH_MAX];
    char *av[MAX_N_ARGS + 1] = { NULL };
    FILE *f = fopen("/proc/self/cmdline", "r");

    /* TODO: use more human-readable names, code below is too complicated, rewrite it */

    n = 0;
    if (f)
    {
      n = fread(args, 1, sizeof(args), f);
    }
    fclose(f);
    p = args;
    for (i = 0 ; p && n && i < MAX_N_ARGS ; i++)
    {
      av[i] = p;
      n -= strlen(p) + 1;
      p = strchr(p, 0);
      if (p)
      {
        p++;
      }
    }
    osif_close_all_descriptor();

    /* readlink doesn't append termination symbol, do it ourselves */
    self_name[self_name_len] = '\0';

    syslog (LOG_NOTICE, "execv(%s, av)", self_name);

    execv(self_name, av);
    syslog(LOG_NOTICE, "restart using %s fail ", self_name);
    closelog();
  }
#else
  ZVUNUSED(self_name);
  ZVUNUSED(self_name_len);
  closelog();
  ZB_ASSERT(0);
#endif

  /* If we are here there was an error during execve() system call. */
//  TRACE_MSG(TRACE_ERROR, "An error occured during execve() system call", (FMT__0));
}


void osif_reset(int sig)
{
  sigset_t sigset;

  setlogmask(LOG_UPTO (LOG_NOTICE));
  openlog("zb_unix", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, "get sig =%i\n", sig);

  // Delete a signal from the signal mask
  sigfillset(&sigset);
  sigprocmask(SIG_SETMASK, &sigset, NULL);
  sigdelset(&sigset, sig);
  sigprocmask(SIG_SETMASK, &sigset, NULL);

  zb_reset(0);
}


void osif_set_reset_at_crash()
{
  struct sigaction act;

  memset(&act, 0, sizeof(act));
  //sigemptyset(&act.sa_mask);
  /* block every signal during the handler */
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = osif_reset;
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGILL, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGTRAP, &act, NULL);
  sigaction(SIGSTKFLT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGFPE, &act, NULL);
  sigaction(SIGSYS, &act, NULL);
  /* 08/23/2018 EE CR:MAJOR Also catch SIGQUIT, SIGFPE, SIGSYS; ignore SIGALARM, SIGHUP, SIGUSER1, SIGUSR2, SIGCHLD(rethink there...),   */
//#ifdef USE_SIG_TERM
  sigaction(SIGTERM, &act, NULL);
//#endif
}


static void osif_signal_default_handler(int sig)
{
  TRACE_MSG(TRACE_ERROR, ">osif_signal_default_handler: %d", (FMT__D, sig));
}


void osif_handle_crash()
{
  struct sigaction act;

  memset(&act, 0, sizeof(act));
  //sigemptyset(&act.sa_mask);
  /* block every signal during the handler */
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = osif_reset;
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGILL, &act, NULL);
  sigaction(SIGTRAP, &act, NULL);
  sigaction(SIGSTKFLT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGFPE, &act, NULL);
  sigaction(SIGSYS, &act, NULL);

  memset(&act, 0, sizeof(act));
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = osif_signal_default_handler;
  sigaction(SIGHUP, &act, NULL);
  sigaction(SIGIOT, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);
  sigaction(SIGPIPE, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGCHLD, &act, NULL);
  sigaction(SIGTSTP, &act, NULL);
  sigaction(SIGTTIN, &act, NULL);
  sigaction(SIGTTOU, &act, NULL);
  sigaction(SIGURG, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
  sigaction(SIGXFSZ, &act, NULL);
  sigaction(SIGVTALRM, &act, NULL);
  sigaction(SIGPROF, &act, NULL);
  sigaction(SIGIO, &act, NULL);
}


void osif_handle_sigterm(int signum)
{
  ZVUNUSED(signum);
  TRACE_MSG(TRACE_ERROR, "SIGTERM caught", (FMT__0));
  exit(0);
}


void osif_set_sigterm_handler(void)
{
  struct sigaction act;

  memset(&act, 0, sizeof(act));
  sigfillset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = osif_handle_sigterm;
  sigaction(SIGTERM, &act, NULL);
}


zb_bool_t zb_osif_is_inside_isr(void)
{
  return ZB_FALSE;
}


zb_uint32_t zb_osif_get_timer_reminder()
{
  return 0;
}


void zb_osif_set_user_io_buffer(zb_byte_array_t* buf_ptr, zb_ushort_t capacity)
{
  ZVUNUSED(buf_ptr);
  ZVUNUSED(capacity);
}


void zb_unix_platform_init(void)
{
  osif_set_sigterm_handler();
  osif_transport_init();

  ZB_TIMER_INIT();
}

#endif  /* UNIX */

/*! @} */
