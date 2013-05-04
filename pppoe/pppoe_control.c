#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
#include "pppoe_util.h"
#include "mem_cache.h"
#include "pppoe_buf.h"
#include "notifier_chain.h"
#include "pppoe_thread.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"

#include "pppoe_ppp.h"
#include "pppoe_lcp.h"
#include "pppoe_chap.h"
#include "pppoe_ipcp.h"
#include "pppoe_ccp.h"

#include "pppoe_control.h"

struct control_config {
	/* control public config, from device config */
	char ifname[IFNAMSIZ];
};

struct control_struct {
	int sk;
	struct pppoe_buf *pbuf;
	
	control_config_t 	*config;
	pppoe_manage_t 		*manage;
	thread_master_t 	*master;
	thread_struct_t		*thread;
};

static char *ppp_codenames[] = {
	"Configuration Request", 
	"Configuration Ack",
	"Configuration Nak",
	"Configuration Reject",
	"Termination Request",
	"Termination Ack",
	"Code Reject",
	"Protocol Reject",
	"Echo Request",
	"Echo Reply",
	"Discard Request",
	"Identification",
	"Time Remaining",
	"UNUSED"
	"LCP OPEN",
	"LCP Close"
};

static inline int
send_lcpConfReq(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = lcpConfigReq_packet(sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d packet CCP ConReq fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send CCP config request fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int
send_lcpEchoReq(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = lcpEchoReq_packet(sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d LCP Echo request packet fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d LCP Echo request fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int
send_lcpTermReq(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	sess->termTimes++;
	if (sess->termTimes > sess->termMaxTimes) {
		ret = PPPOEERR_ETIMEOUT;
		goto error;
	}

	ret = lcpTerm_packet(sess, pbuf, 0/*term request */);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send LCP term request: term packet fail\n", sess->sid);
		goto error;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send LCP term request: send packet fail\n", sess->sid);
		goto error;
	}

	sess->state = SESSION_LCPPOSTCLOSE;
	return PPPOEERR_SUCCESS;

error:
	pppoe_token_log(TOKEN_CONTROL, "session %d LCP Close\n", sess->sid);
	session_opt_unregister(sess, SESSOPT_LCPTERMREQ);
	sess->state = SESSION_LCPCLOSE;
	return ret;
}


static inline int
send_ipcpConfReq(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;
	
	ret = ipcpConfigReq_packet(sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u packet IPCP ConReq fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u send IPCP config request fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int
send_ccpConfReq(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = ccpConfigReq_packet(sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d packet CCP ConReq fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);
	
	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send CCP config request fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int
send_chapChallenge(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = chapChallenge_packet(sess, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d packet CHAP Challenge fail\n", sess->sid);
		return ret;
	}
	
	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send CHAP Challenge fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int 
send_chapSuccess(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = chapResult_packet(sess, pbuf, 0/* success*/);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d packet CHAP Success fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send CHAP Success fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline int 
send_chapFail(session_struct_t *sess, void *arg) {
	control_struct_t *ctl = (control_struct_t *)arg;
	struct pppoe_buf *pbuf = pbuf_init(sess->pbuf);
	struct sockaddr_pppoe poaddr;
	int ret;

	ret = chapResult_packet(sess, pbuf, 1/* success*/);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d packet CHAP Fail fail\n", sess->sid);
		return ret;
	}

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = ntohs(sess->sid);
	memcpy(poaddr.addr.mac, sess->mac, ETH_ALEN);

	ret = ppp_send_packet(ctl->sk, &poaddr, pbuf);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %d send CHAP Fail fail\n", sess->sid);
		return ret;
	}

	return PPPOEERR_SUCCESS;
}

static inline void
control_sess_setup(control_struct_t *ctl, session_struct_t *sess) {
	session_opt_register(sess, SESSOPT_LCPTERMREQ, ctl, send_lcpTermReq);
	
	switch (sess->auth_type) {
		case AUTH_NONE:
			break;
		
		case AUTH_PAP:
			break;

		case AUTH_CHAP:
			session_opt_register(sess, SESSOPT_AUTHPRE, ctl, send_chapChallenge);
			session_opt_register(sess, SESSOPT_AUTHSUCCESS, ctl, send_chapSuccess);
			session_opt_register(sess, SESSOPT_AUTHFAIL, ctl, send_chapFail);
			break;

		case AUTH_EAP:
			break;
	}
}

static int
control_process(thread_struct_t *thread) {
	control_struct_t *ctl = thread_get_arg(thread);
	struct sockaddr_pppoe saddr;
	session_struct_t *sess;
	struct pppoe_ctl *pctl;
	struct pppoe_buf *pbuf = pbuf_init(ctl->pbuf);
	uint16 proto;
	int ret;

	if (PPPOEERR_SUCCESS != 
		(ret = ppp_recv_packet(ctl->sk, &saddr, pbuf))) {
		pppoe_log(LOG_WARNING, "recv packet fail\n");
		goto out;
	}

	sess = __manage_sess_get_by_sid(ctl->manage, ntohs(saddr.addr.sid));
	if (!sess) {
		pppoe_log(LOG_WARNING, "sess %d is not exist\n", saddr.addr.sid);
		ret = PPPOEERR_ENOEXIST;
		goto out;
	}

	if (memcmp(sess->mac, saddr.addr.mac, ETH_ALEN)) {
		pppoe_log(LOG_WARNING, "sess %d mac addr is error\n", saddr.addr.sid);
		ret = PPPOEERR_EADDR;
		goto out;
	}

	if (unlikely(pbuf_may_pull(pbuf, PPP_HEADER_LEN + sizeof(struct pppoe_ctl)))) {
		pppoe_log(LOG_WARNING, "packet length(%d) error\n", pbuf->len);
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	proto = GETSHORT(pbuf->data);	
	pctl = (struct pppoe_ctl * )pbuf_pull(pbuf, PPP_HEADER_LEN);

	ret = ppp_proto_process(proto, sess, pbuf);
	pppoe_token_log(TOKEN_CONTROL, "proto 0x%04x process return %d\n", proto, ret);

	switch (ret) {
	case 0:
		goto state_check;
	
	case CONFREQ:
		pctl->ident = ++sess->ident;
	case CONFACK:			
	case CONFNAK:
	case CONFREJ:
	case CODEREJ:
	case TERMREQ:
	case TERMACK:
		pctl->code = ret;
		pctl->length = htons(pbuf->len + sizeof(struct pppoe_ctl));
		pbuf_push(pbuf, PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		ret = ppp_send_packet(ctl->sk, &saddr, pbuf);
		pppoe_token_log(TOKEN_CONTROL, "send 0x%04x %s packet, "
								"ident %d, session %d, return %d\n", 
								proto, ppp_codenames[pctl->code - 1], 
								pctl->ident, sess->sid, ret);
		break;
			
	default:
		goto out;
	}

state_check:
	switch (sess->state) {
		case SESSION_LCPPREOPEN:
			if (session_opt_perform(sess, SESSOPT_LCPCONFREQ)) {
				_manage_sess_exit(sess);
				break;
			}
			
			session_timer_update(sess, SESSION_TIMEOUT_TIMER, DEFAULT_SESSCONFIG_TIMEOUT); /* wait lcp config*/
			sess->state = SESSION_LCPPOSTOPEN;
			pppoe_token_log(TOKEN_CONTROL, "send LCP Configuration Request packet, "
									"ident %d, session %d\n", pctl->ident, sess->sid);
			break;
			
		case SESSION_LCPOPEN:
			pppoe_token_log(TOKEN_CONTROL, "session %d LCP Open\n", sess->sid);
			control_sess_setup(ctl, sess);
			session_opt_perform(sess, SESSOPT_AUTHPRE);
			session_timer_update(sess, SESSION_TIMEOUT_TIMER, DEFAULT_SESSCONFIG_TIMEOUT);	/*wait auth 10s*/	
			sess->state = SESSION_AUTHPRE;
			pppoe_token_log(TOKEN_CONTROL, "send CHAP Chanllenge packet, "
							"ident %d, session %d\n", pctl->ident, sess->sid);
			break;

		case SESSION_AUTH:
			pppoe_token_log(TOKEN_CONTROL, "session %d radius auth start\n", sess->sid);
			if (session_opt_perform(sess, SESSOPT_AUTHREQ)) {
				_manage_sess_exit(sess);
				break;
			}
			
			sess->state = SESSION_AUTHPOST;
			break;

		case SESSION_IPCPOPEN:
			pppoe_token_log(TOKEN_CONTROL, "session %d IPCP OPEN\n", sess->sid);
			session_opt_perform(sess, SESSOPT_IPREGISTER);
			session_timer_update(sess, SESSION_TIMEOUT_TIMER, DEFAULT_SESSSYSCALL_TIMEOUT);	/* wait session ipaddr register*/
			sess->state = SESSION_ONLINEPRE;
			break;
			
		case SESSION_IPCPCLOSE:
			pppoe_token_log(TOKEN_CONTROL, "session %d IPCP CLOSE\n", sess->sid);
			_manage_sess_exit(sess);
			break;
						
		case SESSION_LCPPRECLOSE:
			_manage_sess_exit(sess);
			break;		
			
		case SESSION_LCPCLOSE:
			pppoe_token_log(TOKEN_CONTROL, "session %d LCP Close\n", sess->sid);
			_manage_sess_exit(sess);
			break;
		
	}

out:
	return ret;
}

static int
control_sock_init(control_struct_t *control) {
	struct sockaddr_pppoe saddr;
	int ret;
	
	if (unlikely(!control->config->ifname[0])) {
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	control->sk = socket(AF_PPPOE, SOCK_RAW, 0);
	if (unlikely(control->sk < 0)) {
		pppoe_log(LOG_WARNING, "creat pppoe control socket fail\n");
		ret = PPPOEERR_ESOCKET;
		goto error;
	}

	pppoe_token_log(TOKEN_CONTROL, "create control socket is %d\n", control->sk);

	set_nonblocking(control->sk);
	
	memset(&saddr, 0, sizeof(saddr));
	saddr.sa_family = AF_PPPOE;
	strncpy(saddr.addr.dev, control->config->ifname, sizeof(saddr.addr.dev) - 1);
	if (bind(control->sk, (struct sockaddr*)&saddr, sizeof(saddr))) {
		pppoe_log(LOG_WARNING, "bind pppoe control sockaddr fail\n");
		ret = PPPOEERR_EBIND;
		goto error1;
	}

	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(control->sk);
error:
	return ret;
}

static inline void
control_setup(control_struct_t *control) {
	manage_sessions_opt_register(control->manage, SESSOPT_LCPCONFREQ, control, send_lcpConfReq);
	manage_sessions_opt_register(control->manage, SESSOPT_LCPECHOREQ, control, send_lcpEchoReq);
	manage_sessions_opt_register(control->manage, SESSOPT_LCPTERMREQ, control, send_lcpTermReq);	
	manage_sessions_opt_register(control->manage, SESSOPT_CCPCONFREQ, control, send_ccpConfReq);
	manage_sessions_opt_register(control->manage, SESSOPT_IPCPCONFREQ, control, send_ipcpConfReq);
}

static inline void
control_unsetup(control_struct_t *control) {
	manage_sessions_opt_unregister(control->manage, SESSOPT_LCPCONFREQ);
	manage_sessions_opt_unregister(control->manage, SESSOPT_LCPECHOREQ);
	manage_sessions_opt_unregister(control->manage, SESSOPT_LCPTERMREQ);
	manage_sessions_opt_unregister(control->manage, SESSOPT_CCPCONFREQ);
	manage_sessions_opt_unregister(control->manage, SESSOPT_IPCPCONFREQ);
}

int
pppoe_control_start(control_struct_t *control) {
	int ret;

	if (unlikely(!control))
		return PPPOEERR_EINVAL;

	ret = control_sock_init(control);
	if (ret) {
		pppoe_log(LOG_ERR, "control socket init failed\n");
		goto error;
	}

	control->thread = thread_add_read(control->master, 
									control_process, 
									control, control->sk);
	if (!control->thread) {
		pppoe_log(LOG_ERR, "control thread add read failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}
	
	pppoe_log(LOG_INFO, "control start sucess\n");		
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(control->sk);
error:
	return ret;
}

int
pppoe_control_stop(control_struct_t *control) {
	if (unlikely(!control))
		return PPPOEERR_EINVAL;

	THREAD_CANCEL(control->thread);
	PPPOE_CLOSE(control->sk);	
	pppoe_log(LOG_INFO, "control stop sucess\n");		
	return PPPOEERR_SUCCESS;
}



control_struct_t * 
pppoe_control_init(thread_master_t *master,
					pppoe_manage_t *manage, 
					control_config_t *config){
	control_struct_t *control;
	
	if (unlikely(!config ||!master || !manage))
		return NULL;

	control = (control_struct_t *)malloc(sizeof(control_struct_t));
	if (unlikely(!control)) {
		pppoe_log(LOG_ERR, "malloc pppoe socket fail\n");
		return NULL;
	}
	
	memset(control, 0, sizeof(*control));
	control->config = config;
	control->master = master;
	control->manage = manage;
	control->sk = -1;

	control->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!control->pbuf)) {
		pppoe_log(LOG_ERR, "malloc pppoe pbuf fail\n");
		PPPOE_FREE(control);
		return NULL;
	}

	control_setup(control);
	pppoe_log(LOG_INFO, "control init sucess\n");		
	return control;
}

/* before destroy must stop control */
void
pppoe_control_destroy(control_struct_t **control) {
	if (unlikely(!control || !(*control)))
		return ;

	control_unsetup(*control);
	PBUF_FREE((*control)->pbuf);
	PPPOE_FREE(*control);
	pppoe_log(LOG_INFO, "control destroy sucess\n");	
}

int
pppoe_control_show_running_config(control_config_t *ctlConf, char *cmd, uint32 len) {
	char *cursor = cmd;

	return cursor - cmd;
}

void
pppoe_control_config_setup(control_config_t *config, char *ifname) {
	strncpy(config->ifname, ifname, sizeof(config->ifname) - 1);
}

int
pppoe_control_config_init(control_config_t **config) {
	if (unlikely(!config))
		return PPPOEERR_EINVAL;

	*config = (control_config_t *)malloc(sizeof(control_config_t));
	if (unlikely(!(*config))) {
		pppoe_log(LOG_ERR, "pppoe control config alloc fail\n");
		return PPPOEERR_ENOMEM;
	}

	memset(*config, 0, sizeof(control_config_t));
	return PPPOEERR_SUCCESS;}

void
pppoe_control_config_exit(control_config_t **config){
	if (unlikely(!config))
		return;

	PPPOE_FREE(*config);
}

