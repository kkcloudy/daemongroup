CFLAGS=-O3 -Wall -ffunction-sections
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L. -ldbusapi -lcgic -lm $(LD_PATH) -lxml2 -lz 

LD_PATH = -L ../../export/opt/lib/
LD_PATH += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

LDFLAGS = -shared -Wl,-soname,libfacl.so
LDFLAGS_CGI += -Wl,--gc-sections

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include

INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(ACCAPI_DIR) 
INCLUDE += -I $(ACCAPI_DIR)/facl
INCLUDE += $(DBUS_INCLUDE)
INCLUDE += -I ../../../had/src/lib

XML_LIB = $(LD_PATH) -lxml2 -lz -ldbus-1

sources := $(shell find ./ -name "*.c")
objs := $(sources:.c=.o)

libfacl.so:$(objs)
	$(RM) $(LIB_EXPORT_DIR)/libfacl.*
	$(CC) $(LDFLAGS) -o $@ $^
	cp libfacl.so $(LIB_EXPORT_DIR)

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.c.o:
	$(CC) $(CFLAGS) -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include $(DBUS_INCLUDE) $(INCLUDE) -c $< -o $@

clean:
	$(RM) $(objs) *.so*

