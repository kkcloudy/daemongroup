#ifndef _ZEBRA_IF_FLOW_STATS_H
#define _ZEBRA_IF_FLOW_STATS_H

/*gujd: 2013-05-29, pm 2:12. Add for interface flow statistics . */
typedef struct process_info
{
  /* Socket to zebra daemon. */
  int sock;

  /*process info*/
  int name_type;

  /* Connection failure count. */
  int fail;

  /* Input buffer for message. */
  struct stream *ibuf;
  /* Output buffer for message. */
  struct stream *obuf;

  /* Buffer of data waiting to be written to zebra. */
  struct buffer *wb;

  /* Read and connect thread. */
  struct thread *t_read;
  struct thread *t_suicide;

  /* Thread to write buffered data to zebra. */
  struct thread *t_write;
  
  /* Thread to timer event to zebra. zhaocg add*/
  struct thread *t_timer;

}process_info;

enum if_flow_events 
{ 
	IF_FLOW_TIPC_CLIENT_SCHEDULE,  /*for se_agent*/
	IF_FLOW_TIPC_CLIENT_READ,      /*for se_agent*/
	IF_FLOW_TIPC_CLIENT_TIMER,	   /*for se_agent zhaocg add*/
	IF_FLOW_UNIX_SERVER_ACCEPT_SNMP,    /*for snmp or acsample*/
/*	IF_FLOW_UNIX_SERVER_ACCEPT_ACSAMPLE,*/	/*for snmp or acsample*/
/*	IF_FLOW_UNIX_SERVER_ACCEPT,*/
	IF_FLOW_UNIX_SERVER_READ_SNMP,       /*for snmp or acsample*/
/*	IF_FLOW_UNIX_SERVER_READ_ACSAMPLE*/	   /*for snmp or acsample*/
};

/*gujd: 2013-05-29, pm 2:12. Add for interface flow statistics . Rtmd to Se_agent use TIPC.*/
#define SE_AGENT_SERVER_TYPE        0x4000
#define SE_AGENT_SERVER_LOWER_INST  1000  /*inst + local board id*/

#define SE_AGENT_SHOW_PART_FAU64                        "show_part_fau64"

/*gujd: 2013-05-30, pm 5:45. Add for interface flow statistics . Rtmd to snmp or acsample use UNIX socket.*/
#define RTM_TO_SNMP_PATH 			"/var/run/rtm_snmp_path"
/*#define RTM_TO_ACSAMPLE_PATH 		"/var/run/rtm_acsample_path"*/

typedef struct if_flow_stats
{
	int tap;
	int length;
	int process_name;
	int cmd;
	char name[INTERFACE_NAMSIZ + 1];
	struct if_stats stats;
		
	
}if_flow_stats;

#if 0/*move to zebra.h*/
/*cmd*/
#define INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED		1
#define INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED			2
#define INTERFACE_FLOW_STATISTICS_DATA						3
#endif
/*process NAME*/
#define PROCESS_NAME_SE_AGENT				0x1
#define PROCESS_NAME_SNMP					0x2
#define PROCESS_NAME_ACSAMPLE				0x3
#define PROCESS_NAME_RTM					0x4

//#define IF_FLOW_STATS_HEADER_SIZE             12
#define IF_FLOW_STATS_HEADER_SIZE             16


typedef struct 
{
	int64_t output_packets_eth;   /*64-bit counter used for total ethernet output eth packets*/
	int64_t output_packets_capwap;   /*64-bit counter used for total ethernet output capwap packets*/
	int64_t output_packets_rpa;   /*64-bit counter used for total ethernet output rpa packets*/
	int64_t input_packets_eth;   /*64-bit counter used for total ethernet input eth packets*/
	int64_t input_packets_capwap;   /*64-bit counter used for total ethernet input capwap packets*/
	int64_t input_packets_rpa;   /*64-bit counter used for total ethernet input rpa packets*/
	int64_t output_bytes_eth;   /*64-bit counter used for total ethernet output eth bytes*/
	int64_t output_bytes_capwap;   /*64-bit counter used for total ethernet output capwap bytes*/
	int64_t output_bytes_rpa;   /*64-bit counter used for total ethernet output rpa bytes*/
	int64_t input_bytes_eth;   /*64-bit counter used for total ethernet input eth bytes*/
	int64_t input_bytes_capwap;   /*64-bit counter used for total ethernet input capwap bytes*/
	int64_t input_bytes_rpa;   /*64-bit counter used for total ethernet input rpa bytes*/

}if_flow_stats_se_agent;



/*For fast foward*/
#if 1/*begin*/

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


typedef union
{
	fau64_info_t    fau64_info;
	fau64_part_info_t fau64_part_info;
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

#define HAND_CMD_SIZE  64   
#define ERR_INFO_SIZE  128	/*The length of the error information*/

typedef struct se_interative_s
{
	char hand_cmd[HAND_CMD_SIZE];
	control_cmd_t  fccp_cmd;
	int cmd_result;
	int cpu_tag;
	char err_info[ERR_INFO_SIZE];/*Error message is returned by se_agent*/	
}se_interative_t;

#endif/*end*/


#endif /* _ZEBRA_IF_FLOW_STATS_H */

