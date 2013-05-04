#ifndef _SE_AGENT_ACL_H_
#define _SE_AGENT_ACL_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <linux/tipc.h>
#include "cvmx-spinlock.h"
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-scratch.h"

#include "se_agent.h"

#define ACL_TBL_RULE_NAME             "acl_tbl_rule"
#define CAPWAP_CACHE_TBL_NAME         "capwap_cache_tbl"
#define ACL_DYNAMIC_TBL_RULE_NAME     "acl_dynamic_tbl_rule"
#define MAX_CAPWAP_CACHE_NUM          8192
/* for print ip */
#define IP_FMT(m)	\
				((uint8_t*)&(m))[0], \
				((uint8_t*)&(m))[1], \
				((uint8_t*)&(m))[2], \
				((uint8_t*)&(m))[3]
				
/* for print mac */
#define MAC_FMT(m)  \
				((uint8_t*)(m))[0], \
				((uint8_t*)(m))[1], \
				((uint8_t*)(m))[2], \
				((uint8_t*)(m))[3], \
				((uint8_t*)(m))[4], \
				((uint8_t*)(m))[5]

/* for print protocol */
#define PROTO_STR(t)  ((t) == 0x6 ? "TCP" : ((t) == 0x11 ? "UDP" : ((t) == 0x1 ? "ICMP" : "Unknown")))


#define PANEL_PORT_GROUP         0  /*group number of packets which received from the physical port */
#define FROM_LINUX_GROUP         2	/*group number of packets which linux send to SE*/
#define TO_LINUX_FCCP_GROUP      14 /*group number of fccp packets which SE send to linux*/
#define TO_LINUX_GROUP           15 /*group number of normal packets which SE send to linux*/
#define DEFAULT_AGENT_TIME       600
extern uint64_t get_sec();
 
extern void se_agent_delete_specified_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_specified_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_rule_sum(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_capwap_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_clear_rule_all(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_clear_aging_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_tolinux_flow(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_user_rule_stats(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_aging_rule_cnt(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_learned_acl(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_learning_acl(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_clear_rule_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);  /* wangjian clear */
extern void se_agent_show_rule_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len); /*wangjian 2012.07.09 add ip */

typedef struct  rule_item_s{
	struct rule_item_s *next;
	struct rule_param_s rules;
	uint16_t valid_entries;    /*only the first bucket valid_entries is used for the number of rule entry*/
	cvmx_spinlock_t      lock; /*only the first bucket lock is used*/
}rule_item_t;
extern  CVMX_SHARED  unsigned int       aging_time;

extern  CVMX_SHARED int                 se_socket; /*se_agent socket */

extern	CVMX_SHARED rule_item_t        *acl_bucket_tbl;
extern	CVMX_SHARED uint32_t            acl_static_tbl_size;
extern	CVMX_SHARED capwap_cache_t     *capwap_cache_tbl;
extern  CVMX_SHARED uint32_t            acl_dynamic_tbl_size;
extern  CVMX_SHARED uint32_t            capwap_cache_tbl_size;

union capwap_hd {
	struct {
		uint64_t dword_0_1;
		uint64_t dword_2_3;
	} u;
#define m_dword0 u.dword0
#define m_dword1 u.dword1
#define m_dword2 u.dword2
#define m_dword3 u.dword3

	struct {
		/* dword0 */
		uint32_t pmb:8;
		uint32_t hlen:5;
		uint32_t rid:5;
		uint32_t wbid:5;
		uint32_t t:1;
		uint32_t f:1;
		uint32_t l:1;
		uint32_t w:1;
		uint32_t m:1;
		uint32_t k:1;
		uint32_t flags:3;

		/* dword1 */
		uint16_t fragid;
		uint16_t offset:13;
		uint16_t rsvd0:3;

		/* dword2 */
		uint8_t wlanid;
		uint8_t rsvd1;
		uint16_t rsvd2;

		/* dword3 */
		uint32_t rsvd3;
	} s;
#define m_pmb s.pmb
#define m_hlen s.hlen
#define m_rid s.rid
#define m_wbid s.wbid
#define m_t s.t
#define m_f s.f
#define m_l s.l
#define m_w s.w
#define m_m s.m
#define m_k s.k
#define m_flags s.flags
#define m_fragid s.fragid
#define m_offset s.offset
#define m_wlanid s.wlanid
}__attribute__ ((packed));



static inline rule_item_t * cvm_five_tuple_hash(uint32_t dip, uint32_t sip, uint8_t proto, uint16_t dport, uint16_t sport)
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

#endif

