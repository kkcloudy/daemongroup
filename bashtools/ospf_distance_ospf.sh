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
#ospf_distance_ospf.sh ON|OFF  [intra-area <1-255>] [inter-area <1-255>] [external <1-255>]	
#    
#############################################################################


source vtysh_start.sh
cmdstr="configure terminal
	router ospf"

if [ $# -eq 1 ] ; then
        	if [ $1 = "off" ] ; then
                 	cmdstr="$cmdstr
                        	no distance ospf"
        	else           
             		exit -3
        	fi 
elif [ $# -le 7 ] ; then
       	if [ $1 = "on" ] ; then
		cmdstr="$cmdstr
			distance ospf "
		i=2
		for i in 2 4 6 ;
		do
			eval str=\$$((i))
			if [ -z $str ] ; then
				break
			fi
			if [ $str = "intra-area" ] ; then
				eval diststr=\$$((i+1))
				 cmdstr="$cmdstr  $str $diststr"
			elif [ $str = "inter-area" ] ; then
				eval diststr=\$$((i+1))
				 cmdstr="$cmdstr  $str $diststr"
			elif [ $str = "external" ] ; then
 				eval diststr=\$$((i+1))
				 cmdstr="$cmdstr  $str $diststr"
			else
				continue
			fi
		done
	else
		exit -3
	fi
else
	exit -2
fi
echo $cmdstr
vtysh -c "$cmdstr"	
	if [ $? -eq 0 ] ; then		
		exit 0	
	else		
		exit -1	
	fi
