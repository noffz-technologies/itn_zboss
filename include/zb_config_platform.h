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
/* PURPOSE: Configuration file: configuration- and platform-specific definitions,
constants etc.
*/
#ifndef ZB_PLATFORM_CONFIG_H
#define ZB_PLATFORM_CONFIG_H 1

/* General Linux platform definitions */
#define UNIX
#define ZB_ENABLE_ZCL
#define ZB_LITTLE_ENDIAN
#define ZB_NEED_ALIGN
#ifndef ZB_PLATFORM_LINUX
#define ZB_PLATFORM_LINUX
#endif

#ifndef ZB_ZGPD_GPFS_SIZE
/* Don't waste traffic, but jow we can send more than one */
#define ZB_ZGPD_GPFS_SIZE 2
#endif
#ifndef ZB_ZGP_TRANSL_CMD_PLD_MAX_SIZE
#define ZB_ZGP_TRANSL_CMD_PLD_MAX_SIZE 3
#endif
#define GPDF_TX_DELAY_DECREASE 200

#ifndef ZB_CONFIGURABLE_MEM
#define ZB_CHILD_HASH_TABLE_SIZE (((ZB_IEEE_ADDR_TABLE_SIZE + ZB_IEEE_ADDR_TABLE_SIZE / 3) + 31) / 32 * 32)
#endif /* ZB_CONFIGURABLE_MEM */

#define MAC_TRANSPORT_USES_SELECT
#define ZB_MAC_PENDING_BIT_SOURCE_MATCHING
#define ZB_SOFT_SECURITY
#define ZB_DEBUG_ENLARGE_TIMEOUT 1

#ifndef ZB_CB_QUANT
#define ZB_CB_QUANT 2
#endif

#ifndef NCP_TRANSPORT_REFRESH_TIME
#define NCP_TRANSPORT_REFRESH_TIME (-1) /* Lock for an infinite amount of time on the transport select if no callbacks and alarms are scheduled. */
#endif /* NCP_TRANSPORT_REFRESH_TIME */

#ifdef ZB_CONFIG_LINUX_NSNG
#define ZB_NSNG

#define MAC_DUMP_FROM_INTR
#define ZB_DSR_MAC
#define ZB_MANUAL_ACK           /* means, MAC LL implements ACK rx and parse, main logic ertransmits */
#define ZB_SEND_BEACON_IMMEDIATELY
#define ZB_MAC_RX_QUEUE_CAP 5
#define ZB_MAC_SOFTWARE_PB_MATCHING

//#define ZB_CERTIFICATION_HACKS
/* If defined, MAC always sets Pending bit in MAC ACK, then sends empty frame.
   If not defined, MAC set Pending bit if either has pending data or device is
   not known (to send injected indirect LEAVE to it).
 */
//#define ZB_MAC_STICKY_PENDING_BIT

#define NS_MAX_N_DEV 32

#define NS_KEY "NS"
#define NS_KEY_DEFAULT "844"

#define NS_MAX_PKT_SIZE 2047

#define NS_FIRST_24_CHANNEL 11
#define NS_LAST_24_CHANNEL 26

#define NS_FIRST_PG28_CHANNEL 0
#define NS_LAST_PG28_CHANNEL 26

/* At page 29 channels: 27 to 34, then a hole, then 62 */
#define NS_FIRST_PG29_CHANNEL 27
#define NS_MID_PG29_CHANNEL 34
#define NS_LAST_PG29_CHANNEL 62

#define NS_FIRST_PG30_CHANNEL 0
#define NS_LAST_PG30_CHANNEL 61

#define NS_FIRST_PG31_CHANNEL 0
#define NS_LAST_PG31_CHANNEL 26

#define NS_MAX_CHANNELS_PER_PAGE 62
#define NS_PAGE_N 5

#define ZB_CONFIGURABLE_MAC_PIB

#define ZB_MANUAL_ADDR_FILTER

#endif  /* ZB_CONFIG_LINUX_NSNG */

#ifdef ZB_CONFIG_LINUX_MACSPLIT_HOST
  #define ZB_MACSPLIT
  #define ZB_MACSPLIT_HOST
  #define ZB_MACSPLIT_FW_UPGRADE
  #define MAC_AUTO_DELAY_IN_MAC_GP_SEND

  #if defined ZB_MAC_INTERFACE_SINGLE || !defined ZB_MAC_MONOLITHIC
    #define ZB_ALIEN_MAC /* TODO: remove this define completely, use ZB_COMPILE_MAC_MONOLITHIC instead */
  #endif /* ZB_MAC_INTERFACE_SINGLE || !ZB_MAC_MONOLITHIC */
#endif /* ZB_CONFIG_LINUX_MACSPLIT_HOST */

#endif  /* ZB_PLATFORM_CONFIG_H */
