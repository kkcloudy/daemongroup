/* drp_handle.h */

#ifndef _DRP_HANDLE_H
#define _DRP_HANDLE_H

DBusMessage *
drp_dbus_method_add_domain( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );

DBusMessage *
drp_dbus_method_del_domain( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );

DBusMessage *
drp_dbus_method_get_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );

DBusMessage *
drp_dbus_method_add_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );

DBusMessage *
drp_dbus_method_del_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );

DBusMessage *
drp_dbus_method_show_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data );



#endif
