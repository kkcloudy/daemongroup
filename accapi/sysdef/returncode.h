#ifndef __RETURNCODE_H__
#define __RETURNCODE_H__
/*
 * This file defines all common return codes 
 * which is used across modules.
 *
 * RETURN_CODE is always a unsigned long integer
 * All available values is devided into different sections to 
 * satisfy different modules requirements.
 *
 * RETURN CODE ASSIGNMENT IN NPD PROCESS IS AS FOLLOWS:
 * 	0x00010000		- ACL branch
 * 	0x00020000		- ARP branch
 * 	0x00030000		- CPLD branch
 * 	0x00040000		- DIAG branch
 * 	0x00050000		- EEPROM branch
 * 	0x00060000		- ETH-PORT branch
 * 	0x00070000		- FDB branch
 * 	0x00080000		- IGMP SNOOPING branch
 * 	0x00090000		- INTERFACE branch
 * 	0x000A0000		- MIRROR branch
 * 	0x000B0000		- PACKET TX/RX branch
 * 	0x000C0000		- PVLAN branch
 * 	0x000D0000		- QOS branch
 * 	0x000E0000		- ROUTE branch
 * 	0x000F0000		- STP/RSTP/MSTP branch
 * 	0x00100000		- TRUNK(LAG) branch
 * 	0x00110000		- UTILITY branch
 * 	0x00120000		- VLAN branch
 * 	0x00130000		- TUNNEL branch
 * 	0x00140000		- DLDP branch
 * 	0x00150000		- VRRP branch
 * 	0x00160000		- DHCP SNOOPING branch
 * 	0x00170000		- DHCP SERVER branch
 * 	....
 *
 * */
/*Common branch*/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define COMMON_SUCCESS						(0x0)								/* common success*/
#define COMMON_RETURN_CODE_BASE				(0x0)								/* common base*/
#define COMMON_ERROR						(COMMON_RETURN_CODE_BASE + 0x1)		/* common general error*/
#define COMMON_RETURN_CODE_NULL_PTR			(COMMON_RETURN_CODE_BASE + 0x2)		/* common null pointer error*/
#define COMMON_RETURN_CODE_NO_RESOURCE		(COMMON_RETURN_CODE_BASE + 0x3)		/* common no resource error*/
#define COMMON_RETURN_CODE_BADPARAM			(COMMON_RETURN_CODE_BASE + 0x4)		/* common bad parameter*/
#define COMMON_FAIL							(COMMON_RETURN_CODE_BASE + 0xFFFF)	/* common fail*/
/*function not support*/
#define COMMON_PRODUCT_NOT_SUPPORT_FUCTION  (COMMON_RETURN_CODE_BASE + 0x5)


/* ACL branch */
#define ACL_RETURN_CODE_BASE						(0x10000)
#define ACL_RETURN_CODE_SUCCESS 					COMMON_SUCCESS
#define ACL_RETURN_CODE_ERROR 						COMMON_ERROR
#define ACL_RETURN_CODE_GROUP_SUCCESS				(ACL_RETURN_CODE_BASE)
#define ACL_RETURN_CODE_GROUP_EXISTED				(ACL_RETURN_CODE_BASE + 1)
#define ACL_RETURN_CODE_GROUP_NOT_EXISTED 			(ACL_RETURN_CODE_BASE + 16)
#define ACL_RETURN_CODE_GROUP_PORT_NOTFOUND			(ACL_RETURN_CODE_BASE + 3)
#define ACL_RETURN_CODE_GROUP_RULE_NOTEXISTED		(ACL_RETURN_CODE_BASE + 4)
#define ACL_RETURN_CODE_GROUP_RULE_EXISTED			(ACL_RETURN_CODE_BASE + 5)
#define ACL_RETURN_CODE_NPD_DBUS_ERR_GENERAL		(ACL_RETURN_CODE_BASE + 6)
#define ACL_RETURN_CODE_GROUP_PORT_BINDED    		(ACL_RETURN_CODE_BASE + 7)
#define ACL_RETURN_CODE_GROUP_NOT_SHARE         	(ACL_RETURN_CODE_BASE + 8)
#define ACL_RETURN_CODE_GLOBAL_NOT_EXISTED          (ACL_RETURN_CODE_BASE + 9)
#define ACL_RETURN_CODE_GLOBAL_EXISTED              (ACL_RETURN_CODE_BASE + 10)
#define ACL_RETURN_CODE_SAME_FIELD				    (ACL_RETURN_CODE_BASE + 11)
#define ACL_RETURN_CODE_EXT_NO_SPACE				(ACL_RETURN_CODE_BASE + 12)
#define ACL_RETURN_CODE_GROUP_NOT_BINDED			(ACL_RETURN_CODE_BASE + 13)
#define ACL_RETURN_CODE_GROUP_VLAN_BINDED           (ACL_RETURN_CODE_BASE + 14)
#define ACL_RETURN_CODE_GROUP_EGRESS_ERROR          (ACL_RETURN_CODE_BASE + 17)
#define ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED   (ACL_RETURN_CODE_BASE + 18)
#define ACL_RETURN_CODE_GROUP_EGRESS_NOT_SUPPORT    (ACL_RETURN_CODE_BASE + 19)
#define ACL_RETURN_CODE_GROUP_WRONG_INDEX		 	(ACL_RETURN_CODE_BASE + 20)
#define ACL_RETURN_CODE_ON_PORT_DISABLE         	(ACL_RETURN_CODE_BASE + 21)
#define ACL_RETURN_CODE_POLICER_ID_NOT_SET      	(ACL_RETURN_CODE_BASE + 22)
#define ACL_RETURN_CODE_GROUP_SAME_ID		    	(ACL_RETURN_CODE_BASE + 23)
#define ACL_RETURN_CODE_RULE_TIME_NOT_SUPPORT         (ACL_RETURN_CODE_BASE + 24)
#define ACL_RETURN_CODE_RULE_INDEX_ERROR             (ACL_RETURN_CODE_BASE + 25)
#define ACL_RETURN_CODE_GROUP_INDEX_ERROR             (ACL_RETURN_CODE_BASE + 26)
#define ACL_RETURN_CODE_NORMAL_MALLOC_ERROR_NONE	 (ACL_RETURN_CODE_BASE + 100)
#define ACL_RETURN_CODEL_MIRROR_USE					(ACL_RETURN_CODE_BASE + 27)
#define ACL_RETURN_CODE_RULE_EXT_ONLY					(ACL_RETURN_CODE_BASE + 28)
#define ACL_RETURN_CODE_UNBIND_FRIST					(ACL_RETURN_CODE_BASE + 29)
#define ACL_RETURN_CODE_ADD_EQUAL_RULE					(ACL_RETURN_CODE_BASE + 30)
#define ACL_RETURN_CODEL_RANGE_NOT_EXIST					(ACL_RETURN_CODE_BASE + 31)
#define ACL_RETURN_CODE_UDP_VLAN_RULE_ENABLE				(ACL_RETURN_CODE_BASE + 32)
#define ACL_RETURN_CODE_PORT_NOT_SUPPORT_BINDED				(ACL_RETURN_CODE_BASE + 33)
#define ACL_RETURN_CODE_ENABLE_FIRST						(ACL_RETURN_CODE_BASE + 34)
#define ACL_RETURN_CODE_ALREADY_PORT						(ACL_RETURN_CODE_BASE + 35)
#define ACL_RETURN_CODE_ALREADY_FLOW						(ACL_RETURN_CODE_BASE + 35)
#define ACL_RETURN_CODE_ALREADY_HYBRID						(ACL_RETURN_CODE_BASE + 36)
#define ACL_RETURN_CODE_NO_QOS_MODE						(ACL_RETURN_CODE_BASE + 37)
#define ACL_RETURN_CODE_HYBRID_DSCP						(ACL_RETURN_CODE_BASE + 38)
#define ACL_RETURN_CODE_HYBRID_UP						(ACL_RETURN_CODE_BASE + 39)
#define ACL_RETURN_CODE_HYBRID_FLOW						(ACL_RETURN_CODE_BASE + 40)
#define ACL_RETURN_CODE_STD_RULE					(ACL_RETURN_CODE_BASE + 41)
#define ACL_RETURN_CODE_BAD_k					(ACL_RETURN_CODE_BASE + 42)
#define ACL_RETURN_CODE_BAD_M					(ACL_RETURN_CODE_BASE + 43)
#define ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT	(ACL_RETURN_CODE_BASE + 44)

/* ARP branch 0x20000 -- 0x2FFFF*/		
#define ARP_RETURN_CODE_BASE				(0x20000)		        		/* arp base code */
#define ARP_RETURN_CODE_SUCCESS				(COMMON_SUCCESS)				/* arp success return code*/ 
#define ARP_RETURN_CODE_DUPLICATED			(ARP_RETURN_CODE_BASE + 0x1)	/* duplicate arp found */
#define ARP_RETURN_CODE_NOTEXISTS			(ARP_RETURN_CODE_BASE + 0x2)	/* the arp item not found */
#define ARP_RETURN_CODE_ACTION_TRAP2CPU		(ARP_RETURN_CODE_BASE + 0x3)	/* trap to cpu */
#define ARP_RETURN_CODE_NOTCONSISTENT		(ARP_RETURN_CODE_BASE + 0x4)	/* not consistent */
#define ARP_RETURN_CODE_TABLE_FULL			(ARP_RETURN_CODE_BASE + 0x5)	/* arp table is full */
#define ARP_RETURN_CODE_STATIC_EXIST		(ARP_RETURN_CODE_BASE + 0x6)	/* static arp item exists */
#define ARP_RETURN_CODE_STASTIC_NOTEXIST	(ARP_RETURN_CODE_BASE + 0x7)	/* static arp item does not exist */
#define ARP_RETURN_CODE_PORT_NOTMATCH		(ARP_RETURN_CODE_BASE + 0x8)	/* the port is not match when delete static arp */
#define ARP_RETURN_CODE_KERN_CREATE_FAILED	(ARP_RETURN_CODE_BASE + 0x9)	/* static arp kernal create failed */
#define ARP_RETURN_CODE_ACTION_HARD_DROP	(ARP_RETURN_CODE_BASE + 0xA)	/* hard drop */
#define ARP_RETURN_CODE_STATIC_ARP_FULL		(ARP_RETURN_CODE_BASE + 0xB)	/* the static arp is full : reach to 1024 */		
#define ARP_RETURN_CODE_HASH_OP_FAILED		(ARP_RETURN_CODE_BASE + 0xC)    /* arp hash table operation failed */
#define ARP_RETURN_CODE_NAM_ERROR			(ARP_RETURN_CODE_BASE + 0xD)    /* operation of nam error,maybe it's asic set failed*/
#define ARP_RETURN_CODE_NULL_PTR			(ARP_RETURN_CODE_BASE + 0xE)    /* null point error */
#define ARP_RETURN_CODE_NORESOURCE			(ARP_RETURN_CODE_BASE + 0xF)    /* no enough resource,eg. no enough memory*/
#define ARP_RETURN_CODE_BADPARAM			(ARP_RETURN_CODE_BASE + 0x10)   /* bad parameter when function call*/
#define ARP_RETURN_CODE_TBLINDEX_GET_FAILED	(ARP_RETURN_CODE_BASE + 0x11)	/* index alloc failed */
#define ARP_RETURN_CODE_INDEX_FREE_FAILED	(ARP_RETURN_CODE_BASE + 0x12)	/* index free failed */
#define ARP_RETURN_CODE_TRUNK_NOTMATCH		(ARP_RETURN_CODE_BASE + 0x13)	/* the trunk id is not match when delete static arp */
#define ARP_RETURN_CODE_CHECK_IP_ERROR		(ARP_RETURN_CODE_BASE + 0x14)  	/* check ip error when create static arp */
#define ARP_RETURN_CODE_NO_HAVE_ANY_IP		(ARP_RETURN_CODE_BASE + 0x15)	/* no have any ip when set static arp  */
#define ARP_RETURN_CODE_HAVE_THE_IP			(ARP_RETURN_CODE_BASE + 0x16)	/* have the same ip with which we want to set, when set static arp */
#define ARP_RETURN_CODE_NOT_SAME_SUB_NET	(ARP_RETURN_CODE_BASE + 0x17)	/* the ip we want to set is not in the same sub net with the intf,when set static arp */
#define ARP_RETURN_CODE_INTERFACE_NOTEXIST	(ARP_RETURN_CODE_BASE + 0x18)	/* interface not exists when arp operation */
#define ARP_RETURN_CODE_PORT_NOT_IN_VLAN	(ARP_RETURN_CODE_BASE + 0x19)   /* the port is not in the vlan when static arp operation */
#define ARP_RETURN_CODE_MAC_MATCHED_BASE_MAC	(ARP_RETURN_CODE_BASE + 0x1A)	/* config mac matched system mac */
#define ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC	(ARP_RETURN_CODE_BASE + 0x1B)	/* config mac matched interface's mac */
#define ARP_RETURN_CODE_FD_ERROR			(ARP_RETURN_CODE_BASE + 0x1C)	/* fd invalidate */
#define ARP_RETURN_CODE_NO_SUCH_PORT		(ARP_RETURN_CODE_BASE + 0x1D)	/* no such port */
#define ARP_RETURN_CODE_VLAN_NOTEXISTS		(ARP_RETURN_CODE_BASE + 0x1E)	/* vlan not exists */
#define ARP_RETURN_CODE_UNSUPPORTED_COMMAND	(ARP_RETURN_CODE_BASE + 0x1F)	/* unsupported command */
#define ARP_RETURN_CODE_TRUNK_NOT_EXISTS	(ARP_RETURN_CODE_BASE + 0x20)	/* trunk not exists when add or del arp */
#define ARP_RETURN_CODE_TRUNK_NOT_IN_VLAN	(ARP_RETURN_CODE_BASE + 0x21)	/* trunk not in the vlan when static arp operation */
#define ARP_RETURN_CODE_NO_VID              (ARP_RETURN_CODE_BASE + 0x22)   /* no vlanid parameter when config static arp*/
#define ARP_RETURN_CODE_PORT_OR_TRUNK_NEEDED    (ARP_RETURN_CODE_BASE + 0x23) /* eth-port or trunk id needed when config static arp */
#define ARP_RETURN_CODE_PORT_OR_TRUNK_NOT_NEEDED     (ARP_RETURN_CODE_BASE + 0x24)   /* port is not needed config static arp for vlan advanced-routing*/
/*wangchao add*/
#define ARP_RETURN_CPU_INTERFACE_CODE_SUCCESS     (ARP_RETURN_CODE_BASE + 0x25) /*used in mng interaface when download to kernel successed*/
#define ARP_RETURN_CODE_SET_STALE_TIME_ERR	(ARP_RETURN_CODE_BASE + 0X26)      /*set arp stale time error*/
#define ARP_RETURN_ARP_ENTRY_NOT_EXISTED	(ARP_RETURN_CODE_BASE + 0X27)	 /*the arp entry would deleted not exsited*/
#define ARP_RETURN_CODE_NO_HAVE_THE_IP		(ARP_RETURN_CODE_BASE)			/* not have the same ip ,but in the same sub net,That's ok */
#define ARP_RETURN_CODE_ERROR				(ARP_RETURN_CODE_BASE + 0xFFFF)	/* error occured ,maybe unknow error*/
#define ARP_RETURN_CODE_MAX					(ARP_RETURN_CODE_BASE + 0xFFFF)	/* the max return code of arp module */

/* CPLD branch */
#define CPLD_RETURN_CODE_BASE				(0x30000)
#define CPLD_RETURN_CODE_OPEN_FAIL			(CPLD_RETURN_CODE_BASE + 0x01)
#define CPLD_RETURN_CODE_IOCTL_FAIL			(CPLD_RETURN_CODE_BASE + 0x02)

/* DIAG branch */
#define DIAG_RETURN_CODE_BASE			(0x40000)
#define DIAG_RETURN_CODE_SUCCESS 					COMMON_SUCCESS
#define DIAG_RETURN_CODE_ERROR 						COMMON_ERROR

/* EEPROM branch */
#define EEPROM_RETURN_CODE_BASE			        (0x50000)
#define EEPROM_RETURN_CODE_ NPD_SUCCESS        	(EEPROM_RETURN_CODE_BASE + 0x1)  /* express operate  success */
#define EEPROM_RETURN_CODE_NPD_FAIL            	(EEPROM_RETURN_CODE_BASE + 0x2)  /* express operate fail */

/* ETH-PORT branch */
#define ETHPORT_RETURN_CODE_BASE            (0x60000)
#define ETHPORT_RETURN_CODE_ERR_NONE      	(COMMON_SUCCESS)       			   /* success */   
#define ETHPORT_RETURN_CODE_ERR_GENERAL     (ETHPORT_RETURN_CODE_BASE + 0x1)   /* general failure */
#define ETHPORT_RETURN_CODE_NO_SUCH_PORT    (ETHPORT_RETURN_CODE_BASE + 0x2)   
#define ETHPORT_RETURN_CODE_NO_SUCH_TRUNK   (ETHPORT_RETURN_CODE_BASE + 0x3)  
#define ETHPORT_RETURN_CODE_NO_SUCH_VLAN    (ETHPORT_RETURN_CODE_BASE + 0x4) 
#define ETHPORT_RETURN_CODE_NO_SUCH_GROUP   (ETHPORT_RETURN_CODE_BASE + 0x5)
#define ETHPORT_RETURN_CODE_UNSUPPORT      	(ETHPORT_RETURN_CODE_BASE + 0x6)   
#define ETHPORT_RETURN_CODE_FLOWCTL_NODE    (ETHPORT_RETURN_CODE_BASE + 0x7)   
#define ETHPORT_RETURN_CODE_BACKPRE_NODE    (ETHPORT_RETURN_CODE_BASE + 0x8)   
#define ETHPORT_RETURN_CODE_DUPLEX_NODE     (ETHPORT_RETURN_CODE_BASE + 0x9)   
#define ETHPORT_RETURN_CODE_SPEED_NODE      (ETHPORT_RETURN_CODE_BASE + 0xA)  
#define ETHPORT_RETURN_CODE_NO_PVE        	(ETHPORT_RETURN_CODE_BASE + 0xB)   
#define ETHPORT_RETURN_CODE_ENABLE_FIRST    (ETHPORT_RETURN_CODE_BASE + 0xC)   
#define ETHPORT_RETURN_CODE_ALREADY_PORT    (ETHPORT_RETURN_CODE_BASE + 0xD)   
#define ETHPORT_RETURN_CODE_ALREADY_FLOW	(ETHPORT_RETURN_CODE_BASE + 0xE)   
#define ETHPORT_RETURN_CODE_ALREADY_HYBRID  (ETHPORT_RETURN_CODE_BASE + 0xF)   
#define ETHPORT_RETURN_CODE_NO_QOS_MODE		(ETHPORT_RETURN_CODE_BASE + 0x10)  
#define ETHPORT_RETURN_CODE_HYBRID_DSCP     (ETHPORT_RETURN_CODE_BASE + 0x11)  
#define ETHPORT_RETURN_CODE_HYBRID_UP		(ETHPORT_RETURN_CODE_BASE + 0x12) 
#define ETHPORT_RETURN_CODE_HYBRID_FLOW     (ETHPORT_RETURN_CODE_BASE + 0x13)
#define ETHPORT_RETURN_CODE_STD_RULE		(ETHPORT_RETURN_CODE_BASE + 0x14)
#define ETHPORT_RETURN_CODE_BAD_VALUE		(ETHPORT_RETURN_CODE_BASE + 0x15)
#define ETHPORT_RETURN_CODE_DUPLEX_MODE		(ETHPORT_RETURN_CODE_BASE + 0x16)
#define ETHPORT_RETURN_CODE_BAD_k		    (ETHPORT_RETURN_CODE_BASE + 0x17)
#define ETHPORT_RETURN_CODE_BAD_M		    (ETHPORT_RETURN_CODE_BASE + 0x18)
#define ETHPORT_RETURN_CODE_BAD_IPG		    (ETHPORT_RETURN_CODE_BASE + 0x19)
#define ETHPORT_RETURN_CODE_BOARD_IPG		(ETHPORT_RETURN_CODE_BASE + 0x1A)
#define ETHPORT_RETURN_CODE_ETH_GE_SFP		(ETHPORT_RETURN_CODE_BASE + 0x1B)
#define ETHPORT_RETURN_CODE_NOT_SUPPORT		(ETHPORT_RETURN_CODE_BASE + 0x1C)
#define ETHPORT_RETURN_CODE_ERR_OPERATE		(ETHPORT_RETURN_CODE_BASE + 0x1D)
#define ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL	(ETHPORT_RETURN_CODE_BASE + 0x1E)
#define ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF	(ETHPORT_RETURN_CODE_BASE + 0x1F)
#define ETHPORT_RETURN_CODE_ERR_HW          (ETHPORT_RETURN_CODE_BASE + 0x20)
#define ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE    (ETHPORT_RETURN_CODE_BASE + 0x21)

/* FDB branch */
#define FDB_RETURN_CODE_BASE               	(0x70000)                      	    /*return code base used in fdb modle*/     
#define FDB_RETURN_CODE_ERR_DBUS           	(FDB_RETURN_CODE_BASE + 0x1)        /*return error occured reference to dbus*/
#define FDB_RETURN_CODE_GENERAL            	(FDB_RETURN_CODE_BASE + 0x2)        /*return error occured not the following reason*/
#define FDB_RETURN_CODE_BADPARA            	(FDB_RETURN_CODE_BASE + 0x3)        /*bad parameters*/
#define FDB_RETURN_CODE_OCCUR_HW           	(FDB_RETURN_CODE_BASE + 0x4)        /*hardware err*/
#define FDB_RETURN_CODE_ITEM_ISMIRROR      	(FDB_RETURN_CODE_BASE + 0x5)        /*item is mirrored*/
#define FDB_RETURN_CODE_NODE_EXIST         	(FDB_RETURN_CODE_BASE + 0x6)        /*node has already exist*/
#define FDB_RETURN_CODE_NODE_NOT_EXIST     	(FDB_RETURN_CODE_BASE + 0x7)         /*node not exist*/
#define FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN (FDB_RETURN_CODE_BASE + 0x8)        /*port not exist in vlan*/
#define FDB_RETURN_CODE_NODE_VLAN_NONEXIST   (FDB_RETURN_CODE_BASE + 0x9)        /* vlan not exist */
#define FDB_RETURN_CODE_SYSTEM_MAC         	(FDB_RETURN_CODE_BASE + 0xA)        /*conflict with system mac*/
#define FDB_RETURN_CODE_HW_NOT_SUPPORT     	(FDB_RETURN_CODE_BASE + 0xB)        /*hardware not support this function*/
#define FDB_RETURN_CODE_MAX                	(FDB_RETURN_CODE_BASE + 0xC)        /*reserved value*/
#define FDB_RETURN_CODE_MALLOC             	(FDB_RETURN_CODE_BASE + 0xD)        /*malloc failed*/


/* IGMP-SNOOPING branch */ 
#define	IGMPSNP_RETURN_CODE_BASE    			(0x80000)							/* return code base  */  
#define	IGMPSNP_RETURN_CODE_OK					(IGMPSNP_RETURN_CODE_BASE)	        /* success   */   
#define	IGMPSNP_RETURN_CODE_ERROR				(IGMPSNP_RETURN_CODE_BASE + 0x1)	/* error     */ 
#define	IGMPSNP_RETURN_CODE_ALREADY_SET			(IGMPSNP_RETURN_CODE_BASE + 0x2)	/* already been setted */   
#define	IGMPSNP_RETURN_CODE_ENABLE_GBL			(IGMPSNP_RETURN_CODE_BASE + 0x3)	/* IGMP_Snooping enabled global */   
#define	IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL		(IGMPSNP_RETURN_CODE_BASE + 0x4)	/* IGMP_Snooping not enabled global  */ 
#define	IGMPSNP_RETURN_CODE_OUT_RANGE			(IGMPSNP_RETURN_CODE_BASE + 0x5)	/* timer value or count out of range  */
#define	IGMPSNP_RETURN_CODE_SAME_VALUE			(IGMPSNP_RETURN_CODE_BASE + 0x6)	/* set same value to IGMP_Snooping timers*/ 
#define	IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST	(IGMPSNP_RETURN_CODE_BASE + 0x7)	/* mc vlan not exixt   */
#define	IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST		(IGMPSNP_RETURN_CODE_BASE + 0x8)	/* L2 vlan not exixt   */
#define	IGMPSNP_RETURN_CODE_NOTENABLE_VLAN		(IGMPSNP_RETURN_CODE_BASE + 0x9)	/* L2 vlan not enable IGMP_Snooping  */
#define	IGMPSNP_RETURN_CODE_HASENABLE_VLAN		(IGMPSNP_RETURN_CODE_BASE + 0xA)	/* L2 vlan has enable IGMP_Snooping  */
#define	IGMPSNP_RETURN_CODE_NO_SUCH_VLAN		(IGMPSNP_RETURN_CODE_BASE + 0xB)	/* illegal vlan */
#define IGMPSNP_RETURN_CODE_NOT_SUPPORT_IGMP_SNP (IGMPSNP_RETURN_CODE_BASE + 0xC)	/* vlan not support IGMP_Snooping*/
#define	IGMPSNP_RETURN_CODE_PORT_NOT_EXIST		(IGMPSNP_RETURN_CODE_BASE + 0xD)	/* port not exixt   */
#define	IGMPSNP_RETURN_CODE_NOTENABLE_PORT		(IGMPSNP_RETURN_CODE_BASE + 0xE)	/* port not enable IGMP_Snooping  */
#define	IGMPSNP_RETURN_CODE_HASENABLE_PORT		(IGMPSNP_RETURN_CODE_BASE + 0xF)	/* port has enable IGMP_Snooping  */
#define	IGMPSNP_RETURN_CODE_ROUTE_PORT_EXIST	(IGMPSNP_RETURN_CODE_BASE + 0x10)	/* vlan has exist router port */
#define	IGMPSNP_RETURN_CODE_ROUTE_PORT_NOTEXIST	(IGMPSNP_RETURN_CODE_BASE + 0x11)	/* vlan has not exist router port */
#define	IGMPSNP_RETURN_CODE_NOTROUTE_PORT		(IGMPSNP_RETURN_CODE_BASE + 0x12)	/* port not configured as router port */
#define	IGMPSNP_RETURN_CODE_PORT_TRUNK_MBR		(IGMPSNP_RETURN_CODE_BASE + 0x13)	/* port is trunk member */
#define	IGMPSNP_RETURN_CODE_NO_SUCH_PORT		(IGMPSNP_RETURN_CODE_BASE + 0x14)	/* illegal slot or port */
#define	IGMPSNP_RETURN_CODE_GROUP_NOTEXIST		(IGMPSNP_RETURN_CODE_BASE + 0x15)	/* multicast group not exist */
#define	IGMPSNP_RETURN_CODE_ERROR_HW			(IGMPSNP_RETURN_CODE_BASE + 0x16)	/* set hardware error */
#define	IGMPSNP_RETURN_CODE_ERROR_SW			(IGMPSNP_RETURN_CODE_BASE + 0x17)	/* check software info error */
#define	IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL		(IGMPSNP_RETURN_CODE_BASE + 0x18)	/* alloc memory null   */ 
#define	IGMPSNP_RETURN_CODE_NULL_PTR			(IGMPSNP_RETURN_CODE_BASE + 0x19)	/* parameter pointer is null  */ 
#define	IGMPSNP_RETURN_CODE_GET_VLANLIFE_E		(IGMPSNP_RETURN_CODE_BASE + 0x1A)	/* get IGMP_Snooping vlan lifetime error */ 
#define	IGMPSNP_RETURN_CODE_GET_GROUPLIFE_E		(IGMPSNP_RETURN_CODE_BASE + 0x1B)	/* get IGMP_Snooping group lifetime error */ 
#define	IGMPSNP_RETURN_CODE_GET_HOSTLIFE_E		(IGMPSNP_RETURN_CODE_BASE + 0x1C)	/* get IGMP_Snooping host lifetime error */ 
#define	IGMPSNP_RETURN_CODE_GET_ROBUST_E		(IGMPSNP_RETURN_CODE_BASE + 0x1D)	/* get IGMP_Snooping robust error */ 
#define	IGMPSNP_RETURN_CODE_GET_QUREY_INTERVAL_E (IGMPSNP_RETURN_CODE_BASE + 0x1E)	/* get IGMP_Snooping query interval error */ 
#define	IGMPSNP_RETURN_CODE_GET_RESP_INTERVAL_E	(IGMPSNP_RETURN_CODE_BASE + 0x1F)	/* get IGMP_Snooping response interval error */ 
#define	IGMPSNP_RETURN_CODE_GET_SOCKET_E		(IGMPSNP_RETURN_CODE_BASE + 0x20)   /* get IGMP_Snooping socket error */   
#define	IGMPSNP_RETURN_CODE_DBUS_CONNECTION_E	(IGMPSNP_RETURN_CODE_BASE + 0x21)   /* dbus connection error */
#define	IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR	(IGMPSNP_RETURN_CODE_BASE + 0x22)   /* create timer error */ 
#define	IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR		(IGMPSNP_RETURN_CODE_BASE + 0x23)   /* add timer error */ 

/* INTERFACE branch */		
#define INTF_TRUE									(0x1)									/* bool true for interface*/
#define INTF_FALSE									(0x0)									/* bool false for interface*/
#define INTERFACE_RETURN_CODE_SUCCESS				(COMMON_SUCCESS)									/* interface success*/
#define INTERFACE_RETURN_CODE_BASE					(0x90000)							  	/* interface base code */
#define INTERFACE_RETURN_CODE_UNKNOWN_ERROR			(INTERFACE_RETURN_CODE_BASE + 0x1)  	/* unknown error code */
#define INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND		(INTERFACE_RETURN_CODE_BASE + 0x2)  	/* unsupport this command */
#define INTERFACE_RETURN_CODE_VLAN_NOTEXIST			(INTERFACE_RETURN_CODE_BASE + 0x3)  	/* vlan does not exist when add or del intf */
#define INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST	(INTERFACE_RETURN_CODE_BASE + 0x4)  	/* interface does not exist */
#define INTERFACE_RETURN_CODE_INTERFACE_EXIST		(INTERFACE_RETURN_CODE_BASE + 0x5)  	/* interface exists */
#define INTERFACE_RETURN_CODE_FD_ERROR				(INTERFACE_RETURN_CODE_BASE + 0x6)		/* fd illegal eg.fd <= 0 */
#define INTERFACE_RETURN_CODE_IOCTL_ERROR			(INTERFACE_RETURN_CODE_BASE + 0x7)		/* ioctl operation failed or other error */
#define INTERFACE_RETURN_CODE_NAM_ERROR				(INTERFACE_RETURN_CODE_BASE + 0x8)		/* error occured in nam maybe driver operation failed */
#define INTERFACE_RETURN_CODE_CHECK_MAC_ERROR		(INTERFACE_RETURN_CODE_BASE + 0x9)		/* mac check error */
#define INTERFACE_RETURN_CODE_FDB_SET_ERROR			(INTERFACE_RETURN_CODE_BASE + 0xA)		/* fdb set error when create or del intf */
#define INTERFACE_RETURN_CODE_MAC_GET_ERROR			(INTERFACE_RETURN_CODE_BASE + 0xB)		/* get mac failed when create or del intf */
#define INTERFACE_RETURN_CODE_ALREADY_ADVANCED		(INTERFACE_RETURN_CODE_BASE + 0xC)		/* the interface already advanced-routing interface*/
#define INTERFACE_RETURN_CODE_TRUNK_NOT_IN_VLAN		(INTERFACE_RETURN_CODE_BASE + 0xD)		/* the trunk is not in the vlan ,when config static arp */
#define INTERFACE_RETURN_CODE_GET_SYSMAC_ERROR		(INTERFACE_RETURN_CODE_BASE + 0xE)		/* get sysmac failed */
#define INTERFACE_RETURN_CODE_ROUTE_CREATE_SUBIF 	(INTERFACE_RETURN_CODE_BASE + 0xF)		/* eth-port interface create sub interface */
#define INTERFACE_RETURN_CODE_VLAN_IF_CREATE_SUBIF	(INTERFACE_RETURN_CODE_BASE + 0x10)		/* vlan interface create sub interface */
#define INTERFACE_RETURN_CODE_SUBIF_NOTEXIST		(INTERFACE_RETURN_CODE_BASE + 0x11)		/* the subif not exists */
#define INTERFACE_RETURN_CODE_PORT_NOT_IN_VLAN 		(INTERFACE_RETURN_CODE_BASE + 0x12)		/* the port is not in the special vlan */
#define INTERFACE_RETURN_CODE_ADD_PORT_FAILED 		(INTERFACE_RETURN_CODE_BASE + 0x13)		/* add port to vlan for subif failed */
#define INTERFACE_RETURN_CODE_SUBIF_EXISTS			(INTERFACE_RETURN_CODE_BASE + 0x14)		/* the sub interface already exists when create */
#define INTERFACE_RETURN_CODE_SUBIF_CREATE_FAILED	(INTERFACE_RETURN_CODE_BASE + 0x15)		/* sub interface create failed */
#define INTERFACE_RETURN_CODE_PARENT_IF_NOTEXIST	(INTERFACE_RETURN_CODE_BASE + 0x16)  	/* the parent interface does not exist when subif operation */
#define INTERFACE_RETURN_CODE_CONTAIN_PROMI_PORT	(INTERFACE_RETURN_CODE_BASE + 0x17)		/* vlan contains promis port when it is not permitted*/
#define INTERFACE_RETURN_CODE_NOT_ADVANCED			(INTERFACE_RETURN_CODE_BASE + 0x18)		/* the interface is not advanced-routing interface when advanced-routing disable */
#define INTERFACE_RETURN_CODE_NO_SUCH_PORT			(INTERFACE_RETURN_CODE_BASE + 0x19)		/* no such port */
#define INTERFACE_RETURN_CODE_ALREADY_THIS_MODE		(INTERFACE_RETURN_CODE_BASE + 0x1A)		/* eth port already run in this mode */
#define INTERFACE_RETURN_CODE_PORT_HAS_SUB_IF		(INTERFACE_RETURN_CODE_BASE + 0x1B)		/* eth port has sub if when change its mode */
#define INTERFACE_RETURN_CODE_DEFAULT_VLAN_IS_L3_VLAN	(INTERFACE_RETURN_CODE_BASE + 0x1C)	/* the default valn is l3 vlan when config port to promis mode */
#define INTERFACE_RETURN_CODE_ADVAN_VLAN_SET2_INTF	(INTERFACE_RETURN_CODE_BASE + 0x1D)		/* vlan 4094 set to intf */
#define INTERFACE_RETURN_CODE_QINQ_TWO_SAME_TAG		(INTERFACE_RETURN_CODE_BASE + 0x1E)		/* the internal tag is same as the external tag when crate qinq subif */
#define INTERFACE_RETURN_CODE_PROMIS_PORT_TAG_IN_VLAN (INTERFACE_RETURN_CODE_BASE + 0x1F)	/* there are promis port in this vlan with tag mode */
#define INTERFACE_RETURN_CODE_VLAN_IS_L3INTF		(INTERFACE_RETURN_CODE_BASE + 0x20)		/* the vlan is l3 intf some config is not allowed */
#define INTERFACE_RETURN_CODE_CAN_NOT_SET2_EMPTY	(INTERFACE_RETURN_CODE_BASE + 0x21)		/* can't set advanced-routing default to empty,maybe there are some eth advanced-routing intfs */
#define INTERFACE_RETURN_CODE_TAG_IS_ADVACED_VID	(INTERFACE_RETURN_CODE_BASE + 0x22)		/* the tag is advanced-routing default-vid */
#define INTERFACE_RETURN_CODE_DEL_PORT_FAILED		(INTERFACE_RETURN_CODE_BASE + 0x23)		/* del port failed when config interface */
#define INTERFCE_RETURN_CODE_ADVANCED_VLAN_NOT_EXISTS (INTERFACE_RETURN_CODE_BASE + 0x24)	/* advanced-routing default vid is empty when create eth advanced routing interface */
#define INTERFACE_RETURN_CODE_PORT_CONFLICT         (INTERFACE_RETURN_CODE_BASE + 0x25)     /* the port is contained by other vlan when create eth-port interface */
#define INTERFACE_RETURN_CODE_QINQ_TYPE_FULL        (INTERFACE_RETURN_CODE_BASE + 0x26)     /* qinq type set MAX COUNT types */
#define INTERFACE_RETURN_CODE_SHORTEN_IFNAME_NOT_SUPPORT	(INTERFACE_RETURN_CODE_BASE + 0x27) /*this interface not support shorten ifname*/
#define INTERFACE_RETURN_CODE_INTERFACE_ISSLAVE		(INTERFACE_RETURN_CODE_BASE + 0x28)		/* the interface is added to dynamic trunk ,so can't set it enable now*/
#define INTERFACE_RETURN_CODE_FPGA_ETH_XGE_FIBER        (INTERFACE_RETURN_CODE_BASE + 0x29)     /*can't set AX81_1X12G12S XGE port */
#define INTERFACE_RETURN_CODE_ERROR					(INTERFACE_RETURN_CODE_BASE + 0xFFFF)	/* general error occured */
#define INTERFACE_RETURN_CODE_MAX					(INTERFACE_RETURN_CODE_BASE + 0xFFFF)	/* mac return code for interface */

/* MIRROR branch*/
#define MIRROR_RETURN_CODE_BASE         					(0xA0000)
#define MIRROR_RETURN_CODE_SUCCESS 							COMMON_SUCCESS
#define MIRROR_RETURN_CODE_ERROR 							COMMON_ERROR
#define MIRROR_RETURN_CODE_BAD_PARAM                    	(MIRROR_RETURN_CODE_BASE + 2)
#define MIRROR_RETURN_CODE_MALLOC_FAIL                  	(MIRROR_RETURN_CODE_BASE + 3)
#define MIRROR_RETURN_CODE_ACTION_NOT_SUPPORT           	(MIRROR_RETURN_CODE_BASE + 4)
#define MIRROR_RETURN_CODE_SRC_PORT_EXIST			    	(MIRROR_RETURN_CODE_BASE + 5)
#define MIRROR_RETURN_CODE_SRC_PORT_NOTEXIST 		    	(MIRROR_RETURN_CODE_BASE + 6)
#define MIRROR_RETURN_CODE_PROFILE_ID_OUTOFRANGE			(MIRROR_RETURN_CODE_BASE + 7)
#define MIRROR_RETURN_CODE_SRC_VLAN_EXIST			    	(MIRROR_RETURN_CODE_BASE + 8)
#define MIRROR_RETURN_CODE_SRC_VLAN_NOTEXIST		    	(MIRROR_RETURN_CODE_BASE + 9)
#define MIRROR_RETURN_CODE_VLAN_NOT_EXIST 			    	(MIRROR_RETURN_CODE_BASE + 10)
#define MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST    	(MIRROR_RETURN_CODE_BASE + 11)
#define MIRROR_RETURN_CODE_DESTINATION_NODE_EXIST 	    	(MIRROR_RETURN_CODE_BASE + 12)
#define MIRROR_RETURN_CODE_DESTINATION_NODE_CREATE_FAIL 	(MIRROR_RETURN_CODE_BASE + 13)
#define MIRROR_RETURN_CODE_SRC_FDB_NOTEXIST   		    	(MIRROR_RETURN_CODE_BASE+14)
#define MIRROR_RETURN_CODE_SRC_FDB_EXIST  			    	(MIRROR_RETURN_CODE_BASE+15)
#define MIRROR_RETURN_CODE_DONT_SUPPORT   			    	(MIRROR_RETURN_CODE_BASE+16)
#define MIRROR_RETURN_CODE_ACL_GLOBAL_NOT_EXISTED       	(MIRROR_RETURN_CODE_BASE+17)
#define MIRROR_RETURN_CODE_SRC_ACL_EXIST   			    	(MIRROR_RETURN_CODE_BASE+18)
#define MIRROR_RETURN_CODE_SRC_ACL_NOTEXIST 		    	(MIRROR_RETURN_CODE_BASE+19)
#define MIRROR_RETURN_CODE_DESTINATION_NODE_MEMBER_EXIST 	(MIRROR_RETURN_CODE_BASE+20)
#define MIRROR_RETURN_CODE_FDB_MAC_BE_SYSMAC             	(MIRROR_RETURN_CODE_BASE+21)
#define MIRROR_RETURN_CODE_SRC_PORT_CONFLICT            	(MIRROR_RETURN_CODE_BASE + 22)
#define MIRROR_RETURN_CODE_DESTINATION_NODE_MEMBER_NOEXIST  (MIRROR_RETURN_CODE_BASE+23)
#define MIRROR_RETURN_CODE_PROFILE_NODE_CREATED				(MIRROR_RETURN_CODE_BASE + 24)
#define MIRROR_RETURN_CODE_PROFILE_NOT_CREATED				(MIRROR_RETURN_CODE_BASE + 25)
#define MIRROR_RETURN_CODE_EXT_RULE_OLNY					(MIRROR_RETURN_CODE_BASE + 26)
#define MIRROR_RETURN_CODE_NOT_SUPPORT_PROFILE 				(MIRROR_RETURN_CODE_BASE + 27)
#define MIRROR_RETURN_CODE_FDB_NOT_SUPPORT    				(MIRROR_RETURN_CODE_BASE + 28)
#define MIRROR_RETURN_CODE_INPUT_DIRECT_ERROR    			(MIRROR_RETURN_CODE_BASE + 29)

/* PACKET RX/TX branch */                                    
#define	PACKET_RETURN_CODE_BASE			        (0xB0000)							/* return code base  */  
#define	PACKET_RETURN_CODE_OK			        (PACKET_RETURN_CODE_BASE + 0x1)		/* success   */  
#define	PACKET_RETURN_CODE_FAIL	            	(PACKET_RETURN_CODE_BASE + 0x2)   	/* fail      */
#define	PACKET_RETURN_CODE_ERROR		        (PACKET_RETURN_CODE_BASE + 0x3)		/* error     */ 
#define	PACKET_RETURN_CODE_ALLOC_MEM_NULL	    (PACKET_RETURN_CODE_BASE + 0x4)		/* alloc memory null   */ 
#define	PACKET_RETURN_CODE_BAD_SIZE		        (PACKET_RETURN_CODE_BASE + 0x5)		/* packet illegal size error */   

/* PVLAN branch */                                     
#define	PVLAN_RETURN_CODE_BASE			       		 	(0xC0000)	                    	/* return code base  */  
#define	PVLAN_RETURN_CODE_SUCCESS			       		COMMON_SUCCESS    	/* success   */   
#define	PVLAN_RETURN_CODE_ERROR			        		COMMON_ERROR		/* error     */ 
#define PVLAN_RETURN_CODE_AVOID_CYCLE_UPLINK			(PVLAN_RETURN_CODE_BASE + 0x3)
#define	PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE			(PVLAN_RETURN_CODE_BASE + 0x4)		/* port have been PVE port */ 
#define	PVLAN_RETURN_CODE_UPLINK_PORT_NOT_IN_SAME_VLAN	(PVLAN_RETURN_CODE_BASE + 0x5)	/* uplink port not be in same vlan */   
#define	PVLAN_RETURN_CODE_PORT_NOT_IN_ONLY_ONE_VLAN		(PVLAN_RETURN_CODE_BASE + 0x6)	/* pvlan port not be in one vlan  */ 
#define	PVLAN_RETURN_CODE_NO_SUCH_PORT	    			(PVLAN_RETURN_CODE_BASE + 0x7)		/* no such ports */   
#define	PVLAN_RETURN_CODE_NOT_SUPPORT		    		(PVLAN_RETURN_CODE_BASE + 0x8)		/* not support */  
#define	PVLAN_RETURN_CODE_NO_PVE		    		(PVLAN_RETURN_CODE_BASE + 0x9)		/* not support */  

/* QOS branch */
#define QOS_RETURN_CODE_BASE			(0xD0000)
#define QOS_RETURN_CODE_SUCCESS 					COMMON_SUCCESS
#define QOS_RETURN_CODE_ERROR 						COMMON_ERROR
#define QOS_RETURN_CODE_BAD_PARAM               (QOS_RETURN_CODE_BASE+3)
#define QOS_RETURN_CODE_PROFILE_EXISTED			(QOS_RETURN_CODE_BASE + 4)
#define QOS_RETURN_CODE_PROFILE_NOT_EXISTED 	(QOS_RETURN_CODE_BASE + 5)
#define QOS_RETURN_CODE_POLICY_EXISTED			(QOS_RETURN_CODE_BASE + 6)
#define QOS_RETURN_CODE_POLICY_NOT_EXISTED 		(QOS_RETURN_CODE_BASE + 7)
#define QOS_RETURN_CODE_POLICY_MAP_BIND		    (QOS_RETURN_CODE_BASE + 8)
#define QOS_RETURN_CODE_POLICER_NOT_EXISTED 	(QOS_RETURN_CODE_BASE + 9)
#define QOS_RETURN_CODE_COUNTER_NOT_EXISTED 	(QOS_RETURN_CODE_BASE + 10)
#define QOS_RETURN_CODE_POLICY_MAP_PORT_WRONG   (QOS_RETURN_CODE_BASE + 11)
#define QOS_RETURN_CODE_POLICER_USE_IN_ACL      (QOS_RETURN_CODE_BASE + 12)
#define QOS_RETURN_CODE_TRAFFIC_NO_INFO         (QOS_RETURN_CODE_BASE + 13)
#define QOS_RETURN_CODE_PROFILE_IN_USE          (QOS_RETURN_CODE_BASE + 14)
#define QOS_RETURN_CODE_POLICER_CBS_BIG         (QOS_RETURN_CODE_BASE + 15)
#define QOS_RETURN_CODE_POLICER_CBS_LITTLE      (QOS_RETURN_CODE_BASE + 16)
#define QOS_RETURN_CODE_NO_MAPPED        		(QOS_RETURN_CODE_BASE + 17)
#define QOS_RETURN_CODE_BAD_PTR					(QOS_RETURN_CODE_BASE + 100)


/* ROUTE branch */
#define ROUTE_RETURN_CODE_BASE             	(0xE0000)                       	/*return code base used in route modle*/
#define ROUTE_RETURN_CODE_SUCCESS          	(COMMON_SUCCESS)    	/*return success*/
#define ROUTE_RETURN_CODE_BAD_VALUE        	(ROUTE_RETURN_CODE_BASE + 0x2)    	/* Illegal value */
#define ROUTE_RETURN_CODE_OUT_OF_RANGE     	(ROUTE_RETURN_CODE_BASE + 0x3)    	/* Value is out of range*/
#define ROUTE_RETURN_CODE_BAD_PARAM        	(ROUTE_RETURN_CODE_BASE + 0x4)    	/* Illegal parameter in function called  */
#define ROUTE_RETURN_CODE_BAD_STATE        	(ROUTE_RETURN_CODE_BASE + 0x5)    	/* Illegal state of state machine        */
#define ROUTE_RETURN_CODE_SET_ERROR        	(ROUTE_RETURN_CODE_BASE + 0x6)    	/* Set operation failed                  */
#define ROUTE_RETURN_CODE_GET_ERROR        	(ROUTE_RETURN_CODE_BASE + 0x7)    	/* Get operation failed                  */
#define ROUTE_RETURN_CODE_CREATE_ERROR     	(ROUTE_RETURN_CODE_BASE + 0x8)    	/* Fail while creating an item           */
#define ROUTE_RETURN_CODE_NOT_FOUND        	(ROUTE_RETURN_CODE_BASE + 0x9)    	/* Item not found                        */
#define ROUTE_RETURN_CODE_NO_MORE          	(ROUTE_RETURN_CODE_BASE + 0xA)  	/* No more items found                   */
#define ROUTE_RETURN_CODE_NO_SUCH          	(ROUTE_RETURN_CODE_BASE + 0xB)  	/* No such item                          */
#define ROUTE_RETURN_CODE_NO_CHANGE        	(ROUTE_RETURN_CODE_BASE + 0xC)  	/* The parameter is already in this value*/
#define ROUTE_RETURN_CODE_NOT_SUPPORTED   	(ROUTE_RETURN_CODE_BASE + 0xD)  	/* This request is not support           */
#define ROUTE_RETURN_CODE_NOT_IMPLEMENTED  	(ROUTE_RETURN_CODE_BASE + 0xE)  	/* This request is not implemented       */
#define ROUTE_RETURN_CODE_NO_RESOURCE      	(ROUTE_RETURN_CODE_BASE + 0xF)  	/* Resource not available/accessble (memory ...)   */
#define ROUTE_RETURN_CODE_FULL            	(ROUTE_RETURN_CODE_BASE + 0x10)   	/* Item is full (Queue or table etc...)  */
#define ROUTE_RETURN_CODE_EMPTY            	(ROUTE_RETURN_CODE_BASE + 0x11)   	/* Item is empty (Queue or table etc...) */
#define ROUTE_RETURN_CODE_INIT_ERROR       	(ROUTE_RETURN_CODE_BASE + 0x12)  	/* Error occurred while INIT process      */
#define ROUTE_RETURN_CODE_ALREADY_EXIST    	(ROUTE_RETURN_CODE_BASE + 0x13)   	/* Tried to create existing item         */
#define ROUTE_RETURN_CODE_NULL_PTR          (ROUTE_RETURN_CODE_BASE + 0x14)     /* route null pointer*/
#define ROUTE_RETURN_CODE_HASH_OP_FAILED	(ROUTE_RETURN_CODE_BASE + 0x15)		/* route hash table push or pull failed */
#define ROUTE_RETURN_CODE_NOTCONSISTENT		(ROUTE_RETURN_CODE_BASE + 0x16)		/* route item is not consistent*/
#define ROUTE_RETURN_CODE_ERROR				(ROUTE_RETURN_CODE_BASE + 0x17)		/* route general error occured*/
#define ROUTE_RETURN_CODE_ACTION_TRAP2CPU	(ROUTE_RETURN_CODE_BASE + 0x18)		/* route action trap to cpu */
#define ROUTE_RETURN_CODE_ACTION_HARD_DROP	(ROUTE_RETURN_CODE_BASE + 0x19)		/* route action hard drop */
	

/* STP/RSTP/MSTP branch */
#define STP_RETURN_CODE_BASE                         (0xF0000)                      	 /*return code base used in stp modle*/
#define STP_RETURN_CODE_SUCCESS                      (STP_RETURN_CODE_BASE + 0x1)     /*return success*/
#define STP_RETURN_CODE_RSTP_NOT_ENABLED             (STP_RETURN_CODE_BASE + 0x2)     /*RSTP hasn't enabled*/
#define STP_RETURN_CODE_RSTP_HAVE_ENABLED            (STP_RETURN_CODE_BASE + 0x3)     /*RSTP is already enabled*/
#define STP_RETURN_CODE_MSTP_NOT_ENABLED             (STP_RETURN_CODE_BASE + 0x4)     /*MSTP hasn't enabled*/
#define STP_RETURN_CODE_MSTP_HAVE_ENABLED            (STP_RETURN_CODE_BASE + 0x5)     /*MSTP is already enabled*/
#define STP_RETURN_CODE_PORT_NOT_ENABLED             (STP_RETURN_CODE_BASE + 0x6)     /*This port hasn't enabled*/
#define STP_RETURN_CODE_PORT_HAVE_ENABLED            (STP_RETURN_CODE_BASE + 0x7)     /*This port is already enabled*/
#define STP_RETURN_CODE_Small_Bridge_Priority        (STP_RETURN_CODE_BASE + 0x8)     /*The bridge priority is small*/
#define STP_RETURN_CODE_Large_Bridge_Priority        (STP_RETURN_CODE_BASE + 0x9)     /*The bridge priority is large*/
#define STP_RETURN_CODE_Small_Max_Hops               (STP_RETURN_CODE_BASE + 0xA)     /*The max hops is small*/
#define STP_RETURN_CODE_Large_Max_Hops               (STP_RETURN_CODE_BASE + 0xB)     /*The max hops is large*/
#define STP_RETURN_CODE_Small_Hello_Time             (STP_RETURN_CODE_BASE + 0xC)     /*The hello time is small*/
#define STP_RETURN_CODE_Large_Hello_Time             (STP_RETURN_CODE_BASE + 0xD)   	 /*The hello time is large*/
#define STP_RETURN_CODE_Small_Max_Age                (STP_RETURN_CODE_BASE + 0xE)     /*The max age is small*/
#define STP_RETURN_CODE_Large_Max_Age                (STP_RETURN_CODE_BASE + 0xF)     /*The max age is large*/
#define STP_RETURN_CODE_Small_Forward_Delay          (STP_RETURN_CODE_BASE + 0x10)    /*The forward delay is small*/
#define STP_RETURN_CODE_Large_Forward_Delay          (STP_RETURN_CODE_BASE + 0x11)    /*The forward delay is large*/
#define STP_RETURN_CODE_Fwd_Delay_And_Max_Age_Are_Inconsistent 		(STP_RETURN_CODE_BASE + 0x12)    /*The forward delay and the max age should be content with: Max-Age<=2*(Forward-Delay-1)*/
#define STP_RETURN_CODE_Hello_Time_And_Max_Age_Are_Inconsistent         (STP_RETURN_CODE_BASE + 0x13)    /*The hello time and the max age should be content with:2*(hello-time+1)<=Max-Age*/
#define STP_RETURN_CODE_Hello_Time_And_Forward_Delay_Are_Inconsistent   (STP_RETURN_CODE_BASE + 0x14)    /*The hello time and the forward delay should be contend with: 2*(hello-time +1)<=2*(forward-delay - 1)*/ 

/* TRUNK(LAG) branch */
#define TRUNK_RETURN_CODE_BASE 			        (0x100000)
#define TRUNK_RETURN_CODE_ERR_NONE      	    (COMMON_SUCCESS)			   /*success */   
#define TRUNK_RETURN_CODE_ERR_GENERAL          	(TRUNK_RETURN_CODE_BASE + 0x1) /* general failure */
#define TRUNK_RETURN_CODE_BADPARAM      	    (TRUNK_RETURN_CODE_BASE + 0x2) /* bad parameters */
#define TRUNK_RETURN_CODE_TRUNK_EXISTS        	(TRUNK_RETURN_CODE_BASE + 0x3) /* trunk have been created already */
#define TRUNK_RETURN_CODE_TRUNK_NOTEXISTS      	(TRUNK_RETURN_CODE_BASE + 0x4) /* trunk does not exists */
#define TRUNK_RETURN_CODE_ERR_HW      		    (TRUNK_RETURN_CODE_BASE + 0x5) /* trunk error when operation on HW */
#define TRUNK_RETURN_CODE_PORT_EXISTS      	    (TRUNK_RETURN_CODE_BASE + 0x6) /* port already exists in trunk */
#define TRUNK_RETURN_CODE_PORT_NOTEXISTS       	(TRUNK_RETURN_CODE_BASE + 0x7) /* port is not a member of trunk */
#define TRUNK_RETURN_CODE_NAME_CONFLICT        	(TRUNK_RETURN_CODE_BASE + 0x8) /* vlan name conflict */
#define TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT   	(TRUNK_RETURN_CODE_BASE + 0x9) /* this port is already a member of other trunk */
#define TRUNK_RETURN_CODE_ALLOW_ERR        	    (TRUNK_RETURN_CODE_BASE + 0xA) /* Error occurs in trunk port add to allowed vlans */
#define TRUNK_RETURN_CODE_REFUSE_ERR           	(TRUNK_RETURN_CODE_BASE + 0xB) /* Error occurs in trunk port delete from refused vlans */ 
#define TRUNK_RETURN_CODE_MEMBER_ADD_ERR       	(TRUNK_RETURN_CODE_BASE + 0xC) /* Error on adding this port to default or allowed vlans */ 
#define TRUNK_RETURN_CODE_MEMBER_DEL_ERR       	(TRUNK_RETURN_CODE_BASE + 0xD) /* Error on deleting this port from allowed vlans */
#define TRUNK_RETURN_CODE_GET_ALLOWVLAN_ERR	    (TRUNK_RETURN_CODE_BASE + 0xE) /* operation on getting trunk allow vlanlist fails */
#define TRUNK_RETURN_CODE_NO_MEMBER        	    (TRUNK_RETURN_CODE_BASE + 0xF)  /* there exists no member in trunk */
#define TRUNK_RETURN_CODE_SET_TRUNKID_ERR	    (TRUNK_RETURN_CODE_BASE + 0x10)  /* Error on setting port trunkId */
#define TRUNK_RETURN_CODE_DEL_MASTER_PORT      	(TRUNK_RETURN_CODE_BASE + 0x11)  /* master port NOT allowd to delete */
#define TRUNK_RETURN_CODE_PORT_ENABLE		    (TRUNK_RETURN_CODE_BASE + 0x12)  /* port enalbe in trunk */
#define TRUNK_RETURN_CODE_PORT_NOTENABLE       	(TRUNK_RETURN_CODE_BASE + 0x13)  /*  port disable in trunk */
#define TRUNK_RETURN_CODE_ALLOW_VLAN		    (TRUNK_RETURN_CODE_BASE + 0x14)  /* vlan already allow in trunk */
#define TRUNK_RETURN_CODE_NOTALLOW_VLAN		    (TRUNK_RETURN_CODE_BASE + 0x15)  /* vlan not allow in trunk */
#define TRUNK_RETURN_CODE_LOAD_BANLC_CONFLIT   	(TRUNK_RETURN_CODE_BASE + 0x16)  /* trunk load balance mode same to original */
#define TRUNK_RETURN_CODE_VLAN_TAGMODE_ERR	    (TRUNK_RETURN_CODE_BASE + 0x17)  /* trunk %d tagMode error in vlan */
#define TRUNK_RETURN_CODE_PORT_LINK_DOWN       	(TRUNK_RETURN_CODE_BASE + 0x18)  /* The port must be link first */
#define TRUNK_RETURN_CODE_UNSUPPORT		        (TRUNK_RETURN_CODE_BASE + 0x19)  /* The device isn't supported this mode set */
#define TRUNK_RETURN_CODE_PORT_L3_INTFG		    (TRUNK_RETURN_CODE_BASE + 0x1A)  /* this port is L3 interface */
#define TRUNK_RETURN_CODE_PORT_MBRS_FULL        (TRUNK_RETURN_CODE_BASE + 0x1B)  /* trunk port member full */
#define TRUNK_RETURN_CODE_PORT_CONFIG_DIFFER    (TRUNK_RETURN_CODE_BASE + 0x1C)  /* port has a differ configuration with master port */
#define TRUNK_RETURN_CODE_DYNAMIC_TRUNK_DELETE_BYNAME	(TRUNK_RETURN_CODE_BASE + 0x1D)	/* delete dynamic trunk by trunk name */
#define TRUNK_RETURN_CODE_NAM_ERROR				(TRUNK_RETURN_CODE_BASE + 0x1E)		/* nam operation failed*/
#define TRUNK_RETURN_CODE_INTERFACE_NOT_EXIST   (TRUNK_RETURN_CODE_BASE + 0x1F)		/* the interface not exist when add port to dynamic trunk*/
#define TRUNK_RETURN_CODE_TRUNKID_OUT_OF_RANGE	(TRUNK_RETURN_CODE_BASE + 0x20)		/*trunk id is out of range*/
#define TRUNK_RETURN_CODE_INTERFACE_L3_ENABLE		(TRUNK_RETURN_CODE_BASE + 0x21)		/*the port is not a l3 disable interface*/
#define TRUNK_RETURN_CODE_NO_SUCH_PORT		(TRUNK_RETURN_CODE_BASE + 0x22)		/* the port is not exists*/
#define TRUNK_RETURN_CODE_SLOT_PORT_FAIL        (TRUNK_RETURN_CODE_BASE + 0x23)   /* slot or port number get failed*/

/* UTILITY branch */
#define UTILUS_RETURN_CODE_BASE        		    (0x110000)
#define UTILUS_RETURN_CODE_STP_OK               (UTILUS_RETURN_CODE_BASE)       /* stp operate success */
#define UTILUS_RETURN_CODE_BASE_STP_ERROR       (UTILUS_RETURN_CODE_BASE + 0x1) /* (VID < 0 || VID > STP_MAX_VID) */
#define UTILUS_RETURN_CODE_STP_CANNOT_CREATE_INSTANCE_FOR_VLAN  (UTILUS_RETURN_CODE_BASE + 0x2)    /* stp can't establish instance base vlan */ 
#define UTILUS_RETURN_CODE_STP_DISABLE          (UTILUS_RETURN_CODE_BASE + 0x3) /* express STP_DISABLE */
#define UTILUS_RETURN_CODE_STP_CANNOT_CREATE_INSTANCE_FOR_PORT   (UTILUS_RETURN_CODE_BASE + 0x4)    /*stp can't establish instance base port */ 
#define UTILUS_RETURN_CODE_ERR_NO_MATCH         (UTILUS_RETURN_CODE_BASE + 0x5) /* no match message */
#define UTILUS_RETURN_CODE_ERR_DEVICE_NUMBER    (UTILUS_RETURN_CODE_BASE + 0x6) /* express devNum >= dev_max */
#define UTILUS_RETURN_CODE_ERR_PORT_ON_DEVICE	(UTILUS_RETURN_CODE_BASE + 0x7) /* express virtPortNo > port_max */
#define UTILUS_RETURN_CODE_SLOT_SUCCESS         (UTILUS_RETURN_CODE_BASE + 0x8) /* express operate  success */
#define UTILUS_RETURN_CODE_ERR_SLOT_OUT_OF_RANGE    (UTILUS_RETURN_CODE_BASE + 0xA) /* express slot > slot max */
#define UTILUS_RETURN_CODE_ERR_PORT_OUT_OF_RANGE    (UTILUS_RETURN_CODE_BASE + 0xB) /* express port > port max */
#define UTILUS_RETURN_CODE_ERR_MODULE_NOT_SUPPORT   (UTILUS_RETURN_CODE_BASE + 0xC) /* express operate not support */
#define UTILUS_RETURN_CODE_DIAG_SUCCESS         (UTILUS_RETURN_CODE_BASE + 0xD)/* express operate  success */
#define UTILUS_RETURN_CODE_FAIL                 (UTILUS_RETURN_CODE_BASE + 0xE)/*  express operate fail */
#define UTILUS_RETURN_CODE_FALSE                (UTILUS_RETURN_CODE_BASE + 0x10)/* express operate false */
#define UTILUS_RETURN_CODE_SUCCESS              (UTILUS_RETURN_CODE_BASE + 0x11)/* express operate  success */
#define UTILUS_RETURN_CODE_LOG_SUCCESS          (UTILUS_RETURN_CODE_BASE + 0x12)/* express operate  success */
#define UTILUS_RETURN_CODE_TRUE                 (UTILUS_RETURN_CODE_BASE + 0x13)/* express operate true */
#define UTILUS_RETURN_CODE_NULL                 (UTILUS_RETURN_CODE_BASE + 0x14)/* show NULL */
#define UTILUS_RETURN_CODE_MAIN_SUCCESS         (UTILUS_RETURN_CODE_BASE + 0x15) /* express operate  success  */
#define UTILUS_RETURN_CODE_ILLEGALL             (UTILUS_RETURN_CODE_BASE + 0x16) /*  parameters illegall*/
#define UTILUS_RETURN_CODE_PARAMETER            (UTILUS_RETURN_CODE_BASE + 0x17) /* input parameter to long */
#define UTILUS_RETURN_CODE_NOT_EQUAL_SYSTEM_MAC (UTILUS_RETURN_CODE_BASE + 0x18) /* given mac address not  same as system base mac address*/
#define UTILUS_RETURN_CODE_EQUAL_SYSTEM_MAC     (UTILUS_RETURN_CODE_BASE + 0x19) /* the given mac address  same as system base mac address*/
#define UTILUS_RETURN_CODE_BUFFWE_SMALL         (UTILUS_RETURN_CODE_BASE + 0x1A) /* mac address buffer size too small*/
#define UTILUS_RETURN_CODE_ILLEGAL_CHARACTER    (UTILUS_RETURN_CODE_BASE + 0x1B) /* illegal character found*/
#define UTILUS_RETURN_CODE_ERROR_OCCUR          (UTILUS_RETURN_CODE_BASE + 0x1C) /* if no error occur */
#define UTILUS_RETURN_CODE_NPD_DBUS_SUCCESS     (UTILUS_RETURN_CODE_BASE + 0x1E) /*   sucess */
#define UTILUS_RETURN_CODE_NPD_DBUS_ERROR       (UTILUS_RETURN_CODE_BASE + 0x1F) /* general use, no detail error information */
#define UTILUS_RETURN_CODE_NAM_DIAG_SUCCESS     (UTILUS_RETURN_CODE_BASE + 0x20) /* general use, show operate successinformation */
#define UTILUS_RETURN_CODE_NAM_DIAG_FAIL        (UTILUS_RETURN_CODE_BASE + 0x21) /* show operate fail */
#define UTILUS_RETURN_CODE_NAM_DIAG_TRUE 	    (UTILUS_RETURN_CODE_BASE + 0x22) /* show operate true */
#define UTILUS_RETURN_CODE_NAM_DIAG_FALSE	    (UTILUS_RETURN_CODE_BASE + 0x23) /* show operate false */
#define UTILUS_RETURN_CODE_NAM_DIAG_NULL	    (UTILUS_RETURN_CODE_BASE + 0x24) /* show  NULL */
#define UTILUS_RETURN_CODE_DEBUG_NAM_SET_FIAL   (UTILUS_RETURN_CODE_BASE + 0x25) /* if debug level has already been set before*/
#define UTILUS_RETURN_CODE_DEBUG_NAM_SET_SUCCESS    (UTILUS_RETURN_CODE_BASE + 0x26) /* debug level setup successfully */
#define UTILUS_RETURN_CODE_DEBUG_NBM_SET_FIAL       (UTILUS_RETURN_CODE_BASE + 0x27) /* if debug level has already been set before */
#define UTILUS_RETURN_CODE_DEBUG_NBM_SET_SUCCESS    (UTILUS_RETURN_CODE_BASE + 0x28) /* debug level setup successfully */
#define UTILUS_RETURN_CODE_BASE_NPD_OK          (UTILUS_RETURN_CODE_BASE + 0x29) /* operate success */

/* VLAN branch */
#define VLAN_RETURN_CODE_BASE			    (0x120000)
#define VLAN_RETURN_CODE_ERR_NONE      		(COMMON_SUCCESS)		        /*success */   
#define VLAN_RETURN_CODE_ERR_GENERAL        (VLAN_RETURN_CODE_BASE + 0x1)   /* general failure */
#define VLAN_RETURN_CODE_BADPARAM      		(VLAN_RETURN_CODE_BASE + 0x2)   /* bad parameters */
#define VLAN_RETURN_CODE_VLAN_EXISTS       	(VLAN_RETURN_CODE_BASE + 0x3)   /* vlan have been created already */
#define VLAN_RETURN_CODE_NAME_CONFLICT      (VLAN_RETURN_CODE_BASE + 0x4)   /* vlan name conflict */
#define VLAN_RETURN_CODE_VLAN_NOT_EXISTS    (VLAN_RETURN_CODE_BASE + 0x5)   /* vlan does not exists */
#define VLAN_RETURN_CODE_ERR_HW      		(VLAN_RETURN_CODE_BASE + 0x6)   /* vlan error when operation on HW */
#define VLAN_RETURN_CODE_PORT_EXISTS       	(VLAN_RETURN_CODE_BASE + 0x7)   /* port already exists in vlan */
#define VLAN_RETURN_CODE_PORT_NOTEXISTS        	(VLAN_RETURN_CODE_BASE + 0x8)   /* port is not a member of vlan */
#define VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT 	(VLAN_RETURN_CODE_BASE + 0x9)   /* port can NOT be Untag member of different vlans */
#define VLAN_RETURN_CODE_L3_INTF        	    (VLAN_RETURN_CODE_BASE + 0xA) /* vlan is L3 interface */
#define VLAN_RETURN_CODE_PORT_TAG_CONFLICT     	(VLAN_RETURN_CODE_BASE + 0xB) /* port Tag-Mode not match */
#define VLAN_RETURN_CODE_TRUNK_MEMBER_NONE    	(VLAN_RETURN_CODE_BASE + 0xC) /* trunk has no member when add to vlan */
#define VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF  	(VLAN_RETURN_CODE_BASE + 0xD) /* the promiscuous mode port add to l3 interface */
#define VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF	(VLAN_RETURN_CODE_BASE + 0xE) /* del promiscuous port but default is l3 intf */
#define VLAN_RETURN_CODE_SUBINTF_EXISTS     	 (VLAN_RETURN_CODE_BASE + 0xF) /* sub intf exists */
#define VLAN_RETURN_CODE_CONTAINS_ROUTE_PORT	 (VLAN_RETURN_CODE_BASE + 0x10)  /* vlan contains route mod port */
#define VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN (VLAN_RETURN_CODE_BASE + 0x11)  /* promis try to add to other vlan(not vlan 1) */
#define VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS	 (VLAN_RETURN_CODE_BASE + 0x12)  /* port sub interface exists*/
#define VLAN_RETURN_CODE_PORT_L3_INTF        	 (VLAN_RETURN_CODE_BASE + 0x13) /* port is L3 interface */
#define VLAN_RETURN_CODE_TRUNK_EXISTS		     (VLAN_RETURN_CODE_BASE + 0x14) /* trunk already member of vlan*/
#define VLAN_RETURN_CODE_TRUNK_NOTEXISTS	     (VLAN_RETURN_CODE_BASE + 0x15) /* trunk is not member of vlan*/
#define VLAN_RETURN_CODE_TRUNK_CONFLICT		     (VLAN_RETURN_CODE_BASE + 0x16) /* trunk already untagged member of other active vlan */
#define VLAN_RETURN_CODE_PORT_TRUNK_MBR		     (VLAN_RETURN_CODE_BASE + 0x17) /* port belong to trunk ,it can NOT add to vlan as port */
#define VLAN_RETURN_CODE_TRUNK_MBRSHIP_CONFLICT	 (VLAN_RETURN_CODE_BASE + 0x18) /* trunk membership conflict */
#define VLAN_RETURN_CODE_NOT_SUPPORT_IGMP_SNP 	 (VLAN_RETURN_CODE_BASE + 0x19) 
#define VLAN_RETURN_CODE_IGMP_ROUTE_PORTEXIST	 (VLAN_RETURN_CODE_BASE + 0x1A) 
#define VLAN_RETURN_CODE_IGMP_ROUTE_PORTNOTEXIST (VLAN_RETURN_CODE_BASE + 0x1B) 
#define VLAN_RETURN_CODE_ARP_STATIC_CONFLICT	 (VLAN_RETURN_CODE_BASE + 0x1C) 
#define VLAN_RETURN_CODE_ERR_MAX	             (VLAN_RETURN_CODE_BASE + 0x1D) 
#define VLAN_RETURN_CODE_PROMIS_PORT_CANNOT_DEL	 (VLAN_RETURN_CODE_BASE + 0x1E) /* del a promis port from advanced-routing-default-vid is not allowed */
#define VLAN_RETURN_CODE_ADVANCED_VLAN_CANNOT_DEL (VLAN_RETURN_CODE_BASE + 0x1F) /* can't delete vlan when it is advanced-routing default-vid */
#define VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED		 (VLAN_RETURN_CODE_BASE + 0x21) 	/*advanced-routing default vlan is not allowed to config*/
#define VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG   (VLAN_RETURN_CODE_BASE + 0x22)   /* protocol-base vlan config checked in this vlan when vlan delete */
#define VLAN_RETURN_CODE_VLAN_CREATE_NOT_ALLOWED (VLAN_RETURN_CODE_BASE + 0x23)   /* 5612E is not allowed to create vlan */
#define VLAN_RETURN_CODE_VLAN_BOND_ERR   (VLAN_RETURN_CODE_BASE + 0x24)   /* vlan bond err */
#define VLAN_RETURN_CODE_VLAN_UNBOND_ERR   (VLAN_RETURN_CODE_BASE + 0x25)   /* vlan unbond err */
#define VLAN_RETURN_CODE_VLAN_ALREADY_BOND (VLAN_RETURN_CODE_BASE + 0x26)   /* vlan is already bond */
#define VLAN_RETURN_CODE_VLAN_NOT_BONDED   (VLAN_RETURN_CODE_BASE + 0x27)   /* vlan is not bonded */
#define VLAN_RETURN_CODE_VLAN_SYNC_ERR   (VLAN_RETURN_CODE_BASE + 0x28)   /* vlan is not bonded */


/* TUNNEL branch */
#define TUNNEL_RETURN_CODE_BASE		        	(0x130000)
#define	TUNNEL_RETURN_CODE_SUCCESS			       		COMMON_SUCCESS    	/* success   */   
#define	TUNNEL_RETURN_CODE_ERROR			        		COMMON_ERROR		/* error     */ 
#define TUNNEL_RETURN_CODE_NOTEXISTS_1		(TUNNEL_RETURN_CODE_BASE + 1)
#define TUNNEL_RETURN_CODE_NULLPOINTER_2		(TUNNEL_RETURN_CODE_BASE + 2)
#define TUNNEL_RETURN_CODE_TABLE_FULL_3		(TUNNEL_RETURN_CODE_BASE + 3)
#define TUNNEL_RETURN_CODE_DUPLICATED_4		(TUNNEL_RETURN_CODE_BASE + 4)
#define TUNNEL_RETURN_CODE_EXISTS_5			(TUNNEL_RETURN_CODE_BASE + 5)
#define TUNNEL_RETURN_CODE_TSFULL_6			(TUNNEL_RETURN_CODE_BASE + 6)
#define TUNNEL_RETURN_CODE_TTFULL_7			(TUNNEL_RETURN_CODE_BASE + 7)
#define TUNNEL_RETURN_CODE_NHFULL_8			(TUNNEL_RETURN_CODE_BASE + 8)
#define TUNNEL_RETURN_CODE_RT_HOST_EXISTS_9			(TUNNEL_RETURN_CODE_BASE + 9)
#define TUNNEL_RETURN_CODE_DSTIP_NOT_EXISTS_10			(TUNNEL_RETURN_CODE_BASE + 10)

/* DLDP branch */
#define	DLDP_RETURN_CODE_BASE	              	(0x140000)		       /* return code base  */  
#define	DLDP_RETURN_CODE_OK	       	            (DLDP_RETURN_CODE_BASE + 0x0)  /* success   */ 
#define	DLDP_RETURN_CODE_ERROR	             	(DLDP_RETURN_CODE_BASE + 0x1)  /* error */ 
#define	DLDP_RETURN_CODE_ALREADY_SET	        (DLDP_RETURN_CODE_BASE + 0x2)  /* already been seted */   
#define	DLDP_RETURN_CODE_ENABLE_GBL	            (DLDP_RETURN_CODE_BASE + 0x3)  /* DLDP enabled global */   
#define	DLDP_RETURN_CODE_NOT_ENABLE_GBL	        (DLDP_RETURN_CODE_BASE + 0x4)  /* DLDP not enabled global  */ 
#define	DLDP_RETURN_CODE_OUT_RANGE	            (DLDP_RETURN_CODE_BASE + 0x5)  /* timer value out of range  */
#define	DLDP_RETURN_CODE_SAME_VALUE	            (DLDP_RETURN_CODE_BASE + 0x6)  /* set same value to DLDP timers*/ 
#define	DLDP_RETURN_CODE_GET_DETECT_E	        (DLDP_RETURN_CODE_BASE + 0x7)  /* get DLDP detect timers error */ 
#define	DLDP_RETURN_CODE_GET_REDETECT_E	        (DLDP_RETURN_CODE_BASE + 0x8)  /* get DLDP re-detect timers error*/  
#define	DLDP_RETURN_CODE_VLAN_NOT_EXIST	        (DLDP_RETURN_CODE_BASE + 0x9)  /* L2 vlan not exixt   */
#define	DLDP_RETURN_CODE_NOTENABLE_VLAN	        (DLDP_RETURN_CODE_BASE + 0xA)  /* L2 vlan not enable DLDP  */
#define	DLDP_RETURN_CODE_HASENABLE_VLAN	        (DLDP_RETURN_CODE_BASE + 0xB)  /* L2 vlan has enable DLDP  */
#define	DLDP_RETURN_CODE_NULL_PTR	            (DLDP_RETURN_CODE_BASE + 0xC)  /* parameter pointer is null  */ 
#define	DLDP_RETURN_CODE_HASH_TABLE_FULL	    (DLDP_RETURN_CODE_BASE + 0xD)  /* hash table has full   */
#define	DLDP_RETURN_CODE_HASH_DUPLICATED	    (DLDP_RETURN_CODE_BASE + 0xE)  /* hash table has duplicated */  
#define	DLDP_RETURN_CODE_HASH_NORESOURCE	    (DLDP_RETURN_CODE_BASE + 0xF)  /* hash item alloc memory null */ 
#define	DLDP_RETURN_CODE_HASH_NOTEXISTS	        (DLDP_RETURN_CODE_BASE + 0x10)  /* hash item not exist   */
#define	DLDP_RETURN_CODE_ALLOC_MEM_NULL	        (DLDP_RETURN_CODE_BASE + 0x11)  /* alloc memory null   */ 
#define	DLDP_RETURN_CODE_HASH_FOUND	            (DLDP_RETURN_CODE_BASE + 0x12)  /* found hash item   */ 
#define	DLDP_RETURN_CODE_HASH_NOTFOUND	        (DLDP_RETURN_CODE_BASE + 0x13)  /* not found hash iteml   */
#define	DLDP_RETURN_CODE_FAIL             	    (DLDP_RETURN_CODE_BASE + 0x14)  /* FAIL */

/* VRRP branch */
#define VRRP_RETURN_CODE_BASE					(0x150000)

/* DHCP SNOOPING branch */
#define DHCP_SNP_RETURN_CODE_BASE			(0x160000)						/* return code base 				*/
#define DHCP_SNP_RETURN_CODE_OK				(DHCP_SNP_RETURN_CODE_BASE)		/* success 						*/
#define DHCP_SNP_RETURN_CODE_ERROR			(DHCP_SNP_RETURN_CODE_BASE + 1)	/* error 							*/
#define DHCP_SNP_RETURN_CODE_ALREADY_SET	(DHCP_SNP_RETURN_CODE_BASE + 2)	/* already been seted				*/
#define DHCP_SNP_RETURN_CODE_ENABLE_GBL		(DHCP_SNP_RETURN_CODE_BASE + 3)	/* DHCP-Snooping enabled global	*/
#define DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	(DHCP_SNP_RETURN_CODE_BASE + 4)	/* DHCP-Snooping not enabled global	*/
#define DHCP_SNP_RETURN_CODE_OUT_RANGE		(DHCP_SNP_RETURN_CODE_BASE + 5)	/* out of range					*/
#define DHCP_SNP_RETURN_CODE_NO_SUCH_PORT	(DHCP_SNP_RETURN_CODE_BASE + 6)	/* slotno or portno is not legal		*/
#define DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL	(DHCP_SNP_RETURN_CODE_BASE + 7)	/* alloc memory null				*/
#define DHCP_SNP_RETURN_CODE_FOUND			(DHCP_SNP_RETURN_CODE_BASE + 8)	/* found						*/
#define DHCP_SNP_RETURN_CODE_NOT_FOUND		(DHCP_SNP_RETURN_CODE_BASE + 9)	/* not found						*/
#define DHCP_SNP_RETURN_CODE_PKT_DROP		(DHCP_SNP_RETURN_CODE_BASE + 10)/* DHCP packet is not legal, drop it	*/
#define DHCP_SNP_RETURN_CODE_PARAM_NULL		(DHCP_SNP_RETURN_CODE_BASE + 11)/* error,parameter is null			*/
#define DHCP_SNP_RETURN_CODE_OPEN_DB_FAIL	(DHCP_SNP_RETURN_CODE_BASE + 12)/* open dhcp snooping db fail		*/
#define DHCP_SNP_RETURN_CODE_EN_OPT82		(DHCP_SNP_RETURN_CODE_BASE + 13)/* DHCP-Snooping enabled option82	*/
#define DHCP_SNP_RETURN_CODE_NOT_EN_OPT82	(DHCP_SNP_RETURN_CODE_BASE + 14)/* DHCP-Snooping not enabled option82	*/
#define DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	(DHCP_SNP_RETURN_CODE_BASE + 15)/* vlan not exist					*/
#define DHCP_SNP_RETURN_CODE_EN_VLAN		(DHCP_SNP_RETURN_CODE_BASE + 16)/* DHCP-Snooping enabled vlan		*/
#define DHCP_SNP_RETURN_CODE_NOT_EN_VLAN	(DHCP_SNP_RETURN_CODE_BASE + 17)/* DHCP-Snooping not enabled vlan	*/
#define DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR	(DHCP_SNP_RETURN_CODE_BASE + 18)/* the port is member of trunk		*/
#define DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST (DHCP_SNP_RETURN_CODE_BASE + 19)/* the port is not member of the vlan	*/
#define DHCP_SNP_RETURN_CODE_EN_INTF		(DHCP_SNP_RETURN_CODE_BASE + 20)/* dhcp snooping is enabled on interface */
#define DHCP_SNP_RETURN_CODE_NOT_EN_INTF	(DHCP_SNP_RETURN_CODE_BASE + 21)/* dhcp snooping is not enabled on interface */
#define DHCP_SNP_RETURN_CODE_NO_SUCH_INTF	(DHCP_SNP_RETURN_CODE_BASE + 22)/* interface is not exist */
#define DHCP_SNP_RETURN_CODE_EN_ANTI_ARP	(DHCP_SNP_RETURN_CODE_BASE + 23)/* anti-arp-spoof is enabled on interface */
#define DHCP_SNP_RETURN_CODE_EN_ADD_ROUTE	(DHCP_SNP_RETURN_CODE_BASE + 24)/* add-route is enabled on interface */


/* DHCP SERVER branch */
#define DHCP_SERVER_RETURN_CODE_BASE                    (0x170000)
#define DHCP_SERVER_RETURN_CODE_SUCCESS                 (COMMON_SUCCESS)
#define DHCP_SERVER_RETURN_CODE_FAIL                    (DHCP_SERVER_RETURN_CODE_BASE + 0x1)
#define DHCP_NOT_FOUND_POOL                                     (DHCP_SERVER_RETURN_CODE_BASE + 0x2)
#define DHCP_SET_ORDER_WANNING                          (DHCP_SERVER_RETURN_CODE_BASE + 0x3)
#define DHCP_UNBIND_BY_INTERFACE                                (DHCP_SERVER_RETURN_CODE_BASE + 0x4)
#define DHCP_HAVE_BIND_INTERFACE                                (DHCP_SERVER_RETURN_CODE_BASE + 0x5)
#define DHCP_NOT_FOUND_ADRESS_RANGE                     (DHCP_SERVER_RETURN_CODE_BASE + 0x6)
#define DHCP_POOL_SUBNET_NULL                                   (DHCP_SERVER_RETURN_CODE_BASE + 0x7)
#define DHCP_NOT_THE_SUBNET                                     (DHCP_SERVER_RETURN_CODE_BASE + 0x8)
#define DHCP_INCORRET_IP_ADDRESS                                (DHCP_SERVER_RETURN_CODE_BASE + 0x9)
#define DHCP_TOO_MANY_HOST_NUM                          (DHCP_SERVER_RETURN_CODE_BASE + 0x10)
#define DHCP_HAVE_CONTAIN_STATIC_USER           (DHCP_SERVER_RETURN_CODE_BASE + 0x11)
#define DHCP_NOT_FOUND_STATIC_USER                      (DHCP_SERVER_RETURN_CODE_BASE + 0x12)
#define DHCP_FAILOVER_HAVE_EXIST                                (DHCP_SERVER_RETURN_CODE_BASE + 0x13)
#define DHCP_SAME_PRIMARY                                               (DHCP_SERVER_RETURN_CODE_BASE + 0x14)
#define DHCP_FAILOVER_NOT_ENABLE                                (DHCP_SERVER_RETURN_CODE_BASE + 0x15)
#define DHCP_POOL_HAVE_NOT_FAILOVER                     (DHCP_SERVER_RETURN_CODE_BASE + 0x16)
#define DHCP_INTERFACE_IN_USR                                   (DHCP_SERVER_RETURN_CODE_BASE + 0x17)
#define DHCP_MAX_OPTION_LIST_WANNING            (DHCP_SERVER_RETURN_CODE_BASE + 0x18)
#define DHCP_REPEATE_SET_OPTION_WANNING         (DHCP_SERVER_RETURN_CODE_BASE + 0x19)
#define DHCP_ALREADY_ADD_OPTION                         (DHCP_SERVER_RETURN_CODE_BASE + 0x20)
#define DHCP_SUBNET_EXIST                        		(DHCP_SERVER_RETURN_CODE_BASE + 0x21)
#define DHCP_INTERFACE_WITHOUT_IPADDR              		(DHCP_SERVER_RETURN_CODE_BASE + 0x22)
#define DHCP_FAILOVER_NAME_WRONG						(DHCP_SERVER_RETURN_CODE_BASE + 0X23)


/* MLD SNOOPING branch */
#define	MLD_RETURN_CODE_BASE	              	(0x180000)		       /* return code base  */  
#define	MLD_RETURN_CODE_OK	       	            (MLD_RETURN_CODE_BASE + 0x0)  /* success   */ 
#define	MLD_RETURN_CODE_ERROR	             	(MLD_RETURN_CODE_BASE + 0x1)  /* error */ 
#define MLD_RETURN_CODE_EQUAL					(MLD_RETURN_CODE_BASE + 0x2)		/**/
#define MLD_RETURN_CODE_NOT_EQUAL				(MLD_RETURN_CODE_BASE + 0x3)	/**/


/* To be continue */
#define MAC_RETURN_CODE							(0xFFFFFFFF)

/*PROTOCOL VLAN branch*/
#define PROTOCOL_VLAN_RETURN_CODE_SUCCESS       (COMMON_SUCCESS)
#define PROTOCOL_VLAN_RETURN_CODE_BASE          (0x190000)
#define PROTOCOL_VLAN_RETURN_CODE_NAM_ERR       (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x1) /* nam operation failed,maybe asic set failed*/
#define PROTOCOL_VLAN_RETURN_CODE_ERROR         (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x2) /* general error */
#define PROTOCOL_VLAN_RETURN_CODE_VID_NOT_SET   (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x3)  /* not set vid when delete */
#define PROTOCOL_VLAN_RETURN_CODE_VID_NOT_MATCH    (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x4)  /* the vlan not match when try to no prot vlan vid*/
#define PROTOCOL_VLAN_RETURN_CODE_ETH_TYPE_RANG_ERR      (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x5)  /* the eth type is < 1536 and not 0*/
#define PROTOCOL_VLAN_RETURN_CODE_ETH_TYPE_EXISTS    (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x6)  /* udf eth type config to ipv4/ipv6/pppoe_d/pppoe_s,etc. */
#define PROTOCOL_VLAN_RETURN_CODE_UNSUPPORT     (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x7)       /* unsupport command */
#define PROTOCOL_VLAN_RETURN_CODE_ALREADY_TAG_MEMBER  (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x8)  /* already tag member of the vlan */
#define PROTOCOL_VLAN_RETURN_CODE_VLAN_CREATE_FAILED  (PROTOCOL_VLAN_RETURN_CODE_BASE + 0x9)  /* create vlan for protocol vlan failed */
#define PROTOCOL_VLAN_RETURN_CODE_ADD_PORT_TO_VLAN_FAILED  (PROTOCOL_VLAN_RETURN_CODE_BASE + 0xA)  /* add port to vlan for protocol vlan failed */


/*PPPoE snooping branch*/
#define PPPoE_SNP_RETURN_CODE_SUCCESS   	    (COMMON_SUCCESS)
#define PPPoE_SNP_RETURN_CODE_BASE          	(0x1a0000)
#define PPPoE_SNP_RETURN_CODE_FAIL				(PPPoE_SNP_RETURN_CODE_BASE + 0x1)
#define PPPoE_SNP_RETURN_CODE_NO_SUCH_INTERFACE (PPPoE_SNP_RETURN_CODE_BASE + 0x2)
#define PPPoE_SNP_RETURN_CODE_INVALID_MRU		(PPPoE_SNP_RETURN_CODE_BASE + 0x3)


/* Cvm-ratelimit :*/
#define CVM_RATELIMIT_RETURN_CODE_SUCCESS COMMON_SUCCESS
#define CVM_RATELIMIT_RETURN_CODE_BASE			0x1b0000
#define CVM_RATELIMIT_RETURN_CODE_FAILED  		(CVM_RATELIMIT_RETURN_CODE_BASE + 0x1)
#define CVM_RATELIMIT_RETURN_CODE_RULE_NOTEXIST (CVM_RATELIMIT_RETURN_CODE_BASE + 0x2)
#define CVM_RATELIMIT_RETURN_CODE_RULE_EXIST 	(CVM_RATELIMIT_RETURN_CODE_BASE + 0x3)
#define CVM_RATELIMIT_RETURN_CODE_RULE_FULL 	(CVM_RATELIMIT_RETURN_CODE_BASE + 0x4)
#define CVM_RATELIMIT_RETURN_CODE_UNSUPPORT 	(CVM_RATELIMIT_RETURN_CODE_BASE + 0x5)
#define CVM_RATELIMIT_RETURN_CODE_INVALID_RULE	(CVM_RATELIMIT_RETURN_CODE_BASE + 0x6)
#define CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING (CVM_RATELIMIT_RETURN_CODE_BASE + 0x7)
#define CVM_RATELIMIT_RETURN_CODE_SERVICE_ALREADY_LOAD (CVM_RATELIMIT_RETURN_CODE_BASE + 0x8)

#endif
