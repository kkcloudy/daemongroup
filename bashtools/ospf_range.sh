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
#ospf_nssa.sh AREA ON|OFF A.B.C.D [ADVERTISE|NOT-ADVERTISE] [COST] [SUBTITUTE A.B.C.D]
#    
#############################################################################


source vtysh_start.sh
cmdstr="configure terminal
	router ospf"
if [ $# -eq 3 ] ; then
	if [ $2 = "on" ] ; then
		cmdstr="$cmdstr
			area $1 range $3"
	elif [ $2 = "off" ] ; then
		cmdstr="$cmdstr 
			no area $1 range $3"
	else
		exit -1
	fi
elif [ $# -eq  4 ] ; then
 		if [ $2 = "on" ] ; then         
               		if [ $4 = "advertise" ] ; then
	                       cmdstr="$cmdstr
                                      area $1 range $3 advertise"
			elif [ $4 = "not-advertise" ] ; then
                   		cmdstr="$cmdstr
					area $1 range $3 not-advertise"
			else
				cmdstr="$cmdstr
					area $1 $3 cost $4"
			fi
		elif [ $2 = "off" ] ; then
                      	if [ $4 = "advertise" ] ; then
                               cmdstr="$cmdstr
                                     no area $1 range $3 dvertise"
                	elif [ $4 = "not-advertise" ] ; then
                               cmdstr="$cmdstr
                               		no area $1 range $3 not-advertise"
             	else
                        	cmdstr="$cmdstr
                               		no area $1 range $3 cost $4"
                	fi
    
		else
			exit -1
		fi	
elif [ $# -eq 5 ] ; then
	if [ $4 = "advertise" ] ; then
		if [ $2 = "on" ] ; then
			cmdstr="$cmdstr
				area $1 range $3 advertise cost $5"
		elif [ $2 = "off ] ; then
			cmdstr="$cmdstr
				no area $1 range $3 advertise cost $5"
		else
			exit -1
		fi
	elif [ $4 = "substitute" ] ; then
		if [ $2 = "on" ] ; then
			cmdstr="$cmdstr
				area $1 range $3 substitute $5"

		elif [ $2 = "off ] ; then
			cmdstr="$cmdstr
				no area $1 range $3 substitute $5"

		else
			exit -1
		fi
	
	else
		exit -1
	fi

elif [ $# -eq 6 ] ; then
aaa ="bb"

else
	exit -1
fi
echo $cmdstr
vtysh -c "$cmdstr"	
	if [ $? -eq 0 ] ; then		
		exit 0	
	else		
		exit -2	
	fi
