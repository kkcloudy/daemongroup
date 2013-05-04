#ifndef _PPPOE_THREAD_H
#define _PPPOE_THREAD_H

typedef struct thread_struct thread_struct_t;
typedef struct thread_master thread_master_t;

/* Thread types. */
typedef enum {
	THREAD_READ,
	THREAD_WRITE,
	THREAD_TIMER,
	THREAD_EVENT
} ThreadType;

typedef int (*ThreadRunFunc) (thread_struct_t *);

#define THREAD_CANCEL(thread)	{ if (thread) { thread_cancel((thread)); (thread) = NULL; } }

int thread_start(thread_struct_t *thread);
int thread_pause(thread_struct_t *thread);
void thread_cancel(thread_struct_t *thread);
void *thread_get_arg(thread_struct_t *thread);
void thread_dispatch(thread_master_t *m, struct timeval timer_wait);
int thread_update_timer(thread_struct_t *thread, long timer, uint32 extern_id);
int thread_pause_timer(thread_struct_t *thread);

thread_struct_t *thread_add_read(thread_master_t * m,
					ThreadRunFunc func, void *arg, int fd);
thread_struct_t *thread_add_write(thread_master_t * m,
					ThreadRunFunc func, void *arg, int fd);
thread_struct_t *thread_add_timer(thread_master_t * m,
					ThreadRunFunc func, void *arg, 
					long timer, uint32 extern_id);
thread_struct_t *thread_add_event(thread_master_t * m,
					ThreadRunFunc func, void *arg, int val);


#define thread_add_read(m, func, arg, fd)	\
				thread_add_read_with_funcname(m, func, arg, fd, #func)

#define thread_add_write(m, func, arg,fd)	\
				thread_add_write_with_funcname(m, func, arg, fd, #func)

#define thread_add_timer(m, func, arg, timer, extern_id)    \
				thread_add_timer_with_funcname(m, func, arg, timer, extern_id, #func)

#define thread_add_event(m, func, arg, val) \
				thread_add_event_with_funcname(m, func, arg, val, #func)

thread_struct_t *
thread_add_read_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int fd, const char *funcname);
thread_struct_t *
thread_add_write_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int fd, const char *funcname);
thread_struct_t *
thread_add_timer_with_funcname(thread_master_t *m, ThreadRunFunc func, 
						void *arg, long timer, uint32 extern_id, const char *funcname);
thread_struct_t *
thread_add_event_with_funcname(thread_master_t *m, ThreadRunFunc func, 
								void *arg, int val, const char *funcname);


thread_master_t *thread_master_create(void);
void thread_master_destroy(thread_master_t *m);



#endif
