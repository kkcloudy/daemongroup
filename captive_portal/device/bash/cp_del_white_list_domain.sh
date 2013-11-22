#!/bin/bash

source cp_start.sh

if [ ! $# -eq 5 ] && [ ! $# -eq 6 ] ; then
	echo "Usage: cp_add_white_list.sh INSTANCE_ID  INSTANCE_TYPE IPADDRS PORT FAMILY"
	echo "eg.:cp_add_white_list.sh ID R/L 192.168.9.1 www.163.com 4 [userif]"
	exit 1;
fi

#. portal_get_cpid.sh
#source cp_start.sh
	

CP_ID=$1
CP_ID_TYPE=$2
CP_WHITELIST_IP=$3
#CP_WHITELIST_PORT=$4
CP_DOMAIN=$4
CP_FA=$5
CP_USER_IF=$6

#echo $cp_id

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
elif [ $CP_FA -eq 6 ] ; then
     IPXTABLES="ip6tables"
     CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_IPV6_SET"
else
     echo "FAMILY should be 4 or 6"
     exit 6;
fi

CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"

CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}"_IPV"${CP_FA}

if [ ! -e $CP_ID_FILE ] ; then 
    echo "Captive Portal Profile $CP_ID_FILE not exist!"
    exit 4;
fi

flag=0
if [ "x${CP_USER_IF}" == "x" ];then
    flag=1
fi

CPS_IF=$(ls /var/run/cpp/CP_IF_INFO* 2>/dev/null)
if [ $CPS_IF ];then
    for file in $CPS_IF
    do
        if [ "x/var/run/cpp/CP_IF_INFO_${CP_USER_IF}"_IPV"${CP_FA}" == "x${file}" ]; then
            flag=1;
            break;
        fi
    done
fi


if [ $flag -eq 0 ]; then
    echo "$CP_USER_IF not add to ${CP_ID_TYPE}${CP_ID}"
    exit 5;
fi


if [ "x${CP_USER_IF}" != "x" ];then
    ${IPXTABLES} -D ${CP_FILTER_DEFAULT} -i $CP_USER_IF -m iprange --dst-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_FILTER;
    ${IPXTABLES} -D ${CP_FILTER_DEFAULT} -i $CP_USER_IF -m iprange --src-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_FILTER;

		${IPXTABLES} -t nat -D ${CP_NAT_DEFAULT} -i $CP_USER_IF -m iprange --dst-range $CP_WHITELIST_IP -j FW_DNAT;
		${IPXTABLES} -t nat -D ${CP_NAT_DEFAULT} -i $CP_USER_IF -m iprange --src-range $CP_WHITELIST_IP -j FW_DNAT;    
else
    ${IPXTABLES} -D ${CP_FILTER_DEFAULT} -m iprange --dst-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_FILTER;
    ${IPXTABLES} -D ${CP_FILTER_DEFAULT} -m iprange --src-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_FILTER;

		${IPXTABLES} -t nat -D ${CP_NAT_DEFAULT} -m iprange --dst-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_DNAT;
		${IPXTABLES} -t nat -D ${CP_NAT_DEFAULT} -m iprange --src-range $CP_WHITELIST_IP -m comment --comment $CP_DOMAIN -j FW_DNAT;
fi

exit 0;