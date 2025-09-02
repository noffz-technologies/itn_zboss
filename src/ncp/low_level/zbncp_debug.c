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
/*  PURPOSE: Debug routines implementation.
*/

#define ZB_TRACE_FILE_ID 29
#include "zbncp_debug.h"

ZBNCP_DBG_STATIC_ASSERT(sizeof(zbncp_uint8_t)   == 1u)
ZBNCP_DBG_STATIC_ASSERT(sizeof(zbncp_uint16_t)  == 2u)
ZBNCP_DBG_STATIC_ASSERT(sizeof(zbncp_uint32_t)  == 4u)
ZBNCP_DBG_STATIC_ASSERT(sizeof(zbncp_uint64_t)  == 8u)
ZBNCP_DBG_STATIC_ASSERT(sizeof(zbncp_uintptr_t) == ZBNCP_PTR_SIZE)

#if ZBNCP_DEBUG

#if !ZBNCP_USE_ZBOSS_TRACE

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct zbncp_dbg_log_s
{
  const char *name;
  FILE *fp;
}
zbncp_dbg_log_t;

static void zbncp_dbg_log_open(zbncp_dbg_log_t *log)
{
  if (log->name != NULL)
  {
    log->fp = fopen(log->name, "w");
  }

  if (log->fp == NULL) {
    log->fp = stdout;
  }
}

static void zbncp_dbg_log_close(zbncp_dbg_log_t *log)
{
  if (log->fp != NULL && log->fp != stdout) {
    fclose(log->fp);
    log->fp = NULL;
  }
}

static zbncp_dbg_log_t zbncp_dbg_log; /* = { "LL.log" }; */

static void zbncp_dbg_log_atexit(void)
{
  zbncp_dbg_log_close(&zbncp_dbg_log);
}

static void zbncp_dbg_init_once(zbncp_dbg_log_t *log)
{
  static zbncp_bool_t initialized = ZBNCP_FALSE;
  if (!initialized)
  {
    atexit(zbncp_dbg_log_atexit);
    initialized = ZBNCP_TRUE;
  }

  if (log->fp == NULL)
  {
    zbncp_dbg_log_open(log);
  }
}

static void zbncp_dbg_logv(zbncp_dbg_log_t *log, const char *fmt, va_list ap)
{
  zbncp_dbg_init_once(log);
  vfprintf(log->fp, fmt, ap);
}

void zbncp_dbg_trace(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  zbncp_dbg_logv(&zbncp_dbg_log, fmt, ap);
  va_end(ap);
}

#else /* ZBNCP_USE_ZBOSS_TRACE */

#ifdef RET_OK
#undef RET_OK
#undef RET_ERROR
#undef RET_BLOCKED
#undef RET_EXIT
#undef RET_BUSY
#endif

#include "zb_common.h"
#ifdef ZB_TRACE_TO_FILE

void zb_file_trace_vprintf(const char *format, va_list arglist);

void zbncp_dbg_trace(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  zb_file_trace_vprintf(fmt, ap);
  va_end(ap);
}

#else /* ZB_TRACE_TO_FILE */

void zbncp_dbg_trace(const char *fmt, ...)
{
  TRACE_MSG(TRACE_COMMON3, "zbncp_dbg_trace: ZBOSS trace supported only to files", (FMT__0));
  ZBNCP_UNUSED(fmt);
}

#endif /* ZB_TRACE_TO_FILE */

#endif /* ZBNCP_USE_ZBOSS_TRACE */

zbncp_dbg_res_t zbncp_dbg_assert(const char *file, unsigned int line, const char *cond)
{
  zbncp_dbg_trace(
    "ZBNCP ASSERTION FAILED!\n"
    "  FILE: %s\n"
    "  LINE: %u\n"
    "  COND: '%s' does not hold!\n",
    file, line, cond);

  return ZBNCP_DBG_BREAK; /* for now - always break to the debugger */
}

void zbncp_dbg_break(void)
{
  /* Not implemented yet */
  for (;;)
  {
    /* Infinite loop */
  }
}

#endif /* ZBNCP_DEBUG */
