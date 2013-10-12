#!/bin/sh
mcast_file=/proc/sys/net/ipv4/neigh/$1/mcast_solicit
if [ -f $mcast_file ];then
	echo $2 > $mcast_file
	echo $mcast_file
fi
echo "set global mcast_socilit done."	
