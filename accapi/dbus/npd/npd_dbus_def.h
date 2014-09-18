#ifndef __NPD_DBUS_DEF_H__
#define __NPD_DBUS_DEF_H__

/*
  NOTICE: The file npd_dbus_def.h and npd_sysdef.h will be exported as
	API document to other dbus client application.

*/

enum npd_dbus_result_no_e {
	NPD_DBUS_SUCCESS,
	NPD_DBUS_ERROR, /* general use, no detail error information*/
	NPD_DBUS_ERROR_NO_SUCH_PORT,
	NPD_DBUS_ERROR_NO_SUCH_TRUNK,
	NPD_DBUS_ERROR_NO_SUCH_VLAN,/*added by wujh*/
	NPD_DBUS_ERROR_NO_SUCH_GROUP, /*added by wujh*/
	NPD_DBUS_ERROR_UNSUPPORT,/*add by zhubo*/
	NPD_DBUS_ERROR_FLOWCTL_NONE, /*add by zhubo*/
	NPD_DBUS_ERROR_BACKPRE_NODE, /*add by zhubo*/
	NPD_DBUS_ERROR_DUPLEX_NONE, 
	NPD_DBUS_ERROR_SPEED_NODE,
	NPD_DBUS_ERROR_AUNE_NONE,
	NPD_DBUS_ERROR_NO_PVE,
	NPD_DBUS_ERROR_MAX,
	NPD_DBUS_ERROR_ENABLE_FIRST,
	NPD_DBUS_ERROR_ALREADY_PORT,
	NPD_DBUS_ERROR_ALREADY_FLOW,
	NPD_DBUS_ERROR_ALREADY_HYBRID,
	NPD_DBUS_ERROR_NO_QOS_MODE,
	NPD_DBUS_ERROR_HYBRID_DSCP,
	NPD_DBUS_ERROR_HYBRID_UP,
	NPD_DBUS_ERROR_HYBRID_FLOW,
	NPD_DBUS_ERROR_STD_RULE,
	NPD_DBUS_BAD_VALUE, /*add by liuxy*/
	NPD_DBUS_ERROR_DUPLEX_MODE,/*add by liuxy*/
	NPD_DBUS_ERROR_BAD_k,
	NPD_DBUS_ERROR_BAD_M,
	NPD_DBUS_ERROR_BAD_IPG, /*add by liuxy*/
	NPD_DBUS_BOARD_IPG ,/*add by liuxy	*/
	NPD_DBUS_ETH_GE_SFP, /*add by liuxy*/
	NPD_DBUS_ERROR_NOT_SUPPORT,	/*added by qiluyu*/
	NPD_DBUS_ERROR_OPERATE, /*add by liuxy*/
	NPD_DBUS_ERROR_DUPLEX_FULL,/* add by qily*/
	NPD_DBUS_ERROR_DUPLEX_HALF /* add by qily*/
};


/*
	this marco indicate config ethport attr type
	and return value
*/
#define DEFAULT 0xff
#define ADMIN (1<<0)
#define SPEED (1<<1)
#define AUTONEGT (1<<2)
#define AUTONEGTS (1<<3)
#define AUTONEGTD (1<<4)
#define AUTONEGTF (1<<5)
#define DUMOD (1<<6)
#define FLOWCTRL (1<<7)
#define BACKPRE (1<<8)
#define LINKS (1<<9)
#define CFGMTU (1<<10)



#define NPD_DBUS_BUSNAME "aw.npd"

#define NPD_DBUS_OBJPATH "/aw/npd"
#define NPD_DBUS_INTERFACE "aw.npd"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF 	= 0x0,	 // default value
 *				 SYSLOG_DBG_DBG 	= 0x1,	 //normal 
 *				 SYSLOG_DBG_WAR 	= 0x2,	 //warning
 *				 SYSLOG_DBG_ERR 	= 0x4,	 // error
 *				 SYSLOG_DBG_EVT 	= 0x8,	 // event
 *				 SYSLOG_DBG_PKT_REV = 0x10,  //packet receive
 *				 SYSLOG_DBG_PKT_SED = 0x20,  //packet send
 *				 SYSLOG_DBG_PKT_ALL = 0x30,  //packet send and receive
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE         "dbg_npd"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF 	= 0x0,	 // default value
 *				 SYSLOG_DBG_DBG 	= 0x1,	 //normal 
 *				 SYSLOG_DBG_WAR 	= 0x2,	 //warning
 *				 SYSLOG_DBG_ERR 	= 0x4,	 // error
 *				 SYSLOG_DBG_EVT 	= 0x8,	 // event
 *				 SYSLOG_DBG_PKT_REV = 0x10,  //packet receive
 *				 SYSLOG_DBG_PKT_SED = 0x20,  //packet send
 *				 SYSLOG_DBG_PKT_ALL = 0x30,  //packet send and receive
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE       "no_dbg_npd"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NAM_DBUS_METHOD_SYSLOG_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_PKT_REV = 0x10,  //packet receive
 *				 SYSLOG_DBG_PKT_SED = 0x20,  //packet send
 *				 SYSLOG_DBG_PKT_ALL = 0x30,  //packet send and receive
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NAM_DBUS_METHOD_SYSLOG_DEBUG           "nam_log_debug"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NAM_DBUS_METHOD_SYSLOG_NO_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_PKT_REV = 0x10,  //packet receive
 *				 SYSLOG_DBG_PKT_SED = 0x20,  //packet send
 *				 SYSLOG_DBG_PKT_ALL = 0x30,  //packet send and receive
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NAM_DBUS_METHOD_SYSLOG_NO_DEBUG        "nam_log_no_debug"

/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_SYSTEM_CONFIG_COUNTER_DROP_STATISTIC "sys_global_config_counter"
/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUT :
 *       
 *
 *
 **************************************************************************************/
#define NPD_DBUS_CONFIG_HOST_DMAC_INGRESS_COUNTER  "sys_global_config_host_dmac_counter"
/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUT :
 *       
 *
 *
 **************************************************************************************/
#define NPD_DBUS_CONFIG_HOST_SMAC_INGRESS_COUNTER  "sys_global_config_host_smac_counter"
/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:
 *
 *
 * OUT :
 *       
 *
 *
 **************************************************************************************/
#define NPD_DBUS_CONFIG_HOST_INGRESS_COUNTER  "sys_global_config_host_mac_counter"



/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_SYSTEM_SHOW_COUNTER_DROP_STATISTIC  "sys_global_show_counter"

/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_SYSTEM_CONFIG_EGRESS_COUNTER "sys_global_egress_counter"

/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_CONFIG_VLAN_EGRESS_COUNTER "sys_egress_vlan_counter"

/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_SYSTEM_SHOW_COUNTER_EGRESS "egress_show_all_counter"
/*****************************************************************************************
 *DESCRIPTION:
 *
 *
 * INPUT:NULL
 *
 *
 * OUT :
 *       show drop packet all reason
 *
 *
 **************************************************************************************/
#define NPD_DBUS_SYSTEM_SHOW_COUNTER_INGRESS "ingress_show_all_counter"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NBM_DBUS_METHOD_SYSLOG_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NBM_DBUS_METHOD_SYSLOG_DEBUG           "nbm_log_debug"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NBM_DBUS_METHOD_SYSLOG_NO_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NBM_DBUS_METHOD_SYSLOG_NO_DEBUG        "nbm_log_no_debug"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SYS_GLOBAL_CFG_SAVE
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		byte  -- showStr 
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_SYS_GLOBAL_CFG_SAVE "sys_global_show_running"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_METHOD_ASIC_SYSLOG_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_ASIC_SYSLOG_DEBUG "asic_log_on"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_METHOD_ASIC_SYSLOG_NO_DEBUG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- different type of debug level defined as follows:*				
 *				 SYSLOG_DBG_DEF = 0x0,	 // default value
 *				 SYSLOG_DBG_PKT = 0x1,	 // packet
 *				 SYSLOG_DBG_DBG = 0x2,	 //normal 
 *				 SYSLOG_DBG_WAR = 0x4,	 //warning
 *				 SYSLOG_DBG_ERR = 0x8,	 // error
 *				 SYSLOG_DBG_EVT = 0x10,  // event
 *				 SYSLOG_DBG_ALL = 0xFF	 // all
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_ASIC_SYSLOG_NO_DEBUG "asic_log_off"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPU_PROTECTION_CONFIG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		flag -- on or off
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPU_PROTECTION_CONFIG "config_cpu_protect"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPU_FC_CONFIG
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		cpuFc -- packetType
 *		time ---timerange 
 *		rate --- packet rate
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPU_FC_CONFIG "config_cpu_fc"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPUFC_SET_QUEUE_QUOTA
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		unit32 - queue
 *		uint32 - quota value
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPUFC_SET_QUEUE_QUOTA "cpufc_set_quota"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPUFC_SET_QUEUE_QUOTA
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		unit32 - queue
 *		uint32 - shaper value in unit of PPS
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPUFC_SET_QUEUE_SHAPER "cpufc_set_shape"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPUFC_SET_QUEUE_QUOTA
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		uint32 - shaper value in unit of PPS
 *
 * OUTPUT:
 *		NONE
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPUFC_SET_PORT_SHAPER "cpufc_set_port_shape"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPU_SHOW_QUEUE_DESC
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		uint8 - cpu port queue number
 *		uint8 - packet Rx/Tx direction( 0 - Rx, 1 - Tx, 2 - bi-direction)
 *
 * OUTPUT:
 *		DBUS_TYPE_STRING - show result info
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPU_SHOW_QUEUE_DESC "cpu_show_queue_conf"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPU_SHOW_PORT_MIB
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		NULL
 *
 * OUTPUT:
 *		DBUS_TYPE_UINT32 - asic device count
 *			for each asice device has following items
 *			DBUS_TYPE_UINT32 - cpu interface type ( sdma or ethernet mac) or (pci or smi)
 *			/////////////////// SDMA interface mib info /////////////////////
 *				DBUS_TYPE_UINT32 - dma channel 0 receive packet count
 *				DBUS_TYPE_UINT32 - dma channel 0 receive byte count
 *				DBUS_TYPE_UINT32 - dma channel 0 receive error count
 *				DBUS_TYPE_UINT32 - dma channel 1 receive packet count
 *				DBUS_TYPE_UINT32 - dma channel 1 receive byte count
 *				DBUS_TYPE_UINT32 - dma channel 1 receive error count
 *				...
 *				DBUS_TYPE_UINT32 - dma channel 7 receive error count
 *				DBUS_TYPE_UINT32 - dma channel 7 receive packet count
 *				DBUS_TYPE_UINT32 - dma channel 7 receive byte count
 *			/////////////////// ethernet mac interface mib info /////////////////////
 *				DBUS_TYPE_UINT32 - receive good packet count low 32-bit
 *				DBUS_TYPE_UINT32 - receive good packet count high 32-bit
 *				DBUS_TYPE_UINT32 - receive bad packet count low 32-bit
 *				DBUS_TYPE_UINT32 - receive bad packet count high 32-bit
 *				DBUS_TYPE_UINT32 - receive good byte count low 32-bit
 *				DBUS_TYPE_UINT32 - receive good byte count high 32-bit
 *				DBUS_TYPE_UINT32 - receive bad byte count low 32-bit
 *				DBUS_TYPE_UINT32 - receive bad byte count high 32-bit
 *				DBUS_TYPE_UINT32 - receive internal drop packet count low 32-bit
 *				DBUS_TYPE_UINT32 - receive internal drop packet count high 32-bit
 *				DBUS_TYPE_UINT32 - transmit good packet count low 32-bit
 *				DBUS_TYPE_UINT32 - transmit good packet count high 32-bit
 *				DBUS_TYPE_UINT32 - transmit good byte count low 32-bit
 *				DBUS_TYPE_UINT32 - transmit good byte count high 32-bit
 *				DBUS_TYPE_UINT32 - transmit mac error packet count low 32-bit
 *				DBUS_TYPE_UINT32 - transmit mac error packet count high 32-bit
 *				DBUS_TYPE_UINT32 - reserved 
 *				...
 *				(append to fillin size upto DMA interface mib< 24 UINT32>)
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPU_SHOW_PORT_MIB "cpu_show_port_mib"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPU_CLEAR_PORT_MIB
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		NULL
 *
 * OUTPUT:
 *		DBUS_TYPE_UINT32	- result value
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPU_CLEAR_PORT_MIB "cpu_clear_port_mib"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CPUFC_GET_QUEUE_QUOTA
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		uint32 - queue 0 quota value
 *		uint32 - queue 1 quota value
 *		uint32 - queue 2 quota value
 *		uint32 - queue 3 quota value
 *		uint32 - queue 4 quota value
 *		uint32 - queue 5 quota value
 *		uint32 - queue 6 quota value
 *		uint32 - queue 7 quota value
 *
 *****************************************************************************************/
#define NPD_DBUS_METHOD_CPUFC_GET_QUEUE_QUOTA "cpufc_get_quota"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint32 - product_id // unsigned int of product type. uniquely identify a product, defined in npd_sysdef.h ,
 *	uint32 - sw_version // bitmap definition in npd_sysdef.h
 * 	string - product_name  // backplane info for chassis product, board info for box product
 *	string - product_base_mac_addr  // 12 char of mac address  with no : or - spliter.
 * 	string - product_serial_number  // 32 bytes string
 *	string - sw_name // software description string
 *
 *****************************************************************************************/
#define NPD_DBUS_INTERFACE_METHOD_VER "show_ver"

/*****************************************************************************************
 * DESCRIPTION:
 * 	This method should also work for box product
 *	If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
 *	arg lists for method NPD_DBUS_SLOTS_INTERFACE_METHOD_HWCONF
 *	in the order as they are appended in the dbus message.
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	byte total_slot_count // total slot count to show out
 *	Array of slot information
 *		byte - slot no // slot number
 *	 	uint32 - module id // module identifier
 *	 	byte - module status // module running status
 *		byte - module hw_version // hardware version
 *		byte - ext_slot_count // extended slot count
 *		string - module serial no // serial number
 *		string - module name // module name 
 *
 *****************************************************************************************/
#define NPD_DBUS_INTERFACE_METHOD_HWCONF "show_hwconf"


#define NPD_DBUS_SYSTEM_SHOW_STATE          "show_sys_state"

/*****************************************************************************************
 * DESCRIPTION:
 * 	Arguments list for NPD_DBUS_SYSTEM_SHUTDOWN_STATE dbus method
 *	This method is used to enable or disable system shutdown process when 
 * system temperature is extremely high
 *
 * INPUT:
 *		uint16 - isenable	(1 for enable, 0 for disable)
 *
 * OUTPUT:
 *		uint32 - ret  // operation return value
 *
 *****************************************************************************************/
#define NPD_DBUS_SYSTEM_SHUTDOWN_STATE      "shut_down_enable"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_ARP_CHECK_STATE
 *
 * INPUT:
 *		uint32   - isenable    // (1 for enable, 0 for disable)
 *
 * OUTPUT:
 *		uint32 - ret  // operation return value
 *
 ***********************************************************************/
#define NPD_DBUS_SYSTEM_ARP_CHECK_STATE		"arp_check_enable"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_ARP_CHECK_STATE
 *
 * INPUT:
 *		uint32   - isenable    // (1 for enable, 0 for disable)
 *
 * OUTPUT:
 *		uint32 - ret  // operation return value
 *
 ***********************************************************************/
#define NPD_DBUS_SYSTEM_ARP_STRICT_CHECK_STATE		"arp_strict_check_enable"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_ARP_CHECK_STATE
 *
 * INPUT:
 *		uint32   - isBroadCast    // (1 for broadcast, 0 for unicast)
 *
 * OUTPUT:
 *		uint32 - ret  // operation return value
 *
 ***********************************************************************/
#define NPD_DBUS_SYSTEM_ARP_AGING_DEST_MAC  "arp_aging_destmac_broadcast"
#define NPD_DBUS_SYSTEM_CONFIG_PORT_LINK    "config_eth_port_link"


#define NPD_DBUS_INTF_OBJPATH	"/aw/npd/intf"
#define NPD_DBUS_INTF_INTERFACE	"aw.npd.intf"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_METHOD_CREATE_VID_INTF
 *
 * INPUT:
 *		uint16   - vid    // vlan id
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_VLAN_BADPARAM - vid error
 *						NPD_VLAN_NOTEXISTS - vlan don't exist
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
  *					DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_CREATE_VID_INTF "create_one_vid_intf"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_METHOD_CREATE_VID_INTF
 *
 * INPUT:
 *		uint16   - vid    // vlan id
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_VLAN_BADPARAM - vid error
 *						NPD_VLAN_NOTEXISTS - vlan don't exist
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
  *					DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_CREATE_VID_INTF_BY_VLAN_IFNAME "create_one_vid_intf_by_vlan_ifname"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_METHOD_CREATE_VID_INTF
 *
 * INPUT:
 *		uint16   - vid    // vlan id
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_VLAN_BADPARAM - vid error
 *						NPD_VLAN_NOTEXISTS - vlan don't exist
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
  *					DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_VLAN_INTERFACE_ADVANCED_ROUTING_ENABLE "vlan_interface_advanced_routing_enable"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_GET_PORT_INDEX
 *
 * INPUT:
 *		uint16   - vid    // vlan id
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_VLAN_BADPARAM - vid error
 *						NPD_VLAN_NOTEXISTS - vlan don't exist
 *						L3_INTF_NOTEXIST - L3 interface hasn't exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_DIS_ROUTING_ERR  - set route disable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_DEL_VID_INTF "delete_one_vid_intf"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SUB_INTERFACE_CREATE
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 	byte   	- port_no  // port number on slot
 *		vlanid   - vlanid    
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_SUB_INTERFACE_CREATE  "create_sub_intf"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SUB_INTERFACE_DELETE
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 	byte   	- port_no  // port number on slot
 *		vlanid   - vlanid    
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_SUB_INTERFACE_DELETE "del_sub_intf"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_CONFIG_ADVANCED_ROUTING_DEFAULT_VID
 *
 * INPUT:
 *		vid - advanced-routing default-vid
 * 				<1-4094>	: set the advanced-routing default vid to this new value
 *					0		: clear the default vid value
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *			INTERFACE_RETURN_CODE_SUCCESS  - success
 *			COMMON_RETURN_CODE_BADPARAM	- bad parameter
 *			INTERFACE_RETURN_CODE_PROMIS_PORT_TAG_IN_VLAN		
 *					- there are some advanced-routing port in the vlan with tag
 *			INTERFACE_RETURN_CODE_VLAN_IS_L3INTF 		- the vlan is l3 interface
 *			INTERFACE_RETURN_CODE_ERROR			- general error 
 *			INTERFACE_RETURN_CODE_CAN_NOT_SET2_EMPTY		
 *					- can't set to empty in the current condition
 *			INTERFACE_RETURN_CODE_VLAN_NOTEXIST				
 *					- vlan is not exists when set advanced-routing default-vid
 *
 ***********************************************************************/

#define NPD_DBUS_CONFIG_ADVANCED_ROUTING_DEFAULT_VID "config_advanced_routing_default_vid"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SHOW_ADVANCED_ROUTING_DEFAULT_VID
 *
 * INPUT:
 *		NONE
 * OUTPUT:
 *		
 *		uint32  vid -	the advanced-routing default-vid vlaue we got
 *
 ***********************************************************************/

#define NPD_DBUS_SHOW_ADVANCED_ROUTING_DEFAULT_VID "show_advanced_routing_default_vid"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_ADVANCED_ROUTING
 *
 * INPUT:
 *		isEnable - enable or disable advanced route
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTERFACE_ADVANCED_ROUTING "advanced_route"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_IP_STATIC_ARP
 *
 * INPUT:
 *		PORTNO - SLOTNO/PORTNO
 *           MAC      -  MAC Address,AS H:H:H:H:H:H
 *           IP         -  IP Address, AS A.B.C.D/M
 *           VID       -  1-4094
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create static arp 
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *
 ***********************************************************************/
#define NPD_DBUS_INTERFACE_IP_STATIC_ARP "ip_static_arp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_IP_STATIC_ARP_FOR_TRUNK
 *
 * INPUT:
 *		TRUNKID - 1-127
 *           MAC      -  MAC Address,AS H:H:H:H:H:H
 *           IP         -  IP Address, AS A.B.C.D/M
 *           VID       -  1-4094
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create static arp
 *
 ***********************************************************************/
#define NPD_DBUS_INTERFACE_IP_STATIC_ARP_FOR_TRUNK "ip_static_arp_for_trunk"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP
 *
 * INPUT:
 *		PORTNO - SLOTNO/PORTNO
 *           MAC      -  MAC Address,AS H:H:H:H:H:H
 *           IP         -  IP Address, AS A.B.C.D/M
 *           VID       -  1-4094
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP "no_ip_static_arp"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP_FOR_TRUNK
 *
 * INPUT:
 *		TRUNKID - trunk id
 *           MAC      -  MAC Address,AS H:H:H:H:H:H
 *           IP         -  IP Address, AS A.B.C.D/M
 *           VID       -  1-4094
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						
 *
 ***********************************************************************/
#define NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP_FOR_TRUNK "no_ip_static_arp_for_trunk"
/****************************************************************************************************************
 * DESCRIPTION:
 * 	config or delete static arp for interface
 *
 * INPUT:
 *                      uint32 - isVlanIntf                          is this interface a vlan interface or not 
 *                                                                              (2  - is vlan advanced-routing, 1 - is, 0 - not)
 *                      uint8  -  slot                                  the static arp is configed for this slot/port
 * 	            uint8  -  port                                 the static arp is configed for this slot/port   if not vlan advanced-routing interface
 *                      uint32  - vlanId                              create/delete for this vlan,if it is a vlan interface. otherwise 0
 *                      uint32  - tag2                                if it is qinq subif this is the second tag,otherwise 0
 *                      uint8   -  mac[0]                             the mac address
 *                      ...                                                     ...
 *                      uint8   -  mac[5]                              mac address
 *                      uint32  - ipAddr                             create/delete for this ip address
 *                      uint32  - isAdd                               create  or  delete (1 - add,  0 - delete)
 *      
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *					            ARP_RETURN_CODE_SUCCESS    -  execute success
 *                                                                        COMMON_RETURN_CODE_NO_RESOURCE                         - if no memory allocatable
 *                                                                        COMMON_RETURN_CODE_NULL_PTR                                - if input parameters have null pointer
 *                                                                        ARP_RETURN_CODE_INTERFACE_NOTEXIST                      -   the interface is not exists
 *                                                                        ARP_RETURN_CODE_PORT_OR_TRUNK_NOT_NEEDED        -   create static arp for eth type interface/vlan adv interface with port/trunk
 *                                                                        ARP_RETURN_CODE_PORT_NOT_IN_VLAN                        -   parameter port is not in the parameter vlan
 *                                                                        ARP_RETURN_CODE_PORT_OR_TRUNK_NEEDED                -    create for simple vlan interface without port/trunk
 *                                                                        ARP_RETURN_CODE_NO_SUCH_PORT                               -   the port does not exist
 *                                                                        ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC       -   the mac configed is the same as the interface's
 *                                                                        ARP_RETURN_CODE_MAC_MATCHED_BASE_MAC               -   the mac is the same as the system mac
 *                                                                        ARP_RETURN_CODE_CHECK_IP_ERROR                            -     check ip error
 *                                                                        ARP_RETURN_CODE_NO_HAVE_ANY_IP
 *                                                                        ARP_RETURN_CODE_NOT_SAME_SUB_NET
 *                                                                        ARP_RETURN_CODE_CHECK_IP_ERROR
 *                                                                        ARP_RETURN_CODE_HAVE_THE_IP
 *                                                                        ARP_RETURN_CODE_STATIC_ARP_FULL                          - if the static arp items are equal to 1024 or more
 *                                                                        ARP_RETURN_CODE_TABLE_FULL                                  - the hash table is full
 *                                                                        ARP_RETURN_CODE_STATIC_EXIST                               - static arp item already exists
 *                                                                        ARP_RETURN_CODE_ERROR                                          - if other error occurs,get dev port failed or nexthop op failed
 *                                                                         
 *
 ******************************************************************************************************************************************************/
/*wangchao add*/
#define NPD_DBUS_DYNAMIC_ARP						   "config_dynamic_arp"

#define NPD_DBUS_INTERFACE_SET_STALE_TIME			   "config_interface_stale_time"	


#define NPD_DBUS_INTERFACE_STATIC_ARP                  "config_interface_static_arp"
/****************************************************************************************************************
 * DESCRIPTION:
 * 	config or delete static arp for interface
 *
 * INPUT:
 *                      uint32 - isVlanIntf                          is this interface a vlan interface or not 
 *                                                                              (2  - is vlan advanced-routing, 1 - is, 0 - not)
 *                      uint8  -  trunkId                                create for this trunk
 *                      uint32  - vlanId                              create/delete for this vlan,if it is a vlan interface. otherwise 0
 *                      uint32  - tag2                                if it is qinq subif this is the second tag,otherwise 0
 *                      uint8   -  mac[0]                             the mac address
 *                      ...                                                     ...
 *                      uint8   -  mac[5]                              mac address
 *                      uint32  - ipAddr                             create/delete for this ip address
 *                      uint32  - isAdd                               create  or  delete (1 - add,  0 - delete)
 *      
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *					            ARP_RETURN_CODE_SUCCESS    -  execute success
 *                                                                        COMMON_RETURN_CODE_NO_RESOURCE                         - if no memory allocatable
 *                                                                        COMMON_RETURN_CODE_NULL_PTR                                - if input parameters have null pointer
 *                                                                        ARP_RETURN_CODE_INTERFACE_NOTEXIST                      -   the interface is not exists
 *                                                                        ARP_RETURN_CODE_PORT_OR_TRUNK_NOT_NEEDED        -   create static arp for eth type interface/vlan adv interface with port/trunk
 *                                                                        ARP_RETURN_CODE_TRUNK_NOT_IN_VLAN                        -   parameter port is not in the parameter vlan
 *                                                                        ARP_RETURN_CODE_PORT_OR_TRUNK_NEEDED                -    create for simple vlan interface without port/trunk
 *                                                                        ARP_RETURN_CODE_TRUNK_NOT_EXISTS                               -   the port does not exist
 *                                                                        ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC       -   the mac configed is the same as the interface's
 *                                                                        ARP_RETURN_CODE_MAC_MATCHED_BASE_MAC               -   the mac is the same as the system mac
 *                                                                        ARP_RETURN_CODE_CHECK_IP_ERROR                            -     check ip error
 *                                                                        ARP_RETURN_CODE_NO_HAVE_ANY_IP
 *                                                                        ARP_RETURN_CODE_NOT_SAME_SUB_NET
 *                                                                        ARP_RETURN_CODE_CHECK_IP_ERROR
 *                                                                        ARP_RETURN_CODE_HAVE_THE_IP
 *                                                                        ARP_RETURN_CODE_STATIC_ARP_FULL                          - if the static arp items are equal to 1024 or more
 *                                                                        ARP_RETURN_CODE_TABLE_FULL                                  - the hash table is full
 *                                                                        ARP_RETURN_CODE_STATIC_EXIST                               - static arp item already exists
 *                                                                        ARP_RETURN_CODE_ERROR                                          - if other error occurs,get dev port failed or nexthop op failed
 *                                                                         
 *
 ******************************************************************************************************************************************************/

#define NPD_DBUS_INTERFACE_STATIC_ARP_TRUNK             "config_interface_static_arp_trunk"
/****************************************************************************************************************
 * DESCRIPTION:
 * 	config or delete static arp for interface
 *
 * INPUT:
 *                      
 *      
 * OUTPUT:
 *		
 *		string  -  static arp show running string such as " interface vlan3
 *                                                                                              config static-arp 00:00:00:01:01:01 192.168.0.1 [1/1]
 *                                                                                              exit"
 *                                                                         
 *
 **********************************************************************************************************************/

#define NPD_DBUS_INTF_INTERFACE_STATIC_ARP_SHOW_RUNNING    "interface_static_show_running"

/****************************************************************************************************************
 * DESCRIPTION:
 * 	show all ip neigh of the device
 *
 * INPUT:
 *                      
 *      
 * OUTPUT:
 *		
 *		haveMore   --   0 - not have any items any more
 *					    1 - have an item ,follow
 *	while(haveMore){
 *		ipAddr		-- the item's ip address
 *		ifname         -- the item's ifname
 *		macAddr[0] ~ macAddr[5] -- the mac of the item
 *		state	-- item's state
 *		haveMore	-- have another item?
 *   }
 *		ret  		--  the return value of the command
 *                                                                         
 *
 ******************************************************************************************************************************************************/
#define NPD_DBUS_SYSTEM_SHOW_IP_NEIGH		"show_ip_neigh"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_STATIC_ARP_METHOD_SHOW_RUNNING_CONFIG
 *
 * INPUT:
 *
 * 	
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/

#define NPD_DBUS_STATIC_ARP_METHOD_SHOW_RUNNING_CONFIG "show_running_arp"

#define NPD_DBUS_INTERRUPT_RXMAX_SHOW_RUNNING_CONFIG "show_running_interrupt_rxmax"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_GET_PORT_INDEX
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 	byte   	- port_no  // port number on slot
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_EXIST - L3 interface has exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_EN_ROUTING_ERR  - set route enable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_CREATE_PORT_INTF "create_one_port_intf"
/***********************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG
 *
 * INPUT:
 *		uint32   	- vlanAdv : is it vlan advanced-routing or eth-port advanced-routing 
 *										the running config we want to get.
 *							TRUE - we will get vlan advanced-routing config.
 *							FALSE - we will get eth-port advanced-routing config.
 *		uint32    - includeRgmii : whether include rgmii portss
 *							TRUE - include them
 *							FALSE - not include
 *
 * OUTPUT:
 *		
 *		string   - showStr : all advanced-routing interface of vlan(TRUE == vlanAdv) or eth-port(FALSE == vlanAdv)
 *							eg. :"interface vlan3\n advanced-routing enable\n exit\n"
 *		
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG "show_running_interface_advanced_routing"
/***********************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_ETH_INTERFACE_ENABLE_SET_SAVE_CFG
 *
 * INPUT:
 *
 * OUTPUT:
 *		
 *		string   - showStr : all l3 disabled interfaces
 *							eg. :"interface eth1-3\n l3-function disable\n exit\n"
 *		
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_ETH_INTERFACE_ENABLE_SET_SAVE_CFG "show_running_eth_interface_enable_set"


/***********************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTF_METHOD_VLAN_ETH_PORT_INTERFACE_ADVANCED_ROUTING_SHOW
 *
 * INPUT:
 *		uint32   	- flag : is it vlan advanced-routing or eth-port advanced-routing 
 *										the running config we want to get.
 *							0 - we will get vlan advanced-routing config.
 *							1 - we will get eth-port advanced-routing config.
 *		uint32   - slot_no : the slot no of the eth-port interface we want to get.
 *		uint32   - port_no : the port no of the eth-port interface we want to get.
 *		uint32   - vid       : the vid of the vlan interface we want to get.
 *
 * OUTPUT:
 *		
 *		uint32   - op_ret  : the return code of the command
 *							0  - if the running config info got success
 *							COMMON_RETURN_CODE_BADPARAM - bad parameter
 *							INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND - unsupport this command
 *		uint32   - isEnable : Only availd when the op_ret is 0
 *							0  - the interface is "advanced-routing disable"
 *							1  - the interface is "advanced-routing enable"
 *		
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_VLAN_ETH_PORT_INTERFACE_ADVANCED_ROUTING_SHOW "vlan_eth_port_interface_advanced_routing_show"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_GET_PORT_INDEX
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 		byte   	- port_no  // port number on slot
 *
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully create l3 intf
 *						NPD_DBUS_ERROR_NO_SUCH_PORT - slot/port  error
 *						L3_INTF_NOTEXIST - L3 interface hasn't exsted
 *						NPD_DBUS_ERROR - failed create l3 intf
 *						L3_INTF_DIS_ROUTING_ERR  - set route disable error
 *						DCLI_SET_FDB_ERR - set fdb error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_METHOD_DEL_PORT_INTF "delete_one_port_intf"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_GET_PORT_INDEX
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 		byte   	- port_no  // port number on slot
 *          uint32    - vid1        // sub interface tag, eg. eth0-1.3,  vid1 is 3
 *          uint32    - vid2        // sub sub interface ,eg. eth0-1.5.6, vid1 is 5,vid2 is 6
 * OUTPUT:
 *		
 *		uint32 - op_ret     
 *						INTERFACE_RETURN_CODE_SUCCESS - successfully create l3 intf
 *						INTERFACE_RETURN_CODE_QINQ_TYPE_FULL - the type value reach MAX count
 *                                   INTERFACE_RETURN_CODE_ERROR - other error
 *
 ***********************************************************************/
#define NPD_DBUS_INTF_SUBIF_SET_QINQ_TYPE   "set_subif_qinq_type"
#define NPD_DBUS_INTF_SET_QINQ_TYPE_SAVE_CFG    "set_qinq_type_show_running"
/*************************************************************************
 * INPUT: 
 *	oldName  - string            old interface name which we want to change it's name
 *	newName - string	           change to this name 
 * OUTPUT
 *   INTERFACE_RETURN_CODE_SUCCESS
 *	COMMON_RETURN_CODE_NULL_PTR
 *	COMMON_RETURN_CODE_BADPARAM
 *   INTERFACE_RETURN_CODE_ERROR
 **************************************************************************/
#define NPD_DBUS_INTF_CHANGE_INTF_NAME		"change_interface_name"


/*************************************************************************
 * INPUT: 
 *	ifnameType   - uint32        eth interface name format "ethx-y" or "ex-y"
 *	slot		-  uint8			eth interface slot number
 *	port		-  uint8			eth interface port number
 *	tag1		-  uint32			tag1 for eth subif
 *	tag2		-  uint32			tag2 for qinq subif
 *	enable	-  uint32			set to enable or disable
 *								0 - disable
 *								1 - enable
 * OUTPUT
 *   			INTERFACE_RETURN_CODE_SUCCESS
 *			INTERFACE_RETURN_CODE_NAM_ERROR
 *			INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *			INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND
 *			INTERFACE_RETURN_CODE_ERROR
 *
 **************************************************************************/

#define NPD_DBUS_INTF_ETH_INTERFACE_ENABLE_SET  "eth_interface_enable_set"


#define NPD_DBUS_FDB_METHOD_CREATE_VRRP_BY_IFNAME          "fdb_vrrp"


#define NPD_DBUS_SLOTS_OBJPATH "/aw/npd/slots"
#define NPD_DBUS_SLOTS_INTERFACE "aw.npd.slots"
#define NPD_DBUS_SLOTS_INTERFACE_METHOD_SHOW_SLOT_STATE		"show_slot_state"

#define NPD_DBUS_VLAN_OBJPATH  	"/aw/npd/vlan"
#define NPD_DBUS_VLAN_INTERFACE "aw.npd.vlan"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be created.
 *	string vlan name - vlanName  //vlan struct's member
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE				"create_vlan_entry"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be created.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE				"config_layer2_one"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_VIA_VLANNAME
 *
 * INPUT:
 *	string vlan name - vlanName  //vlan struct's member
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_VIA_VLANNAME		"config_layer2_one_vname"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_VLAN_ONE
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_VLAN_ONE				"config_vlan_entry"


#define NPD_DBUS_VLAN_METHOD_CONFIG_PROT_VLAN_ONE			"config_prot_vlan"
#define NPD_DBUS_VLAN_METHOD_CONFIG_SUPER_VLAN_ONE			"config_super_vlan"
#define NPD_DBUS_VLAN_METHOD_CONFIG_SUBVLAN_ADD_DEL			"add_del_sub_vlan"
#define NPD_DBUS_VLAN_METHOD_CONFIG_PROT_ETHER_ADD_DEL		"add_del_prot_ether"
#define NPD_DBUS_VLAN_METHOD_CONFIG_SHOW_SUPERVLAN_ENTRY	"show_super_vlan"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL
 *
 * INPUT:
 *	byte add/delete - isAdd	//isAdd=1,add a port to vlan;isAdd=0,delete a port from vlan
 *	byte slot number - slot_no //extended slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *	byte tag mode - isTagged //isTagged=0,add(delete) to(from) vlan untagged;isTagged=1,... 
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL		"add_del_vlan_port_member"		



#define NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_QINQ_ENDIS  "config_vlan_port_member_qinq_endis"	



#define NPD_DBUS_VLAN_METHOD_CONFIG_QINQ_UPDATE_FOR_MASTER  "config_vlan_port_member_qinq_update_for_master"

#define NPD_DBUS_VLAN_METHOD_CONFIG_ALLBACKPORT_QINQ_ENDIS  "config_vlan_allbackport_qinq_endis"

#define NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_ENDIS  "config_vlan_tocpuport_qinq_endis"

#define NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_ENDIS_FOR_OLD  "config_vlan_tocpuport_qinq_endis_for_old"

#define NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_UPDATE_FOR_MASTER  "config_vlan_tocpuport_qinq_update_for_master"	

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_UPDATE_VID
 *
 * INPUT:
 *	uint16 vlan corrent id	- vid_c 	//vlan original ID. 
 *	uint16 vlan new id	 	- vid_new 	//vlan new ID for update to be.
 *
 * OUTPUT:
 *	uint32 ret - ret		   //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_UPDATE_VID						"update_vid"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_ADD_DEL
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to vlan that'll be add(delete) to(from) vlan.
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret - ret		   //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_ADD_DEL	"add_del_vlan_trunk_member"
#define NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_UNTAG_TAG_ADD_DEL "addel_vlan_trunk_tag_untag"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS			"show_vlan_port_members"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1		"show_vlan_port_members_V1"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be config.
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *    uint32 promis port bmp - promisPortBmp[0] //any promis port member set 1 on according bit,low byte
 *    uint32 promis port bmp - promisPortBmp[1] //any promis port member set 1 on according bit,high byte
 *	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp[0] //any untag trunk member set 1 on according bit.low byte 
 *	uint32 untag trunk member bitmap - untagBmp[1] //any untag trunk member set 1 on according bit. highbyte
 *	uint32 tag trunk member bitmap - tagBmp[0] //any tag trunk member set 1 on according bit. low byte
 *	uint32 tag trunk member bitmap - tagBmp[1] //any tag trunk member set 1 on according bit. high byte
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS			"show_vlan_trunk_members"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_VNAME
 *
 * INPUT:
 *	string vlan name - vlanName	//vlan struct's member
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *	string vlan name - vlanId	//vlan struct's member.
 *	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_VNAME	"show_vlan_port_members_vname"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_VNAME
 *
 * INPUT:
 *	string vlan name - vlanName	//vlan struct's member
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint16 product Id -product_id //indicate product type
 *    uint32 promis port bmp - promisPortBmp[0] //any promis port member set 1 on according bit,low byte
 *    uint32 promis port bmp - promisPortBmp[1] //any promis port member set 1 on according bit,high byte
 *	string vlan name - vlanId	//vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp[0] //any untag trunk member set 1 on according bit.low byte 
 *	uint32 untag trunk member bitmap - untagBmp[1] //any untag trunk member set 1 on according bit. highbyte
 *	uint32 tag trunk member bitmap - tagBmp[0] //any tag trunk member set 1 on according bit. low byte
 *	uint32 tag trunk member bitmap - tagBmp[1] //any tag trunk member set 1 on according bit. high byte
 *
 *    uint32               vlanState
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_VNAME_V1	"show_vlan_port_members_vname_v1"

/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS_VNAME
 *
 * INPUT:
 *	string vlan name - vlanName //vlan struct's member
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *	string vlan name - vlanName //vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp //any untag trunk member set 1 on according bit. 
 *	uint32 tag trunk member bitmap - tagBmp //any tag trunk member set 1 on according bit. 
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS_VNAME	"show_vlan_trunk_members_vname"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 vlan count - vlan_count //indicate all active vlan count crrently
 *	uint32 product Id - product_id //indicate product type
 *	Array of vlan information
 *		uint16 vlan id	 - vlanId    //points to vlan.
 *	 	string vlan name - vlanName	//vlan struct's member.
 *	 	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	 	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS		"show_vlan_port_member_list"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 vlan count - vlan_count //indicate all active vlan count crrently
 *	uint32 product Id - product_id //indicate product type
 *    uint32 promis port bmp - promisPortBmp[0] //any promis port member set 1 on according bit,low byte
 *    uint32 promis port bmp - promisPortBmp[1] //any promis port member set 1 on according bit,high byte
 *	Array of vlan information
 *		uint16 vlan id	 - vlanId    //points to vlan.
 *	 	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp[0] //any untag trunk member set 1 on according bit.low byte 
 *	uint32 untag trunk member bitmap - untagBmp[1] //any untag trunk member set 1 on according bit. highbyte
 *	uint32 tag trunk member bitmap - tagBmp[0] //any tag trunk member set 1 on according bit. low byte
 *	uint32 tag trunk member bitmap - tagBmp[1] //any tag trunk member set 1 on according bit. high byte

 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1		"show_vlan_port_member_list_v1"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 vlan count - vlan_count //indicate all active vlan count crrently
 *	uint32 product Id - product_id //indicate product type
 *    uint32 promis port bmp - promisPortBmp[0] //any promis port member set 1 on according bit,low byte
 *    uint32 promis port bmp - promisPortBmp[1] //any promis port member set 1 on according bit,high byte
 *	Array of vlan information
 *		uint16 vlan id	 - vlanId    //points to vlan.
 *	 	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp[0] //any untag trunk member set 1 on according bit.low byte 
 *	uint32 untag trunk member bitmap - untagBmp[1] //any untag trunk member set 1 on according bit. highbyte
 *	uint32 tag trunk member bitmap - tagBmp[0] //any tag trunk member set 1 on according bit. low byte
 *	uint32 tag trunk member bitmap - tagBmp[1] //any tag trunk member set 1 on according bit. high byte

 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V2		"show_vlan_port_member_list_v2"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_TRUNK_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 vlan count - vlan_count //indicate all active vlan count crrently
 *	uint32 product Id - product_id //indicate product type
 *	Array of vlan information
 *		uint16 vlan id	 - vlanId    //points to vlan.
 *	 	string vlan name - vlanName	//vlan struct's member.
 *	 	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	 	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_TRUNK_MEMBERS	"show_vlan_trunk_member_list"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY
 *
 * INPUT:
 *	uint16 vlan id	 - vlanId    //points to vlan that'll be delete.
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY		"del_vlan_entry"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_DELETE_VLAN_ENTRY_VIA_NAME
 *
 * INPUT:
 *	 string vlan name - vlanName	//vlan struct's member.
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_DELETE_VLAN_ENTRY_VIA_NAME		"del_vlan_vid_vname"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_MTU
 *
 * INPUT:
 *	 uint mtu - mtu value	
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_MTU "cfg_vlan_mtu"
#define NPD_DBUS_VLAN_METHOD_CONFIG_FILTER "cfg_vlan_fltr"
#define NPD_DBUS_VLAN_METHOD_CONFIG_EGRESS_FILTER "cfg_vlan_egress_filter"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_PVID
 *
 * INPUT:
 *	byte slot number - slot_no //extended slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *	uint port vid - pvid //port pvid 
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define	NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_PVID				"set_one_port_pvid"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_ONE_PORT_PVID
 *
 * INPUT:
 *	byte slot number - slot_no //extended slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint16 port pvid - pvid		//port pvid
 *
 *****************************************************************************************/
#define	NPD_DBUS_VLAN_METHOD_SHOW_ONE_PORT_PVID				"show_port_pvid" 
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_PORTS_LIST_PVID
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	Array of vlan information
 *		uint16 port pvid - pvid		//port pvid
 *
 *****************************************************************************************/
#define	NPD_DBUS_VLAN_METHOD_SHOW_PORTS_LIST_PVID			"show_ports_list_pvid"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_EGRESS_FILTER
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	Array of vlan egress filter information
 *
 *****************************************************************************************/
#define	NPD_DBUS_VLAN_METHOD_SHOW_EGRESS_FILTER				"show_vlan_egress_filter"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SET_PORT_VLAN_INGRES_FLITER
 *
 * INPUT:
 *	byte slot number - slot_no //extended slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *	byte enable state - enDis //enalbe or disable port vlan ingress filter 
 *
 * OUTPUT:
 *	uint16 version - 
 *
 *****************************************************************************************/
#define	NPD_DBUS_VLAN_METHOD_SET_PORT_VLAN_INGRES_FLITER	"set_port_vlan_ingresfltr"

/*****************************************************************************
*	DESCRIPTION: 
*
*	INPUT:
*  
*	OUTPUT:  // in the order as they are appended in the dbus message.		
*		DBUS_TYPE_STRING - show running-config string
*
********************************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_RUNNING_CONFIG  "show_vlan_running"

/*****************************************************************************
*	DESCRIPTION: 
*
*	INPUT:
*  
*	OUTPUT:  // in the order as they are appended in the dbus message.		
*		DBUS_TYPE_STRING - show running-config string
*
********************************************************************************/

#define NPD_DBUS_VLAN_METHOD_EGRESS_FILTER_SHOW_RUNNING_CONFIG "show_vlan_egress_filter_running"

/*****************************************************************************
*	DESCRIPTION: 
*	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_SHOW_RUNNING_CONFIG
*
*	INPUT:
*		NONE;
*	OUTPUT:  // in the order as they are appended in the dbus message.		
*
********************************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_SHOW_RUNNING_CONFIG "show_igmp_snp_npd_running"


#define	NPD_DBUS_FDB_OBJPATH	"/aw/npd/fdb"
#define	NPD_DBUS_FDB_INTERFACE	"aw.npd.fdb"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME
 *
 * INPUT:
 *	uint16  -agingtime        //set fdb table aging interval time.  
 *
 * OUTPUT:
 *	NONE  - 
 *
 *****************************************************************************************/
#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME			"set_fdb_agingtime"


#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_SAVE			"save_fdb_static"
#define NPD_DBUS_FDB_METHOD_CHECK_FDB_STATIC_EXISTS			"check_fdb_static_exists"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME
 *
 * INPUT:
 *	uint16  -agingtime        //set fdb table default aging interval time(300s).
 *	 
 *
 * OUTPUT:
 *	NONE  - 
 *
 *****************************************************************************************/
#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME    "set_default_agingtime"



/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP
 *
 * INPUT:
 *	ubyte8   -dmac[6]|smac[6]    //two chose one param( destination mar or source mac) which is matched forwarding.
 *	uint16    -vlanid	    			  // valid vlan index which has been configed by vlan config command.
 *                           						
 *
 * OUTPUT:
 *	NONE     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP				"set_fdb_nodrop"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME
 *
 * INPUT:
 *	ubyte8  -dmac[6]|smac[6]    //two chose one param( destination mar or source mac) which is matched forwarding.
*	string    -vlanname			 // according vlanname find vlan index which has been configed by vlan config command ,
*                           						VLANNAME must be to begin with letter or '_' 
 *
 * OUTPUT:
 *	NONE    -
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME	"set_fdb_nodrop_name"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP
 *
 * INPUT:
 *	ubyte   -dmac[6]|smac[6]    // two chose one param ( destination mar or source mac) which is matched dropping. 
 *	uint16  - vlanid 		            //valid vlan index which has been configed by vlan config command.
 *                           					
 *
 * OUTPUT:
 *	NONE  - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP				    "set_fdb_drop"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME
 *
 * INPUT:
 *	ubyte   -dmac[6]|smac[6]    // two chose one param ( destination mar or source mac) which is matched dropping. 
 *	string   - vlanname 		      //according to vlanname find  the vlan index which has been configed ,
 *                           					VLANNAME must be to begin with letter or '_'
 *
 * OUTPUT:
 *	NONE  - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME       "set_fdb_drop_name"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN
 *
 * INPUT:
 *	uint16  -vlanid     //valid vlan index  which had configed by vlan config comand before delete.
 *
 * OUTPUT:
 *	NONE		 - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN     "set_fdb_delete_vlan"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT
 *
 * INPUT:
 *	ubyte8   - slot      //destination  slot which forward to
 *   ubyte8   - port    // destination  port  which forward to
 * OUTPUT:
 *	NONE     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT     "set_fdb_delete_port"



/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN
 *
 * INPUT:
 *	uint16  -vlanid     //valid vlan index  which had configed by vlan config comand before delete.
 *
 * OUTPUT:
 *	NONE		 - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN     "set_fdb_static_delete_vlan"


#define NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_STATIC_DELETE_WITH_VLAN     "set_debug_fdb_static_delete_vlan"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT
 *
 * INPUT:
 *	ubyte8   - slot      //destination  slot which forward to
 *   ubyte8   - port    // destination  port  which forward to
 * OUTPUT:
 *	NONE     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT     "set_fdb_static_delete_port"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK
 *
 * INPUT:
 *	UNSIGNED INT   - TRUNK_NO      //destination  trunk which forward to
 * OUTPUT:
 *	NONE     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK    "set_fdb_delete_trunk"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_SYSTEM_FDB
 *
 * INPUT:
 *	uint16  -vlanid     //valid vlan index  which had configed by vlan config comand 
 *	
 *
 * OUTPUT:
 *	NONE - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_SYSTEM_FDB				"config_system_fdb"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME
 *
 * INPUT:
 *	NONE
 *	
 *
 * OUTPUT:
 *	uint16 agingtime   -set fdb table aging interval time which had set.  
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME				"show_fdb_agingtime"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC
 *
 * INPUT:
 *	NONE
 *	ubyte8      - mac[6]        //destination mac for config fdb item.
 *	uint16       - vlanid           // valid vlan index ,which had been configed by vlan config command befor config .
 *   	ubyte8      -slot               // destination  slot which forward to
 *   ubyte8      - port             //destination  port  which forward to
 *
 * OUTPUT:
 *	NONE           - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC				"set_fdb_static"


#define NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_STATIC				"set_debug_fdb_static"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC
 *
 * INPUT:
 *	NONE
 *	ubyte8      - mac[6]        //destination mac for config fdb item.
 *	uint16       - vlanid           // valid vlan index ,which had been configed by vlan config command befor config .
 *   	ubyte8      -slot               // destination  slot which forward to
 *   ubyte8      - port             //destination  port  which forward to
 *
 * OUTPUT:
 *	NONE           - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC				"set_fdb_trunk_static"
#define NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_TRUNK_STATIC				"set_debug_fdb_trunk_static"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
 *
 * INPUT:
 *	ubyte8      - mac[6]    	//destination mac for config fdb item.
 *	string        - vlanname      //according to vlanname find  the vlan index which had been configed ,
 *                           					VLANNAME must be to begin with letter or '_'
 *   	ubyte8      - slot                 //destination  slot which forward to
 *   ubyte8      - port                //destination  port  which forward to
 * OUTPUT:
 *	NONE	     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_WITH_NAME     "set_fdb_static_name"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
 *
 * INPUT:
 *	ubyte8      - mac[6]    	//destination mac for config fdb item.
 *	string        - vlanname      //according to vlanname find  the vlan index which had been configed ,
 *                           					VLANNAME must be to begin with letter or '_'
 *    unsigned short    -trunkId    //trunk ID 
 * OUTPUT:
 *	NONE	     - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_TRUNK_WITH_NAME     "set_fdb_static_trunk_name"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
 *
 * INPUT:
 *	ubyte8   -  dmac[6]|smac[6]    //two chose one param( destination mar or source mac) which is matched dropping.
*	string     -  vlanname	                 //acording to the vlan name find the vlan index which has been configed by vlan config commad,
*                           						VLANNAME must be to begin with letter or '_'
 *
 * OUTPUT:
 *	NONE    - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC_WITH_NAME  "set_fdb_unstatic_name"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC
 *
 * INPUT:
 *	ubyte8    -  dmac[6]|smac[6]    //two chose one param( destination mar or source mac) which is matched dropping.
 *	uint16     -  vlanid	                        // the vlan index which has been configed by vlan config command, 
 *
 * OUTPUT:
 *	NONE      - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC		    "set_fdb_unstatic"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE
 *
 * INPUT:
 *   NONE  	  -
 *
 * OUTPUT:
 *	uint32        -dnumber              //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING, 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE		              "show_fdb"
#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_DEBUG		              "show_fdb_debug"
#define NPD_DBUS_FDB_METHOD_SYN_FDB_TABLE		              "syn_fdb_table"



/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE
 *
 * INPUT:
 *   NONE  	  -
 *
 * OUTPUT:
 *	uint32        -dnumber              //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING, 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE		              "show_fdb_dynamic"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE
 *
 * INPUT:
 *	NONE 
 *
 * OUTPUT:
 *	uint32        -dnumber              //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING,  
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE		      "show_fdb_static"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE
 *
 * INPUT:
 *	DMAC/SMAC: The flag of the mac address
 *               MAC:  The mac address
 *          VLANID:  Vlan Id
 * OUTPUT:
 *	None.
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE          "show_fdb_blacklist"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	uint16  -dnumber    //the fdb contain total  item .
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT              "show_fdb_count"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE
 *
 * INPUT:===================this command not used.========
 *	NONE
 *	uint32 version - 
 *
 * OUTPUT:
 *	uint16 version - 
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE                "show_fdb_one"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC
 *
 * INPUT:
 *	ubyte8      - mac[6]                 //mac address which is consis of six unsigned byte.
 *
 * OUTPUT:
 *	uint32        -dnumber              //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING,   
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC                "show_fdb_mac"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT
 *
 * INPUT:
 *	ubyte8       -slot                       //destinationslot which matchs forwarding to 
 *	ubyte8       -port                     //destination port which matchs forwarding to
 * OUTPUT:
 *	uint32        -dnumber             //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING,   
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT               "show_fdb_port"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN
 *
 * INPUT:
 *	uint16       - vlanid                   //the vlanid is which had config by vlan config comman .
 *
 * OUTPUT:
 *	uint32        -dnumber             //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING,   
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN               "show_fdb_vlan"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME
 *
 * INPUT:
 *	string         -vlanname          //according to vlanname find  the vlan index which has been configed by vlan config command,
 *                           					VLANNAME must be to begin with letter or '_'
 *
 * OUTPUT:
 *	uint32        -dnumber            //control output item number
 *	DBUS_TYPE_ARRAY,
 *   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
 *  		uint32  -dcli_flag             //acording value distinguish from  slot/port  trunk   vlanid  vldx.
 *  		uint16  -vlanid                 //the port belong to which vlan
 *  		uint32	-trans_value1    //if destination is interface trans_value1 is slot, else if trunk then trunkid .....
 * 		uint32  -trans_value2    //if destination is interface trans_value2 is port, else if value is 0;
 * 		ubyte8 -show_mac[0]  //mac address the first value.
 * 		ubyte8 -show_mac[1]  //mac address the second value.
 * 		ubyte8 -show_mac[2]  //mac address the thirdt value.
 * 		ubyte8 -show_mac[3]  //mac address the forth value.
 * 		ubyte8 -show_mac[4]  //mac address the fifth value.
 * 		ubyte8 -show_mac[5]  //mac address the sixth value.
 *   DBUS_STRUCT_END_CHAR_AS_STRING,    
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME     "show_fdb_vlan_name"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_STATIC_FDB_METHOD_SHOW_RUNNING_CONFIG
 *
 * INPUT:
 *	unsigned char         -slotNum
 *   unsigned char         -portNum  //These two are the slot/port number wanted to strict
 *   unsigned int            - Number // fdb number want to restric on slot/port
 * 
 * OUTPUT:
 *    None
 *   DBUS_STRUCT_END_CHAR_AS_STRING,    
 *
 *****************************************************************************************/

#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_PORT       "config_fdb_port_number"
#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN       "config_fdb_vlan_number"
#define NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN_PORT  "confing_fdb_vlan_port_number"

#define NPD_DBUS_FDB_METHOD_SHOW_FDB_NUMBER_LIMIT_ITEM         "show_fdb_number_limit_item"

#define NPD_DBUS_STATIC_FDB_METHOD_SHOW_RUNNING_CONFIG         "run_static_init"



#define	NPD_DBUS_PVE_OBJPATH	"/aw/npd/pve"
#define	NPD_DBUS_PVE_INTERFACE	"aw.npd.pve"
#define NPD_DBUS_PVE_METHOD_CREATE_PVE                   	   "create_pve"
#define NPD_DBUS_PVE_METHOD_ADD_PORT                   	   	   "add_port"
#define NPD_DBUS_PVE_METHOD_DELETE_PORT                   	   "delete_port"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_CONFIG_PVE_PORT
 *
 * INPUT:
 *	 ubyte8     - slot_no1       //config the pve slot number  
 *    ubyte8     - port_no1	     //config the pve port number
 *    ubyte8     - slot_no2       //destination  slot   number  which forward to
 *    ubyte8     - port_no2	    //destination   port number   which forward to
 * OUTPUT:
 *	 NONE
 *
 *****************************************************************************************/

#define NPD_DBUS_PVE_METHOD_CONFIG_PVE_PORT                    "config_pve_port"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_CONFIG_PVE_SPI_PORT
 *
 * INPUT:
 *	 ubyte8     - slot_no1       //config the pve slot number  
 *    ubyte8     - port_no1	     //config the pve port number
 *    ubyte8     - slot_no2       //destination  dev   number  which forward to spi port
 *    ubyte8     - port_no2	     //destination  port number   which forward to spi port
 * OUTPUT:
 *	NONE 
 *
 *****************************************************************************************/

#define NPD_DBUS_PVE_METHOD_CONFIG_PVE_SPI_PORT                "config_pve_spi"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_DELETE_PVE_PORT
 *
 * INPUT:
 *    ubyte8     - slot_no1       //delete the specify pve slot number  
 *    ubyte8     - port_no1	     //delete the specify port number
 *
 * OUTPUT:
 *	NONE
 *
 *****************************************************************************************/

#define NPD_DBUS_PVE_METHOD_DELETE_PVE_PORT                    "delete_pve_port"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_CONFIG_PVE_TRUNK
 *
 * INPUT:
 *	 ubyte8     - slot_no1       //config the pve slot number  
 *    ubyte8     - port_no1	     //config the pve port number
 *	 ubyte8     - trunkid          //destination trunkid which pve port forwarding to.
 * OUTPUT:
 *	uint16 version - 
 *
 *****************************************************************************************/

#define NPD_DBUS_PVE_METHOD_CONFIG_PVE_TRUNK                   "config_pve_trunk"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_SHOW_PVE
 *
 * INPUT:
 *	
 *
 * OUTPUT:
 *	 uint32      -count             	//control the loop when show the pve item.
 *	 uint16	     -tkflag            	//according the flag juding whether the destinatin is trunk or slot/port.
 *	 ubyte8     - pvslot       	//config the pve slot number  
 *    ubyte8     - pvport	     	//config the pve port number
 *    ubyte8     - lkslot       		//destination  dev   number  which forward to spi port
 *    ubyte8     - lkport	     		//destination  port number   which forward to spi port 
 *
 *****************************************************************************************/
#define NPD_DBUS_PVE_METHOD_SHOW_PVE                   		   "show_pve"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_SHOW_RUNNING_CFG
 *
 * INPUT:
 *	 	NONE
 *	 
 *    
 * OUTPUT:
 *		
 *		RETURN 
 *				showStr -- pvlan configure
 *
 *
 *****************************************************************************************/
#define NPD_DBUS_PVE_METHOD_SHOW_RUNNING_CFG				"show_pve_cfg"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_PVE_METHOD_CONFIG_PVE_CONTROL
 *
 * INPUT:
 *	 uint32	     -cmp                // acording to the value deciding whether control traffic send to uplinkport. 
 *	 ubyte8     - slot_no1       //config the pve slot number  
 *    ubyte8     - port_no1	     //config the pve port numbe
 * OUTPUT:
 *	uint16 version - 
 *
 *****************************************************************************************/

#define NPD_DBUS_PVE_METHOD_CONFIG_PVE_CONTROL                 "config_pve_control"
#define NPD_DBUS_PVE_METHOD_CONFIG_PVE_CPUPORT                 "config_pve_cpuport"



#define NPD_DBUS_TRUNK_OBJPATH		"/aw/npd/trunk"
#define NPD_DBUS_TRUNK_INTERFACE	"aw.npd.trunk"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be created.
 *	string trunk name - trunkName  //trunk struct's member
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE				"create_trunk_one"

/*********************************************************************
 * npd_dbus_dynamic_trunk_create_trunk
 * DESCRIPTION:
 *			dbus function for create a  dynamic trunk
 * INPUT:
 *			trunkId : uint16  - dynamic trunk id <1-8>
 * OUTPUT:
 *			ret : uint32  - the return value 
 *				TRUNK_RETURN_CODE_ERR_NONE            -         success
 *				TRUNK_RETURN_CODE_TRUNK_EXISTS      -	dynamic  trunk exists 
 *				TRUNK_RETURN_CODE_ERR_GENERAL		-	create failed
 *				TRUNK_RETURN_CODE_BADPARAM		-	bad parameter
 *				TRUNK_RETURN_CODE_ERR_HW			-     hardware  execute failed
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_TRUNK_METHOD_CREATE_DYNAMIC_TRUNK				"create_dynamic_trunk"

/*********************************************************************
 * npd_dbus_dynamic_trunk_create_trunk
 * DESCRIPTION:
 *			dbus function for create a  dynamic trunk
 * INPUT:
 *			trunkId : uint16  - dynamic trunk id <1-8>
 * OUTPUT:
 *			ret : uint32  - the return value 
 *				TRUNK_RETURN_CODE_ERR_NONE            -         success
 *				TRUNK_RETURN_CODE_TRUNK_EXISTS      -	dynamic  trunk exists 
 *				TRUNK_RETURN_CODE_ERR_GENERAL		-	create failed
 *				TRUNK_RETURN_CODE_BADPARAM		-	bad parameter
 *				TRUNK_RETURN_CODE_ERR_HW			-     hardware  execute failed
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_TRUNK_METHOD_DELETE_DYNAMIC_TRUNK				"delete_dynamic_trunk"



#define NPD_DBUS_TRUNK_METHOD_DYNAMIC_TRUNK_MAP_TABLE_UPDATE			"dynamic_trunk_map_table_update"


#define NPD_DBUS_TRUNK_METHOD_DYNAMIC_TRUNK_DEL_MAP_TABLE			"dynamic_trunk_del_trunk_map_table"

/*********************************************************************
 * npd_dbus_dynamic_trunk_add_delete_port_member
 * DESCRIPTION:
 *			dbus function for add port to or delete port from dynamic trunk
 * INPUT:
 *			isAdd   : uint32  - is add or delete port for dynamic trunk
 *			slot     : uint8    - the slot no we want to add to or delete from the dynamic trunk
 *			port     : uint8	   -  the port no we want to add to or delete from the dynamic trunk
 *			trunkId : uint16  - dynamic trunk id <1-8>
 * OUTPUT:
 *			ret : uint32  - the return value 
 *				TRUNK_RETURN_CODE_ERR_NONE            -         success
 *				TRUNK_RETURN_CODE_UNSUPPORT		-         rgmii not support this command
 *				TRUNK_RETURN_CODE_PORT_MBRS_FULL	-         port memeber for this trunk is full
 *				TRUNK_RETURN_CODE_TRUNK_NOTEXISTS		-	trunk not exists
 *				TRUNK_RETURN_CODE_ERR_GENERAL		-	add or delete failed
 *				TRUNK_RETURN_CODE_PORT_EXISTS		-	this port is already add to this trunk
 *				TRUNK_RETURN_CODE_INTERFACE_NOT_EXIST		-   the port is not a interface 
 *				TRUNK_RETURN_CODE_INTERFACE_L3_ENABLE		-   the port interface is not l3 disable interface
 *				TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT          -   the port is already member of a dynamic trunk
 *				TRUNK_RETURN_CODE_BADPARAM		-	bad parameter
 *				TRUNK_RETURN_CODE_ERR_HW			-     hardware  execute failed
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_TRUNK_METHOD_DYNAMIC_TRUNK_PORT_MEMBER_ADD_DEL "dynamic_trunk_port_add_del"
 
/*********************************************************************
 * npd_dbus_dynamic_trunk_create_trunk
 * DESCRIPTION:
 *			dbus function for config dynamic trunk, enter dynamic trunk configure node
 * INPUT:
 *			trunkId : uint16  - dynamic trunk id <1-8>
 * OUTPUT:
 *			ret : uint32  - the return value 
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_DYNAMIC_TRUNK_METHOD_CONFIG	"dynamic_trunk_config"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_CONFIG_ONE
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be created.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_CONFIG_ONE					"config_trunk_id"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME
 *
 * INPUT:
 *	string trunk name - trunkName  //trunk struct's member
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be config.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME			"config_trunk_name"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL
 *
 * INPUT:
 *	byte add/delete - isAdd	//isAdd=1,add a port to trunk;isAdd=0,delete a port from trunk
 *	byte slot number - slot_no //extended slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be config.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL			"trunk_add_del_port"
#define NPD_DBUS_TRUNK_METHOD_TRUNK_MAP_TABLE_UPDATE			"trunk_map_table_update"


/* for update g_trunk list of distributed trunk */
#define NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_TRUNK_LIST_ADD_DEL			"trunk_list_add_del_port"


/*****************************************************************************************
 * DESCRIPTION:
 *	arg lists for method NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST
 * INPUT:
 *	byte allow/refuse - isAllow //isAllow = 1,trunk allow to transmit packet of special vlan
 *	int32 vlan count - count //allow/refuse traffic vlan count 
 *	string vlan list - vid[]vlan be allowed or refused
 *	uint16 trunk id - trunkId //points to trunk that'll be config.
 *
 ****************************************************************************************/
 #define NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST		"allow_refuse_vlan_on_trunk"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY			"del_trunk_id"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME
 *
 * INPUT:
 *	string trunk name - trunkName  //trunk struct's member
 *
 * OUTPUT:
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME	"del_trunk_name"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *
 * OUTPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	uint32 product id - productId //for ax7000 and au50000
 *	uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS		"show_one_trunk_port_member"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *
 * OUTPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	uint32 product id - productId //for ax7000 and au50000
 *	uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1		"show_one_trunk_port_member_v1"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME
 *
 * INPUT:
 *	string trunk name - trunkName  //trunk struct's member
 *
 * OUTPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	uint32 product id - productId //for ax7000 and au50000
 *	uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME			"show_one_trunk_by_name"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME
 *
 * INPUT:
 *	string trunk name - trunkName  //trunk struct's member
 *
 * OUTPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	uint32 product id - productId //for ax7000 and au50000
 *	uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME_V1			"show_one_trunk_by_name_v1"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	Array of trunk information
 *	 	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	 	string trunk name - trunkName  //trunk struct's member
 *		uint32 product id - productId //for ax7000 and au50000
 *		uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS	"show_trunk_list"
/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:
 *	Array of trunk information
 *	 	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *	 	string trunk name - trunkName  //trunk struct's member
 *		uint32 product id - productId //for ax7000 and au50000
 *		uint32 portmember Bitmap - portmbrBmp_sp //members of trunk 
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1	"show_trunk_list_v1"


/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG
 *
 * INPUT:
 *	byte slot number - slot_no //slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 * OUTPUT:
 *
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG	"set_trunk_master_port"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE
 *
 * INPUT:
 *	byte slot number - slot_no //slot number
 *	byte eth port number - local_port_no //ethernet port on extended slot
 *  byte enable variable - enable or diable
 *
 * OUTPUT:
 *
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE	"trunk_port_endis"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE
 *
 * INPUT:
 *  unit16 trunkId - trunk id (which trunk will be config)
 *	byte banlcMode - trunk load balance mode select
 *
 * OUTPUT:
 *
 *	uint32 ret		 - ret		 //npd operation return value,indicates whether it success.
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE	"config_load_banlc"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION
 *
 * INPUT:
 *	uint16 trunk id	 - trunkId    //points to trunk that'll be deleted.
 *
 * OUTPUT:
 *	Array of trunk information
 *	 	string trunk name - trunkName  //trunk struct's member
 *	 	uint16 vlan id	 - trunkId    //points to vlan that'll be deleted.
 *	 	string vlan name - trunkName  //vlan struct's member
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION	"show_trunk_allow_vlanlist"

/*****************************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_TRUNK_METHOD_SHOW_RUNNING_CONFIG
 *
 * INPUT:
 *	NONE
 *
 * OUTPUT:  // in the order as they are appended in the dbus message.		
 *
 *****************************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_RUNNING_CONFIG			  "show_trunk_running"

/*********************************************************************
 * npd_dbus_dynamic_trunk_show_running_config
 * DESCRIPTION:
 *			dbus function for show running config
 * INPUT:
 *			NONE
 * OUTPUT:
 *			showStr : string  -  the show running result string, eg. "create dynamic-trunk 1
 *														    config dynamic-trunk 1
 *															add port 1/1
 *														    exit
 *														    "
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_DYNAMIC_TRUNK_METHOD_SHOW_RUNNING_CONFIG			  "show_dynamic_trunk_running"

/*********************************************************************
 * npd_dbus_dynamic_trunk_show_dynamic_trunk_list
 * DESCRIPTION:
 *			dbus function for show dynamic trunk list
 * INPUT:
 *			trunkId : uint16 -  the dynamic trunkId we want to show port list
 *							0  -  show dynamic trunk port list
 *							else - show dynamic trunk [trunkId] port list
 * OUTPUT:
 *			1) the trunkId input is 0:
 *					id  : uint32   -  the trunkId or 0(no more dynamic trunk to show)
 *							0  -  no more dynamic trunk to show
 *							else - show dynamic trunk[id]
 *						while(id){
 *								haveMore : uint32  - have port list or not
 *										0  -  no more port 
 *										else  - have more port to show
 *									while(haveMore){
 *											slot : uint8  -  slot no
 *											port : uint8  -  port no
 *											haveMore : uint32  - have port list or not
 *									}
 *								id : uint32  -  the trunkId or 0(no more dynamic trunk to show)
 *						}
 *			2) the trunkId input is not 0:
 *					ret : uint32	-  return value
 *								TRUNK_RETURN_CODE_TRUNKID_OUT_OF_RANGE
 *								TRUNK_RETURN_CODE_TRUNK_NOTEXISTS
 *								TRUNK_RETURN_CODE_TRUNK_ERR_NONE
 *						if(TRUNK_RETURN_CODE_TRUNK_ERR_NONE == ret){
 *								haveMore : uint32  - have port list or not
 *										0  -  no more port 
 *										else  - have more port to show
 *									while(haveMore){
 *											slot : uint8  -  slot no
 *											port : uint8  -  port no
 *											haveMore : uint32  - have port list or not
 *									}
 *						}
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *********************************************************************/
#define NPD_DBUS_DYNAMIC_TRUNK_METHOD_SHOW_TRUNK_MEMBER_LIST			  "show_dynamic_trunk_member_list"
#define NPD_DBUS_DYNAMIC_TRUNK_METHOD_SHOW_TRUNK_VLAN_MEMBER_LIST			  "show_dynamic_trunk_vlan_member_list"
/*****************************************************************************
 *npd_dbus_dynamic_trunk_show_dynamic_trunk_hardware_information
 *
 * DESCRIPTION:
 *			dbus function for show port members in a dynamic trunk
 * INPUT:
 *			trunkId : uint16 -  the dynamic trunkId that want to show
 *							0  -  show dynamic trunk port list
 *							else - show dynamic trunk [trunkId] port list
 * OUTPUT:
 *                1) the trunkId input is 0:
 *                id	: uint32   -  the trunkId or 0(no more dynamic trunk to show)
 *							0  -  no more dynamic trunk to show
 *							else - show dynamic trunk[id]
 *						while(id){
 *								signal : uint32  - have port list or not
 *										0  -  no more port 
 *										else  - have more port to show
 *									while(signal){
 *											slot : uint8  -  slot no
 *											port : uint8  -  port no
 *											signal : uint32  - have port list or not
 *									}
 *								id : uint32  -	the trunkId or 0(no more dynamic trunk to show)
 *						}
 *			2) the trunkId input is not 0:
 *			the trunkId input range in <1-8>:
 *					ret : uint32	-  return value
 *								TRUNK_RETURN_CODE_TRUNKID_OUT_OF_RANGE
 *								TRUNK_RETURN_CODE_TRUNK_NOTEXISTS
 *								TRUNK_RETURN_CODE_TRUNK_ERR_NONE
 *						if(TRUNK_RETURN_CODE_TRUNK_ERR_NONE == ret){
 *								signal : uint32  - have [E/D]port or not
 *										0  -  no more ports 
 *										else  - have more port to show
 *									while(signal){
 *											slot : uint8  -  slot no
 *											port : uint8  -  port no
 *											signal : uint32  - have port list or not
 *									}
 *						}
 * RETURN:
 *			NULL  -  get args failed 
 *			reply  -  else
 * NOTE:
 *
 *****************************************************************************/
#define NPD_DBUS_TRUNK_METHOD_SHOW_DYNAMIC_TRUNK_HW_INFO         "show_dynamic_trunk_hardware_infomation"
/************************************************
* DESCRIPTION:
*       clear all dynamic or all static arp on this trunk
* INPUT :
*      trunkId  -  the trunkId,
*      isStatic  -  clear static arp or dynamic
* OUTPUT:
*      none
* NOTE:
*      
*************************************************/
#define NPD_DBUS_TRUNK_METHOD_CLEAR_TRUNK_ARP       "clear_trunk_arp"

/* Port interface will be used with VLAN/TRUNK/ and so on.*/
#define NPD_DBUS_ETHPORTS_OBJPATH "/aw/ethports"
#define NPD_DBUS_ETHPORTS_INTERFACE "aw.ethports"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE
 *
 * INPUT:
 *		byte   - slot_no    // slot number
 * 	byte   - port_no  // port number on slot
 *		uint32- mode		// port mode
 *
 * OUTPUT:
 * 	NONE
 *
 *		RETURN
 *			NPD_SUCCESS
 *			NPD_FAILURE
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE   "config_eth_mode"
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE_VE   "config_eth_mode_interface_ve"
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE_DEL_VE   "config_eth_mode_del_ve"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE
 *
 * INPUT:
 *		byte   - slot_no    // slot number
 * 	byte   - port_no  // port number on slot
 *		uint32- mode		// port mode
 *
 * OUTPUT:
 * 	NONE
 *
 *		RETURN
 *			NPD_SUCCESS
 *			NPD_FAILURE
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE   "config_eth_mode_interface"


#define NPD_DBUS_ETHPORTS_INTERFACE_LACP_FUNCTION_ENDIS  "eth_port_lacp_function_endis"



/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE
 *
 * INPUT:
 *		byte   - slot_no    // slot number
 * 		byte   - port_no  // port number on slot
 *		uint32- mode		// port mode
 *
 * OUTPUT:
 * 		NONE
 *
 * RETURN
 *		NPD_SUCCESS
 *		NPD_FAILURE
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_BUFFER_MODE 	"config_buffer_mode"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL
 *
 * INPUT:
 *	byte   - isEnable   // shared or divided global acl buffer mode on device
 *
 * OUTPUT:
 *	uint32 - op_ret     //NPD_DBUS_SUCCESS - successfully set global buffer mode service
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_BUFFER_MODE 	"show_buffer_mode"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_BUFFER_MODE
 *
 * INPUT:
 * 	 NULL
 *
 * OUTPUT:	
 *	uint32  - Isable            // BUFFER_MODE_SHARED  -global buffer mode is shared
 *								// BUFFER_MODE_DIVIDED -global buffer mode is divided
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT "config_eth_port"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR
 *
 * INPUT:
 * 	uint32 - type 	
 *						DEFAULT 		// default value
 *						ADMIN 		// config admin state ( enable/ disable)
 *						SPEED 			//config speed ( 10/100/1000)
 *						AUTONEGT 		//config auto negotiation ( enable/ disable)
 * 					AUTONEGTS		//config auto negotiation speed ( enable/ disable)
 *						AUTONEGTD 	//config auto negotiation duplex ( enable/ disable)
 *						AUTONEGTF 		//config auto negotiation flowcontrol ( enable/ disable)
 *						DUMOD 		//config duplex mode (half/full)
 * 					FLOWCTRL		//config flow-control (enable/disable)
 * 					BACKPRE 		//config back-pressure (enable/disable)
 * 					LINKS 			//config link state (down/up)
 *						CFGMTU 		//config mtu <64-8192>
 *		uint32 - port_index  //port index
 * 	uint32 - value 
 *                   	enbale  1 
 *						disable 0
 *						up		1
 *						down   0
 *						half		0
 *						full		1
 *						speed	input param
 *						mtu		input param
 *
 * OUTPUT:
 * 	uint32 - port_index  //port index
 *		uint32 - op_ret     //NPD_DBUS_SUCCESS - successfully config this port
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR "config_eth_port_attr"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_VCT
 *
 * INPUT:
 * 	 uint8  - slot_no	    //slot number
 *	 uint8  - port_no		//port number
 *	 uint32 - port_index    //port index
 * 	 uint32 - mode			//port mode or config mode 
 *
 * OUTPUT:
 * 	 uint32 - port_index  //port index
 *	 uint32 - op_ret     //NPD_DBUS_SUCCESS - successfully config this port
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_VCT 	"config_eth_port_oct"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_READ_PORT_VCT
 *
 * INPUT:
 * 	 uint8  - slot_no	    //slot number
 *	 uint8  - port_no		//port number
 *	 uint32 - port_index    //port index
 * 	 uint32 - mode			//port mode or config mode 
 *
 * OUTPUT:
 * 	 uint32 - port_index  	//port index
 *	 uint32 - op_ret     	//NPD_DBUS_SUCCESS - successfully config this port
 *	 uint16 - state			//vct state
 *	 uint16 - len			//cable len
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_READ_PORT_VCT	"read_eth_port_oct"

/*****************************************************************************
 * DESCRIPTION:
*		This method should also work for box product
*		If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*		arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST
*
*	INPUT:
*	 	NONE
*
*  OUTPUT:  
*		byte total_slot_count    // 1 for box, more for chassis
*		Array of ethport information of each slotslot
*			byte slot no
*			byte total_port_count
*			Array of eth port information of this slot
*				byte port no
*				byte port type
*				uint32 attr_bitmap
*				uint32 MTU	
*               uint32 link_keep_time
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST "show_ethport_list"

/*****************************************************************************
*	DESCRIPTION: 
*		This method should also work for box product
*		If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*		arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR
*
*	INPUT:
*	 	byte slot    //slot number on the board
*		byte port   //port number on the slot
*  
*	OUTPUT:  // in the order as they are appended in the dbus message.		
*		Array of eth port information of this slot
*			byte port type
*			uint32 attr_bitmap
*			uint32 MTU	
*           uint32 link_keep_time
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR "show_ethport_attr"

/*****************************************************************************
*	DESCRIPTION: 
*			This method should also work for box product
*			If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*			arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT
*
*	INPUT:
*	 	byte slot    //slot number on the board
*		byte port   //port number on the slot
*
*  OUTPUT:  
*		struct of eth port information of this slot
*		tx
* 			uint64 packets; 
* 			uint64 errors;	
* 			uint64 dropped;	
* 			uint64 overruns;
* 			uint64 frame;
* 			uint64 goodbytesl;
* 			uint64 goodbytesh;
* 			uint64	uncastframe;
* 			uint64	bcastframe;
* 			uint64 	fcframe;
* 			uint64	mcastframe;
* 			uint64	crcerror_fifooverrun;
		rx
* 			uint64 uncastpkts;
* 			uint64 bcastpkts;
* 			uint64 mcastpkts;
* 			uint64 CRCerrors;
* 			uint64 dropped;
* 			uint64 overruns;
* 			uint64 carrier;
* 			uint64 collision;
* 			uint64 goodbytesl;
* 			uint64 goodbytesh;
* 			uint64 badbytes;
* 			uint64 fcframe;
* 			uint64 errorframe;
* 			uint64 jabber;
* 			uint64 underSizeframe;
* 			uint64 overSizeframe;
* 			uint64 fragments;	
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT "show_ethport_stat"

/*****************************************************************************
*  DESCRIPTION: 
*			This method should also work for box product
*			arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_XG_ETHPORT_STAT
*
*  INPUT:
*	 	None.
*
*  OUTPUT:  
*		eth-port MIB information detailed as follow in sequence:
*		Rx
* 			uint64 - unicast packet count 
*			uint64 - broadcast packet count
* 			uint64 - multicast packet count
*			uint64 - fc frame count
* 			uint64 - good bytes low 32-bit
*			uint64 - good bytes high 32-bit
* 			uint64 - bad bytes
* 			uint64 - crc error
*		Tx
* 			uint64 - unicast packet count 
*			uint64 - broadcast packet count
* 			uint64 - multicast packet count
*			uint64 - fc frame count
* 			uint64 - good bytes low 32-bit
*			uint64 - good bytes high 32-bit
* 			uint64 - bad bytes
* 			uint64 - crc error
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_XG_ETHPORT_STAT "show_xg_ethport_stat"


/*****************************************************************************
*	DESCRIPTION: 
*			This method should also clear all statistic info;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT
*
*	INPUT:
*	 	byte slot    //slot number on the board
*		byte port   //port number on the slot
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT "clear_ethport_stat"

/*****************************************************************************
*	DESCRIPTION: 
*			This method should clear arp;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT
*
*	INPUT:
*	 	byte slot    //slot number on the board
*		byte port   //port number on the slot
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_ARP "clear_ethport_arp"

/*****************************************************************************
*	DESCRIPTION: 
*			This method should show arp;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ARP
*
*	INPUT:
*	 	byte slot    //slot number on the board
*		byte port   //port number on the slot
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ARP "show_ethport_arp"

/*****************************************************************************
*	DESCRIPTION: 
*			This method should show arp;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_ARP
*
*	INPUT:
*	 	trunkId - trunk id
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_ARP "show_trunk_arp"


/*****************************************************************************
*	DESCRIPTION: 
*			This method should show trunk nexthop info;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_NEXTHOP
*
*	INPUT:
*	 	trunkId - trunk id
*
*  	OUTPUT:  
* 			NONE
*
*		RETURN
*			
*			
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_NEXTHOP "show_trunk_nexthop"


/*****************************************************************************
*	DESCRIPTION: 
*			This method should show nexthop;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ALL_NEXTHOP
*
*	INPUT:
*	 	
*		
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ALL_NEXTHOP "show_all_nexthop"

/*****************************************************************************
*	DESCRIPTION: 
*			This method should show nexthop;
*			 NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_NEXTHOP
*
*	INPUT:
*	 	
*		
*
*  OUTPUT:  
* 			NONE
*
*		RETURN
*			NPD_SUCCESS
*			NPD_FAILURE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_NEXTHOP "show_port_nexthop"

/*****************************************************************************
*	DESCRIPTION: 
*
*	INPUT:
*  
*	OUTPUT:  // in the order as they are appended in the dbus message.		
*
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RUNNING_CONFIG	"show_ethport_runconfig"


/*****************************************************************************
*	DESCRIPTION: 
*		config eth_port transmiting media
*
*	INPUT:
*		uint32 	-  port_index
*		uint32 	-  media 
*  	
*	OUTPUT:  
*
*	REUTURN
*	
*		NPD_SUCCESS
*		OTHER VALUE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA "config_ethport_media"

/*****************************************************************************
*	DESCRIPTION: 
*		get slot/port by port_index
*
*	INPUT:
*		uint32 	-  port_index
*  	
*	OUTPUT:  
*
*	REUTURN
*	
*		NPD_SUCCESS
*		OTHER VALUE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_SLOT_PORT "get_slot_port"

/*****************************************************************************
*	DESCRIPTION: 
*		set slot/port strom control type and value base pps
*
*	INPUT:
*            byte       -  modeType  //config mode,0 means CONFIG mode,1 means ETH-PORT mode
*		byte 	-   slotno     //used in CONFIG 	mode to get eth_g_index
*            byte       -   portno
*           uint32     -   g_index   //used in ETH-PORT mode to get eth_g_index
*           uint32     -   scMode   //pps or bps
*           uint32     -   sctype    //storm control type,dlf/broadcast/multicast
*  	      uint32     -   scvalue  //storm control value
*	OUTPUT:  
*
*	REUTURN
*		NPD_SUCCESS
*		OTHER VALUE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_STORM_CONTROL "storm_control"

/*****************************************************************************
*	DESCRIPTION: 
*		set slot/port strom control type and value base pps
*
*	INPUT:
*            byte       -  modeType  //pps or bps
*	OUTPUT:  
*
*	REUTURN
*		NPD_SUCCESS
*		OTHER VALUE
********************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_STORM_CONTROL_GLOBAL_MODEL "storm_control_model"

/*
 this method should config  port attr to enable RSTP and add port int RSTP programm
*/
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 		byte   	- port_no  // port number on slot
 *		uint32	- isEnable  //enable or disable mstp on the port
 *
 * OUTPUT:
 * 	uint32 - port_index  //port index
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully enable this port
 *						STP_DISABLE - mstp hasn't enabled
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP	 "config_ethport_stp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP;in order to 
 *     bind vlan to stpId;
 *
 * INPUT:
 * 
 *		NONE
 *
 * OUTPUT:
 *
 *		NONE
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP	 "config_all_stp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_LINK_STATE;in order to 
 *    get the port link state;
 *
 * INPUT:
 * 
 *	uint32-	port_index - port index
 *
 * OUTPUT:
 *
 *		up
 *		down
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_LINK_STATE	 "get_stp_portlk_stae"



/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_SPEED;in order to 
 *    get the port link state;
 *
 * INPUT:
 * 
 *	uint32-	port_index - port index
 *
 * OUTPUT:
 *
 *		up
 *		down
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_SPEED	 "get_stp_port_speed"



/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_DUPLEX_MODE;in order to 
 *    get the port link state;
 *
 * INPUT:
 * 
 *	uint32-	port_index - port index
 *
 * OUTPUT:
 *
 *		up
 *		down
 *
 ***********************************************************************/
#define NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_DUPLEX_MODE	 "get_stp_port_duplex_mode"



/*
* mirror part
*/
#define NPD_DBUS_MIRROR_OBJPATH					 "/aw/npd/mirror"
#define NPD_DBUS_MIRROR_INTERFACE				 "aw.npd.mirror"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_MIRROR;in order to 
 *    into mirror mode;
 *
 * INPUT:
 * 		uint32 - profile id
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_CONFIG_MIRROR			 "config_mirror"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_MIRROR_DEST_PORT_CREATE
 *		in order to config destination port
 *    get the port link state;
 *
 * INPUT:
 *    uint32:
 *		port_index - port index
 *		direct   -- ingress or egress
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_MIRROR_DEST_PORT_CREATE		 "config_mirror_dest_port"

#define NPD_DBUS_METHOD_DEBUG_MIRROR_DEST_PORT_CREATE		 "config_debug_mirror_dest_port"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_MIRROR_DEST_PORT_DEL;in order to 
 *    delete mirror destination port
 *
 * INPUT:
 *		byte-slot_no
 *           byte- port_no
 *          uint32-profile
 *          uint32-direct
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_MIRROR_DEST_PORT_DEL		 "del_mirror_dest_port"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL;in order to 
 *    config policy mirror;
 *
 * INPUT:
 *    uint32:
 *		port_index -destination port port index
 *
 * OUTPUT:
 *
 *		up
 *		down
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL   "config_mirror_acl"
#define NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_ACL   "cancel_mirror_acl"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_CREATE;in order to 
 *    config mirror  based on port ;
 *
 * INPUT:
 * 	
 *	byte-	slot 				
 *	byte-	port
 *	uint32-	port_index - destination port index
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_CREATE   "config_mirror_port"
#define NPD_DBUS_METHOD_DEBUG_APPEND_MIRROR_BASE_PORT_CREATE   "config_debug_mirror_port"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_DEL;in order to 
 *    delete mirror based on port;
 *
 * INPUT:
 * 
 *	byte-	slot 				
 *	byte-	port
 *	uint32-	port_index - destination port  index
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_DEL   "del_mirror_port"
#define NPD_DBUS_METHOD_DEBUG_APPEND_MIRROR_BASE_PORT_DEL   "debug_del_mirror_port"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE;in order to 
 *    config mirror based on vlan;
 *
 * INPUT:
 *   uint32- profile
 *	uint16-vid
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE   "config_mirror_vlan"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL;in order to 
 *    delete mirror based on vlan;
 *
 * INPUT:
 *    uint32-profile
 *	uint16-vid
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL   "del_mirror_vlan"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_APPEND_MIRROR_BASE_FDB;in order to 
 *    config mirror based on fdb;
 *
 * INPUT:
 * 
 *    uint16-vid
 *	byte-   slot_no
 *	byte-   port_no
 *	byte-   mac[0]
 *	byte-   mac[1]
 *	byte-   mac[2]
 *	byte-   mac[3]
 *	byte-   mac[4]
 *	byte-   mac[5]
 *	uint32- profile
 *
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_APPEND_MIRROR_BASE_FDB   "config_mirror_fdb"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_FDB;in order to 
 *    delete mirror based on fdb;
 *
 * INPUT:
 * 
 *    uint16-vid
 *	 byte-	 slot_no
 *	 byte-	 port_no
 *	 byte-	 mac[0]
 *	 byte-	 mac[1]
 *	 byte-	 mac[2]
 *	 byte-	 mac[3]
 *	 byte-	 mac[4]
 *	 byte-	 mac[5]
 *	 uint32- profile
 *
 *
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_FDB   "cancel_mirror_fdb"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_MIRROR_SHOW
 *		in order to config destination port
 *    get the port link state;
 *
 * INPUT:
 *        NONE
 * OUTPUT:
 *
 *		FAIL
 *		SUCCESS
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_MIRROR_SHOW		 "show_mirror"


#define NPD_DBUS_METHOD_MIRROR_DELETE    "delete_mirror"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SHOW_RUNNING_CGF;in order to 
 *    save mirror configuration;
 *
 * INPUT:
 * 
 *
 * OUTPUT:
 *
 *		StrShow
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_SHOW_RUNNING_CGF				"show_cfg"

/*
* QOS part
*
*/
#define NPD_DBUS_QOS_OBJPATH  	     			 "/aw/npd/qos"
#define NPD_DBUS_QOS_INTERFACE 		  			 "aw.npd.qos"
#define NPD_DBUS_METHOD_CONFIG_QOS_MODE       "config_qos_mode"
/************************************************************************
 * DESCRIPTION:
 *	config qos mode(flow|port|hybrid)
 *
 * INPUT:
 *		uint32 - qosmode
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QOS_MODE       "show_qos_mode"
/************************************************************************
 * DESCRIPTION:
 *	show qos mode
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - qosmode
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_QOS_PROFILE       "config_qos_profile"
/************************************************************************
 * DESCRIPTION:
 *	config qos profile
 *
 * INPUT:
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_QOS_PROFILE_ATTRIBUTE    "config_qos_profile_attributes"
/************************************************************************
 * DESCRIPTION:
 *	set qos profile dp up tc dscp
 *
 * INPUT:
 *		uint32 - profileIndex
 *		uint32 - dp
 *		uint32 - up
 *		uint32 - tc
 *		uint32 - dscp
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_DSCP_PROFILE_TABLE   "cofig_dscp_to_profile_map_table"
/************************************************************************
 * DESCRIPTION:
 *	set dscp to qos profile
 *
 * INPUT:
 *		uint32 - dscp
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_DSCP_DSCP_TABLE      "config_dscp_to_dscp_map_table"
/************************************************************************
 * DESCRIPTION:
 *	set dscp to dscp
 *
 * INPUT:
 *		uint32 - oldDscp
 *		uint32 - newDscp
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_UP_PROFILE_TABLE     "config_up_to_profile_map_table"
/************************************************************************
 * DESCRIPTION:
 *	set up to qos profile
 *
 * INPUT:
 *		uint32 - up
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_DSCP_PROFILE_TABLE   "delete_dscp_to_profile_map_table"
/************************************************************************
 * DESCRIPTION:
 *	delte dscp to qos profile
 *
 * INPUT:
 *		uint32 - dscp
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_ACL_PROFILE_TABLE   "delete_acl_to_profile_map_table"
/************************************************************************
 * DESCRIPTION:
 *	delete acl append qos profile
 *
 * INPUT:
 *		uint32 - aclIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_DSCP_DSCP_TABLE      "delete_dscp_to_dscp_map_table"
/************************************************************************
 * DESCRIPTION:
 *	delete dscp to dscp 
 *
 * INPUT:
 *		uint32 - oldDscp
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_UP_PROFILE_TABLE     "delete_up_to_profile_map_table"
/************************************************************************
 * DESCRIPTION:
 *	delete up to qos profile
 *
 * INPUT:
 *		uint32 - up
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CREATE_POLICY_MAP		 "create_policy_map"
/************************************************************************
 * DESCRIPTION:
 *	create policy map by policyindex
 *
 * INPUT:
 *		uint32 - policyIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_POLICY_MAP		 "config_policy_map"
/************************************************************************
 * DESCRIPTION:
 *	config policy map
 *
 * INPUT:
 *		uint32 - policyIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_QOS_EGRESS_POLICY_BASE_ON_ACL   "config_egress_qos_base_on_acl"
/************************************************************************
 * DESCRIPTION:
 *	set qos base on acl for egress
 *
 * INPUT:
 *		uint32 - ruleIndex
 *		uint32 - ruleType
 *		uint32 - egrUp
 *		uint32 - egrDscp
 *		uint32 - up
 *		uint32 - dscp
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_QOS_INGRESS_POLICY_BASE_ON_ACL  "config_ingress_qos_base_on_acl"
/************************************************************************
 * DESCRIPTION:
 *	set qos base on acl for ingress
 *
 * INPUT:
 *		uint32 - ruleIndex
 *		uint32 - ruleType
 *		uint32 - profileIndex
 *		uint32 - up
 *		uint32 - dscp
 *		uint32 - policer
 *		uint32 - policerId
 *		uint32 - precedence
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_MODIFY_MARK_QOS         		"allow_qos_mark"
/************************************************************************
 * DESCRIPTION:
 *	set qos-markers (disable|enable)
 *
 * INPUT:
 *		uint32 - IsEnable
 *		uint32 - policyMapIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_PORT_TRUST_L2_MODE     	 	"set_port_trust_mode_l2"
/************************************************************************
 * DESCRIPTION:
 *	set policy map trust mode l2 (enable|disable)
 *
 * INPUT:
 *		uint32 - upEnable
 *		uint32 - policyMapIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_PORT_TRUST_L3_MODE     	 	"set_port_trust_mode_l3"
/************************************************************************
 * DESCRIPTION:
 *	set policy map trust mode l3 (enable|disable) channe dscp (enable|disable)
 *
 * INPUT:
 *		uint32 - dscpEnable
 *		uint32 - dscpRemap
 *		uint32 - policyMapIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_PORT_TRUST_L2_L3_MODE       "set_port_trust_mode_l2_l3"
/************************************************************************
 * DESCRIPTION:
 *	set policy map trust mode l2 (enable|disable) trust mode l3 (enable|disable)
 *	 channe dscp (enable|disable)
 * INPUT:
 *		uint32 - upEnable
 *		uint32 - dscpEnable
 *		uint32 - dscpRemap
 *		uint32 - policyMapIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_PORT_TRUST_UNTRUST_MODE     "set_port_trust_mode_untrust"
#define NPD_DBUS_METHOD_SET_DEFAULT_UP          		 "set_default_up"
#define NPD_DBUS_METHOD_SET_DEFAULT_QOS_PROFILE 		 "set_port_default_qos_profile"

#define NPD_DBUS_ETHPORTS_METHOD_BIND_POLICY_MAP		 "bind_policy_map_to_port"
/************************************************************************
 * DESCRIPTION:
 *	bind policy-map to port
 *
 * INPUT:
 *		uint32 - g_eth_index
 *		uint32 - policyIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_ETHPORT_METHOD_SHOW_POLICY_MAP			 "show_port_polic_map_info"
/************************************************************************
 * DESCRIPTION:
 *	unbind policy-map from port
 *
 * INPUT:
 *		uint32 - g_index
 *
 * OUTPUT:
 *		uint32 - ret 
 *		uint32 - slot_no
 *		uint32 - local_port_no
 *		uint32 - poliyIndex
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_POLICY_MAP                  "show_policy_map"
/************************************************************************
 * DESCRIPTION:
 *	show policy map
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - ret
 *		uint32 - count
 *		uint32 - policyIndex
 *		uint32 - assignPrecedence
 *		uint32 - trustFlag
 *		uint32 - modifyUp
 *		uint32 - modifyDscp
 *		uint32 - remapDscp
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QOS_PROFILE		 		 "show_qos_profile"
/************************************************************************
 * DESCRIPTION:
 *	show qos profile
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - ret
 *		uint32 - count
 *		uint32 - profileIndex
 *		uint32 - TC
 *		uint32 - DP
 *		uint32 - UP
 *		uint32 - DSCP
 *
 ***********************************************************************/

#define NPD_DBUS_ETHPORT_METHOD_SHOW_REMAP_TABLE		 "show_remap_table"
/************************************************************************
 * DESCRIPTION:
 *	show remap table
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - upCount
 *		uint32 - dscpCount
 *		uint32 - dscpReCount
 *		uint32 - countVal
 *		uint32 - specflag
 *		uint32 - profileIndex
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_QOS_PROFILE         "delete_qos_profile"
/************************************************************************
 * DESCRIPTION:
 *	unbind policy-map from port
 *
 * INPUT:
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_POLICY_MAP		   "delete_policy_map"
/************************************************************************
 * DESCRIPTION:
 *	delete policy map
 *
 * INPUT:
 *		uint32 - policyIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_ETHPORTS_METHOD_UNBIND_POLICY_MAP  "delete_policy_map_on_port" 
/************************************************************************
 * DESCRIPTION:
 *	unbind policy-map from port
 *
 * INPUT:
 *	    uint32 - g_eth_index
 *		uint32 - policyIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/
 
#define NPD_DBUS_METHOD_POLICER_ENABLE			"enable_policer"
/************************************************************************
 * DESCRIPTION:
 *	set policer (enable|disable)
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - IsEnable
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_POLICER			"config_policer"
/************************************************************************
 * DESCRIPTION:
 *	config policer by policer index
 *
 * INPUT:
 *	    uint32 - policerIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_POLICER_RANGE	"config_policer_range"
/************************************************************************
 * DESCRIPTION:
 *	config policer range from startpid to endpid 
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - startPid
 *		uint32 - endPid
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_CIR_CBS		    "config_policer_cir_cbs"
/************************************************************************
 * DESCRIPTION:
 *	set cir cbs of policer 
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - cir
 *		uint32 - cbs
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_OUT_PROFILE	    "config_policer_out_profile"
/************************************************************************
 * DESCRIPTION:
 *	config policer out profile
 *
 * INPUT:
 *	    uint32 - policerIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_OUT_PROFILE_DROP_KEEP   "config_policer_out_action"
/************************************************************************
 * DESCRIPTION:
 *	set policer out profile (drop|keep) 
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - action
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_OUT_PROFILE_REMAP	    "config_policer_out_action_remap"
/************************************************************************
 * DESCRIPTION:
 *	set policer out profile remap
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_GLOBAL_METER_MODE		"config_global_meter_mode"
/************************************************************************
 * DESCRIPTION:
 *	set global policer mode of meter
 *
 * INPUT:
 *	    uint32 - mode
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_GLOBAL_MRU				"config_global_mru"
/************************************************************************
 * DESCRIPTION:
 *	set global policer mode of packet mru
 *
 * INPUT:
 *	    uint32 - mode
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_GLOBAL_PACKET_SIZE		"config_global_policing_mode"
/************************************************************************
 * DESCRIPTION:
 *	set global policer mode of packet size
 *
 * INPUT:
 *	    uint32 - mode
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SET_COUNTER				"config_counter"
/************************************************************************
 * DESCRIPTION:
 *	set policer counter
 *
 * INPUT:
 *	    uint32 - counterIndex
 *		uint32 - Inprofile
 *		uint32 - Outprofile
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_POLICER_COUNTER			"set_counter_for_policer"
#define NPD_DBUS_METHOD_POLICER_SHARE			"set_share_for_policer"

/************************************************************************
 * DESCRIPTION:
 *	set policer counter enable or disable
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - counterIndex
 *		uint32 - IsEnable
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_GET_COUNTER				"get_counter_info"
/************************************************************************
 * DESCRIPTION:
 *	read policer counter
 *
 * INPUT:
 *	    uint32 - counterIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *		uint32 - Inprofile
 *		uint32 - Outprofile
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_POLICER_COLOR			"set_policer_color"
/************************************************************************
 * DESCRIPTION:
 *	set policer color
 *
 * INPUT:
 *	    uint32 - policerIndex
 *		uint32 - color	   
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_POLICER		    "show_policer_info"
/************************************************************************
 * DESCRIPTION:
 * 	show all policer
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_POLICER			"delete_policer"
/************************************************************************
 * DESCRIPTION:
 *	delete policer by index
 *
 * INPUT:
 *	   uint32 - policerIndex
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_POLICER_RANGE	"delete_policer_range"
/************************************************************************
 * DESCRIPTION:
 *	delete policer-range
 *
 * INPUT:
 *	   uint32 - startPid
 *	   uint32 - endPid
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_REMAP_TABLE_RUNNIG_CONFIG "show_remap_table_running"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for remap table
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_POLICY_MAP_RUNNIG_CONFIG  "show_policy_map_running"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for policy-map
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QOS_PROFILE_RUNNIG_CONFIG "show_qos_profile_running"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for qos-profile
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QOS_COUNTER_RUNNIG_CONFIG "show_qos_counter_running"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for qos counter
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QUEUE_SCH_RUNNIG_CONFIG   "show_qos_queue_scheduing"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for queue scheduler
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QOS_MODE_RUNNIG_CONFIG   "show_qos_mode_running"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for qos-mode config
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_POLICER_RUNNIG_CONFIG     "show_qos_policer"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for policer config
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_QUEQUE_WRR_GROUP		"set_wrr_group_weight"
/************************************************************************
 * DESCRIPTION:
 *	set queue scheduler wrr group
 *
 * INPUT:
 *	   uint32 - wrrflag
 *	   uint32 - groupFlag
 *	   uint32 - tc
 *	   uint32 - weight
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_QUEQUE_SCH			     "set_queue_alogrithm"
/************************************************************************
 * DESCRIPTION:
 *	set queue scheduler mode
 *
 * INPUT:
 *		uint32 - algFlag
 *
 * OUTPUT:
 *		uint32 - ret
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_QUEUE				 "show_queue_sheduler"
/************************************************************************
 * DESCRIPTION:
 * 	show queue scheduler
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - algFlag
 *		uint32 - groupFlag
 *		uint32 - weight
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_TRAFFIC_SHAPE		    "set_traffic_shape_port"
/************************************************************************
 * DESCRIPTION:
 * 	set traffic shape
 *
 * INPUT:
 *		uint32 - g_eth_index
 *		uint32 - algFlag
 *		uint32 - queueId
 *		uint32 - maxrate
 *		uint32 - kmstate
 *		uint32 - burst
 *
 * OUTPUT:
 *		uint32 - ret	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_TRAFFIC			"show_traffic_shape_port"
/************************************************************************
 * DESCRIPTION:
 * 	delete traffic shape
 *
 * INPUT:
 *		uint32 - g_eth_index
 *		uint32 - algFlag
 *		uint32 - queueId
 *
 * OUTPUT:
 *		uint32 - ret	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_TRAFFIC			"delete_traffic_shape_port"
/************************************************************************
 * DESCRIPTION:
 * 	set acl append qos-profile
 *
 * INPUT:
 *		uint32 - g_eth_index
 *		uint32 - algFlag
 *		uint32 - queueId
 *
 * OUTPUT:
 *		uint32 - ret	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_APPEND_QOS_MARK_BASE_ACL	"append_qos_base_acl"
/************************************************************************
 * DESCRIPTION:
 * 	set acl append qos-profile
 *
 * INPUT:
 *		uint32 - ruleIndex
 *		uint32 - profileIndex
 *
 * OUTPUT:
 *		uint32 - ret	
 *
 ***********************************************************************/

#define NPD_DBUS_SHOW_APPEND_QOS_MARK_BASE_ACL "show_append_qos_base_acl"
/************************************************************************
 * DESCRIPTION:
 * 	show acl append qos-profile
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		uint32 - ret	
 *		uint32 - count
 *		uint32 - aclIndex
 *		uint32 - profileIndex 
 *
 ***********************************************************************/

/* 
 *  ACL part
 *
 */
#define NPD_DBUS_ACL_OBJPATH 			"/aw/npd/acl"
#define NPD_DBUS_ACL_INTERFACE 			"aw.npd.acl"

#define NPD_DBUS_METHOD_SHOW_ACL_GROUP_RUNNIG_CONFIG  "show_running_config_acl_group"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for acl group
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_ACL_QOS_RUNNIG_CONFIG 	"show_running_config_acl_qos"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for qos-acl rule
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_ACL_RULE_RUNNIG_CONFIG  "show_running_config_acl_rule"
/************************************************************************
 * DESCRIPTION:
 * 	showrunning for common acl 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_ACL_GROUP_INDEX		  "show_acl_group_one"
/************************************************************************
 * DESCRIPTION:
 * 	show acl group index, index is group number
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_ACL_INDEX				  "show_acl_one"
/************************************************************************
 * DESCRIPTION:
 * 	show acl rule by acl index
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		showStr	
 *
 ***********************************************************************/

#define NPD_DBUS_ACL_METHOD_SET_TIME_RANGE	"set_time_range"
#define NPD_DBUS_ACL_METHOD_SET_ABSOLUTE    "set_absolute_time"
#define NPD_DBUS_ACL_METHOD_SET_PERIODIC	"set_periodic_time"
#define NPD_DBUS_ACL_METHOD_SHOW_TIME_RANGE "show_time_range"
#define NPD_DBUS_ACL_METHOD_ACL_TIME_RANGE	"acl_time_range"

#define NPD_DBUS_METHOD_CONFIG_ACL 		"config_acl_enable"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL
 *
 * INPUT:
 *	byte   - isEnable   // enable or disable global acl service on device
 *
 * OUTPUT:
 *	uint32 - op_ret     //NPD_DBUS_SUCCESS - successfully set global acl service
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_IP                     "config_acl_rule_trap_ip"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ICMP
 *	trap to cpu with icmp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  byte    - typeno      // icmp type
 *  byte    - codeno	  // icmp code
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ICMP                   "config_acl_rule_trap_icmp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_TCP
 *	trap to cpu with tcp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  uint32  - dstport     // tcp destination port
 *  uint32  - srcport	  // tcp source port
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/


#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_TCP                    "config_acl_rule_trap_tcp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_UDP
 *	trap to cpu with udp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  uint32  - dstport     // udp destination port
 *  uint32  - srcport	  // udp source port
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_UDP                    "config_acl_rule_trap_udp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_MAC
 *	trap to cpu with just about mac packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *	byte    - dmacAddr.arEther[0]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i]  (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_MAC                    "config_acl_rule_trap_layer2_packet"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ARP
 *	trap to cpu with arp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *	byte    - dmacAddr.arEther[0]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  uint32  - vlanId				// vlan id
 * 	uint32  - slot_no 				// packets from which solt number
 *  uint32  - port_no               // packets from which port number
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 *	uint32   - eth_ret    //NPD_DBUS_SUCCESS   - ok
 *						  //NPD_ERROR_NO_SUCH_PORT -port error
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ARP                    "config_acl_rule_trap_arp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_PERMIT_DENY_ARP
 *	permit or deny with arp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruleType    			// acl rule type (standard or extended)
 *	uint32  - actionType			// acl action type 0 permit,1 deny
 *	byte    - dmacAddr.arEther[0]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  uint32  - vlanId				// vlan id
 * 	uint32  - slot_no 				// packets from which solt number
 *  uint32  - port_no               // packets from which port number
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 *	uint32   - eth_ret    //NPD_DBUS_SUCCESS   - ok
 *						  //NPD_ERROR_NO_SUCH_PORT -port error
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_PERMIT_DENY_ARP             "config_acl_rule_permit_deny_arp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_REDIRECT_ARP
 *	mirror to analyzer or redirect with arp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruleType    			// acl rule type (standard or extended)
 *	uint32  - actionType			// acl action type 3 mirror to annayzer ,4 redirect
 *	byte    - dmacAddr.arEther[0]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  uint32  - vlanId				// vlan id
 * 	uint32  - slot_no 				// packets from which solt number
 *  uint32  - port_no               // packets from which port number
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 *	uint32   - eth_ret    //NPD_DBUS_SUCCESS   - ok
 *						  //NPD_ERROR_NO_SUCH_PORT -port error
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_REDIRECT_ARP         "config_acl_rule_mirror_redirect_arp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_TCP_OR_UDP
 *	permit or deny with tcp or udp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  uint32  - dstport     // tcp or udp destination port
 *  uint32  - srcport	  // tcp or udp source port
 *  uint32  - actionType  // peimit 0 ,deny 1
 *  uint32  - packetType  // udp 1,tcp 2
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_TCP_OR_UDP             "config_acl_rule_deny_tcp_or_udp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_MAC
 *	permit or deny with just about mac packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *  uint32  - actiontype			// permit 0,deny 1
 *	byte    - dmacAddr.arEther[0]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i]  (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_MAC                    "config_acl_rule_deny_mac"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IP
 *	permit or deny with matched ip packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *  uint32  - actionType  // permit 0, deny 1
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IP	                    "config_acl_rule_deny_ip"
#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IPV6                    "config_acl_rule_deny_ipv6"

/************************************************************************
 * DESCRIPTION:
 * 	method NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_IPV6
 *	redirect to another port  with matched ip packets
 *
 * INPUT:
 *	DBUS_TYPE_UINT32, &ruleIndex,	
 *	DBUS_TYPE_UINT32, &ruleType,
 *	DBUS_TYPE_UINT32, &actionType,
 *	DBUS_TYPE_UINT32, &nextheader,
 *	DBUS_TYPE_UINT32, &sipmaskLen,
 *	DBUS_TYPE_UINT32, &dipmaskLen,
 *   DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[15]),	
 *	DBUS_TYPE_UINT32, &policer, 
 *	DBUS_TYPE_UINT32, &policerId,

 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/
#define NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_IPV6			      "config_acl_redirect_ipv6"

/************************************************************************
 * DESCRIPTION:
 * 	method NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_IPV6
 *	redirect to another port  with matched tcp ro udp packets
 *
 * INPUT:
 *	DBUS_TYPE_UINT32, &ruleIndex,	
 *	DBUS_TYPE_UINT32, &ruleType,
 *	DBUS_TYPE_UINT32, &actionType,
 *	DBUS_TYPE_UINT32, &nextheader,
 *	DBUS_TYPE_UINT32, &sipmaskLen,
 *	DBUS_TYPE_UINT32, &dipmaskLen,
 *	DBUS_TYPE_UINT32, &dstport,
 *	DBUS_TYPE_UINT32, &srcport,
 *   DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
 *	DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
 *	DBUS_TYPE_BYTE, &(sip.ipbuf[15]),	
 *	DBUS_TYPE_UINT32, &policer, 
 *	DBUS_TYPE_UINT32, &policerId,

 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_TCP_UDP_IPV6		"config_acl_redirect_tcp_udp_ipv6"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IP
 *	permit or deny with matched ip packets
 *
 * INPUT:
 *	uint32  - startindex   // acl rule index
 *	uint32  - endindex   // acl rule index
 *  uint32  - startdip       // dip value 
 *  uint32  - startsip	  // sip value
  *  uint32  - enddip       // dip value 
 *  uint32  - endsip	  // sip value
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_IP_RANGE				"config_acl_rule_ip_range"
#define NPD_DBUS_METHOD_CONFIG_ACL_PERMIT_RULE_IP_RANGE			"config_acl_permit_rule_ip_range"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_ICMP
 *	permit or deny with icmp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *  uint32  - actionType  // permit 0, deny 1
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  byte    - typeno      // icmp type
 *  byte    - codeno	  // icmp code
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/


#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_ICMP					"config_acl_rule_deny_icmp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_IP
 *	mirror to analyzer or redirect with matched ip packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *  uint32  - actionType  // mirror to analyzer 3, redirect 4
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  byte    - slot_no     // mirror to analyzer or redirect slot number
 *  byte    - port_no     // mirror to analyzer or redirect port number
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_IP       "config_acl_rule_mirror_or_redirect_ip"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_TCP_UDP
 *	mirror to analyzer or redirect  with tcp or udp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  uint32  - dstport     // tcp or udp destination port
 *  uint32  - srcport	  // tcp or udp source port
 *  uint32  - actionType  // mirror to analyzer 3, redirect 4
 *  uint32  - packetType  // udp 1,tcp 2
 *  byte    - slot_no     // mirror to analyzer or redirect slot number
 *  byte    - port_no     // mirror to analyzer or redirect port number
 *
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_TCP_UDP  "config_acl_rule_mirror_or_redirect_tcp_udp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_ICMP
 *	mirror to analyzer or redirect with icmp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   // acl rule index
 *	uint32  - ruletype    // acl rule type (standard or extended)
 *  uint32  - actionType  // mirror to analyzer 3, redirect 4
 *	uint32  - sipmaskLen  // sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  // dip mask (8,16,24,32)
 *  uint32  - dipno       // dip value 
 *  uint32  - sipno		  // sip value
 *  byte    - typeno      // icmp type
 *  byte    - codeno	  // icmp code
 *  byte    - slot_no     // mirror to analyzer or redirect slot number
 *  byte    - port_no     // mirror to analyzer or redirect port number
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_ICMP     "config_acl_rule_mirror_or_redirect_icmp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_MAC
 *	mirror to analyzer or redirect with just about mac packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *  uint32  - actiontype			// mirror to analyzer 3, redirect 4
 *	byte    - dmacAddr.arEther[i]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  byte    - slot_no     // mirror to analyzer or redirect slot number
 *  byte    - port_no     // mirror to analyzer or redirect port number
 *
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_MAC      "config_acl_rule_mirror_or_redirect_mac"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_PERMIT_DENY_TRAP_TCP_UDP
 *	permit,deny,trap to cpu with just about tcp,udp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *  uint32  - actiontype			// permit 0,deny 1,trap to cpu 2]
 *  uint32  - packetype				// tcp 2,udp 1
 *  uint32  - dipno       			// dip value 
 *  uint32  - sipno		  			// sip value
 *	uint32  - sipmaskLen  			// sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  			// dip mask (8,16,24,32)
 *  uint32  - dstport     			// tcp or udp destination port
 *  uint32  - srcport	  			// tcp or udp source port
 *	byte    - dmacAddr.arEther[i]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  uint32  - vlanId				//vlan id
 *  byte    - dataslot     			// source slot number
 *  byte    - dataport     			// source port number
 *
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_EXT_NO_SPACE   - extend rule take up the spaces
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_PERMIT_DENY_TRAP_TCP_UDP "config_acl_rule_extend_permit_tcp"
#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_PERMIT_DENY_TCP_UDP_IPV6 "config_acl_rule_extend_permit_tcp_ipv6"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_MIRROR_REDIRECT_TCP_UDP
 *	mirror to analyzer or redirect with just about tcp,udp packets
 *
 * INPUT:
 *	uint32  - ruleIndex   			// acl rule index
 *	uint32  - ruletype    			// acl rule type (standard or extended)
 *  uint32  - actiontype			// mirror to analyzer 3, redirect 4
 *	byte	- slot_no	  			// mirror to analyzer or redirect slot number
 *  byte	- port_no	   			// mirror to analyzer or redirect port number
 *  uint32  - packetype				// tcp 2,udp 1
 *  uint32  - dipno       			// dip value 
 *  uint32  - sipno		  			// sip value
 *	uint32  - sipmaskLen  			// sip mask (8,16,24,32)
 *  uint32  - dipmaskLen  			// dip mask (8,16,24,32)
 *  uint32  - dstport     			// tcp or udp destination port
 *  uint32  - srcport	  			// tcp or udp source port
 *	byte    - dmacAddr.arEther[i]   // destination MAC[i] (i=0,1,2,3,4,5)
 *	byte    - smacAddr.arEther[0]   // source MAC[i] (i=0,1,2,3,4,5)
 *	byte    - maskMac.arEther[0]    // mask with SMAC[i] or DMAC[i] (i=0,1,2,3,4,5)
 *  uint32  - vlanId				//vlan id
 *  byte    - dataslot     			// source slot number
 *  byte    - dataport     			// source port number
 *
 *
 * OUTPUT:
 *	uint32   - op_ret     //NPD_DBUS_SUCCESS   - successfully set the acl rule
 *						  //ACL_GLOBAL_EXISTED - has set this acl rule yet
 *						  //ACL_EXT_NO_SPACE   - extend rule take up the spaces
 *						  //ACL_SAME_FIELD     - set same contents with same packet
 *						  //other value		   - set the acl rule fail
 ***********************************************************************/

#define NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_MIRROR_REDIRECT_TCP_UDP "config_acl_rule_extend_redirect_tcp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CREATE_ACL_GROUP
 *
 * INPUT:
 *	uint32   - groupNum   // cerate acl group number
 *
 * OUTPUT:
 *	uint32 - op_ret     //NPD_DBUS_SUCCESS - successfully create acl group
 *						//NPD_DBUS_ERROR   - acl group existed
 *
 ***********************************************************************/
#define NPD_DBUS_METHOD_CREATE_ACL_GROUP  	"create_acl_group"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_DELETE_ACL_GROUP
 *
 * INPUT:
 *	uint32  - groupNum       // delete acl group number
 *
 * OUTPUT:
 *	uint32  - groupInfo      //ACL_GROUP_PORT_BINDED - acl group has been binded on port or vlan,can not config 
 *						     //NPD_DBUS_ERROR        - acl group not existed
 *	uint32  - op_ret         //NPD_DBUS_ERROR 		 - delete acl group fail
 *							 //NPD_DBUS_SUCCESS		 - successfully delete acl group
 *
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_ACL_GROUP    "delete_acl_group"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_CONFIG_ACL_GROUP
 *
 * INPUT:
 *	uint32  - groupNum       // config acl group which has been created
 *
 * OUTPUT:	
 *	uint32  - op_ret         //ACL_GROUP_NOT_EXISTED   -  acl group not existed
 *							 //ACL_GROUP_SUCCESS	   -  successfully enter acl group configuration mode
 *							 //ACL_GROUP_PORT_BINDED   -  acl group has been bind to vlan or port ,so cancel bind relationship ,then can config acl group
 ***********************************************************************/
#define NPD_DBUS_METHOD_CONFIG_ACL_GROUP    "config_acl_group"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_ADD_ACL_TO_GROUP
 *
 * INPUT:
 *	uint32  - op_flag       // add 0,delete 1
 *  uint32  - acl_group_num // add or delete acl rule in acl group index
 *	uint32  - ruleIndex     // add or delete acl rule index 
 *
 * OUTPUT:	
 *	uint32  - group_inf     // NPD_DBUS_ERROR acl group not existed
 *	uint32  - num			// group index
 *  uint32  - ret           // add: 
 *								ACL_GROUP_RULE_EXISTED  -the acl rule has been added in group 
 *								ACL_GLOBAL_NOT_EXISTED  -acl rule has not set
 *							   delete:
 *							  	ACL_GLOBAL_NOT_EXISTED  -acl rule has not set
 *							  	NPD_DBUS_SUCCESS   		-group has no this acl rule
 *  uint32  - op_info		 //	NPD_DBUS_ERROR		    -add or delete acl rule to/from group fail							    
 *							  
 *	
 ***********************************************************************/

#define NPD_DBUS_METHOD_ADD_ACL_TO_GROUP    "add_acl_to_group"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_ADD_ACL_TO_GROUP
 *
 * INPUT:
 *	uint32  - op_flag       // add 0,delete 1
 *  uint32  - acl_group_num // add or delete acl rule in acl group index
 *	uint32  - startindex     // add or delete acl rule index 
 *	uint32  - endindex     // add or delete acl rule index 
 *
 * OUTPUT:	
 *	uint32  - group_inf     // NPD_DBUS_ERROR acl group not existed
 *	uint32  - num			// group index
 *  uint32  - ret           // add: 
 *								ACL_GROUP_RULE_EXISTED  -the acl rule has been added in group 
 *								ACL_GLOBAL_NOT_EXISTED  -acl rule has not set
 *							   delete:
 *							  	ACL_GLOBAL_NOT_EXISTED  -acl rule has not set
 *							  	NPD_DBUS_SUCCESS   		-group has no this acl rule
 *  uint32  - op_info		 //	NPD_DBUS_ERROR		    -add or delete acl rule to/from group fail							    
 *							  
 *	
 ***********************************************************************/
#define NPD_DBUS_METHOD_ADD_ACL_RANGE_TO_GROUP	 "add_acl_range_to_group"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SHOW_ACL_GROUP
 *
 * INPUT:
 * 	 NULL
 *
 * OUTPUT:	
 *	uint32  - ret            // NPD_DBUS_ERROR   no acl group information
 *					         // NPD_DBUS_SUCCESS existed acl group
 *  uint32  - group_count    // num of acl group 
 *	Array of slot information
 *	   uint32  - group_num	 //acl group index
 *	   uint32  - count       //nums of acl rules in group <group_num>
 *	   uint32  - index       //acl rule index in group <group_num>
***********************************************************************/

#define NPD_DBUS_METHOD_SHOW_ACL_GROUP	    "show_acl_group"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SHOW_ACL_SERVICE
 *
 * INPUT:
 * 	 NULL
 *
 * OUTPUT:	
 *	uint32  - ret            // 1   no any acl rule information
 *					         // 0   existed acl rule
 *  uint32  - group_count    // num of acl group 
 *	Array of acl rule information
 *	   uint32  - ruleIndex	 //acl rule index
 *	   uint32  - actionType  //acl rule action
 *	   uint32  - packetType  //acl rule packte type
 *	   uint32  - dip		 //dip
 *	   uint32  - maskdip     //mask of dip
 *	   uint32  - sip         //sip
 *	   uint32  - masksip     //mask of sip
 *	   uint32  - dstport	 //destination port
 *     uint32  - srcport	 //source port
 *	   byte	   - icmp_code   //icmp code
 *	   byte	   - icmp_type	 //icmp type
 *	   byte    - dmac[i]     //destination MAC (i=0,1,2,3,4,5)
 *	   byte    - smac[i]	 //source MAC  (i=0,1,2,3,4,5)
 *	   uint32  - vlanid	     //vlan id 
 *	   byte    - sourceport  //port which packets comes from
 *	   byte	   - redirectport //redirect port
 *     byte    - mirrorport   //mirror to analyzer port
***********************************************************************/
#define NPD_DBUS_METHOD_SHOW_ACL         	"show_access_lists"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SHOW_ACL_SERVICE
 *
 * INPUT:
 * 	 NULL
 *
 * OUTPUT:	
 *	uint32  - Isable            // ACL_TRUE  -globle acl service enabled
 *								// ACL_FALSE -global acl service disabled
 ***********************************************************************/


#define NPD_DBUS_METHOD_SHOW_ACL_SERVICE 	"show_acl_service"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_DELETE_ACL
 *
 * INPUT:
 * 	 uint32 -ruleIndex  		//acl rule index that will be deleted
 *
 * OUTPUT:	
 *	uint32  - op_ret            // NPD_DBUS_SUCCESS		   - delete acl rule successfully
 *								// NPD_DBUS_ERROR          -acl rule range error
 *								// ACL_GLOBAL_NOT_EXISTED  - acl rule <ruleIndex> not existed
 *								// ACL_GROUP_RULE_EXISTED  -acl rule has been added into group
 *								// ACL_GROUP_PORT_BINDED   -acl rule has been added to group ,and \
 *											                group has been binded to vlan or port
 *	uint32  - group_num 		//group which include the acl rule
 *	uint32  - vlanId			//group if binded to some vlan ,reback vlan id
 *  Array of port information	//if acl rule in group ,and group bind to port
 *		byte - slot_no			//slot number which bind the acl rule by group
 *		byte - local_port_no    //port number which bind the acl rule by group
 ***********************************************************************/

#define NPD_DBUS_METHOD_DELETE_ACL		 	"delete_acl"
#define NPD_DBUS_METHOD_DELETE_ACL_RANGE		 	"delete_acl_range"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_CONFIG_ACL
 *
 * INPUT:
 * 	 uint32 - node_flag  		//port mode 0,vlan mode 1
 *	 uint32 - g_index			//port index or vlan index
 *	 uint32 - isEnable			//port enable acl service or vlan enable acl service
 * 
 * OUTPUT:	
 *	uint32  - op_ret            // NPD_DBUS_ERROR                  -port/vlan enable/disable acl fail
 *								   NPD_DBUS_SUCCESS			      - vlan/port enable/disable acl successfully
 *	uint32  - temp_info         // port mode :
 *									NPD_DBUS_ERROR_NO_SUCH_PORT    - port info error
 *								   vlan mode:
 *								   	NPD_DBUS_ERROR_NO_SUCH_VLAN +1 - vlan illegal
 *								   	NPD_VLAN_NOTEXISTS			   - vlan not existed
 *								   	NPD_DBUS_ERROR_NO_SUCH_PORT    - port in vlan info not existed
 *								   								   	
 ***********************************************************************/

#define NPD_DBUS_ETHPORTS_METHOD_CONFIG_ACL 	  "config_eth_acl_enable"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_SHOW_BIND_ACL
 *
 * INPUT:
 * 	 uint32 - node_flag  		//port mode 0,vlan mode 1
 *	 uint32 - g_index			//port index or vlan index
 * 
 * OUTPUT:	
 *	 port mode:
 *		uint32 - slot_no        //slot number
 *		uint32 - local_port_no  //port number
 *		uint32 - group_num		//group index which bind to the port
 *		uint32 - count			//amount of acl rules in group
 *		array of acl rule of group <group_num> index
 *			uint32 - ruleIndex  //acl rule index
 *	 vlan mode:
 *		uint32 - group_num		////group index which bind to the vlan
 *		uint32 - count			//amount of acl rules in group
 *		array of acl rule of group <group_num> index
 *			uint32 - ruleIndex  //acl rule index
 ***********************************************************************/

#define NPD_DBUS_ETHPORTS_METHOD_SHOW_BIND_ACL    "show_ethport_acl_info"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_BIND_ACL_GROUP
 *
 * INPUT:
 * 	 uint32 - node_flag  		//port mode 0,vlan mode 1
 *	 uint32 - g_index			//port index or vlan index
 *   uint32 - group_num			//group index which will bind to port/vlan
 * OUTPUT:	 
 *	uint32 - sw_info         //port mode:
 *							    NPD_DBUS_ERROR_NO_SUCH_PORT  - port info error
 *								ACL_GROUP_NOT_EXISTED		 - group not exists
 *								ACL_GROUP_PORT_BINDED		 - port bind group yet
 *								NPD_DBUS_ERROR				 - port bind fail
 *							 //vlan mode:	
 *								 NPD_DBUS_ERROR_NO_SUCH_VLAN +1	-illegal vlan
 *								 NPD_VLAN_NOTEXISTS				-vlan not exists
 *								 ACL_GROUP_VLAN_BINDED			-vlan bind group yet
 *								 ACL_GROUP_NOT_EXISTED			-goup not existed
 *								 NPD_DBUS_ERROR					-vlan bind fail
 *  uint32 - IsEnable		  //port/vlan mode:
 *								 NPD_DBUS_ERROR				 - acl has not enabled on this port/vlan
 *								 NPD_DBUS_SUCCESS 			 - acl has enabled on this port/vlan
 *								
 *	uint32 - cfg_info		  //port/vlan mode:
 *								ACL_GROUP_SUCCESS	  -	cfg table set success
 *								ACL_GROUP_ERROR_NONE  -  cfg table fail
 *		
 ***********************************************************************/

#define NPD_DBUS_ETHPORTS_METHOD_BIND_ACL_GROUP	  "bind_acl_group_to_port"	
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_DELETE_ACL_GROUP
 *
 * INPUT:
 * 	 uint32 - node_flag  		//port mode 0,vlan mode 1
 *	 uint32 - g_index			//port index or vlan index
 *   uint32 - group_num			//group index which will unbind to port/vlan
 * OUTPUT:	 
 *	uint32 - port_info         //port mode:
 *								 NPD_DBUS_ERROR_NO_SUCH_PORT      - port info error
 *								vlan mode:								
 *								  NPD_VLAN_NOTEXISTS			  - vlan not existed
 *  uint32 - IsEnable		  //port/vlan mode:
 *								 NPD_DBUS_ERROR				 - acl has not enabled on this port/vlan
 *								 NPD_DBUS_SUCCESS 			 - acl has enabled on this port/vlan
 *								
 *	uint32 - group_info		  //port/vlan mode:
 *								 ACL_GROUP_NOT_EXISTED       - group <group_num> not existed
 *								 ACL_GROUP_RULE_NOTEXISTED   - port/vlan has no such group
 *								 ACL_GROUP_PORT_BINDED  	 - port/vlan has the group <group_num>
 *							
 ***********************************************************************/

#define NPD_DBUS_ETHPORTS_METHOD_DELETE_ACL_GROUP "delete_acl_group_on_port"

/**********************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG
 *
 * INPUT:
 *	 uint32 - g_index			//port index or vlan index
 *  	 byte - port_ipg			//port inter-packet-gap
 * OUTPUT:	 
 *	uint32 - ret         			// indicate suceess or not
 *******************************************************************************/
#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG    "config_eth_ipg"

#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_IPG   "show_eth_ipg"

#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_RATE   "get_eth_port_rate"


/* 
 * RSTP part
 *
 */
#define NPD_DBUS_RSTP_OBJPATH     "/aw/npdrstp"
#define  NPD_DBUS_RSTP_INTERFACE  "aw.npdrstp"
#define NPD_DBUS_RSTP_NAME        "aw.npdrdtp" 

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_GET_PORT_INDEX
 *
 * INPUT:
 *		byte   	- slot_no    // slot number
 * 		byte   	- port_no  // port number on slot
 *
 * OUTPUT:
 * 		uint32 - port_index  //port index
 *		uint32 - op_ret     
 *						NPD_DBUS_SUCCESS - successfully get global index
 *
 ***********************************************************************/
#define STP_DBUS_METHOD_GET_PORT_INDEX "brg_port_index"


/*****************************************************************************
* DESCRIPTION:
*		This method should also work for box product
*		If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*		arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		byte total_slot_count    // 1 for box, more for chassis
*		Array of ethport information of each slotslot
*			byte slot no
*			byte total_port_count
*			Array of eth port information of this slot
*				byte port no
*				uint32 port_index		
********************************************************************************/
#define STP_DBUS_METHOD_GET_ALL_PORTS_INDEX "br_all_ports_index"
/*****************************************************************************
* DESCRIPTION:
*		This method should also work for box product
*		If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*		arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		byte total_slot_count    // 1 for box, more for chassis
*		Array of ethport information of each slotslot
*			byte slot no
*			byte total_port_count
*			Array of eth port information of this slot
*				byte port no
*				uint32 port_index		
********************************************************************************/
#define STP_DBUS_METHOD_GET_ALL_PORTS_INDEX_V1 "br_all_ports_index_v1"


/*****************************************************************************
* DESCRIPTION:
*		This method should also work for box product
*		If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
*		arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST
*	
*	INPUT:
*           uint32--port gloable index
*	OUTPUT: 
*		byte-slot no
*		byte-port no
*			
*			
********************************************************************************/
#define STP_DBUS_METHOD_GET_SLOTS_PORTS_FROM_INDEX "br_slot_port_index"

/*****************************************************************************
* DESCRIPTION:
*		This method get broad product id
*
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		
*		product id
*		op_ret    	- success / fail
*
********************************************************************************/
#define STP_DBUS_METHOD_GET_BROAD_TYPE "product_info"

/*****************************************************************************
* DESCRIPTION:
*		This method obtain stp function support
*
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		
*		NONE
*
********************************************************************************/
#define STP_DBUS_METHOD_FUNCTION_SUPPORT "stp_support"

/*****************************************************************************
* DESCRIPTION:
*		This method get support dldp function
*
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		
*		product id
*		op_ret    	- success / fail
*
********************************************************************************/
#define DLDP_DBUS_METHOD_GET_SUPPORT_FUNCTION "dldp_support_function"

/*****************************************************************************
* DESCRIPTION:
*		This method get support igmp function
*
*	
*	INPUT:
*	 	NONE
*  
*	OUTPUT: 
*		
*		product id
*		op_ret    	- success / fail
*
********************************************************************************/
#define IGMP_DBUS_METHOD_GET_SUPPORT_FUNCTION "igmp_support_function"


#define RSTP_DBUS_NAME					       "aw.stp"
#define RSTP_DBUS_OBJPATH				       "/aw/stp"
#define RSTP_DBUS_INTERFACE			           "aw.stp"
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_STPM_ENABLE
 *
 * INPUT:
 *
 *		uint32 - isEnable //enable or disable mstp
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     //STP_OK - successfully enbale or disable mstp
 *									 //oter  - failed 	
 *		uint32 - mode		// current stp mode stp/mst
 ***********************************************************************/
#define RSTP_DBUS_METHOD_GET_PROTOCOL_STATE	"stp_state"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_STPM_ENABLE
 *
 * INPUT:
 *
 *		uint32 - isEnable //enable or disable mstp
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     //STP_OK - successfully enbale or disable mstp
 *									 //oter  - failed 	
 ***********************************************************************/
#define RSTP_DBUS_METHOD_STPM_ENABLE	"stp_en"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_INIT_MSTP,this is a API,init cist bridge,add courrent all
 *		vlan on the mst instance.
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		
 *   	STP_OK       
 *		OTHER   
 ***********************************************************************/
#define STP_DBUS_METHOD_INIT_MSTP 			"mstp_init"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_INIT_MSTP,this is a API,init cist bridge,add courrent all
 *		vlan on the mst instance.
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		
 *   	STP_OK       
 *		OTHER   
 ***********************************************************************/
#define STP_DBUS_METHOD_INIT_MSTP_V1 			"mstp_init_v1"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method STP_DBUS_METHOD_PORT_STP_ENABLE
 *
 * INPUT:
 *
 *		byte    - slot_no    // slot number
 * 	byte    - port_no  // port number on slot
 *		uint32 - isEnable //enable or disable mstp
 *
 * OUTPUT:
 *
 *		uint32 - op_ret     //STP_OK - successfully enbale or disable mstp
 *									 //oter  - failed 	
 ***********************************************************************/
#define RSTP_DBUS_METHOD_PORT_STP_ENABLE "stp_port_en"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_CFG_VLAN_ON_MST
 *
 * INPUT:
 *
 *		uint16 - vlanid 	//vlan id
 *		uint32 - mstid     //mst instancce id
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued config the vlan on the mst instance
 *     OTER					//failed
 ***********************************************************************/
#define MSTP_DBUS_METHOD_CFG_VLAN_ON_MST "mstp_vlan_mst"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_CONFIG_REG_NAME
 *
 * INPUT:
 *
 *		byte - name 	//config bridge name
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define MSTP_DBUS_METHOD_CONFIG_REG_NAME "br_reg_name"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_CONFIG_BR_REVISION
 *
 * INPUT:
 *
 *		uint32 - revision 	//config bridge revision
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define MSTP_DBUS_METHOD_CONFIG_BR_REVISION "br_revision"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_PRIO
 *
 * INPUT:
 *
 *		uint32 - mstid  	//mst instance id
 *		uint32 - priority 	//config bridge priority
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_PRIO	       "brg_prio"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_MAXAGE
 *
 * INPUT:
 *
 *		uint32 - maxage 	//config bridge max age
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_MAXAGE	       "brg_maxage"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_HELTIME
 *
 * INPUT:
 *
 *		uint32 - heltime 	//config bridge hello time
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_HELTIME        "brg_heltime"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_CONFIG_MAXHOPS
 *
 * INPUT:
 *
 *		uint32 - maxhops 	//config bridge max hops
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define MSTP_DBUS_METHOD_CONFIG_MAXHOPS       "brg_maxhops"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_FORDELAY
 *
 * INPUT:
 *
 *		uint32 - fordelay 	//config bridge forward delay
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_FORDELAY       "brg_fordelay"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_FORVERSION,this is API.some cmd call it
 *
 * INPUT:
 *
 *		uint32 - forversion 	//config bridge forversion
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_FORVERSION     "brg_forversion"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_NOCONFIG,config default value
 *
 * INPUT:
 *
 *		uint32 - mstid 	//mst instance id
 * 	
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued config the vlan on the mst instance
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_NOCONFIG       "brg_noconfig"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32 - mstd			//mst instance id
 *		uint32 - pathcost 	//config port path cost
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST  "brg_pathcost"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_PORTPRIO
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *		uint32  - prioirty 	//config port priority
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_PORTPRIO       "brg_port_prio"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_NONSTP
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *		uint32  - yes/no 	 //config port non stp
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_NONSTP         "brg_nonstp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_P2P
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *		uint32  - yes/no 	 //config port p2p
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_P2P            "brg_p2p"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_EDGE
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *		uint32  - yes/no 	 //config port edge
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_EDGE           "brg_edge"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_MCHECK
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *		uint32  - yes/no 	 //config port mcheck
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_MCHECK         "brg_mcheck"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG,config port default value
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *		uint32  - mstd			//mst instance id
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG  "brg_port_noconfig"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_SHOW_SPANTREE,this is API,not a command.
 *		the message return RSTP bridge courrent	info.
 *
 * INPUT:
 *
 *  	uint32	- port_index				// port index
 *
 * OUTPUT:
 *		
 *			struct of Port Infos.
 *				uint16	port prio
 *				uint32	port cost
 *				uint32	port role
 *				uint32	port State
 *				uint32	port link
 *				uint32	port p2p
 *				uint32	port edge
 *				byte		Desi bridge mac
 *				uint16  Desi bridge priority
 *				uint32	port Desi path cost
 *				uint16	port Desi port
 ***********************************************************************/
#define RSTP_DBUS_METHOD_SHOW_SPANTREE         "brg_spann_tree"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_SHOW_SPANTREE_ADMIN_STATE,this is API,not a command.
 *		the message return RSTP bridge courrent	info.
 *
 * INPUT:
 *
 *  	uint32	- port_index				// port index
 *
 * OUTPUT:
 *		
 *			struct of Port Infos.
 *				uint32	admin state
 ***********************************************************************/
#define RSTP_DBUS_METHOD_SHOW_SPANTREE_ADMIN_STATE         "brg_spann_tree_admin_state"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_CHECK_AND_SAVE_PORT_CONFIG ,config port default value
 *
 * INPUT:
 *
 *		uint32 - port_index  //port index
 *
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/

#define RSTP_DBUS_METHOD_CHECK_AND_SAVE_PORT_CONFIG  "stp_port_save"

/*#define MSTP_DBUS_METHOD_GET_VID_BY_MSTID			"brg_vid"*/
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_SHOW_SPANTREE,this is API,not a command.
 *		the message return bridge courrent	info.
 *
 * INPUT:
 *
 *  NONE
 *
 * OUTPUT:
 *		
 *		byte 		 - rootmac[6]				// root bridge mac addr 
 *     	uint16	 - rootpriority				// root bridge priority
 *		uint32	 -	rootpathcost			// root bridge path cost
 *		uint16 	 - rootport					// root port
 *		uint16   -	maxage					// message life
 *		uint16	 - hellotime					// interval of transmited message
 *		uint16 	 - forwarddelay			// interval of migrated state
 *		byte		 - selfmac[6]				// self mac addr
 *		uint16   - selfpriority				// self priority
 *		uint32	 - selfpathcost				// self path cost
 *		uint32   - selfforceversion		// self running forceversion  
 *		uint16   -	maxage					// message life
 *		uint16	 - hellotime					// interval of transmited message
 *		uint16 	 - forwarddelay			// interval of migrated state
 ***********************************************************************/
#define RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO    "brg_info"

/*#define RSTP_DBUS_METHOD_SHOW_SPANTREE_PORT    "brg_tree_port"*/
/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_DEBUG_SPANNTREE,config RSTP program input info
 *
 * INPUT:
 *
 *		uint32  - value       //  by input param
 * 					all			 //  input all info
 *						error	 //	input err info
 *						info		//	debug info
 *						event    //    event info
 *						packet   //	packet info
 *						state		//	state info
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_DEBUG_SPANNTREE       "brg_stp_debug"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_DEBUG_SPANNTREE,config RSTP program input info
 *
 * INPUT:
 *
 *		uint32  - value       //  by input param
 * 					all			 //  input all info
 *						error	 //	input err info
 *						info		//	debug info
 *						event    //    event info
 *						packet   //	packet info
 *						state		//	state info
 * OUTPUT:
 *		
 *		STP_OK				// succued 
 *     OTER					//failed
 ***********************************************************************/
#define RSTP_DBUS_METHOD_NO_DEBUG_SPANNTREE    "brg_stp_nodebug"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG for saving running cfg
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
 ***********************************************************************/
#define RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG "stp_running_cfg"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_SHOW_CIST_INFO,this is API,not a command.
 *		the message return MSTP CIST root bridge current	 info.
 *
 * INPUT:
 *
 *  NONE
 *
 * OUTPUT:
 *		
 *		byte 		 - rootmac[6]				// root bridge mac addr 
 *     	uint16	 - rootpriority				// root bridge priority
 *		uint32	 -	rootpathcost			// root bridge path cost
 *		uint16 	 - rootport					// root port
 ***********************************************************************/
#define MSTP_DBUS_METHOD_SHOW_CIST_INFO  "mstp_cist"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_SHOW_MSTI_INFO,this is API,not a command.
 *		the message return MSTP region root bridge courrent	 info.
 *
 * INPUT:
 *
 *  	uint32	- mstid   //mst instance id
 *
 * OUTPUT:
 *		
 *		byte 		 - region root mac[6]				// root bridge mac addr 
 *     	uint16	 - region root priority				// root bridge priority
 *		uint32	 -	region root pathcost			// root bridge path cost
 *		uint16 	 - region root port					// root port
 *		uint16   -	root maxage						// rstp message life
 *		uint16	 - root hellotime					// interval of transmited message
 *		uint16 	 - root forwarddelay			// interval of migrated state
 *		uint16   -	root maxhops							// mstp message life
 ***********************************************************************/
#define MSTP_DBUS_METHOD_SHOW_MSTI_INFO   "mstp_one_msti"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_SHOW_SELF_INFO,this is API,not a command.
 *		the message return MSTP instance self bridge courrent	info.
 *
 * INPUT:
 *
 *  	uint32	-	mstid 						//mst instance id
 *
 * OUTPUT:
 *		
 *
 *		string	 -	bridge name				
 *		uint16	 - bridge revision			
 *		uint16 	 - vid							// all vlans on the instance 
 *		byte		 - selfmac[6]				// self mac addr
 *		uint16   - selfpriority				// self priority
 *		uint32	 - selfpathcost				// self path cost
 *		uint32   - selfforceversion		// self running forceversion  
 *		uint16   -	maxage					// message life
 *		uint16	 - hellotime					// interval of transmited message
 *		uint16 	 - forwarddelay			// interval of migrated state
 *		uint16   -	root maxhops			// mstp message life
 ***********************************************************************/
#define MSTP_DBUS_METHOD_SHOW_SELF_INFO		"mstp_self_info"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_GET_PORT_INFO,this is API,not a command.
 *		the message return port info in MSTP instance.
 *
 * INPUT:
 *
 *  	uint32	- port_index				// port index
 *		uint32	-	mstid						// mst instance id
 *
 * OUTPUT:
 *		
 *			struct of Port Infos.
 *				uint16	port prio
 *				uint32	port cost
 *				uint32	port role
 *				uint32	port State
 *				uint32	port link
 *				uint32	port p2p
 *				uint32	port edge
 *				byte		Desi bridge mac
 *				uint16  Desi bridge priority
 *				uint32	port Desi path cost
 *				uint16	port Desi port
 ***********************************************************************/
#define MSTP_DBUS_METHOD_GET_PORT_INFO		"mstp_port_info"


/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ETHPORTS_METHOD_STP_SET_PORT_DUPLEX_MODE;in order to 
 *    get the port link state;
 *
 * INPUT:
 * 
 *		port_index - port index
 *           duplex_mode - duplex_mode
 *
 * OUTPUT:
 *
 *		up
 *		down
 *
 ***********************************************************************/

#define MSTP_DBUS_METHOD_SET_STP_DUPLEX_MODE  "mstp_duplex_mode"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP
 *		enable\disable configuration digest snooping;
 *
 * INPUT:
 * 
 *		port_index - port index
 *      isenable   - enable|disable
 *
 * OUTPUT:
 *
 ***********************************************************************/
#define MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP "stp_port_endis_cfg_digest_snp"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT
 *		set configuration digest content;
 *
 * INPUT:
 *
 *		unsigned char *digest_str	- string of digest
 *
 * OUTPUT:
 *
 ***********************************************************************/
#define MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT	"mstp_config_bridge_digest_confent"

/*
*
* igmp snooping 
*
*/
#define IGMP_DBUS_BUSNAME	"aw.igmp"
#define IGMP_DBUS_OBJPATH	"/aw/igmp"
#define IGMP_DBUS_INTERFACE	"aw.igmp"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON 
 *					 IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF
 * INPUT:
 *	all|error|packet|warning|debug|event
 *		all:		Debug igmp-snooping all
 *		error:	Debug igmp-snooping error
 *		packet:	Debug igmp-snooping packet
 *		warning:	Debug igmp-snooping warning
 *		debug;	Debug igmp-snooping debug
 *		event:	Debug igmp-snooping event
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON		"igmp_snp_debug"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF		"no_igmp_snp_debug"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE 
 *
 * INPUT:
 * 		enable/disable - config IGMP Snooping globolly 
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE	"igmp_snp_en"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_STATE 
 *
 * INPUT:
 * 		show IGMP Snooping globolly config 
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_STATE		"igmp_snp_show_config_state"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP 
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP	"igmp_snp_del_group"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE 
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE	"igmp_snp_del_gvlan_one"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL 
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL	"igmp_snp_del_all_gvlan"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER 
 *
 * INPUT:
 * 		vlanlifetime - vlan (which enabled igmp snooping) life time
 *		robust		 - robust var
 *		grouplife	 - multicast group lifetime
 *		query interval - query interval
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER			"igmp_snp_config_timer"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 * 		vlanlifetime - vlan (which enabled igmp snooping) life time
 *		robust		 - robust var
 *		grouplife	 - multicast group lifetime
 *		query interval - query interval
 *		response interval - response interval
 *		hosttime 	 - hosttime
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER		"igmp_snp_show_timer"

/*****************************************************************
 * DESCRIPTION:
 *      arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT
 *
 * INPUT:
 *              vlanId -- vlan id
 *
 * OUTPUT:
 *              vlanId -- vlan id
 *              slotNo,portNo -- slot no,local port no
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT   "igmp_snp_show_route_port"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_EN_DIS
 *
 * INPUT:
 *		vlanId -- vlan id(which enabled/disabled IGMP Snooping).
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_EN_DIS	"igmp_snp_vlan_add_delete"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_ETH_PORT_EN_DIS
 *
 * INPUT:
 *		slotNo,portNo -- slot no,local port no(which enabled/disabled IGMP Snooping).
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ETH_PORT_EN_DIS	"igmp_snp_port_endis"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		vlanCount -- vlans which enabled IGMP Snooping
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW		"igmp_snp_show_vlan_cnt"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_LIST_SHOW
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_LIST_SHOW		"igmp_snp_show_vlan_list"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW
 *
 * INPUT:
 *
 * OUTPUT:
 *		mgroupCount - mutilcast group count.
 *		
*****************************************************************/
/*#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW		"igmp_snp_show_group_cnt"*/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW	"show_igmp_total_group_count"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_ROUTER_PORT_SHOW
 *
 * INPUT:
 *		vlanId -- vlan id(which enabled IGMP Snooping).
 *
 * OUTPUT:
 *		slotNo,portNo -- slot no,local port no(which enabled IGMP Snooping route port)
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_ROUTER_PORT_SHOW	"igmp_snp_show_group_router_port"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_TIME_PARAMETER_GET
 *
 * INPUT:
 *		NULL
 *
 * OUTPUT:
 *		time-interval {(vlan-lifetime		|IGMP_VLAN_LIFE_TIME)
 *					(group-lifetime	|IGMP_GROUP_LIFETIME)
 *					(robust			|IGMP_ROBUST_VARIABLE)
 *					(query-interval	|IGMP_V2_UNFORCED_QUERY_INTERVAL)
 *					(resp_interval		|IGMP_V2_QUERY_RESP_INTERVAL)
 *				     }
 *		
*****************************************************************/
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_TIME_PARAMETER_GET		"igmp_snp_time_parameter_get"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_STATUS
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_STATUS	"check_igmp_status"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		vlanCount -- vlans which enabled IGMP Snooping
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_COUNT	"igmp_vlan_count_show"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		vlanId -- vlans which enabled IGMP Snooping
 *		slot/port -- vlan members enabled IGMP Snooping
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW "vlan_port_igmp_info_show"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		vlanId -- vlans which enabled IGMP Snooping
 *		slot/port -- vlan members enabled IGMP Snooping
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW_V1 "vlan_port_igmp_info_show_v1"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT
 *
 * INPUT:
 *		vlanId -- vlan id  
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag port member bitmap - untagBmp //any untag port member set 1 on according bit. 
 *	uint32 tag port member bitmap - tagBmp //any tag port member set 1 on according bit. 

*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT "igmp_snp_show_route_port"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT_V1
 *
 * INPUT:
 *		vlanId -- vlan id  
 *
 * OUTPUT:
 *	uint32 ret - ret			//npd operation return value,indicates whether it success.
 *	uint32 product Id -product_id //indicate product type
 *    uint32 promis port bmp - promisPortBmp[0] //any promis port member set 1 on according bit,low byte
 *    uint32 promis port bmp - promisPortBmp[1] //any promis port member set 1 on according bit,high byte
 *	string vlan name - vlanName	//vlan struct's member.
 *	uint32 untag trunk member bitmap - untagBmp[0] //any untag trunk member set 1 on according bit.low byte 
 *	uint32 untag trunk member bitmap - untagBmp[1] //any untag trunk member set 1 on according bit. highbyte
 *	uint32 tag trunk member bitmap - tagBmp[0] //any tag trunk member set 1 on according bit. low byte
 *	uint32 tag trunk member bitmap - tagBmp[1] //any tag trunk member set 1 on according bit. high byte
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT_V1 "igmp_snp_show_route_port_v1"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CHECK_VLAN_IGMP_SNP_STATUS 
 *
 * INPUT:
 *		uint16 vlan id -- vlan ID 
 *
 * OUTPUT:
 *		char status -- vlans which enabled IGMP Snooping Or NOT state
 *		uint32 op_ret -- oparetion return value
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_CHECK_VLAN_IGMP_SNP_STATUS	"igmp_vlan_state"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_VLANMBR_STATUS 
 *
 * INPUT:
 *		slotNo,portNo -- slot no,local port no(which enabled/disabled IGMP Snooping).
 *
 * OUTPUT:
 *		ret -- opeartion result
 *
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_VLANMBR_STATUS  "check_igmpport_status"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_VLAN
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_VLAN	"config_igmp_vlan"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_ETHPORT
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_ETHPORT	"config_igmp_eth_port"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_ONE_MCGROUP_PORT_MEMBERS
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_ONE_MCGROUP_PORT_MEMBERS	"show_one_group_mbr"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS	"show_group_list_mbr"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS_V1	"show_group_list_mbr_v1"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT
 *
 * INPUT:
 *		vid -- vlanId(which will be config a route port for IGMP Snooping)
 *		slotNo,portNo -- slot no,local port no (which will be config to be route port for IGMP SNP).
 *		isEnable -- enable or disable(config variable 1-enable,0-diable)
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT	"add_del_mcroute_port"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_IGMP_SNP_EXCHANGE_IFINDEX_TO_SLOTPORT
 *
 * INPUT:
 *		ifindex[32] -- ifindex route-port(which is a route port for IGMP Snooping)
 *
 * OUTPUT:
 *		slotNo,portNo -- slot no,local port no (which is a route port for IGMP Snooping).
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_IGMP_SNP_EXCHANGE_IFINDEX_TO_SLOTPORT	"exchange_ifindex_to_slotport"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_VLAN_METHOD_GET_SLOTPORT_BY_INDEX
 *
 * INPUT:
 *		eth_g_index -- port global index
 *
 * OUTPUT:
 *		slotNo,portNo -- slot no,port no(convert from eth_g_index).
 *		
*****************************************************************/
#define NPD_DBUS_VLAN_METHOD_GET_SLOTPORT_BY_INDEX	"convert_ethgindex_slotport"

/*
* route table in marvell driver
*
*/
#define RTDRV_DBUS_NAME					       	"aw.npd.rtdrv"
#define RTDRV_DBUS_OBJPATH				       	"/aw/npd/rtdrv"
#define RTDRV_DBUS_INTERFACE			       	"aw.npd.rtdrv"
#define RTDRV_DBUS_METHOD_SHOW_ALL         		"rtdrv_show_all"
#define RTDRV_DBUS_METHOD_SHOW_HOST         	"rtdrv_show_host"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RTDRV_DBUS_METHOD_SHOW_ENTRY,this is for show HW route driver info.
 *
 *
 * INPUT:
 *
 *  		uint32	- destination IP address	// format as 0x0A0B0C0D means A.B.C.D
 *		uint32	- IP mask length or point-split decimal format  // mask 255.255.255.255 or length 32
 *
 * OUTPUT:
 *		
 *		uint32	return value
 *		uint32	route nexthop HW table index
 *		uint16	next-hop vlan id
 *		uint32	next-hop interface index
 *		uint32	isTrunk 
 *					TRUE - the route is about trunk
 *					FALSE - the route is about port
 *		uint8	next-hop destination port or trunk type device number
 *		uint8	next-hop destination port type 
 *					device port number when <isTrunk> is FALSE
 *					trunk id when <isTrunk> is TRUE
 *		uint8	mac[i]	next-hop destination mac address, 
 *		...				i : [0-5],six times 
 *		uint32  next-hop entry reference count 
 ***********************************************************************/
#define RTDRV_DBUS_METHOD_SHOW_ENTRY         	"rtdrv_show_entry"
#define RTDRV_DBUS_METHOD_CONFIG_RPF         	"rtdrv_config_rpf"
#define RTDRV_DBUS_METHOD_SHOW_RPF         		"rtdrv_show_rpf"
#define RTDRV_DBUS_METHOD_SHOW_STATUES         	"rtdrv_show_statues"

/************************************************************************
 * DESCRIPTION:
 * 	arg lists for method RTDRV_DBUS_METHOD_ARP_AGING_TIME,
 *		set arp aging time.
 *
 *
 * INPUT:
 *
 *  	uint32	- aging time	// format as 0x0A0B0C0D means A.B.C.D
 *
 * OUTPUT:
 *		
 *		RETURN
 *			SUCCESS 
 *			FAIL
 *		
 ***********************************************************************/
#define RTDRV_DBUS_METHOD_ARP_AGING_TIME		"rtdrv_arp_aging"
/* dbus for nam*/
/*#define NAM_DBUS_NAME					       "aw.npd.nam"
#define NAM_DBUS_OBJPATH				       "/aw/npd/nam"
#define NAM_DBUS_INTERFACE			           "aw.npd.nam"
#define NAM_DBUS_METHOD_SYSLOG_DEBUG           "nam_log_debug"
#define NAM_DBUS_METHOD_SYSLOG_NO_DEBUG        "nam_log_no_debug"*/

/*dbus for nbm*/
/*#define NBM_DBUS_NAME					       "aw.npd.nbm"
#define NBM_DBUS_OBJPATH				       "/aw/npd/nbm"
#define NBM_DBUS_INTERFACE			           "aw.npd.nbm"
#define NBM_DBUS_METHOD_SYSLOG_DEBUG           "nbm_log_debug"
#define NBM_DBUS_METHOD_SYSLOG_NO_DEBUG        "nbm_log_no_debug"*/


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP
 *
 * INPUT:
 *		eth_g_index -- port global index
 *
 * OUTPUT:
 *		slotNo,portNo -- slot no,port no(convert from eth_g_index).
 *		
*****************************************************************/
#define NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP	"npd_notify_snmp_ip"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP
 *
 * INPUT:
 *		uint32 state -- vrrp state
 *
 * OUTPUT:
 *	
 *		
*****************************************************************/


#define NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP	"npd_notify_snmp_vrrp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_MIB_BY_VRRP
 *
 * INPUT:
 *		uint32 state -- vrrp state
 *
 * OUTPUT:
 *	
 *		
*****************************************************************/


#define NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_MIB_BY_VRRP	"npd_notify_snmp_mib_vrrp"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_IP
 *
 * INPUT:
 *		eth_g_index -- port global index
 *
 * OUTPUT:
 *		slotNo,portNo -- slot no,port no(convert from eth_g_index).
 *		
*****************************************************************/
#define NPD_DBUS_SYSTEM_CONFIG_PROMIS_PORT_ADD2_INTERFACE	"npd_promis_add2_interface"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG
 *
 * INPUT:
 *		unit32	hwtype	-PHY
 *		unit32	opTpye	-read, write
 *		unit32	device	-<0-1>
 *		unit16	portNo	-<1-26>
 *		unit16	regaddr   -register address
 *		unit16	regvalue  -value
 *
 * OUTPUT:
 *		unit32	ret
 *		unit16	regvalue  -value
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_PHY_HW_RW_REG	"Diagnosis_phy_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_XAUI_PHY_HW_RW_REG
 *
 * INPUT:
 *		unit32	  opTpye	-read, write
 *		uint8	  devNum 	-<0-1>
 *		uint8	  portNum	-<0-30>
 *		uint8	  phyId,		-<0-31>
 *		uint8 	  useExternalPhy,
 *		unit16	  phyReg,
 *		uint8	  phyDev	-<0-31>
 *
 * OUTPUT:
 *		unit32	ret
 *		unit16	regvalue  -value
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_XAUI_PHY_HW_RW_REG	"Diagnosis_xaui_phy_hw_rw_reg"

#define NPD_DBUS_SYSTEM_DIAGNOSIS_SET_AP_FAKE_SINGAL_STRENGTH	"Diagnosis_set_ap_fake_singal_strength"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_SHOW_MIB_GE_XG_PORT
 *
 * INPUT:
 *		unit32	  portTpye	-0: ge, 1:xg
 *
 * OUTPUT:
 *		unit32	ret
*****************************************************************/
#define NPD_DBUS_SYSTEM_SHOW_MIB_GE_XG_PORT				"show_mib_ge_xg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_PCI_HW_RW_REG
 *
 * INPUT:
 *		unit32	hwtype	-PCI
 *		unit32	opTpye	-read, write
 *		unit32	device	-<0-1>
 *		unit32	regaddr   -register address
 *		unit32	regvalue  -value
 *
 * OUTPUT:
 *		unit32	regvalue  -value
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_PCI_HW_RW_REG	"Diagnosis_pci_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_CPU_HW_RW_REG
 *
 * INPUT:
 *		uint32 	opTpye	-read, write
 *		uint32 	regaddr   -register address
 *		uint32 	regvalue  -value (ignored when read)
 *
 * OUTPUT:
 *		uint32 	ret - operation result
 *		uint32 	regValue - register value (ignored when write)
 *		
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_CPU_HW_RW_REG	"diag_cpu_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_CPLD_HW_RW_REG
 *
 * INPUT:
 *		unit32	hwtype	-cpld
 *		unit32	opTpye	-read, write
 *		unit32	device	-<0-1>
 *		unit32	regaddr   -register address
 *		unit32	regvalue  -value
 *
 * OUTPUT:
 *		unit32	ret
 *		unit32	regvalue  -value
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_CPLD_HW_RW_REG	"Diagnosis_cpld_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_MAC_HW_RW_REG
 *
 * INPUT:
 *		unit32	hwtype	-mac
 *		unit32	opTpye	-read, write
 *		unit32	device	-<0-1>
 *		unit32	port		-<1-29>
 *		unit32	regtype   -register type
 *		unit32	regvalue  -value
 *
 * OUTPUT:
 *		unit32	ret
 *		unit32	regvalue  -value
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_MAC_HW_RW_REG				"Diagnosis_mac_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG
 *
 * INPUT:
 *		unit32	hwtype	-mac
 *		unit32	opTpye	-read, write
 *		unit32	device	-<0-1>
 *		unit32	regtype   -register type
 *		unit32	regvalue  -value
 *
 * OUTPUT:
 *		unit32	ret
 *		unit32	regvalue  -value
 *****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_HW_RW_REG					"Diagnosis_hw_rw_reg"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PRBS_TEST
 *
 * INPUT:
 *		unit32	opDevice	-<0-1>
 *
 * OUTPUT:
 *		unit32	result
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PRBS_TEST              "Diagnosis_hw_prbs_test"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PORT_MODE_SET
 *
 * INPUT:
 *		unit32	opDevice	-<0-1>
        unit32	opPort	    -<0-59>
 *      unit32  portMode    -xaui,rxaui,rdxaui
 * OUTPUT:
 *		unit32	ret
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_HW_PORT_MODE_SET              "Diagnosis_hw_port_mode_set"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_CSCD_PORT_PRBS_TEST
 *
 * INPUT:
 *		unit32	opDevice	-<0-1>
 *
 * OUTPUT:
 *      unit32	ret
 *		unit32	Gpmem[0]    -group 0 member bitmask
 *      unit32	Gpmem[1]    -group 1 member bitmask
 *      unit32	Gpmem[2]    -group 2 member bitmask
 *      unit32	Gpmem[3]    -group 3 member bitmask
 *      unit32	Gpres[0]    -group 0 member prbs test bitmask
 *      unit32	Gpres[1]    -group 1 member prbs test bitmask
 *      unit32	Gpres[2]    -group 2 member prbs test bitmask
 *      unit32	Gpres[3]    -group 3 member prbs test bitmask
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_HW_CSCD_PORT_PRBS_TEST              "Diagnosis_hw_asic_cscd_port_prbs_test"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_FIELD_HW_DUMP_TAB
 *
 * INPUT:
 *		unit32	device	-<0-1>
 *		unit32	gnum  	-field  number
 *
 * OUTPUT:
 *		string	showStr
*****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_FIELD_HW_DUMP_TAB			"Diagnosis_acl_group_hw_dump_tab"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_COSQ_HW_DUMP_TAB
 *
 * INPUT:
 *		unit32	device	-<0-1>
 *		unit32	port		-<1-29>
 *
 * OUTPUT:
 *		string	showStr
 *****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_COSQ_HW_DUMP_TAB			"Diagnosis_cosq_hw_dump_tab"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_HW_DUMP_TAB
 *
 * INPUT:
 *		unit32	opDevice		-<0-1>
 *		unit32	opBlock		-<0-3>
 *		unit32	tabIndex
 *		unit32	opRegtype
 *
 * OUTPUT:
 *		unit32 	ret
 *		unit32	regdata
 *****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_HW_DUMP_TAB					"Diagnosis_hw_dump_tab"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_ACL_HW_SHOW
 *
 * INPUT:
 *		unit32	rulesize
 *		unit32	aclindex
 *
 * OUTPUT:
 *		string	showStr
 *****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_ACL_HW_SHOW 				"Diagnosis_acl_hw_show"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SYSTEM_WATCHDOG_CONTROL
 *
 * INPUT:
 *		uint32 	-  watchdog enable(1) or disable(0) flag
 *
 * OUTPUT:
 *		uint32 	- operation return code
 *		
*****************************************************************/
#define NPD_DBUS_METHOD_SYSTEM_WATCHDOG_CONTROL	"diag_watchdog_ctrl"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_METHOD_SYSTEM_WATCHDOG_TIMEOUT
 *
 * INPUT:
 *		uint32   -  watchdog timeout set(1) or get(0) operation
 *		uint32 	-  watchdog timeout value, valid range [1,255]
 *
 * OUTPUT:
 *		uint32 	- operation return code
 *		uint32	- timeout value return back, valid only in GET operation
 *		uint32 	- watchdog control flag:enable or disable, valid only in GET operation
 *		
*****************************************************************/
#define NPD_DBUS_METHOD_SYSTEM_WATCHDOG_TIMEOUT	"diag_watchdog_timeout"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_IP_TUNNEL_ADD
 *
 * INPUT:
 *		unit32	sipmaskLen
 *		unit32	dipmaskLen
 *		unit32	dipaddr
 *		unit32	sipaddr
 *		byte		mslot
 *		byte		mport
 *
 * OUTPUT:
 *		unit32	op_ret
 *****************************************************************/
#define NPD_DBUS_IP_TUNNEL_ADD									"Tunnel_ip_add"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_IP_TUNNEL_DELETE
 *
 * INPUT:
 *		unit32	sipmaskLen
 *		unit32	dipmaskLen
 *		unit32	dipaddr
 *		unit32	sipaddr
 *		byte		mslot
 *		byte		mport
 *
 * OUTPUT:
 *		unit32	op_ret
 *****************************************************************/
#define NPD_DBUS_IP_TUNNEL_DELETE									"Tunnel_ip_delete"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_IP_TUNNEL_HOST_ADD
 *
 * INPUT:
 *		unit32	ripmaskLen
 *		unit32	lipmaskLen
 *		unit32	dipmaskLen
 *		unit32	ripaddr
 *		unit32	lipaddr
 *		unit32	dipaddr
 *
 * OUTPUT:
 *		unit32	op_ret
 *****************************************************************/
#define NPD_DBUS_IP_TUNNEL_HOST_ADD								"Tunnel_ip_host_add"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_IP_TUNNEL_HOST_DELETE
 *
 * INPUT:
 *		unit32	ripmaskLen
 *		unit32	lipmaskLen
 *		unit32	dipmaskLen
 *		unit32	ripaddr
 *		unit32	lipaddr
 *		unit32	dipaddr
 *
 * OUTPUT:
 *		unit32	op_ret
 *****************************************************************/
#define NPD_DBUS_IP_TUNNEL_HOST_DELETE							"Tunnel_ip_host_delete"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_IP_TUNNEL_SHOW_TAB
 *
 * INPUT:
 *		unit32	opDevice
 *		unit32	portnum
 * OUTPUT: 
 *		string 	showStr
 *		
 *****************************************************************/
#define NPD_DBUS_IP_TUNNEL_SHOW_TAB								"Tunnel_show"

/*****************************************************************
 * DESCRIPTION:
 * 	config whether accept mutinetwork arp 
 *                                (eg. 30.1.1.1 to 50.2.2.5)
 *
 * INPUT:
 *		isEnable - config enable or disable
 *
 * OUTPUT:
 *		ret - return value
*****************************************************************/

#define NPD_DBUS_CONFIG_ARP_MUTI_NETWORK "Config_arp_muti_network"

/*****************************************************************
 * DESCRIPTION:
 * 	config arp inspection 
 *
 * INPUT:
 *		isEnable - config enable or disable
 *
 * OUTPUT:
 *		ret - return value
*****************************************************************/

#define NPD_DBUS_CONFIG_ARP_INSPECTION "Config_arp_inspection"

/*****************************************************************
 * DESCRIPTION:
 * 	config arp proxy 
 *
 * INPUT:
 *		isEnable - config enable or disable
 *
 * OUTPUT:
 *		ret - return value
*****************************************************************/

#define NPD_DBUS_CONFIG_ARP_PROXY "Config_arp_proxy"



/*******************************
*
*
*vrrp
*
*
******************************/
#define	NPD_DBUS_VRRP_OBJPATH	"/aw/npd/vrrp"
#define	NPD_DBUS_VRRP_INTERFACE	"aw.npd.vrrp"

#define NPD_DBUS_VRRP_METHOD_GET_SYSMAC "npd_vrrp_sysmac"

#define VRRP_DBUS_BUSNAME			"aw.vrrpcli"
#define VRRP_NOTIFY_DBUS_BUSNAME	"aw.vrrpnoti"
	
#define VRRP_DBUS_OBJPATH "/aw/vrrp"
#define VRRP_DBUS_INTERFACE "aw.vrrp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP
 *					VRRP_DBUS_METHOD_BRG_DBUG_VRRP
 * INPUT:
 *	all|error|warning|debug|event
 *		all:		Debug dldp all
 *		error:	Debug dldp error
 *		warning:	Debug dldp warning
 *		debug;	Debug dldp debug
 *		event:	Debug dldp event
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP "vrrp_no_debug"
#define VRRP_DBUS_METHOD_BRG_DBUG_VRRP    "vrrp_debug"
#define VRRP_DBUS_METHOD_BRG_DBUG_VRRP_PROFILE    "vrrp_debug_profile"
#define VRRP_DBUS_METHOD_HAD_TRAP_SWITCH	"vrrp_trap_switch"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --uplink_ifname,
 *     iaddr --uplink_ipv6,
 *    uint32-- uplink_prefix_length
 *     char* --downlink_ifname,
 *    iaddr --downlink_ipv6,
 *   uint32----down_prefix_length
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_START_VRRP_IPV6   "vrrp_start_ipv6"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --uplink_ifname,
 *     uint32 --uplink_ip,
 *    uint32-- uplink_mask
 *     char* --downlink_ifname,
 *    uint32 --downlink_ip,
 *   uint32----downlink_mask
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_START_VRRP   "vrrp_start"
/*****************************************************************
 * DESCRIPTION:
 * 	set vrrp vrid value
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_SET_VRRPID   "vrrp_vrid"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance appiont real ip
 * INPUT:
 *     uint32--vrid,
 *     char* --uplink_ifname,
 *     char* --uplink_ip,
 *     char* --downlink_ifname,
 *     char* --downlink_ip,
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_VRRP_REAL_IP				"vrrp_real_ip"

/****************************************************************
* DESCRIPTION:
*		appoint real IP address to downlink interface
*
* INPUT:
*		uint32 - profile
*		char * - downlink_ifname
*		char * - downlink_ip
*
* OUTPUT:
*		uint32 - ret
*
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_REAL_IP_DOWNLINK		"vrrp_real_ip_downlink"

/****************************************************************
* DESCRIPTION:
*		appoint real IP address to uplink interface
*
* INPUT:
*		uint32 - profile
*		char * - uplink_ifname
*		char * - uplink_ip
*
* OUTPUT:
*		uint32 - ret
*
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_REAL_IP_UPLINK		"vrrp_real_ip_uplink"

/****************************************************************
* DESCRIPTION:
*		cancel real IP address from downlink interface
*
* INPUT:
*		uint32 - profile
*		char * - downlink_ifname
*		char * - downlink_ip
*
* OUTPUT:
*		uint32 - ret
*
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_DOWNLINK	"vrrp_no_real_ip_downlink"

/****************************************************************
* DESCRIPTION:
*		cancel real IP address from uplink interface
*
* INPUT:
*		uint32 - profile
*		char * - uplink_ifname
*		char * - uplink_ip
*
* OUTPUT:
*		uint32 - ret
*
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_UPLINK		"vrrp_no_real_ip_uplink"

#define VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK   "vrrp_heartbeat_link"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance,only downlink
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --downlink_ifname,
 *     char* --downlink_ip,
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_START_VRRP_DOWNLINK   "vrrp_start_downlink"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance,only uplink
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --uplink_ifname,
 *     char* --uplink_ip,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_START_VRRP_UPLINK   "vrrp_start_uplink"


/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance,only downlink ipv6
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --downlink_ifname,
 *     char* --downlink_ipipv6,
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_START_VRRP_DOWNLINK_IPV6 "vrrp_start_downlink_ipv6"

/*****************************************************************
 * DESCRIPTION:
 * 	start one vrrp-based hansi instance,only uplink
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --uplink_ifname,
 *     char* --uplink_ipv6,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_START_VRRP_UPLINK_IPV6 "vrrp_start_uplink_ipv6"

/*****************************************************************
 * DESCRIPTION:
 * 	    add uplink virtual link local ip
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --uplink_ifname,
 *     char* --uplink_ip,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_START_VRRP_UPLINK_LINK_LOCAL "had_dbus_start_uplink_link_local"

/*****************************************************************
 * DESCRIPTION:
 * 	    add downlink virtual link local ip
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --downlink_ifname,
 *     char* --downlink_ip,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_START_VRRP_DOWNLINK_LINK_LOCAL "had_dbus_start_downlink_link_local"

/*****************************************************************
 * DESCRIPTION:
 * 	    add downlink virtual link local ip
 * INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *     char* --vgateway_ifname,
 *     char* --vgateway_ip,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_V6_GATEWAY "vrrp_vgateway_ipv6"

/*****************************************************************
 * DESCRIPTION:
 *		delete uplink|downlink virtual ipv6
 * INPUT:
 *		uint32 --profile,
 *		uint32 --opt_type,
 *		uint32 --link_type,
 *		char*  --ifname,
 *		uint32 --virtual_ipv6,
 *		uint32 --prefix_length
 *
 * OUTPUT:
 *		uint --op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_LINK_DEL_VIPV6   "vrrp_link_del_vipv6"

/*****************************************************************
 * DESCRIPTION:
 *		add uplink|downlink virtual ipv6
 * INPUT:
 *		uint32 --profile,
 *		uint32 --opt_type,
 *		uint32 --link_type,
 *		char*  --ifname,
 *		uint32 --virtual_ipv6,
 *		uint32 --prefix_length
 *
 * OUTPUT:
 *		uint --op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_LINK_ADD_VIPV6   "vrrp_link_add_vipv6"
 
/*****************************************************************
 * DESCRIPTION:
 *		add|delete uplink|downlink virtual ip
 * INPUT:
 *		uint32 --profile,
 *		uint32 --opt_type,
 *		uint32 --link_type,
 *		char*  --ifname,
 *		uint32 --virtual_ip,
 *		uint32 --mask
 *
 * OUTPUT:
 *		uint --op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP   "vrrp_link_add_del_vip"
#define VRRP_DBUS_METHOD_VRRP_LINK_DEL_VIP   "vrrp_link_del_vip"

/*****************************************************************
 * DESCRIPTION:
 * 	start / stop  vrrp service 
 * INPUT:
 *     uint32--vrid,
 *     uint32--enable,
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE   "vrrp_service_enable"


#define VRRP_DBUS_METHOD_VRRP_STATE_CHANGE  "vrrp_state_change"

/*****************************************************************
 * DESCRIPTION:
 * 	set vrrp want state to master 
 * INPUT:
 *     uint32--profile,
 *    uint32 -- enable
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_VRRP_WANT_STATE   "vrrp_want_state"

/*****************************************************************
 * DESCRIPTION:
 * 	start virtual gateway on vrrp
 * INPUT:
 *     uint32--vrid,
 *     char* --vgateway_ifname,
 *     unsigned long --vgateway_ip,
 *     unsigned int  ---vgateway_masklen
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_V_GATEWAY   "vrrp_vgateway"

/*****************************************************************
 * DESCRIPTION:
 * 	NO virtual gateway on vrrp
 * INPUT:
 *     uint32--vrid,
 *     char* --vgateway_ifname,
 *     unsigned long--vgateway_ip,
 *     unsigned int  ---vgateway_masklen
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_NO_V_GATEWAY   "vrrp_no_vgateway"

/*****************************************************************
 * DESCRIPTION:
 * 	NO TRANSFER on vrrp
 * INPUT:
 *     uint32--vrid,
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_NO_TRANSFER   "vrrp_no_transfer"

/*****************************************************************
 * DESCRIPTION:
 * 	kill one vrrp-based hansi instance
 * INPUT:
 *     uint32--vrid
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_END_VRRP     "vrrp_end"
/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance priority
 *  INPUT:
 *     uint32--vrid,
 *     uint32--priority,
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_PROFILE_VALUE "vrrp_profile"
/*****************************************************************
 * DESCRIPTION:
 * 	config global virtual mac 
 * INPUT:
 *     uint32--profile,
 *     uint32--gvmac
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_GLOBAL_VMAC_ENABLE "vrrp_global_vmac"
/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance state
 * INPUT:
 *     uint32--profile,
 *     uint32--preempt
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_PREEMPT_VALUE "vrrp_preempt"
/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance advertisement
 * INPUT:
 *     uint32--vrid,
 *     uint32--advertisement
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_ADVERT_VALUE "vrrp_advert"
/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance interface mac to virtual or not
 * INPUT:
 *     uint32--vrid,
 *     uint32-- chang_flag
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE "vrrp_virtual_mac"

/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance time syn or not
 * INPUT:
 *     uint32--vrid,
 *     uint32-- chang_flag
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_TIME_SYN_VALUE "vrrp_time_syn"

/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance interface mac to virtual or not
 * INPUT:
 *     uint32-- packet count
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_MS_DOWN_PACKT_COUNT "vrrp_ms_count"
/*****************************************************************
 * DESCRIPTION:
 * 	change one vrrp-based hansi instance interface mac to virtual or not
 * INPUT:
 *     uint32-- packet count
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_MULTI_LINK_DETECT  "vrrp_multi_link_detect"

/*****************************************************************
 * DESCRIPTION:
 * 	config hansi profile xx
 * INPUT:
 *     uint32--profile
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE "vrrp_hansi_profile"
/*****************************************************************
 * DESCRIPTION:
 * 	show hansi profile xx
 * INPUT:
 *     uint32--profile
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_SHOW                "vrrp_show"
/*****************************************************************
 * DESCRIPTION:
 * 	show hansi profile xx detail
 * INPUT:
 *     uint32--profile
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_SHOW_DETAIL                "vrrp_show_detail"

#define VRRP_DBUS_METHOD_SHOW_RUNNING                "vrrp_show_running"

/*****************************************************************
 * DESCRIPTION:
 * 	get hansi profile uplink & downlink ifname
 * INPUT:
 *     uint32--profile
 *
 *
 * OUTPUT:
 *		uint32--op-ret
 *           string--uplink ifname
 *		string--downlink ifname
*****************************************************************/

#define VRRP_DBUS_METHOD_GET_IFNAME           "vrrp_show_ifname"

/*****************************************************************
 * DESCRIPTION:
 * 	when portal start,call this method
 * INPUT:
 *     uint32--portal state
 *
 * OUTPUT:
 *		uint32--op-ret
*****************************************************************/

#define VRRP_DBUS_METHOD_SET_PROTAL          "vrrp_set_protal"
#ifndef _VERSION_18SP7_
#define VRRP_DBUS_METHOD_SET_PPPOE          "vrrp_set_pppoe"
#endif
/*****************************************************************
 * DESCRIPTION:
 * 	when wid transfer over,call this method
 * INPUT:
 *
 *      uint32--vrid
 *      uint32--wid state
 *    
 * OUTPUT:
 *		uint32--op-ret
*****************************************************************/

#define VRRP_DBUS_METHOD_SET_TRANSFER_STATE          "vrrp_set_transfer_state"

/*****************************************************************
 * DESCRIPTION:
 * 	when wid transfer over,call this method
 * INPUT:
 *
 *      uint32--vrid
 *      uint32--portal state
 *    
 * OUTPUT:
 *		uint32--op-ret
*****************************************************************/

#define VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE          "vrrp_set_portal_transfer_state"
#ifndef _VERSION_18SP7_
#define VRRP_DBUS_METHOD_SET_PPPOE_TRANSFER_STATE          "vrrp_set_pppoe_transfer_state"
#endif
#define VRRP_DBUS_METHOD_START_SEND_ARP   "vrrp_send_arp"

/*****************************************************************
 * DESCRIPTION:
 *		arg lists for method VRRP_DBUS_METHOD_SET_NOTIFY_OBJ_AND_FLG
 * 
 * INPUT:
 *		uint32--profile,
 *		byte  --notify_obj,
 *		byte  --notify_flg
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_SET_NOTIFY_OBJ_AND_FLG		"vrrp_set_notify_obj_and_flg"

/*****************************************************************
 * DESCRIPTION:
 *		arg lists for method VRRP_DBUS_METHOD_SET_VGATEWAY_TRANSFORM_FLG
 * 
 * INPUT:
 *		uint32--profile,
 *		uint32--vgateway_tf_flg
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_SET_VGATEWAY_TRANSFORM_FLG		"vrrp_set_vgateway_transform_flg"

/*****************************************************************
 * DESCRIPTION:
 *		arg lists for method VRRP_DBUS_METHOD_SET_DHCP_FAILOVER_IPADDR
 * 
 * INPUT:
 *		uint32--profile,
 *		uint32--dhcp failover peer ip address
 *		uint32 -- dhcp failover local ip address
 *
 * OUTPUT:
 *		uint32--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_SET_DHCP_FAILOVER_IPADDR		"vrrp_set_dhcp_failover_ipaddr"

/*****************************************************************
 * DESCRIPTION:
 *		arg lists for method VRRP_DBUS_METHOD_CLEAR_DHCP_FAILOVER_IPADDR
 * 
 * INPUT:
 *		uint32--profile
 *
 * OUTPUT:
 *		uint32--op_ret
 *		
*****************************************************************/
#define VRRP_DBUS_METHOD_CLEAR_DHCP_FAILOVER_IPADDR		"vrrp_clr_dhcp_failover_ipaddr"
/*****************************************************************
 * DESCRIPTION:
 *		arg lists for method VRRP_DBUS_METHOD_CONFIG_CONTINOUS_DISCRETE_VALUE
 * 
 * INPUT:
 *		uint32--profile
 *		uint32 -- mode     CONTINOUS or DISCRETE
 *		uint32 -- value     <1-300>   or <1-50> value for this two mode
 *
 * OUTPUT:
 *		uint32--op_ret
 *		
*****************************************************************/

#define VRRP_DBUS_METHOD_CONFIG_CONTINOUS_DISCRETE_VALUE	"vrrp_config_gratuitous_arp_continous_discrete_value"

/***************************************************
 * INPUT:
 *	uint32  -- profile   hansi node profile which snmp want to it's state
 * OUTPUT:
 *	uint32 --  profile	if the hansi is exist, it is the same as the input profile,else it is 0
 *	uint32 --  state	if the hansi is exist, it is the state of the hansi,else it is 0
 *
 ***************************************************/
#define VRRP_DBUS_METHOD_SNMP_GET_VRRP_STATE	"vrrp_snmp_get_vrrp_state"


/****************************
 *
 * DLDP
 *
 ****************************/
#define	NPD_DBUS_DLDP_OBJPATH	"/aw/npd/dldp"
#define	NPD_DBUS_DLDP_INTERFACE	"aw.npd.dldp"

#define DLDP_DBUS_BUSNAME		"aw.dldp"
#define DLDP_DBUS_OBJPATH		"/aw/dldp"
#define DLDP_DBUS_INTERFACE		"aw.dldp"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_DEBUG_ON 
 *					 DLDP_DBUS_METHOD_DLDP_DEBUG_OFF
 * INPUT:
 *	all|error|warning|debug|event
 *		all:		Debug dldp all
 *		error:	Debug dldp error
 *		warning:	Debug dldp warning
 *		debug;	Debug dldp debug
 *		event:	Debug dldp event
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DLDP_DBUS_METHOD_DLDP_DEBUG_ON		"dldp_debug"
#define DLDP_DBUS_METHOD_DLDP_DEBUG_OFF		"no_dldp_debug"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_ENABLE 
 *
 * INPUT:
 * 		enable/disable  - config DLDP global
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DLDP_DBUS_METHOD_DLDP_ENABLE		"dldp_en"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_SET_TIMER 
 *
 * INPUT:
 *		detection_interval			- time of DLDP detedtion interval
 *		re-detection_interva		- time of DLDP re-detedtion interval
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/

#define DLDP_DBUS_METHOD_DLDP_SET_TIMER		"dldp_config_timer"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_SHOW_TIMER 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DLDP_DBUS_METHOD_DLDP_SHOW_TIMER	"dldp_show_timer"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_SHOW_VLAN_MEMBER_STATUS 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DLDP_DBUS_METHOD_DLDP_SHOW_VLAN_MEMBER_STATUS "dldp_show_vlan_member_status"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method DLDP_DBUS_METHOD_DLDP_TIME_PARAMETER_GET 
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define DLDP_DBUS_METHOD_DLDP_TIME_PARAMETER_GET	"dldp_time_patameter_get"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_CHECK_GLOBAL_STATUS
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_CHECK_GLOBAL_STATUS	"check_dldp_status"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_CONFIG_ETHPORT
 *
 * INPUT:
 *		unsigned char slot_no					- solt no of the eth port
 *		unsigned char port_no					- port no of the eth port
 *		unsigned char enable					- enable/ disable

 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_CONFIG_ETHPORT			"config_dldp_ports"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_CONFIG_VLAN
 *
 * INPUT:
 *		unsigned short vlanid					- vlan id
 *		unsigned char enable					- enable/ disable

 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_CONFIG_VLAN			"config_dldp_vlan"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_VLAN_COUNT
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_GET_VLAN_COUNT			"get_dldp_vlan_count"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_GET_PRODUCT_ID
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_GET_PRODUCT_ID			"get_product_id"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_EXCHANGE_IFINDEX_TO_SLOTPORT
 *
 * INPUT:
 *		ifindex		- ifindex
 *
 * OUTPUT:
 *		slotNo,portNo 	- slot no,local port no
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_EXCHANGE_IFINDEX_TO_SLOTPORT	"dldp_exchange_ifindex_to_slotport"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DLDP_METHOD_EXCHANGE_IFINDEX_TO_SLOTPORT
 *
 * INPUT:
 *		ifindex		- ifindex
 *
 * OUTPUT:
 *		slotNo,portNo 	- slot no,local port no
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DLDP_METHOD_VLAN_SHOW_RUNNING_CONFIG	"dldp_vlan_show_running_config"


/****************************
 *
 * DHCP-Snooping
 *
 ****************************/
#define	NPD_DBUS_DHCP_SNP_OBJPATH	"/aw/npd/dhcpsnp"
#define	NPD_DBUS_DHCP_SNP_INTERFACE	"aw.npd.dhcpsnp"

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
#define NPD_DBUS_DHCP_SNP_METHOD_CHECK_GLOBAL_STATUS	"check_dhcp_snp_status"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE
 *
 * INPUT:
 *		unsigned isEnable		- flag of enable or disable in the global
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE			"endis_global_dhcp_snp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE
 *
 * INPUT:
 *		unsigned isEnable		- flag of enable or disable on vlan
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE			"endis_vlan_dhcp_snp"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE
 *
 * INPUT:
 *		unsigned short vlanId,			- vlan id
 *		unsigned char slot_no,			- slot no of panel port
 *		unsigned char local_port_no,	- local port no of panel port
 *		unsigned char trust_mode,		- trust mode
 *
 * OUTPUT:
 *		ret -- operation result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE		"dhcp_snp_set_port_trust_mode"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE		"dhcp_snp_show_bind_table"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method
 *		NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE		"dhcp_snp_show_static_bind_table"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method
 *		NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_VLAN
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_VLAN			"dhcp_snp_show_static_bind_table_by_vlan"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method
 *		NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_ETHPORT
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_ETHPORT		"dhcp_snp_show_static_bind_table_by_ethport"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_TRUST_PORTS
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_TRUST_PORTS		"dhcp_snp_show_trust_ports"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE			"dhcp_snp_endis_opt82"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPTION82_FORMAT_TYPE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FORMAT_TYPE	"dhcp_snp_set_opt82_format_type"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE	"dhcp_snp_set_opt82_fill_format_type"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT	"dhcp_snp_set_opt82_remoteid_content"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT	"dhcp_snp_set_opt82_port_circuitid_content"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		ret -- opeartion result
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT	"dhcp_snp_set_opt82_port_remoteid_content"


/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY	"dhcp_snp_set_opt82_port_strategy"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_ADD_DEL_STATIC_BINDING
 *
 * INPUT:
 *		unsigned int opt_type				- operate type, 0: add 1: delete
 *		unsigned int ip						- IP Address, AS A.B.C.D
 *		unsigned char macAddr.arEther[0]	- MAC Address, AS H:H:H:H:H:H
 *		unsigned char macAddr.arEther[1]
 *		unsigned char macAddr.arEther[2]
 *		unsigned char macAddr.arEther[3]
 *		unsigned char macAddr.arEther[4]
 *		unsigned char macAddr.arEther[5]
 *		unsigned short vlanId				- vlanId, <1-4094>
 *		unsigned char slot_no				- slot no of panel port
 *		unsigned char local_port_no			- local port no of panel port
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_ADD_DEL_STATIC_BINDING		"dhcp_snp_add_delete_static_binding"

/*****************************************************************
 * DESCRIPTION:
 * 		arg lists for method NPD_DBUS_DHCP_SNP_METHOD_ADD_DYNAMIC_BINDING
 *		inner interface in dhcp snooping, for save content of bindint table
 *		by command "write" before reboot
 *
 * INPUT:
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_ADD_BINDING		"dhcp_snp_add_binding"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG
 *
 * INPUT:
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG	"dhcp_snp_show_running_global_config"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_VLAN_CONFIG
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_VLAN_CONFIG	"dhcp_snp_show_running_vlan_config"

#define HBIP_DBUS_BUSNAME "aw.hbip"
	
#define HBIP_DBUS_OBJPATH "/aw/hbip"
#define HBIP_DBUS_INTERFACE "aw.hbip"
/*****************************************************************
 * DESCRIPTION:
 * 	start one hbip configuration
 * INPUT:
 *    uint32--profile,
 *     char* --primary_ifname,
 *     char* --secondary_ifname
 *
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define HBIP_DBUS_METHOD_START_HBIP   "hbip_start"

/*****************************************************************
 * DESCRIPTION:
 * 	start one hbip configuration
 * INPUT:
 *   uint32--profile
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define HBIP_DBUS_METHOD_STOP_HBIP   "hbip_stop"


/*****************************************************************
 * DESCRIPTION:
 * 	show hbip configuration
 * INPUT:
 *   uint32-- profileid
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define HBIP_DBUS_METHOD_SHOW          "hbip_show"

/*****************************************************************
 * DESCRIPTION:
 * 	show hbip configuration saved
 * INPUT:
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/
#define HBIP_DBUS_METHOD_SHOW_RUNNING          "hbip_show_running"

/*****************************************************************
 * DESCRIPTION:
 *     set hbip debug level
 * INPUT:
 *
 * OUTPUT:
 *		uint--op_ret
 *		
*****************************************************************/

#define HBIP_DBUS_METHOD_BRG_DBUG_HBIP "hbip_debug"
#define HBIP_DBUS_METHOD_BRG_NO_DBUG_HBIP "hbip_no_debug"
/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method
 *		NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_SAVE_BIND_TABLE
 *
 * INPUT:
 *
 *		NONE
 *
 * OUTPUT:
 *		NONE
 *		
*****************************************************************/
#define NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_SAVE_BIND_TABLE	"dhcp_snp_show_running_save_bind_table"

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method NPD_DBUS_SYSTEM_DIAGNOSIS_EEPROM_HW_RW_REG
 *
 * INPUT:
 *		unsigned int opType		- 0:read  1: write
 *		unsigned char twsi_channel	- TWSI channel
 *		unsigned int eeprom_addr	- eeprom address
 *		unsigned int eeprom_type	- eeprom type 
 *		unsigned int validOffset		- whether the slave has offset (i.e. Eeprom  etc.), true: valid false: in valid
 *		unsigned int moreThan256	- whether the ofset is bigger than 256, true: valid false: in valid 
 *		unsigned int regAddr		- address of eeprom's register
 *
 * OUTPUT:
 *		unit32	ret
 *		unsigned char regValue		-value of eeprom's register
 *****************************************************************/
#define NPD_DBUS_SYSTEM_DIAGNOSIS_EEPROM_HW_RW_REG	"Diagnosis_eeprom_hw_rw_reg"
/****************************
 *
 * EAG
 *
 ****************************/

/*****************************************************************
 * DESCRIPTION:
 * 	arg lists for method PORTAL_HA_DBUS_METHOD_EAG_DEBUG_ON 
 *					 	 PORTAL_HA_DBUS_METHOD_EAG_DEBUG_OFF
 * INPUT:
 *	all|error|warning|debug|event
 *		all		: Debug eag all
 *		error	: Debug eag error
 *		warning	: Debug eag warning
 *		debug	: Debug eag debug
 *		event	: Debug eag event
 *		receive	: Debug eag receive packet
 *		send	: Debug eag send packet
 *
 * OUTPUT:
 *		NONE
 *		
 *****************************************************************/
#define PORTAL_HA_DBUS_METHOD_EAG_DEBUG_ON		"portal_ha_debug"
#define PORTAL_HA_DBUS_METHOD_EAG_DEBUG_OFF		"no_portal_ha_debug"
#define VRRP_DBUS_METHOD_SNMP_GET_VRRP_STATE	"vrrp_snmp_get_vrrp_state"
/*  Protocol-based vlan about */
#define NPD_DBUS_PROT_VLAN_OBJPATH "/aw/npd/prot_vlan"
#define NPD_DBUS_PROT_VLAN_INTERFACE "aw.npd.prot_vlan"

#define NPD_DBUS_PROT_VLAN_METHOD_PORT_ENABLE_CONFIG "prot_vlan_port_enable_config"

#define NPD_DBUS_PROT_VLAN_METHOD_CONFIG_VID_BY_PORT_ENTRY "prot_vlan_config_vid_by_port_entry"

#define NPD_DBUS_PROT_VLAN_METHOD_NO_VID_BY_PORT_ENTRY "prot_vlan_no_vid_by_port_entry"

#define NPD_DBUS_PROT_VLAN_METHOD_SHOW_PORT_PROT_VLAN  "prot_vlan_show_port_prot_vlan"

#define NPD_DBUS_PROT_VLAN_METHOD_SHOW_PORT_PROT_VLAN_LIST "prot_vlan_show_port_prot_vlan_list"

#define NPD_DBUS_PROT_VLAN_METHOD_CONFIG_UDF_ETHTYPE_VALUE "prot_vlan_config_ethtype_value"

/* For hardware test cmd */
#define NPD_DBUS_SYSTEM_CPU_TEMPERATURE_TEST    "cpu_temperature_test"
#define NPD_DBUS_SYSTEM_LION1_TRUNK_TEST        "lion1_trunk_test"
#define NPD_DBUS_SYSTEM_TRUNK_PORT_TEST			"trunk_port_test"
#define NPD_DBUS_SYSTEM_GE_XG_TEST				"ge_xg_port_test"
#define NPD_DBUS_SYSTEM_AX8610_PRBS_TEST		"ax8610_prbs_test"
#define NPD_DBUS_SYSTEM_MCB_STATE_TEST				"mcb_state_change_test"
/* add for asic debug  */
#define NPD_DBUS_SYSTEM_DIAG_CREATE_VLAN				"diagnosis_create_vlan"
#define NPD_DBUS_SYSTEM_DIAG_VLAN_ADD_DEL_PORT				"diagnosis_vlan_add_del_port"
#define NPD_DBUS_SYSTEM_DIAG_READ_ASIC_MIB				    "diagnosis_read_asic_mib"
#define NPD_DBUS_SYSTEM_DIAG_ENDIS_ASIC				    "diagnosis_endis_asic"
#define NPD_DBUS_SYSTEM_DIAG_PORT_CSCD_MODE_SET				    "diagnosis_port_cscd_mode_set"
#define NPD_DBUS_SYSTEM_DIAG_VLAN_FDB_DELETE            "diagnosis_vlan_fdb_delete"
#define NPD_DBUS_SYSTEM_DIAG_SHOW_CSCD_PORT_STATUS            "diagnosis_show_cscd_port_status"
#define NPD_DBUS_SYSTEM_DIAG_BOARD_TEST				    "diagnosis_board_test"


/* add for sfd command;In file dcli_fdb.c*/
#define NPD_DBUS_SFD_SHOW_INFO				"show_sfd_info"
#define NPD_DBUS_SFD_SERVICE_DEBUG				"service_sfd_debug"
#define NPD_DBUS_SFD_SERVICE				"service_sfd"


/* Add for update the g_vlan_list[] on SMU */
#define NPD_DBUS_VLAN_METHOD_CONFIG_VLANLIST_PORT_MEMBER_ADD_DEL    "vlan_config_vlanlist_port_add_del"
/* Add for show vlan port-member list of distributed system */
#define NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_DISTRIBUTED     "vlan_port_member_list_distributed"
/* Add for Bond vlan to cpu port on special slot of distributed system */
#define NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU     "vlan_to_cpu_port_add_del"
/* check exist interface under special vlan on special slot of distributed system */
#define NPD_DBUS_VLAN_EXIST_INTERFACE_UNDER_VLAN_TO_SLOT_CPU     "vlan_to_cpu_exist_interface_under_vlan_on_slot"

/*Add for L2 and smart_link for had by sunjc@autelan.com*/
#define VRRP_DBUS_METHOD_L2_UPLINK_ADD_DELETE	"had_l2_uplink_ifname_add_delete"
#define VRRP_DBUS_METHOD_VIP_BACK_DOWN_FLG_SET  "had_vip_back_flag_set"
/* use in dcli_acl.c */
#define	SLOT_PORT_ANALYSIS_SLOT(combination, slot) 	(slot = (((combination)>>6) & 0x1f) + 1)
#define VRRP_DBUS_METHOD_SHOW_SWITCH_TIMES      "had_show_switch_times"
#endif
