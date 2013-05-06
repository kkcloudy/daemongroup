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

# if_show_subif.sh PORTNO
source vtysh_start.sh

if [ $# -eq 1 ] ; then
        PORT_TAGS=$(ip addr | grep "\eth$1\." | sed "/inet/d" | sed "s/.*eth$1\.\(.*\):.*/\1/g")

#       echo ${PORT_TAGS}
        for tag in ${PORT_TAGS}
        do
                IFNAME_OF_TAG=$(ip addr | grep "eth$1\.$tag:" | sed "/inet/d" | awk '{print $2}' | sed "s/:$//g")
                IP_OF_TAG=$(ip addr | grep "eth$1\.$tag$" | grep "inet" | awk '$1=="inet" { print $2 }' | sed ':a;N;s/\n/,/;ba;')
                echo ${IFNAME_OF_TAG}"#"${IP_OF_TAG}"#"${tag}
        done
else
        exit -200
fi
