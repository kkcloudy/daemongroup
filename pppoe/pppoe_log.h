#ifndef _PPPOE_LOG_H
#define _PPPOE_LOG_H

#include <syslog.h>

#ifndef LOG_ERR
#define LOG_EMERG		0       /* system is unusable */
#define LOG_ALERT		1       /* action must be taken immediately */
#define LOG_CRIT		2       /* critical conditions */
#define LOG_ERR			3       /* error conditions */
#define LOG_WARNING		4       /* warning conditions */
#define LOG_NOTICE		5       /* normal but significant condition */
#define LOG_INFO		6       /* informational */
#define LOG_DEBUG		7       /* debug-level messages */
#endif


#define pppoe_log(level, format, args...) 	\
	do {		\
		syslog(level, "%s: "format, __func__, ##args);	\
	} while(0);

#define pppoe_token_log(token, format, args...) \
	do {		\
		if (unlikely(pppoe_token_is_registered(token)))	\
			syslog(LOG_DEBUG, "%s: %s: "format,	\
					pppoe_token_desc(token), __func__, ##args);	\
	} while(0);
	

int pppoe_token_is_registered(PPPOELogToken token);
char *pppoe_token_desc(PPPOELogToken token);
int pppoe_token_register(PPPOELogToken token);
int pppoe_token_unregister(PPPOELogToken token);

void pppoe_debug_config(uint32 type);

void pppoe_log_init(char *daemon);
void pppoe_log_exit(void);
	
#endif
