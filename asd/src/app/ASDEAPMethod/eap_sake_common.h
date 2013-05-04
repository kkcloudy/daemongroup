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
* eap_sake_common.h
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

#ifndef EAP_SAKE_COMMON_H
#define EAP_SAKE_COMMON_H

#define EAP_SAKE_VERSION 2

#define EAP_SAKE_SUBTYPE_CHALLENGE 1
#define EAP_SAKE_SUBTYPE_CONFIRM 2
#define EAP_SAKE_SUBTYPE_AUTH_REJECT 3
#define EAP_SAKE_SUBTYPE_IDENTITY 4

#define EAP_SAKE_AT_RAND_S 1
#define EAP_SAKE_AT_RAND_P 2
#define EAP_SAKE_AT_MIC_S 3
#define EAP_SAKE_AT_MIC_P 4
#define EAP_SAKE_AT_SERVERID 5
#define EAP_SAKE_AT_PEERID 6
#define EAP_SAKE_AT_SPI_S 7
#define EAP_SAKE_AT_SPI_P 8
#define EAP_SAKE_AT_ANY_ID_REQ 9
#define EAP_SAKE_AT_PERM_ID_REQ 10
#define EAP_SAKE_AT_ENCR_DATA 128
#define EAP_SAKE_AT_IV 129
#define EAP_SAKE_AT_PADDING 130
#define EAP_SAKE_AT_NEXT_TMPID 131
#define EAP_SAKE_AT_MSK_LIFE 132

#define EAP_SAKE_RAND_LEN 16
#define EAP_SAKE_MIC_LEN 16
#define EAP_SAKE_ROOT_SECRET_LEN 16
#define EAP_SAKE_SMS_LEN 16
#define EAP_SAKE_TEK_AUTH_LEN 16
#define EAP_SAKE_TEK_CIPHER_LEN 16
#define EAP_SAKE_TEK_LEN (EAP_SAKE_TEK_AUTH_LEN + EAP_SAKE_TEK_CIPHER_LEN)

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_sake_hdr {
	u8 version; /* EAP_SAKE_VERSION */
	u8 session_id;
	u8 subtype;
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


struct eap_sake_parse_attr {
	const u8 *rand_s;
	const u8 *rand_p;
	const u8 *mic_s;
	const u8 *mic_p;
	const u8 *serverid;
	size_t serverid_len;
	const u8 *peerid;
	size_t peerid_len;
	const u8 *spi_s;
	size_t spi_s_len;
	const u8 *spi_p;
	size_t spi_p_len;
	const u8 *any_id_req;
	const u8 *perm_id_req;
	const u8 *encr_data;
	size_t encr_data_len;
	const u8 *iv;
	size_t iv_len;
	const u8 *next_tmpid;
	size_t next_tmpid_len;
	const u8 *msk_life;
};

int eap_sake_parse_attributes(const u8 *buf, size_t len,
			      struct eap_sake_parse_attr *attr);
void eap_sake_derive_keys(const u8 *root_secret_a, const u8 *root_secret_b,
			  const u8 *rand_s, const u8 *rand_p,
			  u8 *tek, u8 *msk, u8 *emsk);
int eap_sake_compute_mic(const u8 *tek_auth,
			 const u8 *rand_s, const u8 *rand_p,
			 const u8 *serverid, size_t serverid_len,
			 const u8 *peerid, size_t peerid_len,
			 int peer, const u8 *eap, size_t eap_len,
			 const u8 *mic_pos, u8 *mic);
void eap_sake_add_attr(struct wpabuf *buf, u8 type, const u8 *data,
		       size_t len);

#endif /* EAP_SAKE_COMMON_H */
