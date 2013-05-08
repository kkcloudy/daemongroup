CFLAGS=-O3 -Wall -ffunction-sections -I$(ACCAPI_DIR) 
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L. -ldbusapi -lcgic -lm $(LD_PATH) -lxml2 -lz 
LIBS= -L. -lcgic -lm $(LD_PATH) -lxml2 -lz 

#LD_PATH = -L ../../../export/opt/lib/
LD_PATH = -L${AUTEWAREFS_DIR}/files/opt/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

LDFLAGS = -shared -Wl,-soname,libcgic.so.0
LDFLAGS_CGI += -Wl,--gc-sections -L ../../lib/nm -lnm -L ../../dcli/src/pub -ldclipub


INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I ../../accapi 
INCLUDE += -I ../../net-snmp/engine/include
INCLUDE += -I ../../AuteCS/cgic205/portal_multi_ins
INCLUDE += -I ../../bsd/inc/

ACCAPIPATH=../../accapi/
INCLUDE += -I $(ACCAPIPATH)
INCLUDE += -I $(ACCAPIPATH)nm/app/
INCLUDE += -I $(ACCAPIPATH)nm/app/snmp/
INCLUDE += -I $(ACCAPIPATH)nm/app/sample/
INCLUDE += -I $(ACCAPIPATH)nm/app/manage/
INCLUDE += -I $(ACCAPIPATH)nm/app/dhcp/
INCLUDE += -I $(ACCAPIPATH)nm/app/eag/
INCLUDE += -I $(ACCAPIPATH)nm/app/firewall/
INCLUDE += -I $(ACCAPIPATH)nm/ip/
INCLUDE += -I $(ACCAPIPATH)nm/public/
INCLUDE += -I $(ACCAPIPATH)nm/switch/             
INCLUDE += -I $(ACCAPIPATH)nm/system/             
INCLUDE += -I $(ACCAPIPATH)nm/wlan/ 

XML_LIB = $(LD_PATH) -lxml2 -lz -ldbus-1

CGIFILES_WP := $(shell echo wp_*.c)
CGIFILES_WS := $(shell echo ws_*.c)

CGIOBJS_WP := $(CGIFILES_WP:.c=.o)
CGIOBJS_WS := $(CGIFILES_WS:.c=.o)

ALL_CGIFILES := $(CGIFILES_WP:.c=.cgi)

EXT_TAGET= wireless portal snmp_agent

sources=$(CGIOBJS_WP) $(CGIOBJS_WS)

$(shell [ rm -f ./portal_multi_ins/user_manage.h ] || ln -s ../../../eag/pkg/src_multi_ins/user_manage.h ./portal_multi_ins/user_manage.h)

all: libdbusapi.a $(ALL_CGIFILES) $(EXT_TAGET) up.cgi

#CGIOBJS_WS += ./portal_multi_ins/ws_user_manage.o

%.cgi:%.o libdbusapi.a libcgic.so 
	$(CC) $<  -o $@_temp $(LDFLAGS_CGI) $(DBUS_INCLUDE) $(INCLUDE) $(LIBS) $(XML_LIB) $(LOCAL_LIB) $(DBUS_LIBS) -ldbusapi 
	$(OBJCOPY) -S --strip-all $@_temp $@;rm -f $@_temp
up.cgi: 
#	$(CC) up.c -o up.cgi
#	$(CC) $(CFLAGS) -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include $(DBUS_INCLUDE) $(INCLUDE) up.c $(DBUS_LIBS) -o $@
	$(CC) up.c -o $@ $(LDFLAGS_CGI) $(DBUS_INCLUDE) $(INCLUDE) $(XML_LIB) $(LOCAL_LIB) $(DBUS_LIBS) 

install: libcgic.a
	cp libcgic.a /usr/local/lib
	cp cgic.h /usr/local/include
	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libcgic.so: cgic.o
	$(CC) $(LDFLAGS) -o libcgic.so.0.1 $^
	ln -s libcgic.so.0.1 libcgic.so.0
	ln -s libcgic.so.0.1 libcgic.so
#	$(AR) rc libcgic.a cgic.o
#	$(RANLIB) libcgic.a

libdbusapi.a:$(CGIOBJS_WS)
#	$(CC) -shared -fPIC -Wl,-soname,libdbusapi.so.0 -o libdbusapi.so.0.1 $^
#	ln -s libdbusapi.so.0.1 libdbusapi.so.0
#	ln -s libdbusapi.so.0.1 libdbusapi.so
	$(AR) rc libdbusapi.a $(CGIOBJS_WS)  
	$(RANLIB) libdbusapi.a

#mingw32 and cygwin users: replace .cgi with .exe


.PHONY: snmp_agent
snmp_agent:
	make -C snmp_agent

.PHONY: wireless
wireless:
	make -C wireless

	
.PHONY: portal
portal:
	if [ $(OLDEAG) -eq 1 ]; then  \
		echo "make old portal "; \
		make -C portal; \
	else \
		echo "make multi ins";\
		make -C portal_multi_ins; \
	fi


#include $(sources:.o=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.c.o:
	$(CC) $(CFLAGS) -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include $(DBUS_INCLUDE) $(INCLUDE) -c $< -o $@

clean:
	rm -f *.o *.a *.d *.so* capture *.cgi

