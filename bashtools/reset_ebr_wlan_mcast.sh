#!/bin/sh
find /proc/sys/net/ipv4/neigh/ebr* -name mcast_solicit > /mnt/temp
find /proc/sys/net/ipv4/neigh/wlan* -name mcast_solicit >> /mnt/temp
while read line
do
	echo 1 > $line     #set the value
done < /mnt/temp

sleep 300
while read line
do
	echo 0 > $line     #set the value
done < /mnt/temp

rm /mnt/temp
find /proc/sys/net/ipv4/neigh/ebr* -name mcast_solicit | xargs cat
echo "set ebr mcast_solicit done."
