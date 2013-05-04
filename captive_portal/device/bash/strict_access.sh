#! /bin/sh

export PATH=$PATH:/opt/bin/

STRICT_INPUT="STRICT_INPUT"
STRICT_OUTPUT="STRICT_OUTPUT"
STRICT_FORWARD="STRICT_FORWARD"

strict_input_enable()
{
	iptables -F $STRICT_INPUT
	
	#Allow dhcp
	iptables -A $STRICT_INPUT -p udp --sport 68 --dport 67 -j RETURN
	iptables -A $STRICT_INPUT -p udp --sport 67 --dport 68 -j RETURN

	#ssh telnet
	iptables -A $STRICT_INPUT -p tcp --sport 22:23 -j RETURN 
	iptables -A $STRICT_INPUT -p tcp --dport 22:23 -j RETURN

	#asd wid wsm
	iptables -A $STRICT_INPUT -p udp --sport 5246:5247 -j RETURN 
	iptables -A $STRICT_INPUT -p udp --dport 5246:5247 -j RETURN

	iptables -A $STRICT_INPUT -p udp --sport 19528:19544 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 19528:19544 -j RETURN

	iptables -A $STRICT_INPUT -p udp --sport 29527:29543 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 29527:29543 -j RETURN

	#bsd
	iptables -A $STRICT_INPUT -p udp --sport 44443 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 44443 -j RETURN
	
	# asd wsm-unknown
	iptables -A $STRICT_INPUT -p udp --sport 44642 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 44642 -j RETURN
	iptables -A $STRICT_INPUT -p udp --sport 8888:8890 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 8888:8890 -j RETURN
	
	#ntpd
	iptables -A $STRICT_INPUT -p udp --sport 123 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 123 -j RETURN
	
	#radius
	iptables -A $STRICT_INPUT -p udp --sport 1812:1813 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 1812:1813 -j RETURN
	iptables -A $STRICT_INPUT -p udp --sport 1645:1646 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 1645:1646 -j RETURN
	iptables -A $STRICT_INPUT -p udp --sport 3645:3646 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 3645:3646 -j RETURN
	iptables -A $STRICT_INPUT -p udp --dport 3799 -j RETURN

	#portal
	iptables -A $STRICT_INPUT -p udp --dport 2000 -j RETURN

	#eag pdc rdc backup
	iptables -A $STRICT_INPUT -p tcp --sport 2002:2004 -j RETURN 
	iptables -A $STRICT_INPUT -p tcp --dport 2002:2004 -j RETURN

	#distribute
	iptables -A $STRICT_INPUT -s 169.254.0.0/255.255.0.0 -j RETURN
	iptables -A $STRICT_INPUT -d 169.254.0.0/255.255.0.0 -j RETURN

	iptables -A $STRICT_INPUT -p vrrp -j RETURN
	iptables -A $STRICT_INPUT -p icmp -j RETURN
	iptables -A $STRICT_INPUT -j DROP

	return 0
}

strict_input_disable()
{
	iptables -F $STRICT_INPUT
	iptables -A $STRICT_INPUT -j RETURN

	return 0;
}

strict_output_enable()
{
	iptables -F $STRICT_OUTPUT

	#Allow dhcp
	iptables -A $STRICT_OUTPUT -p udp --sport 68 --dport 67 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --sport 67 --dport 68 -j RETURN

	#ssh telnet
	iptables -A $STRICT_OUTPUT -p tcp --sport 22:23 -j RETURN
	iptables -A $STRICT_OUTPUT -p tcp --dport 22:23 -j RETURN

	#asd wid wsm
	iptables -A $STRICT_OUTPUT -p udp --sport 5246:5247 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 5246:5247 -j RETURN

	iptables -A $STRICT_OUTPUT -p udp --sport 19528:19544 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 19528:19544 -j RETURN

	iptables -A $STRICT_OUTPUT -p udp --sport 29527:29543 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 29527:29543 -j RETURN

	#bsd
	iptables -A $STRICT_OUTPUT -p udp --sport 44443 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 44443 -j RETURN
	
	# asd wsm-unknown
	iptables -A $STRICT_OUTPUT -p udp --sport 44642 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 44642 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --sport 8888:8890 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 8888:8890 -j RETURN
	
	#ntpd
	iptables -A $STRICT_OUTPUT -p udp --sport 123 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 123 -j RETURN
	
	# radius
	iptables -A $STRICT_OUTPUT -p udp --sport 1812:1813 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 1812:1813 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --sport 1645:1646 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 1645:1646 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --sport 3645:3646 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --dport 3645:3646 -j RETURN
	iptables -A $STRICT_OUTPUT -p udp --sport 3799 -j RETURN
	
	#portal
	iptables -A $STRICT_OUTPUT -p udp --sport 2000 -j RETURN

	#eag pdc rdc backup
	iptables -A $STRICT_OUTPUT -p tcp --sport 2002:2004 -j RETURN 
	iptables -A $STRICT_OUTPUT -p tcp --dport 2002:2004 -j RETURN

	#distribute
	iptables -A $STRICT_OUTPUT -s 169.254.0.0/255.255.0.0 -j RETURN
	iptables -A $STRICT_OUTPUT -d 169.254.0.0/255.255.0.0 -j RETURN

	iptables -A $STRICT_OUTPUT -p vrrp -j RETURN
	iptables -A $STRICT_OUTPUT -p icmp -j RETURN
	iptables -A $STRICT_OUTPUT -j DROP

	return 0
}

strict_output_disable()
{
	iptables -F $STRICT_OUTPUT
	iptables -A $STRICT_OUTPUT -j RETURN

	return 0;
}

strict_forward_enable()
{
	iptables -F $STRICT_FORWARD
	
	#Allow icmp
	iptables -A $STRICT_FORWARD -p icmp -j RETURN

	#Allow tcp status
	iptables -A $STRICT_FORWARD -p tcp -m state --state ESTABLISHED,RELATED -j RETURN

	#Set mss since iptables will cause pmtu doesn't work
	#iptables -A $STRICT_FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1300

	#Allow dhcp
	iptables -A $STRICT_FORWARD -p udp --sport 68 --dport 67 -j RETURN
	iptables -A $STRICT_FORWARD -p udp --sport 67 --dport 68 -j RETURN

	#Allow dns
	iptables -A $STRICT_FORWARD -p udp --dport 53 -j RETURN
	iptables -A $STRICT_FORWARD -p udp --sport 53 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 53 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 53 -j RETURN

	#Allow http or socks proxy
	iptables -A $STRICT_FORWARD -p tcp --dport 80 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 443 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 8080 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 1080 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 7080 -j RETURN
	
	iptables -A $STRICT_FORWARD -p tcp --sport 80 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 443 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 8080 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 1080 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 7080 -j RETURN
	
	#Allow telnet
	iptables -A $STRICT_FORWARD -p tcp --dport 23 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 23 -j RETURN
	
	#Allow ssh
	iptables -A $STRICT_FORWARD -p tcp --dport 22 -j RETURN
  	iptables -A $STRICT_FORWARD -p tcp --sport 22 -j RETURN
  	
	#Allow msn
	iptables -A $STRICT_FORWARD -p tcp --dport 1863 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 1863 -j RETURN
	
	#Allow smtp
	iptables -A $STRICT_FORWARD -p tcp --dport 25 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 25 -j RETURN
	
	#Allow pop3
	iptables -A $STRICT_FORWARD -p tcp --dport 110 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 110 -j RETURN

	#Allow imap4
	iptables -A $STRICT_FORWARD -p tcp --dport 143 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 143 -j RETURN
	
	#Allow qq
	iptables -A $STRICT_FORWARD -p tcp --dport 8000 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 8000 -j RETURN
	
	#Allow ftp
	modprobe ip_conntrack_ftp
	iptables -A $STRICT_FORWARD -p tcp --dport 20 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --dport 21 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 20 -j RETURN
	iptables -A $STRICT_FORWARD -p tcp --sport 21 -j RETURN

	#Deny all
	iptables -A $STRICT_FORWARD -j DROP

	return 0
}

strict_forward_disable()
{
	iptables -F $STRICT_FORWARD
	iptables -A $STRICT_FORWARD -j RETURN

	return 0
}

case "$1" in
start)
    strict_input_enable
	strict_output_enable
	strict_forward_enable
;;
stop)
	strict_input_disable
	strict_output_disable
	strict_forward_disable
;;
*)
	echo "Usage: $0 {start|stop}"
	exit 1
;;
esac

exit 0

