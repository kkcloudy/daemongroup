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
#include "pppoe_thread.h"
#include "notifier_chain.h"
#include "pppoe_backup.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_manage.h"
#include "pppoe_ppp.h"

#include "pppoe_lcp.h"


static char *lcp_codenames[] = {
	"Configuration Request", 
	"Configuration Ack",
	"Configuration Nak",
	"Configuration Reject",
	"Termination Request",
	"Termination Ack",
	"Code Reject",
	"Protocol Reject",
	"Echo Request",
	"Echo Reply",
	"Discard Request",
	"Identification",
	"Time Remaining"
};


static char *lcp_cinames[] = {
	"Vendor Specific", 
	"Maximum Receive Unit", 
	"Async Control Character Map",
	"Authentication Type", 
	"Quality Protocol", 
	"Magic Number", 
	"Protocol Field Compression",
	"Address/Control Field Compression", 
	"FCS-Alternatives",
	"Self-Describing-Pad",
	"Numbered-Mode",
	"callback", 
	"max reconstructed receive unit for multilink", 
	"short sequence numbers for multilink",
	"endpoint discriminator", 
	"Multi-Link-Plus-Procedure", 
	"Link-Discriminator", 
	"LCP Authentication", 
	"Consistent Overhead Byte Stuffing",
	"Prefix Elision", 
	"MP Header Format", 
	"Internationalization", 
	"Simple Data Link"
};

static int
processConfReq(session_struct_t *sess, struct pppoe_buf *pbuf) {
	unsigned char nak_buf[PPPOE_MTU], rej_buf[PPPOE_MTU];	/* where we construct a nak and rej packet */
	unsigned char *cursor = pbuf->data, *pnak = nak_buf, *prej = rej_buf;
	struct session_options *options = sess->options;
	uint32 auth_type = 0, chap_mdtype = 0;
	unsigned char *magic = NULL;
	int len = pbuf->len;

	if (SESSION_INIT == sess->state) {
		sess->state = SESSION_LCPPREOPEN;
	}

	while (len) {
		int cilen;
		uint8 cicode;
		uint8 cichar;			/* Parsed char value */
		uint16 cishort;		/* Parsed short value */

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "LCP Config request process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		if (cicode < sizeof(lcp_cinames) / sizeof(char *)) {
			pppoe_token_log(TOKEN_LCP, "LCP Request config option: %s\n", lcp_cinames[cicode]);
		} else {
			pppoe_log(LOG_WARNING, "process Config request: recv unknown option 0x%02x\n", cicode);
			return CODEREJ;
		}
		
		switch (cicode) {
			case CI_MRU:
				if (!options->neg_mru || CILEN_SHORT != cilen) {
					break;
				}

				cishort = GETSHORT(cursor + 2);
				if (cishort > options->mru) {
					PUTCHAR(pnak, CI_MRU);		/* Nak CI */
					PUTCHAR(pnak, CILEN_SHORT);
					PUTSHORT(pnak, options->mru);
				} else {
					options->mru = cishort;
				}
				goto end_switch;

#if 0				
			case CI_ASYNCMAP:
				if (!options->neg_asyncmap || CILEN_LONG != cilen) {
					break;
				}

				goto end_switch;
#endif

			case CI_AUTHTYPE:
				if (cilen < CILEN_SHORT || 
					!(options->neg_upap || options->neg_chap || options->neg_eap)) {
					break;
				}
				
				cishort = GETSHORT(cursor + 2);
				if (PPP_PAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_SHORT != cilen ||
						(sess->auth_type  && !(sess->auth_type & AUTH_PAP))) {
						break;
					}
					
					if (!options->neg_upap) {
						PUTCHAR(pnak, CI_AUTHTYPE);	/* NAK it and suggest CHAP or EAP */
						if (options->neg_chap) {
							PUTCHAR(pnak, CILEN_CHAP);
							PUTSHORT(pnak, PPP_CHAP);
							PUTCHAR(pnak, CHAP_DIGEST(options->chap_mdtype));
						} else {
							PUTCHAR(pnak, CILEN_SHORT);
							PUTSHORT(pnak, PPP_EAP);
						}
						goto end_switch;
					}
					auth_type = AUTH_PAP;
				} else if (PPP_CHAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_CHAP != cilen ||
						(sess->auth_type  && !(sess->auth_type & AUTH_CHAP))) {
						break;
					}

					if (!options->neg_chap) {
						PUTCHAR(pnak, CI_AUTHTYPE);		/* NAK it and suggest CHAP or EAP */
						PUTCHAR(pnak, CILEN_SHORT);
						if (options->neg_eap) {
							PUTSHORT(pnak, PPP_EAP);
						} else {
							PUTSHORT(pnak, PPP_PAP);
						}
						goto end_switch;
					}

					cichar = GETCHAR(cursor + 4);	/* get digest type */
					if (!CHAP_CANDIGEST(options->chap_mdtype, cichar)){	/*We can't/won't do the requested type, */
						PUTCHAR(pnak, CI_AUTHTYPE);						/*suggest something else.*/
						PUTCHAR(pnak, CILEN_CHAP);
						PUTSHORT(pnak, PPP_CHAP);
						PUTCHAR(pnak, CHAP_DIGEST(options->chap_mdtype));
						goto end_switch;
					}
					auth_type = AUTH_CHAP;
					chap_mdtype = CHAP_MDTYPE_D(cichar);
				}else if (PPP_EAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_SHORT != cilen || 
						(sess->auth_type && !(sess->auth_type & AUTH_EAP))) {
						break;
					}
					
					if (!options->neg_eap) {				/* we don't want to do EAP */
						PUTCHAR(pnak, CI_AUTHTYPE);		/* NAK it and suggest CHAP or PAP */
						if (options->neg_chap) {
							PUTCHAR(pnak, CILEN_CHAP);
							PUTSHORT(pnak, PPP_CHAP);
							PUTCHAR(pnak, CHAP_DIGEST(options->chap_mdtype));
						} else {
							PUTCHAR(pnak, CILEN_SHORT);
							PUTSHORT(pnak, PPP_PAP);
						}
						goto end_switch;
					}
					auth_type = AUTH_PAP;
				} else {					/* we known auth type  */
					PUTCHAR(pnak, CI_AUTHTYPE);
					if (options->neg_eap) {
						PUTCHAR(pnak, CILEN_SHORT);
						PUTSHORT(pnak, PPP_EAP);
					} else if (options->neg_chap) {
						PUTCHAR(pnak, CILEN_CHAP);
						PUTSHORT(pnak, PPP_CHAP);
						PUTCHAR(pnak, CHAP_DIGEST(options->chap_mdtype));
					} else {	/*this is PAP */
						PUTCHAR(pnak, CILEN_SHORT);
						PUTSHORT(pnak, PPP_PAP);
					}
				}
				
				goto end_switch;
				
#if 0				
			case CI_QUALITY:
				/* pppoe can`t accept*/
				goto rej_conf;
#endif

			case CI_MAGICNUMBER:
				if (!options->neg_magicnumber || cilen != CILEN_LONG) {
					break;
				}

				if (!GETLONG(cursor + 2)) {
					pppoe_log(LOG_WARNING, "session %d recv lcp config magicnum is zero\n", sess->sid);
					return CODEREJ;
				}	
					
				/* may be need check magicnumber */
				magic = cursor + 2;
				
				goto end_switch;

#if 0
			case CI_PCOMPRESSION:
				/* pppoe can`t accept*/
				goto rej_conf;
				
			case CI_ACCOMPRESSION:
				/* pppoe can`t accept, this may be need support */
				goto rej_conf;

			case CI_MRRU:
				/* pppoe can`t accept*/
				goto rej_conf;

			case CI_SSNHF:
				/* pppoe can`t accept, this may be need support */
				goto rej_conf;

			case CI_EPDISC:
				/* pppoe can`t accept*/
				goto rej_conf;
#endif

			default:
				pppoe_token_log(TOKEN_LCP, "recv unsupport option [%s]\n", lcp_cinames[cicode]);
				break;
		}
		
		/* copy rej ci */
		memcpy(prej, cursor, cilen);
		prej += cilen;
		goto end_switch;

	end_switch:		
		len -= cilen;
		cursor += cilen;
	}			

	if (prej != rej_buf) {
		uint32 length = prej - rej_buf;
		
		pbuf_trim(pbuf, 0);
		if (pbuf_tailroom(pbuf) < length) {
			pppoe_log(LOG_WARNING, "processConfReq: config LCP rej packet: "
								"pbuf tailroom not meet length %d\n", length);
			return PPPOEERR_ENOMEM;
		}

		memcpy(pbuf_put(pbuf, length), rej_buf, length);
		return CONFREJ;
	} else if (pnak != nak_buf) {
		uint32 length = pnak - nak_buf;
	
		pbuf_trim(pbuf, 0);
		if (pbuf_tailroom(pbuf) < length) {
			pppoe_log(LOG_WARNING, "processConfReq: config LCP nak packet: "
								"pbuf tailroom not meet length %d\n", length);
			return PPPOEERR_ENOMEM;
		}
		
		memcpy(pbuf_put(pbuf, length), nak_buf, length);
		return CONFNAK;
	} 


	if (auth_type) {
		sess->auth_type = auth_type;
		sess->chap_mdtype = chap_mdtype;
	}

	if (magic) {
		memcpy(sess->peermagic, magic, MAGIC_LEN);
	}
		
	if (sess->configState & STATE_RECVACK) {
		pppoe_token_log(TOKEN_LCP, "session %u apply auth type %s\n", 
					sess->sid, AUTH_CHAP == sess->auth_type ? "Auth Chap" : "UNKnown");
		sess->state = SESSION_LCPOPEN;
		sess->configState = STATE_NONE;
	} else {
		sess->configState |= STATE_SENDACK;
	}
	
	return CONFACK;
}


/*when recv this config */
static int
processConfAck(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct session_options *options = sess->options;
	unsigned char *cursor = pbuf->data;	
	int len = pbuf->len;

	while (len) {
		int cilen;
		uint32 cicode;
		uint8 cichar;
		uint16 cishort;

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "LCP Config ACK process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		if (cicode < sizeof(lcp_cinames) / sizeof(char *)) {
			pppoe_token_log(TOKEN_LCP, "LCP ACK config option: %s\n", lcp_cinames[cicode]);
		} else {
			pppoe_log(LOG_WARNING, "process Config ACK: recv unknown option %d\n", cicode);
			return CODEREJ;
		}		

		switch (cicode) {
			case CI_MRU:
				if (!options->neg_mru || CILEN_SHORT != cilen)
					return CODEREJ;

				if (GETSHORT(cursor + 2) > options->mru)
					return CODEREJ;
				
				break;
				
			case CI_AUTHTYPE:
				if (cilen < CILEN_SHORT || 
					!(options->neg_upap || options->neg_chap || options->neg_eap))
					return CODEREJ;

				cishort = GETSHORT(cursor + 2);

				if (PPP_PAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_SHORT != cilen || !options->neg_upap ||
						(sess->auth_type  && !(sess->auth_type & AUTH_PAP)))
						return CODEREJ;

					sess->auth_type = AUTH_PAP;
				} else if (PPP_CHAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_CHAP != cilen || !options->neg_chap ||
						(sess->auth_type  && !(sess->auth_type & AUTH_CHAP)))
						return CODEREJ;

					cichar = GETCHAR(cursor + 4);	/* get digest type */
					if (!CHAP_CANDIGEST(options->chap_mdtype, cichar))	/*We can't/won't do the requested type, */
						return CODEREJ;
					
					sess->auth_type = AUTH_CHAP;
					sess->chap_mdtype = CHAP_MDTYPE_D(cichar);
				}else if (PPP_EAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (CILEN_SHORT != cilen || !options->neg_eap ||
						(sess->auth_type && !(sess->auth_type & AUTH_EAP)))
						return CODEREJ;

					sess->auth_type = AUTH_EAP;
				} else {	/* we unknown auth type  */
					return CODEREJ;
				}
				
				break;
				
			case CI_MAGICNUMBER:
				if (!options->neg_magicnumber || cilen != CILEN_LONG)
					return CODEREJ;

				if (memcmp(sess->magic, cursor + 2, MAGIC_LEN))
					return CODEREJ;

				break;
				
			default:
				return CODEREJ;
		}	
		
		len -= cilen;
		cursor += cilen;
	}

	if (sess->configState & STATE_SENDACK) {
		sess->state = SESSION_LCPOPEN;
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
	uint32 auth_type = 0, chap_mdtype = 0;
	unsigned char *magic = NULL;
	int len = pbuf->len;
	
	while (len) {
		int cilen;
		uint32 cicode;
		uint8 cichar;
		uint16 cishort;

		if (len < 2 || (cilen = GETCHAR(cursor + 1)) < 2 || cilen > len) {
			pppoe_log(LOG_WARNING, "LCP Config NAK process: CI length error\n");
			return CODEREJ;
		}

		cicode = GETCHAR(cursor);
		if (cicode < sizeof(lcp_cinames) / sizeof(char *)) {
			pppoe_token_log(TOKEN_LCP, "LCP NAK config option: %s\n", lcp_cinames[cicode]);
		} else {
			pppoe_log(LOG_WARNING, "process Config NAK: recv unknown option %d\n", cicode);
			return CODEREJ;
		}		

		switch (cicode) {
			case CI_MRU:
				if (!options->neg_mru || CILEN_SHORT != cilen)
					return CODEREJ;

				cishort = GETSHORT(cursor + 2);
				if (cishort > options->mru) {
					pbuf_trim(pbuf, 0);
					return TERMREQ;		/*can not support, so exit*/
				} else {
					options->mru = cishort;
				}

				break;
				
			case CI_AUTHTYPE:
				if (cilen < CILEN_SHORT || 
					!(options->neg_upap || options->neg_chap || options->neg_eap))
					return CODEREJ;

				cishort = GETSHORT(cursor + 2);

				if (PPP_PAP == cishort) {
					if (!options->neg_upap) {
						pbuf_trim(pbuf, 0);
						return TERMREQ;		/*can not support, so exit*/
					}
					
					if (CILEN_SHORT != cilen ||
						(sess->auth_type  && !(sess->auth_type & AUTH_PAP)))
						return CODEREJ;
					
					auth_type = cishort;
				} else if (PPP_CHAP == cishort) {
					/*this need check sess is already assces other auth type*/
					if (!options->neg_chap)
						return CODEREJ;

					if (CILEN_CHAP != cilen || 
						(sess->auth_type  && !(sess->auth_type & AUTH_CHAP)))
						return CODEREJ;
					

					cichar = GETCHAR(cursor + 4);	/* get digest type */
					if (!CHAP_CANDIGEST(options->chap_mdtype, cichar)) {	/*We can't/won't do the requested type, */
						pbuf_trim(pbuf, 0);
						return TERMREQ;		/*can not support, so exit*/
					}
					auth_type = cishort;
					chap_mdtype = cichar;
				}else if (PPP_EAP == cishort) {
					if (!options->neg_eap)
						return CODEREJ;

					if (CILEN_CHAP != cilen || 
						(sess->auth_type  && !(sess->auth_type & AUTH_EAP)))
						return CODEREJ;

					auth_type = cishort;
				} else {	/* we unknown auth type  */
					return CODEREJ;
				}

				break;
				
			case CI_MAGICNUMBER:
				if (!options->neg_magicnumber || cilen != CILEN_LONG)
					return CODEREJ;

				if (!memcmp(sess->magic, cursor + 2, MAGIC_LEN))
					return CODEREJ;

				magic = cursor + 2;
				break;
				
			default:
				return CODEREJ;
		}		

		len -= cilen;
		cursor += cilen;
	}


	pbuf_trim(pbuf, 0);
	
	/* config request mru */
	cursor = pbuf_put(pbuf, CILEN_SHORT);
	PUTCHAR(cursor, CI_MRU);
	PUTCHAR(cursor, CILEN_SHORT);
	PUTSHORT(cursor, options->mru);

	/* config request authtype */
	if (auth_type) {
		if (PPP_CHAP == auth_type) {
			cursor = pbuf_put(pbuf, CILEN_CHAP);
			PUTCHAR(cursor, CI_AUTHTYPE);
			PUTCHAR(cursor, CILEN_CHAP);
			PUTSHORT(cursor, PPP_CHAP);
			PUTCHAR(cursor, chap_mdtype);
		} else {
			cursor = pbuf_put(pbuf, CILEN_SHORT);
			PUTCHAR(cursor, CI_AUTHTYPE);
			PUTCHAR(cursor, CILEN_SHORT);
			PUTSHORT(cursor, auth_type);
		}
	} else {
		cursor = pbuf_put(pbuf, CILEN_CHAP);
		PUTCHAR(cursor, CI_AUTHTYPE);
		PUTCHAR(cursor, CILEN_CHAP);
		PUTSHORT(cursor, PPP_CHAP);
		PUTCHAR(cursor, CHAP_DIGEST(options->chap_mdtype));
	}

	/* config request MAGIC */
	cursor = pbuf_put(pbuf, CILEN_LONG);
	PUTCHAR(cursor, CI_MAGICNUMBER);
	PUTCHAR(cursor, CILEN_LONG);
	memcpy(cursor, magic ? : sess->magic, MAGIC_LEN);

	return CONFREQ;
}

static int
processConfRej(session_struct_t *sess, struct pppoe_buf *pbuf) {
	pbuf_trim(pbuf, 0);
	return TERMREQ;		/*not support , so exit*/
}

static inline int
processTermReq(session_struct_t *sess, struct pppoe_buf *pbuf) {	
	session_termcause_setup(sess, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
	session_opt_unregister(sess, SESSOPT_LCPTERMREQ);

	if (SESSION_ONLINE == sess->state) {
		_manage_sess_offline(sess);
	} else if (SESSION_OFFLINEPRE != sess->state &&
		SESSION_OFFLINEPOST != sess->state){
		sess->state = SESSION_LCPCLOSE;
	}

	pbuf_trim(pbuf, 0);
	return TERMACK;
}

static inline int
processTermAck(session_struct_t *sess, struct pppoe_buf *pbuf) {
	session_opt_unregister(sess, SESSOPT_LCPTERMREQ);
	sess->state = SESSION_LCPCLOSE;
	return PPPOEERR_SUCCESS;
}

static inline int
processCodeRej(session_struct_t *sess, struct pppoe_buf *pbuf) {
	pbuf_trim(pbuf, 0);
	return TERMREQ;	
}

static inline int
processEchoRep(session_struct_t *sess, struct pppoe_buf *pbuf) {
	if (pbuf->len < MAGIC_LEN) {
		pppoe_log(LOG_WARNING, "LCP Echo reply process: packet length error\n");
		return PPPOEERR_ELENGTH;
	}
	
	if (memcmp(sess->peermagic, pbuf->data, MAGIC_LEN)) {
		pppoe_log(LOG_WARNING, "LCP Echo reply process: magic error\n");
		return PPPOEERR_EINVAL;
	}

	/* clear echo lose times */
	sess->echoLoseTimes = 0;
	return PPPOEERR_SUCCESS;
}

int
lcpConfigReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct session_options *options = sess->options;
	struct pppoe_ctl *ctl;
	uint16 *proto;
	uint8 *curosr;

	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl))) {
		pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_LCP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = CONFREQ;
	ctl->ident = sess->ident = 0x1;
	
	/* config request mru */
	if (pbuf_tailroom(pbuf) < CILEN_SHORT) {
		pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
								CILEN_SHORT);
		return PPPOEERR_ENOMEM;
	}
	
	curosr = pbuf_put(pbuf, CILEN_SHORT);
	PUTCHAR(curosr, CI_MRU);
	PUTCHAR(curosr, CILEN_SHORT);
	PUTSHORT(curosr, options->mru);


	/* config request authtype */
	switch (AUTH_NONE == sess->auth_type ? AUTH_CHAP : AUTH_NONE) {
	case AUTH_NONE:
		break;

#if 0
	case AUTH_PAP:
		if (pbuf_tailroom(pbuf) < CILEN_SHORT) {
			pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
									CILEN_SHORT);
			return PPPOEERR_ENOMEM;
		}

		curosr = pbuf_put(pbuf, CILEN_SHORT);
		PUTCHAR(curosr, CI_MRU);
		PUTCHAR(curosr, CILEN_SHORT);
		PUTSHORT(curosr, PPP_PAP);
		break;
#endif			
			
	case AUTH_CHAP:
		if (pbuf_tailroom(pbuf) < CILEN_CHAP) {
			pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
									CILEN_CHAP);
			return PPPOEERR_ENOMEM;
		}

		curosr = pbuf_put(pbuf, CILEN_CHAP);
		PUTCHAR(curosr, CI_AUTHTYPE);
		PUTCHAR(curosr, CILEN_CHAP);
		PUTSHORT(curosr, PPP_CHAP);
		PUTCHAR(curosr, CHAP_DIGEST(options->chap_mdtype));
		break;

#if 0
	case AUTH_EAP:
		if (pbuf_tailroom(pbuf) < CILEN_SHORT) {
			pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
									CILEN_SHORT);
			return PPPOEERR_ENOMEM;
		}

		curosr = pbuf_put(pbuf, CILEN_SHORT);
		PUTCHAR(curosr, CI_AUTHTYPE);
		PUTCHAR(curosr, CILEN_SHORT);
		PUTSHORT(curosr, PPP_EAP);
		break;
#endif			
	}

	/* config request MAGIC */
	if (pbuf_tailroom(pbuf) < CILEN_LONG) {
		pppoe_log(LOG_WARNING, "LCP Config Request packet: pbuf tailroom not meet length %d\n", 
								CILEN_LONG);
		return PPPOEERR_ENOMEM;
	}

	curosr = pbuf_put(pbuf, CILEN_LONG);
	PUTCHAR(curosr, CI_MAGICNUMBER);
	PUTCHAR(curosr, CILEN_LONG);
	memcpy(curosr, sess->magic, MAGIC_LEN);

	ctl->length = htons(pbuf->len - PPP_HEADER_LEN);	
	return PPPOEERR_SUCCESS;
}

/*termType: 0 termreq, 1 termack*/
int
lcpTerm_packet(session_struct_t *sess, struct pppoe_buf *pbuf, uint32 termType) {
	struct pppoe_ctl *ctl;
	uint16 *proto;

	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl))) {
		pppoe_log(LOG_WARNING, "LCP Term packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl));
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_LCP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl));
	ctl->code = termType ? TERMACK: TERMREQ;
	ctl->ident = 0;
	ctl->length = htons(sizeof(struct pppoe_ctl));
	return PPPOEERR_SUCCESS;
}

int
lcpEchoReq_packet(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *ctl;
	uint16 *proto;
	
	if (pbuf_tailroom(pbuf) < (PPP_HEADER_LEN + sizeof(struct pppoe_ctl) + MAGIC_LEN)) {
		pppoe_log(LOG_WARNING, "LCP Echo Request packet: pbuf tailroom not meet length %d\n", 
								PPP_HEADER_LEN + sizeof(struct pppoe_ctl) + MAGIC_LEN);
		return PPPOEERR_ENOMEM;
	}

	proto = (uint16 *)pbuf_put(pbuf, PPP_HEADER_LEN);
	*proto = htons(PPP_LCP);

	ctl = (struct pppoe_ctl *)pbuf_put(pbuf, sizeof(struct pppoe_ctl) + MAGIC_LEN);
	ctl->code = ECHOREQ;
	ctl->ident = 0;
	ctl->length = htons(sizeof(struct pppoe_ctl) + MAGIC_LEN);
	memcpy(ctl->data, sess->magic, MAGIC_LEN);
	return PPPOEERR_SUCCESS;
}



static int
lcp_process(session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct pppoe_ctl *pctl = (struct pppoe_ctl *)pbuf->data;
	int length = ntohs(pctl->length);
	int ret;

	if (sess->state < SESSION_INIT || sess->state >=  SESSION_LCPCLOSE) {
		pppoe_log(LOG_WARNING, "LCP process: session state(%d) is not lcp config, "
								"so drop packet\n", sess->state);
		return PPPOEERR_ESTATE;
	}

	if (unlikely(pbuf_trim(pbuf, length))) {
		pppoe_log(LOG_WARNING, "LCP process: LCP packet length error\n");
		return PPPOEERR_ELENGTH;
	}

	pbuf_pull(pbuf, sizeof(struct pppoe_ctl));

	if (pctl->code && pctl->code <= sizeof(lcp_codenames) / sizeof(char *)) {
		pppoe_token_log(TOKEN_LCP, "recv LCP %s packet, ident %d, session %d\n", 
							lcp_codenames[pctl->code - 1], pctl->ident, sess->sid);
	} else {
		pppoe_log(LOG_WARNING, "LCP process: unknow code 0x%02x\n", pctl->code);
		return CODEREJ;
	}

	switch (pctl->code) {
		case CONFREQ:
			if (sess->state > SESSION_LCPPOSTOPEN)
				return PPPOEERR_ESTATE;
			
			ret = processConfReq(sess, pbuf);
			break;
			
		case CONFACK:
			if (sess->state != SESSION_LCPPOSTOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;
			
			ret = processConfAck(sess, pbuf);
			break;
			
		case CONFNAK:
			if (sess->state != SESSION_LCPPOSTOPEN)
				return PPPOEERR_ESTATE;
			
			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;
			
			ret = processConfNak(sess, pbuf);
			break;
			
		case CONFREJ:
			if (sess->state !=  SESSION_LCPPOSTOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;
			
			ret = processConfRej(sess, pbuf);
			break;

		case TERMREQ:
			if (sess->state >=  SESSION_LCPCLOSE) 
				return PPPOEERR_ESTATE;

			ret = processTermReq(sess, pbuf);
			break;
			
		case TERMACK:
			if (sess->state != SESSION_IPCPPOSTCLOSE) 
				return PPPOEERR_ESTATE;

			ret = processTermAck(sess, pbuf);
			break;

		case CODEREJ:
			if (sess->state >=  SESSION_LCPOPEN)
				return PPPOEERR_ESTATE;

			if (pctl->ident != sess->ident) 
				return PPPOEERR_EINVAL;

			ret = processCodeRej(sess, pbuf);
			break;
			

		/* kernel process echo request packet */	
		case ECHOREP:
			if (sess->state >=  SESSION_LCPCLOSE) 
				return PPPOEERR_ESTATE;

			ret = processEchoRep(sess, pbuf);
			break;

		default:
			return PPPOEERR_EINVAL;
	}

	return ret;
}

int
lcp_proto_init(void) {
	return ppp_proto_register(PPP_LCP, lcp_process);
}

int 
lcp_proto_exit(void) {
	return ppp_proto_unregister(PPP_LCP);
}

