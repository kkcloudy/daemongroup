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
* AsdCertInfo.c
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

#include <stdio.h>
#include <stdlib.h>
 
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
#include "include/proc.h"
#include "include/cert_info.h"
#include "include/x509_cert.h"
//#include "include/typedef.h"
#include "include/debug.h" 
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"

/*证书对象指针数组*/
const struct cert_obj_st_t  *cert_objs[3]; 
extern struct asd_wapi circle;

/*证书对象注册*/
void cert_obj_register(struct cert_obj_st_t *cert_obj)
{
	asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj_register:cert_obj->cert_type=%d,cert_obj->cert_name:%s\n",cert_obj->cert_type,cert_obj->cert_name);
	/*给指向证书编码数据的指针分配空间*/
	cert_obj->cert_bin= (struct cert_bin_t  *)get_buffer(sizeof(struct cert_bin_t));
	
	if(cert_obj->cert_bin == NULL)
	{
		return ;
	}
		
	asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj address 	%p\n", cert_obj);
	asd_printf(ASD_WAPI,MSG_DEBUG,"obj[%d] address 		%p\n", cert_obj->cert_type,  cert_objs[cert_obj->cert_type]);
	asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj_register\n");
	return;
	
}

/*证书对象注消*/
void cert_obj_unregister(const struct cert_obj_st_t *cert_obj)
{
	int index = cert_obj->cert_type;
	
	//struct cert_obj_st_t *obj = NULL;
	if(cert_obj->cert_type > 3)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s: certificate %s has an invalid cert_index %u\n",
			__func__, cert_obj->cert_name,  cert_obj->cert_type);
		return;
	}

	if ((cert_objs[cert_obj->cert_type] != NULL) && (cert_objs[cert_obj->cert_type] != cert_obj))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj address 	%p\n", cert_obj);
		asd_printf(ASD_WAPI,MSG_DEBUG,"obj address 		%p\n", cert_objs[cert_obj->cert_type]);
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s: certificate object %s registered with a different template\n",
			__func__, cert_obj->cert_name);
		return;
	}
	free_buffer((cert_objs[cert_obj->cert_type]->cert_bin), sizeof(struct cert_bin_t));
	cert_objs[index] = NULL;
}

/*给证书对象绑定操作函数*/
void bind_certobj_function(struct cert_obj_st_t *dstobj, int  index)
{
	const struct cert_obj_st_t *srcobj = cert_objs[index];
	if((srcobj == NULL))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"Not found cert_obj\n");
		return;
	}
	else if((srcobj->obj_new == NULL)
		||(srcobj->obj_free == NULL)
		||(srcobj->decode == NULL)
		||(srcobj->encode == NULL)
		||(srcobj->cmp == NULL)
		||(srcobj->get_public_key == NULL)
		||(srcobj->get_subject_name == NULL)
		||(srcobj->get_issuer_name == NULL)
		||(srcobj->get_serial_number == NULL)
		||(srcobj->verify_key == NULL)
		||(srcobj->sign == NULL)
		||(srcobj->verify == NULL))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"Not found cert_obj function\n");
		return;
	}
		
	dstobj->obj_new = srcobj->obj_new;
	dstobj->obj_free = srcobj->obj_free;
	dstobj->encode = srcobj->encode;
	dstobj->decode = srcobj->decode;
	dstobj->get_public_key = srcobj->get_public_key;
	dstobj->get_issuer_name= srcobj->get_issuer_name;
	dstobj->get_subject_name= srcobj->get_subject_name;
	dstobj->get_serial_number = srcobj->get_serial_number;
	dstobj->verify_key = srcobj->verify_key;
	dstobj->sign = srcobj->sign;
	dstobj->verify = srcobj->verify;
}

/*从证书对象指针数组中取证书对象的指针*/
const struct cert_obj_st_t *get_cert_obj(unsigned short index)
{
	//if(index < 3)
	if(index == 1)
	{
		return cert_objs[index];
	}
	else
		return NULL;
}
static int load_none(char *cert_file, const struct cert_obj_st_t *cert_obj)
{
	cert_file = cert_file;
	cert_obj = cert_obj;
	return 0;	
}

/*取X509证书中公钥对应的私钥,并转换为tkey结构*/
int x509_get_privkey(EC_KEY *tmp_eckey, tkey **ppkey)
{
	tkey *pkey = *ppkey;
	const BIGNUM *priv_key;
	int tkey_len = sizeof(tkey);	

	if((ppkey!=NULL) && (pkey  == NULL))
	{
		pkey = (tkey *)get_buffer(tkey_len);
		if(pkey == NULL)
			return -2;

		memset(pkey, 0, tkey_len);
		 *ppkey = pkey; 
	}
	/*从公私钥对中取私钥*/
	priv_key = EC_KEY_get0_private_key(tmp_eckey);
	if(priv_key != NULL)
	{
		/*转换私钥并计算私钥长度*/
		pkey->length = BN_bn2bin(priv_key, pkey->data);
	}
	else
	{
		free_buffer(pkey, tkey_len);
		return -3;
	}
	return 0;
}

/*校验509证书对象中的ASU和用户证书*/
int x509_verify_cert(struct cert_obj_st_t *cert_obj)
{
	
	EVP_PKEY *asu_pkey=NULL;
	int ret= -1;


	X509 *asu_cert = (X509 *)(cert_obj->asu_cert_st);
	X509 *user_cert = (X509 *)(cert_obj->user_cert_st);

	/* 取ASU的公钥*/
	 if ((asu_pkey=X509_get_pubkey(asu_cert)) == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s X509_get_pubkey failure\n", __func__);
		goto err;
	}/* 验证ASU的证书*/

	if (X509_verify(asu_cert, asu_pkey) <= 0)
		/* XXX  For the final trusted self-signed cert,
		 * this is a waste of time.  That check should
		 * optional so that e.g. 'openssl x509' can be
		 * used to detect invalid self-signatures, but
		 * we don't verify again and again in SSL
		 * handshakes and the like once the cert has
		 * been declared trusted. */
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s X509_verify(asu_cert) failure\n", __func__);
		goto err;
	}/* 验证USER的证书*/
	else if (X509_verify(user_cert, asu_pkey) <= 0)
		/* XXX  For the final trusted self-signed cert,
		 * this is a waste of time.  That check should
		 * optional so that e.g. 'openssl x509' can be
		 * used to detect invalid self-signatures, but
		 * we don't verify again and again in SSL
		 * handshakes and the like once the cert has
		 * been declared trusted. */
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s X509_verify(user_cert) failure\n", __func__);
		goto err;
	}
	ret = 0;
err:
	if(asu_pkey)		
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free pkey\n", __func__, __LINE__);
		EVP_PKEY_free(asu_pkey);
		asu_pkey=NULL;
	}
	return ret;

}

void print_err()
{
	const char *file_name = "";
	int line = 0;

	ERR_get_error_line(&file_name, &line);
	asd_printf(ASD_WAPI,MSG_DEBUG,"err at %s:%d\n", file_name, line);
}
void x509_free_obj_data(struct cert_obj_st_t *cert_obj)
{	
	if(cert_obj->asu_cert_st != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free asu_cert_st\n", __func__, __LINE__);
		X509_free(cert_obj->asu_cert_st);
		cert_obj->asu_cert_st = NULL;
	}
	
	//三证书	
	if(cert_obj->ca_cert_st != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free ca_cert_st\n", __func__, __LINE__);
		X509_free(cert_obj->ca_cert_st );
		cert_obj->ca_cert_st  = NULL;
	}
	
	if(cert_obj->user_cert_st != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free user_cert_st\n", __func__, __LINE__);
		X509_free(cert_obj->user_cert_st);
		cert_obj->user_cert_st = NULL;
	}

	if(cert_obj->asu_pubkey != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free asu_pubkey\n", __func__, __LINE__);
		cert_obj->asu_pubkey = free_buffer(cert_obj->asu_pubkey, sizeof(tkey));
	}
	//三证书
	if(cert_obj->ca_pubkey != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free ca_pubkey\n", __func__, __LINE__);
		cert_obj->ca_pubkey = free_buffer(cert_obj->ca_pubkey, sizeof(tkey));
	}

	if(cert_obj->private_key != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free private_key\n", __func__, __LINE__);
		asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj->private_key address = %p\n",cert_obj->private_key );
		cert_obj->private_key = free_buffer((void *)(cert_obj->private_key), sizeof(tkey));
	}

	if(cert_obj->cert_bin->data != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free cert_bin->data\n", __func__, __LINE__);
		memset(cert_obj->cert_bin->data, 0,  cert_obj->cert_bin->length);
		OPENSSL_free(cert_obj->cert_bin->data);
		cert_obj->cert_bin->data = NULL;
	}
}

/*从bp 中分解出ASU的证书,用户的证书及用户证书对应的私钥*/
int get_x509_cert(BIO *bp, struct cert_obj_st_t *cert_obj)
{
	EC_KEY *tmp_pkey=NULL;
	int ret =  EXIT_SUCCESS;
	int len = 0;
	int result = 0;	/*add variable for makeing asd*/

	/*分解私钥*/
	tmp_pkey = PEM_ASN1_read_bio((d2i_of_void *)d2i_ECPrivateKey, 
					PEM_STRING_ECA,
					bp, NULL, NULL,  NULL);
	if(tmp_pkey == NULL)
	{
		const char *file_name = "";
		int line = 0;

		ERR_get_error_line(&file_name, &line);
		asd_printf(ASD_WAPI,MSG_DEBUG,"err at %s:%d\n", file_name, line);
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to read USER's privateKey\n");
		ret = EXIT_FAILURE;
		goto error;
	}

	/*bp 复位即将指针直到开始位置 */
	result = BIO_reset(bp);
	/* 分解ASU  证书*/
	cert_obj->asu_cert_st = PEM_ASN1_read_bio((d2i_of_void *)d2i_X509, 
						PEM_STRING_X509_ASU, 
						bp, NULL, NULL, NULL);

	if(cert_obj->asu_cert_st == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to read CA's certificate\n");
		ret = EXIT_FAILURE;
		goto error;
	}
	
	result = BIO_reset(bp);
	
	/* 分解用户证书*/
	cert_obj->user_cert_st = PEM_ASN1_read_bio((d2i_of_void *)d2i_X509, 
						PEM_STRING_X509_USER,
						bp, NULL,	NULL, NULL);
	if(cert_obj->user_cert_st == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to read USER's certificate\n");
		ret = EXIT_FAILURE;
		goto error;		
	}
#if 0
	if(debug)
	{
		/*打印结构*/
		X509_print_fp(stdout, cert_obj->asu_cert_st);
		X509_print_fp(stdout, cert_obj->user_cert_st);
		EC_KEY_print_fp(stdout, tmp_pkey,0);
	}
#endif
	/*验证公私钥对*/
	if (!EC_KEY_check_key(tmp_pkey))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s EC_KEY_check_key  failed\n", __func__);
		ret = EXIT_FAILURE;
		goto error;
	}
	
	/*验证ASU USER证书*/
	
	/*ret = x509_verify_cert(cert_obj);*/ 
	ret =0;
	//Qiuchen
	/*
	if(ret != 0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s :%d call x509_verify_cert  return %d (failure) \n", __func__, __LINE__,ret);
		print_err();
		ret = EXIT_FAILURE;
		goto error;
	}*/
	//End
	/*证书编码,并把编码数据存储到cert_obj->cert_bin->data中*/
	len = (*cert_obj->encode)(cert_obj->user_cert_st, 
					&(cert_obj->cert_bin->data), 
					cert_obj->cert_bin->length);
	if(len<0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to encode usercert\n");
		ret = EXIT_FAILURE;
		goto error;
	}
	cert_obj->cert_bin->length = len;
	asd_printf(ASD_WAPI,MSG_DEBUG,"cert_obj->cert_bin->length = %d\n", cert_obj->cert_bin->length);

        /*取用户的私钥*/
	ret = x509_get_privkey(tmp_pkey, &(cert_obj->private_key));
	asd_printf(ASD_WAPI,MSG_DEBUG,"x509_get_privkey cert_obj->private_key=%p\n",cert_obj->private_key);
	 if(ret!= 0)
	 {
	 	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s x509_get_privkey failure ,returns %d\n", __func__,ret);
		ret = EXIT_FAILURE;
	 	goto error;
	 }
	/*取ASU 的公钥*/
	cert_obj->asu_pubkey = x509_get_pubkey((X509 *)cert_obj->asu_cert_st);//) == NULL)

	if(cert_obj->asu_pubkey == NULL)
	{
	 	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d x509_get_pubkey failure \n", __func__,__LINE__);
		ret = EXIT_FAILURE;
	 	goto error;
	 }
error:	
	if(tmp_pkey != NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s:%d free tmp_pkey\n", __func__, __LINE__);
		EC_KEY_free(tmp_pkey);
	}
	return ret;
}


//三证书从bp 中分解出CA的证书
int get_x509_ca_cert(BIO *bp, struct cert_obj_st_t *cert_obj)
{
//	EC_KEY *tmp_pkey=NULL;
	int ret =  EXIT_SUCCESS;
//	int len = 0;

	
	/* 分解CA  证书*/
	cert_obj->ca_cert_st = PEM_ASN1_read_bio((d2i_of_void *)d2i_X509, 
						PEM_STRING_X509_CA, 
						bp, NULL, NULL, NULL);

	if(cert_obj->ca_cert_st == NULL)
	{
		asd_printf(ASD_WAPI,MSG_INFO,"unable to read CA's certificate\n");
		ret = EXIT_FAILURE;
		goto error;
	}
	
	
#if 0
	if(debug)
	{
		/*打印结构*/
		X509_print_fp(stdout, cert_obj->asu_cert_st);
		X509_print_fp(stdout, cert_obj->user_cert_st);
		EC_KEY_print_fp(stdout, tmp_pkey,0);
	}
#endif
	
	
	/*取CA 的公钥*/
	cert_obj->ca_pubkey = x509_get_pubkey((X509 *)cert_obj->ca_cert_st);

	if(cert_obj->ca_pubkey == NULL)
	{
	 	asd_printf(ASD_WAPI,MSG_INFO,"in %s:%d x509_get_pubkey failure \n", __func__,__LINE__);
		ret = EXIT_FAILURE;
		goto error;
	 }

error:
	return ret;
}

/*加载X509证书*/
static int load_x509(char *cert_file, const struct cert_obj_st_t *cert_obj)
{
	BIO	*bp_s = NULL;
	BIO	*bp_m = NULL;
	unsigned char tmpbuf[256] = {0,};
	int ret =  EXIT_SUCCESS;
	int len = 0;
	int result = 0;

	asd_printf(ASD_WAPI,MSG_DEBUG,"load_x509\n");
	/*分配新的文件类型BIO对象*/
	if ((bp_s = BIO_new(BIO_s_file())) == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to create I/O context");
		ret = EXIT_FAILURE;
		goto exit;
	}

	/*使用文件名设置bp_s*/
	if(!BIO_read_filename(bp_s, cert_file))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to read cert file %s\n ", cert_file);
		ret = EXIT_FAILURE;
		BIO_free(bp_s);
		return ret;
	}
	if(!(bp_m = BIO_new(BIO_s_mem()))) 
	{
		ret = EXIT_FAILURE;
		goto exit;
	}
	
	BIO_write(bp_m, "\n", 1);
	while ((len = BIO_read(bp_s, tmpbuf, sizeof( tmpbuf)))) 
	{
		if(len < 0) {
			asd_printf(ASD_WAPI,MSG_DEBUG,"BIO READ_ERROR in %s:%d\n", __func__, __LINE__);
			ret = EXIT_FAILURE;
			goto exit;
		}
		BIO_write(bp_m, tmpbuf, len);
	}
	BIO_set_flags(bp_m, BIO_FLAGS_MEM_RDONLY);
	/*分解证书*/
	ret = get_x509_cert(bp_m, (struct cert_obj_st_t *)cert_obj);
	if(ret != 0){
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s :%d call get_x509_cert  return %d (failure) \n",
			__func__, __LINE__,ret);
	}
exit:
	result = BIO_reset(bp_m);
	BIO_clear_flags(bp_m, BIO_FLAGS_MEM_RDONLY);
	BIO_free(bp_m);
	BIO_free(bp_s);
	return ret;
	
}

//加载CA证书  三证书
static int load_ca_x509(char *ca_cert_file, const struct cert_obj_st_t *ca_cert_obj)
{
	BIO	*bp_s = NULL;
	BIO	*bp_m = NULL;
	unsigned char tmpbuf[256] = {0,};
	int ret =  EXIT_SUCCESS;
	int len = 0;
	int result = 0;

	//分配新的文件类型BIO对象
	if ((bp_s = BIO_new(BIO_s_file())) == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to create I/O context");
		ret = EXIT_FAILURE;
		goto exit;
	}

	//使用文件名设置bp_s
	if(!BIO_read_filename(bp_s, ca_cert_file))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unable to read cert file %s\n ", ca_cert_file);
		ret = EXIT_FAILURE;
		BIO_free(bp_s);
		return ret;
	}
	if(!(bp_m = BIO_new(BIO_s_mem()))) 
	{
		ret = EXIT_FAILURE;
		goto exit;
	}
	
	BIO_write(bp_m, "\n", 1);
	while ((len = BIO_read(bp_s, tmpbuf, sizeof( tmpbuf)))) 
	{
		if(len < 0) {
			asd_printf(ASD_WAPI,MSG_DEBUG,"BIO READ_ERROR in %s:%d\n", __func__, __LINE__);
			ret = EXIT_FAILURE;
			goto exit;
		}
		BIO_write(bp_m, tmpbuf, len);
	}
	BIO_set_flags(bp_m, BIO_FLAGS_MEM_RDONLY);
	
// 分解CA  证书
	
	ret = get_x509_ca_cert(bp_m, (struct cert_obj_st_t *)ca_cert_obj);
	if(ret != 0){
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s :%d call get_x509_cert  return %d (failure) \n",
			__func__, __LINE__,ret);
	}
	
exit:
	result = BIO_reset(bp_m);
	BIO_clear_flags(bp_m, BIO_FLAGS_MEM_RDONLY);
	BIO_free(bp_m);
	BIO_free(bp_s);
	return ret;
	
}


int register_certificate(void *user_data)
{
	struct asd_wapi *wapi = (struct asd_wapi *)user_data;
	int res =  -1;
	unsigned short  index = wapi->cert_info.config.used_cert; /*取证书类型号*/
	asd_printf(ASD_WAPI,MSG_DEBUG,"circle.cert_info.config.used_cert = %d\n",index);	

	switch(index)
	{
		case 0:
			res = load_none(wapi->cert_info.config.cert_name, wapi->cert_info.ap_cert_obj);
			break;
		case 1:
			/*释放证书对象中分配的资源*/
			x509_free_obj_data((struct cert_obj_st_t *)wapi->cert_info.ap_cert_obj);
			/*加载X509证书*/
			res = load_x509(wapi->cert_info.config.cert_name, wapi->cert_info.ap_cert_obj);
			asd_printf(ASD_WAPI,MSG_DEBUG,"%s load_x509 res = %d\n",__func__,res);			
			if((wapi->multi_cert) && (res==EXIT_SUCCESS))
			{
				res =load_ca_x509(wapi->cert_info.config.ca_cert_name, wapi->cert_info.ap_cert_obj);
				asd_printf(ASD_WAPI,MSG_DEBUG,"%s load_ca_x509 res = %d\n",__func__,res);			
			}
			break;
		case 2:
			/*加载GBW证书*/
			
			//gbw_free_obj_data((struct cert_obj_st_t *)cert_objs[index]);
			//res = load_gbw(apcert_info->config.cert_name, cert_objs[index]);
			break;
		default:
			break;
	}
	
	if(res == 0)
	{
		circle.cert_info.ap_cert_obj= (struct cert_obj_st_t *)cert_objs[index];
	}
	return res;	
}

/*比较证书公钥算法参数字段OID*/
int cmp_oid(u8 *in , int len)
{
	char pubkey_alg_para_oid_der[16] = {0x06, 0x09,0x2a,0x81,0x1c, 0xd7,0x63,0x01,0x01,0x02,0x01};

	if(len != 11) 
	{
		return -1;
	}
	if(memcmp(pubkey_alg_para_oid_der, in, 11)!=0)
	{
		return -2;
	}
	return 0;
}


/*注销证书*/
void unregister_certificate()
{
	x509_free_obj_data((struct cert_obj_st_t *)cert_objs[1]);
	//gbw_free_obj_data((struct cert_obj_st_t *)cert_objs[2]);
}
