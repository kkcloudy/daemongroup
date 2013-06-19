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


STATUS_PATH=/opt/services/status/snmpd_status.status
TRAP_STATUS_PATH=/opt/services/status/trap-helper_status.status
LOGTAG="snmpcheck"
LOGTRAPTAG="trapcheck"
SUBAGENT_MAXMEM=30000
SNMPD_MAXMEM=200000
TRAP_MAXMEM=300000
SNMPD_PORTMAX=10000000
COREFILE_MAX=200000
snmpexceprstarttimes=0
snmpdrestarttimes=0
traprestarttimes=0
trapexceprstarttimes=0
logger -p daemon.info -t $LOGTAG "Snmp check start."
while true
do
	sleep 50
	ps -ef | grep snmpd | grep -v grep
	result_snmpd=$?
	if [ $result_snmpd -eq 0 ];then
		Spid=`ps -C snmpd -o pid= | sed '2,200d'`
		memSLoad=`pmap -x $Spid | grep "total" | awk '{print $3}'`
		if [ $memSLoad -gt $SNMPD_MAXMEM ];then
			logger -p daemon.warning -t $LOGTAG "Snmpd uses more than $SNMPD_MAXMEM mem, try to restart it."
			sudo /opt/services/init/snmpd_init restart >/dev/null 2>&1 &
			snmpdrestarttimes=$(($snmpdrestarttimes+1))
			echo "snmp mem exceed max value times is:"$snmpdrestarttimes--"time is:"`date` >>/var/log/snmp_restart_times.log
		fi
		
		sleep 50
		
		portSLoad=`sudo netstat -nap | grep "snmpd" | grep ":161" | awk '{print $2}'`
		if [ $portSLoad -gt $SNMPD_PORTMAX ];then
			Spid=`ps -C snmpd -o pid= | sed '2,200d'`
			logger -p daemon.warning -t $LOGTAG "161 port of snmpd is blocking, more than $SNMPD_PORTMAX pkt, try to restart snmpd."
			sudo kill -9 $Spid
			sudo /opt/services/init/snmpd_init start >/dev/null 2>&1 &
			snmpdrestarttimes=$(($snmpdrestarttimes+1))
			echo "The port of snmpd is blocking:"$snmpdrestarttimes--"time is:"`date` >>/var/log/snmp_restart_times.log
		fi
	else
		if [ -e $STATUS_PATH ];then
			value=`cat $STATUS_PATH`
			if [ x$value == x"start" ];then
				sudo /opt/services/init/snmpd_init start >/dev/null 2>&1 &				
				snmpexceprstarttimes=$(($snmpexceprstarttimes+1))
				echo "snmp exception exit times is :"$snmpexceprstarttimes--"time is :"`date`>>/var/log/snmp_restart_times.log
			fi
		fi
	fi
	
	sleep 50
	
	ps -ef | grep trap-helper | grep -v grep
	result_trap=$?
	if [ $result_trap -ne 0 ];then
		if [ -e $TRAP_STATUS_PATH ];then
			value=`cat $TRAP_STATUS_PATH`
			if [ x$value == "xstart" ];then
				logger -p daemon.warning -t $LOGTRAPTAG "trap exception exit, try to start it."
				sudo /opt/bin/trap-helper >/dev/null 2>&1 &
				trapexceprstarttimes=$(($trapexceprstarttimes+1))				
				echo "trap-helper exception exit times is:"$trapexceprstarttimes--"time is:"`date` >>/var/log/trap_restart_times.log
			fi
		fi
	else
		Spid=`ps -C trap-helper -o pid= | sed '2,200d'`
		memSLoad=`pmap -x $Spid | grep "total" | awk '{print $3}'`
		if [ $memSLoad -gt $TRAP_MAXMEM ];then
			logger -p daemon.warning -t $LOGTRAPTAG "trap uses more than $TRAP_MAXMEM mem, try to restart it."
			kill -9 $Spid
			sleep 1
			sudo /opt/bin/trap-helper >/dev/null 2>&1 &
			traprestarttimes=$(($traprestarttimes+1))
			echo "trap-helper mem exceed max value times is:"$traprestarttimes--"time is:"`date` >>/var/log/trap_restart_times.log
		fi
	fi
		        
	coreSload=`du -s /opt/bin/core/ | awk '{print $1}'`
		if [ $coreSload -gt $COREFILE_MAX ];then
		sudo rm /opt/bin/core/*	
	fi
	
	sleep 150
	
done
