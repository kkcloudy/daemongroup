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

source vtysh_start.sh
TIPS1="malloc error"
TIPS2="input id is invalid"
TIPS3="WTP ID is not exist"
TIPS4="Wlan ID is not exist"
TIPS5="RADIO ID is not exist"
TIPS6="BSS is not exist"
TIPS7="WTP is not binding wlan,binding it first"
TIPS8="WTP binding WLAN is not match,check again"
TIPS9="BSS is enable, if you want to operate this, please disable it first"
TIPS10="WLAN is no_interface , BSS can not be wlan_interface"
TIPS11="BSS create l3 interface fail"
TIPS12="BSS delete l3 interface fail"
TIPS13="WLAN create l3 interface fail"
TIPS14="WLAN delete l3 interface fail"
TIPS15="this radio is in ebr"
TIPS16="layer3-interface error"
TIPS17="unknown error occur"
TIPS18="other error"
TIP="/var/run/apache2/radmapinter.txt"

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
flag=$(grep -w -c "$TIPS6" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 6;
                fi
flag=$(grep -w -c "$TIPS7" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 7;
                fi                
flag=$(grep -w -c "$TIPS8" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 8;
                fi                
flag=$(grep -w -c "$TIPS9" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 9;
                fi                
 flag=$(grep -w -c "$TIPS10" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 10;
                fi                
flag=$(grep -w -c "$TIPS11" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 11;
                fi                 
flag=$(grep -w -c "$TIPS12" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 12;
                fi      
 flag=$(grep -w -c "$TIPS13" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 13;
                fi
 flag=$(grep -w -c "$TIPS14" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 14;
                fi             
flag=$(grep -w -c "$TIPS15" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 15;
                fi                
flag=$(grep -w -c "$TIPS16" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 16;
                fi
flag=$(grep -w -c "$TIPS17" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 17;
                fi
flag=$(grep -w -c "$TIPS18" $TIP)
                if [ $flag -gt  0 ]
                        then
                                exit 18;
                fi                                