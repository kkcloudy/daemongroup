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
* AsdWAPINoneCert.c
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
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include "common.h"
#include "include/auth.h"

#include "include/structure.h"
#include "include/config.h"

static void*  none_new(void);
static void  none_free(void **cert_st);
static int  none_decode(void **cert_st, unsigned char *in, int in_len);
static int none_encode(void *cert_st, unsigned char **out, int out_len);
static int none_cmp(void *cert_bin_a, void *cert_bin_b);
static void* none_get_public_key(void *cert_st);
static void* none_get_subject_name(void *cert_st);
static void* none_get_issure_name(void *cert_st);
static void* none_get_serial_number(void *cert_st);
static int none_sign(const unsigned char *priv_s, int priv_sl,	const unsigned char * in, int in_len, unsigned char *out);
static int none_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sig,int sign_len);


static struct cert_obj_st_t  cert_obj_none = {
	.cert_name = "NONE",
	.cert_type	  = CERT_OBJ_NONE,
	.asu_cert_st	= NULL,
	.asu_pubkey = NULL,
	.ca_cert_st	= NULL,                      //三证书
	.ca_pubkey = NULL,                          //三证书
	.user_cert_st = NULL,
	.private_key	= NULL,
	.cert_bin	= NULL,
	.obj_new	= none_new,
	.obj_free = none_free,
	.decode	= none_decode,
	.encode	= none_encode,
	.cmp  = none_cmp,
	.get_public_key	= none_get_public_key,
	.get_subject_name	= none_get_subject_name,
	.get_issuer_name	= none_get_issure_name,
	.get_serial_number	= none_get_serial_number,
	.sign	= none_sign,
	.verify = none_verify,
};

static void*  none_new(void)
{
	return NULL;
}
static void  none_free(void **cert_st)
{
	cert_st = cert_st;
	return ;
}
static int  none_decode(void **cert_st, unsigned char *in, int in_len)
{
	cert_st = cert_st;
	in = in;
	in_len = in_len;
	return 0;
}
static int none_encode(void *cert_st, unsigned char **out, int out_len)
{
	cert_st = cert_st;
	out = out;
	out_len = out_len;

	return 0;
}
static int none_cmp(void *cert_bin_a, void *cert_bin_b)
{
	cert_bin_a = cert_bin_a;
	cert_bin_b = cert_bin_b;
	return 0;
}
static void* none_get_public_key(void *cert_st)
{
	cert_st = cert_st;
	return NULL;
}
static void* none_get_subject_name(void *cert_st)
{
	cert_st = cert_st;
	return NULL;
}
static void* none_get_issure_name(void *cert_st)
{
	cert_st = cert_st;
	return NULL;
}
static void* none_get_serial_number(void *cert_st)
{
	cert_st = cert_st;
	return NULL;
}
	
static int none_sign(const unsigned char *priv_s, int priv_sl,	const unsigned char * in, int in_len, unsigned char *out)
{
	priv_s = priv_s;
	priv_sl = priv_sl;
	in = in;
	in_len = in_len;
	out = out;
	return 0;
}

static int none_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sig,int sign_len)
{
	pub_s = pub_s;
	pub_sl = pub_sl;
	in = in;
	in_len = in_len;
	sig = sig;
	sign_len = sign_len;	
	return 1;
}

int none_cert_init(struct asd_wapi *wapi){
	memcpy(wapi->cert_info.ap_cert_obj,&cert_obj_none, sizeof(struct cert_obj_st_t));
	cert_obj_register(wapi->cert_info.ap_cert_obj);
	return 0;
}


int none_init()
{
	cert_obj_register(&cert_obj_none);
	return 0;
}
void none_exit()
{
	cert_obj_unregister(&cert_obj_none);
}

