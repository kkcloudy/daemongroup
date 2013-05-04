#ifndef _IPFWD_LEARN_H
#define _IPFWD_LEARN_H

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/in.h>
#include <asm/unistd.h>

#include "ipfwd_learn_common.h"

/*****************************************************************************
*   Octeon Hardware related
*****************************************************************************/

#define POW_SEND_GROUP    2 
#define SE_MAGIC_MUN      0x12345678
#define CVMX_FPA_WQE_POOL			(1)
#define DONT_WRITEBACK(x)           (x) /* Use this to have all FPA frees also tell the L2 not to write data to memory */

#ifdef BUILDNO_VERSION2_1
#define CVM_WQE_GET_LEN(work)   cvmx_wqe_get_len((work))
#define CVM_WQE_SET_LEN(work, v)   cvmx_wqe_set_len((work), (v))
#define CVM_WQE_GET_QOS(work)   cvmx_wqe_get_qos((work))
#define CVM_WQE_SET_QOS(work, v)   cvmx_wqe_set_qos((work), (v))
#define CVM_WQE_GET_PORT(work)   cvmx_wqe_get_port((work))
#define CVM_WQE_SET_PORT(work, v)   cvmx_wqe_set_port((work), (v))
#define CVM_WQE_GET_TAG(work)   ((work)->word1.tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->word1.tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->word1.tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->word1.tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   cvmx_wqe_get_unused8((work))
#define CVM_WQE_SET_UNUSED(work, v)   cvmx_wqe_set_unused8((work), (v))
#define CVM_WQE_SET_GRP(work,v)    cvmx_wqe_set_grp((work),(v))
#define CVM_WQE_GET_GRP(work)  cvmx_wqe_get_grp((work))
#define CVMX_HELPER_TCP_TAG_TYPE CVMX_POW_TAG_TYPE_ATOMIC
#else
#define CVM_WQE_GET_LEN(work)   ((work)->len)
#define CVM_WQE_SET_LEN(work, v)   (work)->len = (v)
#define CVM_WQE_GET_QOS(work)   ((work)->qos)
#define CVM_WQE_SET_QOS(work, v)   ((work)->qos = (v))
#define CVM_WQE_GET_PORT(work)   ((work)->ipprt)
#define CVM_WQE_SET_PORT(work, v)   ((work)->ipprt = (v))
#define CVM_WQE_GET_TAG(work)   ((work)->tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   ((work)->unused)
#define CVM_WQE_SET_UNUSED(work, v)   ((work)->unused = (v))
#define CVM_WQE_SET_GRP(work,v)   ((work)->grp = (v))
#define CVM_WQE_GET_GRP(work)  ((work)->grp)
#endif


/*****************************************************************************
*   ACL Rule related
*****************************************************************************/

/* CAPWAP rule table structer */
typedef struct  capwap_cache_tbl_s
{
    uint32_t use_num;
    /* Extern IP header */
    uint32_t dip;
    uint32_t sip;
    uint16_t dport;
    uint16_t sport;
    uint8_t tos;	
	
    /* CAPWAP header */
    uint8_t cw_hd[CW_H_LEN];
} capwap_cache_t ;


/***************************************
 * 114 Bytes, 
 * ACL rule structer 
 ***************************************/
typedef struct  rule_param_s{

	/*L2_header information  34Bytes*/
	uint64_t dsa_info;
	/*rpa head*/
	struct rpa_header{
	    uint16_t rpa_vlan_type;
	    uint16_t rpa_vlan_tag;	
		uint8_t rpa_type;
		uint8_t rpa_dnetdevNum;
		uint8_t rpa_snetdevNum;
		uint8_t rpa_d_s_slotNum;
	}rpa_header;
	uint8_t  ether_dhost[6];
	uint8_t  ether_shost[6];
	uint16_t out_ether_type;
	uint16_t out_tag;
	uint16_t in_ether_type;
	uint16_t in_tag;
	uint16_t ether_type;

	/*the tunnel internal L2 hader*/
	union {
		/*802.11WIFI header  22Bytes*/
		struct wifi_header {
			uint8_t fc[2];
			uint8_t qos[2];
			uint8_t addr[18];
		} wifi_header;

		/*802.3 ethernet header  14Bytes*/	
		struct eth_header {
			uint8_t  dmac[6];
			uint8_t  smac[6];
			uint16_t ether_type;
		} eth_header;
	}tunnel_l2_header;
#define	acl_tunnel_wifi_header_fc		tunnel_l2_header.wifi_header.fc
#define	acl_tunnel_wifi_header_qos		tunnel_l2_header.wifi_header.qos
#define	acl_tunnel_wifi_header_addr		tunnel_l2_header.wifi_header.addr
#define	acl_tunnel_eth_header_dmac		tunnel_l2_header.eth_header.dmac
#define	acl_tunnel_eth_header_smac		tunnel_l2_header.eth_header.smac
#define	acl_tunnel_eth_header_ether		tunnel_l2_header.eth_header.ether_type


	/*L3-4 header, the HASH key  14Bytes	*/	
	uint32_t  sip;
	uint32_t  dip;  
	uint16_t  sport;
	uint16_t  dport;
	uint8_t  protocol;
	uint8_t  direct_flag; /* flag to uplink downlink*/

	/*策略信息44 Bytes */
	uint16_t forward_port; 
	uint32_t action_type;	/*the action for drop/capwap tunnel/forward/etc*/
	uint32_t action_mask;	/*the action mask for meter/nat/etc*/
	uint64_t time_stamp;  /*the rule time stamp for age*/
	void *extend_index;	    /*for future action extend "action_param_t *"  */
	uint16_t tunnel_index; /*if capwap packet, this is the capwap index*/
	uint16_t nat_index;	/*the NAT table index*/
	uint32_t user_index; /*identify the user index */
	uint16_t user_link_index; /*identify the user link index */
	uint8_t  rule_state; /* static/learning / invalid / learned*/
	volatile uint8_t  packet_wait; /*流表学习成功前，转发到linux的报文数量*/
	/* maybe one rule belong to 2 users, e.g. sending a file between two users. add by zhaohan */
	uint32_t user2_index; /*identify the user2 index */	
	uint16_t user2_link_index; /*identify the user2 link index */
	
    // 新增字段
    /* nat related */
    uint16_t nat_flag;
    uint32_t nat_sip;
    uint32_t nat_dip;
    uint16_t nat_sport;
    uint16_t nat_dport;

	/*pppoe related*/
	uint16_t pppoe_flag;
	uint16_t pppoe_session_id;

//}__attribute__ ((packed)) rule_param_t ;
}rule_param_t;










/************************************************
*	flow action type define
*************************************************/
#define FLOW_ACTION_DROP   1
#define FLOW_ACTION_TOLINUX   2
#define FLOW_ACTION_ICMP   3
#define FLOW_ACTION_CAPWAP_802_11_ICMP  4
#define FLOW_ACTION_ETH_FORWARD   5
#define FLOW_ACTION_CAPWAP_FORWARD   6
#define FLOW_ACTION_CAP802_3_FORWARD   7
#define FLOW_ACTION_CAPWAP_802_3_ICMP   8

#define FLOW_ACTION_RPA_ETH_FORWARD         9
#define FLOW_ACTION_RPA_CAPWAP_FORWARD      10
#define FLOW_ACTION_RPA_CAP802_3_FORWARD    11
#define FLOW_ACTION_RPA_CAPWAP_802_3_ICMP   12
#define FLOW_ACTION_RPA_CAPWAP_802_11_ICMP  13
#define FLOW_ACTION_RPA_ICMP                14
#define FLOW_ACTION_RPA_DIRECT_FORWARD      15
#define FLOW_ACTION_HEADER_FRAGMENTS_STORE  16

/********************************************************************/

#define RULE_IS_EMPTY				0
#define RULE_IS_LEARNED				1
#define RULE_IS_LEARNING			2
#define RULE_IS_STATIC				3 /*由控制面直接下发的表项，为静态表项，不老化*/
#define RULE_IS_CMD_SET				4 /*由控制面直接下发的表项*/
#define RULE_IS_INVALID				0xff















/*****************************************************************************
*   FCCP related
*****************************************************************************/
#define FCCP_WQE_TYPE      0x85 /* we can mark in wqe for fccp pkts. maybe it's faster than compare pkts */
#define FCCP_L2_ETH_TYPE   0x8050
#define FCCP_ETH_HEADER_LENGTH  14

/* dst Module definition*/
#define FCCP_MODULE_ACL 1
#define FCCP_MODULE_NAT 2
#define FCCP_MODULE_CAR 3
#define FCCP_MODULE_DBG 4

/* src Module definition */
#define FCCP_MODULE_AGENT_ACL 101
#define FCCP_MODULE_AGENT_NAT 102
#define FCCP_MODULE_AGENT_CAR 103
#define FCCP_MODULE_AGENT_DBG 104

/* opcode definition */
#define FCCP_CMD_INSERT 1
#define FCCP_CMD_DELETE 2
#define FCCP_CMD_SET_AGEING_TIMER 3
#define FCCP_CMD_GET_AGEING_TIMER 4
#define FCCP_CMD_RULE_CNT    5
#define FCCP_CMD_INSERT_ICMP 6


/* fccp return code */
#define FCCP_RETURN_OK        0x12340000
#define FCCP_RETURN_ERROR     0x12340001  
    

typedef struct
{
    int32_t total_pkt;
	int32_t total_pps;
    int32_t err_pkt;
	int32_t err_pps;
    int32_t bcast_pkt;
	int32_t bcast_pps;
    int32_t mcast_pkt;
    int32_t mcast_pps;
	
    /* noip */
    int32_t arp_pkt;
	int32_t arp_pps;
	
    int32_t rarp_pkt;
	int32_t rarp_pps;
    int32_t vrrp_pkt;
    int32_t vrrp_pps;
    int32_t _802_1x_pkt;
	int32_t _802_1x_pps;
	int32_t total_noip_pkt;
    int32_t total_noip_pps;

    /* ip */
    int32_t ip_exception_pkt;
	int32_t ip_exception_pps;
    int32_t frag_pkt;
    int32_t frag_pps;
    int32_t ipv6_pkt;
	int32_t ipv6_pps;
    int32_t l4_err_pkt;
    int32_t l4_err_pps;

    /* tcp */
    int32_t telnet_pkt;
	int32_t telnet_pps;
    int32_t ssh_pkt;
    int32_t ssh_pps;
    int32_t total_tcp_pkt;
	int32_t total_tcp_pps;

    /* udp */
    int32_t dhcp_pkt;
	int32_t dhcp_pps;
    int32_t rip_pkt;
    int32_t rip_pps;
    int32_t cw_ctl_pkt;
	int32_t cw_ctl_pps;
    int32_t cw_dat_pkt;
    int32_t cw_dat_pps;
    int32_t cw_ip_exc_pkt;
	int32_t cw_ip_exc_pps;
    int32_t cw_ip_frag_pkt;
    int32_t cw_ip_frag_pps;
    int32_t cw_8023_dat_pkt;
	int32_t cw_8023_dat_pps;
	
	int32_t access_radius_pkt;
	int32_t access_radius_pps;
	int32_t account_radius_pkt;
	int32_t account_radius_pps;
	
	int32_t portal_pkt;
	int32_t portal_pps;
    int32_t inter_ac_roaming_pkt;
    int32_t inter_ac_roaming_pps;
    int32_t total_udp_pkt;
	int32_t total_udp_pps;
	int32_t cw_arp_pkt;
	int32_t cw_arp_pps;
	int32_t cw_udp_pkt;
	int32_t cw_udp_pps;
	int32_t cw_tcp_pkt;
	int32_t cw_tcp_pps;
	int32_t cw_icmp_pkt;
	int32_t cw_icmp_pps;

    /* icmp */
    int32_t icmp_pkt;
	int32_t icmp_pps;

    /* total ip */
    int32_t total_ip_pkt;
	int32_t total_ip_pps;
}traffic_stats_t;

/*fau unit  information*/
typedef struct 
{
	//int64_t fau_reg_oq_addr_index;/*FAU registers for the position in PKO command buffers*/
	int64_t fau_pko_errors;/*64-bit counter used for total ethernet output error packets*/
	int64_t fau_enet_input_bytes;/*64-bit counter used for total ethernet input bytes*/
	int64_t fau_enet_input_packets;/*64-bit counter used for total ethernet input packets*/
	int64_t fau_enet_output_bytes;/*64-bit counter used for total ethernet output bytes*/
	int64_t fau_enet_output_packets;/*64-bit counter used for total ethernet output packets*/
	int64_t fau_enet_drop_packets;/*64-bit counter used for total ethernet drop packets*/
	int64_t fau_enet_to_linux_packets;/*64-bit counter used for total ethernet to linux packets*/
	int64_t fau_enet_nonip_packets;/*64-bit counter used for total ethernet input noneip packets*/
	int64_t fau_enet_error_packets;/*64-bit counter used for total ethernet input error packets*/
	int64_t fau_enet_fragip_packets;/*64-bit counter used for total ethernet input fragip packets*/
	int64_t fau_ip_short_packets;/*64-bit counter used for short IP packets rcvd*/
	int64_t fau_ip_bad_hdr_len;/*64-bit counter used for IP packets with bad hdr len*/
	int64_t fau_ip_bad_len;/*64-bit counter used for IP packets with bad len*/
	int64_t fau_ip_bad_version;/*64-bit counter used for IP packets with bad version*/
	int64_t fau_ip_skip_addr;/*64-bit counter used for IP packets with SKIP addr*/
	int64_t fau_ip_icmp;/*64-bit counter used for ICMP packets*/
	int64_t fau_capwap_icmp;/*64-bit counter used for Capwap ICMP packets*/
	int64_t fau_ip_proto_error;/*64-bit counter used for ip packets with proto error*/
	int64_t fau_udp_bad_dropt;/*64-bit counter used for udp dport=0 packets*/
	int64_t fau_udp_bad_len;/*64-bit counter used for udp packets with len error*/
	int64_t fau_udp_to_linux;/*64-bit counter used for udp packets that trap to Linux*/
	int64_t fau_flowtable_hit_packets;/*64-bit counter used for ACL HIT packets number*/
	int64_t fau_enet_output_packets_8021q;/*64-bit counter used for total ethernet output 802.1qpackets*/
	int64_t fau_enet_output_packets_qinq;/*64-bit counter used for total ethernet output qinq packets*/
	int64_t fau_flow_lookup_error;/*64-bit counter used for total flow lookup failed counter*/
	int64_t fau_recv_fccp;/*64-bit counter used for total received fccp packets counter*/
	int64_t fau_recv_works;/*64-bit counter used for total received works counter, include input packets and fccp packets*/
	int64_t fau_acl_lookup;/*64-bit counter used for total acl lookup counter*/
	int64_t fau_acl_reg;/*64-bit counter used for total acl setup and regist packets counter*/
	int64_t fau_cw802_11_decap_err;/*64-bit counter used for total capwap 802.11 decap error*/
	int64_t fau_cw802_3_decap_err;/*64-bit counter used for total capwap 802.3 decap error*/
	int64_t fau_enet_to_linux_bytes;/**< 64-bit counter used for total ethernet to linux bytes */
	int64_t fau_alloc_rule_fail;/**< 64-bit counter used for total alloc rule fail counter */
	int64_t fau_max_rule_entries;/**< 64-bit counter used for reach max rule entries */
	int64_t fau_cw_80211_err;/*64-bit counter used for 802.11 error packet*/
	int64_t fau_cw_noip_packets;/*64-bit counter used for capwap no ip packet*/
	int64_t fau_cw_spe_packets;/*64-bit counter used for capwap special packet*/
	int64_t fau_cw_frag_packets;/*64-bit counter used for capwap frag packet*/
	int64_t fau_mcast_packets;
	int64_t fau_rpa_packets;
	int64_t fau_rpa_tolinux_packets;
	int64_t fau_tipc_packets;
	int64_t fau_large_eth2cw_packets;
	int64_t fau_large_cw_rpa_fwd_packets;
	int64_t fau_large_cw8023_rpa_fwd_packets;
	int64_t fau_spe_tcp_hdr;
	int64_t fau_cw_spe_tcp_hdr;
}fau64_info_t;


typedef union{
    struct
	{
		rule_param_t rule_param;
		capwap_cache_t cw_cache;
    }rule_info;
	traffic_stats_t traffic_stats;
	fau64_info_t fau64_info;
}fccp_data_t;


/* FCCP format */
typedef struct  control_cmd_s{   
    uint16_t dest_module; 
    uint16_t src_module; 
    uint16_t cmd_opcode; 
    uint16_t cmd_len; 	
    uint32_t agent_pid;
    uint32_t ret_val;
    fccp_data_t fccp_data;
}control_cmd_t;

#define FCCP_PACKET_LEN		ETH_H_LEN + sizeof(control_cmd_t)





/*****************************************************************************
*   NET DEVICE PRIV port related
*****************************************************************************/
/**
 * These enumerations are the return codes for the Ethernet
 * driver intercept callback. Depending on the return code,
 * the ethernet driver will continue processing in different
 * ways.
 */
typedef enum
{
    CVM_OCT_PASS,               /**< The ethernet driver will pass the packet
                                    to the kernel, just as if the intercept
                                    callback didn't exist */
    CVM_OCT_DROP,               /**< The ethernet driver will drop the packet,
                                    cleaning of the work queue entry and the
                                    skbuff */
    CVM_OCT_TAKE_OWNERSHIP_WORK,/**< The intercept callback takes over
                                    ownership of the work queue entry. It is
                                    the responsibility of the callback to free
                                    the work queue entry and all associated
                                    packet buffers. The ethernet driver will
                                    dispose of the skbuff without affecting the
                                    work queue entry */
    CVM_OCT_TAKE_OWNERSHIP_SKB  /**< The intercept callback takes over
                                    ownership of the skbuff. The work queue
                                    entry and packet buffer will be disposed of
                                    in a way keeping the skbuff valid */
} cvm_oct_callback_result_t;


typedef cvm_oct_callback_result_t (*cvm_oct_callback_t)(struct net_device *dev, void *work_queue_entry, struct sk_buff *skb);

/**
 * This is the definition of the Ethernet driver's private
 * driver state stored in dev->priv.
 */
#ifdef BUILDNO_VERSION2_1
typedef struct octeon_ethernet {
	struct rb_node ipd_tree;
	int key;
	int ipd_port;
	int pko_port;
	int ipd_pkind;
	int interface;
	int interface_port;
	/* My netdev. */
	struct net_device *netdev;
	/* My location in the cvm_oct_list */
	struct list_head list;
	/*
	 * Type of port. This is one of the enums in
	 * cvmx_helper_interface_mode_t
	 */
	int imode;

	unsigned int has_gmx_regs:1;
	unsigned int tx_timestamp_sw:1;
	unsigned int tx_timestamp_hw:1;
	unsigned int rx_timestamp_sw:1;
	unsigned int rx_timestamp_hw:1;
	unsigned int tx_lockless:1;

	/* Optional intecept callback defined above */
	cvm_oct_callback_t      intercept_cb;

	/* Number of elements in tx_queue below */
	int                     num_tx_queues;

	/* SRIO ports add this header for the SRIO block */
	u64 srio_tx_header;

	struct {
		/* PKO hardware queue for the port */
		int	queue;
		/* Hardware fetch and add to count outstanding tx buffers */
		int	fau;
	} tx_queue[16];

	struct phy_device *phydev;
	/* MII bus the PHY is connected to */
	struct mii_bus *mii_bus;
	unsigned int last_link;
	/* Last negotiated link state */
	u64 link_info;
	/* Called periodically to check link status */
	void (*poll) (struct net_device *dev);
	struct delayed_work	port_periodic_work;
	struct work_struct	port_work;	/* may be unused. */
	struct device_node	*of_node;
	u64 last_tx_octets;
	u32 last_tx_packets;
    struct mii_if_info      mii_info;       /* Generic MII info structure */
}cvm_oct_private_t;

#define GET_PRIV_PKO_PORT(priv)  ((priv)->pko_port) 

#else
typedef struct
{
    int                     port;           /* PKO hardware output port */
    int                     queue;          /* PKO hardware queue for the port */
    int                     fau;            /* Hardware fetch and add to count outstanding tx buffers */
    int                     imode;          /* Type of port. This is one of the enums in cvmx_helper_interface_mode_t */
	int						portNum;		/* Port number of Marvell Chip*/
	int						devNum;			/* Device number of Marvell Chip*/
    struct sk_buff_head     tx_free_list;   /* List of outstanding tx buffers */
    struct net_device_stats stats;          /* Device statistics */
    struct mii_if_info      mii_info;       /* Generic MII info structure */
    cvm_oct_callback_t      intercept_cb;   /* Optional intecept callback defined above */
	uint64_t                link_info;      /* Last negotiated link state */
    void (*poll)(struct net_device *dev);   /* Called periodically to check link status */
	//add by baolc, 2008-06-11
	unsigned short          vlanId;         /* VLAN id */
} cvm_oct_private_t;
#define GET_PRIV_PKO_PORT(priv)  ((priv)->port)

#endif


/* PCIe interface name called octxxx */
#define IS_PCIE_IF(m)  ( ((m)[0] == 'v') && ((m)[1] == 'e') && ((m)[4] == 's') )

enum fwd_dev_type_t
{
	DEV_TYPE_NONE = 0,
	DEV_TYPE_OCTEON_ETH = 1,
	DEV_TYPE_OCTEON_PCIE = 2
};

/* Statistics for every exception branch. */
/* It's very useful for debuging */
typedef struct ipfwd_learn_stats_s
{
	int64_t total_recv_cnt;
	int64_t mcast;
	int64_t spe_if_name;
	int64_t l2hdr_parse_fail;
	int64_t spe_ip;
	int64_t local_sip;
	int64_t local_dip;	
	int64_t icmp;
	int64_t udp_decap_fail;
	int64_t wqe_alloc_fail;	
	int64_t pkt_alloc_fail;	
	int64_t priv_flags_err;
	int64_t spe_udp_port;
	int64_t get_send_port;
	int64_t eth_learn_fail;
	int64_t cw_priv_flags_err;
	int64_t cw_spe_ip;
	int64_t cw_spe_udp_port;
	int64_t cw_get_send_port;
	int64_t cw_learn_fail;	
	int64_t cw8023_priv_flags_err;
	int64_t cw8023_spe_ip;
	int64_t cw8023_spe_udp_port;
	int64_t cw8023_get_send_port;
	int64_t cw8023_learn_fail;		
	int64_t icmp_learn_fail;
	int64_t cw_icmp_learn_fail;
	int64_t cw8023_icmp_learn_fail;
	int64_t pkt_type_nonsupport;
	int64_t eth_good_cnt;
	int64_t cw_good_cnt;
	int64_t cw8023_good_cnt;
	int64_t icmp_good_cnt;
	int64_t cw8023_icmp_good_cnt;					
	int64_t cw_icmp_good_cnt;
	int64_t total_good_cnt;	
}ipfwd_learn_stats_t;

CVMX_SHARED ipfwd_learn_stats_t fwd_learn_stats;


#endif


