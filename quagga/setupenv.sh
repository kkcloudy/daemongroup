#!/bin/bash

echo "#################################################################"
echo "Warning:"
echo "This script intends set this copy of quagga as the public quagga in this server which will be used by users while compiling dcli module." 
echo "This script will add environment variables into /etc/profile and /etc/bash.bashrc"
echo "This script should be run as root"
echo "#################################################################"


if [ "`id -u`" -ne 0 ]; then
	echo "You are not root"
	exit 1;
fi

line1='export QUAGGA_DIR="'$(pwd)'"'

echo $line1

cat <<EOF >> /etc/bash.bashrc
${line1}

EOF

cat <<EOF >> /etc/profile
${line1}

EOF

