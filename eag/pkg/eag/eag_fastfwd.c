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
* eag_fastfwd.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* eag fastfwd
*
*
*******************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/tipc.h>

#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "eag_conf.h"  //not need
#include "eag_interface.h"
#include "eag_dbus.h"
#include "eag_util.h"
#include "eag_time.h"
#include "eag_fastfwd.h"
#include "appconn.h"
#include "eag_macauth.h"

struct eag_fastfwd {
	int sockfd;
	int slotid;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	appconn_db_t *appdb;
	eag_macauth_t *macauth;
};

typedef enum {
	EAG_FASTFWD_READ,
} eag_fastfwd_event_t;

static void
eag_fastfwd_event(eag_fastfwd_event_t event,
					eag_fastfwd_t *fastfwd);

eag_fastfwd_t *
eag_fastfwd_new(uint8_t hansi_type, 
		uint8_t hansi_id)
{	
	struct eag_fastfwd *fastfwd = NULL;
	
	fastfwd = eag_malloc(sizeof(*fastfwd));
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_new eag_malloc failed");
		return NULL;
	}
	memset(fastfwd, 0, sizeof(*fastfwd));
	fastfwd->sockfd = -1;
	eag_log_info("fastfwd new ok, sockfd(%d), slot-id(%d)",
				fastfwd->sockfd, fastfwd->slotid);
	return fastfwd;
}

int
eag_fastfwd_free(eag_fastfwd_t *fastfwd)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_free input error");
		return -1;
	}

	if (fastfwd->sockfd >= 0) {
		close(fastfwd->sockfd);
		fastfwd->sockfd = -1;
	}
	eag_free(fastfwd);

	eag_log_info( "fastfwd free ok");
	return 0;
}

int 
eag_fastfwd_start(eag_fastfwd_t *fastfwd)
{
	/*
	int ret = 0;
	int len = 0;
	struct sockaddr_un addr = {0};
	mode_t old_mask = 0;
	*/
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_start fastfwd is null");
		return EAG_ERR_NULL_POINTER;
	}
	if (fastfwd->sockfd >= 0) {
		eag_log_err("eag_fastfwd_start already start fd(%d)", fastfwd->sockfd);
		return EAG_RETURN_OK;
	}
	/*
	fastfwd->sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	*/
	fastfwd->sockfd = socket(AF_TIPC, SOCK_DGRAM, 0);	
	if (fastfwd->sockfd < 0) {
		eag_log_err("eag_fastfwd_start: can't create fastfwd TIPC socket: %s",
											safe_strerror(errno));
		fastfwd->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}
	if (0 != set_nonblocking(fastfwd->sockfd)) {
		eag_log_err("eag_fastfwd_start set socket nonblocking failed");
		close(fastfwd->sockfd);
		fastfwd->sockfd = -1;
		return EAG_ERR_SOCKET_OPT_FAILED;
	}
#if 0  /* socket type turned from UNIX LOCAL socket to TIPC socket */
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, fastfwd->fastfwd_sockpath, sizeof(addr.sun_path)-1);
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = addr.sun_len = SUN_LEN(&addr);
#else
	len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	unlink(addr.sun_path);
	old_mask = umask(0111);
	ret  = bind(fastfwd->sockfd, (struct sockaddr *)&addr, len);
	if (ret < 0) {
		eag_log_err("Can't bind to fastfwd socket(%d): %s",
			fastfwd->sockfd, safe_strerror(errno));
		close(fastfwd->sockfd);
		fastfwd->sockfd = -1;
		umask(old_mask);
		return EAG_ERR_SOCKET_BIND_FAILED;
	}
	umask(old_mask);
#endif
	eag_fastfwd_event(EAG_FASTFWD_READ, fastfwd);
	eag_log_info("eag_fastfwd_start: fastfwd slotid(%d) fd(%d) start ok",
							fastfwd->slotid, fastfwd->sockfd);
	return EAG_RETURN_OK;		
}

int
eag_fastfwd_stop(eag_fastfwd_t *fastfwd)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_stop input error");
		return EAG_ERR_NULL_POINTER;
	}

	eag_log_info("fastfwd_slotid(%d) fd(%d) stop ok", fastfwd->slotid, fastfwd->sockfd);	
	if (NULL != fastfwd->t_read) {
		eag_thread_cancel(fastfwd->t_read);
		fastfwd->t_read = NULL;
	}
	if (fastfwd->sockfd >= 0) {
		close(fastfwd->sockfd);
		fastfwd->sockfd = -1;
	}
	/*
	unlink(fastfwd->fastfwd_sockpath);
	*/
	return EAG_RETURN_OK;
}

static int 
eag_fastfwd_receive(eag_thread_t *thread)
{
	eag_fastfwd_t *fastfwd = NULL;
	struct app_conn_t *appconn = NULL;
	struct sockaddr_tipc addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	se_interative_t se_data;
	user_addr_t user_addr = {0};
	char user_ipstr[IPX_LEN] = "";
	struct timeval tv = {0};
	time_t time_now = 0;
	
	if (NULL == thread) {
		eag_log_err("eag_fastfwd_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	eag_time_gettimeofday(&tv, NULL);
	time_now = tv.tv_sec;
	
	fastfwd = eag_thread_get_arg(thread);
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_receive fastfwd null");
		return EAG_ERR_NULL_POINTER;
	}
	memset(&addr, 0, sizeof(struct sockaddr_tipc));
	len = sizeof(addr);
	nbyte = recvfrom(fastfwd->sockfd, &se_data, sizeof(se_interative_t), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("eag_fastfwd_receive recvfrom failed: %s, fd(%d)",
			safe_strerror(errno), fastfwd->sockfd);
		return EAG_ERR_SOCKET_RECV_FAILED;
	}
	
	eag_log_debug("eag_fastfwd", "fastfwd fd(%d) receive %d bytes",
		fastfwd->sockfd,
		nbyte);
	
	if (nbyte < sizeof(se_interative_t)) {
		eag_log_warning("eag_fastfwd_receive receive bytes %d < se_interative_t size %d",
			nbyte, sizeof(se_interative_t));
		return -1;
	}

	if (se_data.cmd_result != AGENT_RETURN_OK) {
		se_data.err_info[sizeof(se_data.err_info)-1] = '\0';
		eag_log_warning("eag_fastfwd_receive cmd_result: %x err_info: %s",
			se_data.cmd_result, se_data.err_info);
		return -1;
	}
	memset(&user_addr, 0, sizeof(user_addr));
	user_addr.family = EAG_IPV4;
	user_addr.user_ip = se_data.fccp_cmd.fccp_data.user_info.user_ip;
	//memcpy(&user_addr, &(appconn->session.user_addr), sizeof(user_addr_t));
	ipx2str(&user_addr, user_ipstr, sizeof(user_ipstr));
	
	eag_log_debug("eag_fastfwd", 
		"eag_fastfwd_receive: userip:%s, output_octets:%llu, output_packets:%llu, "
		"input_octets:%llu, input_packets:%llu",
		user_ipstr, 
		se_data.fccp_cmd.fccp_data.user_info.forward_up_bytes, 
		se_data.fccp_cmd.fccp_data.user_info.forward_up_packet, 
		se_data.fccp_cmd.fccp_data.user_info.forward_down_bytes, 
		se_data.fccp_cmd.fccp_data.user_info.forward_down_packet);

	appconn = appconn_find_by_userip(fastfwd->appdb, &user_addr);
	if (NULL != appconn && APPCONN_STATUS_AUTHED == appconn->session.state) {
		appconn->fastfwd_data.input_octets = se_data.fccp_cmd.fccp_data.user_info.forward_down_bytes;
		appconn->fastfwd_data.input_packets = se_data.fccp_cmd.fccp_data.user_info.forward_down_packet;
		appconn->fastfwd_data.output_octets = se_data.fccp_cmd.fccp_data.user_info.forward_up_bytes;
		appconn->fastfwd_data.output_packets = se_data.fccp_cmd.fccp_data.user_info.forward_up_packet;
		set_appconn_flux(appconn);
		appconn_check_flux(appconn, time_now);
	}

	flush_preauth_flux_from_fastfwd(fastfwd->macauth, &user_addr,
		se_data.fccp_cmd.fccp_data.user_info.forward_down_bytes,
		se_data.fccp_cmd.fccp_data.user_info.forward_up_bytes);

	return EAG_RETURN_OK;
}

int
eag_fastfwd_send(eag_fastfwd_t *fastfwd, 
		user_addr_t *user_addr, const char *hand_cmd)
{
	se_interative_t se_data;
	/* struct sockaddr_un addr = {0}; */
	struct sockaddr_tipc addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	char user_ipstr[IPX_LEN] = "";

	if (NULL == fastfwd || NULL == hand_cmd) {
		eag_log_err("eag_fastfwd_send input error");
		return EAG_ERR_NULL_POINTER;
	}	
	if (fastfwd->sockfd < 0) {
		eag_log_err("eag_fastfwd_send sockfd(%d) failed", fastfwd->sockfd);
		return EAG_ERR_UNKNOWN;
	}

	memset(&se_data, 0, sizeof(se_interative_t));
	strncpy(se_data.hand_cmd, hand_cmd, sizeof(se_data.hand_cmd)-1);
	se_data.fccp_cmd.fccp_data.user_info.user_ip = user_addr->user_ip;
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	eag_log_debug("eag_fastfwd", 
		"eag_fastfwd_send sockfd(%d) slotid(%d), userip:%s, Op:%s",
		fastfwd->sockfd, fastfwd->slotid, user_ipstr, hand_cmd);

	memset(&addr, 0, sizeof(struct sockaddr_tipc));
	addr.family = AF_TIPC;
	addr.addrtype = TIPC_ADDR_NAME;
	addr.addr.name.name.type = SE_AGENT_SERVER_TYPE;
	addr.addr.name.name.instance = SE_AGENT_SERVER_LOWER_INST + fastfwd->slotid;
	addr.addr.name.domain = 0;
	addr.scope = TIPC_CLUSTER_SCOPE;
	len = sizeof(addr);

	nbyte = sendto(fastfwd->sockfd, &se_data, sizeof(se_data),
				MSG_DONTWAIT, (struct sockaddr*)&addr, len);
	if (nbyte < 0) {
		eag_log_err("eag_fastfwd_send sendto failed, fd(%d), slotid(%d), %s",
						fastfwd->sockfd, fastfwd->slotid, safe_strerror(errno));
		return EAG_ERR_SOCKET_SEND_FAILED;
	}
	if (nbyte != sizeof(se_data)) {
		eag_log_err("eag_fastfwd_send sendto failed, nbyte(%d)!=sizeof(se_buf)(%d)",
			nbyte, sizeof(se_data));
		return EAG_ERR_SOCKET_SEND_FAILED;
	}
	
	return EAG_RETURN_OK;
}

int
eag_fastfwd_set_slot_id(eag_fastfwd_t *fastfwd,
		uint8_t slot_id)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_set_slot_id input error");
		return -1;
	}

	fastfwd->slotid = slot_id;

	return 0;
}

void
eag_fastfwd_set_thread_master(eag_fastfwd_t *fastfwd,
		eag_thread_master_t *master)
{
	if (NULL == fastfwd ||NULL == master) {
		eag_log_err("eag_fastfwd_set_thread_master input error");
		return;
	}
	fastfwd->master = master;
}

static void
eag_fastfwd_event(eag_fastfwd_event_t event,
						eag_fastfwd_t *fastfwd)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_event input error");
		return;
	}
	switch (event) {
	case EAG_FASTFWD_READ:
		fastfwd->t_read =
		    eag_thread_add_read(fastfwd->master, eag_fastfwd_receive,
					fastfwd, fastfwd->sockfd);
		break;
	default:
		break;
	}
}

int
eag_fastfwd_set_appdb(eag_fastfwd_t *fastfwd,
		appconn_db_t *appdb)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_set_appdb input error");
		return -1;
	}

	fastfwd->appdb = appdb;

	return EAG_RETURN_OK;
}

int
eag_fastfwd_set_macauth(eag_fastfwd_t *fastfwd,
		eag_macauth_t *macauth)
{
	if (NULL == fastfwd) {
		eag_log_err("eag_fastfwd_set_macauth input error");
		return -1;
	}

	fastfwd->macauth = macauth;

	return EAG_RETURN_OK;
}

