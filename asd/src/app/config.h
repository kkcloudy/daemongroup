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
* config.h
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

#ifndef ASD_CONFIG_H
#define ASD_CONFIG_H

#include "defs.h"
#include "ip_addr.h"
#include "wpa_common.h"
#include "ASDRadius/radius_client.h"

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

typedef u8 macaddr[ETH_ALEN];

struct asd_radius_servers;
struct ft_remote_r0kh;
struct ft_remote_r1kh;

#define asd_MAX_SSID_LEN 32

#define NUM_WEP_KEYS 4
struct asd_wep_keys {
	u8 idx;
	u8 *key[NUM_WEP_KEYS];
	size_t len[NUM_WEP_KEYS];
	int keys_set;
	size_t default_len; /* key length used for dynamic key generation */
};

typedef enum {
		DEFAULT_VALUE = 0,
		ACCEPT_UNLESS_DENIED = 1,
		DENY_UNLESS_ACCEPTED = 2
} acl_policy;

struct acl_config {
	acl_policy  macaddr_acl;
	acl_policy  wlan_last_macaddr_acl;
	unsigned int num_deny_mac;
	unsigned int num_wids_mac;
	struct maclist *deny_mac;
	unsigned int num_accept_mac;
	struct maclist *accept_mac;
};

typedef enum asd_security_policy {
	SECURITY_PLAINTEXT = 0,
	SECURITY_STATIC_WEP = 1,
	SECURITY_IEEE_802_1X = 2,
	SECURITY_WPA_PSK = 3,
	SECURITY_WPA = 4
} secpolicy;

struct asd_ssid {
	char ssid[asd_MAX_SSID_LEN + 1];
	size_t ssid_len;
	int ssid_set;

	char vlan[IFNAMSIZ + 1];
	secpolicy security_policy;

	struct asd_wpa_psk *wpa_psk;
	char *wpa_passphrase;
	char *wpa_psk_file;

	struct asd_wep_keys wep;

#define DYNAMIC_VLAN_DISABLED 0
#define DYNAMIC_VLAN_OPTIONAL 1
#define DYNAMIC_VLAN_REQUIRED 2
	int dynamic_vlan;
#ifdef ASD_FULL_DYNAMIC_VLAN
	char *vlan_tagged_interface;
#endif /* ASD_FULL_DYNAMIC_VLAN */
	struct asd_wep_keys **dyn_vlan_keys;
	size_t max_dyn_vlan_keys;
};


#define VLAN_ID_WILDCARD -1

struct asd_vlan {
	struct asd_vlan *next;
	int vlan_id; /* VLAN ID or -1 (VLAN_ID_WILDCARD) for wildcard entry */
	char ifname[IFNAMSIZ + 1];
	int dynamic_vlan;
#ifdef ASD_FULL_DYNAMIC_VLAN

#define DVLAN_CLEAN_BR 	0x1
#define DVLAN_CLEAN_VLAN	0x2
#define DVLAN_CLEAN_VLAN_PORT	0x4
#define DVLAN_CLEAN_WLAN_PORT	0x8
	int clean;
#endif /* ASD_FULL_DYNAMIC_VLAN */
};

#define PMK_LEN 32
struct asd_wpa_psk {
	struct asd_wpa_psk *next;
	int group;
	u8 psk[PMK_LEN];
	u8 addr[ETH_ALEN];
};

#define EAP_USER_MAX_METHODS 8
struct asd_eap_user {
	struct asd_eap_user *next;
	u8 *identity;
	size_t identity_len;
	struct {
		int vendor;
		u32 method;
	} methods[EAP_USER_MAX_METHODS];
	u8 *password;
	size_t password_len;
	int phase2;
	int force_version;
	unsigned int wildcard_prefix:1;
	unsigned int password_hash:1; /* whether password is hashed with
				       * nt_password_hash() */
	int ttls_auth; /* EAP_TTLS_AUTH_* bitfield */
};


#define NUM_TX_QUEUES 8

struct asd_tx_queue_params {
	int aifs;
	int cwmin;
	int cwmax;
	int burst; /* maximum burst time in 0.1 ms, i.e., 10 = 1 ms */
	int configured;
};

struct asd_wme_ac_params {
	int cwmin;
	int cwmax;
	int aifs;
	int txopLimit; /* in units of 32us */
	int admission_control_mandatory;
};


/**
 * struct asd_bss_config - Per-BSS configuration
 */
struct asd_bss_config {
	char iface[IFNAMSIZ + 1];
	char bridge[IFNAMSIZ + 1];

	enum asd_logger_level logger_syslog_level, logger_stdout_level;

	unsigned int logger_syslog; /* module bitfield */
	unsigned int logger_stdout; /* module bitfield */

	char *dump_log_name; /* file name for state dump (SIGUSR1) */

	int max_num_sta; /* maximum number of STAs in station table */

	int dtim_period;

	int ieee802_1x; /* use IEEE 802.1X */
	int eapol_version;
	int eap_server; /* Use internal EAP server instead of external
			 * RADIUS server */
	struct asd_eap_user *eap_user;
	char *eap_sim_db;
	struct asd_ip_addr own_ip_addr;
	char *nas_identifier;
	struct asd_radius_servers *radius;

	struct asd_ssid ssid;

	char *eap_req_id_text; /* optional displayable message sent with
				* EAP Request-Identity */
	size_t eap_req_id_text_len;
	int eapol_key_index_workaround;

	size_t default_wep_key_len;
	int individual_wep_key_len;
	int wep_rekeying_period;
	int broadcast_key_idx_min, broadcast_key_idx_max;
	int eap_reauth_period;

	int ieee802_11f; /* use IEEE 802.11f (IAPP) */
	char iapp_iface[IFNAMSIZ + 1]; /* interface used with IAPP broadcast
					* frames */

	u8 assoc_ap_addr[ETH_ALEN];
	int assoc_ap; /* whether assoc_ap_addr is set */

/*
	enum {
		NONE = 0,
		ACCEPT_UNLESS_DENIED = 1,
		DENY_UNLESS_ACCEPTED = 2
	} macaddr_acl;
	macaddr *accept_mac;
	int num_accept_mac;
	macaddr *deny_mac;
	int num_deny_mac;
*/
	int auth_algs; /* bitfield of allowed IEEE 802.11 authentication
			* algorithms, WPA_AUTH_ALG_{OPEN,SHARED,LEAP} */

	int wpa; /* bitfield of WPA_PROTO_WPA, WPA_PROTO_RSN */
	int wapi;
	int wpa_key_mgmt;
#ifdef ASD_IEEE80211W
	enum {
		NO_IEEE80211W = 0,
		IEEE80211W_OPTIONAL = 1,
		IEEE80211W_REQUIRED = 2
	} ieee80211w;
#endif /* ASD_IEEE80211W */
	int wpa_pairwise;
	int wpa_group;
	int wpa_group_rekey;
	int wpa_keyupdate_timeout;
	int wpa_strict_rekey;
	int wpa_gmk_rekey;
	int rsn_pairwise;
	int rsn_preauth;
	char *rsn_preauth_interfaces;
	int peerkey;
	int wpa_once_group_rekey_time;	//mahz add 2011.1.3

#ifdef ASD_IEEE80211R
	/* IEEE 802.11r - Fast BSS Transition */
	u8 mobility_domain[MOBILITY_DOMAIN_ID_LEN];
	u8 r1_key_holder[FT_R1KH_ID_LEN];
	u32 r0_key_lifetime;
	u32 reassociation_deadline;
	struct ft_remote_r0kh *r0kh_list;
	struct ft_remote_r1kh *r1kh_list;
	int pmk_r1_push;
#endif /* ASD_IEEE80211R */

	char *ctrl_interface; /* directory for UNIX domain sockets */
	gid_t ctrl_interface_gid;
	int ctrl_interface_gid_set;

	char *ca_cert;
	char *server_cert;
	char *private_key;
	char *private_key_passwd;
	int check_crl;
	char *dh_file;
	u8 *pac_opaque_encr_key;
	char *eap_fast_a_id;
	int eap_sim_aka_result_ind;

	char *radius_server_clients;
	int radius_server_auth_port;
	int radius_server_ipv6;

	char *test_socket; /* UNIX domain socket path for driver_test */

	int use_pae_group_addr; /* Whether to send EAPOL frames to PAE group
				 * address instead of individual address
				 * (for driver_wired.c).
				 */

	int ap_max_inactivity;
	int ignore_broadcast_ssid;

	int wme_enabled;

	int wapi_radius_auth_enable;			//mahz add 2010.11.24
	u8 *user_passwd;						//mahz add 2010.12.9
	size_t user_passwd_len;					//mahz add 2010.12.9
	
	struct asd_vlan *vlan, *vlan_tail;

	macaddr bssid;
};


typedef enum {
	asd_MODE_IEEE80211B,
	asd_MODE_IEEE80211G,
	asd_MODE_IEEE80211A,
	NUM_asd_MODES
} asd_hw_mode;


/**
 * struct asd_config - Per-radio interface configuration
 */
struct asd_config {
	struct asd_bss_config *bss, *last_bss;
	struct asd_radius_servers *radius;
	size_t num_bss;

	u16 beacon_int;
	int rts_threshold;
	int fragm_threshold;
	u8 send_probe_response;
	u8 channel;
	asd_hw_mode hw_mode; /* asd_MODE_IEEE80211A, .. */
	enum {
		LONG_PREAMBLE = 0,
		SHORT_PREAMBLE = 1
	} preamble;
	enum {
		CTS_PROTECTION_AUTOMATIC = 0,
		CTS_PROTECTION_FORCE_ENABLED = 1,
		CTS_PROTECTION_FORCE_DISABLED = 2,
		CTS_PROTECTION_AUTOMATIC_NO_OLBC = 3,
	} cts_protection_type;

	int *supported_rates;
	int *basic_rates;

	const struct wpa_driver_ops *driver;

	int passive_scan_interval; /* seconds, 0 = disabled */
	int passive_scan_listen; /* usec */
	int passive_scan_mode;
	int ap_table_max_size;
	int ap_table_expiration_time;

	char country[3]; /* first two octets: country code as described in
			  * ISO/IEC 3166-1. Third octet:
			  * ' ' (ascii 32): all environments
			  * 'O': Outdoor environemnt only
			  * 'I': Indoor environment only
			  */

	int ieee80211d;
	unsigned int ieee80211h; /* Enable/Disable 80211h */

	struct asd_tx_queue_params tx_queue[NUM_TX_QUEUES];

	/*
	 * WME AC parameters, in same order as 802.1D, i.e.
	 * 0 = BE (best effort)
	 * 1 = BK (background)
	 * 2 = VI (video)
	 * 3 = VO (voice)
	 */
	struct asd_wme_ac_params wme_ac_params[4];

	enum {
		INTERNAL_BRIDGE_DO_NOT_CONTROL = -1,
		INTERNAL_BRIDGE_DISABLED = 0,
		INTERNAL_BRIDGE_ENABLED = 1
	} bridge_packets;
#ifdef ASD_IEEE80211N
	int ht_op_mode_fixed;
	u16 ht_capab;
#endif /* ASD_IEEE80211N */
	int ieee80211n;
	int secondary_channel;
};


int asd_mac_comp(const void *a, const void *b);
int asd_mac_comp_empty(const void *a);
struct asd_config * asd_config_read(const char *fname);
void asd_config_free(struct asd_config *conf);
int asd_maclist_found(struct maclist *list, const u8 *addr);
int asd_rate_found(int *list, int rate);
int asd_wep_key_cmp(struct asd_wep_keys *a,
			struct asd_wep_keys *b);
const u8 * asd_get_psk(const struct asd_bss_config *conf,
			   const u8 *addr, const u8 *prev_psk);
int asd_setup_wpa_psk(struct asd_bss_config *conf);
const char * asd_get_vlan_id_ifname(struct asd_vlan *vlan,
					int vlan_id);
const struct asd_eap_user *
asd_get_eap_user(const struct asd_bss_config *conf, const u8 *identity,
		     size_t identity_len, int phase2);
void asd_config_defaults_bss(struct asd_bss_config *bss,unsigned char ID);
int add_mac_in_maclist(struct acl_config *conf, unsigned char *addr, char type);//xm add 08/11/04
int del_mac_in_maclist(struct acl_config *conf, u8 *addr, char type);//ht add
int del_mac_in_ac_maclist(struct acl_config *conf, u8 *addr, char type);
int add_mac_in_ac_maclist(struct acl_config *conf, u8 *addr, char type);
int change_maclist_security(struct acl_config *conf, char type);
int asd_config_read_radius_addr(struct asd_radius_server **server,
				int *num_server, const char *val, int def_port,
				struct asd_radius_server **curr_serv);
#endif /* ASD_CONFIG_H */
