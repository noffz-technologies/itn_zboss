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
/*  PURPOSE: osif threads
*/

#define ZB_TRACE_FILE_ID 30008
#include "zboss_api_core.h"
#include <pthread.h>
#include <errno.h>

#if defined UNIX && defined ZB_THREADS

static osif_mutex_t s_global_mutex = PTHREAD_MUTEX_INITIALIZER;

void zb_osif_global_lock()
{
  pthread_mutex_lock(&s_global_mutex);
}


void zb_osif_global_unlock()
{
  pthread_mutex_unlock(&s_global_mutex);
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_init_mutex
 * PURPOSE:
 ****************************************************************/
int
osif_init_mutex(
  osif_mutex_t *mutex)
{
  int ret = pthread_mutex_init(mutex,
                               NULL);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_deinit_mutex
 * PURPOSE:
 ****************************************************************/
int
osif_deinit_mutex(
  osif_mutex_t *mutex)
{
  int ret = pthread_mutex_destroy(mutex);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_lock_mutex
 * PURPOSE:
 ****************************************************************/
int
osif_lock_mutex(
  osif_mutex_t *mutex)
{
  int ret = pthread_mutex_lock(mutex);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


int
osif_trylock_mutex(
  osif_mutex_t *mutex)
{
  int ret = pthread_mutex_trylock(mutex);
  if (ret == EBUSY)
  {
    ret = RET_BUSY;
  }
  else if (ret)
  {
    ret = SYSTEM_ERROR_CODE(ret);
  }
  return ret;
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_unlock_mutex
 * PURPOSE:
 ****************************************************************/
int
osif_unlock_mutex(
  osif_mutex_t *mutex)
{
#if 0
  fprintf(stderr, "Unlock %x by %d\n", (int)mutex, thread_get_id());
#endif
  int ret = pthread_mutex_unlock(mutex);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


#if 0
/*****************************************************************
 *
 * FUNCTION NAME:  init_thread_attr
 * PURPOSE:
 ****************************************************************/
static void
init_thread_attr(
  pthread_attr_t *attr)
{
  int ret;
  int i = 0;
  size_t size = OSIF_OPTIMAL_THREAD_STACK_SIZE;
  pthread_attr_init(attr);
#ifdef PTHREAD_STACK_MIN
  if (PTHREAD_STACK_MIN > OSIF_OPTIMAL_THREAD_STACK_SIZE)
  {
    size = PTHREAD_STACK_MIN;
  }
#endif
  do
  {
    ret = pthread_attr_setstacksize(attr,
                                    size);
  }
  while (ret != 0 && (i++ < 10) && (size *= 2));
  if (ret != 0)
  {
#ifdef PTHREAD_STACK_MIN
    ret = pthread_attr_setstacksize(attr,
                                    PTHREAD_STACK_MIN);
#endif
  }
}
#endif

/*****************************************************************
 *
 * FUNCTION NAME:  osif_start
 * PURPOSE:
 ****************************************************************/
int
osif_start_thread(
  osif_thread_t *thread,
  void * (*func)(void *),
  void * arg)
{
  int ret = 0;
  pthread_attr_t attr;

  *thread = 0;
  pthread_attr_init(&attr);
  ret = pthread_create(thread,
                       &attr,
                       func,
                       arg);
  pthread_attr_destroy(&attr);

  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_start_thread_detached
 * PURPOSE:
 ****************************************************************/
int
osif_start_thread_detached(
  osif_func_t func,
  void * arg)
{
  osif_thread_t thread;
  int ret = 0;
  pthread_attr_t attr;

  pthread_attr_setdetachstate(&attr,
                              1);
  ret = pthread_create(&thread,
                       &attr,
                       func,
                       arg);
  pthread_attr_destroy(&attr);

  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_get_id
 * PURPOSE:
 ****************************************************************/
zb_uint_t
osif_get_thread_id()
{
  return (zb_uint_t)pthread_self();
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_wait_thread
 * PURPOSE:
 ****************************************************************/
void *
osif_thread_join(
  osif_thread_t *thread)
{
  void *tr_ret;
  pthread_join(*thread,
               &tr_ret);
  *thread = 0;
  return tr_ret;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_close_handle
 * PURPOSE:
 ****************************************************************/
int
osif_detach_thread(
    osif_thread_t *handle )
{
  /* Closing thread handle in Win32 means putting thread into detached
   * state. */
  int ret = pthread_detach(*handle);
  *handle = 0;
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_init_cond
 * PURPOSE:
 ****************************************************************/
int osif_init_cond(osif_cond_t *cond)
{
  int ret = pthread_cond_init(cond, NULL);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_init_cond_with_attr
 * PURPOSE:
 ****************************************************************/
int osif_init_cond_with_attr(osif_cond_t *cond, osif_condattr_t *condattr)
{
  int ret = pthread_cond_init(cond, condattr);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_deinit_cond
 * PURPOSE:
 ****************************************************************/
int osif_deinit_cond(osif_cond_t *cond)
{
  int ret = pthread_cond_destroy(cond);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_init_condattr
 * PURPOSE:
 ****************************************************************/
int osif_init_condattr(osif_condattr_t *condattr)
{
  int ret = pthread_condattr_init(condattr);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_deinit_condattr
 * PURPOSE:
 ****************************************************************/
int osif_deinit_condattr(osif_condattr_t *condattr)
{
  int ret = pthread_condattr_destroy(condattr);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_init_condattr
 * PURPOSE:
 ****************************************************************/
int osif_condattr_setclock(osif_condattr_t *condattr, zb_uint8_t clockid)
{
  int ret = pthread_condattr_setclock(condattr, clockid);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_cond_wait
 * PURPOSE:
 ****************************************************************/
int osif_cond_wait(osif_cond_t *cond, osif_mutex_t *mutex)
{
  int ret = pthread_cond_wait(cond, mutex);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


/*
 * Force scheduler to switch to another thread.
 */
void
osif_sched_yield()
{
  sched_yield();
}


/*****************************************************************
 *
 * FUNCTION NAME:  osif_cond_wait with target monotonic timespec
 * PURPOSE:
 ****************************************************************/
int osif_cond_wait_with_target(osif_cond_t *cond, osif_mutex_t *mutex, struct timespec *target)
{
  int ret;

  do
  {
    ret = pthread_cond_timedwait(cond, mutex, target);

    if (ret == EINTR)
    {
      struct timespec t;

      osif_get_clock_monotonic_raw(&t);

      if (osif_timespec_compare(&t, target) >= 0)
      {
        break;
      }
    }
  }
  while (ret == EINTR);

  switch(ret)
  {
    case 0:
    {
      return RET_OK;
    }
    case EINTR:
    case ETIMEDOUT:
    {
      return RET_TIMEOUT;
    }
    case EBUSY:
    {
      return RET_ERROR;
    }
    default:
      return SYSTEM_ERROR_CODE(ret);
  }
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_cond_wait
 * PURPOSE:
 ****************************************************************/
int osif_cond_wait_with_timeout(osif_cond_t *cond, osif_mutex_t *mutex, unsigned wait_interval_ms)
{
  struct timeval now;
  struct timespec timeout;
  zb_uint64_t t;                /* time in usec */
  zb_uint64_t wait_interval_us;
  int ret;

  gettimeofday(&now, NULL);

  wait_interval_us = wait_interval_ms * (zb_uint64_t)1000;
  t = now.tv_sec * (zb_uint64_t)1000000000 + now.tv_usec * (zb_uint64_t)1000 + wait_interval_us * (zb_uint64_t)1000;

  timeout.tv_sec = t / (zb_uint64_t)1000000000;
  timeout.tv_nsec = t % (zb_uint64_t)1000000000;
  do
  {
    ret = pthread_cond_timedwait(cond, mutex, &timeout);
    if(ret == EINTR)
    {
      struct timeval cur;
      gettimeofday(&cur, NULL);
      if ((cur.tv_sec - now.tv_sec) * (zb_uint64_t)1000000 + (cur.tv_usec - now.tv_usec) >= wait_interval_us)
      {
        break;
      }
    }
  }
  while (ret == EINTR);

  switch(ret)
  {
    case 0:
    {
      return RET_OK;
    }
    case EINTR:
    case ETIMEDOUT:
    {
      return RET_TIMEOUT;
    }
    case EBUSY:
    {
      return RET_ERROR;
    }
    default:
      return SYSTEM_ERROR_CODE(ret);
  }
}

/*****************************************************************
 *
 * FUNCTION NAME:  osif_cond_signal
 * PURPOSE:
 ****************************************************************/
int osif_cond_signal(osif_cond_t *cond)
{
  int ret = pthread_cond_signal(cond);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}


int osif_cond_broadcast(osif_cond_t *cond)
{
  int ret = pthread_cond_broadcast(cond);
  return ret ? SYSTEM_ERROR_CODE(ret) : 0;
}

#if 0

/*****************************************************************
 *
 * Equivalents for Win32 InterlockedXXX funxtions
 *
 *****************************************************************/

static OSIF_DECLARE_INITED_MUTEX(s_interlocked_mutex);

osif_interlocked_t
osif_interlocked_decrement(
  osif_interlocked_t *v)
{
  osif_interlocked_t ret;
  osif_lock_mutex(&s_interlocked_mutex);
  ret = --(*v);
  osif_unlock_mutex(&s_interlocked_mutex);
  return ret;
}

osif_interlocked_t
osif_interlocked_increment(
  osif_interlocked_t *v)
{
  osif_interlocked_t ret;
  osif_lock_mutex(&s_interlocked_mutex);
  ret = ++(*v);
  osif_unlock_mutex(&s_interlocked_mutex);
  return ret;
}


osif_interlocked_t
osif_interlocked_exchange(
  osif_interlocked_t *target,
  osif_interlocked_t value)
{
  osif_interlocked_t ret;
  osif_lock_mutex(&s_interlocked_mutex);

  ret = *target;
  *target = value;

  osif_unlock_mutex(&s_interlocked_mutex);
  return ret;
}

/*
 * POSIX semaphores
 */

#ifndef ZB_NO_POSIX_SEMAPHORES
zb_ret_t osif_sem_init(osif_sem_t *sem, zb_int_t value)
{
  int ret = sem_init(sem, 0, value);
  return ret ? SYSTEM_ERROR_CODE(errno) : 0;
}


zb_ret_t osif_sem_wait(osif_sem_t * sem)
{
  int ret = 0;
  do
  {
    ret = sem_wait(sem);
  } while (ret < 0 && errno == EINTR);
  return ret ? SYSTEM_ERROR_CODE(errno) : 0;
}


zb_ret_t osif_sem_trywait(osif_sem_t * sem)
{
  int ret = 0;
  do
  {
    ret = sem_trywait(sem);
  } while (ret < 0 && errno == EINTR);
  return ret ? SYSTEM_ERROR_CODE(errno) : 0;
}


zb_ret_t osif_sem_post(osif_sem_t * sem)
{
  int ret = 0;
  do
  {
    ret = sem_post(sem);
  } while (ret < 0 && errno == EINTR);
  return ret ? SYSTEM_ERROR_CODE(errno) : 0;
}


zb_ret_t osif_sem_getvalue(osif_sem_t * sem, zb_int_t *sval)
{
  return sem_getvalue(sem, sval);
}


zb_ret_t osif_sem_destroy(osif_sem_t * sem)
{
  int ret = sem_destroy(sem);
  return ret ? SYSTEM_ERROR_CODE(errno) : 0;
}
#endif  /* ZB_NO_POSIX_SEMAPHORES */
#endif

#endif /* UNIX */
