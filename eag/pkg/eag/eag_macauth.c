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
* eag_macauth.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* eag mac auth
*
*
*******************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/tipc.h>
#include <iptables.h>
#include <ip6tables.h>

#include "nm_list.h"
#include "hashtable.h"
#include "nmp_process.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "eag_dbus.h"
#include "eag_util.h"
#include "eag_ipinfo.h"
#include "eag_time.h"
#include "eag_macauth.h"
#include "eag_captive.h"
#include "eag_wireless.h"
#include "appconn.h"
#include "eag_ins.h"
#include "eag_fastfwd.h"
#include "eag_portal.h"
#include "radius_packet.h"
#include "session.h"

#define MACAUTH_SERVER_TYPE			0x4000
#define MACAUTH_SERVER_INSTANCE		0x10000

#define EAG_MAC_PREAUTH_HASHSIZE			1021

#define EAG_MAC_PREAUTH_BLKMEM_NAME 		"mac_preauth_blkmem"
#define EAG_MAC_PREAUTH_BLKMEM_ITEMNUM		1024
#define EAG_MAC_PREAUTH_BLKMEM_MAXNUM		32

extern nmp_mutex_t eag_iptables_lock;
extern nmp_mutex_t eag_ip6tables_lock;
extern int cmtest_no_arp_learn;

struct eag_macauth {
	int sockfd;
	int serv_type;
	int serv_instance;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	
	struct list_head head;
	hashtable *htable;
	hashtable *ipv6_htable;
	hashtable *mac_htable;
	eag_blk_mem_t *blkmem;

	uint8_t hansi_type;
	uint8_t hansi_id;
	int macauth_switch;
	int flux_from;
	int flux_interval;
	int flux_threshold;
	int check_interval;

	eag_ins_t *eagins;
	eag_dbus_t *eagdbus;
	appconn_db_t *appdb;
	eag_captive_t *captive;
	eag_fastfwd_t *fastfwd;
	eag_portal_t *portal;
	struct portal_conf *portalconf;
};

struct macauth_wireless_data_t {
	uint64_t init_input_octets;
	uint64_t init_output_octets;

	uint64_t fixed_input_octets;
	uint64_t fixed_output_octets;

	uint64_t last_input_octets;
	uint64_t last_output_octets;

	uint64_t cur_input_octets;
	uint64_t cur_output_octets;
};

struct mac_preauth_t {
	struct list_head node;
	struct hlist_node hnode;
	struct hlist_node ipv6_hnode;
	struct hlist_node mac_hnode;
	user_addr_t user_addr;
	uint8_t usermac[6];
	eag_macauth_t *macauth;

	uint64_t input_octets;
	uint64_t output_octets;
	/* iptables */
	uint64_t iptables_input_octets;
	uint64_t iptables_output_octets;
	/* ip6tables */
	uint64_t ip6tables_input_octets;
	uint64_t ip6tables_output_octets;
	/* wireless */
	struct macauth_wireless_data_t wireless_data;
	/* fastfwd */
	uint64_t fastfwd_input_octets;
	uint64_t fastfwd_output_octets;
	time_t macbind_req_time;
	time_t last_fastfwd_flux_time;
	time_t last_invalid_check_time;

	uint64_t last_input_octets;
	uint64_t last_output_octets;
	time_t last_flux_time;
};

typedef struct dhcp_sta_msg {
    uint8_t family;
	uint8_t usermac[6];
    union {
		uint32_t userip;
		struct in6_addr user_ipv6;
    } addr;
}dhcp_sta_msg_t;

typedef enum {
	EAG_MACAUTH_READ,
} eag_macauth_event_t;

static void
eag_macauth_event(eag_macauth_event_t event,
		eag_macauth_t *macauth);

static struct mac_preauth_t *
mac_preauth_new(eag_macauth_t *macauth,
		user_addr_t *user_addr, uint8_t usermac[6])
{
	struct mac_preauth_t *preauth = NULL;
	char user_macstr[32] = "";
	struct timeval tv = {0};
	char user_ipstr[IPX_LEN] = "";

	preauth = eag_blkmem_malloc_item(macauth->blkmem);
	if (NULL == preauth) {
		eag_log_err("mac_preauth_new blkmem_malloc_item failed");
		return NULL;
	}

	eag_time_gettimeofday(&tv, NULL);
	mac2str(usermac, user_macstr, sizeof(user_macstr), '-');
	memset(preauth, 0, sizeof(struct mac_preauth_t));
	memcpy(&(preauth->user_addr), user_addr, sizeof(user_addr_t));
	memcpy(preauth->usermac, usermac, sizeof(preauth->usermac));
	preauth->macauth = macauth;
	list_add(&(preauth->node), &(macauth->head));
	if (EAG_IPV6 == user_addr->family) {
		hashtable_check_add_node(macauth->ipv6_htable, 
								&(user_addr->user_ipv6), 
								sizeof(struct in6_addr), 
								&(preauth->ipv6_hnode));
	} else if (EAG_IPV4 == user_addr->family) {
		hashtable_check_add_node(macauth->htable, 
								&(user_addr->user_ip), 
								sizeof(struct in_addr), 
								&(preauth->hnode));
	} else if (EAG_MIX == user_addr->family) {
        hashtable_check_add_node(macauth->ipv6_htable, 
                                &(user_addr->user_ipv6), 
                                sizeof(struct in6_addr), 
                                &(preauth->ipv6_hnode));
		hashtable_check_add_node(macauth->htable, 
								&(user_addr->user_ip), 
								sizeof(struct in_addr), 
								&(preauth->hnode));
	}
	hashtable_check_add_node(macauth->mac_htable, usermac, 6, 
						&(preauth->mac_hnode));
	preauth->last_flux_time = tv.tv_sec;
	
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	eag_log_debug("eag_macauth", "mac_preauth_new userip=%s usermac=%s", user_ipstr, user_macstr);
	return preauth;
}

static int
mac_preauth_free(struct mac_preauth_t *preauth)
{
	eag_macauth_t *macauth = NULL;
	char user_ipstr[IPX_LEN] = "";

	ipx2str(&(preauth->user_addr), user_ipstr, sizeof(user_ipstr));
	if (NULL == preauth) {
		eag_log_err("mac_preauth_free input error");
		return -1;
	}
	macauth = preauth->macauth;

	eag_log_debug("eag_macauth", "mac_preauth_free userip=%s", 
			user_ipstr);
	
	list_del(&(preauth->node));
	hlist_del(&(preauth->hnode));
	hlist_del(&(preauth->mac_hnode));
	eag_blkmem_free_item(macauth->blkmem, preauth);
	
	return EAG_RETURN_OK;
}

static struct mac_preauth_t *
mac_preauth_find_by_userip(eag_macauth_t *macauth,
		user_addr_t *user_addr)
{
	struct mac_preauth_t *preauth = NULL;
	struct hlist_head *head = NULL;
	struct hlist_node *node = NULL;
	char user_ipstr[IPX_LEN] = "";

	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	if (EAG_IPV6 == user_addr->family) {
        head = hashtable_get_hash_list( macauth->ipv6_htable, 
							            &(user_addr->user_ipv6),
							            sizeof(struct in6_addr) );
        if (NULL == head) {
            eag_log_err("mac_preauth_find_by_userip head is null");
            return NULL;
        }
        
        hlist_for_each_entry(preauth, node, head, ipv6_hnode) {
            if (!memcmp_ipx(user_addr, &(preauth->user_addr))) {
                eag_log_debug("eag_macauth", "found mac_preauth by userip %s", 
                    user_ipstr);
                return preauth;
            }
        }
	} else {
        head = hashtable_get_hash_list( macauth->htable, 
										&(user_addr->user_ip),
							            sizeof(struct in_addr) );
		if (NULL == head) {
			eag_log_err("mac_preauth_find_by_userip head is null");
			return NULL;
		}
	
		hlist_for_each_entry(preauth, node, head, hnode) {
			if (!memcmp_ipx(user_addr, &(preauth->user_addr))) {
				eag_log_debug("eag_macauth", "found mac_preauth by userip %s", 
					user_ipstr);
				return preauth;
			}
		}
	}

	eag_log_debug("eag_macauth", "not found mac_preauth by userip %s", 
				user_ipstr);
	return NULL;
}

static struct mac_preauth_t *
mac_preauth_find_by_usermac(eag_macauth_t *macauth,
		const uint8_t *usermac)
{	
	struct mac_preauth_t *preauth = NULL;
	struct hlist_head *head = NULL;
	struct hlist_node *node = NULL;
	char macstr[32] = "";
	
	mac2str(usermac, macstr, sizeof(macstr), '-');	
	head = hashtable_get_hash_list(macauth->mac_htable, usermac, 6);
	if (NULL == head) {
		eag_log_err("mac_preauth_find_by_usermac "
			"hashtable_get_hash_list failed, usermac %s", macstr);
		return NULL;
	}

	hlist_for_each_entry(preauth, node, head, mac_hnode) {
		if (0 == memcmp(preauth->usermac, usermac,
						sizeof(preauth->usermac))) {
			return preauth;
		}
	}

	eag_log_debug("eag_macauth", "not found mac_preauth by usermac %s",
			macstr);
	return NULL;
}

eag_macauth_t *
eag_macauth_new(uint8_t hansi_type, uint8_t hansi_id)
{	
	eag_macauth_t *macauth = NULL;

	macauth = eag_malloc(sizeof(*macauth));
	if (NULL == macauth) {
		eag_log_err("eag_macauth_new eag_malloc failed");
		goto failed_0;
	}

	memset(macauth, 0, sizeof(*macauth));
	if (EAG_RETURN_OK != eag_blkmem_create(&(macauth->blkmem),
						EAG_MAC_PREAUTH_BLKMEM_NAME,
						sizeof(struct mac_preauth_t),
						EAG_MAC_PREAUTH_BLKMEM_ITEMNUM,
						EAG_MAC_PREAUTH_BLKMEM_MAXNUM)) {
		eag_log_err("eag_macauth_new blkmem_create failed");
		goto failed_1;
	}
	if (EAG_RETURN_OK != hashtable_create_table(&(macauth->htable),
							EAG_MAC_PREAUTH_HASHSIZE)) {
		eag_log_err("eag_macauth_new ip hashtable_create failed");
		goto failed_2;
	}
	if (EAG_RETURN_OK != hashtable_create_table(&(macauth->ipv6_htable),
							EAG_MAC_PREAUTH_HASHSIZE)) {
		eag_log_err("eag_macauth_new ip hashtable_create failed");
		goto failed_3;
	}
	if (EAG_RETURN_OK != hashtable_create_table(&(macauth->mac_htable),
							EAG_MAC_PREAUTH_HASHSIZE)) {
		eag_log_err("eag_macauth_new mac hashtable_create failed");
		goto failed_4;
	}
	INIT_LIST_HEAD(&(macauth->head));
	macauth->sockfd = -1;
	macauth->serv_type = MACAUTH_SERVER_TYPE;
	macauth->serv_instance = MACAUTH_SERVER_INSTANCE;
	macauth->hansi_type = hansi_type;
	macauth->hansi_id = hansi_id;
	macauth->macauth_switch = 0;
	macauth->flux_from = FLUX_FROM_FASTFWD;
	macauth->flux_interval = DEFAULT_MACAUTH_FLUX_INTERVAL;
	macauth->flux_threshold = DEFAULT_MACAUTH_FLUX_THRESHOLD;
	macauth->check_interval = DEFAULT_MACAUTH_CHECK_INTERVAL;
	eag_log_info("macauth new ok");
	return macauth;

failed_4:
	hashtable_destroy_table(&(macauth->ipv6_htable));
failed_3:
	hashtable_destroy_table(&(macauth->htable));
failed_2:
	eag_blkmem_destroy(&(macauth->blkmem));
failed_1:
	eag_free(macauth);
failed_0:
	return NULL;
}

int
eag_macauth_free(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_free input error");
		return -1;
	}

	if (NULL != macauth->blkmem) {
		eag_blkmem_destroy(&(macauth->blkmem));
	}
	if (NULL != macauth->htable) {
		hashtable_destroy_table(&(macauth->htable));
	}
	if (NULL != macauth->ipv6_htable) {
		hashtable_destroy_table(&(macauth->ipv6_htable));
	}
	if (NULL != macauth->mac_htable) {
		hashtable_destroy_table(&(macauth->mac_htable));
	}
	eag_free(macauth);

	eag_log_info("macauth free ok");
	return 0;

}

int 
eag_macauth_start(eag_macauth_t *macauth)
{
	int ret = 0;
	int len = 0;
	struct sockaddr_tipc addr = {0};
  
	if (NULL == macauth) {
		eag_log_err("eag_macauth_start input error");
		return EAG_ERR_NULL_POINTER;
	}

	if (macauth->sockfd >= 0) {
		eag_log_err("eag_macauth_start already start fd(%d)", 
			macauth->sockfd);
		return EAG_RETURN_OK;
	}

	macauth->sockfd = socket(AF_TIPC, SOCK_RDM, 0);
	if (macauth->sockfd  < 0) {
		eag_log_err("Can't create macauth tipc rdm socket: %s",
			safe_strerror(errno));
		macauth->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	if (0 != set_nonblocking(macauth->sockfd)){
		eag_log_err("eag_macauth_start set socket nonblocking failed");
		close(macauth->sockfd);
		macauth->sockfd = -1;
		return EAG_ERR_SOCKET_OPT_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_tipc));
	addr.family = AF_TIPC;
	addr.addrtype = TIPC_ADDR_NAME;
	addr.addr.name.name.type = MACAUTH_SERVER_TYPE;
	addr.addr.name.name.instance = MACAUTH_SERVER_INSTANCE;
	addr.scope = TIPC_CLUSTER_SCOPE;
	/* addr.scope = TIPC_NODE_SCOPE; */
	len = sizeof(addr);

	ret  = bind(macauth->sockfd, (struct sockaddr *)&addr, len);
	if (ret < 0) {
		eag_log_err("Can't bind to macauth socket(%d): %s",
			macauth->sockfd, safe_strerror(errno));
		close(macauth->sockfd);
		macauth->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}
	
	eag_macauth_event(EAG_MACAUTH_READ, macauth);
	
	eag_log_info("macauth fd(%d) start ok",
			macauth->sockfd);

	return EAG_RETURN_OK;	
}

int
eag_macauth_stop(eag_macauth_t *macauth)
{
	struct mac_preauth_t *preauth = NULL;
	struct mac_preauth_t *next = NULL;
	int flux_from = 0;
	
	if (NULL == macauth) {
		eag_log_err("eag_macauth_stop input error");
		return -1;
	}

	if (NULL != macauth->t_read) {
		eag_thread_cancel(macauth->t_read);
		macauth->t_read = NULL;
	}
	if (macauth->sockfd >= 0) {
		close(macauth->sockfd);
		macauth->sockfd = -1;
	}

	flux_from = eag_macauth_get_flux_from(macauth);
	list_for_each_entry_safe(preauth, next, &(macauth->head), node) {
		eag_captive_del_macpre_authorize(macauth->captive, &(preauth->user_addr));
		if (FLUX_FROM_FASTFWD == flux_from
			|| FLUX_FROM_FASTFWD_IPTABLES == flux_from) 
		{
			eag_fastfwd_send(macauth->fastfwd, &(preauth->user_addr), SE_AGENT_USER_OFFLINE);
		}
		mac_preauth_free(preauth);
	}
	
	eag_log_info("macauth stop ok");

	return EAG_RETURN_OK;

}

static int
macauth_proc(eag_macauth_t *macauth,
		uint8_t usermac[6],
		user_addr_t *user_addr)
{
	unsigned int security_type = 0;
	struct appsession session = {0};
	int ret = 0;
	//DBusConnection *dbus_conn = NULL;
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	struct app_conn_t *appconn = NULL;
	
	mac2str(usermac, user_macstr, sizeof(user_macstr), '-');

	if (0 == memcmp_ipx(user_addr, NULL)) {
		eag_log_warning("macauth_proc usermac=%s, but userip=0",
			user_macstr);
		return -1;
	}
	
	//dbus_conn = eag_dbus_get_dbus_conn(macauth->eagdbus);
	ret = eag_get_sta_info_by_mac_v2(macauth->eagdbus, macauth->hansi_type,
				macauth->hansi_id, usermac, &session, &security_type);
	if (0 != ret) {
		eag_log_warning("macauth_proc usermac %s not belong to hansi %u:%u, ret=%d",
			user_macstr, macauth->hansi_type, macauth->hansi_id, ret);
		return 0;
	}
	if ( 0 == memcmp_ipx(user_addr, &(session.user_addr)) ) {
		memcpy(user_addr, &(session.user_addr), sizeof(user_addr_t));
	}
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	eag_log_info("receive msg from dhcp-snooping, usermac=%s, userip=%s, and security_type=%d",
		user_macstr, user_ipstr, security_type);
	if (SECURITY_MAC_AUTH == security_type && macauth->macauth_switch) {
		appconn = appconn_find_by_userip(macauth->appdb, user_addr);
		if (NULL != appconn && APPCONN_STATUS_AUTHED == appconn->session.state) {
			eag_log_info("userip %s is already authed, not need mac-preauth", user_ipstr);
			return 0;
		}
		
		eag_add_mac_preauth(macauth, user_addr, usermac);
	} 
	#if 0
	else if (SECURITY_NO_NEED_AUTH == security_type) {
		eag_captive_eap_authorize(macauth->captive, userip);
		eag_log_info("macauth_proc add eap or none authorize user_ip %s", user_ipstr);
	}
	#endif
	return 0;
}

static int 
eag_macauth_receive(eag_thread_t *thread)
{
	eag_macauth_t *macauth = NULL;
	struct sockaddr_tipc addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	dhcp_sta_msg_t dhcpmsg = {0};
	uint8_t usermac[6] = {0};
	user_addr_t user_addr = {0};
	
	if (NULL == thread) {
		eag_log_err("eag_macauth_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	macauth = eag_thread_get_arg(thread);
	if (NULL == macauth) {
		eag_log_err("eag_macauth_receive macauth null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(macauth->sockfd, &dhcpmsg, sizeof(dhcp_sta_msg_t), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("eag_macauth_receive recvfrom failed: %s, fd(%d)",
			safe_strerror(errno), macauth->sockfd);
		return EAG_ERR_SOCKET_RECV_FAILED;
	}
	
	eag_log_debug("eag_macauth", "macauth fd(%d) receive %d bytes",
		macauth->sockfd, nbyte);
	
	if (nbyte < sizeof(dhcp_sta_msg_t)) {
		eag_log_warning("eag_macauth_receive msg length %d < msg size %d",
			nbyte, sizeof(dhcp_sta_msg_t));
		return -1;
	}
	
	memcpy(usermac, dhcpmsg.usermac, sizeof(usermac));
	memset(&user_addr, 0, sizeof(user_addr_t));
	if (EAG_IPV4 == dhcpmsg.family) {
        user_addr.user_ip = dhcpmsg.addr.userip;
		user_addr.family = EAG_IPV4;
	} else if (EAG_IPV6 == dhcpmsg.family) {
        user_addr.user_ipv6 = dhcpmsg.addr.user_ipv6;		
		user_addr.family = EAG_IPV6;
	} else {
		eag_log_err("eag_macauth_receive recvfrom msg error: family=EAG_IPV%d",
			dhcpmsg.family);
		return -1;
	}
	macauth_proc(macauth, usermac, &user_addr);

	return EAG_RETURN_OK;
}

int
set_mac_preauth_flux(struct mac_preauth_t *preauth)
{
	eag_macauth_t *macauth = NULL;
	int flux_from = 0;
	char *flux_from_str = NULL;
	char user_ipstr[IPX_LEN] = "";

	ipx2str(&(preauth->user_addr), user_ipstr, sizeof(user_ipstr));
	macauth = preauth->macauth;
	flux_from = eag_macauth_get_flux_from(macauth);

	switch(flux_from) {
	case FLUX_FROM_IPTABLES:
		preauth->input_octets = preauth->iptables_input_octets + preauth->ip6tables_input_octets;
		preauth->output_octets = preauth->iptables_output_octets + preauth->ip6tables_output_octets;
		flux_from_str = "iptables";
		break;
	case FLUX_FROM_IPTABLES_L2:
		flux_from_str = "iptables_L2";
		break;
	case FLUX_FROM_WIRELESS:
		if ( (0 != preauth->wireless_data.cur_input_octets
				&& preauth->wireless_data.cur_input_octets < preauth->wireless_data.last_input_octets)
			|| (0 != preauth->wireless_data.cur_output_octets
				&& preauth->wireless_data.cur_output_octets < preauth->wireless_data.last_output_octets))
		{
			preauth->wireless_data.fixed_input_octets += preauth->wireless_data.last_input_octets;
			preauth->wireless_data.fixed_output_octets += preauth->wireless_data.last_output_octets;
			eag_log_info("set_mac_preauth_flux by wireless userip=%s flux reduced", 
					user_ipstr);
		}
		if (preauth->wireless_data.fixed_input_octets + preauth->wireless_data.cur_input_octets
			> preauth->wireless_data.init_input_octets)
		{
			preauth->input_octets = preauth->wireless_data.fixed_input_octets
				+ preauth->wireless_data.cur_input_octets - preauth->wireless_data.init_input_octets;
		}
		if (preauth->wireless_data.fixed_output_octets + preauth->wireless_data.cur_output_octets
			> preauth->wireless_data.init_output_octets)
		{
			preauth->output_octets = preauth->wireless_data.fixed_output_octets
				+ preauth->wireless_data.cur_output_octets - preauth->wireless_data.init_output_octets;
		}
		preauth->wireless_data.last_input_octets = preauth->wireless_data.cur_input_octets;
		preauth->wireless_data.last_output_octets = preauth->wireless_data.cur_output_octets;
		flux_from_str = "wireless";
		break;
	case FLUX_FROM_FASTFWD:
		preauth->input_octets = preauth->fastfwd_input_octets;
		preauth->output_octets = preauth->fastfwd_output_octets;
		flux_from_str = "fastfwd";
		break;
	case FLUX_FROM_FASTFWD_IPTABLES:
		preauth->input_octets = preauth->fastfwd_input_octets + preauth->iptables_input_octets
			+ preauth->ip6tables_input_octets;
		preauth->output_octets = preauth->fastfwd_output_octets + preauth->iptables_output_octets
			+ preauth->ip6tables_output_octets;
		flux_from_str = "fastfwd_iptables";
		break;
	default:
		flux_from_str = "unknown";
		break;
	}

	eag_log_debug("eag_macauth", "set_mac_preauth_flux from %s: input_octets:%llu, output_octets:%llu",
				flux_from_str, preauth->input_octets, preauth->output_octets);
	return 0;
}

int
flush_all_preauth_flux_from_wireless(eag_macauth_t *macauth)
{	
	int ret = -1;
	DBusConnection *dbus_conn = NULL;
	struct WtpStaInfo *StaHead =NULL;
	struct WtpStaInfo *StaNode =NULL;
	struct mac_preauth_t *preauth = NULL;
	//uint32_t userip = 0;
	//char user_ipstr[32] = "";
	uint8_t usermac[6] = {0};

	dbus_conn = eag_dbus_get_dbus_conn(macauth->eagdbus);
	StaHead = eag_show_sta_info_of_all_wtp(macauth->hansi_id, 
				macauth->hansi_type, dbus_conn, &ret);
	
	if (NULL != StaHead && EAG_RETURN_OK == ret) {
		for (StaNode = StaHead; StaNode; StaNode=StaNode->next) {
			//userip = StaNode->wtpStaIp;
			//ip2str(userip, user_ipstr, sizeof(user_ipstr));
			str2mac(StaNode->wtpTerminalMacAddr, usermac);
			preauth = mac_preauth_find_by_usermac(macauth, usermac);
			if (NULL != preauth) {
				preauth->wireless_data.cur_input_octets = StaNode->wtpTerminalRecvByteMount;
				preauth->wireless_data.cur_output_octets = StaNode->wtpSendTerminalByteMount;
				set_mac_preauth_flux(preauth);
			}
		}
		eag_free_wtp_sta_info_head(StaHead);
	} else {
		eag_log_err("flush_all_preauth_flux_from_wireless StaHead=%p, ret=%d",
			StaHead, ret);
	}
	
	return EAG_RETURN_OK;
}

int
flush_all_preauth_flux_from_iptables (eag_macauth_t *macauth)
{
	struct iptc_handle *handle = NULL;
	char chain_name[128] = "";
	const struct ipt_entry *p_entry = NULL;
	struct ipt_counters *my_counter = NULL;
	unsigned int rule_num = 0;
	struct mac_preauth_t *preauth = NULL;
	user_addr_t user_addr = {0};
	char user_ipstr[IPX_LEN] = "";
		
	eag_log_debug("iptables_lock", "flush_all_preauth_flux_from_iptables iptables lock");
	nmp_mutex_lock(&eag_iptables_lock);
	handle = iptc_init("filter");
	eag_log_debug("iptables_lock", "flush_all_preauth_flux_from_iptables iptables unlock");
	nmp_mutex_unlock(&eag_iptables_lock);
	if (NULL == handle) {
		eag_log_err("flush_all_preauth_flux_from_iptables can't init iptc handle");
		return -1;
	}

	snprintf(chain_name, sizeof(chain_name), "MAC_PRE_AUTH_%c%d_F", 
		(HANSI_LOCAL == macauth->hansi_type)?'L':'R', macauth->hansi_id);
	if (!iptc_is_chain(chain_name, handle)) {
		eag_log_err("flush_all_preauth_flux_from_iptables chain %s not exist",
			chain_name);
		iptc_free(handle);
		return -1;
	}

	for (p_entry = iptc_first_rule(chain_name, handle);
		NULL != p_entry;
		p_entry = iptc_next_rule(p_entry, handle))
	{
		rule_num++;
		my_counter = iptc_read_counter(chain_name, rule_num, handle);
		if (p_entry->ip.dst.s_addr != 0 && p_entry->ip.src.s_addr == 0)
		{
			user_addr.family = EAG_IPV4;
			user_addr.user_ip = ntohl(p_entry->ip.dst.s_addr);
			ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
			preauth = mac_preauth_find_by_userip(macauth, &user_addr);
			if (NULL == preauth) {
				eag_log_err("flush_all_preauth_flux_from_iptables "
					"get mac-preauth ip(%s) from iptables, but not in preauth list",
					user_ipstr);
				continue;
			}
			preauth->iptables_input_octets = my_counter->bcnt;
			set_mac_preauth_flux(preauth);
		}
		else if (p_entry->ip.dst.s_addr == 0 && p_entry->ip.src.s_addr != 0)
		{
			user_addr.family = EAG_IPV4;
			user_addr.user_ip = ntohl(p_entry->ip.src.s_addr);
			ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
			preauth = mac_preauth_find_by_userip(macauth, &user_addr);
			if (NULL == preauth) {
				eag_log_err("flush_all_preauth_flux_from_iptables "
					"get mac-preauth ip(%s) from iptables, but not in preauth list",
					user_ipstr);
				continue;
			}
			preauth->iptables_output_octets = my_counter->bcnt;
			set_mac_preauth_flux(preauth);
		}
	}

	iptc_free(handle);
	return EAG_RETURN_OK;
}

int
flush_all_preauth_flux_from_ip6tables (eag_macauth_t *macauth)
{
	struct ip6tc_handle *handle = NULL;
	char chain_name[128] = "";
	const struct ip6t_entry *p_entry = NULL;
	struct ip6t_counters *my_counter = NULL;
	unsigned int rule_num = 0;
	struct mac_preauth_t *preauth = NULL;
	user_addr_t user_addr = {0};
	char user_ipstr[IPX_LEN] = "";
		
	eag_log_debug("ip6tables_lock", "flush_all_preauth_flux_from_ip6tables ip6tables lock");
	nmp_mutex_lock(&eag_ip6tables_lock);
	handle = ip6tc_init("filter");
	eag_log_debug("ip6tables_lock", "flush_all_preauth_flux_from_ip6tables ip6tables unlock");
	nmp_mutex_unlock(&eag_ip6tables_lock);
	if (NULL == handle) {
		eag_log_err("flush_all_preauth_flux_from_ip6tables can't init ip6tc handle");
		return -1;
	}

	snprintf(chain_name, sizeof(chain_name), "MAC_PRE_AUTH_%c%d_F", 
		(HANSI_LOCAL == macauth->hansi_type)?'L':'R', macauth->hansi_id);
	if (!ip6tc_is_chain(chain_name, handle)) {
		eag_log_err("flush_all_preauth_flux_from_ip6tables chain %s not exist",
			chain_name);
		ip6tc_free(handle);
		return -1;
	}

	for (p_entry = ip6tc_first_rule(chain_name, handle);
		NULL != p_entry;
		p_entry = ip6tc_next_rule(p_entry, handle))
	{
		rule_num++;
		my_counter = ip6tc_read_counter(chain_name, rule_num, handle);
		if ( 0 != ipv6_compare_null(&(p_entry->ipv6.dst))
            && 0 != ipv6_compare_null(&(p_entry->ipv6.src)) )
		{
			user_addr.family = EAG_IPV6;
			user_addr.user_ipv6 = p_entry->ipv6.dst;
			ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
			preauth = mac_preauth_find_by_userip(macauth, &user_addr);
			if (NULL == preauth) {
				eag_log_err("flush_all_preauth_flux_from_ip6tables "
					"get mac-preauth ip(%s) from ip6tables, but not in preauth list",
					user_ipstr);
				continue;
			}
			preauth->ip6tables_input_octets = my_counter->bcnt;
			set_mac_preauth_flux(preauth);
		}
		else if ( 0 != ipv6_compare_null(&(p_entry->ipv6.dst))
            && 0 != ipv6_compare_null(&(p_entry->ipv6.src)) )
		{
			user_addr.family = EAG_IPV6;
			user_addr.user_ipv6 = p_entry->ipv6.src;
			ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
			preauth = mac_preauth_find_by_userip(macauth, &user_addr);
			if (NULL == preauth) {
				eag_log_err("flush_all_preauth_flux_from_ip6tables "
					"get mac-preauth ip(%s) from ip6tables, but not in preauth list",
					user_ipstr);
				continue;
			}
			preauth->ip6tables_output_octets = my_counter->bcnt;
			set_mac_preauth_flux(preauth);
		}
	}

	ip6tc_free(handle);
	return EAG_RETURN_OK;
}

int
flush_all_preauth_flux_from_ipxtables (eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		return -1;
	}
    flush_all_preauth_flux_from_iptables (macauth);
    if (eag_ins_get_ipv6_switch(macauth->eagins)) {
		flush_all_preauth_flux_from_ip6tables (macauth);
	}

	return EAG_RETURN_OK;
}

int
flush_preauth_flux_from_fastfwd(eag_macauth_t *macauth,
		user_addr_t *user_addr,
		uint64_t fastfwd_input_octets,
		uint64_t fastfwd_output_octets)
{
	struct mac_preauth_t *preauth = NULL;

	preauth = mac_preauth_find_by_userip(macauth, user_addr);
	if (NULL != preauth) {
		preauth->fastfwd_input_octets = fastfwd_input_octets;
		preauth->fastfwd_output_octets = fastfwd_output_octets;
		set_mac_preauth_flux(preauth);
	}
	
	return EAG_RETURN_OK;
}

int
preauth_init_flux_from_wireless(struct mac_preauth_t *preauth)
{
	eag_macauth_t *macauth = NULL;
	struct WtpStaInfo *StaHead = NULL;
	struct WtpStaInfo *StaNode = NULL;
	DBusConnection *dbus_conn = NULL;
	int ret = 0;
	char user_ipstr[IPX_LEN] = "";
	int found = 0;
	
	if (NULL == preauth) {
		eag_log_err("preauth_init_flux_from_wireless input error");
		return EAG_ERR_NULL_POINTER;
	}
	
	macauth = preauth->macauth;
	dbus_conn = eag_dbus_get_dbus_conn(macauth->eagdbus);
	ipx2str(&(preauth->user_addr), user_ipstr, sizeof(user_ipstr));
	
	StaHead = eag_show_sta_info_of_all_wtp(macauth->hansi_id,
							macauth->hansi_type, dbus_conn, &ret);
	eag_log_debug("eag_macauth", 
			"eag_show_sta_info_of_all_wtp StaHead=%p, ret=%d", StaHead, ret);

	if (NULL != StaHead && EAG_RETURN_OK == ret) {
		for (StaNode=StaHead; StaNode; StaNode=StaNode->next) {
			if ( 0 == memcmp(StaNode->wtpTerminalMacAddr, preauth->usermac, sizeof(preauth->usermac))
				|| StaNode->wtpStaIp == preauth->user_addr.user_ip
				|| 0 == memcmp(&(StaNode->wtpStaIp6), &(preauth->user_addr.user_ipv6), sizeof(struct in6_addr))) {
				found = 1;
				preauth->wireless_data.init_input_octets
							= StaNode->wtpTerminalRecvByteMount;
				preauth->wireless_data.init_output_octets
							= StaNode->wtpSendTerminalByteMount;
			
				preauth->wireless_data.last_input_octets
							= StaNode->wtpTerminalRecvByteMount;
				preauth->wireless_data.last_output_octets
							= StaNode->wtpSendTerminalByteMount;
				
				eag_log_debug("eag_macauth",
					"preauth_init_flux_from_wireless userip %s, "
					"input_octets %llu, output_octets %llu",
					user_ipstr,
					preauth->wireless_data.init_input_octets,
					preauth->wireless_data.init_output_octets);
				break;
			}				
		}
		eag_free_wtp_sta_info_head(StaHead);
	}
	
	if (!found) {
		eag_log_err("appconn_init_flux_from_wireless not found sta %s",
			user_ipstr);
		return -1;
	}
	
	return EAG_RETURN_OK;
}

int
mac_preauth_check_sta_info(struct mac_preauth_t *preauth)
{
	eag_macauth_t *macauth = NULL;
	struct appsession session = {0};
	int ret = 0;
	uint32_t userip = 0;
	//uint8_t zero_mac[6] = {0};
	//DBusConnection *dbus_conn = NULL;
	unsigned int security_type = 0;
	
	userip = preauth->user_addr.user_ip;
	macauth = preauth->macauth;
if (!cmtest_no_arp_learn)
{
	# if 0
	//eag_ipinfo_get(session.intf, sizeof(session.intf)-1, session.usermac, userip);
	if (0 == memcmp(zero_mac, session.usermac, 6)) {
		return -1;
	}
	#endif
	//dbus_conn = eag_dbus_get_dbus_conn(macauth->eagdbus);
	ret = eag_get_sta_info_by_mac_v2(macauth->eagdbus, macauth->hansi_type, macauth->hansi_id,
				preauth->usermac, &session, &security_type);
	if (0 != ret || SECURITY_MAC_AUTH != security_type) {
		return -1;
	}
}
	return 0;
}

static void
macauth_preauth_interval(struct mac_preauth_t *preauth,
		eag_macauth_t *macauth)
{
	time_t timenow = 0;
	struct timeval tv = {0};
	struct app_conn_t *appconn = NULL;
	user_addr_t user_addr = {0};
	char user_ipstr[IPX_LEN] = "";
	int flux_from = 0;
	struct app_conn_t *tmp_appconn = NULL;
	struct appsession tmpsession = {0};
	int ret = 0;

	memset(&user_addr, 0, sizeof(user_addr_t));
	memcpy(&user_addr, &(preauth->user_addr), sizeof(user_addr_t));
	ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;

	flux_from = eag_macauth_get_flux_from(macauth);
	if ( (FLUX_FROM_FASTFWD == flux_from
			|| FLUX_FROM_FASTFWD_IPTABLES == flux_from)
		&& timenow - preauth->last_fastfwd_flux_time >= macauth->flux_interval)
	{
		eag_fastfwd_send(macauth->fastfwd, &user_addr, SE_AGENT_GET_USER_FLOWS);
		preauth->last_fastfwd_flux_time = timenow;
	}

	if ((timenow < preauth->last_flux_time)
		|| (timenow > preauth->last_flux_time + macauth->check_interval)) {
		
		preauth->last_flux_time = timenow;
		preauth->last_input_octets = preauth->input_octets;
		preauth->last_output_octets = preauth->output_octets;
	}

	if (0 == preauth->macbind_req_time
		&& (preauth->output_octets > preauth->last_output_octets)
		&& (preauth->output_octets + preauth->input_octets 
			> preauth->last_output_octets + preauth->last_input_octets + macauth->flux_threshold))
	{
		appconn = appconn_find_by_userip(macauth->appdb, &user_addr);
		if (NULL == appconn) {
			ret = appconn_check_is_conflict(&user_addr, macauth->appdb, &tmpsession, &tmp_appconn);
			if (EAG_ERR_APPCONN_APP_IS_CONFLICT == ret && NULL != tmp_appconn) {
				if (APPCONN_STATUS_AUTHED == tmp_appconn->session.state) {
					terminate_appconn_nowait(tmp_appconn, macauth->eagins, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
				} else {
					appconn_del_from_db(tmp_appconn);
					appconn_free(tmp_appconn);
				}
				tmp_appconn = NULL;
			} else if (EAG_RETURN_OK != ret) {
				return;
			}		
			
			appconn = appconn_create_no_arp(macauth->appdb, &tmpsession);
		}
		
		if (NULL == appconn) {
			eag_log_err("preauth flux reach threshold, but appconn %s create failed",
				user_ipstr);
			eag_del_mac_preauth(macauth, &user_addr);
			return;
		} 

		if (EAG_RETURN_OK != appconn_config_portalsrv(appconn, macauth->portalconf)) {
			eag_log_err("preauth flux reach threshold, "
				"but appconn %s config portal srv failed", user_ipstr);
			eag_del_mac_preauth(macauth, &user_addr);
			return;
		}
		
		eag_portal_macbind_req(macauth->portal, appconn);
		preauth->macbind_req_time = timenow;
		appconn->last_active_time = tv.tv_sec;
	}

	if (0 == preauth->last_invalid_check_time) {
		preauth->last_invalid_check_time = timenow;
	}
	if (timenow - preauth->last_invalid_check_time > 300)
	{
		preauth->last_invalid_check_time = timenow;
		if ( EAG_RETURN_OK != mac_preauth_check_sta_info(preauth))
		{
			eag_del_mac_preauth(macauth, &user_addr);
		}
	}
}

int
eag_macauth_preauth_check(eag_macauth_t *macauth)
{
	struct timeval tv = {0};
	time_t timenow = 0;
	static time_t last_flux_time = 0;
	static time_t last_check_time = 0;
	struct mac_preauth_t *preauth = NULL;
	struct mac_preauth_t *next = NULL;
	int flux_from = 0;
	
	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	flux_from = eag_macauth_get_flux_from(macauth);
	
	if (timenow - last_flux_time >= macauth->flux_interval) {
		if (FLUX_FROM_WIRELESS == flux_from) {
			flush_all_preauth_flux_from_wireless(macauth);
		}
		else if (FLUX_FROM_IPTABLES == flux_from
			|| FLUX_FROM_IPTABLES_L2 == flux_from
			|| FLUX_FROM_FASTFWD_IPTABLES == flux_from)
		{
			flush_all_preauth_flux_from_ipxtables(macauth);
		}
		last_flux_time = timenow;
	}

	if (timenow - last_check_time >= 1) {
		list_for_each_entry_safe(preauth, next, &(macauth->head), node) {
				macauth_preauth_interval(preauth, macauth);
		}
		last_check_time = timenow;
	}
	
	return EAG_RETURN_OK;
}

int
eag_add_mac_preauth(eag_macauth_t *macauth,
		user_addr_t *user_addr, uint8_t usermac[6])
{
	struct mac_preauth_t *preauth = NULL;
	int flux_from = 0;
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	int find = 0;

	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	mac2str(usermac, user_macstr, sizeof(user_macstr), '-');
	flux_from = eag_macauth_get_flux_from(macauth);
	if (NULL != (preauth = mac_preauth_find_by_userip(macauth, user_addr))) {
		if (0 != memcmp(usermac, preauth->usermac, 6)) {
			eag_log_err("found preauth by userip but preauth.mac doesn't match with usermac, eag_add_mac_preauth failed!");
			return -1;
		}
		find = 1;
	}
	if (NULL != (preauth = mac_preauth_find_by_usermac(macauth, usermac))) {
		if (memcmp_ipx(user_addr, &(preauth->user_addr))) {
			eag_log_err("found preauth by usermac but preauth.ip doesn't match with userip, eag_add_mac_preauth failed!");
			return -1;
		}
		find = 1;
	}
	if (!find) {
		preauth = mac_preauth_new(macauth, user_addr, usermac);
		if (NULL == preauth) {
			eag_log_err("add_mac_preauth mac_preauth_new failed, userip=%s",
				user_ipstr);
			return -1;
		}
		eag_log_info("add mac_preauth userip=%s usermac=%s", user_ipstr, user_macstr);
		eag_captive_macpre_authorize(macauth->captive, user_addr);
		if (FLUX_FROM_FASTFWD == flux_from
			|| FLUX_FROM_FASTFWD_IPTABLES == flux_from) 
		{
			eag_fastfwd_send(macauth->fastfwd, user_addr, SE_AGENT_USER_ONLINE);
		}
		else if (FLUX_FROM_WIRELESS == flux_from) {
			preauth_init_flux_from_wireless(preauth);
		}
	}

	return 0;
}

int
eag_del_mac_preauth(eag_macauth_t *macauth,
		user_addr_t *user_addr)
{
	struct mac_preauth_t *preauth = NULL;
	int flux_from = 0;
	char user_ipstr[IPX_LEN] = "";

	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	flux_from = eag_macauth_get_flux_from(macauth);
	preauth = mac_preauth_find_by_userip(macauth, user_addr);
	if (NULL != preauth) {
		eag_log_info("del mac_preauth userip=%s", user_ipstr);
		eag_captive_del_macpre_authorize(macauth->captive, user_addr);
		if (FLUX_FROM_FASTFWD == flux_from
			|| FLUX_FROM_FASTFWD_IPTABLES == flux_from) 
		{
			eag_fastfwd_send(macauth->fastfwd, user_addr, SE_AGENT_USER_OFFLINE);
		}
		mac_preauth_free(preauth);
	}

	return 0;
}

int
del_eag_preauth_by_ip_or_mac(eag_macauth_t *macauth,
		user_addr_t *user_addr, uint8_t usermac[6])
{
	struct mac_preauth_t *preauth = NULL;
	
	if (0 != memcmp_ipx(user_addr, NULL)) {
		eag_del_mac_preauth(macauth, user_addr);
		return 0;
	}
	
	preauth = mac_preauth_find_by_usermac(macauth, usermac);
	if (NULL != preauth) {
		eag_del_mac_preauth(macauth, &(preauth->user_addr));
		return 0;
	}

	eag_log_debug("eag_macauth", "del_eag_preauth_by_ip_or_mac userip is 0 or not find preauth by mac!");
	return -1;
}

int 
eag_macauth_log_all_preauth(eag_macauth_t *macauth)
{
	struct mac_preauth_t *preauth = NULL;
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	int num = 0;

	if (NULL == macauth) {
		eag_log_err("eag_macauth_log_all_preauth input error");
		return -1;
	}

	eag_log_info( "-----log all mac_preauth begin-----");
	list_for_each_entry(preauth, &(macauth->head), node) {
		num++;
		ipx2str(&(preauth->user_addr), user_ipstr, sizeof(user_ipstr));
		mac2str(preauth->usermac, user_macstr, sizeof(user_macstr), '-');
		eag_log_info("%-5d mac_preauth userip:%s usermac:%s last_input_octets:%llu "
			"last_output_octets:%llu last_flux_time:%lu input_octets:%llu "
			"output_octets:%llu macbind_req_time:%lu",
			num, user_ipstr, user_macstr, preauth->last_input_octets,
			preauth->last_output_octets, preauth->last_flux_time, preauth->input_octets,
			preauth->output_octets, preauth->macbind_req_time);
	}
	eag_log_info( "-----log all mac_preauth end, num: %d-----", num);

	return 0;
}

void
eag_macauth_set_thread_master(eag_macauth_t *macauth,
		eag_thread_master_t *master)
{
	if (NULL == macauth ||NULL == master) {
		eag_log_err("eag_macauth_set_thread_master input error");
		return;
	}
	macauth->master = master;
}

int
eag_macauth_set_macauth_switch(eag_macauth_t *macauth,
		int macauth_switch)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_macauth_switch input error");
		return -1;
	}

	macauth->macauth_switch = macauth_switch;

	return EAG_RETURN_OK;
}

int
eag_macauth_get_macauth_switch(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_get_macauth_switch input error");
		return 0;
	}

	return macauth->macauth_switch;
}

int
eag_macauth_set_flux_from(eag_macauth_t *macauth,
		int flux_from)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_flux_from input error");
		return -1;
	}

	macauth->flux_from = flux_from;

	return EAG_RETURN_OK;
}

int
eag_macauth_get_flux_from(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_get_flux_from input error");
		return 0;
	}

	return macauth->flux_from;
}

int
eag_macauth_set_flux_interval(eag_macauth_t *macauth,
		int flux_interval)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_flux_interval input error");
		return -1;
	}

	macauth->flux_interval = flux_interval;

	return EAG_RETURN_OK;
}

int
eag_macauth_get_flux_interval(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_get_flux_interval input error");
		return 0;
	}

	return macauth->flux_interval;
}

int
eag_macauth_set_flux_threshold(eag_macauth_t *macauth,
		int flux_threshold, int check_interval)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_flux_threshold input error");
		return -1;
	}

	macauth->flux_threshold = flux_threshold;
	macauth->check_interval = check_interval;

	return EAG_RETURN_OK;
}

int
eag_macauth_get_flux_threshold(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_get_flux_threshold input error");
		return 0;
	}

	return macauth->flux_threshold;
}

int
eag_macauth_get_check_interval(eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_get_check_interval input error");
		return 0;
	}

	return macauth->check_interval;
}

static void
eag_macauth_event(eag_macauth_event_t event,
						eag_macauth_t *macauth)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_event input error");
		return;
	}
	switch (event) {
	case EAG_MACAUTH_READ:
		macauth->t_read =
		    eag_thread_add_read(macauth->master, eag_macauth_receive,
					macauth, macauth->sockfd);
		break;
	default:
		break;
	}
}

int
eag_macauth_set_eagins(eag_macauth_t *macauth,
		eag_ins_t *eagins)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_eagins input error");
		return -1;
	}

	macauth->eagins = eagins;

	return EAG_RETURN_OK;
}

int
eag_macauth_set_eagdbus(eag_macauth_t *macauth,
		eag_dbus_t *eagdbus)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_eagdbus input error");
		return -1;
	}
	
	macauth->eagdbus = eagdbus;
	return EAG_RETURN_OK;
}

int
eag_macauth_set_appdb(eag_macauth_t *macauth,
		appconn_db_t *appdb)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_appdb input error");
		return -1;
	}

	macauth->appdb = appdb;

	return EAG_RETURN_OK;
}

int
eag_macauth_set_captive(eag_macauth_t *macauth,
		eag_captive_t *cap)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_captive input error");
		return -1;
	}

	macauth->captive = cap;

	return EAG_RETURN_OK;
}

int
eag_macauth_set_fastfwd(eag_macauth_t *macauth,
		eag_fastfwd_t *fastfwd)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_fastfwd input error");
		return -1;
	}

	macauth->fastfwd = fastfwd;

	return EAG_RETURN_OK;
}

int
eag_macauth_set_portal(eag_macauth_t *macauth,
		eag_portal_t *portal)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_portal input error");
		return -1;
	}

	macauth->portal = portal;

	return EAG_RETURN_OK;
}

int
eag_macauth_set_portalconf(eag_macauth_t *macauth,
		struct portal_conf *portalconf)
{
	if (NULL == macauth) {
		eag_log_err("eag_macauth_set_portal_conf input error");
		return -1;
	}

	macauth->portalconf = portalconf;

	return EAG_RETURN_OK;
}

