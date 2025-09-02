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
/* PURPOSE: trace and traffic dump for platform with filesystem and printf() routine
*/


#define ZB_TRACE_FILE_ID 2145
#include "zb_common.h"

#if (defined ZB_TRACE_TO_FILE || defined ZB_TRACE_TO_SYSLOG)

#include <stdarg.h>
#include "zb_trace_common.h"

#if defined (ZB_TRACE_TO_SYSLOG)

#ifdef __ANDROID__
#include <android/log.h>
#elif defined UNIX
#include <syslog.h>
#endif

#endif

#ifdef ZB_PLATFORM_LINUX
#include <time.h>
#endif

/** @cond DOXYGEN_DEBUG_SECTION */
/** @addtogroup ZB_TRACE */
/** @{ */

#define MAX_SUBSYSTEM_NAME 32

static zb_bool_t is_error_msg(zb_uint_t mask, zb_uint_t level)
{
  return mask == TRACE_SUBSYSTEM_INFO && level == 0;
}

static const zb_char_t *subsystem_name_get(zb_uint_t mask, zb_uint_t level)
{
  zb_uindex_t i;

  const zb_char_t *components[] =
  {
   "COMMON",              /* 0x0001U */
   "MEM",                 /* 0x0002U */
   "MAC",                 /* 0x0004U */
   "NWK",                 /* 0x0008U */
   "APS",                 /* 0x0010U */
   "ZSE",                 /* 0x0020U */
   "ZDO",                 /* 0x0040U */
   "SECUR",               /* 0x0080U */
   "ZCL",                 /* 0x0100U */
   "ZLL",                 /* 0x0200U */
   "SSL",                 /* 0x0400U */
   "APP",                 /* 0x0800U */
   "LWIP",                /* 0x1000U */
   "ALIEN",               /* 0x2000U */
   "ZGP",                 /* 0x4000U */
   "MAC_API",             /* 0x8000U */
   "MACLL",               /* 0x10000U */
   "SPECIAL1",            /* 0x20000U */
   "BATTERY",             /* 0x40000U */
   "OTA",                 /* 0x80000U */
   "TRANSPORT",           /* 0x100000U */
   "USB",                 /* 0x200000U */
   "SPI",                 /* 0x400000U */
   "UART",                /* 0x800000UL */
   "JSON",                /* 0x1000000UL */
   "HTTP",                /* 0x2000000UL */
   "CLOUD",               /* 0x4000000UL */
   "ZBDIRECT"             /* 0x8000000UL */
  };

  for (i = 0; i < ZB_ARRAY_SIZE(components); i++)
  {
    if ((1u << i) == mask)
    {
      return components[i];
    }
  }

  /* No one of default names. Then it's ERROR of INFO */
  if (is_error_msg(mask, level))
  {
    return "ERROR";
  }
  else
  {
    return "INFO";
  }
}

static void fill_subsystem_name(zb_char_t *subsystem_name, zb_uint_t mask, zb_uint_t level)
{
  const zb_char_t *name = subsystem_name_get(mask, level);

  /* Print ERROR messages without level */
  if (is_error_msg(mask, level))
  {
    snprintf(subsystem_name, MAX_SUBSYSTEM_NAME, "%s", name);
  }
  else
  {
    snprintf(subsystem_name, MAX_SUBSYSTEM_NAME, "%s%d", name, level);
  }
}

/**
 * Output trace message.
 * Version with va_list
 *
 * @param format - printf-like format string
 * @param file_name - source file name
 * @param line_number - source file line
 * @param args_size - number of added parameters
 */
void zb_trace_msg_txt_file_vl(
  zb_uint_t mask,
  zb_uint_t level,
  const zb_char_t *format,
  const zb_char_t *file_name,
  zb_int_t line_number,
  zb_int_t args_size,
  va_list arglist)
{
  /* If ZB_TRACE_LEVEL not defined, output nothing */
#ifdef ZB_TRACE_LEVEL
  zb_uint_t sec, msec;
#ifdef ZB_TRACE_TO_SYSLOG
  char buf[4096];
  int printed;
#endif

  if (!zb_trace_check(level, mask))
  {
    return;
  }

#ifndef ZB_TRACE_TO_SYSLOG
  if (!s_trace_file)
  {
    return;
  }
#endif

  ZVUNUSED(args_size);
  zb_osif_trace_lock();
  zb_osif_trace_get_time(&sec, &msec);

#ifdef ZB_TRACE_TO_SYSLOG

  printed = snprintf(buf, sizeof(buf), "%d %x %d/%d/%03d.%03d %s:%d\t",
                         zb_trace_get_counter(), osif_get_thread_id(), ZB_TIMER_GET(),
                         ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET()),
                         sec, msec,
                         file_name, line_number);
  zb_trace_inc_counter();

  if(printed > 0)
  {
    vsnprintf(&buf[printed], sizeof(buf) - printed, format, arglist);
#ifdef __ANDROID__
    __android_log_print(level == 0 ? ANDROID_LOG_ERROR : ANDROID_LOG_DEBUG,
                           g_tag, "%s", buf);
#else
    vsyslog(LOG_LOCAL7, "%s", buf);
#endif
  }

#else /* ZB_TRACE_TO_SYSLOG */

  {
    zb_char_t subsystem_name[MAX_SUBSYSTEM_NAME];
    zb_char_t ltime[200];

    fill_subsystem_name(subsystem_name, mask, level);
    ZB_BZERO(ltime, sizeof(ltime));
#ifdef ZB_PLATFORM_LINUX
    {
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);

      snprintf(ltime, sizeof(ltime), "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
#endif
#ifndef ZB_THREADS
    zb_osif_trace_printf(s_trace_file, "%d %s %d/%d/%03d.%03d %s:%d\t[%s] ",
                        zb_trace_get_counter(), ltime, ZB_TIMER_GET(),
                        ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET()),
                        sec, msec,
                        file_name, line_number, subsystem_name);
#else
    zb_osif_trace_printf(s_trace_file, "%d %x %s %d/%d/%03d.%03d %s:%d\t[%s] ",
                        zb_trace_get_counter(), osif_get_thread_id(), ltime, ZB_TIMER_GET(),
                        ZB_TIME_BEACON_INTERVAL_TO_MSEC(ZB_TIMER_GET()),
                        sec, msec,
                        file_name, line_number, subsystem_name);
#endif
  }
  zb_trace_inc_counter();
  zb_osif_trace_vprintf(s_trace_file, format, arglist);
  if (format[strlen(format) - 1] != '\n')
  {
    zb_osif_trace_printf(s_trace_file, "\n");
  }
  zb_osif_file_flush(s_trace_file);
#ifdef ZB_USE_LOGFILE_ROTATE
  zb_osif_log_file_rotate();
#endif

#endif /* ZB_TRACE_TO_SYSLOG */
  zb_osif_trace_unlock();
#else
  ZVUNUSED(mask);
  ZVUNUSED(level);
  ZVUNUSED(file_name);
  ZVUNUSED(line_number);
  ZVUNUSED(format);
  ZVUNUSED(args_size);
  ZVUNUSED(arglist);
#endif  /* ZB_TRACE_LEVEL */
}

/**
 * Output trace message.
 * Version with variable arguments
 *
 * @param format - printf-like format string
 * @param file_name - source file name
 * @param line_number - source file line
 * @param args_size - number of added parameters
 */
void zb_trace_msg_txt_file(
  zb_uint_t mask,
  zb_uint_t level,
  const zb_char_t *format,
  const zb_char_t *file_name,
#if defined ZB_BINARY_AND_TEXT_TRACE_MODE
  zb_uint16_t file_id,
#endif
  zb_int_t line_number,
  zb_int_t args_size, ...)
{
  va_list   arglist;

#if defined ZB_BINARY_AND_TEXT_TRACE_MODE
  ZVUNUSED(file_id);
#endif

  va_start(arglist, args_size);

  zb_trace_msg_txt_file_vl(mask,
                           level,
                           format,
                           file_name,
                           line_number,
                           args_size,
                           arglist);

  va_end(arglist);
}

#ifdef ZB_USE_LOGFILE_ROTATE
zb_ret_t zb_osif_log_file_rotate();
#endif

void zb_file_trace_vprintf(const char *format, va_list arglist)
{
  zb_osif_trace_vprintf(s_trace_file, (char*)format, arglist);
}

/** @} */
/** @endcond */ /* DOXYGEN_DEBUG_SECTION */

#endif  /* (defined ZB_TRACE_TO_FILE || defined ZB_TRACE_TO_SYSLOG) */
