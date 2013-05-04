CFLAGS += -O3 -Wall  -ffunction-sections
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L.. -ldbusapi -lcgic -lm
#LD_PATH = -L../../../../export/opt/lib
LD_PATH += -L${AUTEWAREFS_DIR}/files/opt/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib


LDFLAGS = -shared -Wl,-soname,libcgic.so.0 

LDFLAGS_CGI += -Wl,--gc-sections -L ../../../lib/nm -lnm -L ../../../dcli/src/pub -ldclipub


INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../

ACCAPIPATH=../../../accapi/
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
LOCAL_LIB = -L ./ 

#EXTERN_WS := $(shell cd ..;echo ws_*.c)
#EXTERN_OBJS := $(EXTERN_WS:.c=.o)

CGIFILES_WP := $(shell echo wp_*.c)
#CGIFILES_WS := $(shell echo ws_*.c)

CGIOBJS_WP := $(CGIFILES_WP:.c=.o)
#CGIOBJS_WS := $(CGIFILES_WS:.c=.o)
#CGIOBJS_WS += $(EXTERN_OBJS)
ALL_CGIFILES := $(CGIFILES_WP:.c=.cgi)

VPATH = ..

sources=$(CGIOBJS_WP) $(CGIOBJS_WS)

all:$(ALL_CGIFILES)
	cp *.cgi ../


%.cgi:%.o dbusapi
	$(CC) $< -o $@_temp $(DEFS) $(LDFLAGS_CGI) $(DBUS_INCLUDE) $(INCLUDE) $(LIBS) $(XML_LIB) 
	$(OBJCOPY) -S --strip-all $@_temp $@;rm -f $@_temp
	
.c.o:
	$(CC) $(DEFS) $(CFLAGS) $(INCLUDE) $(DBUS_INCLUDE) $(INCLUDE) -c $< -o $@ 

dbusapi:
	make -C ../ libdbusapi.a


include $(sources:.o=.d)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$;

#libcgic.so: ../cgic.o ../cgic.h
#	$(CC) $(LDFLAGS) -o libcgic.so.0.1 $^
#	ln -s libcgic.so.0.1 libcgic.so.0
#	ln -s libcgic.so.0.1 libcgic.so
#	$(AR) rc libcgic.a cgic.o
#	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe


clean:
	rm -f *.o *.a *.d capture *.cgi $(EXTERN_OBJS)
