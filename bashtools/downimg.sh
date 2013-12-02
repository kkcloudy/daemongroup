#!/bin/sh

source vtysh_start.sh

if [ $# -eq 4 ] ; then
	cd /mnt/
	if [ -a $4 ] ; then
		rm $4
	fi
	wget --tries=3 --user=$2 --password=$3 $1
#       mv $4 /mnt/aw.img 1 > ~/down.log 2>&1
#        cp $4 /mnt/aw.img 
#	        rm $4
else
#	echo "Usage:downimg.sh URL  USEERNAME PASSWORD FILENAME"
	exit -1
fi
