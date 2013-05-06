#ifndef __NPD_AMAPI_H__
#define __NPD_AMAPI_H__

typedef unsigned char boolean;

#ifndef _1K
#define 	_1K		1024
#endif

/*virtual slot/port number for CPU & SPI(alias HSP) */
#define CPU_PORT_VIRTUAL_SLOT	 0x1F
#define CPU_PORT_VIRTUAL_PORT	 0x3F /* for CPU port (*,63)*/
#define SPI_PORT_VIRTUAL_SLOT	 CPU_PORT_VIRTUAL_SLOT
#define SPI_PORT_VIRTUAL_PORT	 0x1A /* for SPI port (*,26)*/

/* virtual eth-port index for CPU & HSP */
#define CPU_PORT_VINDEX	(0)
#define HSC_PORT_VINDEX_AX7	(0xFFFF)
#define HSC_PORT_VINDEX_AX5	(0xFFFE)
#define HSC_PORT_VINDEX_AX5I (0xFFFD)

/* asic port number for CPU & HSP on */
/* cpu port (0,63) */
#define CPU_PORT_ASIC_DEVNO (0)
#define CPU_PORT_ASIC_PORTNO	(0x3F)
#define CPU_PORT_GENERAL_ASIC_DEVNO	CPU_PORT_ASIC_DEVNO
/* HSC uplink port (1,26) on PRODUCT_ID_AX7K */
#define HSC_PORT_UPLINK_DEVNO_AX7	(1)
#define HSC_PORT_UPLINK_PORTNO_AX7 	(0x1A)
/* HSC uplink port (0,0) on PRODUCT_ID_AX5K */
#define HSC_PORT_UPLINK_DEVNO_AX5	(0)
#define HSC_PORT_UPLINK_PORTNO_AX5 	(0)
/* HSC uplink port (0,24) on PRODUCT_ID_AX5K_I */
#define HSC_PORT_UPLINK_DEVNO_AX5I	(0)
#define HSC_PORT_UPLINK_PORTNO_AX5I 	(0x18)

/* default OAM settings*/
#define OAM_TELNET_DPORT_DEFAULT	(23)
#define OAM_SSH_DPORT_DEFAULT		(22)
#define OAM_HTTP_DPORT_DEFAULT	(80)
#define OAM_HTTPS_DPORT_DEFAULT	(443)
#define OAM_SNMP_DPORT_DEFAULT	(161)
#define WIRELESS_CAPWAP_CNTL_DEFAULT	 (1234)
#define WIRELESS_CAPWAP_DATA_DEFAULT	 (8888)

/* CPU port dma channel */
#define NPD_ENGINE_CPU_CHANNEL_NUM	8

/*max asic nums*/
#define MAX_ASIC_NUM         2      

enum OAM_TRAFFIC_TYPE_ENT {
	OAM_TRAFFIC_TYPE_TELNET_E = 0,
	OAM_TRAFFIC_TYPE_SSH_E,
	OAM_TRAFFIC_TYPE_HTTP_E,
	OAM_TRAFFIC_TYPE_HTTPS_E,
	OAM_TRAFFIC_TYPE_SNMP_E,
	WIRELESS_CAPWAP_CNTL_E,
	WIRELESS_CAPWAP_DATA_E,
	OAM_TRAFFIC_TYPE_MAX
};

enum  ETH_PORT_NOTIFIER_ENT {
	ETH_PORT_NOTIFIER_LINKUP_E = 0,
	ETH_PORT_NOTIFIER_LINKDOWN_E,
	ETH_PORT_NOTIFIER_LINKPOLL_E,	/* for eth-port link status polling message*/
	ETH_PORT_NOTIFIER_TYPE_MAX	
};

enum VLAN_PORT_SUBIF_FLAG {
	VLAN_PORT_SUBIF_NOTEXIST = 0,
	VLAN_PORT_SUBIF_EXIST
};

typedef int (*NPD_ETH_PORT_NOTIFIER_FUNC)(unsigned int,enum ETH_PORT_NOTIFIER_ENT);

typedef int (*NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC)(unsigned int,void*,enum ETH_PORT_NOTIFIER_ENT);

struct npd_common_list_s {
	unsigned int		count;
	struct list_head	list;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_link_change_notifier_func;
};

/**********************************************************************************************/
/*																										   */
/*										ACL Rule info													         */
/*																										   */
/**********************************************************************************************/

/*
 *
 * ACL Rule Main Data Structure.
 *
 */
struct acl_group_list_s
{
  unsigned int count;
  int          devnum;
  unsigned int pclId; 
  unsigned int stdrule;
  unsigned int eth_count;	/*counters of ingress group binded by several ports */
  unsigned int vlan_count;	/* counters of ingress group binded by vlan id*/
  struct list_head list1;  /*acl rule index*/
  struct list_head list2;  /*port index*/
  struct list_head list3;  /*vlan index*/
};

/*
 *
 * ACL Rule info list.
 *
 */
 struct acl_rule_info_s {
	unsigned long      ruleIndex;
	struct list_head   list;
};
/*
  acl direction
*/
typedef enum {
        ACL_DIRECTION_INGRESS_E = 0, 
		ACL_DIRECTION_EGRESS_E,	
		ACL_DIRECTION_TWOWAY_E,
		ACL_DIRECTION_NONE_DISABLE_E
}ACL_DIRECTION_E;
/*
  ethport_acl_group_info
*/
struct eth_group_info{
	unsigned int     groupId;/*ingress groupId*/
	unsigned int     EgGroupId;/*gress groupId*/
	ACL_DIRECTION_E  direction;
};
struct eth_vct_info {
	unsigned int vct_isable;
};
/*
*group_ethport_info
*
*/
struct group_eth_info{
	unsigned int eth_index;
	struct list_head   list;
};

/**/
struct group_vlan_info{
	unsigned int vlan_index;
	struct list_head   list;

};

struct acl_range_rule {
	unsigned int	startIndex; 
	unsigned int	endIndex;
	unsigned long	startDip;
	unsigned long	endDip;
	unsigned long	startSip;
	unsigned long	endSip;
	unsigned int	ruleType;
	unsigned int	policer;
	unsigned int	startPid;
	unsigned int	endPid;
};

struct group_udp_vlan_rule{
	unsigned int	groupNo;
	unsigned int	vlanId;
	unsigned int	ruleType;
};

/**********************************************************************************************/
/*																										   */
/*										QoS Policy info													   */
/*																										   */
/**********************************************************************************************/

/*
*	QOS-profile Data Structure.
*/
typedef enum
{
	QOS_DP_GREEN_E = 0 ,
	QOS_DP_YELLOW_E ,
	QOS_DP_RED_E			   
}QOS_DP_LEVEL_E;
	

typedef struct
{
    QOS_DP_LEVEL_E            dropPrecedence;
    unsigned int              userPriority;
    unsigned int              trafficClass;
    unsigned int              dscp;
    unsigned int              exp;                
}QOS_PROFILE_STC;

typedef struct
{
    unsigned int			profileIndex;   
}QOS_UP_PROFILE_MAP_STC;

typedef struct QOS_APPEND_PROFILE_STC
{
    unsigned int 			aclIndex;
    unsigned int			profileIndex;   
}QosAppendProfile;

typedef struct
{
    unsigned int			profileIndex;
}QOS_DSCP_PROFILE_MAP_STC;

typedef struct
{
    unsigned int			NewDscp;   
}QOS_DSCP_DSCP_REMAP_STC;

/*
 *
 * QOS-policy map Data Structure.
 *
 */

typedef enum 
{
	KEEP_E = 0,
	DISABLE_E ,	
	ENABLE_E, 
	INVALID_E
}QOS_NORMAL_ENABLE_E;
typedef enum
{
	QOS_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E = 0,
	QOS_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E 
}QOS_ATTRIBUTE_ASSIGN_PRECEDENCE_E;

typedef enum
{
	PORT_UNTRUST_E = 0,
	PORT_L2_TRUST_E ,
	PORT_L3_TRUST_E ,
	PORT_L2_L3_TRUST_E
}QOS_PORT_TRUST_MODE_FLAG;

typedef struct
{
	/*unsigned int 			 		defaultUp;*/
	QOS_ATTRIBUTE_ASSIGN_PRECEDENCE_E  assignPrecedence;
	/*unsigned int 					 defaultProfileIndex;*/
	QOS_PORT_TRUST_MODE_FLAG 		trustFlag;
	QOS_NORMAL_ENABLE_E      		modifyUp;
	QOS_NORMAL_ENABLE_E      		modifyDscp;
	unsigned int	         		remapDscp;
	unsigned int					eth_count;
	struct list_head				list;
}QOS_PORT_POLICY_MAP_ATTRIBUTE_STC;

/**
*	QOS eth-policyMap info
*/
typedef struct
{
	unsigned int policyMapId;
}QOS_PORT_POLICY_MAP_INFO_STC;

/*
	QOS policy-map ethInfo
*/
typedef struct
{
	unsigned int 		ethIndex;
	struct list_head 	list;
}QOS_POLICY_MAP_ETH_INDEX_STC;

/*
out profile action
*/
typedef enum
{
	OUT_PROFILE_KEEP_E = 0 ,
	OUT_PROFILE_DROP ,
	OUT_PROFILE_REMAP_ENTRY,
	OUT_PROFILE_REMAP_TABLE
}QOS_PROFILE_OUT_PROFILE_ACTION_E;

typedef enum
{
    POLICER_COLOR_BLIND_E,
    POLICER_COLOR_AWARE_E
} QOS_POLICER_COLOR_MODE_ENT;
typedef enum
{
	POLICER_TB_STRICT_E,
	POLICER_TB_LOOSE_E
}QOS_METER_MODE_E;
typedef enum
{
	POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E, 
    POLICER_PACKET_SIZE_L3_ONLY_E, 
    POLICER_PACKET_SIZE_L2_INCLUDE_E,
    POLICER_PACKET_SIZE_L1_INCLUDE_E
}QOS_POLICING_MODE_E;

typedef enum 
{
	POLICER_MRU_1536_E,
	POLICER_MRU_2K_E,
	POLICER_MRU_10K_E
}QOS_POLICER_MRU_E;


/*
	qos policer
*/
typedef struct{
	unsigned int                              policerEnable;
	unsigned int 					 		rangepolicer;	
	unsigned int 					 		sharemode;
	QOS_POLICER_COLOR_MODE_ENT                meterColorMode;
	unsigned long								cir;
	unsigned long								cbs;
	unsigned int                              counterEnable;
	unsigned int                              counterSetIndex;
	QOS_PROFILE_OUT_PROFILE_ACTION_E          cmd;
	unsigned int                              qosProfile;
	QOS_NORMAL_ENABLE_E     					modifyDscp;
	QOS_NORMAL_ENABLE_E     					modifyUp;
}QOS_POLICER_STC;

typedef struct{
	unsigned int	aliaspolicer;
	unsigned int 	startpolicer;
	unsigned int 	endpolicer;
}QOS_POLICER_ALIAS;


/*
	global qos policer
*/
typedef struct{
	QOS_METER_MODE_E						  meter;
	QOS_POLICING_MODE_E 					  policing;
	QOS_POLICER_MRU_E						  mru;
}QOS_GLOBAL_POLICER_STC;
/*
	counter
*/
typedef struct{	
  unsigned long    outOfProfileBytesCnt;
  unsigned long    inProfileBytesCnt;
}QOS_COUNTER_STC;


/**************
   queue scheduler
**************************/
typedef enum{
	QOS_PORT_TX_WRR_ARB_GROUP0_E = 0,
	QOS_PORT_TX_WRR_ARB_GROUP1_E,	
	QOS_PORT_TX_SP_ARB_GROUP_E,
	QOS_PORT_TX_WRR_ARB_E,
	QOS_PORT_TX_DEFAULT_WRR_ARB_E,
	QOS_PORT_TX_SP_WRR_ARB_E
}QOS_QUEUE_ALGORITHEM_FLAG_E;

typedef struct{
	unsigned int	groupFlag;	
	unsigned int	weight;
}QOS_WRR_TX_WEIGHT_E;
/***********
	traffic shape 
***************/

typedef struct{
	unsigned int     queueEnable;
	unsigned long 	 Maxrate;
	unsigned int 	 burstSize;
	unsigned int	 kmstate;
}QOS_SHAPER_QUEUE;


typedef struct{
	unsigned int		portEnable;
	unsigned long 		Maxrate;
	unsigned int  		burstSize;
	unsigned int		kmstate;
	QOS_SHAPER_QUEUE    queue[8];	
}QOS_TRAFFIC_SHAPE_PORT_STC;

/**********************************************************************************************/
/*																										   */
/*										IGMP Snooping info													   */
/*																										   */
/**********************************************************************************************/

/*
 *
 *  IGMP Snooping Main Data Structure.
 *
 */
struct igmp_snooping_info_s {
	unsigned int	groupId;
	unsigned int	vid;
};

#define SIZE_OF_IPV6_ADDR	8
/*
 *
 * IGMP Snooping info list.
 *
 */
typedef struct npd_common_list_s igmp_snooping_list_s;

struct igmp_snooping_list_node_s {
	struct 	list_head	list;
	unsigned int		groupIp;
	unsigned short 		grouIpV6[SIZE_OF_IPV6_ADDR];
	unsigned short 		vidx;
};


/**********************************************************************************************/
/*																										   */
/*										DHCP Snooping info												   */
/*																										   */
/**********************************************************************************************/

/*
 *
 *  DHCP Snooping Main Data Structure.
 *
 */

struct dhcp_snooping_info_s {
	unsigned int	itemId;
	unsigned long	ipAddress;
	unsigned short	vid;
	unsigned int	ifindex;	/* ether port global index :caculated by slot/port */
};

/*
 *
 * DHCP Snooping info list.
 *
 */
typedef struct npd_common_list_s dhcp_snooping_list_s;

struct dhcp_snooping_list_node_s {
	struct 	list_head	list;
	unsigned int		itemId;
};

/**********************************************************************************************/
/*																										   */
/*										FDB Entry Info     													   */
/*																										   */
/**********************************************************************************************/
#define 	MAC_ADDRESS_LEN	 6
#define    IPV4_ADDRESS_LEN  4

/*
 *
 *	MAC unicast address can be associated with device/port or trunk group.
 *
 */
struct 	fdb_interface_info_s {
	boolean 		isTrunk;
	
	union {
		struct	 {
			unsigned char 	devNum;
			unsigned char	portNum;
		} port;
		struct  {
			unsigned char 	devNum;
			unsigned char	trunkId;
		} trunk;
	} intf;
};

struct eth_port_fdb_s{
	unsigned int fdbLimit; /*port-based FDB protected number,added by zhengcs*/
	unsigned int fdbCount; /*port-based FDB protected number learned count,added by zhengcs*/
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_fdb_port_limit_notifier_func;
};


	
/*
 *
 * VIDX :
 *		value 0~0xEFF means multicast group table index for Multicast( MAC or IPv4/v6) entry.
 *		value 0xFFF means multicast packet is forwarded according to VID group membership.
 *
 */
struct arp_snooping_item_s {
	unsigned int	ifIndex;
	unsigned char 	mac[MAC_ADDRESS_LEN];
	unsigned int	ipAddr;
	unsigned char	isTagged;
	unsigned short	vid;
	unsigned short	vid2;
	struct	fdb_interface_info_s  dstIntf;
	unsigned short	vidx;
	boolean			isStatic;
	boolean         isValid;  /* only used for static arp items.*/
};

/**
 *
 *	Next-Hop Table info (NHT)
 *	here use tblIndex to hold Next-Hop Table index 
 *
 */
struct route_nexthop_brief_s {
	unsigned int	ifIndex;  	/* L3 interface index*/
	unsigned int	ipAddr;		/* ip address associated with L3 interface*/
	unsigned int	tblIndex;	/* Next-Hop Table index to hold Next-Hop detail info*/
	unsigned int	rtUsedCnt;	/* counter for this Next-Hop used by Route entry*/
};
/*
 *
 * FDB entry list (ARP snooping list)
 *
 */
/*typedef struct npd_common_list_s arp_snooping_entry_list_s;*/


/**********************************************************************************************/
/*																										   */
/*										ETHERNET PORT 													   */
/*																										   */
/**********************************************************************************************/

/*
  NOTICE
  This file define data structures that are shared between npd and nam
*/
struct port_tx {
	unsigned int excessiveCollision;
	unsigned int goodbytesl;
	unsigned int goodbytesh;
	unsigned int uncastframe;
	unsigned int bcastframe;
	unsigned int mcastframe;
	unsigned int fcframe;
	unsigned int sentMutiple;
	unsigned int crcerror_fifooverrun;
	unsigned int sent_deferred;	
};
struct port_rx {
	unsigned int uncastframes;
	unsigned int bcastframes;
	unsigned int mcastframes;
	unsigned int fifooverruns;
	unsigned int goodbytesl;
	unsigned int goodbytesh;
	unsigned int badbytes;
	unsigned int fcframe;
	unsigned int errorframe;
	unsigned int jabber;
	unsigned int underSizepkt;
	unsigned int overSizepkt;
	unsigned int fragments;
	unsigned int dropped;
};

struct port_oct_s{
	unsigned int late_collision; 
	unsigned int collision;
	unsigned int badCRC;
	unsigned int b1024oct2max;
	unsigned int b512oct21023;
	unsigned int b256oct511;
	unsigned int b128oct255;
	unsigned int b64oct127;
	unsigned int b64oct;
};

struct npd_port_counter{
	struct port_tx tx;
	struct port_rx rx;
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

/*
 * 
 * 	STP port state (assume STP_LEARNING & STP_FORWARDING are greater than other states)
 *
 */
enum stp_state_ent_s {
	STP_DISABLED,		/* (8.4.5) */
	STP_BLOCKING,		/* (8.4.1) */
	STP_LISTENING,		/* (8.4.2) */
	STP_LEARNING,		/* (8.4.3) */
	STP_FORWARDING, 	/* (8.4.4) */
	STP_INVALID 		/* the stp port is not there */
};


enum stp_running_mode{
	STP_MODE = 0,
	MST_MODE
};
/*
 *
 *	STP state associated with port or trunk group.
 *
 */
struct stp_info_s {
	boolean			stpEnable;
	enum stp_running_mode mode;
	unsigned int 	mstid[MAX_MST_ID];
	unsigned int 	pathcost[MAX_MST_ID];
	unsigned int 	prio[MAX_MST_ID];
	unsigned int 	p2p;
	unsigned int 	edge;
	unsigned int		nonstp;
	enum stp_state_ent_s state;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_stp_port_notifier_func;
};

/*
 *
 *	L3 interface associated with port.
 *
 */
struct eth_port_intf_s {
	unsigned int ifIndex;
	unsigned char uniMac[6];/* l3 interface unique mac, only used when different from system mac (default FF:FF:FF:FF:FF:FF)*/
	unsigned char state; /* l3 interface state (0 - down, 1 -up)*/
	enum VLAN_PORT_SUBIF_FLAG flag;
	unsigned char l3_disable;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_route_port_notifier_func;
};

union uplink_port {

		struct {
			unsigned char slot_no;
			unsigned char port_no;
			
		}nocscd_port;	
		struct {
			unsigned char targetDev;
			unsigned char targetPort;		
		}cscd_port;
		
};

enum uplink_type_s{
	ETH_PORT_CSCD_FOR_PROMISCUOUS = 0,
	ETH_PORT_NOCSCD_FOR_PROMISCUOUS
};


/*
*
*	switch mode struct
*
*/
struct eth_port_switch_s{
	int isSwitch;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_switch_port_notifier_func;
};

struct pve_target_s {
	unsigned char isTrunk;
	union{
		unsigned int trunkId;
		unsigned int eth_g_index;
	} dest;
};

enum pve_mode_e {
	PVE_MODE_SINGLE_E = 0,
	PVE_MODE_MULTI_E
};

/*
 *	eth_port_pve_s
 *
 * pvlan info
 */
 struct eth_port_pve_s{
	unsigned short vid;
	struct pve_target_s trg;
};

/*
 * 	vlan id list port add to as tagged port
 *
 */
typedef struct npd_common_list_s	dot1q_vlan_list_s;

struct eth_port_qinq_list_s {
        unsigned short vid;
	struct list_head list;
};

struct eth_port_dot1q_list_s {
	unsigned short vid;
	struct list_head list;
	dot1q_vlan_list_s subifList;
};

/*
 *	vlan id port add to as untagged port
 *
 */
struct port_based_vlan_s {
	unsigned short vid;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_vlan_untagports_notifier_func;
};

struct pve_info_s{
	enum uplink_type_s type;
	union uplink_port up;	
};

/*
 *	trunk id port add to a port
 *
 */
struct port_based_trunk_s {
	unsigned short trunkId;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_trunk_ports_notifier_func;
};

/*
 * 	igmp snoop notifer list apppend on port 
 *
 */

struct port_based_igmpsnp_s {
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_igmpsnp_ports_notifier_func;
};


/*
*
* promiscuous mode struct
* nocscd_port associate with GEports and XGEports
*
*/
struct eth_port_promi_s{
	enum uplink_type_s type;
	union uplink_port up;
	dot1q_vlan_list_s subifList;
	NPD_ETH_PORT_LINK_CHANGE_NOTIFIER_FUNC npd_promi_port_notifier_func;
	unsigned char uniMac[6];/* l3 interface unique mac, only used when different from system mac (default FF:FF:FF:FF:FF:FF)*/
};

#if 0
/*
 *
 *  Ethernet port driver info shadow.
 *
 */
struct eth_port_drv_info_s {
	unsigned int	pvid;				/* Port VID */
	boolean			bypassBrgEn;		/* by-passed bridge enable */
};
#endif

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

/**********************************************************************************************/
/*																										   */
/*										TRUNK INFO     													   */
/*																										   */
/**********************************************************************************************/

/*
  *
  * we can have 127 trunks at most
  * while trunk ID 0 means this is not a trunk
  * trunk ID 1~127 are legal value for a trunk
  *
  * each trunk support 8 port members at most
  *
  */
#define  NPD_TRUNK_NUMBER_MAX		127
#define  NPD_TRUNKID_NOT_TRUNK	0
#define  NPD_TRUNKID_START		1
#define  NPD_TRUNKID_END			127

#define  NPD_TRUNK_PORT_NUMBER_PER	8

/*******************************************************************************
 * typedef: enumerator NPD_TRUNK_ACTION_ENT
 *
 * Description:
 *      enumerator that hold values for the type of action currently done by the
 *      trunk lib
 *
 * Fields:
 *      NPD_TRUNK_ACTION_NONE_E - 	no current action
 *      NPD_TRUNK_ACTION_ADD_E - 		the member is added to the trunk
 *      NPD_TRUNK_ACTION_ADD_AS_DISABLED_E - add member as disabled
 *      NPD_TRUNK_ACTION_REMOVE_E - 	the member is removed from the trunk
 *      NPD_TRUNK_ACTION_ENABLE_E - 	the member is enabled in the trunk
 *      NPD_TRUNK_ACTION_DISABLE_E - 	the member is disabled in the trunk
 *      NPD_TRUNK_ACTION_NON_TRUNK_TABLE_ADD_E - the member is added only
 *                                      to the non-trunk table
 *      NPD_TRUNK_ACTION_NON_TRUNK_TABLE_REMOVE_E - the member is removed
 *                                      only from the non-trunk table
 *      NPD_TRUNK_ACTION_INITIALIZATION_E - the trunk lib initializing
 *      NPD_TRUNK_ACTION_RE_CALC_DESIGNATED_TABLE_E -re-calc the designated
 *                                      trunk tables
 *      NPD_TRUNK_ACTION_SET_TRUNK_E - set entire trunk entry ,
 *                                          from empty trunk
 *      NPD_TRUNK_ACTION_SET_KEEP_THE_CURRENT_TRUNK_E - set entire trunk
 *                                          entry , with the same configuration
 *                                          as already exists in HW
 *      NPD_TRUNK_ACTION_CLEAR_TRUNK_E - set trunk with no members
 *
 * Comments:
 ******************************************************************************/
enum NPD_TRUNK_ACTION_ENT {
    NPD_TRUNK_ACTION_NONE_E,
    NPD_TRUNK_ACTION_ADD_E,
    NPD_TRUNK_ACTION_ADD_AS_DISABLED_E,
    NPD_TRUNK_ACTION_REMOVE_E,
    NPD_TRUNK_ACTION_ENABLE_E,
    NPD_TRUNK_ACTION_DISABLE_E,
    NPD_TRUNK_ACTION_NON_TRUNK_TABLE_ADD_E,
    NPD_TRUNK_ACTION_NON_TRUNK_TABLE_REMOVE_E,
    NPD_TRUNK_ACTION_INITIALIZATION_E,
    NPD_TRUNK_ACTION_RE_CALC_DESIGNATED_TABLE_E,

    NPD_TRUNK_ACTION_SET_TRUNK_E,
    NPD_TRUNK_ACTION_SET_KEEP_THE_CURRENT_TRUNK_E,
    NPD_TRUNK_ACTION_CLEAR_TRUNK_E
};

/*
 * system support eight ports for each trunk at most
 */
#define NPD_TRUNK_IFNAME_SIZE 0x15
struct trunk_port_bmp {
	unsigned int 	mbrBmp0;
	unsigned int    mbrBmp1;
};

struct trunk_port_map {
	unsigned char master;		
	unsigned char enable;	
	unsigned char slot;
	unsigned char port;
};

struct dst_trunk_port_map{
	unsigned char devnum;
	unsigned char virportnum;
};

/*
 * Trunk port list
 *
 */
typedef struct npd_common_list_s trunk_port_list_s;

struct	trunk_port_list_node_s {
	struct	list_head	list;
	unsigned int 	eth_global_index; 
	unsigned char 	masterPortFlag;
	unsigned char 	enDisFlag;
};
/*
 * Trunk vlan list
 * 
 */

typedef struct npd_common_list_s trunk_vlan_list_s;

struct trunk_vlan_list_node_s {
	struct list_head	list;
	unsigned short		vlanId;
	unsigned char		uTagM;
};
/*
 *
 * Trunk Main Data Structure.
 *
 */
struct trunk_s {
	unsigned short	trunkId;
	unsigned char	trunkName[NPD_TRUNK_IFNAME_SIZE];
	unsigned char 	tLinkState;	/*TRUNK_STATE_UP_E,TRUNK_STATE_DOWN_E*/
	unsigned char 	portLog;	/* log the port_number in trunk */
	struct trunk_port_map protMap[8];
	struct trunk_port_bmp portsMbrBmp;
	trunk_port_list_s	*portList;
	trunk_vlan_list_s	*allowVlanList;
	unsigned char	loadBanlcMode;
	/*struct trunk_drv_info_s		shadow;*/
};

typedef struct distributed_trunk_struct {
	unsigned short trunkId;
	unsigned char trunkName[NPD_TRUNK_IFNAME_SIZE];
	unsigned char tLinkState;
	unsigned char portLog;
	struct trunk_port_map portmap[8];
	struct dst_trunk_port_map dst_port_map[8];
	unsigned char loadBanlcMode;
}dst_trunk_s;

/**********************************************************************************************/
/*																										   */
/*										VLAN INFO     													   */
/*																										   */
/**********************************************************************************************/
#define NPD_VLAN_NUMBER_MAX			(4 * _1K)
#define NPD_VLAN_IFNAME_SIZE			21
#define NPD_VLAN_BITMAP_LENGTH		64	/*port index bitmap row count*/
#define NPD_VLAN_DRV_BITMAP_LENGTH	2	/*driver port bitmap row count*/
#define NPD_VLAN_BITMAP_WIDTH			32	/*bitmap column count*/
#define VLAN_CONFIG_SUCCESS 			0
#define VLAN_CONFIG_FAIL				0xff	
#define BRG_MC_SUCCESS					0
#define BRG_MC_FAIL						0xff
enum vlan_type_e {
	BRG_VLAN_PORT_BASED_E = 0,
	BRG_VLAN_PROTOCOL_BASED_E,
	BRG_VLAN_PORT_PROTOCOL_BASED_E,
	BRG_VLAN_TYPE_MAX
};

enum PACKET_CMD_ENT {
    PACKET_CMD_FORWARD_E          , /* 00 */
    PACKET_CMD_MIRROR_TO_CPU_E    , /* 01 */
    PACKET_CMD_TRAP_TO_CPU_E      , /* 02 */
    PACKET_CMD_DROP_HARD_E        , /* 03 */
    PACKET_CMD_DROP_SOFT_E        , /* 04 */
    PACKET_CMD_ROUTE_E            , /* 05 */
    PACKET_CMD_ROUTE_AND_MIRROR_E , /* 06 */
    PACKET_CMD_BRIDGE_AND_MIRROR_E, /* 07 */
    PACKET_CMD_BRIDGE_E           , /* 08 */
    PACKET_CMD_NONE_E             , /* 09 */
    PACKET_CMD_INVALID_E            /* 10 */
} ;

enum BRG_IP_CTRL_TYPE_ENT {
    BRG_IP_CTRL_NONE_E,
    BRG_IP_CTRL_IPV4_E,
    BRG_IP_CTRL_IPV6_E,
    BRG_IP_CTRL_IPV4_IPV6_E
};

enum BRG_IPM_MODE_ENT {
    BRG_IPM_SGV_E,
    BRG_IPM_GV_E
};

enum IP_SITE_ID_ENT {
    IP_SITE_ID_INTERNAL_E,
    IP_SITE_ID_EXTERNAL_E
};

struct vlan_ports_bmp {
    unsigned int   bitmaps[NPD_VLAN_BITMAP_LENGTH];
};

/*
 *
 * Bridge VLAN info
 *
 */
struct	 brg_vlan_info_s {
	unsigned int	state; /* vlan state: up or down*/
	unsigned int                fdbLimit; /* vlan-based FDB protected number */
	unsigned int				fdbCount; /* FDB learned on this vlan */
	struct vlan_ports_bmp	untaggedPorts;	/* untagged port bitmap */ /*eth_global_index list !!!*/
	struct vlan_ports_bmp	taggedPorts;		/* tagged port bitmap */ /*eth_global_index list !!!*/
};

/*
 *
 * VLAN interface 
 *
 */
struct vlan_intf_info_s {
	unsigned int 	ifIndex;     				/*vlan Id*/
	enum VLAN_PORT_SUBIF_FLAG flag;
	unsigned char uniMac[6];/* l3 interface unique mac, only used when different from system mac (default FF:FF:FF:FF:FF:FF)*/
	unsigned char state; /* l3 interface state (0 - down, 1 - up)*/
};

/*
 *	VLAN list
 *
 */
typedef struct npd_common_list_s vlan_list_s;

struct vlan_list_node_s	{
	struct list_head	list;
	unsigned short		vid;
};
/*
 *
 * VLAN stack (also named as Q-in-Q)
 *
 */
struct vlan_stack_info_s {
	struct vlan_s	*primary;			/* if this is secondary vlan, pointer to primary vlan */
	struct vlan_list_s	*secondary;	/* if this is primary vlan, pointer to secondary vlan list */ /*vid list*/
};

/*
 *  Ethernet port list
 *
 */
typedef struct npd_common_list_s vlan_port_list_s;

typedef struct NPD_DHCP_SNP_PORT_S
{
	unsigned char trust_mode;			/* DHCP-Snooping trust mode of port				*/
	unsigned char opt82_strategy;		/* DHCP-Snooping option82 strategy type of port	*/
	unsigned char opt82_circuitid;		/* DHCP-Snooping option82 circuitid type of port	*/
										/* DHCP-Snooping option82 circuitid content of port */
	unsigned char opt82_circuitid_str[64];
	unsigned char opt82_remoteid;		/* DHCP-Snooping option82 remoteid type of port	*/
										/* DHCP-Snooping option82 remoteid content of port */
	unsigned char opt82_remoteid_str[64];
}NPD_DHCP_SNP_PORT_T;

struct vlan_port_list_node_s {
	struct list_head 	list;
	unsigned int		eth_global_index;
	unsigned char 		igmpEnFlag;
	unsigned int        fdbLimit;
	unsigned int        fdbCount;
	unsigned char		igmp_rport_flag;
	unsigned char		dldpEnFlag;
	NPD_DHCP_SNP_PORT_T dhcp_snp_info;
};

/*
 *
 * Trunk list
 *
 */
typedef struct npd_common_list_s vlan_trunk_list_s;
 
struct vlan_trunk_list_node_s {
	struct list_head 	list;
	unsigned short		trunkId;
};

#if 0
/*
 * IGMP McFDB entry  
 */

typedef struct npd_common_list_s igmp_fdb_list_s;

struct igmp_fdb_list_entry_s{
	struct list_head  list;
	unsigned int groupIp;
	unsigned short vidx;
};
#endif
struct vlan_acl_info_s {
	unsigned int   IsEnable;
	unsigned int   IsEgrEnable;
	unsigned int   groupId;
	unsigned int   EgrGroupId;
};

/*
 * VLAN info main structure.
 */
struct vlan_s {
	unsigned short 				vid;		/* VLAN id */
	char						vlanName[NPD_VLAN_IFNAME_SIZE];	/* VLAN name */
	unsigned char				igmpSnpEnDis;/*VLAN IGMP snooping enable Flag*/
	struct	brg_vlan_info_s		brgVlan;	/* VLAN layer 2 info */
	struct vlan_intf_info_s		intfVlan;	/* VLAN layer 3 info */
	struct vlan_stack_info_s 	qinq;		/* VLAN Q-in-Q info */
	vlan_port_list_s 	*untagPortList;	/* untagged port list */ /*eth_global_index list*/
	vlan_port_list_s 	*tagPortList;	/* tagged port list */ /*eth_global_index list*/
	vlan_trunk_list_s 	*tagTrunkList;		/* trunk list */ /*trunk group id list*/
	vlan_trunk_list_s 	*unTagtrunkList;		/* untag trunk list */ 
	igmp_snooping_list_s *igmpList;		/* IGMP snooping control list */
	struct dhcp_snooping_list_s	 *dhcpList;		/* DHCP snooping control list */
	struct vlan_acl_info_s		 *aclList;			/*ACL control list*/
	unsigned char dldpEnDis;				/*VLAN DLDP enable Flag*/
	unsigned char dhcpSnpEnDis;				/*VLAN dhcp snooping enable Flag*/
	unsigned int isAutoCreated;				/* whether the vlan is auto created flag */
	/*	qos_policy_list_s	 	     *policyList;	*/	/* QoS Policy control list */		
};


#define NPD_VLAN_MTU_VALUE  1522

typedef enum {
	CPU_FC_TYPE_ARP_E =0,
	CPU_FC_TYPE_BPDU_E,
	CPU_FC_TYPE_DHCP_E,
	CPU_FC_TYPE_ICMP_E,
	CPU_FC_TYPE_IGMP_E, 
	CPU_FC_TYPE_OSPF_E,
	CPU_FC_TYPE_RIPv1_E,
	CPU_FC_TYPE_RIPv2_E,
	CPU_FC_TYPE_OAM_E, /* OAM type is collection of (telnet,ssh,http,https,snmp) */
	CPU_FC_TYPE_TELNET_E,
	CPU_FC_TYPE_SSH_E,
	CPU_FC_TYPE_HTTP_E,
	CPU_FC_TYPE_HTTPS_E,
	CPU_FC_TYPE_SNMP_E,
	CPU_FC_TYPE_WIRELESS_E, /* WIRELESS type is collection of CAPWAP control and service data type */
	CPU_FC_TYPE_CAPWAP_CTRL_E,
	CPU_FC_TYPE_CAPWAP_DATA_E,
	CPU_FC_TYPE_ALL_E, /* ALL type is collection of types all above */
	CPU_FC_TYPE_MAX_E
}CPU_FC_PACKET_TYPE;
typedef enum
{
    BRG_DROP_CNTR_COUNT_ALL_E,                    /* 0  */
    BRG_DROP_CNTR_FDB_ENTRY_CMD_E,                /* 1  */
    BRG_DROP_CNTR_UNKNOWN_MAC_SA_E,               /* 2 */
    BRG_DROP_CNTR_INVALID_SA_E,                   /* 3  */
    BRG_DROP_CNTR_INVALID_VLAN_E,                 /* 4  */
    BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E,             /* 5  */
    BRG_DROP_CNTR_VLAN_RANGE_E,                   /* 6  */
    BRG_DROP_CNTR_MOVED_STATIC_ADDR_E,            /* 7  */

    /**************************************************************/
    /* List of Drop Modes not supported by the Dx Ch device       */
    BRG_DROP_CNTR_ARP_SA_MISMATCH_E,              /* 8  */
    BRG_DROP_CNTR_VLAN_MRU_E = 20,                  /* 20*/
	BRG_DROP_CNTR_RATE_LIMIT_E = 21,				  /* 21 */
    BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E =23            /* 23 */  
} BRIDGE_DROP_CNTR_MODE_ENT;                
             

/*
 * typedef: enum CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT
 *
 *
 *	Mirror mode
 *	base of port
 *	base of vlan
 *	base of fdb
 *	base of policy
*/
typedef enum {
	MIRROR_INGRESS_E = 0,
	MIRROR_EGRESS_E,
	MIRROR_BIDIRECTION_E,
	MIRROR_DIRECT_DEL_E
}MIRROR_DIRECTION_TYPE;

struct mirror_struct_s {
	/* mirror destination port index for ingress direction */
	unsigned int in_eth_index;

	/* mirror destination port index for egrss direction */
	unsigned int eg_eth_index;

	/*index base of type*/
	void* func[MIRROR_FUNC_MAX];
};

typedef struct npd_common_list_s mirror_src_port_list_s;

typedef struct npd_common_list_s mirror_src_vlan_list_s;

typedef struct npd_common_list_s mirror_src_fdb_list_s;

typedef struct npd_common_list_s mirror_src_policy_list_s;

struct mirror_src_port_node_s {
	unsigned int eth_g_index;
	MIRROR_DIRECTION_TYPE direct;
	struct list_head list; 
};

struct mirror_src_vlan_node_s {
	unsigned short vid;
	struct list_head list; 
};

struct mirror_src_fdb_node_s {
	unsigned short vid;
	unsigned char mac[MAC_ADDRESS_LEN];
	unsigned char slot_no;
	unsigned char port_no;
	struct list_head list; 
};

struct mirror_src_policy_node_s {
	/*sulong add*/
	unsigned int 	  ruleIndex;
	struct list_head  list; 
};


/* 
 *	OAM controller structure
*/
struct oam_ctrl_s {
	unsigned int 	dport; /* user-defined tcp/udp dport for OAM control traffic */
	unsigned char	enable; /* enable/disable OAM traffic priority promotion */
	unsigned int	hwTblIdx; /* hw table index */
};

/*
 * 	CPU port dma channel mib counter
 */
typedef struct __cpu_port_dma_mib_s {
	unsigned int rxInPkts;	/* Rx packet count */
	unsigned int rxInOctets; /* Rx byte count */
	unsigned int rxErrCnt; /* Rx resource error count */
}CPU_PORT_DMA_CHANNEL_MIB_S;

/*
 *	CPU port (used as Ethernet MAC ) mib counter
 */
typedef struct __cpu_port_mac_mib_s{
	struct {
		unsigned int goodPkts[2]; /* GoodFramesReceived */
		unsigned int badPkts[2]; /* BadFramesReceived */
		unsigned int goodOctets[2]; /* GoodOctetsReceived */
		unsigned int badOctets[2]; /* BadOctetsReceived */
		unsigned int internalDrop[2]; /* Rx internal drop */
	}rx;
	struct {
		unsigned int goodPkts[2]; /* GoodFramesSent */
		unsigned int goodOctets[2]; /* GoodOctetsSent */
		unsigned int macErrPkts[2]; /* MACTransErrorFramesSent */
		
	}tx;
}CPU_PORT_MAC_MIB_S;


/*
 *	port (used as Ethernet MAC ) mib counter
 */
typedef struct __asic_port_mac_mib_s{
	struct {
		unsigned int goodPkts[2]; /* GoodFramesReceived */
		unsigned int badPkts[2]; /* BadFramesReceived */
		unsigned int goodOctets[2]; /* GoodOctetsReceived */
		unsigned int badOctets[2]; /* BadOctetsReceived */
		unsigned int internalDrop[2]; /* Rx internal drop */		
		unsigned int goodBCpkts[2]; /* broadcast  frames received */
		unsigned int goodUCpkts[2]; /* Unicast frames received */
		
	}rx;
	struct {
		unsigned int goodPkts[2]; /* GoodFramesSent */
		unsigned int goodOctets[2]; /* GoodOctetsSent */
		unsigned int macErrPkts[2]; /* MACTransErrorFramesSent */
		unsigned int goodBCpkts[2]; /* broadcast  frames send */
		unsigned int goodUCpkts[2]; /* Unicast frames send */		
	}tx;
}ASIC_PORT_MAC_MIB_S;



/*
 * port mib counter for diag, zhangdi@autelan.com 2012-04-24
 * typedef from struct CPSS_PORT_MAC_COUNTER_SET_STC
 *
 * Description: Counters for BULK API  of the ethernet MAC Counters.
 *              The actual counter size (32 or 64 bits counters) is according
 *              the hardware device, refer to data sheet for more info.
 *
 * Fields:
 *  goodOctetsRcv           - Number of ethernet frames received that are
 *                            not bad ethernet frames or MAC Control pkts.
 *                            This includes Bridge Control packets (LCAP, BPDU)
 *  badOctetsRcv            - Sum of lengths of all bad ethernet frames received
 *  goodPktsRcv             - Number of ethernet frames received that are
 *                            neither bad ethernet frames nor MAC Control pkts.
 *                            This includes Bridge Control packets (LCAP, BPDU)
 *                            Not Supported for DxCh and ExMxPm.
 *  badPktsRcv              - Number of corrupted ethernet frames received
 *                            Not Supported for DxCh and ExMxPm.
 *  brdcPktsRcv             - Total number of undamaged packets received that
 *                            were directed to the broadcast address
 *  mcPktsRcv               - Total number of undamaged packets received that
 *                            were directed to a multicast address
 *  goodPktsSent            - Number of ethernet frames sent from this MAC.
 *                            This does not include 802.3 Flow Control packets,
 *                            packets dropped due to excessive collision or
 *                            packets with a Tx Error.
 *                            Not Supported for DxCh and ExMxPm.
 *  goodOctetsSent          - Sum of lengths of all good ethernet frames sent
 *                            from this MAC.
 *                            This does not include 802.3 Flow Control packets,
 *                            packets dropped due to excessive collision or
 *                            packets with a Tx Error.
 *  brdcPktsSent            - Total number of good packets sent that have a
 *                            broadcast destination MAC address.
 *                            This does not include 802.3 Flow Control packets,
 *                            packets dropped due to excessive collision or
 *                            packets with a Tx Error.
 *  mcPktsSent              - Total number of good packets sent that have a
 *                            multicast destination MAC address.
 *                            This does not include 802.3 Flow Control packets,
 *                            packets dropped due to excessive collision or
 *                            packets with a Tx Error.
 *  pkts64Octets            - Total number of received and transmitted undamaged
 *                            and damaged frames which are 64 bytes in size.
 *                            This does not include MAC Control Frames.
 *  pkts65to127Octets       - Total number of received and transmitted undamaged
 *                            and damaged frames which are 65 to 127 bytes in
 *                            size. This does not include MAC Control Frames.
 *  pkts128to255Octets      - Total number of received and transmitted undamaged
 *                            and damaged frames which are 128 to 255 bytes in
 *                            size. This does not include MAC Control Frames.
 *  pkts256to511Octets      - Total number of received and transmitted undamaged
 *                            and damaged frames which are 256 to 511 bytes in
 *                            size. This does not include MAC Control Frames.
 *  pkts512to1023Octets     - Total number of received and transmitted undamaged
 *                            and damaged frames which are 512 to 1023 bytes in
 *                            size. This does not include MAC Control Frames.
 *  pkts1024tomaxOoctets    - Total number of received and transmitted undamaged
 *                            and damaged frames which are more than 1024 bytes
 *                            in size. This does not include MAC Control Frames.
 *                            (xCat2: the behavior of this counter is determined by
 *                            oversized packets counter mode.
 *                            See: cpssDxChMacOversizedPacketsCounterModeSet.) 
 *  collisions              - Total number of collisions seen by the MAC
 *  lateCollisions          - Total number of late collisions seen by the MAC
 *                            (xCat2: also counts unknown MAC control frames)
 *  excessiveCollisions     - Number of frames dropped in the transmit MAC due
 *                            to excessive collisions. This is an applicable for
 *                            Half-Duplex mode only.
 *  macRcvError             - Number of Rx Error events seen by the receive side
 *                            of the MAC
 *  macTransmitErr          - Number of frames not transmitted correctly or
 *                            dropped due to internal MAC Tx error
 *  badCrc                  - Number of CRC error events.
 *  dropEvents              - Number of instances that the port was unable to
 *                            receive packets due to insufficient bandwidth to
 *                            one of the PP internal resources, such as the DRAM
 *                            or buffer allocation.
 *  undersizePkts           - Number of undersize packets received.
 *  fragmentsPkts           - Number of fragments received.
 *  oversizePkts            - Number of oversize packets received.
 *  jabberPkts              - Number of jabber packets received.
 *  unrecogMacCntrRcv       - Number of received MAC Control frames that have an
 *                            opcode different from 00-01.
 *                            Not Supported for DxCh and ExMxPm.
 *  goodFcRcv               - Number of good Flow Control frames received
 *  fcSent                  - Number of Flow Control frames sent.
 *  badFcRcv                - Number of bad Flow Control frames received.
 *                            Not Supported for DxCh and ExMxPm.
 *  ucPktsRcv               - Number of Ethernet Unicast frames received that
 *                            are not bad Ethernet frames or MAC Control packets.
 *                            Note that this number includes Bridge Control
 *                            packets such as LCAP and BPDU.
 *                            Supported for DxCh and ExMxPm only.
 *  ucPktsSent              - Number of good frames sent that had a Unicast
 *                            destination MAC Address.
 *                            Supported for DxCh and ExMxPm only.
 *  multiplePktsSent        - Valid Frame transmitted on half-duplex link that
 *                            encountered more then one collision.
 *                            Byte count and cast are valid.
 *                            Supported for DxCh and ExMxPm only.
 *  deferredPktsSent       - Valid frame transmitted on half-duplex link with
 *                            no collisions, but where the frame transmission
 *                            was delayed due to media being busy. Byte count
 *                            and cast are valid.
 *                            Supported for DxCh and ExMxPm only.
 *                            (xCat2: the behavior of this counter is determined by
 *                            oversized packets counter mode.
 *                            See: cpssDxChMacOversizedPacketsCounterModeSet.)
 *
 */

typedef struct __asic_port_mac_mib_diag_s{
    unsigned int goodOctetsRcv[2];
    unsigned int badOctetsRcv[2];
    unsigned int macTransmitErr[2];
    unsigned int goodPktsRcv[2];
    unsigned int badPktsRcv[2];
    unsigned int brdcPktsRcv[2];
    unsigned int mcPktsRcv[2];
    unsigned int pkts64Octets[2];
    unsigned int pkts65to127Octets[2];
    unsigned int pkts128to255Octets[2];
    unsigned int pkts256to511Octets[2];
    unsigned int pkts512to1023Octets[2];
    unsigned int pkts1024tomaxOoctets[2];
    unsigned int goodOctetsSent[2];
    unsigned int goodPktsSent[2];
    unsigned int excessiveCollisions[2];
    unsigned int mcPktsSent[2];
    unsigned int brdcPktsSent[2];
    unsigned int unrecogMacCntrRcv[2];
    unsigned int fcSent[2];
    unsigned int goodFcRcv[2];
    unsigned int dropEvents[2];
    unsigned int undersizePkts[2];
    unsigned int fragmentsPkts[2];
    unsigned int oversizePkts[2];
    unsigned int jabberPkts[2];
    unsigned int macRcvError[2];
    unsigned int badCrc[2];
    unsigned int collisions[2];
    unsigned int lateCollisions[2];
    unsigned int badFcRcv[2];
    unsigned int ucPktsRcv[2];
    unsigned int ucPktsSent[2];
    unsigned int multiplePktsSent[2];
    unsigned int deferredPktsSent[2];
} ASIC_PORT_MAC_MIB_DIAG_S;


/*
  * 	CPU port all mib counter 
  */
typedef union __cpu_port_all_mib_s {
	/* CPU port mib when using the SDMA interface */
	CPU_PORT_DMA_CHANNEL_MIB_S dmaMib[NPD_ENGINE_CPU_CHANNEL_NUM]; /* dma channel Rx/Tx mib */
	/* CPU port mib when using "Ethernet CPU port" */
	CPU_PORT_MAC_MIB_S etherMacMib; /* Ethernet MAC mib */
}CPU_PORT_MIB_INFO_S;

/* add for tunnel*/
typedef struct tunnel_host_s {
	unsigned int hostdip;
	unsigned int hdiplen;
	struct list_head   list;
}tunnel_host_t;

typedef enum {
	TUNNEL_STATES_START = 0x0,
	TUNNEL_TS_SW_EN = 0x01,
	TUNNEL_TT_SW_EN = 0x02,
	TUNNEL_NH_SW_EN = 0x04,
	TUNNEL_RT_SW_EN = 0x08,
	TUNNEL_TS_HW_EN = 0x10,
	TUNNEL_TT_HW_EN = 0x20,
	TUNNEL_NH_HW_EN = 0x40,
	TUNNEL_RT_HW_EN = 0x80
}TUNNEL_STATES;

typedef struct tunnel_action_s {
	unsigned long		istunnelstart;
	unsigned int		tunnelstartidx;
}tunnel_action_t;

typedef struct tunnel_kernel_msg_s {
	unsigned char 	mac[6];
	unsigned int		srcip;
	unsigned int		dstip;
	unsigned short	vid;
	unsigned char 	portnum;
	unsigned char 	devnum;
}tunnel_kernel_msg_t;

typedef struct tunnel_item_s {
	TUNNEL_STATES		state;
	unsigned char			sysmac[6];
	unsigned int			istuact; 
	unsigned int 			tsindex;/* init 0 because arp init  use 0 1 2*/
	unsigned int 			ttindex;/* init 0 tt start at 895*/
	unsigned int			nhindex;
	unsigned int			hostnum;
	tunnel_kernel_msg_t	kmsg;
	tunnel_action_t		tunact;
	struct list_head 		list1;
}tunnel_item_t;

enum TUNNEL_DB_ACTION {
	TUNNEL_ADD_ITEM = 0,
	TUNNEL_DEL_ITEM,
	TUNNEL_UPDATE_ITEM,
	TUNNEL_OCCUPY_ITEM,
	TUNNEL_RESET_ITEM,
	TUNNEL_ACION_MAX
};

#define WIFI_MAC_LEN 6
#define WIFI_NL_MAX_PAYLOAD 1024

typedef enum {
	EXT_IP = 0,
	INNER_IP = 1
} msgType;

typedef enum {
	IP_ADD = 0,
	IP_DEL = 1
} msgOp;

struct extIPInfo {
	unsigned int dip;
	unsigned int sip;
	unsigned char WTPMAC[WIFI_MAC_LEN];
};

struct innerIPInfo {
	unsigned int inner_IP;
	unsigned int ext_dip;
	unsigned int ext_sip;
};

/* Netlink message format between wifi and npd */
struct wifi_nl_msg {
	msgType type;
	msgOp op;
	union {
		struct extIPInfo extMsg;
		struct innerIPInfo innerMsg;
	} u;
};

#define TUNNEL_TERM_FRIST_NUM					896
#define TUNNEL_TERM_LAST_NUM					1023
#define TUNNEL_MAX_SIZE						128 /*hash table max len*/
#define TUNNEL_TABLE_SIZE						1024 /*hash table max storage*/

/* add for tunnel*/

/*
  *	Defined for watchdog control
  */
#define SYSTEM_HARDWARE_WATCHDOG_ENABLE		1
#define SYSTEM_HARDWARE_WATCHDOG_DISABLE	0
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_SET		1
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_OP_GET		0
#define SYSTEM_HARDWARE_WATCHDOG_TIMEOUT_DEFAULT	(0x1F) /* 0x1F is TIME_UNIT, while TIME_UNIT is 1~1.6s by default */



#define BOARD_GLOBAL_ETHPORTS_MAXNUM 64
#define GLOBAL_ETHPORTS_MAXNUM       (64*16)

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

#endif

