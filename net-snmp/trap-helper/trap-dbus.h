#ifndef _TRAP_DBUS_H_
#define _TRAP_DBUS_H_

void trap_dbus_init( void );

int trap_dbus_add_rules ( );

void trap_dbus_dispatch(int timeout_milliseconds);

void trap_init_tipc_connection(void);

#endif

