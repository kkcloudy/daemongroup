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
* nm_thread.h
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

/* nm_thread.h */
#ifndef _NM_THREAD_H
#define _NM_THREAD_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct nm_thread nm_thread_t;
typedef struct nm_thread_master nm_thread_master_t;

/* Thread types. */
typedef enum {
	THREAD_READ = 0,
	THREAD_WRITE,
	THREAD_TIMER,
	THREAD_EVENT
} NmThreadType;

typedef int (*NmThreadRunFunc) (nm_thread_t * thread);

#define nm_thread_add_read(m,func,arg,fd) 	\
				nm_funcname_thread_add_read(m,func,arg,fd,#func)

#define nm_thread_add_write(m,func,arg,fd) 	\
				nm_funcname_thread_add_write(m,func,arg,fd,#func)

#define nm_thread_add_timer(m,func,arg,timeout_sec)    \
				nm_funcname_thread_add_timer(m,func,arg,timeout_sec,#func)

#define nm_thread_add_timer_msec(m,func,arg,timeout_msec) \
				nm_funcname_thread_add_timer_msec(m,func,arg,timeout_msec,#func)

#define nm_thread_add_event(m,func,arg,val) \
				nm_funcname_thread_add_event(m,func,arg,val,#func)

nm_thread_master_t *
nm_thread_master_new();

void 
nm_thread_master_free(nm_thread_master_t * m);
int 
nm_thread_master_update_time(nm_thread_master_t * m);
struct timeval *
nm_thread_master_get_timeval(nm_thread_master_t * m);

nm_thread_t *
nm_funcname_thread_add_read(nm_thread_master_t * m,
					   NmThreadRunFunc func,
					   void *arg,
					   int fd,
					   const char *funcname);

nm_thread_t *
nm_funcname_thread_add_write(nm_thread_master_t * m,
					    NmThreadRunFunc func,
					    void *arg,
					    int fd,
					    const char *funcname);

nm_thread_t *
nm_funcname_thread_add_timer(nm_thread_master_t *
					    master,
					    NmThreadRunFunc func,
					    void *arg, long timer,
					    const char *funcname);

nm_thread_t *
nm_funcname_thread_add_timer_msec(nm_thread_master_t *
						 m,
						 NmThreadRunFunc func,
						 void *arg, long timer,
						 const char *funcname);

nm_thread_t *
nm_funcname_thread_add_event(nm_thread_master_t * m,
					    NmThreadRunFunc func,
					    void *arg,
					    int val,
					    const char *funcname);

void *
nm_thread_get_arg(nm_thread_t * thread);

void 
nm_thread_cancel(nm_thread_t * thread);


int
nm_thread_dispatch(nm_thread_master_t * m, struct timeval *timer_wait);


void
nm_thread_log_all_thread( nm_thread_master_t * m );

#if 0

int nm_thread_get_fd(nm_thread_t * thread);

#endif
#ifdef  __cplusplus
}
#endif
#endif				/* _NM_THREAD_H */
