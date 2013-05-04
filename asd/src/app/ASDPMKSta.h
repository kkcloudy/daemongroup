#ifndef PMK_STA_H
#define PMK_STA_H
struct PMK_STAINFO * pmk_ap_get_sta(WID_WLAN *WLAN, const u8 *sta);

//static void pmk_ap_sta_list_del(WID_WLAN *WLAN, struct PMK_STAINFO *sta);

void pmk_ap_sta_hash_add(WID_WLAN *WLAN, struct PMK_STAINFO *sta);

//static void pmk_ap_sta_hash_del(WID_WLAN *WLAN, struct PMK_STAINFO *sta);

void pmk_ap_free_sta(WID_WLAN *WLAN, struct PMK_STAINFO *sta);

struct PMK_STAINFO * pmk_ap_sta_add(WID_WLAN *WLAN, const u8 *addr);

struct rsn_pmksa_cache_entry * wlan_pmksa_cache_get(unsigned char WLANID, unsigned int BSSIndex,
					       const u8 *spa, const u8 *aa, const u8 *pmkid);
void pmk_ap_free_sta_in_prebss(unsigned int BSSIndex, unsigned char *addr);
void pmk_wlan_del_all_sta(unsigned char WLANID);
int pmk_del_bssindex(unsigned int BSSIndex, struct PMK_STAINFO *sta);
int pmk_add_bssindex(unsigned int BSSIndex, struct PMK_STAINFO *sta);
#endif
