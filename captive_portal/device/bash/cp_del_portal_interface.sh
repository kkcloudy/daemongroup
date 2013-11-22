#!/bin/bash

source cp_start.sh

if [  $# -lt 4 ] ; then
	echo "Usage: cp_create_profile.sh INSTANCE_ID INSTANCE_TYPE INTERFACE FAMILY "
	exit 1;
fi

CP_ID=$1  
CP_ID_TYPE=$2  
CP_IF=$3                 
CP_FA=$4           
CP_IF_DB_FILE="/var/run/cpp/CP_IF_INFO"_${CP_IF}"_IPV"${CP_FA}

if [ $CP_ID_TYPE != "R" ] && [ $CP_ID_TYPE != "L" ] ; then
     echo "ID_TYPE should be R or L"
     exit 2;
fi

#captive-portal ID是这个脚本的第一个参数
if [ $CP_ID -gt 16 ] || [ $CP_ID -lt 0 ] ; then
	echo "ID should be 0~16"
	exit 3;
fi

if [ $CP_FA -eq 4 ] ; then
     IPXTABLES="iptables"
     CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_IPV4_SET"
elif [ $CP_FA -eq 6 ] ; then
     IPXTABLES="ip6tables"
     CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_IPV6_SET"
else
     echo "FAMILY should be 4 or 6"
     exit 4;
fi

CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"

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



${IPXTABLES} -D $CP_FILTER -o ${CP_IF} -j ${CP_FILTER_AUTH_IF_IN}
${IPXTABLES} -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_AUTH_IF}
${IPXTABLES} -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_DEFAULT}
${IPXTABLES} -t nat -D CP_DNAT -i ${CP_IF} -j ${CP_NAT_AUTH_IF}
${IPXTABLES} -t nat -D CP_DNAT -i ${CP_IF} -j $CP_NAT_DEFAULT

${IPXTABLES} -F ${CP_FILTER_AUTH_IF}
${IPXTABLES} -X ${CP_FILTER_AUTH_IF}

${IPXTABLES} -F ${CP_FILTER_AUTH_IF_IN}
${IPXTABLES} -X ${CP_FILTER_AUTH_IF_IN}

${IPXTABLES} -t nat -F ${CP_NAT_AUTH_IF}
${IPXTABLES} -t nat -X ${CP_NAT_AUTH_IF}

rm -rf ${CP_IF_DB_FILE}

exit 0;