#!/bin/sh

cmd=`ps -ef|grep /usr/sbin/ntpd|grep -v grep|wc -l`
echo $cmd
if [ $cmd -eq 0 ]; then
    sudo ntpdate $1
fi
if [ $cmd -eq 1 ]; then
     sudo /opt/services/init/ntp_init stop
     sudo ntpdate $1
     sudo /opt/services/init/ntp_init start
fi
