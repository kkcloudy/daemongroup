#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "nm_list.h"
#include "nm_mem.h"
#include "nm_log.h"
#include "nm_blkmem.h"
#include "nm_dbus.h"
#include "nm_thread.h"
#include "facl_db.h"
#include "facl_errcode.h"
#include "facl_command.h"
#include "facl_ins.h"



DBusMessage *
facl_dbus_method_create_policy(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	const char *facl_name = NULL;
	uint32_t facl_tag = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_create_policy "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_create_policy user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_STRING, &facl_name,
						DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_create_policy unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_create_policy %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	
	ret = facl_add_policy(facldb, facl_name, facl_tag);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
facl_dbus_method_delete_policy(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
    uint32_t facl_tag = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_create_policy "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_create_policy user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
				        DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_delete_policy unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_delete_policy %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	
	ret = facl_del_policy_by_tag(facldb, facl_tag);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
facl_dbus_method_get_policy(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	facl_policy_t *policy = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
    uint32_t facl_tag = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_create_policy "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_create_policy user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_delete_policy unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_delete_policy %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	
	policy = facl_policy_find_by_tag(facldb, facl_tag);
	if (NULL == policy) {
		ret = FACL_POLICY_TAG_NOT_EXIST;
	} else {
		ret = (int) policy->facl_tag;
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
facl_dbus_method_add_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	facl_policy_t *policy = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	struct rule_info input;
	char *inif = NULL, *outif = NULL;
	char *srcip = NULL, *dstip = NULL;
	char *srcport = NULL, *dstport = NULL;
	char *domain = NULL;
	uint32_t facl_tag = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_add_rule "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_add_rule user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_UINT32, &(input.id),
						DBUS_TYPE_INT32,  &(input.type),
						DBUS_TYPE_STRING, &inif,
						DBUS_TYPE_STRING, &outif,
						DBUS_TYPE_STRING, &srcip,
						DBUS_TYPE_STRING, &dstip,
						DBUS_TYPE_INT32,  &(input.proto),
						DBUS_TYPE_STRING, &srcport,
						DBUS_TYPE_STRING, &dstport,
						DBUS_TYPE_STRING, &domain,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_add_rule unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_add_rule %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	strncpy(input.inif, inif, sizeof(input.inif));
	strncpy(input.outif, outif, sizeof(input.outif));
	strncpy(input.srcip, srcip, sizeof(input.srcip));
	strncpy(input.dstip, dstip, sizeof(input.dstip));
	strncpy(input.srcport, srcport, sizeof(input.srcport));
	strncpy(input.dstport, dstport, sizeof(input.dstport));
	strncpy(input.domain, domain, sizeof(input.domain));
	policy = facl_policy_find_by_tag(facldb, facl_tag);
	ret = facl_add_rule(policy, &input);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
facl_dbus_method_del_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	facl_policy_t *policy = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	uint32_t facl_tag = 0;
	uint32_t id = 0;
	int i = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_del_rule "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_del_rule user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_UINT32, &id,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_del_rule unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_del_rule %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	policy = facl_policy_find_by_tag(facldb, facl_tag);	
	do {
		ret = facl_del_rule(policy, id);
		i++;
	} while (NM_RETURN_OK == ret);
	
	if (NM_ERR_NULL_POINTER == ret && i > 1) {
		ret = NM_RETURN_OK;
	} 

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
facl_dbus_method_show_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	facl_db_t *facldb = NULL;
	facl_policy_t *policy = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	uint32_t rule_num = 0;
	uint32_t facl_tag = 0;
	facl_rule_t *rule = NULL;
	char *domain = NULL;
	char *inif = NULL;
	char *outif = NULL;
	char *srcip = NULL;
	char *dstip = NULL;
	char *srcport = NULL;
	char *dstport = NULL;  


	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_show_rule "
			"DBUS new reply message error");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_show_rule user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
						DBUS_TYPE_UINT32, &facl_tag,
						DBUS_TYPE_INVALID)))
	{
		nm_log_err("facl_dbus_method_show_rule unable to get input args");
		if (dbus_error_is_set(&err)) {
			nm_log_err("facl_dbus_method_show_rule %s raised:%s",
						err.name, err.message);
			dbus_error_free(&err);
		}
		ret = NM_ERR_DBUS_FAILED;
		goto replyx;
	}
	policy = facl_policy_find_by_tag(facldb, facl_tag);
	if (NULL == policy) {
		ret = FACL_POLICY_TAG_NOT_EXIST;
		goto replyx;
	}
	
	dbus_error_init(&err);
    ret = NM_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	if (NM_RETURN_OK == ret) {
		rule_num = policy->rule_num;
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &rule_num);
	}
	if (NM_RETURN_OK == ret && rule_num > 0) {
		DBusMessageIter iter_array = {0};

		dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING
								DBUS_TYPE_UINT32_AS_STRING
								DBUS_TYPE_INT32_AS_STRING		
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_INT32_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_TYPE_STRING_AS_STRING
								DBUS_STRUCT_END_CHAR_AS_STRING,
								&iter_array);

		list_for_each_entry(rule, &(policy->rule_head), node) {
			DBusMessageIter iter_struct = {0};
			dbus_message_iter_open_container (&iter_array,
								DBUS_TYPE_STRUCT,
								NULL,
								&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
								DBUS_TYPE_UINT32,
								&(rule->index));
			dbus_message_iter_append_basic(&iter_struct,
								DBUS_TYPE_INT32, 
								&(rule->data.type));
			domain = rule->data.domain;
			dbus_message_iter_append_basic(&iter_struct,
								DBUS_TYPE_STRING, 
								&domain);
			inif = rule->data.inif;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&inif);
			outif = rule->data.outif;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&outif);
			srcip = rule->data.srcip;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&srcip);
			dstip = rule->data.dstip;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&dstip);
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_INT32,
								&(rule->data.proto));
			srcport = rule->data.srcport;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&srcport);
			dstport = rule->data.dstport;
			dbus_message_iter_append_basic (&iter_struct,
								DBUS_TYPE_STRING,
								&dstport);
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	return reply;
}
DBusMessage *
facl_dbus_method_show_running(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	facl_db_t *facldb = NULL;
	facl_policy_t *policy = NULL; 
	facl_rule_t *rule = NULL;
	char *show_str = NULL; 
	char *policy_name = NULL;
	int show_str_size = FACL_RULE_INFO_BUF_SIZE*FACL_INDEX_MAX_NUM;
	int policy_buf_num = 0;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		nm_log_err("facl_dbus_method_create_policy "
			"DBUS new reply message error");
		return NULL;
	}
	
	show_str = malloc(show_str_size);
	if (NULL == show_str) {
		nm_log_err("facl_dbus_method_create_policy malloc failed");
		return NULL;
	}

	facldb = (facl_db_t *)user_data;
	if (NULL == facldb) {
		nm_log_err("facl_dbus_method_create_policy user_data error");
		ret = NM_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
    ret = NM_RETURN_OK;

replyx:
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                    DBUS_TYPE_INT32, &ret);
    if (NM_RETURN_OK == ret) {
        policy_buf_num = facldb->policy_num;
        dbus_message_iter_append_basic(&iter,
                                    DBUS_TYPE_UINT32, &policy_buf_num);
    }
    if (NM_RETURN_OK == ret && policy_buf_num > 0) {
            DBusMessageIter iter_array = {0};
    
            dbus_message_iter_open_container (&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    DBUS_TYPE_STRING_AS_STRING
                                    DBUS_TYPE_UINT32_AS_STRING
                                    DBUS_TYPE_STRING_AS_STRING
                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
    
		list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
			
            	memset(show_str, 0, show_str_size);
            	facl_policy_show_running(policy, show_str, show_str_size);
                DBusMessageIter iter_struct = {0};
    			
                dbus_message_iter_open_container (&iter_array,
                                    DBUS_TYPE_STRUCT,
                                    NULL,
                                    &iter_struct);
                dbus_message_iter_append_basic (&iter_struct,
                                    DBUS_TYPE_STRING,
                                    &show_str);
				dbus_message_iter_append_basic (&iter_struct,
		                                    	DBUS_TYPE_UINT32,
		                                    	&(policy->facl_tag));
				policy_name = policy->facl_name;
				dbus_message_iter_append_basic (&iter_struct,
		                                    	DBUS_TYPE_STRING,
		                                    	&policy_name);
                dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	free(show_str);
	
	return reply;
}

void
faclins_register_all_dbus_method(facl_ins_t *faclins)
{
	nm_dbus_t *dbus = facl_ins_get_dbus(faclins);
	facl_db_t *facldb = facl_ins_get_db(faclins);

	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_create_policy, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_delete_policy, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_get_policy, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_add_rule, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_del_rule, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_show_rule, facldb);
	nm_dbus_register_method(dbus, FACL_DBUS_INTERFACE, 
			facl_dbus_method_show_running, facldb);
	return;
}

