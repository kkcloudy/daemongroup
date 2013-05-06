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
# rip_distribute_list.sh ON|OFF LISTNAME IN|OUT [IFNAME]
#    
#############################################################################

source vtysh_start.sh

if [ $# -eq 3 ] ; then 
	ifnamestr=" "
elif [ $# -eq 4 ] ; then 
	ifnamestr="$4"	
else
	exit -2
fi	

	cmdstr="
	configure terminal
	router rip "
if [ $1 = "on" ] ; then
	cmdstr="
	$cmdstr
	distribute-list  $2 $3 $ifnamestr
	"
else
	cmdstr="
	$cmdstr
	no distribute-list  $2 $3 $ifnamestr
	"

fi
#echo $cmdstr
vtysh -c "$cmdstr"
if [ $? -eq 0 ] ; then
exit 0
else
exit -1
fi


