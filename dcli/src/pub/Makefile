
#CC = mips-linux-gnu-gcc

#LDFLAGS = -lssl -lcrypto -lpthread -ldl -D_REENTRANT
#LDFLAGS = ./static/libssl.a ./static/libcrypto.a -lpthread -ldl -D_REENTRANT

INCLUDES = -I$(ACCAPI_DIR) -I$(BUILDROOTDIR)/usr/include -I$(BUILDROOTDIR)/usr/include/dbus-1.0 -I$(BUILDROOTDIR)/usr/lib/dbus-1.0/include
LD_PATH = -L$(BUILDROOTDIR)/usr/lib
DEFS = -DHAVE_CONFIG_H $(DCLI_HAS_WCPSS)
LDFLAGS = -shared -lrt -Wl,-soname,$@

# Memory leak
#LDFLAGS += ../dmalloc-5.5.0/libdmallocth.a
#CFLAGS += -DDMALLOC

# Capwap Debugging
#CFLAGS += -DCW_DEBUGGING
CFLAGS += -D_D_WCPSS_
CFLAGS += -I${BUILD_DIR}/${SNMP_ROOTDIR}/engine/include
DBUS_LIBS = -ldbus-1



PUB_LIB = libdclipub.so
PUB_LIB0 = libdclipub.so.0 
PUB_LIB01 = libdclipub.so.0.1
PUB_OBJS = dcli_dbus_remote.o wid_ac.o wid_wtp.o wid_wqos.o asd_sta.o asd_security.o asd_ac_roaming.o wid_wlan.o wid_radio.o wid_ebr.o wid_ac_ip_list.o wid_ap_group.o hmd_local.o bsd_bsd.o wbmd_wb.o
 

.PHONY: all
all:$(PUB_LIB01)

.PHONY: clean
clean:
	$(RM) *.o $(PUB_LIB) $(PUB_LIB0) $(PUB_LIB01)

$(PUB_LIB01): $(PUB_OBJS)
	$(RM) $(LIB_EXPORT_DIR)/$(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB0) $(LIB_EXPORT_DIR)/$(PUB_LIB)
	$(CC) ${LDFLAGS} $(LD_PATH) -o $@ $^ $(DBUS_LIBS)
	cp $(PUB_LIB01) $(LIB_EXPORT_DIR)/
	ln -sf $(PUB_LIB01) $(PUB_LIB0)
	ln -sf $(PUB_LIB01) $(PUB_LIB)
	ln -sf $(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB0) 
	ln -sf $(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB)             
	
%.o: %.c
	${RM} $@
	$(CC) -c $(DEFS) $(INCLUDES) $(CFLAGS) -o $*.o $<
