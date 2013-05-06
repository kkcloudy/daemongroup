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

# Storage Agent Daemon
# Agent for storage operation
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin

RUNDIR='/var/run/sad'

CMDQUEUE=$RUNDIR'/cmdqueue'
SADSTARTTIME=$RUNDIR"/sadstarttime"
EXECLOG=$RUNDIR"/sadexec.log"
TOTALSPACE=$RUNDIR"/totalspace"
FREESPACE=$RUNDIR"/freespace"


# STOPFLAG used by baseguard/logcheck to stop sad operation
# 1 for sad stop
# 0 for sad work
STOPFLAG=$RUNDIR"/stopflag"

# SADSTATE
# 0 for idle
# 1 for busy
SADSTATE=$RUNDIR'/sadstate'

[ -d $RUNDIR ] || mkdir $RUNDIR
[ -f $CMDQUEUE ] || touch $CMDQUEUE
[ -f $SADSTATE ] || touch $SADSTATE
[ -f $SADSTARTTIME ] || touch $SADSTARTTIME
[ -f $EXECLOG ] || touch $EXECLOG
[ -f $TOTALSPACE ] || touch $TOTALSPACE
[ -f $FREESPACE ] || touch $FREESPACE
[ -f $STOPFLAG ] || touch $STOPFLAG

chmod 777 $RUNDIR
chmod 666 $CMDQUEUE
chmod 666 $SADSTATE
chmod 666 $SADSTARTTIME
chmod 666 $EXECLOG
chmod 666 $TOTALSPACE
chmod 666 $FREESPACE
chmod 666 $STOPFLAG

>$EXECLOG

LOGTAG="sad"

logger -p daemon.info -t $LOGTAG "StorageAgentDaemon started." 

refresh_space() 
{
  part=$1
  # read from pipe only work within while
  df -B 1  | grep $part | 
  while read dev size used avai useper mountpoint 
  do
    echo "$size" > $TOTALSPACE
    echo "$avai" > $FREESPACE
  done
}

logger -p daemon.info -t $LOGTAG "Trying to get storage space usage." 
mount /blk
refresh_space /blk
umount /blk
logger -p daemon.info -t $LOGTAG "Storage total space `cat $TOTALSPACE` Bytes , free space `cat $FREESPACE` Bytes." 

lsimgs()
{
  dir=$1
  output=$2
  tmpfile=$RUNDIR/tmp1
  >$output
  
  for f1 in $dir/* ; do
    if [ -f $f1 ] ; then
      dd if=$f1 of=$tmpfile bs=4 count=1
      cmp $tmpfile /etc/imgmagic
      ret_temp=$?
      if [ $ret_temp -eq 0 ] ; then
        imgname=`basename $f1`
		case "$imgname" in
		*.img) echo $imgname >> $output ;;
		*.IMG) echo $imgname >> $output ;;
		*) ;;
		esac
      fi
    fi 
  done
}

#Huang Leilei add for show ap_boot_img cli
ap_lsimgs()
{
  dir_ap=$1
  output_ap=$2
  tmpfile_ap=$RUNDIR/tmp1
  >$output_ap
  
  for f_ap in $dir_ap/* ; do
    if [ -f $f_ap ] ; then
      dd if=$f_ap of=$tmpfile_ap bs=4 count=1
      cmp $tmpfile_ap /etc/ap_imgmagic
      ret_temp_ap=$?
      if [ $ret_temp_ap -eq 0 ] ; then
        imgname_ap=`basename $f_ap`
		case "$imgname_ap" in
		*.img) echo $imgname_ap >> $output_ap ;;
		*.IMG) echo $imgname_ap >> $output_ap ;;
		*) ;;
		esac
	  else
		dd if=$f_ap of=$tmpfile_ap bs=4 count=1 skip=128
		cmp $tmpfile_ap /etc/ap_imgmagic
		ret_temp_ap=$?
		if [ $ret_temp_ap -eq 0 ] ; then
                  imgname_ap=`basename $f_ap`
		  case "$imgname_ap" in
		    *.img) echo $imgname_ap >> $output_ap ;;
		    *.IMG) echo $imgname_ap >> $output_ap ;;
		    *) ;;
		  esac
                fi
      fi
    fi 
  done
}

#added by zhaocg for fastfwd file
lsfastfwd()
{
  dir=$1
  output=$2
  for f1 in $dir/* ; do
    if [ -f $f1 ] ; then
    	fastfwdname=`basename $f1`
        case "$fastfwdname" in
        	*.fastfwd.bin) echo "testing testing"
						   echo $fastfwdname >> $output
        	                ;;
               	*)
               	   ;;
        esac
    fi 
  done
}

md5img()
{
imgfile=$1
output=$2
pushd /blk
openssl md5 $1 > $2 2>&1
popd
}

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

storage_cmd_execute()
{
opcmd=$1 
opfilename=$2 
synretfile=$3 
resultoutputfile=$4
ret=1

echo "[`date +%Y%m%d%H%M%S`]" > $SADSTARTTIME

    mount /blk
    case "$opcmd" in
      cp)
          destdir=`dirname /blk/$opfilename`
          [ -d $destdir ] || mkdir -p $destdir ; chmod 777 $destdir
          filesize=$(getfilesize $opfilename)
          refresh_space /blk
          freespace=`cat $FREESPACE`
          if [ $filesize -ge $freespace ] ; then
             logger -p daemon.info -t $LOGTAG "No Enough Space. File $OPFILENAME size [$filesize] >= freespace [$freespace]."
             ret=2
          else
#            if [ $filesize -gt 1000000 ] ; then
              cp -rf /mnt/$opfilename $destdir
#            else
#              cp --backup=simple -rf /mnt/$opfilename $destdir
#            fi
	    chmod a+rw /blk/$opfilename
          fi
          ;;
			cpfromblk)
					destdir=`dirname /mnt/$opfilename`
					[ -d $destdir ] || mkdir -p $destdir ; chmod 777 $destdir
					l
					cp -rf /blk/$opfilename $destdir
					;;

      rm)
          rm -rf /blk/$opfilename
          ;;
      ls)
          ls /blk/$opfilename > $resultoutputfile
          ;;
      imgls)
          lsimgs /blk $resultoutputfile
          ;;
      fastfwdls)
      	  lsfastfwd /blk $resultoutputfile
          ;;
      ap_imgls)
		  ap_lsimgs /mnt/wtp $resultoutputfile
	  ;;
      imgmd5)
          md5img $opfilename $resultoutputfile
          ;;
      *)
          logger -p daemon.info -t $LOGTAG "unknow opcmd."
    esac 
    sync
    refresh_space /blk
    if [ `cat $STOPFLAG` -eq 1 ] ; then 
      logger -p daemon.info -t $LOGTAG "Stop flag set, skip umount."
    else
      umount /blk
    fi
    #Set synret file to indicate operation finished.
    logger -p daemon.info -t $LOGTAG "writing $ret to $synretfile"
    echo $ret > $synretfile
}

echo 0 > $STOPFLAG

check_fs()
{
    if [ -e /dev/mmcblk0p1 ]; then
        fsck.vfat -a /dev/mmcblk0p1
        break;
    elif [ -e /dev/cfa1 ]; then
        fsck.vfat -a /dev/cfa1
        break;
    elif [ -e /dev/cfa ]; then
        fsck.vfat -a /dev/cfa
        break;
    elif [ -e /dev/sda1 ]; then
        fsck.vfat -a /dev/sda1
        break;
    elif [ -e /dev/sda ]; then
        fsck.vfat -a /dev/sda
        break;
    else
        printf "No Storage Device\n"
    fi
}

while true 
do 
  if [ `cat $STOPFLAG` -eq 1 ] ; then 
    sleep 10
    continue
  fi
  echo 0 > $SADSTATE
  echo 0 > $SADSTARTTIME
  sleep 1

# Read next cmd
  if read -r opcmd opfilename synretfile resultoutputfile ; then
    #Set SAD state to busy
    echo 1 > $SADSTATE

	#Check and repair the FAT file system before any operation. Jia Lihui.
	#check_fs   

    JOBCMD="Opcmd $opcmd file $opfilename sync-call-return $synretfile result-output-file $resultoutputfile"
    logger -p daemon.info -t $LOGTAG "SAD got cmd $JOBCMD and execute it." 
    echo "Start cmd on `date`" >> $EXECLOG
    storage_cmd_execute $opcmd $opfilename $synretfile $resultoutputfile >> $EXECLOG 2>&1
    echo "End cmd on `date`" >> $EXECLOG 
    logger -p daemon.info -t $LOGTAG "finished executing cmd."
  fi
 
  
done < $CMDQUEUE
