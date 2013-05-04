#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "mem_cache.h"
#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_util.h"
#include "pppoe_thread.h"
#include "pppoe_ippool.h"

#include "pppoe_session.h"


#define SESSION_HASHBITS	16
#define SESSION_HASHSIZE	(1 << SESSION_HASHBITS)

#define SESSION_CACHE_BLK_ITEMNUM	512
#define SESSION_CACHE_MAX_BLKNUM	0		/* blk num not limit */
#define SESSION_CACHE_EMPTY_BLKNUM	4
#define SESSION_CACHE_NAME			"pppoe session cache"


struct session_opt {
	void *arg;
	sessOptFunc func;
};

struct pppoe_sessions {
	uint32 max_sid, count;
	
	mem_cache_t *cache;	
	struct hlist_head *sess_mac;
	session_struct_t **sess_sid;
	struct list_head sess_list;	

	pthread_rwlock_t	sid_lock;
	pthread_rwlock_t	mac_lock;
	pthread_rwlock_t	list_lock;
	
	session_opt_t opt[SESSOPT_NUMS];
	struct session_options options;
};

static inline struct hlist_head *
session_mac_hash(struct hlist_head *sess_mac, unsigned char *mac) {
	return &sess_mac[(mac[4] * 256 + mac[5]) & (SESSION_HASHSIZE - 1)];
}

session_struct_t *
__session_get_by_mac(pppoe_sessions_t *sessions, unsigned char *mac) {
	struct hlist_node *node;

	hlist_for_each(node, session_mac_hash(sessions->sess_mac, mac)) {
		session_struct_t *sess
			= hlist_entry(node, session_struct_t, hash_mac);
		if (!memcmp(sess->mac, mac, ETH_ALEN))  
			return sess;
	}
	return NULL;	
}

session_struct_t *
__session_get_by_sid(pppoe_sessions_t *sessions, unsigned int sid) {
	return sid > sessions->max_sid ? NULL : sessions->sess_sid[sid];
}

struct list_head *
__session_get_list(pppoe_sessions_t *sessions) {
	return &sessions->sess_list;
}

session_struct_t *
session_get_by_mac(pppoe_sessions_t *sessions, unsigned char *mac) {
	session_struct_t *sess;

	pthread_rwlock_rdlock(&sessions->mac_lock);	
	sess = __session_get_by_mac(sessions, mac);
	if (sess) 
		session_hold(sess);
	pthread_rwlock_unlock(&sessions->mac_lock);
	
	return sess;
}

session_struct_t *
session_get_by_sid(pppoe_sessions_t *sessions, unsigned int sid) {
	session_struct_t *sess;

	pthread_rwlock_rdlock(&sessions->sid_lock);	
	sess = __session_get_by_sid(sessions, sid);
	if (sess) 
		session_hold(sess);
	pthread_rwlock_unlock(&sessions->sid_lock);
	return sess;
}

void
session_termcause_setup(session_struct_t *sess, uint32 termcause) {
	if (!sess->terminate_cause) {
		sess->terminate_cause = termcause;
	}
}

int
session_timer_init(session_struct_t *sess, sessionTimerType type,
						ThreadRunFunc func, long timer) {
	thread_struct_t **thread;
	uint32 extern_id = THREAD_EXTIME_NONE;

	switch (type) {
		case SESSION_TIMEOUT_TIMER:
			thread = &sess->timeout;
			break;
			
		case SESSION_ECHO_TIMER:
			thread = &sess->echo;
			extern_id = THREAD_EXTIMER2;
			break;
			
		case SESSION_RETRANS_TIMER:
			thread = &sess->retrans;
			break;
			
		case SESSION_UPDATE_TIMER:
			thread = &sess->update;
			extern_id = THREAD_EXTIMER3;
			break;

		default:
			return PPPOEERR_EINVAL;
	}

	if (unlikely(*thread))
		return PPPOEERR_EEXIST;

	*thread = thread_add_timer(sess->master, func, sess, timer, extern_id);
	if (unlikely(!(*thread))){
		pppoe_log(LOG_WARNING, "session %u %u timer init fail\n", sess->sid, type);
		return PPPOEERR_ENOMEM;
	}

	return PPPOEERR_SUCCESS;
}

int
session_timer_update(session_struct_t *sess, sessionTimerType type, long timer) {
	thread_struct_t **thread;
	uint32 extern_id = THREAD_EXTIME_NONE;

	switch (type) {
		case SESSION_TIMEOUT_TIMER:
			thread = &sess->timeout;
			break;
			
		case SESSION_ECHO_TIMER:
			thread = &sess->echo;
			extern_id = THREAD_EXTIMER2;
			break;
			
		case SESSION_RETRANS_TIMER:
			thread = &sess->retrans;
			break;
			
		case SESSION_UPDATE_TIMER:
			thread = &sess->update;
			extern_id = THREAD_EXTIMER3;
			break;

		default:
			return PPPOEERR_EINVAL;
	}

	if (unlikely(!(*thread)))
		return PPPOEERR_ENOEXIST;
	
	return thread_update_timer(*thread, timer, extern_id);
}

int
session_timer_pause(session_struct_t *sess, sessionTimerType type) {
	thread_struct_t **thread;

	switch (type) {
		case SESSION_TIMEOUT_TIMER:
			thread = &sess->timeout;
			break;
			
		case SESSION_ECHO_TIMER:
			thread = &sess->echo;
			break;
			
		case SESSION_RETRANS_TIMER:
			thread = &sess->retrans;
			break;
			
		case SESSION_UPDATE_TIMER:
			thread = &sess->update;
			break;

		default:
			return PPPOEERR_EINVAL;
	}

	if (unlikely(!(*thread)))
		return PPPOEERR_ENOEXIST;

	return thread_pause_timer(*thread);
}

void
session_timer_destroy(session_struct_t *sess, sessionTimerType type) {
	thread_struct_t **thread;

	switch (type) {
		case SESSION_TIMEOUT_TIMER:
			thread = &sess->timeout;
			break;
			
		case SESSION_ECHO_TIMER:
			thread = &sess->echo;
			break;
			
		case SESSION_RETRANS_TIMER:
			thread = &sess->retrans;
			break;
			
		case SESSION_UPDATE_TIMER:
			thread = &sess->update;
			break;

		default:
			return;
	}

	THREAD_CANCEL(*thread);
}

void
session_timer_clear(session_struct_t *sess) {
	if (!sess)
		return;

	THREAD_CANCEL(sess->timeout);
	THREAD_CANCEL(sess->echo);
	THREAD_CANCEL(sess->retrans);
	THREAD_CANCEL(sess->update);
}

int
session_opt_perform(session_struct_t *sess, sessionOptType optType) {							
	if (!sess->opt[optType].func)
		return PPPOEERR_ENOEXIST;

	return sess->opt[optType].func(sess, sess->opt[optType].arg);
}

int
session_opt_register(session_struct_t *sess, sessionOptType optType, 
						void *arg, sessOptFunc func) {
	if (unlikely(!func))
		return PPPOEERR_EINVAL;
							
	if (unlikely(sess->opt[optType].func))
		return PPPOEERR_EEXIST;

	sess->opt[optType].arg = arg;
	sess->opt[optType].func = func;
	return PPPOEERR_SUCCESS;
}

int
session_opt_unregister(session_struct_t *sess, sessionOptType optType) {							
	if (unlikely(!sess->opt[optType].func))
		return PPPOEERR_ENOEXIST;

	memset(&sess->opt[optType], 0, sizeof(session_opt_t));
	return PPPOEERR_SUCCESS;
}

void
session_register(pppoe_sessions_t *sessions, session_struct_t *sess) {
	if (unlikely(!sessions || !sess))
		return;

	/* add sessions mac hash table */	
	pthread_rwlock_wrlock(&sessions->mac_lock);
	hlist_add_head(&sess->hash_mac, session_mac_hash(sessions->sess_mac, sess->mac));
	pthread_rwlock_unlock(&sessions->mac_lock);

	/* add sessions sid hash table */	
	pthread_rwlock_wrlock(&sessions->sid_lock);
	sessions->sess_sid[sess->sid] = sess;
	pthread_rwlock_unlock(&sessions->sid_lock);

	pthread_rwlock_wrlock(&sessions->list_lock);
	list_add_tail(&sess->next, &sessions->sess_list);
	pthread_rwlock_unlock(&sessions->list_lock);
	
	sess->state = SESSION_INIT;
	sessions->count++;
}

void
session_unregister(pppoe_sessions_t *sessions, session_struct_t *sess) {
	if (unlikely(!sessions || !sess))
		return;

	/* remove sessions mac hash table */	
	pthread_rwlock_wrlock(&sessions->mac_lock);
	hlist_del(&sess->hash_mac);
	pthread_rwlock_unlock(&sessions->mac_lock);

	/* remove sessions sid hash table */	
	pthread_rwlock_wrlock(&sessions->sid_lock);
	sessions->sess_sid[sess->sid] = NULL;
	pthread_rwlock_unlock(&sessions->sid_lock);

	pthread_rwlock_wrlock(&sessions->list_lock);
	list_del(&sess->next);
	pthread_rwlock_unlock(&sessions->list_lock);

	sess->state = SESSION_DEAD;
	sessions->count--;
}

static inline uint32
session_alloc_id(pppoe_sessions_t *sessions) {
	uint32 i;
	for (i = 1; i <= sessions->max_sid; i++) {
		if (NULL == sessions->sess_sid[i])
			return i;
	}

	return 0;	/*0: less session id*/
}

session_struct_t *
session_alloc(pppoe_sessions_t *sessions, uint32 sid, unsigned char *mac) {
	session_struct_t *sess;

	if (sid && __session_get_by_sid(sessions, sid)) {
		pppoe_log(LOG_WARNING, "sid %u session is exist\n", sid);
		return NULL;
	}

	
	if (__session_get_by_mac(sessions, mac)) {
		pppoe_log(LOG_WARNING, "mac %02X:%02X:%02X:%02X:%02X:%02X session is exist\n", 
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return NULL;
	}

	/* get free session */
	sess = mem_cache_alloc(sessions->cache);
	if (unlikely(!sess)) {
		pppoe_log(LOG_WARNING, "mem alloc failed\n");
		return NULL;
	}
	
	memset(sess, 0, sizeof(session_struct_t));
	sess->sid = sid ? : session_alloc_id(sessions);
	if (!sess->sid) {
		pppoe_log(LOG_WARNING, "less free session id\n");
		return NULL;
	}

	memcpy(sess->mac, mac, ETH_ALEN);
	rand_seed_init((uint8 *)&sess->seed, sizeof(sess->seed));
	sess->options = &sessions->options;
	sess->opt = (session_opt_t *)((unsigned char *)sess + sizeof(session_struct_t));
	memcpy(sess->opt, sessions->opt, sizeof(sessions->opt));

	sess->state = SESSION_NONE;
	return sess;
}

void
session_free(pppoe_sessions_t *sessions, session_struct_t *sess) {
	mem_cache_free(sessions->cache, sess);
}

int
pppoe_sessions_opt_register(pppoe_sessions_t *sessions, 
							sessionOptType optType, void *arg, sessOptFunc func) {							
	if (unlikely(sessions->opt[optType].func))
		return PPPOEERR_EEXIST;

	sessions->opt[optType].arg = arg;
	sessions->opt[optType].func = func;
	return PPPOEERR_SUCCESS;
}

int
pppoe_sessions_opt_unregister(pppoe_sessions_t *sessions, sessionOptType optType) {
	if (unlikely(!sessions->opt[optType].func))
		return PPPOEERR_ENOEXIST;

	memset(&sessions->opt[optType], 0, sizeof(session_opt_t));
	return PPPOEERR_SUCCESS;
}

int
pppoe_sessions_show_online(pppoe_sessions_t *sessions, struct pppoeUserInfo **userarray, uint32 *userNum) {
	struct pppoeUserInfo *array;
	struct list_head *pos;
	session_struct_t *sess; 
	long timeNow = time_sysup();
	uint32 num = 0;
	
	if (unlikely(!sessions || !userarray || !userNum))
		return PPPOEERR_EINVAL;

	*userarray = NULL;
	*userNum = 0;

	if (!sessions->count)
		return PPPOEERR_SUCCESS;

	array = (struct pppoeUserInfo *)calloc(sessions->count, sizeof(struct pppoeUserInfo));
	if (unlikely(!array)) {
		pppoe_log(LOG_WARNING, "alloc user array failed\n");
		return PPPOEERR_ENOMEM;
	}

	pthread_rwlock_rdlock(&sessions->list_lock);
	list_for_each(pos, &sessions->sess_list) {
		sess = list_entry(pos, session_struct_t, next);
		if (SESSION_ONLINE != sess->state)
			continue;

		array[num].sid = sess->sid;
		array[num].ip = sess->ipaddr.ip;
		array[num].sessTime = timeNow - sess->onlineTime;
		memcpy(array[num].mac, sess->mac, ETH_ALEN);
		memcpy(array[num].username, sess->username, USERNAMESIZE);

		num++;
	}
	pthread_rwlock_unlock(&sessions->list_lock);

	*userarray = array;
	*userNum = num;
	return PPPOEERR_SUCCESS;
}


static inline void
sessions_setup(pppoe_sessions_t *sessions) {
	sessions->options.neg_mru = 1;
	sessions->options.mru = PPPOE_MTU;

	/* may be need get config */
	sessions->options.neg_chap = 1;
	sessions->options.chap_mdtype = MDTYPE_MD5;

	sessions->options.neg_magicnumber = 1;

	sessions->options.neg_ipaddr = 1;
	sessions->options.neg_ipmsdns = 1;
}

pppoe_sessions_t*
pppoe_sessions_init(uint32 max_sid) {	
	pppoe_sessions_t *sessions;

	sessions = (pppoe_sessions_t *)malloc(sizeof(pppoe_sessions_t));
	if(unlikely(!sessions)) {
		pppoe_log(LOG_ERR, "pppoe alloc sessions fail\n");
		goto error;
	}	

	memset(sessions, 0, sizeof(pppoe_sessions_t));
	
	if (max_sid) {
		sessions->sess_sid = (session_struct_t **)calloc(max_sid + 1, sizeof(session_struct_t *));
		if (!sessions->sess_sid) {
			pppoe_log(LOG_ERR, "pppoe alloc sessions sid hash fail\n");
			goto error1;
		}

		sessions->sess_mac = (struct hlist_head  *)calloc(SESSION_HASHSIZE, sizeof(struct hlist_head));
		if (!sessions->sess_mac) {
			pppoe_log(LOG_ERR, "pppoe alloc sessions mac hash fail\n");
			goto error2;
		}

		sessions->cache = mem_cache_create(SESSION_CACHE_NAME,
										SESSION_CACHE_MAX_BLKNUM, 
										SESSION_CACHE_EMPTY_BLKNUM, 
										sizeof(session_struct_t) + 
										sizeof(session_opt_t) * SESSOPT_NUMS, 
										SESSION_CACHE_BLK_ITEMNUM);
		if (!sessions->cache) {
			pppoe_log(LOG_ERR, "pppoe create session cache fail\n");
			goto error3;
		}
	}

	sessions->max_sid = max_sid;
	INIT_LIST_HEAD(&sessions->sess_list);
	pthread_rwlock_init(&sessions->mac_lock, NULL);
	pthread_rwlock_init(&sessions->sid_lock, NULL);
	pthread_rwlock_init(&sessions->list_lock, NULL);

	sessions_setup(sessions);
	pppoe_log(LOG_INFO, "sessions create success\n");
	return sessions;

error3:
	PPPOE_FREE(sessions->sess_mac);
error2:	
	PPPOE_FREE(sessions->sess_sid);
error1:
	PPPOE_FREE(sessions);
error:
	return NULL;
};

void
pppoe_sessions_destroy(pppoe_sessions_t **sessions) {
	if (unlikely(!sessions || !(*sessions)))
		return ;

	pthread_rwlock_destroy(&(*sessions)->sid_lock);
	pthread_rwlock_destroy(&(*sessions)->mac_lock);
	pthread_rwlock_destroy(&(*sessions)->list_lock);	
	
	PPPOE_FREE((*sessions)->sess_sid);	
	PPPOE_FREE((*sessions)->sess_mac);	
	INIT_LIST_HEAD(&(*sessions)->sess_list);	
	mem_cache_destroy(&(*sessions)->cache);
	PPPOE_FREE(*sessions);
	pppoe_log(LOG_INFO, "sessions destroy success\n");
}

