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
* AsdPreAuth.c
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

#ifdef ASD_RSN_PREAUTH

#include "asd.h"

#include "ASD8021XOp.h"
#include "circle.h"
#include "ASDStaInfo.h"
#include "wpa_common.h"
#include "ASDEapolSM.h"
#include "ASDWPAOp.h"
#include "ASDPreauth.h"


static const int dot11RSNAConfigPMKLifetime = 43200;

struct rsn_preauth_interface {
	struct rsn_preauth_interface *next;
	struct asd_data *wasd;
	struct l2_packet_data *l2;
	char *ifname;
	int ifindex;
};


static void rsn_preauth_receive(void *ctx, const u8 *src_addr,
				const u8 *buf, size_t len)
{
	struct rsn_preauth_interface *piface = ctx;
	struct asd_data *wasd = piface->wasd;
	struct ieee802_1x_hdr *hdr;
	struct sta_info *sta;
	struct l2_ethhdr *ethhdr;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: receive pre-auth packet "
		   "from interface '%s'", piface->ifname);
	if (len < sizeof(*ethhdr) + sizeof(*hdr)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: too short pre-auth packet "
			   "(len=%lu)", (unsigned long) len);
		return;
	}

	ethhdr = (struct l2_ethhdr *) buf;
	hdr = (struct ieee802_1x_hdr *) (ethhdr + 1);

	if (os_memcmp(ethhdr->h_dest, wasd->own_addr, ETH_ALEN) != 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: pre-auth for foreign address "
			   MACSTR, MAC2STR(ethhdr->h_dest));
		return;
	}

	sta = ap_get_sta(wasd, ethhdr->h_source);
	if (sta && (sta->flags & WLAN_STA_ASSOC)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: pre-auth for already association "
			   "STA " MACSTR, MAC2STR(sta->addr));
		return;
	}
	if (!sta && hdr->type == IEEE802_1X_TYPE_EAPOL_START) {
		sta = ap_sta_add(wasd, ethhdr->h_source ,0);
		if (sta == NULL)
			return;
		sta->flags = WLAN_STA_PREAUTH;

		ieee802_1x_new_station(wasd, sta);
		if (sta->eapol_sm == NULL) {
			//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
			if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1)
				ap_free_sta_without_wsm(wasd, sta,1);
			else
				ap_free_sta_without_wsm(wasd, sta,0);
			sta = NULL;
		} else {
			sta->eapol_sm->radius_identifier = -1;
			sta->eapol_sm->portValid = TRUE;
			sta->eapol_sm->flags |= EAPOL_SM_PREAUTH;
		}
	}
	if (sta == NULL)
		return;
	sta->preauth_iface = piface;
	ieee802_1x_receive(wasd, ethhdr->h_source, (u8 *) (ethhdr + 1),
			   len - sizeof(*ethhdr));
}


static int rsn_preauth_iface_add(struct asd_data *wasd, const char *ifname)
{
	struct rsn_preauth_interface *piface;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN pre-auth interface '%s'", ifname);

	piface = os_zalloc(sizeof(*piface));
	if (piface == NULL)
		return -1;
	piface->wasd = wasd;

	piface->ifname = os_strdup(ifname);
	if (piface->ifname == NULL) {
		goto fail1;
	}

	piface->l2 = l2_packet_init(piface->ifname, NULL, ETH_P_PREAUTH,
				    rsn_preauth_receive, piface, 1);
	if (piface->l2 == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to open register layer 2 access "
			   "to ETH_P_PREAUTH");
		goto fail2;
	}

	piface->next = wasd->preauth_iface;
	wasd->preauth_iface = piface;
	return 0;

fail2:
	os_free(piface->ifname);
fail1:
	os_free(piface);
	return -1;
}


void rsn_preauth_iface_deinit(struct asd_data *wasd)
{
	struct rsn_preauth_interface *piface, *prev;

	piface = wasd->preauth_iface;
	wasd->preauth_iface = NULL;
	while (piface) {
		prev = piface;
		piface = piface->next;
		l2_packet_deinit(prev->l2);
		os_free(prev->ifname);
		os_free(prev);
	}
}


int rsn_preauth_iface_init(struct asd_data *wasd)
{
	char *tmp, *start, *end;

	if (wasd->conf->rsn_preauth_interfaces == NULL)
		return 0;

	tmp = os_strdup(wasd->conf->rsn_preauth_interfaces);
	if (tmp == NULL)
		return -1;
	start = tmp;
	for (;;) {
		while (*start == ' ')
			start++;
		if (*start == '\0')
			break;
		end = os_strchr(start, ' ');
		if (end)
			*end = '\0';

		if (rsn_preauth_iface_add(wasd, start)) {
			rsn_preauth_iface_deinit(wasd);
			return -1;
		}

		if (end)
			start = end + 1;
		else
			break;
	}
	os_free(tmp);
	return 0;
}


static void rsn_preauth_finished_cb(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: Removing pre-authentication STA entry for "
		   MACSTR, MAC2STR(sta->addr));
	//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
	if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1)
		ap_free_sta(wasd, sta,1);
	else
		ap_free_sta(wasd, sta,0);
}


void rsn_preauth_finished(struct asd_data *wasd, struct sta_info *sta,
			  int success)
{
	const u8 *key;
	size_t len;
	asd_logger(wasd, sta->addr, asd_MODULE_WPA,
		       asd_LEVEL_INFO, "pre-authentication %s",
		       success ? "succeeded" : "failed");

	key = ieee802_1x_get_key(sta->eapol_sm, &len);
	if (len > PMK_LEN)
		len = PMK_LEN;
	if (success && key) {
		if (wpa_auth_pmksa_add_preauth(wasd->wpa_auth, key, len,
					       sta->addr,
					       dot11RSNAConfigPMKLifetime,
					       sta->eapol_sm) == 0) {
			asd_logger(wasd, sta->addr, asd_MODULE_WPA,
				       asd_LEVEL_DEBUG,
				       "added PMKSA cache entry (pre-auth)");
			
			unsigned char WLANID = wasd->WlanID;
			struct PMK_STAINFO *pmk_sta;
			if(ASD_WLAN[WLANID] != NULL){
				pmk_sta = pmk_ap_sta_add(ASD_WLAN[WLANID],sta->addr);				
				pmk_sta->idhi = sta->acct_session_id_hi;
				pmk_sta->idlo = sta->acct_session_id_lo;
				pmk_add_bssindex(wasd->BSSIndex,pmk_sta);
			}
		} else {
			asd_logger(wasd, sta->addr, asd_MODULE_WPA,
				       asd_LEVEL_DEBUG,
				       "failed to add PMKSA cache entry "
				       "(pre-auth)");
		}
	}

	/*
	 * Finish STA entry removal from timeout in order to avoid freeing
	 * STA data before the caller has finished processing.
	 */
	circle_register_timeout(0, 0, rsn_preauth_finished_cb, wasd, sta);
}


void rsn_preauth_send(struct asd_data *wasd, struct sta_info *sta,
		      u8 *buf, size_t len)
{
	struct rsn_preauth_interface *piface;
	struct l2_ethhdr *ethhdr;

	piface = wasd->preauth_iface;
	while (piface) {
		if (piface == sta->preauth_iface)
			break;
		piface = piface->next;
	}

	if (piface == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "RSN: Could not find pre-authentication "
			   "interface for " MACSTR, MAC2STR(sta->addr));
		return;
	}

	ethhdr = os_zalloc(sizeof(*ethhdr) + len);
	if (ethhdr == NULL)
		return;

	os_memcpy(ethhdr->h_dest, sta->addr, ETH_ALEN);
	os_memcpy(ethhdr->h_source, wasd->own_addr, ETH_ALEN);
	ethhdr->h_proto = htons(ETH_P_PREAUTH);
	os_memcpy(ethhdr + 1, buf, len);

	if (l2_packet_send(piface->l2, sta->addr, ETH_P_PREAUTH, (u8 *) ethhdr,
			   sizeof(*ethhdr) + len) < 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to send preauth packet using "
			   "l2_packet_send\n");
	}
	os_free(ethhdr);
}


void rsn_preauth_free_station(struct asd_data *wasd, struct sta_info *sta)
{
	circle_cancel_timeout(rsn_preauth_finished_cb, wasd, sta);
}

#endif /* ASD_RSN_PREAUTH */
