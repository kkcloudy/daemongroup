#!/bin/bash

source cp_start.sh
#delete the username or password
if [ ! $# -eq 5 ] ; then
	echo "Usage: cp_create_profile.sh ID ID_TYPE PORTALIP PORTALPORT FAMILY "
	exit 1;
fi

#prepare params
CP_ID=$1
CP_ID_TYPE=$2
CP_IP=$3
CP_PORT=$4
CP_FA=$5

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
     exit 5;
fi

FW_FILTER="FW_FILTER"
FW_DNAT="FW_DNAT"
CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
MAC_PRE_DNAT="MAC_PRE_DNAT"
MAC_PRE_FILTER="MAC_PRE_FILTER"
EAP_DNAT="EAP_DNAT"
EAP_FILTER="EAP_FILTER"
MAC_PRE_AUTH_N="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_N"
MAC_PRE_AUTH_F="MAC_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_F"
#add for eap authorize or none-authenticate 
EAP_PRE_AUTH_N="EAP_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_N"
EAP_PRE_AUTH_F="EAP_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_F"
#end
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"

CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}"_IPV"${CP_FA}

[ -d /var/run/cpp ] || mkdir /var/run/cpp

#CP_ID+CP_ID_TYPEΨһ
if [ -e $CP_ID_FILE ] ; then 
    ip=$(cat $CP_ID_FILE)
    echo "Captive Portal Profile ${CP_ID_TYPE}${CP_ID} already exist with IP ${ip}"
    exit 4;
fi
printf "${CP_IP} ${CP_PORT}" > $CP_ID_FILE


${IPXTABLES} -nL $CP_FILTER > /dev/null 2>&1
if [ ! $? -eq 0 ];then                        
    ${IPXTABLES} -N $CP_FILTER
    ${IPXTABLES} -I FORWARD -j $CP_FILTER
    ${IPXTABLES} -A $CP_FILTER -j RETURN
fi

${IPXTABLES} -nL $CP_DNAT -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then                        
    ${IPXTABLES} -t nat -N $CP_DNAT
    ${IPXTABLES} -t nat -I PREROUTING -j $CP_DNAT
    ${IPXTABLES} -t nat -A $CP_DNAT -j RETURN
fi

ipset list $MAC_PRE_IPHASH_SET > /dev/null 2>&1
if [ ! $? -eq 0 ] && [ $CP_FA -eq 4 ] ; then
	ipset create $MAC_PRE_IPHASH_SET hash:ip
elif [ ! $? -eq 0 ] && [ $CP_FA -eq 6 ] ; then
	ipset create $MAC_PRE_IPHASH_SET hash:ip family inet6
fi

${IPXTABLES} -nL $MAC_PRE_AUTH_F  > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	${IPXTABLES} -N $MAC_PRE_AUTH_F
	${IPXTABLES} -I $MAC_PRE_FILTER -j $MAC_PRE_AUTH_F
	${IPXTABLES} -I $MAC_PRE_AUTH_F -m set --match-set ${MAC_PRE_IPHASH_SET} src -j ${FW_FILTER}
	${IPXTABLES} -I $MAC_PRE_AUTH_F -m set --match-set ${MAC_PRE_IPHASH_SET} dst -j ${FW_FILTER}
	${IPXTABLES} -A $MAC_PRE_AUTH_F -j RETURN
fi

${IPXTABLES} -nL $MAC_PRE_AUTH_N -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	${IPXTABLES} -t nat -N $MAC_PRE_AUTH_N
	${IPXTABLES} -t nat -I $MAC_PRE_DNAT -j $MAC_PRE_AUTH_N
	${IPXTABLES} -t nat -I $MAC_PRE_AUTH_N -m set --match-set ${MAC_PRE_IPHASH_SET} src -j ${FW_DNAT}
	${IPXTABLES} -t nat -A $MAC_PRE_AUTH_N -j RETURN
fi

#eap authorize or none-authenticate

${IPXTABLES} -nL $EAP_PRE_AUTH_F  > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	${IPXTABLES} -N $EAP_PRE_AUTH_F
	${IPXTABLES} -I $EAP_FILTER -j   $EAP_PRE_AUTH_F
	${IPXTABLES} -A $EAP_PRE_AUTH_F -j RETURN
fi

${IPXTABLES} -nL $EAP_PRE_AUTH_N -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	${IPXTABLES} -N $EAP_PRE_AUTH_N -t nat
	${IPXTABLES} -t nat -I $EAP_DNAT -j $EAP_PRE_AUTH_N
	${IPXTABLES} -t nat -A $EAP_PRE_AUTH_N -j RETURN
fi

#end eap authorize or none-authenticate

${IPXTABLES} -N $CP_FILTER_DEFAULT
${IPXTABLES} -I $CP_FILTER_DEFAULT -j DROP
${IPXTABLES} -I $CP_FILTER_DEFAULT -p udp --sport 68 --dport 67 -j ACCEPT
${IPXTABLES} -I $CP_FILTER_DEFAULT -p udp --sport 67 --dport 68 -j ACCEPT
${IPXTABLES} -I $CP_FILTER_DEFAULT -d $CP_IP -j ACCEPT

${IPXTABLES} -t nat -N $CP_NAT_DEFAULT
${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -j RETURN
if [ ! $? -eq 0 ] && [ $CP_FA -eq 4 ] ; then
	${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 80 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
	${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 8080 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
	#${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 443 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
elif [ ! $? -eq 0 ] && [ $CP_FA -eq 6 ] ; then
	${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 80 -j DNAT --to-destination [${CP_IP}]:${CP_PORT}
	${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 8080 -j DNAT --to-destination [${CP_IP}]:${CP_PORT}
	#${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 443 -j DNAT --to-destination [${CP_IP}]:${CP_PORT}
fi
${IPXTABLES} -t nat -I $CP_NAT_DEFAULT -d $CP_IP -j ACCEPT

#
${IPXTABLES} -N $CP_FILTER_AUTHORIZED_DEFAULT
${IPXTABLES} -I $CP_FILTER_AUTHORIZED_DEFAULT -j FW_FILTER

${IPXTABLES} -t nat -N $CP_NAT_AUTHORIZED_DEFAULT
${IPXTABLES} -t nat -I $CP_NAT_AUTHORIZED_DEFAULT -j FW_DNAT

if [ $CP_FA -eq 4 ] ; then
	ipset create $CP_IPHASH_SET hash:ip
elif [ $CP_FA -eq 6 ] ; then
	ipset create $CP_IPHASH_SET hash:ip family inet6
fi

exit 0;
