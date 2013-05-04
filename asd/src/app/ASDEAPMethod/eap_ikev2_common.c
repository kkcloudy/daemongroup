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
* AsdEapIkev2Common.c
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
#include "eap_defs.h"
#include "eap_common.h"
#include "ikev2_common.h"
#include "eap_ikev2_common.h"


int eap_ikev2_derive_keymat(int prf, struct ikev2_keys *keys,
			    const u8 *i_nonce, size_t i_nonce_len,
			    const u8 *r_nonce, size_t r_nonce_len,
			    u8 *keymat)
{
	u8 *nonces;
	size_t nlen;

	/* KEYMAT = prf+(SK_d, Ni | Nr) */
	if (keys->SK_d == NULL || i_nonce == NULL || r_nonce == NULL)
		return -1;

	nlen = i_nonce_len + r_nonce_len;
	nonces = os_zalloc(nlen);
	if (nonces == NULL)
		return -1;
	os_memcpy(nonces, i_nonce, i_nonce_len);
	os_memcpy(nonces + i_nonce_len, r_nonce, r_nonce_len);

	if (ikev2_prf_plus(prf, keys->SK_d, keys->SK_d_len, nonces, nlen,
			   keymat, EAP_MSK_LEN + EAP_EMSK_LEN)) {
		os_free(nonces);
		return -1;
	}
	os_free(nonces);

	wpa_hexdump_key(MSG_DEBUG, "EAP-IKEV2: KEYMAT",
			keymat, EAP_MSK_LEN + EAP_EMSK_LEN);

	return 0;
}


struct wpabuf * eap_ikev2_build_frag_ack(u8 id, u8 code)
{
	struct wpabuf *msg;

#ifdef CCNS_PL
	msg = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_IKEV2, 1, code, id);
	if (msg == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "EAP-IKEV2: Failed to allocate memory "
			   "for fragment ack");
		return NULL;
	}
	wpabuf_put_u8(msg, 0); /* Flags */
#else /* CCNS_PL */
	msg = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_IKEV2, 0, code, id);
	if (msg == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "EAP-IKEV2: Failed to allocate memory "
			   "for fragment ack");
		return NULL;
	}
#endif /* CCNS_PL */

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Send fragment ack");

	return msg;
}


int eap_ikev2_validate_icv(int integ_alg, struct ikev2_keys *keys,
			   int initiator, const struct wpabuf *msg,
			   const u8 *pos, const u8 *end)
{
	const struct ikev2_integ_alg *integ;
	size_t icv_len;
	u8 icv[IKEV2_MAX_HASH_LEN];
	const u8 *SK_a = initiator ? keys->SK_ai : keys->SK_ar;

	integ = ikev2_get_integ(integ_alg);
	if (integ == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Unknown INTEG "
			   "transform / cannot validate ICV");
		return -1;
	}
	icv_len = integ->hash_len;

	if (end - pos < (int) icv_len) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Not enough room in the "
			   "message for Integrity Checksum Data");
		return -1;
	}

	if (SK_a == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: No SK_a for ICV validation");
		return -1;
	}

	if (ikev2_integ_hash(integ_alg, SK_a, keys->SK_integ_len,
			     wpabuf_head(msg),
			     wpabuf_len(msg) - icv_len, icv) < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Could not calculate ICV");
		return -1;
	}

	if (os_memcmp(icv, end - icv_len, icv_len) != 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Invalid ICV");
		wpa_hexdump(MSG_DEBUG, "EAP-IKEV2: Calculated ICV",
			    icv, icv_len);
		wpa_hexdump(MSG_DEBUG, "EAP-IKEV2: Received ICV",
			    end - icv_len, icv_len);
		return -1;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-IKEV2: Valid Integrity Checksum Data in "
		   "the received message");

	return icv_len;
}
