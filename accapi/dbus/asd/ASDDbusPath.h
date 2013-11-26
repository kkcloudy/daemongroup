#ifndef _ASD_DBUS_PATH_H
#define _ASD_DBUS_PATH_H

char ASD_DBUS_BUSNAME[PATH_LEN]=	"aw.asd";
char ASD_DBUS_OBJPATH[PATH_LEN]=	"/aw/asd";
char ASD_DBUS_INTERFACE[PATH_LEN]=	"aw.asd";


char ASD_DBUS_STA_OBJPATH[PATH_LEN]=	"/aw/asd";
char ASD_DBUS_STA_INTERFACE[PATH_LEN]=	"aw.asd";

char ASD_DBUS_STA_METHOD_SHOWSTA[PATH_LEN]=	"show_sta";
char ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA	[PATH_LEN]="extend_show_sta";
char ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC[PATH_LEN]=	"show_sta_by_mac";	/*ht add 091216*/
char ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID[PATH_LEN]=	"show_info_bywtpid";
char ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID[PATH_LEN]=	"show_info_bywlanid";
char ASD_DBUS_STA_METHOD_SHOW_INFO_ALLWLAN[PATH_LEN]=  "show_info_allwlan";  //fengwenchao add 20101221
char ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID[PATH_LEN]=	"show_radio_info_bywtpid";	/*ht add 090428*/
char ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID[PATH_LEN]=	"show_wapi_info_bywtpid";	/*ht add 090622*/
char ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME[PATH_LEN]=	"show_channel_access_time";	/*ht add 090513*/

char ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID[PATH_LEN]=	"show_mib_info_byradioid";	/*	xm0616*/
char ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID[PATH_LEN]=	"show_wapi_mib_info_byradioid";	/*	xm0623*/

char ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_OF_ALL_WTP[PATH_LEN] = "show_wapi_mib_info_of_all_wtp";		//mahz add 2011.1.19
char ASD_DBUS_STA_METHOD_SHOW_STA_WAPI_MIB_INFO_OF_ALL_WTP[PATH_LEN] = "show_sta_wapi_mib_info_of_all_wtp";	//mahz add 2011.1.24

char ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX[PATH_LEN]=	"show_traffic_limit_by_bssindex";	/*	xm0723*/
char ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO[PATH_LEN]=	"show_traffic_limit_by_radioid";	/*	xm0723*/


char ASD_DBUS_STA_METHOD_SET_AC_FLOW[PATH_LEN]=	 "set_ac_flow";  /*xm0714*/
char ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE[PATH_LEN]=	 "set_extern_balance";  /*xm0714*/

/*show  information for mib BEGIN*/
/*add for mib optimize in ASD by nl 20100707*/
/*----------------------------------------------------------------------------------------------*/
char ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP[PATH_LEN]=	"show_asd_collect_info_of_all_wtp";
char ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_OF_ALL_WTP[PATH_LEN]=	"show_asd_stats_info_of_all_wtp";
char ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_BY_WLAN_OF_ALL_WTP[PATH_LEN]=	"show_asd_stats_info_of_wlan_of_all_wtp";
char ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP[PATH_LEN]=	"show_asd_ssid_stats_info_of_wlan_of_all_wtp";
char ASD_DBUS_STA_METHOD_SHOW_ALLWLAN_STA_NUM[PATH_LEN]= "show_asd_all_wlan_sta_num";  //fengwenchao add 20101223
char ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_BASIC_INFO_BY_WLAN_OF_ALL_WTP[PATH_LEN]=	"show_asd_wlan_wapi_info_of_all_wlan";
char ASD_DBUS_STA_METHOD_SHOW_USER_LINK_INFO_OF_ALL_WTP[PATH_LEN]=		"show_user_link_info_of_all_wtp";
char ASD_DBUS_STA_METHOD_SHOW_WLAN_UNICAST_INFO_BY_WLAN_OF_ALL_WTP[PATH_LEN]=	"show_asd_wlan_unicast_info_of_all_wlan";
char ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_STATS_INFO_OF_ALL_WLAN[PATH_LEN]=	"show_asd_wapi_stats_performance_info_of_all_wlan";
char ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_EXTEND_CONFIG_INFO_BY_WLAN_OF_ALL_WTP[PATH_LEN]=	"show_asd_wlan_wapi_extend_config_info_of_all_wlan";
char ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_BSS_INFO_OF_ALL_WLAN[PATH_LEN]=	"show_asd_wapi_bss_performance_info_of_all_wlan";

char ASD_DBUS_STA_METHOD_SHOW_RADIO_WIRELESS_INFO_BYWTPID[PATH_LEN]=	"show_radio_wireless_info_bywtpid";	/*nl add 20100502*/
char ASD_DBUS_STA_METHOD_SHOW_RADIO_NEW_WIRELESS_INFO_BYWTPID[PATH_LEN]=	"show_radio_new_wireless_info_bywtpid";/*nl add 20100607*/
char ASD_DBUS_STA_METHOD_SHOW_SECURITY_INFO_BYWTPID[PATH_LEN]=	"show_security_info_bywtpid";/*nl add 20100607*/
char ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_INFORMATION[PATH_LEN]=	"show_all_wtp_information";/*nl add 20100622*/
/*----------------------------------------------------------------------------------------------*/
/*add for mib optimize in ASD by lzh 20100708*/
char ASD_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP[PATH_LEN]=	"show_asd_terminal_info_of_all_wtp";	/*lzh add 20100512*/
char ASD_DBUS_STA_METHOD_SHOW_STA_INFO_OF_ALL_WTP[PATH_LEN]=		"show_asd_sta_info_of_all_wtp"; /*liuzhenhua append 2010-05-28*/
char ASD_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION[PATH_LEN]= "show_asd_ssid_config_information_of_all_wtp"; /*liuzhenhua append 2010-05-21*/
char ASD_DBUS_STA_METHOD_SHOW_BSS_STA_NUM_BY_WLANID_AND_RADIOID[PATH_LEN]= "show_asd_bss_sta_num_by_wlanid_and_radioid";
/*show  information for mib END*/
char ASD_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE[PATH_LEN]= "show_statistics_information_of_all_wtp_whole";  //fengwenchao add 20110331
char ASD_DBUS_STA_METHOD_SHOW_DISTINGUISH_INFO_OF_ALL_WTP[PATH_LEN] = "show_asd_distinguish_info_of_all_wtp";		//mahz add 2011.1.17
char ASD_DBUS_STA_METHOD_SHOW_STA_STATIS_INFO_OF_ALL_WTP[PATH_LEN] = "show_asd_sta_statis_info_of_all_wtp";	//mahz add 2011.11.9 for GuangZhou Mobile
char ASD_DBUS_STA_METHOD_SHOW_AC_STA_INFO_OF_ALL[PATH_LEN] = "show_asd_ac_sta_info_of_all";					//mahz add 2011.11.10

char ASD_DBUS_STA_METHOD_KICKSTA[PATH_LEN]=	"kick_sta";
char ASD_DBUS_STA_METHOD_SET_STA_ARP[PATH_LEN]=	"set_sta_arp";
char ASD_DBUS_STA_METHOD_RADIUS_FORCE_STA_DOWNLINE[PATH_LEN] =	"radius_force_sta_downline";		//mahz add 2011.6.3


char ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST[PATH_LEN]=	"wlan_add_MAC_list_sta";/*xm add 08/11/07*/
char ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST[PATH_LEN]=	"wtp_add_MAC_list_sta";/*ht add 08.12.15*/
char ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST[PATH_LEN]=	"bss_add_MAC_list_sta";/*ht add 08.12.15*/
char ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST[PATH_LEN]=	"wlan_del_MAC_list_sta";/*ht add 08.12.22*/
char ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST[PATH_LEN]=	"wtp_del_MAC_list_sta";/*ht add 08.12.22*/
char ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST[PATH_LEN]=	"bss_del_MAC_list_sta";/*ht add 08.12.22*/

char ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST[PATH_LEN]=	"wlan_use_MAC_list_sta";/*ht add 08.12.15*/
char ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST[PATH_LEN]=	"wtp_use_MAC_list_sta";/*ht add 08.12.15*/
char ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST[PATH_LEN]=	"bss_use_MAC_list_sta";/*ht add 08.12.15*/
char ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST[PATH_LEN]=	 "show_all_wlan_MAC_list";/*ht add 08.12.19*/
char ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST	[PATH_LEN]= "show_all_wtp_MAC_list";/*ht add 08.12.19*/
char ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST[PATH_LEN]=	 "show_all_bss_MAC_list";/*ht add 08.12.19*/
char ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST[PATH_LEN]=	 "show_wlan_MAC_list";/*ht add 08.12.22*/
char ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST[PATH_LEN]=	 "show_wtp_MAC_list";/*ht add 08.12.22*/
char ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST[PATH_LEN]=	 "show_bss_MAC_list";/*ht add 08.12.22*/
char ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG[PATH_LEN]= 	"wlan_list_show_running_config"; /*ht add 08.12.17*/
char ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG[PATH_LEN]= 	"wtp_list_show_running_config"; /*ht add 08.12.17*/
char ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG[PATH_LEN]= 	"bss_list_show_running_config"; /*ht add 08.12.17*/
char ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST[PATH_LEN]=	 "show_wlan_wids_MAC_list";/*ht add 09.07.14*/

char ASD_DBUS_STA_METHOD_STALIST[PATH_LEN]=	"show_sta_list";
char ASD_DBUS_STA_METHOD_STALIST_NEW[PATH_LEN]=	"show_sta_list_new";
char ASD_DBUS_STA_METHOD_GET_STA_INFO[PATH_LEN]=	"get_sta_info"; /* xm add 08/12/10*/
char ASD_DBUS_STA_METHOD_GET_STA_INFO_NEW[PATH_LEN]=  "get_sta_info_new";/*fengwenchao add 20120323*/
char ASD_DBUS_STA_METHOD_STA_BASE_INFO[PATH_LEN]=	"show_sta_base_info";
char ASD_DBUS_STA_METHOD_ROAMING_STALIST[PATH_LEN]= "show_roaming_sta_list";

char ASD_DBUS_STA_METHOD_STA_SUMMARY[PATH_LEN]=	"show_sta_summary";


char ASD_DBUS_STA_METHOD_CLEAN_WLAN_STA[PATH_LEN]=	"clean_wlan_sta";
char ASD_DBUS_STA_METHOD_WLAN_STALIST[PATH_LEN]=	"show_wlan_sta_list";
char ASD_DBUS_STA_METHOD_ALL_WLAN_STALIST[PATH_LEN]=  "show_wlan_sta_of_all";  //fengwenchao add 20110113  

char ASD_DBUS_STA_METHOD_WTP_STALIST[PATH_LEN]=	"show_wtp_sta_list";
char ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST[PATH_LEN]=	"extend_show_wtp_sta_list";


char ASD_DBUS_STA_METHOD_ADD_DEL_STA[PATH_LEN]=	"add_del_sta";
char ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT[PATH_LEN]=	"set_sta_traffic_limit";
char ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL[PATH_LEN]=	"cancel_sta_traffic_limit";
char ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT[PATH_LEN]=	"set_sta_send_traffic_limit";
char ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL[PATH_LEN]=	"cancel_sta_send_traffic_limit";
char ASD_DBUS_STA_METHOD_STA_HOTSPOT_MAP_NAS[PATH_LEN] = "asd_sta_hotspotid_map_nas_information";
char ASD_DBUS_STA_METHOD_DEL_HOTSPOT[PATH_LEN] = "asd_dbus_sta_del_hotspotid";//qiuchen add it 
char ASD_DBUS_STA_METHOD_SHOW_HOTSPOT_LIST[PATH_LEN] = "asd_dbus_sta_show_hotspot_list";//qiuchen

char ASD_DBUS_SECURITY_OBJPATH[PATH_LEN]=	"/aw/asd";
char ASD_DBUS_SECURITY_INTERFACE[PATH_LEN]=	"aw.asd";

char ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY[PATH_LEN]=	"config_security";

char ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST[PATH_LEN]=		"show_security_list";

char ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY[PATH_LEN]=		"show_security";

char ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS[PATH_LEN]=		"show_radius";

char ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_ALL[PATH_LEN]=	"show_radius_all";
char ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY	[PATH_LEN]=	"add_del_security";

char ASD_DBUS_SECURITY_METHOD_APPLY_WLAN[PATH_LEN]=		"apply_wlan";

char ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT[PATH_LEN]=	"asd_update_wtp_count";

char ASD_DBUS_SECURITY_METHOD_SET_ACCT[PATH_LEN]=	"set_acct";
char ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH[PATH_LEN]=	"set_wpai_auth";
char ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH[PATH_LEN]=	"set_wpai_certification_path";
char ASD_DBUS_SECURITY_METHOD_DEL_WAPI_CER[PATH_LEN] = "del_wapi_certification";	//weichao add 20110801
char ASD_DBUS_SECURITY_METHOD_SET_WAPI_P12_CERT_PATH[PATH_LEN]=	"set_wpai_p12_certification_path";
char ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT[PATH_LEN]=	"set_wpai_multi_cert";
char ASD_DBUS_SECURITY_METHOD_WPA_GROUP_REKEY_PERIOD[PATH_LEN]=	"wpa_group_rekey_period";
char ASD_DBUS_SECURITY_METHOD_WPA_KEYUPDATE_TIMEOUT_PERIOD[PATH_LEN]= "wpa_keyupdate_timeout_period";
char ASD_DBUS_SECURITY_METHOD_WPA_ONCE_GROUP_REKEY_TIME[PATH_LEN]= "wpa_once_group_rekey_time"; 	//mahz add 2011.1.3
char  ASD_DBUS_SECURITY_METHOD_TRAFFIC_LIMIT_FROM_RADIUS[PATH_LEN]=  	"traffic_limit_form_radius";
char ASD_DBUS_SECURITY_METHOD_SET_EAP_SM_RUN_ACTIVATED[PATH_LEN]="eap_sm_run_activated";//qiuchen 2013.01.07
//qiuchen add it for master_bak radius server 2012.12.11
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AC_RADIUS_NAME[PATH_LEN]=	"asd_dbus_set_ac_radius_name";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_RES_FAIL_SUC_PERCENT[PATH_LEN]= "asd_dbus_set_radius_res_fail_suc_percent";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_ACCESS_TEST_INTERVAL[PATH_LEN]= "asd_dbus_set_radius_access_test_interval";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_CHANGE_REUSE_TEST_TIMER[PATH_LEN]= "asd_dbus_set_radius_server_change_reuse_test_timer";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_HEART_TEST_TYPE[PATH_LEN]= "asd_dbus_set_radius_server_heart_test_type";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_SERVER_BINDING_ENABLE_DISABLE[PATH_LEN]= "asd_dbus_set_radius_server_binding_enable_disable";
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_RADIUS_HEART_TEST_ON_OFF[PATH_LEN]= "asd_dbus_set_radius_heart_test_on_off";
//end

char ASD_DBUS_SECURITY_METHOD_ASD_SOCKET_OPERATE[PATH_LEN]= "asd_socket_operate";	//mahz add 2011.10.17

char ASD_DBUS_SECURITY_METHOD_SET_AUTH[PATH_LEN]=	"set_auth";

char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AUTH[PATH_LEN]=	"secondary_set_auth"; /*xm 08/09/02*/
char ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD[PATH_LEN]=	"eap_reauth_period"; /*xm 08/09/02*/
char ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL[PATH_LEN]=	"acct_interim_interval"; /*xm 08/09/02*/
char ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_ACCT[PATH_LEN]=	"secondary_set_acct"; /*xm 08/09/03*/
char ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD[PATH_LEN]=	"security_set_quiet_period"; /*ht 090727*/
char ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP[PATH_LEN]= "security_host_ip";
char ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE[PATH_LEN]=	"security_type";

char ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD[PATH_LEN]=	"wapi_ucast_rekey_method";	/*	xm0701*/
char ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA[PATH_LEN]=	"wapi_rekey_para";	/*	xm0701*/

char ASD_DBUS_SECURITY_METHOD_EAP_ALIVE_PERIOD[PATH_LEN]=	"eap_alive_period"; /*weichao 2011.09.22*/
char ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_AUTHORIZE[PATH_LEN]=	"account_after_authorize"; /*weichao 2011.12.01*/
char ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_DHCP[PATH_LEN]=	"account_after_dhcp"; /*weichao 2011.12.01*/
char ASD_DBUS_AP_DETECT_INTERVAL[PATH_LEN] = "ap_detect_interval";			




char ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE[PATH_LEN]=	"encryption_type";
char ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH[PATH_LEN]=	"extensible_auth";
char ASD_DBUS_SECURITY_METHOD_MAC_AUTH[PATH_LEN] = "mac_auth";
char ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT[PATH_LEN]=	"RADIUS_SERVER_SELECT";
char ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK[PATH_LEN]=		"security_wlan_check";

char ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG[PATH_LEN]=	"security_show_running_config";
char ASD_DBUS_SECURITY_METHOD_SECURITY_KEY[PATH_LEN]=	"security_key";

char ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE[PATH_LEN]=	"configure_port_enable";/*xm 08/09/01*/
char ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT[PATH_LEN]=	    "configure_port";/*xm 08/09/01*/

char ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_SECURITY[PATH_LEN]=	    "set_port_vlan_append_security";/*sz20080825*/ 
char ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_ENABLE[PATH_LEN]=	    "set_port_vlan_append_enable";/*sz20080825 */
char ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY[PATH_LEN]=      "set_vlan_list_append_security";/*sz20080825*/
char ASD_DBUS_SECURITY_METHOD_SET_VLAN_LIST_APPEND_ENABLE[PATH_LEN]=    "set_vlan_list_append_enable";/*sz20080825*/
char ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG[PATH_LEN]=    	"set_asd_daemonlog_debug";/*ht 08.12.01*/
char ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG[PATH_LEN]=	"set_asd_logger_printflag";/*ht 08.12.04*/
char ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_LEVEL[PATH_LEN]="set_asd_daemonlog_level";

char ASD_DBUS_SECURITY_METHOD_SET_ASD_LOG_GROUP_ACTIVATED[PATH_LEN]="set_asd_log_group_activated";//qiuchen add it for hn_mobile 
char ASD_DBUS_SECURITY_METHOD_SET_AC_MANAGEMENT_IP[PATH_LEN]="set_ac_management_ip";//qiuchen add it for hn_mobile 
char ASD_DBUS_SECURITY_METHOD_SHOW_AC_MANAGEMENT_IP[PATH_LEN]="show_ac_management_ip";//qiuchen add it for hn_mobile 
char ASD_DBUS_CONFIG_METHOD_SET_LOG_STATISTICS_INTERVAL[PATH_LEN]="set_log_statistcs_interval";

char ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION[PATH_LEN]=		"pre_authentication";
char ASD_DBUS_MOBILE_OPEN[PATH_LEN]=	"mobile_open";
char ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN[PATH_LEN] = "asd_notice_sta_info_to_proto";

char ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON[PATH_LEN]=		"accounting_on";
char ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR[PATH_LEN]=		"radius_extend_attr";

char ASD_DBUS_SECURITY_METHOD_WAPI_RADIUS_AUTH[PATH_LEN] = "wapi_radius_auth";    //mahz add 2010.11.24
char ASD_DBUS_SECURITY_METHOD_SET_USER_PASSWD[PATH_LEN] = "wapi_radius_auth_set_user_passwd";	//mahz add 2010.12.9
char ASD_DBUS_SECURITY_METHOD_HYBRID_AUTH[PATH_LEN] = "hybrid_auth";			//mahz add 2011.2.18
char ASD_DBUS_SECURITY_METHOD_SHOW_ASD_GLOBAL_VARIABLE[PATH_LEN] = "show_asd_global_variable";	//mahz add 2011.3.17
char ASD_DBUS_SECURITY_METHOD_FAST_AUTH[PATH_LEN] =   "fast_auth";			//mahz add 2011.7.8
char ASD_DBUS_SECURITY_METHOD_DISTRIBUTE_ON[PATH_LEN]=	"distribute_on";	//mahz add 2011.10.25
char ASD_DBUS_SECURITY_METHOD_SET_RDC_PARA[PATH_LEN] =	"set_rdc_para";		//mahz add 2011.10.26
char ASD_DBUS_SECURITY_METHOD_SHOW_ASD_RDC_INFO[PATH_LEN] = "show_asd_rdc_info";		//mahz add 2011.11.18

char ASD_DBUS_SECURITY_METHOD_SECURITY_SET_AC[PATH_LEN]= "set_ac_as_secondary";

/*xm add 09/02/13*/
char ASD_DBUS_SIG_STA_LEAVE[PATH_LEN]=	"signal_sta_leave";
char ASD_DBUS_SIG_STA_COME[PATH_LEN]=   "signal_sta_come";
char ASD_DBUS_SIG_WTP_DENY_STA[PATH_LEN]=   "signal_wtp_deny";
char ASD_DBUS_SIG_DE_WTP_DENY_STA[PATH_LEN]= "signal_wtp_de_deny";
char ASD_DBUS_SIG_STA_LEAVE_ABNORMAL[PATH_LEN]= "signal_sta_leave_abnormal";

//char ASD_DBUS_SIG_DENY_STA_NUM   "signal_deny_sta_num"
//ht add 090216
char ASD_DBUS_SIG_WAPI_TRAP[PATH_LEN]=	"signal_wapi_trap";
char ASD_DBUS_SIG_STA_VERIFY[PATH_LEN]=	"signal_sta_verify";
char ASD_DBUS_SIG_STA_VERIFY_FAILED[PATH_LEN]=   "signal_sta_verify_failed";
char ASD_DBUS_SIG_RADIUS_CONNECT_FAILED[PATH_LEN]=   "signal_radius_connect_failed";
char ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN[PATH_LEN]=   "signal_radius_connect_failed_clean";
char ASD_DBUS_SIG_STA_JIANQUAN_FAILED[PATH_LEN]=				"signal_jianquan_fail";
char ASD_DBUS_SIG_STA_ASSOC_FAILED[PATH_LEN]=				"signal_assoc_fail";



char ASD_DBUS_SIG_KEY_CONFLICT[PATH_LEN]=	"signal_key_conflict";
char ASD_DBUS_SIG_DE_KEY_CONFLICT[PATH_LEN]=	"signal_de_key_conflict";
char ASD_DBUS_SECURITY_METHOD_TRAP_OPEN	[PATH_LEN]="asd_trap_open";
char ASD_DBUS_SET_TRAP_ABLE[PATH_LEN]="asd_set_trap_able";
char ASD_DBUS_SHOW_TRAP_STATE[PATH_LEN]="show_asd_trap_state";

char ASD_DBUS_SET_STA_MAC_VLANID[PATH_LEN]=	"set_sta_mac_vlanid"; /*ht add 091028*/
char ASD_DBUS_CHECK_STA_BYMAC[PATH_LEN]= "check_sta_bymac"; /*ht add 011014*/
char ASD_DBUS_SET_STA_STATIC_INFO[PATH_LEN]= "set_sta_static_info"; /*ht add 011014*/
char ASD_DBUS_DEL_STA_STATIC_INFO[PATH_LEN]= "del_sta_static_info"; /*ht add 011014*/
char ASD_DBUS_SHOW_STA_STATIC_INFO_BYMAC[PATH_LEN]= "show_sta_static_info_bymac"; /*ht add 011014*/
char ASD_DBUS_SHOW_STA_STATIC_INFO[PATH_LEN]= "show_sta_static_info"; /*ht add 011014*/
char ASD_DBUS_STA_METHOD_SHOW_STATIC_STA_RUNNING_CONFIG[PATH_LEN]= "show_static_sta_running_config"; /*ht add 011014*/

char ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO[PATH_LEN]=	"show_security_wapi_info" ;
char ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF[PATH_LEN]=	"show_wlan_security_wapi_conf" ;
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE[PATH_LEN]=	"wapi_sub_attr_sertificateupdate"; /*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE[PATH_LEN]=	"wapi_sub_attr_multicastupdate"; /*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE[PATH_LEN]=	"wapi_sub_attr_unicastupdate";/*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE[PATH_LEN]=	"wapi_sub_attr_bklifetimeupdate";/*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE[PATH_LEN]=	"wapi_sub_attr_bkreauth_threasholdupdate";/*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE[PATH_LEN]=	"wapi_sub_attr_satimeoutupdate";/*nl 09/11/02*/
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_CIPHER[PATH_LEN]=	"wapi_sub_attr_multicast_cipher";/*ht 100112*/

char ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_CONF_OF_ALL_WLAN[PATH_LEN]= "show_security_wapi_conf_of_all_wlan";	  //mahz add 2011.1.27 

char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE[PATH_LEN]=	"wapi_sub_attr_wapipreauth";

char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MUTICASEREKEYSTRICT_UPDATE[PATH_LEN]=	"wapi_sub_attr_muticaserekeystrict";
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICASTCIPHERENABLED_UPDATE[PATH_LEN]=	"wapi_sub_attr_unicastcipherenabled";
char ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_AUTHENTICATIONSUITEENABLE_UPDATE[PATH_LEN]=	"wapi_sub_attr_authenticationsuteenable";
char ASD_DBUS_SECURITY_METHOD_WEP_INDEX_PERIOD[PATH_LEN]=	"security_index_period" ;/*nl 10/03/15*/
char ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE[PATH_LEN]=	"set_asd_get_sta_info_able";	//ht add 091111
char ASD_DBUS_SET_ASD_GET_STA_INFO_TIME[PATH_LEN]=	"set_asd_get_sta_info_time";
char ASD_DBUS_SET_ASD_PROCESS_80211N_ABLE[PATH_LEN]=	"set_asd_process_80211n_able";	//ht add 091111
char ASD_DBUS_SET_STA_STATIC_FDB_ABLE[PATH_LEN]=	"set_sta_static_fdb_able";	//ht add 091111

char ASD_DBUS_AC_GROUP_OBJPATH[PATH_LEN]= "/aw/asd";
char ASD_DBUS_AC_GROUP_INTERFACE[PATH_LEN]= "aw.asd";

char ASD_DBUS_AC_GROUP_METHOD_CREATE_GROUP[PATH_LEN]= "create_ac_group";
char ASD_DBUS_AC_GROUP_METHOD_DELETE_GROUP[PATH_LEN]= "delete_ac_group";
char ASD_DBUS_AC_GROUP_METHOD_ADD_GROUP_MEMBER[PATH_LEN]= "add_ac_group_member";
char ASD_DBUS_AC_GROUP_METHOD_DEL_GROUP_MEMBER[PATH_LEN]= "del_ac_group_member";
char ASD_DBUS_AC_GROUP_METHOD_CONFIG[PATH_LEN]= "config_ac_group";
char ASD_DBUS_AC_GROUP_METHOD_HOST_IP[PATH_LEN]= "set_host_ip";
char ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_LIST[PATH_LEN]= "show_ac_group_list";
char ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP[PATH_LEN]= "show_ac_group";
char ASD_DBUS_AC_GROUP_METHOD_SHOW_AC_GROUP_RUNNING_CONFIG[PATH_LEN]= "show_ac_group_running_config";
char ASD_DBUS_AC_GROUP_METHOD_ROAMING_COUNT[PATH_LEN]= "show_ac_roaming_count";

char ASD_DBUS_STA_METHOD_SET_ASD_STA_ARP_LISTEN[PATH_LEN]=	 "set_asd_sta_arp_listen";
char ASD_DBUS_STA_METHOD_SET_ASD_STA_IP_FROM_DHCPSNOOP[PATH_LEN]=  "set_asd_sta_ip_from_dhcpsnoop";
char ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP[PATH_LEN]=	 "set_asd_sta_static_arp";
char ASD_DBUS_STA_METHOD_SET_ASD_STA_STATIC_ARP_IF_GROUP[PATH_LEN]=	 "set_asd_sta_static_arp_if_group";
char ASD_DBUS_STA_METHOD_SET_ASD_SWITCH[PATH_LEN]=	 "set_asd_switch";
char ASD_DBUS_STA_METHOD_SET_ASD_RADIUS_FORMAT[PATH_LEN]=  "set_asd_8021x_radius_format";    /* this define is for asd */

char ASD_DBUS_STA_METHOD_AC_ADD_MAC_LIST[PATH_LEN]=	"ac_add_MAC_list_sta";	/*nl add 2010-08-28*/
char ASD_DBUS_STA_METHOD_AC_DEL_MAC_LIST[PATH_LEN]=	"ac_del_MAC_list_sta";	/*nl add 2010-08-28*/
char ASD_DBUS_STA_METHOD_AC_USE_MAC_LIST[PATH_LEN]=	"ac_use_MAC_list_sta";	/*nl add 2010-08-28*/
char ASD_DBUS_STA_METHOD_SHOW_AC_MAC_LIST[PATH_LEN]= "show_ac_MAC_list";		/*nl add 2010-08-28*/

char ASD_DBUS_NOTICE_STA_INFO_TO_PORTAL_TIMER[PATH_LEN]=	"asd_notice_sta_info_to_portal_timer";
char ASD_DBUS_STA_METHOD_SET_AC_MAX_STA_NUM[PATH_LEN]=	 "set_ac_max_sta_num";
char ASD_DBUS_STA_METHOD_SHOWSTA_V2[PATH_LEN]=	"show_sta_v2";
char ASD_DBUS_SET_DBUS_COUNT[PATH_LEN]=	"set_asd_dbus_count";
char ASD_DBUS_SHOW_DBUS_COUNT[PATH_LEN]=	"show_asd_dbus_count";
char ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME[PATH_LEN]=  "set_asd_sta_idle_time";
char ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME_SWITCH[PATH_LEN] = "set_asd_sta_idle_time_switch";

char ASD_DBUS_STA_METHOD_SET_ASD_IPSET_SWITCH[PATH_LEN] = "set_asd_ipset_switch";
char ASD_DBUS_STA_METHOD_SET_ASD_BAK_STA_UPDATE_TIME[PATH_LEN]=  "set_asd_bak_sta_update_time";
char ASD_DBUS_CONF_METHOD_CHECKING[PATH_LEN] = "asd_checking";
char ASD_DBUS_CONF_METHOD_QUIT[PATH_LEN] = "asd_quit";

char ASD_DBUS_BSS_METHOD_SHOW_BSS_SUMMARY[PATH_LEN] = "asd_show_bss_summary";
char ASD_DBUS_BSS_METHOD_SHOW_BSS_BSSINDEX[PATH_LEN] = "asd_show_bss_bssindex";
#endif
