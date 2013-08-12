#ifndef _DHCP_DBUS_DEF_H_
#define _DHCP_DBUS_DEF_H_

#define DHCP_DBUS_BUSNAME "aw.dhcp"
#define DHCP_DBUS_OBJPATH "/aw/dhcp"
#define DHCP_DBUS_INTERFACE "aw.dhcp"

#define DHCRELAY_DBUS_BUSNAME "aw.dhcrelay"
#define DHCRELAY_DBUS_OBJPATH "/aw/dhcrelay"
#define DHCRELAY_DBUS_INTERFACE "aw.dhcrelay"

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
#define DHCP_DBUS_METHOD_ENTRY_POOL_NODE				"dhcp_entry_pool_node"
#define DHCP_DBUS_METHOD_SET_INTERFACE_POOL				"dhcp_set_interface_pool"
#define DHCP_DBUS_METHOD_CREATE_POOL_NODE				"dhcp_create_pool_node"
#define DHCP_DBUS_METHOD_ADD_IP_POOL_RANGE				"dhcp_add_ip_pool_range"
#define DHCP_DBUS_METHOD_ADD_STATIC_HOST				"dhcp_add_static_host"
#define DHCP_DBUS_METHOD_ADD_DHCP_FAILOVER_PEER			"dhcp_add_failover_peer"
#define DHCP_DBUS_METHOD_CFG_DHCP_FAILOVER_PEER			"dhcp_cfg_failover_peer"
#define DHCP_DBUS_METHOD_DEL_DHCP_FAILOVER_PEER			"dhcp_del_failover_peer"
#define DHCP_DBUS_METHOD_SET_SERVER_DOMAIN_NAME			"dhcp_set_server_domain_name"
#define DHCP_DBUS_METHOD_SET_SERVER_VEO						"dhcp_set_server_vci"
#define DHCP_DBUS_METHOD_SET_SERVER_NO_VEO						"dhcp_set_server_no_vci"
#define DHCP_DBUS_METHOD_SET_SERVER_VEO_OLD				"dhcp_set_server_vci_old"
#define DHCP_DBUS_METHOD_SET_SERVER_OPTION138				"dhcp_set_server_op138"
#define DHCP_DBUS_METHOD_SET_SERVER_NO_OPTION138				"dhcp_set_server_no_op138"
#define DHCP_DBUS_METHOD_SET_SERVER_OPTION60_ENABLE			"dhcp_set_server_option60_enable"
#define DHCP_DBUS_METHOD_SET_SERVER_OPTION60_SET_ID			"dhcp_set_server_option60_set_id"
#define DHCP_DBUS_METHOD_SET_SERVER_ROUTERS_IP			"dhcp_set_server_routers_ip"
#define DHCP_DBUS_METHOD_SET_SERVER_ENABLE				"dhcp_set_server_enable"
#define DHCP_DBUS_METHOD_SET_NAK_RSP_ENABLE				"dhcp_set_nak_rsp_enable"
#define DHCP_DBUS_METHOD_SET_ASN_ENABLE				    "dhcp_set_ASN_enable"
#define DHCP_DBUS_METHOD_SET_DHCP_FOR_LOCAL7_ENABLE				    "dhcp_log_for_local7"
#define DHCP_DBUS_METHOD_SET_DYNAMIC_ARP_ENABLE         "dhcp_set_dynamic_arp_enable"


#define DHCP_DBUS_METHOD_GET_SERVER_STATE				"dhcp_get_server_state"
#define DHCP_DBUS_METHOD_GET_LEASE_INFO					"dhcp_get_lease_info"
#define DHCP_DBUS_METHOD_SET_SERVER_STATIC_ENABLE		"dhcp_set_server_stitac_enable"
#define DHCP_DBUS_METHOD_SET_SERVER_DNS					"dhcp_set_server_dns"
#define DHCP_DBUS_METHOD_SET_SERVER_WINS_IP				"dhcp_set_server_wins_ip"
#define DHCP_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT		"dhcp_set_server_lease_default"
#define DHCP_DBUS_METHOD_SET_SERVER_LEASE_MAX			"dhcp_set_server_lease_max"
#define DHCP_DBUS_METHOD_SAVE_DHCP_LEASE				"dhcp_save_lease"
#define DHCP_DBUS_METHOD_SHOW_DHCP_LEASE				"dhcp_show_lease"
#define DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP			"dhcp_show_lease_by_ip"
#define DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC			"dhcp_show_lease_by_mac"
#define DHCP_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF			"dhcp_show_global_conf"
#define DHCP_DBUS_METHOD_SHOW_IP_POOL_CONF				"dhcp_show_ip_pool_conf"
#define DHCP_DBUS_METHOD_SHOW_STATIC_HOST				"dhcp_show_static_host"
#define DHCP_DBUS_METHOD_SHOW_FAILOVER_CFG				"dhcp_show_failover_cfg"
#define DHCP_DBUS_METHOD_SHOW_RUNNING_CFG				"dhcp_show_running_cfg"
#define DHCP_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG			"dhcp_show_running_hansi_cfg"
#define DHCP_DBUS_METHOD_SHOW_LEASE_STATE				"dhcp_show_lease_state"
#define DHCP_DBUS_METHOD_SHOW_LEASE_STATISTICS			"dhcp_show_lease_statistics"
#define DHCP_DBUS_METHOD_SET_UNICAST_REPLY_MODE			"dhcp_set_unicast_mode"
#define DHCP_DBUS_METHOD_CONFIG_DHCP_LEASE_ENABLE		"dhcp_client_leasetime_enable"
#define DHCP_DBUS_METHOD_SHOW_HASNIS_RUNNING_CFG		"dhcp_show_hansi_running_cfg"
#define DHCP_DBUS_METHOD_GET_POOL_INFO				    "get_pool_info"
#define DHCP_DBUS_METHOD_PING_CHECK_ENABLE				"dhcp_pingcheck_enable"
#define DHCP_DBUS_METHOD_LEASE_EXPIRY_TIME				"dhcp_lease_expiry_time"
#define DHCP_DBUS_METHOD_GET_STATISTICS_INFO			"dhcp_get_statistics_info"
#define DHCP_DBUS_METHOD_OPTIMIZE_ENABLE				"dhcp_optimize_enable"

/* pppoe snoooping */
#define DHCP_DBUS_METHOD_PPPOE_SNP_ENABLE				"pppoe_snooping_enable"
#define DHCP_DBUS_METHOD_PPPOE_SNP_IFACE_ENABLE			"pppoe_snooping_iface_enable"
#define DHCP_DBUS_METHOD_PPPOE_DEBUG				    "pppoe_snooping_set_debug"

/* DBA */
#define DHCP_DBUS_METHOD_DIRECT_BROADCAST_ENABLE		"dhcp_direct_broadcast_enable"

#define DHCP_DBUS_METHOD_GET_FAILOVER_STATE				"dhcp_get_failover_state"
#define DHCP_DBUS_METHOD_SET_BOOTP_ENABLE				"dhcp_set_bootp_enable"
/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCP_DBUS_METHOD_CHECK_SERVER_INTERFACE
 *	show static host
 *
 * INPUT:
 *		 
 *		 string - upifname,					
 *		 string - downifname,
 *		 uint32 -detect,	
 *		 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/
#define DHCP_DBUS_METHOD_CHECK_SERVER_INTERFACE			"dhcp_check_server_interface"
#define DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE_VE   "dhcp_check_relay_interface_ve"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_SET_DHCP_RELAY
 *	set dhcp relay
 *
 * INPUT:		
 *		string - downifname,
 *		string - upifname,
 *		uint32 -ipaddr,
 *		uint32 -add_info,
 *		 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/
#define DHCRELAY_DBUS_METHOD_SET_DHCP_RELAY				"dhcp_set_dhcp_relay"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_SHOW_DHCP_RELAY
 *	show dhcp relay
 *
 * INPUT: 
 *		uint32 -value,		 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/
#define DHCRELAY_DBUS_METHOD_SHOW_DHCP_RELAY			"dhcp_show_dhcp_relay"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_SHOW_RUNNING_CFG
 *	show running cfg
 *
 * INPUT:		
 *		
 *		 
 * OUTPUT:
 *		string - showStr
 *
 *****************************************************************************************/

#define DHCRELAY_DBUS_METHOD_SHOW_RUNNING_CFG			"dhcp_show_running_relay"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG
 *	show running cfg
 *
 * INPUT:		
 *		
 *		 
 * OUTPUT:
 *		string - showStr
 *
 *****************************************************************************************/

#define DHCRELAY_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG			"dhcp_show_running_hansi_relay"


/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_SET_RELAY_ENABLE
 *	set relay enable
 *
 * INPUT: 
 *		uint32 -enable,		 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/
#define DHCRELAY_DBUS_METHOD_SET_RELAY_ENABLE			"dhcp_set_relay_enable"


/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCP_DBUS_METHOD_SET_DEBUG_RELAY_STATE
 *	
 *
 * INPUT:
 *		uint32 - enable,	 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/

#define DHCP_DBUS_METHOD_SET_DEBUG_RELAY_STATE			    "relay_set_debug_state"


/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE
 *	
 *
 * INPUT: 
 *		string -  &ifname,
		uint32 - detect,		 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/

#define DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE		"dhcp_check_relay_interface"
#define DHCP_DBUS_METHOD_CHECK_INTERFACE_VE            "dhcp_check_interface_ve"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method DHCP_DBUS_METHOD_SET_DEBUG_STATE
 *	
 *
 * INPUT: 
 *		uint32 - debug_type,
		uint32 - enable,	 
 * OUTPUT:
 *		uint32 - operation result
 *
 *****************************************************************************************/

#define DHCP_DBUS_METHOD_SET_DEBUG_STATE			    "dhcp_set_debug_state"

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
#define DHCP_DBUS_SET_HA_STATE							"dhcp_ha_set_state"	

#endif
