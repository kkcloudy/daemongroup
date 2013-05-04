#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: portal_add_legal_ip.sh ID"
	exit 1
fi

#. portal_get_cpid.sh
source cp_start.sh
	

CP_ID=$1
cat ${CP_WHITE_BLACK_LIST} | grep "^${CP_WHITE_LIST_FLAG} ${CP_ID} " | awk '{print $3":"$4}'

