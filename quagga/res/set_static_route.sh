#!/bin/bash
if [ $# -eq 3 ] ; then
	cmdstr=" configure terminal
	ip route $1 $2 $3
	"
	echo "$cmdstr"
	vtysh -c "$cmdstr"
	echo $?
	if [ 0 -eq $? ] ; then
		exit 0
	else
		exit 1
	fi
else
	echo "error paraments number"
	exit 1
fi
