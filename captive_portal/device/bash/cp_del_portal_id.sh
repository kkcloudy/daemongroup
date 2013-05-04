#!/bin/bash

source cp_start.sh

if [ $# -ne 2 ] ; then
	echo "Usage: cp_create_profile.sh INSTANCE_ID  INSTANCE_TYPE"
	exit
fi

CP_ID=$1  
CP_ID_TYPE=$2  


if [ $CP_ID_TYPE != "R" ] && [ $CP_ID_TYPE != "L" ] ; then
     echo "ID_TYPE should be R or L"
     exit 2;
fi

if [ $CP_ID -gt 16 ] || [ $CP_ID -lt 0 ] ; then
    echo "ID should be 0~16"
    exit 3;
fi


CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
MAC_PRE_DNAT="MAC_PRE_DNAT"
MAC_PRE_FILTER="MAC_PRE_FILTER"
MAC_PRE_AUTH_N="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_N"
MAC_PRE_AUTH_F="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_F"
MAC_PRE_IPHASH_SET="MAC_PRE_"${CP_ID_TYPE}${CP_ID}"_AUTH_SET"
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"
CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_SET"

CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}

if [ ! -e $CP_ID_FILE ] ; then 
    echo "Captive Portal Profile $CP_ID_FILE not exist!"
    exit 4;
fi

CPS_IF=$(ls /var/run/cpp/CP_IF_INFO* 2>/dev/null)
if [ $CPS_IF ];then
    for file in $CPS_IF
    do
        id=$(cat $file)
        if [ "x$id" == "x${CP_ID_TYPE}${CP_ID}" ]; then
            echo "${CP_ID_TYPE}${CP_ID} has $file not del! you should del it first!"
            exit 4
        fi
    done
fi


iptables -D $MAC_PRE_FILTER -j $MAC_PRE_AUTH_F
iptables -F $MAC_PRE_AUTH_F
iptables -X $MAC_PRE_AUTH_F
echo 111
iptables -t nat -D $MAC_PRE_DNAT -j $MAC_PRE_AUTH_N
iptables -F $MAC_PRE_AUTH_N -t nat
iptables -X $MAC_PRE_AUTH_N -t nat

ipset -F $MAC_PRE_IPHASH_SET
ipset -X $MAC_PRE_IPHASH_SET

echo 222
iptables -F $CP_FILTER_DEFAULT
iptables -X $CP_FILTER_DEFAULT
echo 333
iptables -t nat -F $CP_NAT_DEFAULT
iptables -t nat -X $CP_NAT_DEFAULT
echo 444

iptables -D $CP_FILTER_AUTHORIZED_DEFAULT -j FW_FILTER
iptables -F $CP_FILTER_AUTHORIZED_DEFAULT
iptables -X $CP_FILTER_AUTHORIZED_DEFAULT
echo 555
iptables -t nat -D $CP_NAT_AUTHORIZED_DEFAULT -j FW_DNAT
iptables -t nat -F $CP_NAT_AUTHORIZED_DEFAULT
iptables -t nat -X $CP_NAT_AUTHORIZED_DEFAULT
echo 666
ipset -X $CP_IPHASH_SET

rm -rf ${CP_ID_FILE}
