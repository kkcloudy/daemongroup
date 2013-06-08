#ifndef _WS_DBUS_LIST_H_
#define _WS_DBUS_LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_dbus_def.h"
#include "nm_list.h"

#define snmpd_dbus_list_head (snmpd_dbus_connection_list.connection_list_head)
#define snmpd_local_dbus_node (snmpd_dbus_connection_list.local_dbus_node)
#define dbus_node(p)    ((netsnmp_dbus_connection*)p)
#define list_pos(p)    ((struct list_head *)p)

struct board_info {
    int board_type;     /* /dbm/product/slot/slot%d/is_master */
    int board_active;   /* /dbm/product/slot/slot%d/is_active_master */
    int board_state;    /* /dbm/product/slot/slot%d/board_state */
};

typedef struct netsnmp_dbus_connection_s {
    struct list_head node;
	DBusConnection *connection;
	unsigned int slot_id;
	struct board_info bInfo;
	unsigned int master_instance_count[VRRP_TYPE_NUM];
	unsigned int master_instance[VRRP_TYPE_NUM][INSTANCE_NUM + 1];
	int instance_state[VRRP_TYPE_NUM][INSTANCE_NUM + 1];
	unsigned int connet_times;
} netsnmp_dbus_connection;

typedef struct dbus_connection_list_s {
	struct list_head connection_list_head;
	unsigned int local_slot_num;
	netsnmp_dbus_connection *local_dbus_node;
} dbus_connection_list;


extern DBusConnection *ccgi_dbus_connection;

int get_product_info(char * filename);
void define_distributed_flag(void);
void snmpd_close_dbus_connection(DBusConnection **connection);
DBusConnection *dbus_get_tipc_connection(unsigned int slot_id);
int snmpd_dbus_connection_list_init(void);
void snmpd_vrrp_state_init(void);
void uninit_snmpd_dbus_connection_list(void);
void tipc_dbus_connection_maintenance(void);
void snmpd_had_hansi_advertise(int state);



#endif
