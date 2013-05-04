#!/bin/bash

ADMINGROUP="vtyadmin"
VIEWGROUP="vtyview"

if [ $# -eq 2 ] ; then
	user=$1
	
	if [ "enable" = $2 ] ; then
		usergroup=$ADMINGROUP
		delgroup=$VIEWGROUP
	else
		usergroup=$VIEWGROUP
		delgroup=$ADMINGROUP
	fi
# Sometimes the latest changes in /etc/group could not be showed in the result of id command, so we use cat /etc/group directly.	
#	id $user -a | grep $usergroup
	cat /etc/group | grep $usergroup | grep $user >> /dev/null
	if [ 0 -eq $? ] ; then
		echo "User $user already belong to group $usergroup"
		exit -2
	else
#		id $user -a | grep $delgroup
		cat /etc/group | grep $delgroup | grep $user >> /dev/null
		if [ 0 -eq $? ] ; then
			usermod -G $usergroup $user
			if [ 0 -eq $? ] ; then
			exit 0
			else
			exit -1
			fi
		else
		exit -2
		fi	
	fi
else
	echo "Usage: userrole.sh USERNAME [view|enable]"
	exit -2
fi
