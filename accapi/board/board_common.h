#ifndef __BOARD_COMMON_H_
#define __BOARD_COMMON_H_

enum port_type
{
	CPU,
	ASIC,
	UNKNOWN
};

#define MASTER_TEMP1_INPUT "/sys/bus/i2c/devices/0-0018/temp1_input"
#define MASTER_TEMP2_INPUT "/sys/bus/i2c/devices/0-0018/temp2_input"

/*wangchao add: support to TMP421*/
#define TMP421_MASTER_TEMP1_INPUT "/sys/bus/i2c/devices/0-004c/temp1_input"
#define TMP421_MASTER_TEMP2_INPUT "/sys/bus/i2c/devices/0-004c/temp2_input"


#define BOARD_GLOBAL_ETHPORTS_MAXNUM 64
#define GLOBAL_ETHPORTS_MAXNUM       (64*16)
#define INVALID_ETHPORT    0
#define VALID_ETHPORT      1

#define SEM_TRUE 	1
#define SEM_FALSE	0
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
#define ETH_ATTR_PREFERRED_FIBER_MEDIA      0x20000000
#define ETH_ATTR_MEDIA_MASK                 0x30000000

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

struct eth_port_counter_s {
	struct tx_s {
		unsigned long long excessiveCollision;
		unsigned long long goodbytesl;
		unsigned long long goodbytesh;
		unsigned long long uncastframe;
		unsigned long long bcastframe;
		unsigned long long mcastframe;
		unsigned long long fcframe;
		unsigned long long sentMutiple;
		unsigned long long crcerror_fifooverrun;
		unsigned long long sent_deferred;
	} tx;
	struct rx_s {
		unsigned long long uncastframes;
		unsigned long long bcastframes;
		unsigned long long mcastframes;
		unsigned long long fifooverruns;
		unsigned long long goodbytesl;
		unsigned long long goodbytesh;
		unsigned long long badbytes;
		unsigned long long fcframe;
		unsigned long long errorframe;
		unsigned long long jabber;
		unsigned long long underSizepkt;
		unsigned long long overSizepkt;
		unsigned long long fragments;
	} rx;
	struct oct_s{
		unsigned long long late_collision; 
		unsigned long long collision;
		unsigned long long badCRC;
		unsigned long long b1024oct2max;
		unsigned long long b512oct21023;
		unsigned long long b256oct511;
		unsigned long long b128oct255;
		unsigned long long b64oct127;
		unsigned long long b64oct;
	}otc;
	unsigned int linkupcount;
	unsigned int linkdowncount;
};
#define ETH_PORT_FUNC_MAX 22
struct eth_port_func_data_s {
/*
	Bitmap of functions that is enabled on this port, we support at most 32 ethernet port functions.
	This can be externd to 64 functions in future.
	There is a definition of these bitmap in npd_sysdef.h file
*/
	unsigned int funcs_cap_bitmap; /* Port capability, hardware dependant, initialized in the beginning*/
	unsigned int funcs_run_bitmap; /* Port running funcs, can be configured by user during runtime*/
/*
	NOTICE
*/
	void *func_data[ETH_PORT_FUNC_MAX];

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

typedef enum {
	PORT_SPEED_10_E,
	PORT_SPEED_100_E,
	PORT_SPEED_1000_E,
	PORT_SPEED_10000_E,
	PORT_SPEED_12000_E,
	PORT_SPEED_2500_E,
	PORT_SPEED_5000_E
} PORT_SPEED_ENT;


typedef enum {
	SEM_DUPLEX_HALF = 0,
	SEM_DUPLEX_FULL
}COMBO_PHY_DUPLEX;

/**
  * Enums from
  */
typedef enum {
	PORT_FULL_DUPLEX_E,
	PORT_HALF_DUPLEX_E
} PORT_DUPLEX_ENT;


typedef struct _eth_port_attribute_s {
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
}port_default_attribute_t;

typedef struct global_ethport_s {
	unsigned int isValid;
	unsigned int cpu_or_asic;
	unsigned int eth_g_index;
	unsigned int slot_no;
	unsigned int local_port_no;
	unsigned int local_port_index;
	unsigned int ipg;
	unsigned int phy_id;
	unsigned int devNum;
	unsigned int attr_bitmap; 
	unsigned int mtu;
	unsigned int linkchanged;/*timestamps of last link change in unit of second*/
	unsigned long lastLinkChange;/*timestamps of last link change in unit of second*/
	enum eth_port_type_e port_type;
    port_default_attribute_t port_default_attr;
/*	struct eth_port_func_data_s funcs;
	struct eth_port_counter_s 	counters;
*/	
}global_ethport_t;

/*
 *
 * Ethernet port Main Data Structure.
 *
 */
struct eth_port_s {
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

	struct eth_port_func_data_s funcs;
	struct eth_port_counter_s 	counters;

	unsigned long	lastLinkChange;/* timestamps of last link change in unit of second*/
	#if 0
	struct stp_info_s 			stpInfo;	/* STP/RSTP state and control info */
	struct acl_rule_list_s 		*aclRule;	/* ACL rule list bind to this port */
	struct qos_policy_list_s	*qosProfile;
//	struct eth_port_drv_info_s	drvInfo; 	/* Port<n> Configuration driver info shadow */ /* impl API in cpss !!!!!!!!*/
	#endif
};



typedef struct
{
	int cpu_or_asic;
	int port_type;
	int phy_id;
	int devPort;         /* reserved */
	int ipd_port_id;
	port_default_attribute_t port_attribute;  
}board_port_t;


/*universial cpld reg define*/
#define CPLD_TEST_REG					0x0
#define CPLD_PRODUCT_SERIAL_REG				0x1
#define CPLD_PRODUCT_TYPE_REG				0x2
#define	CPLD_SINGAL_BOARD_TYPE_REG			0x3
#define CPLD_PCB_VERSION_REG				0x4
#define CPLD_VERSION_REG				0x5
#define CPLD_HW_RESET_CONTROL_REG1			0x6
#define CPLD_HW_RESET_CONTROL_REG2			0x7
#define CPLD_HW_SELFT_RESET_REG				0x8
#define CPLD_INTERRUPT_MASK_REG1			0x9
#define CPLD_INTERRUPT_MASK_REG2			0xa
#define CPLD_INTERRUPT_STATE_REG1			0xb
#define CPLD_INTERRUPT_STATE_REG2			0xc
#define CPLD_WATCHDOG_ENABLE_REG			0xd
#define CPLD_WATCHDOG_OVERFLOW_TIME_REG		0xe
#define CPLD_WATCHDOG_RESET_REG				0xf
#define CPLD_INTERRUPT_RESULT_REG2			0x11
#define CPLD_REG16H							0x16							
#define CPLD_INTERRUPT_SOURCE_STATE_REG1	0x17
#define CPLD_FORCE_ACTIVE_OR_STBY_REG		0x1a
#define CPLD_SLOT_ID_REG					0x28
#define CPLD_REMOTE_HW_RESET_REG			0x2a
#define CPLD_REMOTE_ON_STATE_REG			0x29
#define CPLD_INTERRUPT_STATE_REG3			0x31
#define CPLD_INTERRUPT_RESULT_REG3			0x32
#define CPLD_REMOTE_MASTER_STATE_REG		0x37
#define CPLD_LOCAL_MASTER_STATE_REG			0x38
#define CPLD_FAN_RPM                        0x3a

#define CPLD_TEMPERATURE_ALARM				0x3f

// TODO:not defined
/*cpld universial reg mask*/
#define CPLD_PRODUCT_SERIAL_MASK 			0xf	//reg1
#define CPLD_PRODUCT_TYPE_MASK				0x7	//reg2
#define CPLD_SINGAL_BOARD_TYPE_MASK			0xff //reg3
#define SLOT_ID_MASK						0xf	//reg28

#define CPLD_SELF_RESET_DATA				0xfe

enum fan_rpm
{
    FAN_125_RPM,
	FAN_250_RPM,
    FAN_375_RPM,
	FAN_500_RPM,	
    FAN_625_RPM,
	FAN_750_RPM,
    FAN_875_RPM,
	FAN_1000_RPM,	
};

#define BM_IOC_MAGIC 0xEC 

#define BM_IOC_READ_MODULE_SYSINFO _IOWR(BM_IOC_MAGIC, 6, ax_module_sysinfo)/*read module sysinfo for series 7 or series 6*/

typedef struct ax_read_module_sysinfo
{
	int product_id;					/*the product's id must be 3~7 for 3000~7000*/
	int slot_num;							/*0~4 */
	char  ax_sysinfo_module_serial_no[21]; /*data should be 21 bytes */
	char  ax_sysinfo_module_name[25];  /*data max length should be 24 bytes */
}ax_module_sysinfo;

#endif
