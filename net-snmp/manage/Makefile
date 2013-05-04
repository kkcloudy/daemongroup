CC = mips-linux-gnu-gcc
LDFLAGS = -ldl -D_REENTRANT -L ../../../export/opt/lib 
LIBS = -L../../../auteware/files/opt/lib -L/opt/mipslibs/usr/lib -L./  -lm -L/opt/mipslibs/usr/lib -lxml2 -lz -ldbus-1 -lmanage -L ../../lib/nm -lnm -L ../../dcli/src/pub -ldclipub -L$(LIB_EXPORT_DIR) -leagintf
LDFLAGS += -Wl,--gc-sections
OBJCOPY=mips-linux-gnu-objcopy


ACCAPIPATH=../../accapi/

CFLAGS += -I../engine/include -Dlinux -I.
CFLAGS += -I/opt/mipslibs/usr/include/libxml2 -I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
CFLAGS += -I $(ACCAPIPATH)
CFLAGS += -I $(ACCAPIPATH)nm/app/
CFLAGS += -I $(ACCAPIPATH)nm/app/snmp/
CFLAGS += -I $(ACCAPIPATH)nm/app/sample/
CFLAGS += -I $(ACCAPIPATH)nm/app/manage/
CFLAGS += -I $(ACCAPIPATH)nm/app/eag/
CFLAGS += -I $(ACCAPIPATH)nm/app/firewall/
CFLAGS += -I $(ACCAPIPATH)nm/ip/
CFLAGS += -I $(ACCAPIPATH)nm/public/
CFLAGS += -I $(ACCAPIPATH)nm/switch/             
CFLAGS += -I $(ACCAPIPATH)nm/system/             
CFLAGS += -I $(ACCAPIPATH)nm/wlan/ 

CFLAGS += -ffunction-sections

ALL_LOCAL_C := $(shell ls *.c)
ALL_LOCAL_OBJS = $(ALL_LOCAL_C:.c=.o)
ALL_OBJS =$(ALL_LOCAL_OBJS)


TAGETS = acmanage

.PHONY: managelib
managelib:
	make -C managelib

$(TAGETS): managelib $(ALL_OBJS)
	 $(CC) $(ALL_OBJS) $(BUILDAGENTLIBS) $(LDFLAGS) $(LIBS)  -o $(TAGETS)_temp
	 $(OBJCOPY) -S --strip-all $(TAGETS)_temp $@;rm -f $(TAGETS)_temp

	 
%.o: %.c
	$(CC) -E $(CFLAGS) -o $*.o.txt $<
	$(CC) -c $(CFLAGS) -o $*.o $<
	
clean:
	make -C managelib clean
	$(RM) $(TAGETS) *.o *.o.txt
