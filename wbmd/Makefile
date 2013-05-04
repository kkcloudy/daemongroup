
CC = mips-linux-gnu-gcc

LDFLAGS = -lpthread -ldl -lssl -lcrypto
LDFLAGS += -L/opt/mipslibs/usr/lib 
LDFLAGS += -lsctp

CFLAGS = -Wall -g -O0 -I. 
#CFLAGS += -I$(ACCAPI_DIR)
CFLAGS += -I/opt/mipslibs/usr/include
CFLAGS += -I../accapi
CFLAGS += -I/opt/mipslibs/usr/include/libxml2
CFLAGS += -Werror
CFLAGS += -I../net-snmp/engine/include
LIBS = -L/opt/mipslibs/usr/lib -L./  -L/opt/mipslibs/usr/lib -lxml2 -lz -L../net-snmp/engine/lib -lnetsnmpmibs -lnetsnmpagent -lnetsnmphelpers -lnetsnmp -lm -ldl

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
DBUS_LIBS=-ldbus-1


RM = /bin/rm -f 

# list of generated object files for AC. 
WBMD_OBJS = wbmd.o  wbmd_check.o  wbmd_dbus.o  wbmd_dbushandle.o  wbmd_log.o  wbmd_manage.o  wbmd_thread.o  wbmd_timelib.o
 
WBMD_SRCS = $(WBMD_OBJS:.o=.c) 

WBMD_DEPS := $(WBMD_OBJS:.o=.d)

# program executables. 
WBMD_APP = wbmd 

.PHONY: deps clean clean_libs libs

# top-level rule, to compile everything. 
all: $(WBMD_APP) 

$(WBMD_APP): $(WBMD_OBJS) 
	$(CC) $(WBMD_OBJS) $(LDFLAGS) $(LDLIBS)  $(DBUS_LIBS) $(LIBS) -o $(WBMD_APP) 
%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $*.o $<
clean: 
	$(RM) $(WBMD_APP) $(WBMD_OBJS) $(WBMD_DEPS) 

clean_deps:
	$(WBMD_DEPS) 
	
deps: $(WBMD_SRC) 
	$(CC) -MD -E $(WBMD_SRCS) $(CFLAGS) >/dev/null

-include $(WBMD_DEPS)
