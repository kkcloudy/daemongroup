
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"

#include "pppoe_log.h"
#include "pppoe_buf.h"
#include "pppoe_list.h"
#include "pppoe_util.h"
#include "mem_cache.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"

#include "pppoe_backup.h"

#define PROTOCOL_HASHBITS		8
#define PROTOCOL_HASHSIZE		(1 << PROTOCOL_HASHBITS)

#define BACKUP_CACHE_BLK_ITEMNUM	1024
#define BACKUP_CACHE_MAX_BLKNUM		0		/* blk num not limit */
#define BACKUP_CACHE_EMPTY_BLKNUM	4
#define BACKUP_CACHE_NAME			"backup cache"

#define BACKUP_FUNCNAME_LEN			64


struct backup_packet {
	uint16	proto;
	uint16	length;
};

struct backup_proto {
	uint16 proto;
	void *proto_data;
	backupProtoFunc process;
	struct hlist_node next;
	char funcname[BACKUP_FUNCNAME_LEN];
};

struct backup_task {
	struct pppoe_buf *pbuf;			
	struct backup_task *next;
};

struct task_queue {
	backup_task_t *head;
	backup_task_t *tail;
};

struct backup_struct { 
	int sk, lsk;
	uint32 status;
	uint32 prev_status;
	uint32 c_state;		/* channel state*/
	
	uint32 lostTimes;
	uint32 maxTimes;

	uint32 s_ip;
	uint16 s_port;

	uint32 d_ip;
	uint32 d_port;

	mem_cache_t	*cache;

	struct pppoe_buf *recv;	/* this need alloc when backup init */
	struct pppoe_buf *send;

	thread_master_t *master;
	thread_struct_t	*thread;	
	thread_struct_t	*listen;
	thread_struct_t *timer;	
	thread_struct_t *post;	/* backup post*/	

	notifier_chain_t *chain; /* backup channel notifier chain */
	
	struct task_queue tasks;
	struct hlist_head protoHash[PROTOCOL_HASHSIZE];
};

static char *backup_statusnames[] = {
	"Backup None", 
	"Backup Active",
	"Backup Standby",
	"Backup Disable",
};

static inline struct hlist_head *
proto_hash(backup_struct_t *backup, uint16 proto) {
	return &backup->protoHash[proto & (PROTOCOL_HASHSIZE - 1)];
}

static inline struct backup_proto *
proto_get(backup_struct_t *backup, uint16 proto) {
	struct hlist_node *node;
	hlist_for_each(node, proto_hash(backup, proto)) {
		struct backup_proto *pos
			= hlist_entry(node, struct backup_proto, next);
		if (pos->proto == proto)  
			return pos;
	}
	return NULL;
}
static inline int
proto_process(backup_struct_t *backup, 
					uint16 proto, struct pppoe_buf *pbuf) {
	struct backup_proto *pos;
	int ret;

	pppoe_token_log(TOKEN_BACKUP, "proto %u will process\n", proto); 

	pos = proto_get(backup, proto);
	if (!pos) {
		pppoe_token_log(TOKEN_BACKUP, "proto %u is not register\n", proto); 
		PBUF_FREE(pbuf);
		return PPPOEERR_ENOEXIST;
	}

	ret = pos->process(pbuf, pos->proto_data);
	pppoe_token_log(TOKEN_BACKUP, "proto %u func %s process, "
							"ret %d\n", proto, pos->funcname, ret); 
	return ret;
}

static inline void
proto_init(backup_struct_t *backup) {
	int i = 0;
	for(; i < PROTOCOL_HASHSIZE; i++) {
		INIT_HLIST_HEAD(&backup->protoHash[i]);
	}
}

static inline void
proto_exit(backup_struct_t *backup) {
	struct backup_proto *sproto;
	struct hlist_node *pos;
	int i = 0;

	for (; i < PROTOCOL_HASHSIZE; i++) {
		hlist_for_each_entry(sproto, pos,
				&backup->protoHash[i], next) {
			free(sproto);					
		}
		INIT_HLIST_HEAD(&backup->protoHash[i]);
	}
}

static inline void
echo_packet(struct pppoe_buf *pbuf, uint16 proto) {
	struct backup_packet *pack
		= (struct backup_packet *)pbuf_put(pbuf, sizeof(struct backup_packet));
	pack->proto = htons(proto);
	pack->length = htons(sizeof(struct backup_packet));
}

static inline int
task_empty(struct task_queue *tasks) {
	return NULL == tasks->head;
}

static inline backup_task_t *
task_queue_get(struct task_queue *tasks) {
	backup_task_t *task;

	if (!tasks->head)
		return NULL;

	task = tasks->head;
	if (tasks->tail == tasks->head) {
		tasks->head = tasks->tail = NULL;
	} else {
		tasks->head = task->next;
	}
	
	return task;	
}

static inline void
task_queue_clear(backup_struct_t *backup) {
	backup_task_t *task;

	while (backup->tasks.head) {
		task = backup->tasks.head;
		backup->tasks.head = task->next;

		PBUF_FREE(task->pbuf);
		backup_task_destroy(backup, &task);
	}

	backup->tasks.tail = backup->tasks.head;
}

static inline void active_disconnect(backup_struct_t *backup);
static inline void standby_disconnect(backup_struct_t *backup);
static int standby_connect(backup_struct_t *backup);

static inline void
backup_disconnect(backup_struct_t *backup) {
	if (BACKUP_ACTIVE == backup->status) {
		active_disconnect(backup);
	} else if (BACKUP_STANDBY == backup->status) {
		standby_disconnect(backup);
	}
}

static int
backup_post_func(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);
	struct backup_task *task;
	char errbuf[128];
	ssize_t length;

	if (unlikely(backup->sk <= 0))
		return PPPOEERR_EINVAL;

	while (1) {
		if (!backup->send) {
			task = task_queue_get(&backup->tasks);
			if (!task) {
				thread_pause(thread);
				goto out;
			}

			backup->send = task->pbuf;
			backup_task_destroy(backup, &task);
		}

		length = send(backup->sk, backup->send->data, backup->send->len, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;
			
			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				goto out;

			pppoe_log(LOG_WARNING, "socket %d send failed: %s\n", 
							backup->sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			goto disconnect;
		}

		pppoe_token_log(TOKEN_BACKUP, "send length %d, pbuf len %u\n",
										length, backup->send->len); 

		if (backup->send->len == length) {
			PBUF_FREE(backup->send); 
		} else {
			pbuf_pull(backup->send, length);
		}
	}
	
out:
	backup->lostTimes = 0;
	return PPPOEERR_SUCCESS;
	
disconnect:
	backup_disconnect(backup);
	return PPPOEERR_ESOCKET;
}


static int
backup_process(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);
	struct backup_packet *pack;
	struct pppoe_buf *pbuf;
	char errbuf[128];	
	ssize_t length;
	uint32 plen;

	if (unlikely(backup->sk <= 0)) {
		pppoe_log(LOG_WARNING, "standby sk %d error\n", backup->sk);
		return PPPOEERR_EINVAL;
	}

	while (1) {
		length = recv(backup->sk, backup->recv->data, 
					backup->recv->end - backup->recv->data, 0);
		if (length < 0) {
			if (EINTR == errno)
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno)
				goto out;

			pppoe_log(LOG_WARNING, "socket %d recv failed: %s\n", 
							backup->sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			goto disconnect;
		} else if (length == 0) {
			pppoe_log(LOG_NOTICE, "active socket closed...\n");
			goto disconnect;
		} 
		
		pbuf_put(backup->recv, length);
		pbuf_push(backup->recv, pbuf_headroom(backup->recv));

		pppoe_token_log(TOKEN_BACKUP, "recv length %d, pbuf len %u", 
										length, backup->recv->len);

		while (backup->recv->len >= sizeof(struct backup_packet)) {
			pack = (struct backup_packet *)backup->recv->data;
			plen = ntohs(pack->length);
			
			if (plen > (backup->recv->end - backup->recv->head)) {	/* must less pbuf max length*/
				pppoe_log(LOG_WARNING, "recv backup packet length %u error\n", 
										plen);
				pbuf_init(backup->recv);
				return PPPOEERR_ELENGTH;
			}

			if (plen > backup->recv->len)
				goto move;
			
			pbuf_pull(backup->recv, plen);

			pbuf = pbuf_alloc(plen);
			if (unlikely(!pbuf)) {
				pppoe_log(LOG_WARNING, "pppoe buf alloc failed\n");
				continue;
			}

			memcpy(pbuf_put(pbuf, plen), pack,  plen);
			pbuf_pull(pbuf, sizeof(struct backup_packet));
			proto_process(backup, ntohs(pack->proto), pbuf);
		}

		if (!backup->recv->len) {
			pbuf_init(backup->recv);
			continue;
		}
		
	move:
		length = backup->recv->len;
		memmove(backup->recv->head, backup->recv->data, length);
		pbuf_reserve(pbuf_init(backup->recv), length);	
	}	

out:
	backup->lostTimes = 0;	/* when recv packet from active, clear losttimes */
	return PPPOEERR_SUCCESS;

disconnect:
	backup_disconnect(backup);
	return PPPOEERR_ESOCKET;
}

static int
active_sock_init(backup_struct_t *backup) {
	struct sockaddr_in addr;
	char errbuf[128];
	int optval = 1, tcpflag = 1;
	int ret;

	if (unlikely(backup->lsk > 0)) {
		pppoe_log(LOG_ERR, "active socket %d is already exist\n", backup->lsk);
		return PPPOEERR_EEXIST;
	}	
	
	backup->lsk = socket(AF_INET, SOCK_STREAM, 0);
	if (unlikely(backup->lsk <= 0)) {
		pppoe_log(LOG_ERR, "active socket(%u.%u.%u.%u:%u) init failed: %s\n",
						HIPQUAD(backup->s_ip), backup->s_port, 
						strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ESOCKET;
		goto error;
	}

	if (setsockopt(backup->lsk, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
		pppoe_log(LOG_ERR, "active socket(%u.%u.%u.%u:%u) setsockopt SO_REUSEADDR failed: sk(%d) %s\n",
						HIPQUAD(backup->s_ip), backup->s_port, backup->lsk, 
						strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ESYSCALL;		
		goto error1;
	}

	if (setsockopt(backup->lsk, IPPROTO_TCP, TCP_NODELAY, &tcpflag, sizeof(tcpflag))) {
		pppoe_log(LOG_ERR, "active socket(%u.%u.%u.%u:%u) setsockopt TCP_NODELAY failed: sk(%d) %s\n",
						HIPQUAD(backup->s_ip), backup->s_port, backup->lsk, 
						strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ESYSCALL;		
		goto error1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(backup->s_ip);
	addr.sin_port = htons(backup->s_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

	if (bind(backup->lsk, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
		pppoe_log(LOG_ERR, "active socket(%u.%u.%u.%u:%u) bind failed: sk(%d), %s",
							HIPQUAD(backup->s_ip), backup->s_port, backup->lsk, 
							strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_EBIND;		
		goto error1;
	}

	if (listen(backup->lsk, 32)) {
		pppoe_log(LOG_ERR, "active socket(%u.%u.%u.%u:%u) listen failed: sk(%d), %s",
							HIPQUAD(backup->s_ip), backup->s_port, backup->lsk, 
							strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ELISTEN;	
		goto error1;
	}

	backup->c_state = CHANNEL_INIT;
	pppoe_log(LOG_INFO, "active socket(%u.%u.%u.%u:%u) %d init success\n",
						HIPQUAD(backup->s_ip), backup->s_port, backup->lsk);
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(backup->lsk);
error:
	return ret;
}

static int
active_echo_timer_func(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);

	if (backup->lostTimes >= backup->maxTimes) {
		pppoe_log(LOG_WARNING, "lostTimes(%u) is over maxTimes(%u)\n",
								backup->lostTimes, backup->maxTimes);
		active_disconnect(backup);
		return PPPOEERR_ESOCKET;
	}

	if (backup->send || !task_empty(&backup->tasks)) /* is already have packet */
		goto out;
	
	backup->send = pbuf_alloc(sizeof(struct backup_packet));
	if (unlikely(!backup->send)) {
		pppoe_log(LOG_WARNING, "pppoe buf alloc failed\n");
		goto out;
	}
	
	echo_packet(backup->send, BACKUP_ECHO_REQUEST);	
	thread_start(backup->post);	/* start backup post thread to send request */

out:
	backup->lostTimes++;
	thread_update_timer(thread, DEFAULT_BACKUP_ECHO_INTERVAL, THREAD_EXTIME_NONE);
	return PPPOEERR_SUCCESS;
}

static int
active_listen_process(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);
	socklen_t in_len = sizeof(struct sockaddr_in);
    struct sockaddr_in in_addr;	
	char errbuf[128];
	uint32 ip;
	uint16 port;
	int sk, ret;

    if ((sk = accept(backup->lsk, (struct sockaddr *)&in_addr, &in_len)) < 0) {
		pppoe_log(LOG_WARNING, "accept failed: %s\n", 
							strerror_r(errno, errbuf, sizeof(errbuf)));
        ret = PPPOEERR_ESOCKET;
		goto error;		
    }

	/* must check connect validity */
	ip = ntohl(in_addr.sin_addr.s_addr);
	port = ntohs(in_addr.sin_port);
	if (backup->d_ip && backup->d_ip != ip) {
		pppoe_log(LOG_WARNING, "accept ip(%u.%u.%u.%u) is not match(%u.%u.%u.%u)\n",
								HIPQUAD(ip), HIPQUAD(backup->d_ip));
        ret = PPPOEERR_EINVAL;
		goto error1;
	}

	pppoe_log(LOG_INFO, "active socket %d accept form %u.%u.%u.%u:%u\n", 
						sk, HIPQUAD(ip), port);

	/* frist check socket and thread */
	if (unlikely(backup->thread)) {
		THREAD_CANCEL(backup->thread);
	}

	if (unlikely(backup->thread)) {
		THREAD_CANCEL(backup->thread);
	}

	if (unlikely(backup->sk > 0)) {
		PPPOE_CLOSE(backup->sk);
	}
	
	backup->sk = sk;

	ret = set_nonblocking(backup->sk);
	if (unlikely(ret)) {
		pppoe_log(LOG_ERR, "active socket %d set nonblocking failed\n", backup->sk);
		goto error2;
	}
	
	/* active add socket read */	
	backup->thread = thread_add_read(backup->master, 
									backup_process, 
									backup, backup->sk);
	if (unlikely(!backup->thread)) {
		pppoe_log(LOG_WARNING, "active thread socket %d add read failed\n", backup->sk);
		ret = PPPOEERR_ENOMEM;
		goto error2;
	}

	/* active add task post */
	backup->post = thread_add_write(backup->master, 
									backup_post_func, 
									backup, backup->sk);
	if (unlikely(!backup->post)) {
		pppoe_log(LOG_WARNING, "active task post socket %d add write failed\n", backup->sk);
		ret = PPPOEERR_ENOMEM;
		goto error3;
	}
	thread_pause(backup->post); /* need pause post thread */
	
	/* active add echo timer */
	backup->lostTimes = 0;
	backup->timer = thread_add_timer(backup->master, 
									active_echo_timer_func, 
									backup, 
									DEFAULT_BACKUP_ECHO_INTERVAL, 
									THREAD_EXTIME_NONE);
	if (unlikely(!backup->timer)) {
		pppoe_log(LOG_WARNING, "active echo timer add failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error4;
	}

	backup->c_state = CHANNEL_CONNECT;
	notifier_chain_event(backup->chain, CHANNEL_CONNECT);
	pppoe_log(LOG_INFO, "active accept socket %d add write\n", backup->sk);
    return PPPOEERR_SUCCESS;

error4:
	THREAD_CANCEL(backup->post);	
error3:
	THREAD_CANCEL(backup->thread);	
error2:
	backup->sk = -1;
error1:
	close(sk);
error:
	return ret;
}

static inline void
active_disconnect(backup_struct_t *backup) {
	pbuf_init(backup->recv);
	PBUF_FREE(backup->send);

	task_queue_clear(backup);

	THREAD_CANCEL(backup->post);	
	THREAD_CANCEL(backup->thread);	
	THREAD_CANCEL(backup->timer);	

	PPPOE_CLOSE(backup->sk);
	
	backup->c_state = CHANNEL_DISCONNECT;
	notifier_chain_event(backup->chain, CHANNEL_DISCONNECT);
}


static int
standby_sock_init(backup_struct_t *backup) {
	struct sockaddr_in addr;
	char errbuf[128];
	int ret;
	
	if (unlikely(backup->sk > 0)) {
		pppoe_log(LOG_ERR, "standby socket %d is already exist\n", backup->sk);
		return PPPOEERR_EEXIST;
	}	
	
	backup->sk = socket(AF_INET, SOCK_STREAM, 0);
	if (unlikely(backup->sk < 0)) {
		pppoe_log(LOG_ERR, "standby socket(%u.%u.%u.%u) init failed: %s\n",
					HIPQUAD(backup->s_ip), strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ESOCKET;
		goto error;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(backup->s_ip);
	addr.sin_port = 0;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

	if (bind(backup->sk, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
		pppoe_log(LOG_ERR, "standby socket(%u.%u.%u.%u) bind failed: sk(%d), %s",
							HIPQUAD(backup->s_ip), backup->sk, 
							strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_EBIND;
		goto error1;
	}

	backup->c_state = CHANNEL_INIT;	
	pppoe_log(LOG_INFO, "standby socket(%u.%u.%u.%u:%u) %d init success\n",
						HIPQUAD(backup->s_ip), backup->s_port, backup->sk);	
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(backup->sk);
error:
	return ret;
}

static inline int
standby_echo_timer_func(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);
	
	if (backup->lostTimes >= backup->maxTimes) {
		pppoe_log(LOG_WARNING, "lostTimes(%u) is over maxTimes(%u)\n",
								backup->lostTimes, backup->maxTimes);
		standby_disconnect(backup);
		return PPPOEERR_ESOCKET;
	}

	backup->lostTimes++;
	thread_update_timer(thread, DEFAULT_BACKUP_ECHO_INTERVAL, THREAD_EXTIME_NONE);
	return PPPOEERR_SUCCESS;
}

static int
standby_connect_timer_func(thread_struct_t *thread) {
	backup_struct_t *backup = thread_get_arg(thread);
	int ret;
	
	ret = standby_sock_init(backup);
	if (ret) {
		pppoe_log(LOG_WARNING, "standby socket init failed\n");
		goto error;
	}


	ret = standby_connect(backup);
	if (ret) {
		pppoe_log(LOG_WARNING, "standby socket connect failed\n");
		goto error;
	}

	return PPPOEERR_SUCCESS;

error:
	thread_update_timer(thread, DEFAULT_BACKUP_CONNECT_INTERVAL, THREAD_EXTIME_NONE);
	return ret;
}


static int
standby_connect(backup_struct_t *backup) {
    struct sockaddr_in addr;	
	thread_struct_t *timer;	
	char errbuf[128];
	int ret;

	if (unlikely(backup->c_state != CHANNEL_INIT)) {
		pppoe_log(LOG_WARNING, "standby state error\n");
		ret = PPPOEERR_ESTATE;
		goto error;
	}

	if (unlikely(backup->sk <= 0)) {
		pppoe_log(LOG_WARNING, "input socket %d is error\n", backup->sk);
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(backup->d_ip);
	addr.sin_port = htons(backup->d_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

	if (connect(backup->sk, &addr, sizeof(struct sockaddr_in))) {
		pppoe_log(LOG_WARNING, "standby socket %d connect to %u.%u.%u.%u:%u failed: %s\n", 
								backup->sk, HIPQUAD(backup->d_ip), backup->d_port, 
								strerror_r(errno, errbuf, sizeof(errbuf)));
		ret = PPPOEERR_ECONNECT;
		goto error1;
	}

	ret = set_nonblocking(backup->sk);
	if (unlikely(ret)) {
		pppoe_log(LOG_ERR, "standby socket %d set nonblocking failed\n", backup->sk);
		goto error1;
	}

	/* standby add socket recv */
	backup->thread = thread_add_read(backup->master, 
									backup_process, 
									backup, backup->sk);
	if (unlikely(!backup->thread)) {
		pppoe_log(LOG_ERR, "standby socket %d add read failed\n", backup->sk);
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	/* standby add task post */
	backup->post = thread_add_write(backup->master, 
									backup_post_func, 
									backup, backup->sk);
	if (unlikely(!backup->post)) {
		pppoe_log(LOG_WARNING, "standby post socket %d add write failed\n", backup->sk);
		ret = PPPOEERR_ENOMEM;
		goto error2;
	}
	thread_pause(backup->post); /* need pause post thread */

	/* standby add echo timer */
	backup->lostTimes = 0;
	timer = thread_add_timer(backup->master, 
							standby_echo_timer_func, backup, 
							DEFAULT_BACKUP_ECHO_INTERVAL, 
							THREAD_EXTIME_NONE);
	if (unlikely(!timer)) {
		pppoe_log(LOG_ERR, "standby echo timer add failed\n");
		ret = PPPOEERR_ENOMEM;
		goto error3;
	}

	THREAD_CANCEL(backup->timer);	/* del standby connect timer*/
	backup->timer = timer;

	backup->c_state = CHANNEL_CONNECT;	
	notifier_chain_event(backup->chain, CHANNEL_CONNECT);
	pppoe_log(LOG_INFO, "standby socket %d connect to %u.%u.%u.%u:%u\n",
						backup->sk, HIPQUAD(backup->d_ip), backup->d_port);

	return PPPOEERR_SUCCESS;

error3:
	THREAD_CANCEL(backup->post);
error2:
	THREAD_CANCEL(backup->thread);	
error1:
	PPPOE_CLOSE(backup->sk);
	backup->c_state = CHANNEL_NONE;
error:
	return ret;
}

static inline void
standby_disconnect(backup_struct_t *backup) {
	thread_struct_t *timer;

	pbuf_init(backup->recv);
	PBUF_FREE(backup->send);

	task_queue_clear(backup);

	THREAD_CANCEL(backup->post);	
	THREAD_CANCEL(backup->thread);	

	PPPOE_CLOSE(backup->sk);

	backup->c_state = CHANNEL_DISCONNECT;
	notifier_chain_event(backup->chain, CHANNEL_DISCONNECT);
	
	timer = thread_add_timer(backup->master, 
							standby_connect_timer_func, 
							backup, 
							DEFAULT_BACKUP_CONNECT_INTERVAL, 
							THREAD_EXTIME_NONE);
	if (unlikely(!backup->timer)) {	/* timer add failed, need echo timer retry */
		pppoe_log(LOG_ERR, "standby connect timer add failed\n");
		return;
	}
	
	THREAD_CANCEL(backup->timer);	/* delete echo timer thread */
	backup->timer = timer;
}

static int
standby_proto_echo_process(struct pppoe_buf *pbuf, void *proto_data) {
	backup_struct_t *backup = (backup_struct_t *)proto_data;

	if (backup->send) {
		PBUF_FREE(pbuf);
		return PPPOEERR_SUCCESS;
	}

	backup->send = pbuf;	
	echo_packet(pbuf_init(backup->send), BACKUP_ECHO_REPLY);

	thread_start(backup->post);	/* start backup post thread to send reply */
	return PPPOEERR_SUCCESS;
}

backup_task_t *
backup_task_create(backup_struct_t *backup, 
				struct pppoe_buf *pbuf, uint16 proto) {
	struct backup_packet *pack;
	backup_task_t *task;

	if (unlikely(!backup || !pbuf)) {
		pppoe_log(LOG_WARNING, "input backup or pbuf is NULL\n");
		return NULL;
	}

	if (unlikely(pbuf_headroom(pbuf) < sizeof(struct backup_packet))) {
		pppoe_log(LOG_WARNING, "pppoe buf headroom is not enough\n");
		return NULL;
	}

	task = (backup_task_t *)mem_cache_alloc(backup->cache);
	if (unlikely(!task)) {
		pppoe_log(LOG_WARNING, "alloc task failed\n");
		return NULL;
	}

	memset(task, 0, sizeof(*task));

	pack = (struct backup_packet *)pbuf_push(pbuf, sizeof(struct backup_packet));
	memset(pack, 0, sizeof(struct backup_packet));
	pack->proto = htons(proto);
	pack->length = htons(pbuf->len);
	
	task->pbuf = pbuf;
	return task;
}

int
backup_task_add(backup_struct_t *backup, backup_task_t *task) {
	struct task_queue *queue;

	if (unlikely(!backup || !task)) 
		return PPPOEERR_EINVAL;

	if (backup->c_state != CHANNEL_CONNECT) 
		return PPPOEERR_ESTATE;
	
	queue = &backup->tasks;
	if (!queue->tail) {
		queue->head = task;
		thread_start(backup->post);	/* need start post thread */
	} else {
		queue->tail->next = task;
	}
	queue->tail = task;

	return PPPOEERR_SUCCESS;
}

void
backup_task_destroy(backup_struct_t *backup, backup_task_t **task) {
	if (unlikely(!backup || !task || !*task))
		return;

	mem_cache_free(backup->cache, *task);
	*task = NULL;
}

int
backup_proto_register_with_funcname(backup_struct_t *backup, 
								uint16 proto, backupProtoFunc process, 
								void *proto_data, const char *funcname) {
	struct backup_proto *pos;

	if (unlikely(!process || !funcname))
		return PPPOEERR_EINVAL;

	pos = proto_get(backup, proto);
	if (unlikely(pos)) 
		return PPPOEERR_EEXIST;

	pos = (struct backup_proto *)malloc(sizeof(struct backup_proto));
	if (unlikely(!pos))
		return PPPOEERR_ENOMEM;

	memset(pos, 0, sizeof(*pos));
	pos->proto = proto; 
	pos->process = process;
	pos->proto_data = proto_data;
	strncpy(pos->funcname, funcname, sizeof(pos->funcname) - 1);

	hlist_add_head(&pos->next, proto_hash(backup, proto));
	return PPPOEERR_SUCCESS;
}

int
backup_proto_unregister(backup_struct_t *backup, uint16 proto) {
	struct backup_proto *pos = proto_get(backup, proto);
	if (unlikely(!pos)) 
		return PPPOEERR_ENOEXIST;

	hlist_del(&pos->next);
	free(pos);
	return PPPOEERR_SUCCESS;
}

int
backup_notifier_register(backup_struct_t *backup, struct notifier_struct *notifier) {
	if (unlikely(!backup || !notifier))
		return PPPOEERR_EINVAL;
	
	return notifier_chain_register(backup->chain, notifier);
}

int
backup_notifier_unregister(backup_struct_t *backup, struct notifier_struct *notifier) {
	if (unlikely(!backup || !notifier))
		return PPPOEERR_EINVAL;

	return notifier_chain_unregister(backup->chain, notifier);
}

static inline void
backup_setup(backup_struct_t *backup) {
	backup->sk = -1;
	backup->lsk = -1;
	backup->maxTimes = DEFAULT_BACKUP_ECHO_TIMES;
	backup->c_state = CHANNEL_NONE;
}

uint32
backup_status(backup_struct_t *backup) {
	return backup->status;
}

uint32
backup_prevstatus(backup_struct_t *backup) {
	return backup->prev_status;
}

uint32
backup_channel_state(backup_struct_t *backup) {
	if (backup->c_state == CHANNEL_CONNECT)
		return 1;

	return 0;
}

static inline int
backup_active_start(backup_struct_t *backup) {
	int ret;

	ret = active_sock_init(backup);
	if (ret) {
		pppoe_log(LOG_ERR, "active socket init failed, return %d\n", ret);
		goto error;
	}

	backup->listen = thread_add_read(backup->master, 
									active_listen_process, 
									backup, backup->lsk);
	if (unlikely(!backup->listen)) {
		pppoe_log(LOG_ERR, "active listen socket %d add read failed\n", backup->lsk);
		ret = PPPOEERR_ENOMEM;
		goto error1;
	}

	backup->status = BACKUP_ACTIVE;
	pppoe_log(LOG_INFO, "backup active start success\n");	
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(backup->sk);
error:
	return ret;
}

void
backup_active_stop(backup_struct_t *backup) {
	pbuf_init(backup->recv);
	PBUF_FREE(backup->send);

	task_queue_clear(backup);

	THREAD_CANCEL(backup->post);
	THREAD_CANCEL(backup->timer);
	THREAD_CANCEL(backup->listen);
	THREAD_CANCEL(backup->thread);

	PPPOE_CLOSE(backup->sk);
	PPPOE_CLOSE(backup->lsk);
	
	backup_setup(backup);
	pppoe_log(LOG_INFO, "backup active stop success\n");	
}


static inline int
backup_standby_start(backup_struct_t *backup) {
	int ret;

	ret = standby_sock_init(backup);
	if (ret) {
		pppoe_log(LOG_ERR, "standby socket init fail, return %d\n", ret);
		goto error;
	}

	ret = standby_connect(backup);
	if (ret) { 
		backup->timer = thread_add_timer(backup->master, 
										standby_connect_timer_func, 
										backup, 
										DEFAULT_BACKUP_CONNECT_INTERVAL, 
										THREAD_EXTIME_NONE);
		if (unlikely(!backup->timer)) {
			pppoe_log(LOG_ERR, "standby connect timer add failed\n");
			ret = PPPOEERR_ENOMEM;
			goto error1;
		}
	}	
		
	backup->status = BACKUP_STANDBY;
	pppoe_log(LOG_INFO, "backup standby start success\n");	
	return PPPOEERR_SUCCESS;

error1:
	PPPOE_CLOSE(backup->sk);
error:
	return ret;
}

static inline void
backup_standby_stop(backup_struct_t *backup) {
	pbuf_init(backup->recv);
	PBUF_FREE(backup->send);

	task_queue_clear(backup);

	THREAD_CANCEL(backup->post);
	THREAD_CANCEL(backup->timer);
	THREAD_CANCEL(backup->thread);

	PPPOE_CLOSE(backup->sk);
	
	backup_setup(backup);
	pppoe_log(LOG_INFO, "backup standby stop success\n");
}

int
backup_status_setup(backup_struct_t *backup, uint32 status,
				uint32 s_ip, uint16 s_port, uint32 d_ip, uint16 d_port) {
	int ret = PPPOEERR_SUCCESS;
				
	if (unlikely(!backup))
		return PPPOEERR_EINVAL;

	if (unlikely(status >= BACKUP_STATUSNUMS)) {
		pppoe_log(LOG_ERR, "unknown backup status %u\n", status);
		return PPPOEERR_EINVAL;
	}

	switch (backup->status) {
	case BACKUP_NONE:
		break;
		
	case BACKUP_ACTIVE:
		backup_active_stop(backup);
		break;
		
	case BACKUP_STANDBY:
		backup_standby_stop(backup);
		break;
		
	case BACKUP_DISABLE:
		break;
	}

	backup->s_ip = s_ip;
	backup->s_port = s_port;
	backup->d_ip = d_ip;
	backup->d_port = d_port;
	backup->prev_status = backup->status;
	backup->status = status;

	switch (backup->status) {
	case BACKUP_NONE:
		break;
		
	case BACKUP_ACTIVE:
		ret= backup_active_start(backup);
		break;
		
	case BACKUP_STANDBY:
		ret = backup_standby_start(backup);
		break;
		
	case BACKUP_DISABLE:
		break;
	}

	pppoe_log(LOG_INFO, "%s turn to %s, ret %d\n", 
						backup_statusnames[backup->prev_status],
						backup_statusnames[backup->status], ret);
	return ret;
}

backup_struct_t *
backup_init(thread_master_t *master) {
	backup_struct_t *backup;

	if (unlikely(!master)) {
		pppoe_log(LOG_ERR, "input thread master is NULL\n");
		goto error;
	}
	
	backup = (backup_struct_t *)malloc(sizeof(backup_struct_t));
	if (unlikely(!backup)) {
		pppoe_log(LOG_ERR, "alloc backup failed\n");
		goto error;
	}

	memset(backup, 0, sizeof(*backup));
	backup->master = master;

	backup->cache = mem_cache_create(BACKUP_CACHE_NAME, 
									BACKUP_CACHE_MAX_BLKNUM, 
									BACKUP_CACHE_EMPTY_BLKNUM,
									sizeof(backup_task_t), 
									BACKUP_CACHE_BLK_ITEMNUM);
	if (unlikely(!backup->cache)) {
		pppoe_log(LOG_ERR, "%s create failed\n", BACKUP_CACHE_NAME);
		goto error1;
	}	
	
	backup->recv = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!backup->recv)) {
		pppoe_log(LOG_ERR, "alloc backup recv pbuf failed\n");
		goto error2;
	}

	backup->chain = notifier_chain_create();
	if (unlikely(!backup->chain)) {
		pppoe_log(LOG_ERR, "backup notifier chain create failed\n");
		goto error3;
	}
	
	/* backup packet protocol init*/
	proto_init(backup);	
	if (backup_proto_register(backup, BACKUP_ECHO_REQUEST,
							standby_proto_echo_process, backup)) {
		pppoe_log(LOG_ERR, "backup echo proto register failed\n");
		goto error4;
	}

	backup_setup(backup);
	backup->status = BACKUP_NONE;
	pppoe_log(LOG_INFO, "backup init success\n");	
	return backup;

error4:
	notifier_chain_destroy(&backup->chain);
error3:
	PBUF_FREE(backup->recv);
error2:
	mem_cache_destroy(&backup->cache);
error1:
	PPPOE_FREE(backup);
error:
	return NULL;
}

void
backup_exit(backup_struct_t **backup) {
	if (!backup || !(*backup))
		return;

	if (BACKUP_ACTIVE == (*backup)->status) {
		backup_active_stop(*backup);
	} else if (BACKUP_STANDBY == (*backup)->status) {
		backup_standby_stop(*backup);
	}

	proto_exit(*backup);
	notifier_chain_destroy(&(*backup)->chain);
	PBUF_FREE((*backup)->recv);
	mem_cache_destroy(&(*backup)->cache);	
	PPPOE_FREE(*backup);
	pppoe_log(LOG_INFO, "backup exit success\n");	
}

