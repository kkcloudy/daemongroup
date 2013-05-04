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
# File version info: $Id: Makefile 26394 2007-07-25 02:27:17Z rfranz $

#OCTEON_ROOT = /opt/CN/OCTEON-SDK/
#OCTEON_ROOT = /opt/CN/SDK1.8.1/
#OCTEON_ROOT = /opt/CN/SDK2.2/
#OCTEON_ROOT = /home/sunjianchao/SDK1.8.1

OCTEON_CPPFLAGS_GLOBAL_ADD=-DUSE_RUNTIME_MODEL_CHECKS=1

OCTNIC_DIR = ../../../oct-nic
PCI_DRIVER_DIR = $(OCTNIC_DIR)/components/driver
LIBS_LIST += $(PCI_DRIVER_DIR)/bin/liboctapi.a -lpthread

CFLAGS_LOCAL = -g -O2 -W -Wall -Wno-unused \
				-I$(ACCAPI_DIR) \
				-I$(PCI_DRIVER_DIR)/host/include \
				-I$(PCI_DRIVER_DIR)/host/api \
				-I$(PCI_DRIVER_DIR)/common

CFLAGS_LOCAL += -Werror
CFLAGS_LOCAL += -DSDK_VERSION_2_2


all: check

#  standard common Makefile fragment

include $(OCTEON_ROOT)/common.mk

#  include relevant component Makefile fragments

dir := $(OCTEON_ROOT)/executive
include $(dir)/cvmx.mk

check: application-target

TARGET = se_agent
OBJS   = $(OBJ_DIR)/se_agent.o \
		$(OBJ_DIR)/se_agent_log.o \
		$(OBJ_DIR)/se_agent_acl.o \
		$(OBJ_DIR)/se_agent_user.o \
		$(OBJ_DIR)/se_agent_octnic.o

include $(OCTEON_ROOT)/application.mk


clean:
	rm -f $(TARGET)  *.log queue-*
	rm -fr $(OBJ_DIR) $(OBJ_DIR)-*
	rm -f $(CVMX_CONFIG)

run: $(TARGET)
	oct-sim $(TARGET) -quiet -noperf
