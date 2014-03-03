#!/bin/bash

if [ ! $# -eq 1 ] ; then
        echo "Usage: acl_shell_del_policy.sh FACL_TAG"
        exit 1;
fi

FACL_TAG=$1

FW_FILTER="FW_FILTER"
FW_DNAT="FW_DNAT"
FW_SNAT="FW_SNAT"

FACL_DNAT="FACL_DNAT"
FACL_FILTER="FACL_FILTER"

if [ $FACL_TAG -gt 4096 ] || [ $FACL_TAG -lt 1 ] ; then
    echo "FACL_TAG should be 1~4096"
    exit 2;
fi


FW_DNAT="FW_DNAT"
FW_FILTER="FW_FILTER"
FACL_DNAT="FACL_DNAT"
FACL_FILTER="FACL_FILTER"

FACL_POLICY_N="FACL_POLICY_"${FACL_TAG}"_N"
FACL_POLICY_F="FACL_POLICY_"${FACL_TAG}"_F"

/opt/bin/iptables -D $FACL_FILTER -m mark --mark $FACL_TAG -j $FACL_POLICY_F
/opt/bin/iptables -F $FACL_POLICY_F
/opt/bin/iptables -X $FACL_POLICY_F

/opt/bin/iptables -t nat -D $FACL_DNAT -m mark --mark $FACL_TAG -j $FACL_POLICY_N
/opt/bin/iptables -t nat -F $FACL_POLICY_N
/opt/bin/iptables -t nat -X $FACL_POLICY_N

exit 0;
