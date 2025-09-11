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
# PURPOSE: root ZBOSS Makefile. Usually compile ZBOSS libs.
# Type 'make rebuild' to build first time.

include ../Platform_config
# Some defaults to variables {not} defined in Platform_config
#FEATURESET ?= default
#FIND ?= find
#LN ?= ln -fs

# Must not include Options here!
.PHONY: tags
.PHONY: zbosses

all: bldall

clean: cleandep

bldall clean depend doclean:
	cd common                    && $(MAKE) $@
	cd osif                      && $(MAKE) $@
	cd aps                       && $(MAKE) $@
	cd zdo                       && $(MAKE) $@
	cd commissioning/legacy      && $(MAKE) $@
	cd commissioning/bdb         && $(MAKE) $@
	cd commissioning/se          && $(MAKE) $@
	cd zcl                       && $(MAKE) $@
	cd secur                     && $(MAKE) $@
	cd ncp                       && $(MAKE) $@

bldall : deps

deps:
	> deps
	make depend

cleandep :
	> deps

tags:
	($(FIND) . -name \*\.[ch] -print) | grep -v ".*~"| grep -v tests|grep -v build | etags -

ctags:
	ctags -R -h "+.inl.INL" --languages=C,C++ --langmap=C:+.inl.INL,C:.c.C.h.H --c-kinds=+p \
		--fields=+iaS --extra=+q --exclude="build" -I ZB_CALLBACK,ZB_SDCC_REENTRANT,ZB_SDCC_BANKED .

rebuild: clean depend all

zbosses:
	make -f Makefile_zboss zboss_libs
