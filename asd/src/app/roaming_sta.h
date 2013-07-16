#ifndef ROAMING_STA_H
#define ROAMING_STA_H
struct ROAMING_STAINFO * roaming_get_sta(WID_WLAN *WLAN, const u8 *sta);
void roaming_sta_hash_add(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta);
void roaming_free_sta(WID_WLAN *WLAN, struct ROAMING_STAINFO *sta);
struct ROAMING_STAINFO * roaming_sta_add(WID_WLAN *WLAN, const u8 *addr, unsigned int BSSIndex, const u8 *BSSID);
void roaming_del_all_sta(unsigned char WLANID);
struct ROAMING_STAINFO * AsdRoamingStaInfoAdd(struct asd_data *wasd, const u8 *addr);
int RoamingStaInfoToWSM(struct ROAMING_STAINFO *sta, Operate op);
int RoamingStaInfoToWIFI(struct ROAMING_STAINFO *sta, Operate op);
int RoamingStaInfoToWSM_1(struct sta_info *sta, Operate op);
int RoamingStaInfoToWIFI_1(struct sta_info *sta, Operate op);
#endif
