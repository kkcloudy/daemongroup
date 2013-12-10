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
# hmdreloadconfig
#
# CREATOR:
# autelan.software.xxx. team
# 
# DESCRIPTION: 
#     
#############################################################################


CONFIG_PATH="/var/run/config_bak/"

source vtysh_start.sh
vtysh -c "show running"	> /var/run/config/temp_showrun
rm $CONFIG_PATH -rf >/dev/null
mkdir $CONFIG_PATH >/dev/null
cp /var/run/config/Instconfig$1-0-$2 $CONFIG_PATH >/dev/null

filelist=`ls /var/run/config_bak`
for file in $filelist
do
		CONFIG_FILE=$CONFIG_PATH$file
		sed -i '/^\s*ip\s\+address\s\+\([0-9]\{1,3\}\.\)\{3\}[0-9]\{1,3\}\/[0-9]\{1,2\}\s*$/d' $CONFIG_FILE
done

