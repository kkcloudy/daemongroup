#! /bin/bash
#wget -N -P /mnt --user=%s --password=%s %s
#wget -N -P /mnt --user=admin --password=admin --timeout=5 --tries=1 ftp://192.168.2.179:21/AW1.2.10.2827.X7X5.IMG
if [ $# -lt 4 ];then
	echo "Use age: $0 ip port filepath protocal [username] [password] "
	exit 1;
fi

CONF_TRANS_STATUS="/var/run/conf_trans_status"
CONF_TRANS_FAILED="/var/run/conf_trans_failed"

url="$4://$1:$2/$3"
file=`echo $3 | sed "s/.*[\/]\([^\/].*\)$/\1/g"`

echo "1" > $CONF_TRANS_STATUS
chmod 666 $CONF_TRANS_STATUS

(
(wget -N -P /mnt --connect-timeout=5 --read-timeout=120 --tries=1 --user=$5 --password=$6 $url
if [ $? -eq 0 ];then
	echo "2" > $CONF_TRANS_STATUS
else
	echo "3" > $CONF_TRANS_STATUS
fi
) 2>${CONF_TRANS_FAILED}_temp
cat ${CONF_TRANS_FAILED}_temp | tail -5 > ${CONF_TRANS_FAILED}
chmod 666 $CONF_TRANS_FAILED

#if [ $(cat $CONF_TRANS_FAILED | grep -c saved) -gt 0 ] && [ $(cat $CONF_TRANS_FAILED | grep -c "no newer") -eq 0 ];then
#	echo "2" > $CONF_TRANS_STATUS
#else
#	echo "3" > $CONF_TRANS_STATUS
#fi
chmod 666 $CONF_TRANS_STATUS
sor.sh cp $file 30 > /dev/null
) &

