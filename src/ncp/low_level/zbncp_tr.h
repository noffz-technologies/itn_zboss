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
/*  PURPOSE: NCP low level protocol transport interface definition.
*/
#ifndef ZBNCP_INCLUDE_GUARD_TR_H
#define ZBNCP_INCLUDE_GUARD_TR_H 1

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_debug.h"


/** @brief Transport status constants. */
typedef enum zbncp_tr_send_status_e
{
  ZBNCP_TR_SEND_STATUS_SUCCESS,   /**< Transport has sent the data */
  ZBNCP_TR_SEND_STATUS_BUSY,      /**< Transport is busy right now, LL should retransmit packet later */
  ZBNCP_TR_SEND_STATUS_TIMEOUT,   /**< Send operation timed out, LL should retransmit packet later */
  ZBNCP_TR_SEND_STATUS_ERROR,     /**< Transport internal error, LL should retransmit packet later */
}
zbncp_tr_send_status_t;

/**
@addtogroup NCP_TRANSPORT_API
@{
*/

/**
   Maximum size of fragmented packet which can be transferred over NCP LL protocol.

   Contains payload @ref ZBNCP_MAX_FRAGMENTED_PAYLOAD_SIZE and reserved space @ref ZBNCP_BIG_BUF_RESERVE_SIZE

   Fragment size is defined by @ref ZBNCP_LL_PKT_SIZE_MAX
   @see NCP_HOST_MEM
 */
#ifndef ZBNCP_BIG_BUF_SIZE
#define ZBNCP_BIG_BUF_SIZE    1600U
#endif

/**
   Maximum size of fragmented payload.
 */
#ifndef ZBNCP_MAX_FRAGMENTED_PAYLOAD_SIZE
#define ZBNCP_MAX_FRAGMENTED_PAYLOAD_SIZE    1550U
#endif

/**
   Maximum size of reserved space for protocol headers.
 */
#ifndef ZBNCP_BIG_BUF_RESERVE_SIZE
#define ZBNCP_BIG_BUF_RESERVE_SIZE    50U
#endif

ZBNCP_DBG_STATIC_ASSERT((ZBNCP_MAX_FRAGMENTED_PAYLOAD_SIZE + ZBNCP_BIG_BUF_RESERVE_SIZE) <= ZBNCP_BIG_BUF_SIZE)

/** @brief Opaque transport object type used by low-level protocol. */
typedef struct zbncp_transport_s zbncp_transport_t;

/** @brief Opaque user-implemented transport object type. */
typedef struct zbncp_transport_impl_s zbncp_transport_impl_t;

/** @brief Opaque transport callback argument type. */
typedef struct zbncp_ll_proto_s *zbncp_tr_cb_arg_t;


/**
 * @brief Type of transport callback on initialization completion.
 *
 * @param arg - opaque argument for callback
 *
 * @return nothing
 */
typedef void zbncp_tr_init_callback(zbncp_tr_cb_arg_t arg);

/**
 * @brief Type of transport callback on sending data completion.
 *
 * @param arg - opaque argument for callback
 * @param status - send operation status
 *
 * @return nothing
 */
typedef void zbncp_tr_send_callback(zbncp_tr_cb_arg_t arg, zbncp_tr_send_status_t status);

/**
 * @brief Type of transport callback on receiving data completion.
*
 * @param arg - opaque argument for callback
 * @param size - count of received bytes
 *
 * @return nothing
 */
typedef void zbncp_tr_recv_callback(zbncp_tr_cb_arg_t arg, zbncp_size_t size);

/**
 * @brief Table of transport response callbacks.
 * These callbacks are provided by low-level protocol.
 * i/o layer calls that callback to indicate operation complete to LL logic layer.
 */
typedef struct zbncp_transport_cb_s
{
  zbncp_tr_init_callback *init;   /**< Callback on initialization complete */
  zbncp_tr_send_callback *send;   /**< Callback on sending data complete */
  zbncp_tr_recv_callback *recv;   /**< Callback on receiving data complete */
  zbncp_tr_cb_arg_t arg;          /**< Opaque argument for callback */
}
zbncp_transport_cb_t;


/**
 * @brief Creates global low level protocol object
 *
 * @param tr - pointer to a structure containing a table of user-provided
 *             transport implementation functions
 *
 * @return pointer to the global low-level protocol object
 */

/**
 * @brief Type of operation implementing transport initialization.
 *
 * @param tr - pointer to a transport implementation object
 * @param cb - pointer to a transport callback table
 *
 * @return nothing
 */
typedef void zbncp_transport_op_init(zbncp_transport_impl_t *tr, const zbncp_transport_cb_t *cb);

/**
 * @brief Type of operation implementing transport data sending.
 *
 * @param tr - pointer to a transport implementation object
 * @param mem - constant memory reference to a buffer with data to send
 *
 * @return nothing
 */
typedef void zbncp_transport_op_send(zbncp_transport_impl_t *tr, zbncp_cmemref_t mem);

/**
 * @brief Type of operation implementing transport data receiving.
 *
 * @param tr - pointer to a transport implementation object
 * @param mem - memory reference to a buffer for received data
 *
 * @return nothing
 */
typedef void zbncp_transport_op_recv(zbncp_transport_impl_t *tr, zbncp_memref_t mem);

/**
 * @brief Table of i/o layer transport requests: LL logic calls I/o.
 * These operations are provided by the i/o layer of low-level protocol.
 * The LL protocol is thus independent of an underlying transport.
 * The LL protocol logic calls that Requests to initiate appropriate operations. i/o then responses about operation complete using zbncp_transport_cb_t *cb contents.
 */
typedef struct zbncp_transport_ops_s
{
  zbncp_transport_impl_t *impl;   /**< Opaque pointer to a transport implementation object */
  zbncp_transport_op_init *init;  /**< Operation implementing transport initialization */
  zbncp_transport_op_send *send;  /**< Operation implementing transport data sending */
  zbncp_transport_op_recv *recv;  /**< Operation implementing transport data receiving */
}
zbncp_transport_ops_t;

/**
@}
*/

#endif /* ZBNCP_INCLUDE_GUARD_TR_H */
