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
/* PURPOSE: Defines for NS (next gen). Set of defines and prototypes to use nsng
API as a base for MAC to run ZBOSS over NS.
*/
#ifndef ZB_NSNG_H
#define ZB_NSNG_H 1


/*! @cond internals_doc */
/*! \addtogroup ZB_MAC */
/*! @{ */

#ifdef ZB_NSNG

#include "ns_api.h"

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26

#include "zb_mac.h"
#include "zb_osif.h"


void zb_mac_transport_init();
void zb_nsng_io_iteration(zb_bool_t block);
zb_ret_t zb_nsng_send_data(zb_bufid_t buf, ns_api_tx_wait_t tx_wait_type);
zb_uint8_t zb_nsng_recv_data(zb_bufid_t buf);
void zb_nsng_set_tx_power(zb_int8_t tx_power);

#define ZB_NSNG_CTX() g_nsng_ctx

#define ZB_TRANSCEIVER_GET_RSSI(rssi_value) ns_api_get_rssi(&ZB_NSNG_CTX().api_ctx, (rssi_value))
#define ZB_TRANSCEIVER_START_GET_RSSI(_scan_duration_bi) ns_api_start_get_rssi(&ZB_NSNG_CTX().api_ctx)
#define ZB_TRANSCEIVER_GET_ENERGY_LEVEL(energy_level) ns_api_get_energy_level(&ZB_NSNG_CTX().api_ctx, (energy_level))

void zb_nsng_init(void);
void zb_nsng_deinit(void);

/**
   At transceiver init connect to NS
 */
#define ZB_TRANSCEIVER_INIT_RADIO()       zb_nsng_init()

#define ZB_TRANSCEIVER_DEINIT_RADIO()     zb_nsng_deinit()

#define ZB_GO_IDLE()


#define ZB_IS_TRANSPORT_BUSY() (ZB_NSNG_CTX().api_ctx.transport_busy + 0)

#if defined ZB_SUB_GHZ_LBT

#define MULTIMAC_CALL_NSNG(func_24, func_sub_ghz) \
  (MAC_PIB().phy_current_page ? (func_sub_ghz) : (func_24))

/**
   Synchronous data send.
   At HW the function returned when TX initiated, but not completed. At NS it
   returns when entire packet was written to the socket.

   @note Used hack to detect ACK: check hdr_len.
   TODO: change MAC to use separate macro to send ACK.
 */

/* Function aka shutup pedantic gcc:
 * Ternary operators don't allow to call void functions, return 0 and relax.
*/
#define CALL_VOID_WITH_RET(void_foo) ((void_foo), 0)

#define ZB_TRANS_SEND_FRAME(hdr_len, buf, wait_type) \
  MULTIMAC_CALL_NSNG(zb_nsng_send_data(buf, (ns_api_tx_wait_t)wait_type), CALL_VOID_WITH_RET(zb_mac_lbt_tx(hdr_len, buf, (ns_api_tx_wait_t)wait_type)))

#define ZB_TRANS_REPEAT_SEND_FRAME ZB_TRANS_SEND_FRAME

#define ZB_TRANS_SEND_FRAME_SUB_GHZ(hdr_len, buf, wait_type) \
  ((void)hdr_len, zb_nsng_send_data(buf, (ns_api_tx_wait_t)wait_type))

#define ZB_TRANS_SEND_ACK ZB_TRANS_SEND_FRAME_SUB_GHZ

#define ZB_TRANS_SEND_ACK_SUB_GHZ(dsn, frame_timestamp) zb_mac_send_ack(dsn), ZVUNUSED(frame_timestamp)

#else /* defined ZB_R22_MULTIMAC */
/**
   Synchronous data send.
   At HW the function returned when TX initiated, but not completed. At NS it
   returns when entire packet written to the socket.

   Note:used hack to detect ACK: check hdr_len.
   TODO: change MAC to use separate macro to send ACK.
 */
#define ZB_TRANS_SEND_FRAME(hdr_len, buf, wait_type) \
  zb_nsng_send_data(buf, (ns_api_tx_wait_t)wait_type)


#define ZB_TRANS_REPEAT_SEND_FRAME ZB_TRANS_SEND_FRAME

#define ZB_TRANS_SEND_ACK ZB_TRANS_SEND_FRAME

#endif

/**
   Change channel, update PIB
 */
#ifdef ZB_PAGES_REMAP_TO_2_4GHZ
#define ZB_TRANSCEIVER_SET_CHANNEL(page, channel_number) ns_api_set_channel(&ZB_NSNG_CTX().api_ctx, 0, ZB_PAGES_REMAP_LOGICAL_CHANNEL(page, channel_number))
#else
#define ZB_TRANSCEIVER_SET_CHANNEL(page, channel_number) ns_api_set_channel(&ZB_NSNG_CTX().api_ctx, page, channel_number)
#endif

#define ZB_TRANSCEIVER_SET_PROMISCUOUS(promiscuous_mode)

#define ZB_RADIO_INT_DISABLE() ZB_OSIF_GLOBAL_LOCK()
#define ZB_RADIO_INT_ENABLE()  ZB_OSIF_GLOBAL_UNLOCK()

/**
   Send to NS MAC address from PIB
 */
#define ZB_TRANSCEIVER_UPDATE_LONGMAC()            ns_api_set_mac(&ZB_NSNG_CTX().api_ctx, MAC_PIB().mac_extended_address);

#define ZB_TRANS_GET_TX_TIMESTAMP() osif_transceiver_time_get()

/* Do not really use interrupts */
/**
   Get transceiver interrupt - run i/o iteration w/o block
 */
//#define ZB_MAC_GET_TRANS_INT_FLAG() (ZB_MAC_GET_RX_INT_STATUS_BIT() || ZB_MAC_GET_TX_INT_STATUS_BIT())
#define ZB_MAC_GET_TRANS_INT_FLAG() 0

#define ZB_MAC_SET_TRANS_INT()

/**
   Clear transceiver interrupt flag - nothing (no interrupts in NS)
 */
#define ZB_MAC_CLEAR_TRANS_INT()
/**
   Read interrupt status register - nothing (no interrupts in NS)
 */
#define ZB_MAC_READ_INT_STATUS_REG()

/* RX status set when full data packet is received */

/**
   Set RX data status to 1 - indicate packet receive
 */
#define ZB_MAC_SET_RX_INT_STATUS_BIT()    ZB_NSNG_CTX().rx_int = 1

/**
   Clear RX data status
 */
#define ZB_MAC_CLEAR_RX_INT_STATUS_BIT()  ZB_NSNG_CTX().rx_int = 0
/**
   Get RX data status
 */
#define ZB_MAC_GET_RX_INT_STATUS_BIT()    (ZB_NSNG_CTX().rx_int + 0)

#define ZB_MAC_SET_TX_INT_STATUS_BIT()    ZB_NSNG_CTX().tx_int = 1

/* TX status set when full data packet is sent */
#define ZB_MAC_CLEAR_TX_INT_STATUS_BIT()  ZB_NSNG_CTX().tx_int = 0
#define ZB_MAC_GET_TX_INT_STATUS_BIT()    (ZB_NSNG_CTX().tx_int + 0)

/* The only possible channel error is channel access failure during CSMA/CA */
/**
   Return 1 if TX failed because of channel access error
 */
#define ZB_TRANS_CHECK_CHANNEL_BUSY_ERROR() (ZB_NSNG_CTX().api_ctx.tx_status == NS_TX_COLLISION)
#define ZB_TRANS_CHECK_TX_LBT_TO_ERROR() (ZB_NSNG_CTX().api_ctx.tx_status == ZB_TRANS_TX_LBT_TO)
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() 0
#define ZB_TRANS_CHECK_NO_ACK() 0

/* Empty (not used for NS) */
#define ZB_TRANSCEIVER_SET_COORD_EXT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_COORD_SHORT_ADDR(addr)
#define ZB_TRANSCEIVER_SET_PAN_ID(pan_id)
#define ZB_TRANSCEIVER_UPDATE_PAN_ID()
#define ZB_TRANSCEIVER_UPDATE_SHORT_ADDR()
#define ZB_MAC_TRANS_CLEAR_PENDING_BIT()
#define ZB_MAC_TRANS_SET_PENDING_BIT()
#define ZB_MAC_TRANS_PENDING_BIT()  ZB_TRUE
#define ZB_TRANS_CHECK_TX_RETRY_COUNT_EXCEEDED_ERROR() 0
#define ZB_TRANS_TX_CARRIER_DATA(channel, timeout_bi) ((void)channel) /* Start continuous transmission.
                                                             Used in TP_154_PHY24_TRANSMIT_02 test.*/

#define ZB_TRANSCEIVER_START_CHANNEL_NUMBER 11
#define ZB_TRANSCEIVER_MAX_CHANNEL_NUMBER   26

#define ZB_TRANS_CUT_SPECIFIC_HEADER(buf) /*!< nothing here  */

#define ZB_TRANSCEIVER_SET_BROADCAST_FILTERING(accept_br)

#define ZB_TRANSCEIVER_SET_RX_ON_OFF(_rx_on) ns_api_set_rx_on_off(&ZB_NSNG_CTX().api_ctx,(_rx_on))
#define ZB_TRANSCEIVER_GET_RX_ON_OFF(_rx_on) (ZB_NSNG_CTX().api_ctx.rx_on)

#define ZB_TRANSCEIVER_PERFORM_CCA() (PHY_IDLE)
#define ZB_TRANSCEIVER_GET_SYNC_RSSI() (ZB_TRANSCEIVER_PERFORM_CCA())

/* nsng MAC LL does auto-ack by itself, so no interface to transceiver here */
#define ZB_TRANSCEIVER_ENABLE_AUTO_ACK()

void osif_set_default_trasnmit_powers(zb_int8_t *tx_powers);
void zb_nsng_set_tx_power(zb_int8_t tx_power);
void zb_nsng_get_tx_power(zb_int8_t* tx_power);

#define ZB_TRANSCEIVER_SET_TX_POWER(new_power) zb_nsng_set_tx_power(new_power)
#define ZB_TRANSCEIVER_GET_TX_POWER(power) zb_nsng_get_tx_power(power)


#define ZB_TRANS_MAX_TX_POWER_SUB_GHZ 20 /* dBm */

#define ZB_TRANS_MIN_TX_POWER_SUB_GHZ -60 /* dBm */

/**
   Length of received ACK (+2 bytes lqi/rssi + 2 bytes fcs)
 */
#define ZB_MAC_RX_ACK_FRAME_LEN (ZB_MAC_ACK_FRAME_LEN + 2 + 2)

/**
   Transceiver i/o context

   NS-specific data
 */
typedef struct zb_nsng_ctx_s
{
  ns_api_handle_t api_ctx;      /*!< NS API context  */

  zb_uint8_t rx_int;            /*!< 1 to pass rx data up */
  zb_uint8_t tx_int;            /*!< 1 to pass tx status up */

  zb_uint8_t tx_buf;            /*!< Buffer just sent to nsng, no tx status yet  */
  zb_uint8_t pend_tx_buf;       /*!< tx buffer waiting for MAC ACK tx complete */
  zb_uint8_t pend_tx_wait_type; /*!< Wait type for pending tx buffer */

  zb_uint8_t zgpfs_count;
} zb_nsng_ctx_t;

extern zb_nsng_ctx_t g_nsng_ctx;

/*!< empty - just to compile  */
typedef struct zb_transceiver_ctx_s
{
  zb_uint8_t recv_buf_full;
#if 0
  zb_uint8_t dummy;
  zb_uint8_t tx_status;
  zb_uint8_t tx_in_progress;
#endif
} zb_transceiver_ctx_t;


#endif  /* ZB_NSNG */

/*! @} */
/*! @endcond */


#endif /* ZB_NSNG_H */
