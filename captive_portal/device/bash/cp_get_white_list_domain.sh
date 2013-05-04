#!/bin/bash

source cp_start.sh

if [ ! $# -eq 1 ] ; then
	echo "Usage: cp_get_white_list_domain.sh ID"
	exit 1
fi

#. portal_get_cpid.sh
source cp_start.sh
	

CP_ID=$1
cat ${CP_WHITE_BLACK_LIST} | grep "^${CP_WHITE_LIST_FLAG_DOMAIN} ${CP_ID} " | awk '{print $3}'

