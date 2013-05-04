#ifndef __MLD_SNOOP_MAIN_H__
#define __MLD_SNOOP_MAIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "mld_snoop_inter.h"
#include "igmp_snoop_inter.h"
#include "igmp_snoop_com.h"

#define MLD_DBG printf

#define ADDRSHOWV6(addr) \
	(addr)[0], \
	(addr)[1], \
	(addr)[2], \
	(addr)[3], \
	(addr)[4], \
	(addr)[5], \
	(addr)[6], \
	(addr)[7]


#define IGMP_SNP_DEL_INTERTIMER( id ) { id = 0; }
/****************************thread value****************************************************/
pthread_t thread_timer;
pthread_t thread_msg;
pthread_t thread_npd_msg;
pthread_t thread_recvskb;
pthread_t dbus_thread;

pthread_mutex_t mutex;	/*线程锁*/

/************************************global value*****************************************/
#define SYSMAC_ADDRESS_LEN 13
#define SYSINFO_SN_LENGTH 20
#define BM_IOC_MAGIC 0xEC 

extern unsigned int igmp_snp_log_level;

extern unsigned char sysMac[MACADDRLENGTH];

extern struct mcgroup_s **mcgroup;
/*for dgram*/
struct sockaddr_un pktLocalAddr;
struct sockaddr_un pktRemoteAddr;

/*query receiveFlag*/
extern unsigned char	queryRxFlag;

/*get system macaddr info*/
extern unsigned char igmp_get_macaddr;
/************************************config value*****************************************/
extern LONG igmp_snoop_enable ;	/*IGMP snoop enable or not*/
extern LONG mld_snoop_enable ;	/*MLD snoop enable or not*/

extern timer_list igmp_timer_list;

extern INT		kernel_fd;
extern ULONG igmp_genquerytime[IGMP_GENERAL_GUERY_MAX];
extern ULONG igmp_router_timeout;
extern ULONG igmp_vlancount;
extern ULONG igmp_groupcount;
extern ULONG mld_groupcount;
extern ULONG igmp_global_timer;
extern USHORT igmp_robust_variable;
extern ULONG igmp_query_interval;
extern ULONG igmp_rxmt_interval;
extern ULONG igmp_resp_interval;
extern ULONG igmp_grouplife;
extern ULONG igmp_vlanlife;
extern INT		first_mcgroupvlan_idx;	/*初始值为-1,when build multi group ,add multi vlan info in first_mcgroupvlan_idx*/

extern MC_group_vlan 		*mcgroup_vlan_queue[IGMP_GENERAL_GUERY_MAX];
extern igmp_vlan_list		*p_vlanlist;	/*system vlan information,when configure multi vlan enable from dcli,add info in p_vlanlist*/
extern igmp_queryport		*p_queryportlist;
extern igmp_routerport		*p_routerlist;
extern igmp_reporter_port	*p_reporterlist;
/************************************extern value****************************************/
extern l2mc_list_head		p_l2mc_list_head;
extern INT	npdmng_fd;



enum mldmodaddtype
{
	MLD_ADDR_ADD,
	MLD_ADDR_DEL,
	MLD_ADDR_RMV,
	MLD_SYS_SET,
	MLD_TRUNK_UPDATE
};

#define MLD_SYS_SET_INIT	1
#define MLD_SYS_SET_STOP	2

#define IPV6_HEADER_LEN 40


/*
 *	IPv6 address structure
 */
struct v6_addr
{
	union 
	{
		unsigned char	u6_addr8[16];
		unsigned short	u6_addr16[8];
		unsigned int	u6_addr32[4];
	} in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
};

/*
 *	IPv6 fixed header
 *
 *	BEWARE, it is incorrect. The first 4 bits of flow_lbl
 *	are glued to priority now, forming "class".
 */
struct ipv6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	unsigned char	priority:4,
				    version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	unsigned char	version:4,
				    priority:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	unsigned char	flow_lbl[3];

	unsigned short	payload_len;
	unsigned char	nexthdr;
	unsigned char	hop_limit;

	struct	v6_addr	saddr;
	struct	v6_addr	daddr;
};


struct mld {
	unsigned char type; /* type of mld message */
	unsigned char code; /* mld code */
	unsigned short cksum; /* IP-style checksum */
	unsigned short maxresp;	/*max response delay*/
	unsigned short reserved;  /*reserved*/
	unsigned short group[SIZE_OF_IPV6_ADDR]; /* IPV6 group address*/
};

struct mldv2_addrecord {
	unsigned char rec_type;
	unsigned char rec_auxlen;
	unsigned short numofsrc;
	unsigned short group[SIZE_OF_IPV6_ADDR]; /* IPV6 group address*/
};

struct mldv2 {
	unsigned char type;
	unsigned char reservedf;
	unsigned short checksum;
	unsigned short reserveds;
	unsigned short naddrc;	/*number of address count*/
	struct mldv2_addrecord mldv2_rec;
};
	
	
struct mldv2_query {
		unsigned char type;
		unsigned char code;
		unsigned short csum;
		unsigned short max_resp_code;
		unsigned short reserved;
		unsigned short group[SIZE_OF_IPV6_ADDR];
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned char qrv:3,
			     suppress:1,
			         resv:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned char resv:4,
			      suppress:1,
			           qrv:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
		unsigned char qqic;
		unsigned short nsrcs;
		unsigned short srcs[0][SIZE_OF_IPV6_ADDR];
};



struct mld_info{
	unsigned int ifindex;
	unsigned int vlan_id;
	struct ipv6hdr *ip_hdr;
	struct mld *mld_hdr;
	struct igmp_skb *data;
};
/**********************function declaration************************/
INT mld_v6addr_equal_0(unsigned short * group);
INT mld_ntohs(unsigned short ** group_ip);
INT mld_htons(unsigned short ** group_ip);
INT mld_copy_ipv6addr(unsigned short *ugroup,unsigned short **destugroup);
INT mld_compare_ipv6addr(unsigned short *addr1, unsigned short *addr2);
void mld_debug_print_skb(struct igmp_skb *msg_skb);
void mld_debug_print_groupvlan(MC_group_vlan *pvlan);
INT mld_snp_addr_check( unsigned short * group );
LONG mld_snp_delgroup( MC_group_vlan *pVlan, MC_group * pGroup, MC_group **ppNextGroup );
LONG mld_snp_del_mcgroupvlan( long vlan_id );
LONG mld_snp_groupcheck( long lVid );
LONG mld_creat_group_node( long lVid, unsigned short * ulGroup, unsigned long usIfIndex,MC_group ** ppMcGroup, MC_group_vlan ** ppPrevGroupVlan );
LONG mld_snp_searchvlangroup(long lVid,unsigned short *ulGroup,MC_group **ppMcGroup,MC_group_vlan **ppPrevGroupVlan);
LONG mld_recv_pktdata_proc(struct igmp_skb *msg_skb);
VOID mld_snp_file_option_field( unsigned char * pOptionBuf, unsigned long ulLength );
LONG mld_Snoop_Send_mld(mld_snoop_pkt * pPkt );
LONG mld_snp_routerleave( long lVid, unsigned long usIfIndex, unsigned short * ulGroup, unsigned short * ulSaddr );
LONG mld_Event_GroupMember_Timeout( MC_group_vlan *p_vlan,MC_group *p_group,unsigned long ifindex  );
LONG mld_Event_GroupLife_Timeout( MC_group_vlan *p_vlan, MC_group *p_group );
LONG mld_Event_GenQuery_Timeout( MC_group_vlan *p_vlan );
VOID mld_Event_Rxmt_Timeout( timer_element *cur );
VOID mld_Event_Resp_Timeout( timer_element *cur);
LONG mld_snp_flood( struct mld_info *sk_info, unsigned long lVid, unsigned long usIfIndex );
INT mld_snp_pim_msghandle( unsigned short * ulGoup, unsigned long ulIfindex, long lVid, unsigned short * ulSrcAddr );
LONG mld_pim_recv_msg(struct igmp_skb *msg_skb);
LONG mld_snp_routerreport( long lVid, unsigned long usIfIndex, unsigned short *ulGroup, struct mld_info *sk_info );
LONG mld_recv_report( unsigned long usIfIndex, unsigned short *ulGroup, long lVid, unsigned long ulType, struct mld_info *sk_info);
INT mld_recv_query(unsigned long ifindex, unsigned short maxresptime, unsigned short * group,long vlan_id, struct mld_info *sk_info);
LONG mld_recv_leave( unsigned long usIfIndex, unsigned short *ulGroup, long lVid,struct mld_info *sk_info );
LONG mld_recv_unknown(struct mld_info *sk_info );
INT mld_skb_proc(struct igmp_skb *msg_skb);
int mld_print_pkt(unsigned char *data,int len);

/**********************************extern function*********************************************/
extern LONG igmp_snp_addintertimer( ULONG ulSec, ULONG * pulTimerId );
extern INT inet_cksum(USHORT *addr,ULONG len);
extern INT Igmp_Snoop_Send_Packet(struct igmp_skb *msg_skb,UINT datalen,LONG vlan_id,ULONG ifindex);
extern INT init_igmp_snp_timer(void);

#ifdef __cplusplus
	}
#endif
	
#endif
	
