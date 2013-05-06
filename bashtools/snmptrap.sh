#!/bin/sh
#
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
# autelan.software.tangsiqi. team
# 
# DESCRIPTION: 
#     
#############################################################################

source vtysh_start.sh
CONF_FILE="/opt/services/conf/snmpd_conf.conf"
SNMP_CMD="/usr/bin/snmptrap"

if [ "$1" == "v1" ];then
	v1trapdesc=`echo $@ | cut -d ' ' -f4- | sed "s/$1//g"`
	echo "/usr/bin/snmptrap -v 1 -c $2 $3 \"\" 6 17 \"\" $v1trapdesc" | /bin/bash
elif [ "$1" == "v2c" ];then
	v2ctrapdesc=`echo $@ | cut -d ' ' -f4-`
	echo "/usr/bin/snmptrap -v 2c -c $2 $3 \"\" $v2ctrapdesc" | /bin/bash
elif [ "$1" == "v3" ];then
	cat $CONF_FILE | grep "^createUser" | while read line
	do
		array=($line)
		i=0
		v3_domain=""
		while [ $i -lt ${#array[@]} ]
		do
			case ${array[$i]} in
				"createUser")
					let 'i = i + 1'
					v3_domain=$v3_domain" -u ${array[$i]}"
				;;
				"MD5"|"SHA")
					v3_domain=$v3_domain" -a ${array[$i]}"
					let 'i = i + 1'
					v3_domain=$v3_domain" -A ${array[$i]}"
				;;
				"DES"|"AES")
					v3_domain=$v3_domain" -x ${array[$i]}"
					let 'i = i + 1'
					v3_domain=$v3_domain" -X ${array[$i]}"
					;;
				*)
					echo $i
					echo ${array[$i]}	
					exit 1
				;;
			esac
			let 'i = i + 1'
		done

		#下面产生了一个大致的系统起来的时间。不是很准确，因为uptime命令只能显示到分钟。
		timetick=$(uptime | awk '{print $3}' | sed "s/,/:$RANDOM/g" | awk -v FS=":" '{print $1*60*60*100+$2*60*100+$3%300}')
		#snmptrap -v 3 -u myuser -a MD5 -A mypassword -l authNoPriv 192.168.9.114 42 coldStart.0
		v3trapdesc=`echo $@ | cut -d ' ' -f3-`
		echo "/usr/bin/snmptrap -v 3 $v3_domain $2 $timetick $v3trapdesc" | /bin/bash
	done
fi	
