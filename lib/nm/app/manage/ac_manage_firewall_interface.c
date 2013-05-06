
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dbus/dbus.h>

#include "ws_firewall.h"

#include "ac_manage_def.h"

#include "ac_manage_firewall_interface.h"

static void
firewall_rule_copy(fwRule *new_rule, fwRule *rule) {
	if(NULL == new_rule || NULL == rule) {
		return ;
	}

	memset(new_rule, 0, sizeof(fwRule));
	
	new_rule->type 		= rule->type;
	new_rule->id			= rule->id;
	new_rule->ordernum 	= rule->ordernum;	
	new_rule->enable 		= rule->enable;
	new_rule->status 		= rule->status;
	MANAGE_STRDUP(new_rule->name, rule->name);
	MANAGE_STRDUP(new_rule->comment, rule->comment);

	new_rule->srctype 	= rule->srctype;
	new_rule->dsttype 	= rule->dsttype;
	MANAGE_STRDUP(new_rule->ineth, rule->ineth);
	MANAGE_STRDUP(new_rule->outeth, rule->outeth);
	MANAGE_STRDUP(new_rule->srcadd, rule->srcadd);
	MANAGE_STRDUP(new_rule->dstadd, rule->dstadd);
	
	new_rule->protocl 		= rule->protocl;
	new_rule->sptype 		= rule->sptype;
	new_rule->dptype 		= rule->dptype;
	MANAGE_STRDUP(new_rule->sport, rule->sport);
	MANAGE_STRDUP(new_rule->dport, rule->dport);

	MANAGE_STRDUP(new_rule->connlimit, rule->connlimit);
	
	new_rule->act 		= rule->act;
	MANAGE_STRDUP(new_rule->tcpmss_var, rule->tcpmss_var);
	
	MANAGE_STRDUP(new_rule->pkg_state, rule->pkg_state);
	MANAGE_STRDUP(new_rule->string_filter, rule->string_filter);
	
	new_rule->natiptype 	= rule->natiptype;
	new_rule->natpttype 	= rule->natpttype;
	MANAGE_STRDUP(new_rule->natipadd, rule->natipadd);
	MANAGE_STRDUP(new_rule->natport, rule->natport);

	return ;
}

int
ac_manage_config_firewall_service(DBusConnection *connection, u_long status) {
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	dbus_error_init(&err);
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_FIREWALL_SERVICE);
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
ac_manage_config_firewall_rule(DBusConnection *connection, fwRule *rule, u_long config_type) {
	if(NULL == connection || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	char *ineth = rule->ineth ? : "";
	char *outeth = rule->outeth ? : "";
	char *srcadd = rule->srcadd ? : "";
	char *dstadd = rule->dstadd ? : "";
	char *sport = rule->sport ? : "";
	char *dport = rule->dport ? : "";
	char *connlimit = rule->connlimit ? : "";
	char *tcpmss_var = rule->tcpmss_var ? : "";
	char *pkg_state = rule->pkg_state ? : "";
	char *string_filter = rule->string_filter ? : "";
	char *natipadd = rule->natipadd ? : "";
	char *natport = rule->natport ? : "";
	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_FIREWALL_RULE);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &config_type,
							
							DBUS_TYPE_UINT32, &(rule->type),
							DBUS_TYPE_UINT32, &(rule->id),
							DBUS_TYPE_UINT32, &(rule->enable),

							DBUS_TYPE_STRING, &ineth,
							DBUS_TYPE_STRING, &outeth,
							DBUS_TYPE_UINT32, &(rule->srctype),
							DBUS_TYPE_STRING, &srcadd,
							DBUS_TYPE_UINT32, &(rule->dsttype),
							DBUS_TYPE_STRING, &dstadd,

							DBUS_TYPE_UINT32, &(rule->protocl),
							DBUS_TYPE_UINT32, &(rule->sptype),
							DBUS_TYPE_STRING, &sport,
							DBUS_TYPE_UINT32, &(rule->dptype),
							DBUS_TYPE_STRING, &dport,

							DBUS_TYPE_STRING, &connlimit,

							DBUS_TYPE_UINT32, &(rule->act),
							DBUS_TYPE_STRING, &tcpmss_var,

							DBUS_TYPE_STRING, &pkg_state,
							DBUS_TYPE_STRING, &string_filter,

							DBUS_TYPE_UINT32, &(rule->natiptype),
							DBUS_TYPE_STRING, &natipadd,
							DBUS_TYPE_UINT32, &(rule->natpttype),
							DBUS_TYPE_STRING, &natport,
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
ac_manage_change_firewall_index(DBusConnection *connection, u_long new_index, u_long rule_type, u_long index) {
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CHANGE_FIREWALL_INDEX);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &rule_type,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_UINT32, &new_index,
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
ac_manage_del_firewall_rule(DBusConnection *connection, u_long rule_type, u_long index) {
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_DELETE_FIREWALL_RULE);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &rule_type,
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
ac_manage_config_nat_udp_timeout(DBusConnection *connection, u_long timeout) {
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	int ret;

	if (!connection)
		return AC_MANAGE_INPUT_TYPE_ERROR;

	if (timeout < NAT_UDP_TIMEOUT_MIN || timeout > NAT_UDP_TIMEOUT_MAX)
		return AC_MANAGE_INPUT_TYPE_ERROR;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_NAT_UDP_TIMEOUT);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &timeout,
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
ac_manage_show_firewall_rule(DBusConnection *connection, 
							u_long *service_status, u_long *timeout,
							fwRule **rule_array, u_long *rule_num) {

	if(NULL == connection || NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*rule_array = NULL;
	*rule_num = 0;

	fwRule *temp_array = NULL;
	u_long temp_num = 0;
    
	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_FIREWALL_DBUS_OBJPATH,
											AC_MANAGE_FIREWALL_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_FIREWALL_RULE);

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
	if(service_status) {
		dbus_message_iter_get_basic(&iter, service_status);
	}	

	dbus_message_iter_next(&iter);  
	if (timeout) {
		dbus_message_iter_get_basic(&iter, timeout);
	}

	dbus_message_iter_next(&iter);  
	dbus_message_iter_get_basic(&iter, &temp_num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   

	if(AC_MANAGE_SUCCESS == ret && temp_num) {
		temp_array = (fwRule *)calloc(temp_num, sizeof(fwRule));
		if(NULL == temp_array) {
			dbus_message_unref(reply);
			return AC_MANAGE_MALLOC_ERROR;
		}

		int i = 0;
		for(i = 0; i < temp_num; i++) {

	            fwRule tempRule = { 0 };
	                
	            dbus_message_iter_recurse(&iter_array, &iter_struct);
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.type);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.id);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.enable);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.ineth);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.outeth);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.srctype);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.srcadd);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.dsttype);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.dstadd);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.protocl);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.sptype);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.sport);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.dptype);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.dport);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.connlimit);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.act);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.tcpmss_var);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.pkg_state);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.string_filter);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.natiptype);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.natipadd);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.natpttype);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &tempRule.natport);

	            firewall_rule_copy(&temp_array[i], &tempRule);
					
	            dbus_message_iter_next(&iter_array);
		}
        
		*rule_array = temp_array;
		*rule_num = temp_num;
	}

	dbus_message_unref(reply);

	return ret;
}

int
ac_manage_config_strict_access_level(DBusConnection *connection, int level)
{
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	dbus_error_init(&err);
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_FIREWALL_DBUS_OBJPATH,
										AC_MANAGE_FIREWALL_DBUS_INTERFACE,
										AC_MANAGE_DBUS_CONFIG_STRICT_ACCESS_LEVEL);
	dbus_message_append_args(query,
							DBUS_TYPE_INT32, &level,
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
ac_manage_show_strict_access(DBusConnection *connection, int *level)
{

	if(NULL == connection || NULL == level) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	int ret = AC_MANAGE_DBUS_ERROR;
	int temp_level = 0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;	
	
	*level = 0;
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_FIREWALL_DBUS_OBJPATH,
											AC_MANAGE_FIREWALL_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_STRICT_ACCESS);

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

	if (AC_MANAGE_SUCCESS == ret) {
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &temp_level);
	}

	dbus_message_unref(reply);
	*level = temp_level;
	
	return ret;
}

