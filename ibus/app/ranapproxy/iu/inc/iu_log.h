#ifndef IU_LOG_H
#define IU_LOG_H
#define IU_SYSLOG_DEFAULT 0
enum dhcp_debug_type {	
	DEBUG_TYPE_INFO  = 1,
	DEBUG_TYPE_ERROR = 2,
	DEBUG_TYPE_DEBUG = 4,
	DEBUG_TYPE_ALL   = 7,
};

int iu_log_error (const char * fmt, ...);

int iu_log_info (const char *fmt, ...);

int iu_log_debug (const char *fmt, ...);

void iu_log_init();

#endif 
