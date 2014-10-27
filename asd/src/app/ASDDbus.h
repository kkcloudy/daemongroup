#ifndef _ASDDBUS_H_
#define	_ASDDBUS_H_

extern unsigned int STA_STATIC_FDB_ABLE;

int asd_dbus_init(void);

int signal_sta_leave(unsigned char mac[6],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi);//xiaodawei add rssi for telecom, 20110228
int signal_sta_leave_abnormal(unsigned char mac[6],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi);
//int signal_sta_come(unsigned char mac[6],unsigned int g_rdio,unsigned int g_bssindex,unsigned char wlanid,unsigned char rssi);//xiaodawei add rssi for telecom, 20110228
int signal_sta_come
(
	struct asd_data *data,
	struct sta_info  *sta
);

int signal_wtp_deny_sta(unsigned int wtpid);
int signal_wapi_trap(unsigned char mac[MAC_LEN],unsigned int bss_index,unsigned char reason);
int signal_sta_verify(const unsigned char mac[MAC_LEN],unsigned int bss_index);

int signal_sta_verify_failed(const unsigned char mac[6],unsigned int bss_index);


int signal_assoc_failed(unsigned char mac[MAC_LEN],unsigned short reason_code, unsigned int bssindex);
int signal_jianquan_failed(unsigned char mac[MAC_LEN],unsigned short reason_code,unsigned int bssindex);
int signal_key_conflict(void);
int signal_de_key_conflict(void);
int signal_de_wtp_deny_sta(unsigned int wtpid);
void signal_radius_connect_failed(const char *ip,unsigned char type);
void signal_radius_connect_failed_clean(const char *ip,unsigned char type);

void *asd_dbus_thread();
void notice_vrrp_state_change(unsigned int vrrid,unsigned int state);
int asd_dbus_reinit(void);
void *asd_dbus_restart_thread();
int notice_hmd_update_state_change(unsigned int vrrid,unsigned int state);


extern unsigned int local_success_roaming_count;
extern unsigned int total_sta_unconnect_count;
extern CHN_TM channel_time[14];
extern unsigned char asd_sta_arp_listen;
extern unsigned char asd_sta_static_arp;
extern unsigned char asd_sta_getip_from_dhcpsnoop;
extern unsigned int ASD_SWITCH ;
extern unsigned int asd_sta_idle_time;
extern unsigned int asd_sta_check_time;  // xk add for asd sta check
extern unsigned char asd_sta_idle_time_switch;
extern unsigned char asd_sta_check_time_switch;  //xk add for asd sta check
extern unsigned char tablesock_flag ;
extern unsigned char datasock_flag ;
extern unsigned char tablesend_flag;
extern unsigned char datasend_flag;
extern unsigned char netlink_flag;

extern unsigned char asd_ipset_switch;
extern unsigned int asd_bak_sta_update_time;
extern int ASD_NOTICE_STA_INFO_TO_PORTAL;
#endif
