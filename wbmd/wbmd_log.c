#include <syslog.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "wbmd.h"
#include "wbmd_log.h"

int WBMDLOGLEVEL = WBMD_SYSLOG_DEFAULT;


void wbmd_syslog_emerg(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_EME [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_EMERG,buf);
	
}
void wbmd_syslog_alert(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_ALE [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_ALERT,buf);	
}
void wbmd_syslog_crit(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_CRI [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	
	va_end(ptr);
	syslog(LOG_CRIT,buf);
}
void wbmd_syslog_err(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_ERR [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR,buf);
}
void wbmd_syslog_warning(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_WAR [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_WARNING,buf);
}
void wbmd_syslog_notice(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_NOT [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void wbmd_syslog_notice_local7(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_NOT [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void wbmd_syslog_info(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"WBMD_INF [%d-%-2d] ",local,vrrid);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+16,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);
}
void wbmd_syslog_debug_debug(int type,char *format,...)
	
{
	char buf[2048] = {0};
	if(WBMDLOGLEVEL & type)
	{
		sprintf(buf,"WBMD_DEB [%d-%-2d] ",local,vrrid);
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+16,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
	}

}


void wbmd_syslog_init(){
	openlog("wbmd", 0, LOG_DAEMON);
}

