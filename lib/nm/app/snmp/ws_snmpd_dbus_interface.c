#include "ws_dbus_def.h"
#include "ws_snmpd_dbus_interface.h"


void
free_snmpd_dbus_connection_list(struct snmpdInstanceInfo **snmpdHead) {
    if(NULL == snmpdHead)
        return;

    struct snmpdInstanceInfo *tempHead = *snmpdHead;
    while(tempHead) {
        struct snmpdInstanceInfo *tempNode = tempHead->next;
        free(tempHead);
        tempHead = tempNode;
    }
    
    *snmpdHead = NULL;
    return ;
}

int 
show_snmpd_dbus_connection_list(DBusConnection *connection, struct snmpdInstanceInfo **snmpdHead, unsigned int *slot_count) {

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	

	int ret = SNMPD_DBUS_SUCCESS;
    struct snmpdInstanceInfo *tempHead = NULL;
    
	query = dbus_message_new_method_call(SNMPD_DBUS_DBUSNAME, 
	                                     SNMPD_DBUS_OBJPATH, 
	                                     SNMPD_DBUS_INTERFACE,
	                                     SNMPD_DBUS_CONF_METHOD_SHOW_DBUS_CONNECTION_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);	
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, slot_count);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);

	if(SNMPD_DBUS_SUCCESS == ret && *slot_count > 0 ) {
        int index = 0;
        for(index = 0; index < *slot_count; index++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_state_array; 
			DBusMessageIter iter_state_struct;

            struct snmpdInstanceInfo *tempNode = (struct snmpdInstanceInfo *)malloc(sizeof(struct snmpdInstanceInfo));
            if(NULL == tempNode) {
                return SNMPD_DBUS_ERROR;
            }

            memset(tempNode, 0, sizeof(struct snmpdInstanceInfo));
            
            tempNode->next = tempHead;
            tempHead = tempNode;
            
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(tempNode->slot_id));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tempNode->dbus_connection_state));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tempNode->master_count));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_recurse(&iter_struct,&iter_state_array);

            int i, j;
            for(i = 0; i < VRRP_TYPE_NUM; i++) {
                for(j = 0; j <= INSTANCE_NUM; j++) {

					dbus_message_iter_recurse(&iter_state_array, &iter_state_struct);
					dbus_message_iter_get_basic(&iter_state_struct,&(tempNode->instance_state[i][j]));

                    dbus_message_iter_next(&iter_state_struct);
                    dbus_message_iter_next(&iter_state_array);
                    
                }
            }
            dbus_message_iter_next(&iter_array);
        }
	}
	
    dbus_message_unref(reply);
    *snmpdHead = tempHead;
    return ret;
    
}

int
dbus_get_trap_instance_states(DBusConnection *connection, unsigned int *master_instance) {
    
    if(NULL == connection || NULL == master_instance)
        return SNMPD_DBUS_ERROR;
    
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter iter;

    *master_instance = 0;

    int ret = SNMPD_DBUS_SUCCESS;

    query = dbus_message_new_method_call(TRAP_DBUS_BUSNAME, 
                                         TRAP_DBUS_OBJPATH,
                                         TRAP_DBUS_INTERFACE,
                                         TRAP_DBUS_METHOD_GET_INSTANCE_STATES);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return SNMPD_CONNECTION_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, master_instance);
    
    dbus_message_unref(reply);

    return ret;     
}

int
dbus_config_snmp_log_debug(DBusConnection *connection, unsigned int debugLevel) {
    if(NULL == connection)
        return SNMPD_DBUS_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = SNMPD_DBUS_SUCCESS;

	query = dbus_message_new_method_call(SNMPD_DBUS_DBUSNAME, 
	                                     SNMPD_DBUS_OBJPATH, 
	                                     SNMPD_DBUS_INTERFACE,
	                                     SNMPD_DBUS_CONF_METHOD_CONFIG_SNMP_LOG_DEBUG);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &debugLevel,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return SNMPD_CONNECTION_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}


