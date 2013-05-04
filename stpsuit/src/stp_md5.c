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
* stp_md5.c
*
* CREATOR:
*       wangxiangfeng@autelan.com
*
* DESCRIPTION:
*       APIs for MD5 in stp module
*
* DATE:
*       07/03/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using stp_md5_init        **
 **    (2) Call stp_md5_update on mdContext and M                          **
 **    (3) Call stp_md5_final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */
 #include "stp_base.h"

#ifndef SHA_DIGESTSIZE
#define SHA_DIGESTSIZE  20
#endif

#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE   64
#endif

#ifndef SHA_KEYSIZE
#define SHA_KEYSIZE     20
#endif

#ifndef SHA1_DIGEST_LENGTH
#define SHA1_DIGEST_LENGTH 20	 
#endif

#ifndef SHA1_BLOCKSIZE
#define SHA1_BLOCKSIZE   64
#endif

#ifndef SHA1_KEYSIZE
#define SHA1_KEYSIZE     20
#endif

#ifndef SHA_DIGEST_LENGTH
#define SHA_DIGEST_LENGTH	SHA1_DIGEST_LENGTH 
#endif

#ifndef MD5_DIGESTSIZE
#define MD5_DIGESTSIZE  16
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE   64
#endif

#ifndef MD5_KEYSIZE
#define MD5_KEYSIZE    16
#endif

#ifndef KEY_BLOCKSIZE
#define KEY_BLOCKSIZE  64
#endif

#ifndef UINT32
#define UINT32   unsigned int
#endif

#ifndef UINT8
#define UINT8   unsigned char
#endif

typedef struct {
    UINT32 i[2];        /* number of _bits_ handled mod 2^64  */
    UINT32 buf[4];      /* scratch buffer                     */
    UINT8 in[64];       /* input buffer                       */
    UINT8 digest[16];   /* actual digest after stp_md5_final call  */
} MD5_CTX;
/* forward declaration */
static void stp_md5_transform (UINT32 *buf, UINT32 *in);

static UINT8 PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static  UINT8  MD5_PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

#define UL(x)   x##U

/* The routine stp_md5_init initializes the message-digest context
   mdContext. All fields are set to zero.
 */
void stp_md5_init (mdContext)
MD5_CTX *mdContext;
{
  mdContext->i[0] = mdContext->i[1] = (UINT32)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT32)0x67452301L;
  mdContext->buf[1] = (UINT32)0xefcdab89L;
  mdContext->buf[2] = (UINT32)0x98badcfeL;
  mdContext->buf[3] = (UINT32)0x10325476L;
}

/* The routine stp_md5_update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
void stp_md5_update (mdContext, inBuf, inLen)
MD5_CTX *mdContext;
UINT8 *inBuf;
UINT32 inLen;
{
  UINT32 in[16];
  unsigned int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (unsigned int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT32)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT32)inLen << 3);
  mdContext->i[1] += ((UINT32)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT32)mdContext->in[ii+3]) << 24) |
                (((UINT32)mdContext->in[ii+2]) << 16) |
                (((UINT32)mdContext->in[ii+1]) << 8) |
                ((UINT32)mdContext->in[ii]);
      stp_md5_transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

/* The routine stp_md5_final terminates the message-digest computation and
   ends with the desired message digest in mdContext->digest[0...15].
 */
void stp_md5_final (mdContext)
MD5_CTX *mdContext;
{
  UINT32 in[16];
  unsigned int mdi;
  unsigned int i, ii;
  UINT32 padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (unsigned int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  stp_md5_update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT32)mdContext->in[ii+3]) << 24) |
            (((UINT32)mdContext->in[ii+2]) << 16) |
            (((UINT32)mdContext->in[ii+1]) << 8) |
            ((UINT32)mdContext->in[ii]);
  stp_md5_transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (UINT8)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (UINT8)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (UINT8)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (UINT8)((mdContext->buf[i] >> 24) & 0xFF);
  }
}

/* Basic MD5 step. Transforms buf based on in.
 */
static void stp_md5_transform (UINT32 *buf, UINT32 *in)
{
  UINT32 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, UL(3614090360L)); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, UL(3905402710L)); /* 2 */
  FF ( c, d, a, b, in[ 2], S13, UL( 606105819L)); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, UL(3250441966L)); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, UL(4118548399L)); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, UL(1200080426L)); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, UL(2821735955L)); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, UL(4249261313L)); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, UL(1770035416L)); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, UL(2336552879L)); /* 10 */
  FF ( c, d, a, b, in[10], S13, UL(4294925233L)); /* 11 */
  FF ( b, c, d, a, in[11], S14, UL(2304563134L)); /* 12 */
  FF ( a, b, c, d, in[12], S11, UL(1804603682L)); /* 13 */
  FF ( d, a, b, c, in[13], S12, UL(4254626195L)); /* 14 */
  FF ( c, d, a, b, in[14], S13, UL(2792965006L)); /* 15 */
  FF ( b, c, d, a, in[15], S14, UL(1236535329L)); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, UL(4129170786L)); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, UL(3225465664L)); /* 18 */
  GG ( c, d, a, b, in[11], S23, UL( 643717713L)); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, UL(3921069994L)); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, UL(3593408605L)); /* 21 */
  GG ( d, a, b, c, in[10], S22, UL(  38016083L)); /* 22 */
  GG ( c, d, a, b, in[15], S23, UL(3634488961L)); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, UL(3889429448L)); /* 24 */
  GG ( a, b, c, d, in[ 9], S21, UL( 568446438L)); /* 25 */
  GG ( d, a, b, c, in[14], S22, UL(3275163606L)); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, UL(4107603335L)); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, UL(1163531501L)); /* 28 */
  GG ( a, b, c, d, in[13], S21, UL(2850285829L)); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, UL(4243563512L)); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, UL(1735328473L)); /* 31 */
  GG ( b, c, d, a, in[12], S24, UL(2368359562L)); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, UL(4294588738L)); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, UL(2272392833L)); /* 34 */
  HH ( c, d, a, b, in[11], S33, UL(1839030562L)); /* 35 */
  HH ( b, c, d, a, in[14], S34, UL(4259657740L)); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, UL(2763975236L)); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, UL(1272893353L)); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, UL(4139469664L)); /* 39 */
  HH ( b, c, d, a, in[10], S34, UL(3200236656L)); /* 40 */
  HH ( a, b, c, d, in[13], S31, UL( 681279174L)); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, UL(3936430074L)); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, UL(3572445317L)); /* 43 */
  HH ( b, c, d, a, in[ 6], S34, UL(  76029189L)); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, UL(3654602809L)); /* 45 */
  HH ( d, a, b, c, in[12], S32, UL(3873151461L)); /* 46 */
  HH ( c, d, a, b, in[15], S33, UL( 530742520L)); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, UL(3299628645L)); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, UL(4096336452L)); /* 49 */
  II ( d, a, b, c, in[ 7], S42, UL(1126891415L)); /* 50 */
  II ( c, d, a, b, in[14], S43, UL(2878612391L)); /* 51 */
  II ( b, c, d, a, in[ 5], S44, UL(4237533241L)); /* 52 */
  II ( a, b, c, d, in[12], S41, UL(1700485571L)); /* 53 */
  II ( d, a, b, c, in[ 3], S42, UL(2399980690L)); /* 54 */
  II ( c, d, a, b, in[10], S43, UL(4293915773L)); /* 55 */
  II ( b, c, d, a, in[ 1], S44, UL(2240044497L)); /* 56 */
  II ( a, b, c, d, in[ 8], S41, UL(1873313359L)); /* 57 */
  II ( d, a, b, c, in[15], S42, UL(4264355552L)); /* 58 */
  II ( c, d, a, b, in[ 6], S43, UL(2734768916L)); /* 59 */
  II ( b, c, d, a, in[13], S44, UL(1309151649L)); /* 60 */
  II ( a, b, c, d, in[ 4], S41, UL(4149444226L)); /* 61 */
  II ( d, a, b, c, in[11], S42, UL(3174756917L)); /* 62 */
  II ( c, d, a, b, in[ 2], S43, UL( 718787259L)); /* 63 */
  II ( b, c, d, a, in[ 9], S44, UL(3951481745L)); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

/*
 ***********************************************************************
 ** End of md5.c                                                      **
 ******************************** (cut) ********************************
 */
static void stp_md5_encode (output, input, len)
UINT8 *output;
UINT32 *input;
UINT32 len;
{
  UINT32 i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (UINT8)(input[i] & 0xff);
    output[j+1] = (UINT8)((input[i] >> 8) & 0xff);
    output[j+2] = (UINT8)((input[i] >> 16) & 0xff);
    output[j+3] = (UINT8)((input[i] >> 24) & 0xff);
  }
}

void stp_md5_final_h(digest, context)
UINT8 digest[16];                         /* message digest */
MD5_CTX *context;                                       /* context */
{
  UINT8 bits[8];
  UINT32 index, padLen;

  /* Save number of bits */
  stp_md5_encode (bits, context->i, 8);

  /* Pad out to 56 mod 64.
   */
  index = (UINT32)((context->i[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  stp_md5_update (context, MD5_PADDING, padLen);
  
  /* Append length (before padding) */
  stp_md5_update (context, bits, 8);

  /* Store state in digest */
  stp_md5_encode (digest, context->buf, 16);
  
  /* Zeroize sensitive information.
   */
  memset((UINT8*)context, 0, sizeof (*context));
}


/*************************************************************************
	函数组：	bits8_t  *MD5(text,text_len,digest)
	功能：		对输入的数据进行MD5算法的消息散列计算
	包含函数：	stp_md5_init/stp_md5_update/stp_md5_final
	输入参数：	text:	数据输入
				text_len: 输入数据的长度
	返回结果：	digest:	结果文摘的输出
	作者/日期：	qin.hao / 2003-12-5
	修改记录：		
*************************************************************************/
char  *MD5(text,text_len,digest)
char  *text;		/* input text data */   
int  text_len;	/* the length of input text data */   
char  *digest;		/* out of the digest */   
{	
	MD5_CTX context;            
	char *r_digest,*da;
	int steps,i;

	if (digest==NULL){
		r_digest=(char *)malloc(MD5_DIGESTSIZE);
	}else
		r_digest=digest;

	 da=text;			   
	 steps=text_len/MD5_BLOCKSIZE;
	 stp_md5_init (&context);
	 for (i=0;i<steps;i++)
	 {
		 stp_md5_update (&context, da, MD5_BLOCKSIZE);
		 da = (char *)da+MD5_BLOCKSIZE;
	 }
	 stp_md5_update (&context, da, text_len%MD5_BLOCKSIZE);			 
	 stp_md5_final (&context);
	 memcpy(r_digest,context.digest,MD5_DIGESTSIZE);
#if __DEBUG__
	 printf(" \n ");
	 for (i=0;i<MD5_DIGESTSIZE;i++)
		 printf(" %02x ",r_digest[i]);
#endif         
	 
	 return(r_digest);
}




void stp_md5_get_digest(unsigned char*text,int text_len, unsigned char*key, int key_len, unsigned char* digest)
{
        MD5_CTX context;
        unsigned char k_ipad[MD5_BLOCKSIZE+1];    /* inner padding -
                                      * key XORd with ipad
                                      */
        unsigned char k_opad[MD5_BLOCKSIZE+1];    /* outer padding -
                                      * key XORd with opad
                                      */
        unsigned char tk[MD5_KEYSIZE];
        int i;
        /* if key is longer than 64 bytes reset it to key=MD5(key) */
        if (key_len > KEY_BLOCKSIZE) {

                MD5_CTX      tctx;

                stp_md5_init(&tctx);
                stp_md5_update(&tctx, key, key_len);
                stp_md5_final_h(tk, &tctx);

                key = tk;
                key_len = SHA_KEYSIZE;
        }

        /*
         * the HMAC_MD5 transform looks like:
         *
         * MD5(K XOR opad, MD5(K XOR ipad, text))
         *
         * where K is an n byte key
         * ipad is the byte 0x36 repeated 64 times
         * opad is the byte 0x5c repeated 64 times
         * and text is the data being protected
         */

        /* start out by storing key in pads */
	 bzero(k_ipad,sizeof(k_ipad));
        bzero(k_opad,sizeof(k_opad));
        bcopy(key,k_ipad,key_len);
        bcopy(key,k_opad,key_len);


        /* XOR key with ipad and opad values */
        for (i=0; i<MD5_BLOCKSIZE; i++) {
                k_ipad[i] ^= 0x36;
                k_opad[i] ^= 0x5c;
        }
        /*
         * perform inner MD5
         */
        stp_md5_init(&context);                   /* init context for 1st
                                              * pass */
        stp_md5_update(&context, k_ipad, MD5_BLOCKSIZE);      /* start with inner pad */
        stp_md5_update(&context, text, text_len); /* then text of datagram */
        stp_md5_final_h(digest, &context);          /* finish up 1st pass */
        /*
         * perform outer MD5
         */
        stp_md5_init(&context);                   /* init context for 2nd pass */
        stp_md5_update(&context, k_opad, MD5_BLOCKSIZE);     /* start with outer pad */
        stp_md5_update(&context, digest, MD5_KEYSIZE);     /* then results of 1st  hash */
        stp_md5_final_h(digest, &context);          /* finish up 2nd pass */
}
#ifdef __cplusplus
}
#endif

