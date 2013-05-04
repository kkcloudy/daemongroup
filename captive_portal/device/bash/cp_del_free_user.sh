#!/bin/sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: cp_del_free_user.sh ip"
	exit 1
fi

IP=$1

echo "del free user ip=$IP"

sudo /opt/bin/iptables -D FORWARD -d $IP -j ACCEPT
sudo /opt/bin/iptables -D FORWARD -s $IP -j ACCEPT
sudo /opt/bin/iptables -t nat -D PREROUTING -s $IP -j ACCEPT
