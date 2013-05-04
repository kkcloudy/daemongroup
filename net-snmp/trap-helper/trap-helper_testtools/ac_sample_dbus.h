#ifndef _AC_SAMPLE_DBUS_H
#define _AC_SAMPLE_DBUS_H

#include <dbus/dbus.h>


#define AC_SAMPLE_DBUS_BUSNAME      "aw.sample"
#define AC_SAMPLE_DBUS_OBJPATH      "/aw/sample"
#define AC_SAMPLE_DBUS_INTERFACE    "aw.sample"



#define AC_SAMPLE_DBUS_METHOD_SET_SERVICE_STATE  	"ac_sample_dbus_method_set_service_state"
#define AC_SAMPLE_DBUS_METHOD_GET_SERVICE_STATE  	"ac_sample_dbus_method_get_service_state"

#define AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_PARAM   	"ac_sample_dbus_method_set_sample_param"
#define AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_PARAM   	"ac_sample_dbus_method_get_sample_param"

#define AC_SAMPLE_DBUS_METHOD_SET_SIGNAL_THRESHOLD  "ac_sample_dbus_method_set_signal_threshold"
#define AC_SAMPLE_DBUS_METHOD_GET_SIGNAL_THRESHOLD  "ac_sample_dbus_method_get_signal_threshold"

#define AC_SAMPLE_DBUS_METHOD_SET_SAMPLE_STATE		"ac_sample_dbus_method_set_sample_state"
#define AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_STATE		"ac_sample_dbus_method_get_sample_state"

#define AC_SAMPLE_DBUS_METHOD_SET_DEBUG_LEVEL		"ac_sample_dbus_method_set_debug_level"

#define AC_SAMPLE_DBUS_METHOD_GET_SAMPLE_INFO       "ac_sample_dbus_method_get_sample_info"
//接口带宽信息
#define AC_SAMPLE_DBUS_METHOD_GET_IFACE_INFO        "ac_sample_dbus_method_get_iface_info"


int  ac_sample_dbus_init(void);
void ac_sample_dbus_dispach( unsigned int block_usecond );

int ac_sample_dbus_send_signal( const char *signal_name, 
								int first_arg_type,...);

DBusConnection *ac_sample_dbus_get_connection();



/*for trap signal*/
#define TRAP_DBUS_INTERFACE								"aw.trap"

#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_CPU				"ac_sample_cpu_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MEMUSAGE		"ac_sample_memusage_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_TEMPERATURE		"ac_sample_temperature_over_threshold"

#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH		"ac_sample_band_with_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE		"ac_sample_drop_rate_over_threshold"


#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER		"ac_sample_online_user_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ		"ac_sample_radius_req_fail_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL			"ac_sample_ip_pool_over_threshold"

#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH    "ac_sample_portal_server_reach_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH     "ac_sample_radius_auth_server_reach_status_signal"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC    	"ac_sample_radius_acc_server_reach_status_signal"


#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DHCP_EXHAUST	"ac_sample_dhcp_exhaust_over_threshold"
#define AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK	"ac_sample_find_attack_over_threshold"

#endif

