PATH_rn=/var/run/apache2/dhcp_rang.tmp
STAT_PATH=/var/run/apache2/dhcp_stat.tmp
CON_PATH=/opt/services/conf/dhcp_conf.conf
DHCP_LEASE=/var/run/apache2/dhcp_lease.conf

if [ ! -f $PATH_rn ]
	then
		touch $PATH_rn
		chmod 666 $PATH_rn
fi
if [ ! -f $STAT_PATH ]
	then
		touch $STAT_PATH
		chmod 666 $STAT_PATH
fi
if [ -f $CON_PATH ]
	then
cat $CON_PATH|sed -n '/range/p'|sed -n 's/;//p'|sed -n 's/./ /p'|sed -n 's/range//p'|sed -n 's/^ *//p' > $PATH_rn 
fi
if [ -f $DHCP_LEASE ]
	then
cat $DHCP_LEASE |awk 'BEGIN{FS=" ";RS="\n";i=0}{if($5 ~/active/){i++}}END{print i}'>$STAT_PATH 
fi
#cat test.txt |sed -n '/lease.*{/p'|wc -l 
