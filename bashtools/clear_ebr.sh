#!/bin/sh

for ebr in $(brctl show | grep $1 | awk '{print $1}')
	do
		ifconfig $ebr down
		brctl delbr $ebr
	done
