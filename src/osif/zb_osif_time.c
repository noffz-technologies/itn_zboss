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
/* PURPOSE: time- and sleep-related functions
*/

#define ZB_TRACE_FILE_ID 30013
#include "zb_common.h"

#if defined ZB_CONFIG_LINUX_MIPS_GP && defined ZB_GP_MAC
#include "gphal.h"
#endif

static zb_uint64_t s_time_prev_us;

zb_uint32_t osif_get_time_ms()
{
  struct timeval tmv;
  gettimeofday(&tmv, NULL);
  return tmv.tv_sec * 1000LL + tmv.tv_usec / 1000LL;
}


void osif_gettime_us(zb_uint64_t *time_us)
{
  struct timeval t;

  gettimeofday(&t, NULL);
  *time_us = t.tv_sec * 1000000ll + t.tv_usec;
}


void osif_get_clock_monotonic_raw(struct timespec *t)
{
  memset(t, 0, sizeof(struct timespec));
  clock_gettime(CLOCK_MONOTONIC_RAW, t);
}


void osif_gettime_monotonic_us(zb_uint64_t *time_us)
{
  struct timespec time;

  osif_get_clock_monotonic_raw(&time);

  *time_us = time.tv_sec * 1000000LL;
  *time_us += time.tv_nsec / 1000LL;
}


void osif_get_clock_realtime(struct timespec *t)
{
  memset(t, 0, sizeof(struct timespec));
  clock_gettime(CLOCK_REALTIME, t);
}


zb_uint32_t osif_timespec_to_seconds(struct timespec *t)
{
  return t->tv_sec;
}


void osif_seconds_to_timespec(zb_uint32_t s, struct timespec *t)
{
  memset(t, 0, sizeof(struct timespec));
  t->tv_sec = s;
}


zb_uint32_t osif_get_clock_monotonic_sec()
{
  struct timespec t;

  osif_get_clock_monotonic_raw(&t);
  return osif_timespec_to_seconds(&t);
}


zb_uint32_t osif_get_clock_realtime_sec()
{
  struct timespec t;

  osif_get_clock_realtime(&t);
  return osif_timespec_to_seconds(&t);
}


int osif_timespec_compare(struct timespec *t1, struct timespec *t2)
{
  if (t1->tv_sec > t2->tv_sec || (t1->tv_sec == t2->tv_sec && t1->tv_nsec > t2->tv_nsec))
  {
    return 1;
  }
  if (t1->tv_sec == t2->tv_sec && t1->tv_nsec == t2->tv_nsec)
  {
    return 0;
  }
  return -1;
}


void osif_timespec_add_sec(struct timespec *t, zb_uint32_t sec)
{
  t->tv_sec += sec;
}

#define MS_ONE_SECOND 1000LL
#define US_ONE_SECOND 1000000LL
#define NS_ONE_SECOND 1000000000LL

void osif_timespec_add_ms(struct timespec *t, zb_uint32_t ms)
{
  zb_uint32_t nsec;

  nsec = t->tv_nsec + (ms % MS_ONE_SECOND) * US_ONE_SECOND;
  t->tv_sec += ms / MS_ONE_SECOND + nsec / NS_ONE_SECOND;
  t->tv_nsec = nsec % NS_ONE_SECOND;
}


void osif_timespec_add_us(struct timespec *t, zb_uint32_t us)
{
  zb_uint32_t nsec;

  nsec = t->tv_nsec + (us % US_ONE_SECOND) * MS_ONE_SECOND;
  t->tv_sec += us / US_ONE_SECOND + nsec / NS_ONE_SECOND;
  t->tv_nsec = nsec % NS_ONE_SECOND;
}


void osif_timespec_add_ns(struct timespec *t, zb_uint32_t ns)
{
  zb_uint32_t nsec;

  nsec = t->tv_nsec + (ns % NS_ONE_SECOND);
  t->tv_sec += ns / NS_ONE_SECOND + nsec / NS_ONE_SECOND;
  t->tv_nsec = nsec % NS_ONE_SECOND;
}

/* 1 BE = 15.360 ms */
#define BE_IN_US 15360LL
static struct timespec initial_ts2be = {0};
zb_uint32_t osif_current_time_to_be()
{
  struct timespec t;
  zb_uint32_t res;

  osif_get_clock_monotonic_raw(&t);

  if (initial_ts2be.tv_sec == 0 &&
      initial_ts2be.tv_nsec == 0)
  {
    initial_ts2be.tv_sec = t.tv_sec;
    initial_ts2be.tv_nsec = t.tv_nsec;
  }
  res = ((t.tv_sec - initial_ts2be.tv_sec) * US_ONE_SECOND) / BE_IN_US;
  res += ((t.tv_nsec - initial_ts2be.tv_nsec) / 1000LL) / BE_IN_US;
  return res;
}


zb_uint32_t zb_get_utc_time()
{
  time_t t = time(NULL);
  return (zb_uint32_t)t;
}

#if defined ZB_CONFIG_LINUX_MIPS_GP && defined ZB_GP_MAC

zb_time_t osif_transceiver_time_get()
{
  zb_time_t gp_time;

  gpHal_GetTime((UInt32*)&gp_time);

  return gp_time;
}


void osif_sleep_using_transc_timer(zb_time_t interval)
{
  zb_time_t t0 = osif_transceiver_time_get();
  zb_time_t t;

  do
  {
    t = osif_transceiver_time_get();
  }
  while (ZB_TIME_SUBTRACT(t, t0) < interval);
}

#else

zb_time_t osif_transceiver_time_get()
{
  struct timeval tmv;

  gettimeofday(&tmv, NULL);
  return (tmv.tv_sec * (1000000LL) + tmv.tv_usec);
}

zb_uint64_t osif_transceiver_time_get_long()
{
  struct timeval tmv;

  gettimeofday(&tmv, NULL);
  return (tmv.tv_sec * (1000000LL) + tmv.tv_usec);
}

void osif_sleep_using_transc_timer(zb_time_t t)
{
  usleep(t);
}
#endif

void osif_usleep(zb_uint_t us)
{
  usleep(us);
}


void zb_osif_wait_ms(zb_uint16_t ms)
{
  while (ms--)
  {
    osif_usleep(1000);
  }
}


void zb_osif_init_timer(void)
{
  osif_gettime_monotonic_us(&s_time_prev_us);
}


/**
   Move internal stack time taking rounding errors into account
 */
void zb_osif_update_timer(void)
{
  zb_uint64_t curr_time_us;
  zb_time_t bi_diff;

  /* Get time in us, calculate internal ZBOSS time. */
  osif_gettime_monotonic_us(&curr_time_us);
  bi_diff = (curr_time_us - s_time_prev_us) / ZB_BEACON_INTERVAL_USEC;

  s_time_prev_us += bi_diff * ZB_BEACON_INTERVAL_USEC;

  ZB_TIMER_CTX().timer += bi_diff;

  TRACE_MSG(TRACE_OSIF4, "zb_osif_update_timer: timer %d started %d", (FMT__D_D, ZB_TIMER_CTX().timer, ZB_TIMER_CTX().started));
}
