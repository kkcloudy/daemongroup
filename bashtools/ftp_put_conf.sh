#! /bin/bash

if [ $# -ne 5 ];then
	echo "Useage:$0 ip port user passwd files"
	exit 1;
fi
#ftp -u ftp://admin:admin@192.168.2.179:21/test/ cli.conf

CONF_TRANS_STATUS="/var/run/conf_trans_status"
CONF_TRANS_FAILED="/var/run/conf_trans_failed"

echo "1" > $CONF_TRANS_STATUS
chmod 666 $CONF_TRANS_STATUS
(
mount /blk
cd /blk

ftp -u "ftp://$3:$4@$1:$2/" $5 -q 5 > ${CONF_TRANS_FAILED}_temp 2>&1
ret=$?
cat ${CONF_TRANS_FAILED}_temp | tail -5 > ${CONF_TRANS_FAILED}
line=`wc -l $CONF_TRANS_FAILED | awk '{printf $1}'`
status=3
if [ $? -eq 0 ] && [ $line -eq 1 ] ;then
		status=2
fi

if [ $? -eq 0 ];then
	echo $status > $CONF_TRANS_STATUS
else
	echo $status > $CONF_TRANS_STATUS
fi
 
chmod 666 $CONF_TRANS_FAILED
cd /
sleep 5
umount /blk

chmod 666 $CONF_TRANS_STATUS
) &
