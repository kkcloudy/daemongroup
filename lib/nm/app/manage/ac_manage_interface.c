#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ws_webservice_conf.h"
#include "ac_manage_interface.h"

/*
 *检查snmp用户名是否合法。若允许包含字符\，则flag置1，不允许置0
 */
int 
check_snmp_name(char *name, int flag) {	
	if((name == NULL)||strcmp(name, "")==0)
		return -1;

	int len_name = strlen(name);
	
	if( (len_name < 0) || (len_name > 20))
		return -2;
		
	char *p = NULL;
	char *tmp = name;
	int i = 0;
	
	for(i=0; i < len_name; i++, tmp++) {
		if((*tmp) < 33 || (*tmp) > 127 )
			return -4;
	}
	
	switch(*name){
	
		/*不能以 # " '开头*/
		
		case '\"':
			return -3;
		case '\'':
			return -3;
		case '#':
			return -3;	
	}
	
	if(1 == flag) {
		/*特殊符号"\" 只允许出现在末尾*/
		p = strstr(name, "\\");
		if( (p!=NULL)&&(*(++p) != '\0') ) 
			return -3;
	}
	else {
		/*不允许出现特殊符号"\" */
		p = strstr(name, "\\");
		if(p!=NULL) 
			return -5;
	}
	
	return 1;
}


unsigned char 
set_bit(unsigned char num, unsigned char position) {
	unsigned char bit_value[]={1,2,4,8,16,32,64,128};
	return num|bit_value[position];	
}

unsigned char 
clear_bit(unsigned char num, unsigned char position) {
	unsigned char bit_value[]={1,2,4,8,16,32,64,128};
	return num&~bit_value[position];
}

unsigned char 
get_bit(unsigned char num, unsigned char position) {
	if(((num>>position) & 0x01) == 1)
		return 1;
	else
		return 0;
}

int 
string_to_trap_group_switch(const char *switch_string, struct trap_group_switch *group_switch) {
    if(NULL == switch_string || NULL == group_switch) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    memset(group_switch, 0, sizeof(struct trap_group_switch));
    
    int i = 0;
    unsigned int string_len = strlen(switch_string);
    for(i = 0; i < string_len && i < 64; i++) {
        if('1' == switch_string[i]) {
            group_switch->low_switch |= (unsigned long long)0x1 << i;
        }
        else if('0' != switch_string[i]){
            return AC_MANAGE_INPUT_TYPE_ERROR;
        }
    }
    for(; i < string_len && i < (64 * 2); i++) {
        if('1' == switch_string[i]) {
            group_switch->high_switch |= (unsigned long long)0x1 << (i - 64);
        }
        else if('0' != switch_string[i]){
            return AC_MANAGE_INPUT_TYPE_ERROR;
        }
    }
    
    return AC_MANAGE_SUCCESS;
}


int
ac_manage_config_log_debug(DBusConnection *connection, unsigned int state) {
    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_LOG_DEBUG);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &state,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
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
ac_manage_config_token_debug(DBusConnection *connection, unsigned int token, unsigned int state) {
	if(NULL == connection) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}	
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_DBUS_OBJPATH,
											AC_MANAGE_DBUS_INTERFACE,
											AC_MANAGE_DBUS_CONFIG_TOKEN_DEBUG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &token,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
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
ac_manage_config_snmp_log_debug(DBusConnection *connection, unsigned int debugLevel) {
    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_LOG_DEBUG);

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
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}


int
ac_manage_config_trap_log_debug(DBusConnection *connection, unsigned int debugLevel) {
    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_LOG_DEBUG);

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
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}

int
ac_manage_show_snmp_log_debug(DBusConnection *connection, unsigned int *debugLevel) {

    if(NULL == connection || NULL == debugLevel)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter iter;

    *debugLevel = 0;

    int ret = AC_MANAGE_SUCCESS;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_LOG_DEBUG);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, debugLevel);
    
    dbus_message_unref(reply);

    return ret;     
}


int
ac_manage_show_trap_log_debug(DBusConnection *connection, unsigned int *debugLevel) {

    if(NULL == connection || NULL == debugLevel)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter iter;

    *debugLevel = 0;

    int ret = AC_MANAGE_SUCCESS;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_LOG_DEBUG);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, debugLevel);
    
    dbus_message_unref(reply);

    return ret;     
}


int
ac_manage_config_snmp_manual_instance(DBusConnection *connection, unsigned int local_id, unsigned int instance_id, unsigned int status) {

    if(NULL == connection || local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter iter;

    int ret = AC_MANAGE_SUCCESS;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_MANUAL_INSTANCE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &local_id,
                            DBUS_TYPE_UINT32, &instance_id,
                            DBUS_TYPE_UINT32, &status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
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
ac_manage_show_snmp_manual_instance(DBusConnection *connection, unsigned int *manual_instance) {

    if(NULL == connection || NULL == manual_instance)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter iter;

    *manual_instance = 0;

    int ret = AC_MANAGE_SUCCESS;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_MANUAL_INSTANCE);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);
    
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, manual_instance);
    
    dbus_message_unref(reply);

    return ret;     
}


int 
ac_manage_config_snmp_service(DBusConnection *connection, unsigned int state) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_SERVICE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &state,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
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
ac_manage_config_snmp_collection_mode(DBusConnection *connection, unsigned int collection_mode) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_COLLECTION_MODE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &collection_mode,
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
ac_manage_config_snmp_pfm_requestpkts(DBusConnection *connection, 
                                                    char *ifName, 
                                                    unsigned int port,
                                                    unsigned int state) {

    if(NULL == connection || NULL == ifName)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_PFM_REQUESTPKTS);

    dbus_error_init(&err);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &ifName,
                                DBUS_TYPE_UINT32, &port,
                                DBUS_TYPE_UINT32, &state, /*0:delete , 1:add*/
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
ac_manage_config_snmp_pfm_requestpkts_ipv6(DBusConnection *connection, 
                                                    char *ifName, 
                                                    unsigned int port,
                                                    unsigned int state) {

    if(NULL == connection || NULL == ifName)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_PFM_REQUESTPKTS_IPV6);

    dbus_error_init(&err);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &ifName,
                                DBUS_TYPE_UINT32, &port,
                                DBUS_TYPE_UINT32, &state, /*0:delete , 1:add*/
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
ac_manage_config_snmp_version_mode(DBusConnection *connection, unsigned int version, unsigned int state) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_VERSION_MODE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &version,
                            DBUS_TYPE_UINT32, &state,
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
ac_manage_config_snmp_update_sysinfo(DBusConnection *connection) {
    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_UPDATE_SYSINFO);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
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
ac_manage_config_snmp_cachetime(DBusConnection *connection, unsigned int cachetime) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_CACHETIME);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &cachetime,
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
ac_manage_config_snmp_add_community(DBusConnection *connection, STCommunity *community_node) {

    if(!connection || !community_node || 0 == strlen(community_node->community))
        return AC_MANAGE_INPUT_TYPE_ERROR;

    char *community = strdup(community_node->community);
    char *ip_addr = NULL;
    char *ip_mask = NULL;
    if(0 == strlen(community_node->ip_addr)){
        ip_addr = strdup("0.0.0.0");
    }
    else {
        ip_addr = strdup(community_node->ip_addr);
    }

    if(0 == strlen(community_node->ip_mask)){
        ip_mask = strdup("0.0.0.0");
    }
    else {
        ip_mask = strdup(community_node->ip_mask);
    }

    if(!community || !ip_addr || !ip_mask) {
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        MANAGE_FREE(ip_mask);
        return AC_MANAGE_MALLOC_ERROR;
    }
    

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_ADD_COMMUNITY);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &community,
                            DBUS_TYPE_STRING, &ip_addr,
                            DBUS_TYPE_STRING, &ip_mask,
                            DBUS_TYPE_UINT32, &community_node->access_mode,
                            DBUS_TYPE_UINT32, &community_node->status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        MANAGE_FREE(ip_mask);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    MANAGE_FREE(community);
    MANAGE_FREE(ip_addr);
    MANAGE_FREE(ip_mask);
    return ret;
}

int 
ac_manage_config_snmp_add_community_ipv6(DBusConnection *connection, IPV6STCommunity *communityIPV6Node)
{

    if(!connection || !communityIPV6Node || 0 == strlen(communityIPV6Node->community))
	return AC_MANAGE_INPUT_TYPE_ERROR;

    char *community = strdup(communityIPV6Node->community);
    char *ip_addr = NULL;
    unsigned int prefix = 0;
    if(0 == strlen(communityIPV6Node->ip_addr)){
	ip_addr = strdup("::");
    }
    else {
	ip_addr = strdup(communityIPV6Node->ip_addr);
    }

    if(!community || !ip_addr ) {
	MANAGE_FREE(community);
	MANAGE_FREE(ip_addr);
	return AC_MANAGE_MALLOC_ERROR;
    }
    

    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;
    
    int ret = AC_MANAGE_DBUS_ERROR;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
					AC_MANAGE_SNMP_DBUS_OBJPATH,
					AC_MANAGE_SNMP_DBUS_INTERFACE,
					AC_MANAGE_DBUS_CONFIG_SNMP_ADD_COMMUNITY_IPV6);

    dbus_error_init(&err);

    dbus_message_append_args(query,
			    DBUS_TYPE_STRING, &community,
			    DBUS_TYPE_STRING, &ip_addr,
			    DBUS_TYPE_UINT32, &communityIPV6Node->prefix,
			    DBUS_TYPE_UINT32, &communityIPV6Node->access_mode,
			    DBUS_TYPE_UINT32, &communityIPV6Node->status,
			    DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
	if(dbus_error_is_set(&err)) {
	    dbus_error_free(&err);
	}
	MANAGE_FREE(community);
	MANAGE_FREE(ip_addr);
	return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    MANAGE_FREE(community);
    MANAGE_FREE(ip_addr);
    return ret;
}



int 
ac_manage_config_snmp_set_community(DBusConnection *connection,
                                                    char *communityName,
                                                    STCommunity *community_node) {

    if(!connection || !communityName || !community_node || 0 == strlen(community_node->community))
        return AC_MANAGE_INPUT_TYPE_ERROR;

    
    char *community = strdup(community_node->community);
    char *ip_addr = NULL;
    char *ip_mask = NULL;
    if(0 == strlen(community_node->ip_addr)){
        ip_addr = strdup("0.0.0.0");
    }
    else {
        ip_addr = strdup(community_node->ip_addr);
    }

    if(0 == strlen(community_node->ip_mask)){
        ip_mask = strdup("0.0.0.0");
    }
    else {
        ip_mask = strdup(community_node->ip_mask);
    }
    
    if(!community || !ip_addr || !ip_mask) {
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        MANAGE_FREE(ip_mask);
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_SET_COMMUNITY);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &communityName,
                            DBUS_TYPE_STRING, &community,
                            DBUS_TYPE_STRING, &ip_addr,
                            DBUS_TYPE_STRING, &ip_mask,
                            DBUS_TYPE_UINT32, &community_node->access_mode,
                            DBUS_TYPE_UINT32, &community_node->status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        MANAGE_FREE(ip_mask);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    
    MANAGE_FREE(community);
    MANAGE_FREE(ip_addr);
    MANAGE_FREE(ip_mask);

    return ret;    
}

int
ac_manage_config_snmp_set_community_ipv6(DBusConnection *connection,char *communityName,IPV6STCommunity *community_node) 
{

    if(!connection || !communityName || !community_node || 0 == strlen(community_node->community))
        return AC_MANAGE_INPUT_TYPE_ERROR;

    
    char *community = strdup(community_node->community);
    char *ip_addr = NULL;
    if(0 == strlen(community_node->ip_addr)){
        ip_addr = strdup("::");
    }
    else {
        ip_addr = strdup(community_node->ip_addr);
    }
    
    if(!community || !ip_addr) {
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_SET_COMMUNITY_IPV6);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &communityName,
                            DBUS_TYPE_STRING, &community,
                            DBUS_TYPE_STRING, &ip_addr,
                            DBUS_TYPE_UINT32, &community_node->prefix,
                            DBUS_TYPE_UINT32, &community_node->access_mode,
                            DBUS_TYPE_UINT32, &community_node->status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(community);
        MANAGE_FREE(ip_addr);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    
    MANAGE_FREE(community);
    MANAGE_FREE(ip_addr);

    return ret;    
}



int 
ac_manage_config_snmp_del_community(DBusConnection *connection, 
                                                    char *community) {

    if(!connection || !community || strlen(community) >= (MAX_SNMP_NAME_LEN))
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_DEL_COMMUNITY);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &community,
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
ac_manage_config_snmp_del_community_ipv6(DBusConnection *connection, char *community)
{

    if(!connection || !community || strlen(community) >= (MAX_SNMP_NAME_LEN))
	return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_DEL_COMMUNITY_IPV6);

    dbus_error_init(&err);

    dbus_message_append_args(query,
			    DBUS_TYPE_STRING, &community,
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
ac_manage_config_snmp_view(DBusConnection *connection, 
                                      char *view_name,
                                      unsigned int mode) {

    if(!connection || !view_name || strlen(view_name) >= MAX_VIEW_NAME_LEN)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_VIEW);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &view_name,
                            DBUS_TYPE_UINT32, &mode,        /*mode : 1 create view; 0 delete view*/
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
ac_manage_check_snmp_view(DBusConnection *connection, char *view_name) {

    if(!connection || !view_name || strlen(view_name) >= MAX_VIEW_NAME_LEN)
        return AC_MANAGE_INPUT_TYPE_ERROR;

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CHECK_SNMP_VIEW);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &view_name,
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
ac_manage_config_snmp_view_oid(DBusConnection *connection, 
                                          char *view_name,
                                          char *oid,
                                          unsigned int oid_type,
                                          unsigned int mode) {

    if(!connection || !view_name || !oid || 
        strlen(view_name) >= MAX_VIEW_NAME_LEN || strlen(oid) >= MAX_oid) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_VIEW_OID);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &view_name,
                            DBUS_TYPE_STRING, &oid,
                            DBUS_TYPE_UINT32, &oid_type,     /*oid_type : 1 include oid; 0 exclude oid*/
                            DBUS_TYPE_UINT32, &mode,        /*mode : 1 create oid; 0 delete oid*/
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
ac_manage_config_snmp_add_group(DBusConnection *connection, STSNMPGroup *group_node) {

    if(!connection || !group_node || strlen(group_node->group_name) >= MAX_GROUP_NAME_LEN || 
        strlen(group_node->group_view) >= MAX_VIEW_NAME_LEN) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    
    
    char *group_name = strdup(group_node->group_name);
    char *view_name = strdup(group_node->group_view);
     
    if(!group_name || !view_name) {
        MANAGE_FREE(group_name);
        MANAGE_FREE(view_name);
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_ADD_GROUP);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &group_name,
                            DBUS_TYPE_STRING, &view_name,
                            DBUS_TYPE_UINT32, &group_node->access_mode,    
                            DBUS_TYPE_UINT32, &group_node->sec_level,        
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(group_name);
        MANAGE_FREE(view_name);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    MANAGE_FREE(group_name);
    MANAGE_FREE(view_name);
    
    return ret;    
}

int 
ac_manage_config_snmp_del_group(DBusConnection *connection, char *group_name) {

    if(!connection || !group_name || strlen(group_name) >= MAX_GROUP_NAME_LEN) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_DEL_GROUP);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &group_name,
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
ac_manage_config_snmp_add_v3user(DBusConnection *connection, STSNMPV3User *v3user_node) {

    if(NULL == connection || NULL == v3user_node){
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    if(strlen(v3user_node->name) >= MAX_SNMP_NAME_LEN || strlen(v3user_node->authentication.passwd) >= MAX_SNMP_PASSWORD_LEN
        || strlen(v3user_node->privacy.passwd) >= MAX_SNMP_PASSWORD_LEN || strlen(v3user_node->group_name) >= MAX_GROUP_NAME_LEN) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    
    char *v3user_name = strdup(v3user_node->name);
    char *ath_passwd = strdup(v3user_node->authentication.passwd);
    char *priv_passwd = strdup(v3user_node->privacy.passwd);
    char *group_name = strdup(v3user_node->group_name);
    
    if(!v3user_name || !ath_passwd || !priv_passwd || !group_name) {
        MANAGE_FREE(v3user_name);
        MANAGE_FREE(ath_passwd);
        MANAGE_FREE(priv_passwd);
        MANAGE_FREE(group_name);
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_ADD_V3USER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &v3user_name,
                            DBUS_TYPE_UINT32, &v3user_node->authentication.protocal,
                            DBUS_TYPE_STRING, &ath_passwd,    
                            DBUS_TYPE_UINT32, &v3user_node->privacy.protocal,
                            DBUS_TYPE_STRING, &priv_passwd,
                            DBUS_TYPE_STRING, &group_name,    
                            DBUS_TYPE_UINT32, &v3user_node->status,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        
        MANAGE_FREE(v3user_name);
        MANAGE_FREE(ath_passwd);
        MANAGE_FREE(priv_passwd);
        MANAGE_FREE(group_name);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    MANAGE_FREE(v3user_name);
    MANAGE_FREE(ath_passwd);
    MANAGE_FREE(priv_passwd);
    MANAGE_FREE(group_name);
    
    return ret;    
}

int 
ac_manage_config_snmp_del_v3user(DBusConnection *connection, char *v3user_name) {

    if(!connection || !v3user_name || strlen(v3user_name) >= MAX_SNMP_NAME_LEN) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_DEL_V3USER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &v3user_name,
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
ac_manage_show_snmp_state(DBusConnection *connection, unsigned int *snmp_state) {

    if(NULL == connection || NULL == snmp_state) 
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_STATE);

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
	dbus_message_iter_get_basic(&iter, snmp_state);

	dbus_message_unref(reply);
	
	return AC_MANAGE_SUCCESS;
}



int 
ac_manage_show_snmp_base_info(DBusConnection *connection, STSNMPSysInfo *snmp_info) {

    if(NULL == connection || NULL == snmp_info)
        return AC_MANAGE_INPUT_TYPE_ERROR;

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

    int ret = AC_MANAGE_SUCCESS;    
	memset(snmp_info, 0, sizeof(STSNMPSysInfo));
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_BASE_INFO);

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

    if(AC_MANAGE_SUCCESS == ret) {

        char *sys_name = NULL;
        char *sys_description = NULL;
        char *sys_oid = NULL;

    	dbus_message_iter_next(&iter);	
    	dbus_message_iter_get_basic(&iter, &sys_name);
    	
        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&sys_description);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&sys_oid);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->agent_port);   
		
        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->agent_port_ipv6);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->collection_mode);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->cache_time);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->v1_status);   

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->v2c_status); 

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter,&snmp_info->v3_status);
        
        strncpy(snmp_info->sys_name, sys_name, sizeof(snmp_info->sys_name) - 1);
        strncpy(snmp_info->sys_description, sys_description, sizeof(snmp_info->sys_description) - 1);
        strncpy(snmp_info->sys_oid, sys_oid, sizeof(snmp_info->sys_oid) - 1);
        
    }    
    
    dbus_message_unref(reply);
    
    return ret;    
}

int 
ac_manage_show_snmp_pfm_interface(DBusConnection *connection, 
                                                SNMPINTERFACE **interface_array,
                                                unsigned int *interface_num,
                                                unsigned int *snmp_port) {
                                            
    if(NULL == connection || NULL == interface_array || NULL == interface_num || NULL == snmp_port)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *interface_array = NULL;
    *interface_num = 0;
    *snmp_port = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_PFM_INTERFACE);

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
	dbus_message_iter_get_basic(&iter, snmp_port);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){
        SNMPINTERFACE *temp_array = (SNMPINTERFACE *)calloc(temp_num, sizeof(SNMPINTERFACE));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *ifName = NULL;

            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ifName);

            strncpy(temp_array[i].ifName, ifName, sizeof(temp_array[i].ifName) - 1);
            
            dbus_message_iter_next(&iter_array);
        }

        *interface_array = temp_array;
        *interface_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

int 
ac_manage_show_snmp_pfm_interface_ipv6(DBusConnection *connection, 
                                                SNMPINTERFACE **interface_array,
                                                unsigned int *interface_num,
                                                unsigned int *snmp_port) {
                                            
	if(NULL == connection || NULL == interface_array || NULL == interface_num || NULL == snmp_port)
		return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	*interface_array = NULL;
	*interface_num = 0;
	*snmp_port = 0;

	int ret = AC_MANAGE_SUCCESS;
	unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
					AC_MANAGE_SNMP_DBUS_OBJPATH,
					AC_MANAGE_SNMP_DBUS_INTERFACE,
					AC_MANAGE_DBUS_SHOW_SNMP_PFM_INTERFACE_IPV6);

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
	dbus_message_iter_get_basic(&iter, snmp_port);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){
        SNMPINTERFACE *temp_array = (SNMPINTERFACE *)calloc(temp_num, sizeof(SNMPINTERFACE));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *ifName = NULL;

            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ifName);

            strncpy(temp_array[i].ifName, ifName, sizeof(temp_array[i].ifName) - 1);
            
            dbus_message_iter_next(&iter_array);
        }

        *interface_array = temp_array;
        *interface_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}


int 
ac_manage_show_snmp_community(DBusConnection *connection, 
                                            STCommunity **community_array,
                                            unsigned int *community_num) {
                                            
    if(NULL == connection || NULL == community_array || NULL == community_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *community_array = NULL;
    *community_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_COMMUNITY);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   
    if(AC_MANAGE_SUCCESS == ret && temp_num){
        STCommunity *temp_array = (STCommunity *)calloc(temp_num, sizeof(STCommunity));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *community = NULL;
            char *ip_addr = NULL;
            char *ip_mask = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &community);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ip_addr);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ip_mask);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].access_mode));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].status));

            strncpy(temp_array[i].community, community, sizeof(temp_array[i].community) - 1);
            strncpy(temp_array[i].ip_addr, ip_addr, sizeof(temp_array[i].ip_addr) - 1);
            strncpy(temp_array[i].ip_mask, ip_mask, sizeof(temp_array[i].ip_mask) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *community_array = temp_array;
        *community_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

int 
ac_manage_show_snmp_community_ipv6(DBusConnection *connection, 
                                            IPV6STCommunity **community_array,
                                            unsigned int *community_num) {
                                            
	if(NULL == connection || NULL == community_array || NULL == community_num)
		return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	*community_array = NULL;
	*community_num = 0;

	int ret = AC_MANAGE_SUCCESS;
	unsigned int temp_num = 0;    
    
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
					AC_MANAGE_SNMP_DBUS_OBJPATH,
					AC_MANAGE_SNMP_DBUS_INTERFACE,
					AC_MANAGE_DBUS_SHOW_SNMP_COMMUNITY_IPV6);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
    
	if(AC_MANAGE_SUCCESS == ret && temp_num){
	IPV6STCommunity *temp_array = (IPV6STCommunity *)calloc(temp_num, sizeof(IPV6STCommunity));
	if(NULL == temp_array) {
	    dbus_message_unref(reply);
	    return AC_MANAGE_MALLOC_ERROR;
	}

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *community = NULL;
            char *ip_addr = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &community);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ip_addr);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].prefix));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].access_mode));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].status));

            strncpy(temp_array[i].community, community, sizeof(temp_array[i].community) - 1);
            strncpy(temp_array[i].ip_addr, ip_addr, sizeof(temp_array[i].ip_addr) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *community_array = temp_array;
        *community_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}


void
free_ac_manage_show_snmp_view(STSNMPView **view_array, unsigned int view_num) {
    if(NULL == view_array || NULL == *view_array) 
        return ;

    int i = 0;
    for(i = 0; i < view_num; i++) {
        struct oid_list *tempHead = (*view_array)[i].view_included.oidHead;
        while(tempHead) {
            struct oid_list *temp = tempHead->next;
            MANAGE_FREE(tempHead);
            tempHead = temp;
        }

        tempHead = (*view_array)[i].view_excluded.oidHead;        
        while(tempHead) {
            struct oid_list *temp = tempHead->next;
            MANAGE_FREE(tempHead);
            tempHead = temp;
        }
    }
    MANAGE_FREE(*view_array);
    *view_array = NULL;

    return ;
}

int 
ac_manage_show_snmp_view(DBusConnection *connection, 
                                        STSNMPView **view_array,
                                        unsigned int *view_num,
                                        char *view_name) {
                                        
    if(NULL == connection || NULL == view_array || NULL == view_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *view_array = NULL;
    *view_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_view_num = 0; 
    unsigned int mode = 0;  
    char *name_para = (char *)malloc(MAX_VIEW_NAME_LEN);
    if(NULL == name_para)
       return AC_MANAGE_MALLOC_ERROR;

    memset(name_para, 0, MAX_VIEW_NAME_LEN);
    if(view_name) {
        mode = 1;
        strncpy(name_para, view_name, MAX_VIEW_NAME_LEN - 1);
    }


    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_VIEW);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &mode,
                            DBUS_TYPE_STRING, &name_para,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(name_para);
        return AC_MANAGE_DBUS_ERROR;
    }

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
    
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &temp_view_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter, &iter_array);   
    
    if(AC_MANAGE_SUCCESS == ret && temp_view_num){
        STSNMPView *temp_array = (STSNMPView *)calloc(temp_view_num, sizeof(STSNMPView));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_view_num; i++) {
        
            DBusMessageIter  iter_sub_array;
            DBusMessageIter  iter_sub_struct;
            char *temp_name = NULL;
            
            dbus_message_iter_recurse(&iter_array,&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_name);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].view_included.oid_num));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].view_excluded.oid_num));

            strncpy(temp_array[i].name, temp_name, sizeof(temp_array[i].name) - 1);

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_recurse(&iter_struct, &iter_sub_array); 

            int j = 0;
            for(j = 0; j < temp_array[i].view_included.oid_num; j++) {

                char *temp_oid = NULL;
                
                dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &temp_oid);

                struct oid_list *temp_node = (struct oid_list *)malloc(sizeof(struct oid_list));
                if(NULL == temp_node) {
                    continue;
                }
                
                memset(temp_node, 0, sizeof(struct oid_list));
                strncpy(temp_node->oid, temp_oid, sizeof(temp_node->oid) - 1);

                temp_node->next = temp_array[i].view_included.oidHead;
                temp_array[i].view_included.oidHead = temp_node;
                
                dbus_message_iter_next(&iter_sub_array);
            }
            
            for(j = 0; j < temp_array[i].view_excluded.oid_num; j++) {

                char *temp_oid = NULL;
                
                dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &temp_oid);

                struct oid_list *temp_node = (struct oid_list *)malloc(sizeof(struct oid_list));
                if(NULL == temp_node) {
                    continue;
                }
                
                memset(temp_node, 0, sizeof(struct oid_list));
                strncpy(temp_node->oid, temp_oid, sizeof(temp_node->oid) - 1);

                temp_node->next = temp_array[i].view_excluded.oidHead;
                temp_array[i].view_excluded.oidHead = temp_node;
                
                
                dbus_message_iter_next(&iter_sub_array);
            }

            dbus_message_iter_next(&iter_array);
        }
        
        *view_array = temp_array;
        *view_num = temp_view_num;
    }

    MANAGE_FREE(name_para);

    dbus_message_unref(reply);
    
    return ret;    
}


int 
ac_manage_show_snmp_group(DBusConnection *connection, 
                                        STSNMPGroup **group_array,
                                        unsigned int *group_num,
                                        char *group_name) {
                                        
    if(NULL == connection || NULL == group_array || NULL == group_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *group_array = NULL;
    *group_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_group_num = 0; 
    unsigned int mode = 0;  
    char *name_para = (char *)malloc(MAX_GROUP_NAME_LEN);
    if(NULL == name_para)
       return AC_MANAGE_MALLOC_ERROR;

    memset(name_para, 0, MAX_GROUP_NAME_LEN);
    if(group_name) {
        mode = 1;
        strncpy(name_para, group_name, MAX_GROUP_NAME_LEN - 1);
    }

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_GROUP);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &mode,
                            DBUS_TYPE_STRING, &name_para,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(name_para);
        return AC_MANAGE_DBUS_ERROR;
    }

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &temp_group_num);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter, &iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_group_num){
        STSNMPGroup *temp_array = (STSNMPGroup *)calloc(temp_group_num, sizeof(STSNMPGroup));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_group_num; i++) {
        
            char *temp_group_name = NULL;
            char *temp_view_name = NULL;
            
            dbus_message_iter_recurse(&iter_array,&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_group_name);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_view_name);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].access_mode));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].sec_level));

            strncpy(temp_array[i].group_name, temp_group_name, sizeof(temp_array[i].group_name) - 1);
            strncpy(temp_array[i].group_view, temp_view_name, sizeof(temp_array[i].group_view) - 1);

            dbus_message_iter_next(&iter_array);
        }
        
        *group_array = temp_array;
        *group_num = temp_group_num;
    }

    MANAGE_FREE(name_para);

    dbus_message_unref(reply);
    
    return ret;    
}

int 
ac_manage_show_snmp_v3user(DBusConnection *connection, 
                                        STSNMPV3User **v3user_array,
                                        unsigned int *v3user_num,
                                        char *v3user_name) {
                                        
    if(NULL == connection || NULL == v3user_array || NULL == v3user_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *v3user_array = NULL;
    *v3user_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_v3user_num = 0; 

    unsigned int mode = 0;  
    char *name_para = (char *)malloc(MAX_SNMP_NAME_LEN);
    if(NULL == name_para)
       return AC_MANAGE_MALLOC_ERROR;
    memset(name_para, 0, MAX_SNMP_NAME_LEN);
    if(v3user_name) {
        mode = 1;
        strncpy(name_para, v3user_name, MAX_SNMP_NAME_LEN - 1);
    }

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_V3USER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &mode,
                            DBUS_TYPE_STRING, &name_para,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(name_para);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &temp_v3user_num);	

    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter, &iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_v3user_num){

        STSNMPV3User *temp_array = (STSNMPV3User *)calloc(temp_v3user_num, sizeof(STSNMPV3User));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            MANAGE_FREE(name_para);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_v3user_num; i++) {
            char *temp_v3user_name = NULL;
            char *temp_group_name = NULL;
            char *temp_auth_passwd = NULL;
            char *temp_priv_passwd = NULL;
            
            dbus_message_iter_recurse(&iter_array,&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_v3user_name);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].authentication.protocal));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_auth_passwd);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].privacy.protocal));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_priv_passwd);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &temp_group_name);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].status));


            strncpy(temp_array[i].name, temp_v3user_name, sizeof(temp_array[i].name) - 1);
            strncpy(temp_array[i].authentication.passwd, temp_auth_passwd, sizeof(temp_array[i].authentication.passwd) - 1);
            strncpy(temp_array[i].privacy.passwd, temp_priv_passwd, sizeof(temp_array[i].privacy.passwd) - 1);
            strncpy(temp_array[i].group_name, temp_group_name, sizeof(temp_array[i].group_name) - 1);

            dbus_message_iter_next(&iter_array);
        }
        
        *v3user_array = temp_array;
        *v3user_num = temp_v3user_num;
    }
    
    MANAGE_FREE(name_para);

    dbus_message_unref(reply);
    
    return ret;    
}


int 
ac_manage_config_trap_service(DBusConnection *connection, unsigned int state) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_SERVICE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &state,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
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
ac_manage_config_trap_config_receiver(DBusConnection *connection, STSNMPTrapReceiver *receiver, unsigned int type) {

    if(NULL == connection || NULL == receiver)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    if(3 != receiver->version && 0 == strlen(receiver->trapcom))
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    char *temp_name = (char *)malloc(MAX_SNMP_NAME_LEN);
    char *temp_community = (char *)malloc(MAX_SNMP_NAME_LEN);
    char *temp_sourIPAddr = (char *)malloc(MAX_IP_ADDR_LEN);
    char *temp_destIPAddr = (char *)malloc(MAX_IP_ADDR_LEN);
    
    if(!temp_name || !temp_community || !temp_sourIPAddr || !temp_destIPAddr) {
		MANAGE_FREE(temp_name);
		MANAGE_FREE(temp_community);
		MANAGE_FREE(temp_sourIPAddr);
		MANAGE_FREE(temp_destIPAddr);
		return AC_MANAGE_MALLOC_ERROR;
    }
    
    memset(temp_name, 0, MAX_SNMP_NAME_LEN);
    strncpy(temp_name, receiver->name, MAX_SNMP_NAME_LEN - 1);

    memset(temp_sourIPAddr, 0, MAX_IP_ADDR_LEN);
    strncpy(temp_sourIPAddr, receiver->sour_ipAddr, MAX_IP_ADDR_LEN - 1);
    
    memset(temp_destIPAddr, 0, MAX_IP_ADDR_LEN);
    strncpy(temp_destIPAddr, receiver->dest_ipAddr, MAX_IP_ADDR_LEN - 1);
    
    memset(temp_community, 0, MAX_SNMP_NAME_LEN);
    if(3 != receiver->version) {
        strncpy(temp_community, receiver->trapcom, MAX_SNMP_NAME_LEN - 1);    
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_CONFIG_RECEIVER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &receiver->local_id,
                            DBUS_TYPE_UINT32, &receiver->instance_id,
                            DBUS_TYPE_UINT32, &receiver->version,
                            DBUS_TYPE_STRING, &temp_name,
                            DBUS_TYPE_STRING, &temp_sourIPAddr,
                            DBUS_TYPE_STRING, &temp_destIPAddr,   
                            DBUS_TYPE_UINT32, &receiver->dest_port,
							DBUS_TYPE_STRING, &temp_community,
							DBUS_TYPE_UINT32, &receiver->status,
                            DBUS_TYPE_UINT32, &type,        /*0: set receiver    1:add recevier*/
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
		MANAGE_FREE(temp_name);
		MANAGE_FREE(temp_community);
		MANAGE_FREE(temp_sourIPAddr);
		MANAGE_FREE(temp_destIPAddr);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    MANAGE_FREE(temp_name);
	MANAGE_FREE(temp_community);
	MANAGE_FREE(temp_sourIPAddr);
	MANAGE_FREE(temp_destIPAddr);
    
    return ret;
}


int 
ac_manage_config_trap_del_receiver(DBusConnection *connection, char *name) {

    if(NULL == connection || NULL == name)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_DEL_RECEIVER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &name,
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
ac_manage_config_trap_switch(DBusConnection *connection, unsigned int index, TRAP_DETAIL_CONFIG *trapDetail) {

    if(NULL == connection || NULL == trapDetail)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    char *trapName = NULL;
    char *trapEDes = NULL;
    if(0 == strlen(trapDetail->trapName)){
        trapName = strdup("");
    }
    else {
        trapName = strdup(trapDetail->trapName);
    }

    if(0 == strlen(trapDetail->trapEDes)){
        trapEDes = strdup("");
    }
    else {
        trapEDes = strdup(trapDetail->trapEDes);
    }
    
    if(!trapName || !trapEDes) {
        MANAGE_FREE(trapName);
        MANAGE_FREE(trapEDes);
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_SWITCH);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &index,
                            DBUS_TYPE_STRING, &trapName,
                            DBUS_TYPE_STRING, &trapEDes,
                            DBUS_TYPE_UINT32, &trapDetail->trapSwitch,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        
        MANAGE_FREE(trapName);
        MANAGE_FREE(trapEDes);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    MANAGE_FREE(trapName);
    MANAGE_FREE(trapEDes);    
    return ret;
}

int 
ac_manage_config_trap_group_switch(DBusConnection *connection, struct trap_group_switch
*group_switch) {

    if(NULL == connection || NULL == group_switch)
        return AC_MANAGE_INPUT_TYPE_ERROR;

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_GROUP_SWITCH);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT64, &group_switch->low_switch,
                            DBUS_TYPE_UINT64, &group_switch->high_switch,
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
ac_manage_config_trap_instance_heartbeat(DBusConnection *connection, TRAPHeartbeatIP *heartbeatNode) {

    if(NULL == connection || NULL == heartbeatNode)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    if(heartbeatNode->local_id >= VRRP_TYPE_NUM || heartbeatNode->instance_id > INSTANCE_NUM || 
        0 == heartbeatNode->instance_id || 0 == strlen(heartbeatNode->ipAddr)) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    char *ipAddr = strdup(heartbeatNode->ipAddr);
    if(NULL == ipAddr) {
        return AC_MANAGE_MALLOC_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_INSTANCE_HEARTBEAT);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &heartbeatNode->local_id,
                            DBUS_TYPE_UINT32, &heartbeatNode->instance_id,
                            DBUS_TYPE_STRING, &ipAddr,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        
        MANAGE_FREE(ipAddr);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    MANAGE_FREE(ipAddr);
    return ret;
}

int 
ac_manage_clear_trap_instance_heartbeat(DBusConnection *connection, unsigned int local_id, unsigned int instance_id) {

    if(NULL == connection || local_id >= VRRP_TYPE_NUM || instance_id > INSTANCE_NUM || 0 == instance_id)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CLEAR_TRAP_INSTANCE_HEARTBEAT);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &local_id,
                            DBUS_TYPE_UINT32, &instance_id,
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
ac_manage_config_trap_parameter(DBusConnection *connection, char *paraStr, unsigned int paraData) {

    if(NULL == connection || NULL == paraStr)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_TRAP_PARAMETER);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &paraStr,
                            DBUS_TYPE_UINT32, &paraData,
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
ac_manage_show_trap_state(DBusConnection *connection, unsigned int *trap_state) {

    if(NULL == connection || NULL == trap_state) 
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_STATE);

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
	dbus_message_iter_get_basic(&iter, trap_state);

	dbus_message_unref(reply);
	
	return AC_MANAGE_SUCCESS;
}


int 
ac_manage_show_trap_receiver(DBusConnection *connection, 
                                        STSNMPTrapReceiver **receiver_array,
                                        unsigned int *receiver_num) {
                                            
    if(NULL == connection || NULL == receiver_array || NULL == receiver_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *receiver_array = NULL;
    *receiver_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_RECEIVER);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){
        STSNMPTrapReceiver *temp_array = (STSNMPTrapReceiver *)calloc(temp_num, sizeof(STSNMPTrapReceiver));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {
            char *name = NULL;
            char *sour_ipAddr = NULL;
            char *dest_ipAddr = NULL;
            char *community = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].local_id));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].instance_id));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].version));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &name);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &sour_ipAddr);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &dest_ipAddr);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].dest_port));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &community);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].status));

            strncpy(temp_array[i].name, name, sizeof(temp_array[i].name) - 1);
            strncpy(temp_array[i].sour_ipAddr, sour_ipAddr, sizeof(temp_array[i].sour_ipAddr) - 1);
            strncpy(temp_array[i].dest_ipAddr, dest_ipAddr, sizeof(temp_array[i].dest_ipAddr) - 1);
            strncpy(temp_array[i].trapcom, community, sizeof(temp_array[i].trapcom) - 1);

            dbus_message_iter_next(&iter_array);
        }

        *receiver_array = temp_array;
        *receiver_num = temp_num;
    }
    dbus_message_unref(reply);

    return ret;    
}


int 
ac_manage_show_trap_switch(DBusConnection *connection, 
                                        TRAP_DETAIL_CONFIG **trapDetail_array,
                                        unsigned int *trapDetail_num) {
                                            
    if(NULL == connection || NULL == trapDetail_array || NULL == trapDetail_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *trapDetail_array = NULL;
    *trapDetail_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_SWITCH);

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
	dbus_message_iter_get_basic(&iter, &temp_num);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){
        TRAP_DETAIL_CONFIG *temp_array = (TRAP_DETAIL_CONFIG *)calloc(temp_num, sizeof(TRAP_DETAIL_CONFIG));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *trapName = NULL;
            char *trapEDes = NULL;

            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &trapName);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &trapEDes);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].trapSwitch));
            
            strncpy(temp_array[i].trapName, trapName, sizeof(temp_array[i].trapName) - 1);
            strncpy(temp_array[i].trapEDes, trapEDes, sizeof(temp_array[i].trapEDes) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *trapDetail_array = temp_array;
        *trapDetail_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

int 
ac_manage_show_trap_instance_heartbeat(DBusConnection *connection, 
                                                    TRAPHeartbeatIP **heartbeat_array,
                                                    unsigned int *heartbeat_num) {
                                            
    if(NULL == connection || NULL == heartbeat_array || NULL == heartbeat_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *heartbeat_array = NULL;
    *heartbeat_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_INSTANCE_HEARTBEAT);

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
	dbus_message_iter_get_basic(&iter, &temp_num);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){
        TRAPHeartbeatIP *temp_array = (TRAPHeartbeatIP *)calloc(temp_num, sizeof(TRAPHeartbeatIP));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *ipAddr = NULL;

            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].local_id));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].instance_id));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ipAddr);
            
            strncpy(temp_array[i].ipAddr, ipAddr, sizeof(temp_array[i].ipAddr) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *heartbeat_array = temp_array;
        *heartbeat_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}


int 
ac_manage_show_trap_parameter(DBusConnection *connection, 
                                            TRAPParameter **parameter_array,
                                            unsigned int *parameter_num) {
                                            
    if(NULL == connection || NULL == parameter_array || NULL == parameter_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *parameter_array = NULL;
    *parameter_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TRAP_PARAMETER);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   
    
    if(AC_MANAGE_SUCCESS == ret && temp_num){
        TRAPParameter *temp_array = (TRAPParameter *)calloc(temp_num, sizeof(TRAPParameter));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *paraStr = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &paraStr);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].data));
           
            strncpy(temp_array[i].paraStr, paraStr, sizeof(temp_array[i].paraStr) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *parameter_array = temp_array;
        *parameter_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

int
snmp_cllection_mode(DBusConnection *connection) {
    if(NULL == connection)
        return 0;
    
    int ret = AC_MANAGE_SUCCESS;
	STSNMPSysInfo sysInfo;
    ret = ac_manage_show_snmp_base_info(connection, &sysInfo);
    if(AC_MANAGE_SUCCESS == ret) {
        if(sysInfo.collection_mode)
            return 1;
    }
    return 0;
}


int 
ac_manage_manual_set_mib_acif_stats(DBusConnection *connection, struct mib_acif_stats *acif_node) {

    if(!connection || !acif_node || 0 == strlen(acif_node->ifname))
        return AC_MANAGE_INPUT_TYPE_ERROR;

    char *ifname = strdup(acif_node->ifname);
    if(NULL == ifname) {
        return AC_MANAGE_MALLOC_ERROR;
    }

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_MANUAL_SET_MIB_ACIF_STATS);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &ifname,
                            DBUS_TYPE_UINT32, &acif_node->acIfInNUcastPkts,
                            DBUS_TYPE_UINT32, &acif_node->acIfInDiscardPkts,
                            DBUS_TYPE_UINT32, &acif_node->acIfInErrors,
                            DBUS_TYPE_UINT32, &acif_node->acIfInMulticastPkts,
                            DBUS_TYPE_UINT32, &acif_node->acIfOutDiscardPkts,
                            DBUS_TYPE_UINT32, &acif_node->acIfOutErrors,
                            DBUS_TYPE_UINT32, &acif_node->acIfOutNUcastPkts,
                            DBUS_TYPE_UINT32, &acif_node->acIfOutMulticastPkts,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        MANAGE_FREE(ifname);
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    MANAGE_FREE(ifname);
    return ret;
}


int 
ac_manage_show_mib_acif_stats(DBusConnection *connection, 
                                            struct mib_acif_stats **acif_array,
                                            unsigned int *acif_num) {
                                            
    if(NULL == connection || NULL == acif_array || NULL == acif_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

    *acif_array = NULL;
    *acif_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_MIB_ACIF_STATS);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   
    
    if(AC_MANAGE_SUCCESS == ret && temp_num){
        struct mib_acif_stats *temp_array = (struct mib_acif_stats *)calloc(temp_num, sizeof(struct mib_acif_stats));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            char *ifname = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ifname);
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfInNUcastPkts));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfInDiscardPkts));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfInErrors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfInMulticastPkts));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfOutDiscardPkts));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfOutErrors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfOutNUcastPkts));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].acIfOutMulticastPkts));
            
            strncpy(temp_array[i].ifname, ifname, sizeof(temp_array[i].ifname) - 1);
            
            dbus_message_iter_next(&iter_array);
        }
        
        *acif_array = temp_array;
        *acif_num = temp_num;
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

int ac_manage_web_ip_port_check(DBusConnection *connection, char *address_d, int port_d)
{
    if(NULL == connection)
            return AC_MANAGE_INPUT_TYPE_ERROR;
            
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;
    int ret = AC_MANAGE_SUCCESS;

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
                                        AC_MANAGE_DBUS_OBJPATH,
                                        AC_MANAGE_DBUS_INTERFACE,
                                        AC_MANAGE_DBUS_WEB_IP_PORT_CHECK);

    dbus_error_init(&err);

    LOG("send %s %d", address_d, port_d);
    dbus_message_append_args(query,
                            DBUS_TYPE_STRING, &address_d,
                            DBUS_TYPE_UINT32, &port_d, 
                            DBUS_TYPE_INVALID);
	
    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
    
    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    return ret;

}

int ac_manage_web_edit(DBusConnection *connection, void *data, int edit)
{					
    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
    webHostPtr host = NULL;
    webIfPtr in = NULL;
    char *ifname = NULL;
	int ret = AC_MANAGE_SUCCESS;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_WEB_EDIT);

    dbus_error_init(&err);

    switch(edit)
    {
        case HOST_ADD:
            host = (webHostPtr)data;
            LOG("send %s %s %d %d",host->name, host->address, host->port, host->type);
            dbus_message_append_args(query,
                                    DBUS_TYPE_UINT32, &edit,	
                                    DBUS_TYPE_STRING, &host->address,
                                    DBUS_TYPE_UINT32, &host->port, 
                                    DBUS_TYPE_STRING, &host->name,
                                    DBUS_TYPE_UINT32, &host->type, 
                                    DBUS_TYPE_INVALID);
            break;
        case HOST_DEL:
            ifname= (char *)data;
            LOG("send %s", ifname);
            dbus_message_append_args(query,
                                    DBUS_TYPE_UINT32, &edit,
                                    DBUS_TYPE_STRING, &ifname,
                                    DBUS_TYPE_INVALID);
            break;

        case IFNAME_ADD:
        case IFNAME_DEL:
            in = (webIfPtr)data;
            LOG("send %s %s %d", in->name, in->ifname, in->slot);
            dbus_message_append_args(query,
                                    DBUS_TYPE_UINT32, &edit,	
                                    DBUS_TYPE_STRING, &in->name,
                                    DBUS_TYPE_STRING, &in->ifname,
                                    DBUS_TYPE_UINT32, &in->slot,
                                    DBUS_TYPE_INVALID);
            break;
        default:
            _exit(0);
            break;
    }

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);

    return ret;
}

int ac_manage_web_show(DBusConnection *connection, struct webHostHead *head, unsigned int *stat, unsigned int *flag)
{
    if(NULL == connection)
    {
        return WEB_FAILURE;
    }
    
	DBusMessage *query, *reply;
	DBusMessageIter	iter; 
    DBusMessageIter iter_array, iter_struct; 
    DBusMessageIter iter_sub_array, iter_sub_struct;
    
	DBusError err;  

	webHostPtr vh;
    webIfPtr in;

    unsigned int ref = 0, sum = 0; 
    unsigned int i = 0, j = 0;
    unsigned int tmp1,tmp2;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_WEB_SHOW);
										
    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_INVALID);
                
    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
 
    dbus_message_unref(query);
    
    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return WEB_FAILURE;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &sum);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &tmp1);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &tmp2);

    LOG("got reply %d - sum  = %d", ref++, sum);
    LOG("got reply %d - stat = %d", ref++, tmp1);
    LOG("got reply %d - flag = %d", ref++, tmp2);

    *stat = tmp1;
    *flag = tmp2;
    
    if(sum > 0)
    {
        dbus_message_iter_next(&iter);  
        dbus_message_iter_recurse(&iter, &iter_array);

        while(i++ < sum)
        {
            vh = (webHostPtr)malloc(WEB_WEBHOST_SIZE);
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(vh->name)); 
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(vh->address)); 
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(vh->port)); 
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(vh->type)); 
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(vh->count)); 
            LOG("got reply %d - %s %s %d %d %d", ref++, vh->name, vh->address, vh->port, vh->type, vh->count);

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_recurse(&iter_struct, &iter_sub_array);

            LINK_INIT(&(vh->head));
            j = 0;
            while(j++ < vh->count){
                in = (webIfPtr)malloc(WEB_WEBIFNAME_SIZE);
                dbus_message_iter_recurse(&iter_sub_array, &iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(in->name));
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(in->ifname));
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(in->slot));
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(in->opt));
                LOG("got reply %d - %s %s %d %d",ref++, in->name, in->ifname, in->slot, in->opt);
                in->name = strdup(in->name);
                in->ifname = strdup(in->ifname);
                LINK_INSERT_HEAD(&(vh->head), in, entries);
                dbus_message_iter_next(&iter_sub_array);
            }
            vh->name = strdup(vh->name);
            vh->address = strdup(vh->address);

            LINK_INSERT_HEAD(head, vh, entries);
            dbus_message_iter_next(&iter_array);
        } 
    }
	
    dbus_message_unref(reply);
    
    LOG("---got reply end---");
    return sum;
}

int ac_manage_web_conf(DBusConnection *connection, int operate) 
{

    if(NULL == connection)
    {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}
        
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	
	int ret;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										 AC_MANAGE_DBUS_OBJPATH,
										 AC_MANAGE_DBUS_INTERFACE,
										 AC_MANAGE_DBUS_WEB_CONF);

    dbus_error_init(&err);


	dbus_message_append_args(query,
                                DBUS_TYPE_UINT32, &operate,
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

int ac_manage_web_download(DBusConnection *connection, char **p) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusMessageIter iter;
	DBusError err;
	int ret;
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_WEB_DOWNLOAD);

    dbus_error_init(&err);

    dbus_message_iter_init_append(query, &iter);
	 
    while(strcmp(*p,"end"))
    {
    	syslog(LOG_DEBUG,"%s:%d=> %s\n",__func__, __LINE__,*p);
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &(*p++));
    }

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

    return AC_MANAGE_SUCCESS;
}

int ac_manage_web_show_pages(DBusConnection *connection, char **dir, int *count) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusMessageIter iter;
	DBusError err;
	int i ;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_WEB_SHOW_PAGES);
    dbus_error_init(&err);

    dbus_message_iter_init_append(query, &iter);
	 
    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, count);
	for(i = 0; i < *count; i++)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &(dir[i]));
		dir[i] = strdup(dir[i]);
	}
	
    dbus_message_unref(reply);

    return AC_MANAGE_SUCCESS;
}

int ac_manage_web_del_pages(DBusConnection *connection, const char *dir) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusMessageIter iter;
	DBusError err;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_WEB_DEL_PAGES);

    dbus_error_init(&err);

    dbus_message_iter_init_append(query, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,&dir);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
    dbus_message_unref(reply);

    return AC_MANAGE_SUCCESS;
}

int 
ac_manage_config_ntp_pfm_requestpkts(DBusConnection *connection, 
                                                    char *ifName,
                                                    char *ipstr,
                                                    unsigned int state
                                                    ) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_NTP_DBUS_OBJPATH,
										AC_MANAGE_NTP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_NTP_PFM_REQUESTPKTS);

    dbus_error_init(&err);
    syslog(LOG_DEBUG,"ac_manage_config_apache_pfm_requestpkts,ifname=%s,state=%d,llssls",ifName,state);
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &ifName,
                                DBUS_TYPE_STRING, &ipstr,
                                DBUS_TYPE_UINT32, &state, /*0:delete , 1:add*/
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
ac_manage_config_snmp_sysoid_boardtype(DBusConnection *connection, unsigned int sysoid) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SNMP_SYSOID_BOARDTYPE);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &sysoid,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
    dbus_message_unref(reply);

    return AC_MANAGE_SUCCESS;
}

int 
ac_manage_config_ssylogupload_pfm_requestpkts(DBusConnection *connection, 
                                                    char *ifName,
                                                    char *ipstr,
                                                    unsigned int srcport,
                                                    unsigned int state
                                                    ) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_NTP_DBUS_OBJPATH,
										AC_MANAGE_NTP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_SYSLOGUPLOAD_PFM_REQUESTPKTS);

    dbus_error_init(&err);
    syslog(LOG_DEBUG,"ac_manage_config_apache_pfm_requestpkts,ifname=%s,state=%d,llssls",ifName,state);
    dbus_message_append_args(query,
                                DBUS_TYPE_STRING, &ifName,
                                DBUS_TYPE_STRING, &ipstr,
                                DBUS_TYPE_UINT32, &srcport,
                                DBUS_TYPE_UINT32, &state, /*0:delete , 1:add*/
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
ac_manage_config_mem_status_dog(DBusConnection *connection, unsigned int type, unsigned int maxmem) {

    if(NULL == connection)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_MEM_STATUE_DOG);

    dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
                            DBUS_TYPE_UINT32, &maxmem,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
    dbus_message_unref(reply);

    return AC_MANAGE_SUCCESS;
}

