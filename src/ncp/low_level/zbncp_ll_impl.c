/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
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
/*  PURPOSE: NCP low level protocol implementation.
*/

#define ZB_TRACE_FILE_ID 31
#include "zbncp_ll_impl.h"
#include "zbncp_tr_impl.h"

#define SINGLE_BYTE_LENGTH 1
#define EMPTY_PAYLOAD_LENGTH 0


static void zbncp_ll_tx_complete_pkt(zbncp_ll_proto_t *ll, zbncp_uint8_t flags);
static void zbncp_ll_change_sending_pkt_state(zbncp_ll_proto_t *ll, zbncp_bool_t state);


#if ZBNCP_DEBUG

static const char *zbncp_ll_rx_st(const zbncp_ll_proto_t *ll)
{
  const char *result;

  switch (ll->rx.state)
  {
    case ZBNCP_LL_RX_STATE_IDLE          : result = "RX-IDLE"; break;
    case ZBNCP_LL_RX_STATE_RECEIVING_HDR : result = "RX-RECEIVING-HDR"; break;
    case ZBNCP_LL_RX_STATE_HDR_RECEIVED  : result = "RX-HDR-RECEIVED"; break;
    case ZBNCP_LL_RX_STATE_HDR_VALIDATED : result = "RX-HDR-VALIDATED"; break;
    case ZBNCP_LL_RX_STATE_RECEIVING_BODY: result = "RX-RECEIVING_BODY"; break;
    case ZBNCP_LL_RX_STATE_BODY_RECEIVED : result = "RX-BODY-RECEIVED"; break;
    case ZBNCP_LL_RX_STATE_SEND_ACK      : result = "RX-SEND-ACK"; break;
    case ZBNCP_LL_RX_STATE_SENDING_ACK   : result = "RX-SENDING-ACK"; break;
    case ZBNCP_LL_RX_STATE_RESYNC        : result = "RX-RESYNC"; break;
    case ZBNCP_LL_RX_STATE_ERROR         : result = "RX-ERROR"; break;
    default: result = "RX-@UNK"; break;
  }

  return result;
}

static const char *zbncp_ll_tx_st(const zbncp_ll_proto_t *ll)
{
  const char *result;

  switch (ll->tx.state)
  {
    case ZBNCP_LL_TX_STATE_IDLE         : result = "TX-IDLE"; break;
    case ZBNCP_LL_TX_STATE_SENDING_ACK  : result = "TX-SENDING-ACK"; break;
    case ZBNCP_LL_TX_STATE_ACK_SENT     : result = "TX-ACK-SENT"; break;
    case ZBNCP_LL_TX_STATE_SENDING_PKT  : result = "TX-SENDING-PKT"; break;
    case ZBNCP_LL_TX_STATE_PKT_SENT     : result = "TX-PKT-SENT"; break;
    case ZBNCP_LL_TX_STATE_WAITING_ACK  : result = "TX-RECEIVING-ACK"; break;
    case ZBNCP_LL_TX_STATE_ACK_RECEIVED : result = "TX-ACK_RECEIVED"; break;
    case ZBNCP_LL_TX_STATE_ERROR        : result = "TX-ERROR"; break;
    default: result = "TX-@UNK"; break;
  }

  return result;
}
#endif /* ZBNCP_DEBUG */

/** @brief Returns the number of bytes, for which the NCP transport receiver should be configured. */
static zbncp_size_t zbncp_ll_rx_expected_size(zbncp_ll_proto_t *ll)
{
  zbncp_size_t result;

  switch (ll->rx.state) {
    case ZBNCP_LL_RX_STATE_RECEIVING_HDR:
      result = ll->rx.pkt.size;
      break;
    case ZBNCP_LL_RX_STATE_RECEIVING_BODY:
      result = (ll->rx.pkt.size - ZBNCP_LL_BODY_CRC_OFFSET);
      break;
    case ZBNCP_LL_RX_STATE_RESYNC:
      result = 1u;
      break;
    default:
      result = 0u;
      break;
  }

  return result;
}

/** @brief Executes the high level state machine iteration. */
static void zbncp_ll_callme(zbncp_ll_proto_t *ll, zbncp_bool_t immediate)
{
  if (ll->cb.callme != ZBNCP_NULL)
  {
    if (immediate)
    {
      ll->cb.callme(ll->cb.arg);
    }
    else
    {
      ll->service = ZBNCP_TRUE;
    }
  }
}

/** @brief Sets the NCP RX state machine state. */
static inline void zbncp_ll_rx_set_state(zbncp_ll_proto_t *ll, zbncp_ll_rx_state_t state)
{
#if ZBNCP_DEBUG
  const char *prev = zbncp_ll_rx_st(ll);
#endif /* ZBNCP_DEBUG */

  ll->rx.state = state;
  /* Request an NCP high level iteration execution. */
  zbncp_ll_callme(ll, ZBNCP_FALSE);

  ZBNCP_DBG_TRACE("(%s --> %s)", prev, zbncp_ll_rx_st(ll));
}

/** @brief Sets the NCP TX state machine state. */
static inline void zbncp_ll_tx_set_state(zbncp_ll_proto_t *ll, zbncp_ll_tx_state_t state)
{
#if ZBNCP_DEBUG
  const char *prev = zbncp_ll_tx_st(ll);
#endif /* ZBNCP_DEBUG */

  ll->tx.state = state;
  /* Request an NCP high level iteration execution. */
  zbncp_ll_callme(ll, ZBNCP_FALSE);

  ZBNCP_DBG_TRACE("(%s --> %s)", prev, zbncp_ll_tx_st(ll));
}

/** @brief Called after NCP transport is fully initialized and ready to accept RX/TX requests. */
static void zbncp_ll_init_complete(zbncp_ll_proto_t *ll)
{
  ZBNCP_DBG_TRACE("(%s : %s)", zbncp_ll_rx_st(ll), zbncp_ll_tx_st(ll));

  zbncp_ll_callme(ll, ZBNCP_TRUE);
}

/** @brief Called after NCP transport sends the last scheduled byte.
 *
 * This function may be called under the following circumstances:
 *  a) The NCP packet has been transmitted
 *  b) The NCP packet transmission has failed.
 *  c) The ACK/NACK has been transmitted.
 *  d) The ACK/NACK transmission has failed.
 *  e) The NCP transport transmitter encountered an error
 */
static void zbncp_ll_send_complete(zbncp_ll_proto_t *ll, zbncp_tr_send_status_t status)
{
#if ZBNCP_DEBUG
  const char *prev = zbncp_ll_tx_st(ll);
#endif /* ZBNCP_DEBUG */

  switch (ll->tx.state)
  {
    case ZBNCP_LL_TX_STATE_SENDING_ACK:
      if (status == ZBNCP_TR_SEND_STATUS_SUCCESS)
      {
        /* Case c) */
        ll->tx.tx_attempts_cnt = 0;
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_ACK_SENT);
      }
      else if (status == ZBNCP_TR_SEND_STATUS_BUSY)
      {
        /* Case d) */
        /* BUSY status can mean TX/TX conflict or another transaction in progress */
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE); /* Retransmit last ACK */
      }
      else
      {
        /* Case d) */
        ll->tx.tx_attempts_cnt++;
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE); /* Retransmit last ACK */
      }
      break;

    case ZBNCP_LL_TX_STATE_SENDING_PKT:
      if (status == ZBNCP_TR_SEND_STATUS_SUCCESS)
      {
        /* Case a) */
        ll->tx.tx_attempts_cnt = 0;
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_PKT_SENT); /* Start waiting for the ACK/NACK */
      }
      else if (status == ZBNCP_TR_SEND_STATUS_BUSY)
      {
        /* Case b) */
        /* BUSY status can mean TX/TX conflict or another transaction in progress */
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE); /* Retransmit last packet or send pending ACK */
      }
      else
      {
        /* Case b) */
        ll->tx.tx_attempts_cnt++;
        zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE); /* Retransmit last packet or send pending ACK */
      }
      break;

    default:
      /* Case e) */
      ll->tx.tx_attempts_cnt++;
      zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_ERROR);
      ZBNCP_DBG_TRACE("    TX ERROR!!!");
      break;
  }

  ZBNCP_DBG_TRACE("(%s : %s --> %s)", zbncp_ll_rx_st(ll), prev, zbncp_ll_tx_st(ll));

  zbncp_ll_callme(ll, ZBNCP_TRUE);
}

/** @brief Called after NCP transport receives the requested payload.
 *
 * This function may be called under the following circumstances:
 *  a) A new byte for the resynchronization logic has been received (looking for packet signature)
 *  b) The NCP packet header has been received
 *  c) The NCP body has been received
 *  d) The NCP transport was not able to receive the requested amount of bytes within the timeout.
 */
static void zbncp_ll_recv_complete(zbncp_ll_proto_t *ll, zbncp_size_t size)
{
  zbncp_size_t expected = zbncp_ll_rx_expected_size(ll);
#if ZBNCP_DEBUG
  const char *prev = zbncp_ll_rx_st(ll);
  if (size != expected)
  {
    ZBNCP_DBG_TRACE("    RX ERROR!!! expected size %zu != recv size %zu", expected, size);
  }
#endif /* ZBNCP_DEBUG */

  switch (ll->rx.state)
  {
    case ZBNCP_LL_RX_STATE_RESYNC:
    case ZBNCP_LL_RX_STATE_RECEIVING_HDR:
      zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_HDR_RECEIVED);
      break;

    case ZBNCP_LL_RX_STATE_RECEIVING_BODY:
      if (size == expected)
      {
        zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_BODY_RECEIVED);
      }
      else
      {
        zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_ERROR);
      }
      break;

    default:
      zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_ERROR);
      ZBNCP_DBG_TRACE("    RX ERROR!!!");
      break;
  }

  ZBNCP_DBG_TRACE("(%s --> %s : %s)", prev, zbncp_ll_rx_st(ll), zbncp_ll_tx_st(ll));

  zbncp_ll_callme(ll, ZBNCP_TRUE);
}

#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
/** @brief NCP packet send timeout has expired. */
static void zbncp_ll_alert_sending(zbncp_ll_proto_t *ll)
{
  ZBNCP_DBG_TRACE("(%s : %s)", zbncp_ll_rx_st(ll), zbncp_ll_tx_st(ll));

  /* Handle timeout */
  switch (ll->tx.state)
  {
    case ZBNCP_LL_TX_STATE_SENDING_ACK:
    case ZBNCP_LL_TX_STATE_SENDING_PKT:
    case ZBNCP_LL_TX_STATE_PKT_SENT:
      ZBNCP_DBG_TRACE("    ALARM TOO FAST! Sending was not complete.");
      ll->tx.tx_attempts_cnt++;
      zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE);
      break;

    default:
      ZBNCP_DBG_TRACE("    Unexpected TX state!");
      break;
  }

  /* Note that we are in the context of zbncp_ll_poll() routine
     so we do not need to call the "callme" callback because
     we just changed TX state, which means we will return zero
     timeout from the poll routine. */
}
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */

/** @brief NCP ACK/NACK receive timeout expired. */
static void zbncp_ll_alert_wait_ack(zbncp_ll_proto_t *ll)
{
  ZBNCP_DBG_TRACE("(%s : %s)", zbncp_ll_rx_st(ll), zbncp_ll_tx_st(ll));

  /* Reset the pkt_was_sent flag so the packet will be retransmitted. */
  zbncp_ll_change_sending_pkt_state(ll, ZBNCP_FALSE);

  /* Increase missing ACK/NACK counter. */
  ll->tx.missing_ack_cnt++;

  /* Schedule a new iteration of the main state machine */
  zbncp_ll_callme(ll, ZBNCP_FALSE);
}

/** @brief NCP RX/TX reset delay expired. */
static void zbncp_ll_alert_reset(zbncp_ll_proto_t *ll)
{
  ZBNCP_DBG_TRACE("Reset to IDLE: (%s : %s)", zbncp_ll_rx_st(ll), zbncp_ll_tx_st(ll));

  /* Reset the TX state. */
  if (ll->tx.state == ZBNCP_LL_TX_STATE_ERROR)
  {
    zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE);
  }

  /* Reset the RX state. */
  if (ll->rx.state == ZBNCP_LL_RX_STATE_ERROR)
  {
    zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_IDLE);
  }

  /* Schedule a new iteration of the main state machine */
  zbncp_ll_callme(ll, ZBNCP_FALSE);
}

/**
 * @brief Construct NCP low-level protocol implementation object.
 *
 * @details This function shall initialize the NCP LL layer structure (@ref zbncp_ll_proto_t).
 *
 * @param ll - pointer to the LL protocol state context
 * @param tr - pointer to the transport proxy
 */
void zbncp_ll_construct(zbncp_ll_proto_t *ll, zbncp_transport_t *tr)
{
  zbncp_mem_zero(ll, sizeof(*ll));
  ll->tr = tr;
}

/**
 * @brief Initialize NCP low-level protocol implementation.
 *
 * @param ll   - pointer to the low-level protocol internal state
 * @param cb   - pointer to the low-level protocol user callbacks
 * @param time - initial time for timeout calculations
 */
void zbncp_ll_init(zbncp_ll_proto_t *ll, const zbncp_ll_proto_cb_t *cb, zbncp_ll_time_t time)
{
  zbncp_transport_cb_t tcb;

  ll->time = 0;
#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  zbncp_ll_alarm_init(&ll->alarm_sending, zbncp_ll_alert_sending, ll, time);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
  zbncp_ll_alarm_init(&ll->alarm_wait_ack, zbncp_ll_alert_wait_ack, ll, time);
  zbncp_ll_alarm_init(&ll->alarm_reset, zbncp_ll_alert_reset, ll, time);
  ll->service = ZBNCP_FALSE;
  ll->status = ZBNCP_RET_OK;

  zbncp_ll_fifo_init(&ll->tx.fifo, ll->tx.pool, ZBNCP_LL_TX_FIFO_SIZE);

  ll->rx.state = ZBNCP_LL_RX_STATE_IDLE;
  ll->rx.frameno_handled_pkt = ZBNCP_LL_NUM_RX_INIT;
  ll->rx.frameno_ack = ZBNCP_LL_NUM_RX_INIT;
  ll->rx.ackflags = ZBNCP_LL_FLAGS_NONE;
  ll->rx.received = 0u;
  zbncp_ll_pktbuf_init(&ll->rx.pkt, 0u);

  ll->tx.state = ZBNCP_LL_TX_STATE_IDLE;
  ll->tx.frameno = ZBNCP_LL_NUM_TX_INIT;
  zbncp_ll_pktbuf_init(&ll->tx.ack, 0u);

  ll->cb = *cb;

  tcb.init = zbncp_ll_init_complete;
  tcb.send = zbncp_ll_send_complete;
  tcb.recv = zbncp_ll_recv_complete;
  tcb.arg = ll;

  zbncp_transport_init(ll->tr, &tcb);
}

/** @brief Schedule a new RX request for the next size bytes in the NCP transport. */
static inline void zbncp_ll_rx_recv(zbncp_ll_proto_t *ll, zbncp_ll_pktbuf_t *pb, zbncp_size_t size)
{
  zbncp_size_t offset = pb->size;

  if ((offset + size) <= sizeof(pb->pkt))
  {
    void *ptr = zbncp_offset_ptr(&pb->pkt, offset); // Store received bytes at offset
    zbncp_memref_t mem = zbncp_make_memref(ptr, size); // Request size bytes

    pb->size += size; // Increase the length of the current packet by size bytes

    zbncp_transport_recv(ll->tr, mem);
  }
  else
  {
    ZBNCP_DBG_TRACE("ERROR: size %zu too big!", size);
  }
}

/** @brief Schedule a new TX request in the NCP transport. */
static inline void zbncp_ll_tx_send_pkt(zbncp_ll_proto_t *ll, const zbncp_ll_pktbuf_t *pb)
{
  zbncp_cmemref_t mem = zbncp_make_cmemref(&pb->pkt, pb->size);
  zbncp_transport_send(ll->tr, mem);
}

/** @brief State handler: ZBNCP_LL_RX_STATE_IDLE
 *
 * @details The RX is in idle - prepare for receiving the next NCP packet header.
 */
static void zbncp_ll_rx_start_receiving(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *pb = &ll->rx.pkt;

  ZBNCP_DBG_TRACE("(%s)", zbncp_ll_rx_st(ll));

  zbncp_ll_pktbuf_init(pb, 0u);
  zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_RECEIVING_HDR);
  zbncp_ll_rx_recv(ll, pb, zbncp_ll_calc_total_pkt_size(EMPTY_PAYLOAD_LENGTH));
}

/** @brief Schedule a new RX request for a single byte to find the packet signature.
 *
 * @details The finding of the NCP packet header is done in the header reception routine.
 *          If the length of the header is received and the payload does not contain the
 *          correct header, this function will be called on each byte until the correct
 *          packet header will be in found.
 *          Once the correct header (signature, flags, length and CRC) is received,
 *          the NCP LL implementation will transition to the next state.
 */
static void zbncp_ll_rx_resync(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *pb = &ll->rx.pkt;
  char *ptr = (char *) &pb->pkt;

  ZBNCP_DBG_TRACE("(%s) RESYNC!!!", zbncp_ll_rx_st(ll));

  /* Drop the first byte of the packet and receive the
   * next from the transport, then revalidate the header
   * after recv completion. */
  --pb->size;
  zbncp_mem_move(ptr, &ptr[SINGLE_BYTE_LENGTH], pb->size);

  zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_RESYNC);
  zbncp_ll_rx_recv(ll, pb, SINGLE_BYTE_LENGTH);
}

/** @brief State handler: ZBNCP_LL_RX_STATE_HDR_RECEIVED
 *
 * @details This function will schedule reception over the NCP transport until it
 *          receives a valid NCP header.
 */
static void zbncp_ll_rx_validate_header(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *pb = &ll->rx.pkt;

  ZBNCP_DBG_TRACE("(%s) size %zu", zbncp_ll_rx_st(ll), pb->size);

  if (ll->rx.pkt.size != ZBNCP_LL_BODY_CRC_OFFSET)
  {
    /*
     * The NCP transport timed out while receiving the header.
     * Restart the RX for the remaining, missing payload length.
     */
    zbncp_ll_rx_recv(ll, &ll->rx.pkt, zbncp_ll_calc_total_pkt_size(0) - ll->rx.pkt.size);

    ZBNCP_DBG_TRACE("     Header reception ERROR!!! expected size %zu != pkt size %zu", ZBNCP_LL_BODY_CRC_OFFSET, ll->rx.pkt.size);
  }
  else if (pb->pkt.sign[0] == ZBNCP_LL_SIGN_FIRST_BYTE &&
      pb->pkt.sign[1] == ZBNCP_LL_SIGN_SECOND_BYTE)
  {
    if (zbncp_ll_hdr_crc_is_valid(&pb->pkt.hdr))
    {
      if (zbncp_ll_hdr_size_is_valid(&pb->pkt.hdr))
      {
        /* Header is OK - continue to receive body */
        zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_HDR_VALIDATED);
      }
      else
      {
        /* Invalid header length - drop the current payload and try to receive next header */
        zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_IDLE);
      }
    }
    else
    {
      /* Bad HDR crc - drop a single byte and scan for a valid signature again */
      zbncp_ll_rx_resync(ll);
    }
  }
  else
  {
    /* Bad signature - drop a single byte and scan for a valid signature again */
    zbncp_ll_rx_resync(ll);
  }
}

/** @brief State handler: ZBNCP_LL_RX_STATE_HDR_VALIDATED
 *
 * @details The packet contains a valid NCP packet header (including CRC).
 *          Schedule the reception of the packet body (if any).
 */
static void zbncp_ll_rx_continue_receiving(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *pb = &ll->rx.pkt;
  zbncp_size_t bsize = zbncp_ll_hdr_body_size(&pb->pkt.hdr);

  ZBNCP_DBG_TRACE("(%s) size %zu flags", zbncp_ll_rx_st(ll), pb->size);

  if (bsize != 0u)
  {
    /* Receive the body of the packet */
    zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_RECEIVING_BODY);
    zbncp_ll_rx_recv(ll, pb, bsize + ZBNCP_LL_BODY_CRC_SIZE);
  }
  else
  {
    /* No body - go straight to processing packet header with ACK information. */
    zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_BODY_RECEIVED);
  }
}

/** @brief Schedule sending ACK for the received packet.
 *
 * @details This function stores the most recent packet ACK flags. The ACK payload is constructed inside the TX idle state handler.
 */
static void zbncp_ll_acknowledge(zbncp_ll_proto_t *ll, zbncp_uint8_t pkt_num, zbncp_bool_t retransmit)
{
  ZBNCP_DBG_TRACE("zbncp_ll_acknowledge: Create new ACK data. Retransmit: %d", retransmit);

  /* Will use this frameno to detect duplicates */
  ll->rx.frameno_handled_pkt = pkt_num;

  /* Save received frame number to acknowledge received packet */
  /* Schedule the transmission of the ACK. */
  ll->rx.frameno_ack = pkt_num;

  /* Set the ACK frame flags */
  ll->rx.ackflags = ZBNCP_LL_FLAG_ACK;
  if (retransmit == ZBNCP_TRUE)
  {
      ll->rx.ackflags |= ZBNCP_LL_FLAG_RETRANSMIT;
  }
}

/** @brief State handler: ZBNCP_LL_RX_STATE_BODY_RECEIVED
 *
 * @details The packet contains a valid NCP packet header and the packet body (id present, including body CRC).
 *          Validate the packet body.
 */
static zbncp_ll_rx_info_t zbncp_ll_rx_complete_receiving(zbncp_ll_proto_t *ll, zbncp_memref_t mem)
{
  zbncp_ll_pktbuf_t *pb = &ll->rx.pkt;
  zbncp_uint8_t rxflags = pb->pkt.hdr.flags;
  zbncp_ll_rx_info_t rx_info = {0u, 0u};

  ZBNCP_DBG_TRACE("(%s) rxflags %#02x", zbncp_ll_rx_st(ll), rxflags);

  if ((rxflags & ZBNCP_LL_FLAG_ACK) != ZBNCP_LL_FLAGS_NONE)
  {
    /* Try to acknowledge any pending TX packet */
    zbncp_ll_tx_complete_pkt(ll, rxflags);
    /* Do not drop the packet body and make the RX state transition here.
     * It is possible to support piggyback ACKs and transfer packet payload with the ACK included.
     */
  }

  /* Note that any packet with a valid header and non-zero payload has to be acknowledged. */
  if (zbncp_ll_hdr_body_size(&pb->pkt.hdr) != 0)
  {
    zbncp_uint8_t rxframeno = zbncp_ll_flags_pkt_num(rxflags);

    /* Received packet can be a RESET response which always has 0x00 frameno and hasn't to be rejected as a duplicate */
    if ((ll->rx.frameno_handled_pkt == rxframeno) && (ll->rx.frameno_handled_pkt != 0u))
    {
      /* Duplicate - silently acknowledge and discard the packet */
      ZBNCP_DBG_TRACE("     Duplicate frame %#02x - discard", rxframeno);
      zbncp_ll_acknowledge(ll, rxframeno, ZBNCP_FALSE);
    }
    else
    {
      /* Check if body contains CRC. */
      if (zbncp_ll_pkt_has_body(pb))
      {
        if (zbncp_ll_pkt_body_crc_is_valid(pb))
        {
          if (zbncp_ll_hdr_type_is_supported(&pb->pkt.hdr))
          {
            /* OK, TX will eventually send ACK/NACK which means we can return
               body data to the user and start to receive the next packet */
            if (zbncp_memref_is_valid(mem))
            {
              rx_info.rxbytes = size_min(mem.size, zbncp_ll_hdr_body_size(&pb->pkt.hdr));
              rx_info.flags = pb->pkt.hdr.flags;
              zbncp_mem_copy(mem.ptr, pb->pkt.body.data, rx_info.rxbytes);

              /* Packet will be passed to the application - send ACK */
              zbncp_ll_acknowledge(ll, rxframeno, ZBNCP_FALSE);
            }
            else
            {
              /* If LL doesn't have free space to store the packet ACK isn't sent for flow control purposes */
              ZBNCP_DBG_TRACE("     Unable to pass the frame %#02x to the application - skip ACK/NACK transmission", rxframeno);
            }
          }
          else
          {
            /* HDR type is not supported - silently acknowledge and discard the packet */
            ZBNCP_DBG_TRACE("     HDR type %#02x is not supported - discard", pb->pkt.hdr.type);
            zbncp_ll_acknowledge(ll, rxframeno, ZBNCP_FALSE);
          }
        }
        else
        {
          /* Body CRC invalid - ask other side to retransmit the packet */
          ZBNCP_DBG_TRACE("     Invalid body CRC %#04x - should retransmit", pb->pkt.body.crc);
          zbncp_ll_acknowledge(ll, rxframeno, ZBNCP_TRUE);
        }
      }
      else
      {
        /* Packet has no body - nothing to extract */
        ZBNCP_DBG_TRACE("     Packet without body - discard");
        zbncp_ll_acknowledge(ll, rxframeno, ZBNCP_FALSE);
      }
    }
  }

  /* Receive next packet */
  zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_IDLE);
  zbncp_ll_rx_start_receiving(ll);

  return rx_info;
}

/** @brief Entry point of the main RX state machine. */
static zbncp_ll_rx_info_t zbncp_ll_process_rx(zbncp_ll_proto_t *ll, zbncp_memref_t mem)
{
  zbncp_ll_rx_info_t rx_info = {0u, 0u};

  switch (ll->rx.state)
  {
    case ZBNCP_LL_RX_STATE_IDLE:
      zbncp_ll_rx_start_receiving(ll);
      break;

    case ZBNCP_LL_RX_STATE_RECEIVING_HDR:
      /* DO NOTHING - wait for HDR */
      break;

    case ZBNCP_LL_RX_STATE_HDR_RECEIVED:
      zbncp_ll_rx_validate_header(ll);
      break;

    case ZBNCP_LL_RX_STATE_HDR_VALIDATED:
      zbncp_ll_rx_continue_receiving(ll);
      break;

    case ZBNCP_LL_RX_STATE_RECEIVING_BODY:
      /* DO NOTHING - wait for BODY */
      break;

    case ZBNCP_LL_RX_STATE_BODY_RECEIVED:
      rx_info = zbncp_ll_rx_complete_receiving(ll, mem);
      break;

    case ZBNCP_LL_RX_STATE_RESYNC:
      /* DO NOTHING - wait for next byte */
      break;

    case ZBNCP_LL_RX_STATE_ERROR:
      ZBNCP_DBG_TRACE("     RX ERROR!");
      if (zbncp_ll_alarm_timeout(&ll->alarm_reset) != ZBNCP_LL_TIMEOUT_INFINITE)
      {
        zbncp_ll_alarm_set(&ll->alarm_reset, ZBNCP_LL_RESET_TIMEOUT);
      }
      break;

    case ZBNCP_LL_RX_STATE_SEND_ACK:
      /* State not present. ACKs will be generated based on the ll->rx.ackflags
       * from the IDLE state of the TX logic.
       */
    case ZBNCP_LL_RX_STATE_SENDING_ACK:
      /* State not present. Use ZBNCP_LL_TX_STATE_SENDING_ACK instead.
       */
    default:
      ZBNCP_DBG_TRACE("     Invalid RX state %u", ll->rx.state);
      zbncp_ll_rx_set_state(ll, ZBNCP_LL_RX_STATE_IDLE);
      break;
  }

  /*
   * Static analysis tool generates false positives for MISRA Rule 2.2.
   * It claims that rx_info.flags and rx_info.rxbytes are not used. Actually, rx_info is returned from a function.
   * Mark the fields explicitly as unused to silence MISRA checker's false positive.
   */
  (void)rx_info.flags;
  (void)rx_info.rxbytes;
  return rx_info;
}

/** @brief Returns the next NCP LL frame number to be used during transmission. */
static zbncp_uint8_t zbncp_ll_tx_alloc_frameno(zbncp_ll_proto_t *ll)
{
  zbncp_uint8_t prev = ll->tx.frameno;
  zbncp_uint8_t next = (prev + 1u) & ZBNCP_LL_NUM_MASK;
  if (next == ZBNCP_LL_NUM_BOOT)
  {
    /* To prevent throwing out boot indication from NCP as a duplicate
     * at a host side use packet #0 only once, then always skip it. */
    ++next;
  }
  ll->tx.frameno = next;

  return prev;
}

/** @brief Allocate a new TX packet and reset its state. */
static zbncp_size_t zbncp_ll_tx_alloc_pkt(zbncp_ll_proto_t *ll, zbncp_ll_tx_pkt_t tx_pkt)
{
  zbncp_size_t txbytes = 0;

  if (zbncp_cmemref_is_valid(tx_pkt.mem))
  {
    zbncp_ll_pktbuf_t *pb = zbncp_ll_fifo_enqueue(&ll->tx.fifo);
    if (pb != ZBNCP_NULL)
    {
      zbncp_uint8_t frameno = zbncp_ll_tx_alloc_frameno(ll);
      zbncp_ll_pktbuf_fill(pb, frameno, tx_pkt.flags, tx_pkt.mem);
      pb->pkt_was_sent = ZBNCP_FALSE;
      ZBNCP_DBG_TRACE("(%s) size %zu frame #%x", zbncp_ll_tx_st(ll), pb->size, frameno);
      txbytes = pb->size;
    }
  }

  return txbytes;
}

/** @brief Prepares a new ACK payload, based on the ACK flags, stored inside the RX state. */
static void zbncp_ll_tx_pop_ack(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *ack = &ll->tx.ack;

  /* Sanity check - ackflags should have the ACK flag set */
  if ((ll->rx.ackflags & ZBNCP_LL_FLAG_ACK) != ZBNCP_LL_FLAGS_NONE)
  {
    /* Try to construct a new ACK or NACK payload */
    if (ack->size == 0u)
    {
        ZBNCP_DBG_TRACE("(%s) Send ACK frameno %#02x ackflags %#02x",
          zbncp_ll_rx_st(ll), ll->rx.frameno_ack, ll->rx.ackflags);

        /* Prepare TX packet with no body */
        zbncp_ll_pktbuf_fill(ack, ll->rx.frameno_ack, ll->rx.ackflags, zbncp_cmemref_null());
        /* Reset flags to store flags for the next packet. */
        ll->rx.ackflags = ZBNCP_LL_FLAGS_NONE;
    }
    else
    {
      /* TX is busy, we need to try to send ACK/NACK once more -
         stay in the same state */
    }
  }
  else if (ll->rx.ackflags != ZBNCP_LL_FLAGS_NONE)
  {
    ZBNCP_DBG_TRACE("     INTERNAL ERROR: bad ack flags!");
    ll->rx.ackflags = ZBNCP_LL_FLAGS_NONE;
  }
}

/** @brief State handler: ZBNCP_LL_TX_STATE_IDLE
 *
 * @details In this state the NCP LL checks if there are any pending ACKs or packets to be sent.
 *          The ACKs/NACKs should be prioritized over regular NCP packtes.
 */
static void zbncp_ll_tx_start_sending(zbncp_ll_proto_t *ll)
{
  /* ACK packets are TOP RIORITY, so if we have a pending ACK packet
   * then we should send it immediately, and only then we can return
   * to the packets from TX queue. */
  zbncp_ll_pktbuf_t *pb = &ll->tx.ack;
  zbncp_ll_tx_state_t state = ZBNCP_LL_TX_STATE_SENDING_ACK;
  zbncp_bool_t need_to_send_pkt = ZBNCP_FALSE;

  /* Check if the transport TX is working correctly. */
  if ((ZBNCP_LL_TX_ATTEMPTS != ZBNCP_LL_TX_ATTEMPTS_INFINITY) &&
      (ll->tx.tx_attempts_cnt >= ZBNCP_LL_TX_ATTEMPTS))
  {
    zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_ERROR);
#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
    zbncp_ll_alarm_cancel(&ll->alarm_sending);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
    zbncp_ll_alarm_cancel(&ll->alarm_wait_ack);
    ll->status = ZBNCP_RET_TX_FAILED;
  }
  else
  {
    /* Check if ACK timeout has been reached. */
    if ((ZBNCP_LL_WAIT_ACK_RETRY != ZBNCP_LL_ACK_RETRY_INFINITY) &&
        (ll->tx.missing_ack_cnt >= ZBNCP_LL_WAIT_ACK_RETRY))
    {
      zbncp_ll_alarm_cancel(&ll->alarm_wait_ack);
      ll->tx.missing_ack_cnt = 0;
      zbncp_ll_fifo_dequeue(&ll->tx.fifo);
      ll->status = ZBNCP_RET_NO_ACK;
    }

    /* Pop next ACK frame and store inside the ll->tx.ack. */
    /* TODO: Piggyback transmission. */
    zbncp_ll_tx_pop_ack(ll);

    if (pb->size == 0u)
    {
      /* No ACK pending - send regular TX packet */
      pb = zbncp_ll_fifo_head(&ll->tx.fifo);
      state = ZBNCP_LL_TX_STATE_SENDING_PKT;

      if (pb != ZBNCP_NULL)
      {
        /* Check if the packet has been sent and if we are waiting for the ACK timeout. */
        if (!pb->pkt_was_sent)
        {
          /* In this state:
           * - A new packet will be transmitted.
           * - If current packet has already been sent, its state was reset by NACK reception.
           * - Alternatively, the ACK/NACK reception timeout has not been reached and the packet flags were reset in the alarm handler.
           */
          need_to_send_pkt = ZBNCP_TRUE;
          /* Mark the packet as sent, so the TX logic will not retransmit it immediately. */
          zbncp_ll_change_sending_pkt_state(ll, ZBNCP_TRUE);
        }
      }
    }
    else
    {
      need_to_send_pkt = ZBNCP_TRUE;
    }

    if ((need_to_send_pkt) && (pb != ZBNCP_NULL))
    {
      ZBNCP_DBG_TRACE("(%s) size %zu%s", zbncp_ll_tx_st(ll),
        pb->size, ((pb == &ll->tx.ack) ? " -- ACK" : ""));

      zbncp_ll_tx_set_state(ll, state);
#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
      zbncp_ll_alarm_set(&ll->alarm_sending, ZBNCP_LL_PKT_SEND_TIMEOUT);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
      zbncp_ll_tx_send_pkt(ll, pb);
    }
  }
}

/** @brief State handler: ZBNCP_LL_TX_STATE_ACK_SENT
 *
 * @details ACK frame has been sent. Make a space for the next ACK and continue from the idle state.
 */
static void zbncp_ll_tx_complete_ack(zbncp_ll_proto_t *ll)
{
  ZBNCP_DBG_TRACE("(%s) (%s)", zbncp_ll_tx_st(ll), zbncp_ll_rx_st(ll));

#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  zbncp_ll_alarm_cancel(&ll->alarm_sending);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
  /* Free the space for the next ACK payload. */
  ll->tx.ack.size = 0u;
  zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE);
}

/** @brief State handler: ZBNCP_LL_TX_STATE_PKT_SENT
 *
 * @details The packet has been successfully sent through the NCP transport.
 *           - Stop packet TX alarm
 *           - Start packet ACK alarm
 *           - Return to the IDLE state to send any pending ACKs or packets.
 */
static void zbncp_ll_tx_start_waiting_ack(zbncp_ll_proto_t *ll)
{
  zbncp_ll_pktbuf_t *pb = zbncp_ll_fifo_head(&ll->tx.fifo);

  ZBNCP_DBG_TRACE("(%s) (%s)", zbncp_ll_tx_st(ll), zbncp_ll_rx_st(ll));

#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  zbncp_ll_alarm_cancel(&ll->alarm_sending);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */

  /* Verify that the ACK was not received while switching between
   * ZBNCP_LL_TX_STATE_SENDING_PKT and ZBNCP_LL_TX_STATE_PKT_SENT states.
   */
  if ((pb != ZBNCP_NULL) && (pb->pkt_was_sent))
  {
    zbncp_ll_alarm_set(&ll->alarm_wait_ack, ZBNCP_LL_ACK_WAIT_TIMEOUT);
  }

  zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE);
}

/** @brief The NCP packet ACK handler. This function is called whenever a valid ACK data is received. */
static void zbncp_ll_tx_complete_pkt(zbncp_ll_proto_t *ll, zbncp_uint8_t rxflags)
{
  zbncp_ll_pktbuf_t *pb = zbncp_ll_fifo_head(&ll->tx.fifo);

  ZBNCP_DBG_TRACE("(%s) ACK flags %#02x", zbncp_ll_tx_st(ll), rxflags);

  if (pb == ZBNCP_NULL)
  {
    /* Unexpected ACK - silently ignore it */
    ZBNCP_DBG_TRACE("     Received ACK for an unknown packet");
  }
  else
  {
    zbncp_uint8_t txflags = pb->pkt.hdr.flags;
    zbncp_uint8_t rxframeno = zbncp_ll_flags_ack_num(rxflags);
    zbncp_uint8_t txframeno = zbncp_ll_flags_pkt_num(txflags);

    /* Check TX pkt # */
    if (rxframeno == txframeno)
    {
      zbncp_ll_alarm_cancel(&ll->alarm_wait_ack);

      ZBNCP_DBG_TRACE("zbncp_ll_tx_complete_pkt: Retransmit: %d", (rxflags & ZBNCP_LL_FLAG_RETRANSMIT));

      if ((rxflags & ZBNCP_LL_FLAG_RETRANSMIT) != ZBNCP_LL_FLAGS_NONE)
      {
        /* Received NACK */
        /* Reset the pkt_was_sent flag so the packet will be retransmitted. */
        zbncp_ll_change_sending_pkt_state(ll, ZBNCP_FALSE);
      }
      else
      {
        /* Received ACK */
        ll->tx.missing_ack_cnt = 0;
        zbncp_ll_fifo_dequeue(&ll->tx.fifo);
      }
    }
    else
    {
      /* ACK with unexpected frame number - silently ignore it */
      ZBNCP_DBG_TRACE("     ACK with unexpected #%x (expected #%x)", txframeno, rxframeno);
    }
  }

  /* There is no need to schedule the TX loop iteration - it will be done by the RX handler, that called this function. */
}

/** @brief Marks the flag in the most recent packet, indicating if the packet has/has not been passed to the NCP transport. */
static void zbncp_ll_change_sending_pkt_state(zbncp_ll_proto_t *ll, zbncp_bool_t state)
{
  zbncp_ll_pktbuf_t *pb = zbncp_ll_fifo_head(&ll->tx.fifo);

  if (pb != ZBNCP_NULL)
  {
    pb->pkt_was_sent = state;
  }
  else
  {
    /* FIFO is empty */
    ZBNCP_DBG_TRACE("     Packet for sending is absent");
  }
}

/** @brief Entry point of the main TX state machine. */
static zbncp_size_t zbncp_ll_process_tx(zbncp_ll_proto_t *ll, zbncp_ll_tx_pkt_t tx_pkt)
{
  zbncp_size_t txbytes = zbncp_ll_tx_alloc_pkt(ll, tx_pkt);

  switch (ll->tx.state)
  {
    case ZBNCP_LL_TX_STATE_IDLE:
      zbncp_ll_tx_start_sending(ll);
      break;

    case ZBNCP_LL_TX_STATE_SENDING_ACK:
      /* DO NOTHING - state will be changed on send completion */
      break;

    case ZBNCP_LL_TX_STATE_ACK_SENT:
      zbncp_ll_tx_complete_ack(ll);
      break;

    case ZBNCP_LL_TX_STATE_SENDING_PKT:
      /* DO NOTHING - state will be changed on send completion */
      break;

    case ZBNCP_LL_TX_STATE_PKT_SENT:
      zbncp_ll_tx_start_waiting_ack(ll);
      break;

    case ZBNCP_LL_TX_STATE_ERROR:
      ZBNCP_DBG_TRACE("     TX ERROR!");
      ZBNCP_DBG_TRACE("tx attempts: %d", ll->tx.tx_attempts_cnt);
      ll->status = ZBNCP_RET_TX_FAILED;

      /* Cancel all TX alarms. */
#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
      zbncp_ll_alarm_cancel(&ll->alarm_sending);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
      zbncp_ll_alarm_cancel(&ll->alarm_wait_ack);

      /* Reset the pkt_was_sent flag so the packet will be retransmitted. */
      zbncp_ll_change_sending_pkt_state(ll, ZBNCP_FALSE);
      if (zbncp_ll_alarm_timeout(&ll->alarm_reset) != ZBNCP_LL_TIMEOUT_INFINITE)
      {
        zbncp_ll_alarm_set(&ll->alarm_reset, ZBNCP_LL_RESET_TIMEOUT);
      }
      break;

    case ZBNCP_LL_TX_STATE_WAITING_ACK:
      /* State not present. It is allowed to receive ACK at any time from the RX state machine.
       * The ACK timeout is only started from the TX machine and marks the packet state if expired.
       */
    case ZBNCP_LL_TX_STATE_ACK_RECEIVED:
      /* State not present. The pakcet is popped from the TX queue by the RX state machine
       * as a result of the correct ACK reception.
       * The idle state will send the next packet if there are no pending ACKs.
       */
    default:
      ZBNCP_DBG_TRACE("     Unknown TX state %u", ll->tx.state);
      zbncp_ll_tx_set_state(ll, ZBNCP_LL_TX_STATE_IDLE);
      break;
  }

  return txbytes;
}

/**
 * @brief Poll NCP low-level protocol implementation and drive its
 * internal state machine for one time quantum.
 * called from the NCP high level state machine iteration.
 *
 * @param ll   - pointer to the low-level protocol internal state
 * @param q    - pointer to the structure describing quantum input
 *               and output parameters
 */
void zbncp_ll_poll(zbncp_ll_proto_t *ll, zbncp_ll_quant_t *q)
{
  zbncp_ll_time_t timeout;

  ZBNCP_DBG_TRACE("@ time %zu [ack size %zu]", q->req.time, ll->tx.ack.size);

  ll->service = ZBNCP_FALSE;
  ll->status = ZBNCP_RET_OK;

#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  zbncp_ll_alarm_update_time(&ll->alarm_sending, q->req.time);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
  zbncp_ll_alarm_update_time(&ll->alarm_wait_ack, q->req.time);

  q->res.txbytes = zbncp_ll_process_tx(ll, q->req.tx_pkt);
  q->res.rx_info = zbncp_ll_process_rx(ll, q->req.rxmem);

  timeout = zbncp_ll_alarm_timeout(&ll->alarm_wait_ack);
  if (timeout > zbncp_ll_alarm_timeout(&ll->alarm_reset))
  {
    timeout = zbncp_ll_alarm_timeout(&ll->alarm_reset);
  }
#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  if (timeout > zbncp_ll_alarm_timeout(&ll->alarm_sending))
  {
    timeout = zbncp_ll_alarm_timeout(&ll->alarm_sending);
  }
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
  q->res.timeout = timeout;

#ifdef ZBNCP_LL_TX_ENABLE_TIMEOUTS
  zbncp_ll_alarm_check(&ll->alarm_sending);
#endif /* ZBNCP_LL_TX_ENABLE_TIMEOUTS */
  zbncp_ll_alarm_check(&ll->alarm_wait_ack);
  zbncp_ll_alarm_check(&ll->alarm_reset);

  ZBNCP_DBG_TRACE("                   >>> timeout %zu%s [ack size %zu]",
    q->res.timeout, ((ll->service == ZBNCP_TRUE) ? "/SERVICE" : ""), ll->tx.ack.size);

  if (ll->service == ZBNCP_TRUE)
  {
    q->res.timeout = 0;
  }

  q->res.status = ll->status;
}
