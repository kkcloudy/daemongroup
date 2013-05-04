CC 	= mips-linux-gnu-gcc
OBJCOPY = mips-linux-gnu-objcopy

CFLAGS  = -O3 -Wall -ffunction-sections
LD_PATH = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib -L ./
LDFLAGS = -shared -Wl,-soname,libmanage.so
INCLUDE += -I $(ACCAPI_DIR)
INCLUDE += -I $(ACCAPI_DIR)/nm/app/manage

sources := $(shell find . -name '*.c')
objs 	:= $(sources:.c=.o)

libmanage.so:$(objs)
	$(RM) $(LIB_EXPORT_DIR)/libmanage.*
	$(CC) $(LDFLAGS) -o libmanage.so.0.1 $^
	cp libmanage.so.0.1 $(LIB_EXPORT_DIR)/
	-ln -s libmanage.so.0.1 libmanage.so.0
	-ln -s libmanage.so.0 libmanage.so
	-ln -s libmanage.so.0.1 $(LIB_EXPORT_DIR)/libmanage.so.0
	-ln -s libmanage.so.0 $(LIB_EXPORT_DIR)/libmanage.so
	
%.d: %.c
	set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.c.o:
	$(CC) $(CFLAGS) -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include $(INCLUDE) -c $< -o $@

clean:
	$(RM) $(LIB_EXPORT_DIR)/libmanage.*
	$(RM) $(objs) *.so*
