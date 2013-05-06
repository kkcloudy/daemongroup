#!/bin/sh

RUNFLAG="/var/run/snapshot.cgi.run"
runf=0

if [ -f $RUNFLAG ] ; then
runf=`cat $RUNFLAG`
fi

if [ x"$runf" = x"1" ] ; then
printf "Content-type: text/html\n\n"
printf "Already running.\n\n"
exit 0
fi

echo 1 > $RUNFLAG
sudo /usr/bin/debugdownsnapshot.sh
echo 0 > $RUNFLAG

#printf "Location: https://${HTTP_HOST}/debug_download_only/\n\n"
printf "Location: /debug/listsnapshot.cgi\n\n"
