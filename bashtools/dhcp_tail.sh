DHCP_TAIL=/var/run/apache2/dhcp_tail.conf
CONF_FILE="/var/run/apache2/dhcp_head.conf"
CONF_PATH=/opt/services/conf/dhcp_conf.conf
if [ $1 == "0" ]
	then
		echo "##" > $DHCP_TAIL
else
	echo "##" > $DHCP_TAIL	
	echo $1|sed 's/-/ /g'|awk 'BEGIN{FS=" ";RS="^"}{print "host " NR " {" ;print "hardware ethernet "$2";";print "fixed-address "$1";";print "}" }' >> $DHCP_TAIL		
fi
 cat $CONF_FILE > $CONF_PATH
 cat $DHCP_TAIL >> $CONF_PATH


