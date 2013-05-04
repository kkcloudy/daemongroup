#!/bin/bash

source cp_start.sh
#先恢复所有的cpgrp中的用户，读取的是$CP_USER_INFO文件
CP_SH='/bin/bash'
CP_GRP='cpgrp'
#while read userinfo
#do
#	CP_USRNAME=$(echo $userinfo | awk -v FS=" " '{ printf $1 }')
#	CP_PWD=$(echo $userinfo | awk -v FS=" " '{ printf $2 }')
#	useradd -m -p $CP_PWD -g $CP_GRP -G $CP_GRP -s $CP_SH $CP_USRNAME
#done < $CP_USER_INFO


#读取每一行，都进行处理
[ -e $CP_DB_FILE ] || exit 0
cp $CP_DB_FILE ${CP_DB_FILE}_TEMP
while read line 
do
	CP_ID=$(echo $line | awk -v FS=" " '{ print $1 }')
	CP_IP=$(echo $line | awk -v FS=" " '{ print $2 }')
	CP_PORT=$(echo $line | awk -v FS=" " '{ print $3 }')
	CP_USRNAME=$(echo $line | awk -v FS=" " '{ print $4 }')
	CP_PWD=$(echo $line | awk -v FS=" " '{ print $5 }')
	CP_IF=$(echo $line | awk -v FS=" " '{ print $6 }')
	
	CP_PWD_ENCRYPT=$(openssl passwd -1 $CP_PWD)
	useradd -m -p $CP_PWD_ENCRYPT -g $CP_GRP -G $CP_GRP -s $CP_SH $CP_USRNAME
	
	#echo "CP_ID="$CP_ID
	#echo "CP_IP="$CP_IP
	#echo "CP_USRNAME="$CP_USRNAME
	#echo "CP_IF="$CP_IF
	
	if [ -n "$CP_IF" ];then
		sed "s/^.*${line}.*$/${CP_ID}	${CP_IP}	${CP_PORT}	${CP_USRNAME}	${CP_PWD}/g" $CP_DB_FILE > ${CP_DB_FILE}_TT
		mv ${CP_DB_FILE}_TT ${CP_DB_FILE}
	#	cat ${CP_DB_FILE}
		cp_apply_if.sh $CP_ID $CP_IF
	fi
	
	#echo
done < ${CP_DB_FILE}_TEMP

rm -f ${CP_DB_FILE}_TEMP
chmod 666 ${CP_DB_FILE} 
#mv ${CP_DB_FILE}_BACK ${CP_DB_FILE}

# Should not use exit, which will cause calling shell exit too.
#exit 0;
#装载白/黑名单

#重复载入了
#while read line
#do
#	CP_FLAG=$(echo $line | awk -v FS=" " '{ print $1 }')
#	CP_ID=$(echo $line | awk -v FS=" " '{ print $2 }')

#	CP_FILTER_DEFAULT=CP_${CP_ID}_F_DEFAULT
#	CP_NAT_DEFAULT=CP_${CP_ID}_N_DEFAULT
#	CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
#	CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT
#	CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTH
	
#	if [ $CP_FLAG -eq $CP_WHITE_LIST_FLAG ];then
#		PORT=$(echo $line | awk -v FS=" " '{ print $4 }')
			
		
#		IPADDR=$(echo $line | awk -v FS=" " '{ print $3 }')
#		if [ "x${PORT}" == "xall" ];then
#			iptables -t nat -I ${CP_NAT_DEFAULT} -m iprange --dst-range $IPADDR -j FW_DNAT
#			iptables -I ${CP_FILTER_DEFAULT} -m iprange --dst-range $IPADDR -j FW_FILTER
#		else
#			iptables -t nat -I ${CP_NAT_DEFAULT} -p tcp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j FW_DNAT
#			iptables -t nat -I ${CP_NAT_DEFAULT} -p udp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j FW_DNAT
			
#			iptables -I ${CP_FILTER_DEFAULT} -p tcp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j FW_FILTER
#			iptables -I ${CP_FILTER_DEFAULT} -p udp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j FW_FILTER
#		fi
	
	
#		if [ $? -eq 0 ];then
#			echo "add $IPADDR to ${CP_SET_AUTHORIZED}_${CP_CUR_IF}"
#			break;
#		fi

#	elif [ $CP_FLAG -eq $CP_WHITE_LIST_FLAG_DOMAIN ];then
#		CP_WHITLE_LIST_DOMAIN=$(echo ${line} | awk -v FS=" " '{print $3}')

#		#add to iptables
#		iptables -I ${CP_FILTER_DEFAULT} -m string --string ${CP_WHITLE_LIST_DOMAIN} --algo bm --to 65535 -j FW_FILTER
#		iptables -t nat -I ${CP_NAT_DEFAULT} -m string --string ${CP_WHITLE_LIST_DOMAIN} --algo bm --to 65535 -j FW_DNAT
		

#	elif [ $CP_FLAG -eq $CP_BLACK_LIST_FLAG ];then
#		if [ "x${PORT}" == "xall" ];then
#			iptables -t nat -I ${CP_NAT_DEFAULT} -m iprange --dst-range $IPADDR -j DROP
#			iptables -I ${CP_FILTER_DEFAULT} -m iprange --dst-range $IPADDR -j DROP
#		else
#			iptables -t nat -I ${CP_NAT_DEFAULT} -p tcp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j DROP
#			iptables -t nat -I ${CP_NAT_DEFAULT} -p udp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j DROP
			
#			iptables -I ${CP_FILTER_DEFAULT} -p tcp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j DROP
#			iptables -I ${CP_FILTER_DEFAULT} -p udp -m iprange --dst-range $IPADDR -m multiport --dports ${PORT} -j DROP
#		fi
#	elif [ $CP_FLAG -eq $CP_BLACK_LIST_FLAG_DOMAIN ];then
#		CP_BLACK_LIST_DOMAIN=$(echo ${line} | awk -v FS=" " '{print $3}')

#		#add to iptables
#		iptables -I ${CP_FILTER_DEFAULT} -m string --string ${CP_BLACK_LIST_DOMAIN} --algo bm --to 65535 -j DROP
#		iptables -t nat -I ${CP_NAT_DEFAULT} -m string --string ${CP_BLACK_LIST_DOMAIN} --algo bm --to 65535 -j DROP
		

#	fi
#done < ${CP_WHITE_BLACK_LIST}

