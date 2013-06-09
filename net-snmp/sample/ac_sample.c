#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "nm_list.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample.h"
#include "ac_manage_def.h"
#include "board/board_define.h"
#include "hmd/hmdpub.h"
#include <mcheck.h>


#define MAX_SAMPLE_NAME_LEN     64

/*check last  NORMAL_CHECK_TIMES sample val, NORMAL_CHECK_COUNT over threshold to send signal*/
#define NORMAL_CHECK_TIMES		3	
#define NORMAL_CHECK_COUNT      2


struct sample_window_t{
    unsigned int size;
    unsigned int val_num;
    int *vals;
};

struct subsample_config_t{
	struct list_head node;
	unsigned int match_word;
	void *subuser_data;
};

struct subsample_data_t  {
	struct list_head node;
	int flag;	//0 is canceled 1 for do this subsample
	unsigned int match_word;
	sample_window_t *win;
	/*for user define sample data*/
	void 			*subuser_data;
	//USER_DATA_FREE	subuser_data_free;
	//GET_SAMPLE_INFO	get_sample_info;
	unsigned long   last_over_threshold_call_time;
	int				last_over_threshold_check_state;
};

struct multi_sample_t{
	unsigned int size;
	subsample_data_t *curr_sample;
	USER_DATA_FREE	subuser_data_free;
	GET_SAMPLE_INFO	subget_sample_info;
	GENERATE_MATCH_WORD generate_match_word;
	struct list_head head;
};

struct ac_sample_t { 
    struct list_head node;
    char            name[MAX_SAMPLE_NAME_LEN];
    
	/*for sample*/
	int 			sample_on;
    unsigned int    sample_interval;
    unsigned int    statistics_time;
    sample_window_t *win;	
	DO_SAMPLE       do_sample;
	DO_CHECK		do_check;

	/*for user define sample data*/
	void 			*user_data;
	void			*spe_ste;//for cpu rt stat, added by stephen
	USER_DATA_FREE	user_data_free;
	GET_SAMPLE_INFO	get_sample_info;

	/*for trap*/
    int     		over_threshold_check_type;
    int 		clear_check_type;
    int     		threshold;
    unsigned long   last_over_threshold_call_time;
	unsigned long   resend_interval;
	int				last_over_threshold_check_state;
    
    unsigned long   last_sample_time;
    
    ON_SAMPLE_OVER_THRESHOLD  on_over_threshold;

	/*for multi sample*/
	int sample_mode;  //0 is default sample mode 1 is multi sample mode
	multi_sample_t *multi_sample;

	/*for dynamic load config file*/
	LOAD_CONFIG load_config;
	int already_loaded_config;		//insure config file load at least once at reload_config_interval <= 0
	unsigned int reload_config_interval;		//default is 1 set <=0 is load config file only once at equitment running time. < sample_interval express that every sample_interval load once
	unsigned int last_load_config_time;	

};

void *sample_malloc (unsigned int malloc_size,
								const char * malloc_file_name,
								const char * malloc_func_name,
								int 	malloc_line_num )
{
	void *ptr=NULL;
	if (NULL != (ptr=malloc(malloc_size))){ 
		syslog(LOG_DEBUG,"[%s@%s,%d]malloc: address is %p size is %d\n",malloc_func_name,malloc_file_name,malloc_line_num,ptr,malloc_size);
		return ptr;
	}else
	{
		syslog(LOG_ERR,"[%s@%s,%d]malloc: malloc error!\n",malloc_func_name,malloc_file_name,malloc_line_num);
		return NULL;
	}
}

int set_global_soket_timout(int timeout)
{
	if (timeout < 0)
	{
		syslog(LOG_INFO,"set_global_soket_timout timeout value %d error!.", timeout);
		return AS_RTN_PARAM_ERR;
	}

	gTimeout.tv_sec=timeout/1000;
	gTimeout.tv_usec=(timeout%1000)*1000;
	
	return AS_RTN_OK;
}

unsigned int get_global_socket_timout ()
{
	return gTimeout.tv_sec*1000+gTimeout.tv_usec/1000 ;
}

int init_sample_socket_timeout ()
{
	int ret=AS_RTN_OK;
	unsigned int value=DEFAULT_TIMEOUT;
	if ((ret=acsample_read_socket_timeout_conf( &value , SE_XML_ACSAMPLE_TIMEOUT ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SE_XML_ACSAMPLE_TIMEOUT, ret );
	}

	ret=set_global_soket_timout ( value );

	if ( 0!=ret )
	{
		ret = AS_RTN_PARAM_ERR;
	}

	return ret;
}

void log_sample_win( sample_window_t *win )
{
	char buff[256]="";
	int i;

	for( i=0; i<win->val_num; i++ )
	{
		snprintf( buff+strlen(buff), 
					sizeof(buff)-strlen(buff)-1,
					" %d ", win->vals[i] );
	}

	syslog( LOG_DEBUG, "    win->val_num=%d", win->val_num );		//test
	syslog( LOG_DEBUG, "    win->vals: %s", buff );		//test
	syslog( LOG_DEBUG, "    latest:%d    average:%d    max:%d", 
								sw_get_latest(win),
								sw_get_average(win),
								sw_get_max(win) );	//test
	return;
}

void log_sample_data( ac_sample_t *pas )
{
	syslog( LOG_DEBUG, "sample name:%s", pas->name );	//test
	log_sample_win( pas->win );

	return;	
}

char * inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize )
{
	if(ip_str == NULL)
	{
		return "";
	}
	snprintf( ip_str, buffsize, "%u.%u.%u.%u", ip_addr>>24, (ip_addr&0xff0000)>>16,
				(ip_addr&0xff00)>>8, (ip_addr&0xff) );
				
	return ip_str;
}

void sw_push_val( sample_window_t *win, int val )
{
    if( NULL == win )
    {
    	syslog( LOG_ERR, "sw_push_val err: win is NULL!" );
        return;
    }
    
    if( win->val_num < win->size )
    {
        win->vals[win->val_num] = val;
        win->val_num ++;
    }
    else
    {
        memcpy( win->vals, win->vals + 1, (win->size-1)*sizeof(int) );
        win->vals[win->val_num-1] = val;
    }

    return;
}

void sw_reset (sample_window_t *win )
{
	int i;
	
	if ( NULL==win )
	{
		syslog( LOG_ERR, "sw_push_val err: win is NULL!" );
        	return;
	}

	for( i=0; i<win->val_num; i++ )
	{
		win->vals[i]=0;
	}
	
	return;
}

int sw_get_max(sample_window_t *win )
{
    int i;
    int max = 0;
    
    if( NULL == win )
    {
        return max;
    }
    
    for( i=0; i<win->val_num; i++ )
    {
        if( max < win->vals[i] )
        {
            max = win->vals[i];
        }
    }
    
    return max;
}


int sw_get_average( sample_window_t *win )
{
    int i;
    int total=0;

	if( NULL == win )
	{
		syslog( LOG_ERR, "sw_get_average  win is NULL!");
		return 0;
	}

    for( i=0; i<win->val_num; i++ )
    {
        total += win->vals[i];
    }
    
    return (0==win->val_num)?0:total/(win->val_num);
}

int sw_get_latest( sample_window_t *win )
{
	if( NULL == win )
	{
		syslog( LOG_ERR, "sw_get_latest  win is NULL!");
		return 0;
	}

	if( 0 == win->val_num )
	{
		syslog( LOG_ERR,"sw_get_latest no sample val!" );
		return 0;
	}
	
	return win->vals[win->val_num-1];
}
int sw_get_sample_value( ac_sample_t *me )
{
	sample_window_t *win = NULL;
	
	if ( NULL==me ||
		NULL==(win=get_sample_window(me)) )
    {
        return AS_RTN_PARAM_ERR;
    }
    
    switch( me->over_threshold_check_type )
    {
        case OVER_THRESHOLD_CHECK_TYPE_NORMAL:
		{
            return sw_get_average(win);
		}
 
        case OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY:
		{
			return sw_get_latest(win);
		}
         
        default:
            break;
    }
    
    return AS_RTN_PARAM_ERR;
}



int sw_set_size( sample_window_t *win, int new_size )
{  

	syslog( LOG_DEBUG, "sw_set_size %d", new_size);

    win->vals = (int *)realloc( win->vals, new_size*sizeof(int) );
    if( NULL != win->vals )
    {
        win->size = new_size;
        if( win->val_num > new_size )
        {
            win->val_num = new_size;
        }    
    }
	else
	{
		return AS_RTN_MALLOC_ERR;
	}

    return AS_RTN_OK;
}

int sw_get_size( sample_window_t *win )
{
	if( NULL == win )
    {
        return 0;
    }

	return win->size;
}

/*you should free array after use it!*/
int sw_get_data_array( sample_window_t *win, int **array )
{
    if(NULL == win)
    {
        return 0;
    }

	if( win->val_num > 0 )
    {
        return 0;
    }

	*array = (void *)memdup( (void *)(win->vals), (win->val_num)*sizeof(win->vals[0]));
	
	return win->val_num;
}



sample_window_t *create_sample_window( unsigned int size )
{
    sample_window_t *psw = NULL;
    
    psw = (sample_window_t *)SAMPLE_MALLOC( sizeof(sample_window_t) );
    if( NULL != psw )
    {
        memset( psw, 0, sizeof(sample_window_t) );
        if( AS_RTN_OK != sw_set_size( psw, size ) )
        {
			syslog( LOG_CRIT, "create_sample_window failed!" );
			SAMPLE_FREE( psw );
			psw = NULL;
		}
    }
    
    return psw;
}

void destroy_sample_multi(multi_sample_t *multi_sa)
{
	SAMPLE_FREE(multi_sa);
}


void destroy_sample_window( sample_window_t *win )
{
    if( NULL != win && NULL != win->vals )
    {
        SAMPLE_FREE( win->vals );
    }
    
    if( NULL != win )
    {
        SAMPLE_FREE( win );
    }
    
    return;
}



int set_sample_state( ac_sample_t *me, int state )
{
	if( SAMPLE_OFF != state &&
		SAMPLE_ON != state )
	{
		return AS_RTN_SAMPLE_STATE_ERROR;
	}
	else
	{
		me->sample_on = state;
	}
	
	return AS_RTN_OK;
}

int get_sample_state( ac_sample_t *me )
{
	if( NULL == me )
	{
		return AS_RTN_NULL_POINTER;
	}

	return me->sample_on;
}



static int over_threshold_check_immediately( ac_sample_t *me )
{
	if (NULL==me)
	{
		return NOT_OVER_THRESHOLD;	
	}
	
	sample_window_t *win = me->win;
	if (NULL==win  || (win->val_num <= 0) )
	{
		return NOT_OVER_THRESHOLD;
	}
	
	if (NULL != me->do_check)
	{
		return me->do_check(me->threshold, win->vals[me->win->val_num-1]); 
	}
	
    if( me->win->val_num > 0 &&
        me->win->vals[me->win->val_num-1] > me->threshold )
    {
        return OVER_THRESHOLD_FLAG;
    }
    
    return NOT_OVER_THRESHOLD;
}



static int over_threshold_check_normal( ac_sample_t *me )
{
    //int i,count;
    if (NULL==me)
    {
		return NOT_OVER_THRESHOLD;
	}

	syslog( LOG_DEBUG, "over_threshold_check_normal win->val_num=%d\n", me->win->val_num );
    //if( me->win->val_num >= NORMAL_CHECK_TIMES )
    {
#if 0    
        count = 0;
        
        for( i=me->win->val_num-1; i>=(int)(me->win->val_num-NORMAL_CHECK_TIMES); i-- )
        {
        	syslog( LOG_DEBUG, "over_threshold_check_normal me->win->vals[%d]=%d", i, me->win->vals[i] );
            if( me->win->vals[i] > me->threshold )
            {
                count++;
            }
        }
        
        if( count >= NORMAL_CHECK_COUNT )
        {
            return OVER_THRESHOLD_FLAG;
        }
#else		
		sample_window_t *win = me->win;
		
		if (NULL!=me->do_check)
		{
			return me->do_check(me->threshold, sw_get_average(win) );
		}
		
		if( sw_get_average(win) > me->threshold )
		{
			return OVER_THRESHOLD_FLAG;
		}
#endif
    }

    return NOT_OVER_THRESHOLD;
}


static int do_threshold_check( ac_sample_t *me )
{
    int iret = NOT_OVER_THRESHOLD;
    
    
    if( NULL == me )
    {
        return NOT_OVER_THRESHOLD;
    }
    
    switch( me->over_threshold_check_type )
    {
        case OVER_THRESHOLD_CHECK_TYPE_NORMAL:
            iret = over_threshold_check_normal( me );
            break;
        case OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY:
            iret = over_threshold_check_immediately( me );
            break;
        default:
            break;
    }
    
    return iret;
}


ac_sample_t *create_ac_sample(  char *name,
                                unsigned int sample_interval, 
                                unsigned int statistics_time,
                                unsigned int resend_interval)
{
    ac_sample_t *pas=NULL;
    int sw_size = 0;
    /*
    if( sample_interval < MIN_SAMPLE_INTERVAL ||
        statistics_time < MIN_STATISTICS_TIME )        
    {
        return NULL;
    }
    */
    pas = (ac_sample_t *)SAMPLE_MALLOC( sizeof(ac_sample_t) );
    if( NULL != pas )
    {
        memset( pas, 0, sizeof(ac_sample_t) );
        strncpy( pas->name, name, sizeof(pas->name)-1 );
        pas->sample_interval = sample_interval;
        pas->statistics_time = statistics_time;
		pas->resend_interval = resend_interval;
		
		pas->over_threshold_check_type = OVER_THRESHOLD_CHECK_TYPE_NORMAL;
       
        sw_size = (statistics_time + sample_interval-1)/sample_interval;
        pas->win = create_sample_window( sw_size );
        if( NULL == pas->win )
        {
            SAMPLE_FREE( pas );
            pas = NULL;
        }
#if 0
        pas->avlwin = create_sample_window( AVL_WINDOW_LEN );
        if( NULL == pas->avlwin )
        {
            destroy_sample_window( pas->win );
            free( pas );
            pas = NULL;            
        }
#endif		
    }

    set_sample_mode ( pas, SET_DEFAULT_SAMPLE_MODE );
	
    return pas;
}


const char *get_sample_name( ac_sample_t *me )
{
    return me->name;
}

int set_sample_user_data( ac_sample_t *me, void *user_data, USER_DATA_FREE free_cb )
{
	me->user_data = user_data;
	me->user_data_free = free_cb;
	syslog( LOG_DEBUG, "set_sample_user_data  me=%p  user_data=%p", me, user_data );
	return AS_RTN_OK;
}

void *get_sample_user_data( ac_sample_t *me )
{
	syslog( LOG_DEBUG, "get_sample_user_data  me=%p  user_data=%p", me, me->user_data );
	return me->user_data;
}

void *get_sample_cpu_rt_data(ac_sample_t *me)
{
	syslog( LOG_DEBUG, "get_sample_cpu_rt_data  me=%p  spe_ste=%p", me, me->spe_ste );
	return me->spe_ste;	
}

struct cpu_stat_t *init_sample_user_data_rt(ac_sample_t *me)
{
	me->user_data = (struct cpu_stat_t *)malloc(sizeof(struct cpu_stat_t) * 18);
	return me->user_data;

}

cpu_in_t *init_sample_cpu_rt_data(ac_sample_t *me)
{
	me->spe_ste = (cpu_in_t *)malloc(sizeof(cpu_in_t));
	return me->spe_ste;
}



int destroy_ac_sample( ac_sample_t *me )
{
    if( NULL != me && NULL != me->win )
    {
        destroy_sample_window( me->win );
    }

	if( NULL != me && NULL != me->user_data && NULL != me->user_data_free )
	{
		me->user_data_free( me->user_data );
	}	
	if(NULL != me && NULL != me->multi_sample )
	{
		if(NULL != me->multi_sample->subuser_data_free && NULL != me->user_data)
			me->multi_sample->subuser_data_free(me->user_data);
		destroy_sample_multi(me->multi_sample);
	}
	
    if( NULL != me )
    {
        SAMPLE_FREE( me );
    }
    
    return 0;
}


int get_window_size( ac_sample_t *me )
{
    if( NULL == me || NULL == me->win )
    {
        return 0;
    }

	return me->win->size;
}


sample_window_t *get_sample_window( ac_sample_t *me )
{
    if( NULL == me || NULL == me->win )
    {
        return NULL;
    }

    
    return me->win;
}

#if 0
int get_max_val( ac_sample_t  *me )
{
    if( NULL == me || NULL == me->win )
    {
        return 0;
    }
    
    return sw_get_max( me->win );
}


int get_average_val( ac_sample_t  *me )
{
    if( NULL == me || NULL == me->win )
    {
        return 0;
    }    
    
    return sw_get_average( me->win );
}

#endif

int set_over_threshold_type( ac_sample_t *me, int type )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    if( OVER_THRESHOLD_CHECK_TYPE_NORMAL == type ||
        OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY == type )
    {
        me->over_threshold_check_type = type;
    }
    
    return 0;
}

int set_clear_type( ac_sample_t *me, int type )	//at present only used for multi sample mode
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    if( OVER_THRESHOLD_CHECK_TYPE_NORMAL == type ||
        OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY == type )
    {
        me->clear_check_type = type;
    }
    
    return 0;
}


int set_sample_interval( ac_sample_t  *me, unsigned int sample_interval )//´óÓÚ5
{
    int sw_size = 0;
    int statistics_time = 0;

    if( NULL == me || NULL == me->win )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    statistics_time = me->statistics_time; 
    if( sample_interval < MIN_SAMPLE_INTERVAL )
    {
        return AS_RTN_INTERVAL_LETTER_MIN ;
    }
    else if( statistics_time < sample_interval )        
    {
        return AS_RTN_INTERVAL_LAGER_STATICS;
    }

    me->sample_interval = sample_interval;
    sw_size =  (statistics_time + sample_interval-1)/sample_interval;

    if ( SAMPLE_MODE_SINGLE==me->sample_mode )
    {
    	sw_set_size( me->win, sw_size );
    }else
    {
	struct list_head *pos=NULL;
	struct list_head *next=NULL;

	list_for_each_safe(pos,next,&(me->multi_sample->head))
	{
		subsample_data_t * curr_sample = NULL;
		curr_sample = MTSAMPLE ( pos );
		sw_set_size( curr_sample->win, sw_size );
		syslog(LOG_DEBUG,"[multi mode] set_sample_interval: sw_set_size");  //test
	}

    }
    return AS_RTN_OK;
}




int get_sample_interval( ac_sample_t  *me )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    return me->sample_interval;
}


int set_statistics_time( ac_sample_t  *me, unsigned int statistics_time )
{
    int sw_size = 0;
    int sample_interval = 0;

    if( NULL == me || NULL == me->win )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    sample_interval = me->sample_interval;
    if( statistics_time < sample_interval )
    {
        return AS_RTN_STATICS_LETTER_INTERVAL;
    }
    else if( statistics_time < MIN_STATISTICS_TIME )
    {
        return AS_RTN_PARAM_ERR;
    }

    me->statistics_time = statistics_time;
    sw_size = (statistics_time + sample_interval-1)/sample_interval;

    if ( SAMPLE_MODE_SINGLE==me->sample_mode )
    {
    	sw_set_size( me->win, sw_size );
    }else
    {
	struct list_head *pos=NULL;
	struct list_head *next=NULL;

	list_for_each_safe(pos,next,&(me->multi_sample->head))
	{
		subsample_data_t * curr_sample = NULL;
		curr_sample = MTSAMPLE ( pos );
		sw_set_size( curr_sample->win, sw_size );
		syslog(LOG_DEBUG,"set_statistics_time: sw_set_size");  //test
	}

    }
	
    
    return AS_RTN_OK;    
}


int get_statistics_time( ac_sample_t  *me )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    return me->statistics_time;    
}


int set_resend_interval( ac_sample_t  *me, unsigned int resend_interval )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }

	me->resend_interval = resend_interval;
    return AS_RTN_OK;    	
}



int get_resend_interval( ac_sample_t  *me )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    return me->resend_interval;  

}



int set_sample_threshold( ac_sample_t  *me, unsigned int  threshold )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    me->threshold = threshold;
	syslog( LOG_DEBUG, "set_sample_threshold  sample:%s  threshold:%d\n",me->name, threshold  );
    return AS_RTN_OK;
}

int get_sample_threshold( ac_sample_t  *me )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    return me->threshold;
}


int set_do_sample_cb( ac_sample_t *me, DO_SAMPLE do_sample )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }

	me->do_sample = do_sample;

	return AS_RTN_OK;
}

int set_over_threshold_cb( ac_sample_t *me, ON_SAMPLE_OVER_THRESHOLD over_threshold_cb )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    me->on_over_threshold = over_threshold_cb;
	syslog( LOG_DEBUG, "set_over_threshold_cb  sample:%s  on_over_threshold:%p\n",me->name, over_threshold_cb  );
    return AS_RTN_OK;
}


int set_get_sample_info_cb( ac_sample_t *me, GET_SAMPLE_INFO get_sample_info )
{
    if( NULL == me )
    {
		syslog( LOG_ERR, "set_get_sample_info_cb  me is NULL!" );
        return AS_RTN_NULL_POINTER;
    }
    
    me->get_sample_info = get_sample_info;
    
    return AS_RTN_OK;	
}

int set_do_check_cb(ac_sample_t *me, DO_CHECK do_check )
{
	if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }
    
    me->do_check = do_check;
	syslog( LOG_DEBUG, "set_do_check_cb  sample:%s  DO_CHECK:%p\n",me->name, do_check  );
    return AS_RTN_OK;
}

int create_multi_sample ( ac_sample_t *me )
{
	if ( NULL==me )
	{	
		return AS_RTN_NULL_POINTER;
	}

	multi_sample_t *multi_sample=( multi_sample_t * ) SAMPLE_MALLOC ( sizeof(multi_sample_t) );

	if ( NULL==multi_sample )
	{	
		syslog(LOG_ERR,"create_multi_sample: malloc error !\n");
		return AS_RTN_NULL_POINTER;
	}

	multi_sample->size=0;
	multi_sample->curr_sample=NULL;
	multi_sample->generate_match_word=NULL;

	INIT_LIST_HEAD( &(multi_sample->head) );
	syslog(LOG_DEBUG,"create_multi_sample:head %p multi_sample->head {%p,%p} ",&(multi_sample->head) ,multi_sample->head.prev,multi_sample->head.next); //test
	me->multi_sample=multi_sample;

	return AS_RTN_OK;
}

int set_sample_reload_config_interval ( ac_sample_t *me, unsigned int reload_config_interval )
{
	if ( NULL==me )
	{	
		return AS_RTN_NULL_POINTER;
	}

	me->reload_config_interval=reload_config_interval;

	syslog( LOG_DEBUG, "set_sample_reload_config_interval  sample:%s  reload_interval:%u\n",me->name, reload_config_interval );

	return AS_RTN_OK;
}

unsigned int get_sample_reload_config_interval ( ac_sample_t *me )
{
	if ( NULL==me )
	{	
		return AS_RTN_NULL_POINTER;
	}

	syslog( LOG_DEBUG, "get_sample_reload_config_interval  sample:%s  reload_interval:%u\n",me->name, me->reload_config_interval );

	return me->reload_config_interval;
}


int set_sample_load_conf ( ac_sample_t *me, LOAD_CONFIG load_config )
{
	if ( NULL==me )
	{	
		return AS_RTN_NULL_POINTER;
	}
	
	me->load_config=load_config;
	//me->reload_config_interval=reload_config_interval;
	me->last_load_config_time=0;
	
	syslog( LOG_DEBUG, "set_sample_load_conf  sample:%s  load_config_func:%p\n",me->name, load_config );
	return AS_RTN_OK;

}

void *get_subsample_user_data( ac_sample_t *me )
{
	if ( NULL==me || NULL==me->multi_sample || NULL==me->multi_sample->curr_sample )
	{
		return NULL;
	}
	
	syslog( LOG_DEBUG, "get_subsample_user_data:  sample name=%s  user_data=%p", me->name , me->multi_sample->curr_sample->subuser_data );
	return me->multi_sample->curr_sample->subuser_data;
}


int set_multi_sample_user_data_free ( multi_sample_t *multi_sample, USER_DATA_FREE subuser_data_free )
{
	if ( NULL==multi_sample )
	{
		syslog(LOG_INFO,"set_multi_sample_user_data_free: multi_sample is NULL");
		return AS_RTN_NULL_POINTER;
	}

	multi_sample->subuser_data_free=subuser_data_free;

	return AS_RTN_OK;
}

int set_multi_sample_generate_md ( multi_sample_t *multi_sample, GENERATE_MATCH_WORD generate_match_word )
{
	if ( NULL==multi_sample )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	multi_sample->generate_match_word=generate_match_word;
	
	return AS_RTN_OK;
}

int set_multisample_conf( ac_sample_t *me, LOAD_CONFIG load_conf, unsigned int reload_config_interval, USER_DATA_FREE subuser_data_free, GENERATE_MATCH_WORD generate_match_word)
{
	if ( NULL==me )
	{	
		return AS_RTN_NULL_POINTER;
	}
	create_multi_sample( me );
	set_sample_mode ( me, SAMPLE_MODE_MULTI );
	set_sample_load_conf ( me, load_conf );
	set_sample_reload_config_interval( me, reload_config_interval );
	set_multi_sample_user_data_free ( me->multi_sample, subuser_data_free );
	set_multi_sample_generate_md ( me->multi_sample, generate_match_word );
	return AS_RTN_OK;

}

int get_sample_info( ac_sample_t *me, int *latest, int *average, int *max )
{
	int iret = AS_RTN_OK;
	
	if( NULL == me )
	{
		syslog( LOG_ERR, "get_sample_info  me is NULL!" );
		return AS_RTN_NULL_POINTER;
	}

	if( NULL == me->get_sample_info )
	{
		
		if( NULL != latest )
		{
			*latest = sw_get_latest(me->win);
		}
		if( NULL != average )
		{
			*average = sw_get_average(me->win);
		}	
		if( NULL != max )
		{
			*max = sw_get_max(me->win);
		}
	}
	else
	{
		iret = me->get_sample_info( latest, average, max );
	}


	return iret;
}


static int send_threshold_signal( ac_sample_t *me, int type )
{
	me->on_over_threshold( me, type );
	me->last_over_threshold_check_state = type;
	if( NOT_OVER_THRESHOLD == type )
	{
		me->last_over_threshold_call_time = 0;
	}
	else
	{
		me->last_over_threshold_call_time = time(NULL);
	}

    return AS_RTN_OK;
}


/*to check the if the sample should send single for trap or  trap clear!*/
/*this has a complex logic, how to get simple!!?*/
static int check_over_threshold( ac_sample_t *me )
{
	unsigned int time_now=0;

    syslog( LOG_DEBUG, "sample %s threshold =%d, on_over_threshold=%p\n", 
    							me->name, me->threshold, me->on_over_threshold );
    if( me->threshold > 0 &&
        NULL != me->on_over_threshold )
    {
    	
        if( OVER_THRESHOLD_FLAG == do_threshold_check(me) )
        {
        	time_now = time(NULL);
			
        	syslog( LOG_INFO, "%s  check over threshold!  threshold: %d  latest:%d",
									me->name, me->threshold, sw_get_latest(me->win) );

			if( OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY == me->over_threshold_check_type  )
			{
				send_threshold_signal(me, OVER_THRESHOLD_FLAG );
			}
			else if( 0 == me->resend_interval)/*do not resend if resend signal inverval is 0*/
			{
				if( NOT_OVER_THRESHOLD == me->last_over_threshold_check_state )
				{
					send_threshold_signal(me, OVER_THRESHOLD_FLAG );
				}
				else
				{
					syslog( LOG_INFO, "%s not resend signal because resend interval is 0", me->name );
				}
			}
			else if( (time_now-me->last_over_threshold_call_time > me->resend_interval) )/*check signal resend time!*/
			{
				if( NOT_OVER_THRESHOLD == me->last_over_threshold_check_state )
				{
					syslog( LOG_INFO, "%s call callback to send signal!", me->name );
				}
				else
				{
					syslog( LOG_INFO, "%s call callback to resend signal!  resend!", me->name );
				}
				send_threshold_signal(me, OVER_THRESHOLD_FLAG );
				
			}
			else
			{
				syslog( LOG_INFO, "%s is not on resend signal time! ", me->name );	
				syslog( LOG_INFO, "%s resend signal interval %d! last send time %d, now time %d, div %d", 
										me->name, 
										me->resend_interval,
										me->last_over_threshold_call_time,
										time_now,
										time_now-me->last_over_threshold_call_time);
			}

		}
		else /*check if need  send clear signal*/
		{
			if( OVER_THRESHOLD_FLAG == me->last_over_threshold_check_state )
			{
				syslog( LOG_INFO, "%s send trap clear signal, latest :%d", 
										me->name, sw_get_latest(get_sample_window(me)));
				send_threshold_signal(me, NOT_OVER_THRESHOLD );
			}
		}
        
    }
	else if( NULL != me->on_over_threshold && 
			 OVER_THRESHOLD_FLAG == me->last_over_threshold_check_state )/*user might set me->threshold to 0 to get clear!*/
	{
		syslog( LOG_INFO, "%s send trap clear signal because threshold set to 0!!" );
		send_threshold_signal( me, NOT_OVER_THRESHOLD );
	}

	return AS_RTN_OK;
	
}

#if 0
int init_config_list ( struct list_head *head )
{
	if ( NULL==head )
	{
		return AS_RTN_NULL_POINTER;
	}

	//LIST_HEAD_INIT ( *head );

	return AS_RTN_OK;
}
#endif


int
get_master_instance_para(instance_parameter **paraHead)
{
	syslog(LOG_DEBUG, "enter get_master_instance_para\n");
	unsigned int local_slotID = 0;
	if (VALID_DBM_FLAG == get_dbm_effective_flag()) {
		local_slotID = sample_get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if (0 == local_slotID) {
		return -1;
	}

	int ret = 0;
	struct Hmd_Board_Info_show *instance_head = NULL;
	ret = show_broad_instance_info(ac_sample_dbus_get_active_connection(), &instance_head);
	syslog(LOG_DEBUG, "get_master_instance_para: after show_broad_instance_info, ret = %d\n", ret);
	if (1 == ret && instance_head) {
		struct Hmd_Board_Info_show *instance_node = NULL;
		for (instance_node = instance_head->hmd_board_list; NULL != instance_node; instance_node = instance_node->next) {
			unsigned int instance_state = 0;
			int manual_ret = AC_MANAGE_SUCCESS;

			if (local_slotID == instance_node->slot_no) {
				manual_ret = ac_manage_show_snmp_manual_instance(ac_sample_dbus_get_connection(), &instance_state); 
			} else {
				manual_ret = ac_manage_show_snmp_manual_instance(ac_sample_dbus_get_slotconnection(instance_node->slot_no), &instance_state);
			}
			syslog(LOG_DEBUG,"get_all_portal_conf: after show slot %d manual set instance state: manual_ret = %d, instance_state = %d\n",
			       instance_node->slot_no, manual_ret, instance_state);

			int i = 0;
			for(i = 0; i < instance_node->InstNum; i++) {
				if(0 >= instance_node->Hmd_Inst[i]->Inst_ID) {
					continue;
				}

				if ((1 == instance_node->Hmd_Inst[i]->isActive) ||
				    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Inst[i]->Inst_ID - 1))))) {
				 	instance_parameter *paraNode = (instance_parameter *)malloc(sizeof(instance_parameter));
					if (NULL == paraNode) {
						syslog(LOG_WARNING, "get_master_instance_para: malloc remote instance %d-%d parameter fail\n",
					                         instance_node->slot_no, instance_node->Hmd_Inst[i]->Inst_ID);
						continue;
					}

					paraNode->parameter.instance_id = instance_node->Hmd_Inst[i]->Inst_ID;
					paraNode->parameter.local_id = 0;
					paraNode->parameter.slot_id = instance_node->slot_no;
					paraNode->connection = ac_sample_dbus_get_slotconnection(instance_node->slot_no);
					syslog(LOG_DEBUG, "get_master_instance_para: remote hansi %d-%d is master\n", 
					                    instance_node->slot_no, instance_node->Hmd_Inst[i]->Inst_ID);

					paraNode->next = *paraHead;
					*paraHead = paraNode;
				}
			}

			for(i = 0; i < instance_node->LocalInstNum; i++) {
				if(0 >= instance_node->Hmd_Local_Inst[i]->Inst_ID) {
					continue;
				}

				if((1 == instance_node->Hmd_Local_Inst[i]->isActive) ||
				    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Local_Inst[i]->Inst_ID + INSTANCE_NUM - 1))))) {
					instance_parameter *paraNode = (instance_parameter *)malloc(sizeof(instance_parameter));
					if (NULL == paraNode) {
						syslog(LOG_WARNING, "get_master_instance_para: malloc local instance %d-%d parameter fail\n",
					                         instance_node->slot_no, instance_node->Hmd_Local_Inst[i]->Inst_ID);
						continue;
					}

					paraNode->parameter.instance_id = instance_node->Hmd_Local_Inst[i]->Inst_ID;
					paraNode->parameter.local_id = 1;
					paraNode->parameter.slot_id = instance_node->slot_no;
					paraNode->connection = ac_sample_dbus_get_slotconnection(instance_node->slot_no);
					syslog(LOG_DEBUG, "get_master_instance_para: local hansi %d-%d is master\n", 
					                    instance_node->slot_no, instance_node->Hmd_Local_Inst[i]->Inst_ID);
					                    
					paraNode->next = *paraHead;
					*paraHead = paraNode;
				}
			}
		}
	}

	free_broad_instance_info(&instance_head);
	return 0;	
}

void
free_master_instance_para(instance_parameter **paraHead) 
{
    if(NULL == paraHead)
        return ;

    while(*paraHead) {
        instance_parameter *paraNode = (*paraHead)->next;
        free(*paraHead);
        *paraHead = paraNode;
    }
    return ;
}

int create_config_data_1 ( multi_sample_t *multi_sample, struct list_head *head, int match_word_len, char *match_word, void *subuser_data)
{
	if ( NULL==multi_sample || NULL==head )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	subsample_config_t *subsample_conf=NULL;
	
	subsample_conf = (subsample_config_t*)SAMPLE_MALLOC(sizeof(subsample_config_t));
	if ( NULL==subsample_conf )
	{
		return AS_RTN_MALLOC_ERR;
	}
	memset ( subsample_conf, 0, sizeof(subsample_config_t));
	
	if ( NULL!=multi_sample->generate_match_word )
	{
		subsample_conf->match_word = multi_sample->generate_match_word( match_word, match_word_len );
	}

	subsample_conf->subuser_data=subuser_data;

	list_add_tail( LPNLNODE(subsample_conf), head );

	return AS_RTN_OK;
}

int create_config_data_2 ( multi_sample_t *multi_sample, struct list_head *head, unsigned int match_word, void *subuser_data)
{
	if ( NULL==multi_sample || NULL==head )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	subsample_config_t *subsample_conf=NULL;
	
	subsample_conf = (subsample_config_t*)SAMPLE_MALLOC(sizeof(subsample_config_t));
	if ( NULL==subsample_conf )
	{
		return AS_RTN_MALLOC_ERR;
	}
	memset ( subsample_conf, 0, sizeof(subsample_config_t));
	
	subsample_conf->match_word=match_word;

	subsample_conf->subuser_data=subuser_data;

	list_add_tail( LPNLNODE(subsample_conf), head );

	return AS_RTN_OK;
}


//add only for portal and radius. filter as ip and save hansi info
int create_config_data_3( multi_sample_t *multi_sample, struct list_head *head, unsigned int match_word, void *subuser_data, 
				unsigned int slotid, unsigned int localid, unsigned int insid)
{
	if ( NULL==multi_sample || NULL==head || 0 == slotid) {
		return AS_RTN_NULL_POINTER;
	}

	subsample_config_t *subsample_conf = NULL;
	subsample_config_t *tmp = NULL;
	unsigned int (*hansiinfo)[2][16] = NULL;

	list_for_each_entry(tmp, head, node) {
		if (match_word == tmp->match_word) {
			hansiinfo = tmp->subuser_data;
			hansiinfo[slotid-1][localid][insid-1] = 1;				//save hansi info
			return -1;
		} else if (match_word < tmp->match_word) {
			break;
		}
	}
	subsample_conf = (subsample_config_t*)SAMPLE_MALLOC(sizeof(subsample_config_t));
	if (NULL==subsample_conf) {
		return AS_RTN_MALLOC_ERR;
	}
	memset (subsample_conf, 0, sizeof(subsample_config_t));

	subsample_conf->match_word=match_word;
	subsample_conf->subuser_data=subuser_data;
	hansiinfo = subsample_conf->subuser_data;
	hansiinfo[slotid-1][localid][insid-1] = 1;	
	if (list_is_last(&(tmp->node), head)) {
		list_add(LPNLNODE(subsample_conf), &(tmp->node));
	} else {
		list_add_tail(LPNLNODE(subsample_conf), &(tmp->node));
	}
	return AS_RTN_OK;
}

int destroy_config_list ( struct list_head *head )
{
	syslog(LOG_DEBUG,"destroy_config_list: entry.");
	
	if ( NULL==head )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	struct list_head *pos=NULL;
	struct list_head *next=NULL;

	list_for_each_safe(pos,next,head)
	{
		subsample_config_t *curr_config = SUBSAMPLECF ( pos );
		list_del ( LPNLNODE( curr_config ) );
		//subuser_data_free( curr_config->subuser_data );
		//curr_config->subuser_data=NULL;
		SAMPLE_FREE( curr_config );
	}
	
	syslog(LOG_DEBUG,"destroy_config_list: exit.");
	return AS_RTN_OK;
}

subsample_data_t *create_subsample(  unsigned int match_word,
                                unsigned int sample_interval,
                                unsigned int statistics_time,
				   void *subuser_data )
{
    subsample_data_t *pas=NULL;
    int sw_size = 0;
    
    if( sample_interval < MIN_SAMPLE_INTERVAL ||
        statistics_time < MIN_STATISTICS_TIME )        
    {
        return NULL;
    }
    
    pas = (subsample_data_t *)SAMPLE_MALLOC( sizeof(subsample_data_t) );
    if( NULL != pas )
    {
        memset( pas, 0, sizeof(subsample_data_t) );
        pas->match_word=match_word;
       
        sw_size = (statistics_time + sample_interval-1)/sample_interval;
        pas->win = create_sample_window( sw_size );
        if( NULL == pas->win )
        {
            SAMPLE_FREE( pas );
            pas = NULL;
        }
#if 0
        pas->avlwin = create_sample_window( AVL_WINDOW_LEN );
        if( NULL == pas->avlwin )
        {
            destroy_sample_window( pas->win );
            free( pas );
            pas = NULL;            
        }
#endif		
        pas->subuser_data=subuser_data;
        //pas->subuser_data_free=subuser_data_free;
        pas->last_over_threshold_call_time=0;
        pas->last_over_threshold_check_state=NOT_OVER_THRESHOLD;
        set_subsample_flag ( pas , SUBSAMPLE_DO ); //for destroy test
        return pas;
    }
        
    return NULL;
}

int register_subsample( multi_sample_t *multi_sample, subsample_data_t *subsample )
{
	if( (NULL != multi_sample) && (NULL != subsample) && !is_entry_in_list(LPNLNODE(subsample),&(multi_sample->head)) )
	{
		list_add( LPNLNODE(subsample), &(multi_sample->head) );
		syslog( LOG_DEBUG, "register_subsample %p success!", subsample); //test
      		return AS_RTN_OK;
	}

	return AS_RTN_COUNTER_INSTANCE_IN_LIST;
}

int unregister_subsample( multi_sample_t *multi_sample, subsample_data_t *subsample )
{
    if( (NULL != multi_sample) && (NULL != subsample) && is_entry_in_list(LPNLNODE(subsample), &(multi_sample->head)) )
    {
        list_del(LPNLNODE(subsample));
        syslog( LOG_DEBUG, "unregister_subsample %p success!", subsample);
        return AS_RTN_OK;
    }
    
    return AS_RTN_COUNTER_INSTANCE_NOT_IN_LIST;
}

int set_sample_mode( ac_sample_t *me, int sample_mode )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }

	me->sample_mode = sample_mode;

	return AS_RTN_OK;
}

sample_window_t *get_subsample_window( ac_sample_t *me )
{
    if( NULL == me || NULL == me->multi_sample || NULL == me->multi_sample->curr_sample || NULL == me->multi_sample->curr_sample->win )
    {
        return NULL;
    }

    
    return me->multi_sample->curr_sample->win;
}

int set_subsample_flag( subsample_data_t * sub_sample , int flag )	//0 for clear and canceled
{
	if ( NULL==sub_sample )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	sub_sample->flag=flag;
	
	return AS_RTN_OK;
}

int get_subsample_flag( subsample_data_t * sub_sample )	//0 for clear and canceled
{
	if ( NULL==sub_sample )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	return sub_sample->flag;
}

void clear_subsample_flag( multi_sample_t *multi_sample )
{
	syslog(LOG_DEBUG,"clear_subsample_flag entry");
	struct list_head *pos=NULL;
	struct list_head *next=NULL;

	list_for_each_safe(pos,next,&(multi_sample->head))
	{
		subsample_data_t * curr_sample = NULL;
		curr_sample = MTSAMPLE ( pos );
		set_subsample_flag ( curr_sample , SUBSAMPLE_CLEAR );
		syslog(LOG_DEBUG,"set_subsample_flag SUBSAMPLE_CLEAR");
	}
	syslog(LOG_DEBUG,"clear_subsample_flag exit");
	return;
}

unsigned int generate_match_word(const char *buff, unsigned int len )
{
	if (NULL == buff || 0 == len )
	{	
		return 0;
	}

	unsigned int uint32_hash_value = 0;
		
	while (4 <= len)
	{
		uint32_hash_value += *((unsigned int*)buff);
		buff += 4;
		len -= 4;
	}

	while (0 < len)
	{
		uint32_hash_value += (*buff) << ((len-1)*8);
		buff++;
		len--;
	}
	
	return uint32_hash_value;
}

int destroy_subsample ( multi_sample_t *multi_sample, subsample_data_t *subsample)
{
	if ( NULL==subsample )
	{
		return AS_RTN_NULL_POINTER;
	}

	if ( NULL!=multi_sample->subuser_data_free )
	{
		multi_sample->subuser_data_free( subsample->subuser_data );
		subsample->subuser_data=NULL;
	}
	
	SAMPLE_FREE(subsample);
		
	return AS_RTN_OK;
}

int clear_canceled_subsample( multi_sample_t *multi_sample )
{
	syslog(LOG_DEBUG,"clear_canceled_subsample:entry.");
	
	struct list_head *pos=NULL;
	struct list_head *next=NULL;
		
	list_for_each_safe( pos, next, &(multi_sample->head) )
	{
		subsample_data_t *curr_subsample=MTSAMPLE( pos );
		
		if (  SUBSAMPLE_DO!=get_subsample_flag ( curr_subsample ) )
		{
			unregister_subsample( multi_sample, curr_subsample );
			destroy_subsample( multi_sample, curr_subsample );
		}
	}

	syslog(LOG_DEBUG,"clear_canceled_subsample:exit.");
	return AS_RTN_OK;
}

void debug_sample_list ( struct list_head *head )
{	
	syslog(LOG_DEBUG,"debug_sample_list: entry.");
	
	struct list_head *pos=NULL;
	struct list_head *next=NULL;
	int i=0;
	
	if ( NULL==head )
	{
		syslog(LOG_DEBUG,"debug_sample_list: head is NULL");
		return;
	}

	list_for_each_safe( pos, next, head)
	{
		i++;
		syslog(LOG_DEBUG,"debug_sample_list: num (%d) point (%p).",i , pos);

	}
	
	syslog(LOG_DEBUG,"debug_sample_list: exit.");
	return;
}

int subsample_match_set_flag ( ac_sample_t *me, struct list_head *subsample_config_head)	//free config_list subuser_data 
{
	//syslog(LOG_INFO,"subsample_match_set_flag entry."); //test

	if ( NULL==me || NULL==subsample_config_head || NULL==me->multi_sample )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	struct list_head *pos_config=NULL;
	struct list_head *next_config=NULL;
		
	list_for_each_safe( pos_config, next_config, subsample_config_head)
	{
		int flag=0;
		subsample_config_t *curr_sample_config = SUBSAMPLECF ( pos_config );
		subsample_data_t *curr_subsample=NULL;
		
		struct list_head *pos_subsample=NULL;
		struct list_head *next_subsample=NULL;
		syslog(LOG_DEBUG,"head %p me->multi_sample->head {%p,%p}",&(me->multi_sample->head), me->multi_sample->head.prev,me->multi_sample->head.next);
		
		list_for_each_safe( pos_subsample, next_subsample, &(me->multi_sample->head) )
		{
			curr_subsample=MTSAMPLE( pos_subsample );
			
			if ( curr_subsample->match_word==curr_sample_config->match_word )
			{
				flag=1;
				set_subsample_flag ( curr_subsample , SUBSAMPLE_DO );
				break;
			}
		}
		
		if ( 1==flag )
		{
			if ( NULL!=me->multi_sample->subuser_data_free )
			{
				me->multi_sample ->subuser_data_free( curr_sample_config->subuser_data );
				curr_sample_config->subuser_data=NULL;
			}
			
			continue;
		}
		
		subsample_data_t *subsample=NULL;
		subsample=create_subsample( curr_sample_config->match_word, 
							me->sample_interval, 
							me->statistics_time, 
							curr_sample_config->subuser_data );
		
		if ( NULL==subsample )
		{
			syslog(LOG_INFO,"create_subsample failed: return NULL!\n");
			continue; 
		}
		register_subsample( me->multi_sample, subsample );
		
	}

	debug_sample_list(&(me->multi_sample->head));
	
	//syslog(LOG_INFO,"subsample_match_set_flag exit.");  //test
	return AS_RTN_OK;
}

static int check_multi_sample ( ac_sample_t *me )	//maybe a long time check once better
{
	//mtrace();	//test memory leak
	//syslog(LOG_INFO,"check_multi_sample entry.");
#if 0
	if ( NULL==me->load_config )
	{
		syslog(LOG_INFO,"load_config is NULL");
		return AS_RTN_NULL_POINTER;
	}
#endif

	clear_subsample_flag ( me->multi_sample );
	
	struct list_head config_head = LIST_HEAD_INIT(config_head);
	
	if ( 0 != me->load_config ( me->multi_sample, &config_head ))
	{
		return AS_RTN_LOAD_CONFIG_ERR;
	}
	
	subsample_match_set_flag( me , &config_head );

	clear_canceled_subsample( me->multi_sample );

	destroy_config_list( &config_head );
	//syslog(LOG_INFO,"check_multi_sample exit.");
	//muntrace();	//test memory leak
	return AS_RTN_OK;
}


static int dynamic_load_config( ac_sample_t *me )
{
	int ret=-1;
			
	if ( NULL==me || NULL==me->load_config )
	{
		syslog(LOG_INFO,"dynamic_load_config: %s",NULL==me?"me is NULL":"me->load_config is NULL" );
		return AS_RTN_NULL_POINTER;
	}

	if ( 1!=me->already_loaded_config )
	{
		if ( me->reload_config_interval > 0 )
		{
			unsigned int time_now=time(NULL);;
			if ( time_now-me->last_load_config_time > me->reload_config_interval )
			{
				if ( ( ret=check_multi_sample( me ) ) )
				{	
					syslog(LOG_INFO,"check_multi_sample:failed return %d",ret );
					return AS_RTN_LOAD_CONFIG_ERR;
				}
				me->last_load_config_time=time_now;
			}
		}
	}else
	{
		if ( ( ret=check_multi_sample( me ) ) )
		{	
			syslog(LOG_INFO,"check_multi_sample:failed return %d", ret );
			return AS_RTN_LOAD_CONFIG_ERR;
		}
		me->last_load_config_time=time(NULL);
		me->already_loaded_config=1;
	}
	
	return AS_RTN_OK;
}

static int set_curr_sample ( multi_sample_t *multi_sample, subsample_data_t *curr_sample )
{
	if ( NULL == multi_sample || NULL == curr_sample )
		return AS_RTN_NULL_POINTER;

	multi_sample->curr_sample=curr_sample;

	return AS_RTN_OK;
}

void log_subsample_data( ac_sample_t *pas )
{
	syslog( LOG_DEBUG, "sample name:%s", pas->name );
	log_sample_win( pas->multi_sample->curr_sample->win );

	return;	
}

static int subsample_over_threshold_check_immediately( ac_sample_t *me )
{
	if (NULL==me)
	{
		syslog(LOG_INFO,"subsample_over_threshold_check_immediately: NULL==me");
		return NOT_OVER_THRESHOLD;	
	}
	
	sample_window_t *win = me->multi_sample->curr_sample->win;
	if (NULL==win  || (win->val_num <= 0) )
	{
		syslog(LOG_INFO,"subsample_over_threshold_check_immediately: NULL==win  || (win->val_num <= 0)");
		return NOT_OVER_THRESHOLD;
	}
	
	if (NULL != me->do_check)
	{
		syslog(LOG_INFO,"subsample_over_threshold_check_immediately: NULL != me->do_check");
		return me->do_check(me->threshold, win->vals[win->val_num-1]); 
	}

	syslog(LOG_DEBUG,"value:%d threshold %d", win->vals[win->val_num-1], me->threshold );  //test

    if( win->val_num > 0 &&
        (win->vals[win->val_num-1] > me->threshold) )
    {
        return OVER_THRESHOLD_FLAG;
    }
    
    return NOT_OVER_THRESHOLD;
}



static int subsample_over_threshold_check_normal( ac_sample_t *me )
{
    //int i,count;
    if (NULL==me)
    {
		return NOT_OVER_THRESHOLD;
	}

	syslog( LOG_DEBUG, "over_threshold_check_normal win->val_num=%d\n", me->multi_sample->curr_sample->win->val_num );
    //if( me->win->val_num >= NORMAL_CHECK_TIMES )
    {
#if 0    
        count = 0;
        
        for( i=me->win->val_num-1; i>=(int)(me->win->val_num-NORMAL_CHECK_TIMES); i-- )
        {
        	syslog( LOG_DEBUG, "over_threshold_check_normal me->win->vals[%d]=%d", i, me->win->vals[i] );
            if( me->win->vals[i] > me->threshold )
            {
                count++;
            }
        }
        
        if( count >= NORMAL_CHECK_COUNT )
        {
            return OVER_THRESHOLD_FLAG;
        }
#else		
		sample_window_t *win = me->multi_sample->curr_sample->win;
		
		if (NULL!=me->do_check)
		{
			return me->do_check(me->threshold, sw_get_average(win) );
		}
		
		if( sw_get_average(win) > me->threshold )
		{
			return OVER_THRESHOLD_FLAG;
		}
#endif
    }

    return NOT_OVER_THRESHOLD;
}


static int do_subsample_threshold_check( ac_sample_t *me )
{
    int iret = NOT_OVER_THRESHOLD;
    int over_threshold_check_type = 0;
    
    if( NULL == me|| NULL==me->multi_sample || NULL==me->multi_sample->curr_sample )
    {
        return NOT_OVER_THRESHOLD;
    }

    switch ( me->multi_sample->curr_sample->last_over_threshold_check_state )
    {
	case OVER_THRESHOLD_FLAG:
		over_threshold_check_type=me->clear_check_type;
		break;
	case NOT_OVER_THRESHOLD:
		over_threshold_check_type=me->over_threshold_check_type;
		break;
	default:
		break;
    }
    
    switch( over_threshold_check_type )
    {
        case OVER_THRESHOLD_CHECK_TYPE_NORMAL:
            iret = subsample_over_threshold_check_normal( me );
            break;
        case OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY:
            iret = subsample_over_threshold_check_immediately( me );
            break;
        default:
            break;
    }
    
    return iret;
}

static int subsample_send_threshold_signal( ac_sample_t *me, int type )
{
	if ( NULL==me->multi_sample->curr_sample )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	subsample_data_t * curr_sample = me->multi_sample->curr_sample;
	
	me->on_over_threshold( me, type );
	curr_sample->last_over_threshold_check_state = type;
	if( NOT_OVER_THRESHOLD == type )
	{
		curr_sample->last_over_threshold_call_time = 0;
	}
	else
	{
		curr_sample->last_over_threshold_call_time = time(NULL);
	}

    return AS_RTN_OK;
}

static int check_subsample_over_threshold( ac_sample_t *me )
{
	unsigned int time_now=0;
	subsample_data_t * curr_sample = me->multi_sample->curr_sample;

    syslog( LOG_DEBUG, "sample %s threshold =%d, on_over_threshold=%p, curr_sample=%p last_over_threshold_call_time=%d last_over_threshold_check_state=%d.\n", 
    							me->name, me->threshold, me->on_over_threshold, curr_sample ,curr_sample->last_over_threshold_call_time, curr_sample->last_over_threshold_check_state );	//test
    if( me->threshold > 0 &&
        NULL != me->on_over_threshold )
    {
    	
        if( OVER_THRESHOLD_FLAG == do_subsample_threshold_check(me) )
        {
        	time_now = time(NULL);
			
        	syslog( LOG_INFO, "%s  check over threshold!  threshold: %d  latest:%d",
									me->name, me->threshold, sw_get_latest(curr_sample->win) );

		if( OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY == me->over_threshold_check_type  )
		{
			subsample_send_threshold_signal(me, OVER_THRESHOLD_FLAG );
		}
		else if( 0 == me->resend_interval)/*do not resend if resend signal inverval is 0*/
		{
			if( NOT_OVER_THRESHOLD == curr_sample->last_over_threshold_check_state )
			{
				subsample_send_threshold_signal(me, OVER_THRESHOLD_FLAG );
			}
			else
			{
				syslog( LOG_INFO, "%s not resend signal because resend interval is 0", me->name );
			}
		}
		else if( (time_now-curr_sample->last_over_threshold_call_time > me->resend_interval) )/*check signal resend time!*/
		{
			if( NOT_OVER_THRESHOLD == curr_sample->last_over_threshold_check_state )
			{
				syslog( LOG_INFO, "%s call callback to send signal!", me->name );
			}
			else
			{
				syslog( LOG_INFO, "%s call callback to resend signal!  resend!", me->name );
			}
			subsample_send_threshold_signal(me, OVER_THRESHOLD_FLAG );
			
		}
		else
		{
			syslog( LOG_INFO, "%s is not on resend signal time! ", me->name );	
			syslog( LOG_INFO, "%s resend signal interval %d! last send time %d, now time %d, div %d", 
									me->name, 
									me->resend_interval,
									curr_sample->last_over_threshold_call_time,
									time_now,
									time_now-curr_sample->last_over_threshold_call_time);
		}
	}else		/***check if need  send clear signal***/
	{
		if( OVER_THRESHOLD_FLAG == curr_sample->last_over_threshold_check_state )
		{
			syslog( LOG_INFO, "%s send trap clear signal, latest :%d", 
									me->name, sw_get_latest(get_subsample_window(me)));
			subsample_send_threshold_signal(me, NOT_OVER_THRESHOLD );
			sw_reset (curr_sample->win);
		}
	}
        
    }
	else if( NULL != me->on_over_threshold && 
			 OVER_THRESHOLD_FLAG == curr_sample->last_over_threshold_check_state )/*user might set me->threshold to 0 to get clear!*/
	{
		syslog( LOG_INFO, "%s send trap clear signal because threshold set to 0!!", me->name );
		subsample_send_threshold_signal( me, NOT_OVER_THRESHOLD );
	}

	return AS_RTN_OK;
	
}

int do_sample_once( ac_sample_t *me )
{
    int this_sample = 0;
    
    if( NULL==me || NULL==me->do_sample )	// || NULL==me->win )
    {
        return AS_RTN_NULL_POINTER;
    }

	syslog(LOG_DEBUG,"do_sample_once: sample name (%s) sample mode (%s)\n", me->name, me->sample_mode==1?"multi sample mode":"single sample mode"); //test

	if ( NULL==me->win )
	{
		return AS_RTN_NULL_POINTER;
	}
	if( SAMPLE_ON != me->sample_on )
	{
		syslog( LOG_DEBUG, "sample %s is not on!", me->name );
		return AS_RTN_OK;
	}

	if ( SAMPLE_MODE_SINGLE==me->sample_mode )
	{

		this_sample = me->do_sample(me);
		me->last_sample_time = time(NULL);

		sw_push_val( me->win, this_sample );

		syslog( LOG_DEBUG, "-------------------" );
		syslog( LOG_DEBUG, "do_sample_once:" );
		log_sample_data(me);
#if 0	
		sw_push_val( me->avlwin, sw_get_average(me->win) );
#endif

		check_over_threshold( me );
	}else if ( SAMPLE_MODE_MULTI==me->sample_mode )
	{
		//syslog(LOG_INFO,"multi sample entry."); //test
		if ( NULL==me->multi_sample )
		{
			syslog(LOG_INFO,"multi sample is NULL"); //test
			return AS_RTN_NULL_POINTER;
		}

		dynamic_load_config( me );
		
		//if ( list_empty_careful( &(me->multi_sample->head) ) )	//list_for_each_safe can check this
		//	return AS_RTN_OK;
		
		me->last_sample_time = time(NULL);
		
		struct list_head *pos=NULL;
		struct list_head *next=NULL;
		
		list_for_each_safe(pos,next,&(me->multi_sample->head))
		{
			subsample_data_t * curr_sample = NULL;
			curr_sample = MTSAMPLE ( pos );
			set_curr_sample( me->multi_sample , curr_sample );
			
			this_sample = me->do_sample(me);
			sw_push_val( curr_sample->win, this_sample );
			
			syslog( LOG_DEBUG, "-------------------" );	//test
			syslog( LOG_DEBUG, "do_subsample_once: sample name %s curr_sample %p ret %d", me->name, curr_sample, this_sample ); //test
			log_subsample_data( me );
#if 0	
			sw_push_val( me->avlwin, sw_get_average(me->win) );
#endif

			check_subsample_over_threshold( me );
		}

	}else
	{
		syslog(LOG_INFO,"do_sample_once: Unknown samle_mode!\n");
		return AS_RTN_SAMPLE_MODE_ERR;
	}
		return AS_RTN_OK;
}


unsigned int get_last_sample_time( ac_sample_t  *me )
{
    if( NULL == me )
    {
        return AS_RTN_NULL_POINTER;
    }

    return me->last_sample_time;
}

#if _AC_SAMPLE_TEST_

#define TEST_MAX_SAMPLE_VAL  100
#define TEST_SAMPLE_THROSHOLD   90

int do_sample( ac_sample_t *me )
{
    int sample;
    static int count = 0;
    
    sample = rand()%TEST_MAX_SAMPLE_VAL;
    
    count ++;
    
    
    return sample;
}

int on_over_threshold( ac_sample_t *me )
{
    int valid_size = 0;
    int *vals = NULL;
    int i;
    
    printf( "***************over threshold************\n" );
    printf( "average = %d\n", get_average_val(me) );
    printf( "max = %d\n", get_max_val(me) );
    valid_size = get_window_val( me, &vals );
    printf( "val list:   num=%d\n", valid_size );
    for( i=0; i<valid_size; i++ )
    {
        printf( ": %d :", vals[i] );
    }
    printf("\n*****************************************\n");
    
    return 0;
}


int main()
{
    int times = 200;
    ac_sample_t *pas = NULL;
    
	
    pas = create_ac_sample(  "cpu",
                               5, 
                               19,
                               10);
	set_do_sample_cb( pas, do_sample );
	set_over_threshold_cb( pas, on_over_threshold );
    
    
    set_sample_threshold( pas, TEST_SAMPLE_THROSHOLD );
    if( NULL == pas )
    {
        printf("create sample instance failed!\n");
        return 0;
    }
    
    while( times > 0 )
    {
        do_sample_once( pas );
        sleep(1);
        times --;
    }
    
    destroy_ac_sample( pas );
    
    return 0;
}

#endif

