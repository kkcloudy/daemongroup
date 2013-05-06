#!/bin/sh
OPTIONFILE=/opt/services/option/ntp_option
crontime=`awk -F "</*cront>" '{print $2}' $OPTIONFILE`
		
if [ "$crontime"x = ""x ];then
	cronstr=`echo '*/10 * * * * sudo /usr/bin/crontimentp.sh'`
else         
      cronlong=`echo $crontime |cut -f 1 -d "^"`     
      croncycle=`echo $crontime |sed 's/.*^//g'`      
      cronltime=`expr $cronlong + 0`      
      if [ "$croncycle"x = "mins"x ]; then
      	cronstr=`echo '*/'$cronltime' * * * * sudo /usr/bin/crontimentp.sh'`
      fi
      
      if [ "$croncycle"x = "hours"x ]; then
        cronstr=`echo '0 */'$cronltime' * * * sudo /usr/bin/crontimentp.sh'`
      fi
      
      if [ "$croncycle"x = "days"x ]; then
      	cronstr=`echo '0 0 */'$cronltime' * * sudo /usr/bin/crontimentp.sh'`
      fi 
fi
(crontab -l 2>&- | grep -v 'crontimentp'
         echo "$cronstr"
        ) | crontab - 2>&-
