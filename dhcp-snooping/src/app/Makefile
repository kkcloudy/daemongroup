# DHCP Snooping
#
CC  = mips-linux-gnu-gcc

CFLAGS = -g -Wall -ansi
LDFLAGS = -g -L$(LIB_EXPORT_DIR) -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib 
LDLIBS = -ldhcpsnp -lpthread -lsqlite3

#CFLAGS += -DDBUS_API_SUBJECT_TO_CHANGE

INCLUDES += -I$(ACCAPI_DIR)
#DBUS_INCLUDES = $(shell pkg-config --cflags dbus-1)
#DBUS_LIBS = $(shell pkg-config --libs dbus-1)

DHCPSNP_OBJS = dhcp_snp_main.o

.PHONY: all
all: dhcpsnpd 

dhcpsnpd: $(DHCPSNP_OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(DBUS_LIBS) -o $@ $^ 
	mv dhcpsnpd $(BIN_EXPORT_DIR)/


%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $*.o $<

	

.PHONY: clean
clean:
	rm -rf *~ $(DHCPSNP_OBJS) dhcpsnpd

