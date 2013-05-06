CFLAGS=-O3 -Wall -ffunction-sections -I$(ACCAPI_DIR) 
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L. -ldbusapi -lcgic -lm $(LD_PATH) -lxml2 -lz 

LD_PATH = -L ../../../export/opt/lib/
LD_PATH += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

LDFLAGS = -shared -Wl,-soname,libnm.so
LDFLAGS_CGI += -Wl,--gc-sections
LDFLAGS += -L ../../dcli/src/pub/ -ldclipub

INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I ../../accapi
INCLUDE += -I ../../accapi/nm/app/
INCLUDE += -I ../../accapi/nm/app/eag
INCLUDE += -I ../../accapi/nm/app/firewall
INCLUDE += -I ../../accapi/nm/app/snmp
INCLUDE += -I ../../accapi/nm/app/sample
INCLUDE += -I ../../accapi/nm/app/manage
INCLUDE += -I ../../accapi/nm/app/dhcp
INCLUDE += -I ../../accapi/nm/app/pppoe
INCLUDE += -I ../../accapi/nm/ip
INCLUDE += -I ../../accapi/nm/public
INCLUDE += -I ../../accapi/nm/switch
INCLUDE += -I ../../accapi/nm/system
INCLUDE += -I ../../accapi/nm/wlan

					
XML_LIB = $(LD_PATH) -lxml2 -lz -ldbus-1

sources := $(shell find . -name *.c)
objs := $(sources:.c=.o)


#install: libnm.a
#	cp libcgic.a /usr/local/lib
#	cp cgic.h /usr/local/include
#	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libnm.so:$(objs)
	$(RM) $(LIB_EXPORT_DIR)/libnm.*
	$(CC) $(LDFLAGS) -o libnm.so.0.1 $^
	cp libnm.so.0.1 $(LIB_EXPORT_DIR)/
	-ln -s libnm.so.0.1 libnm.so.0
	-ln -s libnm.so.0.1 libnm.so
	-ln -s libnm.so.0.1 $(LIB_EXPORT_DIR)/libnm.so.0
	-ln -s libnm.so.0.1 $(LIB_EXPORT_DIR)/libnm.so
#	$(AR) rc libcgic.a cgic.o
#	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe


#include $(sources:.o=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.c.o:
	$(CC) $(CFLAGS) -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include $(DBUS_INCLUDE) $(INCLUDE) -c $< -o $@

clean:
	$(RM) $(LIB_EXPORT_DIR)/libnm.*
	$(RM) -f $(objs) *.so*

