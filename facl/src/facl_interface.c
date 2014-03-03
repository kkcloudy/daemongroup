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
#include "facl_interface.h"


int
facl_interface_create_policy(DBusConnection *connection, 
				char *facl_name,
				uint32_t facl_tag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	if (NULL == connection) {
		return NM_ERR_NULL_POINTER;
	}

	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}
	
	if (NULL == connection || NULL == facl_name) {
		return NM_ERR_NULL_POINTER;
	}

	if (strlen(facl_name) == 0 || strlen(facl_name) > FACL_NAME_MAX_LENGTH-1) {
		return FACL_NAME_LEN_ERR;
	}

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_CREATE_POLICY);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					DBUS_TYPE_STRING, &facl_name,
					DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_get_args(reply, &err,
					DBUS_TYPE_INT32, &iRet,
					DBUS_TYPE_INVALID);
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
facl_interface_delete_policy(DBusConnection *connection, 
				uint32_t facl_tag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	
	if (NULL == connection) {
		return NM_ERR_NULL_POINTER;
	}
	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}
#if 0
	if (NULL == connection || NULL == facl_name) {
		return NM_ERR_NULL_POINTER;
	}

	if (strlen(facl_name) == 0 || strlen(facl_name) > FACL_NAME_MAX_LENGTH-1) {
		return FACL_NAME_LEN_ERR;
	}
#endif

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_DELETE_POLICY);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
			        DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_get_args(reply, &err,
					DBUS_TYPE_INT32, &iRet,
					DBUS_TYPE_INVALID);
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}


int
facl_interface_get_policy(DBusConnection *connection, 
				uint32_t facl_tag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	
	if (NULL == connection) {
		return NM_ERR_NULL_POINTER;
	}
	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}
#if 0
	if (NULL == connection || NULL == facl_name) {
		return NM_ERR_NULL_POINTER;
	}

	if (strlen(facl_name) == 0 || strlen(facl_name) > FACL_NAME_MAX_LENGTH-1) {
		return FACL_NAME_LEN_ERR;
	}
#endif
	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_GET_POLICY);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
			        DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_get_args(reply, &err,
					DBUS_TYPE_INT32, &iRet,
					DBUS_TYPE_INVALID);
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
facl_interface_add_rule(DBusConnection *connection,
				uint32_t facl_tag,
				uint32_t id,
				uint32_t type,
				char *inif, 
				char *outif,
				char *srcip, 
				char *dstip,
				int proto,
				char *srcport,
				char *dstport,
				char *domain)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	
	if (NULL == connection) {
		return NM_ERR_NULL_POINTER;
	}
	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}	
	if (FACL_INDEX_MAX_NUM < id) {
		return FACL_INDEX_INPUT_ERR;
	}

	if (NULL == connection || NULL == inif || NULL == outif || NULL == srcip 
		|| NULL == dstip || NULL == srcport || NULL == dstport) {
		return NM_ERR_NULL_POINTER;
	}

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_ADD_RULE);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_UINT32, &id,
					DBUS_TYPE_INT32,  &type,
					DBUS_TYPE_STRING, &inif,
					DBUS_TYPE_STRING, &outif,
					DBUS_TYPE_STRING, &srcip,
					DBUS_TYPE_STRING, &dstip,
					DBUS_TYPE_INT32,  &proto,
					DBUS_TYPE_STRING, &srcport,
					DBUS_TYPE_STRING, &dstport,
					DBUS_TYPE_STRING, &domain,
					DBUS_TYPE_INVALID);

	

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_get_args(reply, &err,
					DBUS_TYPE_INT32, &iRet,
					DBUS_TYPE_INVALID);
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
facl_interface_del_rule(DBusConnection *connection, 
				uint32_t facl_tag,
				uint32_t id)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if (NULL == connection) {
		return NM_ERR_NULL_POINTER;
	}
	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}	
	if (FACL_INDEX_MAX_NUM < id) {
		return FACL_INDEX_INPUT_ERR;
	}

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_DEL_RULE);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_UINT32, &id,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_get_args(reply, &err,
					DBUS_TYPE_INT32, &iRet,
					DBUS_TYPE_INVALID);
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int 
facl_interface_free_rule(struct list_head *rule_head)
{
	facl_rule_t *rule = NULL;
	facl_rule_t *next = NULL;
    
    if (NULL == rule_head) {
        return -1;
    }

    list_for_each_entry_safe(rule, next, rule_head, node) {
        list_del(&(rule->node));
        free(rule);
    }

    return 0;
}

int
facl_interface_free_policy_buf(struct list_head *policy_buf_head)
{
	policy_rule_buf_t *policy_buf = NULL;
	policy_rule_buf_t *next = NULL;
    
    if (NULL == policy_buf_head) {
        return -1;
    }

    list_for_each_entry_safe(policy_buf, next, policy_buf_head, node) {
        list_del(&(policy_buf->node));
        free(policy_buf);
    }

    return 0;
}
int
facl_interface_show_rule(DBusConnection *connection, 
				uint32_t facl_tag,
				struct list_head *rule_head)
{
	DBusMessage *query;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	int iRet = 0;
	uint32_t rule_num = 0;
	facl_rule_t *rule = NULL;
	int i = 0;
	char *domain = NULL;
	char *inif = NULL;
	char *outif = NULL;
	char *srcip = NULL;
	char *dstip = NULL;
	char *srcport = NULL;
	char *dstport = NULL;  

	
	if (NULL == connection || NULL == rule_head) {
		return NM_ERR_NULL_POINTER;
	}
	
	if (FACL_TAG_MAX_NUM < facl_tag) {
		return FACL_TAG_VALUE_ERR;
	}

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_SHOW_RULE);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					DBUS_TYPE_UINT32, &facl_tag,
					DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet );
		
	}

	if (0 != iRet) {
		goto error;
	}
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &rule_num);
	if (0 >= rule_num) {
		goto error;
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter, &iter_array);
	
	for(i = 0; i < rule_num; i++) {
		rule = (facl_rule_t *)malloc(sizeof(facl_rule_t));
		if (rule == NULL) {
			iRet = NM_ERR_MALLOC_FAILED;
			facl_interface_free_rule(rule_head);
			goto error;
		}
		memset(rule, 0, sizeof(facl_rule_t));

		dbus_message_iter_recurse(&iter_array, &iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(rule->index));	
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(rule->data.type));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&domain);
		if (NULL != domain) {
			strncpy(rule->data.domain, domain, sizeof(rule->data.domain)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&inif);
		if (NULL != inif) {
			strncpy(rule->data.inif, inif, sizeof(rule->data.inif)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&outif);
		if (NULL != outif) {
			strncpy(rule->data.outif, outif, sizeof(rule->data.outif)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&srcip);
		if (NULL != srcip) {
			strncpy(rule->data.srcip, srcip, sizeof(rule->data.srcip)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dstip);
		if (NULL != dstip) {
			strncpy(rule->data.dstip, dstip, sizeof(rule->data.dstip)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(rule->data.proto));
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&srcport);
		if (NULL != srcport) {
			strncpy(rule->data.srcport, srcport, sizeof(rule->data.srcport)-1);
		}
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dstport);
		if (NULL != dstport) {
			strncpy(rule->data.dstport, dstport, sizeof(rule->data.dstport)-1);
		}
		dbus_message_iter_next(&iter_array);
		list_add_tail(&rule->node, rule_head);
	}
error:
	dbus_message_unref(reply);
	return iRet;
}
int
facl_interface_show_running(DBusConnection *connection,
							struct list_head *policy_buf_head)
{
	DBusMessage *query;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	int iRet = 0;
	policy_rule_buf_t *policy_buf = NULL;
	uint32_t policy_buf_num = 0;
	char *tmpstr = NULL;
	char *facl_name = NULL;
	int i = 0;

	if (NULL == connection || NULL == policy_buf_head) {
		return NM_ERR_NULL_POINTER;
	}

	query = dbus_message_new_method_call(FACL_DBUS_NAME,
						FACL_DBUS_OBJPATH,
						FACL_DBUS_INTERFACE, 
						FACL_DBUS_METHOD_SHOW_RUNNING);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NM_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet );
		
	}

	if (0 != iRet) {
		goto error;
	}
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &policy_buf_num);
	if (0 >= policy_buf_num) {
		goto error;
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter, &iter_array);

    for(i = 0; i < policy_buf_num; i++) {
        policy_buf = (policy_rule_buf_t *)malloc(sizeof(policy_rule_buf_t));
        if (policy_buf == NULL) {
            iRet = NM_ERR_MALLOC_FAILED;
            facl_interface_free_policy_buf(policy_buf_head);
            goto error;
        }
        memset(policy_buf, 0, sizeof(policy_rule_buf_t));
    
        dbus_message_iter_recurse(&iter_array, &iter_struct);
        dbus_message_iter_get_basic(&iter_struct,&tmpstr);
        if (NULL != tmpstr) {
            strncpy(policy_buf->buf, tmpstr, sizeof(policy_buf->buf)-1);
        }
	dbus_message_iter_next(&iter_struct);
	
        dbus_message_iter_get_basic(&iter_struct,&(policy_buf->facl_tag));
	dbus_message_iter_next(&iter_struct);
	dbus_message_iter_get_basic(&iter_struct,&facl_name);
	if (NULL != facl_name) {
            strncpy(policy_buf->facl_name, facl_name, sizeof(policy_buf->facl_name)-1);
        }
	dbus_message_iter_next(&iter_array);
        list_add_tail(&policy_buf->node, policy_buf_head);
	}
error:
	dbus_message_unref(reply);
	return iRet;
}
