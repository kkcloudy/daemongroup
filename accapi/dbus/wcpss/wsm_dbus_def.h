/******************************************************************************
* Description:
*   WSM module Dbus_PATH macro file.
*
* Date:
*   2009-10-13
*
* Author:
*   guoxb@autelan.com
*
* Copyright:
*   Copyright (C) Autelan Technology. All right reserved.
*
* Modify history:
*   2009-10-13  <Guo Xuebin>  Create file.
*   2009-11-04  <Guo Xuebin>  Add wsm watchdog dbus switch macro.
*
******************************************************************************/
#ifndef _WSM_DBUS_DEF_H
#define _WSM_DBUS_DEF_H

#define WSM_DBUS_BUSNAME		"aw.wsm"
#define WSM_DBUS_OBJPATH		"/aw/wsm"
#define WSM_DBUS_INTERFACE	"aw.wsm"

#define WSM_DBUS_CONF_TUNNEL		"wsm_conf_tunnel"
#define WSM_DBUS_CONF_SET_TUNNEL "wsm_conf_set_ipfwd_tunnel"
#define WSM_DBUS_CONF_SHOW_WTP_LIST "wsm_conf_show_wtp_list"
#define WSM_DBUS_CONF_SHOW_LOG_WSM_STATE "wsm_conf_show_log_wsm_state"
#define WSM_DBUS_CONF_SET_LOG_WSM_LEVEL "wsm_conf_set_log_wsm_level"
#define WSM_DBUS_CONF_SHOW_WTP_WTPID "wsm_conf_show_wtp_wtpid"
#define WSM_DBUS_CONF_SHOW_BSS_BSSID "wsm_conf_show_bss_bssid"
#define WSM_DBUS_CONF_SHOW_STA_STAMAC "wsm_conf_show_sta_stamac"
#define WSM_DBUS_CONF_SHOW_WSM_WATCH_DOG_STATE "wsm_conf_show_watchdog_state"
#define WSM_DBUS_CONF_SET_WSM_WATCHDOG_STATE "wsm_conf_set_wsm_watchdog_state"

#define WSM_DBUS_CAPWAP_TUNNEL_SHOW_RUNNING "wsm_capwap_tunnel_show_running"
#define WSM_DBUS_CONF_METHOD_SET_WSM_ERROR_HANDLE_STATE "set_wsm_error_handle_state"
#define WSM_DBUS_CONF_METHOD_CHECKING "wsm_checking"
#define WSM_DBUS_CONF_METHOD_QUIT "wsm_quit"
#define WSM_DBUS_CONF_SET_TUNNEL_WSM_RX_TX "wsm_conf_set_wsm_receive_transmit"

#define WSM_MAC_LEN 6

typedef struct {
	unsigned int WTPID;
	unsigned int WTPIP;
	unsigned int sta_cnt;
	unsigned char bss_cnt;
	unsigned char mac[WSM_MAC_LEN];
} wsm_wtp_info;

typedef struct {
	unsigned int WTPID;
	unsigned int WTPIP;
	unsigned int sta_cnt;
	unsigned char bss_cnt;
} wsm_wtp_tree_info;

typedef struct {
	unsigned int BSSIndex;
	unsigned int Radio_G_ID;
	unsigned int WlanID;
	unsigned int sta_cnt;
	unsigned char BSSID[WSM_MAC_LEN];
} wsm_bss_info;


#endif
