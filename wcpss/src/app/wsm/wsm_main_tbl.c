/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_main_tbl.c
*
*
* DESCRIPTION:
*  The functions about WTP-BSS-STA tables, Add, delete, or modify.There tables 
*  will be used when parse frame.  
*
* DATE:
*  2008-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2009-12-14 <guoxb> Add Inter-AC roaming table operation functions.
*  2009-12-29 <guoxb> Review source code, almost re-write all of table 
*                                operation function.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

/* system public header file */
#include <stdio.h>
#include <syslog.h>

/* wcpss public library header file */
#include "wcpss/waw.h"

/* private header file */
#include "wsm_main_tbl.h"
#include "wsm_wifi_bind.h"
#include "wsm_types.h"


extern WTP_Element WTPtable[HASHTABLE_SIZE];	
extern BSSID_BSSIndex_Element* BSSID_BSSIndex_table[HASHTABLE_SIZE / 4];
extern WTPIP_WTPID_Element* WTPIP_WTPID_table[HASHTABLE_SIZE];
extern struct WSM_WLANID_BSSID_TABLE WLANID_BSSID_Table[WIFI_BIND_MAX_NUM];
/* Inter-AC roaming */
extern roam_tbl_t *roam_tbl[HASHTABLE_SIZE / 4];
extern unsigned int total_sta_cnt;
extern unsigned int total_wtp_cnt;

extern __inline__ void WSMLog(unsigned char type, char *format, ...);


/**
* Description:
*  Initialize wsm module global WTP-BSS-STA tables.
*
* Parameter:
*  void.
*
* Return:
*  void
*
*/
void wsm_tbl_init(void)
{
	int i = 0;
	
	for(i = 0; i < HASHTABLE_SIZE; i++)
		bzero(&WTPtable[i], sizeof(WTP_Element));
		
	for(i = 0; i < HASHTABLE_SIZE / 4; i++)
		BSSID_BSSIndex_table[i] = NULL;
		
	for(i = 0; i < HASHTABLE_SIZE; i++)
		WTPIP_WTPID_table[i] = NULL;

	for (i = 0; i < HASHTABLE_SIZE / 4; i++)
		roam_tbl[i] = NULL;
		
	wifi_bind_init();
}

/**
* Description:
*  Generate hash key from IP addr.
*
* Parameter:
*  ip: IP addr, IPv4 or IPv6 addr pointer.
*
* Return:
*  -1: error; 
*  >=0 :Hash key.
*
*/
int wsm_tbl_ip_key(struct mixwtpip *ip,unsigned wtpindex)
{
	unsigned int ip_addr = 0;
	unsigned short port = 0;
	
	if (!ip)
	{
		WSMLog(L_ERR, "%s: Parameter p = NULL.\n", __func__);
		return -1;
	}
	
	if (ip->addr_family == AF_INET)
	{
		ip_addr = ip->m_v4addr;
		port = ip->port;
		
		return ((ip_addr >> 16) ^ ip_addr ^ wtpindex) & TBL_MASK;
	}
	else if (ip->addr_family == AF_INET6)
	{
		port = ip->port;
		ip_addr = *((unsigned int*)&ip->m_v6addr[12]);
		
		return ((ip_addr >> 16) ^ ip_addr ^ wtpindex) & TBL_MASK;
	}
	else
	{
		WSMLog(L_ERR, "%s: socket family[%d] not ipv4 or ipv6.\n", 
				__func__, ip->addr_family);
		return -1;
	}
	
}

int convert_wAW_BSS_to_wAW_BSS_r(wAW_BSS_r *pDst, wAW_BSS *pSrc)
{
	if(NULL == pDst || NULL == pSrc){
		WSMLog(L_ERR, "%s: convert wAW_BSS to wAW_BSS_r failed.\n", __func__);
		return -1;
	}
	
	pDst->BSSIndex = pSrc->BSSIndex;
	pDst->Radio_L_ID = pSrc->Radio_L_ID;
	pDst->Radio_G_ID = pSrc->Radio_G_ID;
	pDst->WlanID = pSrc->WlanID;
	memcpy(pDst->BSSID, pSrc->BSSID, MAC_LEN);
	pDst->bss_max_sta_num = pSrc->bss_max_sta_num;
	pDst->nas_id_len = pSrc->nas_id_len;
	pDst->protect_type = pSrc->protect_type;
	pDst->bss_ifaces_type = pSrc->bss_ifaces_type;
	pDst->wlan_ifaces_type = pSrc->wlan_ifaces_type;

	return 0;
	
}


/**
* Description:
*  WTP or BSS or STA add function.
*
* Parameter:
*  type: element type, WTP or BSS or STA.
*  pdata: element pointer.
*
* Return:
*  0: failed.
*  1: Successed.
*
*/
int wsm_tbl_add(int type,  void *pdata)
{
	int index = -1;


	if (pdata == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter pdata == NULL, error.\n", __func__);
		return 0;
	}
	
	if (type == WTP_TYPE)
	{
		int WTPip_WTPid_index = -1;
		wWSM_DataChannel *p = (wWSM_DataChannel *)pdata;
		
		index = p->WTPID;

		if (index < 0 || index >= HASHTABLE_SIZE)
		{
			WSMLog(L_ERR, "%s: Add WTP, WTP index = %d, unsupport.\n", 
					__func__, index);
			return 0;
		}
		
		WSMLog(L_DEBUG, "%s: Add WTP[%u].\n", __func__, index);

		/* Empty WTP table, if flag==1, this WTP is exist, else this WTP is empty */
		if (WTPtable[index].flag == 0)
		{
			bzero(&WTPtable[index], sizeof(WTP_Element));
			memcpy(&WTPtable[index].WTP, p, sizeof(wWSM_DataChannel));
			
			if (p->WTPIP.addr_family == AF_INET)
			{
				WSMLog(L_DEBUG, "%s: Add ipv4 WTP. IP=%x\n", 
						__func__, p->WTPIP.m_v4addr);
				WTPtable[index].addr_family = AF_INET;
			}
			else if (p->WTPIP.addr_family == AF_INET6)
			{
				WSMLog(L_DEBUG, "%s: Add ipv6 WTP.\n", __func__);
				WTPtable[index].addr_family = AF_INET6;
			}
			else
			{
				WSMLog(L_ERR, "%s: ADD WTP addr type[%d] not ipv4 or ipv6.\n", 
						__func__, p->WTPIP.addr_family);
				return 0;
			}
			
			WTPtable[index].flag = 1;
			
			if (wsm_wtpip_wtpid_tbl_add(p) == 0)
			{
				WSMLog(L_ERR, "%s: Add WTP in WTPtable success, "
						"but Add WTP in WTPIP_WTPID table failed.\n", __func__);
				return 0;
			}
		}
		else /* WTP table is not empty */
		{
			WSMLog(L_ERR, "%s: Add WTP[%d] collision.\n", __func__, index);
			return 0;
		}
			
		#ifdef WSM_TABLE_PRINT
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		wsm_tbl_print(WTPIP_WTPID_TYPE);
		#endif
			
		return 1;
	}
	

	else if (type == BSS_TYPE)
	{
		wAW_BSS *p = (wAW_BSS *)pdata;	
		struct wsm_bssid_wlanid tmpdata;
		struct wsm_wlanid_bssid_element wlandata;
		unsigned int BSSIndex = 0;
		unsigned int WTPID = 0;
		unsigned int BSS_Local_Index = 0;
		BSS_Element *pBSS = NULL;

		WSMLog(L_DEBUG, "%s: Add BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, p->BSSID[0], p->BSSID[1], p->BSSID[2], p->BSSID[3], 
				p->BSSID[4], p->BSSID[5]);

		/**
			WTPID = BSSIndex / BSS_ARRAY_SIZE
			BSS_Local_index = BSSIndex % BSS_ARRAY_SIZE
			It's fixed relationship.
		*/
		BSSIndex = p->BSSIndex;
		WTPID = BSSIndex / BSS_ARRAY_SIZE;
		BSS_Local_Index = BSSIndex % BSS_ARRAY_SIZE;
		pBSS = NULL;

		bzero(&tmpdata, sizeof(tmpdata));
		tmpdata.WlanID = p->WlanID;
		tmpdata.Radio_L_ID = p->Radio_L_ID;
		tmpdata.protect_type = p->protect_type;
		tmpdata.bss_if_type = p->bss_ifaces_type;
		tmpdata.wlan_if_type = p->wlan_ifaces_type;
		WSM_COPY_MAC(tmpdata.BSSID, p->BSSID);

		bzero(&wlandata, sizeof(wlandata));
		wlandata.WlanID = p->WlanID;
		wlandata.BSSIndex = p->BSSIndex;
		wlandata.bss_if_type = p->bss_ifaces_type;
		wlandata.wlan_if_type = p->wlan_ifaces_type;
		wlandata.protect_type = p->protect_type;
		WSM_COPY_MAC(wlandata.BSSID, p->BSSID);
									
		if (WTPtable[WTPID].flag == 0)
		{
			WSMLog(L_ERR, "%s: ADD BSS failed, WTP[%u] is nonexistent.\n",
					__func__, WTPID);	
			return 0;
		}
				
		if (WTPtable[WTPID].next == NULL)
		{
			WTPtable[WTPID].next = (BSS_Element *)malloc(BSS_ARRAY_SIZE * sizeof(BSS_Element));
			if (WTPtable[WTPID].next == NULL)
			{
				WSMLog(L_CRIT, "%s: Add BSS malloc buffer failed.\n", __func__);
				return 0;
			}
			bzero(WTPtable[WTPID].next, sizeof(BSS_Element) * BSS_ARRAY_SIZE);
		}
		
		pBSS = WTPtable[WTPID].next;
		pBSS += BSS_Local_Index;
		/* flag == 0, BSS not exist; flag == 1, BSS is exist */
		if(pBSS->flag == 1)
		{
			WSMLog(L_ERR, "%s: Add BSS failed, WTP[%u] already have "
					"BSSID[%02x:%02x:%02x:%02x:%02x:%02x].\n",
					__func__, WTPID, p->BSSID[0], p->BSSID[1], p->BSSID[2],
					p->BSSID[3], p->BSSID[4], p->BSSID[5]);	
			return 0;
		}
		
		WTPtable[WTPID].count_BSS++;
		#if 0
		memcpy(&(pBSS->BSS), p, sizeof(wAW_BSS));
		#else 
		//API for convert wAW_BSS to wAW_BSS_r		
		if(0 != convert_wAW_BSS_to_wAW_BSS_r(&(pBSS->BSS), p)){
			WSMLog(L_ERR, "convert wAW_BSS to wAW_BSS_r error.\n");
			return 0;
		}
		#endif
		pBSS->flag = 1;
		pBSS->next = NULL;

		if(wsm_bssid_bssidx_tbl_add(p) == 0)
		{
			WSMLog(L_ERR, "%s: Add BSS to WTPtable ok, "
					"but Add to BSSID_BSSIndex failed.\n", __func__);
			return 0;
		}
		
		if (wsm_wifi_table_add((unsigned char*)&tmpdata, WSM_BSSID_WLANID_TYPE))
		{
			WSMLog(L_ERR, "%s: Add BSS to WSM_BSSID_WLANID failed.\n", __func__);
		}

		if (wsm_wifi_table_add((unsigned char*)&wlandata, WSM_WLANID_BSSID_TYPE))
		{
			WSMLog(L_ERR, "%s: Add BSS to WSM_WLANID_BSSID failed.\n", __func__);
		}
			
		#ifdef WSM_TABLE_PRINT
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		wsm_tbl_print(BSSID_BSSIndex_TYPE);
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
		wsm_wifi_table_print(WSM_WLANID_BSSID_TYPE);
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		#endif
	
		return 1;
	}
	

	else if(STA_TYPE == type)
	{
		aWSM_STA *p = (aWSM_STA *)pdata;
		unsigned int BSSIndex = p->BSSIndex;
		unsigned int WTPID = p->WTPID;
		unsigned int BSS_Local_Index = BSSIndex % BSS_ARRAY_SIZE;
		struct wsm_stamac_bssid_wtpip tmpdata;
		STAStatistics stadata;
		BSS_Element *pBSS = NULL;
		STA_Element *pmalloc = NULL;

		WSMLog(L_DEBUG, "%s: Add STA[%02x:%02x:%02x:%02x:%02x:%02x], WTP[%d]\n",
				__func__, p->STAMAC[0], p->STAMAC[1], p->STAMAC[2], p->STAMAC[3], 
				p->STAMAC[4], p->STAMAC[5], WTPID);
					
		bzero(&tmpdata, sizeof(tmpdata));
		bzero(&stadata, sizeof(stadata));

		/* WTP must exist and has BSS buffer */
		if (WTPtable[WTPID].flag == 0 || WTPtable[WTPID].next == NULL)
		{
			WSMLog(L_ERR, "%s: Add STA[%02x:%02x:%02x:%02x:%02x:%02x] to empty"
					" WTP[%u] or no BSS.\n", __func__, p->STAMAC[0], p->STAMAC[1], 
					p->STAMAC[2], p->STAMAC[3], p->STAMAC[4], p->STAMAC[5], WTPID);
			return 0;
		}
			
		pBSS = WTPtable[WTPID].next;
		pBSS += BSS_Local_Index;

		/* Corresponding BSS is not exist */
		if (pBSS->flag == 0)
		{
			WSMLog(L_ERR, "%s: Add STA[%02x:%02x:%02x:%02x:%02x:%02x] to "
					"empty BSS.[WTP:%u, BSSIndex:%u].\n", __func__, p->STAMAC[0],
					p->STAMAC[1], p->STAMAC[2], p->STAMAC[3], p->STAMAC[4], 
					p->STAMAC[5], WTPID, BSS_Local_Index);
			return 0;
		}

		pmalloc = (STA_Element *)malloc(sizeof(STA_Element));
		if (pmalloc == NULL)
		{
			WSMLog(L_CRIT, "%s: malloc STA buffer failed.\n", __func__);
			return 0;
		}

		memcpy(&pmalloc->STA, p, sizeof(aWSM_STA));
		pmalloc->next = pBSS->next;
		pBSS->next = pmalloc;
		WTPtable[WTPID].sta_cnt++;

		/* Add STA to stamac_bssid_wtpip table */
  		tmpdata.STAState = p->StaState;
		tmpdata.WTPID = WTPID;
		/* Inter-AC roaming data */
		tmpdata.ac_roam_tag = p->ac_roam_tag;
		memcpy(&tmpdata.ac_ip, &p->ac_ip, sizeof(struct mixwtpip));
		WSM_COPY_MAC(tmpdata.BSSID, pBSS->BSS.BSSID);
		WSM_COPY_MAC(tmpdata.STAMAC, p->STAMAC);
		wsm_wifi_table_add((unsigned char*)&tmpdata, WSM_STAMAC_BSSID_WTPIP_TYPE);
	
		stadata.BSSIndex = pBSS->BSS.BSSIndex;
		stadata.Radio_G_ID = pBSS->BSS.Radio_G_ID;
		stadata.WTPID = WTPID;
		WSM_COPY_MAC(stadata.STAMAC, p->STAMAC);
		wsm_wifi_table_add((unsigned char*)&stadata, WSM_STA_Statistics_TYPE);
		
			
		#ifdef WSM_TABLE_PRINT
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		#endif
			
		return 1;
	}

	else
	{
		WSMLog(L_ERR, "%s: ADD unknown type[%d].\n", __func__, type);
		return 0;
	}

}

/**
* Description:
*  Delete WTP or BSS or STA from WTP-BSS-STA relational tables.
*
* Parameter:
*  type: WTP, BSS or STA
*  pdata: element data.
*
* Return:
*  0 : failed; 1: Successed.
*
*/
int wsm_tbl_del(int type,  void *pdata)
{
	int index = -1;

	if (pdata == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter pdata==NULL, error.\n", __func__);
		return 0;
	}

	if (type == WTP_TYPE)
	{
		int WTPip_WTPid_index = -1, i = 0;
		BSS_Element *pBSS_buffer = NULL;
		BSS_Element *pBSS = NULL;
		STA_Element *p_STA = NULL;
		STA_Element *p_tmp_STA = NULL;
		struct wsm_wlanid_bssid_element wlandata;
		wWSM_DataChannel *p = (wWSM_DataChannel *)pdata;
		wAW_BSS BSS;

		index = p->WTPID;
		WSMLog(L_DEBUG, "%s: Delete WTP[%u].\n", __func__, index);

		if (WTPtable[index].flag == 0)
		{
			WSMLog(L_ERR, "%s: Delete nonexistent WTP[%u].\n", __func__, index);
			return 0;
		}

		/* Keep wsm tables sync, reset pointer p */
		p = &WTPtable[index].WTP;
		
		if (wsm_wtpip_wtpid_tbl_del(p) == 0)
		{
			WSMLog(L_ERR, "%s: Del WTP, but WTP is not sync in tables"
					" WTPtbl and WTPIP_WTPID.\n", __func__);
		}

		/* WTP not have BSS */
		if (WTPtable[index].count_BSS == 0)
		{
			bzero(&WTPtable[index], sizeof(WTP_Element));

			#ifdef WSM_TABLE_PRINT
			wsm_tbl_print(WTP_BSS_STA_TYPE);
			wsm_tbl_print(WTPIP_WTPID_TYPE);
			wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
			wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
			wsm_wifi_table_print(WSM_WLANID_BSSID_TYPE);
			#endif
					
			return 1;
		}
		
		pBSS_buffer = WTPtable[index].next;
		pBSS = pBSS_buffer;

		/* Delete relational BSSs and STAs */
		for (i = 0; i < BSS_ARRAY_SIZE; i++)
		{
			if (pBSS[i].flag == 0)
				continue;

			/* Delete BSS in relational tables */
			bzero(&BSS, sizeof(wAW_BSS));
			memcpy(BSS.BSSID, pBSS[i].BSS.BSSID, MAC_LEN);
			BSS.BSSIndex = pBSS[i].BSS.BSSIndex;
			wlandata.WlanID = pBSS[i].BSS.WlanID;
			WSM_COPY_MAC(wlandata.BSSID, pBSS[i].BSS.BSSID);
			wlandata.BSSIndex = pBSS[i].BSS.BSSIndex;
			
			wsm_wifi_table_del((unsigned char*)(&wlandata), WSM_WLANID_BSSID_TYPE);		
			wsm_bssid_bssidx_tbl_del(&BSS);
			wsm_wifi_table_del(pBSS[i].BSS.BSSID, WSM_BSSID_WLANID_TYPE);

			/* Delete relational STA */
			p_STA = pBSS[i].next;
			while (p_STA != NULL)
			{
				wsm_wifi_table_del(p_STA->STA.STAMAC, 
					WSM_STAMAC_BSSID_WTPIP_TYPE);
				wsm_wifi_table_del(p_STA->STA.STAMAC, WSM_STA_Statistics_TYPE);
				p_tmp_STA = p_STA;
				p_STA = p_STA->next;
				free(p_tmp_STA);
			}	
		}
		
		free(pBSS_buffer);
		bzero(&WTPtable[index], sizeof(WTP_Element));

		#ifdef WSM_TABLE_PRINT
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		wsm_tbl_print(WTPIP_WTPID_TYPE);
		wsm_tbl_print(BSSID_BSSIndex_TYPE);
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
		wsm_wifi_table_print(WSM_WLANID_BSSID_TYPE);
		#endif
			
		return 1;
	}
	
	else if (BSS_TYPE == type)
	{
		wAW_BSS *p = (wAW_BSS *)pdata;
		struct wsm_wlanid_bssid_element wlandata;			
		unsigned int BSSIndex = p->BSSIndex;
		unsigned int WTPID = BSSIndex / BSS_ARRAY_SIZE;
		unsigned int BSS_Local_Index = BSSIndex % BSS_ARRAY_SIZE;
		BSS_Element *pBSS_buffer = NULL;
		BSS_Element *pBSS = NULL;
		STA_Element *p_sta = NULL, *p_tmp_sta = NULL;
		
		WSMLog(L_DEBUG, "%s: Delete BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, p->BSSID[0], p->BSSID[1], p->BSSID[2], p->BSSID[3], 
				p->BSSID[4], p->BSSID[5]);

		if (WTPtable[WTPID].flag == 0)
		{
			WSMLog(L_ERR, "%s: Delete BSS[%02x:%02x:%02x:%02x:%02x:%02x], "
					"but WTP[%u] is nonexistent.\n", __func__, p->BSSID[0], 
					p->BSSID[1], p->BSSID[2], p->BSSID[3], p->BSSID[4], 
					p->BSSID[5], WTPID);
			return 0;
		}
		
		if (WTPtable[WTPID].next == NULL)
		{
			WSMLog(L_ERR, "%s: Delete BSS[%02x:%02x:%02x:%02x:%02x:%02x], "
					"but WTP[%u] not has BSS.\n", __func__, p->BSSID[0], 
					p->BSSID[1], p->BSSID[2], p->BSSID[3], p->BSSID[4], 
					p->BSSID[5], WTPID);
			return 0;
		}
			
		pBSS_buffer = WTPtable[WTPID].next;
		pBSS = pBSS_buffer + BSS_Local_Index;

		/* This BSS in nonexistent */
		if(pBSS->flag == 0)
		{
			WSMLog(L_ERR, "%s: Delete BSS[%02x:%02x:%02x:%02x:%02x:%02x] "
					"failed, WTP[%u] not has this BSS.\n", __func__, p->BSSID[0], 
					p->BSSID[1], p->BSSID[2], p->BSSID[3], p->BSSID[4], 
					p->BSSID[5], WTPID);
			return 0;
		}

		/* Get the STA list pointer of this BSS */
		p_sta = pBSS->next;

		/* Delete this BSS */
		wlandata.WlanID = pBSS->BSS.WlanID;
		wlandata.BSSIndex = pBSS->BSS.BSSIndex;
		memcpy(wlandata.BSSID, pBSS->BSS.BSSID, MAC_LEN);
		wsm_wifi_table_del((unsigned char*)(&wlandata), WSM_WLANID_BSSID_TYPE);
		wsm_wifi_table_del(pBSS->BSS.BSSID, WSM_BSSID_WLANID_TYPE);
		wsm_bssid_bssidx_tbl_del(&(pBSS->BSS));
		bzero(pBSS, sizeof(BSS_Element));
		WTPtable[WTPID].count_BSS--;

		/* If the WTP not has BSS, free BSS buffer */
		if(WTPtable[WTPID].count_BSS == 0)
		{
			free(pBSS_buffer);
			WTPtable[WTPID].next = NULL;
		}

		/* Delete STAs that are in this BSS */
		while (p_sta != NULL)
		{
			wsm_wifi_table_del(p_sta->STA.STAMAC, WSM_STAMAC_BSSID_WTPIP_TYPE);
			wsm_wifi_table_del(p_sta->STA.STAMAC, WSM_STA_Statistics_TYPE);
			p_tmp_sta = p_sta;
			p_sta = p_sta->next;
			free(p_tmp_sta);
			WTPtable[WTPID].sta_cnt--;
		}
		
		#ifdef WSM_TABLE_PRINT
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		wsm_tbl_print(BSSID_BSSIndex_TYPE);
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
		wsm_wifi_table_print(WSM_WLANID_BSSID_TYPE);
		#endif
			
		return 1;
	}


	else if (type == STA_TYPE)
	{
		aWSM_STA *p = (aWSM_STA *)pdata;
		STA_Element *p_pre = NULL, *p_next = NULL;
		unsigned int BSSIndex = p->BSSIndex;
		unsigned int WTPID = p->WTPID;
		unsigned int BSS_Local_Index = BSSIndex % BSS_ARRAY_SIZE;
		BSS_Element *pBSS = NULL;

		WSMLog(L_DEBUG, "%s: Delete STA[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, p->STAMAC[0], p->STAMAC[1], p->STAMAC[2], 
				p->STAMAC[3], p->STAMAC[4], p->STAMAC[5]);
		
		if (WTPtable[WTPID].flag == 0)
		{
			WSMLog(L_ERR, "%s: Delete STA[%02x:%02x:%02x:%02x:%02x:%02x] "
					"failed, WTP[%u] is nonexistent.\n", __func__, p->STAMAC[0], 
					p->STAMAC[1], p->STAMAC[2], p->STAMAC[3], p->STAMAC[4],
					p->STAMAC[5], WTPID);
			return 0;
		}
		
		if (WTPtable[WTPID].next == NULL)
		{
			WSMLog(L_ERR, "%s: Delete STA[%02x:%02x:%02x:%02x:%02x:%02x]"
					" failed, WTP[%u] not has BSS.\n", __func__, p->STAMAC[0], 
					p->STAMAC[1], p->STAMAC[2], p->STAMAC[3], p->STAMAC[4],
					p->STAMAC[5], WTPID);
			return 0;
		}
		
		pBSS = WTPtable[WTPID].next;
		pBSS += BSS_Local_Index;
		
		if (pBSS->flag == 0)
		{
			WSMLog(L_WARNING, "%s: Delete STA[%02x:%02x:%02x:%02x:%02x:%02x]"
					" failed, WTP[%u] not has corresponding BSS.\n", __func__, 
					p->STAMAC[0], p->STAMAC[1], p->STAMAC[2], 
					p->STAMAC[3], p->STAMAC[4], p->STAMAC[5], WTPID);
			return 0;
		}
		
		p_pre = pBSS->next;
		p_next = p_pre;

		while (p_next != NULL)
		{
			if (memcmp(p_next->STA.STAMAC, p->STAMAC, MAC_LEN) == 0)
			{
				/* This STA is list header */
				if (p_next == p_pre)
				{
					pBSS->next = p_next->next;
					wsm_wifi_table_del(p_next->STA.STAMAC, 
						WSM_STAMAC_BSSID_WTPIP_TYPE);
					wsm_wifi_table_del(p_next->STA.STAMAC, 
						WSM_STA_Statistics_TYPE);
					free(p_next);
					WTPtable[WTPID].sta_cnt--;

					return 1;
				}
				else
				{
					p_pre->next = p_next->next;
					wsm_wifi_table_del(p_next->STA.STAMAC, 
						WSM_STAMAC_BSSID_WTPIP_TYPE);
					wsm_wifi_table_del(p_next->STA.STAMAC, 
						WSM_STA_Statistics_TYPE);
					free(p_next);
					WTPtable[WTPID].sta_cnt--;

					return 1;
				}
			}
			p_pre = p_next;
			p_next = p_next->next;
		}
		
		WSMLog(L_ERR, "%s: Cannot find STA[%02x:%02x:%02x:%02x:%02x:%02x]"
					" in WTP[%u].\n", __func__, p->STAMAC[0], p->STAMAC[1], 
					p->STAMAC[2], p->STAMAC[3], p->STAMAC[4], p->STAMAC[5],
					WTPID);
					
		#ifdef WSM_TABLE_PRINT
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		wsm_tbl_print(WTP_BSS_STA_TYPE);
		#endif
			
		return 0;
	}

	else
	{
		WSMLog(L_ERR, "%s: unsupport type: %d\n", __func__, type);
		return 0;
	}
}

/**
* Description:
*  Modify WTP or BSS or STA data function. And now, this func just used for
*  Inter-AC roaming or Inter-WTP roaming.
*
* Parameter:
*  type: WTP or BSS or STA
*  pdata: element data.
*
* Return:
*  0 : Failed; 1 : Successed
*
*/
int wsm_tbl_modify(int type, void * pdata)
{
	if (pdata == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter pdata == NULL.\n", __func__);
		return 0;
	}
	
	if (type == STA_TYPE)
	{
		aWSM_STA *p = (aWSM_STA*)pdata;
		struct wsm_stamac_bssid_wtpip *sta_data = NULL;
		roam_sta_ele_t roam_sta;

		bzero(&roam_sta, sizeof(roam_sta_ele_t));
		WSMLog(L_DEBUG, "%s: STA_TYPE modify.\n", __func__);
		if ((sta_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
			p->STAMAC, WSM_STAMAC_BSSID_WTPIP_TYPE)) == NULL)
		{
			WSMLog(L_ERR, "%s: Cannot find this STA[%02x:%02x:%02x:%02x:%02x:%02x]"
					" in tables.\n", __func__, p->STAMAC[0], p->STAMAC[1], 
					p->STAMAC[2], p->STAMAC[3], p->STAMAC[4], p->STAMAC[5]);
			return 0;
		}

		if (sta_data->ac_roam_tag == NO_ROAMING)
		{
			if (p->ac_roam_tag == RAW_AC)
			{
				WSMLog(L_DEBUG, "%s: roam_tbl_add, ac_ip=%x.\n", __func__, p->ac_ip.m_v4addr);
				memcpy(&roam_sta.ac_ip, &p->ac_ip, sizeof(struct mixwtpip));
				memcpy(roam_sta.bssid, sta_data->BSSID, MAC_LEN);
				memcpy(roam_sta.stamac, sta_data->STAMAC, MAC_LEN);
				roam_tbl_add(&roam_sta);
			}
		} 
		else if (sta_data->ac_roam_tag == RAW_AC)
		{
			if (p->ac_roam_tag != RAW_AC)
			{
				WSMLog(L_DEBUG, "%s: roam_tbl_del\n", __func__);
				memcpy(&roam_sta.ac_ip, &sta_data->ac_ip, sizeof(struct mixwtpip));
				memcpy(roam_sta.bssid, sta_data->BSSID, MAC_LEN);
				memcpy(roam_sta.stamac, sta_data->STAMAC, MAC_LEN);
				roam_tbl_del(&roam_sta);
			}
		}
	
		memcpy(sta_data->BSSID_before, p->RBSSID, MAC_LEN);
		sta_data->roaming_flag = p->RoamingTag;
		/* Inter-AC roaming data */
		sta_data->ac_roam_tag = p->ac_roam_tag;
		memcpy(&sta_data->ac_ip, &p->ac_ip, sizeof(struct mixwtpip));

		return 1;		
	}

	else if (type == WLAN_TYPE)
	{
		wASD_WLAN *p = (wASD_WLAN*)pdata;
		unsigned char wlanid = (p->WlanID) - 1;
			
		WLANID_BSSID_Table[wlanid].Roaming_policy = p->Roaming_policy;
		return 1;
	}
	
	else
	{
		WSMLog(L_ERR, "%s: unsupport type: %d\n", __func__, type);
		return 0;
	}
}

/**
* Description:
*  Add element for WTPID_WTPIP table. This table will find WTPID by WTPIP.
*
* Parameter:
*  p: WTP element.
*
* Return:
*  0 : Failed; 1 : Successed.
*
*/
int wsm_wtpip_wtpid_tbl_add(wWSM_DataChannel *p)
{
	int index = 0;
	WTPIP_WTPID_Element *pmalloc = NULL;
	unsigned int wtpindex = p->WTPID;
	if ((index = wsm_tbl_ip_key(&(p->WTPIP),wtpindex)) < 0)
	{
		WSMLog(L_ERR, "%s: Get hash key = %d, error.\n", 
				__func__, index);
		return 0;
	}
	
	if((pmalloc = (WTPIP_WTPID_Element *)malloc(
			sizeof(WTPIP_WTPID_Element))) == NULL)
	{
		WSMLog(L_CRIT, "%s: Malloc failed.\n", __func__);
		return 0;
	}
	
	bzero(pmalloc, sizeof(WTPIP_WTPID_Element));
	pmalloc->next = WTPIP_WTPID_table[index];
	pmalloc->WTPID = p->WTPID;
	memcpy(&pmalloc->wtp_ip, &p->WTPIP, sizeof(struct mixwtpip));
	WTPIP_WTPID_table[index] = pmalloc;	
	total_wtp_cnt++;
	
	WSMLog(L_DEBUG, "%s: IP: %x port:%d\n", __func__, 
			p->WTPIP.m_v4addr, p->WTPIP.port);

	return 1;
}

/**
* Description:
*  Delete element from WTPID_WTPIP table.
*
* Parameter:
*  p: WTP element.
*
* Return:
*  0 : Failed ; 1 : Successed.
*
*/
int wsm_wtpip_wtpid_tbl_del(wWSM_DataChannel *p)
{
	int index = 0;					
	WTPIP_WTPID_Element *p_pre = NULL;
	WTPIP_WTPID_Element *p_next = NULL;

	if (!p)
	{
		WSMLog(L_ERR, "%s: Parameter p = NULL.\n", __func__);
		return 0;
	}
	unsigned int wtpindex = p->WTPID;
	if ((index = wsm_tbl_ip_key(&(p->WTPIP),wtpindex)) < 0)
	{
		WSMLog(L_ERR, "%s: Get hash key failed.\n", __func__);
		return 0;
	}

	p_pre = WTPIP_WTPID_table[index];
	p_next = p_pre;

	while (p_next != NULL)
	{
		if (p_next->WTPID == p->WTPID)
		{
			/* Delete the list header */
			if (p_next == p_pre)
			{
				WTPIP_WTPID_table[index] = p_next->next;
				free(p_next);
				total_wtp_cnt--;
				
				return 1;
			}
			else
			{
				p_pre->next = p_next->next;
				free(p_next);
				total_wtp_cnt--;

				return 1;
			}
		}
		p_pre = p_next;
		p_next = p_next->next;
	}
	
	return 0;
}

/**
* Description:
*  Add element for BSSID_BSSIndex table, from this table, you can find BSSIndex
*  by BSSID
*
* Parameter:
*  p: BSS element.
*
* Return:
*  0 : Failed ; 1 : Successed. 
*
*/
int wsm_bssid_bssidx_tbl_add(wAW_BSS *p)
{
	unsigned char index = wsm_tbl_mac_key(p->BSSID);
	BSSID_BSSIndex_Element *pmalloc = NULL;

	pmalloc = (BSSID_BSSIndex_Element *)malloc(sizeof(BSSID_BSSIndex_Element));
	if (pmalloc == NULL)
	{
		WSMLog(L_CRIT, "%s: Malloc failed.\n", __func__);
		return 0;
	}
	
	bzero(pmalloc, sizeof(BSSID_BSSIndex_Element));
	pmalloc->BSSIndex = p->BSSIndex;
	memcpy(pmalloc->BSSID, p->BSSID, MAC_LEN);
	pmalloc->next = BSSID_BSSIndex_table[index];
	BSSID_BSSIndex_table[index] = pmalloc;

	return 1;
}

/**
* Description:
*  Delete element from BSSID_BSSIndex table.
*
* Parameter:
*  p : BSS element.
*
* Return:
*  0 : Failed ; 1 : Successed.
*
*/
int wsm_bssid_bssidx_tbl_del(wAW_BSS *p)
{
	unsigned char index = 0;
	BSSID_BSSIndex_Element *p_pre_BSS = NULL;
	BSSID_BSSIndex_Element *p_next_BSS = NULL;

	index = wsm_tbl_mac_key(p->BSSID);
	p_pre_BSS = BSSID_BSSIndex_table[index];
	p_next_BSS = p_pre_BSS;
	
	while (p_next_BSS != NULL)
	{
		if (p_next_BSS->BSSIndex == p->BSSIndex)
		{
			/* Delete list header */
			if (p_next_BSS == p_pre_BSS)
			{
				BSSID_BSSIndex_table[index] = p_next_BSS->next;
				free(p_next_BSS);

				return 1;
			}
			else
			{
				p_pre_BSS->next = p_next_BSS->next;
				free(p_next_BSS);

				return 1;
			}
		}
		p_pre_BSS = p_next_BSS;
		p_next_BSS = p_next_BSS->next;
	}

	return 0;
}

/**
* Description:
*  Search table of WTPIP_WTPID or BSSID_BSSIndex to find element. 
*
* Parameter:
*  type: BSS type or WTP type.
*  pdata: element.
* 
* Return:
*   NULL: not found; other: element data.
*
*/
void* wsm_tbl_search(int type,  unsigned char *pdata,unsigned int wtpid)
{
	int index = -1;
	int pdata_len = 0;
	
	if (WTPIP_WTPID_TYPE == type)
	{
		WTPIP_WTPID_Element *p = NULL;
		struct mixwtpip *tmp = NULL;
		
		pdata_len = IPv6_LEN;
		if ((index = wsm_tbl_ip_key((struct mixwtpip*)pdata,wtpid)) < 0)
		{
			WSMLog(L_ERR, "%s: Get hash key error, key = %d\n.",
					__func__, index);
			return NULL;
		}
		WSMLog(L_DEBUG,"%s,index:%d",__func__,index);
		p = WTPIP_WTPID_table[index];
		tmp = (struct mixwtpip *)pdata;
		
		while( p != NULL )
		{
			WSMLog(L_DEBUG,"%s,p:%p,p->wtp_ip.addr_family:%d,p->wtp_ip.port:%d",__func__,p,p->wtp_ip.addr_family,p->wtp_ip.port);
			WSMLog(L_DEBUG,"%s,tmp:%p,tmp->addr_family:%d,tmp->port:%d",__func__,tmp,tmp->addr_family,tmp->port);
		
			if (p->wtp_ip.addr_family == tmp->addr_family /*&& p->wtp_ip.port == tmp->port*/)
			{
				if (p->wtp_ip.addr_family == AF_INET)
				{
					if (p->wtp_ip.m_v4addr == tmp->m_v4addr)
						return (void *)p;
				}
				else if (p->wtp_ip.addr_family == AF_INET6)
				{
					if (memcmp(p->wtp_ip.m_v6addr, tmp->m_v6addr, MIXIPLEN) == 0)
						return (void *)p;
				}
				else
				{
					WSMLog(L_ERR, "%s: unknown socket addr type[%d]\n", 
							__func__, p->wtp_ip.addr_family);
					return NULL;
				}
			}
			p = p->next;
		}
	}

	else if (BSSID_BSSIndex_TYPE == type)
	{
		pdata_len = MAC_LEN;
		index = wsm_tbl_mac_key(pdata);
		BSSID_BSSIndex_Element *p = BSSID_BSSIndex_table[index];

		while (p != NULL)
		{
			if (memcmp(p->BSSID, pdata, pdata_len) == 0)
			{
				return (void *)p;
			}
			p = p->next;
		}
	}
		
	WSMLog(L_DEBUG, "%s: Cannot find WTP info.\n", __func__);

	return NULL;
}


/**
* Description:
*  Genarate hash key from the last byte of mac.
* 
* Parameter:
*  pMAC: mac addr.
*
* Return:
*  Hash key
*
*/
unsigned char wsm_tbl_mac_key(unsigned char *pMAC)
{
	return pMAC[5];
}

/**
* Description:
*  Print WTP-BSS-STA tables. It's for debug, and now it's no use.
*
* Parameter:
*  type: WTP BSS or STA.
*
* Return:
*  void.
*
*/
void wsm_tbl_print(int type)
{
	int i;
	
	printf ("\nHashTable Print Start:\n\n");

	if (type == WTPIP_WTPID_TYPE)
	{
		WTPIP_WTPID_Element* temp;	
		printf ("|-- WTPIP_WTPID_table\n");

		for (i = 0; i < HASHTABLE_SIZE/4; i++)
		{
			if (WTPIP_WTPID_table[i] == NULL)
			{
				continue;
			}
			else
			{
				temp = WTPIP_WTPID_table[i];
				printf ("|   |--WTPIP_WTPID_table[%d]:\n", i);
							
				while (temp != NULL)
				{
					printf ("|      |\n");
					wsm_tbl_element_print(type, temp);
					temp = temp->next;
				}
			}
		}		
	}
	
	else if (type == BSSID_BSSIndex_TYPE)
	{
		BSSID_BSSIndex_Element *temp;
		printf ("|-- BSSIndex_BSSID_table\n");

		for (i = 0; i < HASHTABLE_SIZE/4; i++)
		{
			if (BSSID_BSSIndex_table[i] == NULL)
			{
				continue;
			}
			else
			{
				temp = BSSID_BSSIndex_table[i];
				printf ("|   |--BSSIndex_BSSID_table[%d]\n", i);

				while(temp != NULL)
				{
					printf ("|      |\n");
					wsm_tbl_element_print(type, temp);
					temp = temp->next;
				}
			}
		}	
	}

	else if (type == WTP_BSS_STA_TYPE)
	{
		WTP_Element *temp;
		BSS_Element *temp1;
		STA_Element *temp2;
			
		printf ("|-- WTPtable\n");

		for (i = 0; i < HASHTABLE_SIZE; i++)
		{
			if (WTPtable[i].flag == 0)
			{
				continue;
			}
			else
			{
				temp = &WTPtable[i];
				printf ("|   |--WTPtable[%d]\n", i);
				printf ("|      |\n");
				wsm_tbl_element_print(type, temp);
				printf ("|         |--BSS_Element\n");
					
				temp1 = temp->next;
				if(temp1 == NULL)
					continue;

				int i;
				for(i=0; i<BSS_ARRAY_SIZE; i++, temp1++)
				{
					if (temp1->flag == 0)
						continue;
					temp2 = temp1->next;
					printf ("|           |\n");
					wsm_tbl_element_print(WTP_BSS_PRINT_TYPE, temp1);
					printf ("|              |\n");

					while (temp2 != NULL)
					{
						printf ("|              |--STA\n");
						wsm_tbl_element_print(WTP_STA_PRINT_TYPE, &(temp2->STA));
						temp2 = temp2->next;
						printf ("|              |\n");
					}
				}			
			}
		}
	}
	
	else
	{
		printf("HashTable_Print: Unsupport type\n");
	}

	printf ("HashTable Print end.\n");
}

/**
* Description:
*  Print element of WTP-BSS-STA tables. It's for debug, and now it's no use.
*
* Parameter:
*  type: WTP BSS or STA.
*  pdata: element data.
*
* Return:
*  void.
*
*/
void wsm_tbl_element_print(int type, void *pdata)
{
	int i = 0;

	if (type == WTPIP_WTPID_TYPE)
	{
		WTPIP_WTPID_Element *temp = (WTPIP_WTPID_Element*)pdata;
		printf ("|      |--WTPID=0x%02x\n", temp->WTPID);
		printf ("|      |--WTPIP= ");
		#if 0
		for (i = 0; i < IPv4_LEN; i++)
		{
			printf("%d", *((unsigned char*)(&temp->WTPIP) + i));
			if (i != 3)
				printf(".");
		}
		#endif
		printf("\n|      |\n");
	}
	
	else if (type == BSSID_BSSIndex_TYPE)
	{
		BSSID_BSSIndex_Element *temp = (BSSID_BSSIndex_Element*)pdata;
		printf ("|      |--BSSIndex=0x%02x\n", temp->BSSIndex);
		printf ("|      |--BSSID= ");
		for (i = 0; i < MAC_LEN; i++)
		{
			printf("%02X", temp->BSSID[i]);
			if (i < MAC_LEN - 1)
				printf (":");
		}
		printf("\n|      |\n");
	}

	else if (type == WTP_BSS_STA_TYPE)
	{
		WTP_Element *temp = (WTP_Element*)pdata;
		wWSM_DataChannel *temp1 = &(temp->WTP);
			
		printf ("|      |--flag=0x%02x\n", temp->flag);
		printf ("|      |--countBSS=0x%02x\n", temp->count_BSS);
		printf ("|      |--WTPportNum=0x%02x\n", temp->WTPportNum);
		printf ("|      |--WTP\n");
		printf ("|         |--WTPID=0x%02x\n", temp1->WTPID);
		printf ("|         |--WTPMAC= ");
		for (i = 0; i < MAC_LEN; i++)
		{
			printf ("%02X", temp1->WTPMAC[i]);
			if (i < MAC_LEN - 1)
				printf (":");
			}
			printf ("\n");
			//printf ("|         |--WTPModel=0x%02x\n", temp1->WTPModel);
			printf ("|         |--WTPIP= ");
			for (i = 0; i < IPv4_LEN; i++)
			{
				printf ("%d", *((unsigned char*)(&temp1->WTPIP) + i));
				if (i != 3) printf (".");
			}
			printf ("\n");
			printf ("|         |--ACIfaceNum=0x%02x\n", temp1->ACIfaceNum);
		}

	else if (type == WTP_BSS_PRINT_TYPE)
	{
		BSS_Element *temp = (BSS_Element*)pdata;
			
		printf ("|           |--flag=0x%02x\n", temp->flag);
		printf ("|           |--BSS\n");
		printf ("|              |--BSSIndex=0x%02x\n", temp->BSS.BSSIndex);
		printf ("|              |--Radio_L_ID=0x%02x\n", temp->BSS.Radio_L_ID);
		printf ("|              |--Radio_G_ID=0x%02x\n", temp->BSS.Radio_G_ID);
		printf ("|              |--WlanID=0x%02x\n", temp->BSS.WlanID);
		printf ("|              |--wlan_if_type = %d\n", temp->BSS.wlan_ifaces_type);
		printf ("|              |--bss_if_type = %d\n", temp->BSS.bss_ifaces_type);
		printf ("|              |--BSSID= ");
		for (i = 0; i < MAC_LEN; i++)
		{
			printf ("%02X", temp->BSS.BSSID[i]);
			if (i < MAC_LEN - 1)
				printf(":");
		}
		printf ("\n");
	}

	else if (type == WTP_STA_PRINT_TYPE)
	{
		aWSM_STA *temp = (aWSM_STA*)pdata;
			
		printf ("|                 |--StaState=0x%02x\n", temp->StaState);
		printf ("|                 |--STAMAC= ");
		for (i = 0; i < MAC_LEN; i++)
		{
			printf ("%02X", temp->STAMAC[i]);
			if (i < MAC_LEN - 1)
				printf(":");
		}
		printf ("\n");
		printf ("|                 |--BSSIndex=0x%02x\n", temp->BSSIndex);
		printf ("|                 |--WTPID=0x%02x\n", temp->WTPID);
	}
	
	else
	{
		printf ("wsm_tbl_element_print: Unsupport type.Type=%d\n", type);
	}
}


