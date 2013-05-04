#ifndef _BSD_LOG_H
#define _BSD_LOG_H

#define BSD_SYSLOG_EMERG	0
#define BSD_SYSLOG_ALERT	1
#define BSD_SYSLOG_CRIT		2
#define BSD_SYSLOG_ERR		3
#define BSD_SYSLOG_WARNING	4
#define BSD_SYSLOG_NOTICE	5
#define BSD_SYSLOG_INFO		6
#define BSD_SYSLOG_DEBUG	7
#define BSD_SYSLOG_DEFAULT	0\


#define BSD_SYSLOG_DEBUG_NONE		0
#define BSD_SYSLOG_DEBUG_INFO		1
#define BSD_SYSLOG_DEBUG_DEBUG		8
#define BSD_SYSLOG_DEBUG_ALL		15
#define BSD_SYSLOG_DEBUG_DEFAULT	15

extern int BsdLOGLEVEL;

enum bsd_debug{
	BSD_DEFAULT = 0x1,
	BSD_DBUS = 0x2,
	BSD_BSDINFO = 0x4,
	BSD_MB = 0x8,/*master and bak*/
	BSD_ALL = 0xf
};


void bsd_syslog_emerg(char *format,...);
	
void bsd_syslog_alert(char *format,...);

void bsd_syslog_crit(char *format,...);
	
void bsd_syslog_err(char *format,...);

void bsd_syslog_warning(char *format,...);

void bsd_syslog_notice(char *format,...);
	
void bsd_syslog_notice_local7(char *format,...);
	
void bsd_syslog_info(char *format,...);
	
void bsd_syslog_debug_debug(int type,char *format,...);
	
void bsd_syslog_init();


#endif

