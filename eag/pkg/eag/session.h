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
* session.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* appconn
*
*
*******************************************************************************/

#ifndef _SESSION_H
#define _SESSION_H

#include <stdint.h>
#include "eag_conf.h"
#include "limits2.h"
#include "eag_time.h"
#include "eag_errcode.h"
#include "eag_log.h"
#include <string.h>
#include <arpa/inet.h>

#define SESSION_STA_STATUS_CONNECT	0
#define SESSION_STA_STATUS_UNCONNECT	1

#define SESSION_STA_LEAVE_ABNORMAL	0
#define SESSION_STA_LEAVE_NORMAL	1

#define EAG_IPV6_LEN (sizeof(struct in6_addr))

struct wireless_data_t {
	uint64_t init_input_octets;
	uint32_t init_input_packets;
	uint64_t init_output_octets;
	uint32_t init_output_packets;

	uint64_t fixed_input_octets;
	uint32_t fixed_input_packets;
	uint64_t fixed_output_octets;
	uint32_t fixed_output_packets;

	uint64_t last_input_octets;
	uint32_t last_input_packets;
	uint64_t last_output_octets;
	uint32_t last_output_packets;

	uint64_t cur_input_octets;
	uint32_t cur_input_packets;
	uint64_t cur_output_octets;
	uint32_t cur_output_packets;
};

struct iptables_data_t {
	uint32_t input_packets;
	uint64_t input_octets;
	uint32_t output_packets;
	uint64_t output_octets;
};

struct fastfwd_data_t {
	uint32_t input_packets;
	uint64_t input_octets;
	uint32_t output_packets;
	uint64_t output_octets;

	uint32_t ipv6_input_packets;
	uint64_t ipv6_input_octets;
	uint32_t ipv6_output_packets;
	uint64_t ipv6_output_octets;
};

typedef struct user_addr {
	uint8_t family;
	uint32_t user_ip;
	struct in6_addr user_ipv6;
} user_addr_t;

struct appsession {
	int state;
	uint32_t index;
	uint32_t ipv6_index;
	
	int sta_state;
	int leave_reason;

	user_addr_t user_addr;
	uint8_t usermac[PKT_ETH_ALEN];
	uint8_t apmac[PKT_ETH_ALEN];
	char apname[MAX_APNAME_LENGTH];
	char essid[MAX_ESSID_LENGTH];
	uint8_t wlanid;
	uint8_t radioid;
	uint32_t g_radioid;
	uint32_t wtpid;
	uint32_t vlanid;
	
	char intf[MAX_IF_NAME_LEN];
	uint32_t virtual_ip;
	uint32_t nasip;
	struct in6_addr nasipv6;

	char username[USERNAMESIZE];
	char domain_name[MAX_RADIUS_DOMAIN_LEN];
	char class_attr[RADIUS_CLASS_ATTR_SIZE];
	uint8_t class_attr_len;
	char passwd[USERNAMESIZE];
	uint8_t challenge[PORTAL_CHALLEN];
	uint16_t chapid;
	uint8_t chappasswd[PORTAL_CHALLEN];

	char sessionid[MAX_SESSION_ID_LEN];
	char nasid[RADIUS_MAX_NASID_LEN];
	char nas_port_id[MAX_NASPORTID_LEN];

	unsigned long idle_timeout;
	unsigned long interim_interval;
	unsigned long sessiontimeout;
	uint32_t bandwidthmaxdown;
	uint32_t bandwidthmaxup;
	uint64_t maxinputoctets;
	uint64_t maxoutputoctets;
	uint64_t maxtotaloctets;

	uint64_t input_octets;
	uint32_t input_packets;
	uint64_t output_octets;
	uint32_t output_packets;

	uint64_t ipv6_input_octets;
	uint32_t ipv6_input_packets;
	uint64_t ipv6_output_octets;
	uint32_t ipv6_output_packets;

    /* add for ipv6 radius rfc3162, 2013-1-7 */
	uint64_t framed_interface_id;
	uint8_t framed_ipv6_prefix[MAX_FRAMED_IPV6_PREFIX_LEN];
	struct in6_addr login_ipv6_host;
	char framed_ipv6_route[MAX_FRAMED_IPV6_ATTR_LEN];
	char framed_ipv6_pool[MAX_FRAMED_IPV6_ATTR_LEN];
	uint8_t delegated_ipv6_prefix[MAX_FRAMED_IPV6_PREFIX_LEN];

	struct wireless_data_t wireless_data;

	unsigned long sessionterminatetime;
	
	int terminate_cause;

	struct radius_srv_t radius_srv;

	uint64_t last_idle_check_output_octets;
	uint64_t last_idle_check_octets;
	uint64_t idle_flow;
	uint16_t idle_check;
	
	time_t session_start_time;
	time_t accurate_start_time;
	time_t session_stop_time;
	time_t last_acct_time;
	time_t last_flux_time;
	time_t last_connect_ap_time;

	time_t last_redir_check_time;
	uint32_t redir_count;
	time_t last_fastfwd_flow_time;
	int server_auth_type;
};

static inline void
appsession_copy(struct appsession *dst, struct appsession *src, struct timeval *cb_tv)
{	
	time_t time_now = 0;
	struct timeval tv;
	unsigned long last_syn_time = 0;

	if (NULL == dst || NULL == src || NULL == cb_tv) {
		eag_log_warning("appsession_copy some parameter is NULL");
		return;
	}

	memcpy(dst, src, sizeof(struct appsession));
	eag_log_debug("eag_hansi","before appsession_copy "
			"session_start_time(%lu), session_stop_time(%lu), last_flux_time(%lu),"
			"last_acct_time(%lu), last_fastfwd_flow_time(%lu)",src->session_start_time,
			src->session_stop_time,src->last_flux_time,src->last_acct_time,
			src->last_fastfwd_flow_time);

	/* time_proc */
	eag_time_gettimeofday(&tv, NULL);
	time_now = tv.tv_sec;
	last_syn_time = cb_tv->tv_sec;
	if (0 != last_syn_time) {
			dst->last_acct_time = tv.tv_sec - (last_syn_time - dst->last_acct_time);
			dst->last_flux_time = tv.tv_sec - (last_syn_time - dst->last_flux_time);
			dst->session_start_time = tv.tv_sec - (last_syn_time - dst->session_start_time);
			if (0!= dst->last_fastfwd_flow_time) {
				dst->last_fastfwd_flow_time = tv.tv_sec - (last_syn_time - dst->last_fastfwd_flow_time);
			}
	}
	eag_log_debug("eag_hansi","after appsession_copy "
			"session_start_time(%lu), session_stop_time(%lu), last_flux_time(%lu),"
			"last_acct_time(%lu), last_fastfwd_flow_time(%lu)",dst->session_start_time,
			dst->session_stop_time,dst->last_flux_time,dst->last_acct_time,
			dst->last_fastfwd_flow_time);
	eag_log_debug("eag_hansi","appsession_copy last_syn_time=%lu, back timenow=%lu",last_syn_time,tv.tv_sec);

	return;
}

#endif

