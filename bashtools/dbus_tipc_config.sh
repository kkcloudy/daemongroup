#!/bin/bash
SLOT_NUM_FILE=/proc/product_info/board_slot_id
DBUS_SYSTEM_CONFIG_FILE=/etc/dbus-1/system.conf
get_slot_num_file()
{
	slot_num=-1
	temp="cat $SLOT_NUM_FILE";
	slot_num=`$temp`
	if [ $? -eq 1 ]; then
		echo "slot_num_file is not exist"
		exit 101 
	fi	
	return $slot_num
}
set_system_listen()
{
get_slot_num_file
slot_num=$?
slot_num=`expr $slot_num + 1000`
sed_str="s/<listen>unix:path=\/var\/run\/dbus\/system_bus_socket<\/listen>/<listen>unix:path=\/var\/run\/dbus\/system_bus_socket<\/listen><listen>tipc:inst=$slot_num<\/listen>/"
sed_cmd_str="sed -e $sed_str $DBUS_SYSTEM_CONFIG_FILE"
#echo $sed_cmd_str
$sed_cmd_str > ./system.conf_temp
cat ./system.conf_temp > $DBUS_SYSTEM_CONFIG_FILE
}
set_tipc_config()
{
	get_slot_num_file
	slot_num=$?
	echo "tipc-config -netid=1111" >> /etc/init.d/rc.local
	echo "tipc-config -addr=1.1.1$slot_num" >> /etc/init.d/rc.local
	echo "tipc-config -be=eth:eth1-1" >> /etc/init.d/rc.local
}
#sudo mount /blk
#cp /blk/slot_num_file /mnt/slot_num_file
#if [ $? -eq 1 ] ; then
#	exit 0
#fi
set_system_listen
#set_tipc_config
