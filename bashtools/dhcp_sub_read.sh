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
# autelan.software.liutao. team
# 
# DESCRIPTION: 
#     
#############################################################################

CONF_FILE=/var/run/apache2/dhcp_head.conf
SUB_FILE="/var/run/apache2/dhcp_sub.tmp"
DHCP_get=/opt/awk/dhcp_read.awk
SUB_TMP=/var/run/apache2/dhcp.sub.r
PORT_NAME=""

#if [ $# -lt 1 ];then
#   exit 1
#else
#   PORT_NAME="$1" 
#fi
   
if [ ! -f $CONF_FILE ];then
   touch $CONF_FILE;
   chmod 666 $CONF_FILE;
   echo "#begin" > $CONF_FILE;
   echo "##start" >> $CONF_FILE;
fi

if [ ! -f $SUB_FILE ];then
   touch $SUB_FILE
   chmod 644 $SUB_FILE
fi

cat $CONF_FILE|awk -v patt="$1 $2" 'BEGIN{RS="#@";FS="\n"}{if($1 == patt) print}'|awk 'BEGIN{FS=" ";RS="\n"}{if($1~/range/) print;else if($1~/option/) print;else if($1~/default-lease-time/)print;else if($1~/max-lease-time/)print;}'|sed 's/;//g'|sed 's/^[ \t]*//'|sed 's/"//g' > $SUB_FILE

#cat $CONF_FILE | awk -v patt="$1 $2" 'BEGIN{RS="#@";FS="{\n|}\n";n=1;OFS="/"}($1 == patt){while($n !~ /subnet/ && n<=NF){ n++;}n++;if(n != NF) print $n;}'|sed 's/;//g'|sed 's/^[ \t]*//'|sed 's/"//g' > $SUB_FILE 
#awk -f $DHCP_get $SUB_FILE >$SUB_TMP

#sed 's/;/ /g' $SUB_FILE|sed -n '/range/p'|sed // 
