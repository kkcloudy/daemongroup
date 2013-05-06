#!/bin/bash
source vtysh_start.sh
vtysh -c "download configure $1 $2 $3"
