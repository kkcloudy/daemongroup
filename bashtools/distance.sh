#!/bin/sh

source vtysh_start.sh

if [ $# -eq 2 ] ; then 
	if [ $1 = "on" ] ; then
		cmdstr="configure terminal
		router rip
		distance $2
		"
	else
		cmdstr="configure terminal
	                router rip
			no distance $2
			"
	fi
elif [ $# -eq 3 ] ; then
       if [ $1 = "on" ] ; then
       		cmdstr="configure terminal
		router rip	   
		distance $2 $3
		"
	else 								                       cmdstr="configure terminal
			router rip
			no distance $2 $3
			"
	fi

elif [ $# -eq 4 ]; then 
       if [ $1 = "on" ] ; then
     		cmdstr="configure terminal
		router rip
		distance $2 $3 $4
		"
	else            
									                       cmdstr="configure terminal
			 router rip
			no distance $2 $3 $4
			"
	fi
else

	exit -2
fi
	vtysh -c "$cmdstr"
if [ 0 -eq $? ] ; then
	exit 0
else
	exit -1
fi



