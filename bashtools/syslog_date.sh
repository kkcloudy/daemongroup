#!/bin/sh

FILTERFILE=/var/run/filter_date

if ! test -f $FILTERFILE; then
        sudo touch $FILTERFILE
        sudo chmod 666 $FILTERFILE
else
        sudo chmod 666 $FILTERFILE
fi

if [ $# -eq 1 ];then	
        old_date=`date -d "-$1 days" +"%Y%m%d"`        
        cur_date=`date -d today +"%Y%m%d"`      
        #select folders to the file
        ls -l /var/log/systemlog/ | grep ^d | awk -v old_date=$old_date -v cur_date=$cur_date '{ if(( $9 >= (old_date + 0) )&&( $9 <= (cur_date + 0) )) print $9}' > $FILTERFILE
	#list other folders and delete
	rmfile=`ls -l /var/log/systemlog/ | grep ^d | awk -v old_date=$old_date -v cur_date=$cur_date '{ if(( $9 < (old_date + 0) )||( $9 > (cur_date + 0) )) print $9}'`
	for loop in $rmfile
	do	
	sudo rm -rf /var/log/systemlog/$loop
	done
else
         exit 2
fi
