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

CONF_FILE="/var/run/apache2/dhcp_head.conf"
TEMP_FILE="/var/run/apache2/dhcp_conf.tmp"
CONF_PATH=/opt/services/conf/dhcp_conf.conf
CONF_TAIL=/var/run/apache2/dhcp_tail.conf
make_sub_head(){
   echo "$1 $2"
   echo "  subnet $1 netmask $2 {"
}

make_sub_tail(){
   echo "    }"
}

#if [ $# -lt 4 ];then
#   exit 1
#fi

if [ ! -f $TEMP_FILE ];then
  touch $TEMP_FILE
  chmod 666 $TEMP_FILE
fi
if [ $1 -eq 1 ]
	then
		flag=$(grep -w -c "$2 $3" $CONF_FILE) 
		if [ $flag -gt 0 ]
			then
				exit 1;
		fi
fi



awk -v patt="$2 $3" 'BEGIN{RS="#@";FS="\n";ORS="#@";OFS="\n"}{ if($1 != patt) {print ;} }' $CONF_FILE > $TEMP_FILE 
#echo $4
if [ $4 -eq 4 ]
	then
		 sed -i '$d' $TEMP_FILE
		 mv $TEMP_FILE $CONF_FILE
     		 cat $CONF_FILE > $CONF_PATH
                 cat $CONF_TAIL >> $CONF_PATH
		 exit 0; 
fi		


make_sub_head $2 $3 >> $TEMP_FILE
echo $4 | awk 'BEGIN{RS="^";ORS="\n";}{print "      range "$1" "$2";"}'>>  $TEMP_FILE
echo -n $5 | awk 'BEGIN{RS="^";ORS=";\n";}{print "      "$0}' >>  $TEMP_FILE

make_sub_tail >>  $TEMP_FILE

mv $TEMP_FILE $CONF_FILE
cat $CONF_FILE > $CONF_PATH
cat $CONF_TAIL >> $CONF_PATH
