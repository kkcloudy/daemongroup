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
#include "mem_cache.h"
#include "pppoe_thread.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_ppp.h"

#include "pppoe_ccp.h"

static char *ccp_codenames[] = {
	"Configuration Request", 
	"Configuration Ack",
	"Configuration Nak",
	"Configuration Reject",
	"Termination Request",
	"Termination Ack",
	"Code Reject",
	"UNUSED",
	"UNUSED",
	"UNUSED",
	"UNUSED",
	"UNUSED",
	"UNUSED",
	"Reset Request",
	"Reset Ack",
};

static int
processConfReq(session_struct_t *sess, struct pppoe_buf *pbuf) {
	pbuf_trim(pbuf, 0);
	return TERMREQ;
}

static int
processConfAck(session_struct_t *sess, struct pppoe_buf *pbuf) {				
	return PPPOEERR_SUCCESS;
}

static int
processConfNak(session_struct_t *sess, struct pppoe_buf *pbuf) {
	sess->state = SESSION_LCPPRECLOSE;
	return PPPOEERR_SUCCESS;
}

static int
processConfRej(session_struct_t *sess, struct pppoe_buf *pbuf) {
	sess->state = SESSION_LCPPRECLOSE;
	return PPPOEERR_SUCCESS;
}

static inline int
processTermReq(session_struct_t *sess, struct pppoe_buf *pbuf) {
	pbuf_trim(pbuf, 0);
	return TERMACK;
}

static inline int
processTermAck(session_struct_t *sess, struct pppoe_buf *pbuf) {
	return PPPOEERR_SUCCESS;
}


static inline int
processCodeRej(session_struct_t *sess, struct pppoe_buf *pbuf) {
	pbuf_trim(pbuf, 0);
	return TERMREQ;	
}

int
ccpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *ctl;
	uint16 *proto;
	
	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl))) {
		pppoe_log(LOG_WARNING, "CCP Config Request packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_CCP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = CONFREQ;
	ctl->ident = sess->ident = 0x1;
	ctl->length = htons(sizeof(struct pppoe_ctl));

	return PPPOEERR_SUCCESS;
}



static int
ccp_process(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *pctl = (struct pppoe_ctl *)pbuf->data;
	int length = ntohs(pctl->length);
	int ret;

	if (sess->state < SESSION_LCPOPEN || sess->state >=  SESSION_LCPCLOSE) {
		pppoe_log(LOG_WARNING, "CCP process: session ppp link is not lcp open, so drop packet\n");
		return PPPOEERR_ESTATE;
	}

	if (unlikely(pbuf_trim(pbuf, length))) {
		pppoe_log(LOG_WARNING, "CCP process: CCP packet length error\n");
		return PPPOEERR_ELENGTH;
	}

	pbuf_pull(pbuf, sizeof(struct pppoe_ctl));

	if (pctl->code && pctl->code <= sizeof(ccp_codenames) / sizeof(char *)) {
		pppoe_token_log(TOKEN_LCP, "recv CCP %s packet, ident %d, session %d\n", 
							ccp_codenames[pctl->code - 1], pctl->ident, sess->sid);
	} else {
		pppoe_log(LOG_WARNING, "CCP process: unknow code 0x%02x\n", pctl->code);
		return CODEREJ;
	}

	switch (pctl->code) {
		case CONFREQ:
			ret = processConfReq(sess, pbuf);
			break;
			
		case CONFACK:
			ret = processConfAck(sess, pbuf);
			break;

		case CONFNAK:
			ret = processConfNak(sess, pbuf);
			break;
		
		case CONFREJ:
			ret = processConfRej(sess, pbuf);
			break;

		case TERMREQ:
			ret = processTermReq(sess, pbuf);
			break;
			
		case TERMACK:
			ret = processTermAck(sess, pbuf);
			break;

		case CODEREJ:
			ret = processCodeRej(sess, pbuf);
			break;

		default:
			return PPPOEERR_EINVAL;
	}
	
	return ret;
}


int
ccp_proto_init(void) {
	return ppp_proto_register(PPP_CCP, ccp_process);
}

int 
ccp_proto_exit(void) {
	return ppp_proto_unregister(PPP_CCP);
}

