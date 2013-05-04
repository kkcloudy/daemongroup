
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "md5.h"
#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"

#include "radius_packet.h"

int
radius_packet_init(struct pppoe_buf *pbuf, uint32 *seed, uint8 code) {
	struct radius_packet *packet;
	
	if (unlikely(!pbuf || !seed)) {
		pppoe_log(LOG_WARNING, "Radius Packet init fail (input error)\n");
		return PPPOEERR_EINVAL;
	}

	packet = (struct radius_packet *)pbuf_put(pbuf, sizeof(struct radius_packet));
	packet->code = code;
	packet->id = 0;
	packet->length = htons(sizeof(struct radius_packet));

	if (RADIUS_CODE_ACCESS_REQUEST == packet->code) {
		rand_bytes(seed, packet->authenticator, RADIUS_AUTHLEN);
	}
	
	return PPPOEERR_SUCCESS;
}


/* 
 * radius_addattr()
 * Add an attribute to a packet. The packet length is modified 
 * accordingly.
 * If data==NULL and dlen!=0 insert null attribute.
 */
int
radius_addattr(struct pppoe_buf *pbuf,
		uint8 type, uint32 vendor_id, uint8 vendor_type,
		uint32 value, uint8 *data, uint16 dlen) {
	struct radius_attr *a;
	uint16 vlen;

	if (unlikely(!pbuf)) 
		return PPPOEERR_EINVAL;

	if (type != RADIUS_ATTR_VENDOR_SPECIFIC) {
		if (dlen) {	/* If dlen != 0 it is a text/string attribute */
			vlen = dlen;
		} else {
			vlen = 4;	/* address, integer or time */
		}

		if (vlen > RADIUS_ATTR_VLEN) {
			pppoe_log(LOG_WARNING, "radius_addattr truncating RADIUS attribute "
									"type %d vendor(%d,%d) from %d to %d bytes [%s]",
									type, vendor_id, vendor_type, vlen,
									RADIUS_ATTR_VLEN, data);
			vlen = RADIUS_ATTR_VLEN;
		}

		if (pbuf_tailroom(pbuf) < (vlen + 2)) {
			pppoe_log(LOG_WARNING, "radius packet size %d too large, no more space", vlen + 2);
			return PPPOEERR_ENOMEM;
		}

		a = (struct radius_attr *)pbuf_put(pbuf, vlen + 2);
		a->t = type;
		a->l = vlen + 2;
		
		if (data) {
			memcpy(a->v.t, data, vlen);
		} else if (dlen) {
			memset(a->v.t, 0, vlen);
		} else {
			a->v.i = htonl(value);
		}
	} else {		/* Vendor specific */
		if (dlen) {	/* If dlen != 0 it is a text/string attribute */
			vlen = dlen;
		} else {
			vlen = 4;	/* address, integer or time */
		}

		if (vlen > RADIUS_ATTR_VLEN - 8) {
			pppoe_log(LOG_WARNING, "radius_addattr truncating RADIUS attribute "
									"type %d vendor(%d,%d) from %d to %d bytes [%s]",
									type, vendor_id, vendor_type, vlen,
									RADIUS_ATTR_VLEN - 8, data);
			vlen = RADIUS_ATTR_VLEN - 8;
		}

		if (pbuf_tailroom(pbuf) < (vlen + 8)) {
			pppoe_log(LOG_WARNING, "radius packet size %d too large, no more space", vlen + 8);
			return PPPOEERR_ENOMEM;
		}

		a = (struct radius_attr *)pbuf_put(pbuf, vlen + 2);

		a->t = type;
		a->l = vlen + 8;

		a->v.vv.i = htonl(vendor_id);
		a->v.vv.t = vendor_type;
		a->v.vv.l = vlen + 2;

		if (data) {
			memcpy(pbuf->data + 8, data, dlen);
		} else if (dlen) {
			memset(pbuf->data + 8, 0, dlen);
		} else {
			a->v.vv.v.i = htonl(value);
		}
	}

	/* set packet length */
	pbuf->data[2] = (pbuf->len >> 8) & 0xff;
	pbuf->data[3] = pbuf->len & 0xff;
	return PPPOEERR_SUCCESS;
}

/* 
 * radius_getattr()
 * Search for an attribute in a packet. Returns -1 if attribute is not found.
 * The first instance matching attributes will be skipped
 */
int
radius_getattr(struct pppoe_buf *pbuf, struct radius_attr **attr,
				uint8 type, uint32 vendor_id, uint8 vendor_type, int instance) {
	struct radius_attr *t;
	unsigned char *cursor;
	int len, alen, count = 0;
	
	if (unlikely(!pbuf)) 
		return PPPOEERR_EINVAL;

	cursor = pbuf->data;
	len = pbuf->len;

	while (len) {
		if (len < 2 || (alen = GETCHAR(cursor + 1)) > len) {
			pppoe_log(LOG_WARNING, "radius get attr failed(error length)\n");
			return PPPOEERR_EINVAL;
		}

		t = (struct radius_attr *)cursor;
		if (t->t != type) 
			goto end;

		if (RADIUS_ATTR_VENDOR_SPECIFIC == t->t &&
			(ntohl(t->v.vv.i) != vendor_id || t->v.vv.t != vendor_type))
			goto end;

		if (count == instance) {
			if (type == RADIUS_ATTR_VENDOR_SPECIFIC)
				*attr = (struct radius_attr *)&t->v.vv.t;
			else
				*attr = t;
			
			return PPPOEERR_SUCCESS;
		} else {
			count++;
		}

	end:
		len -= alen;
		cursor += alen;
	}
	
	return PPPOEERR_ENOEXIST;		/* Not found */
}


/* 
 * radius_pwencode()
 * Encode a password using MD5.
 */
static int
radius_pwencode(uint8 *dst, size_t dstsize, size_t *dstlen,
		uint8 *src, size_t srclen,
		uint8 *authenticator, char *secret, size_t secretlen)
{
	uint8_t output[RADIUS_MD5LEN] = {0};
	MD5_CTX context;
	size_t i = 0;
	size_t n = 0;

	memset(dst, 0, dstsize);

	/* Make dstlen multiple of 16 */
	if (srclen & 0x0f) {
		*dstlen = (srclen & 0xf0) + 0x10;	/* Padding 1 to 15 zeros */
	} else {
 		*dstlen = srclen;	/* No padding */
	}

	pppoe_log(LOG_INFO, "radius_pwencode dstlen = %u\n", *dstlen);
	/* Is dstsize too small ? */
	if (dstsize <= *dstlen) {
		pppoe_log(LOG_WARNING, "radius_pwencode dstsize %u < dstlen %u\n",
								dstsize, *dstlen);
		*dstlen = 0;
		return PPPOEERR_EINVAL;
	}

	/* Copy first 128 octets of src into dst */
	if (srclen > 128) {
		memcpy(dst, src, 128);
	} else {
		memcpy(dst, src, srclen);
	}

	/* Get MD5 hash on secret + authenticator */
	MD5Init(&context);
	MD5Update(&context, (uint8 *)secret, secretlen);
	MD5Update(&context, authenticator, RADIUS_AUTHLEN);
	MD5Final(output, &context);

	/* XOR first 16 octets of dst with MD5 hash */
	for (i = 0; i < RADIUS_MD5LEN; i++) {
		dst[i] ^= output[i];
	}

	/* Continue with the remaining octets of dst if any */
	for (n = 0;
	     n < 128 && n < (*dstlen - RADIUS_AUTHLEN); n += RADIUS_AUTHLEN)
	{
		MD5Init(&context);
		MD5Update(&context, (uint8 *)secret, secretlen);
		MD5Update(&context, dst + n, RADIUS_AUTHLEN);
		MD5Final(output, &context);
		for (i = 0; i < RADIUS_AUTHLEN; i++)
			dst[i + n + RADIUS_AUTHLEN] ^= output[i];
	}

	return PPPOEERR_SUCCESS;
}

int
radius_add_userpasswd(struct pppoe_buf *pbuf,
					uint8 *pwd, uint16 pwdlen,
					char *secret, size_t secretlen) {
	uint8 passwd[RADIUS_PWSIZE];
	size_t pwlen;

	radius_pwencode(passwd, RADIUS_PWSIZE, &pwlen,
			pwd, pwdlen, pbuf->data + 4, secret, secretlen);

	pwd = passwd;
	pwdlen = pwlen & 0xffff;

	return radius_addattr(pbuf, RADIUS_ATTR_USER_PASSWORD, 
						0, 0, 0, pwd, pwdlen);
}

/* 
 * radius_hmac_md5()
 * Calculate HMAC MD5 on a radius packet.
 */
int
radius_hmac_md5(struct radius_packet *pack,
			char *secret, size_t secretlen, uint8 *dst) {
	MD5_CTX context;
	uint8 digest[RADIUS_MD5LEN] = {0};
	uint8 k_ipad[65] = {0};
	uint8 k_opad[65] = {0};
	uint8 tk[RADIUS_MD5LEN] = {0};
	uint8 *key = NULL;
	size_t length = 0, key_len = 0;
	int i = 0;

	if (secretlen > 64) {	/* TODO: If Microsoft truncate to 64 instead */
		MD5Init(&context);
		MD5Update(&context, (uint8 *)secret, secretlen);
		MD5Final(tk, &context);
		key = tk;
		key_len = 16;
	} else {
		key = (uint8 *)secret;
		key_len = secretlen;
	}

	length = ntohs(pack->length);

	memset(k_ipad, 0x36, sizeof(k_ipad));
	memset(k_opad, 0x5c, sizeof(k_opad));

	for (i = 0; i < key_len; i++) {
		k_ipad[i] ^= key[i];
		k_opad[i] ^= key[i];
	}

	/* Perform inner MD5 */
	MD5Init(&context);
	MD5Update(&context, k_ipad, 64);
	MD5Update(&context, (uint8 *) pack, length);
	MD5Final(digest, &context);

	/* Perform outer MD5 */
	MD5Init(&context);
	MD5Update(&context, k_opad, 64);
	MD5Update(&context, digest, 16);
	MD5Final(digest, &context);

	memcpy(dst, digest, RADIUS_MD5LEN);

	return 0;
}

/* 
 * radius_acctreq_authenticator()
 * Update a packet with an accounting request authenticator
 */
int
radius_acctreq_authenticator(struct radius_packet *pack,
						char *secret, size_t secretlen) {
	/* From RFC 2866: Authenticator is the MD5 hash of:
	    Code + Identifier + Length + 16 zero octets + request attributes +
	    shared secret */
	MD5_CTX context;

	memset(pack->authenticator, 0, RADIUS_AUTHLEN);

	/* Get MD5 hash on secret + authenticator */
	MD5Init(&context);
	MD5Update(&context, (uint8 *)pack, ntohs(pack->length));
	MD5Update(&context, (uint8 *)secret, secretlen);
	MD5Final(pack->authenticator, &context);

	return 0;
}

/* 
 * radius_authresp_authenticator()
 * Update a packet with an authentication response authenticator
 */
int
radius_authresp_authenticator(struct radius_packet *pack,
				uint8 *req_auth, char *secret, size_t secretlen) {
	/* From RFC 2865: Authenticator is the MD5 hash of:
	    Code + Identifier + Length + request authenticator + request attributes +
	    shared secret */
	MD5_CTX context;

	memcpy(pack->authenticator, req_auth, RADIUS_AUTHLEN);

	/* Get MD5 hash on secret + authenticator */
	MD5Init(&context);
	MD5Update(&context, (uint8 *)pack, ntohs(pack->length));
	MD5Update(&context, (uint8 *)secret, secretlen);
	MD5Final(pack->authenticator, &context);

	return 0;
}

/*
 * radius_reply_check()
 * Check that the authenticator on a reply is correct.
 */
int
radius_reply_check(struct radius_packet *pack,
		struct radius_packet *pack_req,
		char *secret, size_t secretlen) {
	uint8_t auth[RADIUS_AUTHLEN] = {0};
	MD5_CTX context;

	MD5Init(&context);
	MD5Update(&context, (uint8 *)pack,
				RADIUS_PACKET_HEADSIZE - RADIUS_AUTHLEN);
	MD5Update(&context, pack_req->authenticator, RADIUS_AUTHLEN);
	MD5Update(&context, (uint8 *)pack + RADIUS_PACKET_HEADSIZE,
		ntohs(pack->length) - RADIUS_PACKET_HEADSIZE);
	MD5Update(&context, (uint8 *)secret, secretlen);
	MD5Final(auth, &context);

	return memcmp(pack->authenticator, auth, RADIUS_AUTHLEN);
}

/*
 * radius_request_check()
 * Check that the authenticator on an accounting , DM, COA request is correct.
 */
int
radius_request_check(struct radius_packet *pack,
					char *secret, size_t secretlen) {
	uint8 auth[RADIUS_AUTHLEN] = {0};
	uint8 padd[RADIUS_AUTHLEN] = {0};
	MD5_CTX context;

	memset(padd, 0, sizeof(padd));
	MD5Init(&context);
	MD5Update(&context, (uint8 *)pack,
			RADIUS_PACKET_HEADSIZE - RADIUS_AUTHLEN);
	MD5Update(&context, (uint8 *)padd, RADIUS_AUTHLEN);
	MD5Update(&context, (uint8 *)pack + RADIUS_PACKET_HEADSIZE,
			ntohs(pack->length) - RADIUS_PACKET_HEADSIZE);
	MD5Update(&context, (uint8 *)secret, secretlen);
	MD5Final(auth, &context);

	return memcmp(pack->authenticator, auth, RADIUS_AUTHLEN);
}

const char *
radius_terminate_cause_int2str(int int_reason) {
	switch (int_reason) {
		case RADIUS_TERMINATE_CAUSE_USER_REQUEST:
			return "UserRequest";
		case RADIUS_TERMINATE_CAUSE_LOST_CARRIER:
			return "LostCarrier";
		case RADIUS_TERMINATE_CAUSE_LOST_SERVICE:
			return "LostService";
		case RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT:
			return "IdleTimeout";
		case RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT:
			return "SessionTimeout";
		case RADIUS_TERMINATE_CAUSE_ADMIN_RESET:
			return "AdminReset";
		case RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT:
			return "AdminReboot";
		case RADIUS_TERMINATE_CAUSE_PORT_ERROR:
			return "PortError";
		case RADIUS_TERMINATE_CAUSE_NAS_ERROR:
			return "NasError";
		case RADIUS_TERMINATE_CAUSE_NAS_REQUEST:
			return "NasRequest";
		case RADIUS_TERMINATE_CAUSE_NAS_REBOOT:
			return "NasReboot";
		case RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED:
			return "PortUnneeded";
		case RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED:
			return "PortPreempted";
		case RADIUS_TERMINATE_CAUSE_PORT_SUSPEND:
			return "PortSuspend";
		case RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE:
			return "ServiceUnavailable";
		case RADIUS_TERMINATE_CAUSE_CALLBACK:
			return "CallBack";
		case RADIUS_TERMINATE_CAUSE_USER_ERROR:
			return "UserError";
		case RADIUS_TERMINATE_CAUSE_HOST_REQUEST:
			return "HostRequest";
		default:
			return "Unknow Reason";
	}
}

