#/*
# * ZBOSS Zigbee software protocol stack
# *
# * Copyright (c) 2012-2024 DSR Corporation, Denver CO, USA.
# * http://www.dsr-zboss.com
# * http://www.dsr-corporation.com
# * All rights reserved.
# *
# *
# * Use in source and binary forms, redistribution in binary form only, with
# * or without modification, are permitted provided that the following conditions
# * are met:
# *
# * 1. Redistributions in binary form, except as embedded into a Nordic
# *    Semiconductor ASA integrated circuit in a product or a software update for
# *    such product, must reproduce the above copyright notice, this list of
# *    conditions and the following disclaimer in the documentation and/or other
# *    materials provided with the distribution.
# *
# * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
# *    contributors may be used to endorse or promote products derived from this
# *    software without specific prior written permission.
# *
# * 3. This software must only be used in or with a processor manufactured by Nordic
# *    Semiconductor ASA, or in or with a processor manufactured by a third party that
# *    is used in combination with a processor manufactured by Nordic Semiconductor.
# *
# * 4. Any software provided in binary form under this license must not be reverse
# *    engineered, decompiled, modified and/or disassembled.
# *
# * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
# * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
# * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
# * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# */
# PURPOSE:

BUILD_HOME=../..
include $(BUILD_HOME)/src/Options

ifneq ($(NOZigbee),y)
SRCS = zb_debug.c zb_scheduler.c zb_address.c zb_init_common.c zb_init_ncp.c \
    zb_scheduler_init.c zb_time.c zb_random.c zb_file_trace.c zb_watchdog.c \
    zb_error_indication.c zb_sleep.c zb_led_button.c \
    zb_24bit_math.c zb_48bit_math.c \
    zb_trace_common.c zb_channel_page.c zb_globals.c zb_bufpool_mult.c zb_bufpool_mult_storage.c \
    zb_signal_common.c
else
SRCS_U = zb_debug.c zb_scheduler_stubs.c zb_random.c zb_file_trace.c zb_trace_common.c \
 zb_nvram_trace.c zb_time.c zb_nvram.c \
zb_error_indication.c
OBJS_U = ${SRCS_U:.c=.u.o}
endif

SRCS_T = zb_debug.c zb_scheduler.c zb_init_tool.c \
    zb_scheduler_init.c zb_time.c zb_random.c zb_file_trace.c zb_watchdog.c \
    zb_fsm.c zb_error_indication.c \
    zb_file_trace_bin.c zb_trace_common.c zb_globals.c zb_bufpool_new_adapters.c zb_bufpool_mult.c zb_bufpool_mult_storage.c zb_error_indication.c
OBJS_T = ${SRCS_T:.c=.t.o}

all: $(COMMON_LIB) $(COMMON_LIB_ED)

zb_nvram.u.o: zb_nvram.c
	$(CC) -c $(CFLAGS) $(DEFINES) -DAPP_ONLY_NVRAM $(INCPATH) -o $@ $<

zb_nvram.t.o: zb_nvram.c
	$(CC) -c $(CFLAGS) $(DEFINES) -DAPP_ONLY_NVRAM $(INCPATH) -o $@ $<

$(COMMON_LIB) : $(OBJS)
	$(MAKE_LIB)

$(COMMON_LIB_ED) : $(OBJS_ED)
	$(MAKE_LIB)
