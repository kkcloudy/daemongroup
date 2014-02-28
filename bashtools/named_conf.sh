#!/bin/sh

#source vtysh_start.sh
#named_conf.sh OPCMD [argument]...
#OPCMD:add delete modify
#argument: A.B.C.D www nihao.com
if [ $# -ne 4 ] ; then
	echo "Usage:named_conf.sh OPCMD  IP Subdomain Second-Level-Domains "
	exit -1
fi
opcmd=$1
IP=$2
DOMAIN_NAME=$3
DOMAIN=$4

CONFIG_DIR=/etc/bind
NAMED_CONF_NAME=named.conf
NAMED_CONF=$CONFIG_DIR/$NAMED_CONF_NAME
NAMED_CONF_OPT=$CONFIG_DIR/named.conf.opt

#*****************************************************
#bind_forward 192.168.1.7 /etc/bind/named.conf.opt
#*****************************************************
bind_forward()
{	
	ip=$1
	file=$2
	sed "/\<listen-on port/a\forwarders { $ip;};" $file > $file.tmp
	cp $file.tmp $file
	sed "/\<forwarders/a\forward first;" $file > $file.tmp
	cp $file.tmp $file
}

bind_forward_del()
{	
	file=$1
	sed "/\<forward/d" $file > $file.tmp
	cp $file.tmp $file
}
#*****************************************************
#bind_add_A() www autelan923.com 192.168.73.221
#*****************************************************
bind_add_A()
{
	domain_name=$1
	domain=$2
	ip=$3
	ret=`cat $NAMED_CONF  | grep $domain | wc -l `
	if [ $ret -eq 0 ] ; then
		echo "include \"$CONFIG_DIR/$domain.opt\";" >> $NAMED_CONF 
		echo "zone \"$domain.\" IN {
			type master;
			file \"$domain.zone\";
			allow-update { none; };
			};" > $CONFIG_DIR/$domain.opt
 
		echo "\$TTL    1D" > $CONFIG_DIR/$domain.zone
		echo "@             IN   SOA   $domain.  root.$domain. (" >> $CONFIG_DIR/$domain.zone
                echo "	1053891162" >> $CONFIG_DIR/$domain.zone
                echo "	3H" >> $CONFIG_DIR/$domain.zone
                echo "	15M" >> $CONFIG_DIR/$domain.zone
                echo "	1W" >> $CONFIG_DIR/$domain.zone
                echo "	1D )" >> $CONFIG_DIR/$domain.zone

 	        echo "    IN  NS          $domain." >> $CONFIG_DIR/$domain.zone
		echo "$domain_name                IN  A              $ip" >> $CONFIG_DIR/$domain.zone
	else
		ret=`cat $CONFIG_DIR/$domain.zone  | grep $domain_name | wc -l `
		if [ $ret -eq 0 ];then
			echo "$domain_name                IN  A              $ip" >> $CONFIG_DIR/$domain.zone
		else
			echo "$domain_name exist"
			exit 1
		fi
	fi

}
#bind_modify_A()
#bind_modify_A www
#
#
#bind_modify_A()

	
	

#*****************************************************
#bind_delete_A()
#bind_delete_A www autelan923.com
#*****************************************************

bind_delete_A()
{
	domain=$2
	domain_name=$1
	for f1 in $CONFIG_DIR/* ; do
    		if [ -f $f1 ] ; then
    			filename=`basename $f1`
    			if [ $domain.zone = $filename ] ; then
    				ret=`cat $f1  | grep $domain_name | wc -l `
					if [ $ret -eq 0 ];then
						echo "$domain_name don't exist"
						exit 1
					else
						sed "/$domain_name/d" $f1 > $f1.tmp
						cp $f1.tmp $f1
						
					fi	
    			fi
        	
    		fi 
  	done
	
}
#*******************************
#bind_delete_domain autelan923.com
#******************************
bind_delete_domain()
{
	domain=$1
	cd $CONFIG_DIR
	if [ -f $domain.zone ] ; then
    		rm $domain.zone*
    	fi
    	
    	if [ -f $domain.opt ] ; then
    		rm $domain.opt
    	fi
	cat $NAMED_CONF  | grep -v $domain > $NAMED_CONF.tmp
	cp $NAMED_CONF.tmp $NAMED_CONF
}
bind_save_A()
{
	for f1 in `ls $CONFIG_DIR/* | grep *.zone `; do
    		if [ -f $f1 ] ; then
    			filename=`basename $f1`
    			if [ $domain.zone = $filename ] ; then
    				ret=`cat $f1  | grep $domain_name | wc -l `
					if [ $ret -eq 0 ];then
						echo "$domain_name don't exist"
						exit 1
					else
						sed "/$domain_name/d" $f1 > $f1.tmp
						cp $f1.tmp $f1
						
					fi	
    			fi
        	
    		fi 
  	done
	
}
#**************************
#bind_log default|all
#**************************

bind_log_all()
{	
	file=$1
	sed "/\<severity/d" $file > $file.tmp
	cp $file.tmp $file
	sed "/\<syslog daemon/a\severity info;" $file > $file.tmp
	cp $file.tmp $file
	
}

bind_log_default()
{	
	file=$1
	sed "/\<severity/d" $file > $file.tmp
	cp $file.tmp $file
	sed "/\<syslog daemon/a\severity warning;" $file > $file.tmp
	cp $file.tmp $file
}

case "$opcmd" in
      add)
      	bind_add_A $DOMAIN_NAME $DOMAIN $IP
	;;
      delete)
          bind_delete_A $DOMAIN_NAME $DOMAIN
          ;;
	 del_domain)
           bind_delete_domain $DOMAIN
		   ;;
      modify)
          ls /blk/$opfilename > $resultoutputfile
          ;;
      forward)
          bind_forward $IP $NAMED_CONF_OPT
          ;;
      forward_del)
          bind_forward_del $NAMED_CONF_OPT
          ;;   
      log)
          bind_log_all $CONFIG_DIR/logging.opt
          ;;  
      log_def)
          bind_log_default $CONFIG_DIR/logging.opt
          ;;     
      *)
      	echo "unknow opcmd"
#      logger -p daemon.info -t $LOGTAG "unknow opcmd."
    esac 


