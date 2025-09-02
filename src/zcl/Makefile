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

SRCS = zcl_main.c zcl_general_commands.c zcl_attr_value.c zcl_common.c zcl_nvram.c \
       zcl_reporting.c zcl_identify_commands.c zcl_on_off_commands.c \
       ha_sas.c zcl_groups.c zcl_scenes.c zcl_level_control_commands.c \
       zcl_shade_config_commands.c zcl_continuous_value_change_commands.c \
       zcl_color_control_commands.c zcl_poll_control_commands.c zcl_poll_control_client.c \
       zcl_alarms_commands.c zcl_power_config_commands.c zcl_diagnostics_commands.c \
       zcl_ias_ace_commands.c zcl_ias_wd_commands.c zcl_ias_zone_commands.c \
       zcl_ota_upgrade_commands.c zcl_ota_upgrade_srv_commands.c \
       zcl_ota_upgrade_common.c zcl_ota_upgrade_minimal.c \
       zcl_basic_commands.c zcl_temp_measurement.c \
       zcl_thermostat_commands.c zcl_tunnel.c zcl_time.c \
       zcl_en50523_appliance_events_and_alerts.c zcl_c_tunneling.c zcl_s_tunneling.c \
       zcl_dehumid_control.c zcl_door_lock.c zcl_el_measurement.c \
       zcl_fan_control.c zcl_rel_humidity.c \
       zcl_thermostat_ui_config.c zcl_window_covering.c \
       zcl_on_off_switch_config.c zcl_illuminance_measurement.c \
       zcl_c_drlc.c zcl_s_drlc.c zcl_c_price.c zcl_s_price.c zcl_prepayment.c \
       zcl_c_calendar.c zcl_s_calendar.c zcl_c_energy_mgmt.c zcl_s_energy_mgmt.c \
       zcl_c_messaging.c zcl_s_messaging.c zcl_c_events.c zcl_s_events.c \
       zcl_c_subghz.c zcl_s_subghz.c zcl_c_mdu_pairing.c zcl_s_mdu_pairing.c \
       zcl_meter_identification.c zcl_s_metering.c zcl_c_metering.c zcl_device_management.c \
       zcl_binary_input.c zcl_ir_blaster.c zcl_c_wwah.c zcl_s_wwah.c zcl_wwah_common.c \
       zcl_control4_networking.c
# zcl_door_lock_commands.c zcl_custom_attr.c
      # zcl_custom_attr.c \

all: $(ZCL_LIB) $(ZCL_LIB_ED)

$(ZCL_LIB) : $(OBJS)
	$(MAKE_LIB)

$(ZCL_LIB_ED) : $(OBJS_ED)
	$(MAKE_LIB)

else

all:

endif
