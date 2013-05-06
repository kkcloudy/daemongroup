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
#ospf_if_md5_key.sh IFNAME ON|OFF KEYID KEY|NON PREFIX|NON
#    
#############################################################################


source vtysh_start.sh
cmdstr="configure terminal"
if [ $# -eq 5 ] ; then
{
	if [ $2 = "on" ] ; then
		cmdstr="$cmdstr
			interface $1
				ip ospf message-digest-key $3 md5 $4" 
	else 
		cmdstr="$cmdstr
			interface $1
				no ip ospf message-digest-key $3"
	fi
	if [ $5 = "non" ] ; then
		echo
	else 	
		cmdstr="$cmdstr $5"
	fi
}	
else
	exit -3
fi

echo "$cmdstr"	
vtysh -c "$cmdstr"
if [ $? -eq 0 ] ; then 
	exit 0
else 
	exit -3
fi
