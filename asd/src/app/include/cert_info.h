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
* cert_info.h
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

#ifndef __CERT_INFO__H__
#define __CERT_INFO__H__
#include <openssl/bio.h>
#include "structure.h"
#define CERT_OBJ_NONE 0
#define  CERT_OBJ_X509 1
#define  CERT_OBJ_GBW  2
//#define	GETSHORT(frm, v) do { (v) = (((frm[0]) <<8) | (frm[1]))& 0xffff;} while (0)

#include "auth.h"
typedef certificate gwb_cert;

struct cert_obj_st_t{
	int cert_type;
	char *cert_name;
	void *asu_cert_st;
	tkey *asu_pubkey;
	void *ca_cert_st;     //ÈýÖ¤Êé
	tkey *ca_pubkey;	
	void *user_cert_st;
	tkey *private_key;
	struct cert_bin_t  *cert_bin;
	
	/*(void*)  (*obj_new)(void);
	int  (*obj_free)(void **cert_st);*/
	void*  (*obj_new)(void);
	void  (*obj_free)(void **cert_st);
	int  (*decode)(void **cert_st, unsigned char *in, int in_len);
	int (*encode)(void *cert_st, unsigned char **out, int out_len);
	int (*cmp)(void *cert_bin_a, void *cert_bin_b);
	void* (*get_public_key)(void *cert_st);
	void* (*get_subject_name)(void *cert_st);
	void* (*get_issuer_name)(void *cert_st);
	void* (*get_serial_number)(void *cert_st);
	int (*verify_key)(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl);
	int (*sign)(const unsigned char *priv_s, int priv_sl,
			//const unsigned char *pub_s, int pub_sl,
			const unsigned char * in, int in_len, 
			unsigned char *out);
	int (*verify)(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sig,int sign_len);
};

void cert_obj_register(struct cert_obj_st_t *cert_obj);
void cert_obj_unregister(const struct cert_obj_st_t *cert_obj);
int cmp_oid(u8 *in , int len);

int get_x509_cert(BIO *bp, struct cert_obj_st_t *cert_obj);
void x509_free_obj_data(struct cert_obj_st_t *cert_obj);
void bind_certobj_function(struct cert_obj_st_t *dstobj, int  index);
const struct cert_obj_st_t *get_cert_obj(unsigned short index);
int register_certificate(void *user_data);

#endif
