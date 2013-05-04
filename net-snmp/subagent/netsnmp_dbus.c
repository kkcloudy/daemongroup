#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "dbus/npd/npd_dbus_def.h"

#include "ws_public.h"

#include "ws_dbus_def.h"
#include "netsnmp_dbus_handler.h"
#include "netsnmp_dbus.h"


static DBusConnection *snmp_dbus_connection = NULL;

static int
netsnmp_dbus_filter_function(DBusConnection * connection,DBusMessage * message, void *user_data){

	if(dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
        0 == strcmp(dbus_message_get_path (message), DBUS_PATH_LOCAL)) {
		dbus_connection_unref(snmp_dbus_connection);
		snmp_dbus_connection = NULL;
	}
	else if((dbus_message_is_signal(message, "aw.snmp.mib", NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_MIB_BY_VRRP))) {
		netsnmp_dbus_had_master_advertise(message);
	}
	else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}


static DBusHandlerResult 
netsnmp_dbus_message_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {

    DBusMessage	*reply = NULL;
	char sender[20] = { 0 };
    
    syslog(LOG_DEBUG, "message type = %s, message path = %s, message member = %s\n",
                        dbus_message_type_to_string(dbus_message_get_type(message)),
                        dbus_message_get_path(message), dbus_message_get_member(message));
	
	if(strcmp(dbus_message_get_path(message), SNMPD_DBUS_OBJPATH) == 0)	 {
        if(dbus_message_is_method_call(message, SNMPD_DBUS_INTERFACE, SNMPD_DBUS_CONF_METHOD_SHOW_DBUS_CONNECTION_LIST)) {
			reply = netsnmp_dbus_interface_show_dbus_connection_list(connection, message, user_data);
		}		
        else if(dbus_message_is_method_call(message, SNMPD_DBUS_INTERFACE, SNMPD_DBUS_CONF_METHOD_CONFIG_SNMP_LOG_DEBUG)) {
			reply = netsnmp_dbus_interface_config_snmp_log_debug(connection, message, user_data);
		}
	}

	if(reply) {
    	syslog(LOG_DEBUG, "reply destination %s\n", dbus_message_get_destination(reply));
    	memset(sender,0, 20);
    	strcpy(sender, dbus_message_get_destination(reply));
    	dbus_connection_send (connection, reply, NULL);
    	dbus_connection_flush(connection); 
    	dbus_message_unref (reply);
		return DBUS_HANDLER_RESULT_HANDLED;		
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

	
	
static int 
netsnmp_dbus_init(void) {	

	DBusError dbus_error;

    DBusObjectPathVTable snmpd_vtable = {NULL, &netsnmp_dbus_message_handler, NULL, NULL, NULL, NULL};	
    
	dbus_error_init(&dbus_error);
	
	snmp_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
	if(NULL == snmp_dbus_connection) {
        if(dbus_error_is_set (&dbus_error)) {
		    syslog(LOG_WARNING, "netsnmp_dbus_init: dbus_bus_get(): %s\n", dbus_error.message);
            dbus_error_free(&dbus_error);
	    }	    
		return SNMPD_DBUS_ERROR;
	}
	
	if(!dbus_connection_register_fallback(snmp_dbus_connection, SNMPD_DBUS_OBJPATH, &snmpd_vtable, NULL)) {        
        syslog(LOG_WARNING, "netsnmp_dbus_init: dbus register fallback failed!\n");
        if(dbus_error_is_set (&dbus_error)) {
            dbus_error_free(&dbus_error);
        }
    	return SNMPD_DBUS_ERROR;
	}
	
	
	dbus_bus_request_name(snmp_dbus_connection, SNMPD_DBUS_DBUSNAME, 0, &dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		syslog(LOG_WARNING, "netsnmp_dbus_init: dbus_bus_request_name(): %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		return SNMPD_DBUS_ERROR;
	}

	dbus_connection_add_filter(snmp_dbus_connection, netsnmp_dbus_filter_function, NULL, NULL);
				
	dbus_bus_add_match(snmp_dbus_connection,
				        "type='signal'"
						",interface='"SNMPD_DBUS_INTERFACE"'",
				        NULL);

	return SNMPD_DBUS_SUCCESS;  
}

void *
netsnmp_dbus_thread(){
    
    snmp_pid_write(SNMP_THREAD_INFO_PATH, "snmp dbus thread");
    
    if(SNMPD_DBUS_SUCCESS != netsnmp_dbus_init()) {
        syslog(LOG_WARNING, "Init netsnmp dbus failed!\n");
        return NULL;
    }

    while(dbus_connection_read_write_dispatch(snmp_dbus_connection, 500)) {
        
    }

    return NULL;
}

