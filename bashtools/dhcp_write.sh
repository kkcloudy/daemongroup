DHCP_file=/var/run/apache2/dhcp_head.conf
DHCP_file_t=/var/run/apache2/dhcp_tail.conf
DHCP_PATH=/opt/services/conf/dhcp_conf.conf
Dwrite_path=/opt/awk/dhcp_write.awk
echo $*|awk -f $Dwrite_path>$DHCP_file
cat $DHCP_file>$DHCP_PATH
cat $DHCP_file_t>>$DHCP_PATH

