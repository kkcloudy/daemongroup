#ifndef _AC_MANAGE_DEF_H_
#define _AC_MANAGE_DEF_H_

enum {
	AC_MANAGE_SUCCESS,
	AC_MANAGE_DBUS_ERROR,
	AC_MANAGE_CONFIG_FAIL,
	AC_MANAGE_MALLOC_ERROR,
	AC_MANAGE_SERVICE_ENABLE,
	AC_MANAGE_CONFIG_EXIST,
	AC_MANAGE_CONFIG_NONEXIST,
	AC_MANAGE_INPUT_TYPE_ERROR,
	AC_MANAGE_CONFIG_REACH_MAX_NUM,
	AC_MANAGE_FILE_OPEN_FAIL,
	AC_MANAGE_CONFIG_RELEVANCE,
	AC_MANAGE_INIT_DBUS_ERROR,
	AC_MANAGE_SERVICE_START_FAIL,
	AC_MANAGE_SERVICE_STOP_FAIL,
	AC_MANAGE_CONFIG_FLUSH_FAIL,
	AC_MANAGE_CONFIG_SAVE_FAIL,
	AC_MANAGE_CONFIG_EXEC_FAIL,
};

#define LOG_MASK_EMERG			(LOG_MASK(LOG_EMERG))
#define LOG_MASK_ALERT			(LOG_MASK(LOG_ALERT))
#define LOG_MASK_CRIT			(LOG_MASK(LOG_CRIT))
#define LOG_MASK_ERR			(LOG_MASK(LOG_ERR))
#define LOG_MASK_WARNING		(LOG_MASK(LOG_WARNING))
#define LOG_MASK_NOTICE			(LOG_MASK(LOG_NOTICE))
#define LOG_MASK_INFO			(LOG_MASK(LOG_INFO))
#define LOG_MASK_DEBUG			(LOG_MASK(LOG_DEBUG))

#define LOG_ON_ALERT			(LOG_MASK_EMERG|LOG_MASK_ALERT)
#define LOG_ON_CRIT				(LOG_ON_ALERT|LOG_MASK_CRIT)
#define LOG_ON_ERR				(LOG_ON_CRIT|LOG_MASK_ERR)
#define LOG_ON_WARNING			(LOG_ON_ERR|LOG_MASK_WARNING)
#define LOG_ON_NOTICE			(LOG_ON_WARNING|LOG_MASK_NOTICE)
#define LOG_ON_INFO				(LOG_ON_NOTICE|LOG_MASK_INFO)
#define LOG_ON_DEBUG			(LOG_ON_INFO|LOG_MASK_DEBUG)


#define SHOW_RUNNING_SNMP_MANUAL		(0)
#define SHOW_RUNNING_SNMP_CONFIG		(SHOW_RUNNING_SNMP_MANUAL + 0x1) 
#define SHOW_RUNNING_TRAP_RECEIVER		(SHOW_RUNNING_SNMP_MANUAL + 0x2) 
#define SHOW_RUNNING_TRAP_CONFIG		(SHOW_RUNNING_SNMP_MANUAL + 0x3) 

#define SHOW_RUNNING_NTP_CONFIG			(0)


#define AC_MANAGE_LINE_SIZE					(512)


#define AC_MANAGE_EXTEND_COMMAND_DCLI		(0)
#define AC_MANAGE_EXTEND_COMMAND_SYSTEM		(1)


#define MANAGE_FREE(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define MANUAL_SET(obj_data, new_data)		{if(new_data) (obj_data) = (new_data);}
#define MANAGE_STRDUP(new_string, obj_string)	{if((obj_string) && ((obj_string)[0])) (new_string) = strdup((obj_string));}

enum {
	AC_MANAGE_TASK_METHOD_TEST,
	AC_MANAGE_TASK_METHOD_INTERFACE_INFO,
	AC_MANAGE_TASK_METHOD_REGISTER_SLOT,
};


#define PRODUCT_LOCAL_SLOTID		"/dbm/local_board/slot_id"
#define PRODUCT_LOCAL_SERIAL		"/dbm/product/product_serial"
#define BORAD_LOCAL_TYPE			"/proc/board/board_type"
#define PRODUCT_LOCAL_NAME			"/devinfo/product_name"
#define PRODUCT_ACTIVE_MASTER		"/dbm/product/active_master_slot_id"
#define PRODUCT_SLOT_COUNT			"/dbm/product/slotcount"
#define PRODUCT_LOCAL_IS_ACTIVE_MASTER		"/dbm/local_board/is_active_master" 


#define AC_MANAGE_DBUS_DBUSNAME				"aw.acmanage"
#define AC_MANAGE_DBUS_OBJPATH				"/aw/acmanage"
#define AC_MANAGE_DBUS_INTERFACE				"aw.acmanage"

#define AC_MANAGE_SNMP_DBUS_OBJPATH			"/aw/acmanage/snmp"
#define AC_MANAGE_SNMP_DBUS_INTERFACE		"aw.acmanage.snmp"

#define AC_MANAGE_TCRULE_DBUS_OBJPATH		"/aw/acmanage/tcrule"
#define AC_MANAGE_TCRULE_DBUS_INTERFACE		"aw.acmanage.tcrule"

#define AC_MANAGE_FIREWALL_DBUS_OBJPATH		"/aw/acmanage/firewall"
#define AC_MANAGE_FIREWALL_DBUS_INTERFACE	"aw.acmanage.firewall"

#define AC_MANAGE_NTP_DBUS_OBJPATH		"/aw/acmanage/ntp"
#define AC_MANAGE_NTP_DBUS_INTERFACE	"aw.acmanage.ntp"

#define AC_MANAGE_ACINFO_DBUS_OBJPATH	"/aw/acmanage/acinfo"
#define AC_MANAGE_ACINFO_DBUS_INTERFACE	"aw.acmanage.acinfo"

#define AC_MANAGE_DBUS_CONFIG_LOG_DEBUG                 "manage_config_log_debug"
#define AC_MANAGE_DBUS_CONFIG_TOKEN_DEBUG                 "manage_config_token_debug"
#define AC_MANAGE_DBUS_CONFIG_SNMP_LOG_DEBUG            "manage_config_snmp_log_debug"
#define AC_MANAGE_DBUS_CONFIG_TRAP_LOG_DEBUG            "manage_config_trap_log_debug"
#define AC_MANAGE_DBUS_SHOW_SNMP_LOG_DEBUG              "manage_show_snmp_log_debug"
#define AC_MANAGE_DBUS_SHOW_TRAP_LOG_DEBUG              "manage_show_trap_log_debug"

#define AC_MANAGE_DBUS_PROXY_PFM_CONFIG                 "manage_proxy_pfm_config"

#define AC_MANAGE_DBUS_CONFIG_SNMP_MANUAL_INSTANCE      "manage_config_snmp_manual_instance"
#define AC_MANAGE_DBUS_CONFIG_SNMP_SERVICE              "manage_config_snmp_service"
#define AC_MANAGE_DBUS_CONFIG_SNMP_COLLECTION_MODE      "manage_config_snmp_collection_mode"
#define AC_MANAGE_DBUS_CONFIG_SNMP_PFM_REQUESTPKTS      "manage_config_snmp_pfm_requestpkts"
#define AC_MANAGE_DBUS_CONFIG_SNMP_VERSION_MODE         "manage_config_snmp_version_mode"
#define AC_MANAGE_DBUS_CONFIG_SNMP_UPDATE_SYSINFO       "manage_config_snmp_update_sysinfo"
#define AC_MANAGE_DBUS_CONFIG_SNMP_CACHETIME            "manage_config_snmp_cachetime"
#define AC_MANAGE_DBUS_CONFIG_SNMP_ADD_COMMUNITY        "manage_config_snmp_add_community"
#define AC_MANAGE_DBUS_CONFIG_SNMP_SET_COMMUNITY        "manage_config_snmp_set_community"
#define AC_MANAGE_DBUS_CONFIG_SNMP_DEL_COMMUNITY        "manage_config_snmp_del_community"
#define AC_MANAGE_DBUS_CONFIG_SNMP_VIEW                 "manage_config_snmp_view"
#define AC_MANAGE_DBUS_CHECK_SNMP_VIEW                  "manage_check_snmp_view"
#define AC_MANAGE_DBUS_CONFIG_SNMP_VIEW_OID             "manage_config_snmp_view_oid"
#define AC_MANAGE_DBUS_CONFIG_SNMP_ADD_GROUP            "manage_config_snmp_add_group"
#define AC_MANAGE_DBUS_CONFIG_SNMP_DEL_GROUP            "manage_config_snmp_del_group"
#define AC_MANAGE_DBUS_CONFIG_SNMP_ADD_V3USER           "manage_config_snmp_add_v3user"
#define AC_MANAGE_DBUS_CONFIG_SNMP_DEL_V3USER           "manage_config_snmp_del_v3user"
#define AC_MANAGE_DBUS_CONFIG_SNMP_SYSOID_BOARDTYPE     "manage_config_snmp_sysoid_boardtype"
#define AC_MANAGE_DBUS_CONFIG_MEM_STATUE_DOG			"manage_config_mem_status_dog"

#define AC_MANAGE_DBUS_SHOW_SNMP_MANUAL_INSTANCE        "manage_show_snmp_manual_instance"
#define AC_MANAGE_DBUS_SHOW_SNMP_STATE                  "manage_show_snmp_state"
#define AC_MANAGE_DBUS_SHOW_SNMP_BASE_INFO              "manage_show_snmp_base_info"
#define AC_MANAGE_DBUS_SHOW_SNMP_PFM_INTERFACE          "manage_show_snmp_pfm_interface"
#define AC_MANAGE_DBUS_SHOW_SNMP_COMMUNITY              "manage_show_snmp_community"
#define AC_MANAGE_DBUS_SHOW_SNMP_VIEW                   "manage_show_snmp_view"
#define AC_MANAGE_DBUS_SHOW_SNMP_GROUP                  "manage_show_snmp_group"
#define AC_MANAGE_DBUS_SHOW_SNMP_V3USER                 "manage_show_snmp_v3user"
#define AC_MANAGE_DBUS_SHOW_SNMP_RUNNING_CONFIG         "manage_show_snmp_running_config"


#define AC_MANAGE_DBUS_CONFIG_TRAP_SERVICE              "manage_config_trap_service"
#define AC_MANAGE_DBUS_CONFIG_TRAP_CONFIG_RECEIVER      "manage_config_trap_config_receiver"
#define AC_MANAGE_DBUS_CONFIG_TRAP_DEL_RECEIVER         "manage_config_trap_del_receiver"
#define AC_MANAGE_DBUS_CONFIG_TRAP_SWITCH               "manage_config_trap_switch"
#define AC_MANAGE_DBUS_CONFIG_TRAP_GROUP_SWITCH         "manage_config_trap_group_switch"
#define AC_MANAGE_DBUS_CONFIG_TRAP_INSTANCE_HEARTBEAT   "manage_config_trap_instance_heartbeat"
#define AC_MANAGE_DBUS_CLEAR_TRAP_INSTANCE_HEARTBEAT    "manage_clear_trap_instance_heartbeat"
#define AC_MANAGE_DBUS_CONFIG_TRAP_PARAMETER            "manage_config_trap_parameter"

#define AC_MANAGE_DBUS_SHOW_TRAP_STATE                  "manage_show_trap_state"
#define AC_MANAGE_DBUS_SHOW_TRAP_RECEIVER               "manage_show_trap_receiver"
#define AC_MANAGE_DBUS_SHOW_TRAP_SWITCH                 "manage_show_trap_switch"
#define AC_MANAGE_DBUS_SHOW_TRAP_INSTANCE_HEARTBEAT     "manage_show_trap_instance_heartbeat"
#define AC_MANAGE_DBUS_SHOW_TRAP_PARAMETER              "manage_show_trap_parameter"


#define AC_MANAGE_DBUS_MANUAL_SET_MIB_ACIF_STATS        "manage_manual_set_mib_acif_stats"
#define AC_MANAGE_DBUS_SHOW_MIB_ACIF_STATS              "manage_manual_show_acif_stats"
#define AC_MANAGE_DBUS_GET_MIB_LOCALSLOT_ACIF_STATS     "manage_manual_get_localslot_acif_stats"
#define AC_MANAGE_DBUS_SHOW_MIB_ACCUMULATE_ACIF_STATS   "manage_manual_show_mib_accumulate_acif_stats"

#define AC_MANAGE_DBUS_SHOW_RADIUS_CONFIG               "manage_show_radius_config"
#define AC_MANAGE_DBUS_SHOW_PORTAL_CONFIG               "manage_show_portal_config"

#define AC_MANAGE_DBUS_WEB_INIT         				"manage_web_init"
#define AC_MANAGE_DBUS_WEB_IP_PORT_CHECK				"manage_web_ip_port_check"
#define AC_MANAGE_DBUS_WEB_EDIT             		 	"manage_web_edit"
#define AC_MANAGE_DBUS_WEB_SHOW        				 	"manage_web_show"
#define AC_MANAGE_DBUS_WEB_CONF							"manage_web_conf"  
#define AC_MANAGE_DBUS_WEB_DOWNLOAD						"manage_web_download"
#define AC_MANAGE_DBUS_WEB_SHOW_PAGES					"manage_web_show_pages"
#define AC_MANAGE_DBUS_WEB_DEL_PAGES					"manage_web_del_pages"

#define AC_MANAGE_DBUS_EXTEND_COMMAND_EXEC              "manage_dbus_extend_command_exec"

#define AC_MANAGE_DBUS_CONFIG_FLOW_CONTROL_SERVICE		"manage_dbus_config_flow_control_service"
#define AC_MANAGE_DBUS_ADD_TCRULE						"manage_dbus_add_tcrule"
#define AC_MANAGE_DBUS_OFFSET_TCRULE					"manage_dbus_offset_tcrule"
#define AC_MANAGE_DBUS_DELETE_TCRULE					"manage_dbus_delete_tcrule"

#define AC_MANAGE_DBUS_SHOW_FLOW_CONTROL_SERVICE		"manage_dbus_show_flow_control_service"
#define AC_MANAGE_DBUS_SHOW_TCRULE						"manage_dbus_show_tcrule"
#define AC_MANAGE_DBUS_SHOW_TCRULE_OFFSET				"manage_dbus_show_tcrule_offset"
#define AC_MANAGE_DBUS_SHOW_TCRULE_RUNNING_CONFIG		"manage_dbus_show_tcrule_running_config"

#define AC_MANAGE_DBUS_CONFIG_FIREWALL_SERVICE			"manage_dbus_config_firewall_service"
#define AC_MANAGE_DBUS_CONFIG_FIREWALL_RULE				"manage_dbus_config_firewall_rule"
#define AC_MANAGE_DBUS_CHANGE_FIREWALL_INDEX			"manage_dbus_change_firewall_index"
#define AC_MANAGE_DBUS_DELETE_FIREWALL_RULE				"manage_dbus_delete_firewall_rule"
#define AC_MANAGE_DBUS_CONFIG_NAT_UDP_TIMEOUT			"manage_dbus_config_nat_udp_timeout"
#define AC_MANAGE_DBUS_SHOW_FIREWALL_RULE				"manage_dbus_show_firewall_rule"

#define AC_MANAGE_DBUS_SHOW_NTP_RULE				"manage_dbus_show_ntp_rule"
#define AC_MANAGE_DBUS_ADD_NTPSERVER				"manage_dbus_add_ntpserver"
#define AC_MANAGE_DBUS_ADD_NTPCLIENT				"manage_dbus_add_ntpclient"
#define AC_MANAGE_DBUS_SHOW_NTPCLIENT				"manage_dbus_show_ntpclient"
#define AC_MANAGE_DBUS_SHOW_NTPUPSERVER				"manage_dbus_show_ntpupserver"
#define AC_MANAGE_DBUS_INSIDE_NTP					"manage_dbus_inside_ntp"
#define AC_MANAGE_DBUS_SET_NTP_STATUS				"manage_dbus_set_ntp_status"
#define AC_MANAGE_DBUS_CLENA_NTP					"manage_dbus_clean_ntp"
#define AC_MANAGE_DBUS_CONFIG_NTP_PFM_REQUESTPKTS	"manage_config_ntp_pfm_requestpkts"
#define AC_MANAGE_DBUS_SET_TIMEZONE					"manage_dbus_set_timezone"
#define AC_MANAGE_DBUS_SHOW_TIME					"manage_dbus_show_time"
#define AC_MANAGE_DBUS_SET_TIME					    "manage_dbus_set_time"
#define AC_MANAGE_DBUS_SHOW_NTP_RUNNING_CONFIG      "manage_show_ntp_running_config"
#define AC_MANAGE_DBUS_ADD_SYSLOGRULE				"manage_dbus_add_syslogrule"
#define AC_MANAGE_DBUS_CONFIG_SYSLOGRULE			"manage_dbus_config_syslogrule"
#define AC_MANAGE_DBUS_SET_SYSLOG_STATUS			"manage_dbus_set_syslog_status"
#define AC_MANAGE_DBUS_SHOW_SYSLOG_RULE				"manage_dbus_show_syslog_rule"
#define AC_MANAGE_DBUS_SAVE_SYSLOG					"manage_dbus_save_syslog"
#define AC_MANAGE_DBUS_CONFIG_SYSLOGUPLOAD_PFM_REQUESTPKTS	    "manage_config_syslogupload_pfm_requestpkts"

#define AC_MANAGE_DBUS_CONFIG_STRICT_ACCESS_LEVEL	"manage_dbus_config_strict_access_level"
#define AC_MANAGE_DBUS_SHOW_STRICT_ACCESS			"manage_dbus_show_strict_access"

#define AC_MANAGE_DBUS_DOWNLOAD_INTERNAL_PORTAL 	"manage_dbus_download_internal_portal"
#define AC_MANAGE_DBUS_SHOW_INTERNAL_PORTAL			"manage_dbus_show_internal_portal"
#define AC_MANAGE_DBUS_DELETE_INTERNAL_PORTAL		"manage_dbus_delete_internal_portal"

#define AC_MANAGE_DBUS_SET_ACINFO_VALUE				"manage_dbus_set_acinfo_value"
#define AC_MANAGE_DBUS_SET_BKACINFO_VALUE			"manage_dbus_set_bkacinfo_value"
#define AC_MANAGE_DBUS_DEL_AC_VERSION_FILE_VALUE	"manage_dbus_del_ac_version_file_value"


#endif
