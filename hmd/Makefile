
CC = mips-linux-gnu-gcc

LDFLAGS = -lpthread -ldl
LDFLAGS += -L/opt/mipslibs/usr/lib 
LDFLAGS += -lsctp

CFLAGS = -Wall -g -O0 -D_$(AWVERSION)_ -I. 
#CFLAGS += -I$(ACCAPI_DIR)
CFLAGS += -I/opt/mipslibs/usr/include
CFLAGS += -I../accapi
CFLAGS += -I/opt/mipslibs/usr/include/libxml2
CFLAGS += -Werror
LIBS = -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
DBUS_LIBS=-ldbus-1


RM = /bin/rm -f 

# list of generated object files for AC. 
HMD_OBJS = hmd.o HmdLog.o HmdThread.o HmdDbus.o HmdManagement.o HmdStateListen.o HmdTimeLib.o  HmdDbusHandler.o HmdMonitor.o 
 
HMD_SRCS = $(HMD_OBJS:.o=.c) 

HMD_DEPS := $(HMD_OBJS:.o=.d)

# program executables. 
HMD_APP = hmd 

.PHONY: deps clean clean_libs libs

# top-level rule, to compile everything. 
all: $(HMD_APP) 

$(HMD_APP): $(HMD_OBJS) 
	$(CC) $(HMD_OBJS) $(LDFLAGS) $(LDLIBS)  $(DBUS_LIBS) $(LIBS) -o $(HMD_APP) 
	$(CC) clear_radio_if.c -o clear_radio_if

%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $*.o $<
clean: 
	$(RM) $(HMD_APP) $(HMD_OBJS) $(HMD_DEPS) 
	$(RM) clear_radio_if.o clear_radio_if

clean_deps:
	$(HMD_DEPS) 
	
deps: $(HMD_SRC) 
	$(CC) -MD -E $(HMD_SRCS) $(CFLAGS) >/dev/null

-include $(HMD_DEPS)
