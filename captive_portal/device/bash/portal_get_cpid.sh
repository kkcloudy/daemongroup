#!/bin/bash

#CP_DB_FILE='/var/run/cpp/profile_db'
source cp_start.sh
PORTAL_USRNAME=`whoami`
CP_ID=$(awk -v FS="	" -v usr="$PORTAL_USRNAME" '$4==usr { print $1 }' $CP_DB_FILE)
CP_IF=$(awk -v FS="	" -v usr="$PORTAL_USRNAME" '$4==usr { print $6 }' $CP_DB_FILE)

#echo $cp_id

if [ ! $CP_ID ] ; then
	echo "SYSTEM doesn't have portal account information for this user."
	exit 2;
fi

CP_FILTER_DEFAULT=CP_${CP_ID}_F_DEFAULT
CP_NAT_DEFAULT=CP_${CP_ID}_N_DEFAULT
CP_FILTER_AUTHORIZED=CP_${CP_ID}_F_AUTHORIZED_DEFAULT
CP_NAT_AUTHORIZED=CP_${CP_ID}_N_AUTHORIZED_DEFAULT
#CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTHORIZED_DEFAULT
#CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTHORIZED
CP_SET_AUTHORIZED=CP_${CP_ID}_S_AUTH

# Should not use exit, which will cause calling shell exit too.
#exit 0;

return 0;
