#!/bin/sh

LOG="/mnt/patch.log"

if [ $# -lt 1 ] ; then
	echo "need patch name without .sp"
	exit 1
fi

rm $LOG >& /dev/null

if [ -d /mnt/patch/$1 ] ; then
	rm -rf /mnt/patch/$1 >> $LOG
	echo "rm /mnt/patch/$1 success" >> $LOG
fi

if [ -e /mnt/patch/$1.sp ] ; then 
	rm /mnt/patch/$1.sp >> $LOG 
	echo "rm /mnt/patch/$1.sp succeed" >> $LOG 
else
	echo "/mnt/patch no: $1.sp " >> $LOG
fi

if [ `sor.sh ls patch/$1.sp 20 | wc -l` -ge 1 ] ; then
	sor.sh rm patch/$1.sp 30 >> $LOG
	echo "rm /blk/patch/$1.sp succeed" >> $LOG
else
	echo "/blk/patch/ no: $1.sp" >> $LOG
fi

