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
/* PURPOSE: Network Simulator (next gen). Client-server protocol and client's API.
NS initially developed for Zigbee, but that API tries to abstract from Zigbee.
*/
#ifndef NS_API_H
#define NS_API_H 1

/*! \addtogroup ZB_NS_API_INTERNAL */
/*! @{ */

/**
   Time for sync between NS and devices: usec. Not sure it is necessary..
 */
typedef zb_uint64_t ns_time_t;

/**
   NS<->client communication packet header
 */
typedef struct ns_pkt_hdr_s
{
  zb_uint16_t pkt_size;         /*!< packet size, including header and that
                                 * field, little endian  */
  zb_uint8_t  pkt_type;         /*!< Packet type @ref ns_pkt_type_t  */
  zb_uint8_t  status;           /*!< Status code @ref ns_status_code_t  */
  zb_uint32_t cmd_arg;          /*!< argument for command, status etc */
  zb_uint8_t  command;          /*!< command code for control packet */
  zb_uint8_t  cmd_arg2;          /*!< second argument for command (channel page etc) */
  zb_uint8_t  reserved[1];
  zb_uint64_t time_us;          /*!< For Client-NS, TX time, us. For NS-Client,
                                 * time sync. Little endian */
} ZB_PACKED_STRUCT ns_pkt_hdr_t;

/**
   Data packet trailer: lqi &rssi
 */
typedef struct ns_data_pkt_trailer_s
{
  zb_uint8_t  lqi;
  zb_uint8_t  rssi;
} ZB_PACKED_STRUCT ns_data_pkt_trailer_t;


/**
   Union to easier access to the NS<->client communication packet components
 */
typedef union ns_packet_u
{
  zb_uint16_t    size;
  ns_pkt_hdr_t   hdr;
  zb_char_t      all[NS_MAX_PKT_SIZE];
  struct ns_pkt_comps_s
  {
    ns_pkt_hdr_t hdr;
    zb_char_t    body[1];
  } ZB_PACKED_STRUCT pkt;
} ZB_PACKED_STRUCT ns_packet_t;


/**
   NS<->Client packet type
 */
typedef enum ns_pkt_type_e
{
  NS_PKT_DATA  = 0,             /*!< raw data in both directions */
  NS_PKT_CONTROL,               /*!< control messages (like MAC set) MAC - NS */
  NS_PKT_CONTROL_STATUS,        /*!< control messages (like MAC set) NS - MAC */
  NS_PKT_TX_STATUS,             /*!< TX status NS - MAC */
  NS_PKT_TTY,                   /*!< debug commands etc */
  NS_PKT_CONNECT_IND,           /*!< Connect indication */
} ns_pkt_type_t;


/**
   Commands for control packets
 */
typedef enum ns_pkt_control_cmd_e
{
  NS_CTL_SET_CHANNEL,
  NS_CTL_SET_MAC,
  NS_CTL_GET_RSSI,
  NS_CTL_START_GET_RSSI,
  NS_CTL_GET_ENERGY_LEVEL,
  NS_CTL_GET_CHANNEL_BUSY,
  NS_CTL_SET_RX_ON,
  NS_CTL_SET_RX_OFF,
  NS_CTL_SUBSCRIBE_TO_CONNECTS,
  NS_CTL_SERIAL_ON,
} ns_pkt_control_cmd_t;


typedef enum ns_pkt_tty_cmd_e
{
  NS_TTY_DATA,
} ns_pkt_tty_cmd_t;

/**
   NS status codes
 */
typedef enum ns_status_code_e
{
  NS_OK,
  NS_ERROR,
  NS_TX_COLLISION,
} ns_status_code_t;


/**
   Trailed which is added to data packets
 */
typedef struct ns_api_data_trailer_s
{
  zb_uint8_t  lqi;
  zb_uint8_t  rssi;
} ZB_PACKED_STRUCT ns_api_data_trailer_t;


/**
   King of negotiation before TX
 */
typedef enum ns_api_tx_wait_e
{
  NS_TX_WAIT_ACK,
  NS_TX_WAIT_CSMACA,
  NS_TX_WAIT_ZGP,
  ZB_TX_WAIT_NONE,
} ns_api_tx_wait_t;


struct ns_api_ctx_s;
typedef struct ns_api_ctx_s ns_api_handle_t;

/**
   Callback to be called to notify application about incoming emulated serial traffic.

   @param h - NS API handle
   @param src_device_i - slot # of device which sent this data over serial
   @param buf - data buffer
   @param len - data length
 */
typedef void (*ns_api_serial_rx_cb_t)(ns_api_handle_t *h, zb_uint_t src_device_i, zb_uint8_t *buf, zb_uint_t len);


/**
   Callback to be called to notify application about incoming emulated serial traffic.

   @param h - NS API handle
   @param src_device_i - slot # of device which sent this data over serial
*/
typedef void (*ns_api_connect_cb_t)(ns_api_handle_t *h, zb_int_t socket_n, zb_uint8_t is_connect);

#define MAX_TTY_MSG 255
ZB_RING_BUFFER_DECLARE(zb_nsng_tty_rb, zb_uint8_t, MAX_TTY_MSG);

/**
   NS API context data structure.

   To be allocated by the code using NS API.
 */
struct ns_api_ctx_s
{
  osif_ipc_handle_t h;          /*!< os-dependent IPC handler (socket fd in Unix)  */

  ns_packet_t rx_pkt;           /*!< just received packet  */

  zb_uint_t  pkt_cnt;           /*!< statistic: in/out packets counter  */
  zb_uint8_t rx_done;           /*!< 1 if data rx done  */
  zb_uint8_t tx_done;           /*!< 1 if data tx done and response received
                                 * from NS  */
  zb_uint8_t tx_status;         /*!< TX status if tx is done  */
  zb_uint8_t transport_busy;    /*!< 1 if data sent to nsng but got no status
                                 * yet - can't send more now */
  zb_uint8_t rx_on;             /*!< Status of transceiver  */
  ns_api_serial_rx_cb_t serial_rx_cb;
  ns_api_connect_cb_t connect_cb;
  zb_callback_t serial_byte_rx_cb;
  zb_nsng_tty_rb_t tty_rb;
  zb_uint_t pending_tty_i;
};

typedef zb_callback_t zb_osif_uart_byte_received_cb_t;


/*! @} */

/*! \addtogroup ZB_NS_API_INTERNAL */
/*! @{ */

/**
   Generate IPC object name to be used by both MAC and NS.

   TODO: make it OS-dependent?
 */
#define NS_CONSTRUCT_IPC_NAME(name) snprintf(name, sizeof(name), "NS-%s-%d", osif_getenv(NS_KEY, NS_KEY_DEFAULT), osif_getuid())


/**
   Initialize NS client API, connect to NS, assign NS API handler.

   @h_p - NS API handle.

   @return 0 if ok, negative value in case of error (return code = -errno error code in UNIX).
 */
zb_ret_t ns_api_init(ns_api_handle_t *h);

/**
   Deinit NS client API

   @param h - NS API handle.
*/
void ns_api_deinit(ns_api_handle_t *h);


/**
   Return ZB_TRUE if NS APIU is already initialized.
 */
zb_bool_t ns_api_inited(ns_api_handle_t *h);

/**
   Reset NS client API: reconnect to NS

   @h_p - NS API handle.

   @return 0 if ok, negative value in case of error (return code = -errno error code in UNIX).
 */
zb_ret_t ns_api_reset(ns_api_handle_t *h);

/**
   Reset ZB related data without disconnect from nsng

   @h_p - NS API handle.
 */
void ns_api_zb_reset(ns_api_handle_t *h);

/**
   Switch on/off receiver

   @param h - NS API handle
   @param receiver_on - if ZB_TRUE, switch rx on, else switch it off
   @param wait_us - if not 0 and receiver_on is ZB_TRUE, wait for wait_us before
   switch rx on

   @return 0 if ok, else error code
 */
zb_int_t ns_api_switch_receiver(ns_api_handle_t *h, zb_bool_t receiver_on, zb_uint_t wait_us);


/**
   Wait for any event from NS.

   If RX ready event signalled, do actual read.
   If that is data packet, set h->rx_done to 1.
   If that is TX status, set h->tx_done and fill h->tx_status.

   @param h - NS API handle
   @param wait_us - max time to block waiting for event, us. If 0, poll NS once
   and do not block.

   @return 0 if got some event which must be analyzed, else RET_BLOCKED or error code
 */
zb_ret_t ns_api_wait(ns_api_handle_t *h, zb_uint_t wait_us);

/**
   Send data frame to NS.

   NS gets data packet, wait for tx_time_us and send TX status response to the application.

   @param h - NS API handle
   @param data - data buffer
   @param data_len - data length
   @param tx_time_us - transmit time in us which NS must wait before
   distributing packet.

   @return 0 if ok, else error code (errno value)
 */
zb_ret_t ns_api_send_data(ns_api_handle_t *h, zb_uint8_t *data, zb_uint_t data_len, zb_uint_t tx_time_us, ns_api_tx_wait_t tx_wait_type);

/**
   Get data received from NS.

   Data is really already received and stored in the internal buffer.

   @param h - NS API handler
   @param buf - data buffer
   @param buf_len - buffer length

   @return length of returned data, including trailer of @see ns_api_data_trailer_t
 */
zb_uint_t ns_api_recv_data(ns_api_handle_t *h, zb_uint8_t *buf, zb_uint_t buf_len);

/**
   Drop the packet received from NS.

   @param h - NS API handler
 */
void ns_api_drop_packet(ns_api_handle_t *h);

/**
   Set working channel

   @param h - NS API handle
   @param channel - channel number.

   @return 0 if ok, else error code
 */
zb_int_t ns_api_set_channel(ns_api_handle_t *h, zb_uint8_t page, zb_uint8_t channel);


/**
   Set rx_on value for the device

   @param h - NS API handle
   @param rx_on - whether to enable or disable radio.

   @return 0 if ok, else error code
 */
zb_int_t ns_api_set_rx_on_off(ns_api_handle_t *h, zb_uint8_t rx_on);


/**
   Set MAC address

   @param h - NS API handle
   @param mac - mac address

   @return 0 if ok, else error code
 */
zb_int_t ns_api_set_mac(ns_api_handle_t *h, zb_ieee_addr_t mac);


/**
   Get RSSI

   @param h - NS API handle
   @param rssi_value_p - placeholder for RSSI value

   @return 0 if ok, else error code
 */
zb_ret_t ns_api_get_rssi(ns_api_handle_t *h, zb_uint8_t *rssi_value_p);


/**
   Get channel busy state, optionally waiting for timeout_us us.

   @param h - NS API handle
   @param is_busy_p - placeholder for channel busy flag
   @param timeout_ms - time to wait. No wait if 0.

   @return 0 if ok, else error code
 */
zb_ret_t ns_api_get_channel_busy(ns_api_handle_t *h, zb_bool_t rx_on_after_done, zb_bool_t *is_busy_p, zb_uint_t timeout_us);

/**
   Start Get RSSI - reset last received RSSI to zero

   @param h - NS API handle

   @return 0 if ok, else error code
 */
zb_ret_t ns_api_start_get_rssi(ns_api_handle_t *h);

/**
   Get energy level obtained from ED scan.

   @param h - NS API handle
   @param energy_level_p - placeholder for Energy level

   @return 0 if ok, else error code
 */
zb_ret_t ns_api_get_energy_level(ns_api_handle_t *h, zb_uint8_t *energy_level_p);

/*
  Routines for test runner
 */

/**
   Set ns serial rx callback

   @param h - NS API handle
   @param cb - callback to be called when data arrived.

   @return 0 if ok, else error code
 */
zb_int_t ns_api_set_serial_callback(ns_api_handle_t *h, ns_api_serial_rx_cb_t cb);


/**
   Transmit data over nsng serial emulator

   @param h - NS API handle
   @param dst_device_i - slot # of device to which to send. 0 - test run controller.
   @param buf - data buffer
   @param len - data length
 */
void ns_api_serial_tx(ns_api_handle_t *h, zb_uint_t dst_device_i, const zb_uint8_t *buf, zb_uint_t len);

/**
   Set ns serial rx callback

   @param h - NS API handle
   @param cb - callback to be called device connected or disconnected to nsng.
 */
zb_int_t ns_api_subscribe_to_connects(ns_api_handle_t *h, ns_api_connect_cb_t cb);


/**
   Read length, then header rest and packet body.

   Simplify things by using blocked rx

   @param h - address of NS API handle
*/
void ns_api_rx(ns_api_handle_t *h);


/**
   Get handle which could be used in select/poll calls.

   @param h - address of NS API handle
*/
osif_ipc_handle_t ns_api_get_waitable_handle(ns_api_handle_t *h);

/*! @} */

#endif /* NS_API_H */
