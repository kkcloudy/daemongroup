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
* AsdPreAuthBss.c
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
#include "dbus/asd/ASDDbusDef1.h"
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
#include "ASDPreauth.h"
#include "ASDPreauthBSS.h"

#define BSS_HASH(bss) (bss[5])

struct PreAuth_BSSINFO * PreAuth_wlan_get_bss(WID_WLAN *WLAN, const u8 *addr)
{
	struct PreAuth_BSSINFO *s;

	s = WLAN->bss_hash[BSS_HASH(addr)];
	while (s != NULL && os_memcmp(s->addr, addr, 6) != 0)
		s = s->hnext;
	return s;
}


static void PreAuth_wlan_bss_list_del(WID_WLAN *WLAN, struct PreAuth_BSSINFO *bss)
{
	struct PreAuth_BSSINFO *tmp;

	if (WLAN->bss_list == bss) {
		WLAN->bss_list = bss->next;
		return;
	}

	tmp = WLAN->bss_list;
	while (tmp != NULL && tmp->next != bss)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove bss " MACSTR " from "
			   "list.", MAC2STR(bss->addr));
	} else
		tmp->next = bss->next;
}


void PreAuth_wlan_bss_hash_add(WID_WLAN *WLAN, struct PreAuth_BSSINFO *bss)
{
	bss->hnext = WLAN->bss_hash[BSS_HASH(bss->addr)];
	WLAN->bss_hash[BSS_HASH(bss->addr)] = bss;
}


static void PreAuth_wlan_bss_hash_del(WID_WLAN *WLAN, struct PreAuth_BSSINFO *bss)
{
	struct PreAuth_BSSINFO *s;

	s = WLAN->bss_hash[BSS_HASH(bss->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, bss->addr, 6) == 0) {
		WLAN->bss_hash[BSS_HASH(bss->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       os_memcmp(s->hnext->addr, bss->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR, "wlan: could not remove bss " MACSTR
			   " from hash table", MAC2STR(bss->addr));
}


void PreAuth_wlan_free_bss(WID_WLAN *WLAN, struct PreAuth_BSSINFO *bss)
{
	//int set_beacon = 0;
	//int i=0;
		
	PreAuth_wlan_bss_hash_del(WLAN, bss);
	PreAuth_wlan_bss_list_del(WLAN, bss);
	bss->BSSIndex = 0; 
	WLAN->num_bss--;
	os_free(bss);

}


struct PreAuth_BSSINFO* PreAuth_wlan_bss_add(WID_WLAN *WLAN, const u8 *addr, unsigned int BSSIndex)
{
	struct PreAuth_BSSINFO *bss;
	bss = PreAuth_wlan_get_bss(WLAN, addr);
	if (bss)
		return bss;

	bss = os_zalloc(sizeof(struct PreAuth_BSSINFO));
	if (bss == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc failed");
		return NULL;
	}	
	os_memcpy(bss->addr, addr, ETH_ALEN);
	bss->next = WLAN->bss_list;
	WLAN->bss_list = bss;
	WLAN->num_bss++;
	PreAuth_wlan_bss_hash_add(WLAN, bss);
	memset(bss->addr, 0, MAC_LEN);
	memcpy(bss->addr, addr, MAC_LEN);
	bss->BSSIndex = BSSIndex;
	return bss;
	
}

void PreAuth_wlan_del_all_bss(unsigned char WLANID)
{
	struct PreAuth_BSSINFO *bss, *prev;
	if(ASD_WLAN[WLANID] != NULL){
		bss = ASD_WLAN[WLANID]->bss_list;
		while(bss){
			prev = bss;
			bss = bss->next;
			PreAuth_wlan_free_bss(ASD_WLAN[WLANID],prev);
		}
	}
}

void rsn_preauth_receive_thinap(void *ctx, const u8 *src_addr,const u8 *des_addr,
				const u8 *buf, size_t len)
{
	
	struct asd_data *now_wasd = ctx;
	struct ieee802_1x_hdr *hdr;
	struct sta_info *sta;
	struct asd_data * wasd;
	struct PreAuth_BSSINFO *bss;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int RadioID;
	unsigned char BSS_L_ID;
	bss = PreAuth_wlan_get_bss(ASD_WLAN[now_wasd->WlanID],des_addr);
	if(bss == NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"something wrong in finding bss: "MACSTR, MAC2STR(des_addr));
		return;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"we got bss index:%d",bss->BSSIndex);
	RadioID = bss->BSSIndex/L_BSS_NUM;
	BSS_L_ID = bss->BSSIndex%L_BSS_NUM;
	if((interfaces->iface[RadioID]!=NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID]!=NULL))
		wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
	else 
		return;
	if (len < sizeof(*hdr)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: too short pre-auth packet "
			   "(len=%lu)", (unsigned long) len);
		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rsn_preauth_receive_thinap RSN: too short pre-auth packet "
			   "(len=%lu)", (unsigned long) len);
		return;
	}

	hdr = (struct ieee802_1x_hdr *) (buf);

	if (os_memcmp(des_addr, wasd->own_addr, ETH_ALEN) != 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: pre-auth for foreign address "
			   MACSTR, MAC2STR(des_addr));		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rsn_preauth_receive_thinap RSN: pre-auth for foreign address "
			   MACSTR, MAC2STR(des_addr));
		return;
	}

	sta = ap_get_sta(wasd, src_addr);
	if (sta && (sta->flags & WLAN_STA_ASSOC)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: pre-auth for already association "
			   "STA " MACSTR, MAC2STR(sta->addr));
		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"rsn_preauth_receive_thinap RSN: pre-auth for already association "
			   "STA " MACSTR, MAC2STR(sta->addr));
		return;
	}
	if (!sta && hdr->type == IEEE802_1X_TYPE_EAPOL_START) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"IEEE802_1X_TYPE_EAPOL_START\n");
		sta = ap_sta_add(wasd, src_addr,0);
		if (sta == NULL)
			return;
		sta->flags = WLAN_STA_PREAUTH;

		ieee802_1x_new_station(wasd, sta);
		if (sta->eapol_sm == NULL) {
			ap_free_sta(wasd, sta,0);
			sta = NULL;
		} else {
			sta->eapol_sm->radius_identifier = -1;
			sta->eapol_sm->portValid = TRUE;
			sta->eapol_sm->flags |= EAPOL_SM_PREAUTH;
		}
	}
	if (sta == NULL)
		return;
	memcpy(sta->PreAuth_BSSID,now_wasd->own_addr,6);
	sta->PreAuth_BSSIndex = now_wasd->BSSIndex;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"ieee802_1x_receive \n");
	ieee802_1x_receive(wasd, src_addr, (u8 *) (buf),
			   len);
}



int rsn_preauth_send_thinap(void *priv, const u8 *addr, const u8 *data,
			     size_t data_len, int encrypt, const u8 *own_addr, unsigned int PreAuth_BSSIndex)
{
	//struct asd_data *now_wasd = priv;
	struct asd_data *wasd;
	struct ieee80211_hdr *hdr;	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int RadioID;
	unsigned char BSS_L_ID;
	size_t len;
	u8 *pos;
	int res;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rsn_preauth_send_thinap\n");
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"PreAuth_BSSIndex :%d\n",PreAuth_BSSIndex);
	RadioID = PreAuth_BSSIndex/L_BSS_NUM;
	BSS_L_ID = PreAuth_BSSIndex%L_BSS_NUM;
	if((interfaces->iface[RadioID]!=NULL)&&(interfaces->iface[RadioID]->bss[BSS_L_ID]!=NULL))
		wasd = interfaces->iface[RadioID]->bss[BSS_L_ID];
	else
		return -1;	
	struct wid_driver_data *drv = wasd->drv_priv;
	len = sizeof(*hdr) + sizeof(rfc1042_header) + 2 + data_len;
	hdr = os_zalloc(len);
	if (hdr == NULL) {
		asd_printf(ASD_DEFAULT,MSG_WARNING,"malloc() failed for asd_send_data(len=%lu)\n",
		       (unsigned long) len);
		return -1;
	}

	hdr->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	hdr->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	/* Request TX callback */
	hdr->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	if (encrypt)
		hdr->frame_control |= host_to_le16(WLAN_FC_ISWEP);
	memcpy(hdr->IEEE80211_DA_FROMDS, addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_BSSID_FROMDS, wasd->own_addr, ETH_ALEN);
	memcpy(hdr->IEEE80211_SA_FROMDS, own_addr, ETH_ALEN);
	hdr->seq_ctrl = seq_to_le16(wasd->seq_num++);
	if(wasd->seq_num == 4096)
		wasd->seq_num = 0;

	pos = (u8 *) (hdr + 1);
	memcpy(pos, rfc1042_header, sizeof(rfc1042_header));
	pos += sizeof(rfc1042_header);
	*((u16 *) pos) = htons(ETH_P_PREAUTH);
	pos += 2;
	memcpy(pos, data, data_len);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, (u8 *) hdr, len, IEEE802_11_EAP);
//	CWCaptrue(msg.DataLen, msg.Data);
	res = asd_send_mgmt_frame_w(drv, &msg, msglen, 0);
	free(hdr);

	if (res < 0) {
		perror("asd_send_eapol: send");
		asd_printf(ASD_DEFAULT,MSG_WARNING,"asd_send_eapol - packet len: %lu - failed\n",
		       (unsigned long) len);
	}

	return res;
}



