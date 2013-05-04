#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include "ac_manage_def.h"
#include "ac_manage_dbus.h"
#include "ac_manage_dbus_handler.h"

DBusConnection *ac_manage_dbus_connection = NULL;

static DBusHandlerResult 
ac_manage_dbus_message_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage	*reply = NULL;
	char sender[20];

	syslog(LOG_DEBUG, "message type = %s, message path = %s, message member = %s\n",
					dbus_message_type_to_string(dbus_message_get_type(message)),
					dbus_message_get_path(message), dbus_message_get_member(message));

	if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_DBUS_OBJPATH))	 {
		if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_LOG_DEBUG)) {
			reply = ac_manage_dbus_config_log_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TOKEN_DEBUG)) {
			reply = ac_manage_dbus_config_token_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_PROXY_PFM_CONFIG)) {
			reply = ac_manage_dbus_proxy_pfm_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_MANUAL_SET_MIB_ACIF_STATS)) {
			reply = ac_manage_dbus_manual_mib_acif_stats(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_MIB_ACIF_STATS)) {
			reply = ac_manage_dbus_show_mib_acif_stats(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_GET_MIB_LOCALSLOT_ACIF_STATS)) {
			reply = ac_manage_dbus_get_mib_localslot_acif_stats(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_MIB_ACCUMULATE_ACIF_STATS)) {
			reply = ac_manage_dbus_show_mib_accumulate_acif_stats(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_RADIUS_CONFIG)) {
			reply = ac_manage_dbus_show_radius_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_EDIT)) {
			reply = ac_manage_dbus_web_edit(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_SHOW)) {
			reply = ac_manage_dbus_web_show(connection, message, user_data);
		} 
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_CONF)) {
			reply = ac_manage_dbus_web_conf(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_DOWNLOAD)) {
			reply = ac_manage_dbus_web_download(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_SHOW_PAGES)) {
			reply = ac_manage_dbus_web_show_pages(connection, message, user_data);
		} 
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_WEB_DEL_PAGES)) {
			reply = ac_manage_dbus_web_del_pages(connection, message, user_data);
		}
		
		else if(dbus_message_is_method_call(message, AC_MANAGE_DBUS_INTERFACE, AC_MANAGE_DBUS_EXTEND_COMMAND_EXEC)) {
			reply = ac_manage_dbus_extend_command_exec(connection, message, user_data);
		}
	}
	else if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_SNMP_DBUS_OBJPATH)) {
		if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_SERVICE)) {
			reply = ac_manage_dbus_config_snmp_service(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_COLLECTION_MODE)) {
			reply = ac_manage_dbus_config_snmp_collection_mode(connection, message, user_data);
		} 
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_PFM_REQUESTPKTS)) {
			reply = ac_manage_dbus_config_snmp_pfm_requestpkts(connection, message, user_data);
		} 
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_VERSION_MODE)) {
			reply = ac_manage_dbus_config_snmp_version_mode(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_CACHETIME)) {
			reply = ac_manage_dbus_config_snmp_cachetime(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_ADD_COMMUNITY)) {
			reply = ac_manage_dbus_config_snmp_add_community(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_SET_COMMUNITY)) {
			reply = ac_manage_dbus_config_snmp_set_community(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_DEL_COMMUNITY)) {
			reply = ac_manage_dbus_config_snmp_del_community(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_VIEW)) {
			reply = ac_manage_dbus_config_snmp_view(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CHECK_SNMP_VIEW)) {
			reply = ac_manage_dbus_check_snmp_view(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_VIEW_OID)) {
			reply = ac_manage_dbus_config_snmp_view_oid(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_ADD_GROUP)) {
			reply = ac_manage_dbus_config_snmp_add_group(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_DEL_GROUP)) {
			reply = ac_manage_dbus_config_snmp_del_group(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_ADD_V3USER)) {
			reply = ac_manage_dbus_config_snmp_add_v3user(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_DEL_V3USER)) {
			reply = ac_manage_dbus_config_snmp_del_v3user(connection, message, user_data);
		}		
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_SERVICE)) {
			reply = ac_manage_dbus_config_trap_service(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_CONFIG_RECEIVER)) {
			reply = ac_manage_dbus_config_trap_config_receiver(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_DEL_RECEIVER)) {
			reply = ac_manage_dbus_config_trap_del_receiver(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_SWITCH)) {
			reply = ac_manage_dbus_config_trap_switch(connection, message, user_data);
		}			
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_GROUP_SWITCH)) {
			reply = ac_manage_dbus_config_trap_group_switch(connection, message, user_data);
		}	
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_INSTANCE_HEARTBEAT)) {
			reply = ac_manage_dbus_config_trap_instance_heartbeat(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CLEAR_TRAP_INSTANCE_HEARTBEAT)) {
			reply = ac_manage_dbus_clear_trap_instance_heartbeat(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_PARAMETER)) {
			reply = ac_manage_dbus_config_trap_parameter(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_MANUAL_INSTANCE)) {
			reply = ac_manage_dbus_config_snmp_manual_instance(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_UPDATE_SYSINFO)) {
			reply = ac_manage_dbus_config_snmp_update_sysinfo(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_MANUAL_INSTANCE)) {
			reply = ac_manage_dbus_show_snmp_manual_instance(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_STATE)) {
			reply = ac_manage_dbus_show_snmp_state(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_STATE)) {
			reply = ac_manage_dbus_show_trap_state(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_BASE_INFO)) {
			reply = ac_manage_dbus_show_snmp_base_info(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_PFM_INTERFACE)) {
			reply = ac_manage_dbus_show_snmp_pfm_interface(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_RUNNING_CONFIG)) {
			reply = ac_manage_dbus_show_snmp_running_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_COMMUNITY)) {
			reply = ac_manage_dbus_show_snmp_community(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_VIEW)) {
			reply = ac_manage_dbus_show_snmp_view(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_GROUP)) {
			reply = ac_manage_dbus_show_snmp_group(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_V3USER)) {
			reply = ac_manage_dbus_show_snmp_v3user(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_RECEIVER)) {
			reply = ac_manage_dbus_show_trap_receiver(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_SWITCH)) {
			reply = ac_manage_dbus_show_trap_switch(connection, message, user_data);
		}		
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_INSTANCE_HEARTBEAT)) {
			reply = ac_manage_dbus_show_trap_instance_heartbeat(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_PARAMETER)) {
			reply = ac_manage_dbus_show_trap_parameter(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SNMP_LOG_DEBUG)) {
			reply = ac_manage_dbus_show_snmp_log_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_LOG_DEBUG)) {
			reply = ac_manage_dbus_config_snmp_log_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TRAP_LOG_DEBUG)) {
			reply = ac_manage_dbus_show_trap_log_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_TRAP_LOG_DEBUG)) {
			reply = ac_manage_dbus_config_trap_log_debug(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_SNMP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SNMP_SYSOID_BOARDTYPE)) {
			reply = ac_manage_dbus_config_snmp_sysoid_boardtype(connection, message, user_data);
		}
	}
	else if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_TCRULE_DBUS_OBJPATH)) {	    
		if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_FLOW_CONTROL_SERVICE)) {
			reply = ac_manage_dbus_config_flow_control_service(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_ADD_TCRULE)) {
			reply = ac_manage_dbus_add_tcrule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_OFFSET_TCRULE)) {
			reply = ac_manage_dbus_offset_tcrule(connection, message, user_data);
		}        
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_DELETE_TCRULE)) {
			reply = ac_manage_dbus_delete_tcrule(connection, message, user_data);
		}        
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_FLOW_CONTROL_SERVICE)) {
			reply = ac_manage_dbus_show_flow_control_service(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TCRULE)) {
			reply = ac_manage_dbus_show_tcrule(connection, message, user_data);
		}        
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TCRULE_OFFSET)) {
			reply = ac_manage_dbus_show_tcrule_offset(connection, message, user_data);
		}        
		else if(dbus_message_is_method_call(message, AC_MANAGE_TCRULE_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TCRULE_RUNNING_CONFIG)) {
			reply = ac_manage_dbus_show_tcrule_running_config(connection, message, user_data);
		}
	}
	else if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_FIREWALL_DBUS_OBJPATH)) {	    
		if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_FIREWALL_SERVICE)) {
			reply = ac_manage_dbus_config_firewall_service(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_FIREWALL_RULE)) {
			reply = ac_manage_dbus_config_firewall_rule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_CHANGE_FIREWALL_INDEX)) {
			reply = ac_manage_dbus_change_firewall_index(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_DELETE_FIREWALL_RULE)) {
			reply = ac_manage_dbus_del_firewall_rule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_NAT_UDP_TIMEOUT)) {
			reply = ac_manage_dbus_config_nat_udp_timeout(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_FIREWALL_RULE)) {
			reply = ac_manage_dbus_show_firewall_rule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_STRICT_ACCESS_LEVEL)) {
			reply = ac_manage_dbus_config_strict_access_level(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_FIREWALL_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_STRICT_ACCESS)) {
			reply = ac_manage_dbus_show_strict_access(connection, message, user_data);
		}
	}
	else if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_NTP_DBUS_OBJPATH)) {	    
		if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_NTP_RULE)) {
			reply = ac_manage_dbus_show_ntp_rule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_ADD_NTPSERVER)) {
			reply = ac_manage_dbus_add_ntpserver(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_INSIDE_NTP)) {
			reply = ac_manage_dbus_inside_ntp(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_NTP_STATUS)) {
			reply = ac_manage_dbus_set_ntp_status(connection, message, user_data);
		}		
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_CLENA_NTP)) {
			reply = ac_manage_dbus_clean_ntp(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_NTP_PFM_REQUESTPKTS)) {
			reply = ac_manage_dbus_config_ntp_pfm_requestpkts(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_TIMEZONE)) {
			reply = ac_manage_dbus_set_timezone(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_ADD_NTPCLIENT)) {
			reply = ac_manage_dbus_add_ntpclient(connection, message, user_data);
		}
		if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_TIME)) {
			reply = ac_manage_dbus_show_time(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_TIME)) {
			reply = ac_manage_dbus_set_time(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_NTP_RUNNING_CONFIG)) {
			reply = ac_manage_dbus_show_ntp_running_config(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_ADD_SYSLOGRULE)) {
			reply = ac_manage_dbus_add_syslogrule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_SYSLOG_STATUS)) {
			reply = ac_manage_dbus_set_syslog_status(connection, message, user_data);
		}	
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SHOW_SYSLOG_RULE)) {
			reply = ac_manage_dbus_show_syslog_rule(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_SAVE_SYSLOG)) {
			reply = ac_manage_dbus_save_syslog(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SYSLOGUPLOAD_PFM_REQUESTPKTS)) {
			reply = ac_manage_dbus_config_syslogupload_pfm_requestpkts(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message, AC_MANAGE_NTP_DBUS_INTERFACE, AC_MANAGE_DBUS_CONFIG_SYSLOGRULE)) {
			reply = ac_manage_dbus_config_syslogrule(connection, message, user_data);
		}

	}
	else if(0 == strcmp(dbus_message_get_path(message), AC_MANAGE_ACINFO_DBUS_OBJPATH)){
		if(dbus_message_is_method_call(message, AC_MANAGE_ACINFO_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_ACINFO_VALUE)) {
			reply = ac_manage_dbus_set_acinfo_value(connection, message, user_data);
		}
		else  if(dbus_message_is_method_call(message, AC_MANAGE_ACINFO_DBUS_INTERFACE, AC_MANAGE_DBUS_SET_BKACINFO_VALUE)) {
			reply = ac_manage_dbus_set_bkacinfo_value(connection, message, user_data);
		}

	}
	if (reply) {
		syslog(LOG_DEBUG, "reply destination %s\n", dbus_message_get_destination(reply));
		memset(sender,0, sizeof(sender));
		strcpy(sender, dbus_message_get_destination(reply));
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); 
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}

static DBusHandlerResult
ac_manage_dbus_filter_function (DBusConnection * connection, DBusMessage * message, void *user_data) {

	if(dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected") 
		&& strcmp(dbus_message_get_path(message), DBUS_PATH_LOCAL) == 0) {
		dbus_connection_unref(ac_manage_dbus_connection);
		ac_manage_dbus_connection = NULL;
	}
#if 0	
	else if((dbus_message_is_signal (message, SNMP_MANAGE_DBUS_INTERFACE, NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_MIB_BY_VRRP))) {
		snmp_mib_dbus_had_master_advertise(message);
	}
#endif	
	else {
		return TRUE;
	}
	
	return DBUS_HANDLER_RESULT_HANDLED;
}

int
init_ac_manage_dbus(void) {
    
	DBusError dbus_error;

	DBusObjectPathVTable ac_manage_vtable = {NULL, &ac_manage_dbus_message_handler, NULL, NULL, NULL, NULL};	
    
	dbus_error_init (&dbus_error);

	ac_manage_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
	if(NULL == ac_manage_dbus_connection) {
		syslog(LOG_WARNING, "dbus_bus_get(): %s\n", dbus_error.message);
        	dbus_error_free(&dbus_error);
		return AC_MANAGE_DBUS_ERROR;
	}
	
	if(!dbus_connection_register_fallback(ac_manage_dbus_connection, AC_MANAGE_DBUS_OBJPATH, &ac_manage_vtable, NULL)) {
		syslog(LOG_WARNING, "init_ac_manage_dbus: register fallback fail!\n");
		return AC_MANAGE_DBUS_ERROR;
	}
	
	
	dbus_bus_request_name(ac_manage_dbus_connection, AC_MANAGE_DBUS_DBUSNAME, 0, &dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		syslog(LOG_WARNING, "dbus_bus_get(): %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_connection_add_filter (ac_manage_dbus_connection, ac_manage_dbus_filter_function, NULL, NULL);
				
	dbus_bus_add_match (ac_manage_dbus_connection,
							"type='signal'"
							",interface='"AC_MANAGE_DBUS_INTERFACE"'",
							NULL);

	return AC_MANAGE_SUCCESS;  
    
}

