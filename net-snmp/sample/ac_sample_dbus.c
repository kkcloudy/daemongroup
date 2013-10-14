#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include "board/board_define.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "ws_dbus_def.h"

#include "manage_type.h"
#include "manage_api.h"

#include "ac_manage_def.h"
#include "ac_sample_def.h"
#include "ac_sample_dbus.h"
#include "ac_sample_container.h"
#include "ac_sample_cpu.h"
#include "nm_list.h"
#include "ac_sample_interface_flow.h"
#include "ac_sample_multy.h"
#include "ac_sample.h"

static manage_session *sample_session = NULL;
static DBusConnection *ac_sample_dbus_connection = NULL;
static DBusConnection *ac_sample_tipc_connection[SLOT_MAX_NUM + 1] = { 0 };

static char head[32];//for cpu info get 

static int parse(char *line, struct cpu_stat_t *pc)
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




int
sample_tipc_session_init(void) {
	manage_session session;
	manage_tipc_addr local_tipc;
	memset(&session, 0, sizeof(manage_session));
	memset(&local_tipc, 0, sizeof(manage_tipc_addr));
	
	session.flags |= MANAGE_FLAGS_TIPC_SOCKET;
	unsigned int local_slot_id = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slot_id = sample_get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if(0 == local_slot_id || local_slot_id > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "sample_tipc_session_init: get active master slot id failed!\n");
		return -1;
	}

	local_tipc.type = SAMPLE_TIPC_TYPE;	/*ac sample tipc type*/
	local_tipc.instance = (0x1000 + local_slot_id);

	session.local = (void *)&local_tipc;
	session.local_len = sizeof(manage_tipc_addr);
	
	sample_session = manage_open(&session);
	if(NULL == sample_session) {
		syslog(LOG_ERR, "sample_tipc_session_init: s_manage_errno = %d, s_errno = %d\n",
						session.s_manage_errno, session.s_errno);
		return -1;
	}

	return 0;
}

void
sample_tipc_session_destroy(void) {
	manage_close(sample_session);
	return ;
}

manage_session *
sample_get_tipc_session(void) {
	return sample_session;
}


static DBusConnection *
ac_sample_dbus_get_tipc_connection(unsigned int slot_id) {

	if(0 == slot_id || slot_id > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "ac_sample_dbus_get_tipc_connection fun error: the slot_id is %d\n",slot_id);
		return NULL;
	}
	
	DBusError dbus_error;
	DBusConnection *tempConnection = NULL;
	
	dbus_error_init (&dbus_error);
	
	tempConnection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &dbus_error);
	if(NULL == tempConnection) {
		dbus_error_free(&dbus_error);
		addresses_shutdown_func();
		syslog(LOG_INFO, "The slot %d remote server is not ready.\n", slot_id);
		return NULL;
	}
	
	syslog(LOG_INFO, "The slot %d tipc connection is OK\n", slot_id);
	return tempConnection;	
}


DBusConnection *ac_sample_dbus_get_connection()
{
	return ac_sample_dbus_connection;
}
DBusConnection *ac_sample_dbus_get_slotconnection(int slotid)
{
	if(NULL == ac_sample_tipc_connection[slotid]) {
        ac_sample_tipc_connection[slotid] = ac_sample_dbus_get_tipc_connection(slotid);
    }
	return ac_sample_tipc_connection[slotid];
}

DBusConnection *
ac_sample_dbus_get_active_connection(void) {

	unsigned int active_slot_id = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		active_slot_id = sample_get_product_info(PRODUCT_ACTIVE_MASTER);
	}
	if(0 == active_slot_id || active_slot_id > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "ac_sample_dbus_get_active_connection: get active master slot id failed!\n");
		return NULL;
	}

	if(NULL == ac_sample_tipc_connection[active_slot_id]) {
		ac_sample_tipc_connection[active_slot_id] = ac_sample_dbus_get_tipc_connection(active_slot_id);
	}

	return ac_sample_tipc_connection[active_slot_id];
}


void ac_sample_dbus_dispach( unsigned int block_usecond )
{
    static int pre_state = 1;
    
    if( pre_state )
    {
        //printf("ac_sample_dbus_dispach disapch\n");
        pre_state = dbus_connection_read_write_dispatch( ac_sample_dbus_connection, block_usecond );
        //printf("ac_sample_dbus_dispach after disapch  pre_state=%d\n", pre_state );
    }
    else
    {
        usleep(block_usecond);
    }
    
    return;
}



#if 0
DBusMessage *ac_sample_dbus_test
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
    DBusMessage *reply = NULL;
    unsigned int ret = 1;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
 
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply) {
        
        return reply;
    }
    
    dbus_message_iter_init_append( reply, &iter);
    dbus_message_iter_append_basic( &iter,
									DBUS_TYPE_UINT32, &ret);

    printf("ac_sample_dbus_test\n");    

    return reply;
}
#endif

DBusMessage * ac_sample_dbus_method_get_sample_param_timeout
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	
	int time_out;

    syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_param_timeout" );
	
    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
	
    	syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_param_timeout create reply filed!" );
		return reply;
	}

	time_out=get_global_socket_timout();

	if ( time_out<0 )
	{
		ret = AS_RTN_PARAM_ERR;
	}
	
	dbus_message_append_args( reply,
								DBUS_TYPE_INT32,&ret,
								DBUS_TYPE_UINT32, &time_out,
								DBUS_TYPE_INVALID );

	return reply;
}


DBusMessage * ac_sample_dbus_method_set_sample_param_timeout
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	
    unsigned int time_out=DEFAULT_TIMEOUT;

    syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_param_timeout" );
	
    if(dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_UINT32, &time_out,
                            DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
	
    	syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_param_timeout create reply filed!" );
		return reply;
	}

	ret=set_global_soket_timout ( time_out );

	if ( 0!=ret )
	{
		syslog(LOG_INFO,"set_global_soket_timout %u error!",time_out);
		ret = AS_RTN_PARAM_ERR;
	}
	
	dbus_message_append_args( reply,
								DBUS_TYPE_INT32,&ret,
								DBUS_TYPE_INVALID );

	return reply;
}


DBusMessage *ac_sample_dbus_method_get_sample_params_info_by_name
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
	DBusMessage *reply = NULL;
	int ret = AS_RTN_OK;
	DBusMessageIter iter = {0};
	DBusError		err = {0};	  
	unsigned int value=0;
	int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;

	int				sample_on = SAMPLE_ON;
	int		statistics_time = 0;
	int		sample_interval = 0;
	long	resend_interval = 0;
	int		reload_config_interval = 0;
	//int   over_threshold_check_type;	//not use
	//int	clear_check_type;		    //not use
	int	    threshold = 0;

	syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_param_by_name" );
	if(dbus_message_get_args(	msg,
							&err,
							DBUS_TYPE_STRING, &sample_name,
							DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	pas = get_ac_sample_by_name( sample_name );
	if ( NULL!=pas )
	{	
		syslog(LOG_DEBUG,"sample name %s",sample_name);

		sample_on = get_sample_state( pas );
		statistics_time = get_statistics_time( pas );
		sample_interval = get_sample_interval( pas );
		resend_interval = get_resend_interval( pas );
		reload_config_interval = get_sample_reload_config_interval( pas );
		threshold = get_sample_threshold( pas );
		
		if ( sample_on < 0 ||statistics_time < 0 ||sample_interval < 0 ||resend_interval < 0 ||reload_config_interval < 0 ||threshold < 0 )
		{
			ret=AS_RTN_PARAM_ERR;
			syslog(LOG_DEBUG,"ac_sample_dbus_method_get_sample_param_by_name get param error return %d",ret);
		}
	}else
	{
		syslog(LOG_INFO,"Unknown sample name %s", sample_name );
		ret = AS_RTN_PARAM_ERR;
	}

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_param_by_name  reply create failed!" );
		return reply;
	}

	dbus_message_append_args( reply,
								DBUS_TYPE_INT32, &ret,
				                            	DBUS_TYPE_INT32, &sample_on,
				                            	DBUS_TYPE_UINT32, &statistics_time,
				                            	DBUS_TYPE_UINT32, &sample_interval,
				                            	DBUS_TYPE_UINT32, &resend_interval,
				                            	DBUS_TYPE_UINT32, &reload_config_interval,
				                            	DBUS_TYPE_INT32, &threshold,
								DBUS_TYPE_INVALID );


	return reply;

}


DBusMessage *ac_sample_dbus_method_get_cpu_stat_info
(DBusConnection *conn,
		DBusMessage *msg,
		 void *user_data
)
{
	DBusMessage *reply;
	DBusMessageIter args;
	DBusMessageIter iter;
	DBusMessageIter iter_array;

	cpu_in_t rtime;
	char *param;
	FILE *fp;
	int i = 0;
	int j = 0;
	int k = 0;
	char buf[256];
	struct cpu_stat_t data;
	double rate;
	double alldata[18];
	memset(buf, 0, 256);
	memset(alldata, 0, sizeof(double) * 18);

	dbus_message_iter_init(msg, &args);
	//dbus_message_iter_get_basic(&args, &param);

	if(NULL == (fp = fopen("/proc/stat", "r"))){
		syslog(LOG_DEBUG, "open /proc/stat error");
		return NULL;
	}

	while(NULL != fgets(buf, 256, fp)){
		if(0 == strncmp(buf, "cpu", 3)){
			if(0 == parse(buf, &data)){
				rate = ((double)(data.used * 100)) / data.total;
				alldata[i] = rate;
				memset(&data, 0, sizeof(struct cpu_stat_t));
				memset(head, 0, 32);
				i++;
			}
			memset(buf, 0, 256);
		}else{
			break;
		}

	}

	rtime.cpu_cont = i - 1;
	rtime.cpu_average = alldata[0];
	for(j = 1; j < i; j++)
		rtime.cpu_info[j - 1] = alldata[j];

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rtime.cpu_cont);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_DOUBLE, &rtime.cpu_average);
	
	for(k = 0; k < rtime.cpu_cont; k++){
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_DOUBLE, &rtime.cpu_info[k]);
	}

    fclose(fp);
	return reply;
//send the data;
}

//added by stephen for cpu rt stat info
DBusMessage *ac_sample_dbus_method_get_cpu_rt_stat_info
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
		DBusMessage *reply;
		DBusMessageIter args;
		DBusMessageIter iter;
		DBusMessageIter iter_array;
		cpu_in_t *rtime = NULL;
		int k = 0;
		ac_sample_t *pas = NULL;

		pas = get_ac_sample_by_name(SAMPLE_NAME_CPURT);
		if(NULL != pas){
		rtime = (cpu_in_t *)get_sample_cpu_rt_data(pas);
		}

		if (NULL == rtime) {
			return NULL;
		}
		dbus_message_iter_init(msg, &args);

		
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply, &iter);
		
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rtime->cpu_cont);
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_DOUBLE, &rtime->cpu_average);
		
		for(k = 0; k < rtime->cpu_cont; k++){
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_DOUBLE, &rtime->cpu_info[k]);
		}
		
		return reply;
}

DBusMessage *ac_sample_dbus_method_get_sample_info
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;
	unsigned int latest = 0;
	unsigned int average = 0;
	unsigned int max = 0;
	
	syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_info" );
    if(dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_STRING, &sample_name,
                            DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_info create reply failed! name=%s", sample_name );
		return reply;
	}

	pas = get_ac_sample_by_name( sample_name );
	if( NULL != pas )
	{
		ret = get_sample_info( pas, &latest, &average, &max );
		syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_info latest:%d  average:%d   max:%d",
									latest, average, max );
	}
	else
	{
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
		syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_info latest:%d  average:%d   max:%d",
									latest, average, max );
		
	}

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_UINT32, &latest,
							DBUS_TYPE_UINT32, &average,
							DBUS_TYPE_UINT32, &max,
							DBUS_TYPE_INVALID );
	
	return reply;
}



DBusMessage *ac_sample_dbus_method_get_sample_param
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;

	unsigned int sample_interval;
	unsigned int statistics_time;
	unsigned int resend_interval;

    syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_param" );
	
    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
	
    	syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_param create reply filed!" );
		return reply;
	}

	sample_interval = get_all_sample_interval();
	statistics_time = get_all_statistics_time();
	resend_interval = get_all_resend_interval();
	
	dbus_message_append_args( reply,
								DBUS_TYPE_UINT32, &sample_interval,
								DBUS_TYPE_UINT32, &statistics_time,
								DBUS_TYPE_UINT32, &resend_interval,
								DBUS_TYPE_INVALID );

	return reply;
}




DBusMessage *ac_sample_dbus_method_set_sample_param
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	
    syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_param" );
    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INT32, &type,
                            DBUS_TYPE_UINT32, &value,
                            DBUS_TYPE_INVALID );
    
    switch(type)
    {
        case AC_SAMPLE_PARAM_TYPE_INTERVAL:
			syslog( LOG_DEBUG, "set sample interval %d", value );
            ret = set_all_sample_interval( value );
            break;
        case AC_SAMPLE_PARAM_TYPE_STATISTICS:
			syslog( LOG_DEBUG, "set sample statistics %d", value );
            ret= set_all_statistics_time( value );
            break;
        case AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL:
			syslog( LOG_DEBUG, "set sample resend interval %d", value );
	     ret = set_all_resend_interval( value );
	     break;
        default:
            ret = AS_RTN_DBUS_INVALIED_SAMPLE_PARAM_TYPE;
            break;
    }
 

    reply = dbus_message_new_method_return(msg);
    if (NULL == reply) 
    {
    	syslog( LOG_ERR, "ac_sample_dbus_method_set_sample_param  reply create failed!" );
        return reply;
    }
    
	dbus_message_append_args( reply,
					            DBUS_TYPE_INT32, &ret,
					            DBUS_TYPE_INVALID );


    return reply;
}

DBusMessage * ac_sample_dbus_method_get_sample_param_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage *reply = NULL;
	int ret = AS_RTN_OK;
	DBusMessageIter iter = {0};
	DBusError		err = {0};	  
	int value=0;
	int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;

	syslog( LOG_DEBUG, "ac_sample_dbus_method_get_sample_param_by_name" );
	if(dbus_message_get_args(	msg,
							&err,
							DBUS_TYPE_STRING, &sample_name,
							DBUS_TYPE_INT32, &type,
							DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	pas = get_ac_sample_by_name( sample_name );
	if ( NULL!=pas )
	{	
		syslog(LOG_DEBUG,"sample name %s",sample_name);
		switch(type)
		{
			case AC_SAMPLE_PARAM_TYPE_SWITCH:
				value = get_sample_state( pas );
				syslog(LOG_DEBUG,"get sample state %d", value );
				break;
			case AC_SAMPLE_PARAM_TYPE_THRESHOLD:
				value = get_sample_threshold( pas );
				syslog(LOG_DEBUG,"get sample threshold %d",value );
				break;
			case AC_SAMPLE_PARAM_TYPE_INTERVAL:
				syslog( LOG_DEBUG, "get sample interval %d", value );
				value = get_sample_interval( pas );
				break;
			case AC_SAMPLE_PARAM_TYPE_STATISTICS:
				value= get_statistics_time( pas );
				syslog( LOG_DEBUG, "get sample statistics %d", value );
				break;
			case AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL:
				value = get_resend_interval( pas );
				syslog( LOG_DEBUG, "get sample resend interval %d", value );
				break;
			case AC_SAMPLE_PARAM_TYPE_CONFIG_RELOAD_INTERVAL:
				value = get_sample_reload_config_interval( pas );
				syslog(LOG_DEBUG,"get sample reload config interval %d",value );
				break;
			default:
				ret = AS_RTN_DBUS_INVALIED_SAMPLE_PARAM_TYPE;
				break;
		}

		if ( value < 0 )
		{
			ret=AS_RTN_PARAM_ERR;
			syslog(LOG_DEBUG,"ac_sample_dbus_method_get_sample_param_by_name get param error return %d",ret);
		}
	}else
	{
		syslog(LOG_INFO,"Unknown sample name %s",sample_name);
		ret = AS_RTN_PARAM_ERR;
	}

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_ERR, "ac_sample_dbus_method_get_sample_param_by_name  reply create failed!" );
		return reply;
	}

	dbus_message_append_args( reply,
								DBUS_TYPE_INT32, &ret,
								DBUS_TYPE_UINT32, &value,
								DBUS_TYPE_INVALID );


	return reply;
}



DBusMessage * ac_sample_dbus_method_set_sample_param_by_name
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{

	DBusMessage *reply = NULL;
	int ret = AS_RTN_OK;
	DBusMessageIter iter = {0};
	DBusError		err = {0};	  
	unsigned int value=0;
	int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;

	syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_param_by_name" );
	if(dbus_message_get_args(	msg,
							&err,
							DBUS_TYPE_STRING, &sample_name,
							DBUS_TYPE_INT32, &type,
							DBUS_TYPE_UINT32, &value,
							DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	pas = get_ac_sample_by_name( sample_name );
	if ( NULL!=pas )
	{
		syslog(LOG_DEBUG,"sample name %s",sample_name);
		switch(type)
		{
			case AC_SAMPLE_PARAM_TYPE_SWITCH:
				syslog(LOG_DEBUG,"set sample state %d", value );
				ret = set_sample_state( pas, value );
				break;
			case AC_SAMPLE_PARAM_TYPE_THRESHOLD:
				syslog(LOG_DEBUG,"set sample threshold %d",value );
				ret = set_sample_threshold( pas, value);
				break;
			case AC_SAMPLE_PARAM_TYPE_INTERVAL:
				syslog( LOG_DEBUG, "set sample interval %d", value );
				ret = set_sample_interval( pas, value );
				break;
			case AC_SAMPLE_PARAM_TYPE_STATISTICS:
				syslog( LOG_DEBUG, "set sample statistics %d", value );
				ret= set_statistics_time( pas, value );
				break;
			case AC_SAMPLE_PARAM_TYPE_RESEND_INTERVAL:
				syslog( LOG_DEBUG, "set sample resend interval %d", value );
				ret = set_resend_interval( pas, value );
				break;
			case AC_SAMPLE_PARAM_TYPE_CONFIG_RELOAD_INTERVAL:
				syslog(LOG_DEBUG,"set sample reload config interval %d",value );
				ret = set_sample_reload_config_interval( pas, value );
				break;
			default:
				ret = AS_RTN_DBUS_INVALIED_SAMPLE_PARAM_TYPE;
				break;
		}
	}else
	{
		syslog(LOG_INFO,"Unknown sample name %s",sample_name);
		ret=AS_RTN_PARAM_ERR;
	}


	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_ERR, "ac_sample_dbus_method_set_sample_param_by_name  reply create failed!" );
		return reply;
	}

	dbus_message_append_args( reply,
								DBUS_TYPE_INT32, &ret,
								DBUS_TYPE_INVALID );


	return reply;
}



DBusMessage *ac_sample_dbus_method_set_signal_threshold
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;
	unsigned int threshold = 0;

	
	syslog( LOG_DEBUG, "ac_sample_dbus_method_set_signal_threshold" );
    if(dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_STRING, &sample_name,
                            DBUS_TYPE_UINT32, &threshold, 
                            DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_ERR, "ac_sample_dbus_method_set_signal_threshold create reply failed!" );
		return reply;
	}

	pas = get_ac_sample_by_name( sample_name );
	if( NULL != pas )
	{
		ret = set_sample_threshold( pas, threshold );
	}
	else
	{
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
	}

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_INVALID );
	
	return reply;
}





DBusMessage *ac_sample_dbus_method_get_signal_threshold
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;
	unsigned int threshold = 0;

	
	syslog( LOG_DEBUG, "ac_sample_dbus_method_set_signal_threshold" );
    if(dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_STRING, &sample_name,
                            DBUS_TYPE_INVALID) == FALSE) 
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_ERR, "ac_sample_dbus_method_set_signal_threshold create reply failed!" );
		return reply;
	}

	pas = get_ac_sample_by_name( sample_name );
	if( NULL != pas )
	{
		threshold = get_sample_threshold( pas );
	}
	else
	{
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
	}

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_UINT32, &threshold,
							DBUS_TYPE_INVALID );
	
	return reply;
}




DBusMessage *ac_sample_dbus_method_set_sample_state
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
	DBusMessage *reply = NULL;
	int ret = AS_RTN_OK;
	DBusMessageIter iter = {0};
	DBusError		err = {0};    
	unsigned int value=0;
	int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;
	int state = SAMPLE_ON;

	syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state" );
    if(dbus_message_get_args(msg,
                            &err,
                            DBUS_TYPE_STRING, &sample_name,
                            DBUS_TYPE_INT32, &state, 
                            DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state create reply failed" );
		return reply;
	}

	pas = get_ac_sample_by_name( sample_name );
	if( NULL != pas )
	{
		ret = set_sample_state( pas, state );
	}
	else
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state sample %s not fined", sample_name );	
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
	}

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_INVALID );
	
	return reply;
}





DBusMessage *ac_sample_dbus_method_get_sample_state
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int value=0;
    int type=0;
	char *sample_name=NULL;
	ac_sample_t *pas = NULL;
	int state = 0;

	syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state" );
    if(dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_STRING, &sample_name,
                            DBUS_TYPE_INVALID ) == FALSE)
    {
        return NULL;
    }

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state create reply failed" );
		return reply;
	}

	pas = get_ac_sample_by_name( sample_name );
	if( NULL != pas )
	{
		state = get_sample_state( pas );
	}
	else
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state sample %s not fined", sample_name );	
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
	}

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_INT32, &state,
							DBUS_TYPE_INVALID );
	
	return reply;
}





DBusMessage *ac_sample_dbus_method_set_debug_level
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
	unsigned int level = 0;

    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INT32, &level,
                            DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) 
	{
		syslog( LOG_DEBUG, "ac_sample_dbus_method_set_sample_state create reply failed" );
		return reply;
	}

	ret = setlogmask(level);/*return prev log level mask*/

	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &ret,
							DBUS_TYPE_INVALID );
	
	return reply;
}



/******************************
*
*set ac sample service_state
*
*******************************/
DBusMessage *ac_sample_dbus_method_set_service_state 
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    unsigned int service_state;
    
    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INT32, &service_state,
                            DBUS_TYPE_INVALID );
    
    
    syslog(LOG_DEBUG,"ac_sample_dbus_method_set_service_state  %d\n", service_state );
    ret= set_ac_sample_service_state( service_state );

    reply = dbus_message_new_method_return(msg);
    if (NULL == reply) 
    {
        return reply;
    }
    
	dbus_message_append_args( reply,
						       DBUS_TYPE_INT32, &ret,
							   DBUS_TYPE_INVALID );


    return reply;
}


DBusMessage *ac_sample_dbus_method_get_service_state 
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
    DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
    DBusError		err = {0};    
    int service_state = SAMPLE_SERVICE_STATE_OFF;
    
    dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INVALID );
    
    
    service_state = get_ac_sample_service_state( );

    reply = dbus_message_new_method_return(msg);
    if (NULL == reply) 
    {
        return reply;
    }
    
	dbus_message_append_args( reply,
						       DBUS_TYPE_INT32, &ret,
						       DBUS_TYPE_INT32, &service_state,
							   DBUS_TYPE_INVALID );


    return reply;
}

DBusMessage * ac_sample_dbus_method_get_iface_info
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
	DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
	DBusMessageIter	 iter_array = { 0 };	
    DBusError		err = {0};  

	struct ac_sample_t *ifFlowSample = NULL;
	struct if_flow_info_s *pifinfo = NULL;
	
	struct list_head   *ifaceList = NULL;
	struct list_head  	*ifITOR   = NULL;
	
	dbus_message_get_args(  msg,
                            &err,
                            DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL==reply)
	{
		return NULL;
	}

	ifFlowSample = get_ac_sample_by_name(SAMPLE_NAME_INTERFACE_FLOW);
	if (NULL==ifFlowSample)
	{
		return NULL;
	}
	
	ifaceList = (struct list_head *)get_sample_user_data( ifFlowSample );
	if (NULL == ifaceList )
	{
		return NULL;
	}
	
	int nIfcount = 0;
	list_for_each(ifITOR, ifaceList)
	{
		nIfcount++;
	}
	if (0==nIfcount)
	{
		ret = AS_RTN_SAMPLE_NAME_NONBEING;
	}
	syslog(LOG_DEBUG, "ac_sample iface_count=%d data_list=%p", nIfcount, ifaceList);

	//取数据，构造应答消息
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&nIfcount);

	if( 0 != nIfcount )
	{
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT64_AS_STRING
											DBUS_TYPE_UINT64_AS_STRING
											DBUS_TYPE_UINT64_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		list_for_each_entry( pifinfo, ifaceList, node)
		{
			syslog(LOG_DEBUG, "before ac_sample index=%d, rxbandwidth=%llu, txbdwidth=%llu", 
						pifinfo->ifindex, pifinfo->rxbandwidth, pifinfo->txbandwidth);
			
			DBusMessageIter iter_struct;
				
			dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT64, &(pifinfo->ifindex));
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT64, &(pifinfo->rxbandwidth));
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT64, &(pifinfo->txbandwidth));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
			syslog(LOG_DEBUG, "ac_sample index=%d, rxbandwidth=%llu, txbdwidth=%llu", 
						pifinfo->ifindex, pifinfo->rxbandwidth, pifinfo->txbandwidth);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	return reply;
}


DBusMessage * ac_sample_dbus_method_get_radius_req_info
(
    DBusConnection *conn, 
    DBusMessage *msg, 
    void *user_data
)
{
	DBusMessage *reply = NULL;
    int ret = AS_RTN_OK;
    DBusMessageIter iter = {0};
	DBusMessageIter	 iter_array = { 0 };	
    DBusError		err = {0};  

	struct ac_sample_t *radiusReqSample = NULL;
	struct instance_radius_req *pRadiusReqInfo = NULL;
	
	struct list_head  *radiusReqList = NULL;
	struct list_head  *radiusReqOR   = NULL;

	reply = dbus_message_new_method_return(msg);
	if(NULL == reply) {
	    syslog(LOG_WARNING, "ac_sample_dbus_method_get_radius_req_info: dbus_message_new_method_return failed!\n");
		return NULL;
	}

	radiusReqSample = get_ac_sample_by_name(SAMPLE_NAME_RADIUS_REQ_SUC);
	if (NULL == radiusReqSample) {
		return NULL;
	}
	
	radiusReqList = (struct list_head *)get_sample_user_data(radiusReqSample);
	if (NULL == radiusReqList ) {
		return NULL;
	}
	
	unsigned int instance_count = 0;
	list_for_each(radiusReqOR, radiusReqList) {
		instance_count++;
	}
	
	syslog(LOG_DEBUG, "ac_sample instance_count = %d  data_list = %p", instance_count, radiusReqList);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&instance_count);

	if(AS_RTN_OK == ret && instance_count) {
	
		dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING
										DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);

		list_for_each_entry(pRadiusReqInfo, radiusReqList, node) {
			syslog(LOG_DEBUG, "before ac_sample local_id = %d, instance_id = %d, access_accept_rate = %d\n", 
						            pRadiusReqInfo->local_id, pRadiusReqInfo->instance_id, pRadiusReqInfo->access_accept_rate);
			
			DBusMessageIter iter_struct;
				
			dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
			
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32, &(pRadiusReqInfo->local_id));
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32, &(pRadiusReqInfo->instance_id));
			dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32, &(pRadiusReqInfo->access_accept_rate));

			dbus_message_iter_close_container (&iter_array, &iter_struct);
			
			syslog(LOG_DEBUG, "ac_sample local_id = %d, instance_id = %d, access_accept_rate = %d\n", 
						            pRadiusReqInfo->local_id, pRadiusReqInfo->instance_id, pRadiusReqInfo->access_accept_rate);
		}
		
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	return reply;
}






struct {
    char *interface;
    char *method_call;
    DBusMessage *(*method_call_cb)
    (
        DBusConnection *conn, 
        DBusMessage *msg, 
        void *user_data
    );
} sample_method_list[] = {
/*    {AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_INTERVAL, ac_sample_dbus_method_set_sample_interval},*/
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM_TIMEOUT,ac_sample_dbus_method_get_sample_param_timeout },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM_TIMEOUT,ac_sample_dbus_method_set_sample_param_timeout },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_INFO, ac_sample_dbus_method_get_sample_info },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAMS_INFO_BY_NAME, ac_sample_dbus_method_get_sample_params_info_by_name},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM, ac_sample_dbus_method_get_sample_param },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_PARAM, ac_sample_dbus_method_set_sample_param },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_PARAM_BY_NAME, ac_sample_dbus_method_set_sample_param_by_name},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM_BY_NAME, ac_sample_dbus_method_get_sample_param_by_name},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SIGNAL_THRESHOLD, ac_sample_dbus_method_set_signal_threshold },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SIGNAL_THRESHOLD, ac_sample_dbus_method_get_signal_threshold },

	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_STATE, ac_sample_dbus_method_set_sample_state },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_STATE, ac_sample_dbus_method_get_sample_state },
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_PARAM_TIMEOUT,ac_sample_dbus_method_set_sample_param_timeout},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM_TIMEOUT,ac_sample_dbus_method_get_sample_param_timeout},

	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_DEBUG_LEVEL,ac_sample_dbus_method_set_debug_level},

	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_SET_SERVICE_STATE,ac_sample_dbus_method_set_service_state},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_SERVICE_STATE,ac_sample_dbus_method_get_service_state},

	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_IFACE_INFO,ac_sample_dbus_method_get_iface_info},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_CPU_STAT_INFO,ac_sample_dbus_method_get_cpu_stat_info},
    {AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_RADIUS_REQ_INFO,ac_sample_dbus_method_get_radius_req_info},
	{AC_SAMPLE_DBUS_INTERFACE,AC_SAMPLE_DBUS_METHOD_GET_CPU_RT_STAT_INFO,ac_sample_dbus_method_get_cpu_rt_stat_info}
};
//modified for cpu stat info by stephen
    
#define METHOD_CALL_NUM     sizeof(sample_method_list)/sizeof(sample_method_list[0])




static DBusHandlerResult ac_sample_dbus_message_handler
(
    DBusConnection *connection, 
    DBusMessage *message, 
    void *user_data
)
{
    int i;
    DBusMessage 	*reply = NULL;
    
    if (strcmp(dbus_message_get_path(message),AC_SAMPLE_DBUS_OBJPATH) == 0) 
    {
        for( i=0; i<METHOD_CALL_NUM; i++ )
        {
            if( dbus_message_is_method_call(message,
                    									sample_method_list[i].interface,
                    									sample_method_list[i].method_call) )
            {
                reply = sample_method_list[i].method_call_cb(connection, message, user_data);
                break;
            }
        }
    }
    
    if (reply)
    {
        dbus_connection_send (connection, reply, NULL);
        dbus_connection_flush(connection); /* TODO	  Maybe we should let main loop process the flush*/
        dbus_message_unref (reply);
    }
	
	/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
    return DBUS_HANDLER_RESULT_HANDLED ;
}





int ac_sample_dbus_init(void)
{
    DBusError 	dbus_error;
    int			ret = 0;
    DBusObjectPathVTable	vtable = {NULL, ac_sample_dbus_message_handler, NULL, NULL, NULL, NULL};

    dbus_connection_set_change_sigpipe (TRUE);

    dbus_error_init (&dbus_error);
    ac_sample_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
    if (ac_sample_dbus_connection == NULL) 
    {
        return AS_RTN_DBUS_ERR;
    }

    if (!dbus_connection_register_fallback (ac_sample_dbus_connection, AC_SAMPLE_DBUS_OBJPATH, &vtable, NULL)) 
    {
        return AS_RTN_DBUS_ERR;		
    }
		
    ret = dbus_bus_request_name ( ac_sample_dbus_connection, AC_SAMPLE_DBUS_BUSNAME, 0, &dbus_error );
    if( -1 == ret )
    {
		    if ( dbus_error_is_set (&dbus_error) ) 
        {
		        //log_err("portal ha dbus request bus name %s with error %s", AC_SAMPLE_DBUS_BUSNAME, dbus_error.message);
            return AS_RTN_DBUS_ERR;
        }
    }
    else 
    {
		    //log_dbg("portal ha dbus request bus name %s ok\n", AC_SAMPLE_DBUS_BUSNAME);
    }
	
    return AS_RTN_OK;
}

void
ac_sample_tipc_dbus_init(void) {
    syslog(LOG_DEBUG, "enter ac_sample_tipc_dbus_init\n");
    
    memset(ac_sample_tipc_connection, 0, sizeof(ac_sample_tipc_connection));

    int i = 1;
    for(i = 1; i < SLOT_MAX_NUM; i++) {
        ac_sample_tipc_connection[i] = ac_sample_dbus_get_tipc_connection(i);
    }
    
    return ;
}


#if 0
int dbus_send_signal( DBusConnection *conn, 
								const char *obj_path,
								const char *interface_name,
								const char *signal_name, 
								int first_arg_type,...)
{

#endif
int ac_sample_dbus_send_signal( const char *signal_name, 
								int first_arg_type,...)
								
{
	DBusConnection *conn = ac_sample_dbus_connection;
	const char *obj_path = AC_SAMPLE_DBUS_OBJPATH;
	const char *interface_name = TRAP_DBUS_INTERFACE;
	
	va_list var_args;
	int iret = AS_RTN_OK;
 
	
	va_start ( var_args, first_arg_type );
	iret = dbus_send_signale_v( conn, 
								obj_path,
								interface_name,
								signal_name,
								first_arg_type,
								var_args );
 	va_end (var_args);

	syslog(LOG_INFO, "send signal: %s, ret = %d", signal_name, iret);
    
	return iret;
}

int
ac_sample_dbus_send_active_signal(const char *signal_name, 
								                int first_arg_type,...) {

    DBusConnection *conn = ac_sample_dbus_get_active_connection();
    if(NULL == conn) {
        syslog(LOG_WARNING, "ac_sample_dbus_send_active_signal: get active master connection failed!\n");
        return AS_RTN_SAMPLE_DBUS_CONNECTION_ERR;
    }
    
    const char *obj_path = AC_SAMPLE_DBUS_OBJPATH;
    const char *interface_name = TRAP_DBUS_INTERFACE;
    
	va_list var_args;
	int iret = AS_RTN_OK;
 
	
	va_start ( var_args, first_arg_type );
	iret = dbus_send_signale_v( conn, 
								obj_path,
								interface_name,
								signal_name,
								first_arg_type,
								var_args );
 	va_end (var_args);

	syslog(LOG_INFO, "send active signal: %s, ret = %d", signal_name, iret);
    
	return iret;
}


int
ac_sample_dbus_send_slot_signal(const char *signal_name, int slotid,
								                int first_arg_type,...) {

    DBusConnection *conn = ac_sample_dbus_get_slotconnection(slotid);
    if(NULL == conn) {
        syslog(LOG_WARNING, "ac_sample_dbus_send_active_signal: get active master connection failed!\n");
        return AS_RTN_SAMPLE_DBUS_CONNECTION_ERR;
    }
    
    const char *obj_path = AC_SAMPLE_DBUS_OBJPATH;
    const char *interface_name = TRAP_DBUS_INTERFACE;
    
	va_list var_args;
	int iret = AS_RTN_OK;
 
	
	va_start ( var_args, first_arg_type );
	iret = dbus_send_signale_v( conn, 
								obj_path,
								interface_name,
								signal_name,
								first_arg_type,
								var_args );
 	va_end (var_args);

	syslog(LOG_INFO, "send active signal: %s, ret = %d", signal_name, iret);
    
	return iret;
}


