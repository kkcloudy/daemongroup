SHELL = /bin/bash

# If make with h=1, make it run on the host, i.e. i386 machine.
# Otherwise, take mips as the default.
ifdef h 

CC = gcc
DBUS_INCLUDES = $(shell pkg-config --cflags dbus-1)
DBUS_LIBS = $(shell pkg-config --libs dbus-1)


INCLUDES = -I/opt/cvs/userv/quagga/x86build -I/opt/cvs/userv/quagga/debsrc/quagga-0.99.5/ -I/opt/cvs/userv/quagga/debsrc/quagga-0.99.5/lib -I$(ACCAPI_DIR)

else


INCLUDES = -I$(QUAGGA_DIR)/${BUILDDIR} -I$(QUAGGA_DIR)/debsrc/quagga-0.99.5 -I$(QUAGGA_DIR)/debsrc/quagga-0.99.5/lib 
endif


INCLUDES += -I$(ACCAPI_DIR) -I$(BUILDROOTDIR)/usr/include -I$(BUILDROOTDIR)/usr/include/dbus-1.0 -I$(BUILDROOTDIR)/usr/lib/dbus-1.0/include

SNMP_PATH = ../../../AuteCS/cgic205/snmp_agent/
EAG_PATH = ../../../AuteCS/cgic205/portal_multi_ins/
CGI_PATH = ../../../AuteCS/cgic205/
DCLI_WIRELESS_PATH = ./dcli_wireless/
ACCAPIPATH=$(ACCAPI_DIR)

IBUS_PATH = ../../../ibus

INCLUDES += -I $(ACCAPIPATH)
INCLUDES += -I $(ACCAPIPATH)/nm/app/
INCLUDES += -I $(ACCAPIPATH)/nm/app/snmp/
INCLUDES += -I $(ACCAPIPATH)/nm/app/eag/
INCLUDES += -I $(ACCAPIPATH)/nm/app/drp/
INCLUDES += -I $(ACCAPIPATH)/nm/app/sample/
INCLUDES += -I $(ACCAPIPATH)/nm/app/manage/
INCLUDES += -I $(ACCAPIPATH)/nm/app/firewall/
INCLUDES += -I $(ACCAPIPATH)/nm/app/pppoe/
INCLUDES += -I $(ACCAPIPATH)/nm/ip/
INCLUDES += -I $(ACCAPIPATH)/nm/public/
INCLUDES += -I $(ACCAPIPATH)/nm/switch/             
INCLUDES += -I $(ACCAPIPATH)/nm/system/             
INCLUDES += -I $(ACCAPIPATH)/nm/wlan/ 
INCLUDES += -I $(ACCAPIPATH)/facl/
INCLUDES += -I../../../net-snmp/engine/include
INCLUDES += -I../../../AuteCS/cgic205/
INCLUDES += -I $(BUILD_DIR)/src/sem/src/dcli/  #wangchao add
INCLUDES += -I $(BUILD_DIR)/src/npdsuit_mv/npd/src/dcli/#wangchao add
INCLUDES += -I $(BUILD_DIR)/src/dg/wcpss/src/dcli/ #wangchao add

LD_PATH = -L${ROOTFS_DIR}/files/lib -L$(BUILDROOTDIR)/usr/lib -L../pub -L$(IBUS_PATH)/cli -L$(IBUS_PATH)/mapi -L../../../lib/nm 


DEFS = -DHAVE_CONFIG_H $(DCLI_HAS_WCPSS)
LDFLAGS = -lrt -ldclipub -ldcliiuhcli -ldcliiuhmapi -lnm -lxml2 -lz -lpam -shared -Wl,-soname,$@
#LDFLAGS = -lrt -ldclipub -ldcliiuhcli -ldcliiuhmapi -lnm -lxml2 -lz -lpam -shared -Wl,-soname,$@
CCFLAGS = -I../pub
CCFLAGS += -I$(IBUS_PATH)/cli
CCFLAGS += -I$(IBUS_PATH)/mapi
CCFLAGS += -D_DISTRIBUTION_
CCFLAGS += -D_$(AWVERSION)_
SNMPFLAGS = -I$(SNMP_PATH)
EAGFLAGS = -I$(EAG_PATH)
CGIFLAGS = -I$(CGI_PATH)
XMLFLAGS = -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
PUB_PATH = -L../pub
PUB_FLAGS = -ldclipub
DRPLIB_PATH = -L$(LIB_EXPORT_DIR)
DRPLIB_FLAGS = -ldrpintf
EAGLIB_PATH = -L$(LIB_EXPORT_DIR)
EAGLIB_FLAGS = -leagintf
FACLLIB_FLAGS = -lfaclintf -lfacl
PUB1_PATH = -L$(IBUS_PATH)/cli
PUB1_FLAGS = -ldcliiuhcli
PUB2_PATH = -L$(IBUS_PATH)/mapi
PUB2_FLAGS = -ldcliiuhmapi
DCLI_SHLIB_MAJOR = 0
DCLI_SHLIB_MINOR = 1

DBUS_LIBS = -ldbus-1
XML_LIBS = -lxml2


DCLI_SHLIB_LINKERNAME = libdcli.so
DCLI_SHLIB_SONAME = $(DCLI_SHLIB_LINKERNAME).$(DCLI_SHLIB_MAJOR)
DCLI_SHLIB_REALNAME = $(DCLI_SHLIB_SONAME).$(DCLI_SHLIB_MINOR)

$(shell [ rm -f $(EAG_PATH)user_manage.h ] || ln -s ../../../eag/pkg/src_multi_ins/user_manage.h $(EAG_PATH)user_manage.h)
EX_OBJ = $(CGI_PATH)ws_usrinfo.o  
#EX_OBJ = $(SNMP_PATH)ws_snmpd_engine.o $(SNMP_PATH)ws_snmpd_trap_common.o $(EAG_PATH)ws_user_manage.o $(CGI_PATH)ws_public.o $(EAG_PATH)ws_eag_conf.o \
#	$(CGI_PATH)ws_log_conf.o $(EAG_PATH)ws_conf_engine.o $(CGI_PATH)ws_firewall.o $(CGI_PATH)ws_tcrule.o  

DCLI_WIRELESS_OBJS = $(DCLI_WIRELESS_PATH)dcli_wqos.o  $(DCLI_WIRELESS_PATH)dcli_wsm.o  \
				$(DCLI_WIRELESS_PATH)dcli_local_hansi.o   $(DCLI_WIRELESS_PATH)dcli_wireless_main.o
# $(DCLI_WIRELESS_PATH)dcli_wbridge.o $(DCLI_WIRELESS_PATH)dcli_license.o $(DCLI_WIRELESS_PATH)dcli_bsd.o \ $(DCLI_WIRELESS_PATH)dcli_ap_group.o $(DCLI_WIRELESS_PATH)dcli_ac_roaming.o $(DCLI_WIRELESS_PATH)dcli_ac_ip_list.o\ $(DCLI_WIRELESS_PATH)dcli_wlan.o $(DCLI_WIRELESS_PATH)dcli_ebr.o $(DCLI_WIRELESS_PATH)dcli_ac.o $(DCLI_WIRELESS_PATH)dcli_sta.o\ $(DCLI_WIRELESS_PATH)dcli_security.o $(DCLI_WIRELESS_PATH)dcli_wtp.o $(DCLI_WIRELESS_PATH)dcli_radio.o


#DCLI_OBJS = dcli_main.o dcli_acl.o dcli_qos.o dcli_stp.o dcli_vlan.o dcli_trunk.o dcli_dynamic_trunk.o \
		dcli_fdb.o dcli_eth_port.o dcli_system.o dcli_intf.o dcli_common_stp.o dcli_routesyn.o \
		dcli_igmp_snp.o \
		dcli_common_igmp_snp.o dcli_user.o dcli_pvlan.o dcli_mirror.o dcli_boot.o \
		dcli_sys_manage.o dcli_diag.o dcli_tunnel.o dcli_dldp.o dcli_common_dldp.o  \
		dcli_vrrp.o dcli_dhcp_snp.o dcli_hbip.o dcli_iptunnel.o dcli_dhcp.o \
		dcli_pim.o dcli_domain.o dcli_captive.o dcli_acsample.o dcli_snmp.o dcli_eag.o dcli_pdc.o dcli_rdc.o dcli_facl.o \
		dcli_route_policy.o dcli_syslog.o dcli_firewall.o dcli_tc.o \
        dcli_dhcp6_conf.o dcli_mld_snp.o dcli_common_mld_snp.o dcli_prot_vlan.o dcli_spacial.o dcli_dhcp6.o dcli_radius.o dcli_debug.o dcli_sem.o  dcli_apache.o dcli_se_agent.o\
				dcli_ntp.o dcli_timezone.o dcli_pppoe.o dcli_cavium_ratelimit.o
                
DCLI_OBJS = dcli_main.o dcli_user.o dcli_sys_manage.o dcli_boot.o dcli_vrrp.o dcli_dhcp.o dcli_captive.o dcli_se_agent.o\
						dcli_eag.o dcli_pdc.o dcli_rdc.o dcli_domain.o dcli_rpa.o dcli_snmp.o dcli_acsample.o dcli_tc.o dcli_firewall.o dcli_strict_access.o dcli_apache.o dcli_md5.o dcli_cavium_ratelimit.o	

#DCLI_OBJS = dcli_main.o dcli_user.o dcli_sys_manage.o dcli_boot.o dcli_sem.o dcli_fpga.o dcli_vrrp.o dcli_dhcp.o dcli_captive.o dcli_se_agent.o\
#						dcli_eag.o dcli_pdc.o dcli_rdc.o dcli_domain.o dcli_rpa.o dcli_snmp.o dcli_acsample.o dcli_tc.o dcli_firewall.o dcli_strict_access.o dcli_apache.o dcli_md5.o dcli_cavium_ratelimit.o						

# npdsuit needed
DCLI_OBJS += dcli_intf.o dcli_eth_port.o dcli_npd_main.o dcli_system.o dcli_common_dldp.o dcli_debug.o \
			 dcli_dhcp_snp.o dcli_dhcp6.o dcli_ntp.o dcli_timezone.o dcli_syslog.o dcli_pppoe_snp.o dcli_pppoe.o dcli_radius.o dcli_acinfo.o dcli_facl.o
#dcli_mirror.o dcli_common_mld_snp.o  dcli_common_igmp_snp.o dcli_mld_snp.o dcli_igmp_snp.o dcli_prot_vlan.o dcli_pvlan.o dcli_tunnel.o dcli_routesyn.o dcli_fdb.o dcli_diag.o   dcli_common_stp.o dcli_vlan.o dcli_trunk.o dcli_dynamic_trunk.o dcli_qos.o dcli_acl.o dcli_stp.o
			 
#error: dcli_dhcp_snp.o undefined symbol: ReInitDbusConnection
			 
DCLI_OBJS += $(DCLI_WIRELESS_OBJS)
DCLI_OBJS += $(EX_OBJ) 
		
.PHONY: all
all:  libdclicli $(DCLI_SHLIB_REALNAME)

libdclicli: libdclimapi
	$(MAKE) -C $(IBUS_PATH)/cli

libdclimapi:
	@echo "Building dcliiuhmapi module..."
	$(MAKE) -C $(IBUS_PATH)/mapi
.PHONY: clean

clean: cleaniuh_cli
	$(RM) *.o $(DCLI_WIRELESS_PATH)*.o $(DCLI_SHLIB_LINKERNAME) $(DCLI_SHLIB_SONAME) $(DCLI_SHLIB_REALNAME)
	
cleaniuh_cli: cleaniuh_mapi
	$(MAKE) -C $(IBUS_PATH)/cli clean
	
cleaniuh_mapi: 
	$(MAKE) -C $(IBUS_PATH)/mapi clean

$(DCLI_SHLIB_REALNAME): $(DCLI_OBJS) 
	$(RM) $(LIB_EXPORT_DIR)/$(DCLI_SHLIB_REALNAME) $(LIB_EXPORT_DIR)/$(DCLI_SHLIB_SONAME) $(LIB_EXPORT_DIR)/$(DCLI_SHLIB_LINKERNAME)
	$(CC) ${LDFLAGS} $(LD_PATH) -o $@ $^ $(DBUS_LIBS) $(CCFLAGS) $(PUB_PATH) $(PUB_FLAGS) $(EAGLIB_PATH) $(EAGLIB_FLAGS) $(FACLLIB_FLAGS) $(DRPLIB_PATH) $(DRPLIB_FLAGS) $(PUB1_PATH) $(PUB1_FLAGS) $(PUB2_PATH) $(PUB2_FLAGS) $(SNMPFLAGS) $(EAGFLAGS) $(COMFLAGS) $(XML_LIBS) $(XMLFLAGS)  $(CGIFLAGS)
	mv $(DCLI_SHLIB_REALNAME) $(LIB_EXPORT_DIR)/
	ln -sf $(DCLI_SHLIB_REALNAME) $(LIB_EXPORT_DIR)/$(DCLI_SHLIB_SONAME) 
	ln -sf $(DCLI_SHLIB_SONAME) $(LIB_EXPORT_DIR)/$(DCLI_SHLIB_LINKERNAME) 


%.o: %.c
	${RM} $@
	$(CC) -c $(CCFLAGS) $(DEFS)  $(INCLUDES) $(SNMPFLAGS) $(EAGFLAGS) $(COMFLAGS) $(CFLAGS) $(CGIFLAGS) $(XML_LIBS) $(XMLFLAGS) -o $*.o $<
