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

FLAG=$1
CLIENTIFACE=$2
PROXYIP=$3
PORT=$4
FWMARK=$5
PROXY_NUM=$6

if [ $1 == 1 ] ;then
	iptables -t mangle -A PREROUTING -i $CLIENTIFACE -p tcp --dport $PORT -j MARK --set-mark $FWMARK
	iptables -t mangle -A PREROUTING -m mark --mark $FWMARK -j ACCEPT
	echo "$PROXY_NUM   proxy$PROXY_NUM"
	echo "$PROXY_NUM   proxy$PROXY_NUM" >> /etc/iproute2/rt_tables
	echo "ip rule add fwmark $FWMARK table proxy$PROXY_NUM"
	ip rule add fwmark $FWMARK table proxy$PROXY_NUM
	echo " ip route add default via $PROXYIP table proxy$PROXY_NUM"
	ip route add default via $PROXYIP table proxy$PROXY_NUM
fi

if [ $1 == 2 ] ;then
	iptables -t mangle -D PREROUTING -i $CLIENTIFACE -p tcp --dport $PORT -j MARK --set-mark $FWMARK
	iptables -t mangle -D PREROUTING -m mark --mark $FWMARK -j ACCEPT
	echo "sed -i '/$PROXY_NUM/d' /etc/iproute2/rt_tables"
#	sed -i '/$PROXY_NUM/d' /etc/iproute2/rt_tables
	echo "ip rule del fwmark $FWMARK table proxy$PROXY_NUM"
	ip rule del fwmark $FWMARK table proxy$PROXY_NUM
	echo " ip route del default via $PROXYIP table proxy$PROXY_NUM"
	ip route del default via $PROXYIP table proxy$PROXY_NUM
	sed -i '/'$PROXY_NUM'/d' /etc/iproute2/rt_tables
fi






