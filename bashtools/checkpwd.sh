#!/bin/sh
source vtysh_start.sh
if [ $# -eq 3 ]  ; then
#	echo username:$1
#	echo password:$2
#	echo timestamp:$3
#	echo -e "$1\0000$2\0000$3\0000" | /opt/bin/username/checkpassword-pam -s unix --debug --noenv --stdout  3<& 0
#	Turn off debug 
	echo -e "$1\0000$2\0000$3\0000" | checkpassword-pam -s unix --noenv 3<& 0
	if [ 0 -eq $? ] ; then
#		echo "Authentication Successed."
		exit 1
	else
#		echo "Authentication Failed."
		exit 2
	fi
#	chmod 777 /var/run/apache2/login.txt
else
	echo "Usage:./checkpws USERNAME PASSWD TIMESTAMP"
fi

