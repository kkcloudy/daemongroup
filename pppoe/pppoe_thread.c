#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "mem_cache.h"
#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_util.h"

#include "pppoe_thread.h"

#define THREAD_TIMER_NUM			10
#define THREAD_EXTIMER_NUM			3

#define THREAD_CACHE_BLK_ITEMNUM	512
#define THREAD_CACHE_MAX_BLKNUM		0		/* blk num not limit */
#define THREAD_CACHE_EMPTY_BLKNUM	6
#define THREAD_CACHE_NAME			"pppoe thread cache"

#define THREAD_FUNCNAME_LEN			64

/* Thread state. */
typedef enum {
	THREAD_RUN,
	THREAD_PAUSE,
} ThreadState;

struct thread_timer {
	struct list_head array[THREAD_TIMER_NUM];
	struct list_head exarray[THREAD_EXTIMER_NUM];
};

/* Thread itself. */
struct thread_struct {
	ThreadState state;			/* thread state */
	ThreadType type;			/* thread type */
	struct list_head next;		/* previous/next pointer of the thread */
	thread_master_t *master;	/* pointer to the struct thread_master. */
	ThreadRunFunc func;			/* event function */
	void *arg;					/* event argument */
	union {
		int val;				/* second argument of the event. */
		int fd;					/* file descriptor in case of read/write. */
		long sands;				/* rest of time sands value. */
	} u;						/* Indepth usage info. */
	char funcname[THREAD_FUNCNAME_LEN];	
};

/* Master of the theads. */
struct thread_master {
	int maxfd;
	long time_now;
	mem_cache_t *thread_cache;	
	
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;

	struct list_head read;
	struct list_head write;
	struct list_head event;
	struct thread_timer timer;
};

static char *threadTypeName[] = {
	"Thread Read",
	"Thread Write",
	"Thread Timer",
	"Thread Event",
};	

static inline void
thread_set_maxfd(int fd, thread_master_t *m){
	if (m->maxfd < fd)
		m->maxfd = fd;
}

static inline void
thread_reset_maxfd(thread_master_t *m) {
	thread_struct_t *pos;
	
	list_for_each_entry(pos, &m->read, next) {
		thread_set_maxfd(pos->u.fd, m);
	}
	
	list_for_each_entry(pos, &m->write, next) {
		thread_set_maxfd(pos->u.fd, m);
	}	
}

static inline thread_struct_t *
thread_create(thread_master_t *m, ThreadType type,
				ThreadRunFunc func, void *arg, 
				const char *funcname) {
	thread_struct_t *thread
		= mem_cache_alloc(m->thread_cache);
	if (unlikely(!thread))
		return NULL;
	
	memset(thread, 0, sizeof(thread_struct_t));
	thread->type = type;
	thread->func = func;
	thread->arg = arg;
	thread->master = m;
	strncpy(thread->funcname, funcname, sizeof(thread->funcname) - 1);

	pppoe_token_log(TOKEN_THREAD, "create %s %s, arg: %p\n", 
					threadTypeName[thread->type], funcname, arg);
	return thread;
}

static inline void
thread_destroy(thread_master_t *m, thread_struct_t *thread) {
	pppoe_token_log(TOKEN_THREAD, "%s %s destroy ......\n", 
					threadTypeName[thread->type], thread->funcname);

	return mem_cache_free(m->thread_cache, thread);
}


static inline void
thread_run(thread_struct_t *thread) {
	pppoe_token_log(TOKEN_THREAD, "%s %s run ......\n", 
					threadTypeName[thread->type], thread->funcname);
	
	if (likely(thread->func)) {
		thread->func(thread);
	}
}

static inline void
thread_process_fd(struct list_head *head, fd_set *fdset) {
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, head) {
		thread_struct_t *thread
			= list_entry(pos, thread_struct_t, next);
		if (FD_ISSET(thread->u.fd, fdset)) {
			pppoe_token_log(TOKEN_THREAD, "%s %s fd:%d is set\n",
									threadTypeName[thread->type], 
									thread->funcname, thread->u.fd);
			thread_run(thread);
		}
	}
}

int
thread_start(thread_struct_t *thread) {
	thread_master_t *master;

	if (unlikely(!thread))
		return PPPOEERR_EINVAL;

	if (unlikely(THREAD_PAUSE != thread->state))
		return PPPOEERR_ESTATE;

	master = thread->master;
	switch (thread->type) {
	case THREAD_READ:
		FD_SET(thread->u.fd, &master->readfds);
		thread_set_maxfd(thread->u.fd, master);
		list_add(&thread->next, &master->read);
		thread->state = THREAD_RUN;		
		break;
		
	case THREAD_WRITE:
		FD_SET(thread->u.fd, &master->writefds);
		thread_set_maxfd(thread->u.fd, master);
		list_add(&thread->next, &master->write);
		thread->state = THREAD_RUN;		
		break;
	
	default:
		return PPPOEERR_EINVAL;
	}

	pppoe_token_log(TOKEN_THREAD, "%s %s start\n", 
					threadTypeName[thread->type], thread->funcname);
	return PPPOEERR_SUCCESS;
}

int
thread_pause(thread_struct_t *thread) {
	thread_master_t *master;

	if (unlikely(!thread))
		return PPPOEERR_EINVAL;
	
	if (unlikely(THREAD_RUN != thread->state))
		return PPPOEERR_ESTATE;

	master = thread->master;
	switch (thread->type) {
	case THREAD_READ:
		FD_CLR(thread->u.fd, &master->readfds);
		if (thread->u.fd == master->maxfd) {
			thread_reset_maxfd(master);
		}	
		list_del(&thread->next);
		thread->state = THREAD_PAUSE;
		break;
		
	case THREAD_WRITE:
		FD_CLR(thread->u.fd, &master->writefds);
		if (thread->u.fd == master->maxfd) {
			thread_reset_maxfd(master);
		}
		list_del(&thread->next);
		thread->state = THREAD_PAUSE;		
		break;
			
	default:
		return PPPOEERR_EINVAL;
	}

	pppoe_token_log(TOKEN_THREAD, "%s %s pause\n", 
					threadTypeName[thread->type], thread->funcname);
	return PPPOEERR_SUCCESS;
}


void
thread_cancel(thread_struct_t *thread) {
	thread_master_t *master;
	
	if (unlikely(!thread))
		return;

	master = thread->master;	
	switch (thread->type) {
		case THREAD_READ:
			if (THREAD_RUN == thread->state) {
				FD_CLR(thread->u.fd, &master->readfds);
				if (thread->u.fd == master->maxfd) {
					thread_reset_maxfd(master);
				}	
				list_del(&thread->next);
			}		
			break;
			
		case THREAD_WRITE:
			if (THREAD_RUN == thread->state) {
				FD_CLR(thread->u.fd, &master->writefds);
				if (thread->u.fd == master->maxfd) {
					thread_reset_maxfd(master);
				}
				list_del(&thread->next);
			}	
			break;
			
		case THREAD_EVENT:
			if (THREAD_RUN == thread->state) {
				list_del(&thread->next);
			}	
			break;
			
		case THREAD_TIMER:
			if (THREAD_RUN == thread->state) {
				list_del(&thread->next);
			}	
			break;
	}
	
	pppoe_token_log(TOKEN_THREAD, "%s %s cannel\n", 
					threadTypeName[thread->type], thread->funcname);
	
	thread_destroy(thread->master, thread);
}

thread_struct_t *
thread_add_read_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int fd, const char *funcname) {
	thread_struct_t *thread;

	if (FD_ISSET(fd, &m->readfds)) {
		pppoe_log(LOG_WARNING, "fd %d is is already in readfds\n", fd);
		return NULL;
	}

	thread = thread_create(m, THREAD_READ, func, arg, funcname);
	if (unlikely(!thread))
		return NULL;
	
	thread->u.fd = fd;
	FD_SET(fd, &m->readfds);
	thread_set_maxfd(fd, m);
	list_add(&thread->next, &m->read);
	
	thread->state = THREAD_RUN;
	pppoe_token_log(TOKEN_THREAD, "Thread Read %s add, fd %d\n", funcname, fd);
	return thread;
}


thread_struct_t *
thread_add_write_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int fd, const char *funcname) {
	thread_struct_t *thread;

	if (FD_ISSET(fd, &m->writefds)) {
		pppoe_log(LOG_WARNING, "fd %d is is already in writefds\n", fd);
		return NULL;
	}
	
	thread = thread_create(m, THREAD_WRITE, func, arg, funcname);
	if (unlikely(!thread))
		return NULL;

	thread->u.fd = fd;
	FD_SET(fd, &m->writefds);
	thread_set_maxfd(fd, m);
	list_add(&thread->next, &m->write);

	thread->state = THREAD_RUN;
	pppoe_token_log(TOKEN_THREAD, "Thread Write %s add, fd %d\n", funcname, fd);	
	return thread;
}


thread_struct_t *
thread_add_timer_with_funcname(thread_master_t *m, ThreadRunFunc func, 
						void *arg, long timer, uint32 extern_id, const char *funcname) {
	thread_struct_t *thread;
	
	if (unlikely((!extern_id && timer > THREAD_TIMER_NUM) || extern_id > THREAD_TIMER_NUM))
		return NULL;

	thread = thread_create(m, THREAD_TIMER, func, arg, funcname);
	if (unlikely(!thread))
		return NULL;

	thread->u.sands = time_sysup() + timer;

	if (extern_id) {
		list_add_tail(&thread->next, &m->timer.exarray[extern_id - 1]);
	} else {
		list_add_tail(&thread->next, &m->timer.array[timer - 1]);
	}

	thread->state = THREAD_RUN;			
	pppoe_token_log(TOKEN_THREAD, "Thread Timer %s add, timer %ld\n", funcname, timer);
	return thread;
}

int
thread_update_timer(thread_struct_t *thread, long timer, uint32 extern_id) {
	thread_master_t *m = thread->master;

	if (unlikely((!extern_id && timer > THREAD_TIMER_NUM) || extern_id > THREAD_TIMER_NUM))
		return PPPOEERR_EINVAL;
	
	if (unlikely(THREAD_TIMER != thread->type))
		return PPPOEERR_EINVAL;

	if (THREAD_RUN == thread->state)
		list_del(&thread->next);
	
	thread->u.sands = time_sysup() + timer;

	if (extern_id) {
		list_add_tail(&thread->next, &m->timer.exarray[extern_id - 1]);
	} else {
		list_add_tail(&thread->next, &m->timer.array[timer - 1]);
	}

	thread->state = THREAD_RUN;
	pppoe_token_log(TOKEN_THREAD, "Thread Timer %s update, timer %ld\n", thread->funcname, timer);
	return PPPOEERR_SUCCESS;
}

int
thread_pause_timer(thread_struct_t *thread) {
	if (unlikely(THREAD_TIMER != thread->type))
		return PPPOEERR_EINVAL;

	if (likely(THREAD_RUN != thread->state))
		return PPPOEERR_ESTATE;

	list_del(&thread->next);
	
	thread->state = THREAD_PAUSE;
	pppoe_token_log(TOKEN_THREAD, "Thread Timer %s pause\n", thread->funcname);
	return PPPOEERR_SUCCESS;
}

thread_struct_t *
thread_add_event_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int val, const char *funcname) {
	thread_struct_t *thread 
		= thread_create(m, THREAD_EVENT, func, arg, funcname);

	if (unlikely(!thread))
		return NULL;

	thread->u.val = val;
	list_add(&thread->next, &m->event);

	thread->state = THREAD_RUN;			
	pppoe_token_log(TOKEN_THREAD, "Thread Event %s add\n", funcname);
	return thread;
}


static inline void
thread_master_update_time(thread_master_t * m) {
	m->time_now = time_sysup();
}

static inline void
thread_dispatch_event(thread_master_t *m) {
	
}

static inline void
thread_dispatch_timer(thread_master_t *m) {
	struct list_head *pos, *n;
	thread_struct_t *thread;
	int i;

	for (i = 0; i < THREAD_TIMER_NUM; i++) {
		list_for_each_safe(pos, n, &m->timer.array[i]) {
			thread = list_entry(pos, thread_struct_t, next);
			if (time_compare(m->time_now, thread->u.sands) < 0)
				break;

			pppoe_token_log(TOKEN_THREAD, "Thread timer %s, timer %d run .......\n", 
										thread->funcname, i + 1);
			thread_run(thread);
		}	
	}

	for (i = 0; i < THREAD_EXTIMER_NUM; i++) {
		list_for_each_safe(pos, n, &m->timer.exarray[i]) {
			thread = list_entry(pos, thread_struct_t, next);
			if (time_compare(m->time_now, thread->u.sands) < 0)
				break;

			pppoe_token_log(TOKEN_THREAD, "Thread Timer %s, Extern %d run .......\n", 
										thread->funcname, i + 1);
			thread_run(thread);
		}	
	}
}

static void
thread_dispatch_sock(thread_master_t *m, struct timeval *timer_wait) {
	int count;
	fd_set readfds, writefds;

	if (!m->maxfd)
		return;

	readfds = m->readfds;
	writefds = m->writefds;
	
reselect:
	count = select(m->maxfd + 1, &readfds, &writefds, NULL, timer_wait);
	pppoe_token_log(TOKEN_THREAD, "select num = %d\n", count);
	if (count > 0) {
		pppoe_token_log(TOKEN_THREAD, "berfore process read fd\n");
		thread_process_fd(&m->read, &readfds);

		pppoe_token_log(TOKEN_THREAD, "berfore process write fd\n");
		thread_process_fd(&m->write, &writefds);
	} else switch (count) {
	case 0:
		pppoe_token_log(TOKEN_THREAD, "thread select EOF\n");
		break;
			
	case -1:
		if (EINTR == errno)
			goto reselect;

		pppoe_log(LOG_WARNING, "select failed!\n");		
		break;
		
	default:
		pppoe_log(LOG_WARNING, "select failed(unknown reason)!\n");
		break;
	}
}

void
thread_dispatch(thread_master_t *m, struct timeval timer_wait) {
	thread_master_update_time(m);
	thread_dispatch_event(m);
	thread_dispatch_timer(m);
	thread_dispatch_sock(m, &timer_wait);
}

void *
thread_get_arg(thread_struct_t *thread) {
	if (unlikely(!thread))
		return NULL;

	return thread->arg;
}


static inline void
__thread_master_setup(thread_master_t *m) {
	int i;

	INIT_LIST_HEAD(&m->read);
	INIT_LIST_HEAD(&m->write);
	INIT_LIST_HEAD(&m->event);

	for (i = 0; i < THREAD_TIMER_NUM; i++) {
		INIT_LIST_HEAD(&m->timer.array[i]);
	}

	for (i = 0; i < THREAD_EXTIMER_NUM; i++) {
		INIT_LIST_HEAD(&m->timer.exarray[i]);
	}

	FD_ZERO(&m->readfds);
	FD_ZERO(&m->writefds);
	FD_ZERO(&m->exceptfds);
}

static inline void
thread_list_release(thread_master_t * m, struct list_head *head) {
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, head) {
		thread_struct_t *thread
			= list_entry(pos,  thread_struct_t, next);;
		thread_destroy(m, thread);
	}
}

thread_master_t *
thread_master_create(void) {
	thread_master_t *m
		= (thread_master_t *)malloc(sizeof(thread_master_t));
	if (unlikely(!m)) {
		pppoe_log(LOG_ERR, "thread master create: malloc fail\n");
		return NULL;
	}

	memset(m, 0, sizeof (*m));
	m->thread_cache = mem_cache_create(THREAD_CACHE_NAME,
						THREAD_CACHE_MAX_BLKNUM, THREAD_CACHE_EMPTY_BLKNUM,
						sizeof(thread_struct_t), THREAD_CACHE_BLK_ITEMNUM);
	if (unlikely(!m->thread_cache)) {
		pppoe_log(LOG_ERR, "thread master create: cache create fail\n");
		PPPOE_FREE(m);
		return NULL;
	}

	__thread_master_setup(m);
	return m;
}

void
thread_master_destroy(thread_master_t * m) {
	int i;

	if (unlikely(!m))
		return ;

	m->maxfd = 0;
	FD_ZERO(&m->readfds);
	FD_ZERO(&m->writefds);
	FD_ZERO(&m->exceptfds);

	thread_list_release(m, &m->read);
	thread_list_release(m, &m->write);
	thread_list_release(m, &m->event);

	for (i = 0; i < THREAD_TIMER_NUM; i++) {
		thread_list_release(m, &m->timer.array[i]);
	}

	for (i = 0; i < THREAD_EXTIMER_NUM; i++) {
		thread_list_release(m, &m->timer.exarray[i]);
	}

	mem_cache_destroy(&m->thread_cache);
}




