#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"
#include "mem_cache.h"
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"
#include "pppoe_ppp.h"

#include "pppoe_ipcp.h"


static char *ipcp_codenames[] = {
	"Configuration Request", 
	"Configuration Ack",
	"Configuration Nak",
	"Configuration Reject",
	"Termination Request",
	"Termination Ack",
	"Code Reject",
};


int
ipcpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct session_options *options = sess->options;
	struct pppoe_ctl *ctl;
	uint16 *proto;
	uint8 *cursor;

	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl))) {
		pppoe_log(LOG_WARNING, "IPCP Config Request packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_IPCP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = CONFREQ;
	ctl->ident = sess->ident = 0x1;

	if (options->neg_ipaddr) {
		if (pbuf_tailroom(pbuf) < CILEN_ADDR) {
			pppoe_log(LOG_WARNING, "IPCP Config Request packet: pbuf tailroom not meet length %d\n", 
									CILEN_ADDR);
			return PPPOEERR_ENOMEM;
		}
		
		cursor = pbuf_put(pbuf, CILEN_ADDR);
		PUTCHAR(cursor, CI_ADDR);
		PUTCHAR(cursor, CILEN_ADDR);
		PUTLONG(cursor, sess->serverIP);
	}
	
	ctl->length = htons(pbuf->len - PPP_HEADER_LEN);
	return PPPOEERR_SUCCESS;
}


static int
processConfReq(session_struct_t *sess, struct pppoe_buf *pbuf) {
	unsigned char nak_buf[PPPOE_MTU], rej_buf[PPPOE_MTU];	/* where we construct a nak and rej packet */
	unsigned char *cursor = pbuf->data, *pnak = nak_buf, *prej = rej_buf;	
	struct session_options *options = sess->options;
	int len = pbuf->len;

	while (len) {
		int cilen;
		uint8 cicode;
		uint32 ciaddr;	

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "IPCP Config request process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		pppoe_token_log(TOKEN_IPCP, "IPCP Request config option 0x%02x\n", cicode);
	
		switch (cicode) {
			case CI_ADDRS:
				if (!options->neg_ipaddr || CILEN_ADDRS != cilen) {
					break;
				}
				
				break;
				
			case CI_COMPRESSTYPE:
				if (!options->neg_ipcompression || CILEN_COMPRESS != cilen) {
					break;
				}
				
				break;

			case CI_ADDR:
				if (!options->neg_ipaddr || CILEN_ADDR != cilen) {
					break;
				}

				if (!sess->ipaddr.ip && session_opt_perform(sess, SESSOPT_IPAPPLY)) {
					goto noipaddr;
				}

				ciaddr = GETLONG(cursor + 2);
				if (ciaddr == sess->ipaddr.ip) {
					goto end_switch;
				}
				
				PUTCHAR(pnak, CI_ADDR);
				PUTCHAR(pnak, CILEN_ADDR);
				PUTLONG(pnak, sess->ipaddr.ip);
				goto end_switch;
				
			case CI_MS_DNS1:
				if (!options->neg_ipmsdns || CILEN_ADDR != cilen) {
					break;
				}

				if (!sess->ipaddr.ip && session_opt_perform(sess, SESSOPT_IPAPPLY)) {
					goto noipaddr;
				}

				if (!sess->ipaddr.dns1) {
					break;
				}

				ciaddr = GETLONG(cursor + 2);
				if (ciaddr == sess->ipaddr.dns1) {
					goto end_switch;
				}

				PUTCHAR(pnak, CI_MS_DNS1);
				PUTCHAR(pnak, CILEN_ADDR);
				PUTLONG(pnak, sess->ipaddr.dns1);
				goto end_switch;

			case CI_MS_WINS1:
				if (!options->neg_ipmswins || CILEN_ADDR != cilen) {
					break;
				}
				break;
				
			case CI_MS_DNS2:
				if (!options->neg_ipmsdns || CILEN_ADDR != cilen) {
					break;
				}
				
				if (!sess->ipaddr.ip && session_opt_perform(sess, SESSOPT_IPAPPLY)) {
					goto noipaddr;
				}

				if (!sess->ipaddr.dns2) {
					break;
				}

				ciaddr = GETLONG(cursor + 2);
				if (ciaddr == sess->ipaddr.dns2) {
					goto end_switch;
				}
				
				PUTCHAR(pnak, CI_MS_DNS2);
				PUTCHAR(pnak, CILEN_ADDR);
				PUTLONG(pnak, sess->ipaddr.dns2);
				goto end_switch;
				
			case CI_MS_WINS2:
				if (!options->neg_ipmswins || CILEN_ADDR != cilen) {
					break;
				}
				break;
				
			default:
				pppoe_token_log(TOKEN_IPCP, "recv unknown option 0x%02x\n", cicode);
				break;
				
		}	
		
		/* copy rej ci */
		memcpy(prej, cursor, cilen);
		prej += cilen;

	end_switch:		
		len -= cilen;
		cursor += cilen;		
	}

	if (prej != rej_buf) {
		uint32 length = prej - rej_buf;

		pbuf_trim(pbuf, 0);
		if (pbuf_tailroom(pbuf) < length) {
			pppoe_log(LOG_WARNING, "processConfReq: config IPCP rej packet: "
								"pbuf tailroom not meet length %d\n", length);
			return PPPOEERR_ENOMEM;
		}

		memcpy(pbuf_put(pbuf, length), rej_buf, length);	
		return CONFREJ;
	} else if (pnak != nak_buf) {
		uint32 length = pnak - nak_buf;

		pbuf_trim(pbuf, 0);
		if (pbuf_tailroom(pbuf) < length) {
			pppoe_log(LOG_WARNING, "processConfReq: config IPCP nak packet: "
								"pbuf tailroom not meet length %d\n", length);
			return PPPOEERR_ENOMEM;
		}
		
		memcpy(pbuf_put(pbuf, length), nak_buf, length);
		return CONFNAK;
	}

	pppoe_token_log(TOKEN_IPCP, "session %u apply ip %u.%u.%u.%u\n", 
								sess->sid, HIPQUAD(sess->ipaddr.ip));
	
	if (sess->configState & STATE_RECVACK) {
		sess->state = SESSION_IPCPOPEN;				
		sess->configState = STATE_NONE;
	} else {
		sess->configState |= STATE_SENDACK;
	}
	return CONFACK;

noipaddr:
	sess->state = SESSION_LCPPRECLOSE;
	pppoe_log(LOG_WARNING, "session %d can`t apply ip addr\n", sess->sid);
	return PPPOEERR_SUCCESS;
}

static int
processConfAck(session_struct_t *sess, struct pppoe_buf *pbuf) {	
	struct session_options *options = sess->options;
	unsigned char *cursor = pbuf->data;	
	int len = pbuf->len;

	while (len) {
		int cilen;
		uint32 cicode;
		uint32 ciaddr;	

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "IPCP Config ACK process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		pppoe_token_log(TOKEN_IPCP, "IPCP ACK config option 0x%02x\n", cicode);
		
		switch (cicode) {
			case CI_ADDR:
				if (!options->neg_ipaddr || CILEN_ADDR != cilen) 
					return CODEREJ;

				ciaddr = GETLONG(cursor + 2);
				if (ciaddr != sess->serverIP)
					return CODEREJ;
				
				break;
				
			default:
				return CODEREJ;
				
		}	
		len -= cilen;
		cursor += cilen;
	}
				
	if (sess->configState & STATE_SENDACK) {
		sess->state = SESSION_IPCPOPEN;
		sess->configState = STATE_NONE;
	}else {
		sess->configState |= STATE_RECVACK;
	}
	return PPPOEERR_SUCCESS;
}

static int
processConfNak(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct session_options *options = sess->options;
	unsigned char *cursor = pbuf->data;	
	int len = pbuf->len;
	
	while (len) {
		int cilen;
		uint32 cicode;

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "IPCP Config NAK process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		pppoe_token_log(TOKEN_IPCP, "IPCP NAK config option 0x%02x\n", cicode);

		switch (cicode) {
		case CI_ADDR:
			if (!options->neg_ipaddr || CILEN_ADDR != cilen) 
				return CODEREJ;
			
			sess->state = SESSION_IPCPCLOSE;
			pbuf_trim(pbuf, 0);	
			return TERMREQ;
			
		default:
			break;
		}	
		
		len -= cilen;
		cursor += cilen;
	}

	return CODEREJ;
}

static int
processConfRej(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct session_options *options = sess->options;
	unsigned char *cursor = pbuf->data;	
	int len = pbuf->len;

	while (len) {
		int cilen;
		uint32 cicode;

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "IPCP Config NAK process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		pppoe_token_log(TOKEN_IPCP, "IPCP NAK config option 0x%02x\n", cicode);

		switch (cicode) {
			case CI_ADDR:
				if (!options->neg_ipaddr || CILEN_ADDR != cilen) 
					return CODEREJ;

				break;
				
			default:
				return CODEREJ;
				
		}	
		len -= cilen;
		cursor += cilen;
		
	}	

	pbuf_trim(pbuf, 0);
	return CONFREQ;
}


static inline int
processTermReq(session_struct_t *sess, struct pppoe_buf *pbuf) {
	session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
	if (SESSION_ONLINE == sess->state) {
		_manage_sess_offline(sess);
	} else if (SESSION_OFFLINEPRE != sess->state &&
		SESSION_OFFLINEPOST != sess->state){
		sess->state = SESSION_IPCPCLOSE;
	}

	pbuf_trim(pbuf, 0);
	return TERMACK;
}

static inline int
processTermAck(session_struct_t *sess, struct pppoe_buf *pbuf) {
	sess->state = SESSION_IPCPCLOSE;
	return PPPOEERR_SUCCESS;
}


static int
ipcp_process(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *pctl = (struct pppoe_ctl *)pbuf->data;
	int length = ntohs(pctl->length);
	int ret;
	
	if (sess->state < SESSION_IPCPPOSTOPEN || sess->state >= SESSION_IPCPCLOSE) {
		pppoe_log(LOG_WARNING, "IPCP process: session state error\n");
		return PPPOEERR_ESTATE;
	}

	if (unlikely(pbuf_trim(pbuf, length))) {
		pppoe_log(LOG_WARNING, "IPCP process: IPCP packet length error\n");
		return PPPOEERR_ELENGTH;
	}

	pbuf_pull(pbuf, sizeof(struct pppoe_ctl));

	if (pctl->code && pctl->code <= sizeof(ipcp_codenames) / sizeof(char *)) {
		pppoe_token_log(TOKEN_IPCP, "recv IPCP %s packet, ident %d, session %d\n", 
							ipcp_codenames[pctl->code - 1], pctl->ident, sess->sid);
	} else {
		pppoe_log(LOG_WARNING, "IPCP process: unknow code 0x%02x\n", pctl->code);
		return CODEREJ;
	}
	
	switch (pctl->code) {
		case CONFREQ:
			if (sess->state != SESSION_IPCPPOSTOPEN)
				return PPPOEERR_ESTATE;
			
			ret = processConfReq(sess, pbuf);
			break;

		case CONFACK:
			if (sess->state != SESSION_IPCPPOSTOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;

			ret = processConfAck(sess, pbuf);
			break;

		case CONFNAK:
			if (sess->state != SESSION_IPCPPOSTOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;
			
			ret = processConfNak(sess, pbuf);
			break;
			
		case CONFREJ:
			if (sess->state != SESSION_IPCPPOSTOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;
			
			ret = processConfRej(sess, pbuf);
			break;
			
		case TERMREQ:
			ret = processTermReq(sess, pbuf);
			break;
			
		case TERMACK:
			if (sess->state != SESSION_IPCPPOSTCLOSE) 
				return PPPOEERR_ESTATE;

			ret = processTermAck(sess, pbuf);
			break;

		default:
			return PPPOEERR_EINVAL;
	}

	return ret;
}


int
ipcp_proto_init(void) {
	return ppp_proto_register(PPP_IPCP, ipcp_process);
}

int 
ipcp_proto_exit(void) {
	return ppp_proto_unregister(PPP_IPCP);
}


