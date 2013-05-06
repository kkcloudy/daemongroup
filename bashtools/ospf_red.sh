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
#ospf_red.sh ON|OFF TYPE METRIC|-1 MTYPE|-1 ROUTEMAP|NON
#    
#############################################################################


source vtysh_start.sh
cmdstr="configure terminal
		router ospf"
if [ $# -eq 5 ] ; then
{
	if [ $1 = "on" ] ; then
		cmdstr="$cmdstr
			redistribute $2"
	else 
		cmdstr="$cmdstr
			no redistribute $2"
	fi
	if [ $3 -ne -1 -a $1 = "on" ] ; then
		cmdstr="$cmdstr metric $3"
	fi
	if [ $4 -ne -1 -a $1 = "on" ] ; then
		cmdstr="$cmdstr metric-type $4"
	fi
	if [ $5 != "non" -a $1 = "on" ] ; then
		cmdstr="cmdstr route-map $5"
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
