#ifndef _HMD_DBUS_PATH_H
#define _HMD_DBUS_PATH_H
#include "hmd/hmdpub.h"

char HMD_DBUS_BUSNAME[PATH_LEN]=	"aw.hmd";
char HMD_DBUS_OBJPATH[PATH_LEN]=	"/aw/hmd";
char HMD_DBUS_INTERFACE[PATH_LEN]="aw.hmd";
char HMD_DBUS_CONF_METHOD_LOCAL_HANSI[PATH_LEN]= "config_local_hansi";
char HMD_DBUS_METHOD_DELETE_LOCAL_HANSI[PATH_LEN]= "delete_local_hansi";

char HMD_DBUS_CONF_METHOD_REMOTE_HANSI_STATE_CHANGE[PATH_LEN]= "remote_hansi_state_change";
char HMD_DBUS_CONF_METHOD_DHCP_AUTO_RESTART[PATH_LEN]= "auto_dhcp_restart";

char HMD_DBUS_CONF_METHOD_VRRP_GLOBAL_SWITCH_ADD_DEL_HANSI[PATH_LEN] = "config_vrrp_global_switch_add_del_hansi";

char HMD_DBUS_CONF_METHOD_REMOTE_HANSI[PATH_LEN]= "config_remote_hansi";
char HMD_DBUS_METHOD_SET_LOCAL_HANSI_STATE[PATH_LEN]= "set_local_hansi_state";
char HMD_DBUS_METHOD_SET_LOCAL_HANSI_UPLINK_IF[PATH_LEN]= "set_local_hansi_uplink_if";
char HMD_DBUS_METHOD_SET_LOCAL_HANSI_DOWNLINK_IF[PATH_LEN]= "set_local_hansi_downlink_if";
char HMD_DBUS_METHOD_DELETE_SLOTID_IMG[PATH_LEN]= "delete_slotid_img"; /*fengwenchao add 20120228 for AXSSZFI-680*/
char HMD_DBUS_METHOD_SET_LOCAL_HANSI_DISABLE_ENABLE[PATH_LEN]= "set_local_hansi_enable_disable";
char HMD_DBUS_METHOD_SHOW_HMD_INFO[PATH_LEN]= "show_hmd_info";     /*fengwenchao add 20110616*/
char HMD_DBUS_METHOD_SET_LOCAL_HANSI_GATEWAY_IF[PATH_LEN] = "set_local_hansi_gateway_if";
char HMD_DBUS_METHOD_HAD_NOTIFY_HMD_STATE[PATH_LEN]= "had_notify_hmd_state";
char HMD_DBUS_METHOD_WID_NOTIFY_HMD_UPDATE_STATE[PATH_LEN]= "wid_notify_hmd_state";
char HMD_DBUS_METHOD_ASD_NOTIFY_HMD_UPDATE_STATE[PATH_LEN]= "asd_notify_hmd_state";
char HMD_DBUS_HAD_GET_ALIVE_SLOT_NO[PATH_LEN]= "had_get_alive_slot_no";
char HMD_DBUS_METHOD_DELETE_REMOTE_HANSI[PATH_LEN]= "delete_remote_hansi";
char HMD_DBUS_METHOD_SHOW_RUNNING[PATH_LEN] = "hmd_show_running_config";
char HMD_DBUS_METHOD_HANSI_SHOW_RUNNING[PATH_LEN] = "hmd_hansi_show_running_config";
char HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN[PATH_LEN]= "assign_license";
char HMD_DBUS_CONF_METHOD_LICENSE_ASSIGN_SHOW[PATH_LEN]= "assign_license_show";
char HMD_DBUS_METHOD_LICENSE_SHOW_RUNNING[PATH_LEN] = "hmd_license_show_running_config";
char HMD_DBUS_METHOD_SET_HANSI_CHECK_STATE[PATH_LEN]= "set_hansi_check_state";
char HMD_DBUS_CONF_METHOD_SERVICE_TFTP_SWITCH[PATH_LEN]="service_tftp_switch";
char HMD_DBUS_CONF_METHOD_SHOW_SERVICE_TFTP_SWITCH[PATH_LEN]="show_service_tftp";
char HMD_DBUS_CONF_METHOD_SERVICE_FTP_SWITCH[PATH_LEN]="service_ftp_switch";
char HMD_DBUS_CONF_METHOD_SHOW_SERVICE_FTP_SWITCH[PATH_LEN]="show_service_ftp";
char HMD_DBUS_CONF_METHOD_SET_FAST_FORWARDING[PATH_LEN]="set_fast_forwarding";
char HMD_DBUS_CONF_METHOD_SET_TUNNEL_QOS_MAP[PATH_LEN]="set_tunnel_qos_map";
char HMD_DBUS_CONF_METHOD_SET_TUNNEL_MM[PATH_LEN]="set_tunnel_mm";
char HMD_DBUS_CONF_METHOD_SET_IPFRAG_INGRESS_PMTU[PATH_LEN]="set_ipfrag_ingress_pmtu";
char HMD_DBUS_CONF_METHOD_SET_IPFRAG_INFORM_NHMTU_INSTEAD[PATH_LEN]="set_ipfrag_inform_nhmtu_instead";
char HMD_DBUS_CONF_METHOD_SET_IPFRAG_IGNOREDF[PATH_LEN]="set_ipfrag_ignoredf";
char HMD_DBUS_CONF_METHOD_SHOW_AP_IMG_FILES[PATH_LEN] = "show_ap_img_files";
char HMD_DBUS_CONF_METHOD_SHOW_HMD_BAK_FOREVER_INFO[PATH_LEN] = "show_hmd_bak_forever_info";
char HMD_DBUS_CONF_METHOD_SET_WID_CLEAR_BINDING_INTERFACE_FLAG[PATH_LEN] = "set_wid_clear_binding_interface_flag";//fengwenchao copy from 1318 for AXSSZFI-839
char HMD_DBUS_CONF_METHOD_SET_VRRP_BAKUP_FOREVER[PATH_LEN]= "set_vrrp_bakup_forever";
char HMD_DBUS_METHOD_DELETE_LOCAL_HANSI_LINK_IF[PATH_LEN]="delete_link_ifname";
char HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_SWITCH[PATH_LEN] = "femto_service_switch";
char HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_CHECK[PATH_LEN] = "femto_service_state_check";
char HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_STATE[PATH_LEN] = "set_hmd_timer_config_save_state";/*fengwenchao add 20130412 for hmd timer config save*/
char HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_TIMER[PATH_LEN] = "set_hmd_timer_config_save_timer";/*fengwenchao add 20130412 for hmd timer config save*/
char HMD_DBUS_METHOD_SYNC_WCPSS_CFG[PATH_LEN] = "hmd_sync_wcpss_cfg";
char HMD_DBUS_CONF_SYNC_TO_VRRP_BAKUP[PATH_LEN] = "hmd_conf_sync_to_vrrp_backup";
char HMD_DBUS_SET_GLOBAL_BRIDGE_MCAST[PATH_LEN]= "config_global_bridge_mcast";
char HMD_DBUS_CONF_SET_AUTO_SYNC_CONFIG_IP[PATH_LEN] = "set_auto_sync_config_ip";
char HMD_DBUS_CONF_SET_AUTO_SYNC_CONFIG_TIME[PATH_LEN] = "set_auto_sync_config_time";
char HMD_DBUS_CONF_SET_AUTO_SYNC_CONFIG_SWITCH[PATH_LEN] = "set_auto_sync_config_switch";
char HMD_DBUS_CONF_SHOW_AUTO_SYNC_CONFIG_STATE[PATH_LEN] = "show_auto_sync_config_state";
char HMD_DBUS_CONF_HANSI_DEPEND[PATH_LEN] = "hmd_dbus_config_hansi_depend";
char HMD_DBUS_CONF_HANSI_DEPEND_DELETE[PATH_LEN] = "hmd_dbus_config_hansi_depend_delete";
char HMD_DBUS_CONF_HANSI_GROUP[PATH_LEN] = "hmd_dbus_config_hansi_reference_group";
char HMD_DBUS_DELETE_HANSI_GROUP[PATH_LEN] = "hmd_dbus_delete_hansi_reference_group";
char HMD_DBUS_CONF_SHOW_HANSI_REFERENCE_GROUP[PATH_LEN] = "hmd_dbus_show_hansi_reference_group";
char HMD_DBUS_CONF_SHOW_GROUP_MEMBER[PATH_LEN] = "hmd_dbus_show_reference_group_member";
char HMD_DBUS_ADD_HANSI_TO_REFERENCE_GROUP[PATH_LEN] = "hmd_dbus_add_hansi_to_reference_group";
char HMD_DBUS_DEL_HANSI_FROM_REFERENCE_GROUP[PATH_LEN] = "hmd_dbus_del_hansi_from_reference_group";
char HMD_DBUS_METHOD_HANSI_GROUP_SHOW_RUNNING[PATH_LEN] = "hmd_dbus_hansi_reference_group_show_running";
char HMD_DBUS_SHOW_HANSI_GROUP[PATH_LEN] ="hmd_dbus_show_hansi_group";

#endif

