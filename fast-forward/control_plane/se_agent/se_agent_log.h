#ifndef _SE_AGENT_LOG_H_
#define _SE_AGENT_LOG_H_


#include <linux/tipc.h>
enum {
	SYSLOG_DBG_DEF     = 0x6,	/* default value*/
	SYSLOG_DBG_DBG     = 0x1,	/*debug */
	SYSLOG_DBG_INFO    = 0x2,	/*info*/
	SYSLOG_DBG_ERR     = 0x4,	/* error*/
	SYSLOG_DBG_ALL 	   = 0xFF	/*all*/
};
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
void se_agent_log_config_debug_level(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);

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
);

/**********************************************************************************
 *	se_agent_syslog_err
 * 
 *	output the daemon error  info to /var/log/system.log
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
);

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
);

#endif

