#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "mem_cache.h"
#include "pppoe_util.h"
#include "pppoe_buf.h"
#include "rdc_packet.h"
#include "rdc_handle.h"
#include "radius_packet.h"
#include "pppoe_thread.h"
#include "radius_coa.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"

#include "pppoe_radius.h"

#define RADIUS_ID_NUM		256

#define RADIUS_CACHE_BLK_ITEMNUM	128
#define RADIUS_CACHE_MAX_BLKNUM		0				/* blk num not limit */
#define RADIUS_CACHE_EMPTY_BLKNUM	1
#define RADIUS_CACHE_NAME			"pppoe radius cache"

typedef enum {
	RADIUS_ID_FREE,
	RADIUS_ID_USED,
	RADIUS_ID_WAIT,
} radiusIDStatus;

struct radius_config {
	/* radius public config, from device config */
	uint32 slot_id;
	uint32 local_id;
	uint32 instance_id;
	uint32 local_ip;	/* use for rdc */


	/* radius private config, need show running */		
	uint32 nasip;
	struct radius_srv srv;

	uint32 rdc_state;
	uint32 rdc_slotid;
	uint32 rdc_insid;
};

struct radius_id {
	uint8 id;
	long freetime;
	radiusIDStatus 	status;
	session_struct_t 	*sess;
	struct radius_sock 	*sock;
	struct list_head next;
	struct radius_packet req_pack;
};


struct radius_sock {
	int sk;
	uint8  curr_id;	

	struct pppoe_buf *pbuf;

	rdc_client_t	*rdc;
	radius_config_t *config;
	thread_struct_t *thread;
	thread_master_t *master;
	
	struct list_head next;
	struct radius_id id[RADIUS_ID_NUM];
};

struct radius_struct {
	struct pppoe_buf *pbuf;
	
	mem_cache_t 	*cache;
	rdc_client_t	*rdc;
	radius_coa_t	*coa;
	radius_config_t *config;
	thread_master_t *master;
	pppoe_manage_t	*manage;
	struct list_head sock_list;
	struct list_head free_list;
};


static char *radius_codenames[] = {
	"Radius Access Request", 
	"Radius Access Accept", 
	"Radius Access Reject", 
	"Radius Account Request", 
	"Radius Account Response",
};


static inline int
_recv_packet(int sk, struct pppoe_buf *pbuf, struct sockaddr_in *addr, socklen_t *len) {
	char errbuf[128];	
	ssize_t length;

	do {
		length = recvfrom(sk, pbuf->data, pbuf->end - pbuf->data, 
							0, (struct sockaddr *)addr, len);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d recv failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ERECV;			
		}
	} while (length < 0);

	pbuf->len = length;
	pbuf->tail = pbuf->data + pbuf->len;
	return PPPOEERR_SUCCESS;
}

static inline int
_send_packet(int sk, struct pppoe_buf *pbuf, const struct sockaddr_in *addr, socklen_t len) {
	char errbuf[128];	
	ssize_t length;

	do {
		length = sendto(sk, pbuf->data, pbuf->tail - pbuf->data, 
						0, (struct sockaddr *)addr, len);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d send failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ESEND;			
		}
	} while (length < 0);
		
	return PPPOEERR_SUCCESS;
}

static int
coa_recv_packet(void *sk_ptr, int sk, struct pppoe_buf *pbuf, 
					struct sockaddr_in *addr, socklen_t *len) {
	if (sk_ptr)
		return rdc_recvfrom(sk_ptr, sk, pbuf, addr, len);

	return _recv_packet(sk, pbuf, addr, len);
}

static int
coa_send_packet(void *sk_ptr, int sk, struct pppoe_buf *pbuf, 
					struct sockaddr_in *addr, socklen_t len) {
	if (sk_ptr)
		return rdc_sendto(sk_ptr, sk, pbuf, addr, len);
		
	return 	_send_packet(sk, pbuf, addr, len);
}

static struct session_struct *
coa_session_lookup(pppoe_manage_t *manage, 
						struct pppoe_buf *pbuf,
						uint32 *error_cause) {
	struct session_struct *sess;
	struct radius_attr *attr;
	char username[USERNAMESIZE] = { 0 };
	char sessionid[ACCT_SESSIONIDSIZE] = { 0 };
	uint32 userip = 0;

	if (!radius_getattr(pbuf, &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		if ((attr->l - 2) < sizeof(username)) {
			memcpy(username, attr->v.t, attr->l - 2);
			pppoe_token_log(TOKEN_RADIUS, "username %s\n", username);	
		}
	}

	/* Framed-IP-Address */
	if (!radius_getattr(pbuf, &attr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
		userip = ntohl(attr->v.i);
		pppoe_token_log(TOKEN_RADIUS, "userip %u.%u.%u.%u\n", HIPQUAD(userip));	
	}

	/* Acct-Session-ID */
	if (!radius_getattr(pbuf, &attr, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0)) {
		if ((attr->l - 2) < sizeof(sessionid)) {
			memcpy(sessionid, attr->v.t, attr->l-2);
			pppoe_token_log(TOKEN_RADIUS, "acctsessionid %s\n", sessionid);	
		}
	}	

	if (0 == username[0] && 0 == sessionid[0] && 0 == userip) {
		pppoe_log(LOG_WARNING, "coa disconnect request miss attr\n");
		*error_cause = RADIUS_ERROR_CAUSE_402;
		return NULL;
	}

	list_for_each_entry(sess, 
		__manage_sess_get_list(manage), next) { 
		if (sess->state != SESSION_ONLINE)
			continue;
		
		if (username[0] && memcmp(username, sess->username, sizeof(username)))
			continue;

		if (userip && userip != sess->ipaddr.ip)
			continue;

		if (sessionid[0] && memcmp(sessionid, sess->acctSessID, sizeof(sessionid)))
			continue;

		pppoe_token_log(TOKEN_RADIUS, "session %s %u.%u.%u.%u lookup success\n", 
										username, HIPQUAD(userip));
		*error_cause = RADIUS_ERROR_CAUSE_201;
		return sess;
	}

	pppoe_log(LOG_WARNING, "session (username %s, ip %u.%u.%u.%u, "
							"acctSessID %s) lookup failed\n",
							username, HIPQUAD(userip), sessionid);
	*error_cause = RADIUS_ERROR_CAUSE_503;
	return NULL;
}

static inline struct radius_server *
coa_radius_server_lookup(radius_config_t *config, uint32 radius_ip) {
	if (config->srv.auth.ip == radius_ip)
		return &config->srv.auth;

	if (config->srv.backup_auth.ip == radius_ip)
		return &config->srv.backup_auth;

	if (config->srv.acct.ip == radius_ip)
		return &config->srv.acct;

	if (config->srv.backup_acct.ip == radius_ip)
		return &config->srv.backup_acct;

	return NULL;
}

static inline void
coa_session_disconnnect(struct session_struct *sess) {
	sess->offlineTime = time_sysup();
	sess->state = SESSION_OFFLINE;

	/* destroy session timer */
	session_timer_destroy(sess, SESSION_RETRANS_TIMER);
	session_timer_destroy(sess, SESSION_UPDATE_TIMER);
	session_timer_destroy(sess, SESSION_ECHO_TIMER);
	
	session_opt_perform(sess, SESSOPT_OFFLINESYNC);
	_manage_sess_exit(sess);
}

static int
coa_disconnect_request(void *proto_ptr, 
							struct pppoe_buf *pbuf, 
							uint32 radius_ip) {
	radius_struct_t *radius = (radius_struct_t *)proto_ptr;
	struct session_struct *sess;
	struct radius_server *srv;
	struct radius_packet *pack;
	uint8 authenticator[RADIUS_AUTHLEN];
	uint32 error_cause;
	
	if (unlikely(!radius || !pbuf || !radius_ip))
		return PPPOEERR_EINVAL;

	srv = coa_radius_server_lookup(radius->config, radius_ip);
	if (!srv) {
		pppoe_log(LOG_WARNING, "lookup radius config failed "
				"whitch from %u.%u.%u.%u\n", HIPQUAD(radius_ip));
		return PPPOEERR_EADDR;
	}

	pppoe_token_log(TOKEN_RADIUS, "recv radius server %u.%u.%u.%u "
								"session disconnect request\n",
								HIPQUAD(srv->ip));
		
	pack = (struct radius_packet *)pbuf->data;
	pbuf_pull(pbuf, sizeof(struct radius_packet));

	sess = coa_session_lookup(radius->manage, pbuf, &error_cause);
	if (!sess) {
		pppoe_log(LOG_WARNING, "session lookup failed, err %u\n", 
								error_cause);
		pack->code = RADIUS_CODE_DISCONNECT_NAK;
		goto out;
	}

	pppoe_log(LOG_INFO, "coa disconnect session: "
						"username %s, ip %u.%u.%u.%u\n", 
						sess->username, HIPQUAD(sess->ipaddr.ip));
	coa_session_disconnnect(sess);
	pack->code = RADIUS_CODE_DISCONNECT_ACK;

out:	
	pbuf_trim(pbuf, 0);
	pbuf_push(pbuf, sizeof(struct radius_packet));
//	pack->length = htons(pbuf->len);
	
	radius_addattr(pbuf, RADIUS_ATTR_ERROR_CAUSE, 0, 0,
					error_cause, NULL, 0);
	
	memcpy(authenticator, pack->authenticator, RADIUS_AUTHLEN);
	radius_authresp_authenticator(pack, authenticator,
						srv->secret, srv->secretlen);

	return PPPOEERR_SUCCESS;
}

static inline int
sock_send_packet(struct radius_sock *sock,
					uint32 radius_ip, uint16 radius_port,
					struct pppoe_buf *pbuf) {
	struct sockaddr_in addr;
	int ret;
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(radius_ip);
	addr.sin_port = htons(radius_port);

	if (sock->rdc) {
		ret = rdc_sendto(sock->rdc, sock->sk, pbuf, &addr, sizeof(addr));
	} else {
		ret = _send_packet(sock->sk, pbuf, &addr, sizeof(addr));
	}

	if (ret) {
		pppoe_log(LOG_WARNING, "radius send to %u.%u.%u.%u:%u fail, ret %d\n",
								HIPQUAD(radius_ip), radius_port, ret);
		return ret;
	}

	pppoe_token_log(TOKEN_RADIUS, "radius send to %u.%u.%u.%u:%u success\n", 
								HIPQUAD(radius_ip), radius_port);
	return PPPOEERR_SUCCESS;
}

static inline void
sock_setup(struct radius_sock *sock) {
	int i;	

	for (i = 0; i < RADIUS_ID_NUM; i++) {
		sock->id[i].id = i;
		sock->id[i].status = RADIUS_ID_FREE;
		sock->id[i].sock = sock;
	}
}

static void
packet_auth_req(radius_config_t *config, session_struct_t *sess, struct pppoe_buf *pbuf) {
	uint8 chapPasswd[RADIUS_PASSWORD_LEN + 1];
	
	/*service type*/
	radius_addattr(pbuf, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		       	RADIUS_SERVICE_TYPE_FRAMED, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_PROTOCOL, 0, 0,
		       	RADIUS_FRAMED_PROTOCOL_PPP, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_USER_NAME, 0, 0, 0,
				(uint8 *)sess->username, strlen(sess->username));

	radius_addattr(pbuf, RADIUS_ATTR_CHAP_CHALLENGE, 0, 0, 0,
					sess->challenge, RADIUS_CHAP_CHAL_LEN);

	chapPasswd[0] = sess->ident & 0xff;
	memcpy(chapPasswd + 1, sess->passwd, RADIUS_PASSWORD_LEN);
	radius_addattr(pbuf, RADIUS_ATTR_CHAP_PASSWORD, 0, 0, 0,
					chapPasswd, RADIUS_PASSWORD_LEN + 1);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 
					config->nasip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT, 0, 0, 0, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
					RADIUS_NAS_PORT_TYPE_VIRTUAL, NULL, 0);
}

static void
packet_acct_start_req(radius_config_t *config, session_struct_t *sess, struct pppoe_buf *pbuf) {
	radius_addattr(pbuf, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		       		RADIUS_SERVICE_TYPE_FRAMED, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_PROTOCOL, 0, 0,
		       		RADIUS_FRAMED_PROTOCOL_PPP, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
					(uint8 *)sess->acctSessID, strlen(sess->acctSessID));

	radius_addattr(pbuf, RADIUS_ATTR_USER_NAME, 0, 0, 0,
					(uint8 *)sess->username, strlen(sess->username));

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		       		sess->ipaddr.ip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
					RADIUS_STATUS_TYPE_START, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_AUTHENTIC, 0, 0,
					RADIUS_AUTHENTIC_TYPE_RADIUS, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 
					config->nasip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT, 0, 0, 0, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
					RADIUS_NAS_PORT_TYPE_VIRTUAL, NULL, 0);
}

static void
packet_acct_stop_req(radius_config_t *config, session_struct_t *sess, struct pppoe_buf *pbuf) {
	radius_addattr(pbuf, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		       		RADIUS_SERVICE_TYPE_FRAMED, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_PROTOCOL, 0, 0,
		       		RADIUS_FRAMED_PROTOCOL_PPP, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
					(uint8 *)sess->acctSessID, strlen(sess->acctSessID));

	radius_addattr(pbuf, RADIUS_ATTR_USER_NAME, 0, 0, 0,
					(uint8 *)sess->username, strlen(sess->username));

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		       		sess->ipaddr.ip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
					RADIUS_STATUS_TYPE_STOP, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_AUTHENTIC, 0, 0,
					RADIUS_AUTHENTIC_TYPE_RADIUS, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_TERMINATE_CAUSE, 0, 0,
					sess->terminate_cause, NULL, 0);
	
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
					sess->offlineTime - sess->onlineTime, NULL, 0);
	
	/* ACCT_INPUT_OCTETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0, 0,
					sess->stat.rx_bytes & 0xffffffff, NULL, 0);

	/* ACCT_INPUT_GIGAWORDS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS, 0, 0,
					(sess->stat.rx_bytes >> 32) & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_OCTETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0, 0,
					sess->stat.tx_bytes & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_GIGAWORDS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS, 0, 0,
					(sess->stat.tx_bytes >> 32) & 0xffffffff, NULL, 0);

	/* ACCT_INPUT_PACKETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0, 0,
					sess->stat.rx_packets & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_PACKETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0, 0,
					sess->stat.tx_packets & 0xffffffff, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 
					config->nasip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT, 0, 0, 0, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
					RADIUS_NAS_PORT_TYPE_VIRTUAL, NULL, 0);
}

static void
packet_acct_update_req(radius_config_t *config, session_struct_t *sess, struct pppoe_buf *pbuf) {
	radius_addattr(pbuf, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		       		RADIUS_SERVICE_TYPE_FRAMED, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_PROTOCOL, 0, 0,
		       		RADIUS_FRAMED_PROTOCOL_PPP, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
					(uint8 *)sess->acctSessID, strlen(sess->acctSessID));

	radius_addattr(pbuf, RADIUS_ATTR_USER_NAME, 0, 0, 0,
					(uint8 *)sess->username, strlen(sess->username));

	radius_addattr(pbuf, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		       		sess->ipaddr.ip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
					RADIUS_STATUS_TYPE_INTERIM_UPDATE, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_AUTHENTIC, 0, 0,
					RADIUS_AUTHENTIC_TYPE_RADIUS, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
					time_sysup() - sess->onlineTime, NULL, 0);
	
	/* ACCT_INPUT_OCTETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0, 0,
					sess->stat.rx_bytes & 0xffffffff, NULL, 0);

	/* ACCT_INPUT_GIGAWORDS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS, 0, 0,
					(sess->stat.rx_bytes >> 32) & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_OCTETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0, 0,
					sess->stat.tx_bytes & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_GIGAWORDS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS, 0, 0,
					(sess->stat.tx_bytes >> 32) & 0xffffffff, NULL, 0);

	/* ACCT_INPUT_PACKETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0, 0,
					sess->stat.rx_packets & 0xffffffff, NULL, 0);

	/* ACCT_OUTPUT_PACKETS */
	radius_addattr(pbuf, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0, 0,
					sess->stat.tx_packets & 0xffffffff, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 
					config->nasip, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT, 0, 0, 0, NULL, 0);

	radius_addattr(pbuf, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
					RADIUS_NAS_PORT_TYPE_VIRTUAL, NULL, 0);
}

static inline void
_radius_acctSessID_init(session_struct_t *sess) {
	snprintf(sess->acctSessID, sizeof(sess->acctSessID), 
			"%08x%08x", (unsigned int)time(NULL), 
			(sess->sid & 0XFFFF) | ((getpid() & 0xffff) << 16));
}

static inline int
_radius_sess_check(session_struct_t *sess) {
	struct radius_id *radid = sess->priv_data;

	if (!radid) {
		pppoe_log(LOG_WARNING, "radius session check: "
					"session %u radius id is NULL\n", sess->sid);
		return PPPOEERR_EINVAL;
	}	

	if (RADIUS_ID_USED != radid->status || sess != radid->sess) {
		pppoe_log(LOG_WARNING, "radius session check: "
					"session %u not match radius id %u\n", 
					sess->sid, radid->id);
		sess->priv_data = NULL;
		return PPPOEERR_EINVAL;
	}

	return PPPOEERR_SUCCESS;
}



static inline void
_radius_sess_setup(struct radius_id *radid, struct radius_packet *pack, session_struct_t *sess) {
	sess->priv_data = radid;
	pack->id = radid->id;
	radid->status = RADIUS_ID_USED;
	radid->sess = sess;
	memcpy(&radid->req_pack, pack, sizeof(struct radius_packet));
}

static inline void
_radius_sess_clean(session_struct_t *sess) {
	struct radius_id *radid = sess->priv_data;
	
	radid->status = RADIUS_ID_FREE;
	radid->sess = NULL;
	sess->priv_data = NULL;
}

static inline int
_radius_sess_retransport(struct radius_id *radid, session_struct_t *sess) {
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	radius_config_t *config = radid->sock->config;
	uint32 radius_ip;
	uint16 radius_port;

	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));
	memcpy(pbuf_put(pbuf, sizeof(struct radius_packet)), 
			&radid->req_pack, sizeof(struct radius_packet));

	if (SESSION_AUTHPOST == sess->state) {
		packet_auth_req(config, sess, pbuf);

		if (config->srv.backup_auth.ip &&
			sess->retryTimes >= ((sess->retryMaxTimes >> 1) + 1)) {
			radius_ip = config->srv.backup_auth.ip;
			radius_port = config->srv.backup_auth.port;
		} else {
			radius_ip = config->srv.auth.ip;
			radius_port = config->srv.auth.port;
		}
	} else {
		if (SESSION_ONLINE == sess->state){
			packet_acct_update_req(config, sess, pbuf);
		} else if (SESSION_ONLINEPOST == sess->state) {
			packet_acct_start_req(config, sess, pbuf);
		} else {
			packet_acct_stop_req(config, sess, pbuf);
		}

		if (config->srv.backup_acct.ip &&
			sess->retryTimes >= ((sess->retryMaxTimes >> 1) + 1)) {
			radius_ip = config->srv.backup_acct.ip;
			radius_port = config->srv.backup_acct.port;
		} else {
			radius_ip = config->srv.acct.ip;
			radius_port = config->srv.acct.port;
		}
	}

	return sock_send_packet(radid->sock, radius_ip, radius_port, pbuf);
}

static inline void
_radius_sess_auth_fail(session_struct_t *sess) {
	session_opt_perform(sess, SESSOPT_AUTHFAIL);
	session_timer_destroy(sess, SESSION_RETRANS_TIMER);
	_manage_sess_exit(sess);
}

static inline void
_radius_sess_auth_success(session_struct_t *sess) {
	if (unlikely(session_opt_perform(sess, SESSOPT_AUTHSUCCESS))) 
		goto error;

	if (session_opt_perform(sess, SESSOPT_IPCPCONFREQ))
		goto error;

	session_timer_pause(sess, SESSION_RETRANS_TIMER);
	session_timer_update(sess, SESSION_TIMEOUT_TIMER, DEFAULT_SESSCONFIG_TIMEOUT);	/* wait ipcp config*/
	sess->state = SESSION_IPCPPOSTOPEN;
	return;

error:
	session_timer_destroy(sess, SESSION_RETRANS_TIMER);
	_manage_sess_exit(sess);
}

static int
radius_sess_retransport_func(thread_struct_t *thread) {
	session_struct_t *sess = thread_get_arg(thread);

	if (_radius_sess_check(sess))
		goto error;

	if (sess->retryTimes >= sess->retryMaxTimes) {
		_radius_sess_clean(sess);
		thread_pause_timer(thread);
		
		switch (sess->state) {
			case SESSION_AUTHPOST:
				pppoe_token_log(TOKEN_RADIUS, "session %d radius auth timeout\n", sess->sid);
				_radius_sess_auth_fail(sess);
				break;
				
			case SESSION_ONLINEPOST:
				pppoe_token_log(TOKEN_RADIUS, "session %d radius acct start timeout\n", sess->sid);
				session_opt_perform(sess, SESSOPT_ONLINESYNC);
				sess->state = SESSION_ONLINE;
				break;
				
			case SESSION_OFFLINEPOST:
				pppoe_token_log(TOKEN_RADIUS, "session %d radius acct stop timeout\n", sess->sid);
				sess->state = SESSION_OFFLINE;
				_manage_sess_exit(sess);
				break;
				
			case SESSION_ONLINE:
				pppoe_token_log(TOKEN_RADIUS, "session %d radius acct update timeout\n", sess->sid);
				break;
				
			default:
				pppoe_log(LOG_WARNING, "session %d unknow radius retransport\n", sess->sid);
				return PPPOEERR_EINVAL;
		}
		
		return PPPOEERR_ETIMEOUT;
	}	

	sess->retryTimes++;
	thread_update_timer(thread, DEFAULT_SESSRESTRANS_TIMEOUT, THREAD_EXTIME_NONE);
	pppoe_token_log(TOKEN_RADIUS, "session %u radius retransport %u times\n", sess->sid, sess->retryTimes);
	return _radius_sess_retransport(sess->priv_data, sess);

error:
	thread_pause_timer(thread);
	return PPPOEERR_EINVAL;
}

static inline int
radius_sess_retransport_setup(session_struct_t *sess) {
	int ret;

	if (sess->retrans) {
		ret = session_timer_update(sess, SESSION_RETRANS_TIMER, DEFAULT_SESSRESTRANS_TIMEOUT);
		if (unlikely(ret)) {
			pppoe_log(LOG_WARNING, "session %u radius retransport timer update failed\n", sess->sid);	
			goto out;
		}
	} else {
		ret = session_timer_init(sess, SESSION_RETRANS_TIMER,
						radius_sess_retransport_func, DEFAULT_SESSRESTRANS_TIMEOUT);
		if (unlikely(ret)) {
			pppoe_log(LOG_WARNING, "session %u radius retransport timer init failed\n", sess->sid);	
			goto out;
		}
	}

	sess->retryTimes = 0;
	sess->retryMaxTimes = 3;
	
out:
	return ret;
}


static int
process_acctResponse(struct radius_sock *sock, struct radius_id *radid,
							session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct radius_attr *attr;

	if (!radius_getattr(pbuf, &attr, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0)) {
		if (strlen(sess->acctSessID) != (attr->l - 2) || 
			memcmp(sess->acctSessID, attr->v.t, attr->l - 2)) {
			pppoe_log(LOG_WARNING, "acct response process: recv accID (%s) "
						"is not session accID (%s) error\n", attr->v.t, sess->acctSessID);
			return PPPOEERR_EINVAL;
		}
	}
	
	if (!radius_getattr(pbuf, &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		if (strlen(sess->username) != (attr->l - 2) || 
			memcmp(sess->username, attr->v.t, attr->l - 2)) {
			pppoe_log(LOG_WARNING, "acct response process: recv username (%s) "
						"is not session username (%s) error\n", attr->v.t, sess->username);
			return PPPOEERR_EINVAL;
		}
	}

	_radius_sess_clean(sess);
	session_timer_pause(sess, SESSION_RETRANS_TIMER);

	switch (sess->state) {			
		case SESSION_ONLINEPOST:
			pppoe_token_log(TOKEN_RADIUS, "session %d recv radius acct start response\n", sess->sid);
			session_opt_perform(sess, SESSOPT_ONLINESYNC);
			sess->state = SESSION_ONLINE;
			break;
			
		case SESSION_OFFLINEPOST:
			pppoe_token_log(TOKEN_RADIUS, "session %d recv radius acct stop response\n", sess->sid);
			sess->state = SESSION_OFFLINE;
			_manage_sess_exit(sess);
			break;
			
		case SESSION_ONLINE:
			pppoe_token_log(TOKEN_RADIUS, "session %d recv radius acct update response\n", sess->sid);
			break;
	}

	return PPPOEERR_SUCCESS;
}

static int
radius_sock_process(thread_struct_t *thread) {
	struct radius_sock *sock = thread_get_arg(thread);
	struct pppoe_buf *pbuf = pbuf_init(sock->pbuf);
	struct radius_srv *srv = &sock->config->srv;
	session_struct_t *sess;
	struct radius_packet *pack;
	struct radius_id *radid;
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);
	uint32 radius_ip;
	uint16 radius_port;
	int ret;

	if (sock->rdc) {
		ret = rdc_recvfrom(sock->rdc, sock->sk, pbuf, &addr, &len);
	} else {
		ret = _recv_packet(sock->sk, pbuf, &addr, &len);
	}

	if (ret) {
		pppoe_log(LOG_WARNING, "recv packet failed, err %d\n", ret);
		goto out;
	}
	
	if (unlikely(pbuf_may_pull(pbuf, sizeof(struct radius_packet)))) {
		pppoe_log(LOG_WARNING, "pkt length(%d) error\n", pbuf->len);
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	pack = (struct radius_packet *)pbuf->data;
	pbuf_pull(pbuf, sizeof(struct radius_packet));

	if (unlikely(pbuf_trim(pbuf, ntohs(pack->length) - sizeof(struct radius_packet)))) {
		pppoe_log(LOG_WARNING, "radius packet length %d error\n", ntohs(pack->length));
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	radid = &sock->id[pack->id];
	if (RADIUS_ID_USED != radid->status) {
		pppoe_log(LOG_WARNING, "recv fd(%d) id(%d) not used\n", sock->sk, pack->id);
		ret = PPPOEERR_ESTATE;
		goto out;
	}
	
	sess = radid->sess;
	radius_ip = ntohl(addr.sin_addr.s_addr);
	radius_port = ntohs(addr.sin_port);
	pppoe_token_log(TOKEN_RADIUS, "session %u recv radius %u.%u.%u.%u:%u %s\n", 
								sess->sid, HIPQUAD(radius_ip), radius_port,
								(!pack->code || pack->code > sizeof(radius_codenames) / sizeof(radius_codenames[0])) ?
								"UNKnown Packet" : radius_codenames[pack->code - 1]);

	switch (pack->code) {
		case RADIUS_CODE_ACCESS_ACCEPT:
		case RADIUS_CODE_ACCESS_REJECT:
			if (SESSION_AUTHPOST != sess->state) {
				radid->status = RADIUS_ID_FREE;
				radid->sess = NULL;	
				ret = PPPOEERR_ESTATE;
				goto out;
			}
			
			if (RADIUS_CODE_ACCESS_REQUEST != radid->req_pack.code) {
				ret = PPPOEERR_EINVAL;
				goto out;
			}

			if (radius_ip != srv->auth.ip && 
				radius_ip != srv->backup_auth.ip) {
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, radius ip %u.%u.%u.%u is neither "
										"auth ip %u.%u.%u.%u nor backup_auth ip %u.%u.%u.%u\n", sock->sk, pack->id,
										HIPQUAD(radius_ip), HIPQUAD(srv->auth.ip), HIPQUAD(srv->backup_auth.ip));
				ret = PPPOEERR_EADDR;
				goto out;
			}
			
			if (radius_port != srv->auth.port && 
				radius_port != srv->backup_auth.port) {
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, radius port %u is neither "
										"auth port %u nor backup_auth port %u\n", 
										sock->sk, pack->id, radius_port,
										srv->auth.port, srv->backup_auth.port);
				ret = PPPOEERR_EADDR;
				goto out;
			}

			if (radius_reply_check(pack, &radid->req_pack,
							srv->auth.secret, srv->auth.secretlen)) {
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, auth authenticator not match "
										"secret [%s], secretlen %u\n", sock->sk, pack->id, 
										srv->auth.secret, srv->auth.secretlen);
				ret = PPPOEERR_EINVAL;
				goto out;
			}

			_radius_sess_clean(sess);
			
			if (RADIUS_CODE_ACCESS_REJECT == pack->code) {
				pppoe_token_log(TOKEN_RADIUS, "session %d radius auth failure\n", sess->sid);
				_radius_sess_auth_fail(sess);
			} else {
				pppoe_token_log(TOKEN_RADIUS, "session %d radius auth success\n", sess->sid);
				_radius_sess_auth_success(sess);
			}
			break;
			

		case RADIUS_CODE_ACCOUNTING_RESPONSE:
			/*may need edit session state check*/
			if (SESSION_ONLINEPOST != sess->state && 
				SESSION_ONLINE != sess->state &&
				SESSION_OFFLINEPOST != sess->state) {
				radid->status = RADIUS_ID_FREE;
				radid->sess = NULL;	
				ret = PPPOEERR_ESTATE;
				goto out;
			}
			
			if (RADIUS_CODE_ACCOUNTING_REQUEST != radid->req_pack.code) {
				ret = PPPOEERR_EINVAL;
				goto out;
			}
			
			if (radius_ip != srv->acct.ip &&
				radius_ip != srv->backup_acct.ip) { 	
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, radius ip %u.%u.%u.%u is neither "
										"acct ip %u.%u.%u.%u nor backup_acct ip %u.%u.%u.%u\n", sock->sk, pack->id, 
										HIPQUAD(radius_ip), HIPQUAD(srv->acct.ip), HIPQUAD(srv->backup_acct.ip));			
				ret = PPPOEERR_EADDR;
				goto out;
			}

			if (radius_port != srv->acct.port && 
				radius_port != srv->backup_acct.port) {
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, radius port %u is neither "
										"acct port %u nor backup_acct port %u\n", 
										sock->sk, pack->id, radius_port,
										srv->acct.port, srv->backup_acct.port);
				ret = PPPOEERR_EADDR;
				goto out;
			}

			if (radius_reply_check(pack, &radid->req_pack,
							srv->acct.secret, srv->acct.secretlen)) {
				pppoe_log(LOG_WARNING, "radius sock process: fd %d id %d, acct authenticator not match "
										"secret [%s], secretlen %u\n", sock->sk, pack->id, 
										srv->acct.secret, srv->acct.secretlen);
				ret = PPPOEERR_EINVAL;
				goto out;
			}

			ret = process_acctResponse(sock, radid, sess, pbuf);
			break;
			
		default:
			ret = PPPOEERR_EINVAL;
			break;
	}

out:
	return ret;
}


static int 
radius_sock_init(struct radius_sock *sock) {
	struct sockaddr_in addr;
	int ret;

	sock->sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (unlikely(sock->sk < 0)) {
		pppoe_log(LOG_ERR, "Can`t create radius socket\n");
		ret = PPPOEERR_ESOCKET;
		goto error;
	}

	set_nonblocking(sock->sk);
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	if (sock->rdc) {
		addr.sin_addr.s_addr = htonl(sock->config->local_ip);
		pppoe_token_log(TOKEN_RADIUS, "sock bind %u.%u.%u.%u\n", 
									HIPQUAD(sock->config->local_ip));
	} else {
		addr.sin_addr.s_addr = htonl(sock->config->nasip);
		pppoe_token_log(TOKEN_RADIUS, "sock bind %u.%u.%u.%u\n",
									HIPQUAD(sock->config->nasip));
	}
	addr.sin_port = htons(0);
	
	if (bind(sock->sk, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		pppoe_log(LOG_ERR, "Can't bind to radius socket(%d)\n", sock->sk);
		ret = PPPOEERR_EBIND;
		goto error1;
	}

	sock->thread = thread_add_read(sock->master, radius_sock_process, sock, sock->sk);
	if (unlikely(!sock->thread)) {
		pppoe_log(LOG_ERR, "Can't add radius sock read thread\n");
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	sock_setup(sock);
	pppoe_token_log(TOKEN_RADIUS, "radius sock(%d) init success\n", sock->sk);
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(sock->sk);
error:
	return ret;
}

static void 
radius_sock_exit(struct radius_sock *sock) {
	THREAD_CANCEL(sock->thread);
	PPPOE_CLOSE(sock->sk);
	pppoe_token_log(TOKEN_RADIUS, "radius sock exit success\n");
}

static struct radius_sock *
radius_sock_create(radius_struct_t *radius) {
	struct radius_sock *sock;

	if (unlikely(!radius))
		return NULL;

	sock = mem_cache_alloc(radius->cache);
	if (unlikely(!sock)) {
		pppoe_log(LOG_WARNING, "radius sock create fail(mem alloc error)\n");
		return NULL;
	}

	memset(sock, 0, sizeof(struct radius_sock));
	sock->pbuf = radius->pbuf;
	sock->config = radius->config;
	sock->master = radius->master;
	sock->rdc = radius->rdc;

	list_add(&sock->next, &radius->sock_list);
	pppoe_token_log(TOKEN_RADIUS, "radius sock create success\n");
	return sock;
}

static void
radius_sock_destroy(radius_struct_t *radius, struct radius_sock *sock) {
	if (!sock)
		return;

	list_del(&sock->next);
	mem_cache_free(radius->cache, sock);
	pppoe_token_log(TOKEN_RADIUS, "radius sock destroy success\n");
}

static inline struct radius_id *
radius_get_free_id(radius_struct_t *radius) {
	struct radius_sock *sock;
	struct radius_id *radid;

	if (!list_empty(&radius->free_list)) {
		radid = list_entry(radius->free_list.next, struct radius_id, next);
		if (time_compare(time_sysup(), radid->freetime) >= 0) {
			list_del(&radid->next);
			goto out;
		} 
	}

	if (!list_empty(&radius->sock_list)) {
		sock = list_entry(radius->sock_list.next, struct radius_sock, next);
		if (0xff != sock->curr_id) {
			sock->curr_id++;
			radid = &sock->id[sock->curr_id];
			goto out;
		}
	}

	sock = radius_sock_create(radius);
	if (!sock) {
		pppoe_log(LOG_ERR, "radius get free id: radius sock create fail\n");
		goto error;
	}

	if (radius_sock_init(sock)) {
		pppoe_log(LOG_ERR, "radius get free id: radius sock init fail\n");
		goto error1;
	}

	sock->curr_id = 0;
	radid = &sock->id[0];

out:
	radid->freetime = time_sysup() + 24;
	list_add_tail(&radid->next, &radius->free_list);
	return radid;

error1:
	radius_sock_destroy(radius, sock);	
error:
	return NULL;	
}


static inline int
radius_req(radius_struct_t *radius, session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct radius_packet *pack = (struct radius_packet *)sess->pbuf->data;
	struct radius_id *radid = radius_get_free_id(radius);
	struct radius_srv *srv = &radius->config->srv;
	uint32 radius_ip;
	uint16 radius_port;

	if (!radid) {
		pppoe_log(LOG_ERR, "radius req: session %u, radius get free id fail\n", sess->sid);
		return PPPOEERR_ENOMEM;
	}
	
	if (RADIUS_CODE_ACCESS_REQUEST == pack->code) {
		radius_ip = srv->auth.ip;
		radius_port = srv->auth.port;
	} else {
		radius_ip = srv->acct.ip;
		radius_port = srv->acct.port;
	}

	_radius_sess_setup(radid, pack, sess);	
	return sock_send_packet(radid->sock, radius_ip, radius_port, pbuf);
}

static int
radius_auth_req(session_struct_t *sess, void *arg) {
	radius_struct_t *radius = (radius_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	int ret;

	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));
	radius_packet_init(pbuf, &sess->seed, RADIUS_CODE_ACCESS_REQUEST);
	packet_auth_req(radius->config, sess, pbuf);

	ret = radius_req(radius, sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u send radius auth request fail\n", sess->sid);	
		goto out;
	} 
	
	ret = radius_sess_retransport_setup(sess);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u retransport setup failed\n", sess->sid);	
		goto out;
	}
	
out:	
	return ret;	
}

static int
radius_acct_start(session_struct_t *sess, void *arg) {
	radius_struct_t *radius = (radius_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);	
	int ret;

	sess->onlineTime = time_sysup();	
	_radius_acctSessID_init(sess);
	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));
	radius_packet_init(pbuf, &sess->seed, RADIUS_CODE_ACCOUNTING_REQUEST);
	packet_acct_start_req(radius->config, sess, pbuf);

	ret = radius_req(radius, sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u send radius acct start fail\n", sess->sid);	
		goto out;
	}

	ret = radius_sess_retransport_setup(sess);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u retransport setup failed\n", sess->sid);	
		goto out;
	}
	
out:	
	return ret;
}

static int
radius_acct_stop(session_struct_t *sess, void *arg) {
	radius_struct_t *radius = (radius_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);	
	int ret;
	
	sess->offlineTime = time_sysup();
	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));
	radius_packet_init(pbuf, &sess->seed, RADIUS_CODE_ACCOUNTING_REQUEST);
	packet_acct_stop_req(radius->config, sess, pbuf);

	ret = radius_req(radius, sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u send radius acct stop fail\n", sess->sid);	
		goto out;
	}

	ret = radius_sess_retransport_setup(sess);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u retransport setup failed\n", sess->sid);	
		goto out;
	}
	
out:	
	return ret;
}

static int
radius_acct_stop_without_retransport(session_struct_t *sess, void *arg) {
	radius_struct_t *radius = (radius_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);

	sess->offlineTime = time_sysup();
	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));
	radius_packet_init(pbuf, &sess->seed, RADIUS_CODE_ACCOUNTING_REQUEST);
	packet_acct_stop_req(radius->config, sess, pbuf);
	return radius_req(radius, sess, pbuf);
}

static inline int
radius_acct_update(session_struct_t *sess, void *arg) {
	radius_struct_t *radius = (radius_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	int ret;

	pbuf_reserve(pbuf, sizeof(struct rdc_packet_t));	
	radius_packet_init(pbuf, &sess->seed, RADIUS_CODE_ACCOUNTING_REQUEST);
	packet_acct_update_req(radius->config, sess, pbuf);

	ret = radius_req(radius, sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u send radius acct update fail\n", sess->sid);	
		goto out;
	}

	ret = radius_sess_retransport_setup(sess);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u retransport setup failed\n", sess->sid);	
		goto out;
	}

out:	
	return ret;
}

static inline void
radius_setup(radius_struct_t *radius) {
	INIT_LIST_HEAD(&radius->sock_list);	

	if (radius->coa) {
		coa_proto_register(radius->coa, RADIUS_CODE_DISCONNECT_REQUEST, 
							radius, coa_disconnect_request);
	}	

	manage_sessions_opt_register(radius->manage, SESSOPT_AUTHREQ, radius, radius_auth_req);
	manage_sessions_opt_register(radius->manage, SESSOPT_ACCTSTART, radius, radius_acct_start);
	manage_sessions_opt_register(radius->manage, SESSOPT_ACCTSTOP, radius, radius_acct_stop);
	manage_sessions_opt_register(radius->manage, SESSOPT_ACCTSTOPV2, radius, radius_acct_stop_without_retransport);
	manage_sessions_opt_register(radius->manage, SESSOPT_ACCTUPDATE, radius, radius_acct_update);
}

static inline void
radius_unsetup(radius_struct_t *radius) {
	if (radius->coa) {
		coa_proto_unregister(radius->coa, RADIUS_CODE_DISCONNECT_REQUEST);
	}
	
	manage_sessions_opt_unregister(radius->manage, SESSOPT_AUTHREQ);
	manage_sessions_opt_unregister(radius->manage, SESSOPT_ACCTSTART);
	manage_sessions_opt_unregister(radius->manage, SESSOPT_ACCTSTOP);
	manage_sessions_opt_unregister(radius->manage, SESSOPT_ACCTSTOPV2);
	manage_sessions_opt_unregister(radius->manage, SESSOPT_ACCTUPDATE);
};

int
pppoe_radius_start(radius_struct_t *radius) {
	struct list_head *pos;
	int ret;
	
	list_for_each(pos, &radius->sock_list) {
		if (PPPOEERR_SUCCESS != (ret = radius_sock_init(list_entry(pos, struct radius_sock, next))))
			goto error;
	}

	INIT_LIST_HEAD(&radius->free_list);	
	pppoe_log(LOG_INFO, "radius start success\n");			
	return PPPOEERR_SUCCESS;

error:
	list_for_each(pos, &radius->sock_list) {
		radius_sock_exit(list_entry(pos, struct radius_sock, next));
	}
	pppoe_log(LOG_ERR, "radius start fail\n");			
	return ret;
}

void
pppoe_radius_stop(radius_struct_t *radius) {
	struct list_head *pos;
	list_for_each(pos, &radius->sock_list) {
		radius_sock_exit(list_entry(pos, struct radius_sock, next));
	}
	INIT_LIST_HEAD(&radius->free_list);
	pppoe_log(LOG_INFO, "radius stop success\n");			
}

radius_struct_t *
pppoe_radius_init(thread_master_t *master, 
		pppoe_manage_t *manage, radius_config_t *config) {
	radius_struct_t *radius;
	uint32 coa_ip;
	uint16 coa_port;

	if (unlikely(!master || !config)) {
		goto error;
	}

	radius = (radius_struct_t *)malloc(sizeof(radius_struct_t));
	if (unlikely(!radius)) {
		pppoe_log(LOG_ERR, "radius alloc failed\n");
		goto error;
	}

	memset(radius, 0, sizeof(radius_struct_t));
	radius->config = config;
	radius->master = master;
	radius->manage = manage;
	radius->cache = mem_cache_create(RADIUS_CACHE_NAME, 
							RADIUS_CACHE_MAX_BLKNUM, RADIUS_CACHE_EMPTY_BLKNUM, 
							sizeof(struct radius_sock), RADIUS_CACHE_BLK_ITEMNUM);
	if (unlikely(!radius->cache)) {
		pppoe_log(LOG_ERR, " mem cache create failed\n");
		goto error1;
	}

	radius->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!radius->pbuf)) {
		pppoe_log(LOG_ERR, " pppoe buf alloc failed\n");
		goto error2;
	}

	if (config->rdc_state) {
		if (HANSI_LOCAL == config->local_id) {
			coa_ip = SLOT_IPV4_BASE + 100 + config->instance_id;
			coa_port = 0;		
		} else {
			coa_ip = SLOT_IPV4_BASE + config->slot_id;
			coa_port = 0;		
		}

		radius->rdc = rdc_client_init(config->slot_id, config->local_id, 
								config->instance_id, RDC_TYPE_PPPOE,
								config->rdc_slotid, config->rdc_insid, NULL);
		if (unlikely(!radius->rdc)) {
			pppoe_log(LOG_ERR, "radius rdc client init failed\n");
			goto error3;
		}

		/* coa must by rdc forward */
		radius->coa = radius_coa_init(radius->master, 
								radius->rdc, coa_ip, &coa_port,
								coa_send_packet, coa_recv_packet);
		if (unlikely(!radius->coa)) {
			pppoe_log(LOG_ERR, "radius coa init failed\n");
			goto error4;
		}

		rdc_client_setup(radius->rdc, coa_port);
	} else {
		pppoe_log(LOG_NOTICE, "radius rdc not config, so coa can not be used\n");
	}

	radius_setup(radius);
	pppoe_log(LOG_INFO, "radius init success\n");			
	return radius;

error4:
	if (radius->rdc)
		rdc_client_exit(&radius->rdc);
error3:
	PBUF_FREE(radius->pbuf);
error2:
	mem_cache_destroy(&radius->cache);
error1:
	PPPOE_FREE(radius);
error:
	return NULL;
}

void
pppoe_radius_destroy(radius_struct_t **radius){
	struct list_head *pos, *n;

	if (unlikely(!radius || !(*radius)))
		return;
	
	list_for_each_safe(pos, n, &(*radius)->sock_list) {
		radius_sock_destroy(*radius, list_entry(pos, struct radius_sock, next));
	}

	radius_unsetup(*radius);
	
	if ((*radius)->coa)
		radius_coa_exit(&(*radius)->coa);
	
	if ((*radius)->rdc)
		rdc_client_exit(&(*radius)->rdc);
	
	mem_cache_destroy(&(*radius)->cache);
	PBUF_FREE((*radius)->pbuf);
	PPPOE_FREE(*radius);
	pppoe_log(LOG_INFO, "radius destroy success\n");			
}


int
pppoe_radius_config_nas_ipaddr(radius_config_t *config, uint32 nasip) {
	config->nasip = nasip;
	return PPPOEERR_SUCCESS;
}

int
pppoe_radius_config_rdc(radius_config_t *config, uint32 state, 
								uint32 slot_id, uint32 instance_id) {
	config->rdc_state = state;
	config->rdc_slotid = slot_id;
	config->rdc_insid = instance_id;
	return PPPOEERR_SUCCESS;
}

int
pppoe_radius_config_auth_and_acct_server(radius_config_t *config, struct radius_srv *srv) {
	memset(&config->srv, 0, sizeof(struct radius_srv));

	if (srv) {
		memcpy(&config->srv, srv, sizeof(struct radius_srv));
	} 

	return PPPOEERR_SUCCESS;
}

int
pppoe_radius_show_running_config(radius_config_t *config, char *cmd, uint32 len) {
	char *cursor = cmd;

	if (config->nasip) {
		cursor += snprintf(cursor, len - (cursor - cmd), 
						" nas ip address %u.%u.%u.%u\n", HIPQUAD(config->nasip));
	}

	if (config->rdc_state) {
		cursor += snprintf(cursor, len - (cursor - cmd), 
						" radius rdc %u-%u\n", config->rdc_slotid, config->rdc_insid);
	}

	if (config->srv.auth.ip) {
		if (config->srv.backup_auth.ip) {
			cursor += snprintf(cursor, len - (cursor - cmd), 
							" radius server auth %u.%u.%u.%u %u %s acct %u.%u.%u.%u %u %s "
							"backup-auth %u.%u.%u.%u %u %s backup-acct %u.%u.%u.%u %u %s\n", 
							HIPQUAD(config->srv.auth.ip), config->srv.auth.port, config->srv.auth.secret,
							HIPQUAD(config->srv.acct.ip), config->srv.acct.port, config->srv.acct.secret,
							HIPQUAD(config->srv.backup_auth.ip), config->srv.backup_auth.port, config->srv.backup_auth.secret,
							HIPQUAD(config->srv.backup_acct.ip), config->srv.backup_acct.port, config->srv.backup_acct.secret);
		} else {
			cursor += snprintf(cursor, len - (cursor - cmd), 
							" radius server auth %u.%u.%u.%u %u %s acct %u.%u.%u.%u %u %s\n",
							HIPQUAD(config->srv.auth.ip), config->srv.auth.port, config->srv.auth.secret,
							HIPQUAD(config->srv.acct.ip), config->srv.acct.port, config->srv.acct.secret);
		}
	}

	return cursor - cmd;
}


void
pppoe_radius_config_setup(radius_config_t *config, 
			uint32 slot_id, uint32 local_id, uint32 instance_id) {
	config->slot_id = slot_id;
	config->local_id = local_id;
	config->instance_id = instance_id;

	if (HANSI_LOCAL == config->local_id) {
		config->local_ip = SLOT_IPV4_BASE + 100 + config->instance_id;
	} else {
		config->local_ip = SLOT_IPV4_BASE + config->slot_id;
	}
}

int
pppoe_radius_config_init(radius_config_t **config) {
	if (unlikely(!config))
		return PPPOEERR_EINVAL;

	*config = (radius_config_t *)malloc(sizeof(radius_config_t));
	if (unlikely(!(*config)))
		return PPPOEERR_ENOMEM;

	memset(*config, 0, sizeof(radius_config_t));
	return PPPOEERR_SUCCESS;
}

void
pppoe_radius_config_exit(radius_config_t **config) {
	if (unlikely(!config))
		return;

	PPPOE_FREE(*config);
}

