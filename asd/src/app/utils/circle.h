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
* circle.h
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

#ifndef circle_H
#define circle_H
#include "includes.h"

#include "common.h"
#include <sys/poll.h> 		//mahz add for test
#include "../include/auth.h"
/**
 * circle_ALL_CTX - circle_cancel_timeout() magic number to match all timeouts
 */
#define circle_ALL_CTX (void *) -1

/**
 * circle_event_type - circle socket event type for circle_register_sock()
 * @EVENT_TYPE_READ: Socket has data available for reading
 * @EVENT_TYPE_WRITE: Socket has room for new data to be written
 * @EVENT_TYPE_EXCEPTION: An exception has been reported
 */
typedef enum {
	EVENT_TYPE_READ = 0,
	EVENT_TYPE_WRITE,
	EVENT_TYPE_EXCEPTION
} circle_event_type;

/**
 * circle_sock_handler - circle socket event callback type
 * @sock: File descriptor number for the socket
 * @circle_ctx: Registered callback context data (circle_data)
 * @sock_ctx: Registered callback context data (user_data)
 */
typedef void (*circle_sock_handler)(int sock, void *circle_ctx, void *sock_ctx);

/**
 * circle_event_handler - circle generic event callback type
 * @circle_ctx: Registered callback context data (circle_data)
 * @sock_ctx: Registered callback context data (user_data)
 */
typedef void (*circle_event_handler)(void *circle_data, void *user_ctx);

/**
 * circle_timeout_handler - circle timeout event callback type
 * @circle_ctx: Registered callback context data (circle_data)
 * @sock_ctx: Registered callback context data (user_data)
 */
typedef void (*circle_timeout_handler)(void *circle_data, void *user_ctx);

/**
 * circle_signal_handler - circle signal event callback type
 * @sig: Signal number
 * @circle_ctx: Registered callback context data (global user_data from
 * circle_init() call)
 * @signal_ctx: Registered callback context data (user_data from
 * circle_register_signal(), circle_register_signal_terminate(), or
 * circle_register_signal_reconfig() call)
 */
typedef void (*circle_signal_handler)(int sig, void *circle_ctx,
				     void *signal_ctx);

/**
 * circle_init() - Initialize global event loop data
 * @user_data: Pointer to global data passed as circle_ctx to signal handlers
 * Returns: 0 on success, -1 on failure
 *
 * This function must be called before any other circle_* function. user_data
 * can be used to configure a global (to the process) pointer that will be
 * passed as circle_ctx parameter to signal handlers.
 */
int circle_init(void *user_data);

/**
 * circle_register_read_sock - Register handler for read events
 * @sock: File descriptor number for the socket
 * @handler: Callback function to be called when data is available for reading
 * @circle_data: Callback context data (circle_ctx)
 * @user_data: Callback context data (sock_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register a read socket notifier for the given file descriptor. The handler
 * function will be called whenever data is available for reading from the
 * socket. The handler function is responsible for clearing the event after
 * having processed it in order to avoid circle from calling the handler again
 * for the same event.
 */
int circle_register_read_sock(int sock, circle_sock_handler handler,
			     void *circle_data, void *user_data);

/**
 * circle_unregister_read_sock - Unregister handler for read events
 * @sock: File descriptor number for the socket
 *
 * Unregister a read socket notifier that was previously registered with
 * circle_register_read_sock().
 */
void circle_unregister_read_sock(int sock);

/**
 * circle_register_sock - Register handler for socket events
 * @sock: File descriptor number for the socket
 * @type: Type of event to wait for
 * @handler: Callback function to be called when the event is triggered
 * @circle_data: Callback context data (circle_ctx)
 * @user_data: Callback context data (sock_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register an event notifier for the given socket's file descriptor. The
 * handler function will be called whenever the that event is triggered for the
 * socket. The handler function is responsible for clearing the event after
 * having processed it in order to avoid circle from calling the handler again
 * for the same event.
 */
int circle_register_sock(int sock, circle_event_type type,
			circle_sock_handler handler,
			void *circle_data, void *user_data);

/**
 * circle_unregister_sock - Unregister handler for socket events
 * @sock: File descriptor number for the socket
 * @type: Type of event for which sock was registered
 *
 * Unregister a socket event notifier that was previously registered with
 * circle_register_sock().
 */
void circle_unregister_sock(int sock, circle_event_type type);

/**
 * circle_register_event - Register handler for generic events
 * @event: Event to wait (circle implementation specific)
 * @event_size: Size of event data
 * @handler: Callback function to be called when event is triggered
 * @circle_data: Callback context data (circle_data)
 * @user_data: Callback context data (user_data)
 * Returns: 0 on success, -1 on failure
 *
 * Register an event handler for the given event. This function is used to
 * register circle implementation specific events which are mainly targetted for
 * operating system specific code (driver interface and l2_packet) since the
 * portable code will not be able to use such an OS-specific call. The handler
 * function will be called whenever the event is triggered. The handler
 * function is responsible for clearing the event after having processed it in
 * order to avoid circle from calling the handler again for the same event.
 *
 * In case of Windows implementation (circle_win.c), event pointer is of HANDLE
 * type, i.e., void*. The callers are likely to have 'HANDLE h' type variable,
 * and they would call this function with circle_register_event(h, sizeof(h),
 * ...).
 */
int circle_register_event(void *event, size_t event_size,
			 circle_event_handler handler,
			 void *circle_data, void *user_data);

/**
 * circle_unregister_event - Unregister handler for a generic event
 * @event: Event to cancel (circle implementation specific)
 * @event_size: Size of event data
 *
 * Unregister a generic event notifier that was previously registered with
 * circle_register_event().
 */
void circle_unregister_event(void *event, size_t event_size);

/**
 * circle_register_timeout - Register timeout
 * @secs: Number of seconds to the timeout
 * @usecs: Number of microseconds to the timeout
 * @handler: Callback function to be called when timeout occurs
 * @circle_data: Callback context data (circle_ctx)
 * @user_data: Callback context data (sock_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register a timeout that will cause the handler function to be called after
 * given time.
 */
int circle_register_timeout(unsigned int secs, unsigned int usecs,
			   circle_timeout_handler handler,
			   void *circle_data, void *user_data);

/**
 * circle_cancel_timeout - Cancel timeouts
 * @handler: Matching callback function
 * @circle_data: Matching circle_data or %circle_ALL_CTX to match all
 * @user_data: Matching user_data or %circle_ALL_CTX to match all
 * Returns: Number of cancelled timeouts
 *
 * Cancel matching <handler,circle_data,user_data> timeouts registered with
 * circle_register_timeout(). circle_ALL_CTX can be used as a wildcard for
 * cancelling all timeouts regardless of circle_data/user_data.
 */
int circle_cancel_timeout(circle_timeout_handler handler,
			 void *circle_data, void *user_data);

/**
 * circle_register_signal - Register handler for signals
 * @sig: Signal number (e.g., SIGHUP)
 * @handler: Callback function to be called when the signal is received
 * @user_data: Callback context data (signal_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register a callback function that will be called when a signal is received.
 * The callback function is actually called only after the system signal
 * handler has returned. This means that the normal limits for sighandlers
 * (i.e., only "safe functions" allowed) do not apply for the registered
 * callback.
 *
 * Signals are 'global' events and there is no local circle_data pointer like
 * with other handlers. The global user_data pointer registered with
 * circle_init() will be used as circle_ctx for signal handlers.
 */
int circle_register_signal(int sig, circle_signal_handler handler,
			  void *user_data);

/**
 * circle_register_signal_terminate - Register handler for terminate signals
 * @handler: Callback function to be called when the signal is received
 * @user_data: Callback context data (signal_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register a callback function that will be called when a process termination
 * signal is received. The callback function is actually called only after the
 * system signal handler has returned. This means that the normal limits for
 * sighandlers (i.e., only "safe functions" allowed) do not apply for the
 * registered callback.
 *
 * Signals are 'global' events and there is no local circle_data pointer like
 * with other handlers. The global user_data pointer registered with
 * circle_init() will be used as circle_ctx for signal handlers.
 *
 * This function is a more portable version of circle_register_signal() since
 * the knowledge of exact details of the signals is hidden in circle
 * implementation. In case of operating systems using signal(), this function
 * registers handlers for SIGINT and SIGTERM.
 */
int circle_register_signal_terminate(circle_signal_handler handler,
				    void *user_data);

/**
 * circle_register_signal_reconfig - Register handler for reconfig signals
 * @handler: Callback function to be called when the signal is received
 * @user_data: Callback context data (signal_ctx)
 * Returns: 0 on success, -1 on failure
 *
 * Register a callback function that will be called when a reconfiguration /
 * hangup signal is received. The callback function is actually called only
 * after the system signal handler has returned. This means that the normal
 * limits for sighandlers (i.e., only "safe functions" allowed) do not apply
 * for the registered callback.
 *
 * Signals are 'global' events and there is no local circle_data pointer like
 * with other handlers. The global user_data pointer registered with
 * circle_init() will be used as circle_ctx for signal handlers.
 *
 * This function is a more portable version of circle_register_signal() since
 * the knowledge of exact details of the signals is hidden in circle
 * implementation. In case of operating systems using signal(), this function
 * registers a handler for SIGHUP.
 */
int circle_register_signal_reconfig(circle_signal_handler handler,
				   void *user_data);

/**
 * circle_run - Start the event loop
 *
 * Start the event loop and continue running as long as there are any
 * registered event handlers. This function is run after event loop has been
 * initialized with event_init() and one or more events have been registered.
 */
void circle_run(void);

/**
 * circle_terminate - Terminate event loop
 *
 * Terminate event loop even if there are registered events. This can be used
 * to request the program to be terminated cleanly.
 */
void circle_terminate(void);

/**
 * circle_destroy - Free any resources allocated for the event loop
 *
 * After calling circle_destroy(), other circle_* functions must not be called
 * before re-running circle_init().
 */
void circle_destroy(void);

/**
 * circle_terminated - Check whether event loop has been terminated
 * Returns: 1 = event loop terminate, 0 = event loop still running
 *
 * This function can be used to check whether circle_terminate() has been called
 * to request termination of the event loop. This is normally used to abort
 * operations that may still be queued to be run when circle_terminate() was
 * called.
 */
int circle_terminated(void);

/**
 * circle_wait_for_read_sock - Wait for a single reader
 * @sock: File descriptor number for the socket
 *
 * Do a blocking wait for a single read socket.
 */
void circle_wait_for_read_sock(int sock);

/**
 * circle_get_user_data - Get global user data
 * Returns: user_data pointer that was registered with circle_init()
 */
void * circle_get_user_data(void);

struct circle_sock {
	int sock;
	void *circle_data;
	void *user_data;
	circle_sock_handler handler;
};

struct pool_data {
	void *circle_data;
	void *user_data;
	circle_sock_handler handler;
};
struct pool_all {
	int count;
	struct pollfd * poll;
	struct pool_data * data;
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
	struct pool_all poll_table;

	struct circle_timeout *timeout;

	int signal_count;
	struct circle_signal *signals;
	int signaled;
	int pending_terminate;

	int terminate;
	int reader_table_changed;
};

extern struct circle_data circle;

#endif /* circle_H */
