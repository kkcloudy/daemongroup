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

CONF_A=/opt/services/conf/ApInfo_conf.conf
CONF_M=/opt/services/conf/map_conf.conf
STATUS_A=/opt/services/status/ApInfo_status.status
STATUS_M=/opt/services/status/map_status.status


if [ ! -f $STATUS_A ]
	then
		touch $STATUS_A
		chmod 644 $STATUS_A
		echo "1" > $STATUS_A
fi

if [ ! -f $STATUS_M ]
        then
                 touch $STATUS_M
                 chmod 644 $STATUS_M
                 echo "1" > $STATUS_M
fi


cp /opt/www/htdocs/ApMonitor/ApInfo.xml $CONF_A
cp /opt/www/htdocs/ApMonitor/map.xml $CONF_M  
