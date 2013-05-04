#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_tmp.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"
#include "dbus/npd/npd_dbus_def.h"

int do_sample_temperature( ac_sample_t *me )
{
	DBusConnection *conn;
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char fan_power=0;
	unsigned short core_tmprt=0;
	unsigned short surface_tmprt=0;
	int ret = 0;
	conn = ac_sample_dbus_get_connection();

	if( NULL == conn )
	{
		return 0;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_OBJPATH,
											NPD_DBUS_INTERFACE,
											NPD_DBUS_SYSTEM_SHOW_STATE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (conn,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_BYTE,&fan_power,
		DBUS_TYPE_UINT16,&core_tmprt,
		DBUS_TYPE_UINT16,&surface_tmprt,
		DBUS_TYPE_INVALID)) 
	{
		if (dbus_error_is_set(&err)) 
		{
				dbus_error_free(&err);				
				return 0;
		}
	}

	dbus_message_unref(reply);
	
	return core_tmprt;
}





int ac_sample_temperature_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int average=0;
	
	syslog( LOG_INFO, "ac_sample_temperature_send_over_threshold_signal type=%d", type);

	average = sw_get_average( get_sample_window(this) );
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&average,
										 DBUS_TYPE_INVALID );
}




ac_sample_t *create_ac_sample_temperature(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;

	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_TMP ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_TMP, ret );
	}
	
   	pret = create_ac_sample( SAMPLE_NAME_TMP, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_temperature );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_cb( pret, ac_sample_temperature_send_over_threshold_signal );
	}
	syslog( LOG_DEBUG, "create_ac_sample_temperature  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;    
}



