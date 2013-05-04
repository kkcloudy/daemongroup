#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: portal_del_legal_ip.sh IPADDR"
	exit
fi

. portal_get_cpid.sh

IPADDR=$1

CP_FILTER_GROUP_AUTHORIZED=CP_${CP_ID}_GP_F_AUTH
CP_NAT_GROUP_AUTHORIZED=CP_${CP_ID}_GP_N_AUTH
CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT

CP_CUR_IF=`get_user_interface ${IPADDR}`
if [ ${CP_CUR_IF} == "" ];then
	exit 1
fi

if [ $? -eq 0 ];then
	if [ ${CP_CUR_MODE} -eq ${CP_MODE_IPSET} ];then
		ipset -D ${CP_SET_AUTHORIZED}_${CP_CUR_IF} ${IPADDR}
	else
		iptables -D ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} -s ${IPADDR} -j ${CP_FILTER_AUTHORIZED}
		iptables -D ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN -d ${IPADDR} -j ${CP_FILTER_AUTHORIZED}
		iptables -t nat -D ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF} -s ${IPADDR} -j ${CP_NAT_AUTHORIZED}
	fi
	echo "del $IPADDR from ${CP_SET_AUTHORIZED}_${CP_CUR_IF}"
fi

#ipset -D $CP_SET_AUTHORIZED $IPADDR
#CP_IF_TEMP=$(echo $CP_IF | sed "s/,/ /g")

#for CP_CUR_IF in $CP_IF_TEMP
#do
#	ipset -D ${CP_SET_AUTHORIZED}_${CP_CUR_IF} $IPADDR
#	if [ $? -eq 0 ];then
#		echo "del $IPADDR from ${CP_SET_AUTHORIZED}_${CP_CUR_IF}"
#		break;
#	fi
#done

record_active_user.sh "del" ${CP_ID} ${IPADDR}
