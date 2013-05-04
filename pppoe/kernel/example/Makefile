OCTEON_ROOT = /opt/CN/OCTEON-SDK

CC = mips-linux-gnu-gcc

kernel_source := ../../../kernel2.6.32.27cn

CFLAGS = -I/opt/mipslibs/usr/include/libxml2 -I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
CFLAGS += -I../

ALL_LOCAL_C := $(shell ls *.c)
ALL_OBJS := $(ALL_LOCAL_C:$*.c=$*)

$*: $*.c
	$(CC) $(CFLAGS) -o $* $<

default: $(ALL_OBJS)
	mkdir -p export
	mv $(ALL_OBJS) export/
	
clean: 
	rm -rf export
