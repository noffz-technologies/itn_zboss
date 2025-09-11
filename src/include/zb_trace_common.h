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
/* PURPOSE: internal definitions for ZigBee trace.
*/
#ifndef ZB_TRACE_COMMON_H
#define ZB_TRACE_COMMON_H 1

#include "zboss_api_core.h"
#include "zb_mac_transport.h"

#if defined(ZB_TRACE_LEVEL) || defined(ZB_TRAFFIC_DUMP_ON)


#define TRACE_HDR_SIG0 0xdeU
#define TRACE_HDR_SIG1 0xadU

#ifndef ZB_TRACE_TO_PORT
extern zb_osif_file_t *s_trace_file;
#endif

struct trace_hdr_s
{
  zb_uint8_t sig[2];
  zb_mac_transport_hdr_t h;
} ZB_PACKED_STRUCT;

void zb_trace_inc_counter(void);
void zb_trace_flush_bytes(void);

zb_bool_t zb_trace_check(zb_uint_t level, zb_uint_t mask);

#if defined(ZB_BINARY_TRACE)
zb_uint16_t zb_trace_rec_size(zb_uint16_t *args_size);
void zb_trace_put_hdr(zb_uint16_t file_id, zb_uint16_t trace_rec_len);
void zb_trace_put_u16(zb_uint16_t file_id, zb_uint16_t val);
void zb_trace_put_vararg(zb_uint16_t file_id, zb_minimal_vararg_t val);
/* should be implemented in the caller module */
void zb_trace_put_bytes(zb_uint16_t file_id, zb_uint8_t *buf, zb_short_t len);
#endif /* ZB_BINARY_TRACE */

#ifdef ZB_TRAFFIC_DUMP_ON
zb_uint16_t zb_dump_rec_size(zb_uint16_t dump_size);
void zb_dump_put_hdr(zb_uint_t dump_rec_len, zb_uint8_t mask);
void zb_trace_msg_port_do();
#endif /* ZB_TRAFFIC_DUMP_ON */

#endif /* ZB_TRACE_LEVEL || ZB_TRAFFIC_DUMP_ON */

#endif /* ZB_TRACE_COMMON_H */
