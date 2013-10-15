#ifndef SE_AGENT_DEF_H
#define SE_AGENT_DEF_H


#define HAND_CMD_SIZE  64   
#define ERR_INFO_SIZE  128	/*The length of the error information*/
#define CW_H_LEN       16

#define FCCP_L2_ETH_TYPE  0x8050
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
#define FCCP_CMD_INSERT                  	1
#define FCCP_CMD_DELETE                  	2
#define FCCP_CMD_SET_AGEING_TIMER        	3
#define FCCP_CMD_GET_AGEING_TIMER        	4
#define FCCP_CMD_RULE_CNT                	5
#define FCCP_CMD_INSERT_ICMP             	6
#define FCCP_CMD_ENABLE_ICMP             	7
#define FCCP_CMD_GET_ICMP_STATE          	8
#define FCCP_CMD_GET_RULE_STATS 			9
#define FCCP_CMD_CLEAR_RULE 				10
#define FCCP_CMD_CLEAR_AGING_RULE 			11
#define FCCP_CMD_SET_BUCKET_MAX_ENTRY 		12
#define FCCP_CMD_GET_BUCKET_MAX_ENTRY 		13
#define FCCP_CMD_ENABLE_TRAFFIC_MONITOR    	14
#define FCCP_CMD_CLEAR_TRAFFIC_MONITOR  	15
#define FCCP_CMD_GET_TRAFFIC_MONITOR   		16
#define FCCP_CMD_USER_ONLINE 				17
#define FCCP_CMD_USER_OFFLINE 				18
#define FCCP_CMD_USER_STATS_CLEAR 			19
#define FCCP_CMD_ENABLE_PURE_PAYLOAD_ACCT 	20
#define FCCP_CMD_DEL_RULE_BY_IP 			21
#define FCCP_CMD_TEST 						22              
#define FCCP_CMD_SHOW_RULE_BY_IP 			23    
#define FCCP_CMD_EQUIPMENT_TEST 			24
#define FCCP_CMD_ENABLE_LOG 				25
#define FCCP_CMD_TIMESYNC 					26
#define FCCP_CMD_ENABLE_LOG_SHOW 			27
#define FCCP_CMD_SET_LOG_LEVEL 				28
#define FCCP_CMD_SHOW_LOG_LEVEL 			29
#define FCCP_CMD_CONFIG_TAG_TYPE 			37
#define FCCP_CMD_GET_TAG_TYPE 				38
#define FCCP_CMD_SHOW_FAU64 				39
#define FCCP_CMD_CLEAR_FAU64 				40
#define FCCP_CMD_SHOW_FPA_BUFF 				41
#define FCCP_CMD_ENABLE_PURE_IP             42
#define FCCP_CMD_GET_PURE_IP_STATE    		43
#define FCCP_CMD_SHOW_PART_FAU64 			44
#define FCCP_CMD_SHOW_ETH_FAU64 			45
#define FCCP_CMD_SHOW_CAPWAP_FAU64 			46
#define FCCP_CMD_SHOW_RPA_FAU64 			47
#define FCCP_CMD_CLEAR_PART_FAU64 			48
#define FCCP_CMD_ENABLE_PPPOE             	49
#define FCCP_CMD_GET_PPPOE_STATE          	50
#define FCCP_CMD_ENABLE_IPV6	            51
#define FCCP_CMD_GET_IPV6_STATE    			52
#define FCCP_CMD_ENABLE_PURE_IPV6           53
#define FCCP_CMD_GET_PURE_IPV6_STATE    	54






#define FCCP_RETURN_OK          0x12340000
#define FCCP_RETURN_ERROR       0x12340001  

#define AGENT_RETURN_OK         0x12340010
#define AGENT_RETURN_FAIL       0x12340011

#define FUNC_ENABLE         1
#define FUNC_DISABLE        0

/*ipfwd_learned module syslog level*/
#define EMERG_LVL			0
#define ALERT_LVL			1
#define CRIT_LVL			2
#define ERR_LVL				3
#define WARNING_LVL			4
#define NOTICE_LVL			5
#define INFO_LVL			6
#define ICMP_DEBUG			7
#define DEBUG_LVL			8
#define IPFWD_DEFAULT_LVL	INFO_LVL

/*socket communication  block time (sec) */
#define AGENT_WAIT_TIME     10
#define DCLI_WAIT_TIME      (AGENT_WAIT_TIME+1)

/*protocol type*/
#define TCP_TYPE         6
#define UDP_TYPE         17
#define ICMP_TYPE        1
#define INVALID_TYPE     0

/*fast forward module  flow action type */
#define FLOW_ACTION_DROP                  1
#define FLOW_ACTION_TOLINUX               2
#define FLOW_ACTION_ICMP                  3
#define FLOW_ACTION_CAPWAP_802_11_ICMP    4
#define FLOW_ACTION_ETH_FORWARD           5
#define FLOW_ACTION_CAPWAP_FORWARD        6
#define FLOW_ACTION_CAP802_3_FORWARD      7
#define FLOW_ACTION_CAPWAP_802_3_ICMP     8
#define FLOW_ACTION_RPA_ETH_FORWARD         9
#define FLOW_ACTION_RPA_CAPWAP_FORWARD      10
#define FLOW_ACTION_RPA_CAP802_3_FORWARD    11
#define FLOW_ACTION_RPA_CAPWAP_802_3_ICMP   12
#define FLOW_ACTION_RPA_CAPWAP_802_11_ICMP  13
#define FLOW_ACTION_RPA_ICMP                14
#define FLOW_ACTION_RPA_DIRECT_FORWARD      15


/*fast forward module  rule type */
#define RULE_IS_EMPTY				0
#define RULE_IS_LEARNED				1
#define RULE_IS_LEARNING			2
#define RULE_IS_STATIC				3 
#define RULE_IS_CMD_SET				4 
#define RULE_IS_AGE                 5 
#define RULE_IS_NEW                 6
#define RULE_IS_INVALID				0xff

#define CPU_TAG_MASTER              0
#define CPU_TAG_SLAVE				1
#define CPU_TAG_ALL                 2

#define IPV6_CMP(a, b) ((a.s6_addr64[0] == b.s6_addr64[0]) && (a.s6_addr64[1] == b.s6_addr64[1]))


/* ethernet headers*/
typedef struct eth_hdr_s 
{
    uint8_t	 h_dest[6];	   /*destination eth addr */
    uint8_t h_source[6];   /* source ether addr	*/
    uint16_t h_vlan_proto; /* Should always be 0x8100 */
}eth_hd_r;

/*
 *	IPv6 address structure
 */

struct cvm_ip6_in6_addr
{
	union 
	{
		uint8_t			u6_addr8[16];
		uint16_t		u6_addr16[8];
		uint32_t		u6_addr32[4];
		uint64_t		u6_addr64[2];
	} in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
#define s6_addr64		in6_u.u6_addr64
};



typedef struct  capwap_cache_tbl_s
{
    uint32_t use_num;
    /* Extern IP header */
		union {
				uint32_t cw_sip_v4;
				struct	cvm_ip6_in6_addr	cw_sip_v6;
	
		}cw_sip_addr;
	
		union {
				uint32_t cw_dip_v4;
				struct	cvm_ip6_in6_addr	cw_dip_v6;
		
		}cw_dip_addr;
					
#define cw_sip 			cw_sip_addr.cw_sip_v4
#define cw_dip 			cw_dip_addr.cw_dip_v4
#define cw_ipv6_sip 	cw_sip_addr.cw_sip_v6
#define cw_ipv6_sip8 	cw_sip_addr.cw_sip_v6.s6_addr8
#define cw_ipv6_sip16 	cw_sip_addr.cw_sip_v6.s6_addr16
#define cw_ipv6_sip32 	cw_sip_addr.cw_sip_v6.s6_addr32
#define cw_ipv6_sip64 	cw_sip_addr.cw_sip_v6.s6_addr64
#define cw_ipv6_dip 	cw_dip_addr.cw_dip_v6
#define cw_ipv6_dip8 	cw_dip_addr.cw_dip_v6.s6_addr8
#define cw_ipv6_dip16 	cw_dip_addr.cw_dip_v6.s6_addr16
#define cw_ipv6_dip32 	cw_dip_addr.cw_dip_v6.s6_addr32
#define cw_ipv6_dip64 	cw_dip_addr.cw_dip_v6.s6_addr64


	//uint32_t dip;
	//uint32_t sip;
    uint16_t dport;
    uint16_t sport;
    uint8_t tos;
    /*current entry is valid or not*/
    /*uint8_t    valid_bit;	*/
    /* CAPWAP header */
    uint8_t cw_hd[CW_H_LEN];
}capwap_cache_t;


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

	union {
			uint32_t acl_sip_v4;
			struct	cvm_ip6_in6_addr acl_sip_v6;

	}sip_addr;

	union {
			uint32_t acl_dip_v4;
			struct	cvm_ip6_in6_addr acl_dip_v6;

	}dip_addr;
									
#define ipv4_sip 	sip_addr.acl_sip_v4
#define ipv4_dip 	dip_addr.acl_dip_v4
#define ipv6_sip	sip_addr.acl_sip_v6
#define ipv6_sip8 	sip_addr.acl_sip_v6.s6_addr
#define ipv6_sip16 	sip_addr.acl_sip_v6.s6_addr16
#define ipv6_sip32 	sip_addr.acl_sip_v6.s6_addr32
#define ipv6_sip64 	sip_addr.acl_sip_v6.s6_addr64
#define ipv6_dip 	dip_addr.acl_dip_v6
#define ipv6_dip8 	dip_addr.acl_dip_v6.s6_addr
#define ipv6_dip16 	dip_addr.acl_dip_v6.s6_addr16
#define ipv6_dip32 	dip_addr.acl_dip_v6.s6_addr32
#define ipv6_dip64 	dip_addr.acl_dip_v6.s6_addr64

	
	uint8_t ipv6_flag;
	/*L3-4 header, the HASH key  14Bytes	*/	
	//uint32_t  sip;
	//uint32_t  dip;  
	uint16_t  sport;
	uint16_t  dport;
	uint8_t  protocol;
	uint8_t  direct_flag; /* flag to uplink downlink*/

	/*策略信息44 Bytes */
	uint16_t forward_port; 
	uint32_t action_type;	/*the action for drop/capwap tunnel/forward/etc*/
	uint32_t action_mask;	/*the action mask for meter/nat/etc*/
	uint64_t time_stamp;  /*the rule time stamp for age*/
	uint64_t extend_index;	    /*for future action extend "action_param_t *"  */
	uint16_t tunnel_index; /*if capwap packet, this is the capwap index*/
	uint16_t nat_index;	/*the NAT table index*/
	uint32_t user_index; /*identify the user index */
	uint16_t user_link_index; /*identify the user link index */
	uint8_t  rule_state; /* static/learning / invalid / learned*/
	volatile uint8_t  packet_wait; /*before acl learned successful, the number of packets forwarded to the linux*/
	/* maybe one rule belong to 2 users, e.g. sending a file between two users. add by zhaohan */
	uint32_t user2_index; /*identify the user2 index */	
	uint16_t user2_link_index; /*identify the user2 link index */

    /* nat related *//*wangjian_nat*/
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
	int64_t fau_enet_eth_pppoe_nonip_packets; 			/*64-bit counter used for total ethernet input eth pppoe noneip packets*/
	int64_t fau_enet_capwap_pppoe_nonip_packets;	/*64-bit counter used for total ethernet input capwap pppoe noneip packets*/
	int64_t fau_enet_output_packets_eth_pppoe;			/*64-bit counter used for total ethernet output eth pppoe packets*/
	int64_t fau_enet_output_packets_capwap_pppoe;   /*64-bit counter used for total ethernet output capwap pppoe packets*/
}fau64_info_t;

typedef struct 
{
	int64_t fau_enet_output_packets_eth;   /*64-bit counter used for total ethernet output eth packets*/
	int64_t fau_enet_output_packets_capwap;   /*64-bit counter used for total ethernet output capwap packets*/
	int64_t fau_enet_output_packets_rpa;   /*64-bit counter used for total ethernet output rpa packets*/
	int64_t fau_enet_input_packets_eth;   /*64-bit counter used for total ethernet input eth packets*/
	int64_t fau_enet_input_packets_capwap;   /*64-bit counter used for total ethernet input capwap packets*/
	int64_t fau_enet_input_packets_rpa;   /*64-bit counter used for total ethernet input rpa packets*/
	int64_t fau_enet_output_bytes_eth;   /*64-bit counter used for total ethernet output eth bytes*/
	int64_t fau_enet_output_bytes_capwap;   /*64-bit counter used for total ethernet output capwap bytes*/
	int64_t fau_enet_output_bytes_rpa;   /*64-bit counter used for total ethernet output rpa bytes*/
	int64_t fau_enet_input_bytes_eth;   /*64-bit counter used for total ethernet input eth bytes*/
	int64_t fau_enet_input_bytes_capwap;   /*64-bit counter used for total ethernet input capwap bytes*/
	int64_t fau_enet_input_bytes_rpa;   /*64-bit counter used for total ethernet input rpa bytes*/

}fau64_part_info_t;

typedef struct 
{
	int64_t fau_enet_output_packets_eth;   /*64-bit counter used for total ethernet output eth packets*/
	int64_t fau_enet_input_packets_eth;   /*64-bit counter used for total ethernet input eth packets*/
	int64_t fau_enet_output_bytes_eth;   /*64-bit counter used for total ethernet output eth bytes*/
	int64_t fau_enet_input_bytes_eth;   /*64-bit counter used for total ethernet input eth bytes*/
}fau64_eth_info_t;

typedef struct 
{
	int64_t fau_enet_output_packets_capwap;   /*64-bit counter used for total ethernet output capwap packets*/
	int64_t fau_enet_input_packets_capwap;   /*64-bit counter used for total ethernet input capwap packets*/
	int64_t fau_enet_output_bytes_capwap;   /*64-bit counter used for total ethernet output capwap bytes*/
	int64_t fau_enet_input_bytes_capwap;   /*64-bit counter used for total ethernet input capwap bytes*/
}fau64_capwap_info_t;

typedef struct 
{
	int64_t fau_enet_output_packets_rpa;   /*64-bit counter used for total ethernet output rpa packets*/
	int64_t fau_enet_input_packets_rpa;   /*64-bit counter used for total ethernet input rpa packets*/
	int64_t fau_enet_output_bytes_rpa;   /*64-bit counter used for total ethernet output rpa packets*/
	int64_t fau_enet_input_bytes_rpa;   /*64-bit counter used for total ethernet input rpa packets*/
}fau64_rpa_info_t;







typedef struct 
{
	uint32_t acl_static_tbl_size;
	uint32_t acl_dynamic_tbl_size;
	uint32_t s_tbl_used_rule ;
	uint32_t s_tbl_aged_rule ;
	uint32_t s_tbl_learned_rule;
	uint32_t s_tbl_learning_rule ;
	uint32_t s_tbl_static_rule;
	uint32_t d_tbl_used_rule ;
	uint32_t d_tbl_aged_rule ;
	uint32_t d_tbl_learned_rule ;
	uint32_t d_tbl_learning_rule ;
	uint32_t d_tbl_static_rule ;
	uint32_t capwap_cache_tbl_size;
	uint32_t cw_tbl_used ;
	uint32_t cw_tbl_802_3_num ;
	uint32_t cw_tbl_802_11_num ;
	uint32_t s_tbl_uplink_rule;
	uint32_t d_tbl_uplink_rule;
	uint32_t s_tbl_downlink_rule;
	uint32_t d_tbl_downlink_rule; 
}rule_stats_t;
#define USER_IS_EMPTY		0
#define USER_IS_ONLINE		1 /*用户上线*/
#define USER_IS_OFFLINE		2 /*用户下线*/

#define DIRECTION_UP		1
#define DIRECTION_DOWN		2

typedef struct user_info_s
{
	uint64_t forward_up_bytes;
	uint64_t forward_up_packet;
	uint64_t forward_down_bytes;
	uint64_t forward_down_packet;

	uint32_t user_ip;	
	uint16_t meter_index;	/*the CAR index*/	
	uint16_t flow_number;
	/*below is the user car information*/
	uint32_t   depth;       /*CBS in bytes*/
	uint32_t   rate;        /* CIR in kbps*/
	uint64_t   rate_in_cycles_per_byte;
	uint64_t   depth_in_cycles;
	uint64_t   cycles_prev;

	uint8_t  user_state;
	uint8_t  reserved[7];	
} user_info_t;

typedef struct
{
    uint32_t    user_static_tbl_size;
    uint32_t    user_dynamic_tbl_size;
    uint32_t    s_tbl_used_rule;
    uint32_t    d_tbl_used_rule;
}user_stats_t;

typedef struct
{
    uint32_t    user_index;
    uint32_t    user_link_index;
    int32_t     static_index;
    uint32_t    dynamic_index;
    user_info_t user_info;
}user_rule_t;

typedef struct 
{
	uint16_t  level;
	uint16_t  enable;
}agent_dbg_t;
typedef struct
{
	uint64_t   address;
	uint64_t   reg_data;
}register_info_t;


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


/*
 * Autelan product type enum-type
 */
typedef enum board_type_s
{
	AUTELAN_BOARD_AX71_CRSMU,
	AUTELAN_BOARD_AX71_2X12G12S,
	AUTELAN_BOARD_AX81_SMU,
	AUTELAN_BOARD_AX81_AC8C,
	AUTELAN_BOARD_AX81_AC12C,
	AUTELAN_BOARD_AX81_2X12G12S,
	AUTELAN_BOARD_AX81_1X12G12S,
	AUTELAN_BOARD_AX81_AC_12X,
	AUTELAN_BOARD_AX81_AC_4X,
	AUTELAN_BOARD_AX71_1X12G12S,
	AUTELAN_UNKNOWN_BOARD
	/* 
	 * if you want to add some new autelan product types
	 * please add it upon the AUTELAN_UNKNOWN_BOARD in order
	 * and add it in src/kernel2.6.32.27cn/include/linux/autelan_product.h synchronously
	 */
}board_type_t;


typedef struct
{
    uint32_t enable;
    uint32_t board_type;
}equipment_test_cntl_t;

typedef union
{
    struct
	{
        rule_param_t rule_param;
        capwap_cache_t cw_cache;
    }rule_info;
	struct
	{
		uint32_t cw_cache_index;
		uint32_t cw_use_cnt;
		capwap_cache_t cw_cache_rule;	
	}cw_cache_info;

	struct
	{
		rule_param_t acl_param;
		uint32_t acl_static_index;
		uint32_t acl_dynamic_index;
		uint32_t acl_cnt;
		uint32_t acl_static_cnt;
	}acl_info;
    struct
	{
        uint32_t ip_src; 
        uint32_t ip_dst; 
        uint16_t th_sport; 
        uint16_t th_dport; 
        uint8_t  ip_p;
    }rule_tuple;
    struct 
	{
		uint64_t  static_aging_cnt;
		uint64_t  dynamic_aging_cnt;
	}rule_aging_cnt;
	struct
	{
		uint64_t to_linux_bps;
		uint64_t to_linux_pps;
	}to_linux_flow;
	uint32_t bucket_max_entry;
    uint32_t aging_timer;
	uint32_t share;
	agent_dbg_t  agent_dbg;
	uint32_t port_no;
	uint32_t tag_type;
	uint32_t module_enable;/*enable or disable*/
	uint64_t time_sec;   /*linux sec sendto fwd*/
	struct{
        uint32_t fast_fwd_enable;   /*fast forward enable wj*/
        uint32_t fast_fwd_coremask; /*fast forward occupy core */
    }fast_fwd_info;                 /*wangjian 2012.07.09 add fwd info */
	register_info_t reg_param;/*use by read or write register*/
	rule_stats_t  rule_sum;
	user_info_t  user_info;
	traffic_stats_t traffic_stats;
	equipment_test_cntl_t equipment_test;
	user_stats_t    user_stats;
	user_rule_t     user_rule;
	fau64_info_t    fau64_info;
	fau64_part_info_t fau64_part_info;
	fau64_eth_info_t fau64_eth_info;
	fau64_capwap_info_t fau64_capwap_info;
	fau64_rpa_info_t fau64_rpa_info;
	uint32_t pool_buff_count[8];
}fccp_data_t;

typedef struct  control_cmd_s
{   
    uint16_t dest_module; 
    uint16_t src_module; 
    uint16_t cmd_opcode; 
    uint16_t cmd_len; 	
    uint32_t agent_pid;
    uint32_t ret_val;
    fccp_data_t fccp_data;
}control_cmd_t;


typedef struct se_interative_s
{
	char hand_cmd[HAND_CMD_SIZE];
	control_cmd_t  fccp_cmd;
	int cmd_result;
	int cpu_tag;
	char err_info[ERR_INFO_SIZE];/*Error message is returned by se_agent*/	
}se_interative_t;



#define NETLINK_SE_AGENT  22


#define SE_AGENT_SERVER_TYPE        0x4000
#define SE_AGENT_SERVER_LOWER_INST  1000
#define MAX_SLOT_NUM                16

/*return  value*/
#define SE_AGENT_RETURN_FAIL 	-1
#define SE_AGENT_RETURN_OK		 0
/*buff length for se_agent show running config memory*/
#define SE_AGENT_RUNNING_CFG_MEM		(1024)

/*METHOD DEFINE*/
#define SE_AGENT_CONFIG_AGENT_DEBUG_LEVEL               "config_se_agent_debug"
#define SE_AGENT_UART_SWITCH							"uart_switch_to_fastfwd"
#define SE_AGENT_SHOW_RUNNING_CFG                       "show_running_configure"
#define SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT             "show_availiable_buff_count"
#define SE_AGENT_SET_FWD_TAG_TYPE                       "config_tag_type"
#define SE_AGENT_SHOW_FWD_TAG_TYPE						"show_tag_type"
#define SE_AGENT_SET_AGING_TIME                         "config_aging_time"
#define SE_AGENT_SHOW_AGING_TIME						"show_aging_time"
#define SE_AGENT_READ_REG								"se_agent_read_reg"
#define SE_AGENT_WRITE_REG							    "se_agent_write_reg"
#define SE_AGENT_SHOW_FAU64                             "show_fau64"
#define SE_AGENT_CLEAR_FAU64							"clear_fau64"
#define SE_AGENT_ICMP_ENABLE                            "config_icmp_enable"
#define SE_AGENT_PPPOE_ENABLE                           "config_pppoe_enable"
#define SE_AGENT_PURE_IP_ENABLE                         "config_pure_ip_enable"
#define SE_AGENT_SHOW_PURE_IP_ENABLE				    "show_pure_ip_enable"
#define SE_AGENT_PURE_IPV6_ENABLE                       "config_pure_ipv6_enable"
#define SE_AGENT_SHOW_PURE_IPV6_ENABLE				    "show_pure_ipv6_enable"
#define SE_AGENT_IPV6_ENABLE                         	"config_ipv6_enable"
#define SE_AGENT_SHOW_IPV6_ENABLE				    	"show_ipv6_enable"
#define SE_AGENT_FASTFWD_ENABLE							"config_fastfwd_enable"
#define SE_AGENT_DELETE_RULE                            "delete_specified_rule"
#define SE_AGENT_SHOW_FIVE_TUPLE_ACL                    "show_specified_rule"
#define SE_AGENT_SHOW_ACL_STATS                         "show_rule_statistic"
#define SE_AGENT_CLEAR_RULE_ALL                         "clear_rule_all"
#define SE_AGENT_SHOW_CAPWAP                            "show_capwap_table"
#define SE_AGENT_CLEAR_AGING_RULE                       "clear_aging_rules"
#define SE_AGENT_SHOW_AGING_RULE_CNT                    "show_aging_rule_cnt"
#define SE_AGENT_SHOW_USER_ACL                          "show_user_rule_stats"
#define SE_AGENT_SHOW_ACL_LEARNED                       "show_learned_rule"
#define SE_AGENT_SHOW_ACL_LEARNING                      "show_learning_rule"
#define SE_AGENT_SHOW_TOLINUX_FLOW                      "show_tolinux_flow"
#define SE_AGENT_SHOW_BUCKET_ENTRY                      "show_bucket_entry"
#define SE_AGENT_SET_BUCKET_ENTRY			            "config_bucket_entry"
#define SE_AGENT_SHOW_FASTFWD_MEM                       "show_fastfwd_memory"

#define SE_AGENT_USER_ONLINE                            "user_online"
#define SE_AGENT_USER_OFFLINE                           "user_offline"
#define SE_AGENT_GET_USER_FLOWS                         "get_user_flow_statistics"
#define SE_AGENT_SHOW_ETH_FAU64                     	"show_eth_fau64"
#define SE_AGENT_SHOW_CAPWAP_FAU64                  	"show_capwap_fau64"
#define SE_AGENT_SHOW_RPA_FAU64                     	"show_rpa_fau64"
#define SE_AGENT_SHOW_PART_FAU64                        "show_part_fau64"
#define SE_AGENT_CLEAR_PART_FAU64                       "clear_part_fau64"

#define SE_AGENT_CONFIG_PURE_PAYLOAD_ACCT               "config_pure_payload_acct"
#define SE_AGENT_CONFIG_TRAFFIC_MONITOR                 "config_traffic_monitor"
#define SE_AGENT_CLEAR_TRAFFIC_MONITOR                  "clear_traffic_monitor"
#define SE_AGENT_SHOW_TRAFFIC_MONITOR                   "show_traffic_monitor"
#define SE_AGENT_CLEAR_RULE_IP                          "clear_rule_ip" 
#define SE_AGENT_SHOW_FAST_FWD_INFO                     "show fast_fwd_info"   
#define SE_AGENT_SHOW_RULE_IP                           "show_rule_ip"      
#define SE_AGENT_SHOW_RULE_IPV6                         "show_rule_ipv6"  
#define SE_AGENT_CONFIG_FWDLOG_ENABLE                   "config_fwdlog_enable"  
#define SE_AGENT_SHOW_FWDLOG_ENABLE						"show_fwdlog_enable"
#define SE_AGENT_CONFIG_FWDLOG_LEVEL                    "config_fwdlog_level"
#define SE_AGENT_SHOW_FWDLOG_LEVEL                      "show_fwdlog_level"
#define SE_AGENT_CLEAR_FWDLOG                   		"clear_fwdlog"

#define SE_AGENT_EQUIPMENT_TEST_ENABLE                 	"equipment_test_enable"
#define SE_AGENT_SHOW_FWD_USER_STATS                    "show_fastfwd_user_stats"
#define SE_AGENT_SHOW_FWD_USER_RULE_ALL                 "show_fastfwd_user_rule_all"
#define SE_AGENT_SET_CLEAR_AGED_RULE_TIME               "set_clear_aged_rule_time"
#define SE_AGENT_GET_CLEAR_AGED_RULE_TIME               "get_clear_aged_rule_time"
#define SE_AGENT_SHOW_USER_RULE_BY_IP                   "show_user_rule_by_ip"
#endif

