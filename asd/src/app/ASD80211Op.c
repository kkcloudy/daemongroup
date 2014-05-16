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
* Asd80211.c
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


#include <net/if.h>

#include "circle.h"
#include "asd.h"
#include "ASD80211Op.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDRadius/radius.h"
#include "ASDRadius/radius_client.h"
#include "ASDEapolSM.h"
#include "ASD80211AuthOp.h"
#include "ASDStaInfo.h"
#include "rc4.h"
#include "ASD8021XOp.h"
#include "ASDWPAOp.h"
#include "ASDWme.h"
#include "ASDApList.h"
#include "ASDAccounting.h"
#include "ASDCallback.h"
#include "ASD80211HOp.h"
#include "ASDMlme.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "asd_bak.h"

#include "include/cert_auth.h"
#include "ASDDbus.h"
#include "StaFlashDisconnOp.h"
#include "roaming_sta.h"
#include "syslog.h"//Qc
#include "time.h"
#include "ASDPMKSta.h"
#define ASD_STA_EAP_AUTH_TIME 300
int WTP_SEND_RESPONSE_TO_MOBILE=0;//nl1010
unsigned int IEEE_80211N_ENABLE = 0;

extern struct acl_config ac_acl_conf;
extern unsigned char gASDLOGDEBUG;//qiuchen
extern unsigned long gASD_AC_MANAGEMENT_IP;

u8 * asd_eid_ht_capabilities_info(struct asd_data *hapd, u8 *eid)
{
#ifdef ASD_IEEE80211N
	struct ieee80211_ht_capability *cap;
	u8 *pos = eid;

	if (!hapd->iconf->ieee80211n)
		return eid;

	*pos++ = WLAN_EID_HT_CAP;
	*pos++ = sizeof(*cap);

	cap = (struct ieee80211_ht_capability *) pos;
	os_memset(cap, 0, sizeof(*cap));
	SET_2BIT_U8(&cap->mac_ht_params_info,
		    MAC_HT_PARAM_INFO_MAX_RX_AMPDU_FACTOR_OFFSET,
		    MAX_RX_AMPDU_FACTOR_64KB);

	cap->capabilities_info = host_to_le16(hapd->iconf->ht_capab);

	cap->supported_mcs_set[0] = 0xff;
	cap->supported_mcs_set[1] = 0xff;

 	pos += sizeof(*cap);

	return pos;
#else /* ASD_IEEE80211N */
	return eid;
#endif /* ASD_IEEE80211N */
}


u8 * asd_eid_ht_operation(struct asd_data *hapd, u8 *eid)
{
#ifdef ASD_IEEE80211N
	struct ieee80211_ht_operation *oper;
	u8 *pos = eid;

	if (!hapd->iconf->ieee80211n)
		return eid;

	*pos++ = WLAN_EID_HT_OPERATION;
	*pos++ = sizeof(*oper);

	oper = (struct ieee80211_ht_operation *) pos;
	os_memset(oper, 0, sizeof(*oper));

	oper->operation_mode = host_to_le16(hapd->iface->ht_op_mode);

	pos += sizeof(*oper);

	return pos;
#else /* ASD_IEEE80211N */
	return eid;
#endif /* ASD_IEEE80211N */
}


#ifdef ASD_IEEE80211N

/*
op_mode
Set to 0 (HT pure) under the followign conditions
	- all STAs in the BSS are 20/40 MHz HT in 20/40 MHz BSS or
	- all STAs in the BSS are 20 MHz HT in 20 MHz BSS
Set to 1 (HT non-member protection) if there may be non-HT STAs
	in both the primary and the secondary channel
Set to 2 if only HT STAs are associated in BSS,
	however and at least one 20 MHz HT STA is associated
Set to 3 (HT mixed mode) when one or more non-HT STAs are associated
	(currently non-GF HT station is considered as non-HT STA also)
*/
int asd_ht_operation_update(struct asd_iface *iface)
{
	u16 cur_op_mode, new_op_mode;
	int op_mode_changes = 0;

	if (!iface->conf->ieee80211n || iface->conf->ht_op_mode_fixed)
		return 0;

	asd_printf(ASD_80211,MSG_DEBUG, "%s current operation mode=0x%X",
		   __func__, iface->ht_op_mode);

	if (!(iface->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT)
	    && iface->num_sta_ht_no_gf) {
		iface->ht_op_mode |=
			HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	} else if ((iface->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT) &&
		   iface->num_sta_ht_no_gf == 0) {
		iface->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT;
		op_mode_changes++;
	}

	if (!(iface->ht_op_mode & HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
	    (iface->num_sta_no_ht || iface->olbc_ht)) {
		iface->ht_op_mode |= HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	} else if ((iface->ht_op_mode &
		    HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT) &&
		   (iface->num_sta_no_ht == 0 && !iface->olbc_ht)) {
		iface->ht_op_mode &=
			~HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT;
		op_mode_changes++;
	}

	/* Note: currently we switch to the MIXED op mode if HT non-greenfield
	 * station is associated. Probably it's a theoretical case, since
	 * it looks like all known HT STAs support greenfield.
	 */
	new_op_mode = 0;
	if (iface->num_sta_no_ht ||
	    (iface->ht_op_mode & HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT))
		new_op_mode = OP_MODE_MIXED;
	else if ((iface->conf->ht_capab & HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET)
		 && iface->num_sta_ht_20mhz)
		new_op_mode = OP_MODE_20MHZ_HT_STA_ASSOCED;
	else if (iface->olbc_ht)
		new_op_mode = OP_MODE_MAY_BE_LEGACY_STAS;
	else
		new_op_mode = OP_MODE_PURE;

	cur_op_mode = iface->ht_op_mode & HT_INFO_OPERATION_MODE_OP_MODE_MASK;
	if (cur_op_mode != new_op_mode) {
		iface->ht_op_mode &= ~HT_INFO_OPERATION_MODE_OP_MODE_MASK;
		iface->ht_op_mode |= new_op_mode;
		op_mode_changes++;
	}

	asd_printf(ASD_80211,MSG_DEBUG, "%s new operation mode=0x%X changes=%d",
		   __func__, iface->ht_op_mode, op_mode_changes);

	return op_mode_changes;
}

#endif /* ASD_IEEE80211N */


u8 * asd_eid_supp_rates(struct asd_data *wasd, u8 *eid)
{
	u8 *pos = eid;
	int i, num, count;

	if (wasd->iface->current_rates == NULL)
		return eid;
	*pos++ = WLAN_EID_SUPP_RATES;
	num = wasd->iface->num_rates;
	
	if (num > 8) {
		/* rest of the rates are encoded in Extended supported
		 * rates element */
		num = 8;
	}

	*pos++ = num;
	count = 0;
	for (i = 0, count = 0; i < wasd->iface->num_rates && count < num;
	     i++) {
		count++;		
		*pos = wasd->iface->current_rates[i].rate / 5;
		if (wasd->iface->current_rates[i].flags & asd_RATE_BASIC)
			*pos |= 0x80;
		pos++;
	}

	return pos;
}


u8 * asd_eid_ext_supp_rates(struct asd_data *wasd, u8 *eid)
{
	u8 *pos = eid;
	int i, num, count;

	if (wasd->iface->current_rates == NULL)
		return eid;

	num = wasd->iface->num_rates;
	if (num <= 8)
		return eid;
	num -= 8;

	*pos++ = WLAN_EID_EXT_SUPP_RATES;
	*pos++ = num;
	count = 0;
	for (i = 0, count = 0; i < wasd->iface->num_rates && count < num + 8;
	     i++) {
		count++;
		if (count <= 8)
			continue; /* already in SuppRates IE */
		*pos = wasd->iface->current_rates[i].rate / 5;
		if (wasd->iface->current_rates[i].flags & asd_RATE_BASIC)
			*pos |= 0x80;
		pos++;
	}

	return pos;
}


u16 asd_own_capab_info(struct asd_data *wasd, struct sta_info *sta,
			   int probe)
{
	int capab = WLAN_CAPABILITY_ESS;
	int privacy;

	if (wasd->iface->num_sta_no_short_preamble == 0 &&
	    wasd->iconf->preamble == SHORT_PREAMBLE)
		capab |= WLAN_CAPABILITY_SHORT_PREAMBLE;

	privacy = wasd->conf->ssid.wep.keys_set;

	if (wasd->conf->ieee802_1x &&
	    (wasd->conf->default_wep_key_len ||
	     wasd->conf->individual_wep_key_len))
		privacy = 1;

	if (wasd->conf->wpa)
		privacy = 1;

	if (sta) {
		int policy, def_klen;
		if (probe && sta->ssid_probe) {
			policy = sta->ssid_probe->security_policy;
			def_klen = sta->ssid_probe->wep.default_len;
		} else {
			policy = sta->ssid->security_policy;
			def_klen = sta->ssid->wep.default_len;
		}
		privacy = policy != SECURITY_PLAINTEXT;
		if (policy == SECURITY_IEEE_802_1X && def_klen == 0)
			privacy = 0;
	}

	if (privacy)
		capab |= WLAN_CAPABILITY_PRIVACY;

	if (wasd->iface->current_mode &&
	    wasd->iface->current_mode->mode == asd_MODE_IEEE80211G &&
	    wasd->iface->num_sta_no_short_slot_time == 0)
		capab |= WLAN_CAPABILITY_SHORT_SLOT_TIME;

	if (wasd->iface->dfs_enable) 
		capab |= WLAN_CAPABILITY_SPECTRUM_MGMT;

	return capab;
}


#define OUI_MICROSOFT 0x0050f2 /* Microsoft (also used in Wi-Fi specs)
				* 00:50:F2 */
				
#define OUI_BROADCOM 0x00904c /* Broadcom (Epigram) */
	
#define VENDOR_HT_CAPAB_OUI_TYPE 0x33 /* 00-90-4c:0x33 */

static int ieee802_11_parse_vendor_specific(struct asd_data *wasd,
					    u8 *pos, size_t elen,
					    struct ieee802_11_elems *elems,
					    int show_errors)
{
	unsigned int oui;

	/* first 3 bytes in vendor specific information element are the IEEE
	 * OUI of the vendor. The following byte is used a vendor specific
	 * sub-type. */
	if (elen < 4) {
		if (show_errors) {
			asd_printf(ASD_80211,MSG_MSGDUMP, "short vendor specific "
				   "information element ignored (len=%lu)",
				   (unsigned long) elen);
		}
		return -1;
	}

	oui = WPA_GET_BE24(pos);
	switch (oui) {
	case OUI_MICROSOFT:
		/* Microsoft/Wi-Fi information elements are further typed and
		 * subtyped */
		switch (pos[3]) {
		case 1:
			/* Microsoft OUI (00:50:F2) with OUI Type 1:
			 * real WPA information element */
			elems->wpa_ie = pos;
			elems->wpa_ie_len = elen;
			break;
		case WME_OUI_TYPE: /* this is a Wi-Fi WME info. element */
			if (elen < 5) {
				asd_printf(ASD_80211,MSG_MSGDUMP, "short WME "
					   "information element ignored "
					   "(len=%lu)",
					   (unsigned long) elen);
				return -1;
			}
			switch (pos[4]) {
			case WME_OUI_SUBTYPE_INFORMATION_ELEMENT:
			case WME_OUI_SUBTYPE_PARAMETER_ELEMENT:
				elems->wme = pos;
				elems->wme_len = elen;
				break;
			case WME_OUI_SUBTYPE_TSPEC_ELEMENT:
				elems->wme_tspec = pos;
				elems->wme_tspec_len = elen;
				break;
			default:
				asd_printf(ASD_80211,MSG_MSGDUMP, "unknown WME "
					   "information element ignored "
					   "(subtype=%d len=%lu)",
					   pos[4], (unsigned long) elen);
				return -1;
			}
			break;
		default:
			asd_printf(ASD_80211,MSG_MSGDUMP, "Unknown Microsoft "
				   "information element ignored "
				   "(type=%d len=%lu)\n",
				   pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	case OUI_BROADCOM:
		asd_printf(ASD_80211,MSG_DEBUG, "OUI_BROADCOM");
		switch (pos[3]) {
		case VENDOR_HT_CAPAB_OUI_TYPE:
			elems->vendor_ht_cap = pos;
			elems->vendor_ht_cap_len = elen;
			break;
		default:
			asd_printf(ASD_80211,MSG_MSGDUMP, "Unknown Broadcom "
				   "information element ignored "
				   "(type=%d len=%lu)\n",
				   pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	default:
		asd_printf(ASD_80211,MSG_MSGDUMP, "unknown vendor specific information "
			   "element ignored (vendor OUI %02x:%02x:%02x "
			   "len=%lu)",
			   pos[0], pos[1], pos[2], (unsigned long) elen);
		return -1;
	}

	return 0;
}


ParseRes ieee802_11_parse_elems(struct asd_data *wasd, u8 *start,
				size_t len,
				struct ieee802_11_elems *elems,
				int show_errors)
{
	size_t left = len;
	u8 *pos = start;
	int unknown = 0;

	os_memset(elems, 0, sizeof(*elems));

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			if (show_errors) {
				asd_printf(ASD_80211,MSG_DEBUG, "IEEE 802.11 element "
					   "parse failed (id=%d elen=%d "
					   "left=%lu)",
					   id, elen, (unsigned long) left);
				wpa_hexdump(MSG_MSGDUMP, "IEs", start, len);
			}
			return ParseFailed;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			break;
		case WLAN_EID_FH_PARAMS:
			elems->fh_params = pos;
			elems->fh_params_len = elen;
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
		case WLAN_EID_CF_PARAMS:
			elems->cf_params = pos;
			elems->cf_params_len = elen;
			break;
		case WLAN_EID_TIM:
			elems->tim = pos;
			elems->tim_len = elen;
			break;
		case WLAN_EID_IBSS_PARAMS:
			elems->ibss_params = pos;
			elems->ibss_params_len = elen;
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (ieee802_11_parse_vendor_specific(wasd, pos, elen,
							     elems,
							     show_errors))
				unknown++;
			break;
		case WLAN_EID_RSN:
			elems->rsn_ie = pos;
			elems->rsn_ie_len = elen;
			break;
		case WLAN_EID_PWR_CAPABILITY:
			elems->power_cap = pos;
			elems->power_cap_len = elen;
			break;
		case WLAN_EID_SUPPORTED_CHANNELS:
			elems->supp_channels = pos;
			elems->supp_channels_len = elen;
			break;
		case WLAN_EID_MOBILITY_DOMAIN:
			elems->mdie = pos;
			elems->mdie_len = elen;
			break;
		case WLAN_EID_FAST_BSS_TRANSITION:
			elems->ftie = pos;
			elems->ftie_len = elen;
			break;
		case IEEE80211_ELEMID_WAPI:			
			elems->rsn_ie = pos;
			elems->rsn_ie_len = elen;
			break;
		case WLAN_EID_TIMEOUT_INTERVAL:
			elems->timeout_int = pos;
			elems->timeout_int_len = elen;
			break;
		case WLAN_EID_HT_CAP:
			elems->ht_capabilities = pos;
			elems->ht_capabilities_len = elen;
			break;
		case WLAN_EID_HT_OPERATION:
			elems->ht_operation = pos;
			elems->ht_operation_len = elen;
			break;
		default:
			unknown++;
			if (!show_errors)
				break;
			asd_printf(ASD_80211,MSG_MSGDUMP, "IEEE 802.11 element parse "
				   "ignored unknown element (id=%d elen=%d)",
				   id, elen);
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return unknown ? ParseUnknown : ParseOK;
}


void ieee802_11_print_ssid(char *buf, const u8 *ssid, u8 len)
{
	int i;
	if (len > asd_MAX_SSID_LEN)
		len = asd_MAX_SSID_LEN;
	for (i = 0; i < len; i++) {
		if (ssid[i] >= 32 && ssid[i] < 127)
			buf[i] = ssid[i];
		else
			buf[i] = '.';
	}
	buf[len] = '\0';
}


void ieee802_11_send_deauth(struct asd_data *wasd, u8 *addr, u16 reason)
{
	struct ieee80211_mgmt mgmt;
	char buf[30];
	
	//qiuchen
	u8 WTPMAC[MAC_LEN] = {0};
	unsigned int wtpid = 0;
	if(ASD_WTP_AP[wasd->Radio_G_ID/4]){
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
		wtpid = ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPID;
	}
	//end
	asd_printf(ASD_80211,MSG_DEBUG,"func: %s send_deauth to STA_MAC: %s reason:%d\n",__func__,addr,reason);

	asd_logger(wasd, addr, asd_MODULE_IEEE80211,asd_LEVEL_NOTICE,"deauthenticate - reason %d", reason);
	if(gASDLOGDEBUG & BIT(1))
		syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]DEAUTH:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d, ErrorCode:%d.\n",
		slotid,vrrid,MAC2STR(addr),MAC2STR(WTPMAC),wasd->BSSIndex,reason);//qiuchen 2013.01.14
	//asd_syslog_h("BAC-M WMAC/6/WMAC_CLIENT_DEAUTH_WLAN","Client" MACSTR "successfully joins WLAN %s,on APID %d with BSSID" MACSTR".\n",MAC2STR(addr),SSID,wtpid,wasd->own_addr);
	//if(gASDLOGDEBUG & BIT(0))
		//asd_syslog_h("BAC-M WMAC/6/WMAC_CLIENT_DEAUTH_WLAN","Client "MAC_ADDRESS" successfully deauth WLAN %s,on APID %d with BSSID "MAC_ADDRESS".\n",MAC2STR(addr),SSID,wtpid,MAC2STR(wasd->own_addr));
	os_snprintf(buf, sizeof(buf), "SEND-DEAUTHENTICATE %d", reason);
	os_memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DEAUTH);
	os_memcpy(mgmt.da, addr, ETH_ALEN);
	os_memcpy(mgmt.sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(mgmt.bssid, wasd->own_addr, ETH_ALEN);
	mgmt.u.deauth.reason_code = host_to_le16(reason);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, &mgmt, IEEE80211_HDRLEN + sizeof(mgmt.u.deauth), IEEE802_11_MGMT);
	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("ieee802_11_send_deauth: send");
}


static void ieee802_11_sta_authenticate(void *circle_ctx, void *timeout_ctx)
{
	asd_printf(ASD_80211,MSG_DEBUG,"func: %s\n",__func__);
	struct asd_data *wasd = circle_ctx;
	struct ieee80211_mgmt mgmt;
	char ssid_txt[33];

	if (wasd->assoc_ap_state == WAIT_BEACON)
		wasd->assoc_ap_state = AUTHENTICATE;
	if (wasd->assoc_ap_state != AUTHENTICATE)
		return;

	ieee802_11_print_ssid(ssid_txt, (u8 *) wasd->assoc_ap_ssid,
			      wasd->assoc_ap_ssid_len);
	asd_printf(ASD_80211,MSG_DEBUG,"Authenticate with AP " MACSTR " SSID=%s (as station)\n",
	       MAC2STR(wasd->conf->assoc_ap_addr), ssid_txt);

	os_memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_AUTH);
	/* Request TX callback */
	mgmt.frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	os_memcpy(mgmt.da, wasd->conf->assoc_ap_addr, ETH_ALEN);
	os_memcpy(mgmt.sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(mgmt.bssid, wasd->conf->assoc_ap_addr, ETH_ALEN);
	mgmt.u.auth.auth_alg = host_to_le16(WLAN_AUTH_OPEN);
	mgmt.u.auth.auth_transaction = host_to_le16(1);
	mgmt.u.auth.status_code = host_to_le16(0);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, &mgmt, IEEE80211_HDRLEN + sizeof(mgmt.u.auth), IEEE802_11_MGMT);
	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("ieee802_11_sta_authenticate: send");

	/* Try to authenticate again, if this attempt fails or times out. */
	circle_register_timeout(5, 0, ieee802_11_sta_authenticate, wasd, NULL);
}


static void ieee802_11_sta_associate(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	u8 buf[256];
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;
	u8 *p;
	char ssid_txt[33];

	if (wasd->assoc_ap_state == AUTHENTICATE)
		wasd->assoc_ap_state = ASSOCIATE;
	if (wasd->assoc_ap_state != ASSOCIATE)
		return;

	ieee802_11_print_ssid(ssid_txt, (u8 *) wasd->assoc_ap_ssid,
			      wasd->assoc_ap_ssid_len);
	asd_printf(ASD_80211,MSG_DEBUG,"Associate with AP " MACSTR " SSID=%s (as station)\n",
	       MAC2STR(wasd->conf->assoc_ap_addr), ssid_txt);

	os_memset(mgmt, 0, sizeof(*mgmt));
	mgmt->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_ASSOC_REQ);
	/* Request TX callback */
	mgmt->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	os_memcpy(mgmt->da, wasd->conf->assoc_ap_addr, ETH_ALEN);
	os_memcpy(mgmt->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(mgmt->bssid, wasd->conf->assoc_ap_addr, ETH_ALEN);
	mgmt->u.assoc_req.capab_info = host_to_le16(0);
	mgmt->u.assoc_req.listen_interval = host_to_le16(1);
	p = &mgmt->u.assoc_req.variable[0];

	*p++ = WLAN_EID_SSID;
	*p++ = wasd->assoc_ap_ssid_len;
	os_memcpy(p, wasd->assoc_ap_ssid, wasd->assoc_ap_ssid_len);
	p += wasd->assoc_ap_ssid_len;

	p = asd_eid_supp_rates(wasd, p);
	p = asd_eid_ext_supp_rates(wasd, p);

	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, mgmt, p - (u8 *) mgmt, IEEE802_11_MGMT);

	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("ieee802_11_sta_associate: send");

	/* Try to authenticate again, if this attempt fails or times out. */
	circle_register_timeout(5, 0, ieee802_11_sta_associate, wasd, NULL);
}


static u16 auth_shared_key(struct asd_data *wasd, struct sta_info *sta,
			   u16 auth_transaction, u8 *challenge, int iswep)
{
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "authentication (shared key, transaction %d)",
		       auth_transaction);

	if (auth_transaction == 1) {
		if (!sta->challenge) {
			/* Generate a pseudo-random challenge */
			u8 key[8];
			time_t now;
			int r;
			sta->challenge = os_zalloc(WLAN_AUTH_CHALLENGE_LEN);
			if (sta->challenge == NULL)
				return WLAN_STATUS_UNSPECIFIED_FAILURE;

			now = time(NULL);
			r = random();
			os_memcpy(key, &now, 4);
			os_memcpy(key + 4, &r, 4);
			rc4(sta->challenge, WLAN_AUTH_CHALLENGE_LEN,
			    key, sizeof(key));
		}
		return 0;
	}

	if (auth_transaction != 3)
		return WLAN_STATUS_UNSPECIFIED_FAILURE;

	/* Transaction 3 */
	if (!sta->challenge || !challenge ||
	    os_memcmp(sta->challenge, challenge, WLAN_AUTH_CHALLENGE_LEN)) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_INFO,
			       "shared key authentication - invalid "
			       "challenge-response");
		if(challenge)
			wpa_hexdump(MSG_DEBUG,"Challenge ",challenge,WLAN_AUTH_CHALLENGE_LEN);
		if(sta->challenge)
			wpa_hexdump(MSG_DEBUG,"STA Challenge ",sta->challenge,WLAN_AUTH_CHALLENGE_LEN);

		//mahz copy from version 2.0,2011.3.14
		//wasd->info->abort_key_error++;
		//return WLAN_STATUS_CHALLENGE_FAIL;
	}

	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "authentication OK (shared key)");
#ifdef IEEE80211_REQUIRE_AUTH_ACK
	/* Station will be marked authenticated if it ACKs the
	 * authentication reply. */
#else
	sta->flags |= WLAN_STA_AUTH;
	wpa_auth_sm_event(sta->wpa_sm, WPA_AUTH);
#endif
	os_free(sta->challenge);
	sta->challenge = NULL;

	return 0;
}


static void send_auth_reply(struct asd_data *wasd,
			    const u8 *dst, const u8 *bssid,
			    u16 auth_alg, u16 auth_transaction, u16 resp,
			    const u8 *ies, size_t ies_len)
{
	struct ieee80211_mgmt *reply;
	u8 *buf;
	size_t rlen;

	rlen = IEEE80211_HDRLEN + sizeof(reply->u.auth) + ies_len;
	buf = os_zalloc(rlen);
	if (buf == NULL)
		return;

	reply = (struct ieee80211_mgmt *) buf;
	reply->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					    WLAN_FC_STYPE_AUTH);
	/* Request TX callback */
	reply->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	os_memcpy(reply->da, dst, ETH_ALEN);
	os_memcpy(reply->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(reply->bssid, bssid, ETH_ALEN);
	reply->seq_ctrl = seq_to_le16(wasd->seq_num++);
	if(wasd->seq_num == 4096)
		wasd->seq_num = 0;

	reply->u.auth.auth_alg = host_to_le16(auth_alg);
	reply->u.auth.auth_transaction = host_to_le16(auth_transaction);
	reply->u.auth.status_code = host_to_le16(resp);

	if (ies && ies_len)
		os_memcpy(reply->u.auth.variable, ies, ies_len);

	asd_printf(ASD_80211,MSG_DEBUG, "authentication reply: STA=" MACSTR
		   " auth_alg=%d auth_transaction=%d resp=%d (IE len=%lu)",
		   MAC2STR(dst), auth_alg, auth_transaction,
		   resp, (unsigned long) ies_len);
	
	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, reply, rlen, IEEE802_11_MGMT);
	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("send_auth_reply: send");

	os_free(buf);
	buf=NULL;
}


#ifdef ASD_IEEE80211R
static void handle_auth_ft_finish(void *ctx, const u8 *dst, const u8 *bssid,
				  u16 auth_transaction, u16 status,
				  const u8 *ies, size_t ies_len)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta;

	send_auth_reply(wasd, dst, bssid, WLAN_AUTH_FT, auth_transaction,
			status, ies, ies_len);

	if (status != WLAN_STATUS_SUCCESS)
		return;

	sta = ap_get_sta(wasd, dst);
	if (sta == NULL)
		return;

	asd_logger(wasd, dst, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG, "authentication OK (FT)");
	sta->flags |= WLAN_STA_AUTH;
	mlme_authenticate_indication(wasd, sta);
}
#endif /* ASD_IEEE80211R */



static unsigned int 
check_arrival_counter(log_node  *nod){

	bss_arrival_num *bss_nod=NULL;
	unsigned int min=0,max=0;
	
	if(nod==NULL){
		asd_printf(ASD_80211,MSG_DEBUG,"nod==NULL in %s\n",__func__);
		return -1;
	}

	if(nod->from_bss_list==NULL||nod->list_len==0){
		asd_printf(ASD_80211,MSG_DEBUG,"nod->from_bss_list==NULL||nod->list_len==0 in %s\n",__func__);
		return -1;
	}

	bss_nod=nod->from_bss_list;

	min=max=bss_nod->arrival_num;

	while(bss_nod!=NULL){
		if(bss_nod->arrival_num<min){
			min=bss_nod->arrival_num;
		}

		if(bss_nod->arrival_num>max){
			max=bss_nod->arrival_num;
		}
		
		bss_nod=bss_nod->next;
	}

	asd_printf(ASD_80211,MSG_DEBUG,"max=%u  min=%u in %s\n",max,min,__func__);

	if(max>PROB_MAX_COUNTER||min>PROB_MIN_COUNTER){
		asd_printf(ASD_80211,MSG_DEBUG,"return 0 in %s\n",__func__);
		return 0;
	}
	else{
		asd_printf(ASD_80211,MSG_DEBUG,"return -1 in %s\n",__func__);
		return -1;
	}
}
/*blxm0714*/
/*	
	1	--	accept station.
	0	--	deney  station.
	-1	--	something error.
*/

unsigned int 
is_wtp_apply_wlan(unsigned int WTPID,unsigned char wlanid){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int i = 0, j=0;
	//unsigned int num = 0;

	for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
		if(interfaces->iface[i] == NULL)
			continue;
		for(j = 0; j < L_BSS_NUM; j++){
			if(interfaces->iface[i]->bss[j] == NULL)
				continue;
			else if(interfaces->iface[i]->bss[j]->WlanID==wlanid){
				return 1;
			}
			else{
				continue;
			}
		}
	}
	
	return 0;
}


/*blxm0714*/
/*	
	1	--	accept station.
	0	--	deney  station.
	-1	--	something error.
*/
static int flow_balance(struct asd_data *wasd,unsigned char mac[]){

	unsigned int cur_wtp_flow=0,ap_min_flow=0xffffffff;
	unsigned int wtpid=0;
	unsigned int i,ii;
	log_node  *nod=NULL;
	bss_arrival_num *bss_nod=NULL;
	
	if(!wasd||!mac){
		asd_printf(ASD_80211,MSG_DEBUG,"wasd==NULL mac==NULL in %s\n",__func__);
		return -1;
	}

	wtpid=wasd->Radio_G_ID/L_RADIO_NUM;

	if(!ASD_WTP_AP[wtpid]){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WTP_AP[%u]==NULL in %s\n",wtpid,__func__);
		return -1;
	}

	for(ii=0;ii<L_RADIO_NUM;ii++){
		cur_wtp_flow +=ASD_WTP_AP[wtpid]->radio_flow_info[ii].trans_rates;			
	}

	if(cur_wtp_flow<ASD_WTP_AP[wtpid]->wtp_flow_triger*1024){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: accept sta in wtp %u, bssindex %u\n"
			,__func__,wtpid,wasd->BSSIndex);
		return 1;
	}

	if(!ASD_WLAN[wasd->WlanID]){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[%u]==NULL in %s\n",wasd->WlanID,__func__);
		return -1;
	}

	if(ASD_WLAN[wasd->WlanID]->extern_balance==0){

		asd_printf(ASD_80211,MSG_DEBUG,"%s: do balance without prob info.\n",__func__);
		/*get ap min flow*/
		for(i=1;i<WTP_NUM;i++){
		
			if(1==is_wtp_apply_wlan(i,wasd->WlanID)){

				unsigned int flow=0;
				if(ASD_WTP_AP[i]!=NULL){		
					flow+=ASD_WTP_AP[i]->radio_flow_info[0].trans_rates;
					flow+=ASD_WTP_AP[i]->radio_flow_info[1].trans_rates;
					flow+=ASD_WTP_AP[i]->radio_flow_info[2].trans_rates;
					flow+=ASD_WTP_AP[i]->radio_flow_info[3].trans_rates;
				}
				if(flow<ap_min_flow){
					ap_min_flow=flow;
				}
			}
		}
	}else if(ASD_WLAN[wasd->WlanID]->extern_balance==1){
		/*get ap min flow*/
		nod=ASD_WLAN[wasd->WlanID]->sta_from_which_bss_list;
		while(nod!=NULL){
			if(0==memcmp(nod->mac,mac,MAC_LEN)
				&&0==check_arrival_counter(nod)){
				if((nod->from_bss_list!=NULL)&&(nod->list_len!=0)){
					bss_nod=nod->from_bss_list;
					while(bss_nod!=NULL){
						unsigned int flow=0, wtpid=0;
						wtpid=bss_nod->bss_index/(L_BSS_NUM*L_RADIO_NUM);

						if(ASD_WTP_AP[wtpid]!=NULL){
							flow+=ASD_WTP_AP[wtpid]->radio_flow_info[0].trans_rates;
							flow+=ASD_WTP_AP[wtpid]->radio_flow_info[1].trans_rates;
							flow+=ASD_WTP_AP[wtpid]->radio_flow_info[2].trans_rates;
							flow+=ASD_WTP_AP[wtpid]->radio_flow_info[3].trans_rates;
					
							if(flow<ap_min_flow){
								ap_min_flow=flow;
							}
						}

						bss_nod=bss_nod->next;
					}
				
				}
			}
			nod=nod->next;
		}
	}
	
	if(ap_min_flow==0xffffffff){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: get ap min flow fail.\n",__func__);
		return -1;
	}

	asd_printf(ASD_80211,MSG_DEBUG,"%s: ap_min_flow  =%u(kbps).\n",__func__,ap_min_flow);
	asd_printf(ASD_80211,MSG_DEBUG,"%s: cur_wtp_flow =%u(kbps).\n",__func__,cur_wtp_flow);
	
	if(cur_wtp_flow<=(ap_min_flow+ASD_WLAN[wasd->WlanID]->flow_balance_para*1024)){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: accept sta in wtp %u, bssindex %u\n"
			,__func__,wtpid,wasd->BSSIndex);
		return 1;
	}else
		return	0;
	
}

unsigned int 
get_sta_num_by_wtpid(unsigned int WTPID){
	
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	unsigned int i = 0, j=0;
	unsigned int num = 0;

	for(i = WTPID*L_RADIO_NUM; i < (WTPID+1)*L_RADIO_NUM; i++) {
		if(interfaces->iface[i] == NULL)
			continue;
		for(j = 0; j < L_BSS_NUM; j++){
			if(interfaces->iface[i]->bss[j] == NULL)
				continue;
			else {
				num += interfaces->iface[i]->bss[j]->num_sta;
			}	
		}
	}
	
	return num;
}




static int number_balance(struct asd_data *wasd,unsigned char mac[],unsigned char flag){

	/*to be completed*/
	unsigned int wtpid=0,current_wtp_sta_num=0,ap_min_num=0xffffffff;
	unsigned int i=0;
	log_node  *nod=NULL;
	bss_arrival_num *bss_nod=NULL;
	struct sta_info *sta = NULL;
	
	if(!wasd){
		asd_printf(ASD_80211,MSG_DEBUG,"wasd==NULL in %s\n",__func__);
		return -1;
	}

	wtpid=wasd->Radio_G_ID/L_RADIO_NUM;

	if(!ASD_WTP_AP[wtpid]){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WTP_AP[%u]==NULL in %s\n",wtpid,__func__);
		return -1;
	}

	sta = ap_get_sta(wasd, mac);
	if(sta != NULL){
		flag = 0;
	}
	current_wtp_sta_num=get_sta_num_by_wtpid(wtpid);
	if(flag==1){
		current_wtp_sta_num+=1;
	}
	
	asd_printf(ASD_80211,MSG_DEBUG,"%s:	current_wtp_sta_num=%u\n",__func__,current_wtp_sta_num);

	if(current_wtp_sta_num<=ASD_WTP_AP[wtpid]->wtp_triger_num){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: accept sta in wtp %u, bssindex %u\n"
			,__func__,wtpid,wasd->BSSIndex);
		return 1;
	}

	if(!ASD_WLAN[wasd->WlanID]){
		asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[%u]==NULL in %s\n",wasd->WlanID,__func__);
		return -1;
	}

	if(ASD_WLAN[wasd->WlanID]->extern_balance==0){

		asd_printf(ASD_80211,MSG_DEBUG,"%s: do balance without prob info.\n",__func__);
		/*get ap min num*/
		for(i=1;i<WTP_NUM;i++){
			if(1==is_wtp_apply_wlan(i,wasd->WlanID)){

				unsigned int num=0;
			
				num = get_sta_num_by_wtpid(i);
				asd_printf(ASD_80211,MSG_DEBUG,"num = %d\n",num);

				if(num<ap_min_num){
					ap_min_num=num;
				}
				asd_printf(ASD_80211,MSG_DEBUG,"ap_min_num111 = %u\n",ap_min_num);
			}
		}
	}else if(ASD_WLAN[wasd->WlanID]->extern_balance==1){
		/*get ap min num*/
		nod=ASD_WLAN[wasd->WlanID]->sta_from_which_bss_list;
		while(nod!=NULL){
			if(0==memcmp(nod->mac,mac,MAC_LEN)
				&&0==check_arrival_counter(nod)){
				if((nod->from_bss_list!=NULL)&&(nod->list_len!=0)){
					bss_nod=nod->from_bss_list;
					while(bss_nod!=NULL){
						unsigned int wtpid=0;
						unsigned int num=0;
					
						wtpid=bss_nod->bss_index/(L_BSS_NUM*L_RADIO_NUM);
					
						if(wtpid>=1&&wtpid<WTP_NUM){
							num = get_sta_num_by_wtpid(wtpid);
					
							if(num<ap_min_num){
								ap_min_num=num;
							}
						}
						bss_nod=bss_nod->next;
					}
				
				}
			}
			nod=nod->next;
		}
	}
	
	asd_printf(ASD_80211,MSG_DEBUG,"%s: ap_min_num =%u.\n",__func__,ap_min_num);
	asd_printf(ASD_80211,MSG_DEBUG,"%s: current_sta_num =%u.\n",__func__,current_wtp_sta_num);

	if(ap_min_num==0xffffffff){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: get ap min number fail. \n",__func__);
		return 0;
	}
	
	if(current_wtp_sta_num<=(ap_min_num+ASD_WLAN[wasd->WlanID]->balance_para)){
		asd_printf(ASD_80211,MSG_DEBUG,"%s: accept sta in wtp %u, bssindex %u\n"
			,__func__,wtpid,wasd->BSSIndex);
		return 1;
	}else
		return	0;
	
	return 1;
}

static void print_node_list(log_node  *nod,unsigned int len){

	bss_arrival_num *p=NULL;
	
	if(nod==NULL||len==0){
		asd_printf(ASD_80211,MSG_DEBUG,"nod==NULL||len==0 in %s\n",__func__);
		return;
	}

	asd_printf(ASD_80211,MSG_DEBUG,"in %s\n",__func__);
	
	while(nod!=NULL){
		asd_printf(ASD_80211,MSG_DEBUG,"\n\nsta="MACSTR"---bss_num-%d\n\n",MAC2STR(nod->mac),nod->list_len);
		for(p=nod->from_bss_list;p!=NULL;p=p->next){
			asd_printf(ASD_80211,MSG_DEBUG,"\t\t---bss_arrival_counter-%u---bss_index-%u\n",p->arrival_num,p->bss_index);
		}
		nod=nod->next;
	}
		
}

static void handle_auth(struct asd_data *wasd, struct ieee80211_mgmt *mgmt,
			size_t len)
{
	u16 auth_alg, auth_transaction, status_code;
	u16 resp = WLAN_STATUS_SUCCESS;
	struct sta_info *sta = NULL;
	int res;
	u16 fc;
	u8 *challenge = NULL;
	u32 session_timeout, acct_interim_interval;
	int vlan_id = 0;
	u8 resp_ies[2 + WLAN_AUTH_CHALLENGE_LEN];
	size_t resp_ies_len = 0;
	char reas[256] = {0};
	unsigned int wtpid = 0;
	unsigned char SID = 0;
	unsigned int securitytype = 0;
	u8 WTPMAC[MAC_LEN] = {0};
	
	/* For new format of syslog 2013-07-29 */
	int Rcode = 0;
	char *SSID = NULL;

	
	if(NULL == wasd || NULL == mgmt)
		return;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.auth)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_auth - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	wasd->usr_auth_tms++;

	/* Add SSID for BSS, instead of ESSID */
	if(wasd->conf->ssid.ssid != NULL)
	{
		SSID = (char *)wasd->conf->ssid.ssid;  
	}
	else if(ASD_WLAN[wasd->WlanID] != NULL)
	{
		SSID = ASD_WLAN[wasd->WlanID]->ESSID;
	}
	
	if(ASD_WLAN[wasd->WlanID]){
		SID = ASD_WLAN[wasd->WlanID]->SecurityID;
	}
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType == SHARED)){
		wasd->assoc_auth_req_num++;
	}
	//qiuchen
	if(ASD_WTP_AP[wasd->Radio_G_ID/4]){
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
		wtpid = ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPID;
	}
	
	if(gASDLOGDEBUG & BIT(1))
		asd_syslog_auteview(LOG_INFO,STA_AUTH_START,mgmt,wasd,NULL,0,NULL);
	
	auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
	auth_transaction = le_to_host16(mgmt->u.auth.auth_transaction);
	status_code = le_to_host16(mgmt->u.auth.status_code);
	fc = le_to_host16(mgmt->frame_control);

	if (len >= IEEE80211_HDRLEN + sizeof(mgmt->u.auth) +
	    2 + WLAN_AUTH_CHALLENGE_LEN &&
	    mgmt->u.auth.variable[0] == WLAN_EID_CHALLENGE &&
	    mgmt->u.auth.variable[1] == WLAN_AUTH_CHALLENGE_LEN)
		challenge = &mgmt->u.auth.variable[2];

	asd_printf(ASD_80211,MSG_DEBUG, "authentication: STA=" MACSTR " auth_alg=%d "
		   "auth_transaction=%d status_code=%d wep=%d%s\n",
		   MAC2STR(mgmt->sa), auth_alg, auth_transaction,
		   status_code, !!(fc & WLAN_FC_ISWEP),
		   challenge ? " challenge" : "");			

	//mahz copy from verdion 2.0 , 2011.3.14
	if((wasd->conf->auth_algs & WPA_AUTH_ALG_SHARED) && (auth_transaction == 1))
		wasd->info->identify_request++;
	if((wasd->conf->auth_algs & WPA_AUTH_ALG_SHARED) && (auth_transaction != 1) && (auth_transaction != 3)){
		wasd->info->abort_key_error++;
		resp = WLAN_STATUS_CHALLENGE_FAIL;
		Rcode = REASON_CODE_MAX;
		goto fail;
	}
	//
	
	if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->balance_switch==1)){
	/*blxm0714*/

		print_node_list(ASD_WLAN[wasd->WlanID]->sta_from_which_bss_list,ASD_WLAN[wasd->WlanID]->sta_list_len);

		asd_printf(ASD_80211,MSG_DEBUG, "ASD_WLAN[wasd->WlanID]->balance_method==%u\n",ASD_WLAN[wasd->WlanID]->balance_method);
		
		if(ASD_WLAN[wasd->WlanID]->balance_method==2){

			asd_printf(ASD_80211,MSG_DEBUG, "flow balance.\n");
			if(1!=flow_balance(wasd,mgmt->sa)){
				asd_printf(ASD_80211,MSG_DEBUG, "BALANCE:STA=" MACSTR " BSSIndex %d  denied du to flow balance.\n",
					MAC2STR(mgmt->sa), wasd->BSSIndex);
				Rcode = FLOW_BANLANCE;
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(FLOW_BANLANCE,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp =  WLAN_STATUS_UNSPECIFIED_FAILURE;
				goto fail;
			}
		 }

			
		if(ASD_WLAN[wasd->WlanID]->balance_method==1){

			asd_printf(ASD_80211,MSG_DEBUG, "number balance.\n");
			if(1!=number_balance(wasd,mgmt->sa,1)){
				asd_printf(ASD_80211,MSG_DEBUG, "BALANCE:STA=" MACSTR " BSSIndex %d  denied du to number balance.\n",
					MAC2STR(mgmt->sa), wasd->BSSIndex);
				Rcode = NUMBER_BANLANCE;
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(NUMBER_BANLANCE,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp =  WLAN_STATUS_UNSPECIFIED_FAILURE;
				
				goto fail;
			}
		 }

	}
	
	
	asd_printf(ASD_80211,MSG_DEBUG, "authentication: STA=" MACSTR " BSSIndex %d\n",
		MAC2STR(mgmt->sa), wasd->BSSIndex);

	asd_printf(ASD_80211,MSG_DEBUG,"mgmt->bssid:" MACSTR ",  assoc_ap_addr:" MACSTR " \n", MAC2STR(mgmt->bssid), MAC2STR(wasd->conf->assoc_ap_addr));
	asd_printf(ASD_80211,MSG_DEBUG,"ASD_WLAN[Wlanid]->ESSID:%s  \n", ASD_WLAN[wasd->WlanID]->ESSID);
	asd_printf(ASD_80211,MSG_DEBUG,"ASD_BSS[%d]->SSID:%s  \n",wasd->BSSIndex, ASD_BSS[wasd->BSSIndex]->SSID);

		
	if (wasd->assoc_ap_state == AUTHENTICATE && auth_transaction == 2 &&
	    os_memcmp(mgmt->sa, wasd->conf->assoc_ap_addr, ETH_ALEN) == 0 &&
	    os_memcmp(mgmt->bssid, wasd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		if (status_code != 0) {
			asd_printf(ASD_80211,MSG_NOTICE,"Authentication (as station) with AP "
			       MACSTR " failed (status_code=%d)\n",
			       MAC2STR(wasd->conf->assoc_ap_addr),
			       status_code);
			return;
		}
		asd_printf(ASD_80211,MSG_DEBUG,"Authenticated (as station) with AP " MACSTR "\n",
		       MAC2STR(wasd->conf->assoc_ap_addr));
		ieee802_11_sta_associate(wasd, NULL);
		return;
	}

	if (wasd->tkip_countermeasures) {
		resp = WLAN_REASON_MICHAEL_MIC_FAILURE;
		Rcode = REASON_CODE_MAX;		
		goto fail;
	}

	if (!(((wasd->conf->auth_algs & WPA_AUTH_ALG_OPEN) &&
	       auth_alg == WLAN_AUTH_OPEN) ||
#ifdef ASD_IEEE80211R
	      (wasd->conf->wpa &&
	       (wasd->conf->wpa_key_mgmt &
		(WPA_KEY_MGMT_FT_IEEE8021X | WPA_KEY_MGMT_FT_PSK)) &&
	       auth_alg == WLAN_AUTH_FT) ||
#endif /* ASD_IEEE80211R */
	      ((wasd->conf->auth_algs & WPA_AUTH_ALG_SHARED) &&
	       auth_alg == WLAN_AUTH_SHARED_KEY))) {
		asd_printf(ASD_80211,MSG_DEBUG,"Unsupported authentication algorithm (%d)\n",
		       auth_alg);
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(AUTH_ALG_FAIL,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG;
		Rcode = AUTH_ALG_FAIL;
		goto fail;
	}

	if (!(auth_transaction == 1 ||
	      (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 3))) {
		asd_printf(ASD_80211,MSG_DEBUG,"Unknown authentication transaction number (%d)\n",
		       auth_transaction);
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(AUTH_TRANSNUM_WRONG,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION;
		Rcode = AUTH_TRANSNUM_WRONG;
		goto fail;
	}

	if (os_memcmp(mgmt->sa, wasd->own_addr, ETH_ALEN) == 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"Station " MACSTR " not allowed to authenticate.\n",
		       MAC2STR(mgmt->sa));
		Rcode = STAMAC_BSSID;
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(STAMAC_BSSID,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

	res = asd_allowed_address(wasd, mgmt->sa, (u8 *) mgmt, len,
				      &session_timeout,
				      &acct_interim_interval, &vlan_id);
	if (res == asd_ACL_REJECT) {
		asd_printf(ASD_80211,MSG_DEBUG,"Station " MACSTR " not allowed to authenticate.\n",
		       MAC2STR(mgmt->sa));
		asd_printf(ASD_80211,MSG_DEBUG,"The mac is rejected\n");
			Rcode = MAC_REJECTED;
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(MAC_REJECTED,reas);
			asd_syslog_h(LOG_INFO,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}
	if (res == asd_ACL_PENDING) {
		asd_printf(ASD_80211,MSG_DEBUG, "Authentication frame from " MACSTR
			   " waiting for an external authentication",
			   MAC2STR(mgmt->sa));
		/* Authentication code will re-send the authentication frame
		 * after it has received (and cached) information from the
		 * external source. */
		return;
	}

	sta = ap_sta_add(wasd, mgmt->sa ,1);
	if (!sta) {
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		wasd->info->deauth_ap_unable++;				//mahz copy from version 2.0,2011.3.14
		Rcode = NO_RESOURCE;
		goto fail;
	}

	if (vlan_id > 0) {
		if (asd_get_vlan_id_ifname(wasd->conf->vlan,
					       sta->vlan_id) == NULL) {
			asd_logger(wasd, sta->addr, asd_MODULE_RADIUS,
				       asd_LEVEL_INFO, "Invalid VLAN ID "
				       "%d received from RADIUS server",
				       vlan_id);
			Rcode = VLANID_INVALID;
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(VLANID_INVALID,reas);
				asd_syslog_h(LOG_INFO,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}
		sta->vlan_id = vlan_id;
		asd_logger(wasd, sta->addr, asd_MODULE_RADIUS,
			       asd_LEVEL_INFO, "VLAN ID %d", sta->vlan_id);
	}
	if(sta->flags&WLAN_STA_PREAUTH)
		//asd_sta_add("", wasd, sta->addr,0,0,NULL,0, 0);
		asd_sta_roaming_management(sta);//Qc
	sta->flags &= ~WLAN_STA_PREAUTH;
	ieee802_1x_notify_pre_auth(sta->eapol_sm, 0);

	if (wasd->conf->radius->acct_interim_interval == 0 &&
	    acct_interim_interval)
		sta->acct_interim_interval = acct_interim_interval;
	if (res == asd_ACL_ACCEPT_TIMEOUT)
		ap_sta_session_timeout(wasd, sta, session_timeout);
	else
		ap_sta_no_session_timeout(wasd, sta);

	switch (auth_alg) {
	case WLAN_AUTH_OPEN:
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "authentication OK (open system)");
#ifdef IEEE80211_REQUIRE_AUTH_ACK
		/* Station will be marked authenticated if it ACKs the
		 * authentication reply. */
#else
		sta->flags |= WLAN_STA_AUTH;
		wpa_auth_sm_event(sta->wpa_sm, WPA_AUTH);
		sta->auth_alg = WLAN_AUTH_OPEN;
		mlme_authenticate_indication(wasd, sta);
#endif
		break;
	case WLAN_AUTH_SHARED_KEY:
		sta->auth_alg = WLAN_AUTH_SHARED_KEY;
		if(WTP_SEND_RESPONSE_TO_MOBILE==0)//nl1010
		{
			resp = auth_shared_key(wasd, sta, auth_transaction, challenge,1);	//mahz copy from version 2.0,2011.3.14
					     //  fc & WLAN_FC_ISWEP);
			mlme_authenticate_indication(wasd, sta);
			if (sta->challenge && auth_transaction == 1) {
				resp_ies[0] = WLAN_EID_CHALLENGE;
				resp_ies[1] = WLAN_AUTH_CHALLENGE_LEN;
				os_memcpy(resp_ies + 2, sta->challenge,
					  WLAN_AUTH_CHALLENGE_LEN);
				resp_ies_len = 2 + WLAN_AUTH_CHALLENGE_LEN;
			}
		}
		//sta->flags |= WLAN_STA_AUTH;
		break;
#ifdef ASD_IEEE80211R
	case WLAN_AUTH_FT:
		sta->auth_alg = WLAN_AUTH_FT;
		if (sta->wpa_sm == NULL)
			sta->wpa_sm = wpa_auth_sta_init(wasd->wpa_auth,
							sta->addr);
		if (sta->wpa_sm == NULL) {
			asd_printf(ASD_80211,MSG_DEBUG, "FT: Failed to initialize WPA "
				   "state machine");
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(WPA_SM_FAILED,reas);
				asd_syslog_h(LOG_INFO,"WSTA","Station Authentication Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			Rcode = WPA_SM_FAILED;
			goto fail;
		}
		wpa_ft_process_auth(sta->wpa_sm, mgmt->bssid,
				    auth_transaction, mgmt->u.auth.variable,
				    len - IEEE80211_HDRLEN -
				    sizeof(mgmt->u.auth),
				    handle_auth_ft_finish, wasd);
		/* handle_auth_ft_finish() callback will complete auth. */
		return;
#endif /* ASD_IEEE80211R */
	}

 fail:
	if((SID == 0)&&(ASD_WLAN[wasd->WlanID]))
		SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
	if (resp == WLAN_STATUS_SUCCESS){
		wasd->auth_success++;
		if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType == SHARED))
			wasd->assoc_auth_succ_num++;
	}
	else{
		wasd->auth_fail++;
		if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType == SHARED))
			wasd->assoc_auth_fail_num++;
		if(gASDLOGDEBUG & BIT(1)){
			char error_str[128] = {0};
			asd_parse_log_rcode(&Rcode,error_str);
			if(sta && sta->rflag)
				asd_syslog_auteview(LOG_WARNING,STA_ROAM_FAIL,mgmt,wasd,sta,Rcode,error_str);
			else
				asd_syslog_auteview(LOG_WARNING,STA_AUTH_FAIL,mgmt,wasd,NULL,Rcode,error_str);
		}
		if(sta && sta->rflag && (gASDLOGDEBUG & BIT(0)))
			asd_syslog_h(LOG_WARNING,"WSTA","WROAM_ROAM_IN_FAILED:Client "MAC_ADDRESS",Failed to roam: Maximum roam-in clients reached.\n",MAC2STR(sta->addr));
	}

	//mahz copy from version 2.0,2011.3.14
 	if (resp == WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG || resp == WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION){
		wasd->info->auth_invalid++;
		if(wasd->conf->auth_algs & WPA_AUTH_ALG_SHARED)
			wasd->info->abort_invalid++;
	}else if (resp == WLAN_STATUS_AUTH_TIMEOUT)
		wasd->info->auth_timeout++;
	else if (resp == WLAN_STATUS_REASSOC_NO_ASSOC || resp == WLAN_STATUS_ASSOC_DENIED_UNSPEC ||
			resp == WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA || resp == WLAN_STATUS_ASSOC_DENIED_RATES)
		wasd->info->auth_refused++;
	else if (resp == WLAN_STATUS_UNSPECIFIED_FAILURE || resp == WLAN_STATUS_CAPS_UNSUPPORTED)
		wasd->info->auth_others++;
    if(WTP_SEND_RESPONSE_TO_MOBILE==0)//nl1010
    {
    	send_auth_reply(wasd, mgmt->sa, mgmt->bssid, auth_alg,
    			auth_transaction + 1, resp, resp_ies, resp_ies_len);
    }

	wasd->ac_rspauth_tms++;
	
	asd_printf(ASD_80211,MSG_DEBUG,"test information:response resp is %d\n",resp);
}
/*
int StaInfoToWID(struct asd_data *wasd, const u8 *addr){
	TableMsg STA;
	int len;
	STA.Op = WID_ADD;
	STA.Type = STA_TYPE;
	STA.u.STA.BSSIndex = wasd->BSSIndex;
	STA.u.STA.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	memcpy(STA.u.STA.STAMAC, addr, ETH_ALEN);
	len = sizeof(STA);
	if(sendto(TableSock, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		perror("send(wASDSocket)");
		return 0;
	}
}
*/
#if 0
static void  initialize_send_iv(u_int32_t *iv, u_int8_t *init_iv)
{
	int i = 0;
	
	for(i=0; i<16/4; i++)
	{
		*(iv+i) =  ntohl(*(((unsigned int *)(init_iv))+i));
	}
}

static int
sta_wapi_init(unsigned int *wk_txiv)
{
	unsigned char iv_init[16] = {	0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36,
								0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x37};

	/*initialize receive iv */
	/*initialize send iv */
	memset((char *)(wk_txiv), 0, 16);	
	initialize_send_iv(wk_txiv, iv_init);
	return 0;
}
#endif

static void handle_assoc(struct asd_data *wasd,
			 struct ieee80211_mgmt *mgmt, size_t len, int reassoc)
{
	u16 capab_info = 0, listen_interval = 0;
	u16 resp = WLAN_STATUS_SUCCESS;
	u8 *pos = NULL, *wpa_ie = NULL, *wapi_ie = NULL;
	size_t wpa_ie_len = 0, wapi_ie_len = 0;
	int send_deauth = 0, send_len = 0, left = 0, i = 0;
	struct sta_info *sta = NULL;
	struct ieee802_11_elems elems;
	u8 buf[sizeof(struct ieee80211_mgmt) + 512];
	struct ieee80211_mgmt *reply = NULL;
	
	int new_assoc = 1;	
	unsigned char SID = 0;//qiuchen
	unsigned int securitytype = 0;
	u8 WTPMAC[MAC_LEN] = {0};
	int Rcode = 0;//Qc
	unsigned int wtpid = 0;
	char reas[256] = {0};
	char *SSID = NULL;
	//yjl copy from aw3.1.2 for local forwarding.2014-2-28
	char ssid_info[ESSID_LENGTH] = { 0 };
	WID_BSS *bss = NULL;
	
	if(NULL == wasd || NULL == mgmt)
	{
		asd_printf(ASD_80211,MSG_WARNING,"NULL == wasd  or  NULL == mgmt, handle_assoc return !\n");		
		return;
	}
	wasd->assoc_req++;
	if(reassoc == 0) {			//ht add,090213
		wasd->num_assoc++;
	}else {
		wasd->num_reassoc++;
	}	

	if(ASD_WTP_AP[wasd->Radio_G_ID/4]){
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
		wtpid = ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPID;
	}
	if(ASD_WLAN[wasd->WlanID] != NULL)
		SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;

	/* Add SSID for BSS, instead of ESSID */
	if(wasd->conf != NULL)
	{
		SSID = (char *)wasd->conf->ssid.ssid;  
	}
	else if(ASD_WLAN[wasd->WlanID] != NULL)
	{
		SSID = ASD_WLAN[wasd->WlanID]->ESSID;
	}
	
	if(SID){
		if (ASD_AUTH_TYPE_WEP_PSK(SID))
		{
			wasd->u.weppsk.assoc_req++;
			
			if (0 == reassoc)
			{
				wasd->u.weppsk.num_assoc++;
			}
			else
			{
				wasd->u.weppsk.num_reassoc++;
			}

			asd_printf(ASD_80211,MSG_DEBUG,
			       "wasd->u.weppsk.assoc_req is %d wasd->u.weppsk.num_assoc %d num_reassoc is %d",
			       wasd->u.weppsk.assoc_req,wasd->u.weppsk.num_assoc,wasd->u.weppsk.num_reassoc);
		}
	}
	//end
	if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->balance_switch==1)){
	/*blxm0714*/
		if(ASD_WLAN[wasd->WlanID]->balance_method==2){
			if(1!=flow_balance(wasd,mgmt->sa)){
				asd_printf(ASD_80211,MSG_DEBUG, "BALANCE:STA=" MACSTR " BSSIndex %d  denied du to flow balance.(assoc)\n",
					MAC2STR(mgmt->sa), wasd->BSSIndex);
				Rcode = FLOW_BANLANCE;
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(FLOW_BANLANCE,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp = WLAN_REASON_DISASSOC_STA_HAS_LEFT;
				send_deauth = 1;
			
				goto fail;
			}
		 }

		if(ASD_WLAN[wasd->WlanID]->balance_method==1){
			if(1!=number_balance(wasd,mgmt->sa,0)){
				asd_printf(ASD_80211,MSG_DEBUG, "BALANCE:STA=" MACSTR " BSSIndex %d  denied du to number balance.(assoc)\n",
					MAC2STR(mgmt->sa), wasd->BSSIndex);
				Rcode = NUMBER_BANLANCE;
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(NUMBER_BANLANCE,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp = WLAN_REASON_DISASSOC_STA_HAS_LEFT;
				send_deauth = 1;
				goto fail;
			}
		 }
	}

	
	if (len < IEEE80211_HDRLEN + (reassoc ? sizeof(mgmt->u.reassoc_req) :
				      sizeof(mgmt->u.assoc_req))) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_assoc(reassoc=%d) - too short payload (len=%lu)"
		       "\n", reassoc, (unsigned long) len);
		return;
	}

	if (reassoc) {
		capab_info = le_to_host16(mgmt->u.reassoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.reassoc_req.listen_interval);
		asd_printf(ASD_80211,MSG_DEBUG, "reassociation request: STA=" MACSTR
			   " capab_info=0x%02x listen_interval=%d current_ap="
			   MACSTR,
			   MAC2STR(mgmt->sa), capab_info, listen_interval,
			   MAC2STR(mgmt->u.reassoc_req.current_ap));
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.reassoc_req));
		pos = mgmt->u.reassoc_req.variable;
	} else {
		capab_info = le_to_host16(mgmt->u.assoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.assoc_req.listen_interval);
		asd_printf(ASD_80211,MSG_DEBUG, "association request: STA=" MACSTR
			   " capab_info=0x%02x listen_interval=%d",
			   MAC2STR(mgmt->sa), capab_info, listen_interval);
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.assoc_req));
		pos = mgmt->u.assoc_req.variable;
	}

	
	sta = ap_get_sta(wasd, mgmt->sa);	

	if(gASDLOGDEBUG & BIT(1))
	{
		if(sta&&reassoc == 0)
		{
			asd_syslog_auteview(LOG_INFO,STA_ASSOC_START,mgmt,wasd,sta,0,NULL);
		}
	}

	
#ifdef ASD_IEEE80211R
	if (sta && sta->auth_alg == WLAN_AUTH_FT &&
	    (sta->flags & WLAN_STA_AUTH) == 0) {
		asd_printf(ASD_80211,MSG_DEBUG, "FT: Allow STA " MACSTR " to associate "
			   "prior to authentication since it is using "
			   "over-the-DS FT", MAC2STR(mgmt->sa));
	} else
#endif /* ASD_IEEE80211R */

	//mahz copy from version 1.2 , 2011.4.7
	if((sta == NULL) && (((ASD_BSS[wasd->BSSIndex]==NULL)? 0:(ASD_BSS[wasd->BSSIndex]->bss_accessed_sta_num>=ASD_BSS[wasd->BSSIndex]->bss_max_allowed_sta_num))
		||((ASD_WLAN[wasd->WlanID]==NULL)? 0:(ASD_WLAN[wasd->WlanID]->wlan_accessed_sta_num>=ASD_WLAN[wasd->WlanID]->wlan_max_allowed_sta_num))
		||((ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]==NULL)? 0:(ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_accessed_sta_num>=ASD_WTP_AP[wasd->Radio_G_ID/L_RADIO_NUM]->ap_max_allowed_sta_num)))
	){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"STA " MACSTR " is rejected because of no resource.\n", MAC2STR(mgmt->sa));
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(NO_RESOURCE,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		wasd->assoc_reject_no_resource++;
		send_deauth = 1;
		resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
		Rcode = NO_RESOURCE;
		goto fail;
	}
	//
	
	if (sta == NULL || (sta->flags & WLAN_STA_AUTH) == 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"STA " MACSTR " trying to associate before "
		       "authentication\n", MAC2STR(mgmt->sa));
		Rcode = ASSO_BEFORE_AUTH;
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(ASSO_BEFORE_AUTH,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		if (sta) {
			asd_printf(ASD_80211,MSG_DEBUG,"  sta: addr=" MACSTR " aid=%d flags=0x%04x\n",
			       MAC2STR(sta->addr), sta->aid, sta->flags);
		}
		send_deauth = 1;
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}


	if (wasd->tkip_countermeasures) {
		resp = WLAN_REASON_MICHAEL_MIC_FAILURE;
		Rcode = REASON_CODE_MAX;		
		goto fail;
	}


	if (reassoc) {
		os_memcpy(sta->previous_ap, mgmt->u.reassoc_req.current_ap,
			  ETH_ALEN);
	}


	sta->capability = capab_info;


	/* followed by SSID and Supported rates */
	if (ieee802_11_parse_elems(wasd, pos, left, &elems, 1) == ParseFailed
	    || !elems.ssid) {
		asd_printf(ASD_80211,MSG_DEBUG,"STA " MACSTR " sent invalid association request\n",
		       MAC2STR(sta->addr));
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(ASSO_PACKAGE_WRONG,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		Rcode = ASSO_PACKAGE_WRONG;
		goto fail;
	}

    /*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
    bss = ASD_BSS[wasd->BSSIndex];
	ieee802_11_print_ssid(ssid_info, elems.ssid, elems.ssid_len);
	if (bss && bss->SSIDSetFlag)
	{

		if ((elems.ssid_len != strlen(bss->SSID))
			|| os_memcmp(elems.ssid, bss->SSID, elems.ssid_len))
		{
			char ssid_txt[asd_MAX_SSID_LEN + 1] = {0};
			ieee802_11_print_ssid(ssid_txt, elems.ssid, elems.ssid_len);
			asd_printf(ASD_80211,MSG_DEBUG,"check SSID: Station " MACSTR " tried to associate with "
			       "unknown SSID '%s'\n", MAC2STR(sta->addr), ssid_info);
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			Rcode = UNKNOWN_SSID;
			goto fail;
		}
	}
	/*end *********************************************/
	else
	{
		
	    if (elems.ssid_len != wasd->conf->ssid.ssid_len ||
	        os_memcmp(elems.ssid, wasd->conf->ssid.ssid, elems.ssid_len) != 0)
	    {
		    char ssid_txt[asd_MAX_SSID_LEN + 1] = {0};
		    ieee802_11_print_ssid(ssid_txt, elems.ssid, elems.ssid_len);
		    asd_printf(ASD_80211,MSG_DEBUG,"Station " MACSTR " tried to associate with "
		         "unknown SSID '%s'\n", MAC2STR(sta->addr), ssid_info);
		    if(gASDLOGDEBUG & BIT(0)){
			    log_parse_reason(UNKNOWN_SSID,reas);
			    asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		    }
		    resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		    Rcode = UNKNOWN_SSID;
		    goto fail;
	    }
	}


	sta->flags &= ~WLAN_STA_WME;
	if (elems.wme && wasd->conf->wme_enabled) {
		if (asd_eid_wme_valid(wasd, elems.wme, elems.wme_len)){
			asd_logger(wasd, sta->addr,
				       asd_MODULE_WPA,
				       asd_LEVEL_DEBUG,
				       "invalid WME element in association "
				       "request");
			Rcode = WME_ELEM_INVALID;
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(WME_ELEM_INVALID,reas);
				asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
		}
		else
			sta->flags |= WLAN_STA_WME;
	}


	if (!elems.supp_rates) {
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "No supported rates element in AssocReq");
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(RATES_NOT_SUPPORT,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_ASSOC_DENIED_RATES;		//ht change 1 to 18
		Rcode = RATES_NOT_SUPPORT;
		goto fail;
	}


	if (elems.supp_rates_len > sizeof(sta->supported_rates)) {
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "Invalid supported rates element length %d",
			       elems.supp_rates_len);
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(RATES_LEN_INVALID,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		Rcode = RATES_LEN_INVALID;
		goto fail;
	}


	os_memset(sta->supported_rates, 0, sizeof(sta->supported_rates));
	os_memcpy(sta->supported_rates, elems.supp_rates,
		  elems.supp_rates_len);
	sta->supported_rates_len = elems.supp_rates_len;


	if (elems.ext_supp_rates) {
		if (elems.supp_rates_len + elems.ext_supp_rates_len >
		    sizeof(sta->supported_rates)) {
			asd_logger(wasd, mgmt->sa,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG,
				       "Invalid supported rates element length"
				       " %d+%d", elems.supp_rates_len,
				       elems.ext_supp_rates_len);
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(RATES_LEN_INVALID,reas);
				asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;		
			Rcode = RATES_LEN_INVALID;
			goto fail;
		}

		os_memcpy(sta->supported_rates + elems.supp_rates_len,
			  elems.ext_supp_rates, elems.ext_supp_rates_len);
		sta->supported_rates_len += elems.ext_supp_rates_len;
	}

#ifdef ASD_IEEE80211N
	/* save HT capabilities in the sta object */
	if(IEEE_80211N_ENABLE){
		asd_printf(ASD_80211,MSG_DEBUG, "ht_capabilities");
		os_memset(&sta->ht_capabilities, 0, sizeof(sta->ht_capabilities));
		if (elems.ht_capabilities &&
			elems.ht_capabilities_len >=
			sizeof(struct ieee80211_ht_capability)) {
			sta->flags |= WLAN_STA_HT;
			sta->ht_capabilities.id = WLAN_EID_HT_CAP;
			sta->ht_capabilities.length =
				sizeof(struct ieee80211_ht_capability);
			os_memcpy(&sta->ht_capabilities.data,
				  elems.ht_capabilities,
				  sizeof(struct ieee80211_ht_capability));
		} else
			sta->flags &= ~WLAN_STA_HT;
	}
#endif /* ASD_IEEE80211N */

	if ((wasd->conf->wpa & WPA_PROTO_RSN) && elems.rsn_ie) {
		wpa_ie = elems.rsn_ie;
		wpa_ie_len = elems.rsn_ie_len;
	} else if ((wasd->conf->wpa & WPA_PROTO_WPA) &&
		   elems.wpa_ie) {
		wpa_ie = elems.wpa_ie;
		wpa_ie_len = elems.wpa_ie_len;
	} else if((wasd->conf->wapi)&& elems.rsn_ie){
		wapi_ie = elems.rsn_ie;
		wapi_ie_len = elems.rsn_ie_len;
	} 
	else {
		wpa_ie = NULL;
		wpa_ie_len = 0;
	}
	

	if (wasd->conf->wpa && wpa_ie == NULL) {
		asd_printf(ASD_80211,MSG_DEBUG,"STA " MACSTR ": No WPA/RSN IE in association "
		       "request\n", MAC2STR(sta->addr));
		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(NO_WPARASN_IE,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		resp = WLAN_STATUS_INVALID_IE;
		Rcode = NO_WPARASN_IE;
		goto fail;
	}


	if (wasd->conf->wpa && wpa_ie) {
		int res;
		wpa_ie -= 2;
		wpa_ie_len += 2;
		if (sta->wpa_sm == NULL)
			sta->wpa_sm = wpa_auth_sta_init(wasd->wpa_auth,
							sta->addr);
		if (sta->wpa_sm == NULL) {
			asd_printf(ASD_80211,MSG_DEBUG,"Failed to initialize WPA state machine\n");
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(WPA_SM_FAILED,reas);
				asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			Rcode = WPA_SM_FAILED;
			goto fail;
		}
		res = wpa_validate_wpa_ie(wasd->wpa_auth, sta->wpa_sm,
					  wpa_ie, wpa_ie_len,
					  elems.mdie, elems.mdie_len, 
					  wasd->WlanID, wasd->BSSIndex);
		if (res == WPA_INVALID_GROUP)
			resp = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;
		else if (res == WPA_INVALID_PAIRWISE)
			resp = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
		else if (res == WPA_INVALID_AKMP)
			resp = WLAN_STATUS_AKMP_NOT_VALID;
		else if (res == WPA_ALLOC_FAIL)
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
#ifdef ASD_IEEE80211W
		else if (res == WPA_MGMT_FRAME_PROTECTION_VIOLATION)
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE; /* FIX */
		else if (res == WPA_INVALID_MGMT_GROUP_CIPHER)
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE; /* FIX */
#endif /* ASD_IEEE80211W */
		else if (res == WPA_INVALID_MDIE)
			resp = WLAN_STATUS_INVALID_MDIE;
		else if (res != WPA_IE_OK)
			resp = WLAN_STATUS_INVALID_IE;
		if (resp != WLAN_STATUS_SUCCESS)
		{
    		Rcode = REASON_CODE_MAX;			
			goto fail;
		}
#ifdef ASD_IEEE80211R
		if (sta->auth_alg == WLAN_AUTH_FT) {
			if (!reassoc) {
				asd_printf(ASD_80211,MSG_DEBUG, "FT: " MACSTR " tried "
					   "to use association (not "
					   "re-association) with FT auth_alg",
					   MAC2STR(sta->addr));
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(AUTH_ALG_FAIL,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
				Rcode = AUTH_ALG_FAIL;
				goto fail;
			}

			resp = wpa_ft_validate_reassoc(sta->wpa_sm, pos, left);
			if (resp != WLAN_STATUS_SUCCESS)
			{
        		Rcode = REASON_CODE_MAX;				
				goto fail;
			}
		}
#endif /* ASD_IEEE80211R */
#ifdef ASD_IEEE80211N
		if(IEEE_80211N_ENABLE){
			asd_printf(ASD_80211,MSG_DEBUG, "wpa_auth_get_pairwise");
			if ((sta->flags & WLAN_STA_HT) &&
				wpa_auth_get_pairwise(sta->wpa_sm) == WPA_CIPHER_TKIP) {
				asd_printf(ASD_80211,MSG_DEBUG, "HT: " MACSTR " tried to "
					   "use TKIP with HT association",
					   MAC2STR(sta->addr));
				if(gASDLOGDEBUG & BIT(0)){
					log_parse_reason(CIPHER_NOT_MATCH,reas);
					asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
				}
				resp = WLAN_STATUS_CIPHER_REJECTED_PER_POLICY;
				Rcode = CIPHER_NOT_MATCH;
				goto fail;
			}
		}
#endif /* ASD_IEEE80211N */
	}else{
		asd_printf(ASD_80211,MSG_DEBUG, "wpa_auth_sta_no_wpa");
		wpa_auth_sta_no_wpa(sta->wpa_sm);
	}
	
	if (wasd->conf->wapi && wapi_ie == NULL) {
		asd_printf(ASD_80211,MSG_DEBUG,"STA " MACSTR ": No WAPI IE in association "
		       "request\n", MAC2STR(sta->addr));

		if(gASDLOGDEBUG & BIT(0)){
			log_parse_reason(NO_WAPI_IE,reas);
			asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
		}
		Rcode = WLAN_STATUS_INVALID_IE;
		resp = NO_WAPI_IE;
		goto fail;
	}

	if(wasd->conf->wapi && wapi_ie){		
		unsigned char iv_init[16] = {	0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36,
									0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x66};
		
		wapi_ie -= 2;
		wapi_ie_len += 2;
		reset_table_item(&sta->wapi_sta_info);
		sta->wapi_sta_info.status = NO_AUTH;
		sta->wapi_sta_info.usksa.usk[0].valid_flag = 0;/*无效的key*/
		sta->wapi_sta_info.usksa.usk[1].valid_flag = 1;/*有效的key*/
		sta->wapi_sta_info.ae_group_sc = 1;
		sta->wapi_sta_info.pap = wasd->wapi_wasd->vap_user->wapid;
		
		memcpy(sta->wapi_sta_info.mac, sta->addr, WLAN_ADDR_LEN);           
		sta->wapi_sta_info.packet_type = PACKET_AUTH_TYPE;		
		memcpy(sta->wapi_sta_info.gsn, iv_init, 16);
		memcpy(sta->wapi_sta_info.wie, wapi_ie, wapi_ie_len);
	}

	if (wasd->iface->dfs_enable &&
	    wasd->iconf->ieee80211h == SPECT_STRICT_BINDING) {
		if (asd_check_power_cap(wasd, elems.power_cap,
					    elems.power_cap_len)) {
			resp = WLAN_STATUS_PWR_CAPABILITY_NOT_VALID;
			asd_logger(wasd, sta->addr,
				       asd_MODULE_IEEE80211,
				       asd_LEVEL_DEBUG,
				       "Power capabilities of the station not "
				       "acceptable");
			Rcode = STA_POWER_NOT_ACCEPTED;
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(STA_POWER_NOT_ACCEPTED,reas);
				asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			goto fail;
		}
	}


	if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G)
		sta->flags |= WLAN_STA_NONERP;
	for (i = 0; i < sta->supported_rates_len; i++) {
		if ((sta->supported_rates[i] & 0x7f) > 22) {
			sta->flags &= ~WLAN_STA_NONERP;
			break;
		}
	}
	

	if (sta->flags & WLAN_STA_NONERP && !sta->nonerp_set) {
		sta->nonerp_set = 1;
		wasd->iface->num_sta_non_erp++;
		if (wasd->iface->num_sta_non_erp == 1)
			ieee802_11_set_beacons(wasd->iface);
	}


	if (!(sta->capability & WLAN_CAPABILITY_SHORT_SLOT_TIME) &&
	    !sta->no_short_slot_time_set) {
		sta->no_short_slot_time_set = 1;
		wasd->iface->num_sta_no_short_slot_time++;
		if (wasd->iface->current_mode->mode ==
		    asd_MODE_IEEE80211G &&
		    wasd->iface->num_sta_no_short_slot_time == 1)
			ieee802_11_set_beacons(wasd->iface);
	}


	if (sta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		sta->flags |= WLAN_STA_SHORT_PREAMBLE;
	else
		sta->flags &= ~WLAN_STA_SHORT_PREAMBLE;

	if (!(sta->capability & WLAN_CAPABILITY_SHORT_PREAMBLE) &&
	    !sta->no_short_preamble_set) {
		sta->no_short_preamble_set = 1;
		wasd->iface->num_sta_no_short_preamble++;
		if (wasd->iface->current_mode->mode == asd_MODE_IEEE80211G
		    && wasd->iface->num_sta_no_short_preamble == 1)
			ieee802_11_set_beacons(wasd->iface);
	}

#ifdef ASD_IEEE80211N
	if(IEEE_80211N_ENABLE){
		if (sta->flags & WLAN_STA_HT) {
			u16 ht_capab = le_to_host16(
				sta->ht_capabilities.data.capabilities_info);
			asd_printf(ASD_80211,MSG_DEBUG, "HT: STA " MACSTR " HT Capabilities "
				   "Info: 0x%04x", MAC2STR(sta->addr), ht_capab);
			if ((ht_capab & HT_CAP_INFO_GREEN_FIELD) == 0) {
				if (!sta->no_ht_gf_set) {
					sta->no_ht_gf_set = 1;
					wasd->iface->num_sta_ht_no_gf++;
				}
				asd_printf(ASD_80211,MSG_DEBUG, "%s STA " MACSTR " - no "
					   "greenfield, num of non-gf stations %d",
					   __func__, MAC2STR(sta->addr),
					   wasd->iface->num_sta_ht_no_gf);
			}
			if ((ht_capab & HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET) == 0) {
				if (!sta->ht_20mhz_set) {
					sta->ht_20mhz_set = 1;
					wasd->iface->num_sta_ht_20mhz++;
				}
				asd_printf(ASD_80211,MSG_DEBUG, "%s STA " MACSTR " - 20 MHz HT, "
					   "num of 20MHz HT STAs %d",
					   __func__, MAC2STR(sta->addr),
					   wasd->iface->num_sta_ht_20mhz);
			}
		} else {
			if (!sta->no_ht_set) {
				sta->no_ht_set = 1;
				wasd->iface->num_sta_no_ht++;
			}
			if (wasd->iconf->ieee80211n) {
				asd_printf(ASD_80211,MSG_DEBUG, "%s STA " MACSTR
					   " - no HT, num of non-HT stations %d",
					   __func__, MAC2STR(sta->addr),
					   wasd->iface->num_sta_no_ht);
			}
		}

		//if (asd_ht_operation_update(wasd->iface) > 0)
		//	ieee802_11_set_beacons(wasd->iface);
	}
#endif /* ASD_IEEE80211N */

	/* get a unique AID */
	if (sta->aid > 0) {
		asd_printf(ASD_80211,MSG_DEBUG, "  old AID %d", sta->aid);
	} else {
		for (sta->aid = 1; sta->aid <= MAX_AID_TABLE_SIZE; sta->aid++)
			if (wasd->sta_aid[sta->aid - 1] == NULL)
				break;
		if (sta->aid > MAX_AID_TABLE_SIZE) {
			sta->aid = 0;
			resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
			Rcode = NO_MORE_AID;
			asd_printf(ASD_80211,MSG_ERROR, "  no room for more AIDs");
			if(gASDLOGDEBUG & BIT(1))
			syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]ASSOCFAILED:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d,SecurityType:%d,ErrorCode:%d.\n",
				slotid,vrrid,MAC2STR(mgmt->sa),MAC2STR(WTPMAC),wasd->BSSIndex,securitytype,NO_MORE_AID);//qiuchen 2013.01.14
			if(gASDLOGDEBUG & BIT(0)){
				log_parse_reason(NO_MORE_AID,reas);
				asd_syslog_h(LOG_WARNING,"WSTA","Station Association Fail:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,FLOW_BANLANCE,reas,wtpid);
			}
			goto fail;
		} else {
			wasd->sta_aid[sta->aid - 1] = sta;
			asd_printf(ASD_80211,MSG_DEBUG, "  new AID %d", sta->aid);
		}
	}


	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "association OK (aid %d)", sta->aid);
	if(gASDLOGDEBUG & BIT(0)){
		if(sta->rflag && !(sta->logflag&BIT(0))){
			if(securitytype == OPEN || securitytype == SHARED){
				asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_HAPPEN:Client "MAC_ADDRESS" roamed from BSSID "MAC_ADDRESS" of AC %lu.%lu.%lu.%lu to BSSID "MAC_ADDRESS" of %lu.%lu.%lu.%lu.\n",MAC2STR(sta->addr),MAC2STR(sta->PreBSSID),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff),MAC2STR(wasd->own_addr),((gASD_AC_MANAGEMENT_IP & 0xff000000) >> 24),((gASD_AC_MANAGEMENT_IP & 0xff0000) >> 16),	\
										((gASD_AC_MANAGEMENT_IP & 0xff00) >> 8),(gASD_AC_MANAGEMENT_IP & 0xff));
				sta->logflag = BIT(0);
			}
		}
		else
			asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_JOIN_WLAN:Client "MAC_ADDRESS" successfully joins WLAN %s,on APID %d with BSSID "MAC_ADDRESS"\n",MAC2STR(mgmt->sa),SSID,wtpid,MAC2STR(wasd->own_addr));
	
	}
	/* Station will be marked associated, after it acknowledges AssocResp
	 */

	if (sta->last_assoc_req){
		os_free(sta->last_assoc_req);
		sta->last_assoc_req=NULL;
	}
	sta->last_assoc_req = os_zalloc(len);
	if (sta->last_assoc_req)
		os_memcpy(sta->last_assoc_req, mgmt, len);

	/* Make sure that the previously registered inactivity timer will not
	 * remove the STA immediately. */
	sta->timeout_next = STA_NULLFUNC;


 fail:
	if(sta && sta->rflag && resp != WLAN_STATUS_SUCCESS && (gASDLOGDEBUG & BIT(0)))
		asd_syslog_h(LOG_INFO,"WSTA","WROAM_ROAM_IN_FAILED:Client "MAC_ADDRESS",Failed to roam: Maximum roam-in clients reached.\n",MAC2STR(sta->addr));
	if(gASDLOGDEBUG & BIT(1)){
		if(resp != WLAN_STATUS_SUCCESS){
			char error_str[128] = {0};
			asd_parse_log_rcode(&Rcode,error_str);
			if(sta && sta->rflag)
				asd_syslog_auteview(LOG_WARNING,STA_ROAM_FAIL,mgmt,wasd,sta,Rcode,error_str);
			else
				asd_syslog_auteview(LOG_WARNING,STA_ASSOC_FAIL,mgmt,wasd,NULL,Rcode,error_str);
		}
		else{
			if(sta && sta->rflag && (securitytype == OPEN || securitytype == SHARED)){				
				/*signal_jianquan_failed(sta->addr,5,sta->BssIndex); remove for fujian log-test 2013-10-21*/
				asd_syslog_auteview(LOG_INFO,STA_ROAM_SUCCESS,mgmt,wasd,sta,0,NULL);
				if(ASD_WLAN[wasd->WlanID])
					ASD_WLAN[wasd->WlanID]->sta_roaming_suc_times++;
			}
			else{
				asd_syslog_auteview(LOG_INFO,STA_ASSOC_SUCCESS,mgmt,wasd,sta,0,NULL);
				if(securitytype == OPEN || securitytype == SHARED)
					asd_syslog_auteview(LOG_INFO,STA_AUTH_SUCCESS,mgmt,wasd,sta,0,NULL);
			}
		}
	}
 	if( (resp!= WLAN_STATUS_SUCCESS)&&(mgmt!=NULL)) {
		signal_assoc_failed(mgmt->sa,resp,wasd->BSSIndex);
	}   //xm add
 	
	wasd->assoc_resp++;
 	if(resp != WLAN_STATUS_SUCCESS) {			//ht add,090324
		if(reassoc == 0)
			wasd->num_assoc_failure++;
		else
			wasd->num_reassoc_failure++;
	}else if(reassoc == 0) {			
		wasd->assoc_success++;
	}else {
		wasd->reassoc_success++;
		local_success_roaming_count++;
	}
	//qiuchen
	if (resp != WLAN_STATUS_SUCCESS) {
		if (ASD_AUTH_TYPE_WEP_PSK(SID))	{
			wasd->u.weppsk.assoc_resp++;
			if (0 == reassoc) {
				wasd->u.weppsk.num_assoc_failure++;
			} else {
				wasd->u.weppsk.num_reassoc_failure++;
			}
			asd_printf(ASD_80211,MSG_DEBUG,
		       "wasd->u.weppsk.num_assoc_failure is %d wasd->u.weppsk.num_reassoc_failure %d assoc_resp is %d",
		       wasd->u.weppsk.num_assoc_failure,wasd->u.weppsk.num_reassoc_failure,wasd->u.weppsk.assoc_resp);
			
		} 
	} else {
		if (ASD_AUTH_TYPE_WEP_PSK(SID))	{
			wasd->u.weppsk.assoc_resp++;			
			if (0 == reassoc) {
				wasd->u.weppsk.assoc_success++;
			} else {
				wasd->u.weppsk.reassoc_success++;
			}
			asd_printf(ASD_80211,MSG_DEBUG,
		       "wasd->u.weppsk.assoc_resp is %d wasd->u.weppsk.assoc_success %d reassoc_success is %d",
		       wasd->u.weppsk.assoc_resp,wasd->u.weppsk.assoc_success,wasd->u.weppsk.reassoc_success);
		} 
	}

	
	if(reassoc == 0) {			//ht add,090213
		if (resp == WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG || resp == WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION)
			wasd->info->assoc_invalid++;
		else if (resp == WLAN_STATUS_AUTH_TIMEOUT)
			wasd->info->assoc_timeout++;
		else if (resp == WLAN_STATUS_REASSOC_NO_ASSOC || resp == WLAN_STATUS_ASSOC_DENIED_UNSPEC ||
				resp == WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA || resp == WLAN_STATUS_ASSOC_DENIED_RATES)
			wasd->info->assoc_refused++;
		else if (resp == WLAN_STATUS_UNSPECIFIED_FAILURE || resp == WLAN_STATUS_CAPS_UNSUPPORTED)
			wasd->info->assoc_others++;
	}else {
		if (resp == WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG || resp == WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION)
			wasd->info->reassoc_invalid++;
		else if (resp == WLAN_STATUS_AUTH_TIMEOUT)
			wasd->info->reassoc_timeout++;
		else if (resp == WLAN_STATUS_REASSOC_NO_ASSOC || resp == WLAN_STATUS_ASSOC_DENIED_UNSPEC ||
				(resp == WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA && resp == WLAN_STATUS_ASSOC_DENIED_RATES))
			wasd->info->reassoc_refused++;
		else if (resp == WLAN_STATUS_UNSPECIFIED_FAILURE || resp == WLAN_STATUS_CAPS_UNSUPPORTED)
			wasd->info->reassoc_others++;
	}

	os_memset(buf, 0, sizeof(buf));
	reply = (struct ieee80211_mgmt *) buf;
	reply->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_MGMT,
			     (send_deauth ? WLAN_FC_STYPE_DEAUTH :
			      (reassoc ? WLAN_FC_STYPE_REASSOC_RESP :
			       WLAN_FC_STYPE_ASSOC_RESP)));
	os_memcpy(reply->da, mgmt->sa, ETH_ALEN);
	os_memcpy(reply->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(reply->bssid, mgmt->bssid, ETH_ALEN);
	reply->seq_ctrl = seq_to_le16(wasd->seq_num++);
	if(wasd->seq_num == 4096)
		wasd->seq_num = 0;

	send_len = IEEE80211_HDRLEN;
	if (send_deauth) {
		//return;              //xumin delete the return.
		send_len += sizeof(reply->u.deauth);
		reply->u.deauth.reason_code = host_to_le16(resp);
		AsdStaInfoToWID(wasd, mgmt->sa, WID_DEL);			//ht add 090305
		asd_printf(ASD_80211,MSG_DEBUG,"StaInfoToWID DEL\n");
		//printf("StaInfoToWID DEL\n");
	} else {
		u8 *p;
		send_len += sizeof(reply->u.assoc_resp);
		reply->u.assoc_resp.capab_info =
			host_to_le16(asd_own_capab_info(wasd, sta, 0));
		reply->u.assoc_resp.status_code = host_to_le16(resp);
		reply->u.assoc_resp.aid = host_to_le16((sta ? sta->aid : 0)
						       | BIT(14) | BIT(15));
#if 1
		p = reply->u.assoc_resp.variable;
		*p++ = WLAN_EID_SUPP_RATES;
		*p++ = elems.supp_rates_len;
		os_memcpy(p, elems.supp_rates,elems.supp_rates_len);
		p += elems.supp_rates_len;
		*p++ = WLAN_EID_EXT_SUPP_RATES;
		*p++ = elems.ext_supp_rates_len;
		os_memcpy(p, elems.ext_supp_rates,elems.ext_supp_rates_len);
		p += elems.ext_supp_rates_len;
#else
		/* Supported rates */
		
		p = asd_eid_supp_rates(wasd, reply->u.assoc_resp.variable);
		/* Extended supported rates */
		p = asd_eid_ext_supp_rates(wasd, p);
#endif
		if (sta->flags & WLAN_STA_WME)
			p = asd_eid_wme(wasd, p);

		p = asd_eid_ht_capabilities_info(wasd, p);
		p = asd_eid_ht_operation(wasd, p);
		
#ifdef ASD_IEEE80211R
		if (resp == WLAN_STATUS_SUCCESS) {
			/* IEEE 802.11r: Mobility Domain Information, Fast BSS
			 * Transition Information, RSN */
			p = wpa_sm_write_assoc_resp_ies(sta->wpa_sm, p,
							buf + sizeof(buf) - p,
							sta->auth_alg);
		}
#endif /* ASD_IEEE80211R */

		send_len += p - reply->u.assoc_resp.variable;

		/* Request TX callback */
		reply->frame_control |= host_to_le16(0);//zhanglei for ieee802.11 ver 0
	}

	//unsigned char SID = 0;qiuchen
	DataMsg msg;
	memset(msg.Data, 0, dot11_Max_Len);
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, reply, send_len, IEEE802_11_MGMT);
//	CWCaptrue(msg.DataLen, msg.Data);
	asd_printf(ASD_80211,MSG_DEBUG,"resp :%d\n",resp);
	if(WTP_SEND_RESPONSE_TO_MOBILE==0)//nl1010
	{
		if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
			perror("handle_assoc: send");
	}

    /*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
	if (resp == 0)
	{
		asd_virdhcp_handle(wasd, sta, 1);
	}
	/*end*************************************************/
	
	asd_printf(ASD_80211,MSG_DEBUG,"StaInfoToWID\n");
	//if(ASD_WLAN[wasd->WlanID])
		//SID = (unsigned char)ASD_WLAN[wasd->WlanID]->SecurityID;qiuchen
	//mahz modified 2011.2.18
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->extensible_auth == 0)&&((ASD_SECURITY[SID]->securityType == OPEN)||(ASD_SECURITY[SID]->securityType == SHARED))&&(resp == 0)){
		if(ASD_NOTICE_STA_INFO_TO_PORTAL)
			AsdStaInfoToEAG(wasd,sta,WID_ADD);
		AsdStaInfoToWID(wasd, sta->addr, WID_ADD);
		if(is_secondary == 0)
			bak_add_sta(wasd,sta);
		unsigned char mac[6];
		int i=0;
		for(i=0;i<6;i++)
			mac[i]=sta->addr[i];
		unsigned char rssi = sta->rssi;	//xiaodawei add rssi for telecom, 20110301
		
		if (!(sta->flags & WLAN_STA_ASSOC))		//mahz add 2011.3.28
			wasd->acc_tms++;
		signal_sta_come(mac,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,rssi);
		if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type && wasd->br_ifname){
			char ifname[IF_NAME_MAX]={0};
			sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
			add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,1) ;
		}
	}
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->securityType == SHARED)&&(resp == 0)){
		wasd->info->identify_success++;
		wasd->assoc_auth_sta_num++;
		sta->sta_assoc_auth_flag = 1;
	}
	
	/* WEP/PSK assocAuth *///qiuchen 
		if (ASD_AUTH_TYPE_WEP_PSK(SID))
		{		
			wasd->u.weppsk.online_sta_num++;
		}
	//end
	if((reassoc) && (resp == 0)) {
		UpdateStaInfoToWSM(wasd, sta->addr, OPEN_ROAM);
	} /*ht add 091028*/
	if(resp == 0){
		if (sta->flags & WLAN_STA_ASSOC)
			new_assoc = 0;
		sta->flags |= WLAN_STA_ASSOC;
	}
	if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1 && ASD_SECURITY[SID]->hybrid_auth == 0))&&(resp == 0)){
		wasd->info->identify_request++;
		wasd->assoc_auth_req_num++;		
		if((sta->acct_session_started)&&(sta->acct_terminate_cause == 0))
			sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		accounting_sta_stop(wasd, sta);
		/* caojia add for eap radius auth packet with acct_session_id, 2014/4/1 */
		if (ASD_SECURITY[SID]->eap_auth_to_radius_acct_session_id_enable == 1) {
			accounting_get_session_id(sta);
		}
		if (reassoc)
			mlme_reassociate_indication(wasd, sta);
		else
			mlme_associate_indication(wasd, sta);
	
		if (sta->auth_alg == WLAN_AUTH_FT)
			wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC_FT);
		else
			wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);
		asd_new_assoc_sta(wasd, sta, !new_assoc);

		ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);
		circle_cancel_timeout(ap_sta_eap_auth_timer,wasd,sta);
		circle_register_timeout(ASD_STA_EAP_AUTH_TIME,0,ap_sta_eap_auth_timer,wasd,sta);
	}
	if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WAPI_AUTH)||(ASD_SECURITY[SID]->securityType == WAPI_PSK))&&(resp == 0)){
		asd_printf(ASD_80211,MSG_DEBUG,"sta wapi init\n");
		wasd->assoc_auth_req_num++;
		auth_initiate(sta, wasd);
	}

	//mahz add 2011.2.18
	if((ASD_SECURITY[SID])&&(ASD_SECURITY[SID]->hybrid_auth == 1)&&(ASD_SECURITY[SID]->extensible_auth == 1)&&(resp == 0)){
	
		AsdStaInfoToWID(wasd, sta->addr, WID_ADD);
        /*yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
		if (ASD_NOTICE_STA_INFO_TO_PORTAL)
		{
			AsdStaInfoToEAG(wasd, sta, WID_ADD);
		}
		/*end*************************************************/

		if(is_secondary == 0)
			bak_add_sta(wasd,sta);
		unsigned char mac[6];
		int i=0;
		for(i=0;i<6;i++)
			mac[i]=sta->addr[i];
		
		if(new_assoc)							//mahz add 2011.5.9
			wasd->acc_tms++;
		unsigned char rssi = sta->rssi;
		signal_sta_come(mac,wasd->Radio_G_ID,wasd->BSSIndex,wasd->WlanID,rssi);
		if(STA_STATIC_FDB_ABLE && wasd->bss_iface_type){
			char ifname[IF_NAME_MAX]={0};
			sprintf(ifname,"radio%d-%d-%d.%d",vrrid,wasd->Radio_G_ID/4,wasd->Radio_L_ID,wasd->WlanID);
			add_and_del_static_br_fdb(wasd->br_ifname,ifname, sta->addr,1) ;
		}
	
		//wasd->info->identify_request++;			//mahz modify 2011.3.28
		if((sta->acct_session_started)&&(sta->acct_terminate_cause == 0))
			sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		accounting_sta_stop(wasd, sta);
		if (reassoc)
			mlme_reassociate_indication(wasd, sta);
		else
			mlme_associate_indication(wasd, sta);
	
		if (sta->auth_alg == WLAN_AUTH_FT)
			wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC_FT);
		else
			wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);
		asd_new_assoc_sta(wasd, sta, !new_assoc);
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);
		asd_printf(ASD_80211,MSG_DEBUG,"return from ieee802_1x_notify_port_enabled\n");
	}
	if((resp == 0)&&(asd_sta_getip_from_dhcpsnoop == 1)&&(1 == check_sta_authorized(wasd,sta)))
	{
		asd_notice_to_dhcp(wasd,sta->addr,DHCP_IP);     /* send msg to dhcp, get ipv4/ipv6 address */
	}
}


static void handle_assoc_resp(struct asd_data *wasd,
			      struct ieee80211_mgmt *mgmt, size_t len)
{
	u16 status_code, aid;

	if (wasd->assoc_ap_state != ASSOCIATE) {
		asd_printf(ASD_80211,MSG_DEBUG,"Unexpected association response received from " MACSTR
		       "\n", MAC2STR(mgmt->sa));
		return;
	}

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.assoc_resp)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_assoc_resp - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	if (os_memcmp(mgmt->sa, wasd->conf->assoc_ap_addr, ETH_ALEN) != 0 ||
	    os_memcmp(mgmt->bssid, wasd->conf->assoc_ap_addr, ETH_ALEN) != 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"Received association response from unexpected address "
		       "(SA=" MACSTR " BSSID=" MACSTR "\n",
		       MAC2STR(mgmt->sa), MAC2STR(mgmt->bssid));
		return;
	}

	status_code = le_to_host16(mgmt->u.assoc_resp.status_code);
	aid = le_to_host16(mgmt->u.assoc_resp.aid);
	aid &= ~(BIT(14) | BIT(15));

	if (status_code != 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"Association (as station) with AP " MACSTR " failed "
		       "(status_code=%d)\n",
		       MAC2STR(wasd->conf->assoc_ap_addr), status_code);
		/* Try to authenticate again */
		wasd->assoc_ap_state = AUTHENTICATE;
		circle_register_timeout(5, 0, ieee802_11_sta_authenticate,
				       wasd, NULL);
	}

	asd_printf(ASD_80211,MSG_DEBUG,"Associated (as station) with AP " MACSTR " (aid=%d)\n",
	       MAC2STR(wasd->conf->assoc_ap_addr), aid);
	wasd->assoc_ap_aid = aid;
	wasd->assoc_ap_state = ASSOCIATED;

	if (asd_set_assoc_ap(wasd, wasd->conf->assoc_ap_addr)) {
		asd_printf(ASD_80211,MSG_DEBUG,"Could not set associated AP address to kernel "
		       "driver.\n");
	}
}


static void handle_disassoc(struct asd_data *wasd,
			    struct ieee80211_mgmt *mgmt, size_t len)
{
	struct sta_info *sta;
	u16 reason_code;
	u8 WTPMAC[MAC_LEN] = {0};
	unsigned char SID = wasd->SecurityID;
	unsigned int securitytype = 0;
	unsigned int wtpid = 0;
	char *SSID = NULL;
	
	if(NULL == wasd || NULL == mgmt)
	{
		asd_printf(ASD_80211,MSG_WARNING,"NULL == wasd  or  NULL == mgmt, handle_disassoc return !\n");		
		return;
	}

	/* Add SSID for BSS, instead of ESSID */
	if(wasd->conf != NULL)
	{
		SSID = (char *)wasd->conf->ssid.ssid;  
	}
	else if(ASD_WLAN[wasd->WlanID] != NULL)
	{
		SSID = ASD_WLAN[wasd->WlanID]->ESSID;
	}

	
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;
	if(ASD_WTP_AP[wasd->Radio_G_ID/4]){
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
		wtpid = ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPID;
	}
	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.disassoc)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_disassoc - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	asd_printf(ASD_80211,MSG_DEBUG, "disassocation: STA=" MACSTR " reason_code=%d",
		   MAC2STR(mgmt->sa),
		   le_to_host16(mgmt->u.disassoc.reason_code));
	reason_code = le_to_host16(mgmt->u.deauth.reason_code);
	wasd->info->disassoc_request++;
	wasd->normal_st_down_num++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"normal_st_down_num %d " ,wasd->normal_st_down_num );

	if (reason_code == WLAN_REASON_DEAUTH_LEAVING || reason_code == WLAN_REASON_DISASSOC_STA_HAS_LEFT)
		wasd->info->disassoc_user_leave++;
	else if (reason_code == WLAN_REASON_DISASSOC_AP_BUSY)
		wasd->info->disassoc_ap_unable++;
	else if (reason_code == WLAN_REASON_PREV_AUTH_NOT_VALID || reason_code == WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA ||
			reason_code == WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA || reason_code == WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH)
		wasd->info->disassoc_abnormal++;
	else if (reason_code == WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH || reason_code == WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY)
		wasd->info->disassoc_others++;

	if (wasd->assoc_ap_state != DO_NOT_ASSOC &&
	    os_memcmp(mgmt->sa, wasd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		asd_printf(ASD_80211,MSG_INFO,"Assoc AP " MACSTR " sent disassociation "
		       "(reason_code=%d) - try to authenticate\n",
		       MAC2STR(wasd->conf->assoc_ap_addr),
		       le_to_host16(mgmt->u.disassoc.reason_code));
		wasd->assoc_ap_state = AUTHENTICATE;
		ieee802_11_sta_authenticate(wasd, NULL);
		circle_register_timeout(0, 500000, ieee802_11_sta_authenticate,
				       wasd, NULL);
		return;
	}

	sta = ap_get_sta(wasd, mgmt->sa);
	if (sta == NULL) {
		asd_printf(ASD_80211,MSG_DEBUG,"Station " MACSTR " trying to disassociate, but it "
		       "is not associated.\n", MAC2STR(mgmt->sa));
		return;
	}
	if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	sta->flags &= ~WLAN_STA_ASSOC;
	//Qiuchen add it for AXSSZFI-1732
	struct PMK_STAINFO *pmk_sta = NULL;
	if(ASD_SECURITY[wasd->SecurityID] && ASD_SECURITY[wasd->SecurityID]->securityType == WPA2_E){
		pmk_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
		if(pmk_sta)
			pmk_ap_free_sta(ASD_WLAN[wasd->WlanID],pmk_sta);
	}
	//end
//	wasd->normal_st_down_num++;//nl091120
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"normal_st_down_num %d " ,wasd->normal_st_down_num );
	wpa_auth_sm_event(sta->wpa_sm, WPA_DISASSOC);
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_INFO, "disassociated");
	if(gASDLOGDEBUG & BIT(1)){
		if(securitytype == OPEN || securitytype == SHARED || securitytype == WAPI_PSK || securitytype == WAPI_AUTH)
			asd_syslog_auteview(LOG_INFO,STA_LEAVING,mgmt,wasd,sta,reason_code,NULL);
		else
			asd_syslog_auteview(LOG_INFO,DOT1X_USER_OFFLINE,mgmt,wasd,sta,reason_code,NULL);
	}
	//qiuchen
	char reas[256] = {0};
	u8 *identity = NULL;
	if(sta->eapol_sm)
		identity = sta->eapol_sm->identity;
	if(gASDLOGDEBUG & BIT(0)){
		log_parse_reason_80211(reason_code,reas);
		if(securitytype == OPEN || securitytype == SHARED)
			asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:StaMac:"MACSTR" Radio id %d SSIDName:%s Cause %d Desc:%s APID:%d\n",MAC2STR(mgmt->sa),wasd->Radio_L_ID,SSID,reason_code,reas,wtpid);
		else if((securitytype == IEEE8021X)||(securitytype == WPA_E)||(securitytype == WPA2_E)||(securitytype == MD5)||((securitytype == WAPI_AUTH) && (ASD_SECURITY[SID]->wapi_radius_auth == 1))||(ASD_SECURITY[SID]->extensible_auth == 1)){
			asd_syslog_h(LOG_INFO,"WSTA","PORTSEC_DOT1X_LOGOFF:-IfName=GRadio-BSS%d:%d-MACAddr="MACSTR"-VlanId=%d-UserName=%s-ErrCode=%d; Session of the 802.1X user was terminated.\n",wasd->Radio_G_ID,wasd->BSSIndex%128,MAC2STR(mgmt->sa),sta->vlan_id,identity,reason_code);
		}
	}
	//end
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	/* Stop Accounting and IEEE 802.1X sessions, but leave the STA
	 * authenticated. */
	accounting_sta_stop(wasd, sta);
	if(ASD_SECURITY[wasd->SecurityID])
		ieee802_1x_free_alive(sta,&ASD_SECURITY[wasd->SecurityID]->eap_alive_period);
	ieee802_1x_free_station(sta);
	if(ASD_NOTICE_STA_INFO_TO_PORTAL){
		sta->initiative_leave = 1;/* yjl 2014-2-28 */
		AsdStaInfoToEAG(wasd,sta,WID_DEL);
	}

//		flashdisconn_sta_add(ASD_FDStas, sta->addr,wasd->BSSIndex,wasd->WlanID);
	//since the fuction 'asd_sta_remove' is called in ap_free_sta,so it's not necessary to call it here
	//asd_sta_remove(wasd, sta->addr);

	if (sta->timeout_next == STA_NULLFUNC ||
	    sta->timeout_next == STA_DISASSOC) {
		sta->timeout_next = STA_DEAUTH;
		circle_cancel_timeout(ap_handle_timer, wasd, sta);
		//circle_register_timeout(AP_DEAUTH_DELAY, 0, ap_handle_timer,
		//		       wasd, sta);
	}

	mlme_disassociate_indication(
		wasd, sta, le_to_host16(mgmt->u.disassoc.reason_code));
	if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->Roaming_Policy!=0)){
		struct ROAMING_STAINFO *r_sta;
		r_sta = roaming_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
		if(r_sta != NULL)
			roaming_free_sta(ASD_WLAN[wasd->WlanID],r_sta);
	}	

	if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
		ap_free_sta(wasd, sta,1);
	}
	else{
		ap_free_sta(wasd, sta,0);
	}

}


static void handle_deauth(struct asd_data *wasd,
			  struct ieee80211_mgmt *mgmt, size_t len)
{
	struct sta_info *sta;
	u16 reason_code;
	u8 WTPMAC[MAC_LEN] = {0};
	unsigned int wtpid = 0;
	char *SSID =NULL;
	if(NULL == wasd || NULL == mgmt)
	{
		asd_printf(ASD_80211,MSG_WARNING,"NULL == wasd  or  NULL == mgmt, handle_disassoc return !\n");		
		return;
	}

	/* Add SSID for BSS, instead of ESSID */
	if(wasd->conf != NULL)
	{
		SSID = (char *)wasd->conf->ssid.ssid;  
	}
	else if(ASD_WLAN[wasd->WlanID] != NULL)
	{
		SSID = ASD_WLAN[wasd->WlanID]->ESSID;
	}
	
	if(ASD_WTP_AP[wasd->Radio_G_ID/4]){
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
		wtpid = ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPID;
	}

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.deauth)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_deauth - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	asd_printf(ASD_80211,MSG_DEBUG, "deauthentication: STA=" MACSTR
		   " reason_code=%d",
		   MAC2STR(mgmt->sa),
		   le_to_host16(mgmt->u.deauth.reason_code));
	reason_code = le_to_host16(mgmt->u.deauth.reason_code);
	wasd->info->deauth_request++;
	wasd->normal_st_down_num++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"normal_st_down_num %d 1111111111 ",wasd->normal_st_down_num);

	if (reason_code == WLAN_REASON_DEAUTH_LEAVING || reason_code == WLAN_REASON_DISASSOC_STA_HAS_LEFT)
		wasd->info->deauth_user_leave++;
	else if (reason_code == WLAN_REASON_DISASSOC_AP_BUSY)
		wasd->info->deauth_ap_unable++;
	else if (reason_code == WLAN_REASON_PREV_AUTH_NOT_VALID || reason_code == WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA ||
			reason_code == WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA || reason_code == WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH)
		wasd->info->deauth_abnormal++;
	else if (reason_code == WLAN_REASON_UNSPECIFIED || reason_code == WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY)
		wasd->info->deauth_others++;

	if (wasd->assoc_ap_state != DO_NOT_ASSOC &&
	    os_memcmp(mgmt->sa, wasd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		asd_printf(ASD_80211,MSG_DEBUG,"Assoc AP " MACSTR " sent deauthentication "
		       "(reason_code=%d) - try to authenticate\n",
		       MAC2STR(wasd->conf->assoc_ap_addr),
		       le_to_host16(mgmt->u.deauth.reason_code));
		wasd->assoc_ap_state = AUTHENTICATE;
		circle_register_timeout(0, 500000, ieee802_11_sta_authenticate,
				       wasd, NULL);
		return;
	}

	sta = ap_get_sta(wasd, mgmt->sa);
	if (sta == NULL) {
		asd_printf(ASD_80211,MSG_DEBUG,"Station " MACSTR " trying to deauthenticate, but it "
		       "is not authenticated.\n", MAC2STR(mgmt->sa));
		return;
	}

	if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
//	wasd->normal_st_down_num++;//nl091120
//	asd_printf(ASD_DEFAULT,MSG_DEBUG,"normal_st_down_num %d 1111111111 ",wasd->normal_st_down_num);
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
	unsigned char SID = 0;
	if(ASD_WLAN[wasd->WlanID])
		SID = ASD_WLAN[wasd->WlanID]->SecurityID;
	//Qiuchen add it for AXSSZFI-1732
	struct PMK_STAINFO *pmk_sta = NULL;
	if(ASD_SECURITY[SID] && ASD_SECURITY[SID]->securityType == WPA2_E){
		pmk_sta = pmk_ap_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
		if(pmk_sta)
			pmk_ap_free_sta(ASD_WLAN[wasd->WlanID],pmk_sta);
	}
	//end
	if((ASD_SECURITY[SID])&&((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1))){
	wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG, "deauthenticated");
	//qiuchen
	unsigned int securitytype = 0;
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;
	if(gASDLOGDEBUG & BIT(1)){
		if(securitytype == OPEN || securitytype == SHARED || securitytype == WAPI_PSK || securitytype == WAPI_AUTH)
			asd_syslog_auteview(LOG_INFO,STA_LEAVING,mgmt,wasd,sta,reason_code,NULL);
		else
			asd_syslog_auteview(LOG_INFO,DOT1X_USER_OFFLINE,mgmt,wasd,sta,reason_code,NULL);
	}
	//end
	if(gASDLOGDEBUG & BIT(0)){
		asd_syslog_h(LOG_INFO,"WSTA","WMAC_CLIENT_GOES_OFFLINE:Client "MACSTR" disconnected from WLAN %s. Reason Code is %d.\n",MAC2STR(mgmt->sa),SSID,reason_code);
	}
	//end
	mlme_deauthenticate_indication(
		wasd, sta, le_to_host16(mgmt->u.deauth.reason_code));
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	}
	if((ASD_WLAN[wasd->WlanID]!=NULL)&&(ASD_WLAN[wasd->WlanID]->Roaming_Policy!=0)){
		struct ROAMING_STAINFO *r_sta;
		r_sta = roaming_get_sta(ASD_WLAN[wasd->WlanID],sta->addr);
		if(r_sta != NULL)
			roaming_free_sta(ASD_WLAN[wasd->WlanID],r_sta);
	}	
	if(ASD_NOTICE_STA_INFO_TO_PORTAL){
		sta->initiative_leave = 1;/* yjl 2014-2-28 */
		AsdStaInfoToEAG(wasd,sta,WID_DEL);
	}
	//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
	if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1){
		//flashdisconn_sta_add(ASD_FDStas, sta->addr,wasd->BSSIndex,wasd->WlanID);
		ap_free_sta(wasd, sta,1);
	}
	else{
		ap_free_sta(wasd, sta,0);
	}
}

#if 0
static void handle_beacon(struct asd_data *wasd,
			  struct ieee80211_mgmt *mgmt, size_t len,
			  struct asd_frame_info *fi)
{
	asd_printf(ASD_80211,MSG_DEBUG,"func: %s\n",__func__);
	struct ieee802_11_elems elems;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.beacon)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_beacon - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	(void) ieee802_11_parse_elems(wasd, mgmt->u.beacon.variable,
				      len - (IEEE80211_HDRLEN +
					     sizeof(mgmt->u.beacon)), &elems,
				      0);

	if (wasd->assoc_ap_state == WAIT_BEACON &&
	    os_memcmp(mgmt->sa, wasd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		if (elems.ssid && elems.ssid_len <= 32) {
			os_memcpy(wasd->assoc_ap_ssid, elems.ssid,
				  elems.ssid_len);
			wasd->assoc_ap_ssid[elems.ssid_len] = '\0';
			wasd->assoc_ap_ssid_len = elems.ssid_len;
		}
		ieee802_11_sta_authenticate(wasd, NULL);
	}

	ap_list_process_beacon(wasd->iface, mgmt, &elems, fi);
}
#endif

static void handle_action(struct asd_data *wasd,
			  struct ieee80211_mgmt *mgmt, size_t len)
{
	if (len < IEEE80211_HDRLEN + 1) {
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "handle_action - too short payload (len=%lu)",
			       (unsigned long) len);
		return;
	}

	switch (mgmt->u.action.category) {
#ifdef ASD_IEEE80211R
	case WLAN_ACTION_FT:
	{
		struct sta_info *sta;

		sta = ap_get_sta(wasd, mgmt->sa);
		if (sta == NULL || !(sta->flags & WLAN_STA_ASSOC)) {
			asd_printf(ASD_80211,MSG_DEBUG, "IEEE 802.11: Ignored FT Action "
				   "frame from unassociated STA " MACSTR,
				   MAC2STR(mgmt->sa));
			return;
		}

		if (wpa_ft_action_rx(sta->wpa_sm, (u8 *) &mgmt->u.action,
				     len - IEEE80211_HDRLEN))
			break;

		return;
	}
#endif /* ASD_IEEE80211R */
	case WME_ACTION_CATEGORY:
		asd_wme_action(wasd, mgmt, len);
		return;
	}

	asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "handle_action - unknown action category %d or invalid "
		       "frame",
		       mgmt->u.action.category);
	if (!(mgmt->da[0] & 0x01) && !(mgmt->u.action.category & 0x80) &&
	    !(mgmt->sa[0] & 0x01)) {
		/*
		 * IEEE 802.11-REVma/D9.0 - 7.3.1.11
		 * Return the Action frame to the source without change
		 * except that MSB of the Category set to 1.
		 */
		asd_printf(ASD_80211,MSG_DEBUG, "IEEE 802.11: Return unknown Action "
			   "frame back to sender");
		os_memcpy(mgmt->da, mgmt->sa, ETH_ALEN);
		os_memcpy(mgmt->sa, wasd->own_addr, ETH_ALEN);
		os_memcpy(mgmt->bssid, wasd->own_addr, ETH_ALEN);
		mgmt->u.action.category |= 0x80;
		
		DataMsg msg;
		int msglen;
		msglen = Assmble_WSM_msg(&msg, wasd, mgmt, len, IEEE802_11_MGMT);

		asd_send_mgmt_frame(wasd, &msg, msglen, 0);
	}
}


/**
 * ieee802_11_mgmt - process incoming IEEE 802.11 management frames
 * @wasd: asd BSS data structure (the BSS to which the management frame was
 * sent to)
 * @buf: management frame data (starting from IEEE 802.11 header)
 * @len: length of frame data in octets
 * @stype: management frame subtype from frame control field
 *
 * Process all incoming IEEE 802.11 management frames. This will be called for
 * each frame received from the kernel driver through wlan#ap interface. In
 * addition, it can be called to re-inserted pending frames (e.g., when using
 * external RADIUS server as an MAC ACL).
 */
void ieee802_11_mgmt(struct asd_data *wasd, u8 *buf, size_t len, u16 stype,
		     struct asd_frame_info *fi)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;
	//int broadcast;

	if (stype == WLAN_FC_STYPE_BEACON) {
		/* for ONLINEBUG-880 and asd core in INA */
		/*
		handle_beacon(wasd, mgmt, len, fi);
		*/
		return;
	}

	if (fi && fi->passive_scan)
		return;
/*
	broadcast = mgmt->bssid[0] == 0xff && mgmt->bssid[1] == 0xff &&
		mgmt->bssid[2] == 0xff && mgmt->bssid[3] == 0xff &&
		mgmt->bssid[4] == 0xff && mgmt->bssid[5] == 0xff;

	if (!broadcast &&
	    os_memcmp(mgmt->bssid, wasd->own_addr, ETH_ALEN) != 0 &&
	    (wasd->assoc_ap_state == DO_NOT_ASSOC ||
	     os_memcmp(mgmt->bssid, wasd->conf->assoc_ap_addr, ETH_ALEN) != 0))
	{
		asd_printf(ASD_80211,MSG_DEBUG,"MGMT: BSSID=" MACSTR " not our address\n",
		       MAC2STR(mgmt->bssid));
		return;
	}

*/
	if (stype == WLAN_FC_STYPE_PROBE_REQ) {
		handle_probe_req(wasd, mgmt, len);
		return;
	}


	if (os_memcmp(mgmt->da, wasd->own_addr, ETH_ALEN) != 0) {
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "MGMT: DA=" MACSTR " not our address",
			       MAC2STR(mgmt->da));
		return;
	}


	switch (stype) {
	case WLAN_FC_STYPE_AUTH:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::auth");
		handle_auth(wasd, mgmt, len);
		break;
	case WLAN_FC_STYPE_ASSOC_REQ:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::assoc_req");
		handle_assoc(wasd, mgmt, len, 0);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::assoc_resp");
		handle_assoc_resp(wasd, mgmt, len);
		break;
	case WLAN_FC_STYPE_REASSOC_REQ:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::reassoc_req");
		handle_assoc(wasd, mgmt, len, 1);
		break;
	case WLAN_FC_STYPE_DISASSOC:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::disassoc");
		handle_disassoc(wasd, mgmt, len);
		break;
	case WLAN_FC_STYPE_DEAUTH:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::deauth");
		handle_deauth(wasd, mgmt, len);
		break;
	case WLAN_FC_STYPE_ACTION:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::action");
		handle_action(wasd, mgmt, len);
		break;
	default:
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "unknown mgmt frame subtype %d", stype);
		break;
	}
}


static void handle_auth_cb(struct asd_data *wasd,
			   struct ieee80211_mgmt *mgmt,
			   size_t len, int ok)
{
	u16 auth_alg, auth_transaction, status_code;
	struct sta_info *sta;

	if (!ok) {
		asd_logger(wasd, mgmt->da, asd_MODULE_IEEE80211,
			       asd_LEVEL_NOTICE,
			       "did not acknowledge authentication response");
		return;
	}

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.auth)) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_auth_cb - too short payload (len=%lu)\n",
		       (unsigned long) len);
		return;
	}

	auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
	auth_transaction = le_to_host16(mgmt->u.auth.auth_transaction);
	status_code = le_to_host16(mgmt->u.auth.status_code);

	sta = ap_get_sta(wasd, mgmt->da);
	if (!sta) {
		asd_printf(ASD_80211,MSG_WARNING,"handle_auth_cb: STA " MACSTR " not found\n",
		       MAC2STR(mgmt->da));
		return;
	}

	if (status_code == WLAN_STATUS_SUCCESS &&
	    ((auth_alg == WLAN_AUTH_OPEN && auth_transaction == 2) ||
	     (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 4))) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_INFO, "authenticated");
		sta->flags |= WLAN_STA_AUTH;
	}
}

#ifdef ASD_IEEE80211N
static void
asd_get_ht_capab(struct asd_data *wasd,
		     struct ht_cap_ie *ht_cap_ie,
		     struct ht_cap_ie *neg_ht_cap_ie)
{

	os_memcpy(neg_ht_cap_ie, ht_cap_ie, sizeof(struct ht_cap_ie));
	neg_ht_cap_ie->data.capabilities_info =
		ht_cap_ie->data.capabilities_info & wasd->iconf->ht_capab;

	neg_ht_cap_ie->data.capabilities_info &= ~HT_CAP_INFO_SMPS_DISABLED;
	if ((ht_cap_ie->data.capabilities_info & HT_CAP_INFO_SMPS_DISABLED) ==
	    (wasd->iconf->ht_capab & HT_CAP_INFO_SMPS_DISABLED))
		neg_ht_cap_ie->data.capabilities_info |=
			wasd->iconf->ht_capab & HT_CAP_INFO_SMPS_DISABLED;
	else
		neg_ht_cap_ie->data.capabilities_info |=
			HT_CAP_INFO_SMPS_DISABLED;

	/* FIXME: Rx STBC needs to be handled specially */
	neg_ht_cap_ie->data.capabilities_info &= ~HT_CAP_INFO_RX_STBC_MASK;
	neg_ht_cap_ie->data.capabilities_info |=
		wasd->iconf->ht_capab & HT_CAP_INFO_RX_STBC_MASK;
}
#endif /* ASD_IEEE80211N */

static void handle_assoc_cb(struct asd_data *wasd,
			    struct ieee80211_mgmt *mgmt,
			    size_t len, int reassoc, int ok)
{
	u16 status;
	struct sta_info *sta;
	int new_assoc = 1;
#ifdef ASD_IEEE80211N
	struct ht_cap_ie ht_cap;
#endif /* ASD_IEEE80211N */
	struct ht_cap_ie *ht_cap_ptr = NULL;

	if (!ok) {
		asd_logger(wasd, mgmt->da, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "did not acknowledge association response");
		return;
	}

	if (len < IEEE80211_HDRLEN + (reassoc ? sizeof(mgmt->u.reassoc_resp) :
				      sizeof(mgmt->u.assoc_resp))) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_assoc_cb(reassoc=%d) - too short payload "
		       "(len=%lu)\n", reassoc, (unsigned long) len);
		return;
	}

	if (reassoc)
		status = le_to_host16(mgmt->u.reassoc_resp.status_code);
	else
		status = le_to_host16(mgmt->u.assoc_resp.status_code);

	sta = ap_get_sta(wasd, mgmt->da);
	if (!sta) {
		asd_printf(ASD_80211,MSG_DEBUG,"handle_assoc_cb: STA " MACSTR " not found\n",
		       MAC2STR(mgmt->da));
		return;
	}

	if (status != WLAN_STATUS_SUCCESS)
		goto fail;

	/* Stop previous accounting session, if one is started, and allocate
	 * new session id for the new session. */
	accounting_sta_stop(wasd, sta);

	asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_INFO,
		       "associated (aid %d, accounting session %08X-%08X)",
		       sta->aid, sta->acct_session_id_hi,
		       sta->acct_session_id_lo);

	if (sta->flags & WLAN_STA_ASSOC)
		new_assoc = 0;
	sta->flags |= WLAN_STA_ASSOC;

	if (reassoc)
		mlme_reassociate_indication(wasd, sta);
	else
		mlme_associate_indication(wasd, sta);

#ifdef ASD_IEEE80211N
	if (sta->flags & WLAN_STA_HT) {
		ht_cap_ptr = &ht_cap;
		asd_get_ht_capab(wasd, &sta->ht_capabilities, ht_cap_ptr);
	}
#endif /* ASD_IEEE80211N */

	if (asd_sta_add(wasd->conf->iface, wasd, sta->addr, sta->aid,
			    sta->capability, sta->supported_rates,
			    sta->supported_rates_len, 0)) {
		asd_logger(wasd, sta->addr, asd_MODULE_IEEE80211,
			       asd_LEVEL_NOTICE,
			       "Could not add STA to kernel driver");
	}

	if (sta->eapol_sm == NULL) {
		/*
		 * This STA does not use RADIUS server for EAP authentication,
		 * so bind it to the selected VLAN interface now, since the
		 * interface selection is not going to change anymore.
		 */
		ap_sta_bind_vlan(wasd, sta, 0);
	} else if (sta->vlan_id) {
		/* VLAN ID already set (e.g., by PMKSA caching), so bind STA */
		ap_sta_bind_vlan(wasd, sta, 0);
	}
	if (sta->flags & WLAN_STA_SHORT_PREAMBLE) {
		asd_sta_set_flags(wasd, sta->addr, sta->flags,
				      WLAN_STA_SHORT_PREAMBLE, ~0);
	} else {
		asd_sta_set_flags(wasd, sta->addr, sta->flags,
				      0, ~WLAN_STA_SHORT_PREAMBLE);
	}

	if (sta->auth_alg == WLAN_AUTH_FT)
		wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC_FT);
	else
		wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);
	asd_new_assoc_sta(wasd, sta, !new_assoc);

	ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);

 fail:
	/* Copy of the association request is not needed anymore */
	if (sta->last_assoc_req) {
		os_free(sta->last_assoc_req);
		sta->last_assoc_req = NULL;
	}
}


void ieee802_11_mgmt_cb(struct asd_data *wasd, u8 *buf, size_t len,
			u16 stype, int ok)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;

	switch (stype) {
	case WLAN_FC_STYPE_AUTH:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::auth cb");
		handle_auth_cb(wasd, mgmt, len, ok);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::assoc_resp cb");
		handle_assoc_cb(wasd, mgmt, len, 0, ok);
		break;
	case WLAN_FC_STYPE_REASSOC_RESP:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::reassoc_resp cb");
		handle_assoc_cb(wasd, mgmt, len, 1, ok);
		break;
	case WLAN_FC_STYPE_PROBE_RESP:
		asd_printf(ASD_80211,MSG_DEBUG, "mgmt::proberesp cb");
		break;
	case WLAN_FC_STYPE_DEAUTH:
		/* ignore */
		break;
	default:
		asd_printf(ASD_80211,MSG_DEBUG,"unknown mgmt cb frame subtype %d\n", stype);
		break;
	}
}


static void ieee80211_tkip_countermeasures_stop(void *circle_ctx,
						void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	wasd->tkip_countermeasures = 0;
	asd_set_countermeasures(wasd, 0);
	asd_logger(wasd, NULL, asd_MODULE_IEEE80211,
		       asd_LEVEL_INFO, "TKIP countermeasures ended");
}


static void ieee80211_tkip_countermeasures_start(struct asd_data *wasd)
{
	struct sta_info *sta;

	asd_logger(wasd, NULL, asd_MODULE_IEEE80211,
		       asd_LEVEL_INFO, "TKIP countermeasures initiated");

	wpa_auth_countermeasures_start(wasd->wpa_auth);
	wasd->tkip_countermeasures = 1;
	asd_set_countermeasures(wasd, 1);
	wpa_gtk_rekey(wasd->wpa_auth);
	circle_cancel_timeout(ieee80211_tkip_countermeasures_stop, wasd, NULL);
	circle_register_timeout(60, 0, ieee80211_tkip_countermeasures_stop,
			       wasd, NULL);
	for (sta = wasd->sta_list; sta != NULL; sta = sta->next) {
		asd_sta_deauth(wasd, sta->addr,
				   WLAN_REASON_MICHAEL_MIC_FAILURE);
		if(1 == check_sta_authorized(wasd,sta))
			wasd->authorized_sta_num--;
		sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC |
				WLAN_STA_AUTHORIZED);
		asd_sta_remove(wasd, sta->addr);
	}
}


void ieee80211_michael_mic_failure(struct asd_data *wasd, const u8 *addr,
				   int local)
{
	time_t now;

	if (addr && local) {
		struct sta_info *sta = ap_get_sta(wasd, addr);
		if (sta != NULL) {
			wpa_auth_sta_local_mic_failure_report(sta->wpa_sm);
			asd_logger(wasd, addr, asd_MODULE_IEEE80211,
				       asd_LEVEL_INFO,
				       "Michael MIC failure detected in "
				       "received frame");
			mlme_michaelmicfailure_indication(wasd, addr);
		} else {
			asd_printf(ASD_80211,MSG_DEBUG,
				   "MLME-MICHAELMICFAILURE.indication "
				   "for not associated STA (" MACSTR
				   ") ignored", MAC2STR(addr));
			return;
		}
	}

	time(&now);
	if (now > wasd->michael_mic_failure + 60) {
		wasd->michael_mic_failures = 1;
	} else {
		wasd->michael_mic_failures++;
		if (wasd->michael_mic_failures > 1)
			ieee80211_tkip_countermeasures_start(wasd);
	}
	wasd->michael_mic_failure = now;
}


int ieee802_11_get_mib(struct asd_data *wasd, char *buf, size_t buflen)
{
	/* TODO */
	return 0;
}


int ieee802_11_get_mib_sta(struct asd_data *wasd, struct sta_info *sta,
			   char *buf, size_t buflen)
{
	/* TODO */
	return 0;
}


int Assmble_WSM_msg(DataMsg *aMsg, struct asd_data *wasd, const void *msg, size_t len, int type){

	aMsg->Type = type;
	aMsg->WTPID = wasd->Radio_G_ID/L_RADIO_NUM;
	aMsg->Radio_G_ID = wasd->Radio_G_ID;
	aMsg->BSSIndex = wasd->BSSIndex;
	aMsg->DataLen = len;
	memcpy(aMsg->Data, msg, len);
	asd_printf(ASD_80211,MSG_DEBUG,"data len %d\n",len);
	return len+20;
}
void log_parse_reason(int reasons,char *reas){
	switch(reasons){
		case OPERATE_SUCCESS:
			strcpy(reas,"OPERATE SUCCESS");
			break;
		case FLOW_BANLANCE:
			strcpy(reas,"Because of Flow Banlance");
			break;
		case NUMBER_BANLANCE:
			strcpy(reas,"Because of Number Banlance");
			break;
		case AUTH_ALG_FAIL:
			strcpy(reas,"Authentication Algorithm Failed");
			break;
		case AUTH_TRANSNUM_WRONG:
			strcpy(reas,"Authentication Transition Number Wrong");
			break;
		case STAMAC_BSSID:
			strcpy(reas,"");
			break;
		case MAC_REJECTED:
			strcpy(reas,"Mac Address Rejected");
			break;
		case VLANID_INVALID:
			strcpy(reas,"Invalid Vlan ID");
			break;
		case WPA_SM_FAILED:
			strcpy(reas,"");
			break;
		case NO_RESOURCE:
			strcpy(reas,"No More Resources to Use");
			break;
		case ASSO_BEFORE_AUTH:
			strcpy(reas,"Association before Authentication");
			break;
		case ASSO_PACKAGE_WRONG:
			strcpy(reas,"Package Wrong");
			break;
		case UNKNOWN_SSID:
			strcpy(reas,"Unknown SSID");
			break;
		case WME_ELEM_INVALID:
			strcpy(reas,"");
			break;
		case RATES_NOT_SUPPORT:
			strcpy(reas,"Not Supported Rates");
			break;
		case RATES_LEN_INVALID:
			strcpy(reas,"");
			break;
		case NO_WPARASN_IE:
			strcpy(reas,"");
			break;
		case CIPHER_NOT_MATCH:
			strcpy(reas,"Cipher Wrong");
			break;
		case NO_WAPI_IE:
			strcpy(reas,"");
			break;
		case STA_POWER_NOT_ACCEPTED:
			strcpy(reas,"Sta Power not Accespted");
			break;
		case NO_MORE_AID:
			strcpy(reas,"");
			break;
		case RADIUS_FAILED:
			strcpy(reas,"Radius Authentication Failed");
			break;
		case RADIUS_SUCCESS:
			strcpy(reas,"Radius Authentication Success");
			break;
		case PSK_SUCCESS:
			strcpy(reas,"Psk Exchange Success");
			break;
		case PSK_FAILED:
			strcpy(reas,"Psk Exchange Failed");
			break;
		default:
			break;
	}
}
void log_parse_reason_80211(int reasons,char *reas){
	switch(reasons){
		case WLAN_REASON_UNSPECIFIED:
			strcpy(reas,"Unspecified");
			break;
		case WLAN_REASON_PREV_AUTH_NOT_VALID:
			strcpy(reas,"Prev Authentication Not Valid");
			break;
		case WLAN_REASON_DEAUTH_LEAVING:
			strcpy(reas,"Deauth because of Sta Leaving");
			break;
		case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
			strcpy(reas,"Inactivity");
			break;
		case WLAN_REASON_DISASSOC_AP_BUSY:
			strcpy(reas,"Ap Busy");
			break;
		case WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:
			strcpy(reas,"Non Authentication Sta");
			break;
		case WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:
			strcpy(reas,"Non Association Sta");
			break;
		case WLAN_REASON_DISASSOC_STA_HAS_LEFT:
			strcpy(reas,"Disassoc because of Sta Leaving");
			break;
		case WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH:
			strcpy(reas,"Sta Association befor Authentication");
			break;
		case WLAN_REASON_PWR_CAPABILITY_NOT_VALID:
			strcpy(reas,"Not Supported Power");
			break;
		case WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID:
			strcpy(reas,"Not Supported Channel");
			break;
		case WLAN_REASON_INVALID_IE:
			strcpy(reas,"Invalid IE");
			break;
		case WLAN_REASON_MICHAEL_MIC_FAILURE:
			strcpy(reas,"Michael Mic Failure");
			break;
		case WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT:
			strcpy(reas,"4Way Handshake Timeout");
			break;
		case WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT:
			strcpy(reas,"Group Key Update Timeout");
			break;
		case WLAN_REASON_IE_IN_4WAY_DIFFERS:
			strcpy(reas,"IE in 4Way differs");
			break;
		case WLAN_REASON_GROUP_CIPHER_NOT_VALID:
			strcpy(reas,"Group Cipher not Valid");
			break;
		case WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID:
			strcpy(reas,"Pairwise Cipher not Valid");
			break;
		case WLAN_REASON_AKMP_NOT_VALID:
			strcpy(reas,"Akmp not Valid");
			break;
		case WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION:
			strcpy(reas,"Unsupported Rsn IE Version");
			break;
		case WLAN_REASON_INVALID_RSN_IE_CAPAB:
			strcpy(reas,"Invalid Rsn IE Capability");
			break;
		case WLAN_REASON_IEEE_802_1X_AUTH_FAILED:
			strcpy(reas,"802.1X Authentication Failed");
			break;
		case WLAN_REASON_CIPHER_SUITE_REJECTED:
			strcpy(reas,"Cipher Suite Rejected");
			break;
		default:
			break;
	}
}

