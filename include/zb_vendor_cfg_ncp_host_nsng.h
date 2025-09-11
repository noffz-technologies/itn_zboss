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
/* PURPOSE: ZBOSS stack configuration file for NCP nsng build
*/

#ifndef ZB_VENDOR_CFG_NCP_HOST_NSNG_H
#define ZB_VENDOR_CFG_NCP_HOST_NSNG_H 1

#include "zb_vendor_cfg_zoi_base.h"

#define NCP_MODE
#define NCP_MODE_HOST

#define ZB_HAVE_SERIAL
#define ZB_HAVE_ASYNC_SERIAL
#define ZB_NCP_TRANSPORT_TYPE_SERIAL
#define ZB_SERIAL_PTY_SLAVE
#define SERIAL_ENV_NAME "NCP_SLAVE_PTY"
#define SERIAL_DEFAULT_PATH "/tmp/ncp_slave_pty"

#ifndef NCP_PROTOCOL_VERSION
#define NCP_PROTOCOL_VERSION 0x0000
#endif /* NCP_PROTOCOL_VERSION */

#define NCP_STACK_VERSION (ZBOSS_MAJOR << 24u)
#define NCP_FW_VERSION 0x1234

#define ZB_INTERRUPT_SAFE_CALLBACKS
/* #define ZB_INTERRUPT_SAFE_ALARMS */

#define ZB_BDB_MODE
#define ZB_BDB_ENABLE_FINDING_BINDING
#define ZB_LITE_BDB_ONLY_COMMISSIONING

#endif /* ZB_VENDOR_CFG_NCP_HOST_NSNG_H */
