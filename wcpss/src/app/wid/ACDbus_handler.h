//#include "WID.h"

#ifndef __WID_ACDbus_handler__
#define __WID_ACDbus_handler__

#include "CWAC.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

#define ACDBUSHANDLE_11BG_RATE_LIST_LEN	12
#define STA_DEFAULT_TRAFFIC_LIMIT 80000
#define READ_IFNET_INFO_COUNT 5

int istryreadipv6addr;
int istrybindipv6addr;
#define COUNTERMEASURE_DEFAULT_COUNT 6

#define WTP_ID_HASH(ID) (ID%256)
extern int ACDBUS_MSGQ;
extern unsigned int isNoCheck;
int set_balance_probe_extension_command(int wtpid, char * command);

int balance_probe_extend_command(unsigned char wlanid,unsigned char state);

int WID_CREATE_NEW_WLAN(char *WlanName, unsigned char WlanID,unsigned char *ESSID,unsigned char *ESSID_STR,unsigned char cnFlag);
int WID_DELETE_WLAN(unsigned char WlanID);
int WID_CREATE_NEW_WTP(char *WTPNAME, unsigned int WTPID, unsigned char* WTPSN, char* WTPModel,int issn, int apcodeflag, char *code);
int WID_DELETE_WTP(unsigned int WTPID);
int WID_SUSPEND_WTP(unsigned int WTPID);
int WID_USED_WTP(unsigned int WtpID);
int WID_UNUSED_WTP(unsigned int WtpID);
int WID_DISABLE_WLAN(unsigned char WlanID);
int WID_ENABLE_WLAN(unsigned char WlanID);
int WID_ADD_IF_APPLY_WLAN(unsigned char WlanID, char * ifname);
int WID_RADIO_SET_TYPE(unsigned int RadioID, unsigned int RadioType);
int WID_RADIO_SET_TXP(unsigned int RadioID, unsigned short RadioTxp,CWBool flag);
int WID_RADIO_SET_CHAN(unsigned int RadioID, unsigned char RadioChan);
int WID_ADD_WLAN_APPLY_RADIO(unsigned int RadioID,unsigned char WlanID);
//Added by weiay 20080714
//int WID_RADIO_SET_RATE(unsigned int RadioID, unsigned short RadioRate);
int WID_RADIO_SET_SUPPORT_RATE(unsigned int RadioID,int RadioRate [ ],int flag,int num);
int WID_RADIO_SET_MODE(unsigned int RadioID, unsigned int RadioMode);
//added end
int WID_RADIO_SET_BEACON(unsigned int RadioID, unsigned short beaconinterval);
int WID_RADIO_SET_FRAGMENTATION(unsigned int RadioID, unsigned short fragmentation);
int WID_RADIO_SET_DTIM(unsigned int RadioID, unsigned char dtim);
int WID_RADIO_SET_RTSTHRESHOLD(unsigned int RadioID, unsigned short rtsthreshold);
int WID_RADIO_SET_STATUS(unsigned int RadioID, unsigned char status);
int WID_RADIO_SET_PREAMBLE(unsigned int RadioID, unsigned char preamble);
int WID_RADIO_SET_SHORTRETRY(unsigned int RadioID, unsigned char shortretry);
int WID_RADIO_SET_LONGRETRY(unsigned int RadioID, unsigned char longretry);
/*fengwenchao add 20120509 for onlinebug-271*/
int check_whether_in_ebr(unsigned int index, int wtpid, unsigned int radioid,unsigned char wlanid, int *ebr_id);
int DELETE_WLAN_CHECK_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId);
/*fengwenchao add end*/
int WID_BINDING_IF_APPLY_WTP(unsigned int WtpID, char * ifname);
int WID_BINDING_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId);
int Check_And_Bind_Interface_For_WID(char * ifname);
int Check_And_Bind_Ipaddr_For_WID(unsigned int ipaddr,LISTEN_FLAG flag);
int WID_DELETE_IF_APPLY_WLAN(unsigned char WlanID, char * ifname);
int WID_WLAN_HIDE_ESSID(unsigned char WlanID, unsigned char Hideessid);
int WID_WLAN_L3IF_POLICY(unsigned char WlanID, unsigned char wlanPolicy);
int WID_BSS_L3IF_POLICY(unsigned int WlanID,unsigned int wtpID,unsigned int radioID,unsigned char BSSID,unsigned char bssPolicy);
int WID_RADIO_BSS_L3IF_POLICY(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy);
int WID_DELETE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId);
int WID_DISABLE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId);
int WID_ENABLE_WLAN_APPLY_WTP(unsigned int WtpID, unsigned char WlanId);

int WID_DELETE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId);
int WID_DISABLE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId);
int WID_ENABLE_WLAN_APPLY_RADIO(unsigned int RadioId, unsigned char WlanId);

int Bind_Interface_For_WID(struct ifi_info *ifi, int port,LISTEN_FLAG lic_flag);
void Check_gACSokcet_Poll(CWMultiHomedSocket *ptr);
int Repair_WID_Listening_Socket(struct CWMultiHomedInterface *inf);

int Bind_BroadAddr_For_WID(struct ifi_info *ifi, int port);
int Match_And_Modify_Interface(struct CWMultiHomedInterface *p,struct CWMultiHomedInterface *pbr, struct ifi_info *ifi);
int Modify_Interface(struct CWMultiHomedInterface *p, struct CWMultiHomedInterface *pbr,  struct ifi_info *ifi, int port);
void Check_Current_Interface(char * ifname, struct CWMultiHomedInterface **p, struct CWMultiHomedInterface **pbr);
void Check_Current_Interface_and_delete(char * ifname, struct ifi_info *ifi);
int Get_Interface_Info(char * ifname, struct ifi_info *ifi);
int Check_Interface_Config(char * ifname,WTPQUITREASON *quitreason);
int delete_wlan_bss(unsigned int WtpID, unsigned char WlanId);
int delete_wlan_bss_by_radioId(unsigned int RadioId, unsigned char WlanId);
int delete_wlan_all_bss(unsigned int WtpID);

int wid_set_ap_scanning(APScanningSetting scansetting);
int Create_BSS_L3_Interface(unsigned int BSSIndex);
int Delete_BSS_L3_Interface(unsigned int BSSIndex);
int Create_Wlan_L3_Interface(unsigned 	char WlanID);
int Delete_Wlan_L3_Interface(unsigned char WlanID);
int WID_WTP_TUNNEL_MODE_CHECK(int WtpID);
void Check_WLAN_WTP_IF_Index(struct ifi_info *ifi, char *ifname);
void display_ap_info_list(Neighbor_AP_INFOS *paplist);
void destroy_ap_info_list(Neighbor_AP_INFOS **paplist);
Neighbor_AP_INFOS * create_ap_info_list(int count);
Neighbor_AP_INFOS * wid_check_rogue_ap_all();
Neighbor_AP_INFOS * wid_check_rogue_ap_mac(int wtpid);
void wid_mark_rogue_ap(Neighbor_AP_INFOS *paplist);
int wid_count_rogue_ap(Neighbor_AP_INFOS *paplist,int wtpid);
CWBool insert_elem_into_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem);
CWBool modify_elem_into_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem);
CWBool delete_elem_from_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem);
CWBool delete_elem_from_ap_list_wtpid(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem,int wtpid);
void merge_ap_list(Neighbor_AP_INFOS *pdestlist, Neighbor_AP_INFOS **psrclist,int wtpid);
void delete_rouge_ap_list_by_ouilist(Neighbor_AP_INFOS **paplist);

void delete_rouge_ap_list_by_essidlist(Neighbor_AP_INFOS **paplist);

void delete_rouge_ap_list_by_whitelist(Neighbor_AP_INFOS **paplist);
struct Neighbor_AP_ELE * create_mac_elem(char *mac);
Neighbor_AP_INFOS * create_ap_info_list_test(int count);//just for test will delete later
white_mac_list * wid_check_white_mac();
Neighbor_AP_INFOS * wid_get_neighbor_ap_list();
CWBool delete_elem_from_ap_list_bymac(Neighbor_AP_INFOS **paplist,struct white_mac *pmac);

CWBool delete_elem_from_ap_list_byoui(Neighbor_AP_INFOS **paplist,struct oui_node *oui);

CWBool delete_elem_from_ap_list_byessid(Neighbor_AP_INFOS **paplist,struct essid_node *essid);

void wid_destroy_white_mac(white_mac_list **pmaclist);
void display_mac_info_list(white_mac_list *pmaclist);
CWBool check_elem_in_ap_list(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem);
CWBool insert_elem_into_ap_list_head(Neighbor_AP_INFOS *paplist,struct Neighbor_AP_ELE *elem);
struct Neighbor_AP_ELE * create_ap_elem(struct Neighbor_AP_ELE *apelem);
void wid_check_wtp_apply_wlan(unsigned char WlanID, char * ifname);
struct Support_Rate_List *create_support_rate_list(int count);
void destroy_support_rate_list(struct Support_Rate_List *ratelist);
void display_support_rate_list(struct Support_Rate_List *ratelist);
struct Support_Rate_List *delete_rate_from_list(struct Support_Rate_List *ratelist,int rate);
struct Support_Rate_List *insert_rate_into_list(struct Support_Rate_List *ratelist,int rate);
struct Support_Rate_List *find_rate_from_list(struct Support_Rate_List *ratelist,int rate);
int   length_of_rate_list(struct Support_Rate_List *ratelist);
CWBool wid_add_mac_blacklist(unsigned char * pmac);
CWBool wid_add_mac_whitelist(unsigned char * pmac);
CWBool wid_delete_mac_blacklist(unsigned char * pmac);
CWBool wid_delete_mac_whitelist(unsigned char * pmac);
void get_power_control_info(transmit_power_control *txpoer_control_info, Neighbor_AP_INFOS *paplist);
void calc_transmit_power_control(unsigned int wtpid, Neighbor_AP_INFOS *paplist,int *setflag);
void display_power_control_info(transmit_power_control power_control_info);
int WID_INTERFACE_SET_NASID(unsigned char WlanID,char* ifname,char* nas_id);

int wid_set_ap_statistics(int apstatics);

/*fengwenchao add 20110408*/
int WID_RADIO_CHANGE_SUPPORT_RATE_BYGI_MCS_CWMODE(unsigned int RadioID);
int WID_RADIO_CHANGE_SUPPORT_RATE_BYGI_MCS_CWMODE_1(unsigned int RadioID,int count);//qiuchen add it 

/*fengwenchao add end*/
int WID_ADD_QOS_PROFILE(char * name,int ID);
int WID_DELETE_QOS_PROFILE(int ID);
int WID_QOS_SET_QOS_INFO(int ID,int qos_stream_id,unsigned short cwmin,unsigned short cwmax,unsigned char aifs,unsigned char ack,unsigned short txoplimit);
int WID_QOS_SET_QOS_INFO_CLIENT(int ID,int qos_stream_id,unsigned short cwmin,unsigned short cwmax,unsigned char aifs,unsigned short txoplimit);
int WID_QOS_SET_QOS_WMM_MAP(int ID,int isadd);
int WID_QOS_SET_QOS_WMM_MAP_DOT1P(int ID,int wmm_order,unsigned char dot1p);
int WID_QOS_SET_QOS_DOT1P_MAP_WMM(int ID,int wmm_order,unsigned char num,unsigned char dot1p[]);
int WID_ADD_RADIO_APPLY_QOS(unsigned int RadioID,int qosID,int qosstate);
//int WID_ADD_RADIO_DELETE_QOS(unsigned int RadioID,int qosID);
int wid_set_qos_flow_parameter_value(unsigned int qosid,unsigned char streamid,unsigned int type,unsigned int value);
int wid_set_qos_parameter_value(unsigned int qosid,unsigned int type,unsigned int value);

int wid_radio_set_extension_command(int wtpid, char * command);
int wid_radio_set_option60_parameter(int wtpid, char * command);

int wid_radio_set_throughout(int wtpid, unsigned char bandwidth);
int wid_mul_radio_set_throughout(unsigned int radioid,unsigned char bandwidth);
int wid_radio_set_ip_gateway(int wtpid,unsigned int ip,unsigned int gateway,unsigned char mask);
int wid_set_ap_timestamp(int timestamp);
int wid_trap_channel_disturb_enable(unsigned int wtpid);
int wid_trap_channel_disturb_disable(unsigned int wtpid);
void channel_interference_detected(int wtpid);
int wid_send_to_ap_extension_infomation(unsigned int wtpid);
int set_wid_sample_enable();
int wid_radio_bss_set_max_throughput(unsigned int wtp_id,unsigned int l_radio_id,unsigned int l_bss_id,unsigned int throughput);
int wid_set_wlan_vlanid(unsigned char wlanid,unsigned int vlanid);
int wid_set_wlan_vlan_priority(unsigned char wlanid,unsigned int priority);

int wid_radio_wsm_sta_info_report(unsigned int wtpid,unsigned int l_radioid,unsigned int radioid,unsigned int bssindex);

//xm add
int wid_add_manufacturer_oui(unsigned char oui[]);
int wid_add_legal_essid(char *essid);
int wid_add_attack_ap_mac(unsigned char mac[]);
int wid_del_manufacturer_oui(unsigned char oui[]);
int wid_del_legal_essid(char *essid);
int wid_del_attack_ap_mac(unsigned char mac[]);

/*nl add begin*/
unsigned int Wid_Find_Wtp(WID_WTP **WTP);
unsigned char WID_WTP_FIND_RADIO(WID_WTP_RADIO	**AC_RADIO_FOR_SEARCH,WID_WTP *WTP);
unsigned int Wid_Find_ROUGE_Wtp(WID_WTP **WTP,unsigned int *wtp_num);
unsigned int WID_FIND_Running_Wtp(WID_WTP **WTP);

int wid_del_cpu_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num);
int wid_ap_cpu_mem_statistics(unsigned int wtpid);
int wid_accouting_cpu_average(unsigned int wtpid);
int accounting_snr_math_average(unsigned int wtpid,int average_num);
unsigned int Wid_Find_Running_Wtp(WID_WTP **WTP);
int WID_CHECK_WTP_ID(u_int32_t Id);
int WID_CHECK_WLAN_ID(u_int32_t Id);
int WID_CHECK_G_RADIO_ID(u_int32_t Id);
int WID_CHECK_ID(unsigned int TYPE,u_int32_t Id);
int wid_accouting_cpu_sample_average_and_peak(unsigned int wtpid);
int wid_accouting_mem_average(unsigned int wtpid);
int wid_accouting_mem_sample_average_and_peak(unsigned int wtpid);
int wid_del_mem_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num);
int wid_add_cpu_value_for_accounting_average(unsigned int wtpid,unsigned int cpu_value);
int wid_add_mem_value_for_accounting_average(unsigned int wtpid,unsigned int mem_value);
int wid_add_snr_value_for_accounting_average(unsigned int wtpid,unsigned char snr_value,unsigned int l_radioid); //fengwenchao modify  20120314 for onlinebug-162//qiuchen copy from v1.3
int wid_del_snr_value_for_accounting_average(unsigned int wtpid,unsigned int total_node_num,unsigned int l_radioid); //fengwenchao modify 20120314 for onlinebug-162
int wid_accouting_snr_average(unsigned int wtpid,unsigned int l_radioid);  //fengwenchao modify  20120314 for onlinebug-162
int wid_accouting_snr_sample_average_and_peak(unsigned int wtpid,unsigned int l_radioid); //fengwenchao modify  20120314 for onlinebug-162//qiuchen copy from v1.3 end

struct ap_snr_info  * find_next_sample_node(unsigned int wtpid,unsigned int i_sample,struct ap_snr_info  *Node_snr);
struct ap_cpu_info  * find_next_sample_node2(unsigned int wtpid,unsigned int i_sample,struct ap_cpu_info  *Node_snr);

/*nl add end*/

/*fengwenchao add 20110401 for dot11WlanDataPktsTable*/
int WID_CHECK_SAME_ATH_OF_ALL_WTP(unsigned char wlanid,unsigned int *tx_pkt,unsigned long long *rx_bts,unsigned long long *tx_bts);
/*fengwenchao add end*/

/********************add for id check  nl ****************/
int WID_CHECK_WTP_ID(u_int32_t Id);
int WID_CHECK_WLAN_ID(u_int32_t Id);
int WID_CHECK_G_RADIO_ID(u_int32_t Id);
int WID_CHECK_ID(unsigned int TYPE,u_int32_t Id);
/********************add for id check  nl ****************/


//ACMsgq.c
void WID_INSERT_CONTROL_LIST(unsigned int WTPID, struct msgqlist *elem);
struct msgqlist *WID_GET_CONTROL_LIST_ELEM(unsigned int WTPID);
void WID_CLEAN_CONTROL_LIST(unsigned int WTPID);
int wid_set_ap_l2_isolation_enable(unsigned int wtpid,unsigned char wlanid);
int wid_set_ap_l2_isolation_disable(unsigned int wtpid,unsigned char wlanid);
int wid_set_ap_igmp_snoop_enable(unsigned int wtpid);
int wid_set_ap_igmp_snoop_disable(unsigned int wtpid);
int wid_set_ap_dos_def_enable(unsigned int wtpid);
int wid_set_ap_dos_def_disable(unsigned int wtpid);
int WID_ADD_WLAN_APPLY_RADIO_BASE_VLANID(unsigned int RadioID,unsigned char WlanID,unsigned int vlan_id);
int WID_ADD_WLAN_APPLY_RADIO_BASE_NAS_PORT_ID(unsigned int RadioID,unsigned char WlanID,char* nas_port_id);	//mahz add 2011.5.30
//wlan l3 interface turn to br area
int ADD_BSS_L3_Interface_BR(unsigned int BSSIndex);
int Del_BSS_L3_Interface_BR(unsigned int BSSIndex);
int Create_Wlan_L3_BR_Interface(unsigned char WlanID);
int Delete_Wlan_L3_BR_Interface(unsigned char WlanID);
int WID_WLAN_L3IF_POLICY_BR(unsigned char WlanID,unsigned char wlanPolicy);
int WID_RADIO_BSS_L3IF_POLICY_BR(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy);
int wid_set_wlan_br_isolation(unsigned char wlanid,unsigned char state);
int wid_set_wlan_br_multicast_isolation(unsigned char wlanid,unsigned char state);
int wid_set_wlan_br_sameportswitch(unsigned char wlanid,unsigned char state);
int wid_set_tunnel_wlan_vlan(unsigned char wlanid,char * ifname);
int wid_undo_tunnel_wlan_vlan(unsigned char wlanid,char * ifname);
void wtp_get_ifindex_check_nas_id(u_int32_t WTPID);
int WID_RADIO_BSS_FORWARD_MODE(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy);
int WID_RADIO_BSS_TUNNEL_MODE(unsigned char WlanID,unsigned int wtpID,unsigned char radioID,unsigned char BSSID,unsigned char bssPolicy,char nodeFlag);

//ethereal bridge area
int Check_Interface_Exist(char * ifname,WTPQUITREASON * quitreason);
int WID_ADD_ETHEREAL_BRIDGE(char * name,unsigned int ID);
int WID_DELETE_ETHEREAL_BRIDGE(unsigned int ID);
int WID_SET_ETHEREAL_BRIDGE_ENABLE(unsigned int ID);
int WID_SET_ETHEREAL_BRIDGE_DISABLE(unsigned int ID);
int wid_set_ebr_isolation(unsigned int ID,unsigned char state);
int wid_set_ebr_multcast_isolation(unsigned int ID,unsigned char state);
int wid_set_wlan_ebr_br_ucast_solicit(unsigned int Wlan_Ebr_ID, unsigned char state, unsigned char Wlan_Ebr_flag);
int wid_set_wlan_ebr_br_mcast_solicit(unsigned int Wlan_Ebr_ID, unsigned char state, unsigned char Wlan_Ebr_flag);
int wid_set_ebr_multicast_fdb_learn(unsigned int ID,unsigned char state);
int wid_set_ebr_sameportswitch(unsigned int ID,unsigned char state);
int WID_SET_ETHEREAL_BRIDGE_IF_UPLINK(unsigned int ID,char *ifname,int is_radio,int g_radioid,int wlanid);
int WID_SET_ETHEREAL_BRIDGE_IF_DOWNLINK(unsigned int ID,char * ifname,int is_radio,int g_radioid,int wlanid);
int wid_parse_wtp_cpu_mem_trap_info(unsigned int wtpid);
int wid_set_ap_sta_infomation_report(unsigned int wtpid);
int wid_set_ap_if_info_report(unsigned int wtpid);
int wid_set_ap_wids_set(unsigned int wtpid);
int wid_set_ap_if_updown(unsigned int wtpid,unsigned char type,unsigned char ifindex,unsigned char policy);
int wid_set_radio_l2_isolation_enable(unsigned int wtpid,unsigned int radioid,unsigned char wlanid);
int wid_set_radio_l2_isolation_disable(unsigned int wtpid,unsigned int radioid,unsigned char wlanid);
int wid_set_wtp_ntp(unsigned int wtpid);
int wid_set_radio_11n_cwmmode(unsigned int wtpid,unsigned int radioid,unsigned char wlanid,unsigned char policy);
//auto ap login area
int wid_auto_ap_login_insert_iflist(char * ifname);
int wid_auto_ap_login_remove_iflist(char * ifname);

int WID_wds_disable(unsigned char wlanid,unsigned char wds_mesh);
int WID_wds_enable(unsigned char wlanid,unsigned char wds_mesh);
int WID_RADIO_SET_WDS_STATUS(unsigned int RadioID, unsigned char WLANID, unsigned char status);
int CWWIDReInit();
int Delete_Bind_Interface_For_WID(struct ifi_info *ifi);
int wid_update_ap_config(int wtpid,char *ip);
void wid_parse_neighbor_ap_list(unsigned int wtpindex);

void merge_wids_list(wid_wids_device *pdestlist, wid_wids_device **psrclist,int wtpid);
CWBool update_elem_in_wids_list(wid_wids_device  *paplist,struct tag_wids_device_ele *elem);
CWBool find_elem_in_wids_list(wid_wids_device  *paplist,struct tag_wids_device_ele *elem);
CWBool insert_elem_into_wids_list(wid_wids_device *paplist,struct tag_wids_device_ele *elem);
void delete_wids_list(wid_wids_device **paplist);
wid_wids_device * wid_check_wids_device_all();
wid_wids_device * wid_get_wids_device_list();
CWBool check_elem_in_wids_list(wid_wids_device *paplist,struct tag_wids_device_ele *elem);
struct tag_wids_device_ele * create_wids_elem(struct tag_wids_device_ele *apelem);
CWBool insert_elem_into_wids_list_head(wid_wids_device *paplist,struct tag_wids_device_ele *elem);
wid_wids_device * create_wids_info_list(int count);
void display_wids_info_list(wid_wids_device *paplist);

int add_ipip_tunnel(unsigned int BSSIndex);
int delete_ipip_tunnel(unsigned int BSSIndex);
int wid_set_neighbordead_intervalt(unsigned int wtpid, int neighbordead_interval);
int wid_set_radio_auto_channel_able(unsigned int wtpid,unsigned int l_radioid,unsigned int able);
int wid_set_radio_diversity(unsigned int wtpid,unsigned int l_radioid,unsigned int able);
int wid_set_radio_txantenna(unsigned int wtpid,unsigned int l_radioid,unsigned int able);
int wid_set_radio_diversity_txantenna_after_run(unsigned int wtpid,unsigned int l_radioid);
int wid_parse_wtp_code_for_radio_set(unsigned int wtpid);
int wid_set_ap_reboot(unsigned int WtpID);
int  wtp_set_sta_info_report(unsigned int wtpid,int policy); /* fengwenchao add 20101213  */
int wtp_set_wtp_dhcp_snooping(unsigned int wtpid,int policy);/*wcl add for globle variable*/
int wid_set_sta_ip_mac_binding(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned int value);
int wid_set_radio_sector_value(unsigned int wtpid,	unsigned int l_radioid,unsigned short value,unsigned policy);
int wid_set_radio_sector_tx_power_value(unsigned int wtpid,	unsigned int l_radioid,unsigned short sectorid,unsigned int value);
int wid_set_radio_tx_chainmask_value(unsigned int wtpid,	unsigned int l_radioid,unsigned char value,unsigned policy);

int wid_set_dhcp_before_autherized(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned int value);
int wid_set_sta_vlan_id(unsigned int wtpid,	unsigned int l_radioid,unsigned char wlanid,
					unsigned char *mac,unsigned int value);
int wid_set_bss_traffic_limit(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char able);
int wid_set_bss_traffic_limit_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend);
int wid_set_bss_traffic_limit_sta_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char mac0,unsigned char mac1,unsigned char mac2,unsigned char mac3,unsigned char mac4,unsigned char mac5,unsigned int value,unsigned char issend);
int wid_cancel_bss_traffic_limit_sta_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char mac0,unsigned char mac1,unsigned char mac2,unsigned char mac3,unsigned char mac4,unsigned char mac5,unsigned int value,unsigned char flag,unsigned char issend);
int wid_set_bss_traffic_limit_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend);
int wid_radio_set_wlan_traffic_limit_able(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char policy);
int wid_radio_set_wlan_traffic_limit_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend);
int wid_radio_set_wlan_traffic_limit_cancel_average_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned char issend);//fengwenchao add for AXSSZFU-1374

int wid_radio_set_wlan_traffic_limit_sta_value(unsigned int wtpid
											,unsigned int l_radioid
											,unsigned char wlanid
											,unsigned char mac0
											,unsigned char mac1
											,unsigned char mac2
											,unsigned char mac3
											,unsigned char mac4
											,unsigned char mac5
											,unsigned int value
											,unsigned char issend);
int wid_radio_set_wlan_traffic_limit_cancel_sta_value(unsigned int wtpid
											,unsigned int l_radioid
											,unsigned char wlanid
											,unsigned char mac0
											,unsigned char mac1
											,unsigned char mac2
											,unsigned char mac3
											,unsigned char mac4
											,unsigned char mac5
											,unsigned char flag
											,unsigned char issend);

int wid_radio_set_wlan_traffic_limit_value(unsigned int wtpid,unsigned int l_radioid,unsigned char wlanid,unsigned int value,unsigned char issend);
int wid_radio_set_whole_wlan_traffic_limit_value(unsigned char wlanid,unsigned int value,unsigned char issend);
int wid_set_radio_diversity_txantenna_after_run_new(unsigned int wtpid,unsigned int l_radioid);
void update_next_wtp();
void update_current_wtp();

CWBool insert_wtp_list(int id);
CWBool delete_wtp_list(int id);
CWBool find_in_wtp_list(int id);
void destroy_wtp_list();
int wid_remove_wlan_interface_nasid(unsigned char WlanID,char * ifname);
int WID_ADD_WLAN_APPLY_RADIO_CLEAN_VLANID(unsigned int RadioID,unsigned char WlanID);
CWBool CWSaveWTPExtensionInfo2(wid_wifi_info ap_wifi_info, unsigned int WTPIndex);
void save_sample_throughput_info(int WTPIndex, wid_sample_rate_info sample_throughput);
void save_extension_info(int WTPIndex, wid_wifi_info ap_wifi_info);
int wid_radio_set_ip_gateway_dns(int wtpid,unsigned int ip,unsigned int gateway,
								unsigned int mask,unsigned int fstdns,unsigned int snddns);
int wid_set_ap_hotreboot(int hotreboot);
int wid_add_del_wids_mac(unsigned char mac[],int isadd);
int wid_set_ap_sta_wapi_info_report(unsigned int wtpid);
int WID_BINDING_IF_APPLY_WTP_ipv6_ioctl(unsigned int WtpID, char * ifname);
int WID_ADD_IF_APPLY_WLAN_ipv6_ioctl(unsigned char WlanID, char * ifname);
int wid_set_ethereal_bridge_add_uplink(unsigned int ID,char *ifname);
int wid_set_ethereal_bridge_del_uplink(unsigned int ID,char *ifname);

void wid_wtp_radio_extern_command_check(unsigned int WTPIndex,int L_Radio_ID);
int add_ap_group_member(unsigned int GID,unsigned int WTPID);
int del_ap_group_member(unsigned int GID,unsigned int WTPID);
int wid_radio_set_acktimeout_distance(unsigned int RadioID);/*wcl add for RDIR-33*/
/* zhangshu add for set 11n paras, 2010-10-09 */
int wid_radio_set_ampdu_able(unsigned int RadioID, unsigned char type);
int wid_radio_set_ampdu_limit(unsigned int RadioID, unsigned char type);
int wid_radio_set_ampdu_subframe(unsigned int RadioID, unsigned char type);

int wid_update_bss_to_wifi(unsigned int bssindex,unsigned int WTPIndex,unsigned char flag);
void WID_Save_Traffic_Limit(unsigned int bssindex, unsigned int WtpID);//zhangshu add 2011-1-7
void wid_check_radio_max_min_channel(unsigned int RadioID,unsigned int *max_channel,unsigned int *min_channel);//fengwenchao add 20110421
int wid_set_ap_statistics_v1(int WTPID,int apstatics);  //fengwenchao add 20110422
CWBool wid_multicast_listen_setting(CWMultiHomedSocket *sockPtr, int port);
CWBool wid_multicast_listen_close(CWMultiHomedSocket *sockPtr);
int wid_radio_get_11n_rate_paras(struct n_rate_info *nRateInfo, unsigned char stream_num);
int wid_radio_set_11n_rate_paras(int ID, struct n_rate_info nRateInfo, unsigned char chan);
CWBool check_wtpid_func(unsigned int WTPID);
CWBool check_wlanid_func(unsigned int WLANID);
CWBool check_bssid_func(unsigned int BSSID);
CWBool check_g_radioid_func(unsigned int RADIOID);
CWBool check_l_radioid_func(unsigned int RADIOID);
int Delete_listenning_IF(char * ifname);
int wid_update_wtp_bss_infov2(int wtpid,unsigned int BSSIndex);
CWBool oui_mac_filters(unsigned char *mac);
void wid_set_wds_state(unsigned int wtpid, unsigned char radioid, unsigned char wlanid, unsigned char state);
void wid_apstatsinfo_init(unsigned int WTPID);
int wid_prase_heart_time_avarge(unsigned int wtpid);
int wid_count_countermeasure_rogue_ap(Neighbor_AP_INFOS *paplist,int wtpid);
int wid_modify_legal_essid(char *essid,char *essid_new);
CWBool wid_change_mac_whitelist(unsigned char * pmac,unsigned char *pmacdest);
CWBool wid_change_mac_blacklist(unsigned char * pmac,unsigned char *pmacdest);
int wid_set_ap_scanning_wtp(unsigned int wtpid,APScanningSetting scansetting,unsigned char mode);
int check_channel(int check_channel);
int measure_quality_of_network_link(char *wtpip, struct NetworkQuality *networkquality);
int wid_set_country_code_a8();
int Check_Listenning_If(char* ifname,struct ifi_info *ifi,char flag);
int DELETE_LISTENNING_INTERFACE(char *ifname);
int wid_set_ap_statistics_interval(unsigned int wtpid,unsigned int apstatics_interval);
int WID_ADD_IF_APPLY_WLAN_ipv6(unsigned char WlanID, char * ifname);
int parse_mac_addr(char* input,MACADDR* macAddr) ;
int wid_radio_set_whole_wlan_station_average_traffic_limit_value(unsigned char wlanid,unsigned int value,unsigned char issend);
int WID_BINDING_IF_APPLY_WTP_ipv6(unsigned int WtpID, char * ifname);
int WID_RADIO_CHANNEL_OFFSET_CWMODE_CHECK(unsigned int RadioID, unsigned int check_channel,unsigned int max_chanenl,unsigned int min_channel);
int wid_radio_set_guard_interval(unsigned int RadioID);
int wid_radio_set_mixed_puren_switch(unsigned int RadioID);
int wid_radio_set_channel_Extoffset(unsigned int RadioID);
int wid_radio_set_mcs(unsigned int RadioID);
int wid_radio_set_cmmode(unsigned int RadioID);
int wid_wds_remote_bridge_mac_op(int RadioID, int is_add, unsigned char *mac);
int wid_wds_remote_bridge_mac_set_aes_key(int RadioID, unsigned char *mac, char *key);
int WID_WDS_BSSID_OP(unsigned int RadioID, unsigned char WlanID, unsigned char *MAC, unsigned char OP);
int wid_set_qos_flow_able_value(unsigned int qosid,unsigned char streamid,unsigned int able_type,unsigned int flag);
int wid_send_to_ap_Terminal_Disturb_info(unsigned int wtpid);
int wid_radio_set_inter_vap_forwarding_able(unsigned int wtpid,unsigned int l_radioid,unsigned char policy);
int wid_radio_set_keep_alive_period_value(unsigned int wtpid,unsigned int l_radioid,unsigned int idle_period);
int wid_radio_set_congestion_avoid_state(unsigned int wtpid,unsigned int l_radioid,unsigned int congestion_av_state);
int wid_radio_set_chainmask(unsigned int RadioID, unsigned char type); // zhangshu add for set chainmask
int wid_set_radio_netgear_supper_g_technology_state(unsigned int wtpid,	unsigned int l_radioid,unsigned short supper_g_type,unsigned int supper_g_state);
CWBool check_radio_bind_wlan(unsigned int wtpid,unsigned char radio_l_id,unsigned char wlanId);
int br_read_fdb(const char *bridge, struct fdb_entry *fdbs,unsigned long offset, int num);
int create_ac_ip_list_group(unsigned char ID,char *IFNAME);
int wid_radio_set_intra_vap_forwarding_able(unsigned int wtpid,unsigned int l_radioid,unsigned char policy);
int wid_radio_set_keep_alive_idle_time_value(unsigned int wtpid,unsigned int l_radioid,unsigned int idle_time);
int delete_ac_ip_list_group(unsigned char ID);
int add_ac_ip(unsigned char ID, char * ip, unsigned char priority);
int delete_ac_ip(unsigned char ID,char *ip);
int set_ac_ip_priority(unsigned char ID, char * ip, unsigned char priority);
int set_ac_ip_load_banlance(unsigned char ID, unsigned char load_banlance);
int  set_ac_ip_diff_banlance(unsigned char ID, unsigned int diff_banlance);
int set_ac_ip_diffcount(unsigned char ID, char * ip, unsigned int diffcount);
int create_ap_group(unsigned int ID,char *NAME);
int delete_ap_group(unsigned int ID);
int wid_set_ap_eth_if_mtu(unsigned int wtpid,unsigned char eth_index);
void WID_WTP_SSID_KEY_CONFLICT(unsigned int RadioID,unsigned char WlanID);
CWBool WIDWsm_VRRPIFOp_IPv6(struct ifi_info *ifi, unsigned int op);
int wid_dbus_trap_wtp_neighbor_channel_ap_interference(int wtpindex,char chchannel,unsigned char mac[6],unsigned char flag);
int parse_radio_ifname(char* ptr,int *wtpid,int *radioid,int *wlanid);
int parse_radio_ifname_v2(char * ptr,int * wtpid,int * radioid,int * wlanid,unsigned int * vrrid);

int add_mac_in_maclist(struct acl_config *conf, unsigned char *addr, char type);
int del_mac_in_maclist(struct acl_config *conf, unsigned char  *addr, char type);
int change_maclist_security(struct acl_config *conf, char type);
int WID_RADIO_SET_COUNTRYCODE(unsigned int RadioID);
int WID_SET_COUNTRY_CODE_CHECK_CHAN(unsigned int RadioID);
int WLAN_FLOW_CHECK(unsigned char WlanID);
unsigned int getfilesize(char *file_path);
int WIDCheckFreeMem(unsigned int fileLen, char * filePath);
int WID_ADD_WLAN_APPLY_RADIO_CLEAN_NAS_PORT_ID(unsigned int RadioID,unsigned char WlanID);
int WID_ADD_WLAN_APPLY_RADIO_BASE_HOTSPOT_ID(unsigned int RadioID,unsigned char WlanID,unsigned int  hotspot_id);
int WID_ADD_WLAN_APPLY_RADIO_CLEAN_HOTSPOT_ID(unsigned int RadioID,unsigned char WlanID);
int wid_radio_set_mcs_list(unsigned int RadioID);
int check_ac_whether_or_not_set_mcs_list(unsigned int WTPIndex,unsigned int l_radioid);
void Delete_Interface(char *ifname, int ifindex);
int WID_RADIO_WLAN_TUNNEL_MODE(unsigned char WlanID,unsigned char Policy,char nodeFlag);
int setWtpNoRespToStaProReq(unsigned int wtpid,unsigned char l_radioid,unsigned char wlanid,unsigned int policy);
int setWtpUniMutiBroCastIsolation(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned char policy);
int setWtpUniMutiBroCastRate(unsigned int wtpid,unsigned char radioid,unsigned char wlanid,unsigned int rate);
int uni_muti_bro_cast_rate_check(unsigned int wtpindex,unsigned int radioid,unsigned int rate);
int wid_set_ap_username_password(unsigned int wtpid,char *username,char *passwd);
void muti_user_optimize_switch(unsigned char wlanid, unsigned int radio_g_id,unsigned char type);
int set_active_ac_listenning();
int set_bakup_ac_update_license();
int Add_Listenning_IP(char * ifname,unsigned int addr,LISTEN_FLAG flag);
int Delete_listenning_IP(unsigned int ipaddr,LISTEN_FLAG flag);
int Check_Listenning_Ip(char* ifname,unsigned int addr,LISTEN_FLAG flag,char nl_flag);
int DELETE_LISTENNING_IPADDR(unsigned int ipaddr,LISTEN_FLAG flag);
int Set_Interface_binding_Info(char * ifname,char flag);//fengwenchao copy from 1318 for AXSSZFI-839
int wid_send_to_ap_sta_deauth_report(unsigned int wtpid);
int wid_send_to_ap_sta_flow_information_report(unsigned int wtpid);

int set_wlan_tunnel_mode(unsigned char WlanID, unsigned char state);
void set_wtp_5g_switch(unsigned int wtpid,unsigned char type);
int wid_set_wlan_hotspotid(unsigned char Wlanid,unsigned int hotspotid);


int read_board_ap_max_counter(unsigned int * count);//fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
#endif

