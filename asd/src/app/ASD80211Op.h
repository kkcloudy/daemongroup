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
* Asd80211Op.h
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

#ifndef IEEE802_11_H
#define IEEE802_11_H

#include "ieee802_11_defs.h"
#include "wcpss/waw.h"
/* Parsed Information Elements */
struct ieee802_11_elems {
	u8 *ssid;
	u8 ssid_len;
	u8 *supp_rates;
	u8 supp_rates_len;
	u8 *fh_params;
	u8 fh_params_len;
	u8 *ds_params;
	u8 ds_params_len;
	u8 *cf_params;
	u8 cf_params_len;
	u8 *tim;
	u8 tim_len;
	u8 *ibss_params;
	u8 ibss_params_len;
	u8 *challenge;
	u8 challenge_len;
	u8 *erp_info;
	u8 erp_info_len;
	u8 *ext_supp_rates;
	u8 ext_supp_rates_len;
	u8 *wpa_ie;
	u8 wpa_ie_len;
	u8 *rsn_ie;
	u8 rsn_ie_len;
	u8 *wme;
	u8 wme_len;
	u8 *wme_tspec;
	u8 wme_tspec_len;
	u8 *power_cap;
	u8 power_cap_len;
	u8 *supp_channels;
	u8 supp_channels_len;
	u8 *mdie;
	u8 mdie_len;
	u8 *ftie;
	u8 ftie_len;
	u8 *timeout_int;
	u8 timeout_int_len;
	u8 *ht_capabilities;
	u8 ht_capabilities_len;
	u8 *ht_operation;
	u8 ht_operation_len;
	u8 *vendor_ht_cap;
	u8 vendor_ht_cap_len;
};

typedef enum { ParseOK = 0, ParseUnknown = 1, ParseFailed = -1 } ParseRes;


struct asd_frame_info {
	u32 phytype;
	u32 channel;
	u32 datarate;
	u32 ssi_signal;

	unsigned int passive_scan:1;
};

struct asd_data;
struct sta_info;

void ieee802_11_send_deauth(struct asd_data *wasd, u8 *addr, u16 reason);
void ieee802_11_mgmt(struct asd_data *wasd, u8 *buf, size_t len,
		     u16 stype, struct asd_frame_info *fi);
void ieee802_11_mgmt_cb(struct asd_data *wasd, u8 *buf, size_t len,
			u16 stype, int ok);
ParseRes ieee802_11_parse_elems(struct asd_data *wasd, u8 *start,
				size_t len,
				struct ieee802_11_elems *elems,
				int show_errors);
void ieee802_11_print_ssid(char *buf, const u8 *ssid, u8 len);
void ieee80211_michael_mic_failure(struct asd_data *wasd, const u8 *addr,
				   int local);
int ieee802_11_get_mib(struct asd_data *wasd, char *buf, size_t buflen);
int ieee802_11_get_mib_sta(struct asd_data *wasd, struct sta_info *sta,
			   char *buf, size_t buflen);
u16 asd_own_capab_info(struct asd_data *wasd, struct sta_info *sta,
			   int probe);
u8 * asd_eid_supp_rates(struct asd_data *wasd, u8 *eid);
u8 * asd_eid_ext_supp_rates(struct asd_data *wasd, u8 *eid);
unsigned int 
is_wtp_apply_wlan(unsigned int WTPID,unsigned char wlanid);
unsigned int 
get_sta_num_by_wtpid(unsigned int WTPID);

int Assmble_WSM_msg(DataMsg *aMsg, struct asd_data *wasd, const void *msg, size_t len, int type);

void reset_table_item(struct auth_sta_info_t *wapi_sta_info);

void log_parse_reason(int reasons,char *reas);
void log_parse_reason_80211(int reasons,char *reas);

#endif /* IEEE802_11_H */
