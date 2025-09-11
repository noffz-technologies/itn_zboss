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
/*  PURPOSE: NCP High level transport implementation for the host side.
 *  Stubs for not yet used handlers */

#define ZB_TRACE_FILE_ID 95

#include "ncp_host_hl_transport_internal_api.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"



void ncp_host_handle_get_join_status_response(zb_ret_t status_code, zb_bool_t joined)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_get_rx_on_when_idle_response(zb_ret_t status_code,
                                                  zb_bool_t rx_on_when_idle)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_get_keepalive_timeout_response(zb_ret_t status_code,
                                                    zb_uint32_t keepalive_timeout)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_get_end_device_timeout_response(zb_ret_t status_code,
                                                     zb_uint8_t timeout)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_get_zigbee_role_response(zb_ret_t status_code,
                                              zb_uint8_t zigbee_role)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_get_nwk_keys_response(zb_ret_t status_code,
                                           zb_uint8_t *nwk_key1,
                                           zb_uint8_t key_number1,
                                           zb_uint8_t *nwk_key2,
                                           zb_uint8_t key_number2,
                                           zb_uint8_t *nwk_key3,
                                           zb_uint8_t key_number3)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_nwk_get_ieee_by_short_response(zb_ret_t status_code,
                                                    zb_ieee_addr_t ieee_addr)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_nwk_get_neighbor_by_ieee_response(zb_ret_t status_code)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_nwk_get_short_by_ieee_response(zb_ret_t status_code,
                                                    zb_uint16_t short_addr)
{
  ZB_ASSERT(0);
}

#ifdef ZB_APSDE_REQ_ROUTING_FEATURES
void ncp_host_handle_nwk_route_reply_indication_adapter(zb_bufid_t buf)
{
  ZB_ASSERT(0);
}

void ncp_host_handle_nwk_route_request_send_indication_adapter(zb_bufid_t buf)
{
  ZB_ASSERT(0);
}

void ncp_host_handle_nwk_route_record_send_indication_adapter(zb_bufid_t buf)
{
  ZB_ASSERT(0);
}
#endif


void ncp_host_handle_af_delete_endpoint_response(zb_ret_t status_code)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_af_set_node_descriptor_response(zb_ret_t status_code)
{
  ZB_ASSERT(0);
}


void ncp_host_handle_af_set_power_descriptor_response(zb_ret_t status_code)
{
  ZB_ASSERT(0);
}

#pragma GCC diagnostic pop
