#!/bin/sh
source vtysh_start.sh
CLI="ps -ef | grep $1 | grep -v \"grep\" | wc -l"
if [ $# -ne 1 ];then        
                exit 1
fi
count_snmp=`ps -C $1 -o pid=`
outvalue=`echo $?`
if [ $outvalue -eq 0 ];then        
             exit 2
        else
             exit 3
fi
