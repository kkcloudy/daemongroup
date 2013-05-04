#ifndef _AC_MANAGE_DBUS_HANDLER_H
#define _AC_MANAGE_DBUS_HANDLER_H

DBusMessage *ac_manage_dbus_config_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_token_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_manual_instance(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_manual_instance(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_service(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_collection_mode(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_proxy_pfm_config(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_pfm_requestpkts(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_version_mode(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_update_sysinfo(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_cachetime(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_add_community(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_set_community(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_del_community(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_check_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_view_oid(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_add_group(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_del_group(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_add_v3user(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_del_v3user(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_snmp_sysoid_boardtype(DBusConnection *connection, DBusMessage *message, void *user_data);



DBusMessage *ac_manage_dbus_show_snmp_state(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_base_info(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_pfm_interface(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_community(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_group(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_snmp_v3user(DBusConnection *connection, DBusMessage *message, void *user_data) ;

DBusMessage *ac_manage_dbus_show_snmp_running_config(DBusConnection *connection, DBusMessage *message, void *user_data);


DBusMessage *ac_manage_dbus_config_trap_service(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_config_receiver(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_del_receiver(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_switch(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_group_switch(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_clear_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_trap_parameter(DBusConnection *connection, DBusMessage *message, void *user_data);


DBusMessage *ac_manage_dbus_show_trap_state(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_receiver(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_switch(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_parameter(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_trap_running_config(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_manual_mib_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_mib_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_get_mib_localslot_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_mib_accumulate_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_radius_config(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_portal_config(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_edit(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_show(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_conf(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_download(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_show_pages(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_web_del_pages(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_extend_command_exec(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_flow_control_service(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_add_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_offset_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_delete_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_flow_control_service(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_tcrule_offset(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_tcrule_running_config(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_firewall_service(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_change_firewall_index(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_del_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_config_nat_udp_timeout(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_show_ntp_rule(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *ac_manage_dbus_add_ntpserver(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *
ac_manage_dbus_config_strict_access_level(DBusConnection *connection, DBusMessage *message, void *user_data);

DBusMessage *
ac_manage_dbus_show_strict_access(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *
ac_manage_dbus_add_ntpclient(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *
ac_manage_dbus_config_syslogrule(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *ac_manage_dbus_set_acinfo_value(DBusConnection *connection, DBusMessage *message, void *user_data);
DBusMessage *ac_manage_dbus_set_bkacinfo_value(DBusConnection *connection, DBusMessage *message, void *user_data);

#endif
