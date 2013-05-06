#ifndef _MANAGE_LOG_H_
#define _MANAGE_LOG_H_


#ifndef LOG_ERR
#define LOG_EMERG		0       /* system is unusable */
#define LOG_ALERT		1       /* action must be taken immediately */
#define LOG_CRIT			2       /* critical conditions */
#define LOG_ERR			3       /* error conditions */
#define LOG_WARNING	4       /* warning conditions */
#define LOG_NOTICE		5       /* normal but significant condition */
#define LOG_INFO		6       /* informational */
#define LOG_DEBUG		7       /* debug-level messages */
#endif

enum {
	LOG_TOKEN_INIT,
	LOG_TOKEN_SESSION,
	LOG_TOKEN_TRANSPORT,
	LOG_TOKEN_TASK,
	LOG_TOKEN_TIPC,
};

#define MANAGE_LOG_DEBUG_OFF		(0)
#define MANAGE_LOG_DEBUG_ON		(1)		

#define	MANAGE_MAX_DEBUG_TOKENS	(256)


void manage_log(u_int priority, const char *format, ...);

void manage_token_log(u_int token, const char *format, ...);

int manage_token_register(u_int token);

int manage_token_unregister(u_int token);

void manage_debug_config(u_int debug);


#endif
