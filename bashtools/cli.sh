#!/bin/sh
source vtysh_start.sh
vtysh -c "set cli-log $1"

