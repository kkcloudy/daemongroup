
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_method.h"

#include "thread_bus.h"

struct tbus_message {
	uint32 dest;
	uint32 source;
	uint32 flag;

	int errcode;
	void *data;
	void *para;
	uint32 method_id;	
	tbusDataFree free;

	struct tbus_message *next;
};

struct tbus_queue {
	struct tbus_message *head;
	struct tbus_message *tail;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;

};

struct tbus_connection {
	uint32 id;
	uint32 refcnt;
	thread_bus_t *tbus;
	struct tbus_queue queue;
};

struct thread_bus {
	uint32 connection_num;
	tbus_connection_t	**connection;
	pthread_rwlock_t	lock;
};

static inline void
connection_hold(tbus_connection_t *connection) {
	connection->refcnt++;
}

static inline void
connection_put(tbus_connection_t *connection) {
	connection->refcnt--;
}

static inline tbus_connection_t *
connnection_get(thread_bus_t *tbus, uint32 id) {
	tbus_connection_t *connection;

	pthread_rwlock_rdlock(&tbus->lock);
	connection = tbus->connection[id];
	if (connection) 
		connection_hold(connection);
	pthread_rwlock_unlock(&tbus->lock);

	return connection;
}


static inline struct tbus_message *
message_create(uint32 method_id, void *data, void *para, tbusDataFree dfree) {
	struct tbus_message *mess
		= (struct tbus_message *)malloc(sizeof(struct tbus_message));
	if (unlikely(!mess)) {
		return NULL;
	}

	memset(mess, 0, sizeof(*mess));
	mess->data = data;
	mess->para = para;
	mess->free = dfree;
	mess->method_id = method_id;
	
	return mess;
}

static inline void
message_queue_get(struct tbus_queue *queue, struct tbus_message **list, uint32 len) {
	if (queue->head) {
		pthread_mutex_lock(&queue->lock);		
		if (len) {
			struct tbus_message *mess;
			uint32 i = 0;

			*list = queue->head;
			while (1) {
				mess = queue->head;
				queue->head = mess->next;

				if (!queue->head) {
					queue->tail = queue->head;
					break;
				}				

				if (++i >= len) {
					mess->next = NULL;
					break;
				}
			}	
		} else {
			*list = queue->head;
			queue->head = queue->tail = NULL;
		}
		pthread_mutex_unlock(&queue->lock);
	} else {
		*list = NULL;
	}
}

static inline void
message_queue_add(struct tbus_queue *queue, struct tbus_message *mess) {
	pthread_mutex_lock(&queue->lock);

	if (!queue->tail) {
		queue->head = mess;
	} else {
		queue->tail->next = mess;
	}
	queue->tail = mess;

	pthread_mutex_unlock(&queue->lock);
}

static inline int
__message_reply_get(struct tbus_queue *queue, struct tbus_message *mess) {
	struct tbus_message *node, *prev;

	for (node = queue->head, prev = node; 
		NULL != node; prev = node, node = node->next) {
		if (node == mess) {
			if (node == prev) {
				if (queue->head == queue->tail) {
					queue->head = queue->tail = NULL;
				} else {
					queue->head = node->next;
				}
			} else {
				prev->next = node->next;
			}
			return PPPOEERR_SUCCESS;
		}
	}

	return PPPOEERR_ENOEXIST;
}


static inline int
message_queue_reply_wait(struct tbus_queue *queue, struct tbus_message *mess, uint32 timeout) {
	struct timespec tmp_time;
	int ret = PPPOEERR_SUCCESS;

	if (timeout) {
		tmp_time.tv_sec		= time(NULL) + timeout; 
		tmp_time.tv_nsec	= 0;

		pthread_mutex_lock(&queue->lock);
		do {
			if (ETIMEDOUT == pthread_cond_timedwait(&queue->cond, 
											&queue->lock, &tmp_time)) {
				ret = PPPOEERR_ETIMEOUT;
				goto out;
			}
		} while (__message_reply_get(queue, mess));
		pthread_mutex_unlock(&queue->lock);
		
	} else {
		pthread_mutex_lock(&queue->lock);
		do {
			pthread_cond_wait(&queue->cond, &queue->lock) ; 
		} while (__message_reply_get(queue, mess));
		pthread_mutex_unlock(&queue->lock);
	}
	
out:
	return ret;
}

static inline void
connection_queue_init(tbus_connection_t *connection) {
	memset(&connection->queue, 0, sizeof(connection->queue));
	pthread_mutex_init(&connection->queue.lock, NULL);
	pthread_cond_init(&connection->queue.cond, NULL);
}


static inline void
connection_queue_exit(tbus_connection_t *connection) {
	struct tbus_message *mess;
	
	while (connection->queue.head) {
		mess = connection->queue.head;
		connection->queue.head = mess->next;

		if (mess->free) {
			mess->free(mess->data);
		}
		PPPOE_FREE(mess);
	}

	connection->queue.tail = connection->queue.head;
	pthread_mutex_destroy(&connection->queue.lock);
	pthread_cond_destroy(&connection->queue.cond);
	memset(&connection->queue, 0, sizeof(connection->queue));
}

static inline void
connection_dispatch(tbus_connection_t *connection, uint32 len) {
	struct tbus_message *list, *mess;
	tbus_connection_t *s_connect;
	thread_bus_t *tbus = connection->tbus;

	message_queue_get(&connection->queue, &list, len);
	while (list) {
		mess = list;
		list = mess->next;

		if (mess->flag & TBUS_FLAG_UNHANDLE) {
			pppoe_token_log(TOKEN_TBUS, "message %s no need handle\n", 
					(mess->flag & TBUS_FLAG_REPLY) ? "reply" : "request");
			goto endProcess;
		}
				
		mess->errcode = pppoe_method_perform(mess->method_id, mess->data, mess->para);
		pppoe_token_log(TOKEN_TBUS, "pppoe_method_perform %u, errcode %d\n", 
									mess->method_id, mess->errcode);
		
		if (mess->flag & TBUS_FLAG_REPLY) {
			mess->flag |= TBUS_FLAG_UNHANDLE;

			s_connect = connnection_get(tbus, mess->source);
			if (!s_connect) {
				pppoe_log(LOG_WARNING, "source connection %u is not exist\n", mess->source);
				goto endProcess;
			}

			message_queue_add(&s_connect->queue, mess);
			pthread_cond_broadcast(&s_connect->queue.cond);

			connection_put(s_connect);
			continue;
		}

	endProcess:
		if (mess->free && (!(mess->flag & TBUS_FLAG_ERRFREE) || mess->errcode)) {
			mess->free(mess->data);
		}
		PPPOE_FREE(mess);
	}
}

int
tbus_send_signal(tbus_connection_t *connection, uint32 dest,
					uint32 method_id, void *data, void *para, 
					tbusDataFree dfree, uint32 flag) {
	tbus_connection_t *d_connect;
	struct tbus_message *mess;
	thread_bus_t *tbus;	
	int ret;
					
	if (unlikely(!connection)) {
		pppoe_log(LOG_WARNING, "input connection is NULL\n");
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	tbus = connection->tbus;
	if (unlikely(dest >= tbus->connection_num 
		|| dest == connection->id)) {
		pppoe_log(LOG_WARNING, "input dest %d is error\n", dest);
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	mess = message_create(method_id, data, para, dfree);
	if (unlikely(!mess)) {
		pppoe_log(LOG_WARNING, "malloc message fail\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}

	mess->dest = dest;
	mess->source = connection->id;
	mess->flag |= flag;

	d_connect = connnection_get(tbus, dest);
	if (!d_connect) {
		pppoe_log(LOG_WARNING, "dest connection %u is not exist\n", dest);
		ret = PPPOEERR_ENOEXIST;
		goto error1;
	}

	message_queue_add(&d_connect->queue, mess);
	connection_put(d_connect);
	return PPPOEERR_SUCCESS;
	
error1:
	PPPOE_FREE(mess);
error:
	if (dfree) {
		dfree(data);
	}
	return ret;
}

int
tbus_send_method_call_with_reply(tbus_connection_t *connection, uint32 dest,
									uint32 method_id, void *data, void *para,
									tbusDataFree dfree, uint32 flag, uint32 timeout) {
	tbus_connection_t *d_connect;
	struct tbus_message *mess;
	thread_bus_t *tbus;	
	int ret;

	if (unlikely(!connection)) {
		pppoe_log(LOG_WARNING, "input connection is NULL\n");
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	tbus = connection->tbus;
	if (unlikely(dest >= tbus->connection_num 
		|| dest == connection->id)) {
		pppoe_log(LOG_WARNING, "input dest %d is error\n", dest);
		ret = PPPOEERR_EINVAL;
		goto error;
	}

	mess = message_create(method_id, data, para, dfree);
	if (unlikely(!mess)) {
		pppoe_log(LOG_WARNING, "malloc message fail\n");
		ret = PPPOEERR_ENOMEM;
		goto error;
	}
	
	mess->dest = dest;
	mess->source = connection->id;
	mess->flag |= (flag | TBUS_FLAG_REPLY);

	d_connect = connnection_get(tbus, dest);
	if (!d_connect) {
		pppoe_log(LOG_WARNING, "dest connection %u is not exist\n", dest);
		ret = PPPOEERR_ENOEXIST;
		goto error1;
	}

	message_queue_add(&d_connect->queue, mess);
	ret = message_queue_reply_wait(&connection->queue, mess, timeout);
	if (ret) {
		pppoe_log(LOG_WARNING, "get reply failed, ret %d\n", ret);
	} else {
		ret = mess->errcode;
		if (mess->free) {
			mess->free(mess->data);
		}
		PPPOE_FREE(mess);
	}

	connection_put(d_connect);
	return ret;

error1:
	PPPOE_FREE(mess);
error:
	if (dfree) {
		dfree(data);
	}
	return ret;
}


tbus_connection_t *
tbus_connection_create(thread_bus_t *tbus, uint32 connect_id) {
	tbus_connection_t *connection;

	if (unlikely(!tbus)) {
		pppoe_log(LOG_WARNING, "input tbus is NULL\n");
		goto error;
	}

	if (unlikely(connect_id >= tbus->connection_num)) {
		pppoe_log(LOG_WARNING, "input connection id %u is error\n", connect_id);
		goto error;
	}	
	
	connection = (tbus_connection_t *)malloc(sizeof(tbus_connection_t));
	if (unlikely(!connection)) {
		pppoe_log(LOG_WARNING, "tbus connection %u create: malloc failed\n", connect_id);
		goto error;
	}

	memset(connection, 0, sizeof(*connection));
	connection->id = connect_id;
	connection->tbus = tbus;	
	connection_queue_init(connection);

	pthread_rwlock_wrlock(&tbus->lock);
	if (unlikely(tbus->connection[connect_id])) {
		pppoe_log(LOG_WARNING, "tbus connection %u create: is already exist\n", connect_id);
		pthread_rwlock_unlock(&tbus->lock);
		goto error1;
	}
	
	tbus->connection[connect_id] = connection;
	pthread_rwlock_unlock(&tbus->lock);
	
	pppoe_log(LOG_INFO, "tbus connection %u create success\n", connect_id);
	return connection;

error1:
	PPPOE_FREE(connection);
error:
	return NULL;
}

void
tbus_connection_destroy(tbus_connection_t **connection) {
	thread_bus_t *tbus;

	if (unlikely(!connection || !(*connection)))
		return;

	tbus = (*connection)->tbus;
	if (unlikely((*connection)->id >= tbus->connection_num)) {
		pppoe_log(LOG_WARNING, "connection id %u is over tbus max %u\n",
								(*connection)->id, tbus->connection_num);
		return;
	}

	pthread_rwlock_wrlock(&tbus->lock);
	tbus->connection[(*connection)->id] = NULL;
	pthread_rwlock_unlock(&tbus->lock);	

	/* if other thread hold this connnection, must wait */
	while ((*connection)->refcnt) {
		usleep(100000);
	}
	
	connection_queue_exit(*connection);
	PPPOE_FREE(*connection);
}

int
tbus_connection_dispatch(tbus_connection_t *connection, uint32 len) {
	if (unlikely(!connection))
		return PPPOEERR_EINVAL;

	connection_dispatch(connection, len);
	return PPPOEERR_SUCCESS;
}


thread_bus_t *
thread_bus_init(uint32 connection_num) {
	thread_bus_t *tbus;

	tbus = (thread_bus_t *)malloc(sizeof(thread_bus_t) + 
								connection_num * sizeof(tbus_connection_t *));
	if (unlikely(!tbus)) {
		pppoe_log(LOG_ERR, "thread bus alloc memory failed\n");
		return NULL;
	}
	
	memset(tbus, 0, sizeof(thread_bus_t) + connection_num * sizeof(tbus_connection_t *));
	tbus->connection_num = connection_num;
	tbus->connection = (void *)tbus + sizeof(thread_bus_t);
	pthread_rwlock_init(&tbus->lock, NULL);
	
	pppoe_log(LOG_INFO, "thread bus init success, connection num %u\n", connection_num);
	return tbus;
}

void 
thread_bus_exit(thread_bus_t **tbus) {
	if (unlikely(!tbus || !(*tbus)))
		return;

	pthread_rwlock_destroy(&(*tbus)->lock);	
	PPPOE_FREE(*tbus);
	pppoe_log(LOG_INFO, "thread bus exit success\n");
}

