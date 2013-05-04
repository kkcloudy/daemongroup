#############################################################################
## Description:
##
#############################################################################
## Copyright(C) 2009 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
#############################################################################
## Contact:
## vkgupta@shabdcom.org
#############################################################################
## License :
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##############################################################################



SCCPPATH 		= ..

SCCPSRCPATH	= ../src
#SCCPINCPATH	= ../inc
SCCPINCPATH	= ../../../include/sccp/include
SCCPBUILDPATH	= .
LIB_EXPORT_DIR	= ../../../../../rootfs/files/usr/lib/ 
SRCFILES	= 	$(SCCPSRCPATH)/mtp_pcap.c		\
			$(SCCPSRCPATH)/mtp_level3.c		\
			$(SCCPSRCPATH)/sccp.c	


INCLUDE		= 	-I$(SCCPINCPATH)\
			-I ../../../include/m3ua/inc\
			-I ../../../include/ranapd/include\
			-I ../../../app/ranapserver/sigtran2udp/inc\
			-I ../../../include/sccp/include/include\
                        -I /opt/mipslibs/usr/include\
			-I ../../../../accapi/\
			-I ../../asn1/ranap/\
			-I ../../asn1/pub/

#CFLAGS =		$(INCLUDE) -D__LITTLE_ENDIAN__
CFLAGS =		$(INCLUDE)

#CC	=		gcc -c -g -Wall -Wstrict-prototypes
#CC	=		mips-linux-gnu-gcc -c -shared -L/opt/mipslibs/usr/lib -losmocore
CC	=		mips-linux-gnu-gcc 


OBJFILES	= 	$(SCCPSRCPATH)/mtp_pcap.o		\
			$(SCCPSRCPATH)/mtp_level3.o		\
			$(SCCPSRCPATH)/sccp.o	

target =		libsccp.so

$(target):		$(OBJFILES)
#			$(CC) -c $(INCLUDE) $(SRCFILES)
#	   		$(CC) -shared -L/../lib -losmocore -o $(OBJFILES) $(target)
			$(CC) $(INCLUDE) $(SRCFILES) -shared -L../lib -losmocore -L../../asn1/ranap -lranap -o $(target)
			cp *.so $(LIB_EXPORT_DIR)
			cp ../lib/libosmocore* $(LIB_EXPORT_DIR)
#			ar rv libsccp.a $(OBJFILES)

all:	$(target)

depend:
	makedepend -- $(CFLAGS) -- $(SRCFILES)

clean:
	\rm -f $(OBJFILES) $(target)

