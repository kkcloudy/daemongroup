#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "nm_list.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "ac_sample_container.h"

static struct {
    struct list_head    headins;
    struct list_head    *phead;

    unsigned int        sample_interval;
    unsigned int        statistics_time;
	unsigned int 		resend_interval;

	int					service_state;
} sample_container = {
    LIST_HEAD_INIT(sample_container.headins),
    &sample_container.headins,
    DEFAULT_SAMPLE_INTERVAL,
    DEFAULT_STATISTICS_TIME,
    DEFAULT_RESEND_INTERVAL,
    DEFAULT_SERVER_STATE
};


#define     sample_head    sample_container.phead


int init_sample_container()
{
    return AS_RTN_OK;
}


int uninit_sample_container()
{
    ac_sample_t *pas = NULL;
    
    if( NULL == sample_head )
    {
        return AS_RTN_ERR;
    }
    
    /*delete all ac_sample instance*/
    while( !list_empty(sample_head) )
    {
        pas = LPSAMPLE(sample_head->next);
        list_del(LPNLNODE(pas));
        destroy_ac_sample(pas);
    }
    
    return AS_RTN_OK;
}



int set_all_sample_interval( unsigned int sample_interval )		//include cpu memory temperature
{
    struct list_head *pos=NULL;
    ac_sample_t *sample=NULL;
    const char *sample_name=NULL;
        
    sample_container.sample_interval = sample_interval;
    
    list_for_each(pos,sample_head)
    {
    	 sample = LPSAMPLE(pos);
	 sample_name=get_sample_name( sample );
    	 if ( !strcmp ( SAMPLE_NAME_CPU, sample_name )
		 	||!strcmp ( SAMPLE_NAME_MEMUSAGE, sample_name )
		 	||!strcmp ( SAMPLE_NAME_TMP, sample_name ) 
		 	||!strcmp ( SAMPLE_NAME_DHCPUSE, sample_name ))
    	 {
        	set_sample_interval( LPSAMPLE(pos), sample_interval );
    	 }
    }
    
    return AS_RTN_OK;
}

unsigned int get_all_sample_interval()	//include cpu memory temperature
{
	return sample_container.sample_interval;
}

int set_all_statistics_time( unsigned int statistics_time )	//include cpu memory temperature
{
    struct list_head *pos=NULL;
    ac_sample_t *sample=NULL;
    char *sample_name=NULL;
	
    sample_container.statistics_time = statistics_time;

    list_for_each(pos,sample_head)
    {
        	 sample = LPSAMPLE(pos);
		 sample_name=get_sample_name(sample);
    	 if ( !strcmp ( SAMPLE_NAME_CPU, sample_name )
		 	||!strcmp ( SAMPLE_NAME_MEMUSAGE, sample_name )
		 	||!strcmp ( SAMPLE_NAME_TMP, sample_name )
		 	||!strcmp ( SAMPLE_NAME_DHCPUSE, sample_name ))
    	 {
        	 set_statistics_time( LPSAMPLE(pos), statistics_time );
    	 }

    }
    
    return AS_RTN_OK;
}

unsigned int get_all_statistics_time( )	//include cpu memory temperature
{
	return sample_container.statistics_time;
}



int set_all_resend_interval( unsigned int resend_interval )	//all resend interval
{
    struct list_head *pos=NULL;
        
    sample_container.resend_interval = resend_interval;

    list_for_each(pos,sample_head)
    {
        set_resend_interval( LPSAMPLE(pos), resend_interval );
    }
    
    return AS_RTN_OK;
}

unsigned int get_all_resend_interval( )
{
	return sample_container.resend_interval;
}


int set_ac_sample_service_state( int state )
{
    if( SAMPLE_SERVICE_STATE_ON != state &&
		SAMPLE_SERVICE_STATE_OFF != state )
    {
		return AS_RTN_SAMPLE_SERVICE_STATE_ERR;
	}
    sample_container.service_state = state;
    return AS_RTN_OK;
}


int get_ac_sample_service_state( )
{
    return sample_container.service_state;
}



int register_ac_sample( ac_sample_t *pas )
{
    if( (NULL != pas) && !is_entry_in_list(LPNLNODE(pas),sample_head) )
    {
        list_add( LPNLNODE(pas), sample_head );
#if 0
        if( sample_container.sample_interval > 0 )
        {
            set_sample_interval(pas,sample_container.sample_interval);
        }
        if( sample_container.statistics_time > 0 )
        {
            set_statistics_time(pas,sample_container.statistics_time);
        }
#endif
		syslog( LOG_DEBUG, "register_ac_sample %s success!", get_sample_name(pas));
        return AS_RTN_OK;
    }
    
    return AS_RTN_COUNTER_INSTANCE_IN_LIST;
}




int unregister_ac_sample( ac_sample_t *pas )
{
    if( is_entry_in_list(LPNLNODE(pas),sample_head) )
    {
        list_del(LPNLNODE(pas));
        return AS_RTN_OK;
    }
    
    return AS_RTN_COUNTER_INSTANCE_NOT_IN_LIST;
}


ac_sample_t *get_ac_sample_by_name( char *name )
{
    struct list_head *pos=NULL;
    
    list_for_each(pos,sample_head)
    {
        const char *sample_name = get_sample_name(LPSAMPLE(pos));
        //if( 0 == strncmp(name,sample_name,strlen(sample_name) ) )
        if( 0 == strcmp(name,sample_name) )
        {
            return LPSAMPLE(pos);
        }
    }
    
    return NULL;
}

void debug_contain_list(  )
{
    struct list_head *pos=NULL;
	struct list_head *next=NULL;

//    printf( "%s sample_head = %p  next = %p  nnext=%p\n", pos, sample_head, sample_head->next, sample_head->next->next );    
	syslog( LOG_DEBUG, "*******************" );
	syslog( LOG_DEBUG, "sample_head = %p", sample_head );

	list_for_each_safe(pos,next,sample_head)
	{
		syslog( LOG_DEBUG, "pos=%p\n", pos );
		syslog( LOG_DEBUG, "name=%s\n", get_sample_name(LPSAMPLE(pos)) );
		
	}
	syslog( LOG_DEBUG, "*******************" );
	return;
}



int ac_sample_dispach()
{
    struct list_head *pos=NULL;
	struct list_head *next=NULL;
    unsigned int time_now=0;

	if( SAMPLE_SERVICE_STATE_OFF == sample_container.service_state )
	{
		return AS_RTN_OK;
	}
    time_now = time(NULL);

    list_for_each_safe(pos,next,sample_head)/*sample instance might be delete at the callback func, so it should list_for_each_safe  to protect it*/
    {
        int last_sample_time;
        last_sample_time = get_last_sample_time(LPSAMPLE(pos));
		syslog(LOG_DEBUG,"ac_sample_dispach last_sample_time=%d time_now=%d\n", last_sample_time, time_now);
        if( (last_sample_time >= 0) &&
            ((time_now-last_sample_time) > get_sample_interval(LPSAMPLE(pos))) )
        {
        	syslog(LOG_DEBUG, "do sample once name =%s \n", get_sample_name(LPSAMPLE(pos))  );
	        //debug_contain_list();
            do_sample_once( LPSAMPLE(pos) );
			//debug_contain_list();
        }
    }
    
    return AS_RTN_OK;
}

