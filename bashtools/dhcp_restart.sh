pkill dhcpd 
sudo /etc/init.d/dhcpd start >/dev/null
sleep 1
ps -ef|grep dhcpd|grep -v grep >/dev/null
result=$?
if [ $result -eq 0 ];then
  exit 1;
else 
  exit 2;
fi

