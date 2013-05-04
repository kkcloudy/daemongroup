#ifdef CVM_ACL
/***********************license start************************************
 * File name: fwd_main.c 
 * Auther     : lutao
 * 
 * Copyright (c) Autelan . All rights reserved.
 * 
 **********************license end**************************************/
#include <string.h>
#include <stdlib.h>

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
#include "cvmx-malloc.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "acl.h"	
#include "shell.h"
#include "capwap.h"	
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-defs.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-fpa.h"
#include "fastfwd-common-rnd.h"
#else
#include "cvm-common-defs.h"
#include "cvm-common-misc.h"
#include "cvm-common-fpa.h"
#include "cvm-common-rnd.h"
#endif

extern CVMX_SHARED product_info_t product_info;

CVMX_SHARED capwap_cache_t *capwap_cache_bl;
CVMX_SHARED cvmx_spinlock_t capwap_cache_bl_lock; /* add by zhaohan */

CVMX_SHARED cvmx_arena_list_t   rule_arena;
CVMX_SHARED cvmx_arena_list_t   user_table_arena;
CVMX_SHARED rule_item_t  *acl_bucket_tbl;
CVMX_SHARED uint32_t acl_static_tbl_size;
CVMX_SHARED uint32_t acl_dynamic_tbl_size;
CVMX_SHARED uint32_t acl_aging_timer = 600; /*in second*/
CVMX_SHARED uint32_t acl_aging_enable = FUNC_DISABLE;
CVMX_SHARED uint32_t acl_bucket_max_entries = 8;

#ifdef USER_TABLE_FUNCTION
CVMX_SHARED user_item_t *user_bucket_tbl;
CVMX_SHARED uint32_t user_static_tbl_size;
CVMX_SHARED uint32_t user_dynamic_tbl_size;
CVMX_SHARED uint64_t *pfast_acl_mask_tbl;
CVMX_SHARED cvmx_spinlock_t  acl_mask_lock;
#endif
CVMX_SHARED uint8_t *base_mac_table = NULL;
CVMX_SHARED uint32_t base_mac_size = 6 * 20;

CVMX_SHARED uint32_t pfast_num=0;


/**************flow table counters for debug********************/
/* cnt_info add by zhaohan, for testing */
CVMX_SHARED rule_cnt_info_t rule_cnt_info;  /* add by zhaohan */


int acl_get_mem()
{
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
	uint64_t sys_mem = sys_info_ptr->system_dram_size;

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_mem_calc: system memory is %lld M\r\n",CAST64(sys_mem));	

	if (sys_mem <= 16*1024)
	{
		/* convert Mbytes to bytes */
		sys_mem = sys_mem * 1024ull * 1024ull;
	}

	switch (sys_mem)
	{
		case 2048ull * 1024ull * 1024ull:
			acl_static_tbl_size = (256*1024);
			acl_dynamic_tbl_size = (256*1024);			
			break;

		case 4096ull * 1024ull * 1024ull:
			acl_static_tbl_size = (1024*1024);
			acl_dynamic_tbl_size = (1024*1024);
			break;
		case 8192ull * 1024ull * 1024ull:       
			acl_static_tbl_size = (2*1024*1024);
			acl_dynamic_tbl_size = (2*1024*1024);
			break;

		default:
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
					"Warning: system memory does not match (sys_mem=%lld)\n",CAST64(sys_mem));				
			return (RETURN_ERROR);
	}

	return RETURN_OK;
}

int acl_table_init()
{
	void  *ptr = NULL;
	rule_item_t *bucket = NULL;
	user_item_t *user_bucket = NULL;	
	uint32_t   loop = 0;

	switch(product_info.rule_mem_size)
	{
		case RULE_MEM_128M:
			acl_static_tbl_size = (512*1024);
			acl_dynamic_tbl_size = (256*1024);
			break;
		case RULE_MEM_256M:
			acl_static_tbl_size = (1024*1024);
			acl_dynamic_tbl_size = (1024*1024);
			break;
		case RULE_MEM_512M:
			acl_static_tbl_size = (2048*1024);
			acl_dynamic_tbl_size = (2048*1024);			
			break;
		case RULE_MEM_1792M:
			acl_static_tbl_size = (7*1024*1024);
			acl_dynamic_tbl_size = (7*1024*1024);	
			break;
		default:
			acl_get_mem();
			break;
	}

	user_static_tbl_size = (64*1024);
	user_dynamic_tbl_size = (64*1024);

	printf("acl_static_tbl_size = 0x%x\n", acl_static_tbl_size);
	printf("acl_dynamic_tbl_size = 0x%x\n", acl_dynamic_tbl_size);
	printf("user_static_tbl_size = 0x%x\n", user_static_tbl_size);
	printf("user_dynamic_tbl_size = 0x%x\n", user_dynamic_tbl_size);
	/*
	 * cvmx_bootmem_alloc用来分配大块的共享内存，使用后不在释放。
	 * 由应用程序自己负责相关内存的读写
	 */
#ifdef USER_TABLE_FUNCTION
	user_bucket_tbl =  (user_item_t *) cvmx_bootmem_alloc_named(user_static_tbl_size*sizeof(user_item_t), CVMX_CACHE_LINE_SIZE,USER_TBL_RULE_NAME);
	if(user_bucket_tbl == NULL)
	{
		printf("Warning: No memory available for USER static hash table!\n");
		return RETURN_ERROR;
	}
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"user_bucket_tbl address: 0x%lx,  User table size=%ld\n", (uint64_t)user_bucket_tbl, user_static_tbl_size*sizeof(user_item_t));

	memset((void *)user_bucket_tbl, 0, user_static_tbl_size*sizeof(user_item_t));

	for (loop = 0; loop < user_static_tbl_size; loop++)
	{
		user_bucket = (user_item_t *)user_bucket_tbl+loop;
		//cvmx_spinlock_init(&user_bucket->lock);
		cvmx_rwlock_wp_init(&user_bucket->lock);
	}

	ptr = cvmx_bootmem_alloc_named(user_dynamic_tbl_size*sizeof(user_item_t), CVMX_CACHE_LINE_SIZE, USER_DYNAMIC_TBL_RULE_NAME);
	if(ptr == NULL)
	{
		printf("Warning: No memory available for user dynamic hash table!\n");
		return RETURN_ERROR;
	}
	memset((void *)ptr, 0, user_dynamic_tbl_size*sizeof(user_item_t));
	if(cvmx_add_arena  (&user_table_arena, ptr, user_dynamic_tbl_size*sizeof(user_item_t))<0)
	{
		printf("Failed to add arena for user dynamic table\n");
		return RETURN_ERROR;
	}

	if((acl_static_tbl_size%64)!=0)
	{
		pfast_num=acl_static_tbl_size/64+1;
	}
	else
	{
		pfast_num=acl_static_tbl_size/64;
	}

	pfast_acl_mask_tbl = (uint64_t  *) cvmx_bootmem_alloc_named(pfast_num*8, CVMX_CACHE_LINE_SIZE,ACL_FAST_TBL_MASK_NAME);
	if(!pfast_acl_mask_tbl)
	{
		printf("No memory available for ACL hash mask num=%d\n",pfast_num);
		return RETURN_ERROR;
	}
	memset((void *)pfast_acl_mask_tbl, 0, pfast_num*8);

	cvmx_spinlock_init(&acl_mask_lock);
#endif


	acl_bucket_tbl = (rule_item_t *) cvmx_bootmem_alloc_named(acl_static_tbl_size*sizeof(rule_item_t), CVMX_CACHE_LINE_SIZE,ACL_TBL_RULE_NAME);
	if(acl_bucket_tbl == NULL)
	{
		printf("Warning: No memory available for ACL static hash table!\n");
		return RETURN_ERROR;
	}
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"ACL rule static table size=%ld\n",acl_static_tbl_size*sizeof(rule_item_t));
	memset((void *)acl_bucket_tbl, 0, acl_static_tbl_size*sizeof(rule_item_t));

	for (loop = 0; loop < acl_static_tbl_size; loop++)
	{
		bucket = (rule_item_t *)acl_bucket_tbl+loop;
		cvmx_spinlock_init(&bucket->lock);
	}

	ptr = cvmx_bootmem_alloc_named(acl_dynamic_tbl_size*sizeof(rule_item_t), CVMX_CACHE_LINE_SIZE, ACL_DYNAMIC_TBL_RULE_NAME);
	if(ptr == NULL)
	{
		printf("Warning: No memory available for ACL dynamic hash table!\n");
		return RETURN_ERROR;
	}
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"ACL rule dynamic table size=%ld\n",acl_dynamic_tbl_size*sizeof(rule_item_t));	
	memset((void *)ptr, 0, acl_dynamic_tbl_size*sizeof(rule_item_t));

	/*
	 * 创建动态共享内存区，首先通过bootmem分配大块内存通过
	 * add函数初始化，后续只需要cvmx_malloc或者free来进行内存的
	 * 动态申请或者释放
	 */  
	if(cvmx_add_arena  (&rule_arena, ptr, acl_dynamic_tbl_size*sizeof(rule_item_t))<0)
	{
		printf("Failed to add arena for ACL dynamic table\n");
		return RETURN_ERROR;
	}

	capwap_cache_bl = (capwap_cache_t *) cvmx_bootmem_alloc_named((MAX_CAPWAP_CACHE_NUM+1)*sizeof(capwap_cache_t), CVMX_CACHE_LINE_SIZE,CAPWAP_CACHE_TBL_NAME);

	if(!capwap_cache_bl)
	{
		printf("No memory available for slow CAPWAP cache size=%ld\n",sizeof(capwap_cache_t));
		return RETURN_ERROR;
	}

	memset((void *)capwap_cache_bl, 0, (MAX_CAPWAP_CACHE_NUM+1)*sizeof(capwap_cache_t));

	cvmx_spinlock_init(&capwap_cache_bl_lock);

	/*add by rpa base mac table*/
	base_mac_table =  (uint8_t *) cvmx_bootmem_alloc_named(base_mac_size, CVMX_CACHE_LINE_SIZE,BASE_MAC_TABLE_NAME);
	if(base_mac_table == NULL)
	{
		printf("Warning: No memory available for base mac table!\n");
		return RETURN_ERROR;
	}
	memset(base_mac_table,0,base_mac_size);
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"base mac table address: 0x%lx\n", (uint64_t)base_mac_table);

	printf("ACL module rule table init OK \r\n");

	return RETURN_OK;
}


static inline int32_t acl_find_or_insert_cw(uint16_t* p_idx, capwap_cache_t* cw_cache)
{
	uint16_t idx = 0;
	int16_t free_idx = -1;

	if(NULL ==cw_cache)
	{
		return RETURN_ERROR;
	}

	if(NULL == p_idx)
	{
		return RETURN_ERROR;
	}

	for(idx = 0; idx < MAX_CAPWAP_CACHE_NUM; idx++)
	{
		/* find a empty cache */
		if(capwap_cache_bl[idx].use_num == 0)
		{  
			if(free_idx == -1)
			{               
				free_idx = idx;  /* mark the first free cache */
			}
		}
		/* not empty, compare */
		else    
		{
			uint64_t *cache_cw_h = (uint64_t*)(capwap_cache_bl[idx].cw_hd);
			uint64_t *fccp_cw_h = (uint64_t*)(cw_cache->cw_hd);

			/* match a cache */
			if((capwap_cache_bl[idx].dip == cw_cache->dip) &&
					(capwap_cache_bl[idx].sip == cw_cache->sip) &&
					(capwap_cache_bl[idx].dport == cw_cache->dport) &&
					(capwap_cache_bl[idx].sport == cw_cache->sport) &&
					(capwap_cache_bl[idx].tos== cw_cache->tos) &&
					(cache_cw_h[0] == fccp_cw_h[0]) &&
					(cache_cw_h[1] == fccp_cw_h[1]))
			{
				*p_idx = idx;
				atomic_add32_nosync(&( capwap_cache_bl[idx].use_num));
				return RETURN_OK;
			}
		}
	}

	/* not match, fill the free caches */
	if(free_idx != -1)
	{
		uint64_t *cache_cw_h = (uint64_t*)(capwap_cache_bl[free_idx].cw_hd);
		uint64_t *fccp_cw_h = (uint64_t*)(cw_cache->cw_hd);

		/* fill the free cache */
		capwap_cache_bl[free_idx].dip = cw_cache->dip;
		capwap_cache_bl[free_idx].sip = cw_cache->sip;
		capwap_cache_bl[free_idx].dport = cw_cache->dport;
		capwap_cache_bl[free_idx].sport = cw_cache->sport;
		capwap_cache_bl[free_idx].tos = cw_cache->tos;
		cache_cw_h[0] = fccp_cw_h[0]; /* fill in 64 bit */
		cache_cw_h[1] = fccp_cw_h[1];
		atomic_add32_nosync(&( capwap_cache_bl[free_idx].use_num));

		*p_idx = free_idx;
		return RETURN_OK;
	}

	/* only one possible situation can reach here: the capwap table is full */
	return RETURN_ERROR;
}


static inline int32_t acl_fill_rule(rule_param_t* dst_rule_para, rule_param_t* src_rule_para, capwap_cache_t* cw_cache)
{
	int32_t ret = 0;

	if((NULL == dst_rule_para) || (NULL == src_rule_para) || (NULL == cw_cache))
	{
		return RETURN_ERROR;
	}

	if((src_rule_para->rule_state != RULE_IS_STATIC) && (src_rule_para->rule_state != RULE_IS_LEARNED))
	{
		return RETURN_ERROR;
	}

	/* fill capwap first */
	if((src_rule_para->action_type == FLOW_ACTION_CAPWAP_FORWARD) || 
			(src_rule_para->action_type == FLOW_ACTION_CAP802_3_FORWARD) ||
			(src_rule_para->action_type == FLOW_ACTION_CAPWAP_802_11_ICMP) || 
			(src_rule_para->action_type == FLOW_ACTION_CAPWAP_802_3_ICMP))
	{
		/* find a empty cache and fill it, or find a same cache */
		cvmx_spinlock_lock(&capwap_cache_bl_lock);
		ret = acl_find_or_insert_cw(&(dst_rule_para->tunnel_index), cw_cache);
		cvmx_spinlock_unlock(&capwap_cache_bl_lock);
		if(ret == RETURN_ERROR)
		{
			return RETURN_ERROR;
		}
	}

	/* l2 header information */
	dst_rule_para->dsa_info = src_rule_para->dsa_info;
	memcpy(dst_rule_para->ether_dhost, src_rule_para->ether_dhost, MAC_LEN);
	memcpy(dst_rule_para->ether_shost, src_rule_para->ether_shost, MAC_LEN);
	dst_rule_para->out_ether_type = src_rule_para->out_ether_type;
	dst_rule_para->out_tag = src_rule_para->out_tag;
	dst_rule_para->in_ether_type = src_rule_para->in_ether_type;
	dst_rule_para->in_tag = src_rule_para->in_tag;
	dst_rule_para->ether_type = src_rule_para->ether_type;

	/*rpa header information*/
	dst_rule_para->rpa_header.rpa_vlan_type = src_rule_para->rpa_header.rpa_vlan_type;
	dst_rule_para->rpa_header.rpa_vlan_tag = src_rule_para->rpa_header.rpa_vlan_tag;
	dst_rule_para->rpa_header.rpa_type = src_rule_para->rpa_header.rpa_type;
	dst_rule_para->rpa_header.rpa_dnetdevNum = src_rule_para->rpa_header.rpa_dnetdevNum;
	dst_rule_para->rpa_header.rpa_snetdevNum = src_rule_para->rpa_header.rpa_snetdevNum;
	dst_rule_para->rpa_header.rpa_d_s_slotNum = src_rule_para->rpa_header.rpa_d_s_slotNum;

	/* tunnel information */
	memcpy(&(dst_rule_para->tunnel_l2_header), &(src_rule_para->tunnel_l2_header), sizeof(src_rule_para->tunnel_l2_header));

	if(src_rule_para->rule_state == RULE_IS_STATIC)
	{
		/* five tuple */
		dst_rule_para->sip = src_rule_para->sip;
		dst_rule_para->dip = src_rule_para->dip;
		dst_rule_para->sport = src_rule_para->sport;
		dst_rule_para->dport = src_rule_para->dport;
		dst_rule_para->protocol = src_rule_para->protocol;
	}

	/* rule information */
	dst_rule_para->forward_port = src_rule_para->forward_port; 
	dst_rule_para->action_type = src_rule_para->action_type;	
	dst_rule_para->action_mask = src_rule_para->action_mask;	     
	dst_rule_para->user_index = src_rule_para->user_index; 
	dst_rule_para->user_link_index= src_rule_para->user_link_index;
	dst_rule_para->user2_index = src_rule_para->user2_index; 
	dst_rule_para->user2_link_index= src_rule_para->user2_link_index;	
	dst_rule_para->direct_flag= src_rule_para->direct_flag;
	dst_rule_para->nat_index = src_rule_para->nat_index;
	dst_rule_para->time_stamp = get_sec(); 
	dst_rule_para->rule_state = src_rule_para->rule_state;

	/* add by yin for fwd nat *//*wangjian_nat*/
	if (src_rule_para->nat_flag == 1) {
		dst_rule_para->nat_flag = src_rule_para->nat_flag;
		dst_rule_para->nat_sip = src_rule_para->nat_sip;
		dst_rule_para->nat_dip = src_rule_para->nat_dip;
		dst_rule_para->nat_sport = src_rule_para->nat_sport;
		dst_rule_para->nat_dport = src_rule_para->nat_dport;
	} 
	/*wangjian add for slave cpu ip is 0 20121214 */
	else
	{
		dst_rule_para->nat_flag = 0;
		dst_rule_para->nat_sip = 0;
		dst_rule_para->nat_dip = 0;
		dst_rule_para->nat_sport = 0;
		dst_rule_para->nat_dport = 0;
	}

	/*add by wangjian for support pppoe 2013-3-12*/
	/* pppoe_flag != 0 */
	if (1 == src_rule_para->pppoe_flag)
	{
		dst_rule_para->pppoe_flag = src_rule_para->pppoe_flag;
		dst_rule_para->pppoe_session_id = src_rule_para->pppoe_session_id;
	}
	else 
	{
		dst_rule_para->pppoe_flag = 0;
		dst_rule_para->pppoe_session_id = 0;
	}

	return RETURN_OK;
}


/*
 * delete the rule by fccp command
 * input the five tupe
 */
int32_t acl_cmd_delete_rule(five_tuple_t* five_tuple)
{
	rule_item_t  *rule = NULL ;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *pre_rule=NULL ;	
	cvmx_spinlock_t          *head_lock = NULL;

	if (NULL == five_tuple)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_cmd_delete_rule: NULL Pointer !...\r\n");	
		return RETURN_ERROR;		
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_cmd_delete_rule: packet-fiveTuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
			IP_FMT(five_tuple->ip_dst),IP_FMT(five_tuple->ip_src),five_tuple->th_dport,five_tuple->th_sport,five_tuple->ip_p);

	/*look up ACL Table and get the bucket*/
	hash(five_tuple->ip_dst, five_tuple->ip_src, five_tuple->ip_p, five_tuple->th_dport, five_tuple->th_sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	pre_rule = rule;
	head_lock = &rule->lock;

	if(head_rule->valid_entries== 0)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_cmd_delete_rule: the bucket is empty, can not delete\n");
		return RETURN_ERROR;
	}

	cvmx_spinlock_lock(&rule->lock);

	while(1)
	{		   
		if((rule->rules.dip == five_tuple->ip_dst) && (rule->rules.sip == five_tuple->ip_src) &&
				(rule->rules.dport == five_tuple->th_dport) &&(rule->rules.sport == five_tuple->th_sport) &&(rule->rules.protocol == five_tuple->ip_p)) 
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_cmd_delete_rule: find the rule,rule=0x%p\n",rule);									

			/* capwap table */
			if((rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
					(rule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD))
			{
				if(capwap_cache_bl[rule->rules.tunnel_index].use_num == 0)
				{
					printf("acl_cmd_delete_rule: can't reach here\n");
					return RETURN_ERROR;
				}
				/* reduce use_num */
				atomic_dec32_nosync(&(capwap_cache_bl[rule->rules.tunnel_index].use_num));
			}

			/*不是首条规则，则释放*/
			if (head_rule != rule)
			{		
				pre_rule->next = rule->next;	
				cvmx_free(rule);
				rule = NULL;
				head_rule->valid_entries--;
				atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1, add by zhaohan */
			}
			else
			{
				uint32_t tmp = 0;
				uint64_t *ptmp = (uint64_t *)(&(head_rule->rules));
				/*首条rule，清零*/
				while(tmp < (sizeof(struct rule_param_s)/sizeof(uint64_t))) /*112 Bytes*/
				{
					*ptmp = 0;
					ptmp = ptmp + 1;
					tmp++;
				}
				head_rule->valid_entries--;
			}
			cvmx_spinlock_unlock(head_lock);
			return RETURN_OK;
		}			 

		if(rule->next != NULL)
		{
			pre_rule = rule;
			rule = rule->next;
		}
		else
			break;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_cmd_delete_rule: the bucket is empty, can not delete\n");
	cvmx_spinlock_unlock(head_lock);
	return RETURN_ERROR;

}


/*
 * delete the rule by fccp command
 * input the five tupe
 */
/* 要讨论FCCP的删除命令格式，只需包括5元组 */
int32_t acl_delete_rule(five_tuple_t* five_tuple)
{
	if(NULL == five_tuple)
	{
		printf("acl_delete_rule: five_tuple is NULL!\n");
		return RETURN_ERROR;
	}

	return acl_cmd_delete_rule(five_tuple);
}


/**
 * insert the acl rule by FCCP cmd
 * Not finish, lutao tag
 * return:	-1  失败
 *		0    成功
 */
int32_t acl_insert_static_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache)
{
	rule_item_t  *rule = NULL;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock = NULL;

	if((NULL == rule_para) || (NULL == cw_cache))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_insert_static_rule: Invalid argument\n");
		return RETURN_ERROR;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_insert_static_rule: packet-fiveTuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d.  \r\n",
			IP_FMT(rule_para->dip), IP_FMT(rule_para->sip), rule_para->dport,rule_para->sport,rule_para->protocol);

	/*look up ACL Table and get the bucket*/
	hash(rule_para->dip, rule_para->sip, rule_para->protocol, rule_para->dport, rule_para->sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	cvmx_spinlock_lock(&rule->lock);
	head_lock = &rule->lock;


	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			/*表项为空，直接插入表项*/
			if(acl_fill_rule(&(head_rule->rules), rule_para, cw_cache) == RETURN_ERROR)
			{
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
			head_rule->valid_entries++;
			cvmx_spinlock_unlock(head_lock);

#ifdef USER_TABLE_FUNCTION	
			set_acl_mask(head_rule);
#endif
            
			return RETURN_OK;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_insert_static_rule: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);	
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}


	while(1)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_insert_static_rule: lookup the table, rule=0x%p.\r\n",rule);	

		if ((rule->rules.dip == rule_para->dip) && 
				(rule->rules.sip == rule_para->sip) &&
				(rule->rules.dport == rule_para->dport) &&
				(rule->rules.sport == rule_para->sport) &&
				(rule->rules.protocol == rule_para->protocol)) 
		{

			if((rule->rules.rule_state == RULE_IS_STATIC) || (rule->rules.rule_state == RULE_IS_CMD_SET))
			{
#if 0			
				/*找到了匹配项，根据策略决定属于更新还是重复配置*/
				if( rule->rules.XXXX == XXXXX)
				{
					/*策略不同，属于策略更新*/
				}
				else
				{
					/*策略相同，重复插入*/
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG, 
							"acl_insert_static_rule: repeat insert static rule\r\n");
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}
#endif			
				/*策略相同，重复插入*/
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG, 
						"acl_insert_static_rule: repeat insert static rule\r\n");
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
			else
			{
				/*插入的静态表与自学习表项冲突*/
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG, 
						"acl_insert_static_rule:conflict with the self learn rule 0x%p! \r\n",rule);
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;    
			}
		}

		if(rule->next != NULL)
			rule = rule->next;
		else
			break;
	}

	if (free_rule ==  NULL) 
	{
		// TODO: if(head_rule->valid_entries  >  8)

		/*没找到空白处，新申请一条*/
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG, 
				"acl_insert_static_rule: Need to malloc new rule.\r\n");

		rule->next = (rule_item_t *)cvmx_malloc(rule_arena, sizeof(rule_item_t));
		if(rule->next == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG, 
					"InsertFastRule: can't malloc memory for new rules\n");
			cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
			cvmx_spinlock_unlock(head_lock);
			return RETURN_ERROR;
		}
		memset(rule->next, 0, sizeof(rule_item_t));
		free_rule= rule->next;
		atomic_add64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* add by zhaohan */
	}

	/*插入静态表项到free_rule处*/			
	if(acl_fill_rule(&(free_rule->rules), rule_para, cw_cache) == RETURN_ERROR)
	{
		if(free_rule != head_rule)
		{
			cvmx_free(free_rule);
			free_rule = NULL;
			atomic_dec64_nosync(&(rule_cnt_info.dynamic_rule_cnt));  /* -1, add by zhaohan */
		}
		cvmx_spinlock_unlock(head_lock);
		return RETURN_ERROR;
	}
	head_rule->valid_entries++;     /* modify by zhaohan */

	cvmx_spinlock_unlock(head_lock);

	return RETURN_OK;	
}



/*************************************************************
 * Description:
 *         find the user from user table
 *
 * Parameter:
 *         ip  : user's ip address
 *         p_usr_idx   : user index in user table
 *         p_link_idx  : user  linkedlist index in user table(used for hash conflicts)
 *         
 * Return:
 *        
 *************************************************************/
static inline int32_t fwd_find_user_idx(uint32_t ip, uint32_t *p_usr_idx, uint16_t *p_link_idx)
{
	user_item_t *head_user = NULL;
	user_item_t *user = NULL;
	cvmx_rwlock_wp_lock_t *head_lock = NULL;
	
    if(user_bucket_tbl == NULL)
        return RETURN_ERROR;

	*p_usr_idx = user_cache_bucket_lookup(ip);
	head_user = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));
	user = head_user;
	head_lock = &head_user->lock; 
	cvmx_rwlock_wp_read_lock(head_lock);
	
	if(head_user->user_info.user_state == USER_IS_EMPTY)
	{
		if(head_user->valid_entries == 0)
		{
			cvmx_rwlock_wp_read_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			if(head_user->next != NULL)
			{
				user = head_user->next;
				(*p_link_idx)++;
			}
			else
			{
				cvmx_rwlock_wp_read_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{ 
		if(user->user_info.user_ip == ip)
		{
			cvmx_rwlock_wp_read_unlock(head_lock);
			return RETURN_OK;
		}
		else
		{
			user = user->next; 
			if(user == NULL)
			{
				cvmx_rwlock_wp_read_unlock(head_lock);
				return RETURN_ERROR;
			}
			(*p_link_idx)++;
		}
	}
	return RETURN_ERROR;
}

/*************************************************************
 * Description:
 *         Get user information for fastfwd module 
 *         (Contain user index,user direct_flag)
 *
 * Parameter:
 *         rule_param   :  
 *         
 * Return:
 *        
 *************************************************************/
static inline int32_t fwd_get_user_idx(rule_param_t *rule_param)
{
	uint32_t sip_usr_idx = 0xffffffff;
	uint16_t sip_usr_link_idx = 0;
	uint32_t dip_usr_idx = 0xffffffff;
	uint16_t dip_usr_link_idx = 0;
	
	uint8_t sip_user_flag = 0;
	uint8_t dip_user_flag = 0;

    if(rule_param == NULL)
    {
        return RETURN_ERROR;
    }

	if(fwd_find_user_idx(rule_param->sip, &sip_usr_idx, &sip_usr_link_idx) == RETURN_OK)
	{
		sip_user_flag = 1;
	}

	if(fwd_find_user_idx(rule_param->dip, &dip_usr_idx, &dip_usr_link_idx) == RETURN_OK)
	{
		dip_user_flag = 1;
	}

	/* uplink */
	if((sip_user_flag == 1) && (dip_user_flag == 0))
	{
		rule_param->user_index = sip_usr_idx;
		rule_param->user_link_index = sip_usr_link_idx;
		rule_param->direct_flag = DIRECTION_UP;
		return RETURN_OK;
	}

	/* downlink */
	if((sip_user_flag == 0) && (dip_user_flag == 1))
	{
		rule_param->user_index = dip_usr_idx;
		rule_param->user_link_index = dip_usr_link_idx;
		rule_param->direct_flag = DIRECTION_DOWN;
		return RETURN_OK;
	}

	/* both uplink and downlink. e.g. user to user. 
	 in this situation, we define user_index and user_link_index to identify upload user, 
	 and user2_index and user2_link_index to identify download user. zhaohan@autelan.com */
	if((sip_user_flag == 1) && (dip_user_flag == 1))
	{
		rule_param->user_index = sip_usr_idx;
		rule_param->user_link_index = sip_usr_link_idx;
		rule_param->user2_index = dip_usr_idx;
		rule_param->user2_link_index = dip_usr_link_idx;		
		rule_param->direct_flag = DIRECTION_UP_DOWN;
		return RETURN_OK;
	}

	rule_param->user_index = 0xffffffff;
	rule_param->user_link_index = 0;
	rule_param->direct_flag = 0;
	return RETURN_ERROR;
}



/* add by zhaohan */
int32_t acl_self_learn_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache)
{
	rule_item_t  *rule ;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock = NULL;

	if((NULL == rule_para) || (NULL == cw_cache))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_self_learn_rule: Invalid argument\n");
		return RETURN_ERROR;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_self_learn_rule: packet-fiveTuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d,dport=%d, sport=%d,proto=%d. l2 type=0x%x, tag=0x%x \r\n",
			IP_FMT(rule_para->dip), IP_FMT(rule_para->sip), rule_para->dport,rule_para->sport,rule_para->protocol,rule_para->out_ether_type,rule_para->out_tag);

    /* get user table idx, store to rule */
    fwd_get_user_idx(rule_para);

	/*look up ACL Table and get the bucket*/
	hash(rule_para->dip, rule_para->sip, rule_para->protocol, rule_para->dport, rule_para->sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	cvmx_spinlock_lock(&rule->lock);
	head_lock = &rule->lock;


	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			/*表项为空，没有对应状态切换的表项*/
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_self_learn_rule: the acl table have no rule that need to be learned.\r\n");   
			cvmx_spinlock_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_self_learn_rule: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);    
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_self_learn_rule: lookup the table, rule=0x%p.\r\n",rule);   

		if ((rule->rules.dip == rule_para->dip) && 
				(rule->rules.sip == rule_para->sip) &&
				(rule->rules.dport == rule_para->dport) &&
				(rule->rules.sport == rule_para->sport) &&
				(rule->rules.protocol == rule_para->protocol))             
		{
			if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
				/*找到对应表项，学习*/
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}
#ifdef USER_TABLE_FUNCTION	
				set_acl_mask(head_rule);
#endif
				cvmx_spinlock_unlock(head_lock);
				return RETURN_OK;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
#if 0
				/*找到匹配项，根据策略决定属于更新还是重复配置*/
				if( rule->rules.XXXX == XXXXX)
				{
					/*策略不同，属于策略更新*/
				}
				else
				{
					/*学习的表项与以学习的表项冲突*/
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING, 
							"acl_self_learn_rule:conflict with the existing self learn rule 0x%p! \r\n",rule);
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;    
				}
#endif
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}
				
//#ifdef USER_TABLE_FUNCTION	
				//set_acl_mask(head_rule);
//#endif
				cvmx_spinlock_unlock(head_lock);                
				return RETURN_OK;
			}
			else
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;   
					CVMX_SYNC;
				}
				/*自学习的表项与静态表项冲突*/
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"acl_insert_static_rule:conflict with the static rule 0x%p! \r\n",rule);
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;    
			}
		}

		if(rule->next != NULL)
			rule = rule->next;
		else
			break;
	}

	/*表项为空，没有对应状态切换的表项*/
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_insert_eth_rule: the acl table have no rule that need to be learned.\r\n"); 
	cvmx_spinlock_unlock(head_lock);

	return RETURN_ERROR;  

}

int32_t acl_get_rule_stats(rule_stats_t* rule_stats)
{
	uint32_t s_tbl_used_rule = 0;
	uint32_t s_tbl_aged_rule = 0;
	uint32_t s_tbl_learned_rule = 0;
	uint32_t s_tbl_learning_rule = 0;
	uint32_t s_tbl_static_rule = 0;

	uint32_t d_tbl_used_rule = 0;
	uint32_t d_tbl_aged_rule = 0;
	uint32_t d_tbl_learned_rule = 0;
	uint32_t d_tbl_learning_rule = 0;
	uint32_t d_tbl_static_rule = 0;

	/* capwap table info */
	uint32_t cw_tbl_used = 0;
	uint32_t cw_tbl_802_3_num = 0;
	uint32_t cw_tbl_802_11_num = 0;
	union capwap_hd* cw_hdr;

	uint64_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;

	if(NULL == rule_stats)
	{
		printf("rule_stats is null!\n");
		return RETURN_ERROR;
	}

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		s_tbl_rule = (rule_item_t *)acl_bucket_tbl+i;

		if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
			switch(s_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					s_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					s_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					s_tbl_static_rule++;
					break;    
				default:
					break;
			}
			s_tbl_used_rule++;
			if(acl_aging_check(&(s_tbl_rule->rules)) > 0)
			{
				s_tbl_aged_rule++;
			}
		}

		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		while(d_tbl_rule != NULL)
		{
			switch(d_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					d_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					d_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					d_tbl_static_rule++;
					break;    
				default:
					break;
			}
			d_tbl_used_rule++;
			if(acl_aging_check(&(d_tbl_rule->rules)) > 0)
			{
				d_tbl_aged_rule++;
			}

			d_tbl_rule = d_tbl_rule->next;
		}
	}


	for(i = 0; i < MAX_CAPWAP_CACHE_NUM; i++)
	{ 
		if(capwap_cache_bl[i].use_num != 0)
		{
			cw_tbl_used++;
			cw_hdr = (union capwap_hd*)(capwap_cache_bl[i].cw_hd);
			if(cw_hdr->m_t == 0)
			{
				cw_tbl_802_3_num++;
			}            
			else if(cw_hdr->m_wbid == 1)
			{
				cw_tbl_802_11_num++;
			}
		}
	}

	/* static */
	rule_stats->acl_static_tbl_size = acl_static_tbl_size;
	rule_stats->s_tbl_used_rule = s_tbl_used_rule;
	rule_stats->s_tbl_aged_rule = s_tbl_aged_rule;
	rule_stats->s_tbl_learned_rule = s_tbl_learned_rule;
	rule_stats->s_tbl_learning_rule = s_tbl_learning_rule;
	rule_stats->s_tbl_static_rule = s_tbl_static_rule;        

	/* dynamic */
	rule_stats->acl_dynamic_tbl_size = acl_dynamic_tbl_size;
	rule_stats->d_tbl_used_rule = d_tbl_used_rule;
	rule_stats->d_tbl_aged_rule = d_tbl_aged_rule;
	rule_stats->d_tbl_learned_rule = d_tbl_learned_rule;
	rule_stats->d_tbl_learning_rule = d_tbl_learning_rule;
	rule_stats->d_tbl_static_rule = d_tbl_static_rule;

	/* cw */
	rule_stats->capwap_cache_tbl_size = MAX_CAPWAP_CACHE_NUM;
	rule_stats->cw_tbl_used = cw_tbl_used;
	rule_stats->cw_tbl_802_3_num = cw_tbl_802_3_num;
	rule_stats->cw_tbl_802_11_num = cw_tbl_802_11_num;

	return RETURN_OK;
}

/* clear all rules */
void acl_clear_rule(void)
{
	rule_item_t  *del_rule = NULL;
	rule_item_t  *head_rule = NULL;
	uint32_t i;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		head_rule = &acl_bucket_tbl[i];
		cvmx_spinlock_lock(&head_rule->lock);
		del_rule = head_rule->next;
		while(del_rule)
		{
			head_rule->next = del_rule->next;
			cvmx_free(del_rule);
			del_rule = head_rule->next;
		}
		memset(&head_rule->rules, 0, sizeof(rule_param_t));
		head_rule->valid_entries = 0;
		cvmx_spinlock_unlock(&head_rule->lock);
	}

    cvmx_spinlock_lock(&acl_mask_lock);
    memset((void *)pfast_acl_mask_tbl, 0, pfast_num*8);
    cvmx_spinlock_unlock(&acl_mask_lock);
  
	cvmx_spinlock_lock(&capwap_cache_bl_lock);
	memset((void *)capwap_cache_bl, 0, (MAX_CAPWAP_CACHE_NUM+1)*sizeof(capwap_cache_t));
	cvmx_spinlock_unlock(&capwap_cache_bl_lock);
}


/* clear all aging rules */
int acl_clear_aging_rule()
{
	rule_item_t  *del_rule_cur = NULL;
	rule_item_t  *del_rule_pre = NULL;
	rule_item_t  *head_rule = NULL;
	uint32_t i;

	for(i = 0; i < acl_static_tbl_size; i++)
	{
		head_rule = &acl_bucket_tbl[i];
		del_rule_cur = head_rule->next;
		del_rule_pre = head_rule;

		/* delete aging rules from list */
		cvmx_spinlock_lock(&head_rule->lock);
		while(del_rule_cur)
		{
			if((acl_aging_check(&(del_rule_cur->rules)) > 0) && (del_rule_cur->rules.rule_state != RULE_IS_STATIC))
			{
				acl_delete_rule_by_aging( head_rule, del_rule_pre,  del_rule_cur);
				del_rule_cur = del_rule_pre->next;
			}
			else
			{
				del_rule_pre = del_rule_cur;
				del_rule_cur = del_rule_cur->next;
			}
		}

		if(head_rule->rules.rule_state != RULE_IS_EMPTY)
		{
			if((acl_aging_check(&(head_rule->rules)) > 0) && (head_rule->rules.rule_state != RULE_IS_STATIC))
			{
				acl_delete_rule_by_aging( head_rule, head_rule,  head_rule);
			}
		}
		cvmx_spinlock_unlock(&head_rule->lock);
	}

	return RETURN_OK;
}


int32_t acl_self_learn_icmp_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache)
{
	rule_item_t  *rule = NULL;
	rule_item_t  *head_rule = NULL;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock = NULL;

	if((NULL == rule_para) || (NULL == cw_cache))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_self_learn_icmp_rule: Invalid argument\n");
		return RETURN_ERROR;
	}

	if(rule_para->protocol != CVM_COMMON_IPPROTO_ICMP)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_self_learn_icmp_rule: packet is not icmp type !\n");
		return RETURN_ERROR;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_self_learn_icmp_rule: packet-Three Tuple----dip=%d.%d.%d.%d, sip=%d.%d.%d.%d, proto=%d.  \r\n",
			IP_FMT(rule_para->dip), IP_FMT(rule_para->sip), rule_para->protocol);

    /* get user table idx, store to rule */
    fwd_get_user_idx(rule_para);

	/*look up ACL Table and get the bucket*/
	cvm_three_tupe_hash_lookup(rule_para->dip, rule_para->sip, rule_para->protocol);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	cvmx_spinlock_lock(&rule->lock);
	head_lock = &rule->lock;


	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			/*表项为空，没有对应状态切换的表项*/
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_self_learn_icmp_rule: the acl table have no rule that need to be learned.\r\n");   
			cvmx_spinlock_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_self_learn_icmp_rule: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);    
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_self_learn_rule: lookup the table, rule=0x%p.\r\n",rule);   

		if ((rule->rules.dip == rule_para->dip) && 
				(rule->rules.sip == rule_para->sip) &&
				(rule->rules.protocol == rule_para->protocol)) 
		{
			if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
				/*找到对应表项，学习*/
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}
				cvmx_spinlock_unlock(head_lock);
				return RETURN_OK;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
#if 0
				/*找到匹配项，根据策略决定属于更新还是重复配置*/
				if( rule->rules.XXXX == XXXXX)
				{
					/*策略不同，属于策略更新*/
				}
				else
				{
					/*学习的表项与以学习的表项冲突*/
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING, 
							"acl_self_learn_rule:conflict with the existing self learn rule 0x%p! \r\n",rule);
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;    
				}
#endif
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}

				//rule->rules.time_stamp = get_sec();
				cvmx_spinlock_unlock(head_lock);                
				return RETURN_OK;
			}
			else
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;   
					CVMX_SYNC;
				}
				/*自学习的表项与静态表项冲突*/
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"acl_insert_static_rule:conflict with the static rule 0x%p! \r\n",rule);
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;    
			}
		}

		if(rule->next != NULL)
			rule = rule->next;
		else
			break;
	}

	/*表项为空，没有对应状态切换的表项*/
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_insert_eth_rule: the acl table have no rule that need to be learned.\r\n"); 
	cvmx_spinlock_unlock(head_lock);

	return RETURN_ERROR;  

}


int32_t acl_insert_static_icmp_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache)
{
	return RETURN_ERROR;
}


/* add by zhaohan */
int32_t acl_one_tuple_self_learn_rule(rule_param_t *rule_para, capwap_cache_t *cw_cache)
{
	rule_item_t  *rule = NULL;
	rule_item_t  *head_rule=NULL ;
	rule_item_t  *free_rule  = NULL; /*the first free bucket position*/
	cvmx_spinlock_t          *head_lock;

	if((NULL == rule_para) || (NULL == cw_cache))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING,
				"acl_one_tuple_self_learn_rule: Invalid argument\n");
		return RETURN_ERROR;
	}

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_one_tuple_self_learn_rule: packet-oneTuple----dip=%d.%d.%d.%d  \r\n",
			IP_FMT(rule_para->dip));

	/*look up ACL Table and get the bucket*/
	cvm_ip_hash_lookup(rule_para->dip);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));

	head_rule = rule;
	cvmx_spinlock_lock(&rule->lock);
	head_lock = &rule->lock;


	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_rule->rules.rule_state == RULE_IS_EMPTY)
	{
		if(head_rule->valid_entries == 0)
		{
			/*表项为空，没有对应状态切换的表项*/
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_self_learn_rule: the acl table have no rule that need to be learned.\r\n");   
			cvmx_spinlock_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_rule = head_rule; /*record current free bucket position*/

			if(head_rule->next != NULL)
			{
				rule = head_rule->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"acl_self_learn_rule: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,rule,rule->valid_entries,rule->next);    
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"acl_self_learn_rule: lookup the table, rule=0x%p.\r\n",rule);   

		if (rule->rules.dip == rule_para->dip) 
		{
			if(rule->rules.rule_state == RULE_IS_LEARNING)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
				/*找到对应表项，学习*/
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}
				cvmx_spinlock_unlock(head_lock);
				return RETURN_OK;
			}
			else if(rule->rules.rule_state == RULE_IS_LEARNED)
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;
					CVMX_SYNC;
				}
#if 0
				/*找到匹配项，根据策略决定属于更新还是重复配置*/
				if( rule->rules.XXXX == XXXXX)
				{
					/*策略不同，属于策略更新*/
				}
				else
				{
					/*学习的表项与以学习的表项冲突*/
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_WARNING, 
							"acl_self_learn_rule:conflict with the existing self learn rule 0x%p! \r\n",rule);
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;    
				}
#endif
				if(acl_fill_rule(&(rule->rules), rule_para, cw_cache) == RETURN_ERROR)
				{
					cvmx_spinlock_unlock(head_lock);
					return RETURN_ERROR;
				}

				//rule->rules.time_stamp = get_sec();
				cvmx_spinlock_unlock(head_lock);                
				return RETURN_OK;
			}
			else
			{
				/*add by sunjc@autelan.com*/
				if(rule->rules.packet_wait > 0)
				{
					rule->rules.packet_wait--;   
					CVMX_SYNC;
				}
				/*自学习的表项与静态表项冲突*/
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
						"acl_insert_static_rule:conflict with the static rule 0x%p! \r\n",rule);
				cvmx_spinlock_unlock(head_lock);
				return RETURN_ERROR;    
			}
		}

		if(rule->next != NULL)
			rule = rule->next;
		else
			break;
	}

	/*表项为空，没有对应状态切换的表项*/
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"acl_insert_eth_rule: the acl table have no rule that need to be learned.\r\n"); 
	cvmx_spinlock_unlock(head_lock);

	return RETURN_ERROR;  

}

/*
 * user online, notify by fccp.
 * arg: user_ip 
 */
int32_t user_action_online(uint32_t user_ip)
{
	uint32_t user_index = 0xffffffff;
	uint32_t user_link_index = 0;
	user_item_t  *user = NULL;
	user_item_t  *free_user  = NULL; /*the first free bucket position*/
	user_item_t  *head_user = NULL;	
	cvmx_rwlock_wp_lock_t *head_lock = NULL;

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"user_action_online: user %d.%d.%d.%d is online \r\n", IP_FMT(user_ip));

	/*look up user Table and get the bucket*/
	user_index = user_cache_bucket_lookup(user_ip);
	user = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));

	head_user = user;
	head_lock = &head_user->lock;
	cvmx_rwlock_wp_write_lock(head_lock);

	/*if the first bucket is empty and there are no more buckets, then insert current flow*/
	if(head_user->user_info.user_state == USER_IS_EMPTY)
	{
		if(head_user->valid_entries == 0)
		{
			head_user->user_info.user_ip = user_ip;
			head_user->user_info.user_state = USER_IS_ONLINE;
			head_user->valid_entries++;
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_OK;
		}
		else /*first bucket is empty but with other buckets*/
		{
			free_user = head_user; /*record current free bucket position*/

			if(head_user->next != NULL)
			{
				user = head_user->next;
				user_link_index++;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"user_action_online: Should never come to here file%s, line %d, rule=0x%p, num=0x%d,next=0x%p.\r\n",__FILE__, __LINE__,head_user,head_user->valid_entries,head_user->next);	
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		if(user == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"user_action_online: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_ERROR;
		}

		if(user->user_info.user_ip == user_ip)
		{
			if(user->user_info.user_state == USER_IS_ONLINE)
			{
				cvmx_atomic_set64((int64_t *)(&user->user_info.forward_up_packet), 0);
				cvmx_atomic_set64((int64_t *)(&user->user_info.forward_up_bytes), 0);
				cvmx_atomic_set64((int64_t *)(&user->user_info.forward_down_packet), 0);
				cvmx_atomic_set64((int64_t *)(&user->user_info.forward_down_bytes), 0);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"user_action_online: user %d.%d.%d.%d has already online \r\n", IP_FMT(user_ip));
				cvmx_rwlock_wp_write_unlock(head_lock);
				//fast_del_rule_by_ip(user_ip);
				return RETURN_ERROR;
			}
			else if(user->user_info.user_state == USER_IS_OFFLINE)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"user_action_online: user %d.%d.%d.%d is offline, reonline\r\n", IP_FMT(user_ip));
				head_user->user_info.user_state = USER_IS_ONLINE;       
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_OK;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"user_action_online: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
		else
		{
			if(user->next != NULL)
			{
				user = user->next;
				user_link_index++;
			}
			else
			{
				if(free_user == NULL)
				{
					/* need malloc a node */
					user->next = (user_item_t *)cvmx_malloc(user_table_arena, sizeof(user_item_t));
					if(user->next == NULL)
					{
						FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_ERROR, 
								"Insert user: Memory not available for adding user\n");
						cvmx_rwlock_wp_write_unlock(head_lock);
						return RETURN_ERROR;
					}	
					memset(user->next, 0, sizeof(user_item_t));
					free_user = user->next;
				}

				/* free_user == head_bucket, fill head_bucket  */
				free_user->user_info.user_ip = user_ip;
				free_user->user_info.user_state = USER_IS_ONLINE;
				head_user->valid_entries++;
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_OK;
			}
		}

	}

}

/*
 * user offline, notify by fccp.
 * delete user form user table, and remove all rules form acl table.
 * arg: user_ip
 */
int32_t user_action_offline(uint32_t user_ip)
{
	uint32_t user_index = 0xffffffff;
	uint32_t user_link_index = 0;
	user_item_t  *user = NULL;
	user_item_t  *user_prev = NULL;
	user_item_t  *head_user = NULL;	
	cvmx_rwlock_wp_lock_t *head_lock = NULL;

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"user_action_online: user %d.%d.%d.%d is online \r\n", IP_FMT(user_ip));

	/*look up user Table and get the bucket*/
	user_index = user_cache_bucket_lookup(user_ip);
	user = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));

	head_user = user;
	user_prev = user;
	head_lock = &head_user->lock; 
	cvmx_rwlock_wp_write_lock(head_lock);

	/* */
	if(head_user->user_info.user_state == USER_IS_EMPTY)
	{
		if(head_user->valid_entries == 0)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"user_action_offline: can't find user %d.%d.%d.%d\r\n", IP_FMT(user_ip));
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			if(head_user->next != NULL)
			{
				user = head_user->next;
				user_link_index++;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"user_stats_clear: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		if(user == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"user_action_offline: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_ERROR;
		}

		if(user->user_info.user_ip == user_ip)
		{
			if(user->user_info.user_state == USER_IS_OFFLINE)
			{
				/* Warning: user has already offline */
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_ERROR;
			}
			else if(user->user_info.user_state == USER_IS_ONLINE)
			{
				if(user == head_user)    /* head bucket, zero it */
				{
					memset(&user->user_info, 0, sizeof(user_info_t));
				}
				else    /* in list, remove and free it */
				{
					user_prev->next = user->next;
					cvmx_free(user);
					user = NULL;
				}
				head_user->valid_entries--;
				cvmx_rwlock_wp_write_unlock(head_lock);
				fast_del_rule_by_ip(user_ip); /* del all rules belong to this user */	
				return RETURN_OK;
			}
		}
		else
		{
			user_prev = user;
			user = user->next;
			user_link_index++;
		}

	}

	return RETURN_OK;
}


/*
 * Users statistical information reset. notify by fccp packets
 */
int32_t user_stats_clear(uint32_t user_ip)
{
	user_item_t  *user = NULL;
	user_item_t  *head_user = NULL;
	cvmx_rwlock_wp_lock_t *head_lock = NULL;

	/*look up user Table and get the bucket*/
	user_cache_bucket_lookup(user_ip);
	user = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));

	head_user = user;
	head_lock = &head_user->lock; 
	cvmx_rwlock_wp_write_lock(head_lock);

	if(head_user->user_info.user_state == USER_IS_EMPTY)
	{
		if(head_user->valid_entries == 0)
		{
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_ERROR;
		}
		else /*first bucket is empty but with other buckets*/
		{
			if(head_user->next != NULL)
			{
				user = head_user->next;
			}
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
						"user_stats_clear: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
				cvmx_rwlock_wp_write_unlock(head_lock);
				return RETURN_ERROR;
			}
		}
	}

	while(1)
	{
		if(user == NULL)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_FLOWTABLE, FASTFWD_COMMON_DBG_LVL_MUST_PRINT,
					"user_stats_clear: Should never come to here file%s, line %d.\n",__FILE__, __LINE__);
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_ERROR;
		}

		/* zero */
		if(user->user_info.user_ip == user_ip)
		{
			cvmx_atomic_set64((int64_t *)(&user->user_info.forward_up_packet), 0);
			cvmx_atomic_set64((int64_t *)(&user->user_info.forward_up_bytes), 0);
			cvmx_atomic_set64((int64_t *)(&user->user_info.forward_down_packet), 0);
			cvmx_atomic_set64((int64_t *)(&user->user_info.forward_down_bytes), 0);            
			cvmx_rwlock_wp_write_unlock(head_lock);
			return RETURN_OK;
		}
		else
		{
			user = user->next;
		}

	}

	return RETURN_OK;

}

/*
 * find usr index from user tbl by user_ip
 * 
 */
static inline user_item_t* fwd_find_user(uint32_t ip, cvmx_rwlock_wp_lock_t** p_user_lock)
{
	user_item_t *head_user = NULL;
	user_item_t *user = NULL;
	cvmx_rwlock_wp_lock_t *head_lock = NULL;

	user_cache_bucket_lookup(ip);
	head_user = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));
	user = head_user;
	head_lock = &head_user->lock;
	*p_user_lock = head_lock;   
	cvmx_rwlock_wp_read_lock(head_lock);

	if(head_user->user_info.user_state == USER_IS_EMPTY)
	{
		if(head_user->valid_entries == 0)
		{
			cvmx_rwlock_wp_read_unlock(head_lock);
			return NULL;
		}
		else /*first bucket is empty but with other buckets*/
		{
			if(head_user->next != NULL)
			{
				user = head_user->next;
			}
			else
			{
				cvmx_rwlock_wp_read_unlock(head_lock);
				return NULL;
			}
		}
	}

	while(1)
	{ 
		if(user->user_info.user_ip == ip)
		{
			cvmx_rwlock_wp_read_unlock(head_lock);
			return user;
		}
		else
		{ 
			user = user->next; 
			if(user == NULL)
			{
				cvmx_rwlock_wp_read_unlock(head_lock);
				return NULL;
			}
		}
	}

	return NULL;
}


/* calculate_user_paload */
static inline uint32_t calculate_user_paload(cvmx_wqe_t *work, cvm_common_ip_hdr_t *ip)
{
	uint32_t user_payload = 0;
	uint8_t *pkt_ptr = NULL;
	pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);

	/* calculate user paload */
	user_payload = CVM_WQE_GET_LEN(work) - ((uint8_t *)ip - pkt_ptr);
	if(FUNC_ENABLE == cvm_pure_payload_acct)
	{
		user_payload -= (ip->ip_hl << 2);/*decrease ip header length*/
	}
	else
	{
		user_payload += ETH_H_LEN;/*add L2 header length*/
	}

	return user_payload;
}

/*
 * user packets statistics
 * 
 */
void user_flow_statistics_process
(
 cvmx_wqe_t *work, 
 rule_item_t *rule,
 cvm_common_ip_hdr_t *true_ip
 )
{
	user_item_t* user = NULL;
	cvmx_rwlock_wp_lock_t* user_lock = NULL;
	uint32_t user_payload = 0;	

	if(NULL == true_ip)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"user_flow_statistics_process: true_ip is NULL!\n");
		return;
	}

	/* calculate user paload */
	user_payload = calculate_user_paload(work, true_ip);

	/* count pkts which we can fast-forward */
	if((NULL != rule) && 
			((rule->rules.action_type == FLOW_ACTION_ETH_FORWARD) ||
			 (rule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD) ||
			 (rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
			 (rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD) ||
			 (rule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD) ||
			 (rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP)))
	{
		if( rule->rules.direct_flag == DIRECTION_UP)
		{
			user = get_user_item(rule->rules.user_index, rule->rules.user_link_index);
			if(user != NULL){
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_packet)), 1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_bytes)), user_payload);
			}
		}
		else if(rule->rules.direct_flag == DIRECTION_DOWN)
		{
			user = get_user_item(rule->rules.user_index, rule->rules.user_link_index);
			if(user != NULL){
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_packet)),1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_bytes)),user_payload);
			}
		}
		else if(rule->rules.direct_flag == DIRECTION_UP_DOWN)   /* user to user */
		{
			/* user_index and user_link_index identify upload user */
			user = get_user_item(rule->rules.user_index, rule->rules.user_link_index);
			if(user != NULL)
			{
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_packet)),1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_bytes)),user_payload);
			}

			/* user2_index and user2_link_index identify download user */
			user = get_user_item(rule->rules.user2_index, rule->rules.user2_link_index);
			if(user != NULL)
			{
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_packet)),1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_bytes)),user_payload);
			}					
		}		
	}
	/* Damn it! don't forward,but need statistics. shit! */
	else
	{
		/* src ip, forward up */
		if ((user = fwd_find_user(true_ip->ip_src, &user_lock)) != NULL) 
		{
			if(user != NULL) 
			{ 
				cvmx_rwlock_wp_write_lock(user_lock);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_packet)), 1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_up_bytes)), user_payload);
				cvmx_rwlock_wp_write_unlock(user_lock);
			}
		}
		/* dst ip, forward down */
		if ((user = fwd_find_user(true_ip->ip_dst, &user_lock)) != NULL) 
		{
			if(user != NULL) 
			{
				cvmx_rwlock_wp_write_lock(user_lock); 
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_packet)),1);
				cvmx_atomic_add64((int64_t *)(&(user->user_info.forward_down_bytes)),user_payload);			
				cvmx_rwlock_wp_write_unlock(user_lock);
			}
		}
	}

	return;
}


int fastfwd_show_fau64(fau64_info_t *fau64_info)
{
    if(NULL == fau64_info)
        return RETURN_ERROR;

    fau64_info->fau_pko_errors=cvmx_fau_fetch_and_add64(CVM_FAU_PKO_ERRORS, 0);
	fau64_info->fau_enet_input_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_BYTES, 0);
	fau64_info->fau_enet_input_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_PACKETS, 0);
	fau64_info->fau_enet_output_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_BYTES, 0);
	fau64_info->fau_enet_output_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS, 0);
	fau64_info->fau_enet_drop_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_DROP_PACKETS, 0);
	fau64_info->fau_enet_to_linux_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 0);
	fau64_info->fau_enet_nonip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_NONIP_PACKETS, 0);
	fau64_info->fau_enet_error_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_ERROR_PACKETS, 0);
	fau64_info->fau_enet_fragip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_FRAGIP_PACKETS, 0);
	fau64_info->fau_ip_short_packets=cvmx_fau_fetch_and_add64(CVM_FAU_IP_SHORT_PACKETS, 0);
	fau64_info->fau_ip_bad_hdr_len=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_HDR_LEN, 0);
	fau64_info->fau_ip_bad_len=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_LEN, 0);
	fau64_info->fau_ip_bad_version=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_VERSION, 0);
	fau64_info->fau_ip_skip_addr=cvmx_fau_fetch_and_add64(CVM_FAU_IP_SKIP_ADDR, 0);
	fau64_info->fau_ip_icmp=cvmx_fau_fetch_and_add64(CVM_FAU_IP_ICMP, 0);
	fau64_info->fau_capwap_icmp=cvmx_fau_fetch_and_add64(CVM_FAU_CAPWAP_ICMP, 0);
	fau64_info->fau_ip_proto_error=cvmx_fau_fetch_and_add64(CVM_FAU_IP_PROTO_ERROR, 0);
	fau64_info->fau_udp_bad_dropt=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_DPORT, 0);
	fau64_info->fau_udp_bad_len=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_LEN, 0);
	fau64_info->fau_udp_to_linux=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_TO_LINUX, 0);
	fau64_info->fau_flowtable_hit_packets=cvmx_fau_fetch_and_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 0);
	fau64_info->fau_enet_output_packets_8021q=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 0);
	fau64_info->fau_enet_output_packets_qinq=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 0);
	fau64_info->fau_flow_lookup_error=cvmx_fau_fetch_and_add64(CVM_FAU_FLOW_LOOKUP_ERROR, 0);
	fau64_info->fau_recv_fccp=cvmx_fau_fetch_and_add64(CVM_FAU_RECV_FCCP_PACKETS,0);
	fau64_info->fau_recv_works=cvmx_fau_fetch_and_add64(CVM_FAU_RECV_TOTAL_WORKS,0);
	fau64_info->fau_acl_lookup=cvmx_fau_fetch_and_add64(CVM_FAU_TOTAL_ACL_LOOKUP,0);
	fau64_info->fau_acl_reg=cvmx_fau_fetch_and_add64(CVM_FAU_ACL_REG,0);
	fau64_info->fau_cw802_11_decap_err=cvmx_fau_fetch_and_add64(CVM_FAU_CW802_11_DECAP_ERROR,0);
	fau64_info->fau_cw802_3_decap_err=cvmx_fau_fetch_and_add64(CVM_FAU_CW802_3_DECAP_ERROR,0);
	fau64_info->fau_enet_to_linux_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_BYTES,0);
	fau64_info->fau_alloc_rule_fail=cvmx_fau_fetch_and_add64(CVM_FAU_ALLOC_RULE_FAIL,0);
	fau64_info->fau_max_rule_entries=cvmx_fau_fetch_and_add64(CVM_FAU_MAX_RULE_ENTRIES,0);
	fau64_info->fau_cw_80211_err = cvmx_fau_fetch_and_add64(CVM_FAU_CW_80211_ERR,0);
	fau64_info->fau_cw_noip_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_NOIP_PACKETS,0);
	fau64_info->fau_cw_spe_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_SPE_PACKETS,0);
	fau64_info->fau_cw_frag_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_FRAG_PACKETS,0);
    fau64_info->fau_mcast_packets = cvmx_fau_fetch_and_add64(CVM_FAU_MCAST_PACKETS,0);
    fau64_info->fau_rpa_packets = cvmx_fau_fetch_and_add64(CVM_FAU_RPA_PACKETS,0);
    fau64_info->fau_rpa_tolinux_packets = cvmx_fau_fetch_and_add64(CVM_FAU_RPA_TOLINUX_PACKETS,0);
    fau64_info->fau_tipc_packets = cvmx_fau_fetch_and_add64(CVM_FAU_TIPC_PACKETS,0);
    fau64_info->fau_large_eth2cw_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_ETH2CW_PACKET,0);
    fau64_info->fau_large_cw_rpa_fwd_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW_RPA_FWD_PACKET,0);
    fau64_info->fau_large_cw8023_rpa_fwd_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW8023_RPA_FWD_PACKET,0);
    fau64_info->fau_spe_tcp_hdr = cvmx_fau_fetch_and_add64(CVM_FAU_SPE_TCP_HDR,0);
    fau64_info->fau_cw_spe_tcp_hdr = cvmx_fau_fetch_and_add64(CVM_FAU_CW_SPE_TCP_HDR,0);

    return RETURN_OK;
}
#endif
