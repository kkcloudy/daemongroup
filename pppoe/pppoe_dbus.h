#ifndef _PPPOE_DBUS_H
#define _PPPOE_DBUS_H

typedef int (*dbus_filter_func)(DBusMessage *message, void *user_data);
typedef DBusMessage *(*dbus_method_func)(DBusMessage *message, void *user_data);


int pppoe_dbus_method_register(const char *member,
							dbus_method_func func, void *para);
int pppoe_dbus_method_unregister(const char *method_name);

int pppoe_dbus_filter_register(const char *member,
							dbus_filter_func func, void *para);
int pppoe_dbus_filter_unregister(const char *filter_name);

DBusConnection *pppoe_dbus_get_local_connection(void);
DBusConnection *pppoe_dbus_get_slot_connection(uint32 slot_id);

int pppoe_dbus_init(uint32 hansi_type, uint32 hansi_id);
void pppoe_dbus_exit(void);
void pppoe_dbus_dispatch(uint32 msec);

#endif
