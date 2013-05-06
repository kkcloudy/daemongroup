#ifndef _WS_SNMPD_DBUS_INTERFACE_H_
#define _WS_SNMPD_DBUS_INTERFACE_H_

#include <dbus/dbus.h>
#include "ws_dbus_def.h"

struct snmpdInstanceInfo {
    unsigned int slot_id;
    unsigned int dbus_connection_state;
    unsigned int master_count;
    unsigned int instance_state[VRRP_TYPE_NUM][INSTANCE_NUM + 1];

    struct snmpdInstanceInfo *next;
};

void free_snmpd_dbus_connection_list(struct snmpdInstanceInfo **snmpdHead);

int show_snmpd_dbus_connection_list(DBusConnection *connection, struct snmpdInstanceInfo **snmpdHead, unsigned int *slot_count);

int dbus_get_trap_instance_states(DBusConnection *connection, unsigned int *master_instance);

int dbus_config_snmp_log_debug(DBusConnection *connection, unsigned int debugLevel);


#endif

