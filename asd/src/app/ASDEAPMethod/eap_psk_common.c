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
* AsdEapPskCommon.c
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

#include "includes.h"

#include "common.h"
#include "aes_wrap.h"
#include "eap_defs.h"
#include "eap_psk_common.h"

#define aes_block_size 16


int eap_psk_key_setup(const u8 *psk, u8 *ak, u8 *kdk)
{
	os_memset(ak, 0, aes_block_size);
	if (aes_128_encrypt_block(psk, ak, ak))
		return -1;
	os_memcpy(kdk, ak, aes_block_size);
	ak[aes_block_size - 1] ^= 0x01;
	kdk[aes_block_size - 1] ^= 0x02;
	if (aes_128_encrypt_block(psk, ak, ak) ||
	    aes_128_encrypt_block(psk, kdk, kdk))
		return -1;
	return 0;
}


int eap_psk_derive_keys(const u8 *kdk, const u8 *rand_p, u8 *tek, u8 *msk,
			u8 *emsk)
{
	u8 hash[aes_block_size];
	u8 counter = 1;
	int i;

	if (aes_128_encrypt_block(kdk, rand_p, hash))
		return -1;

	hash[aes_block_size - 1] ^= counter;
	if (aes_128_encrypt_block(kdk, hash, tek))
		return -1;
	hash[aes_block_size - 1] ^= counter;
	counter++;

	for (i = 0; i < EAP_MSK_LEN / aes_block_size; i++) {
		hash[aes_block_size - 1] ^= counter;
		if (aes_128_encrypt_block(kdk, hash, &msk[i * aes_block_size]))
			return -1;
		hash[aes_block_size - 1] ^= counter;
		counter++;
	}

	for (i = 0; i < EAP_EMSK_LEN / aes_block_size; i++) {
		hash[aes_block_size - 1] ^= counter;
		if (aes_128_encrypt_block(kdk, hash,
					  &emsk[i * aes_block_size]))
			return -1;
		hash[aes_block_size - 1] ^= counter;
		counter++;
	}

	return 0;
}
