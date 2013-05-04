MIPS_ADDITION_LIB_ROOT_PATH = /opt/mipslibs
CFLAGS = -O2 -Wall -Winline -Werror -g -ffunction-sections -D_GNU_SOURCE
LDFLAGS = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -Wl,--gc-sections
LIBS = -L ./ -lxml2 -lz -lpthread

CC = mips-linux-gnu-gcc
OBJCOPY = mips-linux-gnu-objcopy

INCLUDE += -I ./
INCLUDE += -I ../
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../../accapi
INCLUDE += -I ../../accapi/nm/app/pppoe

ALL_C := $(shell ls *.c)
S_OBJS = ../md5.o ../pppoe_util.o
ALL_OBJS = $(ALL_C:.c=.o) $(S_OBJS)

TAGETS = pppoe_client
	
default: $(ALL_OBJS)
	$(CC) $(ALL_OBJS) $(LDFLAGS) $(LIBS) -o $(TAGETS)_temp
	$(OBJCOPY) -S --strip-all $(TAGETS)_temp $(TAGETS); rm -f $(TAGETS)_temp $(S_OBJS)
	
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $*.o $<

	
clean:
	$(RM) $(TAGETS) test_* *.o *.o.txt $(S_OBJS)
