
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
#include "mem_cache.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"
#include "pppoe_thread.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_ppp.h"

#include "pppoe_chap.h"

static char *chap_codenames[] = {
	"Challenge", "Response", "Success", "Failure"
};

int
chapChallenge_packet(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *ctl;
	uint16 *proto;
	uint32 slen = strlen(sess->sname);
	uint32 length = sizeof(struct pppoe_ctl) + 1 + RADIUS_CHAP_CHAL_LEN + slen;

	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + length)) {
		pppoe_log(LOG_WARNING, "CHAP Challenge packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + length);
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_CHAP);
	
	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = CHAP_CHALLENGE;
	ctl->ident = sess->ident = rand_r(&sess->seed) & 0xff;

	*(ctl->data) = RADIUS_CHAP_CHAL_LEN;
	rand_bytes(&sess->seed, pbuf_put(pbuf, RADIUS_CHAP_CHAL_LEN + 1) + 1, RADIUS_CHAP_CHAL_LEN);
	memcpy(sess->challenge, ctl->data + 1, RADIUS_CHAP_CHAL_LEN);
	
	/* copy session sname */
	memcpy(pbuf_put(pbuf, slen), sess->sname, slen);
	ctl->length = htons(length);
	return PPPOEERR_SUCCESS;
}

int
chapResult_packet(session_struct_t *sess, struct pppoe_buf *pbuf, uint32 result) {
	struct pppoe_ctl *ctl;
	uint16 *proto;
	
	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl))) {
		pppoe_log(LOG_WARNING, "LCP Term packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_CHAP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = result ? CHAP_FAILURE : CHAP_SUCCESS;
	ctl->ident = sess->ident;
	ctl->length = htons(sizeof(struct pppoe_ctl));

	return PPPOEERR_SUCCESS;
}


static int
chapResponse_process(session_struct_t *sess, struct pppoe_buf *pbuf) {
	uint8 *username, *passwd;
	
	if (pbuf_may_pull(pbuf, RADIUS_PASSWORD_LEN + 1) ||
		RADIUS_PASSWORD_LEN != *(pbuf->data)) {
		pppoe_log(LOG_WARNING, "CHAP Resp Process: packet userpasswd length error\n");
		return PPPOEERR_ELENGTH;
	}

	passwd = pbuf->data + 1;
	pbuf_pull(pbuf, RADIUS_PASSWORD_LEN + 1);
	
	if (pbuf->len > (USERNAMESIZE - 1)) {
		pppoe_log(LOG_WARNING, "CHAP Resp Process: packet username length error\n");
		return PPPOEERR_ELENGTH;
	}	
	username = pbuf->data;
	
	memcpy(sess->passwd, passwd, RADIUS_PASSWORD_LEN);
	memcpy(sess->username, username, pbuf->len);
	session_timer_pause(sess, SESSION_TIMEOUT_TIMER);
	sess->state = SESSION_AUTH;
	pppoe_token_log(TOKEN_CHAP, "session %d recv CHAP Resp, username is %s\n", 
								sess->sid, sess->username);
	return PPPOEERR_SUCCESS;
}

static int
chap_process(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *pctl = (struct pppoe_ctl *)pbuf->data;
	int length = ntohs(pctl->length);

	if (SESSION_AUTHPRE != sess->state) {
		pppoe_log(LOG_WARNING, "CHAP process: session state(%d) error, "
								"so drop packet\n", sess->state);
		return PPPOEERR_ESTATE;
	}

	if (unlikely(pbuf_trim(pbuf, length))) {
		pppoe_log(LOG_WARNING, "CHAP process: CHAP packet length error\n");
		return PPPOEERR_ELENGTH;
	}

	pbuf_pull(pbuf, sizeof(struct pppoe_ctl));

	if (pctl->code && pctl->code <= sizeof(chap_codenames) / sizeof(char *)) {
		pppoe_token_log(TOKEN_CHAP, "recv CHAP %s packet, ident %d, session %d\n", 
							chap_codenames[pctl->code - 1], pctl->ident, sess->sid);
	} else {
		pppoe_log(LOG_WARNING, "CHAP process: unknow code 0x%02x, drop pack\n", pctl->code);
		return PPPOEERR_EINVAL;
	}

	/* only chap response packet need process */
	if (CHAP_RESPONSE == pctl->code) {
		return chapResponse_process(sess, pbuf);
	}

	return PPPOEERR_EINVAL;
}


int
chap_proto_init(void) {
	return ppp_proto_register(PPP_CHAP, chap_process);
}

int 
chap_proto_exit(void) {
	return ppp_proto_unregister(PPP_CHAP);
}


