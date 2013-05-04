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
* AsdReConfig.c
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
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDCallback.h"
#include "ASDStaInfo.h"
#include "ASDRadius/radius_client.h"
#include "ASD80211Op.h"
#include "ASDIappOp.h"
#include "ASDApList.h"
#include "ASDWPAOp.h"
#include "vlan_init.h"
#include "ASD80211AuthOp.h"
#include "ASD8021XOp.h"
#include "ASDAccounting.h"
#include "circle.h"


/**
 * struct asd_config_change - Configuration change information
 * This is for two purposes:
 * - Storing configuration information in the asd_iface during
 *   the asynchronous parts of reconfiguration.
 * - Passing configuration information for per-station reconfiguration.
 */
struct asd_config_change {
	struct asd_data *wasd;
	struct asd_config *newconf, *oldconf;
	struct asd_bss_config *newbss, *oldbss;
	int mac_acl_changed;
	int num_sta_remove; /* number of STAs that need to be removed */
	int beacon_changed;
	struct asd_iface *wasd_iface;
	struct asd_data **new_wasd, **old_wasd;
	int num_old_wasd;
};


static int asd_config_reload_sta(struct asd_data *wasd,
				     struct sta_info *sta, void *data)
{
	struct asd_config_change *change = data;
	struct asd_bss_config *newbss, *oldbss;
	int deauth = 0;
	u8 reason = WLAN_REASON_PREV_AUTH_NOT_VALID;

	newbss = change->newbss;
	oldbss = change->oldbss;
	wasd = change->wasd;

	if (sta->ssid == &oldbss->ssid) {
		sta->ssid = &newbss->ssid;

		if (newbss->ssid.ssid_len != oldbss->ssid.ssid_len ||
		    os_memcmp(newbss->ssid.ssid, oldbss->ssid.ssid,
			      newbss->ssid.ssid_len) != 0) {
			/* main SSID was changed - kick STA out */
			deauth++;
		}
	}
	sta->ssid_probe = sta->ssid;

	/*
	 * If MAC ACL configuration has changed, deauthenticate stations that
	 * have been removed from accepted list or have been added to denied
	 * list. If external RADIUS server is used for ACL, all stations are
	 * deauthenticated and they will need to authenticate again. This
	 * limits sudden load on the RADIUS server since the verification will
	 * be done over the time needed for the STAs to reauthenticate
	 * themselves.
	 */
/*	 
	if (change->mac_acl_changed &&
	    (newbss->macaddr_acl == USE_EXTERNAL_RADIUS_AUTH ||
	     !asd_allowed_address(wasd, sta->addr, NULL, 0, NULL, NULL,
				      NULL)))
		deauth++;
*/
	if (newbss->ieee802_1x != oldbss->ieee802_1x &&
	    sta->ssid == &wasd->conf->ssid)
		deauth++;

	if (newbss->wpa != oldbss->wpa)
		deauth++;

	if (!newbss->wme_enabled && (sta->flags & WLAN_STA_WME))
		deauth++;

	if (newbss->auth_algs != oldbss->auth_algs &&
	    ((sta->auth_alg == WLAN_AUTH_OPEN &&
	      !(newbss->auth_algs & WPA_AUTH_ALG_OPEN)) ||
	     (sta->auth_alg == WLAN_AUTH_SHARED_KEY &&
	      !(newbss->auth_algs & WPA_AUTH_ALG_SHARED))))
		deauth++;

	if (change->num_sta_remove > 0) {
		deauth++;
		reason = WLAN_REASON_DISASSOC_AP_BUSY;
	}

	if (deauth) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "STA " MACSTR " deauthenticated during "
			   "config reloading (reason=%d)",
			   MAC2STR(sta->addr), reason);
		ieee802_11_send_deauth(wasd, sta->addr, reason);
		ap_sta_deauthenticate(wasd, sta, reason);
		change->num_sta_remove--;
	}

	return 0;
}


static void asd_reconfig_tx_queue_params(struct asd_data *wasd,
					     struct asd_config *newconf,
					     struct asd_config *oldconf)
{
	int i;
	struct asd_tx_queue_params *o, *n;

	for (i = 0; i < NUM_TX_QUEUES; i++) {
		o = &oldconf->tx_queue[i];
		n = &newconf->tx_queue[i];

		if (!n->configured)
			continue;

		if ((n->aifs != o->aifs || n->cwmin != o->cwmin ||
		     n->cwmax != o->cwmax || n->burst != o->burst) &&
		    asd_set_tx_queue_params(wasd, i, n->aifs, n->cwmin,
						n->cwmax, n->burst))
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set TX queue parameters for queue %d"
			       ".\n", i);
	}
}


static int asd_reconfig_wme(struct asd_data *wasd,
				struct asd_config *newconf,
				struct asd_config *oldconf)
{
	int beacon_changed = 0;
	size_t i;
	struct asd_wme_ac_params *o, *n;

	for (i = 0; i < sizeof(newconf->wme_ac_params) /
			sizeof(newconf->wme_ac_params[0]); i++) {
		o = &oldconf->wme_ac_params[i];
		n = &newconf->wme_ac_params[i];
		if (n->cwmin != o->cwmin ||
		    n->cwmax != o->cwmax ||
		    n->aifs != o->aifs ||
		    n->txopLimit != o->txopLimit ||
		    n->admission_control_mandatory !=
		    o->admission_control_mandatory) {
			beacon_changed++;
			wasd->parameter_set_count++;
		}
	}

	return beacon_changed;
}


static int rate_array_diff(int *a1, int *a2)
{
	int i;

	if (a1 == NULL && a2 == NULL)
		return 0;
	if (a1 == NULL || a2 == NULL)
		return 1;

	i = 0;
	for (;;) {
		if (a1[i] != a2[i])
			return 1;
		if (a1[i] == -1)
			break;
		i++;
	}

	return 0;
}


static int asd_acl_diff(struct asd_bss_config *a,
			    struct asd_bss_config *b)
{
	return 0;
}
/*
{
	int i;

	if (a->macaddr_acl != b->macaddr_acl ||
	    a->num_accept_mac != b->num_accept_mac ||
	    a->num_deny_mac != b->num_deny_mac)
		return 1;

	for (i = 0; i < a->num_accept_mac; i++) {
		if (os_memcmp(a->accept_mac[i], b->accept_mac[i], ETH_ALEN) !=
		    0)
			return 1;
	}

	for (i = 0; i < a->num_deny_mac; i++) {
		if (os_memcmp(a->deny_mac[i], b->deny_mac[i], ETH_ALEN) != 0)
			return 1;
	}

	return 0;
}
*/

/**
 * reload_iface2 - Part 2 of reload_iface
 * @wasd_iface: Pointer to asd interface data.
 */
static void reload_iface2(struct asd_iface *wasd_iface)
{
	struct asd_data *wasd = wasd_iface->bss[0];
	struct asd_config *newconf = wasd_iface->change->newconf;
	struct asd_config *oldconf = wasd_iface->change->oldconf;
	int beacon_changed = wasd_iface->change->beacon_changed;
	asd_iface_cb cb = wasd_iface->reload_iface_cb;

	if (newconf->preamble != oldconf->preamble) {
		if (asd_set_preamble(wasd, wasd->iconf->preamble))
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set preamble for kernel driver\n");
		beacon_changed++;
	}

	if (newconf->beacon_int != oldconf->beacon_int) {
		/* Need to change beacon interval if it has changed or if
		 * auto channel selection was used. */
		if (asd_set_beacon_int(wasd, newconf->beacon_int))
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set beacon interval for kernel "
			       "driver\n");
		if (newconf->beacon_int != oldconf->beacon_int)
			beacon_changed++;
	}

	if (newconf->cts_protection_type != oldconf->cts_protection_type)
		beacon_changed++;

	if (newconf->rts_threshold > -1 &&
	    newconf->rts_threshold != oldconf->rts_threshold &&
	    asd_set_rts(wasd, newconf->rts_threshold))
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set RTS threshold for kernel driver\n");

	if (newconf->fragm_threshold > -1 &&
	    newconf->fragm_threshold != oldconf->fragm_threshold &&
	    asd_set_frag(wasd, newconf->fragm_threshold))
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set fragmentation threshold for kernel "
		       "driver\n");

	asd_reconfig_tx_queue_params(wasd, newconf, oldconf);

	if (asd_reconfig_wme(wasd, newconf, oldconf) > 0)
		beacon_changed++;

	ap_list_reconfig(wasd_iface, oldconf);

	wasd_iface->change->beacon_changed = beacon_changed;

	wasd_iface->reload_iface_cb = NULL;
	cb(wasd_iface, 0);
}


/**
 * reload_iface2_handler - Handler that calls reload_face2
 * @circle_data: Stores the struct asd_iface for the interface.
 * @user_ctx: Unused.
 */
static void reload_iface2_handler(void *circle_data, void *user_ctx)
{
	struct asd_iface *wasd_iface = circle_data;

	reload_iface2(wasd_iface);
}


/**
 * reload_hw_mode_done - Callback for after the HW mode is setup
 * @wasd_iface: Pointer to interface data.
 * @status: Status of the HW mode setup.
 */
static void reload_hw_mode_done(struct asd_iface *wasd_iface, int status)
{
	struct asd_data *wasd = wasd_iface->bss[0];
	struct asd_config_change *change = wasd_iface->change;
	struct asd_config *newconf = change->newconf;
	asd_iface_cb cb;
	int freq;

	if (status) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to select hw_mode.\n");

		cb = wasd_iface->reload_iface_cb;
		wasd_iface->reload_iface_cb = NULL;
		cb(wasd_iface, -1);

		return;
	}

	freq = asd_hw_get_freq(wasd, newconf->channel);
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Mode: %s  Channel: %d  Frequency: %d MHz",
		   asd_hw_mode_txt(newconf->hw_mode),
		   newconf->channel, freq);

	if (asd_set_freq(wasd, newconf->hw_mode, freq)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set channel %d (%d MHz) for kernel "
		       "driver\n", newconf->channel, freq);
	}

	change->beacon_changed++;

	reload_iface2(wasd_iface);
}


/**
 * asd_config_reload_iface_start - Start interface reload
 * @wasd_iface: Pointer to interface data.
 * @cb: The function to callback when done.
 * Returns:  0 if it starts successfully; cb will be called when done.
 *          -1 on failure; cb will not be called.
 */
static int asd_config_reload_iface_start(struct asd_iface *wasd_iface,
					     asd_iface_cb cb)
{
	struct asd_config_change *change = wasd_iface->change;
	struct asd_config *newconf = change->newconf;
	struct asd_config *oldconf = change->oldconf;
	struct asd_data *wasd = wasd_iface->bss[0];

	if (wasd_iface->reload_iface_cb) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,
			   "%s: Interface reload already in progress.",
			   wasd_iface->bss[0]->conf->iface);
		return -1;
	}

	wasd_iface->reload_iface_cb = cb;

	if (newconf->bridge_packets != oldconf->bridge_packets &&
	    wasd->iconf->bridge_packets != INTERNAL_BRIDGE_DO_NOT_CONTROL &&
	    asd_set_internal_bridge(wasd, wasd->iconf->bridge_packets))
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set bridge_packets for kernel driver\n");

	if (newconf->channel != oldconf->channel ||
	    newconf->hw_mode != oldconf->hw_mode ||
	    rate_array_diff(newconf->supported_rates,
			    oldconf->supported_rates) ||
	    rate_array_diff(newconf->basic_rates, oldconf->basic_rates)) {
		asd_free_stas(wasd);

		if (asd_get_hw_features(wasd_iface)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not read HW feature info from the kernel"
			       " driver.\n");
			wasd_iface->reload_iface_cb = NULL;
			return -1;
		}

		if (asd_select_hw_mode_start(wasd_iface,
						 reload_hw_mode_done)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to start select hw_mode.\n");
			wasd_iface->reload_iface_cb = NULL;
			return -1;
		}

		return 0;
	}

	circle_register_timeout(0, 0, reload_iface2_handler, wasd_iface, NULL);
	return 0;
}


static void asd_reconfig_bss(struct asd_data *wasd,
				 struct asd_bss_config *newbss,
				 struct asd_bss_config *oldbss,
				 struct asd_config *oldconf,
				 int beacon_changed)
{
	struct asd_config_change change;
	int encr_changed = 0;
	struct radius_client_data *old_radius;

	//radius_client_flush(wasd->radius, 0);

	if (asd_set_dtim_period(wasd, newbss->dtim_period))
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set DTIM period for kernel driver\n");

	if (newbss->ssid.ssid_len != oldbss->ssid.ssid_len ||
	    os_memcmp(newbss->ssid.ssid, oldbss->ssid.ssid,
		      newbss->ssid.ssid_len) != 0) {
		if (asd_set_ssid(wasd, (u8 *) newbss->ssid.ssid,
				     newbss->ssid.ssid_len))
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set SSID for kernel driver\n");
		beacon_changed++;
	}

	if (newbss->ignore_broadcast_ssid != oldbss->ignore_broadcast_ssid)
		beacon_changed++;

	if (asd_wep_key_cmp(&newbss->ssid.wep, &oldbss->ssid.wep)) {
		encr_changed++;
		beacon_changed++;
	}

	vlan_reconfig(wasd, oldconf, oldbss);

	if (beacon_changed) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Updating beacon frame information");
		ieee802_11_set_beacon(wasd);
	}

	change.wasd = wasd;
	change.oldconf = oldconf;
	change.newconf = wasd->iconf;
	change.oldbss = oldbss;
	change.newbss = newbss;
	change.mac_acl_changed = asd_acl_diff(newbss, oldbss);
	if (newbss->max_num_sta != oldbss->max_num_sta &&
	    newbss->max_num_sta < wasd->num_sta) {
		change.num_sta_remove = wasd->num_sta - newbss->max_num_sta;
	} else
		change.num_sta_remove = 0;
	ap_for_each_sta(wasd, asd_config_reload_sta, &change);

	old_radius = wasd->radius;
	wasd->radius = radius_client_reconfig(wasd->radius, wasd,
					      oldbss->radius, newbss->radius);
	wasd->radius_client_reconfigured = old_radius != wasd->radius ||
		asd_ip_diff(&newbss->own_ip_addr, &oldbss->own_ip_addr);

	ieee802_1x_reconfig(wasd, oldconf, oldbss);
	iapp_reconfig(wasd, oldconf, oldbss);

	asd_acl_reconfig(wasd, oldconf);
	accounting_reconfig(wasd, oldconf);
}


/**
 * config_reload2 - Part 2 of configuration reloading
 * @wasd_iface:
 */
static void config_reload2(struct asd_iface *wasd_iface, int status)
{
	struct asd_config_change *change = wasd_iface->change;
	struct asd_data *wasd = change->wasd;
	struct asd_config *newconf = change->newconf;
	struct asd_config *oldconf = change->oldconf;
	int beacon_changed = change->beacon_changed;
	struct asd_data **new_wasd = change->new_wasd;
	struct asd_data **old_wasd = change->old_wasd;
	int num_old_wasd = change->num_old_wasd;
	size_t i, j, max_bss, same_bssid;
	struct asd_bss_config *newbss, *oldbss;
	u8 *prev_addr;
	asd_iface_cb cb;

	os_free(change);
	wasd_iface->change = NULL;

	if (status) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to setup new interface config\n");

		cb = wasd_iface->config_reload_cb;
		wasd_iface->config_reload_cb = NULL;

		/* Invalid configuration - cleanup and terminate asd */
		wasd_iface->bss = old_wasd;
		wasd_iface->num_bss = num_old_wasd;
		wasd_iface->conf = wasd->iconf = oldconf;
		wasd->conf = &oldconf->bss[0];
		asd_config_free(newconf);
		os_free(new_wasd);

		cb(wasd_iface, -2);

		return;
	}

	/*
	 * If any BSSes have been removed, added, or had their BSSIDs changed,
	 * completely remove and reinitialize such BSSes and all the BSSes
	 * following them since their BSSID might have changed.
	 */
	max_bss = oldconf->num_bss;
	if (max_bss > newconf->num_bss)
		max_bss = newconf->num_bss;

	for (i = 0; i < max_bss; i++) {
		if (os_strcmp(oldconf->bss[i].iface, newconf->bss[i].iface) !=
		    0 || asd_mac_comp(oldconf->bss[i].bssid,
					  newconf->bss[i].bssid) != 0)
			break;
	}
	same_bssid = i;

	for (i = 0; i < oldconf->num_bss; i++) {
		oldbss = &oldconf->bss[i];
		newbss = NULL;
		for (j = 0; j < newconf->num_bss; j++) {
			if (os_strcmp(oldbss->iface, newconf->bss[j].iface) ==
			    0) {
				newbss = &newconf->bss[j];
				break;
			}
		}

		if (newbss && i < same_bssid) {
			wasd = wasd_iface->bss[j] = old_wasd[i];
			wasd->iconf = newconf;
			wasd->conf = newbss;
			asd_reconfig_bss(wasd, newbss, oldbss, oldconf,
					     beacon_changed);
		} else {
			wasd = old_wasd[i];
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Removing BSS (ifname %s)",
				   wasd->conf->iface);
			asd_free_stas(wasd);
			/* Send broadcast deauthentication for this BSS, but do
			 * not clear all STAs from the driver since other BSSes
			 * may have STA entries. The driver will remove all STA
			 * entries for this BSS anyway when the interface is
			 * being removed. */
#if 0
			asd_deauth_all_stas(wasd);
			asd_cleanup(wasd);
#endif

			os_free(wasd);
		}
	}


	prev_addr = wasd_iface->bss[0]->own_addr;
	wasd = wasd_iface->bss[0];
	for (j = 0; j < newconf->num_bss; j++) {
		if (wasd_iface->bss[j] != NULL) {
			prev_addr = wasd_iface->bss[j]->own_addr;
			continue;
		}

		newbss = &newconf->bss[j];

		asd_printf(ASD_DEFAULT,MSG_DEBUG, "Reconfiguration: adding new BSS "
			   "(ifname=%s)", newbss->iface);

#if 0
		wasd = wasd_iface->bss[j] =
			asd_alloc_bss_data(wasd_iface, newconf, newbss);
		if (wasd == NULL) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to initialize new BSS\n");
			/* FIX: This one is somewhat hard to recover
			 * from.. Would need to remove this BSS from
			 * conf and BSS list. */
			exit(1);
		}
#endif
		wasd->driver = wasd_iface->bss[0]->driver;
		wasd->iface = wasd_iface;
		wasd->iconf = newconf;
		wasd->conf = newbss;

		os_memcpy(wasd->own_addr, prev_addr, ETH_ALEN);
		if (asd_mac_comp_empty(wasd->conf->bssid) == 0)
			prev_addr = wasd->own_addr;

#if 0
		if (asd_setup_bss(wasd, j == 0)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to setup new BSS\n");
			/* FIX */
			exit(1);
		}
#endif

	}

	os_free(old_wasd);
	asd_config_free(oldconf);

	cb = wasd_iface->config_reload_cb;
	wasd_iface->config_reload_cb = NULL;

	cb(wasd_iface, 0);
}


/**
 * asd_config_reload_start - Start reconfiguration of an interface
 * @wasd_iface: Pointer to asd interface data
 * @cb: Function to be called back when done.
 *      The status indicates:
 *       0 = success, new configuration in use;
 *      -1 = failed to update configuraiton, old configuration in use;
 *      -2 = failed to update configuration and failed to recover; caller
 *           should cleanup and terminate asd
 * Returns:
 *  0 = reconfiguration started;
 * -1 = failed to update configuration, old configuration in use;
 * -2 = failed to update configuration and failed to recover; caller
 *      should cleanup and terminate asd
 */
int asd_config_reload_start(struct asd_iface *wasd_iface,
				asd_iface_cb cb)
{
	struct asd_config *newconf, *oldconf;
	struct asd_config_change *change;
	struct asd_data *wasd = NULL;
	struct asd_data **old_wasd, **new_wasd;
	int num_old_wasd;

	if (wasd_iface->config_reload_cb) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: Config reload already in progress.",
			   wasd_iface->bss[0]->conf->iface);
		return -1;
	}

	newconf = asd_config_read(wasd_iface->config_fname);
	if (newconf == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to read new configuration file - continuing "
		       "with old.\n");
		return -1;
	}

	if (os_strcmp(newconf->bss[0].iface, wasd_iface->conf->bss[0].iface) !=
	    0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Interface name changing is not allowed in "
		       "configuration reloading (%s -> %s).\n",
		       wasd_iface->conf->bss[0].iface,  newconf->bss[0].iface);
		asd_config_free(newconf);
		return -1;
	}

	new_wasd = os_zalloc(newconf->num_bss *
			     sizeof(struct asd_data *));
	if (new_wasd == NULL) {
		asd_config_free(newconf);
		return -1;
	}
	old_wasd = wasd_iface->bss;
	num_old_wasd = wasd_iface->num_bss;

	wasd_iface->bss = new_wasd;
	wasd_iface->num_bss = newconf->num_bss;
	/*
	 * First BSS remains the same since interface name changing was
	 * prohibited above. Now, this is only used in
	 * asd_config_reload_iface() and following loop will anyway set
	 * this again.
	 */
	wasd = wasd_iface->bss[0] = old_wasd[0];

	oldconf = wasd_iface->conf;
	wasd->iconf = wasd_iface->conf = newconf;
	wasd->conf = &newconf->bss[0];

	change = os_zalloc(sizeof(struct asd_config_change));
	if (change == NULL) {
		asd_config_free(newconf);
		return -1;
	}

	change->wasd = wasd;
	change->newconf = newconf;
	change->oldconf = oldconf;
	change->beacon_changed = 0;
	change->wasd_iface = wasd_iface;
	change->new_wasd = new_wasd;
	change->old_wasd = old_wasd;
	change->num_old_wasd = num_old_wasd;

	wasd_iface->config_reload_cb = cb;
	wasd_iface->change = change;
	if (asd_config_reload_iface_start(wasd_iface, config_reload2)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to start setup of new interface config\n");

		wasd_iface->config_reload_cb = NULL;
		os_free(change);
		wasd_iface->change = NULL;

		/* Invalid configuration - cleanup and terminate asd */
		wasd_iface->bss = old_wasd;
		wasd_iface->num_bss = num_old_wasd;
		wasd_iface->conf = wasd->iconf = oldconf;
		wasd->conf = &oldconf->bss[0];
		asd_config_free(newconf);
		os_free(new_wasd);
		return -2;
	}

	return 0;
}
