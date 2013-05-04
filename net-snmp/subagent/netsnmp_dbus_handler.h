#ifndef _NETSNMP_DBUS_HANDLER_H_
#define _NETSNMP_DBUS_HANDLER_H_

int netsnmp_dbus_had_master_advertise(DBusMessage * message);

DBusMessage *netsnmp_dbus_interface_show_dbus_connection_list(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *netsnmp_dbus_interface_config_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);


#endif
