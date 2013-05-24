
DEFS	=
MAIN_OPT=
PROF_OPT=
TOPDIR 	= .
OBJ_DIR = ./obj
MKDIR 	= mkdir
CP		= cp
RM		= rm 

CC 		= mips-linux-gnu-gcc #-E
CFLAGS += -g -Wall -D_$(AWVERSION)_ -D_REENTRANT -D__LINUX__ -I$(ACCAPI_DIR)
CPPFLAGS += -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include -ansi --std=c99
LDFLAGS +=  -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib
LIBS = -lpthread
INCLUDES = -I.

CPPFLAGS += $(DBUS_INCLUDE)

COMPILE = $(CC) $(DEFS) $(MAIN_OPT) $(PROF_OPT)  $(INCLUDES) $(CFLAGS)

#AR = ar
AR = mips-linux-gnu-ar
#ARFLAGS = rcs
#ARFLAGS = cru
#RANLIB	= ranlib
RANLIB = mips-linux-gnu-ranlib
SOFLAGS =  -nostartfiles -shared

EXECUTABLE=had

DBUS_LIBS = dbus-1
.SUFFIXES: .S .c .o .s

all:prepare $(EXECUTABLE)
VRRPD_OBJS = had_vrrpd.o had_libnetlink.o had_ipaddr.o had_dbus.o had_log.o had_uid.o

had: $(VRRPD_OBJS)
	$(CC) $(LIBS) $(LDFLAGS) -L. -l$(DBUS_LIBS) -o $(OBJ_DIR)/$@ $(VRRPD_OBJS)
	$(CP) ./*.o $(OBJ_DIR)
	$(RM) ./*.o

prepare:
	$(MKDIR) -p $(OBJ_DIR)

clean:
	$(RM) -f all *.o *.a *~ $(EXECUTABLE) *.bak make.depend
	$(RM) -rf $(OBJ_DIR)

depend:
	echo "# DO NOT DELETE THIS LINE -- make  depend  depends  on it." > make.depend
	makedepend -- $(DEFS) $(INCLUDES) $(CFLAGS) -- $(CFILES) -f make.depend

-include make.depend
