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
* Alg_Comm.c
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

/* Standard C library includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "include/openssl/rand.h"
#include "include/openssl/hmac.h"
#include "include/openssl/sha.h"

#include "include/alg_comm.h"
#include "include/debug.h"
#define SHA256_DIGEST_SIZE 32
#define SHA256_DATA_SIZE 64
#define _SHA256_DIGEST_LENGTH 8
#include "syslog.h"
#include "time.h"//qiuchen
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>
#define CKR_DEVICE_ERROR                      0x00000030
#define CKR_OK                                0x00000000
#define	RNG_GenerateGlobalRandomBytes(p,l) \
	(pkcs11_get_nzero_urandom((p), (l)) < 0 ? CKR_DEVICE_ERROR : CKR_OK)
#define	URANDOM_DEVICE		"/dev/urandom"	/* urandom device name */
	static int	urandom_fd = -1;
int overflow(unsigned long *gnonce)
{
	unsigned char flow[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

	if (memcmp(gnonce, flow, 8) == 0)
		return 0;
	else
		return -1;
}

/*大数加*/
void add(unsigned long *a, unsigned long b, unsigned short len)
{
	int i = 0;
	unsigned long carry = 0;
	unsigned long *a1 = NULL;
	unsigned long ca1 = 0;
	unsigned long ca2 = 0;
	unsigned long ca3 = 0;
	unsigned long bb = b;

	for(i=len - 1; i>=0; i--)                         //分段32bits加                      
	{	
		a1=a+i; 
		ca1 = (*a1)&0x80000000;	
		ca2 = bb&0x80000000;
		*a1 += bb + carry;
		bb = 0;
		ca3 = (*a1)&0x80000000;	
		if(ca1==0x80000000 && ca2==0x80000000)  carry=1; 
		else if(ca1!=ca2 && ca3==0)  carry=1; 
        	else carry=0;
		a1++;
	}
	
}

void update_gnonce(unsigned long *gnonce, int type)
{
	add(gnonce, type+1, 4);
}

/*for 3Ci*/
#if 1
void	smash_random(unsigned char *buffer, int len )
{
	unsigned char smash_key[32] = {0};
	int i = 0,j = 0;
	srand((int)(time(NULL)*16));
	for(i=0;i<32;i++){
		j=(int)(256.0*rand()/(RAND_MAX+1.0));
		smash_key[i]=j;
	}
	KD_hmac_sha256(buffer, len, smash_key, 32, buffer, len);
}
#else
void	smash_random(unsigned char *buffer, int len )
{
	 unsigned char smash_key[32] = {  0x09, 0x1A, 0x09, 0x1A, 0xFF, 0x90, 0x67, 0x90,
									0x7F, 0x48, 0x1B, 0xAF, 0x89, 0x72, 0x52, 0x8B,
									0x35, 0x43, 0x10, 0x13, 0x75, 0x67, 0x95, 0x4E,
									0x77, 0x40, 0xC5, 0x28, 0x63, 0x62, 0x8F, 0x75};
	KD_hmac_sha256(buffer, len, smash_key, 32, buffer, len);
}
#endif
/*取随机数*/
int
open_nointr(const char *path, int oflag, ...)
{
	int	fd;
	mode_t	pmode;
	va_list	alist;

	va_start(alist, oflag);
	pmode = va_arg(alist, mode_t);
	va_end(alist);

	do {
		if ((fd = open(path, oflag, pmode)) >= 0) {
			(void) fcntl(fd, F_SETFD, FD_CLOEXEC);
			break;
		}
		/* errno definitely set by failed open() */
	} while (errno == EINTR);
	return (fd);
}
static int
pkcs11_open_common(int *fd, const char *dev, int oflag)
{
	if (*fd < 0) {
		if (*fd < 0)
			*fd = open_nointr(dev, oflag);
	}
	return (*fd);
}
static int
pkcs11_open_urandom(void)
{
	return (pkcs11_open_common(&urandom_fd,   URANDOM_DEVICE, O_RDONLY));
}
ssize_t
readn_nointr(int fd, void *dbuf, size_t dlen)
{
	char	*marker = dbuf;
	size_t	left = dlen;
	ssize_t	nread = 0, err;

	for (err = 0; left > 0 && nread != -1; marker += nread, left -= nread) {
		if ((nread = read(fd, marker, left)) < 0) {
			if (errno == EINTR) {	/* keep trying */
				nread = 0;
				continue;
			}
			err = nread;		/* hard error */
			break;
		} else if (nread == 0) {
			break;
		}
	}
	return (err != 0 ? err : dlen - left);
}
int
pkcs11_get_urandom(void *dbuf, size_t dlen)
{
	if (dbuf == NULL || dlen == 0)
		return (0);

	/* Read random data directly from /dev/urandom */
	if (pkcs11_open_urandom() < 0)
		return (-1);

	if (readn_nointr(urandom_fd, dbuf, dlen) == dlen)
		return (0);
	return (-1);
}
int
pkcs11_get_nzero_urandom(void *dbuf, size_t dlen)
{
	char	extrarand[32];
	size_t	bytesleft = 0;
	size_t	i = 0;

	/* Start with some random data */
	if (pkcs11_get_urandom(dbuf, dlen) < 0)
		return (-1);

	/* Walk through data replacing any 0 bytes with more random data */
	while (i < dlen) {
		if (((char *)dbuf)[i] != 0) {
			i++;
			continue;
		}

		if (bytesleft == 0) {
			bytesleft = sizeof (extrarand);
			if (pkcs11_get_urandom(extrarand, bytesleft) < 0)
				return (-1);
		}
		bytesleft--;

		((char *)dbuf)[i] = extrarand[bytesleft];
	}
	return (0);
}
int  get_randam_nss(unsigned char *buffer, int len)
{
	return RNG_GenerateGlobalRandomBytes(buffer ,len);
}
int  get_RAND_bytes(unsigned char  *buffer, int len)
{
	return get_randam_nss(buffer, len);
}
void get_random(unsigned char *buffer, int len)
{
	get_RAND_bytes(buffer, len);
	smash_random(buffer, len);
}


/*标准SHA256 hash算法*/
int mhash_sha256(unsigned char *data, unsigned length, unsigned char *digest)
{

	SHA256((const unsigned char *)data, length, digest);
        return 0;
}

/*HMAC-SHA256算法*/

int hmac_sha256(unsigned char *text, int text_len, 
	unsigned char *key, unsigned key_len, unsigned char *digest,
	unsigned digest_length)
{
	unsigned char out[SHA256_DIGEST_SIZE] = {0,};
	HMAC(EVP_ecdsasha256(),
			key,
			key_len,
			text,
			text_len,
			out,
			NULL);
	memcpy(digest, out, digest_length);
	return 0;
}

/*KD-HMAC-SHA256算法*/
void KD_hmac_sha256(unsigned char *text,unsigned text_len,unsigned char *key,
					unsigned key_len, unsigned char  *output, unsigned length)
{
	unsigned i;
	
	for(i=0;length/SHA256_DIGEST_SIZE;i++,length-=SHA256_DIGEST_SIZE){
		hmac_sha256(
			text,
			text_len,
			key,
			key_len, 
			&output[i*SHA256_DIGEST_SIZE],
			SHA256_DIGEST_SIZE);
		text=&output[i*SHA256_DIGEST_SIZE];
		text_len=SHA256_DIGEST_SIZE;
	}

	if(length>0)
		hmac_sha256(
			text,
			text_len,
			key,
			key_len, 
			&output[i*SHA256_DIGEST_SIZE],
			length);

}

