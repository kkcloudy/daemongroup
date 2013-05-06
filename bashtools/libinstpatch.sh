#!/bin/bash

log_action_msg1 () {
    echo "$@."
}

apply_one_patch() 
{
patch1=$1
pushd /mnt/patch > /dev/null
tar xvjf $patch1.sp > /dev/null 2>&1
	if [ -d $patch1 ] ; then
	        log_action_msg1 " Extract patch `basename $patch1`"
		if [ -f $patch1/install ] ; then
	        	log_action_msg1 "  Trying to install patch `basename $patch1`"
			pushd $patch1 > /dev/null 
			[ -x install ] || chmod +x install
			./install
			ret=$?
			popd > /dev/null 
	        	log_action_msg1 "  Done($ret) with install patch `basename $patch1`"
		else
		log_action_msg1 "  But no install file found."
		fi
	else
		log_action_msg1 "  Unknown format of $patch1.sp"
	fi

popd > /dev/null
}	

apply_one_patch_sec()
{
	patch1=$1
	echo $patch1
	pushd /mnt/patch > /dev/null
	tar xvjf $patch1.sps > /dev/null 2>&1
	pushd $patch1 > /dev/null
	openssl rsautl -verify -pubin -inkey /etc/pub.pem -in passwd_sec -out passwd_
	
	if [ $? -ne 0 ] ; then
		log_action_msg1 "install patch of $patch1 error[1]"
		exit -1
	fi
	
	openssl enc -d -a -des3 -in $patch1.sp -out patch.tar -kfile passwd_
	
	
	if [ $? -ne 0 ] ; then
		log_action_msg1 "install patch of $patch1 error[2]"
		exit -1
	fi
	
	mv patch.tar $patch1.sp > /dev/null 2>&1
	tar xvjf $patch1.sp > /dev/null 2>&1
		if [ -d $patch1 ] ; then
		        log_action_msg1 " Extract patch `basename $patch1`"
			if [ -f $patch1/install ] ; then
		        	log_action_msg1 "  Trying to install patch `basename $patch1`"
				pushd $patch1 > /dev/null 
				[ -x install ] || chmod +x install
				./install
				ret=$?
				popd > /dev/null 
		        	log_action_msg1 "  Done($ret) with install patch `basename $patch1`"
			else
			log_action_msg1 "  But no install file found."
			fi
		else
			log_action_msg1 "  Unknown format of $patch1.sp"
		fi
	
	popd > /dev/null
	
}
