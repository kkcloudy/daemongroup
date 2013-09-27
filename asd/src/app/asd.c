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
* Asd.c
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
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <openssl/err.h>
#include <openssl/evp.h>


#include "circle.h"
#include "asd.h"
#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDAccounting.h"
#include "ASDEapolSM.h"
#include "ASDIappOp.h"
#include "ap.h"
#include "ASD80211AuthOp.h"
#include "ASDApList.h"
#include "ASDStaInfo.h"
#include "ASDCallback.h"
#include "ASDRadius/radius_client.h"
#include "ASDRadius/radius_server.h"
#include "ASDWPAOp.h"
#include "ASDPreauth.h"
#include "ASDWme.h"
#include "vlan_init.h"
#include "ASDCtrlIface.h"
#include "tls.h"
#include "ASDEAPMethod/eap_sim_db.h"
#include "ASDEAPMethod/eap.h"

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusPath.h"
#include "ASDDbus.h"
#include "asd_bak.h"
#include "Inter_AC_Roaming.h"
#include "ASDNetlinkArpOp.h"
#include "ASDPkcs12App.h"

#include "../app/include/none_cert.h"
#include "../app/include/x509_cert.h"
#include "ASDCallback_asd.h"
#include "time.h"//qiuchen add it
#include "ASDStaManage.h"
WID_WLAN	*ASD_WLAN[WLAN_NUM];
WID_WTP		**ASD_WTP;
WID_WTP_RADIO	**ASD_RADIO;
WID_BSS		**ASD_BSS;
security_profile *ASD_SECURITY[WLAN_NUM];
ASD_WTP_ST **ASD_WTP_AP;  //xm add 08/12/01
ASD_WTP_ST_HISTORY **ASD_WTP_AP_HISTORY;  //mahz add 2011.4.30
struct sta_static_info *STA_STATIC_TABLE[STA_HASH_SIZE];
struct sta_nas_info *ASD_HOTSPOT[HOTSPOT_ID+1];
struct sta_info *ASD_STA_HASH[STA_HASH_SIZE*STA_HASH_SIZE] = {NULL};//add for global sta hash 256*256
struct sta_acct_info *ASD_ACCT_HASH[STA_HASH_SIZE] = {NULL};
struct asd_ip_secret *ASD_SECRET_HASH[STA_HASH_SIZE] = {NULL};
unsigned int	sta_static_num;
int WTP_NUM = 4096;	
int WTP_NUM_AUTELAN = 0;	
int WTP_NUM_OEM  = 0;	
LICENSE_TYPE **g_wtp_count = NULL;	/*xiaodawei add, 20101029*/
LICENSE_TYPE **g_wtp_binding_count = NULL;	/*xiaodawei add, 20101029*/
int glicensecount = 0;
asd_sta_flash_disconn *ASD_FDStas;
int G_RADIO_NUM;
int BSS_NUM;
unsigned char gasdtrapflag = 1;
pthread_mutex_t asd_g_sta_mutex;
pthread_mutex_t asd_g_wtp_mutex;
pthread_mutex_t asd_flash_disconn_mutex;
pthread_mutex_t asd_g_hotspot_mutex;//qiuchen add it 2012.10.26
pthread_mutex_t asd_g_wlan_mutex;
pthread_mutex_t asd_g_bss_mutex;
int vrrid = 0;
int local = 1;
int slotid = 0;
char *ASD_ARP_GROUP[WLAN_NUM];
int nl_sock = 0;
struct acl_config ac_acl_conf;

/*static int asd_radius_get_eap_user(void *ctx, const u8 *identity,
				       size_t identity_len, int phase2,
				       struct eap_user *user);*/
/*
struct wasd_interfaces {
	size_t count;
	struct asd_iface **iface;	
	struct asd_iface *config;//zhang lei add
	void *if_priv;//zhang lei add
};
*/
unsigned char rfc1042_header[6] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };


extern int wpa_debug_level;
extern int wpa_debug_show_keys;
extern int wpa_debug_timestamp;

extern unsigned char gASDLOGDEBUG;
/* For new format of syslog 2013-07-29 */
extern unsigned long gASD_AC_MANAGEMENT_IP;
unsigned int AP_STATISTICS_LOG_INTERVAL = 28800;
unsigned int ROAM_STATISTICS_LOG_INTERVAL = 28800+30;

static void asd_logger_cb(void *ctx, const u8 *addr, unsigned int module,
			      int level, const char *txt, size_t len)
{
	struct asd_data *wasd = ctx;
	char *format, *module_str;
	int maxlen;
	int conf_syslog_level, conf_stdout_level;
	unsigned int conf_syslog, conf_stdout;

	maxlen = len + 100+64;
	format = os_zalloc(maxlen);
	if (!format)
		return;
	char *s = format;
	sprintf(format,"[%d-%d]",slotid,vrrid);
	format = format + strlen(format);
	if (wasd && wasd->conf) {
		conf_syslog_level = wasd->conf->logger_syslog_level;
		conf_stdout_level = wasd->conf->logger_stdout_level;
		conf_syslog = wasd->conf->logger_syslog;
		conf_stdout = wasd->conf->logger_stdout;
	} else {
		conf_syslog_level = conf_stdout_level = 0;
		conf_syslog = conf_stdout = (unsigned int) -1;
	}

	switch (module) {
	case asd_MODULE_IEEE80211:
		module_str = "IEEE 802.11";
		break;
	case asd_MODULE_IEEE8021X:
		module_str = "IEEE 802.1X";
		break;
	case asd_MODULE_RADIUS:
		module_str = "RADIUS";
		break;
	case asd_MODULE_WPA:
		module_str = "WPA";
		break;
	case asd_MODULE_DRIVER:
		module_str = "DRIVER";
		break;
	case asd_MODULE_IAPP:
		module_str = "IAPP";
		break;
	case asd_MODULE_MLME:
		module_str = "MLME";
		break;
	default:
		module_str = NULL;
		break;
	}

	if (wasd && wasd->conf && addr)
		os_snprintf(format, maxlen, "%s: STA " MACSTR "%s%s: %s",
			    wasd->conf->iface, MAC2STR(addr),
			    module_str ? " " : "", module_str, txt);
	else if (wasd && wasd->conf)
		os_snprintf(format, maxlen, "%s:%s%s %s",
			    wasd->conf->iface, module_str ? " " : "",
			    module_str, txt);
	else if (addr)
		os_snprintf(format, maxlen, "STA " MACSTR "%s%s: %s",
			    MAC2STR(addr), module_str ? " " : "",
			    module_str, txt);
	else
		os_snprintf(format, maxlen, "%s%s%s",
			    module_str, module_str ? ": " : "", txt);

	if (gasdPRINT)
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",s);

		switch (level) {
		case asd_LEVEL_DEBUG_VERBOSE:
		case asd_LEVEL_DEBUG:
		asd_syslog_debug(ASD_ALL,s);
			break;
		case asd_LEVEL_INFO:
		asd_syslog_info(s);
			break;
		case asd_LEVEL_NOTICE:
		asd_syslog_notice(s);
			break;
		case asd_LEVEL_WARNING:
		asd_syslog_warning(s);
			break;
		default:
		asd_syslog_info(s);
			break;
		}
/*
	if ((conf_stdout & module) && level >= conf_stdout_level) {
		wpa_debug_print_timestamp();
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n", format);
	}
*/
/*
	if ((conf_syslog & module) && level >= conf_syslog_level) {
		int priority;
		switch (level) {
		case asd_LEVEL_DEBUG_VERBOSE:
		case asd_LEVEL_DEBUG:
			priority = LOG_DEBUG;
			break;
		case asd_LEVEL_INFO:
			priority = LOG_INFO;
			break;
		case asd_LEVEL_NOTICE:
			priority = LOG_NOTICE;
			break;
		case asd_LEVEL_WARNING:
			priority = LOG_WARNING;
			break;
		default:
			priority = LOG_INFO;
			break;
		}
		syslog(priority, "%s", format);
	}
*/	

	os_free(s);
}


//static void asd_deauth_all_stas(struct asd_data *wasd)
//{
#if 0
	u8 addr[ETH_ALEN];

	os_memset(addr, 0xff, ETH_ALEN);
	asd_sta_deauth(wasd, addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
#else
	/* New Prism2.5/3 STA firmware versions seem to have issues with this
	 * broadcast deauth frame. This gets the firmware in odd state where
	 * nothing works correctly, so let's skip sending this for a while
	 * until the issue has been resolved. */
#endif
//}


/**
 * asd_prune_associations - Remove extraneous associations
 * @wasd: Pointer to BSS data for the most recent association
 * @sta: Pointer to the associated STA data
 *
 * This function looks through all radios and BSS's for previous
 * (stale) associations of STA. If any are found they are removed.
 */

/*
static void asd_prune_associations(struct asd_data *wasd,
				       struct sta_info *sta)
{
	struct sta_info *osta;
	struct asd_data *owasd;
	size_t i, j;
	struct wasd_interfaces *interfaces = circle_get_user_data();

	for (i = 0; i < interfaces->count; i++) {
		for (j = 0; j < interfaces->iface[i]->num_bss; j++) {
			owasd = interfaces->iface[i]->bss[j];
			if (owasd == wasd)
				continue;
			osta = ap_get_sta(owasd, sta->addr);
			if (!osta)
				continue;

			ap_sta_disassociate(owasd, osta,
					    WLAN_REASON_UNSPECIFIED);
		}
	}
}
*/

/**
 * asd_new_assoc_sta - Notify that a new station associated with the AP
 * @wasd: Pointer to BSS data
 * @sta: Pointer to the associated STA data
 * @reassoc: 1 to indicate this was a re-association; 0 = first association
 *
 * This function will be called whenever a station associates with the AP. It
 * can be called for ieee802_11.c for drivers that export MLME to asd and
 * from driver_*.c for drivers that take care of management frames (IEEE 802.11
 * authentication and association) internally.
 */
void asd_new_assoc_sta(struct asd_data *wasd, struct sta_info *sta,
			   int reassoc)
{
	if (wasd->tkip_countermeasures) {
		asd_sta_deauth(wasd, sta->addr,
				   WLAN_REASON_MICHAEL_MIC_FAILURE);
		return;
	}
//	asd_prune_associations(wasd, sta);

	/* IEEE 802.11F (IAPP) */
	if (wasd->conf->ieee802_11f)
		iapp_new_station(wasd->iapp, sta);

	/* Start accounting here, if IEEE 802.1X and WPA are not used.
	 * IEEE 802.1X/WPA code will start accounting after the station has
	 * been authorized. */

	
	if (!wasd->conf->ieee802_1x && !wasd->conf->wpa){
		accounting_sta_start(wasd, sta);
	}
//	asd_wme_sta_config(wasd, sta);

	/* Start IEEE 802.1X authentication process for new stations */
	ieee802_1x_new_station(wasd, sta);

	if (reassoc) {
		if (sta->auth_alg != WLAN_AUTH_FT)
			wpa_auth_sm_event(sta->wpa_sm, WPA_REAUTH);
	} else
		wpa_auth_sta_associated(wasd->wpa_auth, sta->wpa_sm);
	
}


#ifdef EAP_SERVER
/*static int asd_sim_db_cb_sta(struct asd_data *wasd,
				 struct sta_info *sta, void *ctx)
{
	if (eapol_auth_eap_pending_cb(sta->eapol_sm, ctx) == 0)
		return 1;
	return 0;
}



static void asd_sim_db_cb(void *ctx, void *session_ctx)
{
	struct asd_data *wasd = ctx;
	if (ap_for_each_sta(wasd, asd_sim_db_cb_sta, session_ctx) == 0)
		radius_server_eap_pending_cb(wasd->radius_srv, session_ctx);
}*/
#endif /* EAP_SERVER */


static void handle_term(int sig, void *circle_ctx, void *signal_ctx)
{
	asd_printf(ASD_DEFAULT,MSG_CRIT,"Signal %d received - terminating\n", sig);
	circle_terminate();
}


static void asd_wpa_auth_conf(struct asd_bss_config *conf,
				  struct wpa_auth_config *wconf)
{
	wconf->wpa = conf->wpa;
	wconf->wpa_key_mgmt = conf->wpa_key_mgmt;
	wconf->wpa_pairwise = conf->wpa_pairwise;
	wconf->wpa_group = conf->wpa_group;
	wconf->wpa_group_rekey = conf->wpa_group_rekey;
	wconf->wpa_keyupdate_timeout = conf->wpa_keyupdate_timeout;
	wconf->wpa_strict_rekey = conf->wpa_strict_rekey;
	wconf->wpa_gmk_rekey = conf->wpa_gmk_rekey;
	wconf->rsn_pairwise = conf->rsn_pairwise;
	wconf->rsn_preauth = conf->rsn_preauth;
	wconf->eapol_version = conf->eapol_version;
	wconf->peerkey = conf->peerkey;
	wconf->wme_enabled = conf->wme_enabled;
	wconf->wpa_once_group_rekey_time = conf->wpa_once_group_rekey_time;		//mahz add 2011.1.3
#ifdef ASD_IEEE80211W
	wconf->ieee80211w = conf->ieee80211w;
#endif /* ASD_IEEE80211W */
#ifdef ASD_IEEE80211R
	wconf->ssid_len = conf->ssid.ssid_len;
	if (wconf->ssid_len > SSID_LEN)
		wconf->ssid_len = SSID_LEN;
	os_memcpy(wconf->ssid, conf->ssid.ssid, wconf->ssid_len);
	os_memcpy(wconf->mobility_domain, conf->mobility_domain,
		  MOBILITY_DOMAIN_ID_LEN);
	if (conf->nas_identifier &&
	    os_strlen(conf->nas_identifier) <= FT_R0KH_ID_MAX_LEN) {
		wconf->r0_key_holder_len = os_strlen(conf->nas_identifier);
		os_memcpy(wconf->r0_key_holder, conf->nas_identifier,
			  wconf->r0_key_holder_len);
	}
	os_memcpy(wconf->r1_key_holder, conf->r1_key_holder, FT_R1KH_ID_LEN);
	wconf->r0_key_lifetime = conf->r0_key_lifetime;
	wconf->reassociation_deadline = conf->reassociation_deadline;
	wconf->r0kh_list = conf->r0kh_list;
	wconf->r1kh_list = conf->r1kh_list;
	wconf->pmk_r1_push = conf->pmk_r1_push;
#endif /* ASD_IEEE80211R */
}


static void handle_reload(int sig, void *circle_ctx, void *signal_ctx)
{
	struct wasd_interfaces *wasds = (struct wasd_interfaces*) circle.user_data;
	struct asd_config *newconf;
	size_t i;
	struct wpa_auth_config wpa_auth_conf;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"Signal %d received - reloading configuration\n", sig);

	for (i = 0; i < wasds->count; i++) {
		struct asd_data *wasd = wasds->iface[i]->bss[0];
		newconf = asd_config_read(wasds->iface[i]->config_fname);
		if (newconf == NULL) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to read new configuration file - "
			       "continuing with old.\n");
			continue;
		}
		/* TODO: update dynamic data based on changed configuration
		 * items (e.g., open/close sockets, remove stations added to
		 * deny list, etc.) */
		//radius_client_flush(wasd->radius, 0);
		asd_config_free(wasd->iconf);

		asd_wpa_auth_conf(&newconf->bss[0], &wpa_auth_conf);
		wpa_reconfig(wasd->wpa_auth, &wpa_auth_conf);

		wasd->iconf = newconf;
		wasd->conf = &newconf->bss[0];
		wasds->iface[i]->conf = newconf;

		if (asd_setup_wpa_psk(wasd->conf)) {
			asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to re-configure WPA PSK "
				   "after reloading configuration");
		}
	}
}


#ifdef asd_DUMP_STATE
static void asd_dump_state(struct asd_data *wasd)
{
	FILE *f;
	time_t now;
	struct sta_info *sta;
	int i;
	char *buf;

	if (!wasd->conf->dump_log_name) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Dump file not defined - ignoring dump request\n");
		return;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"Dumping asd state to '%s'\n", wasd->conf->dump_log_name);
	f = fopen(wasd->conf->dump_log_name, "w");
	if (f == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not open dump file '%s' for writing.\n",
		       wasd->conf->dump_log_name);
		return;
	}

	time(&now);
	fprintf(f, "asd state dump - %s", ctime(&now));
	fprintf(f, "num_sta=%d num_sta_non_erp=%d "
		"num_sta_no_short_slot_time=%d\n"
		"num_sta_no_short_preamble=%d\n",
		wasd->num_sta, wasd->iface->num_sta_non_erp,
		wasd->iface->num_sta_no_short_slot_time,
		wasd->iface->num_sta_no_short_preamble);

	for (sta = wasd->sta_list; sta != NULL; sta = sta->next) {
		fprintf(f, "\nSTA=" MACSTR "\n", MAC2STR(sta->addr));

		fprintf(f,
			"  AID=%d flags=0x%x %s%s%s%s%s%s%s%s%s%s\n"
			"  capability=0x%x listen_interval=%d\n",
			sta->aid,
			sta->flags,
			(sta->flags & WLAN_STA_AUTH ? "[AUTH]" : ""),
			(sta->flags & WLAN_STA_ASSOC ? "[ASSOC]" : ""),
			(sta->flags & WLAN_STA_PS ? "[PS]" : ""),
			(sta->flags & WLAN_STA_TIM ? "[TIM]" : ""),
			(sta->flags & WLAN_STA_PERM ? "[PERM]" : ""),
			(sta->flags & WLAN_STA_AUTHORIZED ? "[AUTHORIZED]" :
			 ""),
			(sta->flags & WLAN_STA_PENDING_POLL ? "[PENDING_POLL" :
			 ""),
			(sta->flags & WLAN_STA_SHORT_PREAMBLE ?
			 "[SHORT_PREAMBLE]" : ""),
			(sta->flags & WLAN_STA_PREAUTH ? "[PREAUTH]" : ""),
			(sta->flags & WLAN_STA_NONERP ? "[NonERP]" : ""),
			sta->capability,
			sta->listen_interval);

		fprintf(f, "  supported_rates=");
		for (i = 0; i < sta->supported_rates_len; i++)
			fprintf(f, "%02x ", sta->supported_rates[i]);
		fprintf(f, "\n");

		fprintf(f,
			"  timeout_next=%s\n",
			(sta->timeout_next == STA_NULLFUNC ? "NULLFUNC POLL" :
			 (sta->timeout_next == STA_DISASSOC ? "DISASSOC" :
			  "DEAUTH")));

		ieee802_1x_dump_state(f, "  ", sta);
	}

	buf = os_zalloc(4096);
	if (buf) {
		int count = radius_client_get_mib(wasd->radius, buf, 4096);
		if (count < 0)
			count = 0;
		else if (count > 4095)
			count = 4095;
		buf[count] = '\0';
		fprintf(f, "%s", buf);

		count = radius_server_get_mib(wasd->radius_srv, buf, 4096);
		if (count < 0)
			count = 0;
		else if (count > 4095)
			count = 4095;
		buf[count] = '\0';
		fprintf(f, "%s", buf);
		os_free(buf);
	}
	fclose(f);
}
#endif /* asd_DUMP_STATE */


static void handle_dump_state(int sig, void *circle_ctx, void *signal_ctx)
{
#ifdef asd_DUMP_STATE
	struct wasd_interfaces *wasds = (struct wasd_interfaces *) circle_ctx;
	size_t i, j;

	for (i = 0; i < wasds->count; i++) {
		struct asd_iface *wasd_iface = wasds->iface[i];
		for (j = 0; j < wasd_iface->num_bss; j++)
			asd_dump_state(wasd_iface->bss[j]);
	}
#endif /* asd_DUMP_STATE */
}
/*
static void asd_broadcast_key_clear_iface(struct asd_data *wasd,
					      char *ifname)
{
	int i;

	for (i = 0; i < NUM_WEP_KEYS; i++) {
		if (asd_set_encryption(ifname, wasd, "none", NULL, i, NULL,
					   0, i == 0 ? 1 : 0)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to clear default encryption keys "
			       "(ifname=%s keyidx=%d)\n", ifname, i);
		}
	}
}


static int asd_broadcast_wep_clear(struct asd_data *wasd)
{
	asd_broadcast_key_clear_iface(wasd, wasd->conf->iface);
	return 0;
}


static int asd_broadcast_wep_set(struct asd_data *wasd)
{
	int errors = 0, idx;
	struct asd_ssid *ssid = &wasd->conf->ssid;

	idx = ssid->wep.idx;
	if (ssid->wep.default_len &&
	    asd_set_encryption(wasd->conf->iface,
				   wasd, "WEP", NULL, idx,
			 	   ssid->wep.key[idx],
			 	   ssid->wep.len[idx],
				   idx == ssid->wep.idx)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set WEP encryption.\n");
		errors++;
	}

	if (ssid->dyn_vlan_keys) {
		size_t i;
		for (i = 0; i <= ssid->max_dyn_vlan_keys; i++) {
			const char *ifname;
			struct asd_wep_keys *key = ssid->dyn_vlan_keys[i];
			if (key == NULL)
				continue;
			ifname = asd_get_vlan_id_ifname(wasd->conf->vlan,
							    i);
			if (ifname == NULL)
				continue;

			idx = key->idx;
			if (asd_set_encryption(ifname, wasd, "WEP", NULL,
						   idx, key->key[idx],
						   key->len[idx],
						   idx == key->idx)) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set dynamic VLAN WEP "
				       "encryption.\n");
				errors++;
			}
		}
	}

	return errors;
}
*/
/**
 * asd_cleanup - Per-BSS cleanup (deinitialization)
 * @wasd: Pointer to BSS data
 *
 * This function is used to free all per-BSS data structures and resources.
 * This gets called in a loop for each BSS between calls to
 * asd_cleanup_iface_pre() and asd_cleanup_iface() when an interface
 * is deinitialized. Most of the modules that are initialized in
 * asd_setup_bss() are deinitialized here.
 */
void asd_cleanup(struct asd_data *wasd)
{
//	asd_ctrl_iface_deinit(wasd);

	os_free(wasd->default_wep_key);
	wasd->default_wep_key = NULL;
	iapp_deinit(wasd->iapp);
	wasd->iapp = NULL;
	accounting_deinit(wasd);
	rsn_preauth_iface_deinit(wasd);
	if (wasd->wpa_auth) {
		wpa_deinit(wasd->wpa_auth);
		wasd->wpa_auth = NULL;

		if (asd_set_privacy(wasd, 0)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not disable "
				   "PrivacyInvoked for interface %s",
				   wasd->conf->iface);
		}

		if (asd_set_generic_elem(wasd, (u8 *) "", 0)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "Could not remove generic "
				   "information element from interface %s",
				   wasd->conf->iface);
		}
	}
	ieee802_1x_deinit(wasd);
	vlan_deinit(wasd);
	asd_acl_deinit(wasd);
	radius_client_deinit(wasd->radius);
	wasd->radius = NULL;
	radius_server_deinit(wasd->radius_srv);
	wasd->radius_srv = NULL;

#ifdef ASD_IEEE80211R
	l2_packet_deinit(wasd->l2);
#endif /* ASD_IEEE80211R */

	asd_wireless_event_deinit(wasd);

#ifdef EAP_TLS_FUNCS
	if (wasd->ssl_ctx) {
		tls_deinit(wasd->ssl_ctx);
		wasd->ssl_ctx = NULL;
	}
#endif /* EAP_TLS_FUNCS */

#ifdef EAP_SERVER
	if (wasd->eap_sim_db_priv) {
		eap_sim_db_deinit(wasd->eap_sim_db_priv);
		wasd->eap_sim_db_priv = NULL;
	}
#endif /* EAP_SERVER */

	if (wasd->interface_added &&
	    asd_bss_remove(wasd, wasd->conf->iface)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to remove BSS interface %s\n",
		       wasd->conf->iface);
	}
}


/**
 * asd_cleanup_iface_pre - Preliminary per-interface cleanup
 * @iface: Pointer to interface data
 *
 * This function is called before per-BSS data structures are deinitialized
 * with asd_cleanup().
 */
static void asd_cleanup_iface_pre(struct asd_iface *iface)
{
}


/**
 * asd_cleanup_iface - Complete per-interface cleanup
 * @iface: Pointer to interface data
 *
 * This function is called after per-BSS data structures are deinitialized
 * with asd_cleanup().
 */
static void asd_cleanup_iface(struct asd_iface *iface)
{
	asd_free_hw_features(iface->hw_features, iface->num_hw_features);
	iface->hw_features = NULL;
	os_free(iface->current_rates);
	iface->current_rates = NULL;
	ap_list_deinit(iface);
	asd_config_free(iface->conf);
	iface->conf = NULL;

	os_free(iface->config_fname);
	os_free(iface->bss);
	os_free(iface);
}

/*
static int asd_setup_encryption(char *iface, struct asd_data *wasd)
{
	int i;

	asd_broadcast_wep_set(wasd);

	if (wasd->conf->ssid.wep.default_len)
		return 0;

	for (i = 0; i < 4; i++) {
		if (wasd->conf->ssid.wep.key[i] &&
		    asd_set_encryption(iface, wasd, "WEP", NULL,
					   i, wasd->conf->ssid.wep.key[i],
					   wasd->conf->ssid.wep.len[i],
					   i == wasd->conf->ssid.wep.idx)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set WEP encryption.\n");
			return -1;
		}
		if (wasd->conf->ssid.wep.key[i] &&
		    i == wasd->conf->ssid.wep.idx)
			asd_set_privacy(wasd, 1);
	}

	return 0;
}


static int asd_flush_old_stations(struct asd_data *wasd)
{
	int ret = 0;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Flushing old station entries");
	if (asd_flush(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not connect to kernel driver.\n");
		ret = -1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "Deauthenticate all stations");
	asd_deauth_all_stas(wasd);

	return ret;
}
*/

static void asd_wpa_auth_logger(void *ctx, const u8 *addr,
				    logger_level level, const char *txt)
{
	struct asd_data *wasd = ctx;
	int hlevel;

	switch (level) {
	case LOGGER_WARNING:
		hlevel = asd_LEVEL_WARNING;
		break;
	case LOGGER_INFO:
		hlevel = asd_LEVEL_INFO;
		break;
	case LOGGER_DEBUG:
	default:
		hlevel = asd_LEVEL_DEBUG;
		break;
	}

	asd_logger(wasd, addr, asd_MODULE_WPA, hlevel, "%s", txt);
}


static void asd_wpa_auth_disconnect(void *ctx, const u8 *addr,
					u16 reason)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta;
	//qiuchen
	unsigned char SID = wasd->SecurityID;
	unsigned int securitytype = 0;
	unsigned char WTPMAC[MAC_LEN] = {0};
	if(ASD_SECURITY[SID])
		securitytype = ASD_SECURITY[SID]->securityType;
	if(ASD_WTP_AP[wasd->Radio_G_ID/4])
		memcpy(WTPMAC,ASD_WTP_AP[wasd->Radio_G_ID/4]->WTPMAC,MAC_LEN);
	//end
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: WPA authenticator requests disconnect: "
		   "STA " MACSTR " reason %d",
		   __func__, MAC2STR(addr), reason);

	sta = ap_get_sta(wasd, addr);
	asd_sta_deauth(wasd, addr, reason);
	if (sta == NULL)
		return;
	if(gASDLOGDEBUG & BIT(1))
		syslog(LOG_INFO|LOG_LOCAL7,"[%d-%d]AUTHFAILED:UserMAC:" MACSTR " APMAC:" MACSTR " BSSIndex:%d,SecurityType:%d,ErrorCode:%d.\n",slotid,vrrid,
			MAC2STR(addr),MAC2STR(WTPMAC),wasd->BSSIndex,securitytype,PSK_FAILED);//qiuchen 2013.01.14
	if(1 == check_sta_authorized(wasd,sta))
		wasd->authorized_sta_num--;
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC | WLAN_STA_AUTHORIZED);
	circle_cancel_timeout(ap_handle_timer, wasd, sta);
	//circle_register_timeout(0, 0, ap_handle_timer, wasd, sta);
	sta->timeout_next = STA_REMOVE;
}


static void asd_wpa_auth_mic_failure_report(void *ctx, const u8 *addr)
{
	struct asd_data *wasd = ctx;
	ieee80211_michael_mic_failure(wasd, addr, 0);
}


static void asd_wpa_auth_set_eapol(void *ctx, const u8 *addr,
				       wpa_eapol_variable var, int value)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = ap_get_sta(wasd, addr);
	if (sta == NULL)
		return;
	switch (var) {
	case WPA_EAPOL_portEnabled:
		ieee802_1x_notify_port_enabled(sta->eapol_sm, value);
		break;
	case WPA_EAPOL_portValid:
		ieee802_1x_notify_port_valid(sta->eapol_sm, value);
		break;
	case WPA_EAPOL_authorized:
		ieee802_1x_set_sta_authorized(wasd, sta, value);
		break;
	case WPA_EAPOL_portControl_Auto:
		if (sta->eapol_sm)
			sta->eapol_sm->portControl = Auto;
		break;
	case WPA_EAPOL_keyRun:
		if (sta->eapol_sm)
			sta->eapol_sm->keyRun = value ? TRUE : FALSE;
		break;
	case WPA_EAPOL_keyAvailable:
		if (sta->eapol_sm)
			sta->eapol_sm->eap_if->eapKeyAvailable =
				value ? TRUE : FALSE;
		break;
	case WPA_EAPOL_keyDone:
		if (sta->eapol_sm)
			sta->eapol_sm->keyDone = value ? TRUE : FALSE;
		break;
	case WPA_EAPOL_inc_EapolFramesTx:
		if (sta->eapol_sm)
			sta->eapol_sm->dot1xAuthEapolFramesTx++;
		break;
	}
}


static int asd_wpa_auth_get_eapol(void *ctx, const u8 *addr,
				      wpa_eapol_variable var)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta = ap_get_sta(wasd, addr);
	if (sta == NULL || sta->eapol_sm == NULL)
		return -1;
	switch (var) {
	case WPA_EAPOL_keyRun:
		return sta->eapol_sm->keyRun;
	case WPA_EAPOL_keyAvailable:
		return sta->eapol_sm->eap_if->eapKeyAvailable;
	default:
		return -1;
	}
}


static const u8 * asd_wpa_auth_get_psk(void *ctx, const u8 *addr,
					   const u8 *prev_psk)
{
	struct asd_data *wasd = ctx;
	return asd_get_psk(wasd->conf, addr, prev_psk);
}


static int asd_wpa_auth_get_msk(void *ctx, const u8 *addr, u8 *msk,
				    size_t *len)
{
	struct asd_data *wasd = ctx;
	const u8 *key;
	size_t keylen;
	struct sta_info *sta;

	sta = ap_get_sta(wasd, addr);
	if (sta == NULL)
		return -1;

	key = ieee802_1x_get_key(sta->eapol_sm, &keylen);
	if (key == NULL)
		return -1;

	if (keylen > *len)
		keylen = *len;
	os_memcpy(msk, key, keylen);
	*len = keylen;

	return 0;
}


static int asd_wpa_auth_set_key(void *ctx, int vlan_id, const char *alg,
				    const u8 *addr, int idx, u8 *key,
				    size_t key_len)
{
	struct asd_data *wasd = ctx;
	const char *ifname = wasd->conf->iface;

	if (vlan_id > 0) {
		ifname = asd_get_vlan_id_ifname(wasd->conf->vlan, vlan_id);
		if (ifname == NULL)
			return -1;
	}

	return asd_set_encryption(ifname, wasd, alg, addr, idx,
				      key, key_len, 1);
}


static int asd_wpa_auth_get_seqnum(void *ctx, const u8 *addr, int idx,
				       u8 *seq)
{
	struct asd_data *wasd = ctx;
	return asd_get_seqnum(wasd->conf->iface, wasd, addr, idx, seq);
}


static int asd_wpa_auth_get_seqnum_igtk(void *ctx, const u8 *addr, int idx,
					    u8 *seq)
{
	struct asd_data *wasd = ctx;
	return asd_get_seqnum_igtk(wasd->conf->iface, wasd, addr, idx,
				       seq);
}


static int asd_wpa_auth_send_eapol(void *ctx, const u8 *addr,
				       const u8 *data, size_t data_len,
				       int encrypt)
{
	struct asd_data *wasd = ctx;
	return asd_send_eapol(wasd, addr, data, data_len, encrypt);
}


static int asd_wpa_auth_for_each_sta(
	void *ctx, int (*cb)(struct wpa_state_machine *sm, void *ctx),
	void *cb_ctx)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta;

	for (sta = wasd->sta_list; sta; sta = sta->next) {
		if (sta->wpa_sm && cb(sta->wpa_sm, cb_ctx))
			return 1;
	}
	return 0;
}


static int asd_wpa_auth_send_ether(void *ctx, const u8 *dst, u16 proto,
				       const u8 *data, size_t data_len)
{
	struct asd_data *wasd = ctx;

	if (wasd->driver && wasd->driver->send_ether)
		return wasd->driver->send_ether(wasd->drv_priv, dst,
						wasd->own_addr, proto,
						data, data_len);
	if (wasd->l2 == NULL)
		return -1;
	return 0;//l2_packet_send(wasd->l2, dst, proto, data, data_len);
}


#ifdef ASD_IEEE80211R

static int asd_wpa_auth_send_ft_action(void *ctx, const u8 *dst,
					   const u8 *data, size_t data_len)
{
	struct asd_data *wasd = ctx;
	int res;
	struct ieee80211_mgmt *m;
	size_t mlen;
	struct sta_info *sta;

	sta = ap_get_sta(wasd, dst);
	if (sta == NULL || sta->wpa_sm == NULL)
		return -1;

	m = os_zalloc(sizeof(*m) + data_len);
	if (m == NULL)
		return -1;
	mlen = ((u8 *) &m->u - (u8 *) m) + data_len;
	m->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					WLAN_FC_STYPE_ACTION);
	os_memcpy(m->da, dst, ETH_ALEN);
	os_memcpy(m->sa, wasd->own_addr, ETH_ALEN);
	os_memcpy(m->bssid, wasd->own_addr, ETH_ALEN);
	os_memcpy(&m->u, data, data_len);

	DataMsg msg;
	int msglen;
	msglen = Assmble_WSM_msg(&msg, wasd, (u8 *) m, mlen, IEEE802_11_MGMT);
	res = asd_send_mgmt_frame(wasd, &msg, msglen, 0);
	os_free(m);
	return res;
}


static struct wpa_state_machine *
asd_wpa_auth_add_sta(void *ctx, const u8 *sta_addr)
{
	struct asd_data *wasd = ctx;
	struct sta_info *sta;

	sta = ap_sta_add(wasd, sta_addr, 1);
	if (sta == NULL)
		return NULL;
	if (sta->wpa_sm)
		return sta->wpa_sm;

	sta->wpa_sm = wpa_auth_sta_init(wasd->wpa_auth, sta->addr);
	if (sta->wpa_sm == NULL) {
		//UpdateStaInfoToWSM(wasd, sta->addr, WID_DEL);	
		//ap_free_sta(wasd, sta,1);		
		if(ASD_WLAN[wasd->WlanID]!=NULL&&ASD_WLAN[wasd->WlanID]->balance_switch == 1&&ASD_WLAN[wasd->WlanID]->balance_method==1)
			ap_free_sta(wasd, sta,1);
		else
			ap_free_sta(wasd, sta,0);
		return NULL;
	}
	sta->auth_alg = WLAN_AUTH_FT;

	return sta->wpa_sm;
}


static void asd_rrb_receive(void *ctx, const u8 *src_addr, const u8 *buf,
				size_t len)
{
	struct asd_data *wasd = ctx;
	wpa_ft_rrb_rx(wasd->wpa_auth, src_addr, buf, len);
}

#endif /* ASD_IEEE80211R */


/**
 * asd_validate_bssid_configuration - Validate BSSID configuration
 * @iface: Pointer to interface data
 * Returns: 0 on success, -1 on failure
 *
 * This function is used to validate that the configured BSSIDs are valid.
 */

#if 0
static int asd_validate_bssid_configuration(struct asd_iface *iface)
{
	u8 mask[ETH_ALEN] = { 0 };
	struct asd_data *wasd = iface->bss[0];
	unsigned int i = iface->conf->num_bss, bits = 0, j;
	int res;

	// Generate BSSID mask that is large enough to cover the BSSIDs. 

	// Determine the bits necessary to cover the number of BSSIDs. 
	for (i--; i; i >>= 1)
		bits++;

	// Determine the bits necessary to any configured BSSIDs,
	//   if they are higher than the number of BSSIDs. 
	for (j = 0; j < iface->conf->num_bss; j++) {
		if (asd_mac_comp_empty(iface->conf->bss[j].bssid) == 0)
			continue;

		for (i = 0; i < ETH_ALEN; i++) {
			mask[i] |=
				iface->conf->bss[j].bssid[i] ^
				wasd->own_addr[i];
		}
	}

	for (i = 0; i < ETH_ALEN && mask[i] == 0; i++)
		;
	j = 0;
	if (i < ETH_ALEN) {
		j = (5 - i) * 8;

		while (mask[i] != 0) {
			mask[i] >>= 1;
			j++;
		}
	}

	if (bits < j)
		bits = j;

	if (bits > 40)
		return -1;

	os_memset(mask, 0xff, ETH_ALEN);
	j = bits / 8;
	for (i = 5; i > 5 - j; i--)
		mask[i] = 0;
	j = bits % 8;
	while (j--)
		mask[i] <<= 1;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "BSS count %lu, BSSID mask " MACSTR " (%d bits)",
		   (unsigned long) iface->conf->num_bss, MAC2STR(mask), bits);

	res = asd_valid_bss_mask(wasd, wasd->own_addr, mask);
	if (res == 0)
		return 0;

	if (res < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Driver did not accept BSSID mask " MACSTR " for start "
		       "address " MACSTR ".\n",
		       MAC2STR(mask), MAC2STR(wasd->own_addr));
		return -1;
	}

	for (i = 0; i < ETH_ALEN; i++) {
		if ((wasd->own_addr[i] & mask[i]) != wasd->own_addr[i]) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Invalid BSSID mask " MACSTR " for start "
			       "address " MACSTR ".\n"
			       "Start address must be the first address in the"
			       " block (i.e., addr AND mask == addr).\n",
			       MAC2STR(mask), MAC2STR(wasd->own_addr));
			return -1;
		}
	}

	return 0;
}


static int mac_in_conf(struct asd_config *conf, const void *a)
{
	size_t i;

	for (i = 0; i < conf->num_bss; i++) {
		if (asd_mac_comp(conf->bss[i].bssid, a) == 0) {
			return 1;
		}
	}

	return 0;
}
#endif

int asd_setup_wpa(struct asd_data *wasd)
{
	struct wpa_auth_config _conf;
	struct wpa_auth_callbacks cb;
	const u8 *wpa_ie;
	size_t wpa_ie_len;

	asd_wpa_auth_conf(wasd->conf, &_conf);
	os_memset(&cb, 0, sizeof(cb));
	cb.ctx = wasd;
	cb.logger = asd_wpa_auth_logger;
	cb.disconnect = asd_wpa_auth_disconnect;
	cb.mic_failure_report = asd_wpa_auth_mic_failure_report;
	cb.set_eapol = asd_wpa_auth_set_eapol;
	cb.get_eapol = asd_wpa_auth_get_eapol;
	cb.get_psk = asd_wpa_auth_get_psk;
	cb.get_msk = asd_wpa_auth_get_msk;
	cb.set_key = asd_wpa_auth_set_key;
	cb.get_seqnum = asd_wpa_auth_get_seqnum;
	cb.get_seqnum_igtk = asd_wpa_auth_get_seqnum_igtk;
	cb.send_eapol = asd_wpa_auth_send_eapol;
	cb.for_each_sta = asd_wpa_auth_for_each_sta;
	cb.send_ether = asd_wpa_auth_send_ether;
#ifdef ASD_IEEE80211R
	cb.send_ft_action = asd_wpa_auth_send_ft_action;
	cb.add_sta = asd_wpa_auth_add_sta;
#endif /* ASD_IEEE80211R */
	wasd->wpa_auth = wpa_init(wasd->own_addr, &_conf, &cb);
	if (wasd->wpa_auth == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"WPA initialization failed.\n");
		return -1;
	}

	if (asd_set_privacy(wasd, 1)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not set PrivacyInvoked "
			   "for interface %s", wasd->conf->iface);
		return -1;
	}

	wpa_ie = wpa_auth_get_wpa_ie(wasd->wpa_auth, &wpa_ie_len);
	if (asd_set_generic_elem(wasd, wpa_ie, wpa_ie_len)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to configure WPA IE for "
			   "the kernel driver.");
		return -1;
	}
/*
	if (rsn_preauth_iface_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Initialization of RSN pre-authentication "
		       "failed.\n");
		return -1;
	}
*/
	return 0;

}

#if 0
static int asd_setup_radius_srv(struct asd_data *wasd,
				    struct asd_bss_config *conf)
{
	struct radius_server_conf srv;
	os_memset(&srv, 0, sizeof(srv));
	srv.client_file = conf->radius_server_clients;
	srv.auth_port = conf->radius_server_auth_port;
	srv.conf_ctx = conf;
	srv.eap_sim_db_priv = wasd->eap_sim_db_priv;
	srv.ssl_ctx = wasd->ssl_ctx;
	srv.pac_opaque_encr_key = conf->pac_opaque_encr_key;
	srv.eap_fast_a_id = conf->eap_fast_a_id;
	srv.eap_sim_aka_result_ind = conf->eap_sim_aka_result_ind;
	srv.ipv6 = conf->radius_server_ipv6;
	srv.get_eap_user = asd_radius_get_eap_user;

	wasd->radius_srv = radius_server_init(&srv);
	if (wasd->radius_srv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"RADIUS server initialization failed.\n");
		return -1;
	}

	return 0;
}


/**
 * asd_setup_bss - Per-BSS setup (initialization)
 * @wasd: Pointer to BSS data
 * @first: Whether this BSS is the first BSS of an interface
 *
 * This function is used to initialize all per-BSS data structures and
 * resources. This gets called in a loop for each BSS when an interface is
 * initialized. Most of the modules that are initialized here will be
 * deinitialized in asd_cleanup().
 */
static int asd_setup_bss(struct asd_data *wasd, int first)
{
	struct asd_bss_config *conf = wasd->conf;
	u8 ssid[asd_MAX_SSID_LEN + 1];
	int ssid_len, set_ssid;

	if (!first) {
		if (asd_mac_comp_empty(wasd->conf->bssid) == 0) {
			/* Allocate the next available BSSID. */
			do {
				inc_byte_array(wasd->own_addr, ETH_ALEN);
			} while (mac_in_conf(wasd->iconf, wasd->own_addr));
		} else {
			/* Allocate the configured BSSID. */
			os_memcpy(wasd->own_addr, wasd->conf->bssid, ETH_ALEN);

			if (asd_mac_comp(wasd->own_addr,
					     wasd->iface->bss[0]->own_addr) ==
			    0) {
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSS '%s' may not have BSSID "
				       "set to the MAC address of the radio\n",
				       wasd->conf->iface);
				return -1;
			}
		}

		wasd->interface_added = 1;
		if (asd_bss_add(wasd->iface->bss[0], wasd->conf->iface,
				    wasd->own_addr)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to add BSS (BSSID=" MACSTR ")\n",
			       MAC2STR(wasd->own_addr));
			return -1;
		}
	}

	/*
	 * Fetch the SSID from the system and use it or,
	 * if one was specified in the config file, verify they
	 * match.
	 */
	ssid_len = asd_get_ssid(wasd, ssid, sizeof(ssid));
	if (ssid_len < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not read SSID from system\n");
		return -1;
	}
	if (conf->ssid.ssid_set) {
		/*
		 * If SSID is specified in the config file and it differs
		 * from what is being used then force installation of the
		 * new SSID.
		 */
		set_ssid = (conf->ssid.ssid_len != (size_t) ssid_len ||
			    os_memcmp(conf->ssid.ssid, ssid, ssid_len) != 0);
	} else {
		/*
		 * No SSID in the config file; just use the one we got
		 * from the system.
		 */
		set_ssid = 0;
		conf->ssid.ssid_len = ssid_len;
		os_memcpy(conf->ssid.ssid, ssid, conf->ssid.ssid_len);
		conf->ssid.ssid[conf->ssid.ssid_len] = '\0';
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"Using interface %s with hwaddr " MACSTR " and ssid '%s'\n",
	       wasd->conf->iface, MAC2STR(wasd->own_addr),
	       wasd->conf->ssid.ssid);

	if (asd_setup_wpa_psk(conf)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"WPA-PSK setup failed.\n");
		return -1;
	}

	/* Set flag for whether SSID is broadcast in beacons */
	if (asd_set_broadcast_ssid(wasd,
				       !!wasd->conf->ignore_broadcast_ssid)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set broadcast SSID flag for kernel "
		       "driver\n");
		return -1;
	}

	if (asd_set_dtim_period(wasd, wasd->conf->dtim_period)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set DTIM period for kernel driver\n");
		return -1;
	}

	/* Set SSID for the kernel driver (to be used in beacon and probe
	 * response frames) */
	if (set_ssid && asd_set_ssid(wasd, (u8 *) conf->ssid.ssid,
					 conf->ssid.ssid_len)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set SSID for kernel driver\n");
		return -1;
	}

	if (wpa_debug_level == MSG_MSGDUMP)
		conf->radius->msg_dumps = 1;
	wasd->radius = radius_client_init(wasd, conf->radius);
	if (wasd->radius == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"RADIUS client initialization failed.\n");
		return -1;
	}

	if (asd_acl_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"ACL initialization failed.\n");
		return -1;
	}

	if (ieee802_1x_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"IEEE 802.1X initialization failed.\n");
		return -1;
	}

	if (wasd->conf->wpa && asd_setup_wpa(wasd))
		return -1;

	if (accounting_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Accounting initialization failed.\n");
		return -1;
	}

	if (wasd->conf->ieee802_11f &&
	    (wasd->iapp = iapp_init(wasd, wasd->conf->iapp_iface)) == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"IEEE 802.11F (IAPP) initialization failed.\n");
		return -1;
	}

	if (asd_ctrl_iface_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to setup control interface\n");
		return -1;
	}

	if (vlan_init(wasd)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"VLAN initialization failed.\n");
		return -1;
	}

#ifdef ASD_IEEE80211R
	wasd->l2 = l2_packet_init(wasd->conf->iface, NULL, ETH_P_RRB,
				  asd_rrb_receive, wasd, 0);
	if (wasd->l2 == NULL &&
	    (wasd->driver == NULL || wasd->driver->send_ether == NULL)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to open l2_packet interface\n");
		return -1;
	}
#endif /* ASD_IEEE80211R */

	ieee802_11_set_beacon(wasd);

	if (conf->radius_server_clients &&
	    asd_setup_radius_srv(wasd, conf))
		return -1;

	return 0;
}

/**
 * setup_interface2 - Setup (initialize) an interface (part 2)
 * @iface: Pointer to interface data.
 * Returns: 0 on success; -1 on failure.
 *
 * Flushes old stations, sets the channel, DFS parameters, encryption,
 * beacons, and WDS links based on the configuration.
 */

static int setup_interface2(struct asd_iface *iface)
{
	struct asd_data *wasd = iface->bss[0];
	int freq;
	size_t j;
	int ret = 0;
	u8 *prev_addr;

	asd_flush_old_stations(wasd);
	asd_set_privacy(wasd, 0);

	if (wasd->iconf->channel) {
		freq = asd_hw_get_freq(wasd, wasd->iconf->channel);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Mode: %s  Channel: %d  Frequency: %d MHz\n",
		       asd_hw_mode_txt(wasd->iconf->hw_mode),
		       wasd->iconf->channel, freq);

		if (asd_set_freq(wasd, wasd->iconf->hw_mode, freq)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set channel for kernel driver\n");
			return -1;
		}
	}

	asd_broadcast_wep_clear(wasd);
	if (asd_setup_encryption(wasd->conf->iface, wasd))
		return -1;

	asd_set_beacon_int(wasd, wasd->iconf->beacon_int);
	ieee802_11_set_beacon(wasd);

	if (wasd->iconf->rts_threshold > -1 &&
	    asd_set_rts(wasd, wasd->iconf->rts_threshold)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set RTS threshold for kernel driver\n");
		return -1;
	}

	if (wasd->iconf->fragm_threshold > -1 &&
	    asd_set_frag(wasd, wasd->iconf->fragm_threshold)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not set fragmentation threshold for kernel "
		       "driver\n");
		return -1;
	}

	prev_addr = wasd->own_addr;

	for (j = 0; j < iface->num_bss; j++) {
		wasd = iface->bss[j];
		if (j)
			os_memcpy(wasd->own_addr, prev_addr, ETH_ALEN);
		if (asd_setup_bss(wasd, j == 0))
			return -1;
		if (asd_mac_comp_empty(wasd->conf->bssid) == 0)
			prev_addr = wasd->own_addr;
	}

	ap_list_init(iface);

	if (asd_driver_commit(wasd) < 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "%s: Failed to commit driver "
			   "configuration", __func__);
		return -1;
	}

	return ret;
}
#endif

static void setup_interface_start(void *circle_data, void *user_ctx);
//static void setup_interface2_handler(void *circle_data, void *user_ctx);

/**
 * setup_interface_finalize - Finish setup interface & call the callback
 * @iface: Pointer to interface data.
 * @status: Status of the setup interface (0 on success; -1 on failure).
 * Returns: 0 on success; -1 on failure (e.g., was not in progress).
 */
static int setup_interface_finalize(struct asd_iface *iface, int status)
{
	asd_iface_cb cb;

	if (!iface->setup_cb)
		return -1;
	
	circle_cancel_timeout(setup_interface_start, iface, NULL);
//	circle_cancel_timeout(setup_interface2_handler, iface, NULL);
//	asd_select_hw_mode_stop(iface);

	cb = iface->setup_cb;

	iface->setup_cb = NULL;

	cb(iface, status);

	return 0;
}


#if 0
/**
 * setup_interface2_wrapper - Wrapper for setup_interface2()
 * @iface: Pointer to interface data.
 * @status: Status of the hw mode select.
 *
 * Wrapper for setup_interface2() to calls finalize function upon completion.
 */


static void setup_interface2_wrapper(struct asd_iface *iface, int status)
{
	int ret = status;
	if (ret)
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Could not select hw_mode and channel. (%d)\n", ret);
	else
		ret = setup_interface2(iface);

	setup_interface_finalize(iface, ret);
}


/**
 * setup_interface2_handler - Used for immediate call of setup_interface2
 * @circle_data: Stores the struct asd_iface * for the interface.
 * @user_ctx: Unused.
 */


static void setup_interface2_handler(void *circle_data, void *user_ctx)
{
	struct asd_iface *iface = circle_data;

	setup_interface2_wrapper(iface, 0);
}


static int asd_radius_get_eap_user(void *ctx, const u8 *identity,
				       size_t identity_len, int phase2,
				       struct eap_user *user)
{
	const struct asd_eap_user *eap_user;
	int i, count;

	eap_user = asd_get_eap_user(ctx, identity, identity_len, phase2);
	if (eap_user == NULL)
		return -1;

	if (user == NULL)
		return 0;

	os_memset(user, 0, sizeof(*user));
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
		user->password_hash = eap_user->password_hash;
	}
	user->force_version = eap_user->force_version;
	user->ttls_auth = eap_user->ttls_auth;

	return 0;
}
#endif

/**
 * setup_interface1 - Setup (initialize) an interface (part 1)
 * @iface: Pointer to interface data
 * Returns: 0 on success, -1 on failure
 *
 * Initializes the driver interface, validates the configuration,
 * and sets driver parameters based on the configuration.
 * Schedules setup_interface2() to be called immediately or after
 * hardware mode setup takes place. 
 */
static int setup_interface1(struct asd_iface *iface)
{
	struct asd_data *wasd = iface->bss[0];
//	struct asd_bss_config *conf = wasd->conf;
//	size_t i;
//	char country[4];
//	u8 *b = conf->bssid;

	/*
	 * Initialize the driver interface and make sure that all BSSes get
	 * configured with a pointer to this driver interface.
	 */
//	if (b[0] | b[1] | b[2] | b[3] | b[4] | b[5]) {
//		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_driver_init_bssid\n");
//		wasd->drv_priv = asd_driver_init_bssid(wasd, b);
//	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"asd_driver_init\n");
		wasd->drv_priv = asd_driver_init(wasd);
//	}

	if (wasd->drv_priv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s driver initialization failed.\n",
			wasd->driver ? wasd->driver->name : "Unknown");
		wasd->driver = NULL;
		return -1;
	}
/*	for (i = 0; i < iface->num_bss; i++) {
		iface->bss[i]->driver = wasd->driver;
		iface->bss[i]->drv_priv = wasd->drv_priv;
	}
*/
/*	if (asd_validate_bssid_configuration(iface))
		return -1;

	os_memcpy(country, wasd->iconf->country, 3);
	country[3] = '\0';
	if (asd_set_country(wasd, country) < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set country code\n");
		return -1;
	}

	if (wasd->iconf->ieee80211d || wasd->iconf->ieee80211h) {
		if (asd_set_ieee80211d(wasd, 1) < 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set ieee80211d (%d)\n",
			       wasd->iconf->ieee80211d);
			return -1;
		}
	}

	if (wasd->iconf->bridge_packets != INTERNAL_BRIDGE_DO_NOT_CONTROL &&
	    asd_set_internal_bridge(wasd, wasd->iconf->bridge_packets)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set bridge_packets for kernel driver\n");
		return -1;
	}
*/
	/* TODO: merge with asd_driver_init() ? 
	if (asd_wireless_event_init(wasd) < 0)
		return -1;

	if (asd_get_hw_features(iface)) {*/
		/* Not all drivers support this yet, so continue without hw
		 * feature data. */
/*	} else {
		return asd_select_hw_mode_start(iface,
						    setup_interface2_wrapper);
	}
*/
//	circle_register_timeout(0, 0, setup_interface2_handler, iface, NULL);
	return 0;
}


/**
 * setup_interface_start - Handler to start setup interface
 * @circle_data: Stores the struct asd_iface * for the interface.
 * @user_ctx: Unused.
 *
 * An circle handler is used so that all errors can be processed by the
 * callback without introducing stack recursion.
 */
static void setup_interface_start(void *circle_data, void *user_ctx)
{
	struct asd_iface *iface = circle_data;

	int ret;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"setup_interface1\n");
	ret = setup_interface1(iface);
	if (ret)
		setup_interface_finalize(iface, ret);
}


/**
 * asd_setup_interface_start - Start the setup of an interface
 * @iface: Pointer to interface data.
 * @cb: The function to callback when done.
 * Returns:  0 if it starts successfully; cb will be called when done.
 *          -1 on failure; cb will not be called.
 *
 * Initializes the driver interface, validates the configuration,
 * and sets driver parameters based on the configuration.
 * Flushes old stations, sets the channel, DFS parameters, encryption,
 * beacons, and WDS links based on the configuration.
 */
int asd_setup_interface_start(struct asd_iface *iface,
				  asd_iface_cb cb)
{
	if (iface->setup_cb) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG,
			   "%s: Interface setup already in progress.\n",
			   iface->bss[0]->conf->iface);
		return -1;
	}

	iface->setup_cb = cb;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"setup_interface_start\n");
	circle_register_timeout(0, 0, setup_interface_start, iface, NULL);

	return 0;
}


/**
 * asd_setup_interace_stop - Stops the setup of an interface
 * @iface: Pointer to interface data
 * Returns:  0 if successfully stopped;
 *          -1 on failure (i.e., was not in progress)
 */
int asd_setup_interface_stop(struct asd_iface *iface)
{
	return setup_interface_finalize(iface, -1);
}


/*
  *asd_get_ip - look up the arp cache for ip by mac
  * @mac:
  * @ip: Point to the ip will be save
  * Returns:  0 if successfully ;
  *          -1 on failure (i.e., was not in progress)
  */
int asd_get_ip(const u8 *mac,u8 *in_addr)		//ht add,081030
{
    FILE * fp;
    char * line = NULL;
    char * res = NULL;
    char smac[18]={0};
    u32 len = 0;
    u32 read;

	fp = fopen("/proc/net/arp","r");
    if(!fp)
    {
        asd_printf(ASD_DEFAULT,MSG_DEBUG,"File /proc/net/arp can't be read!\n");
        return -1;
    }

    sprintf(smac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(mac));
    asd_printf(ASD_DEFAULT,MSG_DEBUG,"MAC: %s\n",smac);

    while ((read = getline(&line, &len, fp)) != -1) {
        if((res = strstr(line,smac)) != NULL)
        {
                memcpy(in_addr,line,15);
                asd_printf(ASD_DEFAULT,MSG_DEBUG,"IP : %s\n",in_addr);
				os_free(line);
				line = NULL;
				fclose(fp);
				return 0;
        }
    }

	if(line) {
	    os_free(line);
		line = NULL;
	}
	fclose(fp);

    return -1;

}

#if 0
/**
 * asd_alloc_bss_data - Allocate and initialize per-BSS data
 * @wasd_iface: Pointer to interface data
 * @conf: Pointer to per-interface configuration
 * @bss: Pointer to per-BSS configuration for this BSS
 * Returns: Pointer to allocated BSS data
 *
 * This function is used to allocate per-BSS data structure. This data will be
 * freed after asd_cleanup() is called for it during interface
 * deinitialization.
 */
static struct asd_data *
asd_alloc_bss_data(struct asd_iface *wasd_iface,
		       struct asd_config *conf,
		       struct asd_bss_config *bss)
{
	struct asd_data *wasd;

	wasd = os_zalloc(sizeof(*wasd));
	if (wasd == NULL)
		return NULL;

	wasd->iconf = conf;
	wasd->conf = bss;
	wasd->iface = wasd_iface;

	if (wasd->conf->individual_wep_key_len > 0) {
		/* use key0 in individual key and key1 in broadcast key */
		wasd->default_wep_key_idx = 1;
	}

#ifdef EAP_TLS_FUNCS
	if (wasd->conf->eap_server &&
	    (wasd->conf->ca_cert || wasd->conf->server_cert ||
	     wasd->conf->dh_file)) {
		struct tls_connection_params params;

		wasd->ssl_ctx = tls_init(NULL);
		if (wasd->ssl_ctx == NULL) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to initialize TLS\n");
			goto fail;
		}

		os_memset(&params, 0, sizeof(params));
		params.ca_cert = wasd->conf->ca_cert;
		params.client_cert = wasd->conf->server_cert;
		params.private_key = wasd->conf->private_key;
		params.private_key_passwd = wasd->conf->private_key_passwd;
		params.dh_file = wasd->conf->dh_file;

		if (tls_global_set_params(wasd->ssl_ctx, &params)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to set TLS parameters\n");
			goto fail;
		}

		if (tls_global_set_verify(wasd->ssl_ctx,
					  wasd->conf->check_crl)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to enable check_crl\n");
			goto fail;
		}
	}
#endif /* EAP_TLS_FUNCS */

#ifdef EAP_SERVER
	if (wasd->conf->eap_sim_db) {
		wasd->eap_sim_db_priv =
			eap_sim_db_init(wasd->conf->eap_sim_db,
					asd_sim_db_cb, wasd);
		if (wasd->eap_sim_db_priv == NULL) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"Failed to initialize EAP-SIM database "
			       "interface\n");
			goto fail;
		}
	}
#endif /* EAP_SERVER */

	if (wasd->conf->assoc_ap)
		wasd->assoc_ap_state = WAIT_BEACON;

	wasd->driver = wasd->iconf->driver;

	return wasd;

#if defined(EAP_TLS_FUNCS) || defined(EAP_SERVER)
fail:
#endif
	/* TODO: cleanup allocated resources(?) */
	os_free(wasd);
	return NULL;
}

/**
 * asd_init - Allocate and initialize per-interface data
 * @config_file: Path to the configuration file
 * Returns: Pointer to the allocated interface data or %NULL on failure
 *
 * This function is used to allocate main data structures for per-interface
 * data. The allocated data buffer will be freed by calling
 * asd_cleanup_iface().
 */
static struct asd_iface * asd_init(const char *config_file)
{
	struct asd_iface *wasd_iface = NULL;
	struct asd_config *conf = NULL;
	struct asd_data *wasd;
	size_t i;

	wasd_iface = os_zalloc(sizeof(*wasd_iface));
	if (wasd_iface == NULL)
		goto fail;

	wasd_iface->config_fname = os_strdup(config_file);
	if (wasd_iface->config_fname == NULL)
		goto fail;

	conf = asd_config_read(wasd_iface->config_fname);
	if (conf == NULL)
		goto fail;
	wasd_iface->conf = conf;

	wasd_iface->num_bss = conf->num_bss;
	wasd_iface->bss = os_zalloc(conf->num_bss *
				    sizeof(struct asd_data *));
	if (wasd_iface->bss == NULL)
		goto fail;

	for (i = 0; i < conf->num_bss; i++) {
		wasd = wasd_iface->bss[i] =
			asd_alloc_bss_data(wasd_iface, conf,
					       &conf->bss[i]);
		if (wasd == NULL)
			goto fail;
	}

	return wasd_iface;

fail:
	if (conf)
		asd_config_free(conf);
	if (wasd_iface) {
		for (i = 0; wasd_iface->bss && i < wasd_iface->num_bss; i++) {
			wasd = wasd_iface->bss[i];
			if (wasd && wasd->ssl_ctx)
				tls_deinit(wasd->ssl_ctx);
		}

		os_free(wasd_iface->config_fname);
		wasd_iface->config_fname=NULL;
		os_free(wasd_iface->bss);
		wasd_iface->bss=NULL;
		os_free(wasd_iface);
		wasd_iface=NULL;
	}
	return NULL;
}
#endif

struct asd_iface * asd_interfaces_init(int DriverID){
	struct asd_iface *wasd_iface = NULL;	
	wasd_iface = os_zalloc(sizeof(*wasd_iface));
	wasd_iface->num_bss = 1;
	wasd_iface->bss = os_zalloc(wasd_iface->num_bss *
				    sizeof(struct asd_data *));
	
	wasd_iface->bss[0] = os_zalloc(sizeof(struct asd_data));
	wasd_iface->bss[0]->info = os_zalloc(sizeof(struct stat_info));	//ht add 090223
	wasd_iface->bss[0]->driver = asd_drivers[DriverID];
	wasd_iface->bss[0]->iface = wasd_iface;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",wasd_iface->bss[0]->driver->name);

	return wasd_iface;
}



/**
 * register_drivers - Register driver interfaces
 *
 * This function is generated by Makefile (into driver_conf.c) to call all
 * configured driver interfaces to register them to core asd.
 */
void register_drivers(void);


/**
 * setup_interface_done - Callback when an interface is done being setup.
 * @iface: Pointer to interface data.
 * @status: Status of the interface setup (0 on success; -1 on failure).
 */
static void setup_interface_done(struct asd_iface *iface, int status)
{
	if (status) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: Unable to setup interface.",
			   iface->bss[0]->conf->iface);
		circle_terminate();
	} else
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: Setup of interface done.",
			   iface->bss[0]->conf->iface);
}

void CWCaptrue(int n ,unsigned char *buffer){
		int t=0;
		while((n-t)>=16)
		{
			int i;
			printf("[");
			for(i=0;i<16;i++)
				printf("%02x ",buffer[t+i]);
			printf("]\t[");
			for(i=0;i<16;i++)
			{
				char ch=buffer[t+i];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
			t+=16;
		}

		if(n>t)
		{
			int i=t;
			printf("[");
			while(i<n)
				printf("%02x ",buffer[i++]);
			printf("]");
			i=n-t;
			i=16-i;
			while(i--)
				printf("   ");
			printf("\t[");
			i=t;
			while(i<n)
			{
				char ch=buffer[i++];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
		}
		printf("\n\n");
}
//weichao add for delete ARP
void asd_delete_sta_arp(struct sta_info  *sta){
	if(sta == NULL)
		return;
	if((sta->ipaddr != 0)&&(asd_sta_arp_listen !=0)){
		char mac[20];		
		memset(mac,0,20);
		sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(sta->addr));
		printf("sta->in_addr %s mac %s sta->arpifname %s\n",sta->in_addr, mac,sta->arpifname);
		ipneigh_modify(RTM_DELNEIGH, 0,sta->in_addr, mac,sta->arpifname);		
		sta->ipaddr = 0;
		sta->ip_addr.s_addr = 0 ; 
		memset(sta->in_addr,0,16);
		os_strncpy(sta->in_addr,"0.0.0.0",7);
		asd_printf(ASD_1X,MSG_DEBUG,"delete the arp !\n");
	}
}	

int AsdStaInfoToWID(struct asd_data *wasd, const u8 *addr, Operate op){
	asd_printf(ASD_80211,MSG_DEBUG,"in func %s\n",__func__);
	if(is_secondary == 1){
		asd_printf(ASD_80211,MSG_DEBUG,"it is bak now,return\n");
		return 0;		
	}
		
	TableMsg STA;
	int len;
	int exist_flag = 0;
	time_t now;
	time(&now);
	struct sta_info *sta = NULL;
	struct sta_static_info *static_sta = NULL;
	struct sta_static_info *tmp_sta = NULL;
	STA.Op = op;
	STA.Type = STA_TYPE;
	
	os_memset(&STA.u.STA,0,sizeof(aWSM_STA));
	STA.u.STA.BSSIndex = wasd->BSSIndex;
	STA.u.STA.WTPID = ((wasd->BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
	STA.u.STA.wlanId = wasd->WlanID;
	memcpy(STA.u.STA.STAMAC, addr, ETH_ALEN);
	sta = ap_get_sta(wasd,addr);
	if((op == WID_ADD) && (sta != NULL)) {
		if((static_sta = asd_get_static_sta(addr)) != NULL){
			tmp_sta = static_sta;
			while(tmp_sta != NULL){//the priority is higher is sta->wlanid is not 0
				if((os_memcmp(tmp_sta->addr, addr, ETH_ALEN) == 0)&&(tmp_sta->wlan_id == wasd->WlanID))
				{
					asd_printf(ASD_80211,MSG_DEBUG,"find correct static sta\n");				
					exist_flag = 1;
					break;
				}
				tmp_sta = tmp_sta->hnext;
			}
			if(exist_flag != 1){
				tmp_sta = static_sta;
				while(tmp_sta != NULL){//the priority is lower is sta->wlanid is 0
					if((os_memcmp(tmp_sta->addr, addr, ETH_ALEN) == 0)&&(tmp_sta->wlan_id == 0))
					{
						asd_printf(ASD_80211,MSG_DEBUG,"find correct static sta 2\n");				
						break;
					}
					tmp_sta = tmp_sta->hnext;
				}
			}

			if(tmp_sta != NULL){
				if(tmp_sta->sta_traffic_limit > 0){
					sta->vip_flag |= 0x1;
					sta->sta_traffic_limit = tmp_sta->sta_traffic_limit;
				}
				if(tmp_sta->sta_send_traffic_limit > 0){
					sta->vip_flag |= 0x2;
					sta->sta_send_traffic_limit = tmp_sta->sta_send_traffic_limit;
				}
				if(tmp_sta->vlan_id > 0)
					sta->vlan_id= tmp_sta->vlan_id;
			}
		}
		asd_delete_sta_arp(sta);
		if(0 == check_sta_authorized(wasd,sta))
			wasd->authorized_sta_num++;
		asd_printf(ASD_80211,MSG_DEBUG,"%s Add STA "MACSTR" \n",__func__,MAC2STR(addr));				
		asd_printf(ASD_80211,MSG_DEBUG,"traffic_limit %d\n",sta->sta_traffic_limit);				
		asd_printf(ASD_80211,MSG_DEBUG,"send_traffic_limit %d\n",sta->sta_send_traffic_limit);					
		asd_printf(ASD_80211,MSG_DEBUG,"vlan_id %d\n",sta->vlan_id);				
		if((sta->sta_traffic_limit > 0) && ((sta->vip_flag & 0x01) != 0))
			STA.u.STA.traffic_limit = sta->sta_traffic_limit;
		if((sta->sta_send_traffic_limit > 0) && ((sta->vip_flag & 0x02) != 0))
			STA.u.STA.send_traffic_limit = sta->sta_send_traffic_limit;
		if(sta->vlan_id > 0)
			STA.u.STA.vlan_id = sta->vlan_id;
	}
	len = sizeof(STA);
	asd_printf(ASD_80211,MSG_DEBUG,"AsdStaInfoToWID1\n");	
	if(op == WID_ADD){
		ASD_WTP_ST *WTP = ASD_WTP_AP[STA.u.STA.WTPID];
		if(WTP){
			unsigned char *ip = (unsigned char*)&(WTP->WTPIP);
			syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]STA :"MACSTR" access WTP %d,WTP MAC:"MACSTR",WTP IP:%d.%d.%d.%d,Access Time:%s\n",slotid,vrrid,
				MAC2STR(STA.u.STA.STAMAC),STA.u.STA.WTPID,MAC2STR(WTP->WTPMAC),ip[0],ip[1],ip[2],ip[3],ctime(&now));
		}        

		asd_printf(ASD_80211,MSG_INFO,"STA :"MACSTR" access WTP %d\n",MAC2STR(addr),STA.u.STA.WTPID);
		STA.u.STA.sta_num = wasd->authorized_sta_num;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"sta num %d\n",STA.u.STA.sta_num);	
		
	}else if(STA.Op==WID_DEL){
		ASD_WTP_ST *WTP = ASD_WTP_AP[STA.u.STA.WTPID];
		if(WTP){
			unsigned char *ip = (unsigned char*)&(WTP->WTPIP);
			syslog(LOG_INFO|LOG_LOCAL7, "[%d-%d]STA :"MACSTR" leave WTP %d,WTP MAC:"MACSTR",WTP IP:%d.%d.%d.%d,Leave Time:%s\n",slotid,vrrid,
				MAC2STR(STA.u.STA.STAMAC),STA.u.STA.WTPID,MAC2STR(WTP->WTPMAC),ip[0],ip[1],ip[2],ip[3],ctime(&now));
		}
		asd_printf(ASD_80211,MSG_INFO,"STA :"MACSTR" leave WTP %d\n",MAC2STR(addr),STA.u.STA.WTPID);
	}
	else if(RADIUS_STA_UPDATE == STA.Op && sta != NULL)
	{
		STA.u.STA.traffic_limit = sta->sta_traffic_limit;
		STA.u.STA.send_traffic_limit  = sta->sta_send_traffic_limit;
	}
	else 
		asd_printf(ASD_80211,MSG_INFO,"WTP %d ""has operation %d to STA:"MACSTR"\n",STA.u.STA.WTPID,STA.Op,MAC2STR(addr));
	if(sendto(TableSend, &STA, len, 0, (struct sockaddr *) &toWID.addr, toWID.addrlen) < 0){
		asd_printf(ASD_80211,MSG_CRIT,"%s sendto %s\n",__func__,strerror(errno));
		perror("send(wASDSocket)");
		asd_printf(ASD_80211,MSG_DEBUG,"AsdStaInfoToWID2\n");
//		close(sock);
		return -1;
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"AsdStaInfoToWID3\n");
	return 0;
}
//weichao add 
int  asd_send_dhcp_w(void *msg, size_t len, int flags)
{	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in %s\n",__func__);	//weichao test
	int tlen = 0;
	tlen = sendto(TableSend, msg, len, flags, (struct sockaddr *)&toDHCP.addr, toDHCP.addrlen);
	if(tlen<0){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s sendtoDHCP %s\n",__func__,strerror(errno));
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"TableSend =  %d\n",TableSend);
	}
	return tlen;
}

void asd_notice_to_dhcp(struct asd_data *wasd, const u8 *addr,int op)
{
	
	struct sta_info *sta = NULL;
	if(wasd == NULL )
		return ;
	
	sta = ap_get_sta(wasd,addr);
	if(sta == NULL)
		return ;
	TableMsg msg;
	int res;
	int length;
	os_memset(&msg.u.STA,0,sizeof(aWSM_STA));
	msg.Type= STA_TYPE;
	msg.Op = op;	
	os_memcpy(msg.u.STA.STAMAC,sta->addr,MAC_LEN);
	msg.u.STA.BSSIndex = wasd->BSSIndex;
	msg.u.STA.vrrid = vrrid;
	msg.u.STA.local = local;
	length = sizeof(msg);
	res = asd_send_dhcp_w(&msg, length, 0);
	if(res < 0)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"send to dhcp failture!\n");
	}
	else{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"send to dhcp messge ok!!!");
	/*	asd_printf(ASD_DEFAULT,MSG_DEBUG,"type = %d\n",msg.Type);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"Op = %d\n",msg.Op);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"BSSIndex = %d\n",msg.u.STA.BSSIndex);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"vrrid = %d\n",msg.u.STA.vrrid);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"local = %d\n",msg.u.STA.local);*/
		asd_printf(ASD_DBUS,MSG_DEBUG,"MAC:"MACSTR"\n",MAC2STR(msg.u.STA.STAMAC));
	}
}

void WAPI_OPENSSL_INIT(){
	CRYPTO_malloc_debug_init();
	CRYPTO_set_mem_debug_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
	ERR_load_crypto_strings();
	OpenSSL_add_all_digests();
}

int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n'))
			return 0;
		else
			return -1;
	}
	else
		return -1;
}

int read_ac_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	if (len < 0) {
		close(fd);
		return 1;
	}	
	close(fd);
	return 0;
}

void ASDInit(){
	/*
	char WTP_COUNT_PATH[] = "/devinfo/maxwtpcount";
	char buf[DEFAULT_LEN];
	memset(buf,0,DEFAULT_LEN);
	char WTP_COUNT_PATH_OEM[] = "/devinfo/maxwtpcount2";
	char buf_oem[DEFAULT_LEN];
	memset(buf_oem,0,DEFAULT_LEN);	
	if(read_ac_info(WTP_COUNT_PATH,buf) == 0){
		if(parse_int_ID(buf, &WTP_NUM_AUTELAN)==-1)
			WTP_NUM_AUTELAN = WTP_DEFAULT_NUM_AUTELAN;
	}else
		WTP_NUM_AUTELAN = WTP_DEFAULT_NUM_AUTELAN;
	
	printf("WTP_NUM_AUTELAN %d\n",WTP_NUM_AUTELAN);

	if(read_ac_info(WTP_COUNT_PATH_OEM,buf_oem) == 0){
		if(parse_int_ID(buf_oem, &WTP_NUM_OEM)==-1)
			WTP_NUM_OEM = WTP_DEFAULT_NUM_OEM;
	}else
		WTP_NUM_OEM = WTP_DEFAULT_NUM_OEM;
	
	printf("WTP_NUM_OEM %d\n",WTP_NUM_OEM);	

	WTP_NUM = WTP_NUM_AUTELAN + WTP_NUM_OEM;
	
	printf("num= %d,autelannum= %d oemnum = %d\n",WTP_NUM,WTP_NUM_AUTELAN,WTP_NUM_OEM);

	*/
	//int licensecount = 0;
	int i = 0;

	char WTP_COUNT_PATH_BASE[] = "/devinfo/maxwtpcount";
	char buf_base[DEFAULT_LEN];
	char strdir[DEFAULT_LEN];

	glicensecount = get_dir_wild_file_count("/devinfo","maxwtpcount");
	printf("glicensecount %d\n",glicensecount);
	/*xiaodawei add, 20101115, initialization for g_wtp_binding_count*/
	g_wtp_binding_count = malloc((glicensecount+1)*sizeof(LICENSE_TYPE *));
	for(i=0; i<glicensecount+1; i++){
		g_wtp_binding_count[i] = NULL;
	}
	if(glicensecount == 0)
	{
		/*xiaodawei modify, 20101029*/
		g_wtp_count = malloc(sizeof(LICENSE_TYPE *));
		g_wtp_count[0] = malloc(sizeof(LICENSE_TYPE));
		g_wtp_count[0]->gcurrent_wtp_count = 0;
		g_wtp_count[0]->gmax_wtp_count = WTP_DEFAULT_NUM_AUTELAN;
		g_wtp_count[0]->flag = 0;

		glicensecount = 1;
		/*WTP_NUM = WTP_DEFAULT_NUM_AUTELAN;*/
	}
	else
	{
		/*xiaodawei modify, 20101029*/
		g_wtp_count = malloc(glicensecount*(sizeof(LICENSE_TYPE *)));
		for(i=0; i<glicensecount; i++)
		{
			g_wtp_count[i] = malloc(sizeof(LICENSE_TYPE));
			g_wtp_count[i]->gcurrent_wtp_count = 0;
			g_wtp_count[i]->flag = 0;
			
			memset(strdir,0,DEFAULT_LEN);
			memset(buf_base,0,DEFAULT_LEN);	

			if(i == 0)
			{
				if(read_ac_info(WTP_COUNT_PATH_BASE,buf_base) == 0)
				{
					if(parse_int_ID(buf_base, &g_wtp_count[i]->gmax_wtp_count)==-1)
					g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_AUTELAN;
				}
				else
				{

					g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_AUTELAN;
				}
	
			}
			else
			{
				sprintf(strdir,"/devinfo/maxwtpcount%d",i+1);
				if(read_ac_info(strdir,buf_base) == 0)
				{
					if(parse_int_ID(buf_base, &g_wtp_count[i]->gmax_wtp_count)==-1)
					g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_OEM;
				}
				else
				{
					g_wtp_count[i]->gmax_wtp_count = WTP_DEFAULT_NUM_OEM;
				}
			}

			//printf("asd################ maxwtp[%d] = %d\n",i,*gmax_wtp_count[i]);
		}
		/*for(i=0; i<glicensecount; i++)
		{
			WTP_NUM += g_wtp_count[i]->gmax_wtp_count;
		}*/		
	}
	WTP_NUM += 1;
	G_RADIO_NUM = WTP_NUM*L_RADIO_NUM;
	BSS_NUM = G_RADIO_NUM*L_BSS_NUM;
	
	ASD_WTP = os_zalloc(WTP_NUM*(sizeof(WID_WTP *)));
	if(ASD_WTP==NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "in %s malloc wtp failed.\n",__func__);
		exit(1);
	}
	memset(ASD_WTP,0,WTP_NUM*(sizeof(WID_WTP *)));
	
	ASD_RADIO = os_zalloc(G_RADIO_NUM*(sizeof(WID_WTP_RADIO *)));
	if(ASD_RADIO==NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	memset(ASD_RADIO,0,G_RADIO_NUM*(sizeof(WID_WTP_RADIO *)));
	
	ASD_BSS = os_zalloc(BSS_NUM*(sizeof(WID_BSS *)));
	if(ASD_BSS ==NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	memset(ASD_BSS,0,BSS_NUM*(sizeof(WID_BSS *)));

	
	ASD_WTP_AP = os_zalloc(WTP_NUM*(sizeof(ASD_WTP_ST *)));
	if(ASD_WTP_AP ==NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	memset(ASD_WTP_AP,0,WTP_NUM*(sizeof(ASD_WTP_ST *)));

	//mahz add 2011.4.30
	ASD_WTP_AP_HISTORY = os_zalloc(WTP_NUM*(sizeof(ASD_WTP_ST_HISTORY *)));
	if(ASD_WTP_AP_HISTORY ==NULL){
		asd_printf(ASD_DEFAULT,MSG_CRIT, "%s malloc failed.\n",__func__);
		exit(1);
	}
	memset(ASD_WTP_AP_HISTORY,0,WTP_NUM*(sizeof(ASD_WTP_ST_HISTORY *)));
	//
//zhanglei add ASD_FDStas init
	ASD_FDStas = malloc(sizeof(asd_sta_flash_disconn));
	memset(ASD_FDStas,0,sizeof(asd_sta_flash_disconn));
}

//mahz copy from hmd.c 2011.10.31
int read_file_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}

void AsdStateInit(){
	char Board_Slot_Path[] = "/dbm/local_board/slot_id";
	unsigned int slot_id = 0;
	char tmpbuf[256] = {0};	//qiuchen
#ifdef _DISTRIBUTION_	
	if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&slot_id) == 0){
			slotid = slot_id;
		}	
	}
#else
	slotid = 0;
#endif
}

int CWCreateThreadMutex(pthread_mutex_t *theMutex) {
	if(theMutex == NULL) return 0;
	
	switch(pthread_mutex_init(theMutex, NULL)) {
		case 0: // success
			break;
		case ENOMEM:
			return 0;
		default:
			return 0;
	}
	return 1;
}

void CWDestroyThreadMutex(pthread_mutex_t *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}
void asd_pid_write()
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	pid_t myPid = 0;
	int fd;
#ifndef _DISTRIBUTION_	
	sprintf(pidPath,"%s%d%s", "/var/run/wcpss/asd", \
				vrrid, ".pid");
#else
	sprintf(pidPath,"%s%d_%d%s", "/var/run/wcpss/asd", \
				local,vrrid, ".pid");
#endif		
	fd = open(pidPath, O_RDWR|O_CREAT);
	myPid = getpid();	
	sprintf(pidBuf,"%d\n",myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	return;
}
void asd_pid_write_v2(char *name)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	pid_t myPid = 0;
	int fd;
#ifndef _DISTRIBUTION_	
	sprintf(pidPath,"%s%d%s", "/var/run/wcpss/asd_thread", \
				vrrid, ".pid");
#else
	sprintf(pidPath,"%s%d_%d%s", "/var/run/wcpss/asd_thread", \
				local,vrrid, ".pid");
#endif		
	fd = open(pidPath, O_RDWR|O_CREAT|O_APPEND);
	if(fd < 0)
		return;
	myPid = getpid();	
	sprintf(pidBuf,"%s -%d\n", name,myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	return;
}


int main(int argc, char *argv[])
{
	struct wasd_interfaces interfaces;
	int ret = 1;
	size_t i, j;
//	size_t num=4;
//	int c, debug = 2;
	int daemonize = 0;
	const char *pid_file = NULL;
	pthread_t ASD_DBUS;
	pthread_t STA_MANAGE;
//	pthread_t ASD_NETLINK;
#ifdef _DISTRIBUTION_	
		if(argc >= 2){
			local =  atoi(argv[1]);
			//slotid =  atoi(argv[2]);
			vrrid =  atoi(argv[2]);
		}else if(argc != 1){
			printf("argc %d, something wrong\n",argc);
			//return 0;
		}
#else
	if(argc > 1)
		vrrid = atoi(argv[1]);
	else
		vrrid = 0;
#endif
	openlog("asd", 0, LOG_DAEMON);
	asd_printf(ASD_DEFAULT,MSG_INFO,"syslog has opened!\n");
	AsdStateInit();
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"asd main islocal=%d, hansi%d-%d",local,slotid,vrrid);	
	asd_pid_write();
	asd_pid_write_v2("asd_main");	
	asd_logger_register_cb(asd_logger_cb);
	WAPI_OPENSSL_INIT();
    apps_startup();
	if (eap_server_register_methods()) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to register EAP methods");
		return -1;
	}
	none_init();	
	X509_init();
	ASDInit();	/* init global struct */
	CWASDDbusPathInit();	
	if (rtnl_open(&rth, 0) < 0)
	{
    	asd_printf(ASD_DEFAULT,MSG_CRIT,"rtnl_open(&rth, 0) failed,asd exit(1)!\n");	
		exit(1);
	}
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"asd netlink socket for arp modify ok!\n");

	
	if (rtnl_open(&rth1, RTMGRP_NEIGH) < 0)
	{
    	asd_printf(ASD_DEFAULT,MSG_CRIT,"rtnl_open(&rth1, RTMGRP_NEIGH) failed,asd exit(1)!\n");	
		exit(1);
	}
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"asd netlink socket for arp listen ok!\n");

	/* init global mutex */
	CWCreateThreadMutex(&asd_g_sta_mutex);
	CWCreateThreadMutex(&asd_g_wtp_mutex); 
	CWCreateThreadMutex(&asd_flash_disconn_mutex); 	
	CWCreateThreadMutex(&asd_g_hotspot_mutex);//qiuchen add it
	CWCreateThreadMutex(&asd_g_wlan_mutex);
	CWCreateThreadMutex(&asd_g_bss_mutex);

	/* init global socket */
	asd_sock = init_asd_bak_socket();	
	gsock = init_asd_sctp_socket(10086);
	asd_update_select_mode(NULL,NULL);
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"init asd_sock %d, for sta sync.\n",asd_sock);
	interfaces.count = G_RADIO_NUM;

	interfaces.iface = os_zalloc(interfaces.count *
				     sizeof(struct asd_iface *));
	if (interfaces.iface == NULL) {
		asd_printf(ASD_DEFAULT,MSG_CRIT, "in %s malloc wtp failed.\n",__func__);
		exit(1);
	}
	memset(interfaces.iface,0, interfaces.count * sizeof(struct asd_iface *));
	
	interfaces.count_wired = port_max_num;
	
	interfaces.iface_wired = os_zalloc(interfaces.count_wired *
				     sizeof(struct asd_iface *));
	if (interfaces.iface_wired == NULL) {
		asd_printf(ASD_DEFAULT,MSG_CRIT, "in %s malloc wtp failed.\n",__func__);
		exit(1);
	}
	memset(interfaces.iface_wired,0, interfaces.count_wired * sizeof(struct asd_iface *));

	if (circle_init(&interfaces)) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Failed to initialize event loop");
		return -1;
	}
	
	circle_register_signal(SIGHUP, handle_reload, NULL);
	circle_register_signal(SIGUSR1, handle_dump_state, NULL);
	circle_register_signal_terminate(handle_term, NULL);

	if(pthread_create(&ASD_DBUS, NULL, asd_dbus_thread, NULL) != 0) {
		asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_DBUS thread failed\n");
		return -1;
	}
	if(pthread_create(&STA_MANAGE, NULL, asd_sta_manage, NULL) != 0)	
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"ASD_STA_MANAGE thread failed!\n");
		return -1;
	}
	ASD_NETLINIK_INIT();
	interfaces.config = asd_interfaces_init(0);
	interfaces.config->interfaces = &interfaces;
	ret = asd_setup_interface_start(interfaces.config,
						    setup_interface_done);
#ifdef _D_WIRED_	
	interfaces.config_wired = asd_interfaces_init(1);
	interfaces.config_wired->interfaces = &interfaces;
	ret = asd_setup_interface_start(interfaces.config_wired,
						    setup_interface_done);
#endif	
	if (ret)
		goto out;
//	}
	

	if (daemonize && os_daemonize(pid_file)) {
		perror("daemon");
		goto out;
	}

    /* main thread run circle */
	circle_run();

	/* Disconnect associated stations from all interfaces and BSSes */
	for (i = 0; i < interfaces.count; i++) 
		{

		if(interfaces.iface[i]!=NULL)
			{	
				for (j = 0; j < interfaces.iface[i]->num_bss; j++) 
				{
					struct asd_data *wasd =
									interfaces.iface[i]->bss[j];
							asd_free_stas(wasd);
//			asd_flush_old_stations(wasd);
				}
			}
		}

	ret = 0;

 out:
	/* Deinitialize all interfaces */
	for (i = 0; i < interfaces.count; i++) {
		if (!interfaces.iface[i])
			continue;
		asd_setup_interface_stop(interfaces.iface[i]);
		asd_cleanup_iface_pre(interfaces.iface[i]);
		for (j = 0; j < interfaces.iface[i]->num_bss; j++) {
			struct asd_data *wasd =
				interfaces.iface[i]->bss[j];
			asd_cleanup(wasd);
			if (j == interfaces.iface[i]->num_bss - 1 &&
			    wasd->driver)
				asd_driver_deinit(wasd);
		}
		for (j = 0; j < interfaces.iface[i]->num_bss; j++){
			os_free(interfaces.iface[i]->bss[j]);
			interfaces.iface[i]->bss[j]=NULL;
		}
		asd_cleanup_iface(interfaces.iface[i]);
	}
	os_free(interfaces.iface);
	interfaces.iface=NULL;

	//circle_cancel_timeout(sta_info_to_wsm_timer, NULL, NULL);
	circle_destroy();

	closelog();

	eap_server_unregister_methods();

	os_daemonize_terminate(pid_file);

	return ret;
}
int get_dir_wild_file_count(char *dir, char *wildfile)
{
	DIR *dp = NULL;
	struct dirent *dirp;
	int wildfilecount = 0;

	dp = opendir(dir);
	if(dp == NULL)
	{
		return wildfilecount;
	}

	while((dirp = readdir(dp)) != NULL)
	{
		//printf("dirname = %s count = %d\n",dirp->d_name,wildfilecount);
		if((memcmp(dirp->d_name,wildfile,strlen(wildfile))) ==  0)
		{
			wildfilecount++;
			//printf("count = %d\n",wildfilecount);
		}
	}
	
	printf("last count = %d\n",wildfilecount);
	closedir(dp);
	return wildfilecount;
	
}
//qiuchen add it 2012.10.31
void get_sysruntime(time_t *sysruntime){
	struct timespec *sysrunt = NULL;;
	sysrunt = (struct timespec *)os_zalloc(sizeof(*sysrunt));
	if(sysrunt == NULL){
		asd_printf(ASD_DBUS,MSG_DEBUG,"%s,%d malloc error.\n",__func__,__LINE__);
		return;
	}
	clock_gettime(CLOCK_MONOTONIC,sysrunt);
	*sysruntime = sysrunt->tv_sec;
	free(sysrunt);
	sysrunt = NULL;
}
/* add for new format log for china mobile */
static char *asd_sprintf_desc(char *buf,char *fmt,char *desc)
{
	sprintf(buf,fmt,desc);
	return buf;
}
static void asd_get_ap_statistics(ap_statistics *ap_stat)
{
	int i = 0,j = 0;
	struct wasd_interfaces *interfaces = (struct wasd_interfaces*) circle.user_data;
	for(i=0;i<G_RADIO_NUM;){
		if((interfaces->iface[i] != NULL)&&(interfaces->iface[i]->bss != NULL)){
			for(j = 0; j < L_BSS_NUM; j++){
				if(interfaces->iface[i]->bss[j] == NULL)
					continue;
				else if(interfaces->iface[i]->bss[j] != NULL){
						ap_stat[i/4].total_assoc_num += interfaces->iface[i]->bss[j]->num_assoc;
						ap_stat[i/4].total_assoc_fail_num+= interfaces->iface[i]->bss[j]->num_assoc_failure;
						ap_stat[i/4].total_assoc_fail_sl_num+= interfaces->iface[i]->bss[j]->num_assoc_failure_sl;
						ap_stat[i/4].total_reassoc_num+= interfaces->iface[i]->bss[j]->num_reassoc;
						ap_stat[i/4].total_reassoc_fail_num+= interfaces->iface[i]->bss[j]->num_reassoc_failure;
						ap_stat[i/4].online_sta_num+= interfaces->iface[i]->bss[j]->num_sta;
						ap_stat[i/4].sta_drop_abnormal_num+= interfaces->iface[i]->bss[j]->abnormal_st_down_num;
					continue;
				}
			}
			i++;
		} else 
			i += 4 - i%L_RADIO_NUM;
	}
}
static void asd_get_roam_statistics_wlan(unsigned int total_online_sta_num,unsigned int r_times,unsigned int r_suc_times,unsigned int r_sta_num)
{
	int i=0;
	for(i=0;i<WLAN_NUM;i++){
		if(ASD_WLAN[i]){
			total_online_sta_num += ASD_WLAN[i]->wlan_accessed_sta_num;
			r_times += ASD_WLAN[i]->sta_roaming_times;
			r_suc_times += ASD_WLAN[i]->sta_roaming_suc_times;
			r_sta_num += ASD_WLAN[i]->r_num_sta;
		}
	}
}
void asd_log_ap_statistics()
{
	asd_syslog_auteview(LOG_INFO,AP_STAT,NULL,NULL,NULL,0,NULL);
	circle_register_timeout(AP_STATISTICS_LOG_INTERVAL,0,asd_log_ap_statistics,NULL,NULL);
}
void asd_log_roam_statistics()
{
	asd_syslog_auteview(LOG_INFO,RADIO_STAT,NULL,NULL,NULL,0,NULL);
	circle_register_timeout(ROAM_STATISTICS_LOG_INTERVAL,0,asd_log_roam_statistics,NULL,NULL);
}
void asd_syslog_auteview_acct_stop(struct asd_data *wasd,struct sta_info *sta,struct asd_sta_driver_data *data,int sessiontime)
{
	char *nas_id = NULL;
	char *nas_port_id = NULL;
	u8 *identity = NULL;
	if(data == NULL)
		return;
	if(sta == NULL || sta->eapol_sm == NULL)
		return;
	identity = sta->eapol_sm->identity;
	if(wasd){
		if(ASD_HOTSPOT[wasd->hotspot_id]){
			nas_id = ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier;
			nas_port_id = ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id;
		}
		else{
			if(wasd->conf&&wasd->conf->nas_identifier)
				nas_id = wasd->conf->nas_identifier;
			if(wasd->nas_port_id)
				nas_port_id = wasd->nas_port_id;
		}
	}
	syslog(LOG_INFO|LOG_LOCAL7,"RADIUS_ACCOUNT_STOP "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" ip=\"%s\" "LOG_MAC" vlan=\"%d\"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"][ACT"AUTELANID" "LOG_ACT"]",
		slotid,vrrid,identity,sta->in_addr,MAC2STR(sta->addr),sta->vlan_id,nas_id,nas_port_id,1,data->tx_bytes,data->rx_bytes,data->tx_packets,data->rx_packets,sessiontime);
}

#if 1
void asd_parse_log_rcode(int *Rcode,char *str)
{

	switch(*Rcode){
		case OPERATE_SUCCESS:
			//*Rcode = mobile_code;     /* for mobile code, we need make a exchange. zhangdi@autelan.com */
			strcpy(str,"OPERATE SUCCESS");
			break;
		case FLOW_BANLANCE:
			strcpy(str,"Because of Flow Banlance");
			break;
		case NUMBER_BANLANCE:
			strcpy(str,"Because of Number Banlance");
			break;
		case AUTH_ALG_FAIL:
			strcpy(str,"Authentication Algorithm Failed");
			break;
		case AUTH_TRANSNUM_WRONG:
			strcpy(str,"Authentication Transition Number Wrong");
			break;
		case STAMAC_BSSID:
			strcpy(str,"");
			break;
		case MAC_REJECTED:
			strcpy(str,"Mac Address Rejected");
			break;
		case VLANID_INVALID:
			strcpy(str,"Invalid Vlan ID");
			break;
		case WPA_SM_FAILED:
			strcpy(str,"");
			break;
		case NO_RESOURCE:
			strcpy(str,"No More Resources to Use");
			break;
		case ASSO_BEFORE_AUTH:
			strcpy(str,"Association before Authentication");
			break;
		case ASSO_PACKAGE_WRONG:
			strcpy(str,"Package Wrong");
			break;
		case UNKNOWN_SSID:
			strcpy(str,"Unknown SSID");
			break;
		case WME_ELEM_INVALID:
			strcpy(str,"");
			break;
		case RATES_NOT_SUPPORT:
			strcpy(str,"Not Supported Rates");
			break;
		case RATES_LEN_INVALID:
			strcpy(str,"");
			break;
		case NO_WPARASN_IE:
			strcpy(str,"NO WPA/RASN IE");
			break;
		case CIPHER_NOT_MATCH:
			strcpy(str,"Cipher Wrong");
			break;
		case NO_WAPI_IE:
			strcpy(str,"No Wapi IE");
			break;
		case STA_POWER_NOT_ACCEPTED:
			strcpy(str,"Sta Power not Accespted");
			break;
		case NO_MORE_AID:
			strcpy(str,"No more Association ID");
			break;
		case RADIUS_FAILED:
			strcpy(str,"Radius Authentication Failed");
			break;
		case RADIUS_SUCCESS:
			strcpy(str,"Radius Authentication Success");
			break;
		case PSK_SUCCESS:
			strcpy(str,"Psk Exchange Success");
			break;
		case PSK_FAILED:
			strcpy(str,"Psk Exchange Failed");
			break;
		default:
			strcpy(str,"Psk Exchange Failed");			
			break;
	}
}

#else 
void asd_parse_log_rcode(int *Rcode,char *str)
{
	switch(*Rcode){
		case FLOW_BANLANCE:
			*Rcode = 999;
			strcpy(str,"Load balance");
			break;
		case NUMBER_BANLANCE:
			*Rcode = 999;
			strcpy(str,"Load balance");
			break;
		case STAMAC_BSSID:
			*Rcode = 999;
			strcpy(str,"Sta MAC conflict with BSSID");
			break;
		case MAC_REJECTED:
			*Rcode = 999;
			strcpy(str,"Deny MAC");
			break;
		case VLANID_INVALID:
			*Rcode = 999;
			strcpy(str,"Invalid vlan ID");
			break;
		case ASSO_BEFORE_AUTH:
			*Rcode = 999;
			strcpy(str,"Assoc before Auth");
			break;
		case ASSO_PACKAGE_WRONG:
			*Rcode = 999;
			strcpy(str,"Wrong package");
			break;
		case UNKNOWN_SSID:
			*Rcode = 999;
			strcpy(str,"Unknown SSID");
			break;
		case WME_ELEM_INVALID:
			*Rcode = 999;
			strcpy(str,"Wrong package");
			break;
				/*
			WPA_SM_FAILED,
			NO_RESOURCE,
			RATES_NOT_SUPPORT,
			RATES_LEN_INVALID,
			NO_WPARASN_IE,
			CIPHER_NOT_MATCH,
			NO_WAPI_IE,
			STA_POWER_NOT_ACCEPTED,
			NO_MORE_AID,
			RADIUS_FAILED,
			RADIUS_SUCCESS,
			PSK_SUCCESS,
			PSK_FAILED*/
		default:
			break;
	}
}
#endif
void asd_syslog_auteview(int level,int type,struct ieee80211_mgmt *mgmt,struct asd_data *wasd,struct sta_info *sta,int Rcode,char *error_str)
{
	//FILE* fp;
	u8 sta_mac[MAC_LEN] = {0};
	u8 ap_mac[MAC_LEN] = {0};
	u8 preap_mac[MAC_LEN] = {0};
	unsigned int LRID = 0;/*Local radio id*/
	unsigned char bssid[ETH_ALEN] = {0};
	unsigned char wlanid = 0;
	unsigned char SID = 0;
	unsigned char wtpindex = 0;
	unsigned int securitytype = 0;
	unsigned int extensible_auth = 0;
	unsigned int encryptiontype = 0;
	char *ssid = NULL;
	char buf[2048] = {0};
	ap_statistics	ap_stat[WTP_NUM+1];
	int i = 0;
	memset(ap_stat,0,(WTP_NUM+1)*sizeof(ap_statistics));
	unsigned char *username = NULL;
	int vlanid = 0;
	char nas_port_id[NAS_PORT_ID_LEN+1] = {0};
	char nas_iden[NAS_IDENTIFIER_NAME+1] = {0};

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\nfunc:%s level %d type %d\n",__func__,level,type);
	
	if(wasd && ASD_HOTSPOT[wasd->hotspot_id]){
		memcpy(nas_iden,ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier,NAS_IDENTIFIER_NAME+1);
		memcpy(nas_port_id,ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id,NAS_PORT_ID_LEN+1);
	}
	if(mgmt)
		memcpy(sta_mac,mgmt->sa,MAC_LEN);
	else if(sta)
		memcpy(sta_mac,sta->addr,MAC_LEN);
	if(wasd){
		wlanid = wasd->WlanID;
		LRID = wasd->Radio_G_ID%4;
		wtpindex = wasd->Radio_G_ID/4;
		SID = wasd->SecurityID;
		memcpy(bssid,wasd->own_addr,ETH_ALEN);
	}
	if(ASD_WTP_AP[wtpindex]){
		memcpy(ap_mac,ASD_WTP_AP[wtpindex]->WTPMAC,MAC_LEN);
	}
	if(ASD_SECURITY[SID]){
		securitytype = ASD_SECURITY[SID]->securityType;
		extensible_auth = ASD_SECURITY[SID]->extensible_auth;
		encryptiontype = ASD_SECURITY[SID]->encryptionType;
	}
	
	/* Add SSID for BSS, instead of ESSID */
	if(wasd->conf != NULL)
	{
		ssid = (char *)wasd->conf->ssid.ssid;  
	}
	else if(ASD_WLAN[wasd->WlanID] != NULL)
	{
		ssid = ASD_WLAN[wasd->WlanID]->ESSID;
	}	
	
	if(sta){
		if(sta->eapol_sm)
			username =  sta->eapol_sm->identity;
		vlanid = sta->vlan_id;
	}
	switch(type){
		case STA_ASSOC_START: 
			/*fp = fopen("/var/log/qc.log","a+");
			if(fp == NULL){
				syslog(LOG_INFO|LOG_LOCAL7,"failed to open qc.log");
			}
			else{
				fprintf(fp,"1STA_ASSOC_START "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]\n",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
				fclose(fp);
			}*/
			syslog(level|LOG_LOCAL7,"STA_ASSOC_START "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
			break;
		case STA_ASSOC_SUCCESS: 
			syslog(level|LOG_LOCAL7,"STA_ASSOC_SUCCESS "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
			break;
		case STA_ASSOC_FAIL: 
			syslog(level|LOG_LOCAL7,"STA_ASSOC_FAIL "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"][REASON"AUTELANID" "LOG_CODE"%s",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype,/*"ASF"*/" ",Rcode,asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			break;
		case STA_AUTH_START: 
			syslog(level|LOG_LOCAL7,"STA_AUTH_START "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
			break;			
		case STA_AUTH_SUCCESS: 
			syslog(level|LOG_LOCAL7,"STA_AUTH_SUCCESS "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
			break;
		case STA_AUTH_FAIL:/*Reason Code need to fix!!!!!*//*有问题需处理*/
			/*if(((securitytype == IEEE8021X)&&(encryptiontype == WEP))||(((securitytype == WPA_E)||(securitytype == WPA2_E)))||(securitytype == MD5)||(extensible_auth == 1))
				syslog(level|LOG_LOCAL7,"STA_AUTH_FAIL "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"][REASON"AUTELANID" code=\"%s\"]",
					slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype,error_str);
			else{*/
			syslog(level|LOG_LOCAL7,"STA_AUTH_FAIL "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"][REASON"AUTELANID" "LOG_CODE"%s",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype,/*"WL"*/" ",Rcode,asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			//}
			break;
		case STA_REAUTH_SUCCESS: 
			syslog(level|LOG_LOCAL7,"STA_REAUTH_SUCCESS "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"]",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype);
			break;
		case STA_REAUTH_FAIL: 
			break;
		case DOT1X_USER_ONLINE:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"]",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,0);
			break;
		case DOT1X_USER_ONLINE_FAIL:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE_FAIL "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"][REASON"AUTELANID" "LOG_CODE"%s",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,0,"WL",Rcode,(Rcode != 999)?"]":asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			break;
		case DOT1X_USER_OFFLINE:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_OFFLINE "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS"][REASON"AUTELANID" "LOG_CODE"%s",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,"WL",Rcode,(Rcode != 999)?"]":asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			break;
		case DOT1X_USER_REONLINE:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"]",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,1);
			break;
		case DOT1X_USER_REONLINE_FAIL:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE_FAIL "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"][REASON"AUTELANID" code=\"%s\"]",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,1,error_str);
			break;
		case RADIUS_REREJECT:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE_FAIL "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"][REASON"AUTELANID" code=\"%s\"]",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,1,error_str);
			break;
		case RADIUS_REJECT:
			syslog(level|LOG_LOCAL7,"DOT1X_USER_ONLINE_FAIL "DS_STRUCTURE_ALL"[USER"AUTELANID" "LOG_NAME" "LOG_VLAN" "LOG_MAC"][RDS"AUTELANID" "LOG_RDS" "LOG_TYPE"][REASON"AUTELANID" code=\"%s\"]",
				slotid,vrrid,username,vlanid,MAC2STR(sta_mac),nas_iden,nas_port_id,0,error_str);
			break;
		case STA_LEAVING: 
			syslog(level|LOG_LOCAL7,"STA_LEAVE "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][AP"AUTELANID" "LOG_MAC" "LOG_RADIO"][WLAN"AUTELANID" "LOG_BSSID" "LOG_SSID" "LOG_SEC"][REASON"AUTELANID" "LOG_CODE"%s",
				slotid,vrrid,MAC2STR(sta_mac),MAC2STR(ap_mac),LRID,MAC2STR(bssid),ssid,securitytype,"WL",Rcode,(Rcode != 999)?"]":asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			break;			
		case STA_ROAM_START:					
			if(sta == NULL)
				break;
			if(ASD_WTP_AP[sta->preAPID])
				memcpy(preap_mac,ASD_WTP_AP[sta->preAPID]->WTPMAC,MAC_LEN);
			syslog(level|LOG_LOCAL7,"STA_ROAM_START "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][FROM"AUTELANID" "LOG_ROAM"][TO"AUTELANID" "LOG_ROAM"]",
				slotid,vrrid,MAC2STR(sta_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(sta->PreBSSID),MAC2STR(preap_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(bssid),MAC2STR(ap_mac));
			break;			
		case STA_ROAM_SUCCESS: 
			if(sta == NULL)
				break;
			if(ASD_WTP_AP[sta->preAPID])
				memcpy(preap_mac,ASD_WTP_AP[sta->preAPID]->WTPMAC,MAC_LEN);
			syslog(level|LOG_LOCAL7,"STA_ROAM_SUCCESS "DS_STRUCTURE_ALL"[STA"AUTELANID" "LOG_MAC"][FROM"AUTELANID" "LOG_ROAM"][TO"AUTELANID" "LOG_ROAM"]",
				slotid,vrrid,MAC2STR(sta_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(sta->PreBSSID),MAC2STR(preap_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(bssid),MAC2STR(ap_mac));
			break;
		case STA_ROAM_FAIL: 
			syslog(level|LOG_LOCAL7,"STA_ROAM_FAIL [STA"AUTELANID" "LOG_MAC"][FROM"AUTELANID" "LOG_ROAM"][TO"AUTELANID" "LOG_ROAM"][REASON"AUTELANID" "LOG_CODE"%s",
				MAC2STR(sta_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(sta->PreBSSID),MAC2STR(preap_mac),IPSTRINT(gASD_AC_MANAGEMENT_IP),MAC2STR(bssid),MAC2STR(ap_mac),/*"WL"*/" ",Rcode,asd_sprintf_desc(buf," "LOG_DESC"]",error_str));
			break;
		case AP_STAT: 
			if(is_secondary == 1)
				break;
			asd_get_ap_statistics(ap_stat);
			for(i=0;i<WTP_NUM+1;i++){
				if(ASD_WTP_AP[i])
					syslog(level|LOG_LOCAL7,"AP_STAT "DS_STRUCTURE_ALL"[AP"AUTELANID" "LOG_MAC"][STAT"AUTELANID" assoc_num=\"%d\" assoc_fail=\"%d\" assoc_fail_sl=\"%d\" reassoc_num=\"%d\" reassoc_fail=\"%d\" sta_num=\"%d\" sta_drop_num=\"%d\"]",
						slotid,vrrid,MAC2STR(ASD_WTP_AP[i]->WTPMAC),ap_stat[i].total_assoc_num,ap_stat[i].total_assoc_fail_num,ap_stat[i].total_assoc_fail_sl_num,ap_stat[i].total_reassoc_num,ap_stat[i].total_reassoc_fail_num,ap_stat[i].online_sta_num,ap_stat[i].sta_drop_abnormal_num);
			}
			break;
		case ROAM_STAT: 
			if(is_secondary == 1)
				break;
			unsigned int total_online_sta_num = 0;
			unsigned int r_suc_times = 0;
			unsigned int r_times = 0;
			unsigned int r_sta_num = 0;
			asd_get_roam_statistics_wlan(total_online_sta_num,r_times,r_suc_times,r_sta_num);
			syslog(level|LOG_LOCAL7,"ROAM_STAT "DS_STRUCTURE_ALL"[STAT"AUTELANID" online_sta=\"%d\" roam_sta=\"%d\" roam_num=\"%d\" roam_fail=\"%d\"]",
				slotid,vrrid,total_online_sta_num,r_sta_num,r_times,(r_times - r_suc_times));
			break;
	/*	case TUNNEL_STAT://? 
			syslog(level|LOG_LOCAL7,"",
				);
			break;
		case RADIO_STAT: //?
			syslog(level|LOG_LOCAL7,"",
				);
			break;*/
		case RADIUS_ACCOUNT_STOP: 
			/*	asd_syslog_auteview_acct_stop	*/
			break;
		default: 
			break;
	}
}
/* add end */

