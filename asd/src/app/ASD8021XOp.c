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
* Asd1X.c
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
#include "ASD8021XOp.h"
#include "ASDAccounting.h"
#include "ASDRadius/radius.h"
#include "ASDRadius/radius_client.h"
#include "ASDEapolSM.h"
#include "md5.h"
#include "rc4.h"
#include "circle.h"
#include "ASDStaInfo.h"
#include "ASDWPAOp.h"
#include "ASDPreauth.h"
#include "ASDPMKCache.h"
#include "ASDCallback.h"
#include "ASDHWInfo.h"
#include "ASDEAPMethod/eap.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ASD80211Op.h"
#include "ASDMlme.h"

#include "ASDDbus.h"
#include "asd_bak.h"
#include "ASDPMKSta.h"
#include "ASDPreauthBSS.h"
#include "asd_iptables.h"			//mahz add 2011.2.25	
#include "ASDDbus_handler.h"
#include "ASDEAPAuth.h"
#include "syslog.h"//qiuchen
extern unsigned char gASDLOGDEBUG;
extern unsigned long gASD_AC_MANAGEMENT_IP;

static void ieee802_1x_finished(struct asd_data *wasd,
				struct sta_info *sta, int success);


static void ieee802_1x_send(struct asd_data *wasd, struct sta_info *sta,
			    u8 type, const u8 *data, size_t datalen)
{
	u8 *buf;
	struct ieee802_1x_hdr *xhdr;
	size_t len;
	int encrypt = 0;
	unsigned char SecurityID;
	len = sizeof(*xhdr) + datalen;
	buf = os_zalloc(len);
	if (buf == NULL) {
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	SecurityID = wasd->SecurityID;
	if((ASD_SECURITY[SecurityID] != NULL)&&((ASD_SECURITY[SecurityID]->extensible_auth == 1)&&(ASD_SECURITY[SecurityID]->hybrid_auth == 0))&&(ASD_SECURITY[SecurityID]->encryptionType == WEP))
		encrypt = 1;
	xhdr = (struct ieee802_1x_hdr *) buf;
	xhdr->version = wasd->conf->eapol_version;
	xhdr->type = type;
	xhdr->length = host_to_be16(datalen);

	if (datalen > 0 && data != NULL)
		os_memcpy(xhdr + 1, data, datalen);

	if (wpa_auth_pairwise_set(sta->wpa_sm))
		encrypt = 1;
	if (sta->flags & WLAN_STA_PREAUTH) {
		rsn_preauth_send_thinap(wasd, sta->addr, buf, len, encrypt, wasd->own_addr, sta->PreAuth_BSSIndex);
	} else {
		asd_send_eapol(wasd, sta->addr, buf, len, encrypt);
	}

	os_free(buf);
}


void ieee802_1x_set_sta_authorized(struct asd_data *wasd,
				   struct sta_info *sta, int authorized)
{
	int res = 0;

	if (sta->flags & WLAN_STA_PREAUTH)
		return;

	//mahz modified  2011.2.24
	unsigned char SID = 0;
	if(ASD_WLAN[wasd->WlanID])
		SID = ASD_WLAN[wasd->WlanID]->SecurityID;
	if (authorized) {
		if(!((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1))){
			res = asd_sta_set_flags(wasd, sta->addr, sta->flags,
					    WLAN_STA_AUTHORIZED, ~0);
			if(ASD_NOTICE_STA_INFO_TO_PORTAL)
				AsdStaInfoToEAG(wasd,sta,WID_ADD);
			//AsdStaInfoToWID(wasd, sta->addr);
			asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_INFO, "authorizing port");
			eapol_keep_alive_timer(sta->eapol_sm,&ASD_SECURITY[SID]->eap_alive_period);
		}
		sta->flags |= WLAN_STA_AUTHORIZED;
		wasd->info->identify_success++;
		
		/*else if((ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1)){
			sta->security_type = HYBRID_AUTH_EAPOL;
			sta_ip_addr_check(wasd,sta);
		}*/
		if (sta->eapol_sm != NULL&&ASD_SECURITY[SID]&&(ASD_SECURITY[SID]->eap_sm_run_activated==0)) 
			circle_cancel_timeout(eapol_port_timers_tick, NULL, sta->eapol_sm);
	} else {
		if(!((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1))){
		res = asd_sta_set_flags(wasd, sta->addr, sta->flags,
					    0, ~WLAN_STA_AUTHORIZED);
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_NOTICE, "unauthorizing port");
		}
		sta->flags &= ~WLAN_STA_AUTHORIZED;
	}

	if (res && errno != ENOENT) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not set station " MACSTR " flags for kernel "
		       "driver (errno=%d).\n", MAC2STR(sta->addr), errno);
	}

	if (authorized){
		struct PMK_STAINFO *p_sta;
		//mahz add 2011.7.13
		//unsigned char SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
		if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->fast_auth == 1)){
			p_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID], sta->addr);
			if((p_sta != NULL) && (p_sta->PreBssIndex != 0) && (wasd->conf->radius != NULL) && (wasd->conf->radius->radius_extend_attr != 0)){
				asd_printf(ASD_1X,MSG_DEBUG,"STA PreBssindex is %d\n",p_sta->PreBssIndex);
				sta->acct_session_id_hi = p_sta->idhi;
				sta->acct_session_id_lo = p_sta->idlo;
				sta->acct_session_started = 1;
			}
		}
		if(is_secondary == 0)
			bak_add_sta(wasd,sta);
		if((ASD_SECURITY[SID])&&(1 == ASD_SECURITY[SID]->account_after_authorize)){
			accounting_sta_start(wasd, sta);
		}
		else if(ASD_SECURITY[SID]&&(sta->ipaddr != 0))
		{
			accounting_sta_start(wasd,sta);
			if((NO_NEED_AUTH == sta->security_type) || (HYBRID_AUTH_EAPOL_SUCCESS(sta)))
			{
				if(asd_ipset_switch)
					eap_connect_up(sta->ip_addr.s_addr);
				else
					AsdStaInfoToEAG(wasd,sta,ASD_AUTH);
			}
		}
		if((1 ==asd_sta_getip_from_dhcpsnoop)&&(0 == sta->ipaddr))
			asd_notice_to_dhcp(wasd,sta->addr,DHCP_IP);
		unsigned char mac[6];
		int i=0;
		for(i=0;i<6;i++)
			mac[i]=sta->addr[i];

		unsigned char rssi = sta->rssi;	//xiaodawei add rssi for telecom, 20110301

		if(!((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1)))	//mahz add 2011.5.9
			wasd->acc_tms++;
		wasd->assoc_auth_sta_num++; 	//mahz add 2011.11.9 for GuangZhou Mobile
		wasd->assoc_auth_succ_num++;
		sta->sta_assoc_auth_flag = 1;
		//qiuchen
		if (ASD_AUTH_TYPE_EAP(wasd->SecurityID)) /* EAP auth */
		{
			if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))	/* SIM/PEAP auth sta */
			{
				wasd->u.eap_auth.autoauth.online_sta_num++;
			}
		}
		//end
		//qiuchen add it for Henan Mobile
		if(gASDLOGDEBUG & BIT(1)){
			if(sta->rflag && !(sta->logflag&BIT(1)) && sta->flags & WLAN_STA_AUTHORIZED){
				syslog(LOG_INFO|LOG_LOCAL3,"STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
					MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					sta->preAPID,MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
				);
				sta->logflag = BIT(1);
			}
		}
		if(gASDLOGDEBUG & BIT(0)){
			if(sta->rflag && !(sta->logflag&BIT(0)) && sta->flags & WLAN_STA_AUTHORIZED){
				asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
									((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
									((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
				sta->logflag = BIT(0);
			}
			//else
				//asd_syslog_h("A BAC-M PORTSEC/6/PORTSEC_DOT1X_LOGIN_SUCC","-IfName=%s-MACAddr="MACSTR"-VlanId=%d-UserName=%s; The user passed 802.1X authentication and got online successfully\n",NULL,MAC2STR(sta->addr),sta->vlan_id,sta->eapol_sm->identity);
		}
		//end
		signal_sta_come(mac,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,rssi);
		if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type && wasd->br_ifname){
			char ifname[IF_NAME_MAX]={0};
			sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
			add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,1) ;
		}
	}
}


static void ieee802_1x_eap_timeout(void *circle_ctx, void *timeout_ctx)
{
	struct sta_info *sta = circle_ctx;
	struct eapol_state_machine *sm = sta->eapol_sm;
	if (sm == NULL)
		return;
	asd_logger(sm->wasd, sta->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "EAP timeout");
	sm->eap_if->eapTimeout = TRUE;
	eapol_auth_step(sm);
}


static void ieee802_1x_tx_key_one(struct asd_data *wasd,
				  struct sta_info *sta,
				  int idx, int broadcast,
				  u8 *key_data, size_t key_len)
{
	u8 *buf, *ekey;
	struct ieee802_1x_hdr *hdr;
	struct ieee802_1x_eapol_key *key;
	size_t len, ekey_len;
	struct eapol_state_machine *sm = sta->eapol_sm;

	if (sm == NULL)
		return;

	len = sizeof(*key) + key_len;
	buf = os_zalloc(sizeof(*hdr) + len);
	if (buf == NULL)
		return;

	hdr = (struct ieee802_1x_hdr *) buf;
	key = (struct ieee802_1x_eapol_key *) (hdr + 1);
	key->type = EAPOL_KEY_TYPE_RC4;
	key->key_length = htons(key_len);
	wpa_get_ntp_timestamp(key->replay_counter);

	if (os_get_random(key->key_iv, sizeof(key->key_iv))) {
		asd_printf(ASD_1X,MSG_ERROR, "Could not get random numbers");
		os_free(buf);
		return;
	}

	key->key_index = idx | (broadcast ? 0 : BIT(7));
	if (wasd->conf->eapol_key_index_workaround) {
		/* According to some information, WinXP Supplicant seems to
		 * interpret bit7 as an indication whether the key is to be
		 * activated, so make it possible to enable workaround that
		 * sets this bit for all keys. */
		key->key_index |= BIT(7);
	}

	/* Key is encrypted using "Key-IV + MSK[0..31]" as the RC4-key and
	 * MSK[32..63] is used to sign the message. */
	if (sm->eap_if->eapKeyData == NULL || sm->eap_if->eapKeyDataLen < 64) {
		asd_printf(ASD_1X,MSG_ERROR, "No eapKeyData available for encrypting "
			   "and signing EAPOL-Key");
		os_free(buf);
		return;
	}
	os_memcpy((u8 *) (key + 1), key_data, key_len);
	ekey_len = sizeof(key->key_iv) + 32;
	ekey = os_zalloc(ekey_len);
	if (ekey == NULL) {
		asd_printf(ASD_1X,MSG_ERROR, "Could not encrypt key");
		os_free(buf);
		return;
	}
	os_memcpy(ekey, key->key_iv, sizeof(key->key_iv));
	os_memcpy(ekey + sizeof(key->key_iv), sm->eap_if->eapKeyData, 32);
	rc4((u8 *) (key + 1), key_len, ekey, ekey_len);
	os_free(ekey);

	/* This header is needed here for HMAC-MD5, but it will be regenerated
	 * in ieee802_1x_send() */
	hdr->version = wasd->conf->eapol_version;
	hdr->type = IEEE802_1X_TYPE_EAPOL_KEY;
	hdr->length = host_to_be16(len);
	hmac_md5(sm->eap_if->eapKeyData + 32, 32, buf, sizeof(*hdr) + len,
		 key->key_signature);

	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Sending EAPOL-Key to " MACSTR
		   " (%s index=%d)", MAC2STR(sm->addr),
		   broadcast ? "broadcast" : "unicast", idx);
	ieee802_1x_send(wasd, sta, IEEE802_1X_TYPE_EAPOL_KEY, (u8 *) key, len);
	if (sta->eapol_sm)
		sta->eapol_sm->dot1xAuthEapolFramesTx++;
	os_free(buf);
}


static struct asd_wep_keys *
ieee802_1x_group_alloc(struct asd_data *wasd, const char *ifname)
{
	struct asd_wep_keys *key;

	key = os_zalloc(sizeof(*key));
	if (key == NULL)
		return NULL;

	key->default_len = wasd->conf->default_wep_key_len;

	if (key->idx >= wasd->conf->broadcast_key_idx_max ||
	    key->idx < wasd->conf->broadcast_key_idx_min)
		key->idx = wasd->conf->broadcast_key_idx_min;
	else
		key->idx++;

	if (!key->key[key->idx])
		key->key[key->idx] = os_zalloc(key->default_len);
	if (key->key[key->idx] == NULL ||
	    os_get_random(key->key[key->idx], key->default_len)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not generate random WEP key (dynamic VLAN).\n");
		os_free(key->key[key->idx]);
		key->key[key->idx] = NULL;
		os_free(key);
		return NULL;
	}
	key->len[key->idx] = key->default_len;

	asd_printf(ASD_1X,MSG_DEBUG, "%s: Default WEP idx %d for dynamic VLAN\n",
		   ifname, key->idx);
	wpa_hexdump_key(MSG_DEBUG, "Default WEP key (dynamic VLAN)",
			key->key[key->idx], key->len[key->idx]);

	if (asd_set_encryption(ifname, wasd, "WEP", NULL, key->idx,
				   key->key[key->idx], key->len[key->idx], 1))
		asd_printf(ASD_1X,MSG_DEBUG,"Could not set dynamic VLAN WEP encryption key.\n");

	asd_set_ieee8021x(ifname, wasd, 1);

	return key;
}


static struct asd_wep_keys *
ieee802_1x_get_group(struct asd_data *wasd, struct asd_ssid *ssid,
		     size_t vlan_id)
{
	const char *ifname;

	if (vlan_id == 0)
		return &ssid->wep;

	if (vlan_id <= ssid->max_dyn_vlan_keys && ssid->dyn_vlan_keys &&
	    ssid->dyn_vlan_keys[vlan_id])
		return ssid->dyn_vlan_keys[vlan_id];

	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Creating new group "
		   "state machine for VLAN ID %lu",
		   (unsigned long) vlan_id);

	ifname = asd_get_vlan_id_ifname(wasd->conf->vlan, vlan_id);
	if (ifname == NULL) {
		asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Unknown VLAN ID %lu - "
			   "cannot create group key state machine",
			   (unsigned long) vlan_id);
		return NULL;
	}

	if (ssid->dyn_vlan_keys == NULL) {
		int size = (vlan_id + 1) * sizeof(ssid->dyn_vlan_keys[0]);
		ssid->dyn_vlan_keys = os_zalloc(size);
		if (ssid->dyn_vlan_keys == NULL)
			return NULL;
		ssid->max_dyn_vlan_keys = vlan_id;
	}

	if (ssid->max_dyn_vlan_keys < vlan_id) {
		struct asd_wep_keys **na;
		int size = (vlan_id + 1) * sizeof(ssid->dyn_vlan_keys[0]);
		na = os_realloc(ssid->dyn_vlan_keys, size);
		if (na == NULL)
			return NULL;
		ssid->dyn_vlan_keys = na;
		os_memset(&ssid->dyn_vlan_keys[ssid->max_dyn_vlan_keys + 1], 0,
			  (vlan_id - ssid->max_dyn_vlan_keys) *
			  sizeof(ssid->dyn_vlan_keys[0]));
		ssid->max_dyn_vlan_keys = vlan_id;
	}

	ssid->dyn_vlan_keys[vlan_id] = ieee802_1x_group_alloc(wasd, ifname);

	return ssid->dyn_vlan_keys[vlan_id];
}


void ieee802_1x_tx_key(struct asd_data *wasd, struct sta_info *sta)
{
	struct asd_wep_keys *key = NULL;
	struct eapol_state_machine *sm = sta->eapol_sm;
	int vlan_id;

	if (sm == NULL || !sm->eap_if->eapKeyData)
		return;

	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Sending EAPOL-Key(s) to " MACSTR,
		   MAC2STR(sta->addr));

	vlan_id = sta->vlan_id;
	if (vlan_id < 0 || vlan_id > MAX_VLAN_ID)
		vlan_id = 0;

	if (vlan_id) {
		key = ieee802_1x_get_group(wasd, sta->ssid, vlan_id);
		if (key && key->key[key->idx])
			ieee802_1x_tx_key_one(wasd, sta, key->idx, 1,
					      key->key[key->idx],
					      key->len[key->idx]);
	} else if (wasd->default_wep_key) {
		ieee802_1x_tx_key_one(wasd, sta, wasd->default_wep_key_idx, 1,
				      wasd->default_wep_key,
				      wasd->conf->default_wep_key_len);
	}

	if (wasd->conf->individual_wep_key_len > 0) {
		u8 *ikey;
		ikey = os_zalloc(wasd->conf->individual_wep_key_len);
		if (ikey == NULL ||
		    os_get_random(ikey, wasd->conf->individual_wep_key_len)) {
			asd_printf(ASD_1X,MSG_ERROR, "Could not generate random "
				   "individual WEP key.");
			os_free(ikey);
			return;
		}

		wpa_hexdump_key(MSG_DEBUG, "Individual WEP key",
				ikey, wasd->conf->individual_wep_key_len);

		ieee802_1x_tx_key_one(wasd, sta, 0, 0, ikey,
				      wasd->conf->individual_wep_key_len);

		/* TODO: set encryption in TX callback, i.e., only after STA
		 * has ACKed EAPOL-Key frame */
		if (asd_set_encryption(wasd->conf->iface, wasd, "WEP",
					   sta->addr, 0, ikey,
					   wasd->conf->individual_wep_key_len,
					   1)) {
			asd_printf(ASD_1X,MSG_ERROR, "Could not set individual WEP "
				   "encryption.");
		}

		os_free(ikey);
	}
}


const char *radius_mode_txt(struct asd_data *wasd)
{
	if (wasd->iface->current_mode == NULL)
		return "802.11";

	switch (wasd->iface->current_mode->mode) {
	case asd_MODE_IEEE80211A:
		return "802.11a";
	case asd_MODE_IEEE80211G:
		return "802.11g";
	case asd_MODE_IEEE80211B:
	default:
		return "802.11b";
	}
}


int radius_sta_rate(struct asd_data *wasd, struct sta_info *sta)
{
	int i;
	u8 rate = 0;

	for (i = 0; i < sta->supported_rates_len; i++)
		if ((sta->supported_rates[i] & 0x7f) > rate)
			rate = sta->supported_rates[i] & 0x7f;

	return rate;
}


static void ieee802_1x_learn_identity(struct asd_data *wasd,
				      struct eapol_state_machine *sm,
				      const u8 *eap, size_t len)
{
	const u8 *identity;
	size_t identity_len;

	if (len <= sizeof(struct eap_hdr) ||
	    eap[sizeof(struct eap_hdr)] != EAP_TYPE_IDENTITY)
		return;

	identity = eap_get_identity(sm->eap, &identity_len);
	if (identity == NULL)
		return;

	/* Save station identity for future RADIUS packets */
	os_free(sm->identity);
	sm->identity = os_zalloc(identity_len + 1);
	if (sm->identity == NULL) {
		sm->identity_len = 0;
		return;
	}

	os_memcpy(sm->identity, identity, identity_len);
	sm->identity_len = identity_len;
	sm->identity[identity_len] = '\0';
#if 0
	asd_logger(wasd, sm->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "STA identity '%s'", sm->identity);
#endif
	sm->dot1xAuthEapolRespIdFramesRx++;
}


static void ieee802_1x_encapsulate_radius(struct asd_data *wasd,
					  struct sta_info *sta,
					  const u8 *eap, size_t len)
{
	struct radius_msg *msg;
	char buf[128];
	struct eapol_state_machine *sm = sta->eapol_sm;
	unsigned char SID = wasd->SecurityID;
	unsigned int wtpid;
	struct radius_client_info *client_info;
	if (sm == NULL){
		asd_printf(ASD_1X,MSG_DEBUG, "sm is null in %s\n",__func__);
		return;
	}
	client_info = radius_client_get_sock(wasd->WlanID,1);
	if(client_info == NULL)
	{
		asd_printf(ASD_1X,MSG_ERROR,"Find radius auth socket error,there is no socket exist!\n");
		return;
	}
	if(wasd->conf->wapi_radius_auth_enable != 1){
		ieee802_1x_learn_identity(wasd, sm, eap, len);
		
		asd_printf(ASD_1X,MSG_DEBUG, "Encapsulating EAP message into a RADIUS "
			   "packet");
			if((sm->wasd)&&(sm->radius_identifier < 256)&&(sm->radius_identifier >= 0))
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free the sta prev radius msg sta->wlanid :%d\n",sm->wasd->WlanID);
				radius_client_info_free(radius_client_get_sta_info(sta,sm->wasd->WlanID,RADIUS_AUTH));
			}
			sm->radius_identifier = radius_client_get_id(client_info);
			if((sm->radius_identifier <0)||(sm->radius_identifier > 255))
			{
				asd_printf(ASD_1X,MSG_WARNING,"Could not find the correct id!\n");
				return ; 
			}
		msg = radius_msg_new(RADIUS_CODE_ACCESS_REQUEST,
					 sm->radius_identifier);
		if (msg == NULL) {
			asd_printf(ASD_1X,MSG_WARNING,"Could not create net RADIUS packet\n");
			return;
		}
		
		radius_msg_make_authenticator(msg, (u8 *) sta, sizeof(*sta));
		
		if (sm->identity &&
			!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
					 sm->identity, sm->identity_len)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
			goto fail;
		}
	}else{
	//	asd_printf(ASD_1X,MSG_DEBUG,"enter wapi radius auth func : %s\n",__func__);
			
			if((sm->wasd)&&(sm->radius_identifier < 256)&&(sm->radius_identifier >= 0))
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"free the sta prev radius msg sta->wlanid :%d\n",sm->wasd->WlanID);
				radius_client_info_free(radius_client_get_sta_info(sta,sm->wasd->WlanID,RADIUS_AUTH));
			}
			sm->radius_identifier = radius_client_get_id(client_info);
			if((sm->radius_identifier <0)||(sm->radius_identifier > 255))
			{
				asd_printf(ASD_1X,MSG_WARNING,"Could not find the correct id!\n");
				return ; 
			}
		msg = radius_msg_new(RADIUS_CODE_ACCESS_REQUEST,
					 sm->radius_identifier);
		if (msg == NULL) {
			asd_printf(ASD_1X,MSG_WARNING,"Could not create net RADIUS packet\n");
			return;
		}
		
		radius_msg_make_authenticator(msg, (u8 *) sta, sizeof(*sta));

		if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
			(u8 *)sta->wapi_sta_info.serial_no.data,sta->wapi_sta_info.serial_no.length)) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
				goto fail;		
		}
	
		//mahz add 2010.12.7
		if(!radius_msg_add_attr_user_password(msg, wasd->conf->user_passwd, wasd->conf->user_passwd_len,
					 wasd->conf->radius->auth_server->shared_secret, 
					  wasd->conf->radius->auth_server->shared_secret_len)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User_passwd\n");
			goto fail;
		}
	}
	//		
	if (wasd->conf->own_ip_addr.af == AF_INET &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IP_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v4, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IP-Address\n");
		goto fail;
	}

#ifdef ASD_IPV6
	if (wasd->conf->own_ip_addr.af == AF_INET6 &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IPV6_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v6, 16)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IPv6-Address\n");
		goto fail;
	}
#endif /* ASD_IPV6 */

	pthread_mutex_lock(&asd_g_hotspot_mutex);
	if((ASD_HOTSPOT[wasd->hotspot_id]!= NULL)&&(ASD_HOTSPOT[wasd->hotspot_id]->nasid_len != 0))
	{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
						 (u8 *) ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier,
						 os_strlen(ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier))) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
				pthread_mutex_unlock(&asd_g_hotspot_mutex);
				goto fail;
			}
	}
	else{
		if (wasd->conf->nas_identifier &&
		    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
					 (u8 *) wasd->conf->nas_identifier,
					 os_strlen(wasd->conf->nas_identifier))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}
	}	

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT, sta->aid)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port\n");
		pthread_mutex_unlock(&asd_g_hotspot_mutex);
		goto fail;
	}
	if((ASD_HOTSPOT[wasd->hotspot_id]!= NULL)&&(ASD_HOTSPOT[wasd->hotspot_id]->nasid_len != 0))
	{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID,
					(u8 *) ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id, 
					os_strlen(ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}
	}
	
	else if(wasd->nas_port_id[0] != 0){
		//mahz add 2011.5.26
		memset(buf,0,sizeof(buf));
		memcpy(buf,wasd->nas_port_id,os_strlen(wasd->nas_port_id));
		buf[sizeof(buf) - 1] = '\0';
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID, (u8 *) buf, os_strlen(buf))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}
	}
	pthread_mutex_unlock(&asd_g_hotspot_mutex);

	//
	wtpid = (wasd->BSSIndex)/L_BSS_NUM/L_RADIO_NUM;
	if(ASD_WTP_AP[wtpid])
	{		
		os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				MAC2STR(ASD_WTP_AP[wtpid]->WTPMAC), wasd->conf->ssid.ssid);
	}
	else
	{
		os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				    MAC2STR(wasd->own_addr), wasd->conf->ssid.ssid);
	}
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLED_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Called-Station-Id\n");
		goto fail;
	}

	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT,
		    MAC2STR(sta->addr));
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLING_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Calling-Station-Id\n");
		goto fail;
	}

	/* TODO: should probably check MTU from driver config; 2304 is max for
	 * IEEE 802.11, but use 1400 to avoid problems with too large packets
	 */
	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_FRAMED_MTU, 1400)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Framed-MTU\n");
		goto fail;
	}
	if((ASD_SECURITY[SID] != NULL)&&(ASD_SECURITY[SID]->wired_radius == 1)){
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_3)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}else{
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_11)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}

	/*
	  *ht add, 091019
	  */
	//mahz modified 2010.11.26 
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_FRAME_IP_ADDRESS,		
				 (u8 *) &sta->ip_addr, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add FRAME-IP-Address in %s\n",__func__);
		goto fail;
	}
	
	if (sta->flags & WLAN_STA_PREAUTH) {
		os_strlcpy(buf, "IEEE 802.11i Pre-Authentication",
			   sizeof(buf));
	} else {
		os_snprintf(buf, sizeof(buf), "CONNECT %d%sMbps %s",
			    radius_sta_rate(wasd, sta) / 2,
			    (radius_sta_rate(wasd, sta) & 1) ? ".5" : "",
			    radius_mode_txt(wasd));
		buf[sizeof(buf) - 1] = '\0';
	}
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CONNECT_INFO,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Connect-Info\n");
		goto fail;
	}

	if (eap && !radius_msg_add_eap(msg, eap, len)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add EAP-Message\n");
		goto fail;
	}

	//mahz modified 2010.11.27
	/* State attribute must be copied if and only if this packet is
 	* Access-Request reply to the previous Access-Challenge */
 	if(wasd->conf->wapi_radius_auth_enable != 1){
		if ( sm->last_recv_radius && sm->last_recv_radius->hdr->code ==
	    	RADIUS_CODE_ACCESS_CHALLENGE) {
			int res = radius_msg_copy_attr(msg, sm->last_recv_radius,
						       RADIUS_ATTR_STATE);
			if (res < 0) {
				asd_printf(ASD_1X,MSG_ERROR,"Could not copy State attribute from previous "
			    	   "Access-Challenge\n");
				goto fail;
			}
			if (res > 0) {
				asd_printf(ASD_1X,MSG_DEBUG, "Copied RADIUS State Attribute");
			}
		}
		else
		{  
			if(ASD_AUTH_TYPE_EAP(wasd->SecurityID)) /* EAP auth */		 
			{			
				wasd->u.eap_auth.autoauth.auth_req_cnt++;
		
				asd_printf(ASD_1X,MSG_DEBUG, " FUNC %s Securityid %d, type %d auth_req_cnt is %d",
					__func__, wasd->SecurityID, sta->eapol_sm->eap_type_authsrv,
					  wasd->u.eap_auth.autoauth.auth_req_cnt);
			}
		}
 	}
		
	if(is_secondary == 1){//qiuchen add it
		
		radius_msg_free(msg);
		os_free(msg);
		msg = NULL;
		return;
	}
		//radius_client_send(wasd->radius, msg, RADIUS_AUTH, sta->addr);
		radius_client_send(client_info,msg,RADIUS_AUTH,sta);
	return;
 fail:
	radius_msg_free(msg);
	os_free(msg);
	msg = NULL;
}


char *eap_type_text(u8 type)
{
	switch (type) {
	case EAP_TYPE_IDENTITY: return "Identity";
	case EAP_TYPE_NOTIFICATION: return "Notification";
	case EAP_TYPE_NAK: return "Nak";
	case EAP_TYPE_MD5: return "MD5-Challenge";
	case EAP_TYPE_OTP: return "One-Time Password";
	case EAP_TYPE_GTC: return "Generic Token Card";
	case EAP_TYPE_TLS: return "TLS";
	case EAP_TYPE_TTLS: return "TTLS";
	case EAP_TYPE_PEAP: return "PEAP";
	case EAP_TYPE_SIM: return "SIM";
	case EAP_TYPE_FAST: return "FAST";
	case EAP_TYPE_SAKE: return "SAKE";
	case EAP_TYPE_PSK: return "PSK";
	case EAP_TYPE_PAX: return "PAX";
	default: return "Unknown";
	}
}


static void handle_eap_response(struct asd_data *wasd,
				struct sta_info *sta, struct eap_hdr *eap,
				size_t len)
{
	u8 type, *data;
	struct eapol_state_machine *sm = sta->eapol_sm;
	if (sm == NULL)
		return;

	data = (u8 *) (eap + 1);

	if (len < sizeof(*eap) + 1) {
		asd_printf(ASD_1X,MSG_DEBUG,"handle_eap_response: too short response data\n");
		return;
	}

	sm->eap_type_supp = type = data[0];
	circle_cancel_timeout(ieee802_1x_eap_timeout, sta, NULL);

	asd_logger(wasd, sm->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "received EAP packet (code=%d "
		       "id=%d len=%d) from STA: EAP Response-%s (%d)",
		       eap->code, eap->identifier, be_to_host16(eap->length),
		       eap_type_text(type), type);

	sm->dot1xAuthEapolRespFramesRx++;

	wpabuf_free(sm->eap_if->eapRespData);
	sm->eap_if->eapRespData = wpabuf_alloc_copy(eap, len);
	sm->eapolEap = TRUE;
}


/* Process incoming EAP packet from Supplicant */
static void handle_eap(struct asd_data *wasd, struct sta_info *sta,
		       u8 *buf, size_t len)
{
	struct eap_hdr *eap;
	u16 eap_len;

	if (len < sizeof(*eap)) {
		asd_printf(ASD_1X,MSG_DEBUG,"   too short EAP packet\n");
		return;
	}

	eap = (struct eap_hdr *) buf;

	eap_len = be_to_host16(eap->length);
	asd_printf(ASD_1X,MSG_DEBUG, "EAP: code=%d identifier=%d length=%d",
		   eap->code, eap->identifier, eap_len);
	if (eap_len < sizeof(*eap)) {
		asd_printf(ASD_1X,MSG_DEBUG, "   Invalid EAP length");
		return;
	} else if (eap_len > len) {
		asd_printf(ASD_1X,MSG_DEBUG, "   Too short frame to contain this EAP "
			   "packet");
		return;
	} else if (eap_len < len) {
		asd_printf(ASD_1X,MSG_DEBUG, "   Ignoring %lu extra bytes after EAP "
			   "packet", (unsigned long) len - eap_len);
	}

	switch (eap->code) {
	case EAP_CODE_REQUEST:
		asd_printf(ASD_1X,MSG_DEBUG, " (request)");
		return;
	case EAP_CODE_RESPONSE:
		asd_printf(ASD_1X,MSG_DEBUG, " (response)");
		handle_eap_response(wasd, sta, eap, eap_len);
		break;
	case EAP_CODE_SUCCESS:
		asd_printf(ASD_1X,MSG_DEBUG, " (success)");
		return;
	case EAP_CODE_FAILURE:
		asd_printf(ASD_1X,MSG_DEBUG, " (failure)");
		return;
	default:
		asd_printf(ASD_1X,MSG_DEBUG, " (unknown code)");
		return;
	}
}


/* Process the EAPOL frames from the Supplicant */
void ieee802_1x_receive(struct asd_data *wasd, const u8 *sa, const u8 *buf,
			size_t len)
{
	struct sta_info *sta;
	struct ieee802_1x_hdr *hdr;
	struct ieee802_1x_eapol_key *key;
	u16 datalen;
	struct rsn_pmksa_cache_entry *pmksa;
	unsigned char SID = 0;

	if (!wasd->conf->ieee802_1x && !wasd->conf->wpa)
		return;

	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: %lu bytes from " MACSTR,
		   (unsigned long) len, MAC2STR(sa));
	sta = ap_get_sta(wasd, sa);
	if (!sta) {
		asd_printf(ASD_1X,MSG_DEBUG,"   no station information available\n");
		return;
	}

	if (len < sizeof(*hdr)) {
		asd_printf(ASD_1X,MSG_DEBUG,"   too short IEEE 802.1X packet\n");
		return;
	}

	hdr = (struct ieee802_1x_hdr *) buf;
	datalen = be_to_host16(hdr->length);
	asd_printf(ASD_1X,MSG_DEBUG, "   IEEE 802.1X: version=%d type=%d length=%d",
		   hdr->version, hdr->type, datalen);

	if (len - sizeof(*hdr) < datalen) {
		asd_printf(ASD_1X,MSG_DEBUG,"   frame too short for this IEEE 802.1X packet\n");
		if (sta->eapol_sm)
			sta->eapol_sm->dot1xAuthEapLengthErrorFramesRx++;
		return;
	}
	if (len - sizeof(*hdr) > datalen) {
		asd_printf(ASD_1X,MSG_DEBUG, "   ignoring %lu extra octets after "
			   "IEEE 802.1X packet",
			   (unsigned long) len - sizeof(*hdr) - datalen);
	}

	if (sta->eapol_sm) {
		sta->eapol_sm->dot1xAuthLastEapolFrameVersion = hdr->version;
		sta->eapol_sm->dot1xAuthEapolFramesRx++;
	}


	key = (struct ieee802_1x_eapol_key *) (hdr + 1);
	if (datalen >= sizeof(struct ieee802_1x_eapol_key) &&
	    hdr->type == IEEE802_1X_TYPE_EAPOL_KEY &&
	    (key->type == EAPOL_KEY_TYPE_WPA ||
	     key->type == EAPOL_KEY_TYPE_RSN)) {
		wpa_receive(wasd->wpa_auth, sta->wpa_sm, (u8 *) hdr,
			    sizeof(*hdr) + datalen);
		return;
	}


	if (!wasd->conf->ieee802_1x ||
	    wpa_auth_sta_key_mgmt(sta->wpa_sm) == WPA_KEY_MGMT_PSK ||
	    wpa_auth_sta_key_mgmt(sta->wpa_sm) == WPA_KEY_MGMT_FT_PSK)
		return;

	//mahz add 2011.3.16
	if(sta->security_type == HYBRID_AUTH_PORTAL){
		return;
	}

	if (!sta->eapol_sm) {
		sta->eapol_sm = eapol_auth_alloc(wasd->eapol_auth, sta->addr,
						 sta->flags & WLAN_STA_PREAUTH,
						 sta);
		if (!sta->eapol_sm)
			return;
	}


	/* since we support version 1, we can ignore version field and proceed
	 * as specified in version 1 standard [IEEE Std 802.1X-2001, 7.5.5] */
	/* TODO: actually, we are not version 1 anymore.. However, Version 2
	 * does not change frame contents, so should be ok to process frames
	 * more or less identically. Some changes might be needed for
	 * verification of fields. */

	switch (hdr->type) {
	case IEEE802_1X_TYPE_EAP_PACKET:
		asd_printf(ASD_1X,MSG_DEBUG,"IEEE802_1X_TYPE_EAP_PACKET\n");
		if((sta->alive_flag)&&(sta->flags & WLAN_STA_AUTHORIZED)){
			sta->alive_total = 0;
		}

		else{
			handle_eap(wasd, sta, (u8 *) (hdr + 1), datalen);
		}	
		break;

	case IEEE802_1X_TYPE_EAPOL_START:
		
		asd_printf(ASD_1X,MSG_DEBUG,"IEEE802_1X_TYPE_EAPOL_START\n");
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG, "received EAPOL-Start "
			       "from STA");
		if((sta->alive_flag)&&(sta->flags & WLAN_STA_AUTHORIZED)){
			sta->alive_total = 0;
			break;
		}
		sta->eapol_sm->flags &= ~EAPOL_SM_WAIT_START;
		pmksa = wpa_auth_sta_get_pmksa(sta->wpa_sm);
		if (pmksa) {
			asd_logger(wasd, sta->addr, asd_MODULE_WPA,
				       asd_LEVEL_DEBUG, "cached PMKSA "
				       "available - ignore it since "
				       "STA sent EAPOL-Start");
			wpa_auth_sta_clear_pmksa(sta->wpa_sm, pmksa);
		}
		sta->eapol_sm->eapolStart = TRUE;
		sta->eapol_sm->dot1xAuthEapolStartFramesRx++;
		wpa_auth_sm_event(sta->wpa_sm, WPA_REAUTH_EAPOL);

		//mahz  add 2011.3.28
		 if(ASD_WLAN[wasd->WlanID])
			 SID = ASD_WLAN[wasd->WlanID]->SecurityID;
		if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1))
			wasd->info->identify_request++;	
		break;

	case IEEE802_1X_TYPE_EAPOL_LOGOFF:
		
		asd_printf(ASD_1X,MSG_DEBUG,"IEEE802_1X_TYPE_EAPOL_LOGOFF\n");
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG, "received EAPOL-Logoff "
			       "from STA");
		sta->acct_terminate_cause =
			RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		sta->eapol_sm->eapolLogoff = TRUE;
		sta->eapol_sm->dot1xAuthEapolLogoffFramesRx++;
		if((sta->ipaddr  != 0)&&(HYBRID_AUTH_EAPOL_SUCCESS(sta)))
		{
			if(asd_ipset_switch)
				eap_connect_down(sta->ipaddr);
			else
				AsdStaInfoToEAG(wasd,sta,ASD_DEL_AUTH);
		
		}
		// AC kick the sta 
		
		sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		unsigned char SID = (unsigned char)wasd->SecurityID;
		if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
			wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
			mlme_deauthenticate_indication(
			wasd, sta, 0);
			ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		}
		ieee802_11_send_deauth(wasd, sta->addr, 3);
		if(ASD_NOTICE_STA_INFO_TO_PORTAL)
			AsdStaInfoToEAG(wasd,sta,WID_DEL);
		AsdStaInfoToWID(wasd,sta->addr,WID_DEL);
		if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
			ap_free_sta(wasd, sta, 1);
		}
		else{				
			ap_free_sta(wasd, sta, 0);
		}
		return;
		//break;

	case IEEE802_1X_TYPE_EAPOL_KEY:
		
		asd_printf(ASD_1X,MSG_DEBUG,"IEEE802_1X_TYPE_EAPOL_KEY\n");
		asd_printf(ASD_1X,MSG_DEBUG, "   EAPOL-Key");
		if (!(sta->flags & WLAN_STA_AUTHORIZED)) {
			asd_printf(ASD_1X,MSG_DEBUG, "   Dropped key data from "
				   "unauthorized Supplicant");
			break;
		}
		break;

	case IEEE802_1X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT:
		
		asd_printf(ASD_1X,MSG_DEBUG,"IEEE802_1X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT\n");
		asd_printf(ASD_1X,MSG_DEBUG, "   EAPOL-Encapsulated-ASF-Alert");
		/* TODO: implement support for this; show data */
		break;

	default:
		
		asd_printf(ASD_1X,MSG_DEBUG,"default\n");
		asd_printf(ASD_1X,MSG_DEBUG, "   unknown IEEE 802.1X packet type");
		sta->eapol_sm->dot1xAuthInvalidEapolFramesRx++;
		break;
	}


	eapol_auth_step(sta->eapol_sm);
}


void ieee802_1x_new_station(struct asd_data *wasd, struct sta_info *sta)
{
	struct rsn_pmksa_cache_entry *pmksa;
	int reassoc = 1;
	int force_1x = 0;

	if ((!force_1x && !wasd->conf->ieee802_1x) ||
	    wpa_auth_sta_key_mgmt(sta->wpa_sm) == WPA_KEY_MGMT_PSK ||
	    wpa_auth_sta_key_mgmt(sta->wpa_sm) == WPA_KEY_MGMT_FT_PSK)
		return;

	if (sta->eapol_sm == NULL) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG, "start authentication");
		sta->eapol_sm = eapol_auth_alloc(wasd->eapol_auth, sta->addr,
						 sta->flags & WLAN_STA_PREAUTH,
						 sta);
		if (sta->eapol_sm == NULL) {
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE8021X,
				       asd_LEVEL_INFO,
				       "failed to allocate state machine");
			return;
		}
		reassoc = 0;
	}
	else
	{
		if((sta != NULL)&&(sta->eapol_sm != NULL))
		{	
			if(wasd&&(ASD_SECURITY[wasd->SecurityID] != NULL))
				ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
		}
	}
	sta->eapol_sm->eap_if->portEnabled = TRUE;
	sta->alive_flag = 0;							//weichao add 2011.09.23
	sta->alive_total = 0;							//weichao add 2011.09.23

	pmksa = wpa_auth_sta_get_pmksa(sta->wpa_sm);

	//mahz add 2011.7.11
	unsigned char SID = 0;
	if(ASD_WLAN[wasd->WlanID])
		SID = ASD_WLAN[wasd->WlanID]->SecurityID;
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->fast_auth == 0)){
		pmksa = NULL;
	}
	//
	if (pmksa) {
		int old_vlanid;

		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG,
			       "PMK from PMKSA cache - skip IEEE 802.1X/EAP");
		/* Setup EAPOL state machines to already authenticated state
		 * because of existing PMKSA information in the cache. */
		sta->eapol_sm->keyRun = TRUE;
		sta->eapol_sm->eap_if->eapKeyAvailable = TRUE;
		sta->eapol_sm->auth_pae_state = AUTH_PAE_AUTHENTICATING;
		sta->eapol_sm->be_auth_state = BE_AUTH_SUCCESS;
		sta->eapol_sm->authSuccess = TRUE;
		if((wasd->conf->radius != NULL) && (wasd->conf->radius->radius_extend_attr != 0)){
			sta->eapol_sm->reAuthWhen = 3;
			//sta->eapol_sm->reAuthenticate = TRUE;
			//eapol_auth_step(sta->eapol_sm);
		}	/*ht add 091027*/
		
		if (sta->eapol_sm->eap)
			eap_sm_notify_cached(sta->eapol_sm->eap);
		old_vlanid = sta->vlan_id;
		pmksa_cache_to_eapol_data(pmksa, sta->eapol_sm);
		if (sta->ssid->dynamic_vlan == DYNAMIC_VLAN_DISABLED)
			sta->vlan_id = 0;
		ap_sta_bind_vlan(wasd, sta, old_vlanid);
	} else {
		if (reassoc) {
			/*
			 * Force EAPOL state machines to start
			 * re-authentication without having to wait for the
			 * Supplicant to send EAPOL-Start.
			 */
			sta->eapol_sm->reAuthenticate = TRUE;
		}
		eapol_auth_step(sta->eapol_sm);
	}
}


void ieee802_1x_free_radius_class(struct radius_class_data *class)
{
	size_t i;
	if (class == NULL)
		return;
	for (i = 0; i < class->count; i++)
		os_free(class->attr[i].data);
	os_free(class->attr);
	class->attr = NULL;
	class->count = 0;
}


int ieee802_1x_copy_radius_class(struct radius_class_data *dst,
				 struct radius_class_data *src)
{
	size_t i;

	if (src->attr == NULL)
		return 0;

	dst->attr = os_zalloc(src->count * sizeof(struct radius_attr_data));
	if (dst->attr == NULL)
		return -1;

	dst->count = 0;

	for (i = 0; i < src->count; i++) {
		dst->attr[i].data = os_zalloc(src->attr[i].len);
		if (dst->attr[i].data == NULL)
			break;
		dst->count++;
		os_memcpy(dst->attr[i].data, src->attr[i].data,
			  src->attr[i].len);
		dst->attr[i].len = src->attr[i].len;
	}

	return 0;
}


void ieee802_1x_free_station(struct sta_info *sta)
{
	struct eapol_state_machine *sm = sta->eapol_sm;

	circle_cancel_timeout(ieee802_1x_eap_timeout, sta, NULL);

	if (sm == NULL)
		return;
	if(sm->wasd)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"free stationg sta->wlanid :%d,radius_id :%d\n",sm->wasd->WlanID,sm->radius_identifier);
		radius_client_info_free(radius_client_get_sta_info(sta,sm->wasd->WlanID,RADIUS_AUTH));
		radius_client_info_free(radius_client_get_sta_info(sta,sm->wasd->WlanID,RADIUS_ACCT));		
	}
	sta->eapol_sm = NULL;

	if (sm->last_recv_radius) {
		radius_msg_free(sm->last_recv_radius);
		os_free(sm->last_recv_radius);
		sm->last_recv_radius = NULL;
	}

	os_free(sm->identity);
	ieee802_1x_free_radius_class(&sm->radius_class);
	eapol_auth_free(sm);
}


static void ieee802_1x_decapsulate_radius(struct asd_data *wasd,
					  struct sta_info *sta)
{
	u8 *eap;
	size_t len;
	struct eap_hdr *hdr;
	int eap_type = -1;
	char buf[64];
	struct radius_msg *msg;
	struct eapol_state_machine *sm = sta->eapol_sm;

	if (sm == NULL || sm->last_recv_radius == NULL) {
		if (sm)
			sm->eap_if->aaaEapNoReq = TRUE;
		return;
	}

	msg = sm->last_recv_radius;

	eap = radius_msg_get_eap(msg, &len);
	if (eap == NULL) {
		/* RFC 3579, Chap. 2.6.3:
		 * RADIUS server SHOULD NOT send Access-Reject/no EAP-Message
		 * attribute */
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_WARNING, "could not extract "
			       "EAP-Message from RADIUS message");
		sm->eap_if->aaaEapNoReq = TRUE;
		return;
	}

	if (len < sizeof(*hdr)) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_WARNING, "too short EAP packet "
			       "received from authentication server");
		os_free(eap);
		sm->eap_if->aaaEapNoReq = TRUE;
		return;
	}

	if (len > sizeof(*hdr))
		eap_type = eap[sizeof(*hdr)];

	hdr = (struct eap_hdr *) eap;
	switch (hdr->code) {
	case EAP_CODE_REQUEST:
		if (eap_type >= 0)
			sm->eap_type_authsrv = eap_type;
		os_snprintf(buf, sizeof(buf), "EAP-Request-%s (%d)",
			    eap_type >= 0 ? eap_type_text(eap_type) : "??",
			    eap_type);
		break;
	case EAP_CODE_RESPONSE:
		os_snprintf(buf, sizeof(buf), "EAP Response-%s (%d)",
			    eap_type >= 0 ? eap_type_text(eap_type) : "??",
			    eap_type);
		break;
	case EAP_CODE_SUCCESS:
		os_strlcpy(buf, "EAP Success", sizeof(buf));
		break;
	case EAP_CODE_FAILURE:
		os_strlcpy(buf, "EAP Failure", sizeof(buf));
		break;
	default:
		os_strlcpy(buf, "unknown EAP code", sizeof(buf));
		break;
	}
	buf[sizeof(buf) - 1] = '\0';
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "decapsulated EAP packet (code=%d "
		       "id=%d len=%d) from RADIUS server: %s",
		       hdr->code, hdr->identifier, be_to_host16(hdr->length),
		       buf);
	sm->eap_if->aaaEapReq = TRUE;

	wpabuf_free(sm->eap_if->aaaEapReqData);
	sm->eap_if->aaaEapReqData = wpabuf_alloc_ext_data(eap, len);
}

//weichao add
void eapol_keep_alive_timer(struct eapol_state_machine *sm,int *alive_time)
{
	if (sm == NULL)
		return;
	eapol_alive_step(sm,alive_time);
}

static void ieee802_1x_get_keys(struct asd_data *wasd,
				struct sta_info *sta, struct radius_msg *msg,
				struct radius_msg *req,
				u8 *shared_secret, size_t shared_secret_len)
{
	struct radius_ms_mppe_keys *keys;
	struct eapol_state_machine *sm = sta->eapol_sm;
	if (sm == NULL)
		return;

	keys = radius_msg_get_ms_keys(msg, req, shared_secret,
				      shared_secret_len);

	if (keys && keys->send && keys->recv) {
		size_t len = keys->send_len + keys->recv_len;
		wpa_hexdump_key(MSG_DEBUG, "MS-MPPE-Send-Key",
				keys->send, keys->send_len);
		wpa_hexdump_key(MSG_DEBUG, "MS-MPPE-Recv-Key",
				keys->recv, keys->recv_len);

		os_free(sm->eap_if->aaaEapKeyData);
		sm->eap_if->aaaEapKeyData = os_zalloc(len);
		if (sm->eap_if->aaaEapKeyData) {
			os_memcpy(sm->eap_if->aaaEapKeyData, keys->recv,
				  keys->recv_len);
			os_memcpy(sm->eap_if->aaaEapKeyData + keys->recv_len,
				  keys->send, keys->send_len);
			sm->eap_if->aaaEapKeyDataLen = len;
			sm->eap_if->aaaEapKeyAvailable = TRUE;
		}
	}

	if (keys) {
		os_free(keys->send);
		os_free(keys->recv);
		os_free(keys);
	}
}

void ieee802_1x_update_traffic_limit(struct asd_data *wasd,
	struct sta_info *sta,struct radius_msg *msg)
{
	u32 traffic_limit = 0, send_traffic_limit = 0;
	u32 traffic_limit_n = 0, send_traffic_limit_n = 0;
	unsigned char *up_limit = NULL, *down_limit = NULL;
	size_t up_limit_len, down_limit_len;

	asd_printf(ASD_1X,MSG_DEBUG,"%s\n",__func__);
	down_limit = radius_msg_get_vendor_attr(msg, RADIUS_VENDOR_ID_RUIJIE,
					 RADIUS_VENDOR_ATTR_RJ_DOWNLINK_TRAFFIC_LIMIT,
					 &down_limit_len);

	up_limit = radius_msg_get_vendor_attr(msg, RADIUS_VENDOR_ID_RUIJIE,
					 RADIUS_VENDOR_ATTR_RJ_UPLINK_TRAFFIC_LIMIT,
					 &up_limit_len);
	
	if(down_limit != NULL) {
		os_memcpy(&send_traffic_limit_n, down_limit, 4);
		send_traffic_limit = ntohl(send_traffic_limit_n);
		os_free(down_limit);
		down_limit = NULL;
	}

	if(up_limit != NULL) {
		os_memcpy(&traffic_limit_n, up_limit, 4);
		traffic_limit = ntohl(traffic_limit_n);
		os_free(up_limit);
		up_limit = NULL;
	}
	
	asd_printf(ASD_1X,MSG_DEBUG,"Sta "MACSTR" traffic_limit that radius server set is %d\n",MAC2STR(sta->addr),traffic_limit);
	asd_printf(ASD_1X,MSG_DEBUG,"Sta "MACSTR" send_traffic_limit that radius server set is %d\n",MAC2STR(sta->addr),send_traffic_limit);

	if(wasd->traffic_limit!=0 && traffic_limit>wasd->traffic_limit){
		asd_printf(ASD_1X,MSG_DEBUG,"Traffic_limit received from radius server is bigger than bss traffic_limit\n");
	}else{
		sta->vip_flag |= 0x01;	
		sta->sta_traffic_limit=traffic_limit;
	}

	if(wasd->send_traffic_limit!=0 && send_traffic_limit>wasd->send_traffic_limit){
		asd_printf(ASD_1X,MSG_DEBUG,"Send_traffic_limit received from radius server is bigger than bss send_traffic_limit\n");
	}else{
		sta->vip_flag |= 0x02;	
		sta->sta_send_traffic_limit=send_traffic_limit;

	}
	
}

static void ieee802_1x_store_radius_class(struct asd_data *wasd,
					  struct sta_info *sta,
					  struct radius_msg *msg)
{
	u8 *class;
	size_t class_len;
	struct eapol_state_machine *sm = sta->eapol_sm;
	int count, i;
	struct radius_attr_data *nclass;
	size_t nclass_count;

	if (!wasd->conf->radius->acct_server || wasd->radius == NULL ||
	    sm == NULL)
		return;

	ieee802_1x_free_radius_class(&sm->radius_class);
	count = radius_msg_count_attr(msg, RADIUS_ATTR_CLASS, 1);
	if (count <= 0)
		return;

	nclass = os_zalloc(count * sizeof(struct radius_attr_data));
	if (nclass == NULL)
		return;

	nclass_count = 0;

	class = NULL;
	for (i = 0; i < count; i++) {
		do {
			if (radius_msg_get_attr_ptr(msg, RADIUS_ATTR_CLASS,
						    &class, &class_len,
						    class) < 0) {
				i = count;
				break;
			}
		} while (class_len < 1);

		nclass[nclass_count].data = os_zalloc(class_len);
		if (nclass[nclass_count].data == NULL)
			break;

		os_memcpy(nclass[nclass_count].data, class, class_len);
		nclass[nclass_count].len = class_len;
		nclass_count++;
	}

	sm->radius_class.attr = nclass;
	sm->radius_class.count = nclass_count;
	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Stored %lu RADIUS Class "
		   "attributes for " MACSTR,
		   (unsigned long) sm->radius_class.count,
		   MAC2STR(sta->addr));
	//weichao add
	unsigned char SID = wasd->SecurityID;
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->traffic_limite_radius)&&(sm->radius_class.attr[0].len == 32))
	{
		unsigned char *class_data = NULL;
		char traffic[9] = {0};
		char send_traffic[9] = {0};
		asd_printf(ASD_1X,MSG_DEBUG,"size of class is %d\n",nclass_count);
		class_data = sm->radius_class.attr[0].data;
		asd_printf(ASD_1X,MSG_DEBUG,"class_data = %s\n",class_data);
		asd_printf(ASD_1X,MSG_DEBUG,"strlen = %d\n",strlen((char *)class_data));
		class_data = class_data+8;
		memcpy(traffic,class_data,8);
		asd_printf(ASD_1X,MSG_DEBUG,"traffic = %s\n",traffic);
		class_data = class_data+16;
		memcpy(send_traffic,class_data,8);
		asd_printf(ASD_1X,MSG_DEBUG,"send_traffic = %s\n",send_traffic);
		sta->sta_traffic_limit = (unsigned int )strtol(traffic,NULL,10);
		sta->sta_send_traffic_limit = (unsigned int)strtol(send_traffic,NULL,10);
		asd_printf(ASD_1X,MSG_DEBUG,"sta->sta_traffic_limit = %d\n",sta->sta_traffic_limit);
		asd_printf(ASD_1X,MSG_DEBUG,"sta->sta_send_traffic_limit = %d\n",sta->sta_send_traffic_limit);
		AsdStaInfoToWID(wasd, sta->addr, RADIUS_STA_UPDATE);
	}
}


/* Update sta->identity based on User-Name attribute in Access-Accept */
static void ieee802_1x_update_sta_identity(struct asd_data *wasd,
					   struct sta_info *sta,
					   struct radius_msg *msg)
{
	u8 *buf, *identity;
	size_t len;
	struct eapol_state_machine *sm = sta->eapol_sm;

	if (sm == NULL)
		return;

	if (radius_msg_get_attr_ptr(msg, RADIUS_ATTR_USER_NAME, &buf, &len,
				    NULL) < 0)
		return;

	identity = os_zalloc(len + 1);
	if (identity == NULL)
		return;

	os_memcpy(identity, buf, len);
	identity[len] = '\0';

#if 0
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "old identity '%s' updated with "
		       "User-Name from Access-Accept '%s'",
		       sm->identity ? (char *) sm->identity : "N/A",
		       (char *) identity);
#endif
	os_free(sm->identity);
	sm->identity = identity;
	sm->identity_len = len;
}


struct sta_id_search {
	u8 identifier;
	struct eapol_state_machine *sm;
};

/*
static int ieee802_1x_select_radius_identifier(struct asd_data *wasd,
					       struct sta_info *sta,
					       void *ctx)
{
	struct sta_id_search *id_search = ctx;
	struct eapol_state_machine *sm = sta->eapol_sm;

	if (sm && sm->radius_identifier >= 0 &&
	    sm->radius_identifier == id_search->identifier) {
		id_search->sm = sm;
		return 1;
	}
	return 0;
}


static struct eapol_state_machine *
ieee802_1x_search_radius_identifier(struct asd_data *wasd, u8 identifier)
{
	struct sta_id_search id_search;
	id_search.identifier = identifier;
	id_search.sm = NULL;
	ap_for_each_sta(wasd, ieee802_1x_select_radius_identifier, &id_search);
	return id_search.sm;
}
*/

/* Process the RADIUS frames from Authentication Server */
static RadiusRxResult
ieee802_1x_receive_auth(struct radius_msg *msg, struct radius_msg *req,
			u8 *shared_secret, size_t shared_secret_len,
			void *data)
{
	struct asd_data *wasd = NULL;
	struct radius_client_info *auth = data;
	struct radius_send_info *send_info = NULL;
	struct sta_info *sta = NULL;
	int wlanid = 0;
	u32 session_timeout = 0, termination_action, acct_interim_interval;
	int session_timeout_set, old_vlanid = 0;
	int eap_timeout;
	struct eapol_state_machine *sm;
	int override_eapReq = 0;
	u8 WTPMAC[MAC_LEN] = {0};

	//sm = ieee802_1x_search_radius_identifier(wasd, msg->hdr->identifier);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func: %s  :client_info sock :%d ;radius_id : %d\n",__func__,auth->sock,msg->hdr->identifier);
	send_info  = auth->sta[msg->hdr->identifier];
	if(send_info == NULL)
	{
		asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Could not find matching "
			   "station for this RADIUS message");
		return RADIUS_RX_UNKNOWN;
	}
	sta = send_info->sta;
	if(sta == NULL)
	{
		asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Could not find matching "
			   "station for this RADIUS message");
		return RADIUS_RX_UNKNOWN;

	}
	sm = sta->eapol_sm;
	if (sm == NULL) {
		asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: Could not find matching "
			   "station for this RADIUS message");
		return RADIUS_RX_UNKNOWN;
	}
	wasd = sm->wasd;
	if(wasd == NULL)
	{
		asd_printf(ASD_1X,MSG_INFO,"sta list is not sync in radius and wasd!\n");
		return RADIUS_RX_UNKNOWN;
	}
	wlanid = wasd->WlanID;
	if(ASD_WTP_AP[wasd->Radio_G_ID/4])
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
	//qiuchen
	u8 *identity = NULL;
	unsigned char SID = wasd->SecurityID;
	unsigned int securitytype = 0;
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;
	char *SSID = NULL;
	if(ASD_WLAN[wlanid])
		SSID = ASD_WLAN[wlanid]->ESSID;
	//end
	//mahz modified 2010.12.8
	if((wasd->conf) && (wasd->conf->wapi_radius_auth_enable != 1)){

		/* RFC 2869, Ch. 5.13: valid Message-Authenticator attribute MUST be
		 * present when packet contains an EAP-Message attribute */
		if (msg->hdr->code == RADIUS_CODE_ACCESS_REJECT &&
		    radius_msg_get_attr(msg, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, NULL,
					0) < 0 &&
		    radius_msg_get_attr(msg, RADIUS_ATTR_EAP_MESSAGE, NULL, 0) < 0) {
			asd_printf(ASD_1X,MSG_DEBUG, "Allowing RADIUS Access-Reject without "
				   "Message-Authenticator since it does not include "
				   "EAP-Message");
		} else if (radius_msg_verify(msg, shared_secret, shared_secret_len,
					     req, 1)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Incoming RADIUS packet did not have correct "
			       "Message-Authenticator - dropped\n");
			if(gASDLOGDEBUG & BIT(1))
				syslog(LOG_INFO|LOG_LOCAL7, "AUTHFAILED:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d,SecurityType:%d,ErrorCode:%d.\n",
					MAC2STR(sta->addr),MAC2STR(WTPMAC),wasd->BSSIndex,securitytype,RADIUS_FAILED);//qiuchen 2013.01.14
			return RADIUS_RX_INVALID_AUTHENTICATOR;
		}
	}
	//
	if (msg->hdr->code != RADIUS_CODE_ACCESS_ACCEPT &&
	    msg->hdr->code != RADIUS_CODE_ACCESS_REJECT &&
	    msg->hdr->code != RADIUS_CODE_ACCESS_CHALLENGE) {
		asd_printf(ASD_1X,MSG_DEBUG,"Unknown RADIUS message code\n");
		return RADIUS_RX_UNKNOWN;
	}
	//sm->radius_identifier = -1;
	asd_printf(ASD_1X,MSG_DEBUG, "RADIUS packet matching with station " MACSTR,
		   MAC2STR(sta->addr));

	if (sm->last_recv_radius) {
		radius_msg_free(sm->last_recv_radius);
		os_free(sm->last_recv_radius);
		sm->last_recv_radius = NULL;
	}

	sm->last_recv_radius = msg;

	session_timeout_set =
		!radius_msg_get_attr_int32(msg, RADIUS_ATTR_SESSION_TIMEOUT,
					   &session_timeout);
	if (radius_msg_get_attr_int32(msg, RADIUS_ATTR_TERMINATION_ACTION,
				      &termination_action))
		termination_action = RADIUS_TERMINATION_ACTION_DEFAULT;

	if ((ASD_WLAN[wlanid])&&(ASD_WLAN[wlanid]->radius_server)&&(ASD_WLAN[wlanid]->radius_server->acct_interim_interval == 0) &&
	    (msg->hdr->code == RADIUS_CODE_ACCESS_ACCEPT) &&
	    (radius_msg_get_attr_int32(msg, RADIUS_ATTR_ACCT_INTERIM_INTERVAL,
				      &acct_interim_interval) == 0)) {
		if (acct_interim_interval < 60) {
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE8021X,
				       asd_LEVEL_INFO,
				       "ignored too small "
				       "Acct-Interim-Interval %d",
				       acct_interim_interval);
		} else
			sta->acct_interim_interval = acct_interim_interval;
	}


	switch (msg->hdr->code) {
	case RADIUS_CODE_ACCESS_ACCEPT:
		if(gASDLOGDEBUG & BIT(1)){
			if(sta->rflag && !(sta->logflag&BIT(1)) && sta->flags & WLAN_STA_AUTHORIZED){
				syslog(LOG_INFO|LOG_LOCAL3,"STA_ROAM_SUCCESS:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
					MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					sta->preAPID,MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
				);
				sta->logflag = BIT(1);
			}
			else{
				time_t at = time(NULL);
				if(sta->eapol_sm)
					identity = sta->eapol_sm->identity;
				syslog(LOG_INFO|LOG_LOCAL3, "STA_8021X_SUCCESS:UserMAC:" MACSTR "BSSID:"MACSTR" AP%d:" MACSTR "(L_R:%d) Wlan%d:%s(%d) Username:%s TIME %s.\n",
					MAC2STR(sta->addr),MAC2STR(wasd->own_addr),wasd->Radio_G_ID/4,MAC2STR(WTPMAC),wasd->Radio_G_ID%4,wasd->WlanID,SSID,securitytype,identity,ctime(&at));//qiuchen 2013.01.14
			}
		}
		//qiuchen add it for Henan Mobile
		if(gASDLOGDEBUG & BIT(0)){
			if(sta->rflag && !(sta->logflag&BIT(0))){
				asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
									((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
									((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
				sta->logflag = BIT(0);
			}
			else
				asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGIN_SUCC:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s; The user passed 802.1X authentication and got online successfully\n",wasd->Radio_G_ID,wasd->BSSIndex%128,MAC2STR(sta->addr),sta->vlan_id,sta->eapol_sm->identity);
		}
		//end
		//qiuchen add it for Guang Zhou mobile to count the number of radius response (SIM/PEAP)
		if (ASD_AUTH_TYPE_EAP((unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID) && ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
			wasd->u.eap_auth.autoauth.auth_resp_cnt++;
		//end
		if((ASD_WLAN[wlanid])&&(ASD_WLAN[wlanid]->radius_server)&& (ASD_WLAN[wlanid]->radius_server->radius_extend_attr)){
			unsigned char *buf = NULL;
			u32 vlan_id_n = 0;
			size_t 	buf_len;		
			ieee802_1x_update_traffic_limit(wasd, sta, msg);
			buf = radius_msg_get_vendor_attr(msg, RADIUS_VENDOR_ID_RUIJIE,
							 RADIUS_VENDOR_ATTR_RJ_USER_VLAN_ID,&buf_len);
			if(buf != NULL){
				os_memcpy(&vlan_id_n, buf, 4);
				sta->vlan_id = ntohl(vlan_id_n);
				os_free(buf);
				buf = NULL;
			}
			asd_printf(ASD_1X,MSG_DEBUG,"Sta "MACSTR" vlan_id is %d\n",MAC2STR(sta->addr),sta->vlan_id);
		}else{
			if (sta->ssid->dynamic_vlan == DYNAMIC_VLAN_DISABLED)
				sta->vlan_id = 0;
			else {
				old_vlanid = sta->vlan_id;
				sta->vlan_id = radius_msg_get_vlanid(msg);
			}
			if (sta->vlan_id > 0 &&
			    asd_get_vlan_id_ifname(wasd->conf->vlan,
						       sta->vlan_id)) {
				asd_logger(wasd, sta->addr,
					       asd_MODULE_RADIUS,
					       asd_LEVEL_INFO,
					       "VLAN ID %d", sta->vlan_id);
			} else if (sta->ssid->dynamic_vlan == DYNAMIC_VLAN_REQUIRED) {
				sta->eapol_sm->authFail = TRUE;
				asd_logger(wasd, sta->addr,
					       asd_MODULE_IEEE8021X,
					       asd_LEVEL_INFO, "authentication "
					       "server did not include required VLAN "
					       "ID in Access-Accept");
				break;
			}

			ap_sta_bind_vlan(wasd, sta, old_vlanid);
		}
		
		/* RFC 3580, Ch. 3.17 */
		if (session_timeout_set && termination_action ==
		    RADIUS_TERMINATION_ACTION_RADIUS_REQUEST) {
			sm->reAuthPeriod = session_timeout;
		} else if (session_timeout_set)
			ap_sta_session_timeout(wasd, sta, session_timeout);

		sm->eap_if->aaaSuccess = TRUE;
		override_eapReq = 1;
		ieee802_1x_get_keys(wasd, sta, msg, req, shared_secret,
				    shared_secret_len);
		ieee802_1x_store_radius_class(wasd, sta, msg);
		ieee802_1x_update_sta_identity(wasd, sta, msg);
		if (sm->eap_if->eapKeyAvailable &&
		    wpa_auth_pmksa_add(sta->wpa_sm, sm->eapol_key_crypt,
				       session_timeout_set ?
				       (int) session_timeout : -1, sm) == 0) {
			unsigned char WLANID = wasd->WlanID;
			struct PMK_STAINFO *pmk_sta;
			if(ASD_WLAN[WLANID] != NULL){
				pmk_sta = pmk_ap_sta_add(ASD_WLAN[WLANID],sta->addr);
				pmk_sta->idhi = sta->acct_session_id_hi;
				pmk_sta->idlo = sta->acct_session_id_lo;
				pmk_add_bssindex(wasd->BSSIndex,pmk_sta);
			}
			asd_logger(wasd, sta->addr, asd_MODULE_WPA,
				       asd_LEVEL_DEBUG,
				       "Added PMKSA cache entry");
		}

		//mahz add 2010.11.26
		if(wasd && sta && wasd->conf->wapi_radius_auth_enable){
			accounting_sta_stop(wasd, sta);
			accounting_sta_start( wasd, sta);
			asd_printf(ASD_1X,MSG_DEBUG,"%s : radius start acct!\n",__func__);
		}
		
		if (ASD_AUTH_TYPE_EAP(wasd->SecurityID))	/* EAP auth */
		{			
			if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
			{	
				
				wasd->u.eap_auth.autoauth.auth_suc_cnt++;
			}
			asd_printf(ASD_1X,MSG_DEBUG, "%s ,type is %d,auth_suc_cnt is %d\n",__func__, sta->eapol_sm->eap_type_authsrv,wasd->u.eap_auth.autoauth.auth_suc_cnt);	  
		}
		break;
	case RADIUS_CODE_ACCESS_REJECT:
		if(gASDLOGDEBUG & BIT(1)){
			if(sta->rflag && !(sta->logflag&BIT(1)) && sta->flags & WLAN_STA_AUTHORIZED){
				syslog(LOG_INFO|LOG_LOCAL3,"STA_ROAM_FAILED:UserMAC:"MACSTR" From AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR") To AC(%lu.%lu.%lu.%lu)-AP%d-BSSID("MACSTR").\n",
					MAC2STR(sta->addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					sta->preAPID,MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),
					wasd->Radio_G_ID/4,MAC2STR(wasd->own_addr)
				);
				sta->logflag = BIT(1);
			}
			else{
				if(sta->eapol_sm)
					identity = sta->eapol_sm->identity;
				syslog(LOG_INFO|LOG_LOCAL3, "STA_8021X_FAILED:UserMAC:" MACSTR "BSSID:"MACSTR" AP%d:" MACSTR "(L_R:%d) Wlan%d:%s(%d) Username:%s.\n",
					MAC2STR(sta->addr),MAC2STR(wasd->own_addr),wasd->Radio_G_ID/4,MAC2STR(WTPMAC),wasd->Radio_G_ID%4,wasd->WlanID,SSID,securitytype,identity);//qiuchen 2013.01.14
			}
		}
		if(gASDLOGDEBUG & BIT(0)){
			if(sta->rflag)
				asd_syslog_h(LOG_WARNING,"WSTA","WROAM_ROAM_IN_FAILED:Client "MAC_ADDRESS",Failed to roam: Maximum roam-in clients reached.\n",MAC2STR(sta->addr));
			else
				asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGIN_FAILURE:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s; The user failed the 802.1X authentication.\n",wasd->Radio_G_ID,wasd->BSSIndex%128,MAC2STR(sta->addr),sta->vlan_id,NULL);
		}
		//qiuchen add it for Guang Zhou mobile to count the number of radius response (SIM/PEAP)
		if (ASD_AUTH_TYPE_EAP((unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID) && ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
			wasd->u.eap_auth.autoauth.auth_resp_cnt++;
		//end
		sm->eap_if->aaaFail = TRUE;
		override_eapReq = 1;
		//qiuchen
		if (ASD_AUTH_TYPE_EAP(wasd->SecurityID))	/* EAP auth */
		{
			if (ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
			{
				wasd->u.eap_auth.autoauth.auth_fail_cnt++;
				
			asd_printf(ASD_1X,MSG_DEBUG, " auth_fail_cnt is %d\n",wasd->u.eap_auth.autoauth.auth_fail_cnt);  
			}
		}
		break;
	case RADIUS_CODE_ACCESS_CHALLENGE:
		//qiuchen add it for Guang Zhou mobile to count the number of radius response (SIM/PEAP)
		if (ASD_AUTH_TYPE_EAP((unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID) && ASD_EAP_TYPE_SIM_PEAP(sta->eapol_sm))
			wasd->u.eap_auth.autoauth.auth_resp_cnt++;
		//end
		sm->eap_if->aaaEapReq = TRUE;
		if (session_timeout_set) {
			/* RFC 2869, Ch. 2.3.2; RFC 3580, Ch. 3.17 */
			eap_timeout = session_timeout;
		} else
			eap_timeout = 30;
		asd_logger(wasd, sm->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG,
			       "using EAP timeout of %d seconds%s",
			       eap_timeout,
			       session_timeout_set ? " (from RADIUS)" : "");
		circle_cancel_timeout(ieee802_1x_eap_timeout, sta, NULL);
		circle_register_timeout(eap_timeout, 0, ieee802_1x_eap_timeout,
				       sta, NULL);
		sm->eap_if->eapTimeout = FALSE;
		break;
	}

	//mahz modified 2010.12.1
	if((wasd->conf) && (wasd->conf->wapi_radius_auth_enable != 1)){

		ieee802_1x_decapsulate_radius(wasd, sta);
		if (override_eapReq)
			sm->eap_if->aaaEapReq = FALSE;

		eapol_auth_step(sm);
	}
	//
	return RADIUS_RX_QUEUED;
}


void ieee802_1x_abort_auth(struct asd_data *wasd, struct sta_info *sta)
{
	struct eapol_state_machine *sm = sta->eapol_sm;
	if (sm == NULL)
		return;

	asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
		       asd_LEVEL_DEBUG, "aborting authentication");

	if (sm->last_recv_radius) {
		radius_msg_free(sm->last_recv_radius);
		os_free(sm->last_recv_radius);
		sm->last_recv_radius = NULL;
	}
}


#ifdef asd_DUMP_STATE
static void fprint_char(FILE *f, char c)
{
	if (c >= 32 && c < 127)
		fprintf(f, "%c", c);
	else
		fprintf(f, "<%02x>", c);
}


void ieee802_1x_dump_state(FILE *f, const char *prefix, struct sta_info *sta)
{
	struct eapol_state_machine *sm = sta->eapol_sm;
	if (sm == NULL)
		return;

	fprintf(f, "%sIEEE 802.1X:\n", prefix);

	if (sm->identity) {
		size_t i;
		fprintf(f, "%sidentity=", prefix);
		for (i = 0; i < sm->identity_len; i++)
			fprint_char(f, sm->identity[i]);
		fprintf(f, "\n");
	}

	fprintf(f, "%slast EAP type: Authentication Server: %d (%s) "
		"Supplicant: %d (%s)\n", prefix,
		sm->eap_type_authsrv, eap_type_text(sm->eap_type_authsrv),
		sm->eap_type_supp, eap_type_text(sm->eap_type_supp));

	fprintf(f, "%scached_packets=%s\n", prefix,
		sm->last_recv_radius ? "[RX RADIUS]" : "");

	eapol_auth_dump_state(f, prefix, sm);
}
#endif /* asd_DUMP_STATE */


static int ieee802_1x_rekey_broadcast(struct asd_data *wasd)
{
	if (wasd->conf->default_wep_key_len < 1)
		return 0;

	os_free(wasd->default_wep_key);
	wasd->default_wep_key = os_zalloc(wasd->conf->default_wep_key_len);
	if (wasd->default_wep_key == NULL ||
	    os_get_random(wasd->default_wep_key,
			  wasd->conf->default_wep_key_len)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not generate random WEP key.\n");
		os_free(wasd->default_wep_key);
		wasd->default_wep_key = NULL;
		return -1;
	}

	wpa_hexdump_key(MSG_DEBUG, "IEEE 802.1X: New default WEP key",
			wasd->default_wep_key,
			wasd->conf->default_wep_key_len);

	return 0;
}


static int ieee802_1x_sta_key_available(struct asd_data *wasd,
					struct sta_info *sta, void *ctx)
{
	if (sta->eapol_sm) {
		sta->eapol_sm->eap_if->eapKeyAvailable = TRUE;
		eapol_auth_step(sta->eapol_sm);
	}
	return 0;
}


static void ieee802_1x_rekey(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;

	if (wasd->default_wep_key_idx >= 3)
		wasd->default_wep_key_idx =
			wasd->conf->individual_wep_key_len > 0 ? 1 : 0;
	else
		wasd->default_wep_key_idx++;
	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: New default WEP key index %d",
		   wasd->default_wep_key_idx);
		      
	if (ieee802_1x_rekey_broadcast(wasd)) {
		asd_logger(wasd, NULL, asd_MODULE_IEEE8021X,
			       asd_LEVEL_WARNING, "failed to generate a "
			       "new broadcast key");
		os_free(wasd->default_wep_key);
		wasd->default_wep_key = NULL;
		return;
	}

	/* TODO: Could setup key for RX here, but change default TX keyid only
	 * after new broadcast key has been sent to all stations. */
	 
	if (asd_set_encryption(wasd->conf->iface, wasd, "WEP", NULL,
				   wasd->default_wep_key_idx,
				   wasd->default_wep_key,
				   wasd->conf->default_wep_key_len, 1)) {
		asd_logger(wasd, NULL, asd_MODULE_IEEE8021X,
			       asd_LEVEL_WARNING, "failed to configure a "
			       "new broadcast key");
		os_free(wasd->default_wep_key);
		wasd->default_wep_key = NULL;
		return;
	}

	ap_for_each_sta(wasd, ieee802_1x_sta_key_available, NULL);

	if (wasd->conf->wep_rekeying_period > 0) {
		circle_register_timeout(wasd->conf->wep_rekeying_period, 0,
				       ieee802_1x_rekey, wasd, NULL);
	}
	
}


static void ieee802_1x_eapol_send(void *ctx, void *sta_ctx, u8 type,
				  const u8 *data, size_t datalen)
{
	ieee802_1x_send(ctx, sta_ctx, type, data, datalen);
}


static void ieee802_1x_aaa_send(void *ctx, void *sta_ctx,
				const u8 *data, size_t datalen)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;

	ieee802_1x_encapsulate_radius(wasd, sta, data, datalen);
}


static void _ieee802_1x_finished(void *ctx, void *sta_ctx, int success,
				 int preauth)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;
	if (preauth)
		rsn_preauth_finished(wasd, sta, success);
	else
		ieee802_1x_finished(wasd, sta, success);
}


static int ieee802_1x_get_eap_user(void *ctx, const u8 *identity,
				   size_t identity_len, int phase2,
				   struct eap_user *user)
{
	struct asd_data *wasd = ctx;
	const struct asd_eap_user *eap_user;
	int i, count;

	eap_user = asd_get_eap_user(wasd->conf, identity,
					identity_len, phase2);
	if (eap_user == NULL)
		return -1;

	os_memset(user, 0, sizeof(*user));
	user->phase2 = phase2;
	count = EAP_USER_MAX_METHODS;
	if (count > EAP_MAX_METHODS)
		count = EAP_MAX_METHODS;
	for (i = 0; i < count; i++) {
		user->methods[i].vendor = eap_user->methods[i].vendor;
		user->methods[i].method = eap_user->methods[i].method;
	}

	if (eap_user->password) {
		user->password = os_zalloc(eap_user->password_len);
		if (user->password == NULL)
			return -1;
		os_memcpy(user->password, eap_user->password,
			  eap_user->password_len);
		user->password_len = eap_user->password_len;
	}
	user->force_version = eap_user->force_version;
	user->ttls_auth = eap_user->ttls_auth;

	return 0;
}


static int ieee802_1x_sta_entry_alive(void *ctx, const u8 *addr)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta;
	sta = ap_get_sta(wasd, addr);
	if (sta == NULL || sta->eapol_sm == NULL)
		return 0;
	return 1;
}


static void ieee802_1x_logger(void *ctx, const u8 *addr,
			      eapol_logger_level level, const char *txt)
{
	struct asd_data *wasd = ctx;
	int hlevel;

	switch (level) {
	case EAPOL_LOGGER_WARNING:
		hlevel = asd_LEVEL_WARNING;
		break;
	case EAPOL_LOGGER_INFO:
		hlevel = asd_LEVEL_INFO;
		break;
	case EAPOL_LOGGER_DEBUG:
	default:
		hlevel = asd_LEVEL_DEBUG;
		break;
	}

	asd_logger(wasd, addr, asd_MODULE_IEEE8021X, hlevel, "%s",
		       txt);
}


static void ieee802_1x_set_port_authorized(void *ctx, void *sta_ctx,
					   int authorized)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;
	ieee802_1x_set_sta_authorized(wasd, sta, authorized);
}


static void _ieee802_1x_abort_auth(void *ctx, void *sta_ctx)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;
	ieee802_1x_abort_auth(wasd, sta);
}


static void _ieee802_1x_tx_key(void *ctx, void *sta_ctx)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;
	ieee802_1x_tx_key(wasd, sta);
}


int ieee802_1x_init(struct asd_data *wasd)
{
	int i;
	struct eapol_auth_config conf;
	struct eapol_auth_cb cb;

	os_memset(&conf, 0, sizeof(conf));
	conf.wasd = wasd;
	conf.eap_reauth_period = wasd->conf->eap_reauth_period;
	conf.wpa = wasd->conf->wpa;
	conf.individual_wep_key_len = wasd->conf->individual_wep_key_len;
	conf.eap_server = wasd->conf->eap_server;
	conf.ssl_ctx = wasd->ssl_ctx;
	conf.eap_sim_db_priv = wasd->eap_sim_db_priv;
	conf.eap_req_id_text = wasd->conf->eap_req_id_text;
	conf.eap_req_id_text_len = wasd->conf->eap_req_id_text_len;
	conf.pac_opaque_encr_key = wasd->conf->pac_opaque_encr_key;
	conf.eap_fast_a_id = wasd->conf->eap_fast_a_id;
	conf.eap_sim_aka_result_ind = wasd->conf->eap_sim_aka_result_ind;

	os_memset(&cb, 0, sizeof(cb));
	cb.eapol_send = ieee802_1x_eapol_send;
	cb.aaa_send = ieee802_1x_aaa_send;
	cb.finished = _ieee802_1x_finished;
	cb.get_eap_user = ieee802_1x_get_eap_user;
	cb.sta_entry_alive = ieee802_1x_sta_entry_alive;
	cb.logger = ieee802_1x_logger;
	cb.set_port_authorized = ieee802_1x_set_port_authorized;
	cb.abort_auth = _ieee802_1x_abort_auth;
	cb.tx_key = _ieee802_1x_tx_key;

	wasd->eapol_auth = eapol_auth_init(&conf, &cb);
	if (wasd->eapol_auth == NULL)
		return -1;

	if ((wasd->conf->ieee802_1x || wasd->conf->wpa) &&
	    asd_set_ieee8021x(wasd->conf->iface, wasd, 1))
		return -1;
	/*
	if (radius_client_register(wasd->radius, RADIUS_AUTH,
				   ieee802_1x_receive_auth, wasd))
		return -1;
	*/	
	/*
	if (radius_client_register(ASD_WLAN[wasd->WlanID]->radius->auth, 
				   ieee802_1x_receive_auth, ASD_WLAN[wasd->WlanID]->radius->auth))
		return -1;
*/
	if (wasd->conf->default_wep_key_len) {
		asd_set_privacy(wasd, 1);

		for (i = 0; i < 4; i++)
			asd_set_encryption(wasd->conf->iface, wasd,
					       "none", NULL, i, NULL, 0, 0);
//		asd_printf(ASD_1X,MSG_DEBUG,"\n\n\n\n\n ieee802_1x_rekey(wasd, NULL);\n\n\n\n");
		ieee802_1x_rekey(wasd, NULL);

		if (wasd->default_wep_key == NULL)
			return -1;
	}

	return 0;
}


void ieee802_1x_deinit(struct asd_data *wasd)
{
	circle_cancel_timeout(ieee802_1x_rekey, wasd, NULL);

	if (wasd->driver != NULL &&
	    (wasd->conf->ieee802_1x || wasd->conf->wpa))
		asd_set_ieee8021x(wasd->conf->iface, wasd, 0);

	eapol_auth_deinit(wasd->eapol_auth);
	wasd->eapol_auth = NULL;
}


int ieee802_1x_reconfig(struct asd_data *wasd, 
			struct asd_config *oldconf,
			struct asd_bss_config *oldbss)
{
	ieee802_1x_deinit(wasd);
	return ieee802_1x_init(wasd);
}


int ieee802_1x_tx_status(struct asd_data *wasd, struct sta_info *sta,
			 u8 *buf, size_t len, int ack)
{
	struct ieee80211_hdr *hdr;
	struct ieee802_1x_hdr *xhdr;
	struct ieee802_1x_eapol_key *key;
	u8 *pos;
	const unsigned char rfc1042_hdr[ETH_ALEN] =
		{ 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

	if (sta == NULL)
		return -1;
	if (len < sizeof(*hdr) + sizeof(rfc1042_hdr) + 2 + sizeof(*xhdr))
		return 0;

	hdr = (struct ieee80211_hdr *) buf;
	pos = (u8 *) (hdr + 1);
	if (os_memcmp(pos, rfc1042_hdr, sizeof(rfc1042_hdr)) != 0)
		return 0;
	pos += sizeof(rfc1042_hdr);
	if (WPA_GET_BE16(pos) != ETH_P_PAE)
		return 0;
	pos += 2;

	xhdr = (struct ieee802_1x_hdr *) pos;
	pos += sizeof(*xhdr);

	asd_printf(ASD_1X,MSG_DEBUG, "IEEE 802.1X: " MACSTR " TX status - version=%d "
		   "type=%d length=%d - ack=%d",
		   MAC2STR(sta->addr), xhdr->version, xhdr->type,
		   be_to_host16(xhdr->length), ack);

	/* EAPOL EAP-Packet packets are eventually re-sent by either Supplicant
	 * or Authenticator state machines, but EAPOL-Key packets are not
	 * retransmitted in case of failure. Try to re-sent failed EAPOL-Key
	 * packets couple of times because otherwise STA keys become
	 * unsynchronized with AP. */
	if (xhdr->type == IEEE802_1X_TYPE_EAPOL_KEY && !ack &&
	    pos + sizeof(*key) <= buf + len) {
		key = (struct ieee802_1x_eapol_key *) pos;
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE8021X,
			       asd_LEVEL_DEBUG, "did not Ack EAPOL-Key "
			       "frame (%scast index=%d)",
			       key->key_index & BIT(7) ? "uni" : "broad",
			       key->key_index & ~BIT(7));
		/* TODO: re-send EAPOL-Key couple of times (with short delay
		 * between them?). If all attempt fail, report error and
		 * deauthenticate STA so that it will get new keys when
		 * authenticating again (e.g., after returning in range).
		 * Separate limit/transmit state needed both for unicast and
		 * broadcast keys(?) */
	}
	/* TODO: could move unicast key configuration from ieee802_1x_tx_key()
	 * to here and change the key only if the EAPOL-Key packet was Acked.
	 */

	return 1;
}


u8 * ieee802_1x_get_identity(struct eapol_state_machine *sm, size_t *len)
{
	if (sm == NULL || sm->identity == NULL)
		return NULL;

	*len = sm->identity_len;
	return sm->identity;
}


u8 * ieee802_1x_get_radius_class(struct eapol_state_machine *sm, size_t *len,
				 int idx)
{
	if (sm == NULL || sm->radius_class.attr == NULL ||
	    idx >= (int) sm->radius_class.count)
		return NULL;

	*len = sm->radius_class.attr[idx].len;
	return sm->radius_class.attr[idx].data;
}


const u8 * ieee802_1x_get_key(struct eapol_state_machine *sm, size_t *len)
{
	if (sm == NULL)
		return NULL;

	*len = sm->eap_if->eapKeyDataLen;
	return sm->eap_if->eapKeyData;
}


void ieee802_1x_notify_port_enabled(struct eapol_state_machine *sm,
				    int enabled)
{
	if (sm == NULL)
		return;
	sm->eap_if->portEnabled = enabled ? TRUE : FALSE;
	eapol_auth_step(sm);
}


void ieee802_1x_notify_port_valid(struct eapol_state_machine *sm,
				  int valid)
{
	if (sm == NULL)
		return;
	sm->portValid = valid ? TRUE : FALSE;
	eapol_auth_step(sm);
}


void ieee802_1x_notify_pre_auth(struct eapol_state_machine *sm, int pre_auth)
{
	if (sm == NULL)
		return;
	if (pre_auth)
		sm->flags |= EAPOL_SM_PREAUTH;
	else
		sm->flags &= ~EAPOL_SM_PREAUTH;
}


static const char * bool_txt(Boolean bool)
{
	return bool ? "TRUE" : "FALSE";
}


int ieee802_1x_get_mib(struct asd_data *wasd, char *buf, size_t buflen)
{
	/* TODO */
	return 0;
}


int ieee802_1x_get_mib_sta(struct asd_data *wasd, struct sta_info *sta,
			   char *buf, size_t buflen)
{
	int len = 0, ret;
	struct eapol_state_machine *sm = sta->eapol_sm;

	if (sm == NULL)
		return 0;

	ret = os_snprintf(buf + len, buflen - len,
			  "dot1xPaePortNumber=%d\n"
			  "dot1xPaePortProtocolVersion=%d\n"
			  "dot1xPaePortCapabilities=1\n"
			  "dot1xPaePortInitialize=%d\n"
			  "dot1xPaePortReauthenticate=FALSE\n",
			  sta->aid,
			  EAPOL_VERSION,
			  sm->initialize);
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	/* dot1xAuthConfigTable */
	ret = os_snprintf(buf + len, buflen - len,
			  "dot1xAuthPaeState=%d\n"
			  "dot1xAuthBackendAuthState=%d\n"
			  "dot1xAuthAdminControlledDirections=%d\n"
			  "dot1xAuthOperControlledDirections=%d\n"
			  "dot1xAuthAuthControlledPortStatus=%d\n"
			  "dot1xAuthAuthControlledPortControl=%d\n"
			  "dot1xAuthQuietPeriod=%u\n"
			  "dot1xAuthServerTimeout=%u\n"
			  "dot1xAuthReAuthPeriod=%u\n"
			  "dot1xAuthReAuthEnabled=%s\n"
			  "dot1xAuthKeyTxEnabled=%s\n",
			  sm->auth_pae_state + 1,
			  sm->be_auth_state + 1,
			  sm->adminControlledDirections,
			  sm->operControlledDirections,
			  sm->authPortStatus,
			  sm->portControl,
			  sm->quietPeriod,
			  sm->serverTimeout,
			  sm->reAuthPeriod,
			  bool_txt(sm->reAuthEnabled),
			  bool_txt(sm->keyTxEnabled));
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	/* dot1xAuthStatsTable */
	ret = os_snprintf(buf + len, buflen - len,
			  "dot1xAuthEapolFramesRx=%u\n"
			  "dot1xAuthEapolFramesTx=%u\n"
			  "dot1xAuthEapolStartFramesRx=%u\n"
			  "dot1xAuthEapolLogoffFramesRx=%u\n"
			  "dot1xAuthEapolRespIdFramesRx=%u\n"
			  "dot1xAuthEapolRespFramesRx=%u\n"
			  "dot1xAuthEapolReqIdFramesTx=%u\n"
			  "dot1xAuthEapolReqFramesTx=%u\n"
			  "dot1xAuthInvalidEapolFramesRx=%u\n"
			  "dot1xAuthEapLengthErrorFramesRx=%u\n"
			  "dot1xAuthLastEapolFrameVersion=%u\n"
			  "dot1xAuthLastEapolFrameSource=" MACSTR "\n",
			  sm->dot1xAuthEapolFramesRx,
			  sm->dot1xAuthEapolFramesTx,
			  sm->dot1xAuthEapolStartFramesRx,
			  sm->dot1xAuthEapolLogoffFramesRx,
			  sm->dot1xAuthEapolRespIdFramesRx,
			  sm->dot1xAuthEapolRespFramesRx,
			  sm->dot1xAuthEapolReqIdFramesTx,
			  sm->dot1xAuthEapolReqFramesTx,
			  sm->dot1xAuthInvalidEapolFramesRx,
			  sm->dot1xAuthEapLengthErrorFramesRx,
			  sm->dot1xAuthLastEapolFrameVersion,
			  MAC2STR(sm->addr));
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	/* dot1xAuthDiagTable */
	ret = os_snprintf(buf + len, buflen - len,
			  "dot1xAuthEntersConnecting=%u\n"
			  "dot1xAuthEapLogoffsWhileConnecting=%u\n"
			  "dot1xAuthEntersAuthenticating=%u\n"
			  "dot1xAuthAuthSuccessesWhileAuthenticating=%u\n"
			  "dot1xAuthAuthTimeoutsWhileAuthenticating=%u\n"
			  "dot1xAuthAuthFailWhileAuthenticating=%u\n"
			  "dot1xAuthAuthEapStartsWhileAuthenticating=%u\n"
			  "dot1xAuthAuthEapLogoffWhileAuthenticating=%u\n"
			  "dot1xAuthAuthReauthsWhileAuthenticated=%u\n"
			  "dot1xAuthAuthEapStartsWhileAuthenticated=%u\n"
			  "dot1xAuthAuthEapLogoffWhileAuthenticated=%u\n"
			  "dot1xAuthBackendResponses=%u\n"
			  "dot1xAuthBackendAccessChallenges=%u\n"
			  "dot1xAuthBackendOtherRequestsToSupplicant=%u\n"
			  "dot1xAuthBackendAuthSuccesses=%u\n"
			  "dot1xAuthBackendAuthFails=%u\n",
			  sm->authEntersConnecting,
			  sm->authEapLogoffsWhileConnecting,
			  sm->authEntersAuthenticating,
			  sm->authAuthSuccessesWhileAuthenticating,
			  sm->authAuthTimeoutsWhileAuthenticating,
			  sm->authAuthFailWhileAuthenticating,
			  sm->authAuthEapStartsWhileAuthenticating,
			  sm->authAuthEapLogoffWhileAuthenticating,
			  sm->authAuthReauthsWhileAuthenticated,
			  sm->authAuthEapStartsWhileAuthenticated,
			  sm->authAuthEapLogoffWhileAuthenticated,
			  sm->backendResponses,
			  sm->backendAccessChallenges,
			  sm->backendOtherRequestsToSupplicant,
			  sm->backendAuthSuccesses,
			  sm->backendAuthFails);
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	/* dot1xAuthSessionStatsTable */
	ret = os_snprintf(buf + len, buflen - len,
			  /* TODO: dot1xAuthSessionOctetsRx */
			  /* TODO: dot1xAuthSessionOctetsTx */
			  /* TODO: dot1xAuthSessionFramesRx */
			  /* TODO: dot1xAuthSessionFramesTx */
			  "dot1xAuthSessionId=%08X-%08X\n"
			  "dot1xAuthSessionAuthenticMethod=%d\n"
			  "dot1xAuthSessionTime=%u\n"
			  "dot1xAuthSessionTerminateCause=999\n"
			  "dot1xAuthSessionUserName=%s\n",
			  sta->acct_session_id_hi, sta->acct_session_id_lo,
			  (wpa_auth_sta_key_mgmt(sta->wpa_sm) ==
			   WPA_KEY_MGMT_IEEE8021X ||
			   wpa_auth_sta_key_mgmt(sta->wpa_sm) ==
			   WPA_KEY_MGMT_FT_IEEE8021X) ? 1 : 2,
			  (unsigned int) (time(NULL) -
					  sta->acct_session_start),
			  sm->identity);
	if (ret < 0 || (size_t) ret >= buflen - len)
		return len;
	len += ret;

	return len;
}


static void ieee802_1x_finished(struct asd_data *wasd,
				struct sta_info *sta, int success)
{
	const u8 *key;
	size_t len;
	/* TODO: get PMKLifetime from WPA parameters */
	static const int dot11RSNAConfigPMKLifetime = 43200;

	key = ieee802_1x_get_key(sta->eapol_sm, &len);
	if (success && key && len >= PMK_LEN &&
	    wpa_auth_pmksa_add(sta->wpa_sm, key, dot11RSNAConfigPMKLifetime,
			       sta->eapol_sm) == 0) {
		unsigned char WLANID = wasd->WlanID;
		struct PMK_STAINFO *pmk_sta;
		if(ASD_WLAN[WLANID] != NULL){
			pmk_sta = pmk_ap_sta_add(ASD_WLAN[WLANID],sta->addr);			
			pmk_sta->idhi = sta->acct_session_id_hi;
			pmk_sta->idlo = sta->acct_session_id_lo;
			pmk_add_bssindex(wasd->BSSIndex,pmk_sta);
		}
		asd_logger(wasd, sta->addr, asd_MODULE_WPA,
			       asd_LEVEL_DEBUG,
			       "Added PMKSA cache entry (IEEE 802.1X)");
	}
}

//mahz 2010.11.30
void wapi_radius_auth_send(void *ctx, void *sta_ctx)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;
	if((sta)&&(sta->in_addr)&&(!(strcmp(sta->in_addr, "0.0.0.0") == 0))){
		inet_aton(sta->in_addr,&(sta->ip_addr));
		ieee802_1x_encapsulate_radius(wasd, sta, NULL, 0);
	}
	else{
		circle_register_timeout(5, 0, wapi_radius_auth_send, wasd, sta);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"else branch\n");
	}
}

//mahz add 2011.3.7
void sta_ip_addr_check(void *ctx, void *sta_ctx){

	struct asd_data *wasd = ctx;
	struct sta_info *sta = sta_ctx;

	if((sta)&&(sta->in_addr)&&(!(strcmp(sta->in_addr, "0.0.0.0") == 0))){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta->in_addr : %s\n",sta->in_addr);
		inet_aton(sta->in_addr,&sta->ip_addr);
		if(sta->security_type == MAC_AUTH){
			UpdateStaInfoToCHILL(wasd->BSSIndex,sta,ASD_MAC_AUTH);

		}
		else{
			UpdateStaInfoToCHILL(wasd->BSSIndex,sta,ASD_AUTH);
		}
	}
}
void  asd_wlan_radius_free(unsigned int wlanid)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in func:%s\n",__func__);
	if(!ASD_WLAN[wlanid])
		return;
	if(ASD_WLAN[wlanid]->radius)
		radius_client_deinit(ASD_WLAN[wlanid]->radius);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"radius deinit over!\n");
	
	if(ASD_WLAN[wlanid]->radius_server)
	{
		int i = 0;
		if(ASD_WLAN[wlanid]->radius_server->auth_servers != NULL) {
		
			struct asd_radius_server *auth_server;
			for(i=0; i<ASD_WLAN[wlanid]->radius_server->num_auth_servers; i++) {
				auth_server = &ASD_WLAN[wlanid]->radius_server->auth_servers[i];
				if(auth_server->shared_secret != NULL) {
					os_free(auth_server->shared_secret);
					auth_server->shared_secret = NULL;
				}
			}
			os_free(ASD_WLAN[wlanid]->radius_server->auth_servers);
			ASD_WLAN[wlanid]->radius_server->auth_servers = NULL;
		}
		if(ASD_WLAN[wlanid]->radius_server->acct_servers != NULL) {
			struct asd_radius_server *acct_server;
			for(i=0; i<ASD_WLAN[wlanid]->radius_server->num_acct_servers; i++) {
				acct_server = &ASD_WLAN[wlanid]->radius_server->acct_servers[i];
				if(acct_server->shared_secret != NULL) {
					os_free(acct_server->shared_secret);
					acct_server->shared_secret = NULL;
				}
			}
			os_free(ASD_WLAN[wlanid]->radius_server->acct_servers);
			ASD_WLAN[wlanid]->radius_server->acct_servers = NULL;
		}
		os_free(ASD_WLAN[wlanid]->radius_server);
		ASD_WLAN[wlanid]->radius_server = NULL;
	}
}
int asd_wlan_radius_init(unsigned int wlanid)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in func:%s\n",__func__);
	int SID;
	int len = 0;
	if(ASD_WLAN[wlanid] == NULL)
		return ASD_WLAN_NOT_EXIST;
	SID = ASD_WLAN[wlanid]->SecurityID;
	if(SID <= 0 ||SID >= 129)
	{
		return ASD_SECURITY_NOT_EXIST;
	}	
	if(ASD_SECURITY[SID] == NULL)
	{
		return ASD_SECURITY_NOT_EXIST;
	}	
	if(!((ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == MD5)||((ASD_SECURITY[SID]->securityType == WAPI_AUTH) && (ASD_SECURITY[SID]->wapi_radius_auth == 1))||(ASD_SECURITY[SID]->extensible_auth == 1)))
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"security type is no need to create radius server!\n");
		return ASD_DBUS_SUCCESS;
	}
	asd_wlan_radius_free(wlanid);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"wlan free over!\n");
	ASD_WLAN[wlanid]->radius_server = os_zalloc(sizeof(struct asd_radius_servers));
	if(ASD_WLAN[wlanid]->radius_server == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"radius_server malloc error!\n");
		return -1;
	}
	if(ASD_SECURITY[SID]->acct_interim_interval != 0){			
		ASD_WLAN[wlanid]->radius_server->acct_interim_interval = ASD_SECURITY[SID]->acct_interim_interval;
	}
	if( os_strncmp(ASD_SECURITY[SID]->host_ip,"127.0.0.1",9) )	{			
		if (asd_parse_ip_addr(ASD_SECURITY[SID]->host_ip, &ASD_WLAN[wlanid]->radius_server->client_addr) == 0) {
			ASD_WLAN[wlanid]->radius_server->force_client_addr = 1;
		}else {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",SID,ASD_SECURITY[SID]->host_ip);
			return -1;
		}
	}
	
	if((ASD_SECURITY[SID]->auth.auth_ip == NULL)&&(wlanid != 0)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"security %d auth ip is null!\n",SID);
		return -1;
	}
	
	if(ASD_SECURITY[SID]->auth.auth_ip != NULL)
	{
		if (asd_config_read_radius_addr(
				&ASD_WLAN[wlanid]->radius_server->auth_servers,
				&ASD_WLAN[wlanid]->radius_server->num_auth_servers, ASD_SECURITY[SID]->auth.auth_ip, 1812,
				&ASD_WLAN[wlanid]->radius_server->auth_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->auth.auth_ip);
			
		}
	
		ASD_WLAN[wlanid]->radius_server->auth_server->port = ASD_SECURITY[SID]->auth.auth_port;
		
		int len = os_strlen(ASD_SECURITY[SID]->auth.auth_shared_secret);
		if (len == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		ASD_WLAN[wlanid]->radius_server->auth_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->auth.auth_shared_secret);
		ASD_WLAN[wlanid]->radius_server->auth_server->shared_secret_len = len;
	}
	if((ASD_SECURITY[SID]->acct.acct_ip == NULL)&&(wlanid != 0))
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"security %d auth ip is null!\n",SID);
		return -1;
	}
	if(ASD_SECURITY[SID]->acct.acct_ip != NULL)
	{
		if (asd_config_read_radius_addr(
				&ASD_WLAN[wlanid]->radius_server->acct_servers,
				&ASD_WLAN[wlanid]->radius_server->num_acct_servers, ASD_SECURITY[SID]->acct.acct_ip, 1813,
				&ASD_WLAN[wlanid]->radius_server->acct_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->acct.acct_ip);
		}
	
		ASD_WLAN[wlanid]->radius_server->acct_server->port = ASD_SECURITY[SID]->acct.acct_port;
		len = os_strlen(ASD_SECURITY[SID]->acct.acct_shared_secret);
		if (len == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		ASD_WLAN[wlanid]->radius_server->acct_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->acct.acct_shared_secret);
		ASD_WLAN[wlanid]->radius_server->acct_server->shared_secret_len = len;
	
	}
	if(ASD_SECURITY[SID]->auth.secondary_auth_ip != NULL){
		if (asd_config_read_radius_addr(
				&ASD_WLAN[wlanid]->radius_server->auth_servers,
				&ASD_WLAN[wlanid]->radius_server->num_auth_servers, ASD_SECURITY[SID]->auth.secondary_auth_ip, 1812,
				&ASD_WLAN[wlanid]->radius_server->auth_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->auth.secondary_auth_ip);
			
		}
		
		ASD_WLAN[wlanid]->radius_server->auth_server->port = ASD_SECURITY[SID]->auth.secondary_auth_port;
		
		 len = os_strlen(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
		if (len == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		ASD_WLAN[wlanid]->radius_server->auth_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->auth.secondary_auth_shared_secret);
		ASD_WLAN[wlanid]->radius_server->auth_server->shared_secret_len = len;
	}
	if(ASD_SECURITY[SID]->acct.secondary_acct_ip != NULL){
		if (asd_config_read_radius_addr(
				&ASD_WLAN[wlanid]->radius_server->acct_servers,
				&ASD_WLAN[wlanid]->radius_server->num_acct_servers, ASD_SECURITY[SID]->acct.secondary_acct_ip, 1813,
				&ASD_WLAN[wlanid]->radius_server->acct_server)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: invalid IP address '%s'\n",
				   SID, ASD_SECURITY[SID]->acct.secondary_acct_ip);
		}
		ASD_WLAN[wlanid]->radius_server->acct_server->port = ASD_SECURITY[SID]->acct.secondary_acct_port;
		len = os_strlen(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
		if (len == 0) {
			/* RFC 2865, Ch. 3 */
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Security %d: empty shared secret is not "
				   "allowed.\n", SID);
		}
		ASD_WLAN[wlanid]->radius_server->acct_server->shared_secret =
			(u8 *) os_strdup(ASD_SECURITY[SID]->acct.secondary_acct_shared_secret);
		ASD_WLAN[wlanid]->radius_server->acct_server->shared_secret_len = len;
	}
	//qiuchen
	if((ASD_SECURITY[SID]->auth_server_current_use == RADIUS_BAK) && ASD_WLAN[wlanid]->radius_server->num_auth_servers > 1)
		ASD_WLAN[wlanid]->radius_server->auth_server = &ASD_WLAN[wlanid]->radius_server->auth_servers[1];
	else
		ASD_WLAN[wlanid]->radius_server->auth_server = ASD_WLAN[wlanid]->radius_server->auth_servers;
	if((ASD_SECURITY[SID]->acct_server_current_use == RADIUS_BAK) && ASD_WLAN[wlanid]->radius_server->num_acct_servers > 1)
		ASD_WLAN[wlanid]->radius_server->acct_server = &ASD_WLAN[wlanid]->radius_server->acct_servers[1];
	else
		ASD_WLAN[wlanid]->radius_server->acct_server = ASD_WLAN[wlanid]->radius_server->acct_servers;
	ASD_WLAN[wlanid]->radius_server->slot_value = ASD_SECURITY[SID]->slot_value;	//mahz add 2011.10.26
	ASD_WLAN[wlanid]->radius_server->inst_value = ASD_SECURITY[SID]->inst_value;
	if(ASD_SECURITY[SID]->accounting_on_disable){
		ASD_WLAN[wlanid]->radius_server->accounting_on_disable = 1;
	}
	if(ASD_SECURITY[SID]->radius_extend_attr){
		ASD_WLAN[wlanid]->radius_server->radius_extend_attr = 1;
	}
	if(ASD_SECURITY[SID]->distribute_off){
		ASD_WLAN[wlanid]->radius_server->distribute_off = 1;
	}	
	struct radius_client_info *client_info;
	ASD_WLAN[wlanid]->radius  = radius_client_init(ASD_WLAN[wlanid],ASD_WLAN[wlanid]->radius_server);
	if(ASD_WLAN[wlanid]->radius == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"radius init failed!\n");
		return -1;
	}
	client_info = ASD_WLAN[wlanid]->radius->auth;
	while(client_info){
		if (radius_client_register(client_info,
					   ieee802_1x_receive_auth,client_info))
			return -1;
		client_info = client_info->next;
	}
	if(wlanid != 0)
	{
		if (accounting_init(wlanid)) {
			asd_printf(ASD_DEFAULT,MSG_ERROR,"Accounting initialization failed.\n");
			return -1;
		}
	}
	return ASD_DBUS_SUCCESS;
}
//qiuchen add it for master_bak radius server
void RADIUS_PACKAGESEND_AUTH(struct heart_test_radius_data *wasd)
{
	struct radius_msg *msg;
	unsigned int ip_addr = 0;
	char A[6] = {0};
	char buf[128];
	unsigned char identifier = 0;
	identifier = radius_client_test_get_id(wasd);
	unsigned char SID = wasd->SecurityID;
			
	msg = radius_msg_new(RADIUS_CODE_ACCESS_REQUEST,identifier);
	if (msg == NULL) {
		asd_printf(ASD_1X,MSG_WARNING,"Could not create net RADIUS packet\n");
		return;
	}
	radius_msg_make_authenticator(msg, (u8 *)wasd, sizeof(*wasd));
	if(ASD_SECURITY[SID]->ac_radius_name != NULL){
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
				(u8*)ASD_SECURITY[SID]->ac_radius_name, strlen(ASD_SECURITY[SID]->ac_radius_name))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
			goto fail;
		}
	}
	else{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
					(u8*)"000000", 6)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
			goto fail;
		}
	}
	if (wasd->own_ip_addr.af == AF_INET &&
		!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IP_ADDRESS,
					(u8 *) &wasd->own_ip_addr.u.v4, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IP-Address\n");
		goto fail;
	}
	
#ifdef ASD_IPV6
	if (wasd->own_ip_addr.af == AF_INET6 &&
		!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IPV6_ADDRESS,
				(u8 *) &wasd->own_ip_addr.u.v6, 16)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IPv6-Address\n");
		goto fail;
	}
#endif /* ASD_IPV6 */

	if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
					(u8 *) "127.0.0.1",
					os_strlen("127.0.0.1"))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
		goto fail;
	}

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT, 1)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port\n");
		goto fail;
	}
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID,
		(u8 *) "127.0.0.1", 
		os_strlen("127.0.0.1"))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
		goto fail;
	}
	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				MAC2STR(A), "radius_test");
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLED_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Called-Station-Id\n");
		goto fail;
	}
	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT,
			MAC2STR(A));
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLING_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Calling-Station-Id\n");
		goto fail;
	}
	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_FRAMED_MTU, 1400)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Framed-MTU\n");
		goto fail;
	}
	if((ASD_SECURITY[SID] != NULL)&&(ASD_SECURITY[SID]->wired_radius == 1)){
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_3)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}else{
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_11)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_FRAME_IP_ADDRESS, 	
				 (u8 *) &ip_addr, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add FRAME-IP-Address in %s\n",__func__);
		goto fail;
	}

	os_snprintf(buf, sizeof(buf), "CONNECT %d Mbps %s",
			0,"802.11g");
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CONNECT_INFO,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Connect-Info\n");
		goto fail;
	}
	radius_test_client_send_without_retrans(wasd, msg, RADIUS_AUTH);
	wasd->conf->radius_auth_heart_req_num++;
	ASD_SECURITY[SID]->heart_test_identifier_auth = (unsigned int)identifier;
	fail:
		radius_msg_free(msg);
		os_free(msg);
		msg = NULL;
}
void RADIUS_PACKAGESEND_ACCT(struct heart_test_radius_data *wasd)
{
	struct radius_msg *msg;
	//unsigned int ip_addr = 0;
	char A[6] = {0};
	char buf[128];
	unsigned char identifier = 0;
	identifier = radius_client_test_get_id(wasd);
	unsigned char SID = wasd->SecurityID;

	msg = radius_msg_new(RADIUS_CODE_ACCOUNTING_REQUEST,identifier);
	if (msg == NULL) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not create net RADIUS packet\n");
		return;
	}
	radius_msg_make_authenticator(msg, (u8 *) wasd, sizeof(*wasd));
	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_STATUS_TYPE,
					   RADIUS_ACCT_STATUS_TYPE_START)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Status-Type\n");
		goto fail;
	}
	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_AUTHENTIC,
					   wasd->ieee802_1x ?
					   RADIUS_ACCT_AUTHENTIC_RADIUS :
					   RADIUS_ACCT_AUTHENTIC_LOCAL)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Authentic\n");
		goto fail;
	}
	if(ASD_SECURITY[SID]->ac_radius_name != NULL){
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
			(u8*)ASD_SECURITY[SID]->ac_radius_name, strlen(ASD_SECURITY[SID]->ac_radius_name))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
			goto fail;
		}
	}
	else{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
			(u8*)"000000", 6)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
			goto fail;
		}
	}
	if (wasd->own_ip_addr.af == AF_INET &&
		!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IP_ADDRESS,
					(u8 *) &wasd->own_ip_addr.u.v4, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IP-Address\n");
		goto fail;
	}

#ifdef ASD_IPV6
	if (wasd->own_ip_addr.af == AF_INET6 &&
		!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IPV6_ADDRESS,
					(u8 *) &wasd->own_ip_addr.u.v6, 16)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IPv6-Address\n");
		goto fail;
	}
#endif /* ASD_IPV6 */

	if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
					(u8 *) "127.0.0.1",
					os_strlen("127.0.0.1"))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
		goto fail;
	}

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT, 1)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port\n");
		goto fail;
	}
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID,
		(u8 *) "127.0.0.1", 
		os_strlen("127.0.0.1"))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
		goto fail;
	}

	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				MAC2STR(A), "radius_test");
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLED_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Called-Station-Id\n");
		goto fail;
	}
	os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT,
			MAC2STR(A));
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLING_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Calling-Station-Id\n");
		goto fail;
	}
	if((ASD_SECURITY[SID] != NULL)&&(ASD_SECURITY[SID]->wired_radius == 1)){
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_3)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}else{
		if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT_TYPE,
						   RADIUS_NAS_PORT_TYPE_IEEE_802_11)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
			goto fail;
		}
	}
	os_snprintf(buf, sizeof(buf), "CONNECT %d Mbps %s",
			0,"802.11g");
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CONNECT_INFO,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Connect-Info\n");
		goto fail;
	}
	ASD_SECURITY[SID]->heart_test_identifier_acct = (unsigned int)identifier;
	radius_test_client_send_without_retrans(wasd, msg, RADIUS_ACCT);
	wasd->conf->radius_acct_heart_req_num++;
	fail:
		radius_msg_free(msg);
		os_free(msg);
		msg = NULL;
}
void radius_encapsulate_heart_test_radius(void *circle_ctx, void *timeout_ctx){
	struct heart_test_radius_data *wasd = circle_ctx;
	if(wasd == NULL || wasd->conf == NULL)
		return;
	unsigned char *type = timeout_ctx;
	if(type == NULL)
		return;
	unsigned char SID = wasd->SecurityID;
	unsigned int iden = 0;
	unsigned int heart_test_interval = wasd->conf->radius_access_test_interval;
	unsigned int i = 0;
	unsigned char *c_test;
	unsigned char *r_test;
	int ret = 0;
	if(is_secondary == 1){
		circle_cancel_timeout(radius_encapsulate_heart_test_radius,wasd,&(ASD_SECURITY[SID]->radius_heart_test_type));
		circle_register_timeout(heart_test_interval,0,radius_encapsulate_heart_test_radius,wasd,&(ASD_SECURITY[SID]->radius_heart_test_type));
		return;
	}
	if(*type == RADIUS_AUTH_TEST){
		iden = ASD_SECURITY[SID]->heart_test_identifier_auth;
		if(ASD_SECURITY[SID]->auth_server_current_use == RADIUS_MASTER){
			if((iden != 32768) && (iden <256)){
				c_test = wasd->conf->c_test_window_auth;
				wasd->conf->radius_auth_heart_fail_num++;
				i = (wasd->conf->radius_auth_heart_req_num-1)%wasd->conf->c_window_size;
				if(c_test[i] == RADIUS_TEST_FAIL)
					wasd->conf->radius_auth_heart_fail_num--;
				else if(c_test[i] == RADIUS_TEST_SUC)
					wasd->conf->radius_auth_heart_res_num--;
				c_test[i] = RADIUS_TEST_FAIL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"auth_fail is %d,auth_suc is %d,auth_request is %d\n",wasd->conf->radius_auth_heart_fail_num,wasd->conf->radius_auth_heart_res_num,wasd->conf->radius_auth_heart_req_num);
				if((((double)wasd->conf->radius_auth_heart_fail_num)/wasd->conf->c_window_size) > wasd->conf->radius_res_fail_percent){
					radius_server_change(SID,RADIUS_AUTH_TEST,RADIUS_BAK);
				}
			}
		}
		else if(ASD_SECURITY[SID]->auth_server_current_use == RADIUS_BAK){
			if((iden != 32768) && (iden >=0) && (iden <256)){
				r_test = wasd->conf->r_test_window_auth;
				wasd->conf->radius_auth_heart_fail_num++;
				i = (wasd->conf->radius_auth_heart_req_num-1)%wasd->conf->r_window_size;
				if(r_test[i] == RADIUS_TEST_FAIL)
					wasd->conf->radius_auth_heart_fail_num--;
				else if(r_test[i] == RADIUS_TEST_SUC)
					wasd->conf->radius_auth_heart_res_num--;
				r_test[i] = RADIUS_TEST_FAIL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_fail:auth_fail is %d,auth_suc is %d,auth_request is %d\n",wasd->conf->radius_auth_heart_fail_num,wasd->conf->radius_auth_heart_res_num,wasd->conf->radius_auth_heart_req_num);
			}
			else{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_suc:auth_fail is %d,auth_suc is %d,auth_request is %d\n",wasd->conf->radius_auth_heart_fail_num,wasd->conf->radius_auth_heart_res_num,wasd->conf->radius_auth_heart_req_num);
				if((((double)wasd->conf->radius_auth_heart_res_num)/wasd->conf->r_window_size) > wasd->conf->radius_res_suc_percent)
					radius_server_change(SID,RADIUS_AUTH_TEST,RADIUS_MASTER);
			}
		}
			
		RADIUS_PACKAGESEND_AUTH(wasd);
		circle_register_timeout(heart_test_interval,0,radius_encapsulate_heart_test_radius,wasd,&(ASD_SECURITY[SID]->radius_auth));
	}
	else if (*type == RADIUS_ACCT_TEST){
		iden = ASD_SECURITY[SID]->heart_test_identifier_acct;
		if(ASD_SECURITY[SID]->acct_server_current_use == RADIUS_MASTER){
			if((iden != 32768) && (iden >=0) && (iden <256)){
				c_test = wasd->conf->c_test_window_acct;
				wasd->conf->radius_acct_heart_fail_num++;
				i = (wasd->conf->radius_acct_heart_req_num-1)%wasd->conf->c_window_size;
				if(c_test[i] == RADIUS_TEST_FAIL)
					wasd->conf->radius_acct_heart_fail_num--;
				else if(c_test[i] == RADIUS_TEST_SUC)
					wasd->conf->radius_acct_heart_res_num--;
				c_test[i] = RADIUS_TEST_FAIL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"acct_fail is %d,acct_suc is %d,acct_request is %d\n",wasd->conf->radius_acct_heart_fail_num,wasd->conf->radius_acct_heart_res_num,wasd->conf->radius_acct_heart_req_num);
				if((((double)wasd->conf->radius_acct_heart_fail_num)/wasd->conf->c_window_size) > wasd->conf->radius_res_fail_percent){
					radius_server_change(SID,RADIUS_ACCT_TEST,RADIUS_BAK);
				}
			}
		}
		else if(ASD_SECURITY[SID]->acct_server_current_use == RADIUS_BAK){
			if((iden != 32768) && (iden >=0) && (iden <256)){
				r_test = wasd->conf->r_test_window_acct;
				wasd->conf->radius_acct_heart_fail_num++;
				i = (wasd->conf->radius_acct_heart_req_num-1)%wasd->conf->r_window_size;
				if(r_test[i] == RADIUS_TEST_FAIL)
					wasd->conf->radius_acct_heart_fail_num--;
				else if(r_test[i] == RADIUS_TEST_SUC)
					wasd->conf->radius_acct_heart_res_num--;
				r_test[i] = RADIUS_TEST_FAIL;
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_fail:acct_fail is %d,acct_suc is %d,acct_request is %d\n",wasd->conf->radius_acct_heart_fail_num,wasd->conf->radius_acct_heart_res_num,wasd->conf->radius_acct_heart_req_num);
			}
			else{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"bak_suc:acct_fail is %d,acct_suc is %d,acct_request is %d\n",wasd->conf->radius_acct_heart_fail_num,wasd->conf->radius_acct_heart_res_num,wasd->conf->radius_acct_heart_req_num);
				if((((double)wasd->conf->radius_acct_heart_res_num)/wasd->conf->r_window_size) > wasd->conf->radius_res_suc_percent)
					radius_server_change(SID,RADIUS_ACCT_TEST,RADIUS_MASTER);
			}
		}
		RADIUS_PACKAGESEND_ACCT(wasd);
		circle_register_timeout(heart_test_interval,0,radius_encapsulate_heart_test_radius,wasd,&(ASD_SECURITY[SID]->radius_acct));
	}
	else if (*type == RADIUS_BOTH_TEST){
		if(ASD_SECURITY[SID]->radius_server_binding_type == RADIUS_SERVER_BINDED){
			if((ASD_SECURITY[SID]->acct_server_current_use == RADIUS_MASTER) && (ASD_SECURITY[SID]->auth_server_current_use == RADIUS_MASTER)){
				iden = ASD_SECURITY[SID]->heart_test_identifier_auth;
				if((iden != 32768) && (iden >=0) && (iden <256)){
					c_test = wasd->conf->c_test_window_acct;
					wasd->conf->radius_acct_heart_fail_num++;
					i = (wasd->conf->radius_acct_heart_req_num-1)%wasd->conf->c_window_size;
					if(c_test[i] == RADIUS_TEST_FAIL)
						wasd->conf->radius_acct_heart_fail_num--;
					else if(c_test[i] == RADIUS_TEST_SUC)
						wasd->conf->radius_acct_heart_res_num--;
					c_test[i] = RADIUS_TEST_FAIL;
				}
				double c_fail_auth = ((double)wasd->conf->radius_acct_heart_fail_num)/wasd->conf->c_window_size;
				iden = ASD_SECURITY[SID]->heart_test_identifier_acct;
				if((iden != 32768) && (iden >=0) && (iden <256)){
					c_test = wasd->conf->c_test_window_acct;
					wasd->conf->radius_acct_heart_fail_num++;
					i = (wasd->conf->radius_acct_heart_req_num-1)%wasd->conf->c_window_size;
					if(c_test[i] == RADIUS_TEST_FAIL)
						wasd->conf->radius_acct_heart_fail_num--;
					else if(c_test[i] == RADIUS_TEST_SUC)
						wasd->conf->radius_acct_heart_res_num--;
					c_test[i] = RADIUS_TEST_FAIL;
				}
				double c_fail_acct = ((double)wasd->conf->radius_acct_heart_fail_num)/wasd->conf->c_window_size;
				if((c_fail_auth > wasd->conf->radius_res_fail_percent) || (c_fail_acct > wasd->conf->radius_res_fail_percent)){
						ret = radius_change_server_both(SID,RADIUS_BAK);
						if(ret)
							asd_printf(ASD_80211,MSG_DEBUG,"%s:radius change server both fail!\n",__func__);
						else{
							update_radius_state_to_bakAC(SID,RADIUS_BAK,RADIUS_BAK);
							ASD_SECURITY[SID]->acct_server_current_use = RADIUS_BAK;
							ASD_SECURITY[SID]->auth_server_current_use = RADIUS_BAK;
							wasd->conf->radius_auth_heart_fail_num = 0;
							wasd->conf->radius_auth_heart_req_num = 0;
							wasd->conf->radius_auth_heart_res_num = 0;
							memset(wasd->conf->c_test_window_auth,0,wasd->conf->c_window_size);
							wasd->conf->radius_acct_heart_fail_num = 0;
							wasd->conf->radius_acct_heart_req_num = 0;
							wasd->conf->radius_acct_heart_res_num = 0;
							memset(wasd->conf->c_test_window_acct,0,wasd->conf->c_window_size);
						}
				}
			}
			else if((ASD_SECURITY[SID]->acct_server_current_use == RADIUS_BAK) && (ASD_SECURITY[SID]->auth_server_current_use == RADIUS_BAK)){
				iden = ASD_SECURITY[SID]->heart_test_identifier_auth;
				if((iden != 32768) && (iden >=0) && (iden <256)){
					r_test = wasd->conf->r_test_window_auth;
					wasd->conf->radius_auth_heart_fail_num++;
					i = (wasd->conf->radius_auth_heart_req_num-1)%wasd->conf->r_window_size;
					if(r_test[i] == RADIUS_TEST_FAIL)
						wasd->conf->radius_auth_heart_fail_num--;
					else if(r_test[i] == RADIUS_TEST_SUC)
						wasd->conf->radius_auth_heart_res_num--;
					r_test[i] = RADIUS_TEST_FAIL;
				}
				double r_suc_auth = ((double)wasd->conf->radius_auth_heart_res_num)/wasd->conf->r_window_size;
				iden = ASD_SECURITY[SID]->heart_test_identifier_acct;
				if((iden != 32768) && (iden >=0) && (iden <256)){
					r_test = wasd->conf->r_test_window_acct;
					wasd->conf->radius_acct_heart_fail_num++;
					i = (wasd->conf->radius_acct_heart_req_num-1)%wasd->conf->r_window_size;
					if(r_test[i] == RADIUS_TEST_FAIL)
						wasd->conf->radius_acct_heart_fail_num--;
					else if(r_test[i] == RADIUS_TEST_SUC)
						wasd->conf->radius_acct_heart_res_num--;
					r_test[i] = RADIUS_TEST_FAIL;
				}
				double r_suc_acct = ((double)wasd->conf->radius_acct_heart_res_num)/wasd->conf->r_window_size;
				if((r_suc_auth > wasd->conf->radius_res_suc_percent)&&(r_suc_acct > wasd->conf->radius_res_suc_percent)){
					ret = radius_change_server_both(SID,RADIUS_MASTER);
					if(ret)
						asd_printf(ASD_80211,MSG_DEBUG,"%s:radius change server both fail!\n",__func__);
					else{
						update_radius_state_to_bakAC(SID,RADIUS_MASTER,RADIUS_MASTER);
						ASD_SECURITY[SID]->acct_server_current_use = RADIUS_MASTER;
						ASD_SECURITY[SID]->auth_server_current_use = RADIUS_MASTER;
						wasd->conf->radius_auth_heart_fail_num = 0;
						wasd->conf->radius_auth_heart_req_num = 0;
						wasd->conf->radius_auth_heart_res_num = 0;
						memset(wasd->conf->r_test_window_auth,0,wasd->conf->r_window_size);
						wasd->conf->radius_acct_heart_fail_num = 0;
						wasd->conf->radius_acct_heart_req_num = 0;
						wasd->conf->radius_acct_heart_res_num = 0;
						memset(wasd->conf->r_test_window_acct,0,wasd->conf->r_window_size);
					}
				}
			}else
				asd_printf(ASD_80211,MSG_DEBUG,"%s:The radius server is binding but the servers using type is not the same!\n",__func__);
			RADIUS_PACKAGESEND_ACCT(wasd);
			RADIUS_PACKAGESEND_AUTH(wasd);
			circle_register_timeout(heart_test_interval,0,radius_encapsulate_heart_test_radius,wasd,&(ASD_SECURITY[SID]->radius_both));
		}
		else {
			radius_encapsulate_heart_test_radius(wasd,&(ASD_SECURITY[SID]->radius_auth));
			radius_encapsulate_heart_test_radius(wasd,&(ASD_SECURITY[SID]->radius_acct));
		}
	}
	//return;
		//if(is_secondary != 1)
			//circle_register_timeout(heart_test_interval,0,radius_encapsulate_heart_test_radius,wasd,type);
}
void ieee802_1x_encapsulate_dm_radius(struct radius_coa_info *coa_info)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"func:%s \n",__func__);
	struct radius_msg *msg = NULL;	
	if(!coa_info||!coa_info->msg)
	{
		asd_printf(ASD_1X,MSG_WARNING,"%s: coa info is error!\n",__func__);
		return;
	}
	msg = radius_msg_new(coa_info->code,
				 coa_info->msg->hdr->identifier);
	if (msg == NULL) {
		asd_printf(ASD_1X,MSG_WARNING,"Could not create net RADIUS packet\n");
		return;
	}	
	if(radius_msg_add_attr_int32(msg,RADIUS_ATTR_ERROR_CAUSE,coa_info->error_code) < 0 )
	{
		asd_printf(ASD_1X,MSG_WARNING,"Could not add error code in dm radius packet!\n");
		goto fail;
	}	
		
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"msg->hdr->code : %d,length:%d,buf_used:%d\n",msg->hdr ->code,msg->hdr ->length,msg->buf_used);
	msg->hdr->length = msg->buf_used;
	radius_msg_make_resp_authenticator(msg, coa_info->msg,coa_info->secret,coa_info->secret_len);
	radius_client_send_dm_message(msg,coa_info);
	//has no return because of the msg need to free;
fail:
	radius_msg_free(msg);
	msg = NULL;
	return;

}

//weichao add for process disconnect message
void asd_radius_coa_request(struct radius_msg *msg,unsigned int radius_ip,int radius_port)
{
	struct sta_info *sta = NULL;	
	struct asd_data *wasd = NULL;
	unsigned char acct_id[256] = {0};
	unsigned char user_name[256] = {0};
	struct radius_coa_info coa_info;
	struct sta_acct_info *acct_info;
	struct asd_ip_secret *ip_secret;
	u8 *secret = NULL;
	size_t secret_len = 0;
	if (radius_msg_get_attr(msg, RADIUS_ATTR_ACCT_SESSION_ID,
				      acct_id,sizeof(acct_id)) < 0)
	{	
		asd_printf(ASD_1X,MSG_ERROR,"func:%s : has not get session id!drop it\n",__func__);
		return;
	}	
	
	//there is user name in message,but we don't use current
	if(radius_msg_get_attr(msg,RADIUS_ATTR_USER_NAME,user_name,sizeof(user_name))  < 0)
	{
		asd_printf(ASD_1X,MSG_ERROR,"func:%s : has not get user name !drop it\n",__func__);
		return;
	}
	asd_printf(ASD_1X,MSG_DEBUG,"%s : receive username:%s\n",__func__,user_name);
	asd_printf(ASD_1X,MSG_DEBUG,"%s : receive acct_id:%s\n",__func__,acct_id);
	asd_printf(ASD_1X,MSG_DEBUG,"will get ip_secret by radius_ip(%d)\n",radius_ip);
	ip_secret = asd_ip_secret_get(radius_ip);
	if(ip_secret == NULL)
	{	
		asd_printf(ASD_1X,MSG_INFO,"has not get ip_secret by radius_ip and port,drop the packet!\n");
		return;
	}
	secret = ip_secret->shared_secret;
	secret_len = ip_secret->shared_secret_len;			

	acct_info = sta_acct_info_get(acct_id);
	if(NULL != acct_info)
	{
		sta = asd_sta_hash_get(acct_info->addr);
	}
	if(NULL == sta)
	{
		asd_printf(ASD_1X,MSG_DEBUG,"%s:sta acct_id(%s) is not found!\n",__func__,acct_id);
	}
	if(0 != radius_request_check(msg,secret,secret_len))
	{
		asd_printf(ASD_1X,MSG_INFO,"%s : authenticator not match secret(%s) ,dropped it!\n",__func__,secret);
		return;
	}
	
	coa_info.msg = msg;
	coa_info.ip = radius_ip;
	coa_info.port = radius_port;
	coa_info.secret = secret;
	coa_info.secret_len = secret_len;
	coa_info.slot_value = ip_secret->slot_value;
	coa_info.inst_value = ip_secret->inst_value;
	switch(msg->hdr->code){
	case RADIUS_CODE_DISCONNECT_REQUEST:
		 if(sta != NULL)
		 {		 	
			ap_kick_eap_sta(sta);
			coa_info.code = RADIUS_CODE_DISCONNECT_ACK;
			coa_info.error_code = RADIUS_ERROR_CAUSE_201;
		 }
		else
		 {
			coa_info.code = RADIUS_CODE_DISCONNECT_NAK;
			coa_info.error_code = RADIUS_ERROR_CAUSE_503;
		 }
		break;
	case RADIUS_CODE_COA_REQUEST:
		 if(sta  != NULL )
		 {
			 ap_kick_eap_sta(sta);
			 coa_info.code = RADIUS_CODE_COA_ACK;
			 coa_info.error_code = RADIUS_ERROR_CAUSE_201;
		 }
		else
		 {
			coa_info.code = RADIUS_CODE_COA_NAK;
			coa_info.error_code = RADIUS_ERROR_CAUSE_503;
		 }
		
		break;
	default:
		asd_printf(ASD_1X,MSG_ERROR," %s: the dm request code is unknow!\n",__func__);
		return;
	}
	ieee802_1x_encapsulate_dm_radius(&coa_info);

//set the point null
	sta = NULL;
	wasd = NULL;
	secret = NULL;
}
void eap_sm_run_cancel_timer(unsigned char securityid)
{
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	struct asd_data *wasd = NULL;
	struct sta_info *sta = NULL;
	unsigned int i = 4,j = 0;
	unsigned char SID = 0;
	if(interfaces != NULL){
		for(i=4;i<G_RADIO_NUM;i++){
			if(interfaces->iface[i]){
				for(j=0;j<L_BSS_NUM;j++){
					wasd = interfaces->iface[i]->bss[j];
					if(wasd){
						SID = wasd->SecurityID;
						if(SID == securityid)
							sta = wasd->sta_list;
						while(sta){
							if(sta->eapol_sm != NULL)
								circle_cancel_timeout(eapol_port_timers_tick,NULL,sta->eapol_sm);
							sta = sta->next;
						}
					}
				}
			}
		}
	}
}


