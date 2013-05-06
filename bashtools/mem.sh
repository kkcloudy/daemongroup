#!/bin/bash
function getmem
{
	memuse=`ps -o vsz -p $1 | grep -v VSZ`
	((memuse /= 1000))
	echo $memuse
}
getmem $1

