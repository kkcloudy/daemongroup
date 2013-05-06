#!/bin/bash
# if_cfg_subif.sh add/del PORTNO tag
source vtysh_start.sh

if [ $# -eq 3 ] ; then
	if [ $1 == "add" ];then
		cmdstr="
		configure terminal
		interface $2.$3 
		"
	elif [ $1 == "del" ];then
		cmdstr="
		configure terminal
		no interface $2.$3 
		"
	else
		exit -100
	fi
	
#	echo $cmdstr
	vtysh -c "$cmdstr"
	exit $?
else
	exit -200
fi


