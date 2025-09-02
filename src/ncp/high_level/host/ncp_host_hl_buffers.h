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
/*  PURPOSE: NCP High level transport buffers declarations for the host side
*/

#ifndef NCP_HOST_HL_BUFFERS_H
#define NCP_HOST_HL_BUFFERS_H 1

#include "zb_common.h"
#include "zb_types.h"
#include "ncp_hl_proto.h"

typedef struct ncp_host_hl_rx_buf_handle_s
{
  zb_uint8_t *ptr;
  zb_size_t len;
  zb_size_t pos;
} ncp_host_hl_rx_buf_handle_t;

typedef struct ncp_host_hl_tx_buf_handle_s
{
  zb_uint8_t *ptr;
  zb_size_t len;
  zb_size_t pos;
  zb_uint8_t buf_id;
} ncp_host_hl_tx_buf_handle_t;

zb_ret_t ncp_host_hl_init_indication_body(ncp_hl_ind_header_t *hdr,
                                          zb_uint16_t len,
                                          ncp_host_hl_rx_buf_handle_t *handle);

zb_ret_t ncp_host_hl_init_response_body(ncp_hl_response_header_t *hdr,
                                        zb_uint16_t len,
                                        ncp_host_hl_rx_buf_handle_t *handle);

zb_ret_t ncp_host_hl_init_request_body(zb_uint8_t buf_id,
                                       ncp_hl_request_header_t *hdr,
                                       zb_uint16_t max_len,
                                       ncp_host_hl_tx_buf_handle_t *handle);


void ncp_host_hl_buf_get_u8(ncp_host_hl_rx_buf_handle_t *buf, zb_uint8_t *val);

void ncp_host_hl_buf_get_u16(ncp_host_hl_rx_buf_handle_t *buf, zb_uint16_t *val);

void ncp_host_hl_buf_get_u32(ncp_host_hl_rx_buf_handle_t *buf, zb_uint32_t *val);

void ncp_host_hl_buf_get_u64(ncp_host_hl_rx_buf_handle_t *buf, zb_uint64_t *val);

void ncp_host_hl_buf_get_u64addr(ncp_host_hl_rx_buf_handle_t *buf, zb_64bit_addr_t addr);

void ncp_host_hl_buf_get_u128key(ncp_host_hl_rx_buf_handle_t *buf, zb_uint8_t *key);

void ncp_host_hl_buf_get_array(ncp_host_hl_rx_buf_handle_t *buf, zb_uint8_t *data_ptr, zb_size_t size);

void ncp_host_hl_buf_get_ptr(ncp_host_hl_rx_buf_handle_t *buf, zb_uint8_t **data_ptr, zb_size_t size);


void ncp_host_hl_buf_put_u8(ncp_host_hl_tx_buf_handle_t *buf, zb_uint8_t val);

void ncp_host_hl_buf_put_u16(ncp_host_hl_tx_buf_handle_t *buf, zb_uint16_t val);

void ncp_host_hl_buf_put_u32(ncp_host_hl_tx_buf_handle_t *buf, zb_uint32_t val);

void ncp_host_hl_buf_put_u64(ncp_host_hl_tx_buf_handle_t *buf, zb_uint64_t val);

void ncp_host_hl_buf_put_u64addr(ncp_host_hl_tx_buf_handle_t *buf, const zb_64bit_addr_t addr);

void ncp_host_hl_buf_put_u128key(ncp_host_hl_tx_buf_handle_t *buf, zb_uint8_t *key);

void ncp_host_hl_buf_put_array(ncp_host_hl_tx_buf_handle_t *buf, zb_uint8_t *data_ptr, zb_size_t size);


zb_ret_t ncp_host_hl_buf_check_len(const ncp_host_hl_tx_buf_handle_t *buf, zb_size_t expected);

zb_uint16_t ncp_host_hl_tx_buf_get_len(ncp_host_hl_tx_buf_handle_t *handle);

#endif /* NCP_HOST_HL_BUFFERS_H */
