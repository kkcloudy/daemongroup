
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "ws_tcrule.h"
#include "ac_manage_def.h"
#include "ac_manage_tcrule_interface.h"


static int
tcrule_data_copy_v2(TCRule *dest, TCRule *sour){
    if(NULL == dest || NULL == sour) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
         
    dest->name = (sour->name && sour->name[0]) ? strdup(sour->name) : NULL;
    if((sour->name && sour->name[0]) && NULL == dest->name) {
        return AC_MANAGE_MALLOC_ERROR;
    }

	dest->enable = sour->enable;
	dest->ruleIndex = sour->ruleIndex;
    
    dest->comment = (sour->comment && sour->comment[0]) ? strdup(sour->comment) : NULL;
    if((sour->comment && sour->comment[0]) && NULL == dest->comment) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->interface = (sour->interface && sour->interface[0]) ? strdup(sour->interface) : NULL;
    if((sour->interface && sour->interface[0]) && NULL == dest->interface) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->up_interface = (sour->up_interface && sour->up_interface[0]) ? strdup(sour->up_interface) : NULL;
    if((sour->up_interface && sour->up_interface[0]) && NULL == dest->up_interface) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->protocol = (sour->protocol && sour->protocol[0]) ? strdup(sour->protocol) : NULL;
    if((sour->protocol && sour->protocol[0]) && NULL == dest->protocol) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->p2p_detail = (sour->p2p_detail && sour->p2p_detail[0]) ? strdup(sour->p2p_detail) : NULL;
    if((sour->p2p_detail && sour->p2p_detail[0]) && NULL == dest->p2p_detail) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    dest->addrtype = (sour->addrtype && sour->addrtype[0]) ? strdup(sour->addrtype) : NULL;
    if((sour->addrtype && sour->addrtype[0]) && NULL == dest->addrtype) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->addr_begin = (sour->addr_begin && sour->addr_begin[0]) ? strdup(sour->addr_begin) : NULL;
    if((sour->addr_begin && sour->addr_begin[0]) && NULL == dest->addr_begin) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->addr_end = (sour->addr_end && sour->addr_end[0]) ? strdup(sour->addr_end) : NULL;
    if((sour->addr_end && sour->addr_end[0]) && NULL == dest->addr_end) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    dest->mode = (sour->mode && sour->mode[0]) ? strdup(sour->mode) : NULL;
    if((sour->mode && sour->mode[0]) && NULL == dest->mode) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    dest->uplink_speed = (sour->uplink_speed && sour->uplink_speed[0]) ? strdup(sour->uplink_speed) : NULL;
    if((sour->uplink_speed && sour->uplink_speed[0]) && NULL == dest->uplink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->downlink_speed = (sour->downlink_speed && sour->downlink_speed[0]) ? strdup(sour->downlink_speed) : NULL;
    if((sour->downlink_speed && sour->downlink_speed[0]) && NULL == dest->downlink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->useP2P = sour->useP2P;

    dest->p2p_uplink_speed = (sour->p2p_uplink_speed && sour->p2p_uplink_speed[0]) ? strdup(sour->p2p_uplink_speed) : NULL;
    if((sour->p2p_uplink_speed && sour->p2p_uplink_speed[0]) && NULL == dest->p2p_uplink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    dest->p2p_downlink_speed = (sour->p2p_downlink_speed && sour->p2p_downlink_speed[0]) ? strdup(sour->p2p_downlink_speed) : NULL;
    if((sour->p2p_downlink_speed && sour->p2p_downlink_speed[0]) && NULL == dest->p2p_downlink_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    dest->time_begin = sour->time_begin;    
    dest->time_end = sour->time_end;

    dest->limit_speed = (sour->limit_speed && sour->limit_speed[0]) ? strdup(sour->limit_speed) : NULL;
    if((sour->limit_speed && sour->limit_speed[0]) && NULL == dest->limit_speed) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    return AC_MANAGE_SUCCESS;
}



int
ac_manage_config_flow_control_service(DBusConnection *connection, unsigned int status) {
    if(NULL == connection) {
        return  AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
	DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;

    int ret = AC_MANAGE_DBUS_ERROR;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
                                         AC_MANAGE_TCRULE_DBUS_OBJPATH,
                                         AC_MANAGE_TCRULE_DBUS_INTERFACE,
                                         AC_MANAGE_DBUS_CONFIG_FLOW_CONTROL_SERVICE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    return ret;
    
}


int 
ac_manage_add_tcrule(DBusConnection *connection, TCRule *tcRuleNew) {

    if(!connection || !tcRuleNew)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    char *name = tcRuleNew->name ? tcRuleNew->name : "";
    char *comment = tcRuleNew->comment ? tcRuleNew->comment : "";
    char *interface = tcRuleNew->interface ? tcRuleNew->interface : "";
    char *up_interface = tcRuleNew->up_interface ? tcRuleNew->up_interface : "";
    char *protocol = tcRuleNew->protocol ? tcRuleNew->protocol : "";
    char *p2p_detail = tcRuleNew->p2p_detail ? tcRuleNew->p2p_detail : "";
    char *addrtype = tcRuleNew->addrtype ? tcRuleNew->addrtype : "";
    char *addr_begin = tcRuleNew->addr_begin ? tcRuleNew->addr_begin : "";
    char *addr_end = tcRuleNew->addr_end ? tcRuleNew->addr_end : "";
    char *mode = tcRuleNew->mode ? tcRuleNew->mode : "";
    char *uplink_speed = tcRuleNew->uplink_speed ? tcRuleNew->uplink_speed : "";
    char *downlink_speed = tcRuleNew->downlink_speed ? tcRuleNew->downlink_speed : "";    
    char *p2p_uplink_speed = tcRuleNew->p2p_uplink_speed ? tcRuleNew->p2p_uplink_speed : "";
    char *p2p_downlink_speed = tcRuleNew->p2p_downlink_speed ? tcRuleNew->p2p_downlink_speed : "";
    char *limit_speed = tcRuleNew->limit_speed ? tcRuleNew->limit_speed : "";
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_ADD_TCRULE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &name,
                            DBUS_TYPE_UINT32, &tcRuleNew->enable,
                            DBUS_TYPE_UINT32, &tcRuleNew->ruleIndex,
                            DBUS_TYPE_STRING, &comment,
                            DBUS_TYPE_STRING, &interface,
                            DBUS_TYPE_STRING, &up_interface,
                            DBUS_TYPE_STRING, &protocol,
                            DBUS_TYPE_STRING, &p2p_detail,
                            DBUS_TYPE_STRING, &addrtype,
                            DBUS_TYPE_STRING, &addr_begin,
                            DBUS_TYPE_STRING, &addr_end,
                            DBUS_TYPE_STRING, &mode,
                            DBUS_TYPE_STRING, &uplink_speed,
                            DBUS_TYPE_STRING, &downlink_speed,
                            DBUS_TYPE_UINT32, &tcRuleNew->useP2P,
                            DBUS_TYPE_STRING, &p2p_uplink_speed,
                            DBUS_TYPE_STRING, &p2p_downlink_speed,
                            DBUS_TYPE_UINT32, &tcRuleNew->time_begin,
                            DBUS_TYPE_UINT32, &tcRuleNew->time_end,    
                            DBUS_TYPE_STRING, &limit_speed,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}

int 
ac_manage_offset_tcrule(DBusConnection *connection, struct tcrule_offset_s *offset) {

    if(NULL == connection || NULL == offset)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_OFFSET_TCRULE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &offset->ruleIndex,
                            DBUS_TYPE_UINT32, &offset->uplink_offset,
                            DBUS_TYPE_UINT32, &offset->downlink_offset,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}


int 
ac_manage_delete_tcrule(DBusConnection *connection, unsigned int index) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_DELETE_TCRULE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &index,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}

int 
ac_manage_show_flow_control_service(DBusConnection *connection, unsigned int *status) {

    if(NULL == connection || NULL == status)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_SHOW_FLOW_CONTROL_SERVICE);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, status);

    dbus_message_unref(reply);

    return AC_MANAGE_SUCCESS;
}

int 
ac_manage_show_tcrule(DBusConnection *connection, TCRule **rule_array, unsigned int *count) {

    if(NULL == connection || NULL == rule_array || NULL == count)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    *rule_array = NULL;
    *count = 0;

    unsigned int temp_count = 0;
    
	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_SHOW_TCRULE);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &temp_count);
    
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_count) {
        TCRule *temp_array = (TCRule *)calloc(temp_count, sizeof(TCRule));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }
        
        int i = 0;
        for(i = 0; i < temp_count; i++) {

            TCRule tempRule = { 0 };
                
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &tempRule.name);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.enable);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.ruleIndex);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.comment);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.interface);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.up_interface);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.protocol);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.p2p_detail);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.addrtype);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.addr_begin);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.addr_end);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.mode);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.uplink_speed);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.downlink_speed);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.useP2P);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.p2p_uplink_speed);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.p2p_downlink_speed);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.time_begin);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.time_end);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &tempRule.limit_speed);

            if(AC_MANAGE_SUCCESS != tcrule_data_copy_v2(&temp_array[i],&tempRule)) {
                dbus_message_unref(reply);
                tcFreeArray(&temp_array, temp_count);
                return AC_MANAGE_MALLOC_ERROR;
            }
            
            dbus_message_iter_next(&iter_array);
        }
        
        *rule_array = temp_array;
        *count = temp_count;
    }
	
    dbus_message_unref(reply);
    
    return ret;
}


int 
ac_manage_show_tcrule_offset(DBusConnection *connection, struct tcrule_offset_s **offset_array, unsigned int *count) {

    if(NULL == connection || NULL == offset_array || NULL == count)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    *offset_array = NULL;
    *count = 0;

    unsigned int temp_count = 0;
    
	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
	                                     AC_MANAGE_TCRULE_DBUS_OBJPATH,
	                                     AC_MANAGE_TCRULE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_SHOW_TCRULE_OFFSET);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &temp_count);
    
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_count) {
        struct tcrule_offset_s *temp_array = (struct tcrule_offset_s *)calloc(temp_count, sizeof(struct tcrule_offset_s));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }
        
        int i = 0;
        for(i = 0; i < temp_count; i++) {
                
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_array[i].ruleIndex);
            
            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &temp_array[i].uplink_offset);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_get_basic(&iter_struct, &temp_array[i].downlink_offset);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *offset_array = temp_array;
        *count = temp_count;
    }
	
    dbus_message_unref(reply);
    
    return ret;
}

