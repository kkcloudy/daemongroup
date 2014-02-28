#!/bin/sh 

CONFIG_DIR=/etc/bind

if [ $# -ne 1 ] ; then
	echo "Usage: "
	exit -1
fi
opcmd=$1
case "$opcmd" in
      sec_domain)
      	ls $CONFIG_DIR|grep "zone$"|sed -e "/localhost/d" -e "s/.zone//"
	;;
      domain)
          for f1 in `ls $CONFIG_DIR/*|grep "zone$"|grep -v "localhost"` ; do
          	filename=`basename $f1`
			domain=`echo $filename |sed "s/.zone//"`
			if [ -f $f1 ] ; then	
				sed -n "s/IN.*[^O]A/$domain/p" $f1 | awk '{print $3"\t\t"$1"."$2"\t\t"$2}'       	
   			fi 
   	  done
          ;;
      forward)
      	sed -n -e "/\<forwarders/p" $CONFIG_DIR/named.conf.opt | sed -e "s/;//g" -e "s/{//g" -e "s/}//g"
#        sed -e -n "/\<forwarders/p" -e "s/;//g" $CONFIG_DIR/named.conf.opt
	;;
      *)
      	echo "unknow opcmd"
esac 
