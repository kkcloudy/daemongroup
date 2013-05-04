CC = mips-linux-gnu-gcc
LIBS = -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz -ldbus-1
LDFLAGS += -Wl -g
CFLAGS += -I/opt/mipslibs/usr/include/libxml2 -I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include

ALL_LOCAL_C := $(shell ls *.c)
ALL_LOCAL_OBJS = $(ALL_LOCAL_C:.c=.o)
ALL_OBJS = $(ALL_LOCAL_OBJS)
TAGETS = test-trap-helper

all:$(TAGETS)

$(TAGETS): $(ALL_OBJS)
#	make makelink
	$(CC) $(ALL_OBJS) $(BUILDAGENTLIBS) $(LDFLAGS) $(LIBS)  -o $(TAGETS)
%.o: %.c
	$(CC) -c $(CFLAGS)  -o $*.o $< -g

clean:
	$(RM) $(TAGETS) *.o

