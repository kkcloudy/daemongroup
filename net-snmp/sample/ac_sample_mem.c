#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <syslog.h>
#include <string.h>


#include "ac_sample.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"
#include "ac_sample_mem.h"



#define GET_MEM_STATE  "/proc/meminfo"
#define U64    unsigned long long

static int do_sample_memusage( ac_sample_t *me )
{
    U64 sample = 1;
    U64 totalram, freeram;
    int test = 1;
    struct sysinfo info;
    
    memset( &info, 0, sizeof(info) );
    if( 0 == sysinfo(&info) )
    {
        totalram = (U64)info.totalram;
        freeram = (U64)info.freeram;
        sample = (totalram-freeram)*100/totalram;
    }
        
    return (int)sample;
}



int ac_sample_memusage_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int average=0;
	
	syslog( LOG_INFO, "ac_sample_memusage_send_over_threshold_signal  type=%d\n", type);

	average = sw_get_average( get_sample_window(this) );
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&average,
										 DBUS_TYPE_INVALID );
}



ac_sample_t *create_ac_sample_memusage(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;

	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_MEMUSAGE ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_MEMUSAGE, ret );
	}
	
    pret = create_ac_sample(SAMPLE_NAME_MEMUSAGE, config.sample_interval, config.statistics_time, config.resend_interval );//, do_sample_memusage, NULL );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_memusage );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_cb( pret, ac_sample_memusage_send_over_threshold_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_memusage  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}



