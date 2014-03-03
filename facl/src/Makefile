CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L.. -ldbusapi -lm

CFLAGS = -Wall -g -ffunction-sections
#CFLAGS += -Werror

LD_PATH = -L $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

INCLUDE += -I ./
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../../accapi
INCLUDE += -I ../../accapi/facl
INCLUDE += $(DBUS_INCLUDE)

LDFLAGS += $(LD_PATH) -ldbus-1
#LDFLAGS += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -liptc
LDFLAGS += -lpthread
LDFLAGS += -L ./libfacl -lfacl
LOCAL_LIB = -L ./ -lfaclintf

libfaclsource = facl_interface.o

faclsource = facl_main.o facl_ins.o facl_interface.o facl_command.o facl_db.o facl_shell.o


testsource = facl_dcli_test.o


all:libfacl.so libfaclintf.so facl
	cp facl $(BIN_EXPORT_DIR)
	cp libfaclintf.so $(LIB_EXPORT_DIR)

facl:$(faclsource)
	$(CC) $^ $(LDFLAGS) -ldl -o $@

libfaclintf.so:$(libfaclsource)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libfaclintf.so -o $@ $^

libfacl.so:
	make -C ./libfacl

test:$(testsource) libfaclintf.so
	$(CC) $^ $(LDFLAGS) $(LOCAL_LIB) -ldl -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ 

%.test:%.c
	$(CC) -D$(<:.c=_test)=1  $(CFLAGS) $(INCLUDE) $< -o $@ 
	rm $@;\
	

%.d:%.c
	set -e; rm -f $@;\
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$;



clean:
	make -C ./libfacl clean
	$(RM) $(LIB_EXPORT_DIR)/libfacl*
	rm -f *.o *.d *.test $(TARGET) facl libfaclintf.so
