#ifndef _DHCP_SNP_DBUS_H
#define _DHCP_SNP_DBUS_H
#include <dbus/dbus.h>

/**********************************************************************************
 * dhcp_snp_dbus_message_handler
 *	DHCP snooping dbus message handler
 *
 *	INPUT:
 *		connection - D-BUS connection
 *		message - dbus message
 *		user_data - user data along within dbus message
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		dbus reply message
 **********************************************************************************/
static DBusHandlerResult dhcp_snp_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_filter_function
 *	DHCP snooping dbus message filter
 *
 *	INPUT:
 *		connection - dbus connection
 *		message - dbus message
 *		user_data - user data along within dbus message
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		dbus reply message
 **********************************************************************************/
DBusHandlerResult dhcp_snp_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
);

/**********************************************************************************
 * dhcp_snp_dbus_init
 *	Initialize DHCP snooping dbus connection
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_dbus_init(void);

/**********************************************************************************
 * dhcp_snp_dbus_thread_main
 *	Main routine for dbus message handle thread
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
void * dhcp_snp_dbus_thread_main
(
	void *arg
);

#endif

