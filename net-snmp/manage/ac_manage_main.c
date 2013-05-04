#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <syslog.h>

#include <signal.h>

#include <dbus/dbus.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include <pthread.h>
#include "board/board_define.h"

#include "ws_init_dbus.h"
#include "ws_snmpd_engine.h"

#include "manage_log.h"
#include "manage_type.h"
#include "manage_api.h"

#include "ac_manage_def.h"
#include "ac_manage_dbus.h"
#include "ac_manage_snmp_config.h"
#include "ac_manage_session.h"
#include "ac_manage_ntpsyslog.h"

#define AC_MANAGE_PID_FILE      "/var/run/acmanage.pid"
#define AC_MANAGE_STATE_FILE    "/var/run/acmanage.state"

#define AC_MANAGE_DBM_FLAG_TIME	60

//static int main_pid = 0;
static int manage_running = 1;

static void 
set_deamon_state(char *file, unsigned int state) {
	if(NULL == file) {
		return ;
	}
	FILE *fp = NULL;
	mode_t oldmask;

	oldmask = umask(022);/*modify the file which create leter to 755!*/
	fp = fopen(file, "w");
	umask(oldmask);

	if(NULL == fp) {
		manage_log(LOG_WARNING, "open file %s failed!", file);
		return ;
	}
	fprintf(fp, "%d", state);

	fclose(fp);
	return ;
}

static int 
get_deamon_state(char *file) {
	if(NULL == file) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	int state = 0;
	FILE *fp = NULL;

	fp = fopen(file, "r");
	if(NULL == fp) {
		manage_log(LOG_WARNING, "open file %s failed!", file);
		return 0;
	}

	if(fscanf(fp, "%d", &state) <= 0) {
		manage_log(LOG_WARNING, "fscanf %s error\n", file);
		fclose(fp);	
		return 0;
	}

	fclose(fp); 
	return state;
}


static void  
log_pid(char *file) {
    if(NULL == file)
        return ;
        
    FILE *fp = NULL;
    mode_t oldmask;
    
    oldmask = umask(022);/*modify the file which create leter to 755!*/
    fp = fopen(file, "w");
    umask(oldmask);

    if(NULL == fp) {
        manage_log(LOG_WARNING, "open file %s failed!", file);
        return ;
    }
    fprintf(fp, "%d\n", getpid());
//    main_pid = getpid ();
    
    fclose(fp);
    return ;
}

static void 
_termination_handler(int signum) {
	manage_log(LOG_INFO, "SIGTERM received! acmanage  exit!\n");
	manage_running = 0;
	
	set_deamon_state(AC_MANAGE_STATE_FILE, 0);
	manage_log(LOG_INFO, "set acmanage service disable state\n");
	
	return ;
}

static void
_thread_attribute_init() {
	pthread_attr_t attr;
	int op = PTHREAD_CREATE_JOINABLE;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, op);

	return ;
}

static void *
_dbus_thread(void) {
	manage_log(LOG_INFO, "_dbus_thread: PID %d\n", getpid());
	while(manage_running 
		&& dbus_connection_read_write_dispatch(ac_manage_dbus_connection, 500)) {
	}

	return NULL;
}

static void *
_task_thread(void) {
	manage_log(LOG_INFO, "_task_thread: PID %d\n", getpid());
	manage_task_process();
	
	return NULL;
}


static void
manage_deamon(void) {
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
	freopen("/dev/null", "r", stdin);
	
	if(daemon(1, 1)) {
		manage_log(LOG_ERR, "daemon() failed!");
		exit(-1);	
	}

	return ;
}

static void
manage_init_log(void) {
	openlog("acmanage", LOG_PID, LOG_DAEMON);
	manage_log(LOG_INFO, "acmanage log on!\n" );
	setlogmask(LOG_ON_INFO);
	manage_debug_config(MANAGE_LOG_DEBUG_ON);
}

static void
manage_shutdown_log(void) {
	closelog();
}

static void
manage_attribute_init(void) {
	_thread_attribute_init();
}

static void
manage_library_init(void) {
	dl_dcli_init();
}

static void
manage_singal_init(void) {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler = _termination_handler;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
}


static void
manage_config_init(void) {
	init_snmp_config();
	init_trap_config();
	init_ntp_config();
}

static void
manage_dbus_init(void) {
	pthread_t thread_dbus;

	if(AC_MANAGE_SUCCESS != init_ac_manage_dbus()) {
		manage_log(LOG_WARNING, "acmanage service enable failed!!!\n");
		exit(-1);
	}

	if(pthread_create(&thread_dbus, NULL, (void *(*) (void *))_dbus_thread, NULL)) {
		manage_log(LOG_NOTICE,"An error occurs when create dbus thread!");
		exit(-1);
	}
	
	manage_log(LOG_NOTICE,"create dbus thread success!");
	return ;
}
static void
manage_flag_init(void) 
{
	int i = 0;
	while(VALID_DBM_FLAG != get_dbm_effective_flag())
		{
				i++;
				if(AC_MANAGE_DBM_FLAG_TIME == i){
					break;
				}
				sleep(2);
		}
	manage_log(LOG_INFO,"acmanage service get_dbm_effective_flag:%d\n",i);
}

static void
manage_init(void) {
	manage_init_log();
	manage_deamon();
	manage_flag_init();
	manage_attribute_init();	
	manage_library_init();
	manage_singal_init();
	init_manage_tipc_dbus();
	manage_config_init();
	manage_dbus_init();
	manage_session_init();
	manage_task_init();

	return ;
}


static void
manage_process(void) {
	pthread_t thread_task;
	
	if(pthread_create(&thread_task, NULL, (void *(*) (void *))_task_thread, NULL)) {
		manage_log(LOG_NOTICE,"An error occurs when create task thread!");
		exit(-1);
	}
	
	manage_log(LOG_NOTICE,"create task thread success!");
	return ;
}


static int
manage_receive(void) {
	int	numfds;
	fd_set readfds;
	struct timeval  timeout, *tvp;
	int	count, block;

	while(manage_running) {

		tvp = &timeout;
		manage_select_info(&numfds, &readfds, tvp, &block);
		if(block) {
			tvp = NULL;
		}
		
	reselect:	
		manage_log(LOG_DEBUG, "manage select .........\n");
		count = select(numfds, &readfds, NULL, NULL, tvp);
		manage_log(LOG_DEBUG, "manage select: count = %d\n", count);
		if (count > 0) {
			manage_read(&readfds);
		} else switch(count) {
			case 0:
				//manage_timeout();
				break;
				
			case -1:
				manage_log(LOG_INFO, "manage select , errno = %d\n", errno);
				if (errno == EINTR) {
					/*
					* likely that we got a signal. Check our special signal
					* flags before retrying select.
					*/
					if(manage_running) {
						goto reselect;
					}
					continue;
				} 
				
				return -1;
				
			default:
				manage_log(LOG_ERR, "manage select returned %d\n", count);
				return -1;
		}	
	}

	manage_log(LOG_INFO, "Received TERM or STOP signal...  shutting down...\n");
	return 0;
}

static void
manage_destory(void) {
	manage_session_destroy();
	free_manage_tipc_dbus();
	manage_shutdown_log();

	exit(0);
}

int
main(void) {
    /*
      * lixiang modify at 2012-02-06
      * if acmanage exit, cron will start it.
      */
#if 0    
    if(get_deamon_state(AC_MANAGE_STATE_FILE)) {
        manage_log(LOG_WARNING, "Ac manage is already running!\n" );
        return -1;
    }
#endif

	manage_init();
//	manage_log(LOG_INFO,"rety = %d\n",rety);

	log_pid(AC_MANAGE_PID_FILE);

	set_deamon_state(AC_MANAGE_STATE_FILE, 1);
	
	manage_register();
	manage_log(LOG_INFO, "acmanage service enable, start process data...\n");
	manage_process();
	manage_receive();

	manage_log(LOG_INFO,"acmanage service disable...\n");
	manage_destory();
	
	return 0;
}
