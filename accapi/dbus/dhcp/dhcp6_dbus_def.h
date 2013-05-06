#ifndef _DHCP6_DBUS_DEF_H_
#define _DHCP6_DBUS_DEF_H_

#define DHCP6_DBUS_BUSNAME "aw.dhcp6"
#define DHCP6_DBUS_OBJPATH "/aw/dhcp6"
#define DHCP6_DBUS_INTERFACE "aw.dhcp6"

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
#define DHCP6_DBUS_METHOD_ENTRY_POOL_NODE				"dhcp6_entry_pool_node"
#define DHCP6_DBUS_METHOD_SET_INTERFACE_POOL				"dhcp6_set_interface_pool"
#define DHCP6_DBUS_METHOD_CREATE_POOL_NODE				"dhcp6_create_pool_node"
#define DHCP6_DBUS_METHOD_ADD_IP_POOL_RANGE				"dhcp6_add_ip_pool_range"
#define DHCP6_DBUS_METHOD_ADD_STATIC_HOST				"dhcp6_add_static_host"
#define DHCP6_DBUS_METHOD_ADD_DHCP_FAILOVER_PEER			"dhcp6_add_failover_peer"
#define DHCP6_DBUS_METHOD_CFG_DHCP_FAILOVER_PEER			"dhcp6_cfg_failover_peer"
#define DHCP6_DBUS_METHOD_DEL_DHCP_FAILOVER_PEER			"dhcp6_del_failover_peer"
#define DHCP6_DBUS_METHOD_SET_SERVER_DOMAIN_SEARCH			"dhcp6_set_server_domain_search"
#define DHCP6_DBUS_METHOD_SET_SERVER_VEO					"dhcp6_set_server_vci"
#define DHCP6_DBUS_METHOD_SET_SERVER_ROUTERS_IP			"dhcp6_set_server_routers_ip"
#define DHCP6_DBUS_METHOD_SET_SERVER_ENABLE				"dhcp6_set_server_enable"
#define DHCP6_DBUS_METHOD_SET_SERVER_DNS					"dhcp6_set_server_dns"
#define DHCP6_DBUS_METHOD_SET_SERVER_WINS_IP				"dhcp6_set_server_wins_ip"
#define DHCP6_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT		"dhcp6_set_server_lease_default"
#define DHCP6_DBUS_METHOD_SET_SERVER_LEASE_MAX			"dhcp6_set_server_lease_max"
#define DHCP6_DBUS_METHOD_SAVE_DHCP_LEASE				"dhcp6_save_lease"
#define DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE				"dhcp6_show_lease"
#define DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP			"dhcp6_show_lease_by_ip"
#define DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC			"dhcp6_show_lease_by_mac"
#define DHCP6_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF			"dhcp6_show_global_conf"
#define DHCP6_DBUS_METHOD_SHOW_IP_POOL_CONF				"dhcp6_show_ip_pool_conf"
#define DHCP6_DBUS_METHOD_SHOW_STATIC_HOST				"dhcp6_show_static_host"
#define DHCP6_DBUS_METHOD_SHOW_FAILOVER_CFG				"dhcp6_show_failover_cfg"
#define DHCP6_DBUS_METHOD_SHOW_RUNNING_CFG				"dhcp6_show_running_cfg"
#define DHCP6_DBUS_METHOD_SET_SERVER_OPTION52				"dhcp6_set_server_option52"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCP_DBUS_SET_HA_STATE
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		uint32 - vrid
 *		uint32 - vrrp state for the instance
 *		uint32 - uplink interface count
 *			ARRAY of uplink interface info as follows:
 *			uint32 - uplink ip address for master device
 *			uint32 - uplink ip address for backup device
 *			uint32 - uplink virtual ip address
 *			string - local uplink interface name
 *		uint32 - downlink interface count
 *			ARRAY of downlink interface info as follows:
 *			uint32 - downlink ip address for master device
 *			uint32 - downlink ip address for backup device
 *			uint32 - downlink virtual ip address
 *			string - local downlink interface name
 *		string - heartbeat link interface name
 *		uint32 - heartbeat link ip address
 *		uint32 - peer size heartbeat link ip address
 *
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/
#define DHCP6_DBUS_SET_HA_STATE							"dhcp6_ha_set_state"	

#endif

