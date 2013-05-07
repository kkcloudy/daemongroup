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


###########################################################################
# Makefile for building M3UA library.
# Use __LITTLE_ENDIAN__ while compiling over little endian architecture.
# This makefile has been used for i586 architecture. For SPARC machines
# remove __LITTLE_ENDIAN__ flag from CFLAGS.
# ------------------------------------------------------------------------
# To build dependencies, use "make depend"
# To build M3UA library, use "make"
###########################################################################

M3PATH 		= ..

M3SRCPATH	= ../src
#M3INCPATH	= ../inc
M3INCPATH	= ../../../include/m3ua/inc/
M3BUILDPATH	= .
LIB_EXPORT_DIR	= ${BUILD_DIR}/rootfs/files/usr/lib/
SRCFILES	= 	$(M3SRCPATH)/m3ua_as.c		\
			$(M3SRCPATH)/m3ua_asp.c		\
			$(M3SRCPATH)/m3ua_conn.c	\
			$(M3SRCPATH)/m3ua_init.c	\
			$(M3SRCPATH)/m3ua_lrout.c	\
			$(M3SRCPATH)/m3ua_ntfy.c	\
			$(M3SRCPATH)/m3ua_nwapp.c	\
			$(M3SRCPATH)/m3ua_ras.c		\
			$(M3SRCPATH)/m3ua_rasp.c	\
			$(M3SRCPATH)/m3ua_route.c	\
			$(M3SRCPATH)/m3ua_rsgp.c	\
			$(M3SRCPATH)/m3ua_sg.c		\
			$(M3SRCPATH)/m3ua_sgp.c		\
			$(M3SRCPATH)/m3ua_usr.c		\
			$(M3SRCPATH)/m3ua_rkey.c	\
			$(M3SRCPATH)/m3ua_as_proc.c	\
			$(M3SRCPATH)/m3ua_aspsm.c	\
			$(M3SRCPATH)/m3ua_asptm.c	\
			$(M3SRCPATH)/m3ua_asp_proc.c\
			$(M3SRCPATH)/m3ua_co_proc.c	\
			$(M3SRCPATH)/m3ua_mgmt.c	\
			$(M3SRCPATH)/m3ua_rkm.c		\
			$(M3SRCPATH)/m3ua_ssnm.c	\
			$(M3SRCPATH)/m3ua_ssnm_ui.c	\
			$(M3SRCPATH)/m3ua_txr.c		\
			$(M3SRCPATH)/m3ua_ui.c		\
			$(M3SRCPATH)/m3ua_tran.c	\
			$(M3SRCPATH)/m3ua_msg_dcd.c	\
			$(M3SRCPATH)/m3ua_msg_ecd.c	\
			$(M3SRCPATH)/m3ua_msg_proc.c\
			$(M3SRCPATH)/m3uaTimerMgr.c	\
			$(M3SRCPATH)/m3uaTraceMgr.c	\
			$(M3SRCPATH)/m3uaMemMgr.c	\
			$(M3SRCPATH)/m3ua_diag.c

INCLUDE		= 	-I$(M3INCPATH)\
			-I ../../../include/sccp/include/sccp\
			-I /opt/mipslibs/usr/include\
                        -I ../../../include/sccp/include/include

CFLAGS =		$(INCLUDE) -D__BIG_ENDIAN__

#CC	=		mips-linux-gnu-gcc -c -g -Wall -Wstrict-prototypes
CC	=		mips-linux-gnu-gcc

OBJFILES =		$(M3SRCPATH)/m3ua_as.o			\
			$(M3SRCPATH)/m3ua_asp.o		\
			$(M3SRCPATH)/m3ua_conn.o		\
			$(M3SRCPATH)/m3ua_init.o		\
			$(M3SRCPATH)/m3ua_lrout.o		\
			$(M3SRCPATH)/m3ua_ntfy.o		\
			$(M3SRCPATH)/m3ua_nwapp.o		\
			$(M3SRCPATH)/m3ua_ras.o		\
			$(M3SRCPATH)/m3ua_rasp.o		\
			$(M3SRCPATH)/m3ua_route.o		\
			$(M3SRCPATH)/m3ua_rsgp.o		\
			$(M3SRCPATH)/m3ua_sg.o			\
			$(M3SRCPATH)/m3ua_sgp.o		\
			$(M3SRCPATH)/m3ua_usr.o		\
			$(M3SRCPATH)/m3ua_rkey.o		\
			$(M3SRCPATH)/m3ua_as_proc.o		\
			$(M3SRCPATH)/m3ua_aspsm.o		\
			$(M3SRCPATH)/m3ua_asptm.o		\
			$(M3SRCPATH)/m3ua_asp_proc.o	\
			$(M3SRCPATH)/m3ua_co_proc.o	\
			$(M3SRCPATH)/m3ua_mgmt.o		\
			$(M3SRCPATH)/m3ua_rkm.o		\
			$(M3SRCPATH)/m3ua_ssnm.o		\
			$(M3SRCPATH)/m3ua_ssnm_ui.o		\
			$(M3SRCPATH)/m3ua_txr.o		\
			$(M3SRCPATH)/m3ua_ui.o			\
			$(M3SRCPATH)/m3ua_tran.o			\
			$(M3SRCPATH)/m3ua_msg_dcd.o		\
			$(M3SRCPATH)/m3ua_msg_ecd.o		\
			$(M3SRCPATH)/m3ua_msg_proc.o		\
			$(M3SRCPATH)/m3uaTimerMgr.o	\
			$(M3SRCPATH)/m3uaTraceMgr.o	\
			$(M3SRCPATH)/m3uaMemMgr.o	\
			$(M3SRCPATH)/m3ua_diag.o

target =		libm3ua.so

$(target):		$(OBJFILES)
			$(CC) $(INCLUDE) $(SRCFILES) -shared -L../../sccp/lib -losmocore -o $(target)
			cp *.so $(LIB_EXPORT_DIR)
#			ar rv libm3ua.a $(OBJFILES)

all:	$(target)

depend:
	makedepend -- $(CFLAGS) -- $(SRCFILES)

clean:
	\rm -f $(OBJFILES) $(target)

