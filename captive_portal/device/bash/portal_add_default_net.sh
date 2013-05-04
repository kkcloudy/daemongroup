#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: portal_add_default_net.sh NETWORK"
	exit
fi

. portal_get_cpid.sh

ALLOWNET=$1


iptables -I $CP_FILTER_DEFAULT --dst $ALLOWNET -j ACCEPT
iptables -t nat -I $CP_NAT_DEFAULT --dst $ALLOWNET -j ACCEPT

