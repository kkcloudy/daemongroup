#!/bin/sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: cp_add_free_user.sh ip"
	exit 1
fi

IP=$1

echo "add free user ip=$IP"

sudo /opt/bin/iptables -I FORWARD -d $IP -j ACCEPT
sudo /opt/bin/iptables -I FORWARD -s $IP -j ACCEPT
sudo /opt/bin/iptables -t nat -I PREROUTING -s $IP -j ACCEPT
