#!/bin/sh

source vtysh_start.sh
tmplst=/tmp/`whoami``ls /proc/self/task/`.lst
tmpcnt=$tmplst.1
rm -rf $tmplst*

iplist=$(arp -n | awk -v cntfile=$tmpcnt '$1!="Address" {print $1} END { print (NR-1) > cntfile }')

ip_count=`cat $tmpcnt`

echo "Total $ip_count src ip found"

i=0
for ip1 in $iplist
do
	#echo "$ip1 `conntrack -L | grep src=$ip1 | wc -l`" >>$tmplst 
	echo "$ip1	`cat /proc/net/ip_conntrack | grep src=$ip1 | wc -l`" >>$tmplst 
	i=$(($i+1))
	per=$(($i*100/$ip_count))
#	echo "i=$i per=$per"
	echo -en "$per%\r"
done

echo "IP Address	Connection Count"
sort -k2nr $tmplst 

rm -rf $tmplst

echo "Total Theory Connection Count [`cat /proc/sys/net/ipv4/netfilter/ip_conntrack_count`]"
