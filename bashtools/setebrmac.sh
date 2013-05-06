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

if [ $? -eq 3 ] ; then
	echo "Usage: setebrmac.sh IFNAME VMAC VIP."
fi

BRIF=$1
VMAC=$2
VIP=$3

logfile="/var/log/setmac.log"

date >> $logfile
setifmac()
{
SIF=$1
ifconfig $SIF down >> $logfile 2>&1
ifconfig $SIF hw ether $VMAC >> $logfile 2>&1
ifconfig $SIF up  >> $logfile 2>&1

}


ifconfig $BRIF down >> $logfile 2>&1

for if in /sys/class/net/$BRIF/brif/* ; do 
pureifname=`basename $if`
setifmac $pureifname
done

ifconfig $BRIF up >> $logfile 2>&1
 


arping -c 1 -I $BRIF $VIP >> $logfile 2>&1

echo "setmac done." >> $logfile
date >> $logfile 
