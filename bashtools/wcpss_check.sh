#!/bin/sh
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin

NORMAL=1
CHECKED_TIMES=0
CHECK_INTERVAL=40
LOCAL_SLOT_ID=`cat /dbm/local_board/slot_id 2>/dev/null`

#asd memory and process check
check_asd()
{
	if [ -s "/var/run/MaxMemValue_asd" ] ; then
		MAX_ASDMEM=`cat /var/run/MaxMemValue_asd 2>/dev/null`
	else
		MAX_ASDMEM=30
	fi
	#	sh-3.1# ps aux | grep asd
	#	root     11907  0.0  0.1  15024  4968 ?        Ss   00:02   0:00 /opt/bin/asd 0
	#	root     11931  0.0  0.1  15024  4968 ?        S    00:02   0:00 /opt/bin/asd 0
	#	root     11932  0.0  0.1  15024  4968 ?        S    00:02   0:00 /opt/bin/asd 0
	#	root     11933  0.0  0.1  15024  4968 ?        S    00:02   0:00 /opt/bin/asd 0
	#	root     14994  0.0  0.0   2132   656 ttyS0    S+   00:09   0:00 grep asd
	VRRID=1
	while [ $VRRID -le 16 ]
	do
		if [ -s "/var/run/wcpss/asd0_$VRRID.pid" ] ; then
			echo 0 > /var/run/wcpss/process_check_flag0_$VRRID
			echo 0 > /var/run/wcpss/memory_check_flag0_$VRRID	
			ASDPID=`cat /var/run/wcpss/asd0_$VRRID.pid 2>/dev/null`
			ps aux | grep asd | grep $ASDPID > /dev/null
			ret=$?
			if [ ! $ret -eq 0 ] ; then
				echo 1 > /var/run/wcpss/process_check_flag0_$VRRID
				echo &date >> /var/log/baseguard.log
				echo "ASD_ProcessCheck: hansi $LOCAL_SLOT_ID-$VRRID ASD pid $ASDPID dead,shall restart wcpss.
				Checked times $CHECKED_TIMES." >> /var/log/baseguard.log
			else
				ASDMEM=`ps aux | grep asd | grep $ASDPID | awk '{ print $4 }'`
				A=`echo "${ASDMEM} > $MAX_ASDMEM"|bc`
				if [[ $A -eq 0 ]] ; then
					echo "ASD_MEMCHECK: ASD memory is normal,do nothing! Checked times $CHECKED_TIMES." > /dev/null
				elif [[ $A -eq 1 ]] ; then
					echo &date >> /var/log/baseguard.log
					echo "ASD_MEMCHECK: hansi $LOCAL_SLOT_ID-$VRRID ASD process $ASDPID memory use: $ASDMEM large than $MAX_ASDMEM kill it.
					Checked times $CHECKED_TIMES." >> /var/log/baseguard.log
					echo 1 > /var/run/wcpss/memory_check_flag0_$VRRID
				fi
			fi
		fi
		VRRID=$(($VRRID + 1 ))
	done
}

#wid process check
check_wid()
{
	#	sh-3.1# cat /var/run/wcpss/wid0_1.pid  
	#	Thu Jan  1 00:02:35 1970 11738
	VRRID=1
	while [ $VRRID -le 16 ]
	do
		if [ -s "/var/run/wcpss/wid0_$VRRID.pid" ] ; then
			WIDPID=`cat /var/run/wcpss/wid0_$VRRID.pid | awk '{ print $6 }' 2>/dev/null`
			ps aux | grep wid | grep $WIDPID > /dev/null
			ret=$?
			if [ ! $ret -eq 0 ] ; then
				echo 2 > /var/run/wcpss/process_check_flag0_$VRRID
				echo &date >> /var/log/baseguard.log
				echo "WID_ProcessCheck: hansi $LOCAL_SLOT_ID-$VRRID WID pid $WIDPID dead,shall restart wcpss.
				Checked times $CHECKED_TIMES." >> /var/log/baseguard.log

			fi
		fi
		VRRID=$(($VRRID + 1 ))
	done
}
#check_had()

runf()
{
	if [ $NORMAL = "1" ] ; then
		eval $1
	fi
}

while [ $NORMAL = "1" ]
do
	CHECKED_TIMES=$(($CHECKED_TIMES + 1))
	runf check_asd
	runf check_wid
	sleep ${CHECK_INTERVAL}
done

