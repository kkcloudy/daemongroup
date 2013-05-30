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

LOGTAG=VARCHK

######################################################

# get scrpit file name
SCRIPT_NAME=${0##*/}

if [ `ps -e | grep -c $SCRIPT_NAME` -gt 2 ] ; then
	logger -t $LOGTAG -p cron.notice "logcheck already running."
	exit 1
fi

debug=0

#logarchive=`cat /var/log/archive 2>/dev/null`
logarchive=0

# Use Maxsize set in mount-movable
if [ -f /var/log/log_maxsize ] ; then
	MAXSIZE=`cat /var/log/log_maxsize 2>/dev/null`
	if [ "$MAXSIZE" = "" -o "$MAXSIZE" = "0" ] ; then
		MAXSIZE=$((64 * 1024 * 1024))
	fi
else
	MAXSIZE=$((64 * 1024 * 1024))
fi

MAX_ARCOUNT=10

# if free memory less than mem_limit (MB)
# we see this as memory leaked
MEM_LIMIT=300

decho()
{
	if [ $debug -eq 1 ] ; then
		echo $*
	fi
}

logger -t $LOGTAG -p cron.notice "MAXSIZE is [$MAXSIZE] and logarchive is [$logarchive]"

rotate_log_archive()
{
	MAX_R=$1
	logrlist=`/usr/bin/sor.sh ls logarchives 30`
	#We use count to check 
	#if [ x"$logrlist" = x"" ] ; then
	#	logger -t $LOGTAG -p cron.notice "No log archives need to be clean."
	#	return 1
	#fi
	count=0
	for eachfile in $logrlist
	do
		count=$(($count + 1))
		if [ $count -eq 1 ] ; then
			cleanfile=$eachfile
		fi
	done

	if [ $count -eq 0 ] ; then
		logger -t $LOGTAG -p cron.notice "No log archives need to be clean."
		return 1
	fi

	if [ $MAX_R -eq 0 ] || [ $count -ge $MAX_R ] ; then
		/usr/bin/sor.sh rm logarchives/$cleanfile 30
		ret=$?
		if [ $ret -eq 0 ] ; then
			logger -t $LOGTAG -p cron.notice "Log archive $cleanfile was cleaned successfully."
			return 0
		else
			logger -t $LOGTAG -p cron.notice "Failed to clean log archive $cleanfile with error code($ret)."
			return 2
		fi
	else
		return 0
	fi
}

archive_log()
{
	if [ x"$logarchive" = x"1" ] ; then
		rotate_log_archive $MAX_ARCOUNT
		ALOG=$1
		archivename=`date +%Y%m%d%H%M%S``basename $ALOG`.tar.bz2
		tar cjf /mnt/logarchives/$archivename $ALOG
		/usr/bin/sor.sh cp logarchives/$archivename 30
		ret=$?
		while [ $ret -eq 5 ]  
		do
			logger -t $LOGTAG -p cron.notice "$ALOG failed archived with error code($ret), Trying to rotate log."
			rotate_log_archive 0
			ret=$?
			if [ ! $ret -eq 0 ] ; then
				/usr/bin/sor.sh cp logarchives/$archivename 30
				ret=$?
			fi 
		done
		if [ $ret -eq 0 ] ; then
			logger -t $LOGTAG -p cron.notice "$ALOG was archived as $archivename successfully."
		else
			logger -t $LOGTAG -p cron.notice "$ALOG failed archived with error code($ret)."
		fi
		rm -rf /mnt/logarchives/$archivename
	fi
}

truncate_log()
{
	logf=$1
	tail -n 100000 $logf > /var/log/templog
	/opt/services/init/syslog_init stop
	mv /var/log/templog $logf
	/opt/services/init/syslog_init start
}

check_and_clean_log()
{
	LOG=$1
	decho "Processing file $LOG"

	LOGSIZE=`ls -l $LOG | awk '{print $5}'`
	decho "LOGSIZE is [$LOGSIZE]"
	if [ $LOGSIZE -ge $MAXSIZE ] ; then
		#	savelog -c 3 $LOG
		logger -t $LOGTAG -p cron.notice "$LOG with size [$LOGSIZE] is larger than max size [$MAXSIZE] will be cleaned."
		logger -t $LOGTAG -p daemon.notice "$LOG with size [$LOGSIZE] is larger than max size [$MAXSIZE] will be cleaned."
		if [ $LOG = "/var/log/syslogservice.log" ] || [ $LOG = "/var/log/system.log" ] ; then
			truncate_log $LOG
		else
			archive_log $LOG
			> $LOG
		fi
	fi

	LOGSIZE=`ls -l $LOG | awk '{print $5}'`
	decho "LOGSIZE is [$LOGSIZE]"
	if [ $LOGSIZE -ge $MAXSIZE ] ; then
		logger -t $LOGTAG -p cron.notice "$LOG still have size [$LOGSIZE], force delete it."
		logger -t $LOGTAG -p daemon.notice "$LOG still have size [$LOGSIZE], force delete it."
		rm -rf $LOG
		if [ "x$LOG" == "x/var/log/system.log" || "x$LOG" == "x/var/log/cron.log" || "x$LOG" == "x/var/log/sudo.log" ] ; then
			if [ -f $LOG ] ; then
				pkill -9 syslog-ng
				rm -rf $LOG
				/opt/services/init/syslog_init start
			else
				/opt/services/init/syslog_init restart
			fi
		fi
	fi
}

checkdir()
{
	for VFILE in $1/*
	do
		if [ -f $VFILE ] ; then
			logger -t $LOGTAG -p cron.info "checking $VFILE."
			check_and_clean_log $VFILE
		elif [ -d $VFILE ] ; then
			checkdir $VFILE
		fi
	done
}

# cron log print
clog_print()
{
	logger -t $LOGTAG -p cron.notice $*
	#logger -t $SCRIPT_NAME -p cron.notice $*
}

# daemon log print
dlog_print()
{
	logger -t $LOGTAG -p daemon.notice $*
	#logger -t $SCRIPT_NAME -p daemon.notice $*
}

# check log file state
# argument 1: log path
check_logfile_state()
{
	clog_print "checking $1 exist state."
	if [ ! -f $1 ] ; then
		clog_print "$1 missed restart syslog-ng."
		touch $1
		/opt/services/init/syslog_init restart
	fi
}

# check syslog-ng running state
check_syslog_state()
{
	# check whether syslog-ng started
	if [ `ps -e | grep -c syslog-ng` -eq 0 ] ; then
		/opt/services/init/syslog_init start
	fi

	# check whether syslog-ng can log
	touch /var/log/system.log
	file_size1=`ls -l /var/log/system.log | awk '{print $5}'`
	dlog_print "checking syslog-ng state."
	touch /var/log/system.log
	file_size2=`ls -l /var/log/system.log | awk '{print $5}'`
	if [ $file_size1 = $file_size2 ] ; then
		clog_print "syslog failed restart syslog-ng."
		touch /var/log/system.log
		/opt/services/init/syslog_init restart
	fi

	syslog_num=`ps -e | grep -c syslog-ng`
	if [ $syslog_num -gt 1 ] ; then
		clog_print "$syslog_num syslog-ng running kill all."
		pkill -9 syslog-ng
		touch /var/log/system.log
		/opt/services/init/syslog_init start
	fi
}

clear_all_file_content_in_dir()
{
	for file in `ls  $1`
	do
		if [ -d $1"/"$file ] ; then
			clear_all_file_content_in_dir $1"/"$file
		else
			local path=$1"/"$file
			echo "logcheck.sh clear file $path" > /dev/console
			echo > $path
		fi
	done
}

check_mem_state()
{
	# get free memory (MB)
	free_mem=`free -m -o | grep Mem | awk '{print $4}'`

	if [ $free_mem -lt $MEM_LIMIT ] ; then
		dlog_print "memory leaked: $free_mem MB memory left."
		pkill -9 syslog-ng
		sync
		echo 2 > /proc/sys/vm/drop_caches
		echo 3 > /proc/sys/vm/drop_caches
		#rm -rf /var/log/*
		clear_all_file_content_in_dir "/var/log"
		rm -rf /mnt/patch/*
		echo $MAXSIZE > /var/log/log_maxsize
		touch /var/log/cron.log
		touch /var/log/sudo.log
		touch /var/log/system.log
		/opt/services/init/syslog_init start
	else
		dlog_print "memory check: $free_mem MB memory left."
	fi
}

# check log file size
checkdir /var/log

# check log file exist state
check_logfile_state /var/log/cron.log
check_logfile_state /var/log/sudo.log
check_logfile_state /var/log/system.log

# check syslog-ng running state
check_syslog_state

# check memory state
check_mem_state

exit 0
