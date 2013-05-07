#############################################################################
## Description:
##
###########################################################################

export FGW_MOD="src/fgw"

export GCC		= mips-linux-gnu-gcc

PREREQ_FGW=$(shell if [ -d ${FGW_MOD} ] ; then echo "fgw"; fi )
CLEAN_FGW=$(subst fgw,cleanfgw,${PREREQ_FGW})

EXPORT_EXE_DIR	= ${BUILD_DIR}/rootfs/files/usr/lib/
M3SRCPATH	= ../../../lib/m3ua/src 
RANAPPATH	= ../../../lib/asn1/ranap 
M3INCPATH	= ../../../include/m3ua/inc
APPLPATH 		= .
MAKE		= make
UDPSRCFILES	=	$(APPLPATH)/src/timer.c		\
			$(APPLPATH)/src/udp.c\
			$(APPLPATH)/src/confudp.c

SCTPSRCFILES	=	$(APPLPATH)/src/sctp.c		\
			$(APPLPATH)/src/timer.c		\
			$(APPLPATH)/src/iu_log.c	\
			$(APPLPATH)/src/iu_dbus.c

ASPSRCFILES	= 	$(APPLPATH)/src/confasp1.c
SGPSRCFILES	=	$(APPLPATH)/src/confsgp1.c
SCCP_DIR = ../../../lib/sccp/src
M3UA_DIR = ../../../lib/m3ua/src
SCCP_LIB = ../../../lib/sccp/lib
INCLUDE		= 	-I$(M3INCPATH)	\
			-I$(APPLPATH)/inc\
			-I ../../../include/sccp/include/sccp\
			-I ../../../include/ranapd/include\
			-I /opt/mipslibs/usr/include\
                        -I ../../../include/sccp/include/include\
			-I ${ACCAPI_DIR} 

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
DBUS_LIBS=-ldbus-1

all:
	$(MAKE) ranap
	$(MAKE) sccp
	$(MAKE) m3ua
	$(MAKE) sctp

ranap:
	$(MAKE) -C $(RANAPPATH)

sccp:
	$(MAKE) -C $(SCCP_DIR)

m3ua:
	$(MAKE) -C $(M3UA_DIR)

asp:	$(ASPSRCFILES) $(UDPSRCFILES) $(M3SRCPATH)/libm3ua.a
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(ASPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o asp -lpthread -lnsl confasp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl

sgp:	$(SGPSRCFILES) $(UDPSRCFILES) $(M3SRCPATH)/libm3ua.a
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(SGPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o sgp -lpthread -lnsl confsgp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl

#sctp:	$(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES) $(M3SRCPATH)/libm3ua.a
sctp:	$(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES)
	$(GCC) -Wall -Wstrict-prototypes -D_DISTRIBUTION_ $(INCLUDE) $(DBUS_INCLUDE) -c $(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES)
	$(GCC) -o ranapproxy -lpthread -lsctp confasp1.o timer.o sctp.o iu_dbus.o iu_log.o -L$(M3SRCPATH) -lm3ua -L/opt/mipslibs/usr/lib -lsctp -L$(SCCP_LIB) -losmocore -L$(SCCP_DIR) -lsccp -L../../../lib/asn1/ranap -lranap $(DBUS_LIBS) -lnsl
	cp ranapproxy $(EXPORT_EXE_DIR)
#	$(GCC) -o sgp -lpthread -lsctp confsgp1.o timer.o sctp.o -L$(M3SRCPATH) -lm3ua -L/opt/mipslibs/usr/lib -lsctp -losmocore -L$(SCCP_DIR) -lsccp -L$(RANAPPATH) -lranap -lnsl

udp:	$(ASPSRCFILES) $(SGPSRCFILES) $(UDPSRCFILES) 
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(ASPSRCFILES) $(SGPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o sgp -lpthread -lnsl confsgp1.o timer.o udp.o -lnsl

clean:	
	$(MAKE) clean -C $(SCCP_DIR)
	$(MAKE) clean -C $(M3UA_DIR)
	\rm -rf ranapproxy asp sgp *.o

cleanranap:
	$(MAKE) clean -C $(RANAPPATH)

cleansccp:
	$(MAKE) clean -C $(SCCP_DIR)

cleanm3ua:
	$(MAKE) clean -C $(M3UA_DIR)

cleanfgw:
	$(MAKE) clean -C $(SCCP_DIR)
	$(MAKE) clean -C $(M3UA_DIR)
	$(MAKE) clean -C $(RANAPPATH)
	\rm -rf asp sgp *.o

