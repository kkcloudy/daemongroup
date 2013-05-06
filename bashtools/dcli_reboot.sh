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
printf "Preparing for software reboot ."


ISSUE_REBOOT=$1
if [ x$ISSUE_REBOOT = x"1" ] ; then
SNAPBIG=2
else
SNAPBIG=1
fi

echo 1 > /mnt/softreboot
/usr/bin/sor.sh  cp softreboot 30

printf "."
. /usr/bin/libcritlog.sh

crit_sysop_log " EXEC reboot."

printf "."

. /usr/bin/libsnapshot.sh

printf "\nSaving syslog to storage media before reboot."

guardlog "[`who am i`] Take snapshot before manual reboot."
printf "."
sudo /usr/bin/takesnapshot.sh 1 $SNAPBIG 
printf "."

printf "\nShutdown storage media and reboot now ..."


if [ $# -eq 0 ] ; then
	reboot
fi
