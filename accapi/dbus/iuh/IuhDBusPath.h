#ifndef _IUH_DBUS_PATH_H
#define _IUH_DBUS_PATH_H

char IUH_DBUS_BUSNAME[PATH_LEN]=	"aw.iuh";
char IUH_DBUS_OBJPATH[PATH_LEN]=	"/aw/iuh";
char IUH_DBUS_INTERFACE[PATH_LEN]="aw.iuh";

char IUH_DBUS_CONF_METHOD_SET_IUH_DYNAMIC_HNB_LOGIN_INTERFACE[PATH_LEN]=	"set_auto_hnb_binding_interface";
char IUH_DBUS_SECURITY_METHOD_SET_IUH_DAEMONLOG_DEBUG[PATH_LEN]=    "set_iuh_daemonlog";
char IUH_DBUS_HNB_METHOD_SHOW_HNBINFO_BY_HNBID[PATH_LEN]=  "show_hnb_info_by_hnbid";
char IUH_DBUS_HNB_METHOD_SHOW_HNB_LIST[PATH_LEN]=  "show_hnb_list";
char IUH_DBUS_SECURITY_METHOD_DELETE_HNB_BY_HNBID[PATH_LEN]=  "delete_hnb_by_hnbid";
char IUH_DBUS_HNB_METHOD_SHOW_UEINFO_BY_UEID[PATH_LEN]=  "show_ue_info_by_ueid";
char IUH_DBUS_HNB_METHOD_SHOW_UE_LIST[PATH_LEN]=  "show_ue_list";
char IUH_DBUS_SECURITY_METHOD_DELETE_UE_BY_UEID[PATH_LEN]=  "delete_ue_by_ueid";
char IUH_DBUS_IUH_SET_ASN_DEBUG_SWITCH[PATH_LEN]= "set_asn_debug_switch";
char IUH_DBUS_IUH_SET_RNCID[PATH_LEN]= "set_rncid";
char IUH_DBUS_METHOD_SHOW_RUNNING_CFG[PATH_LEN]= "show_iuh_running_config";
char IUH_SET_PAGING_OPTIMIZE_SWITCH[PATH_LEN]= "set_paging_optimize_switch";
char IUH_FEMTO_ACL_WHITE_LIST[PATH_LEN]=	"femto_acl_white_list";
char IUH_DBUS_IUH_TIPC_INIT[PATH_LEN]= "femto_iuh_tipc_init";

#endif
