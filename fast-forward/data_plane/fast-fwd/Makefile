######################*license start*###################################
# OCTEON SDK
# 
# Copyright (c) 2003-2005 Cavium Networks. All rights reserved.
# 
# This file, which is part of the OCTEON SDK from Cavium Networks,
# contains proprietary and confidential information of Cavium Networks and
# its suppliers.
# 
# Any licensed reproduction, distribution, modification, or other use of
# this file or the confidential information or patented inventions
# embodied in this file is subject to your license agreement with Cavium
# Networks. Unless you and Cavium Networks have agreed otherwise in
# writing, the applicable license terms can be found at:
# licenses/cavium-license-type2.txt
# 
# All other use and disclosure is prohibited.
# 
# Contact Cavium Networks at info@caviumnetworks.com for more information.
#####################*license end*#####################################/
#
#  fast-fwd Makefile lutao
#
OCTEON_ROOT = /opt/CN/SDK2.2

OCTEON_CPPFLAGS_GLOBAL_ADD=-DUSE_RUNTIME_MODEL_CHECKS=1
OCTEON_INC = .

DRIVER_ROOT = $(OCTEON_ROOT)/components/driver

MODE=${OCTEON_MODEL}
OLD_MODE=${OCTEON_MODEL}

ifeq ($(PREFIX), _68)
export OCTEON_MODEL=OCTEON_CN68XX 
endif


#  default target

default: diag application-target

diag:
	@echo "current OCTEON_ROOT is"$(OCTEON_ROOT)

#  standard common Makefile fragment

include $(OCTEON_ROOT)/common.mk

#  include needed component Makefile fragments

#dir := $(OCTEON_ROOT)/executive
dir := ./components/executive_sdk2.2
include $(dir)/cvmx.mk

dir := $(OCTEON_ROOT)/components/common
include $(dir)/common.mk

dir := $(OCTEON_ROOT)/components/driver
include $(dir)/driver.mk

COMPILE += $(OCTDRVFLAGS)

#  include required application common
dir := $(OCTEON_ROOT)/applications/pci-core-app/common
include $(dir)/cvmcs-common.mk

#  include required Core driver Makefile fragments
dir := $(DRIVER_ROOT)/core
include $(dir)/core-drv.mk

#  application specification

TARGET := fastfwd$(PREFIX)

VPATH = $(OCTEON_INC)/components/common \
		$(OCTEON_INC)/components/shell \
		$(OCTEON_INC)/components/protocal \
		$(OCTEON_INC)/components/system \
		$(OCTEON_INC)/components/nic \
		$(OCTEON_INC)/components/debug
		
OBJS = $(OBJ_DIR)/builddate.o $(OBJ_DIR)/fwd_main.o $(OBJ_DIR)/fastfwd-common.o $(OBJ_DIR)/fastfwd-common-utils.o \
			$(OBJ_DIR)/readcmd.o $(OBJ_DIR)/shell.o $(OBJ_DIR)/shell_common.o $(OBJ_DIR)/uart.o $(OBJ_DIR)/autelan_product_info.o \
            $(OBJ_DIR)/acl.o $(OBJ_DIR)/car.o $(OBJ_DIR)/traffic_monitor.o $(OBJ_DIR)/fwd_rpa.o \
	    $(OBJ_DIR)/cvmcs-nic-api.o $(OBJ_DIR)/cvmcs-nic-init.o $(OBJ_DIR)/cvmcs-nic.o $(OBJ_DIR)/cvm_ip_reass.o $(OBJ_DIR)/cvm_ratelimit.o $(OBJ_DIR)/equipment_test.o \
	    $(OBJ_DIR)/fwd_debug.o


CFLAGS_LOCAL =  -I$(OCTEON_INC)/components/common \
		-I$(OCTEON_INC)/components/shell \
		-I$(OCTEON_INC)/components/system \
		-I$(OCTEON_INC)/components/protocal \
		-I$(OCTEON_INC)/components/nic \
		-I$(OCTEON_INC)/components/debug \
		-I$(OCTEON_INC) \
		-I$(OCTEON_ROOT)/applications/pci-core-app/common \
		-I$(DRIVER_ROOT)/common \
		-g -O2 -W -Wall -Wno-unused-parameter -Wno-missing-field-initializers \
		-DOCTEON_DEBUG_LEVEL -DCVM_ACL -DCVM_CAR -DUSER_TABLE_FUNCTION 

CFLAGS_LOCAL += -Werror
CFLAGS_LOCAL += -DSDK_VERSION_2_2


# make standalone mode
#ifdef FWDMODE_STANDALONE
#CFLAGS_LOCAL += -DFWDMODE_STANDALONE
#endif

include $(OCTEON_ROOT)/application.mk

#  clean target

clean:
	rm -f $(TARGET) config/cvmx-config.h
	rm -fr $(OBJ_DIR)
	rm -f $(TARGET)-strip

cleanobj:
	@echo "clean obj, OBJ_DIR is "$(OBJ_DIR)
	rm -rf $(OBJ_DIR) config/cvmx-config.h

strip: 
	mipsisa64-octeon-elf-strip $(TARGET) -o $(TARGET)
