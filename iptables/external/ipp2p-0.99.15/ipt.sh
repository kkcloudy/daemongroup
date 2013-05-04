#!/bin/bash
iptables-save > tmp.save
/etc/init.d/iptables stop
rmmod ipt_ipp2p
/etc/init.d/iptables start
iptables-restore tmp.save
rm tmp.save
