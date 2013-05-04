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
* AsdEapGtc.c
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
#include "eap_i.h"


struct eap_gtc_data {
	enum { CONTINUE, SUCCESS, FAILURE } state;
	int prefix;
};


static void * eap_gtc_init(struct eap_sm *sm)
{
	struct eap_gtc_data *data;

	data = os_zalloc(sizeof(*data));
	if (data == NULL)
		return NULL;
	data->state = CONTINUE;

#ifdef EAP_FAST
	if (sm->m && sm->m->vendor == EAP_VENDOR_IETF &&
	    sm->m->method == EAP_TYPE_FAST) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: EAP-FAST tunnel - use prefix "
			   "with challenge/response");
		data->prefix = 1;
	}
#endif /* EAP_FAST */

	return data;
}


static void eap_gtc_reset(struct eap_sm *sm, void *priv)
{
	struct eap_gtc_data *data = priv;
	os_free(data);
}


static struct wpabuf * eap_gtc_buildReq(struct eap_sm *sm, void *priv, u8 id)
{
	struct eap_gtc_data *data = priv;
	struct wpabuf *req;
	char *msg;
	size_t msg_len;

	msg = data->prefix ? "CHALLENGE=Password" : "Password";

	msg_len = os_strlen(msg);
	req = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_GTC, msg_len,
			    EAP_CODE_REQUEST, id);
	if (req == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "EAP-GTC: Failed to allocate memory for "
			   "request");
		data->state = FAILURE;
		return NULL;
	}

	wpabuf_put_data(req, msg, msg_len);

	data->state = CONTINUE;

	return req;
}


static Boolean eap_gtc_check(struct eap_sm *sm, void *priv,
			     struct wpabuf *respData)
{
	const u8 *pos;
	size_t len;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_GTC, respData, &len);
	if (pos == NULL || len < 1) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: Invalid frame");
		return TRUE;
	}

	return FALSE;
}


static void eap_gtc_process(struct eap_sm *sm, void *priv,
			    struct wpabuf *respData)
{
	struct eap_gtc_data *data = priv;
	const u8 *pos;
	size_t rlen;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_GTC, respData, &rlen);
	if (pos == NULL || rlen < 1)
		return; /* Should not happen - frame already validated */

	wpa_hexdump_ascii_key(MSG_MSGDUMP, "EAP-GTC: Response", pos, rlen);

#ifdef EAP_FAST
	if (data->prefix) {
		const u8 *pos2, *end;
		/* "RESPONSE=<user>\0<password>" */
		if (rlen < 10) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: Too short response "
				   "for EAP-FAST prefix");
			data->state = FAILURE;
			return;
		}

		end = pos + rlen;
		pos += 9;
		pos2 = pos;
		while (pos2 < end && *pos2)
			pos2++;
		if (pos2 == end) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: No password in "
				   "response to EAP-FAST prefix");
			data->state = FAILURE;
			return;
		}

		wpa_hexdump_ascii(MSG_MSGDUMP, "EAP-GTC: Response user",
				  pos, pos2 - pos);
		os_free(sm->identity);
		sm->identity_len = pos2 - pos;
		sm->identity = os_zalloc(sm->identity_len);
		if (sm->identity == NULL) {
			data->state = FAILURE;
			return;
		}
		os_memcpy(sm->identity, pos, sm->identity_len);

		if (eap_user_get(sm, sm->identity, sm->identity_len, 1) != 0) {
			wpa_hexdump_ascii(MSG_DEBUG, "EAP-GTC: Phase2 "
					  "Identity not found in the user "
					  "database",
					  sm->identity, sm->identity_len);
			data->state = FAILURE;
			return;
		}

		pos = pos2 + 1;
		rlen = end - pos;
		wpa_hexdump_ascii_key(MSG_MSGDUMP,
				      "EAP-GTC: Response password",
				      pos, rlen);
	}
#endif /* EAP_FAST */

	if (sm->user == NULL || sm->user->password == NULL ||
	    sm->user->password_hash) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: Plaintext password not "
			   "configured");
		data->state = FAILURE;
		return;
	}

	if (rlen != sm->user->password_len ||
	    os_memcmp(pos, sm->user->password, rlen) != 0) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: Done - Failure");
		data->state = FAILURE;
	} else {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-GTC: Done - Success");
		data->state = SUCCESS;
	}
}


static Boolean eap_gtc_isDone(struct eap_sm *sm, void *priv)
{
	struct eap_gtc_data *data = priv;
	return data->state != CONTINUE;
}


static Boolean eap_gtc_isSuccess(struct eap_sm *sm, void *priv)
{
	struct eap_gtc_data *data = priv;
	return data->state == SUCCESS;
}


int eap_server_gtc_register(void)
{
	struct eap_method *eap;
	int ret;

	eap = eap_server_method_alloc(EAP_SERVER_METHOD_INTERFACE_VERSION,
				      EAP_VENDOR_IETF, EAP_TYPE_GTC, "GTC");
	if (eap == NULL)
		return -1;

	eap->init = eap_gtc_init;
	eap->reset = eap_gtc_reset;
	eap->buildReq = eap_gtc_buildReq;
	eap->check = eap_gtc_check;
	eap->process = eap_gtc_process;
	eap->isDone = eap_gtc_isDone;
	eap->isSuccess = eap_gtc_isSuccess;

	ret = eap_server_method_register(eap);
	if (ret)
		eap_server_method_free(eap);
	return ret;
}
