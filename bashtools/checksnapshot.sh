#!/bin/sh

snapshotlist=`/usr/bin/sor.sh ls snapshot 30`

LOGTAG=SNAPCHK

MAXSNAPCOUNT=6

count=`echo $snapshotlist | wc -w`

logger -t $LOGTAG -p cron.info "Checked [$count] snapshot dirs."

if [ $count -gt $MAXSNAPCOUNT ]  ; then

for eachfile in $snapshotlist
do
	if [ $count -gt $MAXSNAPCOUNT ] ; then
		/usr/bin/sor.sh rm snapshot/$eachfile 30
		logger -t $LOGTAG -p cron.info "Clean snapshot dirs [$eachfile]."
	fi
	count=$(($count - 1))
done

fi
