
/** include glibc **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include <pthread.h>

/** include manage **/
#include "manage_log.h"
#include "manage_type.h"
#include "manage_transport.h"

#include "manage_api.h"


typedef struct manage_task_s	manage_task;

/*
 * Internal information about the state of the manage session.
 */
struct manage_internal_session {
	int (*hook_pre) (void *, u_char *, size_t, void *, size_t);
	
	manage_task *(*hook_task_create) (void *, void *, size_t);
	
	int (*hook_task_parse) (manage_task *, u_char *, size_t);
	
	int (*hook_post) (manage_session *, manage_task *, int);
};

struct session_list {
	struct session_list *next;
	manage_session *session;
	manage_transport *transport;
	struct manage_internal_session *internal;
};

struct manage_task_s {
	u_long  task_id;     			/* task id */
	
	manage_message 	message;
	task_method		method;				/* user method process task */

	void		*transport_data;
	size_t     transport_data_length;

	void		*para_data;				/* user para data for process task */
	size_t	para_data_length;

	void			*method_data;		/* user method data process task */
	size_t		method_data_length;
	
	u_long          timeout;        		/* length to wait for timeout */
	struct timeval  time;   				/* Time this task was made */
	struct timeval  expire; 			/* time this task is due to expire */

	struct session_list 	*sessp;
	struct manage_task_s	*next;
};


static struct session_list 		*Sessions = NULL;
static manage_task 			*task_queue = NULL, *task_tail = NULL;
static manage_method			*task_method_array = NULL;
static u_short					  task_method_size = 0;


static u_long global_sessid 	= 0;
static u_long global_taskid		= 0;


static pthread_mutex_t session_mutex;
static pthread_mutex_t task_mutex;
static pthread_cond_t	task_cond;

static void *manage_sess_open(manage_session * pss);

static int manage_sess_read(void *sessp, fd_set *fdset);

static void manage_sess_free(manage_session *s);

static int manage_sess_close(void *sessp);

static int manage_sess_send(manage_session *session, 
								void *packetptr, size_t length, 
								void *transport_data, size_t transport_data_length);

static int manage_sess_receive(manage_session *session,
									void **rxbuf, size_t *rxbuf_len, struct timeval *tvp);


static void
_pthread_mutex_init(void) {	
	manage_token_log(LOG_TOKEN_INIT, "manage pthread mutex init\n");
	
	pthread_mutex_init(&session_mutex, NULL); 
	pthread_mutex_init(&task_mutex, NULL); 
	pthread_cond_init(&task_cond, NULL);

	return ;
}

static void
_pthread_mutex_destroy(void) {
	manage_token_log(LOG_TOKEN_INIT, "manage pthread mutex destroy\n");
	
	pthread_mutex_destroy(&session_mutex); 
	pthread_mutex_destroy(&task_mutex);
	pthread_cond_destroy(&task_cond);

	return ;
}


static u_int _init_manage_init_done = 0;
static void
_init_manage(void) {
	struct timeval  tv;

	if(_init_manage_init_done) {
		return ;
	}
	_init_manage_init_done = 1;

	_pthread_mutex_init();
	
	manage_tdomain_init();

	gettimeofday(&tv, (struct timezone *) 0);

	manage_token_log(LOG_TOKEN_INIT, "manage pthread mutex destroy\n");

	return ;
}

static inline manage_task *
_task_create(void *sessp, void *opaque, u_long olength) {

	struct session_list *slp = (struct session_list *)sessp;
	manage_task *task = (manage_task *)calloc(1, sizeof(manage_task));
	if(NULL == task) {
		manage_log(LOG_ERR, "_task_create: can't malloc space for task\n");
		return NULL;
	}

	task->task_id = ++global_taskid;
	task->transport_data = opaque;
	task->transport_data_length = olength;
	task->sessp = slp;
	
	gettimeofday(&task->time, NULL);
		
	return task;
}

static inline void
_task_add(manage_task *task) {

	int queue_empty = 0;
	
	pthread_mutex_lock(&task_mutex);	

	if(NULL == task_tail) {
		task_queue = task_tail = task;
		queue_empty = 1;
	} else {
		task_tail->next = task;
		task_tail = task;
	}

	pthread_mutex_unlock(&task_mutex);	
	if(queue_empty) {
		pthread_cond_broadcast(&task_cond);
	}	
	
	return ;
}

static inline manage_task *
_task_get(void){
	manage_token_log(LOG_TOKEN_TASK, "enter _task_get\n");
	
	manage_task *task = NULL;	
	
	pthread_mutex_lock(&task_mutex);	
	while(NULL == task_queue) {
		pthread_cond_wait(&task_cond, &task_mutex) ;		/** wait for task cond **/
	} 

	task = task_queue;
	task_queue = task_queue->next;
	if(NULL == task_queue) {
		task_tail = NULL;
	}
	pthread_mutex_unlock(&task_mutex);	

	manage_token_log(LOG_TOKEN_TASK, "exit _task_get, task (%p) id (%d)\n", task, task->task_id);
	return task;
}

static inline int
_task_parse(manage_task *task, u_char *data, size_t length) {
	manage_token_log(LOG_TOKEN_TASK, "enter _task_parse, task (%p) id (%d)\n", task, task->task_id);

	if(length < sizeof(manage_message)) {
		task->message.m_errno = (u_char)MANAGEERR_BAD_PARSE;
		return MANAGEERR_BAD_PARSE;
	}

	memcpy(&(task->message), data, sizeof(manage_message));

	if(task->message.method_id >= task_method_size
		|| NULL == (task->method = task_method_array[task->message.method_id].method)) {
		manage_log(LOG_WARNING, "_task_parse: task (%d), bad method id(%d) input\n", 
									task->task_id, task->message.method_id);
		task->message.m_errno = (u_char)MANAGEERR_BAD_INPUT;
		return MANAGEERR_BAD_INPUT;
	}

	task->para_data_length = task->message.data_length;
	task->para_data = (void *)calloc(1, task->para_data_length); /*  core dump for acsample */
	if(NULL == task->para_data) {
		manage_log(LOG_ERR, "_task_parse: task (%d), can't malloc space for task para data\n", task->task_id);
		task->message.m_errno = (u_char)MANAGEERR_MALLOC_FAIL;
		return MANAGEERR_MALLOC_FAIL;
	}
	memcpy(task->para_data, data + sizeof(manage_message), task->para_data_length);

	manage_token_log(LOG_TOKEN_TASK, "exit _task_parse, task (%p) id (%d)\n", task, task->task_id);
	return MANAGEERR_SUCCESS;
}


static inline int
_task_post(manage_task *task) {
	if(NULL == task){
		manage_log(LOG_WARNING, "_task_process: task is NULL\n");
		return MANAGEERR_BAD_INPUT;
	}
	manage_token_log(LOG_TOKEN_TASK, "enter _task_post, task (%p) id (%d)\n", task, task->task_id);
	
	if(NULL != task->sessp) {
		struct session_list *slp = (struct session_list *)(task->sessp);
		manage_transport *transport = slp->transport;
		size_t length = 0;
		u_char *buf = NULL;
		
		if(task->message.m_errno || NULL == task->method_data || 0 == task->message.data_length) {
			task->message.data_length = 0;
			length = sizeof(manage_message);
			buf = (u_char *)&(task->message);
		} else {
			buf = (u_char *)calloc(1, sizeof(manage_message) + task->message.data_length);
			if(NULL == buf) {
				task->message.m_errno = (u_char)MANAGEERR_MALLOC_FAIL;
				task->message.data_length = 0;
				length = sizeof(manage_message);
				buf = (u_char *)&(task->message);
			} else {
				length = sizeof(manage_message) + task->message.data_length;
				memset(buf, 0 ,sizeof(length));
				memcpy(buf, &(task->message), sizeof(manage_message));
				memcpy(buf + sizeof(manage_message), task->method_data, task->method_data_length);
			}
		}

		if(transport->f_send(transport, buf, length, task->transport_data, task->transport_data_length) < 0) {
			manage_log(LOG_WARNING, "_task_post: task (%d), task post fail\n", task->task_id);
			if(task->message.data_length) {
				MANAGE_FREE(buf);
			}
			return MANAGEERR_OPEN_FAIL;
		}

		if(task->message.data_length) {
			MANAGE_FREE(buf);
		}
		
		manage_token_log(LOG_TOKEN_TASK, "exit _task_post, task (%p) id (%d)\n", task, task->task_id);
		return MANAGEERR_SUCCESS;
	}

	manage_log(LOG_WARNING, "_task_post: bad para input\n");
	return MANAGEERR_BAD_INPUT;
}


static inline int
_task_process(manage_task *task) {
	
	if(NULL == task) {
		manage_log(LOG_WARNING, "_task_process: task is NULL\n");
		return MANAGEERR_BAD_INPUT;
	}
	manage_token_log(LOG_TOKEN_TASK, "enter _task_process, task (%p) id (%d)\n", task, task->task_id);

	switch(task->message.m_type) {
		case MANAGE_MESSAGE_SIGNAL:
			manage_token_log(LOG_TOKEN_TASK, "_task_process: task %lu process message\n", task->task_id);
			if(!task->method){
				manage_token_log(LOG_TOKEN_TASK, "_task_process: task->method is NULL\n");
				break;
			}
			task->message.m_errno = (u_char)task->method(task->para_data, task->para_data_length, 
										&(task->transport_data), &(task->transport_data_length));

			break;

		case MANAGE_MESSAGE_METHOD_CALL:
			manage_token_log(LOG_TOKEN_TASK, "_task_process: task %lu process method call\n", task->task_id);
			
			task->message.m_errno = (u_char)task->method(task->para_data, task->para_data_length,
														&(task->method_data), &(task->method_data_length));
			task->message.data_length = task->method_data_length;
			task->message.m_type = MANAGE_MESSAGE_METHOD_CALL_REPLY;

			_task_post(task);
			break;

		case MANAGE_MESSAGE_METHOD_CALL_REPLY:
			manage_token_log(LOG_TOKEN_TASK, "_task_process: task %lu process method call reply\n", task->task_id);
			break;

		default:
			manage_log(LOG_WARNING, "_task_process: task (%d), unknown task type(%d)\n", 
										task->task_id, task->message.m_type);
			return MANAGEERR_BAD_TYPE;
		
	}
	
	manage_token_log(LOG_TOKEN_TASK, "exit _task_process, task (%p) id (%d)\n", task, task->task_id);
	return MANAGEERR_SUCCESS;
}

static inline void
_task_free(manage_task **task) {
	if(NULL == task || NULL == *task) {
		return ;
	}

	MANAGE_FREE((*task)->transport_data);
	MANAGE_FREE((*task)->para_data);
	MANAGE_FREE((*task)->method_data);
	MANAGE_FREE(*task);
	
	return ;
}

static int
_task_netlink_parse(manage_task *task, u_char *data, size_t length) {
	task->method = task->sessp->session->Method_s;
	task->para_data_length = length;
	task->para_data = (void *)calloc(1, task->para_data_length);
	if(NULL == task->para_data) {
		manage_log(LOG_ERR, "_task_netlink_parse: task (%d), can't malloc space for task para data\n", task->task_id);
		task->message.m_errno = (u_char)MANAGEERR_MALLOC_FAIL;
		return MANAGEERR_MALLOC_FAIL;
	}
	memcpy(task->para_data, data, task->para_data_length);
	return MANAGEERR_SUCCESS;
}


#if 0
static void
_sess_test(struct session_list *slp) {
	manage_transport *transport = slp->transport;
	manage_tipc_addr_group addr_group;
	
	memset(&addr_group, 0, sizeof(manage_tipc_addr_group));
	addr_group.dest.type = MANAGE_TIPC_TYPE;	/*tipc*/
	addr_group.dest.instance = 0x1000 + 9;		/*slot 9*/

	memcpy(&(addr_group.sour), transport->addr, sizeof(manage_tipc_addr));
	
		
	char *buf = (char *)malloc(512);
	if(NULL == buf){
		manage_log(LOG_ERR, "_sess_test: malloc buf fail\n");
		return ;		
	}

	memset(buf, 0, 512);
	manage_message *message = (manage_message *)buf;
	message->m_type = MANAGE_MESSAGE_METHOD_CALL;
	manage_log(LOG_DEBUG, "_sess_test: before transport send\n");
	
	if(transport->f_send(transport, buf, 512, &addr_group, sizeof(manage_tipc_addr_group)) < 0) {
		manage_log(LOG_WARNING, "_sess_test: send test tipc pkt fail\n");
	}
	
	MANAGE_FREE(buf);

	return ;
}
#endif

static void *
_sess_pointer(manage_session *session) {
	struct session_list *slp;
	
	//pthread_mutex_lock(&session_mutex);	
	for(slp = Sessions; slp; slp = slp->next) {
		if(slp->session == session) {
			return slp;
		}
	}
	//pthread_mutex_unlock(&session_mutex);	

	return NULL;
}

static struct session_list *
_sess_copy(manage_session * in_session) {
	struct session_list *slp = NULL;
	struct manage_internal_session *isp = NULL;

	in_session->s_manage_errno = 0;
	in_session->s_errno = 0;

	/*
	 * Copy session structure and link into list 
	 */
	slp = (struct session_list *) calloc(1, sizeof(struct session_list));
	if(NULL == slp) {
		in_session->s_manage_errno = MANAGEERR_MALLOC_FAIL;
		return NULL;
	}

	slp->transport = NULL;

	isp = (struct manage_internal_session *)calloc(1, sizeof(struct manage_internal_session));
	if(NULL == isp) {
		manage_sess_close(slp);
		in_session->s_manage_errno = MANAGEERR_MALLOC_FAIL;
		return NULL;
	}

	slp->internal = isp;
	slp->session = (manage_session *)calloc(1, sizeof(manage_session));
	if(NULL == slp->session) {
		manage_sess_close(slp);
		in_session->s_manage_errno = MANAGEERR_MALLOC_FAIL;
		return NULL;
	}
	
	slp->session->sessid = ++global_sessid;
	slp->session->flags = in_session->flags;
	slp->session->callback = in_session->callback;
	slp->session->Method_s = in_session->Method_s;
	
	if(in_session->local && in_session->local_len) {
		slp->session->local = calloc(1, in_session->local_len);
		if(NULL == slp->session->local) {
			manage_sess_close(slp);
			in_session->s_manage_errno = MANAGEERR_MALLOC_FAIL;
			return NULL;
		}
		memcpy(slp->session->local, in_session->local, in_session->local_len);
	}
	
	if(in_session->remote && in_session->remote_len) {
		slp->session->remote = calloc(1, in_session->remote_len);
		if(NULL == slp->session->remote) {
			manage_sess_close(slp);
			in_session->s_manage_errno = MANAGEERR_MALLOC_FAIL;
			return NULL;
		}
		memcpy(slp->session->remote, in_session->remote, in_session->remote_len);
	}
	
	return slp;
}


static void *
_sess_add(manage_session *in_session, 
				manage_transport *transport, 
				struct manage_internal_session *interval) {
	struct session_list *slp = NULL;
	
	_init_manage();

	if(NULL == transport) {
		manage_log(LOG_WARNING, "_sess_add: input transport is NULL\n");
		return NULL;
	}
	
	if(NULL == in_session) {
		manage_log(LOG_WARNING, "_sess_add: input in_session is NULL\n");
		transport->f_close(transport);
		manage_transport_free(transport);
		return NULL;
	}

	if(NULL == (slp = _sess_copy(in_session))) {
		manage_log(LOG_ERR, "_sess_add: _sess_copy in_session fail\n");
		transport->f_close(transport);
		manage_transport_free(transport);
		return NULL;
	}
	
	slp->transport = transport;
	
	if(interval) {
		slp->internal->hook_pre = interval->hook_pre;
		slp->internal->hook_task_create = interval->hook_task_create;
		slp->internal->hook_task_parse = interval->hook_task_parse;
	}
	
	return (void *)slp;
}

static int
_sess_process_packet(void *sessp,
							u_char *packetptr, size_t length,
							void *opaque, size_t olength) {
	int op_ret;
	manage_task	*task = NULL;
	struct session_list *slp = (struct session_list *)sessp;
	
	struct manage_internal_session *isp = slp ? slp->internal : NULL;
	
	manage_token_log(LOG_TOKEN_SESSION, "enter _sess_process_packet\n");

	if(!slp || !isp) {
		manage_log(LOG_ERR, "_sess_process_packet: slp or isp is NULL\n");
		op_ret = MANAGEERR_BAD_INPUT;
		goto PROCESS_ERROR;
	}


	if(isp->hook_pre) {
		if(isp->hook_pre(slp, packetptr, length, opaque, olength)) {
			manage_log(LOG_ERR, "sess_process_packet: pre fail\n");
			op_ret = MANAGEERR_BAD_PRE;
			goto PROCESS_ERROR;
		}
	}

	
	if(isp->hook_task_create) {
		task = isp->hook_task_create(slp, opaque, olength);
	} else {
		task = _task_create(slp, opaque, olength);
	}

	if(NULL == task) {
		manage_log(LOG_ERR, "sess_process_packet: task create fail\n");
		op_ret = MANAGEERR_MALLOC_FAIL;
		goto PROCESS_ERROR;
	}


	if(isp->hook_task_parse) {
		op_ret = isp->hook_task_parse(task, packetptr, length);
	} else {
		op_ret = _task_parse(task, packetptr, length);
	}

	if(MANAGEERR_SUCCESS != op_ret) {
		manage_log(LOG_ERR, "sess_process_packet: task prase fail\n");
		_task_free(&task);
		return MANAGEERR_BAD_PARSE;
	}


	_task_add(task);
	
	return MANAGEERR_SUCCESS;


PROCESS_ERROR:
	MANAGE_FREE(opaque);
	return op_ret;
}

static void *
manage_sess_open(manage_session *in_session) {
	manage_transport *transport = NULL;
	struct manage_internal_session *interval = NULL;

	in_session->s_manage_errno = 0;
	in_session->s_errno = 0;

	_init_manage();

	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_open: in_session->flags (%x)\n", in_session->flags);

	if(in_session->flags & MANAGE_FLAGS_TIPC_SOCKET) {
		manage_token_log(LOG_TOKEN_SESSION, "manage_sess_open: create tipc transport\n");
		transport = manage_transport_open(MANAGE_TIPC_DOMAIN, in_session->flags, 
										in_session->local, in_session->local_len);
	}else if(in_session->flags & MANAGE_FLAGS_NETLINK_SEM_SOCKET){
		manage_token_log(LOG_TOKEN_SESSION, "manage_sess_open: create netlink transport\n");
		transport = manage_transport_open(MANAGE_NETLINK_DOMAIN, in_session->flags, 
										in_session->local, in_session->local_len);

		interval = (struct manage_internal_session *)malloc(sizeof(struct manage_internal_session));
		memset(interval, 0, sizeof(struct manage_internal_session));
		interval->hook_task_parse = _task_netlink_parse;
	}

	if(transport == NULL) {
		manage_log(LOG_ERR, "_sess_open: can not interpret local\n");
		in_session->s_manage_errno = MANAGEERR_BAD_ADDRESS;
		in_session->s_errno = errno;
		return NULL;
	}
	
	return _sess_add(in_session, transport, interval);
}

static int
manage_sess_read(void *sessp, fd_set *fdset) {
	struct session_list *slp = (struct session_list *)sessp;
	manage_transport *transport = slp ? slp->transport : NULL;
	static u_char *rxbuf = NULL;
	void *opaque = NULL;
	size_t rxbuf_len = 65536, olength = 0;
	int length = 0, rc = 0;

	manage_token_log(LOG_TOKEN_SESSION, "enter manage_sess_read\n");	
	
	if(!slp || !transport) {
		manage_log(LOG_WARNING, "manage_sess_read : read fail: closing...\n");
		return 0;
	}
	
	/*  */ 
	if(transport->sock < 0) { 
		manage_log(LOG_WARNING, "manage_sess_read: transport->sock got negative fd value %d\n", transport->sock);
		return 0; 
	}

	if(!fdset || !FD_ISSET(transport->sock, fdset)) {
		manage_log(LOG_WARNING, "manage_sess_read: not reading %d (fdset %p set %d)\n",
				            transport->sock, fdset, fdset ? FD_ISSET(transport->sock, fdset) : -9);
		return 0;
	}

	if(rxbuf) {
		manage_token_log(LOG_TOKEN_SESSION, "manage_sess_read: the buf is already malloc\n");
	} else {
		if(NULL == (rxbuf = (u_char *) malloc(rxbuf_len))) {
			manage_log(LOG_ERR, "manage_sess_read: can't malloc %u bytes for rxbuf\n", rxbuf_len);
			return 0;
		}
	}

    	length = transport->f_recv(transport, rxbuf, rxbuf_len, &opaque, &olength);
	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_read: transport recv %d\n", length);	
	if(length <=  0) {
		MANAGE_FREE(opaque);
		return -1;
	} 

	rc = _sess_process_packet(sessp, rxbuf, length, opaque, olength);
	return rc;
}

static void
manage_sess_free(manage_session *s) {
	if(NULL == s) {
		manage_log(LOG_WARNING, "manage_sess_free: input *s of manage_session point is NULL\n");
		return ;
	}

	MANAGE_FREE(s->local);
	MANAGE_FREE(s->remote);
	MANAGE_FREE(s);
	return ;
}
static void
manage_inter_free(struct manage_internal_session *s) {
	if(NULL == s) {
		manage_log(LOG_WARNING, "manage_inter_free: input *s of manage_session point is NULL\n");
		return ;
	}
	MANAGE_FREE(s)
	return ;
}


static int
manage_sess_close(void *sessp) {
	struct session_list *slp = (struct session_list *) sessp;
	//struct manage_internal_session *isp = NULL;

	if(NULL == slp) {
		manage_log(LOG_WARNING, "manage_sess_close: input *sessp is NULL\n");
		return MANAGEERR_BAD_INPUT;
	}

	//isp = slp->internal;
	//slp->internal = NULL;

	if(slp->transport) {
		slp->transport->f_close(slp->transport);
		manage_transport_free(slp->transport);
		slp->transport = NULL;
	}

	if(slp->session) {
		manage_sess_free(slp->session);
		slp->session = NULL;
	}

	if(slp->internal){
		manage_inter_free(slp->internal);
		slp->internal = NULL;
	}
	
	MANAGE_FREE(slp);
	
	return MANAGEERR_SUCCESS;
}

static int
manage_sess_send(manage_session *session, 
							void *packetptr, size_t length, 
							void *transport_data, size_t transport_data_length) {
	struct session_list *slp = _sess_pointer(session);
	manage_transport *transport = slp ? slp->transport : NULL;
	if(NULL == transport || transport->sock < 0) {
		return -1;
	}

	return transport->f_send(transport, packetptr, length, transport_data, transport_data_length);
}

static int
manage_sess_receive(manage_session *session,
							void **rxbuf, size_t *rxbuf_len, struct timeval *tvp) {
	int numfds, count;
	fd_set readfds;
	
	FD_ZERO(&readfds);
	*rxbuf = NULL;
	*rxbuf_len = 0;

	struct session_list *slp = _sess_pointer(session);
	manage_transport *transport = slp ? slp->transport : NULL;
	if(NULL == transport || transport->sock < 0) {
		return -1;
	}	

	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_receive: tvp->tv_sec = %d, tvp->tv_usec = %d\n",
											tvp ? (int)tvp->tv_sec : 0, tvp ? (int)tvp->tv_usec : 0);

	numfds = transport->sock + 1;
	FD_SET(transport->sock, &readfds);
	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_receive: numfds %d\n", numfds);
	
	count = select(numfds, &readfds, NULL, NULL, tvp);
	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_receive, count = %d\n", count);
	if(count > 0) {
		if(FD_ISSET(transport->sock, &readfds)) {
			void *buf;
			size_t buf_len = 65536;
			int length = 0;

			if(NULL == (buf = malloc(buf_len))) {
				manage_log(LOG_ERR, "manage_sess_receive: can't malloc %u bytes for rxbuf\n", buf_len);
				return -1;
			}
			
		    	length = transport->f_recv(transport, buf, buf_len, NULL, NULL);
			if(length > 0) {
				*rxbuf = buf;
				*rxbuf_len = buf_len;
				return 0;
			} 

			MANAGE_FREE(*rxbuf);				
		}
	} 
	
	manage_log(LOG_WARNING, "manage_sess_receive: select fail\n");
	return -1;
}

manage_session *
manage_open(manage_session *session) {
	struct session_list *slp = NULL;

	if(NULL == session) {
		manage_log(LOG_WARNING, "manage_open: input session is NULL\n");
		return NULL;
	}
	
	slp = (struct session_list *)manage_sess_open(session);
	if(NULL == slp) {
		manage_log(LOG_ERR, "manage_open: manage_sess_open fail\n");
		return NULL;
	}
	
	pthread_mutex_lock(&session_mutex);	
	slp->next = Sessions;
	Sessions = slp;
	pthread_mutex_unlock(&session_mutex);	
	
	return (slp->session);
}


void
manage_read(fd_set *fdset) {
	struct session_list *slp;
		
	pthread_mutex_lock(&session_mutex);	
	for(slp = Sessions; slp; slp = slp->next) {
		manage_sess_read(slp, fdset);
	}
	pthread_mutex_unlock(&session_mutex);	

	return ;
}

void
manage_close(manage_session *session) {
	struct session_list *slp;

	if(NULL == session) {
		manage_log(LOG_WARNING, "manage_close: input session is NULL\n");
		return ;
	}

	pthread_mutex_lock(&session_mutex);	
	for(slp = Sessions; slp; slp = slp->next) {
		if(slp->session->sessid == session->sessid) {
			manage_sess_close(slp);
		}
	}
	pthread_mutex_unlock(&session_mutex);	
	
	return ;
}

void
manage_close_all(void) {
	struct session_list *slp;
	
	pthread_mutex_lock(&session_mutex);
	for(slp = Sessions; slp; slp = slp->next) {
		manage_sess_close(slp);
	}
	pthread_mutex_unlock(&session_mutex);	
	
	_pthread_mutex_destroy();

	return ;
}

int
manage_select_info(int *numfds, fd_set *fdset,
					struct timeval *timeout, int *block) {
	struct session_list *slp;
	
	*numfds = 0;
	*block = 1;

	FD_ZERO(fdset);

	pthread_mutex_lock(&session_mutex);
	for(slp = Sessions; slp; slp = slp->next) {

	sess_select:
		
		if(NULL == slp->transport) {
			/*
	 		 * Close in progress -- skip this one.  
	 		 */
	 		manage_token_log(LOG_TOKEN_SESSION, "manage_sess_select_info:  skip\n");
			continue;
		}

		if(slp->transport->sock < 0) {
			/*
			 * This session was marked for deletion.  
			 */
	 		manage_token_log(LOG_TOKEN_SESSION, "manage_sess_select_info:  delete session\n");
			
			struct session_list *next = slp->next;
			manage_sess_close(slp);
			slp = next;

			if(slp) {
				goto sess_select;
			}
			
			break;
		}
		
		if((slp->transport->sock + 1) > *numfds) {
		    *numfds = (slp->transport->sock + 1);
		}
	 	manage_token_log(LOG_TOKEN_SESSION, "manage_sess_select_info:  fd_set %d, numfds = %d\n", 
												slp->transport->sock, *numfds);
		FD_SET(slp->transport->sock, fdset);
	}	
	pthread_mutex_unlock(&session_mutex);	

    return MANAGEERR_SUCCESS;
}


void
manage_method_regist(manage_method *array, u_long size) {
	if(NULL == array || 0 == size) {
		return ;
	}

	task_method_array = array;
	task_method_size = size;

	return ;
}

void 
manage_task_process(void) {	
	while(1) {
		manage_task *task = _task_get();
		_task_process(task);
		_task_free(&task);
	}

	return ;
}

manage_message *
manage_message_new(u_short method_id, void *data, size_t data_length) {
	if(NULL == data && data_length) {
		manage_log(LOG_WARNING, "manage_message_new: input data(%p) data_length(%d)\n",
									data, data_length);
		return NULL;
	}
	
	manage_message *message = (void *)calloc(1, sizeof(manage_message) + data_length);
	if(NULL == message) {
		manage_log(LOG_WARNING, "manage_message_new: malloc message fail\n");
		return NULL;
	}

	message->method_id = method_id;
	message->data_length = data_length;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	message->m_check  = (u_int)tv.tv_usec;

	if(data) {
		memcpy((void *)message + sizeof(manage_message), data, data_length);
	}	

	return message;
}


int
manage_message_send(manage_session *session, manage_message *message,
									void *transport_data, size_t transport_data_length) {

	if(!session || !message || !transport_data) {
		manage_log(LOG_WARNING, "manage_message_send: input session(%p) message(%p) transport_data(%p)\n",
									session, message, transport_data);
		return -1;
	}

	size_t m_size = sizeof(manage_message) + message->data_length;
	message->m_type = (u_char)MANAGE_MESSAGE_SIGNAL;
	
	return manage_sess_send(session, message, m_size, transport_data, transport_data_length);	
}

static inline int
_message_compare(manage_message *reply, manage_message *query) {
	if(MANAGE_MESSAGE_METHOD_CALL_REPLY != reply->m_type) {
		return -1;
	}
	
	if((reply->method_id != query->method_id) 
		|| (reply->m_check != query->m_check)) {
		return -1;
	}

	return 0;
}

manage_message *
manage_message_send_with_reply_and_block(manage_session *session, manage_message *message, int timeout_milliseconds, 
															void *transport_data, size_t transport_data_length) {

	if(!session || !message || !transport_data) {
		manage_log(LOG_WARNING, "manage_message_send_with_reply_and_block: input session(%p) message(%p) transport_data(%p)\n",
									session, message, transport_data);
		return NULL;
	}

	size_t m_size = sizeof(manage_message) + message->data_length;
	message->m_type = (u_char)MANAGE_MESSAGE_METHOD_CALL;
	
	if(manage_sess_send(session, message, m_size, transport_data, transport_data_length) < 0) {
		manage_log(LOG_WARNING, "manage_message_send_with_reply_and_block: send message fail\n");
		return NULL;
	}

	void *rx_buf = NULL;
	size_t buf_size = 0;
	struct timeval wait_time, *tvp;

	if(timeout_milliseconds < 0) {
		tvp = &wait_time;
		tvp->tv_sec = 3;	/*default time_out*/
		tvp->tv_usec = 0;
	} else if(timeout_milliseconds > 0) {
		tvp = &wait_time;
		tvp->tv_sec = timeout_milliseconds / 1000;
		tvp->tv_usec = (timeout_milliseconds - (1000 * tvp->tv_sec)) * 1000;
	} else {
		tvp = NULL;
	}

rereceive:	
	if(manage_sess_receive(session, &rx_buf, &buf_size, tvp)) {
		return NULL;
	}
	
	if(NULL == rx_buf || buf_size < sizeof(manage_message) 
		|| _message_compare(rx_buf, message)) {
		manage_log(LOG_WARNING, "manage_message_send_with_reply_and_block: recv buf is error\n");
		MANAGE_FREE(rx_buf);
		goto rereceive;
	}
	
	return rx_buf;	
}


