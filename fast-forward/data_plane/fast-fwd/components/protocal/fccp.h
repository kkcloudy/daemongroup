#ifndef _FCCP_H
#define _FCCP_H

#include "acl.h"
#include "capwap.h"

/*
* Fast Command Control Protocol£¨FCCP£©definition
*/
#define FCCP_WQE_TYPE  0x85     /* add by zhaohan */
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
#define FCCP_CMD_INSERT 1
#define FCCP_CMD_DELETE 2
#define FCCP_CMD_SET_AGEING_TIMER 3
#define FCCP_CMD_GET_AGEING_TIMER 4
#define FCCP_CMD_RULE_CNT 5
#define FCCP_CMD_INSERT_ICMP 6
#define FCCP_CMD_ENABLE_ICMP 7
#define FCCP_CMD_GET_ICMP_STATE 8
#define FCCP_CMD_GET_RULE_STATS 9
#define FCCP_CMD_CLEAR_RULE 10
#define FCCP_CMD_CLEAR_AGING_RULE 11
#define FCCP_CMD_SET_BUCKET_MAX_ENTRY 12
#define FCCP_CMD_GET_BUCKET_MAX_ENTRY 13
#define FCCP_CMD_ENABLE_TRAFFIC_MONITOR  14
#define FCCP_CMD_CLEAR_TRAFFIC_MONITOR  15
#define FCCP_CMD_GET_TRAFFIC_MONITOR   16
#define FCCP_CMD_USER_ONLINE 17
#define FCCP_CMD_USER_OFFLINE 18
#define FCCP_CMD_USER_STATS_CLEAR 19
#define FCCP_CMD_ENABLE_PURE_PAYLOAD_ACCT 20
#define FCCP_CMD_DEL_RULE_BY_IP 21
#define FCCP_CMD_TEST 22                /*wangjian 2012.07.09 add fwd info */
#define FCCP_CMD_SHOW_RULE_BY_IP 23     /*wangjian 2012.07.09 add ip */
#define FCCP_CMD_EQUIPMENT_TEST 24
#define FCCP_CMD_ENABLE_LOG 25          /* fast-fwd group add for  logsave */      
#define FCCP_CMD_TIMESYNC 26
#define FCCP_CMD_ENABLE_LOG_SHOW 27
#define FCCP_CMD_SET_LOG_LEVEL 28
#define FCCP_CMD_SHOW_LOG_LEVEL 29







#define FCCP_CMD_CONFIG_TAG_TYPE 37
#define FCCP_CMD_GET_TAG_TYPE 38
#define FCCP_CMD_SHOW_FAU64 39
#define FCCP_CMD_CLEAR_FAU64 40
#define FCCP_CMD_SHOW_FPA_BUFF 41


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

typedef struct
{
    uint32_t enable;
    uint32_t board_type;
}equipment_test_cntl_t;

/* fccp return code */
#define FCCP_RETURN_OK          0x12340000
#define FCCP_RETURN_ERROR     0x12340001  





typedef union{
    /* rule learn */
    struct{
        rule_param_t rule_param;
        capwap_cache_t cw_cache;
    }rule_info;

    /* 5 tuple */
    struct{
        uint32_t ip_src; 
        uint32_t ip_dst; 
        uint16_t th_sport; 
        uint16_t th_dport; 
        uint8_t ip_p;
    }rule_tuple;
    
    rule_cnt_info_t rule_cnt_info;
    uint32_t aging_timer;
	uint32_t share;
    struct{
        uint32_t fast_fwd_enable;   /*fast forward enable wangjian*/
        uint32_t fast_fwd_coremask; /*fast forward occupy core */
    }fast_fwd_info; 
    uint32_t module_enable;
	uint64_t time_sec;   /*linux sec sendto fwd*/
    rule_stats_t rule_stats;
    uint32_t bucket_max_entry;
    uint32_t tag_type;
    /* user info */
    user_info_t user_info;
	traffic_stats_t traffic_stats;
    equipment_test_cntl_t equipment_test;
    fau64_info_t fau64_info;
    uint32_t pool_buff_count[8];
}fccp_data_t;

typedef struct  control_cmd_s{   
    uint16_t dest_module; 
    uint16_t src_module; 
    uint16_t cmd_opcode; 
    uint16_t cmd_len; 	
    uint32_t agent_pid;
    uint32_t ret_val;
    fccp_data_t fccp_data;
}control_cmd_t;


#endif
