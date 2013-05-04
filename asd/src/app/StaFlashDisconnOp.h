#ifndef STA_FLASH_DISCONN_H
#define STA_FLASH_DISCONN_H
struct FLASHDISCONN_STAINFO * flashdisconn_sta_add(asd_sta_flash_disconn *FDStas, const u8 *addr, unsigned int BSSIndex, unsigned char WLANID);
void sta_flash_disconn_check(asd_sta_flash_disconn *FDStas, const unsigned char *addr);
void sta_flash_disconn_timer(void *eloop_ctx, void *timeout_ctx);
void flashdisconn_del_all_sta(asd_sta_flash_disconn *FDStas);
extern int ASD_NOTICE_STA_INFO_TO_PORTAL;
extern int ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER;

#endif

