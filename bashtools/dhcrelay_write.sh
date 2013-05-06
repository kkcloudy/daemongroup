#!/bin/sh

CONF_FILE="/opt/services/option/dhcrelay_option"
export PATH="$PATH":/sbin:/usr/sbin:/bin:/usr/sbin
#create a config file
if [ ! -f $CONF_FILE ];then 
   touch $CONF_FILE 
   fi
   chmod 777 $CONF_FILE

   echo "SERVERS=\"$1\"">$CONF_FILE
   echo "INTERFACES=\"$2\"">>$CONF_FILE
   echo "OPTIONS=\"$3\"">>$CONF_FILE
exit 0
