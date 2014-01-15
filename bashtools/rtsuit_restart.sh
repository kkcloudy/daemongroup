#! /bin/sh
### BEGIN INIT INFO
# Provides:          quaggasuit 
# Required-Start:    $local_fs 
# Required-Stop:     $local_fs 
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: quagga 
# Description:       Routing Software Suit
### END INIT INFO


# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=/sbin:/usr/sbin:/bin:/usr/bin:/opt/bin
DESC="Routing Software Suit"
NAME=RTSUIT
SCRIPTNAME=/etc/init.d/$NAME
DAEMONS="rtmd  ospfd ripd"
D_PATH=/opt/bin

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions


rtmd_options=" --daemon -A 127.0.0.1 "
bgpd_options="  --daemon -A 127.0.0.1"
ospfd_options=" --daemon -A 127.0.0.1"
ospf6d_options="--daemon -A ::1"
ripd_options="  --daemon -A 127.0.0.1"
ripngd_options="--daemon -A ::1"
isisd_options=" --daemon -A 127.0.0.1"


pidfile()
{
	echo "/var/run/rtsuit/$1.pid"
}

# Check if daemon is started by using the pidfile.
started()
{
	[ -e `pidfile $1` ] && kill -0 `cat \`pidfile $1\`` 2> /dev/null && return 0
	return 1
}
start()
{
#	echo -n " $1"

#	start-stop-daemon \
#		--start \
#		--pidfile=`pidfile $1` \
#		--exec "$D_PATH/$1" \
#		-- \
#		`eval echo "$""$1""_options"`
	#$D_PATH/$1 1> /var/log/$1.log 2>&1 &
	ulimit -c unlimited
	if [ $1 = "rtmd" ] ; then
		$D_PATH/$1 -k -f /mnt/rtsuit/$1.conf 1> /dev/null 2>&1 &
	else
		$D_PATH/$1 -f /mnt/rtsuit/$1.conf 1> /dev/null 2>&1 &	
	fi
}


stop()
{
	pkill -SIGKILL $1
}


# Stop the daemon given in the parameter, printing its name to the terminal.
stop_old()
{
    if ! started "$1" ; then
#	echo -n " ($1)"
	return 0
    else
	PIDFILE=`pidfile $1`
	PID=`cat $PIDFILE 2>/dev/null`
	start-stop-daemon --stop --quiet --oknodo --exec "$D_PATH/$1"
	rm -rf `pidfile $1`
	# We need not wait for those daemons.
	return 0 
	#
	#       Now we have to wait until $DAEMON has _really_ stopped.
	#
	#
	if test -n "$PID" && kill -0 $PID 2>/dev/null; then
	    echo -n " (waiting) ."
	    cnt=0
	    while kill -0 $PID 2>/dev/null; do
		cnt=`expr $cnt + 1`
		if [ $cnt -gt 60 ]; then
		    # Waited 120 secs now, fail.
		    echo -n "Failed.. "
		    break
		fi
		sleep 2
		echo -n "."
		done
	    fi
#	echo -n " $1"
	rm -f `pidfile $1`
    fi
}


#
# Function that starts the daemon/service
#
do_start()
{
	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	for daemon_name in $DAEMONS; do
		start "$daemon_name"
	done
	return 0
	# Add code here, if necessary, that waits for the process to be ready
	# to handle requests from services started subsequently which depend
	# on this one.  As a last resort, sleep for some time.
}

#
# Function that stops the daemon/service
#
do_stop()
{
	for daemon_name in $DAEMONS; do
		stop "$daemon_name"
	done
	return 0
}

if [ ! -d /tar/run/rtsuit ] ; then
	mkdir -p /var/run/rtsuit
#	chown quagga:quagga /var/run/quagga
#	chmod 755 /var/run/quagga
fi


case "$1" in
  start)
	[ "$VERBOSE" != no ] && log_daemon_msg "Starting $DESC" "$NAME"
	echo "Starting $DESC": "$NAME"
	do_start
	case "$?" in
		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
	esac
	;;
  stop)
	[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $DESC" "$NAME"
	echo "Stoping $DESC": "$NAME"
	do_stop
	case "$?" in
		0|1) [ "$VERBOSE" != no ] && log_end_msg 0 ;;
		2) [ "$VERBOSE" != no ] && log_end_msg 1 ;;
	esac
	;;
  #reload|force-reload)
	#
	# If do_reload() is not implemented then leave this commented out
	# and leave 'force-reload' as an alias for 'restart'.
	#
	#log_daemon_msg "Reloading $DESC" "$NAME"
	#do_reload
	#log_end_msg $?
	#;;
  restart|force-reload)
	#
	# If the "reload" option is implemented then remove the
	# 'force-reload' alias
	#
	log_daemon_msg "Restarting $DESC" "$NAME"
	do_stop
	case "$?" in
	  0|1)
		do_start
		case "$?" in
			0) log_end_msg 0 ;;
			1) log_end_msg 1 ;; # Old process is still running
			*) log_end_msg 1 ;; # Failed to start
		esac
		;;
	  *)
	  	# Failed to stop
		log_end_msg 1
		;;
	esac
	;;
  *)
	#echo "Usage: $SCRIPTNAME {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $SCRIPTNAME {start|stop|restart|force-reload}" >&2
	exit 3
	;;
esac
