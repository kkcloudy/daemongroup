/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* CWLog.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#include "CWCommon.h"
#include <syslog.h>
#include <assert.h>
#include "wcpss/wid/WID.h"
#include "CWAC.h"


//#define WRITE_STD_OUTPUT 1 

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static FILE *gLogFile = NULL;

#ifndef CW_SINGLE_THREAD
	CWThreadMutex gFileMutex;
#endif
int gWIDLogdebugLevel = WID_SYSLOG_DEBUG_DEFAULT;
int gWIDLOGLEVEL = WID_SYSLOG_DEFAULT;
unsigned char gWIDLOGHN = 0;//qiuchen add it
unsigned long gWID_AC_MANAGEMENT_IP = 0;
void CWLogInitFile(char *fileName) {
	if(fileName == NULL) {
		wid_syslog_debug_debug(WID_DEFAULT,"Wrong File Name for Log File");
	}
	
	if((gLogFile = fopen(fileName, "a")) == NULL) {
		wid_syslog_crit("Can't open log file: %s", strerror(errno));
		exit(1);
	}
	
	#ifndef CW_SINGLE_THREAD
		if(!CWCreateThreadMutex(&gFileMutex)) {
			wid_syslog_crit("Can't Init File Mutex for Log");
			exit(1);
		}
	#endif
}


CWBool checkResetFile()
{
	long fileSize=0;

	if((fileSize=ftell(gLogFile))==-1)
	{
		wid_syslog_crit("An error with log file occurred: %s", strerror(errno));
		return 0;
	}
	if (fileSize>=gMaxLogFileSize)
	{
		fclose(gLogFile);
		if((gLogFile = fopen(gLogFileName, "w")) == NULL) 
		{
			wid_syslog_crit("Can't open log file: %s", strerror(errno));
			return 0;
		}
	}
	return 1;
}


void CWLogCloseFile() {
	#ifndef CW_SINGLE_THREAD
		CWDestroyThreadMutex(&gFileMutex);
	#endif
	
	fclose(gLogFile);
}

__inline__ void CWVLog(const char *format, va_list args) {
	char *logStr = NULL;
	time_t now;
	char *nowReadable = NULL;
		
	if(format == NULL) return;
	
	now = time(NULL);
	nowReadable = ctime(&now);
	
	nowReadable[strlen(nowReadable)-1] = '\0';
	
	// return in case of memory err: we're not performing a critical task
	CW_CREATE_STRING_ERR_WID(logStr, (strlen(format)+strlen(nowReadable)+100), return;);
	
	//sprintf(logStr, "[CAPWAP::%s]\t\t %s\n", nowReadable, format);
	sprintf(logStr, "[CAPWAP::%s]\t%08x\t %s\n", nowReadable, (unsigned int)CWThreadSelf(), format);

	if(gLogFile != NULL) {
		char fileLine[256];
		
		#ifndef CW_SINGLE_THREAD
			CWThreadMutexLock(&gFileMutex);
			fseek(gLogFile, 0L, SEEK_END);
		#endif
		
		vsnprintf(fileLine, 255, logStr, args);
	
		if(!checkResetFile()) 
		{
			CWThreadMutexUnlock(&gFileMutex);
			exit (1);
		}
		
		fwrite(fileLine, strlen(fileLine), 1, gLogFile);
		fflush(gLogFile);
		
		#ifndef CW_SINGLE_THREAD
			CWThreadMutexUnlock(&gFileMutex);
		#endif
	}
#ifdef WRITE_STD_OUTPUT
	vprintf(logStr, args);
#endif	
	
	CW_FREE_OBJECT_WID(logStr);
}

__inline__ void CWLog(const char *format, ...) {
	va_list args;
	
	va_start(args, format);
	if (gEnabledLog)
		{CWVLog(format, args);}
	va_end(args);
}

__inline__ void CWDebugLog(const char *format, ...) {
	#ifdef CW_DEBUGGING
		char *logStr = NULL;
		va_list args;
		time_t now;
		char *nowReadable = NULL;
		
		if (!gEnabledLog) {return;}

		if(format == NULL) {
#ifdef WRITE_STD_OUTPUT
			printf("\n");
#endif
			return;
		}
		
		now = time(NULL);
		nowReadable = ctime(&now);
		
		nowReadable[strlen(nowReadable)-1] = '\0';
		
		// return in case of memory err: we're not performing a critical task
		CW_CREATE_STRING_ERR_WID(logStr, (strlen(format)+strlen(nowReadable)+100), return;);
		
		//sprintf(logStr, "[[CAPWAP::%s]]\t\t %s\n", nowReadable, format);
		sprintf(logStr, "[CAPWAP::%s]\t%08x\t %s\n", nowReadable, (unsigned int)CWThreadSelf(), format);

		va_start(args, format);
		
		if(gLogFile != NULL) {
			char fileLine[256];
			
			#ifndef CW_SINGLE_THREAD
				CWThreadMutexLock(&gFileMutex);
				fseek(gLogFile, 0L, SEEK_END);
			#endif
			
			vsnprintf(fileLine, 255, logStr, args);

			if(!checkResetFile()) 
			{
				CWThreadMutexUnlock(&gFileMutex);
				exit (1);
			}

			fwrite(fileLine, strlen(fileLine), 1, gLogFile);
			
			fflush(gLogFile);
			
			#ifndef CW_SINGLE_THREAD
			CWThreadMutexUnlock(&gFileMutex);
			#endif
		}
#ifdef WRITE_STD_OUTPUT	
		vprintf(logStr, args);
#endif
		
		va_end(args);
		CW_FREE_OBJECT_WID(logStr);
	#endif
}

__inline__ void WIDVLog(int level,const char *format, va_list args) {
	char *logStr = NULL;
	time_t now;
	char *nowReadable = NULL;
	char widloglevel[20];	
	if(format == NULL) return;
	
	now = time(NULL);
	nowReadable = ctime(&now);
	
	nowReadable[strlen(nowReadable)-1] = '\0';
	
	// return in case of memory err: we're not performing a critical task
	CW_CREATE_STRING_ERR_WID(logStr, (strlen(format)+strlen(nowReadable)+100), return;);
	
	switch(level){
		case WID_SYSLOG_EMERG:
			strcpy(widloglevel, "Emerg");
			break;
			
		case WID_SYSLOG_DEBUG_INFO:
			strcpy(widloglevel, "Info");
			break;
			
		case WID_SYSLOG_CRIT:
			strcpy(widloglevel, "Crit");
			break;

		case WID_SYSLOG_ERR:
			strcpy(widloglevel, "Err");
			break;

		case WID_SYSLOG_WARNING:
			strcpy(widloglevel, "Warning");
			break;

		case WID_SYSLOG_NOTICE:
			strcpy(widloglevel, "Notice");
			break;
			
		case WID_SYSLOG_INFO:
			strcpy(widloglevel, "Info");
			break;
			
		case WID_SYSLOG_DEBUG:
			strcpy(widloglevel, "Debug");
			break;

		case WID_SYSLOG_DEBUG_DEBUG:
			strcpy(widloglevel, "Debug");
			break;
			
		default :
			strcpy(widloglevel, "All");
			break;
	}
	sprintf(logStr, "[CAPWAP::%s]\t%08x\t %s\t%s\n", nowReadable, (unsigned int)CWThreadSelf(),widloglevel,format);

	if(gLogFile != NULL) {
		char fileLine[256];
		
		#ifndef CW_SINGLE_THREAD
			CWThreadMutexLock(&gFileMutex);
			fseek(gLogFile, 0L, SEEK_END);
		#endif
		
		vsnprintf(fileLine, 255, logStr, args);
	
		if(!checkResetFile()) 
		{
			CWThreadMutexUnlock(&gFileMutex);
			exit (1);
		}
		
		fwrite(fileLine, strlen(fileLine), 1, gLogFile);
		fflush(gLogFile);
		
		#ifndef CW_SINGLE_THREAD
			CWThreadMutexUnlock(&gFileMutex);
		#endif
	}
#ifdef WRITE_STD_OUTPUT
	vprintf(logStr, args);
#endif	
	
	CW_FREE_OBJECT_WID(logStr);
}
__inline__ void WID_Log(int level,const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	if (gEnabledLog)
		{
		if ( level == WID_SYSLOG_DEBUG_DEBUG )
			{
			if (gWIDLogdebugLevel & level)
				{
				level = WID_SYSLOG_DEBUG;
				WIDVLog(level,format, args);
				}
			}
		else if ( level == WID_SYSLOG_DEBUG_INFO)
			{
			if (gWIDLogdebugLevel & level)
				{
				level = WID_SYSLOG_DEBUG;
				WIDVLog(level,format, args);}
			}
		else 
			WIDVLog(level,format, args);
		}
	va_end(args);

}
//sz20080927




void wid_syslog_emerg(char *format,...)
	
{
    //char *ident = "WID_EME";
	//int wid_log_level = WID_SYSLOG_EMERG;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	
	//openlog(ident, 0, LOG_DAEMON); 
		sprintf(buf,"WID_EME ");
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+8,format,ptr);
		va_end(ptr);
		syslog(LOG_EMERG,buf);

	
	//closelog();
	
}
void wid_syslog_alert(char *format,...)
	
{
    //char *ident = "WID_ALE";
	//int wid_log_level = WID_SYSLOG_ALERT;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//openlog(ident, 0, LOG_DAEMON); 
		sprintf(buf,"WID_ALE ");
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+8,format,ptr);
		va_end(ptr);
		syslog(LOG_ALERT,buf);

	
	//closelog();
	
}
void wid_syslog_crit(char *format,...)
	
{
    //char *ident = "WID_CRI";
	//int wid_log_level = WID_SYSLOG_CRIT;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	
	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_CRI L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_CRI R[%d-%d] ",slotid,vrrid);	

		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		
		va_end(ptr);
		syslog(LOG_CRIT,buf);

	
	//closelog();

}
void wid_syslog_err(char *format,...)
	
{
    //char *ident = "WID_ERR";
	//int wid_log_level = WID_SYSLOG_ERR;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_ERR L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_ERR R[%d-%d] ",slotid,vrrid);	

		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_ERR,buf);

	
	//closelog();

}
void wid_syslog_warning(char *format,...)
	
{
    //char *ident = "WID_WAR";
	//int wid_log_level = WID_SYSLOG_WARNING;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_WAR L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_WAR R[%d-%d] ",slotid,vrrid); 

		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_WARNING,buf);

	
	//closelog();

}
void wid_syslog_notice(char *format,...)
	
{
    //char *ident = "WID_NOT";
	//int wid_log_level = WID_SYSLOG_NOTICE;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
		

	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_NOT L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_NOT R[%d-%d] ",slotid,vrrid); 

		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_NOTICE,buf);

	
	//closelog();
	
}

void wid_syslog_notice_local7(char *format,...)
	
{
    //char *ident = "WID_NOT";
	//int wid_log_level = WID_SYSLOG_NOTICE;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
		

	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_NOT L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_NOT R[%d-%d] ",slotid,vrrid); 
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_NOTICE,buf);

	
	//closelog();
	
}

void wid_syslog_info(char *format,...)
	
{
    //char *ident = "WID_INF";
	//int wid_log_level = WID_SYSLOG_INFO;
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//openlog(ident, 0, LOG_DAEMON); 
		if(local)
			sprintf(buf,"WID_INF L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_INF R[%d-%d] ",slotid,vrrid); 
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_INFO,buf);

	
	//closelog();

}
void wid_syslog_debug_debug(int type,char *format,...)
	
{
	char buf[2048] = {0};

	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	if(gWIDLOGLEVEL & type)
	{
		//openlog(ident, 0, LOG_DAEMON);
		if(local)
			sprintf(buf,"WID_DEB L[%d-%d] ",slotid,vrrid);
		else
			sprintf(buf,"WID_DEB R[%d-%d] ",slotid,vrrid); 
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf+15,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
		//closelog();
	}

}

void wid_syslog_debug_info(char *format,...)
	
{
	char ident[20] = {0};

	if(local)
		sprintf(ident,"WID_DBI L[%d-%d] ",slotid,vrrid);
	else
		sprintf(ident,"WID_DBI R[%d-%d] ",slotid,vrrid); 

    	//char *ident = "WID_DBI";
	int wid_log_level = WID_SYSLOG_DEBUG_INFO;
	char buf[2048] = {0};
	
	
	//first,write to 'ac.log.txt' which is belong to wid itself
	//if 'ac.log.txt' will be removed,delete this step
	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//start syslog writing
	if(gWIDLOGLEVEL & wid_log_level)
	{
		openlog(ident, 0, LOG_DAEMON);
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
		closelog();
	}
}
void wid_syslog_debug(char *format,...)
	
{
	char ident[20] = {0};

	if(local)
		sprintf(ident,"WID_DBD L[%d-%d] ",slotid,vrrid);
	else
		sprintf(ident,"WID_DBD R[%d-%d] ",slotid,vrrid); 

    	//char *ident = "WID_DBD";
	int wid_log_level = WID_SYSLOG_DEBUG;
	char buf[2048] = {0};

	//first,write to 'ac.log.txt' which is belong to wid itself
	//if 'ac.log.txt' will be removed,delete this step
	//va_list args;
	//va_start(args,format);
	//WIDVLog(wid_log_level,format,args);
	//va_end(args);
	
	//start syslog writing
	if(gWIDLOGLEVEL & wid_log_level)
	{ 
		openlog(ident, 0, LOG_DAEMON);
		va_list ptr;
		va_start(ptr,format);
		vsprintf(buf,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
		closelog();
	}
}

char *wid_get_current_time(void)
{
	
	time_t now;
	struct tm *timenow;
	
	time(&now);
	timenow = localtime(&now);
	
	return asctime(timenow);
}


void wid_pid_write(unsigned int vrrid)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	char *time_ptr = NULL, *temp_ptr=NULL;
	pid_t myPid = 0;
	int fd;
#ifndef _DISTRIBUTION_
	sprintf(pidPath,"%s%d%s", "/var/run/wcpss/wid", \
				vrrid, ".pid");
#else
	sprintf(pidPath,"%s%d_%d%s", "/var/run/wcpss/wid", \
				local,vrrid, ".pid");
#endif
		
	fd = open(pidPath, O_RDWR|O_CREAT);

	time_ptr = wid_get_current_time();
	strcpy(pidBuf, time_ptr);
	temp_ptr = (char *)(pidBuf + strlen(time_ptr) -1);
	*temp_ptr = ' ';
	temp_ptr++;

	myPid = getpid();	
	sprintf(temp_ptr,"%d\n", myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	return;
}




void wid_pid_write_v2(char *name,int id,unsigned int vrrid)
{
	//printf("%s start\n",__func__);
	char pidBuf[128] = {0}, pidPath[128] = {0};
	char idbuf[128] = {0};
	char *time_ptr = NULL, *temp_ptr = NULL;
	pid_t myPid = 0;
	static int fd;
	static unsigned char opened = 0;
#ifndef _DISTRIBUTION_
	sprintf(pidPath,"%s%d%s", "/var/run/wcpss/wid_thread", \
				vrrid, ".pid");
#else
	sprintf(pidPath,"%s%d_%d%s", "/var/run/wcpss/wid_thread", \
				local,vrrid, ".pid");
#endif
	if(!opened){
		
		fd = open(pidPath, O_RDWR|O_CREAT);
		wid_syslog_info("fd=%d,%s,path:%s\n",fd,__func__,pidPath);
		if(fd < 0){
			wid_syslog_err("<err>,%s\n",__func__);
			return ;
		}
		opened = 1;
	}
		
	if(id != 0){
		sprintf(idbuf,"HANSI[%d]%s[%d]",vrrid,name,id);
	}else{
		sprintf(idbuf,"HANSI[%d]%s",vrrid,name);
	}

	time_ptr = wid_get_current_time();
	strcpy(pidBuf, time_ptr);
	temp_ptr = (char *)(pidBuf + strlen(time_ptr) -1);
	*temp_ptr = ' ';
	temp_ptr++;
	
	//fd = open(pidPath, O_RDWR|O_CREAT);
	myPid = getpid();	
	sprintf(temp_ptr,"%s-%d\n",idbuf,myPid);
	//printf("11pidBuf:%s\n",pidBuf);
	write(fd, pidBuf, strlen(pidBuf));
	//printf("22pidBuf:%s\n",pidBuf);
	//close(fd);
	return;
}

void wid_syslog_hn(int level,char *iden,char *fmt,...)
{
	char ident[256] = {0};
	memcpy(ident,iden,strlen(iden));
	/*va_list pptr;
	va_start(pptr,iden);
	vsprintf(ident,iden,pptr);
	va_end(pptr);*/
	char buf[2048] = {0};
	va_list ptr;
	va_start(ptr,fmt);
	vsprintf(buf,fmt,ptr);
	va_end(ptr);
	openlog(ident, 0,LOG_DAEMON);
    syslog(level|LOG_LOCAL3,buf);
    closelog();
}

/* For new format of syslog 2013-07-29 */
void wid_syslog_auteview(int level,int type,void *parameter,int Rcode)
{

	WID_WTP *AP = (WID_WTP*)parameter;
	if(AP == NULL)
		return;
	switch(type){
		case AP_UP:
			syslog(level|LOG_LOCAL7,"AP_UP "DS_STRUCTURE_ALL"[AP"AUTELANID" "LOG_MAC" "LOG_NAME" wtpid=\"%d\" seid=\"%s\" model=\"%s\" "LOG_IP_STR" "LOG_RADIOS"][AC"AUTELANID" "LOG_IP_V4" "LOG_IF"]",
				slotid,vrrid,MAC2STR(AP->WTPMAC),AP->WTPNAME,AP->WTPID,AP->WTPSN,AP->WTPModel,AP->WTPIP,AP->RadioCount,IPSTRINT(gWID_AC_MANAGEMENT_IP),AP->BindingIFName);
			break;
		case AP_DOWN:
			syslog(level|LOG_LOCAL7,"AP_DOWN "DS_STRUCTURE_ALL"[AP"AUTELANID" "LOG_MAC" wtpid=\"%d\"][REASON"AUTELANID" "LOG_CODE"]",
				slotid,vrrid,MAC2STR(AP->WTPMAC),AP->WTPID,"APD",Rcode);
			break;
		default:
			break;
	}
}

