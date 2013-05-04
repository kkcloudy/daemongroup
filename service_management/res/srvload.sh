#!/bin/sh
source vtysh_start.sh

 
 CON_XML_PATH=/mnt/conf_xml.conf
 CMD_PATH=/opt/bin/srvload
 EXEC_PATH=/opt/bin/srvcmd
 ERR_LOG=/var/run/apache2/err_log.txt
CLI_PATH=/opt/services/conf/cli_conf.conf
START_TRAP=/var/run/conf.trap 
TRAPCONF_PATH=/opt/www/htdocs/trap/trapconf_option
TRAPDATA_PATH=/opt/www/htdocs/trap/trapdata_option
if ! test -f $ERR_LOG
	then
	 	touch $ERR_LOG
		chmod 666 $ERR_LOG
fi

 date>>$ERR_LOG

if ! test -f $START_TRAP
	then
		touch $START_TRAP
		chmod 777 $START_TRAP
fi

printf "Start Parsing Config File ."

if test -s $CON_XML_PATH
       then
                $CMD_PATH 
		ret=$?
		 echo $ret > $START_TRAP 
		if [ $ret -eq 0 ] ; then
			printf "."
		else
			printf "*"
		fi
 fi
printf ".\n"
if test -f $CLI_PATH
	then
	cp $CLI_PATH /mnt/cli.conf

fi
if test -f $TRAPCONF_PATH
	then
	cp $TRAPCONF_PATH /opt/services/option/
	chmod 666 /opt/services/option/trapconf_option
fi
if test -f $TRAPDATA_PATH
	then
	cp $TRAPDATA_PATH /opt/services/option/
	chmod 666 /opt/services/option/trapdata_option
fi
echo "Start Network Base Config...."
if test -f /mnt/cli.conf
	then
		vtysh -b
fi
echo "Start Network Services...."
#$EXEC_PATH 
