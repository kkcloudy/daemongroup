#include <syslog.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "bsd.h"
#include "bsd_log.h"

int BsdLOGLEVEL = BSD_SYSLOG_DEBUG_NONE;


void bsd_syslog_emerg(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_EME [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_EMERG,buf);
	
}
void bsd_syslog_alert(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_ALE [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_ALERT,buf);	
}
void bsd_syslog_crit(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_CRI [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	
	va_end(ptr);
	syslog(LOG_CRIT,buf);
}
void bsd_syslog_err(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_ERR [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR,buf);
}
void bsd_syslog_warning(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_WAR [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_WARNING,buf);
}
void bsd_syslog_notice(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_NOT [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void bsd_syslog_notice_local7(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_NOT [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void bsd_syslog_info(char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	sprintf(buf,"BSD_INF [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);
}
void bsd_syslog_debug_debug(int type,char *format,...)
	
{
	char buf[BSD_BIG_BUF_LENGTH] = {0};
	if(BsdLOGLEVEL & type)
	{
		sprintf(buf,"BSD_DEB [%d] ",HOST_SLOT_NO);
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+8,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
	}

}


void bsd_syslog_init(){
	openlog("bsd", 0, LOG_DAEMON);
}

