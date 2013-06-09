#ifndef _AC_SAMPLE_H
#define _AC_SAMPLE_H
#include "ac_sample_def.h"
#include "ws_dbus_list_interface.h"
extern struct timeval gTimeout;

typedef struct ac_sample_t ac_sample_t;
typedef struct sample_window_t sample_window_t;
typedef struct subsample_data_t subsample_data_t;
typedef struct multi_sample_t multi_sample_t;
typedef struct subsample_config_t subsample_config_t;



#define LPSAMPLE(p)     ((ac_sample_t *)p)
#define MTSAMPLE(p)    ((subsample_data_t *)p)
#define SUBSAMPLECF(p)	((subsample_config_t *)p)

/*callback function*/
/*typedef int (*DO_SAMPLE)( ac_sample_t *this );*/
typedef int (*DO_SAMPLE)( ac_sample_t *me );
typedef int (*ON_SAMPLE_OVER_THRESHOLD) ( ac_sample_t *this, int type );
typedef void (*USER_DATA_FREE)( void *user_data );
typedef int (*GET_SAMPLE_INFO)( int *latest, int *average,  int *max );

typedef int (*DO_CHECK)( int threshold, int sample );

typedef int (*LOAD_CONFIG)( multi_sample_t *multi_sample, struct list_head *head );

typedef unsigned int (*GENERATE_MATCH_WORD)( const char *buff, unsigned int len );




#define OVER_THRESHOLD_CHECK_TYPE_NORMAL            0       /*default*/
#define OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY       1

#define SAMPLE_MALLOC(size) sample_malloc(size,__FILE__,__FUNCTION__,__LINE__)

#define SAMPLE_FREE(s)	do {\
	if (s) {	\
		syslog(LOG_DEBUG,"[%s@%s,%d]free: address is %p\n",__FUNCTION__,__FILE__,__LINE__, s);	\
		free((void *)s);	\
		s=NULL;		\
	}else{	\
		syslog(LOG_INFO,"[%s@%s,%d]free: error ptr is NULL\n",__FUNCTION__,__FILE__,__LINE__);	\
	}	\
} while(0)



ac_sample_t *create_ac_sample(  char *name,
                                unsigned int sample_interval, 
                                unsigned int statistics_time,
                                unsigned int resend_interval);
int destroy_ac_sample( ac_sample_t *me );
const char *get_sample_name( ac_sample_t *me );


int set_sample_state( ac_sample_t *me, int state );
int get_sample_state( ac_sample_t *me );

int set_do_sample_cb( ac_sample_t *me, DO_SAMPLE do_sample );
int set_over_threshold_cb( ac_sample_t *me, ON_SAMPLE_OVER_THRESHOLD over_threshold_cb );
int set_get_sample_info_cb( ac_sample_t *me, GET_SAMPLE_INFO get_sample_info );
int set_over_threshold_cb( ac_sample_t *me, ON_SAMPLE_OVER_THRESHOLD over_threshold_cb );

int set_do_check_cb(ac_sample_t *me, DO_CHECK do_check );

int set_sample_user_data( ac_sample_t *me, void *user_data, USER_DATA_FREE free_cb );
void *get_sample_user_data( ac_sample_t *me );
void *get_sample_cpu_rt_data(ac_sample_t *me);

struct cpu_stat_t *init_sample_user_data_rt(ac_sample_t *me);
cpu_in_t *init_sample_cpu_rt_data(ac_sample_t *me);


int set_over_threshold_type( ac_sample_t *me, int type );


int set_sample_interval( ac_sample_t  *me, unsigned int sample_interval );
int get_sample_interval( ac_sample_t  *me );

int set_statistics_time( ac_sample_t  *me, unsigned int statistics_time );
int get_statistics_time( ac_sample_t  *me );


int set_sample_threshold( ac_sample_t  *me, unsigned int  threshold );
int get_sample_threshold( ac_sample_t  *me );



sample_window_t *get_sample_window( ac_sample_t *me);
int get_sample_info( ac_sample_t *me, int *latest, int *average, int *max );

int
get_master_instance_para(instance_parameter **paraHead);
void
free_master_instance_para(instance_parameter **paraHead);

int create_config_data_3( multi_sample_t *multi_sample, struct list_head *head, unsigned int match_word, void *subuser_data,
					unsigned int slotid, unsigned int localid, unsigned int insid);

unsigned int get_last_sample_time( ac_sample_t  *me );

void *get_subsample_user_data( ac_sample_t *me );

int do_sample_once( ac_sample_t *me );





/*sample window api*/
sample_window_t *create_sample_window( unsigned int size );
void destroy_sample_window( sample_window_t *win );

int sw_set_size( sample_window_t *win, int new_size );
int sw_get_size( sample_window_t *win );

void sw_push_val( sample_window_t *win, int val );
int sw_get_max( sample_window_t *win );
int sw_get_average( sample_window_t *win );
int sw_get_latest( sample_window_t *win );
int sw_get_sample_value( ac_sample_t *me);


int sw_get_data_array( sample_window_t *win, int **array );/*you should free array after use it! return buff int num*/



/*for debug*/
void log_sample_win( sample_window_t *win );
void log_sample_data( ac_sample_t *pas );


/*for user*/
unsigned int generate_match_word(const char *buff, unsigned int len );

/*public api*/

char * inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize );

#define INET_ATON(ipaddr,addr_str)	\
{	\
	unsigned int a1,a2,a3,a4;	\
	int ret;	\
	ret = sscanf(addr_str,"%u.%u.%u.%u",&a1,&a2,&a3,&a4);	\
	if( ret == 4 ){		\
		ipaddr = a1*256*256*256+a2*256*256+a3*256+a4;	\
	}else{	\
		ipaddr=0;	\
	}	\
}


#endif


