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

TIPS1="malloc error"
TIPS2="input id is invalid"
TIPS3="EBR ID is not exist"
TIPS4="EBR interface error"
TIPS5="error"
TIP="/var/run/apache2/ebrmapinter.txt"

flag=$(grep -w -c "$TIPS1" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 1;
                fi
flag=$(grep -w -c "$TIPS2" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 2;
                fi
flag=$(grep -w -c "$TIPS3" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 3;
                fi
flag=$(grep -w -c "$TIPS4" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 4;
                fi        
flag=$(grep -w -c "$TIPS5" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 5;
                fi
               
