#ifndef _WID_DBUS_PATH_H
#define _WID_DBUS_PATH_H

char WID_DBUS_BUSNAME[PATH_LEN]=	"aw.wid";
char WID_DBUS_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_INTERFACE[PATH_LEN]="aw.wid";
/*char WID_DBUS_SUCCESS 0
char WID_DBUS_ERROR	1
char WID_DBUS_NO_WLAN_ID	2
char WID_DBUS_NO_WTP_ID	3
char WID_DBUS_NO_Radios_ID	4
char WLAN_ID_BE_USED	5
char WLAN_ID_NOT_EXIST	6
char WTP_ID_BE_USED	7
char WTP_ID_NOT_EXIST	8
*/

/*
 arg lists for method WID_DBUS_CONF_METHOD_SHOWWLAN
  in arg list:
	BYTE WlanID //ID of WLAN which is unique sign of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	STRING 	WLANNAME//name of wlan
	BYTE 	WLANID//ID of WLAN which is unique sign of wlan
	STRING 	ESSID//essid of wlan
	BYTE	AAW//aute add wtp 0(disable)/1(default enable)
	BYTE	STATUS//WLAN Status 0(default disable)/1(enable)
	BYTE	VID//num of vlan which wlan in
	BYTE	Wlan_Index
*/
char WID_DBUS_CONF_METHOD_SHOWWLAN[PATH_LEN]="show_wlan";
char WID_DBUS_CONF_METHOD_SHOWWLAN_OF_ALL[PATH_LEN]= "show_wlan_of_all";  //fengwenchao add 20101223

char WID_DBUS_CONF_METHOD_WLANLIST[PATH_LEN]=	"show_wlan_list";

char WID_DBUS_CONF_METHOD_WTPLIST[PATH_LEN]=	"show_wtp_list";
char WID_DBUS_CONF_METHOD_WTPLIST_BYMAC[PATH_LEN]=	"show_wtp_list_bymac";
char WID_DBUS_CONF_METHOD_WTPLIST_NEW[PATH_LEN]=	"show_wtp_list_new";
char WID_DBUS_CONF_METHOD_WTPLIST_NEW_BYINTERFACE[PATH_LEN]=	"show_wtp_list_new_byinterface";	//xiaodawei add, 20110301
char WID_DBUS_CONF_METHOD_SHOWWTP_BYMODEL[PATH_LEN]=  "show_wtp_list_bymodel";      //fengwenchao add 20110226
char WID_DBUS_CONF_METHOD_SHOWWTP_BYVERSION[PATH_LEN]= "show_wtp_list_byversion";   //fengwenchao add 20110314
char WID_DBUS_CONF_METHOD_WTPLIST_UPDATE[PATH_LEN]=	"show_wtp_list_update";
char WID_DBUS_CONF_METHOD_AC_ACCESS_WTPLIST[PATH_LEN]=	"show_ac_access_wtp_list";
char WID_DBUS_CONF_METHOD_CLEAN_AC_ACCESS_WTPLIST[PATH_LEN]=	"clean_ac_access_wtp_list";
char WID_DBUS_CONF_METHOD_OLD_AP_IMG[PATH_LEN]=		"old_ap_img_op";

char  WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_MTU[PATH_LEN] =	"set_ap_interface_eth_mtu";   //fengwenchao add 20110126

char WID_DBUS_CONF_METHOD_ACVERSION[PATH_LEN]=	"show_ac_version";
char WID_DBUS_CONF_METHOD_ECHOTIMER[PATH_LEN]=	"show_ap_echotimer";
char WID_DBUS_CONF_METHOD_SHOW_WTP_RUNNING_CONFIG[PATH_LEN]=	"show_wtp_running_config";

char WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START[PATH_LEN]=	"wlan_show_running_config_start";

char WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END[PATH_LEN]=	"wlan_show_running_config_end";

char WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_START[PATH_LEN]=	"wtp_show_running_config_start";

char WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_END[PATH_LEN]=	"wtp_show_running_config_end";
/*
 arg lists for method WID_DBUS_CONF_METHOD_WLAN
  in arg list:
	BYTE WlanID //ID of WLAN which is unique sign of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_CONF_METHOD_WLAN[PATH_LEN]=	"wlan";


/*
 arg lists for method WID_DBUS_CONF_METHOD_SHOWWTP
  in arg list:
	BYTE WTPID //ID of WTP which is unique sign of wtp
  out arg list:  // in the order as they are appended in the dbus message.
	STRING 	WTPNAME//name of wtp
	BYTE 	WTPID//ID of wtp which is unique sign of wtp
	UNIT32	WTPSN//sn of wtp
	UNIT32	WTPMOD//model of wtp
	STRING	WTPIP//ip of wtp
	STRING	WTPMAC//mac of wtp
	BYTE	CTR_ID//id of control tunnel
	BYTE	DTA_ID//id of data tunnel
*/
char	WID_DBUS_CONF_METHOD_SHOWWTP[PATH_LEN]=	"show_wtp";
char    WID_DBUS_CONF_METHOD_SHOW_CONFIG_WTP[PATH_LEN]= "show_wtp_config_of_all_wtp"; //fengwenchao add 20101223

char	WID_DBUS_WTP_METHOD_SHOW_WTP_MODEL_CODE_VERSION[PATH_LEN]=	"show_wtp_model_code_version";


/*
 arg lists for method WID_DBUS_CONF_METHOD_WTP
  in arg list:
	BYTE WTPID //ID of WLAN which is unique sign of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_CONF_METHOD_WTP[PATH_LEN]=	"wtp";


/*
 arg lists for method WID_DBUS_CONF_METHOD_SHOWRADIO
  in arg list:
	BYTE Radio_G_ID //global ID of radio which is unique sign of radio
  out arg list:  // in the order as they are appended in the dbus message.
	BYTE	WTPID
	BYTE	Radio_G_ID
	BYTE	Radio_L_ID
	UNIT32	Radio_TYPE//type of Radio(a/b/g/n)
	BYTE	Radio_Chan//channel of radio
	UNIT16	Radio_TXP//tx power of radio
	BYTE	AdStat//state of admin
	BYTE	OpStat//state of operate
*/
char	WID_DBUS_CONF_METHOD_SHOWRADIO[PATH_LEN]=	"show_radio";

char WID_DBUS_CONF_METHOD_RADIOLIST[PATH_LEN]=	"show_radio_list";
char WID_DBUS_CONF_METHOD_RADIO[PATH_LEN]=		"radio";

/*
 arg lists for method WID_DBUS_CONF_METHOD_ADD_DEL_WLAN
  in arg list:
	BYTE	isADD //determine ADD(1) or DEL(0)	
  	STRING  WLANNAME//name of wlan
  	BYTE	  WLANID//ID of WLAN which is unique sign of wlan
  	STRING  ESSID//essid of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char	WID_DBUS_CONF_METHOD_ADD_DEL_WLAN[PATH_LEN]=	"add_del_wlan";
char	WID_DBUS_CONF_METHOD_ADD_DEL_WLAN_CN[PATH_LEN]=		"add_del_wlan_cn";

/*
 arg lists for method WID_DBUS_CONF_METHOD_ADD_DEL_WTP
  in arg list:
	BYTE	isADD //determine ADD(1) or DEL(0)	
	STRING 	WTPNAME//name of wtp
	BYTE 	WTPID//ID of wtp which is unique sign of wtp
	UNIT32	WTPSN//sn of wtp
	UNIT32	WTPMOD//model of wtp
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char	WID_DBUS_CONF_METHOD_ADD_DEL_WTP[PATH_LEN]=	"add_del_wtp";

char	WID_DBUS_CONF_METHOD_ADD_DEL_WTP_BY_MAC[PATH_LEN]=	"add_del_wtp_by_mac";


char WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT[PATH_LEN]=	"update_wtp_count";
	
char	WID_DBUS_WLAN_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_WLAN_INTERFACE[PATH_LEN]=	"aw.wid";


/*
arg lists for method WID_DBUS_CONF_METHOD_WLAN_ENABLE
 in arg list:
   	BYTE	WlanID//id of wlan which you want enable or disable
  	BYTE	DisWLAN //determine disable(1) or enable(0)WLAN  
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_WLAN_METHOD_ENABLE[PATH_LEN]=	"wlan_enable";
char WID_DBUS_WDS_METHOD_ENABLE[PATH_LEN]=	"wds_enable";


/*
arg lists for method WID_DBUS_CONF_METHOD_WLAN_APPLY_IF
 in arg list:
   	BYTE	WlanID//id of wlan which you want enable or disable
  	STRING	interface//which interface wlan apply  
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_WLAN_METHOD_APPLY_IF[PATH_LEN]=	"wlan_apply_if";

char WID_DBUS_WTP_METHOD_APPAY_WTP_IFNAME[PATH_LEN]=  "apply_wtp_ifname";
char WID_DBUS_WTP_METHOD_APPAY_WTP_MAX_STA[PATH_LEN]=   "wtp_max_sta";
char WID_DBUS_WTP_INTERFACE_CHECK_WTP_STA[PATH_LEN]= "check_wtp_sta";
char WID_DBUS_WTP_METHOD_APPAY_WTP_TRIGER[PATH_LEN]=  "wtp_triger";
char WID_DBUS_WTP_METHOD_APPAY_WTP_FLOW_TRIGER[PATH_LEN]=   "wtp_flow_triger";

char WID_DBUS_WLAN_METHOD_FLOW_CHECK[PATH_LEN] = "wlan_flow_check";			//weichao add 2011.10.28
char WID_DBUS_WLAN_METHOD_NO_FLOW_TIME[PATH_LEN] = "wlan_no_flow_time";			//weichao add 2011.10.31
char WID_DBUS_WLAN_METHOD_LIMIT_MIN_FLOW[PATH_LEN] = "wlan_limit_min_flow";		//weichao add 2011.10.31

char WID_DBUS_WTP_METHOD_APPAY_WTP_WLANID[PATH_LEN]=	"apply_wtp_wlanid";
char WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME[PATH_LEN]=	"apply_wlan_ifname";

char WID_DBUS_WLAN_METHOD_APPAY_WLAN_MAX_STA[PATH_LEN]=	"wlan_max_sta";
char WID_DBUS_WLAN_METHOD_WLAN_MAX_STA_NEW[PATH_LEN]= "wlan_max_sta_new";/*fengwenchao add 20120323*/
char WID_DBUS_WLAN_METHOD_WLAN_L2_ISOLATION[PATH_LEN]=  "wlan_l2_isolation"; /*fengwenchao add 20120323*/
char WID_DBUS_WLAN_METHOD_WLAN_STA_STATIC_ARP[PATH_LEN]= "wlan_sta_static_arp"; /*fengwenchao add 20120323*/
char WID_DBUS_WLAN_METHOD_WLAN_ACCESS_STA_LIMIT_RSSI[PATH_LEN]= "wlan_access_sta_limit_rssi";  /*fengwenchao add 20120323*/
char WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_PARA[PATH_LEN]=	"wlan_balance_para"; /*xm  08/12/29*/
char WID_DBUS_WLAN_METHOD_APPAY_WLAN_FLOW_BALANCE_PARA[PATH_LEN]=	"wlan_flow_balance_para" ;/*xm  09/02/05*/

char WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_SWITCH[PATH_LEN]=	"wlan_balance_switch"; /*xm  08/12/29*/
char WID_DBUS_WTP_METHOD_WTP_USED[PATH_LEN]=            "wtp_used";
/*added by weiay 20080701*/
char WID_DBUS_WTP_METHOD_DELETE_WLAN_ID[PATH_LEN]=            "wtp_delete_wlan_id";
char WID_DBUS_WTP_METHOD_DISABLE_WLAN_ID[PATH_LEN]=            "wtp_disable_wlan_id";
char WID_DBUS_WTP_METHOD_ENABLE_WLAN_ID[PATH_LEN]=				"wtp_enable_wlan_id";
char WID_DBUS_WTP_METHOD_SHOW_WLAN_ID[PATH_LEN]=            "wtp_show_wlan_id";
char WID_DBUS_WTP_METHOD_SET_VERSION_PATH[PATH_LEN]=		"wtp_set_version_path";/*xm 08/08/29*/
char WID_DBUS_WTP_METHOD_CONFIG_PORT_ENABLE[PATH_LEN]=      "wtp_configure_port_enable";/*xm 08/09/01*/
char WID_DBUS_WTP_METHOD_CONFIG_PORT[PATH_LEN]=    		 "wtp_configure_port";/*xm 08/09/01*/
char WID_DBUS_WTP_METHOD_SET_WTPNAME[PATH_LEN]=    		 "wtp_set_wtpname";
char WID_DBUS_WTP_METHOD_SET_WTPSN[PATH_LEN]=    		 "wtp_set_wtpsn";

char WID_DEBUS_WLAN_METHOD_SET_EAP_MAC[PATH_LEN] =      "wtp_set_eap_mac";//zhangshu, 2010-10-22
/* zhangshu add for terminal distrub info, 2010-10-08 */
char WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_SWITCH[PATH_LEN]=      "set_terminal_distrub_infomation_switch";
char WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_PKT[PATH_LEN]=      "set_terminal_distrub_infomation_pkt";
char WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_STA_NUM[PATH_LEN]=      "set_terminal_distrub_infomation_sta_num";
char WID_DBUS_METHOD_SET_AP_HEART_STATISTICS_SWITCH[PATH_LEN]= "set_ap_heart_statistics_switch";  //fengwenchao add for GM-3,20111130
char WID_DBUS_METHOD_SET_AP_HEART_STATISTICS_COLLECT_TIME[PATH_LEN]= "set_ap_heart_statistics_collect_time"; //fengwenchao add for GM-3,20111130
char WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_SWITCH[PATH_LEN]=      "set_extension_infomation_switch";
char WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_REPORTINTERVAL[PATH_LEN]=      "set_extension_infomation_reportinterval";
char WID_DBUS_WTP_METHOD_SET_WTP_USERNAME_PASSWORD[PATH_LEN] = "set_ap_username_password";
char WID_DBUS_WTP_METHOD_SET_WTP_STA_DEAUTH_MESSAGE_REPORT_SWITCH[PATH_LEN]=      "set_sta_deauth_message_report_switch";
char  WID_DBUS_WTP_METHOD_SET_WTP_STA_ALL_FLOW_INFORMATION_REPORT_SWITCH[PATH_LEN]=	"set_sta_all_flow_information_report_switch";
char WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_SWITCH[PATH_LEN]=      "set_sta_infomation_switch";
char WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_REPORTINTERVAL[PATH_LEN]=      "set_sta_infomation_reportinterval";

char WID_DBUS_WTP_METHOD_SET_WTP_CHECK_WLANID[PATH_LEN]=      "check_wlan_id";

char WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL[PATH_LEN]=      "set_wtp_wids_interval";
char WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD[PATH_LEN]=      "set_wtp_wids_threshold";
char WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK[PATH_LEN]=      "set_wtp_wids_lasttime_in_black";
char WID_DBUS_WTP_METHOD_SET_AC_MASTER_BAK_CORRECT_WTP_STATE_TIMER[PATH_LEN]=   "set_ac_master_bak_correct_wtp_state_timer"; //fengwenchao add 20120117 for onlinebug-96
char WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_SWITCH[PATH_LEN]=      "set_ap_if_infomation_switch";
char WID_DBUS_WTP_METHOD_SET_AC_ACTIVE_BAK_STATE[PATH_LEN]=		"set_ac_active_bak_state";


char WID_DBUS_WTP_METHOD_SET_WTP_WIDS_SET[PATH_LEN]=      "set_ap_wids_set";
char WID_DBUS_WTP_METHOD_SHOW_WTP_WIDS_SET[PATH_LEN]=      "show_ap_wids_set";

char WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_REPORTINTERVAL[PATH_LEN]=      "set_ap_if_infomation_reportinterval";
char WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_UPDOWN[PATH_LEN]=      "set_ap_interface_updown";
char WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_RATE[PATH_LEN] =		"set_ap_interface_eth_rate";
char WID_DBUS_WTP_METHOD_SHOW_WTP_IF_INFO[PATH_LEN]=      "show_ap_interface_info";

char WID_DBUS_WTP_METHOD_SET_WTP_NTPCLIENT[PATH_LEN]=      "set_ap_ntpclient";
char WID_DBUS_WTP_METHOD_UPDATE_WTP_CONFIG[PATH_LEN]=      "update_ap_config";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT[PATH_LEN]=      "set_ap_reboot";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_ALL[PATH_LEN]=      "set_ap_reboot_all";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_WLANID[PATH_LEN]=      "set_ap_reboot_by_wlanid";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_INTERFACE[PATH_LEN]=      "set_ap_reboot_by_interface";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_MODEL[PATH_LEN]=      "set_ap_reboot_by_model";
char WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_LIST[PATH_LEN]=      "set_ap_reboot_by_list";

char WID_DBUS_WTP_METHOD_GET_WTP_BSS_PKT_INFO[PATH_LEN]=      "get_wtp_bss_pkt_info";
char WID_DBUS_WTP_METHOD_SHOW_WTP_BSS_PKT_INFO[PATH_LEN]=      "show_wtp_bss_pkt_info";
char WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO[PATH_LEN]=      "show_wtp_radio_pkt_info";
char WID_DBUS_WTP_METHOD_SET_AP_CHECKTIMER[PATH_LEN]=	   	"wtp_set_ap_checktimer";
char WID_DBUS_WTP_METHOD_SHOW_WTP_ETH_PKT_INFO[PATH_LEN]=      "show_wtp_eth_pkt_info";

char WID_DBUS_WTP_METHOD_SHOW_WTP_WIFI_SNR_INFO[PATH_LEN]=      "show_wtp_wifi_snr_info";

char WID_DBUS_WTP_METHOD_SET_PORT_VLAN_SECURITY[PATH_LEN]=    	"wtp_set_port_vlan_security";  /*sz20080827*/
char WID_DBUS_WTP_METHOD_SET_PORT_VLAN_ENABLE[PATH_LEN]=   	"wtp_set_port_vlan_enable";  /*sz20080827*/
char WID_DBUS_WTP_METHOD_SET_SECURITY[PATH_LEN]=   	        "wtp_set_security";  /*sz20080827*/
char WID_DBUS_WTP_METHOD_SET_VLAN_LIST_ENABLE[PATH_LEN]=   	  "wtp_set_vlan_list_enable";  /*sz20080827*/
char WID_DBUS_WTP_METHOD_DELETE_AP_MODEL[PATH_LEN]=		"wtp_delete_model";
char WID_DBUS_WTP_METHOD_SET_AP_TIMER[PATH_LEN]=	   	"wtp_set_ap_echotimer";
char WID_DBUS_WTP_METHOD_SET_AP_COLLECT_TIME[PATH_LEN]=		"wtp_set_ap_collect_time";/*nl add 20100713*/
char WID_DBUS_WLAN_METHOD_DELETE_IF[PATH_LEN]=            "wlan_delete_if";
char WID_DBUS_WLAN_METHOD_SHOW_IF[PATH_LEN]=            "wlan_show_if";
char WID_DBUS_WLAN_METHOD_HIDE_ESSID[PATH_LEN]=         "wlan_hide_essid";
char WID_DBUS_WLAN_METHOD_L3IF_POLICY[PATH_LEN]=         "wlan_l3if_policy";
char WID_DBUS_WLAN_METHOD_L3IF_POLICY2[PATH_LEN]=         "wlan_l3if_policy2";
char WID_DBUS_WLAN_METHOD_SET_NAS_IDENTIFIER[PATH_LEN]=    "set_interface_identifier";	
char WID_DBUS_WLAN_METHOD_REMOVE_NAS_IDENTIFIER[PATH_LEN]=    "remove_interface_identifier";
char WID_DBUS_WLAN_METHOD_ROAMING_POLICY[PATH_LEN]=		"wlan_roaming_policy"; /*zhanglei add*/
char WID_DBUS_WLAN_METHOD_SET_BRIDGE_ISOLATION[PATH_LEN]=		"wlan_set_bridge_isolation"; 
char WID_DBUS_WLAN_METHOD_SET_BRIDGE_MULTICAST_ISOLATION[PATH_LEN]=		"wlan_set_bridge_multicast_isolation"; 
char WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION[PATH_LEN]=		"wlan_show_bridge_isolation" ;
char WID_DBUS_WLAN_METHOD_SET_BRIDGE_SAMEPORTSWICTH[PATH_LEN]=		"wlan_set_bridge_sameportswitch"; 
char WID_DBUS_WLAN_METHOD_SET_ESSID[PATH_LEN]=		"wlan_set_essid" ;
char WID_DBUS_WLAN_METHOD_SET_ASCII_ESSID[PATH_LEN]=   "wlan_set_ascii_essid" ;   /*fengwenchao add 20110307*/
char WID_DBUS_WLAN_METHOD_WHOLE_WLAN_TRAFFIC_LIMIT_VALUE[PATH_LEN]=		"wlan_set_traffic_limit_value" ;
char WID_DBUS_WLAN_METHOD_WHOLE_WLAN_SEND_TRAFFIC_LIMIT_VALUE[PATH_LEN]=	"wlan_set_send_traffic_limit_value" ;


char WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_TRAFFIC_LIMIT_VALUE[PATH_LEN]=		"wlan_set_station_average_traffic_limit_value" ;
char WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_SEND_TRAFFIC_LIMIT_VALUE[PATH_LEN]=	"wlan_set_station_average_send_traffic_limit_value" ;//
char WID_DBUS_WLAN_METHOD_WHOLE_WLAN_BSS_MULTI_USER_OPTIMIZE_SWITCH[PATH_LEN] = "wlan_set_all_bss_multi_user_optimize_switch";



char WID_DBUS_WLAN_METHOD_UNDO_WLAN_VLANID[PATH_LEN]=		"wid_dbus_wlan_undo_wlan_vlanid";
char WID_DBUS_WLAN_METHOD_SET_WLAN_VLANID[PATH_LEN]=		"wid_dbus_wlan_set_wlan_vlanid";
char WID_DBUS_WLAN_METHOD_SET_NAS_PORT_ID[PATH_LEN] =		"wid_dbus_wlan_set_nas_port_id";		//mahz add 2011.5.25
char WID_DBUS_WLAN_METHOD_SET_WLAN_HOTSPOTID[PATH_LEN] =		"wid_dbus_wlan_set_hotspotid"	;		
char WID_DBUS_WLAN_METHOD_CLEAN_WLAN_HOTSPOTID[PATH_LEN] =		"wid_dbus_wlan_clean_hotspotid"	;		
char WID_DBUS_WLAN_METHOD_SET_TUNNEL_WLAN_VLAN[PATH_LEN]=		"wid_dbus_wlan_set_tunnel_wlan_vlan";
char WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN[PATH_LEN]=		"wid_dbus_wlan_show_tunnel_wlan_vlan";

char WID_DBUS_WLAN_METHOD_SET_WLAN_VLAN_PRIORITY[PATH_LEN]=		"wid_dbus_wlan_set_wlan_vlan_priority";
char WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO[PATH_LEN]=		"wid_dbus_wlan_show_wlan_vlan_info";
/*wlan for br area*/
char WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR[PATH_LEN]=         "wlan_l3if_policy_br";
char WID_DBUS_WLAN_METHOD_L3IF_POLICY2_BR[PATH_LEN]=         "wlan_l3if_policy2_br";

/*added end 20080701*/

char WID_DBUS_WTP_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_WTP_INTERFACE[PATH_LEN]=	"aw.wid";



char	WID_DBUS_RADIO_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_RADIO_INTERFACE[PATH_LEN]=	"aw.wid";

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_STXP
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	UNIT16	Radio_TXP//tx power of radio
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_RADIO_METHOD_SET_TXP[PATH_LEN]=	"radio_set_txpower";
char WID_DBUS_RADIO_METHOD_SET_TXPOF[PATH_LEN]=	"radio_set_txpowerof";

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_SCHAN
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	BYTE	Radio_Chan//channel of radio
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_RADIO_METHOD_SET_CHAN[PATH_LEN]=	"radio_set_channel";
/*added by weiay 20080714*/

char WID_DBUS_RADIO_METHOD_SET_RATE[PATH_LEN]=	"radio_set_rate";
char WID_DBUS_RADIO_METHOD_SET_MODE[PATH_LEN]=	"radio_set_mode";
char WID_DBUS_RADIO_METHOD_SET_FRAGMENTATION[PATH_LEN]=	"radio_set_fragmentation";
char WID_DBUS_RADIO_METHOD_SET_BEACON[PATH_LEN]=	"radio_set_beacon";
char WID_DBUS_RADIO_METHOD_SET_RTSTHROLD[PATH_LEN]=	"radio_set_hidessic";
char WID_DBUS_RADIO_METHOD_SET_DTIM[PATH_LEN]=	"radio_set_dtim";
char WID_DBUS_RADIO_METHOD_SET_STATUS[PATH_LEN]=	"radio_set_status";
char WID_DBUS_RADIO_METHOD_SET_WDS_STATUS[PATH_LEN]=	"wds_set_status";
char WID_DBUS_RADIO_METHOD_SET_PREAMBLE[PATH_LEN]=	"radio_set_preamble";
char WID_DBUS_RADIO_METHOD_SET_LONGRETRY[PATH_LEN]=	"radio_set_longretry";
char WID_DBUS_RADIO_METHOD_SET_SHORTRETRY[PATH_LEN]=	"radio_set_shortretry";
char WID_DBUS_RADIO_METHOD_SET_BSS_MAX_STA[PATH_LEN]=	"radio_set_max_bss_sta";



char WID_DBUS_RADIO_METHOD_SET_SUPPORT_RATELIST[PATH_LEN]=	"radio_set_support_ratelist";
char WID_DBUS_RADIO_METHOD_SET_MAX_RATE[PATH_LEN]=	"radio_set_max_rate";
char WID_DBUG_RADIO_METHOD_SET_11N_RATE_PARAS[PATH_LEN]=    "radio_set_11n_rate_paras";//book add
char WID_DBUS_RADIO_METHOD_SET_BSS_L3_POLICY[PATH_LEN]=	"radio_set_bss_l3_policy";
char WID_DBUS_RADIO_METHOD_SET_BSS_MAX_THROUGHPUT[PATH_LEN]=	"radio_set_bss_max_throughput";
char WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT[PATH_LEN]=	"radio_show_bss_max_throughput";
char WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE[PATH_LEN]=	"radio_show_channel_change";

char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID[PATH_LEN]=	"radio_apply_wlanid_base_vlanid";
char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID[PATH_LEN]=	"radio_apply_wlanid_clean_vlanid";
char WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER[PATH_LEN]= "check_radio_member";
char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_NAS_PORT_ID[PATH_LEN]= "radio_apply_wlanid_base_nas_port_id";		//mahz add 2011.5.30
char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_NAS_PORT_ID[PATH_LEN]= "radio_apply_wlanid_clean_nas_port_id";		//mahz add 2011.5.30
char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_HOTSPOT_ID[PATH_LEN] = "radio_apply_wlanid_base_hotspotid";
char WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_HOTSPOT_ID[PATH_LEN] = "radio_apply_wlanid_clean_hotspotid";

char WID_DBUS_RADIO_METHOD_SET_BSS_L3_IF_WLAN_BR[PATH_LEN]=	"set_bss_l3_if_wlan_br";
char WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS[PATH_LEN]=	"show_radio_qos";

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_APPLY_WLAN
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	BYTE	WlanID//id of wlan which radio apply
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
char WID_DBUS_RADIO_METHOD_APPLY_WLAN[PATH_LEN]=	"radio_apply_wlan";
char WID_DBUS_RADIO_METHOD_DELETE_WLAN[PATH_LEN]=	"radio_delete_wlan";
char WID_DBUS_RADIO_METHOD_ENABLE_WLAN[PATH_LEN]=	"radio_enable_wlan";
char WID_DBUS_RADIO_METHOD_DISABLE_WLAN[PATH_LEN]=	"radio_diable_wlan";

char WID_DBUS_RADIO_METHOD_WDS_WLAN_SET[PATH_LEN]=	"radio_wds_wlan_set";
char WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO[PATH_LEN]=	"show_radio_wds_bssid_list";

char WID_DBUS_RADIO_METHOD_REVOVER_DEFAULT_CONFIG[PATH_LEN]=	"radio_default_config";

char WID_DBUS_RADIO_METHOD_SET_MAX_THROUGHOUT[PATH_LEN]=	"set_radio_max_thoughout";
char WID_DBUS_RADIO_METHOD_SET_RADIO_L2_ISOLATION_ABLE[PATH_LEN]=          "set_radio_l2_isolation_able";
char WID_DBUS_RADIO_METHOD_11N_SET_RADIO_CWMMODE[PATH_LEN]=          "set_radio_11n_cwmmode";
char WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST[PATH_LEN]=	"show_radio_bss_list";

char WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL[PATH_LEN]=      "set_ap_radio_auto_channel";
char WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_DIVERSITY[PATH_LEN]=      "set_ap_radio_diversity";
char WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_TXANTENNA[PATH_LEN]=      "set_ap_radio_txantenna";
char WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL_CONT[PATH_LEN]=      "set_ap_radio_auto_channel_cont";
char WID_DBUS_RADIO_METHOD_INTER_VAP_FORVARDING_ABLE[PATH_LEN]=		"radio_inter_vap_forwarding_able";
char WID_DBUS_RADIO_METHOD_INTRA_VAP_FORVARDING_ABLE[PATH_LEN]=		"radio_intra_vap_forwarding_able";
char WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_PERIOD[PATH_LEN]=		"radio_set_keep_alive_period";
char WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_IDLE_TIME[PATH_LEN]=		"radio_set_keep_alive_idle_time";
char WID_DBUS_RADIO_METHOD_SET_CONGESTION_AVOID_STATE[PATH_LEN]=		"radio_set_congestion_avoid_state";



char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_ABLE[PATH_LEN]=	"radio_wlan_traffic_limit_able";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_average_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_sta_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_cancel_sta_value";

char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_SEND_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_send_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_SEND_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_average_send_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_sta_send_value";
char WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE[PATH_LEN]=	"radio_wlan_traffic_limit_cancel_sta_send_value";

char	WID_DBUS_QOS_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_QOS_INTERFACE[PATH_LEN]=	"aw.wid";

char WID_DBUS_QOS_METHOD_ADD_DEL_QOS[PATH_LEN]=		"add_del_qos";

char WID_DBUS_QOS_METHOD_SHOW_QOS_LIST[PATH_LEN]=	"show_qos_list";
char WID_DBUS_QOS_METHOD_SHOW_QOS[PATH_LEN]=		"show_qos";
char WID_DBUS_QOS_METHOD_CONFIG_QOS[PATH_LEN]=		"config_qos";

char WID_DBUS_QOS_METHOD_SET_QOS_INFO[PATH_LEN]=		"set_wid_radio_qos_info";
char WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT[PATH_LEN]=		"set_wid_client_qos_info";
char WID_DBUS_QOS_METHOD_SET_QOS_MAP[PATH_LEN]=		"set_wid_qos_wmm_map";
char WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P[PATH_LEN]=		"set_wid_qos_wmm_map_dot1p";
char WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM[PATH_LEN]=		"set_wid_qos_dot1p_map_wmm";

char WID_DBUS_RADIO_METHOD_APPLY_QOS[PATH_LEN]=	"radio_apply_qos";
char WID_DBUS_RADIO_METHOD_DELETE_QOS[PATH_LEN]=	"radio_delete_qos";
/*set wireless qos info for mib*/
char WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH[PATH_LEN]=		"set_wid_qos_total_bandwidth";
char WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO[PATH_LEN]=		"show_qos_extension_info";
char WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER[PATH_LEN]=		"set_wid_qos_flow_parameter";
char WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER[PATH_LEN]=		"set_wid_qos_parameter";
char WID_DBUS_CONF_METHOD_DELETE_RADIO_WITH_QOS_PROFILE[PATH_LEN]=  "delete_radio_with_qos_profile";   //fengwenchao add 20110427
char WID_DBUS_QOS_METHOD_SET_QOS_POLICY[PATH_LEN]=		"set_wid_qos_policy";
char WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME[PATH_LEN]=		"set_wid_qos_policy_name";
char WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME[PATH_LEN]=		"set_wid_qos_manage_arithmetic_name";
char WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO[PATH_LEN] = 	"show_radio_apply_qos_info";

/*ethereal bridge infomation area*/
char	WID_DBUS_EBR_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_DBUS_EBR_INTERFACE[PATH_LEN]=	"aw.wid";

char WID_DBUS_EBR_METHOD_ADD_DEL_EBR[PATH_LEN]=		"add_del_ebr";
char WID_DBUS_EBR_METHOD_SHOW_EBR_LIST[PATH_LEN]=	"show_ebr_list";
char WID_DBUS_EBR_METHOD_SHOW_EBR[PATH_LEN]=		"show_ebr";
char WID_DBUS_EBR_METHOD_CONFIG_EBR[PATH_LEN]=		"config_ebr";
char WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE[PATH_LEN]=		"config_ebr_enable";

char WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION[PATH_LEN]=		"ebr_set_bridge_isolation" ;
char WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION[PATH_LEN]=		"ebr_set_bridge_multicast_isolation" ;

char WID_DBUS_EBR_METHOD_SET_BRIDGE_UCAST_SOLICT[PATH_LEN] =	"set_bridge_ucast_solicit";			/* Huang Leilei add, 2012-11-12 11:24 */
char WID_DBUS_EBR_METHOD_SET_BRIDGE_MCAST_SOLICT[PATH_LEN] =	"set_bridge_mcast_solicit";			/* Huang Leilei add, 2012-11-12 20:09 */

char WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF[PATH_LEN]=		"ebr_set_ebr_add_del_if" ;
char WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH[PATH_LEN]=		"ebr_set_bridge_sameportswitch" ;
char WID_DBUS_EBR_METHOD_EBR_INTERFACE_EBR[PATH_LEN]=	"ebr_interface_ebr";
char WID_DBUS_EBR_METHOD_SET_MULTICAST_FDB_LEARN[PATH_LEN]= "ebr_set_multicast_fdb_learn";

char WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_START[PATH_LEN]=	"ebr_show_running_config_start";
char WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_END[PATH_LEN]=	"ebr_show_running_config_end";

/*
Wid config infomation 
*/

char WID_DBUS_CONF_METHOD_ADD_OUI[PATH_LEN]=          "add_legal_oui";
char WID_DBUS_CONF_METHOD_ADD_ESSID[PATH_LEN]=          "add_legal_essid";
char WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC[PATH_LEN]=          "add_attack_ap_mac";

char WID_DBUS_CONF_METHOD_DEL_OUI[PATH_LEN]=          "del_legal_oui";
char WID_DBUS_CONF_METHOD_DEL_ESSID[PATH_LEN]=          "del_legal_essid";
char WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC[PATH_LEN]=       "del_attack_ap_mac";

char WID_DBUS_CONF_METHOD_OUI_SHOW[PATH_LEN]=          "show_manufacturer_oui_list";
char WID_DBUS_CONF_METHOD_ESSID_SHOW[PATH_LEN]=          "show_legal_essid_list";
char WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW[PATH_LEN]=          "show_attack_mac_list";




char WID_DBUS_CONF_METHOD_WIDCONFIG[PATH_LEN]=       "show_wid_config"	;
char WID_DBUS_CONF_METHOD_LEV3_PROTOCOL[PATH_LEN]=    "set_wid_lev3_protocol"	;
char WID_DBUS_CONF_METHOD_SECURITY_TYPE[PATH_LEN]=    "set_wid_security_type";	
char WID_DBUS_CONF_METHOD_HW_VERSION[PATH_LEN]=       "set_wid_hw_version";	
char WID_DBUS_CONF_METHOD_SW_VERSION[PATH_LEN]=       "set_wid_sw_version";
char WID_DBUS_CONF_METHOD_AC_NAME[PATH_LEN]=          "set_wid_ac_name";

char WID_DBUS_CONF_METHOD_MAX_WTP[PATH_LEN]=          "set_wid_max_wtp";
char WID_DBUS_CONF_METHOD_MAX_MTU[PATH_LEN]=          "set_wid_max_mtu";
char WID_DBUS_CONF_METHOD_LOG_SWITCH[PATH_LEN]=          "set_wid_log_switch";
char WID_DBUS_CONF_METHOD_LOG_SIZE[PATH_LEN]=          "set_wid_log_size";
char WID_DBUS_CONF_METHOD_LOG_LEVEL[PATH_LEN]=          "set_wid_log_level";
char WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN[PATH_LEN]=          "set_wid_daemonlog_debug_open";

char WID_DBUS_CONF_METHOD_HNLOG_SWITCH_ACTIVATED[PATH_LEN]= "set_wid_log_hn_enable";//qiuchen add it for Henan mobile 2013.02.21
char WID_DBUS_CONF_METHOD_SET_AC_MANAGEMENT_IP[PATH_LEN]= "set_wid_ac_management_ip";
char WID_DBUS_CONF_METHOD_AP_SCANNING[PATH_LEN]=          "set_ap_scanning";
char WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL[PATH_LEN]=          "set_ap_scanning_report_interval";

char WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO[PATH_LEN]=          "update_ap_scanning_info";
char WID_DBUS_CONF_METHOD_WHITELIST[PATH_LEN]=          "set_mac_whitelist";
char WID_DBUS_CONF_METHOD_BLACKLIST[PATH_LEN]=          "set_mac_blacklist";
char WID_DBUS_CONF_METHOD_WHITELIST_DELETE[PATH_LEN]=          "delete_mac_whitelist";
char WID_DBUS_CONF_METHOD_BLACKLIST_DELETE[PATH_LEN]=          "delete_mac_blacklist";
char WID_DBUS_CONF_METHOD_ROGUE_AP_LIST[PATH_LEN]=          "show_rogue_ap_list";
char WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD[PATH_LEN]=			"show_rogue_ap_threshold";

char WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1[PATH_LEN]=          "show_rogue_ap_list_v1";
char WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID[PATH_LEN]=          "show_rogue_ap_list_bywtpid";
char WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION[PATH_LEN]=		"dynamic_channel_selection";
char WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION_RANGE[PATH_LEN]=		"dynamic_channel_selection_range";
char WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID[PATH_LEN]=          "show_neighbor_ap_list_bywtpid";
char WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST[PATH_LEN]=   "show_neighbor_ap_list";     //fengwenchao add 20101220
char WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2[PATH_LEN]=          "show_neighbor_ap_list_bywtpid2";
char WID_DBUS_CONF_METHOD_WHITELIST_SHOW[PATH_LEN]=          "show_mac_whitelist";
char WID_DBUS_CONF_METHOD_BLACKLIST_SHOW[PATH_LEN]=        "show_mac_blacklist";
/*country code area*/
char WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE[PATH_LEN]=          "set_system_country_code";
char WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE[PATH_LEN]=          "undo_system_country_code";
char WID_DBUS_CONF_METHOD_SHOW_COUNTRY_CODE[PATH_LEN]=          "show_system_country_code";
char WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL[PATH_LEN]=		"transmit_power_control";
char WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD[PATH_LEN]=		"set_txpower_threshold";
char WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD[PATH_LEN]=  "set_rogue_ap_threshold_trap";
char WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD[PATH_LEN]=		"set_coverage_threshold";
char WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO[PATH_LEN]=	"show_neighbor_rssi_info_bywtpid";
char WID_DBUS_CONF_METHOD_CONTROL_SCOPE[PATH_LEN]=    "set_transmit_power_control_scope"	;
/*auto ap area*/
char WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH[PATH_LEN]=	"set_auto_ap_login_switch";
char WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE[PATH_LEN]=	"set_auto_ap_login_binding_l3_interface";
char WID_DBUS_CONF_METHOD_SET_WID_LISTEN_L3_INTERFACE[PATH_LEN]=	"set_wid_listen_l3_interface";
char WID_DBUS_CONF_METHOD_SET_WID_LISTEN_IP[PATH_LEN]=	"set_wid_listen_ip";
char WID_DBUS_CONF_METHOD_SHOW_WID_LISTEN_L3_INTERFACE[PATH_LEN]=	"show_wid_listen_l3_interface";

char WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID[PATH_LEN]=	"set_auto_ap_login_binding_wlanid";
char WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID[PATH_LEN]=	"del_auto_ap_login_binding_wlanid";
char WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH[PATH_LEN]=	"set_auto_ap_login_save_config_switch";
char WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG[PATH_LEN]=	"show_auto_ap_login_save_config";
char WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG[PATH_LEN]=	"clear_auto_ap_login_config";
char WID_DBUS_CONF_METHOD_AP_DATA_EXCLUDE_MULTICAST[PATH_LEN]=    "set_ap_data_exclude_multicast";
char WID_DBUS_CONF_METHOD_AP_STATISTICS[PATH_LEN]=          "set_ap_statistics";
char WID_DBUS_CONF_METHOD_AP_STATISTICS_INTERVAL[PATH_LEN]=			"set_ap_statistics_interval";
char WID_DBUS_CONF_METHOD_AP_TIMESTAMP[PATH_LEN]=          "set_ap_timestamp";
char WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST[PATH_LEN]=      "show_ap_statistics_list";
char WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR[PATH_LEN]=      "show_ap_ip_bywtpid";
char WID_DBUS_CONF_METHOD_SET_WID_AP_IP_GATEWAY_DNS[PATH_LEN]= "set_ap_networkaddr";
char WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK[PATH_LEN] = "show_ap_network";
char WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE[PATH_LEN] = "set_ac_num_or_flow_balance";
char WID_DBUS_CONF_AC_LOAD_BALANCE_PARA[PATH_LEN] = "set_ac_balance_para";



char WID_DBUS_CONF_METHOD_SET_WID_AC_EXTENTION_INFOR_ENABLE[PATH_LEN] = "set_ac_extention_infor_enable";

char WID_DBUS_CONF_METHOD_SET_WID_AP_MAX_THROUGHOUT[PATH_LEN]=	"set_ap_max_thoughout";

char WID_DBUS_CONF_METHOD_SET_WID_AP_EXTENSION_COMMAND[PATH_LEN]=	"set_ap_extension_command";
char WID_DBUS_CONF_METHOD_SET_WID_AP_OPTION60_PARAMETER[PATH_LEN] =  "set_ap_special_command";
char WID_DBUS_CONF_METHOD_SET_WID_AP_IP_GATEWAY[PATH_LEN]= "set_ap_ipaddr_command_cmdset_ap_ipaddr_command_cmd";
char WID_DBUS_CONF_METHOD_TCPDUMP_AP_EXTENSION_COMMAND[PATH_LEN]="tcpdump";

/*mib area*/
char WID_TRAP_OBJPATH[PATH_LEN]=	"/aw/trap";
char WID_TRAP_INTERFACE[PATH_LEN]=	"aw.trap";

char WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE[PATH_LEN]=		"wid_wtp_enter_imagedata_state";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE[PATH_LEN]=		"wid_wtp_channel_change";
char WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE[PATH_LEN]=		"wid_wlan_encryption_type_change";
char WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE[PATH_LEN]=		"wid_wlan_preshared_key_change";
char WID_DBUS_TRAP_WID_WTP_TRANFER_FILE[PATH_LEN]=		"wid_wtp_transfer_file";
char WID_DBUS_TRAP_WID_WTP_UPDATE_SUCCESSFUL[PATH_LEN]=   "wid_wtp_update_successful";  /*fengwenchao add 20110216*/
char WID_DBUS_TRAP_WID_WTP_UPDATE_FAIL[PATH_LEN]=    "wid_wtp_update_fail";  /*fengwenchao add 20110216*/

char WID_DBUS_TRAP_WID_WTP_CODE_START[PATH_LEN]=		"wid_dbus_trap_wtp_code_start";
char WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE[PATH_LEN]=		"wid_dbus_trap_wtp_electrify_register_circle";
char WID_DBUS_TRAP_WID_WTP_AP_DOWN[PATH_LEN]=		"wid_dbus_trap_wtp_ap_power_off";
char WID_DBUS_TRAP_WID_WTP_AP_REBOOT[PATH_LEN]=		"wid_dbus_trap_wtp_ap_reboot";
char WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM[PATH_LEN]=		"wid_dbus_trap_wtp_ip_change_alarm";
char WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE[PATH_LEN]=		"wid_dbus_trap_wtp_ap_ACTimeSynchroFailure";
char WID_DBUS_TRAP_WID_WTP_DEVICE_INTERFERENCE_DETECTED[PATH_LEN]=		"wid_dbus_trap_wtp_device_interference";
char WID_DBUS_TRAP_WID_WTP_SHOW_RUNTIME[PATH_LEN]=		"wid_dbus_trap_wtp_show_runtime";
char WID_DBUS_TRAP_WID_WTP_REMOTE_RESTART[PATH_LEN]=		"wid_dbus_trap_wtp_remote_restart";
char WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK[PATH_LEN]=		"wid_dbus_trap_wtp_divorce_network";
char WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL[PATH_LEN]=		"wid_dbus_trap_wtp_ap_flash_write_fail";

char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_RUNTIME[PATH_LEN]=		"wid_dbus_show_wtp_runtime";
char WID_DBUS_CONF_METHOD_SET_WID_WTP_LOCATION[PATH_LEN]=		"wid_dbus_set_wtp_location";
char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_LOCATION[PATH_LEN]=		"wid_dbus_show_wtp_location";
char WID_DBUS_CONF_METHOD_SET_WID_WTP_NETID[PATH_LEN]=		"wid_dbus_set_wtp_netid";
char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_NETID[PATH_LEN]=		"wid_dbus_show_wtp_netid";
char WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME[PATH_LEN]=		"wid_dbus_set_monitor_time";
char WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME[PATH_LEN]=		"wid_dbus_set_sample_time";
char WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO[PATH_LEN]=		"wid_dbus_show_sample_info";
char WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE[PATH_LEN]=		"wid_dbus_set_monitor_enable";
char WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE[PATH_LEN]=		"wid_dbus_set_sample_enable";
char WID_DBUS_CONF_METHOD_SET_WID_IPFWD[PATH_LEN]=		"wid_dbus_set_ipfwd";
char WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD[PATH_LEN]=     "wid_dbus_show_ipfwd";

char WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER[PATH_LEN]=	"wid_set_ap_update_timer";
char WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER[PATH_LEN]=	"wid_show_ap_update_timer";

char WID_DBUS_CONF_METHOD_SET_AP_L2_ISOLATION_ABLE[PATH_LEN]=          "set_ap_l2_isolation_able";
char WID_DBUS_CONF_METHOD_SET_AP_PREVENT_DOS_ATTACK[PATH_LEN]=          "set_ap_prevent_dos_attack";
char WID_DBUS_CONF_METHOD_SET_AP_IGMP_SNOOPING[PATH_LEN]=          "set_ap_igmp_snooping";
char WID_DBUS_CONF_METHOD_SHOW_AP_MIB_INFO[PATH_LEN]=          "show_ap_mib_info";

char WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD[PATH_LEN]=          "set_ap_cm_threshold";
char WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD[PATH_LEN]=          "show_ap_cm_threshold";
char WID_DBUS_CONF_METHOD_SHOW_AP_CM_STATISTICS[PATH_LEN]=          "show_ap_cm_statistics";
char WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION[PATH_LEN]=          "show_ac_balance_configuration";

char WID_DBUS_CONF_METHOD_AP_CHANNEL_DISTURB_TRAP[PATH_LEN]=          "set_ap_channel_disturb_trap";
char WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION[PATH_LEN]=     "show_ap_model_infomation";
char WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION[PATH_LEN]=      "show_ap_model_code_infomation";

char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION[PATH_LEN]=      "show_ap_extension_infomation";
char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V2[PATH_LEN]=      "show_ap_extension_infomation_v2";
char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V3[PATH_LEN]=      "show_ap_extension_infomation_v3";
char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V4[PATH_LEN]=      "show_ap_extension_infomation_v4";

char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_MAX_POWER[PATH_LEN]=      "show_ap_max_power";

char WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG[PATH_LEN]=      "show_rrm_config";
char WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL[PATH_LEN]=      "show_ap_txpower_control";
char WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL[PATH_LEN]=      "set_receiver_signal_level";
char WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL[PATH_LEN]=      "show_receiver_signal_level";

char WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE[PATH_LEN]=		"wid_dbus_trap_wtp_channel_device_interference";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE[PATH_LEN]=		"wid_dbus_trap_wtp_channel_ap_interference";
char WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE[PATH_LEN] =  "wid_dbus_trap_wtp_neighbor_channel_ap_interference";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE[PATH_LEN]=		"wid_dbus_trap_channel_terminal_interference";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR[PATH_LEN]=		"wid_dbus_trap_channel_count_minor";

char WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP[PATH_LEN]=		"wid_dbus_trap_wtp_ac_discovery_danger_ap";
char WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE[PATH_LEN]=		"wid_dbus_trap_wtp_ac_discovery_cover_hole";

char WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD[PATH_LEN]=		"wid_dbus_trap_ap_cpu_threshold";
char WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD[PATH_LEN]=		"wid_dbus_trap_ap_mem_threshold";
char WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD[PATH_LEN]=		"wid_dbus_trap_ap_temp_threshold";
char WID_DBUS_TRAP_WID_AP_ROGUE_THRESHOLD[PATH_LEN]=		"wid_dbus_trap_ap_rogue_threshold";
char WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR[PATH_LEN]=		"wid_dbus_trap_ap_wifi_if_error";
char WID_DBUS_TRAP_WID_AP_ATH_ERROR[PATH_LEN]=		"wid_dbus_trap_ap_ath_error";
char WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE[PATH_LEN]=		"wid_dbus_trap_ap_rrm_state_change";
char WID_DBUS_TRAP_WID_AP_RUN_QUIT[PATH_LEN]=		"wid_dbus_trap_ap_run_quit";

char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_SAMPLE_THROUGHPUT_INFO[PATH_LEN]=		"wid_dbus_show_sample_throughput_info";
char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_WLAN_VLAN_INFO[PATH_LEN]=		"wid_dbus_show_wlan_vlan_info";
char WID_DBUS_CONF_METHOD_SHOW_WID_WTP_ETH_IF_INFO[PATH_LEN]=		"wid_dbus_show_eth_if_info";
char WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG[PATH_LEN]=   "set_wid_watch_dog"	;

/* add for sample  begin*/
char WID_DBUS_WTP_METHOD_SET_WTP_MOMENT_INFOMATION_SWITCH[PATH_LEN]=   "set_moment_infomation_switch";  
char WID_DBUS_WTP_METHOD_SET_WTP_MOMENT_INFOMATION_REPORTINTERVAL[PATH_LEN]=    "set_moment_infomation_reportinterval";
char WID_DBUS_WTP_METHOD_SET_WTP_SAMPLE_INFOMATION_REPORTINTERVAL[PATH_LEN]=	"set_sample_infomation_reportinterval";
char WID_DBUS_WTP_METHOD_SHOW_WTP_ROUTINE_INFOMATION_REPORTINTERVAL[PATH_LEN]=	"set_routine_infomation_reportinterval";
char WID_DBUS_WTP_METHOD_SHOW_WTP_MOMENT_INFOMATION_REPORTINTERVAL[PATH_LEN]=	"show_moment_infomation_reportinterval";
/* add for sample  end*/
/*******************************************MIB optimize BEGIN*****************************************************/
/*add  for showing wtp information for mib need 20100702 by nl*/
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_BASIC_INFORMATION[PATH_LEN]=		"show_all_wtp_basic_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_COLLECT_INFORMATION[PATH_LEN]=		"show_all_wtp_collect_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_PARA_INFORMATION[PATH_LEN]=		"show_all_wtp_para_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_IFSTATS_INFORMATION[PATH_LEN]=		"show_all_wtp_wireless_ifstats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DEVICE_INFORMATION[PATH_LEN]=		"show_all_wtp_device_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DATA_PKTS_INFORMATION[PATH_LEN]=		"show_all_wtp_data_pkts_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_STATS_INFORMATION[PATH_LEN]=		"show_all_wtp_stats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_STATS_INFORMATION[PATH_LEN]=		"show_all_wlan_stats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_STATS_INFORMATION[PATH_LEN]=		"show_all_wlan_ssid_stats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_INFORMATION[PATH_LEN]=  "show_all_wtp_information";  /*fengwenchao add 20110617*/
char WID_DBUS_CONF_METHOD_SHOW_INFO_BYWLANID[PATH_LEN]=  "show_info_bywlanid";   /*fengwenchao add 20110617*/
char WD_DBUS_CONF_METHOD_SHOW_INFO_ALLWLAN[PATH_LEN]=    "show_info_allwlan";   /*fengwenchao add 20110617*/
char WID_DBUS_CONF_METHOD_SHOW_INFO_BYWTPID[PATH_LEN]=   "show_info_bywtpid";/*fengwenchao add 20110617*/
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_IFNAME_INFORMATION[PATH_LEN]=		"show_all_wtp_ifname_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_PARA_INFOR_INFORMATION[PATH_LEN]=		"show_all_wtp_radio_para_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ETH_PORT_INFORMATION[PATH_LEN]=		"show_all_wtp_eth_port_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_STATS_INFORMATION[PATH_LEN]=		"show_all_wtp_radio_stats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_CONFIG_INFORMATION[PATH_LEN]=	"show_all_wtp_radio_config_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRED_STATS_INFORMATION[PATH_LEN]=	"show_all_wtp_wired_ifstats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_STATS_INFORMATION[PATH_LEN]=	"show_all_the_wtp_wireless_if_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NEW_WTPWIRELESS_IF_INFORMATION[PATH_LEN]=	"show_all_wtp_new_wireless_if_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_NEW_WTP_WIRELESS_IFSTATS_INFORMATION[PATH_LEN]=	"show_all_wtp_new_wireless_ifstats_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_ROGUE_WTP_INFORMATION[PATH_LEN]=		"show_all_rogue_ap_information";
char WID_DBUS_CONF_METHOD_HIDE_QUIT_WTP_INFOR_IN_MIB_SHOWING[PATH_LEN]= 	"hide_quit_wtp_infor_in_mib_showing";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NETWORK_INFO[PATH_LEN]=		"show_all_wtp_network_info";/*xdw add for show ap network information of all wtp, 20101215*/
/* zhangshu add 2010-08-30 */
char WID_DBUS_CONF_METHOD_SET_WTP_TRAP_IGNORE_PERCENT[PATH_LEN]= 	"set_wtp_trap_ignore_percent";
char WID_DBUS_CONF_METHOD_SHOW_HIDE_QUIT_WTP_INFOR_IN_MIB_SHOWING[PATH_LEN]= 	"show_hide_quit_wtp_info_in_mib_showing";

//mahz add 2011.1.21
char WID_DBUS_CONF_METHOD_SHOW_RADIO_INFO_BYWTPID_WID[PATH_LEN]=   "show_radio_info_bywtpid_wid";
char WID_DBUS_CONF_METHOD_BSS_ADD_MAC_LIST[PATH_LEN]=   "bss_add_mac_list";		//mahz add 2011.5.12
char WID_DBUS_CONF_METHOD_BSS_DEL_MAC_LIST[PATH_LEN]=   "bss_del_mac_list";
char WID_DBUS_CONF_METHOD_BSS_USE_MAC_LIST[PATH_LEN]=   "bss_use_mac_list";
char WID_DBUS_CONF_METHOD_SHOW_BSS_MAC_LIST[PATH_LEN]=   "show_bss_mac_list";

/*fengwenchao add 20110329*/
char WID_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE[PATH_LEN]=  "show_statistics_infomation_of_all_wtp_whole";


/*luzhenhua append  2010-05-26 */
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WLAN_DATA_PKTS_INFORMATION[PATH_LEN]=	"show_all_wtp_wlan_data_pkts_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ATH_STATISTICS_INFOMATION[PATH_LEN]=	"show_all_wtp_ath_statistics_information";
char WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION[PATH_LEN]= "show_wid_ssid_config_information_of_all_wtp";
char WID_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP[PATH_LEN] = "show_wid_terminal_info_of_all_wtp";

/*******************************************MIB optimize END********************************************************/
/*
char ASD_DBUS_SIG_STA_LEAVE	"signal_sta_leave"



char ASD_DBUS_SIG_STA_COME   "signal_sta_come"
char ASD_DBUS_SIG_WTP_DENY_STA   "signal_wtp_deny"
char ASD_DBUS_SIG_DE_WTP_DENY_STA   "signal_de_wtp_deny"


char ASD_DBUS_SIG_STA_VERIFY	"signal_sta_verify"
char ASD_DBUS_SIG_STA_VERIFY_FAILED   "signal_sta_verify_failed"


char ASD_DBUS_SIG_STA_ASSOC_FAILED   "signal_sta_assoc_failed"
char ASD_DBUS_SIG_STA_JIANQUAN_FAILED   "signal_sta_jianquan_failed"
*/

char WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN[PATH_LEN]=		"wid_dbus_trap_wtp_wireless_interface_down";
char WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR[PATH_LEN]=		"wid_dbus_trap_wtp_wireless_interface_down_clear";
char WID_DBUS_TRAP_WID_WTP_CONFIG_CHANGE[PATH_LEN]=		"wid_dbus_trap_wtp_config_change";

char WID_DBUS_CONF_METHOD_SET_MODEL[PATH_LEN]=	"set_model";
char WID_DBUS_CONF_METHOD_SHOW_MODEL[PATH_LEN]=	"show_model";
char WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST[PATH_LEN]=	"show_model_list";


char WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR[PATH_LEN]=		"wid_dbus_trap_wtp_ac_discovery_cover_hole_clear";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR[PATH_LEN]=		"wid_dbus_trap_wtp_channel_device_interference_clear";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR[PATH_LEN]=		"wid_dbus_trap_wtp_channel_ap_interference_clear";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR[PATH_LEN]=		"wid_dbus_trap_channel_terminal_interference_clear";
char WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR[PATH_LEN]=		"wid_dbus_trap_channel_count_minor_clear";

char WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID[PATH_LEN]=		"wid_dbus_trap_wtp_find_unsafe_essid";
char WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK[PATH_LEN]=		"wid_dbus_trap_wtp_find_wids_attack";
char WID_DBUS_TRAP_WID_SSID_KEY_CONFLICT[PATH_LEN]=		"wid_dbus_trap_ssid_key_conflict";

char WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID[PATH_LEN]=          "show_wids_statistics_list_bywtpid";
char WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST[PATH_LEN]=          "show_wids_statistics_list";
char WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID[PATH_LEN]=          "clear_wids_statistics_list_bywtpid";
char WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST[PATH_LEN]=          "clear_wids_statistics_list";

char WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID[PATH_LEN]=          "show_wids_device_list_bywtpid";
char WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST[PATH_LEN]=          "show_wids_device_list";
char WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_OF_ALL[PATH_LEN]=   "show_wids_device_of_all";  //fengwenchao add 20101227
char WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID[PATH_LEN]=          "clear_wids_device_list_bywtpid";
char WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST[PATH_LEN]=          "clear_wids_device_list";
char WID_BAK_OBJPATH[PATH_LEN]=	"/aw/wid";
char WID_BAK_INTERFACE[PATH_LEN]=	"aw.wid";
char WID_DBUS_WTP_METHOD_MASTER_BAK_SET[PATH_LEN]=	"master_and_bak_set";
char WID_DBUS_AC_METHOD_UPDATE_BAK_INFO[PATH_LEN]=	"update_bak_ac_info";
char WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO[PATH_LEN]=	"synchronize_wsm_table_info";
char WID_DBUS_AC_METHOD_ASD_SYNCHRONIZE_INFO[PATH_LEN]=	"synchronize_asd_table_info";
char WID_DBUS_AC_METHOD_NOTICE_INFO[PATH_LEN]=	"notice_vrrp";
char WID_DBUS_AC_METHOD_VRRP_INFO[PATH_LEN]=	"vrrp_info";
char WID_DBUS_AC_METHOD_VRRP_SOCK_INFO[PATH_LEN]=	"vrrp_sock_info";

char WID_DBUS_WLAN_METHOD_FORMARD_MODE[PATH_LEN]=        "wlan_formard_mode";
char WID_DBUS_WLAN_METHOD_TUNNEL_MODE[PATH_LEN]=         "wlan_tunnel_mode";

char WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL[PATH_LEN]=	"set_neighbordead_interval";
char WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL[PATH_LEN]=	"show_neighbordead_interval";

char WID_DBUS_CONF_METHOD_WTPLIST_BYWLANID[PATH_LEN]= "show_wtp_bywlanid";

char WID_DBUS_WTP_METHOD_SET_UPDATE_PATH[PATH_LEN]=		"wtp_set_update_path";
char WID_DBUS_CONF_METHOD_SHOW_UPDATE_CONFIG[PATH_LEN]=	"show_update_config";
char WID_DBUS_CONF_METHOD_CLEAR_UPDATE_CONFIG[PATH_LEN]=	"clear_update_config";
char WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN[PATH_LEN]=          "set_wid_trap_open";
char WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE[PATH_LEN]=          "set_wid_trap_switch_able";
char WID_DBUS_CONF_METHOD_TRAP_SWITCH_SHOW[PATH_LEN]=          "set_wid_trap_switch_show";
char	WID_DBUS_WTP_METHOD_UPDATE_WTP_IMG[PATH_LEN]=	"update_wtp_img";
char	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_WTP_IMG[PATH_LEN]=	"clear_update_wtp_img";
char	WID_DBUS_WTP_METHOD_UPDATE_WTP_IMG_LIST[PATH_LEN]=	"update_wtp_img_list";
char	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_WTP_IMG_LIST[PATH_LEN]=	"clear_update_wtp_img_list";
/*mahz add for ap upgrade automatically*/
char WID_DBUS_WTP_METHOD_BIND_MODEL_WITH_FILE[PATH_LEN]= "wtp_bind_model_with_file";
char WID_DBUS_WTP_METHOD_SET_AP_UPDATE_BASE_MODEL[PATH_LEN]= "wtp_set_ap_update_base_model";
char WID_DBUS_WTP_METHOD_SHOW_MODEL_BIND_INFO[PATH_LEN]= "show_model_bind_info";
char WID_DBUS_WTP_METHOD_DELETE_MODEL_BIND_INFO[PATH_LEN]= "delete_model_bind_info";
char WID_DBUS_CONF_METHOD_CLEAR_ONE_MODEL_UPDATE_CONFIG[PATH_LEN]=	"clear_one_model_update_config";
char WID_DBUS_WTP_METHOD_SHOW_DETAIL_MODEL_BIND_INFO[PATH_LEN]= "show_model_detail_bind_info";
char WID_DBUS_WTP_METHOD_SET_AP_UPDATE_COUNT_ONETIME[PATH_LEN]= "wtp_set_ap_update_count_onetime";
char WID_DBUS_WTP_METHOD_SET_AP_UPDATE_CONTROL[PATH_LEN]= "wtp_set_ap_update_control";
char WID_DBUS_WTP_METHOD_SHOW_AP_UPGRADE_RESULT_INFO[PATH_LEN]= "show_ap_upgrade_result_info";

char WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE[PATH_LEN]= "update_wtpcompatible";
char WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT[PATH_LEN]=	"wid_set_ap_update_fail_count";
char WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT[PATH_LEN]=	"wid_show_ap_update_fail_count";
char WID_DBUS_CONF_METHOD_WTPLIST_UPDATE_FAIL_LIST[PATH_LEN]=	"show_wtp_list_update_fail";
char	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_FAIL_WTP_LIST[PATH_LEN]=	"clear_update_fail_wtp_list";
char WID_DBUS_CONF_METHOD_SHOW_OLD_AP_IMG[PATH_LEN]=		"show_old_ap_img_op";

/*zhanglei add for bak ac ip list group*/
char	WID_DBUS_ACIPLIST_OBJPATH[PATH_LEN]=		"/aw/wid";
char WID_DBUS_ACIPLIST_INTERFACE[PATH_LEN]=		"aw.wid";
char	WID_DBUS_ACIPLIST_METHOD_CONFIG[PATH_LEN]=	"config_ac_ip_list";
char	WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP[PATH_LEN]=	"add_ac_ip_list_group";
char	WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP[PATH_LEN]=	"del_ac_ip_list_group";
char	WID_DBUS_ACIPLIST_METHOD_CONFIG_AC_IP_LIST[PATH_LEN]=		"config_ac_ip_list";
char	WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP[PATH_LEN]=				"add_ac_ip";
char	WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP[PATH_LEN]=				"del_ac_ip";
char	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY[PATH_LEN]=		"set_ac_ip_priority";
char	WID_DBUS_ACIPLIST_METHOD_AC_IP_LIST_GROUP_SWITCH[PATH_LEN]=	"ac_ip_list_group_switch";
char WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST[PATH_LEN]=		"show_ac_ip_list";
char WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE[PATH_LEN]=	"show_ac_ip_list_one";
char WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG[PATH_LEN]=		"show_ac_ip_list_running_config";

char WID_DBUS_RADIO_METHOD_WLAN_SET_STA_IP_MAC_BINDING[PATH_LEN]=	"radio_wlan_set_sta_ip_mac_binding";

char WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME_IPV6[PATH_LEN]=	"apply_wlan_ifname_ipv6";
char WID_DBUS_WTP_METHOD_APPAY_WTP_IFNAME_IPV6[PATH_LEN]=   "apply_wtp_ifname_ipv6";
char WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID[PATH_LEN]=	"radio_wlan_set_sta_vlanid";
char WID_DBUS_RADIO_METHOD_WLAN_SET_STA_DHCP_BEFORE_AUTHERIZED[PATH_LEN]=	"radio_wlan_set_sta_dhcp_before_autherized";
char WID_DBUS_CONF_METHOD_AP_HOTREBOOT[PATH_LEN]=          "set_ap_hotreboot";
char WID_DBUS_CONF_METHOD_AP_ACCESS_THROUGH_NAT[PATH_LEN]=          "set_ap_access_through_nat";

 char WID_DBUS_WTP_METHOD_SET_WTP_WIDS_POLICY[PATH_LEN] =  "set_wtp_wids_policy";
 char WID_DBUS_CONF_METHOD_ADD_WID_MAC[PATH_LEN] = "add_wids_mac";
 char WID_DBUS_CONF_METHOD_DEL_WID_MAC[PATH_LEN] = "del_wids_mac";
 char WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW[PATH_LEN] = "show_wids_mac_list";
 char WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES[PATH_LEN] = "set_ap_countermeasures";
 char WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE[PATH_LEN] = "set_ap_countermeasures_mode";
 char  WID_DBUS_RADIO_METHOD_SET_ACKTIMEOUT_DISTANCE[PATH_LEN] = "radio_set_acktimeout_distance";/*wcl add for RDIR-33*/
 char  WID_DBUS_RADIO_METHOD_SET_GUARD_INTERVAL[PATH_LEN] = "radio_set_guardinterval";
 char  WID_DBUS_RADIO_METHOD_SET_MCS[PATH_LEN]= "radio_set_mcs";
 char  WID_DBUS_RADIO_METHOD_SET_CMMODE[PATH_LEN] ="radio_set_cmmode";
 char  WID_DBUS_RADIO_SECTOR_SET_CMD[PATH_LEN] =	 "set_sector_value";
 char  WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD[PATH_LEN] =	 "set_tx_chainmask_value";
 char  WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD_V2[PATH_LEN] =	"set_tx_chainmask_value_v2";

 char  WID_DBUS_RADIO_SECTOR_POWER_SET_CMD[PATH_LEN] =	 "set_sector_power_value";
 char  WID_DBUS_RADIO_NETGEAR_G_SET_CMD[PATH_LEN] =	 "set_radio_netgear_supper_g_type_cmd";
char WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_UPLINK[PATH_LEN]=		"ebr_set_ebr_add_del_uplink" ;
char WID_DBUS_EBR_METHOD_SHOW_FDB_SUMMARY_DETAIL[PATH_LEN]=		"ebr_show_fdb_summary_detail" ;

char  WID_DBUS_RADIO_METHOD_SET_WDS_DISTANCE[PATH_LEN] ="radio_set_wds_distance";
char  WID_DBUS_RADIO_METHOD_SET_WDS_REMOTE_BRMAC[PATH_LEN] ="radio_set_wds_remote_brmac";
char  WID_DBUS_RADIO_METHOD_SET_WDS_ENCRYPTION_TYPE[PATH_LEN] ="radio_set_wds_encryption_type";
char  WID_DBUS_RADIO_METHOD_SET_WDS_WEP_KEY[PATH_LEN] ="radio_set_wds_wep_key";
char  WID_DBUS_RADIO_METHOD_SET_WDS_AES_KEY[PATH_LEN] ="radio_set_wds_aes_key";


char  WID_DBUS_WTP_LIST_METHOD_SET_DHCP_SNOOPING[PATH_LEN] ="wtp_list_set_dhcp_snooping";
char  WID_DBUS_WTP_METHOD_SET_DHCP_SNOOPING[PATH_LEN] ="wtp_set_dhcp_snooping";
char  WID_DBUS_WTP_LIST_METHOD_STA_INFO_REPORT[PATH_LEN] ="wtp_list_sta_info_report";
char  WID_DBUS_WTP_METHOD_STA_INFO_REPORT[PATH_LEN] ="wtp_sta_info_report";
char  WID_DBUS_WLAN_METHOD_STA_IP_MAC_BINDING[PATH_LEN] ="wlan_sta_ip_mac_bind";
char  WID_DBUS_WLAN_WTP_LIST_METHOD_STA_STATIC_ARP[PATH_LEN] ="wlan_wtp_list_sta_static_arp";
char  WID_DBUS_WLAN_WTP_METHOD_STA_STATIC_ARP[PATH_LEN] ="wlan_wtp_sta_static_arp";
char  WID_DBUS_WLAN_METHOD_TIMER_ABLE[PATH_LEN] ="wlan_timer_able";
char  WID_DBUS_WLAN_METHOD_SERVICE_CONTROL_TIMER[PATH_LEN] ="wlan_service_control_timer";
char  WID_DBUS_CONF_METHOD_SHOW_AP_TRAP_ROGUE_AP_TERMINAL_CPU_MEM_THRESHOLD[PATH_LEN] ="show_ap_trap_rogue_ap_terminal_cpu_mem_threshold";
char  WID_DBUS_CONF_METHOD_SET_WTP_ROGUEAP_ROGUETERMINAL_CPU_MEM_TRAP_THRESHOLD[PATH_LEN] ="set_wtp_rogueap_terminal_cpu_mem_trap_threshold";
char  WID_DBUS_CONF_METHOD_SET_WTP_TRAP_SWITCH[PATH_LEN] ="set_wtp_trap_swith";
char  WID_DBUS_CONF_METHOD_SET_WTP_SEQNUM_SWITCH[PATH_LEN] ="set_wtp_seqnum_swith";   /*wcl add*/
char  WID_DBUS_CONF_METHD_SET_WID_ROGUE_DANGER_UNSAFE_ATTACK_TRAP_STATE[PATH_LEN] ="set_wid_rogue_danger_unsafe_attack_trap_state";   /*fengwencaho add 20110221*/
char  WID_DBUS_CONF_METHOD_MODIFY_ESSID[PATH_LEN] ="modify_legal_essid";
char  WID_DBUS_RADIO_METHOD_SET_AMPDU_ABLE[PATH_LEN] ="radio_set_ampdu_able";
char  WID_DBUS_RADIO_METHOD_SET_AMPDU_LIMIT[PATH_LEN] ="radio_set_ampdu_limit";
char  WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFRAME[PATH_LEN] ="radio_set_ampdu_subframe";//zhangshu add for subframe, 2010-10-09

char  WID_DBUS_RADIO_METHOD_SET_MIXED_PURE_N[PATH_LEN] ="radio_set_mixed_pure_n";

char WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_SWITCH[PATH_LEN] = "set_sta_wapi_info_switch";
char WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_REPORTINTERVAL[PATH_LEN] = "set_sta_wapi_info_reportinterval";

char WID_DBUS_RADIO_METHOD_SET_CHANNEL_OFFSET[PATH_LEN] =	"radio_set_channel_offset";
char WID_DBUS_RADIO_METHOD_SET_TXPOWER_STEP[PATH_LEN] = "radio_set_txpower_step";
char WID_DBUS_RADIO_METHOD_SET_RADIO_WLAN_LIMIT_RSSI_ACCESS_STA[PATH_LEN] = "set_radio_wlan_access_sta_limit_rssi"; //fengwenchao add 20120222 for RDIR-25
char WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_BANLANCE_FLAG[PATH_LEN] =	"set_ac_ip_list_banlance";
char WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH[PATH_LEN] =	"set_wid_mac_whitelist";
char WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH[PATH_LEN] =	"set_wid_essid_whitelist";
char WID_DBUS_CONF_METHOD_CHANGE_WHITELIST[PATH_LEN] =          "change_mac_whitelist";
char WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST[PATH_LEN] =          "change_mac_blacklist";
char	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_THRESHOLD[PATH_LEN] =	"set_ac_ip_list_threshold";
char	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_DIFF_BANLANCE[PATH_LEN] =	"set_ac_ip_list_diff_banlance";

char WID_DBUS_RADIO_METHOD_MOLTI_USER_OPTIMIZE_SWITH[PATH_LEN] =  "radio_multi_user_optimize_switch";
char WID_DBUS_WTP_METHOD_5G_SWITH[PATH_LEN] = "radio_5g_switch";

char WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTSWITCH[PATH_LEN]  =	"wsm_sta_info_reportswitch";
char  WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTINTERVAL[PATH_LEN] =	"wsm_sta_info_reportinterval" ;
char  WID_DBUS_RADIO_METHOD_TIMER_ABLE[PATH_LEN] ="radio_timer_able";
char  WID_DBUS_RADIO_METHOD_SERVICE_CONTROL_TIMER[PATH_LEN] ="radio_service_control_timer";
char WID_DBUS_AC_METHOD_SET_INTERFACE_VMAC[PATH_LEN] = "set_interface_vmac";
char WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY[PATH_LEN] =      "wids_judge_policy";
char WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY_SHOW[PATH_LEN] =      "wids_judge_policy_show";
char WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW[PATH_LEN] = "show_access_wtp_vendor_count";
char WID_DBUS_CONF_METHOD_SET_LICENSE_BINDING[PATH_LEN] = "set_license_binding";	/*xiaodawei append, 20101029*/
char WID_DBUS_IPERF_WTPIP[PATH_LEN] = "iperf_wtpip";								//xiaodawei add, 20110303
char WID_DBUS_CONF_METHOD_SET_WTP_LINK_DETECT[PATH_LEN] = "set_wtp_link_detect";
char WID_DBUS_CONF_METHOD_SET_WSM_SWITCH[PATH_LEN] = "set_wsm_switch";
char WID_DBUS_CONF_METHOD_SET_VLAN_SWITCH[PATH_LEN] = "set_vlan_switch";
char WID_DBUS_CONF_METHOD_SET_DHCP_OPTION82_SWITCH[PATH_LEN] = "set_dhcp_option82_switch";

char WID_DBUS_CONF_METHOD_TUNNEL_MODE_POLICY[PATH_LEN] =     "tunnel_mode_policy";
/*zhaoruijia,tranlate  neighbor_channel_interference to 1.3,start*/
char WID_DBUS_CONF_METHOD_SET_NEIGHBOR_SAME_CHANNELRSSI_THRESHOLD[PATH_LEN]= "set_wtp_neighbor_same_channelrssi_threshold";
char WID_DBUS_RADIO_METHOD_SET_MIMO[PATH_LEN] =	"radio_set_mimo";
char WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFREAM[PATH_LEN] =	"radio_set_ampdu_subframe";
char WID_DBUS_AP_GROUP_OBJPATH[PATH_LEN]=		"/aw/wid";
char WID_DBUS_AP_GROUP_INTERFACE[PATH_LEN]=		"aw.wid";
char WID_DBUS_AP_GROUP_METHOD_CONFIG[PATH_LEN]=	"config_ap_group";
char WID_DBUS_AP_GROUP_METHOD_CREATE[PATH_LEN]=	"create_ap_group";
char WID_DBUS_AP_GROUP_METHOD_DEL[PATH_LEN]=	"delete_ap_group";
char WID_DBUS_AP_GROUP_METHOD_ADD_DEL_MEMBER[PATH_LEN]=	"add_delete_ap_group_member";
char WID_DBUS_AP_GROUP_METHOD_SHOW_MEMBER[PATH_LEN]= "show_ap_group_member";
char WID_DBUS_CONF_METHOD_SERVICE_TFTP_SWITCH[PATH_LEN]="service_tftp_switch";
char WID_DBUS_CONF_METHOD_SHOW_SERVICE_TFTP_SWITCH[PATH_LEN]="show_service_tftp";

char WID_DBUS_CONF_METHOD_SERVICE_FTP_SWITCH[PATH_LEN]="service_ftp_switch";
char WID_DBUS_CONF_METHOD_SHOW_SERVICE_FTP_SWITCH[PATH_LEN]="show_service_ftp";

char WID_DBUS_CONF_METHOD_SHOW_WLAN_RADIO_INFORMATION[PATH_LEN]="show_wlan_radio_information";
char WID_DBUS_CONF_METHOD_SET_WID_ERROR_HANDLE_STATE[PATH_LEN] = "set_wid_error_handle_state";

char WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO_TUNNEL[PATH_LEN] = "show_wtp_radio_pkt_info_tunnel";
char WID_DBUS_WLAN_METHOD_SHOW_WLAN_PTK_INFO[PATH_LEN] = "wid_dbus_wlan_show_wlan_ptk_info";
char WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME[PATH_LEN] = "radio_receive_data_dead_time";
char WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME_SHOW[PATH_LEN] = "radio_receive_data_dead_time_show";
char WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER_V2[PATH_LEN] = "set_wid_qos_flow_parameter_v2";
char WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE[PATH_LEN] = "set_wid_qos_flow_able";
char WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE_V2[PATH_LEN] = "set_wid_qos_flow_able_v2";
char WID_DBUS_CONF_METHOD_SET_WTP_COLLECT_TIME[PATH_LEN] = "set_wtp_trap_collect_time";
char WID_DBUS_CONF_METHOD_SHOW_DBUS_COUNT[PATH_LEN] = "wid_show_dbus_count";
char WID_DBUS_CONF_METHOD_SET_DBUS_COUNT[PATH_LEN] = "wid_set_dbus_count";
char WID_DBUS_CONF_METHOD_ADD_BLACK_WHITE_OUI_MAC[PATH_LEN] = "add_black_white_oui_mac";
char WID_DBUS_CONF_METHOD_DEL_BLACK_WHITE_OUI_MAC[PATH_LEN] = "del_black_white_oui_mac";
char WID_DBUS_CONF_METHOD_SHOW_BLACK_WHITE_OUI_INFO[PATH_LEN] = "show_black_white_oui_info_list";
char WID_DBUS_CONF_METHOD_UPDATE_BLACK_WHITE_OUI_INFO_LIST[PATH_LEN] = "update_black_white_oui_info";
char WID_DBUS_CONF_METHOD_USE_BLACK_WHITE_NONE_OUI_POLICY[PATH_LEN] = "use_black_white_none_policy";
char WID_DBUS_CONF_METHOD_SHOW_OUI_POLICY[PATH_LEN] = "show_oui_policy";
char WID_DBUS_CONF_METHOD_SHOW_CONFLICT_WTP_LIST[PATH_LEN] = "show_conflict_wtp_list";
char WID_DBUS_CONF_METHOD_CHECKING[PATH_LEN]=	"wid_checking";
char WID_DBUS_CONF_METHOD_QUIT[PATH_LEN]=	"wid_quit";
char WID_DBUS_CONF_METHOD_LOCAL_HASNI_STATE_CHANGE[PATH_LEN] = "wid_local_hansi_state_change";
char  WID_DBUS_CONF_METHOD_WIDS_MONITOR_MODE[PATH_LEN] =          "wids_monitor_mode";
char  WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE[PATH_LEN] =	"wids_scanning_mode";
char  WID_DBUS_CONF_METHOD_WIDS_SCANNING_CHANNEL[PATH_LEN] =	"wids_scanning_channel";
char  WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE_CHANNEL_SHOW[PATH_LEN] =	"wids_scanning_mode_channel_show";
char WID_DBUS_CONF_METHOD_MULTICAST_LISTEN_SETTING[PATH_LEN] = "wireless_multicast_setting";
char WID_DBUS_CONF_METHOD_NO_RESPONSE_TO_STA_PROBLE_REQUEST[PATH_LEN] = "wid_cmd_no_response_to_sta_proble_request";
char WID_DBUS_CONF_METHOD_UNI_MUTI_BR_CAST_ISOLATION_SW_AND_RATE_SET[PATH_LEN] = "wid_cmd_uni_muti_bro_cast_isolation_sw_and_rate_set";
char WID_DBUS_CONF_METHOD_MUTI_BR_CAST_RATE_SET[PATH_LEN] = "wid_cmd_muti_bro_cast_rate_set";
char WID_DBUS_CONF_METHOD_WLAN_NO_RESPONSE_TO_STA_PROBLE_REQUEST[PATH_LEN] = "wid_cmd_wlan_no_response_to_sta_proble_request";
char WID_DBUS_CONF_METHOD_WLAN_UNI_MUTI_BR_CAST_ISOLATION_SW_AND_RATE_SET[PATH_LEN] = "wid_cmd_wlan_uni_muti_bro_cast_isolation_sw_and_rate_set";
char WID_DBUS_CONF_METHOD_WLAN_MUTI_BR_CAST_RATE_SET[PATH_LEN] = "wid_cmd_wlan_muti_bro_cast_rate_set";
char WID_DBUS_CONF_METHOD_BAK_CHECK_INTERVAL[PATH_LEN] 			=	"bak_check_interval";			/* Huang Leilei 2012-10-22 add */
char WID_DBUS_CONF_METHOD_LIC_BAK_REQ_INTERVAL[PATH_LEN]		=	"lic_bak_req_interval";			/* Huang Leilei 2012-10-25 add */
char WID_DBUS_CONF_METHOD_BAK_CHECK_INTERVAL_SHOW[PATH_LEN]		=	"show_bak_check_interval";		/* Huang Leilei 2012-10-24 add */
char WID_DBUS_CONF_METHOD_LIC_BAK_REQ_INTERVAL_SHOW[PATH_LEN]	=	"show_lic_bak_check_interval";	/* Huang Leilei 2012-10-25 add */
char WID_DBUS_WLAN_METHOD_TUNNEL_NODE_SETTING[PATH_LEN] ="wlan_tunnel_mode_setting";

#endif
