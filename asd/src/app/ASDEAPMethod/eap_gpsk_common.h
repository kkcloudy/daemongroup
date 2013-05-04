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
* eap_gpsk_common.h
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

#ifndef EAP_GPSK_COMMON_H
#define EAP_GPSK_COMMON_H

#define EAP_GPSK_OPCODE_GPSK_1 1
#define EAP_GPSK_OPCODE_GPSK_2 2
#define EAP_GPSK_OPCODE_GPSK_3 3
#define EAP_GPSK_OPCODE_GPSK_4 4
#define EAP_GPSK_OPCODE_FAIL 5
#define EAP_GPSK_OPCODE_PROTECTED_FAIL 6

/* Failure-Code in GPSK-Fail and GPSK-Protected-Fail */
#define EAP_GPSK_FAIL_PSK_NOT_FOUND 0x00000001
#define EAP_GPSK_FAIL_AUTHENTICATION_FAILURE 0x00000002
#define EAP_GPSK_FAIL_AUTHORIZATION_FAILURE 0x00000003

#define EAP_GPSK_RAND_LEN 32
#define EAP_GPSK_MAX_SK_LEN 32
#define EAP_GPSK_MAX_PK_LEN 32
#define EAP_GPSK_MAX_MIC_LEN 32

#define EAP_GPSK_VENDOR_IETF		0x00000000
#define EAP_GPSK_CIPHER_RESERVED	0x000000
#define EAP_GPSK_CIPHER_AES		0x000001
#define EAP_GPSK_CIPHER_SHA256		0x000002


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_gpsk_csuite {
	u8 vendor[4];
	u8 specifier[2];
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

int eap_gpsk_supported_ciphersuite(int vendor, int specifier);
int eap_gpsk_derive_keys(const u8 *psk, size_t psk_len, int vendor,
			 int specifier,
			 const u8 *rand_client, const u8 *rand_server,
			 const u8 *id_client, size_t id_client_len,
			 const u8 *id_server, size_t id_server_len,
			 u8 *msk, u8 *emsk, u8 *sk, size_t *sk_len,
			 u8 *pk, size_t *pk_len);
size_t eap_gpsk_mic_len(int vendor, int specifier);
int eap_gpsk_compute_mic(const u8 *sk, size_t sk_len, int vendor,
			 int specifier, const u8 *data, size_t len, u8 *mic);

#endif /* EAP_GPSK_COMMON_H */
