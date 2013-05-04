#!/bin/bash

source cp_start.sh

if [  $# -lt 3 ] ; then
	echo "Usage: cp_create_profile.sh INSTANCE_ID INSTANCE_TYPE INTERFACE "
	exit
fi

CP_ID=$1  
CP_ID_TYPE=$2  
CP_IF=$3                 
CP_IF_DB_FILE="/var/run/cpp/CP_IF_INFO"_${CP_IF}

if [ $CP_ID_TYPE != "R" ] && [ $CP_ID_TYPE != "L" ] ; then
     echo "ID_TYPE should be R or L"
     exit 2;
fi

#captive-portal ID是这个脚本的第一个参数
if [ $CP_ID -gt 16 ] || [ $CP_ID -lt 0 ] ; then
	echo "ID should be 0~16"
	exit 1
fi


CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"
CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_SET"

CP_FILTER_AUTH_IF=CP_${CP_ID_TYPE}${CP_ID}_F_${CP_IF}
CP_FILTER_AUTH_IF_IN=CP_${CP_ID_TYPE}${CP_ID}_F_${CP_IF}_IN
CP_NAT_AUTH_IF=CP_${CP_ID_TYPE}${CP_ID}_N_${CP_IF}

if [ ! -e $CP_IF_DB_FILE ] ; then 
    echo "Captive Portal Profile $CP_IF not be used !"
    exit 5;
fi

id=$(cat $CP_IF_DB_FILE)
if [ "x${id}" != "x${CP_ID_TYPE}${CP_ID}" ] ; then
    echo "Captive Portal Profile $CP_IF not be used by ${CP_ID_TYPE}${CP_ID} but by $id!"
    exit 6;
fi



iptables -D $CP_FILTER -o ${CP_IF} -j ${CP_FILTER_AUTH_IF_IN}
iptables -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_AUTH_IF}
iptables -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_DEFAULT}
iptables -t nat -D CP_DNAT -i ${CP_IF} -j ${CP_NAT_AUTH_IF}
iptables -t nat -D CP_DNAT -i ${CP_IF} -j $CP_NAT_DEFAULT

iptables -F ${CP_FILTER_AUTH_IF}
iptables -X ${CP_FILTER_AUTH_IF}

iptables -F ${CP_FILTER_AUTH_IF_IN}
iptables -X ${CP_FILTER_AUTH_IF_IN}

iptables -t nat -F ${CP_NAT_AUTH_IF}
iptables -t nat -X ${CP_NAT_AUTH_IF}

rm -rf ${CP_IF_DB_FILE}
