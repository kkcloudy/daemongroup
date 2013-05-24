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
* eag_redir.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* eag redir
*
*
*******************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "eag_conf.h"  //not need
#include "eag_interface.h"
#include "eag_dbus.h"
#include "eag_hansi.h"
#include "eag_time.h"
#include "eag_util.h"

#include "appconn.h"
#include "eag_redir.h"
#include "eag_portal.h"
#include "radius_packet.h"
#include "eag_ins.h"
#include "eag_statistics.h"
#include "eag_stamsg.h"
#include "eag_ipinfo.h"
#include "eag_wireless.h"
#include "wcpss/waw.h"

#define EAG_REDIR_BLKMEM_NAME		"eag_redir_blkmem"
#define EAG_REDIR_BLKMEM_BLKNUM		256
#define EAG_REDIR_BLKMEM_MAXNUM		16

extern int username_check_switch;

#define EAG_REDIR_READ_TIME_OUT		5
#define EAG_REDIR_WRITE_TIME_OUT	5

#define EAG_REDIR_HTTP_REQUEST_MATCH		1
#define EAG_REDIR_HTTP_REQUEST_NOT_MATCH	0

#define INTERNATIONAL_ROAMING_XML \
	"\r\n<HTML><BODY><H2>Browser error!</H2>Browser does not support redirects!</BODY>\r\n" \
	"<!--\r\n" \
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" \
	"<WISPAccessGatewayParam\r\n" \
	"  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n" \
	"  xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\">\r\n" \
	"<Proxy>\r\n" \
	"<MessageType>110</MessageType>\r\n" \
	"<NextURL>%s</NextURL>\r\n" \
	"<ResponseCode>200</ResponseCode>\r\n" \
	"</Proxy>\r\n" \
	"</WISPAccessGatewayParam>\r\n" \
	"-->\r\n" \
	"\r\n" \
	"</HTML>\r\n"

typedef struct eag_redirconn eag_redirconn_t;

struct eag_redir {
	int listen_fd;
	uint16_t redir_port;
	uint32_t local_ip;
	uint16_t local_port;

	int hansi_type;
	int hansi_id;

	eag_thread_master_t *master;
	eag_thread_t *t_accept;
	eag_blk_mem_t *conn_blkmem;
	struct list_head conn_head;

	/* max_redir_times/redir_check_interval = max_redir_times per second */
	int max_redir_times;
	int redir_check_interval;
	int force_dhcplease;
	
	struct portal_conf *portalconf;
	appconn_db_t *appdb;
	eag_portal_t *portal;
	eag_ins_t *eagins;
	eag_statistics_t *eagstat;
	eag_stamsg_t *stamsg;
	eag_dbus_t *eagdbus;
};

struct eag_redirconn {
	struct list_head node;
	int conn_fd;
	uint32_t sta_ip;
	eag_redir_t *redir;
	eag_thread_master_t *master;
	eag_thread_t *t_read;
	eag_thread_t *t_write;
	eag_thread_t *t_timeout;

	char redirurl[512];
	char ibuf[4096];
	uint32_t ibuflen;
	uint32_t recvlines;
	int recvdone;

	char request_url[1024];
	char request_host[256];
	char user_agent[256];
};

typedef enum {
	EAG_REDIR_SERV,
} eag_redir_event_t;

typedef enum {
	EAG_REDIRCONN_READ,
	EAG_REDIRCONN_WRITE,
} eag_redirconn_event_t;


static inline int 
build_wlanapmac(char *dst, unsigned char *src, int size)
{
	snprintf(dst, size, "%02X%02X%02X%02X%02X%02X", 
			src[0], src[1], src[2], src[3], src[4], src[5]);
	return 0;
}


static void 
eag_redir_event(eag_redir_event_t event,
						eag_redir_t *redir);

static void 
eag_redirconn_event(eag_redirconn_event_t event,
						eag_redirconn_t *redirconn);

static eag_redirconn_t *
eag_redirconn_new(eag_redir_t *redir, uint32_t user_ip)
{
	eag_redirconn_t *redirconn = NULL;

	if (NULL == redir) {
		eag_log_err("eag_redirconn_new input error");
		return NULL;
	}
	
	redirconn = eag_blkmem_malloc_item(redir->conn_blkmem);
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(redirconn, 0, sizeof(*redirconn));
	list_add_tail(&(redirconn->node), &(redir->conn_head));

	redirconn->conn_fd = -1;
	redirconn->sta_ip = user_ip;
	redirconn->redir = redir;
	redirconn->master = redir->master;

	eag_log_debug("eag_redir", "eag_redirconn_new ok");
	return redirconn;
}

static int
eag_redirconn_free(eag_redirconn_t *redirconn)
{
	eag_redir_t *redir = NULL;
	char ipstr[32] = "";
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_free input error");
		return -1;
	}
	redir = redirconn->redir;
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	eag_log_debug("eag_redir", "eag_redirconn free, staip %s",
		ipstr);
	
	if (NULL != redirconn->t_read) {
		eag_thread_cancel(redirconn->t_read);
		redirconn->t_read = NULL;
	}
	if (NULL != redirconn->t_write) {
		eag_thread_cancel(redirconn->t_write);
		redirconn->t_write = NULL;
	}
	if (NULL != redirconn->t_timeout) {
		eag_thread_cancel(redirconn->t_timeout);
		redirconn->t_timeout = NULL;
	}
	if (redirconn->conn_fd >= 0) {
		close(redirconn->conn_fd);
		redirconn->conn_fd = -1;
	}
	list_del(&(redirconn->node));
		
	eag_blkmem_free_item(redir->conn_blkmem, redirconn);

	return EAG_RETURN_OK;
}

static int
eag_redirconn_start_read(eag_redirconn_t *redirconn)
{
	char ipstr[32] = "";
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_start_read input error");
		return -1;
	}
	if (redirconn->conn_fd < 0) {
		eag_log_err("eag_redirconn_start_read conn_fd < 0");
		return -1;
	}
	if (NULL != redirconn->t_read) {
		eag_log_err("eag_redirconn_start_read t_read not null");
		return -1;
	}

	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	memset(redirconn->ibuf, 0, sizeof (redirconn->ibuf));
	redirconn->ibuflen = 0;
	redirconn->recvlines = 0;
	redirconn->recvdone = 0;

	eag_redirconn_event(EAG_REDIRCONN_READ, redirconn);

	eag_log_debug("eag_redir", "eag_redirconn start read, staip %s",
		ipstr);
	return EAG_RETURN_OK;
}

static int
eag_redirconn_stop_read(eag_redirconn_t *redirconn)
{
	char ipstr[32] = "";
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_stop_read input error");
		return -1;
	}
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	if (NULL != redirconn->t_read) {
		eag_thread_cancel(redirconn->t_read);
		redirconn->t_read = NULL;
	}

	if (NULL != redirconn->t_timeout) {
		eag_thread_cancel(redirconn->t_timeout);
		redirconn->t_timeout = NULL;
	}
	
	eag_log_debug("eag_redir", "eag_redirconn stop read, staip %s",
		ipstr);
	return EAG_RETURN_OK;
}

static int
eag_redirconn_start_write(eag_redirconn_t *redirconn)
{	
	char ipstr[32] = "";

	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_start_write input error");
		return -1;
	}
	if (redirconn->conn_fd < 0) {
		eag_log_err("eag_redirconn_start_write conn_fd < 0");
		return -1;
	}
	if (NULL != redirconn->t_write) {
		eag_log_err("eag_redirconn_start_write t_write not null");
		return -1;
	}
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	eag_redirconn_event(EAG_REDIRCONN_WRITE, redirconn);

	eag_log_debug("eag_redir", "eag_redirconn start write, staip %s",
		ipstr);
	return EAG_RETURN_OK;
}

static int
eag_redirconn_stop_write(eag_redirconn_t *redirconn)
{
	char ipstr[32] = "";
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_stop_write input error");
		return -1;
	}
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	if (NULL != redirconn->t_write) {
		eag_thread_cancel(redirconn->t_write);
		redirconn->t_write = NULL;
	}

	if (NULL != redirconn->t_timeout) {
		eag_thread_cancel(redirconn->t_timeout);
		redirconn->t_timeout = NULL;
	}
	
	eag_log_debug("eag_redir", "eag_redirconn stop write, staip %s",
		ipstr);

	return EAG_RETURN_OK;
}

static int
eag_redirconn_read_timeout(eag_thread_t *thread)
{
	eag_redirconn_t *redirconn = NULL;
	char ipstr[32] = "";

	if (NULL == thread) {
		eag_log_err("eag_redirconn_read_timeout input error");
		return -1;
	}
	redirconn = eag_thread_get_arg(thread);
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_read_timeout redirconn null");
		return -1;
	}
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	eag_log_debug("eag_redir_warning", "eag_redirconn read timeout, staip %s",
		ipstr);
	eag_redirconn_stop_read(redirconn);
	eag_redirconn_free(redirconn);

	return 0;
}

static int
eag_redirconn_write_timeout(eag_thread_t * thread)
{

	eag_redirconn_t *redirconn = NULL;
	char ipstr[32] = "";
	
	if (NULL == thread) {
		eag_log_err("eag_redirconn_write_timeout input error");
		return -1;
	}
	redirconn = eag_thread_get_arg(thread);
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_write_timeout redirconn null");
		return -1;
	}
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	eag_log_debug("eag_redir_warning", "eag_redirconn write timeout, staip %s",
		ipstr);
	eag_redirconn_stop_write(redirconn);
	eag_redirconn_free(redirconn);

	return 0;
}

static int
http_request_match(const char *line, const char *key,
					char *buf, size_t buflen)
{
	const char *p = NULL;
	int len = 0;

	if (NULL == line || NULL == key || NULL == buf || buflen <= 0) {
		return EAG_REDIR_HTTP_REQUEST_NOT_MATCH;
	}
	memset(buf, 0, buflen);

	if (strncasecmp(line, key, strlen(key)) == 0) {
		p = line + strlen(key);
		for (; isspace(*p); p++) {;
		}
		len = strlen(p);
		if (len > buflen - 1) {
			len = buflen - 1;
		}
		strncpy(buf, p, len);
		return EAG_REDIR_HTTP_REQUEST_MATCH;
	}

	return EAG_REDIR_HTTP_REQUEST_NOT_MATCH;
}

static int
http_request_method_is_valid(const char *buff)
{
	int cmp_len = 0;
	int buf_len = 0;
	
	if (NULL == buff) {
		return 0;
	}

	buf_len = strlen(buff);
	cmp_len = (buf_len < 4) ? buf_len : 4;
	if (strncmp("GET ", buff, cmp_len) == 0) {
		return 1;
	}
	cmp_len = (buf_len < 5) ? buf_len : 5;
	if (strncmp("HEAD ",  buff, cmp_len) == 0) {
		return 1;
	}
	if (strncmp("POST ",  buff, cmp_len) == 0) {
		return 1;
	}

	return 0;
}

static int
eag_redirconn_parse_request(eag_redirconn_t *redirconn)
{
	char *line_end = NULL;
	size_t line_len = 0;
	int i = 0;
	char user_ipstr[32] = "";
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_parse_request input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ip2str(redirconn->sta_ip, user_ipstr, sizeof(user_ipstr));
	
	eag_log_debug("eag_redir", 
		"redirconn parse request userip %s, recvlines=%u, ibuf(%s)",
		user_ipstr, redirconn->recvlines, redirconn->ibuf);
	
	if (redirconn->ibuflen != strlen(redirconn->ibuf)) {
		eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
			"ibuflen %u != ibuf strlen %u, http request have nil byte",
			user_ipstr, redirconn->ibuflen, strlen(redirconn->ibuf));
		return -1;
	}
	if (0 == redirconn->recvlines
		&& !http_request_method_is_valid(redirconn->ibuf)) {
		eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
			"invalid http request method %.5s",
			user_ipstr, redirconn->ibuf);
		return -1;
	}

	while ((line_end = strstr(redirconn->ibuf, "\r\n")) != NULL) {
		line_len = line_end - redirconn->ibuf;
		*line_end = '\0';
		redirconn->recvlines++;
		if (redirconn->recvlines > 50) {
			eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
					"http request lines %u reach limit 50",
					user_ipstr, redirconn->recvlines);
			return -1;
		}

		if (1 == redirconn->recvlines) {
			int str_len = 0;
			char *p1 = redirconn->ibuf;
			char *p2 = NULL;

			eag_log_debug("eag_redir", "redirconn parse request userip %s, "
					"receive http request: %s",
					user_ipstr, redirconn->ibuf);
			if (strncmp("GET ", p1, 4) == 0) {
				p1 += 4;
			} else if (strncmp("HEAD ", p1, 5) == 0) {
				p1 += 5;
			} else if (strncmp("POST ", p1, 5) == 0) {
				p1 += 5;
			} else {
				eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
						"unknown http request method", user_ipstr);
				return EAG_ERR_REDIR_HTTP_UNKNOWN_REQUEST_METHORD;
			}
			for (; ' ' == *p1; p1++) ;
			if ('/' != *p1) {
				eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
						"http request url has no root dir", user_ipstr);
				return EAG_ERR_REDIR_HTTP_URL_HAS_NO_ROOT_DIR;
			}
			p2 = strchr(p1, ' ');
			if (NULL == p2) {
				eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
					"no http request version", user_ipstr);
				return EAG_ERR_REDIR_HTTP_HAS_NO_REQUEST_VERSION;
			}
			str_len = p2 - p1;
			if (str_len > sizeof(redirconn->request_url) - 1) {
				eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
					"http request url length %d, truncated",
					user_ipstr, str_len);
				str_len = sizeof (redirconn->request_url) - 1;
			}
			strncpy(redirconn->request_url, p1, str_len);
			eag_log_debug("eag_redir", "redirconn parse request userip %s, "
					"HTTP request URL: %s",
				     user_ipstr, redirconn->request_url);
		} else if (0 == line_len) {
			eag_log_debug("eag_redir", "redirconn parse request userip %s, "
					"line=%u, line_len=0, recvdone=1",
					user_ipstr, redirconn->recvlines);
			redirconn->recvdone = 1;
			return EAG_RETURN_OK;
		} else {
			char getmatch[1024];
			eag_log_debug("eag_redir", "redirconn parse request userip %s, "
				"line=%u, buf=%s",
				user_ipstr, redirconn->recvlines, redirconn->ibuf);
			if (EAG_REDIR_HTTP_REQUEST_MATCH ==
					http_request_match(redirconn->ibuf,
						"Host:",
						getmatch,/*redirconn->request_host,*//*http_request_match will memset the buff even not match!!*/
						sizeof(getmatch)/*sizeof (redirconn->request_host)*/))
			{
				strncpy( redirconn->request_host, getmatch, sizeof(redirconn->request_host)-1);
				eag_log_debug("eag_redir",
						"redirconn parse request userip %s, "
						"HTTP Request Host: %s",
						user_ipstr, redirconn->request_host);
			} else if (EAG_REDIR_HTTP_REQUEST_MATCH ==
					http_request_match(redirconn->ibuf,
						"User-Agent:",
						getmatch,/*redirconn->user_agent,*/
						sizeof(getmatch)/*sizeof (redirconn->user_agent)*/))
			{
				strncpy( redirconn->user_agent, getmatch, sizeof(redirconn->user_agent)-1);
				eag_log_debug("eag_redir",
						"redirconn parse request userip %s, "
						"HTTP Request User Agent: %s",
						user_ipstr, redirconn->user_agent);
			}
		}

		line_len += 2;
		eag_log_debug("eag_redir", "redirconn parse request userip %s, "
				"ibuflen=%u line_len=%u ibuflen-line_len=%u",
				user_ipstr, redirconn->ibuflen, line_len,
				redirconn->ibuflen - line_len);
		for (i = 0; i < redirconn->ibuflen - line_len; i++) {
			redirconn->ibuf[i] = redirconn->ibuf[line_len + i];
		}
		redirconn->ibuf[i] = '\0';
		redirconn->ibuflen -= line_len;
		eag_log_debug("eag_redir", "redirconn parse request userip %s, "
			"ibuflen=%u", user_ipstr, redirconn->ibuflen);
	}

	if (redirconn->ibuflen >= sizeof(redirconn->ibuf)-1) {
		eag_log_debug("eag_redir_warning", "redirconn parse request userip %s, "
			"ibuflen %u >= buf size %u",
			user_ipstr, redirconn->ibuflen, sizeof(redirconn->ibuf)-1);
		return -1;
	}
	
	return EAG_RETURN_OK;
}

int
test_char_in_set(char c)
{
	const char set[]="/:?#;,+=";
	int i;
	for (i=0;i<sizeof(set);i++) {
		if (c == set[i]) {
			return 1;
		}
	}

	return 0;
}

void
decodeURIComponent(char *dst, uint32_t dstsize, char *src)
{
	
	if (NULL==dst || NULL==src) {
		return;
	}

	while ((dstsize-1)>0 && *src >0) {
		if (test_char_in_set(*src)) {
			if (dstsize <= 3) {
				break;
			}
			*dst = '%';
			dst++;
			sprintf(dst,"%2X",*src);
			dst+=2;
			dstsize -= 3;
		}else{
			*dst = *src;
			dst++;
			dstsize--;
		}
		src++;
	}
	*dst = 0;
	return;
}

static int
eag_redirconn_build_redirurl( eag_redirconn_t *redirconn )
{
	struct portal_srv_t * portal_srv = NULL;
	char user_ipstr[32] = "";
	char acip_str[32]= "";
	uint32_t nasip = 0;
	eag_redir_t *redir = NULL;
	
	appconn_db_t *appdb = NULL;
	eag_ins_t *eagins = NULL;
	struct app_conn_t *appconn = NULL;

	redir = redirconn->redir;
	appdb = redir->appdb;
	eagins = redir->eagins;
	ip2str(redirconn->sta_ip, user_ipstr, sizeof(user_ipstr));

	appconn = appconn_find_by_userip(appdb, redirconn->sta_ip);
	if (NULL == appconn) {
		eag_log_warning("redirconn build redirurl failed, cannot find appconn, userip=%s",
			user_ipstr);
		return -1;
	}

	if (EAG_RETURN_OK != 
			appconn_config_portalsrv(appconn, redir->portalconf)) 
	{
		eag_log_debug("eag_redir_warning", "eag_redirconn_build_redirurl "
				"appconn_config_portalsrv failed, userip %s", user_ipstr);
		return -1;
	}
	
	portal_srv = &(appconn->session.portal_srv);
	snprintf(redirconn->redirurl, sizeof(redirconn->redirurl)-1,
			"%s?wlanuserip=%s&wlanacname=%s",
			portal_srv->portal_url, user_ipstr, 
			portal_srv->acname);
	/* acip to url */
	if (1 == portal_srv->acip_to_url) {
		nasip = eag_ins_get_nasip(eagins);
		ip2str(nasip, acip_str, sizeof(acip_str));
		strncat( redirconn->redirurl, "&wlanacip=", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat( redirconn->redirurl, acip_str, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	/* nasid to url */
	if (1 == portal_srv->nasid_to_url) {
		strncat( redirconn->redirurl, "&NASID=", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat( redirconn->redirurl, appconn->session.nasid, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	/*wlanparameter_to_url*/
	if (1 == portal_srv->wlanparameter) {
		char wlanparameter_str[100];
		unsigned char usermac_str[24];
	
		int outlen = 0;
		unsigned char *outbuf=NULL;		
		//des encrypt
		mac2str( appconn->session.usermac, (char *)usermac_str, sizeof(usermac_str), '-');
		outlen = BIO_des_encrypt(usermac_str, &outbuf, 
						strlen((char *)usermac_str), (unsigned char*)portal_srv->deskey);
		if (outlen>0 && NULL !=outbuf) {
			memset( wlanparameter_str, 0, sizeof(wlanparameter_str) );
			hex2str( outbuf, outlen, (unsigned char*)wlanparameter_str, sizeof(wlanparameter_str));
			strncat( redirconn->redirurl, "&wlanparameter=", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
			strncat( redirconn->redirurl, wlanparameter_str, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		}else{
			eag_log_err("eag_redirconn_build_redirurl build wlanparameter error! "\
						"outlen=%d outbuf=%p", outlen, outbuf );
		}	
		if (NULL != outbuf) {
			free(outbuf);
			outbuf = NULL;
		}
	}

	/* wlanapmac */
	if (1 == portal_srv->wlanapmac) {
		char apmac_str[20] = {0};
		char para_apmac_name[16] = "&wlanapmac=";
		if (1 == username_check_switch) {
			memset(para_apmac_name, 0, sizeof(para_apmac_name));
			strncpy(para_apmac_name, "&ap_mac=", sizeof(para_apmac_name)-1);
		}
		//build_wlanapmac(apmac_str, appconn->session.apmac, sizeof(apmac_str));	
		mac2str( appconn->session.apmac, (char *)apmac_str, sizeof(apmac_str), ':');
		strncat( redirconn->redirurl, para_apmac_name, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat( redirconn->redirurl, apmac_str, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	/* usermac to url */
	if (1 == portal_srv->usermac_to_url) {
		char usermac_str[24];
		char para_usermac_name[16] = "&usermac=";
		if (1 == username_check_switch) {
			memset(para_usermac_name, 0, sizeof(para_usermac_name));
			strncpy(para_usermac_name, "&client_mac=", sizeof(para_usermac_name)-1);
		}
		mac2str( appconn->session.usermac, (char *)usermac_str, sizeof(usermac_str), ':');
		strncat( redirconn->redirurl, para_usermac_name, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat( redirconn->redirurl, usermac_str, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	/* wlanusermac*/
	if (1 == portal_srv->wlanusermac) {
		char wlanusermac_str[100];
		unsigned char usermac_str[24];
	
		int outlen = 0;
		unsigned char *outbuf=NULL;		
		//des encrypt
		mac2str( appconn->session.usermac, (char *)usermac_str, sizeof(usermac_str), '-');
		outlen = BIO_des_encrypt(usermac_str, &outbuf, 
						strlen((char *)usermac_str), (unsigned char*)portal_srv->wlanusermac_deskey);
		if (outlen>0 && NULL !=outbuf) {
			memset( wlanusermac_str, 0, sizeof(wlanusermac_str) );
			hex2str( outbuf, outlen, (unsigned char*)wlanusermac_str, sizeof(wlanusermac_str));
			strncat( redirconn->redirurl, "&wlanusermac=", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
			strncat( redirconn->redirurl, wlanusermac_str, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		}else{
			eag_log_err("eag_redirconn_build_redirurl build wlanparameter error! "\
						"outlen=%d outbuf=%p", outlen, outbuf );
		}
		if (NULL != outbuf) {
			free(outbuf);
			outbuf = NULL;
		}
	}

	if (WISPR_URL_HTTP == portal_srv->wisprlogin ||
		WISPR_URL_HTTPS == portal_srv->wisprlogin ) 
	{
		char wisprloginurl[128];
		char wisprloginurl_encode[256];
		memset (wisprloginurl, 0, sizeof(wisprloginurl) );
		memset (wisprloginurl_encode, 0, sizeof(wisprloginurl_encode));

		nasip = eag_ins_get_nasip(eagins);
		ip2str(nasip, acip_str, sizeof(acip_str));		

		if (WISPR_URL_HTTP == portal_srv->wisprlogin) {
			snprintf (wisprloginurl, sizeof(wisprloginurl),
						"http://%s:8081/wispr/login?UserIP=%s",acip_str,user_ipstr); 
		}else{
			snprintf (wisprloginurl, sizeof(wisprloginurl),
						"https://%s:8082/wispr/login?UserIP=%s",acip_str,user_ipstr);
		}
		decodeURIComponent(wisprloginurl_encode,sizeof(wisprloginurl_encode),wisprloginurl);
		
		strncat (redirconn->redirurl, "&loginurl=", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat (redirconn->redirurl, wisprloginurl_encode, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	/* essid */
    char para_essid_name[16] = "&ssid=";
    if (1 == username_check_switch) {
		memset(para_essid_name, 0, sizeof(para_essid_name));
		strncpy(para_essid_name, "&wlan=", sizeof(para_essid_name)-1);
    }
	strncat(redirconn->redirurl, para_essid_name, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));	
	strncat(redirconn->redirurl, appconn->session.essid, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));

	/* url-suffix */
	if( 0 != strcmp(portal_srv->url_suffix, "")) {
		strncat( redirconn->redirurl, "&", sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
		strncat( redirconn->redirurl, portal_srv->url_suffix, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
	}
	
    /*wlanuserfirsturl_to_url*/
    if (1 == portal_srv->wlanuserfirsturl) {
        char userfirsturl[512];
		char para_firsturl_name[32] = "&wlanuserfirsturl=";
    	if (1 == username_check_switch) {
			memset(para_firsturl_name, 0, sizeof(para_firsturl_name));
			strncpy(para_firsturl_name, "&redirect=", sizeof(para_firsturl_name)-1);
    	}
        snprintf(userfirsturl, sizeof(userfirsturl)-1,"http://%s%s",
                        redirconn->request_host, redirconn->request_url);
        strncat( redirconn->redirurl, para_firsturl_name, sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
        strncat( redirconn->redirurl, userfirsturl,
                 sizeof(redirconn->redirurl)-strlen(redirconn->redirurl));
    }

	eag_log_info("eag_redirconn_build_redirurl userip %s, redirURL = (%s)",
			user_ipstr, redirconn->redirurl);
	
	return EAG_RETURN_OK;
}


static int
eag_redirconn_build_response(eag_redirconn_t *redirconn)
{
	int len = 0;
	char ipstr[32] = "";
	int ret = 0;
	
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_build_response input error");
		return -1;
	}
	
	ret = eag_redirconn_build_redirurl( redirconn );
	if (EAG_RETURN_OK != ret) {
		return -1;
	}
	
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));

	snprintf(redirconn->ibuf, sizeof(redirconn->ibuf) - 1,
			"HTTP/1.0 302 Moved Temporarily\r\n"
			"Location: %s\r\n", redirconn->redirurl);

	len = strlen(redirconn->ibuf);
	snprintf(redirconn->ibuf+len, sizeof(redirconn->ibuf)-len-1,
			INTERNATIONAL_ROAMING_XML, redirconn->redirurl);

	redirconn->ibuflen = strlen(redirconn->ibuf);

	eag_log_debug("eag_redir",
		"redirconn build response userip %s, response(%s)",
		ipstr, redirconn->ibuf);
	
	return EAG_RETURN_OK;
}

static int
eag_redirconn_read(eag_thread_t *thread)
{
	eag_redirconn_t *redirconn = NULL;
	eag_redir_t *redir = NULL;
	ssize_t nbyte = 0;
	int ret = 0;
	struct app_conn_t *appconn = NULL;

	if (NULL == thread) {
		eag_log_err("eag_redirconn_read input error");
		return -1;
	}
	redirconn = eag_thread_get_arg(thread);
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_read redirconn null");
		return -1;
	}
	redir = redirconn->redir;
	
	nbyte = recv(redirconn->conn_fd, redirconn->ibuf + redirconn->ibuflen,
		     sizeof(redirconn->ibuf) - redirconn->ibuflen - 1, 0);
	if (nbyte < 0) {
		eag_log_debug("eag_redir_warning", "Read failed on redir conn socket %d: %s",
				redirconn->conn_fd, safe_strerror(errno));
		if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno) {
			eag_redirconn_stop_read(redirconn);
			eag_redirconn_free(redirconn);
			return EAG_ERR_REDIR_HTTP_RECV_ERROR;
		}
	} else if (0 == nbyte) {
		eag_log_debug("eag_redir", "eag_redirconn_read recv 0 byte");
		redirconn->recvdone = 1;
	} else { /* nbyte > 0 */
		eag_log_debug("eag_redir", "eag_redirconn_read recv %d bytes",
				nbyte);
		redirconn->ibuflen += nbyte;
		redirconn->ibuf[redirconn->ibuflen] = '\0';
		ret = eag_redirconn_parse_request(redirconn);
		eag_log_debug("eag_redir", 
			"eag_redirconn_read redirconn_parse_request ret = %d",
			ret);
		if (EAG_RETURN_OK != ret) {
			eag_redirconn_stop_read(redirconn);
			eag_redirconn_free(redirconn);
			return ret;
		}
	}
	eag_log_debug("eag_redir", "eag_redirconn_read recvdone = %d",
			redirconn->recvdone);
	if (redirconn->recvdone) {
		eag_redirconn_stop_read(redirconn);
		eag_portal_set_pdc_usermap(redir->portal, redirconn->sta_ip);
		ret = eag_redirconn_build_response(redirconn);
		if (EAG_RETURN_OK != ret) {
			eag_redirconn_free(redirconn);
			return ret;
		}
		eag_redirconn_start_write(redirconn);
		appconn = appconn_find_by_userip(redir->appdb, redirconn->sta_ip);
		if (NULL != appconn) {
			strncpy(appconn->user_agent, redirconn->user_agent,
					sizeof(appconn->user_agent));
		}
	}

	return EAG_RETURN_OK;
}

static int
eag_redirconn_write(eag_thread_t *thread)
{
	eag_redirconn_t *redirconn = NULL;
	eag_redir_t *redir = NULL;
	struct app_conn_t *appconn = NULL;
	ssize_t writen = 0;
	int i = 0;
	char ipstr[32] = "";
	
	if (NULL == thread) {
		eag_log_err("eag_redirconn_write input error");
		return -1;
	}
	redirconn = eag_thread_get_arg(thread);
	if (NULL == redirconn) {
		eag_log_err("eag_redirconn_write redirconn null");
		return -1;
	}
	redir = redirconn->redir;
	ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));
	
	writen = write(redirconn->conn_fd, redirconn->ibuf, redirconn->ibuflen);
	if (writen < 0) {
		eag_log_debug("eag_redir_warning", "Write failed on redir conn socket %d: %s, userip %s",
				redirconn->conn_fd, safe_strerror(errno), ipstr);
		if (EAGAIN != errno && EWOULDBLOCK != errno && EINTR != errno) {
			eag_redirconn_stop_write(redirconn);
			eag_redirconn_free(redirconn);
			return EAG_ERR_REDIR_HTTP_WRITE_SYSTEM_ERROR;
		}
	} else if (0 == writen) {
		eag_log_debug("eag_redir_warning", "Write 0 byte on redir conn socket: %d, userip %s",
					redirconn->conn_fd, ipstr);
	} else {
		eag_log_debug("eag_redir",
		      "eag_redirconn_write buff=%s buflen=%u writen=%d, userip %s",
		      redirconn->ibuf, redirconn->ibuflen, writen, ipstr);
		for (i = 0; i < redirconn->ibuflen - writen; i++) {
			redirconn->ibuf[i] = redirconn->ibuf[writen + i];
		}
		redirconn->ibuflen -= writen;
		redirconn->ibuf[redirconn->ibuflen] = '\0';
	}

	eag_log_debug("eag_redir", "eag_redirconn_write ibuflen=%d, userip %s",
		redirconn->ibuflen, ipstr);
	if (redirconn->ibuflen <= 0) {
		appconn = appconn_find_by_userip(redir->appdb, redirconn->sta_ip);
		eag_bss_message_count(redir->eagstat, appconn, BSS_HTTP_REDIR_REQ_COUNT, 1);
		eag_bss_message_count(redir->eagstat, appconn, BSS_HTTP_REDIR_SUCCESS_COUNT, 1);
		eag_redirconn_stop_write(redirconn);
		eag_redirconn_free(redirconn);
	}

	return EAG_RETURN_OK;
}

eag_redir_t *
eag_redir_new(int hansi_type, int hansi_id)
{
	eag_redir_t *redir = NULL;

	redir = eag_malloc(sizeof(eag_redir_t));
	if (NULL == redir) {
		eag_log_err("eag_redir_new eag_malloc failed");
		return NULL;
	}

	memset(redir, 0, sizeof(eag_redir_t));
	if (EAG_RETURN_OK != eag_blkmem_create(&(redir->conn_blkmem),
					       EAG_REDIR_BLKMEM_NAME,
					       sizeof(eag_redirconn_t),
					       EAG_REDIR_BLKMEM_BLKNUM,
					       EAG_REDIR_BLKMEM_MAXNUM)) {
		eag_log_err("eag_redir_new eag_blkmem_create failed");
		eag_free(redir);
		return NULL;
	}
	redir->listen_fd = -1;
	redir->redir_port = EAG_REDIR_LISTEN_PORT;
	redir->max_redir_times = DEFAULT_MAX_REDIR_TIMES;
	redir->redir_check_interval = DEFAULT_REDIR_CHECK_INTERVAL;
	redir->force_dhcplease = 0;
	redir->hansi_type = hansi_type;
	redir->hansi_id = hansi_id;
	INIT_LIST_HEAD(&(redir->conn_head));

	eag_log_info("redir new ok");
	
	return redir;
}

int
eag_redir_free(eag_redir_t *redir)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_free input error");
		return -1;
	}
	if (NULL != redir->conn_blkmem) {
		eag_blkmem_destroy(&(redir->conn_blkmem));
	}

	eag_free(redir);

	eag_log_info("redir free ok");
	
	return EAG_RETURN_OK;
}

int
eag_redir_start(eag_redir_t * redir)
{
	int ret = 0;
	struct sockaddr_in addr;
	char ipstr[32] = "";
	uint32_t nasip = 0;
	int is_distributed = 0;
	int pdc_distributed = 0;
	
	if (NULL == redir) {
		eag_log_err("eag_redir_start input error");
		return EAG_ERR_NULL_POINTER;
	}

	if( redir->listen_fd >= 0 ){
		eag_log_err("eag_redir_start redir already start");
		return EAG_RETURN_OK;
	}

	is_distributed = eag_ins_get_distributed(redir->eagins);
	pdc_distributed = eag_ins_get_pdc_distributed(redir->eagins);
	nasip = eag_ins_get_nasip(redir->eagins);
	redir->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (redir->listen_fd < 0) {
		eag_log_err("Can't create redir stream socket: %s",
			    safe_strerror(errno));
		redir->listen_fd = -1;
		return EAG_ERR_REDIR_SOCKET_INIT_FAILED;
	}

	memset(&addr, 0, sizeof (struct sockaddr_in));
	addr.sin_family = AF_INET;
	if (1 == is_distributed && 1 == pdc_distributed) {
		addr.sin_addr.s_addr = htonl(redir->local_ip);
		addr.sin_port = htons(redir->local_port);
	} else {
		addr.sin_addr.s_addr = htonl(nasip);
		addr.sin_port = htons(redir->redir_port);
	}
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof (struct sockaddr_in);
#endif				/* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */

	sockopt_reuseaddr(redir->listen_fd);
	ret = bind(redir->listen_fd, (struct sockaddr *) &addr,
				sizeof (struct sockaddr_in));
	if (ret < 0) {
		eag_log_err("Can't bind to redir stream socket: %s",
			    safe_strerror(errno));
		close(redir->listen_fd);
		redir->listen_fd = -1;
		return EAG_ERR_REDIR_SOCKET_BIND_FAILED;
	}

	ret = listen(redir->listen_fd, 32);
	if (ret < 0) {
		eag_log_err("Can't listen to redir stream socket: %s",
			    safe_strerror(errno));
		close(redir->listen_fd);
		redir->listen_fd = -1;
		return EAG_ERR_REDIR_SOCKET_LISTEN_FAILED;
	}

	eag_redir_event(EAG_REDIR_SERV, redir);

	if (1 == is_distributed && 1 == pdc_distributed) {
		eag_log_info("redir(%s:%u) fd(%d) start ok", 
			ip2str(redir->local_ip, ipstr, sizeof(ipstr)),
			redir->local_port,
			redir->listen_fd);
	} else {
		eag_log_info("redir(%s:%u) fd(%d) start ok", 
			ip2str(nasip, ipstr, sizeof(ipstr)),
			redir->redir_port,
			redir->listen_fd);
	}

	return EAG_RETURN_OK;
}

int
eag_redir_stop(eag_redir_t *redir)
{
	eag_redirconn_t *redirconn = NULL;
	eag_redirconn_t *next = NULL;
	char ipstr[32] = "";
	uint32_t nasip = 0;
	int is_distributed = 0;
    int pdc_distributed = 0;

	if (NULL == redir) {
		eag_log_err("eag_redir_stop input error");
		return -1;
	}

	is_distributed = eag_ins_get_distributed(redir->eagins);
	pdc_distributed = eag_ins_get_pdc_distributed(redir->eagins);
	nasip = eag_ins_get_nasip(redir->eagins);
	if (NULL != redir->t_accept) {
		eag_thread_cancel(redir->t_accept);
		redir->t_accept = NULL;
	}
	if (redir->listen_fd >= 0) {
		close(redir->listen_fd);
		redir->listen_fd = -1;
	}

	list_for_each_entry_safe(redirconn, next, &(redir->conn_head), node){
			eag_redirconn_free(redirconn);
	}

	if (1 == is_distributed && 1 == pdc_distributed) {
		eag_log_info("redir(%s:%u) stop ok",
			ip2str(redir->local_ip, ipstr, sizeof(ipstr)),
			redir->local_port);
	} else {
		eag_log_info("redir(%s:%u) stop ok",
			ip2str(nasip, ipstr, sizeof(ipstr)),
			redir->redir_port);
	}

	return EAG_RETURN_OK;
}

static int
eag_redir_accept(eag_thread_t *thread)
{
	eag_redir_t *redir = NULL;
	eag_redirconn_t *redirconn = NULL;
	int conn_fd = -1;
	struct sockaddr_in client;
	socklen_t len = 0;
	uint32_t user_ip = 0;
	char ipstr[32] = "";
	char macstr[32] = "";
	uint8_t zero_mac[6] = {0};
	struct app_conn_t *appconn = NULL;
	struct app_conn_t *tmp_appconn = NULL;
	struct appsession tmpsession = {0};
	int ret = 0;
	unsigned int security_type = 0;
	int force_wireless = 0;
	
	if (NULL == thread) {
		eag_log_err("eag_redir_accept input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	redir = eag_thread_get_arg(thread);
	if (NULL == redir) {
		eag_log_err("eag_redir_accept redir null");
		return EAG_ERR_REDIR_THREAD_ARG_ERROR;
	}

	len = sizeof(struct sockaddr_in);
	conn_fd = accept(redir->listen_fd, (struct sockaddr *) &client, &len);
	if (conn_fd < 0) {
		eag_log_err("Can't accept redir socket %d: %s",
			    redir->listen_fd, safe_strerror(errno));
		return EAG_ERR_REDIR_ACCEPT_FAILED;
	}
	user_ip = ntohl(client.sin_addr.s_addr);
	ip2str(user_ip, ipstr, sizeof(ipstr));
	
	if (0 != set_nonblocking(conn_fd)) {
		eag_log_err("eag_redir_accept set socket nonblocking failed, "
			"fd(%d), userip %s", conn_fd, ipstr);
		close(conn_fd);
		conn_fd = -1;
		return EAG_ERR_SOCKET_OPT_FAILED;
	}

	redirconn = eag_redirconn_new(redir, user_ip);
	if (NULL == redirconn) {
		eag_log_err("eag_redir_accept eag_redirconn_new failed");
		close(conn_fd);
		conn_fd = -1;
		return EAG_ERR_REDIR_CREATE_REDIRCONN_ERROR;
	}
	redirconn->conn_fd = conn_fd;
#if 0
	appconn = appconn_find_by_ip_autocreate(redir->appdb, user_ip);
	if (NULL == appconn) {
		eag_log_warning("eag_redir_accept "
			"appconn_find_by_ip_autocreate failed, userip %s", ipstr);
		eag_redirconn_free(redirconn);
		return -1;
	}
#endif
	appconn = appconn_find_by_userip(redir->appdb, user_ip);
	if (NULL == appconn) {
		ret = appconn_check_is_conflict(user_ip, redir->appdb, &tmpsession, &tmp_appconn);
		if (EAG_ERR_APPCONN_APP_IS_CONFLICT == ret && NULL != tmp_appconn) {
			if (APPCONN_STATUS_AUTHED == tmp_appconn->session.state) {
				terminate_appconn_nowait(tmp_appconn, redir->eagins, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
			} else {
				appconn_del_from_db(tmp_appconn);
				appconn_free(tmp_appconn);
			}
			tmp_appconn = NULL;
		} else if (EAG_RETURN_OK != ret) {
			eag_redirconn_free(redirconn);
			return -1;
		}
		mac2str(tmpsession.usermac, macstr, sizeof(macstr), ':');
		
		ret = eag_get_sta_info_by_mac_v2(redir->eagdbus, redir->hansi_type, redir->hansi_id,
					tmpsession.usermac, &tmpsession, &security_type);
		
		force_wireless = appconn_db_get_force_wireless(redir->appdb);
		if (0 == ret && NO_NEED_AUTH == security_type) {
			eag_log_info("eag_redir_accept  eap or none authorize user_ip=%s by user redir",
				ipstr);
			if (0 == memcmp(zero_mac, tmpsession.usermac, 6)
				&& force_wireless) {
				eag_log_debug("eag_redir_warning", "eag_redir_accept"
					"userip %s, usermac is zero, force_wireless enable",
					ipstr);
				return -1;
			}
			#if 0
			if (strlen(tmpsession.intf) == 0) {
				ip_interface(user_ip, tmpsession.intf,  
						sizeof(tmpsession.intf)-1);
				eag_log_debug("eag_redir","eag_redir_accept ip_interface userip %s, interface(%s)",
					ipstr, tmpsession.intf);
				if (strlen(tmpsession.intf) == 0) {
					eag_log_debug("eag_redir_warning", "eag_redir_accept"
						"ip_interface userip %s, interface not found",
						ipstr);
					return -1;
				}
			}
			#endif
			if (strlen(tmpsession.intf) == 0) {
				eag_log_debug("eag_redir_warning", 
					"userip %s, interface not found", ipstr);
				return -1;
			}
			eag_stamsg_send(redir->stamsg, &tmpsession, EAG_NTF_ASD_STA_INFO);
			//redirconn->asd_auth = 1;
			//eag_redirconn_start_read(redirconn);
			eag_redirconn_free(redirconn);
			return EAG_RETURN_OK;
		} else if (0 != ret && force_wireless) {
			eag_log_debug("eag_redir_err", "eag_redir_accept "
				"eag_get_sta_info_by_mac_v2 failed, userip=%s, usermac=%s, ret=%d",
				ipstr, macstr, ret);
			eag_redirconn_free(redirconn);
			return -1;
		} else {
			appconn = appconn_create_by_sta_v2(redir->appdb, &tmpsession);
		}
	}

	if (NULL == appconn) {
		eag_log_debug("eag_redir_warning", "eag_redir_accept "
			"appconn_create_by_sta_v2 failed, userip %s", ipstr);
		eag_redirconn_free(redirconn);
		return -1;
	}

	if (EAG_RETURN_OK != appconn_check_redir_count(appconn,
								redir->max_redir_times,
								redir->redir_check_interval))
	{
		eag_log_debug("eag_redir_warning", "eag_redir_accept check_redir_count failed, "
			"userip %s, redir_count %d > max %d, check_interval %d",
			ipstr, appconn->session.redir_count,
			redir->max_redir_times, redir->redir_check_interval);
		eag_redirconn_free(redirconn);
		return -1;
	}

	if (0 != redir->force_dhcplease 
		&& EAG_RETURN_OK != appconn_check_dhcplease(appconn))
	{
		eag_log_debug("eag_redir_warning", "eag_redir_accept appconn %s check_dhcplease failed",
			ipstr);
		eag_redirconn_free(redirconn);
		return -1;
	}

	if (EAG_RETURN_OK != 
			appconn_config_portalsrv(appconn, redir->portalconf)) 
	{
		eag_log_debug("eag_redir_warning", "eag_redir_accept "
				"appconn_config_portalsrv failed, userip %s", ipstr);
		eag_redirconn_free(redirconn);
		return -1;
	}
			
	eag_redirconn_start_read(redirconn);
		
	return EAG_RETURN_OK;
}


int
eag_redir_set_local_addr(eag_redir_t *redir,
							uint32_t local_ip,
							uint16_t local_port)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_local_addr input error");
		return -1;
	}

	redir->local_ip = local_ip;
	redir->local_port = local_port;
	
	return 0;
}

int
eag_redir_set_thread_master(eag_redir_t *redir,
							eag_thread_master_t *master)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_thread_master input error");
		return -1;
	}

	redir->master = master;

	return 0;
}

int
eag_redir_set_portal_conf(eag_redir_t *redir,
							struct portal_conf *portalconf)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_portal_conf input error");
		return -1;
	}

	redir->portalconf = portalconf;

	return 0;
}

int
eag_redir_set_eagstat(eag_redir_t *redir,
						eag_statistics_t *eagstat)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_eagstat input error");
		return -1;
	}

	redir->eagstat = eagstat;
	
	return EAG_RETURN_OK;
}

int
eag_redir_set_stamsg(eag_redir_t *redir,
						eag_stamsg_t *stamsg)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_stamsg input error");
		return -1;
	}

	redir->stamsg = stamsg;
	
	return EAG_RETURN_OK;
}

int
eag_redir_set_eagdbus(eag_redir_t *redir,
						eag_dbus_t *eagdbus)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_eagdbus input error");
		return -1;
	}

	redir->eagdbus = eagdbus;
	
	return EAG_RETURN_OK;
}

int
eag_redir_set_appdb(eag_redir_t *redir,
						appconn_db_t *appdb)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_appdb input error");
		return -1;
	}

	redir->appdb = appdb;

	return 0;
}

int
eag_redir_set_portal(eag_redir_t *redir,
						eag_portal_t *portal)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_portal input error");
		return -1;
	}

	redir->portal = portal;

	return 0;
}

int
eag_redir_set_eagins(eag_redir_t *redir,
						eag_ins_t *eagins)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_eagins input error");
		return -1;
	}

	redir->eagins = eagins;

	return 0;
}

int 
eag_redir_set_max_redir_times(eag_redir_t *redir,
									int max_redir_times)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_max_redir_times input error");
		return -1;
	}

	redir->max_redir_times = max_redir_times;

	return 0;
}

int 
eag_redir_get_max_redir_times(eag_redir_t *redir,
									int *max_redir_times)
{
	if (NULL == redir || NULL == max_redir_times) {
		eag_log_err("eag_redir_get_max_redir_times input error");
		return -1;
	}

	*max_redir_times = redir->max_redir_times;
	
	return 0;
}

int
eag_redir_set_force_dhcplease(eag_redir_t *redir,
						int force_dhcplease)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_set_force_dhcplease input error");
		return -1;
	}

	redir->force_dhcplease = force_dhcplease;

	return 0;
}

int
eag_redir_get_force_dhcplease(eag_redir_t *redir)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_get_force_dhcplease input error");
		return 0;
	}

	return redir->force_dhcplease;
}

int 
eag_redir_log_all_redirconn(eag_redir_t *redir)
{
	eag_redirconn_t *redirconn = NULL;
	char ipstr[32] = "";
	int num = 0;

	if (NULL == redir) {
		eag_log_err("eag_redir_log_all_redirconn input error");
		return -1;
	}

	eag_log_info( "-----log all redirconn begin-----");
	list_for_each_entry(redirconn, &(redir->conn_head), node) {
		num++;
		ip2str(redirconn->sta_ip, ipstr, sizeof(ipstr));	
		eag_log_info("%-5d redirconn connfd:%d userip:%s",
			num, redirconn->conn_fd, ipstr);
	}
	eag_log_info( "-----log all redirconn end, num: %d-----", num);

	return 0;
}

static void
eag_redir_event(eag_redir_event_t event, eag_redir_t *redir)
{
	if (NULL == redir) {
		eag_log_err("eag_redir_event input error");
		return;
	}
	
	switch (event) {
	case EAG_REDIR_SERV:
		redir->t_accept =
		    eag_thread_add_read(redir->master,
					eag_redir_accept, redir, redir->listen_fd);
		if (NULL == redir->t_accept) {
			eag_log_err("eag_redir_event thread_add_read failed");
		}
		break;
	default:
		break;
	}
}

static void
eag_redirconn_event(eag_redirconn_event_t event, eag_redirconn_t * redirconn)
{
	switch (event) {
	case EAG_REDIRCONN_READ:
		redirconn->t_read =
		    eag_thread_add_read(redirconn->master, eag_redirconn_read,
					redirconn, redirconn->conn_fd);
		redirconn->t_timeout =
		    eag_thread_add_timer(redirconn->master,
					 eag_redirconn_read_timeout, redirconn,
					 EAG_REDIR_READ_TIME_OUT);
		break;
	case EAG_REDIRCONN_WRITE:
		redirconn->t_write =
		    eag_thread_add_write(redirconn->master, eag_redirconn_write,
					 redirconn, redirconn->conn_fd);
		redirconn->t_timeout =
		    eag_thread_add_timer(redirconn->master,
					 eag_redirconn_write_timeout, redirconn,
					 EAG_REDIR_WRITE_TIME_OUT);
		break;
	default:
		break;
	}
}

