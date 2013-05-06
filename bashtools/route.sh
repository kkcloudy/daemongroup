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

COUNT=4

SLEEP_DIV=0.3
ROUTE_CONTENT=/var/run/route_temp
CANNOT_REACH=" "

#rm -f $ROUTE_CONTENT
KEY="traceroute -m $COUNT  $1"

route_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
while [ $route_pid ]
do
        kill -9 $route_pid
        route_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
done

#开一个子进程来
echo "$KEY" | sh 2>/dev/null 1>$ROUTE_CONTENT_$$ &

#在父进程中查询子进程的状态。
#如果在CHECK_SEQ还没有退出，则强行将进程杀掉，然后退出shell，同时输出不能到达的提示。

sleep 5

while [ $route_pie ]
do
        sleep $SLEEP_DIV
        route_pid=`ps -ef | grep "$KEY" | sed '2,100d'|sed '/grep/d' | awk '{print $2}'`
        if [ "x$route_pid" == "x" ];then
                content=`cat $ROUTE_CONTENT_$$`
                if [ "x$content" == "x" ];then
                        echo $CANNOT_REACH            
                else
                        cat $ROUTE_CONTENT
                fi
                rm -f $ROUTE_CONTENT_$$
                exit 0
        fi
done

kill -15 $route_pid 2>/dev/null
cat $ROUTE_CONTENT_$$

echo $CANNOT_REACH
rm -f $ROUTE_CONTENT_$$

exit 1

