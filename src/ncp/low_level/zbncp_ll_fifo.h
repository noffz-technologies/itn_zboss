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
/*  PURPOSE: NCP LL packet FIFO implementation.
*/
#ifndef ZBNCP_INCLUDE_GUARD_LL_FIFO_H
#define ZBNCP_INCLUDE_GUARD_LL_FIFO_H 1

#include "zbncp_fifo.h"
#include "zbncp_ll_pktbuf.h"

/** @brief LL packet buffer FIFO for packet transmission */
typedef struct zbncp_ll_fifo_s
{
  zbncp_fifo_t hdr;           /**< FIFO book-keeping structure */
  zbncp_ll_pktbuf_t *pool;    /**< Packet buffer pool */
}
zbncp_ll_fifo_t;

/** @brief Initialize the packet buffer FIFO */
static inline void zbncp_ll_fifo_init(zbncp_ll_fifo_t *fifo, zbncp_ll_pktbuf_t *pool, zbncp_size_t size)
{
  zbncp_fifo_init(&fifo->hdr, (zbncp_fifo_size_t) size);
  fifo->pool = pool;
}

/** @brief Obtain a pointer to the head packet buffer in the FIFO */
static inline zbncp_ll_pktbuf_t *zbncp_ll_fifo_head(const zbncp_ll_fifo_t *fifo)
{
  if (!zbncp_fifo_is_empty(&fifo->hdr))
  {
    zbncp_fifo_size_t head = zbncp_fifo_head(&fifo->hdr);
    return &fifo->pool[head];
  }
  return ZBNCP_NULL;
}

/** @brief Enqueue a packet buffer in the FIFO and return a pointer to the buffer */
static inline zbncp_ll_pktbuf_t *zbncp_ll_fifo_enqueue(zbncp_ll_fifo_t *fifo)
{
  if (!zbncp_fifo_is_full(&fifo->hdr))
  {
    zbncp_fifo_size_t tail = zbncp_fifo_enqueue(&fifo->hdr);
    return &fifo->pool[tail];
  }
  return ZBNCP_NULL;
}

/** @brief Dequeue a packet buffer from the FIFO */
static inline void zbncp_ll_fifo_dequeue(zbncp_ll_fifo_t *fifo)
{
  if (!zbncp_fifo_is_empty(&fifo->hdr))
  {
    zbncp_fifo_dequeue(&fifo->hdr);
  }
}

#endif /* ZBNCP_INCLUDE_GUARD_LL_FIFO_H */
