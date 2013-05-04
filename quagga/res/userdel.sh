#!/bin/bash

if [ $# -eq 1 ] ; then
	userdel $1
	if[ $? -eq 0] ; then
	exit 0
	else
	exit -1
	fi
else
	exit -2
fi
