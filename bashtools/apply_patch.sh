#!/bin/sh
. /usr/bin/libinstpatch.sh


patch=$1

log_action_msg1 "get type of $patch"
if [ "xsp" == "x${patch##*.}" ] ; then
	log_action_msg1 "$patch is normal install patch"
	patch_temp=`basename $patch .sp`
  apply_one_patch $patch_temp
fi


if [ "xsps" == "x${patch##*.}" ] ; then
	log_action_msg1 "$patch was encrypted install patch"
	patch_temp=`basename $patch .sps`
  apply_one_patch_sec $patch_temp
fi

echo "done ($patch)"
