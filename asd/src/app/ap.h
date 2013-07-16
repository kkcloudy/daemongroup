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
* ap.h
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

#ifndef AP_H
#define AP_H

#include "ip_addr.h"
#ifdef ASD_IEEE80211N
#include "ieee802_11_defs.h"
#endif /* ASD_IEEE80211N */


/* STA flags */
#define WLAN_STA_AUTH BIT(0)
#define WLAN_STA_ASSOC BIT(1)
#define WLAN_STA_PS BIT(2)
#define WLAN_STA_TIM BIT(3)
#define WLAN_STA_PERM BIT(4)
#define WLAN_STA_AUTHORIZED BIT(5)
#define WLAN_STA_PENDING_POLL BIT(6) /* pending activity poll not ACKed */
#define WLAN_STA_SHORT_PREAMBLE BIT(7)
#define WLAN_STA_PREAUTH BIT(8)
#define WLAN_STA_WME BIT(9)
#define WLAN_STA_MFP BIT(10)
#define WLAN_STA_HT BIT(11)
#define WLAN_STA_WPS BIT(12)
#define WLAN_STA_MAYBE_WPS BIT(13)
#define WLAN_STA_DEL BIT(27)  /*ht add,set when ap told sta is leave,011027*/
#define WLAN_STA_FREE BIT(29)  //ht add,090219
#define WLAN_STA_AUTH_ACK BIT(30)  //ht add,090219
#define WLAN_STA_NONERP BIT(31)

#define WLAN_STA_ROAMING BIT(28) //zhanglei add

/* Maximum number of supported rates (from both Supported Rates and Extended
 * Supported Rates IEs). */
#define WLAN_SUPP_RATES_MAX 32


struct sta_info {
	struct sta_info *next; /* next entry in sta list */
	struct sta_info *hnext; /* next entry in hash table list */
	struct sta_info *g_hnext;/*global hash next sta*/
	u8 addr[6];
	u16 aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	u32 flags;
	char *in_addr;	//ht add,081025
	struct in_addr ip_addr;
	u64 rxbytes;
	u64 txbytes;
	u64 pre_rxbytes;
	u64 pre_txbytes;
	u64 rxdatapackets;  /*ap report*/ 
	u64 txdatapackets;  /*ap report*/ 
	u64 rxpackets;
	u64 txpackets;
	u64 pre_rxpackets;
	u64 pre_txpackets;
	u64 rxframepackets;  /*ap report*/ 
	u64 txframepackets;  /*ap report*/ 
	u64 retrybytes;
	u64 retrypackets;
	u64 errpackets;
	u16 capability;
	u16 listen_interval; /* or beacon_int for APs */
	u8 supported_rates[WLAN_SUPP_RATES_MAX];
	int supported_rates_len;
	u64 rxbytes_old;
	u64 txbytes_old;  //xm add  09/02/16
	u64 rr;  
	u64 tr;   //receive rate    transmit rate   byte
	u64 tp;   //throughput
	u32 snr;
	
	unsigned int nonerp_set:1;
	unsigned int no_short_slot_time_set:1;
	unsigned int no_short_preamble_set:1;
	unsigned int no_ht_gf_set:1;
	unsigned int no_ht_set:1;
	unsigned int ht_20mhz_set:1;
	int portal_auth_success;		   //weichao add 2011.10.20

	u16 auth_alg;
	u8 previous_ap[6];

	enum {
		STA_NULLFUNC = 0, STA_DISASSOC, STA_DEAUTH, STA_REMOVE
	} timeout_next;

	/* IEEE 802.1X related data */
	struct eapol_state_machine *eapol_sm;

	/* IEEE 802.11f (IAPP) related data */
	struct ieee80211_mgmt *last_assoc_req;

	u32 acct_session_id_hi;
	u32 acct_session_id_lo;
	time_t acct_session_start;
	int acct_session_started;
	int acct_terminate_cause; /* Acct-Terminate-Cause */
	int acct_interim_interval; /* Acct-Interim-Interval */

	unsigned long last_rx_bytes;
	unsigned long last_tx_bytes;
	u32 acct_input_gigawords; /* Acct-Input-Gigawords */
	u32 acct_output_gigawords; /* Acct-Output-Gigawords */

	u8 *challenge; /* IEEE 802.11 Shared Key Authentication Challenge */

	struct wpa_state_machine *wpa_sm;
	struct rsn_preauth_interface *preauth_iface;

	struct asd_ssid *ssid; /* SSID selection based on (Re)AssocReq */
	struct asd_ssid *ssid_probe; /* SSID selection based on ProbeReq */
	int alive_flag;						//weichao add 2011.09.23
	int alive_total;						//weichao add 2011.09.23

	int vlan_id;
	unsigned int	security_type;		//mahz add 2011.3.1
	unsigned char	sta_assoc_auth_flag;

#ifdef ASD_IEEE80211N
		struct ht_cap_ie ht_capabilities; /* IEEE 802.11n capabilities */
#endif /* ASD_IEEE80211N */
	
	//char *add_time;
	time_t *add_time;
	char PreAuth_BSSID[6];
	unsigned int PreAuth_BSSIndex;

	/*sta info single intensity etc.*/
	unsigned char mode;  //11b-0x01,11a-0x02,11g-0x04,11n-0x08,
	unsigned char channel;
	unsigned char rssi;
	unsigned short nRate;   /*sta rx rate*/
	unsigned short txRate;
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned char info_channel;
	struct auth_sta_info_t wapi_sta_info;

	unsigned char	vip_flag;	/*bit0 for up flag,bit1 for down flag*/
	unsigned int	sta_traffic_limit;		/*上行*//*指station 上传到AP的带宽限制 */
	unsigned int	sta_send_traffic_limit; /*下行*//*指station 从AP下载的带宽限制 */
	unsigned int BssIndex;
	unsigned int PreBssIndex;
	unsigned char PreBSSID[MAC_LEN];
	unsigned int PreACIP;
	unsigned int	ipaddr;
	unsigned int	gifidx;
	char	arpifname[16];
	 time_t add_time_sysruntime;//qiuchen add it
	 time_t sta_online_time;//qiuchen add it
	 
	 unsigned int MAXofRateset;	 /* 终端与AP刚关联时根据双方能力而协商的无线速率集中的最高速率 */
	 struct WID_WTP_STA_INFO wtp_sta_statistics_info;

	 enum{
		 ASD_ROAM_NONE = 0,
		 ASD_ROAM_2 = 1,//L2 roaming
		 ASD_ROAM_3 = 2	//L3 roaming
	 }rflag;
	 unsigned char logflag;//logflag = 1 henan mobile roaming log had printed!	0-not
	 unsigned char preAPID;
	 struct asd_data *wasd;
	 unsigned char BSSID[MAC_LEN];
};
struct sta_acct_info{	
	u8 acct_id[ACCT_ID_LEN+1];
	u8 addr[6];	
	struct sta_acct_info *next;
};
struct asd_ip_secret
{
	u32 ip;
	u8 *shared_secret;
	size_t  shared_secret_len;
	unsigned int slot_value;
	unsigned int inst_value;
	struct asd_ip_secret *next;
};
/* Maximum number of AIDs to use for STAs; must be 2007 or lower
 * (8802.11 limitation) */
#define MAX_AID_TABLE_SIZE 128

#define STA_HASH_SIZE 256
#define STA_HASH(sta) (sta[5])
#define STA_GLOBAL_HASH(sta) (sta[4]<<8 |sta[5])
#define SECRET_IP_HASH(ip) (ip >> 24)
/* Default value for maximum station inactivity. After AP_MAX_INACTIVITY has
 * passed since last received frame from the station, a nullfunc data frame is
 * sent to the station. If this frame is not acknowledged and no other frames
 * have been received, the station will be disassociated after
 * AP_DISASSOC_DELAY seconds. Similarily, the station will be deauthenticated
 * after AP_DEAUTH_DELAY seconds has passed after disassociation. */
#define AP_MAX_INACTIVITY (5 * 60)
#define AP_DISASSOC_DELAY (1)
#define AP_DEAUTH_DELAY (1)
/* Number of seconds to keep STA entry with Authenticated flag after it has
 * been disassociated. */
#define AP_MAX_INACTIVITY_AFTER_DISASSOC (1 * 30)
/* Number of seconds to keep STA entry after it has been deauthenticated. */
#define AP_MAX_INACTIVITY_AFTER_DEAUTH (1 * 5)

#endif /* AP_H */
