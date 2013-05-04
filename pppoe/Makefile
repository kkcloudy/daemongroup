MIPS_ADDITION_LIB_ROOT_PATH = /opt/mipslibs
CFLAGS = -O2 -Wall -Winline -Werror -g -ffunction-sections -D_GNU_SOURCE
LDFLAGS = -L ../../auteware/files/opt/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -Wl,--gc-sections
LIBS = -L ./ -lxml2 -lz -ldbus-1 -lpthread


CC=mips-linux-gnu-gcc
OBJCOPY=mips-linux-gnu-objcopy


INCLUDE += -I ./
INCLUDE += -I ../accapi
INCLUDE += -I ../accapi/nm/app/pppoe
INCLUDE += -I ../dbus-1.4.0
INCLUDE += -I ${KERNEL_ROOT}
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/dbus-1.0
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib/dbus-1.0/include

ALL_C := $(shell ls *.c)
ALL_OBJS = $(ALL_C:.c=.o)

test_mem_objs = pppoe_log.o mem_cache.o
test_thread_objs = pppoe_log.o mem_cache.o time.o pppoe_thread.o pppoe_netlink.o  pppoe_device.o pppoe_session.o pppoe_control.o pppoe_instance.o pppoe_discover.o
test_ippool_objs = pppoe_log.o pppoe_ippool.o

TAGETS = pppoed
	
default: $(ALL_OBJS)
	$(CC) $(ALL_OBJS) $(LDFLAGS) $(LIBS) -o $(TAGETS)_temp
	$(OBJCOPY) -S --strip-all $(TAGETS)_temp $(TAGETS);rm -f $(TAGETS)_temp
	
test_mem: $(test_mem_objs)
	$(CC) $(test_mem_objs) $(LDFLAGS) $(LIBS) -o test_mem_temp
	$(OBJCOPY) -S --strip-all test_mem_temp $@;rm -f test_mem_temp

test_thread: $(test_thread_objs)
	$(CC) $(test_mem_objs) $(LDFLAGS) $(LIBS) -o test_thread_temp
	$(OBJCOPY) -S --strip-all test_thread_temp $@;rm -f test_thread_temp

test_ippool: $(test_ippool_objs)
	$(CC) $(test_ippool_objs) $(LDFLAGS) $(LIBS) -o test_ippool_temp
	$(OBJCOPY) -S --strip-all test_ippool_temp $@;rm -f test_ippool_temp

.c.o:
#	$(CC) -E $(CFLAGS) $(INCLUDE) -o $*.o.txt $<
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $*.o $<

	
clean:
	$(RM) $(TAGETS) test_* *.o *.o.txt
