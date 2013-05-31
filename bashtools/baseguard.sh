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

. /usr/bin/libsnapshot.sh

. /usr/bin/libdbuscheck.sh
. /usr/bin/libcritlog.sh

guardlog "Started."

INTERVAL=60
NORMAL=1
CHECKTIMES=10
check_mem_s()
{
FREEMEM=`free | grep Mem | awk '{ print $4 }'`
FREEPER=$(($FREEMEM * 100 / $TOTALMEM))
if [ $FREEPER -lt 8 ] ; then
        echo "System reboot for $CRITMSG" > /dev/console
        echo 2 > /blk/softreboot
        reboot
fi
}


final_job()
{
	NORMAL=0
	guardlog $CRITMSG
	echo $CRITMSG > /dev/console
	shutdown_hansi_heartbeat
	take_snapshot 1 3
	if [ $# -eq 1 ]; then
		check_mem_s
		NORMAL=1
	else
		echo "System reboot for $CRITMSG" > /dev/console
		echo 2 > /blk/softreboot
# For development version, it is not necessary to reboot system. 
		reboot
	fi
}

shutdown_hansi_heartbeat()
{
	id=1
	while [ $id -le 16 ] ; do
		if test -d /var/run/had/had$id ; then
        		heartbeat=`cat /var/run/had/had$[id]/heartbeat`
        		state=`cat /var/run/had/had$[id]/state`

        		if [ $state = "MASTER" ] ; then
                		echo "hansi $[id] state take over by other side..." > /dev/console
                		ifconfig $heartbeat down
        		fi
		fi
		id=$[$id+1]
	done
}

# for wid wsm asd process
check_one_process_by_wwa()
{
        id=1
        while [ $id -le 16 ] ; do
	    if test -d /var/run/had/had$id ; then
#               echo "check hansi $id key process..." >> /var/log/baseguard.log
                hadcnt=`ps aux | grep "had $id$" | grep -v grep -wc`
                if [ $hadcnt -gt 1 ] ; then
#			echo "  hansi $id is running $hadcnt" >> /var/log/baseguard.log
                        kpcnt=`ps aux | grep "$1 $id$" | grep -v grep -wc`
# 			echo "  $1 $id is running $kpcnt" >> /var/log/baseguard.log
                        if [ $kpcnt -lt 2 ] ; then
                                CRITMSG="CRITICAL : Process $1 $id not found."
#                               echo "CRITICAL : Process $1 $id not found." >> /var/log/baseguard.log
                                final_job
#                        else
#                                echo "  $1 $id is running $kpcnt" >> /var/log/baseguard.log
                        fi
                fi
	    fi
            id=$[$id+1]
        done
}

check_one_process_by_name()
{
ps -e | grep $1 
ret=$?
if [ ! $ret -eq 0 ] ; then
		CRITMSG="CRITICAL : Process $1 not found."
		final_job
fi
}

check_key_process()
{
#check_one_process_by_name wid
#check_one_process_by_name wsm
#check_one_process_by_name asd
check_one_process_by_name npd
#check_one_process_by_name rtmd 
check_one_process_by_name sem 
check_one_process_by_name dbus-daemon
}

check_rebootable_process()
{
	nothing_to_do="nothing to do"
}

mem_info_collect()
{
D_PATH=/var/log/$1
if [ -d $D_PATH ] ; then
	nothing_to_do="nothing to do"
else
	mkdir $D_PATH
fi
guardlog "mem info collect $1%"
ps -e -o pid,f,stat,rss=RESID-MEM-in-KiB,size=SWAP-SPACE,vsize=VIRT-MEM-in-KiB,%mem,ruser,euser,fuser,suser,cmd --sort=-rss> /var/log/$1/ps_mem
cp /proc/slabinfo /var/log/$1/
free -blt > /var/log/$1/free
date > /var/log/$1/timeinfo
uptime >> /var/log/$1/timeinfo
du -h --max-depth=1 --exclude=/proc --exclude=/sys --exclude=/blk / > /var/log/$1/du
df -h > /var/log/$1/df
sync
echo 3 > /proc/sys/vm/drop_caches
}

TOTALMEM=`free | grep Mem | awk '{ print $2 }'`
MEM10FIRST=1
MEM12FIRST=1
MEM15FIRST=1
check_mem()
{
#sh-3.1# free
#             total       used       free     shared    buffers     cached
#Mem:       1991476     622384    1369092          0          0     212916
#-/+ buffers/cache:     409468    1582008
#Swap:            0          0          0
# Use result -/+ buffers/cache which is more accurate.
FREEMEM=`free | grep Mem | awk '{ print $4 }'`
#FREEMEM=`free | grep + | awk '{ print $4 }'`
FREEPER=$(($FREEMEM * 100 / $TOTALMEM))
if [ $FREEPER -lt 8 ] ; then
	CRITMSG="CRITICAL: Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
	final_job mem
elif [ $FREEPER -lt 10 ] ; then
	CHECKTIMES=1
	pkill -9 snmp
	if [ $MEM10FIRST -eq 1 ] ; then
		guardlog "Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
		mem_info_collect 10
	fi
	MEM10FIRST=0
	MEM12FIRST=1
	MEM15FIRST=1
	CHECKTIMES=1
elif [ $FREEPER -lt 12 ] ; then
        if [ $MEM12FIRST -eq 1 ] ; then
		guardlog "Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
                mem_info_collect 12
        fi
	MEM10FIRST=1
	MEM12FIRST=0
	MEM15FIRST=1
	CHECKTIMES=3
elif [ $FREEPER -lt 15 ] ; then
        if [ $MEM15FIRST -eq 1 ] ; then
		guardlog "Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
                mem_info_collect 15
        fi
	MEM10FIRST=1
	MEM12FIRST=1
	MEM15FIRST=0
	CHECKTIMES=5
else
	if [ $MEM10FIRST -eq 0 ] || [ $MEM12FIRST -eq 0 ] || [ $MEM15FIRST -eq 0 ]; then
                guardlog "Freemem $FREEMEM/$TOTALMEM=$FREEPER%."
        fi
	MEM10FIRST=1
	MEM12FIRST=1
	MEM15FIRST=1	
	CHECKTIMES=10
fi 
#guardlog "Checking memory" 
# Kernel Out-of-Memory Killer will work firstly.
# System will reboot if wid/wsm/asd/npd was killed.
# If kernel used too much memeory, baseguard should work.
}

MIN_FDNUM=1024
FDMAX=`cat /proc/sys/fs/file-max`

check_fd()
{
#FDNUM=`lsof -nP | wc -l`
FDNUM=`cat /proc/sys/fs/file-nr | awk '{ print $1 }'`
FDLEFT=$(($FDMAX - $FDNUM))

if [ $FDLEFT -lt 1024 ] ; then
CRITMSG="CRITICAL: Opened too much fd [$FDNUM] , System max fd is [$FDMAX], less than [$MIN_FDNUM] left."
final_job
fi 
}

CONNMAX="/proc/sys/net/ipv4/netfilter/ip_conntrack_max"
CONNCUR="/proc/sys/net/ipv4/netfilter/ip_conntrack_count"
CONNTCPTIMEOUT="/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established"

CONNTRACK_COUNTER=0
CONNTRACK_COUNTER_MAX=2400

check_conntrack()
{
if [ -f $CONNMAX ] ; then
CONNMAX_V=`cat $CONNMAX`
CONNCUR_V=`cat $CONNCUR`
CONNTCPTIMEOUT_V=`cat $CONNTCPTIMEOUT`
CONNUSED=$(($CONNCUR_V * 100 / $CONNMAX_V))
if [ $CONNTCPTIMEOUT_V -ge 1200 ] ; then
	if [ $CONNUSED -ge 50 ] ; then
		guardlog "ip_conntrack used $CONNCUR_V/$CONNMAX_V = $CONNUSED%, trying to set tcp_timeout_established from $CONNTCPTIMEOUT_V to 120"
		echo 120 > $CONNTCPTIMEOUT
		guardlog "first clear conntrack count: $CONNCUR_V"
		conntrack -F
		guardlog "New tcp_timeout_established value is `cat $CONNTCPTIMEOUT`"
	fi
else
	#if [ $CONNUSED -ge 60 -a $CONNUSED -lt 90 ] ; then
	if [ $CONNUSED -ge 60 ] ; then
		guardlog "clear conntrack count:$CONNCUR_V : $CONNCUR_V/$CONNMAX_V = $CONNUSED%"
		conntrack -F
	fi
	
	#if [ $CONNUSED -lt 60 ] ; then
	#	CONNTRACK_COUNTER=0
	#fi

	#cann't clear conntrack table
	if [ $CONNUSED -ge 90 ] ; then
		CONNTRACK_COUNTER=$(($CONNTRACK_COUNTER + 1))
		guardlog "ip_conntrack used $CONNCUR_V/$CONNMAX_V=$CONNUSED%. $CONNTRACK_COUNTER"
		if [ $CONNTRACK_COUNTER -ge $CONNTRACK_COUNTER_MAX ] ; then
			CRITMSG="CRITICAL: ip_conntrack is greater than 90% in $CONNTRACK_COUNTER minutes restart needed."
			final_job
		fi
	else
		CONNTRACK_COUNTER=0
	fi
fi
fi

}

check_dbus()
{

if check_asddbus ; then 
	CRITMSG="CRITICAL: asd dbus timeout."
	final_job
fi

if check_widdbus ; then 
	CRITMSG="CRITICAL: wid dbus timeout."
	final_job
fi

}

check_npd()
{
#NPDCOUNT=`ps -e | grep npd | wc -l`
#if [ ! $NPDCOUNT -ge 30 ] ; then
ps -e | grep -v dhc | grep npd 
ret=$?
if [ ! $ret -eq 0 ] ; then
	CRITMSG="CRITICAL : Process npd not found."
	crit_syslog $CRITMSG
	final_job
fi
}
check_rtmd()
{
#cmdstr="show debugging rtm"
#if vtysh -c "$cmdstr" ; then
#	return 1;
#else
#	return 0;
#fi
	return 0;
}

check_rtsuit()
{
ps -ef | grep rtmd | grep -v grep
result_rtmd=$?
if [ $result_rtmd -eq 0 ];then
	if [ ! 0 ] ; then
		guardlog "Rtmd restart"
		/usr/bin/rtsuit_restart.sh restart
#rtmd restart then vtysh will restart too
		pkill vtysh
	fi
else
	guardlog "Rtmd restart"
 	/usr/bin/rtsuit_restart.sh restart
	pkill vtysh
fi

}

syn_date()
{
	is_active_master=`cat /dbm/local_board/is_active_master`
	if [ $is_active_master == 1 ] ; then
		echo "is avtve master return"
		return -1
	fi
	active_slot=`cat /dbm/product/active_master_slot_id`
	/usr/sbin/ntpdate -d 169.254.1.$active_slot > /dev/null 2> /dev/null
}

check_file_system()
{
	ls /mnt/ | grep "^Temp.*\.conf$" > /dev/null
	result_tempconf=$?
	if [ $result_tempconf -eq 0 ] ; then
		Tempconf_num=`ls /mnt/Temp*.conf | wc -l | awk '{ print $1 }'`
		if [ $Tempconf_num -gt 5 ] ; then
#echo "Too many temp conf file."
			rm /mnt/Temp*.conf
		fi
	fi
}

drop_mem_caches()
{
	#clean the caches
	echo 3 > /proc/sys/vm/drop_caches
}
check_process_mem_by_name()
{
	for pid in $(ps -e | grep "$1" | awk '{print $1}')
	do
		mem_use=`/usr/bin/mem.sh $pid`
		#echo $1 $pid $mem_use
		if [ $mem_use -gt $2 ] ; then
			if [ $3 == 1 ] ; then
				crit_syslog "process $1 $pid memory use: $mem_use large than $2 kill it."
				kill $pid
			else
				crit_syslog "process $1 $pid memory use: $mem_use large than $2 waitting for check."
			fi
		fi
	done
}
check_process_mem()
{
	#check_process_mem_by_name npd 100 1
	check_process_mem_by_name telnet 10 1

}

runf()
{
if [ $NORMAL = "1" ] ; then
	eval $1
fi
}

#*********route cache **********

DEFAULT_NET_CACHE_GC_INTERVAL=`cat /proc/sys/net/ipv4/route/gc_interval`
NET_CACHE_GC_INTERVAL=$DEFAULT_NET_CACHE_GC_INTERVAL
NET_CACHE_GC_MIN_INTERVAL=`cat /proc/sys/net/ipv4/route/gc_min_interval`

check_nat_cache()
{
#cat /proc/slabinfo | grep ip_dst_cache
# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail>
#ip_dst_cache          65     90    384   10    1 : tunables   54   27    8 : slabdata      9      9      0
rt_maxrat1=60
rt_maxrat2=80
maxrat=100
#echo "default route cache maxrat is $maxrat%" >> /var/log/baseguard.log
nat_maxsize=`cat /proc/sys/net/ipv4/route/max_size`
#echo "route cahce maxsize is $nat_maxsize Byte" >> /var/log/baseguard.log
dstobj_num=`cat /proc/slabinfo | grep ip_dst_cache | awk '{ print $2 }'`
#echo "dstobj_num: $dstobj_num" >> /var/log/baseguard.log
nat_cache_rat=$(($dstobj_num * 100 / $nat_maxsize))
#echo "route cache use rate is $nat_cache_rat%" >> /var/log/baseguard.log

if [ $nat_cache_rat -ge $maxrat ] ; then 
	CRITMSG="CRITICAL: Route cache is $nat_cache_rat%."
	echo "CRITICAL: Route cache is $nat_cache_rat%." >> /var/log/baseguard.log
	final_job
elif [ $nat_cache_rat -ge $rt_maxrat2 ] ; then 
  NET_CACHE_WARRNING=1
	ip route flush cach
  NET_CACHE_GC_INTERVAL=$(($NET_CACHE_GC_INTERVAL / 2))
  if [ $NET_CACHE_GC_INTERVAL -le  10 ] ; then
    NET_CACHE_GC_INTERVAL=10
  fi
	echo $NET_CACHE_GC_INTERVAL > /proc/sys/net/ipv4/route/gc_interval
	echo "WARNING: Route cache is $nat_cache_rat%, flush route cache." >> /var/log/baseguard.log
	echo "WARNING: Dec route cache garbage collect interval to $NET_CACHE_GC_INTERVAL s." >> /var/log/baseguard.log
elif [ $nat_cache_rat -ge $rt_maxrat1 ] ; then 
	echo "WARNING: Route cache is $nat_cache_rat%, Flush route cache." >> /var/log/baseguard.log	
	ip route flush cach
else
  if [ $NET_CACHE_WARRNING -ne 0 ] ; then
    NET_CACHE_WARRNING=0
    NET_CACHE_GC_INTERVAL=$DEFAULT_NET_CACHE_GC_INTERVAL
    echo $NET_CACHE_GC_INTERVAL > /proc/sys/net/ipv4/route/gc_interval
    echo "DEBUG: Renew route cache garbage collect interval to $NET_CACHE_GC_INTERVAL s." >> /var/log/baseguard.log
  fi
fi
#echo "check route cache rate success." >> /var/log/baseguard.log
}

#*********arp cache **********

ARP_MAXSIZE=`cat /proc/sys/net/ipv4/neigh/default/gc_thresh3`
DEFAULT_ARP_BASEREACHABLE_TIME=`cat /proc/sys/net/ipv4/neigh/default/base_reachable_time`
ARP_BASEREACHABLE_TIME=$DEFAULT_ARP_BASEREACHABLE_TIME

check_arp_cache()
{
#cat /proc/slabinfo | grep arp_cache
# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail>
#arp_cache              2     30    256   15    1 : tunables  120   60    8 : slabdata      2      2      0
arp_maxrat=80
maxrat=100
arpobj_num=`cat /proc/slabinfo | grep arp_cache | awk '{ print $2 }'`
#echo "arpobj_num: $arpobj_num" >> /var/log/baseguard.log
arp_cache_rat=$(($arpobj_num * 100 / $ARP_MAXSIZE))
#echo "arp cache use rate is $arp_cache_rat%" >> /var/log/baseguard.log

if [ $arp_cache_rat -ge $maxrat ] ; then 
	CRITMSG="CRITICAL: arp cache is $arp_cache_rat%."
	echo "CRITICAL: arp cache is $arp_cache_rat%." >> /var/log/baseguard.log
  final_job
elif [ $arp_cache_rat -ge $arp_maxrat ] ; then 
  ARP_CACHE_WARRNING=1
  ARP_BASEREACHABLE_TIME=$(($ARP_BASEREACHABLE_TIME / 2))
  if [ $ARP_BASEREACHABLE_TIME -le  150 ] ; then
    ARP_BASEREACHABLE_TIME=150
  fi
  echo $ARP_BASEREACHABLE_TIME > /proc/sys/net/ipv4/neigh/default/base_reachable_time
  echo "WARNING: Dec arp stale time to $ARP_BASEREACHABLE_TIME s." >> /var/log/baseguard.log
  
else
  if [ $ARP_CACHE_WARRNING -ne 0 ] ; then
    ARP_CACHE_WARRNING=0
    ARP_BASEREACHABLE_TIME=$DEFAULT_ARP_BASEREACHABLE_TIME
    echo $ARP_BASEREACHABLE_TIME > /proc/sys/net/ipv4/neigh/default/base_reachable_time
    echo "DEBUG: Renew stale time to $ARP_BASEREACHABLE_TIME s." >> /var/log/baseguard.log
  fi
fi
#echo "check arp cache rate success." >> /var/log/baseguard.log
}



renew_interval()
{
  RT_INTERVAL=3
  MAX_CTIME=20
  echo "DEBUG: Renew baseguard interval to $(($RT_INTERVAL * $MAX_CTIME)) s." >> /var/log/baseguard.log
}

dec_interval()
{
  if [ $RT_INTERVAL -gt 1 ] ; then 
	  RT_INTERVAL=$(($RT_INTERVAL - 1))
	elif [ $MAX_CTIME -gt 1 ] ; then
	  MAX_CTIME=$(($MAX_CTIME - 1))
	fi
	echo "DEBUG: Dec baseguard interval to $(($RT_INTERVAL * $MAX_CTIME)) s." >> /var/log/baseguard.log
}

ARP_CACHE_WARRNING=0
NET_CACHE_WARRNING=0

check_warrning()
{
  if [ $ARP_CACHE_WARRNING -ne 0 -o $NET_CACHE_WARRNING -ne 0 ] ; then
    runf dec_interval
  elif [ $RT_INTERVAL -ne 3 -o $MAX_CTIME -ne 20 ] ; then
    runf renew_interval
  fi
}

COREMAXSIZE=$((20 * 1024 * 1024))

check_core_dump()
{
  core_files="`ls -A /opt/bin/core`"
  cmd_file="/usr/bin/gen_dump"
  
  if [ "${core_files}" ] ; then
    if [ ! -e ${cmd_file} ] ; then 
      echo "bt" > ${cmd_file}
    fi
    sudo mount /blk
    for file in ${core_files}
    do
      suffix="`echo ${file#*.}`"
      if [ ${suffix} = "core" ] ; then
        obj="`echo ${file%_*}`"
        gdb /opt/bin/${obj} /opt/bin/core/${file} -batch --command=${cmd_file} > /blk/${file}.txt
        #CORESIZE=`ls -l /opt/bin/core/${file} | awk '{print $5}'`
        #if [ $CORESIZE -lt $COREMAXSIZE ] ; then
          cp /opt/bin/core/${file} /blk
          sync
        #fi
        rm -rf /opt/bin/core/${file}
        echo "DEBUG: rm /opt/bin/core/${file}." >> /var/log/baseguard.log
      fi
    done
    sudo umount /blk
  fi
}

MAXFILESIZE=$((64 * 1024 * 1024))
MAXDIRSIZE=$((160 * 1024 * 1024))
check_file_and_dir()
{
  for FILES in `ls -A $1`
	do
	  if [ "$1" = "/" ] ; then
	    VFILE="/$FILES"
	  else
	    VFILE="$1/$FILES"
	  fi
	  	  
		if [ -f $VFILE ] ; then
		  FILESIZE=`du -sb $VFILE | awk '{print $1}'`
		  if [ $FILESIZE -gt $MAXFILESIZE ] ; then
  			echo "DEBUG: checking $VFILE with size [$FILESIZE] is larger than max FILE size [$MAXFILESIZE]." >> /var/log/baseguard.log
			fi
		elif [ -d $VFILE ] ; then
		  if [ "x$VFILE" = "x/proc" ] || [ "x$VFILE" = "x/sys" ] || [ "x$VFILE" = "x/blk" ] \
		    || [ "x$VFILE" = "x/dev" ] || [ "x$VFILE" = "x/devinfo" ] || [ "x$VFILE" = "x/lib" ] || [ "x$VFILE" = "x/lib64" ] \
		    || [ "x$VFILE" = "x/bin" ] || [ "x$VFILE" = "x/sbin" ]  || [ "x$VFILE" = "x/usr" ] \
		    || [ "x$VFILE" = "x/var/log" ]|| [ "x$VFILE" = "x/opt/bin" ] ; then
		    continue
		  fi
		  DIRSIZE=`du -sb $VFILE | awk '{print $1}'`
		  if [ $DIRSIZE -lt $MAXFILESIZE ] ; then
		    continue
		  elif [ $DIRSIZE -gt $MAXDIRSIZE ] ; then
			  echo "DEBUG: checking $VFILE with size [$DIRSIZE] is larger than max DIR size [$MAXDIRSIZE]." >> /var/log/baseguard.log
		  fi
		  check_file_and_dir $VFILE
		fi
	done
}

RT_INTERVAL=3
RTMD_CHECKED_TIMES=0
CTIME=0
MAX_CTIME=20
CTIME_5MIN=0
CTIME_MEMCHECK=0;
INTERVAL=3

while [ $NORMAL = "1" ]  
do
CTIME=$(($CTIME + 1 ))
CTIME_5MIN=$(($CTIME_5MIN + 1))
CTIME_MEMCHECK=$(($CTIME_MEMCHECK + 1))
RTMD_CHECKED_TIMES=$(($RTMD_CHECKED_TIMES + 1 ))
#check_npd
if [ $CTIME_MEMCHECK -eq $CHECKTIMES ] ; then
	runf check_mem
	CTIME_MEMCHECK=0
fi

if [ $CTIME -eq $MAX_CTIME ] ; then	
	runf check_npd
#	runf check_mem	
	runf check_key_process
	runf check_rebootable_process
	runf check_fd
	runf check_conntrack
	runf check_file_system
#	runf syn_date
	runf check_process_mem	
	runf check_nat_cache
	runf check_arp_cache
	
  runf check_warrning
  runf check_core_dump
  runf check_file_and_dir /
	CTIME=0
fi
if [ $RTMD_CHECKED_TIMES -eq 30 ] ; then	
	runf check_rtsuit
	RTMD_CHECKED_TIMES=0
fi
if [ $CTIME_5MIN -eq 100 ] ; then
#runf drop_mem_caches
	CTIME_5MIN=0
fi

	sleep ${RT_INTERVAL}
	#read -t ${RT_INTERVAL}
done
