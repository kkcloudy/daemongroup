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
#include "ac_sample_dhcp.h"
#include "ws_dcli_dhcp.h"


static int do_sample_dhcpusage( ac_sample_t *me )
{
    int  sample = 1;
    int  totalram = 0, activeram = 0;
    int test = 1;

	int retu = 0;
	int iprate = 0;
	unsigned int lease_count = 0;
	unsigned int lease_free = 0;
	struct lease_state  total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];
	unsigned int subnet_num = 0;	
	int i  = 0;
	int active_tnum = 0;
	int total_tnum = 0;

	memset(&total_state, 0, sizeof(struct lease_state));
	for(i = 0; i < MAX_SUB_NET; ++i)
	{
		memset(sub_state, 0, sizeof(struct sub_lease_state));
	}
	DBusConnection *connection = NULL;
	connection = ac_sample_dbus_get_connection();
	retu = ccgi_show_lease_state_slot(&total_state, sub_state, &subnet_num,connection);
	if(1 == retu)
	{
		totalram = total_state.total_lease_num;
	    //freeram = total_state.free_lease_num;
		activeram = total_state.active_lease_num;
	    sample = activeram*100/totalram;
	}
	else
	{
		sample = 0;
	}
    return (int)sample;
}

static int do_sample_alldhcpusage( ac_sample_t *me )
{
    int  sample = 1;
    int  totalram = 0, activeram = 0;
    int test = 1;

	int retu = 0;
	int iprate = 0;
	unsigned int lease_count = 0;
	unsigned int lease_free = 0;
	struct lease_state  total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];
	unsigned int subnet_num = 0;	
	int i  = 0;

	memset(&total_state, 0, sizeof(struct lease_state));
	for(i = 0; i < MAX_SUB_NET; ++i)
	{
		memset(&sub_state[i], 0, sizeof(struct sub_lease_state));
	}
	DBusConnection *connection = NULL;
	for(i=1;i<11;i++)
	{
		connection = ac_sample_dbus_get_slotconnection(i);
		if(NULL != connection)
		{
			retu = ccgi_show_lease_state_slot(&total_state, sub_state, &subnet_num,connection);
			if(1 == retu)
			{
				totalram += total_state.total_lease_num;
			    //freeram = (U64)total_state.free_lease_num;
				activeram += total_state.active_lease_num;
			}
		}
	}
	if(0 != totalram)
	{
		sample = activeram*100/totalram;
	}
	else
	{
		sample = 100;
	}
    return (int)sample;
}

int ac_sample_dhcpusage_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int average=0;
	
	syslog( LOG_INFO, "ac_sample_dhcpusage_send_over_threshold_signal  type=%d\n", type);

	average = sw_get_average( get_sample_window(this) );
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&average,
										 DBUS_TYPE_INVALID );
}


ac_sample_t *create_ac_sample_dhcpusage(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;

	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_DHCPUSE) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_DHCPUSE, ret );
	}
	
    pret = create_ac_sample(SAMPLE_NAME_DHCPUSE, config.sample_interval, config.statistics_time, config.resend_interval );//, do_sample_memusage, NULL );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_alldhcpusage );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_cb( pret, ac_sample_dhcpusage_send_over_threshold_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_dhcpsusage  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}
