#ifndef _CVM_IP_REASS_

#define _CVM_IP_REASS_
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
#include "capwap.h"	
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"

#define	ZCTT_DISPATCH_REASS_CRC_POLYNOMIAL		0x1edc6f41
#define DISPATCHHASHSIZE	32768
//#define DISPATCHHASHSIZE	4096
#define	HASH_ARRAY_ITEM_NUM	8

#define MAXTIMESTAMP        256
#define	TIME_DISTANCE	    30

#if 0

/*hash四元组，生成hash值，分别是 源IP，目的IP，流标志ID，协议字段*/
typedef struct
{
	uint32_t  srcAddr;
	uint32_t  destAddr;
	uint16_t	ipId;
	uint8_t	protocal;
}hash_tuple_t;

/*hash表中记录的关于数据包的分流方式*/
typedef struct
{
	uint64_t	coremask;
	uint64_t	portmask;
	uint32_t	action;
}hash_dispatchmode_t;

/*hash表的节点元素，包括时间戳、四元组、分流信息*/
typedef struct
{
    hash_dispatchmode_t	dispatch;
    hash_tuple_t	tuple;
    cvmx_spinlock_t itemLock;
	uint16_t	timestamp;
}hash_inode_t;



/*hash表的链表节点类型*/
typedef struct hash_linknode_s
{
    struct hash_linknode_s *next;
    hash_inode_t	inode;	
}hash_linknode_t;
#endif

typedef struct
{
	uint64_t    coremask;
	uint64_t	portmask;
	uint32_t	action;
    uint32_t    srcAddr;
    uint32_t    destAddr;
    uint16_t    identifcation;
    uint8_t     protocol;
    uint8_t     timestamp;
    uint16_t  th_sport;		/* source port */
    uint16_t  th_dport;		/* destination port */
	cvmx_wqe_t *work;
	rule_item_t *rule;
}hash_inode_t;

/*hash表项类型*/
typedef struct
{
	hash_inode_t	arrayItem[HASH_ARRAY_ITEM_NUM];	
}hash_item_t;

typedef struct
{
	uint64_t    CpuMask;
	uint64_t	PortMask;
	uint32_t	action;
}dispatch_info;


void cvm_ip_reass_hash_additem(cvmx_wqe_t	*work, uint32_t action_type,
		cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th,rule_item_t *prule,uint8_t is_qos, uint8_t is_pppoe);
//void zctt_dispatch_hash_aging();
void cvm_ip_reass_hash_finditem(cvmx_wqe_t * work, dispatch_info * dispatch_infor);
void cvm_ip_reass_hash_removeitem(cvm_common_ip_hdr_t *ip);
uint32_t cvm_ip_reass_bucket_lookup(cvm_common_ip_hdr_t * ip, uint16_t extra_id);
rule_item_t* cvm_ip_reass_hash_processitem(cvmx_wqe_t *work, cvm_common_ip_hdr_t *iph);

void cvm_ip_reass_init();

#endif

