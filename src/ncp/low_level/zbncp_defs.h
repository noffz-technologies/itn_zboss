/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
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
/*  PURPOSE: ZBOSS NCP global definitions.
*/
#ifndef ZBNCP_INCLUDE_GUARD_DEFS_H
#define ZBNCP_INCLUDE_GUARD_DEFS_H 1

#ifndef ZBNCP_DEBUG
/**
 * @brief Configuration parameter defining whether to use debug
 * facilities of the library (debug tracing and asserting).
 */
#define ZBNCP_DEBUG 0
#endif /* ZBNCP_DEBUG */

#ifndef ZBNCP_USE_ZBOSS_TRACE
/**
 * @brief Configuration parameter defining whether to use ZBOSS
 * debug tracing facilities or implement custom ones.
 */
#define ZBNCP_USE_ZBOSS_TRACE 0
#endif /* ZBNCP_USE_ZBOSS_TRACE */

#ifndef ZBNCP_USE_STDTYPES
/**
 * @brief Configuration parameter defining whether to use types provided
 * by the C standard library, or to declare all the types ourselves.
 */
#define ZBNCP_USE_STDTYPES 0
#endif /* ZBNCP_USE_STDTYPES */

#ifndef ZBNCP_USE_STDMEM
/**
 * @brief Configuration parameter defining whether to use memory manipulation
 * routines from the standard library or to implement them ourselves.
 */
#define ZBNCP_USE_STDMEM 0
#endif /* ZBNCP_USE_STDMEM */

#ifndef ZBNCP_USE_STRUCT_PACKING
/**
 * @brief Configuration parameter defining whether to use compiler
 * extensions to define packed structures (i.e. structures with
 * fields aligned on a byte boundary without any padding). All the
 * structures requiring the packing are declared so that all the
 * fields are naturally aligned without any paddings. Nevertheless
 * it is very easy to break this behaviour unintentionally, so the
 * enforcement of the byte alignment rules can be desireable.
 */
#define ZBNCP_USE_STRUCT_PACKING 1
#endif /* ZBNCP_USE_STRUCT_PACKING */

#ifndef ZBNCP_USE_U8_FIFO_INDEX
/**
 * @brief Configuration parameter defining whether to use 8-bit integers
 * or platform native-sized integers for indexing FIFO buffers.
 */
#define ZBNCP_USE_U8_FIFO_INDEX 1
#endif /* ZBNCP_USE_U8_FIFO_INDEX */

#if ZBNCP_USE_STRUCT_PACKING

#if (defined __IAR_SYSTEMS_ICC__ || defined __ARMCC_VERSION) && !defined ZB8051
  /* IAR or Keil ARM CPU */
  #define ZBNCP_PACKED_PRE
  #define ZBNCP_PACKED_POST
#endif

#if defined __GNUC__ || defined __TI_COMPILER_VERSION__
  #define ZBNCP_PACKED_STRUCT __attribute__((packed))
#else
  #define ZBNCP_PACKED_STRUCT
#endif

#else

#define ZBNCP_PACKED_STRUCT

#endif

#if defined __GNUC__ && defined __x86_64__
/** @brief Definition of pointer size for 64-bit platform. */
#define ZBNCP_PTR_SIZE 8u
#else
/** @brief Definition of pointer size for 32-bit platform. */
#define ZBNCP_PTR_SIZE 4u
#endif

/**
 * @brief Macro to show to the compiler that we intentionally want
 * to have unused function argument or variable.
 */
#define ZBNCP_UNUSED(x) ((void)(x))

/** @brief Macro to determine a count of an array elements. */
#define ZBNCP_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))

/**
 Convert from microseconds to milliseconds
*/
#define ZBNCP_USECS_TO_MILLISECONDS(usec) ((usec) / (1000ull))

#endif /* ZBNCP_INCLUDE_GUARD_DEFS_H */
