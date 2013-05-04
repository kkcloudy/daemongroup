#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_cpu.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"

#define AC_CPU_INFO_FILE "/proc/stat"
#define CPU_STAT_SIZE 256


static char head[32];
/*
struct cpu_stat_t{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long unknown;
    
    unsigned long long used;
    unsigned long long total;
};
*/
//modified by stephen, move this define to ac_sample_def.h



int load_cpu_stat( char *line, struct cpu_stat_t *pcs )
{
    char head[32]="";
    if( NULL == pcs || NULL == line )
    {
        return -1;
    }
    
    if( strncmp( line, "cpu", 3 ) != 0 )
    {
        return -1;
    }

    sscanf( line, "%31s %llu%llu%llu%llu%llu%llu%llu%llu", head,
                &(pcs->user),
                &(pcs->nice),
                &(pcs->system),
                &(pcs->idle),
                &(pcs->iowait),
                &(pcs->irq),
                &(pcs->softirq),
                &(pcs->unknown) );

    pcs->used = pcs->user + pcs->nice + pcs->system;
    pcs->total = pcs->used + pcs->idle + pcs->iowait + pcs->irq + pcs->softirq;

#if 0
    printf("pcs->user = %llu\n", pcs->user);
    printf("pcs->nice = %llu\n", pcs->nice);
    printf("pcs->system = %llu\n", pcs->system);
    printf("pcs->idle = %llu\n", pcs->idle);
    printf("pcs->iowait = %llu\n", pcs->iowait);
    printf("pcs->irq = %llu\n", pcs->irq);
    printf("pcs->softirq = %llu\n", pcs->softirq);
    printf("pcs->unknown = %llu\n", pcs->unknown);
    
    printf("pcs->used = %llu\n", pcs->used);
    printf("pcs->total = %llu\n", pcs->total);
#endif	

    return 0;
}




static int parse1(char *line, struct cpu_stat_t *pc)
{
	if(NULL == pc || NULL == line)
	{
		return -1;
	}
	sscanf(line, "%5s%llu%llu%llu%llu%llu%llu%llu%llu",
					head,
					&(pc->user),
					&(pc->nice),
					&(pc->system),
					&(pc->idle),
					&(pc->iowait),
					&(pc->irq),
					&(pc->softirq),
					&(pc->unknown));
	pc->used = pc->user + pc->nice + pc->system;
	pc->total = pc->used + pc->idle + pc->iowait + pc->irq + pc->softirq;
	
	return 0;

}

/*added by stephen for cpu rt stat*/
int get_cpu_rt_stat(struct cpu_stat_t *pcs )
{
    FILE *fp = NULL ;
    char buff[CPU_STAT_SIZE];
	int m = 0;
    
    if(NULL != (fp = fopen(AC_CPU_INFO_FILE, "r")))
    {
    	syslog(LOG_DEBUG, "+++++++++++hehe\n");
        while( NULL != fgets(buff, CPU_STAT_SIZE, fp))
        {
           if(0 == strncmp(buff, "cpu", 3)){
		   	syslog(LOG_DEBUG, "+++++++haha\n");
			if(0 == parse1(buff, &(pcs[m]))){
				memset(head, 0, 32);
				m++;
			}
			memset(buff, 0, 256);
		   }else{
			break;
		   }
        }
        fclose( fp );
    }

	return m;
}


int get_cpu_stat( struct cpu_stat_t *pcs )
{
    FILE *fp = NULL ;
    char buff[CPU_STAT_SIZE];
    
    if(NULL != (fp = fopen(AC_CPU_INFO_FILE, "r")))
    {
        if( NULL != fgets(buff, CPU_STAT_SIZE, fp))
        {
            if( 0 != load_cpu_stat(buff, pcs ) )
            {
                fclose( fp );
                return -1;
            }
        }
        fclose( fp );
    }

	return 0;
}


int get_cpu_usage( unsigned int sleep_time )
{
    struct cpu_stat_t sta1, sta2;
    int iret = 0;
    
    memset( &sta1, 0, sizeof(sta1) );
    memset( &sta2, 0, sizeof(sta2) );
    
    get_cpu_stat( &sta1 );
    usleep( sleep_time*1000);
    get_cpu_stat( &sta2 );
    
    iret = (sta2.total-sta1.total)?(sta2.used-sta1.used)*100/(sta2.total-sta1.total):0;
    
    if( 0 == iret )
    {
        iret = 1;
    }
    
    return iret;		
}

//modified by stephen
static int do_sample_cpurt( ac_sample_t *me )
{
	syslog(LOG_INFO,"do_sample_cpurt");
    unsigned long long sample = 0;
	int m = 0;
	int t = 0;
	int i = 0;
	int j = 0;
	double rate;
	double alldata[18];
	struct cpu_stat_t stat[18];
	struct cpu_stat_t *prev=(struct cpu_stat_t *)get_sample_user_data(me);
	//prev = (struct cpu_stat_t *)malloc(sizeof(struct cpu_stat_t) * 18);

	
	cpu_in_t *sample_data = (cpu_in_t *)get_sample_cpu_rt_data(me);
	
	syslog(LOG_DEBUG,"do_sample_cpu user_data = %p", prev );

	//memset( &spe, 0, sizeof(spe));
	m = get_cpu_rt_stat( stat );
	syslog(LOG_DEBUG, "------------%d\n", m);
	//syslog( LOG_DEBUG, "do_sample_cpu prev->used = %llu  stat.used=%llu",prev->used, stat.used  );
	//syslog( LOG_DEBUG, "do_sample_cpu prev->total = %llu stat.total=%llu", prev->total, stat.total );
	for(t = 0; t < m; t++){
		if( (0 == prev[t].used) || 0 == (stat[t].used - prev[t].used))/*get average from system up*/
		{
    		rate = ((double)((stat[t].used) * 100))/stat[t].total;
			alldata[i] = rate;
			i++;
			//memcpy( prev, &stat, sizeof(stat) );
		}
		else/*get average between  to sample!*/
		{
			rate = ((stat[t].total-prev[t].total) > 0)?((double)(stat[t].used-prev[t].used)*100)/(stat[t].total-prev[t].total):0;
			alldata[i] = rate;
			i++;
			syslog(LOG_DEBUG, "*********************\n");
			syslog(LOG_DEBUG, "stat[t].total: %f\n", stat[t].total);
			syslog(LOG_DEBUG, "stat[t].used: %f\n", stat[t].used);
			syslog(LOG_DEBUG, "prev[t].total: %f\n", prev[t].total);
			syslog(LOG_DEBUG, "prev[t].used: %f\n", prev[t].used);
			//memcpy( prev, &stat, sizeof(stat) );
		
		}
	}
	syslog(LOG_DEBUG, "---------%d\n", i);
	syslog(LOG_DEBUG, "-----------haha\n");
	memcpy(prev, &stat, sizeof(struct cpu_stat_t) * 18);
	syslog(LOG_DEBUG, "-----------hehe\n");
	sample_data->cpu_cont = i - 1;
	syslog(LOG_DEBUG, "sample_data->cpu_cont: %d\n",sample_data->cpu_cont);
	sample_data->cpu_average = alldata[0];
	syslog(LOG_DEBUG, "alldata[0]: %f\n", alldata[0]);
	syslog(LOG_DEBUG, "sample_data->cpu_average = %f\n", sample_data->cpu_average);
	for(j = 1; j < i; j++){
		syslog(LOG_DEBUG, "alldata[%d]=", j);
		syslog(LOG_DEBUG, "%f\n", alldata[j]);
		
		sample_data->cpu_info[j - 1] = alldata[j];
		syslog(LOG_DEBUG, "sample_data->cpu_info[%d-1]=%f\n", j, sample_data->cpu_info[j - 1]);
	}

    if(0 == sample)
    {
        sample = 1;
    }
	//free(prev);

	syslog( LOG_DEBUG, "do_sample_cpu sample= %llu ", sample );
    return (int)sample;/*at least 1!!*/
}



static int do_sample_cpu( ac_sample_t *me )
{
	syslog(LOG_INFO,"do_sample_cpu");
    unsigned long long sample;
	struct cpu_stat_t stat;
	struct cpu_stat_t *prev=(struct cpu_stat_t *)get_sample_user_data(me);


	syslog(LOG_DEBUG,"do_sample_cpu user_data = %p", prev );

	memset( &stat, 0, sizeof(stat));
	get_cpu_stat( &stat );

	syslog( LOG_DEBUG, "do_sample_cpu prev->used = %llu  stat.used=%llu",prev->used, stat.used  );
	syslog( LOG_DEBUG, "do_sample_cpu prev->total = %llu stat.total=%llu", prev->total, stat.total );

	if( 0 == prev->used || 0 == stat.used-prev->used)/*get average from system up*/
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

	syslog( LOG_DEBUG, "do_sample_cpu sample= %llu ", sample );
    return (int)sample;/*at least 1!!*/
}


int ac_sample_cpu_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int average=0;
	
	syslog( LOG_INFO, "ac_sample_cpu_send_over_threshold_signal  type=%d\n", type);

	average = sw_get_average( get_sample_window(this) );
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&average,
										 DBUS_TYPE_INVALID );
}
/*cpu_real_time, added by stephen */
ac_sample_t *create_ac_sample_cpu_real_time_stat(
															unsigned int sample_interval,
															unsigned int statistics_time)
{
		ac_sample_t *pret = NULL;
		static struct cpu_stat_t stat;
		struct cpu_stat_t *prev;
		cpu_in_t *sample_data;
		cpu_in_t spe;/*added by stephen*/
		acsample_conf config;
		int ret=-1;
	
		memset(&config, 0, sizeof(config));
		set_cpu_rt_default_config ( &config );
		if ((ret=acsample_read_conf( &config, SAMPLE_NAME_CPURT ) ))
		{
			syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_CPURT, ret );
		}
		
		pret = create_ac_sample( SAMPLE_NAME_CPURT, config.sample_interval, config.statistics_time, config.resend_interval );
		
		if( NULL != pret )
		{
			//prev=(struct cpu_stat_t *)get_sample_user_data(pret);
			//prev = (struct cpu_stat_t *)malloc(sizeof(struct cpu_stat_t) * 18);
			//prev = init_sample_user_data_rt(pret);
			//sample_data = (cpu_in_t *)malloc(sizeof(cpu_in_t));
			//modified by stephen 
			//sample_data = init_sample_cpu_rt_data(pret);
			set_do_sample_cb( pret, do_sample_cpurt );
			set_sample_state( pret, config.sample_on );
		//	set_sample_threshold( pret, config.threshold );
			
			memset( &stat, 0, sizeof(stat));
			syslog(LOG_DEBUG,"create_ac_sample_cpu_rt user_data = %p", &stat );
			//set_sample_user_data( pret, &stat, NULL );
			
		//	set_over_threshold_cb( pret, ac_sample_cpu_send_over_threshold_signal );
		}
		syslog( LOG_DEBUG, "create_ac_sample_cpu  %p", pret );
	
		syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
								config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
								config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	
		return pret;	


}



ac_sample_t *create_ac_sample_cpu(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;
	static struct cpu_stat_t stat;
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_CPU ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_CPU, ret );
	}
	
   	pret = create_ac_sample( SAMPLE_NAME_CPU, config.sample_interval, config.statistics_time, config.resend_interval );
	
	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_cpu );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		
		memset( &stat, 0, sizeof(stat));
		syslog(LOG_DEBUG,"create_ac_sample_cpu user_data = %p", &stat );
		set_sample_user_data( pret, &stat, NULL );
		
		set_over_threshold_cb( pret, ac_sample_cpu_send_over_threshold_signal );
	}
	syslog( LOG_DEBUG, "create_ac_sample_cpu  %p", pret );

	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );

	return pret;    
}


