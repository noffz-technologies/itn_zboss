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
/* PURPOSE: Common service routines from zdo_app.c
   To be used at both NCP Host and Soc as well as in monolithic build.
*/

#define ZB_TRACE_FILE_ID 5
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_nwk_nib.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zdo_common.h"
#include "zb_secur.h"
#include "zb_secur_api.h"
#include "zb_nvram.h"
#include "zb_bdb_internal.h"
#include "zb_watchdog.h"
#include "zb_ncp.h"
#include "zdo_wwah_parent_classification.h"
#include "zb_zdo_globals.h"

#if defined ZB_ENABLE_ZLL
#include "zll/zb_zll_common.h"
#include "zll/zb_zll_nwk_features.h"
#endif /* defined ZB_ENABLE_ZLL */

/*! \addtogroup ZB_ZDO */
/*! @{ */



#ifndef ZB_ALIEN_SCHEDULER
void zdo_main_loop()
{
  while (!ZB_SCHEDULER_IS_STOP())
  {
    zb_sched_loop_iteration();
  }
}


void zcl_main_loop()
{
  zdo_main_loop();
}
#endif


/**
   Initiate ZBOSS start without zb_send_no_autostart_signal auto-start
 */
zb_ret_t zb_zdo_start_no_autostart()
{
  zb_ret_t ret;

  /* zb_zdo_dev_init always returns RET_OK, no need to check it */
  (void)zb_zdo_dev_init();
#ifdef ZB_MACSPLIT_HOST
  /* Flag is used for macsplit host only */
  ZG->zdo.handle.start_no_autostart = ZB_TRUE;
#endif

  ret = zb_buf_get_out_delayed(zb_send_no_autostart_signal);

  return ret;
}


void zb_app_signal_pack_with_data(zb_uint8_t param, zb_uint32_t signal_code, zb_int16_t status)
{
  zb_uint8_t *body;

  ZB_ASSERT(param);

  body = zb_buf_alloc_left(param, sizeof(signal_code));

  ZB_MEMCPY(body, &signal_code, sizeof(signal_code));
  zb_buf_set_status(param, (status == RET_OK ? RET_OK : RET_ERROR));
}

/**
   Send ZB_ZDO_SIGNAL_SKIP_STARTUP signal to zdo_startup_complete()
 */
void zb_send_no_autostart_signal(zb_uint8_t param)
{
  TRACE_MSG(TRACE_ZDO1, "zb_send_no_autostart_signal param %hd", (FMT__H, param));
  (void)zb_app_signal_pack(param, ZB_ZDO_SIGNAL_SKIP_STARTUP, RET_OK, 0);
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
}


void zb_send_leave_indication_signal(zb_uint8_t param, zb_ieee_addr_t device_addr, zb_uint8_t rejoin)
{
  zb_zdo_signal_leave_indication_params_t leave_ind_params;
  zb_zdo_signal_leave_indication_params_t *leave_ind_params_p;
  TRACE_MSG(TRACE_ZDO1, "zb_send_leave_indication_signal", (FMT__0));
  /* Get params from buf before zb_app_signal_pack() - it does initial_alloc. */
  leave_ind_params.rejoin = rejoin;
  ZB_IEEE_ADDR_COPY(leave_ind_params.device_addr, device_addr);
  leave_ind_params_p =
    (zb_zdo_signal_leave_indication_params_t *)zb_app_signal_pack(param,
                                                                 ZB_ZDO_SIGNAL_LEAVE_INDICATION,
                                                                 (zb_int16_t)zb_buf_get_status(param),
                                                                 (zb_uint8_t)sizeof(zb_zdo_signal_leave_indication_params_t));
  /* Put params to buf again */
  leave_ind_params_p->rejoin = leave_ind_params.rejoin;
  ZB_IEEE_ADDR_COPY(leave_ind_params_p->device_addr, leave_ind_params.device_addr);
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
}


/**
 * @brief Send @ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED signal
 *
 * @param param - reference to the buffer
 * @param long_addr - long address of the authorized device
 * @param short_addr - short address of the authorized device
 * @param authorization_type - authorization type (legacy, r21 TCLK)
 * @param authorization_status - authorization status (depends on authorization_type)
 */
void zb_send_device_authorized_signal(zb_uint8_t param,
                                      zb_ieee_addr_t long_addr,
                                      zb_uint16_t short_addr,
                                      zb_uint8_t authorization_type,
                                      zb_uint8_t authorization_status)
{
  zb_zdo_signal_device_authorized_params_t *params;

  TRACE_MSG(TRACE_ZDO1, "<> zb_send_device_authorized_signal param %hd", (FMT__H, param));
  TRACE_MSG(TRACE_ZDO1, "long_addr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(long_addr)));
  TRACE_MSG(TRACE_ZDO1, "short_addr 0x%x, auth_type 0x%hx, auth_status 0x%hx",
            (FMT__D_H_H, short_addr, authorization_type, authorization_status));

  params = (zb_zdo_signal_device_authorized_params_t*)
    zb_app_signal_pack(param,
                       ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED,
                       RET_OK,
                       (zb_uint8_t)sizeof(zb_zdo_signal_device_authorized_params_t));
  ZB_IEEE_ADDR_COPY(params->long_addr, long_addr);
  params->short_addr = short_addr;
  params->authorization_type = authorization_type;
  params->authorization_status = authorization_status;
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
}


/**
 * @brief Send @ZB_ZDO_SIGNAL_DEVICE_UPDATE signal
 *
 * @param param - reference to the buffer
 * @param long_addr - long address of the updated device
 * @param short_addr - short address of the updated device
 * @param status - the updated status of the device
 *        (see r21 spec, Table 4.14 APSME-UPDATE-DEVICE.request Parameters)
 */
void zb_send_device_update_signal(zb_uint8_t param,
                                  zb_ieee_addr_t long_addr,
                                  zb_uint16_t short_addr,
                                  zb_uint8_t status,
                                  zb_uint16_t parent_short,
                                  zb_uint8_t action)
{
  zb_zdo_signal_device_update_params_t *params;

  TRACE_MSG(TRACE_ZDO1, "<> zb_send_device_update_signal, param %hd", (FMT__H, param));
  TRACE_MSG(TRACE_ZDO1, "long_addr " TRACE_FORMAT_64, (FMT__A, TRACE_ARG_64(long_addr)));
  TRACE_MSG(TRACE_ZDO1, "short_addr 0x%x, status 0x%hx", (FMT__D_H, short_addr, status));

  params = (zb_zdo_signal_device_update_params_t*)
    zb_app_signal_pack(param,
                       ZB_ZDO_SIGNAL_DEVICE_UPDATE,
                       RET_OK,
                       (zb_uint8_t)sizeof(zb_zdo_signal_device_update_params_t));
  ZB_IEEE_ADDR_COPY(params->long_addr, long_addr);
  params->short_addr = short_addr;
  params->status = status;
  params->parent_short = parent_short;
  params->tc_action = action;
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
}


#if defined ZB_ROUTER_ROLE && !defined ZB_COORDINATOR_ONLY
/**
   Send ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT signal to zdo_startup_complete()
 */
void zb_send_no_active_links_left_signal(zb_uint8_t param)
{
  TRACE_MSG(TRACE_ZDO1, "Send ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT signal", (FMT__0));
  (void)zb_app_signal_pack(param, ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT, RET_OK, 0);
  ZB_SCHEDULE_CALLBACK(zb_zdo_startup_complete, param);
}
#endif /* ZB_ROUTER_ROLE && !ZB_COORDINATOR_ONLY */


/*! @} */
