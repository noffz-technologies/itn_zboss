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
# PURPOSE: Makefile for NCP FW and related tests.


BUILD_HOME=../..
include $(BUILD_HOME)/src/Options

#Low level
LL_SRCS = \
  low_level/zbncp_ll_impl.c \
  low_level/zbncp_tr_impl.c \
  low_level/zbncp_node.c \
  low_level/zbncp_mem.c \
  low_level/zbncp_utils.c \
  low_level/zbncp_debug.c \
  low_level/zbncp_divider.c \
  low_level/zbncp_fragmentation.c \
  low_level/zbncp_reassembling.c


#NSNG transport
TR_NSNG_HOME = transport/nsng
TR_NSNG_HOST_SRCS = \
  $(TR_NSNG_HOME)/zb_ncp_tr_host_nsng.c

TR_SERIAL_SRCS = transport/osif/serial/zb_ncp_tr_osif_serial.c

#HOST
HOST_HL_HOME = high_level/host

HOST_HL_SRCS_BASE = \
  $(HOST_HL_HOME)/ncp_host_hl_proto.c \
  $(HOST_HL_HOME)/ncp_host_hl_buffers.c \
  $(HOST_HL_HOME)/ncp_host_soc_state.c \
  $(HOST_HL_HOME)/ncp_host_startup.c \
  $(HOST_HL_HOME)/ncp_host_transport_stubs.c \
  $(HOST_HL_HOME)/zb_ncp_ll_host.c

HOST_HL_TRANSPORT_HOME = $(HOST_HL_HOME)/transport
HOST_HL_SRCS_TRANSPORT = \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_configuration.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_af.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_zdo.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_aps.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_nwkmgmt.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_secur.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_manuf_test.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_ota.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_intrp.c \
  $(HOST_HL_TRANSPORT_HOME)/ncp_host_hl_proto_vendor.c \

HOST_HL_ADAPTERS_HOME = $(HOST_HL_HOME)/adapters
HOST_HL_SRCS_ADAPTERS = \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_aps.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_nwkmgmt.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_zdo.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_secur.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_nvram.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_configuration.c \
  $(HOST_HL_ADAPTERS_HOME)/ncp_host_hl_adapter_intrp.c \

HOST_HL_SRCS = \
  $(HOST_HL_SRCS_BASE) \
  $(HOST_HL_SRCS_TRANSPORT) \
  $(HOST_HL_SRCS_ADAPTERS)


HOST_SRCS = \
  $(HOST_HL_SRCS) \
  $(LL_SRCS) \
  $(HOST_HL_HOME)/ncp_host_stubs.c

HOST_ZB_LIBS = \
  $(COMMON_LIB) \
  $(MAC_LIB) \
  $(OSIF_LIB)

# shared libs are using object files with a suffix .shared.o
# see OBJS_SHARED and LINK_BINARY_SHARED_LIB in Options

SRCS = \
  $(HOST_SRCS) \
  $(TR_SERIAL_SRCS)

#/* 07/30/2020 EE CR:MAJOR Trace setting is a debug stuff. Put it into Options, or vendor file, or whatever */
# Pass LL trace into ZBOSS common trace file. FIXME: how to handle RPi .so lib?
CFLAGS += -DZBNCP_USE_ZBOSS_TRACE=1
CFLAGS += -DZBNCP_DEBUG=1
CFLAGS += -DDISABLE_TRACE_INIT=1

INCPATH += -I$(BUILD_HOME)/src/ncp -I$(BUILD_HOME)/src/ncp/high_level/common -I$(HOST_HL_HOME)

$(NCP_HOST_LIB) : $(OBJS)
	$(MAKE_LIB)

$(NCP_HOST_LIB_ED) : $(OBJS_ED)
	$(MAKE_LIB)

OTHERS_CLEAN += $(OBJS) $(OBJS_ED) $(NCP_HOST_LIB) $(NCP_HOST_LIB_ED)

all: $(NCP_HOST_LIB) $(NCP_HOST_LIB_ED)
