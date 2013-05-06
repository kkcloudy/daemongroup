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

ADMINGROUP="vtyadmin"
VIEWGROUP="vtyview"

if [ $# -eq 2 ] ; then
	user=$1
	if [ "enable" = $2 ] ; then
		usergroup=$ADMINGROUP
		delgroup=$VIEWGROUP
	else
		usergroup=$VIEWGROUP
		delgroup=$ADMINGROUP
	fi
# Sometimes the latest changes in /etc/group could not be showed in the result of id command, so we use cat /etc/group directly.	
#	id $user -a | grep $usergroup
#	cat /etc/group | grep $usergroup | grep $user >> /dev/null
#	if [ 0 -eq $? ] ; then
#		echo "User $user already belong to group $usergroup"
#		exit -3
#	else
#		id $user -a | grep $delgroup
#		cat /etc/group | grep $delgroup | grep $user >> /dev/null
#		if [ 0 -eq $? ] ; then
			usermod -G $usergroup $user
			if [ 0 -eq $? ] ; then
				exit 0
			else
				exit -1
			fi
#		else
#			exit -2
#		fi
#	fi
else
#	echo "Usage: userrole.sh USERNAME [view|enable]"
	exit -2
fi
