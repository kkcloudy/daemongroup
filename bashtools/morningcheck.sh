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

LOGTAG=MORNING

check_dhcpd()
{
DHCPDSTATEF="/opt/services/status/dhcp_status.status"
if [ -f $DHCPDSTATEF ] ; then
dhcps=`cat $DHCPDSTATEF`
	logger -t $LOGTAG "Dhcpd status [$dhcps]."
	if [ x$dhcps = x"start" ] ; then
		logger -t $LOGTAG -p cron.info "Trying to restart dhcpd."
		/opt/services/init/dhcp_init restart
		logger -t $LOGTAG -p cron.info "Restarted dhcpd."
	else
		logger -t $LOGTAG -p cron.info "Dhcpd not started."
	fi

fi
}

check_snmpd()
{
SNMPDSTATEF="/opt/services/status/snmpd_status.status"
if [ -f $SNMPDSTATEF ] ; then
	snmpds=`cat $SNMPDSTATEF`
	logger -t $LOGTAG "Snmpd status [$snmpds]."
	if [ x$snmpds = x"start" ] ; then
		logger -t $LOGTAG -p cron.info "Trying to restart snmpd."
#		/opt/services/init/snmpd_init stop
		/opt/services/init/snmpd_init restart
		logger -t $LOGTAG -p cron.info "Restarted snmpd."
	else
		logger -t $LOGTAG -p cron.info "Snmpd not started."
	fi
fi
}

check_sad()
{
logger -t $LOGTAG -p cron.info "Trying to stop sad."
#start-stop-daemon -p /var/run/sad.sh.pid --stop --exec /usr/bin/sad.sh
#changed by huxuefeng20120228
#reason:start-stop-daemon -p /var/run/sad.sh.pid --stop --exec /usr/bin/sad.sh couldn't kill the process.
start-stop-daemon --stop --quiet --pidfile /var/run/sad.sh.pid
rm -rf /var/run/sad/*
rm -rf /var/run/sad.sh.pid
logger -t $LOGTAG -p cron.info "Stopped sad."
logger -t $LOGTAG -p cron.info "Trying to start sad."
start-stop-daemon -b -m -p /var/run/sad.sh.pid --start --exec /usr/bin/sad.sh
logger -t $LOGTAG -p cron.info "Started sad."
}


################################################################
#We don't restart dhcp server since we use new version of dhcpd.

#DHCRESETF="/var/run/dhcpdreset"
#if [ -f $DHCRESETF ] ; then
#  DHCRESETV=`cat $DHCRESETF`
#  if [ x$DHCRESETV = x"0" ] ; then
#    logger -t $LOGTAG -p cron.info "Morning check was disabled."
#    exit 0
#  fi
#fi

logger -t $LOGTAG -p cron.info "Morning check."

#check_dhcpd
check_snmpd
check_sad
