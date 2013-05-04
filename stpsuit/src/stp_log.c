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
* stp_log.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for syslog in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include "sysdef/npd_sysdef.h"
#include "stp_log.h"
#include <fcntl.h>

unsigned  int	stp_log_level = STP_LOG_ALL;
unsigned int stp_log_close = 1;
int stp_bootlog_fd = -1;
int stp_startup_end = 0;
void stp_syslog_emit
(
	char *buf
)
{
	if(!buf)
		return;

	/* bootlog is output to different file*/
	/* check if system already startup*/
	if(stp_bootlog_fd < 0) {
		stp_bootlog_fd = open(STP_SYSTEM_STARTUP_LOG_PATH, O_RDWR|O_CREAT|O_APPEND,0666);
	}
	
	if(stp_bootlog_fd > 0){
		if(0 == stp_startup_end){
			write(stp_bootlog_fd, buf, strlen(buf));
			return;
		}
	}
	/* write log*/
	syslog(LOG_DEBUG,buf);
	return; 
}


/**********************************************************************************
 *	stp_syslog_error
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
void stp_syslog_error
(
	char *format,
	...
)
{
	va_list ptr;
	/*static char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};*/
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_ERR& stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}


/**********************************************************************************
 *	stp_syslog_warning
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
void stp_syslog_warning
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_WAR& stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}

/**********************************************************************************
 *	stp_syslog_dbg
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
void stp_syslog_dbg
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_DBG& stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}



/**********************************************************************************
 *	stp_syslog_event
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
void stp_syslog_event
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_EVT & stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);
	return;	
}





/**********************************************************************************
 *	stp_syslog_protocol
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
void stp_syslog_protocol
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_PROTOCOL & stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);
	return;	
}


/**********************************************************************************
 *	stp_syslog_packet_send
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
void stp_syslog_packet_send
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_PKT_SED & stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}





/**********************************************************************************
 *	stp_syslog_packet_receive
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
void stp_syslog_packet_receive
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_PKT_REV & stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}



/**********************************************************************************
 *	stp_syslog_packet_all
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
void stp_syslog_packet_all
(
	char *format,
	...
)
{
	va_list ptr;
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	char *ident = "STP";

	// event not triggered
	if(0 == (STP_LOG_PKT_ALL & stp_log_level)) {
		return;
	}

	// ident null or format message null
	if(!ident || !format) {
		return;
	}

	// assure log file open only once globally
	if(stp_log_close) {
		openlog(ident, 0, LOG_DAEMON);
	}

	// put log
	va_start(ptr, format);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	stp_syslog_emit(buf);

	return;	
}

void STP_LOG(unsigned int level,char * format,...)
{
	static char buf[STP_SYSLOG_BUFFER_SIZE]= {0};
	va_list argP;
	int i, ret;
	
	if(level & stp_log_level)
	{
		va_start(argP,format);	
		i = vsprintf(buf, format, argP);
		va_end(argP);

		ret = printf(buf);
	}
	else
		return;

	/*if(level < stp_log_level)
		return;

	va_start(argP,format);	
	i = vsprintf(buf, format, argP);
	va_end(argP);

	ret = printf(buf);*/



}
#ifdef __cplusplus
}
#endif
