

#include <stdio.h>
#include <string.h>
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-pow.h"
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
#else
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-defs.h"
#endif
#include "acl.h"
#include "cvm-ip-in.h"
#include "fwd_rpa.h"
#include "fwd_debug.h"

extern CVMX_SHARED uint8_t *base_mac_table ;
extern inline int8_t rx_l2hdr_decap( uint8_t* eth_head,cvm_common_ip_hdr_t **ip_head);
//extern int cvmx_dump_128_packet(cvmx_wqe_t *work);

inline int rpa_packet_handle(cvmx_wqe_t* work,cvm_common_ip_hdr_t **ip,uint32_t *action_type)
{
	uint8_t *eth_head = NULL;
	cvm_common_ip_hdr_t  *ip_head = NULL;
	uint8_t *pkt_ptr = NULL;

    /* check mac table */
    if((NULL == base_mac_table) || is_zero_ether_addr(base_mac_table))
    {
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
			"base_mac_table not ready!\n");
        return RETURN_ERROR;
    }
	
	pkt_ptr = (uint8_t*)cvmx_phys_to_ptr(work->packet_ptr.s.addr);

    /* find real eth hdr */
    eth_head = pkt_ptr + RPA_HEAD_LEN;
	if(work->word2.s.vlan_valid)
	{
        eth_head += VLAN_HLEN;
	}
	
	if(RETURN_ERROR == rx_l2hdr_decap(eth_head,&ip_head))
	{
		return RETURN_ERROR;
	}
	*ip = ip_head;
	if((ip_head->ip_off&0x3f) != 0)/*frag ip*/
	{		
		return RETURN_ERROR;
	}

	work->word2.s.ip_offset = ((uint8_t*)ip_head - pkt_ptr);
	return RETURN_OK;
}

inline void add_rpa_head(cvmx_wqe_t *work, rule_item_t *rule)
{
	uint8_t *pkt_ptr = NULL;
    int dslotid = 0;
    int sslotid = 0;

	if((work == NULL) || (rule == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"add_rpa_head: work or rule is Null!\r\n");
		return ;
	}

    dslotid = ((rule->rules.rpa_header.rpa_d_s_slotNum)>> 4)&0x0f;
    sslotid = (rule->rules.rpa_header.rpa_d_s_slotNum &0x0f);
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
        "dslotid = %d sslotid = %d\n,base mac address = %p\n dest mac address = %p",\
        dslotid,sslotid,base_mac_table,(base_mac_table + sslotid * RPA_ENET_ETHER_ADDR_LEN));
    
	pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);

    if(rule->rules.rpa_header.rpa_vlan_tag)
    {
        rpa_vlan_eth_head_t * dst_rpa_head = NULL;
        pkt_ptr = pkt_ptr - sizeof(rpa_vlan_eth_head_t);
        dst_rpa_head = (rpa_vlan_eth_head_t *)pkt_ptr;
        memcpy(dst_rpa_head->ether_dhost, (base_mac_table + dslotid * RPA_ENET_ETHER_ADDR_LEN), RPA_ENET_ETHER_ADDR_LEN);
    	memcpy(dst_rpa_head->ether_shost, (base_mac_table + sslotid * RPA_ENET_ETHER_ADDR_LEN), RPA_ENET_ETHER_ADDR_LEN);
        dst_rpa_head->vlan_type = CVM_ETH_P_8021Q;
        dst_rpa_head->vlan_tag = rule->rules.rpa_header.rpa_vlan_tag;
    	dst_rpa_head->cookie = RPA_COOKIE;
    	dst_rpa_head->type = rule->rules.rpa_header.rpa_type;
    	dst_rpa_head->dnetdevNum = rule->rules.rpa_header.rpa_dnetdevNum;
    	dst_rpa_head->snetdevNum = rule->rules.rpa_header.rpa_snetdevNum;
    	dst_rpa_head->d_s_slotNum = rule->rules.rpa_header.rpa_d_s_slotNum;
    	CVM_WQE_SET_LEN(work,(CVM_WQE_GET_LEN(work)+ sizeof(rpa_vlan_eth_head_t)));
    }
    else
    {
    	rpa_eth_head_t * dst_rpa_head = NULL;
	    pkt_ptr = pkt_ptr - sizeof(rpa_eth_head_t);
	    dst_rpa_head = (rpa_eth_head_t *)pkt_ptr;
    	memcpy(dst_rpa_head->ether_dhost, (base_mac_table + dslotid * RPA_ENET_ETHER_ADDR_LEN), RPA_ENET_ETHER_ADDR_LEN);
    	memcpy(dst_rpa_head->ether_shost, (base_mac_table + sslotid * RPA_ENET_ETHER_ADDR_LEN), RPA_ENET_ETHER_ADDR_LEN);
    	dst_rpa_head->cookie = RPA_COOKIE;
    	dst_rpa_head->type = rule->rules.rpa_header.rpa_type;
    	dst_rpa_head->dnetdevNum = rule->rules.rpa_header.rpa_dnetdevNum;
    	dst_rpa_head->snetdevNum = rule->rules.rpa_header.rpa_snetdevNum;
    	dst_rpa_head->d_s_slotNum = rule->rules.rpa_header.rpa_d_s_slotNum;
    	CVM_WQE_SET_LEN(work,(CVM_WQE_GET_LEN(work)+ sizeof(rpa_eth_head_t)));
	}
	
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);
	work->packet_ptr.s.back   = CVM_COMMON_CALC_BACK(work->packet_ptr);

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"add_rpa_head: back is %d, addr at 0x%llx\r\n",work->packet_ptr.s.back,(unsigned long long)work->packet_ptr.s.addr);

#ifdef OCTEON_DEBUG_LEVEL
	if((fastfwd_common_debug_level == FASTFWD_COMMON_DBG_LVL_INFO) &&((1 << cvmx_get_core_num() ) & (core_mask) ))
	{
		//cvmx_dump_128_packet(work);
		fwd_debug_dump_packet(work);
	}
#endif

	return;
}







	
