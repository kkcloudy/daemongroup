###########################################################################
#
#              Copyright (C) Autelan Technology
#
#This software file is owned and distributed by Autelan Technology 
#
############################################################################
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################
#
# eag_init
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#    
#############################################################################

BEGIN{
	FS="/"
	RS="\n"
	i=0
	j=0
	
	print "#!/bin/bash"
	print ""
	print "export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/bin"
	print ""
	print " count=`ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\"|wc -l`"
	print " interface=`ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\"` "
	print "i=1"
	print "#while [ $i -le $count ]"
	print "for tmp in $interface"
	print "do"
	print "#       tmp=`echo $interface|awk -v flag=\"$i\" '{print $flag }'`"
	print "       tc qdisc del dev $tmp root"
	print "       let i=i+1"
	print "done"
	print "iptables -t mangle -N TRAFFIC_CONTROL"
	print "iptables -t mangle -F TRAFFIC_CONTROL"
	print "iptables -t mangle -D PREROUTING -j TRAFFIC_CONTROL"
	print "iptables -t mangle -I PREROUTING -j TRAFFIC_CONTROL"
	print "iptables -N TRAFFIC_CONTROL"
	print "iptables -F TRAFFIC_CONTROL"	
	
}
{
#//输出规则， interface，up_down_flag, ipaddr， mask， bandwidth， p2pstate, begintime,endtime
	rule_if=$1
	rule_up_down_flag=$3
	rule_ip=$4
	rule_mask=$5
	rule_bandwidth=$6
	rule_p2pstate=$7
	rule_share_state=$10
	
	if( rule_up_down_flag == "src" )
		iptabls_up_down_flage = "s"
	else
		iptabls_up_down_flage = "d"

	print ""
	print "#rule begin"
	print "#"$0
	if( tmp != rule_if )
	{
		#如果当前的if上还没有根，就添加一个根class
		tmp = rule_if
		print "tc qdisc add dev "rule_if" root handle 1: htb default 10" 
		print "tc class add dev "rule_if" parent 1:0 classid 1:1 htb rate 10Mbit ceil 10Mbit burst 15k prio 1"
	}
	
	#添加class
	print "#rule_bandwidth = "rule_bandwidth
	if( rule_bandwidth != "0" )
	{
		print "#"rule_share_state
		if( rule_share_state != "notshare" )
		{
			++i
			print "tc class add dev "rule_if" parent 1:1 classid 1:1"i" htb rate "rule_bandwidth"Kbit ceil "rule_bandwidth"Kbit burst 15k prio 1"
		#添加filter
		#非p2p
			if( rule_p2pstate != "1" )
			{
				print "tc filter add dev "rule_if"  parent 1:0 protocol ip prio "i" u32 match ip "rule_up_down_flag" "rule_ip"/"rule_mask" flowid 1:1"i
			}
			else
			{
				#标记p2p
				print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --ipp2p -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j CONNMARK --set-mark "i
				print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --xunlei -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j CONNMARK --set-mark "i
				print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --mute -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j CONNMARK --set-mark "i
				print "tc filter add dev "rule_if" parent 1:0 protocol ip prio "i" handle "i" flowid 1:1"i
			}
			print "tc qdisc add dev "rule_if" parent 1:1"i" handle 1"i": sfq perturb 10"
		}
		else
		{
			#循环输出每个ip的流量控制。
			split(rule_ip,ipv,".")
			ip_int=ipv[1]*256*256*256+ipv[2]*256*256+ipv[3]*256+ipv[4]
			j=32-rule_mask
			mask_int_rev=0
			while( j > 0 )
			{
				j--
				mask_int_rev*=2
				mask_int_rev+=1
			}
			mask_int=256*256*256*256-1-mask_int_rev
			ip_begin=and(ip_int,mask_int)
			ip_end=or(ip_int,mask_int_rev)
			while( ip_begin < ip_end )
			{
				ip1=(ip_begin-ip_begin%(256*256*256))/256/256/256;
				ip2=(ip_begin-and(ip_begin,0xff000000));
				ip2=(ip2-ip2%(256*256))/256/256;
				ip3=(ip_begin-and(ip_begin,0xffff0000));
				ip3=(ip3-ip3%(256))/256;
				ip4=(ip_begin-and(ip_begin,0xffffff00));
				ip_begin++
				#输出class
				++i
				print "tc class add dev "rule_if" parent 1:1 classid 1:1"i" htb rate "rule_bandwidth"Kbit ceil "rule_bandwidth"Kbit burst 15k prio 1"
				#输出相应的filter
				if( rule_p2pstate != "1" )
				{
					print "tc filter add dev "rule_if"  parent 1:0 protocol ip prio "i" u32 match ip "rule_up_down_flag" "ip1"."ip2"."ip3"."ip4"/"32" flowid 1:1"i
				}
				else
				{
					#标记p2p
					print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --ipp2p -"iptabls_up_down_flage" "ip1"."ip2"."ip3"."ip4"/"32" -j CONNMARK --set-mark "i
					print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --xunlei -"iptabls_up_down_flage" "ip1"."ip2"."ip3"."ip4"/"32" -j CONNMARK --set-mark "i
					print "iptables -t mangle -A TRAFFIC_CONTROL -p tcp -m ipp2p --mute -"iptabls_up_down_flage" "ip1"."ip2"."ip3"."ip4"/"32" -j CONNMARK --set-mark "i
					print "tc filter add dev "rule_if" parent 1:0 protocol ip prio "i" handle "i" flowid 1:1"i
				}				
				print "tc qdisc add dev "rule_if" parent 1:1"i" handle 1"i": sfq perturb 10"
			}
		}
	}
	else
	{
		#添加iptables禁止链接
		if( rule_p2pstate != "1" )
		{

		}	
		else
		{
			#禁止p2p链接
			print "iptables -A TRAFFIC_CONTROL -p tcp -m ipp2p --ipp2p -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j DROP"
			print "iptables -A TRAFFIC_CONTROL -p tcp -m ipp2p --xunlei -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j DROP"
			print "iptables -A TRAFFIC_CONTROL -p tcp -m ipp2p --mute -"iptabls_up_down_flage" "rule_ip"/"rule_mask" -j DROP"	
		}
	}
	
	print "#rule end"
	
}
END{
	print "iptables -t mangle -A TRAFFIC_CONTROL -j RETURN"
	print "iptables -A TRAFFIC_CONTROL -j RETURN"
#	print "#如果没有 FW_FILTER链则创建FW_FILTER链,并将FW_FILTER链的规则设置好."    
	print "/usr/bin/firewall_init.sh check_chain"
#	print "iptables -L FW_FILTER"
#	print "if [ $? = "1" ];then"
#	print "     iptables -N FW_FILTER"
#	print "     iptables -A FORWARD -j FW_FILTER"
#	print "		iptables -A FW_FILTER -j TRAFFIC_CONTROL"
#	print "		iptables -A FW_FILTER -j ACCEPT"
#	print "else"
#	print "		iptables -D FW_FILTER -j TRAFFIC_CONTROL"
#	print "		iptables -D FW_FILTER -j ACCEPT"
#	print "		iptables -I FW_FILTER -j TRAFFIC_CONTROL"
#	print "		iptables -A FW_FILTER -j ACCEPT"
#	print "fi"
	print "echo \"start\" > /opt/services/status/traffic_status.status"
	print "echo \"start\" > /opt/services/status/iptables_status.status"
}

