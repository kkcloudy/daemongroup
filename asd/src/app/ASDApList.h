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
* AsdApList.h
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

#ifndef AP_LIST_H
#define AP_LIST_H

struct ap_info {
	/* Note: next/prev pointers are updated whenever a new beacon is
	 * received because these are used to find the least recently used
	 * entries. iter_next/iter_prev are updated only when adding new BSSes
	 * and when removing old ones. These should be used when iterating
	 * through the table in a manner that allows beacons to be received
	 * during the iteration. */
	struct ap_info *next; /* next entry in AP list */
	struct ap_info *prev; /* previous entry in AP list */
	struct ap_info *hnext; /* next entry in hash table list */
	struct ap_info *iter_next; /* next entry in AP iteration list */
	struct ap_info *iter_prev; /* previous entry in AP iteration list */
	u8 addr[6];
	u16 beacon_int;
	u16 capability;
	u8 supported_rates[WLAN_SUPP_RATES_MAX];
	u8 ssid[33];
	size_t ssid_len;
	int wpa;
	int erp; /* ERP Info or -1 if ERP info element not present */

	int phytype; /* .11a / .11b / .11g / Atheros Turbo */
	int channel;
	int datarate; /* in 100 kbps */
	int ssi_signal;

	unsigned int num_beacons; /* number of beacon frames received */
	time_t last_beacon;

	int already_seen; /* whether API call AP-NEW has already fetched
			   * information about this AP */
};

struct ieee802_11_elems;
struct asd_frame_info;

struct ap_info * ap_get_ap(struct asd_iface *iface, u8 *sta);
int ap_ap_for_each(struct asd_iface *iface,
		   int (*func)(struct ap_info *s, void *data), void *data);
void ap_list_process_beacon(struct asd_iface *iface,
			    struct ieee80211_mgmt *mgmt,
			    struct ieee802_11_elems *elems,
			    struct asd_frame_info *fi);
int ap_list_init(struct asd_iface *iface);
void ap_list_deinit(struct asd_iface *iface);
int ap_list_reconfig(struct asd_iface *iface,
		     struct asd_config *oldconf);

#endif /* AP_LIST_H */
