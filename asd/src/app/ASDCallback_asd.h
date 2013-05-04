#ifndef _ASDCALLBACK_ASD_H_
#define	_ASDCALLBACK_ASD_H_
#include "asd.h"
extern int CmdSock_s;
extern unixAddr toASD_C;
extern int DmSock;
void handle_msg(int sock, void *circle_ctx, void *sock_ctx);
void handle_read(int sock, void *circle_ctx, void *sock_ctx);
void ASD_NETLINIK_INIT();
int del_mac_in_blacklist_for_wids(struct acl_config *conf,  u8 *addr);
int asd_hw_feature_init(struct asd_iface *iface);
int asd_send_wapi(void *priv, const u8 *addr, const u8 *data,
			     size_t data_len, int encrypt, const u8 *own_addr);
void asd_free_bss_conf(struct asd_bss_config *bss);
int ASD_BSS_SECURITY_INIT(struct asd_data * wasd);
void asd_sock_reinit(int fd,void *handler,void *circle_data, void *user_data);
//qiuchen add it for master_bak radius server 2012.12.17
int ASD_RADIUS_INIT(unsigned char SID);
void radius_heart_test_on(unsigned char SID);
void asd_free_radius(struct heart_test_radius_data* radius);
int ASD_RADIUS_DEINIT(unsigned char SID);
void radius_heart_test_off(unsigned char SID);
void radius_test_cancel_timers(unsigned SID);
//end
#endif

