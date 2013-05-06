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
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################

#
# Function that provides help message for the script
#
source vtysh_start.sh
help()
{
	if [ $# != 0 -a "$1" == "error" ]
	then
		echo "Incorrect parameters!"
	else 
		echo "run $0 as follows:"	
	fi
	echo "Usage: $0 $OPTIONS <A.B.C.D>"
	echo "  Supported options are:"
	echo "  1.> t - for tcp traffic test"
	echo "  2.> u - for udp traffic test"
	echo "  3.> b - for both tcp and udp traffic tests"
	echo ""
	echo "Examples: "
	echo "$0 -b 192.168.1.1"
	exit 1
}

#
# Function print user message header part
#
print_header()
{
	# check if input argument correct: either null or 'b' or 't' or 'u'
	if [ $# != 0 -a $1 != 'b' -a $1 != 't' -a $1 != 'u' ]
	then
		return 0
	fi
	
	#echo $# $1
	if [ $# == 0 ] 
	then
		flag='b'  #default for both tcp and udp test result
	else
		flag=$1
	fi
	
	printf "Codes: e - traffic trace failed due to 'No route to host'\n"
	printf "       x - traffic trace failed due to 'Connection refused' which means\n"
	printf "           no server end started\n"
	printf "       - - skip this process due to previous error\n\n"

	if [ $flag == 'b' ]
	then
		printf "%-1s%-15s%-1s%-59s%-1s\n" \
			 "+" "---------------" "+" "-----------------------------------------------------------" "+"
		printf "%-1s%-15s%-1s%-29s%-1s%-29s%-1s\n" \
			 "|" "" "|" "             TCP" "|" "             UDP" "|"
		printf "%-1s%-15s%-1s%-29s%-1s%-29s%-1s\n" \
			 "|" "   target ip" "|" "-----------------------------" "+" "-----------------------------" "|"
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s%-14s%-1s%-14s%-1s\n" \
			 "|" "" "|" "   Vol(MB)" "|" " Speed(Mbps)" "|" "   Vol(MB)" "|" " Speed(Mbps)" "|"
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" "--------------" "+" "--------------" "+"
	elif [ $flag == 't' ]
	then
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "+" "---------------" "+" "-----------------------------" "+"
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "|" "" "|" "             TCP" "|"
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "|" "   target ip" "|" "-----------------------------" "|"
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			 "|" "" "|" "   Vol(MB)" "|" " Speed(Mbps)" "|" 
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" 
	else 
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "+" "---------------" "+" "-----------------------------" "+"
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "|" "" "|" "             UDP" "|"
		printf "%-1s%-15s%-1s%-29s%-1s\n" \
			 "|" "   target ip" "|" "-----------------------------" "|"
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			 "|" "" "|" "   Vol(MB)" "|" " Speed(Mbps)" "|" 
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" 
	fi
}

#
# Function print user message data part
# Parameter list: target_ip volume speed
#
print_one_stream_data()
{
	if [ $# != 3 ]
	then
		print "null"
		return
	fi

	# build up input parameters
	TARGET_IP=$1
	VOL=$2
	SPEED=$3

	printf "%-1s%-15s%-1s %-11s%-1s %-11s%-1s\n" \
		"|" $TARGET_IP "|" $VOL "|" $SPEED "|" 
	
	return 0
}

#
# Function print user message data part
# Parameter list: target_ip volumeT speedT volumeU speedU
#
print_two_stream_data()
{
	if [ $# != 5 ]
	then
		printf "null\n"
		return
	fi

	# build up input parameters
	TARGET_IP=$1
	VOLT=$2
	SPEEDT=$3
	VOLU=$4
	SPEEDU=$5

	printf "%-1s%-15s%-1s %-13s%-1s %-13s%-1s %-13s%-1s %-13s%-1s\n" \
		"|" $TARGET_IP "|" $VOLT "|" $SPEEDT "|" $VOLU "|" $SPEEDU "|"
	
	return 0
}

#
# Function print user message tail part
#
print_tail()
{
	# check if input argument correct: either null or 'b' or 't' or 'u'
	if [ $# != 0 -a $1 != 'b' -a $1 != 't' -a $1 != 'u' ]
	then
		return 0
	fi
	
	#echo $# $1
	if [ $# == 0 ] 
	then
		flag='b'  #default for both tcp and udp test result
	else
		flag=$1
	fi

	if [ $flag == 'b' ]
	then
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" "--------------" "+" "--------------" "+"
	elif [ $flag == 't' ]
	then
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" 
	else 
		printf "%-1s%-15s%-1s%-14s%-1s%-14s%-1s\n" \
			"+" "---------------" "+" "--------------" "+" "--------------" "+" 
	fi
}

# temp result file
DATE_VAL=`date +%Y%m%d%H%M%S%N`
TCP_PARSE_TARGET="iperf.tcp.result"$DATE_VAL
TCP_ERR="iperf.tcp.err"$DATE_VAL
UDP_PARSE_TARGET="iperf.udp.result"$DATE_VAL
UDP_ERR="iperf.udp.err"$DATE_VAL

# packet size = 14B(ethernet header) + 20B(ip header) + 8B(icmp header) + NB(payload)
IPERF_TCP="iperf -c " 
IPERF_UDP="iperf -u -c "
OPTIONS="HhTtIi"
OPT_FMT="-H -T -I"

# first clear previous garbages
TCP_GARBAGE=`ls -la | grep "iperf.tcp" | wc -l`
#echo $TCP_GARBAGE
if [ $TCP_GARBAGE != 0 ]
then
	rm -rf iperf.tcp.*
fi
UDP_GARBAGE=`ls -la | grep "iperf.udp" | wc -l`
if [ $UDP_GARBAGE != 0 ]
then
	rm -rf iperf.udp.*
fi

# arguments:[SCRIPTS.sh OPTIONS TRAFFIC TARGET_IP] 
if [ $# != 3 ] 
then 
	help error
fi

# get options
#while getopts $OPTIONS opt
#do
	case $1 in
	-H | -h )
	  #echo "option H or h"
	  PRINT_HEAD=1
	  PRINT_TAIL=0
	  ;;
	-T | -t )
	  #echo "option T or t"
	  PRINT_HEAD=0
	  PRINT_TAIL=1
	  ;;
	-I | -i )
	  #echo "option I or i"
	  # options may ignored
	  PRINT_HEAD=0
	  PRINT_TAIL=0
	  ;;
	-HT | -Ht | -hT | -ht)
	  PRINT_HEAD=1
	  PRINT_TAIL=1
	  ;;
	*)
	  echo "Unsupported control option $1"
	  exit 1
	;;
	esac
#done

# check packet type
case $2 in 
	'T' | 't')
	packet='t'
	;;
	'U' | 'u')
	packet='u'
	;;
	'B' | 'b')
	packet='b'
	;;
	*)
	packet='b'
	;;
esac

#echo $0 $1 $2 $3

# normal help message 
if [ ! -n "$1"  -a "$1" == "--help" ]
then
	help clean
fi

if [ "$PRINT_HEAD" == "1" ]
then
	print_header $packet
fi

# parse target_ip field 
target_ip=`echo $3 | awk -F":" '{print $1}'`

# iperf tcp traffic test
if [ $packet == 't' -o $packet == 'b' ]
then
	$IPERF_TCP $target_ip 2> $TCP_ERR > $TCP_PARSE_TARGET
	NORMAL_STAT=`cat $TCP_PARSE_TARGET | grep "Mbits/sec"`
	NO_ROUTE_STAT=`cat $TCP_ERR | grep "No route to host"`
	REFUSE_STAT=`cat $TCP_ERR | grep "Connection refused"`
#echo "normal:" $NORMAL_STAT
#echo "no route:"$NO__ROUTE__STAT
#echo "refuse:"$REFUSE_STAT
	if [ "$NO_ROUTE_STAT" != "" ]
	then
		VOLT_SEG="     e"
		SPEEDT_SEG="     e" 
		next_skip=1
	elif [ "$REFUSE_STAT" != "" ]
	then
		VOLT_SEG="     x"
		SPEEDT_SEG="     x" 
		next_skip=1
	elif [ "$NORMAL_STAT" != "" ] 
	then
		VOLT_SEG=`echo $NORMAL_STAT | awk -F" " '{print $5}'`
		SPEEDT_SEG=`echo $NORMAL_STAT | awk -F" " '{print $7}'`
		next_skip=0
	fi
fi
#echo $VOLT_SEG
#echo $SPEEDT_SEG

# iperf udp traffic test
if [ "$next_skip" == "1" ]
then
	VOLU_SEG="     -"
	SPEEDU_SEG="     -"
elif [ $packet == 'u' -o $packet == 'b' ]
then
	$IPERF_UDP $target_ip "-b 100M" 2>$UDP_ERR > $UDP_PARSE_TARGET
	NORMAL_STAT_U=`cat $UDP_PARSE_TARGET | grep "Mbits/sec"`
	NO_ROUTE_STAT=`cat $UDP_ERR | grep "No route to host"`
	REFUSE_STAT=`cat $UDP_ERR | grep "Connection refused"`
#echo "normal:"$NORMAL_STAT_U
#echo "no route:"$NO_ROUTE_STAT
#echo "refuse:"$REFUSE_STAT
	if [ "$NO_ROUTE_STAT" != "" ]
	then
		VOLU_SEG="     e"
		SPEEDU_SEG="     e" 
	elif [ "$REFUSE_STAT" != "" ]
	then
		VOLU_SEG="     x"
		SPEEDU_SEG="     x" 
	elif [ "$NORMAL_STAT_U" != "" ] 
	then
		VOLU_SEG=`echo $NORMAL_STAT_U | awk -F" " '{print $5}'`
		SPEEDU_SEG=`echo $NORMAL_STAT_U | awk -F" " '{print $7}'`
	fi
fi
#echo $VOLU_SEG
#echo $SPEEDU_SEG

if [ $packet == 'b' ]
then
	print_two_stream_data $target_ip $VOLT_SEG $SPEEDT_SEG $VOLU_SEG $SPEEDU_SEG
elif [ $packet == 't' ]
then
	print_one_stream_data $target_ip $VOLT_SEG $SPEEDT_SEG
else
	print_one_stream_data $target_ip $VOLU_SEG $SPEEDU_SEG
fi

rm -rf $TCP_PARSE_TARGET
rm -rf $UDP_PARSE_TARGET
rm -rf $TCP_ERR
rm -rf $UDP_ERR

if [ "$PRINT_TAIL" == "1" ]
then
	print_tail $packet
fi
