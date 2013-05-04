#ifndef _MAPI_IU_H
#define _MAPI_IU_H


int sigtran_set_self_point_code(int index, int localid, unsigned int my_ip , unsigned short port, unsigned int self_ptcode, DBusConnection *dbus_connection, char *DBUS_METHOD);

int sigtran_set_msc(int index, int localid, unsigned int msc_ip, unsigned short port, unsigned int d_point_code, unsigned int cn_mode, DBusConnection *dbus_connection, char *DBUS_METHOD);

unsigned int sigtran_set_debug_state(int index, int localid, unsigned int debug_type, unsigned int debug_enable, DBusConnection *dbus_connection, char *DBUS_METHOD);

int sigtran_show_running_cfg_lib(int index, int localid, char *showStr, DBusConnection *dbus_connection, char *DBUS_METHOD);

unsigned int set_sigtran2udp_enable(int index, int localid, unsigned int enable, DBusConnection *dbus_connection, char *DBUS_METHOD);

int sigtran_set_cn(int index, int localid, unsigned int cn_ip, unsigned short port, DBusConnection *dbus_connection, char *DBUS_METHOD);

#endif
