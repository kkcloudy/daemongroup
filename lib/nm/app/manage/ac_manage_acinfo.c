#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dbus/dbus.h>

#include "ws_log_conf.h"
#include "ac_manage_def.h"
#include "ac_manage_acinfo.h"
#include "manage_log.h"
#include "ws_acinfo.h"


int
ac_manage_set_acinfo_rule(DBusConnection *connection,char *status,char *key,int opt_type) {
	if(NULL == connection || NULL == status) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_ACINFO_DBUS_OBJPATH,
											AC_MANAGE_ACINFO_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_ACINFO_VALUE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &status,
							DBUS_TYPE_STRING, &key,
							DBUS_TYPE_INT32,&opt_type,
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
ac_manage_set_bkacinfo_rule(DBusConnection *connection,char *status,struct bkacinfo_st *rule,int opt_type) {
	if(NULL == connection || NULL == status || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	char *key = rule->key? rule->key : "";
	char *insid = rule->insid? rule->insid : "";
	char *netip = rule->netip? rule->netip : "";

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_ACINFO_DBUS_OBJPATH,
											AC_MANAGE_ACINFO_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_BKACINFO_VALUE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &status,							
							DBUS_TYPE_STRING, &key,
							DBUS_TYPE_STRING, &insid,
							DBUS_TYPE_STRING, &netip,
							DBUS_TYPE_INT32,&opt_type,
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
ac_manage_delete_system_version_file(DBusConnection *connection,char *version_file) {		
	if(NULL == connection || NULL == version_file) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}	

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_ACINFO_DBUS_OBJPATH,
											AC_MANAGE_ACINFO_DBUS_INTERFACE,
											AC_MANAGE_DBUS_DEL_AC_VERSION_FILE_VALUE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &version_file,
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


