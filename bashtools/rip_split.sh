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
# 
#
# CREATOR:
# autelan.software.team
# 
# DESCRIPTION: 
# rip_split.sh on|off IFNAME normal|poisoned
#    
#############################################################################
#
source vtysh_start.sh

if [ $# -eq 3 ] ; then
	if [ $1 = "on" ] ; then
		if [ $3 = "normal" ] ; then
			ver=" "
		else
			ver="poisoned-reverse"
		fi
		cmdstr="
		configure terminal
		interface $2
		ip rip split-horizon $ver
		"
	elif [ $1 = "off" ] ; then
		if [ $3 = "normal" ] ; then
			ver=" "
		else
			ver="poisoned-reverse"
		fi
		cmdstr="
		configure terminal
		interface $2
		no ip rip split-horizon $ver 
		"
	else
		exit -3
	fi
#	echo $cmdstr
	vtysh -c "$cmdstr"
	if [ $? -eq 0 ] ; then
	exit 0
	else
	exit -1
	fi
else
	exit -2
fi


