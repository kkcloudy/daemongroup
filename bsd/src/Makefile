
CC = mips-linux-gnu-gcc

LDFLAGS = -lpthread -ldl
LDFLAGS += -L/opt/mipslibs/usr/lib 
LDFLAGS += -lsctp

CFLAGS = -Wall -Werror -g -O0 -I. 
#CFLAGS += -I$(ACCAPI_DIR)
CFLAGS += -I/opt/mipslibs/usr/include
CFLAGS += -I../../accapi
CFLAGS += -I/opt/mipslibs/usr/include/libxml2
CFLAGS += -I../inc

LIBS = -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
DBUS_LIBS=-ldbus-1


RM = /bin/rm -f 

# list of generated object files for AC. 
BSD_OBJS = bsd.o bsd_log.o bsd_thread.o bsd_dbus.o bsd_management.o bsd_timerLib.o bsd_tipc.o bsd_monitor.o bsd_tcp.o
 
BSD_SRCS = $(BSD_OBJS:.o=.c) 

BSD_DEPS := $(BSD_OBJS:.o=.d)

# program executables. 
BSD_APP = bsd

.PHONY: deps clean clean_libs libs

# top-level rule, to compile everything. 
all: $(BSD_APP) 

$(BSD_APP): $(BSD_OBJS) 
	$(CC) $(BSD_OBJS) $(LDFLAGS) $(LDLIBS)  $(DBUS_LIBS) $(LIBS) -o $(BSD_APP) 
%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $*.o $< -D_GNU_SOURCE
clean: 
	$(RM) $(BSD_APP) $(BSD_OBJS) $(BSD_DEPS) 

clean_deps:
	$(BSD_DEPS) 
	
deps: $(BSD_SRC) 
	$(CC) -MD -E $(BSD_SRCS) $(CFLAGS) >/dev/null

-include $(BSD_DEPS)