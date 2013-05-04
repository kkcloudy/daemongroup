

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_drop_rate.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"


#define AWK_GET_DROP_PACKET_DROP_INFO 	"ip -s link | sed \"/[a-zA-Z]/d\" | awk 'BEGIN{i=true;total=0}{if(i==true){total+=$4;}i=!i;}END{print total}'"
#define AWK_GET_DROP_PACKET_TOTAL_INFO 	"ip -s link | sed \"/[a-zA-Z]/d\" | awk 'BEGIN{i=true;total=0}{if(i==true){total+=$1;}i=!i;}END{print total}'"

#define BUF_LEN   1024


struct drop_rate_info
{
	int  drop;
    int  total;
};

int get_drop_stat( struct drop_rate_info *stat)
{
	FILE * fp1 = NULL;
	FILE * fp2 = NULL;
	char * iter = NULL;
	char buffer1[BUF_LEN] = { 0 };
	char buffer2[BUF_LEN] = { 0 };
	
	int cur_total_pack = 0;
	int cur_drop_pack = 0;
	int ave_value = 0;	
	int drop_rate = 0;
	
	if (NULL != (fp1=popen(AWK_GET_DROP_PACKET_DROP_INFO, "r")) )
	{ 
		fgets (buffer1, BUF_LEN, fp1);
		iter = buffer1;
		cur_drop_pack = strtoul(iter, &iter, 0);
		pclose(fp1);
	}
	else
	{
		syslog(LOG_INFO, "file can not open %s\n", AWK_GET_DROP_PACKET_DROP_INFO);
		return -1;
	}
	
	if (NULL != (fp2=popen(AWK_GET_DROP_PACKET_TOTAL_INFO, "r")) )
	{
		fgets (buffer2, BUF_LEN, fp2);        
		iter = buffer2;                       
		cur_total_pack = strtoul(iter, &iter, 0);
		pclose(fp2);
	}
	else
	{
		syslog(LOG_INFO, "file can not open %s\n", AWK_GET_DROP_PACKET_TOTAL_INFO);
		return -1;
	}

	stat->drop  = cur_drop_pack;
	stat->total = cur_total_pack;
	
	return 0;
}

static int do_sample_drop_rate(ac_sample_t *me)
{
	unsigned long long sample = 0;
	struct drop_rate_info stat= { 0 };
	struct drop_rate_info *prev=(struct drop_rate_info *)get_sample_user_data(me);

	syslog(LOG_DEBUG,"do_sample_drop_rate pre_user_data = %p", prev );

	get_drop_stat( &stat );

	syslog( LOG_DEBUG, "do_sample_drop_rate prev->drop = %llu  stat.drop=%llu",prev->drop, stat.drop  );
	syslog( LOG_DEBUG, "do_sample_drop_rate prev->total = %llu stat.total=%llu", prev->total, stat.total );

	if (0==prev->drop|| 0==stat.drop-prev->drop)/*get average from system up*/
	{
    	sample = stat.drop*100/stat.total;
		memcpy( prev, &stat, sizeof(stat) );
	}
	else/*get average between  to sample!*/
	{
		sample = (stat.total-prev->total)?(stat.drop-prev->drop)*100/(stat.total-prev->total):1;
		memcpy( prev, &stat, sizeof(stat) );
		
	}

	sample = ((0==sample)?1:sample);

	syslog( LOG_DEBUG, "do_sample_drop_rate sample= %llu ", sample );
    return (int)sample;/*at least 1!!*/
}

int ac_sample_drop_rate_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int samplerate=0;
	
	syslog( LOG_INFO, "ac_sample_drop_rate_threshold_signal  type=%d\n", type);

	samplerate = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == samplerate)
	{
		return AS_RTN_ERR;
	}
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&samplerate,
										 DBUS_TYPE_INVALID );
}


ac_sample_t *create_ac_sample_drop_rate(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;
	//static struct drop_rate_info info;
	
	/*
		本采样的数据保存在接口采样点里，本采样点只提供阀值等关于采样的控制信息，
		不进行数据采样，不进行阀值检查。
	*/
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_DROPRATE ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_DROPRATE, ret );
	}

	pret = create_ac_sample( SAMPLE_NAME_DROPRATE, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb(pret, NULL);
		set_sample_state( pret,  config.sample_on );
		set_sample_threshold( pret, config.threshold );
		
		set_over_threshold_cb( pret, NULL );
	}
	syslog( LOG_DEBUG, "create_ac_sample_drop_rate  %p", pret );

	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );

	return pret;
}

