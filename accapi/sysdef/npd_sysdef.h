#ifndef __NPD_SYSDEF_H__
#define __NPD_SYSDEF_H__

#include <config/auteware_config.h>

#define  ASIC_DMA_VIRT_ADDR   0x50000000
#define  ASIC_CFG_VIRT_ADDR   0x60000000
#define  ASIC_REG_VIRT_ADDR   0x30000000 /* 0x70000000->0x30000000 */


/*
NOTICE
	Definition in this file could be transfered over dbus,
	it should be usable by other software application or module.

        These values might be transportted over dbus connection
	1. enum
	2. bit definition
*/

/*
  Product stackable type definition
*/
enum product_stackable_type_e {
	PRODUCT_STK_BOX_E = 0xB, /* box product means single slot */	
	PRODUCT_STK_CHASSIS_E = 0xC, /* chassis product means multi-slot exists */
	PRODUCT_STK_MAX
};

/*
  Product asic cluster definition
*/
enum product_asic_cluster_e {
	PRODUCT_ASIC_MV_E = 0xA, /* product use marvell asic device */
	PRODUCT_ASIC_BCM_E = 0xB, /* product use broadcom asic device */
	PRODUCT_CHIP_CAVIUM_E = 0xC, /* product use cavium chip suit */
	PRODUCT_ASIC_MAX
};

/*
  Product used Marvell asic devices
*/
enum asic_mv_device_e {
	ASIC_MV_CHEETAH2_275_E = 0, /* product use marvell cheetah2 device MV98Dx275 */
	ASIC_MV_CHEETAH2_265_E = 1, /* product use marvell cheetah2 device MV98Dx265 */
	ASIC_MV_CHEETAH2_255_E = 2, /* product use marvell cheetah2 device MV98Dx255 */
	ASIC_MV_CHEETAH2_265L_E = 3, /* product use marvell cheetah2 device MV98Dx265-lite(only use 12GE+2XG) */
	ASIC_MV_CHEETAH2_265E_E = 4, /* product use marvell cheetah2 device MV98Dx265-enforced(only use 8GE+2XG) */
	ASIC_MV_XCAT_BOBCAT_E = 5, /* product use marvell soc BobCat */
	ASIC_MV_XCAT_TOMCAT_E = 6, /*product use marvell soc TomCat */
	ASIC_MV_MAX
};

/*
  Product used Broadcom asic devices
*/
enum asic_bcm_device_e {
	ASIC_BCM_RAVEN_56024B0_E = 0, /* product use broadcom raven device BCM56024_B0 */
	ASIC_BCM_MV_MAX
};

/*
  Product used Cavium CPU suit chip
*/
enum cpu_cavium_device_e {
	CPU_CVM_CN5230_E = 0, /* product use cavium cpu OCTEON CN5230 */
	CPU_CVM_CN5220_E = 1, /* product use cavium cpu OCTEON CN5220 */
	CPU_CVM_MAX
};

/*
  Product id generator - 32-bit product id is composed as follows:
            +--------+---------+---------+---------+
            | OCTET0 |  OCTET1 |  OCTET2 |  OCTET3 |
            +--------+---------+---------+---------+
            |  RSVD  |4(S)+4(C)|4(A)+4(D)|  RSVD   |
            +--------+---------+---------+---------+
            where digital and abbreviation means
            	digital - field bit length
            	S - stackable type value
            	C - chassisc prodcut slot number, 1 for box product
            	A - product asic cluster type
            	D - asic device type in one asic cluster
*/
#define PRODUCT_ID_GENERATE(stack, slot, asic, device)	\
	(((stack & 0xF) << 20) | ((slot & 0xF) << 16) | ((asic & 0xF) << 12) | ((device & 0xF) << 8))

/*
  * Full product type id definition, communicated between control processes and management processes
  * currently supported product id as follows:
  *		PRODUCT_ID_AX8600 	- 0xC8A500	(chassis,8 slots, marvell, cheetah2 4122)
  *		PRODUCT_ID_AX7K 	- 0xC5A000	(chassis,5 slots, marvell, cheetah2 275)
  *		PRODUCT_ID_AX5K 	- 0xB1A300	(box, 1 slots, marvell, cheetah2 265_lite<only use 12GE+2XG>)
  *		PRODUCT_ID_AX5K_I	- 0xB1A400	(box, 1 slots, marvell, cheetah2 265e_e<only use 8GE+2XG>)
  *		PRODUCT_ID_AX5K_E	- 0xB1C000  (box, 1 slots, cavium, octeon CN5230)
  *		PRODUCT_ID_AX5608	- 0xB1C100  (box, 1 slots, cavium, octeon CN5220)
  *		PRODUCT_ID_AU4K 	- 0xB1A100	(box, 1 slots, marvell, cheetah2 265)
  *		PRODUCT_ID_AU3K 	- 0xB1A200	(box, 1 slots, marvell, cheetah2 255)
  *		PRODUCT_ID_AU3K_BCM - 0xB1B000 	(box, 1 slots, broadcom, raven 56024B0)
  *
*/
enum product_id_e {
	PRODUCT_ID_NONE,
	PRODUCT_ID_AX8600 = PRODUCT_ID_GENERATE(PRODUCT_STK_CHASSIS_E, 8, PRODUCT_ASIC_MV_E, ASIC_MV_XCAT_BOBCAT_E),
	PRODUCT_ID_AX7K = PRODUCT_ID_GENERATE(PRODUCT_STK_CHASSIS_E, 5, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_275_E),
	PRODUCT_ID_AX7K_I = PRODUCT_ID_GENERATE(PRODUCT_STK_CHASSIS_E, 2, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_275_E),
	PRODUCT_ID_AX5K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_265L_E), 
	PRODUCT_ID_AX5K_I = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_265E_E), 
	PRODUCT_ID_AX5K_E = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_CHIP_CAVIUM_E, CPU_CVM_CN5230_E), 
	PRODUCT_ID_AX5608 = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_CHIP_CAVIUM_E, CPU_CVM_CN5220_E),
	PRODUCT_ID_AU4K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_265_E),
	PRODUCT_ID_AU3K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_255_E),
	PRODUCT_ID_AU3K_BCM = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_BCM_E, ASIC_BCM_RAVEN_56024B0_E),
	PRODUCT_ID_AU3K_BCAT = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_XCAT_BOBCAT_E),
	PRODUCT_ID_AU2K_TCAT = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_XCAT_TOMCAT_E),
	PRODUCT_ID_MAX
};

enum function_type_vlaue{
	FUNCTION_ACL_VLAUE = 0x1,
	FUNCTION_QOS_VLAUE = 0x2,
	FUNCTION_TUNNEL_VLAUE = 0x4,
	FUNCTION_PVLAN_VLAUE = 0x8,
	FUNCTION_IGMP_VLAUE = 0x10,
	FUNCTION_MLD_VLAUE = 0x20,
	FUNCTION_DLDP_VLAUE = 0x40,
	FUNCTION_STP_VLAUE = 0x80,
	FUNCTION_VLAN_VLAUE = 0x100,
	FUNCTION_INTERFACE_VLAUE = 0x200,
	FUNCTION_MIRROR_VLAUE = 0x400,
	FUNCTION_FDB_VLAUE = 0x800,
	FUNCTION_TRUNK_VLAUE = 0x1000,
	FUNCTION_ARP_VLAUE = 0x2000,
	FUNCTION_ESLOT_VLAUE = 0x4000,
	FUNCTION_ASIC_VLAUE = 0x8000,
	FUNCTION_OAM_VLAUE = 0x10000,
	FUNCTION_ROUTE_VLAUE = 0x20000
	
};

enum module_id_e {
	MODULE_ID_NONE ,	
	MODULE_ID_SUBCARD_TYPE_A0, /* module type for sub card with maximum 1 port (either Giga or XGiga) for AU5612 */
	MODULE_ID_SUBCARD_TYPE_A1, /* module type for sub card with maximum 1 port (either Giga or XGiga) for AX5612i */
	MODULE_ID_SUBCARD_TYPE_B, /* module type for sub card with maximum 2 ports (either Giga or XGiga) for AU4626 */
	MODULE_ID_AX7_CRSMU = PRODUCT_ID_AX7K + 1,       
	MODULE_ID_AX7_6GTX ,        
	MODULE_ID_AX7_6GE_SFP,
	MODULE_ID_AX7_XFP ,
	MODULE_ID_AX7_6GTX_POE ,   /* not implemented currently */
	MODULE_ID_AX5_5612 = PRODUCT_ID_AX5K + 1,
	MODULE_ID_AX5_5612I = PRODUCT_ID_AX5K_I + 1,
	MODULE_ID_AX5_5612E = PRODUCT_ID_AX5K_E + 1,
	MODULE_ID_AX5_5608 = PRODUCT_ID_AX5608 + 1,
	MODULE_ID_AU4_4626 = PRODUCT_ID_AU4K + 1,
	MODULE_ID_AU4_4524 , /* not implemented currently */
	MODULE_ID_AU4_4524_POE , /* not implemented currently */ 
	MODULE_ID_AU3_3524 = PRODUCT_ID_AU3K + 1,
	MODULE_ID_AU3_3028 = PRODUCT_ID_AU3K_BCM + 1, /* not implemented currently */
	MODULE_ID_AU3_3052,
	MODULE_ID_AU3_3528 = PRODUCT_ID_AU3K_BCAT + 1,
	MODULE_ID_AU2_2528 = PRODUCT_ID_AU2K_TCAT + 1,
	MODULE_ID_AX7I_CRSMU = PRODUCT_ID_AX7K_I + 1,
	MODULE_ID_AX7I_GTX,
	MODULE_ID_AX7I_XG_CONNECT,
	MODULE_ID_AX71_2X12G12S,
	MODULE_ID_AX81_SMU = PRODUCT_ID_AX8600 + 1,
	MODULE_ID_AX81_AC12C,
	MODULE_ID_DISTRIBUTED = 0xF0F0F0F0,
	MODULE_ID_MAX     
};

enum module_status_e {
	MODULE_STAT_NONE,
	MODULE_STAT_INITIALIZING,
	MODULE_STAT_RUNNING,
	MODULE_STAT_DISABLED,
	MODULE_STAT_MAX     /* max values */
};

enum system_fan_status_e {
	SYSTEM_FAN_STAT_NORMAL = 0,	/* system fan working regularly */
	SYSTEM_FAN_STAT_ALARM,		/* system fan alarming */
	SYSTEM_FAN_STAT_MAX	
};

/* NOTICE:
IMPORTANT, no unified ifindex, interface will be indexed based on different PHY IFTYPE.
There will be two major types of interface:
1). one is interface that displayed in the front panel
2). The other is interface that is logically defined, not displayed in the front panel 
*/

enum pif_type_e {      /* there could be some detail sub-type for each if types. */
	PIF_TYPE_ETH_PORT,    /* 802.3 ethernet port. */
	PIF_TYPE_WIFI_RADIO,  /* 802.11 wireless radio */
/*
	//More types that might be available in the future	
	PIF_TYPE_POS,		
	PIF_TYPE_ATM,		
	PIF_TYPE_FR,
	PIF_TYPE_X25,
	PIF_TYPE_SERIAL,
*/	
	PIF_TYPE_MAX
};


enum vif_type_e {
	VIF_TYPE_ETH_LAG,  /* Link Aggregation, Trunk */
    	VIF_TYPE_VLAN,
	VIF_TYPE_ETH_1QTAG,
	VIF_TYPE_IP4,
	VIF_TYPE_IP6,
	VIF_TYPE_CAPWAP,
	VIF_TYPE_IPSEC,
	VIF_TYPE_GRE,
	VIF_TYPE_MAX
};


enum eth_port_type_e {
	ETH_INVALID,    /* invalid for no iocard presented, port index 0 when start with 1, and initializing usage.*/
	ETH_FE_TX,  /* 100M copper */
	ETH_FE_FIBER, /* 100M FIBER */
	ETH_GTX, /* GE copper */
	ETH_GE_FIBER, /* GE fiber */
	ETH_GE_SFP,
	ETH_XGE_XFP,
	ETH_XGTX, /* XGE Copper */
	ETH_XGE_FIBER,
	ETH_MAX
};

#define NPD_SUCCESS 0
#define NPD_FAIL -1

#define NPD_TRUE 	1
#define NPD_FALSE	0

#define ETH_ATTR_ON 1
#define ETH_ATTR_OFF 0
#define ETH_ATTR_ENABLE 1
#define ETH_ATTR_DISABLE 0
#define ETH_ATTR_LINKUP 1
#define ETH_ATTR_LINKDOWN 0
#define ETH_ATTR_DUPLEX_FULL 0
#define ETH_ATTR_DUPLEX_HALF 1
#define ETH_ATTR_AUTONEG_DONE 1
#define ETH_ATTR_AUTONEG_NOT_DONE 0
#define ETH_ATTR_FC_ENABLE 1
#define ETH_ATTR_FC_DISABLE 0
#define ETH_ATTR_AUTONEG_SPEEDON 1 
#define ETH_ATTR_AUTONEG_SPEEDDOWN 0
#define ETH_ATTR_AUTONEG_FCON  1  
#define ETH_ATTR_AUTONEG_FCDOWN  0
#define ETH_ATTR_AUTONEG_DUPLEX_ENABLE 1 
#define ETH_ATTR_AUTONEG_DUPLEX_DISABLE 0 


#define ETH_ATTR_BP_ENABLE 1
#define ETH_ATTR_BP_DISABLE 0
#define ETH_ATTR_MEDIA_EXIST_PRIO  1
#define ETH_ATTR_MEDIA_NOT_EXIST_PRIO 0

/* Bits 0~11 to have 12 kinds of binary attributes */
#define ETH_ATTR_ADMIN_STATUS 	0x1	/* bit0 */
#define ETH_ATTR_LINK_STATUS 	0x2	/* bit1 */
#define ETH_ATTR_AUTONEG 	0x4	/* bit2: port auto-negotiation status */
#define ETH_ATTR_DUPLEX 	0x8	/* bit3 */
#define ETH_ATTR_FLOWCTRL 	0x10	/* bit4 */
#define ETH_ATTR_BACKPRESSURE 	0x20	/* bit5 */
#define ETH_ATTR_AUTONEG_SPEED 0x40	/* bit6 */
#define ETH_ATTR_AUTONEG_DUPLEX 0x80	/* bit7 */
#define ETH_ATTR_AUTONEG_FLOWCTRL 0x100	/* bit8 */
#define ETH_ATTR_AUTONEG_CTRL	0x200 	/* bit9 */

#define ETH_ADMIN_STATUS_BIT	0x0
#define ETH_LINK_STATUS_BIT	0x1
#define ETH_AUTONEG_BIT		0x2
#define ETH_DUPLEX_BIT		0x3
#define ETH_FLOWCTRL_BIT	0x4
#define ETH_BACKPRESSURE_BIT	0x5
#define ETH_AUTONEG_SPEED_BIT 	0x6
#define ETH_AUTONEG_DUPLEX_BIT 	0x7
#define ETH_AUTONEG_FLOWCTRL_BIT 0x8
#define ETH_AUTONEG_CTRL_BIT	0x9
#define ETH_ATTR_MEDIA_BIT              0x1c
#define ETH_PREFERRED_COPPER_MEDIA_BIT  0x1c
#define ETH_PREFERRED_FIBER_MEDIA_BIT 0x1d

/* Bits 12~15 to represent 4bits 16 kinds of speed */
#define ETH_ATTR_SPEED_MASK 		0xF000
#define ETH_ATTR_SPEED_10M 		0x0
#define ETH_ATTR_SPEED_100M 		0x1
#define ETH_ATTR_SPEED_1000M 		0x2
#define ETH_ATTR_SPEED_10G 		0x3
#define ETH_ATTR_SPEED_12G 		0x4
#define ETH_ATTR_SPEED_2500M 		0x5
#define ETH_ATTR_SPEED_5G			0x6
#define ETH_ATTR_SPEED_MAX 		0xF

#define ETH_SPEED_BIT				0xC

/* Bits 16~19 to represent 4bits of 16 kinds of pluggable port status, e.g. sfp/gbic */
#define ETH_ATTR_PLUGGABLE_PHY_TYPE_MASK 0xF0000
#define ETH_ATTR_PLUGGABLE_PHY_LX 0x10

/* Bits 20~21 reserved */


/* Bits 24~27 to represent 4bits(kinds) of basic functions with two state and no extend data structure */
#define ETH_ATTR_BASICFUNC_L2BRDGE 0x1000000 

/* preferred media 28~29 */
#define ETH_ATTR_PREFERRED_COPPER_MEDIA		0x10000000
#define ETH_ATTR_PREFERRED_FIBER_MEDIA 0x20000000
#define ETH_ATTR_MEDIA_MASK                 0x30000000


/* for mode convertion between fiber and copper */
typedef enum {
	PHY_MEDIA_MODE_NONE = 0,
	PHY_MEDIA_MODE_FIBER,
	PHY_MEDIA_MODE_COPPER
}PHY_MEDIA;  

/* Bits 30~31 reserved */

typedef enum {
	COMBO_PHY_MEDIA_PREFER_NONE = 0,
	COMBO_PHY_MEDIA_PREFER_FIBER,
	COMBO_PHY_MEDIA_PREFER_COPPER
}COMBO_PHY_MEDIA;

/*stp cfg default value*/

#define DEF_BR_PRIO 32768
#define MIN_BR_PRIO 0
#define MAX_BR_PRIO 61440

#define DEF_BR_HELLOT   2
#define MIN_BR_HELLOT   1
#define MAX_BR_HELLOT   10

#define DEF_BR_MAXAGE   20
#define MIN_BR_MAXAGE   6
#define MAX_BR_MAXAGE   40

#define DEF_BR_FWDELAY  15
#define MIN_BR_FWDELAY  4
#define MAX_BR_FWDELAY  30

#define DEF_BR_REVISION  0
#define MIN_BR_REVISION  0
#define MAX_BR_REVISION  65535

#define DEF_REMAINING_HOPS 20
#define MIN_REMAINING_HOPS 6
#define MAX_REMAINING_HOPS 40

#define MIN_MST_ID 0
#define MAX_MST_ID 64

#define STP_FORCE_VERS  2 /* NORMAL_RSTP */
#define MST_FORCE_VERS 3 /*NORMAL_MSTP*/

/* port configuration */
#define DEF_PORT_PRIO   128
#define MIN_PORT_PRIO   0
#define MAX_PORT_PRIO   240 /* in steps of 16 */

#define ADMIN_PORT_PATH_COST_AUTO   200000000 /*zhengcaisheng change 20->200000000*/
#define DEF_ADMIN_NON_STP   NPD_FALSE
#define DEF_ADMIN_EDGE      NPD_TRUE 
#define DEF_LINK_DELAY      3 /* see edge.c */
#define P2P_AUTO  2
#define DEF_P2P        P2P_AUTO

#define PVE_ERROR_MASK_BASE	0x100
#define PVE_AVOID_CYCLE_UPLINK (PVE_ERROR_MASK_BASE + 1)
#define PVE_ERROR_NOT_EXSIT (PVE_ERROR_MASK_BASE + 2)
#define PVE_ERROR_THIS_PORT_HAVE_PVE (PVE_ERROR_MASK_BASE + 3)
#define PVE_ERROR_PVE_UPLINK_PORT_NOT_IN_SAME_VLAN (PVE_ERROR_MASK_BASE + 4)
#define PVE_ERROR_PVE_PORT_NOT_IN_ONLY_ONE_VLAN (PVE_ERROR_MASK_BASE + 5)
enum eth_port_func_index_e {
	ETH_PORT_FUNC_PORT_BASED_VLAN,			/* untagged vlan */
	ETH_PORT_FUNC_DOT1Q_BASED_VLAN,			/* tagged vlan */
	ETH_PORT_FUNC_PROTOCOL_BASED_VLAN,		/* protocol-based vlan */
	ETH_PORT_FUNC_SUBNET_BASED_VLAN,		/* subnet-based vlan */
	ETH_PORT_FUNC_DOT1AD_EDGE, /* to be studied later on provider bridge functions. */
	ETH_PORT_FUNC_BRIDGE,	/* switch mode */
	ETH_PORT_FUNC_DOT1W,	/* spanning-tree protocol */
	ETH_PORT_FUNC_VLAN_TRANSLATION,
	ETH_PORT_FUNC_LINK_AGGREGATION,			/* trunk */
	ETH_PORT_FUNC_IPV4,				/* L3 interface route mode */
	ETH_PORT_FUNC_ACL_RULE,				/* ACL */
	ETH_PORT_FUNC_QoS_PROFILE,			/* QoS profile */
	ETH_PORT_FUNC_MODE_PROMISCUOUS,          	/* pve profile */
	ETH_PORT_FUNC_SUBIF,   				/* port have or not subif */
	ETH_PORT_FUNC_IGMP_SNP,				/* port enabled/disabled IGMP snooping Protocal */
	ETH_PORT_FUNC_PVE,   				/* private vlan info */
	ETH_PORT_FUNC_QoS_SHAPE,			/* qos shape */
	ETH_PORT_FUNC_FDB, 				/* port fdb setting */
	ETH_PORT_FUNC_STORM_CONTROL,
	ETH_PORT_FUNC_DLDP, 				/* port enabled/disabled DLDP Protocal */
	ETH_PORT_FUNC_DHCP_SNP,				/* DHCP-Snooping	*/
	ETH_PORT_FUNC_OCT_INFO,				/*port oct enable or disabe*/
	ETH_PORT_FUNC_MAX,
};

#define ETH_PORT_FUNC_BITMAP_FLAG(func_index) (0x1 << (func_index))

/* BOARD means chassis board(slot) or box product */
#define MAX_ETHPORT_PER_BOARD 64

/* 6~10  5bits respresent slot index of the chassis in the eth_index , 32 slots are supported at most */
#define SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_index)   (((eth_index) & 0x000007C0) >> 6)

/* 0~5   6bits represent eth port index in the board of eth_index, 64 ports per board are supported */
#define ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_index) ((eth_index) & 0x0000003F)

#define ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_INDEX(slot_index,eth_local_index) ((((slot_index) & 0x0000001F) << 6) + ((eth_local_index) & 0x0000003F))


/*
  *	SW build number format: X.Y.Z.n or X.Y.Z build n
  *	individual number meas as:
  *		X - major version number
  *		Y - minor version number
  *		Z - compatible version number, which also means internal test release version
  *		n - build number, which increase linearly when X and Y not changed.
  *		     When X or Y change, n reset to 0, Z is irrelevant of n's increase.
  */
#define SW_MAJOR_VER(ver) ((ver) >> 28)
/* 4bit 31-28 ,Major version usually start from 1, */
#define SW_MINOR_VER(ver) (((ver) & 0x0FFFFFFF ) >> 21)
/* 7bit 27-21 ,Minor version,usually start from 0, */
#define SW_COMPATIBLE_VER(ver) (((ver) & 0x001FFFFF) >> 14)
/* 7bit 20-14, Compatible version, usually start from 0, */
#define SW_BUILD_VER(ver) ((ver) & 0x00003FFF )
/* 14bit 13-0  ,Build No, usually start from 0, */

#define SW_INT_VER(maj,min,comp,build) 		\
	((((maj) & 0x0F) << 28) + (((min) & 0x7F) << 21) + (((comp) & 0x7F) << 14) + ((build) & 0x3FFF))

/*
   Hardware version definition
All module will have version like below, 4 bits of PCB version and 4 bits of cpld version.
*/

#define HW_PCB_VER(ver) (((ver) & 0x0000FFFF) >> 8)
/* indicate pcb version number,start with 0,then 1, 2, 3, and so on. */
#define HW_CPLD_VER(ver) ((ver) & 0x000000FF)
/* indicate cpld version number, start withs 0, then 1,2,3, */

/*
If HW version is 0xFF, then cpld register is not avaible, 
*/
#define HW_VER_IGNORE 0xFF


enum mirror_func_index_e {
	MIRROR_FUNC_PORT,		/* port type */
	MIRROR_FUNC_VLAN,		/* vlan */
	MIRROR_FUNC_FDB,		/* fdb */
	MIRROR_FUNC_POLICY,		/* policy */
	MIRROR_FUNC_MAX
};

/*****************************************************************************************/

/*****************************************************************************************/

/* defined for high speed channel ports short for HSP */
#define MAX_HSP_COUNT  32
#define HSP_MIB_ITEM_COUNT_EACH 16


/* npd syslog buffer size  - shared by npd/nam/nbm/cpss */
#define NPD_SYSLOG_BUFFER_SIZE	(256)
/* npd/nam/nbm/cpss syslog line buffer size */
#define NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE		(256)
/* syslog line buffer size used in npd */
#define NPD_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in nam */
#define NAM_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in nbm */
#define NBM_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* syslog line buffer size used in asic driver */
#define ASIC_SYSLOG_LINE_BUFFER_SIZE		NPD_COMMON_SYSLOG_LINE_BUFFER_SIZE
/* stp/rstp/mstp syslog buffer size */
#define STP_SYSLOG_BUFFER_SIZE	NPD_SYSLOG_BUFFER_SIZE

/* protocol-based vlan */

#define PROTOCOL_VLAN_MAP_MAX	8
typedef enum e_protocol_type {	/* ether-type */
	PROT_TYPE_IPV4 = 0,		/* 0x0800 */
	PROT_TYPE_IPV6,			/* 0x86DD */
	PROT_TYPE_PPPOE_D,		/* 0x8863 - PPPoE Discovery */
	PROT_TYPE_PPPOE_S,		/* 0x8864 - PPPoE Session */ 
	PROT_TYPE_UDF,			/* User-Defined-Field for ethertype */
	PROT_TYPE_MAX = 0xFF
} prot_type_e;

struct prot_vlan_map_t {
	unsigned short  vid:12,		/* vlan id mapped to protocol */
				  flag:1, 	/* this map is valid on the port or not */
				  rsvd:2;		/* reserved bits */
};

struct prot_vlan_ctrl_t {
	unsigned char state; 		/* protocol-VLAN is enable(1) or disable(0) */
	struct prot_vlan_map_t  vinfo[PROTOCOL_VLAN_MAP_MAX];
};

/* Add the define of board_info zhangdi*/
typedef struct {
    int system_type;
	int product_type;
    int board_type;
    int slot_id;
	int is_master;
	int is_active_master;
}BoardInfo;

typedef enum {
    BOARD_TYPE_AX81_SMU = 0,
    BOARD_TYPE_AX81_AC12C = 1,   
    BOARD_TYPE_AX81_AC8C = 2,   
    BOARD_TYPE_AX81_12X = 3,   
    BOARD_TYPE_AX71_2X12G12S = 4, /*include AX81_2X12G12S and AX71_2X12G12S*/
    BOARD_TYPE_AX81_1X12G12S = 5,
    BOARD_TYPE_AX81_AC4X = 6,
    BOARD_TYPE_AX81_SMUE = 7,
    BOARD_TYPE_AX71_CRSMU = 0x80,
    BOARD_TYPE_AXXX_VIRTUALBOARD=0x81,
    BOARD_TYPE_MAX
}BOARD_TYPE_E;
typedef enum
{
	ASIC_TYPE_START_EXMX_E,
	ASIC_TYPE_TWISTC_E,
	ASIC_TYPE_TWISTD_E,
	ASIC_TYPE_SAMBA_E,
	ASIC_TYPE_TIGER_E,
	ASIC_TYPE_END_EXMX_E,
	ASIC_TYPE_START_DXSAL_E,
	ASIC_TYPE_SALSA_E,
	ASIC_TYPE_END_DXSAL_E,
	ASIC_TYPE_START_DXCH_E,
	ASIC_TYPE_CHEETAH_E,
	ASIC_TYPE_CHEETAH2_E,
	ASIC_TYPE_CHEETAH3_E,
	ASIC_TYPE_DXCH_XCAT_E,
	ASIC_TYPE_DXCH_LION_E,
	ASIC_TYPE_DXCH_XCAT2_E,
	ASIC_TYPE_END_DXCH_E,
	ASIC_TYPE_START_EXMXPM_E,
	ASIC_TYPE_PUMA_E,
	ASIC_TYPE_END_EXMXPM_E,
	ASIC_TYPE_LAST_E,
	ASIC_TYPE_MAX_FAMILY = 0x7FFFFFFF
}ASIC_TYPE;

typedef enum {
	NO_DISTRIBUTED = 0,   /* need modify */
    IS_DISTRIBUTED = 1,
}SYSTEM_TYPE_E;

extern BoardInfo board_info;   /*  define in npd_board.c */
#define  SYSTEM_TYPE board_info.system_type
#define  PRODUCT_TYPE board_info.product_type
#define  BOARD_TYPE board_info.board_type
#define  SLOT_ID    board_info.slot_id
#define  IS_MASTER_NPD    board_info.is_master
#define  IS_ACTIVE_MASTER_NPD    board_info.is_active_master
extern unsigned char npd_cscd_type;     /*  define in npd_board.c */
#define  CSCD_TYPE npd_cscd_type
/* add end */

/*  Add for distributed  */
/* ETH-PORT attribute struc used to set port default attributes */

typedef struct asic_port_attribute_s {
	unsigned char 	admin_state; /* administrative status */
	unsigned short 	mtu;		/* port MTU or MRU */
	unsigned char 	duplex;		/* duplex mode */
	unsigned char 	speed;		/* speed */
	unsigned char 	fc;			/* flow-control */
	unsigned char 	bp;			/* back-pressure */
	unsigned char 	autoNego;	/* Auto-Negotiation */
	unsigned char 	duplex_an;	/* duplex Auto-Negotiation */
	unsigned char 	speed_an;	/* speed Auto-Negotiation */
	unsigned char 	fc_an;		/* flow-control Auto-Negotiation */
	/*unsigned char   copper;	*/	/* preferred copper*/
	/*unsigned char   fiber;*/		/* preferred fiber*/
	unsigned char   mediaPrefer;/* media preferred*/
}ASIC_PORT_ATTRIBUTE_S;


/*
 * Ethernet port Main Data Structure.
 */
typedef struct asic_eth_port_s {
	enum eth_port_type_e port_type;
	/*
	32 bits of attributes, defined in npd_sysdef.h
	Bits 0~11 to have 12 kinds of binary attributes
	Bits 12~15 to represent 4bits 16 kinds of speed,
	Bits 16~31 reserved for future more attributes
	*/
	unsigned int attr_bitmap;
	unsigned int mtu;
	unsigned int ipg;
	unsigned int dev_num;   /* asic dev num, the scan order of PP */
	unsigned int port_num;   /* asic port num, used to get reg address */
	unsigned long	lastLinkChange;/* timestamps of last link change in unit of second*/
	
    ASIC_PORT_ATTRIBUTE_S* asic_default_attr;

	struct eth_port_func_data_s* funcs;
	struct eth_port_counter_s* 	counters;

	#if 0
	struct stp_info_s 			stpInfo;	/* STP/RSTP state and control info */
	struct acl_rule_list_s 		*aclRule;	/* ACL rule list bind to this port */
	struct qos_policy_list_s	*qosProfile;
	struct eth_port_drv_info_s	drvInfo; 	/* Port<n> Configuration driver info shadow */ /* impl API in cpss !!!!!!!!*/
	#endif
}asic_eth_port_t;
typedef struct asic_cscd_port_s {
	unsigned int dev_num;   /*  PPcounter >1 */
	unsigned int port_num;	
}asic_cscd_port_t;

typedef struct asic_cpu_port_s {
	int dev_num;
	int port_num;	
}asic_cpu_port_t;

typedef struct asic_board_s {
	int board_type;
	char *board_name;
    int slot_id;
	int asic_type;
	int capbmp;
	int runbmp;	
	int asic_port_cnt;
	int asic_cscd_port_cnt;
	int asic_port_cnt_visable;	
	int asic_port_start_no;
	int* asic_port_mapArr;
    asic_eth_port_t* asic_eth_ports;
	asic_cscd_port_t * asic_cscd_ports;
	asic_cpu_port_t * asic_to_cpu_ports;
    /* module_info_s */
	enum module_id_e id;
	enum module_status_e state;
	unsigned char hw_version;
	char *sn;
	void *ext_slot_data;   /* extend info could be organized as a link list.*/
	
	
    long (* func_test) (int product_type, int delete_board_type, int delete_slotid);
    long (* asic_init)();
    unsigned int (* my_equipment_test)();	
	int (* distributed_system_dev_map_table_set)();
	int (* board_cscd_port_trunk_config)();
}asic_board_t;
extern asic_board_t* asic_board;
/* add for distributed vlan */
typedef struct port_bmp{
    unsigned int low_bmp;
    unsigned int high_bmp;
}port_bmp_t;
typedef struct vlan_list_distributed{
    unsigned char vlanStat;
    unsigned char updown;
	unsigned char cscd_qinq_state;/*cscd port qinq state*/
    char vlanName[21];
	unsigned int cpu_port_qinq[16];/*to cpu port qinq state*/
    unsigned int bond_slot[16];	
#ifdef __VERSION_2_0_22
	unsigned int bond_x86_slot[16];	
#endif
    port_bmp_t untagPortBmp[16]; 
	port_bmp_t tagPortBmp[16];
	port_bmp_t qinq[16];     /*eth port qinq state*/
	unsigned char untagTrkBmp[127];
	unsigned char tagTrkBmp[127];
}vlan_list_t;
extern vlan_list_t* g_vlanlist;
extern vlan_list_t* g_vlanback;
typedef struct asic_cscd_bport_s{
    char cscd_port;
	char master;
	char asic_id;
	char bport;
	char trunk_id;
}asic_cscd_bport_t;

typedef struct asic_board_cscd_bport_s {
    char board_type;
	char slot_id;
	char slot_num;
	char asic_cscd_port_cnt;
	char asic_to_cpu_ports;
	asic_cscd_bport_t asic_cscd_bports[16];
}asic_board_cscd_bport_t;
extern asic_board_cscd_bport_t* g_bportlist;
/* Add end */

#define MAX_CPU_PORT_NUM 16     /* max cpu port num on per board,8 * cpu_num */
#define MAX_SLOT_COUNT  16
#define INVALID_ETHPORT    0
#define VALID_ETHPORT      1
/*
sush@autelan.com 2/Jul/2011
remove redefinitions.

enum board_state
{
	BOARD_REMOVED,
	BOARD_INSERTED,
	BOARD_INITIALIZING,
	BOARD_READY,
	BOARD_RUNNING
};

enum
{
	PRODUCT_INITIAL,
	PRODUCT_DSICOVER,
	PRODUCT_READY,
	PRODUCT_IDLE,
	PRODUCT_RUN
};
*/
#endif
