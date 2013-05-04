#ifndef PREAUTH_BSS_H
#define PREAUTH_BSS_H
struct PreAuth_BSSINFO * PreAuth_wlan_get_bss(WID_WLAN *WLAN, const u8 *addr);
void PreAuth_wlan_free_bss(WID_WLAN *WLAN, struct PreAuth_BSSINFO *bss);
struct PreAuth_BSSINFO* PreAuth_wlan_bss_add(WID_WLAN *WLAN, const u8 *addr, unsigned int BSSIndex);
void PreAuth_wlan_del_all_bss(unsigned char WLANID);
void rsn_preauth_receive_thinap(void *ctx, const u8 *src_addr,const u8 *des_addr,
				const u8 *buf, size_t len);
int asd_send_mgmt_frame_w(void *priv, const void *msg, size_t len,
				  int flags);
int rsn_preauth_send_thinap(void *priv, const u8 *addr, const u8 *data,
			     size_t data_len, int encrypt, const u8 *own_addr, unsigned int PreAuth_BSSIndex);
#endif
