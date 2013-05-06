#!/bin/bash

LOGTAG="kick_out_user"

logger -p daemon.info -t $LOGTAG " Process ${PPID} called kick_out_user"

if [ ! $# -eq 1 ] ; then
	echo -e "\nError: Wrong Parameters. \n"
	echo -e "\nUsage: kick_out_use.sh USER_NAME\n"
	logger -p daemon.info -t $LOGTAG "wrong args,exit 1"
	exit 1
fi

USER_NAME=$1
#check out user 
check_user="`who | grep $USER_NAME`"
if [ ! -n "$check_user" ]; then
	logger -p daemon.info -t $LOGTAG "Can't find usr in the system ,exit 2"
	echo can\'t find usr
	exit 2
fi


TTY="`who | awk '{if ($1=="'$USER_NAME'") printf ($2 "\n")}'`"
logger -p daemon.info -t $LOGTAG "step 1:echo all terminal $TTY,and break up it"
i=1
while true ;do

var="`echo $TTY | awk -F ' ' '{printf($'$i')}'`"
if [ -n "$var" ] 
then
	logger -p daemon.info -t $LOGTAG "step 2:take No. $i terminal $var,and get process id"
	
	pid="`ps -ef | awk '{if ($6=="'$var'"&&$8=="-vtysh") printf("%d\n", $2);}'`" 
	logger -p daemon.info -t $LOGTAG "step 3:get No.$i terminal $var process id is $pid,and kill it"
	kill $pid
	i=`expr $i + 1`
else
	logger -p daemon.info -t $LOGTAG "step 4:all terminal are close,exit 0"
	exit 0
fi
done
