#!/bin/bash

if [ $# -eq 3 ] ; then
	if [ "normal" = $3 ] ; then
	password=$(openssl passwd -1 $2)
	else
	password=$2
	fi
	usermod -p $password $1
	if [ $? -eq 0 ] ; then
	exit 0
	else
	exit -1
	fi
else 
	echo "Usage: chpass.sh USERNAME PASSWORD"
	exit -1
fi
