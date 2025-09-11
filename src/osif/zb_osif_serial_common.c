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
/* PURPOSE: serial transport, common
*/

#define ZB_TRACE_FILE_ID 30016
#include "zb_common.h"

#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#if defined ZB_HAVE_SERIAL

#define SERIAL_RX_BUF_SIZE 256

typedef struct serial_ctx_s
{
#if defined ZB_HAVE_ASYNC_SERIAL
  zb_uint8_t *rx_buf;
  zb_size_t rx_buf_size;
  zb_uindex_t rx_buf_offset;
  zb_size_t rx_buf_remaining_size;
  serial_recv_data_cb_t rx_done_cb;
  serial_send_data_cb_t tx_done_cb;
#else /* !defined ZB_HAVE_ASYNC_SERIAL */
  zb_uint8_t rx_buf[SERIAL_RX_BUF_SIZE];
  zb_callback_t rx_byte_cb;
#endif /* !defined ZB_HAVE_ASYNC_SERIAL */
  int serial_fd;
  zb_bool_t inited;
} serial_ctx_t;

static serial_ctx_t serial_ctx;

static zb_ret_t send_data(const zb_uint8_t *buf, zb_ushort_t len)
{
  zb_ret_t ret = RET_OK;
  zb_size_t bytes_written = 0;


  if (TRACE_ENABLED(TRACE_OSIF4))
  {
    zb_uindex_t i;
    for (i = 0; i < len; i++)
    {
      TRACE_MSG(TRACE_OSIF4, "buf[%d] = 0x%hx", (FMT__D_H, i, buf[i]));
    }
  }

  do
  {
    errno = 0;
    bytes_written = write(serial_ctx.serial_fd, buf, len);
    if (bytes_written > 0)
    {
      buf += bytes_written;
      len -= bytes_written;
    }
    else
    {
      if (errno != EAGAIN && errno != EINTR)
      {
        TRACE_MSG(TRACE_ERROR, "failed to write to file %d, errno: %d",
                  (FMT__D_D, serial_ctx.serial_fd, errno));
        break;
      }
    }
  } while (len);

  if (len)
  {
    ret = RET_ERROR;
  }

  return ret;
}


#if defined ZB_HAVE_ASYNC_SERIAL
static void free_rx_buffer(void)
{
  serial_ctx.rx_buf = NULL;
  serial_ctx.rx_buf_size = 0;
  serial_ctx.rx_buf_remaining_size = 0;
  serial_ctx.rx_buf_offset = 0;
}


void zb_osif_serial_recv_data(zb_uint8_t *buf, zb_ushort_t len)
{
  TRACE_MSG(TRACE_OSIF2, ">> zb_osif_serial_recv_data, buf %p, len %d", (FMT__P_D, buf, len));

  if (serial_ctx.inited)
  {
    serial_ctx.rx_buf = buf;
    serial_ctx.rx_buf_size = len;
    serial_ctx.rx_buf_offset = 0;
    serial_ctx.rx_buf_remaining_size = len;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Read error: serial transport is not initialized!", (FMT__0));
  }

  TRACE_MSG(TRACE_OSIF4, "<< zb_osif_serial_recv_data", (FMT__0));
}


void zb_osif_serial_set_cb_recv_data(serial_recv_data_cb_t cb)
{
  TRACE_MSG(TRACE_OSIF2, "zb_osif_serial_set_cb_recv_data, cb %p", (FMT__P, cb));
  serial_ctx.rx_done_cb = cb;
}


void zb_osif_serial_send_data(zb_uint8_t *buf, zb_ushort_t len)
{
  zb_ret_t status;

  TRACE_MSG(TRACE_OSIF4, ">> zb_osif_serial_send_data, buf %p, len %d", (FMT__P_D, buf, len));

  if (serial_ctx.inited)
  {

    status = send_data(buf, len);
    ZB_ASSERT(status == RET_OK);

    if (serial_ctx.tx_done_cb)
    {
      ZB_SCHEDULE_CALLBACK(serial_ctx.tx_done_cb, SERIAL_SEND_SUCCESS);
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Send error: serial transport is not initialized!", (FMT__0));

    if (serial_ctx.tx_done_cb)
    {
      ZB_SCHEDULE_CALLBACK(serial_ctx.tx_done_cb, SERIAL_SEND_ERROR);
    }
  }

  TRACE_MSG(TRACE_OSIF4, "<< zb_osif_serial_send_data", (FMT__0));
}


void zb_osif_serial_set_cb_send_data(serial_send_data_cb_t cb)
{
  TRACE_MSG(TRACE_OSIF2, "zb_osif_serial_set_cb_send_data, cb %p", (FMT__P, cb));
  serial_ctx.tx_done_cb = cb;
}


static void handle_read_event(zb_uint8_t event_mask)
{
  ssize_t bytes_read = 0;
  ZB_ASSERT(event_mask == OSIF_IPC_SIGNAL_RX);

  TRACE_MSG(TRACE_OSIF4, ">> handle_read_event", (FMT__0));

  if (!serial_ctx.rx_buf)
  {
    uint8_t buf[1];
    TRACE_MSG(TRACE_ERROR, "no rx buf!", (FMT__0));
    do
    {
      bytes_read = read(serial_ctx.serial_fd,
                        buf,
                        sizeof(buf));
    } while (bytes_read < 0 && errno == EINTR);
  }
  else
  {
    errno = 0;

    do
    {
      bytes_read = read(serial_ctx.serial_fd,
                        serial_ctx.rx_buf + serial_ctx.rx_buf_offset,
                        serial_ctx.rx_buf_remaining_size);
    } while (bytes_read < 0 && errno == EINTR);

    TRACE_MSG(TRACE_OSIF4, "received %d bytes", (FMT__D, bytes_read));

    if (bytes_read > 0)
    {
      serial_ctx.rx_buf_offset += bytes_read;
      serial_ctx.rx_buf_remaining_size -= bytes_read;

      if (TRACE_ENABLED(TRACE_OSIF4))
      {
        zb_uindex_t i;
        for (i = 0; i < bytes_read; i++)
        {
          TRACE_MSG(TRACE_OSIF4, "received byte: %d", (FMT__D, serial_ctx.rx_buf[i]));
        }
      }

      /* Note: that if() makes impossible using of that cpode for MAC split
       * transport. MAC split low level does not know how many bytes it must
       * read and wants to read up to free stace in the ring buffer.
       * Will NCP protocol logic be broken if we call its callback when not all bytes have been read?
       */
      if (!serial_ctx.rx_buf_remaining_size)
      {
        zb_uint8_t* buf = serial_ctx.rx_buf;
        zb_size_t buf_size = serial_ctx.rx_buf_size;

        free_rx_buffer();
        if (serial_ctx.rx_done_cb)
        {
          serial_ctx.rx_done_cb(buf, buf_size);
        }
      }
    }
    else
    {
      TRACE_MSG(TRACE_ERROR, "read error, errno: %d", (FMT__D, errno));
      // NOFFZ TODO: figure out reason of error, handle it
      ZB_ERROR_RAISE(ZB_ERROR_SEVERITY_FATAL,
                     ERROR_CODE(ERROR_CATEGORY_SERIAL, ZB_ERROR_SERIAL_READ_FAILED),
                     NULL);
    }
  }

  TRACE_MSG(TRACE_OSIF4, "<< handle_read_event", (FMT__0));
}

#else /* !defined ZB_HAVE_ASYNC_SERIAL */

void zb_osif_set_uart_byte_received_cb(zb_callback_t cb)
{
  TRACE_MSG(TRACE_OSIF2, "zb_osif_set_uart_byte_received_cb, cb %p", (FMT__P, cb));

  serial_ctx.rx_byte_cb = cb;
}


void zb_osif_serial_put_bytes(const zb_uint8_t *buf, zb_short_t len)
{
  zb_ret_t status = RET_ERROR;

  TRACE_MSG(TRACE_OSIF4, ">> zb_osif_serial_put_bytes, buf %p, len %d", (FMT__P_D, buf, len));

  ZB_ASSERT(serial_ctx.inited);

  status = send_data(buf, len);

  ZB_ASSERT(status == RET_OK);

  TRACE_MSG(TRACE_OSIF4, "<< zb_osif_serial_put_bytes", (FMT__0));
}


static void handle_read_event(zb_uint8_t event_mask)
{
  ssize_t bytes_read = 0;
  ZB_ASSERT(event_mask == OSIF_IPC_SIGNAL_RX);

  TRACE_MSG(TRACE_OSIF4, ">> handle_read_event", (FMT__0));

  bytes_read = read(serial_ctx.serial_fd,
                    serial_ctx.rx_buf,
                    SERIAL_RX_BUF_SIZE);

  TRACE_MSG(TRACE_OSIF4, "received %d bytes", (FMT__D, bytes_read));

  ZB_ASSERT(bytes_read != 0);

  if (bytes_read > 0)
  {
    zb_uindex_t i = 0;

    if (TRACE_ENABLED(TRACE_OSIF4))
    {
      for (i = 0; i < bytes_read; i++)
      {
        TRACE_MSG(TRACE_OSIF4, "received byte: 0x%x", (FMT__D, serial_ctx.rx_buf[i]));
      }
    }

    if (serial_ctx.rx_byte_cb)
    {
      for (i = 0; i < bytes_read; i++)
      {
        serial_ctx.rx_byte_cb(serial_ctx.rx_buf[i]);
      }
    }
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "errno: %d", (FMT__D, errno));
  }

  TRACE_MSG(TRACE_OSIF4, "<< handle_read_event", (FMT__0));
}

#endif /* !defined ZB_HAVE_ASYNC_SERIAL */

void osif_serial_common_init(int fd)
{
  zb_ret_t status = RET_ERROR;

  ZB_BZERO(&serial_ctx, sizeof(serial_ctx));

  ZB_ASSERT(fd >= 0);
  serial_ctx.serial_fd = fd;

  status = osif_transport_add_io_handler(serial_ctx.serial_fd,
                                         OSIF_IPC_SIGNAL_RX,
                                         handle_read_event);
  ZB_ASSERT(status == RET_OK);

  serial_ctx.inited = ZB_TRUE;
}


void osif_serial_common_deinit(int fd)
{
  osif_transport_remove_io_handler(fd);
  serial_ctx.inited = ZB_FALSE;
}

#endif /* defined ZB_HAVE_SERIAL */
