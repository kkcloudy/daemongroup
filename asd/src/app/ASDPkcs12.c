#include <openssl/opensslconf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ASDPkcs12App.h"

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>


EVP_CIPHER const *enc;
CONF *config=NULL;
BIO *bio_err=NULL;


#define NOKEYS		0x1
#define NOCERTS 	0x2
#define INFO		0x4
#define CLCERTS		0x8
#define CACERTS		0x10

int get_cert_chain (X509 *cert, X509_STORE *store, STACK_OF(X509) **chain);
int dump_certs_keys_p12(BIO *out, PKCS12 *p12, char *pass, int passlen, int options, char *pempass);
int dump_certs_pkeys_bags(BIO *out, STACK_OF(PKCS12_SAFEBAG) *bags, char *pass,
			  int passlen, int options, char *pempass);
int dump_certs_pkeys_bag(BIO *out, PKCS12_SAFEBAG *bags, char *pass, int passlen, int options, char *pempass);
int print_attribs(BIO *out, STACK_OF(X509_ATTRIBUTE) *attrlst,const char *name);
void hex_prin(BIO *out, unsigned char *buf, int len);
int alg_print(BIO *x, X509_ALGOR *alg);
int cert_load(BIO *in, STACK_OF(X509) *sk);

int pkcs12_decryption(char * o_file, char * n_file, char * passwd)
{
    char *infile=NULL, *outfile=NULL;
    BIO *in=NULL, *out = NULL;
    PKCS12 *p12 = NULL;
    char pass[50], macpass[50];
    int export_cert = 0;
    int options = 0;
    int twopass = 0;
    int ret = 1;
	int rret = 0;
    int macver = 1;
    int noprompt = 0;
    char *cpass = NULL, *mpass = NULL;
    char *passargin = NULL, *passargout = NULL, *passarg = NULL;
    char *passin = NULL, *passout = NULL;
 /*   char *inrand = NULL;
    char *CApath = NULL, *CAfile = NULL;
	char *keyname = NULL;	
    char *certfile=NULL;
    char **args;
    char *name = NULL;
    char *csp_name = NULL;
    int chain = 0;
    int badarg = 0;
    int iter = PKCS12_DEFAULT_ITER;
    int maciter = PKCS12_DEFAULT_ITER;
    int keytype = 0;
    int cert_pbe = NID_pbe_WithSHA1And40BitRC2_CBC;
    int key_pbe = NID_pbe_WithSHA1And3_Key_TripleDES_CBC;
    ENGINE *e = NULL;
    STACK *canames = NULL;
*/
    enc = EVP_des_ede3_cbc();
    if (bio_err == NULL ) bio_err = BIO_new_fp (stderr, BIO_NOCLOSE);

	if (!load_config(bio_err, NULL)){
		rret = 1;
		goto end;
	}

	infile = o_file;
	outfile = n_file;
	printf("infile %s outfile %s\n",infile, outfile);
printf("1\n");
    if(passarg) {
		printf("2\n");
	if(export_cert) passargout = passarg;
	else passargin = passarg;
    }

    if(!app_passwd(bio_err, passargin, passargout, &passin, &passout)) {
	BIO_printf(bio_err, "Error getting passwords\n");
	rret = 1;
	goto end;
    }

    if(!cpass) {
		printf("3\n");
    	if(export_cert) cpass = passout;
    	else cpass = passin;
    }

    if(cpass) {
		printf("4\n");
	mpass = cpass;
	noprompt = 1;
    } else {
		printf("5\n");
	cpass = pass;
	mpass = macpass;
    }

	printf("7\n");
    ERR_load_crypto_strings();
	printf("8\n");


    if (!infile) {
		printf("9\n");
		in = BIO_new_fp(stdin, BIO_NOCLOSE);
	}
    else {
		printf("10\n");
		in = BIO_new_file(infile, "rb");
	}
    if (!in) {
	    BIO_printf(bio_err, "Error opening input file %s\n",
						infile ? infile : "<stdin>");
	    perror (infile);
		rret = 1;
	    goto end;
   }


    if (!outfile) {
		printf("11\n");
	out = BIO_new_fp(stdout, BIO_NOCLOSE);
    } else {
		printf("12\n");
		out = BIO_new_file(outfile, "wb");
	}
    if (!out) {
	BIO_printf(bio_err, "Error opening output file %s\n",
						outfile ? outfile : "<stdout>");
	perror (outfile);
	rret = 1;
	goto end;
    }

	printf("33\n");
    if (!(p12 = d2i_PKCS12_bio (in, NULL))) {
	ERR_print_errors(bio_err);
	rret = 1;
	goto end;
    }
	printf("34\n");
	printf("35\n");
	printf("pass1 %s\n",pass);
	memset(pass,0,50);
	memcpy(pass,passwd,strlen(passwd));
	printf("pass2 %s\n",pass);
	printf("36\n");
	printf("37\n");
    if (!twopass) BUF_strlcpy(macpass, pass, sizeof macpass);
	printf("38\n");

    if (options & INFO) BIO_printf (bio_err, "MAC Iteration %ld\n", p12->mac->iter ? ASN1_INTEGER_get (p12->mac->iter) : 1);
    if(macver) {
	/* If we enter empty password try no password first */
	printf("39\n");
	int ret111;
	int ret222;
	ret111 = PKCS12_verify_mac(p12, NULL, 0);
	printf("ret111 %d\n",ret111);
	if(!mpass[0] && ret111) {
		printf("40\n");
		/* If mac and crypto pass the same set it to NULL too */
		if(!twopass){ 
			printf("41\n");
			cpass = NULL;
		}
	} else {
		printf("mpass %s\n",mpass);
		ret222 = PKCS12_verify_mac(p12, mpass, -1);
		printf("ret222 %d\n",ret222);
		printf("mpass %s\n",mpass);
		if (!ret222) {
	    BIO_printf (bio_err, "Mac verify error: invalid password?\n");
	    ERR_print_errors (bio_err);
		rret = 1;
	    goto end;
	}
	}
	printf("42\n");
	BIO_printf (bio_err, "MAC verified OK\n");
    }

	printf("passout1 %s\n",passout);
    if (!dump_certs_keys_p12 (out, p12, cpass, -1, options, passout)) {
		printf("44\n");
	BIO_printf(bio_err, "Error outputting keys and certificates\n");
	ERR_print_errors (bio_err);
	rret = 1;
	goto end;
    }
	printf("passout2 %s\n",passout);
	printf("45\n");
    ret = 0;
 end:
    if (p12) PKCS12_free(p12);
	printf("46\n");
	printf("47\n");
    BIO_free(in);
    BIO_free_all(out);
	return rret;
}

int dump_certs_keys_p12 (BIO *out, PKCS12 *p12, char *pass,
	     int passlen, int options, char *pempass)
{
	STACK_OF(PKCS7) *asafes = NULL;
	STACK_OF(PKCS12_SAFEBAG) *bags;
	int i, bagnid;
	int ret = 0;
	PKCS7 *p7;
printf("11\n");
	if (!( asafes = PKCS12_unpack_authsafes(p12))) return 0;
	printf("22\n");
	for (i = 0; i < sk_PKCS7_num (asafes); i++) {
		printf("33\n");
		p7 = sk_PKCS7_value (asafes, i);
		printf("44\n");
		bagnid = OBJ_obj2nid (p7->type);
		printf("55\n");
		if (bagnid == NID_pkcs7_data) {
			printf("66\n");
			bags = PKCS12_unpack_p7data(p7);
			printf("77\n");
			if (options & INFO) BIO_printf (bio_err, "PKCS7 Data\n");
			printf("88\n");
		} else if (bagnid == NID_pkcs7_encrypted) {
			printf("99\n");
			if (options & INFO) {
				printf("1010\n");
				BIO_printf(bio_err, "PKCS7 Encrypted data: ");
				alg_print(bio_err, 
					p7->d.encrypted->enc_data->algorithm);
			}
			printf("1111\n");
			bags = PKCS12_unpack_p7encdata(p7, pass, passlen);
			printf("1212\n");
		} else continue;
		if (!bags) goto err;
		printf("1313\n");
	    	if (!dump_certs_pkeys_bags (out, bags, pass, passlen, 
						 options, pempass)) {
				printf("1414\n");
			sk_PKCS12_SAFEBAG_pop_free (bags, PKCS12_SAFEBAG_free);
			goto err;
		}
			printf("1515\n");
		sk_PKCS12_SAFEBAG_pop_free (bags, PKCS12_SAFEBAG_free);
		printf("1616\n");
		bags = NULL;
		printf("1717\n");
	}
	printf("1818\n");
	ret = 1;
	printf("1919\n");

	err:

	if (asafes)
		sk_PKCS7_pop_free (asafes, PKCS7_free);
	return ret;
}

int dump_certs_pkeys_bags (BIO *out, STACK_OF(PKCS12_SAFEBAG) *bags,
			   char *pass, int passlen, int options, char *pempass)
{
	int i;
	for (i = 0; i < sk_PKCS12_SAFEBAG_num (bags); i++) {
		if (!dump_certs_pkeys_bag (out,
					   sk_PKCS12_SAFEBAG_value (bags, i),
					   pass, passlen,
					   options, pempass))
		    return 0;
	}
	return 1;
}

int dump_certs_pkeys_bag (BIO *out, PKCS12_SAFEBAG *bag, char *pass,
	     int passlen, int options, char *pempass)
{
	EVP_PKEY *pkey;
	PKCS8_PRIV_KEY_INFO *p8;
	X509 *x509;
	
	switch (M_PKCS12_bag_type(bag))
	{
	case NID_keyBag:
		printf("%s1\n",__func__);
		if (options & INFO) BIO_printf (bio_err, "Key bag\n");
		printf("%s2\n",__func__);
		if (options & NOKEYS) return 1;
		printf("%s3\n",__func__);
		//print_attribs (out, bag->attrib, "Bag Attributes");
		p8 = bag->value.keybag;
		printf("%s4\n",__func__);
		if (!(pkey = EVP_PKCS82PKEY (p8))) return 0;
		printf("%s5\n",__func__);
		//print_attribs (out, p8->attributes, "Key Attributes");
		printf("%s6\n",__func__);
		PEM_write_bio_PrivateKey (out, pkey, NULL, NULL, 0, NULL, pempass);
		printf("%s7\n",__func__);
		EVP_PKEY_free(pkey);
		printf("%s8\n",__func__);
	break;

	case NID_pkcs8ShroudedKeyBag:
		printf("%s9\n",__func__);
		if (options & INFO) {
			printf("%s10\n",__func__);
			BIO_printf (bio_err, "Shrouded Keybag: ");
			alg_print (bio_err, bag->value.shkeybag->algor);
		}
		printf("%s11\n",__func__);
		if (options & NOKEYS) return 1;
		printf("%s12\n",__func__);
		//print_attribs (out, bag->attrib, "Bag Attributes");
		printf("%s13\n",__func__);
		if (!(p8 = PKCS12_decrypt_skey(bag, pass, passlen)))
				return 0;
		printf("%s14\n",__func__);
		if (!(pkey = EVP_PKCS82PKEY (p8))) {
			PKCS8_PRIV_KEY_INFO_free(p8);
			return 0;
		}
		printf("%s15\n",__func__);
		//print_attribs (out, p8->attributes, "Key Attributes");
		PKCS8_PRIV_KEY_INFO_free(p8);
		PEM_write_bio_PrivateKey (out, pkey, NULL, NULL, 0, NULL, pempass);
		printf("%s16\n",__func__);
		EVP_PKEY_free(pkey);
		printf("%s17\n",__func__);
	break;

	case NID_certBag:
		printf("%s18\n",__func__);
		if (options & INFO) BIO_printf (bio_err, "Certificate bag\n");
		printf("%s19\n",__func__);
		if (options & NOCERTS) return 1;
		printf("%s20\n",__func__);
                if (PKCS12_get_attr(bag, NID_localKeyID)) {
			if (options & CACERTS) return 1;
		} else if (options & CLCERTS) return 1;
				printf("%s21\n",__func__);
		//print_attribs (out, bag->attrib, "Bag Attributes");
		printf("%s22\n",__func__);
		if (M_PKCS12_cert_bag_type(bag) != NID_x509Certificate )
								 return 1;
		printf("%s23\n",__func__);
		if (!(x509 = PKCS12_certbag2x509(bag))) return 0;
		printf("%s24\n",__func__);
		//dump_cert_text (out, x509);
		printf("%s25\n",__func__);
		PEM_write_bio_X509 (out, x509);
		X509_free(x509);
		printf("%s26\n",__func__);
	break;

	case NID_safeContentsBag:
		printf("%s27\n",__func__);
		if (options & INFO) BIO_printf (bio_err, "Safe Contents bag\n");
		//print_attribs (out, bag->attrib, "Bag Attributes");
		printf("%s28\n",__func__);
		return dump_certs_pkeys_bags (out, bag->value.safes, pass,
							    passlen, options, pempass);
					
	default:
		BIO_printf (bio_err, "Warning unsupported bag type: ");
		i2a_ASN1_OBJECT (bio_err, bag->type);
		BIO_printf (bio_err, "\n");
		return 1;
	break;
	}
	return 1;
}

/* Given a single certificate return a verified chain or NULL if error */

/* Hope this is OK .... */

int get_cert_chain (X509 *cert, X509_STORE *store, STACK_OF(X509) **chain)
{
	X509_STORE_CTX store_ctx;
	STACK_OF(X509) *chn;
	int i = 0;

	/* FIXME: Should really check the return status of X509_STORE_CTX_init
	 * for an error, but how that fits into the return value of this
	 * function is less obvious. */
	X509_STORE_CTX_init(&store_ctx, store, cert, NULL);
	if (X509_verify_cert(&store_ctx) <= 0) {
		i = X509_STORE_CTX_get_error (&store_ctx);
		if (i == 0)
			/* avoid returning 0 if X509_verify_cert() did not
			 * set an appropriate error value in the context */
			i = -1;
		chn = NULL;
		goto err;
	} else
		chn = X509_STORE_CTX_get1_chain(&store_ctx);
err:
	X509_STORE_CTX_cleanup(&store_ctx);
	*chain = chn;
	
	return i;
}	

int alg_print (BIO *x, X509_ALGOR *alg)
{
	PBEPARAM *pbe;
	const unsigned char *p;
	p = alg->parameter->value.sequence->data;
	pbe = d2i_PBEPARAM(NULL, &p, alg->parameter->value.sequence->length);
	if (!pbe)
		return 1;
	BIO_printf (bio_err, "%s, Iteration %ld\n", 
		OBJ_nid2ln(OBJ_obj2nid(alg->algorithm)),
		ASN1_INTEGER_get(pbe->iter));
	PBEPARAM_free (pbe);
	return 1;
}

/* Load all certificates from a given file */

int cert_load(BIO *in, STACK_OF(X509) *sk)
{
	int ret;
	X509 *cert;
	ret = 0;
#ifdef CRYPTO_MDEBUG
	CRYPTO_push_info("cert_load(): reading one cert");
#endif
	while((cert = PEM_read_bio_X509(in, NULL, NULL, NULL))) {
#ifdef CRYPTO_MDEBUG
		CRYPTO_pop_info();
#endif
		ret = 1;
		sk_X509_push(sk, cert);
#ifdef CRYPTO_MDEBUG
		CRYPTO_push_info("cert_load(): reading one cert");
#endif
	}
#ifdef CRYPTO_MDEBUG
	CRYPTO_pop_info();
#endif
	if(ret) ERR_clear_error();
	return ret;
}

/* Generalised attribute print: handle PKCS#8 and bag attributes */

int print_attribs (BIO *out, STACK_OF(X509_ATTRIBUTE) *attrlst,const char *name)
{
	X509_ATTRIBUTE *attr;
	ASN1_TYPE *av;
	char *value;
	int i, attr_nid;
	if(!attrlst) {
		BIO_printf(out, "%s: <No Attributes>\n", name);
		return 1;
	}
	if(!sk_X509_ATTRIBUTE_num(attrlst)) {
		BIO_printf(out, "%s: <Empty Attributes>\n", name);
		return 1;
	}
	BIO_printf(out, "%s\n", name);
	for(i = 0; i < sk_X509_ATTRIBUTE_num(attrlst); i++) {
		attr = sk_X509_ATTRIBUTE_value(attrlst, i);
		attr_nid = OBJ_obj2nid(attr->object);
		BIO_printf(out, "    ");
		if(attr_nid == NID_undef) {
			i2a_ASN1_OBJECT (out, attr->object);
			BIO_printf(out, ": ");
		} else BIO_printf(out, "%s: ", OBJ_nid2ln(attr_nid));

		if(sk_ASN1_TYPE_num(attr->value.set)) {
			av = sk_ASN1_TYPE_value(attr->value.set, 0);
			switch(av->type) {
				case V_ASN1_BMPSTRING:
        			value = uni2asc(av->value.bmpstring->data,
                                	       av->value.bmpstring->length);
				BIO_printf(out, "%s\n", value);
				OPENSSL_free(value);
				break;

				case V_ASN1_OCTET_STRING:
				hex_prin(out, av->value.octet_string->data,
					av->value.octet_string->length);
				BIO_printf(out, "\n");	
				break;

				case V_ASN1_BIT_STRING:
				hex_prin(out, av->value.bit_string->data,
					av->value.bit_string->length);
				BIO_printf(out, "\n");	
				break;

				default:
					BIO_printf(out, "<Unsupported tag %d>\n", av->type);
				break;
			}
		} else BIO_printf(out, "<No Values>\n");
	}
	return 1;
}

void hex_prin(BIO *out, unsigned char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) BIO_printf (out, "%02X ", buf[i]);
}


