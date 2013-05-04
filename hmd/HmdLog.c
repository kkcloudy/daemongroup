#include <syslog.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "hmd.h"
#include "HmdLog.h"

int HmdLOGLEVEL = HMD_SYSLOG_DEFAULT;


void hmd_syslog_emerg(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_EME [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_EMERG,buf);
	
}
void hmd_syslog_alert(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_ALE [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_ALERT,buf);	
}
void hmd_syslog_crit(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_CRI [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	
	va_end(ptr);
	syslog(LOG_CRIT,buf);
}
void hmd_syslog_err(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_ERR [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR,buf);
}
void hmd_syslog_warning(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_WAR [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_WARNING,buf);
}
void hmd_syslog_notice(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_NOT [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void hmd_syslog_notice_local7(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_NOT [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);
}

void hmd_syslog_info(char *format,...)
	
{
	char buf[2048] = {0};
	sprintf(buf,"HMD_INF [%d] ",HOST_SLOT_NO);
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf+12,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);
	return;
}
void hmd_syslog_debug_debug(int type,char *format,...)
	
{
	char buf[2048] = {0};
	if(HmdLOGLEVEL & type)
	{
		sprintf(buf,"HMD_DEB [%d] ",HOST_SLOT_NO);
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+12,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
	}

}


void hmd_syslog_init(){
	openlog("hmd", 0, LOG_DAEMON);
}

