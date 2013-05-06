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
help()
{
	if [ $# != 0 -a "$1" == "error" ]
	then
		echo "Incorrect parameters!"
	else 
		echo "run $0 as follows:"	
	fi
	echo "Usage: $0 $OPTIONS <ID> <A.B.C.D>"
	echo "Examples: "
	echo "$0 -ht 1 192.168.1.1"
	exit 1
}

#
# Function print user message header part
#
print_header()
{
	printf "Codes: u - errors due to 'Destination Host Unreachable'\n"
	printf "       f - errors due to 'Frag needed and DF set'\n"
	printf "       o - no respondse but with no error\n"
	printf "       - - skip this process due to previous error\n\n"
	printf "%-1s%-6s%-1s%-15s%-1s%-59s%-1s\n" \
		"+" "------" "+" "---------------" "+" "-----------------------------------------------------------" "+"
	printf "%-1s%-6s%-1s%-15s%-1s%-59s%-1s\n" \
		"|" "" "|" "" "|" "                per packet time elapse (ms)" "|"
	printf "%-1s%-6s%-1s%-15s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s\n" \
		"|" "  ID" "|" "   target ip   " "+" "-----------" "+"  "-----------" "+"  \
		"-----------" "+" "-----------" "+" "-----------" "+" 
	printf "%-1s%-6s%-1s%-15s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s\n" \
		"|" "" "|" "" "|" "    64B" "|" "   512B" "|" "   1518B" "|" "   1522B" "|" "   5000B" "|"
	printf "%-1s%-6s%-1s%-15s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s\n" \
		"+" "------" "+" "---------------" "+" "-----------" "+"  "-----------" "-" \
		"-----------" "+"  "-----------" "+" "-----------" "+"
}

#
# Function print user message data part
# Parameter list: target_id target_ip 64B_time 512B_time 1518B_time 1522B_time 5000B_time
#
print_data()
{
	if [ $# != 7 ]
	then
		print "null"
		return
	fi

	# build up input parameters
	TARGET_ID=$1
	TARGET_IP=$2
	T_64B=$3
	T_512B=$4
	T_1518B=$5
	T_1522B=$6
	T_5000B=$7

	printf "%-1s%-6s%-1s%-15s%-1s %-10s%-1s %-10s%-1s %-10s%-1s %-10s%-1s %-10s%-1s\n" \
		"|" $TARGET_ID "|" $TARGET_IP "|" $T_64B "|" $T_512B "|" $T_1518B "|" $T_1522B "|" $T_5000B "|"
	return
}

#
# Function print user message tail part
#
print_tail()
{
	printf "%-1s%-6s%-1s%-15s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s%-11s%-1s\n" \
		"+" "------" "+" "---------------" "+" "-----------" "+"  "-----------" "+" \
		"-----------" "+"  "-----------" "+" "-----------" "+"
}

# temp result file
DATE_VAL=`date +%Y%m%d%H%M%S%N`
PARSE_TARGET="ping.result"$DATE_VAL
#echo $PARSE_TARGET

# packet size = 14B(ethernet header) + 20B(ip header) + 8B(icmp header) + NB(payload)
PING64="ping -c 1 -s 22 -M do -n" 
PING512="ping -c 1 -s 470 -M do -n"
PING1518="ping -c 1 -s 1476 -M do -n"
PING1522="ping -c 1 -s 1480 -M do -n"
PING5000="ping -c 1 -s 4958 -n"
OPTIONS="HhTtIi"
OPT_FMT="-T -I"

# arguments:[SCRIPTS.sh OPTIONS TARGET_ID TARGET_IP] 
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
	  echo "Unsupported option $1"
	  exit 1
	;;
	esac
#done

#echo $0 $1 $2 $3

# normal help message 
if [ ! -n "$1"  -a "$1" == "--help" ]
then
	help clean
fi

if [ "$PRINT_HEAD" == "1" ]
then
	print_header
fi

# parse target_ip field 
target_ip=`echo $3 | awk -F":" '{print $1}'`

# 64B packet ping test
$PING64 $target_ip > $PARSE_TARGET
TIME_STAT=`cat $PARSE_TARGET | grep "rtt"`
DF_STAT=`cat $PARSE_TARGET | grep "DF"`
UNREACH_STAT=`cat $PARSE_TARGET | grep "Unreachable"`
if [ "$TIME_STAT" != "" ] 
then
	TIME_LINE=`echo $TIME_STAT | awk '{print $4}'`
	t_64B=`echo $TIME_LINE | awk -F"/" '{print $1}'`
	next_skip=0
elif [ "$DF_STAT" != "" ]
then
	t_64B="  f"
	next_skip=0
elif [ "$UNREACH_STAT" != "" ]
then
	t_64B="  u"
	next_skip=1
else
	t_64B="  o"
	next_skip=1
fi
# echo $t_64B

# 512B packet ping test
if [ "1" != "$next_skip" ]
then
$PING512 $target_ip > $PARSE_TARGET
	TIME_STAT=`cat $PARSE_TARGET | grep "rtt"`
	DF_STAT=`cat $PARSE_TARGET | grep "DF"`
	UNREACH_STAT=`cat $PARSE_TARGET | grep "Unreachable"`
	if [ "$TIME_STAT" != "" ] 
	then
		TIME_LINE=`echo $TIME_STAT | awk '{print $4}'`
		t_512B=`echo $TIME_LINE | awk -F"/" '{print $1}'`
		next_skip=0
	elif [ "$DF_STAT" != "" ]
	then
		t_512B="  f"
		next_skip=0
	elif [ "$UNREACH_STAT" != "" ]
	then
		t_512B="  u"
		next_skip=1
	else
		t_512B="  o"
		next_skip=1
	fi
else
	t_512B="  -"
fi
# echo $t_512B

# 1518B packet ping test
if [ "1" != "$next_skip" ]
then
$PING1518 $target_ip > $PARSE_TARGET
	TIME_STAT=`cat $PARSE_TARGET | grep "rtt"`
	DF_STAT=`cat $PARSE_TARGET | grep "DF"`
	UNREACH_STAT=`cat $PARSE_TARGET | grep "Unreachable"`
	if [ "$TIME_STAT" != "" ] 
	then
		TIME_LINE=`echo $TIME_STAT | awk '{print $4}'`
		t_1518B=`echo $TIME_LINE | awk -F"/" '{print $1}'`
		next_skip=0
	elif [ "$DF_STAT" != "" ]
	then
		t_1518B="  f"
		next_skip=0
	elif [ "$UNREACH_STAT" != "" ]
	then
		t_1518B="  u"
		next_skip=1
	else
		t_1518B="  o"
		next_skip=1
	fi
else
	t_1518B="  -"
fi

# echo $t_1518B

# 1522B packet ping test
if [ "1" != "$next_skip" ]
then
$PING1522 $target_ip > $PARSE_TARGET
	TIME_STAT=`cat $PARSE_TARGET | grep "rtt"`
	DF_STAT=`cat $PARSE_TARGET | grep "DF"`
	UNREACH_STAT=`cat $PARSE_TARGET | grep "Unreachable"`
	if [ "$TIME_STAT" != "" ] 
	then
		TIME_LINE=`echo $TIME_STAT | awk '{print $4}'`
		t_1522B=`echo $TIME_LINE | awk -F"/" '{print $1}'`
		next_skip=0
	elif [ "$DF_STAT" != "" ]
	then
		t_1522B="  f"
		next_skip=0
	elif [ "$UNREACH_STAT" != "" ]
	then
		t_1522B="  u"
		next_skip=1
	else
		t_1522B="  o"
		next_skip=1
	fi
else
	t_1522B="  -"
fi

# echo $t_1522B

# 5000B packet ping test
if [ "1" != "$next_skip" ]
then
$PING5000 $target_ip > $PARSE_TARGET
	TIME_STAT=`cat $PARSE_TARGET | grep "rtt"`
	DF_STAT=`cat $PARSE_TARGET | grep "DF"`
	UNREACH_STAT=`cat $PARSE_TARGET | grep "Unreachable"`
	if [ "$TIME_STAT" != "" ] 
	then
		TIME_LINE=`echo $TIME_STAT | awk '{print $4}'`
		t_5000B=`echo $TIME_LINE | awk -F"/" '{print $1}'`
		next_skip=0
	elif [ "$DF_STAT" != "" ]
	then
		t_5000B="  f"
		next_skip=0
	elif [ "$UNREACH_STAT" != "" ]
	then
		t_5000B="  u"
		next_skip=1
	else
		t_5000B="  o"
		next_skip=1
	fi
else
	t_5000B="  -"
fi

# echo $t_5000B

rm -rf $PARSE_TARGET

# echo $2 $3 $t_64B $t_512B $t_1518B $t_1522B $t_5000B
print_data $2 $target_ip $t_64B $t_512B $t_1518B $t_1522B $t_5000B

if [ "$PRINT_TAIL" == "1" ]
then
	print_tail
fi
