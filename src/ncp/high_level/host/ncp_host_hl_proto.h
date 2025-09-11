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
/*  PURPOSE: NCP High level transport declarations for the host side
*/

#ifndef NCP_HOST_HL_PROTO_H
#define NCP_HOST_HL_PROTO_H 1

#include "zb_common.h"
#include "zb_types.h"
#include "ncp_hl_proto.h"
#include "ncp_host_hl_adapter.h"
#include "ncp_host_hl_transport_internal_api.h"
#include "ncp/ncp_host_api.h"
#include "ncp_host_hl_buffers.h"
#include "ncp_host_hl_proto_configuration.h"

/**
 * Handles a CONFIGURATION response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_configuration_response(void* data, zb_uint16_t len);

/**
 * Handles a CONFIGURATION indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_configuration_indication(void* data, zb_uint16_t len);

/**
 * Handles an AF response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_af_response(void* data, zb_uint16_t len);

/**
 * Handles an AF indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_af_indication(void* data, zb_uint16_t len);

/**
 * Handles a ZDO response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_zdo_response(void* data, zb_uint16_t len);

/**
 * Handles a ZDO indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_zdo_indication(void* data, zb_uint16_t len);

#ifdef ZB_ENABLE_INTER_PAN_NON_DEFAULT_CHANNEL
/**
 * Handles an Inter-PAN data indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_intrp_indication(void* data, zb_uint16_t len);

/**
 * Handles an Inter-PAN data response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_intrp_response(void* data, zb_uint16_t len);
#endif /* ZB_ENABLE_INTER_PAN_NON_DEFAULT_CHANNEL */

/**
 * Handles an APS response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_aps_response(void* data, zb_uint16_t len);

/**
 * Handles an APS indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_aps_indication(void* data, zb_uint16_t len);

/**
 * Handles an NWKMGMT response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_nwkmgmt_response(void* data, zb_uint16_t len);

/**
 * Handles an NWKMGMT indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_nwkmgmt_indication(void* data, zb_uint16_t len);

/**
 * Handles a SECUR response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_secur_response(void* data, zb_uint16_t len);

/**
 * Handles a SECUR indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_secur_indication(void* data, zb_uint16_t len);

/**
 * Handles a MANUF_TEST response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_manuf_test_response(void* data, zb_uint16_t len);

/**
 * Handles a MANUF_TEST indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_manuf_test_indication(void* data, zb_uint16_t len);

/**
 * Handles an OTA response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_ota_response(void* data, zb_uint16_t len);

/**
 * Handles an OTA indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_ota_indication(void* data, zb_uint16_t len);

/**
 * Handles a VENDOR response.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_vendor_response(void *data, zb_uint16_t len);

/**
 * Handles a VENDOR indication.
 *
 * @param data - pointer to the packet
 * @param len - length of the packet
 *
 */
void ncp_host_handle_vendor_indication(void *data, zb_uint16_t len);

/**
 * Obtains a buffer for a blocking request
 *
 * @param call_id
 * @param[out] handle - a handle to the new buffer, valid if return value is RET_OK
 * @param[out] tsn - a sequence number of request
 *                   Could be used for matching responses with requests.
 *
 * @return status of operation
 */
zb_ret_t ncp_host_get_buf_for_blocking_request(zb_uint16_t call_id,
                                               ncp_host_hl_tx_buf_handle_t* handle,
                                               zb_uint8_t* tsn);

/**
 * Obtains a buffer for a nonblocking request
 *
 * @param call_id
 * @param[out] handle - a handle to the new buffer, valid if return value is RET_OK
 * @param[out] tsn - a sequence number of request
 *                   Could be used for matching responses with requests.
 *
 * @return status of operation
 */
zb_ret_t ncp_host_get_buf_for_nonblocking_request(zb_uint16_t call_id,
                                                  ncp_host_hl_tx_buf_handle_t* buf,
                                                  zb_uint8_t* tsn);

/**
 * Marks current blocking request as finished which enables processing of other blocking requests.
 */
void ncp_host_mark_blocking_request_finished(void);


/**
 * Initializes the protocol and underlying LL transport.
 */
void ncp_host_hl_proto_init(void);


/**
 * Sends a packet over the NCP transport.
 *
 * @param buf - data buffer, obtained with ncp_host_get_buf_* call
 *
 * @return status of operation
 *         RET_OK - buffer is scheduled for transmission
 *         RET_BUSY - another transmission is in progress, try again later.
 */
zb_ret_t ncp_host_hl_send_packet(ncp_host_hl_tx_buf_handle_t *buf);


#endif /* NCP_HOST_HL_PROTO_H */
