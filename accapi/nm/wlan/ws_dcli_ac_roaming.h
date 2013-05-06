#ifndef _WS_DCLI_AC_ROAMING_H
#define _WS_DCLI_AC_ROAMING_H

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

struct roaming_count_profile
{
	int total_count;
	int in_count;
	int out_count;
};

extern int show_inter_ac_roaming_count_cmd(dbus_parameter parameter, DBusConnection *connection, struct roaming_count_profile *count_info);/*返回0表示失败，返回1表示成功*/
																																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#endif
