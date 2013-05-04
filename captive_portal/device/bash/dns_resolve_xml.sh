#!/bin/bash

#dns_xml_path
DNS_XML_FILE=/opt/services/option/domain_option
DNS_XML_ORIGIN_FILE=/opt/www/htdocs/dns_cache.xml

if [ ! $# -eq 1 ];then
	echo "Usage:dns.sh DOMAIN"
	echo "eg.:dns.sh www.baidu.com"
	exit 1;
fi

#check_dns_xml_file_state
if [ ! -f $DNS_XML_FILE ];then
	echo "file domain_option is not exist !"
	if [ ! -f $DNS_XML_ORIGIN_FILE ];then
		echo "file dns_cache.xml not exist !"
		touch $DNS_XML_FILE
		chmod +w $DNS_XML_FILE
		echo -e "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root></root>">$DNS_XML_FILE
		echo "touch file domain_option"
	else
		sudo cp $DNS_XML_ORIGIN_FILE $DNS_XML_FILE
		echo "cp "$DNS_XML_ORIGIN_FILE" "$DNS_XML_FILE
	fi
	chmod +w $DNS_XML_FILE
fi

function check_domain_in_file()
{
	#for test
	echo $1
	echo "run check_domain_in_file function"
	if [ `cat $DNS_XML_FILE | grep "$1" | wc -l` -ge 1 ];then
		echo "$1 exist in file domain_option"
		exit 2;
	else
		return 0;
	fi
}

#check_domain_already_in_file
check_domain_in_file $1;

#check_dns
flag_dns=`grep nameserver /etc/resolv.conf|wc -l`
if [ $flag_dns -eq 0 ];then
	echo "DNS server not configured !"
	exit 3;
fi

#for test
echo $1

DNS_XML_FORMAT_RET=`host -s 5 $1 |grep -v CNAME| awk -v FS=" " '{print $3}'|awk 'BEGIN{FS="\n";RS="";ORS="";OFS=""} {for(i=1;i<=NF;i++) print "<ip",i,">",$i,"</ip",i,">"}'`

if [ `echo $DNS_XML_FORMAT_RET | grep "<ip" | wc -l` -lt 1 ];then
	echo "dns domain name resolve error !"
	exit 4;
fi

#for test
echo $DNS_XML_FORMAT_RET

DNS_XML_OUTPUT="<domain attribute=\""$1"\">"$DNS_XML_FORMAT_RET"</domain>"

#for test
echo $DNS_XML_OUTPUT

#check_domain_already_in_file
check_domain_in_file $1;

#add new dns result to DNS_XML_FILE

if [ `cat $DNS_XML_FILE | grep "</root>" | wc -l` -eq 1 ];then
	sed -i "s:<\/root>:$DNS_XML_OUTPUT&:" $DNS_XML_FILE
	if [ ! $? -eq 0 ];then
		echo "write to file domain_option error!"
		exit 5;
	fi
	
elif [ `cat $DNS_XML_FILE | grep "<root/>" | wc -l` -eq 1 ];then
	sed -i "s:<root\/>:<root>$DNS_XML_OUTPUT<\/root>:" $DNS_XML_FILE
	if [ ! $? -eq 0 ];then
		echo "write to file domain_option error!"
		exit 5;
	fi
	
else
	echo "dns cache file domain_option format wrong!"
	exit 6;
	
fi

exit 0;

