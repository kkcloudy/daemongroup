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
* AsdX509Cert.c
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

#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include "common.h"
#include "include/auth.h"

#include "include/structure.h"
#include "include/cert_info.h"
#include "include/ecc_crypt.h"
#include "include/proc.h"
#include "include/debug.h"
#include "wcpss/asd/asd.h"

static void * X509_wapi_new(void)
{
	return NULL;
}
static void X509_obj_free(void **cert_st)
{
	X509 *a = (X509 *)(*cert_st);
	X509_free(a);
	*cert_st = NULL;
	return ;
}

/*
	将der编码二进制数据转换成X509结构
	参数:
	             void **cert_st: 输出的结构,如果(cert_st != NULL)&&(*cert_st == NULL)
	                                     则程序自动分配空间,释放空间使用X509_free
	                                     函数;
	             unsigned char in: DER编码的二进制数据的地址
	             in_len: DER编码的二进制数据的长度
	 返回值:0表示解码成功;其他表示失败            
*/
static  int X509_decode(void **cert_st, unsigned char *in, int in_len)
{

        const unsigned char  *p = NULL;
	X509 **x = (X509 **)cert_st;
        /* Something to setup buf and len */
        p = in;
        if(!d2i_X509(x, &p, in_len))			
        {
        	return -1;
        	/* Some error */
	}
	return 0;	
}

/*
	将X509结构编码成DER编码的二进制数据
	参数: void *vert_st : 输入的X509结构
		      unsigned char **out:输出的DER编码二进制数据.
		                              如果(out!=NULL)&&(*out==NULL),则函数
		                              自动分配空间,长度为函数返回值
	返回值:<0表示失败;>=0表示成功
*/
static int X509_encode(void *cert_st, unsigned char **out, int outlen)
{
	int len = 0;

	if((out != NULL) && (*out == NULL)) 
	{
		len = i2d_X509((X509 *)cert_st, out);
		DPrintf("%s: %d lines :out at %p\n", __func__, __LINE__, out);
	}
	else
	{
	
		
		unsigned char *p = *out;
		unsigned char len1 = 0;
		
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s: %d lines :len =%d\n", __func__, __LINE__, len);
		len1 = i2d_X509((X509 *)cert_st, NULL);

		if(len1>outlen)
		{
			return -1;
		}
		len = i2d_X509((X509 *)cert_st, &p);
	}
	DPrintf("%s: %d lines :len =%d\n", __func__, __LINE__, len);
	return len;
}
static int X509_bincmp(void *a, void *b)
{
	struct cert_bin_t *cert_bin_a = a;
	struct cert_bin_t *cert_bin_b = b;
#if 0
	assert(cert_bin_a);
	assert(cert_bin_b);
#endif 
	if(cert_bin_a == NULL ||cert_bin_b == NULL) return -3;
	if(cert_bin_a->length != cert_bin_b->length)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"length of certificates is  differently\n");
		return -1;
	}
	if(memcmp(cert_bin_a->data, cert_bin_b->data, cert_bin_a->length) != 0)
	{
		return -2;
	}

	return 0;
}

/*证书颁发者或证书持有者名称编码*/
static void * X509_get_name_value(BUF_MEM *bytes)
{
	byte_data  *out;

	out = (byte_data *) get_buffer(sizeof(byte_data));
	if(out == NULL)
		return NULL;

	if(bytes->length>MAX_BYTE_DATA_LEN)
	{
		os_free(out);//qiuchen
		return NULL;
	}

	memcpy(out->data, bytes->data, bytes->length);
	out->length = bytes->length;
	return out;
}
/*取颁发者名称*/
static void *X509_wapi_get_subject_name(void *cert_st)
{
	X509 *x = (X509 *)cert_st;
	X509_NAME  *subject_name = NULL;
	BUF_MEM *bytes = NULL;
	
	subject_name=X509_get_subject_name(x);

	bytes = subject_name->bytes;
	
	return X509_get_name_value(bytes);
}
/*证书颁发者或证书持有者名称编码*/
static void * X509_name_encode(X509_NAME *name)
{
	byte_data  *out;

	unsigned char *p = NULL;
	unsigned int len1 = 0;
	out = (byte_data *) get_buffer(sizeof(byte_data));
	if(out == NULL)
		return NULL;

	len1 = i2d_X509_NAME((X509_NAME *)name, NULL);

	if(len1>64)
	{
		os_free(out);//qiuchen
		return NULL;
	}
	p = (unsigned char *)out->data;
	out->length = i2d_X509_NAME((X509_NAME *)name, &p);
	return out;
}
/*取颁发者名称*/
void *X509_wapi_get_subject_name_vip(void *cert_st)
{
	X509 *x = (X509 *)cert_st;
	X509_NAME  *subject_name = NULL;
	
	subject_name=X509_get_subject_name(x);
	return X509_name_encode(subject_name);
}
/*证书颁发者或证书持有者名称编码*/
int X509_name_get_cn(byte_data  *in, char *cn)
{
        const unsigned char *p = NULL;
        X509_NAME *name = NULL;
        char *name_str = NULL;
        char *cn_b = NULL;
        char *cn_t = NULL;
        char *cn_e = NULL;
 
        if ((in == NULL) || (in->length == 0) || (cn == NULL))
        {
                asd_printf(ASD_WAPI,MSG_INFO,"Bad param , is NULL.\n");
                return -1;
        }
 
        p = (unsigned char *)in->data;
        d2i_X509_NAME(&name, (const unsigned char **)&p, in->length);
        if (name == NULL)
                return -1;
 
        name_str = X509_NAME_oneline(name, NULL, 0);
        X509_NAME_free(name);
        if ((name_str == NULL) || (name_str[0] == '\0')
                || ((cn_b = strstr(name_str, "CN=")) == NULL))
        {
                if (name_str != NULL) 
                        OPENSSL_free(name_str);
                return -1;
        }
 
        cn_b = cn_b+3;
        cn_t = cn_b + strlen(cn_b);
        cn_e = cn_b;
        while ((cn_e < cn_t) && (*cn_e != '/'))
                cn_e++;
        if (cn_e != cn_b)
                strncpy(cn, cn_b, cn_e-cn_b);
 
        OPENSSL_free(name_str);
        return 0;
}

/*取持有者名称*/
static void *X509_wapi_get_issuer_name(void *cert_st)
{
	X509 *x = (X509 *)cert_st;
	X509_NAME  *issure_name = NULL;
	BUF_MEM *bytes = NULL;
	
	issure_name=X509_get_issuer_name(x);
	bytes = issure_name->bytes;
	return X509_get_name_value(bytes);
}

/*取序列号*/
static void *X509_wapi_get_serial_number(void *cert_st)
{
	X509 *x = (X509 *)cert_st;
	ASN1_INTEGER *serial = NULL;

	byte_data *out = NULL;
	unsigned char *p = NULL;

	serial = X509_get_serialNumber(x);
	if(serial == NULL)
		return NULL;
	out = (byte_data *)get_buffer(sizeof(byte_data));
	if(out == NULL)
		return NULL;
	p = (unsigned char *)out->data;

	out->length = i2d_ASN1_INTEGER(serial, &p);
	return out;
}

/*从x509证书中取出公钥,并转换成tkey结构*/
tkey * x509_get_pubkey(X509 *x)
{
	EC_KEY *eckey = NULL;
	const EC_GROUP *group1;
	const EC_POINT *point;
	BIGNUM  *pubm=NULL;
	BN_CTX  *ctx=NULL;
	tkey *tpubkey = NULL;
	int tpubkeyl = sizeof(tkey);
	
	EVP_PKEY *pkey=NULL;
	
	if(x == NULL)
	{
		asd_printf(ASD_WAPI,MSG_INFO,"in %s : %d x is NULL \n", __func__, __LINE__);
		return NULL;
	}

	pkey=X509_get_pubkey(x);

	if (pkey == NULL)
	{
		DPrintf("in %12s Unable to load Public Key\n",__func__);
		return NULL;
	}

	if (pkey->type == EVP_PKEY_EC)
	{
		DPrintf("%12s EC Public Key:\n", __func__);
		eckey = pkey->pkey.ec;
		//EC_KEY_print(bp, pkey->pkey.ec, 16);
	}
	else
	{
		DPrintf("in %12s Unknown Public Key:\n",__func__);
		goto error;
	}
	
	if ((group1 = EC_KEY_get0_group(eckey)) == NULL)
	{
		asd_printf(ASD_WAPI,MSG_ERROR,"in %s EC_KEY_get0_group failure\n", __func__);
		goto error;
	}

	point = EC_KEY_get0_public_key(eckey);
	if ((pubm = EC_POINT_point2bn(group1, point,
		EC_KEY_get_conv_form(eckey), NULL, ctx)) == NULL)
	{
		asd_printf(ASD_WAPI,MSG_ERROR,"in %s EC_POINT_point2bn failure\n", __func__);
		goto error;
	}
	tpubkey = (tkey *)get_buffer(tpubkeyl);
	
	if(tpubkey == NULL)
	{
		asd_printf(ASD_WAPI,MSG_ERROR,"in %s:%d get_buffer failure\n", __func__, __LINE__);
		goto error;
	}
	
	tpubkey->length = BN_bn2bin(pubm, tpubkey->data);
exit:	
	if (pubm) 
		BN_free(pubm);
	if(pkey)
		EVP_PKEY_free(pkey);
	if (ctx)
		BN_CTX_free(ctx);
	return tpubkey;
error:
	if(tpubkey)
		tpubkey = free_buffer(tpubkey, tpubkeyl);
	goto exit;
}	
/*取公钥*/
static void *X509_wapi_get_pubkey(void *cert_st)
{
	X509 *x = (X509 *)cert_st;

	return x509_get_pubkey(x);
}
/*x509证书对象描述*/
static struct cert_obj_st_t  cert_obj_x509 = {
	.cert_type	  = CERT_OBJ_X509,
	.cert_name = "x509v3",
	.asu_cert_st	= NULL,
	.asu_pubkey = NULL,
	.ca_cert_st	= NULL,                      //三证书
	.ca_pubkey = NULL,                          //三证书
	.user_cert_st = NULL,
	.private_key	= NULL,
	.cert_bin	= NULL,
	
	.obj_new	= X509_wapi_new,
	.obj_free  = X509_obj_free,
	.decode	= X509_decode,
	.encode	= X509_encode,
	.cmp		= X509_bincmp,
	.get_public_key	= X509_wapi_get_pubkey,
	.get_subject_name	= X509_wapi_get_subject_name,
	.get_issuer_name	= X509_wapi_get_issuer_name,
	.get_serial_number	= X509_wapi_get_serial_number,
	.verify_key = ecc192_verify_key,
	.sign	= ecc192_sign,
	.verify = ecc192_verify,
};
/*注册x509证书对象*/
int X509_cert_init(struct asd_wapi *wapi){
	memcpy(wapi->cert_info.ap_cert_obj,&cert_obj_x509, sizeof(struct cert_obj_st_t));
	cert_obj_register(wapi->cert_info.ap_cert_obj);
	return 0;
}

int X509_init()
{
	cert_obj_register(&cert_obj_x509);
	return 0;
}
/*注销x509证书对象*/
void X509_exit()
{
	cert_obj_unregister(&cert_obj_x509);
}

