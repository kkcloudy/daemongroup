#ifndef _IU_DBUS_DEF_H_
#define _IU_DBUS_DEF_H_

#ifndef PATH_LEN
#define PATH_LEN (64)
#endif

extern char IU_DBUS_BUSNAME[PATH_LEN];
extern char IU_DBUS_OBJPATH[PATH_LEN];
extern char IU_DBUS_INTERFACE[PATH_LEN];

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCP_DBUS_METHOD_ENTRY_POOL_NODE
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		string - ip pool name
 *
 * OUTPUT:
 *		uint32 - operation result
 *		uint32 - ip pool index(mapped from dhcp server side) 
 *
 *****************************************************************************************/
extern char IU_SET_LOCAL_PARAS[PATH_LEN];
extern char IU_SET_REMOTE_PARAS[PATH_LEN];
extern char IU_DBUS_METHOD_SET_DEBUG_STATE[PATH_LEN];
extern char IU_DBUS_METHOD_SHOW_RUNNING_CFG[PATH_LEN];
extern char IU_DBUS_METHOD_SET_IU_ENABLE[PATH_LEN];
extern char IU_DBUS_METHOD_SET_IU_TO_SIGTRAN_ENABLE[PATH_LEN];


extern char IU_SET_ROUTING_CONTEXT[PATH_LEN];
extern char IU_SET_TRAFFIC_MODE[PATH_LEN];
extern char IU_SET_NETWORK_INDICATOR[PATH_LEN];
extern char IU_SET_NETWORK_APPERANCE[PATH_LEN];

extern char IU_SET_ADDRESS[PATH_LEN];
extern char IU_SET_POINT_CODE[PATH_LEN];
extern char IU_SET_CONNECTION_MODE[PATH_LEN];
extern char IU_SET_MULTI_SWITCH[PATH_LEN];
extern char IU_GET_LINK_STATUS[PATH_LEN];

#endif
