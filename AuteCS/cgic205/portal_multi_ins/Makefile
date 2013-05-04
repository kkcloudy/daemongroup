CFLAGS=-O3 -Wall -ffunction-sections -I$(ACCAPI_DIR) 
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L../ -ldbusapi -lcgic -lm $(LD_PATH) -lxml2 -lz 

#LD_PATH = -L ../../../../export/opt/lib/
LD_PATH += -L${AUTEWAREFS_DIR}/files/opt/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

LDFLAGS = -shared -Wl,-soname,libcgic.so.0 

LDFLAGS_CGI += -Wl,--gc-sections -L ../../../lib/nm -lnm -L ../../../dcli/src/pub -ldclipub


INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../../../accapi
INCLUDE += -I ../

ACCAPIPATH=../../../accapi/
CFLAGS += -I $(ACCAPIPATH)
CFLAGS += -I $(ACCAPIPATH)nm/app/
CFLAGS += -I $(ACCAPIPATH)nm/app/snmp/
CFLAGS += -I $(ACCAPIPATH)nm/app/eag/
CFLAGS += -I $(ACCAPIPATH)nm/app/firewall/
CFLAGS += -I $(ACCAPIPATH)nm/ip/
CFLAGS += -I $(ACCAPIPATH)nm/public/
CFLAGS += -I $(ACCAPIPATH)nm/switch/             
CFLAGS += -I $(ACCAPIPATH)nm/system/             
CFLAGS += -I $(ACCAPIPATH)nm/wlan/ 

XML_LIB = $(LD_PATH) -lxml2 -lz
LOCAL_LIB = -L ./ 

#EXTERN_WS := $(shell cd ..;echo ws_*.c)
#EXTERN_OBJS := $(EXTERN_WS:.c=.o)

CGIFILES_WP := $(shell echo wp_*.c)
CGIFILES_WS := $(shell echo ws_*.c)

CGIOBJS_WP := $(CGIFILES_WP:.c=.o)
CGIOBJS_WS := $(CGIFILES_WS:.c=.o)
#CGIOBJS_WS += $(EXTERN_OBJS)
ALL_CGIFILES := $(CGIFILES_WP:.c=.cgi)

VPATH = ..

sources=$(CGIOBJS_WP) $(CGIOBJS_WS)


$(shell [ rm -f user_manage.h ] || ln -s ../../../eag/pkg/src_multi_ins/user_manage.h user_manage.h)

all:$(ALL_CGIFILES)
	cp *.cgi ../


%.cgi:%.o $(CGIOBJS_WS)
	$(CC) $< -o $@_temp $(CGIOBJS_WS) $(DEFS) $(LDFLAGS_CGI) $(DBUS_INCLUDE) $(INCLUDE) $(LIBS) $(XML_LIB) $(LOCAL_LIB) $(DBUS_LIBS) 
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
