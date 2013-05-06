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

# Storage Operation Request

source vtysh_start.sh
RUNDIR='/var/run/sad'
CMDQUEUE=$RUNDIR'/cmdqueue'
SADSTARTTIME=$RUNDIR"/sadstarttime"
FREESPACE=$RUNDIR"/freespace"

SYNRETFILE=$RUNDIR"/sor_ret_${PPID}_$$"
RESULTOUTPUTFILE=$RUNDIR"/sor_result_${PPID}_$$"

[ -f $SYNRETFILE ] || touch $SYNRETFILE
[ -f $RESULTOUTPUTFILE ] || touch $RESULTOUTPUTFILE
chmod 666 $SYNRETFILE
chmod 666 $RESULTOUTPUTFILE

LOGTAG="SOR$$"

logger -p daemon.info -t $LOGTAG "Process ${PPID} called SOR with CMD:[$@]"

if [ ! $# -eq 3 ] ; then
    echo -e "\nError: Wrong parameters.\n"
    echo -e "\nUsage: sor.sh OPCMD OPFILENAME TIMEOUT\n"
    echo "OPCMD: cp rm ls imgls imgmd5 cpformblk ap_imgls"
    echo "OPFILENAME: full filename WITHOUT /mnt or /blk, e.g. /wtp/wtpcompatible.xml, /AW1.2.8.2335.X7X5.IMG, etc."
    echo -e "TIMEOUT: timeout process after TIMEOUT seconds if sad still hasn't return\n\n"
    logger -p daemon.info -t $LOGTAG "wrong args, exit 1"
    exit 1
# 1 for input args wrong 
fi

OPCMD=$1
OPFILENAME=$2
TIMEOUT=$3


if [ ! -f $CMDQUEUE ] ; then
    echo "Storage Agent Daemon not Ready."
    logger -p daemon.info -t $LOGTAG "SAD not ready, exit 2"
    exit 2
fi
SADSTATE=$RUNDIR"/sadstate"

TIME_OUT_COUNT=0
while true 
do 
SADSTATEV=`cat $SADSTATE 2> /dev/null`
  if [ "$SADSTATEV" = "" ] ; then 
    sleep 1
    TIME_OUT_COUNT=`expr $TIME_OUT_COUNT + 1`

    if [ $TIME_OUT_COUNT -eq 3 ] ; then
	  errmsg="SAD not ready, exit 2"
      echo "$errmsg"
      logger -p daemon.info -t $LOGTAG "$errmsg"
      exit 2
    fi
  continue
  fi
break;

done 

if [ $SADSTATEV -eq 1 ] ; then
    echo "Storage Agent Daemon has been BUSY since `cat $SADSTARTTIME`, current time is [`date +%Y%m%d%H%M%S`]."
    logger -p daemon.info -t $LOGTAG "SAD busy since `cat $SADSTARTTIME`, current time is [`date +%Y%m%d%H%M%S`], exit 3"
    exit 3
fi

getfilesize() 
{
  file=/mnt/$1
  # read from pipe only work within while
  ls --block-size=1 -s $file | 
  while read size filename
  do
    echo $size
  done
}

# Set 0 to synret file, will be set to 1 when sad finished operation.
# and will be 2 if no enough space
#echo 0 > $SYNRETFILE
# CAN't echo 0 to this file here, this will cause reading opret always fail

logger -p daemon.info -t $LOGTAG "send cmd to SAD CMD QUEUE:[$CMDQUEUE]."
echo $OPCMD $OPFILENAME $SYNRETFILE $RESULTOUTPUTFILE >> $CMDQUEUE

CMDEXECTIME=0

while true  
do

 if read -r opret ; then 
  logger -p daemon.info -t $LOGTAG "got $opret from $SYNRETFILE"
  if [ $opret -eq 1 ] ; then
    # SAD operation finished
    logger -p daemon.info -t $LOGTAG "after [$CMDEXECTIME] seconds result return from sad."
    if [ $OPCMD = "ls" ] || [ $OPCMD = "imgls" ] || [ $OPCMD = "imgmd5" ] || [ $OPCMD = "fastfwdls" ] || [ $OPCMD = "ap_imgls" ]; then
      cat $RESULTOUTPUTFILE
    fi 
    # return with success code 0;
    exit 0;
  elif [ $opret -eq 2 ] ; then
    if [ $OPCMD = "cp" ] ; then
      filesize=$(getfilesize $OPFILENAME)
      freespace=`cat $FREESPACE`
      errmsg="No Enough Space. File $OPFILENAME size [$filesize] >= freespace [$freespace]."
      echo "$errmsg"
      logger -p daemon.info -t $LOGTAG "$errmsg"
      exit 5
    fi

  fi  
 fi


# echo "nothing readed from $SYNRETFILE"
 sleep 1

 CMDEXECTIME=$(($CMDEXECTIME + 1))
 if [ $CMDEXECTIME -gt $TIMEOUT ] ; then
   errmsg="sor timeout after [$CMDEXECTIME] seconds."
   logger -p daemon.info -t $LOGTAG "errmsg"
   echo "$errmsg"
   # cmd failed due to sad timeout.
   exit 4
 fi
done < $SYNRETFILE

