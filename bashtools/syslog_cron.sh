#!/bin/sh
OPTIONFILE=/opt/services/option/syslog_option
str1=`awk -F "</*savecycle>" '{print $2}' $OPTIONFILE`
strblk1=`awk -F "</*saveblk>" '{print $2}' $OPTIONFILE`

if [ "$str1"x = ""x ];then
        sudo /usr/bin/syslog_date.sh 2 > /dev/null
        str2=`echo '00 00 * * * sudo /usr/bin/syslog_date.sh 2'`
else
	nums=`expr $str1 - 1`
        sudo /usr/bin/syslog_date.sh $str1 > /dev/null
        str2=`echo '00 00 * * * sudo /usr/bin/syslog_date.sh' $nums`
fi

strblk2=`echo '00 00 * * * sudo /usr/bin/syslog_save.sh'`

(crontab -l 2>&- | grep -v 'syslog'
         echo "$str2"        
        ) | crontab - 2>&-
