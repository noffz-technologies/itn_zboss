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
/* PURPOSE: File i/o, trace, traffic dump
*/

#define ZB_TRACE_FILE_ID 30012
#include "zb_common.h"
#include <errno.h>
#include <syslog.h>
#include <dirent.h>

#ifdef ZB_USE_LOGFILE_ROTATE
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#endif

#ifdef ZB_PLATFORM_LINUX
#include "linux/limits.h"
#include "fcntl.h"
#include <sys/stat.h>
#endif

struct timespec start_t = { .tv_sec = ~0, .tv_nsec = ~0 };

/**
   Global trace mutex for Unix trace implementation
 */
static pthread_mutex_t s_trace_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef ZB_FILE_PATH_MGMNT

zb_file_path_base_type_t zb_file_path_types[ZB_FILE_PATH_MAX_TYPES];
#endif  /* ZB_FILE_PATH_MGMNT */

#ifdef ZB_USE_LOGFILE_ROTATE
#define ZB_LOG_FILE_INDEX_UNDEFINED ((zb_uint32_t )(~0))

static zb_char_t s_logname[255];

zb_uint32_t zb_osif_get_file_size(zb_char_t *name)
{
  struct stat file_stat;

  if(stat(name, &file_stat) == -1)
  {
    return -1;
  }
  return file_stat.st_size;
}


static zb_uint32_t zb_osif_log_file_get_index(void)
{
  struct dirent *de;
  zb_char_t   file_name[ZB_MAX_FILE_PATH_SIZE];
  zb_char_t  *ptr;
  zb_uint32_t val = 0;
  zb_uint32_t f_idx = 0;

  DIR *dr = opendir(ZB_FILE_PATH_GET(ZB_FILE_PATH_BASE_RAMFS_TRACE_LOGS,
                                     ZB_TMP_FILE_PATH_PREFIX));

  if (dr == NULL)
  {
    return 0;
  }

  sprintf(file_name, "%s.log.", s_logname);

  while ((de = readdir(dr)) != NULL)
  {
    if (de->d_type == DT_REG)
    {
      ptr = strstr(de->d_name, file_name);
      if (ptr)
      {
        val = atoi(ptr + strlen(file_name));

        if (val + 1 > 0)
        {
          if (val + 1 > f_idx)
          {
            f_idx = val + 1;
          }
        }
        else
        {
          /* TODO: handle index overflow in the proper way! */
          f_idx = 0;
          break;
        }
      }
    }
  }

  closedir(dr);

  return f_idx;
}


zb_int8_t zb_osif_log_file_rotate(void)
{
  static zb_uint32_t f_idx = ZB_LOG_FILE_INDEX_UNDEFINED;
  zb_char_t suffix[10];
  zb_char_t rm_file_name[ZB_MAX_FILE_PATH_SIZE];
  zb_char_t new_name[ZB_MAX_FILE_PATH_SIZE];
  zb_char_t namefile[ZB_MAX_FILE_PATH_SIZE];

  /* 09/19/2018 EE CR:MINOR Better have full path in s_logname and do not compose the name in 3 places */
  sprintf(namefile, "%s%s.log",
          ZB_FILE_PATH_GET(ZB_FILE_PATH_BASE_RAMFS_TRACE_LOGS,
                           ZB_TMP_FILE_PATH_PREFIX),
          s_logname);
  strcpy(new_name, namefile);

  /* get last log file index */
  if (f_idx == ZB_LOG_FILE_INDEX_UNDEFINED)
  {
    f_idx = zb_osif_log_file_get_index();
  }
  else
  {
    f_idx++;
  }

  sprintf(suffix, ".%d", f_idx);
  strcat(new_name, suffix);

  zb_trace_deinit_file();
  /* 09/19/2018 EE CR:MAJOR Check ret code! Maybe, file is not renamed? In such case there will be no rotation. */
  rename(namefile, new_name);
  zb_trace_init_file(s_logname);

  /* remove old log files */
  if(f_idx >= MAX_LOGFILE_INDEX)
  {
/* 09/19/2018 EE CR:MAJOR unlink not just one file, but all previous files. It coule be there if we are crashing. */
    sprintf(rm_file_name, "%s.%d", namefile, f_idx-MAX_LOGFILE_INDEX);
    remove(rm_file_name);
  }

  return 0;
}

#endif

#ifdef ZB_FILE_PATH_MGMNT
void zb_file_path_init(void)
{
  ZB_BZERO(zb_file_path_types, sizeof(zb_file_path_types));

  zb_file_path_declare(ZB_FILE_PATH_BASE_ROMFS_BINARIES, ZB_BINARY_FILE_PATH_PREFIX);
  zb_file_path_declare(ZB_FILE_PATH_BASE_MNTFS_BINARIES, ZB_BINARY_FILE_PATH_PREFIX);
  zb_file_path_declare(ZB_FILE_PATH_BASE_MNTFS_USER_DATA, ZB_USERDATA_FILE_PATH_PREFIX);
  zb_file_path_declare(ZB_FILE_PATH_BASE_RAMFS_UNIX_SOCKET, UNIX_DOMAIN_SOCKETS_DIR);
  zb_file_path_declare(ZB_FILE_PATH_BASE_RAMFS_TRACE_LOGS, ZB_TMP_FILE_PATH_PREFIX);
  zb_file_path_declare(ZB_FILE_PATH_BASE_RAMFS_TMP_DATA, ZB_TMP_FILE_PATH_PREFIX);
}


zb_ret_t zb_file_path_declare(zb_uint8_t base_type, const char *base)
{
  ZB_ASSERT(base_type < ZB_FILE_PATH_MAX_TYPES);
  strcpy(zb_file_path_types[base_type].base, base);

  zb_file_path_types[base_type].declared = ZB_TRUE;
  return RET_OK;
}


const char* zb_file_path_get(zb_uint8_t base_type, const char *default_base)
{
  ZB_ASSERT(base_type < ZB_FILE_PATH_MAX_TYPES);

  if (zb_file_path_types[base_type].declared)
  {
    return zb_file_path_types[base_type].base;
  }
  return default_base;
}


void  zb_file_path_get_with_postfix(zb_uint8_t base_type,const char *default_base,
				    const char *postfix, char *file_path)
{
  const char *path = NULL;

  ZB_ASSERT(base_type < ZB_FILE_PATH_MAX_TYPES);

  if (zb_file_path_types[base_type].declared)
  {
    path = zb_file_path_types[base_type].base;
  }
  else
  {
    path = default_base;
  }

  snprintf(file_path, ZB_MAX_FILE_PATH_SIZE, "%s%s", path, postfix);
}
#endif  /* ZB_FILE_PATH_MGMNT */

/**
   Unix-specific trace file open and mutex init

   @param name - process name. Trace file name generated from this name and current pid
 */
zb_osif_file_t *zb_osif_init_trace(zb_char_t *name)
{
  zb_osif_file_t *f;
  zb_char_t namefile[ZB_MAX_FILE_PATH_SIZE];
  const char *path;

  path = ZB_FILE_PATH_GET(ZB_FILE_PATH_BASE_RAMFS_TRACE_LOGS,
                          ZB_TMP_FILE_PATH_PREFIX);
  if (strlen(path))
  {
    sprintf(namefile, "%s%s.log", path, name);
  }
  else
  {
#ifdef ZB_BINARY_TRACE
    sprintf(namefile, "%s.bin.log", name);
#else
    sprintf(namefile, "%s.log", name);
#endif
  }
  f = fopen(namefile, "a+");
  if (!f)
  {
    f = fopen(namefile, "w");
  }

  if (start_t.tv_nsec == ~0 && start_t.tv_sec == ~0)
  {
    osif_get_clock_monotonic_raw(&start_t);
  }

#ifdef ZB_USE_LOGFILE_ROTATE
 /* save log file name, for futher use */
  strcpy(s_logname, name);
#endif

  if (f)
  {
/* 09/19/2018 EE CR:MINOR Not meaningful for binary trace. And, anyway, better call flush explicitly. */
    /* set line buffering so not need to call fflush() */
    setvbuf(f, NULL, _IOLBF, 512);
  }
  return f;
}


/* 08/23/2018 EE CR:MAJOR How do you rotate a traffic dump file? Will it grow forever? */

zb_osif_file_t *zb_osif_init_dump(zb_char_t *name)
{
  zb_osif_file_t *f;
  zb_char_t namefile[255];

  sprintf(namefile, "%s.dump", name);
  f = fopen(namefile, "a+");
  return f;
}


zb_bool_t zb_osif_check_file_exist(const zb_char_t *name, const zb_uint8_t mode)
{
  if (!access(name, mode))
  {
    return ZB_TRUE;
  }
  return ZB_FALSE;
}


zb_osif_file_t *zb_osif_file_open(const zb_char_t *name, const zb_char_t *mode)
{
  zb_osif_file_t *f;
  f = fopen(name, mode);
  return f;
}

#define COPY_BUF_SIZE 4096

void zb_osif_file_copy(const zb_char_t *name_src, const zb_char_t *name_dst)
{
  int f_src;
  int f_dst;
  int err_r;
  int err_w;
  unsigned char buf[COPY_BUF_SIZE];

  f_src = open(name_src, O_RDONLY);
  f_dst = open(name_dst, O_CREAT | O_WRONLY, S_IWRITE | S_IREAD);

  while (1)
  {
    err_r = read(f_src, buf, COPY_BUF_SIZE);

    if (err_r == -1)
    {
      ZB_ASSERT(0);
    }
    else if (err_r == 0)
    {
      break;
    }
    else
    {
      err_w = write(f_dst, buf, err_r);
      if (err_w == -1)
      {
        ZB_ASSERT(0);
      }
    }
  }

  close(f_src);
  close(f_dst);
}


zb_osif_file_t *zb_osif_popen(zb_char_t *arg)
{
  return popen(arg, "w");
}


zb_osif_file_t *zb_osif_file_stdout()
{
  return stdout;
}


zb_osif_file_t *zb_osif_file_stdin()
{
  return stdin;
}


void zb_osif_file_close(zb_osif_file_t *f)
{
  fflush(f);
  fclose(f);
}


int zb_osif_file_remove(const zb_char_t *name)
{
  return remove(name);
}


void zb_osif_trace_printf(zb_osif_file_t *f, const zb_char_t *format, ...)
{
  va_list   arglist;
  va_start(arglist, format);
  vfprintf(f, format, arglist);
  va_end(arglist);
}


void zb_osif_trace_vprintf(zb_osif_file_t *f, const zb_char_t *format, va_list arglist)
{
  vfprintf(f, format, arglist);
}


int zb_osif_file_read(zb_osif_file_t *f, zb_uint8_t *buf, zb_uint_t len)
{
  return fread(buf, 1, len, f);
}


int zb_osif_file_write(zb_osif_file_t *f, const zb_uint8_t *buf, zb_uint_t len)
{
  return fwrite(buf, 1, len, f);
}


int zb_osif_file_flush(zb_osif_file_t *f)
{
  /* 08/23/2018 EE CR:MAJOR There can be more than one file open, so it is strange to initiate log rotate at _every_ file flush.
     But, ok, it will work. At least add a comment.
   */
  zb_int_t ret;

  ret = fflush(f);

#ifdef ZB_USE_LOGFILE_ROTATE
  {
    zb_uint32_t file_size = 0;
    zb_char_t namefile[ZB_MAX_FILE_PATH_SIZE];

    sprintf(namefile, "%s%s.log",
            ZB_FILE_PATH_GET(ZB_FILE_PATH_BASE_RAMFS_TRACE_LOGS,
                             ZB_TMP_FILE_PATH_PREFIX),
            s_logname);
    file_size = zb_osif_get_file_size(namefile);

    if(file_size > ZB_DEFAULT_MAX_LOGFILE_SIZE)
    {
      zb_osif_log_file_rotate();
    }
  }
#endif

  return ret;
}


int zb_osif_file_sync(zb_osif_file_t *f)
{
  return fsync(fileno(f));
}


int zb_osif_file_truncate(zb_osif_file_t *f, zb_uint32_t off)
{
  zb_osif_file_flush(f);
  zb_osif_file_seek(f, 0, SEEK_SET);
  return ftruncate(fileno(f), off);
}


int zb_osif_file_seek(zb_osif_file_t *f, zb_uint32_t off, zb_uint8_t mode)
{
  return fseek(f, off, mode);
}


void zb_osif_trace_get_time(zb_uint_t *sec, zb_uint_t *msec)
{
  struct timespec time;

  osif_get_clock_monotonic_raw(&time);

  *sec = time.tv_sec - start_t.tv_sec;
  *msec = (time.tv_nsec - start_t.tv_nsec) / 1000000LL;
  if (*msec > 1000)
  {
    *sec -= 1;
    *msec += 1000;
  }
}


/**
   Unix-specific trace lock: lock mutex
 */
void zb_osif_trace_lock()
{
  pthread_mutex_lock(&s_trace_mutex);
}


/**
   Unix-specific trace unlock: unlock mutex
 */
void zb_osif_trace_unlock()
{
  pthread_mutex_unlock(&s_trace_mutex);
}


void osif_close_all_descriptor()
{
    int max_fd;
    int i;

    /* 08/23/2018 EE CR:MINOR Use DSR code style: brackets always; at the separate line. Here and everywhere. */
#if defined(F_MAXFD)
    do {
        max_fd = fcntl(0, F_MAXFD);
    } while (max_fd == -1 && errno == EINTR);
    if (max_fd == -1) {
        max_fd = sysconf(_SC_OPEN_MAX);
    }
#else
  DIR *dir = NULL;
  struct dirent *ent;
  max_fd = 3;

  dir = opendir("/proc/self/fd");
  if (dir == NULL) {
    dir = opendir("/proc/fd");
  }

  if (dir != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_name[0] != '.') {
        int number = atoi(ent->d_name);
        if (number > max_fd) {
          max_fd = number;
        }
      }
    }
    closedir(dir);
    ++max_fd;
  }
#endif
/* 08/23/2018 EE CR:MAJOR Either use <= here or do ++max_fd not inside ifdef. I prefer <=.  */
  for (i = 3; i < max_fd; ++i)
    close(i);
}

int zb_osif_stream_read(zb_osif_file_t *stream, zb_uint8_t *buf, zb_uint_t len)
{
  return read(fileno(stream), buf, len);
}

int zb_osif_stream_write(zb_osif_file_t *stream, zb_uint8_t *buf, zb_uint_t len)
{
  return write(fileno(stream), buf, len);
}
