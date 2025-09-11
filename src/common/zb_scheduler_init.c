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
/* PURPOSE: Zigbee scheduler: init
*/

/*! \addtogroup ZB_BASE */
/*! @{ */

#define ZB_TRACE_FILE_ID 125
#include "zb_common.h"


void zb_sched_init()
{
  zb_uint8_t i;

  ZB_POOLED_LIST8_INIT(ZG->sched.tm_freelist);
  ZB_POOLED_LIST8_INIT(ZG->sched.tm_queue);
  for (i = 0 ; i < ZB_SCHEDULER_Q_SIZE ; ++i)
  {
    ZB_POOLED_LIST8_INSERT_HEAD(ZG->sched.tm_buffer, ZG->sched.tm_freelist, next, i);
  }
}

zb_bool_t zb_scheduler_is_stop()
{
  if (ZB_OSIF_IS_EXIT())
  /*cstat !MISRAC2012-Rule-2.1_b */
  /** @mdr{00011,0} */
  {
    return ZB_TRUE;
  }

  if (ZG->sched.stop)
  {
    return ZB_TRUE;
  }
  return ZB_FALSE;
}

void zb_sched_stop()
{
  TRACE_MSG(TRACE_ERROR, ">> zb_sched_stop", (FMT__0));
  ZG->sched.stop = ZB_TRUE;
  ZG->sched.stopping = ZB_FALSE;
}



/*! @} */
