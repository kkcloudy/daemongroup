
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_dbus_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_dbus.h"


struct dbus_method {
	void *para;
	dbus_method_func func;
	struct list_head next;
	char member[DBUSMEMBER_LEN];	
};

struct dbus_filter {
	void *para;
	dbus_filter_func func;
	struct list_head next;
	char member[DBUSMEMBER_LEN];	
};

struct dbus_struct {
	uint32 local_id, instance_id;
	DBusConnection *dbus_connection;
	DBusConnection *tipc_connection[SLOT_MAX_NUM + 1];
	struct list_head methods;
	struct list_head filters;
};

static struct dbus_struct pppoe_dbus;

static DBusMessage *
dbus_config_log_debug(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	 iter;
	int ret = PPPOEERR_SUCCESS;
	uint32 type, state;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &type,
					DBUS_TYPE_UINT32, &state,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (!type) {
		pppoe_debug_config(state);
	} else {
		/* config kernel debug */
	}
	
	reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusMessage *
dbus_config_log_token(DBusMessage *message, void *user_data) {
	DBusMessage *reply;	
	DBusError err;
	DBusMessageIter	 iter;
	PPPOELogToken token;
	uint32 state;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
					DBUS_TYPE_UINT32, &token,
					DBUS_TYPE_UINT32, &state,
					DBUS_TYPE_INVALID))){
		pppoe_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			pppoe_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (state){
		ret = pppoe_token_register(token);
	} else {
		ret = pppoe_token_unregister(token);
	}

	reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}

static DBusHandlerResult 
dbus_message_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {
	struct list_head *pos;
	DBusMessage	*reply;

	pppoe_log(LOG_DEBUG, "message type = %s, message path = %s, message member = %s\n",
						dbus_message_type_to_string(dbus_message_get_type(message)),
						dbus_message_get_path(message), dbus_message_get_member(message));

	if (0 == strcmp(dbus_message_get_path(message), PPPOE_DBUS_OBJPATH)) {
		list_for_each(pos, &pppoe_dbus.methods) {
			struct dbus_method *method 
				= list_entry(pos, struct dbus_method, next);
            if (dbus_message_is_method_call(message, PPPOE_DBUS_INTERFACE, method->member)) {
				reply = method->func(message, method->para);
                goto out;
            }
		}
	}

	pppoe_log(LOG_DEBUG, "message type = %s, message path = %s, message member = %s not handled\n",
						dbus_message_type_to_string(dbus_message_get_type(message)),
						dbus_message_get_path(message), dbus_message_get_member(message));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

out:	
	if (reply) {
		pppoe_log(LOG_DEBUG, "reply destination %s\n", dbus_message_get_destination(reply));
		dbus_connection_send(connection, reply, NULL);
		dbus_connection_flush(connection); 
		dbus_message_unref(reply);
	}
	
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
dbus_filter_function(DBusConnection *connection, DBusMessage *message, void *user_data) {
	struct list_head *pos;
	const char *member;
	int ret, type;

	if (DBUS_MESSAGE_TYPE_SIGNAL != (type = dbus_message_get_type(message)))
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	pppoe_log(LOG_DEBUG, "signal type = %s, signal path = %s, "
						"signal interface = %s, signal member = %s\n",
						dbus_message_type_to_string(type),
						dbus_message_get_path(message),
						dbus_message_get_interface(message),
						dbus_message_get_member(message));


	if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected") 
		&& strcmp(dbus_message_get_path(message), DBUS_PATH_LOCAL) == 0) {
		dbus_connection_unref(pppoe_dbus.dbus_connection);
		pppoe_dbus.dbus_connection = NULL;
	} 

	member = dbus_message_get_member(message);
	list_for_each(pos, &pppoe_dbus.filters) {
		struct dbus_filter *filter 
			= list_entry(pos, struct dbus_filter, next);
		if (!strcmp(member, filter->member)) {
			ret = filter->func(message, filter->para);
			pppoe_log(LOG_DEBUG, "filter %s, ret %d\n", filter->member, ret);
			return DBUS_HANDLER_RESULT_HANDLED;
		}
	}

	pppoe_log(LOG_DEBUG, "signal type = %s, signal path = %s, "
						"signal member = %s not handled\n",
						dbus_message_type_to_string(type),
						dbus_message_get_path(message), 
						dbus_message_get_member(message));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


int
pppoe_dbus_method_register(const char *member,
							dbus_method_func func, void *para) {
	struct dbus_method *method;
	struct list_head *pos;

	if (unlikely(!func || !member
		|| strlen(member) > (DBUSMEMBER_LEN - 1))) {
		pppoe_log(LOG_WARNING, "dbus method register input error\n");
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &pppoe_dbus.methods) {
		method = list_entry(pos, struct dbus_method, next);
		if (!strcmp(method->member, member)) {
			pppoe_log(LOG_WARNING, "dbus method %s is already exist\n",
									member);
			return PPPOEERR_EEXIST;
		}
	}

	method = (struct dbus_method *)malloc(sizeof(struct dbus_method));
	if (unlikely(!method)) {
		pppoe_log(LOG_WARNING, "dbus method malloc failed\n");
		return PPPOEERR_ENOMEM;
	}
	
	memset(method, 0, sizeof(*method));
	strncpy(method->member, member, sizeof(method->member) - 1);
	method->func = func;
	method->para = para;

	list_add(&method->next, &pppoe_dbus.methods);
	return PPPOEERR_SUCCESS;
}

int
pppoe_dbus_method_unregister(const char *member) {
	struct dbus_method *method;
	struct list_head *pos;

	if (unlikely(!member || strlen(member) > (DBUSMEMBER_LEN - 1))) {
		pppoe_log(LOG_WARNING, "dbus method unregister input error\n");
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &pppoe_dbus.methods) {
		method = list_entry(pos, struct dbus_method, next);
		if (!strcmp(method->member, member)) {
			goto out;
		}
	}
	
	pppoe_log(LOG_WARNING, "dbus method %s is not exist\n", member);
	return PPPOEERR_ENOEXIST;

out:
	list_del(&method->next);
	PPPOE_FREE(method);
	return PPPOEERR_SUCCESS;
}

int
pppoe_dbus_filter_register(const char *member,
						dbus_filter_func func, void *para) {
	struct dbus_filter *filter;
	struct list_head *pos;

	if (unlikely(!func || !member
		|| strlen(member) > (DBUSMEMBER_LEN - 1))) {
		pppoe_log(LOG_WARNING, "dbus filter register input error\n");
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &pppoe_dbus.methods) {
		filter = list_entry(pos, struct dbus_filter, next);
		if (!strcmp(filter->member, member)) {
			pppoe_log(LOG_WARNING, "dbus filter %s is already exist\n",
									member);
			return PPPOEERR_EEXIST;
		}
	}

	filter = (struct dbus_filter *)malloc(sizeof(struct dbus_filter));
	if (unlikely(!filter)) {
		pppoe_log(LOG_WARNING, "dbus filter malloc failed\n");
		return PPPOEERR_ENOMEM;
	}
	
	memset(filter, 0, sizeof(*filter));
	strncpy(filter->member, member, sizeof(filter->member) - 1);
	filter->func = func;
	filter->para = para;

	list_add(&filter->next, &pppoe_dbus.filters);
	return PPPOEERR_SUCCESS;
}

int
pppoe_dbus_filter_unregister(const char *member) {
	struct dbus_filter *filter;
	struct list_head *pos;

	if (unlikely(!member 
		|| strlen(member) > (DBUSMEMBER_LEN - 1))) {
		pppoe_log(LOG_WARNING, "dbus filter unregister input error\n");
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &pppoe_dbus.methods) {
		filter = list_entry(pos, struct dbus_filter, next);
		if (!strcmp(filter->member, member)) {
			goto out;
		}
	}
	
	pppoe_log(LOG_WARNING, "dbus filter %s is not exist\n",
							member);
	return PPPOEERR_ENOEXIST;

out:
	list_del(&filter->next);
	PPPOE_FREE(filter);
	return PPPOEERR_SUCCESS;
}


static inline DBusConnection *
dbus_get_tipc_connection(uint32 slot_id) {  
	DBusConnection *connection;
	DBusError err;	

	if (unlikely(!slot_id || slot_id > SLOT_MAX_NUM)) 
		return NULL;
	
	dbus_error_init(&err);
	connection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &err);
	if (!connection) {
		if (dbus_error_is_set(&err)){
			pppoe_log(LOG_WARNING, "dbus_get_tipc_connection: "
								"%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		addresses_shutdown_func(NULL);
		pppoe_log(LOG_WARNING, "dbus_get_tipc_connection: "
							"slot %u remote dbus is not ready\n", slot_id);
		return NULL;
	}

	pppoe_log(LOG_DEBUG, "slot %u tipc dbus connection is OK\n", slot_id);
	return connection;
}

DBusConnection *
pppoe_dbus_get_local_connection(void) {
	return pppoe_dbus.dbus_connection;
}

DBusConnection *
pppoe_dbus_get_slot_connection(uint32 slot_id) {
	if (unlikely(!slot_id || slot_id > SLOT_MAX_NUM)) 
		return NULL;

	if (!pppoe_dbus.tipc_connection[slot_id]) {
		pppoe_dbus.tipc_connection[slot_id] = dbus_get_tipc_connection(slot_id);
	}

	return pppoe_dbus.tipc_connection[slot_id];
}


int
pppoe_dbus_init(uint32 local_id, uint32 instance_id) {
	DBusError dbus_error;
	DBusObjectPathVTable pppoe_vtable = {NULL, &dbus_message_handler, NULL, NULL, NULL, NULL};	
	char dbusName[DBUSNAME_LEN];
	int ret;

	memset(&pppoe_dbus, 0, sizeof(pppoe_dbus));
	pppoe_dbus.local_id = local_id;
	pppoe_dbus.instance_id = instance_id;
	INIT_LIST_HEAD(&pppoe_dbus.methods);
	INIT_LIST_HEAD(&pppoe_dbus.filters);
	
	memset(dbusName, 0, sizeof(dbusName));
	snprintf(dbusName, sizeof(dbusName), "%s%d_%d", PPPOE_DBUS_DBUSNAME, local_id, instance_id);

	dbus_error_init(&dbus_error);
	pppoe_dbus.dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
	if (!pppoe_dbus.dbus_connection) {
		pppoe_log(LOG_ERR, "pppoe dbus init fail, %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		ret = PPPOEERR_EDBUS;
		goto error;
	}
	
	if (!dbus_connection_register_fallback(pppoe_dbus.dbus_connection, PPPOE_DBUS_OBJPATH, &pppoe_vtable, NULL)) {
		pppoe_log(LOG_ERR, "pppoe dbus register fallback fail, %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		ret = PPPOEERR_EDBUS;
		goto error;
	}
	
	dbus_bus_request_name(pppoe_dbus.dbus_connection, dbusName, 0, &dbus_error);
	if (dbus_error_is_set(&dbus_error)) {
		pppoe_log(LOG_ERR, "pppoe dbus get name fail, %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		ret = PPPOEERR_EDBUS;
		goto error;
	}

	dbus_connection_add_filter(pppoe_dbus.dbus_connection, dbus_filter_function, NULL, NULL);
	
	dbus_bus_add_match(pppoe_dbus.dbus_connection,
						"type='signal'"
						",interface='"PPPOE_DBUS_INTERFACE"'",
						NULL);

	/* register debug dbus method */
	ret = pppoe_dbus_method_register(PPPOE_DBUS_CONFIG_LOG_DEBUG, 
									dbus_config_log_debug, NULL);
	if (unlikely(ret))
		goto error;

	ret = pppoe_dbus_method_register(PPPOE_DBUS_CONFIG_LOG_TOKEN, 
									dbus_config_log_token, NULL);
	if (unlikely(ret))
		goto error1;
	
	pppoe_log(LOG_INFO, "pppoe init dbus %s success\n", dbusName);
	return PPPOEERR_SUCCESS;  

error1:
	pppoe_dbus_method_unregister(PPPOE_DBUS_CONFIG_LOG_DEBUG);
error:
	return ret;
}

void
pppoe_dbus_exit(void) {
	int i;

	if (pppoe_dbus.dbus_connection) {
		dbus_connection_close(pppoe_dbus.dbus_connection);
		pppoe_dbus.dbus_connection = NULL;
	}
	
	for (i = 0; i < ARRAY_SIZE(pppoe_dbus.tipc_connection); i++) {
		if (pppoe_dbus.tipc_connection[i]) { 
			dbus_connection_close(pppoe_dbus.tipc_connection[i]);
			pppoe_dbus.tipc_connection[i] = NULL;
		}	
	}
}

void
pppoe_dbus_dispatch(uint32 msec) {
	dbus_connection_read_write_dispatch(pppoe_dbus.dbus_connection, msec);
}

