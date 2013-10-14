
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "nm_list.h"
#include "ac_sample.h"

#include "ac_sample_container.h"
#include "ac_sample_cpu.h"
#include "ac_sample_mem.h"
#include "ac_sample_tmp.h"
#include "ac_sample_drop_rate.h"
#include "ac_sample_multy.h"
#include "ac_sample_def.h"

#include "ac_sample_interface_flow.h"
#include "ac_sample_portal.h"
#include "ac_sample_radius.h"

#include "ac_manage_def.h"
#include "ac_sample_def.h"

#define SAMPLE_PID_FILE "/var/run/acsample.pid"


#define DEBUS_BLOCK_USECOND         1000

static int main_pid = 0;
static int keep_going = 1;
struct timeval gTimeout;

void static log_pid(char *pidfile) 
{
    FILE *file;
    mode_t oldmask;
    
    oldmask = umask(022);/*modify the file which create leter to 755!*/
    file = fopen(pidfile, "w");
    umask(oldmask);
    if(!file) return;
    fprintf(file, "%d\n", getpid());
    main_pid = getpid ();
    fclose(file);
}


static void termination_handler(int signum) 
{
	syslog(LOG_DEBUG, "SIGTERM received! sample  exit!\n");
	keep_going = 0;
}


int main()
{
	struct sigaction act;
    ac_sample_t *reg_sample=NULL;
	

	
    openlog("acsample", LOG_PID, LOG_DAEMON);
	syslog( LOG_DEBUG, "sample log on!" );
	setlogmask(LOG_AT_LEAST_INFO);

	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
	freopen("/dev/null", "r", stdin);
	if (daemon(1, 1)) 
	{
	  syslog(LOG_ERR,"daemon() failed!");
	}
	
    log_pid(SAMPLE_PID_FILE);	

	memset(&act, 0, sizeof(act));

	act.sa_handler = termination_handler;
    sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);

    init_sample_socket_timeout();
	
    init_sample_container();
	/*cpu*/
    reg_sample = create_ac_sample_cpu( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if( NULL == reg_sample )
	{
		syslog( LOG_CRIT, "create cpu sample failed!  exit!!" );
		return 0;
	}
    register_ac_sample( reg_sample );

	/*mem*/
    reg_sample = create_ac_sample_memusage( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if( NULL == reg_sample )
	{
		syslog( LOG_CRIT, "create memusage sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );

	/*tmp*/
	reg_sample = create_ac_sample_temperature( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if( NULL == reg_sample )
	{
		syslog( LOG_CRIT, "create temperature sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );
	/*dhcp*/
    reg_sample = create_ac_sample_dhcpusage( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if( NULL == reg_sample )
	{
		syslog( LOG_CRIT, "create dhcpusage sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );





#if 0
	/*band_with */
	reg_sample = create_ac_sample_band_with( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create band width sample failed!  exit!!" );
		return 0;
	}	
 	 register_ac_sample( reg_sample );
#endif
	/*drop rate */

	reg_sample = create_ac_sample_drop_rate( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create drop rate sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );

#if 0
	/*max online user*/
	reg_sample = create_ac_sample_max_user( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create max online user sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );
#endif

	/* interface flow data */
	reg_sample = create_ac_sample_interface_flow(DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME);
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create interface flow sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );
	

	/*find syn attack */
	reg_sample = create_ac_sample_find_attack( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create syn attack sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );
    
#if 1   
    /*radius req success*/
    reg_sample = create_ac_sample_radius_req( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
    if (NULL==reg_sample)
    {
        syslog( LOG_CRIT, "create radius request sample failed!  exit!!" );
        return 0;
    }   
    register_ac_sample( reg_sample );
#endif
    
	/*portal server status 	*/
	reg_sample = create_ac_sample_portal_server( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create portal server sample failed!  exit!!" );
		return 0;
	}	
	
    register_ac_sample( reg_sample );

	/*cpu real time status 	*/
	reg_sample = create_ac_sample_cpu_real_time_stat( DEFAULT_SAMPLE_INTERVAL_CPURT, DEFAULT_STATISTICS_TIME_CPURT );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create cpu real time sample failed!  exit!!" );
		return 0;
	}	
	
    register_ac_sample( reg_sample );







	/*radius auth server status 	*/
	reg_sample = create_ac_sample_radius_auth( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create radius auth server sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );


	/*radius acct server status 	*/
	reg_sample = create_ac_sample_radius_count( DEFAULT_SAMPLE_INTERVAL, DEFAULT_STATISTICS_TIME );
	if (NULL==reg_sample)
	{
		syslog( LOG_CRIT, "create radius count server sample failed!  exit!!" );
		return 0;
	}	
    register_ac_sample( reg_sample );

    
    /*init dbus*/
    ac_sample_dbus_init();
	ac_sample_tipc_dbus_init();
	sample_tipc_session_init();
	

	dl_dcli_init();
	
	/*main loop*/
    while( keep_going )
    {
        /*do sample*/
#if 0        
        struct timeval  start,end;
static unsigned long long time_ms = 0;
        gettimeofday( &start, NULL );
#endif        
        ac_sample_dispach();
#if 0        
        gettimeofday( &end, NULL );
        time_ms = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec; 
        printf( "time_ms = %llu\n", time_ms );
#endif        
        /*dispach dbus*/
        ac_sample_dbus_dispach( DEBUS_BLOCK_USECOND );
    }
	sample_tipc_session_destroy();
	uninit_sample_container();

    return 0;
}













