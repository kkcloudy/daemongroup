#ifndef _MAPI_IU_H
#define _MAPI_IU_H

unsigned int iu_set_debug_state(int index, int localid, unsigned int debug_type,	unsigned int debug_enable, DBusConnection *dbus_connection,	char *DBUS_METHOD);

unsigned int set_iu_enable(int index, int localid, unsigned int enable, unsigned char isps, DBusConnection *dbus_connection, char *DBUS_METHOD);

unsigned int set_iu2sig_enable(int index, int localid, unsigned int enable, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_traffic_mode(int index, int localid, unsigned int trfmode, unsigned int as_flag, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_routing_context(int index, int localid, unsigned int rtctx, unsigned char as_flag, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_network_indicator(int index, int localid, int ni, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_network_apperance(int index, int localid, int nwapp, unsigned char isps, DBusConnection *dbus_connection, char *DBUS_METHOD);

/* book add, 2011-12-28 */
int iu_set_address(int index, int localid, unsigned int my_ip, unsigned short port, unsigned char is_local, unsigned char is_primary, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_multi_switch(int index, int localid, unsigned char multi_switch, unsigned char is_local, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_connection_mode(int index, int localid, unsigned char conn_mode, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD);

int iu_set_point_code(int index, int localid, unsigned int my_pc, unsigned char is_local, unsigned char is_ps, DBusConnection *dbus_connection, char *DBUS_METHOD);

/* book add, 2012-1-4 */
int get_iu_link_status
(
    int index, 
    int localid,
    unsigned char isps, 
    unsigned char *m3_asp_state, 
    unsigned char *m3_conn_state, 
    int *sctp_state, 
    DBusConnection *dbus_connection, 
    char *DBUS_METHOD
);


#endif
