#!/bin/sh

source vtysh_start.sh

if [ $# -eq 4 ] ; then
	cd /mnt/wtp
	if [ -a $4 ] ; then
		rm $4
	fi
	sudo wget --user=$2 --password=$3 $1
else
	exit -1
fi
