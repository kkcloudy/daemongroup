#ifndef _ASD_DBUS_DEF_H
#define _ASD_DBUS_DEF_H

#define ASD_DBUS_BUSNAME	"aw.asd"
#define ASD_DBUS_OBJPATH	"/aw/asd"
#define ASD_DBUS_INTERFACE	"aw.asd"


#define ASD_DBUS_STA_OBJPATH	"/aw/asd/sta"
#define ASD_DBUS_STA_INTERFACE	"aw.asd.sta"

#define ASD_DBUS_STA_METHOD_SHOWSTA	"show_sta"
#define ASD_DBUS_STA_METHOD_EXTEND_SHOWSTA	"extend_show_sta"
#define ASD_DBUS_STA_METHOD_SHOWSTA_BYMAC "show_sta_by_mac"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_BYWTPID	"show_info_bywtpid"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_BYWLANID	"show_info_bywlanid"
#define ASD_DBUS_STA_METHOD_SHOW_INFO_ALLWLAN   "show_info_allwlan"   //fengwenchao add 20101221
#define ASD_DBUS_STA_METHOD_SHOW_RADIO_INFO_BYWTPID	"show_radio_info_bywtpid"	/*ht add 090428*/
#define ASD_DBUS_STA_METHOD_SHOW_WAPI_INFO_BYWTPID	"show_wapi_info_bywtpid"	/*ht add 090622*/
#define ASD_DBUS_STA_METHOD_SHOW_CHANNEL_ACCESS_TIME	"show_channel_access_time"	/*ht add 090513*/

#define ASD_DBUS_STA_METHOD_SHOW_MIB_INFO_BYRDID	"show_mib_info_byradioid"	/*	xm0616*/
#define ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_BYRDID	"show_wapi_mib_info_byradioid"	/*	xm0623*/

#define ASD_DBUS_STA_METHOD_SHOW_WAPI_MIB_INFO_OF_ALL_WTP	"show_wapi_mib_info_of_all_wtp"		//mahz add 2011.1.19
#define ASD_DBUS_STA_METHOD_SHOW_STA_WAPI_MIB_INFO_OF_ALL_WTP	"show_sta_wapi_mib_info_of_all_wtp"		//mahz add 2011.1.24


#define ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYBSSINDEX	"show_traffic_limit_by_bssindex"	/*	xm0723*/
#define ASD_DBUS_STA_METHOD_SHOW_TRAFFIC_LIMIT_BYRADIO	"show_traffic_limit_by_radioid"	/*	xm0723*/


#define ASD_DBUS_STA_METHOD_SET_AC_FLOW	 "set_ac_flow"  /*xm0714*/
#define ASD_DBUS_STA_METHOD_SET_EXTERN_BALANCE	 "set_extern_balance"  /*xm0714*/



#define ASD_DBUS_STA_METHOD_KICKSTA	"kick_sta"
#define ASD_DBUS_STA_METHOD_RADIUS_FORCE_STA_DOWNLINE	"radius_force_sta_downline"		//mahz add 2011.6.3

#define ASD_DBUS_STA_METHOD_WLAN_ADD_MAC_LIST	"wlan_add_MAC_list_sta"/*xm add 08/11/07*/
#define ASD_DBUS_STA_METHOD_WTP_ADD_MAC_LIST	"wtp_add_MAC_list_sta"/*ht add 08.12.15*/
#define ASD_DBUS_STA_METHOD_BSS_ADD_MAC_LIST	"bss_add_MAC_list_sta"/*ht add 08.12.15*/
#define ASD_DBUS_STA_METHOD_WLAN_DEL_MAC_LIST	"wlan_del_MAC_list_sta"/*ht add 08.12.22*/
#define ASD_DBUS_STA_METHOD_WTP_DEL_MAC_LIST	"wtp_del_MAC_list_sta"/*ht add 08.12.22*/
#define ASD_DBUS_STA_METHOD_BSS_DEL_MAC_LIST	"bss_del_MAC_list_sta"/*ht add 08.12.22*/

#define ASD_DBUS_STA_METHOD_WLAN_USE_MAC_LIST	"wlan_use_MAC_list_sta"/*ht add 08.12.15*/
#define ASD_DBUS_STA_METHOD_WTP_USE_MAC_LIST	"wtp_use_MAC_list_sta"/*ht add 08.12.15*/
#define ASD_DBUS_STA_METHOD_BSS_USE_MAC_LIST	"bss_use_MAC_list_sta"/*ht add 08.12.15*/
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WLAN_MAC_LIST	 "show_all_wlan_MAC_list"/*ht add 08.12.19*/
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_MAC_LIST	 "show_all_wtp_MAC_list"/*ht add 08.12.19*/
#define ASD_DBUS_STA_METHOD_SHOW_ALL_BSS_MAC_LIST	 "show_all_bss_MAC_list"/*ht add 08.12.19*/
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_MAC_LIST	 "show_wlan_MAC_list"/*ht add 08.12.22*/
#define ASD_DBUS_STA_METHOD_SHOW_WTP_MAC_LIST	 "show_wtp_MAC_list"/*ht add 08.12.22*/
#define ASD_DBUS_STA_METHOD_SHOW_BSS_MAC_LIST	 "show_bss_MAC_list"/*ht add 08.12.22*/
#define ASD_DBUS_STA_METHOD_WLAN_LIST_SHOW_RUNNING_CONFIG 	"wlan_list_show_running_config" /*ht add 08.12.17*/
#define ASD_DBUS_STA_METHOD_WTP_LIST_SHOW_RUNNING_CONFIG 	"wtp_list_show_running_config" /*ht add 08.12.17*/
#define ASD_DBUS_STA_METHOD_BSS_LIST_SHOW_RUNNING_CONFIG 	"bss_list_show_running_config" /*ht add 08.12.17*/
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WIDS_MAC_LIST	 "show_wlan_wids_MAC_list"/*ht add 09.07.14*/

#define ASD_DBUS_STA_METHOD_AC_ADD_MAC_LIST		"ac_add_MAC_list_sta"/*nl add 2010-08-28*/
#define ASD_DBUS_STA_METHOD_AC_DEL_MAC_LIST		"ac_del_MAC_list_sta"/*nl add 2010-08-28*/
#define ASD_DBUS_STA_METHOD_AC_USE_MAC_LIST		"ac_use_MAC_list_sta"/*nl add 2010-08-28*/
#define ASD_DBUS_STA_METHOD_SHOW_AC_MAC_LIST 	"show_ac_MAC_list"/*nl add 2010-08-28*/
#define ASD_DBUS_STA_METHOD_SET_ASD_STA_IP_FROM_DHCPSNOOP  "set_asd_sta_ip_from_dhcpsnoop"


#define ASD_DBUS_STA_METHOD_STALIST	"show_sta_list"
#define ASD_DBUS_STA_METHOD_STALIST_NEW	"show_sta_list_new"
#define ASD_DBUS_STA_METHOD_GET_STA_INFO	"get_sta_info" /* xm add 08/12/10*/
#define ASD_DBUS_STA_METHOD_STA_BASE_INFO	"show_sta_base_info"
#define ASD_DBUS_STA_METHOD_GET_STA_INFO_NEW  "get_sta_info_new" /*fengwenchao add 20120323*/
#define ASD_DBUS_STA_METHOD_STA_SUMMARY	"show_sta_summary"


#define ASD_DBUS_STA_METHOD_WLAN_STALIST	"show_wlan_sta_list"
#define ASD_DBUS_STA_METHOD_ALL_WLAN_STALIST  "show_wlan_sta_of_all"  //fengwenchao add 20110113

#define ASD_DBUS_STA_METHOD_WTP_STALIST	"show_wtp_sta_list"
#define ASD_DBUS_STA_METHOD_EXTEND_WTP_STALIST	"extend_show_wtp_sta_list"


#define ASD_DBUS_STA_METHOD_ADD_DEL_STA	"add_del_sta"
#define ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT	"set_sta_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL	"cancel_sta_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT	"set_sta_send_traffic_limit"
#define ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL	"cancel_sta_send_traffic_limit"

#define ASD_DBUS_SET_STA_MAC_VLANID			"set_sta_mac_vlanid" /*ht add 091028*/
#define ASD_DBUS_CHECK_STA_BYMAC			"check_sta_bymac" /*ht add 100113*/
#define ASD_DBUS_SET_STA_STATIC_INFO		"set_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_DEL_STA_STATIC_INFO		"del_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_SHOW_STA_STATIC_INFO_BYMAC	"show_sta_static_info_bymac" /*ht add 100113*/
#define ASD_DBUS_SHOW_STA_STATIC_INFO		"show_sta_static_info" /*ht add 100113*/
#define ASD_DBUS_STA_METHOD_SHOW_STATIC_STA_RUNNING_CONFIG	"show_static_sta_running_config"

#define ASD_DBUS_SECURITY_OBJPATH	"/aw/asd/security"
#define ASD_DBUS_SECURITY_INTERFACE	"aw.asd.security"

#define ASD_DBUS_SECURITY_METHOD_CONFIG_SECURITY	"config_security"

#define ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST		"show_security_list"

#define ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY		"show_security"

#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS		"show_radius"

#define ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_ALL	"show_radius_all"		//mahz add 2011.1.12
#define ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO	"show_security_wapi_info" 

#define ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF	"show_wlan_security_wapi_conf"

#define ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_CONF_OF_ALL_WLAN	"show_security_wapi_conf_of_all_wlan"	//mahz add 2011.1.27


#define ASD_DBUS_SECURITY_METHOD_ADD_DEL_SECURITY		"add_del_security"

#define ASD_DBUS_SECURITY_METHOD_APPLY_WLAN		"apply_wlan"

#define ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT	"asd_update_wtp_count"

#define ASD_DBUS_SECURITY_METHOD_SET_ACCT	"set_acct"
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_AUTH	"set_wpai_auth"
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_PATH	"set_wpai_certification_path"
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_P12_CERT_PATH	"set_wpai_p12_certification_path"
#define ASD_DBUS_SECURITY_METHOD_DEL_WAPI_CER	"del_wapi_certification"  //weichao add 20110801
#define ASD_DBUS_SECURITY_METHOD_SET_WAPI_MULTI_CERT "set_wpai_multi_cert"
#define ASD_DBUS_SECURITY_METHOD_WPA_GROUP_REKEY_PERIOD	"wpa_group_rekey_period"
#define ASD_DBUS_SECURITY_METHOD_WPA_KEYUPDATE_TIMEOUT_PERIOD	"wpa_keyupdate_timeout_period"
#define ASD_DBUS_SECURITY_METHOD_WPA_ONCE_GROUP_REKEY_TIME	"wpa_once_group_rekey_time" 	//mahz add 2011.1.3
#define ASD_DBUS_SECURITY_METHOD_ASD_SOCKET_OPERATE		"asd_socket_operate"	//mahz add 2011.10.17
#define ASD_DBUS_SECURITY_METHOD_TRAFFIC_LIMIT_FROM_RADIUS  	"traffic_limit_form_radius" 



#define ASD_DBUS_SECURITY_METHOD_SET_AUTH	"set_auth"

#define ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_AUTH	"secondary_set_auth" /*xm 08/09/02*/
#define ASD_DBUS_SECURITY_METHOD_EAP_REAUTH_PERIOD	"eap_reauth_period" /*xm 08/09/02*/
#define ASD_DBUS_SECURITY_METHOD_ACCT_INTERIM_INTERVAL	"acct_interim_interval" /*xm 08/09/02*/
#define ASD_DBUS_SECURITY_METHOD_SECONDARY_SET_ACCT	"secondary_set_acct" /*xm 08/09/03*/
#define ASD_DBUS_SECURITY_METHOD_QUIET_PERIOD	"security_set_quiet_period" /*ht 090727*/
#define ASD_DBUS_SECURITY_METHOD_SECURITY_HOST_IP "security_host_ip"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_TYPE	"security_type"
#define ASD_DBUS_SECURITY_METHOD_MAC_AUTH 	"mac_auth"
#define ASD_DBUS_SECURITY_METHOD_EAP_ALIVE_PERIOD	"eap_alive_period" /*weichao 2011.09.22*/
#define ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_AUTHORIZE   "account_after_authorize" /*weichao 2011.12.01*/
#define ASD_DBUS_SECURITY_METHOD_ACCOUNT_AFTER_DHCP	"account_after_dhcp" /*weichao 2011.09.22*/
#define ASD_DBUS_AP_DETECT_INTERVAL 			"ap_detect_interval"			
#define ASD_DBUS_SECURITY_METHOD_WAPI_UCAST_REKEY_METHOD	"wapi_ucast_rekey_method"	/*	xm0701*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_REKEY_PARA	"wapi_rekey_para"	/*	xm0701*/


#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_CERTIFICATE_UPDATE	"wapi_sub_attr_sertificateupdate" /*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_UPDATE	"wapi_sub_attr_multicastupdate" /*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICAST_COUNT_UPDATE	"wapi_sub_attr_unicastupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKLIFETIME_UPDATE	"wapi_sub_attr_bklifetimeupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_BKREAUTH_THREASHOLD_UPDATE	"wapi_sub_attr_bkreauth_threasholdupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_SA_TIMEOUT_UPDATE	"wapi_sub_attr_satimeoutupdate"/*nl 09/11/02*/
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MULTICAST_CIPHER 	"wapi_sub_attr_multicast_cipher"/*ht 100112*/

#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_WAPIPREAUTH_UPDATE	"wapi_sub_attr_wapipreauth"

#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_MUTICASEREKEYSTRICT_UPDATE	"wapi_sub_attr_muticaserekeystrict"
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_UNICASTCIPHERENABLED_UPDATE	"wapi_sub_attr_unicastcipherenabled"
#define ASD_DBUS_SECURITY_METHOD_WAPI_SUB_ATTR_AUTHENTICATIONSUITEENABLE_UPDATE	"wapi_sub_attr_authenticationsuteenable"
#define ASD_DBUS_SECURITY_METHOD_WEP_INDEX_PERIOD	"security_index_period" /*nl 10/03/15*/






#define ASD_DBUS_SECURITY_METHOD_ENCRYPTION_TYPE	"encryption_type"
#define ASD_DBUS_SECURITY_METHOD_EXTENSIBLE_AUTH	"extensible_auth"
#define ASD_DBUS_SECURITY_METHOD_MAC_AUTH 	"mac_auth"
#define ASD_DBUS_SECURITY_METHOD_RADIUS_SERVER_SELECT	"RADIUS_SERVER_SELECT"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK		"security_wlan_check"

#define ASD_DBUS_SECURITY_METHOD_SECURITY_SHOW_RUNNING_CONFIG	"security_show_running_config"
#define ASD_DBUS_SECURITY_METHOD_SECURITY_KEY	"security_key"

#define ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT_ENABLE	"configure_port_enable"/*xm 08/09/01*/
#define ASD_DBUS_SECURITY_METHOD_CONFIGURE_PORT	    "configure_port"/*xm 08/09/01*/

#define ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_SECURITY	    "set_port_vlan_append_security"/*sz20080825*/ 
#define ASD_DBUS_SECURITY_METHOD_SET_PORT_VLAN_APPEND_ENABLE	    "set_port_vlan_append_enable"/*sz20080825 */
#define ASD_DBUS_SECURITY_METHOD_SET_VLAN_APPEND_SECURITY       "set_vlan_list_append_security"/*sz20080825*/
#define ASD_DBUS_SECURITY_METHOD_SET_VLAN_LIST_APPEND_ENABLE    "set_vlan_list_append_enable"/*sz20080825*/
#define ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG    	"set_asd_daemonlog_debug"/*ht 08.12.01*/
#define ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG	"set_asd_logger_printflag"/*ht 08.12.04*/
#define ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_LEVEL    	"set_asd_daemonlog_level"

#define ASD_DBUS_SECURITY_METHOD_PRE_AUTHENTICATION		"pre_authentication"
#define ASD_DBUS_MOBILE_OPEN	"mobile_open"
#define ASD_DBUS_NOTICE_STA_INFO_TO_PROTO_OPEN	"asd_notice_sta_info_to_proto"

#define ASD_DBUS_SECURITY_METHOD_ACCOUNTING_ON		"accounting_on"
#define ASD_DBUS_SECURITY_METHOD_RADIUS_EXTEND_ATTR		"radius_extend_attr"
#define ASD_DBUS_SECURITY_METHOD_FAST_AUTH 			"fast_auth"			//mahz add 2011.7.8
#define ASD_DBUS_SECURITY_METHOD_DISTRIBUTE_ON		"distribute_on"		//mahz add 2011.10.25
#define ASD_DBUS_SECURITY_METHOD_SET_RDC_PARA		"set_rdc_para"		//mahz add 2011.10.26
#define ASD_DBUS_SECURITY_METHOD_SHOW_ASD_RDC_INFO	"show_asd_rdc_info"		//mahz add 2011.11.18

#define ASD_DBUS_SECURITY_METHOD_WAPI_RADIUS_AUTH       "wapi_radius_auth"			//mahz add 2010.11.24
#define ASD_DBUS_SECURITY_METHOD_SET_USER_PASSWD 		"wapi_radius_auth_set_user_passwd"	//mahz add 2010.12.9
#define ASD_DBUS_SECURITY_METHOD_HYBRID_AUTH 			"hybrid_auth"			//mahz add 2011.2.18
#define ASD_DBUS_SECURITY_METHOD_SHOW_ASD_GLOBAL_VARIABLE 	"show_asd_global_variable"	//mahz add 2011.3.17

#define ASD_DBUS_SECURITY_METHOD_SECURITY_SET_AC	"set_ac_as_secondary"

#define ASD_DBUS_SET_ASD_GET_STA_INFO_ABLE	"set_asd_get_sta_info_able"	//ht add 091111
#define ASD_DBUS_SET_ASD_GET_STA_INFO_TIME	"set_asd_get_sta_info_time"
#define ASD_DBUS_SET_ASD_PROCESS_80211N_ABLE	"set_asd_process_80211n_able"	//ht add 091111
#define ASD_DBUS_SET_STA_STATIC_FDB_ABLE	"set_sta_static_fdb_able"	//ht add 091111

/*xm add 09/02/13*/
#define ASD_DBUS_SIG_STA_LEAVE	"signal_sta_leave"
#define ASD_DBUS_SIG_STA_COME   "signal_sta_come"
#define ASD_DBUS_SIG_WTP_DENY_STA   "signal_wtp_deny"
#define ASD_DBUS_SIG_DE_WTP_DENY_STA "signal_wtp_de_deny"
#define ASD_DBUS_SIG_STA_LEAVE_ABNORMAL	"signal_sta_leave_abnormal"

//#define ASD_DBUS_SIG_DENY_STA_NUM   "signal_deny_sta_num"
//ht add 090216
#define ASD_DBUS_SIG_WAPI_TRAP	"signal_wapi_trap"
#define ASD_DBUS_SIG_STA_VERIFY	"signal_sta_verify"
#define ASD_DBUS_SIG_STA_VERIFY_FAILED   "signal_sta_verify_failed"
#define ASD_DBUS_SIG_RADIUS_CONNECT_FAILED   "signal_radius_connect_failed"
#define ASD_DBUS_SIG_RADIUS_CONNECT_FAILED_CLEAN   "signal_radius_connect_failed_clean"
#define ASD_DBUS_SIG_STA_JIANQUAN_FAILED				"signal_jianquan_fail"
#define ASD_DBUS_SIG_STA_ASSOC_FAILED				"signal_assoc_fail"


#define ASD_DBUS_SIG_KEY_CONFLICT				"signal_key_conflict"
#define ASD_DBUS_SIG_DE_KEY_CONFLICT				"signal_de_key_conflict"

#define ASD_DBUS_SECURITY_METHOD_TRAP_OPEN 		"asd_trap_open"
#define ASD_DBUS_SET_TRAP_ABLE 			"asd_set_trap_able"
#define ASD_DBUS_SHOW_TRAP_STATE 		"show_asd_trap_state"
#define ASD_DBUS_SHOW_DBUS_COUNT 		"show_asd_dbus_count"
#define ASD_DBUS_SET_DBUS_COUNT 		"set_asd_dbus_count"
/*#define ASD_DBUS_SIG_AUTH_FAIL     "signal_auth_fail"*/

/*showting  information for mib nl*/
/*----------------------------------------------------------------------------------------------*/
#define ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP	"show_asd_collect_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_OF_ALL_WTP	"show_asd_stats_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_stats_info_of_wlan_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_ssid_stats_info_of_wlan_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_ALLWLAN_STA_NUM "show_asd_all_wlan_sta_num"  //fengwenchao add 20101223
#define ASD_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP	"show_asd_terminal_info_of_all_wtp"	/*lzh add 20100512*/
#define ASD_DBUS_STA_METHOD_SHOW_STA_INFO_OF_ALL_WTP		"show_asd_sta_info_of_all_wtp" /*liuzhenhua append 2010-05-28*/
#define ASD_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_CONFIG_INFORMATION "show_asd_ssid_config_information_of_all_wtp" /*liuzhenhua append 2010-05-21*/
#define ASD_DBUS_STA_METHOD_SHOW_DISTINGUISH_INFO_OF_ALL_WTP		"show_asd_distinguish_info_of_all_wtp"		//mahz add 2011.1.17
#define ASD_DBUS_STA_METHOD_SHOW_STA_STATIS_INFO_OF_ALL_WTP	"show_asd_sta_statis_info_of_all_wtp"	//mahz add 2011.11.9 for GuangZhou Mobile
#define ASD_DBUS_STA_METHOD_SHOW_AC_STA_INFO_OF_ALL	"show_asd_ac_sta_info_of_all"	//mahz add 2011.11.10

#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_BASIC_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_wapi_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_USER_LINK_INFO_OF_ALL_WTP		"show_user_link_info_of_all_wtp"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_UNICAST_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_unicast_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_STATS_INFO_OF_ALL_WLAN	"show_asd_wapi_stats_performance_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_EXTEND_CONFIG_INFO_BY_WLAN_OF_ALL_WTP	"show_asd_wlan_wapi_extend_config_info_of_all_wlan"
#define ASD_DBUS_STA_METHOD_SHOW_WLAN_WAPI_PERFORMANCE_BSS_INFO_OF_ALL_WLAN	"show_asd_wapi_bss_performance_info_of_all_wlan"

#define ASD_DBUS_STA_METHOD_SHOW_RADIO_WIRELESS_INFO_BYWTPID	"show_radio_wireless_info_bywtpid"	/*nl add 20100502*/
#define ASD_DBUS_STA_METHOD_SHOW_RADIO_NEW_WIRELESS_INFO_BYWTPID	"show_radio_new_wireless_info_bywtpid"/*nl add 20100607*/
#define ASD_DBUS_STA_METHOD_SHOW_SECURITY_INFO_BYWTPID	"show_security_info_bywtpid"/*nl add 20100607*/
#define ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_INFORMATION	"show_all_wtp_information"/*nl add 20100622*/

#define ASD_DBUS_STA_METHOD_SHOW_BSS_STA_NUM_BY_WLANID_AND_RADIOID "show_asd_bss_sta_num_by_wlanid_and_radioid"
#define ASD_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE "show_statistics_information_of_all_wtp_whole"  //fengwenchao add 20110331
#define ASD_DBUS_STA_METHOD_SET_ASD_STA_IDLE_TIME "set_asd_sta_idle_time"
#define ASD_DBUS_STA_METHOD_SET_ASD_STA_CHECK_TIME "set_asd_sta_idle_time"  //xk add for check sta
/*----------------------------------------------------------------------------------------------*/





enum asd_dbus_result {
	ASD_DBUS_SUCCESS,
	ASD_DBUS_ERROR,
	ASD_STA_NOT_EXIST,
	ASD_WLAN_NOT_EXIST,
	ASD_WAPI_WLAN_NOT_EXIST,
	ASD_WTP_NOT_EXIST,
	ASD_SECURITY_NOT_EXIST,
	ASD_RADIO_NOT_EXIST,				/*add for id check*/
	ASD_SECURITY_BE_USED,	
	ASD_SECURITY_ACCT_NOT_EXIST,
	ASD_SECURITY_ACCT_BE_USED,	
	ASD_SECURITY_AUTH_NOT_EXIST,
	ASD_SECURITY_AUTH_BE_USED,
	ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE,
	ASD_SECURITY_TYPE_WITHOUT_8021X,
	ASD_SECURITY_TYPE_WITHOUT_WAPI_AUTH,
	ASD_SECURITY_PROFILE_NOT_INTEGRITY,
	ASD_SECURITY_PROFILE_NOT_BIND_WLAN,
	ASD_WLAN_HAS_BEEN_BINDED,
	ASD_SECURITY_KEY_NOT_PERMIT,
	ASD_SECURITY_KEY_LEN_NOT_PERMIT,
	ASD_SECURITY_KEY_LEN_NOT_PERMIT_HEX,
	ASD_SECURITY_KEY_HEX_FORMAT,
	ASD_SECURITY_KEY_HAS_BEEN_SET,
	ASD_SECURITY_WLAN_SHOULD_BE_DISABLE,
	ASD_BSS_NOT_EXIST,
	ASD_BSS_VALUE_INVALIDE,
	ASD_WLAN_VALUE_INVALIDE,
	ASD_UNKNOWN_ID,
	ASD_EXTENSIBLE_AUTH_NOT_SUPPORT,
	ASD_PRE_AUTH_NOT_SUPPORT,
	ASD_UPDATE_ERROR,	
	ASD_WTP_ID_LARGE_THAN_MAX,
	ASD_RADIO_ID_LARGE_THAN_MAX,
	ASD_WLAN_ID_LARGE_THAN_MAX,			/*add for id check*/
	ASD_BSS_ID_LARGE_THAN_MAX,
	ASD_SECURITY_LARGE_THAN_MAX,		/*add for id check*/
	ASD_WIDS_OPEN,
	ASD_SECURITY_TYPE_HAS_CHANGED,		/*nl  add 09/10/27*/	
	ASD_IFNAME_NOT_EXIST,
	ASD_ARP_GROUP_EXIST,
	ASD_NEED_REBOOT,
	ASD_SOCK_NOT_EXIST,		//mahz add 2011.10.17
	ASD_MAC_AUTH_NOT_SUPPORT,
	ASD_ACCOUNT_AFTER_AUTHORIZE
};

/*add for id check 20100804 by nl*/
enum asd_check_type {
	ASD_WTP_CHECK,
	ASD_WLAN_CHECK,
	ASD_RADIO_CHECK,
	ASD_SECURITY_CHECK,
	ASD_BSS_CHECK,

};


enum asd_security_type {
	OPEN,
	SHARED,
	IEEE8021X,
	WPA_P,
	WPA2_P,
	WPA_E,
	WPA2_E,
	MD5,
	WAPI_PSK,
	WAPI_AUTH  
};

enum asd_encryption_type {
	NONE,
	WEP,
	AES,
	TKIP,
	SMS4
};
enum { AUTH_REQUEST, 
	   AUTH_RESPONSE, 
	   AUTH_SUCCESS,
	   AUTH_FAIL,
	   AUTH_TIMEOUT,
	   AUTH_IDLE,
	   AUTH_INITIALIZE,
	   AUTH_IGNORE
} auth_state;

enum { PAE_INITIALIZE,
	   PAE_DISCONNECTED,
	   PAE_CONNECTING,
	   PAE_AUTHENTICATING,
	   PAE_AUTHENTICATED,
	   PAE_ABORTING,
	   PAE_HELD,
	   PAE_FORCE_AUTH,
	   PAE_FORCE_UNAUTH,
	   PAE_RESTART
} pae_state;

#endif
