/* eag_trap.c */
#include <stdio.h>
#include <dbus/dbus.h>
#include "eag_trap.h"
#include "eag_dbus.h"
#include "eag_log.h"
#include "eag_errcode.h"

#define TRAP_DBUS_OBJPATH						"/aw/eag"
#define TRAP_DBUS_INTERFACE					"aw.trap"

static eag_dbus_t *trap_eagdbus = NULL;

int eag_trap_set_eagdbus( eag_dbus_t *eagdbus)
{
	trap_eagdbus = eagdbus;
	eag_log_debug("eag_trap","eag_trap_set_eagdbus trap_eagdbus = %p",trap_eagdbus);
	return 0;
}

int eag_send_trap_signal(const char *signal_name,int first_arg_type,...)
{
	const char *obj_path = TRAP_DBUS_OBJPATH;
	const char *interface_name = TRAP_DBUS_INTERFACE;
	
	eag_log_debug("eag_trap","eag_send_trap_signal trap_eagdbus=%p, obj_path=%s, interface_name=%s",
												trap_eagdbus,obj_path,interface_name);
	
	va_list var_args;
	int iret = 0;
 
	va_start ( var_args, first_arg_type );
	iret = eag_dbus_send_signal( trap_eagdbus, 
							obj_path,
							interface_name,
							signal_name,
							first_arg_type,
							var_args );
 	va_end (var_args);

	return iret;
}

