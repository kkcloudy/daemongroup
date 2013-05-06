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

gengroupconf()
{

	local groupname=$1
	local enabled=$2

	echo $groupname
	echo $enabled
	echo $group1
	enuserlist=$(awk -v FS=":" -v group1="$groupname" '$1==group1 { print $4 }' /etc/group)

	echo $group1
	echo $enuserlist

	enuserlist1=$(echo $enuserlist | sed 's/,/ /g')  
#	enuserlist1=$(echo $enuserlist | tr ',' ' ')
########################################
#	Use g option in sed substitution to replace multiple occurrences
#	tr has the same result if we just replace one char
########################################
#	echo $enuserlist1
	for user1 in $enuserlist1
	do
		passwd1=$(awk -v FS=":" -v user2="$user1" '$1==user2 { print $2 }' /etc/shadow)

		echo "user add $user1 passwd $passwd1"
		if [ $enabled = "1" ] ; then
			echo "user role $user1 admin"
		fi	
	done

}



if [ $# -eq 2 ] ; then
	echo $#
	echo $1
	echo $2
	gengroupconf $1 0
	gengroupconf $2 1
else
	echo "Usage: genuserconf.sh VIEWGROUPNAME ENGROOPNAME"
fi
