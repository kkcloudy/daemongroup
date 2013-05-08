#CFLAGS=-O3 -Wall -ffunction-sections
CFLAGS= -Wall -g 
#CFLAGS+= -Werror
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L.. -ldbusapi -lm

LD_PATH = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

#LDFLAGS = -Wl,--gc-sections

INCLUDE += -I ./
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../../../accapi
INCLUDE += -I ../../../accapi/nm/app/drp
INCLUDE += -I ../../../had/src/lib
INCLUDE += -I /opt/mipslibs/usr/include/
INCLUDE += -I ../../../accapi/nm/public
INCLUDE += $(DBUS_INCLUDE)
INCLUDE += -I ../

XML_LIB = $(LD_PATH) -lz -lxml2 
LDFLAGS += $(XML_LIB)
LDFLAGS += -L../../../export/opt/lib $(DBUS_LIBS)
LDFLAGS += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -liptc
LDFLAGS += -lpthread
LOCAL_LIB = -L ./

libdrpsources = drp_main.c drp_handle.c drp_opxml.c drp_dbus.c drp_log.c drp_mem.c drp_util.c

libdrpintfsources=drp_interface.c

libdrp_objs=$(libdrpsources:.o=.c)
libdrpintf_objs=$(libdrpintfsources:.o=.c)

TARGET = drp

all:drp libdrpintf.so
	cp drp ${AUTEWAREFS_DIR}/files/opt/bin/
	cp libdrpintf.so $(LIB_EXPORT_DIR)
	-mkdir $(shell pwd)/../../../accapi/nm/app/drp/
	-ln -s $(shell pwd)/drp_interface.h $(shell pwd)/../../../accapi/nm/app/drp/drp_interface.h
	-ln -s $(shell pwd)/drp_def.h $(shell pwd)/../../../accapi/nm/app/drp/drp_def.h

drp:$(libdrp_objs)
	$(CC) $^ $(INCLUDE) $(LDFLAGS) $(CFLAGS) -o $@

libdrpintf.so:$(libdrpintf_objs)
	$(CC) $(INCLUDE) $(LDFLAGS) $(CFLAGS) -shared -Wl,-soname,libdrpintf.so -o $@ $^

drp_interface:drp_interface.o
	$(CC) $< $(INCLUDE) $(LDFLAGS) $(CFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ 


space:
	df|grep sdb2 > aaa
	awk '{printf $$4}' aaa > space
	
codeformat:
	space=$(shell df|grep sdb2|awk '{print $$4}');
	echo $(space);
	if [ $$(space) -gt 10000 ];then \
		ls *.h | xargs indent -kr -i8 -ts8 -sob -l80 -ss -bs -psl; \
		echo "yyy";\
		ls *.c | xargs indent -kr -i8 -ts8 -sob -l80 -ss -bs -psl; \
	fi)

%.test:%.c
	gcc -D$(<:.c=_test)=1  $(CFLAGS) $(INCLUDE) $< -o $@ $(LDFLAGS)
#	./$@; \
#	if [ $$? -eq 0 ];	then \
#		echo $(<:.c=_test_ok);\
#	else \
#		echo $(<:.c=_test_failed_failed_failed);\
#	fi;\
	rm $@;\
	

%.d:%.c
	set -e; rm -f $@;\
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$;

clean:
	rm -f *.o *.d $(TARGET) libdrpintf.so drp_interface
