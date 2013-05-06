sudo /opt/bin/dhcp6d -6 -lf /etc/dhcpd6.leases & >/dev/null
sleep 1
ps -ef|grep dhcp6d|grep -v grep >/dev/null
result=$?
if [ $result -eq 0 ];then
  exit 1;
else 
  exit 2;
fi

