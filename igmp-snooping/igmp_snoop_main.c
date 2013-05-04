/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* igmp_snoop_main.c
*
*
* CREATOR:
* 		chenb@autelan.com
*
* DESCRIPTION:
* 		igmp main routine
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.35 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/wait.h>
#include <sys/time.h>

#include "igmp_snoop_com.h"
#include "igmp_snoop_inter.h"
#include "igmp_snp_dbus.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"
#include "mld_snoop_inter.h"
#include "mld_snoop.h"
#include "mld_snoop_main.h"

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define IGMP_SNP_DEL_INTERTIMER( id ) { id = 0; }
/****************************thread value****************************************************/
pthread_t thread_timer;
pthread_t thread_msg;
pthread_t thread_npd_msg;
pthread_t thread_recvskb;
pthread_t dbus_thread;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	/*线程锁*/

/************************************global value*****************************************/
#define SYSMAC_ADDRESS_LEN 13
#define SYSINFO_SN_LENGTH 20
#define BM_IOC_MAGIC 0xEC 

extern unsigned int igmp_snp_log_level;

typedef struct bm_op_sysinfo_common {
	unsigned char mac_addr[SYSMAC_ADDRESS_LEN]; // system mac address
	unsigned char sn[SYSINFO_SN_LENGTH]; // module or backplane or mainboard serial number
}sysinfo_args;
typedef sysinfo_args bm_op_mainboard_sysinfo;
#define BM_IOC_MAINBOARD_SYSINFO_READ	_IOWR(BM_IOC_MAGIC, 6, bm_op_mainboard_sysinfo) // read main board sysinfo
typedef unsigned char BOOL;

unsigned char sysMac[MACADDRLENGTH] = {0};

struct mcgroup_s **mcgroup = NULL;
/* for distributed igmp local slot */
extern int igmp_slot = 1;

/*for dgram*/
struct sockaddr_un pktLocalAddr;
struct sockaddr_un pktRemoteAddr;

/*query receiveFlag*/
unsigned char	queryRxFlag = IGMP_SNOOP_NO;

/*get system macaddr info*/
unsigned char igmp_get_macaddr = IGMP_SNOOP_NO;
/************************************config value*****************************************/
LONG igmp_snoop_enable = IGMP_SNOOP_NO;	/*IGMP snoop enable or not*/
LONG mld_snoop_enable = MLD_SNOOP_NO;	/*MLD snoop enable or not*/

timer_list igmp_timer_list;

INT		kernel_fd = 0;
ULONG igmp_genquerytime[IGMP_GENERAL_GUERY_MAX] = {0};
ULONG igmp_router_timeout = 260 ;
 ULONG igmp_vlancount = 0;
ULONG igmp_groupcount = 0;
ULONG mld_groupcount = 0;
ULONG igmp_global_timer = 0;
USHORT igmp_robust_variable = IGMP_ROBUST_VARIABLE;
ULONG igmp_query_interval = IGMP_V2_UNFORCED_QUERY_INTERVAL;
ULONG igmp_rxmt_interval = IGMP_V2_LEAVE_LATENCY *10;
ULONG igmp_resp_interval = IGMP_V2_QUERY_RESP_INTERVAL;
ULONG igmp_grouplife = IGMP_GROUP_LIFETIME;
ULONG igmp_vlanlife = IGMP_VLAN_LIFE_TIME;
INT		first_mcgroupvlan_idx = -1;	/*初始值为-1,when build multi group ,add multi vlan info in first_mcgroupvlan_idx,ckeck vlan exist or not*/

MC_group_vlan 		*mcgroup_vlan_queue[IGMP_GENERAL_GUERY_MAX];
igmp_vlan_list		*p_vlanlist = NULL;	/*system vlan information,when configure multi vlan enable from dcli,add info in p_vlanlist,check vlan enable or not*/
igmp_queryport		*p_queryportlist = NULL;
igmp_routerport		*p_routerlist = NULL;
igmp_reporter_port	*p_reporterlist = NULL;
/************************************extern value****************************************/
extern l2mc_list_head		p_l2mc_list_head;
extern INT	npdmng_fd;
/***************************************declare functions**********************************/
INT init_igmp_snp_timer(void);
static VOID igmp_snp_global_timer_func( timer_element *cur );
static LONG Igmp_Event_GenQuery_Timeout( MC_group_vlan *p_vlan );
static LONG Igmp_Event_GroupLife_Timeout( MC_group_vlan *p_vlan, MC_group * p_group );
static LONG Igmp_Event_Proxy_Timeout(MC_group_vlan *p_vlan, MC_group * p_group);
static LONG Igmp_Event_GroupMember_Timeout( MC_group_vlan *p_vlan,MC_group *p_group,
							ULONG ifindex );
static VOID Igmp_Event_Resp_Timeout( timer_element *cur);
static VOID Igmp_Event_Rxmt_Timeout( timer_element *cur );
LONG igmp_snp_addintertimer( ULONG ulSec, ULONG * pulTimerId );
static LONG Igmp_Snoop_Send_Igmp( igmp_snoop_pkt * pPkt );
static VOID igmp_snp_file_option_field( UCHAR * pOptionBuf, ULONG ulLength );
INT igmp_snp_get_vlan_mbrs_igmpvlanlist(ULONG Vid,INT igmpVlanMbr[],INT *pCount);
INT inet_cksum(USHORT *addr,ULONG len);
INT Igmp_Snoop_Send_Packet(struct igmp_skb *msg_skb,UINT datalen,LONG vlan_id,ULONG ifindex);
static LONG igmp_recv_report( ULONG usIfIndex, ULONG ulGroup, LONG lVid, 
								ULONG ulType, struct igmp_info *sk_info);
static INT igmp_recv_query(ULONG ifindex, USHORT maxresptime, ULONG group,
					LONG vlan_id, struct igmp_info *sk_info);	
static LONG igmp_recv_leave( ULONG usIfIndex, ULONG ulGroup, LONG lVid,struct igmp_info *sk_info );
static LONG igmp_recv_unknown(struct igmp_info *sk_info );
static LONG igmp_snp_routerreport( LONG lVid, ULONG usIfIndex, ULONG ulGroup, struct igmp_info *sk_info );
static LONG igmp_snp_routerleave( LONG lVid, ULONG usIfIndex, ULONG ulGroup, ULONG ulSaddr );
static LONG igmp_snp_flood( struct igmp_info *sk_info, LONG lVid, ULONG usIfIndex );
/**************************************************************************************/
extern INT igmp_set_enable(LONG flag);
extern int igmp_snp_dbus_init(void);
extern INT mld_v6addr_equal_0(unsigned short * group);
extern INT mld_ntohs(unsigned short ** group_ip);
extern INT mld_htons(unsigned short ** group_ip);
extern INT mld_copy_ipv6addr(unsigned short *ugroup,unsigned short **destugroup);
extern INT mld_compare_ipv6addr(unsigned short *addr1, unsigned short *addr2);
extern LONG mld_Snoop_Send_mld(mld_snoop_pkt * pPkt );
extern LONG mld_Event_GroupMember_Timeout( MC_group_vlan *p_vlan,MC_group *p_group,unsigned long ifindex  );
extern LONG mld_Event_GroupLife_Timeout( MC_group_vlan *p_vlan, MC_group *p_group );
extern LONG mld_Event_GenQuery_Timeout( MC_group_vlan *p_vlan );
extern VOID mld_Event_Rxmt_Timeout( timer_element *cur );
extern VOID mld_Event_Resp_Timeout( timer_element *cur);
extern void mld_debug_print_skb(struct igmp_skb *msg_skb);

/*****************************************functions**************************************/
LONG Igmp_Event_AddVlan(unsigned short vid);
LONG Igmp_Event_PortUp(unsigned short vid, unsigned int port_index);
LONG Igmp_Event_VlanAddPort( igmp_snoop_pkt * pPkt );
/**************************************************************************************/


static void Igmp_print_packet( char * pBuffer, long lLength )
{
	unsigned long i = 0;
	unsigned char *p = NULL;
	unsigned long length;
	unsigned char printBuffer[50] = {0}, printLen = 0, *pPtr = NULL;
	
	if ( !pBuffer ) {
		return;
	}
	
	p = (unsigned char *)pBuffer;
	length = lLength;
	igmp_snp_syslog_dbg("*****************packet detail*****************\n");

	pPtr = printBuffer;
	for ( i = 0; i < length; i++ )
	{
		if((0 != i) && !(i%16)) {
			printBuffer[printLen] = '\0';
			igmp_snp_syslog_dbg("%s", printBuffer);
			printLen = 0;
			pPtr = printBuffer;
			memset(printBuffer, 0, sizeof(printBuffer));
		}	
		printLen += sprintf(pPtr, "%02X ", *(p+i));
		pPtr = printBuffer + printLen;
	}
	// give last line less than 16B
	if(i%16) {
		igmp_snp_syslog_dbg("%s\n", printBuffer);
	}

	return;
}
/*******************************************************************************
 * igmp_debug_print_skb
 *
 * DESCRIPTION:
 *   		print the packet info get from npd
 *
 * INPUTS:
 * 		msg_skb - point to the packet 
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
void igmp_debug_print_skb(struct igmp_skb *msg_skb)
{
	struct iphdr *ip_addr = (struct iphdr *)(msg_skb->buf + 14);	
	struct igmp *igmphd = (struct igmp *)((unsigned char *)ip_addr+ip_addr->ihl*4);
	struct pim_header *pimhd = (struct pim_header *)((unsigned char *)ip_addr + ip_addr->ihl*4);
	struct igmpv3_report *v3_report = NULL;

	if(0 == (SYSLOG_DBG_PKT_REV & igmp_snp_log_level))
	{
		return;
	}

	if((msg_skb->nlh.nlmsg_len - sizeof(msg_skb->nlh))< 0){
		igmp_snp_syslog_warn("actual packet size %d less than msg header.\n",msg_skb->nlh.nlmsg_len);
		return;
	}
	
	igmp_snp_syslog_pkt_rev("*****************packet brief******************\n");
	igmp_snp_syslog_pkt_rev("vid %d if %#x ipv%d ihl %d tos %d prot %d\n",	\
							(msg_skb->vlan_id),(msg_skb->ifindex),ip_addr->version,	\
							ip_addr->ihl*4,ip_addr->tos,ip_addr->protocol);
	igmp_snp_syslog_pkt_rev("sip %u.%u.%u.%u dip %u.%u.%u.%u\n",	\
							NIPQUAD(ip_addr->saddr),NIPQUAD(ip_addr->daddr));
	if( IPPROTO_PIM == ip_addr->protocol )
	{
		igmp_snp_syslog_pkt_rev("pim ver %d type %d cksum %d rsvd %d\n",
							pimhd->pim_vers,pimhd->pim_type,pimhd->pim_cksum,pimhd->pim_reserved);
	}
	else
	{
		if(0x22 != igmphd->igmp_type){
			igmp_snp_syslog_pkt_rev("type %d code %d cksum %d group %u.%u.%u.%u\n",	\
									igmphd->igmp_type,igmphd->igmp_code,igmphd->igmp_cksum,	\
									NIPQUAD(igmphd->igmp_group.s_addr));
		}
		else if(0x22 == igmphd->igmp_type){
			v3_report = (struct igmpv3_report *)igmphd;
			igmp_snp_syslog_pkt_rev("v3 type %d code %d cksum %d group %u.%u.%u.%u\n", \
									v3_report->type,v3_report->resv1,v3_report->csum,	\
									NIPQUAD(v3_report->grec[0].grec_mca));
		}
	}
	igmp_snp_syslog_pkt_rev("*****************packet over*******************\n");
	// give out detailed packet byte stream
	igmp_snp_syslog_pkt_rev(msg_skb->buf, (msg_skb->nlh.nlmsg_len - sizeof(msg_skb->nlh)));

	return;
}

void igmp_debug_print_groupvlan(MC_group_vlan *pvlan)
{
	struct MC_port_state *t_port = NULL;
	struct MC_group		*t_group = NULL;
	igmp_router_entry	*t_router = NULL;

	if(0 == (SYSLOG_DBG_PKT_ALL & igmp_snp_log_level))
	{
		return;
	}

	if(NULL == pvlan)
	{
		igmp_snp_syslog_err("igmp_debug_print_groupvlan:vlan is NULL.\n");
		return;
	}
	igmp_snp_syslog_dbg("@@@@@@@@@@@@@@@@@@@Vlan data@@@@@@@@@@@@@@@@@@@@\n");
	igmp_snp_syslog_dbg("next:%d prev:%d vlan_id:%d last_requery_addr:%u.%u.%u.%u\n",
						pvlan->next,pvlan->prev,
						pvlan->vlan_id,NIPQUAD(pvlan->saddr));
	igmp_snp_syslog_dbg("querytimeinterval:%d querytimer_id:%d vlanlife:%d\n",
						pvlan->querytimeinterval,pvlan->querytimer_id,
						pvlan->mcgvlanlife);

	igmp_snp_syslog_dbg("route port:");
	t_router = pvlan->routerlist;
	while (t_router != NULL)
	{
		igmp_snp_syslog_dbg("%d  ", t_router->mroute_ifindex);
		t_router = t_router->next;
	}
	
	t_group = pvlan->firstgroup;
	if( NULL != t_group )
	{
		igmp_snp_syslog_dbg("------------------Group data--------------------\n");
		igmp_snp_syslog_dbg("MC_ipadd:0x%x report_ipadd:0x%x\n",
							t_group->MC_ipadd,
							t_group->report_ipadd);			
		igmp_snp_syslog_dbg("vlan_id:%d version:%s McGroup_id:%d\n",
							t_group->vid,
							(t_group->ver_flag?"IGMP V1":"IGMP V2"),
							t_group->mgroup_id);
		t_port = pvlan->firstgroup->portstatelist;
		igmp_snp_syslog_dbg("-------------------MC_ports---------------------\n");
		while( NULL != t_port )
		{
			igmp_snp_syslog_dbg("ifindex:0x%x state:0x%x\n",t_port->ifindex,t_port->state);
			t_port = t_port->next;
		}
		t_group = t_group->next;
	}
	igmp_snp_syslog_dbg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	return;
}
int igmp_snp_print_list(void)
{
	igmp_routerport* pRoutePort = NULL;
	pRoutePort = p_routerlist;
	while( NULL != pRoutePort ){
		igmp_snp_syslog_dbg("pRoutePort@@%p, next@@0x%x.\n",pRoutePort,pRoutePort->next);
		pRoutePort = pRoutePort->next;
	}
	igmp_snp_syslog_dbg("global router por--p_routerlist@@%p,p_routerlist->next@@0x%x.\n",\
					p_routerlist,p_routerlist->next);
	return IGMPSNP_RETURN_CODE_OK;
}
#if 0
int igmp_read_sys_mac() {
    // TODO read EEPROM and fill product sysinfo
	bm_op_mainboard_sysinfo	 sysinfo;
	int fd;
	int result = 0;
	char *ptr = NULL;

	fd = open("/dev/bm0",0);
	if(fd < 0)
	{
		IGMP_SNP_DEBUG("open dev %s error(%d) when read main board sysinfo!","/dev/bm0",fd);
		return -1;
	}

	memset(&sysinfo, 0, sizeof(bm_op_mainboard_sysinfo));
	
	result = ioctl(fd,BM_IOC_MAINBOARD_SYSINFO_READ,&sysinfo);
	if (result != 0) {
		IGMP_SNP_DEBUG("read mainboard sysinfo error!");
		return -1;
	}
	// read system base mac
	ptr = (char*)malloc(SYSMAC_ADDRESS_LEN);
	if(NULL == ptr) {
		IGMP_SNP_DEBUG("read mainboard sysinfo alloc MAC memory error\n");
		return -1;
	}
	memset(ptr,0,SYSMAC_ADDRESS_LEN);
	sprintf(ptr,"%02x%02x%02x%02x%02x%02x",sysinfo.mac_addr[0],sysinfo.mac_addr[1],\
			sysinfo.mac_addr[2],sysinfo.mac_addr[3],sysinfo.mac_addr[4],sysinfo.mac_addr[5]);
	sysMac = ptr;

	IGMP_SNP_DEBUG("read mainboard sysinfo get base mac %s ",sysMac);

	ptr = NULL;
	close(fd);		
	return IGMP_SNOOP_OK;
}
#endif

/*******************************************************************************
 * igmp_snp_sock_init
 *
 * DESCRIPTION:
 *   	some descriptions here...
 *
 * INPUTS:
 * 	var name  - meanings of this variable
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	TRUE   	- on success
 * 	FALSE	- otherwise
 *
 * COMMENTS:
 *      comments here
 **
 ********************************************************************************/
int	create_packet_clientsock_dgram()
{
	int cltsockPkt= 0;
	memset(&pktLocalAddr,0,sizeof(pktLocalAddr));
	memset(&pktRemoteAddr,0,sizeof(pktRemoteAddr));

	if((cltsockPkt = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		igmp_snp_syslog_err("Create_packet_clientsock_dgram socket Fail.\n");
		return -1;
	}
	
	pktLocalAddr.sun_family = AF_LOCAL;
	strcpy(pktLocalAddr.sun_path, "/tmp/igmp_client");
	
	pktRemoteAddr.sun_family = AF_LOCAL;
	strcpy(pktRemoteAddr.sun_path,"/tmp/igmp_serv");					

    unlink(pktLocalAddr.sun_path);

	if(bind(cltsockPkt, (struct sockaddr *)&pktLocalAddr, sizeof(pktLocalAddr)) == -1) 
	{
		igmp_snp_syslog_err("Create_packet_clientsock_dgram socket bind Fail.\n");
		return -1;
	}

	chmod(pktLocalAddr.sun_path, 0777);
	return cltsockPkt;	
	
}


/*******************************************************************************
 * igmp_snp_addr_check
 *
 * DESCRIPTION:
 * 		check the group id is in range or not
 *
 * INPUTS:
 * 		Group  - Multicast group ip address
 *
 * OUTPUTS:
 *    	NULL	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   - on success
 * 		IGMPSNP_RETURN_CODE_OUT_RANGE	- otherwise
 *
 * COMMENTS:
 *		1. 224.0.0.X resever.
 *		2. available address 224.0.0.0 ~ 239.255.255.255 
 **
 ********************************************************************************/
 INT igmp_snp_addr_check( ULONG Group )
{
	/* range 224.0.0.0 ~ 239.255.255.255 */
	if ( ( Group < 0xE0000000 ) || ( Group > 0xEFFFFFFF ) )
	{
		igmp_snp_syslog_dbg("check group address, Invalid group address 0x%x\n", Group);
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	/* range 224.0.0.0 ~ 224.0.0.255 */
	if ( ( Group >= 0xE0000000 ) && ( Group <= 0xE00000FF ) )
	{
		igmp_snp_syslog_dbg("check group address, reserved group address 0x%x.\n", Group);
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	igmp_snp_syslog_dbg("check group address: 0x%x, is in available range.\n", Group);
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * inet_cksum
 *
 *  DESCRIPTION:
 *  Our algorithm is simple, using a 32 bit accumulator (sum),
 *  we add sequential 16 bit words to it, and at the end, fold
 *  back all the carry bits from the top 16 bits into the lower
 *  16 bits.
 * 
 *  INPUTS:
 * 	addr  - the pointer point to ip head
 *    len    -the add length
 *   
 * OUTPUTS:
 *    	NULL
 *
 * RETURNS:
 * 	answer  	-  show the ckecksum result
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT inet_cksum(USHORT *addr,ULONG len)
{	
	/*len = num Bytes*/
	register INT nleft = ( int ) len;
	register USHORT *w = addr;
	USHORT answer = 0;
	register INT sum = 0;

	/*
	*  Our algorithm is simple, using a 32 bit accumulator (sum),
	*  we add sequential 16 bit words to it, and at the end, fold
	*  back all the carry bits from the top 16 bits into the lower
	*  16 bits.
	*/
	while ( nleft > 1 )
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if ( nleft == 1 )
	{
		*( UCHAR * ) ( &answer ) = *( UCHAR * ) w ;
		sum += answer;
	}

	/*
	* add back carry outs from top 16 bits to low 16 bits
	*/
	sum = ( sum >> 16 ) + ( sum & 0xffff );	/* add hi 16 to low 16 */
	sum += ( sum >> 16 );			/* add carry */
	answer = ~( USHORT )sum;				/* truncate to 16 bits */
	return ( answer );
}

/*******************************************************************************
 * igmp_snp_groupcheck
 *
 * DESCRIPTION:
 *		igmp_snp_groupcheck() check Vlan-group chain. If vlan was destroyed
 *		delete the groups and the chain.
 * INPUTS:
 * 	lVid  - the vlan id
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	IGMPSNP_RETURN_CODE_OK   	- the vlan exist
 * 	IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST	- vlan not exist
 *
 * COMMENTS:
 *    
 **
 ********************************************************************************/

LONG igmp_snp_groupcheck( LONG lVid )
{
	ULONG ulRet = 0;

	igmp_vlanisused( lVid, &ulRet );
	/* if lVid does not exist, delete all groups belongs to this vlan node */
	if ( 0 == ulRet )
	{
		igmp_snp_syslog_dbg("delete all groups belongs to this vlan %d\n.", lVid);
		igmp_snp_del_mcgroupvlan( lVid );
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	return IGMPSNP_RETURN_CODE_OK;
}

/**************************************************************************
* igmp_snp_searchvlangroup()
*
* DESCRIPTION:
*		This function finds a node in group vlan list.
*
* INPUTS:		
*		 lVid	    -  vlan id 
*		 ulGroup  - MC group ip address
*
* OUTPUTS:
*		 ppMcgroup - MC group struct pointer
*		 ppPrevGroupVlan - list head
*						  
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - not found
*		1)NULL = ppPrevGroupVlan;
*		2)NULL!= ppPrevGroupVlan &&iVid != ppPrevGroupVlan->vlan_id
*		3)NULL!= ppMcGroup && ppMcGroup->MC_ipadd == ulGroup
*
*
*		
**************************************************************************/
LONG igmp_snp_searchvlangroup( LONG lVid,
										ULONG ulGroup,
										MC_group **ppMcGroup,
										MC_group_vlan **ppPrevGroupVlan
										)

{
	MC_group_vlan *pCurrent = NULL;
	MC_group *pGroupTemp = NULL;

	igmp_snp_syslog_event("find multicast-group 0x%x of vlan %d in mcgroup_vlan_queue.\n",
						  ulGroup, lVid );

	pCurrent = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);/*macro return a pointer*/
	//pCurrent = GET_VLAN_POINT_BY_INDEX(0);
	*ppPrevGroupVlan = pCurrent;

	/* check vlan */
	if ( 0 == lVid )
	{
		igmp_snp_syslog_warn("find multicast-group with vlan %d doesn't exist.\n", lVid );
	}

	if ( IGMPSNP_RETURN_CODE_OK != igmp_snp_groupcheck( lVid ) )
	{
		/*vlan is not used by igmp,it'll delete accroding mcgroupvlan */
		igmp_snp_syslog_err("vlan %d doesn't exist.\n", lVid  );
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	/*find the lVid pointing vlan in mcgroupvlan list.*/
	while ( NULL != pCurrent )
	{
		*ppPrevGroupVlan = pCurrent;
		if ( pCurrent->vlan_id != lVid )
		{
			pCurrent = GET_VLAN_POINT_BY_INDEX(pCurrent->next);/*find along the group vlan list*/
			continue;
		}
		else
		{
			igmp_snp_syslog_dbg("found multicast-groupVlan vlan_id %d.\n", pCurrent->vlan_id);		
			/*findout the mcGroupVlanNode pointed by lVid in mcgroup_vlan_list*/
			/*so,well,looking for the mcGroup in mcGroupVlanNode-> group list*/
			/*use the same method to find group in Groupvlan just founded.*/
			pGroupTemp = pCurrent->firstgroup;
			while ( NULL != pGroupTemp )
			{
				if ( pGroupTemp->MC_ipadd != ulGroup )
				{
					pGroupTemp = pGroupTemp->next;
					continue;
				}
				else
				{
					igmp_snp_syslog_dbg("found multicast-group 0x%x IpAddr 0x%x.\n",
										ulGroup, pGroupTemp->MC_ipadd);			
					*ppMcGroup = pGroupTemp;/*find Both mcGroupVLan & mcGroup*/
					return IGMPSNP_RETURN_CODE_OK;
				}
			}

			igmp_snp_syslog_dbg("no found multicast-group 0x%x.\n", ulGroup);
			*ppMcGroup = NULL;
			return IGMPSNP_RETURN_CODE_OK; /* find only mcGroupVlan*/
		}
	}

	/* ppPrevGroupVlan is  must NULL, when run here */
	igmp_snp_syslog_dbg("no find multicast-groupVlan vlanid %d.\n", lVid);
	*ppMcGroup = NULL;
	*ppPrevGroupVlan = NULL;
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_searchvlangroup_by_groupcount()
*
* INPUTS:		
*		 NULL
*
* OUTPUTS:
*		nonUsedGroupCnt - the group count get form igmp_groupcount
*						  
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_OUT_RANGE - not found
*
* DESCRIPTION:
*		This function finds a group id for the new group in group vlan list.
*
*		
**************************************************************************/
LONG igmp_searchvlangroup_by_groupcount
(
	ULONG * nonUsedGroupCnt
)
{
	MC_group_vlan * pCurrent = NULL;
	MC_group *pGroupTemp = NULL;
	INT		i = 0,unGroupEst = 0;
	INT		nEqualCnt[IGMP_SNP_GRP_MAX];
	unsigned long gpcount = 0;
	
	if(IGMP_SNP_ISENABLE()){
		gpcount = igmp_groupcount;
	}
	else if(MLD_SNP_ISENABLE()){
		gpcount = mld_groupcount;	
	}
	if(gpcount >= IGMP_SNP_GRP_MAX){
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	/* init the array with "0xFFFF" */
	for (i = 0; i < IGMP_SNP_GRP_MAX; i++)
	{
		nEqualCnt[i] = 0xFFFF;
	}

	pCurrent = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);/*macro return a pointer*/
	//pCurrent = GET_VLAN_POINT_BY_INDEX(0);
	while ( NULL != pCurrent ) /*along mcgroupvlan list */
	{
		pGroupTemp = pCurrent->firstgroup;
		while ( NULL != pGroupTemp )/* along group list */
		{
			//for(i=1;i<=igmp_groupcount;i++)
			//{
				nEqualCnt[pGroupTemp->mgroup_id] = pGroupTemp->mgroup_id;
			//}
			pGroupTemp = pGroupTemp->next;
		}
		pCurrent = GET_VLAN_POINT_BY_INDEX(pCurrent->next);/*find along the mcgroup vlan list*/
	}
	for(i =1;i<=gpcount; i++){
		if( 0xFFFF == nEqualCnt[i])
		{
			unGroupEst = i;
			break;
		}
	}
	if(0 == unGroupEst) 
	{
		*nonUsedGroupCnt = gpcount;
	}
	else{
		*nonUsedGroupCnt = unGroupEst;
	}
	igmp_snp_syslog_dbg("igmp_searchvlangroup_by_groupcount: find nonUsedGroupCnt = %d.\n",*nonUsedGroupCnt);
	return IGMPSNP_RETURN_CODE_OK;
	/*exist the possibility that mcgroupvlan Or group Pointer == NULL */
}

/**************************************************************************
* igmp_creat_group_node()
*
* INPUTS:
*		 
*		 lVid	    -  vlan id 
*		 ulGroup  - MC group ip address
* 		 usIfIndex - Interface Index. The multicast router port. 
*		 ppMcgroup - MC group struct pointer
*		 ppPrevGroupVlan - list head.As a input arg, it contains the pointer refer to
*						   the list head.
*
* OUTPUTS:
*		ppMcGroup - MC group struct pointer
*		ppPrevGroupVlan - as a output arg, it contains the vlan node the group belongs to.
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK                - creat group node on success.
*		IGMPSNP_RETURN_CODE_OUT_RANGE    -  out the group count range
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL  -  alloc mem fail
*		IGMPSNP_RETURN_CODE_NULL_PTR      -  pointer is null
*
* DESCRIPTION:
*		This function creates a new node in group vlan list.
*		Notice: group node is created in 2 condition:
*				1. recv'd a group specific query
*				2. recv'd a report and there is no group of that group address.
*
*
**************************************************************************/
LONG igmp_creat_group_node( LONG lVid, ULONG ulGroup, ULONG usIfIndex,
					MC_group ** ppMcGroup, MC_group_vlan ** ppPrevGroupVlan )
{
	int tmp;
	MC_group_vlan * pVlanNode = NULL;
	MC_group *pGroup = NULL;
	USHORT usIsCreate = IGMP_SNOOP_NO;   /* 0 - indicate the vlan node is created. 1 - new vlan node */
	ULONG ulSlot = 0;
	ULONG ulMaxGroup = 0;
	ULONG ulGroup_id = 0;
	
	igmp_snp_syslog_dbg("create new group: vid %d group 0x%x If %d pVlan 0x%x\n",
						lVid, ulGroup, usIfIndex, (*ppPrevGroupVlan));

	igmp_getmaxgroupcnt(usIfIndex,&ulMaxGroup);
	/* if the total group count is more than the max group number supported, return error*/
	if ( igmp_groupcount > ulMaxGroup )
	{
		igmp_snp_syslog_err("Group full! MaxGroup %d, GroupCount %d, lVid %d usIfIndex %d\n",
							ulMaxGroup, igmp_groupcount, lVid, usIfIndex);
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	pGroup = ( MC_group * )malloc( sizeof( MC_group ));
	if ( NULL == pGroup )
	{
		igmp_snp_syslog_err("Multicast group alloc memory failed.\n ");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset( pGroup,0, sizeof( MC_group ) );
	/*  outside already check mc group exists or not */
	pGroup->MC_ipadd= ulGroup;
	pGroup->vid = lVid;
	pGroup->grouplifetime = igmp_grouplife;
	/*no use of "pGroup->queryresposetime",it's identical with "responsetime" */
	//pGroup->queryresposetime = IGMP_V2_QUERY_RESP_INTERVAL;

	/**********************************
	 *no use of "pGroup->memberinterval",Mc_port_state:membretimer takes place of this one actually!
	 *when recv report,it set the "Mc_port_state:membretimer" tobe robust times query interval plus
	 *response interval,once membertimer expires,it tiggers "Event_GroupMember_Timeout" 
	 **********************************/
	//pGroup->memberinterval = igmp_robust_variable * igmp_query_interval + igmp_resp_interval;

	/**********************************
	 *no use of "pGroup->resendquerytime",
	 *when recv leave pkt,it'll create new
	 *timer with interval of Const variable:
	 *"igmp_rxmt_interval",and add to timer
	 *list
	 **********************************/
	//pGroup->resendquerytime = igmp_rxmt_interval;
	#if 0
	/*here can not determines vidx assigned to this new group****08/08/21/
	/* pGroup->mgroup_id = igmp_groupcount;*/	//added by wujh
	igmp_searchvlangroup_by_groupcount(&ulGroup_id);
	pGroup->mgroup_id = ulGroup_id;
	#endif 
	igmp_snp_addintertimer( pGroup->grouplifetime, &( pGroup->grouplifetimer_id ) );
	igmp_snp_syslog_dbg("add group life-timer %d.\n", pGroup->grouplifetime);

	/**********	Create vlan node,but not certainly use. ********************/
	pVlanNode = ( MC_group_vlan * ) malloc( sizeof( MC_group_vlan ));
	if ( NULL == pVlanNode )
	{
		igmp_snp_syslog_err("Multicast groupVlan alloc memory failed.\n");
		if ( NULL != pGroup )/*even fail here,pVlanNode possiblelly already exists*/
			free( pGroup );
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	memset( pVlanNode,0, sizeof( MC_group_vlan ) );

	/* If the router port is configured, add the routers entry onto the gvlan node */
	/* here is a loop to add route  */
	igmp_routerport *pRouter = NULL;
	do
	{
		igmp_snp_syslog_dbg("loop for add route which have in p_routerlist to new pVlanNode.\n");
		igmp_snp_searchrouterportlist( lVid, 0, IGMP_PORT_QUERY, &pRouter );
		if ( pRouter != NULL )
		{
			igmp_snp_syslog_dbg("find router port,ifIndex %d,vlan_id %d,saddr 0x%x.\n",\
								pRouter->ifindex, pRouter->vlan_id, pRouter->routeport_saddr);
			igmp_router_entry * Igmp_snoop_router_temp = NULL;
			Igmp_snoop_router_temp = ( igmp_router_entry * )malloc( sizeof( igmp_router_entry ));
			if ( NULL == Igmp_snoop_router_temp )
			{
				igmp_snp_syslog_err("igmp router entry alloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
			memset( Igmp_snoop_router_temp,0, sizeof( igmp_routerport ) );  

			Igmp_snoop_router_temp->mroute_ifindex = pRouter->ifindex;
			Igmp_snoop_router_temp->saddr = pRouter->routeport_saddr;
			Igmp_snoop_router_temp->next = NULL;

			/* add first route to vlannode, when vlannode have no route  */
			if (pVlanNode->routerlist == NULL)
			{igmp_snp_syslog_dbg("add first route %d to pVlanNode.\n", Igmp_snoop_router_temp->mroute_ifindex);
				pVlanNode->routerlist = Igmp_snoop_router_temp;
			}
			else
			{/* find the last position in vlannode's routerlist, and add the new route */
				igmp_router_entry *last_route = NULL;
				igmp_router_entry *pre_route = NULL;				
				last_route = pVlanNode->routerlist;
				pre_route = pVlanNode->routerlist;
				while (last_route != NULL)
				{
					pre_route = last_route;
					last_route = pre_route->next;					
				}
				pre_route->next = Igmp_snoop_router_temp;
				igmp_snp_syslog_dbg("add no first route %d to pVlanNode.\n", Igmp_snoop_router_temp->mroute_ifindex);
			}
		}
	}
	while(pRouter != NULL);

	if ( *ppPrevGroupVlan != NULL )		/* groupvlan node exists */
	{
		if ( (*ppPrevGroupVlan) ->vlan_id == lVid )	/* vlan node ( lvid ) exists, no need to create a mcgroupvlan node */
		{
			igmp_snp_syslog_dbg("groupvlan node %d has exists.\n", (*ppPrevGroupVlan) ->vlan_id);

			free( pVlanNode );/*used original one ,free new one just malloc()*/
			pVlanNode = *ppPrevGroupVlan;
			usIsCreate = IGMP_SNOOP_NO;
		}
		else
		{
			igmp_snp_syslog_dbg("groupvlan node %d no exists.\n", lVid);
			/*fillin the vlanNode into global mcGroupVlans,'mcgroup_vlan_queue[]'*/
			tmp = 0;
			while((tmp<IGMP_GENERAL_GUERY_MAX)&&(mcgroup_vlan_queue[tmp])){
				tmp++;
			}

			if( tmp < IGMP_GENERAL_GUERY_MAX ){
				mcgroup_vlan_queue[tmp] = pVlanNode;
				igmp_snp_syslog_dbg("create a new multicast-groupVlan,in mcgroup_vlan_queue[%d],and address is %p.\n",
									tmp,mcgroup_vlan_queue[tmp]);
			}
			/*if breach can happen here?*/
			#if 0
			if ( GET_VLAN_POINT_BY_INDEX(( *ppPrevGroupVlan ) ->next) != NULL ){
				( GET_VLAN_POINT_BY_INDEX((*ppPrevGroupVlan ) ->next)) ->prev = tmp;
			}

			pVlanNode->next = (*ppPrevGroupVlan)->next;
			if(GET_VLAN_POINT_BY_INDEX((*ppPrevGroupVlan)->prev)){
				pVlanNode->prev = GET_VLAN_POINT_BY_INDEX((*ppPrevGroupVlan)->prev)->next;
			}
			( *ppPrevGroupVlan ) ->next = tmp;
			#endif
			pVlanNode->next = -1;
			if(NULL != GET_VLAN_POINT_BY_INDEX(tmp-1)) {
				MC_group_vlan *pre_group_vlan;
				pre_group_vlan = GET_VLAN_POINT_BY_INDEX(tmp-1);
				pre_group_vlan->next = tmp;
			
				pVlanNode->prev = tmp-1;	
			}
			else {
				pVlanNode->prev = -1;
			}

			// set next point
			MC_group_vlan *next_group_vlan = NULL;
			int tmp_index = 0;
			
			tmp_index = tmp + 1;
			while (tmp_index < IGMP_GENERAL_GUERY_MAX) {
				next_group_vlan = GET_VLAN_POINT_BY_INDEX(tmp_index);
				if (NULL != next_group_vlan) {
					pVlanNode->next = tmp_index;
					next_group_vlan->prev = tmp;
					break;
				}
				
				tmp_index += 1;
			}

			( *ppPrevGroupVlan ) ->next = tmp;
			usIsCreate = IGMP_SNOOP_YES;
		}
	}
	else	/* The first mcgroupvlan node */
	{
		tmp = 0;
		while ((tmp < IGMP_GENERAL_GUERY_MAX) && (mcgroup_vlan_queue[tmp]))
		{
			tmp++;
		}
		mcgroup_vlan_queue[tmp] = pVlanNode;
		if (tmp == 0) {							/* add first mcgroup vlan, set index*/
			first_mcgroupvlan_idx = 0;
		}

		pVlanNode->next = -1;
		if (NULL != GET_VLAN_POINT_BY_INDEX(tmp-1))
		{
			MC_group_vlan *pre_group_vlan;
			pre_group_vlan = GET_VLAN_POINT_BY_INDEX(tmp-1);
			pre_group_vlan->next = tmp;
			
			pVlanNode->prev = tmp-1;
		}
		else
		{
			pVlanNode->prev = -1;
		}

		// set next point
		MC_group_vlan *next_group_vlan = NULL;
		int tmp_index = 0;

		tmp_index = tmp + 1;
		while (tmp_index < IGMP_GENERAL_GUERY_MAX) {
			next_group_vlan = GET_VLAN_POINT_BY_INDEX(tmp_index);
			if (NULL != next_group_vlan) {
				pVlanNode->next = tmp_index;
				next_group_vlan->prev = tmp;
				break;
			}
			
			tmp_index += 1;
		}

		usIsCreate = IGMP_SNOOP_YES;
		igmp_snp_syslog_dbg("multicast-groupVlan %d is not exist in mcgroup_vlan_queue, add new one.\n",
							pVlanNode->vlan_id);
	}
	*ppPrevGroupVlan = pVlanNode;  /* we need return this pointer.OutPut */

	pGroup->next = pVlanNode->firstgroup;

	if ( pVlanNode->firstgroup != NULL )
	{
		( pVlanNode->firstgroup ) ->prev = pGroup;
	}
	pGroup->prev= NULL;
	pVlanNode->firstgroup = pGroup;/*fillin the mcGroup into groupVlan structure*/

	/* Group count increase */
	igmp_groupcount++;

	//pGroup->mgroup_id = igmp_groupcount;//added by wujh
	//pGroup->mgroup_id = igmp_groupcount;
	igmp_searchvlangroup_by_groupcount(&ulGroup_id);
	pGroup->mgroup_id = ulGroup_id;
	igmp_snp_syslog_dbg("igmp create group_node: groupAddr 0x%x,groupId %d.\n",pGroup->MC_ipadd,pGroup->mgroup_id);
	/*flag indicate pVlanNode is new or original one*/
	if ( usIsCreate )
	{
		pVlanNode->vlan_id = lVid;
		pVlanNode->querytimeinterval = igmp_query_interval;
		pVlanNode->mcgvlanlife = igmp_vlanlife;
	}

	if ( pVlanNode->querytimer_id == 0 )
	{
		/* Add gen query timer */
		igmp_snp_addintertimer( igmp_query_interval, \
						&( pVlanNode->querytimer_id ) );
	}
	*ppMcGroup = pGroup; /* Output */

	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* igmp_snp_route_port_show()
*
* INPUTS:
*	 	unsigned short vlanId		-vlan id
*
* OUTPUTS:
*		unsigned int count		-count of ifindex fo route-port
*		long *eth_g_index		-ifindex array of route-port
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK	- on success.
*		IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST	- on error
*		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL	-the vlan is not support igmp-snooping
*
* DESCRIPTION:
*		This function show router port in mcGroupVlan.
*
**************************************************************************/
INT igmp_snp_route_port_show
(
	unsigned short	vlanId,
	unsigned int	*count,
	long			*eth_g_index_array
)
{
	unsigned int		ret = IGMPSNP_RETURN_CODE_OK;
	unsigned int		tmp_count = 0;
	MC_group_vlan		*t_gpvlan = NULL;
	igmp_router_entry	*Igmp_snoop_router_temp = NULL;

	if (IGMP_SNP_ISENABLE() || MLD_SNP_ISENABLE())
	{
		t_gpvlan = GET_VLAN_POINT_BY_INDEX(0);
	
		/*find the vlanid pointing vlan in mcgroupvlan list.*/
		while (NULL != t_gpvlan)
		{
			if (t_gpvlan->vlan_id != vlanId)
			{
				t_gpvlan = GET_VLAN_POINT_BY_INDEX(t_gpvlan->next);
				continue;
			}
			else
			{
				break;
			}
		}

		if (t_gpvlan != NULL && t_gpvlan->vlan_id == vlanId)
		{
			igmp_snp_syslog_dbg("find mcGroupVlan->vlanId %d, mcGroupVlan->routerlist@@%p.\n",
								t_gpvlan->vlan_id, t_gpvlan->routerlist);
			if (t_gpvlan->routerlist != NULL)
			{
				tmp_count = 0;
				Igmp_snoop_router_temp = t_gpvlan->routerlist;
				while (Igmp_snoop_router_temp != NULL)
				{
					eth_g_index_array[tmp_count] = Igmp_snoop_router_temp->mroute_ifindex;
					tmp_count++;
					Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
				}
			}
			*count = tmp_count;
			ret = IGMPSNP_RETURN_CODE_OK;
		}
		else
		{
			igmp_snp_syslog_err("not found mcGroupVlan %d.\n", vlanId);
			ret = IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST;
		}
	}
	else
	{
		igmp_snp_syslog_err("igmp or mld snoop is disable.\n");
		ret = IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	return ret;
}

/**************************************************************************
* igmp_snp_route_port_add()
*
* INPUTS:
*		 pRoutePort - the position maybe add action 
*		 lVid	- vlan id
*		 usIfIndex - port Index 
*
* OUTPUTS:
*		ppRouter - the added router port, when its value is NULL, add action is failed
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success.
*		IGMPSNP_RETURN_CODE_NULL_PTR - malloc fail
*		IGMPSNP_RETURN_CODE_ROUTE_PORT_EXIST - the usIfIndex has a router port in the lVid
*
* DESCRIPTION:
*		This function creates a new router porter in p_routerlist.
*
**************************************************************************/
INT igmp_snp_route_port_add
(
	igmp_routerport *pRoutePort,
	LONG lVid,
	LONG usIfIndex,
	igmp_routerport **ppRouter
)
{
	igmp_routerport *pTemp = NULL;
	igmp_routerport *pPort = NULL;
	igmp_routerport *pPrev = NULL;
	
	if (NULL == pRoutePort)
	{
		igmp_snp_syslog_err("parameter pRoutePort is NULL!\n");
		*ppRouter = NULL;
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	igmp_snp_syslog_dbg("add NOT first routePort: lVid %d, usIfindex %d.\n", lVid, usIfIndex);
	igmp_snp_syslog_dbg("sub-head routerPort@%p: vlan_id %d, ifindex %d,sip 0x%x",
						"sipv6 %4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x	next@%p.\n",\
						pRoutePort, pRoutePort->vlan_id, pRoutePort->ifindex,    \
						pRoutePort->routeport_saddr, ADDRSHOWV6(pRoutePort->routeport_saddrv6), pRoutePort->next);
	
	pTemp = pRoutePort;
	pPrev = pTemp;	
	while (NULL != pTemp && pTemp->vlan_id == lVid)
	{
		if (pTemp->ifindex == usIfIndex)
		{
			igmp_snp_syslog_dbg("port index %d already be route port.\n", usIfIndex);
			*ppRouter = pTemp;
			return IGMPSNP_RETURN_CODE_ROUTE_PORT_EXIST;
		}

		pPrev = pTemp;
		pTemp = pTemp->next;
		igmp_snp_syslog_dbg("get routeport pPrev %p,pTemp %p\n", pPrev, pTemp);
	}

	/*Not found same ifindex as route port*/
	pPort = (igmp_routerport *)malloc(sizeof(igmp_routerport));
	if (NULL == pPort)
	{
		igmp_snp_syslog_err("alloc memory failed.\n");
		*ppRouter = NULL;
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	memset(pPort, 0, sizeof(igmp_routerport));
	
	pPort->vlan_id = lVid;
	pPort->ifindex = usIfIndex;
	pPort->next = NULL;

	/* add node to list */
	pPrev->next = pPort;
	pPort->next = pTemp;

	*ppRouter = pPort;
	igmp_snp_syslog_dbg("Create new router-port %p added into vlan %d, port_index %d, sucess\n",
						pPort, lVid, usIfIndex);

	return IGMPSNP_RETURN_CODE_OK;
}

/*************************************************************************/
/**************************************************************************
* igmp_snp_searchrouterportlist()
*
* Input: 	  lVid 		-	vlan id 
*              usIfIndex - the port index which to be searched
*		  enflag		-  indicate the action to be taken 
* Output: 
* 		ppRouter	- pointer to pointer of router node 
*
*  RETURNS:
*  		IGMPSNP_RETURN_CODE_NULL_PTR  - pointer is null
*  		IGMPSNP_RETURN_CODE_NOTROUTE_PORT - not find route port
* 		IGMPSNP_RETURN_CODE_ERROR  -  not find 
* DESCRIPTION:
*		This function will search the router port list. If an element with the 
*		same port index to the input ifindex, then return the pointer to the
*		router node. If no router is found, there is a choice
*		whether create a new element and add to the list, the usFlag determine
*		how to do. If the usIfIndex == NULL and enFlag == IGMP_PORT_DEL, delete
*		any port in the vlan.
*	 
**************************************************************************/
LONG igmp_snp_searchrouterportlist( LONG lVid, ULONG usIfIndex, igmpportstate enFlag,
									igmp_routerport **ppRouter )

{
	igmp_routerport *pTemp = NULL;
	igmp_routerport *pPort = NULL;
	igmp_routerport *pPrev = NULL;
	int sRoutePort = 0;
	int del_flg = 0;
	int ret = IGMPSNP_RETURN_CODE_OK;

#if 1
//for debug
	igmp_routerport *loopupput = NULL;
	igmp_snp_syslog_dbg("loop for ouput the p_routerlist\n");

	loopupput = p_routerlist;
	while (NULL != loopupput)
	{
		igmp_snp_syslog_dbg("    vid[%d], port[%d], address[0x%x], addressv6 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", \
							loopupput->vlan_id, loopupput->ifindex, loopupput->routeport_saddr,ADDRSHOWV6(loopupput->routeport_saddrv6));
		loopupput = loopupput->next;
	}
#endif

	igmp_snp_syslog_dbg("search router-portlist: vid %d portifindex %d, flag %d\n",\
						lVid, usIfIndex, enFlag );
	if(IGMP_PORT_QUERY == enFlag) {
		if(NULL == *ppRouter) 
			pTemp = p_routerlist;
		else 
			pTemp = (*ppRouter)->next; // find next router port node
	}
	else {
		pTemp = p_routerlist;
	}

	pPrev = pTemp;
	while (NULL != pTemp)
	{
		if (pTemp->vlan_id == lVid)
		{
			break;
		}
		pPrev = pTemp;				/*NOT pPrev->next = pTemp;*/
		pTemp = pTemp->next;
	}

	if (NULL != pTemp)             /* Found the vlan */
	{
		igmp_snp_syslog_dbg("find route port: vid= %d,ifIndex= %d.\n", pTemp->vlan_id, pTemp->ifindex);
		if (IGMP_PORT_DEL == enFlag)
		{
			while (NULL != pTemp && lVid == pTemp->vlan_id)
			{
				del_flg++;
				if (pTemp->ifindex == usIfIndex)
				{
					igmp_snp_syslog_dbg("found route port:index %d .\n", usIfIndex);
					if (pTemp == p_routerlist)
					{
						p_routerlist = pTemp->next;	
					}
					else
					{
						pPrev->next = pTemp->next;
					}

					free(pTemp);
					pTemp = NULL;
					igmp_snp_syslog_dbg("delete router port ok in p_routerlist.\n");
					return IGMPSNP_RETURN_CODE_OK;
				}
				else
				{
					igmp_snp_syslog_dbg("search times %d!", del_flg);
					pPrev = pTemp;
					pTemp = pPrev->next;
				}
			}

			igmp_snp_syslog_dbg("Error: No such Route port configured in p_routerlist.\n");
			return IGMPSNP_RETURN_CODE_NOTROUTE_PORT;
		}
		else if (IGMP_PORT_ADD == enFlag)
		{
			/*for add router port in the same vlan */
			ret = igmp_snp_route_port_add(pTemp, lVid, usIfIndex, ppRouter);
			switch (ret)
			{
				case IGMPSNP_RETURN_CODE_OK:
				case IGMPSNP_RETURN_CODE_ROUTE_PORT_EXIST:
				case IGMPSNP_RETURN_CODE_NULL_PTR:
					return ret;
			}
		}
		else if (IGMP_PORT_QUERY == enFlag)
		{
			*ppRouter = pTemp;/*possible to return a router port that ifindex is not equal to input port ifindex*/
			igmp_snp_syslog_dbg("query, found route port %d of vlan %d in p_routerlist.\n", pTemp->vlan_id, pTemp->ifindex);	
			return IGMPSNP_RETURN_CODE_OK;
		}
	}

	/* If NOT FOUND, look at the usflag */
	/* Code below can only be executed once,the first router port in the list create.*/
	igmp_snp_syslog_dbg("not found any route-port of vlan %d in p_routerlist.\n", lVid);
	switch (enFlag)
	{
		case IGMP_PORT_QUERY:
			*ppRouter = NULL;
			break;
		case IGMP_PORT_ADD:
			{
				/* create a new list element and add to node */
				pPort = (igmp_routerport *)malloc(sizeof(igmp_routerport));
				if (NULL == pPort)
				{
					igmp_snp_syslog_err("alloc memory for routerport failed.\n");
					*ppRouter = NULL;
					return IGMPSNP_RETURN_CODE_NULL_PTR;
				}
				memset(pPort, 0, sizeof(igmp_routerport));
				pPort->vlan_id = lVid;
				pPort->ifindex = usIfIndex;
				pPort->next = NULL; /*p_routerlist*/
				
				if (NULL == pPrev)
				{
					p_routerlist = pPort;
				}
				else
				{/*for add router port in new vlan*/
					pPrev->next = pPort;
					pPort->next = pTemp;
				}
				igmp_snp_syslog_dbg("Add first routerPort:pPort@@%p,vlanId %d,ifIndex %d, sucess.\n",pPort,lVid,usIfIndex);
				
				*ppRouter = pPort;
			}
			break;
		case IGMP_PORT_DEL:
			return IGMPSNP_RETURN_CODE_ERROR;       /* not found */
		default :
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_snp_searchportstatelist()
*
* Input: usIfIndex - the port index which to be searched 
*		  pPortList -  port list head
*		  enflag		-  indicate the action to be taken 
* Output: ppPortState	- pointer to pointer of struct MC_port_state 
*		   pPortList - list head 
*
*RETURNS:
*		IGMPSNP_RETURN_CODE_PORT_NOT_EXIST - not such port in mcgroup
*		IGMPSNP_RETURN_CODE_OK -on success
*		IGMPSNP_RETURN_CODE_NULL_PTR - pointer is null
*
* DESCRIPTION:
*		This function will search a portstate list. If an element with the 
*		same port index to the input ifindex, then return the pointer to the
*		portState block. If no portstate element is found, there is a choice
*		whether create a new element and add to the list, the usFlag determine
*		how to do.
*	 
**************************************************************************/
LONG igmp_snp_searchportstatelist( struct MC_port_state ** ppPortList,
					ULONG usIfIndex, igmpportstate enFlag,
					struct MC_port_state **ppPortState )
{
	struct MC_port_state *pTemp = NULL;
	struct MC_port_state *pPort = NULL;
	struct MC_port_state *pPrev = NULL;

	igmp_snp_syslog_dbg("search port state list port %d, flag %d groupPortList 0x%x\n",
						usIfIndex, enFlag, *ppPortList );
	pTemp = *ppPortList;
	pPrev = pTemp;
	while ( NULL != pTemp )
	{
		if ( pTemp->ifindex == usIfIndex ){
			break;
		}
		pPrev = pTemp;
		pTemp = pTemp->next;
	}

	if ( NULL != pTemp )             /* Found the port */
	{
#if 0
		if ( IGMP_PORT_DEL == enFlag )	{
			if(pTemp->ifindex == usIfIndex) {
				if ( pTemp == *ppPortList )         	/* if it is the list head, set the list head to NULL */
				{
					*ppPortList = pTemp->next;
				}

				/*delete the mcgroup port found.*/
				if ( NULL != pPrev )
				{
					pPrev->next = pTemp->next;
				}

				free( pTemp );
				return IGMP_SNOOP_OK;
			}
			else {
				*ppPortState = pTemp;
				igmp_snp_syslog_err("Error: No such port in mcgroup.\n");
				return IGMP_SNOOP_ERR;
			}
		}
#endif
		if ( IGMP_PORT_DEL == enFlag )	{
			if(pTemp->ifindex == usIfIndex) {
				/*delete the mcgroup port found.*/
				if (pTemp == *ppPortList)
				{
					*ppPortList = pTemp->next;
				}
				else
				{
					pPrev->next = pTemp->next;
				}
				free(pTemp);
				pTemp = NULL;
				return IGMPSNP_RETURN_CODE_OK;
			}
			else {
				*ppPortState = pTemp;
				igmp_snp_syslog_err("Error: No such port in mcgroup.\n");
				return IGMPSNP_RETURN_CODE_PORT_NOT_EXIST;
			}
		}
		else if ( IGMP_PORT_ADD == enFlag )
		{
			if(pTemp->ifindex == usIfIndex) {
				/* need to reboot the  MembershipTimer */
				igmp_snp_addintertimer((igmp_robust_variable * igmp_query_interval + igmp_resp_interval),
										&(pTemp->membertimer_id ));
				*ppPortState = pTemp;
				igmp_snp_syslog_warn("Warning: port already exist in mcgroup.\n");
				return IGMPSNP_RETURN_CODE_OK;
			}
			else {
				struct MC_port_state *newPort = NULL;
				newPort = (struct MC_port_state *)malloc(sizeof(struct MC_port_state));
				if(NULL == newPort){
					igmp_snp_syslog_err("Error: new port malloc fail.\n");
					return IGMPSNP_RETURN_CODE_NULL_PTR;
				}
				memset( newPort,0, sizeof( struct MC_port_state ) );
				newPort->ifindex = usIfIndex;
				/* need to boot the  MembershipTimer */
				igmp_snp_addintertimer((igmp_robust_variable * igmp_query_interval + igmp_resp_interval),
										&(newPort->membertimer_id));
				newPort->state = IGMP_SNP_GROUP_NOMEMBER;
				pTemp->next = newPort;

				*ppPortState = newPort;/*return newPort*/
				return IGMPSNP_RETURN_CODE_OK;
			}
		}
		else
		{
			if(pTemp->ifindex == usIfIndex) {
				*ppPortState = pTemp;
				igmp_snp_syslog_dbg("found the port %d in port state list.\n", pTemp->ifindex);
			}
			else {
				*ppPortState = NULL;
			}
		}
		return IGMP_SNOOP_OK;
	}

	/* If NOT FOUND, look at the usflag */
	switch ( enFlag )
	{
		case IGMP_PORT_QUERY:
			*ppPortState = NULL;
			igmp_snp_syslog_err("not found port %d in port state list\n", usIfIndex);
			break;
		case IGMP_PORT_ADD:
			{
			/* create a new list element and add to nod*/
			pPort = ( struct MC_port_state * ) malloc( sizeof( struct MC_port_state ));
			if ( NULL == pPort )
			{
				igmp_snp_syslog_err(("search port state list: alloc memory failed. \n"));
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
			memset( pPort,0, sizeof( struct MC_port_state ) );
			pPort->ifindex = usIfIndex;
			pPort->state = IGMP_SNP_GROUP_NOMEMBER;
			pPort->next = *ppPortList;
			*ppPortList = pPort;
			igmp_snp_syslog_dbg("search port state list: add first member@@%p, member->next@@%p.\n",pPort,pPort->next);
			*ppPortState = pPort;
			}
			break;
		case IGMP_PORT_DEL:
			return IGMPSNP_RETURN_CODE_NO_SUCH_PORT;       /* not found */
		default :
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_snp_searchreporterlist()
*
* Input: ulIfIndex - the port index which to be searched 
*		  lVid 		-	vlan id
*		  enflag		-  indicate the action to be taken 
* Output: ppReporter	- pointer to pointer of reporter node 
*
*RETURNS:
*		IGMPSNP_RETURN_CODE_ALREADY_SET - set again
*		IGMPSNP_RETURN_CODE_NO_SUCH_PORT - can find the port
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc the memery fail
*		IGMPSNP_RETURN_CODE_OK - on success
*		
* DESCRIPTION:
*		This function will search the reporter list. If an element with the 
*		same port index to the input ifindex, then return the pointer to the
*		reporter node. If no reporter is found, there is a choice
*		whether create a new element and add to the list, the usFlag determine
*		how to do. If the ulIfIndex == NULL and enFlag == IGMP_PORT_DEL, delete
*		any port in the vlan.
*	 
**************************************************************************/
LONG igmp_snp_searchreporterlist( LONG lVid, ULONG ulIfIndex, igmpportstate enFlag,
					igmp_reporter_port **ppReporter )
{
	igmp_reporter_port *pTemp = NULL;
	igmp_reporter_port *pPort = NULL;
	igmp_reporter_port *pPrev = NULL;
	member_port_list *pMemberPort = NULL, *pTempMemberPort = NULL, *pPreTempMemberPort = NULL;

	igmp_snp_syslog_dbg("search reporter list in vlan: vid %d port 0x%x, flag %d \n",
									lVid, ulIfIndex, enFlag);
	/*Find the report list according vid*/
	pTemp = p_reporterlist;
	pPrev = pTemp;
	while ( NULL != pTemp )
	{
		if ( pTemp->vlan_id == lVid )
			break;
		pPrev = pTemp;
		pTemp = pTemp->next;
	}

	/*Found the report list*/
	if ( NULL != pTemp )
	{
		igmp_snp_syslog_dbg( "search reporter list in vlan:found vlan\n");
		if ( IGMP_PORT_DEL == enFlag )/*IGMP_PORT_DEL proccess here!*/
		{
			pTempMemberPort = pTemp->portlist;
			pPreTempMemberPort = pTempMemberPort;

			while ( NULL != pTempMemberPort )
			{
				if ( ulIfIndex == pTempMemberPort->ifindex )
				{
					if ( pPreTempMemberPort == pTempMemberPort && NULL == pTempMemberPort->next )
					{ /*The last member port is deleted from vlan,so del the vlan node*/
						free( pTempMemberPort );

						if ( pPrev == pTemp && NULL == pTemp->next )
						{ /*The last vlan node in member port list*/
							free( pTemp );
							p_reporterlist = NULL;
						}
						else if ( pPrev == pTemp )
						{ /*del the first vlan node in portlist*/
							p_reporterlist = pTemp->next;
							free( pTemp );
						}
						else
						{
							pPrev->next = pTemp->next;
							free( pTemp );
						}
					}
					else if ( pPreTempMemberPort == pTempMemberPort )
					{ /*del the first member port in portlist*/
						pTemp->portlist = pTempMemberPort->next;
						free( pTempMemberPort );
					}
					else
					{
						pPreTempMemberPort->next = pTempMemberPort->next;
						free( pTempMemberPort );
					}
					break;
				}
				pPreTempMemberPort = pTempMemberPort;
				pTempMemberPort = pTempMemberPort->next;
			}
			if ( NULL == pTempMemberPort )
			{
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
		}
		else if ( IGMP_PORT_ADD == enFlag )  /*The member port is already in portlist when add member port*/
		{
			pTempMemberPort = pTemp->portlist;
			pPreTempMemberPort = pTempMemberPort;

			while ( NULL != pTempMemberPort )
			{
				if ( ulIfIndex == pTempMemberPort->ifindex )
				{ /*This port is in portlist already*/
					return IGMPSNP_RETURN_CODE_ALREADY_SET;
				}

				pPreTempMemberPort = pTempMemberPort;
				pTempMemberPort = pTempMemberPort->next;
			}

			if ( NULL == pTempMemberPort )
			{
				pMemberPort = ( member_port_list * )malloc( sizeof( member_port_list ));
				if ( NULL == pMemberPort )
				{
					igmp_snp_syslog_dbg( ( "search reporter list in vlan:  alloc mem fail. \n" ) );
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset(pMemberPort,0,sizeof(member_port_list));
				pMemberPort->ifindex = ulIfIndex;
				pMemberPort->next = NULL;
				pPreTempMemberPort->next = pMemberPort;
			}
		}
		else if ( IGMP_PORT_QUERY == enFlag )
		{
			*ppReporter = pTemp;
		}
		return IGMPSNP_RETURN_CODE_OK;
	}

	igmp_snp_syslog_dbg("no found reporter port in vlan %d\n",lVid);

	/* If NOT FOUND, look at the usflag */
	switch ( enFlag )
	{
		case IGMP_PORT_QUERY:
			*ppReporter = NULL;
			break;
		case IGMP_PORT_ADD:
			{
			/* create a new port node and add to list*/
			pPort = ( igmp_reporter_port * )malloc( sizeof( igmp_reporter_port ));
			if ( NULL == pPort )
			{
				igmp_snp_syslog_dbg("search reporter list in vlan: alloc memory failed. \n" );
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset( pPort,0, sizeof( igmp_reporter_port ) );

			pMemberPort = ( member_port_list * )malloc( sizeof( member_port_list ));
			if ( NULL == pMemberPort )
			{
				igmp_snp_syslog_dbg( "search reporter list in vlan: alloc memory failed. \n" );
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset( pMemberPort, 0,sizeof( member_port_list ) );

			pPort->vlan_id= lVid;
			pMemberPort->ifindex = ulIfIndex;
			pMemberPort->next = NULL;
			pPort->portlist = pMemberPort;

			pPort->next = p_reporterlist;
			p_reporterlist = pPort;

			*ppReporter = pPort;
			}
			break;

		case IGMP_PORT_DEL:
			return IGMPSNP_RETURN_CODE_NO_SUCH_PORT;       /* not found */
		default :
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * Igmp_Snoop_Del_Reporter_ByVlan
 *
 * DESCRIPTION:
 *   Igmp_Snoop_Del_Reporter_ByVlan
 *
 * INPUTS:
 * 	lVid  - vlan id
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	IGMPSNP_RETURN_CODE_OK - delete successfully or no this vlan in member port list
 *
 * COMMENTS:
 *      comments here
 **
 ********************************************************************************/
LONG Igmp_Snoop_Del_Reporter_ByVlan( USHORT lVid )
{
	igmp_reporter_port * pTemp = NULL;
	igmp_reporter_port *pPrev = NULL;

	igmp_snp_syslog_dbg( "Igmp_Snoop_Del_Reporter_ByVlan: vid %d \n", lVid);

	/*Find the vlan node*/
	pTemp = p_reporterlist;
	pPrev = pTemp;
	while ( NULL != pTemp )
	{
		if ( pTemp->vlan_id== lVid )
			break;
		pPrev = pTemp;
		pTemp = pTemp->next;
	}

	if ( NULL != pTemp )
	{
		if ( pPrev == pTemp && NULL == pTemp->next )
		{ /*The last node in member port list*/
			free( pTemp );
			p_reporterlist = NULL;
		}
		else
		{
			if ( pTemp == pPrev )
			{
				p_reporterlist = pTemp->next;
			}
			else
			{
				pPrev->next = pTemp->next;
			}
			free( pTemp );
		}
	}
	else
	{
		igmp_snp_syslog_dbg("Igmp_Snoop_Del_Reporter_ByVlan:  no this vlan in member port list\n");
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_snp_vlan_port_delete
 *
 * DESCRIPTION:
 *   	delete the igmp vlan from vlanlist
 *
 * INPUTS:
 * 	pPkt  - igmp snoop pkt structure
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	IGMPSNP_RETURN_CODE_OK   	- on success
 * 	IGMPSNP_RETURN_CODE_ERROR	- the vlan port not exist
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
LONG igmp_snp_vlan_port_delete( igmp_snoop_pkt* pPkt)
{
	/*delete port from igmp_vlan_port*/
	igmp_vlan_list	*pVlan = NULL;
	igmp_vlan_port_list *pVlanPort = NULL, *preVlanPort = NULL;
	pVlan = p_vlanlist;
	if(pVlan != NULL){
		while(pVlan){
			if(pVlan->vlan_id == pPkt->vlan_id){
				break;
			}
			pVlan = pVlan->next;
		}
		if(pVlan == NULL || pVlan->vlan_id != pPkt->vlan_id){
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		else {
			pVlanPort = pVlan->first_port;
			while(pVlanPort){
				if(pVlanPort->ifindex == pPkt->ifindex){
					break;
				}
				preVlanPort = pVlanPort;
				pVlanPort = pVlanPort->next;
			}
			if(pVlanPort == NULL || pVlanPort->ifindex != pPkt->ifindex){
				return IGMPSNP_RETURN_CODE_ERROR;
			}
			else {
				if(pVlanPort == pVlan->first_port){
					pVlan->first_port = NULL;
				}
				else {
					preVlanPort->next = pVlanPort->next;
				}
			}
			free(pVlanPort);
			return IGMPSNP_RETURN_CODE_OK;
		}
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * Igmp_Event_AddVlan
 *
 * DESCRIPTION:
 * 		increase vlan port to the vlanlist
 *
 * INPUTS:
 * 	vid  - vlan id
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK   	                     - on success
 *		IGMPSNP_RETURN_CODE_OUT_RANGE	              - the vid is illegal
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL      - the malloc fail
 *		IGMPSNP_RETURN_CODE_ALREADY_SET            - vlan already exist
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
LONG Igmp_Event_AddVlan(unsigned short vid)
{
	igmp_snp_syslog_dbg("igmp_snp_vlan_add_dbus::add vlan %d\n",vid);
	igmp_vlan_list *t_vlan = NULL;
	igmp_vlan_list *pre_vlan = NULL;
	if( vid > 4094 )	/*max value*/
	{
		igmp_snp_syslog_dbg("igmp_cfg_add_vlan:parameter error.\n");
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	else
	{
		if(!p_vlanlist)	/*first*/
		{
			igmp_snp_syslog_dbg("The first vlan in the system.\n");
			t_vlan = (igmp_vlan_list *)malloc(sizeof(igmp_vlan_list));
			if(NULL == t_vlan )
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_vlan:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(t_vlan,0,sizeof(igmp_vlan_list));
			t_vlan->next = NULL;
			t_vlan->vlan_id = vid;
			t_vlan->first_port = NULL;
			p_vlanlist = t_vlan;
		}
		else
		{
			igmp_vlan_list *new_vlan =NULL;
			t_vlan = p_vlanlist;
			while(t_vlan){
				if(t_vlan->vlan_id == vid){
					igmp_snp_syslog_dbg("igmp_cfg_add_vlan:vlan_id alread existence.\n");
					return IGMPSNP_RETURN_CODE_ALREADY_SET;
				}
				pre_vlan = t_vlan;
				t_vlan = t_vlan->next;
			}
			#if 0
			be taken placed by while() above
			while((t_vlan->vlan_id != vid)&&(NULL != t_vlan->next ))/*==, !=*/
			{
				t_vlan = t_vlan->next;
			}
			if( t_vlan->vlan_id == vid)
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_vlan:vlan_id alread existence.\n");
				return IGMP_SNOOP_ERR;
			}
			#endif
			new_vlan= (igmp_vlan_list *)malloc(sizeof(igmp_vlan_list));
			if(NULL == new_vlan )
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_vlan:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(new_vlan,0,sizeof(igmp_vlan_list));
			new_vlan->next = NULL;
			new_vlan->vlan_id = vid;
			new_vlan->first_port = NULL;
			pre_vlan->next = new_vlan;
			//t_vlan->next = new_vlan;
		}
	}
	igmp_vlancount++;
	return IGMPSNP_RETURN_CODE_OK;
	/*notify to Hw, to enable IGMP in vlan.*/
}


/**************************************************************************
* Igmp_Event_DelVlan()
*
* INPUTS:
*		pPkt - Igmp_Snoop_pkt_t structure pointer 	
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -   sucess to delete vlan or vlan not exist already
*
*
* DESCRIPTION:
*		Igmp_Event_VlanDelPort handles the vlan deleting event. It will
*		delete the vlan node and all group node belongs to this vlan.
*		
*
**************************************************************************/
LONG Igmp_Event_DelVlan( igmp_snoop_pkt * pPkt )
{
	igmp_snp_syslog_dbg( "\nEnter Igmp_Event_DelVlan. \n");
	/*delete vlan node from igmp_vlan_list*/
	igmp_vlan_list *pVlan = NULL,*preVlan = NULL;

	igmp_snp_syslog_dbg("pPkt->vlan_id=[%d]\n", pPkt->vlan_id);
	
	pVlan = p_vlanlist;
	while(pVlan){
		if(pVlan->vlan_id == pPkt->vlan_id) {
			igmp_snp_syslog_dbg("find the vlan=[%d] need to delete.\n", pPkt->vlan_id);
			break;
		}
		preVlan = pVlan;
		pVlan = pVlan->next;
	}
	if( NULL ==pVlan || pVlan->vlan_id != pPkt->vlan_id){
		return IGMPSNP_RETURN_CODE_OK;
	}
	else {
		if(pVlan == p_vlanlist){
			p_vlanlist = NULL;
			igmp_vlancount = 0;
		}
		else {
			preVlan->next = pVlan->next;
			igmp_vlancount --;
		}
		free( pVlan);
		igmp_snp_syslog_dbg("delete the vlan=[%d] in p_vlanlist, next step to delete mcgroups.\n", pPkt->vlan_id);
		/* search the vlan node
		Notice: DO NOT use the group search routine, because it will validate the vlan id 
		and it is possible that the mcgroupvlan didn't exist at that time */
		igmp_snp_del_mcgroupvlan( pPkt->vlan_id);
		Igmp_Snoop_Del_Reporter_ByVlan( pPkt->vlan_id);

	}

	/*notify to Hw, to enable IGMP in vlan.*/

	return IGMPSNP_RETURN_CODE_OK;
}

 /*******************************************************************************
 * Igmp_Event_PortUp:
 *
 * DESCRIPTION:
 *		when Port Up from State of Down,we should find all the 
 *		vlan that the port once belonged to. And Rebuild the
 *		entries,such as: fdb entry,Vidx entry.So,we must NOT 
 *		delete data structrues according to the port.This means 
 *		what we should do is NOT delete anything when port Down?
 *
 * INPUTS:
 * 		vid  - vlan id
 *    	port_index  - the up port info
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 	
 * 	IGMPSNP_RETURN_CODE_NULL_PTR	- can not find any vlan or malloc memory failed
 * 	IGMPSNP_RETURN_CODE_ALREADY_SET - port already exist
 *
 * COMMENTS:
 *   	Do nothing originally.   
 **
 ********************************************************************************/
LONG Igmp_Event_PortUp
(
	unsigned short vid,
	unsigned int port_index
)
{
	igmp_snp_syslog_dbg("Enter add vlan port...\n");
	int i,vlan_id,ifindex;
	igmp_vlan_list *t_vlan = NULL;

	vlan_id = vid;
	ifindex = port_index;

	if( !p_vlanlist )
	{
		igmp_snp_syslog_dbg("Igmp_Event_PortUp: can not find any vlan.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	else
	{
		t_vlan = p_vlanlist;
		/*find igmp_vlan by vlan_id*/
		while((t_vlan->vlan_id != vlan_id )&&(NULL != t_vlan))/*==,!= wujh*/
			t_vlan = t_vlan->next;
		if( NULL == t_vlan )
		{
			igmp_snp_syslog_dbg("igmp_cfg_add_port:can not find vlan.\n");
			return IGMPSNP_RETURN_CODE_NULL_PTR;
		}
		
		igmp_vlan_port_list *t_port = t_vlan->first_port;
		if( NULL == t_port )		/*first*/
		{
			igmp_snp_syslog_dbg("add first port...\n");
			t_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if( NULL == t_port)
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_port:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
			memset(t_port,0,sizeof(igmp_vlan_port_list));
			t_port->next = NULL;
			t_port->ifindex = ifindex;/*t_port->ifindex is just portNum.*/
			t_port->trunkflag = 0;		
			t_vlan->first_port = t_port;
			igmp_snp_syslog_dbg("vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		else
		{
			/*igmp_vlan has port member,find out whether exist,if not,create port node*/
			igmp_vlan_port_list *new_port = NULL;
			while( t_port->ifindex != ifindex )
			{
				if( NULL == t_port->next)
					break;
				t_port = t_port->next;
			}
			if( t_port->ifindex == ifindex)
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_port:ifindex has alread existence.\n");
				return IGMPSNP_RETURN_CODE_ALREADY_SET;
			}
			/*create port node*/
			new_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if(NULL == new_port)
			{
				igmp_snp_syslog_dbg("igmp_cfg_add_port:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(new_port,0,sizeof(igmp_vlan_port_list));
			new_port->next = NULL;
			new_port->ifindex = ifindex;
			new_port->trunkflag = 0;	
			t_port->next = new_port;
			igmp_snp_syslog_dbg("239vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\n",\
				t_vlan->vlan_id,new_port->ifindex,new_port->trunkflag,new_port->next);
		}

	}
}
 /*******************************************************************************
  * Igmp_Event_PortDown:
  *
  * DESCRIPTION:
  * 			Del vlan router port,Del group member.	
  * 	 
  * INPUTS:
  * 			 pPkt  - igmp snoop pkt structure
  *
  * OUTPUTS:
  * 	 
  *
  * RETURNS:
  *  		IGMPSNP_RETURN_CODE_OK	 - on success
  *
  * COMMENTS:
  * 		An IGMP snooping switch should be aware of link layer topology
  *		changes caused by Spanning Tree operation. When a port is
  *		enabled or disabled by Spanning Tree, a General Query may be
  *		sent on all active non-router ports in order to reduce network
  *		convergence time.  
  **
  ********************************************************************************/
LONG Igmp_Event_PortDown( igmp_snoop_pkt * pPkt )
{
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;

	LONG lRet;
	LONG lVid;
	struct MC_port_state *pstPort = NULL;
	igmp_snoop_pkt stPkt;
	mld_snoop_pkt mldstPkt; 
	igmp_router_entry * Igmp_snoop_router_temp;
	igmp_router_entry ** Igmp_snoop_router_pre;

	igmp_snp_syslog_event("Igmp Event:PortDown\n" );

	pPrevGroupVlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
	while (pPrevGroupVlan != NULL)
	{     
		lVid = pPrevGroupVlan->vlan_id;
		if( pPkt->vlan_id && lVid != pPkt->vlan_id)/*?????????*/
		{
			if( pPrevGroupVlan->next == -1)	/*到队列尾*/
				break;	
			pPrevGroupVlan = GET_VLAN_POINT_BY_INDEX(pPrevGroupVlan->next);
			continue;
		}
		/* delete vlan router port */
		/* if router port is deleted from vlan, delete it from port list */
		igmp_snp_syslog_dbg("if the downport is a router port Del this router port.\n");
		Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
		Igmp_snoop_router_pre = &( pPrevGroupVlan->routerlist );
		while ( Igmp_snoop_router_temp != NULL )
		{
			if ( Igmp_snoop_router_temp->mroute_ifindex == pPkt->ifindex )
			{
				*Igmp_snoop_router_pre = Igmp_snoop_router_temp->next;
				free( Igmp_snoop_router_temp );
				Igmp_snoop_router_temp = NULL;
				break;
			}
			Igmp_snoop_router_pre = &( Igmp_snoop_router_temp->next );
			Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
		}

		igmp_snp_syslog_dbg(" Igmp_Event_PortDown:if the downport is a group member port Del this group member.\n");
		pMcGroup = pPrevGroupVlan->firstgroup;
		while ( pMcGroup != NULL )
		{
			if ( Igmp_Snoop_IF_Recv_Is_Report_Discard( lVid, pPkt->ifindex ) == IGMPSNP_RETURN_CODE_ERROR )
			{
				igmp_snp_syslog_dbg("This is a reporter port. \n");
				free( pPkt );
				return IGMPSNP_RETURN_CODE_OK;
			}
			/* scan portlist */
			lRet = igmp_snp_searchportstatelist( &( pMcGroup->portstatelist ), pPkt->ifindex,
			                        IGMP_PORT_DEL, &pstPort );
			if ( NULL == pMcGroup->portstatelist )
			{
				igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
			}
			igmp_snp_syslog_dbg( " Igmp_Event_PortDown:Enter Del group member.\n");

			if ( (IGMPSNP_RETURN_CODE_OK == lRet) && ( 0 != pMcGroup->MC_ipadd) )
			{
				stPkt.type = IGMP_ADDR_DEL;
				stPkt.ifindex = pPkt->ifindex;
				stPkt.groupadd= pMcGroup->MC_ipadd;
				stPkt.vlan_id= lVid;
				//stPkt.group_id;
				igmp_snp_syslog_dbg("Here call :igmp_snp_mod_addr!\n");
				if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_mod_addr(&stPkt,IGMP_ADDR_DEL)))
				{
					igmp_snp_syslog_dbg("Igmp_Event_PortDown: failed in setting L2 MC table. Slot/port: 0x%x \n", pPkt->ifindex);
				}
			}
			else if((IGMPSNP_RETURN_CODE_OK == lRet)&&( 0 == pMcGroup->MC_ipadd )){
				mldstPkt.type = MLD_ADDR_DEL;
				mldstPkt.ifindex = pPkt->ifindex;
				USHORT *pstIp6 = mldstPkt.groupadd;
				mld_copy_ipv6addr(pMcGroup->MC_v6ipadd, &(pstIp6));
				mldstPkt.vlan_id= lVid;
				
				igmp_snp_syslog_dbg("Here call :igmp_snp_mod_addr!\n");
				if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_mod_addr(&mldstPkt,MLD_ADDR_DEL)))
				{
					igmp_snp_syslog_dbg("Igmp_Event_PortDown: failed in setting L2 MC table via mld snp mod addr."\
										"Slot/port: 0x%x \n", pPkt->ifindex);
				}					
			}
			pMcGroup = pMcGroup->next;
		}
		if( pPrevGroupVlan->next == -1)	/*到队列尾*/
			break;	
		pPrevGroupVlan = GET_VLAN_POINT_BY_INDEX(pPrevGroupVlan->next);
	}
	/*
	An IGMP snooping switch should be aware of link layer topology
	changes caused by Spanning Tree operation. When a port is
	enabled or disabled by Spanning Tree, a General Query may be
	sent on all active non-router ports in order to reduce network
	convergence time. */
	Igmp_Event_GenQuery_Timeout( pPrevGroupVlan );
	mld_Event_GenQuery_Timeout( pPrevGroupVlan );
	free( pPkt );
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * Igmp_Event_VlanAddPort
 *
 * DESCRIPTION:
 * 		increase vlan port to the vlanlist
 *
 * INPUTS:
 * 		pPkt  - igmp snoop pkt structure
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK   	                     - on success
 *		IGMPSNP_RETURN_CODE_OUT_RANGE	              - the vid is illegal
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL      - the malloc fail
 *		IGMPSNP_RETURN_CODE_ALREADY_SET            - vlan already exist
 *		IGMPSNP_RETURN_CODE_NULL_PTR			-  can not find vlan
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
LONG Igmp_Event_VlanAddPort( igmp_snoop_pkt * pPkt )
{
	igmp_snp_syslog_dbg("Enter Igmp_Event_VlanAddPort...\n");
	int i,vlan_id,ifindex;
	igmp_vlan_list *t_vlan = NULL;

	vlan_id = pPkt->vlan_id;
	ifindex = pPkt->ifindex;

	if( !p_vlanlist )
	{
		igmp_snp_syslog_dbg("Igmp_Event_VlanAddPort: can not find any vlan.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	else
	{
		t_vlan = p_vlanlist;
		/*find igmp_vlan by vlan_id*/
		while((t_vlan->vlan_id != vlan_id )&&(NULL != t_vlan))/*==,!= wujh*/
			t_vlan = t_vlan->next;
		if( NULL == t_vlan )
		{
			igmp_snp_syslog_dbg("Igmp_Event_VlanAddPort:can not find vlan.\n");
			return IGMPSNP_RETURN_CODE_NULL_PTR;
		}
		
		igmp_vlan_port_list *t_port = t_vlan->first_port;
		if( NULL == t_port )		/*first*/
		{
			igmp_snp_syslog_dbg("add first port...\n");
			t_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if( NULL == t_port)
			{
				igmp_snp_syslog_dbg("Igmp_Event_VlanAddPort:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			t_port->next = NULL;
			t_port->ifindex = ifindex;/*t_port->ifindex is just portNum.*/
			t_port->trunkflag = 0;		
			t_vlan->first_port = t_port;
			igmp_snp_syslog_dbg("vlan %ld port ifindex %ld,trunkflag %ld,next port pointer %p\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		else
		{
			/*igmp_vlan has port member,find out whether exist,if not,create port node*/
			igmp_vlan_port_list *new_port = NULL;
			while( t_port->ifindex != ifindex )
			{
				if( NULL == t_port->next)
					break;
				t_port = t_port->next;
			}
			if( t_port->ifindex == ifindex)
			{
				igmp_snp_syslog_dbg("Igmp_Event_VlanAddPort:ifindex has alread existence.\n");
				return IGMPSNP_RETURN_CODE_ALREADY_SET;
			}
			/*create port node*/
			new_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if(NULL == new_port)
			{
				igmp_snp_syslog_dbg("Igmp_Event_VlanAddPort:malloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			new_port->next = NULL;
			new_port->ifindex = ifindex;
			new_port->trunkflag = 0;	
			t_port->next = new_port;
			igmp_snp_syslog_dbg("vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\n",\
				t_vlan->vlan_id,new_port->ifindex,new_port->trunkflag,new_port->next);
		}

	}
}


/**************************************************************************
* Igmp_Event_VlanDelPort()
*
* INPUTS:
*		pPkt - igmp_snoop_pkt structure pointer 	
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  operation succuss
*		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST  - search fail
*		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - not found vlan
*
* DESCRIPTION:
*		Igmp_Event_VlanDelPort handles the vlan port deleting event. It will
*	iterate vlan group list, search the port and delete from group member .
*
* CALLED BY:
*		Igmp_TimerExp_Proc()
* CALLS:
*		
*
**************************************************************************/
LONG Igmp_Event_VlanDelPort( igmp_snoop_pkt * pPkt )
{
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;

	LONG lRet;
	struct MC_port_state *pstPort = NULL;
	igmp_snoop_pkt stPkt;
	mld_snoop_pkt mldstPkt;
	igmp_router_entry * Igmp_snoop_router_temp;
	igmp_router_entry ** Igmp_snoop_router_pre;

	igmp_snp_syslog_dbg("\nEnter Igmp_Event_VlanDelPort. \n");
	/*delete port from vlan_port_list */
	igmp_snp_vlan_port_delete(pPkt);

	/* delete port from user configured router port list */
	{
		igmp_routerport *pRouter = NULL;
		igmp_snp_searchrouterportlist( pPkt->vlan_id, pPkt->ifindex, IGMP_PORT_DEL, &pRouter );
	}

	/* delete port from user configured member port list */
	{
		igmp_reporter_port *pReporter = NULL;
		igmp_snp_searchreporterlist( pPkt->vlan_id, pPkt->ifindex, IGMP_PORT_DEL, &pReporter );
	}

	/* set 0 to ulGroup, because there just want to check the vlan_id existence, 
	but maybe we can get pMcGroup, because the mld multcast group's MC_ipadd =0*/
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_searchvlangroup( pPkt->vlan_id, 0, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_dbg("Igmp_Event_VlanDelPort: search failed\n");
		free( pPkt );
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	if ( NULL == pPrevGroupVlan )
	{
		igmp_snp_syslog_dbg(" Igmp_Event_VlanDelPort:No vlan node found.\n");
		free( pPkt );
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	if ( pPrevGroupVlan->vlan_id!= pPkt->vlan_id)
	{
		igmp_snp_syslog_dbg(" Igmp_Event_VlanDelPort:vlan:%d does not exist.\n", pPkt->vlan_id);
		free( pPkt );
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}
	/* delete vlan router port */
	/* if router port is deleted from vlan, delete it from port list */
	Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
	Igmp_snoop_router_pre = &( pPrevGroupVlan->routerlist );
	while ( Igmp_snoop_router_temp != NULL )
	{
		igmp_snp_syslog_dbg( " Igmp_Event_VlanDelPort:Del vlan router port.\n");
		if ( Igmp_snoop_router_temp->mroute_ifindex == pPkt->ifindex )
		{
			*Igmp_snoop_router_pre = Igmp_snoop_router_temp->next;
			free( Igmp_snoop_router_temp );
			Igmp_snoop_router_temp = NULL;
			break;
		}
		Igmp_snoop_router_pre = &( Igmp_snoop_router_temp->next );
		Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
	}
	
	pMcGroup = pPrevGroupVlan->firstgroup;
	while ( pMcGroup != NULL )
	{
		igmp_snp_syslog_dbg(" Igmp_Event_VlanDelPort:Del group member.\n");
		/* scan portlist */
		lRet = igmp_snp_searchportstatelist( &( pMcGroup->portstatelist ), pPkt->ifindex,
		                IGMP_PORT_DEL, &pstPort );
		if ( NULL == pMcGroup->portstatelist )
		{
			igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
		}

		if ( (IGMPSNP_RETURN_CODE_OK == lRet)&& (0 == pMcGroup->MC_ipadd) ){
			mldstPkt.vlan_id= pPkt->vlan_id;
			mldstPkt.type = MLD_ADDR_DEL;
			mldstPkt.ifindex = pPkt->ifindex;
			USHORT *pstIp6 = mldstPkt.groupadd;
			mld_copy_ipv6addr(pMcGroup->MC_v6ipadd, &(pstIp6));
			mldstPkt.group_id = pMcGroup->mgroup_id;

			if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_mod_addr( &mldstPkt , MLD_ADDR_DEL ) ) )
			{
				igmp_snp_syslog_dbg("Igmp_Event_VlanDelPort:mld failed in setting L2 MC table. Slot/port: 0x%x \n", pPkt->ifindex );
			}
		}		
		else if ( IGMPSNP_RETURN_CODE_OK == lRet ){
			stPkt.vlan_id= pPkt->vlan_id;
			stPkt.type = IGMP_ADDR_DEL;
			stPkt.ifindex = pPkt->ifindex;
			stPkt.groupadd= pMcGroup->MC_ipadd;
			stPkt.group_id = pMcGroup->mgroup_id;

			if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_mod_addr( &stPkt , IGMP_ADDR_DEL ) ) )
			{
				igmp_snp_syslog_dbg("Igmp_Event_VlanDelPort:igmp failed in setting L2 MC table. Slot/port: 0x%x \n", pPkt->ifindex );
			}
		}
		pMcGroup = pMcGroup->next;
	}

	free( pPkt );
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * Igmp_Event_AddRouterPort
 *
 * DESCRIPTION:
 *   		add router port.
 *
 * INPUTS:
 * 		vid  - vlan id
 *		port_index - include the port info
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   	- add success
 * 		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL	- not enalbe igmp snooping global
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST  -  can not find vlan
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
LONG Igmp_Event_AddRouterPort
(
	unsigned short vid,
	unsigned int port_index
)
{
	INT	ret =IGMPSNP_RETURN_CODE_OK;
	
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_warn("igmp snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	/*if((0 == vid )||(0 == port_index))*/ /*au5000 portindex begins form 0--(1/1).*/
	if(0 == vid)
	{
		igmp_snp_syslog_warn("vlan_id:%d ifindex:0x%x\n", vid, port_index);
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	#if 0 
	/*check vlan & port is enable IGMP or Not
		outside called fun:igmp_is_vlan_ifindex() 
		so, here not do it again -- wujh 08/09/24*/
	INT	existed = 0;
	igmp_vlan_list *t_vlanlist = NULL;
	igmp_vlan_list *find_vlanlist = NULL;
	igmp_vlan_port_list	*t_port = NULL;

	t_vlanlist = p_vlanlist;
	while(t_vlanlist )
	{
		if( t_vlanlist->vlan_id == vid)
		{
			find_vlanlist = t_vlanlist;
			IGMP_SNP_DEBUG("fing vlan %d from igmpVlan list.\n",find_vlanlist->vlan_id);
			break;
		}
		t_vlanlist = t_vlanlist->next;
	}
	if( find_vlanlist )	/*find vlan*/
	{
		if( find_vlanlist->first_port )
		{
			t_port = find_vlanlist->first_port;
			while(t_port)
			{
				if(t_port->ifindex == port_index )
				{
					existed = 1;
					break;
				}
				t_port = t_port->next;
			}
		}
		else
		{
			IGMP_SNP_DEBUG("igmp_add_mcroute_port:no any ports in this vlan %d\n",
				vid);
			return IGMP_SNOOP_ERR;
		}
	}
	else
	{
		IGMP_SNP_DEBUG("igmp_add_mcroute_port:vlan %d is not exist.\n",vid);
		return IGMP_SNOOP_ERR;
	}

	if( existed )/*vlan port both exist.*/
	#endif
	{
		ret = igmp_add_routeport(vid,port_index,0);
	}
	return ret;
}
/*******************************************************************************
 * Igmp_Event_DelRouterPort
 *
 * DESCRIPTION:
 *   		delete router port.
 *
 * INPUTS:
 * 		vid  - vlan id
 *		port_index - include the port info
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   	- add success
 * 		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL	- not enalbe igmp snooping global
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST  -  can not find vlan
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
LONG Igmp_Event_DelRouterPort
(
	unsigned short vid,
	unsigned int port_index
)
{
	INT	existed = 0;
	INT	ret =IGMPSNP_RETURN_CODE_OK;
	cmd_opt_info *opt = NULL;
	igmp_vlan_list *t_vlanlist = NULL;
	igmp_vlan_list *find_vlanlist = NULL;
	igmp_vlan_port_list	*t_port = NULL;
	
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_warn("igmp snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	/*if((0 == vid )||(0 == port_index))*/
	if(0 == vid ){
		igmp_snp_syslog_warn("vlan_id:%d ifindex:0x%x\n", vid, port_index);
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

#if 0
	/*	check vlan & port is enable IGMP or Not
		outside called fun:igmp_is_vlan_ifindex() 
		so, here not do it again
	*/

	t_vlanlist = p_vlanlist;
	while(t_vlanlist )
	{
		if( t_vlanlist->vlan_id == vid)
		{
			find_vlanlist = t_vlanlist;
			break;
		}
		t_vlanlist = t_vlanlist->next;
	}
	if( find_vlanlist )	/*find vlan*/
	{
		if( find_vlanlist->first_port )
		{
			t_port = find_vlanlist->first_port;
			while(t_port)
			{
				if(t_port->ifindex == port_index )
				{
					existed = 1;
					break;
				}
				t_port = t_port->next;
			}
		}
		else
		{
			igmp_snp_syslog_dbg("igmp_del_mcroute_port:no any ports in this vlan %d\n",
				vid);
			return IGMP_SNOOP_ERR;
		}
	}
	else
	{
		igmp_snp_syslog_dbg("igmp_del_mcroute_port:vlan %d is not exist.\n",vid);
		return IGMP_SNOOP_ERR;
	}

	if( existed )
	{
		ret = igmp_del_routeport(vid,port_index,0);
	}
#endif
	ret = igmp_del_routeport(vid, port_index, 0);

	return ret;
}
/*******************************************************************************
 * Igmp_Event_SysMac_Get
 *
 * DESCRIPTION:
 *   		add system mac to sysMac
 *
 * INPUTS:
 * 		macAddr  - the mac address
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   	- on success
 *
 * COMMENTS:
 *  
 **
 ********************************************************************************/
LONG Igmp_Event_SysMac_Get
(
	unsigned char macAddr[]
)
{
 int iter;
 for(iter =0;iter <MACADDRLENGTH; iter++){
	sysMac[iter] = macAddr[iter];
	igmp_snp_syslog_dbg("###sysMac[%d] = 0x%x.\n",iter,sysMac[iter]);
 }
	igmp_get_macaddr = IGMP_SNOOP_YES;
	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* igmp_snp_delv_mcgrouplan()
*
* DESCRIPTION:
*		igmp_snp_del_mcgroupvlan() delete a vlan node and its group nodes. 
*
* INPUTS:
* 		vlan_id - vlan id
*
* OUTPUTS:
*
* RETURES:
*		IGMPSNP_RETURN_CODE_OK  - delete vlan and group ok
*
* COMMENTS:
*
*
**************************************************************************/
LONG igmp_snp_del_mcgroupvlan( LONG vlan_id )
{
	MC_group * pMcGroup = NULL;
	MC_group *pNext = NULL;
	MC_group_vlan *pVlan = NULL;
	struct igmp_routerport	*Igmp_router_port_temp = NULL ,*Igmp_router_port_next = NULL;
	igmp_router_entry* Igmp_snoop_router_temp = NULL,*ppRouter =NULL;
	igmp_router_entry* Igmp_snoop_router_pre =NULL;
	static ULONG ulCount=0;
	INT	vlan_idx;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;

	igmp_snp_syslog_dbg("START:igmp_snp_del_mcgroupvlan\n");

	igmp_snp_syslog_dbg("vlanId %d.\n", vlan_id);

	/* get  */
	vlan_idx = first_mcgroupvlan_idx;
	//vlan_idx = 0;
	pVlan = GET_VLAN_POINT_BY_INDEX(vlan_idx);
	if(NULL != pVlan) {
		igmp_snp_syslog_dbg("Find mcGroupVlan->vlanId %d, mcGroupVlan->firstGroup@@%p.\n",\
							pVlan->vlan_id, pVlan->firstgroup);
	}
	
	while ( pVlan != NULL )
	{
		if ( vlan_id == pVlan->vlan_id )
		{
			/* delete McGroups of the pVlan */
			pMcGroup = pVlan->firstgroup;
			while ( pMcGroup != NULL )
			{
				ulCount++;
				/*if the group is mld group use mld_snp_delgroup  --yangxs*/
				if(0 == pMcGroup->MC_ipadd){	
					igmp_snp_syslog_dbg("Before delete group: pMcGroup->MC_ipadd x%x",
										"pMcGroup->MC_v6ipadd %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
										pMcGroup->MC_ipadd, ADDRSHOWV6(pMcGroup->MC_v6ipadd));
					mld_snp_delgroup( pVlan, pMcGroup, &pNext );
				}
				else{					
					igmp_snp_syslog_dbg("Before delete group: pMcGroup->MC_ipadd x%x.\n",\
										pMcGroup->MC_ipadd);
					igmp_snp_delgroup( pVlan, pMcGroup, &pNext );					
				}
				
				pMcGroup = pNext;
			}

			/*zgm add delete router entry*/
			if ( pVlan->routerlist != NULL )
			{
				Igmp_snoop_router_temp = pVlan->routerlist;
				while ( Igmp_snoop_router_temp != NULL )
				{
					Igmp_snoop_router_pre = Igmp_snoop_router_temp;
					Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
					free( Igmp_snoop_router_pre );
				}
				pVlan->routerlist = NULL;
			}

			/*delete router port list for this mcGroupVlan Node--wujh*/
/*			
			Igmp_router_port_temp = p_routerlist;
			if(NULL != Igmp_router_port_temp){
				
				while(NULL != Igmp_router_port_temp)
				{
					Igmp_router_port_next = Igmp_router_port_temp->next;
					if(pVlan->vlan_id == (Igmp_router_port_temp->vlan_id)){
						igmp_snp_syslog_dbg("delete vlan %d router port %d.\n",\
														Igmp_router_port_temp->vlan_id,\
														Igmp_router_port_temp->ifindex);
						lRet = igmp_snp_searchrouterportlist(Igmp_router_port_temp->vlan_id,\
														Igmp_router_port_temp->ifindex,\
														IGMP_PORT_DEL,\
														&ppRouter);
				
					}
					Igmp_router_port_temp = Igmp_router_port_next;
					igmp_snp_syslog_dbg("get next router port %p.\n", Igmp_router_port_temp);
				}
			}
*/
			/* delete general query timer */
			IGMP_SNP_DEL_INTERTIMER( pVlan->querytimer_id );

			if ( ( pVlan->next == -1 ) && ( pVlan->prev== -1 ) )
			{
				CLEAR_VLAN_POINT_BY_INDEX(vlan_idx);//vlan_idx has not be update.
				first_mcgroupvlan_idx = -1;
			}
			else
			{
				if ( pVlan->next != -1 )
				{
					GET_VLAN_POINT_BY_INDEX(pVlan->next)->prev = pVlan->prev;
				}
				if ( pVlan->prev != -1 )
				{
					GET_VLAN_POINT_BY_INDEX(pVlan->prev)->next = pVlan->next;
				}
				else
				{
					first_mcgroupvlan_idx = pVlan->next;
				}
			
				CLEAR_VLAN_POINT_BY_INDEX(vlan_idx);//vlan_idx has not be update.
			}
			free( pVlan );
			igmp_snp_syslog_dbg( "Vlan node %d deleted.\n", vlan_id );
			break;
		}
		else
		{
#if 0
			pVlan = GET_VLAN_POINT_BY_INDEX(pVlan->next);
			if(NULL != pVlan && pVlan->vlan_id == vlan_id){
				vlan_idx = pVlan->next; //wujh 08/09/08
				igmp_snp_syslog_dbg("get next pVlan.\n");
			}
			else {
				igmp_snp_syslog_dbg("get next pVlan NULL.\n");
				break;
			}
#endif
			if (pVlan->next != -1)
			{
				vlan_idx = pVlan->next;
			}

			pVlan = GET_VLAN_POINT_BY_INDEX(vlan_idx);
			if (NULL != pVlan)
			{
				igmp_snp_syslog_dbg("get next pVlan.\n");
			}
		}
	}

	igmp_snp_syslog_dbg("END:igmp_snp_del_mcgroupvlan\n");
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_snp_delgroup
*
* DESCRIPTION:
*		igmp_snp_delgroup() delete a group node. At first, the group
*		should be searched. When return, output the next group.
*
* INPUTS:
* 		pVlan  -  the pointer to point the vlan id
*		pGroup - the pointer to point the delete group
*
* OUTPUTS:
*		ppNextGroup - the pointer to point the delete group's next group
*
* RETURES:
*		IGMPSNP_RETURN_CODE_OK  - delete  group ok
*		IGMPSNP_RETURN_CODE_NULL_PTR - the group or vlan is null
*
* COMMENTS:
*
*
**************************************************************************/ 
LONG igmp_snp_delgroup( MC_group_vlan *pVlan, MC_group * pGroup, MC_group **ppNextGroup )
{
	/*ULONG ulTimerId = 0;*/
	igmp_snoop_pkt pkt ;
	struct MC_port_state *pstPort = NULL;
	struct MC_port_state *nextPort = NULL;

	igmp_snp_syslog_dbg("START:igmp_snp_delgroup\n");

	
	if ( NULL == pGroup || NULL == pVlan)
	{
		igmp_snp_syslog_err("parameter is null pointer!\n");
		igmp_snp_syslog_dbg("END:igmp_snp_delgroup\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	*ppNextGroup = pGroup->next;

	pkt.vlan_id = pGroup->vid;
	pkt.groupadd= pGroup->MC_ipadd;
	pkt.type = IGMP_ADDR_RMV;
	pkt.ifindex = 0;
	pkt.group_id = pGroup->mgroup_id; //by wujh
	
	igmp_snp_syslog_dbg("McGroupVlan %d,McGroupVidx %d,McGroupAddr 0x%x.\n",\
					pVlan->vlan_id,pGroup->mgroup_id,pGroup->MC_ipadd);
	/* remove group from hardware table*/
	if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_mod_addr((VOID*)&pkt,IGMP_ADDR_RMV)))
	{
		igmp_snp_syslog_err("failed in setting L2 MC table. 0x%x \n", pkt.ifindex);
	}

	/* delete ports of the group */
#if 0
	while ( NULL != pGroup->portstatelist )
	{
		pstPort = pGroup->portstatelist;
		pGroup->portstatelist = pstPort->next;
		free( pstPort );
	}
#endif
	pstPort = pGroup->portstatelist;
	nextPort = pstPort;
	while (NULL != pstPort)
	{
		nextPort = pstPort->next;
		free(pstPort);
		pstPort = NULL;
		pstPort = nextPort;
	}
	pGroup->portstatelist = NULL;
	pstPort = NULL;
	nextPort = NULL;

	/* delete group node  */
	if ( !( pGroup->next== NULL && pGroup->prev== NULL ) )
	{
		if ( pGroup->next != NULL && pGroup->prev != NULL)
		{
			pGroup->next->prev = pGroup->prev;
			pGroup->prev->next = pGroup->next;
			igmp_snp_syslog_dbg("delete pGroupNode middle one of list.\n");
		}
		else if ( pGroup->prev != NULL && pGroup->next == NULL)
		{
			pGroup->prev->next= pGroup->next;//NULL
			//pVlan->firstgroup = pGroup->prev;
			igmp_snp_syslog_dbg("delete pGroupNode new one of list.\n");
		}
		else
		{
			/*pGroup->prev==NULL */
			pVlan->firstgroup= pGroup->next;//useful?
			pGroup->next->prev = pGroup->prev;//NULL
			igmp_snp_syslog_dbg("delete pGroupNode oldest of list.\n");
		}
	}
	else
	{
		pVlan->firstgroup = NULL;
		/* don't delete vlan node, because we want to save the user configured router port,
			So just delete the general query timer */
		IGMP_SNP_DEL_INTERTIMER( pVlan->querytimer_id );
	}

	free( pGroup );

	/* Group count decrease */
	if ( igmp_groupcount > 0 )
	{
		igmp_groupcount--;
	}

	igmp_snp_syslog_dbg("END:igmp_snp_delgroup\n");
	return IGMPSNP_RETURN_CODE_OK;
}

INT igmp_snp_get_vlan_mbrs_igmpvlanlist(ULONG Vid,INT igmpVlanMbr[],INT *pCount)
{
	INT pFlag = 0,Conter = 0,ret = IGMPSNP_RETURN_CODE_OK;
	igmp_vlan_list* 	igmpVlanNode = NULL;
	igmp_vlan_port_list* igmpVlanPort = NULL;
	if(NULL != p_vlanlist)
	{
		igmpVlanNode = p_vlanlist;
		while( igmpVlanNode){
			if( Vid == igmpVlanNode->vlan_id ) {
				igmpVlanPort = igmpVlanNode->first_port;
				while(igmpVlanPort){
					pFlag = 1;
					igmpVlanMbr[Conter] = igmpVlanPort->ifindex;
					Conter ++;

					igmpVlanPort = igmpVlanPort->next;
				}
			}
			igmpVlanNode = igmpVlanNode->next;
		}
	}
	else {
		igmp_snp_syslog_dbg("There is no such vlan in igmp_snooping.");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	*pCount = Conter;
	if(pFlag){
		return IGMPSNP_RETURN_CODE_OK;
	}
	else
		return IGMPSNP_RETURN_CODE_ERROR;
}
/*******************************************************************************
 * igmp_snp_file_option_field
 *
 * DESCRIPTION:
 *	 igmp_snp_file_option_field() fill ip option field
 * INPUTS:
 * 	pOptionBuf - Ip option buffer
 *	ulLength - Ip option buffer length
 *	
 * OUTPUTS:
 *    	
 *
 * RETURNS: 
 *
 * COMMENTS:
 *       
 **
 ********************************************************************************/ 
VOID igmp_snp_file_option_field( UCHAR * pOptionBuf, ULONG ulLength )
{
	if ( ( !pOptionBuf )
	   || ( ulLength != IGMP_ROUTER_ALERT_LEN ) )
		return ;

	pOptionBuf[ 0 ] = 0x94;
	pOptionBuf[ 1 ] = 0x04;
	pOptionBuf[ 2 ] = 0x00;
	pOptionBuf[ 3 ] = 0x00;
}

/*******************************************************************************
 * Igmp_Snoop_Send_Packet
 *
 * DESCRIPTION:
 *   			Send Packet Data via socket
 *
 * INPUTS:
 * 			msg_skb - pointer to the data
 *			datalen  -  data length
 *			vlan_id  -  vlan id
 *			ifindex  -  interface index
 *
 * OUTPUTS:
 *    		
 *
 * RETURNS:
 *			IGMPSNP_RETURN_CODE_GET_SOCKET_E - No Socket Fd Get
 *
 * COMMENTS:
 *       
 **
 ********************************************************************************/

INT Igmp_Snoop_Send_Packet(struct igmp_skb *msg_skb,UINT datalen,LONG vlan_id,ULONG ifindex)
{		
	igmp_snp_syslog_dbg("Igmp_snoop_send_packet...\n");
	INT	rc,byteSend = 0;

	#if 0
	*((LONG *)msg_skb->buf ) = ifindex;
	*(( ULONG *)msg_skb->buf +1) = vlan_id;
	#endif
	msg_skb->ifindex = ifindex;	/*eth_g_index*/
	msg_skb->vlan_id = vlan_id;
	msg_skb->nlh.nlmsg_len = datalen;
	/***wujh**get the packet actual length**/
	struct iphdr *piphdr = NULL ;
	piphdr = ( struct iphdr* )(msg_skb->buf + sizeof(struct ether_header_t) );/*14-macHead Length;{8-length of(append vlan_id & ifindex)}*/
	INT pktLen = sizeof(struct nlmsghdr) + 8 + sizeof(struct ether_header_t) + piphdr->tot_len ; 

	/***************************************/
	#if 0
	if( 0 != kernel_fd ){
		write(kernel_fd,(char *)(msg_skb),datalen);
		
	}
	#endif
		   /*pktLen*/
	while(sizeof(struct igmp_skb)!= byteSend)
	{
		if(0 == kernel_fd){
			igmp_snp_syslog_dbg("Igmp_Snoop_Send_Packet::No Socket Fd Get!\n");
			return IGMPSNP_RETURN_CODE_GET_SOCKET_E;
		}
		igmp_snp_syslog_dbg("Send Packet Data via socket\n");
													/*pktLen*/
		rc = send(kernel_fd,(const void*)(msg_skb),sizeof(struct igmp_skb),0);
		/*rc = sendto(kernel_fd,(const void*)(msg_skb),pktLen,MSG_DONTWAIT,
							(struct sockaddr *)&pktRemoteAddr, sizeof(pktRemoteAddr));*/
		if(rc < 0)
		{
			if(errno == EINTR)/*send() be interrupted.*/
			{
				igmp_snp_syslog_err("send() be interrupted.");
				continue;
			}
			else if(errno == EACCES)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send() permission is denied on the destination socket file");
				break;
			}
			else if(errno == EWOULDBLOCK)/*send()*/
			{
				igmp_snp_syslog_err("send0");
				break;
			}
			else if(errno == EBADF)
			{
				igmp_snp_syslog_err("send1");
				break;
			}
			else if(errno == ECONNRESET)
			{
				igmp_snp_syslog_err("send2");
				break;
			}
			else if(errno == EDESTADDRREQ)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send3");
				break;
			}
			else if(errno == EFAULT)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send4");
				break;
			}
			else if(errno == EINVAL)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send5");
				break;
			}
			else if(errno == EISCONN)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send6");
				break;
			}
			else if(errno == EMSGSIZE)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send7");
				break;
			}
			else if(errno == ENOBUFS)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send8");
				break;
			}
			else if(errno == ENOMEM)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send9");
				break;
			}
			else if(errno == ENOTCONN)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("sendconn");
				break;
			}
			else if(errno == ENOTSOCK)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send10");
				break;
			}
			else if(errno == EOPNOTSUPP)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send11");
				break;
			}
			else if(errno == EPIPE)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send12");
				break;
			}
			else
			{
				igmp_snp_syslog_dbg("IGMP>>cpss Npd Cmd Write To Rstp Fail.\n");
				break;//return -1;/*send fail unknow error*/
			}
		}
		byteSend += rc;
	}
	igmp_snp_syslog_dbg("send :fd %d\n",kernel_fd);							
	igmp_snp_syslog_dbg("send :byteSend %d,originCmdbytes %d\n",byteSend,datalen);
	igmp_snp_syslog_dbg("############send Packet format###############\n");
	//igmp_debug_print_skb(msg_skb);
}


/*******************************************************************************
 * igmp_snp_routerleave
 *
 * DESCRIPTION:
 *		When a leave pkt is received and the rxmt timeout is expiration,
 *	 	send a leave to let the router del the member port.
 *
 * INPUTS:
 *		lVid  - vlan id
 *		usIfIndex  -  interface index
 *		ulGroup  -  gourp addr
 *		ulSaddr  -  source addr
 *
 * OUTPUTS:
 *		
 *
 * RETURNS:
 *
 * COMMENTS:
 *		
 **
 ********************************************************************************/
LONG igmp_snp_routerleave( LONG lVid, ULONG usIfIndex, ULONG ulGroup, ULONG ulSaddr )
{
	igmp_snoop_pkt stPkt;

	igmp_snp_syslog_dbg("send leave message to router: lVid %d, usIfIndex %d, ulGroup %u.%u.%u.%u\n",
	      				lVid, usIfIndex, NIPQUAD(ulGroup));	

	stPkt.vlan_id= lVid;
	stPkt.ifindex = usIfIndex;
	stPkt.groupadd= ulGroup;
	stPkt.type = IGMP_MSG_LEAVE;
	stPkt.saddr = ulSaddr;
	
	return Igmp_Snoop_Send_Igmp( &stPkt );
}
/*******************************************************************************
 * igmp_snp_routerreport
 *
 * DESCRIPTION:
 *		When a group is added or a router port is configured to a vlan,
 *		send a report ( or a report + leave ) to let the router add the 
 *		member port.
 *
 * INPUTS:
 *		lVid  - vlan id
 *		usIfIndex  - route port interface index
 *		ulGroup  -  gourp addr
 *		sk_info  -  packet information
 *
 * OUTPUTS:
 *		null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *		
 **
 ********************************************************************************/
LONG igmp_snp_routerreport( LONG lVid, ULONG usIfIndex, ULONG ulGroup, struct igmp_info *sk_info )
{
	igmp_snoop_pkt stPkt;
	struct iphdr *pIphd = NULL;
	
	igmp_snp_syslog_dbg("send report message to router: lVid %d, usIfIndex %d, ulGroup 0x%x\n",
	      				lVid, usIfIndex, ulGroup);
	
	pIphd = sk_info->ip_hdr;
	stPkt.vlan_id = lVid;
	stPkt.ifindex = usIfIndex;
	stPkt.groupadd= ulGroup;
	stPkt.type = IGMP_MSG_REPORT;
	stPkt.saddr = pIphd->saddr;
	
	return Igmp_Snoop_Send_Igmp( &stPkt );
}

/*******************************************************************************
 * igmp_snp_flood
 *
 * DESCRIPTION:
 *		igmp_snp_flood() flood Igmp packets in a specific vlan
 * 		what's the different between sk_info->vlan_id,sk_info->ifindex 
 *		and lVid,usIfIndex
 *
 * INPUTS:
 *		lVid  - vlan id
 *		usIfIndex  -  port's interface,this port get the query message
 *		sk_info  -  packege information
 *
 * OUTPUTS:
 
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_ERROR  -  can not get ifindex by vlan_id
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL  - failed in alloc memory for pBuf
 *		IGMPSNP_RETURN_CODE_ERROR  -  flood success
 *
 * COMMENTS:
 *		
 **
 ********************************************************************************/
LONG igmp_snp_flood( struct igmp_info *sk_info, LONG lVid, ULONG usIfIndex )
{
	LONG unIfIndex = 0;/*ULONG --> LONG: lead to fatal loop! 08/08/23@wujh*/
	UCHAR *pBuf = NULL;
	LONG lBufSize;
	ULONG ulportstatus;
	ULONG nextifindex;
	igmp_queryport *PTemp = NULL;
	
	lBufSize = sizeof(struct igmp_skb);
	pBuf = ( UCHAR * )malloc( lBufSize);
	if ( NULL == pBuf )
	{
		igmp_snp_syslog_err( "failed in alloc memory for pBuf, when igmp-snooping flood.\n" );
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset( pBuf,0, lBufSize );
	memcpy( pBuf, ( UCHAR* )( sk_info->data), lBufSize );/*GET igmp_skb form igmp_info*/
	igmp_snp_syslog_dbg("data:pBuf = 0x%x, lBufSize = %d,usIfIndex %d.\n", \
						pBuf, lBufSize, usIfIndex);

	//IGMP_SNP_DEBUG(":::::::::::::igmp_snp_flood:::::::::::::::::::\n");
	//igmp_debug_print_skb(sk_info->data);
	if(IGMPSNP_RETURN_CODE_OK != igmp_getifindex_byvlanid(lVid,&unIfIndex) )
	{
		free(pBuf);
		pBuf = NULL;
		igmp_snp_syslog_dbg("can not get ifindex by vlan_id.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	igmp_snp_syslog_dbg("find first ifindex %d by vlan_id %d.\n",unIfIndex,lVid);

	#if 0
	/*****************/
	INT i,vlanMbrCnt = 0,igmpvlanMbr[24] = {0};
	igmp_snp_get_vlan_mbrs_igmpvlanlist(lVid,igmpvlanMbr,&vlanMbrCnt);
	for(i =0;i < vlanMbrCnt; i++){
		if( usIfIndex!=igmpvlanMbr[i]){
			Igmp_Snoop_Send_Packet((struct igmp_skb *)pBuf, lBufSize, lVid, igmpvlanMbr[i]);
		}
	}
	#endif 
	while( !(0 > unIfIndex) )
	{
		if ( unIfIndex == usIfIndex )
		{
			igmp_snp_syslog_dbg("find a same IfIndex,continue\n");
			if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(lVid,unIfIndex,&nextifindex))
				{unIfIndex = nextifindex;}
			else unIfIndex = -1;
			continue;
		}

		if ( p_queryportlist != NULL )
		{
			PTemp = p_queryportlist;
			while ( PTemp != NULL )
			{
				if ( PTemp->ifindex == unIfIndex )
					break;
				PTemp = PTemp->next;
			}

			if ( PTemp != NULL )
			{
				igmp_getifstatus( unIfIndex, &ulportstatus );
				if ( ulportstatus == 1 )	/*UP*/
					Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, lBufSize, lVid, unIfIndex );
			}
		}
		else
		{
			igmp_getifstatus( unIfIndex, &ulportstatus );
			if ( ulportstatus == 1 )		/*UP*/
				Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, lBufSize, lVid, unIfIndex );
		}
		if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(lVid,unIfIndex,&nextifindex))
			{
				unIfIndex = nextifindex;
				}
			else 
				unIfIndex = -1;
		igmp_snp_syslog_dbg("get next ifindex %d.\n",unIfIndex);
	}

	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * Igmp_Snoop_Send_Igmp
 *
 * DESCRIPTION:
 *		Igmp_Snoop_Send_Igmp is send Igmp packet   			
 *
 * INPUTS:
 * 		pPkt  - pointer to igmp  paket
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   -  on success
 * 		IGMPSNP_RETURN_CODE_ERROR	- get ifindex targged failed
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL   - alloc mem for pBuf failed
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
LONG Igmp_Snoop_Send_Igmp( igmp_snoop_pkt * pPkt )
{
	UINT	i, ethlen;
	ULONG taged = 0;
	struct ether_header_t *pEthhd = NULL;
	struct iphdr * pIphd = NULL;
	struct igmp *pIgmp = NULL;
	ULONG unVlanIf;
	LONG lRet;
	LONG lVid = 0,unIfIndex = 0;	/*ULONG --> LONG: lead to fatal loop! 08/08/23@wujh*/
	UCHAR *pBuf = NULL;		/* IP PDU */
	ULONG ulBufLen = 0;
	ULONG ulIpAddr, ulIpMask;
	ULONG ulportstatus;
	ULONG nextifindex;
	igmp_queryport *PTemp = NULL;
	
	igmp_snp_syslog_dbg( "\nEnter Igmp_Snoop_Send_Igmp: vlan %d, group 0x%x, type %d, If 0x%x\n",
								pPkt->vlan_id, pPkt->groupadd, pPkt->type, pPkt->ifindex);

	unIfIndex = pPkt->ifindex;
	lVid = pPkt->vlan_id;
	/*for igmp report/leave packet , need add ip option: router alert*/

	/******************/
	/* Set ulBufLen's value */
	/******************/
	switch ( pPkt->type )
	{
		case IGMP_MSG_GEN_QUERY:
		case IGMP_MSG_GS_QUERY:
		case IGMP_MSG_V1_REPORT:
			ulBufLen = sizeof( struct iphdr ) + sizeof( struct igmp );
			break;
		case IGMP_MSG_REPORT:
		case IGMP_MSG_LEAVE:
			ulBufLen = sizeof( struct iphdr ) + sizeof( struct igmp ) + IGMP_ROUTER_ALERT_LEN;
			break;
		default:
			/*It seems that igmp v3 report need add */
			ulBufLen = sizeof( struct iphdr ) + sizeof( struct igmp ) + IGMP_ROUTER_ALERT_LEN;
			break;
	}

	/*****************/
	/* Set ethlen's value  */
	/*****************/
	if(IGMPSNP_RETURN_CODE_OK != igmp_vlanportrelation(pPkt->ifindex,pPkt->vlan_id,&taged))
	{
		igmp_snp_syslog_dbg("Igmp_Snoop_Send_Igmp:get ifindex targged failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	if( taged )		/*targged*/
		ethlen = 18;
	else
		ethlen = 14;
	
	pBuf = ( UCHAR * )malloc( ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr));
	if ( NULL == pBuf )
	{
		igmp_snp_syslog_dbg("Igmp_Snoop_Send_Igmp: alloc mem for pBuf failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset( pBuf,0, (ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr)) );

	pEthhd = (struct ether_header_t* )(pBuf +8 +sizeof(struct nlmsghdr));

	pIphd = ( struct iphdr * ) (pBuf + 8 + ethlen + sizeof(struct nlmsghdr));
	/*for igmp report/leave packet , need add ip option: router alert*/

	/*****************/
	/* Set pIgmp's value  */
	/*****************/
	switch ( pPkt->type )
	{
		case IGMP_MSG_GEN_QUERY:
		case IGMP_MSG_GS_QUERY:
		case IGMP_MSG_V1_REPORT:
			pIgmp = ( struct igmp * ) ( pIphd + 1 );
			break;
		case IGMP_MSG_REPORT:
		case IGMP_MSG_LEAVE:
			igmp_snp_file_option_field( ( UCHAR * ) ( pIphd + 1 ), IGMP_ROUTER_ALERT_LEN );
			pIgmp = ( struct igmp * ) ( ( UCHAR * ) ( pIphd + 1 ) + IGMP_ROUTER_ALERT_LEN );
			break;
		default:
			/*It seems that igmp v3 report need add */
			igmp_snp_file_option_field( ( UCHAR * ) ( pIphd + 1 ), IGMP_ROUTER_ALERT_LEN );
			pIgmp = ( struct igmp * ) ( ( UCHAR * ) ( pIphd + 1 ) + IGMP_ROUTER_ALERT_LEN );
			break;
	}
	/* generate IGMP PDU */
	pIgmp->igmp_code = 0;
	pIgmp->igmp_group.s_addr = pPkt->groupadd;

	switch ( pPkt->type )
	{
		case IGMP_MSG_GEN_QUERY:
			/* If a general query, send to all port except the router port in the pkt->ifindex */
			pIgmp->igmp_code = igmp_resp_interval * IGMP_V2_SEC_2_MILL;
			pIgmp->igmp_group.s_addr = 0;  /* general query */
			pIgmp->igmp_type = IGMP_MEMSHIP_QUERY;
			pIphd->daddr = htonl( IGMP_ALL_SYSTEM_ADDR );
			break;
		case IGMP_MSG_GS_QUERY:
			/* If a g-s query, send to the specific port */
			pIgmp->igmp_code = igmp_rxmt_interval / IGMP_V2_TIME_SCALE;
			pIgmp->igmp_type = IGMP_MEMSHIP_QUERY;
			pIphd->daddr = htonl( pPkt->groupadd );
			break;
		case IGMP_MSG_REPORT:
			/* If a report, send to the router port */
			pIgmp->igmp_type = IGMP_V2_MEMSHIP_REPORT;
			pIphd->daddr = htonl( pPkt->groupadd);
			break;
		case IGMP_MSG_V1_REPORT:
			/* If a V1 report, send to the router port */
			pIgmp->igmp_type = IGMP_V1_MEMSHIP_REPORT;
			pIphd->daddr = htonl( pPkt->groupadd );
			break;
		case IGMP_MSG_LEAVE:
			pIgmp->igmp_type = IGMP_V2_LEAVE_GROUP;
			pIphd->daddr = htonl( IGMP_ALL_ROUTER_ADDR );
			break;
		default:
			break;
	}
	pIgmp->igmp_cksum = 0;
	pIgmp->igmp_cksum = inet_cksum( ( USHORT * ) pIgmp, sizeof( struct igmp ) );

	/* generate IP header */
	pIphd->ttl = IGMP_TTL; /* IGMP packet ttl = 1 */
	/*pIphd->ihl = sizeof( struct iphdr ) >> 2; */ /* numbers of 32-bit block*/
	pIphd->protocol = IPPROTO_IGMP;
	pIphd->tos = IGMP_TOS;
	pIphd->version = IPVERSION;
	/*pIphd->tot_len = sizeof( struct iphdr ) + sizeof( struct igmp );*/
	pIphd->id = 1;
	pIphd->frag_off = 0;
	pIphd->check = 0;

	/*for igmp report/leave packet , need add ip option: router alert*/
	switch ( pPkt->type )
	{
		case IGMP_MSG_GEN_QUERY:
		case IGMP_MSG_GS_QUERY:
		case IGMP_MSG_V1_REPORT:
			pIphd->ihl = sizeof( struct iphdr ) >> 2;
			pIphd->tot_len = sizeof( struct iphdr ) + sizeof( struct igmp );
			break;
		case IGMP_MSG_REPORT:
		case IGMP_MSG_LEAVE:
			pIphd->ihl = ( sizeof( struct iphdr ) + IGMP_ROUTER_ALERT_LEN ) >> 2;
			pIphd->tot_len = sizeof( struct iphdr ) + sizeof( struct igmp ) + IGMP_ROUTER_ALERT_LEN;
			break;
		default:
			/*It seems that igmp v3 report need add */
			pIphd->ihl = ( sizeof( struct iphdr ) + IGMP_ROUTER_ALERT_LEN ) >> 2;
			pIphd->tot_len = sizeof( struct iphdr ) + sizeof( struct igmp ) + IGMP_ROUTER_ALERT_LEN;
			break;
	}

	/********/	
	ulIpAddr = pIphd->daddr;
	pEthhd->dmac[0] = 0x01;
	pEthhd->dmac[1] = 0x00;
	pEthhd->dmac[2] = 0x5e;
	pEthhd->dmac[3] = (unsigned char)((ulIpAddr>>16) & 0x7F);
	pEthhd->dmac[4] = (unsigned char)((ulIpAddr>>8) & 0xFF);
	pEthhd->dmac[5] = (unsigned char)(ulIpAddr & 0xFF);

	if(!igmp_get_macaddr){
		/**/
		pEthhd->smac[0] = 0x00;
		pEthhd->smac[1] = 0x1f;
		pEthhd->smac[2] = 0x64;
		pEthhd->smac[3] = 0x00;
		pEthhd->smac[4] = 0x00;
		pEthhd->smac[5] = 0x01;
	}
	else {
		memcpy(pEthhd->smac,sysMac,MACADDRLENGTH);
	}

	pEthhd->etherType = 0x0800;

	/* Get vlan's IP address. If no ip addr, use 0.0.0.0 as SIP */
#if 0
	SYS_CREATE_VLAN_IF( unVlanIf, pPkt->lVid );
	lRet = IFM_Get3LayerIpAddr( unVlanIf, &ulIpAddr, &ulIpMask, 0 );
#endif
	lRet = igmp_getvlan_addr(pPkt->vlan_id,&ulIpAddr);
	if ( lRet != IGMPSNP_RETURN_CODE_OK )
	{
		pIphd->saddr = pPkt->saddr;
		igmp_snp_syslog_dbg("Igmp_Snoop_Send_Igmp:: pPkt->saddr 0x%x <--> pIphd->saddr 0x%x.\n",pPkt->saddr,pIphd->saddr);
	}
	else
	{
		pIphd->saddr = htonl( ulIpAddr );
	}
	pIphd->check = inet_cksum( ( USHORT * ) pIphd, ( ULONG ) ( pIphd->ihl ) << 2 );

	/* If it is the general query, should send packet to vlan except the router port
	If the other types, send the packet directly to unIfIndex.
	*/
	if ( pPkt->type == IGMP_MSG_GEN_QUERY )/* general query, send pkt to all port except the unIfIndex port */
	{
		igmp_snp_syslog_dbg("Igmp_Snoop_Send_Igmp::Send General query.\n");

		if(IGMPSNP_RETURN_CODE_OK !=igmp_getifindex_byvlanid(pPkt->vlan_id,&unIfIndex) )
		{
			igmp_snp_syslog_dbg("igmp_snp_flood: can not get ifindex by vlan_id.\n");
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		//IFM_PORTONVLANSTART( unIfIndex, ( USHORT ) ( pPkt->lVid ) )
		while( !(0 > unIfIndex))
		{

			if ( unIfIndex == pPkt->ifindex )/* route port index(mroute_ifindex) ==pPkt->ifindex */
			{	/*send packet to vlan except the router port,get next ifindex. */
				if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(lVid,unIfIndex,&nextifindex))
				{
				 unIfIndex = nextifindex;
				  }else unIfIndex = -1;
				continue;
			}
			
			igmp_getifstatus( unIfIndex, &ulportstatus );
			if ( ulportstatus == 1 )	/*call socket to send packet*/
				Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, 
								ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr), 
								pPkt->vlan_id, unIfIndex );
			if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(pPkt->vlan_id,unIfIndex,&nextifindex))
									{
									  unIfIndex = nextifindex;
				  					}else unIfIndex = -1;
		} /*End of "If it is the general query, should send packet to vlan except the router port" */
		//IFM_PORTONVLANEND;
	}
	/* igmp_snp_syslog_dbg("Send to If 0x%x\n",unIfIndex.usIfIndex); */
	else if ( pPkt->type == IGMP_MSG_GS_QUERY )
	{
		if ( p_queryportlist != NULL )
		{
			PTemp = p_queryportlist;
			while ( PTemp != NULL )
			{
				if ( PTemp->ifindex == unIfIndex )
					break;
				PTemp = PTemp->next;
			}
			if ( PTemp != NULL )
			{
				igmp_getifstatus( unIfIndex, &ulportstatus );
				if ( ulportstatus == 1 )
				{
					Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, 
									ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr),
									pPkt->vlan_id, unIfIndex );
					igmp_snp_syslog_dbg("Configure:Send to If 0x%x\n", unIfIndex);
				}
			}
		}
		else
		{
			igmp_getifstatus( unIfIndex, &ulportstatus );
			if ( ulportstatus == 1 )
			{
				Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, 
									ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr), 
									pPkt->vlan_id, unIfIndex );
				igmp_snp_syslog_dbg( "Send  to If 0x%x\n", unIfIndex);
			}
		}
	}
	else
	{
		igmp_getifstatus( unIfIndex, &ulportstatus );
		if ( ulportstatus == 1 )
			Igmp_Snoop_Send_Packet( (struct igmp_skb *)pBuf, 
									ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr), 
									pPkt->vlan_id, pPkt->ifindex );
	}
	free( pBuf );
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_snp_addintertimer()
*
* Input: ulSec -  the time value in seconds
*        ulFlag - Timer flag: HOS_TIMER_LOOP, HOS_TIMER_NO_LOOP etc.
*
* Output: 
*		  if returns ok, it is the timer id
*
* RETURNS:
*		IGMPSNP_RETURN_CODE_OK - add is success
*
* DESCRIPTION:
*		igmp_snp_addintertimer() add a local timer.
*
**************************************************************************/
LONG igmp_snp_addintertimer( ULONG ulSec, ULONG * pulTimerId )
{
	*pulTimerId = ulSec;
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* Igmp_Event_GenQuery_Timeout()
*
* INPUTS:
*		pPkt - igmp_snoop_pkt structure pointer 	
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_NULL_PTR - parameter null pointer
*
* DESCRIPTION:
*		Igmp_Event_GenQuery_Timeout handles the general query interval timeout
*		message. Every vlan has a general query interval timer. When the timer
*		expired, general query message should be sent to this vlan.
*		
*		Notice: Because some of the normal host doesn't reply general query, so we should
*		send g-s query here instead.
* CALLED BY:
*		Igmp_TimerExp_Proc()
*		
**************************************************************************/
LONG Igmp_Event_GenQuery_Timeout( MC_group_vlan *p_vlan )
{
	MC_group_vlan *pPrevGroupVlan = NULL;
	igmp_snoop_pkt stData;
	struct igmp_router_entry * Igmp_snoop_router_temp;  /*zgm add router port */
	time_t timep ;
	memset(&timep,0,sizeof(time_t));
	char *timebuf = NULL;
	
	timebuf = ctime( &timep);
	igmp_snp_syslog_dbg( ( "\nSystem corrent time: %s. \n",timebuf ) );
	if( !p_vlan )
	{
		igmp_snp_syslog_dbg("GenQuery Timeout,parameter null pointer.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	pPrevGroupVlan = p_vlan;
	/* send general query to all ports except the router port */
	Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
	if ( Igmp_snoop_router_temp != NULL )
	{
		while ( Igmp_snoop_router_temp != NULL )
		{

			/*send general skb*/
			stData.vlan_id = p_vlan->vlan_id;
			stData.groupadd= 0;
			stData.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
			stData.saddr = Igmp_snoop_router_temp->saddr;
			stData.type = IGMP_MSG_GEN_QUERY;

			if ( IGMPSNP_RETURN_CODE_OK != ( Igmp_Snoop_Send_Igmp( &stData ) ) )
			{
				igmp_snp_syslog_dbg( ( "GenQuery Timeout, failed in sending query packet\n" ) );
			}

			Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
		}
	}
	else
	{
		/*send general skb*/
		stData.vlan_id= p_vlan->vlan_id;
		stData.groupadd= 0;
		stData.ifindex = 0xffff;/*non-exist port in system*/
		stData.saddr = 0;
		stData.type = IGMP_MSG_GEN_QUERY;

		if ( IGMPSNP_RETURN_CODE_OK != ( Igmp_Snoop_Send_Igmp( &stData ) ) )
		{
			igmp_snp_syslog_dbg( ( "GenQuery Timeout, failed in sending query packet\n" ) );
		} 
	}
	return IGMPSNP_RETURN_CODE_OK;
}

#if 0
/**************************************************************************
* Igmp_Event_GenQuery_Timeout()
*
* INPUTS:
*		pPkt - igmp_snoop_pkt structure pointer 	
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMP_SNOOP_OK -  on sucess
*		IGMP_SNOOP_ERR - not found
*
* DESCRIPTION:
*		Igmp_Event_GenQuery_Timeout handles the general query interval timeout
*		message. Every vlan has a general query interval timer. When the timer
*		expired, general query message should be sent to this vlan.
*		
*		Notice: Because some of the normal host doesn't reply general query, so we should
*		send g-s query here instead.
* CALLED BY:
*		Igmp_TimerExp_Proc()
* CALLS:
*		
**************************************************************************/
LONG Igmp_Event_GenQuery_Timeout( MC_group_vlan *p_vlan )
{
	MC_group_vlan *pPrevGroupVlan = NULL;
	igmp_snoop_pkt stData;
	struct igmp_routerport	*tmpRoutePort = NULL;
	struct igmp_router_entry *Igmp_snoop_router_temp = NULL;  /*zgm add router port */

	IGMP_SNP_DEBUG("\nEnter Igmp_Event_GenQuery_Timeout. \n");

	if( !p_vlan )
	{
		IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: parameter error.\n");
		return IGMP_SNOOP_ERR;
	}

	pPrevGroupVlan = p_vlan;
	/* send general query to all ports except the router port */
	#if 0
	Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
	if ( Igmp_snoop_router_temp != NULL )
	{
		while ( Igmp_snoop_router_temp != NULL )
		{

			/*send general skb*/
			stData.vlan_id = p_vlan->vlan_id;
			stData.groupadd= 0;
			stData.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
			stData.saddr = Igmp_snoop_router_temp->saddr;
			stData.type = IGMP_MSG_GEN_QUERY;
			IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: vlan_id %d,ifIndex %d,saddr 0x%x.\n",
							stData.vlan_id,stData.ifindex,stData.saddr);
								/*Igmp_Snoop_Send_Igmp :Don't send Pkt from the port self*/
			if ( IGMP_SNOOP_OK != ( Igmp_Snoop_Send_Igmp( &stData ) ) )
			{
				IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: failed in sending query packet\n");
			}

			Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
		}
	}
	#endif
	
	tmpRoutePort = p_routerlist;
	if(NULL != tmpRoutePort){
		while (NULL !=tmpRoutePort){
			if(pPrevGroupVlan->vlan_id == tmpRoutePort->vlan_id){
				/*send general skb*/
				stData.vlan_id = p_vlan->vlan_id;
				stData.groupadd= 0;
				stData.ifindex = tmpRoutePort->ifindex;
				stData.saddr = tmpRoutePort->routeport_saddr;
				stData.type = IGMP_MSG_GEN_QUERY;
				IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: vlan_id %d,ifIndex %d,saddr 0x%x.\n",
								stData.vlan_id,stData.ifindex,stData.saddr);
									/*Igmp_Snoop_Send_Igmp :Don't send Pkt from the port self*/
				if ( IGMP_SNOOP_OK != ( Igmp_Snoop_Send_Igmp( &stData ) ) )
				{
					IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: failed in sending query packet\n");
				}
			}
			tmpRoutePort = tmpRoutePort->next;
		}
	}
	else
	{
		IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout:: can NOT find routerEntry in pPrevGroupVlan->vid %d.\n",\
										pPrevGroupVlan->vlan_id);
		INT i,vlanMbrCnt = 0,igmpvlanMbr[24] = {0};
		/*send general skb*/
		stData.vlan_id= p_vlan->vlan_id;
		if(IGMP_SNOOP_OK == igmp_snp_get_vlan_mbrs_igmpvlanlist(p_vlan->vlan_id,igmpvlanMbr,&vlanMbrCnt))
		{
			for(i=0;i<vlanMbrCnt;i++){
				stData.groupadd= 0;
				stData.ifindex = igmpvlanMbr[i];
				stData.saddr = 0;
				stData.type = IGMP_MSG_GEN_QUERY;

				if ( IGMP_SNOOP_OK != (Igmp_Snoop_Send_Igmp( &stData) ) )
				{
					IGMP_SNP_DEBUG("Igmp_Event_GenQuery_Timeout: failed in sending query packet\n");
				} 
			}
		}
	}
	return IGMP_SNOOP_OK;
}
#endif
/**************************************************************************
* Igmp_Event_GroupLife_Timeout()
*
* INPUTS:
*		p_vlan - pointer to vlan
*		p_group - pointer to group
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_NULL_PTR - group or vlan not exist
*
* DESCRIPTION:
*		Igmp_Event_GroupLife_Timeout handles the group life timeout message.
*		A group haven't received query message for a long time, it will be 
*		taken to thought that it was dead. So delete the group.
*		
*  
*
**************************************************************************/
LONG Igmp_Event_GroupLife_Timeout( MC_group_vlan *p_vlan, MC_group *p_group )
{
	MC_group * pNextGroup = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;

	igmp_snp_syslog_dbg("START:Igmp_Event_GroupLife_Timeout\n");

	if ( !(p_group) || !(p_vlan) )
	{
		igmp_snp_syslog_err("parameter NULL pointer.\n");
		igmp_snp_syslog_dbg("END:Igmp_Event_GroupLife_Timeout\n");	
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	/* delete group node */
	lRet = igmp_snp_delgroup( p_vlan, p_group, &pNextGroup );
	if( lRet != IGMPSNP_RETURN_CODE_OK)
	{
		igmp_snp_syslog_dbg("END:Igmp_Event_GroupLife_Timeout\n");
		return lRet;
	}
	else if( lRet == IGMPSNP_RETURN_CODE_OK ) {
		p_group = NULL;		/*release success, set p_group = NULL*/
	}

	/* if the group is the last group in the vlan, delete vlan node */
	if ( NULL == p_vlan->firstgroup)
	{
		igmp_snp_syslog_dbg("p_vlan %d@@%p no multicast group exists. delete this p_vlan\n",\
						p_vlan->vlan_id,p_vlan);
		if( IGMPSNP_RETURN_CODE_OK == igmp_snp_del_mcgroupvlan( p_vlan->vlan_id)){
			p_vlan = NULL;
		}
	}
	
	igmp_snp_syslog_dbg("END:Igmp_Event_GroupLife_Timeout\n");
	return lRet;
}
/**************************************************************************
* Igmp_Event_Proxy_Timeout
*
* INPUTS:
*		p_vlan - pointer to vlan
*		p_group - pointer to group
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_NULL_PTR - group or vlan not exist
*
* DESCRIPTION:
*		Igmp Event Proxy Timeout handles the group life timeout message.
*		A group haven't received query message for a long time, it will be 
*		taken to thought that it was dead. So delete the group.
*		
*  
*
**************************************************************************/

LONG Igmp_Event_Proxy_Timeout(MC_group_vlan *p_vlan, MC_group * p_group)
{
	/*ULONG unIfIndex;*/
	igmp_router_entry * Igmp_snoop_router_temp;
	igmp_snoop_pkt stPkt;
	
	if ( NULL == p_vlan || NULL == p_group)
	{
		igmp_snp_syslog_dbg("Igmp_Event_Proxy_Timeout: parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	if ( NULL != p_group->portstatelist )		/* has member, send report */
	{
		if ( NULL != p_vlan->routerlist )
		{
			Igmp_snoop_router_temp = p_vlan->routerlist;
			while ( Igmp_snoop_router_temp != NULL )
			{
				/*send report packet to each router port(not each port)!could create skb directly.*/
				stPkt.vlan_id= p_group->vid;
				stPkt.groupadd= p_group->MC_ipadd;

				stPkt.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
				stPkt.type = ( p_group->ver_flag == IGMP_SNOOP_NO ) ? IGMP_MSG_REPORT : IGMP_MSG_V1_REPORT ;
				stPkt.saddr = p_group->report_ipadd;

				igmp_snp_syslog_dbg( "Igmp_Event_Proxy_Timeout: send report. 0x%8x %d\n",
						p_group->MC_ipadd, stPkt.ifindex );

				if ( IGMPSNP_RETURN_CODE_OK != Igmp_Snoop_Send_Igmp( &stPkt ) )
				{
					igmp_snp_syslog_dbg("Igmp_Event_Proxy_Timeout: send report failed.\n");
					/* if failed in send report, doesn't return, and go on */
				}
				Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
			}
		}
		/*
		else {//else breach for Debug,should delete.
			IGMP_SNP_DEBUG("No Route port in vlan %d !\n",p_vlan->vlan_id);
		}
		*/
	}
	else
	{
		igmp_snp_syslog_dbg("\nIgmp_Event_Proxy_Timeout:: No Member port in Group 0x%x !\n",p_group->MC_ipadd);
		/*delete timer in here is not right but simple */
		p_group->router_reporttimer = 0;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/**************************************************************************
* Igmp_Event_GroupMember_Timeout
*
* INPUTS:
*		p_vlan - pointer to vlan
*		p_group - pointer to group
* 		ifindex - interface index
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK -  on sucess
*		IGMPSNP_RETURN_CODE_NULL_PTR - group or vlan not exist
*
* DESCRIPTION:
*		Igmp_Event_GroupMember_Timeout handles the IGMP group member timeout
*		message. A group haven't received membership message for the specific time,
*		it will be thought that the port has no member now. So change the port
*		state to NoMember state( 00 ).
*		If the member is the last member of this group, we should start the group
*		life timer. If no report received before the group life timer expired,
*		The group should be deleted.
*  
*
**************************************************************************/
LONG Igmp_Event_GroupMember_Timeout( MC_group_vlan *p_vlan,MC_group *p_group,
							ULONG ifindex  )
{ 
	struct MC_port_state *t_port = NULL;
	igmp_snoop_pkt 	stPkt;

	igmp_snp_syslog_dbg( " igmp_Event_GroupMember_Timeout handle.\n") ;
	
	if ( !p_group ||!p_vlan)
	{
		igmp_snp_syslog_dbg( "parameter null pointer.\n") ;
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	/* if the group membership timer is timeout, delete the port from member list
	*/
	igmp_snp_searchportstatelist( &( p_group->portstatelist ), ifindex,
						IGMP_PORT_DEL, &t_port );

	/* del L2 L3 MC address */
	stPkt.vlan_id= p_vlan->vlan_id;
	stPkt.ifindex = ifindex;
	stPkt.type = IGMP_ADDR_DEL;
	stPkt.groupadd = p_group->MC_ipadd;
	stPkt.group_id = p_group->mgroup_id; 
	if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_mod_addr (&stPkt,IGMP_ADDR_DEL) ) )
	{
		igmp_snp_syslog_err("failed in setting L2 MC table. 0x%x \n", stPkt.ifindex);
	}

	/* if no member present, start group life timer */
	if ( NULL == p_group->portstatelist )	
	{
		/* Add group life timer */
		igmp_snp_addintertimer( igmp_grouplife, &( p_group->grouplifetimer_id ) );
		igmp_snp_syslog_dbg("add group life timer.%d\n", \
														p_group->grouplifetimer_id);
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/**************************************************************************
* Igmp_Event_Resp_Timeout
*
* INPUTS:
*		cur - pointer to timer element
*
* OUTPUTS:
*
* RETURN VALUE:
*
*
* DESCRIPTION:
*		Igmp_Event_Resp_Timeout handles the response timeout message.When the 
*		Igmp Snooping switch receives a query packet, it will start the response
*		timer to delay the report packet sending process. So while the reponse 
*		timeout send the report message.

*  
*
**************************************************************************/
VOID Igmp_Event_Resp_Timeout( timer_element *cur)
{
	igmp_snoop_pkt * pPkt = NULL;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	ULONG unIfIndex;
	ULONG unRouterIfIndex;
	igmp_snoop_pkt stPkt;
	MC_group_vlan *pMcVlan = NULL;
	igmp_router_entry * Igmp_snoop_router_temp;

	//unRouterIfIndex = pPkt->ifindex;/*???,move to line:2801*/

	igmp_snp_syslog_dbg("START:Igmp_Event_Resp_Timeout,timer Id =%d \n",cur->id);

	if( NULL == cur )
	{
		igmp_snp_syslog_dbg("cur is null\n");
		igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
		return;
	}
	pPkt = (igmp_snoop_pkt *)cur->data;
	if ( (NULL == pPkt)||(cur->datalen < sizeof(igmp_snoop_pkt)) )
	{
		igmp_snp_syslog_dbg("pPkt is null\n");
		igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
		return;
	}

	unRouterIfIndex = pPkt->ifindex; /*move to here*/
	/* search McGroupvlan */
	pMcVlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
	//pMcVlan = GET_VLAN_POINT_BY_INDEX(0);
	while ( pMcVlan != NULL )
	{
		if ( pMcVlan->vlan_id == pPkt->vlan_id )
		{
			break;
		}
		else
		{
			pMcVlan = GET_VLAN_POINT_BY_INDEX(pMcVlan->next);
		}
	}

	if ( pMcVlan ==NULL)
	{
		igmp_snp_syslog_dbg("No such mcgroupvlan node %d.\n", pPkt->vlan_id);
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
		return;
	}

	/* if a general query, search all group, reponse*/
	if ( 0 == pPkt->groupadd)                     /* general query response timeout */
	{
		/* go through mc groups, send report */
		pMcGroup = pMcVlan->firstgroup;
		while ( pMcGroup != NULL )
		{
			if ( NULL != pMcGroup->portstatelist )	/* has member, send report to route port */
			{
				if ( NULL != pMcVlan->routerlist )
				{
					Igmp_snoop_router_temp = pMcVlan->routerlist;
					while ( Igmp_snoop_router_temp != NULL ) 
					{
						if (unRouterIfIndex == Igmp_snoop_router_temp->mroute_ifindex)
						{
							stPkt.vlan_id = pMcGroup->vid;
							stPkt.groupadd= pMcGroup->MC_ipadd;

							stPkt.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
							stPkt.type = ( pMcGroup->ver_flag== IGMP_SNOOP_NO ) ? IGMP_MSG_REPORT : IGMP_MSG_V1_REPORT ;
							stPkt.saddr = pMcGroup->report_ipadd;/*the last Port from which received report packet*/

							unIfIndex = stPkt.ifindex;

							igmp_snp_syslog_dbg("send report. vid %d,port_index %d,sIP 0x%x, 0x%8x %d\n",
											pMcGroup->vid,unIfIndex,pMcGroup->report_ipadd,pMcGroup->MC_ipadd);

							if ( IGMPSNP_RETURN_CODE_OK != Igmp_Snoop_Send_Igmp( &stPkt ) )
							{
								igmp_snp_syslog_dbg("send report failed.\n");
								/* if failed in send report, doesn't return, and go on */
							}
						}
						Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
					}
				}
			}
			else
			{
				/* group has No Member,so start the group life timer.*/
				/* add group life timer */
				if ( pMcGroup->grouplifetimer_id == 0 )
				{
					/* Add group life timer */
					igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
					igmp_snp_syslog_dbg("start group life timer. time:%d\n", pMcGroup->grouplifetimer_id);
				}
			}
			pMcGroup = pMcGroup->next;
		}
		/*why not start vlan life timer? mcgroupvlan has No group.
		 *Maybe Cause there's no Event process of vlan lifetimer expiration. */
	}

	/* g-s query response timeout */
	else		
	{
		/* search group */
		if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_searchvlangroup( pPkt->vlan_id, pPkt->groupadd, &pMcGroup, &pPrevGroupVlan ) ) )
		{
			igmp_snp_syslog_dbg("search group failed\n");
			free( pPkt );
			cur->data = NULL;
			cur->datalen = 0;
			igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
			return;
		}
		if ( NULL == pMcGroup )
		{
			igmp_snp_syslog_dbg( "group:%d does not exist.\n", pPkt->groupadd);
			free( pPkt );
			cur->data = NULL;
			cur->datalen = 0;
			igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
			return;
		}
		/* send reports */
		if ( NULL != pMcGroup->portstatelist )                                                                                             /* has member, send report */
		{
			if ( NULL != pMcVlan->routerlist )
			{
				Igmp_snoop_router_temp = pMcVlan->routerlist;
				while ( Igmp_snoop_router_temp != NULL )
				{
					if (unRouterIfIndex == Igmp_snoop_router_temp->mroute_ifindex)
					{
						stPkt.vlan_id = pMcGroup->vid;
						stPkt.groupadd= pMcGroup->MC_ipadd;
						stPkt.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
						stPkt.type = ( pMcGroup->ver_flag== IGMP_SNOOP_NO ) ? IGMP_MSG_REPORT : IGMP_MSG_V1_REPORT ;
						stPkt.saddr = pMcGroup->report_ipadd;
						igmp_snp_syslog_dbg("send report. 0x%8x  port 0x%x\n",
								pMcGroup->MC_ipadd, stPkt.ifindex);

						if ( IGMPSNP_RETURN_CODE_OK != Igmp_Snoop_Send_Igmp( &stPkt ) )
						{
							igmp_snp_syslog_dbg("send report failed.\n");
							/* if failed in send report, doesn't return, and go on */
						}
					}
					Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
				}
			}
		}
		else
		{
			/* group has No Member,so start the group life timer.*/
			/* add group life timer */
			if ( pMcGroup->grouplifetimer_id == 0 )
			{
				/* Add group life timer */
				igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
				igmp_snp_syslog_dbg( "start group life timer. time:%d\n", \
																	pMcGroup->grouplifetimer_id);
			}
		}
		/* No need to delete timer */
	}
	free( pPkt );
	cur->data = NULL;
	cur->datalen = 0;

	igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
	return;
}

/**************************************************************************
* Igmp_Event_Rxmt_Timeout()
*
* INPUTS:
*		cur - pointer to timer element
*
* OUTPUTS:
*
* RETURN VALUE:
*	
*
* DESCRIPTION:
*		Igmp_Event_Rxmt_Timeout handles the retransmit query interval 
*		timeout	message. when report was received from the port, the timer should
*		be deleted. Notice: it should retransmit g-s query for Robustnss variable
*		times.		
*  
*
**************************************************************************/
VOID Igmp_Event_Rxmt_Timeout( timer_element *cur )
{
	igmp_snoop_pkt * pPkt = NULL;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	unsigned int lRet;
	ULONG nextifindex;
	ULONG ulTimerId = 0;
	struct MC_port_state *pstPort = NULL;
	igmp_snoop_pkt stPkt;
	igmp_router_entry * Igmp_snoop_router_temp;

	igmp_snp_syslog_dbg("\nEnter Igmp_Event_Rxmt_Timeout. \n");

	if( NULL == cur )
	{
		igmp_snp_syslog_dbg("Igmp_Event_Rxmt_Timeout: cur is null\n");
		return;
	}
	pPkt = (igmp_snoop_pkt *)cur->data;
	if ( (NULL == pPkt)||(cur->datalen < sizeof(igmp_snoop_pkt)) )
	{
		igmp_snp_syslog_dbg("Igmp_Event_Rxmt_Timeout: pPkt is null\n");
		return;
	}
	/* search group */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_searchvlangroup( pPkt->vlan_id, pPkt->groupadd, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_dbg("Igmp_Recv_Query: search failed\n");
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		return;
	}
	if ( NULL == pMcGroup )
	{
		igmp_snp_syslog_dbg(" Igmp_Event_Rxmt_Timeout: group:0x%x does not exist.\n", pPkt->groupadd);
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		return;
	}

	/* if port state is not check member, no need to send g-s query or do anything */
	igmp_snp_searchportstatelist( &( pMcGroup->portstatelist ), pPkt->ifindex,IGMP_PORT_QUERY, &pstPort );
	if ( pstPort == NULL )
	{
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		igmp_snp_syslog_dbg("#########Igmp_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg(" return by pstPort = NULL.\n");
		return;
	}

	if ( pstPort->state != IGMP_SNP_CHECK_MEMBER )
	{
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		igmp_snp_syslog_dbg("#########Igmp_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg(" return by pstPort->state != IGMP_SNP_CHECK_MEMBER.\n");
		return;
	}

	/* if retransmit times >0 , send g-s query, reset rxmt timer, rxmtcount-- */
	if ( pPkt->retranscnt != 0 )
	{
		stPkt.vlan_id= pPkt->vlan_id;
		stPkt.groupadd= pPkt->groupadd;
		stPkt.type = IGMP_MSG_GS_QUERY;
		stPkt.ifindex = pPkt->ifindex;
		stPkt.saddr = pPrevGroupVlan->saddr;
		if ( IGMPSNP_RETURN_CODE_OK != ( lRet = Igmp_Snoop_Send_Igmp( &stPkt ) ) )
		{
			igmp_snp_syslog_dbg(" Igmp_Event_Rxmt_Timeout: send g-s query failed.\n");
		}

		pPkt->retranscnt --;
		igmp_snp_syslog_dbg("#########Igmp_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg("pPkt->retransmitCount = %d.\n",pPkt->retranscnt);
		igmp_snp_syslog_dbg("#########Igmp_Event_Rxmt_Timeout############\n");
		if ( pPkt->retranscnt )
		{	/*continue timer*/
			igmp_snp_syslog_dbg("#########Igmp_Event_(Igmp_Event_Rxmt_Timeout)Rxmt_Timeout############\n");
			timer_element *new_timer = NULL;
			lRet = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
									igmp_rxmt_interval,
									(void *)Igmp_Event_Rxmt_Timeout,
									(void *)pPkt,sizeof(igmp_snoop_pkt),
									&new_timer);
			if( lRet != IGMPSNP_RETURN_CODE_OK || NULL == new_timer )
			{
				igmp_snp_syslog_dbg("Igmp_recv_query:create timer failed.\n");
				return;
			}
			if( IGMPSNP_RETURN_CODE_OK != add_timer(&igmp_timer_list, new_timer,&(ulTimerId)) )
			{
				igmp_snp_syslog_dbg("Igmp_recv_query:add timer failed.\n");
				return;
			}
		}
	}

	/* if rxmtcount == 0 or send g-s query failed or creat timer failed,
	modify the checking member state port to no member state  */
	if ( ( 0 == pPkt->retranscnt ) || ( lRet != IGMPSNP_RETURN_CODE_OK ) || ( 0 == ulTimerId ) )
	{
		if ( pstPort->state == IGMP_SNP_CHECK_MEMBER )
		{
			stPkt.vlan_id= pPkt->vlan_id;
			stPkt.groupadd= pPkt->groupadd;
			stPkt.type = IGMP_ADDR_DEL;
			stPkt.ifindex = pPkt->ifindex;
			stPkt.group_id= pPkt->group_id;
			
			if (IGMPSNP_RETURN_CODE_OK != (lRet = igmp_snp_mod_addr(&stPkt,IGMP_ADDR_DEL)))
			{
				igmp_snp_syslog_dbg("Igmp_Event_Rxmt_Timeout: failed in setting L2 MC table. 0x%x \n", stPkt.ifindex);
			}
			/* delete port from port list */
			igmp_snp_searchportstatelist( &( pMcGroup->portstatelist), pPkt->ifindex,IGMP_PORT_DEL, &pstPort );
			/* if find router port,send leave to its upstream switch ,
			   else flood leave in vlan,only when it is last member .
*/
			if ( NULL == pMcGroup->portstatelist )
			{
				if ( pPrevGroupVlan->routerlist != NULL )
				{
					Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
					while ( Igmp_snoop_router_temp != NULL )
					{
						igmp_snp_routerleave( pPkt->vlan_id, Igmp_snoop_router_temp->mroute_ifindex, pPkt->groupadd, pPkt->saddr );
						Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
					}
				}
				/*necessary to send leave pkt to other port of the igmpvlan?*/
				else
				{
					LONG unIfIndex;/*ULONG --> LONG: lead to fatal loop! 08/08/23@wujh*/
					ULONG ulportstatus;
					//IFM_PORTONVLANSTART( unIfIndex, ( USHORT ) pPkt->vlan_id )
					if(IGMPSNP_RETURN_CODE_OK !=igmp_getifindex_byvlanid(pPkt->vlan_id,&unIfIndex) )
					{
						igmp_snp_syslog_dbg("igmp_snp_flood: can not get ifindex by vlan_id.\n");
						return;
					}
					while( !(0 > unIfIndex))
					{
						if ( unIfIndex == pPkt->ifindex )
						{
								if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(pPkt->vlan_id,unIfIndex,&nextifindex))
									{
									  unIfIndex = nextifindex;
				  					}else unIfIndex = -1;
							continue;
						}
						/* send report here */
						igmp_getifstatus( unIfIndex, &ulportstatus );
						if ( ulportstatus == 1 )
						{
							igmp_snp_routerleave( pPkt->vlan_id, unIfIndex, pPkt->groupadd, pPkt->saddr );
						}
						if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(pPkt->vlan_id,unIfIndex,&nextifindex))
									{
									  unIfIndex = nextifindex;
				  					}else unIfIndex = -1;
						//IFM_PORTONVLANEND
					}
				}
				igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
				igmp_snp_syslog_dbg(" Igmp_Event_Rxmt_Timeout: add group life timer.%d\n", pMcGroup->grouplifetimer_id);
			}
			/* start Group life timer */
		}
		if( 0 != ulTimerId )
			del_timer(&igmp_timer_list, ulTimerId );
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
	}
	igmp_debug_print_groupvlan(pPrevGroupVlan);
	return;
}

/**********************************************************************************
*igmp_enable_init()
*INPUTS:
*none
*
*OUTPUTS:
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - init success
*		IGMPSNP_RETURN_CODE_ERROR - failed in set port or fail in set global timer
*
*DESCRIPTION:
*	enable igmp snooping init function
*
***********************************************************************************/
INT igmp_enable_init(void)
{
	INT ret;	
	igmp_snoop_pkt t_pkt;
	INT index = 0;
	
	t_pkt.vlan_id = 0;
	t_pkt.groupadd = 0;
	t_pkt.ifindex = 0;
	t_pkt.group_id = 0;
	t_pkt.type = IGMP_SYS_SET_INIT;		/*init*/

	igmp_snp_syslog_event("START:igmp_enable_init\n");

	if( IGMPSNP_RETURN_CODE_OK != igmp_snp_mod_addr(&t_pkt,IGMP_SYS_SET))
	{
		igmp_snp_syslog_err("Igmp_snp_mod_addr: failed in set port pfm.\n");
		goto error;
	}

	if( IGMPSNP_RETURN_CODE_OK != init_igmp_snp_timer())
	{
		igmp_snp_syslog_err("init_igmp_snp_timer: fail in set global timer.\n");
		goto error;
	}

	// init mcgroup_vlan_queue
	for (index = 0; index < IGMP_GENERAL_GUERY_MAX; index++) {
		mcgroup_vlan_queue[index] = NULL;
	}

	igmp_snp_syslog_event("END:igmp_enable_init\n");
	return IGMPSNP_RETURN_CODE_OK;
error:

	igmp_snp_syslog_err("END:igmp_enable_init\n");
	return IGMPSNP_RETURN_CODE_ERROR;
}

/**********************************************************************************
*igmp_enable_init()
*
*INPUTS:
*none
*
*OUTPUTS:
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - stop success
*		IGMPSNP_RETURN_CODE_ERROR - failed in set port or fail in set global timer
*
*DESCRIPTION:
*	stop IGMP SNOOPING，release data
*
***********************************************************************************/
INT igmp_snp_stop(VOID)
{			
	INT tmp = 0;
	INT i,ret = IGMPSNP_RETURN_CODE_OK;
	ULONG ulRet = 0;//usIfIndex = 0;
	igmp_routerport *pRoutePort = NULL;
	MC_group_vlan *tvlan = NULL;
	igmp_snoop_pkt t_pkt;

	igmp_snp_syslog_event("START:igmp_stop start !\n");
	/*  //do not need this, zhangdi@autelan.com 2012-09-18
	if(IGMP_SNP_ISENABLE())
	{
		igmp_snp_syslog_dbg("Fail to stop IGMP snooping:task is not started.\n");
		return IGMPSNP_RETURN_CODE_OK;
	}
	*/
	
	t_pkt.vlan_id = 0;
	t_pkt.group_id = 0;
	t_pkt.groupadd = 0;
	t_pkt.ifindex = 0;
	t_pkt.type = IGMP_SYS_SET_STOP;	

	if( IGMPSNP_RETURN_CODE_OK != igmp_snp_mod_addr(&t_pkt,IGMP_SYS_SET))
	{
		igmp_snp_syslog_dbg("Igmp_snp_mod_addr:failed in set port pfm.\n");
		goto error;
	}

	/*delete timer*/
	if(igmp_timer_list.cnt)
	{
		if( IGMPSNP_RETURN_CODE_OK != del_all_timer(&igmp_timer_list))
		{
			igmp_snp_syslog_dbg("Igmp_timer_delete:fail in delete global timer.\n");
			goto error;
		}
	}

	/*delete message queue*/
	/*not support message queue*/
	
	/*delete mc group*/
	tvlan = mcgroup_vlan_queue[tmp];
	while(NULL != tvlan)
	{
		if(NULL != tvlan )
			igmp_snp_del_mcgroupvlan(tvlan->vlan_id);
		mcgroup_vlan_queue[tmp] = NULL;
		tmp++;
		tvlan = mcgroup_vlan_queue[tmp];
	}

	first_mcgroupvlan_idx = -1;

	/*delete route port list*/
	pRoutePort == p_routerlist;
	while(pRoutePort){
		/*delete first router port*/
		igmp_snp_syslog_dbg("Delete router port: ifindex %d,vid %d,routerport_saddr 0x%x,next 0x%x",\
									pRoutePort->ifindex,\
									pRoutePort->vlan_id,\
									pRoutePort->routeport_saddr,\
									pRoutePort->next);
		igmp_snp_searchrouterportlist(pRoutePort->vlan_id,\
									pRoutePort->ifindex,\
									IGMP_PORT_DEL,\
									NULL);
		/*fet next routerport*/
		pRoutePort = pRoutePort->next;
	}

	/* here need to add function for delete p_vlanlist and something about the vlan */

	/*set disable, when delete information in software and hardware */
	igmp_snoop_enable = IGMP_SNOOP_NO;

	igmp_snp_syslog_event("END:igmp_stop !\n");
	return IGMPSNP_RETURN_CODE_OK;

error:
	igmp_snoop_enable = IGMP_SNOOP_YES;
	return IGMPSNP_RETURN_CODE_ERROR;
}

/*************************************************
 *proccess pakcet flag :IGMP_SNP_FLAG_PKT_UNKNOWN
 ************************************************
 *TODO :just parse ipheader in packet,
		but not parse the further data in packet.
 ************************************************/
/**********************************************************************************
*igmp_recv_pktdata_proc()
*
*INPUTS:
*		msg_skb - receive message pointer
*
*OUTPUTS:
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success
*		IGMPSNP_RETURN_CODE_ERROR - unknown igmp type packet with null skb or ttl illegal or
*										check checksum failed
*		IGMPSNP_RETURN_CODE_GROUP_NOTEXIST - can not find group
*		IGMPSNP_RETURN_CODE_ERROR_HW - add address to hardware failed
*
*DESCRIPTION:
*	
*
***********************************************************************************/
LONG igmp_recv_pktdata_proc(struct igmp_skb *msg_skb )
{
	USHORT	*tmp;
	struct iphdr * pIphd = NULL;
	ULONG ulGroup;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	LONG lVid = msg_skb->vlan_id;/* format msg_skb --wujh@autelan.com 08/09/23*/
	ULONG usIfIndex = msg_skb->ifindex;
	
	if( !msg_skb )
	{
		igmp_snp_syslog_err("unknown type igmp packet with null skb, drop!\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	tmp = (USHORT *)(msg_skb->buf + 12 );
	if( 0x8100 == *tmp ) /*packet vlan tagged*/
		pIphd = (struct iphdr *)(msg_skb->buf  + 18);
	else
		pIphd = (struct iphdr *)(msg_skb->buf  + 14);

	ulGroup = pIphd->daddr;/*NOT always GroupIp just is pIphd->daddr -- wujh*/

	if ( pIphd->ttl < 1 )
	{
		igmp_snp_syslog_err("unknown igmp type packet ttl %d illegal, drop!\n", pIphd->ttl);
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	/* check header checksum */
	if ( inet_cksum( ( USHORT * ) pIphd, ( ULONG ) ( pIphd->ihl ) << 2 ) )
	{
		igmp_snp_syslog_err("unknown igmp type packet check checksum failed, drop!\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	lRet = igmp_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan );
	if ( IGMPSNP_RETURN_CODE_OK != lRet )
	{
		igmp_snp_syslog_err("find group %u.%u.%u.%u in vlan %d failed\n",NIPQUAD(ulGroup),lVid);
		return IGMPSNP_RETURN_CODE_GROUP_NOTEXIST;
	}
	if ( NULL != pMcGroup )
	{
		return IGMPSNP_RETURN_CODE_OK;
	}
	else
	{
		if ( IGMPSNP_RETURN_CODE_OK != igmp_creat_group_node( lVid, ulGroup, usIfIndex, &pMcGroup, &pPrevGroupVlan ) )
		{
			igmp_snp_syslog_err("create new group failed. pVlan 0x%x \n", pPrevGroupVlan);
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	
	/* add address to hardware */
	igmp_snoop_pkt stPkt ;

	stPkt.vlan_id = lVid;
	stPkt.group_id = pMcGroup->mgroup_id;/*original is ZERO*/
	stPkt.groupadd = ulGroup;
	stPkt.ifindex = usIfIndex;
	stPkt.type = IGMP_ADDR_ADD;

	lRet = igmp_snp_mod_addr( &stPkt, IGMP_ADDR_ADD );
	if ( IGMPSNP_RETURN_CODE_OK != lRet )
	{
		igmp_snp_syslog_dbg("add address to hardware failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR_HW;
	}


	/* add group member timer */
	igmp_snp_addintertimer( pMcGroup->grouplifetime, &( pMcGroup->grouplifetimer_id ) );
	return IGMPSNP_RETURN_CODE_OK;
}


/**********************************************************************************
* create_recvskb_thread()
*
* INPUTS:
*		msg_skb - 	receive message pointer
*
* OUTPUTS:
*		null
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on  success
*		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp service not global enabled
*		IGMPSNP_RETURN_CODE_NULL_PTR  - receive packet with null skb
*
*DESCRIPTION:
*	init igmp_snoop receive messsage distribute function
***********************************************************************************/
INT igmp_message_proc(struct igmp_skb *msg_skb)
{
	if((!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE())){
		igmp_snp_syslog_warn("receive packet but igmp and mld service not enabled, drop!\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	if(NULL == msg_skb){
		igmp_snp_syslog_err("receive packet with null skb, drop!\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	switch(msg_skb->nlh.nlmsg_type)
	{
		case IGMP_SNP_TYPE_PACKET_MSG:/*2*/
			
			if(DISABLE == igmp_snoop_enable){break;}

			switch(msg_skb->nlh.nlmsg_flags)
			{
				case IGMP_SNP_FLAG_PKT_UNKNOWN:/*1*/
					igmp_snp_syslog_warn("receive packet with unknown igmp type.\n");
					igmp_recv_pktdata_proc(msg_skb);
					break;
				case IGMP_SNP_FLAG_PKT_IGMP:/*2*/
					igmp_skb_proc(msg_skb);
					break;
				default:
					igmp_snp_syslog_warn("receive packet with unknown message type, drop!\n");
					break;
			}
			break;

		case MLD_SNP_TYPE_PACKET_MSG:/*5*/

			if(DISABLE == mld_snoop_enable){break;}
			
			switch(msg_skb->nlh.nlmsg_flags)
			{
				case MLD_SNP_FLAG_PKT_UNKNOWN:/*4*/
					igmp_snp_syslog_warn("receive packet with unknown MLD type.\n");
					mld_recv_pktdata_proc(msg_skb);
					break;
				case MLD_SNP_FLAG_PKT_MLD:/*5*/
					mld_skb_proc(msg_skb);
					break;
				default:
					igmp_snp_syslog_warn("receive packet with unknown message type, drop!\n");
					break;
			}
			break;

		default:
			igmp_snp_syslog_warn("unkown message type from socket,drop!\n");
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_snp_pim_msghandle
 *
 * DESCRIPTION:
 *		
 *
 * INPUTS:
 *		lVid  - vlan id
 *		ulIfIndex  -  interface index
 *		ulGroup  -  gourp addr
 *		ulSrcAddr  -  source address
 *
 * OUTPUTS:		
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - on success
 *		IGMPSNP_RETURN_CODE_ERROR - search multicast vlan-group failed
 *		IGMPSNP_RETURN_CODE_NULL_PTR - when pim packet is handle, alloc memory failed.
 *
 * COMMENTS:
 *		
 **
 ********************************************************************************/
INT igmp_snp_pim_msghandle( ULONG ulGoup, ULONG ulIfindex, LONG lVid, ULONG ulSrcAddr )
{
	LONG result = 0;
	ULONG trunkIfIndex = 0;      
	igmp_routerport *pRouter = NULL;
	MC_group *pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	igmp_router_entry * Igmp_snoop_router_temp;
	igmp_router_entry ** Igmp_snoop_router_pre;

	/* search vlan-group list, try to find a node */
	if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_searchvlangroup( lVid, ulGoup, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_err("search multicast vlan-group failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	/*check if source port is a member of trunk*/
	result = IFM_PhyIsMerged2TrunkApi(lVid,ulIfindex,&trunkIfIndex );
	if ( result == IGMPSNP_RETURN_CODE_OK ) /*trunk member*/
	{
		igmp_snp_syslog_warn("trunk Ifindex:0x%x.\n", trunkIfIndex);
		ulIfindex = trunkIfIndex;
	}
	else
	{
		ulIfindex = ulIfindex;
	}

	if  ( NULL == pPrevGroupVlan ) 
	{
		return IGMPSNP_RETURN_CODE_OK;
	}

	/* If a query message came from a multicasting router(it's SIP not 0.0.0.0),
	we should change the multicast router port to this port whether it is a 
	designated port */
 	igmp_snp_searchrouterportlist(lVid,ulIfindex,IGMP_PORT_QUERY,&pRouter);

	if ( ( ulSrcAddr != 0 ) && ( pRouter == NULL ) )       /* no router port configured */
	{
		Igmp_snoop_router_pre = &( pPrevGroupVlan->routerlist );

		while ( *Igmp_snoop_router_pre != NULL )
		{
			if ( ( *Igmp_snoop_router_pre ) ->mroute_ifindex == ulIfindex )
			{
				break;
			}
			Igmp_snoop_router_pre = &( ( *Igmp_snoop_router_pre ) ->next );
		}
		if ( *Igmp_snoop_router_pre == NULL )
		{
			Igmp_snoop_router_temp = ( igmp_router_entry * )malloc(sizeof( igmp_router_entry ));
			if ( NULL == Igmp_snoop_router_temp )
			{
				igmp_snp_syslog_err("when pim packet is handle, alloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
			memset( Igmp_snoop_router_temp,0, sizeof( igmp_router_entry ) );
			Igmp_snoop_router_temp->saddr = ulSrcAddr;
			Igmp_snoop_router_temp->mroute_ifindex = ulIfindex;
			Igmp_snoop_router_temp->next = NULL;
			igmp_snp_addintertimer( igmp_router_timeout, &( Igmp_snoop_router_temp->timer_id ) );
			*Igmp_snoop_router_pre = Igmp_snoop_router_temp;
			igmp_snp_syslog_dbg( "get a route port from pim hello, add router port vlan =%d,ifindex=%d\n", pPrevGroupVlan->vlan_id, ulIfindex);
		}
		else
		{
			( *Igmp_snoop_router_pre ) ->saddr = ulSrcAddr;
			igmp_snp_addintertimer( igmp_router_timeout, &( ( *Igmp_snoop_router_pre ) ->timer_id ) );
		}
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * pim_recv_msg
 *
 * DESCRIPTION:
 *		When igmp_snoop rev pim hello packet add route port 		
 *
 * INPUTS:
 *		 msg_skb -	 receive message pointer
 *
 * OUTPUTS:		
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - on success
 *		IGMPSNP_RETURN_CODE_ERROR - search multicast vlan-group failed
 *		IGMPSNP_RETURN_CODE_ERROR_SW - IP header checksum error or Pim packet checksum error
 *
 * COMMENTS:
 *		
 **
 ********************************************************************************/
LONG pim_recv_msg(struct igmp_skb *msg_skb)
{
	struct igmp_info *msg_info = NULL;
	struct iphdr	*pIphd = NULL;
	pim_header_t *pPimphd = NULL;
	ULONG	ulPimLen;
	ULONG lRet = IGMPSNP_RETURN_CODE_OK;
	LONG	lVid = 0;
	ULONG ifindex = 0;
	ULONG ulGroup;
	ULONG ulSrcaddr;

	pIphd = (struct iphdr *)(msg_skb->buf + 8 + 14);  /*asic need the 8 bytes*/

	ulSrcaddr = pIphd->saddr;
	/*get PIM header*/
	pPimphd = ( struct pim_header * ) ( ( UCHAR * ) pIphd + sizeof(   struct iphdr ) );

	igmp_snp_syslog_dbg("receive PIM packet: ver 0x%x,type 0x%x, cksum %d.\n",
	      				pPimphd->pim_vers, pPimphd->pim_type, pPimphd->pim_cksum);

	/* check header checksum */
	if ( inet_cksum( ( USHORT * ) pIphd, ( ULONG ) ( pIphd->ihl ) << 2 ) )
	{
		igmp_snp_syslog_err("IP header checksum error in the PIM packet.\n");
		return IGMPSNP_RETURN_CODE_ERROR_SW;
	}
	/*check pim checksum*/    
	//ulPimLen = (ULONG)pIphd->tot_len - (ULONG) (( pIphd->ihl ) << 2);
	ulPimLen = sizeof(struct pim_header );

	if ( 0 != ( lRet = inet_cksum( ( USHORT * ) pPimphd, ( ULONG ) ulPimLen ) ) )                                                                                                   /* check IGMP packet */
	{
		igmp_snp_syslog_err("Pim packet checksum error. cksum: %d\n", lRet);
		return IGMPSNP_RETURN_CODE_ERROR_SW;
	}
	/* check port/trunk-vlan relation */
	ifindex = *((ULONG *)(msg_skb->buf));
	lVid = *((LONG *)(msg_skb->buf) +1);
	/* distribute message */
	switch ( pPimphd->pim_type )
	{
		/* Pim Hello  packet process */
		case 0x0:          /*PIM_HELLO =0*/
			if ( pPimphd->pim_vers == 1 )
			{
				ulGroup = 0;
			}
			else if ( pPimphd->pim_vers == 2 )
			{
				ulGroup = 0;
			}
			else
			{
				return IGMPSNP_RETURN_CODE_ERROR;
			}
			igmp_snp_pim_msghandle( ulGroup, ifindex, lVid, ulSrcaddr );
			break;
		default:
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}


/**********************************************************************************
* igmp_recv_report()
*
* DESCRIPTION:
*		handle the report message
*
* INPUTS:
*		usIfIndex - interface index which get the report message	 
*		ulGroup	- report to the multicast group addr 
*		lVid	- vlan id	which the port be
*		ulType - 	packet type
*		sk_info - point to packet info
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - success or group address invalid
*		IGMPSNP_RETURN_CODE_ERROR_HW - failed in add address to hardware
*		IGMPSNP_RETURN_CODE_ERROR - create group error or search or add port error
*
*
***********************************************************************************/
LONG igmp_recv_report( ULONG usIfIndex, ULONG ulGroup, LONG lVid, 
								ULONG ulType, struct igmp_info *sk_info)
{
	
	ULONG group_addr = htonl(ulGroup);
	
	LONG result = 0;
	ULONG trunkIfIndex = 0;
	ULONG ulIfIndex = 0;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	struct MC_port_state *pstPort = NULL;
	struct iphdr * pIphd = NULL;
	igmp_router_entry * Igmp_snoop_router_temp, *output;
	igmp_reporter_port *pReporter = NULL;
	member_port_list *pMemberPort = NULL;
	
	pIphd = sk_info->ip_hdr;
	igmp_snp_syslog_event("START:igmp_recv_report\n");

	igmp_snp_syslog_dbg("receive report packet,vlan_id:%d group:%u.%u.%u.%u type:%d ifindex:0x%x\n",
			lVid,NIPQUAD(group_addr),ulType,usIfIndex);

  	/* validate Group address */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_addr_check( ulGroup ) ) )/*not be some specific Ip addr.*/
	{
		igmp_snp_syslog_err( "receive report packet,group address invalid: %u.%u.%u.%u \n", NIPQUAD(group_addr) );
		igmp_snp_syslog_dbg("END:igmp recv report\n");
		return IGMPSNP_RETURN_CODE_OK;
	}

	/* get the multicast-groupVlan and multicast-group */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_err( "search multicast-groupVlan or group failed.\n" );
		igmp_snp_syslog_dbg("END:igmp recv report\n");
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}
	/*check if the report port is a member of trunk */
	result = IFM_PhyIsMerged2TrunkApi( lVid,usIfIndex,&trunkIfIndex );
	if ( result == IGMPSNP_RETURN_CODE_OK ) /*trunk member*/
	{
		igmp_snp_syslog_dbg("port %d its trunkIfindex:0x%x\n", usIfIndex, trunkIfIndex);
		ulIfIndex = trunkIfIndex;
	}
	else
	{
		ulIfIndex = usIfIndex;
	}
	/* if report recv'd from the router port in this vlan, it's a invalid report, discard it */

	if ( NULL == pMcGroup )
	{
		/*NOT find any McGroup,it'll create one.And mcGroupVlan maybe exist or NOT,whatever it's both OK.*/
		igmp_snp_syslog_dbg("McGroup == NULL:Create McGroup and GroupVlan.\n");
		if ( IGMPSNP_RETURN_CODE_OK != igmp_creat_group_node( lVid, ulGroup, ulIfIndex, &pMcGroup, &pPrevGroupVlan ) )
		{
			igmp_snp_syslog_err("create group error. pVlan 0x%x\n", pPrevGroupVlan);
			igmp_snp_syslog_dbg("END:igmp_recv_report\n");
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	/*we have Got the mcGroupVlan and group structure for the report packet.*/
	/*zgm add for if the first group report ,forward report to all router port*/
											/*??conflict with line 2885?*/
	if ( pMcGroup->portstatelist == NULL )
	{
		/*No port member in group,get router port in mcgroupvlan,send report to every router port. */
		igmp_snp_syslog_dbg("this is first member of group's member-list, send report to router port.\n");
		if ( pPrevGroupVlan->routerlist != NULL )
		{
			igmp_snp_syslog_dbg("####loop for ouput the GroupVlan %d routerlist\n", pPrevGroupVlan->vlan_id);
			output = pPrevGroupVlan->routerlist;
			while (NULL != output)
			{
				igmp_snp_syslog_dbg("####port[%d], sddress[%d] nextnode[%d]\n",
									output->mroute_ifindex, output->saddr, output->next);
				output = output->next;
			}
		
			igmp_snp_syslog_dbg("multicast-groupVlan's routlist is not NULL.\n");
			/*got router port in mcgroupvlan*/
			Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
			while ( Igmp_snoop_router_temp != NULL )
			{
				igmp_snp_syslog_dbg("Send report to Router port:vlan:%d group:%u.%u.%u.%u ifindex:0x%x\n",
							lVid,NIPQUAD(group_addr),Igmp_snoop_router_temp->mroute_ifindex);
				igmp_snp_routerreport( lVid, Igmp_snoop_router_temp->mroute_ifindex, ulGroup, sk_info );
				Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
			}
		}
		/*report packet can not send to Non-router port*/
		#if 0
		else 
		{
			/*can NOT find router port in mcgroupvlan,find port in igmp_vlan_list*/
			/*but the port in igmp_vlan_list is NOT router port.*/
			DEBUG_OUT("IGMP>>PGroupVlan has not user configured router port!\n");
			LONG unIfIndex = 0;/*ULONG --> LONG: lead to fatal loop! 08/08/23@wujh*/
			ULONG ulportstatus = 0;

			if(IGMP_SNOOP_OK !=igmp_getifindex_byvlanid(lVid,&unIfIndex) )
			{
				IGMP_SNP_DEBUG("igmp_snp_flood: can not get ifindex by vlan_id.\n");
				return IGMP_SNOOP_ERR;
			}
			while( !(0 > unIfIndex))
			{
				if ( unIfIndex == ulIfIndex )   
				{
					unIfIndex = igmp_get_nextifindex_byifindex(lVid,unIfIndex);
					IGMP_SNP_DEBUG("FUCKING! unIfIndex %d ,ulIfIndex %d.\n",unIfIndex,ulIfIndex);
					continue;
				}
				/* send report here */
				igmp_getifstatus( unIfIndex, &ulportstatus );/*force ulportStatue = 1*/
				if ( ulportstatus == 1 )
				{
					DEBUG_OUT("Send report to router port:\tvlan:%d\tgroup:%u.%u.%u.%u\tifindex:0x%x\n",
							lVid,NIPQUAD(group_addr),unIfIndex);
					/*here the port has already NOT router port!! so,we should call Func:
					 *Igmp_Snoop_Send_Igmp()!But,hearteaselly,here call igmp_snp_routerreport 
					 *is so well-suited.*/
					#if 0
					igmp_snoop_pkt stPkt;
					struct iphdr *pIphd = NULL;
					pIphd = sk_info->ip_hdr;
					pIphd = sk_info->ip_hdr;
					stPkt.vlan_id = lVid;
					stPkt.ifindex = usIfIndex;
					stPkt.groupadd= ulGroup;
					stPkt.type = IGMP_MSG_REPORT;
					stPkt.saddr = pIphd->saddr;
					Igmp_Snoop_Send_Igmp( &stPkt );
					#endif
					igmp_snp_routerreport( lVid, unIfIndex, ulGroup, sk_info);
				}
				unIfIndex = igmp_get_nextifindex_byifindex(lVid,unIfIndex);
				IGMP_SNP_DEBUG("FUCKING 2! unIfIndex %d ,ulIfIndex %d.\n",unIfIndex,ulIfIndex);
			}
			//IFM_PORTONVLANEND
		}
		#endif
		igmp_snp_addintertimer( 100, &( pMcGroup->router_reporttimer ) );
	}

	/*in the protocol,the ports in the same group only need to report one time to the route. 
	so if the reporterlist has port in same group,no need to add it  -- yangxs*/						  
	if ( IGMPSNP_RETURN_CODE_OK != igmp_snp_searchreporterlist( lVid, 0, IGMP_PORT_QUERY, &pReporter ) )
	{
		igmp_snp_syslog_dbg("receive report packet,search reporter list failed\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	/*****************************************************************************/
	/* FUNCTION  igmp_snp_searchreporterlist  ALWAYS RETURN  pReporter == NULL, 
	    no one add something to p_reporterlist  */
	/*****************************************************************************/


	if ( NULL != pReporter ) 	/*There is some member port in vlan*/
	{       
		pMemberPort = pReporter->portlist;
		while ( NULL != pMemberPort )
		{
			if ( ulIfIndex != pMemberPort->ifindex ) 	/*There is a reporter port in this vlan,add this port in outport list*/
			{               
				/* search portlist, if not found create one */
				lRet = igmp_snp_searchportstatelist( &( pMcGroup->portstatelist ), pMemberPort->ifindex, \
				                      IGMP_PORT_ADD, &pstPort );
				if ( ( IGMPSNP_RETURN_CODE_OK != lRet ) || ( NULL == pstPort ) )
				{
					igmp_snp_syslog_dbg("Add Reporter,search port failed. 0x%x\n", pstPort);
					return IGMPSNP_RETURN_CODE_ERROR;
				}

				if ( IGMP_SNP_GROUP_NOMEMBER == pstPort->state )
				{
					igmp_snoop_pkt stPkt ;

					stPkt.vlan_id = lVid;
					stPkt.group_id= pMcGroup->mgroup_id;/*It's the Vidx.Need a mechanism for assignment vidx!*/
					stPkt.groupadd= ulGroup;
					stPkt.ifindex = pMemberPort->ifindex;
					stPkt.type = IGMP_ADDR_ADD;
					if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_mod_addr( &stPkt, IGMP_ADDR_ADD ) ) )
					{
						igmp_snp_syslog_dbg( "receive report packet,failed in add address to hardware.\n");
						return IGMPSNP_RETURN_CODE_ERROR_HW;
					}
				}
				/* delete group life timer, ResponseSwitchTimer */
				IGMP_SNP_DEL_INTERTIMER( pMcGroup->grouplifetimer_id);

				/* Set port state */
				if ( IGMP_SNP_V1_MEMBER != pstPort->state )
					pstPort->state = IGMP_SNP_HAS_MEMBER;

			}
			pMemberPort = pMemberPort->next;
		}
	}

	/* begin handle report message. handle the port - yangxs*/
	/* The difference between v1 and v2 report process is that :
	1. we need to change the port state to v1 member state whether the previous state is.
	2. we should add a v1 host timer to this port, and also the group member timer should 
	be added either.
	*/
	/* search portlist, if not found create one */
	lRet = igmp_snp_searchportstatelist( &( pMcGroup->portstatelist ), ulIfIndex, \
	                    IGMP_PORT_ADD, &pstPort );
	if ( ( IGMPSNP_RETURN_CODE_OK != lRet ) || ( NULL == pstPort ) )
	{
		igmp_snp_syslog_dbg("igmp_recv_report: search port failed. 0x%x\n", pstPort);
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	/* Add L2 L3 mc address to hardware */
	if ( IGMP_SNP_GROUP_NOMEMBER == pstPort->state )
	{
		igmp_snoop_pkt stPkt ;

		stPkt.vlan_id = lVid;
		stPkt.group_id= pMcGroup->mgroup_id;
		stPkt.groupadd= ulGroup;
		stPkt.ifindex = ulIfIndex;
		stPkt.type = IGMP_ADDR_ADD;
		igmp_snp_syslog_dbg("Here to Call :igmp_snp_mod_addr.GroupId = %d!\n",pMcGroup->mgroup_id);
		if ( IGMPSNP_RETURN_CODE_OK != ( lRet = igmp_snp_mod_addr( &stPkt, IGMP_ADDR_ADD ) ) )
		{
			igmp_snp_syslog_dbg("igmp_recv_report: failed in add address.\n");
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}

	/* Set port state */
	if ( IGMP_SNP_V1_MEMBER != pstPort->state )
		pstPort->state = IGMP_SNP_HAS_MEMBER;

	/* delete group life timer.Just do it?*/
	IGMP_SNP_DEL_INTERTIMER( pMcGroup->grouplifetimer_id );
	/*is report port*/
	if ( Igmp_Snoop_IF_Recv_Is_Report_Discard( lVid, ulIfIndex ) == IGMPSNP_RETURN_CODE_ERROR )
	{
		igmp_snp_syslog_dbg( "\n This is a reporter port. \n");  
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	/* add group member timer */
	igmp_snp_addintertimer( igmp_robust_variable * igmp_query_interval
	               	 + igmp_resp_interval, &( pstPort->membertimer_id ) );
	igmp_snp_syslog_dbg("igmp_recv_report: start group member timer. 0x%x\n", pstPort->membertimer_id);

	/* if this is a v1 report, do some extra process */
	if ( IGMP_V1_MEMSHIP_REPORT == ulType )
	{
		/* change member state */
		pstPort->state = IGMP_SNP_V1_MEMBER;
		/* Set v1 host timer */
		igmp_snp_addintertimer( IGMP_V1_ROUTER_PRESENT_TIMEOUT, &( pstPort->hosttimer_id ) );
	}
	pIphd = sk_info->ip_hdr;
	pMcGroup->report_ipadd= ntohl(pIphd->saddr);

	/* output the multicast-groupVlan's detail */
	if(0 != (SYSLOG_DBG_PKT_ALL & igmp_snp_log_level)) {
		igmp_debug_print_groupvlan(pPrevGroupVlan);
	}

	igmp_snp_syslog_dbg( "Out igmp_recv_report.\n");
	return IGMPSNP_RETURN_CODE_OK;
}

/**********************************************************************************
*igmp_recv_query()
*
*INPUTS:
*		IfIndex - interface index	 
*		group	- group addr 
*		vlan_id	- vlan id	 
*		maxresptime - 	the max time to response the query
*		sk_info - point to packet info
*
*OUTPUTS:
*		null
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success 
*		IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR - create timer error 
*		IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR - add timer error
*		IGMPSNP_RETURN_CODE_ERROR - group address invalid 
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc the memery fail
*
*DESCRIPTION:
*		If a query message came from a multicasting router(it's SIP not 0.0.0.0)(server),
*		we should change the multicast router port to this port whether it is a designated port	.
*
***********************************************************************************/
INT igmp_recv_query(ULONG ifindex, USHORT maxresptime, ULONG group,
					LONG vlan_id, struct igmp_info *sk_info)
{
	unsigned int result = IGMPSNP_RETURN_CODE_ERROR;
	ULONG t_ifindex = 0;
	ULONG trunkifindex = 0;
	struct iphdr *ip_hdr = NULL;
	MC_group *t_mcgroup = NULL;
	MC_group_vlan *t_mcgroupvlan = NULL;

	igmp_routerport *t_router = NULL;
	igmp_router_entry *igmp_snp_router_temp;
	igmp_router_entry **igmp_snp_router_pre;

	/*if group !=0,query for specific mc group,
	  if group ==0,query for general mc group*/
	igmp_snp_syslog_event("receive query packet, vlan_id %d group %u.%u.%u.%u maxresptime %d ifindex %d\n",
						vlan_id, NIPQUAD(group), maxresptime, ifindex);

	/*mc group address check*/
	if( 0 != group)
	{	
		if( IGMPSNP_RETURN_CODE_OK != igmp_snp_addr_check(group))
		{
			igmp_snp_syslog_warn("receive query packet,group address invalid %u.%u.%u.%u\n", NIPQUAD(group));
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}

	/*set queryRxFlag*/
	if(IGMP_SNOOP_NO == queryRxFlag){
		queryRxFlag = IGMP_SNOOP_YES;
	}
	
	/*search mc group*//*vlan_id,and the group(group_ip - D_class IP) as double index*/
	if( IGMPSNP_RETURN_CODE_OK != igmp_snp_searchvlangroup(vlan_id,group,&t_mcgroup,&t_mcgroupvlan))
	{
		igmp_snp_syslog_err("search multicast-groupVlan %d and multicast-group %u.%u.%u.%u failed.\n",
							vlan_id, group);
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	/*is trunk port*/
	result = IFM_PhyIsMerged2TrunkApi(vlan_id,ifindex,&trunkifindex );
	if ( result == IGMPSNP_RETURN_CODE_OK ) /*trunk member*/
	{
		igmp_snp_syslog_dbg("port %d its trunkIfindex %d.\n", ifindex, trunkifindex);
		t_ifindex = trunkifindex;
	}
	else
	{
		t_ifindex = ifindex;
	}

	/*the 3 if()s steps along while() in Fun:igmp_snp_searchvlangroup*/
	if( NULL == t_mcgroupvlan)/*mcgroupvlan struct list VACANT*/
	{
		/*NOT find any mcgroupvlan ,the packet flooded in the vlan*/
		igmp_snp_syslog_dbg("FLOOD:multicast-groupvlan = NULL, vlan_id %d ifindex %d.\n",
							vlan_id, t_ifindex);
		igmp_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}
	
	if( (t_mcgroupvlan->vlan_id != vlan_id)&&(0 == group))
	{
		igmp_snp_syslog_dbg("FLOOD:multicast-groupvlan->vlan_id[%d] != vlan_id[%d] && 0==group:ifindex:%d.\n",
							t_mcgroupvlan->vlan_id, vlan_id, t_ifindex);
		igmp_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}
	
	if( (NULL == t_mcgroup)&&(0 == group) )/*mcgroup structure doesnot exists.*/
	{
		igmp_snp_syslog_dbg("FLOOD:multicast-group==NULL && 0==group: vlan_id:%d ifindex:%d\n",
							vlan_id, t_ifindex);
		igmp_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}

	/*If a query message came from a multicasting router(it's SIP not 0.0.0.0)(server),
	* we should change the multicast router port to this port whether it is a designated port*/
	igmp_snp_searchrouterportlist(vlan_id,t_ifindex,IGMP_PORT_ADD,&t_router);
													/*IGMP_PORT_QUERY-modify by wujh 08/08/26/0:37*/
	ip_hdr = sk_info->ip_hdr;
	if((0 != ip_hdr)&&(NULL !=t_router)){
		igmp_snp_syslog_dbg("Set router port: t_router@@%p,t_router->vlan_id %d, t_router->ifindex %d,",\
							t_router, t_router->vlan_id, t_router->ifindex);
		t_router->routeport_saddr = ip_hdr->saddr;/*set router port->saddr*/
		igmp_snp_syslog_dbg("t_router->routeport_saddr 0x%x.\n",t_router->routeport_saddr);

		
		igmp_snp_router_pre = &(t_mcgroupvlan->routerlist);
		
		while(NULL != *igmp_snp_router_pre)
		{
			if((*igmp_snp_router_pre)->mroute_ifindex == t_ifindex )
			{
				break;
			}
			igmp_snp_router_pre = &((*igmp_snp_router_pre)->next);
		}
		if(NULL == *igmp_snp_router_pre)
		{
			igmp_snp_router_temp = (igmp_router_entry *)malloc(sizeof(igmp_router_entry));
			if( NULL == igmp_snp_router_temp)
			{
				igmp_snp_syslog_err("receive query packet, alloc memory to router-entity failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(igmp_snp_router_temp,0,sizeof(igmp_router_entry));
			igmp_snp_router_temp->saddr = ip_hdr->saddr;
			igmp_snp_router_temp->mroute_ifindex = t_ifindex;
			igmp_snp_router_temp->next = NULL;
			/*add router timer*/
			igmp_snp_addintertimer(igmp_router_timeout,&(igmp_snp_router_temp->timer_id));
			
			*igmp_snp_router_pre = igmp_snp_router_temp;
			igmp_snp_syslog_dbg("receive query packet, add router entry: vlan = %d,port = %d,router_entry@@%p,t_mcgroupvlan->routerlist@@%p.\n",\
								vlan_id,t_ifindex,igmp_snp_router_temp,t_mcgroupvlan->routerlist);
		}
		else
		{
			(*igmp_snp_router_pre)->saddr = ip_hdr->saddr;
			(*igmp_snp_router_pre)->mroute_ifindex = t_ifindex;/*08062602:01*/
			/*add router timer*/
			igmp_snp_addintertimer(igmp_router_timeout,&((*igmp_snp_router_pre)->timer_id));
		}
		
		
	}
	
	/*start response timer.(A random number between[0,maxresptime])*/
	if((NULL != t_mcgroup)&&(0 != group))
	{	
		/*query for this mcgroup (the group points to.)*/
		if(NULL != t_mcgroup->portstatelist)	/*only response when group has member*/
		{
			t_mcgroup->resposetime = (maxresptime>0)?(maxresptime * IGMP_V2_TIME_SCALE)
						:(IGMP_V2_QUERY_RESP_INTERVAL * 1000);
			t_mcgroup->resposetime = rand()%((LONG)t_mcgroup->resposetime);
			/*Add Resp Timer*/
			{
				igmp_snoop_pkt *pkt = NULL;
				timer_element *new_timer = NULL;

				pkt = (igmp_snoop_pkt *)malloc(sizeof(igmp_snoop_pkt));
				if(NULL == pkt )
				{
					igmp_snp_syslog_dbg("receive query packet, malloc timer data memory failed.\n");
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset(pkt,0,sizeof(igmp_snoop_pkt));
				pkt->vlan_id = vlan_id;
				pkt->groupadd = group;
				pkt->type = IGMP_TIMEOUT_RESP;
				pkt->ifindex = t_ifindex;
				/*I Really Need pkt->group_id*/
				igmp_snp_syslog_dbg("create timer:resposetime!\n");
				result = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
										t_mcgroup->resposetime,
										(void *)Igmp_Event_Resp_Timeout,
										(void *)pkt,sizeof(igmp_snoop_pkt),
										&new_timer);
				if( IGMPSNP_RETURN_CODE_OK != result || NULL == new_timer )
				{
					igmp_snp_syslog_dbg("receive query packet, create timer failed.\n");
					return IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR;
				}

				if( IGMPSNP_RETURN_CODE_OK != add_timer(&igmp_timer_list, new_timer,&(t_mcgroup->resposetime_id)))
				{
					igmp_snp_syslog_dbg("receive query packet, add timer failed.\n");
					return IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR;
				}
			}
			igmp_snp_syslog_dbg("receive query packet, start resp timer for g-s query: time:%d\n",
							t_mcgroup->resposetime);
			return IGMPSNP_RETURN_CODE_OK;
		}
	}
	else if((0==group)&&(t_mcgroupvlan->vlan_id== vlan_id))
	{
		/*query for general in this vlan.*/
		/*add Resp timer*/
		if( NULL != t_mcgroupvlan->firstgroup )
		{
			igmp_snoop_pkt *pkt = NULL;
			timer_element *new_timer = NULL;
			ULONG resptime; 
			ULONG resptimer_id;

			resptime = (maxresptime>0)?(maxresptime * IGMP_V2_TIME_SCALE)
						:(IGMP_V2_QUERY_RESP_INTERVAL * 1000);
			resptime = rand()%((LONG)t_mcgroup->resposetime);
			igmp_snp_syslog_dbg("receive query packet, resp time 0x%x\n",resptime);
			pkt = (igmp_snoop_pkt *)malloc(sizeof(igmp_snoop_pkt));
			if(NULL == pkt )
			{
				igmp_snp_syslog_dbg("receive query packet, malloc timer data memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(pkt,0,sizeof(igmp_snoop_pkt));
			pkt->vlan_id = vlan_id;
			pkt->groupadd = group; /*Here :group = 0*/
			pkt->type = IGMP_TIMEOUT_RESP;
			pkt->ifindex = t_ifindex;
			/*I Really Need pkt->group_id*/
			result = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
									t_mcgroup->resposetime,/*not be assigned any value.instead of resptime*/
									(void *)Igmp_Event_Resp_Timeout,
									(void *)pkt,sizeof(igmp_snoop_pkt),
									&new_timer);
			if( IGMPSNP_RETURN_CODE_OK != result || NULL == new_timer )
			{
				igmp_snp_syslog_dbg("receive query packet, create timer failed.\n");
				return IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR;
			}

			if( IGMPSNP_RETURN_CODE_OK != add_timer(&igmp_timer_list, new_timer,&(resptimer_id)) )
			{
				igmp_snp_syslog_dbg("receive query packet, add timer failed.\n");
				return IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR;
			}
		}
	}

	return IGMPSNP_RETURN_CODE_OK;
}

/**********************************************************************************
* igmp_recv_leave()
*
* INPUTS:
*		 usIfIndex - Interface Index. 
*		 ulGroup  - multicast group address. 
*		 lVid	 -  vlan id
*		sk_info - point to packet info
*
* OUTPUTS:
*		null
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success 
*		IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR - create timer error 
*		IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR - add timer error
*		IGMPSNP_RETURN_CODE_ERROR - search multicast-groupVlan or group fail 
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc the memery fail
*		IGMPSNP_RETURN_CODE_GROUP_NOTEXIST - group not exist
*
* DESCRIPTION:
*		This function handles IGMP v1 Leave message. If the port received 
*		a Leave message, chang the port state to  checking member state.
*		send g-s query and start rxmt timer. 
*
***********************************************************************************/
LONG igmp_recv_leave( ULONG usIfIndex, ULONG ulGroup, LONG lVid,struct igmp_info *sk_info )
{
	MC_group *pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	struct MC_port_state * pstPort = NULL;
	igmp_snoop_pkt stPkt;
	struct iphdr *pIphd = NULL;
	ULONG ulRxmtTimerId = 0;
	/*
	ULONG unIfIndex;
	*/
	unsigned int result = IGMPSNP_RETURN_CODE_OK;
	ULONG trunkIfIndex = 0,ulIfIndex = 0,ulVidx =0;

	igmp_snp_syslog_dbg("receive leave packet: vid %d group %u.%u.%u.%u if %d.\n",
						lVid, NIPQUAD(ulGroup), usIfIndex);

	if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_addr_check( ulGroup ) ) )
	{
		igmp_snp_syslog_dbg("receive leave packet: group address %u.%u.%u.%u invalid.\n",
							NIPQUAD(ulGroup));
		return IGMPSNP_RETURN_CODE_OK;
	}

	/* check vlan and group (inlcuded in search group process ) */
	if ( IGMPSNP_RETURN_CODE_OK != ( igmp_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_err("search multicast-groupVlan %d and multicast-group %u.%u.%u.%u failed.\n",
							lVid, NIPQUAD(ulGroup));
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	if ( NULL == pMcGroup )
	{
		igmp_snp_syslog_dbg("receive leave packet: group %u.%u.%u.%u does not exist.\n",
							NIPQUAD(ulGroup));
		return IGMPSNP_RETURN_CODE_GROUP_NOTEXIST;
	}

	/*check if the report port is a member of trunk */
	result = IFM_PhyIsMerged2TrunkApi( lVid,usIfIndex,&trunkIfIndex);
	if ( result == IGMPSNP_RETURN_CODE_OK ) /*trunk member*/
	{
		igmp_snp_syslog_dbg("receive leave packet: trunkIfindex %d.\n",
							trunkIfIndex);
		ulIfIndex = trunkIfIndex;
	}
	else
	{
		ulIfIndex = usIfIndex;
	}

	if ( Igmp_Snoop_IF_Recv_Is_Report_Discard( lVid, ulIfIndex ) == IGMPSNP_RETURN_CODE_ERROR )
	{
		igmp_snp_syslog_err("receive leave packet from a report %d of vlan %d, dicard!\n",
							ulIfIndex, lVid);
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	/* search port list */
	igmp_snp_searchportstatelist(&(pMcGroup->portstatelist), ulIfIndex, IGMP_PORT_QUERY, &pstPort);
	if ( pstPort == NULL )           /* no such port */
	{
		igmp_snp_syslog_dbg( "igmp receive leave: port 0x%x is not a member.\n", ulIfIndex);
		return IGMPSNP_RETURN_CODE_OK;
	}
	igmp_snp_syslog_dbg("find port: port:%d state:%c\n", ulIfIndex, pstPort->state);
	/* change port??????(mcGroup) state */
	pstPort->state = IGMP_SNP_CHECK_MEMBER;

	igmp_snp_syslog_dbg("igmp receive leave: %x state %d \n", usIfIndex, pstPort->state);

	/* send g-s query to this port */
	stPkt.vlan_id = lVid;
	stPkt.groupadd= ulGroup;
	stPkt.ifindex = ulIfIndex;
	stPkt.type = IGMP_MSG_GS_QUERY;
	pIphd = sk_info->ip_hdr;
	stPkt.saddr = pIphd->saddr;
	igmp_snp_syslog_dbg("igmp receive leave: pIphd->saddr=[%u.%u.%u.%u]\n", NIPQUAD(pIphd->saddr));

	stPkt.saddr = pPrevGroupVlan->saddr;
	igmp_snp_syslog_dbg("igmp receive leave: pPrevGroupVlan->saddr=[%d]\n", pPrevGroupVlan->saddr);

	igmp_snp_syslog_dbg("igmp receive leave: Ready to send the GS query\n");

	if ( IGMPSNP_RETURN_CODE_OK != ( Igmp_Snoop_Send_Igmp( &stPkt ) ) )
	{
		igmp_snp_syslog_dbg( "igmp receive leave: send leave message failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	/************************************/
	/* set retransmit timer						*/
	/************************************/	
	igmp_snp_syslog_dbg("Set leave timer:\n");

	igmp_snoop_pkt *pkt = NULL;
	timer_element *new_timer = NULL;

	pkt = (igmp_snoop_pkt *)malloc(sizeof(igmp_snoop_pkt));
	if(NULL == pkt )
	{
		igmp_snp_syslog_dbg("igmp receive leave: malloc timer data memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(pkt,0,sizeof(igmp_snoop_pkt));
	pkt->vlan_id = lVid;
	pkt->groupadd = ulGroup;
	pkt->type = IGMP_TIMEOUT_RXMT;
	pkt->retranscnt = igmp_robust_variable -1;
	pkt->ifindex = ulIfIndex;
	//pIphd = sk_info->ip_hdr;/*useless*/
	if( IGMPSNP_RETURN_CODE_OK == L2mc_Entry_GetVidx(lVid,ulGroup,&ulVidx)){
		pkt->group_id  = ulVidx;
	}
	else{
		igmp_snp_syslog_dbg("can NOT find vidx by vlanId & groupIp.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	igmp_snp_syslog_dbg("receive leave packet: groupIp 0x%x,pkt->saddr 0x%x.\n",\
						ulGroup,pkt->saddr);
	 result = create_timer(TIMER_TYPE_NOLOOP,
							TIMER_PRIORI_NORMAL,
							igmp_rxmt_interval,
							(void *)Igmp_Event_Rxmt_Timeout,
							(void *)pkt,sizeof(igmp_snoop_pkt),
							&new_timer);
	if( result != IGMPSNP_RETURN_CODE_OK || NULL == new_timer )
	{
		igmp_snp_syslog_dbg("receive leave packet:create Rxmt-Timeout timer failed.\n");
		return IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR;
	}
	if( IGMPSNP_RETURN_CODE_OK != add_timer(&igmp_timer_list, new_timer,&(ulRxmtTimerId)) )
	{
		igmp_snp_syslog_dbg("receive leave packet:add timer failed.\n");
		return IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR;
	}
	igmp_snp_syslog_dbg("Set leave timer successed.\n");

	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* igmp_recv_unknown()
*
* INPUTS:
*		sk_info - pointer to packet structure 
*
* OUTPUTS:
*
* RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success.
*
* DESCRIPTION:
*		This function hanles unknown types of IGMP message. Because there may
*		be some higher version IGMP message which the IGMP snooping  process
*		do not support. We flood these packet in the vlan and don't use any 
*		information in this message.
* 
*
**************************************************************************/
LONG igmp_recv_unknown(struct igmp_info *sk_info )
{
	LONG lVid;
	ULONG usIfIndex;
	
	igmp_snp_syslog_event("recvive packet unknown its igmp_type\n" );

	lVid = sk_info->vlan_id;
	usIfIndex = sk_info->ifindex;
	igmp_snp_flood( sk_info, lVid, usIfIndex );
	return IGMPSNP_RETURN_CODE_OK;
}

/**********************************************************************************
* igmp_skb_proc()
*
* INPUTS:
*		msg_skb -point to igmp packet
*
* OUTPUTS:
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success
*		IGMPSNP_RETURN_CODE_NULL_PTR -igmp packet with null skb 
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL -fail to alloc memery 
*		IGMPSNP_RETURN_CODE_ERROR - error
*		IGMPSNP_RETURN_CODE_ERROR_SW - when ip address ttl is not 1 
*DESCRIPTION:
*	IGMP/PIM  packet handle
*
***********************************************************************************/
INT igmp_skb_proc(struct igmp_skb *msg_skb)
{
	struct ether_header_t *ethhdr = NULL;
	struct iphdr *ip_addr = NULL;
	struct igmp *igmphd = NULL;
	//ULONG igmplen;
	LONG vlan_id = 0;
	INT igmpType = 0x0,ret = IGMPSNP_RETURN_CODE_OK;
	ULONG uRet = IGMPSNP_RETURN_CODE_OK;
	ULONG group_ip = 0,taged = 0,ifindex =0;
	struct igmp_info *msg_info = NULL;
	struct igmpv3_report *v3_report = NULL;

	igmp_snp_syslog_dbg("Start for skb handle!\n");

	if(NULL == msg_skb) {
		igmp_snp_syslog_err("igmp packet with null skb, drop!\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	#if 0 /*adding & deleting both by wujianhui,delete 08/08/06 17:05*/
	/*defalut : not support 802.1q */
	igmpType = *(char*)(msg_skb->buf+8+sizeof(struct ether_header_t)+sizeof(struct iphdr)+IGMP_ROUTER_ALERT_LEN);

	IGMP_SNP_DEBUG("##########IGMP Packet Type = 0x%x !##############\n",igmpType);
	if(IGMP_V3_MEMSHIP_REPORT == igmpType){
		struct igmpv3_report *v3_report=NULL;
		v3_report = (struct igmpv3_report *)malloc(sizeof(struct igmpv3_report));
		if(NULL == v3_report){
			IGMP_SNP_DEBUG("igmp_skb_proc:malloc memory failed.\n");
			return IGMP_SNOOP_ERR;
		}
		memset(v3_report,0,sizeof(struct igmpv3_report));
		v3_report = (struct igmpv3_report*)(msg_skb->buf+8+sizeof(struct ether_header_t)+sizeof(struct iphdr)+IGMP_ROUTER_ALERT_LEN);
		if( v3_report->numofgrec >1 ){
			IGMP_SNP_DEBUG("IGMP SNP can NOT support multi-group report.");
			return IGMP_SNOOP_ERR;
		}
		group_ip = ntohl(v3_report->grec[0].grec_mca);/*report IGMP GroupIpAddr*/
	}
	#endif 
	/*The most important: parse to build struct igmp_info form struct igmp_skb*/
	msg_info = (struct igmp_info *)malloc(sizeof(struct igmp_info));
	if( NULL == msg_info)
	{
		igmp_snp_syslog_err("malloc mag_info memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(msg_info,0,sizeof(struct igmp_info));
	msg_info->ifindex = msg_skb->ifindex;/*format msg_skb wujh@autelan.com -- 08/09/23*/
	msg_info->vlan_id = msg_skb->vlan_id;

	/*check vlan & port has enabled igmp snooping*/
	ret = igmp_vlanisused(msg_skb->vlan_id,&uRet);
	if (1 != uRet)
	{
		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_ERROR;		
	}

	ret = igmp_is_vlan_ifindex(msg_skb->vlan_id,msg_skb->ifindex);
	if(IGMPSNP_RETURN_CODE_OK != ret){
		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	if( IGMPSNP_RETURN_CODE_OK != igmp_vlanportrelation(msg_info->ifindex,msg_info->vlan_id,&taged))
	{
		igmp_snp_syslog_err("get ifindex tag-mode failed.\n");

		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	if( 1 == taged )	/*taged*/
		msg_info->ip_hdr = (struct iphdr *)(msg_skb->buf + 18);
	else				/*untaged*/
		msg_info->ip_hdr = (struct iphdr *)(msg_skb->buf + 14);

	/*handle PIM packet*/
	if( IPPROTO_PIM == msg_info->ip_hdr->protocol )
	{
		igmp_snp_syslog_event("receive PIM packet.\n");
		pim_recv_msg(msg_skb);

		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_OK;
	}
	
	msg_info->igmp_hdr = (struct igmp *)((ULONG *)msg_info->ip_hdr + msg_info->ip_hdr->ihl);
	msg_info->data = msg_skb;
	ip_addr = msg_info->ip_hdr;
	igmphd = msg_info->igmp_hdr;

	if(IGMP_V3_MEMSHIP_REPORT != igmphd->igmp_type){
		group_ip = ntohl(igmphd->igmp_group.s_addr);//ntohl()---network to host long
	}
	else {
		v3_report = (struct igmpv3_report*)((ULONG *)msg_info->ip_hdr + msg_info->ip_hdr->ihl);
		if( v3_report->numofgrec >1 ){
			igmp_snp_syslog_err("IGMP SNP can NOT support V3 multi-group report, when number of Group Records > 1.\n");

			free(msg_info);
			msg_info = NULL;			
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		group_ip = ntohl(v3_report->grec[0].grec_mca);/*report IGMP GroupIpAddr*/
		
	}
	
	igmp_snp_syslog_dbg("vlan_id %d,ifindex %d,type 0x%x, time:%d, group:0x%x cksum:%d.\n",
					msg_skb->vlan_id,msg_skb->ifindex,igmphd->igmp_type,igmphd->igmp_code,group_ip,\
					igmphd->igmp_cksum);
	
	if(1 != ip_addr->ttl)
	{
		igmp_snp_syslog_err("IP header ttl=[%d] is not equal to 1.\n",ip_addr->ttl);

		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_ERROR_SW;
	}
	#if 0
	IGMP_SNP_DEBUG(":::::::::::::Receive IGMP Packet:igmp_skb_proc:::::::::::::::\n");
	igmp_debug_print_skb(msg_skb);
	/*checksum*/
	if(inet_cksum((USHORT *)ip_addr,(ULONG)(ip_addr->ihl)<<2))/*ip_addr->ihl: ip head length*/
	{
		IGMP_SNP_DEBUG("igmp_skb_proc:IP header checksum error!!\n");
		return IGMP_SNOOP_ERR_IPCHECKSUM;
	}
	igmplen = sizeof(struct igmp);
	IGMP_SNP_DEBUG("IP header len:%d Igmp data len:%d\n",(ip_addr->ihl)<<2,igmplen);
	if( 0 != (ret = inet_cksum((USHORT *)igmphd,(ULONG)(igmplen))))
	{
		IGMP_SNP_DEBUG("igmp_skb_proc:IGMP packet checksum error:cksum:%d\n",ret);
		return IGMP_SNOOP_ERR_IGMPCHKSUM;
	}
	#endif

	#if 0 /*delete 08/08/06*/
	/*if port/trunk not belong to the vlan,not handle this msg*/
	//if(IFM_VlanPortRelationApi(sk_info->ifindex,vlan_id,&taged))
	taged = 1;
	if(0)
	{
		IGMP_SNP_DEBUG("igmp_skb_proc:check the recv port %d and vlan %d.\n",ifindex,vlan_id);
		return IGMP_SNOOP_ERR;
	}
	else if(!taged)
	{
		IGMP_SNP_DEBUG("igmp_skb_proc:the recv port %d and vlan %d.\n",ifindex,vlan_id);
		return IGMP_SNOOP_ERR;
	}
	#endif
	ifindex = msg_info->ifindex;
	vlan_id = msg_info->vlan_id;
	switch(igmphd->igmp_type)
	{
		case IGMP_MEMSHIP_QUERY:		/*IGMP Query */
			igmp_recv_query(ifindex,igmphd->igmp_code,group_ip,vlan_id,msg_info);
			break;
		case IGMP_V3_MEMSHIP_REPORT:
		case IGMP_V1_MEMSHIP_REPORT:
		case IGMP_V2_MEMSHIP_REPORT:	/*IGMP report*/
			igmp_recv_report(ifindex,group_ip,vlan_id,igmphd->igmp_type,msg_info);
			break;
		case IGMP_V2_LEAVE_GROUP:	/*IGMP leave*/
			igmp_recv_leave(ifindex,group_ip,vlan_id,msg_info);
			break;
		default:
			igmp_recv_unknown(msg_info);
			break;	
	}
	free(msg_info);
	msg_info = NULL;
	igmp_snp_syslog_dbg("End for skb handle!\n");

	return IGMPSNP_RETURN_CODE_OK;
}


/**********************************************************************************
* create_recvskb_thread()
*
* DESCRIPTION:
*	init igmp_snoop receive skb thread
*
* INPUTS:
* none
*
* OUTPUTS:
*
* RETURN VALUE:
*	
***********************************************************************************/
static void *create_recvskb_thread(void)
{
	int ret,write_flag;
	int sock=0,accept_sock,len;
	INT recvlen=0;
	struct igmp_skb *msg_skb = NULL; 
	struct sockaddr_un client;
	fd_set rfds;

	struct timeval    tv;	/*select pause--wujh@autelan.com -08/09/24*/
  	struct timeval    now, earliest;
	gettimeofday(&earliest, NULL);
  	earliest.tv_sec++;

	if(IGMPSNP_RETURN_CODE_OK != creatclientsock_stream(IGMP_SNOOP_PKT_SVR_SOCK_PATH,&sock))
	{
		igmp_snp_syslog_err("create receive packet socket failed %d.\n",sock);
		return;
	}
	igmp_snp_syslog_dbg("receive packet socket created %d.\n",sock);

	kernel_fd = sock;
//	igmp_enable_init(); /*ignored when get it.*/
	memset(&client,0,sizeof(struct sockaddr_un));
	len = sizeof(struct sockaddr_un);

	msg_skb = (struct igmp_skb *)malloc(sizeof(struct igmp_skb));
	if( NULL == msg_skb )
	{
		igmp_snp_syslog_err("alloc igmp sbk memory failed.\n");
		return;
	}
	memset(msg_skb,0,sizeof(struct igmp_skb));

	while(1)
	{
#if 0
		if((accept_sock = accept(sock,(struct sockaddr *)&client,&len))<0)
		{
			IGMP_SNP_DEBUG("Accept failed:errno %d [%s].\n",errno,strerror(errno));
			close(sock);
			/*需要增加对其他线程的退出信号*/
			return;
		}
#endif
		accept_sock = sock;
		FD_ZERO(&rfds);
		FD_SET(accept_sock,&rfds);
    	gettimeofday (&now, 0);
    	tv.tv_usec = 0;
    	tv.tv_sec = 0;

    	if (now.tv_sec < earliest.tv_sec) 
		{ 
			/* we must wait more than 1 sec. */
      		tv.tv_sec = 2;
      		tv.tv_usec = 0;
    	}
		else if (now.tv_sec == earliest.tv_sec) 
		{
      		if (now.tv_usec < earliest.tv_usec)
			{
          		tv.tv_usec = earliest.tv_usec - now.tv_usec;
      		}
    	}
		
		switch(select(accept_sock+1,&rfds,NULL,NULL,&tv))
		{
			case -1:
				break;
			case 0:
				gettimeofday(&earliest, NULL);
				earliest.tv_sec++;
				break;
			default:
				if(FD_ISSET(accept_sock,&rfds))
				{
					do
					{
						recvlen = recv(accept_sock,(void *)msg_skb,sizeof(struct igmp_skb),0);
						if(recvlen < 0)
						{
							if(errno == EAGAIN){
								igmp_snp_syslog_err("err again");
								break;
							}
							else if(errno == EBADF){
								igmp_snp_syslog_err("err bad frame");
								break;
							}
							else if(errno == ECONNREFUSED){
								igmp_snp_syslog_err("err connection refused");
								break;
							}
							else if(errno == EFAULT){
							    igmp_snp_syslog_err("err fault");

								break;
							}
							else if(errno == EINTR){
								igmp_snp_syslog_err("err interrupt");
								break;
							}
							else if(errno == EINVAL){
								igmp_snp_syslog_err("err invalid");
								break;
							}
							else if(errno == ENOMEM){
								igmp_snp_syslog_err("err no memory");
								break;
							}
							else if(errno == ENOTCONN){
								igmp_snp_syslog_err("err no memory");
								break;
							}
							else if(errno == ENOTSOCK){
								igmp_snp_syslog_err("err no socket");
								break;
							}
							else{
								igmp_snp_syslog_err("socket receive general err %d.\n", errno);
								break;
							}
						}
						else if(0 == recvlen) {
							igmp_snp_syslog_err("socket receive 0 message,peer side may shutdown... close my socket %d.\n", accept_sock);
							free(msg_skb);
							msg_skb = NULL;
							return ;
						}
						else {
							igmp_snp_syslog_event("packet socket receive %d bytes data.\n",recvlen);
							/*handle skb*/
							if(recvlen > 0 && recvlen <= sizeof(struct igmp_skb))
							{
								if( msg_skb->nlh.nlmsg_len >sizeof(struct igmp_skb)||
									msg_skb->nlh.nlmsg_len <= (sizeof(struct nlmsghdr) + 8)){	
										igmp_snp_syslog_warn("warn: received packet has no payload,drop it!\n", msg_skb->nlh.nlmsg_len);
								}
								else {
									if (0 != (SYSLOG_DBG_PKT_ALL & igmp_snp_log_level))
									{
										if(MLD_SNP_TYPE_PACKET_MSG == msg_skb->nlh.nlmsg_type){
											mld_debug_print_skb(msg_skb);
										}
										else{
												igmp_debug_print_skb(msg_skb);/*ignored when get it.*/
										}		
									}
									ret = igmp_message_proc(msg_skb);
									if(IGMPSNP_RETURN_CODE_OK != ret){
										igmp_snp_syslog_err("igmp message proc err.");
										}
								}
							}
						}
						// reset skb memory
						memset(msg_skb,0,sizeof(struct igmp_skb));
					}while(recvlen > 0);
			}
			break;
		}
	}
	close(accept_sock);
	free(msg_skb);
	msg_skb = NULL;
	return ;
}

/**********************************************************************************
* init_igmp_snp_timer()
*
* INPUTS:
*		cur - mc group timer
* OUTPUTS:
*		null
* RETURN VALUE:
*		null
* DESCRIPTION:
*		 mc group timeout handle
***********************************************************************************/
VOID igmp_snp_global_timer_func( timer_element *cur )
{
	INT i =0;
	INT ret = 0;
	INT next = 0;
	MC_group_vlan *p_vlan = NULL;
	igmp_router_entry *p_router_entry = NULL;
	igmp_router_entry *p_prev_router_entry = NULL;
	MC_group *p_group = NULL;
	MC_group *next_group = NULL;
	struct MC_port_state	*p_port = NULL;
	
	for(i = 0; i<IGMP_GENERAL_GUERY_MAX; ++i)
	{
		if(0 < igmp_genquerytime[i]){
			igmp_genquerytime[i]--;
		}
	}
	
	p_vlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
	//p_vlan = GET_VLAN_POINT_BY_INDEX(0);
	while(NULL != p_vlan)
	{
		/*IGMP_SNP_DEBUG("Enter :igmp_snp_global_timer_func...\n");*/
		next = p_vlan->next;		/*get next index firstly*/
#if 1
		if( 0 != p_vlan->vlan_id )
		{
			if( p_vlan->querytimer_id == 1 && igmp_genquerytime[p_vlan->vlan_id] <=1)
			{
				igmp_snp_syslog_event("multicast-groupVlan %d GenQueryTimeOut.\n", p_vlan->vlan_id );
				Igmp_Event_GenQuery_Timeout(p_vlan);
				mld_Event_GenQuery_Timeout(p_vlan);
				p_vlan->querytimer_id = igmp_query_interval;
				igmp_genquerytime[p_vlan->vlan_id] = igmp_query_interval;
			}
			p_vlan->querytimer_id--;
		}
#endif
		if(NULL != p_vlan->routerlist)
		{
			p_router_entry = p_vlan->routerlist;
			p_prev_router_entry = p_vlan->routerlist;
			while( NULL != p_router_entry )
			{
				if( 0 < p_router_entry->timer_id )
				{
					if( 1 == p_router_entry->timer_id )
					{				
						igmp_snp_syslog_event("router %d timeout.\n", p_router_entry->mroute_ifindex);
						p_prev_router_entry = p_router_entry->next;
						if( p_vlan->routerlist == p_router_entry )	/*The first route port,need update vlan->routerlist*/
							p_vlan->routerlist = p_prev_router_entry;	
						free(p_router_entry);
						p_router_entry = p_prev_router_entry;	
						continue;
					}
					else
					{
						p_router_entry->timer_id--;
					}
				}
				p_prev_router_entry = p_router_entry;
				p_router_entry = p_router_entry->next;
			}
		}

		p_group = p_vlan->firstgroup;
		while(NULL != p_group)
		{
			if( 0 < p_group->grouplifetimer_id )
			{
				if( 1 == p_group->grouplifetimer_id )
				{
					next_group = p_group->next;
					
					igmp_snp_syslog_dbg("GroupLifetimer p_group->vid %d,p_group->mgroup_id %d, p_group->MC_ipadd 0x%x \n"\
										"p_group->MC_v6ipadd %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,\n"\
										"p_group->grouplifetimer_id %d,p_group=%p.\n",\
							p_group->vid,
							p_group->mgroup_id,
							p_group->MC_ipadd,
							ADDRSHOWV6(p_group->MC_v6ipadd),
							p_group->grouplifetimer_id,
							p_group);
					if(NULL != p_group->next){
						igmp_snp_syslog_dbg("next->vid %d next->mgroup_id %d, next->MC_ipadd 0x%x \n"\
											"next->MC_v6ipadd %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"\
											"next->grouplifetimer_id %d.",\
							p_group->next->vid,
							p_group->next->mgroup_id,
							p_group->next->MC_ipadd,
							ADDRSHOWV6(p_group->next->MC_v6ipadd),
							p_group->next->grouplifetimer_id);
					}
					
					igmp_snp_syslog_event("p_vlan %d group %d GroupLife_Timeout.",p_vlan->vlan_id, p_group->mgroup_id);

					if(0 == p_group->MC_ipadd)
						ret = mld_Event_GroupLife_Timeout(p_vlan,p_group);
					else
						ret = Igmp_Event_GroupLife_Timeout(p_vlan,p_group);
					if( IGMPSNP_RETURN_CODE_OK != ret)
					{
						igmp_snp_syslog_err("delete group failed.\n");
						return;
					}

					if( NULL == p_vlan ){
						igmp_snp_syslog_dbg("Delete group and vlan successful.\n");
						break;	/*vlan is already released */
					}
					else{
						igmp_snp_syslog_dbg("this p_vlan has have multicast group.\n");
					}

					p_group = next_group;
					igmp_snp_syslog_dbg(" next group continue.the next p_group address@@%p\n",p_group);
					continue;
				}

				p_group->grouplifetimer_id--;
			}
			
			p_port = p_group->portstatelist;
			while( NULL != p_port )
			{
				/*groupMember timeout*/
				if( 0 != p_port->membertimer_id )
				{
					if( 1 == p_port->membertimer_id )
					{	
						/*Release this port state*/
						struct MC_port_state *next_port = p_port->next;
						igmp_snp_syslog_event("multicast-groupVlan %d, group %u.%u.%u.%u,\n"\
												"or v6 group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, port %d GroupMember_Timeout",\
												p_vlan->vlan_id, 
												NIPQUAD(p_group->MC_ipadd),
												ADDRSHOWV6(p_group->MC_v6ipadd),
												p_port->ifindex);

						if(0 == p_group->MC_ipadd)  /*when it is mld group, the MC_ipadd is 0*/
						ret = mld_Event_GroupMember_Timeout(p_vlan,p_group,p_port->ifindex);
						else
						ret = Igmp_Event_GroupMember_Timeout(p_vlan,p_group,p_port->ifindex);
						
						p_port = next_port;
						igmp_snp_syslog_dbg("next port continue\n");
						continue;
					}

					p_port->membertimer_id--;
				}

				/*v1 host timeout*/
				if( 0 != p_port->hosttimer_id )
				{
					if( 1 == p_port->hosttimer_id )
					{
						p_port->hosttimer_id = 0;	/*time expire, set state*/
						p_port->state = IGMP_SNP_HAS_MEMBER;
						p_port = p_port->next;
						continue;
					}
					p_port->hosttimer_id--;
				}

				p_port = p_port->next;
			} //end of group->port

			p_group = p_group->next;
		} //end of mcgvlan->group

		p_vlan = GET_VLAN_POINT_BY_INDEX(next);
	} //end of mcgvlan,end of while

	return ;
}

/**********************************************************************************
*init_igmp_snp_timer()
*
*INPUTS:
*	none
*
*OUTPUTS:
*
*RETURN VALUE:
*		IGMPSNP_RETURN_CODE_OK - on success
*		IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR - create timer error
*		IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR - add timer error 
*
*DESCRIPTION:
*	create mc group timer
*
***********************************************************************************/
INT init_igmp_snp_timer(void)
{
	timer_element *new_timer = NULL;
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;
	igmp_snp_syslog_dbg("START:init_igmp_snp_timer\n");

	ret = create_timer(TIMER_TYPE_LOOP,\
							TIMER_PRIORI_NORMAL,\
							500,	/* here 1 means 1ms, for timer thread wake up every usleep(1000) */
							(void *)igmp_snp_global_timer_func,\
							NULL,\
							0,\
							&new_timer);
	if( IGMPSNP_RETURN_CODE_OK != ret || NULL == new_timer )
	{
		igmp_snp_syslog_err("create timer failed.ret %x, new_timer %p\n", ret, new_timer);
		igmp_snp_syslog_dbg("END:init_igmp_snp_timer\n");
		return IGMPSNP_RETURN_CODE_CREATE_TIMER_ERROR;
	}

	if( IGMPSNP_RETURN_CODE_OK != add_timer(&igmp_timer_list,new_timer,NULL))
	{
		igmp_snp_syslog_err("add timer failed.\n");
		igmp_snp_syslog_dbg("END:init_igmp_snp_timer\n");
		return IGMPSNP_RETURN_CODE_ADD_TIMER_ERROR;
	}

	igmp_snp_syslog_dbg("END:init_igmp_snp_timer\n");
	return IGMPSNP_RETURN_CODE_OK;
}

 
/**********************************************************************************
*create_timer_thread()
*
*DESCRIPTION:
*	init igmp_snoop timer thread
*
*INPUTS:
*	null
*OUTPUTS:
*	null
*RETURN VALUE:
*	null
*
***********************************************************************************/

static void *create_timer_thread(void)
{
	INT ret = 0;
	int timer_id;
	timer_element *tnext = NULL;
		
	while(1)
	{
		ret = usleep(1000);
		if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()))	/*igmp snoop disable*/
			continue;

		if( -1 == ret )
		{
			igmp_snp_syslog_err("create timer thread:usleep error.\n");
			return;
		}
		pthread_mutex_trylock(&mutex);
		while(igmp_timer_list.lock)
			igmp_timer_list.lock = 0;

		if( igmp_timer_list.first_timer )
		{
			tnext = igmp_timer_list.first_timer;
			while( tnext )
			{
				tnext->current++;
				if(tnext->current == tnext->expires )
				{
					if( NULL != tnext->func )
						tnext->func(tnext);

					if( TIMER_TYPE_NOLOOP == tnext->type )
					{		/*NO LOOP*/
						timer_id = tnext->id;
						tnext = tnext->next;
						igmp_timer_list.lock = 1;
						del_timer(&igmp_timer_list,timer_id);
						igmp_timer_list.lock = 0;
						continue;
					}
					else
					{		/*LOOP*/
						tnext->current = 0;
					}
				}
				tnext = tnext->next;
			}
		}
		igmp_timer_list.lock = 1;
		pthread_mutex_unlock(&mutex);
	}	
	igmp_snp_syslog_dbg("Create timer thread success!\n");
}

#if 0
/**********************************************************************************
*igmp_config_init()
*INPUTS:
*none
*OUTPUTS:
*RETURN VALUE:
*	0 - success
*	!=0 - error
*DESCRIPTION:
*	init igmp_snoop according to config file
***********************************************************************************/
static INT igmp_config_init(void)
{	
	CHAR buf[CONF_FILE_MAX_ROW];	
	FILE *fd=NULL;
	
	/*默认配置*/
	igmp_snoop_debug = 1;
	//igmp_snoop_enable = IGMP_SNOOP_YES;

	p_l2mc_list_head.stListHead = NULL;
	p_l2mc_list_head.ulListNodeCount = 0;
	memset(&igmp_timer_list,0,sizeof(timer_list));
	igmp_timer_list.lock = 1;
	
	/*init l2 Mcast group list*/
	mcgroup = (struct mcgroup_s*)malloc(sizeof(struct mcgroup_s)*IGMP_SNP_GRP_MAX);
	if(NULL == mcgroup){
		return -1;
	}
	memset(mcgroup,0,(sizeof(struct mcgroup_s)*IGMP_SNP_GRP_MAX));

	IGMP_SNP_DEBUG("init igmp\n");

	if(NULL ==(fd = fopen(IGMP_SNOOP_CONFIG_PATH,"rt")) )
	{
		IGMP_SNP_DEBUG("Config file is not existence.\n");
	}
	else{
		IGMP_SNP_DEBUG("find config file success on path %s\n",IGMP_SNOOP_CONFIG_PATH);
		memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
		//while(fgets(buf,CONF_FILE_MAX_ROW-1,fd))
		while(!feof(fd))
		{
			fgets(buf,CONF_FILE_MAX_ROW - 1,fd);
			if('#' == buf[0])
			{
				IGMP_SNP_DEBUG("read a line head with '#'.\n");
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				continue;
			}
			read_config(buf);
			memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
		}

	}
	return IGMP_SNOOP_OK;
}
#endif

/**********************************************************************************
*igmp_config_init_dbus()
*INPUTS:
*none
*
*OUTPUTS:
*
*RETURN VALUE:
*	IGMPSNP_RETURN_CODE_OK - config ok
*
*DESCRIPTION:
*	init igmp_snoop NOT according to config file
***********************************************************************************/
static INT igmp_config_init_no_config_file(void)
{	
	INT i =0;
	igmp_snp_syslog_dbg("init igmp with no config file\n");
	/*默认配置*/
	igmp_snp_log_level = SYSLOG_DBG_DEF;
	/*igmp_snoop_enable = IGMP_SNOOP_YES;*/

	igmp_get_macaddr = IGMP_SNOOP_NO;
	
	p_l2mc_list_head.stListHead = NULL;
	p_l2mc_list_head.ulListNodeCount = 0;
	
	memset(&igmp_timer_list,0,sizeof(timer_list));
	igmp_timer_list.lock = 1;

	for(i=0;i<IGMP_GENERAL_GUERY_MAX;i++){
		mcgroup_vlan_queue[i] = NULL;
	}
	
	#if 0
	/*init l2 Mcast group list*/
	mcgroup = (struct mcgroup_s*)malloc(sizeof(struct mcgroup_s)*IGMP_SNP_GRP_MAX);
	if(NULL == mcgroup){
		return -1;
	}
	memset(mcgroup,0,(sizeof(struct mcgroup_s)*IGMP_SNP_GRP_MAX));

	/*get device system mac address*/
	igmp_read_sys_mac();
	#endif
		
	return IGMPSNP_RETURN_CODE_OK;
}
#if 0
/**********************************************************************************
*create_recvskb_thread_dgram()
*INPUTS:
*none
*OUTPUTS:
*RETURN VALUE:
*	0 - success
*	!=0 - error
*DESCRIPTION:
*	init igmp_snoop receive skb thread
***********************************************************************************/
static void *create_recvskb_thread_dgram(void)
{
	INT recvlen,sock;
	struct igmp_skb *msg_skb = NULL; 
	socklen_t addrlen = sizeof(struct sockaddr);
	fd_set rfds;

	char buf[IGMP_MSG_MAX_SIZE];
	msg_skb = (struct igmp_skb*)buf;

	if(0 >(sock = create_packet_clientsock_dgram()))
	{
		IGMP_SNP_DEBUG("igmp_snoop_main::Create skb socket failed.\n");
		return;
	}
	IGMP_SNP_DEBUG("create_recvskb_thread_dgram::Create skb socket ok,SkbSock fd=%d.\n",sock);
	kernel_fd = sock;
	while(1)
	{
		memset(buf,0,IGMP_MSG_MAX_SIZE);
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		switch(select(sock+1,&rfds,NULL,NULL,NULL))
		{
			case -1:
				break;
			case 0:
				break;
			default:
				if(FD_ISSET(sock,&rfds))
				{
					do{
						recvlen = recvfrom(sock,(void *)buf,IGMP_MSG_MAX_SIZE,0,(struct sockaddr *)&pktRemoteAddr, &addrlen);
						IGMP_SNP_DEBUG("Read data successful,recv length =%d!\n",recvlen);
						/*handle skb*/
						if(recvlen>=60)
						{
							//igmp_debug_print_skb(msg_skb);/*ignored when get it.*/
							IGMP_SNP_DEBUG("process skb after Read data successful!\n");
							igmp_message_proc(msg_skb);
						}
					}while(recvlen > 0);
				}
		}
	}
	close(sock);
}
#endif


/**********************************************************************************
*main()
*
*INPUTS:
*none
*
*OUTPUTS:
*
*RETURN VALUE:
*	IGMPSNP_RETURN_CODE_OK - success
*	IGMPSNP_RETURN_CODE_ERROR - Create thread fail
*
*DESCRIPTION:
*
***********************************************************************************/
int main(VOID)
{
	int ret = 0;
#if 0
	if(IGMP_SNOOP_OK != igmp_config_init())
	{
		igmp_snp_syslog_dbg("Init igmp snooping failed.\n");
		goto error;
	}
#endif
    /* get my local slot */
    igmp_slot = igmp_get_num_from_file("/proc/board/slot_id");
    if(igmp_slot > 0)
    {
	    igmp_snp_syslog_dbg("Igmp get local slot %d.\n",igmp_slot);
    }
	else
	{
	    igmp_snp_syslog_dbg("Igmp get local slot eror!!!\n");		
	}
	
	ret = pthread_create(&thread_timer,NULL,(void *)create_timer_thread,NULL);
	if( 0 != ret )
	{
		igmp_snp_syslog_err("Create timer thread fail. %s\n", strerror(errno));
		goto error;
	}
	igmp_snp_syslog_dbg("Create timer thread ok.\n");
	#if 0
	ret = pthread_create(&thread_msg,NULL,(void *)create_msg_thread,NULL);
	if( 0 != ret )
	{
		igmp_snp_syslog_dbg("Create message thread fail.\n");
		pthread_join(thread_timer,NULL);
		goto error;
	}
	igmp_snp_syslog_dbg("Create ctrl msg thread ok.\n");
	#endif

	ret = pthread_create(&thread_recvskb,NULL,(void *)create_recvskb_thread,NULL);
	//ret = pthread_create(&thread_recvskb,NULL,(void *)create_recvskb_thread_dgram,NULL);
	if( 0 != ret )
	{
		igmp_snp_syslog_dbg("Create packet skb thread fail.\n");
		pthread_join(thread_timer,NULL);
		goto error;
	}
	igmp_snp_syslog_dbg("Create data recvskb thread ok.\n");

	if(IGMPSNP_RETURN_CODE_OK != igmp_snp_dbus_init())
	{
		igmp_snp_syslog_dbg("IGMP_Snooping dbus init error!\n");
	}
	igmp_snp_syslog_dbg("IGMP_Snooping dbus init ok!\n");

	ret = pthread_create(&dbus_thread,NULL,(void *)igmp_snp_dbus_thread_main,NULL);
	if( 0 != ret )
	{
		igmp_snp_syslog_dbg("Create message thread fail.\n");
		pthread_join(thread_recvskb,NULL);
		pthread_join(thread_timer,NULL);
		goto error;
	}
	//ret = pthread_create(&thread_npd_msg,NULL,(void *)create_npd_msg_thread,NULL);
	ret = pthread_create(&thread_npd_msg,NULL,(void *)create_npd_msg_thread_dgram,NULL);
	if( 0 != ret )
	{
		igmp_snp_syslog_dbg("Create npd message thread fail.\n");
		pthread_join(dbus_thread,NULL);
		pthread_join(thread_recvskb,NULL);
		pthread_join(thread_timer,NULL);
		goto error;
	}
	igmp_snp_syslog_dbg("Create npd msg thread ok.\n");

	if(IGMPSNP_RETURN_CODE_OK != igmp_config_init_no_config_file())
	{
		igmp_snp_syslog_dbg("Init igmp snooping failed.\n");
		goto error;
	}
	igmp_snp_syslog_dbg("Igmp snooping config ok.\n");
	pthread_join(thread_npd_msg,NULL);
	pthread_join(dbus_thread,NULL);
	pthread_join(thread_recvskb,NULL);
	pthread_join(thread_timer,NULL);
	igmp_snp_syslog_dbg("igmp snoop task start!");

	return IGMP_RETURN_CODE_0;
error:
	return IGMP_RETURN_CODE_1;
}
#ifdef __cplusplus
}
#endif
