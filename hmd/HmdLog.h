#ifndef _HMD_LOG_H
#define _HMD_LOG_H

#define HMD_SYSLOG_EMERG	0
#define HMD_SYSLOG_ALERT	1
#define HMD_SYSLOG_CRIT		2
#define HMD_SYSLOG_ERR		3
#define HMD_SYSLOG_WARNING	4
#define HMD_SYSLOG_NOTICE	5
#define HMD_SYSLOG_INFO		6
#define HMD_SYSLOG_DEBUG	7
#define HMD_SYSLOG_DEFAULT	0


#define HMD_SYSLOG_DEBUG_NONE		0
#define HMD_SYSLOG_DEBUG_INFO		1
#define HMD_SYSLOG_DEBUG_DEBUG		8
#define HMD_SYSLOG_DEBUG_ALL		15
#define HMD_SYSLOG_DEBUG_DEFAULT	15

enum hmd_debug{
	HMD_DEFAULT = 0x1,
	HMD_DBUS = 0x2,
	HMD_WTPINFO = 0x4,
	HMD_MB = 0x8,/*master and bak*/
	HMD_ALL = 0xf
};


void hmd_syslog_emerg(char *format,...);
	

void hmd_syslog_alert(char *format,...);
	

void hmd_syslog_crit(char *format,...);
	

void hmd_syslog_err(char *format,...);
	

void hmd_syslog_warning(char *format,...);
	

void hmd_syslog_notice(char *format,...);
	


void hmd_syslog_notice_local7(char *format,...);
	


void hmd_syslog_info(char *format,...);
	

void hmd_syslog_debug_debug(int type,char *format,...);
	



void hmd_syslog_init();


#endif

