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
/*  PURPOSE:  ZBOSS NCP global type definitions.
*/
#ifndef ZBNCP_INCLUDE_GUARD_TYPES_H
#define ZBNCP_INCLUDE_GUARD_TYPES_H 1

#include "zbncp_defs.h"

/** @par Basic type definitions used across the library. */

#if ZBNCP_USE_STDTYPES

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>


typedef int8_t      zbncp_int8_t;
typedef int16_t     zbncp_int16_t;
typedef int32_t     zbncp_int32_t;
typedef int64_t     zbncp_int64_t;
typedef uint8_t     zbncp_uint8_t;
typedef uint16_t    zbncp_uint16_t;
typedef uint32_t    zbncp_uint32_t;
typedef uint64_t    zbncp_uint64_t;
typedef intptr_t    zbncp_intptr_t;
typedef uintptr_t   zbncp_uintptr_t;
typedef ptrdiff_t   zbncp_ptrdiff_t;
typedef size_t      zbncp_size_t;
typedef bool        zbncp_bool_t;

#define ZBNCP_FALSE false
#define ZBNCP_TRUE  true

#else /* ZBNCP_USE_STDTYPES */

typedef char                   zbncp_int8_t;
typedef short int              zbncp_int16_t;
typedef int                    zbncp_int32_t;
typedef long long int          zbncp_int64_t;
typedef unsigned char          zbncp_uint8_t;
typedef unsigned short int     zbncp_uint16_t;
typedef unsigned int           zbncp_uint32_t;
typedef unsigned long long int zbncp_uint64_t;

#if (ZBNCP_PTR_SIZE == 4u)
typedef zbncp_int32_t          zbncp_intptr_t;
typedef zbncp_uint32_t         zbncp_uintptr_t;
typedef zbncp_intptr_t         zbncp_ptrdiff_t;
typedef zbncp_uint32_t         zbncp_size_t;
#elif (ZBNCP_PTR_SIZE == 8u)
typedef zbncp_int64_t          zbncp_intptr_t;
typedef zbncp_uint64_t         zbncp_uintptr_t;
typedef zbncp_intptr_t         zbncp_ptrdiff_t;
typedef zbncp_uint64_t         zbncp_size_t;
#else
#error ZBNCP_PTR_SIZE should be defined either as 4u or as 8u.
#endif

typedef _Bool zbncp_bool_t;

#define ZBNCP_FALSE   ((zbncp_bool_t)0)
#define ZBNCP_TRUE    (!ZBNCP_FALSE)

#endif /* ZBNCP_USE_STDTYPES */

#define  ZBNCP_RET_OK         (0)
#define  ZBNCP_RET_ERROR      (-1)
#define  ZBNCP_RET_BLOCKED    (-2)
#define  ZBNCP_RET_EXIT       (-3)
#define  ZBNCP_RET_BUSY       (-4)
#define  ZBNCP_RET_TX_FAILED  (-5)
#define  ZBNCP_RET_NO_ACK     (-6)

#endif /* ZBNCP_INCLUDE_GUARD_TYPES_H */
