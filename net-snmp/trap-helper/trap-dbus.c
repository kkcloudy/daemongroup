#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include <linux/kernel.h>
#include <mcheck.h> 
#include "board/board_define.h"
#include "trap-util.h"
#include "trap-def.h"
#include "nm_list.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-signal.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "trap-resend.h"
#include "trap-hash.h"
#include "trap-signal-handle.h"
#include "trap-instance.h"

#include "dbus/npd/npd_dbus_def.h"

#include "trap-dbus.h"


static DBusConnection *connection = NULL;
static DBusConnection * vrrp_dbus_connection = NULL;

static DBusConnection *trap_tipc_connection[SLOT_MAX_NUM + 1] = { 0 };

static unsigned int local_slotID = 0;
static unsigned int proxy_flag = 0;

static long StandbySwitchTime = 0;


void trap_dbus_init( void )
{
	DBusError error;
	
	dbus_connection_set_change_sigpipe(TRUE);
	dbus_error_init(&error);
	
	if ((connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error)) == NULL) {
		trap_syslog(LOG_ERR, "Failed to connect system bus: %s\n", error.message);
		dbus_error_free (&error);
	}	
	
	dbus_bus_request_name(connection, TRAP_DBUS_BUSNAME, 0, &error);
	if (dbus_error_is_set(&error)) {
		trap_syslog(LOG_ERR, "Failed to request name: %s\n", error.message);
		dbus_error_free (&error);
	}
	
	vrrp_dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &error);
	dbus_bus_request_name (vrrp_dbus_connection, "aw.traphelper",0, &error);
	
	
}


void TRAPReInitHadDbusPath(int index, char * path, char * newpath, int islocaled)
{
	sprintf(newpath,"%s%d",path,index);	
}

unsigned int trap_notice_vrrp_config_service_change_state(unsigned int InstID, unsigned int enable)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
#define PATH_LEN 64
	char OBJPATH[PATH_LEN] = {0};
	char BUSNAME[PATH_LEN] = {0};
	int ret = 0;	
	char cmd[128] = {0};
	DBusConnection * connection = vrrp_dbus_connection;
	TRAPReInitHadDbusPath(InstID,VRRP_DBUS_BUSNAME,BUSNAME,0);//book modify
	TRAPReInitHadDbusPath(InstID,VRRP_DBUS_OBJPATH,OBJPATH,0);//book modify
	query = dbus_message_new_method_call(BUSNAME,
										OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &InstID,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		trap_syslog(LOG_INFO, "%s %d\n",__func__,__LINE__);
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}

		return NULL;
	}
	else if (!(dbus_message_get_args (reply, &err, DBUS_TYPE_UINT32,&ret, DBUS_TYPE_INVALID))){		
		trap_syslog(LOG_INFO, "%s %d\n",__func__,__LINE__);
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	
	trap_syslog(LOG_INFO, "%s %d\n",__func__,__LINE__);
	dbus_message_unref(reply);

	return 0;
}

DBusMessage *trap_dbus_method_get_instance_states 
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};	  

	dbus_message_get_args(	msg,
							&err,
							DBUS_TYPE_INVALID );

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		syslog( LOG_INFO, "trap_dbus_method_get_instance_states: create reply failed" );
		return reply;
	}

	unsigned int instance_master = 0;
	int ret = trap_is_ac_trap_enabled(&gInsVrrpState);
	switch(ret)
	{
		case AC_IS_MASTER:
		    {
		        int i = 0, j = 0;
		        for(i = 0; i < VRRP_TYPE_NUM; i++) {
		            for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++)
                        instance_master = instance_master | (0x1 << ((i * INSTANCE_NUM) + gInsVrrpState.instance_master[i][j] - 1));
                }                   
                TRAP_TRACE_LOG(LOG_DEBUG,"trap_dbus_method_get_instance_states: instance_master is %d",instance_master);
            }
			break;
		case AC_IS_BACKUP:
			break;
		default:
			TRAP_TRACE_LOG(LOG_INFO,"trap_is_ac_trap_enabled: error return %d", ret);
			break;
	}
	
	dbus_message_append_args( reply,
							DBUS_TYPE_INT32, &instance_master,
							DBUS_TYPE_INVALID );

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
} trap_method_list[] = {
	{TRAP_DBUS_INTERFACE,TRAP_DBUS_METHOD_GET_INSTANCE_STATES, trap_dbus_method_get_instance_states }
};


#define METHOD_CALL_NUM	sizeof(trap_method_list)/sizeof(trap_method_list[0])

static DBusHandlerResult trap_dbus_message_handler
(
    DBusConnection *connection, 
    DBusMessage *message, 
    void *user_data
)
{
	DBusMessage 	*reply = NULL;
	int i;
	
	if (strcmp(dbus_message_get_path(message), TRAP_DBUS_OBJPATH) == 0) 
	{
		for(i = 0; i < METHOD_CALL_NUM; i++)
		{
			if( dbus_message_is_method_call(message,
											trap_method_list[i].interface,
											trap_method_list[i].method_call) )
			{
				reply = trap_method_list[i].method_call_cb(connection, message, user_data);
				break;
			}
		}

	}else
	{
		TRAP_TRACE_LOG(LOG_INFO,"trap dbus message handler: Unknown method call path %s!\n",dbus_message_get_path (message));
	}
	
	if (reply)
	{
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); // TODO	  Maybe we should let main loop process the flush
		dbus_message_unref (reply);
	}
	
	//	dbus_message_unref(message); //TODO who should unref the incoming message? 
	return TRAP_RTN_OK ;
}


static int trap_dbus_message_add_rules ( DBusConnection *connection )
{
	DBusError 	dbus_error;
	int			ret = 0;
	DBusObjectPathVTable	vtable = {NULL, trap_dbus_message_handler, NULL, NULL, NULL, NULL};

	//dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);

	if (connection == NULL) 
	{
		return TRAP_RTN_NULL_POINT;
	}

	if (!dbus_connection_register_fallback (connection, TRAP_DBUS_OBJPATH, &vtable, NULL)) 
	{
		return TRAP_RTN_DBUS_ERR;		
	}
	
	dbus_bus_add_match (connection, "type='message',interface='aw.trap'", NULL);
	return TRAP_RTN_OK;

}


static DBusConnection *
trap_bus_get_tipc_connection(unsigned int slot_id) {  

	if(0 == slot_id || slot_id > SLOT_MAX_NUM) {
		trap_syslog(LOG_WARNING, "trap_bus_get_tipc_connection fun error: the slot_id is %d\n",slot_id);
		return NULL;
	}
	
	DBusError dbus_error;
	DBusConnection *tempConnection = NULL;
	char temp_dbus_name[60] = { 0 };
	
	dbus_error_init (&dbus_error);
	
	tempConnection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &dbus_error);
	if(NULL == tempConnection) {
		dbus_error_free(&dbus_error);
		addresses_shutdown_func();
		trap_syslog(LOG_WARNING, "trap_bus_get_tipc_connection: The slot %d remote server is not ready.\n", slot_id);
		return NULL;
	}
	
	snprintf(temp_dbus_name, sizeof(temp_dbus_name) - 1, "aw.trap.slot%d", slot_id);
	dbus_bus_request_name(tempConnection, temp_dbus_name, 0, &dbus_error);
	if (dbus_error_is_set(&dbus_error))  {
		trap_syslog(LOG_WARNING, "request name failed: %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		return NULL;
	}
	
	trap_syslog(LOG_INFO, "The tipc connection %s of slot %d is OK\n", temp_dbus_name, slot_id);
	return tempConnection;
}

void
trap_init_tipc_connection(void) {
    
	trap_syslog(LOG_INFO, "Enter trap_init_tipc_connection\n");
	
    memset(trap_tipc_connection, 0, sizeof(trap_tipc_connection));

    int i = 1;
    for(i = 1; i <= SLOT_MAX_NUM; i++) {
        trap_tipc_connection[i] = trap_bus_get_tipc_connection(i);
        if(trap_tipc_connection[i]) {
            trap_syslog(LOG_INFO, "trap_init_tipc_connection: slot %d is connect\n", i);
        }
    }

    unsigned int active_master_slotID = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotID = trap_get_product_info(PRODUCT_LOCAL_SLOTID);
		active_master_slotID = trap_get_product_info(PRODUCT_ACTIVE_MASTER);
	}

    if(local_slotID && local_slotID == active_master_slotID) {
        proxy_flag = 1;
    }
    
	trap_syslog(LOG_INFO, "Exit trap_init_tipc_connection: local_slotID = %d, active_master_slotID = %d, proxy_flag = %d\n",
	                       local_slotID, active_master_slotID, proxy_flag);

    return ;
}

static void 
proxy_dbus_signal(DBusMessage *message, unsigned int slot_id) {
    if(NULL == message) {
        trap_syslog(LOG_WARNING, "proxy_dbus_signal: input message is NULL!\n");
        return ;
    }
    
    if(0 == slot_id) {
        int i = 1;
        for(i = 1; i <= SLOT_MAX_NUM; i++) {
        
            if(local_slotID == i) {
                continue;
            }
            else if(trap_tipc_connection[i]) {
                dbus_connection_send(trap_tipc_connection[i], message, NULL);
            }
        }
    }
    else if(local_slotID == slot_id){
        trap_syslog(LOG_DEBUG, "proxy_dbus_signal: the dest slotid is local, no need!\n");
    }
    else if(slot_id <= SLOT_MAX_NUM) {
        if(trap_tipc_connection[slot_id] || (trap_tipc_connection[slot_id] = trap_bus_get_tipc_connection(slot_id))) {
            dbus_connection_send(trap_tipc_connection[slot_id], message, NULL);
        }
        else {
            trap_syslog(LOG_WARNING, "proxy_dbus_signal: the slot (%d) is not connect\n", slot_id);
        }
    }
    else {
        trap_syslog(LOG_WARNING, "proxy_dbus_signal: the slot (%d) input is error\n", slot_id);
    }
    
    return ;    
}


static DBusHandlerResult trap_signal_filter_func (DBusConnection *connection, 
												DBusMessage *message, 
												void *user_data)
{
	TrapSignal *tSignal = NULL;
	const char *member = NULL;
	int type=0;

	struct sysinfo info;
	sysinfo(&info);
	trap_syslog(LOG_DEBUG,"uptime=%d StandbySwitchTime=%d\n",info.uptime,StandbySwitchTime);
	if((info.uptime - StandbySwitchTime < 180)
		&&(ac_trap_get_flag("/var/run/standby_switch_trap_flag")))
	{
		return DBUS_HANDLER_RESULT_HANDLED;
	}


	type=dbus_message_get_type (message);
	member=dbus_message_get_member(message);
	
	trap_syslog(LOG_DEBUG, "********** message, path=%s, type=%d, interface=%s, member=%s\n",
				dbus_message_get_path(message),
				dbus_message_get_type (message),
				dbus_message_get_interface(message),
				dbus_message_get_member(message));
	
	if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected"))
		trap_syslog(LOG_WARNING, "Received unexpected signal, path=%s, interface=%s, member=%s\n",
				dbus_message_get_path(message),
				dbus_message_get_interface(message),
				dbus_message_get_member(message));

	if( DBUS_MESSAGE_TYPE_SIGNAL == type){
		if((strcmp(member, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP) == 0) && 
			(ac_trap_get_flag("/var/run/standby_switch_trap_flag"))) {
			trap_syslog(LOG_INFO,"uptime=%d member=%s\n",info.uptime,member);
			trap_syslog(LOG_INFO,"%s\n",NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP);
			StandbySwitchTime = info.uptime;
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		
		if (trap_is_ac_trap_enabled(&gInsVrrpState) 
			|| strcmp(member, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP) == 0) {

			tSignal = trap_signal_hashtable_get_item(global.gSignalList_hash, member );

			if (NULL != tSignal){
				
				if (tSignal->signal_handle_func != NULL){
					if (TRAP_SIGNAL_HANDLE_SEND_TRAP_OK == tSignal->signal_handle_func(message))
                        ;//trap_heartbeat_update_last_time(&gHeartbeatInfo, 0);
				}else{
					trap_syslog(LOG_DEBUG,"no signal_handle_func error!\n");
				}
			}else {
			
				syslog(LOG_DEBUG, "Received unknown signal, path=%s, interface=%s, member=%s\n",
				dbus_message_get_path(message),
				dbus_message_get_interface(message),
				dbus_message_get_member(message));
			}
		}
		else if(proxy_flag && trap_proxy_hashtable_get_item(global.gProxyList_hash, member)) {
            syslog(LOG_INFO, "proxy trap to ethor slot: member = %s\n", member);
            proxy_dbus_signal(message, 0);                            
        }
		
#if 0
		for (tNode = global.gSignalList.first; tNode; tNode = tNode->next){
			tSignal = tNode->data;
			if (dbus_message_is_signal(message, "aw.trap", tSignal->signal_name)){
				trap_syslog(LOG_DEBUG, "Received signal, path=%s, interface=%s, member=%s\n",
					dbus_message_get_path(message),
					dbus_message_get_interface(message),
					dbus_message_get_member(message));
				if (trap_is_ac_trap_enabled(&gInsVrrpState) 
					|| strcmp(tSignal->signal_name, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP) == 0) {
					if (tSignal->signal_handle_func != NULL)
						if (TRAP_SIGNAL_HANDLE_SEND_TRAP_OK == tSignal->signal_handle_func(message))
							trap_heartbeat_update_last_time(&gHeartbeatInfo, 0);
				}
				break;
			}
		}
#endif

		return DBUS_HANDLER_RESULT_HANDLED;
	}
}

static int trap_dbus_signal_add_rules( DBusConnection *connection )
{
	if (connection == NULL) 
	{
		return TRAP_RTN_NULL_POINT;
	}
	
	dbus_connection_add_filter(connection, trap_signal_filter_func, NULL, NULL);
	dbus_bus_add_match (connection, "type='signal',interface='aw.trap'", NULL);
}

int trap_dbus_add_rules ( )
{

	trap_dbus_message_add_rules( connection );
	trap_dbus_signal_add_rules( connection );

}

void trap_dbus_dispatch(int timeout_milliseconds)
{
	dbus_connection_read_write_dispatch(connection,  timeout_milliseconds);
	return;
}


