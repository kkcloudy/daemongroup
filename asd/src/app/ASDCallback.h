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
* ASDCallback.h
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
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"

#ifndef DRIVER_H
#define DRIVER_H

enum asd_driver_if_type {
	asd_IF_VLAN, asd_IF_WDS
};

struct wpa_driver_ops {
	const char *name;		/* as appears in the config file */

	void * (*init)(struct asd_data *wasd);
	void * (*init_bssid)(struct asd_data *wasd, const u8 *bssid);
	void (*deinit)(void *priv);

	int (*wireless_event_init)(void *priv);
	void (*wireless_event_deinit)(void *priv);

	/**
	 * set_8021x - enable/disable IEEE 802.1X support
	 * @ifname: Interface name (for multi-SSID/VLAN support)
	 * @priv: driver private data
	 * @enabled: 1 = enable, 0 = disable
	 *
	 * Returns: 0 on success, -1 on failure
	 *
	 * Configure the kernel driver to enable/disable 802.1X support.
	 * This may be an empty function if 802.1X support is always enabled.
	 */
	int (*set_ieee8021x)(const char *ifname, void *priv, int enabled);

	/**
	 * set_privacy - enable/disable privacy
	 * @priv: driver private data
	 * @enabled: 1 = privacy enabled, 0 = disabled
	 *
	 * Return: 0 on success, -1 on failure
	 *
	 * Configure privacy.
	 */
	int (*set_privacy)(const char *ifname, void *priv, int enabled);

	int (*set_encryption)(const char *ifname, void *priv, const char *alg,
			      const u8 *addr, int idx,
			      const u8 *key, size_t key_len, int txkey);
	int (*get_seqnum)(const char *ifname, void *priv, const u8 *addr,
			  int idx, u8 *seq);
	int (*get_seqnum_igtk)(const char *ifname, void *priv, const u8 *addr,
			       int idx, u8 *seq);
	int (*flush)(void *priv);
	int (*set_generic_elem)(const char *ifname, void *priv, const u8 *elem,
				size_t elem_len);

	int (*read_sta_data)(void *priv, struct asd_sta_driver_data *data,
			     const u8 *addr);
	int (*send_eapol)(void *priv, const u8 *addr, const u8 *data,
			  size_t data_len, int encrypt, const u8 *own_addr);
	int (*sta_deauth)(void *priv, const u8 *addr, int reason);
	int (*sta_disassoc)(void *priv, const u8 *addr, int reason);
	int (*sta_remove)(void *priv, const u8 *addr);
	int (*get_ssid)(const char *ifname, void *priv, u8 *buf, int len);
	int (*set_ssid)(const char *ifname, void *priv, const u8 *buf,
			int len);
	int (*set_countermeasures)(void *priv, int enabled);
	int (*send_mgmt_frame)(void *priv, const void *msg, size_t len,
			       int flags);
	int (*set_assoc_ap)(void *priv, const u8 *addr);
	int (*sta_add)(const char *ifname, void *priv, const u8 *addr, u16 aid,
		       u16 capability, u8 *supp_rates, size_t supp_rates_len,
		       int flags);
	int (*get_inact_sec)(void *priv, const u8 *addr);
	int (*sta_clear_stats)(void *priv, const u8 *addr);

	int (*set_freq)(void *priv, int mode, int freq);
	int (*set_rts)(void *priv, int rts);
	int (*get_rts)(void *priv, int *rts);
	int (*set_frag)(void *priv, int frag);
	int (*get_frag)(void *priv, int *frag);
	int (*set_retry)(void *priv, int short_retry, int long_retry);
	int (*get_retry)(void *priv, int *short_retry, int *long_retry);

	int (*sta_set_flags)(void *priv, const u8 *addr,
			     int total_flags, int flags_or, int flags_and);
	int (*set_rate_sets)(void *priv, int *supp_rates, int *basic_rates,
			     int mode);
	int (*set_channel_flag)(void *priv, int mode, int chan, int flag,
				unsigned char power_level,
				unsigned char antenna_max);
	int (*set_regulatory_domain)(void *priv, unsigned int rd);
	int (*set_country)(void *priv, const char *country);
	int (*set_ieee80211d)(void *priv, int enabled);
	int (*set_beacon)(const char *ifname, void *priv,
			  u8 *head, size_t head_len,
			  u8 *tail, size_t tail_len);

	/* Configure internal bridge:
	 * 0 = disabled, i.e., client separation is enabled (no bridging of
	 *     packets between associated STAs
	 * 1 = enabled, i.e., bridge packets between associated STAs (default)
	 */
	int (*set_internal_bridge)(void *priv, int value);
	int (*set_beacon_int)(void *priv, int value);
	int (*set_dtim_period)(const char *ifname, void *priv, int value);
	/* Configure broadcast SSID mode:
	 * 0 = include SSID in Beacon frames and reply to Probe Request frames
	 *     that use broadcast SSID
	 * 1 = hide SSID from Beacon frames and ignore Probe Request frames for
	 *     broadcast SSID
	 */
	int (*set_broadcast_ssid)(void *priv, int value);
	int (*set_cts_protect)(void *priv, int value);
	int (*set_key_tx_rx_threshold)(void *priv, int value);
	int (*set_preamble)(void *priv, int value);
	int (*set_short_slot_time)(void *priv, int value);
	int (*set_tx_queue_params)(void *priv, int queue, int aifs, int cw_min,
				   int cw_max, int burst_time);
	int (*bss_add)(void *priv, const char *ifname, const u8 *bssid);
	int (*bss_remove)(void *priv, const char *ifname);
	int (*valid_bss_mask)(void *priv, const u8 *addr, const u8 *mask);
	int (*passive_scan)(void *priv, int now, int our_mode_only,
			    int interval, int _listen, int *channel,
			    int *last_rx);
	struct asd_hw_modes * (*get_hw_feature_data)(void *priv,
							 u16 *num_modes,
							 u16 *flags);
	int (*if_add)(const char *iface, void *priv,
		      enum asd_driver_if_type type, char *ifname,
		      const u8 *addr);
	int (*if_update)(void *priv, enum asd_driver_if_type type,
			 char *ifname, const u8 *addr);
	int (*if_remove)(void *priv, enum asd_driver_if_type type,
			 const char *ifname, const u8 *addr);
	int (*set_sta_vlan)(void *priv, const u8 *addr, const char *ifname,
			    int vlan_id);
	/**
	 * commit - Optional commit changes handler
	 * @priv: driver private data
	 * Returns: 0 on success, -1 on failure
	 *
	 * This optional handler function can be registered if the driver
	 * interface implementation needs to commit changes (e.g., by setting
	 * network interface up) at the end of initial configuration. If set,
	 * this handler will be called after initial setup has been completed.
	 */
	int (*commit)(void *priv);

	int (*send_ether)(void *priv, const u8 *dst, const u8 *src, u16 proto,
			  const u8 *data, size_t data_len);
};

static inline void *
asd_driver_init(struct asd_data *wasd)
{
	if (wasd->driver == NULL || wasd->driver->init == NULL)
		return NULL;
	return wasd->driver->init(wasd);
}

static inline void *
asd_driver_init_bssid(struct asd_data *wasd, const u8 *bssid)
{
	if (wasd->driver == NULL || wasd->driver->init_bssid == NULL)
		return NULL;
	return wasd->driver->init_bssid(wasd, bssid);
}

static inline void
asd_driver_deinit(struct asd_data *wasd)
{
	if (wasd->driver == NULL || wasd->driver->deinit == NULL)
		return;
	wasd->driver->deinit(wasd->drv_priv);
}

static inline int
asd_wireless_event_init(struct asd_data *wasd)
{
	if (wasd->driver == NULL ||
	    wasd->driver->wireless_event_init == NULL)
		return 0;
	return wasd->driver->wireless_event_init(wasd->drv_priv);
}

static inline void
asd_wireless_event_deinit(struct asd_data *wasd)
{
	if (wasd->driver == NULL ||
	    wasd->driver->wireless_event_deinit == NULL)
		return;
	wasd->driver->wireless_event_deinit(wasd->drv_priv);
}

static inline int
asd_set_ieee8021x(const char *ifname, struct asd_data *wasd,
		      int enabled)
{
	if (wasd->driver == NULL || wasd->driver->set_ieee8021x == NULL)
		return 0;
	return wasd->driver->set_ieee8021x(ifname, wasd->drv_priv, enabled);
}

static inline int
asd_set_privacy(struct asd_data *wasd, int enabled)
{
	if (wasd->driver == NULL || wasd->driver->set_privacy == NULL)
		return 0;
	return wasd->driver->set_privacy(wasd->conf->iface, wasd->drv_priv,
					 enabled);
}

static inline int
asd_set_encryption(const char *ifname, struct asd_data *wasd,
		       const char *alg, const u8 *addr, int idx,
		       u8 *key, size_t key_len, int txkey)
{
	if (wasd->driver == NULL || wasd->driver->set_encryption == NULL)
		return 0;
//	printf("asd_set_encryption\n");
	return wasd->driver->set_encryption(ifname, wasd, alg, addr,
					    idx, key, key_len, txkey);
//	return wasd->driver->set_encryption(ifname, wasd->drv_priv, alg, addr,
//					    idx, key, key_len, txkey);
}

static inline int
asd_get_seqnum(const char *ifname, struct asd_data *wasd,
		   const u8 *addr, int idx, u8 *seq)
{
	if (wasd->driver == NULL || wasd->driver->get_seqnum == NULL)
		return 0;
	return wasd->driver->get_seqnum(ifname, wasd->drv_priv, addr, idx,
					seq);
}

static inline int
asd_get_seqnum_igtk(const char *ifname, struct asd_data *wasd,
			const u8 *addr, int idx, u8 *seq)
{
	if (wasd->driver == NULL || wasd->driver->get_seqnum_igtk == NULL)
		return -1;
	return wasd->driver->get_seqnum_igtk(ifname, wasd->drv_priv, addr, idx,
					     seq);
}

static inline int
asd_flush(struct asd_data *wasd)
{
	if (wasd->driver == NULL || wasd->driver->flush == NULL)
		return 0;
	return wasd->driver->flush(wasd->drv_priv);
}

static inline int
asd_set_generic_elem(struct asd_data *wasd, const u8 *elem,
			 size_t elem_len)
{
	if (wasd->driver == NULL || wasd->driver->set_generic_elem == NULL)
		return 0;
	return wasd->driver->set_generic_elem(wasd->conf->iface,
					      wasd->drv_priv, elem, elem_len);
}

static inline int
asd_read_sta_data(struct asd_data *wasd,
		      struct asd_sta_driver_data *data, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->read_sta_data == NULL)
		return -1;
	return wasd->driver->read_sta_data(wasd->drv_priv, data, addr);
}

static inline int
asd_send_eapol(struct asd_data *wasd, const u8 *addr, const u8 *data,
		   size_t data_len, int encrypt)
{
	if (wasd->driver == NULL || wasd->driver->send_eapol == NULL)
		return 0;
/*	return wasd->driver->send_eapol(wasd->drv_priv, addr, data, data_len,
					encrypt, wasd->own_addr);
*/	
	return wasd->driver->send_eapol(wasd, addr, data, data_len,
					encrypt, wasd->own_addr);//zhanglei change wasd->drv_priv to wasd
}

static inline int
asd_sta_deauth(struct asd_data *wasd, const u8 *addr, int reason)
{
	if (wasd->driver == NULL || wasd->driver->sta_deauth == NULL)
		return 0;
//	return wasd->driver->sta_deauth(wasd->drv_priv, addr, reason);
	return wasd->driver->sta_deauth(wasd, addr, reason);//zhanglei change
}

static inline int
asd_sta_disassoc(struct asd_data *wasd, const u8 *addr, int reason)
{
	if (wasd->driver == NULL || wasd->driver->sta_disassoc == NULL)
		return 0;
//	return wasd->driver->sta_disassoc(wasd->drv_priv, addr, reason);	
	return wasd->driver->sta_disassoc(wasd, addr, reason);
}

static inline int
asd_sta_remove(struct asd_data *wasd, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->sta_remove == NULL)
		return 0;
	//return wasd->driver->sta_remove(wasd->drv_priv, addr);	
	return wasd->driver->sta_remove(wasd, addr);
}

static inline int
asd_get_ssid(struct asd_data *wasd, u8 *buf, size_t len)
{
	if (wasd->driver == NULL || wasd->driver->get_ssid == NULL)
		return 0;
	return wasd->driver->get_ssid(wasd->conf->iface, wasd->drv_priv, buf,
				      len);
}

static inline int
asd_set_ssid(struct asd_data *wasd, const u8 *buf, size_t len)
{
	if (wasd->driver == NULL || wasd->driver->set_ssid == NULL)
		return 0;
	return wasd->driver->set_ssid(wasd->conf->iface, wasd->drv_priv, buf,
				      len);
}

static inline int
asd_send_mgmt_frame(struct asd_data *wasd, const void *msg, size_t len,
			int flags)
{
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "asd_send_mgmt_frame\n");
	if (wasd->driver == NULL || wasd->driver->send_mgmt_frame == NULL)
		return 0;
	wasd->info->tx_mgmt_pkts++;
	return wasd->driver->send_mgmt_frame(wasd->drv_priv, msg, len, flags);
}

static inline int
asd_set_assoc_ap(struct asd_data *wasd, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->set_assoc_ap == NULL)
		return 0;
	return wasd->driver->set_assoc_ap(wasd->drv_priv, addr);
}

static inline int
asd_set_countermeasures(struct asd_data *wasd, int enabled)
{
	if (wasd->driver == NULL || wasd->driver->set_countermeasures == NULL)
		return 0;
	return wasd->driver->set_countermeasures(wasd->drv_priv, enabled);
}

static inline int
asd_sta_add(const char *ifname, struct asd_data *wasd, const u8 *addr,
		u16 aid, u16 capability, u8 *supp_rates, size_t supp_rates_len,
		int flags)
{
	if (wasd->driver == NULL || wasd->driver->sta_add == NULL)
		return 0;
	//return wasd->driver->sta_add(ifname, wasd->drv_priv, addr, aid,
	//			     capability, supp_rates, supp_rates_len,
	//			     flags);	
	return wasd->driver->sta_add(ifname, wasd, addr, aid,
				     capability, supp_rates, supp_rates_len,
				     flags);
}

static inline int
asd_get_inact_sec(struct asd_data *wasd, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->get_inact_sec == NULL)
		return 0;
	return wasd->driver->get_inact_sec(wasd->drv_priv, addr);
}

static inline int
asd_set_freq(struct asd_data *wasd, int mode, int freq)
{
	if (wasd->driver == NULL || wasd->driver->set_freq == NULL)
		return 0;
	return wasd->driver->set_freq(wasd->drv_priv, mode, freq);
}

static inline int
asd_set_rts(struct asd_data *wasd, int rts)
{
	if (wasd->driver == NULL || wasd->driver->set_rts == NULL)
		return 0;
	return wasd->driver->set_rts(wasd->drv_priv, rts);
}

static inline int
asd_get_rts(struct asd_data *wasd, int *rts)
{
	if (wasd->driver == NULL || wasd->driver->get_rts == NULL)
		return 0;
	return wasd->driver->get_rts(wasd->drv_priv, rts);
}

static inline int
asd_set_frag(struct asd_data *wasd, int frag)
{
	if (wasd->driver == NULL || wasd->driver->set_frag == NULL)
		return 0;
	return wasd->driver->set_frag(wasd->drv_priv, frag);
}

static inline int
asd_get_frag(struct asd_data *wasd, int *frag)
{
	if (wasd->driver == NULL || wasd->driver->get_frag == NULL)
		return 0;
	return wasd->driver->get_frag(wasd->drv_priv, frag);
}

static inline int
asd_set_retry(struct asd_data *wasd, int short_retry, int long_retry)
{
	if (wasd->driver == NULL || wasd->driver->set_retry == NULL)
		return 0;
	return wasd->driver->set_retry(wasd->drv_priv, short_retry,
				       long_retry);
}

static inline int
asd_get_retry(struct asd_data *wasd, int *short_retry, int *long_retry)
{
	if (wasd->driver == NULL || wasd->driver->get_retry == NULL)
		return 0;
	return wasd->driver->get_retry(wasd->drv_priv, short_retry,
				       long_retry);
}

static inline int
asd_sta_set_flags(struct asd_data *wasd, u8 *addr,
		      int total_flags, int flags_or, int flags_and)
{
	if (wasd->driver == NULL || wasd->driver->sta_set_flags == NULL)
		return 0;
	//return wasd->driver->sta_set_flags(wasd->drv_priv, addr, total_flags,
	//				   flags_or, flags_and);	
	return wasd->driver->sta_set_flags(wasd, addr, total_flags,
					   flags_or, flags_and);
}

static inline int
asd_set_rate_sets(struct asd_data *wasd, int *supp_rates,
		      int *basic_rates, int mode)
{
	if (wasd->driver == NULL || wasd->driver->set_rate_sets == NULL)
		return 0;
	return wasd->driver->set_rate_sets(wasd->drv_priv, supp_rates,
					   basic_rates, mode);
}

static inline int
asd_set_channel_flag(struct asd_data *wasd, int mode, int chan,
			 int flag, unsigned char power_level,
			 unsigned char antenna_max)
{
	if (wasd->driver == NULL || wasd->driver->set_channel_flag == NULL)
		return 0;
	return wasd->driver->set_channel_flag(wasd->drv_priv, mode, chan, flag,
					      power_level, antenna_max);
}

static inline int
asd_set_regulatory_domain(struct asd_data *wasd, unsigned int rd)
{
	if (wasd->driver == NULL ||
	    wasd->driver->set_regulatory_domain == NULL)
		return 0;
	return wasd->driver->set_regulatory_domain(wasd->drv_priv, rd);
}

static inline int
asd_set_country(struct asd_data *wasd, const char *country)
{
	if (wasd->driver == NULL ||
	    wasd->driver->set_country == NULL)
		return 0;
	return wasd->driver->set_country(wasd->drv_priv, country);
}

static inline int
asd_set_ieee80211d(struct asd_data *wasd, int enabled)
{
	if (wasd->driver == NULL ||
	    wasd->driver->set_ieee80211d == NULL)
		return 0;
	return wasd->driver->set_ieee80211d(wasd->drv_priv, enabled);
}

static inline int
asd_sta_clear_stats(struct asd_data *wasd, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->sta_clear_stats == NULL)
		return 0;
	return wasd->driver->sta_clear_stats(wasd->drv_priv, addr);
}

static inline int
asd_set_beacon(const char *ifname, struct asd_data *wasd,
		   u8 *head, size_t head_len,
		   u8 *tail, size_t tail_len)
{
	if (wasd->driver == NULL || wasd->driver->set_beacon == NULL)
		return 0;
	return wasd->driver->set_beacon(ifname, wasd->drv_priv, head, head_len,
					tail, tail_len);
}

static inline int
asd_set_internal_bridge(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_internal_bridge == NULL)
		return 0;
	return wasd->driver->set_internal_bridge(wasd->drv_priv, value);
}

static inline int
asd_set_beacon_int(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_beacon_int == NULL)
		return 0;
	return wasd->driver->set_beacon_int(wasd->drv_priv, value);
}

static inline int
asd_set_dtim_period(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_dtim_period == NULL)
		return 0;
	return wasd->driver->set_dtim_period(wasd->conf->iface, wasd->drv_priv,
					     value);
}

static inline int
asd_set_broadcast_ssid(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_broadcast_ssid == NULL)
		return 0;
	return wasd->driver->set_broadcast_ssid(wasd->drv_priv, value);
}

static inline int
asd_set_cts_protect(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_cts_protect == NULL)
		return 0;
	return wasd->driver->set_cts_protect(wasd->drv_priv, value);
}

static inline int
asd_set_key_tx_rx_threshold(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL ||
	    wasd->driver->set_key_tx_rx_threshold == NULL)
		return 0;
	return wasd->driver->set_key_tx_rx_threshold(wasd->drv_priv, value);
}

static inline int
asd_set_preamble(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_preamble == NULL)
		return 0;
	return wasd->driver->set_preamble(wasd->drv_priv, value);
}

static inline int
asd_set_short_slot_time(struct asd_data *wasd, int value)
{
	if (wasd->driver == NULL || wasd->driver->set_short_slot_time == NULL)
		return 0;
	return wasd->driver->set_short_slot_time(wasd->drv_priv, value);
}

static inline int
asd_set_tx_queue_params(struct asd_data *wasd, int queue, int aifs,
			    int cw_min, int cw_max, int burst_time)
{
	if (wasd->driver == NULL || wasd->driver->set_tx_queue_params == NULL)
		return 0;
	return wasd->driver->set_tx_queue_params(wasd->drv_priv, queue, aifs,
						 cw_min, cw_max, burst_time);
}

static inline int
asd_bss_add(struct asd_data *wasd, const char *ifname, const u8 *bssid)
{
	if (wasd->driver == NULL || wasd->driver->bss_add == NULL)
		return 0;
	return wasd->driver->bss_add(wasd->drv_priv, ifname, bssid);
}

static inline int
asd_bss_remove(struct asd_data *wasd, const char *ifname)
{
	if (wasd->driver == NULL || wasd->driver->bss_remove == NULL)
		return 0;
	return wasd->driver->bss_remove(wasd->drv_priv, ifname);
}

static inline int
asd_valid_bss_mask(struct asd_data *wasd, const u8 *addr,
		       const u8 *mask)
{
	if (wasd->driver == NULL || wasd->driver->valid_bss_mask == NULL)
		return 1;
	return wasd->driver->valid_bss_mask(wasd->drv_priv, addr, mask);
}

static inline int
asd_if_add(struct asd_data *wasd, enum asd_driver_if_type type,
	       char *ifname, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->if_add == NULL)
		return -1;
	return wasd->driver->if_add(wasd->conf->iface, wasd->drv_priv, type,
				    ifname, addr);
}

static inline int
asd_if_update(struct asd_data *wasd, enum asd_driver_if_type type,
		  char *ifname, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->if_update == NULL)
		return -1;
	return wasd->driver->if_update(wasd->drv_priv, type, ifname, addr);
}

static inline int
asd_if_remove(struct asd_data *wasd, enum asd_driver_if_type type,
		  char *ifname, const u8 *addr)
{
	if (wasd->driver == NULL || wasd->driver->if_remove == NULL)
		return -1;
	return wasd->driver->if_remove(wasd->drv_priv, type, ifname, addr);
}

static inline int
asd_passive_scan(struct asd_data *wasd, int now, int our_mode_only,
		     int interval, int _listen, int *channel,
		     int *last_rx)
{
	if (wasd->driver == NULL || wasd->driver->passive_scan == NULL)
		return -1;
	return wasd->driver->passive_scan(wasd->drv_priv, now, our_mode_only,
					  interval, _listen, channel, last_rx);
}

static inline struct asd_hw_modes *
asd_get_hw_feature_data(struct asd_data *wasd, u16 *num_modes,
			    u16 *flags)
{
	if (wasd->driver == NULL || wasd->driver->get_hw_feature_data == NULL)
		return NULL;
	return wasd->driver->get_hw_feature_data(wasd->drv_priv, num_modes,
						 flags);
}

static inline int
asd_set_sta_vlan(const char *ifname, struct asd_data *wasd,
		     const u8 *addr, int vlan_id)
{
	if (wasd->driver == NULL || wasd->driver->set_sta_vlan == NULL)
		return 0;
	return wasd->driver->set_sta_vlan(wasd->drv_priv, addr, ifname, vlan_id);
}

static inline int
asd_driver_commit(struct asd_data *wasd)
{
	if (wasd->driver == NULL || wasd->driver->commit == NULL)
		return 0;
	return wasd->driver->commit(wasd->drv_priv);
}


extern struct wpa_driver_ops *asd_drivers[];

#endif /* DRIVER_H */
