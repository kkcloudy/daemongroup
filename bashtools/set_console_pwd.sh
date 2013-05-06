#!/bin/sh
#
###########################################################################
#
#              Copyright (C) Autelan Technology
#
#This software file is owned and distributed by Autelan Technology 
#
############################################################################
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################
#
# eag_init
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################
TTYS0PWDFILE="/etc/ttyS0pwd"
TTYS0USRFILE="/etc/ttyS0usr"
source vtysh_start.sh
if [ -e $TTYS0PWDFILE ] ; then 
	rm $TTYS0PWDFILE
fi
if [ -e $TTYS0USRFILE ] ; then
        rm $TTYS0USRFILE
fi

if [ $# -eq 2 ] ; then
	if [ "normal" = "$2" ] ; then
		password=$(echo $1 | openssl  md5)
	else
		password=$1
	fi
	echo "$password" > $TTYS0PWDFILE
	if [ 0 -eq $? ] ; then
		exit 0
	else
		exit -1
	fi
elif [ $# -eq 3 ] ; then
	if [ "normal" = "$3" ] ; then
		password=$(echo $2 | openssl  md5)
	else
		password=$2
	fi
	echo "$1" > $TTYS0USRFILE
	echo "$password" > $TTYS0PWDFILE
	if [ 0 -eq $? ] ; then
		exit 0
	else
		exit -1
	fi

else
	exit -2
fi
