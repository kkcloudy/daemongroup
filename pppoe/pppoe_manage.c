
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "pppoe_method_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "mem_cache.h"
#include "thread_bus.h"
#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_netlink.h"

#include "pppoe_manage.h"

enum {
	MANAGE_SYNC_NONE,
	MANAGE_SYNC_FINISHED,
};


struct online_sync {
	uint32 sid;
	uint8 mac[ETH_ALEN];
	uint8 serverMac[ETH_ALEN];
	uint32 serverIP;
	struct ipaddr_struct ipaddr;

	long startTime;
	long onlineTime;

	uint32 auth_type;
	uint32 chap_mdtype;
	
	char sname[PPPOE_NAMELEN];	
	uint8 magic[MAGIC_LEN];
	uint8 peermagic[MAGIC_LEN];
	
	uint8 challenge[RADIUS_CHAP_CHAL_LEN];
	uint8 passwd[RADIUS_PASSWORD_LEN];
	char username[USERNAMESIZE];
	char acctSessID[ACCT_SESSIONIDSIZE];

	struct session_stat stat;
};

struct offline_sync {
	uint32 sid;
	uint8 mac[ETH_ALEN];
};

struct update_sync {
	uint32 sid;
	uint8 mac[ETH_ALEN];
	struct session_stat stat;
};

struct manage_config {
	/* manage public config, from device config */
	uint32 slot_id;
	uint32 local_id;
	uint32 instance_id;
	
	int ifindex;
	uint32 dev_id;
	uint32 ipaddr;
	char ifname[IFNAMSIZ];

	/* manage private config, need show running */		
	uint32 max_sid;
	uint32 minIP, maxIP;
	uint32 dns1, dns2;
};

struct pppoe_manage {
	int sk;

	uint32 sync_flag;	/* standby check session sync finished */
	
	struct pppoe_buf *pbuf;
	struct manage_config *config;
	
	thread_master_t	*master;
	thread_struct_t	*thread;
	thread_struct_t *update;
	backup_struct_t *backup;
	ippool_struct_t	*ippool;
	pppoe_sessions_t *sessions;
	tbus_connection_t *connection;	

	struct list_head sess_dead;
};

static inline int
sess_stat_update(pppoe_manage_t *manage) {
	struct session_info *s_info;
	int ret;

	s_info = (struct session_info *)malloc(sizeof(struct session_info));
	if (unlikely(!s_info)) {
		pppoe_log(LOG_WARNING, "session info malloc fail\n");
		return PPPOEERR_ENOMEM;
	}

	memset(s_info, 0, sizeof(*s_info));
	memcpy(s_info->ifname, manage->config->ifname, IFNAMSIZ);

	ret = tbus_send_signal(manage->connection, 0, 
							PPPOE_METHOD_SESSION_STAT_UPDATE, 
							s_info, manage, free, 0);
	
	pppoe_token_log(TOKEN_MANAGE, "send signal, ret %d\n", ret);
	return ret;
}

static inline int
sess_get_wireless_info(pppoe_manage_t *manage, session_struct_t *sess) {
	struct session_wireless_info *sess_info
		= (struct session_wireless_info *)malloc(sizeof(struct session_wireless_info));
	if (unlikely(!sess_info)) {
		pppoe_log(LOG_WARNING, "session get wireless info: session info malloc fail\n");
		return PPPOEERR_ENOMEM;
	}

	memset(sess_info, 0, sizeof(*sess_info));
	sess_info->info.sid = sess->sid;
	sess_info->info.local_id = manage->config->local_id;;
	sess_info->info.instance_id = manage->config->instance_id;;
	memcpy(sess_info->info.mac, sess->mac, ETH_ALEN);
	
	return tbus_send_signal(manage->connection, 0, 
							PPPOE_METHOD_SESSION_WIRELESS_INFO_SHOW, 
							sess_info, manage, free, 0);
}

static inline int
sess_backup_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf) {
	return tbus_send_signal(manage->connection, 0, 
							PPPOE_METHOD_INSTANCE_BACKUP_TASK, 
							pbuf, manage->backup, 
							(tbusDataFree)pbuf_free, TBUS_FLAG_ERRFREE);
}

static inline int
sess_echo_func(thread_struct_t *thread) {
	session_struct_t *sess = thread_get_arg(thread);

	pppoe_token_log(TOKEN_MANAGE, "Session %d: Echo lose times is %d\n", sess->sid, sess->echoLoseTimes);
	
	if (sess->echoLoseTimes >= 3) {		
		pppoe_log(LOG_INFO, "session %d echo timout %d times, so exit\n", 
					sess->sid, sess->echoLoseTimes);
		
		session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
		manage_sess_exit(sess);
		return PPPOEERR_ETIMEOUT;
	}

	sess->echoLoseTimes++;
	session_opt_perform(sess, SESSOPT_LCPECHOREQ);
	thread_update_timer(thread, DEFAULT_ECHOUPDATE_TIMEOUT, THREAD_EXTIMER2);
	return PPPOEERR_SUCCESS;
}

static inline int
sess_timeout_func(thread_struct_t *thread) {
	session_struct_t *sess = thread_get_arg(thread);	

	if (unlikely(!sess))
		return PPPOEERR_EINVAL;

	pppoe_token_log(TOKEN_MANAGE, "session %u timeout, so exit\n", sess->sid);
	_manage_sess_exit(sess);
	return PPPOEERR_SUCCESS;
}

static inline int
sess_stat_update_func(thread_struct_t *thread) {
	pppoe_manage_t *manage = thread_get_arg(thread);
	thread_update_timer(thread, DEFAULT_STATUPDATE_TIMEOUT, THREAD_EXTIMER1);	/*update timer need from manage config*/
	return sess_stat_update(manage);
}

static inline int
sess_acct_update_func(thread_struct_t *thread) {
	session_struct_t *sess = thread_get_arg(thread);
	thread_update_timer(thread, DEFAULT_ACCTUPDATE_TIMEOUT, THREAD_EXTIMER3);	/*update timer need from manage config*/
	session_opt_perform(sess, SESSOPT_UPDATESYNC);
	return session_opt_perform(sess, SESSOPT_ACCTUPDATE);
}

static inline int
sess_online(session_struct_t *sess) {
	int ret;

	session_timer_pause(sess, SESSION_TIMEOUT_TIMER);

	ret = session_timer_init(sess, SESSION_ECHO_TIMER, sess_echo_func, DEFAULT_ECHOUPDATE_TIMEOUT);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u echo timer init fail\n", sess->sid);
		goto error;
	}
	
	ret = session_timer_init(sess, SESSION_UPDATE_TIMER, sess_acct_update_func, DEFAULT_ACCTUPDATE_TIMEOUT);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u acct update timer init fail\n", sess->sid);
		goto error1;
	}

	ret = session_opt_perform(sess, SESSOPT_ACCTSTART);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "session %u acct start request fail\n", sess->sid);
		goto error2;
	}

	sess->state = SESSION_ONLINEPOST;
	pppoe_log(LOG_INFO, "session %u %02X:%02X:%02X:%02X:%02X:%02X "
						"%u.%u.%u.%u online\n", sess->sid,
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5], 
						HIPQUAD(sess->ipaddr.ip));
	
	return PPPOEERR_SUCCESS;

error2:
	session_timer_destroy(sess, SESSION_UPDATE_TIMER);
error1:
	session_timer_destroy(sess, SESSION_ECHO_TIMER);
error:
	return ret;
}

static inline int
sess_offline(session_struct_t *sess) {
	int ret;
	
	session_timer_destroy(sess, SESSION_UPDATE_TIMER);
	session_timer_destroy(sess, SESSION_ECHO_TIMER);

	ret = session_opt_perform(sess, SESSOPT_ACCTSTOP);
	if (unlikely(ret)) {
		pppoe_log(LOG_WARNING, "Session Offline: session %u acct stop request fail\n", sess->sid);
		sess->state = SESSION_OFFLINE;
		goto out;
	}

	sess->state = SESSION_OFFLINEPOST;
	
out:	
	session_opt_perform(sess, SESSOPT_OFFLINESYNC);	
	pppoe_log(LOG_INFO, "session %u %02X:%02X:%02X:%02X:%02X:%02X "
						"%u.%u.%u.%u offline\n", sess->sid,
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5], 
						HIPQUAD(sess->ipaddr.ip));	
	return ret;
}

static inline void
__sess_free(pppoe_manage_t *manage, session_struct_t *sess) {
	if (sess->ipaddr.ip) {
		ipaddr_recover(manage->ippool, &sess->ipaddr);
	}
	
	session_free(manage->sessions, sess);
}

static inline void
sess_dead_free(pppoe_manage_t *manage) {
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, &manage->sess_dead) {
		session_struct_t *sess
			= list_entry(pos, session_struct_t, next);
		if (!sess->refcnt) {
			list_del(&sess->next);
			__sess_free(manage, sess);
		}
	}
}

/**********************************************************************************
 *  sess_free
 *
 *	DESCRIPTION:
 * 		free dead session. 
 *		if session being held by other thread, 
 *		add session to manage dead list;
 *
 *	INPUT:
 *		pppoe_manage_t *manage
 *		session_struct_t *sess
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 **********************************************************************************/
static inline void
sess_free(pppoe_manage_t *manage, session_struct_t *sess) {
	if (sess->refcnt) {
		list_add(&sess->next, &manage->sess_dead);
	} else {
		__sess_free(manage, sess);
	}
	sess_dead_free(manage);
}


/**********************************************************************************
 *  sess_exit
 *
 *	DESCRIPTION:
 * 		session exit.
 *		frist all of session timer will closed,
 *		then remove session from manage hash table, 
 *		end free it.
 *
 *	INPUT:
 *		pppoe_manage_t *manage
 *		session_struct_t *sess
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 **********************************************************************************/
static inline void
sess_exit(pppoe_manage_t *manage, session_struct_t *sess) {
	session_timer_clear(sess);
	session_unregister(manage->sessions, sess);
	sess_free(manage, sess);
}

static int
sessopt_exit(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;
	if (unlikely(!manage))
		return PPPOEERR_EINVAL;

	netlink_channel_unregister(manage->sk, pbuf_init(manage->pbuf), 
								manage->config->ifindex, 
								sess->sid, sess->mac);
	sess_exit(manage, sess);
	return PPPOEERR_SUCCESS;
}

static int
sessopt_ipapply(session_struct_t *sess, void *arg) {
	return ipaddr_apply(((pppoe_manage_t *)arg)->ippool, &sess->ipaddr);
}

static int
sessopt_iprecover(session_struct_t *sess, void *arg) {
	return ipaddr_recover(((pppoe_manage_t *)arg)->ippool, &sess->ipaddr);
}

static int
sessopt_ipregister(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;

	if (unlikely(!sess->ipaddr.ip))
		return PPPOEERR_EINVAL;

	return netlink_channel_authorize(manage->sk, pbuf_init(manage->pbuf),
						manage->config->ifindex, sess->sid, sess->ipaddr.ip);
}

static int
sessopt_ipunregister(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;

	if (unlikely(!sess->ipaddr.ip))
		return PPPOEERR_EINVAL;

	session_timer_destroy(sess, SESSION_ECHO_TIMER);
	session_timer_destroy(sess, SESSION_RETRANS_TIMER);
	session_timer_destroy(sess, SESSION_UPDATE_TIMER);
	return netlink_channel_unauthorize(manage->sk, pbuf_init(manage->pbuf),
						manage->config->ifindex, sess->sid, sess->ipaddr.ip);
}

static int
sessopt_onlinesync(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;
	struct instance_sync *sync;
	struct online_sync *online;
	struct pppoe_buf *pbuf;
	long upTime;

	if (BACKUP_ACTIVE != backup_status(manage->backup))
		return PPPOEERR_SUCCESS;

	if (!backup_channel_state(manage->backup)) {
		pppoe_token_log(TOKEN_MANAGE, "backup channel is not connect\n");
		return PPPOEERR_ESTATE;
	}

	pbuf = pbuf_alloc(BACKUP_HEADER_LEN + 
					sizeof(struct instance_sync) + 
					sizeof(struct online_sync));
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		return PPPOEERR_ENOMEM;
	}

	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	memset(sync, 0, sizeof(struct instance_sync));
	sync->code = INSTANCE_SESSONLINE_SYNC;
	sync->dev_id = manage->config->dev_id;

	online = (struct online_sync *)pbuf_put(pbuf, sizeof(struct online_sync));
	memset(online, 0, sizeof(struct online_sync));	
	online->sid = sess->sid;
	online->ipaddr = sess->ipaddr;	
	online->serverIP = sess->serverIP;
	online->auth_type = sess->auth_type;
	online->chap_mdtype = sess->chap_mdtype;
	online->stat = sess->stat;

	upTime = time_sysup();
	online->startTime = upTime - sess->startTime;
	online->onlineTime = upTime - sess->onlineTime;

	memcpy(online->mac, sess->mac, sizeof(online->mac));
	memcpy(online->serverMac, sess->serverMac, sizeof(online->serverMac));
	memcpy(online->sname, sess->sname, sizeof(online->sname));
	memcpy(online->magic, sess->magic, sizeof(online->magic));
	memcpy(online->peermagic, sess->peermagic, sizeof(online->peermagic));
	memcpy(online->challenge, sess->challenge, sizeof(online->challenge));
	memcpy(online->passwd, sess->passwd, sizeof(online->passwd));
	memcpy(online->username, sess->username, sizeof(online->username));
	memcpy(online->acctSessID, sess->acctSessID, sizeof(online->acctSessID));

	return sess_backup_sync(manage, pbuf);
}

static int
sessopt_offlinesync(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;
	struct instance_sync *sync;
	struct offline_sync *offline;
	struct pppoe_buf *pbuf;

	if (BACKUP_ACTIVE != backup_status(manage->backup))
		return PPPOEERR_SUCCESS;

	if (!backup_channel_state(manage->backup)) {
		pppoe_token_log(TOKEN_MANAGE, "backup channel is not connect\n");
		return PPPOEERR_ESTATE;
	}

	pbuf= pbuf_alloc(BACKUP_HEADER_LEN + 
					sizeof(struct instance_sync) + 
					sizeof(struct offline_sync));
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		return PPPOEERR_ENOMEM;
	}

	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	memset(sync, 0, sizeof(struct instance_sync));
	sync->code = INSTANCE_SESSOFFLINE_SYNC;
	sync->dev_id = manage->config->dev_id;

	offline = (struct offline_sync *)pbuf_put(pbuf, sizeof(struct offline_sync));
	memset(offline, 0, sizeof(struct offline_sync));	
	offline->sid = sess->sid;
	memcpy(offline->mac, sess->mac, sizeof(offline->mac));

	return sess_backup_sync(manage, pbuf);
}

static int
sessopt_updatesync(session_struct_t *sess, void *arg) {
	pppoe_manage_t *manage = (pppoe_manage_t *)arg;
	struct instance_sync *sync;
	struct update_sync *update;
	struct pppoe_buf *pbuf;

	if (BACKUP_ACTIVE != backup_status(manage->backup))
		return PPPOEERR_SUCCESS;

	if (!backup_channel_state(manage->backup)) {
		pppoe_token_log(TOKEN_MANAGE, "backup channel is not connect\n");
		return PPPOEERR_ESTATE;
	}

	pbuf= pbuf_alloc(BACKUP_HEADER_LEN + 
					sizeof(struct instance_sync) + 
					sizeof(struct update_sync));
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		return PPPOEERR_ENOMEM;
	}
	
	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	memset(sync, 0, sizeof(struct instance_sync));
	sync->code = INSTANCE_SESSUPDATE_SYNC;
	sync->dev_id = manage->config->dev_id;

	update = (struct update_sync *)pbuf_put(pbuf, sizeof(struct update_sync));
	memset(update, 0, sizeof(struct update_sync));	
	update->sid = sess->sid;
	update->stat = sess->stat;
	memcpy(update->mac, sess->mac, sizeof(update->mac));

	return sess_backup_sync(manage, pbuf);
}

static inline int 
process_register(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_register_msg *reg = &mess->data.m_register;
	session_struct_t *sess;
	
	pppoe_token_log(TOKEN_MANAGE, "recv register packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);

	sess = __session_get_by_sid(manage->sessions, reg->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "sid %d is not exist\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (memcmp(sess->mac, reg->mac, ETH_ALEN)) {
		pppoe_log(LOG_NOTICE, "sid %d mac is not match\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (mess->errorcode) {
		pppoe_log(LOG_WARNING, "sess %u register failed, so exit\n", sess->sid);
		session_opt_perform(sess, SESSOPT_DISTREM);	
		sess_exit(manage, sess);	
		return PPPOEERR_ESYSCALL;
	}
	
	return PPPOEERR_SUCCESS;
}

static inline int 
process_unregister(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_register_msg *reg = &mess->data.m_register;

	pppoe_token_log(TOKEN_MANAGE, "recv unregister packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);
	return PPPOEERR_SUCCESS;
}


static inline int 
process_authorize(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_authorize_msg *reg = &mess->data.m_authorize;
	session_struct_t *sess;
	int ret;

	pppoe_token_log(TOKEN_MANAGE, "recv authorize packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);

	sess = __session_get_by_sid(manage->sessions, reg->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "sid %d is not exist\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (sess->ipaddr.ip != reg->ip) {
		pppoe_log(LOG_NOTICE, "sid %d ip is not match\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (PPPOEERR_SUCCESS != (ret = mess->errorcode)) {
		pppoe_log(LOG_WARNING, "authorize sess fail(%d), sess %d will exit\n", 
								mess->errorcode, sess->sid);
		goto error;
	}

	ret = sess_online(sess);
	if (unlikely(ret)) 
		goto error;

	return PPPOEERR_SUCCESS;

error:
	_manage_sess_exit(sess);
	return ret;
}

static inline int 
process_unauthorize(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_authorize_msg *reg = &mess->data.m_authorize;
	session_struct_t *sess;
	
	pppoe_token_log(TOKEN_MANAGE, "recv unauthorize packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);

	sess = __session_get_by_sid(manage->sessions, reg->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "sid %d is not exist\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (sess->ipaddr.ip != reg->ip) {
		pppoe_log(LOG_NOTICE, "sid %d ip is not match\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	manage_sess_exit(sess);
	return PPPOEERR_SUCCESS;
}

static int
manage_process(thread_struct_t *thread) {
	struct pppoe_manage *manage = thread_get_arg(thread);
	struct pppoe_message *mess;
	int ret;

	while (1) {
		ret = netlink_recv_message(manage->sk, pbuf_init(manage->pbuf));
		if (ret) {
			if (PPPOEERR_EAGAIN == ret) 
				break;

			pppoe_log(LOG_WARNING, "recv message failed, ret %d\n", ret);
			return ret;
		}

		mess = (struct pppoe_message *)manage->pbuf->data;
		if (PPPOE_MESSAGE_REPLY != mess->type) {
			pppoe_log(LOG_WARNING, "recv message type is not reply\n");
			continue;
		}

		pppoe_token_log(TOKEN_MANAGE, "recv mange packet, code = %d\n", mess->code);
			
		switch (mess->code) {
			case PPPOE_CHANNEL_REGISTER:
				process_register(manage, mess);
				break;
				
			case PPPOE_CHANNEL_UNREGISTER:
				process_unregister(manage, mess);
				break;
				
			case PPPOE_CHANNEL_AUTHORIZE:
				process_authorize(manage, mess);
				break;
				
			case PPPOE_CHANNEL_UNAUTHORIZE:
				process_unauthorize(manage, mess);
				break;
				
			default:
				break;
		}
	}
	
	return PPPOEERR_SUCCESS;
}

int
manage_sess_kick_by_mac(pppoe_manage_t *manage, unsigned char *mac) {
	session_struct_t *sess;

	if (unlikely(!manage || !mac)) 
		return PPPOEERR_EINVAL;

	sess = session_get_by_mac(manage->sessions, mac);
	if (!sess || SESSION_ONLINE != sess->state) {
		return PPPOEERR_ENOEXIST;
	}

	session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_NAS_REQUEST);
	manage_sess_exit(sess);
	return PPPOEERR_SUCCESS;
}

int
manage_sess_kick_by_sid(pppoe_manage_t *manage, uint32 sid) {
	session_struct_t *sess;

	if (unlikely(!manage)) 
		return PPPOEERR_EINVAL;

	sess = __session_get_by_sid(manage->sessions, sid);
	if (!sess || SESSION_ONLINE != sess->state) {
		return PPPOEERR_ENOEXIST;
	}

	session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_NAS_REQUEST);
	manage_sess_exit(sess);
	return PPPOEERR_SUCCESS;
}

session_struct_t *
__manage_sess_get_by_mac(pppoe_manage_t *manage, unsigned char *mac) {
	return __session_get_by_mac(manage->sessions, mac);
}


session_struct_t *
__manage_sess_get_by_sid(pppoe_manage_t *manage, uint32 sid) {
	return __session_get_by_sid(manage->sessions, sid);
}

struct list_head *
__manage_sess_get_list(pppoe_manage_t *manage) {
	return __session_get_list(manage->sessions);
}

session_struct_t *
manage_sess_get_by_mac(pppoe_manage_t *manage, unsigned char *mac) {
	if (unlikely(!manage || !mac)) 
		return NULL;

	return session_get_by_mac(manage->sessions, mac);
}


session_struct_t *
manage_sess_get_by_sid(pppoe_manage_t *manage, uint32 sid) {
	if (unlikely(!manage)) 
		return NULL;

	return session_get_by_sid(manage->sessions, sid);
}

session_struct_t *
manage_sess_init(pppoe_manage_t *manage, uint8 *mac, uint8 *serverMac) {
	session_struct_t *sess;

	if (unlikely(!manage || !mac || !serverMac)) 
		goto error;

	sess = session_alloc(manage->sessions, 0/*auto assign*/, mac);
	if (!sess){
		pppoe_log(LOG_WARNING, "session alloc failed\n");
		goto error;
	}

	sess->pbuf = manage->pbuf;
	sess->master = manage->master;
	sess->serverIP = manage->config->ipaddr;
	memcpy(sess->serverMac, serverMac, ETH_ALEN);
	rand_bytes(&sess->seed, sess->magic, sizeof(sess->magic));
		
	if (session_timer_init(sess, SESSION_TIMEOUT_TIMER, 
					sess_timeout_func, DEFAULT_SESSCONFIG_TIMEOUT)) {
		pppoe_log(LOG_WARNING, "session timeout timer init failed\n");
		goto error1;
	}	

	if (netlink_channel_register(manage->sk, pbuf_init(manage->pbuf),
								manage->config->ifindex, sess->sid, 
								sess->mac, sess->serverMac, sess->magic)) {
		pppoe_log(LOG_WARNING, "send channel register failed\n");
		goto error2;
	}

	session_register(manage->sessions, sess);
	sess_get_wireless_info(manage, sess);
	sess->startTime = time_sysup();
	pppoe_log(LOG_INFO, "session %d %02X:%02X:%02X:%02X:%02X:%02X init\n", 
						sess->sid, 
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5]);
	return sess;

error2:
	session_timer_destroy(sess, SESSION_TIMEOUT_TIMER);
error1:
	session_free(manage->sessions, sess);
error:
	return NULL;
}

int
_manage_sess_offline(session_struct_t *sess){
	return sess_offline(sess);
}

void
_manage_sess_exit(session_struct_t *sess) {
	if (!session_opt_perform(sess, SESSOPT_LCPTERMREQ)) {
		session_timer_update(sess, SESSION_TIMEOUT_TIMER, DEFAULT_SESSTERM_TIMEOUT);
		return;
	}
	
	session_opt_perform(sess, SESSOPT_DISTREM);	

	pppoe_log(LOG_INFO, "session %d %02X:%02X:%02X:%02X:%02X:%02X exit\n", 
						sess->sid, 
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5]);	
	session_opt_perform(sess, SESSOPT_EXIT);	
}

void 
manage_sess_exit(session_struct_t *sess) {
	if (unlikely(!sess))
		return;

	switch (sess->state) {
	case SESSION_ONLINE:
		if (!sess_offline(sess)) {
			return;
		}
		break;
			
	case SESSION_OFFLINEPRE:
	case SESSION_OFFLINEPOST:
		return;	
	}
	
	_manage_sess_exit(sess);
}

int
manage_sessions_opt_register(pppoe_manage_t *manage, sessionOptType optType, 
								 void *arg, sessOptFunc func) {
	if (unlikely(!func))
		return PPPOEERR_EINVAL;
								
	return pppoe_sessions_opt_register(manage->sessions, optType, arg, func);
}

int
manage_sessions_opt_unregister(pppoe_manage_t *manage, sessionOptType optType) {	
	return pppoe_sessions_opt_unregister(manage->sessions, optType);
}

static inline int
__manage_sessions_sync_finished(pppoe_manage_t *manage) {
	struct instance_sync *sync;
	struct pppoe_buf *pbuf;

	if (!backup_channel_state(manage->backup)) {
		pppoe_token_log(TOKEN_MANAGE, "backup channel is not connect\n");
		return PPPOEERR_ESTATE;
	}

	pbuf= pbuf_alloc(BACKUP_HEADER_LEN + sizeof(struct instance_sync));
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		return PPPOEERR_ENOMEM;
	}

	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	memset(sync, 0, sizeof(struct instance_sync));
	sync->code = INSTANCE_SESSSYNC_FINISHED;
	sync->dev_id = manage->config->dev_id;

	return sess_backup_sync(manage, pbuf);
}

static inline int
__manage_sessions_clear_sync(pppoe_manage_t *manage) {
	struct instance_sync *sync;
	struct pppoe_buf *pbuf;

	if (BACKUP_ACTIVE != backup_status(manage->backup))
		return PPPOEERR_SUCCESS;

	if (!backup_channel_state(manage->backup)) {
		pppoe_token_log(TOKEN_MANAGE, "backup channel is not connect\n");
		return PPPOEERR_ESTATE;
	}

	pbuf= pbuf_alloc(BACKUP_HEADER_LEN + sizeof(struct instance_sync));
	if (unlikely(!pbuf)) {
		pppoe_log(LOG_ERR, "pbuf alloc failed\n");
		return PPPOEERR_ENOMEM;
	}

	pbuf_reserve(pbuf, BACKUP_HEADER_LEN);
	sync = (struct instance_sync *)pbuf_put(pbuf, sizeof(struct instance_sync));
	memset(sync, 0, sizeof(struct instance_sync));
	sync->code = INSTANCE_SESSCLEAR_SYNC;
	sync->dev_id = manage->config->dev_id;

	return sess_backup_sync(manage, pbuf);
}


void
manage_sessions_sync(pppoe_manage_t *manage) {
	struct list_head *head, *pos;
	session_struct_t *sess;	

	pppoe_log(LOG_INFO, "session sync start\n");

	head = __session_get_list(manage->sessions);
	list_for_each(pos, head) {
		sess = list_entry(pos, session_struct_t, next);
		if (SESSION_ONLINE == sess->state) {
			session_opt_perform(sess, SESSOPT_ONLINESYNC);			
		}
	}	

	__manage_sessions_sync_finished(manage);	/* tell standby sync finish */
	pppoe_log(LOG_INFO, "session sync finished\n");	
}

void
manage_sessions_sync_finished(pppoe_manage_t *manage) {
	manage->sync_flag = MANAGE_SYNC_FINISHED;	/* notice: only standby set sync flag */
}

uint32
manage_sync_flag(pppoe_manage_t *manage) {
	return manage->sync_flag;
}

void
manage_sessions_clear(pppoe_manage_t *manage) {
	uint32 max_sid = manage->config->max_sid;
	session_struct_t *sess;
	int i;

	/* frist tell kernel clear session channel */
	netlink_channel_clear(manage->sk, 
						pbuf_init(manage->pbuf), 
						manage->config->ifindex);

	/* session clear need sync */
	__manage_sessions_clear_sync(manage);

	for (i = 1; i <= max_sid; i++) {
		sess = __session_get_by_sid(manage->sessions, i);
		if (!sess) {
			continue;
		}

		if (SESSION_ONLINE == sess->state) {
			session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_NAS_REBOOT);
			session_opt_perform(sess, SESSOPT_ACCTSTOPV2);
			pppoe_log(LOG_INFO, "session %u %02X:%02X:%02X:%02X:%02X:%02X "
								"%u.%u.%u.%u offline\n", sess->sid,
								sess->mac[0], sess->mac[1], sess->mac[2], 
								sess->mac[3], sess->mac[4], sess->mac[5],
								HIPQUAD(sess->ipaddr.ip));
		}

		session_opt_perform(sess, SESSOPT_LCPTERMREQ);
		session_opt_perform(sess, SESSOPT_DISTREM);

		pppoe_log(LOG_INFO, "session %d %02X:%02X:%02X:%02X:%02X:%02X exit\n", 
							sess->sid, 
							sess->mac[0], sess->mac[1], sess->mac[2], 
							sess->mac[3], sess->mac[4], sess->mac[5]);	
		sess_exit(manage, sess);
	}
}

void
manage_sessions_clear_v2(pppoe_manage_t *manage) {
	uint32 max_sid = manage->config->max_sid;
	session_struct_t *sess;
	int i;

	/* first clear sync flag */
	manage->sync_flag = MANAGE_SYNC_NONE;

	/* frist tell kernel clear session channel */
	netlink_channel_clear(manage->sk, 
						pbuf_init(manage->pbuf), 
						manage->config->ifindex);
	
	for (i = 1; i <= max_sid; i++) {
		sess = __session_get_by_sid(manage->sessions, i);
		if (!sess) {
			continue;
		}
		
		pppoe_log(LOG_INFO, "standby session %u %02X:%02X:%02X:%02X:%02X:%02X "
						"%u.%u.%u.%u offline\n", sess->sid,
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5],
						HIPQUAD(sess->ipaddr.ip));
		sess_exit(manage, sess);
	}
}

static inline void
manage_sessions_wakeup(pppoe_manage_t *manage) {
	struct list_head *pos, *head;

	head = __session_get_list(manage->sessions);
	list_for_each(pos, head) {
		struct session_struct *sess 
			= list_entry(pos, struct session_struct, next);
		if (unlikely(sess->state != SESSION_ONLINE))
			continue;

		session_timer_update(sess, SESSION_ECHO_TIMER, DEFAULT_ECHOUPDATE_TIMEOUT);
		session_timer_update(sess, SESSION_UPDATE_TIMER, DEFAULT_ACCTUPDATE_TIMEOUT);
	}	
}

static inline int
standby_register(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_register_msg *reg = &mess->data.m_register;
	session_struct_t *sess;
	
	pppoe_token_log(TOKEN_MANAGE, "recv register packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);

	sess = __session_get_by_sid(manage->sessions, reg->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "sid %d is not exist\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (memcmp(sess->mac, reg->mac, ETH_ALEN)) {
		pppoe_log(LOG_NOTICE, "sid %d mac is not match\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (mess->errorcode) {
		pppoe_log(LOG_WARNING, "sess %u register failed, so exit\n", sess->sid);
		sess_exit(manage, sess);
		return PPPOEERR_ESYSCALL;
	}
	
	return PPPOEERR_SUCCESS;
}

static inline int 
standby_unregister(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_register_msg *reg = &mess->data.m_register;

	pppoe_token_log(TOKEN_MANAGE, "recv unregister packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);
	return PPPOEERR_SUCCESS;
}

static inline int 
standby_authorize(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_authorize_msg *reg = &mess->data.m_authorize;
	session_struct_t *sess;
	int ret;

	pppoe_token_log(TOKEN_MANAGE, "recv authorize packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);

	sess = __session_get_by_sid(manage->sessions, reg->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "sid %d is not exist\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (sess->ipaddr.ip != reg->ip) {
		pppoe_log(LOG_NOTICE, "sid %d ip is not match\n", reg->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (PPPOEERR_SUCCESS != (ret = mess->errorcode)) {
		pppoe_log(LOG_WARNING, "authorize sess fail(%d), sess %d will exit\n", 
								mess->errorcode, sess->sid);
		goto error;
	}

	session_timer_pause(sess, SESSION_TIMEOUT_TIMER);
	sess->state = SESSION_ONLINE;	/* set session online state */
	return PPPOEERR_SUCCESS;

error:
	sess_exit(manage, sess);
	return ret;
}

static inline int 
standby_unauthorize(struct pppoe_manage *manage, struct pppoe_message *mess) {
	struct pppoe_authorize_msg *reg = &mess->data.m_authorize;
	pppoe_token_log(TOKEN_MANAGE, "recv unauthorize packet, sid is %d, errocde = %d\n",
									reg->sid, mess->errorcode);
	return PPPOEERR_SUCCESS;
}

static int
standby_process(thread_struct_t *thread) {
	struct pppoe_manage *manage = thread_get_arg(thread);
	struct pppoe_message *mess;
	int ret;
	
	while (1) {
		ret = netlink_recv_message(manage->sk, pbuf_init(manage->pbuf));
		if (ret) {
			if (PPPOEERR_EAGAIN == ret) 
				break;

			pppoe_log(LOG_WARNING, "recv message failed, ret %d\n", ret);
			return ret;
		}

		mess = (struct pppoe_message *)manage->pbuf->data;
		if (PPPOE_MESSAGE_REPLY != mess->type) {
			pppoe_log(LOG_WARNING, "recv message type is not reply\n");
			continue;
		}

		pppoe_token_log(TOKEN_MANAGE, "recv mange packet, code = %d\n", mess->code);
			
		switch (mess->code) {
			case PPPOE_CHANNEL_REGISTER:
				standby_register(manage, mess);
				break;
				
			case PPPOE_CHANNEL_UNREGISTER:
				standby_unregister(manage, mess);
				break;
				
			case PPPOE_CHANNEL_AUTHORIZE:
				standby_authorize(manage, mess);
				break;
				
			case PPPOE_CHANNEL_UNAUTHORIZE:
				standby_unauthorize(manage, mess);
				break;
				
			default:
				break;
		}
	}
	
	return PPPOEERR_SUCCESS;	
}


int
manage_sess_online_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf) {
	struct online_sync *sync;
	session_struct_t *sess;
	long upTime;
	int ret;

	if (pbuf_may_pull(pbuf, sizeof(struct online_sync))) {
		pppoe_log(LOG_ERR, "input pbuf length error\n");
		ret = PPPOEERR_ELENGTH;
		goto error;
	}

	sync = (struct online_sync *)pbuf->data;
	sess = session_alloc(manage->sessions, sync->sid/*auto assign*/, sync->mac);
	if (!sess){
		pppoe_log(LOG_WARNING, "session alloc fail\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	sess->pbuf = manage->pbuf;
	sess->master = manage->master;
	sess->serverIP = sync->serverIP;
	sess->ipaddr = sync->ipaddr;
	sess->auth_type = sync->auth_type;
	sess->chap_mdtype = sync->chap_mdtype;
	sess->stat = sess->bk_stat = sync->stat;

	upTime = time_sysup();
	sess->startTime = upTime - sync->startTime;
	sess->onlineTime = upTime - sync->onlineTime;
	
	memcpy(sess->mac, sync->mac, sizeof(sess->mac));
	memcpy(sess->serverMac, sync->serverMac, sizeof(sess->serverMac));
	memcpy(sess->sname, sync->sname, sizeof(sess->sname));
	memcpy(sess->magic, sync->magic, sizeof(sess->magic));
	memcpy(sess->peermagic, sync->peermagic, sizeof(sess->peermagic));
	memcpy(sess->challenge, sync->challenge, sizeof(sess->challenge));
	memcpy(sess->passwd, sync->passwd, sizeof(sess->passwd));
	memcpy(sess->username, sync->username, sizeof(sess->username));
	memcpy(sess->acctSessID, sync->acctSessID, sizeof(sess->acctSessID));

	
	ret = session_timer_init(sess, SESSION_TIMEOUT_TIMER, 
					sess_timeout_func, DEFAULT_SESSCONFIG_TIMEOUT);
	if (ret) {
		pppoe_log(LOG_WARNING, "session timeout timer init failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	ret = session_timer_init(sess, SESSION_ECHO_TIMER, 
					sess_echo_func, DEFAULT_ECHOUPDATE_TIMEOUT);
	if (ret) {
		pppoe_log(LOG_WARNING, "session echo timer init failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error2;
	}

	ret = session_timer_init(sess, SESSION_UPDATE_TIMER, 
					sess_acct_update_func, DEFAULT_ACCTUPDATE_TIMEOUT);
	if (ret) {
		pppoe_log(LOG_WARNING, "session acct update timer init failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error3;
	}

	ret = netlink_channel_register(manage->sk, pbuf_init(manage->pbuf),
									manage->config->ifindex, sess->sid, 
									sess->mac, sess->serverMac, sess->magic);
	if (ret) {
		pppoe_log(LOG_WARNING, "send channel register failed\n");
		goto error4;
	}

	ret = netlink_channel_authorize(manage->sk, pbuf_init(manage->pbuf),
									manage->config->ifindex, 
									sess->sid, sess->ipaddr.ip);
	if (ret) {
		pppoe_log(LOG_WARNING, "send channel register failed\n");
		goto error5;
	}

	session_timer_pause(sess, SESSION_ECHO_TIMER);
	session_timer_pause(sess, SESSION_UPDATE_TIMER);
	session_register(manage->sessions, sess);
	pppoe_log(LOG_INFO, "standby session %u %02X:%02X:%02X:%02X:%02X:%02X "
						"%u.%u.%u.%u online\n", sess->sid,
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5], 
						HIPQUAD(sess->ipaddr.ip));
	return PPPOEERR_SUCCESS;

error5:
	netlink_channel_unregister(manage->sk, pbuf_init(manage->pbuf), 
						manage->config->ifindex, sess->sid, sess->mac);
error4:
	session_timer_destroy(sess, SESSION_UPDATE_TIMER);
error3:
	session_timer_destroy(sess, SESSION_ECHO_TIMER);
error2:
	session_timer_destroy(sess, SESSION_TIMEOUT_TIMER);
error1:
	PPPOE_FREE(sess);
error:
	return ret;
}

int
manage_sess_offline_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf) {
	struct offline_sync *sync;
	session_struct_t *sess;

	if (pbuf_may_pull(pbuf, sizeof(struct offline_sync))) {
		pppoe_log(LOG_ERR, "input pbuf length error\n");
		return PPPOEERR_ELENGTH;
	}

	sync = (struct offline_sync *)pbuf->data;
	sess = __session_get_by_sid(manage->sessions, sync->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "session %u is not exist\n", sync->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (memcmp(sess->mac, sync->mac, ETH_ALEN)) {
		pppoe_log(LOG_NOTICE, "session %u mac is not match, "
							"but also offline it\n", sync->sid);
	}

	pppoe_log(LOG_INFO, "standby session %u %02X:%02X:%02X:%02X:%02X:%02X "
						"%u.%u.%u.%u offline\n", sess->sid,
						sess->mac[0], sess->mac[1], sess->mac[2], 
						sess->mac[3], sess->mac[4], sess->mac[5], 
						HIPQUAD(sess->ipaddr.ip));
	return session_opt_perform(sess, SESSOPT_EXIT);
}

int
manage_sess_update_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf) {
	struct update_sync *sync;
	session_struct_t *sess;

	if (pbuf_may_pull(pbuf, sizeof(struct update_sync))) {
		pppoe_log(LOG_ERR, "input pbuf length error\n");
		return PPPOEERR_ELENGTH;
	}

	sync = (struct update_sync *)pbuf->data;
	sess = __session_get_by_sid(manage->sessions, sync->sid);
	if (!sess) {
		pppoe_log(LOG_NOTICE, "session %u is not exist\n", sync->sid);
		return PPPOEERR_ENOEXIST;
	}

	if (memcmp(sess->mac, sync->mac, ETH_ALEN)) {
		pppoe_log(LOG_NOTICE, "session %u mac is not match, "
							"need offline it\n", sync->sid);
		return session_opt_perform(sess, SESSOPT_EXIT);
	}

	sess->stat = sess->bk_stat = sync->stat;
	return PPPOEERR_SUCCESS;
}

int
pppoe_manage_start(pppoe_manage_t *manage) {
	int ret;

	if (unlikely(!manage))
		return PPPOEERR_EINVAL;

	if ((manage->sk = netlink_init()) <= 0) {
		pppoe_log(LOG_ERR, "netlink socket init fail\n");
		ret = PPPOEERR_ESOCKET;
		goto error;
	}

	switch (backup_status(manage->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		manage->thread = thread_add_read(manage->master, 
										manage_process, 
										manage, manage->sk);
		if (!manage->thread) {
			pppoe_log(LOG_ERR, "manage thread add read fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error1;
		}
		
		manage->update = thread_add_timer(manage->master, 
								sess_stat_update_func, manage, 
								DEFAULT_STATUPDATE_TIMEOUT, THREAD_EXTIMER1);
		if (!manage->update) {
			pppoe_log(LOG_ERR, "session update timer add fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error2;
		}
		break;

	case BACKUP_STANDBY:
		manage->thread = thread_add_read(manage->master, 
										standby_process, 
										manage, manage->sk);
		if (!manage->thread) {
			pppoe_log(LOG_ERR, "manage thread add read fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error1;
		}
		break;
	}
	
	pppoe_log(LOG_INFO, "manage start sucess\n");
	return PPPOEERR_SUCCESS;

error2:
	THREAD_CANCEL(manage->thread);
error1:
	PPPOE_CLOSE(manage->sk);
error:
	return ret;
}

int
pppoe_manage_stop(struct pppoe_manage *manage) {
	if (unlikely(!manage))
		return PPPOEERR_EINVAL;

	switch (backup_status(manage->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		manage_sessions_clear(manage);
		THREAD_CANCEL(manage->update);
		THREAD_CANCEL(manage->thread);
		break;

	case BACKUP_STANDBY:
		manage_sessions_clear_v2(manage);
		THREAD_CANCEL(manage->thread);
		break;	
	}

	PPPOE_CLOSE(manage->sk);	
	pppoe_log(LOG_INFO, "manage stop sucess\n");
	return PPPOEERR_SUCCESS;
}

int
pppoe_manage_restart(pppoe_manage_t *manage) {
	int ret;

	if (unlikely(!manage))
		return PPPOEERR_EINVAL;

	switch (backup_prevstatus(manage->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		manage_sessions_clear_v2(manage);	/* need clear session */
		THREAD_CANCEL(manage->update);
		THREAD_CANCEL(manage->thread);
		break;

	case BACKUP_STANDBY:
		if (backup_status(manage->backup) == BACKUP_ACTIVE) {
			/* standby switch to active can not clear session */
			THREAD_CANCEL(manage->thread);
		} else {
			/* standby disable need clear session */
			manage_sessions_clear_v2(manage);
			THREAD_CANCEL(manage->update);
			THREAD_CANCEL(manage->thread);
		}
		break;
	}

	switch (backup_status(manage->backup)) {
	case BACKUP_NONE:
	case BACKUP_ACTIVE:
		manage->thread = thread_add_read(manage->master, 
										manage_process, 
										manage, manage->sk);
		if (!manage->thread) {
			pppoe_log(LOG_ERR, "manage thread add read fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error;
		}
		
		manage->update = thread_add_timer(manage->master, 
								sess_stat_update_func, manage, 
								DEFAULT_STATUPDATE_TIMEOUT, THREAD_EXTIMER1);
		if (!manage->update) {
			pppoe_log(LOG_ERR, "session update timer add fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error1;
		}

		manage_sessions_wakeup(manage);
		break;

	case BACKUP_STANDBY:
		manage->thread = thread_add_read(manage->master, 
										standby_process, 
										manage, manage->sk);
		if (!manage->thread) {
			pppoe_log(LOG_ERR, "manage thread add read fail\n");
			ret = PPPOEERR_ENOMEM;
			goto error;
		}
		break;
	}
	
	pppoe_log(LOG_INFO, "manage restart sucess\n");
	return PPPOEERR_SUCCESS;

error1:
	THREAD_CANCEL(manage->thread);
error:	
	return ret;
}


static inline void
manage_setup(pppoe_manage_t *manage) {
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_EXIT, manage, sessopt_exit);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_IPAPPLY, manage, sessopt_ipapply);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_IPRECOVER, manage, sessopt_iprecover);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_IPREGISTER, manage, sessopt_ipregister);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_IPUNREGISTER, manage, sessopt_ipunregister);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_ONLINESYNC, manage, sessopt_onlinesync);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_OFFLINESYNC, manage, sessopt_offlinesync);
	pppoe_sessions_opt_register(manage->sessions, SESSOPT_UPDATESYNC, manage, sessopt_updatesync);	
}

static inline void
manage_unsetup(pppoe_manage_t *manage) {
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_EXIT);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_IPAPPLY);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_IPRECOVER);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_IPREGISTER);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_IPUNREGISTER);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_ACCTUPDATE);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_ONLINESYNC);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_OFFLINESYNC);
	pppoe_sessions_opt_unregister(manage->sessions, SESSOPT_UPDATESYNC);	
};

pppoe_manage_t * 
pppoe_manage_init(thread_master_t *master, 
					backup_struct_t *backup,
					tbus_connection_t *connection,
					struct manage_config *config) {
	pppoe_manage_t *manage;

	if (unlikely(!master || !backup || !connection || !config))
		goto error;

	manage = (pppoe_manage_t *)malloc(sizeof(pppoe_manage_t));
	if (unlikely(!manage)) {
		pppoe_log(LOG_ERR, "pppoe_manage_init: malloc pppoe manage fail\n");
		goto error;
	}
	
	memset(manage, 0, sizeof(*manage));
	manage->config = config;
	manage->master = master;
	manage->backup = backup;
	manage->connection = connection;
	manage->sk = -1;

	manage->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!manage->pbuf)) {
		pppoe_log(LOG_ERR, "pppoe_manage_init: malloc pppoe manage fail\n");
		goto error1;
	}

	manage->sessions = pppoe_sessions_init(config->max_sid);
	if (unlikely(!manage->sessions)) {
		pppoe_log(LOG_ERR, "pppoe_manage_init: sessions init fail\n");
		goto error2;
	}

	manage->ippool = ippool_create(config->minIP, config->maxIP, config->dns1, config->dns2);	
	if (unlikely(!manage->sessions)) {
		pppoe_log(LOG_ERR, "pppoe_manage_init: ippool init fail\n");
		goto error3;
	}

	INIT_LIST_HEAD(&manage->sess_dead);
	manage_setup(manage);	
	pppoe_log(LOG_INFO, "manage init sucess\n");
	return manage;

error3:
	pppoe_sessions_destroy(&manage->sessions);
error2:
	PBUF_FREE(manage->pbuf);
error1:
	PPPOE_FREE(manage);
error:
	return NULL;
}

void
pppoe_manage_destroy(pppoe_manage_t **manage) {
	if (unlikely(!manage || !(*manage)))
		return ;

	manage_unsetup(*manage);
	pppoe_sessions_destroy(&(*manage)->sessions);
	ippool_destroy(&(*manage)->ippool);
	PBUF_FREE((*manage)->pbuf);
	PPPOE_FREE(*manage);
	pppoe_log(LOG_INFO, "manage destroy sucess\n");	
}

int
pppoe_manage_config_sessions_max_sid(manage_config_t *config, uint32 max_sid) {
	if (unlikely(max_sid > DEFAULT_MAX_SESSIONID))
		return PPPOEERR_EINVAL;

	config->max_sid = max_sid;
	return PPPOEERR_SUCCESS;
}

int
pppoe_manage_config_sessions_ipaddr(manage_config_t *config, uint32 minIP, uint32 maxIP) {
	if (unlikely(minIP > maxIP))
		return PPPOEERR_EINVAL;

	config->minIP = minIP;
	config->maxIP = maxIP;
	return PPPOEERR_SUCCESS;	
}

int
pppoe_manage_config_sessions_dns(manage_config_t *config, uint32 dns1, uint32 dns2) {
	if (unlikely(!dns1 && dns2))
		return PPPOEERR_EINVAL;

	config->dns1 = dns1;
	config->dns2 = dns2;
	return PPPOEERR_SUCCESS;	
}

int
pppoe_manage_show_online_user(pppoe_manage_t *manage, 
							struct pppoeUserInfo **userarray, uint32 *userNum) {	
	if (unlikely(!manage))
		return PPPOEERR_EINVAL;

	return pppoe_sessions_show_online(manage->sessions, userarray, userNum);
}


int 
pppoe_manage_show_running_config(manage_config_t *config, char *cmd, uint32 len) {
	char *cursor = cmd;
	
	if (config->minIP) {
		cursor += snprintf(cursor, len - (cursor - cmd), 
						" session ip address range %u.%u.%u.%u %u.%u.%u.%u\n", 
						HIPQUAD(config->minIP), HIPQUAD(config->maxIP));
	}

	if (config->dns1) {
		if (config->dns2) {
			cursor += snprintf(cursor, len - (cursor - cmd), 
							" session dns %u.%u.%u.%u %u.%u.%u.%u\n", 
							HIPQUAD(config->dns1), HIPQUAD(config->dns2));
		} else {
			cursor += snprintf(cursor, len - (cursor - cmd), 
							" session dns %u.%u.%u.%u\n", HIPQUAD(config->dns1));
		}
	}

	return cursor - cmd;
}

void
pppoe_manage_config_setup(manage_config_t *config, 
						uint32 slot_id, uint32 local_id, uint32 instance_id,
						int ifindex, uint32 dev_id, uint32 ipaddr, char *ifname) {
	config->slot_id = slot_id;
	config->local_id = local_id;
	config->instance_id = instance_id;
	config->ifindex = ifindex;
	config->dev_id = dev_id;
	config->ipaddr = ipaddr;
	strncpy(config->ifname, ifname, sizeof(config->ifname) - 1);
}

int
pppoe_manage_config_init(manage_config_t **config){
	int ret;

	if (unlikely(!config))
		return PPPOEERR_EINVAL;

	*config = (manage_config_t *)malloc(sizeof(manage_config_t));
	if (unlikely(!(*config))) {
		pppoe_log(LOG_ERR, "pppoe manage config alloc fail\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	memset(*config, 0, sizeof(manage_config_t));
	(*config)->max_sid = DEFAULT_MAX_SESSIONID;
	return PPPOEERR_SUCCESS;
	
error:
	return ret;
}

void
pppoe_manage_config_exit(manage_config_t **config){
	if (unlikely(!config))
		return;

	PPPOE_FREE(*config);
}


