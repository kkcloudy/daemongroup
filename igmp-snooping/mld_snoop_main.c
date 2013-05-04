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
* 		yangxs@autelan.com
*
* DESCRIPTION:
* 		mld main routine
*
* DATE:
*		3/24/2010
*
* FILE REVISION NUMBER:
*  		$Revision: 1.4 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


#include <sys/wait.h>
#include <sys/time.h>
#include <stdio.h>

#include "mld_snoop_main.h"
#include "mld_snoop.h"
#include "igmp_snp_dbus.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"


/*******************************************************************************
* mld_v6addr_equal_0
*
* DESCRIPTION:
*		
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
INT mld_v6addr_equal_0
(
	u_short * group
)
{
	if((0 == group[0])&&(0 == group[1])&&(0 == group[2])&&(0 == group[3])&& \
		(0 == group[4])&&(0 == group[5])&&(0 == group[6])&&(0 == group[7])){
		return MLD_RETURN_CODE_EQUAL;
	}
	else{
		return MLD_RETURN_CODE_NOT_EQUAL;
	}
}
/*******************************************************************************
* mld_ntohs
*
* DESCRIPTION:
*		
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
INT mld_ntohs
(
	u_short ** group_ip
)
{
	int i = 0;
	u_short tempgp = 0;
	
	for(i; i < SIZE_OF_IPV6_ADDR; i++){ 
		tempgp = ntohs((*group_ip)[i]);//ntohs()---network to host ushort
		(*group_ip)[i] = tempgp;
	}
	/*change the position of 8 u_short symmetrically*/
	tempgp = 0;
	for(i=0; i<(SIZE_OF_IPV6_ADDR/2); i++){
		tempgp = (*group_ip)[i];
		(*group_ip)[i] = (*group_ip)[SIZE_OF_IPV6_ADDR-1-i];
		(*group_ip)[SIZE_OF_IPV6_ADDR-1-i] = tempgp;
	}
}

/*******************************************************************************
* mld_htons
*
* DESCRIPTION:
*		
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
INT mld_htons
(
	u_short ** group_ip
)
{
	int i = 0;
	u_short tempgp = 0;
	
	for(i; i < SIZE_OF_IPV6_ADDR; i++){ 
		tempgp = ntohs((*group_ip)[i]);//ntohs()---network to host ushort
		(*group_ip)[i] = tempgp;
	}
	/*change the position of 8 u_short symmetrically*/
	tempgp = 0;
	for(i=0; i<(SIZE_OF_IPV6_ADDR/2); i++){
		tempgp = (*group_ip)[i];
		(*group_ip)[i] = (*group_ip)[SIZE_OF_IPV6_ADDR-1-i];
		(*group_ip)[SIZE_OF_IPV6_ADDR-1-i] = tempgp;
	}
}

/*******************************************************************************
 * mld_copy_ipv6addr
 *
 * DESCRIPTION:
 *		
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
INT mld_copy_ipv6addr
(
	USHORT *ugroup,
	USHORT **destugroup
)
{
	int i =0;
	for(i=0; i<SIZE_OF_IPV6_ADDR; i++){
		(*destugroup)[i] = ugroup[i];
	}
	return MLD_RETURN_CODE_OK;
}


/*******************************************************************************
 * mld_compare_ipv6addr
 *
 * DESCRIPTION:
 *		
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
/*compare ipv6 address*/
INT mld_compare_ipv6addr
(
	unsigned short *addr1, 
	unsigned short *addr2
)
{
	unsigned short temp1 = 0;
	unsigned short temp2 = 0;
	int i =0;
	for(i; i<SIZE_OF_IPV6_ADDR; i++){
		temp1 = addr1[i];
		temp2 = addr2[i];
		if(temp1 != temp2) break;
	}

	if(SIZE_OF_IPV6_ADDR == i) 
		return MLD_RETURN_CODE_EQUAL;
	else
		return MLD_RETURN_CODE_NOT_EQUAL;		
}
/**************************************************************************
* mld_print_pkt()
*
* INPUTS:		
*		data - pointer to what to print
*		len - print length 
*
* OUTPUTS:
*		NULL
*						  
* RETURN VALUE:
*		-1 - error
*		0 - print success
*
* DESCRIPTION:
*		 mld packet print.
*		
**************************************************************************/
int mld_print_pkt
(
	unsigned char *data,
	int len
)
{
	int i = 0;

	if(!data) {
		return MLD_RETURN_CODE_ERROR;
	}

	MLD_DBG("................mld hex pkt...............%d\n", len);
	for(i = 0; i < len; i++) {
		if(i && !(i%16)){	
			MLD_DBG("\n");
		}
		MLD_DBG("%02x ", data[i]);
	}
	MLD_DBG("\n");
	MLD_DBG("................mld hex pkt...............\n");
	
	return MLD_RETURN_CODE_OK;
}

/*******************************************************************************
 * mld_debug_print_skb
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
void mld_debug_print_skb(struct igmp_skb *msg_skb)
{
	struct ipv6hdr *ip_addr = (struct ipv6hdr *)(msg_skb->buf + 14);	
	struct mld *mldhd = (struct mld *)((unsigned char *)ip_addr+IPV6_HEADER_LEN);
	struct mld *mldhdopt = (struct mld *)((unsigned char *)ip_addr+IPV6_HEADER_LEN + MLD_IPV6_OPTION_LEN);
	struct pim_header *pimhd = (struct pim_header *)((unsigned char *)ip_addr + IPV6_HEADER_LEN);
	struct mldv2 *v2_report = NULL;

	if(0 == (SYSLOG_DBG_PKT_REV & igmp_snp_log_level))
	{
		return;
	}

	if((msg_skb->nlh.nlmsg_len - sizeof(msg_skb->nlh))< 0){
		igmp_snp_syslog_warn("actual packet size %d less than msg header.\n",msg_skb->nlh.nlmsg_len);
		return;
	}
	
	igmp_snp_syslog_pkt_rev("*****************packet brief******************\n");
	igmp_snp_syslog_pkt_rev("vid %d if %#x ipv%d nexthdr %d payload_len %d hop_limit %d\n",	\
							(msg_skb->vlan_id),(msg_skb->ifindex),ip_addr->version,	\
							ip_addr->nexthdr,ip_addr->payload_len,ip_addr->hop_limit);
	igmp_snp_syslog_pkt_rev("sip %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"\
							"dip %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",\
							ADDRSHOWV6(ip_addr->saddr.s6_addr16),ADDRSHOWV6(ip_addr->daddr.s6_addr16));
	if( IPPROTO_PIM == ip_addr->nexthdr)
	{
		igmp_snp_syslog_pkt_rev("pim ver %d type %d cksum %d rsvd %d\n",
							pimhd->pim_vers,pimhd->pim_type,pimhd->pim_cksum,pimhd->pim_reserved);
	}
	else if(0 == ip_addr->nexthdr){
		if(MLD_V2_MEMSHIP_REPORT != mldhdopt->type){
			igmp_snp_syslog_pkt_rev("type %d code %d cksum %d\n"\
									"group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",	\
									mldhdopt->type,mldhdopt->code,mldhdopt->cksum,	\
									ADDRSHOWV6(mldhdopt->group));			
		}
		else if(MLD_V2_MEMSHIP_REPORT == mldhdopt->type){
			v2_report = (struct mldv2 *)mldhdopt;
			igmp_snp_syslog_pkt_rev("type %d reservedc %d cksum %d reserveds %d naddrc %d\n"\
									"dest group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",	\
									v2_report->type,v2_report->reservedf,v2_report->checksum,v2_report->reserveds,\
									v2_report->naddrc,ADDRSHOWV6(v2_report->mldv2_rec.group));
		}
	}
	else{
			igmp_snp_syslog_pkt_rev("type %d code %d cksum %d\n"
									"group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
									mldhd->type,mldhd->code,mldhd->cksum,ADDRSHOWV6(mldhd->group));
	}
	igmp_snp_syslog_pkt_rev("*****************packet over*******************\n");
	// give out detailed packet byte stream
	igmp_snp_syslog_pkt_rev(msg_skb->buf, (msg_skb->nlh.nlmsg_len - sizeof(msg_skb->nlh)));

	return;
}


/*******************************************************************************
 * mld_debug_print_groupvlan
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
void mld_debug_print_groupvlan(MC_group_vlan *pvlan)
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
		igmp_snp_syslog_err("mld_debug_print_groupvlan:vlan is NULL.\n");
		return;
	}
	igmp_snp_syslog_dbg("@@@@@@@@@@@@@@@@@@@Vlan data@@@@@@@@@@@@@@@@@@@@\n");
	igmp_snp_syslog_dbg("next:%d prev:%d vlan_id:%d"\
						"last_requery_addr:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",\
						pvlan->next,pvlan->prev,\
						pvlan->vlan_id,\
						ADDRSHOWV6(pvlan->saddrv6));
	igmp_snp_syslog_dbg("querytimeinterval:%d querytimer_id:%d vlanlife:%d\n",\
						pvlan->querytimeinterval,pvlan->querytimer_id,\
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
		igmp_snp_syslog_dbg("MC_ipadd:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x \n"\
							" report_ipadd:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",\
							ADDRSHOWV6(t_group->MC_v6ipadd),
							ADDRSHOWV6(t_group->report_v6ipadd));			
		igmp_snp_syslog_dbg("vlan_id:%d version:%s McGroup_id:%d\n",\
							t_group->vid,\
							(t_group->ver_flag?"MLD V2":"MLD V1"),\
							t_group->mgroup_id);
		t_port = pvlan->firstgroup->portstatelist;
		igmp_snp_syslog_dbg("-------------------MC_ports---------------------\n");
		while( NULL != t_port )
		{
			igmp_snp_syslog_dbg("ifindex:%d state:0x%x\n",t_port->ifindex,t_port->state);
			t_port = t_port->next;
		}
		t_group = t_group->next;
	}
	igmp_snp_syslog_dbg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
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
INT mld_enable_init(void)
{
	INT ret;	
	mld_snoop_pkt t_pkt;
	INT index = 0;
	int i =0;
	
	t_pkt.vlan_id = 0;
	for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
		t_pkt.groupadd[i] = 0;
	}
	t_pkt.ifindex = 0;
	t_pkt.group_id = 0;
	t_pkt.type = MLD_SYS_SET_INIT;		/*init*/

	igmp_snp_syslog_event("START:mld_enable_init\n");

	if( IGMPSNP_RETURN_CODE_OK != mld_snp_mod_addr(&t_pkt,MLD_SYS_SET))
	{
		igmp_snp_syslog_err("mld_snp_mod_addr: failed in set port pfm.\n");
		goto error;
	}

	if( IGMPSNP_RETURN_CODE_OK != init_igmp_snp_timer())
	{
		igmp_snp_syslog_err("init_mld_snp_timer: fail in set global timer.\n");
		goto error;
	}

	// init mcgroup_vlan_queue
	for (index = 0; index < IGMP_GENERAL_GUERY_MAX; index++) {
		mcgroup_vlan_queue[index] = NULL;
	}

	igmp_snp_syslog_event("END:mld_enable_init\n");
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
*	stop IGMP SNOOPING£¬release data
*
***********************************************************************************/
INT mld_snp_stop(VOID)
{			
	INT tmp = 0;
	INT i,ret = IGMPSNP_RETURN_CODE_OK;
	ULONG ulRet = 0;//usIfIndex = 0;
	igmp_routerport *pRoutePort = NULL;
	MC_group_vlan *tvlan = NULL;
	mld_snoop_pkt t_pkt;
	#if 0
	if(MLD_SNP_ISENABLE())/*before call mld_snp_stop,the mld_snp_isenable already set 0*/
	{
		igmp_snp_syslog_dbg("Fail to stop MLD snooping:task is not started.\n");
		return IGMPSNP_RETURN_CODE_OK;
	}
	#endif
	
	t_pkt.vlan_id = 0;
	t_pkt.group_id = 0;
	for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
		t_pkt.groupadd[i] = 0;
	}
	t_pkt.ifindex = 0;
	t_pkt.type = MLD_SYS_SET_STOP;	

	if( IGMPSNP_RETURN_CODE_OK != mld_snp_mod_addr(&t_pkt,MLD_SYS_SET))
	{
		igmp_snp_syslog_dbg("mld_snp_mod_addr:failed in set port pfm.\n");
		goto error;
	}

	/*delete timer*/
	if(igmp_timer_list.cnt)
	{
		if( IGMPSNP_RETURN_CODE_OK != del_all_timer(&igmp_timer_list))
		{
			igmp_snp_syslog_dbg("mld_timer_delete:fail in delete global timer.\n");
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
	mld_snoop_enable = IGMP_SNOOP_NO;

	return IGMPSNP_RETURN_CODE_OK;

error:
	mld_snoop_enable = IGMP_SNOOP_YES;
	return IGMPSNP_RETURN_CODE_ERROR;
}


/*******************************************************************************
 * mld_snp_addr_check
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
 *		2. available address  
 **
 ********************************************************************************/
 INT mld_snp_addr_check( USHORT * group )
{
	/* range FF00:: ~ FF7F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF */
	if ( ( group[0] < 0xFF00 ) || ( group[0] > 0xFF7F ) )
	{
		igmp_snp_syslog_dbg("check group address, Invalid group address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", ADDRSHOWV6(group));
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	/* range FF00:: ~ FF0F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF */
	if ( ( group[0] >= 0xFF00 ) && ( group[0] < 0xFF10 ) )
	{
		igmp_snp_syslog_dbg("check group address, reserved group address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", ADDRSHOWV6(group));
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}

	igmp_snp_syslog_dbg("check group address: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, is in available range.\n", ADDRSHOWV6(group));
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
LONG mld_snp_delgroup( MC_group_vlan *pVlan, MC_group * pGroup, MC_group **ppNextGroup )
{
	/*ULONG ulTimerId = 0;*/
	mld_snoop_pkt pkt ;
	struct MC_port_state *pstPort = NULL;
	struct MC_port_state *nextPort = NULL;
	int i = 0;

	igmp_snp_syslog_dbg("START:igmp_snp_delgroup\n");

	
	if ( NULL == pGroup || NULL == pVlan)
	{
		igmp_snp_syslog_err("parameter is null pointer!\n");
		igmp_snp_syslog_dbg("END:mld_snp_delgroup\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	*ppNextGroup = pGroup->next;

	pkt.vlan_id = pGroup->vid;
	USHORT *pstIp6 = pkt.groupadd;
	mld_copy_ipv6addr(pGroup->MC_v6ipadd, &(pstIp6));
	pkt.type = MLD_ADDR_RMV;
	pkt.ifindex = 0;
	pkt.group_id = pGroup->mgroup_id; //by wujh
	
	igmp_snp_syslog_dbg("McGroupVlan %d,McGroupVidx %d,",\
						"McGroupAddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
					pVlan->vlan_id,pGroup->mgroup_id,ADDRSHOWV6(pGroup->MC_v6ipadd));
	/* remove group from hardware table*/
	if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_mod_addr((VOID*)&pkt,MLD_ADDR_RMV)))
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
	if ( mld_groupcount > 0 )
	{
		mld_groupcount--;
	}

	igmp_snp_syslog_dbg("END:igmp_snp_delgroup\n");
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
LONG mld_snp_del_mcgroupvlan( LONG vlan_id )
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

	igmp_snp_syslog_dbg("START:mld_snp_del_mcgroupvlan\n");

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
				mld_snp_delgroup( pVlan, pMcGroup, &pNext);
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

	igmp_snp_syslog_dbg("END:mld_snp_del_mcgroupvlan\n");
	return IGMPSNP_RETURN_CODE_OK;
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

LONG mld_snp_groupcheck( LONG lVid )
{
	ULONG ulRet = 0;

	igmp_vlanisused( lVid, &ulRet );
	/* if lVid does not exist, delete all groups belongs to this vlan node */
	if ( 0 == ulRet )
	{
		igmp_snp_syslog_dbg("delete all groups belongs to this vlan %d\n.", lVid);
		mld_snp_del_mcgroupvlan( lVid );
		return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
	}

	return IGMPSNP_RETURN_CODE_OK;
}





/**************************************************************************
* mld_creat_group_node()
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
LONG mld_creat_group_node( LONG lVid, USHORT * ulGroup, ULONG usIfIndex,
					MC_group ** ppMcGroup, MC_group_vlan ** ppPrevGroupVlan )
{
	int tmp;
	MC_group_vlan * pVlanNode = NULL;
	MC_group *pGroup = NULL;
	USHORT usIsCreate = IGMP_SNOOP_NO;   /* 0 - indicate the vlan node is created. 1 - new vlan node */
	ULONG ulSlot = 0;
	ULONG ulMaxGroup = 0;
	ULONG ulGroup_id = 0;
	
	igmp_snp_syslog_dbg("create new group: vid %d group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x If %d pVlan 0x%p\n",
						lVid, ADDRSHOWV6(ulGroup), usIfIndex, (*ppPrevGroupVlan));

	igmp_getmaxgroupcnt(usIfIndex,&ulMaxGroup);
	/* if the total group count is more than the max group number supported, return error*/
	if ( mld_groupcount > ulMaxGroup )
	{
		igmp_snp_syslog_err("Group full! MaxGroup %d, GroupCount %d, lVid %d usIfIndex %d\n",
							ulMaxGroup, mld_groupcount, lVid, usIfIndex);
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
	USHORT *pstIp6 = pGroup->MC_v6ipadd;
	mld_copy_ipv6addr(ulGroup, &pstIp6);
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
								pRouter->ifindex, pRouter->vlan_id, pRouter->routeport_saddrv6);
			igmp_router_entry * Igmp_snoop_router_temp = NULL;
			Igmp_snoop_router_temp = ( igmp_router_entry * )malloc( sizeof( igmp_router_entry ));
			if ( NULL == Igmp_snoop_router_temp )
			{
				igmp_snp_syslog_err("igmp router entry alloc memory failed.\n");
				return IGMPSNP_RETURN_CODE_NULL_PTR;
			}
			memset( Igmp_snoop_router_temp,0, sizeof( igmp_router_entry ) );  

			USHORT *pstIp6 = Igmp_snoop_router_temp->saddrv6;
			mld_copy_ipv6addr(pRouter->routeport_saddrv6, &(pstIp6));
			Igmp_snoop_router_temp->mroute_ifindex = pRouter->ifindex;
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
		if (tmp == 0) {							// add first mcgroup vlan, set index
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
	mld_groupcount++;

	igmp_searchvlangroup_by_groupcount(&ulGroup_id);
	pGroup->mgroup_id = ulGroup_id;
	igmp_snp_syslog_dbg("igmp create group_node: groupAddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,groupId %d.\n",\
						ADDRSHOWV6(pGroup->MC_v6ipadd),pGroup->mgroup_id);
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
LONG mld_snp_searchvlangroup
(
	LONG lVid,
	unsigned short *ulGroup,
	MC_group **ppMcGroup,
	MC_group_vlan **ppPrevGroupVlan
)
{
	MC_group_vlan *pCurrent = NULL;
	MC_group *pGroupTemp = NULL;

	igmp_snp_syslog_event("find multicast-group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x of vlan %d in mcgroup_vlan_queue.\n",\
						  ADDRSHOWV6(ulGroup), lVid );

	pCurrent = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);/*macro return a pointer*/
	//pCurrent = GET_VLAN_POINT_BY_INDEX(0);
	*ppPrevGroupVlan = pCurrent;

	/* check vlan */
	if ( 0 == lVid )
	{
		igmp_snp_syslog_warn("find multicast-group with vlan %d doesn't exist.\n", lVid );
	}

	if ( IGMPSNP_RETURN_CODE_OK != mld_snp_groupcheck( lVid ) )
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
			while ( NULL != pGroupTemp ){
				if(MLD_RETURN_CODE_EQUAL != mld_compare_ipv6addr(pGroupTemp->MC_v6ipadd, ulGroup)){
					pGroupTemp = pGroupTemp->next;
					continue;
				}
				else{
					igmp_snp_syslog_dbg("found multicast-group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,\n"\
										"IpAddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
										ADDRSHOWV6(ulGroup), ADDRSHOWV6(pGroupTemp->MC_v6ipadd));			
					*ppMcGroup = pGroupTemp;/*find Both mcGroupVLan & mcGroup*/
					return IGMPSNP_RETURN_CODE_OK;
				}
			}

			igmp_snp_syslog_dbg("no found multicast-group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", ADDRSHOWV6(ulGroup));
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



/*************************************************
 *proccess pakcet flag :IGMP_SNP_FLAG_PKT_UNKNOWN
 ************************************************
 *TODO :just parse ipheader in packet,
		but not parse the further data in packet.
 ************************************************/
/**********************************************************************************
*mld_recv_pktdata_proc()
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
LONG mld_recv_pktdata_proc
(
	struct igmp_skb *msg_skb
)
{
	USHORT	*tmp;
	struct iphdr * pIphd = NULL;
	struct ipv6hdr * pIpv6hd = NULL;
	USHORT * ulGroup = NULL;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	LONG lVid = msg_skb->vlan_id;/* format msg_skb --wujh@autelan.com 08/09/23*/
	ULONG usIfIndex = msg_skb->ifindex;
	
	igmp_snp_syslog_dbg("Start for mld unknow skb handle!\n");
	if( !msg_skb )
	{
		igmp_snp_syslog_err("unknown type igmp packet with null skb, drop!\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	tmp = (USHORT *)(msg_skb->buf + 12 );
	if( 0x8100 == *tmp ) /*packet vlan tagged*/
		pIpv6hd = (struct ipv6hdr *)(msg_skb->buf  + 18);
	else
		pIpv6hd = (struct ipv6hdr *)(msg_skb->buf  + 14);

	ulGroup = pIpv6hd->daddr.s6_addr16;/*NOT always GroupIp just is daddr*/

	if ( pIpv6hd->hop_limit < 1 )
	{
		igmp_snp_syslog_err("unknown mld type packet hop_limit %d illegal, drop!\n", pIpv6hd->hop_limit);
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	lRet = mld_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan );
	if ( IGMPSNP_RETURN_CODE_OK != lRet )
	{
		igmp_snp_syslog_err("find group %X4:%X4:%X4:%X4:%X4:%X4:%X4:%X4 in vlan %d failed\n",ADDRSHOWV6(ulGroup),lVid);
		return IGMPSNP_RETURN_CODE_GROUP_NOTEXIST;
	}
	if ( NULL != pMcGroup )
	{
		return IGMPSNP_RETURN_CODE_OK;
	}
	else
	{
		if ( IGMPSNP_RETURN_CODE_OK != mld_creat_group_node( lVid, ulGroup, usIfIndex, &pMcGroup, &pPrevGroupVlan ) )
		{
			igmp_snp_syslog_err("create new group failed. pVlan 0x%x \n", pPrevGroupVlan);
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	
	/* add address to hardware */
	mld_snoop_pkt stPkt ;

	stPkt.vlan_id = lVid;
	stPkt.group_id = pMcGroup->mgroup_id;/*original is ZERO*/
	USHORT *pstIp6 = stPkt.groupadd;
	mld_copy_ipv6addr(ulGroup, &(pstIp6));
	stPkt.ifindex = usIfIndex;
	stPkt.type = IGMP_ADDR_ADD;

	lRet = mld_snp_mod_addr( &stPkt, IGMP_ADDR_ADD );
	if ( IGMPSNP_RETURN_CODE_OK != lRet )
	{
		igmp_snp_syslog_dbg("add address to hardware failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR_HW;
	}


	/* add group member timer */
	igmp_snp_addintertimer( pMcGroup->grouplifetime, &( pMcGroup->grouplifetimer_id ) );
	return IGMPSNP_RETURN_CODE_OK;
}


/*******************************************************************************
 * mld_snp_file_option_field
 *
 * DESCRIPTION:
 *	 mld_snp_file_option_field() fill ip option field:
 *		[0] - option type
 *		[1] - option length,unit is 8 bytes and first 8 bytes is not in count
 *		[2]-[5] - ipv6 router alert
 *		[6] - padN type
 *		[7] - padN length
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
VOID mld_snp_file_option_field( UCHAR * pOptionBuf, ULONG ulLength )
{
	if ( ( !pOptionBuf )|| ( ulLength != MLD_IPV6_OPTION_LEN ) )
		return ;

	pOptionBuf[0] = 0x3A; /*option type*/
	pOptionBuf[1] = 0x00;/*option length,unit is 8 bytes and first 8 bytes is not in count*/
	pOptionBuf[2] = 0x05;/*ipv6 router alert*/
	pOptionBuf[3] = 0x02;
	pOptionBuf[4] = 0x00;
	pOptionBuf[5] = 0x00;
	pOptionBuf[6] = 0x01;/*padN type*/
	pOptionBuf[7] = 0x00;/*padN length*/
}

/*******************************************************************************
 * mld_Snoop_Send_mld
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
LONG mld_Snoop_Send_mld( mld_snoop_pkt * pPkt )
{
	UINT	i, ethlen;
	ULONG taged = 0;
	struct ether_header_t *pEthhd = NULL;
	struct ipv6hdr * pIphd = NULL;
	struct mld *pMld = NULL;
	struct mldv2 *pMldv2 = NULL;
	ULONG unVlanIf;
	LONG lRet;
	LONG lVid = 0,unIfIndex = 0;	/*ULONG --> LONG: lead to fatal loop! 08/08/23@wujh*/
	UCHAR *pBuf = NULL;		/* IP PDU */
	ULONG ulBufLen = 0;
	ULONG  ulIpMask;
	USHORT ulIpAddr[SIZE_OF_IPV6_ADDR] = {0};
	USHORT usIpv6Addr[SIZE_OF_IPV6_ADDR] = {0};
	ULONG ulportstatus;
	ULONG nextifindex;
	igmp_queryport *PTemp = NULL;
	USHORT all_system_addr[SIZE_OF_IPV6_ADDR] = {0xFF02,0x0,0x0,0x0,0x0,0x0,0x0,0x01};
	USHORT all_route_addr[SIZE_OF_IPV6_ADDR] = {0xFF02,0x0,0x0,0x0,0x0,0x0,0x0,0x02};
	USHORT all_mldv2_route_addr[SIZE_OF_IPV6_ADDR] = {0xFF02,0x0,0x0,0x0,0x0,0x0,0x0,0x16};
	USHORT *pstIp6 = NULL;
	
	igmp_snp_syslog_dbg( "\nEnter mld_Snoop_Send_mld: vlan %d, group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, type %d, If %d\n",
								pPkt->vlan_id, ADDRSHOWV6(pPkt->groupadd), pPkt->type, pPkt->ifindex);

	unIfIndex = pPkt->ifindex;
	lVid = pPkt->vlan_id;
	/*for igmp report/leave packet , need add ip option: router alert*/

	/******************/
	/* Set ulBufLen's value */
	/******************/
	switch ( pPkt->type )
	{
		case MLD_MSG_GEN_QUERY:
		case MLD_MSG_GS_QUERY:
		case MLD_MSG_REPORT:
		case MLD_MSG_LEAVE:
			ulBufLen = sizeof( struct ipv6hdr ) + sizeof( struct mld ) + MLD_IPV6_OPTION_LEN;
			break;
		default:
			ulBufLen = sizeof( struct ipv6hdr ) + sizeof( struct mldv2 ) + MLD_IPV6_OPTION_LEN;
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

	/*the 8 bytes is vlan_id + ifindex in igmp_skb*/
	pBuf = ( UCHAR * )malloc( ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr)); 
	if ( NULL == pBuf ) 
	{
		igmp_snp_syslog_dbg("mld_Snoop_Send_Igmp: alloc mem for pBuf failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset( pBuf,0, (ethlen + ulBufLen + 8 + sizeof(struct nlmsghdr)) );

	pEthhd = (struct ether_header_t* )(pBuf +8 +sizeof(struct nlmsghdr));

	pIphd = ( struct ipv6hdr * ) (pBuf + 8 + ethlen + sizeof(struct nlmsghdr));
	/*for mld report/leave packet , need add ip option: router alert*/

	/*****************/
	/* Set pMld's value  */
	/*****************/
	switch ( pPkt->type )
	{
		case MLD_MSG_GEN_QUERY:
		case MLD_MSG_GS_QUERY:			
		case MLD_MSG_REPORT:
		case MLD_MSG_LEAVE:
			mld_snp_file_option_field( ( UCHAR * ) ( pIphd + 1 ), MLD_IPV6_OPTION_LEN );
			pMld = ( struct mld * ) ( ( UCHAR * ) ( pIphd + 1 ) + MLD_IPV6_OPTION_LEN );
			break;
		default:
			/*It seems that mld v2 report need add */
			mld_snp_file_option_field( ( UCHAR * ) ( pIphd + 1 ), MLD_IPV6_OPTION_LEN );
			pMldv2 = ( struct mldv2 * ) ( ( UCHAR * ) ( pIphd + 1 ) + MLD_IPV6_OPTION_LEN );
			break;
	}

	
	switch ( pPkt->type )
	{
		case MLD_MSG_GEN_QUERY:
			/* If a general query, send to all port except the router port in the pkt->ifindex */
			pMld->maxresp = igmp_resp_interval * IGMP_V2_SEC_2_MILL;
			for(i=0; i< SIZE_OF_IPV6_ADDR; i++){  /* general query */
				pMld->group[i] = 0;				
			}
			pMld->type = MLD_MEMSHIP_QUERY;
			/*pstIp6 = usIpv6Addr;
			mld_copy_ipv6addr(all_system_addr, &pstIp6);
			mld_htons( &pstIp6 );*/
			pstIp6 = pIphd->daddr.s6_addr16;
			mld_copy_ipv6addr(all_system_addr, &(pstIp6));
			break;
		case MLD_MSG_GS_QUERY:
			/* If a g-s query, send to the specific port */
			pMld->maxresp = igmp_rxmt_interval / IGMP_V2_TIME_SCALE;
			pMld->type = MLD_MEMSHIP_QUERY;
			/*pstIp6 = usIpv6Addr;
			mld_copy_ipv6addr(pPkt->groupadd, &pstIp6);
			mld_htons( &pstIp6 );*/
			pstIp6 = pIphd->daddr.s6_addr16;
			mld_copy_ipv6addr(all_system_addr, &(pstIp6));
			break;
		case MLD_MSG_REPORT:
			/* If a V1 report, send to the router port */
			pMld->type = MLD_V1_MEMSHIP_REPORT;
			/*pstIp6 = usIpv6Addr;
			mld_copy_ipv6addr(pPkt->groupadd, &pstIp6);
			mld_htons( &pstIp6 );*/
			pstIp6 = pIphd->daddr.s6_addr16;
			mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
			break;
		case MLD_MSG_V2_REPORT:
			/* If a V2 report, send to the router port */
			pMldv2->type = MLD_V2_MEMSHIP_REPORT;
			/*pstIp6 = usIpv6Addr;
			mld_copy_ipv6addr(pPkt->groupadd, &pstIp6);
			mld_htons( &pstIp6 );*/
			pstIp6 = pIphd->daddr.s6_addr16;
			mld_copy_ipv6addr(all_mldv2_route_addr, &(pstIp6));
			break;
		case MLD_MSG_LEAVE:
			/* If a leave, send to the router port */
			pMld->type = MLD_V1_LEAVE_GROUP;
			/*pstIp6 = usIpv6Addr;
			mld_copy_ipv6addr(all_route_addr, &pstIp6);
			mld_htons( &pstIp6 );*/
			pstIp6 = pIphd->daddr.s6_addr16;
			mld_copy_ipv6addr(all_route_addr, &(pstIp6));
			break;
		default:
			break;
	}


	if(MLD_MSG_V2_REPORT != pPkt->type){
		pMld->code = 0;
		pMld->cksum = 0;
		pMld->cksum = inet_cksum( ( USHORT * ) pMld, sizeof( struct mld ) );
		pstIp6 = pMld->group;
		mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
	}
	else{
		pMldv2->reservedf = 0;
		pMldv2->checksum = 0;
		pMldv2->checksum = inet_cksum( ( USHORT * ) pMldv2, sizeof( struct mldv2 ) );
		pMldv2->reserveds = 0;
		pMldv2->naddrc = 1;
		pstIp6 = pMldv2->mldv2_rec.group;
		mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
		pMldv2->mldv2_rec.rec_type = 0x04;
		pMldv2->mldv2_rec.rec_auxlen = 0;
		pMldv2->mldv2_rec.numofsrc = 1;
	}
	/* generate IP header */
	pIphd->version = MLD_VERSION;
	pIphd->priority = MLD_PRIORITY;
	pIphd->flow_lbl[0] = MLD_FLOW_LABEL;	
	pIphd->flow_lbl[1] = MLD_FLOW_LABEL;
	pIphd->flow_lbl[2] = MLD_FLOW_LABEL;
	pIphd->nexthdr = 0;      /*add the option need to set nexthdr 0*/
	pIphd->hop_limit = MLD_HOP_LIMIT; /* MLD packet HOP_LIMIT = 1 */

	/*for igmp report/leave packet , need add ip option: router alert*/
	switch ( pPkt->type )
	{
		case MLD_MSG_GEN_QUERY:
		case MLD_MSG_GS_QUERY:
		case MLD_MSG_REPORT:
		case MLD_MSG_LEAVE:
			pIphd->payload_len = sizeof( struct mld ) + MLD_IPV6_OPTION_LEN;
			break;
		default:
			/*maybe the mldv2 report set*/
			pIphd->payload_len = sizeof( struct mldv2 ) + MLD_IPV6_OPTION_LEN;
			break;
	}
	pstIp6 = ulIpAddr;
	mld_copy_ipv6addr(pIphd->daddr.s6_addr16, &pstIp6);
	pEthhd->dmac[0] = 0x33;
	pEthhd->dmac[1] = 0x33;
	pEthhd->dmac[2] = (unsigned char)((ulIpAddr[6]>>8) & 0xFF);
	pEthhd->dmac[3] = (unsigned char)(ulIpAddr[6] & 0xFF);
	pEthhd->dmac[4] = (unsigned char)((ulIpAddr[7]>>8) & 0xFF);
	pEthhd->dmac[5] = (unsigned char)(ulIpAddr[7] & 0xFF);

	if(!igmp_get_macaddr){
		/**/
		pEthhd->smac[0] = 0x33;
		pEthhd->smac[1] = 0x33;
		pEthhd->smac[2] = 0x00;
		pEthhd->smac[3] = 0x00;
		pEthhd->smac[4] = 0x00;
		pEthhd->smac[5] = 0x01;
	}
	else {
		memcpy(pEthhd->smac,sysMac,MACADDRLENGTH);
	}

	pEthhd->etherType = 0x86DD;

	/* Get vlan's IP address. If no ip addr, use 0.0.0.0 as SIP */
#if 0
	SYS_CREATE_VLAN_IF( unVlanIf, pPkt->lVid );
	lRet = IFM_Get3LayerIpAddr( unVlanIf, &ulIpAddr, &ulIpMask, 0 );
#endif
	lRet = mld_getvlan_addr(pPkt->vlan_id,ulIpAddr);
	if ( lRet != IGMPSNP_RETURN_CODE_OK )
	{
		pstIp6 = pIphd->saddr.s6_addr16;
		mld_copy_ipv6addr(pPkt->saddr, &(pstIp6));
		igmp_snp_syslog_dbg("Igmp_Snoop_Send_Igmp:: pPkt->saddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"\
							"---> pIphd->saddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",
			ADDRSHOWV6(pPkt->saddr),ADDRSHOWV6(pIphd->saddr.s6_addr16));
	}
	else
	{
		pstIp6 = ulIpAddr;
		mld_htons( &pstIp6 );
		pstIp6 = pIphd->saddr.s6_addr16;
		mld_copy_ipv6addr(ulIpAddr, &(pstIp6));
	}

	/* If it is the general query, should send packet to vlan except the router port
	If the other types, send the packet directly to unIfIndex.
	*/
	if ( pPkt->type == MLD_MSG_GEN_QUERY )/* general query, send pkt to all port except the unIfIndex port */
	{
		igmp_snp_syslog_dbg("mld_Snoop_Send_mld::Send General query.\n");

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
			}
			else unIfIndex = -1;
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




/*******************************************************************************
 * mld_snp_routerleave
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
LONG mld_snp_routerleave( LONG lVid, ULONG usIfIndex, USHORT * ulGroup, USHORT * ulSaddr )
{
	mld_snoop_pkt stPkt;

	igmp_snp_syslog_dbg("send leave message to router: lVid %d, usIfIndex %d,",\
							"ulGroup %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
	      				lVid, usIfIndex, ADDRSHOWV6(ulGroup));	

	stPkt.vlan_id= lVid;
	stPkt.ifindex = usIfIndex;
	USHORT *pstIp6 = stPkt.groupadd;
	mld_copy_ipv6addr(ulGroup, &(pstIp6));
	stPkt.type = MLD_MSG_LEAVE;
	pstIp6 = stPkt.saddr;
	mld_copy_ipv6addr(ulSaddr, &(pstIp6));
	
	return mld_Snoop_Send_mld( &stPkt );
}





/**************************************************************************
* mld_Event_GroupMember_Timeout
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
LONG mld_Event_GroupMember_Timeout( MC_group_vlan *p_vlan,MC_group *p_group,
							ULONG ifindex)
{ 
	struct MC_port_state *t_port = NULL;
	mld_snoop_pkt 	stPkt;

	igmp_snp_syslog_dbg( " mld_Event_GroupMember_Timeout handle.\n") ;
	if ( !p_group ||!p_vlan)
	{
		igmp_snp_syslog_dbg( " mld parameter null pointer.\n") ;
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	/* if the group membership timer is timeout, delete the port from member list
	*/
	igmp_snp_searchportstatelist( &( p_group->portstatelist ), ifindex,
						IGMP_PORT_DEL, &t_port );

	/* del L2 L3 MC address */
	stPkt.vlan_id= p_vlan->vlan_id;
	stPkt.ifindex = ifindex;
	stPkt.type = MLD_ADDR_DEL;
	USHORT *pstIp6 = stPkt.groupadd;
	mld_copy_ipv6addr(p_group->MC_v6ipadd, &(pstIp6));
	stPkt.group_id = p_group->mgroup_id; 
	if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_mod_addr (&stPkt,MLD_ADDR_DEL) ) )
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
* mld_Event_GroupLife_Timeout()
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
LONG mld_Event_GroupLife_Timeout( MC_group_vlan *p_vlan, MC_group *p_group )
{
	MC_group * pNextGroup = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;

	igmp_snp_syslog_dbg("START:Mld_Event_GroupLife_Timeout\n");

	if ( !(p_group) || !(p_vlan) )
	{
		igmp_snp_syslog_err("mld parameter NULL pointer.\n");
		igmp_snp_syslog_dbg("END:mld_Event_GroupLife_Timeout\n");	
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	/* delete group node */
	lRet = mld_snp_delgroup( p_vlan, p_group, &pNextGroup );
	if( lRet != IGMPSNP_RETURN_CODE_OK)
	{
		igmp_snp_syslog_dbg("END:MLD_Event_GroupLife_Timeout\n");
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
		if( IGMPSNP_RETURN_CODE_OK == mld_snp_del_mcgroupvlan( p_vlan->vlan_id)){
			p_vlan = NULL;
		}
	}
	
	igmp_snp_syslog_dbg("END:mld_Event_GroupLife_Timeout\n");
	return lRet;
}



/**************************************************************************
* mld_Event_GenQuery_Timeout()
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
LONG mld_Event_GenQuery_Timeout( MC_group_vlan *p_vlan )
{
	MC_group_vlan *pPrevGroupVlan = NULL;
	mld_snoop_pkt stData;
	struct igmp_router_entry * Igmp_snoop_router_temp;  /*zgm add router port */
	time_t timep ;
	memset(&timep,0,sizeof(time_t));
	char *timebuf = NULL;
	int i = 0;
	
	timebuf = ctime( &timep);
	igmp_snp_syslog_dbg( ( "\n now handle GenQuery Timeout, System corrent time: %s. \n",timebuf ) );
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
			for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
				stData.groupadd[i] = 0;
			}
			stData.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
			USHORT *pstIp6 = stData.saddr;
			USHORT *temIp6 = Igmp_snoop_router_temp->saddrv6;
			mld_copy_ipv6addr(temIp6, &(pstIp6));
			stData.type = MLD_MSG_GEN_QUERY;

			if ( IGMPSNP_RETURN_CODE_OK != ( mld_Snoop_Send_mld( &stData ) ) )
			{
				igmp_snp_syslog_dbg( ( "mld GenQuery Timeout, failed in sending query packet\n" ) );
			}

			Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
		}
	}
	else
	{
		/*send general skb*/
		stData.vlan_id= p_vlan->vlan_id;
		for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
			stData.groupadd[i] = 0;
		}
		stData.ifindex = 0xffff;/*non-exist port in system*/
		for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
			stData.groupadd[i] = 0;
		}
		stData.type = MLD_MSG_GEN_QUERY;

		if ( IGMPSNP_RETURN_CODE_OK != ( mld_Snoop_Send_mld( &stData ) ) )
		{
			igmp_snp_syslog_dbg( ( "GenQuery Timeout, failed in sending query packet\n" ) );
		} 
	}
	return IGMPSNP_RETURN_CODE_OK;
}


/**************************************************************************
* mld_Event_Rxmt_Timeout()
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
VOID mld_Event_Rxmt_Timeout( timer_element *cur )
{
	mld_snoop_pkt * pPkt = NULL;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	unsigned int lRet;
	ULONG nextifindex;
	ULONG ulTimerId = 0;
	struct MC_port_state *pstPort = NULL;
	mld_snoop_pkt stPkt;
	igmp_router_entry * Igmp_snoop_router_temp;

	igmp_snp_syslog_dbg("\nEnter mld_Event_Rxmt_Timeout. \n");

	if( NULL == cur )
	{
		igmp_snp_syslog_dbg("mld_Event_Rxmt_Timeout: cur is null\n");
		return;
	}
	pPkt = (mld_snoop_pkt *)cur->data;
	if ( (NULL == pPkt)||(cur->datalen < sizeof(mld_snoop_pkt)) )
	{
		igmp_snp_syslog_dbg("mld_Event_Rxmt_Timeout: pPkt is null\n");
		return;
	}
	/* search group */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_searchvlangroup( pPkt->vlan_id, pPkt->groupadd, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_dbg("mld_Recv_Query: search failed\n");
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		return;
	}
	if ( NULL == pMcGroup )
	{
		igmp_snp_syslog_dbg(" mld_Event_Rxmt_Timeout: group:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x does not exist.\n", 
								ADDRSHOWV6(pPkt->groupadd));
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
		igmp_snp_syslog_dbg("#########mld_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg(" return by pstPort = NULL.\n");
		return;
	}

	if ( pstPort->state != MLD_SNP_CHECK_MEMBER )
	{
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
		igmp_snp_syslog_dbg("#########mld_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg(" return by pstPort->state != MLD_SNP_CHECK_MEMBER.\n");
		return;
	}

	/* if retransmit times >0 , send g-s query, reset rxmt timer, rxmtcount-- */
	if ( pPkt->retranscnt != 0 )
	{
		stPkt.vlan_id= pPkt->vlan_id;
		USHORT *pstIp6 = stPkt.groupadd;
		mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
		stPkt.type = MLD_MSG_GS_QUERY;
		stPkt.ifindex = pPkt->ifindex;
		pstIp6 = stPkt.saddr;
		mld_copy_ipv6addr(pPrevGroupVlan->saddrv6, &(pstIp6));
		if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_Snoop_Send_mld( &stPkt ) ) )
		{
			igmp_snp_syslog_dbg(" mld_Event_Rxmt_Timeout: send g-s query failed.\n");
		}

		pPkt->retranscnt --;
		igmp_snp_syslog_dbg("#########mld_Event_Rxmt_Timeout############\n");
		igmp_snp_syslog_dbg("pPkt->retransmitCount = %d.\n",pPkt->retranscnt);
		igmp_snp_syslog_dbg("#########mld_Event_Rxmt_Timeout############\n");
		if ( pPkt->retranscnt )
		{	/*continue timer*/
			igmp_snp_syslog_dbg("#########mld_Event_(mld_Event_Rxmt_Timeout)Rxmt_Timeout############\n");
			timer_element *new_timer = NULL;
			lRet = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
									igmp_rxmt_interval,
									(void *)mld_Event_Rxmt_Timeout,
									(void *)pPkt,sizeof(mld_snoop_pkt),
									&new_timer);
			if( lRet != IGMPSNP_RETURN_CODE_OK || NULL == new_timer )
			{
				igmp_snp_syslog_dbg("mld_recv_query:create timer failed.\n");
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
		if ( pstPort->state == MLD_SNP_CHECK_MEMBER )
		{
			stPkt.vlan_id= pPkt->vlan_id;
			USHORT *pstIp6 = stPkt.groupadd;
			mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
			stPkt.type = MLD_ADDR_DEL;
			stPkt.ifindex = pPkt->ifindex;
			stPkt.group_id= pPkt->group_id;
			
			if (IGMPSNP_RETURN_CODE_OK != (lRet = mld_snp_mod_addr(&stPkt,MLD_ADDR_DEL)))
			{
				igmp_snp_syslog_dbg("MLD_Event_Rxmt_Timeout: failed in setting L2 MC table. 0x%x \n", stPkt.ifindex);
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
						mld_snp_routerleave( pPkt->vlan_id, Igmp_snoop_router_temp->mroute_ifindex, pPkt->groupadd, pPkt->saddr );
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
						igmp_snp_syslog_dbg("mld_snp_flood: can not get ifindex by vlan_id.\n");
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
							mld_snp_routerleave( pPkt->vlan_id, unIfIndex, pPkt->groupadd, pPkt->saddr );
						}
						if(IGMPSNP_RETURN_CODE_OK == igmp_get_nextifindex_byifindex(pPkt->vlan_id,unIfIndex,&nextifindex))
									{
									  unIfIndex = nextifindex;
				  					}else unIfIndex = -1;
						//IFM_PORTONVLANEND
					}
				}
				igmp_snp_addintertimer( igmp_grouplife, &( pMcGroup->grouplifetimer_id ) );
				igmp_snp_syslog_dbg(" mld_Event_Rxmt_Timeout: add group life timer.%d\n", pMcGroup->grouplifetimer_id);
			}
			/* start Group life timer */
		}
		if( 0 != ulTimerId )
			del_timer(&igmp_timer_list, ulTimerId );
		free( pPkt );
		cur->data = NULL;
		cur->datalen = 0;
	}
	mld_debug_print_groupvlan(pPrevGroupVlan);
	return;
}



/**************************************************************************
* mld_Event_Resp_Timeout
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
VOID mld_Event_Resp_Timeout( timer_element *cur)
{
	mld_snoop_pkt * pPkt = NULL;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	ULONG unIfIndex;
	ULONG unRouterIfIndex;
	mld_snoop_pkt stPkt;
	MC_group_vlan *pMcVlan = NULL;
	igmp_router_entry * Igmp_snoop_router_temp = NULL;

	//unRouterIfIndex = pPkt->ifindex;/*???,move to line:2801*/

	igmp_snp_syslog_dbg("START:Igmp_Event_Resp_Timeout,timer Id =%d \n",cur->id);

	if( NULL == cur )
	{
		igmp_snp_syslog_dbg("cur is null\n");
		igmp_snp_syslog_dbg("END:Igmp_Event_Resp_Timeout\n");
		return;
	}
	pPkt = (mld_snoop_pkt *)cur->data;
	if ( (NULL == pPkt)||(cur->datalen < sizeof(mld_snoop_pkt)) )
	{
		igmp_snp_syslog_dbg("pPkt is null\n");
		igmp_snp_syslog_dbg("END:mld_Event_Resp_Timeout\n");
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
	if ( MLD_RETURN_CODE_EQUAL == mld_v6addr_equal_0(pPkt->groupadd) )    /* general query response timeout */
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
							USHORT *pstIp6 = stPkt.groupadd;
							mld_copy_ipv6addr(pMcGroup->MC_v6ipadd, &(pstIp6));

							stPkt.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
							stPkt.type = ( pMcGroup->ver_flag== IGMP_SNOOP_YES ) ? MLD_MSG_REPORT : MLD_MSG_V2_REPORT ;
							pstIp6 = stPkt.saddr;
							mld_copy_ipv6addr(pMcGroup->report_v6ipadd, &(pstIp6)); /*the last Port from which received report packet*/

							unIfIndex = stPkt.ifindex;

							igmp_snp_syslog_dbg("send report. vid %d,port_index %d,",\
								"sIP %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, groupIP %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x .\n",\
											pMcGroup->vid,unIfIndex,ADDRSHOWV6(pMcGroup->report_v6ipadd),ADDRSHOWV6(pMcGroup->MC_v6ipadd));

							if ( IGMPSNP_RETURN_CODE_OK != mld_Snoop_Send_mld( &stPkt ) )
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
		if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_searchvlangroup( pPkt->vlan_id, pPkt->groupadd, &pMcGroup, &pPrevGroupVlan ) ) )
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
						USHORT *pstIp6 = stPkt.groupadd;
						mld_copy_ipv6addr(pMcGroup->MC_v6ipadd, &(pstIp6));
						stPkt.ifindex = Igmp_snoop_router_temp->mroute_ifindex;
						stPkt.type = ( pMcGroup->ver_flag== IGMP_SNOOP_YES ) ? MLD_MSG_REPORT : MLD_MSG_V2_REPORT ;
						pstIp6 = stPkt.saddr;
						mld_copy_ipv6addr(pMcGroup->report_v6ipadd, &(pstIp6));
						igmp_snp_syslog_dbg("send report: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x  port 0x%x\n",
								ADDRSHOWV6(pMcGroup->MC_v6ipadd), stPkt.ifindex);

						if ( IGMPSNP_RETURN_CODE_OK != mld_Snoop_Send_mld( &stPkt ) )
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

	igmp_snp_syslog_dbg("END:mld_Event_Resp_Timeout\n");
	return;
}


/*******************************************************************************
 * mld_snp_flood
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
LONG mld_snp_flood( struct mld_info *sk_info, unsigned long lVid, ULONG usIfIndex )
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
	igmp_snp_syslog_dbg("data:pBuf = %p, lBufSize = %d,usIfIndex %d.\n", \
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
 * mld_snp_pim_msghandle
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
INT mld_snp_pim_msghandle( USHORT * ulGoup, ULONG ulIfindex, LONG lVid, USHORT * ulSrcAddr )
{
	LONG result = 0;
	ULONG trunkIfIndex = 0;      
	igmp_routerport *pRouter = NULL;
	MC_group *pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	igmp_router_entry * Igmp_snoop_router_temp;
	igmp_router_entry ** Igmp_snoop_router_pre;

	/* search vlan-group list, try to find a node */
	if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_searchvlangroup( lVid, ulGoup, &pMcGroup, &pPrevGroupVlan ) ) )
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

	if ( ( ulSrcAddr != NULL ) && ( pRouter == NULL ) )       /* no router port configured */
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
			USHORT *pstIp6 = Igmp_snoop_router_temp->saddrv6;
			mld_copy_ipv6addr(ulSrcAddr, &(pstIp6));
			Igmp_snoop_router_temp->mroute_ifindex = ulIfindex;
			Igmp_snoop_router_temp->next = NULL;
			igmp_snp_addintertimer( igmp_router_timeout, &( Igmp_snoop_router_temp->timer_id ) );
			*Igmp_snoop_router_pre = Igmp_snoop_router_temp;
			igmp_snp_syslog_dbg( "get a route port from pim hello, add router port vlan =%d,ifindex=%d\n", pPrevGroupVlan->vlan_id, ulIfindex);
		}
		else
		{
			USHORT *pstIp6 = ( *Igmp_snoop_router_pre )->saddrv6;
			mld_copy_ipv6addr(ulSrcAddr, &(pstIp6));
			igmp_snp_addintertimer( igmp_router_timeout, &( ( *Igmp_snoop_router_pre ) ->timer_id ) );
		}
	}
	return IGMPSNP_RETURN_CODE_OK;
}



/*******************************************************************************
* mld_pim_recv_msg
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
LONG mld_pim_recv_msg(struct igmp_skb *msg_skb)
{
	int i =0;
	struct mld_info *msg_info = NULL;
	struct ipv6hdr	*pIphd = NULL;
	pim_header_t *pPimphd = NULL;
	ULONG	ulPimLen;
	ULONG lRet = IGMPSNP_RETURN_CODE_OK;
	LONG	lVid = 0;
	ULONG ifindex = 0;
	USHORT ulGroup[SIZE_OF_IPV6_ADDR];
	USHORT ulSrcaddr[SIZE_OF_IPV6_ADDR];

	pIphd = (struct ipv6hdr *)(msg_skb->buf + 14); /*asic need the 8 bytes*/

	USHORT *pstIp6 = ulSrcaddr;
	mld_copy_ipv6addr(pIphd->saddr.s6_addr16, &(pstIp6));
	/*get PIM header*/
	pPimphd = ( struct pim_header * ) ( ( UCHAR * ) pIphd + sizeof(   struct ipv6hdr ) );

	igmp_snp_syslog_dbg("receive PIM packet: ver and type 0x%x,reserve 0x%x, cksum 0x%x 0x%x.\n",*(unsigned char *)pPimphd,*((unsigned char *)pPimphd + 1),*((unsigned char *)pPimphd + 2),*((unsigned char *)pPimphd + 3));
	igmp_snp_syslog_dbg("receive PIM packet: ver 0x%x,type 0x%x, cksum %d.\n",
	      				pPimphd->pim_vers, pPimphd->pim_type, pPimphd->pim_cksum);

	/*check pim checksum*/    
	ulPimLen = sizeof(struct pim_header );

	if ( 0 != ( lRet = inet_cksum( ( USHORT * ) pPimphd, ( ULONG ) ulPimLen ) ) )
	{
		igmp_snp_syslog_err("Pim packet checksum error. cksum: %d\n", lRet);
		return IGMPSNP_RETURN_CODE_ERROR_SW;
	}

	
	ifindex = (ULONG)(msg_skb->ifindex);
	lVid = (LONG)(msg_skb->vlan_id);
	/* distribute message */
	switch ( pPimphd->pim_type )
	{
		/* Pim Hello  packet process */
		case 0x0:          /*PIM_HELLO =0*/
			if ( pPimphd->pim_vers == 1 )
			{
				for(i; i<SIZE_OF_IPV6_ADDR; i++){
					ulGroup[i] = 0;
				}

			}
			else if ( pPimphd->pim_vers == 2 )
			{
				for(i; i<SIZE_OF_IPV6_ADDR; i++){
					ulGroup[i] = 0;
				}
			}
			else
			{
				igmp_snp_syslog_err("Pim packet vers %d error, not support.\n", pPimphd->pim_vers);
				return IGMPSNP_RETURN_CODE_ERROR;
			}
			igmp_snp_syslog_dbg("entry the pim msghandle: ulGroup %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,\n"\
				"ifindex %d lVid %d ulSrcaddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
				ADDRSHOWV6(ulGroup),ifindex,lVid,ADDRSHOWV6(ulSrcaddr));
			mld_snp_pim_msghandle( ulGroup, ifindex, lVid, ulSrcaddr );
			break;
		default:
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}



/*******************************************************************************
 * mld_snp_routerreport
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
LONG mld_snp_routerreport( LONG lVid, ULONG usIfIndex, USHORT *ulGroup, struct mld_info *sk_info )
{
	mld_snoop_pkt stPkt;
	struct ipv6hdr *pIphd = NULL;
	
	igmp_snp_syslog_dbg("send report message to router: lVid %d, usIfIndex %d, ulGroup %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
	      				lVid, usIfIndex, ADDRSHOWV6(ulGroup));
	
	pIphd = sk_info->ip_hdr;
	stPkt.vlan_id = lVid;
	stPkt.ifindex = usIfIndex;
	USHORT *pstIp6 = stPkt.groupadd;
	mld_copy_ipv6addr(ulGroup, &(pstIp6));
	stPkt.type = MLD_MSG_REPORT;
	pstIp6 = stPkt.saddr;
	mld_copy_ipv6addr(pIphd->saddr.s6_addr16, &(pstIp6));
	
	return mld_Snoop_Send_mld( &stPkt );
}


/**********************************************************************************
* mld_recv_report()
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
LONG mld_recv_report( ULONG usIfIndex, USHORT *ulGroup, LONG lVid, 
								ULONG ulType, struct mld_info *sk_info)
{	
	LONG result = 0;
	ULONG trunkIfIndex = 0;
	ULONG ulIfIndex = 0;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	struct MC_port_state *pstPort = NULL;
	struct ipv6hdr * pIphd = NULL;
	igmp_router_entry * Igmp_snoop_router_temp, *output;
	igmp_reporter_port *pReporter = NULL;
	member_port_list *pMemberPort = NULL;
	USHORT group_addr[SIZE_OF_IPV6_ADDR] = {0};
	USHORT group_tp[SIZE_OF_IPV6_ADDR] = {0};
	USHORT *pstIp6 = NULL;

	pstIp6 = group_addr;
	mld_copy_ipv6addr(ulGroup, &pstIp6);
	mld_htons(&pstIp6);
	
	pIphd = sk_info->ip_hdr;
	igmp_snp_syslog_event("START:mld_recv_report\n");

	igmp_snp_syslog_dbg("receive report packet,vlan_id:%d group:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x type:%d ifindex:0x%x\n",\
			lVid,ADDRSHOWV6(ulGroup),ulType,usIfIndex);

  	/* validate Group address */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_addr_check( ulGroup ) ) )/*not be some specific Ip addr.*/
	{
		igmp_snp_syslog_err( "receive report packet,group address invalid: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x \n", ADDRSHOWV6(ulGroup) );
		igmp_snp_syslog_dbg("END:mld recv report\n");
		return IGMPSNP_RETURN_CODE_OK;
	}

	/* get the multicast-groupVlan and multicast-group */
	if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_err( "search multicast-groupVlan or group failed.\n" );
		igmp_snp_syslog_dbg("END:mld recv report\n");
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
		if ( IGMPSNP_RETURN_CODE_OK != mld_creat_group_node( lVid, ulGroup, ulIfIndex, &pMcGroup, &pPrevGroupVlan ) )
		{
			igmp_snp_syslog_err("create group error. pVlan 0x%x\n", pPrevGroupVlan);
			igmp_snp_syslog_dbg("END:mld_recv_report\n");
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}
	/*we have Got the mcGroupVlan and group structure for the report packet.*/
	/*zgm add for if the first group report ,forward report to all router port*/
											/*??conflict with line 2885?*/
	if ( pMcGroup->portstatelist == NULL )
	{
		/*No port member in group,get router port in mcgroupvlan,send report to every router port. */
		igmp_snp_syslog_dbg("this is first member of group member-list, send report to router port.\n");
		if ( pPrevGroupVlan->routerlist != NULL )
		{
			igmp_snp_syslog_dbg("####loop for ouput the GroupVlan %d routerlist\n", pPrevGroupVlan->vlan_id);
			output = pPrevGroupVlan->routerlist;
			while (NULL != output)
			{
				igmp_snp_syslog_dbg("####port[%d], sddress[%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x] nextnode[%d]\n",\
									output->mroute_ifindex, ADDRSHOWV6(output->saddrv6), output->next);
				output = output->next;
			}
		
			igmp_snp_syslog_dbg("multicast-groupVlan's routlist is not NULL.\n");
			/*got router port in mcgroupvlan*/
			Igmp_snoop_router_temp = pPrevGroupVlan->routerlist;
			while ( Igmp_snoop_router_temp != NULL )
			{
				igmp_snp_syslog_dbg("Send report to Router port:vlan:%d group:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ifindex:0x%x\n",\
							ADDRSHOWV6(ulGroup),Igmp_snoop_router_temp->mroute_ifindex);
				mld_snp_routerreport( lVid, Igmp_snoop_router_temp->mroute_ifindex, ulGroup, sk_info );
				Igmp_snoop_router_temp = Igmp_snoop_router_temp->next;
			}
		}
		/*report packet can not send to Non-router port*/
		igmp_snp_addintertimer( 100, &( pMcGroup->router_reporttimer ) );
	}

	/*in the protocol,the ports in the same group only need to report one time to the route. 
	so if the reporterlist has port in same group,no need to add it  -- yangxs*/
	/*in the init design document contain the reporter,but it's not used factually*/
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

				if ( MLD_SNP_GROUP_NOMEMBER == pstPort->state )
				{
					mld_snoop_pkt stPkt ;

					stPkt.vlan_id = lVid;
					stPkt.group_id= pMcGroup->mgroup_id;/*It's the Vidx.Need a mechanism for assignment vidx!*/
					pstIp6 = stPkt.groupadd;
					mld_copy_ipv6addr(ulGroup, &(pstIp6));
					stPkt.ifindex = pMemberPort->ifindex;
					stPkt.type = MLD_ADDR_ADD;
					if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_mod_addr( &stPkt, MLD_ADDR_ADD ) ) )
					{
						igmp_snp_syslog_dbg( "receive report packet,failed in add address to hardware.\n");
						return IGMPSNP_RETURN_CODE_ERROR_HW;
					}
				}
				/* delete group life timer, ResponseSwitchTimer */
				IGMP_SNP_DEL_INTERTIMER( pMcGroup->grouplifetimer_id);

				/* Set port state */
				pstPort->state = MLD_SNP_HAS_MEMBER;

			}
			pMemberPort = pMemberPort->next;
		}
	}

	/* begin handle report message. handle the port - yangxs*/
	/* The difference between IGMP v1 and IGMP v2 report process is that :
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
	if ( MLD_SNP_GROUP_NOMEMBER == pstPort->state )
	{
		mld_snoop_pkt stPkt ;

		stPkt.vlan_id = lVid;
		stPkt.group_id= pMcGroup->mgroup_id;
		pstIp6 = stPkt.groupadd;
		mld_copy_ipv6addr(ulGroup, &(pstIp6));
		stPkt.ifindex = ulIfIndex;
		stPkt.type = MLD_ADDR_ADD;
		igmp_snp_syslog_dbg("Here to Call :MLD_snp_mod_addr.GroupId = %d!\n",pMcGroup->mgroup_id);
		if ( IGMPSNP_RETURN_CODE_OK != ( lRet = mld_snp_mod_addr( &stPkt, MLD_ADDR_ADD ) ) )
		{
			igmp_snp_syslog_dbg("mld_recv_report: failed in add address.\n");
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}

	/* Set port state */
		pstPort->state = MLD_SNP_HAS_MEMBER;

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
	igmp_snp_syslog_dbg("igmp_recv_report: start group member timer. %d\n", pstPort->membertimer_id);


	pIphd = sk_info->ip_hdr;
	pstIp6 = NULL;
	pstIp6 = group_tp;
	mld_copy_ipv6addr(pIphd->saddr.s6_addr16, &pstIp6);
	mld_ntohs(&pstIp6);
	pstIp6 = NULL;
	pstIp6 = pMcGroup->report_v6ipadd;
	mld_copy_ipv6addr(group_tp, &(pstIp6));

	/* output the multicast-groupVlan's detail */
	if(0 != (SYSLOG_DBG_PKT_ALL & igmp_snp_log_level)) {
		mld_debug_print_groupvlan(pPrevGroupVlan);
	}

	igmp_snp_syslog_dbg( "Out mld_recv_report.\n");
	return IGMPSNP_RETURN_CODE_OK;
}

/**********************************************************************************
*mld_recv_query()
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
INT mld_recv_query(ULONG ifindex, USHORT maxresptime, USHORT * group,
					LONG vlan_id, struct mld_info *sk_info)
{
	unsigned int result = IGMPSNP_RETURN_CODE_ERROR;
	ULONG t_ifindex = 0;
	ULONG trunkifindex = 0;
	struct ipv6hdr *ip_hdr = NULL;
	MC_group *t_mcgroup = NULL;
	MC_group_vlan *t_mcgroupvlan = NULL;

	igmp_routerport *t_router = NULL;
	igmp_router_entry *igmp_snp_router_temp;
	igmp_router_entry **igmp_snp_router_pre;

	/*if group !=0,query for specific mc group,
	  if group ==0,query for general mc group*/
	igmp_snp_syslog_event("receive query packet, vlan_id %d\n"\
						"group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x maxresptime %d ifindex %d\n",\
						vlan_id, ADDRSHOWV6(group), maxresptime, ifindex);

	/*mc group address check*/
	if( MLD_RETURN_CODE_EQUAL != mld_v6addr_equal_0(group)){	
		if( IGMPSNP_RETURN_CODE_OK != mld_snp_addr_check(group)){
			igmp_snp_syslog_warn("receive query packet,group address invalid %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", ADDRSHOWV6(group));
			return IGMPSNP_RETURN_CODE_ERROR;
		}
	}

	/*set queryRxFlag*/
	if(IGMP_SNOOP_NO == queryRxFlag){
		queryRxFlag = IGMP_SNOOP_YES;
	}
	
	/*search mc group*//*vlan_id,and the group(group_ip - D_class IP) as double index*/
	if( IGMPSNP_RETURN_CODE_OK != mld_snp_searchvlangroup(vlan_id,group,&t_mcgroup,&t_mcgroupvlan))
	{
		igmp_snp_syslog_err("search multicast-groupVlan %d and multicast-group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", \
							ADDRSHOWV6(group));
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
		mld_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}
	
	if( (t_mcgroupvlan->vlan_id != vlan_id)&&(MLD_RETURN_CODE_EQUAL == mld_v6addr_equal_0(group)))
	{
		igmp_snp_syslog_dbg("FLOOD:multicast-groupvlan->vlan_id[%d] != vlan_id[%d] && 0==group:ifindex:%d.\n",
							t_mcgroupvlan->vlan_id, vlan_id, t_ifindex);
		mld_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}
	
	if( (NULL == t_mcgroup)&&(MLD_RETURN_CODE_EQUAL == mld_v6addr_equal_0(group)) )/*mcgroup structure doesnot exists.*/
	{
		igmp_snp_syslog_dbg("FLOOD:multicast-group==NULL && 0==group: vlan_id:%d ifindex:%d\n",
							vlan_id, t_ifindex);
		mld_snp_flood(sk_info,vlan_id,t_ifindex);
		return IGMPSNP_RETURN_CODE_OK;
	}

	/*If a query message came from a multicasting router(it's SIP not 0.0.0.0)(server),
	* we should change the multicast router port to this port whether it is a designated port*/
	igmp_snp_searchrouterportlist(vlan_id,t_ifindex,IGMP_PORT_ADD,&t_router);
													/*IGMP_PORT_QUERY-modify by wujh 08/08/26/0:37*/
	ip_hdr = sk_info->ip_hdr;
	if((NULL != ip_hdr)&&(NULL !=t_router)){
		igmp_snp_syslog_dbg("Set router port: t_router@@%p,t_router->vlan_id %d, t_router->ifindex %d,",\
							t_router, t_router->vlan_id, t_router->ifindex);

		USHORT *pstIp6 = t_router->routeport_saddrv6;
		mld_copy_ipv6addr(ip_hdr->saddr.s6_addr16, &(pstIp6)); /*set router port->saddr*/
		igmp_snp_syslog_dbg("t_router->routeport_saddrv6 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",ADDRSHOWV6(t_router->routeport_saddrv6));

		
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
			USHORT *pstIp6 = igmp_snp_router_temp->saddrv6;
			mld_copy_ipv6addr(ip_hdr->saddr.s6_addr16, &(pstIp6)); 
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
			USHORT *pstIp6 = (*igmp_snp_router_pre)->saddrv6;
			mld_copy_ipv6addr(ip_hdr->saddr.s6_addr16, &(pstIp6)); 
			(*igmp_snp_router_pre)->mroute_ifindex = t_ifindex;/*08062602:01*/
			/*add router timer*/
			igmp_snp_addintertimer(igmp_router_timeout,&((*igmp_snp_router_pre)->timer_id));
		}
	}
	
	/*start response timer.(A random number between[0,maxresptime])*/
	if((NULL != t_mcgroup)&&(MLD_RETURN_CODE_EQUAL != mld_v6addr_equal_0(group)))
	{	
		/*query for this mcgroup (the group points to.)*/
		if(NULL != t_mcgroup->portstatelist)	/*only response when group has member*/
		{
			t_mcgroup->resposetime = (maxresptime>0)?(maxresptime * IGMP_V2_TIME_SCALE)
						:(IGMP_V2_QUERY_RESP_INTERVAL * 1000);
			t_mcgroup->resposetime = rand()%((LONG)t_mcgroup->resposetime);
			/*Add Resp Timer*/
			
			mld_snoop_pkt *pkt = NULL;
			timer_element *new_timer = NULL;

			pkt = (mld_snoop_pkt *)malloc(sizeof(mld_snoop_pkt));
			if(NULL == pkt )
			{
				igmp_snp_syslog_dbg("receive query packet, malloc timer data memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(pkt,0,sizeof(mld_snoop_pkt));
			pkt->vlan_id = vlan_id;
			USHORT *pstIp6 = pkt->groupadd;
			mld_copy_ipv6addr(group, &(pstIp6)); 
			pkt->type = IGMP_TIMEOUT_RESP;
			pkt->ifindex = t_ifindex;
			/*I Really Need pkt->group_id*/
			igmp_snp_syslog_dbg("create timer:resposetime!\n");
			result = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
									t_mcgroup->resposetime,
									(void *)mld_Event_Resp_Timeout,
									(void *)pkt,sizeof(mld_snoop_pkt),
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
			
			igmp_snp_syslog_dbg("receive query packet, start resp timer for g-s query: time:%d\n",
							t_mcgroup->resposetime);
			return IGMPSNP_RETURN_CODE_OK;
		}
	}
	else if((MLD_RETURN_CODE_EQUAL == mld_v6addr_equal_0(group))&&(t_mcgroupvlan->vlan_id== vlan_id))
	{
		/*query for general in this vlan.*/
		/*add Resp timer*/
		if( NULL != t_mcgroupvlan->firstgroup )
		{
			mld_snoop_pkt *pkt = NULL;
			timer_element *new_timer = NULL;
			ULONG resptime; 
			ULONG resptimer_id;

			resptime = (maxresptime>0)?(maxresptime * IGMP_V2_TIME_SCALE)
						:(IGMP_V2_QUERY_RESP_INTERVAL * 1000);
			resptime = rand()%((LONG)t_mcgroup->resposetime);
			igmp_snp_syslog_dbg("receive query packet, resp time 0x%x\n",resptime);
			pkt = (mld_snoop_pkt *)malloc(sizeof(mld_snoop_pkt));
			if(NULL == pkt )
			{
				igmp_snp_syslog_dbg("receive query packet, malloc timer data memory failed.\n");
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			memset(pkt,0,sizeof(mld_snoop_pkt));
			pkt->vlan_id = vlan_id;
			USHORT *pstIp6 = pkt->groupadd;
			mld_copy_ipv6addr(group, &(pstIp6)); /*Here :group = 0*/			
			pkt->type = IGMP_TIMEOUT_RESP;
			pkt->ifindex = t_ifindex;
			/*I Really Need pkt->group_id*/
			result = create_timer(TIMER_TYPE_NOLOOP, TIMER_PRIORI_NORMAL,
									t_mcgroup->resposetime,/*not be assigned any value.instead of resptime*/
									(void *)mld_Event_Resp_Timeout,
									(void *)pkt,sizeof(mld_snoop_pkt),
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
* mld_recv_leave()
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
LONG mld_recv_leave( ULONG usIfIndex, USHORT *ulGroup, LONG lVid,struct mld_info *sk_info )
{
	MC_group *pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	struct MC_port_state * pstPort = NULL;
	mld_snoop_pkt stPkt;
	struct ipv6hdr *pIphd = NULL;
	ULONG ulRxmtTimerId = 0;
	/*
	ULONG unIfIndex;
	*/
	unsigned int result = IGMPSNP_RETURN_CODE_OK;
	ULONG trunkIfIndex = 0,ulIfIndex = 0,ulVidx =0;
	USHORT *pstIp6 = NULL;

	igmp_snp_syslog_dbg("receive leave packet: vid %d group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x if %d.\n",
						lVid, ADDRSHOWV6(ulGroup), usIfIndex);

	if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_addr_check( ulGroup ) ) )
	{
		igmp_snp_syslog_dbg("receive leave packet: group address %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x invalid.\n",
							ADDRSHOWV6(ulGroup));
		return IGMPSNP_RETURN_CODE_OK;
	}

	/* check vlan and group (inlcuded in search group process ) */
	if ( IGMPSNP_RETURN_CODE_OK != ( mld_snp_searchvlangroup( lVid, ulGroup, &pMcGroup, &pPrevGroupVlan ) ) )
	{
		igmp_snp_syslog_err("search multicast-groupVlan %d and multicast-group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x failed.\n",
							lVid, ADDRSHOWV6(ulGroup));
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	if ( NULL == pMcGroup )
	{
		igmp_snp_syslog_dbg("receive leave packet: group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x does not exist.\n",
							ADDRSHOWV6(ulGroup));
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
	pstPort->state = MLD_SNP_CHECK_MEMBER;

	igmp_snp_syslog_dbg("igmp receive leave: %x state %d \n", usIfIndex, pstPort->state);

	/* send g-s query to this port */
	stPkt.vlan_id = lVid;
	pstIp6 = stPkt.groupadd;
	mld_copy_ipv6addr(ulGroup, &(pstIp6));
	stPkt.ifindex = ulIfIndex;
	stPkt.type = MLD_MSG_GS_QUERY;
	pIphd = sk_info->ip_hdr;
	pstIp6 = stPkt.saddr;
	mld_copy_ipv6addr(pIphd->saddr.s6_addr16, &(pstIp6));
	igmp_snp_syslog_dbg("igmp receive leave: pIphd->saddr=[%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x]\n",
						ADDRSHOWV6(pIphd->saddr.s6_addr16));

	pstIp6 = stPkt.saddr;
	mld_copy_ipv6addr(pPrevGroupVlan->saddrv6, &pstIp6);
	igmp_snp_syslog_dbg("igmp receive leave: pPrevGroupVlan->saddr"\
		"=[%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x]\n", ADDRSHOWV6(pPrevGroupVlan->saddrv6));

	igmp_snp_syslog_dbg("igmp receive leave: Ready to send the GS query\n");

	if ( IGMPSNP_RETURN_CODE_OK != ( mld_Snoop_Send_mld( &stPkt ) ) )
	{
		igmp_snp_syslog_dbg( "mld receive leave: send leave message failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	/************************************/
	/* set retransmit timer				*/
	/************************************/	
	igmp_snp_syslog_dbg("Set leave timer:\n");

	mld_snoop_pkt *pkt = NULL;
	timer_element *new_timer = NULL;

	pkt = (mld_snoop_pkt *)malloc(sizeof(struct mld_snoop_pkt));
	if(NULL == pkt )
	{
		igmp_snp_syslog_dbg("mld receive leave: malloc timer data memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(pkt,0,sizeof(struct mld_snoop_pkt));
	pkt->vlan_id = lVid;
	pstIp6 = pkt->groupadd;
	mld_copy_ipv6addr(ulGroup, &(pstIp6));
	pkt->type = MLD_TIMEOUT_RXMT;
	pkt->retranscnt = igmp_robust_variable -1;
	pkt->ifindex = ulIfIndex;
	//pIphd = sk_info->ip_hdr;/*useless*/
	if( IGMPSNP_RETURN_CODE_OK == mld_L2mc_Entry_GetVidx(lVid,ulGroup,&ulVidx)){
		pkt->group_id  = ulVidx;
	}
	else{
		igmp_snp_syslog_dbg("can NOT find vidx by vlanId & groupIp.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	igmp_snp_syslog_dbg("receive leave packet:"\
						"groupIp %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,\n"\
						"pkt->saddr %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
						ADDRSHOWV6(ulGroup),ADDRSHOWV6(pkt->saddr));
	 result = create_timer(TIMER_TYPE_NOLOOP,
							TIMER_PRIORI_NORMAL,
							igmp_rxmt_interval,
							(void *)mld_Event_Rxmt_Timeout,
							(void *)pkt,sizeof(mld_snoop_pkt),
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
* mld_recv_unknown()
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
LONG mld_recv_unknown(struct mld_info *sk_info )
{
	LONG lVid;
	ULONG usIfIndex;
	
	igmp_snp_syslog_event("Recvive packet unknown its mld type.\n" );

	lVid = sk_info->vlan_id;
	usIfIndex = sk_info->ifindex;
	mld_snp_flood( sk_info, lVid, usIfIndex );
	return IGMPSNP_RETURN_CODE_OK;
}





/**********************************************************************************
* mld_skb_proc()
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
INT mld_skb_proc
(
	struct igmp_skb *msg_skb
)
{
	struct ether_header_t *ethhdr = NULL;
	struct ipv6hdr *ip_addr = NULL;
	struct mld *mldhd = NULL;
	//ULONG igmplen;
	LONG vlan_id = 0;
	ULONG igmpType = 0x0,ret = IGMPSNP_RETURN_CODE_OK,uRet = IGMPSNP_RETURN_CODE_OK;
	USHORT group_ip[SIZE_OF_IPV6_ADDR] = {0},ifindex =0;
	ULONG taged = 0;
	struct mld_info *msg_info = NULL;
	struct mldv2 *v2_report = NULL;

	igmp_snp_syslog_dbg("Start for mld skb handle!\n");

	if(NULL == msg_skb) {
		igmp_snp_syslog_err("igmp packet with null skb, drop!\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	/*The most important: parse to build struct igmp_info form struct igmp_skb*/
	msg_info = (struct mld_info *)malloc(sizeof(struct mld_info));
	if( NULL == msg_info)
	{
		igmp_snp_syslog_err("malloc mag_info memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(msg_info,0,sizeof(struct mld_info));
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
	/*make sure the port is igmp snooping enable*/
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
		msg_info->ip_hdr = (struct ipv6hdr *)(msg_skb->buf + 18);
	else				/*untaged*/
		msg_info->ip_hdr = (struct ipv6hdr *)(msg_skb->buf + 14);


	/*handle PIM packet PIM is not support now from Hardware seting*/
	if( IPPROTO_PIM == msg_info->ip_hdr->nexthdr)
	{
		igmp_snp_syslog_event("receive PIM packet.\n");
		mld_pim_recv_msg(msg_skb);

		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_OK;
	}
	/*if there is option item?*/
	if(0 == msg_info->ip_hdr->nexthdr){
		msg_info->mld_hdr = (struct mld *)((unsigned char *)(msg_info->ip_hdr) + IPV6_HEADER_LEN + MLD_IPV6_OPTION_LEN);
	}
	else{
		msg_info->mld_hdr = (struct mld *)((unsigned char *)(msg_info->ip_hdr) + IPV6_HEADER_LEN);
	}
	msg_info->data = msg_skb;
	ip_addr = msg_info->ip_hdr;
	mldhd = msg_info->mld_hdr;

	if(MLD_V2_MEMSHIP_REPORT != mldhd->type){
		unsigned short *pstIp6 = group_ip;
		mld_copy_ipv6addr(mldhd->group, &(pstIp6));
		/*mld_ntohs(&pstIp6);*//*change report mld Group Ipv6Addr byte orders.no use now*/
	}
	else {
		v2_report = (struct mldv2 *)((char *)msg_info->ip_hdr + IPV6_HEADER_LEN + MLD_IPV6_OPTION_LEN);
		if( v2_report->naddrc >1 ){
			igmp_snp_syslog_err("MLD SNP can NOT support V2 multi-group report, when number of Group Records > 1.\n");

			free(msg_info);
			msg_info = NULL;			
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		USHORT *pstIp6 = group_ip;
		mld_copy_ipv6addr(v2_report->mldv2_rec.group, &pstIp6);
		/*mld_ntohs(&pstIp6);*/ /*change report mld GroupIpAddr byte orders.no use now*/
	}
	
	igmp_snp_syslog_dbg("vlan_id %d,ifindex %d,type %d, code %d, group:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x cksum:%d.\n",
					msg_skb->vlan_id,msg_skb->ifindex,mldhd->type,mldhd->code,ADDRSHOWV6(group_ip),mldhd->cksum);
	
	if(1 != ip_addr->hop_limit)
	{
		igmp_snp_syslog_err("IP header hop_limit =[%d] is not equal to 1.\n",ip_addr->hop_limit);

		free(msg_info);
		msg_info = NULL;
		return IGMPSNP_RETURN_CODE_ERROR_SW;
	}

	ifindex = msg_info->ifindex;
	vlan_id = msg_info->vlan_id;
	switch(mldhd->type)
	{
		case MLD_MEMSHIP_QUERY:		/*MLD Query */
			mld_recv_query(ifindex, mldhd->maxresp, group_ip, vlan_id, msg_info);
			break;
		case MLD_V2_MEMSHIP_REPORT: /*MLD report v2*/
		case MLD_V1_MEMSHIP_REPORT:	/*MLD report v1*/
			mld_recv_report(ifindex,group_ip,vlan_id,mldhd->type,msg_info);
			break;
		case MLD_V1_LEAVE_GROUP:	/*MLD leave*/
			mld_recv_leave(ifindex,group_ip,vlan_id,msg_info);
			break;
		default:
			mld_recv_unknown(msg_info);
			break;	
	}
	free(msg_info);
	msg_info = NULL;
	igmp_snp_syslog_dbg("End for skb handle!\n");

	return IGMPSNP_RETURN_CODE_OK;
}




#ifdef __cplusplus
}
#endif

