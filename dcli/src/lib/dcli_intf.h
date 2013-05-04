#ifndef __DCLI_INTF_H__
#define __DCLI_INTF_H__

#define DCLI_SUCCESS 0
#define DCLI_CREATE_ERROR 		1
#define DCLI_INTF_CHECK_IP_ERR  4
#define DCLI_INTF_CHECK_MAC_ERR		5
#define DCLI_VLAN_BADPARAM			12  
#define DCLI_VLAN_NOTEXISTS			15

#define DCLI_INTF_DIS_ROUTING_ERR 19
#define DCLI_INTF_EN_ROUTING_ERR 20
#define DCLI_INTF_EXISTED 21
#define DCLI_INTF_NOTEXISTED 22
#define DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF 23
#define DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF 24
#define DCLI_ONLY_RUN_IN_VLAN 25
#define DCLI_ALREADY_ADVANCED 26
#define DCLI_NOT_ADVANCED 27
#define DCLI_PARENT_INTF_NOT_EXSIT 28
#define DCLI_PROMI_SUBIF_EXIST 29
#define DCLI_PROMI_SUBIF_NOTEXIST 30
#define DCLI_DBUS_PORT_NOT_IN_VLAN 31
#define DCLI_MAC_MATCHED_BASE_MAC 32
#define DCLI_L3_INTF_NOT_ACTIVE 33
#define DCLI_INTF_NO_HAVE_ANY_IP 34
#define DCLI_INTF_HAVE_THE_IP 35
#define DCLI_INTF_NOT_SAME_SUB_NET 36
#define DCLI_INTF_STATUS_CHECK_ERR 37
#define DCLI_INTF_GET_SYSMAC_ERR   38
#define DCLI_INTF_VLAN_CONTAIN_PROMI_PORT  39
#define DCLI_SUBIF_CREATE_FAILED  40
#define DCLI_SUBIF_ADD_PORT_TO_VLAN_FAILED  41
#define DCLI_ARP_SNOOPING_PORT_ADVANCED_ROUTING 42
#define DCLI_ARP_SNOOPING_ADVANCED_ROUTING    43
#define DCLI_INTERFACE_TRUNK_NOT_EXISTS     44
#define DCLI_INTERFACE_TRUNK_NOT_IN_VLAN    45
#define DCLI_INTERFACE_MAC_MATCHED_INTERFACE_MAC  46
#define DCLI_INTERFACE_PORT_NEED_VLAN 			47
#define DCLI_INTERFACE_ARP_DEL_NAM_OP_ERROR   101




#define DCLI_ARP_SNOOPING_ERR_NONE		0
#define DCLI_ARP_SNOOPING_ERR_STATIC_EXIST (DCLI_ARP_SNOOPING_ERR_NONE + 8)
#define DCLI_ARP_SNOOPING_ERR_STATIC_NOTEXIST	(DCLI_ARP_SNOOPING_ERR_NONE + 9)
#define DCLI_ARP_SNOOPING_ERR_PORT_NOTMATCH     (DCLI_ARP_SNOOPING_ERR_NONE + 10)
#define DCLI_ARP_SNOOPING_ERR_KERN_CREATE_FAILED (DCLI_ARP_SNOOPING_ERR_NONE + 11)
#define DCLI_ARP_SNOOPING_ERR_STATIC_ARP_FULL    (DCLI_ARP_SNOOPING_ERR_NONE + 13)  /* static arp is full (1k)*/
#define DCLI_ARP_SNOOPING_ERR_HASH_OP_FAILED     (DCLI_ARP_SNOOPING_ERR_NONE + 14)  /* hash push or pull failed*/

#define DCLI_ARP_SNOOPING_ERR_MAX			(DCLI_ARP_SNOOPING_ERR_NONE + 255)

#define MIN_VLANID 1
#define MAX_VLANID 4094
#define MAX_L3INTF_VLANID 4095
#define MIN_BONDID       0
#define MAX_BONDID       7
#define MAXLEN_BOND_CMD  128
#define MAXLEN_BOND_NAME 5   /*bond0~bond7*/

#define IP_MASK_CHECK(num) 					\
if((num<0)||(num>32)) 		\
 {													\
		vty_out(vty,"%% Illegal ip mask value!\n");    \
		return CMD_WARNING;							\
 }


#define DCLI_SET_FDB_ERR 0xff

#ifdef _D_WCPSS_  /*zhanglei add*/
#endif

#define INTERFACE_NAMSIZ      20
#ifdef _D_WCPSS_  /*zhanglei add*/
#endif

#define CONFIG_INTF_ETH  0
#define CONFIG_INTF_E    1
#define CONFIG_INTF_CSCD 2
#define CONFIG_INTF_OBC  3
#define CONFIG_INTF_VE   4
#define CONFIG_INTF_MNG  5

int create_l3intf_by_port_index(unsigned char slot_no,unsigned char port_no);

int del_l3intf_by_port_index(unsigned char slot_no,unsigned char port_no);

char * dcli_error_info_intf(int errorCode);
int dcli_create_vlan_intf
(
	struct vty*	vty,
	unsigned short vid,
	unsigned int advanced
);
int dcli_create_vlan_intf_by_vlan_ifname
(
	struct vty*	vty,
	unsigned short vid
);
int dcli_vlan_interface_advanced_routing_enable
(
	struct vty*	vty,
	unsigned short vid,
	unsigned int enable
);

int dcli_advanced_routing_config

(
    struct vty * vty,
    char * ifName,
    unsigned int isEnable
);

int dcli_del_vlan_intf
(
    struct vty * vty,
    unsigned short vId
);
int dcli_create_eth_port_sub_intf
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned char cpu_no,
    unsigned char cpu_port_no,    
    unsigned int vid,
    unsigned int vid2
);
int dcli_del_eth_port_sub_intf
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned char cpu_no,
    unsigned char cpu_port_no,     
    unsigned int vid,
    unsigned int vid2
);

unsigned int parse_param_ifName
(
    char * ifName,
    unsigned char * slot_no,
    unsigned char * port_no,
    unsigned int  * vid
);

int dcli_intf_advanced_routing_show_running
(
	void
);
unsigned int dcli_intf_vlan_eth_port_interface_show_advanced_routing
(
	struct vty * vty,
	unsigned int flag,
	unsigned int slot_no,
	unsigned int port_no,
	unsigned int vid
);

int dcli_intf_show_advanced_routing
(
	struct vty * vty,
	unsigned int vlanAdv,
	unsigned int includeRgmii
);
void  dcli_qinq_type_init();

int dcli_interface_dbus_eth_interface_enable_set
	(
		struct vty * vty,
		unsigned char slot,
		unsigned char port,
		unsigned int tag1,
		unsigned int tag2,
		unsigned int enable
	);


#endif
