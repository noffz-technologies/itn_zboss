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
/*  PURPOSE: NCP Host API declarations
*/
#ifndef NCP_HOST_API_H
#define NCP_HOST_API_H 1

#include "zb_common.h"
#include "ncp/ncp_common_api.h"

/** @cond DOXYGEN_NCP_SECTION */

/** @addtogroup ncp_host_api
  *  @{
  */

/** Predefined value, which is returned when TSN is reserved */
#define NCP_TSN_RESERVED 0xFFU

/**
 * Custom response callback
 *
 * @param param - the ZBOSS buffer with the payload received from SoC and @ref ncp_hl_custom_resp_t in param.
 *                It's freed afterwards.
*/
typedef void (*zb_ncp_custom_response_cb_t)(zb_uint8_t param);

/**
 * Custom indication callback
 *
 * @param - the ZBOSS buffer with the payload received from SoC. It's freed afterwards.
 */
typedef void (*zb_ncp_custom_indication_cb_t)(zb_uint8_t param);

/** @cond DOXYGEN_INTERNAL_DOC */
/**
 * Initializes NCP Host subsystem
 *
 * @note This function is used inside ZBOSS stack and should not be called from the application.
 */
void ncp_host_init(void);
/** @endcond */ /* DOXYGEN_INTERNAL_DOC */

/**
 * Get the firmware version read from SoC
 */
zb_uint32_t ncp_host_get_firmware_version(void);

/**
 * Runs the bootloader on SoC
 *
 * @return status of operation
 */
zb_ret_t ncp_host_ota_run_bootloader(void);

/**
 * Sends a custom request.
 *
 * @param param - The buffer filled with data to be sent. It's freed by the stack.
 * @param cb    - Called with the returned payload, if NCP SoC replies with the @ref zb_ncp_custom_response.
 *                Otherwise, only the status and TSN fields are valid and payload should not be read.
 *
 * @note Payload passed in the request is user-defined and can vary between requests.
 *       It's up to user to format the data in payload and handle requests on the SoC.
 *
 * @return tsn on success, @ref NCP_TSN_RESERVED otherwise
 */
zb_uint8_t ncp_host_custom_request(zb_uint8_t param, zb_ncp_custom_response_cb_t cb);

/**
 * Registers a callback for processing the custom indication.
 *
 * @param cb - the indication callback
 *
 * @note Custom indications are user-defined and can vary. It's up to user to handle them.
 *       The indication payload is passed as a buffer payload.
 *
 *       Unless callback is registered or callback argument is NULL, the custom indication will be ignored.
 */
void ncp_host_custom_register_ind_cb(zb_ncp_custom_indication_cb_t cb);

/**
 * This error is raised if current NCP protocol version of the Host does not match with the NCP FW
 * protocol version of the SoC. NCP protocol version compatibility is checked at the Host startup.
 */
#define ZB_ERROR_NCP_PROTOCOL_VERSION_MISMATCH 1

/** @} */ /* ncp_host_api */
/** @endcond */ /* DOXYGEN_NCP_SECTION */

#endif /* NCP_HOST_API_H */
