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
* ecc_crypt.h
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

#ifndef _ECC_CCRYPT_H
#define _ECC_CCRYPT_H

#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/ecdh.h"
#include "openssl/evp.h"
#include "openssl/err.h"

#define EC962_PRIVKEY_LEN	24
#define EC962_SIGN_LEN		48


int point2bin(EC_KEY *eckey, unsigned char *pub_s , int pub_sl);
EC_KEY *geteckey_by_curve(int nid );
EC_KEY * octkey2eckey(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl,	int nid);

int sign2bin(EC_KEY  *eckey, ECDSA_SIG *s, unsigned char *signature,  int *siglen);
int   ecc192_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sig,int sign_len);
int ecc192_sign(const unsigned char *priv_s, int priv_sl,	const unsigned char * in, int in_len, unsigned char *out);

int ecc192_ecdh(const unsigned char *apub_s, int apub_sl,
				const unsigned char *bpriv_s, int bpriv_sl,
				const unsigned char *bpub_s, int bpub_sl,
				unsigned char *key, int keyl);
int ecc192_verify_key(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl);
#endif
