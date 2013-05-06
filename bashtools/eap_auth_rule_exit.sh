#!/bin/bash

if [ ! $# -eq 2 ] ; then
        echo "Usage: eap_auth_rule_exit.sh HANSITYPE HANSIID"
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

iptables -D $EAP_FILTER -j $EAP_PRE_AUTH_F
iptables -F $EAP_PRE_AUTH_F
iptables -X $EAP_PRE_AUTH_F

iptables -t nat -D $EAP_DNAT -j $EAP_PRE_AUTH_N
iptables -t nat -F $EAP_PRE_AUTH_N
iptables -t nat -X $EAP_PRE_AUTH_N

ipset -X $EAP_IPHASH_SET

