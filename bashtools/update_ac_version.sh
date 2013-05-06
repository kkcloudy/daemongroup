#!/bin/bash
(source vtysh_start.sh
vtysh -c "download img $1 $2 $3
	 set boot_img $4
	 reboot") &
