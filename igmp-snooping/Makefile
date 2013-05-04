#MAKEFILE for igmp_snoop

TARGETS := igmp_snoop

CFLAGS = -g -c 
ACCAPI_INCLUDE = -I$(ACCAPI_DIR)
INCLUDES += $(DBUS_INCLUDE) $(ACCAPI_INCLUDE)

target_OBJECTS = igmp_snoop_main.o igmp_snoop_com.o igmp_snoop_hw.o igmp_snoop_cli.o igmp_snp_dbus.o igmp_snp_log.o mld_snoop_main.o mld_snoop_hw.o mld_snoop_cli.o
DBUS_LIBS = -ldbus-1
LIBS = -lpthread -lrt
LIBS += $(DBUS_LIBS)
ifeq ($(CPU_ARCH_FLAG),arm)
LIBINCLUDE = -L${BUILDROOTDIR}/usr/lib
else
LIBINCLUDE = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib
endif
COMPILE = $(CC) $(CFLAGS) $(INCLUDES)
LINK = $(COMPILE) -o $@

.SUFFIXES:
.SUFFIXES: .c .o 

all: $(TARGETS)

igmp_snoop:$(target_OBJECTS)
	@rm -f $(TARGETS)
#	$(LINK) $(target_OBJECTS) $(LIBS)
	$(CC) -o igmp_snoop $(LIBINCLUDE) $(LIBS) $(target_OBJECTS)
	@rm -f *.o
	echo "Compile complete!"
	
.c.o:
	source='$<' object='$@' \
	$(COMPILE) -c `test -f '$<' || echo '$(srcdir)/'`$<
	
clean:
	-rm -f *.a *.o
	-rm -f $(TARGETS)

.PHONY: all clean
