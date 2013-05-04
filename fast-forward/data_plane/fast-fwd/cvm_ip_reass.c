#include <stdio.h>
#include <string.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
//#include "cvmx-ciu.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-dfa.h"
#include "cvmx-pow.h"
#include "cvmx-gmx.h"
//#include "cvmx-asx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"
#include "cvmx-tim.h"

#ifdef SDK_VERSION_2_2
#include "fastfwd-common-rnd.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-defs.h"
#include "cvm-common-fpa.h"
#endif
#include "autelan_product_info.h"
#include "shell.h"
#include "acl.h"
#include "fwd_main.h"
#include "cvm-ip-in.h"
#include "capwap.h"
#include "fccp.h"
#include "car.h"

#include "cvm_ip_reass.h"


CVMX_SHARED hash_item_t *cvm_ip_reass_hash_table_base = NULL;
CVMX_SHARED cvmx_spinlock_t *cvm_ip_spinlock_table = NULL;


void cvm_ip_reass_init()
{
	cvm_ip_reass_hash_table_base = (hash_item_t *)cvmx_bootmem_alloc(sizeof(hash_item_t)*DISPATCHHASHSIZE, 8);
	CVM_COMMON_KASSERT(cvm_ip_reass_hash_table_base,  ("cvm_ip_reass_init: dispatch_reass_hash_table alloc failed"));
    
	cvm_ip_spinlock_table = (cvmx_spinlock_t *)cvmx_bootmem_alloc(sizeof(cvmx_spinlock_t)*128, sizeof(cvmx_spinlock_t));
    CVM_COMMON_KASSERT(cvm_ip_spinlock_table,  ("cvm_ip_reass_init: spinlock alloc failed"));

    printf("cvm_ip_reass_hash_table_base size = %d\n", (int)sizeof(hash_item_t)*DISPATCHHASHSIZE);
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_init:cvm_ip_reass_hash_table address: %p\n", cvm_ip_reass_hash_table_base);    
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_init:cvm_ip_reass_hash_table size in bytes:%ld\n", sizeof(hash_item_t)*DISPATCHHASHSIZE);
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_init:cvm_ip_spinlock_table address:%p\n", cvm_ip_spinlock_table);
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_init:cvm_ip_spinlock_table size in bytes:%ld\n", sizeof(cvmx_spinlock_t)*128);

    
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_init:cpu-%uhz\n", cvmx_sysinfo_get()->cpu_clock_hz);
	memset(cvm_ip_reass_hash_table_base, 0, sizeof(hash_item_t)*DISPATCHHASHSIZE);
    memset(cvm_ip_spinlock_table, 0, sizeof(cvmx_spinlock_t)*128);

    int i = 0;
    for(i = 0; i<128; i++)
    {
        cvmx_spinlock_init(cvm_ip_spinlock_table + i);
    }
}

uint32_t cvm_ip_reass_bucket_lookup(cvm_common_ip_hdr_t *ip,  uint16_t extra_id)
{
   uint32_t idx;
   #if 0
   uint16_t ip_id = ip->ip_id;
   uint8_t proto = ip->ip_p;
   
   CVMX_MT_CRC_POLYNOMIAL(ZCTT_DISPATCH_REASS_CRC_POLYNOMIAL);
   CVMX_MT_CRC_IV(0);
   CVMX_MT_CRC_WORD(*(uint32_t*)(&ip->ip_src));
   CVMX_MT_CRC_WORD(*(uint32_t*)(&ip->ip_dst));
   CVMX_MT_CRC_HALF(*(uint16_t*)(&ip_id));
   CVMX_MT_CRC_HALF(extra_id);
   CVMX_MT_CRC_BYTE(*(uint8_t*)(&proto));
   CVMX_MF_CRC_IV(idx);
   
   // Lookup entry found
   #endif
   idx = (ip->ip_src)^(ip->ip_dst)^(ip->ip_id)^(ip->ip_p);
   return idx;
}

#if 0
/**
 *查询某个数据包的分流模式 是否存在于hash表中；若存在，以指针形式
 *返回；否则，返回丢包的分流模式
 * param  work 查询该数据包
 * param  dispatch_infor  承载数据包的分流模式
 */
void cvm_ip_reass_hash_finditem(cvmx_wqe_t *work, dispatch_info  *dispatch_infor)
{
	struct cvm_common_ip_hdr *ip = NULL;
	hash_inode_t  *inode = NULL;
	uint32_t idx = 0, i = 0;

	if(work == NULL || dispatch_infor == NULL)
	{
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR, "cvm_ip_reass_hash_finditem: Param is NULL Pointer!\n");
        return;
    }
    ip = (struct cvm_common_ip_hdr *)&work->packet_data[CVM_COMMON_PD_ALIGN];
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_finditem: Packet:srcIp-0x%x,destIp-0x%x. \r\n", ip->ip_src,ip->ip_dst);

	idx = cvm_ip_reass_bucket_lookup(ip, 0);
	idx = idx &(DISPATCHHASHSIZE-1);	
	inode = ((hash_item_t *)cvm_ip_reass_hash_table_base + idx)->arrayItem;
	/*预设分流模式，若无法命中，为丢包模式；若命中，返回相应模式*/
	dispatch_infor->action = FLOW_ACTION_DROP;

	/*固定数组里查询*/
	for(i = 0; i < HASH_ARRAY_ITEM_NUM; i++)
	{ 
		if((ip->ip_dst == inode[i].destAddr)  && ( ip->ip_src == inode[i].srcAddr) &&
		   (ip->ip_id == inode[i].identifcation) && (ip->ip_p == inode[i].protocol))
		{
            FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_finditem: Found at inode[%d]--\n", i);
			dispatch_infor->action = inode[i].action;
			dispatch_infor->CpuMask = inode[i].coremask;
			dispatch_infor->PortMask = inode[i].portmask;	
//#ifdef ZCTT_ENET_STATISTICS
//		        cvmx_fau_atomic_add64(CVM_FAU_IP_FRAGS_HIT_TBL_NUM, 1);
//#endif 
			break;
		}
	}
	if(i == HASH_ARRAY_ITEM_NUM)    
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,"cvm_ip_reass_hash_finditem: Packet:srcIp-0x%x,destIp-0x%x.Inode not found!Packet will be droped!\n",ip->ip_src,ip->ip_dst);   
}
#endif

/**
 * 删除制定数据包对应的hash表项
 * param  work 制定数据包
 */
void cvm_ip_reass_hash_removeitem(cvm_common_ip_hdr_t *ip)
{
	uint32_t	 idx = 0, i = 0;
	hash_inode_t  *inode = NULL;

	if(ip == NULL)
	{
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING, "cvm_ip_reass_hash_finditem: work is NULL!\n");
        return;
    }
    
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_finditem: Packet: srcIp-0x%x,destIp-0%x \r\n", ip->ip_src,ip->ip_dst);

	idx = cvm_ip_reass_bucket_lookup(ip, 0);
	idx = idx & ( DISPATCHHASHSIZE - 1);
	inode = ((hash_item_t *)cvm_ip_reass_hash_table_base + idx)->arrayItem;

	//cvmx_spinlock_lock(&cvm_ip_spinlock_table[idx>>9]);
	for(i = 0; i < HASH_ARRAY_ITEM_NUM; i++)
	{
		if((ip->ip_dst == inode[i].destAddr)&&( ip->ip_src == inode[i].srcAddr) &&
		   (ip->ip_id == inode[i].identifcation) && (ip->ip_p == inode[i].protocol))
		{
		//printf("cvm_ip_reass_hash_removeitem sucuss !!\n");
		    memset(inode + i, 0, sizeof(hash_inode_t));
			break;
		}
	}
	//cvmx_spinlock_unlock(&cvm_ip_spinlock_table[idx>>9]);
	if(i == HASH_ARRAY_ITEM_NUM)
	{
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_finditem: Remove item failed!Not found!\n");
	}
	else
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_finditem: remove item-%u successfully!\n", i);

}
/**
 * 添加表项到hash表	
 * param 	work 相关数据包的wqe数据指针
 * param		dispatch_infor 需添加的数据包的分流模式
*/
void cvm_ip_reass_hash_additem(cvmx_wqe_t	*work, uint32_t action_type,
		cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th,rule_item_t *prule,uint8_t is_qos,uint8_t is_pppoe)
{
    uint8_t timestamp = 0, uTimeDistance = 0, uMaxTimeDistance = 0;
	int i = 0, maxStampIndex = 0;
	uint32_t  idx = 0;
	uint64_t globalCycles = 0;
	hash_inode_t	*inode = NULL;	
	rule_item_t *rule = NULL;
	cvmx_spinlock_t   *first_lock =NULL;

	//if(work == NULL || dispatch_infor == NULL)
	if(work == NULL)
	{
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING, "cvm_ip_reass_hash_additem: Param is NULL Pointer!\n");
        return;
    }
    
	//struct cvm_common_ip_hdr  *ip = (struct cvm_common_ip_hdr_t *) &work->packet_data[CVM_COMMON_PD_ALIGN];
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_additem: srcIp-0x%x,destIp-0x%x,id-%d,proto-%d\r\n", ip->ip_src,ip->ip_dst,ip->ip_id,ip->ip_p);

	/*计算hash 值*/
	idx = cvm_ip_reass_bucket_lookup(ip, 0);
	idx = idx & ( DISPATCHHASHSIZE - 1);	
	inode = (hash_inode_t *)((hash_item_t  *)cvm_ip_reass_hash_table_base + idx)->arrayItem;
	
	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_additem: hashIndex--%d, spinLockIndex--%d\n",idx, idx>>9);
	
	//globalCycles = cvmx_get_cycle_global();
	globalCycles = cvmx_get_cycle();
        
	/*计算当前时间戳*/
	timestamp = (uint8_t)(globalCycles/cvmx_sysinfo_get()->cpu_clock_hz)% MAXTIMESTAMP;
	cvmx_spinlock_lock(&cvm_ip_spinlock_table[idx>>9]);
	
	/*遍历数组，查找超时元素，覆盖*/
	for(i = 0; i < HASH_ARRAY_ITEM_NUM; i++)
	{   
	    uTimeDistance = (uint8_t)(((timestamp - inode[i].timestamp)+ MAXTIMESTAMP)% MAXTIMESTAMP);
        
           FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "destAddr-%x,istamp-%d,cstamp-%d,timeDistance-%d\n", inode[i].destAddr,inode[i].timestamp,timestamp,uTimeDistance);
	    
		/*填充时间戳，四元组，分流模式*/
		// if uTimeDistance >= TIME_DISTANCE must free wqe and packet
		if (((0 == inode[i].destAddr) || 
			(uTimeDistance >= TIME_DISTANCE)) ||
			((inode[i].destAddr == ip->ip_dst) ||
			(inode[i].srcAddr == ip->ip_src) ||
			(inode[i].identifcation == ip->ip_id) ||
			(inode[i].protocol == ip->ip_p)))
		{
            		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_additem: add to index[%d]!\n", i);
			inode[i].timestamp = timestamp;
			inode[i].destAddr = ip->ip_dst;
			inode[i].srcAddr = ip->ip_src;
			inode[i].identifcation = ip->ip_id;
			inode[i].protocol = ip->ip_p;
			inode[i].th_sport = th->th_sport;
			inode[i].th_dport = th->th_dport;
			//inode[i].portmask = dispatch_infor->PortMask;
			//inode[i].coremask = dispatch_infor->CpuMask;
			//inode[i].action = dispatch_infor->action;
			inode[i].work = work;
			// if a header packet send
			//if ((FRAGMENT_MORE(ip->ip_off) != 0) && 
				//(FRAGMENT_OFFSET(ip->ip_off) == 0)) {
				rule = acl_table_lookup(ip, th, &first_lock);
				inode[i].rule = rule;
				if (rule == NULL) {
					action_type = FLOW_ACTION_TOLINUX;			
				}
				
				//printf("cvm_ip_reass_hash_additem: STORE at inode[%d] rule %x\n", i, rule->rules.rule_state);
				flow_action_process(inode[i].work, action_type, ip, th, rule, is_qos, first_lock, is_pppoe);
			//}
			break;
		}
	
	    uMaxTimeDistance = (uint8_t)(((timestamp - inode[maxStampIndex].timestamp)+ MAXTIMESTAMP)%MAXTIMESTAMP);
		if(uTimeDistance > uMaxTimeDistance)
		    maxStampIndex = i;		
	}
	//if(HASH_ARRAY_ITEM_NUM == i)
	if((HASH_ARRAY_ITEM_NUM - 1) == maxStampIndex)
	{
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_additem: No inode out-of-time! Index %d stay longest!\n", maxStampIndex);
	    inode[maxStampIndex].timestamp = timestamp;	    
        inode[maxStampIndex].destAddr = ip->ip_dst;
        inode[maxStampIndex].srcAddr = ip->ip_src;
        inode[maxStampIndex].identifcation = ip->ip_id;
        inode[maxStampIndex].protocol = ip->ip_p;      
        //inode[maxStampIndex].portmask = dispatch_infor->PortMask;
        //inode[maxStampIndex].coremask = dispatch_infor->CpuMask;
        //inode[maxStampIndex].action = dispatch_infor->action;
		inode[maxStampIndex].work = work;
		inode[maxStampIndex].th_sport = th->th_sport;
		inode[maxStampIndex].th_dport = th->th_dport;
		// if a header packet send to linux, outer do this
		//if ((FRAGMENT_MORE(ip->ip_off) == 1) && 
			//(FRAGMENT_OFFSET(ip->ip_off) == 0)) {
			rule = acl_table_lookup(ip, th, &first_lock);
			inode[i].rule = rule;
			if (rule == NULL) {
				action_type = FLOW_ACTION_TOLINUX;			
			}
			
			//printf( "cvm_ip_reass_hash_additem: No inode out-of-time! Index %d stay longest!\n", i);
			flow_action_process(inode[maxStampIndex].work, action_type, ip, th, rule, is_qos, first_lock, is_pppoe);
		//}
	}   
	
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_additem:add inode successfully!\n");
	cvmx_spinlock_unlock(&cvm_ip_spinlock_table[idx>>9]);
}

rule_item_t * cvm_ip_reass_hash_processitem(cvmx_wqe_t *work, cvm_common_ip_hdr_t *iph)
{
	hash_inode_t  *inode = NULL;
	uint32_t idx = 0, i = 0;
	rule_item_t *rule = NULL;
	cvm_common_tcp_hdr_t th;
	cvmx_spinlock_t   *first_lock =NULL;

	//if(work == NULL || dispatch_infor == NULL
	if(work == NULL)
	{
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING, "cvm_ip_reass_hash_processitem: Param is NULL Pointer!\n");
        return NULL;
    }
	
	//pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
	//ip = (cvm_common_ip_hdr_t *)(pkt_ptr + work->word2.s.ip_offset); 
    //ip = (struct cvm_common_ip_hdr *)&work->packet_data[CVM_COMMON_PD_ALIGN];
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_processitem: Packet:srcIp-0x%x,destIp-0x%x. \r\n", iph->ip_src,iph->ip_dst);

	idx = cvm_ip_reass_bucket_lookup(iph, 0);
	idx = idx &(DISPATCHHASHSIZE-1);	
	inode = ((hash_item_t *)cvm_ip_reass_hash_table_base + idx)->arrayItem;
	/*预设分流模式，若无法命中，为丢包模式；若命中，返回相应模式*/
	//dispatch_infor->action = FLOW_ACTION_DROP;
	
	//cvmx_spinlock_lock(&cvm_ip_spinlock_table[idx>>9]);///////big spinlock

	/*固定数组里查询*/
	for(i = 0; i < HASH_ARRAY_ITEM_NUM; i++)
	{ 
		if((iph->ip_dst == inode[i].destAddr)  && ( iph->ip_src == inode[i].srcAddr) &&
		   (iph->ip_id == inode[i].identifcation) && (iph->ip_p == inode[i].protocol))
		{
            FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG, "cvm_ip_reass_hash_processitem: Found at inode[%d]--\n", i);

			//dispatch_infor->action = inode[i].action;
			//dispatch_infor->CpuMask = inode[i].coremask;
			//dispatch_infor->PortMask = inode[i].portmask;	
//#ifdef ZCTT_ENET_STATISTICS
//		        cvmx_fau_atomic_add64(CVM_FAU_IP_FRAGS_HIT_TBL_NUM, 1);
//#endif 
			// then, send the packet and remove the item
			// use inner ip for 80211 to eth			
			// we havn't the right rule, find it and store.
			if (inode[i].rule == NULL) {
				th.th_sport = inode[i].th_sport;
				th.th_dport = inode[i].th_dport;
				rule = acl_table_lookup(iph, &th, &first_lock);
				inode[i].rule = rule;
			}
			else {
				rule = inode[i].rule;
			}
			
            //printf("cvm_ip_reass_hash_processitem: Found at inode[%d] rule %x\n", i, rule->rules.rule_state);

			//flow_action_process(work, action_type, iph, &th, rule, is_qos);
			// if we receive the tailer packet, remove inode
			
			//if ((FRAGMENT_MORE(ip->ip_off) == 0) && 
				//(FRAGMENT_OFFSET(ip->ip_off) != 0)) {				
				//cvm_ip_reass_hash_removeitem(iph);
			//}
			break;
		}
	}
	
	if(i == HASH_ARRAY_ITEM_NUM) {   
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,"cvm_ip_reass_hash_processitem: Packet:srcIp-0x%x,destIp-0x%x.Inode not found!Packet will send to linux!\n",iph->ip_src,iph->ip_dst);   
		//can not find the inode, send it to linux
		//printf("cvm_ip_reass_hash_processitem: Packet:srcIp-0x%x,destIp-0x%x.Inode not found!Packet will send to linux!\n",iph->ip_src,iph->ip_dst);
		//flow_action_process(work, action_type, iph, &th, rule, is_qos);
	}
	
	//cvmx_spinlock_unlock(&cvm_ip_spinlock_table[idx>>9]);///////

	return rule;
}

