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
/*  PURPOSE: NCP low level fragmentation internal structures and prototypes
*/
#ifndef ZBNCP_INCLUDE_GUARD_FRAG_INTERNAL_H
#define ZBNCP_INCLUDE_GUARD_FRAG_INTERNAL_H 1

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_ll_proto.h"

typedef struct zbncp_res_ctx_s
{
  void *user_buf;
  zbncp_size_t max_size;
  zbncp_size_t offset;
  zbncp_memref_t mem;
  zbncp_uint8_t big_buf[ZBNCP_BIG_BUF_SIZE];
}
zbncp_res_ctx_t;

typedef struct zbncp_div_ctx_s
{
  zbncp_size_t offset;
  zbncp_size_t pkt_size;
  zbncp_ll_tx_pkt_t tx_pkt;
  zbncp_uint8_t big_buf[ZBNCP_BIG_BUF_SIZE];
}
zbncp_div_ctx_t;

void zbncp_res_initialize(zbncp_res_ctx_t *res);
void zbncp_div_initialize(zbncp_div_ctx_t *div);

zbncp_int32_t zbncp_div_store_tx_pkt(zbncp_div_ctx_t *div, const void* buf, zbncp_size_t size);
zbncp_ll_tx_pkt_t zbncp_div_request_fragment(const zbncp_div_ctx_t *div);
zbncp_bool_t zbncp_div_process_response(zbncp_div_ctx_t *div, const zbncp_ll_quant_res_t *rsp);

void zbncp_res_set_place_for_rx_pkt(zbncp_res_ctx_t *res, void* buf, zbncp_size_t size);
zbncp_memref_t zbncp_res_get_place_for_fragment(const zbncp_res_ctx_t *res);
zbncp_size_t zbncp_res_process_response(zbncp_res_ctx_t *res, const zbncp_ll_rx_info_t *info);

#endif /* ZBNCP_INCLUDE_GUARD_FRAG_INTERNAL_H */
