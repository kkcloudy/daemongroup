#!/bin/bash

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/bin


INDEX=`brctl showmacs $1 |grep $2|awk  '{ print $1;}'`
#echo $INDEX

brctl show|sed '1d'|awk  'BEGIN{flag = 0;}{ if($1=="ebr1"&&(NF==4)) {flag =1;print $4;next} if(flag==1&& NF!=4&& NF!=3){print $1;} if(flag==1&& NF!=1){flag=0;}}'|sed -n "$INDEX"p
