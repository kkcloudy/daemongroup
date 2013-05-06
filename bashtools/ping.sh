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
#cgi中直接用system，popen时，如果主机不可达，则命令不能返回，这个脚本用于解决这个问题。

COUNT=4

#下面的值只是为了循环，没有实际意义。最多等待数字个数×SLEEP_DIV时间后就退出
CHECK_SEQ="1 2 3 4 5 6 7 8"

TIME_DIV_PACKAGE=0.2
SLEEP_DIV=0.3
PING_CONTENT=/var/run/ping_temp
CANNOT_REACH="Network $1 is unreachable"

#rm -f $PING_CONTENT
KEY="ping -c $COUNT -i $TIME_DIV_PACKAGE $1"

ping_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
while [ $ping_pid ]
do
        kill -9 $ping_pid
        ping_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
done

#开一个子进程来ping
echo "$KEY" | sh 2>/dev/null 1>$PING_CONTENT_$$ &
#在父进程中查询子进程的ping的状态。
#如果子进程结束，说明能ping通。退出脚本。
#如果在CHECK_SEQ.NUM*SLEEP_DIV还没有退出，则强行将ping进程杀掉，然后退出shell，同时输出不能ping通的提示。
for i in $CHECK_SEQ
do
        sleep $SLEEP_DIV
        ping_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
        if [ "x$ping_pid" == "x" ];then
                content=`cat $PING_CONTENT_$$`
                if [ "x$content" == "x" ];then
                        echo $CANNOT_REACH            
                else
                        cat $PING_CONTENT_$$
                fi
                rm -f $PING_CONTENT_$$ 
                exit 0
        fi
done

kill -15 $ping_pid 2>/dev/null
cat $PING_CONTENT_$$

echo $CANNOT_REACH
rm -f $PING_CONTENT_$$
exit 1
