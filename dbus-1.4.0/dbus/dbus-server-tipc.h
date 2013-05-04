
#ifndef DBUS_SERVER_TIPC_H
#define DBUS_SERVER_TIPC_H

#include <dbus/dbus-internals.h>
#include <dbus/dbus-server-protected.h>

DBUS_BEGIN_DECLS

DBusServer* _dbus_server_new_for_domain_socket (const char       *path,
                                                dbus_bool_t       abstract,
                                                DBusError        *error);
DBusServerListenResult
_dbus_server_listen_tipc_socket (DBusAddressEntry *entry,
									   DBusServer	   **server_p,
									   DBusError		*error);

dbus_bool_t
__dbus_open_tipc_socket (int              *fd,
                        DBusError        *error);
DBusServer*
_dbus_server_new_for_tipc_domain_socket (const char     *server_inst,
                                    DBusError      *error);
int
_dbus_listen_tipc_socket (const char     *server_inst,
                          DBusError      *error);
static dbus_bool_t
_dbus_set_local_creds (int fd, dbus_bool_t on);






DBUS_END_DECLS

#endif /* DBUS_SERVER_UNIX_H */
