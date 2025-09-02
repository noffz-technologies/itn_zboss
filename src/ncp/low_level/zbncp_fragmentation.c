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
/*  PURPOSE: NCP implementation of interface part of fragmentation.
*/

#define ZB_TRACE_FILE_ID 32

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_ll_pkt.h"
#include "zbncp.h"
#include "zbncp_fragmentation.h"
#include "zbncp_frag_internal.h"


void zbncp_frag_initialize(zbncp_frag_ctx_t *frag_ctx)
{
  zbncp_res_initialize(&frag_ctx->res);
  zbncp_div_initialize(&frag_ctx->div);
}


zbncp_int32_t zbncp_frag_store_tx_pkt(zbncp_frag_ctx_t *frag_ctx, const void* buf, zbncp_size_t size)
{
  return zbncp_div_store_tx_pkt(&frag_ctx->div, buf, size);
}


void zbncp_frag_set_place_for_rx_pkt(zbncp_frag_ctx_t *frag_ctx, void* buf, zbncp_size_t size)
{
  zbncp_res_set_place_for_rx_pkt(&frag_ctx->res, buf, size);
}


void zbncp_frag_fill_request(const zbncp_frag_ctx_t *frag_ctx, zbncp_ll_quant_req_t *req)
{
  req->rxmem = zbncp_res_get_place_for_fragment(&frag_ctx->res);
  req->tx_pkt = zbncp_div_request_fragment(&frag_ctx->div);
}


zbncp_bool_t zbncp_frag_process_tx_response(zbncp_frag_ctx_t *frag_ctx, const zbncp_ll_quant_res_t *rsp)
{
  return zbncp_div_process_response(&frag_ctx->div, rsp);
}


zbncp_size_t zbncp_frag_process_rx_response(zbncp_frag_ctx_t *frag_ctx, const zbncp_ll_quant_res_t *rsp)
{
  return zbncp_res_process_response(&frag_ctx->res, &rsp->rx_info);
}
