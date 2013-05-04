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
* wsm_wifi_bind.c
*
*
* DESCRIPTION:
*  In this file, there has some functions and structures about wifi bind L3IF.
*
* DATE:
*  2009-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2009-12-14 <guoxb> Add Inter-AC roaming table operation functions.
*  2009-12-30 <guoxb> Review source code.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "wsm_wifi_bind.h"
#include "wsm_types.h"


extern WTP_Element WTPtable[HASHTABLE_SIZE];
extern pthread_mutex_t sta_statistics_update_mutex;
/* Inter-AC roaming table */
extern roam_tbl_t *roam_tbl[HASHTABLE_SIZE / 4];
/* WID BSS packet statistics */
extern int iBSSPktsocket;
extern struct sockaddr_un WID_sockaddr;
extern unsigned int total_sta_cnt;

extern __inline__ void WSMLog(unsigned char type, char *format, ...);
extern int sta_statistics_to_asd(int cnt, unsigned char* buff);

struct WSM_BSSID_WLANID_TABLE *BSSID_WLANID_Table[WIFI_BIND_MAX_NUM];
struct WSM_STAMAC_BSSID_WTPIP_TABLE *STAMAC_BSSID_WTPIP_Table[WSM_STA_FDB_MAX_NUM];
int STAMAC_BSSID_WTPIP_NUM[WSM_STA_FDB_MAX_NUM];
struct WSM_WLANID_BSSID_TABLE WLANID_BSSID_Table[WIFI_BIND_MAX_NUM]; 
struct WSM_STA_Statistics *WSM_STA_Statistics_Table[WSM_STA_FDB_MAX_NUM];


/**  
* Description:
*  Initialize the tables about split mac
*
*/
void wifi_bind_init(void)
{
	unsigned int i = 0;

	for (i = 0; i < WIFI_BIND_MAX_NUM; i++)
	{
		BSSID_WLANID_Table[i] = NULL;
		memset(WLANID_BSSID_Table + i, 0, sizeof(struct WSM_WLANID_BSSID_TABLE));
	}
	
	for (i = 0; i < WSM_STA_FDB_MAX_NUM; i++)
	{
		STAMAC_BSSID_WTPIP_Table[i] = NULL;
		STAMAC_BSSID_WTPIP_NUM[i] = 0;
		WSM_STA_Statistics_Table[i] = NULL;
	}
}

/**
*  Description:
*   Get Hash key and index entry is the last 2 mac address fields in 6 fields
*
*  Parameter:
*  mac:Target mac address
*
* Return:
*  Hash key
*
*/
unsigned short wsm_mac_table_hash(unsigned char *mac)
{
	return *((unsigned short*)(mac + 4));
}

/**
* Description:
*  Get Hash key and index entry is the last mac address fields in 6 fields
* 
* Parameter:
* mac: Target mac address
*
* Return:
*  Hash key
*
*/
unsigned char wsm_mac_table_hash2(unsigned char *mac)
{
	return mac[5];
}


/**
* Description:
*  Tables Add Function, for frame tranform.
* 
* Parameter:
*  data: element
*  type: table type
*
* Return:
*  -1 : failed
*  0 : successed.
*
*/
int wsm_wifi_table_add(unsigned char *data, int type)
{
	unsigned short key = 0;
	
	if (type == WSM_BSSID_WLANID_TYPE)
	{
		struct wsm_bssid_wlanid *pdata = (struct wsm_bssid_wlanid *)data;
		struct WSM_BSSID_WLANID_TABLE *tmp = NULL, *pmalloc = NULL;
		
		key = wsm_mac_table_hash2(pdata->BSSID);
		tmp = BSSID_WLANID_Table[key];
		
		pmalloc = (struct WSM_BSSID_WLANID_TABLE*)malloc(
				sizeof(struct WSM_BSSID_WLANID_TABLE));	
		if (pmalloc == NULL)
		{
			WSMLog(L_CRIT, "%s: WSM_BSSID_WLANID_TYPE malloc failed.\n", __func__);
			return -1;
		}
			
		bzero(pmalloc, sizeof(struct WSM_BSSID_WLANID_TABLE));
		pmalloc->next = tmp;
		BSSID_WLANID_Table[key] = pmalloc;
		pmalloc->element.Radio_L_ID = pdata->Radio_L_ID;
		pmalloc->element.WlanID = pdata->WlanID;
		pmalloc->element.protect_type = pdata->protect_type;
		pmalloc->element.bss_if_type = pdata->bss_if_type;
		pmalloc->element.wlan_if_type = pdata->wlan_if_type;
		WSM_COPY_MAC(pmalloc->element.BSSID, pdata->BSSID);
			
		return 0;
	}

	else if (type == WSM_STAMAC_BSSID_WTPIP_TYPE)
	{
		struct wsm_stamac_bssid_wtpip *pdata = NULL;
		struct WSM_STAMAC_BSSID_WTPIP_TABLE *tmp = NULL, *pmalloc = NULL;
		roam_sta_ele_t roam_sta;
		struct wsm_stamac_bssid_wtpip *stamac_bssid_data = NULL;

		pdata = (struct wsm_stamac_bssid_wtpip *)data;
		key = wsm_mac_table_hash(pdata->STAMAC);
		if ((stamac_bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
								pdata->STAMAC, WSM_STAMAC_BSSID_WTPIP_TYPE)) != NULL)
		{
			
			stamac_bssid_data->STAState = pdata->STAState;
			stamac_bssid_data->WTPID = pdata->WTPID;
			stamac_bssid_data->ac_roam_tag = pdata->ac_roam_tag;
			memcpy(&stamac_bssid_data->ac_ip, &pdata->ac_ip, sizeof(struct mixwtpip));
			WSM_COPY_MAC(stamac_bssid_data->BSSID, pdata->BSSID);
		}else{
			pmalloc = (struct WSM_STAMAC_BSSID_WTPIP_TABLE *)malloc(
					sizeof(struct WSM_STAMAC_BSSID_WTPIP_TABLE));
			if (pmalloc == NULL)
			{
				WSMLog(L_CRIT, "%s: WSM_STAMAC_BSSID_WTPIP_TYPE malloc failed.\n", __func__);
				return -1;
			}

			bzero(pmalloc, sizeof(struct WSM_STAMAC_BSSID_WTPIP_TABLE));
			bzero(&roam_sta, sizeof(roam_sta_ele_t));
				
			tmp = STAMAC_BSSID_WTPIP_Table[key];
			pmalloc->next = tmp;
			STAMAC_BSSID_WTPIP_Table[key] = pmalloc;
			pmalloc->element.STAState = pdata->STAState;
			pmalloc->element.WTPID = pdata->WTPID;
			/* Inter-AC roaming data */
			pmalloc->element.ac_roam_tag = pdata->ac_roam_tag;
			memcpy(&pmalloc->element.ac_ip, &pdata->ac_ip, sizeof(struct mixwtpip));
			WSM_COPY_MAC(pmalloc->element.BSSID, pdata->BSSID);
			WSM_COPY_MAC(pmalloc->element.STAMAC, pdata->STAMAC);
			total_sta_cnt++;
			STAMAC_BSSID_WTPIP_NUM[key] += 1;
			//STAMAC_BSSID_WTPIP_Table[key] += 1;

			if (pdata->ac_roam_tag == RAW_AC)
			{
				memcpy(&roam_sta.ac_ip, &pdata->ac_ip, sizeof(struct mixwtpip));
				memcpy(roam_sta.bssid, pdata->BSSID, MAC_LEN);
				memcpy(roam_sta.stamac, pdata->STAMAC, MAC_LEN);
				roam_tbl_add(&roam_sta);
			}
		}
		return 0;
	}

	else if (type == WSM_WLANID_BSSID_TYPE)
	{
		struct wsm_wlanid_bssid_element *pdata = (struct wsm_wlanid_bssid_element*)data;
		struct wsm_wlanid_bssid *tmp = NULL, *pmalloc = NULL;

		pmalloc = (struct wsm_wlanid_bssid*)malloc(sizeof(struct wsm_wlanid_bssid));
		if (pmalloc == NULL)
		{
			WSMLog(L_CRIT, "%s: WSM_WLANID_BSSID_TYPE malloc error.\n", __func__);
			return -1;
		}
		
		bzero(pmalloc, sizeof(struct wsm_wlanid_bssid));
		tmp = WLANID_BSSID_Table[pdata->WlanID - 1].next;
		pmalloc->next = tmp;
		WLANID_BSSID_Table[pdata->WlanID - 1].next = pmalloc;
		pmalloc->BSSIndex = pdata->BSSIndex;
		pmalloc->protect_type = pdata->protect_type;
		pmalloc->bss_if_type = pdata->bss_if_type;
		pmalloc->wlan_if_type = pdata->wlan_if_type;
		WSM_COPY_MAC(pmalloc->BSSID, pdata->BSSID);
		WLANID_BSSID_Table[pdata->WlanID - 1].bsscount++;

		return 0;
	}

	else if (type == WSM_STA_Statistics_TYPE)
	{
		STAStatistics *pdata = (STAStatistics *)data;
		struct WSM_STA_Statistics *tmp = NULL, *pmalloc = NULL;

		key = wsm_mac_table_hash(pdata->STAMAC);
		
		pmalloc = (struct WSM_STA_Statistics *)malloc(
				sizeof(struct WSM_STA_Statistics));
		if (pmalloc == NULL)
		{
			WSMLog(L_CRIT, "%s: WSM_STA_Statistics_TYPE malloc error.\n", __func__);
			return -1;
		}
		
		bzero(pmalloc, sizeof(struct WSM_STA_Statistics));
		pmalloc->element.BSSIndex = pdata->BSSIndex;
		pmalloc->element.Radio_G_ID = pdata->Radio_G_ID;
		pmalloc->element.WTPID = pdata->WTPID;
		WSM_COPY_MAC(pmalloc->element.STAMAC, pdata->STAMAC);
		tmp = WSM_STA_Statistics_Table[key];
		WSM_STA_Statistics_Table[key] = pmalloc;
		pmalloc->next = tmp;
		return 0;
	}

	else 
	{
		WSMLog (L_ERR, "%s: unsupport type: %d\n", __func__, type);
		return -1;
	}
}

/**
* Description:
*  Delete WTP-BSS-STA relational tables.
*
* Parameter:
*  data: element data.
*  type: table type.
*
* Return:
*  0 : Successed; -1: failed.
*
*/
int wsm_wifi_table_del(unsigned char *data, int type)
{
	unsigned short key = 0;
	
	if (type == WSM_BSSID_WLANID_TYPE)
	{
		unsigned char *mac;
		struct WSM_BSSID_WLANID_TABLE *tmp, *pretmp;

		mac = data;
		key = wsm_mac_table_hash2(mac);
			
		if (BSSID_WLANID_Table[key] == NULL)
		{
			WSMLog(L_ERR, "%s: WSM_BSSID_WLANID_TYPE Empty table.\n", __func__);
			return -1;
		}
		else
		{
			tmp = BSSID_WLANID_Table[key];
			pretmp = tmp;

			while (tmp != NULL)
			{
				if (memcmp(tmp->element.BSSID, mac, MAC_LEN) == 0)
				{
					/* Delete the list head */
					if (pretmp == tmp)
					{
						BSSID_WLANID_Table[key] = tmp->next;
						free(tmp);
						return 0;
					}
					else
					{
						pretmp->next = tmp->next;
						free(tmp);
						return 0;
					}
				}
				
				pretmp = tmp;
				tmp = tmp->next;
			}

			WSMLog(L_ERR, "%s: WSM_BSSID_WLANID_TYPE cannot find element.\n", __func__);
			return -1;
		}
	}

	else if (type == WSM_STAMAC_BSSID_WTPIP_TYPE)
	{
		unsigned char *mac;
		struct WSM_STAMAC_BSSID_WTPIP_TABLE *tmp, *pretmp;
		roam_sta_ele_t roam_sta;
		int i = 0;	
		mac = data;
		key = wsm_mac_table_hash(mac);
		bzero(&roam_sta, sizeof(roam_sta_ele_t));
			
		if ((STAMAC_BSSID_WTPIP_NUM[key] == 0)||(STAMAC_BSSID_WTPIP_Table[key] == NULL))
		{
			WSMLog(L_ERR, "%s: WSM_STAMAC_BSSID_WTPIP_TYPE empty table.\n", __func__);
			return -1;
		}
		else
		{
			tmp = STAMAC_BSSID_WTPIP_Table[key];
			pretmp = tmp;

			//while (tmp != NULL)
			for(i = 0; i < STAMAC_BSSID_WTPIP_NUM[key]; i++)
			{
				if (memcmp(tmp->element.STAMAC, mac, MAC_LEN) == 0)
				{
					/* Delete the list head */
					if (pretmp == tmp)
					{
						STAMAC_BSSID_WTPIP_Table[key] = tmp->next;
						/* Delete STA from Inter-AC roaming table */
						if (tmp->element.ac_roam_tag == RAW_AC)
						{
							memcpy(&roam_sta.ac_ip, &tmp->element.ac_ip, sizeof(struct mixwtpip));
							memcpy(roam_sta.bssid, tmp->element.BSSID, MAC_LEN);
							memcpy(roam_sta.stamac, tmp->element.STAMAC, MAC_LEN);
							roam_tbl_del(&roam_sta);
						}
						free(tmp);
						STAMAC_BSSID_WTPIP_NUM[key] -= 1;
						if(STAMAC_BSSID_WTPIP_NUM[key] < 0)
							STAMAC_BSSID_WTPIP_NUM[key] = 0;
						total_sta_cnt--;
						return 0;
					}
					else
					{
						pretmp->next = tmp->next;
						/* Delete STA from Inter-AC roaming table */
						if (tmp->element.ac_roam_tag == RAW_AC)
						{
							memcpy(&roam_sta.ac_ip, &tmp->element.ac_ip, sizeof(struct mixwtpip));
							memcpy(roam_sta.bssid, tmp->element.BSSID, MAC_LEN);
							memcpy(roam_sta.stamac, tmp->element.STAMAC, MAC_LEN);
							roam_tbl_del(&roam_sta);
						}
						free(tmp);
						STAMAC_BSSID_WTPIP_NUM[key] -= 1;
						if(STAMAC_BSSID_WTPIP_NUM[key] < 0)
							STAMAC_BSSID_WTPIP_NUM[key] = 0;
						total_sta_cnt--;
						return 0;
					}
				}
				pretmp = tmp;
				tmp = tmp->next;
			}
					
			WSMLog(L_ERR, "%s: WSM_STAMAC_BSSID_WTPIP_TYPE cannot find element.\n", __func__);
			return -1;
		}	
	}

	else if (type == WSM_WLANID_BSSID_TYPE)
	{	
		struct wsm_wlanid_bssid_element *pdata = (struct wsm_wlanid_bssid_element*)data;
		struct WSM_WLANID_BSSID_TABLE *element = NULL;
		struct wsm_wlanid_bssid *tmp = NULL;
		struct wsm_wlanid_bssid *pretmp = NULL;

		element = WLANID_BSSID_Table + (pdata->WlanID -1);
		if (element->bsscount == 0)
		{
			WSMLog(L_ERR, "%s: Wlan[%d] not has BSS.\n", __func__, pdata->WlanID);
			return -1;
		}

		if (element->bsscount == 1)
		{
			if (memcmp(pdata->BSSID, element->next->BSSID, MAC_LEN) == 0)
			{
				free(element->next);
				element->next = NULL;
				element->bsscount = 0;
				return 0;
			}
			else
			{
				WSMLog(L_ERR, "%s: Cannot find BSS, Delete failed.\n", __func__);
				return -1;
			}
		}
		else
		{
			tmp = element->next;
			pretmp = tmp;
		
			while(tmp != NULL)
			{
				if (memcmp(tmp->BSSID, pdata->BSSID, MAC_LEN) == 0)
				{
					if (pretmp == tmp)
					{
						element->next = tmp->next;
						free(tmp);
						element->bsscount--;
						return 0;
					}
					else
					{
						pretmp->next = tmp->next;
						free(tmp);
						element->bsscount--;
						return 0;
					}
				}
				
				pretmp = tmp;
				tmp = tmp->next;
			}
			
			WSMLog(L_ERR, "%s: Cannot find BSS, Delete error\n", __func__);	
			return -1;
		}
	}

	else if (type == WSM_STA_Statistics_TYPE)
	{
		struct WSM_STA_Statistics *tmp = NULL, *pretmp = NULL;
		unsigned char *mac = data;

		key = wsm_mac_table_hash(mac);
		if (WSM_STA_Statistics_Table[key] == NULL)
		{
			WSMLog(L_ERR, "%s: Delete empty WSM_STA_Statistics_Table\n", __func__);
			return -1;
		}
		tmp = pretmp = WSM_STA_Statistics_Table[key];

		while(tmp != NULL)
		{
			if (memcmp(tmp->element.STAMAC, mac, MAC_LEN) == 0)
			{
				if (tmp == pretmp)
				{
					WSM_STA_Statistics_Table[key] = tmp->next;
					free(tmp);
									
					return 0;
				}
				else
				{
					pretmp->next = tmp->next;
					free(tmp);
									
					return 0;
				}
			}
			
			pretmp = tmp;
			tmp = tmp->next;
		}

		WSMLog(L_ERR, "%s: WSM_STA_Statistics_Table, Delete error\n", __func__);
		return -1;
	}

	else
	{
		WSMLog(L_ERR, "%s: Unsupport type[%d].\n", __func__, type);
		return -1;
	}
}

/**
* Description:
*  Find element in tables
*
* Parameter:
*  data: element data.
*  type: table types.
* 
* Return:
*  data buff
*
*/ 
void * wsm_wifi_table_search(unsigned char *data, int type)
{
	unsigned short key;

	if (type == WSM_BSSID_WLANID_TYPE)
	{
		unsigned char *mac = data;
		struct WSM_BSSID_WLANID_TABLE *tmp;
			
		key = wsm_mac_table_hash2(mac);
		if (BSSID_WLANID_Table[key] == NULL)
		{
			return NULL;
		}
		else
		{
			tmp = BSSID_WLANID_Table[key];
					
			while(tmp != NULL)
			{
				if(memcmp(tmp->element.BSSID, mac, MAC_LEN) == 0)
				{
					return (void*)(&tmp->element);
				}
				tmp = tmp->next;
			}

			return NULL;
		}
	}

	else if (type == WSM_STAMAC_BSSID_WTPIP_TYPE)
	{
		unsigned char *mac = data;
		struct WSM_STAMAC_BSSID_WTPIP_TABLE *tmp;
		int i = 0;

		key = wsm_mac_table_hash(mac);

		if ((STAMAC_BSSID_WTPIP_NUM[key] == 0)||(STAMAC_BSSID_WTPIP_Table[key] == NULL))
		{
			return NULL;
		}
		else
		{
			tmp = STAMAC_BSSID_WTPIP_Table[key];

			//while(tmp != NULL)
			for(i = 0; i < STAMAC_BSSID_WTPIP_NUM[key]; i++)
			{
				if (memcmp(tmp->element.STAMAC, mac, MAC_LEN) == 0)
				{
					return (void*)(&tmp->element);
				}
				tmp = tmp->next;
			}

			return NULL;
		}
	}

	else if (type == WSM_STA_Statistics_TYPE)
	{
		unsigned char *mac = data;
		struct WSM_STA_Statistics *tmp = NULL;

		key = wsm_mac_table_hash(mac);

		if (WSM_STA_Statistics_Table[key] == NULL)
		{
			return NULL;
		}
		else
		{
			tmp = WSM_STA_Statistics_Table[key];

			while (tmp != NULL)
			{
				if (memcmp(tmp->element.STAMAC, mac, MAC_LEN) == 0)
				{
					return (void *)(&tmp->element);
				}
				tmp = tmp->next;
			}
					
			return NULL;
		}
	}

	else
	{
		WSMLog(L_ERR, "%s: Unknown type[%d].\n", __func__, type);
		return NULL;
	}
}

/**
* Description:
*  Print tables for debug. And now it's no use.
* 
* Parameter
*  type: tables type 
*
* return:
*  0: Success, -1: failed, unknown type.
*
*/
int wsm_wifi_table_print(int type)
{
	int i = 0;
	printf ("WSM_Wifi_Table_Print Start:\n");
	
	if (type == WSM_BSSID_WLANID_TYPE)
	{
		struct WSM_BSSID_WLANID_TABLE *pnext;

		printf ("\n");
		printf ("|-----BSSID_WLANID_Table:\n");
		printf ("|\n");

		for (i = 0; i < WIFI_BIND_MAX_NUM; i++)
		{
			if (BSSID_WLANID_Table[i] == NULL)
				continue;

			pnext = BSSID_WLANID_Table[i];
					
			while(pnext != NULL)
			{
				printf ("|----\\\n");
				printf ("|    |----------------------------------\n");
				printf ("|    |--BSSID_WLANID_Table[%d]: \n", i);
				printf ("|    |--BSSID = ");
				WSM_PRINT_MAC(pnext->element.BSSID);
				printf ("|    |--WlanID = %d\n", pnext->element.WlanID);
				printf ("|    |--wlan_if_type = %d\n", pnext->element.wlan_if_type);
				printf ("|    |--bss_if_type = %d\n", pnext->element.bss_if_type);
				printf ("|    |--Radio_L_ID = %d\n", pnext->element.Radio_L_ID);
				printf ("|    `----------------------------------\n");
				printf ("|\n");
				pnext = pnext->next;
			}		
		}
		return 0;
	}

	else if (type == WSM_STAMAC_BSSID_WTPIP_TYPE)
	{
		struct WSM_STAMAC_BSSID_WTPIP_TABLE *pnext;

		printf ("\n");
		printf ("|-----STAMAC_BSSID_WTPIP_Table:\n");
		printf ("|\n");
			
		for (i = 0; i < WSM_STA_FDB_MAX_NUM; i++)
		{
			if (STAMAC_BSSID_WTPIP_Table[i] == NULL)
				continue;

			pnext = STAMAC_BSSID_WTPIP_Table[i];
					
			while(pnext != NULL)
			{	
				printf ("|----\\\n");
				printf ("|    |----------------------------------\n");
				printf ("|    |--STAMAC_BSSID_WTPIP_Table[%d]: \n", i);
				printf ("|    |--STAMAC = ");
				WSM_PRINT_MAC(pnext->element.STAMAC);
				printf ("|    |--BSSID = ");
				WSM_PRINT_MAC(pnext->element.BSSID);
				printf ("|    |--STAState = %d\n", pnext->element.STAState);
				printf ("|    |--WTPIP = \n");
				/* WSM_PRINT_IP(*((unsigned int*)pnext->element.WTPIP.addr)); */
				printf ("|    `----------------------------------\n");
				printf ("|\n");
							
				pnext = pnext->next;
			}		
		}
		return 0;
	}

	else if (type == WSM_WLANID_BSSID_TYPE)
	{
		struct wsm_wlanid_bssid *element;

		printf ("\n");
		printf ("|-----WLANID_BSSID_Table:\n");
		printf ("|----\\\n");

		for (i = 0; i < WIFI_BIND_MAX_NUM; i++)
		{
			if (WLANID_BSSID_Table[i].bsscount == 0)
				continue;
			element = (WLANID_BSSID_Table + i)->next;

			printf ("|    |-------------------------------------\n");
			printf ("|    |--WlanID = %d\n", i + 1);
			printf ("|    |-- BSSCount = %d Roaming_policy = %d\n", (WLANID_BSSID_Table + i)->bsscount, 
				(WLANID_BSSID_Table + i)->Roaming_policy);
			printf ("|    |-------------------------------------\n");
			printf ("|    |\n");
			while(element != NULL)
			{
				printf ("|    |----BSSID=");
				WSM_PRINT_MAC(element->BSSID);
				printf ("|    |----BSSIndex=%d\n", element->BSSIndex);
				printf ("|    |----wlan_if_type = %d\n", element->wlan_if_type);
				printf ("|    |----bss_if_type = %d\n", element->bss_if_type);
				element = element->next;
				printf ("|    |\n");			
			}
			printf ("|    |-------------------------------------\n");
		}
		return 0;
	}

	else if (type == WSM_STA_Statistics_TYPE)
	{
		struct WSM_STA_Statistics *tmp;

		for (i = 0; i < WSM_STA_FDB_MAX_NUM; i++)
		{
			if ((tmp = WSM_STA_Statistics_Table[i]) != NULL)
			{
				while(tmp != NULL)
				{
					printf ("+----------------------------------\n");
					printf ("----BSSIndex = %d\n", tmp->element.BSSIndex);
					printf ("----WTPID = %d\n", tmp->element.WTPID);
					printf ("----Radio_G_ID %d\n", tmp->element.Radio_G_ID);
					printf ("----STAMAC = ");
					WSM_PRINT_MAC(tmp->element.STAMAC);
					//printf ("----rx = %llu, tx = %llu\n", tmp->element.rx, tmp->element.tx); */
					printf ("+----------------------------------\n");
					tmp = tmp->next;
				}
				tmp = NULL;
			}
		}
		return 0;
	}
	else 
	{
		printf("%s: unsupported type.\n", __func__);
		return -1;
	}
}

/**
* Descriptoin:
*   Print MAC addr in a human-readable format.
*
* Parameter:
*  mac:mac address
*
* Return:
*  void.
*
*/
__inline__ void WSM_PRINT_MAC(unsigned char *mac)
{
	int tmp;

	for (tmp = 0; tmp < MAC_LEN; tmp++)
		{
			printf ("%02x", mac[tmp]);

			if (tmp < MAC_LEN -1) 
				printf (":");
		}
	printf ("\n");
	
}

/**
* Description:
*  Print IP addr in a human-readable format.
*
* Parameter:
*  ip : ip addr.
*
* Return:
*  void.
*
*/
__inline__ void WSM_PRINT_IP(unsigned int ip)
{
	int tmp;
	unsigned char *p = (unsigned char*)&ip;
	
	for (tmp = 0; tmp < 4; tmp++)
		{
			printf ("%d", p[tmp]);

			if (tmp < 3)
				printf (".");
		}
	
	printf ("\n");
}

/**
* Description:
*   Updata STA tx/rx packets statistics table
*
* Parameter:
*  mac: Target STA mac
*  size: size of packet(bytes)
*  type: rx or tx
*
* Return:
*  void.
*
*/
void wsm_sta_statistics_update(unsigned char *mac, int size, int type)
{
	STAStatistics *data = NULL;

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);

	data = (STAStatistics *)wsm_wifi_table_search(mac, WSM_STA_Statistics_TYPE);
	if (data == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_ERR, "%s: STA statistics update failed, cannot find sta\n", __func__);
		return;
	}
	
	if (type == WSM_STA_RX_UNI_TYPE)
	{
		/* Mutex for multi-thread update statistics table */
		pthread_mutex_lock(&sta_statistics_update_mutex);
		data->rx_pkt_unicast += size;
		++(data->rx_unicast);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;
	}
	
	else if (type == WSM_STA_TX_UNI_TYPE)
	{
		pthread_mutex_lock(&sta_statistics_update_mutex);
		data->tx_pkt_unicast += size;
		++(data->tx_unicast);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;
	}
	
	else if (type == WSM_STA_RETRY_TYPE)
	{
		pthread_mutex_lock(&sta_statistics_update_mutex);
		data->retry += size;
		++(data->retry_pkt);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;	
	}
	
	else if (type == WSM_STA_ERR_TYPE)
	{
		pthread_mutex_lock(&sta_statistics_update_mutex);
		++(data->err);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;	
	}
	
	else if (type == WSM_STA_TX_BRD_TYPE)
	{
		pthread_mutex_lock(&sta_statistics_update_mutex);
		data->tx_pkt_broadcast += size;
		++(data->tx_broadcast);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;
	}
	
	else if (type == WSM_STA_RX_BRD_TYPE)
	{
		pthread_mutex_lock(&sta_statistics_update_mutex);
		data->rx_pkt_broadcast += size;
		++(data->rx_broadcast);
		pthread_mutex_unlock(&sta_statistics_update_mutex);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;
	}
	
	else
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return;
	}
}

/**
* Description:
*  Update STA broadcast packet statistics for per STA in the BSS.
*
* Parameter:
*  pdata: STA pointer in a BSS
*  len: packet len
*
* Return: 
*  void
*
*/
__inline__ void wsm_bss_brd_statistics_update(STA_Element *pdata, unsigned int len)
{
	if (pdata == NULL)
		return;

	while (pdata != NULL)
	{
		wsm_sta_statistics_update(pdata->STA.STAMAC, len, WSM_STA_RX_BRD_TYPE);
		pdata = pdata->next;
	}
}

/**
* Description:
*  Get STA Statistics and send it to ASD
* 
* Parameter:
*  void.
*
* Return:
*  void.
*
*/
void wsm_get_STAStatistics(void)
{
	int i = 0;
	int maxcnt = 35, cnt = 0; 
	int offset = sizeof(STAStatisticsMsg);
	unsigned char buff[4096] = {0}, *pbuff = buff + offset;
	struct WSM_STA_Statistics *tmp;

	for (i = 0; i < WSM_STA_FDB_MAX_NUM; i++)
	{
		if ((tmp = WSM_STA_Statistics_Table[i]) != NULL)
		{
			while(tmp != NULL)
			{
				memcpy(pbuff, (unsigned char*)&tmp->element, sizeof(STAStatistics));
				pbuff += sizeof(STAStatistics);
				++cnt;
					
				if (cnt == maxcnt)
				{
					if (sta_statistics_to_asd(cnt, buff))
					{
						WSMLog(L_ERR, "%s: sta_statistics_to_asd failed\n", __func__);
					}
					cnt = 0;
					bzero(buff, 4096);
					pbuff = buff + offset;
				}
				tmp = tmp->next;
			}
		}
	}

	if (cnt != 0)
	{
		if (sta_statistics_to_asd(cnt, buff))
		{
			WSMLog(L_ERR, "%s: sta_statistics_to_asd failed\n", __func__);
		}
		return;
	}
	else
	{
		return;
	}
}


/**
* Description:
*  Get BSS packets Statistics info for WID, based on BSS.
*
* Parameter:
*  WTPID: Get this WTP's packets statistics.
*
* Return:
*  void
*
*/
int wsm_get_BSSStatistics(unsigned int WTPID)
{
	unsigned char buff[8192] = {0};
	BSS_Element *BSS = NULL;
	unsigned char *pdata = buff + sizeof(MsgType) + sizeof(BSS_pkt_header);
	MsgType *type = (MsgType*)buff;
	unsigned int cnt = 0;
	
	if ((WTPID == 0) || (WTPID > HASHTABLE_SIZE))
	{
		WSMLog(L_ERR, "%s: WTP[%u],  WTPID out of range.\n", __func__, WTPID);
		return -1;
	}

	if ((WTPtable[WTPID].flag == 0) || (WTPtable[WTPID].next == NULL))
	{
		WSMLog(L_ERR, "%s: WTP[%d] is unused.\n", __func__, WTPID);
		return -1;
	}

	BSS = WTPtable[WTPID].next;
	*type = BSS_PKT_TYPE;
	
	/* Unknown error */
	if ((cnt = wsm_get_bss_per_wtp(pdata, BSS)) < 0)
		cnt = 0;
	wsm_bss_info_sendto_wid(buff, WTPID, cnt);

	return 1;
}

/**
* Description:
*  Get BSS entry from a WTP for BSS packet statistics.
*
* Parameter:
*  pdata: buffer that would send to WID
*  BSS: WTP BSS array pointer
*
* Return:
*  BSS count
*
*/
__inline__ int wsm_get_bss_per_wtp(unsigned char *pdata, BSS_Element *BSS)
{
	BSSStatistics *BSS_Statistics = NULL;
	BSS_Element *tmpBSS = NULL;
	unsigned int i = 0;
	unsigned int tmp = 0;

	if ((pdata == NULL) || (BSS == NULL))
	{
		WSMLog(L_ERR, "%s: Parameter error.\n", __func__);
		return -1;
	}
	
	BSS_Statistics = (BSSStatistics *)pdata;
	tmpBSS = BSS;

	while(tmp < BSS_ARRAY_SIZE)
	{		
		if (tmpBSS[tmp].flag == 0)
		{
			tmp++;
			continue;
		}

		BSS_Statistics[i].Radio_G_ID = tmpBSS[tmp].BSS.Radio_G_ID;
		BSS_Statistics[i].BSSIndex = tmpBSS[tmp].BSS.BSSIndex;
		wsm_get_bss_statistics(&tmpBSS[tmp], &BSS_Statistics[i]);
		tmp++;
		i++;
	}

	return i;
}

/**
* Description:
*  Get every sta info from a BSS
*
* Parameter:
*  BSS: BSS entry
*  data: buffer that would send to WID
*
* Return:
*  0: successed; -1 failed 
*
*/
__inline__ int wsm_get_bss_statistics(BSS_Element *BSS, BSSStatistics *data)
{
	STA_Element *STA = NULL;
	STAStatistics *pSTA_info = NULL;
	
	if (BSS == NULL || data == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter error.\n", __func__);
		return -1;
	}

	/* Not has STA */
	if (BSS->flag == 0 || BSS->next == NULL)
	{
		wsm_fill_in_pkt_statistics(data, NULL);
		return 0;
	}
	STA = BSS->next;

	while(STA != NULL)
	{
		pSTA_info = wsm_get_sta_statistics(STA->STA.STAMAC);
		if (pSTA_info != NULL)
			wsm_fill_in_pkt_statistics(data, pSTA_info);
			
		STA = STA->next;
	}
	
	return 0;	
}

/**
* Description:
*  Get STA pkt statistics for BSS Statistics.
*
* Parameter:
*  mac: STA MAC
*
* Return:
*  STA packet statistics
*
*/
__inline__ STAStatistics *wsm_get_sta_statistics(unsigned char *mac)
{
	STAStatistics *pdata = NULL;

	if (mac == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter error.\n", __func__);
		return NULL;
	}
	
	if ((pdata = (STAStatistics *)wsm_wifi_table_search(mac, WSM_STA_Statistics_TYPE)) == NULL)
	{
		WSMLog(L_ERR, "%s: Cannot find STA.\n", __func__);
		return NULL;
	}

	return pdata;
}

/**
* Description:
*  Send BSS packet Statistics infomation to WID.
*
* Parameter:
*  buff: buffer that would send to WID
*  WTPID: WTP ID
*  cnt: BSS count
*
* Return:
*  0:successed; -1: failed
*
*/
__inline__ int wsm_bss_info_sendto_wid(unsigned char *buff, unsigned int WTPID,
										unsigned int cnt)
{
	BSS_pkt_header *BSS_header = NULL;
	unsigned int len = 0;
	unsigned int sock_len = 0;
	
	if (buff == NULL || WTPID == 0 || WTPID > HASHTABLE_SIZE)
	{
		WSMLog(L_ERR, "%s: Parameter error.\n", __func__);
		return -1;
	}
	
	BSS_header = (BSS_pkt_header *)(buff + sizeof(MsgType));
	BSS_header->bss_cnt = cnt;
	BSS_header->WTPID = WTPID;
	len = sizeof(MsgType) + sizeof(BSS_pkt_header) + cnt * sizeof(BSSStatistics);
	sock_len = strlen(WID_sockaddr.sun_path) + sizeof(WID_sockaddr.sun_family);

	if (sendto(iBSSPktsocket, buff, len, 0, (struct sockaddr *)&WID_sockaddr, sock_len) < 0)
	{
		WSMLog(L_INFO, "%s: sendto failed.\n", __func__);
		return -1;
	}

	return 0;
}

/**
* Description:
*  Fill in STA statistics structure
*
* Parameter:
*  dest: destitaton 
*  src: if src == NULL, dest would be fill in zero
*
* Return:
*  void
*
*/
void wsm_fill_in_pkt_statistics(BSSStatistics *dest, STAStatistics *src)
{
	if (dest == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter error.\n", __func__);
		return;
	}

	/*
	 When STA recv a packet, BSS send a packet. So STA and BSS tx/rx has
	 some difference.
	 And when BSS send a broadcast packet, it would send to all of STAs, So
	 there just need statistics one STA's broadcast packet.
	*/
	if (src != NULL)
	{
		dest->rx_unicast += src->tx_unicast;
		dest->tx_unicast += src->rx_unicast;
		dest->rx_broadcast += src->tx_broadcast;
		if (dest->tx_broadcast < src->rx_broadcast) 
			dest->tx_broadcast = src->rx_broadcast;
		dest->rx_pkt_unicast += src->tx_pkt_unicast;
		dest->tx_pkt_unicast += src->rx_pkt_unicast;
		dest->rx_pkt_broadcast += src->tx_pkt_broadcast;
		if (dest->tx_pkt_broadcast < src->rx_pkt_broadcast)
			dest->tx_pkt_broadcast = src->rx_pkt_broadcast;
		dest->retry += src->retry;
		dest->retry_pkt += src->retry_pkt;
		dest->err += src->err;

		return;
	}
	else
	{
		dest->rx_unicast = 0;
		dest->tx_unicast = 0;
		dest->rx_broadcast = 0;
		dest->tx_broadcast = 0;
		dest->rx_pkt_unicast = 0;
		dest->tx_pkt_unicast = 0;
		dest->rx_pkt_broadcast = 0;
		dest->tx_pkt_broadcast = 0;
		dest->retry = 0;
		dest->retry_pkt = 0;
		dest->err = 0;

		return;
	}
}

/**
* Description:
*  Inter-AC roaming table add function.
*
* Parameter:
*  sta_info: roaming sta information.
*
* Return:
*  -1: Failed.
*   0: Successed.
*/
int roam_tbl_add(roam_sta_ele_t *sta_info)
{
	unsigned int key = 0;
	roam_tbl_t *pBSS = NULL;
	roam_sta_t *pSTA = NULL;
	roam_tbl_t *ptbl = NULL;
	
	if (sta_info == NULL)
	{
		WSMLog(L_ERR, "%s: sta_info == NULL, parameter error.\n", __func__);
		return -1;
	}
	
	key = wsm_mac_table_hash2(sta_info->bssid);
	ptbl = roam_tbl_search(sta_info->bssid);
	
	if (roam_tbl[key] == NULL || ptbl == NULL)
	{
		pBSS = (roam_tbl_t*)malloc(sizeof(roam_tbl_t));
		pSTA = (roam_sta_t*)malloc(sizeof(roam_sta_t));

		if (pBSS == NULL || pSTA == NULL)
		{
			WSMLog(L_CRIT, "%s: malloc pBSS or pSTA failed.\n", __func__);
			return -1;
		}
		
		memcpy(&pSTA->ac_ip, &sta_info->ac_ip, sizeof(struct mixwtpip));
		memcpy(pSTA->stamac, sta_info->stamac, MAC_LEN);
		pSTA->next = NULL;

		memcpy(pBSS->bssid, sta_info->bssid, MAC_LEN);
		pBSS->sta = pSTA;
		pBSS->next = roam_tbl[key];
		roam_tbl[key] = pBSS;

		return 0;
	}

	pSTA = (roam_sta_t *)malloc(sizeof(roam_sta_t));
	if (pSTA == NULL)
	{
		WSMLog(L_CRIT, "%s: malloc pSTA failed.\n", __func__);
		return -1;
	}

	memcpy(&pSTA->ac_ip, &sta_info->ac_ip, sizeof(struct mixwtpip));
	memcpy(pSTA->stamac, sta_info->stamac, MAC_LEN);
	pSTA->next = ptbl->sta;
	ptbl->sta = pSTA;

	return 0;
}

/**
*  Description:
*   Inter-AC roaming table delete function.
* 
*  Parameter:
*   sta_info: roaming sta information.
*
*  Return:
*   -1: Failed
*   0: Successed.
*
*/
int roam_tbl_del(roam_sta_ele_t *sta_info)
{
	roam_tbl_t *ptbl = NULL;
	roam_sta_t *pSTA = NULL, *preSTA = NULL;

	if (sta_info == NULL)
	{
		WSMLog(L_ERR, "%s: sta_info == NULL, parameter error.\n", __func__);
		return -1;
	}

	ptbl = roam_tbl_search(sta_info->bssid);
	if (ptbl == NULL)
	{
		WSMLog(L_ERR, "%s: BSSID[%02x:%02x:%02x:%02x:%02x:%02x]"
					" not have roaming sta[%02x:%02x:%02x:%02x:%02x:%02x].\n", 
				__func__, sta_info->bssid[0], sta_info->bssid[1], sta_info->bssid[2], 
				sta_info->bssid[3], sta_info->bssid[4], sta_info->bssid[5],
				sta_info->stamac[0], sta_info->stamac[1], sta_info->stamac[2], 
				sta_info->stamac[3], sta_info->stamac[4], sta_info->stamac[5] );
		return -1;
	}
	preSTA = ptbl->sta;
	pSTA = preSTA;
	
	while (pSTA != NULL)
	{
		if (memcmp(pSTA->stamac, sta_info->stamac, MAC_LEN) == 0)
		{
			/* Delete the first element */
			if (pSTA == preSTA)
			{
				ptbl->sta = pSTA->next;
				free(pSTA);
				pSTA = preSTA = NULL;
				
				if (ptbl->sta == NULL)
				{
					roam_tbl_del_bss(sta_info->bssid);
				}
			
				return 0;
			}
			else
			{
				preSTA->next = pSTA->next;
				free(pSTA);
				pSTA = preSTA = NULL;
				
				return 0;
			}
		}
		preSTA= pSTA;
		pSTA = pSTA->next;
	}

	return -1;
}

/**
*  Description:
*   Inter-AC roaming table search function.
* 
*  Parameter:
*   bssid: BSSID.
*
*  Return:
*   NULL: Search failed.
*   other: roaming-sta information.
*/
roam_tbl_t *roam_tbl_search(unsigned char *bssid)
{
	unsigned int key = 0;
	roam_tbl_t *ptbl = NULL;
	
	if (bssid == NULL)
	{
		WSMLog(L_ERR, "%s: bssid == NULL, parameter error.\n", __func__);
		return NULL;
	}

	key = wsm_mac_table_hash2(bssid);
	ptbl = roam_tbl[key];

	while (ptbl != NULL)
	{
		if (memcmp(ptbl->bssid, bssid, MAC_LEN) == 0)
		{
			return ptbl;
		}
		ptbl = ptbl->next;
	}

	return NULL;
}

/**
* Description:
*  Deleter BSS information from inter-ac roaming table.
*
* Parameter:
*  bssid: BSSID
*
* Return:
*  void.
*
*/
void roam_tbl_del_bss(unsigned char *bssid)
{
	unsigned int key = 0;
	roam_tbl_t *ptbl = NULL, *pretbl = NULL;

	if (bssid == NULL)
	{
		WSMLog(L_ERR, "%s: bssid == NULL, parameter error.\n", __func__);
	}

	key = wsm_mac_table_hash2(bssid);
	ptbl = roam_tbl[key];
	pretbl =ptbl;

	while (ptbl != NULL)
	{
		if (memcmp(ptbl->bssid, bssid, MAC_LEN) == 0)
		{
			if (pretbl == ptbl)
			{
				roam_tbl[key] = ptbl->next;
				free(ptbl);
				ptbl = pretbl = NULL;
				return;
			}
			else
			{
				pretbl->next = ptbl->next;
				free(ptbl);
				pretbl = ptbl = NULL;
				return;
			}
		}
		pretbl = ptbl;
		ptbl = ptbl->next;
	}

	WSMLog(L_ERR, "%s: Cannot find BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n", \
		__func__, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
}


