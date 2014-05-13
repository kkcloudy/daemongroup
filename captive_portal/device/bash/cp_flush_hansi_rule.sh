#! /bin/sh

PATH=/sbin:/bin:/opt/bin:/usr/sbin:/usr/bin

HANSI_TYPE=$1
HANSI_ID=$2

if [ $HANSI_TYPE -eq 1 ]; then
	HANSI_CHAR="L"
else
	HANSI_CHAR="R"
fi

echo "HANSI_TYPE = ${HANSI_TYPE}, HANSI_ID = ${HANSI_ID}, HANSI_CHAR = ${HANSI_CHAR}"

CP_DNAT="CP_DNAT"
CP_FILTER="CP_FILTER"
ASD_DNAT="ASD_DNAT"
ASD_FILTER="ASD_FILTER"
ASD_PRE_AUTH_N="ASD_PRE_AUTH_"${HANSI_CHAR}${HANSI_ID}"_N"
ASD_PRE_AUTH_F="ASD_PRE_AUTH_"${HANSI_CHAR}${HANSI_ID}"_F"

CP_FILTER_DEFAULT="CP_"${HANSI_CHAR}${HANSI_ID}"_F_DEFAULT"
CP_FILTER_AUTHORIZED_DEFAULT="CP_"${HANSI_CHAR}${HANSI_ID}"_F_AUTH_DEFAULT"
CP_NAT_DEFAULT="CP_"${HANSI_CHAR}${HANSI_ID}"_N_DEFAULT"
CP_NAT_AUTHORIZED_DEFAULT="CP_"${HANSI_CHAR}${HANSI_ID}"_N_AUTH_DEFAULT"
CP_IPHASH_SET="CP_"${HANSI_CHAR}${HANSI_ID}"_AUTHORIZED_SET"

CP_ID_FILE="/var/run/cpp/CP_"${HANSI_CHAR}${HANSI_ID}
CP_INTF_FILE_PREFIX="/var/run/cpp/CP_IF_INFO_"
CP_TAG_FILE_PREFIX="/var/run/cpp/CP_TAG_INFO_"

[ -d /var/run/cpp ] || mkdir /var/run/cpp

function get_intf_name()
{
	_intf_file=$1
	_len1=`expr length "${CP_INTF_FILE_PREFIX}"`
	_len2=`expr length "$_intf_file"`
	_pos=$(($_len1+1))
	_len=$(($_len2-$_len1))
	echo `expr substr "$_intf_file" $_pos $_len`
}

function get_flux_tag()
{
	_tag_file=$1
	_len1=`expr length "${CP_TAG_FILE_PREFIX}"`
	_len2=`expr length "$_tag_file"`
	_pos=$(($_len1+1))
	_len=$(($_len2-$_len1))
	echo `expr substr "$_tag_file" $_pos $_len`
}

function flush_intf_rule()
{
	echo "---flush_intf_rule---"
	
	_intf_name=$1
	CP_FILTER_AUTH_IF=CP_${HANSI_CHAR}${HANSI_ID}_F_${_intf_name}
	CP_FILTER_AUTH_IF_IN=CP_${HANSI_CHAR}${HANSI_ID}_F_${_intf_name}_IN
	CP_NAT_AUTH_IF=CP_${HANSI_CHAR}${HANSI_ID}_N_${_intf_name}

	iptables -D ${CP_FILTER} -i ${_intf_name} -j ${CP_FILTER_DEFAULT}
	iptables -D ${CP_FILTER} -i ${_intf_name} -j ${CP_FILTER_AUTH_IF}
	iptables -D ${CP_FILTER} -o ${_intf_name} -j ${CP_FILTER_AUTH_IF_IN}

	iptables -t nat -D ${CP_DNAT} -i ${_intf_name} -j ${CP_NAT_DEFAULT}
	iptables -t nat -D ${CP_DNAT} -i ${_intf_name} -j ${CP_NAT_AUTH_IF}

	iptables -F ${CP_FILTER_AUTH_IF}
	iptables -X ${CP_FILTER_AUTH_IF}
	
	iptables -F ${CP_FILTER_AUTH_IF_IN}
	iptables -X ${CP_FILTER_AUTH_IF_IN}

	iptables -t nat -F ${CP_NAT_AUTH_IF}
	iptables -t nat -X ${CP_NAT_AUTH_IF}
}

function flush_tag_rule()
{
	echo "---flush_tag_rule---"
	_flux_tag=$1
	
	iptables -D ${CP_FILTER} -m mark --mark ${_flux_tag} -j ${CP_FILTER_DEFAULT}
	iptables -t nat -D ${CP_DNAT} -m mark --mark ${_flux_tag} -j ${CP_NAT_DEFAULT}
}

function flush_hansi_rule()
{
	echo "---flush_hansi_rule---"
	# del MAC_PRE_AUTH chain 
	iptables -nL ${ASD_PRE_AUTH_F} > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -D ${ASD_FILTER} -j ${ASD_PRE_AUTH_F}
		iptables -F ${ASD_PRE_AUTH_F}
		iptables -X ${ASD_PRE_AUTH_F}
	fi
	
	iptables -nL ${ASD_PRE_AUTH_N} -t nat > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -t nat -D ${ASD_DNAT} -j ${ASD_PRE_AUTH_N}
		iptables -F ${ASD_PRE_AUTH_N} -t nat
		iptables -X ${ASD_PRE_AUTH_N} -t nat
	fi
	
	iptables -nL ${CP_FILTER_DEFAULT} > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -F ${CP_FILTER_DEFAULT}
		iptables -X ${CP_FILTER_DEFAULT}
	fi
	
	iptables -nL ${CP_NAT_DEFAULT} -t nat > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -t nat -F ${CP_NAT_DEFAULT}
		iptables -t nat -X ${CP_NAT_DEFAULT}
	fi
	
	iptables -nL ${CP_FILTER_AUTHORIZED_DEFAULT} > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -F ${CP_FILTER_AUTHORIZED_DEFAULT}
		iptables -X ${CP_FILTER_AUTHORIZED_DEFAULT}
	fi
	
	iptables -nL ${CP_NAT_AUTHORIZED_DEFAULT} -t nat > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		iptables -t nat -F ${CP_NAT_AUTHORIZED_DEFAULT}
		iptables -t nat -X ${CP_NAT_AUTHORIZED_DEFAULT}
	fi
	
	ipset -nL ${CP_IPHASH_SET} > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		ipset -F ${CP_IPHASH_SET}
		ipset -X ${CP_IPHASH_SET}
	fi
}

intf_file_list=`ls ${CP_INTF_FILE_PREFIX}* 2>/dev/null`
for intf_file in $intf_file_list ; do
	echo "$intf_file"
	id=`cat "$intf_file" 2>/dev/null`
	if [ "$id" = "$HANSI_CHAR""$HANSI_ID" ] ; then
		echo "${intf_file} is belong to ${id}"
		intf_name=`get_intf_name ${intf_file}`
		echo intf_name is ${intf_name}
		flush_intf_rule ${intf_name}
		rm -f "$intf_file"
	fi
done

tag_file_list=`ls ${CP_TAG_FILE_PREFIX}* 2>/dev/null`
for tag_file in $tag_file_list ; do
	echo "$tag_file"
	id=`cat "$tag_file" 2>/dev/null`
	if [ "$id" = "${HANSI_CHAR}""${HANSI_ID}" ] ; then
		echo "${tag_file} is belong to ${id}"
		flux_tag=`get_flux_tag $tag_file`
		echo flux_tag is ${flux_tag}
		flush_tag_rule ${flux_tag}
		rm -f "$tag_file"
	fi
done

flush_hansi_rule
rm -f ${CP_ID_FILE}
