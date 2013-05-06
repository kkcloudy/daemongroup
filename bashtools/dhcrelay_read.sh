#!/bin/sh

CONF_FILE="/opt/services/option/dhcrelay_option"
INFO_FILE="/var/run/apache2/dhcrelay.msg"
TEMP_FILE="/var/run/apache2/dhcrelay.option"
STATUS_FILE="/opt/services/status/dhcrelay_status.status"

if [ ! -f $STATUS_FILE ];then
        sudo touch $STATUS_FILE
        echo "stop">$STATUS_FILE
fi
       sudo chmod 666 $STATUS_FILE
if [ ! -f $TEMP_FILE ];then
        sudo touch $TEMP_FILE
               
fi
        sudo chmod 666 $TEMP_FILE  
if [ ! -f $INFO_FILE ];then
        sudo touch $INFO_FILE
        sudo chmod 666 $INFO_FILE  
fi
        sudo chmod 666 $INFO_FILE
if [ ! -f $CONF_FILE ];then
        sudo touch $CONF_FILE
        sudo chmod 666 $CONF_FILE
        echo "SERVERS=\"\"">$CONF_FILE
        echo "INTERFACES=\"\"">>$CONF_FILE
        echo "OPTIONS=\"\"">>$CONF_FILE
fi
               

cat $CONF_FILE|awk 'BEGIN{FS="=";RS="\n"}{if($1~/SERVERS/)print $2}'|sed s/\"//g>$INFO_FILE
cat $CONF_FILE|awk 'BEGIN{FS="=";RS="\n"}{if($1~/OPTIONS/)print $2}'|sed s/\"//g>>$INFO_FILE
cat $CONF_FILE|awk 'BEGIN{FS="=";RS="\n"}{if($1~/INTERFACES/)print $2}'|sed s/\"//g>$TEMP_FILE
