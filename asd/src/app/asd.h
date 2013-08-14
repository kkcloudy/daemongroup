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
* Asd.h
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

#ifndef asd_H
#define asd_H
#include <pthread.h>
#include "common.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "include/auth.h"
#include "ap.h"
#include <dirent.h>
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif
#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */

#define ETH_P_WAPI  0x88B4

#endif /* ETH_P_PAE */
#ifndef ETH_P_EAPOL
#define ETH_P_EAPOL ETH_P_PAE
#endif /* ETH_P_EAPOL */

#ifndef ETH_P_RRB
#define ETH_P_RRB 0x890D
#endif /* ETH_P_RRB */

#include "config.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

#define MAX_VLAN_ID 4094

struct ieee8023_hdr {
	u8 dest[6];
	u8 src[6];
	u16 ethertype;
} STRUCT_PACKED;


struct ieee80211_hdr {
	le16 frame_control;
	le16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	le16 seq_ctrl;
	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
	 */
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

#define IEEE80211_DA_FROMDS addr1
#define IEEE80211_BSSID_FROMDS addr2
#define IEEE80211_SA_FROMDS addr3

#define IEEE80211_HDRLEN (sizeof(struct ieee80211_hdr))

#define IEEE80211_FC(type, stype) host_to_le16((type << 2) | (stype << 4))
#define HYBRID_AUTH_EAPOL_SUCCESS(sta) (HYBRID_AUTH_EAPOL == sta->security_type)&&(sta->flags & WLAN_STA_AUTHORIZED)

/* MTU to be set for the wlan#ap device; this is mainly needed for IEEE 802.1X
 * frames that might be longer than normal default MTU and they are not
 * fragmented */
#define asd_MTU 2290

extern int gasdPRINT;
extern unsigned char rfc1042_header[6];
extern struct sta_info *ASD_STA_HASH[STA_HASH_SIZE*STA_HASH_SIZE];//add for global sta hash 256*256
extern struct sta_acct_info *ASD_ACCT_HASH[STA_HASH_SIZE];
extern struct asd_ip_secret *ASD_SECRET_HASH[STA_HASH_SIZE];

struct asd_sta_driver_data {
	unsigned long rx_packets, tx_packets, rx_bytes, tx_bytes;
	unsigned long current_tx_rate;
	unsigned long inactive_msec;
	unsigned long flags;
	unsigned long num_ps_buf_frames;
	unsigned long tx_retry_failed;
	unsigned long tx_retry_count;
	int last_rssi;
	int last_ack_rssi;
};

struct wpa_driver_ops;
struct wpa_ctrl_dst;
struct radius_server_data;

#ifdef ASD_FULL_DYNAMIC_VLAN
struct full_dynamic_vlan;
#endif /* ASD_FULL_DYNAMIC_VLAN */

/**
 *struct stat_info -asd per BSS statistics infomation(auth,assoc,reassoc)
 */
struct stat_info {	
	u32 auth_invalid;
	u32 auth_timeout;
	u32 auth_refused;
	u32 auth_others;
	
	u32 assoc_invalid;
	u32 assoc_timeout;
	u32 assoc_refused;
	u32 assoc_others;

	u32 reassoc_invalid;
	u32 reassoc_timeout;
	u32 reassoc_refused;
	u32 reassoc_others;

	u32 identify_request;
	u32 identify_success;
	u32 abort_key_error;
	u32 abort_invalid;
	u32 abort_timeout;
	u32 abort_refused;
	u32 abort_others;
	
	u32 deauth_request;
	u32 deauth_user_leave;
	u32 deauth_ap_unable;
	u32 deauth_abnormal;
	u32 deauth_others;

	u32 disassoc_request;
	u32 disassoc_user_leave;
	u32 disassoc_ap_unable;
	u32 disassoc_abnormal;
	u32 disassoc_others;

	u32 rx_mgmt_pkts;
	u32 tx_mgmt_pkts;
	u32 rx_ctrl_pkts;
	u32 tx_ctrl_pkts;
	u32 rx_data_pkts;
	u32 tx_data_pkts;
	u32 rx_auth_pkts;
	u32 tx_auth_pkts;

	u64 rx_data_bytes;
	u64 tx_data_bytes;
};
#ifdef ASD_MULTI_THREAD_MODE
#define ASD_USE_PERBSS_LOCK
#endif
/**
 * struct asd_data - asd per-BSS data structure
 */
struct asd_data {
	struct asd_iface *iface;
	struct asd_config *iconf;
	struct asd_bss_config *conf;
	int interface_added; /* virtual interface added for this BSS */
	unsigned int Radio_G_ID;
	unsigned char Radio_L_ID;
	unsigned char WlanID;
	unsigned char SecurityID;
	unsigned int BSSIndex;
	unsigned int PORTID;
	unsigned int VLANID;
	unsigned char own_addr[ETH_ALEN];
	unsigned char bss_iface_type;
	char br_ifname[IF_NAME_MAX];
	unsigned short seq_num ;

	unsigned int th_deny_num;   //for   trap_helper   use. xm add
	unsigned int abnormal_st_down_num;//nl091120
	unsigned int normal_st_down_num;//nl091120
	unsigned int assoc_reject_no_resource;   //ht add for omc,110121,mahz copy from 1.2,2011.4.7

	int num_sta; /* number of entries in sta_list */
	struct sta_info *sta_list; /* STA info list head */
	struct sta_info *sta_hash[STA_HASH_SIZE];

	/* pointers to STA info; based on allocated AID or NULL if AID free
	 * AID is in the range 1-2007, so sta_aid[0] corresponders to AID 1
	 * and so on
	 */
	struct sta_info *sta_aid[MAX_AID_TABLE_SIZE];

	const struct wpa_driver_ops *driver;
	void *drv_priv;	

	u8 *default_wep_key;
	u8 default_wep_key_idx;
	char	nas_port_id[11];				//mahz add 2011.5.26

	struct radius_client_data *radius;
	int radius_client_reconfigured;
	u32 acct_session_id_hi, acct_session_id_lo;
	u32 num_assoc, num_reassoc, num_assoc_failure, num_reassoc_failure;		//ht add,090213
	u32 assoc_success, reassoc_success, assoc_req, assoc_resp;
	u32 auth_success, auth_fail;
	u32 sta_assoced;
	u32 assoc_req_timer_update, assoc_resp_timer_update;
	u32 assoc_success_all_timer_update;
	u32 acc_tms;
	u32 usr_auth_tms;
	u32 ac_rspauth_tms;

	u64 total_past_online_time;	//	xm0703
	u64 total_present_online_time;	//	ht,10.03.31
	time_t total_present_online_time_sysruntime;//qiuchen add it

	struct stat_info *info;
	struct iapp_data *iapp;

	enum { DO_NOT_ASSOC = 0, WAIT_BEACON, AUTHENTICATE, ASSOCIATE,
	       ASSOCIATED } assoc_ap_state;
	char assoc_ap_ssid[33];
	int assoc_ap_ssid_len;
	u16 assoc_ap_aid;

	struct asd_cached_radius_acl *acl_cache;
	struct asd_acl_query_data *acl_queries;

	struct wpa_authenticator *wpa_auth;
	struct eapol_authenticator *eapol_auth;

	struct rsn_preauth_interface *preauth_iface;
	time_t michael_mic_failure;
	int michael_mic_failures;
	int tkip_countermeasures;

	int ctrl_sock;
	struct wpa_ctrl_dst *ctrl_dst;

	void *ssl_ctx;
	void *eap_sim_db_priv;
	struct radius_server_data *radius_srv;

	int parameter_set_count;

#ifdef ASD_FULL_DYNAMIC_VLAN
	struct full_dynamic_vlan *full_dynamic_vlan;
#endif /* ASD_FULL_DYNAMIC_VLAN */

	struct l2_packet_data *l2;
	int wapi;
	struct asd_wapi *wapi_wasd;

	unsigned int	traffic_limit;
	unsigned int	sta_average_traffic_limit;
	unsigned int	send_traffic_limit;	/*下行,ht add 090902*/
	unsigned int	sta_average_send_traffic_limit;
#ifdef ASD_USE_PERBSS_LOCK	
	pthread_mutex_t asd_sta_mutex;
#endif

	//mahz add 2011.11.9 for GuangZhou Mobile
	unsigned int no_auth_sta_num;			/*免认证接入用户数*/
	unsigned int assoc_auth_sta_num;	/*关联认证接入用户数*/
	
	unsigned int no_auth_downline_time;
	unsigned int no_auth_online_time;
	unsigned int assoc_auth_downline_time;
	unsigned int assoc_auth_online_time;
	unsigned int no_auth_accessed_total_time;			/*免认证用户连接AP总时长*/
	unsigned int assoc_auth_accessed_total_time;	/*关联认证用户连接AP 总时长*/
	
	unsigned int no_auth_sta_abnormal_down_num;		/*异常掉线次数*/
	unsigned int assoc_auth_sta_abnormal_down_num;

	unsigned int assoc_auth_req_num;
	unsigned int assoc_auth_succ_num;
	unsigned int assoc_auth_fail_num;
	unsigned int hotspot_id;
	char master_to_disable;//qiuchen add it for AXSSZFI-1191
	unsigned int authorized_sta_num;
//qiuchen copy from 1.3.16
	union status {		
		struct weppsk /* WEP/PSK associate auth (SHARE:WEP) */
		{
			unsigned int online_sta_num;
			unsigned long total_offline_sta_time;		/* only statistics offline  sta time */
			unsigned int total_sta_drop_cnt;		/* total sta drop count */
			u32 num_assoc, num_reassoc, num_assoc_failure, num_reassoc_failure;
			u32 assoc_success, reassoc_success, assoc_req, assoc_resp;
		}weppsk;
		
		struct assoc_auth	/* assoc auth */
		{
			unsigned int assoc_auth_req_num, assoc_auth_succ_num, assoc_auth_fail_num;
		} assoc_auth;
		
		struct eapauth
		{
			struct autoauth 	/* SIM/PEAP */
			{
				unsigned int online_sta_num;
				unsigned long long total_offline_sta_time;		/* only statistics offline  sta time */
				unsigned int total_sta_drop_cnt;		/* sta drop count */

				unsigned int auth_req_cnt; 		/* first access request times */
				unsigned int auth_suc_cnt; 
				unsigned int auth_fail_cnt;
				unsigned int auth_resp_cnt; 	/* radius auth auth response count */
			} autoauth;
		} eap_auth;
	}u;
	struct statistics
	{
		unsigned int online_sta_num;
		unsigned long long offline_sta_time; 	/* total offline sta time 所有用户在线时长*/
		unsigned int sta_drop_cnt;		/* sta drop count 所有异常离线用户数*/

		unsigned int auth_req_cnt;		
		unsigned int auth_suc_cnt; 
		unsigned int auth_fail_cnt;
	} statistics;
};


struct asd_stainfo{
	struct asd_data *bss;
	struct sta_info	*sta;
};
struct asd_to_wifi_sta{
	unsigned char STAMAC[MAC_LEN];
	unsigned char BSSID_Before[MAC_LEN];
	unsigned char BSSID[MAC_LEN];
	unsigned char roaming_flag;
};


/**
 * asd_iface_cb - Generic callback type for per-iface asynchronous requests
 * @iface: the interface the event occured on.
 * @status: 0 if the request succeeded; -1 if the request failed.
 */
typedef void (*asd_iface_cb)(struct asd_iface *iface, int status);


struct asd_config_change;

/**
 * struct asd_iface - asd per-interface data structure
 */
struct asd_iface {
	struct wasd_interfaces *interfaces;
	char *config_fname;
	struct asd_config *conf;

	asd_iface_cb setup_cb;

	size_t num_bss;
	struct asd_data **bss;

	int num_ap; /* number of entries in ap_list */
	struct ap_info *ap_list; /* AP info list head */
	struct ap_info *ap_hash[STA_HASH_SIZE];
	struct ap_info *ap_iter_list;

	struct asd_hw_modes *hw_features;
	int num_hw_features;
	struct asd_hw_modes *current_mode;
	/* Rates that are currently used (i.e., filtered copy of
	 * current_mode->channels */
	int num_rates;
	struct asd_rate_data *current_rates;
	asd_iface_cb hw_mode_sel_cb;

	u16 hw_flags;
	
	u32 auth_times;	 				/*ht 100408*/
	u32 access_times;	 			/*ht 100408*/
	u64 total_past_online_time;	 	/*ht 100408*/
	u64 past_online_time;	 	/*ht 100408*/		/*clean when wtp quit*/

	/* Number of associated Non-ERP stations (i.e., stations using 802.11b
	 * in 802.11g BSS) */
	int num_sta_non_erp;

	/* Number of associated stations that do not support Short Slot Time */
	int num_sta_no_short_slot_time;

	/* Number of associated stations that do not support Short Preamble */
	int num_sta_no_short_preamble;

	int olbc; /* Overlapping Legacy BSS Condition */

	/* Number of HT associated stations that do not support greenfield */
	int num_sta_ht_no_gf;

	/* Number of associated non-HT stations */
	int num_sta_no_ht;

	/* Number of HT associated stations 20 MHz */
	int num_sta_ht_20mhz;

	/* Overlapping BSS information */
	int olbc_ht;
	
#ifdef ASD_IEEE80211N
	u16 ht_op_mode;
#endif /* ASD_IEEE80211N */

	int dfs_enable;
	u8 pwr_const;
	unsigned int tx_power;
	unsigned int sta_max_power;

	unsigned int channel_switch;

	struct asd_config_change *change;
	asd_iface_cb reload_iface_cb;
	asd_iface_cb config_reload_cb;
};

struct wasd_interfaces {
	size_t count;
	size_t count_wired;//zhang lei add
	struct asd_iface **iface;	
	struct asd_iface *config;//zhang lei add
	struct asd_iface **iface_wired;//zhang lei add
	struct asd_iface *config_wired;//zhang lei add
};
typedef enum{ 
	HANSI_REMOTE = 0,
	HANSI_LOCAL
} HANSI_TYPE;

extern unsigned char gasdtrapflag;
extern pthread_mutex_t asd_g_sta_mutex;
extern pthread_mutex_t asd_g_wtp_mutex;
extern pthread_mutex_t asd_flash_disconn_mutex;
extern pthread_mutex_t asd_g_hotspot_mutex;//qiuchen add it 2012.10.26
extern pthread_mutex_t asd_g_wlan_mutex;
extern pthread_mutex_t asd_g_bss_mutex;
extern asd_sta_flash_disconn *ASD_FDStas;

extern int vrrid;
extern int local;
extern int slotid;
extern char *ASD_ARP_GROUP[WLAN_NUM];
void asd_new_assoc_sta(struct asd_data *wasd, struct sta_info *sta,
			   int reassoc);
int asd_get_ip(const u8 *mac,u8 *in_addr);		//ht add,081030

void CWCaptrue(int n ,unsigned char *buffer);
int AsdStaInfoToWID(struct asd_data *wasd, const u8 *addr, Operate op);
int CWCreateThreadMutex(pthread_mutex_t *theMutex);
void CWDestroyThreadMutex(pthread_mutex_t  *theMutex);
void InitPath(unsigned int vrrid,char *buf);
void CWASDDbusPathInit();
int pkcs12_decryption(char * o_file, char * n_file, char * passwd);
int get_dir_wild_file_count(char *dir, char *wildfile);

ssize_t getline(char **lineptr, size_t *n, FILE *stream);
int read_ac_info(char *FILENAME,char *buff);
int parse_int_ID(char* str,unsigned int* ID);
int asd_setup_wpa(struct asd_data *wasd);
void asd_cleanup(struct asd_data *wasd);
void asd_notice_to_dhcp(struct asd_data *wasd, const u8 *addr,int op);
void get_sysruntime(time_t *sysruntime);//qiuchen add it 2012.10.31
void asd_pid_write_v2(char *name);
/* WEP/PSK associate auth  (SHARE:WEP) */
#define ASD_AUTH_TYPE_WEP_PSK(security_id)	\
		(NULL != ASD_SECURITY[(security_id)]		\
			&& (SHARED == ASD_SECURITY[(security_id)]->securityType))

/* EAP auth  (wpa_e:, wpa2_e:) */
#define ASD_AUTH_TYPE_EAP(security_id)	\
		(NULL != ASD_SECURITY[(security_id)]		\
			&& ((WPA_E == ASD_SECURITY[(security_id)]->securityType)	\
				|| (WPA2_E == ASD_SECURITY[(security_id)]->securityType)))
//qiuchen add it for master_bak radius 2012.12.11
#define	RADIUS_RES_FAIL_PERCENT	0.1
#define	RADIUS_RES_SUC_PERCENT	0.9
#define	RADIUS_ACCESS_TEST_INTERVAL	60
#define	RADIUS_SERVER_CHANGE_TIMER	600
#define	RADIUS_SERVER_REUSE_TIMER	3600
#define TEST_WINDOW_LEAST_SIZE	5
enum{
	RADIUS_SERVER_UNBINDED,
	RADIUS_SERVER_BINDED	
};
enum{
	RADIUS_AUTH_TEST,
	RADIUS_ACCT_TEST,
	RADIUS_BOTH_TEST
};
enum{
	RADIUS_MASTER,
	RADIUS_BAK,
	RADIUS_DISABLE,
	RADIUS_TEST_SUC,
	RADIUS_TEST_FAIL,
	RADIUS_HOLD_ON	//no need to change server
};
//end
//qiuchen add it for log information 2013.01.14
enum{
	OPERATE_SUCCESS,/*This only for open/shared */
	FLOW_BANLANCE,
	NUMBER_BANLANCE,
	AUTH_ALG_FAIL,
	AUTH_TRANSNUM_WRONG,
	STAMAC_BSSID,
	MAC_REJECTED,
	VLANID_INVALID,
	WPA_SM_FAILED,
	NO_RESOURCE,
	ASSO_BEFORE_AUTH,
	ASSO_PACKAGE_WRONG,
	UNKNOWN_SSID,
	WME_ELEM_INVALID,
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
	PSK_FAILED
};
struct r_sta_wlan_info{
 unsigned int roaming_sta_num;
 struct r_sta_info *r_sta_list;
};
struct asd_bss_summary_info{
	int ID;
	int local_bss_num;
	struct bss_summary_info *bss_list;
};
//end
#endif /* asd_H */
