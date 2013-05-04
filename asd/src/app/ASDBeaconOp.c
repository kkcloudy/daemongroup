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
* AsdBeacon.c
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


#include "includes.h"


#include "asd.h"
#include "ASD80211Op.h"
#include "ASDWPAOp.h"
#include "ASDWme.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDCallback.h"
#include "ASDStaInfo.h"
#include "ASD80211HOp.h"
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "circle.h"


static u8 ieee802_11_erp_info(struct asd_data *wasd)
{
	u8 erp = 0;

	if (wasd->iface->current_mode == NULL ||
	    wasd->iface->current_mode->mode != asd_MODE_IEEE80211G)
		return 0;

	switch (wasd->iconf->cts_protection_type) {
	case CTS_PROTECTION_FORCE_ENABLED:
		erp |= ERP_INFO_NON_ERP_PRESENT | ERP_INFO_USE_PROTECTION;
		break;
	case CTS_PROTECTION_FORCE_DISABLED:
		erp = 0;
		break;
	case CTS_PROTECTION_AUTOMATIC:
		if (wasd->iface->olbc)
			erp |= ERP_INFO_USE_PROTECTION;
		/* continue */
	case CTS_PROTECTION_AUTOMATIC_NO_OLBC:
		if (wasd->iface->num_sta_non_erp > 0) {
			erp |= ERP_INFO_NON_ERP_PRESENT |
				ERP_INFO_USE_PROTECTION;
		}
		break;
	}
	if (wasd->iface->num_sta_no_short_preamble > 0)
		erp |= ERP_INFO_BARKER_PREAMBLE_MODE;

	return erp;
}


static u8 * asd_eid_ds_params(struct asd_data *wasd, u8 *eid)
{
	*eid++ = WLAN_EID_DS_PARAMS;
	*eid++ = 1;
	if(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM] != NULL){
		
		*eid++ =ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ra_ch[wasd->Radio_G_ID%L_RADIO_NUM];
		
	}else{
		*eid++ = wasd->iconf->channel;
	}
	return eid;
}


static u8 * asd_eid_erp_info(struct asd_data *wasd, u8 *eid)
{
	if (wasd->iface->current_mode == NULL ||
	    wasd->iface->current_mode->mode != asd_MODE_IEEE80211G)
		return eid;

	/* Set NonERP_present and use_protection bits if there
	 * are any associated NonERP stations. */
	/* TODO: use_protection bit can be set to zero even if
	 * there are NonERP stations present. This optimization
	 * might be useful if NonERP stations are "quiet".
	 * See 802.11g/D6 E-1 for recommended practice.
	 * In addition, Non ERP present might be set, if AP detects Non ERP
	 * operation on other APs. */

	/* Add ERP Information element */
	*eid++ = WLAN_EID_ERP_INFO;
	*eid++ = 1;
	*eid++ = ieee802_11_erp_info(wasd);

	return eid;
}


static u8 * asd_eid_country(struct asd_data *wasd, u8 *eid,
				int max_len)
{
	u8 *pos = eid;

	if ((!wasd->iconf->ieee80211d && !wasd->iface->dfs_enable) ||
	    max_len < 6)
		return eid;

	*pos++ = WLAN_EID_COUNTRY;
	pos++; /* length will be set later */
	os_memcpy(pos, wasd->iconf->country, 3); /* e.g., 'US ' */
	pos += 3;

	if ((pos - eid) & 1)
		*pos++ = 0; /* pad for 16-bit alignment */

	eid[1] = (pos - eid) - 2;

	return pos;
}


static u8 * asd_eid_power_constraint(struct asd_data *wasd, u8 *eid)

{
	if (!wasd->iface->dfs_enable)
		return eid;
	*eid++ = WLAN_EID_PWR_CONSTRAINT;
	*eid++ = 1;
	*eid++ = wasd->iface->pwr_const;
	return eid;
}


static u8 * asd_eid_tpc_report(struct asd_data *wasd, u8 *eid)

{
	if (!wasd->iface->dfs_enable)
		return eid;
	*eid++ = WLAN_EID_TPC_REPORT;
	*eid++ = 2;
	*eid++ = wasd->iface->tx_power; /* TX POWER */
	*eid++ = 0; /* Link Margin */
	return eid;
}

static u8 * asd_eid_channel_switch(struct asd_data *wasd, u8 *eid)

{
	if (!wasd->iface->dfs_enable || !wasd->iface->channel_switch)
		return eid;
	*eid++ = WLAN_EID_CHANNEL_SWITCH;
	*eid++ = 3;
	*eid++ = CHAN_SWITCH_MODE_QUIET;
	*eid++ = wasd->iface->channel_switch; /* New channel */
	/* 0 - very soon; 1 - before next TBTT; num - after num beacons */
	*eid++ = 0;
	return eid;
}


static u8 * asd_eid_wpa(struct asd_data *wasd, u8 *eid, size_t len,
			    struct sta_info *sta)
{
	const u8 *ie;
	size_t ielen;

	ie = wpa_auth_get_wpa_ie(wasd->wpa_auth, &ielen);
	if (ie == NULL || ielen > len)
		return eid;

	os_memcpy(eid, ie, ielen);
	return eid + ielen;
}



/*xm add for balance*/
int free_balance_info_all_in_wlan(unsigned char wlanid){
	/*xm add 08/12/17*/
	asd_printf(ASD_80211,MSG_DEBUG,"enter %s\n",__func__);
	
	if(wlanid>WLAN_NUM-1){//qiuchen
		asd_printf(ASD_80211,MSG_DEBUG,"wlanid>WLAN_NUM in %s\n",__func__);
		return -1;
	}

	if(ASD_WLAN[wlanid]==NULL){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[wlanid]==NULL in %s\n",__func__);
		return -1;
	}
	
	if(ASD_WLAN[wlanid]->sta_from_which_bss_list!=NULL){
	
		log_node * p,*pp;
		bss_arrival_num *q,*qq;
	
		p=ASD_WLAN[wlanid]->sta_from_which_bss_list;
		while(p!=NULL){
			pp=p->next;
			q=p->from_bss_list;
			while(q!=NULL){
				qq=q->next;
				free(q);
				q=NULL;
				q=qq;
			}
							
			free(p);
			p=NULL;
			p=pp;
			}
	}

	ASD_WLAN[wlanid]->sta_from_which_bss_list=NULL;
	ASD_WLAN[wlanid]->sta_list_len=0;
	return 0;

}

int free_balance_info_in_wlan(unsigned char mac[],unsigned char wlanid){

	if(mac==NULL||wlanid>WLAN_NUM-1){
		asd_printf(ASD_80211,MSG_DEBUG,"mac==NULL||wlanid<0||wlanid>WLAN_NUM-1 in %s\n",__func__);
		return -1;
	}

	asd_printf(ASD_80211,MSG_DEBUG, "Removing STA " MACSTR " from wlan %u.\n",
		   MAC2STR(mac),wlanid);
	
	if(ASD_WLAN[wlanid]==NULL){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[%u]==NULL in %s\n",wlanid,__func__);
		return -1;
	}

	if(ASD_WLAN[wlanid]->sta_from_which_bss_list==NULL){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[%u]->sta_from_which_bss_list in %s\n",wlanid,__func__);
		return -1;
	}

	log_node * p,*pold;
	bss_arrival_num *q,*qq;

	p=pold=ASD_WLAN[wlanid]->sta_from_which_bss_list;
	for(;(p!=NULL)&&(0==memcmp(mac,p->mac,MAC_LEN));pold=p,p=p->next){

		if(p==pold){
			ASD_WLAN[wlanid]->sta_from_which_bss_list=p->next;
		}else{
			pold->next=p->next;
		}
		
		ASD_WLAN[wlanid]->sta_list_len--;

		q=p->from_bss_list;
		while(q!=NULL){
			qq=q->next;
			free(q);
			q=NULL;
			q=qq;
		}
		free(p);
		p=NULL;
		break;
	}
		
	return 0;
}


void sta_probe_login_wlan(struct asd_data *wasd, struct ieee80211_mgmt *mgmt){
//xm add 08/12/17

	asd_printf(ASD_80211,MSG_DEBUG,"func: %s\n",__func__);
	int match_mac=0,match_bss=0;
	WID_WLAN	*wlan;

	if(ASD_WLAN[wasd->WlanID]==NULL)
		return;
	
	wlan=ASD_WLAN[wasd->WlanID];

	if(wlan->sta_from_which_bss_list==NULL){//probe frame firest come.
	
		log_node *tem=(log_node*)os_zalloc(sizeof(log_node));
		if(tem==NULL){
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			exit(1);
		}

		tem->from_bss_list=NULL;
		
		memcpy(tem->mac,mgmt->sa,6);
		tem->next=NULL;

		bss_arrival_num *ban=(bss_arrival_num *)os_zalloc(sizeof(bss_arrival_num));
		if(ban==NULL) {
			asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
			exit(1);
		}
		
		ban->arrival_num=1;
		ban->bss_index=wasd->BSSIndex;
		ban->next=NULL;
		
		tem->from_bss_list=ban;
		tem->list_len=1;
		wlan->sta_from_which_bss_list=tem;
		wlan->sta_list_len=1;

	}
	else if(wlan->sta_from_which_bss_list!=NULL){// if probe frame is not the  first time,then  match mac

		log_node *p1=wlan->sta_from_which_bss_list;
		while(p1!=NULL){
			if(0==memcmp(p1->mac,mgmt->sa,6)){
				bss_arrival_num *p2;
				if(p1->from_bss_list==NULL) return;
				p2=p1->from_bss_list;

				while(p2!=NULL){
					if(p2->bss_index==wasd->BSSIndex){
						p2->arrival_num++;
						match_bss=1;
						break;
					}
					else
						p2=p2->next;
				}

				if(match_bss==0){

					bss_arrival_num *tem;
					tem=(bss_arrival_num*)os_zalloc(sizeof(bss_arrival_num));
					if(tem==NULL) {
						asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
						exit(1);
					}

					tem->arrival_num=1;
					tem->bss_index=wasd->BSSIndex;
					tem->next=p1->from_bss_list;
					p1->from_bss_list=tem;
					p1->list_len++;

				}else{
					match_bss=0;
				}
				match_mac=1;
				break;
			}
			else
				p1=p1->next;
		}
		
		if(0==match_mac){
			
			log_node *tem=(log_node*)os_zalloc(sizeof(log_node));
			if(tem==NULL) {
				asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
				exit(1);
			}
			
			tem->from_bss_list=NULL;
			
			memcpy(tem->mac,mgmt->sa,6);
			tem->next=NULL;
			
			bss_arrival_num *ban=(bss_arrival_num *)os_zalloc(sizeof(bss_arrival_num));
			if(ban == NULL) {
				asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
				exit(1);
			}
			
			ban->arrival_num=1;
			ban->bss_index=wasd->BSSIndex;
			ban->next=NULL;

			ban->next=tem->from_bss_list;
			tem->from_bss_list=ban;
			tem->list_len++;

			tem->next=wlan->sta_from_which_bss_list;
			wlan->sta_from_which_bss_list=tem;
			wlan->sta_list_len++;

		}
		else
			match_mac=0;
	}

}

void clear_prob_log_timer(void *circle_ctx, void *timeout_ctx){
	/*clear prob log in wlan every 90 seconds.
	   xm0805
	*/
	WID_WLAN	*wlan=NULL;

	if(circle_ctx==NULL){
		return ;
	}

	wlan=(WID_WLAN *)circle_ctx;
	
	asd_printf(ASD_80211,MSG_DEBUG,"enter %s  clear wlan %u log.\n",__func__,wlan->WlanID);
	
	free_balance_info_all_in_wlan(wlan->WlanID);

	circle_cancel_timeout(clear_prob_log_timer, circle_ctx, NULL);
	circle_register_timeout(CLEAR_PROB_INTERVAL, 0, clear_prob_log_timer, circle_ctx, NULL);
}

void handle_probe_req(struct asd_data *wasd, struct ieee80211_mgmt *mgmt,
		      size_t len)
{
	asd_printf(ASD_80211,MSG_DEBUG,"func: %s\n",__func__);
	struct ieee80211_mgmt *resp;
	struct ieee802_11_elems elems;
	char *ssid;
	u8 *pos, *epos, *ie;
	size_t ssid_len, ie_len;
	struct sta_info *sta = NULL;
	//unsigned int ret_bss_index = 0xffffffff;
	//struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;

	ie = mgmt->u.probe_req.variable;
	ie_len = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.probe_req));

	//xm add 08/12/17       
	if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->balance_switch==1)){         

		sta_probe_login_wlan(wasd, mgmt);
	}
	
	return;	 //asd don't send probe response.

	
	if (!wasd->iconf->send_probe_response)
		return;

	if (ieee802_11_parse_elems(wasd, ie, ie_len, &elems, 0) == ParseFailed)
	{
		asd_printf(ASD_80211,MSG_DEBUG, "Could not parse ProbeReq from " MACSTR,
			   MAC2STR(mgmt->sa));
		return;
	}


	ssid = NULL;
	ssid_len = 0;

	if ((!elems.ssid || !elems.supp_rates)) {
		asd_printf(ASD_80211,MSG_DEBUG, "STA " MACSTR " sent probe request "
			   "without SSID or supported rates element",
			   MAC2STR(mgmt->sa));
		return;
	}


	if (wasd->conf->ignore_broadcast_ssid && elems.ssid_len == 0) {
		asd_printf(ASD_80211,MSG_MSGDUMP, "Probe Request from " MACSTR " for "
			   "broadcast SSID ignored", MAC2STR(mgmt->sa));
		return;
	}


	sta = ap_get_sta(wasd, mgmt->sa);


	if (elems.ssid_len == 0 ||
	    (elems.ssid_len == wasd->conf->ssid.ssid_len &&
	     os_memcmp(elems.ssid, wasd->conf->ssid.ssid, elems.ssid_len) ==
	     0)) {
		ssid = wasd->conf->ssid.ssid;
		ssid_len = wasd->conf->ssid.ssid_len;
		if (sta)
			sta->ssid_probe = &wasd->conf->ssid;
	}


	if (!ssid) {
		if (!(mgmt->da[0] & 0x01)) {
			char ssid_txt[33];
			ieee802_11_print_ssid(ssid_txt, elems.ssid,
					      elems.ssid_len);
			asd_printf(ASD_80211,MSG_MSGDUMP, "Probe Request from " MACSTR
				   " for foreign SSID '%s'",
				   MAC2STR(mgmt->sa), ssid_txt);
		}
		return;
	}


	/* TODO: verify that supp_rates contains at least one matching rate
	 * with AP configuration */
#define MAX_PROBERESP_LEN 768
	resp = os_zalloc(MAX_PROBERESP_LEN);


	if (resp == NULL)
		return;
	epos = ((u8 *) resp) + MAX_PROBERESP_LEN;

	resp->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_PROBE_RESP);
	

	os_memcpy(resp->da, mgmt->sa, ETH_ALEN);


	os_memcpy(resp->sa, wasd->own_addr, ETH_ALEN);


	os_memcpy(resp->bssid, wasd->own_addr, ETH_ALEN);
	resp->u.probe_resp.beacon_int =
		host_to_le16(wasd->iconf->beacon_int);


	/* hardware or low-level driver will setup seq_ctrl and timestamp */
	resp->u.probe_resp.capab_info =
		host_to_le16(asd_own_capab_info(wasd, sta, 1));


	pos = resp->u.probe_resp.variable;
	*pos++ = WLAN_EID_SSID;
	*pos++ = ssid_len;


	os_memcpy(pos, ssid, ssid_len);


	pos += ssid_len;

	/*xm add  09.5.18*/
	if (elems.supp_rates!=NULL) {	//supp rates add.

			*pos++ = WLAN_EID_SUPP_RATES;
			*pos++ = elems.supp_rates_len;
			os_memcpy(pos, elems.supp_rates, elems.supp_rates_len);
			pos += elems.supp_rates_len;
	
	}
	//////////////////////////////////////////////////////
	

	/* Supported rates */
	pos = asd_eid_supp_rates(wasd, pos);


	/* DS Params */
	pos = asd_eid_ds_params(wasd, pos);


	pos = asd_eid_country(wasd, pos, epos - pos);


	pos = asd_eid_power_constraint(wasd, pos);
	pos = asd_eid_tpc_report(wasd, pos);


	/* ERP Information element */
	pos = asd_eid_erp_info(wasd, pos);


	/* Extended supported rates */
	pos = asd_eid_ext_supp_rates(wasd, pos);

	/*xm add  09.5.18*/
	if (elems.ext_supp_rates!=NULL) {	//ext rates add.

			*pos++ = WLAN_EID_EXT_SUPP_RATES;
			*pos++ = elems.ext_supp_rates_len;
			os_memcpy(pos, elems.ext_supp_rates, elems.ext_supp_rates_len);
			pos += elems.ext_supp_rates_len;
	
	}
	//////////////////////////////////////////////////////


	pos = asd_eid_wpa(wasd, pos, epos - pos, sta);


	/* Wi-Fi Wireless Multimedia Extensions */
	if (wasd->conf->wme_enabled)
		pos = asd_eid_wme(wasd, pos);
	

	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, (u8*)resp, pos - (u8 *) resp, IEEE802_11_MGMT);
	
//	CWCaptrue(msg.DataLen, msg.Data);
	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("handle_probe_req: send");


	os_free(resp);
	resp=NULL;

	asd_printf(ASD_80211,MSG_MSGDUMP, "STA " MACSTR " sent probe request for %s "
		   "SSID", MAC2STR(mgmt->sa),
		   elems.ssid_len == 0 ? "broadcast" : "our");
}


void ieee802_11_set_beacon(struct asd_data *wasd)
{
	struct ieee80211_mgmt *head;
	u8 *pos, *tail, *tailpos;
	int preamble;
	u16 capab_info;
	size_t head_len, tail_len;
	int cts_protection = ((ieee802_11_erp_info(wasd) &
			      ERP_INFO_USE_PROTECTION) ? 1 : 0);

#define BEACON_HEAD_BUF_SIZE 256
#define BEACON_TAIL_BUF_SIZE 512
	head = os_zalloc(BEACON_HEAD_BUF_SIZE);
	tailpos = tail = os_zalloc(BEACON_TAIL_BUF_SIZE);
	if (head == NULL || tail == NULL) {
		asd_printf(ASD_80211,MSG_ERROR, "Failed to set beacon data");
		os_free(head);
		os_free(tail);
		return;
	}

	head->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_BEACON);
	head->duration = host_to_le16(0);
	os_memset(head->da, 0xff, ETH_ALEN);

	os_memcpy(head->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(head->bssid, wasd->own_addr, ETH_ALEN);
	head->u.beacon.beacon_int =
		host_to_le16(wasd->iconf->beacon_int);

	/* hardware or low-level driver will setup seq_ctrl and timestamp */
	capab_info = asd_own_capab_info(wasd, NULL, 0);
	head->u.beacon.capab_info = host_to_le16(capab_info);
	pos = &head->u.beacon.variable[0];

	/* SSID */
	*pos++ = WLAN_EID_SSID;
	if (wasd->conf->ignore_broadcast_ssid == 2) {
		/* clear the data, but keep the correct length of the SSID */
		*pos++ = wasd->conf->ssid.ssid_len;
		os_memset(pos, 0, wasd->conf->ssid.ssid_len);
		pos += wasd->conf->ssid.ssid_len;
	} else if (wasd->conf->ignore_broadcast_ssid) {
		*pos++ = 0; /* empty SSID */
	} else {
		*pos++ = wasd->conf->ssid.ssid_len;
		os_memcpy(pos, wasd->conf->ssid.ssid,
			  wasd->conf->ssid.ssid_len);
		pos += wasd->conf->ssid.ssid_len;
	}

	/* Supported rates */
	pos = asd_eid_supp_rates(wasd, pos);

	/* DS Params */
	pos = asd_eid_ds_params(wasd, pos);

	head_len = pos - (u8 *) head;

	tailpos = asd_eid_country(wasd, tailpos,
				      tail + BEACON_TAIL_BUF_SIZE - tailpos);

	tailpos = asd_eid_power_constraint(wasd, tailpos);
	tailpos = asd_eid_channel_switch(wasd, tailpos);
	tailpos = asd_eid_tpc_report(wasd, tailpos);

	/* ERP Information element */
	tailpos = asd_eid_erp_info(wasd, tailpos);

	/* Extended supported rates */
	tailpos = asd_eid_ext_supp_rates(wasd, tailpos);

	tailpos = asd_eid_wpa(wasd, tailpos, tail + BEACON_TAIL_BUF_SIZE -
				  tailpos, NULL);

	/* Wi-Fi Wireless Multimedia Extensions */
	if (wasd->conf->wme_enabled)
		tailpos = asd_eid_wme(wasd, tailpos);

	tail_len = tailpos > tail ? tailpos - tail : 0;

	if (asd_set_beacon(wasd->conf->iface, wasd, (u8 *) head, head_len,
			       tail, tail_len))
		asd_printf(ASD_80211,MSG_ERROR, "Failed to set beacon head/tail");

	os_free(tail);
	os_free(head);

	if (asd_set_cts_protect(wasd, cts_protection))
		asd_printf(ASD_80211,MSG_ERROR, "Failed to set CTS protect in kernel "
			   "driver");

	if (wasd->iface->current_mode &&
	    wasd->iface->current_mode->mode == asd_MODE_IEEE80211G &&
	    asd_set_short_slot_time(wasd,
					wasd->iface->num_sta_no_short_slot_time
					> 0 ? 0 : 1))
		asd_printf(ASD_80211,MSG_ERROR, "Failed to set Short Slot Time option "
			   "in kernel driver");

	if (wasd->iface->num_sta_no_short_preamble == 0 &&
	    wasd->iconf->preamble == SHORT_PREAMBLE)
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;
	if (asd_set_preamble(wasd, preamble))
		asd_printf(ASD_80211,MSG_ERROR, "Could not set preamble for kernel "
			   "driver");
}


void ieee802_11_set_beacons(struct asd_iface *iface)
{
	size_t i;
	for (i = 0; i < iface->num_bss; i++)
	{
		if(iface->bss[i] == NULL){
			asd_printf(ASD_80211,MSG_DEBUG,"iface->num_bss:%d\n",iface->num_bss);
			return;
		}
		ieee802_11_set_beacon(iface->bss[i]);
	}
}

