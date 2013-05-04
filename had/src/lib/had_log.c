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
* had_log.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in HAD module for syslog.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


#include "had_log.h"

unsigned  int	vrrp_log_level = VRRP_LOG_DEF | VRRP_LOG_ERR | VRRP_LOG_EVT;

unsigned int vrrp_log_close = 1;

int g_vrrp_pid_fd = VRRP_FD_INIT;

extern unsigned int global_current_instance_no;
extern pthread_mutex_t IoMutex;

/**********************************************************************************
 *	vrrp_syslog_error
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_error
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	#if 0 /* no filter for LOG_ERR level */
	if(0 == (VRRP_LOG_ERR& vrrp_log_level)) {
		return;
	}
	#endif

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}


/**********************************************************************************
 *	vrrp_syslog_warning
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_warning
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_WAR& vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}

/**********************************************************************************
 *	vrrp_syslog_dbg
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_dbg
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_DBG& vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}



/**********************************************************************************
 *	vrrp_syslog_event
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_event
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	#if 0/* no filter for LOG_NOTICE level */
	if(0 == (VRRP_LOG_EVT & vrrp_log_level)) {
		return;
	}
	#endif

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}





/**********************************************************************************
 *	vrrp_syslog_protocol
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_protocol
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_PROTOCOL & vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}


/**********************************************************************************
 *	vrrp_syslog_packet_send
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_send
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_PKT_SED & vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}





/**********************************************************************************
 *	vrrp_syslog_packet_receive
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_receive
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_PKT_REV & vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}



/**********************************************************************************
 *	vrrp_syslog_packet_all
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_packet_all
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// event not triggered
	if(0 == (VRRP_LOG_PKT_ALL & vrrp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_DEBUG, "inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}

/**********************************************************************************
 *	vrrp_syslog_info
 * 
 *	output the daemon debug info to /var/log/daemon.log
 *
 *  INPUT:
 * 	 	event - 
 *		format - 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void vrrp_syslog_info
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[2048] = {0};
	char *ident = "VRRP";

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(vrrp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,"inst[%d] %s\n", global_current_instance_no, buf);

	return;	
}
#if 0
void VRRP_LOG(unsigned int level,char * format,...)
{
	char buf[2048] = "";
	va_list argP;
	int i, ret;
	
	if(level & vrrp_log_level)
	{
		va_start(argP,format);	
		i = vsprintf(buf, format, argP);
		va_end(argP);

		ret = printf(buf);
	}
	else
		return;

	/*if(level < vrrp_log_level)
		return;

	va_start(argP,format);	
	i = vsprintf(buf, format, argP);
	va_end(argP);

	ret = printf(buf);*/



}
#endif

int set_debug_value(unsigned int val_mask)
{
	vrrp_log_level |= val_mask;
	return 0;
}

int set_no_debug_value(unsigned int val_mask)
{
	vrrp_log_level &= ~val_mask;
	return 0;
}

/*
 *	write current thread pid to pid file
 */
void vrrp_debug_tell_whoami
(
	char * myName,
	int isLast
)
{
	char pidBuf[VRRP_PID_BUFFER_LEN] = {0}, pidPath[VRRP_PID_PATH_LEN] = {0};
	pid_t myPid = 0;
	
	if(!myName) {
		vrrp_syslog_error("tell pid to pid file with null pointer error!\n");
		return;
	}

	sprintf(pidPath,"%s%s%d%s", VRRP_PID_FILE_PATH, VRRP_PID_FILE_PREFIX, \
				global_current_instance_no, VRRP_PID_FILE_SUFFIX);
		
	if(VRRP_FD_INIT == g_vrrp_pid_fd){	
		char pidChmod[VRRP_PID_BUFFER_LEN] = {0};
		g_vrrp_pid_fd = open(pidPath, O_RDWR|O_CREAT|O_APPEND);
		if(VRRP_FD_INIT == g_vrrp_pid_fd) {
			vrrp_syslog_error("%s tell pid but open file failed!\n", myName);
			return;
		}
		sprintf(pidChmod, "chmod 644 %s", pidPath);
		system(pidChmod);/* chmod for other user */
	}

	myPid = getpid();
	
	sprintf(pidBuf,"instance %d %s has pid %d\n", global_current_instance_no, myName, myPid);
	pthread_mutex_lock(&IoMutex);
	write(g_vrrp_pid_fd, pidBuf, strlen(pidBuf));
	pthread_mutex_unlock(&IoMutex);
	vrrp_syslog_dbg("current %d %s tell pid %d to %s%s\n",  \
				global_current_instance_no, myName, myPid, pidPath, isLast ? " last":"");

	/* close pid file by last teller */
	if(isLast) {
		close(g_vrrp_pid_fd);
		g_vrrp_pid_fd = VRRP_FD_INIT;
	}

	return;
}
#ifdef __cplusplus
}
#endif

