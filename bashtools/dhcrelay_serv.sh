#!/bin/sh

export PATH="$PATH":/sbin:/usr/sbin:/bin:/usr/sbin
SERVFILE="/opt/services/init/dhcrelay_init"


if [ -f $SERVFILE ];then
      $SERVFILE $1
      exit $?
else
      exit 3
fi
       

