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
* se_agent_log.c
*
* MODIFY:
*		
*
* CREATOR:
*		
*
* DESCRIPTION:
*		APIs used in se_agent  for logging process.
*
* DATE:
*		07/28/2011
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.0 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>

#include "cvmx.h"

#include "se_agent.h"





#define  SE_AGENT_SYSLOG_BUFFER_SIZE  4096

unsigned int se_agent_log_status = 0; /* syslog opened or not: 0 - not opened, 1 - open */
unsigned int se_agent_log_level = SYSLOG_DBG_DEF;

char *se_agent_log_ident = "se_agent";

/**********************************************************************************
 *	se_agent_log_config_debug_level
 * 
 *	set se_agent debug level
 *
 *  INPUT:
 *		buf			  ->       pointer of store data buffer
 *          client_addr     ->        client socket address
 *		len 			  ->       the length of client socket address
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
void se_agent_log_config_debug_level(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	unsigned int current = 0;
	unsigned int level = 0;
	unsigned int enable = 0;
	se_interative_t *se_buf = NULL;
	int ret = -1;
	if(NULL == buf || NULL == client_addr )
	{
		se_agent_syslog_err("se_agent_log_config_debug_level parmter is error\n");
		return;
	}
	se_buf = (se_interative_t *)buf;
	level = (unsigned int)se_buf->fccp_cmd.fccp_data.agent_dbg.level;
	enable = (unsigned int)se_buf->fccp_cmd.fccp_data.agent_dbg.enable;
	current = se_agent_log_level;
	if(enable) {
		current |= level;
	}
	else {
		current &= ~level;
	}

	if(!se_agent_log_status) {
		openlog(se_agent_log_ident, LOG_PID, LOG_DAEMON);
		se_agent_log_status = 1;
	}

	syslog(LOG_DEBUG,"se_agent log level change %d -> %d!\n", se_agent_log_level, current);
	se_agent_log_level = current;
	return;	
}

/**********************************************************************************
 *	se_agent_syslog_dbg
 * 
 *	output the daemon debug info to /var/log/system.log
 *
 *  INPUT:
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
void se_agent_syslog_dbg
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[SE_AGENT_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_DBG & se_agent_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsprintf(buf, "se_agent", ptr);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!se_agent_log_status) {
		openlog(se_agent_log_ident, LOG_PID, LOG_DAEMON);
		se_agent_log_status = 1;
	}

	syslog(LOG_DEBUG,buf);

	return;	
}

/**********************************************************************************
 *	se_agent_syslog_err
 * 
 *	output the daemon debug info to /var/log/system.log
 *
 *  INPUT:
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
void se_agent_syslog_err
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[SE_AGENT_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_ERR & se_agent_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsprintf(buf, "se_agent", ptr);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!se_agent_log_status) {
		openlog(se_agent_log_ident, LOG_PID, LOG_DAEMON);
		se_agent_log_status = 1;
	}

	syslog(LOG_ERR,buf);

	return;	
}

/**********************************************************************************
 *	se_agent_syslog_info
 * 
 *	output the daemon informations to /var/log/system.log
 *
 *  INPUT:
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
void se_agent_syslog_info
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[SE_AGENT_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_INFO & se_agent_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsprintf(buf, "se_agent", ptr);
	vsprintf(buf,format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!se_agent_log_status) {
		openlog(se_agent_log_ident, 0, LOG_DAEMON);
		se_agent_log_status = 1;
	}

	syslog(LOG_INFO,buf);

	return;	
}

#ifdef __cplusplus
}
#endif

