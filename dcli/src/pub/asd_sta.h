#ifdef _D_WCPSS_
#ifndef _ASD_STA_H
#define _ASD_STA_H

#define OKB 1024
#define OMB (1024*1024)
#define OGB (1024*1024*1024)
#define MAC_LEN 6
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDRESS "%02X%02X-%02X%02X-%02X%02X"//qiuchen add it for Henan Mobile 2013.02.20
#define DCLI_NEW 1


int parse2_char_ID(char* str,unsigned char* ID);

int parse2_int_ID(char* str,unsigned int* ID);

void asd_state_check(unsigned char *ieee80211_state, unsigned int sta_flags, unsigned char *PAE, unsigned int pae_state, unsigned char *BACKEND, unsigned int backend_state);

void dcli_free_sta(struct dcli_sta_info *sta);

void dcli_free_bss(struct dcli_bss_info *bss);

void dcli_free_radio(struct dcli_radio_info *radio);

void dcli_free_wlan(struct dcli_wlan_info *wlan);

void dcli_free_wtp(struct dcli_wtp_info *wtp);
void dcli_free_wtp_list(struct dcli_wtp_info *wtp);		//mahz add 2011.1.18
void dcli_sta_wapi_mib_info_free_wtp(struct wapi_mib_wtp_info *wtp);		//mahz add 2011.1.25
void dcli_sta_wapi_mib_info_free_wtp_list(struct wapi_mib_wtp_info *wtp);		//mahz add 2011.1.25


void dcli_free_ac(struct dcli_ac_info *ac);

void dcli_free_channel(struct dcli_channel_info *channel);

struct dcli_sta_info* get_sta_info_by_mac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN],int localid, unsigned int *ret);

struct dcli_ac_info* show_sta_summary(DBusConnection *dcli_dbus_connection,int index,int localid, unsigned int *ret);

struct dcli_ac_info* show_sta_list(DBusConnection *dcli_dbus_connection,int index,int localid, unsigned int *ret);

struct dcli_wtp_info* show_sta_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid,int localid, unsigned int *ret);

struct dcli_wlan_info* show_sta_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid,int localid, unsigned int *ret);

struct dcli_wlan_info* show_info_bywlan(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid,int localid, unsigned int *ret);
/*nl add 0091229*/
struct dcli_wtp_info* show_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid,int localid, unsigned int *ret);
/*nl add 20100104*/
struct dcli_wtp_info* show_wapi_mib_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid,int localid, unsigned int *ret);
/*nl add 20100107*/
struct dcli_wtp_info* show_radio_info_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid,int localid, unsigned int *ret);
/*nl add 20100105*/
struct dcli_ac_info* show_traffic_limit_byradio(DBusConnection *dcli_dbus_connection, int index, unsigned int radioid,int localid, unsigned int *ret);

struct dcli_radio_info* show_mib_info_byradio(DBusConnection *dcli_dbus_connection, int index, unsigned int radioid,int localid, unsigned int *ret);

struct dcli_bss_info* show_traffic_limit_bybss(DBusConnection *dcli_dbus_connection, int index, unsigned int bssindex,int localid, unsigned int *ret);

struct dcli_wtp_info* show_wtp_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned int wtpid,int localid, unsigned int *ret);
/*nl add 20100111*/
struct dcli_wlan_info* show_wlan_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid,int localid, unsigned int *ret);

struct dcli_ac_info* show_all_wtp_maclist(DBusConnection *dcli_dbus_connection, int index,int localid, unsigned int *ret);
/*nl add 20100111*/
struct dcli_ac_info* show_all_wlan_maclist(DBusConnection *dcli_dbus_connection, int index,int localid, unsigned int *ret);
/*nl add 20100111*/
struct dcli_ac_info* show_all_bss_maclist(DBusConnection *dcli_dbus_connection, int index,int localid, unsigned int *ret);
/*nl add 20100111*/
struct dcli_bss_info* show_radio_bss_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned int radio_id, unsigned char wlan_id,int localid, unsigned int *ret);
/*nl add 20100205*/

struct dcli_wtp_info* extend_show_sta_bywtp(DBusConnection *dcli_dbus_connection, int index, unsigned int wtp_id,int localid, unsigned int *ret);

struct dcli_channel_info* show_channel_access_time(DBusConnection *dcli_dbus_connection, int index, unsigned char *num,int localid, unsigned int *ret);
/*nl add 20100108*/

struct dcli_wlan_info* show_wlan_wids_maclist(DBusConnection *dcli_dbus_connection, int index, unsigned char wlanid,int localid, unsigned int *ret);

struct dcli_sta_info* check_sta_by_mac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char type, unsigned int value,int localid, unsigned int *ret);
void set_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value,int localid, unsigned int *ret);
void del_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN],unsigned char wlan_id,int localid, unsigned int *ret);
struct sta_static_info *show_sta_static_info_bymac(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN],unsigned char wlan_id,int localid, unsigned int *ret);
struct sta_static_info *show_sta_static_info(DBusConnection *dcli_dbus_connection,int index, unsigned int *num,int localid, unsigned int *ret);
int dcli_asd_set_sta_arp(int index,int localid, int is_add, char *ip, char*macAddr, char*ifname,DBusConnection *dcli_dbus_connection);
#endif
#endif

