#ifndef _WID_DBUS_H_
#define _WID_DBUS_H_



int wid_dbus_init(void);
void *wid_dbus_thread();

#define ACDBUS_MAC_LEN 6
#define ACDBUS_OUI_LEN 3
#define ACDBUS_LOG_LEVEL_LEN 20
#define ACDBUS_WLAN_VLAN_LEN WLAN_NUM  //fengwenchao modify  "16" to "WLAN_NUM" for TESETBED-71 ,20120614
#define ACDBUS_MODEL_LEN 16
#define ACDBUS_RADOI_TYPE_LEN 8

//#define ACDBUS_GROUP 1

typedef enum{
	check_wtpid=0,
	check_sub,
	check_sub2,	
	check_sub3,
	check_radioid,
	check_wlanid,
	check_point,
	check_fail,
	check_end,
	check_success,
	check_vrrip,	
	check_slotid
}radio_ifname_state;

#define PARSE_RADIO_IFNAME_SUB '-'
#define PARSE_RADIO_IFNAME_POINT '.'

struct Wtp_List{
	int WtpId;
	char FailReason;
	//unsigned char wtpMacAddr[6];
	/*struct WtpList *next;
	struct WtpList *WtpList_list;
	struct WtpList *WtpList_last;*/
};
struct Radio_List{
	int RadioId;
	char FailReason;
	//char countrycode_failreason;
	//int Failtxpof_ID; 
};

typedef struct
{
    unsigned char       macaddr[ACDBUS_MAC_LEN];
}WIDMACADDR;
#define FREE(STR) {if(STR) {free(STR); STR=NULL;}}	//xiaodawei add for iperf, 20110407
#define IPERF_LINE_NUM 32
#define IPERF_LINE_CONTENT 128
/*struct NetworkQuality{//xiaodawei add, 20110312
	double jitter;	//latency variation, in Iperf UDP test, millisecond, e.g. 0.037ms
	double datagramloss;		//datagram loss, % percent, e.g. 2.2%
};*/
int wid_dbus_trap_wtp_enter_imagedata_state(int wtpindex);
int wid_dbus_trap_wtp_channel_change(unsigned char chan_past,unsigned char chan_curr,unsigned int radioid);
int wid_dbus_trap_wtp_code_start(int wtpindex);
int wid_dbus_trap_wtp_electrify_register_circle(int wtpindex, int registertimer);
int wid_dbus_trap_wlan_encryption_type_change(int wlanid);
int wid_dbus_trap_wlan_preshared_key_change(int wlanid,char *key);
int wid_dbus_trap_wtp_tranfer_file(int wtpindex);
int wid_dbus_trap_wtp_update_successful(int wtpindex);    /*fengwenchao add 20110216*/
int wid_dbus_trap_wtp_update_fail(int wtpindex);              /*fengwenchao add 20110216*/
int wid_dbus_trap_wtp_ap_power_off(int wtpindex);
int wid_dbus_trap_wtp_ip_change_alarm(int wtpindex);
int wid_dbus_trap_wtp_device_interference(int wtpindex);
int wid_dbus_trap_set_wtp_remote_restart(unsigned int wtpindex);
int wid_dbus_trap_wtp_divorce_networwok(unsigned int wtpindex);
int wid_dbus_trap_wtp_ap_reboot(int wtpindex);
int wid_dbus_trap_wtp_ap_ACTimeSynchroFailure(int wtpindex,unsigned char flag);
int wid_dbus_trap_wtp_channel_device_interference(int wtpindex,char chchannel,unsigned char mac[6]);
int wid_dbus_trap_wtp_channel_ap_interference(int wtpindex,char chchannel,unsigned char mac[6]);
int wid_dbus_trap_wtp_channel_terminal_interference(int wtpindex,unsigned char radio_id, char chchannel,unsigned char mac[6]);
int wid_dbus_trap_wtp_channel_count_minor(int wtpindex);
int wid_dbus_trap_wtp_ap_flash_write_failed(unsigned int wtpindex);
int wid_dbus_trap_wtp_ac_discovery_danger_ap(unsigned int wtpindex,struct Neighbor_AP_ELE *p_rssi);   //fengwenchao modify 20110509
int wid_dbus_trap_wtp_ac_discovery_cover_hole(unsigned int wtpindex);
/*int signal_sta_leave(unsigned char mac[6],unsigned int wtpid,unsigned int g_bssindex,unsigned char wlanid);
int signal_sta_come(unsigned char mac[6],unsigned int wtpid,unsigned int g_bssindex,unsigned char wlanid);
int signal_wtp_deny_sta(unsigned int wtpid);
int signal_de_wtp_deny_sta(unsigned int wtpid);

int signal_sta_verify(unsigned char mac[6],int wtpid);
int signal_sta_verify_failed(unsigned char mac[6],int wtpid);
int signal_sta_assoc_failed(unsigned char mac [ 6 ],int wtpid,unsigned short ret);
int signal_sta_jianquan_failed(unsigned char mac [ 6 ],int wtpid,unsigned short ret);*/
int wid_dbus_trap_wtp_wireless_interface_down(unsigned int wtpindex);
int wid_dbus_trap_wtp_wireless_interface_down_clear(unsigned int wtpindex);

int wid_dbus_trap_ap_cpu_threshold(unsigned int wtpindex,unsigned char flag);
int wid_dbus_trap_ap_mem_threshold(unsigned int wtpindex,unsigned char flag);
int wid_dbus_trap_ap_temp_threshold(unsigned int wtpindex,unsigned char flag);
int wid_dbus_trap_ap_rogue_threshold(unsigned int wtpindex,unsigned int count,unsigned char flag);
int wid_dbus_trap_ap_wifi_if_error(unsigned int wtpindex,unsigned char ifindex,unsigned char flag);
int wid_dbus_trap_ap_ath_error(unsigned int wtpindex,unsigned char radioid,unsigned char wlanid,unsigned char type,unsigned char flag);

int wid_dbus_trap_wtp_channel_device_interference_clear(int wtpindex,char chchannel);
int wid_dbus_trap_wtp_channel_ap_interference_clear(int wtpindex,char chchannel);
int wid_dbus_trap_wtp_channel_terminal_interference_clear(int wtpindex,unsigned char radio_id, char chchannel,unsigned char mac[6]);
int wid_dbus_trap_wtp_channel_count_minor_clear(int wtpindex);
int wid_dbus_trap_wtp_ac_discovery_cover_hole_clear(unsigned int wtpindex);
int wid_dbus_trap_ap_rrm_state_change(unsigned int wtpindex,unsigned char state);
int wid_dbus_trap_ap_run_quit(unsigned int wtpindex,unsigned char state);
int read_ac_info(char *FILENAME,char *buff);
int parse_int_ID(char* str,unsigned int* ID);
int wid_dbus_trap_wtp_find_unsafe_essid(unsigned int wtpindex,char * name);
int wid_dbus_trap_wtp_find_wids_attack(unsigned int wtpindex,struct tag_wids_device_ele *phead);   //fengwenchao modify 20110509
int wid_dbus_reinit(void);
void *wid_dbus_thread_restart();
extern CWBool find_in_uptfail_wtp_list(int id);
int notice_hmd_update_state_change(unsigned int vrrid,unsigned int state);
int wid_dbug_trap_more_ssid_key_conflict(unsigned int RadioID,unsigned char wlan1, unsigned char wlan2,char *ESSID1,char *ESSID2);
int wid_dbug_trap_ssid_key_conflict(unsigned int wtpid,unsigned char radio_l_id, unsigned char wlan1, unsigned char wlan2);


#endif

