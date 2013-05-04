#if 0

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_band_with.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"

#define AWK_GET_PORT_FLOW_INFO "ifconfig | awk '/RX bytes/{if($8!~/KiB/){print $3,$7}}'"
#define BUF_LEN 	1024

struct band_with_info
{
	unsigned long long used;
    unsigned long long total;
};

int get_band_stat( struct band_with_info *stat)
{
	char * iter = NULL;
	FILE * fp = NULL;
	char buffer[BUF_LEN] = { 0 };
	double value = 0 ; 
	double cur_value_total = 0;
	int count = 0;
	
	int i;
	
	if (NULL != (fp=popen(AWK_GET_PORT_FLOW_INFO, "r")) )
	{
		while (NULL != (fgets(buffer,BUF_LEN,fp)))   //统计每个接口
		{
			iter = buffer;
			for (i=0; i<2; i++)        //统计收发2个数据和
			{
				if (0==i)
				{
					iter++;  //去除多余符号
				}
				else if(1==i)
				{
					iter = iter + 2; //去除多余符号
				}
				value = 0;
				value = strtod(iter, &iter);
				cur_value_total += value;
				count++;
			}
		}
		pclose(fp);
	}
	else
	{
		syslog( LOG_INFO, "file can not open %s", AWK_GET_PORT_FLOW_INFO );
		return -1;
	}

	//不明白
	//bandwith_rate = ( (cur_value_total-prv_value_total) * 12 * 100)/(2*PORT_NUM*PORT_BANDWITH*3600); //数值12代表5分钟转化为1个小时
	stat->total = (unsigned long long)cur_value_total;
	stat->used  = cur_value_total/count;
	
	return 0;
}

static int do_sample_bandwith(ac_sample_t *me)
{
	unsigned long long sample = 0;
	struct band_with_info stat;
	struct band_with_info *prev=(struct band_with_info *)get_sample_user_data(me);


	syslog(LOG_DEBUG,"do_sample_bandwith pre_user_data = %p", prev );

	memset( &stat, 0, sizeof(stat));
	get_band_stat( &stat );

	syslog( LOG_DEBUG, "do_sample_bandwith prev->used = %llu  stat.used=%llu",prev->used, stat.used  );
	syslog( LOG_DEBUG, "do_sample_bandwith prev->total = %llu stat.total=%llu", prev->total, stat.total );

	if (0==prev->used || 0==stat.used-prev->used)/*get average from system up*/
	{
    	sample = stat.used*100/stat.total;
		memcpy( prev, &stat, sizeof(stat) );
	}
	else/*get average between  to sample!*/
	{
		sample = (stat.total-prev->total)?(stat.used-prev->used)*100/(stat.total-prev->total):1;
		memcpy( prev, &stat, sizeof(stat) );
		
	}

	sample = ((0==sample)?1:sample);

	syslog( LOG_DEBUG, "do_sample_bandwith sample= %llu ", sample );
    return (int)sample;/*at least 1!!*/
}

int ac_sample_band_with_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int samplerate=0;
	
	syslog( LOG_INFO, "ac_sample_band_with_threshold_signal  type=%d\n", type);

	samplerate = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == samplerate)
	{
		return AS_RTN_ERR;
	}
	
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&samplerate,
										 DBUS_TYPE_INVALID );
}


ac_sample_t *create_ac_sample_band_with(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;
	//static struct band_with_info band_info;
	/*
		本采样的数据保存在接口采样点里，本采样点只提供阀值等关于采样的控制信息，
		不进行数据采样，不进行阀值检查。
	*/
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_BANDWITH ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_BANDWITH, ret );
	}
	pret = create_ac_sample( SAMPLE_NAME_BANDWITH, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb(pret, NULL);
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		
		set_over_threshold_cb( pret, NULL );
	}
	syslog( LOG_DEBUG, "create_ac_sample_band_with  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );

	return pret;
}

#endif
