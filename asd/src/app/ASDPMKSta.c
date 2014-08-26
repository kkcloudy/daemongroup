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
* AsdPmkSta.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/
#include <time.h>
#include <sys/time.h>
#include "includes.h"

#include "asd.h"
#include "ASDStaInfo.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef.h"
#include "common.h"
#include "include/auth.h"
#include "ap.h"
#include "config.h"
#include "common.h"
#include "circle.h"
#include "sha1.h"
#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "ASDEapolSM.h"
#include "ASDWPAOp.h"
#include "ASDPMKCache.h"
#include "ASDWPAAuthIE.h"
#include "ASDWPAAuthI.h"

#include "ASDPMKSta.h"



struct PMK_STAINFO * pmk_ap_get_sta(WID_WLAN *WLAN, const u8 *sta)
{
	struct PMK_STAINFO *s;

	s = WLAN->sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


static void pmk_ap_sta_list_del(WID_WLAN *WLAN, struct PMK_STAINFO *sta)
{
	struct PMK_STAINFO *tmp;

	if (WLAN->sta_list == sta) {
		WLAN->sta_list = sta->next;
		return;
	}

	tmp = WLAN->sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
}


void pmk_ap_sta_hash_add(WID_WLAN *WLAN, struct PMK_STAINFO *sta)
{
	sta->hnext = WLAN->sta_hash[STA_HASH(sta->addr)];
	WLAN->sta_hash[STA_HASH(sta->addr)] = sta;
}


static void pmk_ap_sta_hash_del(WID_WLAN *WLAN, struct PMK_STAINFO *sta)
{
	struct PMK_STAINFO *s;

	s = WLAN->sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		WLAN->sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR, "AP: could not remove STA " MACSTR
			   " from hash table", MAC2STR(sta->addr));
}


void pmk_ap_free_sta(WID_WLAN *WLAN, struct PMK_STAINFO *sta)
{
	//int set_beacon = 0;
	struct PMK_BSSInfo *bss;
	struct PMK_BSSInfo *bss1;
	//int i=0;
		
	pmk_ap_sta_hash_del(WLAN, sta);
	pmk_ap_sta_list_del(WLAN, sta);
	bss = sta->bss;
	while(bss != NULL){
		bss1 = bss;
		bss = bss->next;
		free(bss1);
	}
	sta->bss = NULL;
	sta->BssNum = 0; 
	WLAN->num_sta--;
	os_free(sta->BSSIndex);
	os_free(sta);

}


struct PMK_STAINFO * pmk_ap_sta_add(WID_WLAN *WLAN, const u8 *addr)
{
	struct PMK_STAINFO *sta;
	//char* nowtime;
	//time_t now;//zhanglei add
	sta = pmk_ap_get_sta(WLAN, addr);
	if (sta)
		return sta;

	sta = os_zalloc(sizeof(struct PMK_STAINFO));
	if (sta == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "sta malloc failed");
		return NULL;
	}	
	os_memcpy(sta->addr, addr, ETH_ALEN);
	sta->next = WLAN->sta_list;
	WLAN->sta_list = sta;
	WLAN->num_sta++;
	sta->BSSIndex = (unsigned char*)malloc(BSS_NUM);
	if(sta->BSSIndex == NULL){
		asd_printf(ASD_DEFAULT,MSG_ERROR, "sta->BSSIndex malloc failed");
		return NULL;
	}
	pmk_ap_sta_hash_add(WLAN, sta);
	memset(sta->BSSIndex, 0, BSS_NUM);
	sta->bss = NULL;
	sta->BssNum = 0;
	return sta;
	
}

int pmk_add_bssindex(unsigned int BSSIndex, struct PMK_STAINFO *sta){
	struct PMK_BSSInfo *bss;
	if((sta == NULL))
		return 0;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA "MACSTR" PreBssIndex is %d, to add BssIndex %d\n",MAC2STR(sta->addr),sta->PreBssIndex,BSSIndex);
	if(sta->BSSIndex[BSSIndex] == 1){
		if (sta->PreBssIndex != BSSIndex)
			pmk_ap_free_sta_in_prebss(sta->PreBssIndex,sta->addr);	
		sta->PreBssIndex = BSSIndex;
		return 1;
	}
	bss = os_zalloc(sizeof(struct PMK_BSSInfo));
	if (bss == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc failed");
		return 0;
	}
	bss->BSSIndex = BSSIndex;
	bss->next = sta->bss;
	sta->bss = bss;	
//	pmk_del_bssindex(sta->PreBssIndex,sta);
	if (sta->PreBssIndex != BSSIndex)
		pmk_ap_free_sta_in_prebss(sta->PreBssIndex,sta->addr);
	sta->PreBssIndex = BSSIndex;
	sta->BSSIndex[BSSIndex] = 1;	
	return 1;
}

int pmk_del_bssindex(unsigned int BSSIndex, struct PMK_STAINFO *sta){
	struct PMK_BSSInfo *bss;
	struct PMK_BSSInfo *bss1;
	bss1 = NULL;
	if((sta == NULL)||(sta->BSSIndex[BSSIndex] == 0))
		return 0;
	bss = sta->bss;
	if (bss == NULL) {
		return 0;
	}
	while((bss != NULL)&&(bss->BSSIndex != BSSIndex)){
		bss1 = bss;
		bss = bss->next;
	}
	if(bss != NULL){
		bss1->next = bss->next;
	}
	free(bss);
	bss = NULL;
	sta->BSSIndex[BSSIndex] = 0;
	return 0;
}

struct rsn_pmksa_cache_entry * wlan_pmksa_cache_get(unsigned char WLANID, unsigned int BSSIndex,
					       const u8 *spa, const u8 *aa, const u8 *pmkid)
{
	struct PMK_STAINFO *pmk_sta;
	struct PMK_BSSInfo *bss;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data *wasd;
	struct rsn_pmksa_cache_entry *entry;
	unsigned int RadioID;
	unsigned int BSS_L_ID;
	unsigned char new_pmkid[PMKID_LEN];
	if((ASD_WLAN[WLANID] == NULL)||(ASD_WLAN[WLANID]->sta_list == NULL))
		return NULL;
	pmk_sta = pmk_ap_get_sta(ASD_WLAN[WLANID],spa);
	if(pmk_sta == NULL)
		return NULL;
	bss = pmk_sta->bss;
	while(bss != NULL){
		if(bss->BSSIndex != BSSIndex){
			RadioID = bss->BSSIndex/L_BSS_NUM;
			BSS_L_ID = bss->BSSIndex%L_BSS_NUM;
			if((interfaces->iface[RadioID]!=NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID]!=NULL))
			{
				wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
				if ((wasd == NULL) ||
					(wasd->wpa_auth == NULL) ||
					(wasd->wpa_auth->pmksa == NULL))
					continue;
				entry = pmksa_cache_get(wasd->wpa_auth->pmksa, spa, NULL);
				if(entry == NULL){
					asd_printf(ASD_DEFAULT,MSG_WARNING, "entry is NULL in func: %s\n",__func__);
					bss = bss->next;
					continue;
				}
				rsn_pmkid(entry->pmk,entry->pmk_len,aa,spa,new_pmkid);
				if(memcmp(new_pmkid, pmkid, PMKID_LEN) == 0)
				{

					if (memcmp(entry->pmkid, pmkid, PMKID_LEN)) {
						wpa_hexdump(MSG_INFO, "STA PMKID: ", entry->pmkid, PMKID_LEN);
						wpa_hexdump(MSG_INFO, "PMKID: ", pmkid, PMKID_LEN);
					}

					/* delete from hash, then update pmkid, insert again */					
					pmksa_cache_delete_from_hash(wasd->wpa_auth->pmksa, entry);
					memcpy(entry->pmkid, pmkid, PMKID_LEN);
					pmksa_cache_insert_to_hash(wasd->wpa_auth->pmksa, entry);
					if (pmk_sta->PreBssIndex != BSSIndex)
						pmk_ap_free_sta_in_prebss(pmk_sta->PreBssIndex,pmk_sta->addr);
					pmk_sta->PreBssIndex = BSSIndex;
					return entry;
				}
			}
		}
		bss = bss->next;
	}
	return NULL;
}

void pmk_ap_free_sta_in_prebss(unsigned int BSSIndex, unsigned char *addr)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data *wasd;	
	struct sta_info *sta;
	unsigned int RadioID;
	unsigned int BSS_L_ID;
	RadioID = BSSIndex/L_BSS_NUM;
	BSS_L_ID = BSSIndex%L_BSS_NUM;
	if((interfaces->iface[RadioID]!=NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID]!=NULL))
	{		
		wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
		sta = ap_get_sta(wasd,addr);
		if(sta == NULL)
			return;
		ap_free_sta_for_pmk(wasd,sta);
	}
	return;
}

void pmk_wlan_del_all_sta(unsigned char WLANID)
{
	struct PMK_STAINFO *pmk_sta, *prev;
	if(ASD_WLAN[WLANID] != NULL){
		pmk_sta = ASD_WLAN[WLANID]->sta_list;
		while(pmk_sta){
			prev = pmk_sta;
			pmk_sta = pmk_sta->next;
			pmk_ap_free_sta(ASD_WLAN[WLANID],prev);
		}
	}
}

