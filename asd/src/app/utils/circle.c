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
* Asdcircle.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include "includes.h"

#include "common.h"
#include "circle.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "ASDCallback_asd.h"
unsigned char FD_CHANGE = 0;/*Qiuchen add this incase:
							in function circle_sock_table_dispatch_test:
							if one handler close a sock,it will break out the loop in case of the table after 
							handle the message from the sock which has already been closed!
							*/
/*

struct circle_sock {
	int sock;
	void *circle_data;
	void *user_data;
	circle_sock_handler handler;
};

struct circle_timeout {
	struct os_time time;
	void *circle_data;
	void *user_data;
	circle_timeout_handler handler;
	struct circle_timeout *next;
};

struct circle_signal {
	int sig;
	void *user_data;
	circle_signal_handler handler;
	int signaled;
};

struct circle_sock_table {
	int count;
	struct circle_sock *table;
	int changed;
};

struct circle_data {
	void *user_data;

	int max_sock;

	struct circle_sock_table readers;
	struct circle_sock_table writers;
	struct circle_sock_table exceptions;

	struct circle_timeout *timeout;

	int signal_count;
	struct circle_signal *signals;
	int signaled;
	int pending_terminate;

	int terminate;
	int reader_table_changed;
};
*/
struct circle_data circle;

int circle_init(void *user_data)
{
	os_memset(&circle, 0, sizeof(circle));
	circle.user_data = user_data;
	return 0;
}


static int circle_sock_table_add_sock(struct circle_sock_table *table,
                                     int sock, circle_sock_handler handler,
                                     void *circle_data, void *user_data)
{
	struct circle_sock *tmp;

	if (table == NULL)
		return -1;

	tmp = (struct circle_sock *)
		os_realloc(table->table,
			   (table->count + 1) * sizeof(struct circle_sock));
	if (tmp == NULL)
		return -1;

	tmp[table->count].sock = sock;
	tmp[table->count].circle_data = circle_data;
	tmp[table->count].user_data = user_data;
	tmp[table->count].handler = handler;
	table->count++;
	table->table = tmp;
	if (sock > circle.max_sock)
		circle.max_sock = sock;
	table->changed = 1;

	return 0;
}


static void circle_sock_table_remove_sock(struct circle_sock_table *table,
                                         int sock)
{
	int i;

	if (table == NULL || table->table == NULL || table->count == 0)
		return;

	for (i = 0; i < table->count; i++) {
		if (table->table[i].sock == sock)
			break;
	}
	if (i == table->count)
		return;
	if (i != table->count - 1) {
		os_memmove(&table->table[i], &table->table[i + 1],
			   (table->count - i - 1) *
			   sizeof(struct circle_sock));
	}
	table->count--;
	table->changed = 1;
}


static void circle_sock_table_set_fds_test(struct circle_sock_table *table,
				     struct pool_all *poll_table)
{
	int i;

	if (table->table == NULL)
		return;
	
	poll_table->count = table->count;
	poll_table->poll = os_zalloc(sizeof(struct pollfd) * table->count);
	poll_table->data = os_zalloc(sizeof(struct pool_data)*table->count);
	
	if((poll_table->poll == NULL)||(poll_table->data == NULL))	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"member of poll_table malloc failed\n"); 	//for test
		return;
	}

	for (i = 0; i < table->count; i++){
		poll_table->poll[i].fd = table->table[i].sock;
		poll_table->poll[i].events = POLLIN|POLLPRI;
		poll_table->data[i].circle_data = table->table[i].circle_data;
		poll_table->data[i].user_data = table->table[i].user_data;
		poll_table->data[i].handler = table->table[i].handler;
	}
}
#if 0
static void circle_sock_table_set_fds(struct circle_sock_table *table,
				     fd_set *fds)
{
	int i;

	FD_ZERO(fds);

	if (table->table == NULL)
		return;

	for (i = 0; i < table->count; i++)
		FD_SET(table->table[i].sock, fds);
}
#endif
static void circle_sock_table_dispatch_test(struct circle_sock_table *table, struct pool_all  *poll_table)
{
	int i;

	if (table == NULL || poll_table==NULL)
		return;

	for (i = 0; i < poll_table->count; i++) {
		FD_CHANGE = 0;
		if(((poll_table->poll[i].revents&POLLIN) == POLLIN)||((poll_table->poll[i].revents&POLLPRI) == POLLPRI)){
			poll_table->data[i].handler(poll_table->poll[i].fd,
										poll_table->data[i].circle_data,
										poll_table->data[i].user_data);
			if(FD_CHANGE == 1){
				asd_printf(ASD_DEFAULT,MSG_NOTICE,"circle_sock_table_dispatch_test FD_CHANGE break!\n");
				break;
			}
		}
		else if((poll_table->poll[i].revents&POLLERR) == POLLERR)
		{
			asd_printf(ASD_DEFAULT,MSG_INFO,"func:%s :poll %d  fd: %d revents = %d\n",__func__,i,poll_table->poll[i].fd,poll_table->poll[i].revents);
			asd_printf(ASD_80211,MSG_INFO,"%s poll_table %s\n",__func__,strerror(errno));
			asd_sock_reinit(poll_table->poll[i].fd,poll_table->data[i].handler,poll_table->data[i].circle_data,poll_table->data[i].user_data);
		}
	}
}
#if 0
static void circle_sock_table_dispatch(struct circle_sock_table *table,
				      fd_set *fds)
{
	int i;

	if (table == NULL || table->table == NULL)
		return;

	table->changed = 0;
	for (i = 0; i < table->count; i++) {
		if (FD_ISSET(table->table[i].sock, fds)) {
			table->table[i].handler(table->table[i].sock,
						table->table[i].circle_data,
						table->table[i].user_data);
			if (table->changed)
				break;
		}
	}
}
#endif

static void circle_sock_table_destroy(struct circle_sock_table *table)
{
	if (table) {
		int i;
		for (i = 0; i < table->count && table->table; i++) {
			asd_printf(ASD_DEFAULT,MSG_INFO,"circle: remaining socket: sock=%d "
			       "circle_data=%p user_data=%p handler=%p\n",
			       table->table[i].sock,
			       table->table[i].circle_data,
			       table->table[i].user_data,
			       table->table[i].handler);
		}
		os_free(table->table);
	}
}


int circle_register_read_sock(int sock, circle_sock_handler handler,
			     void *circle_data, void *user_data)
{
	return circle_register_sock(sock, EVENT_TYPE_READ, handler,
				   circle_data, user_data);
}


void circle_unregister_read_sock(int sock)
{
	circle_unregister_sock(sock, EVENT_TYPE_READ);
}


static struct circle_sock_table *circle_get_sock_table(circle_event_type type)
{
	switch (type) {
	case EVENT_TYPE_READ:
		return &circle.readers;
	case EVENT_TYPE_WRITE:
		return &circle.writers;
	case EVENT_TYPE_EXCEPTION:
		return &circle.exceptions;
	}

	return NULL;
}


int circle_register_sock(int sock, circle_event_type type,
			circle_sock_handler handler,
			void *circle_data, void *user_data)
{
	struct circle_sock_table *table;

	table = circle_get_sock_table(type);
	return circle_sock_table_add_sock(table, sock, handler,
					 circle_data, user_data);
}


void circle_unregister_sock(int sock, circle_event_type type)
{
	struct circle_sock_table *table;

	table = circle_get_sock_table(type);
	circle_sock_table_remove_sock(table, sock);
}


int circle_register_timeout(unsigned int secs, unsigned int usecs,
			   circle_timeout_handler handler,
			   void *circle_data, void *user_data)
{
	struct circle_timeout *timeout, *tmp, *prev;

	timeout = os_zalloc(sizeof(*timeout));
	if (timeout == NULL)
		return -1;
	//os_get_time(&timeout->time);
	os_get_time_sys(&timeout->time);//qiuchen add it
	timeout->time.sec += secs;
	timeout->time.usec += usecs;
	while (timeout->time.usec >= 1000000) {
		timeout->time.sec++;
		timeout->time.usec -= 1000000;
	}
	timeout->circle_data = circle_data;
	timeout->user_data = user_data;
	timeout->handler = handler;
	timeout->next = NULL;

	if (circle.timeout == NULL) {
		circle.timeout = timeout;
#ifdef ASD_MULTI_THREAD_MODE
		if(sendto(LoopSend, "hello", sizeof("hello"), 0, (struct sockaddr *) &ASD_LOOP.addr, ASD_LOOP.addrlen) < 0){
			asd_printf(ASD_DEFAULT,MSG_CRIT,"%s sendto %s\n",__func__,strerror(errno));
			perror("send(ASD_LOOP_Socket)");
			asd_printf(ASD_DEFAULT,MSG_WARNING,"send to asd_loop failed!\n");
			return -1;
		}
#endif
		return 0;
	}

	prev = NULL;
	tmp = circle.timeout;
	while (tmp != NULL) {
		if (os_time_before(&timeout->time, &tmp->time))
			break;
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL) {
		timeout->next = circle.timeout;
		circle.timeout = timeout;
	} else {
		timeout->next = prev->next;
		prev->next = timeout;
	}
#ifdef ASD_MULTI_THREAD_MODE
	if(sendto(LoopSend, "hello", sizeof("hello"), 0, (struct sockaddr *) &ASD_LOOP.addr, ASD_LOOP.addrlen) < 0){
		asd_printf(ASD_DEFAULT,MSG_CRIT,"%s sendto %s\n",__func__,strerror(errno));
		perror("send(ASD_LOOP_Socket)");
		asd_printf(ASD_DEFAULT,MSG_WARNING,"send to asd_loop failed!\n");
		return -1;
	}
#endif	
	return 0;
}


int circle_cancel_timeout(circle_timeout_handler handler,
			 void *circle_data, void *user_data)
{
	struct circle_timeout *timeout, *prev, *next;
	int removed = 0;

	prev = NULL;
	timeout = circle.timeout;
	while (timeout != NULL) {
		next = timeout->next;

		if (timeout->handler == handler &&
		    (timeout->circle_data == circle_data ||
		     circle_data == circle_ALL_CTX) &&
		    (timeout->user_data == user_data ||
		     user_data == circle_ALL_CTX)) {
			if (prev == NULL)
				circle.timeout = next;
			else
				prev->next = next;
			os_free(timeout);
			removed++;
		} else
			prev = timeout;

		timeout = next;
	}

	return removed;
}


static void circle_handle_alarm(int sig)
{
	asd_printf(ASD_DEFAULT,MSG_ERROR, "circle: could not process SIGINT or SIGTERM in two "
		"seconds. Looks like there\n"
		"is a bug that ends up in a busy loop that "
		"prevents clean shutdown.\n"
		"Killing program forcefully.\n");
	exit(1);
}


static void circle_handle_signal(int sig)
{
	int i;

	if ((sig == SIGINT || sig == SIGTERM) && !circle.pending_terminate) {
		/* Use SIGALRM to break out from potential busy loops that
		 * would not allow the program to be killed. */
		circle.pending_terminate = 1;
		signal(SIGALRM, circle_handle_alarm);
		alarm(2);
	}

	circle.signaled++;
	for (i = 0; i < circle.signal_count; i++) {
		if (circle.signals[i].sig == sig) {
			circle.signals[i].signaled++;
			break;
		}
	}
}


static void circle_process_pending_signals(void)
{
	int i;

	if (circle.signaled == 0)
		return;
	circle.signaled = 0;

	if (circle.pending_terminate) {
		alarm(0);
		circle.pending_terminate = 0;
	}

	for (i = 0; i < circle.signal_count; i++) {
		if (circle.signals[i].signaled) {
			circle.signals[i].signaled = 0;
			circle.signals[i].handler(circle.signals[i].sig,
						 circle.user_data,
						 circle.signals[i].user_data);
		}
	}
}


int circle_register_signal(int sig, circle_signal_handler handler,
			  void *user_data)
{
	struct circle_signal *tmp;

	tmp = (struct circle_signal *)
		os_realloc(circle.signals,
			   (circle.signal_count + 1) *
			   sizeof(struct circle_signal));
	if (tmp == NULL)
		return -1;

	tmp[circle.signal_count].sig = sig;
	tmp[circle.signal_count].user_data = user_data;
	tmp[circle.signal_count].handler = handler;
	tmp[circle.signal_count].signaled = 0;
	circle.signal_count++;
	circle.signals = tmp;
	signal(sig, circle_handle_signal);

	return 0;
}


int circle_register_signal_terminate(circle_signal_handler handler,
				    void *user_data)
{
	int ret = circle_register_signal(SIGINT, handler, user_data);
	if (ret == 0)
		ret = circle_register_signal(SIGTERM, handler, user_data);
	return ret;
}


int circle_register_signal_reconfig(circle_signal_handler handler,
				   void *user_data)
{
	return circle_register_signal(SIGHUP, handler, user_data);
}


void circle_run(void)
{
	//fd_set *rfds, *wfds, *efds;
	int res;
	struct timeval _tv = {0,0};
	struct timeval _tv2 = {0,0};
	struct os_time tv, now;
	//int i = 0;
	int t1 = 0, t2 = 0;
	/*
	rfds = os_zalloc(sizeof(*rfds));
	wfds = os_zalloc(sizeof(*wfds));
	efds = os_zalloc(sizeof(*efds));
	if (rfds == NULL || wfds == NULL || efds == NULL) {
		asd_printf(ASD_DBUS,MSG_CRIT,"%s :malloc fail.\n",__func__);
		exit(1);
	}*/
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"asd_main start circle...\n");
	while (!circle.terminate &&
	       (circle.timeout || circle.readers.count > 0 ||
		circle.writers.count > 0 || circle.exceptions.count > 0)) {
		if (circle.timeout) {
			//os_get_time(&now);
			os_get_time_sys(&now);//qiuchen add it
			if (os_time_before(&now, &circle.timeout->time))
				os_time_sub(&circle.timeout->time, &now, &tv);
			else{
				tv.sec = 0;
				tv.usec = 0;
			}
#if 0
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"next timeout in %lu.%06lu sec\n",
			       tv.sec, tv.usec);
#endif
			_tv.tv_sec = tv.sec;
			_tv.tv_usec = tv.usec;
			t1 = _tv.tv_sec*1000 + _tv.tv_usec/1000;			
		}
		_tv2.tv_sec = 3;
		_tv2.tv_usec = 0;
		t2 = 3000;
		
		circle_sock_table_set_fds_test(&circle.readers,&circle.poll_table);
/*		circle_sock_table_set_fds(&circle.readers, rfds);
		circle_sock_table_set_fds(&circle.writers, wfds);
		circle_sock_table_set_fds(&circle.exceptions, efds);
*/		
/*		res = select(circle.max_sock + 1, rfds, wfds, efds,
			((circle.timeout) && (_tv.tv_sec < _tv2.tv_sec)) ? &_tv : &_tv2);
			//		 (_tv.tv_sec < _tv2.tv_sec) ? &_tv : &_tv2);
		if (res < 0 && errno != EINTR && errno != 0) {
			asd_printf(ASD_DEFAULT,MSG_CRIT,"%s select %s\n",__func__,strerror(errno));
			perror("select");
			//continue;
			//goto out;
		}
*/		circle_process_pending_signals();

		res = poll(circle.poll_table.poll,(unsigned long)circle.poll_table.count,
		((circle.timeout) && (_tv.tv_sec < _tv2.tv_sec)) ? t1 : t2);
		if (res < 0) {
			asd_printf(ASD_DEFAULT,MSG_CRIT,"%s select %s\n",__func__,strerror(errno));
		}

		/* check if some registered timeouts have occurred */
		if (circle.timeout) {
			struct circle_timeout *tmp;

			//os_get_time(&now);
			os_get_time_sys(&now);//qiuchen add it
			if (!os_time_before(&now, &circle.timeout->time)) {
				tmp = circle.timeout;
				circle.timeout = circle.timeout->next;
				tmp->handler(tmp->circle_data,
					     tmp->user_data);
				os_free(tmp);
			}

		}

		//if (res <= 0)
			//continue;

		circle_sock_table_dispatch_test(&circle.readers, &circle.poll_table);
/*		circle_sock_table_dispatch(&circle.readers, rfds);
		circle_sock_table_dispatch(&circle.writers, wfds);
		circle_sock_table_dispatch(&circle.exceptions, efds);
*/
		os_free(circle.poll_table.poll);
		circle.poll_table.poll = NULL;
		os_free(circle.poll_table.data);
		circle.poll_table.data = NULL;
	
	}
	asd_printf(ASD_DEFAULT,MSG_NOTICE,"asd end circle\n");
/*
out:
	os_free(rfds);
	os_free(wfds);
	os_free(efds);
*/
}


void circle_terminate(void)
{
	circle.terminate = 1;
}


void circle_destroy(void)
{
	struct circle_timeout *timeout, *prev;
	struct os_time now;

	timeout = circle.timeout;
	if (timeout)
		//os_get_time(&now);
		os_get_time_sys(&now);//qiuchen add it
	while (timeout != NULL) {
		int sec, usec;
		prev = timeout;
		timeout = timeout->next;
		sec = prev->time.sec - now.sec;
		usec = prev->time.usec - now.usec;
		if (prev->time.usec < now.usec) {
			sec--;
			usec += 1000000;
		}
		asd_printf(ASD_DEFAULT,MSG_INFO,"circle: remaining timeout: %d.%06d circle_data=%p "
		       "user_data=%p handler=%p\n",
		       sec, usec, prev->circle_data, prev->user_data,
		       prev->handler);
		os_free(prev);
	}
	circle_sock_table_destroy(&circle.readers);
	circle_sock_table_destroy(&circle.writers);
	circle_sock_table_destroy(&circle.exceptions);
	os_free(circle.signals);
}


int circle_terminated(void)
{
	return circle.terminate;
}


void circle_wait_for_read_sock(int sock)
{
	fd_set rfds;

	if (sock < 0)
		return;

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	select(sock + 1, &rfds, NULL, NULL, NULL);
}


void * circle_get_user_data(void)
{
	return circle.user_data;
}
