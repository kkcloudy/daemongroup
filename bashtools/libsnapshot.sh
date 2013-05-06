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

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/opt/bin

SADSTOP="/var/run/sad/stopflag"
GUARDLOG="/var/log/baseguard.log"
DEBUGDOWN="/opt/debugdown"

def_sig()
{
	#echo catch sig put 0 to stop flag
	echo 0 > $SADSTOP
	exit 0
}
guardlog()
{
    echo "`date +%Y%m%d%H%M%S`:$*" >> $GUARDLOG
}

syncnow()
{
if [ $SYNCNOW -eq 1 ] ; then
sync
fi
}

makesure_blk()
{
guardlog "Check blk."
cat /proc/mounts | grep blk > /dev/null
if [ $? -eq 1 ]; then
  guardlog "blk not mounted, try to mount it."
  mount /blk
  guardlog "Confirm blk."
  makesure_blk
  echo "mount blk failed,try again" > /dev/console
else
  guardlog "blk already mounted."
fi
}

takecontrolof_storage()
{
#echo put 1 to stop flag
trap "def_sig" 1 2 3 24
#echo 1 > $SADSTOP
#guardlog "Pause sad."
pkill sad.sh
makesure_blk

}
stopcontrolof_storage()
{
trap 1 2 3 24
#echo put 0 to stop flag
#echo 0 > $SADSTOP
#guardlog "restart sad."
sudo sad.sh &

}


checksnapshotdir_log_count()
{
snapshotlist=`/usr/bin/sor.sh ls snapshot 30`
 
LOGTAG=SNAPCHK

MAXSNAPCOUNT=5

count=`echo $snapshotlist | wc -w`

#logger -t $LOGTAG -p cron.info "Checked [$count] snapshot dirs."
guardlog "Checked [$count] snapshot dirs."
 
if [ $count -gt $MAXSNAPCOUNT ]  ; then

 for eachfile in $snapshotlist
 do
         if [ $count -gt $MAXSNAPCOUNT ] ; then
                 /usr/bin/sor.sh rm snapshot/$eachfile 30
                 #logger -t $LOGTAG -p cron.info "Clean snapshot dirs [$eachfile]."
		 guardlog "Clean snapshot dirs [$eachfile.]"
         fi
         count=$(($count - 1))
 done
 
fi
}

createsnapshotdir()
{
SNAPSHOTDIR="/blk/snapshot/`date +%Y%m%d%H%M%S`"
[ -d $SNAPSHOTDIR ] || mkdir -p $SNAPSHOTDIR
guardlog "Create snapshot dir $SNAPSHOTDIR."
}

forcequit()
{
progname=$1
timeout=$2
guardlog "Force quit check $progname."
while true 
do
  ps -e | grep $progname > /dev/null
  fret=$?
  if [ $fret -eq 0 ] ; then
    if [ $timeout -gt 0 ] ; then
      guardlog "timeout now $timeout."
      timeout=$(($timeout -1 ))
      sleep 1
      continue
    else
      guardlog "$progname still running, force kill it."
      pkill $progname
    fi
  else
    guardlog "$progname exit already."
    break;
  fi
done
guardlog "Done force quit check $progname."
}

getbaseguardlog()
{
guardlog "Get base guard log"
cp $GUARDLOG $SNAPSHOTDIR/
cp -a /var/log/10 $SNAPSHOTDIR/
cp -a /var/log/12 $SNAPSHOTDIR/
cp -a /var/log/15 $SNAPSHOTDIR/
syncnow
guardlog "Done getbaseguardlog."
}

getbaseguardlogend()
{
guardlog "Get base guard log end"
cp $GUARDLOG $SNAPSHOTDIR/baseguard_end.log
syncnow
}

getcoredumplist()
{
guardlog "Get coredumplist"
ls /opt/bin/core/*core* -lh > $SNAPSHOTDIR/corefilelist 2>&1
syncnow
guardlog "Done getcoredumplist."
}

getcoredumpfiles()
{
corefilesize=`du /opt/bin/core -B 1 --max-depth=0 | awk '{ print $1 }'`
freespace=`cat /var/run/sad/freespace`
reservspace=$(($freespace - $corefilesize))
guardlog "Corefiles size [$corefilesize] freespace [$freespace] reservspace [$reservspace]"
if [ $reservspace -ge 104857600 ] && [ $corefilesize -gt 0 ] ; then
guardlog "Copy coredumpfiles."
cp /opt/bin/core/*core* $SNAPSHOTDIR/
syncnow
else
guardlog "Skip copy coredumpfiles."
fi
guardlog "Done getcoredumpfiles."
}

#huangjing
getsystemsensitiveinfo()
{
echo "the system Sensitive information :" > $SNAPSHOTDIR/sys_sensitive_info.log
echo "OBJS ACTIVE  USE OBJ SIZE  SLABS OBJ/SLAB CACHE SIZE NAME" >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==8' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==9' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==11' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==12' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==13' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==14' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==17' >> $SNAPSHOTDIR/sys_sensitive_info.log
sudo slabtop -o -s -c |awk 'NR==18' >> $SNAPSHOTDIR/sys_sensitive_info.log
echo >> $SNAPSHOTDIR/sys_sensitive_info.log
du -sh /var >> $SNAPSHOTDIR/sys_sensitive_info.log
du -sh /mnt >> $SNAPSHOTDIR/sys_sensitive_info.log
du -sh /opt >> $SNAPSHOTDIR/sys_sensitive_info.log
echo >> $SNAPSHOTDIR/sys_sensitive_info.log
echo "the processes number for current system:" >> $SNAPSHOTDIR/sys_sensitive_info.log
ps -ef|wc -l >> $SNAPSHOTDIR/sys_sensitive_info.log
echo >> $SNAPSHOTDIR/sys_sensitive_info.log
echo "show wid npd sem eag asd wsm process" >> $SNAPSHOTDIR/sys_sensitive_info.log
echo "PID %%CPU %%MEM  PROCESS" >> $SNAPSHOTDIR/sys_sensitive_info.log
ps aux f |grep wid |grep -v _ |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log
ps aux f |grep npd |grep -v _ |grep -v dhcpsnpd |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log

ps aux f |grep sem |grep -v _ |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log
ps aux f |grep eag |grep -v _ |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log
ps aux f |grep asd |grep -v _ |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log
ps aux f |grep wsm|grep -v _ |awk '{print $2,$3,$4,$11}' >> $SNAPSHOTDIR/sys_sensitive_info.log
}

getmem()
{
guardlog "Get mem overview"
free -blt > $SNAPSHOTDIR/free
date > $SNAPSHOTDIR/timeinfo
uptime >> $SNAPSHOTDIR/timeinfo
du -h --max-depth=1 --exclude=/proc --exclude=/sys --exclude=/blk / > $SNAPSHOTDIR/du
df -h > $SNAPSHOTDIR/df
syncnow
guardlog "Done getmem"
}


getprocess()
{
guardlog "Get process list"
ps -e -m -o ppid,pid,sess,nlwp,f,lwp,stat,psr,ni,pri,lstart,etime,time,%cpu,user,comm > $SNAPSHOTDIR/ps_cpu
ps -e -o pid,f,stat,rss=RESID-MEM-in-KiB,size=SWAP-SPACE,vsize=VIRT-MEM-in-KiB,%mem,ruser,euser,fuser,suser,cmd --sort=-rss> $SNAPSHOTDIR/ps_mem
ps -e H -o lwp,stat,psr,sgi_p,ni,pri,rtprio,sched,policy,wchan=SLEEPING-KERNEL-FUNC-ADDR,stackp,esp,eip,comm > $SNAPSHOTDIR/ps_sched
ps -e H -o lwp,stat,psr,sgi_p,ni,pri,rtprio,sig,sigcatch,sigignore,sigmask,comm > $SNAPSHOTDIR/ps_signal
syncnow
guardlog "Done getprocess."
}

getdmesg()
{
guardlog "Get dmesg"
dmesg > $SNAPSHOTDIR/dmesg
syncnow
guardlog "Done getdmesg"
}

getprocfiles()
{
guardlog "Get Proc files"
#Files under /proc can't be tar zipped
#tar cjf $SNAPSHOTDIR/proc.tar.bz2 /proc/cmdline /proc/coremask /proc/cpuinfo /proc/devices /proc/interrupts /proc/loadavg /proc/meminfo /proc/modules /proc/product_id /proc/slabinfo /proc/vmstat /proc/zoneinfo /proc/uptime /proc/sysinfo/* --ignore-failed-read
[ -d $SNAPSHOTDIR/proc ] || mkdir -p $SNAPSHOTDIR/proc
cp -t $SNAPSHOTDIR/proc /proc/cmdline /proc/coremask /proc/cpuinfo /proc/devices /proc/interrupts /proc/loadavg /proc/meminfo /proc/modules /proc/sys/fs/file-nr /proc/product_id /proc/slabinfo /proc/vmstat /proc/stat /proc/iomem /proc/ioports /proc/zoneinfo /proc/uptime /proc/sysinfo/module_sn /proc/sysinfo/product_sn /proc/sysinfo/product_base_mac_addr /proc/octeon_info /proc/octeon_perf /proc/version /proc/pci /proc/dma /proc/buddyinfo /proc/execdomains /proc/misc /proc/locks  
syncnow
guardlog "Done getprocfiles."
}

getlog()
{
guardlog "Get /var/log"
tar -cjPf $SNAPSHOTDIR/varlog.tar.bz2 /var/log/* >/dev/null 2>&1
syncnow
guardlog "Done getlog."
}

getrun()
{
guardlog "Get /var/run"
tar -cjPf $SNAPSHOTDIR/varrun.tar.bz2 /var/run/* >/dev/null 2>&1
syncnow
guardlog "Done getrun"
}

getlib()
{
guardlog "Get /var/lib"
tar -cjPf $SNAPSHOTDIR/varlib.tar.bz2 /var/lib/* >/dev/null 2>&1
syncnow
guardlog "Done getlib"
}

getversion()
{
guardlog "Get /etc/version"
tar -cjPf $SNAPSHOTDIR/etcversion.tar.bz2 /etc/version/* >/dev/null 2>&1
syncnow
guardlog "Done getversion."
}

gethardware()
{
guardlog "Get hardware config"
/opt/bin/vtysh -c "show system hardware" >  $SNAPSHOTDIR/hardwareconf
syncnow
guardlog "Done gethardware."
}

getuserlog()
{
guardlog "Get userlog"
tar cjPf /blk/syslog.tar.bz2 /var/log/syslogservice.log /var/log/system.log
syncnow
guardlog "Done get userlog"
}

getconfig()
{
guardlog "Get software config"
/opt/bin/vtysh -c "show running-conf" >  $SNAPSHOTDIR/softwareconf
cp /mnt/conf_xml.conf $SNAPSHOTDIR/
syncnow
guardlog "Done getconfig"
}

getmntfiles()
{
guardlog "Get mnt files"
#tar cjf $SNAPSHOTDIR/mnt.tar.bz2 /mnt/conf_xml.conf /mnt/devinfo /mnt/forcevstring /mnt/wtp/wtpcompatible.xml /mnt/lic/* --ignore-failed-read 
tar -cjPf $SNAPSHOTDIR/mnt.tar.bz2 /mnt/  --exclude="*img" --exclude="*bin" --exclude="*tar" --ignore-case >/dev/null 2>&1
syncnow
guardlog "Done getmntfile."
}


# Although we backup core file, directly generate bt file is still convenient for debug.
getcoredumpbt()
{
guardlog "Get coredumpbt"
for corefile in /opt/bin/core/*core*
do
    [ ! -f $corefile ] && continue
    basecorefile=`basename $corefile`
    #binfile=echo $corefile | sed -e 's/.core$//' -e 's/[[:digit:]]*//g' -e 's/_$//'
    basebinfile=`echo $basecorefile | sed -e 's/_[[:digit:]]*.core$//g'` 
    echo "gdb /opt/bin/$basebinfile /opt/bin/core/$basecorefile" > $SNAPSHOTDIR/gdb_bt.$basecorefile
    (sleep 1; echo "bt"; sleep 2; echo "quit") | gdb /opt/bin/$basebinfile /opt/bin/core/$basecorefile >> $SNAPSHOTDIR/gdb_bt.$basecorefile 2>&1 &
    forcequit gdb 5
done
syncnow
guardlog "Done getcoredumpbt"
}

getlsof()
{
guardlog "Get lsof"
lsof -nP > $SNAPSHOTDIR/lsof.txt 2>&1 &
forcequit lsof 15
syncnow
guardlog "Done getlsof"
}

getnetstat()
{
guardlog "Get netstat"
netstat -anpNeo > $SNAPSHOTDIR/netstat_anpNeo.txt 2>&1
netstat -s > $SNAPSHOTDIR/netstat_s.txt 2>&1
syncnow
guardlog "Done getnetstat"
}

save_sem_log()
{
guardlog "save sem log"
cp /var/run/bootlog.sem $SNAPSHOTDIR
guardlog "save sem log done"
}

record_octeon_reg()
{
guardlog "record octeon register value"
record_octeon_reg.sh $SNAPSHOTDIR/octeon_reg
guardlog "record octeon register value done"
}

take_snapshot()
{
SYNCNOW=$1
SNAPBIG=$2
guardlog "Start take snapshow with SYNCNOW[$SYNCNOW] SNAPBIG[$SNAPBIG]."

checksnapshotdir_log_count

if [ $SYNCNOW -eq 1 ] ; then
takecontrolof_storage
fi

createsnapshotdir

#huangjing
getsystemsensitiveinfo

# Get files won't be too large
getbaseguardlog
#If npd crash, hardware watchdog will reboot after less than 30 seconds.
# So get coredumpbt firstly.
getcoredumplist
getcoredumpbt

save_sem_log

getmem
getprocess
getdmesg
getversion
getprocfiles
gethardware
getuserlog

record_octeon_reg
# Although take sometime , these data are very important
getnetstat
getlsof

if [ $SNAPBIG -ge 2 ] ; then
getconfig
getlog
getrun
getlib
fi

# Get files might be too large

if [ $SNAPBIG -ge 3 ] ; then
getcoredumpfiles
fi

if [ $SNAPBIG -ge 4 ] ; then
getmntfiles
fi

getbaseguardlogend

stopcontrolof_storage
cd
umount /blk
mount /blk
cd -
}


