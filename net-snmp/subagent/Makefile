#******************************************************************************
#opyright (C) Autelan Technology


#his software file is owned and distributed by Autelan Technology 
#*******************************************************************************


#HIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#NY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#ARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#ISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
#NY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#OSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#NY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#OFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#*******************************************************************************
# Makefile
#
#
# CREATOR:
# autelan.software.Network Dep. team
#
# DESCRIPTION:
# subagent's Makefile
#
#
#******************************************************************************/

CC=mips-linux-gnu-gcc
OBJCOPY=mips-linux-gnu-objcopy

#OBJS2=example-demon.o autelanAcGroup.o
#TARGETS=example-demon 

#CFLAGS=-I. `net-snmp-config --cflags`
#BUILDLIBS= `net-snmp-config --libs`
#BUILDAGENTLIBS= `net-snmp-config --agent-libs`

#CFLAGS=-I/usr/inlucde
#BUILDLIBS=-L/usr/lib -lnetsnmp
#BUILDAGENTLIBS=-L/usr/lib -lnetsnmp -lnetsnmpagent

# shared library flags (assumes gcc)
#DLFLAGS=-fPIC -shared

LDFLAGS += -Wl,--gc-sections -L ../../lib/nm -lnm -L$(LIB_EXPORT_DIR) -lmanage -leagintf -ldrpintf

CFLAGS += -ffunction-sections

CFLAGS += -I../engine/include -Dlinux -I.
CFLAGS += -I/opt/mipslibs/usr/include/libxml2 -I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
ENGINELIBDIR="../engine/lib"
BUILDAGENTLIBS=-L../engine/lib -lnetsnmpmibs -lnetsnmpagent -lnetsnmphelpers -lnetsnmp -lm -ldl
BUILDAGENTLIBS+= -lsensors
BUILDAGENTLIBS+= -lwrap -lcrypto -lz -lsysfs
CFLAGS += -I ../../accapi
CFLAGS += -I ../../AuteCS/cgic205/
CFLAGS += -I ../../AuteCS/cgic205/snmp_agent
CFLAGS += -I ../../AuteCS/cgic205/portal_multi_ins
CFLAGS += -I ../../eag/pkg/src/

ACCAPIPATH=../../accapi/
CFLAGS += -I $(ACCAPIPATH)
CFLAGS += -I $(ACCAPIPATH)nm/app/
CFLAGS += -I $(ACCAPIPATH)nm/app/snmp/
CFLAGS += -I $(ACCAPIPATH)nm/app/eag/
CFLAGS += -I $(ACCAPIPATH)nm/app/drp/
CFLAGS += -I $(ACCAPIPATH)nm/app/firewall/
CFLAGS += -I $(ACCAPIPATH)nm/app/manage/
CFLAGS += -I $(ACCAPIPATH)nm/app/sample/
CFLAGS += -I $(ACCAPIPATH)nm/app/dhcp/
CFLAGS += -I $(ACCAPIPATH)nm/ip/
CFLAGS += -I $(ACCAPIPATH)nm/public/
CFLAGS += -I $(ACCAPIPATH)nm/switch/             
CFLAGS += -I $(ACCAPIPATH)nm/system/             
CFLAGS += -I $(ACCAPIPATH)nm/wlan/ 


DBUS_OBJ_PATH=../../AuteCS/cgic205/
ALL_DBUS_OBJS = $(DBUS_OBJ_PATH)ws_dcli_vlan.o
#$(DBUS_OBJ_PATH)ws_dcli_wlans.o $(DBUS_OBJ_PATH)ws_sta.o $(DBUS_OBJ_PATH)ws_dcli_bss.o $(DBUS_OBJ_PATH)ws_dcli_wqos.o \
#$(DBUS_OBJ_PATH)ws_dcli_portconf.o $(DBUS_OBJ_PATH)ws_security.o $(DBUS_OBJ_PATH)ws_nm_status.o \
#$(DBUS_OBJ_PATH)snmp_agent/ws_snmpd_engine.o $(DBUS_OBJ_PATH)snmp_agent/ws_snmpd_trap_common.o \
#$(DBUS_OBJ_PATH)ws_log_conf.o $(DBUS_OBJ_PATH)ws_stp.o $(DBUS_OBJ_PATH)ws_version_param.o $(DBUS_OBJ_PATH)ws_tcrule.o \
#$(DBUS_OBJ_PATH)ws_dhcp_conf.o $(DBUS_OBJ_PATH)ws_public.o $(DBUS_OBJ_PATH)portal/ws_user_manage.o $(DBUS_OBJ_PATH)ws_returncode.o \
#$(DBUS_OBJ_PATH)portal/ws_conf_engine.o $(DBUS_OBJ_PATH)portal_multi_ins/ws_eag_conf.o $(DBUS_OBJ_PATH)ws_dcli_ac_roaming.o $(DBUS_OBJ_PATH)ws_portal_conf.o $(DBUS_OBJ_PATH)ws_dcli_dhcp.o

ALL_LOCAL_C := $(shell find ./ -name '*.c')
ALL_LOCAL_OBJS = $(ALL_LOCAL_C:.c=.o)
ALL_OBJS = $(ALL_DBUS_OBJS) $(ALL_LOCAL_OBJS)

LIBS = -L../../../export/opt/lib -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz -ldbus-1 -lpthread

TAGETS=subagent

$(shell rm -f user_manage.h || ln -s ../../eag/pkg/src_multi_ins/user_manage.h user_manage.h)

$(TAGETS): $(ALL_OBJS)
	make makelink
	$(CC) $(CFLAGS) $(BUILDAGENTLIBS) $(INCLUDE) $(LIBS) $(LDFLAGS)  -o $@ $^ 
#	$(OBJCOPY) -S --strip-all $@_temp $@;rm -f $@_temp

DLFLAGS=-fPIC -shared

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

subagent_plugin.so: $(ALL_OBJS)
	make makelink
	$(CC) $(CFLAGS) $(BUILDAGENTLIBS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(DLFLAGS)  -o $@_temp $^ 
	 $(OBJCOPY) -S --strip-all $@_temp $@;rm -f $@_temp

%.o:%.c
	$(CC) $(CFLAGS)  -c $< -o $@

clean:
	rm -rf subagent *.o $(TAGES) subagent_plugin.so
	-find . -name *.o -exec rm -rf {} \;

#nstAgentPluginObject.so: nstAgentPluginObject.c Makefile
#	$(CC) $(CFLAGS) $(DLFLAGS) -c -o nstAgentPluginObject.o nstAgentPluginObject.c
#	$(CC) $(CFLAGS) $(DLFLAGS) -c -o nstAgentPluginObject1.o nstAgentPluginObject1.c
#	$(CC) $(CFLAGS) $(DLFLAGS) -o nstAgentPluginObject.so nstAgentPluginObject.o nstAgentPlugin_O
