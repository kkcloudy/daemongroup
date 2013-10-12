#!/bin/sh
mcast_config_save_file=/var/run/hmd/mcast_config_save
if [ -f $mcast_config_save_file ] ; then
	sed -i '/'$1'/'d $mcast_config_save_file 
fi
echo "set global_bridge_mcast $1 disable" >> $mcast_config_save_file 

