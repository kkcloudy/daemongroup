#ifndef _WBMD_LOG_H
#define _WBMD_LOG_H

#define WBMD_SYSLOG_EMERG	0
#define WBMD_SYSLOG_ALERT	1
#define WBMD_SYSLOG_CRIT		2
#define WBMD_SYSLOG_ERR		3
#define WBMD_SYSLOG_WARNING	4
#define WBMD_SYSLOG_NOTICE	5
#define WBMD_SYSLOG_INFO		6
#define WBMD_SYSLOG_DEBUG	7
#define WBMD_SYSLOG_DEFAULT	0


#define WBMD_SYSLOG_DEBUG_NONE		0
#define WBMD_SYSLOG_DEBUG_INFO		1
#define WBMD_SYSLOG_DEBUG_DEBUG		8
#define WBMD_SYSLOG_DEBUG_ALL		15
#define WBMD_SYSLOG_DEBUG_DEFAULT	15

enum wbmd_debug{
	WBMD_DEFAULT = 0x1,
	WBMD_DBUS = 0x2,
	WBMD_WTPINFO = 0x4,
	WBMD_MB = 0x8,/*master and bak*/
	WBMD_ALL = 0xf
};


void wbmd_syslog_emerg(char *format,...);
	

void wbmd_syslog_alert(char *format,...);
	

void wbmd_syslog_crit(char *format,...);
	

void wbmd_syslog_err(char *format,...);
	

void wbmd_syslog_warning(char *format,...);
	

void wbmd_syslog_notice(char *format,...);
	


void wbmd_syslog_notice_local7(char *format,...);
	


void wbmd_syslog_info(char *format,...);
	

void wbmd_syslog_debug_debug(int type,char *format,...);
	



void wbmd_syslog_init();


#endif

