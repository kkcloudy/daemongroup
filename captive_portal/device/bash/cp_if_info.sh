#!/bin/bash

source cp_start.sh



ip addr | sed '/^[^1-90]/d' | sed 's/^[0-9]*: \(.*\):.*/\1/g' | sed '/lo/d' | while read line
do
	#检查是否有ip
	CP_CUR_IF_IP=$(ip addr show dev "$line" | grep "inet" | awk '$1="inet" { print $2}' | sed -n '1,1p' )
	
	#检查ip的掩码是否符合要求
	CP_CUR_IF_MASK=$(echo ${CP_CUR_IF_IP} | sed "s/.*\/\(.*\)/\1/g" )

	#检查是否在某个id中使用过了
	if [ -e ${CP_DB_FILE} ]; then
		CP_CUR_IF_USED_IN_ID=$(cat ${CP_DB_FILE} | grep "$line" | awk '{print $1}' )
	fi
	
	if [ $CP_CUR_IF_USED_IN_ID ];then
		CUR_IF_USED=1
	else
		CUR_IF_USED=0
	fi
	
	OUT="$line#"
	#输出表格数据
	if [ $CP_CUR_IF_IP ];then
		if [ -n "$CP_CUR_IF_USED_IN_ID" -o ${CP_CUR_IF_MASK} -lt 16 ];then
			OUT=$OUT"false#"
		else
			OUT=$OUT"true#"
		fi
	else
		OUT=$OUT"false#"
	fi
	
	if [ $CP_CUR_IF_IP ];then
		OUT=$OUT"$CP_CUR_IF_IP#"
	else
		OUT=$OUT"has no ip#"
	fi
	
	if [ $CP_CUR_IF_USED_IN_ID ];then
		OUT=$OUT"${CP_CUR_IF_USED_IN_ID}#"
	else
		OUT=$OUT"x#"
	fi
	
	if [ ! -n "$CP_CUR_IF_MASK" ]; then 
		CP_CUR_IF_MASK=0;
	fi
	
	if [ ${CP_CUR_IF_MASK} -lt 16 ];then
		OUT=$OUT"MASK_ERR"
	else
		OUT=$OUT"MASK_OK"
	fi
	
	echo $OUT
done
