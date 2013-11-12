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
* dhcp_snp_log.c
*
* MODIFY:
*		by <qinhs@autelan.com> 
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		APIs used in DHCP snooping for logging process.
*
* DATE:
*		04/16/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
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
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sysdef/npd_sysdef.h>

#include "dhcp_snp_log.h"

/* syslog opened or not: 0 - not opened, 1 - open */
unsigned int dhcpsnp_log_status = 0; 
unsigned int dhcpsnp_log_level = SYSLOG_DBG_EVT | SYSLOG_DBG_ERR | SYSLOG_DBG_WAR;

char *dhcpsnp_log_ident = "dhcpsnp";

/* use for debug log */
char static_buffer[DHCP_SNP_STATIC_BUFFER_SIZE];
int static_buffer_len = sizeof(static_buffer);


/**********************************************************************************
 *	dhcp_snp_log_config_debug_level
 * 
 *	set dhcp snooping debug level
 *
 *  INPUT:
 *		level  - debug level value 
 *		enable - enable or disable flag
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 NULL
 * 	 
 *
 **********************************************************************************/
int dhcp_snp_log_config_debug_level
(
	unsigned int level,
	unsigned int enable
)
{
	unsigned int current;

	current = dhcpsnp_log_level;

	if(enable) {
		current |= level;
	}
	else {
		current &= ~level;
	}

	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_DEBUG,"dhcp snp log level change %d -> %d!\n", dhcpsnp_log_level, current);
	dhcpsnp_log_level = current;
	return 0;	
}

/**********************************************************************************
 *	log_debug
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
void log_debug
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_DBG & dhcpsnp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_DEBUG,buf);

	return;	
}


/**********************************************************************************
 *	log_info
 * 
 *	output the daemon information info to /var/log/system.log
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
void log_info
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_EVT & dhcpsnp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_INFO,buf);

	return;	
}


/**********************************************************************************
 *	dhcp_snp_syslog_err
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
void log_error
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_ERR & dhcpsnp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_ERR,buf);

	return;	
}

/**********************************************************************************
 *	log_warn
 * 
 *	output the daemon warnings to /var/log/system.log
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
void log_warn
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_WAR & dhcpsnp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_WARNING,buf);

	return;	
}

/**********************************************************************************
 *	syslog_ax_dhcp_snp_pkt_rx
 * 
 *	output the daemon warnings to /var/log/system.log
 *
 *	INPUT:
 *		format - 
 *	
 *	OUTPUT:
 *	 NULL
 *
 *	RETURN:
 *	 NULL
 *	 
 *
 **********************************************************************************/
void syslog_ax_dhcp_snp_pkt_rx
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* event not triggered*/
	if(0 == (SYSLOG_DBG_PKT_REV & dhcpsnp_log_level)) {
		return;
	}

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	syslog(LOG_DEBUG,buf);

	return; 
}

/*****************************************************************************
 *	npd_syslog7_notice
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
 ****************************************************************************/
void dhcp_snp_syslog7_notice
(
	char *format,
	...
)
{
	va_list ptr;
	char buf[NPD_SYSLOG_BUFFER_SIZE] = {0};

	/* ident null or format message null*/
	if(!format) {
		return;
	}
	
	/* buffering log*/
	va_start(ptr, format);
	vsnprintf(buf,sizeof(buf),format,ptr);
	va_end(ptr);

	/* assure log file open only once globally*/
	if(!dhcpsnp_log_status) {
		openlog(dhcpsnp_log_ident, 0, LOG_DAEMON);
		dhcpsnp_log_status = 1;
	}

	/* write log*/
	syslog(LOG_LOCAL7 | LOG_NOTICE, buf);

	return;	
}


inline int dhcp_snp_log_dbug(void)
{
	return ((SYSLOG_DBG_DBG & dhcpsnp_log_level));
}



/*****************************************************************************
 *	mac2str
 * 
 *	mac to strig
 *
 *  INPUT:
 *		haddr - mac address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 static_buferr - mac string
 * 	 NULL
 *
 ****************************************************************************/
inline char *mac2str(unsigned char *haddr)
{
	int len = sizeof("00:11:22:33:44:55\0");
	
	if (!haddr) {
		return NULL;
	}

	memset(static_buffer, 0, len > static_buffer_len ? static_buffer_len : len);
	snprintf(static_buffer, sizeof(static_buffer), 
		"%02x:%02x:%02x:%02x:%02x:%02x", 
		haddr[0], haddr[1], haddr[2],
		haddr[3], haddr[4], haddr[5]);
	
	return static_buffer;
}

/*****************************************************************************
 *	u32ip2str
 * 
 *	IPv4 address to string (EXP: 0x0a01010a -> 10.1.1.10)
 *
 *  INPUT:
 *		u32_ipaddr - IPv4 address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 char * - ipv4 address string
 * 	 NULL - failed
 *
 ****************************************************************************/
inline char *u32ip2str(unsigned int u32_ipaddr)
{
	struct in_addr inaddr;

	inaddr.s_addr = u32_ipaddr;

	return inet_ntoa(inaddr);
}

/*****************************************************************************
 *	u128ip2str
 * 
 *	IPv4 address to string (EXP: 0x0a01010a -> 10:1::1:10)
 *
 *  INPUT:
 *		u128_ipaddr - IPv6 address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 char * - ipv6 address string
 * 	 NULL - failed
 *
 ****************************************************************************/
const char *u128ip2str(unsigned char* u128_ipaddr)
{
	static char
		pbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	
	return inet_ntop(AF_INET6, u128_ipaddr, pbuf, sizeof(pbuf));
	log_fatal(" u128ip2str ->   piaddr():error.");
		/* quell compiler warnings */
	return NULL;
}


#ifdef __cplusplus
}
#endif
