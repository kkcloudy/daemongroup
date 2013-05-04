CON_PATH=/opt/services/conf/	
INIT_PATH=/opt/services/init/ 		
OPTION_PATH=/opt/services/option/	
STATUS_PATH=/opt/services/status/
CON_XML_PATH=conf_xml.conf
CMD_PATH=/opt/bin/srvsave
CLI_PATH=/mnt/cli.conf
CLI_STA_PATH=/opt/services/status/cli_status.status
CLI_CON_PATH=/opt/services/conf/cli_conf.conf
ERR_LOG=/var/run/apache2/err_log.txt
date>>$ERR_LOG
if ! test -d $CON_PATH 
	then
		mkdir -p $CON_PATH
		echo creat $CON_PATH sucessful!>>$ERR_LOG 
fi
if ! test -d $INIT_PATH 
	then
		mkdir -p $INIT_PATH
		echo creat $INIT_PATH sucessful!>>$ERR_LOG 
fi

if ! test -d $OPTION_PATH 
	then

		mkdir -p $OPTION_PATH
		echo creat $OPTION_PATH sucessful!>>$ERR_LOG 
fi
if ! test -d $STATUS_PATH 
	then
		mkdir -p $STATUS_PATH
		echo creat $STATUS_PATH sucessful!>>$ERR_LOG 
fi

#/usr/bin/save_config.sh > /dev/null

if test -f $CLI_PATH
	then
#		if ! test -f $CLI_STA_PATH
#			then
#				touch $CLI_STA_PATH
#		fi
#		if ! test -f $CLI_CON_PATH
#			then
#				touch $CLI_CON_PATH
#		fi
#		sudo chmod 666 $CLI_STA_PATH
#		sudo chmod 666 $CLI_CON_PATH
#		echo 1 >  $CLI_STA_PATH
#		cp $CLI_PATH $CLI_CON_PATH
		cp $CLI_PATH /mnt/$CON_XML_PATH >> $ERR_LOG 2>&1
fi

. /usr/bin/libcritlog.sh

crit_sysop_log " EXEC write config."

#        $CMD_PATH >>$ERR_LOG
      	/usr/bin/sor.sh cp $CON_XML_PATH 30

		


