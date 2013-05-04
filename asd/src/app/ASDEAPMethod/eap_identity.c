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
* AsdEapIdentity.c
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
#include "eap_i.h"
#include "wcpss/waw.h"
#include "wcpss/asd/asd.h"



struct eap_identity_data {
	enum { CONTINUE, SUCCESS, FAILURE } state;
	int pick_up;
};


static void * eap_identity_init(struct eap_sm *sm)
{
	struct eap_identity_data *data;

	data = os_zalloc(sizeof(*data));
	if (data == NULL)
		return NULL;
	data->state = CONTINUE;

	return data;
}


static void * eap_identity_initPickUp(struct eap_sm *sm)
{
	struct eap_identity_data *data;
	data = eap_identity_init(sm);
	if (data) {
		data->pick_up = 1;
	}
	return data;
}


static void eap_identity_reset(struct eap_sm *sm, void *priv)
{
	struct eap_identity_data *data = priv;
	os_free(data);
}


static struct wpabuf * eap_identity_buildReq(struct eap_sm *sm, void *priv,
					     u8 id)
{
	struct eap_identity_data *data = priv;
	struct wpabuf *req;
	const char *req_data;
	size_t req_data_len;

	if (sm->eapol_cb->get_eap_req_id_text) {
		req_data = sm->eapol_cb->get_eap_req_id_text(sm->eapol_ctx,
							     &req_data_len);
	} else {
		req_data = NULL;
		req_data_len = 0;
	}
	req = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_IDENTITY, req_data_len,
			    EAP_CODE_REQUEST, id);
	if (req == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "EAP-Identity: Failed to allocate "
			   "memory for request");
		data->state = FAILURE;
		return NULL;
	}

	wpabuf_put_data(req, req_data, req_data_len);

	return req;
}


static Boolean eap_identity_check(struct eap_sm *sm, void *priv,
				  struct wpabuf *respData)
{
	const u8 *pos;
	size_t len;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_IDENTITY,
			       respData, &len);
	if (pos == NULL) {
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-Identity: Invalid frame");
		return TRUE;
	}

	return FALSE;
}


static void eap_identity_process(struct eap_sm *sm, void *priv,
				 struct wpabuf *respData)
{
	struct eap_identity_data *data = priv;
	const u8 *pos;
	size_t len;

	if (data->pick_up) {
		if (eap_identity_check(sm, data, respData)) {
			asd_printf(ASD_DEFAULT,MSG_DEBUG, "EAP-Identity: failed to pick "
				   "up already started negotiation");
			data->state = FAILURE;
			return;
		}
		data->pick_up = 0;
	}

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_IDENTITY,
			       respData, &len);
	if (pos == NULL)
		return; /* Should not happen - frame already validated */

	wpa_hexdump_ascii(MSG_DEBUG, "EAP-Identity: Peer identity", pos, len);
	os_free(sm->identity);
	sm->identity = os_zalloc(len ? len : 1);
	if (sm->identity == NULL) {
		data->state = FAILURE;
	} else {
		os_memcpy(sm->identity, pos, len);
		sm->identity_len = len;
		data->state = SUCCESS;
	}
}


static Boolean eap_identity_isDone(struct eap_sm *sm, void *priv)
{
	struct eap_identity_data *data = priv;
	return data->state != CONTINUE;
}


static Boolean eap_identity_isSuccess(struct eap_sm *sm, void *priv)
{
	struct eap_identity_data *data = priv;
	return data->state == SUCCESS;
}


int eap_server_identity_register(void)
{
	struct eap_method *eap;
	int ret;

	eap = eap_server_method_alloc(EAP_SERVER_METHOD_INTERFACE_VERSION,
				      EAP_VENDOR_IETF, EAP_TYPE_IDENTITY,
				      "Identity");
	if (eap == NULL)
		return -1;

	eap->init = eap_identity_init;
	eap->initPickUp = eap_identity_initPickUp;
	eap->reset = eap_identity_reset;
	eap->buildReq = eap_identity_buildReq;
	eap->check = eap_identity_check;
	eap->process = eap_identity_process;
	eap->isDone = eap_identity_isDone;
	eap->isSuccess = eap_identity_isSuccess;

	ret = eap_server_method_register(eap);
	if (ret)
		eap_server_method_free(eap);
	return ret;
}
