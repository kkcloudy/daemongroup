#############################################################################
## Description:
##
###########################################################################

#export FGW_MOD="src/fgw"

export GCC		= mips-linux-gnu-gcc

PREREQ_FGW=$(shell if [ -d ${FGW_MOD} ] ; then echo "fgw"; fi )
CLEAN_FGW=$(subst fgw,cleanfgw,${PREREQ_FGW})


M3SRCPATH	= ../../../lib/m3ua/src 
RANAPPATH	= ../../../lib/ranapd 
M3INCPATH	= ../../../include/m3ua/inc
APPLPATH 	= .
MAKE		= make
UDPSRCFILES	=	$(APPLPATH)/src/timer.c		\
			$(APPLPATH)/src/udp.c

SCTPSRCFILES	=	$(APPLPATH)/src/sctp.c		\
			$(APPLPATH)/src/timer.c		\
			$(APPLPATH)/src/sigtran2udp_log.c\
			$(APPLPATH)/src/sigtran2udp_dbus.c

TRANSSRCFILES	=	$(APPLPATH)/src/trans.c		\
				$(APPLPATH)/src/timer.c

ASPSRCFILES	= 	$(APPLPATH)/src/confasp1.c
SGPSRCFILES	=	$(APPLPATH)/src/confsgp1.c
SCCP_DIR = ../../../lib/sccp/src
M3UA_DIR = ../../../lib/m3ua/src
RANAP_DIR = ../../../lib/asn1/ranap
SCCP_LIB = ../../../lib/sccp/lib
INCLUDE		= 	-I$(M3INCPATH)	\
			-I$(APPLPATH)/inc\
			-I ../../../include/sccp/include/sccp\
			-I ../../../include/ranapd/include\
			-I /opt/mipslibs/usr/include\
			-I ../../../include/sccp/include/include\
			-I ../../../../accapi

DBUS_INCLUDE=-I/opt/mipslibs/usr/include/dbus-1.0 -I/opt/mipslibs/usr/lib/dbus-1.0/include
DBUS_LIBS=-ldbus-1

all:	
	$(MAKE) sccp
	$(MAKE) m3ua 
	$(MAKE) sigtran2udp 

#ranap:
#	$(MAKE) -C $(RANAPPATH)

sccp:
	$(MAKE) -C $(SCCP_DIR)

m3ua:
	$(MAKE) -C $(M3UA_DIR)

asp:	$(ASPSRCFILES) $(UDPSRCFILES) $(M3SRCPATH)/libm3ua.a
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(ASPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o sigtran2udp -lpthread -lnsl confasp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl

sgp:	$(SGPSRCFILES) $(UDPSRCFILES) $(M3SRCPATH)/libm3ua.a
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(SGPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o sgp -lpthread -lnsl confsgp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl

#sctp:	$(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES) $(M3SRCPATH)/libm3ua.a
sigtran2udp:	$(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES) $(TRANSSRCFILES)
	$(GCC) -Wall -Wstrict-prototypes $(INCLUDE) $(DBUS_INCLUDE) -c $(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES) $(TRANSSRCFILES)
	$(GCC) -o sigtran2udp -lpthread -lsctp confsgp1.o sigtran2udp_dbus.o sigtran2udp_log.o timer.o trans.o -L$(M3SRCPATH) -lm3ua -L/opt/mipslibs/usr/lib -lsctp -L$(SCCP_LIB) -losmocore -L$(SCCP_LIB) -lsccp -L$(RANAP_DIR) -lranap -L$(SCCP_DIR) $(DBUS_LIBS) -lnsl

sctp:	$(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES)
	$(GCC) -Wall -Wstrict-prototypes $(INCLUDE)  -c $(ASPSRCFILES) $(SGPSRCFILES) $(SCTPSRCFILES)
	$(GCC) -o asp -lpthread -lsctp confasp1.o timer.o sctp.o -L$(M3SRCPATH) -lm3ua -L/opt/mipslibs/usr/lib -lsctp -losmocore -L$(SCCP_DIR) -lsccp -lnsl
	$(GCC) -o sgp -lpthread -lsctp confsgp1.o timer.o sctp.o -L$(M3SRCPATH) -lm3ua -L/opt/mipslibs/usr/lib -lsctp -losmocore -L$(SCCP_DIR) -lsccp -L$(RANAPPATH) -lranap   -lnsl

udp:	$(ASPSRCFILES) $(SGPSRCFILES) $(UDPSRCFILES) $(M3SRCPATH)/libm3ua.a
	$(GCC) -Wall -Wstrict-prototypes -D__UDP__ $(INCLUDE) -c $(ASPSRCFILES) $(SGPSRCFILES) $(UDPSRCFILES)
	$(GCC) -o asp -lpthread -lnsl confasp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl
	$(GCC) -o sgp -lpthread -lnsl confsgp1.o timer.o udp.o -L$(M3SRCPATH) -lm3ua -lnsl

clean:	
	\rm -rf sigtran2udp asp sgp *.o

#cleanranap:
#	$(MAKE) clean -C $(RANAPPATH)

cleansccp:
	$(MAKE) clean -C $(SCCP_DIR)

cleanm3ua:
	$(MAKE) clean -C $(M3UA_DIR)

cleanall:
	$(MAKE) clean -C $(SCCP_DIR)
	$(MAKE) clean -C $(M3UA_DIR)
	\rm -rf sigtran2udp asp sgp *.o

