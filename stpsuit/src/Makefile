
DEFS	=
TOPDIR 	= .
OBJ_DIR = ./OBJ
LIB_DIR = ./LIB
MKDIR 	= mkdir
CP		= cp
RM		= rm 

#CC 		= mips-linux-gnu-gcc #-E
#CFLAGS = -g -Wall -D_REENTRANT -D__LINUX__ -DSTP_DBG -DOLD_READLINE
CFLAGS = -g -Wall -D_REENTRANT -D__LINUX__ -DSTP_DBG -DSTRONGLY_SPEC_802_1W -I$(ACCAPI_DIR) # -DOLD_READLINE

ifeq ($(CPU_ARCH_FLAG),arm)
LDFLAGS = -L${BUILDROOTDIR}/usr/lib
else
CPPFLAGS = -I$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include --std=c99
LDFLAGS =  -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib
endif

INCLUDES = -I.

CPPFLAGS += $(DBUS_INCLUDE)

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)
LIBS= -lutil -lreadline -ltermcap -lpthread
LIBS= -lutil -lpthread

#AR = ar

ifndef AR
AR = mips-linux-gnu-ar
endif

#ARFLAGS = rcs
#ARFLAGS = cru
#RANLIB	= ranlib

ifndef RANLIB
RANLIB = mips-linux-gnu-ranlib
endif

SOFLAGS =  -nostartfiles -shared

EXECUTABLE=  bridge #mngr
RSTPLIBS = rstp
RSTPLIBF = lib$(RSTPLIBS).a
UIDLIBS  = uid
UIDLIBF  = lib$(UIDLIBS).a
CLILIBS  = cli
CLILIBF  = lib$(CLILIBS).a

DBUS_LIBS = dbus-1

#MNGRCFILES = stp_mngr.c
#MNGROFILES = stp_mngr.o
BRIDGECFILES = stp_dcli.c stp_to.c stp_dbus.c 
BRIDGEOFILES = stp_bridge.o stp_dcli.o stp_to.o stp_dbus.o

RSTPCFILES = stp_statmch.c stp_stpm.c stp_port.c stp_mgmt.c stp_in.c stp_rolesel.c \
stp_vector.c stp_times.c stp_receive.c\
stp_portinfo.c stp_roletrns.c stp_sttrans.c stp_topoch.c stp_migrate.c stp_transmit.c \
stp_p2p.c stp_edge.c stp_pcost.c stp_md5.c stp_bitmap.c stp_param.c stp_log.c stp_npd_cmd.c stp_uid.c
RSTPOFILES = stp_statmch.o stp_stpm.o stp_port.o stp_mgmt.o stp_in.o stp_rolesel.o \
stp_vector.o stp_times.o stp_receive.o\
stp_portinfo.o stp_roletrns.o stp_sttrans.o stp_topoch.o stp_migrate.o stp_transmit.o \
stp_p2p.o stp_edge.o stp_pcost.o stp_md5.o stp_bitmap.o stp_param.o stp_log.o stp_npd_cmd.o stp_uid.o

UIDCFILES = stp_uid_sock.c
UIDOFILES = stp_uid_sock.o

CLICFILES = stp_cli.c
CLIOFILES = stp_cli.o

CFILES=  $(BRIDGECFILES) $(RSTPCFILES) $(UIDCFILES) $(CLICFILES)

OFILES=  $(BRIDGEOFILES) $(RSTPOFILES) $(UIDOFILES) $(CLIOFILES)

.SUFFIXES: .S .c .o .s

all:prepare $(EXECUTABLE)

#mngr: ${MNGROFILES} $(UIDLIBF) $(CLILIBF)
#	$(CC) ${MNGROFILES} $(LIBS) $(LDFLAGS) -L. -l$(UIDLIBS) -l$(CLILIBS) -o $(OBJ_DIR)/$@

bridge: ${BRIDGEOFILES} $(RSTPLIBF) $(UIDLIBF)
	$(RM) -f $(OBJ_DIR)/bridge
	$(CC) ${BRIDGEOFILES} $(LIBS) $(LDFLAGS) -L. -l$(RSTPLIBS) -l$(UIDLIBS) -l$(DBUS_LIBS) -o $(OBJ_DIR)/$@
	$(CP) ./*.o $(OBJ_DIR)
	$(RM) ./*.o
	$(CP) ./*.a $(LIB_DIR)
	$(RM) ./*.a

prepare:
	$(MKDIR) -p $(OBJ_DIR)
	$(MKDIR) -p $(LIB_DIR)


$(RSTPLIBF): ${RSTPOFILES}
	-rm -f $@
	$(AR) $(ARFLAGS) $@ $(RSTPOFILES)
	$(RANLIB)  $@
	
$(UIDLIBF): ${UIDOFILES}
	-rm -f $@
	$(AR) $(ARFLAGS) $@ $(UIDOFILES)
	$(RANLIB) $@
	
$(CLILIBF): $(CLIOFILES)
	-rm -f $@
	$(AR) $(ARFLAGS) $@ $(CLIOFILES)
	$(RANLIB) $@

clean:
	$(RM) -f all *.o *.a *~ $(EXECUTABLE) *.bak make.depend
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/*

depend:
	echo "# DO NOT DELETE THIS LINE -- make  depend  depends  on it." > make.depend
	makedepend -- $(DEFS) $(INCLUDES) $(CFLAGS) -- $(CFILES) -f make.depend

-include make.depend
