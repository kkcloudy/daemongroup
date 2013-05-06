#!/bin/bash
if [ $# -gt 3 ] ; then 
	echo " error arg \n"
	return
fi
umount $1 
umount $11 
umount $12
echo -e "d\n1\nd\n2\nd\n3\nd\n4\no\nn\np\n1\n\n+512M\nt\nb\nn\np\n2\n\n\nw\n" | fdisk $1

umount $11 
mkfs.vfat $11
umount $12
mke2fs -q -j $12

