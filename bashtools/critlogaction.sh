#!/bin/sh
#
###########################################################################
#
#              Copyright (C) Autelan Technology
#
#This software file is owned and distributed by Autelan Technology 
#
############################################################################
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################
#
# eag_init
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin

. /usr/bin/libcritlog.sh

guardlog "[$$]Start of critlogaction.sh,trying to read."

read logmsg
guardlog "[$$]got log msg $logmsg."
crit_syslog $logmsg
guardlog "[$$]finished crit_syslog."

# If no reboot request was found, then exit.
echo $logmsg | grep "reboot is necessary" > /dev/null
ret=$?
guardlog "[$$]checked reboot string."
if [ $ret -eq 1 ] ; then
guardlog "[$$]exit 0."
	exit 0
fi
guardlog "[$$]reboot is necessary."

# If reboot request was found ,create system snapshot and restart
. /usr/bin/libsnapshot.sh

guardlog "[$$]Take snapshot caused by critical log [$logmsg]"
sudo /usr/bin/takesnapshot.sh 1 2

echo "System reboot for $CRITMSG" > /dev/console
echo 2 > /blk/softreboot

reboot

exit 0
