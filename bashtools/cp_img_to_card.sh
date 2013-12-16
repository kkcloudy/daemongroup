#!/bin/sh

device="sdb"
if [ ! $# == 1 ]; then
	echo "Usage:input parameter error"
    	exit 1
fi

if [ ! -b "/dev/$device" ];then
    echo "not cheack usb device"
    exit 1
fi


if ! `mkdir /home/usb`;then
 	if [-d "/home/usb"];then
		rm -r /home/usb
		mkdir /home/usb
	 else
		echo "creat usb file failed"
		 exit 1

	fi
fi

device_rel="sdb1"
if [ -b "/dev/sdb1" ];then
	device_rel="sdb1"
elif [ -b "/dev/sdb2" ];then
	device_rel="sdb2"
elif [ -b "/dev/sdb3" ];then
	device_rel="sdb3"
elif [ -b "/dev/sdb4" ];then
	device_rel="sdb4"
fi
echo "find USB $device_rel"

if ! `sudo mount /dev/$device /home/usb/`;then
    echo " if usb device have partition,will mount $device_rel partition"
    if [ -b "/dev/$device_rel" ];then
	if ! `sudo mount /dev/$device_rel /home/usb/`;then

   		 if [-d "/home/usb"];then
			sudo umount /home/usb
			rm -r /home/usb
			echo "mount usb device failed"
			exit 1
		else
			echo "mount /mnt/usb file faile"
			exit 1
		fi
	  fi
    else 
	echo "not find usb device $device_rel"
	exit 1
    fi
fi

#	free_mem_size=`free -m | grep "Mem" | awk '{print $4}'`
#	echo "free_mem_size = $free_mem_size MB"
#	file_size = `ls -lh /home/usb/$1 | awk '{print $5}'`
#	file_size=`du -m /mnt/$1 | awk '{print $1}'`
#	echo "IMGfile_size = file_size"
#if [ `expr $free_mem_size / 2` -gt $file_size ];then
#	    echo "free mem is bigger enough"

TOTALMEM=`free | grep Mem | awk '{ print $2}'`
FREEMEM=`free | grep Mem | awk '{ print $4}'`
FREEPER=$(($FREEMEM * 100 / $TOTALMEM))
if [ $FREEPER -gt 15 ] ; then
	CRITMSG="CRITICAL: Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
	if [ -f /home/usb/$1 ];then
		if ! `cp /home/usb/$1 /mnt`;then
			sudo umount /home/usb
			rm -r /home/usb
			exit 1
		fi
	else
		echo "usb device not file"
		sudo umount /home/usb
		rm -r /home/usb
		exit 1
	fi

	if ! `sudo umount /home/usb`;then
    		echo "umount usb device failed"
    		exit 1
	fi
else
    		echo "free mem is not enough"
		sudo umount /home/usb
		rm -r /home/usb
		exit 1
fi
	
rm -r /home/usb
#echo "copy $1 to mnt success"
exit 0
