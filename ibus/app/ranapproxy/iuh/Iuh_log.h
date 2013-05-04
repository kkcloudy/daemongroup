#ifndef _IUH_LOG_H
#define _IUH_LOG_H

#define IUH_SYSLOG_EMERG	0
#define IUH_SYSLOG_ALERT	1
#define IUH_SYSLOG_CRIT		2
#define IUH_SYSLOG_ERR		3
#define IUH_SYSLOG_WARNING	4
#define IUH_SYSLOG_NOTICE	5
#define IUH_SYSLOG_INFO		6
#define IUH_SYSLOG_DEBUG	7
#define IUH_SYSLOG_DEFAULT	0


#define IUH_SYSLOG_DEBUG_NONE		0
#define IUH_SYSLOG_DEBUG_INFO		1
#define IUH_SYSLOG_DEBUG_DEBUG		8
#define IUH_SYSLOG_DEBUG_ALL		15
#define IUH_SYSLOG_DEBUG_DEFAULT	15

extern int gIuhLOGLEVEL;

enum iuh_debug{
	IUH_DEFAULT = 0x1,
	IUH_DBUS = 0x2,
	IUH_HNBINFO = 0x4,
	IUH_MB = 0x8,/*master and bak*/
	IUH_ALL = 0xf
};


void iuh_syslog_emerg(char *format,...);
	

void iuh_syslog_alert(char *format,...);
	

void iuh_syslog_crit(char *format,...);
	

void iuh_syslog_err(char *format,...);
	

void iuh_syslog_warning(char *format,...);
	

void iuh_syslog_notice(char *format,...);
	


void iuh_syslog_notice_local7(char *format,...);
	


void iuh_syslog_info(char *format,...);
	

void iuh_syslog_debug_debug(int type,char *format,...);
	



void iuh_syslog_init();


#endif
