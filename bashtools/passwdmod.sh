#!/bin/sh

source vtysh_start.sh

if [ $# -eq 1 ]; then
		cmdstr="configure terminal
		$1"
		vtysh -c "$cmdstr"
		if [ $? -eq 0 ];then
			exit 0
		else
			exit -1
		fi
else
	exit -1
fi
