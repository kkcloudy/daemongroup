#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include "ws_snmpd_engine.h"

#include "acsample_util.h"




/*MD5加密可能用到的函数--------------*/

void byteReverse(unsigned char *buf, size_t longs)
{
    uint32_t t;
    do {
      t = (uint32_t)((uint16_t)(buf[3] << 8 | buf[2])) << 16 |
	            ((uint16_t)(buf[1] << 8 | buf[0]));
      *(uint32_t *) buf = t;
      buf += 4;
    } while (--longs);
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )


/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Transform(uint32_t buf[4], uint32_t const in[16])
{
    register uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}



/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Update(struct MD5Context *ctx, unsigned char const *buf, size_t len)
{
    uint32_t t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint32_t) len << 3)) < t)
	ctx->bits[1]++;		/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) ctx->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32_t *) ctx->in);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy(ctx->in, buf, 64);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32_t *) ctx->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
    size_t count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;
    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, count);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32_t *) ctx->in);

	/* Now fill the next block with 56 bytes */
	memset(ctx->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((uint32_t *) ctx->in)[14] = ctx->bits[0];
    ((uint32_t *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (uint32_t *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(ctx));	/* In case it's sensitive */
}


/* 
 * radius_pwencode()
 * Encode a password using MD5.
 */
int radius_pwencode(struct radius_t *this, 
		uint8_t *dst, size_t dstsize,
		size_t *dstlen, 
		uint8_t *src, size_t srclen, 
		uint8_t *authenticator, 
		char *secret, size_t secretlen) {

	unsigned char output[RADIUS_MD5LEN];
	MD5_CTX context;
	size_t i, n;

	memset(dst, 0, dstsize);

	/* Make dstlen multiple of 16 */
	if (srclen & 0x0f) 
		*dstlen = (srclen & 0xf0) + 0x10; /* Padding 1 to 15 zeros */
	else
		*dstlen = srclen;                 /* No padding */

	/* Is dstsize too small ? */
	if (dstsize <= *dstlen) {
		*dstlen = 0;
		return -1;
	}

	/* Copy first 128 octets of src into dst */
	if (srclen > 128) 
		memcpy(dst, src, 128);
	else
		memcpy(dst, src, srclen);

	/* Get MD5 hash on secret + authenticator */
	MD5Init(&context);
	MD5Update(&context, (uint8_t*) secret, secretlen);
	MD5Update(&context, authenticator, RADIUS_AUTHLEN);
	MD5Final(output, &context);

	/* XOR first 16 octets of dst with MD5 hash */
	for (i = 0; i < RADIUS_MD5LEN; i++)
		dst[i] ^= output[i];

	/* if (*dstlen <= RADIUS_MD5LEN) return 0;  Finished */

	/* Continue with the remaining octets of dst if any */
	for (n = 0; 
			n < 128 && n < (*dstlen - RADIUS_AUTHLEN); 
			n += RADIUS_AUTHLEN) {
		MD5Init(&context);
		MD5Update(&context, (uint8_t*) secret, secretlen);
		MD5Update(&context, dst + n, RADIUS_AUTHLEN);
		MD5Final(output, &context);
		for (i = 0; i < RADIUS_AUTHLEN; i++)
			dst[i + n + RADIUS_AUTHLEN] ^= output[i];
	}    

	return 0;
}


/* 
 * radius_default_pack()
 * Return an empty packet which can be used in subsequent to 
 * radius_addattr()
 */
	int
radius_default_pack(struct radius_t *this,
		struct radius_packet_t *pack, 
		int code)
{
	memset(pack, 0, RADIUS_PACKSIZE);
	pack->code = code;
	pack->id = 0; /* Let the send procedure queue the packet and assign id */
	pack->length = htons(RADIUS_HDRSIZE);

	if (fread(pack->authenticator, 1, RADIUS_AUTHLEN, this->urandom_fp) != RADIUS_AUTHLEN) {
		syslog(LOG_INFO,"fread() failed");	//test
		return EAG_RETURN_CODE_ERROR;
	}
	return EAG_RETURN_CODE_OK;
}


/* 
 * radius_addattr()
 * Add an attribute to a packet. The packet length is modified 
 * accordingly.
 * If data==NULL and dlen!=0 insert null attribute.
 */
int 
radius_addattr(struct radius_t *this, struct radius_packet_t *pack, 
		uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
		uint32_t value, uint8_t *data, uint16_t dlen) {
	struct radius_attr_t *a = NULL;
	char passwd[RADIUS_PWSIZE] = {0};
	uint16_t length = ntohs(pack->length);
	uint16_t vlen = 0;
	size_t pwlen = 0;

	a = (struct radius_attr_t *)((uint8_t*)pack + length);

	if (type == RADIUS_ATTR_USER_PASSWORD) {
		radius_pwencode(this, 
				(uint8_t*) passwd, RADIUS_PWSIZE, 
				&pwlen, 
				data, dlen, 
				pack->authenticator,
				this->secret, this->secretlen);
		data = (uint8_t *)passwd;
		syslog(LOG_INFO, "user password encoded:%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x", \
				data[0], data[1], data[2],data[3],data[4],data[5], data[6], \
				data[7], data[8],data[9], data[10],data[11],data[12] );//test
		dlen = (uint16_t)pwlen;
	}

	if (type != RADIUS_ATTR_VENDOR_SPECIFIC) {
		if (dlen) { /* If dlen != 0 it is a text/string attribute */
			vlen = dlen;
		}
		else {
			vlen = 4; /* address, integer or time */
		}

		if (vlen > RADIUS_ATTR_VLEN) {
			syslog(LOG_INFO,"truncating RADIUS attribute type %d vendor(%d,%d) from %d to %d bytes [%s]", 
					type, vendor_id, vendor_type, vlen, RADIUS_ATTR_VLEN, data); //test
			vlen = RADIUS_ATTR_VLEN;
		}

		if ((length+vlen+2) > RADIUS_PACKSIZE) {
			syslog(LOG_INFO,"radius packet size %d too large,no more space!", (length+vlen+2));	//test
			return EAG_RETURN_CODE_OUT_OF_MEMORY;
		}

		length += vlen + 2;

		pack->length = htons(length);

		a->t = type;
		a->l = vlen+2;

		if (data)
			memcpy(a->v.t, data, vlen);
		else if (dlen)
			memset(a->v.t, 0, vlen);
		else
			a->v.i = htonl(value);
	}
	else { /* Vendor specific */
		if (dlen) { /* If dlen != 0 it is a text/string attribute */
			vlen = dlen;
		}
		else {
			vlen = 4; /* address, integer or time */
		}

		if (vlen > RADIUS_ATTR_VLEN-8) {
			syslog(LOG_INFO,"truncating RADIUS attribute type %d vendor(%d,%d) from %d to %d bytes [%s]", 
					type, vendor_id, vendor_type, vlen, RADIUS_ATTR_VLEN-8, data);	//test
			vlen = RADIUS_ATTR_VLEN-8;
		}

		if ((length+vlen+2) > RADIUS_PACKSIZE) { 
			syslog(LOG_INFO,"radius packet size %d too large,no more space!", (length+vlen+2));	//test
			return EAG_RETURN_CODE_OUT_OF_MEMORY;
		}

		length += vlen + 8;

		pack->length = htons(length);

		a->t = type;
		a->l = vlen+8;

		a->v.vv.i = htonl(vendor_id);
		a->v.vv.t = vendor_type;
		a->v.vv.l = vlen+2;

		if (data)
			memcpy(((void*) a)+8, data, dlen);
		else if (dlen)
			memset(((void*) a)+8, 0, dlen); 
		else
			a->v.vv.v.i = htonl(value);
	}

	return EAG_RETURN_CODE_OK;
}

