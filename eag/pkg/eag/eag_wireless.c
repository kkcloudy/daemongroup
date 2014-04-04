/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* eag_wireless.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* eag_wireless
*
*
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
//#include "dbus/wcpss/ACDbusDef.h"
#include "dbus/asd/ASDDbusDef.h"

#include "eag_errcode.h"
#include "eag_log.h"
#include "eag_util.h"
#include "session.h"
#include "eag_wireless.h"

#define MAX_BANDWIDTH_LIMIT  884736
#define BANDWIDTH_TYPE_UP		1		
#define BANDWIDTH_TYPE_DOWN		2

void *eag_dcli_dl_handle = NULL;

static const char *
safe_dlerror(void)
{
	const char *s = dlerror();
	return (s != NULL) ? s : "Unknown error";
}

int
eag_dcli_dl_init(void)
{ 
	eag_dcli_dl_handle = dlopen("/opt/lib/libdclipub.so.0", RTLD_NOW);
	if (NULL == eag_dcli_dl_handle) {
		eag_log_err("eag_dcli_dl_init failed, %s", safe_dlerror());
		return EAG_ERR_FILE_OPEN_FAILED;
	}
	
	return EAG_RETURN_OK;
}

int
eag_dcli_dl_uninit(void)
{
	if (NULL != eag_dcli_dl_handle){
		dlclose(eag_dcli_dl_handle);
	}
	
	return EAG_RETURN_OK;
}

int
eag_get_sta_info_by_mac_v2( eag_dbus_t *eag_dbus,
					int localid, int inst_id,
					uint8_t *sta_mac,
					struct appsession *session,
					unsigned int *security_type,
					int notice_to_asd)
{
	int return_ret = EAG_RETURN_OK;
	unsigned int ret = 0;
	struct dcli_sta_info_v2 *sta = NULL;
	void *handle = eag_dcli_dl_handle;
	static struct dcli_sta_info_v2* (*dl_get_sta_info_by_mac_v2)
			(DBusConnection *, int, unsigned char *, int, unsigned int *) = NULL;
	static void (*dl_dcli_free_sta_v2)(struct dcli_sta_info_v2 *) = NULL;
	char str_sta_mac[32] = "";
	char str_wtp_mac[32] = "";
	uint8_t zero_mac[6] = {0};
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char login_ipv6_host_str[48] = "";
	char framed_ipv6_prefix_str[48] = "";
	DBusConnection * conn = eag_dbus_get_dbus_conn(eag_dbus);
	
	if (NULL == eag_dbus || NULL == sta_mac || NULL == session || NULL == handle) {
		eag_log_err("eag_get_sta_info_by_mac_v2 input error "
					"conn=%p, sta_mac=%p, session=%p, handle=%p",
					eag_dbus, sta_mac, session, handle);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (memcmp(zero_mac, sta_mac, 6) == 0) {
		eag_log_err("eag_get_sta_info_by_mac_v2 sta_mac is 00:00:00:00:00:00");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	mac2str(sta_mac, str_sta_mac, sizeof(str_sta_mac), '-');

	eag_log_info("eag_get_sta_info_by_mac_v2 begin, "
			"sta_mac=%s, hansitype=%d, hansiid=%d",
			str_sta_mac, localid, inst_id);
	
	if (NULL == dl_get_sta_info_by_mac_v2) {
		dl_get_sta_info_by_mac_v2 = dlsym(handle, "get_sta_info_by_mac_v2");
		if (NULL == dl_get_sta_info_by_mac_v2) {
			eag_log_err("dlsym get_sta_info_by_mac_v2 error: %s", safe_dlerror());
			dlerror(); //clear exist error;
			return EAG_ERR_WIRELESS_DLSYM_FAILED;
		}
	}
	
	if (NULL == dl_dcli_free_sta_v2) {
		dl_dcli_free_sta_v2 = dlsym(handle, "dcli_free_sta_v2");
		if (NULL == dl_dcli_free_sta_v2) {
			eag_log_err("dlsym dcli_free_sta_v2 error: %s", safe_dlerror());
			dlerror(); //clear exist error;
			return EAG_ERR_WIRELESS_DLSYM_FAILED;
		}
	}
	
	sta = dl_get_sta_info_by_mac_v2(conn, inst_id, sta_mac, localid, &ret);
	if (NULL == sta || 0 != ret) {
		eag_log_err("dl_get_sta_info_by_mac_v2 failed, usermac=%s, ret=%d",
					str_sta_mac, ret);
		if (1 == ret) { /* ret=1 means dbus connection error: NoReply */
			/* reinit and reget */
			eag_dbus_reinit(eag_dbus);
			sta = dl_get_sta_info_by_mac_v2(conn, inst_id, sta_mac, localid, &ret);
			if (NULL!=sta && 0==ret) {
				eag_log_info("dl_get_sta_info_by_mac_v2 reget_sta_info success, usermac=%s, ret=%d",
						str_sta_mac, ret);
				goto reget;
			}
			eag_log_err("dl_get_sta_info_by_mac_v2 reget_sta_info failed, usermac=%s, ret=%d",str_sta_mac, ret);
		}
		return_ret = EAG_ERR_WIRELESS_STA_NOT_EXIST;
		goto end;
	}

reget:
	session->wlanid = sta->wlan_id;
	session->g_radioid = sta->radio_g_id;
	session->radioid = sta->radio_g_id%L_RADIO_NUM;
	session->wtpid= sta->radio_g_id/L_RADIO_NUM;
	if (notice_to_asd) {
		session->audit_ip = sta->realip;
	} else {
		session->audit_ip = session->user_addr.user_ip;
	}
#if 0/*add by shaojunwu for 2.1*/
	session->idle_check = sta->flow_check;
	session->idle_timeout = sta->no_flow_time;	
	session->idle_flow = sta->limit_flow;
#endif	
	if (NULL != sta->essid) {
		strncpy(session->essid, sta->essid, sizeof(session->essid)-1);
	}
	if (NULL != sta->wtp_name) {
		strncpy(session->apname, sta->wtp_name, sizeof(session->apname)-1);
	}
	memcpy(session->apmac, sta->addr, sizeof(session->apmac));
	session->vlanid = sta->vlan_id;
	*security_type = sta->auth_type;
	
	if (EAG_IPV4 == session->user_addr.family
		&& 0 != ipv6_compare_null(&(sta->ip6_addr))) {
		session->user_addr.user_ipv6 = sta->ip6_addr;
		session->user_addr.family = EAG_MIX;
	} else if (EAG_IPV6 == session->user_addr.family
		&& 0 != sta->ip_addr.s_addr) {
		session->user_addr.user_ip = sta->ip_addr.s_addr;
		session->user_addr.family = EAG_MIX;
	}

	session->framed_interface_id = sta->Framed_Interface_Id;
	session->framed_ipv6_prefix[0] = 0;
	session->framed_ipv6_prefix[1] = sta->IPv6_Prefix_length;
	memcpy((uint8_t *)&(session->framed_ipv6_prefix[2]), 
			(uint8_t *)&(sta->Framed_IPv6_Prefix), sizeof(struct in6_addr));
	session->login_ipv6_host = sta->Login_IPv6_Host;
	memcpy(session->delegated_ipv6_prefix, 
			session->framed_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

	mac2str(session->apmac, str_wtp_mac, sizeof(str_wtp_mac), '-');
	ip2str(session->user_addr.user_ip, ipstr, sizeof(ipstr));
	ipv6tostr(&(session->user_addr.user_ipv6), ipv6str, sizeof(ipv6str));
	ipv6tostr(&(session->login_ipv6_host), login_ipv6_host_str, sizeof(login_ipv6_host_str));
	ipv6tostr((struct in6_addr *)&(session->framed_ipv6_prefix[2]),
			framed_ipv6_prefix_str, sizeof(framed_ipv6_prefix_str));
	snprintf(session->framed_ipv6_pool, MAX_FRAMED_IPV6_ATTR_LEN - 1, 
			"%s/%hhu", framed_ipv6_prefix_str, session->framed_ipv6_prefix[1]);
	snprintf(session->framed_ipv6_route, MAX_FRAMED_IPV6_ATTR_LEN - 1,
			"%s %s1 1", session->framed_ipv6_pool, framed_ipv6_prefix_str);

	eag_log_info("eag_get_sta_info_by_mac_v2 success, "
			"sta_mac=%s, sta_ip=%s, sta_ipv6=%s, radio_id=%d, wlan_id=%d, wtp_id=%d, "
			"essid=%s, wtp_mac=%s, wtp_name=%s, vlanid=%d, idle_check=%u, "
			"idle_timeout=%lu, idle_flow=%llu, security_type=%d,audit_ip=%x",
			str_sta_mac, ipstr, ipv6str, session->radioid, session->wlanid, session->wtpid,
			session->essid, str_wtp_mac, session->apname, session->vlanid, session->idle_check,
			session->idle_timeout, session->idle_flow, *security_type, session->audit_ip);
	eag_log_info("framed_interface_id=%lld, login_ipv6_host=%s, framed_ipv6_prefix=%s, ipv6_prefix_length=%hhu",
			session->framed_interface_id, login_ipv6_host_str, framed_ipv6_prefix_str, session->framed_ipv6_prefix[1]);
end:
	if( NULL != sta && NULL != dl_dcli_free_sta_v2)
	{
		dl_dcli_free_sta_v2(sta);
	}
	
	return return_ret;
}

struct WtpStaInfo *
eag_show_sta_info_of_all_wtp(int ins_id,
				int local_id,
				DBusConnection *conn,
				int *iRet)
{
	unsigned int ret = 0;
	unsigned int wtp_num = 0;
	struct WtpStaInfo *StaHead = NULL;
	void *handle = eag_dcli_dl_handle;
	static void* (*dl_show_sta_info_of_all_wtp)(int, int, DBusConnection *,
						unsigned int *, unsigned int *) = NULL;

	if (NULL == conn || NULL == handle) {
		eag_log_err("eag_show_sta_info_of_all_wtp input error");
		*iRet = EAG_ERR_UNKNOWN;
        return NULL;
	}

	if (NULL == dl_show_sta_info_of_all_wtp) {
		dl_show_sta_info_of_all_wtp = dlsym(handle, "show_sta_info_of_all_wtp");
		if (NULL == dl_show_sta_info_of_all_wtp) {
			eag_log_err("dlsym show_sta_info_of_all_wtp error: %s", safe_dlerror());
			dlerror(); //clear exist error;
			*iRet = EAG_ERR_UNKNOWN;
			return NULL;
		}
	}

	StaHead = (*dl_show_sta_info_of_all_wtp)(ins_id, local_id,
								conn, &wtp_num, &ret);
	if (NULL != StaHead && 0 == ret) {
		*iRet = EAG_RETURN_OK;
		return StaHead;
	} else if (0 == ret) {
		*iRet = EAG_ERR_WIRELESS_NO_STA;
		eag_log_err("eag_show_sta_info_of_all_wtp, there is no sta now");
	} else if (ASD_DBUS_ERROR == ret) {
		*iRet = EAG_ERR_WIRELESS_CONNECTION_ERR;
		eag_log_err("eag_show_sta_info_of_all_wtp, asd dbus error");
	} else if (ASD_WTP_NOT_EXIST == ret) {
		*iRet = EAG_ERR_WIRELESS_NO_WTP;
		eag_log_err("eag_show_sta_info_of_all_wtp, wtp not exist");
	} else {
		*iRet = EAG_ERR_UNKNOWN;
		eag_log_err("eag_show_sta_info_of_all_wtp, unknown error");
	}
	return NULL;
}

int
eag_free_wtp_sta_info_head(struct WtpStaInfo *StaHead)
{
	static void (*dl_free_wtp_sta_info_head)(struct WtpStaInfo *) = NULL;
	void *handle = eag_dcli_dl_handle;

	if (NULL == StaHead) {
		eag_log_err("eag_free_wtp_sta_info_head input error");
		return -1;
	}
	
	if (NULL == dl_free_wtp_sta_info_head) {
		dl_free_wtp_sta_info_head = dlsym(handle, "dcli_free_wtp_sta_info_head");
		if (NULL == dl_free_wtp_sta_info_head) {
			eag_log_err("dlsym dcli_free_wtp_sta_info_head error: %s", safe_dlerror());
			dlerror(); //clear exist error;
			return EAG_ERR_WIRELESS_DLSYM_FAILED;
		}
	}

	(*dl_free_wtp_sta_info_head)(StaHead);

	return 0;
}

int
eag_set_sta_up_traffic_limit(DBusConnection *conn,
					int localid, int ins_id,
					uint8_t sta_mac[6],
					uint32_t radio_id,
					uint8_t wlan_id,
					uint32_t value)
{
	void *dl_handle = eag_dcli_dl_handle;
	void (*dcli_func_asd_set_sta_info)(  DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	void (*dcli_func_wid_set_sta_info)(	DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	void (*dcli_func_set_sta_static_info)(	DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	unsigned int ret = 0;
	int return_ret = EAG_ERR_UNKNOWN;
	char str_sta_mac[32] = "";
	
	if (NULL == conn || NULL == sta_mac) {
		eag_log_err("eag_set_sta_up_traffic_limit failed, input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (WLAN_NUM <= wlan_id || 0 == wlan_id) {
		eag_log_err("eag_set_sta_up_traffic_limit failed, wlan_id=%u", wlan_id);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (value > MAX_BANDWIDTH_LIMIT || value < 1) {
		eag_log_err("eag_set_sta_up_traffic_limit failed, value=%u",value);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (NULL == dl_handle) {
		eag_log_err("eag_set_sta_up_traffic_limit failed, dl_handle null");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	mac2str(sta_mac, str_sta_mac, sizeof(str_sta_mac), '-');
	eag_log_debug("eag_wireless","set sta up traffic limit, stamac=%s, value=%u Kbps",
		str_sta_mac, value);
	dcli_func_asd_set_sta_info = dlsym(dl_handle, "asd_set_sta_info");
	if (NULL == dcli_func_asd_set_sta_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym asd_set_sta_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}
	(*dcli_func_asd_set_sta_info)(	conn,
									ins_id,
									sta_mac,
									wlan_id,
									radio_id,
									BANDWIDTH_TYPE_UP,
									value,
									localid,
									&ret);
	if (0 != ret) {
		if (ASD_STA_NOT_EXIST == ret) {
			eag_log_err("up traffic limit(asd_set_sta_info) error, "\
						"asd sta %s not exist", str_sta_mac);
		}
		else if (ASD_WLAN_NOT_EXIST == ret) {
			eag_log_err("up traffic limit(asd_set_sta_info) error, "\
						"asd wlan %d not exist.", wlan_id);
		}
		else {
			eag_log_err("up traffic limit(asd_set_sta_info) error, ret=%d", ret);
		}
		return_ret = EAG_ERR_WIRELESS_TRAFFIC_LIMIT_FAILED;
		goto end;
	}

	dcli_func_wid_set_sta_info = dlsym(dl_handle,"wid_set_sta_info");
	if (NULL == dcli_func_wid_set_sta_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym wid_set_sta_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}	
	(*dcli_func_wid_set_sta_info)(	conn,
									ins_id,
									sta_mac,
									wlan_id,
									radio_id,
									BANDWIDTH_TYPE_UP,
									value,
									localid,
									&ret);
	if (0 != ret) {
		eag_log_err("up traffic limit(wid_set_sta_info) error, ret=%d", ret);
		return_ret = EAG_ERR_WIRELESS_TRAFFIC_LIMIT_FAILED;
		goto end;
	}

	dcli_func_set_sta_static_info = dlsym(dl_handle, "set_sta_static_info");
	if (NULL == dcli_func_set_sta_static_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym set_sta_static_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}
	(*dcli_func_set_sta_static_info)(	conn,
										ins_id,
										sta_mac,
										wlan_id,
										radio_id,
										BANDWIDTH_TYPE_UP,
										value,
										localid,
										&ret);
	if (0 != ret) {
		eag_log_err("up traffic limit(set_sta_static_info) error, ret=%d", ret);
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}

	return_ret = EAG_RETURN_OK;
	eag_log_debug("eag_wireless", "set sta up traffic limit success");
	
end:

	return return_ret;
}

int
eag_set_sta_down_traffic_limit(DBusConnection *conn,
					int localid, int ins_id,
					uint8_t sta_mac[6],
					uint32_t radio_id,
					uint8_t wlan_id,
					uint32_t value)
{
	void *dl_handle = eag_dcli_dl_handle;
	void (*dcli_func_asd_set_sta_info)(  DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	void (*dcli_func_wid_set_sta_info)(	DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	void (*dcli_func_set_sta_static_info)(	DBusConnection *,
										int,
										unsigned char *,
										unsigned char,
										unsigned int,
										unsigned char,
										unsigned int,
										int,
										unsigned int *) = NULL;
	unsigned int ret = 0;
	int return_ret = EAG_ERR_UNKNOWN;
	char str_sta_mac[32] = "";
	
	if (NULL == conn || NULL == sta_mac) {
		eag_log_err("eag_set_sta_down_traffic_limit failed, input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (WLAN_NUM <= wlan_id || 0 == wlan_id) {
		eag_log_err("eag_set_sta_down_traffic_limit failed, wlan_id=%u", wlan_id);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (value > MAX_BANDWIDTH_LIMIT || value < 1) {
		eag_log_err("eag_set_sta_down_traffic_limit failed, value=%u",value);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (NULL == dl_handle) {
		eag_log_err("eag_set_sta_down_traffic_limit failed, dl_handle null");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	mac2str(sta_mac, str_sta_mac, sizeof(str_sta_mac), '-');
	eag_log_debug("eag_wireless","set sta down traffic limit, stamac=%s, value=%u Kbps",
		str_sta_mac, value);
	dcli_func_asd_set_sta_info = dlsym(dl_handle, "asd_set_sta_info");
	if (NULL == dcli_func_asd_set_sta_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym asd_set_sta_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}
	(*dcli_func_asd_set_sta_info)(	conn,
									ins_id,
									sta_mac,
									wlan_id,
									radio_id,
									BANDWIDTH_TYPE_DOWN,
									value,
									localid,
									&ret);
	if (0 != ret) {
		if (ASD_STA_NOT_EXIST == ret) {
			eag_log_err("dowm traffic limit(asd_set_sta_info) error, asd sta not exist");
		}
		else if (ASD_WLAN_NOT_EXIST == ret) {
			eag_log_err("dowm traffic limit(asd_set_sta_info) error, asd wlan not exist");
		}
		else {
			eag_log_err("dowm traffic limit(asd_set_sta_info) error, ret=%d", ret);
		}
		return_ret = EAG_ERR_WIRELESS_TRAFFIC_LIMIT_FAILED;
		goto end;
	}

	dcli_func_wid_set_sta_info = dlsym(dl_handle,"wid_set_sta_info");
	if (NULL == dcli_func_wid_set_sta_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym wid_set_sta_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}	
	(*dcli_func_wid_set_sta_info)(	conn,
									ins_id,
									sta_mac,
									wlan_id,
									radio_id,
									BANDWIDTH_TYPE_DOWN,
									value,
									localid,
									&ret);
	if (0 != ret) {
		eag_log_err("dowm traffic limit(wid_set_sta_info) error, ret=%d", ret);
		return_ret = EAG_ERR_WIRELESS_TRAFFIC_LIMIT_FAILED;
		goto end;
	}

	dcli_func_set_sta_static_info = dlsym(dl_handle, "set_sta_static_info");
	if (NULL == dcli_func_set_sta_static_info) {
		eag_log_err("eag_set_sta_up_traffic_limit, "
			"dlsym set_sta_static_info failed, %s",
			safe_dlerror());
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}
	(*dcli_func_set_sta_static_info)(	conn,
										ins_id,
										sta_mac,
										wlan_id,
										radio_id,
										BANDWIDTH_TYPE_DOWN,
										value,
										localid,
										&ret);
	if (0 != ret) {
		eag_log_err("dowm traffic limit(set_sta_static_info) error, ret=%d", ret);
		return_ret = EAG_ERR_WIRELESS_DLSYM_FAILED;
		goto end;
	}

	return_ret = EAG_RETURN_OK;
	eag_log_debug("eag_wireless", "set sta down traffic limit success");
	
end:

	return return_ret;
}

