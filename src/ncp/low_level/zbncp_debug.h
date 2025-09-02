/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
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
/*  PURPOSE: Debug routines declaration.
*/
#ifndef ZBNCP_INCLUDE_GUARD_DEBUG_H
#define ZBNCP_INCLUDE_GUARD_DEBUG_H 1

#include "zbncp_types.h"

#if ZBNCP_DEBUG

typedef enum {
  ZBNCP_DBG_IGNORE,
  ZBNCP_DBG_BREAK,
  ZBNCP_DBG_ABORT,
} zbncp_dbg_res_t;

zbncp_dbg_res_t zbncp_dbg_assert(const char *file, unsigned int line, const char *cond);
void zbncp_dbg_break(void);
void zbncp_dbg_trace(const char *fmt, ...);

#define ZBNCP_DBG_ASSERT(cond)                          \
  do {                                                  \
    if (!(cond)) {                                      \
      zbncp_dbg_res_t res = zbncp_dbg_assert(__FILE__,  \
        __LINE__, #cond);                               \
      if (res == ZBNCP_DBG_BREAK) {                     \
        zbncp_dbg_break();                              \
      }                                                 \
    }                                                   \
  } while (0)

#define ZBNCP_DBG_STATIC_ASSERT(cond)                   \
  _Static_assert((cond), "Static assertion failed");

#define ZBNCP_DBG_TRACE(...)                            \
  do {                                                  \
    zbncp_dbg_trace("%24s: ", __func__);                \
    zbncp_dbg_trace(__VA_ARGS__);                       \
    zbncp_dbg_trace("\n");                              \
  } while (0)

#else /* ZBNCP_DEBUG */

#define ZBNCP_DBG_TRACE(...)          ((void) 0)
#define ZBNCP_DBG_ASSERT(cond)        ((void) 0)
#define ZBNCP_DBG_STATIC_ASSERT(cond)

#endif /* ZBNCP_DEBUG */

#endif /* ZBNCP_INCLUDE_GUARD_DEBUG_H */
