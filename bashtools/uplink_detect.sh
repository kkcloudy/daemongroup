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

if [ ! $# -eq 5 ] ; then 
	echo "Usage: uplink_detect.sh UPLINKIPADDR WLANID INSTID SLOTID ISLOCAL"
	exit 1;
fi

UPLINKIP=$1
OPWLANID=$2
VRRPID=$3
SLOTID=$4
ISLOCAL=$5
PATH=/var/run/uplink_detect$5-$4-$3-$2
# suppose initial status is ok.
STATUS=1

echo 1 > $PATH 

restore_wlan()
{
	if [ $VRRPID -eq 0 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config wlan $OPWLANID
	service enable" 1>/dev/null 2>&1
	elif [ $ISLOCAL -eq 0 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config hansi-profile $SLOTID-$VRRPID
	config wlan $OPWLANID
	service enable" 1>/dev/null 2>&1 
	elif [ $ISLOCAL -eq 1 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config local-hansi $SLOTID-$VRRPID
	config wlan $OPWLANID
	service enable" 1>/dev/null 2>&1 
	fi
}

pause_wlan()
{
	if [ $VRRPID -eq 0 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config wlan $OPWLANID
	service disable" 1>/dev/null 2>&1
	elif [ $ISLOCAL -eq 0 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config hansi-profile $SLOTID-$VRRPID
	config wlan $OPWLANID
	service disable" 1>/dev/null 2>&1 
	elif [ $ISLOCAL -eq 1 ] ; then
	/opt/bin/vtysh -c "configure terminal 
	config local-hansi $SLOTID-$VRRPID
	config wlan $OPWLANID
	service disable" 1>/dev/null 2>&1 
	fi
}

while true 
do

/bin/sleep 5

# Check whether this script should be stopped.
stop=`/bin/cat $PATH`
if [ $stop -eq 0 ] ; then
restore_wlan
exit 0
fi

/bin/ping -c 1 $1 1>/dev/null 2>&1
# if uplink addr is reachable
if [ $? -eq 0 ] ; then
# if status is 0, down , then restore network
#	if [ $STATUS -eq 1 ] ; then
		restore_wlan
#	fi
else
#if uplink addr is not reachable
# if status is 1, up, then pause network
#	if [ $STATUS -eq 1 ] ; then
		pause_wlan
#	fi
fi


done
