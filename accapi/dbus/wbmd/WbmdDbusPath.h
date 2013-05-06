#ifndef _WBMD_DBUS_PATH_H
#define _WBMD_DBUS_PATH_H
char MSGQ_PATH[PATH_LEN] = "/var/run/wbmd/wmsgq";
char WBMD_DBUS_BUSNAME[PATH_LEN]=	"aw.wbmd";
char WBMD_DBUS_OBJPATH[PATH_LEN]=	"/aw/wbmd";
char WBMD_DBUS_INTERFACE[PATH_LEN]="aw.wbmd";

char WBMD_DBUS_CONF_METHOD_CREATE_WBRIDGE[PATH_LEN]="create_wbridge";
char WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_LIST[PATH_LEN]="show_wbridge_list";
char WBMD_DBUS_CONF_METHOD_WBRIDGE[PATH_LEN]="config_wbridge";
char WBMD_DBUS_CONF_METHOD_SET_WBRIDGE_SNMP[PATH_LEN]="set_wbridge_snmp";
char WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_BASIC_INFO[PATH_LEN]="show_wbridge_basic_info";
char WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RF_INFO[PATH_LEN]="show_wbridge_rf_info";
char WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_MINT_INFO[PATH_LEN]="show_wbridge_mint_info";
char WBMD_DBUS_CONF_METHOD_SHOW_WBRIDGE_RUNNING_CONFIG[PATH_LEN]="show_wbridge_running_config";
char WBMD_DBUS_CONF_METHOD_DELETE_WBRIDGE[PATH_LEN]="delete_wbridge";

#endif
