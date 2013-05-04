#!/bin/bash
if [ $# -eq 4 ] ; then
	if [ "normal" = $4 ] ; then
		password=$(openssl passwd -1 $2)
	else
		password=$2
	fi
	if [ "enable" = $3 ] ; then
		usergroup="vtyadmin"
	else
		usergroup="vtyview"
	fi
	useradd -p $password -G $usergroup -s /opt/bin/vtysh $1
	if [ 0 -eq $? ] ; then
	exit 0
	else
	exit -1
	fi
else
	exit -2
fi
