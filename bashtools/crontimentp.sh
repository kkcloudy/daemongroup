#!/bin/sh
OPTIONFILE=/opt/services/option/ntp_option
serverip=`awk -F "</*cipz>" '{print $2}' $OPTIONFILE`
crontpv=`awk -F "</*ntpv>" '{print $2}' $OPTIONFILE`

ntpstate=`ps -ef|grep /usr/sbin/ntpd|grep -v grep|wc -l`

if [ "$serverip"x = ""x ];then
       exit 1
else		
	if [ "$ntpstate"x = "0"x ];then	
		if [ `echo $crontpv|grep '3'` ];then
			sudo /usr/sbin/ntpdate -o 3 $serverip > /var/run/ntpcron.tmp	2>&1
		else
			sudo /usr/sbin/ntpdate $serverip > /var/run/ntpcron.tmp	2>&1
		fi
		else
		sudo /opt/services/init/ntp_init stop
		sleep 2
		if [ `echo $crontpv|grep '3'` ];then
			sudo /usr/sbin/ntpdate -o 3 $serverip > /var/run/ntpcron.tmp	2>&1
		else
			sudo /usr/sbin/ntpdate $serverip > /var/run/ntpcron.tmp	2>&1
		fi		
		sudo /opt/services/init/ntp_init start
	fi
fi
