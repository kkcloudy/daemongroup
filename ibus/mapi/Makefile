
#CC = mips-linux-gnu-gcc

#LDFLAGS = -lssl -lcrypto -lpthread -ldl -D_REENTRANT
#LDFLAGS = ./static/libssl.a ./static/libcrypto.a -lpthread -ldl -D_REENTRANT

DCLIDIR=../../dcli/src/lib/
IUDBUSDIR=../../accapi/dbus/iu/

INCLUDES = -I$(ACCAPI_DIR) -I$(BUILDROOTDIR)/usr/include -I$(BUILDROOTDIR)/usr/include/dbus-1.0 -I$(BUILDROOTDIR)/usr/lib/dbus-1.0/include -I$(DCLIDIR) -I$(IUDBUSDIR)
INCLUDES += ../../accapi/iuh

LD_PATH = -L$(BUILDROOTDIR)/usr/lib
DEFS = -DHAVE_CONFIG_H $(DCLI_HAS_WCPSS)
LDFLAGS = -shared -Wl,-soname,$@

# Memory leak
#LDFLAGS += ../dmalloc-5.5.0/libdmallocth.a
#CFLAGS += -DDMALLOC

# Capwap Debugging
#CFLAGS += -DCW_DEBUGGING

DBUS_LIBS = -ldbus-1



PUB_LIB = libdcliiuhmapi.so
PUB_LIB0 = libdcliiuhmapi.so.0 
PUB_LIB01 = libdcliiuhmapi.so.0.1
PUB_OBJS = mapi_hnb.o mapi_iu.o mapi_sigtran2udp.o 
 

.PHONY: all
all:$(PUB_LIB01)

.PHONY: clean
clean:
	$(RM) *.o $(PUB_LIB) $(PUB_LIB0) $(PUB_LIB01)

$(PUB_LIB01): $(PUB_OBJS)
	$(RM) $(LIB_EXPORT_DIR)/$(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB0) $(LIB_EXPORT_DIR)/$(PUB_LIB)
	$(CC) ${LDFLAGS} $(LD_PATH) -o $@ $^ $(DBUS_LIBS)
	cp $(PUB_LIB01) $(LIB_EXPORT_DIR)/
	ln -sf $(PUB_LIB01) $(PUB_LIB0)
	ln -sf $(PUB_LIB01) $(PUB_LIB)
	ln -sf $(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB0) 
	ln -sf $(PUB_LIB01) $(LIB_EXPORT_DIR)/$(PUB_LIB)             
	
%.o: %.c
	${RM} $@
	$(CC) -c $(DEFS) $(INCLUDES) $(CFLAGS) -o $*.o $<
