#!/bin/bash

source cp_start.sh

if [ ! $# -eq 2 ] ; then
	echo "Usage: cp_set_legal_ip_group.sh PROFILEID INTERFACE_SET"
	exit
fi

CP_ID=$1
CP_INTERFACES=$2
#CP_NETWORK=$2

CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT
#CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTHORIZED_DEFAULT
#CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTHORIZED
CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTH
CP_FILTER_GROUP_AUTHORIZED=CP_${CP_ID}_GP_F_AUTH
CP_NAT_GROUP_AUTHORIZED=CP_${CP_ID}_GP_N_AUTH

iptables -N ${CP_FILTER_AUTHORIZED}
iptables -t nat -N ${CP_NAT_AUTHORIZED}

#将其中的","换成空格，因为后面的for循环识别空格为分割符
CP_IF_TEMP=$(echo "$CP_INTERFACES" | sed "s/,/ /g")

for CP_CUR_IF in $CP_IF_TEMP
do
	# Only retrieve the first ip address of this interface
	CP_CUR_IF_NETWORKS=$(ip addr show dev ${CP_CUR_IF} | grep inet | awk '$1=="inet" { print $2"\n" }' )
	IPCOUNT=$(echo ${CP_CUR_IF_NETWORKS} | wc -w )
	
	if [ ! $IPCOUNT -eq 0 ] ; then
		# 只使用一个ip
		if [ $IPCOUNT -gt 1 ] ; then
#			CP_CUR_IF_NETWORK=$(echo ${CP_CUR_IF_NETWORKS} | sed -n '1,1p' )
#CP_CUR_IF_NETWORKS 中没有换行符号，导致 sed不能提取出第一个network   直接用命令却有换行符号
			CP_CUR_IF_NETWORK=$( ip addr show dev ${CP_CUR_IF} | grep inet | awk '$1=="inet" { print $2 }' | sed -n '1,1p' )
		else
			CP_CUR_IF_NETWORK=$CP_CUR_IF_NETWORKS
		fi
		
		if [ ${CP_CUR_MODE} -eq ${CP_MODE_IPSET} ];then
			ipset -F ${CP_SET_AUTHORIZED}_${CP_CUR_IF}
			ipset -X ${CP_SET_AUTHORIZED}_${CP_CUR_IF}
			ipset -N ${CP_SET_AUTHORIZED}_${CP_CUR_IF} ipmap --network $CP_CUR_IF_NETWORK
			if [ $? -eq 0 ]; then
				iptables -I FORWARD -m set --match-set ${CP_SET_AUTHORIZED}_${CP_CUR_IF} src,dst -j ${CP_FILTER_AUTHORIZED}
				iptables -t nat -I PREROUTING -m set --match-set ${CP_SET_AUTHORIZED}_${CP_CUR_IF} src,dst  -j ${CP_NAT_AUTHORIZED}
			fi
		else
			iptables -X ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}
			iptables -X ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN
			iptables -t nat -X ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF}

			iptables -N ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}
			iptables -N ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN
			iptables -t nat -N ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF}
			
			iptables -F ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}
			iptables -F ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN
			iptables -t nat -F ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF}
			
			iptables -I ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF} -j RETURN
			iptables -I ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN -j RETURN
			iptables -t nat -I ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF} -j RETURN
						
			
			iptables -I FORWARD -i ${CP_CUR_IF} -j ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}
			iptables -I FORWARD -o ${CP_CUR_IF} -j ${CP_FILTER_GROUP_AUTHORIZED}_${CP_CUR_IF}_IN
			iptables -t nat -I PREROUTING -i ${CP_CUR_IF} -j ${CP_NAT_GROUP_AUTHORIZED}_${CP_CUR_IF}
		fi
	fi
done


iptables -nL FW_FILTER > /dev/null 2>&1
if [ ! $? -eq 0 ];then
	iptables -N FW_FILTER
	iptables -A FW_FILTER -j ACCEPT
fi

iptables -t nat -L FW_DNAT > /dev/null 2>&1
if [ ! $? -eq 0 ];then
	iptables -t nat -N FW_DNAT
	iptables -t nat -A FW_DNAT -j ACCEPT
fi
	
iptables -t nat -L FW_SNAT > /dev/null 2>&1
if [ ! $? -eq 0 ];then
	iptables -t nat -N FW_SNAT
	iptables -t nat -A FW_SNAT -j ACCEPT
fi


iptables -A ${CP_FILTER_AUTHORIZED} -j FW_FILTER
iptables -t nat -A ${CP_NAT_AUTHORIZED} -j FW_DNAT



#reload whitelist
#description of the first flag.
#0  white list with IP
#1  black list with IP
#3  white list with domain
#4  black list with domain
#--add by wk
#echo ${CP_WHITE_BLACK_LIST}
if [ ! -f $CP_WHITE_BLACK_LIST  ];then
	touch $CP_WHITE_BLACK_LIST
fi

while read line
do
	THIS_FLAG=$(echo ${line} | awk -v FS=" " '{print $1}')
	THIS_CP_ID=$(echo ${line} | awk -v FS=" " '{print $2}')
	if [ $THIS_CP_ID -eq $CP_ID ];then
		if [ ${THIS_FLAG} -eq ${CP_WHITE_LIST_FLAG} ];then
			IPS=$(echo ${line} | awk -v FS=" "  '{print $3}')
			PORT=$(echo ${line} | awk -v FS=" "  '{print $4}')	
			cp_add_white_list.sh ${THIS_CP_ID} ${IPS} ${PORT}
		elif [ ${THIS_FLAG} -eq ${CP_BLACK_LIST_FLAG} ];then
			IPS=$(echo ${line} | awk -v FS=" "  '{print $3}')
			PORT=$(echo ${line} | awk -v FS=" "  '{print $4}')		
			cp_add_black_list.sh ${THIS_CP_ID} ${IPS} ${PORT}
		elif [ ${THIS_FLAG} -eq ${CP_WHITE_LIST_FLAG_DOMAIN} ];then
			DOMAIN=$(echo ${line} | awk -v FS=" "  '{print $3}')
			cp_add_white_list_domain.sh ${THIS_CP_ID} ${DOMAIN}				
		elif [ $THIS_FLAG -eq $CP_BLACK_LIST_FLAG_DOMAIN ];then
			DOMAIN=$(echo ${line} | awk -v FS=" "  '{print $3}')
			cp_add_black_list_domain.sh ${THIS_CP_ID} ${DOMAIN}	
		fi
	fi
done < ${CP_WHITE_BLACK_LIST}



echo "start" > /opt/services/status/portal_status.status
echo "start" > /opt/services/status/iptables_status.status
