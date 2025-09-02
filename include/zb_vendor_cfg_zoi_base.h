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
/* PURPOSE: ZBOSS stack basic configuration file
*/

#ifndef ZB_VENDOR_CFG_BASE_H
#define ZB_VENDOR_CFG_BASE_H 1

/* do we really need it for all builds? I guess, we need it for tests only */
#define ZB_LIMIT_VISIBILITY


/* Features enabled for all builds */
#define ZB_USE_SLEEP
#define APS_FRAGMENTATION
#define ZB_ALL_DEVICE_SUPPORT
#define ZB_PRODUCTION_CONFIG
#define ZB_SECURITY_INSTALLCODES
#define ZB_MAC_CONFIGURABLE_TX_POWER
#define ZB_APS_USER_PAYLOAD
#define ZB_USE_OSIF_OTA_ROUTINES
#define ZB_R22_MULTIMAC


/* configuration common for all builds */
#define ZB_RESET_AUTORESTART
#define ZB_REDUCE_NWK_LOAD_ON_LOW_MEMORY

#ifdef ZB_CONFIG_DEFAULT_KERNEL_DEFINITION

/* Default memory storage configuration - to be used if user does not include any of zb_mem_config_xxx.h */

#ifndef ZB_ED_ROLE
#define ZB_CONFIG_ROLE_ZC
#else
#define ZB_CONFIG_ROLE_ZED
#endif

#define ZB_CONFIG_OVERALL_NETWORK_SIZE 128

#define ZB_CONFIG_HIGH_TRAFFIC

#define ZB_CONFIG_APPLICATION_COMPLEX

#endif  /*  ZB_CONFIG_DEFAULT_KERNEL_DEFINITION */

/* Compile-time memory configuration: hard-coded parameters */
/* FIXME: 32 seems too big for zb_mlme_scan_confirm() - stack is allocating 5 + (5 *
 * ZB_PANID_TABLE_SIZE) in buf. So maximum possilbe ZB_PANID_TABLE_SIZE is 28 */
#define ZB_SCHEDULER_Q_SIZE 64
#define ZB_MAX_EP_NUMBER 6
#define ZB_IOBUF_POOL_SIZE 40
#define ZB_PANID_TABLE_SIZE 28
#define ZB_DEV_MANUFACTORER_TABLE_SIZE 32
#define ZB_BUF_Q_SIZE 8
#define ZDO_TRAN_TABLE_SIZE 16
#define ZB_APS_ENDPOINTS_IN_GROUP_TABLE 8
#define ZB_NWK_BTR_TABLE_SIZE 16
#define ZB_NWK_BRR_TABLE_SIZE 16
#ifndef ZB_ED_ROLE
#define ZB_APS_SRC_BINDING_TABLE_SIZE 16
#define ZB_APS_DST_BINDING_TABLE_SIZE 32
#define ZB_APS_GROUP_TABLE_SIZE 16
#else
#define ZB_APS_SRC_BINDING_TABLE_SIZE 8
#define ZB_APS_DST_BINDING_TABLE_SIZE 16
#define ZB_APS_GROUP_TABLE_SIZE 8
#endif  /* ZB_ED_ROLE */
#define ZB_ZGP_SINK_TBL_SIZE 24
#define ZB_ZGP_PROXY_TBL_SIZE 5
#define ZB_ZGP_TRANSL_CMD_PLD_MAX_SIZE 3


#ifdef DEBUG
#define ZB_DEBUG_BUFFERS
#define ZB_TRAFFIC_DUMP_ON
#define ZB_CHECK_OOM_STATUS

#if !defined(USE_ASSERT)
#define USE_ASSERT
#endif /* !USE_ASSERT */

#endif /* DEBUG */

/* Enable CLI interface for ZVD devices */
#define ZBD_CLI_ENABLED

#endif /* ZB_VENDOR_CFG_BASE_H */
