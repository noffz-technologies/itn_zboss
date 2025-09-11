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
/*  PURPOSE: NCP low level protocol packet implementation.
*/
#ifndef ZBNCP_INCLUDE_GUARD_LL_PKTBUF_H
#define ZBNCP_INCLUDE_GUARD_LL_PKTBUF_H 1

#include "zbncp_ll_pkt.h"

/** @brief Buffer for the NCP low-level protocol packet */
typedef struct zbncp_ll_pktbuf_s
{
  zbncp_size_t size;          /**< Size of the lwo-level packet */
  zbncp_ll_pkt_t pkt;         /**< Low-level packet data */
  zbncp_bool_t pkt_was_sent;  /**< Flag showed the packet was sent */
}
zbncp_ll_pktbuf_t;

/**
 * @brief Predicate to check whether the packet buffer contains packet body.
 *
 * @param pb - pointer to the packet buffer to check
 *
 * @return ZBNCP_TRUE if the buffer contains packet body, ZBNCP_FALSE otherwise
 */
static inline zbncp_bool_t zbncp_ll_pkt_has_body(const zbncp_ll_pktbuf_t *pb)
{
  return (pb->size > ZBNCP_LL_BODY_CRC_OFFSET);
}

/**
 * @brief Predicate to check whether the packet body in the packet buffer has valid CRC.
 *
 * @param pb - pointer to the packet buffer to check
 *
 * @return ZBNCP_TRUE if the buffer contains valid packet body, ZBNCP_FALSE otherwise
 */
static inline zbncp_bool_t zbncp_ll_pkt_body_crc_is_valid(const zbncp_ll_pktbuf_t *pb)
{
  zbncp_size_t bsize = zbncp_ll_hdr_body_size(&pb->pkt.hdr);
  zbncp_uint16_t bcrc = zbncp_ll_calc_body_crc(&pb->pkt.body, bsize);
  return (pb->pkt.body.crc == bcrc);
}

/**
 * @brief Initialize the packet buffer.
 *
 * @param pb - pointer to the packet buffer to check
 * @param size - initial size of the contained packet data
 *
 * @return nothing
 */
static inline void zbncp_ll_pktbuf_init(zbncp_ll_pktbuf_t *pb, zbncp_size_t size)
{
  pb->size = size;
}

/**
 * @brief Fill the packet buffer with the packet data.
 *
 * @param pb - pointer to the packet buffer to check
 * @param frameno - frame number to set in the packet header
 * @param flags - flags to set in the packet header
 * @param body - constant memory reference to the packet body data
 *
 * @return nothing
 */
static inline void zbncp_ll_pktbuf_fill(zbncp_ll_pktbuf_t *pb,
  zbncp_uint8_t frameno, zbncp_uint8_t flags, zbncp_cmemref_t body)
{
  zbncp_size_t shift = (((flags & ZBNCP_LL_FLAG_ACK) != 0u) ?
    ZBNCP_LL_ACK_NUM_SHIFT : ZBNCP_LL_PKT_NUM_SHIFT);

  if (body.size <= ZBNCP_LL_BODY_SIZE_MAX)
  {
    zbncp_ll_pktbuf_init(pb, zbncp_ll_calc_total_pkt_size(body.size));
    pb->pkt.sign[0] = ZBNCP_LL_SIGN_FIRST_BYTE;
    pb->pkt.sign[1] = ZBNCP_LL_SIGN_SECOND_BYTE;
    pb->pkt.hdr.len = zbncp_ll_calc_hdr_len(body.size);
    pb->pkt.hdr.type = ZBNCP_LL_TYPE_NCP;
    pb->pkt.hdr.flags = (zbncp_uint8_t)((flags & ZBNCP_LL_FLAGS_MASK) | ((frameno & ZBNCP_LL_NUM_MASK) << shift));
    pb->pkt.hdr.crc = zbncp_ll_hdr_calc_crc(&pb->pkt.hdr);
    if (zbncp_cmemref_is_valid(body))
    {
      zbncp_mem_copy(pb->pkt.body.data, body.ptr, body.size);
      pb->pkt.body.crc = zbncp_ll_calc_body_crc(&pb->pkt.body, body.size);
    }
  }
  else
  {
    zbncp_ll_pktbuf_init(pb, 0u);
  }
}

#endif /* ZBNCP_INCLUDE_GUARD_LL_PKTBUF_H */
