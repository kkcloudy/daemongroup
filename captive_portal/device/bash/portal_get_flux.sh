#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: portal_get_flux.sh IPADDR"
	exit 1
fi

. portal_get_cpid.sh



IPADDR=$1

CP_FILTER_GROUP_AUTHORIZED=CP_${CP_ID}_GP_F_AUTH
CP_NAT_GROUP_AUTHORIZED=CP_${CP_ID}_GP_N_AUTH
CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT



CP_CUR_IF=`get_user_interface ${IPADDR}`
if [ ${CP_CUR_IF} == "" ];then
#用户在线的时候，其有可能不存在于arp中，需要ping一下用户，使更新arp(还有其它跟新arp的方法吗？)
	ping.sh $IPADDR
	CP_CUR_IF=`get_user_interface ${IPADDR}`
	if [ ${CP_CUR_IF} == "" ];then
#找不到该用户
		exit 2
	fi
fi

#获得ｉｐ的流量，只能用于通过portal认证的用户
if [ ${CP_CUR_MODE} -eq ${CP_MODE_IPTABLES} ];then

	iptables -nvxL ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} | grep ${IPADDR}" " | sed "2,100d" | awk '{printf $2}'

else
	exit 3
fi
