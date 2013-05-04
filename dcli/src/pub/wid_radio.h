#ifdef _D_WCPSS_
#ifndef _WID_RADIO_H
#define _WID_RADIO_H

int radio_set_wds_bridge_distance(int localid, int index,int RadioID,int distance,DBusConnection *dcli_dbus_connection);
int radio_set_wds_remote_brmac(int localid,int index,int RadioID,int is_add, unsigned char *macAddr,DBusConnection *dcli_dbus_connection);
int radio_set_wds_wep_key(int localid,int index,int RadioID,char *key,DBusConnection *dcli_dbus_connection);
int radio_set_wds_encryption_type(int localid,int index,int RadioID,int type,DBusConnection *dcli_dbus_connection);
int radio_set_wds_aes_key(int localid,int index,int RadioID,char *key, unsigned char *macAddr,DBusConnection *dcli_dbus_connection);
void wid_set_sta_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value, int localid,unsigned int *ret);
void asd_set_sta_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value, int localid,unsigned int *ret);
int radio_asd_sta_traffic_cancel(int localid,int index,DBusConnection *dbus_connection,unsigned int id,unsigned char wlan_id,unsigned char *mac1,unsigned int *ret_asd);
int radio_asd_sta_taffic_value(int localid,int index,DBusConnection *dbus_connection,unsigned int id,unsigned char wlan_id,unsigned char *mac1,unsigned int *ret_asd);
int set_radio_bss_max_num_asd_wid(int localid,int index,DBusConnection *dbus_connection,unsigned int id,unsigned int type,unsigned int max_sta_num,unsigned int bss_index,int *ret);  //fengwenchao modify 20110512
int dcli_radio_service_control_timer(int localid,int index,int policy,unsigned int radioid,int is_once,int wday,int time,DBusConnection *dcli_dbus_connection);
int dcli_radio_timer_able(int localid,int index,int policy,int timer,unsigned int radioid,DBusConnection *dcli_dbus_connection);
int set_11n_rate_paras(int localid,int index,DBusConnection *dbus_connection,int mcs,int cwmode,int gi,unsigned int id,unsigned char chan);
struct WtpList * set_radio_max_rate_cmd_set_max_rate_v1(int localid, int index,DBusConnection *dbus_connection,unsigned int id,int rate,int *ret,
    int *ret1,int *num,int *mode,int *list1, struct dcli_n_rate_info *nRateInfo, unsigned char *chan,int wflag);

#endif
#endif
