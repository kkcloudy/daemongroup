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
* igmp_snoop_hw.c
*
*
* CREATOR:
* 		chenbin@autelan.com
*
* DESCRIPTION:
* 		igmp inter source, handle hardware function.
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.22 $
*
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "igmp_snoop_com.h"
#include "igmp_snoop_inter.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

/********************************************global value****************************************/
l2mc_list_head		p_l2mc_list_head;
USHORT	igmp_snp_pfm_mode = 0;

/********************************************extern value***************************************/
//extern ULONG igmp_groupcount;
extern INT npdmng_fd;
extern INT recv_fd;
extern struct sockaddr_un remote_addr;
extern igmp_vlan_list *p_vlanlist;

/***********************************declare functions**************************************/
static LONG L2mc_Entry_Insert( igmp_snoop_pkt * pPkt );
static LONG L2mc_Entry_Update(  ULONG ulVlanId,char * pMac,ULONG ulVlanIdx );
//static LONG L2mc_Entry_GetVidx(  ULONG ulVlanId,char * pMac );
LONG L2mc_Entry_GetVidx(  ULONG ulVlanId,ULONG uGroupAddr,ULONG* ulVidx );
static LONG L2mc_Entry_Delete( igmp_snoop_pkt * pPkt );
static LONG L2mc_Entry_Node_Delete( igmp_snoop_pkt * pPkt );
static LONG L2mc_Entry_Group_Delete( igmp_snoop_pkt * pPkt );
static LONG igmp_notify_msg(igmp_snoop_pkt * pPkt,ULONG ulArg );
static LONG send_msg_npd(VOID *data,INT datalen,ULONG msg_type);
/***************************************************************************************/
/***********************************functions ********************************************/
void igmp_groupip2mac(unsigned long group,unsigned char *mac_addr)
{
	unsigned long low;
	unsigned char tmac[6];
	
	//low = 0x7FFFFFFF&group;
	tmac[0] = 0x01;
	tmac[1] = 0x0;
	tmac[2] = 0x5E;
	tmac[3] = (unsigned char)((group>>16)&0x7F);
	tmac[4] = (unsigned char)((group>>8)&0xFF);
	tmac[5] = (unsigned char)(group&0xFF);
	igmp_snp_syslog_dbg("##groupIP 0x%x <-->groupMac %02x:%02x:%02x:%02x:%02x:%02x!\n",\
					group,\
					tmac[0],tmac[1],tmac[2],\
					tmac[3],tmac[4],tmac[5]);
	memcpy(mac_addr,tmac,6);
}
/***************************************************************************************/
/*******************************************************************************
 * igmp_debug_print_notify
 *
 * DESCRIPTION:
 *			print the notify message
 *
 * INPUTS:
 *		send - pointer to the igmp message 
 *
 * OUTPUTS:
 *
 * RETURNS:
 *
 * COMMENTS:
 **
 ********************************************************************************/

VOID igmp_debug_print_notify(struct igmp_mng *send)
{
	igmp_snp_syslog_dbg("######################NOTIFY message##############\n");
	if( IGMP_SNP_FLAG_ADDR_MOD == send->nlh.nlmsg_flags )
	{
		igmp_notify_mod_pkt *send_noti = &(send->igmp_noti_npd);
		
		igmp_snp_syslog_dbg("Message Type:%d[%s]\n",
			send->nlh.nlmsg_flags,"IGMP_SNP_FLAG_ADDR_MOD");
		igmp_snp_syslog_dbg("-----------------------------------------\n");
		switch( send_noti->mod_type)
		{
			case IGMP_ADDR_ADD:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"IGMP_ADDR_ADD");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"IGMP_ADDR_ADD":"UNKNOWN");
				break;
			case IGMP_ADDR_DEL:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"IGMP_ADDR_DEL");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"IGMP_ADDR_DEL":"UNKNOWN");
				break;
			case IGMP_ADDR_RMV:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"IGMP_ADDR_RMV");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"IGMP_ADDR_RMV":"UNKNOWN");
				break;
			case IGMP_SYS_SET:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"IGMP_SYS_SET");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(IGMP_SYS_SET_STOP == send_noti->reserve)?"IGMP_SYS_SET_STOP":"IGMP_SYS_SET_INIT");
				break;
			case IGMP_TRUNK_UPDATE:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"IGMP_TRUNK_UPDATE");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"IGMP_TRUNK_UPDATE":"UNKNOWN");
				break;
			default:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"UNKNOWN");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,"UNKNOWN");
				break;
		}
		igmp_snp_syslog_dbg("Vlan_id:0x%04x Ifindex:0x%04x\n",
				send_noti->vlan_id,send_noti->ifindex);
		igmp_snp_syslog_dbg("Group:0x%x\n",send_noti->groupadd);
	}
	else
	{
		igmp_snp_syslog_dbg("Message Type:%d[%s]\n",
			send->nlh.nlmsg_flags,"Unknown");
	}
	igmp_snp_syslog_dbg("##################################################\n");
}


INT igmp_vlanportrelation(ULONG ifindex,LONG vlan_id,ULONG *targged)
{
	*targged = 0;		/*untargged*/
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_is_vlan_ifindex
 *
 * DESCRIPTION:
 *		check port is vlan member (which enable igmp snooping)
 *
 * INPUTS:
 * 		vlan_id  - vlan id
 *		ifindex - interface index
 *
 * OUTPUTS:
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - find vlan  port index
 *		IGMPSNP_RETURN_CODE_NOTENABLE_PORT - port is not enable igmp-snoop in vlan
 *		IGMPSNP_RETURN_CODE_NULL_PTR - igmp vlanlist is null
 *
 * COMMENTS:
 **
 ********************************************************************************/
INT igmp_is_vlan_ifindex(LONG vlan_id, ULONG ifindex)
{
	igmp_vlan_list *t_vlan = NULL;
	igmp_vlan_port_list *t_port = NULL;
	
	if( !p_vlanlist )
	{
		igmp_snp_syslog_dbg("igmp vlanlist is null.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			if( NULL != t_vlan->first_port )
			{
				t_port = t_vlan->first_port;
				while(t_port)
				{
					if( t_port->ifindex == ifindex )
					{
						igmp_snp_syslog_dbg("find vlan %d port_index %d.\n", vlan_id, ifindex);
						return IGMPSNP_RETURN_CODE_OK;		/*find*/
					}
					t_port = t_port->next;
				}
			}
		}
		t_vlan = t_vlan->next;
	}

	igmp_snp_syslog_dbg("port %d is not enable igmp-snoop in vlan %d.\n", ifindex, vlan_id);
	return IGMPSNP_RETURN_CODE_NOTENABLE_PORT;
}

INT igmp_get_vid_by_port(ULONG ifindex,ULONG ulVid[],INT *vCnt)
{
	INT tmpVlanCnt = 0;
	igmp_vlan_list *t_vlan = NULL;
	igmp_vlan_port_list *t_port = NULL;
	
	if( !p_vlanlist )
	{
		igmp_snp_syslog_dbg("igmp_is_vlan_ifindex:can not find any vlan.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	igmp_snp_syslog_dbg("igmp_is_vlan_ifindex:ifindex:%d\n",ifindex);
	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( NULL != t_vlan->first_port )
		{
			t_port = t_vlan->first_port;
			while(t_port)
			{
				if( t_port->ifindex == ifindex )
				{
					ulVid[tmpVlanCnt] = t_vlan->vlan_id;
					tmpVlanCnt ++;
					igmp_snp_syslog_dbg("find vlan %d port_index %d.\n",ulVid[tmpVlanCnt],ifindex);
				}
				t_port = t_port->next;
			}
		}
		t_vlan = t_vlan->next;
	}
	
	*vCnt = tmpVlanCnt;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_getifindex_byvlanid
 *
 * DESCRIPTION:
 *   		get the interface index by vlan id 
 *
 * INPUTS:
 * 		vlan_id  - vlan id
 *
 * OUTPUTS:
 *		first_ifindex - the first interface index
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - find the index
 *		IGMPSNP_RETURN_CODE_ERROR - vlan is empty
 *		IGMPSNP_RETURN_CODE_NULL_PTR - can not find any vlan
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - can not find vlan
 *
 * COMMENTS:
 **
 ********************************************************************************/
int igmp_getifindex_byvlanid(LONG vlan_id,ULONG *first_ifindex)
{
	igmp_vlan_list *t_vlan = NULL;
	
	if(!p_vlanlist)
	{
		igmp_snp_syslog_dbg("igmp_getifindex_byvlanid:can not find any vlan.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			if( NULL != t_vlan->first_port )
			{
				*first_ifindex = t_vlan->first_port->ifindex;
				return IGMPSNP_RETURN_CODE_OK;
			}
			else
			{
				*first_ifindex  = 0;
				igmp_snp_syslog_dbg("igmp_getifindex_byvlanid:this vlan is empty.\n");
				return IGMPSNP_RETURN_CODE_ERROR;
			}
		}
		t_vlan = t_vlan->next;
	}
	*first_ifindex = 0;
	igmp_snp_syslog_dbg("igmp_getifindex_byvlanid:can not find vlan.\n");
	return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
}
/*******************************************************************************
 * igmp_get_nextifindex_byifindex
 *
 * DESCRIPTION:
 *   		get the next interface index by the current interface index 
 *
 * INPUTS:
 * 		vlan_id  - vlan id
 *		ifindex - interface index
 *
 * OUTPUTS:
 *		nextifindex - the next interface index
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the next interface index success
 *		IGMPSNP_RETURN_CODE_ERROR - can not get the index
 *
 * COMMENTS:
 **
 ********************************************************************************/
LONG igmp_get_nextifindex_byifindex(LONG vlan_id, ULONG ifindex, ULONG *nextifindex)
{
	ULONG ret = IGMPSNP_RETURN_CODE_ERROR;
	igmp_vlan_list *t_vlan = NULL;
	
	if(!p_vlanlist)
	{
		igmp_snp_syslog_err("can not find any vlan.\n");
		return ret;
	}
	
	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			if( NULL != t_vlan->first_port )
			{
				igmp_vlan_port_list *t_port = t_vlan->first_port;
				while(NULL != t_port)
				{
					if( t_port->ifindex == ifindex)
					{
						if( NULL != t_port->next ) {
							igmp_snp_syslog_dbg("get ifIndex %d.\n",t_port->next->ifindex);
							*nextifindex = t_port->next->ifindex;
							return IGMPSNP_RETURN_CODE_OK;
						}
						else
							return ret;
					}
					t_port = t_port->next;
				}
				igmp_snp_syslog_dbg("can not find ifindex %d.\n", ifindex);
				return ret;
			}
			else
			{
				igmp_snp_syslog_dbg("this vlan %d is empty.\n", vlan_id);
				return ret;
			}
		}
		t_vlan = t_vlan->next;
	}
	igmp_snp_syslog_dbg("can not find vlan %d.\n", vlan_id);
	return ret;

}


void igmp_getifstatus(ULONG ifindex,ULONG *portstate)
{
	//DEBUG_OUT("igmp_getifstatus:excuted successful.\n");
	*portstate = 1;
}

void igmp_getmaxgroupcnt(ULONG ifindex,ULONG *maxgroupcnt)
{
	//DEBUG_OUT("igmp_getmaxgroupcnt:excuted successful.\n");
	*maxgroupcnt = IGMP_SNP_GRP_MAX;
}


INT igmp_getvlan_addr(ULONG vlan_id, ULONG *vlan_addr)
{
	//DEBUG_OUT("igmp_getvlan_addr:excuted successful.\n");
	return IGMPSNP_RETURN_CODE_ERROR;
}
/*******************************************************************************
 * IFM_PhyIsMerged2TrunkApi
 *
 * DESCRIPTION:
 *   		check port is member of trunck
 *
 * INPUTS:
 * 		vlan_id  - vlan id
 *		ifindex - vlan interface index
 
 *
 * OUTPUTS:
 *		 trunkifindex - trunk interface index
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   	- port is member of trunk
 * 		IGMPSNP_RETURN_CODE_NULL_PTR	- vlanlist is null
 *		IGMPSNP_RETURN_CODE_ERROR - port is  not member of trunk
 *
 * COMMENTS:
 **
 ********************************************************************************/
int IFM_PhyIsMerged2TrunkApi(LONG vlan_id,ULONG ifindex,ULONG *trunkifindex)
{	/*check port is member of trunck*/
	igmp_vlan_list *t_vlan = NULL;
	igmp_vlan_port_list *t_port = NULL;
	
	//DEBUG_OUT("Entry IFM_PhyIsMerged2TrunkApi\n");
	if( !p_vlanlist )
	{
		igmp_snp_syslog_err("can not find any vlan, when check port %d is member of trunck.\n", ifindex);
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			if( NULL != t_vlan->first_port )
			{
				t_port = t_vlan->first_port;
				while(t_port)
				{
					if( t_port->ifindex == ifindex )
					{
						if(t_port->trunkflag != 0 )
						{/* port is member of trunk */
							*trunkifindex = t_port->ifindex;
							return IGMPSNP_RETURN_CODE_OK;		/*trunk*/
						}
						else
						{/* port is not member of trunk */
							return IGMPSNP_RETURN_CODE_ERROR;
						}
					}
					t_port = t_port->next;
				}
			}
		}
		t_vlan = t_vlan->next;
	}
	return IGMPSNP_RETURN_CODE_ERROR;
}

int IFM_isTrunkIf(LONG vlan_id, ULONG ifindex)
{
	igmp_vlan_list *t_vlan = NULL;
	igmp_vlan_port_list *t_port = NULL;
	
	if( !p_vlanlist )
	{
		igmp_snp_syslog_dbg("igmp_is_vlan_ifindex:can not find any vlan.\n");
		return IGMP_SNOOP_ERR;
	}
	igmp_snp_syslog_dbg("igmp_is_vlan_ifindex:vlan_id:%d\tifindex:%d\n",vlan_id,ifindex);
	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			if( NULL != t_vlan->first_port )
			{
				t_port = t_vlan->first_port;
				while(t_port)
				{
					if( t_port->ifindex == ifindex )
						if(t_port->trunkflag != 0 )
							return 1;		/*trunk*/
						else
							return 0;
				}
				t_port = t_port->next;
			}
		}
		t_vlan = t_vlan->next;
	}
	return 0;
}
/*******************************************************************************
 * igmp_vlanisused
 *
 * DESCRIPTION:
 *   		check vlan exists/active
 *
 * INPUTS:
 * 		vlan_id  - vlan id
 *
 * OUTPUTS:
 *		ret - flag
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   	- find vlan or not but the process is success
 * 		IGMPSNP_RETURN_CODE_NULL_PTR	- vlanlist is null
 *
 * COMMENTS:
 **
 ********************************************************************************/
LONG igmp_vlanisused( LONG vlan_id, ULONG * ret )
{
	igmp_vlan_list *t_vlan = NULL;
	if( !p_vlanlist )
	{
		igmp_snp_syslog_err("igmp-snoop vlanlist is null.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	t_vlan = p_vlanlist;
	while(NULL != t_vlan)
	{
		if( t_vlan->vlan_id == vlan_id)
		{
			*ret = 1;
			igmp_snp_syslog_dbg("find vlan %d in igmp-snoop vlanlist.\n", vlan_id);
			return IGMPSNP_RETURN_CODE_OK;
		}
		t_vlan = t_vlan->next;
	}
	*ret = 0;
	
	igmp_snp_syslog_dbg("no find vlan %d in igmp-snoop vlanlist.\n", vlan_id);
	return IGMPSNP_RETURN_CODE_OK;
}


LONG igmp_check_ip_from_mac(ULONG group, char *groupmac)
{
	igmp_snp_syslog_dbg("##groupIP 0x%x <-->groupMac %02x:%02x:%02x:%02x:%02x:%02x!\n",\
					group,\
					groupmac[0],groupmac[1],groupmac[2],\
					groupmac[3],groupmac[4],groupmac[5]);
	if (( groupmac[0] != 0x1) || (groupmac[1] != 0x0) ||(groupmac[2] != 0x5e)){
		igmp_snp_syslog_dbg("groupIp convert Mac Error0.\n");
		return IGMPSNP_RETURN_CODE_ERROR;}
	if (groupmac[3] != ((group >> 16) & 0x7f) ){
		igmp_snp_syslog_dbg("groupIp convert Mac Error1.\n");
		return IGMPSNP_RETURN_CODE_ERROR;}
	if (groupmac[4] != ((group >> 8) & 0xff) ){
		igmp_snp_syslog_dbg("groupIp convert Mac Error2.\n");
		return IGMPSNP_RETURN_CODE_ERROR;}
	if (groupmac[5] != ((group >> 0) & 0xff) ){
		igmp_snp_syslog_dbg("groupIp convert Mac Error3.\n");
		return IGMPSNP_RETURN_CODE_ERROR;}
	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* L2mc_Entry_Insert()
*
* DESCRIPTION:
*		insert the packet info into layer 2 entry
*
* INPUTS:
* 		pPkt - pointer to packet
*
* OUTPUTS:
*		
* RETURN:
*		IGMPSNP_RETURN_CODE_OK - insert success
*		IGMPSNP_RETURN_CODE_NULL_PTR -pPkt is null
*		IGMPSNP_RETURN_CODE_ERROR	- layer 2 mc-entry is null
*		IGMPSNP_RETURN_CODE_OUT_RANGE - layer 2 multi list is out range
*		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc memery fail
*
**************************************************************************/
LONG L2mc_Entry_Insert( igmp_snoop_pkt * pPkt )
{
	
	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	l2mc_list * tmpEntry = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;
	igmp_snp_syslog_dbg("Enter :L2mc_Entry_Insert...\n");
	if ( NULL == pPkt )
	{
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	if( p_l2mc_list_head.stListHead )
	{
		pstTemp = p_l2mc_list_head.stListHead;
		while(pstTemp)
		{
			if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
			{
				return IGMPSNP_RETURN_CODE_OUT_RANGE;
			}
			lCount++;
			if ( ( pstTemp ->ulGroup == pPkt->groupadd ) && 
				 ( pstTemp ->ulVlanId == pPkt->vlan_id ) &&
				 ( pstTemp ->ulIndex == pPkt->ifindex ) &&
				 ( pstTemp ->ulVlanIdx == pPkt->group_id) )
			{
				EntryExist = 1;
				break;
			}
			//if( pstTemp == p_l2mc_list_head.stListHead )	/*can not find*/
			if(pstTemp == pstTemp->next){
				if ( ( NULL != pstTemp ) &&
					 ( pstTemp ->ulGroup == pPkt->groupadd ) && 
					 ( pstTemp ->ulVlanId == pPkt->vlan_id ) &&
					 ( pstTemp ->ulIndex == pPkt->ifindex ) &&
					 ( pstTemp ->ulVlanIdx == pPkt->group_id))
				{
					 EntryExist = 1;
					 break;
				}
				else
					{break;}
			}
			pstTemp = pstTemp->next;
		}
		if ( EntryExist != 1 )
		{
			if ( p_l2mc_list_head.ulListNodeCount >= IGMP_SNP_GRP_MAX + 1 )
			{
				return IGMPSNP_RETURN_CODE_OUT_RANGE;
			}
			pstL2mcEntry = (l2mc_list *)malloc( sizeof(l2mc_list ));
			if ( pstL2mcEntry == NULL )
			{
				return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
			}
			igmp_snp_syslog_dbg("L2mc_Entry_Insert:not first entry,l2mcEntry@@%p!\n",pstL2mcEntry);
			pstL2mcEntry ->ulIndex = pPkt->ifindex;
			pstL2mcEntry ->ulGroup = pPkt->groupadd;
			pstL2mcEntry ->ulVlanId = pPkt->vlan_id;
			pstL2mcEntry ->ulVlanIdx = pPkt->group_id;
			/*
			pstL2mcEntry->next = p_l2mc_list_head.stListHead;
			pstL2mcEntry->prev = p_l2mc_list_head.stListHead->prev;
			if(NULL != p_l2mc_list_head.stListHead->prev){
				(p_l2mc_list_head.stListHead->prev)->next = pstL2mcEntry;
			}
			p_l2mc_list_head.stListHead->prev = pstL2mcEntry;
			*/
			tmpEntry = p_l2mc_list_head.stListHead;
			tmpEntry->prev = pstL2mcEntry;
			pstL2mcEntry->next = tmpEntry;
			pstL2mcEntry->prev = pstL2mcEntry;
			p_l2mc_list_head.stListHead = pstL2mcEntry;
			p_l2mc_list_head.ulListNodeCount++;
		}
	}
	else
	{
		pstL2mcEntry = (l2mc_list*)malloc( sizeof( l2mc_list ) );
		if ( pstL2mcEntry == NULL )
		{
			return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
		}
		pstL2mcEntry->next = pstL2mcEntry;
		pstL2mcEntry->prev = pstL2mcEntry;
		pstL2mcEntry ->ulIndex = pPkt->ifindex;
		pstL2mcEntry ->ulGroup = pPkt->groupadd;
		pstL2mcEntry ->ulVlanId = pPkt->vlan_id;
		pstL2mcEntry->ulVlanIdx = pPkt->group_id;
		p_l2mc_list_head.stListHead = pstL2mcEntry;
		p_l2mc_list_head.ulListNodeCount = 1;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* L2mc_Entry_GetVidx()
*
* DESCRIPTION:
*		get vidx
*
* INPUTS:
* 		ulVlanId - vlan id
*		uGroupAddr - group address
*		
* OUTPUTS:
*		ulVidx - the vlan's vidx
*
* RETURN:
*		IGMPSNP_RETURN_CODE_OK - get success
*		IGMPSNP_RETURN_CODE_ERROR	- not get the vidx
*		IGMPSNP_RETURN_CODE_OUT_RANGE - layer 2 multi list is out range
*
**************************************************************************/
LONG L2mc_Entry_GetVidx(  ULONG ulVlanId,ULONG uGroupAddr,ULONG* ulVidx )
{
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;  
	ULONG ulVlanIdx=0,ulRet = IGMPSNP_RETURN_CODE_ERROR;

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		if( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMPSNP_RETURN_CODE_OUT_RANGE;
		}
		lCount++;
		
		if( (pstTemp->ulVlanId == ulVlanId) &&
			(pstTemp->ulGroup == uGroupAddr) 
			/*&&( IGMP_SNOOP_OK == igmp_check_ip_from_mac (pstTemp->ulGroup,pMac))*/)
		{
			ulVlanIdx= pstTemp->ulVlanIdx;
			ulRet = IGMPSNP_RETURN_CODE_OK;
			break;
		}
		if( pstTemp == pstTemp->next )	/*can not find*/
		{
			if( (pstTemp->ulVlanId == ulVlanId) &&
				(pstTemp->ulGroup == uGroupAddr) )
			{
				ulVlanIdx= pstTemp->ulVlanIdx;
				ulRet = IGMPSNP_RETURN_CODE_OK;
				break;
			}
			else { break; }	 
		}	
		pstTemp = pstTemp->next;
	}

	*ulVidx = ulVlanIdx;
	return ulRet;
}

#if 0
/*update the l2mc entry's Vidx*/
LONG L2mc_Entry_Update(  ULONG ulVlanId,char * pMac,ULONG ulVlanIdx )
{
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;     

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMP_SNOOP_ERR;
		}
		lCount++;
		
		if (( pstTemp->ulVlanId== ulVlanId) 
			&&( IGMP_SNOOP_OK == igmp_check_ip_from_mac (pstTemp->ulGroup,pMac)))	
		{
			pstTemp->ulVlanIdx = ulVlanIdx;/*update the vidx.*/
			break;
		}
		pstTemp = pstTemp->next;
		if( pstTemp == p_l2mc_list_head.stListHead )	/*can not find*/
			break;
	}
	return IGMP_SNOOP_OK;
}

/*delete one Node in l2mc entry list*/
/*we need to delete one group member*/
LONG L2mc_Entry_Delete( igmp_snoop_pkt * pPkt )
{

	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;

	if ( NULL == pPkt )
	{
		return IGMP_SNOOP_ERR;
	}

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{

		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMP_SNOOP_ERR;
		}
		lCount++;

		if ( ( pstTemp ->ulGroup == pPkt->groupadd)
				&& ( pstTemp->ulVlanId == pPkt->vlan_id) )
		{
			EntryExist = 1;
			pstL2mcEntry = pstTemp;
			break;
		}
		pstTemp = pstTemp->next;
		if( pstTemp == p_l2mc_list_head.stListHead )	/*can not find*/
			break;
	}
	if ( EntryExist == 1 )
	{
		if ( pstL2mcEntry == NULL )
		{
			return IGMP_SNOOP_ERR;
		}

		(pstL2mcEntry->prev)->next = pstL2mcEntry->next;
		(pstL2mcEntry->next)->prev = pstL2mcEntry->prev;
		free( pstL2mcEntry );
		pstL2mcEntry = NULL;
		p_l2mc_list_head.ulListNodeCount--;
		if(0== p_l2mc_list_head.ulListNodeCount)
			p_l2mc_list_head.stListHead = NULL;
		IGMP_SNP_DEBUG(" L2mc_Entry_Delete: remove entry.\r\n");
	}
	
	return IGMP_SNOOP_OK;
}
#endif
/**************************************************************************
* L2mc_Entry_Node_Delete()
*
* DESCRIPTION:
*
*
* INPUTS:
* 		pPkt - pointer to packet
*
* OUTPUTS:
*		
* RETURN:
*		IGMPSNP_RETURN_CODE_OK - delete success
*		IGMPSNP_RETURN_CODE_NULL_PTR -pPkt is null
*		IGMPSNP_RETURN_CODE_ERROR	- layer 2 mc-entry is null
*		IGMPSNP_RETURN_CODE_OUT_RANGE - layer 2 multi list is out range
*
**************************************************************************/
LONG L2mc_Entry_Node_Delete( igmp_snoop_pkt * pPkt )
{
	igmp_snp_syslog_dbg("Enter: L2mc_Entry_Node_Delete::vlanId %d,vlanIdx %d,ifindex %d,groupIp 0x%x.\n",\
					pPkt->vlan_id,pPkt->group_id,pPkt->ifindex,pPkt->groupadd);
	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;

	if ( NULL == pPkt )
	{
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		igmp_snp_syslog_dbg("GOLBAL g_l2mc_list_head.stListHead@@%p.\n",pstTemp);
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMPSNP_RETURN_CODE_OUT_RANGE;
		}
		lCount++;

		if ( (pstTemp ->ulGroup == pPkt->groupadd)	&& 
			 (pstTemp->ulVlanId == pPkt->vlan_id) )
		{
			if( (pstTemp->ulIndex == pPkt->ifindex ) && 
				(pstTemp->ulVlanIdx == pPkt->group_id) )
			{
				EntryExist = 1;
				pstL2mcEntry = pstTemp;
				igmp_snp_syslog_dbg("find out an l2mc_entry:vidx %d,groupIp 0x%x,vlan_id %d,ifindex %d.\n",\
					pstL2mcEntry->ulVlanIdx,pstL2mcEntry->ulGroup,pstL2mcEntry->ulVlanId,pstL2mcEntry->ulIndex);
				break;
			}
		}
		{		//if( pstTemp == p_l2mc_list_head.stListHead )
			if( pstTemp == pstTemp->next)
			{
				/*can not find*/
				if((pstTemp ->ulGroup == pPkt->groupadd)&& 
				   (pstTemp->ulVlanId == pPkt->vlan_id) &&
				   (pstTemp->ulIndex == pPkt->ifindex) && 
				   (pstTemp->ulVlanIdx == pPkt->group_id))
				{
					EntryExist = 1;
					pstL2mcEntry = pstTemp;
					igmp_snp_syslog_dbg("find out an l2mc_entry:vidx %d,groupIp 0x%x,vlan_id %d,ifindex %d.\n",\
						pstL2mcEntry->ulVlanIdx,pstL2mcEntry->ulGroup,pstL2mcEntry->ulVlanId,pstL2mcEntry->ulIndex);
					break;
				}
				igmp_snp_syslog_dbg("can NOT find a l2mc_list entry: vlanId %d,vidx %d,ifindex %d,groupIp 0x%x.\n",\
								pPkt->vlan_id,pPkt->group_id,pPkt->ifindex,pPkt->groupadd);
				break;
			}
			pstTemp = pstTemp->next;
		}
	}
	if ( EntryExist == 1 )
	{
		if ( pstL2mcEntry == NULL )
		{
			return IGMPSNP_RETURN_CODE_ERROR;
		}

		if(!(pstL2mcEntry == pstL2mcEntry->prev && pstL2mcEntry == pstL2mcEntry->next) ){
			if(pstL2mcEntry == pstL2mcEntry->prev){
				pstL2mcEntry->next->prev = pstL2mcEntry->next;
				p_l2mc_list_head.stListHead = pstL2mcEntry->next;
			}
			else if(pstL2mcEntry == pstL2mcEntry->next){
				pstL2mcEntry->prev->next =pstL2mcEntry->prev;
			}
			else {
				(pstL2mcEntry->prev)->next = pstL2mcEntry->next;
				(pstL2mcEntry->next)->prev = pstL2mcEntry->prev;
			}
		}
		/*
		else{
			p_l2mc_list_head.stListHead = NULL;
		}
		*/
		free( pstL2mcEntry );
		p_l2mc_list_head.ulListNodeCount--;
		if(0== p_l2mc_list_head.ulListNodeCount)
			p_l2mc_list_head.stListHead = NULL;
	}
	//IGMP_SNP_DEBUG("##########L2mc_Entry_Node_Delete:return OK.\n");
	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* L2mc_Entry_Group_Delete_All()
*
* DESCRIPTION:
*		delete one Group in l2mc entry list
*
* INPUTS:
* 		pPkt - pointer to packet
*
* OUTPUTS:
*		
* RETURN:
*		IGMPSNP_RETURN_CODE_OK - delete success
*		IGMPSNP_RETURN_CODE_NULL_PTR -pPkt is null
*		IGMPSNP_RETURN_CODE_ERROR	- layer 2 mc-entry is null
*		IGMPSNP_RETURN_CODE_OUT_RANGE - layer 2 multi list is out range
*
**************************************************************************/

/*we need to delete one group member*/
LONG L2mc_Entry_Group_Delete( igmp_snoop_pkt * pPkt )
{

	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;

	if ( NULL == pPkt )
	{
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		igmp_snp_syslog_dbg("l2mc_list entry pstTemp@@%p,ulVlanId %d,ulVlanIdx %d,ulGroup 0x%x,ulIndex %d.\n",\
			pstTemp,pstTemp->ulVlanId,pstTemp->ulVlanIdx,pstTemp->ulGroup,pstTemp->ulIndex);
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMPSNP_RETURN_CODE_OUT_RANGE;
		}
		lCount++;

		if ( (pstTemp->ulGroup == pPkt->groupadd) &&
			 (pstTemp->ulVlanId == pPkt->vlan_id) &&
			 (pstTemp->ulVlanIdx == pPkt->group_id))
		{
			EntryExist = 1;
			pstL2mcEntry = pstTemp;
			
		}
		if( pstTemp == pstTemp->next )	/*can not find*/
		{  
			if ( (pstTemp->ulGroup == pPkt->groupadd) &&
				 (pstTemp->ulVlanId == pPkt->vlan_id) &&
				 (pstTemp->ulVlanIdx == pPkt->group_id))
			{
				EntryExist = 1;
				pstL2mcEntry = pstTemp;
			}
			break;
		}
		
		pstTemp = pstTemp->next;

		if(1 == EntryExist){
			if ( pstL2mcEntry == NULL )
			{
				return IGMPSNP_RETURN_CODE_ERROR;
			}

			if(!(pstL2mcEntry == pstL2mcEntry->prev && pstL2mcEntry == pstL2mcEntry->next) ){
				if(pstL2mcEntry == pstL2mcEntry->prev){
					pstL2mcEntry->next->prev = pstL2mcEntry->next;
					p_l2mc_list_head.stListHead = pstL2mcEntry->next;
				}
				else if(pstL2mcEntry == pstL2mcEntry->next){
					pstL2mcEntry->prev->next =pstL2mcEntry->prev;
				}
				else {
					(pstL2mcEntry->prev)->next = pstL2mcEntry->next;
					(pstL2mcEntry->next)->prev = pstL2mcEntry->prev;
				}
			}
			igmp_snp_syslog_dbg("L2mc_Entry_Group_Delete: remove l2Mcentry :vid %d,vidx %d,groupIp 0x%x ifIndex %d,in loop while().\n",
								pstL2mcEntry->ulVlanId,pstL2mcEntry->ulVlanIdx,pstL2mcEntry->ulGroup,pstL2mcEntry->ulIndex);
			free( pstL2mcEntry );
			pstL2mcEntry = NULL;

			p_l2mc_list_head.ulListNodeCount--;
			if(0== p_l2mc_list_head.ulListNodeCount){
				p_l2mc_list_head.stListHead = NULL;
				break;
			}
		}
		EntryExist = 0;
		pstL2mcEntry = NULL;
	}
	igmp_snp_syslog_dbg(" L2mc_Entry_Group_Delete: remove one group Done after loop while().\n");
	return IGMPSNP_RETURN_CODE_OK;
}

#if 0
LONG L2mc_Entry_Group_Delete_All( igmp_snoop_pkt * pPkt )
{

	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;

	if ( NULL == pPkt )
	{
		return IGMP_SNOOP_ERR;
	}

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMP_SNOOP_ERR;
		}
		lCount++;

		if ( pstTemp->ulVlanId == pPkt->vlan_id )
		{
			EntryExist = 1;
			pstL2mcEntry = pstTemp;
		}
		
		if( pstTemp == pstTemp->next )	/*can not find*/
		{	
			if ( pstTemp->ulVlanId == pPkt->vlan_id ){
				EntryExist = 1;
				pstL2mcEntry = pstTemp;
			}
			break;
		}

		pstTemp = pstTemp->next;

		if(1 == EntryExist) {
			if ( pstL2mcEntry == NULL )
			{
				return IGMP_SNOOP_ERR;
			}
			if(!(pstL2mcEntry == pstL2mcEntry->prev && pstL2mcEntry == pstL2mcEntry->next) ){
				if(pstL2mcEntry == pstL2mcEntry->prev){
					pstL2mcEntry->next->prev = pstL2mcEntry->next;
					p_l2mc_list_head.stListHead = pstL2mcEntry->next;
				}
				else if(pstL2mcEntry == pstL2mcEntry->next){
					pstL2mcEntry->next =pstL2mcEntry->prev;
				}
				else {
					(pstL2mcEntry->prev)->next = pstL2mcEntry->next;
					(pstL2mcEntry->next)->prev = pstL2mcEntry->prev;
				}
			}
			free( pstL2mcEntry );
			pstL2mcEntry = NULL;

			p_l2mc_list_head.ulListNodeCount--;
			if(0== p_l2mc_list_head.ulListNodeCount) {
				p_l2mc_list_head.stListHead = NULL;
				break;
			}
			IGMP_SNP_DEBUG(" L2mc_Entry_Group_Delete_All: remove one entry in loop while().\n");
		}
	}
	IGMP_SNP_DEBUG(" L2mc_Entry_Group_Delete_All: remove one group Done after loop while().\n");
	return IGMP_SNOOP_OK;
}
#endif
/**************************************************************************
* L2mc_Entry_Group_Delete_All()
*
* DESCRIPTION:
*		delete all the group
*
* INPUTS:
* 		pPkt - pointer to packet
*
* OUTPUTS:
*		
* RETURN:
*		IGMPSNP_RETURN_CODE_OK - delete success
*		IGMPSNP_RETURN_CODE_NULL_PTR -pPkt is null
*		IGMPSNP_RETURN_CODE_ERROR	- layer 2 mcentry is null
*
**************************************************************************/
LONG L2mc_Entry_Group_Delete_All( igmp_snoop_pkt * pPkt )
{

	l2mc_list * pstL2mcEntry = NULL;
	l2mc_list * pstTemp = NULL;
	LONG lCount = 0;
	ULONG EntryExist = 0;

	if ( NULL == pPkt )
	{
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	pstTemp = p_l2mc_list_head.stListHead;
	while(pstTemp)
	{
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		lCount++;
		pstL2mcEntry = pstTemp;
		pstTemp = pstTemp->next;

		if ( pstL2mcEntry == NULL )
		{
			return IGMPSNP_RETURN_CODE_ERROR;
		}
		if(!(pstL2mcEntry == pstL2mcEntry->prev && pstL2mcEntry == pstL2mcEntry->next)){
			if(pstL2mcEntry == pstL2mcEntry->prev){
				pstL2mcEntry->next->prev = pstL2mcEntry->next;
				p_l2mc_list_head.stListHead = pstL2mcEntry->next;
			}
			else if(pstL2mcEntry == pstL2mcEntry->next){
				pstL2mcEntry->next =pstL2mcEntry->prev;
			}
			else {
				(pstL2mcEntry->prev)->next = pstL2mcEntry->next;
				(pstL2mcEntry->next)->prev = pstL2mcEntry->prev;
			}
		}
		igmp_snp_syslog_dbg(" L2mc_Entry_Group_Delete_All: remove entry pstL2mcEntry@@%p,VlanId %d,VlanIdx %d,GroupIp 0x%x,ifIndex %d in loop while().\n",\
						pstL2mcEntry,pstL2mcEntry->ulVlanId,pstL2mcEntry->ulVlanIdx,pstL2mcEntry->ulGroup,pstL2mcEntry->ulIndex);
		free( pstL2mcEntry );
		pstL2mcEntry = NULL;

		p_l2mc_list_head.ulListNodeCount--;
		if(0== p_l2mc_list_head.ulListNodeCount) {
			p_l2mc_list_head.stListHead = NULL;
			break;
		}
	}
	igmp_snp_syslog_dbg(" L2mc_Entry_Group_Delete_All: remove one group Done after loop while().\n");
	return IGMPSNP_RETURN_CODE_OK;
}
/**************************************************************************
* igmp_snp_mod_addr()
*
* DESCRIPTION:
*		Modify L2/L3 MC address  hardware table and set port filter mode.
*
* INPUTS:
* 		pPkt - pointer to packet
*		ulArg - modify address type
*
* OUTPUTS:
*		
* RETURN:
*		IGMPSNP_RETURN_CODE_NULL_PTR -pPkt is null
*		IGMPSNP_RETURN_CODE_ERROR	- send error
*
**************************************************************************/
LONG igmp_snp_mod_addr( igmp_snoop_pkt * pPkt, ULONG ulArg )
{
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	
	igmp_snp_syslog_dbg("START:igmp_snp_mod_addr\n");

	if ( NULL == pPkt )
	{
		igmp_snp_syslog_err("parameter null pointer!\n");
		igmp_snp_syslog_dbg("END:igmp_snp_mod_addr\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	igmp_snp_syslog_dbg("GroupId %d vid %d group 0x%.8x If 0x%x arg %d\n",
	      						pPkt->group_id,pPkt->vlan_id, pPkt->groupadd, pPkt->ifindex, ulArg);
	switch ( ulArg )
	{
		case IGMP_ADDR_ADD:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			stPkt.vlan_id = lVid;
			stPkt.group_id= pMcGroup->mgroup_id;//It's the Vidx.Need a mechanism for assignment vidx!
			stPkt.groupadd= ulGroup;
			stPkt.ifindex = pMemberPort->ifindex;
			stPkt.type = IGMP_ADDR_ADD;
			*/
			lRet = L2mc_Entry_Insert( pPkt );
			lRet = igmp_notify_msg(pPkt,ulArg);
			break;
		case IGMP_ADDR_DEL:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			stPkt.type = IGMP_ADDR_DEL;
			stPkt.ifindex = pPkt->ifindex;
			stPkt.groupadd= pMcGroup->MC_ipadd;
			stPkt.vlan_id= lVid;
			*/
			lRet = L2mc_Entry_Node_Delete((VOID *)pPkt);/*added by wujh*/
			lRet = igmp_notify_msg(pPkt,ulArg);
			break;
		case IGMP_ADDR_RMV:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			pkt.vlan_id = pGroup->vid;
			pkt.groupadd= pGroup->MC_ipadd;
			pkt.type = IGMP_ADDR_RMV;
			pkt.ifindex = 0;
			pkt.group_id = pGroup->mgroup_id;
			*/
			lRet = L2mc_Entry_Group_Delete((VOID *)pPkt);
			lRet = igmp_notify_msg(pPkt,ulArg);
			break;
		case IGMP_SYS_SET:
			if(IGMP_SYS_SET_INIT == pPkt->type){
				/*
				t_pkt.vlan_id = 0;
				t_pkt.groupadd = 0;
				t_pkt.ifindex = 0;
				t_pkt.type = IGMP_SYS_SET_INIT; //init
				*/
				lRet = igmp_notify_msg(pPkt,ulArg);
				break;
			}
			if(IGMP_SYS_SET_STOP == pPkt->type){
				/*
				t_pkt.vlan_id = 0;
				t_pkt.groupadd = 0;
				t_pkt.ifindex = 0;
				t_pkt.type = IGMP_SYS_SET_STOP;	
				*/
				lRet = L2mc_Entry_Group_Delete_All((VOID *)pPkt);
				lRet = igmp_notify_msg( pPkt, ulArg);
				break;
			}
		default:
			lRet = IGMPSNP_RETURN_CODE_ERROR;
	}

	igmp_snp_syslog_dbg("END:igmp_snp_mod_addr\n");
	return lRet;
}
/*******************************************************************************
 * igmp_notify_msg
 *
 * DESCRIPTION:
 *   			send motify message to npd
 *
 * INPUTS:
 * 		pPkt - pointer to packet
 *		ulArg - modify address type
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   - on success
 * 		IGMPSNP_RETURN_CODE_NULL_PTR	- null pPkt
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc memery fail
 *		IGMPSNP_RETURN_CODE_ERROR - No match type or send message fail
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/

LONG igmp_notify_msg(igmp_snoop_pkt * pPkt,ULONG ulArg )
{
	LONG ret = IGMPSNP_RETURN_CODE_OK;
	igmp_notify_mod_pkt *send_noti = NULL;
	ULONG lVidx = 0;
	unsigned char pMac[6] = {0};
	if( NULL == pPkt )
	{
		igmp_snp_syslog_err("igmp_notify_add: null pPkt \n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	send_noti = (igmp_notify_mod_pkt *)malloc(sizeof(igmp_notify_mod_pkt));
	if( NULL == send_noti )
	{
		igmp_snp_syslog_err("igmp_notify_add: malloc memory failed. \n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(send_noti,0,sizeof(igmp_notify_mod_pkt));
	/*
	send_noti->mod_type = pPkt->type;
	send_noti->reserve = pPkt->type;
	*/
	send_noti->mod_type = ulArg;
	send_noti->ifindex = pPkt->ifindex;
	send_noti->vlan_id = pPkt->vlan_id;
	send_noti->groupadd = pPkt->groupadd;

	switch (ulArg){
		case IGMP_ADDR_ADD:
		case IGMP_ADDR_DEL:
		case IGMP_ADDR_RMV:
			{
				//igmp_groupip2mac(send_noti->groupadd,pMac);
				//lVidx = L2mc_Entry_GetVidx(send_noti->vlan_id, pMac);
				/*
				if(1 == L2mc_Entry_GetVidx(send_noti->vlan_id, send_noti->groupadd,&lVidx)
					send_noti->reserve = lVidx;
				*/
				send_noti->reserve = pPkt->group_id;
				igmp_snp_syslog_dbg("event type %d,vlan_id %d,groupadd 0x%x,vidx %d,ifindex %d.\n",\
					ulArg,send_noti->vlan_id,send_noti->groupadd,send_noti->reserve,send_noti->ifindex);
			}	
			break;
		case IGMP_SYS_SET:
			{
				send_noti->reserve = pPkt->type;//IGMP_SYS_SET INIT/STOP
			}
			break;
		default :
			{
				igmp_snp_syslog_dbg("No match igmpmodaddtype!\n");
				return IGMPSNP_RETURN_CODE_ERROR;
			}
	}
	ret = send_msg_npd((void *)send_noti,sizeof(igmp_notify_mod_pkt),IGMP_SNP_FLAG_ADDR_MOD );
	if( IGMPSNP_RETURN_CODE_OK != ret )
	{
		igmp_snp_syslog_dbg("igmp_notify_add: send notify message failed. \n");
		free(send_noti);
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	//DEBUG_OUT("Igmp_notify_add:send message successful.\n");
	free(send_noti);
	return ret;
}
/*******************************************************************************
 * send_msg_npd
 *
 * DESCRIPTION:
 * 		send manage msg to npd.
 *
 * INPUTS:
 * 		data - point to igmp notify mod pkt
 *		datelen - sizeof(igmp notify mod pkt)
 *		msg _type - message type
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 * 	IGMPSNP_RETURN_CODE_OK   - on success
 * 	IGMPSNP_RETURN_CODE_NULL_PTR	- data is null
 *	IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc memery fail
 *	IGMPSNP_RETURN_CODE_ERROR - Igmp Snp Cmd Write To Npd Fail
 *
 * COMMENTS:
 *      comments here
 **
 ********************************************************************************/

/**/
LONG send_msg_npd
(	
	VOID *data, /*struct igmp_notify_mod_pkt*/
	INT datalen,/*sizeof(igmp_notify_mod_pkt)*/
	ULONG msg_type
)
{
	struct igmp_mng *send = NULL;
	fd_set wfds;
	int rc, send_len = 0,tmp_fd;
	int cmdLen = sizeof(struct igmp_mng);
	int addrlen = sizeof(struct sockaddr);
	if( NULL == data )
	{
		igmp_snp_syslog_dbg("send_msg: null data. \n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	send = (struct igmp_mng *)malloc(sizeof(struct igmp_mng));
	if(NULL == send)
	{
		igmp_snp_syslog_dbg("send_msg: malloc memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(send,0,sizeof(struct igmp_mng));
	send->nlh.nlmsg_type = IGMP_SNP_TYPE_NOTIFY_MSG;
	send->nlh.nlmsg_flags = msg_type; /*always be IGMP_SNP_FLAG_ADDR_MOD*/
	send->nlh.nlmsg_len = sizeof(struct nlmsghdr) + datalen;
	memcpy(&(send->igmp_noti_npd),data,datalen);

	igmp_debug_print_notify(send);
	/*
	if( 0 != npdmng_fd )
		write(npdmng_fd,(char *)(send),sizeof(struct nlmsghdr)+datalen);
	*/

	tmp_fd = recv_fd;
	while(cmdLen != send_len)
	{
		igmp_snp_syslog_dbg("Before Sendto by socket fd = %d.",recv_fd);
		rc = sendto(recv_fd,( char* )send,sizeof(struct igmp_mng),MSG_DONTWAIT,
							(struct sockaddr *)&remote_addr, sizeof(remote_addr));
		igmp_snp_syslog_dbg("After Sendto by socket fd = %d.",recv_fd);
		if(rc < 0)
		{
			if(errno == EINTR)/*send() be interrupted.*/
			{
				
				igmp_snp_syslog_err("npd sendto is interrupted");
				continue;
			}
			else if(errno == EACCES)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("send() permission is denied on the destination socket file");
				break;
			}
			else if(errno == EWOULDBLOCK)/*send()*/
			{
				igmp_snp_syslog_err("igmp2npdsendto1");
				break;
			}
			else if(errno == EBADF)
			{
				igmp_snp_syslog_err("igmp2npdsendto2");
				break;
			}
			else if(errno == ECONNRESET)
			{
				igmp_snp_syslog_err("igmp2npdsendto3");
				break;
			}
			else if(errno == EDESTADDRREQ)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto4");
				break;
			}
			else if(errno == EFAULT)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto5");
				break;
			}
			else if(errno == EINVAL)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto6");
				break;
			}
			else if(errno == EISCONN)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto7");
				break;
			}
			else if(errno == EMSGSIZE)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto8");
				break;
			}
			else if(errno == ENOBUFS)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto9");
				break;
			}
			else if(errno == ENOMEM)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto10");
				break;
			}
			else if(errno == ENOTCONN)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto11");
				break;
			}
			else if(errno == ENOTSOCK)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto12");
				break;
			}
			else if(errno == EOPNOTSUPP)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto13");
				break;
			}
			else if(errno == EPIPE)/*send() permission is denied on the destination socket file*/
			{
				igmp_snp_syslog_err("igmp2npdsendto14");
				break;
			}
			else {
				igmp_snp_syslog_dbg("Igmp Snp Cmd Write To Npd Fail.\n");
				return IGMPSNP_RETURN_CODE_ERROR;
			}
		}
		else{
			send_len += rc;
		}
		igmp_snp_syslog_dbg("End Sendto by socket fd = %d.",recv_fd);
	}
	if(send_len == cmdLen) {
		rc = IGMPSNP_RETURN_CODE_OK;	
	}
	free(send);
	igmp_snp_syslog_dbg("Send MSG:msg_type:%s\n",
				(msg_type == IGMP_SNP_FLAG_ADDR_MOD)?"addr_mod":"UNKNOWN");
	return rc;
}
/*******************************************************************************
 * igmp_snp_device_event
 *
 * DESCRIPTION:
 * 		device event handle  		
 *
 * INPUTS:
 * 		event  - which event will be handled
 *		ptr - device info
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   - on success
 * 		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - alloc memery fail
 *		IGMPSNP_RETURN_CODE_NULL_PTR - pointer ptr is null
 *
 * COMMENTS:
 *      	before proccess the event, check the vlan &(|)port igmp enabled.
 **
 ********************************************************************************/
LONG igmp_snp_device_event( ULONG event, dev_notify_msg* ptr )
{

	ULONG ulIfIndex,ret=0;
	ULONG ulVlanId[IGMP_SNP_GRP_MAX] = {0};
	INT   i,vCount = 0;
	if ( NULL == ptr )
	{
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	igmp_snp_syslog_dbg("igmp_snp_device_event: event:%ld\n",event);
	switch ( event )
	{
		case IGMPSNP_EVENT_DEV_UP:
			/*
			igmp_get_vid_by_port(ptr->ifindex,ulVlanId,&vCount);
			for(i=0;i<vCount,i++){
				Igmp_Event_PortUp(ulVlanId[i],ptr->ifindex);
			}
			*/
			break;
		case IGMPSNP_EVENT_DEV_DOWN:
			{
				igmp_snoop_pkt * pPkt = NULL;  

				if ( IGMPSNP_RETURN_CODE_OK != igmp_is_vlan_ifindex( ptr->vlan_id, ptr->ifindex )){
					/*check vlan & port not eanbled igmp*/
					return IGMPSNP_RETURN_CODE_ERROR;
				}

				ulIfIndex =ptr->ifindex;	       
				pPkt = ( igmp_snoop_pkt * )malloc( sizeof( igmp_snoop_pkt ));
				if( NULL == pPkt )
				{
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset( pPkt, 0, sizeof( igmp_snoop_pkt ) );
				/*pPkt->vlan_id = ptr ->vlan_id;*/ /*every vlan the port belonged to*/
				pPkt->ifindex =  ulIfIndex;
				pPkt->type = IGMP_PORT_DOWN;
				igmp_snp_syslog_dbg("igmp_snp_device_event: PORT_DOWN message enter.\n");
				/*call proccess function*/
				Igmp_Event_PortDown(pPkt);/*free(pPkt) in func*/
			}
			break;
		case IGMPSNP_EVENT_DEV_REGISTER:
			ret = 0;
			igmp_vlanisused(ptr->vlan_id,&ret);
			if(0 == ret)
			{
				/*vlan not enabled igmp*/
				Igmp_Event_AddVlan(ptr->vlan_id);
			}
			break;
		case IGMPSNP_EVENT_DEV_UNREGISTER:
			/*if (igmp_is_vlan_ifindex( ptr->vlan_id, ptr->ifindex ))*/
			igmp_vlanisused(ptr->vlan_id,&ret);
			if(1 == ret)
			{
				igmp_snoop_pkt* pPkt = NULL;
				pPkt = ( igmp_snoop_pkt * ) malloc( sizeof( igmp_snoop_pkt ));
				if ( NULL == pPkt )
				{
					igmp_snp_syslog_dbg("igmp_snp_device_event: failed in alloc mem for pPkt.\n");
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset( pPkt,0, sizeof( igmp_snoop_pkt ) );

				pPkt->vlan_id= ptr->vlan_id;
				/*pPkt->ifindex =  ptr->ifindex;*/ /*no use*/
				igmp_snp_syslog_dbg ("igmp_snp_device_event: DEL_VLAN: ulIfIndex %x,lVid  %x\n", ptr->ifindex, pPkt->vlan_id);
				pPkt->type = IGMP_VLAN_DEL_VLAN;
				/*call proccess function*/
				Igmp_Event_DelVlan(pPkt);/*free(pPkt) in func*/
				free( pPkt );
			}
			break;
		case IGMPSNP_EVENT_DEV_VLANADDIF:
			if (IGMPSNP_RETURN_CODE_NOTENABLE_PORT == igmp_is_vlan_ifindex(ptr->vlan_id,ptr->ifindex))/*not find port,so add it*/
			{
				ulIfIndex = ptr->ifindex;
				igmp_snoop_pkt* pPkt = NULL;

				pPkt = ( igmp_snoop_pkt * )malloc( sizeof( igmp_snoop_pkt ));
				if ( NULL == pPkt )
				{
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset( pPkt, 0,sizeof( igmp_snoop_pkt ) );
				pPkt->vlan_id= ptr->vlan_id;
				pPkt->ifindex =  ulIfIndex;
				pPkt->type = IGMP_VLAN_ADD_PORT;
				igmp_snp_syslog_dbg("igmp_snp_device_event: ADD_PORT:ulIfIndex %d,lVid  %d\n", pPkt->ifindex, pPkt->vlan_id);
				Igmp_Event_VlanAddPort(pPkt);/* no free(pPkt) in func*/
				free(pPkt);
			}
			break;
		case IGMPSNP_EVENT_DEV_VLANDELIF:
			if ( IGMPSNP_RETURN_CODE_OK == igmp_is_vlan_ifindex( ptr->vlan_id,ptr->ifindex ))
			{
				ulIfIndex = ptr->ifindex;
				igmp_snoop_pkt* pPkt = NULL;

				pPkt = ( igmp_snoop_pkt * )malloc( sizeof( igmp_snoop_pkt ));
				if ( NULL == pPkt )
				{
					return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
				}
				memset( pPkt, 0,sizeof( igmp_snoop_pkt ) );
				pPkt->vlan_id= ptr->vlan_id;
				pPkt->ifindex =  ulIfIndex;
				pPkt->type = IGMP_VLAN_DEL_PORT;
				igmp_snp_syslog_dbg("igmp_snp_device_event: DEL_PORT:ulIfIndex %x,lVid  %x\n", pPkt->ifindex, pPkt->vlan_id);
				Igmp_Event_VlanDelPort(pPkt);/*free(pPkt) in func*/
			}
			break;
		case IGMPSNP_EVENT_DEV_MCROUTER_PORT_UP:
			if(IGMPSNP_RETURN_CODE_OK == igmp_is_vlan_ifindex( ptr->vlan_id,ptr->ifindex )){
				Igmp_Event_AddRouterPort(ptr->vlan_id,ptr->ifindex);
			}
			break;
		case IGMPSNP_EVENT_DEV_MCROUTER_PORT_DOWN:
			if(IGMPSNP_RETURN_CODE_OK == igmp_is_vlan_ifindex( ptr->vlan_id,ptr->ifindex )){
				Igmp_Event_DelRouterPort(ptr->vlan_id,ptr->ifindex);
			}
			break;
		case IGMPSNP_EVENT_DEV_SYS_MAC_NOTI:
			Igmp_Event_SysMac_Get(ptr->sys_mac);		
			break;
		default:
			break;
		}
	return IGMPSNP_RETURN_CODE_OK;

}


/*************************************************************************
 *  Igmp_Snoop_IF_Recv_Is_Report_Discard
 *
 * DESCRIPTION:
 *  		If the packet is received on a port,which port is mcgroupVlan's reporter
 *  		Port.It'll discard the packet received.
 * 
 * INPUTS:
 *		lVid - vlan id
 *		ulIfIndex - interface index
 *
 * OUTPUTS:
 *	
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - no a reporter port
 *		IGMPSNP_RETURN_CODE_ERROR - search reporter list failed or is a report port
 * 
 ***************************************************************************/
LONG Igmp_Snoop_IF_Recv_Is_Report_Discard( LONG lVid , ULONG ulIfIndex )
{
	igmp_reporter_port * pReporter = NULL;
	member_port_list *pMemberPort = NULL;
	
	if ( IGMPSNP_RETURN_CODE_OK != igmp_snp_searchreporterlist( lVid, 0, IGMP_PORT_QUERY, &pReporter ) )
	{
		igmp_snp_syslog_dbg("Igmp_Snoop_IF_Recv_Is_Report_Discard: search reporter list failed\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	if ( NULL != pReporter ) 	/*There is some member port in vlan*/
	{
		pMemberPort = pReporter->portlist;
		while ( NULL != pMemberPort )
		{
			if ( ulIfIndex == pMemberPort->ifindex )
			{
				igmp_snp_syslog_err("receive packet from a report of vlan, dicard!\n");			
				return IGMPSNP_RETURN_CODE_ERROR; /*this is a mem port,so discard the pkt*/
			}
			pMemberPort = pMemberPort->next;
		}
	}
	return IGMPSNP_RETURN_CODE_OK;
}
#ifdef __cplusplus
}
#endif


