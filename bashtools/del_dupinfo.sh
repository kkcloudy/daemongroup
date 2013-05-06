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

source vtysh_start.sh

A_FILE=/var/run/apache2/dhcp_leasez.conf
B_FILE=/var/run/apache2/b.lease
C_FILE=/var/run/apache2/c.lease
D_FILE=/var/run/apache2/d.lease


if ! test -f $A_FILE; then
        exit 1
        else

                sed '/^$/d' $A_FILE > $B_FILE  #删除空行
                D_NUM=`cat $B_FILE |wc -l` #计算行数

                while [ $D_NUM != "0" ]
                do
                D_STR=`cat $B_FILE |awk '{print $1}' |head -n 1`
                cat $B_FILE |grep $D_STR|tail -n 1 >> $D_FILE
                sed "/$D_STR/d" $B_FILE > $C_FILE
                cat $C_FILE > $B_FILE
                D_NUM=`cat $B_FILE |wc -l`
                done

                if ! test -f $D_FILE; then
                        exit 0
                else
                        cat $D_FILE > $A_FILE
                        rm $D_FILE
                        exit 2
                fi
        exit 0
fi
