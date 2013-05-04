#!/bin/bash
if [ $# -eq 0 ] ; then
   	vtysh -c "show ip route"
	if [ 0 -eq $? ] ; then
	 exit 0
	 else
	 exit 1
	 fi

else
	exit 1
fi
