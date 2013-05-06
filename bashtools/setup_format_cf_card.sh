#!/bin/bash
if [ $# -gt 3 ] ; then 
	echo " error arg \n"
	return
fi
sudo umount $1 
sudo umount $11 
sudo umount $12

sudo fdisk $1 << EOF
d
1
d
2
d
3
d
4
o
n
p
1

+512M
t
b
n
p
2

+256M
t
2
b
n
p
3


w
EOF

sudo umount $11 
sudo mkfs.vfat $11
sudo umount $12
sudo mkfs.vfat $12 
sudo umount $13
sudo mke2fs -q -j $13

