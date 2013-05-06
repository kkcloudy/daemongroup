#!/bin/bash

if [ ! $# -eq 2 ] ; then
        echo "Usage: eap_auth_clean_all.sh HANSITYPE HANSIID"
        exit 1;
fi

#prepare params
HANSI_TYPE=$1
HANSI_ID=$2

if [ $HANSI_TYPE != "R" ] && [ $HANSI_TYPE != "L" ] ; then
     echo "HANSITYPE should be R or L"
     exit 2;
fi

if [ $HANSI_ID -gt 16 ] || [ $HANSI_ID -lt 0 ] ; then
    echo "HANSI_ID should be 0~16"
    exit 3;
fi

FW_DNAT="FW_DNAT"
FW_FILTER="FW_FILTER"
EAP_DNAT="EAP_DNAT"
EAP_FILTER="EAP_FILTER"
EAP_PRE_AUTH_N="EAP_PRE_AUTH_"${HANSI_TYPE}${HANSI_ID}"_N"
EAP_PRE_AUTH_F="EAP_PRE_AUTH_"${HANSI_TYPE}${HANSI_ID}"_F"

EAP_IPHASH_SET="EAP_"${HANSI_TYPE}${HANSI_ID}"_AUTH_SET"

ipset -F $EAP_IPHASH_SET
iptables -F $EAP_PRE_AUTH_F
iptables -t nat -F $EAP_PRE_AUTH_N

iptables -I $EAP_PRE_AUTH_F -j RETURN
iptables -I $EAP_PRE_AUTH_F -m set --set ${EAP_IPHASH_SET} src -j ${FW_FILTER}
iptables -I $EAP_PRE_AUTH_F -m set --set ${EAP_IPHASH_SET} dst -j ${FW_FILTER}
iptables -t nat -I $EAP_PRE_AUTH_N -j RETURN
iptables -t nat -I $EAP_PRE_AUTH_N -m set --set ${EAP_IPHASH_SET} src -j ${FW_DNAT}

