#!/bin/bash
if [ $# -eq 3 ] ; then
        cmdstr="configure terminal
	       no ip route $1 $2 $3
		"
	
	vtysh -c "$cmdstr"
	if [ 0 -eq $? ] ; then
		exit 0
	else
		exit 1
	fi
else
	echo "error paraments number"
	exit 1
fi
															
