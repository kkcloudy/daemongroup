#!/bin/bash

source cp_start.sh

if [ $# -ne 3 ] ; then
	echo "Usage: cp_create_profile.sh INSTANCE_ID  INSTANCE_TYPE FAMILY"
	exit 1;
fi

CP_ID=$1  
CP_ID_TYPE=$2  
CP_FA=$3

if [ $CP_ID_TYPE != "R" ] && [ $CP_ID_TYPE != "L" ] ; then
     echo "ID_TYPE should be R or L"
     exit 2;
fi

if [ $CP_ID -gt 16 ] || [ $CP_ID -lt 0 ] ; then
    echo "ID should be 0~16"
    exit 3;
fi

if [ $CP_FA -eq 4 ] ; then
     IPXTABLES="iptables"
     CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_IPV4_SET"
     MAC_PRE_IPHASH_SET="MAC_PRE_"${CP_ID_TYPE}${CP_ID}"_AUTH_IPV4_SET"
elif [ $CP_FA -eq 6 ] ; then
     IPXTABLES="ip6tables"
     CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_IPV6_SET"
     MAC_PRE_IPHASH_SET="MAC_PRE_"${CP_ID_TYPE}${CP_ID}"_AUTH_IPV6_SET"
else
     echo "FAMILY should be 4 or 6"
     exit 6;
fi

CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
MAC_PRE_DNAT="MAC_PRE_DNAT"
MAC_PRE_FILTER="MAC_PRE_FILTER"
MAC_PRE_AUTH_N="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_N"
MAC_PRE_AUTH_F="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_F"
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"
CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}"_IPV"${CP_FA}

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
            exit 5;
        fi
    done
fi


${IPXTABLES} -D $MAC_PRE_FILTER -j $MAC_PRE_AUTH_F
${IPXTABLES} -F $MAC_PRE_AUTH_F
${IPXTABLES} -X $MAC_PRE_AUTH_F
echo 111
${IPXTABLES} -t nat -D $MAC_PRE_DNAT -j $MAC_PRE_AUTH_N
${IPXTABLES} -F $MAC_PRE_AUTH_N -t nat
${IPXTABLES} -X $MAC_PRE_AUTH_N -t nat

ipset flush $MAC_PRE_IPHASH_SET
ipset destroy $MAC_PRE_IPHASH_SET

echo 222
${IPXTABLES} -F $CP_FILTER_DEFAULT
${IPXTABLES} -X $CP_FILTER_DEFAULT
echo 333
${IPXTABLES} -t nat -F $CP_NAT_DEFAULT
${IPXTABLES} -t nat -X $CP_NAT_DEFAULT
echo 444

${IPXTABLES} -D $CP_FILTER_AUTHORIZED_DEFAULT -j FW_FILTER
${IPXTABLES} -F $CP_FILTER_AUTHORIZED_DEFAULT
${IPXTABLES} -X $CP_FILTER_AUTHORIZED_DEFAULT
echo 555
${IPXTABLES} -t nat -D $CP_NAT_AUTHORIZED_DEFAULT -j FW_DNAT
${IPXTABLES} -t nat -F $CP_NAT_AUTHORIZED_DEFAULT
${IPXTABLES} -t nat -X $CP_NAT_AUTHORIZED_DEFAULT
echo 666

ipset flush $CP_IPHASH_SET
ipset destroy $CP_IPHASH_SET

rm -rf ${CP_ID_FILE}

exit 0;