/* eag_stamsg.c */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "eag_conf.h"  //not need
#include "eag_interface.h"
#include "eag_dbus.h"
#include "eag_hansi.h"
#include "eag_time.h"
#include "eag_util.h"


#include "wcpss/waw.h"
#include "eag_stamsg.h"
#include "eag_ins.h"
#include "eag_portal.h"
#include "eag_captive.h"
#include "radius_packet.h"
#include "eag_wireless.h"
#include "eag_statistics.h"
#include "eag_macauth.h"

struct eag_stamsg {
	int sockfd;
	uint8_t hansi_type;
	uint8_t hansi_id;
	char sockpath[128];
	char ntf_asd_path[128];
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	eag_ins_t *eagins;
	eag_portal_t *portal;
	eag_dbus_t *eagdbus;
	appconn_db_t *appdb;
	eag_captive_t *captive;
	eag_statistics_t *eagstat;
	eag_macauth_t *macauth;
	struct portal_conf *portalconf;
	struct nasid_conf *nasidconf;
	struct nasportid_conf *nasportidconf;
	eag_hansi_t *eaghansi;
};

typedef enum {
	EAG_STAMSG_READ,
} eag_stamsg_event_t;

static void
eag_stamsg_event(eag_stamsg_event_t event,
		eag_stamsg_t *stamsg);

eag_stamsg_t *
eag_stamsg_new(uint8_t hansi_type, 
		uint8_t hansi_id)
{
	eag_stamsg_t *stamsg = NULL;

	stamsg = eag_malloc(sizeof(*stamsg));
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_new eag_malloc failed");
		return NULL;
	}

	memset(stamsg, 0, sizeof(*stamsg));
	stamsg->sockfd = -1;
	stamsg->hansi_type = hansi_type;
	stamsg->hansi_id = hansi_id;
	snprintf(stamsg->sockpath, sizeof(stamsg->sockpath),
			STAMSG_SOCK_PATH_FMT, hansi_type, hansi_id);
	
	snprintf(stamsg->ntf_asd_path, sizeof(stamsg->sockpath),
			STAMSG_ASD_SOCK_PATH_FMT, hansi_type, hansi_id);
	
	eag_log_debug("eag_stamsg", "stamsg new ok, sockpath=%s",
			stamsg->sockpath);
	return stamsg;
}

int
eag_stamsg_free(eag_stamsg_t *stamsg)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_free input error");
		return -1;
	}

	if (stamsg->sockfd >= 0) {
		close(stamsg->sockfd);
		stamsg->sockfd = -1;
	}
	eag_free(stamsg);

	eag_log_debug("eag_stamsg", "stamsg free ok");
	return 0;
}

int
eag_stamsg_start(eag_stamsg_t *stamsg)
{
	int ret = 0;
	int len = 0;
	struct sockaddr_un addr = {0};
	mode_t old_mask = 0;
  
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_start input error");
		return EAG_ERR_NULL_POINTER;
	}

	if (stamsg->sockfd >= 0) {
		eag_log_err("eag_stamsg_start already start fd(%d)", 
			stamsg->sockfd);
		return EAG_RETURN_OK;
	}

	stamsg->sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (stamsg->sockfd  < 0) {
		eag_log_err("Can't create stamsg unix dgram socket: %s",
			safe_strerror(errno));
		stamsg->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	if (0 != set_nonblocking(stamsg->sockfd)){
		eag_log_err("eag_stamsg_start set socket nonblocking failed");
		close(stamsg->sockfd);
		stamsg->sockfd = -1;
		return EAG_ERR_SOCKET_OPT_FAILED;
	}
		
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, stamsg->sockpath, sizeof(addr.sun_path)-1);
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = addr.sun_len = SUN_LEN(&addr);
#else
	len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	unlink(addr.sun_path);
	old_mask = umask(0111);
	ret  = bind(stamsg->sockfd, (struct sockaddr *)&addr, len);
	if (ret < 0) {
		eag_log_err("Can't bind to stamsg socket(%d): %s",
			stamsg->sockfd, safe_strerror(errno));
		close(stamsg->sockfd);
		stamsg->sockfd = -1;
		umask(old_mask);
		return EAG_ERR_SOCKET_BIND_FAILED;
	}
	umask(old_mask);
	
	eag_stamsg_event(EAG_STAMSG_READ, stamsg);
	
	eag_log_info("stamsg(%s) fd(%d) start ok",
			stamsg->sockpath,
			stamsg->sockfd);

	return EAG_RETURN_OK;
}

int
eag_stamsg_stop(eag_stamsg_t *stamsg)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_stop input error");
		return EAG_ERR_NULL_POINTER;
	}

	eag_log_info("stamsg(%s) fd(%d) stop ok",
			stamsg->sockpath,
			stamsg->sockfd);
		
	if (NULL != stamsg->t_read) {
		eag_thread_cancel(stamsg->t_read);
		stamsg->t_read = NULL;
	}
	if (stamsg->sockfd >= 0)
	{
		close(stamsg->sockfd);
		stamsg->sockfd = -1;
	}
	unlink(stamsg->sockpath);
	
	return EAG_RETURN_OK;
}

static int
stamsg_proc(eag_stamsg_t *stamsg, uint8_t usermac[6],
		user_addr_t *user_addr, EagMsg *sta_msg)
{
	struct app_conn_t *appconn = NULL;
	struct appsession tmpsession = {0};
	struct timeval tv = {0};
	time_t timenow = 0;
	int ret = 0;
	char user_macstr[32] = "";
	char user_ipstr[IPX_LEN] = "";
	char ap_macstr[32] = "";
	char new_apmacstr[32] = "";
	unsigned int security_type = 0;
	int macauth_switch = 0;
	int notice_to_asd = 0;
	
	eag_time_gettimeofday(&tv,NULL);
	timenow = tv.tv_sec;
	macauth_switch = eag_macauth_get_macauth_switch(stamsg->macauth);
	
	switch(sta_msg->Op) {
	case WID_ADD:
		if (0 == memcmp_ipx(user_addr, NULL)) {
			eag_log_warning("stamsg_proc receive WID_ADD, userip = 0");
		}

		/* TODO: if essid changed, del mac_preauth */
		mac2str(usermac, user_macstr, sizeof(user_macstr), ':');

		if (eag_hansi_is_enable(stamsg->eaghansi)
			&& !eag_hansi_is_master(stamsg->eaghansi))
		{
			eag_log_info("receive WID_ADD usermac=%s, but hansi is backup, ignore it",
				user_macstr);
			return 0;
		}
		
		appconn = appconn_find_by_usermac(stamsg->appdb, usermac);
		if (NULL == appconn) {
			eag_log_info("stamsg_proc, appconn not exist, usermac=%s",
				user_macstr);
			return 0;
		}		
		appconn->session.sta_state = SESSION_STA_STATUS_CONNECT;

		tmpsession.idle_check = 1;
		appconn_db_get_idle_params(appconn->appdb, &(tmpsession.idle_timeout), &(tmpsession.idle_flow));

		mac2str(appconn->session.apmac, ap_macstr, sizeof(ap_macstr), ':');
		ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));
		
		tmpsession.wlanid = sta_msg->STA.wlan_id;
		tmpsession.g_radioid = sta_msg->STA.radio_id;
		tmpsession.radioid = sta_msg->STA.radio_id%L_RADIO_NUM;
		tmpsession.wtpid= sta_msg->STA.wtp_id;
		strncpy(tmpsession.essid, (char *)sta_msg->STA.essid, sizeof(tmpsession.essid)-1);
		strncpy(tmpsession.apname, (char *)sta_msg->STA.wtp_name, sizeof(tmpsession.apname)-1);
		memcpy(tmpsession.apmac, sta_msg->STA.wtp_mac, sizeof(tmpsession.apmac));
		tmpsession.vlanid = sta_msg->STA.vlan_id;
		security_type = sta_msg->STA.auth_type;

        tmpsession.framed_interface_id = sta_msg->STA.Framed_Interface_Id;
        tmpsession.framed_ipv6_prefix[0] = 0;
        tmpsession.framed_ipv6_prefix[1] = sta_msg->STA.IPv6_Prefix_length;
        memcpy((uint8_t *)&(tmpsession.framed_ipv6_prefix[2]), 
                (uint8_t *)&(sta_msg->STA.Framed_IPv6_Prefix), sizeof(struct in6_addr));
        tmpsession.login_ipv6_host = sta_msg->STA.Login_IPv6_Host;
        strncpy(tmpsession.framed_ipv6_route, 
                "2003:4:1::/64 2014:4:1::1988:923 1", MAX_FRAMED_IPV6_ATTR_LEN - 1);
        strncpy(tmpsession.framed_ipv6_pool, 
                "2003:4:1::/64", MAX_FRAMED_IPV6_ATTR_LEN - 1);
        memcpy(tmpsession.delegated_ipv6_prefix, 
                tmpsession.framed_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

		mac2str(tmpsession.apmac, new_apmacstr, sizeof(new_apmacstr), ':');

		eag_log_info("Receive WID_ADD msg usermac:%s, userip:%s, status:%s,"
			" from apmac:%s, apname:%s, ssid:%s to apmac:%s, apname:%s, ssid:%s",
			user_macstr, user_ipstr,
			APPCONN_STATUS_AUTHED == appconn->session.state?
				"Authed":"NotAuthed",
			ap_macstr, appconn->session.apname, appconn->session.essid,
			new_apmacstr, tmpsession.apname, tmpsession.essid);

		if (0 != strcmp(tmpsession.essid, appconn->session.essid)) {
			if (macauth_switch) {
				del_eag_preauth_by_ip_or_mac(stamsg->macauth, user_addr, usermac);
			}
			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				appconn->session.session_stop_time = timenow;
				eag_portal_notify_logout_nowait(stamsg->portal, appconn);
				terminate_appconn(appconn, stamsg->eagins, 
						RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
			} else {
				appconn_del_from_db(appconn);
				appconn_free(appconn);
			}
		} else {   /* essid not changed */
			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				if (EAG_AUTH_TYPE_PORTAL == appconn->session.server_auth_type) {
        			eag_bss_message_count(stamsg->eagstat, appconn, BSS_USER_CONNECTED_TOTAL_TIME, 
                        	(timenow - appconn->session.last_connect_ap_time));
    			} else {
        			eag_bss_message_count(stamsg->eagstat, appconn, BSS_MACAUTH_USER_CONNECTED_TOTAL_TIME, 
                        	(timenow - appconn->session.last_connect_ap_time));
    			}
			}
			appconn->session.last_connect_ap_time = timenow;
			appconn->session.wlanid = tmpsession.wlanid;
			appconn->session.g_radioid = tmpsession.g_radioid;
			appconn->session.radioid = tmpsession.radioid;
			appconn->session.wtpid= tmpsession.wtpid;
			strncpy(appconn->session.essid, tmpsession.essid,
								sizeof(appconn->session.essid)-1);
			strncpy(appconn->session.apname, tmpsession.apname,
								sizeof(appconn->session.apname)-1);
			memcpy(appconn->session.apmac, tmpsession.apmac, 
							sizeof(appconn->session.apmac));
			appconn->session.vlanid = tmpsession.vlanid;

            appconn->session.framed_interface_id = tmpsession.framed_interface_id;
            memcpy(appconn->session.framed_ipv6_prefix, 
            		tmpsession.framed_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);
            appconn->session.login_ipv6_host = tmpsession.login_ipv6_host;
            strncpy(appconn->session.framed_ipv6_route, 
            		tmpsession.framed_ipv6_route, MAX_FRAMED_IPV6_ATTR_LEN - 1);
            strncpy(appconn->session.framed_ipv6_pool, 
            		tmpsession.framed_ipv6_pool, MAX_FRAMED_IPV6_ATTR_LEN - 1);
            memcpy(appconn->session.delegated_ipv6_prefix, 
            		tmpsession.delegated_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

			appconn_set_nasid(appconn, stamsg->nasidconf);
			appconn_set_nasportid(appconn, stamsg->nasportidconf);
			ret = appconn_config_portalsrv(appconn, stamsg->portalconf);
			if (0 != ret) {
				eag_log_warning("stamsg_proc "
					"appconn_config_portalsrv failed, usermac:%s ret=%d",
					user_macstr, ret);
			}

			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				notice_to_asd = eag_ins_get_notice_to_asd(stamsg->eagins);
				eag_log_info("stamsg_proc, appconn authed wlanid=%d, notice_to_asd switch %s", 
				appconn->session.wlanid, (1 == notice_to_asd)?"on":"off");
				if (appconn->session.wlanid > 0) {      
					eag_stamsg_send(stamsg, &(appconn->session), EAG_AUTH, notice_to_asd);
				}
				eag_ins_syn_user(stamsg->eagins, appconn);
			}
		}
		break;		
	case WID_DEL:
		if (0 == memcmp_ipx(user_addr, NULL)) {
			eag_log_warning("stamsg_proc receive WID_DEL, userip = 0");
		}

		if (macauth_switch) {
			del_eag_preauth_by_ip_or_mac(stamsg->macauth, user_addr, usermac);
		}
		mac2str(usermac, user_macstr, sizeof(user_macstr), ':');
	
		if (eag_hansi_is_enable(stamsg->eaghansi)
			&& !eag_hansi_is_master(stamsg->eaghansi))
		{
			eag_log_info("receive WID_DEL usermac=%s, but hansi is backup, ignore it",
				user_macstr);
			return 0;
		}
		
		appconn = appconn_find_by_usermac(stamsg->appdb, usermac);
		if (NULL == appconn) {
			eag_log_info("stamsg_proc, appconn not exist, usermac=%s",
				user_macstr);
			return 0;
		}

		mac2str(appconn->session.apmac, ap_macstr, sizeof(ap_macstr), ':');
		ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));

		appconn->session.sta_state = SESSION_STA_STATUS_UNCONNECT;
		appconn->session.leave_reason = sta_msg->STA.reason;
		
		eag_log_info("Receive leave msg usermac:%s, userip:%s, status:%s,"
			" apmac:%s, apname:%s, ssid:%s, leave_reason:%d",
			user_macstr, user_ipstr,
			APPCONN_STATUS_AUTHED == appconn->session.state?
				"Authed":"NotAuthed",
			ap_macstr, appconn->session.apname, appconn->session.essid, 
			appconn->session.leave_reason);

		if (macauth_switch) {
			del_eag_preauth_by_ip_or_mac(stamsg->macauth, &(appconn->session.user_addr), usermac);
		}
		
		if (APPCONN_STATUS_AUTHED == appconn->session.state) {
#if 0
			if (SESSION_STA_LEAVE_NORMAL == sta_msg->STA.reason) {
				eag_log_debug("eag_stamsg", "stamsg_proc receive WID_DEL"
					" and user(%s) leave normal", user_ipstr);
				appconn->session.session_stop_time = timenow;
				eag_portal_notify_logout_nowait(stamsg->portal, appconn);
				terminate_appconn(appconn, stamsg->eagins,
					RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
			} else {
				eag_log_debug("eag_stamsg", "stamsg_proc receive WID_DEL"
					" and user(%s) leave abnormal(%u)", user_ipstr, sta_msg->STA.reason);
			}
#endif			
		} else {
			appconn_del_from_db(appconn);
			appconn_free(appconn);
		}
		break;	
	case OPEN_ROAM:
		/* STAMSG_ROAM */
		if (0 == memcmp_ipx(user_addr, NULL)) {
			eag_log_warning("stamsg_proc receive OPEN_ROAM, userip = 0");
		}

		/* TODO: if essid changed, del mac_preauth */
		mac2str(usermac, user_macstr, sizeof(user_macstr), ':');

		if (eag_hansi_is_enable(stamsg->eaghansi)
			&& !eag_hansi_is_master(stamsg->eaghansi))
		{
			eag_log_info("receive OPEN_ROAM usermac=%s, but hansi is backup, ignore it",
				user_macstr);
			return 0;
		}
		
		appconn = appconn_find_by_usermac(stamsg->appdb, usermac);
		if (NULL == appconn) {
			eag_log_info("stamsg_proc, appconn not exist, usermac=%s",
				user_macstr);
			return 0;
		}
		appconn->session.sta_state = SESSION_STA_STATUS_CONNECT;
		
		mac2str(appconn->session.apmac, ap_macstr, sizeof(ap_macstr), ':');
		ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));
		
		tmpsession.idle_check = 1;
		appconn_db_get_idle_params(appconn->appdb, &(tmpsession.idle_timeout), &(tmpsession.idle_flow));
		ret = eag_get_sta_info_by_mac_v2(stamsg->eagdbus, stamsg->hansi_type,
					stamsg->hansi_id, usermac, &tmpsession, &security_type,
					eag_ins_get_notice_to_asd(stamsg->eagins));
		if (0 != ret) {
			eag_log_err("stamsg_proc, get_sta_info_by_mac_v2 failed,"
				" usermac:%s ret=%d", user_macstr, ret);
			return -1;
		}

		mac2str(tmpsession.apmac, new_apmacstr, sizeof(new_apmacstr), ':');

		eag_log_info("Receive roam msg usermac:%s, userip:%s, status:%s,"
			" from apmac:%s, apname:%s, ssid:%s to apmac:%s, apname:%s, ssid:%s",
			user_macstr, user_ipstr,
			APPCONN_STATUS_AUTHED == appconn->session.state?
				"Authed":"NotAuthed",
			ap_macstr, appconn->session.apname, appconn->session.essid,
			new_apmacstr, tmpsession.apname, tmpsession.essid);

		if (0 != strcmp(tmpsession.essid, appconn->session.essid)) {
			if (macauth_switch) {
				del_eag_preauth_by_ip_or_mac(stamsg->macauth, user_addr, usermac);
			}
			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				appconn->session.session_stop_time = timenow;
				eag_portal_notify_logout_nowait(stamsg->portal, appconn);
				terminate_appconn(appconn, stamsg->eagins, 
						RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
			} else {
				appconn_del_from_db(appconn);
				appconn_free(appconn);
			}
		} else {   /* essid not changed */
			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				if (EAG_AUTH_TYPE_PORTAL == appconn->session.server_auth_type) {
        			eag_bss_message_count(stamsg->eagstat, appconn, BSS_USER_CONNECTED_TOTAL_TIME, 
                        	(timenow - appconn->session.last_connect_ap_time));
    			} else {
        			eag_bss_message_count(stamsg->eagstat, appconn, BSS_MACAUTH_USER_CONNECTED_TOTAL_TIME, 
                        	(timenow - appconn->session.last_connect_ap_time));
    			}
			}
			appconn->session.last_connect_ap_time = timenow;
			appconn->session.wlanid = tmpsession.wlanid;
			appconn->session.g_radioid = tmpsession.g_radioid;
			appconn->session.radioid = tmpsession.radioid;
			appconn->session.wtpid= tmpsession.wtpid;
			strncpy(appconn->session.essid, tmpsession.essid,
								sizeof(appconn->session.essid)-1);
			strncpy(appconn->session.apname, tmpsession.apname,
								sizeof(appconn->session.apname)-1);
			memcpy(appconn->session.apmac, tmpsession.apmac, 
							sizeof(appconn->session.apmac));
			appconn->session.vlanid = tmpsession.vlanid;

			appconn_set_nasid(appconn, stamsg->nasidconf);
			appconn_set_nasportid(appconn, stamsg->nasportidconf);
			ret = appconn_config_portalsrv(appconn, stamsg->portalconf);
			if (0 != ret) {
				eag_log_warning("stamsg_proc "
					"appconn_config_portalsrv failed, usermac:%s ret=%d",
					user_macstr, ret);
			}

			if (APPCONN_STATUS_AUTHED == appconn->session.state) {
				eag_ins_syn_user(stamsg->eagins, appconn);
			}
		}
		break;
	default:
		eag_log_err("stamsg_proc unexpected stamsg type %u", sta_msg->Op);
		break;
	}

	return EAG_RETURN_OK;
}

static int
stamsg_receive(eag_thread_t *thread)
{
	eag_stamsg_t *stamsg = NULL;
	struct sockaddr_un addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	EagMsg sta_msg = {0};
	uint8_t usermac[6] = {0};
	user_addr_t user_addr = {0};
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	
	if (NULL == thread) {
		eag_log_err("stamsg_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	stamsg = eag_thread_get_arg(thread);
	if (NULL == stamsg) {
		eag_log_err("stamsg_receive stamsg null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(stamsg->sockfd, &sta_msg, sizeof(EagMsg), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("stamsg_receive recvfrom failed: %s, fd(%d)",
			safe_strerror(errno), stamsg->sockfd);
		return EAG_ERR_SOCKET_RECV_FAILED;
	}
	
	addr.sun_path[sizeof(addr.sun_path)-1] = '\0'; 
	eag_log_debug("eag_stamsg", "stamsg fd(%d) receive %d bytes from sockpath(%s)",
		stamsg->sockfd,
		nbyte,
		addr.sun_path);
	
	if (nbyte < sizeof(EagMsg)) {
		eag_log_warning("stamsg_receive msg size %d < EagMsg size %d",
			nbyte, sizeof(EagMsg));
		return -1;
	}
	if (EAG_TYPE != sta_msg.Type) {
		eag_log_warning("stamsg receive unexpected EagMsg Type:%d",
			sta_msg.Type);
		return -1;
	}
	if (WID_ADD != sta_msg.Op && WID_DEL != sta_msg.Op && OPEN_ROAM != sta_msg.Op 
		&& ASD_AUTH != sta_msg.Op && ASD_DEL_AUTH != sta_msg.Op) {
		eag_log_warning("stamsg receive unexpected EagMsg Op:%d",
			sta_msg.Op);
		return -1;
	}

	memcpy(usermac, sta_msg.STA.addr, sizeof(sta_msg.STA.addr));
	user_addr.family = EAG_IPV4;
	user_addr.user_ip = sta_msg.STA.ipaddr;
	ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
	mac2str(usermac, user_macstr, sizeof(user_macstr), ':');
	
	eag_log_debug("eag_stamsg",
		"stamsg receive EagMsg tm.Op=%d, userip=%s, usermac=%s",
		sta_msg.Op, user_ipstr, user_macstr);

	stamsg_proc(stamsg, usermac, &user_addr, &sta_msg);

	return EAG_RETURN_OK;
}

/*notify ASD  the user authorize state*/
int
eag_stamsg_send(eag_stamsg_t *stamsg,
		struct appsession *session,
		Operate Op, int notice_to_asd)
{
	EagMsg sta_msg = {0};
	struct sockaddr_un addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	char user_ipstr[IPX_LEN] = "";
	char macstr[32] = "";
	
	if (NULL == stamsg || NULL == session) {
		eag_log_err("eag_stamsg_send input error");
		return EAG_ERR_NULL_POINTER;
	}

	memset(&sta_msg, 0, sizeof(sta_msg));
	sta_msg.Type = EAG_TYPE;
	sta_msg.Op = Op;
	sta_msg.STA.wtp_id = session->wtpid;
	sta_msg.STA.ipaddr = session->user_addr.user_ip;
	sta_msg.STA.ip6_addr = session->user_addr.user_ipv6;
	memcpy(sta_msg.STA.addr, session->usermac, sizeof(sta_msg.STA.addr));
	strncpy(sta_msg.STA.arpifname, session->intf, sizeof(sta_msg.STA.arpifname)-1);
	/* for REQUIREMENTS-524 */
	sta_msg.STA.portal_info_switch = notice_to_asd;
	sta_msg.STA.portal_info.portal_ip = session->inv_portal_ip;
	memcpy(sta_msg.STA.portal_info.portal_mac, session->inv_portal_mac, sizeof(sta_msg.STA.portal_info.portal_mac));

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, stamsg->ntf_asd_path, sizeof(addr.sun_path)-1);
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = addr.sun_len = SUN_LEN(&addr);
#else
	len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */
	mac2str(session->usermac, macstr, sizeof(macstr), ':');
	ipx2str(&(session->user_addr), user_ipstr, sizeof(user_ipstr));
	eag_log_info("stamsg send sockpath:%s, userip:%s, usermac:%s, Op:%d",
			addr.sun_path, user_ipstr, macstr, Op);
	if (notice_to_asd) {
		ip2str(session->inv_portal_ip, user_ipstr, sizeof(user_ipstr));
		mac2str(session->inv_portal_mac, macstr, sizeof(macstr), ':');
		eag_log_info("stamsg send inv_portal_ip:%s, inv_portal_mac:%s", user_ipstr, macstr);
	}
	nbyte = sendto(stamsg->sockfd, &sta_msg, sizeof(EagMsg), MSG_DONTWAIT,
					(struct sockaddr *)(&addr), len);
	if (nbyte < 0) {
		eag_log_err("eag_stamsg_send sendto failed, fd(%d), path(%s), %s",
			stamsg->sockfd, addr.sun_path, safe_strerror(errno));
		return -1;
	}
	if (nbyte != sizeof(sta_msg)) {
		eag_log_err("eag_stamsg_send sendto failed, nbyte(%d)!=sizeof(tm)(%d)",
			nbyte, sizeof(sta_msg));
		return -1;
	}

	return 0;
}

int
eag_stamsg_set_thread_master(eag_stamsg_t *stamsg,
		eag_thread_master_t *master)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_thread_master input error");
		return -1;
	}

	stamsg->master = master;

	return 0;
}

int
eag_stamsg_set_eagins(eag_stamsg_t *stamsg,
		eag_ins_t *eagins)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_eagins input error");
		return -1;
	}

	stamsg->eagins = eagins;

	return 0;
}

int
eag_stamsg_set_portal(eag_stamsg_t *stamsg,
		eag_portal_t *portal)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_portal input error");
		return -1;
	}

	stamsg->portal = portal;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_eagdbus(eag_stamsg_t *stamsg,
		eag_dbus_t *eagdbus)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_eagdbus input error");
		return -1;
	}

	stamsg->eagdbus = eagdbus;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_appdb(eag_stamsg_t *stamsg,
		appconn_db_t *appdb)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_appdb input error");
		return -1;
	}

	stamsg->appdb = appdb;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_captive(eag_stamsg_t *stamsg,
		eag_captive_t *captive)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_captive input error");
		return -1;
	}

	stamsg->captive = captive;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_macauth(eag_stamsg_t *stamsg,
		eag_macauth_t *macauth)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_macauth input error");
		return -1;
	}

	stamsg->macauth = macauth;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_portal_conf(eag_stamsg_t *stamsg,
		struct portal_conf *portalconf)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_portal_conf input error");
		return -1;
	}

	stamsg->portalconf = portalconf;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_nasid_conf(eag_stamsg_t *stamsg,
		struct nasid_conf *nasidconf)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_nasid_conf input error");
		return -1;
	}

	stamsg->nasidconf = nasidconf;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_nasportid_conf(eag_stamsg_t *stamsg,
		struct nasportid_conf *nasportidconf)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_nasportid_conf input error");
		return -1;
	}

	stamsg->nasportidconf = nasportidconf;

	return EAG_RETURN_OK;
}

int
eag_stamsg_set_eagstat(eag_stamsg_t *stamsg,
		eag_statistics_t *eagstat)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_eagstat input error");
		return -1;
	}

	stamsg->eagstat = eagstat;

	return EAG_RETURN_OK;

}

int
eag_stamsg_set_eaghansi(eag_stamsg_t *stamsg,
		eag_hansi_t *eaghansi)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_set_eaghansi input error");
		return -1;
	}

	stamsg->eaghansi = eaghansi;

	return EAG_RETURN_OK;

}

static void
eag_stamsg_event(eag_stamsg_event_t event,
		eag_stamsg_t *stamsg)
{
	if (NULL == stamsg) {
		eag_log_err("eag_stamsg_event input error");
		return;
	}

	switch (event) {
	case EAG_STAMSG_READ:
		stamsg->t_read =
		    eag_thread_add_read(stamsg->master, stamsg_receive,
					stamsg, stamsg->sockfd);
		break;
	default:
		break;
	}
}

