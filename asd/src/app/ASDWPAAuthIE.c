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
* AsdWapAuthIE.c
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
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"

#include "common.h"
#include "config.h"
#include "ASD80211Op.h"
#include "ASDEapolSM.h"
#include "ASDWPAOp.h"
#include "ASDPMKCache.h"
#include "ASDWPAAuthIE.h"
#include "ASDWPAAuthI.h"
#include "ASDPMKSta.h"
#include "wcpss/asd/asd.h"




static int wpa_write_wpa_ie(struct wpa_auth_config *conf, u8 *buf, size_t len)
{
	struct wpa_ie_hdr *hdr;
	int num_suites;
	u8 *pos, *count;

	hdr = (struct wpa_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_VENDOR_SPECIFIC;
	RSN_SELECTOR_PUT(hdr->oui, WPA_OUI_TYPE);
	WPA_PUT_LE16(hdr->version, WPA_VERSION);
	pos = (u8 *) (hdr + 1);

	if (conf->wpa_group == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_CCMP);
	} else if (conf->wpa_group == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_TKIP);
	} else if (conf->wpa_group == WPA_CIPHER_WEP104) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_WEP104);
	} else if (conf->wpa_group == WPA_CIPHER_WEP40) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_WEP40);
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid group cipher (%d).",
			   conf->wpa_group);
		return -1;
	}
	pos += WPA_SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_pairwise & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_CCMP);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_pairwise & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_TKIP);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_pairwise & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, WPA_CIPHER_SUITE_NONE);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid pairwise cipher (%d).",
			   conf->wpa_pairwise);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += WPA_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid key management type (%d).",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* WPA Capabilities; use defaults, so no need to include it */

	hdr->len = (pos - buf) - 2;

	return pos - buf;
}


int wpa_write_rsn_ie(struct wpa_auth_config *conf, u8 *buf, size_t len,
		     const u8 *pmkid)
{
	struct rsn_ie_hdr *hdr;
	int num_suites;
	u8 *pos, *count;
	u16 capab;

	hdr = (struct rsn_ie_hdr *) buf;
	hdr->elem_id = WLAN_EID_RSN;
	WPA_PUT_LE16(hdr->version, RSN_VERSION);
	pos = (u8 *) (hdr + 1);

	if (conf->wpa_group == WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
	} else if (conf->wpa_group == WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
	} else if (conf->wpa_group == WPA_CIPHER_WEP104) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP104);
	} else if (conf->wpa_group == WPA_CIPHER_WEP40) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_WEP40);
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid group cipher (%d).",
			   conf->wpa_group);
		return -1;
	}
	pos += RSN_SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->rsn_pairwise & WPA_CIPHER_CCMP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->rsn_pairwise & WPA_CIPHER_TKIP) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->rsn_pairwise & WPA_CIPHER_NONE) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_NONE);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid pairwise cipher (%d).",
			   conf->rsn_pairwise);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_UNSPEC_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
#ifdef ASD_IEEE80211R
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_FT_IEEE8021X) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_FT_802_1X);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (conf->wpa_key_mgmt & WPA_KEY_MGMT_FT_PSK) {
		RSN_SELECTOR_PUT(pos, RSN_AUTH_KEY_MGMT_FT_PSK);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
#endif /* ASD_IEEE80211R */

	if (num_suites == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid key management type (%d).",
			   conf->wpa_key_mgmt);
		return -1;
	}
	WPA_PUT_LE16(count, num_suites);

	/* RSN Capabilities */
	capab = 0;
	if (conf->rsn_preauth)
		capab |= WPA_CAPABILITY_PREAUTH;
	if (conf->peerkey)
		capab |= WPA_CAPABILITY_PEERKEY_ENABLED;
	if (conf->wme_enabled) {
		/* 4 PTKSA replay counters when using WME */
		capab |= (RSN_NUM_REPLAY_COUNTERS_16 << 2);
	}
#ifdef ASD_IEEE80211W
	if (conf->ieee80211w != WPA_NO_IEEE80211W)
		capab |= WPA_CAPABILITY_MGMT_FRAME_PROTECTION;
#endif /* ASD_IEEE80211W */
	WPA_PUT_LE16(pos, capab);
	pos += 2;

	if (pmkid) {
		if (pos + 2 + PMKID_LEN > buf + len)
			return -1;
		/* PMKID Count */
		WPA_PUT_LE16(pos, 1);
		pos += 2;
		os_memcpy(pos, pmkid, PMKID_LEN);
		pos += PMKID_LEN;
	}

#ifdef ASD_IEEE80211W
	if (conf->ieee80211w != WPA_NO_IEEE80211W) {
		if (pos + 2 + 4 > buf + len)
			return -1;
		if (pmkid == NULL) {
			/* PMKID Count */
			WPA_PUT_LE16(pos, 0);
			pos += 2;
		}

		/* Management Group Cipher Suite */
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_AES_128_CMAC);
		pos += RSN_SELECTOR_LEN;
	}
#endif /* ASD_IEEE80211W */

	hdr->len = (pos - buf) - 2;

	return pos - buf;
}


int wpa_auth_gen_wpa_ie(struct wpa_authenticator *wpa_auth)
{
	u8 *pos, buf[128];
	int res;

	pos = buf;

	if (wpa_auth->conf.wpa & WPA_PROTO_RSN) {
		res = wpa_write_rsn_ie(&wpa_auth->conf,
				       pos, buf + sizeof(buf) - pos, NULL);
		if (res < 0)
			return res;
		pos += res;
	}
#ifdef ASD_IEEE80211R
	if (wpa_auth->conf.wpa_key_mgmt &
	    (WPA_KEY_MGMT_FT_IEEE8021X | WPA_KEY_MGMT_FT_PSK)) {
		res = wpa_write_mdie(&wpa_auth->conf, pos,
				     buf + sizeof(buf) - pos);
		if (res < 0)
			return res;
		pos += res;
	}
#endif /* ASD_IEEE80211R */
	if (wpa_auth->conf.wpa & WPA_PROTO_WPA) {
		res = wpa_write_wpa_ie(&wpa_auth->conf,
				       pos, buf + sizeof(buf) - pos);
		if (res < 0)
			return res;
		pos += res;
	}

	os_free(wpa_auth->wpa_ie);
	wpa_auth->wpa_ie = os_zalloc(pos - buf);
	if (wpa_auth->wpa_ie == NULL)
		return -1;
	os_memcpy(wpa_auth->wpa_ie, buf, pos - buf);
	wpa_auth->wpa_ie_len = pos - buf;

	return 0;
}


u8 * wpa_add_kde(u8 *pos, u32 kde, const u8 *data, size_t data_len,
		 const u8 *data2, size_t data2_len)
{
	*pos++ = WLAN_EID_VENDOR_SPECIFIC;
	*pos++ = RSN_SELECTOR_LEN + data_len + data2_len;
	RSN_SELECTOR_PUT(pos, kde);
	pos += RSN_SELECTOR_LEN;
	os_memcpy(pos, data, data_len);
	pos += data_len;
	if (data2) {
		os_memcpy(pos, data2, data2_len);
		pos += data2_len;
	}
	return pos;
}


static int wpa_selector_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;
	return 0;
}


static int wpa_key_mgmt_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_NONE)
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}


static int wpa_parse_wpa_ie_wpa(const u8 *wpa_ie, size_t wpa_ie_len,
				struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const u8 *pos;
	int left;
	int i, count;

	os_memset(data, 0, sizeof(*data));
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->mgmt_group_cipher = 0;

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr))
		return -1;

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != WLAN_EID_VENDOR_SPECIFIC ||
	    hdr->len != wpa_ie_len - 2 ||
	    RSN_SELECTOR_GET(hdr->oui) != WPA_OUI_TYPE ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		return -2;
	}

	pos = (const u8 *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0)
		  return -3;

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN)
			return -4;
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1)
		return -5;

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN)
			return -6;
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1)
		return -7;

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		return -8;
	}

	return 0;
}


int wpa_validate_wpa_ie(struct wpa_authenticator *wpa_auth,
			struct wpa_state_machine *sm,
			const u8 *wpa_ie, size_t wpa_ie_len,
			const u8 *mdie, size_t mdie_len, 
			unsigned char WLANID, unsigned int BSSIndex)
{
	struct wpa_ie_data data;
	int ciphers, key_mgmt, res, version;
	u32 selector;
	size_t i;
	struct rsn_pmksa_cache_entry * entry;
	unsigned char pmkid[PMKID_LEN];
	if (wpa_auth == NULL || sm == NULL)
		return WPA_NOT_ENABLED;

	if (wpa_ie == NULL || wpa_ie_len < 1)
		return WPA_INVALID_IE;

	if (wpa_ie[0] == WLAN_EID_RSN)
		version = WPA_PROTO_RSN;
	else
		version = WPA_PROTO_WPA;

	if (version == WPA_PROTO_RSN) {
		res = wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, &data);

		selector = RSN_AUTH_KEY_MGMT_UNSPEC_802_1X;
		if (0) {
		}
#ifdef ASD_IEEE80211R
		else if (data.key_mgmt & WPA_KEY_MGMT_FT_IEEE8021X)
			selector = RSN_AUTH_KEY_MGMT_FT_802_1X;
		else if (data.key_mgmt & WPA_KEY_MGMT_FT_PSK)
			selector = RSN_AUTH_KEY_MGMT_FT_PSK;
#endif /* ASD_IEEE80211R */
		else if (data.key_mgmt & WPA_KEY_MGMT_IEEE8021X)
			selector = RSN_AUTH_KEY_MGMT_UNSPEC_802_1X;
		else if (data.key_mgmt & WPA_KEY_MGMT_PSK)
			selector = RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X;
		wpa_auth->dot11RSNAAuthenticationSuiteSelected = selector;

		selector = RSN_CIPHER_SUITE_CCMP;
		if (data.pairwise_cipher & WPA_CIPHER_CCMP)
			selector = RSN_CIPHER_SUITE_CCMP;
		else if (data.pairwise_cipher & WPA_CIPHER_TKIP)
			selector = RSN_CIPHER_SUITE_TKIP;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP104)
			selector = RSN_CIPHER_SUITE_WEP104;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP40)
			selector = RSN_CIPHER_SUITE_WEP40;
		else if (data.pairwise_cipher & WPA_CIPHER_NONE)
			selector = RSN_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAPairwiseCipherSelected = selector;

		selector = RSN_CIPHER_SUITE_CCMP;
		if (data.group_cipher & WPA_CIPHER_CCMP)
			selector = RSN_CIPHER_SUITE_CCMP;
		else if (data.group_cipher & WPA_CIPHER_TKIP)
			selector = RSN_CIPHER_SUITE_TKIP;
		else if (data.group_cipher & WPA_CIPHER_WEP104)
			selector = RSN_CIPHER_SUITE_WEP104;
		else if (data.group_cipher & WPA_CIPHER_WEP40)
			selector = RSN_CIPHER_SUITE_WEP40;
		else if (data.group_cipher & WPA_CIPHER_NONE)
			selector = RSN_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAGroupCipherSelected = selector;
	} else {
		res = wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, &data);

		selector = WPA_AUTH_KEY_MGMT_UNSPEC_802_1X;
		if (data.key_mgmt & WPA_KEY_MGMT_IEEE8021X)
			selector = WPA_AUTH_KEY_MGMT_UNSPEC_802_1X;
		else if (data.key_mgmt & WPA_KEY_MGMT_PSK)
			selector = WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X;
		wpa_auth->dot11RSNAAuthenticationSuiteSelected = selector;

		selector = WPA_CIPHER_SUITE_TKIP;
		if (data.pairwise_cipher & WPA_CIPHER_CCMP)
			selector = WPA_CIPHER_SUITE_CCMP;
		else if (data.pairwise_cipher & WPA_CIPHER_TKIP)
			selector = WPA_CIPHER_SUITE_TKIP;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP104)
			selector = WPA_CIPHER_SUITE_WEP104;
		else if (data.pairwise_cipher & WPA_CIPHER_WEP40)
			selector = WPA_CIPHER_SUITE_WEP40;
		else if (data.pairwise_cipher & WPA_CIPHER_NONE)
			selector = WPA_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAPairwiseCipherSelected = selector;

		selector = WPA_CIPHER_SUITE_TKIP;
		if (data.group_cipher & WPA_CIPHER_CCMP)
			selector = WPA_CIPHER_SUITE_CCMP;
		else if (data.group_cipher & WPA_CIPHER_TKIP)
			selector = WPA_CIPHER_SUITE_TKIP;
		else if (data.group_cipher & WPA_CIPHER_WEP104)
			selector = WPA_CIPHER_SUITE_WEP104;
		else if (data.group_cipher & WPA_CIPHER_WEP40)
			selector = WPA_CIPHER_SUITE_WEP40;
		else if (data.group_cipher & WPA_CIPHER_NONE)
			selector = WPA_CIPHER_SUITE_NONE;
		wpa_auth->dot11RSNAGroupCipherSelected = selector;
	}
	if (res) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Failed to parse WPA/RSN IE from "
			   MACSTR " (res=%d)", MAC2STR(sm->addr), res);
		wpa_hexdump(MSG_DEBUG, "WPA/RSN IE", wpa_ie, wpa_ie_len);
		return WPA_INVALID_IE;
	}

	if (data.group_cipher != wpa_auth->conf.wpa_group) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid WPA group cipher (0x%x) from "
			   MACSTR, data.group_cipher, MAC2STR(sm->addr));
		return WPA_INVALID_GROUP;
	}

	key_mgmt = data.key_mgmt & wpa_auth->conf.wpa_key_mgmt;
	if (!key_mgmt) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid WPA key mgmt (0x%x) from "
			   MACSTR, data.key_mgmt, MAC2STR(sm->addr));
		return WPA_INVALID_AKMP;
	}
	if (0) {
	}
#ifdef ASD_IEEE80211R
	else if (key_mgmt & WPA_KEY_MGMT_FT_IEEE8021X)
		sm->wpa_key_mgmt = WPA_KEY_MGMT_FT_IEEE8021X;
	else if (key_mgmt & WPA_KEY_MGMT_FT_PSK)
		sm->wpa_key_mgmt = WPA_KEY_MGMT_FT_PSK;
#endif /* ASD_IEEE80211R */
	else if (key_mgmt & WPA_KEY_MGMT_IEEE8021X)
		sm->wpa_key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	else
		sm->wpa_key_mgmt = WPA_KEY_MGMT_PSK;

	if (version == WPA_PROTO_RSN)
		ciphers = data.pairwise_cipher & wpa_auth->conf.rsn_pairwise;
	else
		ciphers = data.pairwise_cipher & wpa_auth->conf.wpa_pairwise;
	if (!ciphers) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Invalid %s pairwise cipher (0x%x) "
			   "from " MACSTR,
			   version == WPA_PROTO_RSN ? "RSN" : "WPA",
			   data.pairwise_cipher, MAC2STR(sm->addr));
		return WPA_INVALID_PAIRWISE;
	}

#ifdef ASD_IEEE80211W
	if (wpa_auth->conf.ieee80211w == WPA_IEEE80211W_REQUIRED) {
		if (!(data.capabilities &
		      WPA_CAPABILITY_MGMT_FRAME_PROTECTION)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Management frame protection "
				   "required, but client did not enable it");
			return WPA_MGMT_FRAME_PROTECTION_VIOLATION;
		}

		if (ciphers & WPA_CIPHER_TKIP) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Management frame protection "
				   "cannot use TKIP");
			return WPA_MGMT_FRAME_PROTECTION_VIOLATION;
		}

		if (data.mgmt_group_cipher != WPA_CIPHER_AES_128_CMAC) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Unsupported management group "
				   "cipher %d", data.mgmt_group_cipher);
			return WPA_INVALID_MGMT_GROUP_CIPHER;
		}
	}

	if (wpa_auth->conf.ieee80211w == WPA_NO_IEEE80211W ||
	    !(data.capabilities & WPA_CAPABILITY_MGMT_FRAME_PROTECTION))
		sm->mgmt_frame_prot = 0;
	else
		sm->mgmt_frame_prot = 1;
#endif /* ASD_IEEE80211W */

#ifdef ASD_IEEE80211R
	if (sm->wpa_key_mgmt == WPA_KEY_MGMT_FT_IEEE8021X ||
	    sm->wpa_key_mgmt == WPA_KEY_MGMT_FT_PSK) {
		if (mdie == NULL || mdie_len < MOBILITY_DOMAIN_ID_LEN + 1) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: Trying to use FT, but "
				   "MDIE not included");
			return WPA_INVALID_MDIE;
		}
		if (os_memcmp(mdie, wpa_auth->conf.mobility_domain,
			      MOBILITY_DOMAIN_ID_LEN) != 0) {
			wpa_hexdump(MSG_DEBUG, "RSN: Attempted to use unknown "
				    "MDIE", mdie, MOBILITY_DOMAIN_ID_LEN);
			return WPA_INVALID_MDIE;
		}
	}
#endif /* ASD_IEEE80211R */

	if (ciphers & WPA_CIPHER_CCMP)
		sm->pairwise = WPA_CIPHER_CCMP;
	else
		sm->pairwise = WPA_CIPHER_TKIP;

	/* TODO: clear WPA/WPA2 state if STA changes from one to another */
	if (wpa_ie[0] == WLAN_EID_RSN)
		sm->wpa = WPA_VERSION_WPA2;
	else
		sm->wpa = WPA_VERSION_WPA;
	struct PMK_STAINFO *pmk_sta;
	if((data.num_pmkid == 0)&&((wpa_ie[0] == WLAN_EID_RSN))){
		pmk_sta = pmk_ap_get_sta(ASD_WLAN[WLANID],sm->addr);
		if(pmk_sta != NULL)
		{
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"we free sta in pre bssindex %d\n",pmk_sta->PreBssIndex);
		//	pmk_del_bssindex(pmk_sta->PreBssIndex,pmk_sta);
			if (pmk_sta->PreBssIndex != BSSIndex)
				pmk_ap_free_sta_in_prebss(pmk_sta->PreBssIndex,pmk_sta->addr);
		}
	}
	for (i = 0; i < data.num_pmkid; i++) {
		wpa_hexdump(MSG_DEBUG, "RSN IE: STA PMKID",
			    &data.pmkid[i * PMKID_LEN], PMKID_LEN);
		entry = pmksa_cache_get(wpa_auth->pmksa, sm->addr,
					    NULL);
		sm->pmksa = NULL;
		if(entry){
			if(memcmp(entry->pmkid, &data.pmkid[i * PMKID_LEN], PMKID_LEN)==0)
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"we get it in bssindex %d\n",BSSIndex);
				sm->pmksa = entry;				
				pmk_sta = pmk_ap_get_sta(ASD_WLAN[WLANID],sm->addr);
				if(pmk_sta != NULL)
				{
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"we free sta in pre bssindex %d\n",pmk_sta->PreBssIndex);
				//	pmk_del_bssindex(pmk_sta->PreBssIndex,pmk_sta);
					if (pmk_sta->PreBssIndex != BSSIndex)
						pmk_ap_free_sta_in_prebss(pmk_sta->PreBssIndex,pmk_sta->addr);
					pmk_sta->PreBssIndex = BSSIndex;
				}
			}else{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"check pmkid again\n");
				rsn_pmkid(entry->pmk,entry->pmk_len,sm->wpa_auth->addr,sm->addr,pmkid);
				if(memcmp(pmkid, &data.pmkid[i * PMKID_LEN], PMKID_LEN)==0){
					asd_printf(ASD_DEFAULT,MSG_DEBUG,"we get it by check\n");
					if (memcmp(entry->pmkid, pmkid, PMKID_LEN)) {
						wpa_hexdump(MSG_INFO, "STA PMKID :", entry->pmkid, PMKID_LEN);
						wpa_hexdump(MSG_INFO, "PMKID: ", pmkid, PMKID_LEN);
					}

					/* delete from hash, then update pmkid, insert again */
					pmksa_cache_delete_from_hash(wpa_auth->pmksa, entry);
					memcpy(entry->pmkid, pmkid, PMKID_LEN);
					pmksa_cache_insert_to_hash(wpa_auth->pmksa, entry);
					sm->pmksa = entry;
					pmk_sta = pmk_ap_get_sta(ASD_WLAN[WLANID],sm->addr);
					if(pmk_sta != NULL)
					{
						asd_printf(ASD_DEFAULT,MSG_DEBUG,"we free sta in pre bssindex %d\n",pmk_sta->PreBssIndex);
					//	pmk_del_bssindex(pmk_sta->PreBssIndex,pmk_sta);
						if (pmk_sta->PreBssIndex != BSSIndex)
							pmk_ap_free_sta_in_prebss(pmk_sta->PreBssIndex,pmk_sta->addr);
						pmk_sta->PreBssIndex = BSSIndex;
					}
				}
			}
		}
		if (sm->pmksa) {
			wpa_auth_vlogger(wpa_auth, sm->addr, LOGGER_DEBUG,
					 "PMKID found from PMKSA cache "
					 "eap_type=%d vlan_id=%d",
					 sm->pmksa->eap_type_authsrv,
					 sm->pmksa->vlan_id);
			os_memcpy(wpa_auth->dot11RSNAPMKIDUsed,
				  sm->pmksa->pmkid, PMKID_LEN);
			break;
		}else{
			sm->pmksa = wlan_pmksa_cache_get(WLANID, BSSIndex, sm->addr, wpa_auth->addr,
					    &data.pmkid[i * PMKID_LEN]);
			if(sm->pmksa){
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"I got it\n");
			}
		}
	}

	if (sm->wpa_ie == NULL || sm->wpa_ie_len < wpa_ie_len) {
		os_free(sm->wpa_ie);
		sm->wpa_ie = os_zalloc(wpa_ie_len);
		if (sm->wpa_ie == NULL)
			return WPA_ALLOC_FAIL;
	}
	os_memcpy(sm->wpa_ie, wpa_ie, wpa_ie_len);
	sm->wpa_ie_len = wpa_ie_len;

	return WPA_IE_OK;
}


/**
 * wpa_parse_generic - Parse EAPOL-Key Key Data Generic IEs
 * @pos: Pointer to the IE header
 * @end: Pointer to the end of the Key Data buffer
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, 1 if end mark is found, -1 on failure
 */
static int wpa_parse_generic(const u8 *pos, const u8 *end,
			     struct wpa_eapol_ie_parse *ie)
{
	if (pos[1] == 0)
		return 1;

	if (pos[1] >= 6 &&
	    RSN_SELECTOR_GET(pos + 2) == WPA_OUI_TYPE &&
	    pos[2 + WPA_SELECTOR_LEN] == 1 &&
	    pos[2 + WPA_SELECTOR_LEN + 1] == 0) {
		ie->wpa_ie = pos;
		ie->wpa_ie_len = pos[1] + 2;
		return 0;
	}

	if (pos + 1 + RSN_SELECTOR_LEN < end &&
	    pos[1] >= RSN_SELECTOR_LEN + PMKID_LEN &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_PMKID) {
		ie->pmkid = pos + 2 + RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_GROUPKEY) {
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_MAC_ADDR) {
		ie->mac_addr = pos + 2 + RSN_SELECTOR_LEN;
		ie->mac_addr_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

#ifdef ASD_PEERKEY
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_SMK) {
		ie->smk = pos + 2 + RSN_SELECTOR_LEN;
		ie->smk_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_NONCE) {
		ie->nonce = pos + 2 + RSN_SELECTOR_LEN;
		ie->nonce_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_LIFETIME) {
		ie->lifetime = pos + 2 + RSN_SELECTOR_LEN;
		ie->lifetime_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_ERROR) {
		ie->error = pos + 2 + RSN_SELECTOR_LEN;
		ie->error_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}
#endif /* ASD_PEERKEY */

#ifdef ASD_IEEE80211W
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_IGTK) {
		ie->igtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->igtk_len = pos[1] - RSN_SELECTOR_LEN;
		return 0;
	}
#endif /* ASD_IEEE80211W */

	return 0;
}


/**
 * wpa_parse_kde_ies - Parse EAPOL-Key Key Data IEs
 * @buf: Pointer to the Key Data buffer
 * @len: Key Data Length
 * @ie: Pointer to parsed IE data
 * Returns: 0 on success, -1 on failure
 */
int wpa_parse_kde_ies(const u8 *buf, size_t len, struct wpa_eapol_ie_parse *ie)
{
	const u8 *pos, *end;
	int ret = 0;

	os_memset(ie, 0, sizeof(*ie));
	for (pos = buf, end = pos + len; pos + 1 < end; pos += 2 + pos[1]) {
		if (pos[0] == 0xdd &&
		    ((pos == buf + len - 1) || pos[1] == 0)) {
			/* Ignore padding */
			break;
		}
		if (pos + 2 + pos[1] > end) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "WPA: EAPOL-Key Key Data "
				   "underflow (ie=%d len=%d pos=%d)",
				   pos[0], pos[1], (int) (pos - buf));
			wpa_hexdump_key(MSG_DEBUG, "WPA: Key Data",
					buf, len);
			ret = -1;
			break;
		}
		if (*pos == WLAN_EID_RSN) {
			ie->rsn_ie = pos;
			ie->rsn_ie_len = pos[1] + 2;
#ifdef ASD_IEEE80211R
		} else if (*pos == WLAN_EID_MOBILITY_DOMAIN) {
			ie->mdie = pos;
			ie->mdie_len = pos[1] + 2;
#endif /* ASD_IEEE80211R */
		} else if (*pos == WLAN_EID_VENDOR_SPECIFIC) {
			ret = wpa_parse_generic(pos, end, ie);
			if (ret < 0)
				break;
			if (ret > 0) {
				ret = 0;
				break;
			}
		} else {
			wpa_hexdump(MSG_DEBUG, "WPA: Unrecognized EAPOL-Key "
				    "Key Data IE", pos, 2 + pos[1]);
		}
	}

	return ret;
}
