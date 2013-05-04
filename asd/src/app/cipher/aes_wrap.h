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
* Aes_Wrap.h
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
#ifndef AES_WRAP_H
#define AES_WRAP_H

int __must_check aes_wrap(const u8 *kek, int n, const u8 *plain, u8 *cipher);
int __must_check aes_unwrap(const u8 *kek, int n, const u8 *cipher, u8 *plain);
int __must_check omac1_aes_128_vector(const u8 *key, size_t num_elem,
				      const u8 *addr[], const size_t *len,
				      u8 *mac);
int __must_check omac1_aes_128(const u8 *key, const u8 *data, size_t data_len,
			       u8 *mac);
int __must_check aes_128_encrypt_block(const u8 *key, const u8 *in, u8 *out);
int __must_check aes_128_ctr_encrypt(const u8 *key, const u8 *nonce,
				     u8 *data, size_t data_len);
int __must_check aes_128_eax_encrypt(const u8 *key,
				     const u8 *nonce, size_t nonce_len,
				     const u8 *hdr, size_t hdr_len,
				     u8 *data, size_t data_len, u8 *tag);
int __must_check aes_128_eax_decrypt(const u8 *key,
				     const u8 *nonce, size_t nonce_len,
				     const u8 *hdr, size_t hdr_len,
				     u8 *data, size_t data_len, const u8 *tag);
int __must_check aes_128_cbc_encrypt(const u8 *key, const u8 *iv, u8 *data,
				     size_t data_len);
int __must_check aes_128_cbc_decrypt(const u8 *key, const u8 *iv, u8 *data,
				     size_t data_len);

#endif /* AES_WRAP_H */
