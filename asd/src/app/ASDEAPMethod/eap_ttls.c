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
* AsdEapTtls.c
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
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"

#include "includes.h"

#include "common.h"
#include "ASDEAPMethod/eap_i.h"
#include "ASDEAPMethod/eap_tls_common.h"
#include "ms_funcs.h"
#include "sha1.h"
#include "ASDEAPMethod/chap.h"
#include "tls.h"
#include "ASDEAPMethod/eap_ttls.h"


/* Maximum supported TTLS version
 * 0 = draft-ietf-pppext-eap-ttls-03.txt / draft-funk-eap-ttls-v0-00.txt
 * 1 = draft-funk-eap-ttls-v1-00.txt
 */
#ifndef EAP_TTLS_VERSION
#define EAP_TTLS_VERSION 0 /* TTLSv1 implementation is not yet complete */
#endif /* EAP_TTLS_VERSION */


#define MSCHAPV2_KEY_LEN 16


static void eap_ttls_reset(struct eap_sm *sm, void *priv);


struct eap_ttls_data {
	struct eap_ssl_data ssl;
	enum {
		START, PHASE1, PHASE2_START, PHASE2_METHOD,
		PHASE2_MSCHAPV2_RESP, PHASE_FINISHED, SUCCESS, FAILURE
	} state;

	int ttls_version;
	int force_version;
	const struct eap_method *phase2_method;
	void *phase2_priv;
	int mschapv2_resp_ok;
	u8 mschapv2_auth_response[20];
	u8 mschapv2_ident;
	int tls_ia_configured;
	struct wpabuf *pending_phase2_eap_resp;
};


static const char * eap_ttls_state_txt(int state)
{
	switch (state) {
	case START:
		return "START";
	case PHASE1:
		return "PHASE1";
	case PHASE2_START:
		return "PHASE2_START";
	case PHASE2_METHOD:
		return "PHASE2_METHOD";
	case PHASE2_MSCHAPV2_RESP:
		return "PHASE2_MSCHAPV2_RESP";
	case PHASE_FINISHED:
		return "PHASE_FINISHED";
	case SUCCESS:
		return "SUCCESS";
	case FAILURE:
		return "FAILURE";
	default:
		return "Unknown?!";
	}
}


static void eap_ttls_state(struct eap_ttls_data *data, int state)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: %s -> %s",
		   eap_ttls_state_txt(data->state),
		   eap_ttls_state_txt(state));
	data->state = state;
}


static u8 * eap_ttls_avp_hdr(u8 *avphdr, u32 avp_code, u32 vendor_id,
			     int mandatory, size_t len)
{
	struct ttls_avp_vendor *avp;
	u8 flags;
	size_t hdrlen;

	avp = (struct ttls_avp_vendor *) avphdr;
	flags = mandatory ? AVP_FLAGS_MANDATORY : 0;
	if (vendor_id) {
		flags |= AVP_FLAGS_VENDOR;
		hdrlen = sizeof(*avp);
		avp->vendor_id = host_to_be32(vendor_id);
	} else {
		hdrlen = sizeof(struct ttls_avp);
	}

	avp->avp_code = host_to_be32(avp_code);
	avp->avp_length = host_to_be32((flags << 24) | (hdrlen + len));

	return avphdr + hdrlen;
}


static struct wpabuf * eap_ttls_avp_encapsulate(struct wpabuf *resp,
						u32 avp_code, int mandatory)
{
	struct wpabuf *avp;
	u8 *pos;

	avp = wpabuf_alloc(sizeof(struct ttls_avp) + wpabuf_len(resp) + 4);
	if (avp == NULL) {
		wpabuf_free(resp);
		return NULL;
	}

	pos = eap_ttls_avp_hdr(wpabuf_mhead(avp), avp_code, 0, mandatory,
			       wpabuf_len(resp));
	os_memcpy(pos, wpabuf_head(resp), wpabuf_len(resp));
	pos += wpabuf_len(resp);
	AVP_PAD((const u8 *) wpabuf_head(avp), pos);
	wpabuf_free(resp);
	wpabuf_put(avp, pos - (u8 *) wpabuf_head(avp));
	return avp;
}


struct eap_ttls_avp {
	 /* Note: eap is allocated memory; caller is responsible for freeing
	  * it. All the other pointers are pointing to the packet data, i.e.,
	  * they must not be freed separately. */
	u8 *eap;
	size_t eap_len;
	u8 *user_name;
	size_t user_name_len;
	u8 *user_password;
	size_t user_password_len;
	u8 *chap_challenge;
	size_t chap_challenge_len;
	u8 *chap_password;
	size_t chap_password_len;
	u8 *mschap_challenge;
	size_t mschap_challenge_len;
	u8 *mschap_response;
	size_t mschap_response_len;
	u8 *mschap2_response;
	size_t mschap2_response_len;
};


static int eap_ttls_avp_parse(u8 *buf, size_t len, struct eap_ttls_avp *parse)
{
	struct ttls_avp *avp;
	u8 *pos;
	int left;

	pos = buf;
	left = len;
	os_memset(parse, 0, sizeof(*parse));

	while (left > 0) {
		u32 avp_code, avp_length, vendor_id = 0;
		u8 avp_flags, *dpos;
		size_t pad, dlen;
		avp = (struct ttls_avp *) pos;
		avp_code = be_to_host32(avp->avp_code);
		avp_length = be_to_host32(avp->avp_length);
		avp_flags = (avp_length >> 24) & 0xff;
		avp_length &= 0xffffff;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: AVP: code=%d flags=0x%02x "
			   "length=%d", (int) avp_code, avp_flags,
			   (int) avp_length);
		if ((int) avp_length > left) {
			asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: AVP overflow "
				   "(len=%d, left=%d) - dropped",
				   (int) avp_length, left);
			goto fail;
		}
		if (avp_length < sizeof(*avp)) {
			asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: Invalid AVP length "
				   "%d", avp_length);
			goto fail;
		}
		dpos = (u8 *) (avp + 1);
		dlen = avp_length - sizeof(*avp);
		if (avp_flags & AVP_FLAGS_VENDOR) {
			if (dlen < 4) {
				asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: vendor AVP "
					   "underflow");
				goto fail;
			}
			vendor_id = be_to_host32(* (be32 *) dpos);
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: AVP vendor_id %d",
				   (int) vendor_id);
			dpos += 4;
			dlen -= 4;
		}

		wpa_hexdump(MSG_DEBUG, "EAP-TTLS: AVP data", dpos, dlen);

		if (vendor_id == 0 && avp_code == RADIUS_ATTR_EAP_MESSAGE) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: AVP - EAP Message");
			if (parse->eap == NULL) {
				parse->eap = os_zalloc(dlen);
				if (parse->eap == NULL) {
					asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: "
						   "failed to allocate memory "
						   "for Phase 2 EAP data");
					goto fail;
				}
				os_memcpy(parse->eap, dpos, dlen);
				parse->eap_len = dlen;
			} else {
				u8 *neweap = os_realloc(parse->eap,
							parse->eap_len + dlen);
				if (neweap == NULL) {
					asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: "
						   "failed to allocate memory "
						   "for Phase 2 EAP data");
					goto fail;
				}
				os_memcpy(neweap + parse->eap_len, dpos, dlen);
				parse->eap = neweap;
				parse->eap_len += dlen;
			}
		} else if (vendor_id == 0 &&
			   avp_code == RADIUS_ATTR_USER_NAME) {
			wpa_hexdump_ascii(MSG_DEBUG, "EAP-TTLS: User-Name",
					  dpos, dlen);
			parse->user_name = dpos;
			parse->user_name_len = dlen;
		} else if (vendor_id == 0 &&
			   avp_code == RADIUS_ATTR_USER_PASSWORD) {
			u8 *password = dpos;
			size_t password_len = dlen;
			while (password_len > 0 &&
			       password[password_len - 1] == '\0') {
				password_len--;
			}
			wpa_hexdump_ascii_key(MSG_DEBUG, "EAP-TTLS: "
					      "User-Password (PAP)",
					      password, password_len);
			parse->user_password = password;
			parse->user_password_len = password_len;
		} else if (vendor_id == 0 &&
			   avp_code == RADIUS_ATTR_CHAP_CHALLENGE) {
			wpa_hexdump(MSG_DEBUG,
				    "EAP-TTLS: CHAP-Challenge (CHAP)",
				    dpos, dlen);
			parse->chap_challenge = dpos;
			parse->chap_challenge_len = dlen;
		} else if (vendor_id == 0 &&
			   avp_code == RADIUS_ATTR_CHAP_PASSWORD) {
			wpa_hexdump(MSG_DEBUG,
				    "EAP-TTLS: CHAP-Password (CHAP)",
				    dpos, dlen);
			parse->chap_password = dpos;
			parse->chap_password_len = dlen;
		} else if (vendor_id == RADIUS_VENDOR_ID_MICROSOFT &&
			   avp_code == RADIUS_ATTR_MS_CHAP_CHALLENGE) {
			wpa_hexdump(MSG_DEBUG,
				    "EAP-TTLS: MS-CHAP-Challenge",
				    dpos, dlen);
			parse->mschap_challenge = dpos;
			parse->mschap_challenge_len = dlen;
		} else if (vendor_id == RADIUS_VENDOR_ID_MICROSOFT &&
			   avp_code == RADIUS_ATTR_MS_CHAP_RESPONSE) {
			wpa_hexdump(MSG_DEBUG,
				    "EAP-TTLS: MS-CHAP-Response (MSCHAP)",
				    dpos, dlen);
			parse->mschap_response = dpos;
			parse->mschap_response_len = dlen;
		} else if (vendor_id == RADIUS_VENDOR_ID_MICROSOFT &&
			   avp_code == RADIUS_ATTR_MS_CHAP2_RESPONSE) {
			wpa_hexdump(MSG_DEBUG,
				    "EAP-TTLS: MS-CHAP2-Response (MSCHAPV2)",
				    dpos, dlen);
			parse->mschap2_response = dpos;
			parse->mschap2_response_len = dlen;
		} else if (avp_flags & AVP_FLAGS_MANDATORY) {
			asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: Unsupported "
				   "mandatory AVP code %d vendor_id %d - "
				   "dropped", (int) avp_code, (int) vendor_id);
			goto fail;
		} else {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Ignoring unsupported "
				   "AVP code %d vendor_id %d",
				   (int) avp_code, (int) vendor_id);
		}

		pad = (4 - (avp_length & 3)) & 3;
		pos += avp_length + pad;
		left -= avp_length + pad;
	}

	return 0;

fail:
	os_free(parse->eap);
	parse->eap = NULL;
	return -1;
}


static u8 * eap_ttls_implicit_challenge(struct eap_sm *sm,
					struct eap_ttls_data *data, size_t len)
{
	struct tls_keys keys;
	u8 *challenge, *rnd;

	if (data->ttls_version == 0) {
		return eap_server_tls_derive_key(sm, &data->ssl,
						 "ttls challenge", len);
	}

	os_memset(&keys, 0, sizeof(keys));
	if (tls_connection_get_keys(sm->ssl_ctx, data->ssl.conn, &keys) ||
	    keys.client_random == NULL || keys.server_random == NULL ||
	    keys.inner_secret == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Could not get inner secret, "
			   "client random, or server random to derive "
			   "implicit challenge");
		return NULL;
	}

	rnd = os_zalloc(keys.client_random_len + keys.server_random_len);
	challenge = os_zalloc(len);
	if (rnd == NULL || challenge == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: No memory for implicit "
			   "challenge derivation");
		os_free(rnd);
		os_free(challenge);
		return NULL;
	}
	os_memcpy(rnd, keys.server_random, keys.server_random_len);
	os_memcpy(rnd + keys.server_random_len, keys.client_random,
		  keys.client_random_len);

	if (tls_prf(keys.inner_secret, keys.inner_secret_len,
		    "inner application challenge", rnd,
		    keys.client_random_len + keys.server_random_len,
		    challenge, len)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to derive implicit "
			   "challenge");
		os_free(rnd);
		os_free(challenge);
		return NULL;
	}

	os_free(rnd);

	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS: Derived implicit challenge",
			challenge, len);

	return challenge;
}


static void * eap_ttls_init(struct eap_sm *sm)
{
	struct eap_ttls_data *data;

	data = os_zalloc(sizeof(*data));
	if (data == NULL)
		return NULL;
	data->ttls_version = EAP_TTLS_VERSION;
	data->force_version = -1;
	if (sm->user && sm->user->force_version >= 0) {
		data->force_version = sm->user->force_version;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: forcing version %d",
			   data->force_version);
		data->ttls_version = data->force_version;
	}
	data->state = START;

	if (!(tls_capabilities(sm->ssl_ctx) & TLS_CAPABILITY_IA) &&
	    data->ttls_version > 0) {
		if (data->force_version > 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Forced TTLSv%d and "
				   "TLS library does not support TLS/IA.",
				   data->force_version);
			eap_ttls_reset(sm, data);
			return NULL;
		}
		data->ttls_version = 0;
	}

	if (eap_server_tls_ssl_init(sm, &data->ssl, 0)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to initialize SSL.");
		eap_ttls_reset(sm, data);
		return NULL;
	}

	return data;
}


static void eap_ttls_reset(struct eap_sm *sm, void *priv)
{
	struct eap_ttls_data *data = priv;
	if (data == NULL)
		return;
	if (data->phase2_priv && data->phase2_method)
		data->phase2_method->reset(sm, data->phase2_priv);
	eap_server_tls_ssl_deinit(sm, &data->ssl);
	wpabuf_free(data->pending_phase2_eap_resp);
	os_free(data);
}


static struct wpabuf * eap_ttls_build_start(struct eap_sm *sm,
					    struct eap_ttls_data *data, u8 id)
{	
	struct wpabuf *req;

	req = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_TTLS, 1,
			    EAP_CODE_REQUEST, id);
	if (req == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "EAP-TTLS: Failed to allocate memory for"
			   " request");
		eap_ttls_state(data, FAILURE);
		return NULL;
	}

	wpabuf_put_u8(req, EAP_TLS_FLAGS_START | data->ttls_version);

	eap_ttls_state(data, PHASE1);

	return req;
}


static struct wpabuf * eap_ttls_build_req(struct eap_sm *sm,
					  struct eap_ttls_data *data, u8 id)
{
	int res;
	struct wpabuf *req;

	res = eap_server_tls_buildReq_helper(sm, &data->ssl, EAP_TYPE_TTLS,
					     data->ttls_version, id, &req);

	if (tls_connection_established(sm->ssl_ctx, data->ssl.conn)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Phase1 done, starting "
			   "Phase2");
		eap_ttls_state(data, PHASE2_START);
	}

	if (res == 1)
		return eap_server_tls_build_ack(id, EAP_TYPE_TTLS,
						data->ttls_version);
	return req;
}


static struct wpabuf * eap_ttls_encrypt(struct eap_sm *sm,
					struct eap_ttls_data *data,
					u8 id, u8 *plain, size_t plain_len)
{
	int res;
	struct wpabuf *buf;

	/* TODO: add support for fragmentation, if needed. This will need to
	 * add TLS Message Length field, if the frame is fragmented. */
	buf = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_TTLS,
			    1 + data->ssl.tls_out_limit,
			    EAP_CODE_REQUEST, id);
	if (buf == NULL)
		return NULL;

	wpabuf_put_u8(buf, data->ttls_version);

	res = tls_connection_encrypt(sm->ssl_ctx, data->ssl.conn,
				     plain, plain_len, wpabuf_put(buf, 0),
				     data->ssl.tls_out_limit);
	if (res < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to encrypt Phase 2 "
			   "data");
		wpabuf_free(buf);
		return NULL;
	}

	wpabuf_put(buf, res);
	eap_update_len(buf);

	return buf;
}


static struct wpabuf * eap_ttls_build_phase2_eap_req(
	struct eap_sm *sm, struct eap_ttls_data *data, u8 id)
{
	struct wpabuf *buf, *encr_req;
	u8 *req;
	size_t req_len;


	buf = data->phase2_method->buildReq(sm, data->phase2_priv, id);
	if (buf == NULL)
		return NULL;

	wpa_hexdump_buf_key(MSG_DEBUG,
			    "EAP-TTLS/EAP: Encapsulate Phase 2 data", buf);

	buf = eap_ttls_avp_encapsulate(buf, RADIUS_ATTR_EAP_MESSAGE, 1);
	if (buf == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Failed to encapsulate "
			   "packet");
		return NULL;
	}

	req = wpabuf_mhead(buf);
	req_len = wpabuf_len(buf);
	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS/EAP: Encrypt encapsulated Phase "
			"2 data", req, req_len);

	encr_req = eap_ttls_encrypt(sm, data, id, req, req_len);
	wpabuf_free(buf);

	return encr_req;
}


static struct wpabuf * eap_ttls_build_phase2_mschapv2(
	struct eap_sm *sm, struct eap_ttls_data *data, u8 id)
{
	struct wpabuf *encr_req;
	u8 *req, *pos, *end;
	int ret;
	size_t req_len;

	pos = req = os_zalloc(100);
	if (req == NULL)
		return NULL;
	end = req + 100;

	if (data->mschapv2_resp_ok) {
		pos = eap_ttls_avp_hdr(pos, RADIUS_ATTR_MS_CHAP2_SUCCESS,
				       RADIUS_VENDOR_ID_MICROSOFT, 1, 43);
		*pos++ = data->mschapv2_ident;
		ret = os_snprintf((char *) pos, end - pos, "S=");
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex_uppercase(
			(char *) pos, end - pos, data->mschapv2_auth_response,
			sizeof(data->mschapv2_auth_response));
	} else {
		pos = eap_ttls_avp_hdr(pos, RADIUS_ATTR_MS_CHAP_ERROR,
				       RADIUS_VENDOR_ID_MICROSOFT, 1, 6);
		os_memcpy(pos, "Failed", 6);
		pos += 6;
		AVP_PAD(req, pos);
	}

	req_len = pos - req;
	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Encrypting Phase 2 "
			"data", req, req_len);

	encr_req = eap_ttls_encrypt(sm, data, id, req, req_len);
	os_free(req);

	return encr_req;
}


static struct wpabuf * eap_ttls_build_phase_finished(
	struct eap_sm *sm, struct eap_ttls_data *data, u8 id, int final)
{
	int len;
	struct wpabuf *req;
	const int max_len = 300;

	len = 1 + max_len;
	req = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_TTLS, len,
			    EAP_CODE_REQUEST, id);
	if (req == NULL)
		return NULL;

	wpabuf_put_u8(req, data->ttls_version);

	len = tls_connection_ia_send_phase_finished(sm->ssl_ctx,
						    data->ssl.conn, final,
						    wpabuf_mhead(req),
						    max_len);
	if (len < 0) {
		wpabuf_free(req);
		return NULL;
	}
	wpabuf_put(req, len);
	eap_update_len(req);

	return req;
}


static struct wpabuf * eap_ttls_buildReq(struct eap_sm *sm, void *priv, u8 id)
{
	struct eap_ttls_data *data = priv;

	switch (data->state) {
	case START:
		return eap_ttls_build_start(sm, data, id);
	case PHASE1:
		return eap_ttls_build_req(sm, data, id);
	case PHASE2_METHOD:
		return eap_ttls_build_phase2_eap_req(sm, data, id);
	case PHASE2_MSCHAPV2_RESP:
		return eap_ttls_build_phase2_mschapv2(sm, data, id);
	case PHASE_FINISHED:
		return eap_ttls_build_phase_finished(sm, data, id, 1);
	default:
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: %s - unexpected state %d",
			   __func__, data->state);
		return NULL;
	}
}


static Boolean eap_ttls_check(struct eap_sm *sm, void *priv,
			      struct wpabuf *respData)
{
	const u8 *pos;
	size_t len;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_TTLS, respData, &len);
	if (pos == NULL || len < 1) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Invalid frame");
		return TRUE;
	}

	return FALSE;
}


static int eap_ttls_ia_permute_inner_secret(struct eap_sm *sm,
					    struct eap_ttls_data *data,
					    const u8 *key, size_t key_len)
{
	u8 *buf;
	size_t buf_len;
	int ret;

	if (key) {
		buf_len = 2 + key_len;
		buf = os_zalloc(buf_len);
		if (buf == NULL)
			return -1;
		WPA_PUT_BE16(buf, key_len);
		os_memcpy(buf + 2, key, key_len);
	} else {
		buf = NULL;
		buf_len = 0;
	}

	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS: Session keys for TLS/IA inner "
			"secret permutation", buf, buf_len);
	ret = tls_connection_ia_permute_inner_secret(sm->ssl_ctx,
						     data->ssl.conn,
						     buf, buf_len);
	os_free(buf);

	return ret;
}


static void eap_ttls_process_phase2_pap(struct eap_sm *sm,
					struct eap_ttls_data *data,
					const u8 *user_password,
					size_t user_password_len)
{
	if (!sm->user || !sm->user->password || sm->user->password_hash ||
	    !(sm->user->ttls_auth & EAP_TTLS_AUTH_PAP)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/PAP: No plaintext user "
			   "password configured");
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (sm->user->password_len != user_password_len ||
	    os_memcmp(sm->user->password, user_password, user_password_len) !=
	    0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/PAP: Invalid user password");
		eap_ttls_state(data, FAILURE);
		return;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/PAP: Correct user password");
	eap_ttls_state(data, data->ttls_version > 0 ? PHASE_FINISHED :
		       SUCCESS);
}


static void eap_ttls_process_phase2_chap(struct eap_sm *sm,
					 struct eap_ttls_data *data,
					 const u8 *challenge,
					 size_t challenge_len,
					 const u8 *password,
					 size_t password_len)
{
	u8 *chal, hash[CHAP_MD5_LEN];

	if (challenge == NULL || password == NULL ||
	    challenge_len != EAP_TTLS_CHAP_CHALLENGE_LEN ||
	    password_len != 1 + EAP_TTLS_CHAP_PASSWORD_LEN) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: Invalid CHAP attributes "
			   "(challenge len %lu password len %lu)",
			   (unsigned long) challenge_len,
			   (unsigned long) password_len);
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (!sm->user || !sm->user->password || sm->user->password_hash ||
	    !(sm->user->ttls_auth & EAP_TTLS_AUTH_CHAP)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: No plaintext user "
			   "password configured");
		eap_ttls_state(data, FAILURE);
		return;
	}

	chal = eap_ttls_implicit_challenge(sm, data,
					   EAP_TTLS_CHAP_CHALLENGE_LEN + 1);
	if (chal == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: Failed to generate "
			   "challenge from TLS data");
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (os_memcmp(challenge, chal, EAP_TTLS_CHAP_CHALLENGE_LEN) != 0 ||
	    password[0] != chal[EAP_TTLS_CHAP_CHALLENGE_LEN]) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: Challenge mismatch");
		os_free(chal);
		eap_ttls_state(data, FAILURE);
		return;
	}
	os_free(chal);

	/* MD5(Ident + Password + Challenge) */
	chap_md5(password[0], sm->user->password, sm->user->password_len,
		 challenge, challenge_len, hash);

	if (os_memcmp(hash, password + 1, EAP_TTLS_CHAP_PASSWORD_LEN) == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: Correct user password");
		eap_ttls_state(data, data->ttls_version > 0 ? PHASE_FINISHED :
			       SUCCESS);
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/CHAP: Invalid user password");
		eap_ttls_state(data, FAILURE);
	}
}


static void eap_ttls_process_phase2_mschap(struct eap_sm *sm,
					   struct eap_ttls_data *data,
					   u8 *challenge, size_t challenge_len,
					   u8 *response, size_t response_len)
{
	u8 *chal, nt_response[24];

	if (challenge == NULL || response == NULL ||
	    challenge_len != EAP_TTLS_MSCHAP_CHALLENGE_LEN ||
	    response_len != EAP_TTLS_MSCHAP_RESPONSE_LEN) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: Invalid MS-CHAP "
			   "attributes (challenge len %lu response len %lu)",
			   (unsigned long) challenge_len,
			   (unsigned long) response_len);
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (!sm->user || !sm->user->password ||
	    !(sm->user->ttls_auth & EAP_TTLS_AUTH_MSCHAP)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: No user password "
			   "configured");
		eap_ttls_state(data, FAILURE);
		return;
	}

	chal = eap_ttls_implicit_challenge(sm, data,
					   EAP_TTLS_MSCHAP_CHALLENGE_LEN + 1);
	if (chal == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: Failed to generate "
			   "challenge from TLS data");
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (os_memcmp(challenge, chal, EAP_TTLS_MSCHAP_CHALLENGE_LEN) != 0 ||
	    response[0] != chal[EAP_TTLS_MSCHAP_CHALLENGE_LEN]) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: Challenge mismatch");
		os_free(chal);
		eap_ttls_state(data, FAILURE);
		return;
	}
	os_free(chal);

	if (sm->user->password_hash)
		challenge_response(challenge, sm->user->password, nt_response);
	else
		nt_challenge_response(challenge, sm->user->password,
				      sm->user->password_len, nt_response);

	if (os_memcmp(nt_response, response + 2 + 24, 24) == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: Correct response");
		eap_ttls_state(data, data->ttls_version > 0 ? PHASE_FINISHED :
			       SUCCESS);
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAP: Invalid NT-Response");
		wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAP: Received",
			    response + 2 + 24, 24);
		wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAP: Expected",
			    nt_response, 24);
		eap_ttls_state(data, FAILURE);
	}
}


static void eap_ttls_process_phase2_mschapv2(struct eap_sm *sm,
					     struct eap_ttls_data *data,
					     u8 *challenge,
					     size_t challenge_len,
					     u8 *response, size_t response_len)
{
	u8 *chal, *username, nt_response[24], *rx_resp, *peer_challenge,
		*auth_challenge;
	size_t username_len, i;

	if (challenge == NULL || response == NULL ||
	    challenge_len != EAP_TTLS_MSCHAPV2_CHALLENGE_LEN ||
	    response_len != EAP_TTLS_MSCHAPV2_RESPONSE_LEN) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Invalid MS-CHAP2 "
			   "attributes (challenge len %lu response len %lu)",
			   (unsigned long) challenge_len,
			   (unsigned long) response_len);
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (!sm->user || !sm->user->password ||
	    !(sm->user->ttls_auth & EAP_TTLS_AUTH_MSCHAPV2)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: No user password "
			   "configured");
		eap_ttls_state(data, FAILURE);
		return;
	}

	/* MSCHAPv2 does not include optional domain name in the
	 * challenge-response calculation, so remove domain prefix
	 * (if present). */
	username = sm->identity;
	username_len = sm->identity_len;
	for (i = 0; i < username_len; i++) {
		if (username[i] == '\\') {
			username_len -= i + 1;
			username += i + 1;
			break;
		}
	}

	chal = eap_ttls_implicit_challenge(
		sm, data, EAP_TTLS_MSCHAPV2_CHALLENGE_LEN + 1);
	if (chal == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Failed to generate "
			   "challenge from TLS data");
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (os_memcmp(challenge, chal, EAP_TTLS_MSCHAPV2_CHALLENGE_LEN) != 0 ||
	    response[0] != chal[EAP_TTLS_MSCHAPV2_CHALLENGE_LEN]) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Challenge mismatch");
		os_free(chal);
		eap_ttls_state(data, FAILURE);
		return;
	}
	os_free(chal);

	auth_challenge = challenge;
	peer_challenge = response + 2;

	wpa_hexdump_ascii(MSG_MSGDUMP, "EAP-TTLS/MSCHAPV2: User",
			  username, username_len);
	wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAPV2: auth_challenge",
		    auth_challenge, EAP_TTLS_MSCHAPV2_CHALLENGE_LEN);
	wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAPV2: peer_challenge",
		    peer_challenge, EAP_TTLS_MSCHAPV2_CHALLENGE_LEN);

	if (sm->user->password_hash) {
		generate_nt_response_pwhash(auth_challenge, peer_challenge,
					    username, username_len,
					    sm->user->password,
					    nt_response);
	} else {
		generate_nt_response(auth_challenge, peer_challenge,
				     username, username_len,
				     sm->user->password,
				     sm->user->password_len,
				     nt_response);
	}

	rx_resp = response + 2 + EAP_TTLS_MSCHAPV2_CHALLENGE_LEN + 8;
	if (os_memcmp(nt_response, rx_resp, 24) == 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Correct "
			   "NT-Response");
		data->mschapv2_resp_ok = 1;
		if (data->ttls_version > 0) {
			const u8 *pw_hash;
			u8 pw_hash_buf[16], pw_hash_hash[16], master_key[16];
			u8 session_key[2 * MSCHAPV2_KEY_LEN];

			if (sm->user->password_hash)
				pw_hash = sm->user->password;
			else {
				nt_password_hash(sm->user->password,
						 sm->user->password_len,
						 pw_hash_buf);
				pw_hash = pw_hash_buf;
			}
			hash_nt_password_hash(pw_hash, pw_hash_hash);
			get_master_key(pw_hash_hash, nt_response, master_key);
			get_asymetric_start_key(master_key, session_key,
						MSCHAPV2_KEY_LEN, 0, 0);
			get_asymetric_start_key(master_key,
						session_key + MSCHAPV2_KEY_LEN,
						MSCHAPV2_KEY_LEN, 1, 0);
			eap_ttls_ia_permute_inner_secret(sm, data,
							 session_key,
							 sizeof(session_key));
		}

		if (sm->user->password_hash) {
			generate_authenticator_response_pwhash(
				sm->user->password,
				peer_challenge, auth_challenge,
				username, username_len, nt_response,
				data->mschapv2_auth_response);
		} else {
			generate_authenticator_response(
				sm->user->password, sm->user->password_len,
				peer_challenge, auth_challenge,
				username, username_len, nt_response,
				data->mschapv2_auth_response);
		}
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Invalid "
			   "NT-Response");
		wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAPV2: Received",
			    rx_resp, 24);
		wpa_hexdump(MSG_MSGDUMP, "EAP-TTLS/MSCHAPV2: Expected",
			    nt_response, 24);
		data->mschapv2_resp_ok = 0;
	}
	eap_ttls_state(data, PHASE2_MSCHAPV2_RESP);
	data->mschapv2_ident = response[0];
}


static int eap_ttls_phase2_eap_init(struct eap_sm *sm,
				    struct eap_ttls_data *data,
				    EapType eap_type)
{
	if (data->phase2_priv && data->phase2_method) {
		data->phase2_method->reset(sm, data->phase2_priv);
		data->phase2_method = NULL;
		data->phase2_priv = NULL;
	}
	data->phase2_method = eap_server_get_eap_method(EAP_VENDOR_IETF,
							eap_type);
	if (!data->phase2_method)
		return -1;

	sm->init_phase2 = 1;
	data->phase2_priv = data->phase2_method->init(sm);
	sm->init_phase2 = 0;
	return 0;
}


static void eap_ttls_process_phase2_eap_response(struct eap_sm *sm,
						 struct eap_ttls_data *data,
						 u8 *in_data, size_t in_len)
{
	u8 next_type = EAP_TYPE_NONE;
	struct eap_hdr *hdr;
	u8 *pos;
	size_t left;
	struct wpabuf buf;
	const struct eap_method *m = data->phase2_method;
	void *priv = data->phase2_priv;

	if (priv == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: %s - Phase2 not "
			   "initialized?!", __func__);
		return;
	}

	hdr = (struct eap_hdr *) in_data;
	pos = (u8 *) (hdr + 1);

	if (in_len > sizeof(*hdr) && *pos == EAP_TYPE_NAK) {
		left = in_len - sizeof(*hdr);
		wpa_hexdump(MSG_DEBUG, "EAP-TTLS/EAP: Phase2 type Nak'ed; "
			    "allowed types", pos + 1, left - 1);
		eap_sm_process_nak(sm, pos + 1, left - 1);
		if (sm->user && sm->user_eap_method_index < EAP_MAX_METHODS &&
		    sm->user->methods[sm->user_eap_method_index].method !=
		    EAP_TYPE_NONE) {
			next_type = sm->user->methods[
				sm->user_eap_method_index++].method;
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: try EAP type %d",
				   next_type);
			eap_ttls_phase2_eap_init(sm, data, next_type);
		} else {
			eap_ttls_state(data, FAILURE);
		}
		return;
	}

	wpabuf_set(&buf, in_data, in_len);

	if (m->check(sm, priv, &buf)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Phase2 check() asked to "
			   "ignore the packet");
		return;
	}

	m->process(sm, priv, &buf);

	if (sm->method_pending == METHOD_PENDING_WAIT) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Phase2 method is in "
			   "pending wait state - save decrypted response");
		wpabuf_free(data->pending_phase2_eap_resp);
		data->pending_phase2_eap_resp = wpabuf_dup(&buf);
	}

	if (!m->isDone(sm, priv))
		return;

	if (!m->isSuccess(sm, priv)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Phase2 method failed");
		eap_ttls_state(data, FAILURE);
		return;
	}

	switch (data->state) {
	case PHASE2_START:
		if (eap_user_get(sm, sm->identity, sm->identity_len, 1) != 0) {
			wpa_hexdump_ascii(MSG_DEBUG, "EAP_TTLS: Phase2 "
					  "Identity not found in the user "
					  "database",
					  sm->identity, sm->identity_len);
			eap_ttls_state(data, FAILURE);
			break;
		}

		eap_ttls_state(data, PHASE2_METHOD);
		next_type = sm->user->methods[0].method;
		sm->user_eap_method_index = 1;
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: try EAP type %d", next_type);
		break;
	case PHASE2_METHOD:
		if (data->ttls_version > 0) {
			if (m->getKey) {
				u8 *key;
				size_t key_len;
				key = m->getKey(sm, priv, &key_len);
				eap_ttls_ia_permute_inner_secret(sm, data,
								 key, key_len);
			}
			eap_ttls_state(data, PHASE_FINISHED);
		} else
			eap_ttls_state(data, SUCCESS);
		break;
	case FAILURE:
		break;
	default:
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: %s - unexpected state %d",
			   __func__, data->state);
		break;
	}

	eap_ttls_phase2_eap_init(sm, data, next_type);
}


static void eap_ttls_process_phase2_eap(struct eap_sm *sm,
					struct eap_ttls_data *data,
					const u8 *eap, size_t eap_len)
{
	struct eap_hdr *hdr;
	size_t len;

	if (data->state == PHASE2_START) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: initializing Phase 2");
		if (eap_ttls_phase2_eap_init(sm, data, EAP_TYPE_IDENTITY) < 0)
		{
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: failed to "
				   "initialize EAP-Identity");
			return;
		}
	}

	if (eap_len < sizeof(*hdr)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: too short Phase 2 EAP "
			   "packet (len=%lu)", (unsigned long) eap_len);
		return;
	}

	hdr = (struct eap_hdr *) eap;
	len = be_to_host16(hdr->length);
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: received Phase 2 EAP: code=%d "
		   "identifier=%d length=%lu", hdr->code, hdr->identifier,
		   (unsigned long) len);
	if (len > eap_len) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Length mismatch in Phase 2"
			   " EAP frame (hdr len=%lu, data len in AVP=%lu)",
			   (unsigned long) len, (unsigned long) eap_len);
		return;
	}

	switch (hdr->code) {
	case EAP_CODE_RESPONSE:
		eap_ttls_process_phase2_eap_response(sm, data, (u8 *) hdr,
						     len);
		break;
	default:
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/EAP: Unexpected code=%d in "
			   "Phase 2 EAP header", hdr->code);
		break;
	}
}


static void eap_ttls_process_phase2(struct eap_sm *sm,
				    struct eap_ttls_data *data,
				    u8 *in_data, size_t in_len)
{
	u8 *in_decrypted;
	int len_decrypted, res;
	struct eap_ttls_avp parse;
	size_t buf_len;

	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: received %lu bytes encrypted data for"
		   " Phase 2", (unsigned long) in_len);

	if (data->pending_phase2_eap_resp) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Pending Phase 2 EAP response "
			   "- skip decryption and use old data");
		eap_ttls_process_phase2_eap(
			sm, data, wpabuf_head(data->pending_phase2_eap_resp),
			wpabuf_len(data->pending_phase2_eap_resp));
		wpabuf_free(data->pending_phase2_eap_resp);
		data->pending_phase2_eap_resp = NULL;
		return;
	}

	res = eap_server_tls_data_reassemble(sm, &data->ssl, &in_data,
					     &in_len);
	if (res < 0 || res == 1)
		return;

	buf_len = in_len;
	if (data->ssl.tls_in_total > buf_len)
		buf_len = data->ssl.tls_in_total;
	in_decrypted = os_zalloc(buf_len);
	if (in_decrypted == NULL) {
		os_free(data->ssl.tls_in);
		data->ssl.tls_in = NULL;
		data->ssl.tls_in_len = 0;
		asd_printf(ASD_DEFAULT,MSG_WARNING, "EAP-TTLS: failed to allocate memory "
			   "for decryption");
		return;
	}

	len_decrypted = tls_connection_decrypt(sm->ssl_ctx, data->ssl.conn,
					       in_data, in_len,
					       in_decrypted, buf_len);
	os_free(data->ssl.tls_in);
	data->ssl.tls_in = NULL;
	data->ssl.tls_in_len = 0;
	if (len_decrypted < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to decrypt Phase 2 "
			   "data");
		os_free(in_decrypted);
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (data->state == PHASE_FINISHED) {
		if (len_decrypted == 0 &&
		    tls_connection_ia_final_phase_finished(sm->ssl_ctx,
							   data->ssl.conn)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: FinalPhaseFinished "
				   "received");
			eap_ttls_state(data, SUCCESS);
		} else {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Did not receive valid "
				   "FinalPhaseFinished");
			eap_ttls_state(data, FAILURE);
		}

		os_free(in_decrypted);
		return;
	}

	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS: Decrypted Phase 2 EAP",
			in_decrypted, len_decrypted);

	if (eap_ttls_avp_parse(in_decrypted, len_decrypted, &parse) < 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to parse AVPs");
		os_free(in_decrypted);
		eap_ttls_state(data, FAILURE);
		return;
	}

	if (parse.user_name) {
		os_free(sm->identity);
		sm->identity = os_zalloc(parse.user_name_len);
		if (sm->identity) {
			os_memcpy(sm->identity, parse.user_name,
				  parse.user_name_len);
			sm->identity_len = parse.user_name_len;
		}
		if (eap_user_get(sm, parse.user_name, parse.user_name_len, 1)
		    != 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Phase2 Identity not "
				   "found in the user database");
			eap_ttls_state(data, FAILURE);
			goto done;
		}
	}

	if (parse.eap) {
		eap_ttls_process_phase2_eap(sm, data, parse.eap,
					    parse.eap_len);
	} else if (parse.user_password) {
		eap_ttls_process_phase2_pap(sm, data, parse.user_password,
					    parse.user_password_len);
	} else if (parse.chap_password) {
		eap_ttls_process_phase2_chap(sm, data,
					     parse.chap_challenge,
					     parse.chap_challenge_len,
					     parse.chap_password,
					     parse.chap_password_len);
	} else if (parse.mschap_response) {
		eap_ttls_process_phase2_mschap(sm, data,
					       parse.mschap_challenge,
					       parse.mschap_challenge_len,
					       parse.mschap_response,
					       parse.mschap_response_len);
	} else if (parse.mschap2_response) {
		eap_ttls_process_phase2_mschapv2(sm, data,
						 parse.mschap_challenge,
						 parse.mschap_challenge_len,
						 parse.mschap2_response,
						 parse.mschap2_response_len);
	}

done:
	os_free(in_decrypted);
	os_free(parse.eap);
}


static void eap_ttls_process(struct eap_sm *sm, void *priv,
			     struct wpabuf *respData)
{
	struct eap_ttls_data *data = priv;
	const u8 *pos;
	u8 flags;
	size_t left;
	unsigned int tls_msg_len;
	int peer_version;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_TTLS, respData,
			       &left);
	if (pos == NULL || left < 1)
		return;
	flags = *pos++;
	left--;
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Received packet(len=%lu) - "
		   "Flags 0x%02x", (unsigned long) wpabuf_len(respData),
		   flags);
	peer_version = flags & EAP_PEAP_VERSION_MASK;
	if (peer_version < data->ttls_version) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: peer ver=%d, own ver=%d; "
			   "use version %d",
			   peer_version, data->ttls_version, peer_version);
		data->ttls_version = peer_version;
	}

	if (data->ttls_version > 0 && !data->tls_ia_configured) {
		if (tls_connection_set_ia(sm->ssl_ctx, data->ssl.conn, 1)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to enable "
				   "TLS/IA");
			eap_ttls_state(data, FAILURE);
			return;
		}
		data->tls_ia_configured = 1;
	}

	if (flags & EAP_TLS_FLAGS_LENGTH_INCLUDED) {
		if (left < 4) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Short frame with TLS "
				   "length");
			eap_ttls_state(data, FAILURE);
			return;
		}
		tls_msg_len = WPA_GET_BE32(pos);
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: TLS Message Length: %d",
			   tls_msg_len);
		if (data->ssl.tls_in_left == 0) {
			data->ssl.tls_in_total = tls_msg_len;
			data->ssl.tls_in_left = tls_msg_len;
			os_free(data->ssl.tls_in);
			data->ssl.tls_in = NULL;
			data->ssl.tls_in_len = 0;
		}
		pos += 4;
		left -= 4;
	}

	switch (data->state) {
	case PHASE1:
		if (eap_server_tls_process_helper(sm, &data->ssl, pos, left) <
		    0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: TLS processing "
				   "failed");
			eap_ttls_state(data, FAILURE);
		}
		break;
	case PHASE2_START:
	case PHASE2_METHOD:
	case PHASE_FINISHED:
		/* FIX: get rid of const->non-const typecast */
		eap_ttls_process_phase2(sm, data, (u8 *) pos, left);
		break;
	case PHASE2_MSCHAPV2_RESP:
		if (data->mschapv2_resp_ok && left == 0) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Peer "
				   "acknowledged response");
			eap_ttls_state(data, data->ttls_version > 0 ?
				       PHASE_FINISHED : SUCCESS);
		} else if (!data->mschapv2_resp_ok) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Peer "
				   "acknowledged error");
			eap_ttls_state(data, FAILURE);
		} else {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS/MSCHAPV2: Unexpected "
				   "frame from peer (payload len %lu, "
				   "expected empty frame)",
				   (unsigned long) left);
			eap_ttls_state(data, FAILURE);
		}
		break;
	default:
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Unexpected state %d in %s",
			   data->state, __func__);
		break;
	}

	if (tls_connection_get_write_alerts(sm->ssl_ctx, data->ssl.conn) > 1) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Locally detected fatal error "
			   "in TLS processing");
		eap_ttls_state(data, FAILURE);
	}
}


static Boolean eap_ttls_isDone(struct eap_sm *sm, void *priv)
{
	struct eap_ttls_data *data = priv;
	return data->state == SUCCESS || data->state == FAILURE;
}


static u8 * eap_ttls_v1_derive_key(struct eap_sm *sm,
				   struct eap_ttls_data *data)
{
	struct tls_keys keys;
	u8 *rnd, *key;

	os_memset(&keys, 0, sizeof(keys));
	if (tls_connection_get_keys(sm->ssl_ctx, data->ssl.conn, &keys) ||
	    keys.client_random == NULL || keys.server_random == NULL ||
	    keys.inner_secret == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Could not get inner secret, "
			   "client random, or server random to derive keying "
			   "material");
		return NULL;
	}

	rnd = os_zalloc(keys.client_random_len + keys.server_random_len);
	key = os_zalloc(EAP_TLS_KEY_LEN);
	if (rnd == NULL || key == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: No memory for key derivation");
		os_free(rnd);
		os_free(key);
		return NULL;
	}
	os_memcpy(rnd, keys.client_random, keys.client_random_len);
	os_memcpy(rnd + keys.client_random_len, keys.server_random,
		  keys.server_random_len);

	if (tls_prf(keys.inner_secret, keys.inner_secret_len,
		    "ttls v1 keying material", rnd, keys.client_random_len +
		    keys.server_random_len, key, EAP_TLS_KEY_LEN)) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to derive key");
		os_free(rnd);
		os_free(key);
		return NULL;
	}

	wpa_hexdump(MSG_DEBUG, "EAP-TTLS: client/server random",
		    rnd, keys.client_random_len + keys.server_random_len);
	wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS: TLS/IA inner secret",
			keys.inner_secret, keys.inner_secret_len);

	os_free(rnd);

	return key;
}


static u8 * eap_ttls_getKey(struct eap_sm *sm, void *priv, size_t *len)
{
	struct eap_ttls_data *data = priv;
	u8 *eapKeyData;

	if (data->state != SUCCESS)
		return NULL;

	if (data->ttls_version == 0) {
		eapKeyData = eap_server_tls_derive_key(sm, &data->ssl,
						       "ttls keying material",
						       EAP_TLS_KEY_LEN);
	} else {
		eapKeyData = eap_ttls_v1_derive_key(sm, data);
	}

	if (eapKeyData) {
		*len = EAP_TLS_KEY_LEN;
		wpa_hexdump_key(MSG_DEBUG, "EAP-TTLS: Derived key",
				eapKeyData, EAP_TLS_KEY_LEN);
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-TTLS: Failed to derive key");
	}

	return eapKeyData;
}


static Boolean eap_ttls_isSuccess(struct eap_sm *sm, void *priv)
{
	struct eap_ttls_data *data = priv;
	return data->state == SUCCESS;
}


int eap_server_ttls_register(void)
{
	struct eap_method *eap;
	int ret;

	eap = eap_server_method_alloc(EAP_SERVER_METHOD_INTERFACE_VERSION,
				      EAP_VENDOR_IETF, EAP_TYPE_TTLS, "TTLS");
	if (eap == NULL)
		return -1;

	eap->init = eap_ttls_init;
	eap->reset = eap_ttls_reset;
	eap->buildReq = eap_ttls_buildReq;
	eap->check = eap_ttls_check;
	eap->process = eap_ttls_process;
	eap->isDone = eap_ttls_isDone;
	eap->getKey = eap_ttls_getKey;
	eap->isSuccess = eap_ttls_isSuccess;

	ret = eap_server_method_register(eap);
	if (ret)
		eap_server_method_free(eap);
	return ret;
}
