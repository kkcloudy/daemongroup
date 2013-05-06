#ifndef _IU_DBUS_PATH_H
#define _IU_DBUS_PATH_H

#ifndef PATH_LEN
#define PATH_LEN (64)
#endif

char IU_DBUS_BUSNAME[PATH_LEN]=		"aw.iu";
char IU_DBUS_OBJPATH[PATH_LEN]=		"/aw/iu";
char IU_DBUS_INTERFACE[PATH_LEN]=		"aw.iu";

char IU_SET_LOCAL_PARAS[PATH_LEN]=		"iu_set_self_paras";
char IU_SET_REMOTE_PARAS[PATH_LEN]=		"iu_set_remote_paras";
char IU_DBUS_METHOD_SET_DEBUG_STATE[PATH_LEN]=	"iu_set_debug_state";
char IU_DBUS_METHOD_SHOW_RUNNING_CFG[PATH_LEN]=	"iu_show_running_cfg";
char IU_DBUS_METHOD_SET_IU_ENABLE[PATH_LEN]=	"iu_service_enable";
char IU_DBUS_METHOD_SET_IU_TO_SIGTRAN_ENABLE[PATH_LEN]=	"iu_to_sigtran_enable";

char IU_SET_ROUTING_CONTEXT[PATH_LEN]="iu_set_routing_context";
char IU_SET_TRAFFIC_MODE[PATH_LEN]="iu_set_traffic_mode";

char IU_SET_NETWORK_INDICATOR[PATH_LEN]="iu_set_network_indicator";
char IU_SET_NETWORK_APPERANCE[PATH_LEN]="iu_set_network_apperance";

/* book modify, 2011-12-28 */
char IU_SET_ADDRESS[PATH_LEN]="iu_set_address";
char IU_SET_POINT_CODE[PATH_LEN]="iu_set_point_code";
char IU_SET_CONNECTION_MODE[PATH_LEN]="iu_set_connection_mode";
char IU_SET_MULTI_SWITCH[PATH_LEN]="iu_set_multi_switch";
/* book add, 2012-1-4 */
char IU_GET_LINK_STATUS[PATH_LEN]="iu_get_link_status";

#endif

