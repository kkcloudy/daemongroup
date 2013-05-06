#ifndef _SIGTRAN_DBUS_PATH_H
#define _SIGTRAN_DBUS_PATH_H

#ifndef PATH_LEN
#define PATH_LEN (64)
#endif

char UDP_IU_DBUS_BUSNAME[PATH_LEN]=		"aw.sigtran";
char UDP_IU_DBUS_OBJPATH[PATH_LEN]=		"/aw/sigtran";
char UDP_IU_DBUS_INTERFACE[PATH_LEN]=	"aw.sigtran";

char UDP_IU_SET_SELF_POINT_CODE[PATH_LEN]=	"sigtran_point_node";
char UDP_IU_SET_MSC[PATH_LEN]=	"sigtran_set_msc_node";
char UDP_IU_SET_SGSN[PATH_LEN]=	"sigtran_set_sgsn_node";
char UDP_IU_DBUS_METHOD_SET_DEBUG_STATE[PATH_LEN]=	"sigtran_set_debug_state";
char UDP_IU_DBUS_METHOD_SHOW_RUNNING_CFG[PATH_LEN]=	"sigtran_show_running_cfg";
char UDP_IU_DBUS_METHOD_SET_SIGTRAN_ENABLE[PATH_LEN]=	"sigtran_enable";

#endif

