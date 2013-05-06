#ifndef _DHCPSNP_DBUS_DEF_H_
#define _DHCPSNP_DBUS_DEF_H_
#define DHCPSNP_DBUS_BUSNAME "aw.dhcpsnp"

#define DHCPSNP_DBUS_OBJPATH "/aw/dhcpsnp"
#define DHCPSNP_DBUS_INTERFACE "aw.dhcpsnp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_CHECK_GLOBAL_STATUS
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_CHECK_GLOBAL_STATUS "check_dhcp_snp_status" 

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_GLOBAL_ENABLE		"endis_global_dhcp_snp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_CONFIG "dhcp_snp_show_running_global_config"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG_HANSI
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_HANSI_CONFIG "dhcp_snp_show_running_global_hansi_config"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DHCPSNP_DBUS_METHOD_INTERFACE_ENABLE
 *
 * INPUT:
 *		uint32 - interface index
 *		int - enable/disable flag
 *
 * OUTPUT:
 *		uint32 - opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_INTERFACE_ENABLE			"endis_intf_dhcp_snp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DHCPSNP_DBUS_METHOD_INTERFACE_ENABLE
 *
 * INPUT:
 *		uint32 - interface index
 *		int - enable/disable flag
 *		int - anti arp spoof type( via dhcp snooping interface or normal interface)
 *
 * OUTPUT:
 *		uint32 - opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_INTERFACE_ARP_ENABLE			"endis_intf_anti_arp_spoof"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DHCPSNP_DBUS_METHOD_INTERFACE_ONOFF_STA_STATIC_ARP
 *
 * INPUT:
 *		int - enable/disable flag
 *
 * OUTPUT:
 *		uint32 - opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_INTERFACE_ONOFF_STA_STATIC_ARP			"endis_sta_static_arp"
#define DHCPSNP_DBUS_METHOD_CHECK_SNP_INTERFACE_VE					"check_snp_interface_ve"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DHCPSNP_DBUS_METHOD_DEBUG_STATE
 *
 * INPUT:
 *		uint32 - enable/disable flag
 *		uint32 - debug level value
 *
 * OUTPUT:
 *		uint32 - opeartion result
 *		
*****************************************************************/
#define DHCPSNP_DBUS_METHOD_DEBUG_STATE			"endis_dbg"

#define DHCPSNP_DBUS_METHOD_SHOW_WAN_BIND_TABLE		"dhcp_snp_show_wan_bindtable"
#define DHCPSNP_DBUS_METHOD_DELETE_WAN_TABLE		"dhcp_snp_delelte_wan_bind_table"
#define DHCPSNP_DBUS_METHOD_CONFIG_AGING_TIME		"dhcp_snp_config_aging_time"
#define DHCPSNP_DBUS_METHOD_ADD_WAN_TABLE		"dhcp_snp_add_wan_bind_table"
#define DHCPSNP_DBUS_METHOD_ENABLE_ARP_PROXY		"dhcp_snp_enable_arp_proxy"
#endif
