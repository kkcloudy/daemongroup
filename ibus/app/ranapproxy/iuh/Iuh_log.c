#include <syslog.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "iuh/Iuh.h"
#include "Iuh_log.h"

int gIuhLOGLEVEL = IUH_SYSLOG_DEFAULT;


void iuh_syslog_emerg(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_EME ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_EMERG,buf);
	
}
void iuh_syslog_alert(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_ALE ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_ALERT,buf);	
}
void iuh_syslog_crit(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_CRI ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	
	va_end(ptr);
	syslog(LOG_CRIT,buf);
}
void iuh_syslog_err(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_ERR ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR,buf);
}
void iuh_syslog_warning(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_WAR ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_WARNING,buf);
}
void iuh_syslog_notice(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_NOT ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void iuh_syslog_notice_local7(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_NOT ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void iuh_syslog_info(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"IUH_INF ");
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+8,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);
}
void iuh_syslog_debug_debug(int type,char *format,...)
	
{
	char buf[2048] = {0};
	if(gIuhLOGLEVEL & type)
	{
		sprintf(buf,"IUH_DEB ");
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+8,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
	}

}


void iuh_syslog_init(){
	char buf[16] = {0};
	sprintf(buf,"iuh[%d_%d]", local, vrrid);
	openlog(buf, 0, LOG_DAEMON);
}


