CC = mips-linux-gnu-gcc
LDFLAGS = -lpthread -ldl -D_REENTRANT -L ../../../export/opt/lib 
LIBS = -L${AUTEWAREFS_DIR}/files/opt/lib -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz -ldbus-1 -L ../../lib/nm -lnm -L ../../dcli/src/pub -ldclipub
LDFLAGS += -Wl,--gc-sections
OBJCOPY=mips-linux-gnu-objcopy


ACCAPIPATH=../../accapi/

CFLAGS += -I../engine/include -Dlinux -I.
CFLAGS += -I/opt/mipslibs/usr/include/libxml2 -I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
CFLAGS += -I $(ACCAPIPATH)
CFLAGS += -I $(ACCAPIPATH)nm/app/
CFLAGS += -I $(ACCAPIPATH)nm/app/snmp/
CFLAGS += -I $(ACCAPIPATH)nm/app/manage/
CFLAGS += -I $(ACCAPIPATH)nm/app/sample/
CFLAGS += -I $(ACCAPIPATH)nm/app/eag/
CFLAGS += -I $(ACCAPIPATH)nm/app/firewall/
CFLAGS += -I $(ACCAPIPATH)nm/ip/
CFLAGS += -I $(ACCAPIPATH)nm/public/
CFLAGS += -I $(ACCAPIPATH)nm/switch/             
CFLAGS += -I $(ACCAPIPATH)nm/system/             
CFLAGS += -I $(ACCAPIPATH)nm/wlan/ 
#CFLAGS += -I ../../AuteCS/cgic205/
#CFLAGS += -I ../subagent/
ENGINELIBDIR="../engine/lib"
BUILDAGENTLIBS=-L../engine/lib -lnetsnmpmibs -lnetsnmpagent -lnetsnmphelpers -lnetsnmp -lm -ldl
BUILDAGENTLIBS+= -lsensors
BUILDAGENTLIBS+= -lwrap -lcrypto -lz -lsysfs

CFLAGS += -ffunction-sections

DBUS_OBJ_PATH=../../AuteCS/cgic205/
#ALL_DBUS_OBJS = $(DBUS_OBJ_PATH)ws_sysinfo.o $(DBUS_OBJ_PATH)ws_init_dbus.o $(DBUS_OBJ_PATH)ws_dcli_ac.o \
#$(DBUS_OBJ_PATH)ws_dcli_wlans.o $(DBUS_OBJ_PATH)ws_sta.o $(DBUS_OBJ_PATH)ws_dcli_bss.o $(DBUS_OBJ_PATH)ws_dcli_wqos.o \
#$(DBUS_OBJ_PATH)ws_dcli_portconf.o $(DBUS_OBJ_PATH)ws_security.o $(DBUS_OBJ_PATH)ws_nm_status.o \
#$(DBUS_OBJ_PATH)snmp_agent/ws_snmpd_engine.o $(DBUS_OBJ_PATH)ws_log_conf.o $(DBUS_OBJ_PATH)snmp_agent/ws_snmpd_trap_common.o

ALL_LOCAL_C := $(shell ls *.c)
ALL_LOCAL_OBJS = $(ALL_LOCAL_C:.c=.o)
#SNMP_PUBLIC_API_OBJS = ../subagent/mibs_public.o
ALL_OBJS =$(ALL_LOCAL_OBJS)

TAGETS = trap-helper

makelink:
	-ln -s libnetsnmpagent.so.20.0.0 ${ENGINELIBDIR}/libnetsnmpagent.so.20
	-ln -s libnetsnmpagent.so.20 ${ENGINELIBDIR}/libnetsnmpagent.so
	-ln -s libnetsnmphelpers.so.20.0.0 ${ENGINELIBDIR}/libnetsnmphelpers.so.20
	-ln -s libnetsnmphelpers.so.20 ${ENGINELIBDIR}/libnetsnmphelpers.so
	-ln -s libnetsnmpmibs.so.20.0.0 ${ENGINELIBDIR}/libnetsnmpmibs.so.20
	-ln -s libnetsnmpmibs.so.20 ${ENGINELIBDIR}/libnetsnmpmibs.so
	-ln -s libnetsnmptrapd.so.20.0.0 ${ENGINELIBDIR}/libnetsnmptrapd.so.20
	-ln -s libnetsnmptrapd.so.20 ${ENGINELIBDIR}/libnetsnmptrapd.so
	-ln -s libnetsnmp.so.20.0.0 ${ENGINELIBDIR}/libnetsnmp.so.20
	-ln -s libnetsnmp.so.20 ${ENGINELIBDIR}/libnetsnmp.so
$(TAGETS): $(ALL_OBJS)
	make makelink
	 $(CC) $(ALL_OBJS) $(BUILDAGENTLIBS) $(LDFLAGS) $(LIBS)  -o $(TAGETS)_temp
	 $(OBJCOPY) -S --strip-all $(TAGETS)_temp $@;rm -f $(TAGETS)_temp
	 
%.o: %.c
	$(CC) -c $(CFLAGS)  -o $*.o $<
clean:
	$(RM) $(TAGETS) *.o
