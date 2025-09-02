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
/* PURPOSE: File i/o trace, traffic dump and, maybe, other unix-specific
*/

#define ZB_TRACE_FILE_ID 30009
#include "zboss_api_core.h"
#include <pthread.h>
#include <errno.h>

#include "zb_common.h"
#ifdef ZB_STACK_REGRESSION_TESTING_API
#include "zb_g_context.h"
#endif

#ifndef ZB_NVRAM_PAGE_SIZE
#define ZB_NVRAM_PAGE_SIZE   0x4000
#endif
#define ZB_NVRAM_PAGE_COUNT  2

/*! \addtogroup ZB_OSIF */
/*! @{ */

#if defined ZB_USE_NVRAM || defined DOXYGEN

zb_osif_file_t *nvram_file;
//zb_osif_file_t *nvram_file_raw;


enum zb_osif_nvram_last_operation_e
{
  ZB_OSIF_NVRAM_LAST_OPERATION_UNDEFINED,
  ZB_OSIF_NVRAM_LAST_OPERATION_READ,
  ZB_OSIF_NVRAM_LAST_OPERATION_WRITE,
};

static zb_uint8_t zb_osif_nvram_last_operation;

#ifdef ZB_PRODUCTION_CONFIG

zb_osif_file_t *production_file;

#define ZB_OSIF_PRODUCTION_NAME                      "product.dat"
#define ZB_OSIF_PRODUCTION_CONFIG_MAGIC             {0xE7, 0x37, 0xDD, 0xF6}
#define ZB_OSIF_PRODUCTION_CONFIG_MAGIC_SIZE        4

zb_bool_t zb_osif_prod_cfg_check_presence()
{
  zb_uint8_t hdr[] = ZB_OSIF_PRODUCTION_CONFIG_MAGIC;
  zb_uint8_t buffer[ZB_OSIF_PRODUCTION_CONFIG_MAGIC_SIZE];
//  production_file = fopen(ZB_OSIF_PRODUCTION_NAME, "r+");
//  if(!production_file)
//  {
//    TRACE_MSG(TRACE_COMMON1, "new production file", (FMT__0));
//    production_file = fopen(ZB_OSIF_PRODUCTION_NAME, "w+");
//  }
  fseek(production_file, 0, SEEK_END);
  TRACE_MSG(TRACE_COMMON1, "production size %d", (FMT__D, ftell(production_file)));
  fflush(production_file);
  fseek(production_file, 0, SEEK_SET);
  TRACE_MSG(TRACE_COMMON1, "production read pos %ld len %d", (FMT__L_D, ftell(production_file), 4));
  SUPPRESS_UNUSED_RETURN(fread(buffer, 4, 1, production_file));
  return !ZB_MEMCMP(buffer, hdr, sizeof(buffer));
}

zb_ret_t zb_osif_prod_cfg_read_header(zb_uint8_t *prod_cfg_hdr, zb_uint16_t hdr_len)
{
  fflush(production_file);
  fseek(production_file, ZB_OSIF_PRODUCTION_CONFIG_MAGIC_SIZE, SEEK_SET);
  TRACE_MSG(TRACE_COMMON1, "production read pos %ld len %d",
            (FMT__L_D, ftell(production_file), hdr_len));
  SUPPRESS_UNUSED_RETURN(fread(prod_cfg_hdr, hdr_len, 1, production_file));

  return RET_OK;
}

zb_ret_t zb_osif_prod_cfg_read(zb_uint8_t *buffer, zb_uint16_t len, zb_uint16_t offset)
{
  fflush(production_file);
  fseek(production_file, ZB_OSIF_PRODUCTION_CONFIG_MAGIC_SIZE + offset, SEEK_SET);
  TRACE_MSG(TRACE_COMMON1, "production read pos %ld len %d", (FMT__L_D, ftell(production_file), len));
  SUPPRESS_UNUSED_RETURN(fread(buffer, len, 1, production_file));

  return RET_OK;
}

#endif
/* 07.04.2017 [AEV] end */

static zb_bool_t nvram_inited = ZB_FALSE;

void zb_osif_nvram_init(const zb_char_t *name)
{
  zb_char_t namefile[ZB_MAX_FILE_PATH_SIZE];
  zb_char_t file_path[ZB_MAX_FILE_PATH_SIZE];
  zb_uint8_t i;

  if (nvram_inited)
  {
    TRACE_MSG(TRACE_COMMON1, "NVRAM is already initialized, second attemp is skipped", (FMT__0));
    return;
  }

  ZB_BZERO(file_path, sizeof(file_path));
  ZB_FILE_PATH_GET_WITH_POSTFIX(ZB_FILE_PATH_BASE_MNTFS_USER_DATA,
				ZB_USERDATA_FILE_PATH_PREFIX,
				"%s.nvram", file_path);
  sprintf(namefile, file_path, name);

  nvram_file = fopen(namefile, "r+");
  if(!nvram_file)
  {
    TRACE_MSG(TRACE_COMMON1, "new file", (FMT__0));
    nvram_file = fopen(namefile, "w+");
  }

/*  sprintf(namefile, ZB_USERDATA_FILE_PATH_PREFIX "%s.nvram.raw", name);

  nvram_file_raw = fopen(namefile, "r+");
  if(!nvram_file_raw)
  {
    TRACE_MSG(TRACE_COMMON1, "new raw file", (FMT__0));
    nvram_file_raw = fopen(namefile, "w+");
    }*/

  zb_osif_nvram_last_operation = ZB_OSIF_NVRAM_LAST_OPERATION_UNDEFINED;

#ifdef ZB_PRODUCTION_CONFIG
  ZB_BZERO(file_path, sizeof(file_path));
  ZB_FILE_PATH_GET_WITH_POSTFIX(ZB_FILE_PATH_BASE_MNTFS_BINARIES,
				ZB_BINARY_FILE_PATH_PREFIX, "%s.prod", file_path);
  sprintf(namefile, file_path, name);
  production_file = fopen(namefile, "r+");

  if(!production_file)
  {
    TRACE_MSG(TRACE_COMMON1, "new production file", (FMT__0));
    production_file = fopen(namefile, "w+");
  }
#endif
  fseek(nvram_file, 0, SEEK_END);
  if (ftell(nvram_file)<ZB_NVRAM_PAGE_SIZE*ZB_NVRAM_PAGE_COUNT)
  {
    TRACE_MSG(TRACE_COMMON1, "nvram size %d", (FMT__D, ftell(nvram_file)));
    for(i=0;i<ZB_NVRAM_PAGE_COUNT;i++)
    {
      zb_osif_nvram_erase_async(i);
    }
    zb_osif_nvram_flush();
  }

    nvram_inited = ZB_TRUE;

//  fseek(nvram_file_raw, 0, SEEK_END);
}


void zb_osif_nvram_deinit(void)
{
  if (nvram_file != NULL)
  {
    zb_osif_nvram_flush();
    fclose(nvram_file);
    nvram_file = NULL;
  }
#ifdef ZB_PRODUCTION_CONFIG
  if (production_file != NULL)
  {
    fclose(production_file);
    production_file = NULL;
  }
#endif  /* #ifdef ZB_PRODUCTION_CONFIG */
  zb_osif_nvram_last_operation = ZB_OSIF_NVRAM_LAST_OPERATION_UNDEFINED;
  nvram_inited = ZB_FALSE;
}


zb_uint32_t zb_get_nvram_page_length()
{
  return ZB_NVRAM_PAGE_SIZE;
}

zb_uint8_t zb_get_nvram_page_count()
{
  return ZB_NVRAM_PAGE_COUNT;
}

zb_ret_t zb_osif_nvram_read(zb_uint8_t page, zb_uint32_t pos, zb_uint8_t *buf, zb_uint16_t len )
{
  zb_int32_t cur_pos;

  if(page>=ZB_NVRAM_PAGE_COUNT)
  {
    return RET_PAGE_NOT_FOUND;
  }
  if(pos+len>ZB_NVRAM_PAGE_SIZE)
  {
    return RET_INVALID_PARAMETER;
  }

  if (zb_osif_nvram_last_operation != ZB_OSIF_NVRAM_LAST_OPERATION_READ)
  {
    zb_osif_nvram_last_operation = ZB_OSIF_NVRAM_LAST_OPERATION_READ;
    zb_osif_nvram_flush();
  }

  cur_pos = ftell(nvram_file);

  if ((zb_uint32_t)cur_pos != page*ZB_NVRAM_PAGE_SIZE+pos)
  {
    fseek(nvram_file, page*ZB_NVRAM_PAGE_SIZE+pos, SEEK_SET);
    cur_pos = page*ZB_NVRAM_PAGE_SIZE+pos;
  }

  TRACE_MSG(TRACE_COMMON1, "nvram read pos %d len %d", (FMT__D_D, cur_pos, len));
  SUPPRESS_UNUSED_RETURN(fread(buf, len, 1, nvram_file));

  return RET_OK;
}

zb_ret_t zb_osif_nvram_write(zb_uint8_t page, zb_uint32_t pos, void *buf, zb_uint16_t len )
{
  zb_int32_t cur_pos;

  if(page>=ZB_NVRAM_PAGE_COUNT)
  {
    return RET_PAGE_NOT_FOUND;
  }
  if(pos+len>ZB_NVRAM_PAGE_SIZE)
  {
    return RET_INVALID_PARAMETER;
  }

  if (buf == NULL)
  {
    return RET_INVALID_PARAMETER_3;
  }

  if (len == 0)
  {
    return RET_INVALID_PARAMETER_4;
  }

  if (zb_osif_nvram_last_operation != ZB_OSIF_NVRAM_LAST_OPERATION_WRITE)
  {
    zb_osif_nvram_last_operation = ZB_OSIF_NVRAM_LAST_OPERATION_WRITE;
    zb_osif_nvram_flush();
  }

  cur_pos = ftell(nvram_file);

  if ((zb_uint32_t)cur_pos != page*ZB_NVRAM_PAGE_SIZE+pos)
  {
    fseek(nvram_file, page*ZB_NVRAM_PAGE_SIZE+pos, SEEK_SET);
    cur_pos = page*ZB_NVRAM_PAGE_SIZE+pos;
  }

#ifdef ZB_STACK_REGRESSION_TESTING_API
  if (ZB_REGRESSION_TESTS_API().perform_crash_on_nvram_write)
  {
    /* Perform crash manually */
    TRACE_MSG(TRACE_ERROR, "Aborted on nvram write, written len %d", (FMT__D, 0));
    ZB_ABORT();
  }
#endif

#ifdef ZB_STACK_REGRESSION_TESTING_API
  if (ZB_REGRESSION_TESTS_API().perform_crash_on_nvram_write)
  {
    /* Perform crash manually */
    TRACE_MSG(TRACE_ERROR, "Aborted on nvram write, written len %d", (FMT__D, 0));
    ZB_ABORT();
  }
#endif

  TRACE_MSG(TRACE_COMMON1, "nvram write pos %d len %d", (FMT__D_D, cur_pos, len));
  fwrite(buf, len, 1, nvram_file);

//  fwrite(buf, len, 1, nvram_file_raw);
  /* Moved fsync to zb_osif_nvram_flush because it is tooo slow even at PC */

  return RET_OK;
}


zb_ret_t zb_osif_nvram_erase_async(zb_uint8_t page)
{
  zb_uint8_t buf[ZB_NVRAM_PAGE_SIZE];

  if(page>=ZB_NVRAM_PAGE_COUNT)
  {
    return RET_PAGE_NOT_FOUND;
  }

  fseek(nvram_file, page*ZB_NVRAM_PAGE_SIZE, SEEK_SET);
  ZB_MEMSET(buf, 0xFF, ZB_NVRAM_PAGE_SIZE);
  fwrite(buf, ZB_NVRAM_PAGE_SIZE, 1, nvram_file);
  zb_osif_nvram_last_operation = ZB_OSIF_NVRAM_LAST_OPERATION_WRITE;

  /* Really, erase is synchronous for file based nvram */
  zb_nvram_erase_finished(page);

  return RET_OK;
}

void zb_osif_nvram_flush()
{
  fflush(nvram_file);
#ifndef ZB_NSNG
  /* removed fsync from nsng builds: too slow and not erally neccary for nsng */
  fsync(fileno(nvram_file));
#endif
//  fflush(nvram_file_raw);
//  fsync(fileno(nvram_file_raw));
}

void zb_osif_nvram_wait_for_last_op()
{
  zb_osif_nvram_flush();
}

zb_ret_t zb_osif_erase_nvram_trace_sector()
{
  /* TODO implement it correctly. */
  return RET_OK;
}

zb_ret_t zb_osif_bl_app_flash_read(zb_uint32_t address, zb_uint8_t *buf, zb_uint16_t len)
{
  ZVUNUSED(address);
  ZVUNUSED(buf);
  ZVUNUSED(len);
  /* TODO implement it correctly. */
  return RET_OK;
}

zb_ret_t zb_osif_bl_app_flash_write(zb_uint32_t address, zb_uint8_t *buf, zb_uint16_t len)
{
  ZVUNUSED(address);
  ZVUNUSED(buf);
  ZVUNUSED(len);
  /* TODO implement it correctly. */
  return RET_OK;
}


#endif  /* defined ZB_USE_NVRAM || defined DOXYGEN */

#ifdef ZB_USE_OSIF_OTA_ROUTINES
/* OTA stubs for Unix */
void zb_osif_ota_read(void *dev, zb_uint8_t *data, zb_uint32_t addr, zb_uint32_t size)
{
  /* TODO: implement */
  ZVUNUSED(dev);
  ZVUNUSED(data);
  ZVUNUSED(addr);
  ZVUNUSED(size);
}

void zb_osif_ota_write(void *dev, zb_uint8_t *data, zb_uint_t off, zb_uint_t size, zb_uint32_t image_size)
{
  /* TODO: implement */
  ZVUNUSED(dev);
  ZVUNUSED(data);
  ZVUNUSED(off);
  ZVUNUSED(size);
  ZVUNUSED(image_size);
}

void zb_sec_b6_hash_iter_done(void *dev, zb_uint32_t input_len, zb_uint8_t *calc_hash)
{
  /* TODO: implement */
  ZVUNUSED(dev);
  ZVUNUSED(input_len);
  ZVUNUSED(calc_hash);

}

zb_bool_t zb_osif_ota_fw_size_ok(zb_uint32_t image_size)
{
  ZVUNUSED(image_size);
  /* TODO: implement */
  return ZB_FALSE;
}

void *zb_osif_ota_open_storage()
{
  /* handle of storage */
  return NULL;
}

void zb_osif_ota_close_storage(void *dev)
{
  ZVUNUSED(dev);
}

void zb_osif_ota_mark_fw_absent()
{
  /* Nothing to do here */
}

void zb_osif_ota_erase_fw(void *dev, zb_uint_t offset, zb_uint32_t size)
{
  ZVUNUSED(dev);
  ZVUNUSED(offset);
  ZVUNUSED(size);
}

zb_bool_t zb_osif_ota_verify_integrity(void *dev, zb_uint32_t raw_len)
{
  ZVUNUSED(dev);
  ZVUNUSED(raw_len);
  return ZB_FALSE;
}

zb_bool_t zb_osif_ota_verify_integrity_async(void *dev, zb_uint32_t raw_len)
{
  ZVUNUSED(dev);
  ZVUNUSED(raw_len);
  return ZB_FALSE;
}

void zb_osif_ota_mark_fw_ready(void *ota_dev, zb_uint32_t size, zb_uint32_t revision)
{
  ZVUNUSED(ota_dev);
  ZVUNUSED(size);
  ZVUNUSED(revision);
}

#endif

/*! @} */
