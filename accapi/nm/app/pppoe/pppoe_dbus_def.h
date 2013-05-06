#ifndef _PPPOE_DBUS_DEF_H
#define _PPPOE_DBUS_DEF_H

#define DBUSNAME_LEN		64
#define DBUSOBJPATH_LEN		128
#define DBUSINTERFACE_LEN	128
#define DBUSMEMBER_LEN		128

#define PPPOE_DBUS_DBUSNAME			"aw.pppoe"
#define PPPOE_DBUS_OBJPATH			"/aw/pppoe"
#define PPPOE_DBUS_INTERFACE		"aw.pppoe"


#define PPPOE_DBUS_CONFIG_LOG_DEBUG					"pppoe_dbus_config_log_debug"
#define PPPOE_DBUS_CONFIG_LOG_TOKEN					"pppoe_dbus_config_log_token"

#define PPPOE_DBUS_DEVICE_CREATE					"pppoe_dbus_device_create"
#define PPPOE_DBUS_DEVICE_DESTROY					"pppoe_dbus_device_destroy"
#define PPPOE_DBUS_DEVICE_CONFIG_BASE				"pppoe_dbus_device_config_base"
#define PPPOE_DBUS_DEVICE_CONFIG_APPLY				"pppoe_dbus_device_config_apply"
#define PPPOE_DBUS_DEVICE_CONFIG_SERVICE			"pppoe_dbus_device_config_service"
#define PPPOE_DBUS_DEVICE_CONFIG_IPADDR				"pppoe_dbus_device_config_ipaddr"
#define PPPOE_DBUS_DEVICE_CONFIG_VIRTUAL_MAC		"pppoe_dbus_device_config_virtual_mac"
#define PPPOE_DBUS_DEVICE_CONFIG_SESSION_IPADDR		"pppoe_dbus_device_config_session_ipaddr"
#define PPPOE_DBUS_DEVICE_CONFIG_SESSION_DNS		"pppoe_dbus_device_config_session_dns"
#define PPPOE_DBUS_DEVICE_CONFIG_NAS_IPADDR			"pppoe_dbus_device_config_nas_ipaddr"
#define PPPOE_DBUS_DEVICE_CONFIG_RADIUS_RDC			"pppoe_dbus_device_config_radius_rdc"
#define PPPOE_DBUS_DEVICE_CONFIG_RADIUS_SERVER		"pppoe_dbus_device_config_radius_server"
#define PPPOE_DBUS_DEVICE_CONFIG_SNAME				"pppoe_dbus_device_config_sname"
#define PPPOE_DBUS_DEVICE_KICK_USER					"pppoe_dbus_device_kick_user"

#define PPPOE_DBUS_INSTANCE_VRRP_SWITCH				"pppoe_dbus_instance_vrrp_switch"

#define PPPOE_DBUS_SHOW_ONLINE_USER					"pppoe_dbus_show_online_user"
#define PPPOE_DBUS_SHOW_DEVICE_LIST					"pppoe_dbus_show_device_list"
#define PPPOE_DBUS_SHOW_PFM_ENTRY					"pppoe_dbus_show_pfm_entry"
#define PPPOE_DBUS_DETECT_DEVICE_EXIST				"pppoe_dbus_detect_device_exist"
#define PPPOE_DBUS_SHOW_RUNNING_CONFIG				"pppoe_dbus_show_running_config"

#endif

