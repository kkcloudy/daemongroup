#!/bin/sh

printf "Content-type: text/html\n\n"

printf "<html><head><title>debug download only</title></head>\n\n"
printf "<body><p>Snapshot file list</p>"

printf "<table>"

count=0

for f1 in /opt/debugdown/* ; do
if [ -f $f1 ] ; then
fbase=`basename $f1`
printf "<tr>"
printf "<td><a href=\"/debug_download_only/$fbase\">$fbase</a></td>"
printf "<td><form name=delete action=/debug/deletesnapshot.cgi method=POST><input type=submit name=submit value=delete><input type=hidden name=fname value=$fbase></form> </td>"
printf "</tr>\n\n"
count=$(($count + 1))
fi
done

printf "</table>"
printf "<p>Total $count files</p></body></html>"
