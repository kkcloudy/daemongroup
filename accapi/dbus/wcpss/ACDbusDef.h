#ifndef _WID_DBUS_DEF_H
#define _WID_DBUS_DEF_H

#define WID_DBUS_BUSNAME	"aw.wid"
#define WID_DBUS_OBJPATH	"/aw/wid"
#define WID_DBUS_INTERFACE	"aw.wid"
/*#define WID_DBUS_SUCCESS 0
#define WID_DBUS_ERROR	1
#define WID_DBUS_NO_WLAN_ID	2
#define WID_DBUS_NO_WTP_ID	3
#define WID_DBUS_NO_Radios_ID	4
#define WLAN_ID_BE_USED	5
#define WLAN_ID_NOT_EXIST	6
#define WTP_ID_BE_USED	7
#define WTP_ID_NOT_EXIST	8
*/
enum wid_dbus_result_no_e {
	WID_DBUS_SUCCESS,
	WID_DBUS_ERROR, /* general use, no detail error information*/
	WLAN_ID_BE_USED,
	WLAN_ID_NOT_EXIST,
	WTP_ID_BE_USED,
	WTP_ID_NOT_EXIST,	
	BSS_NOT_EXIST,
	BSS_NOT_ENABLE,
	RADIO_ID_BE_USED,
	RADIO_ID_NOT_EXIST,
	APPLY_IF_FAIL,
	WLAN_BE_ENABLE,
	INTERFACE_NOT_BE_BINDED,
	INTERFACE_NOT_EXIST,
	WTP_BE_USING,
	Wlan_IF_NOT_BE_BINDED,
	WTP_IF_NOT_BE_BINDED,
	WTP_WLAN_BINDING_NOT_MATCH,
	WTP_CLEAR_BINDING_WLAN_SUCCESS,
	WLAN_NOT_BINDING_IF,
	WID_UNKNOWN_ID,
	WTP_NO_SURPORT_TYPE,
	WTP_NO_SURPORT_Rate,
	RADIO_IS_DISABLE,
	WTP_OVER_MAX_BSS_NUM,
	WTP_IS_NOT_BINDING_WLAN_ID,
	WLAN_CREATE_L3_INTERFACE_FAIL,
	WLAN_DELETE_L3_INTERFACE_FAIL,
	BSS_CREATE_L3_INTERFACE_FAIL,
	BSS_DELETE_L3_INTERFACE_FAIL,
	WLAN_CREATE_BR_FAIL,
	WLAN_DELETE_BR_FAIL,
	BSS_L3_INTERFACE_ADD_BR_FAIL,
	BSS_L3_INTERFACE_DEL_BR_FAIL,
	PAST_L3_INTERFACE_ERROE,
	UNKNOWN_ERROR,
	WTP_NOT_IN_RUN_STATE,
	WLAN_BE_DISABLE,
	BSS_BE_ENABLE,
	BSS_IF_NEED_CREATE,
	SYSTEM_CMD_ERROR,
	BSS_IF_NEED_DELETE,
	L3_INTERFACE_ERROR,
	AUTO_AP_LOGIN_INTERFACE_ERROR,
	AUTO_AP_LOGIN_INTERFACE_NOT_SET,
	AUTO_AP_LOGIN_WLAN_NOT_SET,
	WID_QOS_BE_USED,
	WID_QOS_NOT_EXIST,
	WID_QOS_RADIO_SHOULD_BE_DISABLE,
	WID_QOS_WMM_MAP_DISABLE,
	WID_QOS_STREAM_ERROR,
	WID_QOS_PARAMETER_ERROR,
	WID_QOS_PARAMETER_VALUE_ERROR,
	WID_EBR_ERROR,
	WID_EBR_BE_USED,
	WID_EBR_NOT_EXIST,
	WID_EBR_SHOULD_BE_DISABLE,
	WID_EBR_IFLIST_EMPTY,
	WID_WTP_LOCATION_NOT_SET,
	SUPPORT_VERSION_NOT_EXIST,
	VERSION_NOT_SUPPORTED,
	SAMPLE_THROUGHPUT_NOT_ENABLE,
	IF_POLICY_CONFLICT,
	WID_NO_SURPORT_MODEL,
	WTP_OVER_MAX_NUM,
	WTP_LESS_THAN_STATIC_WTP,
	WTP_SN_BE_USED,
	RADIO_SUPPORT_RATE_EMPTY,
	WTP_NO_NEIGHBORAP,
	RADIO_SUPPORT_RATE_ONE,
	RADIO_SUPPORT_RATE_EXIST,
	RADIO_SUPPORT_RATE_NOT_EXIST,
	RADIO_SUPPORT_MAX_RATE_NOT_ONE,
	RADIO_SUPPORT_RATE_CONFLICT,
	WID_NO_WHITELIST,
	WID_NO_BLACKLIST,
	WID_AP_SCANNING_DISABLE,
	WID_TRANSMIT_POWER_CONTROL_DISABLE,
	WLAN_APPLY_SECURITY_FIRST,
	WLAN_NOT_NEED_NAS,
	WID_BINDING_WLAN,
	WLAN_NOT_BINDING_VLAN,
	WLAN_BINDING_VLAN,
	MALLOC_ERROR,
	SWITCH_IS_DISABLE,
	SWITCH_HAS_ALREADY_SETTED,
	WID_AP_NO_STATICS,
	TXPOWER_OVER_TW,
	TXPOWER_OVER_TW_THREE,
	MODEL_NO_EXIST,
	MODEL_NO_CONFIG,
	RAOMING_DISABLE_FIRST,
	ISOLATION_CONFLICT,
	UPDATE_CONFIG_FAIL,
	WTP_WEP_NUM_OVER,
	RADIO_MODE_IS_11N,
	WDS_MODE_BE_USED,
	RADIO_NO_BINDING_WLAN,
	WTP_ID_LARGE_THAN_MAX,
	RADIO_ID_LARGE_THAN_MAX,
	WLAN_ID_LARGE_THAN_MAX,				/*for check*/
	BSS_ID_LARGE_THAN_MAX,
	WTP_MAC_BE_USED,
	NO_WIDS_DEVICE,
	NO_SURPPORT_IPIP,
	VALUE_IS_NONEED_TO_CHANGE,/*used to mark value is the same to the command,so no need to be changed*/
	UPDATE_PROCESS,
	AC_IP_EXIST,
	AC_IP_NOT_EXIST,
	AC_IP_BIND_IF_EXIST,
	AUTO_CHANNEL_STATE,
	AC_BALANCE_STATE,
	AC_BALANCE_FLOW,
	AC_BALANCE_NUM,
	ETH_NOT_EXIST,
	RADIO_NO_BINDING_WQOS,
	BINDING_IPV6_ADDRE_RROR,
	WID_EBR_IF_NOEXIT,
	WTP_NO_SURPORT_CHANNEL,	
	MAC_DOESNOT_EXIT,
	MAC_ALREADY_EXIT,
	WID_NEED_REBOOT,
	RADIO_HT_IS_NOT_40,
	RADIO_CHANNEL_OFFSET_NEED_BE_RESET,
	WID_ILLEGAL_INPUT,
	RADIO_11N_AMPDU_MUTEX,    //zhangshu add 2010-10-12
	RADIO_11N_AMSDU_MUTEX,     //zhangshu add 2010-10-12
	WRONG_LICENSE_LIST,				//xiaodawei append, 20101104
	LICENSE_NUM_LARGER_THAN_MAX,	//xiaodawei append, 20101104
	LICENSE_NUM_LESS_THAN_TWO,		//xiaodawei append, 20101104
	LICENSE_NOT_EXIST,				//xiaodawei append, 20101104
	LICENSE_TYPE_DUPLICATED,		//xiaodawei append, 20101104
	LICENSE_TYPE_BINDED,			//xiaodawei append, 20101104
	LICENSE_NO_BINDING_SUCCESS,		//xiaodawei append, 20101108
	LICENSE_NOT_BINDED,				//xiaodawei append, 20101108
	LICENSE_BINDED_ANOTHER,			//xiaodawei append, 20101108
	CURRENT_NUM_MORE_THAN_MAX,		//xiaodawei append, 20101108
	AP_L2_ISOLATION_ENABLE_OR_DIS_ALREADY,			//xiaodawei add for l2 isolation enable or disable already, 20101207
	RADIO_SET_TXPOF_COUNTRY_CODE_ERROR,		//fengwenchao append,20101213
	SET_MAX_STANUM_SMALLER_THAN_CURRENT_STANUM,   //fengwenchao append,20101213
	CHECK_CHANNEL_CWMODE_FAIL,		//fengwenchao append,20101213
	CHECK_COUNTRY_CODE_FAIL,			//fengwenchao append,20101213
	ESSID_LIST_IS_NULL,				//weichao append, 20110914
	ESSID_NOT_EXIT,					//weichao append,20110914
	RADIO_IN_EBR,    //fengwenchao add for radio whether in ebr check
	FLOW_CHECK_BE_DISABLE,
	WID_MAC_ADD_ALREADY,
	WID_WANT_TO_DELETE_WLAN,			/* Huangleilei add for ASXXZFI-1622 */
	VALUE_OUT_OF_RANGE,					/* Huangleilei copy from 1.3.18, 20130610 */
	MIN_LARGER_THAN_MAX					/* Huangleilei copy from 1.3.18, 20130610 */
};

enum wid_radio_type {
	IEEE80211_11B = 0x1,
	IEEE80211_11A = 0x2,
	IEEE80211_11G = 0x4,
	IEEE80211_11N = 0x8,
	IEEE80211_11AN = 0x10,		//fengwenchao add for GM 20111109
	IEEE80211_11GN = 0x20,		//fengwenchao add for GM 20111109

};

enum wid_check_type {
	WID_WTP_CHECK,
	WID_WLAN_CHECK,
	WID_RADIO_CHECK,
	WID_SECURITY_CHECK,

};

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
#define WID_DBUS_CONF_METHOD_SHOWWLAN	"show_wlan"
#define WID_DBUS_CONF_METHOD_SHOWWLAN_OF_ALL  "show_wlan_of_all"    //fengwenchao add 20101223
#define WID_DBUS_CONF_METHOD_WLANLIST	"show_wlan_list"

#define WID_DBUS_CONF_METHOD_WTPLIST	"show_wtp_list"
#define WID_DBUS_CONF_METHOD_WTPLIST_BYMAC	"show_wtp_list_bymac"
#define WID_DBUS_CONF_METHOD_WTPLIST_NEW	"show_wtp_list_new"
#define WID_DBUS_CONF_METHOD_WTPLIST_NEW_BYINTERFACE	"show_wtp_list_new_byinterface"		//xiaodawei add, 20110301
#define WID_DBUS_CONF_METHOD_SHOWWTP_BYMODEL   "show_wtp_list_bymodel"      //fengwenchao add 20110226
#define WID_DBUS_CONF_METHOD_SHOWWTP_BYVERSION   "show_wtp_list_byversion"  //fengwenchao add 20110314
#define WID_DBUS_CONF_METHOD_WTPLIST_UPDATE	"show_wtp_list_update"
#define WID_DBUS_CONF_METHOD_AC_ACCESS_WTPLIST	"show_ac_access_wtp_list"
#define WID_DBUS_CONF_METHOD_CLEAN_AC_ACCESS_WTPLIST	"clean_ac_access_wtp_list"
#define WID_DBUS_CONF_METHOD_OLD_AP_IMG		"old_ap_img_op"


#define WID_DBUS_CONF_METHOD_ACVERSION	"show_ac_version"
#define WID_DBUS_CONF_METHOD_ECHOTIMER	"show_ap_echotimer"
#define WID_DBUS_CONF_METHOD_SHOW_WTP_RUNNING_CONFIG	"show_wtp_running_config"

#define WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START	"wlan_show_running_config_start"

#define WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END	"wlan_show_running_config_end"

#define WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_START	"wtp_show_running_config_start"

#define WID_DBUS_CONF_METHOD_WTP_SHOW_RUNNING_CONFIG_END	"wtp_show_running_config_end"
/*
 arg lists for method WID_DBUS_CONF_METHOD_WLAN
  in arg list:
	BYTE WlanID //ID of WLAN which is unique sign of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_CONF_METHOD_WLAN	"wlan"


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
#define	WID_DBUS_CONF_METHOD_SHOWWTP	"show_wtp"
#define WID_DBUS_CONF_METHOD_SHOW_CONFIG_WTP  "show_wtp_config_of_all_wtp"  //fengwenchao add 20101223

#define	WID_DBUS_WTP_METHOD_SHOW_WTP_MODEL_CODE_VERSION	"show_wtp_model_code_version"


/*
 arg lists for method WID_DBUS_CONF_METHOD_WTP
  in arg list:
	BYTE WTPID //ID of WLAN which is unique sign of wlan
  out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_CONF_METHOD_WTP	"wtp"


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
#define	WID_DBUS_CONF_METHOD_SHOWRADIO	"show_radio"

#define WID_DBUS_CONF_METHOD_RADIOLIST	"show_radio_list"
#define WID_DBUS_CONF_METHOD_RADIO		"radio"

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
#define	WID_DBUS_CONF_METHOD_ADD_DEL_WLAN	"add_del_wlan"
#define	WID_DBUS_CONF_METHOD_ADD_DEL_WLAN_CN	"add_del_wlan_cn"

#define	WID_DBUS_CONF_METHOD_ADD_DEL_WLAN_EBR	"add_del_wlan_ebr"
#define WID_DBUS_CONF_METHOD_AUTO_ADD_RADIO_EBR_SWITCH	"auto_add_radio_to_ebr_switch"
#define WID_DBUS_CONF_METHOD_SHOW_AUTO_ADD_RADIO_EBR_SWITCH "show_auto_add_radio_to_ebr_switch"
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
#define	WID_DBUS_CONF_METHOD_ADD_DEL_WTP	"add_del_wtp"

#define	WID_DBUS_CONF_METHOD_ADD_DEL_WTP_BY_MAC	"add_del_wtp_by_mac"


#define WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT	"update_wtp_count"
	
#define	WID_DBUS_WLAN_OBJPATH	"/aw/wid/wlan"
#define WID_DBUS_WLAN_INTERFACE	"aw.wid.wlan"


/*
arg lists for method WID_DBUS_CONF_METHOD_WLAN_ENABLE
 in arg list:
   	BYTE	WlanID//id of wlan which you want enable or disable
  	BYTE	DisWLAN //determine disable(1) or enable(0)WLAN  
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_WLAN_METHOD_ENABLE	"wlan_enable"
#define WID_DBUS_WDS_METHOD_ENABLE	"wds_enable"


/*
arg lists for method WID_DBUS_CONF_METHOD_WLAN_APPLY_IF
 in arg list:
   	BYTE	WlanID//id of wlan which you want enable or disable
  	STRING	interface//which interface wlan apply  
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_WLAN_METHOD_APPLY_IF	"wlan_apply_if"

#define WID_DBUS_WTP_METHOD_APPAY_WTP_IFNAME   "apply_wtp_ifname"
#define WID_DBUS_WTP_METHOD_APPAY_WTP_IFNAME_IPV6   "apply_wtp_ifname_ipv6"
#define WID_DBUS_WTP_METHOD_APPAY_WTP_MAX_STA   "wtp_max_sta"
#define WID_DBUS_WTP_INTERFACE_CHECK_WTP_STA "check wtp sta";
#define WID_DBUS_WTP_METHOD_APPAY_WTP_TRIGER   "wtp_triger"
#define WID_DBUS_WTP_METHOD_APPAY_WTP_FLOW_TRIGER   "wtp_flow_triger"


#define WID_DBUS_WTP_METHOD_APPAY_WTP_WLANID	"apply_wtp_wlanid"
#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME	"apply_wlan_ifname"
#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME_IPV6	"apply_wlan_ifname_ipv6"

#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_MAX_STA	"wlan_max_sta"
#define WID_DBUS_WLAN_METHOD_WLAN_MAX_STA_NEW   "wlan_max_new"  //fengwenchao add 20120323
#define WID_DBUS_WLAN_METHOD_WLAN_L2_ISOLATION   "wlan_l2_isolation"   //fengwenchao add 20120323
#define WID_DBUS_WLAN_METHOD_WLAN_STA_STATIC_ARP  "wlan_sta_static_arp" //fengwenchao add 20120323
#define WID_DBUS_WLAN_METHOD_WLAN_ACCESS_STA_LIMIT_RSSI  "wlan_access_sta_limit_rssi"  //fengwenchao add 20120323
#define WID_DBUS_WLAN_METHOD_FLOW_CHECK 			"wlan_flow_check"		//weichao add 2011.10.28
#define WID_DBUS_WLAN_METHOD_NO_FLOW_TIME 			"wlan_no_flow_time"		//weichao add 2011.10.31
#define WID_DBUS_WLAN_METHOD_LIMIT_MIN_FLOW		"wlan_limit_min_flow"		//weichao add 2011.10.31

#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_PARA	"wlan_balance_para" /*xm  08/12/29*/
#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_FLOW_BALANCE_PARA	"wlan_flow_balance_para" /*xm  09/02/05*/

#define WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_SWITCH	"wlan_balance_switch" /*xm  08/12/29*/
#define WID_DBUS_WTP_METHOD_WTP_USED            "wtp_used"
/*added by weiay 20080701*/
#define WID_DBUS_WTP_METHOD_DELETE_WLAN_ID            "wtp_delete_wlan_id"
#define WID_DBUS_WTP_METHOD_DISABLE_WLAN_ID            "wtp_disable_wlan_id"
#define WID_DBUS_WTP_METHOD_ENABLE_WLAN_ID				"wtp_enable_wlan_id"
#define WID_DBUS_WTP_METHOD_SHOW_WLAN_ID            "wtp_show_wlan_id"
#define WID_DBUS_WTP_METHOD_SET_VERSION_PATH		"wtp_set_version_path"/*xm 08/08/29*/
#define WID_DBUS_WTP_METHOD_CONFIG_PORT_ENABLE      "wtp_configure_port_enable"/*xm 08/09/01*/
#define WID_DBUS_WTP_METHOD_CONFIG_PORT    		 "wtp_configure_port"/*xm 08/09/01*/
#define WID_DBUS_WTP_METHOD_SET_WTPNAME    		 "wtp_set_wtpname"
#define WID_DBUS_WTP_METHOD_SET_WTPSN    		 "wtp_set_wtpsn"

/* zhangshu add for eap mac, 2010-10-22 */
#define WID_DEBUS_WLAN_METHOD_SET_EAP_MAC "wlan_set_eap_mac"

/* zhangshu add for terminal distrub info, 2010-10-08 */
#define WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_SWITCH      "set_terminal_distrub_infomation_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_PKT      "set_terminal_distrub_infomation_pkt"
#define WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_STA_NUM      "set_terminal_distrub_infomation_sta_num"
#define WID_DBUS_METHOD_SET_AP_HEART_STATISTICS_SWITCH  "set_ap_heart_statistics_switch"  //fengwenchao add for GM-3,20111130
#define WID_DBUS_METHOD_SET_AP_HEART_STATISTICS_COLLECT_TIME "set_ap_heart_statistics_collect_time" //fengwenchao add for GM-3,20111130
#define WID_DBUS_WTP_METHOD_SET_WTP_USERNAME_PASSWORD "set_ap_username_password";

#define WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_SWITCH      "set_extension_infomation_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_REPORTINTERVAL      "set_extension_infomation_reportinterval"

#define WID_DBUS_WTP_METHOD_SET_WTP_UNAUTHORIZED_MAC_REPORTINTERVAL			"set_unauthorized_mac_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_UNAUTHORIZED_MAC_REPORT_SWITCH			"set_unauthorized_mac_report_switch"

#define WID_DBUS_WTP_METHOD_SET_WBS_CPE_SWITCH								"set_wbs_cpe_switch"

#define WID_DBUS_WTP_METHOD_SET_WTP_CONFIGURE_ERR_REPORTINTERVAL			"set_configure_error_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_CONFIGURE_ERR_REPORTSWITCH				"set_configure_error_reportswitch"

#define WID_DBUS_WTP_METHOD_SET_WTP_ONLINE_STA_FULL_REPORTEINTERVAL			"set_online_sta_full_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_ONLINE_STA_FULL_REPORTE_SWITCH			"set_online_sta_full_reportswitch"

#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_RX_TX_OVERLFOW_REPORTINTERVAL		"set_sta_flow_rx_tx_overflow_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_RX_TX_OVERLFOW_THRESHOLD			"set_sta_flow_rx_tx_overflow_threshold"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_RX_TX_OVERLFOW_REPORT_SWITCH		"set_sta_flow_rx_tx_overflow_reportswitch"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_RX_TX_OVERLFOW_TRAP_SWITCH 		"set_sta_flow_rx_tx_overflow_trapswitch"

//#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_TX_OVERLFOW_REPORTINTERVAL		"set_sta_flow_tx_overflow_reportinterval"
//#define WID_DBUS_WTP_METHOD_SET_WTP_STA_FLOW_TX_OVERLFOW_REPORT_SWITCH		"set_sta_flow_tx_overflow_reportswitch"




#define WID_DBUS_WTP_METHOD_SET_WTP_STA_DEAUTH_MESSAGE_REPORT_SWITCH      "set_sta_deauth_message_report_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_ALL_FLOW_INFORMATION_REPORT_SWITCH	"set_sta_all_flow_information_report_switch"
/*for sample  begin*/
#define WID_DBUS_WTP_METHOD_SET_WTP_MOMENT_INFOMATION_SWITCH      "set_moment_infomation_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_MOMENT_INFOMATION_REPORTINTERVAL      "set_moment_infomation_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_SAMPLE_INFOMATION_REPORTINTERVAL	"set_sample_infomation_reportinterval"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_ROUTINE_INFOMATION_REPORTINTERVAL		"set_routine_infomation_reportinterval"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_MOMENT_INFOMATION_REPORTINTERVAL		"show_moment_infomation_reportinterval"
/*for sample  end*/

#define WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_SWITCH      "set_sta_infomation_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_REPORTINTERVAL      "set_sta_infomation_reportinterval"

#define WID_DBUS_WTP_METHOD_SET_WTP_CHECK_WLANID      "check_wlan_id"

#define WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL      "set_wtp_wids_interval"
#define WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD      "set_wtp_wids_threshold"
#define WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK      "set_wtp_wids_lasttime_in_black"
#define WID_DBUS_WTP_METHOD_SET_AC_MASTER_BAK_CORRECT_WTP_STATE_TIMER  "set_ac_master_bak_correct_wtp_state_timer"; //fengwenchao add 20120117 for onlinebug-96
#define WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_SWITCH      "set_ap_if_infomation_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_MTU      "set_ap_interface_eth_mtu"   //fengwenchao add  end


#define WID_DBUS_WTP_METHOD_SET_WTP_WIDS_SET      "set_ap_wids_set"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_WIDS_SET      "show_ap_wids_set"

/*20130617 start*/
#define WID_DBUS_WTP_METHOD_SHOW_DHCP_FLOODING_STATUS_SET		"show_ap_dhcp_flooding_status_set"
#define WID_DBUS_WTP_METHOD_SET_DHCP_FLOODING_STATUS_SET		"set_ap_dhcp_flooding_status_set"
/*20130617 end*/
#define WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_REPORTINTERVAL      "set_ap_if_infomation_reportinterval"
#define WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_UPDOWN      "set_ap_interface_updown"
#define WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_RATE      "set_ap_interface_eth_rate"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_IF_INFO      "show_ap_interface_info"

#define WID_DBUS_WTP_METHOD_SET_WTP_NTPCLIENT      "set_ap_ntpclient"
#define WID_DBUS_WTP_METHOD_UPDATE_WTP_CONFIG      "update_ap_config"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT      "set_ap_reboot"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_ALL      "set_ap_reboot_all"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_WLANID      "set_ap_reboot_by_wlanid"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_INTERFACE      "set_ap_reboot_by_interface"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_MODEL      "set_ap_reboot_by_model"
#define WID_DBUS_WTP_METHOD_SET_WTP_REBOOT_BY_LIST      "set_ap_reboot_by_list"

#define WID_DBUS_WTP_METHOD_GET_WTP_BSS_PKT_INFO      "get_wtp_bss_pkt_info"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_BSS_PKT_INFO      "show_wtp_bss_pkt_info"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO      "show_wtp_radio_pkt_info"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO_TUNNEL      "show_wtp_radio_pkt_info_tunnel"
#define WID_DBUS_WTP_METHOD_SHOW_WTP_ETH_PKT_INFO      "show_wtp_eth_pkt_info"

#define WID_DBUS_WTP_METHOD_SHOW_WTP_WIFI_SNR_INFO      "show_wtp_wifi_snr_info"

#define WID_DBUS_WTP_METHOD_SET_PORT_VLAN_SECURITY    	"wtp_set_port_vlan_security"  /*sz20080827*/
#define WID_DBUS_WTP_METHOD_SET_PORT_VLAN_ENABLE   	"wtp_set_port_vlan_enable"  /*sz20080827*/
#define WID_DBUS_WTP_METHOD_SET_SECURITY   	        "wtp_set_security"  /*sz20080827*/
#define WID_DBUS_WTP_METHOD_SET_VLAN_LIST_ENABLE   	  "wtp_set_vlan_list_enable"  /*sz20080827*/
#define WID_DBUS_WTP_METHOD_DELETE_AP_MODEL		"wtp_delete_model"
#define WID_DBUS_WTP_METHOD_SET_AP_TIMER	   	"wtp_set_ap_echotimer"
#define WID_DBUS_WTP_METHOD_SET_AP_CHECKTIMER	   	"wtp_set_ap_checktimer"
#define WID_DBUS_WTP_METHOD_SET_AP_COLLECT_TIME		"wtp_set_ap_collect_time"/*nl add 20100713*/
#define WID_DBUS_WLAN_METHOD_DELETE_IF            "wlan_delete_if"
#define WID_DBUS_WLAN_METHOD_SHOW_IF            "wlan_show_if"
#define WID_DBUS_WLAN_METHOD_HIDE_ESSID         "wlan_hide_essid"
#define WID_DBUS_WLAN_METHOD_L3IF_POLICY         "wlan_l3if_policy"
#define WID_DBUS_WLAN_METHOD_L3IF_POLICY2         "wlan_l3if_policy2"
#define WID_DBUS_WLAN_METHOD_SET_NAS_IDENTIFIER    "set_interface_identifier"	
#define WID_DBUS_WLAN_METHOD_REMOVE_NAS_IDENTIFIER    "remove_interface_identifier"
#define WID_DBUS_WLAN_METHOD_ROAMING_POLICY		"wlan_roaming_policy" /*zhanglei add*/
#define WID_DBUS_WLAN_METHOD_SET_BRIDGE_ISOLATION		"wlan_set_bridge_isolation" 
#define WID_DBUS_WLAN_METHOD_SET_BRIDGE_MULTICAST_ISOLATION		"wlan_set_bridge_multicast_isolation" 
#define WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION		"wlan_show_bridge_isolation" 
#define WID_DBUS_WLAN_METHOD_SET_BRIDGE_SAMEPORTSWICTH		"wlan_set_bridge_sameportswitch" 
#define WID_DBUS_WLAN_METHOD_SET_ESSID		"wlan_set_essid" 
#define WID_DBUS_WLAN_METHOD_SET_ASCII_ESSID   "wlan_set_ascii_essid"    /*fengwenchao add 20110307*/
#define WID_DBUS_WLAN_METHOD_WHOLE_WLAN_TRAFFIC_LIMIT_VALUE		"wlan_set_traffic_limit_value"
#define WID_DBUS_WLAN_METHOD_WHOLE_WLAN_SEND_TRAFFIC_LIMIT_VALUE		"wlan_set_send_traffic_limit_value"

#define WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_TRAFFIC_LIMIT_VALUE		"wlan_set_station_average_traffic_limit_value"
#define WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_SEND_TRAFFIC_LIMIT_VALUE	"wlan_set_station_average_send_traffic_limit_value"
#define WID_DBUS_WLAN_METHOD_WHOLE_WLAN_BSS_MULTI_USER_OPTIMIZE_SWITCH		"wlan_set_all_bss_multi_user_optimize_switch"


#define WID_DBUS_WLAN_METHOD_UNDO_WLAN_VLANID		"wid_dbus_wlan_undo_wlan_vlanid"
#define WID_DBUS_WLAN_METHOD_SET_WLAN_VLANID		"wid_dbus_wlan_set_wlan_vlanid"
#define WID_DBUS_WLAN_METHOD_SET_NAS_PORT_ID		"wid_dbus_wlan_set_nas_port_id"			//mahz add 2011.5.25
#define WID_DBUS_WLAN_METHOD_SET_WLAN_HOTSPOTID		"wid_dbus_wlan_set_hotspotid"			
#define WID_DBUS_WLAN_METHOD_CLEAN_WLAN_HOTSPOTID		"wid_dbus_wlan_clean_hotspotid"			
#define WID_DBUS_WLAN_METHOD_SET_TUNNEL_WLAN_VLAN		"wid_dbus_wlan_set_tunnel_wlan_vlan"
#define WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN		"wid_dbus_wlan_show_tunnel_wlan_vlan"

#define WID_DBUS_WLAN_METHOD_SET_WLAN_VLAN_PRIORITY		"wid_dbus_wlan_set_wlan_vlan_priority"
#define WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO		"wid_dbus_wlan_show_wlan_vlan_info"
#define WID_DBUS_WLAN_METHOD_SHOW_WLAN_PTK_INFO		"wid_dbus_wlan_show_wlan_ptk_info"
/*wlan for br area*/
#define WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR         "wlan_l3if_policy_br"
#define WID_DBUS_WLAN_METHOD_L3IF_POLICY2_BR         "wlan_l3if_policy2_br"

/*added end 20080701*/

#define WID_DBUS_WTP_OBJPATH	"/aw/wid/wtp"
#define WID_DBUS_WTP_INTERFACE	"aw.wid.wtp"



#define	WID_DBUS_RADIO_OBJPATH	"/aw/wid/radio"
#define WID_DBUS_RADIO_INTERFACE	"aw.wid.radio"

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_STXP
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	UNIT16	Radio_TXP//tx power of radio
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_RADIO_METHOD_SET_TXP	"radio_set_txpower"
#define WID_DBUS_RADIO_METHOD_SET_TXPOF	"radio_set_txpowerof"

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_SCHAN
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	BYTE	Radio_Chan//channel of radio
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_RADIO_METHOD_SET_CHAN	"radio_set_channel"
/*added by weiay 20080714*/

#define WID_DBUS_RADIO_METHOD_SET_RATE	"radio_set_rate"
#define WID_DBUS_RADIO_METHOD_SET_MODE	"radio_set_mode"
#define WID_DBUS_RADIO_METHOD_SET_FRAGMENTATION	"radio_set_fragmentation"
#define WID_DBUS_RADIO_METHOD_SET_BEACON	"radio_set_beacon"
#define WID_DBUS_RADIO_METHOD_SET_RTSTHROLD	"radio_set_hidessic"
#define WID_DBUS_RADIO_METHOD_SET_DTIM	"radio_set_dtim"
#define WID_DBUS_RADIO_METHOD_SET_STATUS	"radio_set_status"
#define WID_DBUS_RADIO_METHOD_SET_WDS_STATUS	"wds_set_status"
#define WID_DBUS_RADIO_METHOD_SET_PREAMBLE	"radio_set_preamble"
#define WID_DBUS_RADIO_METHOD_SET_LONGRETRY	"radio_set_longretry"
#define WID_DBUS_RADIO_METHOD_SET_SHORTRETRY	"radio_set_shortretry"
#define WID_DBUS_RADIO_METHOD_SET_BSS_MAX_STA	"radio_set_max_bss_sta"


#define WID_DBUS_RADIO_METHOD_SET_SUPPORT_RATELIST	"radio_set_support_ratelist"
#define WID_DBUS_RADIO_METHOD_SET_MAX_RATE	"radio_set_max_rate"
#define WID_DBUS_RADIO_METHOD_SET_BSS_L3_POLICY	"radio_set_bss_l3_policy"
#define WID_DBUS_RADIO_METHOD_SET_BSS_MAX_THROUGHPUT	"radio_set_bss_max_throughput"
#define WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT	"radio_show_bss_max_throughput"
#define WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE	"radio_show_channel_change"
#define WID_DBUG_RADIO_METHOD_SET_11N_RATE_PARAS    "radio_set_11n_rate_paras"

#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID	"radio_apply_wlanid_base_vlanid"
#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID	"radio_apply_wlanid_clean_vlanid"
#define WID_DBUS_RADIO_METHOD_RADIO_CPE_CHANNEL_APPLY_WLANID_BASE_VLANID	"radio_cpe_channel_apply_wlanid_base_vlanid"
#define WID_DBUS_RADIO_METHOD_RADIO_CPE_CHANNEL_APPLY_WLANID_CLEAN_VLANID	"radio_cpe_channel_apply_wlanid_clean_vlanid"

#define WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER  "check_radio_member"
#define WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME	"radio_receive_data_dead_time"
#define WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME_SHOW	"radio_receive_data_dead_time_show"
#define WID_DBUS_RADIO_METHOD_SET_BSS_L3_IF_WLAN_BR	"set_bss_l3_if_wlan_br"
#define WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS	"show_radio_qos"
#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_NAS_PORT_ID	"radio_apply_wlanid_base_nas_port_id"		//mahz add 2011.5.30
#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_NAS_PORT_ID	"radio_apply_wlanid_clean_nas_port_id"
#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_HOTSPOT_ID  "radio_apply_wlanid_base_hotspotid"
#define WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_HOTSPOT_ID  "radio_apply_wlanid_clean_hotspotid"

/*
arg lists for method WID_DBUS_CONF_METHOD_RADIO_APPLY_WLAN
 in arg list:
   	BYTE	Radio_G_ID//Global id of Radio
	BYTE	WlanID//id of wlan which radio apply
out arg list:  // in the order as they are appended in the dbus message.
	UNIT32	ret//success or fail(reason)
*/
#define WID_DBUS_RADIO_METHOD_APPLY_WLAN	"radio_apply_wlan"
#define WID_DBUS_RADIO_METHOD_DELETE_WLAN	"radio_delete_wlan"
#define WID_DBUS_RADIO_METHOD_APPLY_WLAN_BASE_ESSID	    "radio_apply_wlan_base_essid"
#define WID_DBUS_RADIO_METHOD_DELETE_WLAN_BASE_ESSID	"radio_delete_wlan_base_essid"

#define WID_DBUS_RADIO_METHOD_ENABLE_WLAN	"radio_enable_wlan"
#define WID_DBUS_RADIO_METHOD_DISABLE_WLAN	"radio_diable_wlan"

#define WID_DBUS_RADIO_METHOD_WDS_WLAN_SET	"radio_wds_wlan_set"
#define WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO	"show_radio_wds_bssid_list"

#define WID_DBUS_RADIO_METHOD_REVOVER_DEFAULT_CONFIG	"radio_default_config"

#define WID_DBUS_RADIO_METHOD_SET_MAX_THROUGHOUT	"set_radio_max_thoughout"
#define WID_DBUS_RADIO_METHOD_SET_RADIO_L2_ISOLATION_ABLE          "set_radio_l2_isolation_able"
#define WID_DBUS_RADIO_METHOD_11N_SET_RADIO_CWMMODE          "set_radio_11n_cwmmode"
#define WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST	"show_radio_bss_list"

#define WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL      "set_ap_radio_auto_channel"
#define WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL_CONT      "set_ap_radio_auto_channel_cont"
#define WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_DIVERSITY      "set_ap_radio_diversity"
#define WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_TXANTENNA      "set_ap_radio_txantenna"
#define WID_DBUS_RADIO_METHOD_INTER_VAP_FORVARDING_ABLE		"radio_inter_vap_forwarding_able"
#define WID_DBUS_RADIO_METHOD_INTRA_VAP_FORVARDING_ABLE		"radio_intra_vap_forwarding_able"
#define WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_PERIOD		"radio_set_keep_alive_period"
#define WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_IDLE_TIME	"radio_set_keep_alive_idle_time"
#define WID_DBUS_RADIO_METHOD_SET_CONGESTION_AVOID_STATE	"radio_set_congestion_avoid_state"



#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_ABLE	"radio_wlan_traffic_limit_able"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_VALUE	"radio_wlan_traffic_limit_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_VALUE	"radio_wlan_traffic_limit_average_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_AVERAGE_VALUE  "radio_wlan_traffic_limit_cancel_average_value"  //fengwenchao add 20130416 for AXSSZFI-1374
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_AVERAGE_SEND_VALUE "radio_wlan_traffic_limit_cancel_average_send_value"  //fengwenchao add 20130416 for AXSSZFI-1374

#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE	"radio_wlan_traffic_limit_sta_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE	"radio_wlan_traffic_limit_cancel_sta_value"

#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_SEND_VALUE	"radio_wlan_traffic_limit_send_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_SEND_VALUE	"radio_wlan_traffic_limit_average_send_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE	"radio_wlan_traffic_limit_sta_send_value"
#define WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE	"radio_wlan_traffic_limit_cancel_sta_send_value"
#define WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID	"radio_wlan_set_sta_vlanid"
#define WID_DBUS_RADIO_METHOD_WLAN_SET_STA_DHCP_BEFORE_AUTHERIZED	"radio_wlan_set_sta_dhcp_before_autherized"
#define WID_DBUS_RADIO_METHOD_WLAN_SET_STA_IP_MAC_BINDING	"radio_wlan_set_sta_ip_mac_binding"


#define	WID_DBUS_QOS_OBJPATH	"/aw/wid/qos"
#define WID_DBUS_QOS_INTERFACE	"aw.wid.qos"

#define WID_DBUS_QOS_METHOD_ADD_DEL_QOS		"add_del_qos"

#define WID_DBUS_QOS_METHOD_SHOW_QOS_LIST	"show_qos_list"
#define WID_DBUS_QOS_METHOD_SHOW_QOS		"show_qos"
#define WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO		"show_radio_apply_qos_info"
#define WID_DBUS_QOS_METHOD_CONFIG_QOS		"config_qos"

#define WID_DBUS_QOS_METHOD_SET_QOS_INFO		"set_wid_radio_qos_info"
#define WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT		"set_wid_client_qos_info"
#define WID_DBUS_QOS_METHOD_SET_QOS_MAP		"set_wid_qos_wmm_map"
#define WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P		"set_wid_qos_wmm_map_dot1p"
#define WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM		"set_wid_qos_dot1p_map_wmm"

#define WID_DBUS_RADIO_METHOD_APPLY_QOS	"radio_apply_qos"
#define WID_DBUS_RADIO_METHOD_DELETE_QOS	"radio_delete_qos"
/*set wireless qos info for mib*/
#define WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH		"set_wid_qos_total_bandwidth"
#define WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO		"show_qos_extension_info"
#define WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER		"set_wid_qos_flow_parameter"
#define WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER_V2		"set_wid_qos_flow_parameter_v2"
#define WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE		"set_wid_qos_flow_able"
#define WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE_V2		"set_wid_qos_flow_able_v2"
#define WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER		"set_wid_qos_parameter"
#define WID_DBUS_CONF_METHOD_DELETE_RADIO_WITH_QOS_PROFILE     "delete_radio_with_qos_profile"   //fengwenchao add 20110427
#define WID_DBUS_QOS_METHOD_SET_QOS_POLICY		"set_wid_qos_policy"
#define WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME		"set_wid_qos_policy_name"
#define WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME		"set_wid_qos_manage_arithmetic_name"

/*ethereal bridge infomation area*/
#define	WID_DBUS_EBR_OBJPATH	"/aw/wid/ebr"
#define WID_DBUS_EBR_INTERFACE	"aw.wid.ebr"

#define WID_DBUS_EBR_METHOD_ADD_DEL_EBR		"add_del_ebr"
#define WID_DBUS_EBR_METHOD_SHOW_EBR_LIST	"show_ebr_list"
#define WID_DBUS_EBR_METHOD_SHOW_EBR		"show_ebr"
#define WID_DBUS_EBR_METHOD_CONFIG_EBR		"config_ebr"
#define WID_DBUS_EBR_METHOD_CONFIG_EBR_ENABLE		"config_ebr_enable"

#define WID_DBUS_EBR_METHOD_SET_BRIDGE_ISOLATION		"ebr_set_bridge_isolation" 
#define WID_DBUS_EBR_METHOD_SET_BRIDGE_MULTICAST_ISOLATION		"ebr_set_bridge_multicast_isolation" 

#define WID_DBUS_EBR_METHOD_SET_BRIDGE_UCAST_SOLICT		"set_bridge_ucast_solicit"			/* Huang Leilei add, 2012-11-12 11:24 */
#define	WID_DBUS_EBR_METHOD_SET_BRIDGE_MCAST_SOLICT		"set_bridge_mcast_solicit"			/* Huang Leilei add, 2012-11-12 20:09 */

#define WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_IF		"ebr_set_ebr_add_del_if" 
#define WID_DBUS_EBR_METHOD_SET_BRIDGE_SAMEPORTSWITCH		"ebr_set_bridge_sameportswitch" 
#define WID_DBUS_EBR_METHOD_EBR_INTERFACE_EBR	"ebr_interface_ebr"
#define WID_DBUS_EBR_METHOD_SET_MULTICAST_FDB_LEARN		"ebr_set_multicast_fdb_learn" 

#define WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_START	"ebr_show_running_config_start"
#define WID_DBUS_CONF_METHOD_EBR_SHOW_RUNNING_CONFIG_END	"ebr_show_running_config_end"

/*
Wid config infomation 
*/

#define WID_DBUS_CONF_METHOD_ADD_OUI          "add_legal_oui"
#define WID_DBUS_CONF_METHOD_ADD_ESSID          "add_legal_essid"
#define WID_DBUS_CONF_METHOD_MODIFY_ESSID          "modify_legal_essid"
#define WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC          "add_attack_ap_mac"

#define WID_DBUS_CONF_METHOD_DEL_OUI          "del_legal_oui"
#define WID_DBUS_CONF_METHOD_DEL_ESSID          "del_legal_essid"
#define WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC       "del_attack_ap_mac"

#define WID_DBUS_CONF_METHOD_OUI_SHOW          "show_manufacturer_oui_list"
#define WID_DBUS_CONF_METHOD_ESSID_SHOW          "show_legal_essid_list"
#define WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW          "show_attack_mac_list"




#define WID_DBUS_CONF_METHOD_WIDCONFIG       "show_wid_config"	
#define WID_DBUS_CONF_METHOD_LEV3_PROTOCOL    "set_wid_lev3_protocol"	
#define WID_DBUS_CONF_METHOD_SECURITY_TYPE    "set_wid_security_type"	
#define WID_DBUS_CONF_METHOD_HW_VERSION       "set_wid_hw_version"	
#define WID_DBUS_CONF_METHOD_SW_VERSION       "set_wid_sw_version"
#define WID_DBUS_CONF_METHOD_AC_NAME          "set_wid_ac_name"

#define WID_DBUS_CONF_METHOD_MAX_WTP          "set_wid_max_wtp"
#define WID_DBUS_CONF_METHOD_MAX_MTU          "set_wid_max_mtu"
#define WID_DBUS_CONF_METHOD_LOG_SWITCH          "set_wid_log_switch"
#define WID_DBUS_CONF_METHOD_LOG_SIZE          "set_wid_log_size"
#define WID_DBUS_CONF_METHOD_LOG_LEVEL          "set_wid_log_level"
#define WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN          "set_wid_daemonlog_debug_open"

#define WID_DBUS_CONF_METHOD_AP_SCANNING          "set_ap_scanning"
#define WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL          "set_ap_scanning_report_interval"

#define WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO          "update_ap_scanning_info"
#define WID_DBUS_CONF_METHOD_WHITELIST          "set_mac_whitelist"
#define WID_DBUS_CONF_METHOD_BLACKLIST          "set_mac_blacklist"
#define WID_DBUS_CONF_METHOD_WHITELIST_DELETE          "delete_mac_whitelist"
#define WID_DBUS_CONF_METHOD_BLACKLIST_DELETE          "delete_mac_blacklist"
#define WID_DBUS_CONF_METHOD_ROGUE_AP_LIST          "show_rogue_ap_list"
#define WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD			"show_rogue_ap_threshold"

#define WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1          "show_rogue_ap_list_v1"
#define WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID          "show_rogue_ap_list_bywtpid"
#define WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION		"dynamic_channel_selection"
#define WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID          "show_neighbor_ap_list_bywtpid"
#define WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST   "show_neighbor_ap_list"    //fengwenchao add 20101220
#define WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2          "show_neighbor_ap_list_bywtpid2"
#define WID_DBUS_CONF_METHOD_WHITELIST_SHOW          "show_mac_whitelist"
#define WID_DBUS_CONF_METHOD_BLACKLIST_SHOW        "show_mac_blacklist"
/*country code area*/
#define WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE          "set_system_country_code"
#define WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE          "undo_system_country_code"
#define WID_DBUS_CONF_METHOD_SHOW_COUNTRY_CODE          "show_system_country_code"

#define WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL		"transmit_power_control"
#define WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD		"set_txpower_threshold"
#define WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD  "set_rogue_ap_threshold_trap"
#define WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD		"set_coverage_threshold"
#define WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO	"show_neighbor_rssi_info_bywtpid"
#define WID_DBUS_CONF_METHOD_CONTROL_SCOPE    "set_transmit_power_control_scope"	
/*auto ap area*/
#define WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH	"set_auto_ap_login_switch"
#define WID_DBUS_CONF_METHOD_SET_WID_MEMORY_TRACE_SWITCH	"set_memory_trace_switch"
#define WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE	"set_auto_ap_login_binding_l3_interface"
#define WID_DBUS_CONF_METHOD_SET_WID_LISTEN_L3_INTERFACE	"set_wid_listen_l3_interface"
#define WID_DBUS_CONF_METHOD_SET_WID_LISTEN_IP	"set_wid_listen_ip"
#define WID_DBUS_CONF_METHOD_SHOW_WID_LISTEN_L3_INTERFACE	"show_wid_listen_l3_interface"
#define WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID	"set_auto_ap_login_binding_wlanid"
#define WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID	"del_auto_ap_login_binding_wlanid"
#define WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH	"set_auto_ap_login_save_config_switch"
#define WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG	"show_auto_ap_login_save_config"
#define WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG	"clear_auto_ap_login_config"
#define WID_DBUS_CONF_METHOD_AP_DATA_EXCLUDE_MULTICAST          "set_ap_data_exclude_multicast"
#define WID_DBUS_CONF_METHOD_AP_STATISTICS          "set_ap_statistics"
#define WID_DBUS_CONF_METHOD_AP_STATISTICS_INTERVAL			"set_ap_statistics_interval"
#define WID_DBUS_CONF_METHOD_AP_TIMESTAMP          "set_ap_timestamp"
#define WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST      "show_ap_statistics_list"
#define WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR      "show_ap_ip_bywtpid"


#define WID_DBUS_CONF_METHOD_SET_WID_AP_MAX_THROUGHOUT	"set_ap_max_thoughout"

#define WID_DBUS_CONF_METHOD_SET_WID_AP_EXTENSION_COMMAND	"set_ap_extension_command"
#define WID_DBUS_CONF_METHOD_SET_WID_AP_LONGITUDE_LATITUDE_COMMAND	"set_ap_longitude_latitude_command"
#define WID_DBUS_CONF_METHOD_SET_WID_AP_OPTION60_PARAMETER "set_ap_special_command"
#define WID_DBUS_CONF_METHOD_SET_WID_AP_IP_GATEWAY "set_ap_ipaddr_command_cmdset_ap_ipaddr_command_cmd"
#define WID_DBUS_CONF_METHOD_TCPDUMP_AP_EXTENSION_COMMAND "tcpdump"

/*mib area*/
#define WID_TRAP_OBJPATH	"/aw/trap"
#define WID_TRAP_INTERFACE	"aw.trap"

#define WID_DBUS_TRAP_WID_WTP_ENTER_IMAGEDATA_STATE		"wid_wtp_enter_imagedata_state"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_CHANGE		"wid_wtp_channel_change"
#define WID_DBUS_TRAP_WID_WLAN_ENCRYPTION_TYPE_CHANGE		"wid_wlan_encryption_type_change"
#define WID_DBUS_TRAP_WID_WLAN_PRESHARED_KEY_CHANGE		"wid_wlan_preshared_key_change"
#define WID_DBUS_TRAP_WID_WTP_TRANFER_FILE		"wid_wtp_transfer_file"
#define WID_DBUS_TRAP_WID_WTP_UPDATE_SUCCESSFUL  "wid_wtp_update_successful"   /*fengwenchao add 20110216*/
#define WID_DBUS_TRAP_WID_WTP_UPDATE_FAIL  "wid_wtp_update_fail"   /*fengwenchao add 20110216*/

#define WID_DBUS_TRAP_WID_WTP_CODE_START		"wid_dbus_trap_wtp_code_start"
#define WID_DBUS_TRAP_WID_WTP_ELECTRIFY_REGISTER_CIRCLE		"wid_dbus_trap_wtp_electrify_register_circle"
#define WID_DBUS_TRAP_WID_WTP_AP_DOWN		"wid_dbus_trap_wtp_ap_power_off"
#define WID_DBUS_TRAP_WID_WTP_STA_UNAUTHORIZED_MAC	"wid_dbus_trap_wtp_sta_unauthroized_mac"
#define WID_DBUS_TRAP_WID_WTP_CONFIGURE_ERROR		"wid_dbus_trap_wtp_configure_error"
#define WID_DBUS_TRAP_WID_WTP_STA_FLOW_RX_TX_OVERFLOW	"wid_dbus_trap_sta_flow_rx_tx_overflow"
#define WID_DBUS_TRAP_WID_WTP_STA_ONLINE_FULL		"wid_dbus_trap_wtp_sta_online_full"
#define WID_DBUS_TRAP_WID_WTP_AP_REBOOT		"wid_dbus_trap_wtp_ap_reboot"
#define WID_DBUS_TRAP_WID_WTP_IP_CHANGE_ALARM		"wid_dbus_trap_wtp_ip_change_alarm"
#define WID_DBUS_TRAP_WID_WTP_ACTIMESYNCHROFAILURE		"wid_dbus_trap_wtp_ap_ACTimeSynchroFailure"
#define WID_DBUS_TRAP_WID_WTP_DEVICE_INTERFERENCE_DETECTED		"wid_dbus_trap_wtp_device_interference"
#define WID_DBUS_TRAP_WID_WTP_SHOW_RUNTIME		"wid_dbus_trap_wtp_show_runtime"
#define WID_DBUS_TRAP_WID_WTP_REMOTE_RESTART		"wid_dbus_trap_wtp_remote_restart"
#define WID_DBUS_TRAP_WID_WTP_DIVORCE_NETWORK		"wid_dbus_trap_wtp_divorce_network"
#define WID_DBUS_TRAP_WID_WTP_AP_FLASH_WRITE_FAIL		"wid_dbus_trap_wtp_ap_flash_write_fail"

#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_RUNTIME		"wid_dbus_show_wtp_runtime"
#define WID_DBUS_CONF_METHOD_SET_WID_WTP_LOCATION		"wid_dbus_set_wtp_location"
#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_LOCATION		"wid_dbus_show_wtp_location"
#define WID_DBUS_CONF_METHOD_SET_WID_WTP_NETID		"wid_dbus_set_wtp_netid"
#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_NETID		"wid_dbus_show_wtp_netid"
#define WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME		"wid_dbus_set_monitor_time"
#define WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME		"wid_dbus_set_sample_time"
#define WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO		"wid_dbus_show_sample_info"
#define WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE		"wid_dbus_set_monitor_enable"
#define WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE		"wid_dbus_set_sample_enable"
#define WID_DBUS_CONF_METHOD_SET_WID_IPFWD		"wid_dbus_set_ipfwd"
#define WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD     "wid_dbus_show_ipfwd"

#define WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER	"wid_set_ap_update_timer"
#define WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER	"wid_show_ap_update_timer"

#define WID_DBUS_CONF_METHOD_SET_AP_L2_ISOLATION_ABLE          "set_ap_l2_isolation_able"
#define WID_DBUS_CONF_METHOD_SET_AP_PREVENT_DOS_ATTACK          "set_ap_prevent_dos_attack"
#define WID_DBUS_CONF_METHOD_SET_AP_IGMP_SNOOPING          "set_ap_igmp_snooping"
#define WID_DBUS_CONF_METHOD_SHOW_AP_MIB_INFO          "show_ap_mib_info"

#define WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD          "set_ap_cm_threshold"
#define WID_DBUS_CONF_METHOD_SET_WTP_ROGUEAP_ROGUETERMINAL_CPU_MEM_TRAP_THRESHOLD          "set_wtp_rogueap_terminal_cpu_mem_trap_threshold"
#define WID_DBUS_CONF_METHOD_SET_WTP_COLLECT_TIME          "set_wtp_trap_collect_time"
#define WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD          "show_ap_cm_threshold"
#define WID_DBUS_CONF_METHOD_SET_WTP_TRAP_SWITCH          "set_wtp_trap_swith"
#define WID_DBUS_CONF_METHOD_SET_WTP_SEQNUM_SWITCH          "set_wtp_seqnum_swith"  /*wcl add*/
#define WID_DBUS_CONF_METHD_SET_WID_ROGUE_DANGER_UNSAFE_ATTACK_TRAP_STATE    "set_wid_rogue_danger_unsafe_attack_trap_state"    /*fengwenchao add 20110221*/
#define WID_DBUS_CONF_METHOD_SHOW_AP_TRAP_ROGUE_AP_TERMINAL_CPU_MEM_THRESHOLD	"show_ap_trap_rogue_ap_terminal_cpu_mem_threshold"
#define WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION          "show_ac_balance_configuration"
#define WID_DBUS_CONF_METHOD_SHOW_AP_CM_STATISTICS          "show_ap_cm_statistics"

#define WID_DBUS_CONF_METHOD_AP_CHANNEL_DISTURB_TRAP          "set_ap_channel_disturb_trap"
#define WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION      "show_ap_model_infomation"
#define WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION      "show_ap_model_code_infomation"

#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION      "show_ap_extension_infomation"
#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V2      "show_ap_extension_infomation_v2"
#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V3      "show_ap_extension_infomation_v3"
#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V4      "show_ap_extension_infomation_v4"

#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_MAX_POWER      "show_ap_max_power"

#define WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG      "show_rrm_config"
#define WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL      "show_ap_txpower_control"
#define WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL      "set_receiver_signal_level"
#define WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL      "show_receiver_signal_level"

#define WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE		"wid_dbus_trap_wtp_channel_device_interference"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE		"wid_dbus_trap_wtp_channel_ap_interference"
#define WID_DBUS_TRAP_WID_WTP_NEIGHBOR_CHANNEL_AP_INTERFERENCE		"wid_dbus_trap_wtp_neighbor_channel_ap_interference"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE		"wid_dbus_trap_channel_terminal_interference"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR		"wid_dbus_trap_channel_count_minor"

#define WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_DANGER_AP		"wid_dbus_trap_wtp_ac_discovery_danger_ap"
#define WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE		"wid_dbus_trap_wtp_ac_discovery_cover_hole"

#define WID_DBUS_TRAP_WID_AP_CPU_THRESHOLD		"wid_dbus_trap_ap_cpu_threshold"
#define WID_DBUS_TRAP_WID_AP_MEM_THRESHOLD		"wid_dbus_trap_ap_mem_threshold"
#define WID_DBUS_TRAP_WID_AP_TEMP_THRESHOLD		"wid_dbus_trap_ap_temp_threshold"
#define WID_DBUS_TRAP_WID_AP_ROGUE_THRESHOLD		"wid_dbus_trap_ap_rogue_threshold"
#define WID_DBUS_TRAP_WID_AP_WIFI_IF_ERROR		"wid_dbus_trap_ap_wifi_if_error"
#define WID_DBUS_TRAP_WID_AP_ATH_ERROR		"wid_dbus_trap_ap_ath_error"
#define WID_DBUS_TRAP_WID_AP_RRM_STATE_CHANGE		"wid_dbus_trap_ap_rrm_state_change"
#define WID_DBUS_TRAP_WID_AP_RUN_QUIT		"wid_dbus_trap_ap_run_quit"

#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_SAMPLE_THROUGHPUT_INFO		"wid_dbus_show_sample_throughput_info"
#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_WLAN_VLAN_INFO		"wid_dbus_show_wlan_vlan_info"
#define WID_DBUS_CONF_METHOD_SHOW_WID_WTP_ETH_IF_INFO		"wid_dbus_show_eth_if_info"
#define WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG   "set_wid_watch_dog"	
#define WID_DBUS_CONF_METHOD_SHOW_DBUS_COUNT	"wid_show_dbus_count"
#define WID_DBUS_CONF_METHOD_SET_DBUS_COUNT	"wid_set_dbus_count"
/********************************BEGIN*********************************/
/*add by nl for showing wtp information for mib need 20100507*/
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_BASIC_INFORMATION		"show_all_wtp_basic_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_COLLECT_INFORMATION		"show_all_wtp_collect_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_PARA_INFORMATION		"show_all_wtp_para_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_IFSTATS_INFORMATION		"show_all_wtp_wireless_ifstats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DEVICE_INFORMATION		"show_all_wtp_device_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DATA_PKTS_INFORMATION		"show_all_wtp_data_pkts_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_STATS_INFORMATION		"show_all_wtp_stats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_STATS_INFORMATION		"show_all_wlan_stats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_STATS_INFORMATION		"show_all_wlan_ssid_stats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_INFORMATION       "show_all_wtp_information";  /*fengwenchao add 20110617*/
#define WID_DBUS_CONF_METHOD_SHOW_INFO_BYWLANID  "show_info_bywlanid";  /*fengwenchao add 20110617*/
#define WD_DBUS_CONF_METHOD_SHOW_INFO_ALLWLAN      "show_info_allwlan";   /*fengwenchao add 20110617*/
#define WID_DBUS_CONF_METHOD_SHOW_INFO_BYWTPID   "show_info_bywtpid"; /*fengwenchao add 20110617*/
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_IFNAME_INFORMATION		"show_all_wtp_ifname_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_PARA_INFOR_INFORMATION		"show_all_wtp_radio_para_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ETH_PORT_INFORMATION		"show_all_wtp_eth_port_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_STATS_INFORMATION		"show_all_wtp_radio_stats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_CONFIG_INFORMATION	"show_all_wtp_radio_config_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRED_STATS_INFORMATION	"show_all_wtp_wired_ifstats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_STATS_INFORMATION	"show_all_the_wtp_wireless_if_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NEW_WTPWIRELESS_IF_INFORMATION	"show_all_wtp_new_wireless_if_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_NEW_WTP_WIRELESS_IFSTATS_INFORMATION	"show_all_wtp_new_wireless_ifstats_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_ROGUE_WTP_INFORMATION		"show_all_rogue_ap_information"
#define WID_DBUS_CONF_METHOD_HIDE_QUIT_WTP_INFOR_IN_MIB_SHOWING	"hide_quit_wtp_infor_in_mib_showing"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NETWORK_INFO	"show_all_wtp_network_info"/*xdw add for show ap network information of all wtp, 20101215*/
/* zhangshu append 2010-08-24 */
#define WID_DBUS_CONF_METHOD_SET_WTP_TRAP_IGNORE_PERCENT "set_wtp_trap_ignore_percent"
/* zhangshu append 2010-08-21 */
#define WID_DBUS_CONF_METHOD_SHOW_HIDE_QUIT_WTP_INFOR_IN_MIB_SHOWING "show_hide_quit_wtp_info_in_mib_showing"

//mahz add 2011.1.21
#define WID_DBUS_CONF_METHOD_SHOW_RADIO_INFO_BYWTPID_WID "show_radio_info_bywtpid_wid"
#define WID_DBUS_CONF_METHOD_BSS_ADD_MAC_LIST "bss_add_mac_list"		//mahz add 2011.5.12
#define WID_DBUS_CONF_METHOD_BSS_DEL_MAC_LIST "bss_del_mac_list"
#define WID_DBUS_CONF_METHOD_BSS_USE_MAC_LIST "bss_use_mac_list"
#define WID_DBUS_CONF_METHOD_SHOW_BSS_MAC_LIST "show_bss_mac_list"


/*fengwenchao add 20110329*/
#define WID_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE  "show_statistics_infomation_of_all_wtp_whole"

/*luzhenhua append  2010-05-26 */
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WLAN_DATA_PKTS_INFORMATION	"show_all_wtp_wlan_data_pkts_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ATH_STATISTICS_INFOMATION		"show_all_wtp_ath_statistics_information"
#define WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION "show_wid_ssid_config_information_of_all_wtp"
#define WID_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP	"show_wid_terminal_info_of_all_wtp"

/********************************END*********************************/

/*
#define ASD_DBUS_SIG_STA_LEAVE	"signal_sta_leave"



#define ASD_DBUS_SIG_STA_COME   "signal_sta_come"
#define ASD_DBUS_SIG_WTP_DENY_STA   "signal_wtp_deny"
#define ASD_DBUS_SIG_DE_WTP_DENY_STA   "signal_de_wtp_deny"


#define ASD_DBUS_SIG_STA_VERIFY	"signal_sta_verify"
#define ASD_DBUS_SIG_STA_VERIFY_FAILED   "signal_sta_verify_failed"


#define ASD_DBUS_SIG_STA_ASSOC_FAILED   "signal_sta_assoc_failed"
#define ASD_DBUS_SIG_STA_JIANQUAN_FAILED   "signal_sta_jianquan_failed"
*/

#define WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN		"wid_dbus_trap_wtp_wireless_interface_down"
#define WID_DBUS_TRAP_WID_WTP_WIRELESS_INTERFACE_DOWN_CLEAR		"wid_dbus_trap_wtp_wireless_interface_down_clear"

#define ASD_DBUS_SIG_STA_ASSOC_FAILED   "signal_sta_assoc_failed"
#define ASD_DBUS_SIG_STA_JIANQUAN_FAILED   "signal_sta_jianquan_failed"

#define WID_DBUS_CONF_METHOD_SET_MODEL	"set_model"
#define WID_DBUS_CONF_METHOD_SHOW_MODEL	"show_model"
#define WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST	"show_model_list"


#define WID_DBUS_TRAP_WID_WTP_AC_DISCOVERY_COVER_HOLE_CLEAR		"wid_dbus_trap_wtp_ac_discovery_cover_hole_clear"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_DEVICE_INTERFERENCE_CLEAR		"wid_dbus_trap_wtp_channel_device_interference_clear"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_AP_INTERFERENCE_CLEAR		"wid_dbus_trap_wtp_channel_ap_interference_clear"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_TERMINAL_INTERFERENCE_CLEAR		"wid_dbus_trap_channel_terminal_interference_clear"
#define WID_DBUS_TRAP_WID_WTP_CHANNEL_COUNT_MINOR_CLEAR		"wid_dbus_trap_channel_count_minor_clear"

#define WID_DBUS_TRAP_WID_WTP_FIND_UNSAFE_ESSID		"wid_dbus_trap_wtp_find_unsafe_essid"
#define WID_DBUS_TRAP_WID_WTP_FIND_WIDS_ATTACK		"wid_dbus_trap_wtp_find_wids_attack"
#define WID_DBUS_TRAP_WID_SSID_KEY_CONFLICT		"wid_dbus_trap_ssid_key_conflict"

#define WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID          "show_wids_statistics_list_bywtpid"
#define WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST          "show_wids_statistics_list"
#define WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID          "clear_wids_statistics_list_bywtpid"
#define WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST          "clear_wids_statistics_list"

#define WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID          "show_wids_device_list_bywtpid"
#define WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST          "show_wids_device_list"
#define WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_OF_ALL    "show_wids_device_of_all"    //fengwenchao add 20101227
#define WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID          "clear_wids_device_list_bywtpid"
#define WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST          "clear_wids_device_list"

#define WID_BAK_OBJPATH	"/aw/wid/bak"
#define WID_BAK_INTERFACE	"aw.wid.bak"
#define WID_DBUS_WTP_METHOD_MASTER_BAK_SET	"master_and_bak_set"
#define WID_DBUS_AC_METHOD_UPDATE_BAK_INFO	"update_bak_ac_info"
#define WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO	"synchronize_wsm_table_info"
#define WID_DBUS_AC_METHOD_ASD_SYNCHRONIZE_INFO	"synchronize_asd_table_info"
#define WID_DBUS_AC_METHOD_NOTICE_INFO	"notice_vrrp"
#define WID_DBUS_AC_METHOD_VRRP_INFO	"vrrp_info"
#define WID_DBUS_AC_METHOD_VRRP_SOCK_INFO	"vrrp_sock_info"

#define WID_DBUS_WLAN_METHOD_FORMARD_MODE         "wlan_formard_mode"
#define WID_DBUS_WLAN_METHOD_TUNNEL_MODE         "wlan_tunnel_mode"

#define WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL	"set_neighbordead_interval"
#define WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL	"show_neighbordead_interval"

#define WID_DBUS_CONF_METHOD_WTPLIST_BYWLANID "show_wtp_bywlanid"

#define WID_DBUS_WTP_METHOD_SET_UPDATE_PATH		"wtp_set_update_path"
#define WID_DBUS_CONF_METHOD_SHOW_UPDATE_CONFIG	"show_update_config"
#define WID_DBUS_CONF_METHOD_CLEAR_UPDATE_CONFIG	"clear_update_config"
#define WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN          "set_wid_trap_open"
#define WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE          "set_wid_trap_switch_able"
#define WID_DBUS_CONF_METHOD_TRAP_SWITCH_SHOW          "set_wid_trap_switch_show"
#define	WID_DBUS_WTP_METHOD_UPDATE_WTP_IMG	"update_wtp_img"
#define	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_WTP_IMG	"clear_update_wtp_img"
#define	WID_DBUS_WTP_METHOD_UPDATE_WTP_IMG_LIST	"update_wtp_img_list"
#define	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_WTP_IMG_LIST	"clear_update_wtp_img_list"
/*mahz add for ap upgrade automatically*/
#define WID_DBUS_WTP_METHOD_BIND_MODEL_WITH_FILE		"wtp_bind_model_with_file"
#define WID_DBUS_WTP_METHOD_SET_AP_UPDATE_BASE_MODEL	"wtp_set_ap_update_base_model"
#define WID_DBUS_WTP_METHOD_SHOW_MODEL_BIND_INFO		"show_model_bind_info"
#define WID_DBUS_WTP_METHOD_DELETE_MODEL_BIND_INFO		"delete_model_bind_info"
#define WID_DBUS_CONF_METHOD_CLEAR_ONE_MODEL_UPDATE_CONFIG	"clear_one_model_update_config"
#define WID_DBUS_WTP_METHOD_SHOW_DETAIL_MODEL_BIND_INFO		"show_model_detail_bind_info"
#define WID_DBUS_WTP_METHOD_SET_AP_UPDATE_COUNT_ONETIME	"wtp_set_ap_update_count_onetime"
#define WID_DBUS_WTP_METHOD_SET_AP_UPDATE_CONTROL	"wtp_set_ap_update_control"
#define WID_DBUS_WTP_METHOD_SHOW_AP_UPGRADE_RESULT_INFO		"show_ap_upgrade_result_info"

#define WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE "update_wtpcompatible"
#define WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT	"wid_set_ap_update_fail_count"
#define WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT	"wid_show_ap_update_fail_count"
#define WID_DBUS_CONF_METHOD_WTPLIST_UPDATE_FAIL_LIST	"show_wtp_list_update_fail"
#define	WID_DBUS_WTP_METHOD_CLEAR_UPDATE_FAIL_WTP_LIST	"clear_update_fail_wtp_list"
#define WID_DBUS_CONF_METHOD_SHOW_OLD_AP_IMG		"show_old_ap_img_op"

/*zhanglei add for bak ac ip list group*/
#define	WID_DBUS_ACIPLIST_OBJPATH		"/aw/wid/aciplist"
#define WID_DBUS_ACIPLIST_INTERFACE		"aw.wid.aciplist"
#define	WID_DBUS_ACIPLIST_METHOD_CONFIG		"config_ac_ip_list"
#define	WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP_LIST_GROUP	"add_ac_ip_list_group"
#define	WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP_LIST_GROUP	"del_ac_ip_list_group"
#define	WID_DBUS_ACIPLIST_METHOD_CONFIG_AC_IP_LIST		"config_ac_ip_list"
#define	WID_DBUS_ACIPLIST_METHOD_ADD_AC_IP				"add_ac_ip"
#define	WID_DBUS_ACIPLIST_METHOD_DEL_AC_IP				"del_ac_ip"
#define	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_PRIORITY		"set_ac_ip_priority"
#define	WID_DBUS_ACIPLIST_METHOD_AC_IP_LIST_GROUP_SWITCH	"ac_ip_list_group_switch"
#define WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST		"show_ac_ip_list"
#define WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE	"show_ac_ip_list_one"
#define WID_DBUS_ACIPLIST_METHOD_SHOW_RUNNING_CONFIG		"show_ac_ip_list_running_config"

#define WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK      "show_ap_network_bywtpid_cmd"
#define WID_DBUS_CONF_METHOD_SET_WID_AP_IP_GATEWAY_DNS "set_ap_network_command_cmd"
#define WID_DBUS_CONF_METHOD_SET_WID_AP_IP_MODE "set_ap_network_ipmode_command_cmd"
#define WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE      "set_ac_num_or_flow_balance"
#define WID_DBUS_CONF_METHOD_AP_HOTREBOOT          "set_ap_hotreboot"
#define WID_DBUS_CONF_AC_LOAD_BALANCE_PARA      "set_ac_balance_para"


#define WID_DBUS_CONF_METHOD_SET_WID_AC_EXTENTION_INFOR_ENABLE      "set_ac_extention_infor_enable"


#define WID_DBUS_WTP_METHOD_SET_WTP_WIDS_POLICY      "set_wtp_wids_policy"
#define WID_DBUS_CONF_METHOD_ADD_WID_MAC          "add_wids_mac"
#define WID_DBUS_CONF_METHOD_DEL_WID_MAC          "del_wids_mac"
#define WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW          "show_wids_mac_list"
//added by weianying for countermeasures rogue ap  and adhoc device
#define WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES          "set_ap_countermeasures"
#define WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE          "set_ap_countermeasures_mode"

#define WID_DBUS_RADIO_METHOD_SET_ACKTIMEOUT_DISTANCE "radio_set_acktimeout_distance"/*wcl add for RDIR-33*/
#define WID_DBUS_RADIO_METHOD_SET_GUARD_INTERVAL	"radio_set_guardinterval"
#define WID_DBUS_RADIO_METHOD_SET_MCS	"radio_set_mcs"
#define WID_DBUS_RADIO_METHOD_SET_CMMODE	"radio_set_cmmode"
#define WID_DBUS_RADIO_SECTOR_SET_CMD	"set_sector_value";
#define WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD	"set_tx_chainmask_value";
#define WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD_V2	"set_tx_chainmask_value_v2";

#define WID_DBUS_RADIO_SECTOR_POWER_SET_CMD		"set_sector_power_value";
#define WID_DBUS_RADIO_NETGEAR_G_SET_CMD[PATH_LEN] =	"set_radio_netgear_supper_g_type_cmd";
#define WID_DBUS_EBR_METHOD_SET_EBR_ADD_DEL_UPLINK		"ebr_set_ebr_add_del_uplink" 
#define WID_DBUS_EBR_METHOD_SHOW_FDB_SUMMARY_DETAIL		"ebr_show_fdb_summary_detail"

#define WID_DBUS_RADIO_METHOD_SET_AMPDU_ABLE	"radio_set_ampdu_able"
#define WID_DBUS_RADIO_METHOD_SET_AMPDU_LIMIT	"radio_set_ampdu_limit"
/* zhangshu add for subframe, 2010-10-09 */
#define WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFRAME "radio_set_ampdu_subframe"

#define WID_DBUS_RADIO_METHOD_SET_MIXED_PURE_N	"radio_set_mixed_pure_n"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_SWITCH = "set_sta_wapi_info_switch"
#define WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_REPORTINTERVAL = "set_sta_wapi_info_reportinterval"

#define WID_DBUS_RADIO_METHOD_SET_CHANNEL_OFFSET =	"radio_set_channel_offset"

//added by weianying 2010/03/23
#define	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_BANLANCE_FLAG	"set_ac_ip_list_banlance"

#define WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH    "set_wid_mac_whitelist"
#define WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH    "set_wid_essid_whitelist"	
#define WID_DBUS_CONF_METHOD_CHANGE_WHITELIST          "change_mac_whitelist"
#define WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST          "change_mac_blacklist"
#define WID_DBUS_WTP_METHOD_SET_WTP_TPC_POLICY  "set_wtp_tpc_policy"
#define	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_THRESHOLD	"set_ac_ip_list_threshold"
#define	WID_DBUS_ACIPLIST_METHOD_SET_AC_IP_DIFF_BANLANCE	"set_ac_ip_list_diff_banlance"

#define WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME	"radio_receive_data_dead_time"
#define WID_DBUS_RADIO_METHOD_SET_RADIO_RX_DATA_DEAD_TIME_SHOW	"radio_receive_data_dead_time_show"

#define WID_DBUS_CONF_METHOD_WIDS_MONITOR_MODE          "wids_monitor_mode"
#define WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE          "wids_scanning_mode"
#define WID_DBUS_CONF_METHOD_WIDS_SCANNING_CHANNEL          "wids_scanning_channel"
#define WID_DBUS_CONF_METHOD_WIDS_SCANNING_MODE_CHANNEL_SHOW          "wids_scanning_mode_channel_show"
#define WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW          "show_access_wtp_vendor_count"
#define WID_DBUS_CONF_METHOD_SET_LICENSE_BINDING		"set_license_binding"	/*xiaodawei append, 20101029*/
#define WID_DBUS_IPERF_WTPIP							"iperf_wtpip"	//xiaodawei add, 20110303
#define WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY          "wids_judge_policy"
#define WID_DBUS_CONF_METHOD_WIDS_JUDGE_POLICY_SHOW          "wids_judge_policy_show"

#define WID_DBUS_CONF_METHOD_TUNNEL_MODE_POLICY          "tunnel_mode_policy"
#define WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFREAM	"radio_set_ampdu_subframe"
#define WID_DBUS_RADIO_METHOD_SET_MIMO	"radio_set_mimo"
#define WID_DBUS_RADIO_METHOD_MOLTI_USER_OPTIMIZE_SWITH "radio_multi_user_optimize_switch"
#define WID_DBUS_WTP_METHOD_5G_SWITH "set_wtp_5g_switch"
#define WID_DBUS_CONF_METHOD_SET_WEB_REPORT_SNR_RANGE		"set_web_report_snr_range"		/* Huangleilei copy from 1.3.18, 20130610 */

#define WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTSWITCH  						"wsm_sta_info_reportswitch"
#define WID_DBUS_RADIO_METHOD_WSM_STA_INFO_REPORTINTERVAL 						"wsm_sta_info_reportinterval" 
#endif
