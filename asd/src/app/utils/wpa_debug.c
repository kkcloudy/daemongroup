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
* AsdWpaDebug.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include "includes.h"

#include "common.h"
#include <syslog.h>
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"


#ifdef ASD_DEBUG_FILE
static FILE *out_file = NULL;
#endif /* ASD_DEBUG_FILE */
int wpa_debug_level = MSG_NOTICE;
int wpa_debug_show_keys = 1;//zhanglei change 0 to 1
int wpa_debug_timestamp = 0;

int gASDDEBUG = 0;
int gasdPRINT = 0;
//qiuchen
unsigned char gASDLOGDEBUG = 0;
unsigned long gASD_AC_MANAGEMENT_IP = 0;
extern int slotid;
extern int vrrid;
void asd_syslog_emerg(char *format,...)
{
    char *ident = "asd_eme";
	char buf[2048] = {0};
	
	//openlog(ident, 0, LOG_DAEMON); 
	
	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_EMERG,buf);

	
	//closelog();
	
}
void asd_syslog_alert(char *format,...)
	
{
    char *ident = "asd_ale";
	char buf[2048] = {0};
	
	//openlog(ident, 0, LOG_DAEMON); 

	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_ALERT,buf);

	
	//closelog();
	
}
void asd_syslog_crit(char *format,...)
	
{
    char *ident = "asd_cri";
	char buf[2048] = {0};

	
	//openlog(ident, 0, LOG_DAEMON); 
	
	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_CRIT,buf);

	
	//closelog();

}
void asd_syslog_err(char *format,...)
	
{
    char *ident = "asd_err";
	char buf[2048] = {0};

	
	//openlog(ident, 0, LOG_DAEMON); 
	
	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_ERR,buf);

	
	//closelog();

}
void asd_syslog_warning(char *format,...)
	
{
    char *ident = "asd_war";
	char buf[2048] = {0};

	
	//openlog(ident, 0, LOG_DAEMON); 
	
	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_WARNING,buf);

	
	//closelog();

}
void asd_syslog_notice(char *format,...)
	
{
    char *ident = "asd_not";
	char buf[2048] = {0};


	//openlog(ident, 0, LOG_DAEMON); 

	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_NOTICE,buf);

	
	//closelog();
	
}
void asd_syslog_info(char *format,...)
	
{
    char *ident = "asd_inf";
	char buf[2048] = {0};

	//openlog(ident, 0, LOG_DAEMON); 
	
	va_list ptr;
	va_start(ptr,format);
	sprintf(buf,"%s ",ident);
	//vsprintf(buf+8,format,ptr);
	vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
	va_end(ptr);
	syslog(LOG_INFO,buf);

	
	//closelog();

}
void asd_syslog_debug(int type, char *format,...)
	
{
    char *ident = "asd_deb";
	char buf[2048] = {0};

	
	if( gASDDEBUG&type ) {
		//openlog(ident, 0, LOG_DAEMON); 
		va_list ptr;
		va_start(ptr,format);
		sprintf(buf,"%s ",ident);
		//vsprintf(buf+8,format,ptr);
		vsnprintf(buf+8,sizeof(buf)-8,format,ptr);
		va_end(ptr);
		syslog(LOG_DEBUG,buf);
		//closelog();
	}
	
}

#ifndef ASD_NO_STDOUT_DEBUG

void wpa_debug_print_timestamp(void)
{
	struct os_time tv;

	if (!wpa_debug_timestamp)
		return;

	os_get_time(&tv);
#ifdef ASD_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%ld.%06u: ", (long) tv.sec,
			(unsigned int) tv.usec);
	} else
#endif /* ASD_DEBUG_FILE */
	printf("%ld.%06u: ", (long) tv.sec, (unsigned int) tv.usec);
}

/**
 * asd_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void asd_printf(int type,int level, char *fmt, ...)
{
	char buf[2048]={0};
	
	if (level < wpa_debug_level)
		return;
	if ((MSG_DEBUG == level) && (!(gASDDEBUG & type))) {
		return ;
	}
	
	va_list ap;
	va_start(ap, fmt);

/*	
	if (level >= wpa_debug_level) {
		wpa_debug_print_timestamp();
#ifdef ASD_DEBUG_FILE
		if (out_file) {
			vfprintf(out_file, fmt, ap);
			fprintf(out_file, "\n");
		} else {
#endif 
		vprintf(fmt, ap);
		printf("\n");
#ifdef ASD_DEBUG_FILE
		}
#endif 
	}
*/
//	if (gasdPRINT)
//		vprintf(fmt, ap);

	//vsprintf(buf,fmt,ap);
	vsnprintf(buf, sizeof(buf), fmt, ap);//Qc change it to add slotid and vrrid
	char sbuf[64+2048] = {0};
	sprintf(sbuf,"[%d-%d]",slotid,vrrid);
	memcpy(sbuf+strlen(sbuf),buf,sizeof(buf));
	switch(level) {
		case MSG_EMERG:
			asd_syslog_emerg(sbuf);
			break;
		case MSG_ALERT:
			asd_syslog_alert(sbuf);
			break;
		case MSG_CRIT:
			asd_syslog_crit(sbuf);
			break;
		case MSG_ERROR:
			asd_syslog_err(sbuf);
			break;
		case MSG_WARNING:
			asd_syslog_warning(sbuf);
			break;
		case MSG_NOTICE:
			asd_syslog_notice(sbuf);
			break;
		case MSG_INFO:
			asd_syslog_info(sbuf);
			break;
		case MSG_DEBUG:
		case MSG_MSGDUMP:
			asd_syslog_debug(type,sbuf);
			break;
		default:
			break;
	}		

	va_end(ap);
}

//qiuchen add it for Henan Mobile log
void asd_syslog_h(int level,char *iden,char *fmt,...)
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
	openlog(ident, 0, LOG_DAEMON);
    syslog(level|LOG_LOCAL3,buf);
    closelog();
}

static void _wpa_hexdump(int level, const char *title, const u8 *buf,
			 size_t len, int show)
{
	size_t i;
	char format[2048]={0};
	char buffer[512]={0};

	if (level < wpa_debug_level)
		return;
	wpa_debug_print_timestamp();
#ifdef ASD_DEBUG_FILE
	if (out_file) {
		fprintf(out_file, "%s - hexdump(len=%lu):",
			title, (unsigned long) len);
		if (buf == NULL) {
			fprintf(out_file, " [NULL]");
		} else if (show) {
			for (i = 0; i < len; i++)
				fprintf(out_file, " %02x", buf[i]);
		} else {
			fprintf(out_file, " [REMOVED]");
		}
		fprintf(out_file, "\n");
	} else {
#endif /* ASD_DEBUG_FILE */
	//sprintf(format,"%s - hexdump(len=%lu):", title, (unsigned long) len);
	#if 0
	snprintf(format,sizeof(format),"%s - hexdump(len=%lu):", title, (unsigned long) len);
	#else
	asd_printf(ASD_WPA,MSG_DEBUG,"%s - hexdump(len=%d):\n",title,(unsigned int)len);
	#endif
	if (buf == NULL) {
		strcat(format," [NULL]");
	} else if (show) {
	    /* bugfix for format[] overflow if len is too long, zhangdi@autelan.com 2013-11-05 */
		if((len*3) > 2000)
		{
        	asd_printf(ASD_WPA,MSG_NOTICE,"len %d is too long, just dump %d:\n",(unsigned int)len,2000/3);
			len = 2000/3;
		}	
		for (i = 0; i < len; i++) {
			sprintf(buffer," %02x", buf[i]);
			strncat(format,buffer,sizeof(buffer));
		}
	} else {
		strcat(format," [REMOVED]");
	}
	strcat(format,"\n");
#ifdef ASD_DEBUG_FILE
	}
#endif /* ASD_DEBUG_FILE */
	asd_printf(ASD_WPA,level,format);

}

void wpa_hexdump(int level, const char *title, const u8 *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, 1);
}


void wpa_hexdump_key(int level, const char *title, const u8 *buf, size_t len)
{
	_wpa_hexdump(level, title, buf, len, wpa_debug_show_keys);
}


static void _wpa_hexdump_ascii(int level, const char *title, const u8 *buf,
			       size_t len, int show)
{
	size_t i, llen;
	const u8 *pos = buf;
	const size_t line_len = 16;
	char format[2048]={0};
	char buffer[512]={0};
	if(len > 512){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s-len is too long:%d\n",__func__,len);
		return;
	}
	if (level < wpa_debug_level)
		return;
	wpa_debug_print_timestamp();
#ifdef ASD_DEBUG_FILE
	if (out_file) {
		if (!show) {
			fprintf(out_file,
				"%s - hexdump_ascii(len=%lu): [REMOVED]\n",
				title, (unsigned long) len);
			return;
		}
		if (buf == NULL) {
			fprintf(out_file,
				"%s - hexdump_ascii(len=%lu): [NULL]\n",
				title, (unsigned long) len);
			return;
		}
		fprintf(out_file, "%s - hexdump_ascii(len=%lu):\n",
			title, (unsigned long) len);
		while (len) {
			llen = len > line_len ? line_len : len;
			fprintf(out_file, "    ");
			for (i = 0; i < llen; i++)
				fprintf(out_file, " %02x", pos[i]);
			for (i = llen; i < line_len; i++)
				fprintf(out_file, "   ");
			fprintf(out_file, "   ");
		#if 0
			for (i = 0; i < llen; i++) {
				if (isprint(pos[i]))
					fprintf(out_file, "%c", pos[i]);
				else
					fprintf(out_file, "_");
			}
			for (i = llen; i < line_len; i++)
				fprintf(out_file, " ");
		#endif
			fprintf(out_file, "\n");
			pos += llen;
			len -= llen;
		}
	} else {
#endif /* ASD_DEBUG_FILE */
	if (!show) {
		sprintf(format,"%s - hexdump_ascii(len=%lu): [REMOVED]\n",
		       title, (unsigned long) len);
		asd_printf(ASD_WPA,MSG_DEBUG,format);
		printf("%s : %s\n",__func__,format);
		return;
	}
	if (buf == NULL) {
		sprintf(format,"%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		asd_printf(ASD_WPA,MSG_DEBUG,format);
		printf("%s : %s\n",__func__,format);
		return;
	}
	//sprintf(format,"%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	snprintf(format,sizeof(format),"%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		strcat(format,"    ");
		for (i = 0; i < llen; i++) {
			sprintf(buffer," %02x", pos[i]);
			strncat(format,buffer,sizeof(buffer));
		}
		for (i = llen; i < line_len; i++)
			strcat(format,"   ");
		strcat(format,"   ");
	#if 0
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i])) {
				sprintf(buffer,"%c", pos[i]);
				strncat(format,buffer,sizeof(buffer));
			}
			else
				strcat(format,"_");
		}
		for (i = llen; i < line_len; i++)
			strcat(format," ");
	#endif
		strcat(format,"\n");
		pos += llen;
		len -= llen;
	}
#ifdef ASD_DEBUG_FILE
	}
#endif /* ASD_DEBUG_FILE */
	asd_printf(ASD_WPA,level,format);
}


void wpa_hexdump_ascii(int level, const char *title, const u8 *buf, size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, 1);
}


void wpa_hexdump_ascii_key(int level, const char *title, const u8 *buf,
			   size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, wpa_debug_show_keys);
}


int wpa_debug_open_file(const char *path)
{
#ifdef ASD_DEBUG_FILE
	if (!path)
		return 0;
	out_file = fopen(path, "a");
	if (out_file == NULL) {
		asd_printf(ASD_WPA,MSG_ERROR, "wpa_debug_open_file: Failed to open "
			   "output file, using standard output");
		return -1;
	}
#ifndef _WIN32
	setvbuf(out_file, NULL, _IOLBF, 0);
#endif /* _WIN32 */
#endif /* ASD_DEBUG_FILE */
	return 0;
}


void wpa_debug_close_file(void)
{
#ifdef ASD_DEBUG_FILE
	if (!out_file)
		return;
	fclose(out_file);
	out_file = NULL;
#endif /* ASD_DEBUG_FILE */
}

#endif /* ASD_NO_STDOUT_DEBUG */


#ifndef ASD_NO_WPA_MSG
static wpa_msg_cb_func wpa_msg_cb = NULL;

void wpa_msg_register_cb(wpa_msg_cb_func func)
{
	wpa_msg_cb = func;
}


void wpa_msg(void *ctx, int level, char *fmt, ...)
{
	va_list ap;
	char *buf;
	const int buflen = 2048;
	int len;

	buf = os_zalloc(buflen);
	if (buf == NULL) {
		asd_printf(ASD_WPA,MSG_ERROR, "wpa_msg: Failed to allocate message "
			   "buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	asd_printf(ASD_WPA,level, "%s", buf);
	if (wpa_msg_cb)
		wpa_msg_cb(ctx, level, buf, len);
	os_free(buf);
}
#endif /* ASD_NO_WPA_MSG */


#ifndef ASD_NO_asd_LOGGER
static asd_logger_cb_func asd_logger_cb = NULL;

void asd_logger_register_cb(asd_logger_cb_func func)
{
	asd_logger_cb = func;
}


void asd_logger(void *ctx, const u8 *addr, unsigned int module, int level,
		    const char *fmt, ...)
{
	va_list ap;
	char *buf;
	const int buflen = 2048;
	int len;

	buf = os_zalloc(buflen);
	if (buf == NULL) {
		asd_printf(ASD_WPA,MSG_ERROR, "asd_logger: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	if (asd_logger_cb)
		asd_logger_cb(ctx, addr, module, level, buf, len);
	else
		asd_printf(ASD_WPA,MSG_DEBUG, "asd_logger: %s", buf);
	os_free(buf);
}
#endif /* ASD_NO_asd_LOGGER */

/*yjl copy from aw3.1.2 for local forwarding.2014-2-28**********************************************************/
/****************************************************************************
 * asd_op2str
 * 
 * 	operate code to string
 *
 *	INPUT:
 *		opcode	- operate code
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		char *  - operate string
 *
 *		
 ****************************************************************************/
char *asd_op2str(unsigned int opcode)
{
	static char buf[64] = {0};
	
	switch (opcode) 
	{
		case WID_ADD:				/* 0 */
			return "WID_ADD";
		case WID_DEL:				/* 1 */
			return "WID_DEL";
		case WID_MODIFY:			/* 2 */
			return "WID_MODIFY";
		case STA_INFO:				/* 3 */
			return "STA_INFO";
		case RADIO_INFO:			/* 4 */
			return "RADIO_INFO";
		case WTP_DENEY_STA:			/* 5 */
			return "WTP_DENEY_STA";
		case STA_COME:				/* 6 */
			return "STA_COME";
		case STA_LEAVE:				/* 7 */
			return "STA_LEAVE";
		case VERIFY_INFO:			/* 8 */
			return "VERIFY_INFO";
		case VERIFY_FAIL_INFO:		/* 9 */
			return "VERIFY_FAIL_INFO";
		case WTP_DE_DENEY_STA:		/* 10 */
			return "WTP_DE_DENEY_STA";
			
		case BSS_INFO:				/* 11 */
			return "BSS_INFO";
		case ASSOC_FAIL_INFO:		/* 12 */
			return "ASSOC_FAIL_INFO";
		case JIANQUAN_FAIL_INFO:	/* 13 */
			return "JIANQUAN_FAIL_INFO";
		case CHANNEL_CHANGE_INFO:	/* 14 */
			return "CHANNEL_CHANGE_INFO";
		case WID_UPDATE:			/* 15 */
			return "WID_UPDATE";
		case WID_CONFLICT:			/* 16 */
			return "WID_CONFLICT";
		case WID_ONE_UPDATE:		/* 17 */
			return "WID_ONE_UPDATE";
		case TRAFFIC_LIMIT:			/* 18 */
			return "TRAFFIC_LIMIT";
		case WIDS_INFO:				/* 19 */
			return "WIDS_INFO";
		case WIDS_SET:				/* 20 */
			return "WIDS_SET";

		case WAPI_INVALID_CERT:		/* 21 */
			return "WAPI_INVALID_CERT";
		case WAPI_CHALLENGE_REPLAY:	/* 22 */
			return "WAPI_CHALLENGE_REPLAY";
		case WAPI_MIC_JUGGLE:		/* 23 */
			return "WAPI_MIC_JUGGLE";
		case WAPI_LOW_SAFE_LEVEL:	/* 24 */
			return "WAPI_LOW_SAFE_LEVEL";
		case WAPI_ADDR_REDIRECTION:	/* 25 */
			return "WAPI_ADDR_REDIRECTION";
		case OPEN_ROAM:				/* 26 */
			return "OPEN_ROAM";
		case VRRP_IF:				/* 27 */
			return "VRRP_IF";
		case STA_WAPI_INFO:			/* 28 */
			return "STA_WAPI_INFO";
		case CANCEL_TRAFFIC_LIMIT:	/* 29 */
			return "CANCEL_TRAFFIC_LIMIT";
		case WTP_STA_CHECK:			/* 30 */
			return "WTP_STA_CHECK";

		case WID_WIFI_INFO:			/* 31 */
			return "WID_WIFI_INFO";
		case ASD_AUTH:				/* 32 */
			return "ASD_AUTH";
		case ASD_DEL_AUTH:			/* 33 */
			return "ASD_DEL_AUTH";
		case EAG_AUTH:				/* 34 */
			return "EAG_AUTH";
		case EAG_DEL_AUTH:			/* 35 */
			return "EAG_DEL_AUTH";
		case BSS_UPDATE:			/* 36 */
			return "BSS_UPDATE";
		case EAG_MAC_AUTH:			/* 37 */
			return "EAG_MAC_AUTH";
		case EAG_MAC_DEL_AUTH:		/* 38 */
			return "EAG_DEL_MAC_AUTH";
		case ASD_MAC_AUTH:			/* 39 */
			return "ASD_MAC_AUTH";
		case ASD_MAC_DEL_AUTH:		/* 40 */
			return "ASD_MAC_DEL_AUTH";

		case STA_FLOW_CHECK:		/* 41 */
			return "STA_FLOW_CHECK";
		case DHCP_IP:				/* 42 */
			return "DHCP_IP";
		case IDLE_STA_DEL:			/* 43 */
			return "IDLE_STA_DEL";
		case MAC_LIST_ADD:			/* 44 */
			return "MAC_LIST_ADD";
		case STA_LEAVE_REPORT:		/* 45 */
			return "STA_LEAVE_REPORT";
		case EAG_NTF_ASD_STA_INFO:	/* 46 */
			return "EAG_NTF_ASD_STA_INFO";			
		case STA_PORTAL_AUTH:		/* 47 */
			return "STA_PORTAL_AUTH";			
		case STA_PORTAL_DEAUTH:		/* 48 */
			return "STA_PORTAL_DEAUTH";
			/*
		case WPA_PMK:				//49 
			return "WPA_PMK";			
		case KEY_NEGOTI: 			// 50 
			return "KEY_NEGOTI";
			*/

		default:
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf)-1, "unkown operate %d", opcode);
			
			return buf;
	}
	/* never get here */
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

char *mac2str(unsigned char *haddr)
{
	static int count = 0;
	static unsigned char buffer[STATIC_BUFFER_SIZE][MAX_MAC_STRING_LEN];
	memset(buffer,0,sizeof(buffer));
	int len = sizeof("00:11:22:33:44:55\0");	
	unsigned char *tmp = NULL;

	if (NULL == haddr) 
	{
		return NULL;
	}

	/* Coverity: CID: 16462 Error-Type: Out-of-bounds read */
	count++;

	if (count >= STATIC_BUFFER_SIZE)
	{
		count = 0;
	}
	
	tmp = (unsigned char *)&(buffer[count][0]);
	
	memset(tmp, 0, len);
	if (NULL != haddr)
	{
		snprintf((char *)tmp, MAX_MAC_STRING_LEN, "%02X:%02X:%02X:%02X:%02X:%02X",
				 haddr[0], haddr[1], haddr[2],
				 haddr[3], haddr[4], haddr[5]);
	}
	
	return (char *)tmp;
}	

/**********************************************************************
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
 **********************************************************************/
char *u32ip2str(unsigned int u32_ipaddr)
{	
#if 1
	struct in_addr inaddr;

	memset(&inaddr, 0, sizeof(struct in_addr));

	inaddr.s_addr = u32_ipaddr;

	return inet_ntoa(inaddr);
#else
	int len = sizeof("255.255.255.255\0");
	
	memset(static_buffer, 0, len);
	snprintf(static_buffer, sizeof(static_buffer), "%u.%u:%u.%u", 
		((u32_ipaddr >> 24) & 0xff), ((u32_ipaddr >> 16) & 0xff),
		((u32_ipaddr >> 8) & 0xff), ((u32_ipaddr >> 0) & 0xff));
	
	return static_buffer;

#endif
}
/*end******************************************yjl copy from aw3.1.2 for local forwarding.2014-2-28*/
