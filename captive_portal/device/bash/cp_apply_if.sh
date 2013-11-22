#!/bin/bash
#add the captive-portal ID type,(Local hansi or Remote hansi) in EAG2.0 version
source cp_start.sh

if [ ! $# -eq 4 ] ; then
	echo "Usage: cp_apply_if.sh ID ID_TYPE INTERFACE FAMILY "
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

CP_FILTER_AUTH_IF=CP_${CP_ID_TYPE}${CP_ID}_F_${CP_IF}
CP_FILTER_AUTH_IF_IN=CP_${CP_ID_TYPE}${CP_ID}_F_${CP_IF}_IN
CP_NAT_AUTH_IF=CP_${CP_ID_TYPE}${CP_ID}_N_${CP_IF}

CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}"_IPV"${CP_FA}
#确保id，已经创建
if [ ! -e $CP_ID_FILE ] ; then 
    echo "Captive Portal Profile ${CP_ID_TYPE}${CP_ID} not exist!"
    exit 4;
fi

#确保接口未被使用过
#exist_cp_if=$(awk -v FS=" " -v exist_if="${CP_IF}${CP_ID}" '$2==exist_if { print $1 }' ${CP_IF_DB_FILE}) 
if [ -e $CP_IF_DB_FILE ] ; then 
    id=$(cat $CP_IF_DB_FILE)
    echo "Captive Portal Profile $CP_IF used by $id"
    exit 5;
fi


${IPXTABLES} -N $CP_FILTER_AUTH_IF
${IPXTABLES} -I $CP_FILTER_AUTH_IF -j RETURN
${IPXTABLES} -I $CP_FILTER_AUTH_IF -m set --match-set ${CP_IPHASH_SET} src -j ${CP_FILTER_AUTHORIZED_DEFAULT}

${IPXTABLES} -N $CP_FILTER_AUTH_IF_IN
${IPXTABLES} -I $CP_FILTER_AUTH_IF_IN -j RETURN
${IPXTABLES} -I $CP_FILTER_AUTH_IF_IN -m set --match-set ${CP_IPHASH_SET} dst -j ${CP_FILTER_AUTHORIZED_DEFAULT}

${IPXTABLES} -t nat -N $CP_NAT_AUTH_IF
${IPXTABLES} -t nat -I $CP_NAT_AUTH_IF -j RETURN
${IPXTABLES} -t nat -I $CP_NAT_AUTH_IF -m set --match-set ${CP_IPHASH_SET} src -j ${CP_NAT_AUTHORIZED_DEFAULT}

${IPXTABLES} -I $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_DEFAULT}
${IPXTABLES} -I $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_AUTH_IF}
${IPXTABLES} -I $CP_FILTER -o ${CP_IF} -j ${CP_FILTER_AUTH_IF_IN}


${IPXTABLES} -t nat -I CP_DNAT -i ${CP_IF} -j $CP_NAT_DEFAULT
${IPXTABLES} -t nat -I CP_DNAT -i ${CP_IF} -j $CP_NAT_AUTH_IF

printf "${CP_ID_TYPE}${CP_ID}" > $CP_IF_DB_FILE

exit 0;