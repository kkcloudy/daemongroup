#!/bin/bash

source cp_start.sh
#delete the username or password
if [ ! $# -eq 4 ] ; then
	echo "Usage: cp_create_profile.sh ID ID_TYPE PORTALIP PORTALPORT  "
	exit 1;
fi

#prepare params
CP_ID=$1
CP_ID_TYPE=$2
CP_IP=$3
CP_PORT=$4

if [ $CP_ID_TYPE != "R" ] && [ $CP_ID_TYPE != "L" ] ; then
     echo "ID_TYPE should be R or L"
     exit 2;
fi

if [ $CP_ID -gt 16 ] || [ $CP_ID -lt 0 ] ; then
    echo "ID should be 0~16"
    exit 3;
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
MAC_PRE_IPHASH_SET="MAC_PRE_"${CP_ID_TYPE}${CP_ID}"_AUTH_SET"
#add for eap authorize or none-authenticate 
EAP_PRE_AUTH_N="EAP_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_N"
EAP_PRE_AUTH_F="EAP_PRE_AUTH_"${CP_ID_TYPE}${CP_ID}"_F"
#end
CP_FILTER_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${CP_ID_TYPE}${CP_ID}"_N_AUTH_DEFAULT"
CP_IPHASH_SET="CP_"${CP_ID_TYPE}${CP_ID}"_AUTHORIZED_SET"

CP_ID_FILE="/var/run/cpp/CP_"${CP_ID_TYPE}${CP_ID}

[ -d /var/run/cpp ] || mkdir /var/run/cpp

#CP_ID+CP_ID_TYPEΨһ
if [ -e $CP_ID_FILE ] ; then 
    ip=$(cat $CP_ID_FILE)
    echo "Captive Portal Profile ${CP_ID_TYPE}${CP_ID} already exist with IP ${ip}"
    exit 4;
fi
printf "${CP_IP} ${CP_PORT}" > $CP_ID_FILE


iptables -nL $CP_FILTER > /dev/null 2>&1
if [ ! $? -eq 0 ];then                        
    iptables -N $CP_FILTER
    iptables -I FORWARD -j $CP_FILTER
    iptables -A $CP_FILTER -j RETURN
fi

iptables -nL $CP_DNAT -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then                        
    iptables -t nat -N $CP_DNAT
    iptables -t nat -I PREROUTING -j $CP_DNAT
    iptables -t nat -A $CP_DNAT -j RETURN
fi

ipset -L $MAC_PRE_IPHASH_SET > /dev/null 2>&1
if [ ! $? -eq 0 ]; then
	ipset -N $MAC_PRE_IPHASH_SET iphash
fi

iptables -nL $MAC_PRE_AUTH_F  > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	iptables -N $MAC_PRE_AUTH_F
	iptables -I $MAC_PRE_FILTER -j $MAC_PRE_AUTH_F
	iptables -I $MAC_PRE_AUTH_F -m set --set ${MAC_PRE_IPHASH_SET} src -j ${FW_FILTER}
	iptables -I $MAC_PRE_AUTH_F -m set --set ${MAC_PRE_IPHASH_SET} dst -j ${FW_FILTER}
	iptables -A $MAC_PRE_AUTH_F -j RETURN
fi

iptables -nL $MAC_PRE_AUTH_N -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	iptables -t nat -N $MAC_PRE_AUTH_N
	iptables -t nat -I $MAC_PRE_DNAT -j $MAC_PRE_AUTH_N
	iptables -t nat -I $MAC_PRE_AUTH_N -m set --set ${MAC_PRE_IPHASH_SET} src -j ${FW_DNAT}
	iptables -t nat -A $MAC_PRE_AUTH_N -j RETURN
fi

#eap authorize or none-authenticate

iptables -nL $EAP_PRE_AUTH_F  > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	iptables -N $EAP_PRE_AUTH_F
	iptables -I $EAP_FILTER -j   $EAP_PRE_AUTH_F
	iptables -A $EAP_PRE_AUTH_F -j RETURN
fi

iptables -nL $EAP_PRE_AUTH_N -t nat > /dev/null 2>&1
if [ ! $? -eq 0 ];then   
	iptables -N $EAP_PRE_AUTH_N -t nat
	iptables -t nat -I $EAP_DNAT -j $EAP_PRE_AUTH_N
	iptables -t nat -A $EAP_PRE_AUTH_N -j RETURN
fi

#end eap authorize or none-authenticate

iptables -N $CP_FILTER_DEFAULT
iptables -I $CP_FILTER_DEFAULT -j DROP
iptables -I $CP_FILTER_DEFAULT -p udp --sport 68 --dport 67 -j ACCEPT
iptables -I $CP_FILTER_DEFAULT -p udp --sport 67 --dport 68 -j ACCEPT
iptables -I $CP_FILTER_DEFAULT -d $CP_IP -j ACCEPT

iptables -t nat -N $CP_NAT_DEFAULT
iptables -t nat -I $CP_NAT_DEFAULT -j RETURN
iptables -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 80 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
iptables -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 8080 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
#iptables -t nat -I $CP_NAT_DEFAULT -p tcp -m tcp --dport 443 -j DNAT --to-destination ${CP_IP}:${CP_PORT}
iptables -t nat -I $CP_NAT_DEFAULT -d $CP_IP -j ACCEPT

#
iptables -N $CP_FILTER_AUTHORIZED_DEFAULT
iptables -I $CP_FILTER_AUTHORIZED_DEFAULT -j FW_FILTER

iptables -t nat -N $CP_NAT_AUTHORIZED_DEFAULT
iptables -t nat -I $CP_NAT_AUTHORIZED_DEFAULT -j FW_DNAT

ipset -N $CP_IPHASH_SET iphash

