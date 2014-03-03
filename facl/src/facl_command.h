#ifndef _facl_COMMAND_H_
#define _facl_COMMAND_H_

#include "nm_dbus.h"
#include "facl_ins.h"

#define FACL_DBUS_NAME		"aw.nm_facl"
#define FACL_DBUS_OBJPATH	"/aw/nm_facl"
#define FACL_DBUS_INTERFACE	"aw.nm_facl"

#define FACL_DBUS_METHOD_CREATE_POLICY			"facl_dbus_method_create_policy"
#define FACL_DBUS_METHOD_DELETE_POLICY			"facl_dbus_method_delete_policy"
#define FACL_DBUS_METHOD_GET_POLICY			"facl_dbus_method_get_policy"
#define FACL_DBUS_METHOD_ADD_RULE			"facl_dbus_method_add_rule"
#define FACL_DBUS_METHOD_DEL_RULE			"facl_dbus_method_del_rule"
#define FACL_DBUS_METHOD_SHOW_RULE			"facl_dbus_method_show_rule"
#define FACL_DBUS_METHOD_SHOW_RUNNING			"facl_dbus_method_show_running"

DBusMessage *
facl_dbus_method_create_policy(DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);

DBusMessage *
facl_dbus_method_delete_policy(DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);

DBusMessage *
facl_dbus_method_add_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);

DBusMessage *
facl_dbus_method_del_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);
				
DBusMessage *
facl_dbus_method_show_rule(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);
				
DBusMessage *
facl_dbus_method_show_running(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data);

void
faclins_register_all_dbus_method(facl_ins_t *faclins);


#endif
