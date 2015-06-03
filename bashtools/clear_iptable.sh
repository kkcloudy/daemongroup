#!/bin/sh

INSTANCE_ID=$1
INSTANCE_TYPE=$2
FAMILY=$3
slot_id=`cat /dbm/local_board/slot_id 2>/dev/null`

for INTERFACE in $(cat /var/run/config/slot$slot_id/hansi_eag$INSTANCE_ID |grep "add captive-interface" |awk '{print $NF}')
	do
		sudo /usr/bin/cp_del_portal_interface.sh $INSTANCE_ID $INSTANCE_TYPE $INTERFACE $FAMILY
	done

sudo /usr/bin/cp_del_portal_id.sh $INSTANCE_ID $INSTANCE_TYPE $FAMILY


