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
/* PURPOSE: serial transport, pseudoterminal master
*/

#define _XOPEN_SOURCE 600
#define _DEFAULT_SOURCE
#define ZB_TRACE_FILE_ID 30015
#include "zb_common.h"


#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#if defined ZB_SERIAL_PTY_MASTER

static const char default_slave_pty_path[] = SLAVE_PTY_DEFAULT_PATH;


static zb_ret_t create_slave_pty_symlink(const char *slave_pty_name)
{
  struct stat stat_buf;
  zb_ret_t ret = RET_ERROR;
  const char *symlink_name = NULL;

  ZB_ASSERT(slave_pty_name);

  do
  {
    symlink_name = getenv(SLAVE_PTY_ENV_NAME);
    if (symlink_name == NULL)
    {
      TRACE_MSG(TRACE_ERROR, "%s envvar is not set, use default pty slave name",
                (FMT__P, SLAVE_PTY_ENV_NAME));
      symlink_name = default_slave_pty_path;
    }

    if (!lstat(symlink_name, &stat_buf))
    {
      if (stat_buf.st_mode & S_IFLNK)
      {
        char referred_filename[ZB_MAX_FILE_PATH_SIZE];
        ssize_t filename_size = readlink(symlink_name, referred_filename, ZB_MAX_FILE_PATH_SIZE);
        if (filename_size == ZB_MAX_FILE_PATH_SIZE)
        {
          TRACE_MSG(TRACE_ERROR, "readlink failed, too long filename", (FMT__0));
          break;
        }
        if (filename_size == -1)
        {
          TRACE_MSG(TRACE_ERROR, "readlink failed, errno: %s", (FMT__D, errno));
          break;
        }

        if (!access(referred_filename, F_OK)
            && strcmp(slave_pty_name, referred_filename))
        {
            TRACE_MSG(TRACE_ERROR, "%s exists, refers to %s, can't create a symlink to slave pty",
                      (FMT__P_P, symlink_name, referred_filename));
            break;
        }

        if(unlink(symlink_name))
        {
          TRACE_MSG(TRACE_ERROR, "failed to unlink %s", (FMT__P, symlink_name));
          break;
        }
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "%s exists, can't create a symlink to slave pty",
                  (FMT__P, symlink_name));
        break;
      }
    }

    if (symlink(slave_pty_name, symlink_name))
    {
      TRACE_MSG(TRACE_ERROR, "failed to create symlink %s to %s, errno: %d",
                (FMT__P_P_D, symlink_name, slave_pty_name, errno));
      break;
    }

    TRACE_MSG(TRACE_OSIF2, "created symlink %s to %s", (FMT__P_P, symlink_name, slave_pty_name));

    ret = RET_OK;
  } while(0);

  return ret;
}


static zb_ret_t set_master_pty_params(int fd)
{
  struct termios ts;
  zb_ret_t ret = RET_ERROR;

  do
  {
    if (tcgetattr(fd, &ts) < 0)
    {
      TRACE_MSG(TRACE_ERROR, "failed to get terminal attributes, errno: %d", (FMT__D, errno));
      break;
    }

    cfmakeraw(&ts);

    /* TODO: make it configurable? */
    cfsetospeed(&ts, B115200);
    cfsetispeed(&ts, B115200);

    if (tcsetattr(fd, TCSANOW, &ts))
    {
      TRACE_MSG(TRACE_ERROR, "failed to set terminal attributes, errno: %d", (FMT__D, errno));
      break;
    }

    ret = RET_OK;
  } while(0);

  return ret;
}


static int open_master_pty(void)
{
  const char *slave_pty_name = NULL;
  int master_pty_fd;
  int ret = -1;

  TRACE_MSG(TRACE_OSIF1, ">> open_master_pty", (FMT__0));

  do
  {
    master_pty_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_pty_fd == -1)
    {
      TRACE_MSG(TRACE_ERROR, "failed to open the master pty, errno: %d", (FMT__D, errno));
      break;
    }

    if (set_master_pty_params(master_pty_fd) != RET_OK)
    {
      TRACE_MSG(TRACE_ERROR, "failed to set master pty params", (FMT__0));
      break;
    }

    if (grantpt(master_pty_fd) == -1)
    {
      TRACE_MSG(TRACE_ERROR, "failed to grant access to the slave pty, errno: %d", (FMT__D, errno));
      break;
    }

    if (unlockpt(master_pty_fd) == -1)
    {
      TRACE_MSG(TRACE_ERROR, "failed to unlock the slave pty, errno: %d", (FMT__D, errno));
      break;
    }

    slave_pty_name = ptsname(master_pty_fd);
    if (!slave_pty_name)
    {
      TRACE_MSG(TRACE_ERROR, "failed to get the slave pty name", (FMT__0));
      break;
    }
    TRACE_MSG(TRACE_OSIF2, "slave pty: %s", (FMT__P, slave_pty_name));

    if (create_slave_pty_symlink(slave_pty_name) != RET_OK)
    {
      TRACE_MSG(TRACE_ERROR, "failed to create pty slave symlink", (FMT__0));
      break;
    }

    ret = master_pty_fd;
  } while(0);

  TRACE_MSG(TRACE_OSIF1, "<< open_master_pty, master_pty_fd %d", (FMT__D, ret));

  return ret;
}


void zb_osif_serial_init(void)
{
  int master_pty_fd;

  TRACE_MSG(TRACE_OSIF1, ">> zb_osif_serial_init", (FMT__0));

  master_pty_fd = open_master_pty();
  ZB_ASSERT(master_pty_fd != -1);

  osif_serial_common_init(master_pty_fd);

  TRACE_MSG(TRACE_OSIF1, "<< zb_osif_serial_init", (FMT__0));
}

#endif /* defined ZB_SERIAL_PTY_MASTER */
