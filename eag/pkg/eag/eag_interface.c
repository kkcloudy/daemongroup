#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "eag_errcode.h"
#include "limits2.h"
#include "nm_list.h"
#include "hashtable.h"
#include "eag_conf.h"
#include "session.h"
#include "eag_interface.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#define MAX_DBUS_OBJPATH_LEN	128
#define MAX_DBUS_BUSNAME_LEN	128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

/*base conf*/
#define EAG_DBUS_METHOD_SET_NASIP			"eag_dbus_method_set_nasip"
//#define EAG_DBUS_METHOD_SET_DISTRIBUTED		"eag_dbus_method_set_distributed"
#define EAG_DBUS_METHOD_SET_RDC_DISTRIBUTED	"eag_dbus_method_set_rdc_distributed"
#define EAG_DBUS_METHOD_SET_PDC_DISTRIBUTED	"eag_dbus_method_set_pdc_distributed"
//#define EAG_DBUS_METHOD_SET_PDCRDC_INS		"eag_dbus_method_set_rdcpdc_ins"
#define EAG_DBUS_METHOD_SET_RDC_INS			"eag_dbus_method_set_rdc_ins"
#define EAG_DBUS_METHOD_SET_PDC_INS			"eag_dbus_method_set_pdc_ins"

#define EAG_DBUS_METHOD_SET_PORTAL_PORT				"eag_dbus_method_set_portal_port"
#define EAG_DBUS_METHOD_SET_PORTAL_RETRY_PARAMS		"eag_dbus_method_set_portal_retry_params"
#define EAG_DBUS_METHOD_SET_AUTO_SESSION			"eag_dbus_method_set_auto_session"

#define EAG_DBUS_METHOD_SET_RADIUS_ACCT_INTERVAL	"eag_dbus_method_set_radius_acct_interval"
#define EAG_DBUS_METHOD_SET_RADIUS_RETRY_PARAMS		"eag_dbus_method_set_radius_retry_params"

#define EAG_DBUS_METHOD_SET_MAX_REDIR_TIMES	"eag_dbus_method_set_max_redir_times"
#define EAG_DBUS_METHOD_SET_FORCE_DHCPLEASE		"eag_dbus_method_set_force_dhcplease"
#define EAG_DBUS_METHOD_SET_CHECK_ERRID		"eag_dbus_method_set_check_errid"

#define EAG_DBUS_METHOD_SET_IDLE_PARAMS			"eag_dbus_method_set_idle_params"
#define EAG_DBUS_METHOD_SET_FORCE_WIRELESS		"eag_dbus_method_set_force_wireless"
#define EAG_DBUS_METHOD_SET_FLUX_FROM			"eag_dbus_method_set_flux_from"
#define EAG_DBUS_METHOD_SET_FLUX_INTERVAL		"eag_dbus_method_set_flux_interval"
#define EAG_DBUS_METHOD_SET_IPSET_AUTH			"eag_dbus_method_set_ipset_auth"
#define EAG_DBUS_METHOD_SET_TRAP_ONLINEUSERNUM_SWITCH "eag_dbus_method_set_trap_onlineusernum_switch"
#define EAG_DBUS_METHOD_SET_THRESHOLD_ONLINEUSERNUM "eag_dbus_method_set_threshold_onlineusernum"
#define EAG_DBUS_METHOD_SET_CHECK_NASPORTID			"eag_dbus_method_set_check_nasportid"
#define EAG_DBUS_METHOD_SET_OCTETS_CORRECT_FACTOR	"eag_dbus_method_set_octets_correct_factor"

#define EAG_DBUS_METHOD_SET_SERVICE_STATUS	"eag_dbus_method_set_service_status"
#define EAG_DBUS_METHOD_GET_BASE_CONF		"eag_dbus_method_get_base_conf"
#define EAG_DBUS_METHOD_SHOW_RELATIVE_TIME	"eag_dbus_method_show_relative_time"
#define EAG_DBUS_METHOD_SET_TRAP_SWITCH_ABNORMAL_LOGOFF		"eag_dbus_method_set_trap_switch_abnormal_logoff"
#define EAG_DBUS_METHOD_SET_CLASS_TO_BANDWIDTH_SWITCH		"eag_dbus_method_set_class_to_bandwidth_switch"
#define EAG_DBUS_METHOD_SET_PORTAL_PROTOCOL			"eag_dbus_method_set_portal_protocol"
#define EAG_DBUS_METHOD_SET_MACAUTH_SWITCH			"eag_dbus_method_set_macauth_switch"
#define EAG_DBUS_METHOD_SET_L2SUPER_VLAN_SWITCH		"eag_dbus_method_set_l2super_vlan_switch"
#define EAG_DBUS_METHOD_SET_MACAUTH_IPSET_AUTH		"eag_dbus_method_set_macauth_ipset_auth"
#define EAG_DBUS_METHOD_SET_MACAUTH_FLUX_FROM		"eag_dbus_method_set_macauth_flux_from"
#define EAG_DBUS_METHOD_SET_MACAUTH_FLUX_INTERVAL	"eag_dbus_method_set_macauth_flux_interval"
#define EAG_DBUS_METHOD_SET_MACAUTH_FLUX_THRESHOLD	"eag_dbus_method_set_macauth_flux_threshold"
#define EAG_DBUS_METHOD_SET_MACAUTH_NOTICE_BINDSERVER "eag_dbus_method_set_macauth_notice_bindserver"

/*nasid*/
#define EAG_DBUS_METHOD_ADD_NASID				"eag_dbus_method_add_nasid"
#define EAG_DBUS_METHOD_MODIFY_NASID			"eag_dbus_method_modify_nasid"
#define EAG_DBUS_METHOD_DEL_NASID				"eag_dbus_method_del_nasid"
#define EAG_DBUS_METHOD_GET_NASID				"eag_dbus_method_get_nasid"

/*nasportid*/
#define EAG_DBUS_METHOD_ADD_NASPORTID			"eag_dbus_method_add_nasportid"
#define EAG_DBUS_METHOD_DEL_NASPORTID			"eag_dbus_method_del_nasportid"
#define EAG_DBUS_METHOD_GET_NASPORTID			"eag_dbus_method_get_nasportid"

/*captive*/
#define EAG_DBUS_METHOD_ADD_CAPTIVE_INTF			"eag_dbus_method_add_captive_intf"
#define EAG_DBUS_METHOD_DEL_CAPTIVE_INTF			"eag_dbus_method_del_captive_intf"
#define EAG_DBUS_METHOD_SHOW_CAPTIVE_INTFS			"eag_dbus_method_show_captive_intfs"
#define EAG_DBUS_METHOD_CONF_CAPTIVE_LIST			"eag_dbus_method_conf_captive_list"
#define EAG_DBUS_METHOD_SHOW_WHITE_LIST				"eag_dbus_method_show_white_list"
#define EAG_DBUS_METHOD_SHOW_BLACK_LIST				"eag_dbus_method_show_black_list"

/*portal*/
#define EAG_DBUS_METHOD_ADD_PORTAL					"eag_dbus_method_add_portal"
#define EAG_DBUS_METHOD_MODIFY_PORTAL				"eag_dbus_method_modify_portal"
#define EAG_DBUS_METHOD_DEL_PORTAL					"eag_dbus_method_del_portal"
#define EAG_DBUS_METHOD_GET_PORTAL_CONF				"eag_dbus_method_get_portal_conf"
#define EAG_DBUS_METHOD_SET_ACNAME					"eag_dbus_method_set_acname"
#define EAG_DBUS_METHOD_SET_ACIP_TO_URL				"eag_dbus_method_set_acip_to_url"
#define EAG_DBUS_METHOD_SET_NASID_TO_URL			"eag_dbus_method_set_nasid_to_url"
#define EAG_DBUS_METHOD_SET_WLANPARAMETER			"eag_dbus_method_set_wlanparameter"
#define EAG_DBUS_METHOD_SET_WLANUSERFIRSTURL		"eag_dbus_method_set_wlanuserfirsturl"
#define EAG_DBUS_METHOD_SET_URL_SUFFIX				"eag_dbus_method_set_url_suffix"
#define EAG_DBUS_METHOD_SET_PORTAL_SECRET			"eag_dbus_method_set_portal_secret"
#define EAG_DBUS_METHOD_SET_WLANAPMAC				"eag_dbus_method_set_wlanapmac"
#define EAG_DBUS_METHOD_SET_USERMAC_TO_URL			"eag_dbus_method_set_usermac_to_url"
#define EAG_DBUS_METHOD_SET_CLIENTMAC_TO_URL		"eag_dbus_method_set_clientmac_to_url"
#define EAG_DBUS_METHOD_SET_APMAC_TO_URL			"eag_dbus_method_set_apmac_to_url"
#define EAG_DBUS_METHOD_SET_WLAN_TO_URL				"eag_dbus_method_set_wlan_to_url"
#define EAG_DBUS_METHOD_SET_REDIRECT_TO_URL			"eag_dbus_method_set_redirect_to_url"
#define EAG_DBUS_METHOD_SET_WLANUSERMAC				"eag_dbus_method_set_wlanusermac"
#define EAG_DBUS_METHOD_SET_WISPRLOGIN				"eag_dbus_method_set_wisprlogin"

/*radius*/
#define EAG_DBUS_METHOD_ADD_RADIUS					"eag_dbus_method_add_radius"
#define EAG_DBUS_METHOD_MODIFY_RADIUS				"eag_dbus_method_modify_radius"
#define EAG_DBUS_METHOD_DEL_RADIUS          		"eag_dbus_method_del_radius"
#define EAG_DBUS_METHOD_SET_REMOVE_DOMAIN_SWITCH    "eag_dbus_method_set_remove_domain_switch"
#define EAG_DBUS_METHOD_GET_RADIUS_CONF				"eag_dbus_method_get_radius_conf"

/*debug*/
#define EAG_DBUS_METHOD_ADD_DEBUG_FILTER	"eag_dbus_method_add_debug_filter"
#define EAG_DBUS_METHOD_DEL_DEBUG_FILTER	"eag_dbus_method_del_debug_filter"
#define EAG_DBUS_METHOD_LOG_ALL_APPCONN     "eag_dbus_method_log_all_appconn"
#define EAG_DBUS_METHOD_LOG_ALL_REDIRCONN	"eag_dbus_method_log_all_redirconn"
#define EAG_DBUS_METHOD_LOG_ALL_PORTALSESS	"eag_dbus_method_log_all_portalsess"
#define EAG_DBUS_METHOD_LOG_ALL_SOCKRADIUS	"eag_dbus_method_log_all_sockradius"
#define EAG_DBUS_METHOD_LOG_ALL_THREAD		"eag_dbus_method_log_all_thread"
#define EAG_DBUS_METHOD_LOG_ALL_BLKMEM		"eag_dbus_method_log_all_blkmem"
#define EAG_DBUS_METHOD_LOG_ALL_MAC_PREAUTH	"eag_dbus_method_log_all_mac_preauth"
#define EAG_DBUS_METHOD_LOG_DEBUG_ON		"eag_dbus_method_log_debug_on"
#define EAG_DBUS_METHOD_LOG_DEBUG_OFF		"eag_dbus_method_log_debug_off"

#define EAG_DBUS_METHOD_GET_EAG_STATISTICS	"eag_dbus_method_get_eag_statistics"
#define EAG_DBUS_METHOD_GET_AP_STATISTICS	"eag_dbus_method_get_ap_statistics"
#define EAG_DBUS_METHOD_GET_BSS_STATISTICS	"eag_dbus_method_get_bss_statistics"
#define EAG_DBUS_METHOD_SET_RDC_CLIENT_LOG	"eag_dbus_method_set_rdc_client_log"
#define EAG_DBUS_METHOD_SET_PDC_CLIENT_LOG	"eag_dbus_method_set_pdc_client_log"

/*portal station info*/
#define EAG_DBUS_METHOD_SHOW_USER_ALL			"eag_dbus_method_show_user_all"
#define EAG_DBUS_METHOD_SHOW_USER_BY_USERNAME	"eag_dbus_method_show_user_by_username"
#define EAG_DBUS_METHOD_SHOW_USER_BY_USERIP		"eag_dbus_method_show_user_by_userip"
#define EAG_DBUS_METHOD_SHOW_USER_BY_USERMAC	"eag_dbus_method_show_user_by_usermac"
#define EAG_DBUS_METHOD_SHOW_USER_BY_INDEX		"eag_dbus_method_show_user_by_index"
#define EAG_DBUS_METHOD_KICK_USER_BY_USERNAME	"eag_dbus_method_kick_user_by_username"
#define EAG_DBUS_METHOD_KICK_USER_BY_USERIP		"eag_dbus_method_kick_user_by_userip"
#define EAG_DBUS_METHOD_KICK_USER_BY_USERMAC	"eag_dbus_method_kick_user_by_usermac"
#define EAG_DBUS_METHOD_KICK_USER_BY_INDEX		"eag_dbus_method_kick_user_by_index"
#define EAG_DBUS_METHOD_SET_USER_LOG_STATUS		"eag_dbus_method_set_user_log_status"
#define EAG_DBUS_METHOD_SET_LOG_FORMAT_STATUS	"eag_dbus_method_set_log_format_status"
#define EAG_DBUS_METHOD_SET_USERNAME_CHECK_STATUS	"eag_dbus_method_set_username_check_status"

static char EAG_DBUS_NAME[MAX_DBUS_BUSNAME_LEN]="";
static char EAG_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN]="";
static char EAG_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN]="";

#define EAG_DBUS_NAME_FMT		"aw.eag_%s_%d"
#define EAG_DBUS_OBJPATH_FMT	"/aw/eag_%s_%d"
#define EAG_DBUS_INTERFACE_FMT	"aw.eag_%s_%d"

#define EAG_MACADDR_LEN	6

#define INTERFACE_IP_CHECK_SUCCESS		0
#define INTERFACE_IP_CHECK_FAILURE		1

#define INTERFACE_MAC_CHECK_SUCESS		0
#define INTERFACE_MAC_CHECK_FAILURE		1

#define EAG_LOOPBACK(x)		(((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define EAG_MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define EAG_BADCLASS(x)		(((x) & htonl(0xf0000000)) == htonl(0xf0000000))
#define EAG_ZERONET(x)		(((x) & htonl(0xff000000)) == htonl(0x00000000))
#define EAG_LOCAL_MCAST(x)	(((x) & htonl(0xFFFFFF00)) == htonl(0xE0000000))

static int 
eag_u32ipaddr_check(unsigned int ipaddr)
{
	if (EAG_LOOPBACK(ipaddr)
		|| EAG_MULTICAST(ipaddr)
		|| EAG_BADCLASS(ipaddr)
		|| EAG_ZERONET(ipaddr)
		|| EAG_LOCAL_MCAST(ipaddr)) 
	{
		return -1;
	}

	return 0;
}

int 
eag_check_interface_addr(char *ifname) 
{
	struct ifreq tmp;
	int sock = -1;
	struct sockaddr_in *addr = NULL;

	if (NULL == ifname) {
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&tmp, 0, sizeof(tmp));
	strncpy(tmp.ifr_name, ifname, sizeof(tmp.ifr_name) - 1);
	if (ioctl(sock, SIOCGIFADDR, &tmp) < 0) {
		close(sock);
		sock = -1;
		return -1;
	}
	close(sock);
	sock = -1;

	addr = (struct sockaddr_in *)&tmp.ifr_addr;
	
	if (eag_u32ipaddr_check(htonl(addr->sin_addr.s_addr))) {
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

static void 
eag_dbus_path_reinit(int type,int insid)
{
	snprintf(EAG_DBUS_NAME, sizeof(EAG_DBUS_NAME),
				EAG_DBUS_NAME_FMT, 
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(EAG_DBUS_OBJPATH, sizeof(EAG_DBUS_OBJPATH),
				EAG_DBUS_OBJPATH_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(EAG_DBUS_INTERFACE, sizeof(EAG_DBUS_INTERFACE),
				EAG_DBUS_INTERFACE_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	return;
}

int
eag_set_nasip(DBusConnection *connection, 
				int hansitype, int insid,
				uint32_t nasip)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_NASIP );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,  &nasip,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

/*
int
eag_set_distributed(DBusConnection *connection, 
				int hansitype, int insid,
				int distributed)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_DISTRIBUTED );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &distributed,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}
*/

int
eag_set_rdc_distributed(DBusConnection *connection, 
				int hansitype, int insid,
				int rdc_distributed)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_RDC_DISTRIBUTED );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &rdc_distributed,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_pdc_distributed(DBusConnection *connection, 
				int hansitype, int insid,
				int pdc_distributed)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PDC_DISTRIBUTED );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &pdc_distributed,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

/*
int
eag_set_rdcpdc_ins(DBusConnection *connection, 
				int hansitype, int insid,
				int rdcpdc_slotid, int rdcpdc_insid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PDCRDC_INS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &rdcpdc_slotid,
								DBUS_TYPE_INT32,  &rdcpdc_insid,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}
*/

int
eag_set_rdc_ins(DBusConnection *connection, 
				int hansitype, int insid,
				int rdc_slotid, int rdc_insid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_RDC_INS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &rdc_slotid,
								DBUS_TYPE_INT32,  &rdc_insid,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_pdc_ins(DBusConnection *connection, 
				int hansitype, int insid,
				int pdc_slotid, int pdc_insid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PDC_INS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &pdc_slotid,
								DBUS_TYPE_INT32,  &pdc_insid,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_port(DBusConnection *connection, 
				int hansitype,int insid, uint16_t portal_port)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=-1;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PORTAL_PORT );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							  DBUS_TYPE_UINT16,  &portal_port,
							  DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_retry_params(DBusConnection *connection, 
				int hansitype, int insid,
				int retry_interval,
				int retry_times)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = -1;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PORTAL_RETRY_PARAMS);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &retry_interval,
								DBUS_TYPE_INT32, &retry_times,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_auto_session(DBusConnection *connection, 
				int hansitype, int insid,
				int auto_session)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_AUTO_SESSION);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &auto_session,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_acct_interval( DBusConnection *connection, 
				int hansitype, int insid, 					
				int acct_interval)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_RADIUS_ACCT_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &acct_interval,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_radius_retry_params( DBusConnection *connection, 
				int hansitype, int insid,					
				int retry_interval,
				int retry_times,
				int vice_retry_times)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = -1;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_RADIUS_RETRY_PARAMS );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &retry_interval,
								DBUS_TYPE_INT32, &retry_times,
								DBUS_TYPE_INT32, &vice_retry_times,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_max_redir_times( DBusConnection *connection, 
				int hansitype, int insid,
				int max_redir_times)
{
	
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	DBusMessageIter  iter;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MAX_REDIR_TIMES);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &max_redir_times,
								DBUS_TYPE_INVALID ); 
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}
 
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_force_dhcplease(DBusConnection *connection, 
				int hansitype, int insid,
				int force_dhcplease)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_FORCE_DHCPLEASE);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &force_dhcplease,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_check_errid(DBusConnection *connection, 
				int hansitype, int insid,
				int check_errid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_CHECK_ERRID);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &check_errid,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_idle_params( DBusConnection *connection, 
				int hansitype, int insid,
				unsigned long idle_timeout,
				uint64_t idle_flow)
{
	
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter  iter;
	int iRet = 0;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_IDLE_PARAMS);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&idle_timeout,
								DBUS_TYPE_UINT64, &idle_flow,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}

	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_force_wireless(DBusConnection *connection, 
				int hansitype, int insid,
				int force_wireless)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_FORCE_WIRELESS);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &force_wireless,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_flux_from( DBusConnection *connection, 
							int hansitype, int insid,
							int flux_from)
{
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter  iter;
	int iRet = 0;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_FLUX_FROM);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flux_from,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}

	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_flux_interval( DBusConnection *connection, 
							int hansitype, int insid,
							int flux_interval)
{
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter  iter;
	int iRet = 0;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_FLUX_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flux_interval,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}

	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_ipset_auth(DBusConnection *connection, 
				int hansitype, int insid,
				int ipset_auth)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_IPSET_AUTH);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &ipset_auth,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_check_nasportid(DBusConnection *connection, 
				int hansitype, int insid,
				int check_nasportid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_CHECK_NASPORTID);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &check_nasportid,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}

int eag_set_trap_onlineusernum_switch(DBusConnection *connection,
											int hansitype, int insid,
											int  switch_t)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype,insid);
	
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_TRAP_ONLINEUSERNUM_SWITCH);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &switch_t,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int eag_set_threshold_onlineusernum(DBusConnection *connection,
											int hansitype, int insid,
											int num)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype,insid);
	
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_THRESHOLD_ONLINEUSERNUM);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &num,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_octets_correct_factor( DBusConnection *connection, 
				int hansitype, int insid,
				uint32_t input_correct_factor,
				uint32_t output_correct_factor)
{
	
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter  iter;
	int iRet = 0;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_OCTETS_CORRECT_FACTOR);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &input_correct_factor,
								DBUS_TYPE_UINT32, &output_correct_factor,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}

	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_portal_protocol(DBusConnection *connection, 
				int hansitype, int insid,
				int portal_protocol)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PORTAL_PROTOCOL);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &portal_protocol,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_l2super_vlan_switch(DBusConnection *connection, 
				int hansitype, int insid,
				int l2super_vlan_switch)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_L2SUPER_VLAN_SWITCH);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &l2super_vlan_switch,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_macauth_switch(DBusConnection *connection, 
				int hansitype, int insid,
				int macauth_switch)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_SWITCH);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &macauth_switch,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_macauth_ipset_auth(DBusConnection *connection, 
				int hansitype, int insid,
				int ipset_auth)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_IPSET_AUTH);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &ipset_auth,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_macauth_flux_from( DBusConnection *connection, 
							int hansitype, int insid,
							int flux_from)
{
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter  iter;
	int iRet = 0;
	
	eag_dbus_path_reinit(hansitype,insid);
 
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_FLUX_FROM);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flux_from,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
	}

	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_macauth_flux_interval(DBusConnection *connection, 
				int hansitype, int insid,
				int flux_interval)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_FLUX_INTERVAL);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &flux_interval,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_macauth_flux_threshold(DBusConnection *connection, 
				int hansitype, int insid,
				int flux_threshold, int check_interval)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_FLUX_THRESHOLD);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flux_threshold, 
								DBUS_TYPE_INT32, &check_interval, 
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_macauth_notice_bindserver(DBusConnection *connection,
				int hansitype, int insid,
				int notice_bindserver)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_MACAUTH_NOTICE_BINDSERVER);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &notice_bindserver,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_trap_switch_abnormal_logoff(DBusConnection *connection, 
				int hansitype, int insid,
				int trap_switch)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_ERR_UNKNOWN;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_TRAP_SWITCH_ABNORMAL_LOGOFF);
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &trap_switch,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_services_status( DBusConnection *connection, 
				int hansitype, int insid,
				int status)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_SERVICE_STATUS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, 50000, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_get_base_conf( DBusConnection *connection, 
				int hansitype, int insid,
				struct eag_base_conf *baseconf)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	DBusMessageIter  iter;
	if( NULL == baseconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_BASE_CONF);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
		if( EAG_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->status));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->nasip));
			/* dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->is_distributed)); */
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->rdc_distributed));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->pdc_distributed));
			/* dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->rdcpdc_slotid));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->rdcpdc_insid)); */			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->rdc_slotid));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->rdc_insid));	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->pdc_slotid));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->pdc_insid));	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->portal_port));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->portal_retry_times));	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->portal_retry_interval));	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->auto_session));
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->radius_acct_interval));		
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->radius_retry_interval));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->radius_retry_times));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->vice_radius_retry_times));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->max_redir_times));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->force_dhcplease));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->check_errid));
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->idle_timeout));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->idle_flow));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->force_wireless));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->flux_from));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->flux_interval));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->input_correct_factor));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->output_correct_factor));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->ipset_auth));			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->check_nasportid));			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->trap_switch_abnormal_logoff));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->trap_onlineusernum_switch));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->threshold_onlineusernum));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->portal_protocol));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_switch));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_ipset_auth));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_flux_from));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_flux_interval));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_flux_threshold));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_check_interval));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->macauth_notice_bindserver));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->autelan_log));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->henan_log));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->username_check));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(baseconf->l2super_vlan));
		}
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_get_relative_time (DBusConnection *connection, 
				int hansitype, int insid, 
				unsigned long *timenow )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	DBusMessageIter  iter;
	if( NULL == timenow ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SHOW_RELATIVE_TIME );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&iRet);
		if( EAG_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, timenow);
		}
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

#if 0
int
eag_add_nasid( DBusConnection *connection, 
				int hansitype,int insid,
				struct nasid_map_t *nasidmap)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
	unsigned long keytype=0;
	unsigned long keywd_1=0;
	unsigned long keywd_2=0;
	char        *keystr="";
	char 	*nasid="";
	unsigned long conid=0;
	
	if (NULL==nasidmap){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if (nasidmap->conid > 99 || nasidmap->conid < 0){
		return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
	}
	
	switch(nasidmap->key_type){
		case NASID_KEYTYPE_WLANID:
			keywd_1 = nasidmap->key.wlanid;
			if (keywd_1>MAX_WLANID_INPUT||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_VLANID:
			keywd_1 = nasidmap->key.vlanid;
			if (keywd_1>MAX_MAPED_VLANID||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_WTPID:
			keywd_1 = nasidmap->key.wtpid;
			if (keywd_1>MAX_WTPID_INPUT||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_IPRANGE:
			if (nasidmap->key.iprange.ip_end <= 0
				||nasidmap->key.iprange.ip_begin > nasidmap->key.iprange.ip_end){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keywd_1 = nasidmap->key.iprange.ip_begin;
			keywd_2 = nasidmap->key.iprange.ip_end;
			break;
		case NASID_KEYTYPE_INTF:
			if (""==nasidmap->key.intf){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keystr = nasidmap->key.intf;
			break;
		default:
			return EAG_ERR_INPUT_PARAM_ERR;
			break;
	}
	keytype = nasidmap->key_type;
	nasid=nasidmap->nasid;
	conid=nasidmap->conid;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							DBUS_TYPE_UINT32, &keytype,
								DBUS_TYPE_UINT32, &keywd_1,
								DBUS_TYPE_UINT32, &keywd_2,
								DBUS_TYPE_STRING, &keystr,
							DBUS_TYPE_STRING, &nasid,
							DBUS_TYPE_UINT32, &conid,
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

/*
*	delete nasid only depends on key_type and key.
*/
int
eag_del_nasid( DBusConnection *connection, 
				int hansitype,int insid,
				struct nasid_map_t *nasidmap)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
	unsigned long keytype;
	unsigned long keywd_1;
	unsigned long keywd_2;
	char        *keystr="";

	if (NULL==nasidmap){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(nasidmap->key_type){
		case NASID_KEYTYPE_WLANID:
			keywd_1 = nasidmap->key.wlanid;
			keywd_1 = nasidmap->key.wlanid;
			if (keywd_1>MAX_WLANID_INPUT||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_VLANID:
			keywd_1 = nasidmap->key.vlanid;
			keywd_1 = nasidmap->key.wlanid;
			if (keywd_1>MAX_MAPED_VLANID||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_WTPID:
			keywd_1 = nasidmap->key.wtpid;
			keywd_1 = nasidmap->key.wlanid;
			if (keywd_1>MAX_WTPID_INPUT||keywd_1<=0){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_IPRANGE:
			if (0==nasidmap->key.iprange.ip_end
				||nasidmap->key.iprange.ip_begin > nasidmap->key.iprange.ip_end){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keywd_1 = nasidmap->key.iprange.ip_begin;
			keywd_2 = nasidmap->key.iprange.ip_end;
			break;
		case NASID_KEYTYPE_INTF:
			keystr = nasidmap->key.intf;
			break;
		default:
			return EAG_ERR_INPUT_PARAM_ERR;
			break;
	}
	keytype = nasidmap->key_type;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							DBUS_TYPE_UINT32, &keytype,
								DBUS_TYPE_UINT32, &keywd_1,
								DBUS_TYPE_UINT32, &keywd_2,
								DBUS_TYPE_STRING, &keystr,	//can't be NULL, dbus format.
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int 
eag_get_nasid ( DBusConnection *connection, 
				int hansitype,int insid,
				struct api_nasid_conf *nasidconf)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret = EAG_RETURN_OK;
	int num = 0;
	int i;
	char *keystr=NULL;
	char *nasid=NULL;

	if( NULL == nasidconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	memset (nasidconf, 0, sizeof(struct api_nasid_conf));
	nasidconf->current_num = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > 0 ){
				if( num > MAX_NASID_NUM ){
					num = MAX_NASID_NUM;
				}
				nasidconf->current_num= num;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].key_type));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].keywd_1));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].keywd_2));
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &keystr);
					if( ""!=keystr)
						strncpy(nasidconf->nasid_map[i].keystr, keystr, MAX_NASID_KEY_BUFF_LEN-1);
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct, &nasid);
					if( ""!=nasid)
						strncpy(nasidconf->nasid_map[i].nasid, nasid, MAX_NASID_LEN-1);
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct, &nasidconf->nasid_map[i].conid);
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	return ret;
}
#endif

 /* nasid config for wlan range */
int
eag_add_nasid( DBusConnection *connection, 
				int hansitype,int insid, struct nasid_map_t *nasidmap)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
	unsigned long keytype=0;
	unsigned long keywd_1=0;
	unsigned long keywd_2=0;
	char        *keystr="";
	char 	*nasid="";
	unsigned long conid=0;
	
	if (NULL==nasidmap){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if (nasidmap->conid > 99){
		return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
	}
	
	switch(nasidmap->key_type){
		case NASID_KEYTYPE_WLANID:
			keywd_1 = nasidmap->key.wlanidrange.id_begin;
			keywd_2 = nasidmap->key.wlanidrange.id_end;
			if (keywd_1>MAX_WLANID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WLANID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_VLANID:
			keywd_1 = nasidmap->key.vlanidrange.id_begin;
			keywd_2 = nasidmap->key.vlanidrange.id_end;
			if (keywd_1>MAX_MAPED_VLANID
				||keywd_1<=0
				||keywd_2>MAX_MAPED_VLANID
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_WTPID:
			keywd_1 = nasidmap->key.wtpidrange.id_begin;
			keywd_2 = nasidmap->key.wtpidrange.id_end;
			if (keywd_1>MAX_WTPID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WTPID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}			
			break;
		case NASID_KEYTYPE_IPRANGE:
			if (nasidmap->key.iprange.ip_end <= 0
				||nasidmap->key.iprange.ip_begin > nasidmap->key.iprange.ip_end){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keywd_1 = nasidmap->key.iprange.ip_begin;
			keywd_2 = nasidmap->key.iprange.ip_end;
			break;
		case NASID_KEYTYPE_INTF:
			if (0 == strcmp(nasidmap->key.intf, "")){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keystr = nasidmap->key.intf;
			break;
		default:
			return EAG_ERR_INPUT_PARAM_ERR;
			break;
	}
	keytype = nasidmap->key_type;
	nasid=nasidmap->nasid;
	conid=nasidmap->conid;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							DBUS_TYPE_UINT32, &keytype,
								DBUS_TYPE_UINT32, &keywd_1,
								DBUS_TYPE_UINT32, &keywd_2,
								DBUS_TYPE_STRING, &keystr,
							DBUS_TYPE_STRING, &nasid,
							DBUS_TYPE_UINT32, &conid,
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

/*
*	modify nasid only depends on key_type and key.
*/

int
eag_modify_nasid( DBusConnection *connection, 
				int hansitype,int insid, struct nasid_map_t *nasidmap)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
	unsigned long keytype=0;
	unsigned long keywd_1=0;
	unsigned long keywd_2=0;
	char        *keystr="";
	char 	*nasid="";
	unsigned long conid=0;
	
	if (NULL==nasidmap){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if (nasidmap->conid > 99){
		return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
	}
	
	switch(nasidmap->key_type){
		case NASID_KEYTYPE_WLANID:
			keywd_1 = nasidmap->key.wlanidrange.id_begin;
			keywd_2 = nasidmap->key.wlanidrange.id_end;
			if (keywd_1>MAX_WLANID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WLANID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_VLANID:
			keywd_1 = nasidmap->key.vlanidrange.id_begin;
			keywd_2 = nasidmap->key.vlanidrange.id_end;
			if (keywd_1>MAX_MAPED_VLANID
				||keywd_1<=0
				||keywd_2>MAX_MAPED_VLANID
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_WTPID:
			keywd_1 = nasidmap->key.wtpidrange.id_begin;
			keywd_2 = nasidmap->key.wtpidrange.id_end;
			if (keywd_1>MAX_WTPID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WTPID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}			
			break;
		case NASID_KEYTYPE_IPRANGE:
			if (nasidmap->key.iprange.ip_end <= 0
				||nasidmap->key.iprange.ip_begin > nasidmap->key.iprange.ip_end){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keywd_1 = nasidmap->key.iprange.ip_begin;
			keywd_2 = nasidmap->key.iprange.ip_end;
			break;
		case NASID_KEYTYPE_INTF:
			if (0 == strcmp(nasidmap->key.intf, "")){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keystr = nasidmap->key.intf;
			break;
		default:
			return EAG_ERR_INPUT_PARAM_ERR;
			break;
	}
	keytype = nasidmap->key_type;
	nasid=nasidmap->nasid;
	conid=nasidmap->conid;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_MODIFY_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							DBUS_TYPE_UINT32, &keytype,
							DBUS_TYPE_UINT32, &keywd_1,
							DBUS_TYPE_UINT32, &keywd_2,
							DBUS_TYPE_STRING, &keystr,
							DBUS_TYPE_STRING, &nasid,
							DBUS_TYPE_UINT32, &conid,
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

/*
*	delete nasid only depends on key_type and key.
*/

int
eag_del_nasid( DBusConnection *connection, 
				int hansitype,int insid, struct nasid_map_t *nasidmap)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
	unsigned long keytype;
	unsigned long keywd_1;
	unsigned long keywd_2;
	char        *keystr="";

	if (NULL==nasidmap){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(nasidmap->key_type){
		case NASID_KEYTYPE_WLANID:
			keywd_1 = nasidmap->key.wlanidrange.id_begin;
			keywd_2 = nasidmap->key.wlanidrange.id_end;
			if (keywd_1>MAX_WLANID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WLANID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_VLANID:
			keywd_1 = nasidmap->key.vlanidrange.id_begin;
			keywd_2 = nasidmap->key.vlanidrange.id_end;
			if (keywd_1>MAX_MAPED_VLANID
				||keywd_1<=0
				||keywd_2>MAX_MAPED_VLANID
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			break;
		case NASID_KEYTYPE_WTPID:
			keywd_1 = nasidmap->key.wtpidrange.id_begin;
			keywd_2 = nasidmap->key.wtpidrange.id_end;
			if (keywd_1>MAX_WTPID_INPUT
				||keywd_1<=0
				||keywd_2>MAX_WTPID_INPUT
				||keywd_2<=0
				||keywd_1>keywd_2) {
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}			
			break;
		case NASID_KEYTYPE_IPRANGE:
			if (0==nasidmap->key.iprange.ip_end
				||nasidmap->key.iprange.ip_begin > nasidmap->key.iprange.ip_end){
				return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
			}
			keywd_1 = nasidmap->key.iprange.ip_begin;
			keywd_2 = nasidmap->key.iprange.ip_end;
			break;
		case NASID_KEYTYPE_INTF:
			keystr = nasidmap->key.intf;
			break;
		default:
			return EAG_ERR_INPUT_PARAM_ERR;
			break;
	}
	keytype = nasidmap->key_type;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
							DBUS_TYPE_UINT32, &keytype,
								DBUS_TYPE_UINT32, &keywd_1,
								DBUS_TYPE_UINT32, &keywd_2,
								DBUS_TYPE_STRING, &keystr,	//can't be NULL, dbus format.
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}


int 
eag_get_nasid ( DBusConnection *connection, 
				int hansitype,int insid, struct api_nasid_conf *nasidconf)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret = EAG_RETURN_OK;
	int num = 0;
	int i;
	char *keystr=NULL;
	char *nasid=NULL;

	if( NULL == nasidconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	memset (nasidconf, 0, sizeof(struct api_nasid_conf));
	nasidconf->current_num = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_NASID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > 0 ){
				if( num > MAX_NASID_NUM ){
					num = MAX_NASID_NUM;
				}
				nasidconf->current_num= num;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].key_type));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].keywd_1));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(nasidconf->nasid_map[i].keywd_2));
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &keystr);
					if(0 != strcmp(keystr, ""))
						strncpy(nasidconf->nasid_map[i].keystr, keystr, MAX_NASID_KEY_BUFF_LEN-1);
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct, &nasid);
					if(0 != strcmp(nasid, ""))
						strncpy(nasidconf->nasid_map[i].nasid, nasid, MAX_NASID_LEN-1);
					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct, &nasidconf->nasid_map[i].conid);
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	return ret;
}

int
eag_add_nasportid( DBusConnection *connection, 
				int hansitype,int insid, 
				struct eag_id_range_t wlanid,
				struct eag_id_range_t wtpid,
				struct eag_id_range_t vlanid,
				unsigned long key_type,
				unsigned long nasportid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = EAG_RETURN_OK;
#if 0	
	if(wlanid.begin <= 0
		|| wlanid.begin > MAX_WLANID_INPUT 
		|| wlanid.end <= 0
		|| wlanid.end > MAX_WLANID_INPUT
		|| wlanid.end < wlanid.begin
		|| wtpid.begin <= 0
		|| wtpid.begin > MAX_WTPID_INPUT
		|| wtpid.end <= 0
		|| wtpid.end > MAX_WTPID_INPUT
		|| wtpid.end < wtpid.begin 
		|| vlanid.begin <= 0
		|| vlanid.begin > MAX_VLANID_INPUT 
		|| vlanid.end <= 0
		|| vlanid.end > MAX_VLANID_INPUT
		|| vlanid.end < vlanid.begin
		|| nasportid > MAX_MAPED_VLANID){
		return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
	}
#endif	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_NASPORTID );
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &wlanid.begin,
							DBUS_TYPE_UINT32, &wlanid.end,
							DBUS_TYPE_UINT32, &wtpid.begin,
							DBUS_TYPE_UINT32, &wtpid.end,
							DBUS_TYPE_UINT32, &vlanid.begin,
							DBUS_TYPE_UINT32, &vlanid.end,
							DBUS_TYPE_UINT32, &nasportid,
							DBUS_TYPE_UINT32, &key_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_del_nasportid( DBusConnection *connection, 
				int hansitype,int insid, 
				struct eag_id_range_t wlanid,
				struct eag_id_range_t wtpid,
				struct eag_id_range_t vlanid,
				unsigned long key_type,
				unsigned long nasportid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=EAG_RETURN_OK;
#if 0	
	if(wlanid.begin <= 0
		|| wlanid.begin > MAX_WLANID_INPUT 
		|| wlanid.end <= 0
		|| wlanid.end > MAX_WLANID_INPUT
		|| wlanid.end < wlanid.begin
		|| wtpid.begin <= 0
		|| wtpid.begin > MAX_WTPID_INPUT
		|| wtpid.end <= 0
		|| wtpid.end > MAX_WTPID_INPUT
		|| wtpid.end < wtpid.begin 
		|| vlanid.begin <= 0
		|| vlanid.begin > MAX_VLANID_INPUT 
		|| vlanid.end <= 0
		|| vlanid.end > MAX_VLANID_INPUT
		|| vlanid.end < vlanid.begin
		|| nasportid > MAX_MAPED_VLANID){
		return EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE;
	}
#endif	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_NASPORTID );
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &wlanid.begin,
							DBUS_TYPE_UINT32, &wlanid.end,
							DBUS_TYPE_UINT32, &wtpid.begin,
							DBUS_TYPE_UINT32, &wtpid.end,
							DBUS_TYPE_UINT32, &vlanid.begin,
							DBUS_TYPE_UINT32, &vlanid.end,
							DBUS_TYPE_UINT32, &nasportid,
							DBUS_TYPE_UINT32, &key_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block ( 
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_get_nasportid ( DBusConnection *connection, 
				int hansitype,int insid,
				struct nasportid_conf *nasportid)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret = EAG_RETURN_OK;
	int num = 0;
	int i;

	if( NULL == nasportid ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	memset (nasportid, 0, sizeof(struct nasportid_conf));
	nasportid->current_num = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_NASPORTID );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
#if 1		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > 0 ){
				if( num > MAX_NASPORTID_NUM ){
					num = MAX_NASPORTID_NUM;
				}
				nasportid->current_num= num;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for (i = 0; i < num; i++) {
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&nasportid->nasportid_map[i].key_type);					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.wlan_wtp.wlanid_begin);
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.wlan_wtp.wlanid_end);
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.wlan_wtp.wtpid_begin);					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.wlan_wtp.wtpid_end);
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.vlan.vlanid_begin);
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].key.vlan.vlanid_end);
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &nasportid->nasportid_map[i].nasportid);
					dbus_message_iter_next(&iter_array);		
				}
			}
		}
#endif		
	}
	
	dbus_message_unref(reply);
	return ret;
}

int
eag_add_captive_intf( DBusConnection *connection, 
				int hansitype, int insid,
				char *intfs )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_CAPTIVE_INTF );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &intfs,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_del_captive_intf(DBusConnection *connection, 
				int hansitype, int insid, 
				char *intfs)
{
	
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_CAPTIVE_INTF );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &intfs,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

int
eag_get_captive_intfs(DBusConnection *connection, 
				int hansitype, int insid, 
				eag_captive_intfs *captive_intfs)
{
	int ret = -1, i = 0;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	
	unsigned long num = 0;
	char *intfs = NULL;
		
	if( NULL == captive_intfs ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SHOW_CAPTIVE_INTFS );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > CP_MAX_INTERFACE_NUM ){
				num = CP_MAX_INTERFACE_NUM;
			}
			captive_intfs->curr_ifnum = num;
			if( num > 0 ){
				for( i=0; i<num; i++ ){
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter, &intfs);
					strncpy(captive_intfs->cpif[i], intfs, MAX_IF_NAME_LEN-1);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	return ret; 
}

int
eag_conf_captive_list(DBusConnection *connection, 
				int hansitype, int insid,
				RULE_TYPE type_tmp, char *iprange, char *portset, char * domain, char * intfs,
				char *add_or_del, char *white_or_black)

{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	int type = (int)type_tmp;
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_CONF_CAPTIVE_LIST );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &type,
								DBUS_TYPE_STRING, &iprange,
								DBUS_TYPE_STRING, &portset,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_STRING, &intfs,
								DBUS_TYPE_STRING, &add_or_del,
								DBUS_TYPE_STRING, &white_or_black,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_show_white_list(DBusConnection *connection, 
				int hansitype, int insid,
				struct bw_rules *white)

{	
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = 0, i = 0, num = 0;
	struct bw_rule_t *rule= NULL;
	char * domain = NULL;
	char * ports = NULL;
	char * intf = NULL;
	int type = -1;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SHOW_WHITE_LIST );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > MAX_BW_RULES_NUM ){
				num = MAX_BW_RULES_NUM;
			}
			white->curr_num = num;
			rule = white->rule;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){

					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &type);
					if((RULE_TYPE)type == RULE_DOMAIN)
						rule[i].type = RULE_DOMAIN;
					else
						rule[i].type = RULE_IPADDR;

					switch(rule[i].type){						
						case RULE_IPADDR:	
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &(rule[i].key.ip.ipbegin));
					
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &(rule[i].key.ip.ipend));

							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &ports);					
							strncpy(rule[i].key.ip.ports, ports, CP_MAX_PORTS_BUFF_LEN-1);

							dbus_message_iter_next(&iter_struct);
							break;
						case RULE_DOMAIN:
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);		
							dbus_message_iter_get_basic(&iter_struct, &domain);
							strncpy(rule[i].key.domain.name, domain, CP_MAX_BW_DOMAIN_NAME_LEN-1);
							break;
						}
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &intf);
					strncpy(rule[i].intf, intf, MAX_IF_NAME_LEN-1);

					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return ret;

}

int
eag_show_black_list(DBusConnection *connection, 
				int hansitype, int insid,
				struct bw_rules *black)

{	
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int ret = 0, i = 0, num = 0;
	struct bw_rule_t *rule= NULL;
	char * domain = NULL;
	char * ports = NULL;
	char * intf = NULL;
	int type = -1;
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SHOW_BLACK_LIST );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > MAX_BW_RULES_NUM ){
				num = MAX_BW_RULES_NUM;
			}
			black->curr_num = num;
			rule = black->rule;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){

					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &type);
					if((RULE_TYPE)type == RULE_DOMAIN)
						rule[i].type = RULE_DOMAIN;
					else
						rule[i].type = RULE_IPADDR;

					switch(rule[i].type){						
						case RULE_IPADDR:	
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &(rule[i].key.ip.ipbegin));
					
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &(rule[i].key.ip.ipend));

							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct, &ports);					
							strncpy(rule[i].key.ip.ports, ports, CP_MAX_PORTS_BUFF_LEN-1);

							dbus_message_iter_next(&iter_struct);
							break;
						case RULE_DOMAIN:
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_next(&iter_struct);		
							dbus_message_iter_get_basic(&iter_struct, &domain);
							strncpy(rule[i].key.domain.name, domain, CP_MAX_BW_DOMAIN_NAME_LEN-1);
							break;
						}
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &intf);
					strncpy(rule[i].intf, intf, MAX_IF_NAME_LEN-1);

					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return ret;
}

int
eag_add_portal_server( DBusConnection *connection, 
				int hansitype, int insid, 					
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				const char *keystr,
				const char *portal_url, 
				uint16_t ntf_port,
				const char *domain,
				uint32_t mac_server_ip,
				uint16_t mac_server_port)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if( NULL == portal_url ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		keystr = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_PORTAL );
	dbus_error_init(&err);

	if( NULL == domain ){
		domain = "";
	}
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &keystr,
								DBUS_TYPE_STRING, &portal_url,
								DBUS_TYPE_UINT16, &ntf_port,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_UINT32, &mac_server_ip,
								DBUS_TYPE_UINT16, &mac_server_port,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_modify_portal_server( DBusConnection *connection, 
				int hansitype, int insid, 					
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				const char *keystr,
				const char *portal_url, 
				unsigned short ntf_port,
				const char *domain,
				uint32_t mac_server_ip,
				uint16_t mac_server_port)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if( NULL == portal_url ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		keystr = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_MODIFY_PORTAL );
	dbus_error_init(&err);

	if( NULL == domain ){
		domain = "";
	}
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &keystr,
								DBUS_TYPE_STRING, &portal_url,
								DBUS_TYPE_UINT16, &ntf_port,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_UINT32, &mac_server_ip,
								DBUS_TYPE_UINT16, &mac_server_port,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_del_portal_server( DBusConnection *connection, 
				int hansitype, int insid, 					
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *keystr)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = -1;

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		keystr = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_PORTAL );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &keystr,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_get_portal_conf( DBusConnection *connection, 
				int hansitype, int insid, 					
				struct portal_conf *portalconf)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret = -1;
	unsigned long num = 0;
	int i;
	char *key = NULL;
	char *portal_url=NULL;
	char *domain=NULL;
	char *acname = NULL;
	int acip_to_url = 0;
	int usermac_to_url = 0;
	int clientmac_to_url = 0;
	int apmac_to_url = 0;
	int wlan_to_url = 0;
	int redirect_to_url = 0;
	int nasid_to_url = 0;
	int wlanparameter = 0;
	char *DESkey = NULL;
	int wlanuserfirsturl = 0;
	char *url_suffix = NULL;
	char *secret = NULL;
	int wlanapmac = 0;
	int wlanusermac = 0;
	char *wlanusermac_DESkey = NULL;
	int wisprlogin = 0;

	if( NULL == portalconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_PORTAL_CONF );
	dbus_error_init(&err);
	
	dbus_message_append_args( query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > MAX_PORTAL_NUM ){
				num = MAX_PORTAL_NUM;
			}
			portalconf->current_num= num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){

					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &portalconf->portal_srv[i].key_type);					
					switch(portalconf->portal_srv[i].key_type){						
					case PORTAL_KEYTYPE_ESSID:
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct, &key);
						if( NULL!=key )
							strncpy(portalconf->portal_srv[i].key.essid, key, MAX_PORTAL_KEY_BUFF_LEN-1);
						break;
					case PORTAL_KEYTYPE_WLANID:
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct, &key);
						if(NULL != key)
							portalconf->portal_srv[i].key.wlanid = strtoul(key, NULL, 0);
						break;
					case PORTAL_KEYTYPE_VLANID:
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct, &key);
						if(NULL != key)
							portalconf->portal_srv[i].key.vlanid = strtoul(key, NULL, 0);
						break;
					case PORTAL_KEYTYPE_WTPID:
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct, &key);
						if(NULL != key)
							portalconf->portal_srv[i].key.wtpid = strtoul(key, NULL, 0);
						break;
					case PORTAL_KEYTYPE_INTF:
						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct, &key);
						if( NULL!=key )
							strncpy(portalconf->portal_srv[i].key.intf, key, MAX_PORTAL_KEY_BUFF_LEN-1);
						break;
					default:
						dbus_message_iter_next(&iter_struct);
						break;
					}
					portal_url = portalconf->portal_srv[i].portal_url;
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &portal_url);
					if( NULL!=portal_url)
						strncpy(portalconf->portal_srv[i].portal_url, portal_url, MAX_PORTAL_URL_LEN-1);
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &portalconf->portal_srv[i].ntf_port);
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct, &domain);
					if( NULL!=domain)
						strncpy(portalconf->portal_srv[i].domain, domain, MAX_RADIUS_DOMAIN_LEN-1);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &portalconf->portal_srv[i].ip);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &acname);
					if( NULL != acname)
						strncpy(portalconf->portal_srv[i].acname, acname, MAX_MULTPORTAL_ACNAME_LEN-1);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &portalconf->portal_srv[i].mac_server_ip);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &portalconf->portal_srv[i].mac_server_port);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &acip_to_url);
					portalconf->portal_srv[i].acip_to_url = acip_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &usermac_to_url);
					portalconf->portal_srv[i].usermac_to_url = usermac_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &clientmac_to_url);
					portalconf->portal_srv[i].clientmac_to_url = clientmac_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &apmac_to_url);
					portalconf->portal_srv[i].apmac_to_url = apmac_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &wlan_to_url);
					portalconf->portal_srv[i].wlan_to_url = wlan_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &redirect_to_url);
					portalconf->portal_srv[i].redirect_to_url = redirect_to_url;
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &nasid_to_url);
					portalconf->portal_srv[i].nasid_to_url = nasid_to_url;
					dbus_message_iter_next(&iter_struct);		
					/*wlanparameter*/
					dbus_message_iter_get_basic(&iter_struct, &wlanparameter);
					portalconf->portal_srv[i].wlanparameter = wlanparameter;
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct, &DESkey );/*DES key*/
					if (1==wlanparameter 
						&& NULL!=DESkey 
						&& strlen(DESkey)>0 
						&& strlen(DESkey)<=MAX_DES_KEY_LEN) 
					{
						memset( portalconf->portal_srv[i].deskey, 0, 
										sizeof(portalconf->portal_srv[i].deskey) );
						memcpy( portalconf->portal_srv[i].deskey, DESkey,strlen(DESkey));
					}
					dbus_message_iter_next(&iter_struct);
					/*wlanparameter end*/
					dbus_message_iter_get_basic(&iter_struct, &wlanuserfirsturl);
					portalconf->portal_srv[i].wlanuserfirsturl = wlanuserfirsturl;
					dbus_message_iter_next(&iter_struct);
					/*wlanuserfirsturl end*/
					dbus_message_iter_get_basic(&iter_struct, &url_suffix);
					if( NULL != url_suffix)
						strncpy(portalconf->portal_srv[i].url_suffix, url_suffix, MAX_PORTAL_URL_SUFFIX_LEN-1);
					dbus_message_iter_next(&iter_struct);					
					dbus_message_iter_get_basic(&iter_struct, &secret);
					if( NULL != secret)
						strncpy(portalconf->portal_srv[i].secret, secret, PORTAL_SECRETSIZE-1);
					dbus_message_iter_next(&iter_struct);
					/* wlanapmac */
					dbus_message_iter_get_basic(&iter_struct, &wlanapmac);
					portalconf->portal_srv[i].wlanapmac = wlanapmac;
					dbus_message_iter_next(&iter_struct);
					/* wlanusermac */
					dbus_message_iter_get_basic(&iter_struct, &wlanusermac);
					portalconf->portal_srv[i].wlanusermac = wlanusermac;
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct, &wlanusermac_DESkey );/*DES key*/
					if (1==wlanusermac 
						&& NULL!=wlanusermac_DESkey 
						&& strlen(wlanusermac_DESkey)>0 
						&& strlen(wlanusermac_DESkey)<=MAX_DES_KEY_LEN) 
					{
						memset( portalconf->portal_srv[i].wlanusermac_deskey, 0, 
								sizeof(portalconf->portal_srv[i].wlanusermac_deskey) );
						memcpy( portalconf->portal_srv[i].wlanusermac_deskey, 
								wlanusermac_DESkey,strlen(wlanusermac_DESkey));
					}
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct, &wisprlogin);
					portalconf->portal_srv[i].wisprlogin = wisprlogin;
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	return ret; 
}

int
eag_set_portal_server_acname( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				const char *acname )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == acname ){
		acname = "";
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_ACNAME );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_STRING, &acname,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_acip_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int acip_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_ACIP_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &acip_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_nasid_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int nasid_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_NASID_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &nasid_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_wlanparameter( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int status,
				char *deskey)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}

	if (NULL!=deskey && strlen(deskey)>MAX_DES_KEY_LEN) {
		return EAG_ERR_PORTAL_WLANPARAMTER_DESKEY_LEN_LIMITE;
	}
	if (NULL==deskey || strlen(deskey)==0) {
		deskey = "";
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WLANPARAMETER );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_STRING, &deskey,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args(  reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}


int
eag_set_portal_server_wlanuserfirsturl( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int wlanuserfirsturl_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WLANUSERFIRSTURL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &wlanuserfirsturl_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}




int
eag_set_portal_server_url_suffix( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				const char *url_suffix )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == url_suffix ){
		url_suffix = "";
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_URL_SUFFIX );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_STRING, &url_suffix,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_secret( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				const char *key_word,
				const char *secret )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == secret ){
		secret = "";
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PORTAL_SECRET);
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_STRING, &secret,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_wlanapmac( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int wlanapmac_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WLANAPMAC);
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &wlanapmac_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_usermac_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int usermac_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_USERMAC_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &usermac_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_clientmac_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int clientmac_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_CLIENTMAC_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &clientmac_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_apmac_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int apmac_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_APMAC_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &apmac_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_wlan_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int wlan_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WLAN_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &wlan_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_redirect_to_url( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int redirect_to_url )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_REDIRECT_TO_URL );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &redirect_to_url,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_portal_server_wlanusermac( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int status,
				char *deskey)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}

	if (NULL!=deskey && strlen(deskey)>MAX_DES_KEY_LEN) {
		return EAG_ERR_PORTAL_WLANPARAMTER_DESKEY_LEN_LIMITE;
	}
	if (NULL==deskey || strlen(deskey)==0) {
		deskey = "";
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WLANUSERMAC);
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_STRING, &deskey,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args(  reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}


int
eag_set_portal_server_wisprlogin( DBusConnection *connection, 
				int hansitype, int insid, 	
				PORTAL_KEY_TYPE key_type,
				unsigned long keyid,
				char *key_word,
				int status,
				char *type)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = 0;

	if( NULL == key_word ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	switch(key_type){
	case PORTAL_KEYTYPE_ESSID:
	case PORTAL_KEYTYPE_INTF:
		keyid = 0;
		break;
	case PORTAL_KEYTYPE_WLANID:
	case PORTAL_KEYTYPE_VLANID:
	case PORTAL_KEYTYPE_WTPID:
		key_word = "";
		break;
	default:
		return EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE;
	}
	
	if( status ){
		if (type &&(0 == strcmp(type, "http"))){
			status = WISPR_URL_HTTP;
		}else if (type &&(0 == strcmp(type, "https"))){
			status = WISPR_URL_HTTPS;
		}else{
			return EAG_ERR_PORTAL_SET_WISPR_URL_TYPE_ERR;
		}
	}else{
		status = WISPR_URL_NO;
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_WISPRLOGIN);
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &key_type,
								DBUS_TYPE_UINT32, &keyid,
								DBUS_TYPE_STRING, &key_word,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		
		dbus_message_get_args(  reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}


int
eag_add_radius( DBusConnection *connection, 
				int hansitype, int insid, 					
				char *domain,
				uint32_t auth_ip,
				uint16_t auth_port,
				char *auth_secret,
				uint32_t acct_ip,
				uint16_t acct_port,
				char *acct_secret,
				uint32_t backup_auth_ip,
				uint16_t backup_auth_port,
				char *backup_auth_secret,
				uint32_t backup_acct_ip,
				uint16_t backup_acct_port,
				char *backup_acct_secret )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if( strlen(domain)==0||strlen(domain)>MAX_RADIUS_DOMAIN_LEN-1 ){
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}
	if( (0 == auth_ip || 0 == auth_port)
		||(0 == acct_ip || 0 == acct_port)){
		return EAG_ERR_RADIUS_PARAM_ERR;
	}
	if((NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1)
		||(NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1)){
		return EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE;
	}
	if( NULL == backup_auth_secret ){
		backup_auth_secret = "";
	}

	if( NULL == backup_acct_secret ){
		backup_acct_secret = "";
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_RADIUS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_UINT32, &auth_ip,
								DBUS_TYPE_UINT16, &auth_port,
								DBUS_TYPE_STRING, &auth_secret,
								DBUS_TYPE_UINT32, &acct_ip,
								DBUS_TYPE_UINT16, &acct_port,
								DBUS_TYPE_STRING, &acct_secret,
								DBUS_TYPE_UINT32, &backup_auth_ip,
								DBUS_TYPE_UINT16, &backup_auth_port,
								DBUS_TYPE_STRING, &backup_auth_secret,
								DBUS_TYPE_UINT32, &backup_acct_ip,
								DBUS_TYPE_UINT16, &backup_acct_port,
								DBUS_TYPE_STRING, &backup_acct_secret,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_modify_radius( DBusConnection *connection, 
				int hansitype, int insid, 					
				char *domain,
				uint32_t auth_ip,
				uint16_t auth_port,
				char *auth_secret,
				uint32_t acct_ip,
				uint16_t acct_port,
				char *acct_secret,
				uint32_t backup_auth_ip,
				uint16_t backup_auth_port,
				char *backup_auth_secret,
				uint32_t backup_acct_ip,
				uint16_t backup_acct_port,
				char *backup_acct_secret )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if( strlen(domain)==0||strlen(domain)>MAX_RADIUS_DOMAIN_LEN-1 ){
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}
	if( (0 == auth_ip || 0 == auth_port)
		||(0 == acct_ip || 0 == acct_port)){
		return EAG_ERR_RADIUS_PARAM_ERR;
	}
	if((NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1)
		||(NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1)){
		return EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE;
	}
	if( NULL == backup_auth_secret ){
		backup_auth_secret = "";
	}

	if( NULL == backup_acct_secret ){
		backup_acct_secret = "";
	}
	
	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_MODIFY_RADIUS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_UINT32, &auth_ip,
								DBUS_TYPE_UINT16, &auth_port,
								DBUS_TYPE_STRING, &auth_secret,
								DBUS_TYPE_UINT32, &acct_ip,
								DBUS_TYPE_UINT16, &acct_port,
								DBUS_TYPE_STRING, &acct_secret,
								DBUS_TYPE_UINT32, &backup_auth_ip,
								DBUS_TYPE_UINT16, &backup_auth_port,
								DBUS_TYPE_STRING, &backup_auth_secret,
								DBUS_TYPE_UINT32, &backup_acct_ip,
								DBUS_TYPE_UINT16, &backup_acct_port,
								DBUS_TYPE_STRING, &backup_acct_secret,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_del_radius( DBusConnection *connection, 
				int hansitype, int insid, 
				const char *domain )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if( strlen(domain)==0||strlen(domain)>MAX_RADIUS_DOMAIN_LEN-1 ){
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_RADIUS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_remove_domain_switch(DBusConnection *connection, 
				int hansitype, int insid, 
				char *domain,
				int remove_domain_switch)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if(strlen(domain) == 0 || strlen(domain) > MAX_RADIUS_DOMAIN_LEN-1) {
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_REMOVE_DOMAIN_SWITCH);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_INT32, &remove_domain_switch,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_class_to_bandwidth_switch(DBusConnection *connection, 
				int hansitype, int insid,
				char *domain,
				int class_to_bandwidth)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if(strlen(domain) == 0 || strlen(domain) > MAX_RADIUS_DOMAIN_LEN-1) {
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_CLASS_TO_BANDWIDTH_SWITCH);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_INT32, &class_to_bandwidth,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_get_radius_conf(DBusConnection *connection, 
				int hansitype, int insid, 
				char *domain,
				struct radius_conf *radiusconf )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret=0;
	unsigned long num = 0;
	int i;
	
	char *auth_secret=NULL;
	char *acct_secret=NULL;
	char *backup_auth_secret=NULL;
	char *backup_acct_secret=NULL;


	if( NULL == radiusconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( NULL == domain ){
		domain = "";
	}

	if( strlen(domain)>MAX_RADIUS_DOMAIN_LEN-1 ){
		return EAG_ERR_RADIUS_DOMAIN_LEN_ERR;
	}

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_RADIUS_CONF );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);

			if( num > MAX_RADIUS_SRV_NUM ){
				num = MAX_RADIUS_SRV_NUM;
			}
			radiusconf->current_num = num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			
			
				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					/*domain*/
					dbus_message_iter_get_basic(&iter_struct, &domain );

					if( NULL != domain ){
						strncpy( radiusconf->radius_srv[i].domain, 
										domain, MAX_RADIUS_DOMAIN_LEN-1);
					}
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].remove_domain_name));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].class_to_bandwidth));
					dbus_message_iter_next(&iter_struct);
					
					/*auth*/
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].auth_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].auth_port));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&auth_secret);
#if 1
					if( NULL != auth_secret ){
						strncpy(radiusconf->radius_srv[i].auth_secret, 
								auth_secret, RADIUS_SECRETSIZE-1);
					}
#endif
			
					/*acct*/
					dbus_message_iter_next(&iter_struct);		
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].acct_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].acct_port));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&acct_secret);
#if 1
					if( NULL != acct_secret ){
						strncpy(radiusconf->radius_srv[i].acct_secret, 
								acct_secret, RADIUS_SECRETSIZE-1);	
					}
#endif				
					/*backup auth*/
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].backup_auth_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].backup_auth_port));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&backup_auth_secret);
#if 1
					if( NULL != backup_auth_secret ){
						strncpy(radiusconf->radius_srv[i].backup_auth_secret, 
												backup_auth_secret, RADIUS_SECRETSIZE-1);				
					}
#endif
					/*backup acct*/
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].backup_acct_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].backup_acct_port));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&backup_acct_secret);
#if 1
					if( NULL != backup_acct_secret ){
						strncpy(radiusconf->radius_srv[i].backup_acct_secret, 
												backup_acct_secret, RADIUS_SECRETSIZE-1);
					}
#endif
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return ret;	
}

int
eag_get_bss_statistics (DBusConnection *connection, 
				int hansitype, int insid, 
				struct list_head *bss_stat)
{
	DBusMessage *query;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	int iRet = 0;
	
	int i = 0;
	unsigned int bss_num;
	struct eag_bss_stat *tmp = NULL;
	
	if (NULL == bss_stat)
		return EAG_ERR_INPUT_PARAM_ERR;

    INIT_LIST_HEAD(bss_stat);

	eag_dbus_path_reinit(hansitype, insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_BSS_STATISTICS );
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet );
		
	}

	if (0 != iRet)
		goto error;
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &bss_num);
	if (0 == bss_num) {
		goto error;
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter, &iter_array);
	
	for(i = 0; i < bss_num; i++) {
		tmp = (struct eag_bss_stat *)malloc(sizeof(struct eag_bss_stat));
		if (tmp == NULL) {
			iRet = EAG_ERR_MALLOC_FAILED;
			eag_free_bss_statistics(bss_stat);
			goto error;
		}
		memset (tmp,0,sizeof(struct eag_bss_stat));
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[0]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[1]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[2]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[3]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[4]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[5]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tmp->wlanid);				
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->radioid));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(tmp->online_user_num));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->user_connect_total_time));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_online_user_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_user_connect_total_time));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->http_redir_request_count));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->http_redir_success_count));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_req_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_0_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_1_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_2_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_req_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_0_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_1_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_2_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_req_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_0_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_1_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_2_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->normal_logoff_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->abnormal_logoff_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_abnormal_logoff_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_timeout_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_busy_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->req_auth_password_missing_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->req_auth_unknown_type_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ack_auth_busy_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_disorder_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_retry_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_timeout_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_accept_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_reject_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_start_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_start_retry_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_start_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_update_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_update_retry_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_update_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_stop_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_stop_retry_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_stop_count));
			dbus_message_iter_next(&iter_struct);
		dbus_message_iter_next(&iter_array);
		list_add_tail(&tmp->node, bss_stat);
	}
error:
	dbus_message_unref(reply);
	return iRet;
}


int 
eag_free_bss_statistics (struct list_head *bss_stat)
{
	struct eag_bss_stat *pos = NULL;
	struct eag_bss_stat *n = NULL;

	if ( NULL == bss_stat )
		return EAG_ERR_INPUT_PARAM_ERR;


	list_for_each_entry_safe (pos, n, bss_stat, node) {
		free(pos);
	}

	bss_stat = NULL;
	return EAG_RETURN_OK;
}


int
eag_get_ap_statistics (DBusConnection *connection, 
				int hansitype, int insid, 
				struct list_head *ap_stat)
{
	DBusMessage *query;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusMessageIter iter_struct;
	DBusError err;
	int iRet = 0;

	int i = 0;
	unsigned int ap_num = 0;
	struct eag_ap_stat *tmp = NULL;
	
	if (NULL == ap_stat)
		return EAG_ERR_INPUT_PARAM_ERR;

	INIT_LIST_HEAD(ap_stat);

	eag_dbus_path_reinit(hansitype, insid);

	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_AP_STATISTICS);
	dbus_error_init(&err);
	dbus_message_append_args(query, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply, &iter);
		dbus_message_iter_get_basic(&iter, &iRet);
	}

	if (0 != iRet)
		goto error;
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &ap_num);
	if (0 == ap_num) {
		goto error;
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter, &iter_array);

	for (i = 0; i < ap_num; i++) {
		tmp = (struct eag_ap_stat *)malloc(sizeof(struct eag_ap_stat));
		if (tmp == NULL) {
			iRet = EAG_ERR_MALLOC_FAILED;
			eag_free_ap_statistics(ap_stat);
			goto error;
		}
		memset (tmp, 0, sizeof(struct eag_ap_stat));

		dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[0]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[1]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[2]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[3]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[4]));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ap_mac[5]));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->online_user_num));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->user_connect_total_time));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_online_user_num));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_user_connect_total_time));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->http_redir_request_count));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->http_redir_success_count));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_req_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_0_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_1_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_2_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_req_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_0_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_1_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_2_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_req_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_0_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_1_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_2_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_3_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_ack_4_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->normal_logoff_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->abnormal_logoff_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->macauth_abnormal_logoff_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_timeout_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->challenge_busy_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->req_auth_password_missing_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->req_auth_unknown_type_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->ack_auth_busy_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth_disorder_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_retry_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_request_timeout_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_accept_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->access_reject_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_start_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_start_retry_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_start_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_update_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_update_retry_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_update_count)); 
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_stop_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_request_stop_retry_count));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct_response_stop_count));
			dbus_message_iter_next(&iter_struct);
		dbus_message_iter_next(&iter_array);
		list_add_tail(&tmp->node, ap_stat);
	}

error:
	dbus_message_unref(reply);
	return iRet;
}


int
eag_free_ap_statistics (struct list_head *ap_stat)
{
	struct eag_ap_stat *pos = NULL;
	struct eag_ap_stat *n = NULL;

	if ( NULL == ap_stat )
		return EAG_ERR_INPUT_PARAM_ERR;

	list_for_each_entry_safe(pos, n, ap_stat, node) {
		free(pos);
	}

	ap_stat = NULL;
	return EAG_RETURN_OK;
}

int
eag_get_eag_statistics (DBusConnection *connection, 
				int hansitype, int insid, 
				struct eag_all_stat *eag_stat)
{
	DBusMessage *query;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusError err;
	int iRet = 0;
	
	if (NULL == eag_stat)
		return EAG_ERR_INPUT_PARAM_ERR;

	memset(eag_stat, 0, sizeof(struct eag_all_stat));

	eag_dbus_path_reinit(hansitype, insid);

	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_GET_EAG_STATISTICS);
	dbus_error_init(&err);
	dbus_message_append_args(query, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply, &iter);
		dbus_message_iter_get_basic(&iter, &iRet);
	}

	if (0 != iRet)
		goto error;
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter, &(eag_stat->ap_num));
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->online_user_num));	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->user_connect_total_time));	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_online_user_num));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_user_connect_total_time));	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->http_redir_request_count));	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->http_redir_success_count));	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_req_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_ack_0_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_ack_1_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_ack_2_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_ack_3_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_ack_4_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_req_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_ack_0_count)); 
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_ack_1_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_ack_2_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_ack_3_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_ack_4_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_req_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_ack_0_count)); 
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_ack_1_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_ack_2_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_ack_3_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_ack_4_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->normal_logoff_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->abnormal_logoff_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->macauth_abnormal_logoff_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_timeout_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->challenge_busy_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->req_auth_password_missing_count)); 
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->req_auth_unknown_type_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->ack_auth_busy_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->auth_disorder_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->access_request_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->access_request_retry_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->access_request_timeout_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->access_accept_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->access_reject_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_start_count)); 
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_start_retry_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_response_start_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_update_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_update_retry_count));	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_response_update_count)); 
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_stop_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_request_stop_retry_count));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(eag_stat->acct_response_stop_count));
	dbus_message_iter_next(&iter);

error:
	dbus_message_unref(reply);
	return iRet;
}


int
eag_userdb_init(struct eag_userdb *userdb)
{
	if (NULL == userdb) {
		return -1;
	}

	memset(userdb, 0, sizeof(*userdb));
	INIT_LIST_HEAD(&(userdb->head));
	userdb->num = 0;

	return 0;
}

int
eag_userdb_destroy(struct eag_userdb *userdb)
{
	struct eag_user *user = NULL;
	struct eag_user *next = NULL;
	
	if (NULL == userdb) {
		return -1;
	}

	list_for_each_entry_safe(user, next, &(userdb->head), node) {
		list_del(&(user->node));
		free(user);
	}
	userdb->num = 0;

	return 0;
}

int
eag_show_user_all(DBusConnection *connection,
				int hansitype, int insid,
				struct eag_userdb *userdb)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter iter = {0};
	DBusMessageIter iter_array = {0};
	DBusMessageIter iter_struct = {0};
	struct eag_user *user = NULL;
	char *username = NULL;
	int iRet = 0;
	int i = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_SHOW_USER_ALL);

	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if (EAG_RETURN_OK == iRet) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(userdb->num));

			if (userdb->num > 0) {
				dbus_message_iter_next(&iter);
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < userdb->num; i++) {
					user = malloc(sizeof(*user));
					if (NULL == user) {
						eag_userdb_destroy(userdb);
						dbus_message_unref(reply);
						return EAG_ERR_MALLOC_FAILED;
					}
					memset(user, 0, sizeof(*user));
					
					dbus_message_iter_recurse(&iter_array, &iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(username));
					if (NULL != username) {
						strncpy(user->username, username, 
								sizeof(user->username)-1);
					}
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->usermac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->session_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->accurate_start_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->apmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->vlanid));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->sta_state));
					dbus_message_iter_next(&iter_array);
					list_add_tail(&(user->node), &(userdb->head));
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_show_user_by_username(DBusConnection *connection,
				int hansitype, int insid,
				struct eag_userdb *userdb,
				const char *username)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter iter = {0};
	DBusMessageIter iter_array = {0};
	DBusMessageIter iter_struct = {0};
	struct eag_user *user = NULL;
	char *name = NULL;
	int iRet = 0;
	int i = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_SHOW_USER_BY_USERNAME);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &username,
								DBUS_TYPE_INVALID );
	
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if (EAG_RETURN_OK == iRet) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(userdb->num));

			if (userdb->num > 0) {
				dbus_message_iter_next(&iter);
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < userdb->num; i++) {
					user = malloc(sizeof(*user));
					if (NULL == user) {
						eag_userdb_destroy(userdb);
						dbus_message_unref(reply);
						return EAG_ERR_MALLOC_FAILED;
					}
					memset(user, 0, sizeof(*user));
					
					dbus_message_iter_recurse(&iter_array, &iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(name));
					if (NULL != name) {
						strncpy(user->username, name,
								sizeof(user->username)-1);
					}
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->usermac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->session_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->apmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[5]));
					dbus_message_iter_next(&iter_array);
					list_add_tail(&(user->node), &(userdb->head));
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_show_user_by_userip(DBusConnection *connection,
				int hansitype, int insid,
				struct eag_userdb *userdb,
				uint32_t userip)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter iter = {0};
	DBusMessageIter iter_array = {0};
	DBusMessageIter iter_struct = {0};
	struct eag_user *user = NULL;
	char *username = NULL;
	int iRet = 0;
	int i = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_SHOW_USER_BY_USERIP);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_INVALID );
	
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if (EAG_RETURN_OK == iRet) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(userdb->num));

			if (userdb->num > 0) {
				dbus_message_iter_next(&iter);
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < userdb->num; i++) {
					user = malloc(sizeof(*user));
					if (NULL == user) {
						eag_userdb_destroy(userdb);
						dbus_message_unref(reply);
						return EAG_ERR_MALLOC_FAILED;
					}
					memset(user, 0, sizeof(*user));
					
					dbus_message_iter_recurse(&iter_array, &iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(username));
					if (NULL != username) {
						strncpy(user->username, username,
								sizeof(user->username)-1);
					}
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->usermac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->session_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->apmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[5]));
					dbus_message_iter_next(&iter_array);
					list_add_tail(&(user->node), &(userdb->head));
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_show_user_by_usermac(DBusConnection *connection,
				int hansitype, int insid,
				struct eag_userdb *userdb,
				uint8_t usermac[6])
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter iter = {0};
	DBusMessageIter iter_array = {0};
	DBusMessageIter iter_struct = {0};
	struct eag_user *user = NULL;
	char *username = NULL;
	int iRet = 0;
	int i = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_SHOW_USER_BY_USERMAC);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE, &usermac[0],
								DBUS_TYPE_BYTE, &usermac[1],
								DBUS_TYPE_BYTE, &usermac[2],
								DBUS_TYPE_BYTE, &usermac[3],
								DBUS_TYPE_BYTE, &usermac[4],
								DBUS_TYPE_BYTE, &usermac[5],
								DBUS_TYPE_INVALID );
	
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if (EAG_RETURN_OK == iRet) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(userdb->num));

			if (userdb->num > 0) {
				dbus_message_iter_next(&iter);
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < userdb->num; i++) {
					user = malloc(sizeof(*user));
					if (NULL == user) {
						eag_userdb_destroy(userdb);
						dbus_message_unref(reply);
						return EAG_ERR_MALLOC_FAILED;
					}
					memset(user, 0, sizeof(*user));
					
					dbus_message_iter_recurse(&iter_array, &iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(username));
					if (NULL != username) {
						strncpy(user->username, username,
								sizeof(user->username)-1);
					}
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->usermac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->session_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->apmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[5]));
					dbus_message_iter_next(&iter_array);
					list_add_tail(&(user->node), &(userdb->head));
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;	
}

int
eag_show_user_by_index(DBusConnection *connection,
				int hansitype, int insid,
				struct eag_userdb *userdb,
				uint32_t index)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter iter = {0};
	DBusMessageIter iter_array = {0};
	DBusMessageIter iter_struct = {0};
	struct eag_user *user = NULL;
	char *username = NULL;
	int iRet = 0;
	int i = 0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_SHOW_USER_BY_INDEX);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_INVALID );
	
	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	} else {
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if (EAG_RETURN_OK == iRet) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(userdb->num));

			if (userdb->num > 0) {
				dbus_message_iter_next(&iter);
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < userdb->num; i++) {
					user = malloc(sizeof(*user));
					if (NULL == user) {
						eag_userdb_destroy(userdb);
						dbus_message_unref(reply);
						return EAG_ERR_MALLOC_FAILED;
					}
					memset(user, 0, sizeof(*user));
					
					dbus_message_iter_recurse(&iter_array, &iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(username));
					if (NULL != username) {
						strncpy(user->username, username,
								sizeof(user->username)-1);
					}
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->usermac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->usermac[5]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->session_time));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_octets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->input_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->output_packets));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
													&(user->apmac[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[4]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
													&(user->apmac[5]));
					dbus_message_iter_next(&iter_array);
					list_add_tail(&(user->node), &(userdb->head));
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;	
}

int
eag_kick_user_by_username(DBusConnection *connection,
				int hansitype, int insid,
				const char *username)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype, insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_KICK_USER_BY_USERNAME);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &username,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_kick_user_by_userip(DBusConnection *connection,
				int hansitype, int insid,
				uint32_t userip)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype, insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_KICK_USER_BY_USERIP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_kick_user_by_usermac(DBusConnection *connection,
				int hansitype, int insid,
				uint8_t usermac[6])
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype, insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_KICK_USER_BY_USERMAC);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE, &usermac[0],
								DBUS_TYPE_BYTE, &usermac[1],
								DBUS_TYPE_BYTE, &usermac[2],
								DBUS_TYPE_BYTE, &usermac[3],
								DBUS_TYPE_BYTE, &usermac[4],
								DBUS_TYPE_BYTE, &usermac[5],
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

int
eag_kick_user_by_index(DBusConnection *connection,
				int hansitype, int insid,
				uint32_t index)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	eag_dbus_path_reinit(hansitype, insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE,
									EAG_DBUS_METHOD_KICK_USER_BY_INDEX);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;

}

int
eag_set_user_log_status( DBusConnection *connection, 
				int hansitype, int insid,
				int status)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_USER_LOG_STATUS );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);

	return iRet;
}

int
eag_set_log_format_status( DBusConnection *connection, 
				int hansitype, int insid,
				int key, int status)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_LOG_FORMAT_STATUS );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &key,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_set_username_check_status( DBusConnection *connection, 
				int hansitype, int insid,
				int status)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_USERNAME_CHECK_STATUS );
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
eag_add_debug_filter( DBusConnection *connection, 
				int hansitype,int insid, 					
				char *filter )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_ADD_DEBUG_FILTER );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_del_debug_filter( DBusConnection *connection, 
				int hansitype,int insid, 					
				char *filter )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_DEL_DEBUG_FILTER );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_appconn( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_APPCONN );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_redirconn( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_REDIRCONN );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}


int
eag_log_all_portalsess( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_PORTALSESS );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_sockradius( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_SOCKRADIUS );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_thread( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_THREAD );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_blkmem( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_BLKMEM );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_log_all_mac_preauth( DBusConnection *connection, 
				int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_ALL_MAC_PREAUTH);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_dbus_eag_debug_on(DBusConnection *connection, 
						int hansitype, int insid, unsigned int flag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_DEBUG_ON);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_dbus_eag_debug_off(DBusConnection *connection, 
						int hansitype, int insid, unsigned int flag)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_LOG_DEBUG_OFF);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_rdc_client_log( DBusConnection *connection, 
				int hansitype,int insid, 					
				int flag )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_RDC_CLIENT_LOG );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flag,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
eag_set_pdc_client_log( DBusConnection *connection, 
				int hansitype,int insid, 					
				int flag )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	eag_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									EAG_DBUS_NAME,
									EAG_DBUS_OBJPATH,
									EAG_DBUS_INTERFACE, 
									EAG_DBUS_METHOD_SET_PDC_CLIENT_LOG );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &flag,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int captive_check_ip_format(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return INTERFACE_IP_CHECK_FAILURE;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return INTERFACE_IP_CHECK_FAILURE;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return INTERFACE_IP_CHECK_FAILURE;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return INTERFACE_IP_CHECK_FAILURE;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return INTERFACE_IP_CHECK_FAILURE;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return INTERFACE_IP_CHECK_SUCCESS;
		else
			return INTERFACE_IP_CHECK_FAILURE;
	}
	else
		return INTERFACE_IP_CHECK_FAILURE;		
}

int captive_check_mac_format
(
	const char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = INTERFACE_MAC_CHECK_SUCESS;
	char c = 0;
	if( 17 != len){
	   return INTERFACE_MAC_CHECK_FAILURE;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c))
				return INTERFACE_MAC_CHECK_FAILURE;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = INTERFACE_MAC_CHECK_FAILURE;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
        result = INTERFACE_MAC_CHECK_FAILURE;
		return result;
	}
	return result;
}

