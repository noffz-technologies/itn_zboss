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
/* PURPOSE: serial transport
*/

#define ZB_TRACE_FILE_ID 30017
#include "zb_common.h"
#include "zb_error_indication.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#if defined ZB_SERIAL_PTY_SLAVE || defined ZB_TRANSPORT_LINUX_UART

#ifndef ZB_MAC_TRANSPORT_UART_SPEED
#define ZB_MAC_TRANSPORT_UART_SPEED B115200
#endif

#ifndef ZB_MAC_TRANSPORT_UART_PATH_SIZE
#define ZB_MAC_TRANSPORT_UART_PATH_SIZE 256
#endif

/* Let's ZB_MAC_TRANSPORT_UART_PATH have a priority */
#ifdef ZB_MAC_TRANSPORT_UART_PATH
#undef SERIAL_DEFAULT_PATH
#endif

#ifndef SERIAL_DEFAULT_PATH
#ifdef ZB_MAC_TRANSPORT_UART_PATH
#define SERIAL_DEFAULT_PATH ZB_MAC_TRANSPORT_UART_PATH
#else
#define SERIAL_DEFAULT_PATH ""
#endif
#endif

static int s_fd = -1;

/**
 * Provide UART port path
 */
ZB_WEAK_PRE const char* zb_mac_uart_path() ZB_WEAK;
const char* zb_mac_uart_path(void)
{
  return SERIAL_DEFAULT_PATH;
}

static int open_low(const char* filename)
{
  int fd = -1;
  fd = open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(fd > 1024)
  {
    do
    {
      TRACE_MSG(TRACE_ERROR, "fd too high: %d", (FMT__D, fd));
      usleep(100000);
      int new_fd = fcntl(fd, F_DUPFD, 3);
      close(fd);
      fd = new_fd;
    } while(fd > 1024);
  }
  return fd;
}

static int open_serial_port(void)
{
  int fd = -1;
  const char* filename = NULL;
  char uart_path[ZB_MAC_TRANSPORT_UART_PATH_SIZE];

  do
  {
    filename = getenv(SERIAL_ENV_NAME);
    if (filename == NULL)
    {
      TRACE_MSG(TRACE_ERROR, "%s envvar is not set, use default pty slave name",
                (FMT__P, SERIAL_ENV_NAME));
      sprintf((char*)uart_path, "%s", zb_mac_uart_path());
      filename = (char *)uart_path;
    }

    do
    {
      errno = 0;
      fd = open_low(filename);
    } while (fd < 0 && errno == EINTR);
    if (fd == -1)
    {
      TRACE_MSG(TRACE_ERROR, "failed to open file %s, errno: %d", (FMT__P_D, filename, errno));
      break;
    }

    /* Flush IO buffers so we don't receive buffered messaged which
       were sent before we opened the port.
       Needed for NCP: we need to flush buffers so host don't receive many duplicates
       of unsolicited NCP Module Reset Response */
    osif_usleep(10000);
    tcflush(fd, TCIOFLUSH);

  } while(0);

  return fd;
}


static zb_ret_t set_serial_port_params(int fd)
{
  struct termios ts;
  zb_ret_t ret = RET_ERROR;
  int fcntl_return_code = 0;

  do
  {
    if (tcgetattr(fd, &ts) < 0)
    {
      TRACE_MSG(TRACE_ERROR, "failed to get terminal attributes, errno: %d", (FMT__D, errno));
      break;
    }

    cfmakeraw(&ts);

    /* TODO: make it configurable */
    cfsetospeed(&ts, ZB_MAC_TRANSPORT_UART_SPEED);
    cfsetispeed(&ts, ZB_MAC_TRANSPORT_UART_SPEED);
    ts.c_cflag &= ~(CSIZE | PARENB);
    ts.c_cflag |= CS8 | CREAD | HUPCL | CLOCAL;
    errno = 0;

    if (tcsetattr(fd, TCSANOW, &ts))
    {
      TRACE_MSG(TRACE_ERROR, "failed to set terminal attributes, errno: %d", (FMT__D, errno));
      break;
    }
    /* Port is opened with O_NONBLOCK flag */
    if ((fcntl_return_code = fcntl(fd, F_GETFL, 0)) < 0)
    {
      TRACE_MSG(TRACE_ERROR, "fcntl get error", (FMT__0));
    }
    else
    {
      fcntl_return_code &= ~O_NONBLOCK;
      if (fcntl(fd, F_SETFL, fcntl_return_code) < 0)
      {
        TRACE_MSG(TRACE_ERROR, "fcntl set error", (FMT__0));
      }
    }

    ret = RET_OK;
  } while(0);

  return ret;
}


void zb_osif_serial_init(void)
{
  zb_ret_t status;

  do
  {
    zb_osif_serial_deinit();

    s_fd = open_serial_port();
    if (s_fd == -1)
    {
      ZB_ERROR_RAISE(ZB_ERROR_SEVERITY_FATAL,
                     ERROR_CODE(ERROR_CATEGORY_SERIAL, ZB_ERROR_SERIAL_INIT_FAILED),
                     (void*)errno);
      break;
    }

    status = set_serial_port_params(s_fd);
    ZB_ASSERT(status == RET_OK);

    osif_serial_common_init(s_fd);
  } while(0);
}


void zb_osif_serial_deinit(void)
{
  if (s_fd != -1)
  {
    osif_serial_common_deinit(s_fd);
    close(s_fd);
    s_fd = -1;
  }
}

#endif /* defined ZB_SERIAL_PTY_SLAVE */
