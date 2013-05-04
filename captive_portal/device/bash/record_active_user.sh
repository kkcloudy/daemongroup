#!/bin/bash

source cp_start.sh

if [ ! $# -eq 3 ] ; then
	echo "Usage: record_active_user.sh add(del) id ip"
	exit
fi

#添加、删除已认证用户时将信息记录到一个文件


CP_RECORD_ACTION=$1
CP_RECORD_ID=$2
CP_RECORD_IP=$3

#文件第一行是通过的总认证数。
#文件没一行都是一个记录，每个记录开口记录了开通该ip的id。

#文件不存在，创建文件
[ -d /var/run/cpp ] || mkdir /var/run/cpp
[ -e $CP_RECORD_DB ] || touch $CP_RECORD_DB

#得到总数
CP_ACTIVE_USER_NUM=$(cat $CP_RECORD_DB | head -1)

if [ ! -n "${CP_ACTIVE_USER_NUM}" ] ; then
	CP_ACTIVE_USER_NUM=0
	echo ${CP_ACTIVE_USER_NUM} > ${CP_RECORD_DB}
fi


CP_CUR_ACTIVE_USER=${CP_RECORD_ID}"#"${CP_RECORD_IP};
#先查找该记录是否已经有了
RECORD_TEMP=`grep "^${CP_CUR_ACTIVE_USER}$" ${CP_RECORD_DB}`

#是添加一个用户
if [ ${CP_RECORD_ACTION} == 'add' ] ; then
	if [ ! -n "${RECORD_TEMP}" ] ; then
	#添加到末尾
		echo ${CP_CUR_ACTIVE_USER} >> ${CP_RECORD_DB}
	#总数加加  #注意，expr执行运算的时候，运算符前后要有 空格，否则不能正常执行
		CP_ACTIVE_USER_NUM=$(expr ${CP_ACTIVE_USER_NUM} + 1)
	fi
fi

#是删除一个用
if [ ${CP_RECORD_ACTION} == 'del' ];then
	#检查ip是否存在
	if [ -n "${RECORD_TEMP}" ] ; then
		#删除该ip
		sed "/^${CP_CUR_ACTIVE_USER}$/d" ${CP_RECORD_DB} > ${CP_RECORD_DB}_temp;mv ${CP_RECORD_DB}_temp ${CP_RECORD_DB}
		#总数减减
		CP_ACTIVE_USER_NUM=$(expr ${CP_ACTIVE_USER_NUM} - 1)
		if [ ${CP_ACTIVE_USER_NUM} -lt 0 ];then
			CP_ACTIVE_USER_NUM=0
		fi
	fi
fi


#修改总数
sed "1 c\\${CP_ACTIVE_USER_NUM}" ${CP_RECORD_DB} > ${CP_RECORD_DB}_temp; mv ${CP_RECORD_DB}_temp ${CP_RECORD_DB}

