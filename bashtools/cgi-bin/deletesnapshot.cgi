#!/bin/sh

read parameters
fname=`echo $parameters | sed -e 's/^.*fname=\([^&]*\).*$/\1/' | sed 's/%20/ /g'`

rm -rf /opt/debugdown/$fname
printf "Location: /debug/listsnapshot.cgi\n\n"
