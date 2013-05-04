#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: portal_add_legal_ip.sh IPADDR"
	exit
fi

. portal_get_cpid.sh



IPADDR=$1
CP_IF_TEMP=$(echo $CP_IF | sed "s/,/ /g")
CP_CUR_IF=`get_user_interface ${IPADDR}`
if [ ${CP_CUR_IF} == "" ];then
	exit 1
fi

CP_FILTER_GROUP_AUTHORIZED=CP_${CP_ID}_GP_F_AUTH
CP_NAT_GROUP_AUTHORIZED=CP_${CP_ID}_GP_N_AUTH

CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT


if [ $? -eq 0 ];then
	if [ ${CP_CUR_MODE} -eq ${CP_MODE_IPSET} ];then
		ipset -A ${CP_SET_AUTHORIZED}_${CP_CUR_IF} $IPADDR
	else
		#先判断用户已经存在了
		iptables -nvxL ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} | grep ${IPADDR}" "
		if [ $? -eq 0 ];then
			exit 0;
		fi
		#用户不存在添加用户。
		iptables -I ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} -s ${IPADDR} -j ${CP_FILTER_AUTHORIZED}
		iptables -I ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN -d ${IPADDR} -j ${CP_FILTER_AUTHORIZED}
		iptables -t nat -I ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF} -s ${IPADDR} -j ${CP_NAT_AUTHORIZED}
	fi
fi


#for CP_CUR_IF in $CP_IF_TEMP
#do
#	if [ ${CP_CUR_MODE} -eq ${CP_MODE_IPSET} ];fi
#		ipset -A ${CP_SET_AUTHORIZED}_${CP_CUR_IF} $IPADDR
#		if [ $? -eq 0 ];then
#			echo "add $IPADDR to ${CP_SET_AUTHORIZED}_${CP_CUR_IF}"
#			break;
#		fi
#	else
#		iptables -I ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} -s $IPADDR
#	fi
#done

record_active_user.sh "add" ${CP_ID} ${IPADDR}

