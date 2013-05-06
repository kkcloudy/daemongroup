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

Dget_path=/opt/awk/dhcp_read.awk
get_head=/opt/awk/dhcp_head.awk
get_tail=/opt/awk/dhcp_tail.awk
get_nr=/opt/awk/dhcp_nr.awk
DHCP_PATH=/opt/services/conf/dhcp_conf.conf
DHCP_file=/var/run/apache2/dhcp_head.conf
DHCP_file_t=/var/run/apache2/dhcp_tail.conf
DHCP_NR_TMP=/var/run/apache2/dhcp_nr.tmp
#DHCP_FILE=/opt/services/conf/dhcp_conf.conf
DHCP_OPTION=/opt/services/option/dhcp_option
DHCP_STATUS=/opt/services/status/dhcp_status.status
IP_FILE=/var/run/apache2/ip_addr.file
IP_ADDR=/opt/awk/ip_addr.awk
if [ ! -f $IP_FILE ]
         then
                touch $IP_FILE
                chmod 755 $IP_FILE
 fi
 ip addr|awk -f $IP_ADDR >$IP_FILE
 
 if [ ! -f $DHCP_PATH ]
   then
              touch $DHCP_PATH
              chmod 666 $DHCP_PATH
	      echo "ddns-update-style none;"> $DHCP_PATH
              echo "##">>$DHCP_PATH
 fi
 if [ ! -f $DHCP_OPTION ]
         then
             touch $DHCP_OPTION
             chmod 777 $DHCP_OPTION
             echo "INTERFACES=\" \"" >$DHCP_OPTION
 fi
 if [ ! -f $DHCP_STATUS ]
         then
          touch $DHCP_STATUS
          chmod 666 $DHCP_STATUS
         echo "stop" >$DHCP_STATUS
 fi

if [ ! -f $Tem_path ];then
   touch $Tem_path
   chmod 666 $Tem_path
fi
if [ ! -f $DHCP_file ] ;then 
   touch $DHCP_file
   chmod 666 $DHCP_file


fi
  #awk -f  $get_tmp $DHCP_PATH >$DHCP_file
  awk -f $get_head $DHCP_PATH >$DHCP_file
  awk -f $get_tail $DHCP_PATH >$DHCP_file_t
  awk -f $get_nr $DHCP_file > $DHCP_NR_TMP
