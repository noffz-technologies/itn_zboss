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
/*  PURPOSE: NCP low level protocol API definition.
*/
#ifndef ZBNCP_INCLUDE_GUARD_LL_PROTO_H
#define ZBNCP_INCLUDE_GUARD_LL_PROTO_H 1

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_tr.h"
#include "zbncp_ll_time.h"

/** @brief NCP low-level protocol user callback argument type. */
typedef void *zbncp_ll_cb_arg_t;

/** @brief NCP low-level protocol user callback procedure type. */
typedef void zbncp_ll_callback(zbncp_ll_cb_arg_t arg);

/** @brief NCP low-level protocol user callback structure. */
typedef struct zbncp_ll_proto_cb_s
{
  zbncp_ll_callback *callme;    /**< User notification callback requesting for time quantum */
  zbncp_ll_cb_arg_t arg;        /**< Argument for the notification callback */
}
zbncp_ll_proto_cb_t;

#ifndef ZBNCP_LL_PROTO_T_DEFINED
#define ZBNCP_LL_PROTO_T_DEFINED
/** @brief Opaque definition of the low-level protocol internal state structure. */
typedef struct zbncp_ll_proto_s zbncp_ll_proto_t;
#endif /* ZBNCP_LL_PROTO_T_DEFINED */

/**
 * @brief Initialize NCP low-level protocol implementation.
 *
 * @param ll   - pointer to the low-level protocol internal state
 * @param cb   - pointer to the low-level protocol user callbacks
 * @param time - initial time for timeout calculations
 *
 * @return nothing
 */
void zbncp_ll_init(zbncp_ll_proto_t *ll, const zbncp_ll_proto_cb_t *cb, zbncp_ll_time_t time);

/**
 * @brief Input parameters (request) for tx packet
 */
typedef struct zbncp_ll_tx_pkt_s
{
  zbncp_uint8_t flags;      /**< Flags are showing start and end of a transmitted packet */
  zbncp_cmemref_t mem;    /**< Constant memory reference to a buffer with a data to be transmitted */
}
zbncp_ll_tx_pkt_t;

/**
 * @brief Information about a received packet
 */
typedef struct zbncp_ll_rx_info_s
{
  zbncp_uint8_t flags;     /**< Flags are showing start and end of a received packet */
  zbncp_size_t rxbytes;    /**< Count of bytes received during the time quantum */
}
zbncp_ll_rx_info_t;

/**
 * @brief Input parameters (request) ot the time quantum.
 */
typedef struct zbncp_ll_quant_req_s
{
  zbncp_ll_time_t time;            /**< Current time to be used for timeout calculation */
  zbncp_memref_t rxmem;            /**< Memory reference to a buffer for a data to be received */
  zbncp_ll_tx_pkt_t tx_pkt;        /**< Input parameters for a transmitted packet */
}
zbncp_ll_quant_req_t;

/**
 * @brief Output parameters (result) ot the time quantum.
 */
typedef struct zbncp_ll_quant_res_s
{
  zbncp_ll_rx_info_t rx_info;   /**< Information about a received packet */
  zbncp_size_t txbytes;         /**< Count of bytes transmitted during the time quantum */
  zbncp_ll_time_t timeout;      /**< Timeout till the next time quantum, 0 means "call the
                                 polling routine again as soon as possible" */
  zbncp_int32_t status;         /**< Status of LL state machines for RX/TX */
}
zbncp_ll_quant_res_t;

/**
 * @brief Structure describing time quantum input and output parameters.
 */
typedef struct zbncp_ll_quant_s
{
  zbncp_ll_quant_req_t req; /**< Request - input parameters */
  zbncp_ll_quant_res_t res; /**< Result - output parameters */
}
zbncp_ll_quant_t;

/**
 * @brief Poll NCP low-level protocol implementation and drive its
 * internal state machine for one time quantum.
 *
 * @param ll   - pointer to the low-level protocol internal state
 * @param q    - pointer to the structure describing quantum input
 *               and output parameters
 *
 * @return nothing
 */
void zbncp_ll_poll(zbncp_ll_proto_t *ll, zbncp_ll_quant_t *q);

#endif /* ZBNCP_INCLUDE_GUARD_LL_PROTO_H */
