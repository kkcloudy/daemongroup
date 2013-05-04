/***********************license start************************************
 * File name: fwd_main.c 
 * Auther     : lutao
 * 
 * Copyright (c) Autelan . All rights reserved.
 * 
 **********************license end**************************************/
#ifndef  _ACL_H_
#define   _ACL_H_
#include <stdint.h>
#include <stdio.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-dfa.h"
#include "cvmx-pow.h"
#include "cvmx-gmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-defs.h"
#include "fastfwd-common-rnd.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-defs.h"
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-fpa.h"
#endif
#include "autelan_product_info.h"
#include "capwap.h"	
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"

#include "shell.h"

#include "cvmx-rwlock.h"


#define ACL_TBL_RULE_NAME "acl_tbl_rule"
#define ACL_DYNAMIC_TBL_RULE_NAME   "acl_dynamic_tbl_rule"
#define USER_TBL_RULE_NAME "user_tbl"
#define USER_DYNAMIC_TBL_RULE_NAME "user_dynamic_tbl"
#define BASE_MAC_TABLE_NAME     "basemac_table"



/*contain the packet fast parse result*/
typedef enum { 	
	PREPARSE_ACTION_TO_ACL = 1,
	PREPARSE_ACTION_TO_LINUX = 2,
	PREPARSE_ACTION_TO_ICMP = 3,
	PREPARSE_ACTION_TO_DROP = 4,
	PREPARSE_ACTION_INVALIDE 
} parse_scheme_action_t;

#define RULE_IS_EMPTY				0
#define RULE_IS_LEARNED			1
#define RULE_IS_LEARNING			2
#define RULE_IS_STATIC				3 /*由控制面直接下发的表项，为静态表项，不老化*/
#define RULE_IS_CMD_SET			4 /*由控制面直接下发的表项*/
#define RULE_IS_INVALID				0xff

#define USER_IS_EMPTY		0
#define USER_IS_ONLINE		1 /*用户上线*/
#define USER_IS_OFFLINE		2 /*用户下线*/

#define DIRECTION_UP		      1
#define DIRECTION_DOWN		      2
#define DIRECTION_UP_DOWN		3   /* use to user */


/*
   1. 流表都以user index进行比较删除或者统计更新，se-agent负责把hash表转
   user index, 这里的思想是采用静态内存分配，对于冲突直接步进，
   不采用链表，优点是可以一次命中，缺点是浪费内存。
   HASH算法采用cvm_ip_cache_bucket_lookup。Se-agent在自学习时，把用户的
   索引下发给转发面，转发面更新流表。

   2. the user table struct contain the information about the user like the counters, meter information, etc.

   3. 因冲突，直接覆盖用户表时，要删除原有表对应的流表
   用户下线触发对应流表的老化.

 */
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

	/*uint64_t	begin_time_stamp;*/
} user_info_t;

/********************************************************
 *	user tables
 *********************************************************/
typedef struct  user_item_s{
	struct user_item_s *next;
	struct user_info_s user_info;
	//cvmx_spinlock_t      lock; /*only the first bucket lock is used*/
	cvmx_rwlock_wp_lock_t lock;
	uint16_t valid_entries;  /*only the first bucket valid_entries is used for the number of rule entry*/
	uint8_t reserved[2];	
}user_item_t;



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
	volatile uint8_t  packet_wait; /*before acl learned successful, the number of packets forwarded to the linux*/
	/* maybe one rule belong to 2 users, e.g. sending a file between two users. add by zhaohan */
	uint32_t user2_index; /*identify the user2 index */	
	uint16_t user2_link_index; /*identify the user2 link index */

    /* nat related */
    uint16_t nat_flag; /*wangjian_nat*/
    uint32_t nat_sip;
    uint32_t nat_dip;
    uint16_t nat_sport;
    uint16_t nat_dport;

	/*pppoe related*/
	uint16_t pppoe_flag;                   
	uint16_t pppoe_session_id;
}rule_param_t ;


typedef struct work_node_s
{	
	cvmx_wqe_t *work;
	struct work_node_s *next;
}work_node_t;

typedef struct extend_link_s
{	
	/*报文缓存链表*/
	work_node_t *head_link;
	work_node_t *tail_link;
	int link_node_num;

	/*For the future...*/

}extend_link_t;

/********************************************************
 *	acl rule tables ,one bucket contains one rule for now, 128Bytes
 *********************************************************/
typedef struct  rule_item_s{
	struct rule_item_s *next;
	struct rule_param_s rules;
	uint16_t valid_entries;  /*only the first bucket valid_entries is used for the number of rule entry*/
	cvmx_spinlock_t      lock; /*only the first bucket lock is used*/
	//uint8_t reserved[0];	
}rule_item_t;



/************************************************
 *	acl rule statistics, add by zhaohan
 *     
 *************************************************/
#define atomic_add32_nosync(v) cvmx_atomic_add32_nosync((v), 1)      // +1 (32bit)
#define atomic_dec32_nosync(v) cvmx_atomic_add32_nosync((v), -1)    //  -1 (32bit)

#define atomic_add64_nosync(v) cvmx_atomic_add64_nosync((v), 1)     // +1 (64bit)
#define atomic_dec64_nosync(v) cvmx_atomic_add64_nosync((v), -1)    // -1 (64bit)

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

typedef struct rule_cnt_info_s{
	int64_t rule_age_cnt;
	int64_t rule_hit_cnt;
	int64_t rule_miss_cnt;
	int64_t over_max_entry_cnt;
	int64_t extern_cnt;    

	/* forward count */
	int64_t fwd_eth_cnt;
	int64_t fwd_cw_cnt;
	int64_t fwd_cw_802_3_cnt;

	int64_t dynamic_rule_cnt;   // dynamic rule alloc by cvmx_malloc()
} rule_cnt_info_t;


/************************************************
 *	five_tuple, add by zhaohan
 *     
 *************************************************/
typedef struct
{
	uint32_t ip_src; 
	uint32_t ip_dst; 
	uint16_t th_sport; 
	uint16_t th_dport; 
	uint8_t ip_p;
} five_tuple_t;


/************************************************
 *	fccp format define
 *************************************************/



/************************************************
 *	flow action mask bit define, these action can coexist 
 *     so use mask
 *************************************************/
#define FLOW_ACTION_METER (0x1)
#define FLOW_ACTION_NAT (0x2)

/************************************************
 *	flow action type define
 *************************************************/
#define FLOW_ACTION_DROP                    1
#define FLOW_ACTION_TOLINUX                 2
#define FLOW_ACTION_ICMP                    3
#define FLOW_ACTION_CAPWAP_802_11_ICMP      4
#define FLOW_ACTION_ETH_FORWARD             5
#define FLOW_ACTION_CAPWAP_FORWARD          6
#define FLOW_ACTION_CAP802_3_FORWARD        7
#define FLOW_ACTION_CAPWAP_802_3_ICMP       8

#define FLOW_ACTION_RPA_ETH_FORWARD         9
#define FLOW_ACTION_RPA_CAPWAP_FORWARD      10
#define FLOW_ACTION_RPA_CAP802_3_FORWARD    11
#define FLOW_ACTION_RPA_CAPWAP_802_3_ICMP   12
#define FLOW_ACTION_RPA_CAPWAP_802_11_ICMP  13
#define FLOW_ACTION_RPA_ICMP                14
#define FLOW_ACTION_RPA_DIRECT_FORWARD      15
#define FLOW_ACTION_HEADER_FRAGMENTS_STORE  16

/********************************************************************/

extern CVMX_SHARED capwap_cache_t *capwap_cache_bl;
extern CVMX_SHARED cvmx_arena_list_t   rule_arena;
extern CVMX_SHARED rule_item_t  *acl_bucket_tbl;
extern CVMX_SHARED uint32_t acl_static_tbl_size;
extern CVMX_SHARED uint32_t acl_dynamic_tbl_size;
extern CVMX_SHARED uint32_t acl_aging_timer;
extern CVMX_SHARED rule_cnt_info_t rule_cnt_info;   /* add by zhaohan*/
extern CVMX_SHARED cvmx_spinlock_t capwap_cache_bl_lock; /* add by zhaohan */
extern CVMX_SHARED uint32_t acl_aging_enable;
extern CVMX_SHARED uint32_t acl_bucket_max_entries;
extern CVMX_SHARED cvmx_arena_list_t   user_table_arena;
extern CVMX_SHARED int cvm_pure_payload_acct;


/*******************************************************************/
#ifdef USER_TABLE_FUNCTION	
static int acl_index_age_num=0;
extern CVMX_SHARED uint64_t *pfast_acl_mask_tbl;
extern CVMX_SHARED user_item_t *user_bucket_tbl;
extern CVMX_SHARED uint32_t user_static_tbl_size;
extern CVMX_SHARED cvmx_spinlock_t  acl_mask_lock;
#define ACL_FAST_TBL_MASK_NAME "acl_fast_tbl mask"

#define FREE_ACL_BUCKET 2
#define FREE_ACL_ENTRY   1

/*设置ACL掩码表的对应位*/
static inline void set_acl_mask(rule_item_t *prule_addr )
{
	uint64_t  index ;
	uint64_t mask_group;

	index=(prule_addr - acl_bucket_tbl);
	cvmx_spinlock_lock(&acl_mask_lock);
	mask_group = *(pfast_acl_mask_tbl+(index/64));
	*(pfast_acl_mask_tbl+(index/64)) = mask_group | (1ull<<(index%64)) ;
	cvmx_spinlock_unlock(&acl_mask_lock);

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"set aclmask age: *(pfast_acl_mask_tbl+(index/64)) is 0x%lx ,pfast_acl_mask_tbl+(index/64) is 0x%p \r\n" , *(pfast_acl_mask_tbl+(index/64) ),(pfast_acl_mask_tbl+(index/64)));

}

/* 清除hash 索引的mask掩码*/
static inline void clr_acl_mask(rule_item_t *prule_addr )
{
	uint32_t  index ;
	uint64_t mask_group;

	index=(prule_addr - acl_bucket_tbl); ;
	cvmx_spinlock_lock(&acl_mask_lock);
	mask_group =*(pfast_acl_mask_tbl+(index/64));
	*(pfast_acl_mask_tbl+(index/64))=mask_group&( ~(1ull<<(index%64))) ;
	cvmx_spinlock_unlock(&acl_mask_lock);

	//FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
	//		"clr aclmask age ,the addr is %p ,mask group is 0x%lx  \n" , prule_addr,mask_group);
}

/*
prule_hash_head	:当前删除entry对应的HASH表位置,即头指针
p_entry_del_pre	:指向当前删除entry对应的bucket指针的前指针
p_entry_del_cur	:当前删除entry对应的bucket指针
 */
static inline int delete_fast_rule_by_addr( rule_item_t *head_rule ,rule_item_t *del_rule_pre ,  rule_item_t *del_rule_cur)
{
	uint32_t tmp = 0;
	uint64_t *ptmp = NULL;

	if((head_rule == NULL) || (del_rule_pre == NULL) || (del_rule_cur == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"delete_fast_rule_by_addr: rule is NULL pointer! \r\n");

		return RETURN_ERROR;
	}

	ptmp = (uint64_t *)(&(head_rule->rules));

	/* capwap table */
	if((del_rule_cur->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
			(del_rule_cur->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD))
	{
		if(capwap_cache_bl[del_rule_cur->rules.tunnel_index].use_num > 0)
		{
			/* reduce use_num */
			atomic_dec32_nosync(&(capwap_cache_bl[del_rule_cur->rules.tunnel_index].use_num));
		}
		else
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"delete_rule_by_aging: capwap use_num == 0! can not reach here! \r\n");
		}
	}

	/*The first bucket need to be aging delete, only zero the rule param, modify by zhaohan */
	if(head_rule == del_rule_cur)
	{
		while(tmp < sizeof(rule_param_t)/8) /*112 Bytes*/
		{
			*ptmp = 0;
			ptmp = ptmp + 1;
			tmp++;
		}
		head_rule->valid_entries--;
		if((!(head_rule->next))&&(head_rule->valid_entries==0))
		{
			clr_acl_mask(head_rule);                                          
		}		
		return FREE_ACL_ENTRY;	
	}
	else
	{
		if(del_rule_cur->next != NULL) /*队中*/
			del_rule_pre->next = del_rule_cur->next;
		else /*队尾*/
			del_rule_pre->next = NULL;

		cvmx_free(del_rule_cur);
		del_rule_cur = NULL;
		head_rule->valid_entries--;
		atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1 */
		if((!(head_rule->next))&&(head_rule->valid_entries==0))
		{
			clr_acl_mask(head_rule);                                          
		}		
		return FREE_ACL_BUCKET;	
	}
}

/*
 * delete the acl rule by user index
 * bitmask could be used for the performance, lutao tag
 */
static inline void del_fast_by_user_index(uint32_t user_index, uint16_t user_link_index)
{
	uint64_t *ptr_mask_64;
	uint64_t loop,loop_1;
	rule_item_t *p_age_index ; /*页的基址*/
	rule_item_t	*p_pre_age_index; /*除页首地址外，需要删除元素的前地址，用来连接链表；*/
	uint32_t num,check_last_entry=0;
	int age_entry_num=0, free_entry_flag=0;
	rule_item_t  *p_loop_addr , *p_base_addr;
	ptr_mask_64 = pfast_acl_mask_tbl;
	p_loop_addr = acl_bucket_tbl;


	if((acl_static_tbl_size%64)!=0)
	{
		num = acl_static_tbl_size/64+1;
	}
	else
	{
		num = acl_static_tbl_size/64;
	}

	for(loop=0;loop < num;loop++)
	{
		/*选acl hash base  addr ,是64位图的变量地址*/
		p_loop_addr=acl_bucket_tbl + loop*64;

		for(loop_1 = 0; loop_1 < 64; loop_1++)
		{
			if(((*(ptr_mask_64+loop))&(1ull<<loop_1))!=0)
			{
				/* 这个地址的索引有可能需要老化的快表；
				   选位图赋值为1的地址*/
				p_age_index= p_loop_addr+loop_1;
				p_base_addr=p_age_index;

				/*保存这个hash链的前一个指针*/
				p_pre_age_index=p_age_index;
				age_entry_num=0;
				check_last_entry=0;

				cvmx_spinlock_lock(&p_base_addr->lock);
				while(1)
				{   
					free_entry_flag=0;							  

					if(((p_age_index->rules.user_index == user_index) && 
								(p_age_index->rules.user_link_index == user_link_index)) ||
							((p_age_index->rules.direct_flag == DIRECTION_UP_DOWN) && 
							 (p_age_index->rules.user2_index == user_index) && 
							 (p_age_index->rules.user2_link_index == user_link_index))
					  )
					{							
						/*删除一个表项，可能还有需要删除其他的，所以不能返回;*/
						if (delete_fast_rule_by_addr(p_base_addr,p_pre_age_index ,p_age_index) == FREE_ACL_BUCKET)
						{								      	    	          
							free_entry_flag = 1;										    
						}
						acl_index_age_num++;		
					}

					if(check_last_entry==1)
					{                                    
						break;
					}

					/* 只有释放了entry,前一个指针才不需要移动；如果p_age_index不释放资源，
					   p_pre_age_index要移动 如果 p_age_index释放了，则必须由p_pre_age_index得到*/
					if(free_entry_flag==1)
					{

						if(p_pre_age_index!=NULL)
						{
							/*p_age_index 被释放了，所以必须由前指针告知*/
							p_age_index=p_pre_age_index->next;
						}
						else
						{
							FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
									"p_pre_age_index is NULL ! -- so shouldn''t come here \n ");						
							break;
						}

						/*释放的是最后entry ，可结束循环了*/
						if(p_pre_age_index->next==NULL)
						{
							break;
						}					
					}
					else
						/*没有释放，则p_pre_age_index ，p_age_index 需要更新*/
					{ 
						p_pre_age_index=p_age_index;

						if((p_age_index->next==NULL))
						{
							check_last_entry=1;	 						          
						}
						else
						{
							p_age_index=p_age_index->next;
						}
					}
				}    
				cvmx_spinlock_unlock(&p_base_addr->lock);
			}
		}	    
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"del_fast_acl_index : delete fast rule is 0x%x \n " , acl_index_age_num);
}

/*
 * delete the acl rule by ip address
 * bitmask could be used for the performance, lutao tag
 * add from 1.3.18 by wangjian
 */
static inline void fast_del_rule_by_ip(uint32_t ip)
{
	uint64_t *ptr_mask_64;
	uint64_t loop,loop_1;
	rule_item_t *p_age_index ; /*页的基址*/
	rule_item_t	*p_pre_age_index; /*除页首地址外，需要删除元素的前地址，用来连接链表；*/
	uint32_t num,check_last_entry=0;
	int age_entry_num=0, free_entry_flag=0;
	rule_item_t  *p_loop_addr , *p_base_addr;
	ptr_mask_64 = pfast_acl_mask_tbl;
	p_loop_addr = acl_bucket_tbl;

	if((acl_static_tbl_size%64)!=0)
	{
		num = acl_static_tbl_size/64+1;
	}
	else
	{
		num = acl_static_tbl_size/64;
	}

	for(loop=0;loop < num;loop++)
	{
		/*选acl hash base  addr ,是64位图的变量地址*/
		p_loop_addr=acl_bucket_tbl + loop*64;

		for(loop_1 = 0; loop_1 < 64; loop_1++)
		{
			if(((*(ptr_mask_64+loop))&(1ull<<loop_1))!=0)
			{
				/* 这个地址的索引有可能需要老化的快表；
				   选位图赋值为1的地址*/
				p_age_index= p_loop_addr+loop_1;
				p_base_addr=p_age_index;

				/*保存这个hash链的前一个指针*/
				p_pre_age_index=p_age_index;
				age_entry_num=0;
				check_last_entry=0;
            
				cvmx_spinlock_lock(&p_base_addr->lock);
				while(1)
				{   
					free_entry_flag=0;							  

					if((p_age_index->rules.sip == ip) || (p_age_index->rules.dip == ip))
					{							
						/*删除一个表项，可能还有需要删除其他的，所以不能返回;*/
						if (delete_fast_rule_by_addr(p_base_addr,p_pre_age_index ,p_age_index) == FREE_ACL_BUCKET)
						{								      	    	          
							free_entry_flag = 1;										    
						}
						acl_index_age_num++;		
					}

					if(check_last_entry==1)
					{                                    
						break;
					}

					/* 只有释放了entry,前一个指针才不需要移动；如果p_age_index不释放资源，
					   p_pre_age_index要移动 如果 p_age_index释放了，则必须由p_pre_age_index得到*/
					if(free_entry_flag==1)
					{

						if(p_pre_age_index!=NULL)
						{
							/*p_age_index 被释放了，所以必须由前指针告知*/
							p_age_index=p_pre_age_index->next;
						}
						else
						{
							FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
									"p_pre_age_index is NULL ! -- so shouldn''t come here. \n ");						
							break;
						}

						/*释放的是最后entry ，可结束循环了*/
						if(p_pre_age_index->next==NULL)
						{
							break;
						}					
					}
					else
						/*没有释放，则p_pre_age_index ，p_age_index 需要更新*/
					{ 
						p_pre_age_index=p_age_index;

						if((p_age_index->next==NULL))
						{
							check_last_entry=1;	 						          
						}
						else
						{
							p_age_index=p_age_index->next;
						}
					}
				}    
				cvmx_spinlock_unlock(&p_base_addr->lock);
			}
		}	    
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"del_fast_acl_index : delete fast rule is 0x%x \n " , acl_index_age_num);
}

/* get user info by user_index and user_link_index */
/* this function must return quickly */
/* add by zhaohan */
static inline user_item_t * get_user_item(uint32_t usr_idx, uint16_t link_idx)
{
	uint32_t i = 0;
	user_item_t* user = NULL;

	if(cvmx_unlikely(usr_idx > user_static_tbl_size - 1))
		return NULL;

	user = &user_bucket_tbl[usr_idx];

	for(i = 0; i < link_idx; i++)
	{
		user = user->next;
		if(cvmx_unlikely(user == NULL))
			return NULL;
	}

	return user;
}

#endif
/****************************************************************/

static inline uint64_t get_msec()
{
	uint64_t msec;
	msec= (cvmx_get_cycle() / (cvmx_sysinfo_get()->cpu_clock_hz / 1000));
	return msec  ;
}

static inline uint64_t get_sec()
{
	uint64_t sec;
	sec= (uint64_t ) (cvmx_clock_get_count(CVMX_CLOCK_SCLK) / cvmx_sysinfo_get()->cpu_clock_hz);
	return sec  ;
}

/*
static inline uint64_t get_global_sec()
{
	uint64_t sec;
	sec= (uint64_t ) (cvmx_get_cycle_global() / cvmx_sysinfo_get()->cpu_clock_hz);
	return sec  ;
}
*/
static inline rule_item_t* cvm_ip_hash_lookup (uint32_t ip)
{
	rule_item_t *bucket;
	uint64_t idx;

#if defined(__KERNEL__) && defined(linux)
	set_c0_status(ST0_CU2);
#endif
	CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
	CVMX_MT_CRC_IV(0);
	CVMX_MT_CRC_WORD(*(uint32_t*)(&ip));
	CVMX_MF_CRC_IV(idx);
	idx &= (acl_static_tbl_size - 1);

	bucket = &acl_bucket_tbl[idx];
	cvmx_scratch_write64(CVM_SCR_ACL_CACHE_PTR, (uint64_t) (CAST64(bucket)));
	return (bucket);
}

static inline rule_item_t* cvm_three_tupe_hash_lookup (uint32_t dip, uint32_t sip, uint8_t proto)
{
	rule_item_t *bucket;
	uint64_t idx;

#if defined(__KERNEL__) && defined(linux)
	set_c0_status(ST0_CU2);
#endif
	CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
	CVMX_MT_CRC_IV(0);
	CVMX_MT_CRC_WORD(*(uint32_t*)(&dip));
	CVMX_MT_CRC_WORD(*(uint32_t*)(&sip));
	CVMX_MT_CRC_BYTE(*(uint8_t*)(&proto));
	CVMX_MF_CRC_IV(idx);
	idx &= (acl_static_tbl_size - 1);

	bucket = &acl_bucket_tbl[idx];
	cvmx_scratch_write64(CVM_SCR_ACL_CACHE_PTR, (uint64_t) (CAST64(bucket)));
	return (bucket);
}

/* hash function */
static inline rule_item_t * hash(uint32_t dip, uint32_t sip, uint8_t proto, uint16_t dport, uint16_t sport)
{
	rule_item_t *bucket;
	uint64_t  result = 0;

	CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
	CVMX_MT_CRC_IV(0);
	CVMX_MT_CRC_WORD(dip);
	CVMX_MT_CRC_WORD(sip);
	CVMX_MT_CRC_HALF(dport);
	CVMX_MT_CRC_HALF(sport);
	CVMX_MT_CRC_BYTE(proto);
	CVMX_MF_CRC_IV(result);

	result &= (acl_static_tbl_size - 1);

	/*Save bucket address in the scratch memory.*/
	bucket = &acl_bucket_tbl[result];	
	cvmx_scratch_write64(CVM_SCR_ACL_CACHE_PTR, (uint64_t) (CAST64(bucket)));
	return (bucket);
}

/*
   区分上下行，分别使用SIP/DIP获取用户表的索引
 */
static inline uint32_t user_cache_bucket_lookup (uint32_t ip_address)
{
	user_item_t *bucket;
	uint64_t idx;

#if defined(__KERNEL__) && defined(linux)
	set_c0_status(ST0_CU2);
#endif
	CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
	CVMX_MT_CRC_IV(0);
	CVMX_MT_CRC_WORD(*(uint32_t*)(&ip_address));
	CVMX_MF_CRC_IV(idx);
	idx &= (user_static_tbl_size - 1);

	bucket = &user_bucket_tbl[idx];
	cvmx_scratch_write64(CVM_SCR_USER_CACHE_PTR, (uint64_t) (CAST64(bucket)));
	return (idx);
}
/********************************************************************/
#define MAX_AGING_TIMER     0xffffffff
#define MIN_AGING_TIMER     1

static inline int acl_set_aging_timer(uint32_t aging_timer)
{
	if((aging_timer > MAX_AGING_TIMER) || (aging_timer < MIN_AGING_TIMER))
	{
		return RETURN_ERROR;
	}

	acl_aging_timer = aging_timer;

	return RETURN_OK;
}

static inline uint32_t acl_get_aging_timer()
{
	return acl_aging_timer;
}

static inline int acl_set_bucket_max_entries(uint32_t max_entry)
{
	if(max_entry < 2)
	{
		return RETURN_ERROR;
	}	
	acl_bucket_max_entries = max_entry;
	return RETURN_OK;
}

static inline uint32_t acl_get_bucket_max_entries()
{
	return acl_bucket_max_entries;
}



static inline int acl_aging_check(rule_param_t *check_rule)
{
	uint64_t sec = get_sec();

	if(check_rule->time_stamp > sec) /*反转*/
	{
		/*if((65535-check_rule->timestamp+sec) > FLOW_AGE_TIME)*/
		return 0;			
	}
	else
	{
		if(sec -check_rule->time_stamp > acl_aging_timer)
			return 1;
	}

	return 0;
}

static inline int acl_delete_rule_by_aging( rule_item_t *head_rule ,rule_item_t *del_rule_pre ,  rule_item_t *del_rule_cur)
{
	uint32_t tmp = 0;
	uint64_t *ptmp = NULL;

	if((head_rule == NULL) || (del_rule_pre == NULL) || (del_rule_cur == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"delete_rule_by_aging: rule is NULL pointer! \r\n");

		return RETURN_ERROR;
	}

	ptmp = (uint64_t *)(&(head_rule->rules));

	/* capwap table */
	if((del_rule_cur->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
			(del_rule_cur->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD))
	{
		if(capwap_cache_bl[del_rule_cur->rules.tunnel_index].use_num > 0)
		{
			/* reduce use_num */
			atomic_dec32_nosync(&(capwap_cache_bl[del_rule_cur->rules.tunnel_index].use_num));
		}
		else
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"delete_rule_by_aging: capwap use_num == 0! can not reach here! \r\n");
		}
	}

	/*The first bucket need to be aging delete, only zero the rule param, modify by zhaohan */
	if(head_rule == del_rule_cur)
	{
		while(tmp < sizeof(rule_param_t)/8) /*112 Bytes*/
		{
			*ptmp = 0;
			ptmp = ptmp + 1;
			tmp++;
		}
		head_rule->valid_entries--;
	}
	else
	{
		if(del_rule_cur->next != NULL) /*队中*/
			del_rule_pre->next = del_rule_cur->next;
		else /*队尾*/
			del_rule_pre->next = NULL;

        /* debug trace.add by zhaohan */
        if((head_rule->valid_entries == acl_bucket_max_entries)&&(head_rule->next == NULL))
        {
            printf("Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
        }

		cvmx_free(del_rule_cur);
		del_rule_cur = NULL;
		head_rule->valid_entries--;     /* add by zhaohan */
		atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1, add by zhaohan */
	}

	if((!(head_rule->next))&&(head_rule->valid_entries==0))
	{
		clr_acl_mask(head_rule);                                          
	}

	return RETURN_OK;
}

/*
 * lookup the table, if find, check the state,
 * learned: get the action
 * learning: forward to the Linux
 * if not find, malloc the new entry, and insert, set the state to learning
 * lutao
 */
static inline rule_item_t  * acl_table_lookup(cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th,
		cvmx_spinlock_t  **first_lock)

{    
	rule_item_t  *rule  = NULL;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	rule_item_t  *aging_cur_rule = NULL;
	rule_item_t  *head_rule = NULL;
	rule_item_t  *aging_pre_rule = NULL;
	cvmx_spinlock_t  *head_lock = NULL;

	if ((ip == NULL) || (th == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_table_lookup: NULL Pointer !...\r\n");	
		return NULL;		
	}

	cvmx_fau_atomic_add64(CVM_FAU_TOTAL_ACL_LOOKUP, 1);
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_table_lookup: packet-fiveTuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
			IP_FMT(ip->ip_dst),IP_FMT(ip->ip_src),th->th_dport,th->th_sport,ip->ip_p);

	/*look up ACL Table and get the bucket*/
	hash(ip->ip_dst, ip->ip_src, ip->ip_p, th->th_dport, th->th_sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	aging_cur_rule = rule;
	aging_pre_rule = rule;

	head_lock = &rule->lock;
	*first_lock = head_lock;
	cvmx_spinlock_lock(head_lock);

    /* debug trace.add by zhaohan */
    if((head_rule->valid_entries == acl_bucket_max_entries)&&(head_rule->next == NULL))
    {
        printf("Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
    }

	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			head_rule->rules.time_stamp = get_sec(); 
			head_rule->rules.dip = ip->ip_dst;
			head_rule->rules.sip = ip->ip_src;
			head_rule->rules.protocol= ip->ip_p;
			head_rule->rules.dport= th->th_dport;
			head_rule->rules.sport= th->th_sport;
			head_rule->rules.rule_state = RULE_IS_LEARNING;
			head_rule->rules.action_type = FLOW_ACTION_TOLINUX;
			head_rule->valid_entries++;
			cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
			cvmx_spinlock_unlock(head_lock);

#ifdef USER_TABLE_FUNCTION	
			set_acl_mask(head_rule);
#endif
            
			return rule;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				aging_cur_rule = head_rule->next;
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
	}

	/*aging the current rule first then compare the rule with the key,loop*/			 
	while(1)
	{
		if(rule == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"acl_table_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_spinlock_unlock(head_lock);
			return NULL;
		}

		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_table_lookup: current Rule info: dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
				IP_FMT(rule->rules.dip),IP_FMT(rule->rules.sip),rule->rules.dport,rule->rules.sport,rule->rules.protocol);

		/************************aging first*****************************/
		if((acl_aging_check(&(rule->rules)) > 0) && (rule->rules.rule_state != RULE_IS_STATIC))
		{   
			atomic_add64_nosync(&(rule_cnt_info.rule_age_cnt)); /*rule need to be aging*/
			if(acl_delete_rule_by_aging(head_rule,aging_pre_rule ,aging_cur_rule)==RETURN_OK)
			{
				if(aging_cur_rule == head_rule)     /* add by zhaohan */
				{
					free_rule = head_rule;
				}
				if(aging_pre_rule !=NULL)
				{
					rule=aging_pre_rule->next;
					aging_cur_rule=aging_pre_rule->next;

					if(rule==NULL)/*after age, there is no more bucket*/
					{
						if(free_rule == NULL)
						{
							aging_pre_rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
							if(aging_pre_rule->next == NULL)
							{
								FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, 
										FASTFWD_COMMON_DBG_LVL_ERROR,
										"InsertFastRule: Memory not available for adding rules line=%d\n",__LINE__);
								cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}	
							memset(aging_pre_rule->next, 0, sizeof(rule_item_t));
							free_rule = aging_pre_rule->next;
							atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* add by zhaohan */
						}

						free_rule->rules.time_stamp = get_sec(); 
						free_rule->rules.dip = ip->ip_dst;
						free_rule->rules.sip = ip->ip_src;
						free_rule->rules.protocol= ip->ip_p;
						free_rule->rules.dport= th->th_dport;
						free_rule->rules.sport= th->th_sport;
						free_rule->rules.rule_state = RULE_IS_LEARNING;
						free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						head_rule->valid_entries++; /* ---------- modify by zhaohan ----------- */
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						cvmx_spinlock_unlock(head_lock);

#ifdef USER_TABLE_FUNCTION	
            			set_acl_mask(head_rule);
#endif

						return free_rule;
					}
					else
						continue;
				}
				else
				{
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
							"acl_table_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
					cvmx_spinlock_unlock(head_lock);
					return NULL;
				}
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d.\r\n",__FILE__, __LINE__);
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
		/************************aging end*****************************/
		if((rule->rules.dip == ip->ip_dst) && (rule->rules.sip == ip->ip_src) &&
				(rule->rules.dport == th->th_dport) &&(rule->rules.sport == th->th_sport) &&(rule->rules.protocol == ip->ip_p)) 
		{		
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_table_lookup: Find the rule=0x%p\n",rule);

			if(rule->rules.rule_state== RULE_IS_STATIC) 
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}

			/*是否打开周期性强制老化*/
			if(acl_aging_enable == FUNC_ENABLE)
				rule->rules.time_stamp = get_sec(); 

			if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);		
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d, rule_state=%d.\r\n",__FILE__, __LINE__,rule->rules.rule_state);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}

		}
		else
		{
			if(rule->next != NULL)
			{
				aging_pre_rule=rule;
				rule = rule->next;
				aging_cur_rule=rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"Need to malloc new rule\n");

				if(free_rule == NULL)
				{
					/* if more than 8 node at one list, replace the first un-static dynamic rule, add by zhaohan */
					if(head_rule->valid_entries >= acl_bucket_max_entries)
					{
						rule_item_t* replace_rule = head_rule->next;
						rule_item_t* pre_rule = head_rule;
						uint64_t *ptmp;
						uint32_t tmp = 0;  

                        /* bug trace */
                        if(replace_rule == NULL)
                        {
                            printf("replace_rule == NULL! should not be here!\n");
                            printf("head_rule->valid_entries = %d\n", head_rule->valid_entries);
                            printf("head_rule->rules.rule_state=%d\n", head_rule->rules.rule_state);
                            printf("header rule info: dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
                                            IP_FMT(head_rule->rules.dip),IP_FMT(head_rule->rules.sip),head_rule->rules.dport,head_rule->rules.sport,head_rule->rules.protocol);
                            return NULL;
                        }

						/* find the first none-static rule */
						while(replace_rule->rules.rule_state == RULE_IS_STATIC)
						{
							if(NULL == replace_rule->next)
							{
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}
							pre_rule = replace_rule;
							replace_rule = replace_rule->next;
						}

						/* remove & clear replace rule */
						if(replace_rule->next != NULL)
						{
    						pre_rule->next = replace_rule->next;  /* there is a bug when acl_bucket_max_entries = 2 */
                            rule->next = replace_rule;  
                        }
						
						ptmp = (uint64_t*)replace_rule;
						while(tmp < sizeof(rule_item_t)/8) /*144 Bytes*//*没有考虑capwap表项删除*/
						{
							*ptmp = 0;
							ptmp = ptmp + 1;
							tmp++;
						}

						/* add replace_rule to tail */						
						replace_rule->next = NULL;
                		/* debug trace.add by zhaohan */
                        if((head_rule->valid_entries == acl_bucket_max_entries)&&(head_rule->next == NULL))
                        {
                            printf("Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
                        }
						replace_rule->rules.time_stamp = get_sec();
						replace_rule->rules.dip = ip->ip_dst;
						replace_rule->rules.sip = ip->ip_src;
						replace_rule->rules.protocol= ip->ip_p;
						replace_rule->rules.dport= th->th_dport;
						replace_rule->rules.sport= th->th_sport;
						replace_rule->rules.rule_state = RULE_IS_LEARNING;
						replace_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						cvmx_spinlock_unlock(head_lock);
						cvmx_fau_atomic_add64(CVM_FAU_MAX_RULE_ENTRIES, 1);
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						return replace_rule;
					}

					rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
					if(rule->next == NULL)
					{
						FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"InsertFastRule: Memory not available for adding rules\n");
						cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);	
						cvmx_spinlock_unlock(head_lock);
						return NULL;
					}	
					memset(rule->next, 0, sizeof(rule_item_t));
					free_rule = rule->next;
					atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  // add by zhaohan
				}

				free_rule->rules.time_stamp = get_sec(); 
				free_rule->rules.dip = ip->ip_dst;
				free_rule->rules.sip = ip->ip_src;
				free_rule->rules.protocol= ip->ip_p;
				free_rule->rules.dport= th->th_dport;
				free_rule->rules.sport= th->th_sport;
				free_rule->rules.rule_state = RULE_IS_LEARNING;
				free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
				head_rule->valid_entries++;
				cvmx_spinlock_unlock(head_lock);
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				return free_rule;

			}
		}
	}
}

static inline rule_item_t  * acl_table_one_tuple_lookup(cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th)
{    
	rule_item_t  *rule  = NULL;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	rule_item_t  *aging_cur_rule = NULL;
	rule_item_t  *head_rule = NULL;
	rule_item_t  *aging_pre_rule = NULL;
	cvmx_spinlock_t  *head_lock = NULL;


	if (ip == NULL || th == NULL)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_table_one_tuple_lookup: NULL Pointer !...\r\n");	
		return NULL;		
	}

	cvmx_fau_atomic_add64(CVM_FAU_TOTAL_ACL_LOOKUP, 1);
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_table_one_tuple_lookup: packet-oneTuple----sip=%d.%d.%d.%d \r\n",
			IP_FMT(ip->ip_src));

	/*look up ACL Table and get the bucket*/
	cvm_ip_hash_lookup(ip->ip_dst);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	aging_cur_rule = rule;
	aging_pre_rule = rule;

	head_lock = &rule->lock;
	cvmx_spinlock_lock(head_lock);

	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			head_rule->rules.time_stamp = get_sec(); 
			head_rule->rules.dip = ip->ip_dst;
			head_rule->rules.sip = ip->ip_src;
			head_rule->rules.protocol= ip->ip_p;
			head_rule->rules.dport= th->th_dport;
			head_rule->rules.sport= th->th_sport;
			head_rule->rules.rule_state = RULE_IS_LEARNING;
			head_rule->rules.action_type = FLOW_ACTION_TOLINUX;
			head_rule->valid_entries++;
			cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
			cvmx_spinlock_unlock(head_lock);
			return rule;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				aging_cur_rule = head_rule->next;
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
	}

	/*aging the current rule first then compare the rule with the key,loop*/			 
	while(1)
	{
		if(rule == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, 
					FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"acl_table_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_spinlock_unlock(head_lock);
			return NULL;
		}

		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_table_lookup: current Rule info: dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
				IP_FMT(rule->rules.dip),IP_FMT(rule->rules.sip),rule->rules.dport,rule->rules.sport,rule->rules.protocol);

		/************************aging first*****************************/
		if((acl_aging_check(&(rule->rules)) > 0) && (rule->rules.rule_state != RULE_IS_STATIC))
		{   
			atomic_add64_nosync(&(rule_cnt_info.rule_age_cnt)); /*rule need to be aging*/
			if(acl_delete_rule_by_aging(head_rule,aging_pre_rule ,aging_cur_rule)==RETURN_OK)
			{
				if(aging_cur_rule == head_rule)     /* add by zhaohan */
				{
					free_rule = head_rule;
				}
				if(aging_pre_rule !=NULL)
				{
					rule=aging_pre_rule->next;
					aging_cur_rule=aging_pre_rule->next;

					if(rule==NULL)/*after age, there is no more bucket*/
					{
						if(free_rule == NULL)
						{
							aging_pre_rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
							if(aging_pre_rule->next == NULL)
							{
								FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, 
										FASTFWD_COMMON_DBG_LVL_ERROR,
										"InsertFastRule: Memory not available for adding rules line=%d\n",__LINE__);
								cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}	
							memset(aging_pre_rule->next, 0, sizeof(rule_item_t));
							free_rule = aging_pre_rule->next;
							atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* add by zhaohan */
						}

						free_rule->rules.time_stamp = get_sec(); 
						free_rule->rules.dip = ip->ip_dst;
						free_rule->rules.sip = ip->ip_src;
						free_rule->rules.protocol= ip->ip_p;
						free_rule->rules.dport= th->th_dport;
						free_rule->rules.sport= th->th_sport;
						free_rule->rules.rule_state = RULE_IS_LEARNING;
						free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						head_rule->valid_entries++; /* ---------- modify by zhaohan ----------- */
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						cvmx_spinlock_unlock(head_lock);
						return free_rule;
					}
					else
						continue;
				}
				else
				{
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
							"acl_table_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
					cvmx_spinlock_unlock(head_lock);
					return NULL;
				}
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d.\r\n",__FILE__, __LINE__);
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
		/************************aging end*****************************/

		if(rule->rules.dip== ip->ip_dst)    /* only compare dest ip */
		{		
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_table_lookup: Find the rule=0x%p\n",rule);

			if(rule->rules.rule_state== RULE_IS_STATIC) 
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}

			/*是否打开周期性强制老化*/
			if(acl_aging_enable == FUNC_ENABLE)
				rule->rules.time_stamp = get_sec(); 

			if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d, rule_state=%d.\r\n",__FILE__, __LINE__,rule->rules.rule_state);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}

		}
		else
		{
			if(rule->next != NULL)
			{
				aging_pre_rule=rule;
				rule = rule->next;
				aging_cur_rule=rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"Need to malloc new rule\n");

				if(free_rule == NULL)
				{
					/* if more than 8 node at one list, replace the first un-static dynamic rule, add by zhaohan */
					if(head_rule->valid_entries >= acl_bucket_max_entries)
					{
						rule_item_t* replace_rule = head_rule->next;
						rule_item_t* pre_rule = head_rule;
						uint64_t *ptmp;
						uint32_t tmp = 0;  

						/* find the first none-static rule */
						while(replace_rule->rules.rule_state == RULE_IS_STATIC)
						{
							if(NULL == replace_rule->next)
							{
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}
							pre_rule = replace_rule;
							replace_rule = replace_rule->next;
						}

						/* remove & clear replace rule */
						pre_rule->next = replace_rule->next; 
						ptmp = (uint64_t*)replace_rule;
						while(tmp < 16) /*128 Bytes*/
						{
							*ptmp = 0;
							ptmp = ptmp + 1;
							tmp++;
						}

						/* add replace_rule to tail */
						rule->next = replace_rule;  
						replace_rule->next = NULL;
						replace_rule->rules.time_stamp = get_sec();
						replace_rule->rules.dip = ip->ip_dst;
						replace_rule->rules.sip = ip->ip_src;
						replace_rule->rules.protocol= ip->ip_p;
						replace_rule->rules.dport= th->th_dport;
						replace_rule->rules.sport= th->th_sport;
						replace_rule->rules.rule_state = RULE_IS_LEARNING;
						replace_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						cvmx_spinlock_unlock(head_lock);
						cvmx_fau_atomic_add64(CVM_FAU_MAX_RULE_ENTRIES, 1);
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						return replace_rule;
					}

					rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
					if(rule->next == NULL)
					{
						FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"InsertFastRule: Memory not available for adding rules\n");
						cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);	
						cvmx_spinlock_unlock(head_lock);
						return NULL;
					}	
					memset(rule->next, 0, sizeof(rule_item_t));
					free_rule = rule->next;
					atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  // add by zhaohan
				}

				free_rule->rules.time_stamp = get_sec(); 
				free_rule->rules.dip = ip->ip_dst;
				free_rule->rules.sip = ip->ip_src;
				free_rule->rules.protocol= ip->ip_p;
				free_rule->rules.dport= th->th_dport;
				free_rule->rules.sport= th->th_sport;
				free_rule->rules.rule_state = RULE_IS_LEARNING;
				free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
				head_rule->valid_entries++;
				cvmx_spinlock_unlock(head_lock);
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				return free_rule;

			}
		}
	}
}



static inline rule_item_t  * acl_table_icmp_lookup(cvm_common_ip_hdr_t *ip)
{
	rule_item_t  *rule  = NULL;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	rule_item_t  *aging_cur_rule = NULL;
	rule_item_t  *head_rule = NULL;
	rule_item_t  *aging_pre_rule = NULL;
	cvmx_spinlock_t  *head_lock = NULL;


	if (ip == NULL)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_table_icmp_lookup: ip is NULL Pointer !...\r\n");	
		return NULL;		
	}
	if(ip->ip_p != CVM_COMMON_IPPROTO_ICMP)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR,
				"acl_table_icmp_lookup: packet is not icmp type!...\r\n");	
		return NULL;	
	}

	cvmx_fau_atomic_add64(CVM_FAU_TOTAL_ACL_LOOKUP, 1);
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_table_icmp_lookup: packet-Three Tuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,proto=%d.  \r\n",
			IP_FMT(ip->ip_dst),IP_FMT(ip->ip_src), ip->ip_p);

	/*look up ACL Table and get the bucket*/
	cvm_three_tupe_hash_lookup(ip->ip_dst, ip->ip_src,ip->ip_p);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	aging_cur_rule = rule;
	aging_pre_rule = rule;

	head_lock = &rule->lock;
	cvmx_spinlock_lock(head_lock);

	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			head_rule->rules.time_stamp = get_sec(); 
			head_rule->rules.dip = ip->ip_dst;
			head_rule->rules.sip = ip->ip_src;
			head_rule->rules.protocol= ip->ip_p;
			head_rule->rules.rule_state = RULE_IS_LEARNING;
			head_rule->rules.action_type = FLOW_ACTION_TOLINUX;
			head_rule->valid_entries++;
			cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
			cvmx_spinlock_unlock(head_lock);
			return rule;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				aging_cur_rule = head_rule->next;
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_icmp_lookup: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
	}

	/*aging the current rule first then compare the rule with the key,loop*/			 
	while(1)
	{
		if(rule == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, 
					FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"acl_table_icmp_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_spinlock_unlock(head_lock);
			return NULL;
		}

		/************************aging first*****************************/
		if((acl_aging_check(&(rule->rules)) > 0) && (rule->rules.rule_state != RULE_IS_STATIC))
		{   
			atomic_add64_nosync(&(rule_cnt_info.rule_age_cnt)); /*rule need to be aging*/
			if(acl_delete_rule_by_aging(head_rule,aging_pre_rule ,aging_cur_rule)==RETURN_OK)
			{
				if(aging_cur_rule == head_rule)     /* add by zhaohan */
				{
					free_rule = head_rule;
				}
				if(aging_pre_rule !=NULL)
				{
					rule=aging_pre_rule->next;
					aging_cur_rule=aging_pre_rule->next;

					if(rule==NULL)/*after age, there is no more bucket*/
					{
						if(free_rule == NULL)
						{
							aging_pre_rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
							if(aging_pre_rule->next == NULL)
							{
								FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, 
										FASTFWD_COMMON_DBG_LVL_ERROR,
										"InsertFastRule: Memory not available for adding rules line=%d\n",__LINE__);
								cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}	
							memset(aging_pre_rule->next, 0, sizeof(rule_item_t));
							free_rule = aging_pre_rule->next;
							atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* add by zhaohan */
						}

						free_rule->rules.time_stamp = get_sec(); 
						free_rule->rules.dip = ip->ip_dst;
						free_rule->rules.sip = ip->ip_src;
						free_rule->rules.protocol= ip->ip_p;
						free_rule->rules.rule_state = RULE_IS_LEARNING;
						free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						head_rule->valid_entries++; /* ---------- modify by zhaohan ----------- */
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						cvmx_spinlock_unlock(head_lock);
						return free_rule;
					}
					else
						continue;
				}
				else
				{
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
							"acl_table_icmp_lookup: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
					cvmx_spinlock_unlock(head_lock);
					return NULL;
				}
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_icmp_lookup: Should never come to here file%s, line %d.\r\n",__FILE__, __LINE__);
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}
		}
		/************************aging end*****************************/

		if((rule->rules.dip == ip->ip_dst) && (rule->rules.sip == ip->ip_src) && (rule->rules.protocol == ip->ip_p))
		{		
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_table_icmp_lookup: Find the rule: dip=%d.%d.%d.%d, sip=%d.%d.%d.%d, proto=%d.  \r\n",
					IP_FMT(rule->rules.dip),IP_FMT(rule->rules.sip),rule->rules.protocol);

			if(rule->rules.rule_state== RULE_IS_STATIC) 
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}

			/*是否打开周期性强制老化*/
			if(acl_aging_enable == FUNC_ENABLE)
				rule->rules.time_stamp = get_sec(); 

			if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				cvmx_fau_atomic_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				cvmx_spinlock_unlock(head_lock);
				return rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_table_lookup: Should never come to here file%s, line %d, rule_state=%d.\r\n",__FILE__, __LINE__,rule->rules.rule_state);	
				cvmx_spinlock_unlock(head_lock);
				return NULL;
			}

		}
		else
		{
			if(rule->next != NULL)
			{
				aging_pre_rule=rule;
				rule = rule->next;
				aging_cur_rule=rule;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"Need to malloc new rule\n");

				if(free_rule == NULL)
				{
					/* if more than 8 node at one list, replace the first un-static dynamic rule, add by zhaohan */
					if(head_rule->valid_entries >= acl_bucket_max_entries)
					{
						rule_item_t* replace_rule = head_rule->next;
						rule_item_t* pre_rule = head_rule;
						uint64_t *ptmp;
						uint32_t tmp = 0;  

						/* find the first none-static rule */
						while(replace_rule->rules.rule_state == RULE_IS_STATIC)
						{
							if(NULL == replace_rule->next)
							{
								cvmx_spinlock_unlock(head_lock);
								return NULL;
							}
							pre_rule = replace_rule;
							replace_rule = replace_rule->next;
						}

						/* remove & clear replace rule */
						pre_rule->next = replace_rule->next; 
						ptmp = (uint64_t*)replace_rule;
						while(tmp < 16) /*128 Bytes*/
						{
							*ptmp = 0;
							ptmp = ptmp + 1;
							tmp++;
						}

						/* add replace_rule to tail */
						rule->next = replace_rule;  
						replace_rule->next = NULL;
						replace_rule->rules.time_stamp = get_sec();
						replace_rule->rules.dip = ip->ip_dst;
						replace_rule->rules.sip = ip->ip_src;
						replace_rule->rules.protocol= ip->ip_p;
						replace_rule->rules.rule_state = RULE_IS_LEARNING;
						replace_rule->rules.action_type = FLOW_ACTION_TOLINUX;
						cvmx_spinlock_unlock(head_lock);
						cvmx_fau_atomic_add64(CVM_FAU_MAX_RULE_ENTRIES, 1);
						cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
						return replace_rule;
					}

					rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
					if(rule->next == NULL)
					{
						FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"InsertFastRule: Memory not available for adding rules\n");
						cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);	
						cvmx_spinlock_unlock(head_lock);
						return NULL;
					}	
					memset(rule->next, 0, sizeof(rule_item_t));
					free_rule = rule->next;
					atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  // add by zhaohan
				}

				free_rule->rules.time_stamp = get_sec(); 
				free_rule->rules.dip = ip->ip_dst;
				free_rule->rules.sip = ip->ip_src;
				free_rule->rules.protocol= ip->ip_p;
				free_rule->rules.rule_state = RULE_IS_LEARNING;
				free_rule->rules.action_type = FLOW_ACTION_TOLINUX;
				head_rule->valid_entries++;
				cvmx_spinlock_unlock(head_lock);
				cvmx_fau_atomic_add64(CVM_FAU_ACL_REG, 1);
				return free_rule;
			}
		}
	}

}


static inline void check_rule_entry_num()
{
	rule_item_t  *rule = NULL;
	rule_item_t  *head_rule = NULL;
	uint32_t list_num = 0;
	uint32_t no_match_num = 0;
	uint32_t i;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		list_num = 0;
		head_rule = &acl_bucket_tbl[i];
		cvmx_spinlock_lock(&head_rule->lock);

		if(head_rule->rules.rule_state != RULE_IS_EMPTY)
			list_num++;

		rule = head_rule->next;
		while(rule)
		{
			list_num++;
			rule = rule->next;
		}

		if(head_rule->valid_entries != list_num)
			no_match_num++;
		cvmx_spinlock_unlock(&head_rule->lock);
	}

	printf("no match rule num: %d\n", no_match_num);
}

static inline void check_pfast_entry_num()
{
	rule_item_t  *head_rule = NULL;
    uint32_t i = 0;
    uint32_t no_match_num = 0;
    uint64_t flag = 0;
    uint64_t index = 0;
    uint64_t mask_group = 0;
    
    for(i = 0; i < acl_static_tbl_size; i++)
	{
		head_rule = &acl_bucket_tbl[i];
		cvmx_spinlock_lock(&head_rule->lock);

		if((head_rule->rules.rule_state != RULE_IS_EMPTY) || (NULL != head_rule->next))
		{
			flag = 1;
        }else
        {
            flag = 0;
        }
        
	    index = head_rule - acl_bucket_tbl;
	    cvmx_spinlock_lock(&acl_mask_lock);
    	mask_group = *(pfast_acl_mask_tbl+(index/64));
    	if((mask_group & (1ull<<(index%64))) != (flag<<(index%64)))
    	{
    	    no_match_num++;
    	}
    	cvmx_spinlock_unlock(&acl_mask_lock);
    	
		cvmx_spinlock_unlock(&head_rule->lock);	
	}

	printf("no match rule num: %u\n", no_match_num);
}

int acl_table_init();
int32_t acl_insert_static_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache);
int32_t acl_self_learn_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache);
int32_t acl_delete_rule(five_tuple_t* five_tuple);
int32_t acl_get_rule_stats(rule_stats_t* rule_stats);
void acl_clear_rule(void);
int acl_clear_aging_rule(void);

int32_t acl_one_tuple_self_learn_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache);
int32_t acl_insert_static_icmp_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache);
int32_t acl_self_learn_icmp_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache);

int32_t user_action_online(uint32_t user_ip);
int32_t user_action_offline(uint32_t user_ip);
int32_t user_stats_clear(uint32_t user_ip);
void user_flow_statistics_process(cvmx_wqe_t *work, rule_item_t *rule,cvm_common_ip_hdr_t *true_ip);

void flow_action_process(cvmx_wqe_t *work, uint32_t action_type, cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th, rule_item_t *prule,uint8_t is_qos, cvmx_spinlock_t *first_lock, uint8_t is_pppoe);

int fastfwd_show_fau64(fau64_info_t *fau64_info);
#endif

