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
* AsdWme.c
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
#include "ASDWme.h"
#include "ASDStaInfo.h"
#include "ASDCallback.h"


/* TODO: maintain separate sequence and fragment numbers for each AC
 * TODO: IGMP snooping to track which multicasts to forward - and use QOS-DATA
 * if only WME stations are receiving a certain group */


static u8 wme_oui[3] = { 0x00, 0x50, 0xf2 };


/* Add WME Parameter Element to Beacon and Probe Response frames. */
u8 * asd_eid_wme(struct asd_data *wasd, u8 *eid)
{
	u8 *pos = eid;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_eid_wme\n");
	struct wme_parameter_element *wme =
		(struct wme_parameter_element *) (pos + 2);
	int e;

	if (!wasd->conf->wme_enabled)
		return eid;
	eid[0] = WLAN_EID_VENDOR_SPECIFIC;
	wme->oui[0] = 0x00;
	wme->oui[1] = 0x50;
	wme->oui[2] = 0xf2;
	wme->oui_type = WME_OUI_TYPE;
	wme->oui_subtype = WME_OUI_SUBTYPE_PARAMETER_ELEMENT;
	wme->version = WME_VERSION;
	wme->acInfo = wasd->parameter_set_count & 0xf;

	/* fill in a parameter set record for each AC */
	for (e = 0; e < 4; e++) {
		struct wme_ac_parameter *ac = &wme->ac[e];
		struct asd_wme_ac_params *acp =
			&wasd->iconf->wme_ac_params[e];

		ac->aifsn = acp->aifs;
		ac->acm = acp->admission_control_mandatory;
		ac->aci = e;
		ac->reserved = 0;
		ac->eCWmin = acp->cwmin;
		ac->eCWmax = acp->cwmax;
		ac->txopLimit = host_to_le16(acp->txopLimit);
	}

	pos = (u8 *) (wme + 1);
	eid[1] = pos - eid - 2; /* element length */
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_eid_wme end\n");

	return pos;
}


/* This function is called when a station sends an association request with
 * WME info element. The function returns zero on success or non-zero on any
 * error in WME element. eid does not include Element ID and Length octets. */
int asd_eid_wme_valid(struct asd_data *wasd, u8 *eid, size_t len)
{
	struct wme_information_element *wme;

	wpa_hexdump(MSG_MSGDUMP, "WME IE", eid, len);

	if (len < sizeof(struct wme_information_element)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Too short WME IE (len=%lu)",
			   (unsigned long) len);
		return -1;
	}

	wme = (struct wme_information_element *) eid;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Validating WME IE: OUI %02x:%02x:%02x  "
		   "OUI type %d  OUI sub-type %d  version %d",
		   wme->oui[0], wme->oui[1], wme->oui[2], wme->oui_type,
		   wme->oui_subtype, wme->version);
	if (os_memcmp(wme->oui, wme_oui, sizeof(wme_oui)) != 0 ||
	    wme->oui_type != WME_OUI_TYPE ||
	    wme->oui_subtype != WME_OUI_SUBTYPE_INFORMATION_ELEMENT ||
	    wme->version != WME_VERSION) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Unsupported WME IE OUI/Type/Subtype/"
			   "Version");
		return -1;
	}

	return 0;
}


/* This function is called when a station sends an ACK frame for an AssocResp
 * frame (status=success) and the matching AssocReq contained a WME element.
 */
int asd_wme_sta_config(struct asd_data *wasd, struct sta_info *sta)
{
	/* update kernel STA data for WME related items (WLAN_STA_WPA flag) */
	if (sta->flags & WLAN_STA_WME)
		asd_sta_set_flags(wasd, sta->addr, sta->flags,
				      WLAN_STA_WME, ~0);
	else
		asd_sta_set_flags(wasd, sta->addr, sta->flags,
				      0, ~WLAN_STA_WME);

	return 0;
}


static void wme_send_action(struct asd_data *wasd, const u8 *addr,
			    const struct wme_tspec_info_element *tspec,
			    u8 action_code, u8 dialogue_token, u8 status_code)
{
	u8 buf[256];
	struct ieee80211_mgmt *m = (struct ieee80211_mgmt *) buf;
	struct wme_tspec_info_element *t =
		(struct wme_tspec_info_element *)
		m->u.action.u.wme_action.variable;
	int len;

	asd_logger(wasd, addr, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "action response - reason %d", status_code);
	os_memset(buf, 0, sizeof(buf));
	m->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					WLAN_FC_STYPE_ACTION);
	os_memcpy(m->da, addr, ETH_ALEN);
	os_memcpy(m->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(m->bssid, wasd->own_addr, ETH_ALEN);
	m->u.action.category = WME_ACTION_CATEGORY;
	m->u.action.u.wme_action.action_code = action_code;
	m->u.action.u.wme_action.dialog_token = dialogue_token;
	m->u.action.u.wme_action.status_code = status_code;
	os_memcpy(t, tspec, sizeof(struct wme_tspec_info_element));
	len = ((u8 *) (t + 1)) - buf;

	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, m, len, IEEE802_11_MGMT);

	if (asd_send_mgmt_frame(wasd, &msg, msglen, 0) < 0)
		perror("wme_send_action: send");
}


/* given frame data payload size in bytes, and data_rate in bits per second
 * returns time to complete frame exchange */
/* FIX: should not use floating point types */
static double wme_frame_exchange_time(int bytes, int data_rate, int encryption,
				      int cts_protection)
{
	/* TODO: account for MAC/PHY headers correctly */
	/* TODO: account for encryption headers */
	/* TODO: account for WDS headers */
	/* TODO: account for CTS protection */
	/* TODO: account for SIFS + ACK at minimum PHY rate */
	return (bytes + 400) * 8.0 / data_rate;
}


static void wme_setup_request(struct asd_data *wasd,
			      struct ieee80211_mgmt *mgmt,
			      struct wme_tspec_info_element *tspec, size_t len)
{
	/* FIX: should not use floating point types */
	double medium_time, pps;

	/* TODO: account for airtime and answer no to tspec setup requests
	 * when none left!! */

	pps = (tspec->mean_data_rate / 8.0) / tspec->nominal_msdu_size;
	medium_time = (tspec->surplus_bandwidth_allowance / 8) * pps *
		wme_frame_exchange_time(tspec->nominal_msdu_size,
					tspec->minimum_phy_rate, 0, 0);
	tspec->medium_time = medium_time * 1000000.0 / 32.0;

	wme_send_action(wasd, mgmt->sa, tspec, WME_ACTION_CODE_SETUP_RESPONSE,
			mgmt->u.action.u.wme_action.dialog_token,
			WME_SETUP_RESPONSE_STATUS_ADMISSION_ACCEPTED);
}


void asd_wme_action(struct asd_data *wasd, struct ieee80211_mgmt *mgmt,
			size_t len)
{
	int action_code;
	int left = len - IEEE80211_HDRLEN - 4;
	u8 *pos = ((u8 *) mgmt) + IEEE80211_HDRLEN + 4;
	struct ieee802_11_elems elems;
	struct sta_info *sta = ap_get_sta(wasd, mgmt->sa);

	/* check that the request comes from a valid station */
	if (!sta ||
	    (sta->flags & (WLAN_STA_ASSOC | WLAN_STA_WME)) !=
	    (WLAN_STA_ASSOC | WLAN_STA_WME)) {
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "wme action received is not from associated wme"
			       " station");
		/* TODO: respond with action frame refused status code */
		return;
	}

	/* extract the tspec info element */
	if (ieee802_11_parse_elems(wasd, pos, left, &elems, 1) == ParseFailed)
	{
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "asd_wme_action - could not parse wme "
			       "action");
		/* TODO: respond with action frame invalid parameters status
		 * code */
		return;
	}

	if (!elems.wme_tspec ||
	    elems.wme_tspec_len != (sizeof(struct wme_tspec_info_element) - 2))
	{
		asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
			       asd_LEVEL_DEBUG,
			       "asd_wme_action - missing or wrong length "
			       "tspec");
		/* TODO: respond with action frame invalid parameters status
		 * code */
		return;
	}

	/* TODO: check the request is for an AC with ACM set, if not, refuse
	 * request */

	action_code = mgmt->u.action.u.wme_action.action_code;
	switch (action_code) {
	case WME_ACTION_CODE_SETUP_REQUEST:
		wme_setup_request(wasd, mgmt, (struct wme_tspec_info_element *)
				  elems.wme_tspec, len);
		return;
#if 0
	/* TODO: needed for client implementation */
	case WME_ACTION_CODE_SETUP_RESPONSE:
		wme_setup_request(wasd, mgmt, len);
		return;
	/* TODO: handle station teardown requests */
	case WME_ACTION_CODE_TEARDOWN:
		wme_teardown(wasd, mgmt, len);
		return;
#endif
	}

	asd_logger(wasd, mgmt->sa, asd_MODULE_IEEE80211,
		       asd_LEVEL_DEBUG,
		       "asd_wme_action - unknown action code %d",
		       action_code);
}
