#!/bin/sh

OPTIONFILE=/opt/services/option/syslog_option
str1=`awk -F "</*saveblk>" '{print $2}' $OPTIONFILE`

if [ "$str1"x = ""x ];then
        sudo /usr/bin/syslog_date.sh 2 > /dev/null
        cd /var/log/
        sudo tar -cvjf systemlog.tar.gz systemlog > /dev/null         
else	
        sudo /usr/bin/syslog_date.sh $str1 > /dev/null
        cd /var/log/
         sudo tar -cvjf systemlog.tar.gz systemlog > /dev/null        
fi