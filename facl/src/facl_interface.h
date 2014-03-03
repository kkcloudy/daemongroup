#ifndef _FACL_INTERFACE_H
#define _FACL_INTERFACE_H

#include <stdint.h>
#include "facl_command.h"


int
facl_interface_create_policy(DBusConnection *connection, 
				char *facl_name,
				uint32_t facl_tag);

int
facl_interface_delete_policy(DBusConnection *connection, 
				uint32_t facl_tag);

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
				char *domian);

int
facl_interface_del_rule(DBusConnection *connection, 
				uint32_t facl_tag,
				uint32_t id);

int 
facl_interface_free_rule(struct list_head *rule_head);

int
facl_interface_show_running(DBusConnection *connection,
							struct list_head *policy_buf_head);

int
facl_interface_free_policy_buf(struct list_head *policy_buf_head);

#endif
