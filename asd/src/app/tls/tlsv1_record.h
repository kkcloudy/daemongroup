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
* Tlsv1_record.h
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

#ifndef TLSV1_RECORD_H
#define TLSV1_RECORD_H

#include "crypto.h"

#define TLS_MAX_WRITE_MAC_SECRET_LEN 20
#define TLS_MAX_WRITE_KEY_LEN 32
#define TLS_MAX_IV_LEN 16
#define TLS_MAX_KEY_BLOCK_LEN (2 * (TLS_MAX_WRITE_MAC_SECRET_LEN + \
				    TLS_MAX_WRITE_KEY_LEN + TLS_MAX_IV_LEN))

#define TLS_SEQ_NUM_LEN 8
#define TLS_RECORD_HEADER_LEN 5

/* ContentType */
enum {
	TLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC = 20,
	TLS_CONTENT_TYPE_ALERT = 21,
	TLS_CONTENT_TYPE_HANDSHAKE = 22,
	TLS_CONTENT_TYPE_APPLICATION_DATA = 23
};

struct tlsv1_record_layer {
	u8 write_mac_secret[TLS_MAX_WRITE_MAC_SECRET_LEN];
	u8 read_mac_secret[TLS_MAX_WRITE_MAC_SECRET_LEN];
	u8 write_key[TLS_MAX_WRITE_KEY_LEN];
	u8 read_key[TLS_MAX_WRITE_KEY_LEN];
	u8 write_iv[TLS_MAX_IV_LEN];
	u8 read_iv[TLS_MAX_IV_LEN];

	size_t hash_size;
	size_t key_material_len;
	size_t iv_size; /* also block_size */

	enum crypto_hash_alg hash_alg;
	enum crypto_cipher_alg cipher_alg;

	u8 write_seq_num[TLS_SEQ_NUM_LEN];
	u8 read_seq_num[TLS_SEQ_NUM_LEN];

	u16 cipher_suite;
	u16 write_cipher_suite;
	u16 read_cipher_suite;

	struct crypto_cipher *write_cbc;
	struct crypto_cipher *read_cbc;
};


int tlsv1_record_set_cipher_suite(struct tlsv1_record_layer *rl,
				  u16 cipher_suite);
int tlsv1_record_change_write_cipher(struct tlsv1_record_layer *rl);
int tlsv1_record_change_read_cipher(struct tlsv1_record_layer *rl);
int tlsv1_record_send(struct tlsv1_record_layer *rl, u8 content_type, u8 *buf,
		      size_t buf_size, size_t payload_len, size_t *out_len);
int tlsv1_record_receive(struct tlsv1_record_layer *rl,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t *out_len, u8 *alert);

#endif /* TLSV1_RECORD_H */
