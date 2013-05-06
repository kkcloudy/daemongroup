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

DHCP_LEASE=/var/lib/dhcp3/dhcpd.leases
LEASE_CONF=/var/run/apache2/dhcp_leasez.conf

if [ ! -f $LEASE_CONF ]
        then
                sudo touch $LEASE_CONF
                sudo chmod 666 $LEASE_CONF
fi
if [ -f $DHCP_LEASE ]
then
cat $DHCP_LEASE|grep -v ^#|awk  '{if(/^lease|^  binding|^  starts|^  ends|^  hardware/) { ORS=" " ; for(i=2;i<=NF;i++) {print $i}} if(/^}$/) { ORS="\n"; print ""} }'| grep active|awk '{print $1,$12,$4"-"$5,$7"-"$8}'|sed 's/;//g' > $LEASE_CONF
fi
