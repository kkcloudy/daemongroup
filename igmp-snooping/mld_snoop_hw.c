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
*  		$Revision: 1.2 $
*
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "mld_snoop_main.h"
#include "mld_snoop_inter.h"
#include "mld_snoop.h"
#include "igmp_snoop_com.h"
#include "igmp_snoop_inter.h"
#include "igmp_snp_dbus.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"


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
VOID mld_debug_print_notify(struct mld_mng *send)
{
	igmp_snp_syslog_dbg("######################NOTIFY message##############\n");
	if( MLD_SNP_FLAG_ADDR_MOD == send->nlh.nlmsg_flags )
	{
		mld_notify_mod_pkt *send_noti = &(send->mld_noti_npd);
		
		igmp_snp_syslog_dbg("Message Type:%d[%s]\n",
			send->nlh.nlmsg_flags,"MLD_SNP_FLAG_ADDR_MOD");
		igmp_snp_syslog_dbg("-----------------------------------------\n");
		switch( send_noti->mod_type)
		{
			case MLD_ADDR_ADD:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"MLD_ADDR_ADD");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"MLD_ADDR_ADD":"UNKNOWN");
				break;
			case MLD_ADDR_DEL:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"MLD_ADDR_DEL");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"MLD_ADDR_DEL":"UNKNOWN");
				break;
			case MLD_ADDR_RMV:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"MLD_ADDR_RMV");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"MLD_ADDR_RMV":"UNKNOWN");
				break;
			case MLD_SYS_SET:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"MLD_SYS_SET");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(MLD_SYS_SET_STOP == send_noti->reserve)?"MLD_SYS_SET_STOP":"MLD_SYS_SET_INIT");
				break;
			case MLD_TRUNK_UPDATE:
				igmp_snp_syslog_dbg("Mod_type:0x%x[%s]\n",
						send_noti->mod_type,"MLD_TRUNK_UPDATE");
				igmp_snp_syslog_dbg("Reserve:0x%x[%s]\n",
						send_noti->reserve,
						(send_noti->mod_type == send_noti->reserve)?"MLD_TRUNK_UPDATE":"UNKNOWN");
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
		igmp_snp_syslog_dbg("sip %4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x\n", ADDRSHOWV6(send_noti->groupadd));
	}
	else
	{
		igmp_snp_syslog_dbg("Message Type:%d[%s]\n",
			send->nlh.nlmsg_flags,"Unknown");
	}
	igmp_snp_syslog_dbg("##################################################\n");
}



INT mld_getvlan_addr(ULONG vlan_id, USHORT *vlan_addr)
{
	//DEBUG_OUT("igmp_getvlan_addr:excuted successful.\n");
	return IGMPSNP_RETURN_CODE_ERROR;
}


/**************************************************************************
* mld_L2mc_Entry_GetVidx()
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
LONG mld_L2mc_Entry_GetVidx(  ULONG ulVlanId,USHORT *uGroupAddr,ULONG* ulVidx )
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
			(MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp->ulGroupv6, uGroupAddr)) 
			/*&&( IGMP_SNOOP_OK == igmp_check_ip_from_mac (pstTemp->ulGroup,pMac))*/)
		{
			ulVlanIdx= pstTemp->ulVlanIdx;
			ulRet = IGMPSNP_RETURN_CODE_OK;
			break;
		}
		if( pstTemp == pstTemp->next )	/*can not find*/
		{
			if( (pstTemp->ulVlanId == ulVlanId) &&
				(MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp->ulGroupv6, uGroupAddr)))
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


/**************************************************************************
* mld_L2mc_Entry_Insert()
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
LONG mld_L2mc_Entry_Insert( mld_snoop_pkt * pPkt )
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
			
			if ( ( MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp ->ulGroupv6, pPkt->groupadd) ) && 
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
					 ( MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp ->ulGroupv6, pPkt->groupadd) ) && 
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
			USHORT *pstIp6 = pstL2mcEntry ->ulGroupv6;
			mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
			pstL2mcEntry ->ulIndex = pPkt->ifindex;
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
		USHORT *pstIp6 = pstL2mcEntry ->ulGroupv6;
		mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));
		pstL2mcEntry ->ulVlanId = pPkt->vlan_id;
		pstL2mcEntry->ulVlanIdx = pPkt->group_id;
		p_l2mc_list_head.stListHead = pstL2mcEntry;
		p_l2mc_list_head.ulListNodeCount = 1;
	}
	return IGMPSNP_RETURN_CODE_OK;
}


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
LONG mld_L2mc_Entry_Node_Delete( mld_snoop_pkt * pPkt )
{
	igmp_snp_syslog_dbg("Enter: L2mc_Entry_Node_Delete::vlanId %d,vlanIdx %d,ifindex %d,groupIp %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
					pPkt->vlan_id,pPkt->group_id,pPkt->ifindex,ADDRSHOWV6(pPkt->groupadd));
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
		
		if ( (MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp ->ulGroupv6, pPkt->groupadd))	&& 
			 (pstTemp->ulVlanId == pPkt->vlan_id) )
		{
			if( (pstTemp->ulIndex == pPkt->ifindex ) && 
				(pstTemp->ulVlanIdx == pPkt->group_id) )
			{
				EntryExist = 1;
				pstL2mcEntry = pstTemp;
				igmp_snp_syslog_dbg("1 find out an l2mc_entry:vidx %d,groupIp %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,"\
					"vlan_id %d,ifindex %d.\n",\
					pstL2mcEntry->ulVlanIdx,ADDRSHOWV6(pstL2mcEntry->ulGroupv6),pstL2mcEntry->ulVlanId,pstL2mcEntry->ulIndex);
				break;
			}
		}
		//if( pstTemp == p_l2mc_list_head.stListHead )
		if( pstTemp == pstTemp->next)
		{
			/*can not find*/
			if((MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp ->ulGroupv6, pPkt->groupadd))&& 
			   (pstTemp->ulVlanId == pPkt->vlan_id) &&
			   (pstTemp->ulIndex == pPkt->ifindex) && 
			   (pstTemp->ulVlanIdx == pPkt->group_id))
			{
				EntryExist = 1;
				pstL2mcEntry = pstTemp;
				igmp_snp_syslog_dbg("2 find out an l2mc_entry:vidx %d,groupIp %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,vlan_id %d,ifindex %d.\n",\
					pstL2mcEntry->ulVlanIdx,ADDRSHOWV6(pstL2mcEntry->ulGroupv6),pstL2mcEntry->ulVlanId,pstL2mcEntry->ulIndex);
				break;
			}
			igmp_snp_syslog_dbg("can NOT find a l2mc_list entry: vlanId %d,vidx %d,ifindex %d,groupIp %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n",\
							pPkt->vlan_id,pPkt->group_id,pPkt->ifindex,ADDRSHOWV6(pPkt->groupadd));
			break;
		}
		pstTemp = pstTemp->next;
	
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
* mld_L2mc_Entry_Group_Delete()
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
LONG mld_L2mc_Entry_Group_Delete( mld_snoop_pkt * pPkt )
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
		igmp_snp_syslog_dbg("l2mc_list entry pstTemp@@%p,ulVlanId %d,ulVlanIdx %d,ulGroup %p,ulIndex %d.\n",\
			pstTemp,pstTemp->ulVlanId,pstTemp->ulVlanIdx,pstTemp->ulGroupv6,pstTemp->ulIndex);
		if ( lCount >= IGMP_SNP_GRP_MAX + 1 )
		{
			return IGMPSNP_RETURN_CODE_OUT_RANGE;
		}
		lCount++;


		if ((MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp->ulGroupv6, pPkt->groupadd)) &&
			(pstTemp->ulVlanId == pPkt->vlan_id) &&
			(pstTemp->ulVlanIdx == pPkt->group_id))
		{
			EntryExist = 1;
			pstL2mcEntry = pstTemp;
			
		}
		if( pstTemp == pstTemp->next ){  	/*can not find*/
			if ((MLD_RETURN_CODE_EQUAL == mld_compare_ipv6addr(pstTemp->ulGroupv6, pPkt->groupadd)) &&
				(pstTemp->ulVlanId == pPkt->vlan_id) &&
				(pstTemp->ulVlanIdx == pPkt->group_id)){

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
								pstL2mcEntry->ulVlanId,pstL2mcEntry->ulVlanIdx,pstL2mcEntry->ulGroupv6,pstL2mcEntry->ulIndex);
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
LONG mld_L2mc_Entry_Group_Delete_All( mld_snoop_pkt * pPkt )
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
LONG mld_send_msg_npd
(	
	VOID *data, /*struct mld_notify_mod_pkt*/
	INT datalen,/*sizeof(mld_notify_mod_pkt)*/
	ULONG msg_type
)
{
	struct mld_mng *send = NULL;
	fd_set wfds;
	int rc, send_len = 0,tmp_fd;
	int cmdLen = sizeof(struct mld_mng);
	int addrlen = sizeof(struct sockaddr);
	if( NULL == data )
	{
		igmp_snp_syslog_dbg("mld_send_msg_npd: null data. \n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}

	send = (struct mld_mng *)malloc(sizeof(struct mld_mng));
	if(NULL == send)
	{
		igmp_snp_syslog_dbg("send_msg: malloc memory failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(send,0,sizeof(struct mld_mng));
	send->nlh.nlmsg_type = MLD_SNP_TYPE_NOTIFY_MSG;
	send->nlh.nlmsg_flags = msg_type; /*always be IGMP_SNP_FLAG_ADDR_MOD*/
	send->nlh.nlmsg_len = sizeof(struct nlmsghdr) + datalen;
	memcpy(&(send->mld_noti_npd),data,datalen);

	mld_debug_print_notify(send);
	/*
	if( 0 != npdmng_fd )
		write(npdmng_fd,(char *)(send),sizeof(struct nlmsghdr)+datalen);
	*/

	tmp_fd = recv_fd;
	while(cmdLen != send_len)
	{
		igmp_snp_syslog_dbg("Before Sendto by socket fd = %d.",recv_fd);
		rc = sendto(recv_fd,( char* )send,sizeof(struct mld_mng),MSG_DONTWAIT,
							(struct sockaddr *)&remote_addr, sizeof(remote_addr));
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
				igmp_snp_syslog_dbg("mld Snp Cmd Write To Npd Fail.\n");
				return IGMPSNP_RETURN_CODE_ERROR;
			}
		}
		else{
			send_len += rc;
		}
		igmp_snp_syslog_dbg("Sendto by socket fd = %d.",recv_fd);
	}
	if(send_len == cmdLen) {
		rc = IGMPSNP_RETURN_CODE_OK;	
	}
	free(send);
	igmp_snp_syslog_dbg("Send MSG:msg_type:%s\n",
				(msg_type == MLD_SNP_FLAG_ADDR_MOD)?"addr_mod":"UNKNOWN");
	return rc;
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
LONG mld_notify_msg(mld_snoop_pkt * pPkt,ULONG ulArg )
{
	LONG ret = IGMPSNP_RETURN_CODE_OK;
	mld_notify_mod_pkt *send_noti = NULL;
	ULONG lVidx = 0;
	unsigned char pMac[6] = {0};
	int i=0;
	if( NULL == pPkt )
	{
		igmp_snp_syslog_err("igmp_notify_add: null pPkt \n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	send_noti = (mld_notify_mod_pkt *)malloc(sizeof(mld_notify_mod_pkt));
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
	USHORT *pstIp6 = send_noti->groupadd;
	mld_copy_ipv6addr(pPkt->groupadd, &(pstIp6));

	switch (ulArg){
		case MLD_ADDR_ADD:
		case MLD_ADDR_DEL:
		case MLD_ADDR_RMV:
			{
				//igmp_groupip2mac(send_noti->groupadd,pMac);
				//lVidx = L2mc_Entry_GetVidx(send_noti->vlan_id, pMac);
				/*
				if(1 == L2mc_Entry_GetVidx(send_noti->vlan_id, send_noti->groupadd,&lVidx)
					send_noti->reserve = lVidx;
				*/
				send_noti->reserve = pPkt->group_id;
				igmp_snp_syslog_dbg("event type %d,vlan_id %d,groupadd %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, vidx %d,ifindex %d.\n",\
					ulArg,send_noti->vlan_id,ADDRSHOWV6(send_noti->groupadd),send_noti->reserve,send_noti->ifindex);
			}	
			break;
		case MLD_SYS_SET:
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
	ret = mld_send_msg_npd((void *)send_noti,sizeof(mld_notify_mod_pkt),MLD_SNP_FLAG_ADDR_MOD );
	if( IGMPSNP_RETURN_CODE_OK != ret )
	{
		igmp_snp_syslog_dbg("mld_notify_add: send notify message failed. \n");
		free(send_noti);
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	//DEBUG_OUT("Igmp_notify_add:send message successful.\n");
	free(send_noti);
	return ret;
}


/**************************************************************************
* mld_snp_mod_addr()
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
LONG mld_snp_mod_addr( mld_snoop_pkt * pPkt, ULONG ulArg )
{
	LONG lRet = IGMPSNP_RETURN_CODE_OK;
	
	igmp_snp_syslog_dbg("START:mld_snp_mod_addr\n");

	if ( NULL == pPkt )
	{
		igmp_snp_syslog_err("parameter null pointer!\n");
		igmp_snp_syslog_dbg("END:mld_snp_mod_addr\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	igmp_snp_syslog_dbg("GroupId %d vid %d group %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x If %d arg %d\n",
	      						pPkt->group_id,pPkt->vlan_id, ADDRSHOWV6(pPkt->groupadd), pPkt->ifindex, ulArg);
	switch ( ulArg )
	{
		case MLD_ADDR_ADD:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			stPkt.vlan_id = lVid;
			stPkt.group_id= pMcGroup->mgroup_id;//It's the Vidx.Need a mechanism for assignment vidx!
			stPkt.groupadd= ulGroup;
			stPkt.ifindex = pMemberPort->ifindex;
			stPkt.type = IGMP_ADDR_ADD;
			*/
			lRet = mld_L2mc_Entry_Insert( pPkt );
			lRet = mld_notify_msg(pPkt,ulArg);
			break;
		case MLD_ADDR_DEL:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			stPkt.type = IGMP_ADDR_DEL;
			stPkt.ifindex = pPkt->ifindex;
			stPkt.groupadd= pMcGroup->MC_ipadd;
			stPkt.vlan_id= lVid;
			*/
			lRet = mld_L2mc_Entry_Node_Delete((VOID *)pPkt);/*added by wujh*/
			lRet = mld_notify_msg(pPkt,ulArg);
			break;
		case MLD_ADDR_RMV:
			/*details of igmp_snoop_pkt :pPkt*/
			/*
			pkt.vlan_id = pGroup->vid;
			pkt.groupadd= pGroup->MC_ipadd;
			pkt.type = IGMP_ADDR_RMV;
			pkt.ifindex = 0;
			pkt.group_id = pGroup->mgroup_id;
			*/
			lRet = mld_L2mc_Entry_Group_Delete((VOID *)pPkt);
			lRet = mld_notify_msg(pPkt,ulArg);
			break;
		case MLD_SYS_SET:
			if(MLD_SYS_SET_INIT == pPkt->type){
				/*
				t_pkt.vlan_id = 0;
				t_pkt.groupadd = 0;
				t_pkt.ifindex = 0;
				t_pkt.type = MLD_SYS_SET_INIT; //init
				*/
				lRet = mld_notify_msg(pPkt,ulArg);
				break;
			}
			if(MLD_SYS_SET_STOP == pPkt->type){
				/*
				t_pkt.vlan_id = 0;
				t_pkt.groupadd = 0;
				t_pkt.ifindex = 0;
				t_pkt.type = IGMP_SYS_SET_STOP;	
				*/
				lRet = mld_L2mc_Entry_Group_Delete_All((VOID *)pPkt);
				lRet = mld_notify_msg( pPkt, ulArg);
				break;
			}
		default:
			lRet = IGMPSNP_RETURN_CODE_ERROR;
	}

	igmp_snp_syslog_dbg("END:mld_snp_mod_addr\n");
	return lRet;
}



#ifdef __cplusplus
}
#endif

