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
/*  PURPOSE: NCP low level fragmentation prototypes
*/
#ifndef ZBNCP_INCLUDE_GUARD_FRAGMENTATION_H
#define ZBNCP_INCLUDE_GUARD_FRAGMENTATION_H 1

#include "zbncp_types.h"
#include "zbncp_ll_proto.h"
#include "zbncp_frag_internal.h"

/**
 * @brief Fragmentation context structure
 */
typedef struct zbncp_frag_ctx_s
{
    zbncp_res_ctx_t res;    /**< Context for packet reassembling */
    zbncp_div_ctx_t div;    /**< Context for packet divider */
}
zbncp_frag_ctx_t;

/**
 * @brief Initialization of fragmentation context
 *
 * @param frag_ctx - context for fragmentation procedure
 */
void zbncp_frag_initialize(zbncp_frag_ctx_t *frag_ctx);

/**
 * @brief Storing a packet for transmission into internal buffer
 *
 * @param frag_ctx - context for fragmentation procedure
 * @param buf - pointer to a transmitted data
 * @param size - size of transmitted data
 * @return 0 if everything OK, otherwise error
 */
zbncp_int32_t zbncp_frag_store_tx_pkt(zbncp_frag_ctx_t *frag_ctx, const void* buf, zbncp_size_t size);

/**
 * @brief Pass place for copying a received packet to user
 *
 * @param frag_ctx - context for fragmentation procedure
 * @param buf - pointer to a memory where a packet will be stored
 * @param size - size of user buffer to not exceeding
 */
void zbncp_frag_set_place_for_rx_pkt(zbncp_frag_ctx_t *frag_ctx, void* buf, zbncp_size_t size);

/**
 * @brief Filling request to pass into low level
 *
 * @param frag_ctx - context for fragmentation procedure
 * @param req - request parameters, where fragments memory parameters are filled
 */
void zbncp_frag_fill_request(const zbncp_frag_ctx_t *frag_ctx, zbncp_ll_quant_req_t *req);

/**
 * @brief Processing TX response from low level
 *
 * @param frag_ctx - context for fragmentation procedure
 * @param rsp - information about received packet and transmitted packet
 * @return NCP LL transmitter readiness status, true - if NCP LL is ready to transmit next packet
 */
zbncp_bool_t zbncp_frag_process_tx_response(zbncp_frag_ctx_t *frag_ctx, const zbncp_ll_quant_res_t *rsp);

/**
 * @brief Processing RX response from low level
 *
 * @param frag_ctx - context for fragmentation procedure
 * @param rsp - information about received packet and transmitted packet
 * @return size of received packet, 0 - if no packet
 */
zbncp_size_t zbncp_frag_process_rx_response(zbncp_frag_ctx_t *frag_ctx, const zbncp_ll_quant_res_t *rsp);

#endif /* ZBNCP_INCLUDE_GUARD_FRAGMENTATION_H */
