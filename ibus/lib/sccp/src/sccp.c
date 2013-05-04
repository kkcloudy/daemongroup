/*
 * SCCP management code
 *
 * (C) 2009, 2010 by Holger Hans Peter Freyther <zecke@selfish.org>
 * (C) 2009, 2010 by On-Waves
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
 
#include <string.h>

#include <osmocore/msgb.h>
#include <osmocore/talloc.h>
#include <osmocore/logging.h>

#include <sccp/sccp.h>
#include "m3ua_defines.h"
#include "m3ua_types.h"
#include "m3ua_api.h"
//#include "ranap/ran_appl.h"
#include "trans.h"
#include "iuh/Iuh.h"
#include "asn_codecs.h"
#include "constr_TYPE.h"
#include "RANAP-PDU.h"
#include "RANAP-ProtocolIE-Container.h"
#include "RelocationRequest.h"
#include "Paging.h"
#include "PagingAreaID.h"
#include "PagingCause.h"
#include "RANAP-CN-DomainIndicator.h"
#include "PermanentNAS-UE-ID.h"
#include "TemporaryUE-ID.h"
#include "NonSearchingIndication.h"
#include "GlobalCN-ID.h"
#include "DRX-CycleLengthCoefficient.h"
#include "RAB-AssignmentRequest.h"
#include "RAB-ID.h"
#include "RAB-Parameters.h"
#include "NAS-SynchronisationIndicator.h"
#include "UserPlaneMode.h"
#include "TransportLayerInformation.h"
#include "RAB-SetupOrModifyList.h"
#include "RAB-SetupOrModifiedList.h"
#include "RAB-SetupOrModifiedItem.h"
#include "RAB-SetupOrModifyItemFirst.h"
#include "RAB-SetupOrModifyItemSecond.h"
#include "RAB-ReleaseRequest.h"
#include "RAB-ReleaseList.h"
#include "RAB-ReleaseItem.h"


// Unassigned debug area
static int DSCCP = 0;
#define UNIX_PATH_MAX    108

#define SOCKPATH_IU "/var/run/femto/iu"
#define SOCKPATH_IUH "/var/run/femto/iuh"
int	iuh_fd;
int IuhTipcsock;
struct sockaddr_tipc Iu2Iuh_addr;
int gNi;
int gCnDomain;
static unsigned int gFlag = 0x000000;

/*****************add for sigtran2udp use one connect*****************/
struct sccp_source_reference global_reference;
/*********************************************************************/

/*****************add for ranap decode use one connect*****************/
extern asn_enc_rval_t
uper_encode(asn_TYPE_descriptor_t *td, void *sptr, asn_app_consume_bytes_f *cb, void *app_key);

#define IS_CONNECTIONLESS(id) ((id_Reset == id || \
									id_Paging == id || \
									id_OverloadControl == id || \
									id_ErrorIndication == id || \
									id_ResetResource == id) ? 1 : 0)

/*success return 0,else return 1, present(Macro in RANAP-PDU.h ): 1-4 */
unsigned int 
ranap_get_msg_type
(
	unsigned char *decoded_data, 
	unsigned int *present, 
	unsigned short *procedure_code
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if(!decoded_data || !present || !procedure_code){
			return 1;
	}
	RANAP_PDU_t * p_ranap_pdu = (RANAP_PDU_t *)decoded_data;
	
	*present = p_ranap_pdu->present;
    switch (p_ranap_pdu->present) {
	    case RANAP_PDU_PR_initiatingMessage:
	        *procedure_code = p_ranap_pdu->choice.initiatingMessage.procedureCode;
	        break;
	    case RANAP_PDU_PR_successfulOutcome:
	        *procedure_code = p_ranap_pdu->choice.successfulOutcome.procedureCode;
	        break;
	    case RANAP_PDU_PR_unsuccessfulOutcome:
	        *procedure_code = p_ranap_pdu->choice.unsuccessfulOutcome.procedureCode;
	        break;
	    case RANAP_PDU_PR_outcome:
	        *procedure_code = p_ranap_pdu->choice.outcome.procedureCode;
	        break;
	    default:
    		*present = 0;
    		*procedure_code = 0;
			return 1;
			break;	
    }
	
    return 0;
}

unsigned int 
ranap_decode_msg_type
(
	unsigned char *buf, 
	unsigned int buf_len,
	unsigned int *present, /* out put */
	unsigned short *procedure_code, /* out put */
	RANAP_PDU_t *ranap_pdu
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	asn_dec_rval_t rval;
	asn_codec_ctx_t * opt_codec_ctx = NULL;
	asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
	void * structure = NULL;
	const void * buffer = buf;
	size_t size = (size_t)buf_len;
	int skip_bits = 0;
	int unused_bits = 0;
	asn_TYPE_descriptor_t *pdu_type = &asn_DEF_RANAP_PDU;

	rval = uper_decode(opt_codec_ctx, (asn_TYPE_descriptor_t *)pdu_type, (void **)&structure, buffer, size, skip_bits, unused_bits);

    memcpy(ranap_pdu, structure, sizeof(RANAP_PDU_t));
    
	switch(rval.code) {
		case RC_OK:
			return ranap_get_msg_type((unsigned char *)structure, present, procedure_code);
			break;
		case RC_FAIL:
		    iu_log_debug("Error : uper_decode ranap message error!\n");
			/* uper_decode() returns bits! */
			/* Extra bits */
			/* Convert into bytes! */
			break;
		case RC_WMORE:
			/* PER does not support restartability */

			/* Continue accumulating data */
			break;
	}

	return 1;
}




/*********************************************************************/

static void *tall_sccp_ctx;
static LLIST_HEAD(sccp_connections);

#define SCCP_MSG_SIZE 4096
#define SCCP_MSG_HEADROOM 128

/* global data */
const struct sockaddr_sccp sccp_ssn_bssap = {
	.sccp_family	= 0,
	.sccp_ssn	= SCCP_SSN_BSSAP,
};
/*add for get ranap data from sccp packet*/
const struct sockaddr_sccp sccp_ssn_ranap = {
	.sccp_family	= 0,
	.sccp_ssn	= SCCP_SSN_RANAP,
};


/*book add for get scmg data from sccp packet, 2011-12-13*/
const struct sockaddr_sccp sccp_ssn_management = {
	.sccp_family	= 0,
	.sccp_ssn	= SCCP_SSN_MANAGEMENT,
};


struct sccp_system {
	/* layer3 -> layer2 */
	void (*write_data)(struct sccp_connection *conn, struct msgb *data, void *context);
	void *write_context;
};

static struct sccp_system sccp_system = {
	.write_data = NULL,
};

struct sccp_data_callback {
	/* connection based */
	int (*accept_cb)(struct sccp_connection *, void *);
	void *accept_context;

	/* connection less used for UDT*/
	int (*read_cb)(struct msgb *, unsigned int, void *);
	void *read_context;

	uint8_t ssn;
	struct llist_head callback;
};

static LLIST_HEAD(sccp_callbacks);

static struct sccp_data_callback *_find_ssn(uint8_t ssn)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_data_callback *cb;

	llist_for_each_entry(cb, &sccp_callbacks, callback) {
		if (cb->ssn == ssn)
			return cb;
	}

	/* need to add one */
	cb = talloc_zero(tall_sccp_ctx, struct sccp_data_callback);
	if (!cb) {
		LOGP(DSCCP, LOGL_ERROR, "Failed to allocate sccp callback.\n");
		return NULL;
	}

	cb->ssn = ssn;
	llist_add_tail(&cb->callback, &sccp_callbacks);
	return cb;
}


static void _send_msg(struct sccp_connection *conn, struct msgb *msg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	sccp_system.write_data(conn, msg, sccp_system.write_context);
}

/*
 * parsing routines
 */
static int copy_address(struct sccp_address *addr, uint8_t offset, struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_called_party_address *party;

	int room = msgb_l2len(msgb) - offset;
	uint8_t read = 0;
	uint8_t length;

	if (room <= 0) {
		LOGP(DSCCP, LOGL_ERROR, "Not enough room for an address: %u\n", room);
		return -1;
	}

	length = msgb->l2h[offset];
	if (room <= length) {
		LOGP(DSCCP, LOGL_ERROR, "Not enough room for optional data %u %u\n", room, length);
		return -1;
	}


	party = (struct sccp_called_party_address *)(msgb->l2h + offset + 1);
	if (party->point_code_indicator) {
		if (length <= read + 2) {
		    LOGP(DSCCP, LOGL_ERROR, "POI does not fit %u\n", length);
		    return -1;
		}


		memcpy(&addr->poi, &party->data[read], 2);
		read += 2;
	}

	if (party->ssn_indicator) {
		if (length <= read + 1) {
		    LOGP(DSCCP, LOGL_ERROR, "SSN does not fit %u\n", length);
		    return -1;
		}

		addr->ssn = party->data[read];
		read += 1;
	}

	/* copy the GTI over */
	if (party->global_title_indicator) {
		addr->gti_len = length - read - 1;
		addr->gti_data = &party->data[read];
	}

	addr->address = *party;
	return 0;
}

static int _sccp_parse_optional_data(const int offset,
				     struct msgb *msgb, struct sccp_optional_data *data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	uint16_t room = msgb_l2len(msgb) - offset;
	uint16_t read = 0;

	while (room > read) {
		uint8_t type = msgb->l2h[offset + read];
		if (type == SCCP_PNC_END_OF_OPTIONAL)
			return 0;

		if (read + 1 >= room) {
			LOGP(DSCCP, LOGL_ERROR, "no place for length\n");
			return 0;
		}

		uint8_t length = msgb->l2h[offset + read + 1];
		read += 2 + length;


		if (room <= read) {
			LOGP(DSCCP, LOGL_ERROR,
			       "no space for the data: type: %d read: %d room: %d l2: %d\n",
			       type, read, room, msgb_l2len(msgb));
			return 0;
		}

		if (type == SCCP_PNC_DATA) {
			data->data_len = length;
			data->data_start = offset + read - length;
		}

	}

	return -1;
}

int _sccp_parse_connection_request(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static const uint32_t header_size =
			sizeof(struct sccp_connection_request);
	static const uint32_t optional_offset =
			offsetof(struct sccp_connection_request, optional_start);
	static const uint32_t called_offset =
			offsetof(struct sccp_connection_request, variable_called);

	struct sccp_connection_request *req = (struct sccp_connection_request *)msgb->l2h;
	struct sccp_optional_data optional_data;

	/* header check */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	/* copy out the calling and called address. Add the offset */
	if (copy_address(&result->called, called_offset + req->variable_called, msgb) != 0)
		return -1;

	result->source_local_reference = &req->source_local_reference;

	/*
	 * parse optional data.
	 */
	memset(&optional_data, 0, sizeof(optional_data));
	if (_sccp_parse_optional_data(optional_offset + req->optional_start, msgb, &optional_data) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "parsing of optional data failed.\n");
		return -1;
	}

	if (optional_data.data_len != 0) {
		msgb->l3h = &msgb->l2h[optional_data.data_start];
		result->data_len = optional_data.data_len;
	} else {
		result->data_len = 0;
	}

	return 0;
}

int _sccp_parse_connection_released(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static int header_size = sizeof(struct sccp_connection_released);
	static int optional_offset = offsetof(struct sccp_connection_released, optional_start);

	struct sccp_optional_data optional_data;
	struct sccp_connection_released *rls = (struct sccp_connection_released *) msgb->l2h;

	/* we don't have enough size for the struct */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb > header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	memset(&optional_data, 0, sizeof(optional_data));
	if (_sccp_parse_optional_data(optional_offset + rls->optional_start, msgb, &optional_data) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "parsing of optional data failed.\n");
		return -1;
	}

	result->source_local_reference = &rls->source_local_reference;
	result->destination_local_reference = &rls->destination_local_reference;

	if (optional_data.data_len != 0) {
		msgb->l3h = &msgb->l2h[optional_data.data_start];
		result->data_len = optional_data.data_len;
	} else {
		result->data_len = 0;
	}

	return 0;
}

int _sccp_parse_connection_refused(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static const uint32_t header_size =
			sizeof(struct sccp_connection_refused);
	static int optional_offset = offsetof(struct sccp_connection_refused, optional_start);

	struct sccp_optional_data optional_data;
	struct sccp_connection_refused *ref;

	/* header check */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	ref = (struct sccp_connection_refused *) msgb->l2h;

	result->destination_local_reference = &ref->destination_local_reference;

	memset(&optional_data, 0, sizeof(optional_data));
	if (_sccp_parse_optional_data(optional_offset + ref->optional_start, msgb, &optional_data) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "parsing of optional data failed.\n");
		return -1;
	}

	/* optional data */
	if (optional_data.data_len != 0) {
		msgb->l3h = &msgb->l2h[optional_data.data_start];
		result->data_len = optional_data.data_len;
	} else {
		result->data_len = 0;
	}

	return 0;
}

int _sccp_parse_connection_confirm(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static uint32_t header_size =
		    sizeof(struct sccp_connection_confirm);
	static const uint32_t optional_offset =
			offsetof(struct sccp_connection_confirm, optional_start);

	struct sccp_optional_data optional_data;
	struct sccp_connection_confirm *con;

	/* header check */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	con = (struct sccp_connection_confirm *) msgb->l2h;
	result->destination_local_reference = &con->destination_local_reference;
	result->source_local_reference = &con->source_local_reference;

	memset(&optional_data, 0, sizeof(optional_data));
	if (_sccp_parse_optional_data(optional_offset + con->optional_start, msgb, &optional_data) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "parsing of optional data failed.\n");
		return -1;
	}

	if (optional_data.data_len != 0) {
		msgb->l3h = &msgb->l2h[optional_data.data_start];
		result->data_len = optional_data.data_len;
	} else {
		result->data_len = 0;
	}

	return 0;
}

int _sccp_parse_connection_release_complete(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static int header_size = sizeof(struct sccp_connection_release_complete);

	struct sccp_connection_release_complete *cmpl;

	/* header check */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	cmpl = (struct sccp_connection_release_complete *) msgb->l2h;
	result->source_local_reference = &cmpl->source_local_reference;
	result->destination_local_reference = &cmpl->destination_local_reference;

	return 0;
}

int _sccp_parse_connection_dt1(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static int header_size = sizeof(struct sccp_data_form1);
	static int variable_offset = offsetof(struct sccp_data_form1, variable_start);

	struct sccp_data_form1 *dt1 = (struct sccp_data_form1 *)msgb->l2h;

	/* we don't have enough size for the struct */
	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb > header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	if (dt1->segmenting != 0) {
		LOGP(DSCCP, LOGL_ERROR, "This packet has segmenting, not supported: %d\n", dt1->segmenting);
		return -1;
	}

	result->destination_local_reference = &dt1->destination_local_reference;

	/* some more  size checks in here */
	if (msgb_l2len(msgb) < variable_offset + dt1->variable_start + 1) {
		LOGP(DSCCP, LOGL_ERROR, "Not enough space for variable start: %u %u\n",
			msgb_l2len(msgb), dt1->variable_start);
		return -1;
	}

	result->data_len = msgb->l2h[variable_offset + dt1->variable_start];
	msgb->l3h = &msgb->l2h[dt1->variable_start + variable_offset + 1];

	if (msgb_l3len(msgb) < result->data_len) {
		LOGP(DSCCP, LOGL_ERROR, "Not enough room for the payload: %u %u\n",
			msgb_l3len(msgb), result->data_len);
		return -1;
	}

	return 0;
}

int _sccp_parse_udt(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static const uint32_t header_size = sizeof(struct sccp_data_unitdata);
	static const uint32_t called_offset = offsetof(struct sccp_data_unitdata, variable_called);
	static const uint32_t calling_offset = offsetof(struct sccp_data_unitdata, variable_calling);
	static const uint32_t data_offset = offsetof(struct sccp_data_unitdata, variable_data);

	struct sccp_data_unitdata *udt = (struct sccp_data_unitdata *)msgb->l2h;

	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	/* copy out the calling and called address. Add the off */
	if (copy_address(&result->called, called_offset + udt->variable_called, msgb) != 0)
		return -1;

	if (copy_address(&result->calling, calling_offset + udt->variable_calling, msgb) != 0)
		return -1;

	/* we don't have enough size for the data */
	if (msgb_l2len(msgb) < data_offset + udt->variable_data + 1) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header + offset %u %u %u\n",
			msgb_l2len(msgb), header_size, udt->variable_data);
		return -1;
	}


	msgb->l3h = &udt->data[udt->variable_data];
	result->data_len = msgb_l3len(msgb);

	if (msgb_l3len(msgb) <  msgb->l3h[-1]) {
		LOGP(DSCCP, LOGL_ERROR, "msgb is truncated is: %u should: %u\n",
			msgb_l3len(msgb), msgb->l3h[-1]);
		return -1;
	}

	return 0;
}

static int _sccp_parse_it(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static const uint32_t header_size = sizeof(struct sccp_data_it);

	struct sccp_data_it *it;

	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		        msgb_l2len(msgb), header_size);
		return -1;
	}

	it = (struct sccp_data_it *) msgb->l2h;
	result->data_len = 0;
	result->source_local_reference = &it->source_local_reference;
	result->destination_local_reference = &it->destination_local_reference;
	return 0;
}

static int _sccp_parse_err(struct msgb *msgb, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	static const uint32_t header_size = sizeof(struct sccp_proto_err);

	struct sccp_proto_err *err;

	if (msgb_l2len(msgb) < header_size) {
		LOGP(DSCCP, LOGL_ERROR, "msgb < header_size %u %u\n",
		     msgb_l2len(msgb), header_size);
		return -1;
	}

	err = (struct sccp_proto_err *) msgb->l2h;
	result->data_len = 0;
	result->destination_local_reference = &err->destination_local_reference;
	return 0;
}

static void create_sccp_addr(struct msgb *msg, const struct sockaddr_sccp *sock)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int pos = 2;
	uint8_t *data;

	data = msgb_put(msg, 1 + 2 + sock->gti_len);
	data[0] = 2;

	if (sock->gti)
		data[1] = 0 << 6 | (sock->gti_ind & 0x0f) << 2;
	else
		data[1] = 1 << 6 | 1 << 1;

	/* store a point code */
	if (sock->use_poi) {
		msgb_put(msg, 2);
		data[0] += 2;
		data[1] |= 0x01;
		data[pos++] = sock->poi[0];
		data[pos++] = sock->poi[1];
	}


	data[pos++] = sock->sccp_ssn;

	/* copy the gti if it is present */
	memcpy(&data[pos++], sock->gti, sock->gti_len);
}

/*
 * Send UDT. Currently we have a fixed address...
 */
struct msgb *sccp_create_udt(int class, const struct sockaddr_sccp *in,
			     const struct sockaddr_sccp *out, uint8_t *in_data, int len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_data_unitdata *udt;
	uint8_t *data;

	if (len > 256) {
		LOGP(DSCCP, LOGL_ERROR, "The payload is too big for one udt\n");
		return NULL;
	}

	struct msgb *msg = msgb_alloc_headroom(SCCP_MSG_SIZE,
					       SCCP_MSG_HEADROOM, "sccp: udt");
	if (!msg)
		return NULL;

	msg->l2h = &msg->data[0];
	udt = (struct sccp_data_unitdata *)msgb_put(msg, sizeof(*udt));

	udt->type = SCCP_MSG_TYPE_UDT;
	udt->proto_class = class;
	udt->variable_called = 3;
/*	
	udt->variable_calling = 5 + out->gti_len;
	udt->variable_data = 7 + out->gti_len + in->gti_len;
*/
	if (out->use_poi) {
		udt->variable_calling = udt->variable_called + 4 + out->gti_len;
	}
	else {	
		udt->variable_calling = udt->variable_called + 2 + out->gti_len;
	}

	if (in->use_poi) {
		udt->variable_data = udt->variable_calling + 4 + out->gti_len + in->gti_len;
	}
	else {
		udt->variable_data = udt->variable_calling + 2 + out->gti_len + in->gti_len;
	}

	/* for variable data we start with a size and the data */
	create_sccp_addr(msg, out);
	create_sccp_addr(msg, in);

	/* copy the payload */
	data = msgb_put(msg, 1 + len);
	data[0] = len;
	memcpy(&data[1], in_data, len);

	return msg;
}

static int _sccp_send_data(int class, const struct sockaddr_sccp *in,
			   const struct sockaddr_sccp *out, struct msgb *payload)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msg;

	msg = sccp_create_udt(class, in, out, payload->l3h, msgb_l3len(payload));
	if (!msg)
		return -1;

	_send_msg(NULL, msg);
	return 0;
}

static int _sccp_handle_read(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_data_callback *cb;
	struct sccp_parse_result result;

	if (_sccp_parse_udt(msgb, &result) != 0)
		return -1;

	cb = _find_ssn(result.called.ssn);
	if (!cb || !cb->read_cb) {
		LOGP(DSCCP, LOGL_ERROR, "No routing for UDT for called SSN: %u\n", result.called.ssn);
		return -1;
	}

	/* sanity check */
	return cb->read_cb(msgb, msgb_l3len(msgb), cb->read_context);
}

/*
 * handle connection orientated methods
 */
static int source_local_reference_is_free(struct sccp_source_reference *reference)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;

	llist_for_each_entry(connection, &sccp_connections, list) {
		if (memcmp(reference, &connection->source_local_reference, sizeof(*reference)) == 0)
			return -1;
	}

	return 0;
}

struct sccp_connection *find_sccp_connection_by_sou_loc_ref(struct sccp_source_reference *reference)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;

	llist_for_each_entry(connection, &sccp_connections, list) {
		if (memcmp(reference, &connection->source_local_reference, sizeof(*reference)) == 0)
			return connection;
	}

	return NULL;
}

static int destination_local_reference_is_free(struct sccp_source_reference *reference)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;

	llist_for_each_entry(connection, &sccp_connections, list) {
		if (memcmp(reference, &connection->destination_local_reference, sizeof(*reference)) == 0)
			return -1;
	}

	return 0;
}

struct sccp_connection *find_sccp_connection_by_dst_loc_ref(struct sccp_source_reference *reference)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;

	llist_for_each_entry(connection, &sccp_connections, list) {
		if (memcmp(reference, &connection->destination_local_reference, sizeof(*reference)) == 0)
			return connection;
	}

	return NULL;
}

struct sccp_connection *find_sccp_connection_by_context(unsigned char *context, unsigned int domainid)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;
	iu_log_debug("domainid = %d\n",domainid);
	llist_for_each_entry(connection, &sccp_connections, list) {
	    if((connection != NULL) && (context != NULL) && (connection->contextid != NULL)){
			iu_log_debug("context[0]=%x,context[1]=%x,context[2]=%x\n",context[0],context[1],context[2]);
			iu_log_debug("connection->contextid[0]=%x,connection->contextid[1]=%x,connection->contextid[2]=%x\n",connection->contextid[0],connection->contextid[1],connection->contextid[2]);
			if ((memcmp(context, connection->contextid, 3) == 0) && (domainid == connection->domainid)){
				iu_log_debug("find contextid from connection\n"); 
    			return connection;
    		}
    		else{
    		    iu_log_debug("can not find contextid from connection\n"); 
    		}
		}
	}
	iu_log_debug("loop end\n");
	return NULL;
}


/* -----------------------------------------------
  book add for release sccp connection only by contextid
  2011-12-16
 ------------------------------------------------*/
int release_sccp_connection_by_context(unsigned char *context)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_connection *connection;
	llist_for_each_entry(connection, &sccp_connections, list) {
	    if((connection != NULL) && (context != NULL) && (connection->contextid != NULL)){
    		if(memcmp(context, connection->contextid, 3) == 0){
				iu_log_debug("find contextid from connection\n"); 
				return sccp_connection_close(connection, 0);
    		}
    		else{
    		    iu_log_debug("can not find connection by context[0]=%x,context[1]=%x,context[2]=%x\n",context,context+1,context+2); 
    		}
		}
	}
	iu_log_debug("loop end\n");
	return 0;
}



static int assign_source_local_reference(struct sccp_connection *connection)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	/*0 < ref < 0x00ffffff  1 < contextid < 8192 so
	10 0000 0000 0000 14 bits for contextid from iuh, furthest left for cs or ps */
	static uint32_t last_ref = 0x30000;
	last_ref = last_ref | gFlag;
	gFlag++;
	if(gFlag == 0x00000a)
	{
	    gFlag = 0x000000;
	}
	int wrapped = 2;

	if (connection->contextid) {
		last_ref = last_ref | 0x800000;
	}
	
	do {
		struct sccp_source_reference reference;
		reference.octet3 = ((last_ref >>  0) | connection->contextid[0]) & 0xff;
		reference.octet2 = ((last_ref >>  8) | connection->contextid[1]) & 0xff;
		reference.octet1 = ((last_ref >> 16) | connection->contextid[2]) & 0xff;

iu_log_debug("assign_source_local_reference context id is %x %x %x, domain id is %x, reference is %x %x %x \n",
			connection->contextid[2], connection->contextid[1], connection->contextid[0], connection->domainid,\
			reference.octet1, reference.octet2, reference.octet3);
		//++last_ref;
		
		/* do not use the reversed word and wrap around 
		if ((last_ref & 0x00FFFFFF) == 0x00FFFFFF) {
			LOGP(DSCCP, LOGL_DEBUG, "Wrapped searching for a free code\n");
			last_ref = 0;
			++wrapped;
		}*/

		if (source_local_reference_is_free(&reference) == 0) {
			connection->source_local_reference = reference;
			return 0;
		}
	} while (wrapped != 2);

	LOGP(DSCCP, LOGL_ERROR, "Finding a free reference failed\n");
	return -1;
}

static void _sccp_set_connection_state(struct sccp_connection *connection, int new_state)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int old_state = connection->connection_state;
/*****************use one con*****************
	if (SCCP_CONNECTION_STATE_ESTABLISHED == new_state) {
		start_sccp_con = 0;
		global_reference = connection->source_local_reference;
	}
***********************************************/
	connection->connection_state = new_state;
	if (connection->state_cb)
		connection->state_cb(connection, old_state);
}

struct msgb *sccp_create_refuse(struct sccp_source_reference *src_ref, int cause, uint8_t *inp, int length)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb;
	struct sccp_connection_refused *ref;
	uint8_t *data;

	msgb = msgb_alloc_headroom(SCCP_MSG_SIZE,
				   SCCP_MSG_HEADROOM, "sccp ref");
	if (!msgb) {
		LOGP(DSCCP, LOGL_ERROR, "Failed to allocate refusal msg.\n");
		return NULL;
	}
    
	msgb->l2h = &msgb->data[0];

	ref = (struct sccp_connection_refused *) msgb_put(msgb, sizeof(*ref));
	ref->type = SCCP_MSG_TYPE_CREF;
	memcpy(&ref->destination_local_reference, src_ref,
	       sizeof(struct sccp_source_reference));
	ref->cause = cause;
	ref->optional_start = 1;

	if (inp) {
		data = msgb_put(msgb, 1 + 1 + length);
		data[0] = SCCP_PNC_DATA;
		data[1] = length;
		memcpy(&data[2], inp, length);
	}

	data = msgb_put(msgb, 1);
	data[0] = SCCP_PNC_END_OF_OPTIONAL;
	return msgb;
}

static int _sccp_send_refuse(struct sccp_source_reference *src_ref, int cause)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb = sccp_create_refuse(src_ref, cause, NULL, 0);
	if (!msgb)
		return -1;

	_send_msg(NULL, msgb);
	return 0;
}

struct msgb *sccp_create_cc(struct sccp_source_reference *src_ref,
			    struct sccp_source_reference *dst_ref)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *response;
	struct sccp_connection_confirm *confirm;
	uint8_t *optional_data;

	response = msgb_alloc_headroom(SCCP_MSG_SIZE,
				       SCCP_MSG_HEADROOM, "sccp confirm");
	if (!response) {
		LOGP(DSCCP, LOGL_ERROR, "Failed to create SCCP Confirm.\n");
		return NULL;
	}

	response->l2h = &response->data[0];

	confirm = (struct sccp_connection_confirm *) msgb_put(response, sizeof(*confirm));

	confirm->type = SCCP_MSG_TYPE_CC;
	memcpy(&confirm->destination_local_reference,
	       dst_ref, sizeof(*dst_ref));
	memcpy(&confirm->source_local_reference,
	       src_ref, sizeof(*src_ref));
	confirm->proto_class = 2;
	confirm->optional_start = 1;

	optional_data = (uint8_t *) msgb_put(response, 1);
	optional_data[0] = SCCP_PNC_END_OF_OPTIONAL;
	return response;
}

static int _sccp_send_connection_confirm(struct sccp_connection *connection)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *response;

	if (assign_source_local_reference(connection) != 0)
		return -1;

	response = sccp_create_cc(&connection->source_local_reference,
				  &connection->destination_local_reference);
	if (!response)
		return -1;
	iu_log_debug("+++++++++_sccp_send_connection_confirm conn->source_local_reference %d, %d, %d\n", connection->source_local_reference.octet1,
		connection->source_local_reference.octet2, connection->source_local_reference.octet3);

	_send_msg(connection, response);
	_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_ESTABLISHED);
	
 	return 0;
}

static int _sccp_send_connection_request(struct sccp_connection *connection,
					 const struct sockaddr_sccp *called, struct msgb *msg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *request;
	struct sccp_connection_request *req;
	uint8_t *data;
	uint8_t extra_size = 3 + 1;


	if (msg && (msgb_l3len(msg) < 3 || msgb_l3len(msg) > 130)) {
		LOGP(DSCCP, LOGL_ERROR, "Invalid amount of data... %d\n", msgb_l3len(msg));
		iu_log_error("Invalid amount of data... %d\n", msgb_l3len(msg));
		return -1;
	}
 	/* try to find a id */
	if (assign_source_local_reference(connection) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "Assigning a local reference failed.\n");
		_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_SETUP_ERROR);
		iu_log_error("Assigning a local reference failed.\n");
		return -1;
	}
	iu_log_debug("<<<<_sccp_send_connection_request conn->source_local_reference %d, %d, %d\n", connection->source_local_reference.octet1,
		connection->source_local_reference.octet2, connection->source_local_reference.octet3);
 
	if (msg)
		extra_size += 2 + msgb_l3len(msg);
	request = msgb_alloc_headroom(SCCP_MSG_SIZE,
				      SCCP_MSG_HEADROOM, "sccp connection request");
	request->l2h = &request->data[0];
	req = (struct sccp_connection_request *) msgb_put(request, sizeof(*req));

	req->type = SCCP_MSG_TYPE_CR;
	memcpy(&req->source_local_reference, &connection->source_local_reference,
	       sizeof(connection->source_local_reference));
	req->proto_class = 2;
	req->variable_called = 2;
	req->optional_start = 4 + called->gti_len;

	/* write the called party address */
	create_sccp_addr(request, called);
 
	/* write the payload */
	if (msg) {
	    data = msgb_put(request, 2 + msgb_l3len(msg));
	    data[0] = SCCP_PNC_DATA;
	    data[1] = msgb_l3len(msg);
	    memcpy(&data[2], msg->l3h, msgb_l3len(msg));
	}

	data = msgb_put(request, 1);
	data[0] = SCCP_PNC_END_OF_OPTIONAL;

	llist_add_tail(&connection->list, &sccp_connections);
	_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_REQUEST);
 
	_send_msg(connection, request);
	return 0;
}

struct msgb *sccp_create_dt1(struct sccp_source_reference *dst_ref, uint8_t *inp_data, uint8_t len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb;
	struct sccp_data_form1 *dt1;
	uint8_t *data;

	msgb = msgb_alloc_headroom(SCCP_MSG_SIZE,
				   SCCP_MSG_HEADROOM, "sccp dt1");
	if (!msgb) {
		LOGP(DSCCP, LOGL_ERROR, "Failed to create DT1 msg.\n");
		return NULL;
	}

	msgb->l2h = &msgb->data[0];

	dt1 = (struct sccp_data_form1 *) msgb_put(msgb, sizeof(*dt1));
	dt1->type = SCCP_MSG_TYPE_DT1;
	memcpy(&dt1->destination_local_reference, dst_ref,
	       sizeof(struct sccp_source_reference));
	dt1->segmenting = 0;

	/* copy the data */
	dt1->variable_start = 1;
	data = msgb_put(msgb, 1 + len);
	data[0] = len;
	memcpy(&data[1], inp_data, len);

	return msgb;
}

static int _sccp_send_connection_data(struct sccp_connection *conn, struct msgb *_data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb;

	if (msgb_l3len(_data) < 2 || msgb_l3len(_data) > 256) {
		LOGP(DSCCP, LOGL_ERROR, "data size too big, segmenting unimplemented.\n");
		return -1;
	}

	msgb = sccp_create_dt1(&conn->destination_local_reference,
			       _data->l3h, msgb_l3len(_data));
	if (!msgb)
		return -1;

	_send_msg(conn, msgb);
	return 0;
}

static int _sccp_send_connection_it(struct sccp_connection *conn)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb;
	struct sccp_data_it *it;

	msgb = msgb_alloc_headroom(SCCP_MSG_SIZE,
				   SCCP_MSG_HEADROOM, "sccp it");
	msgb->l2h = &msgb->data[0];
	it = (struct sccp_data_it *) msgb_put(msgb, sizeof(*it));
	it->type = SCCP_MSG_TYPE_IT;
	memcpy(&it->destination_local_reference, &conn->destination_local_reference,
		sizeof(struct sccp_source_reference));
	memcpy(&it->source_local_reference, &conn->source_local_reference,
		sizeof(struct sccp_source_reference));

	it->proto_class = 0x2;
	it->sequencing[0] = it->sequencing[1] = 0;
	it->credit = 0;

	_send_msg(conn, msgb);
	return 0;
}

struct msgb *sccp_create_rlsd(struct sccp_source_reference *src_ref,
			      struct sccp_source_reference *dst_ref, int cause)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msg;
	struct sccp_connection_released *rel;
	uint8_t *data;

	msg = msgb_alloc_headroom(SCCP_MSG_SIZE, SCCP_MSG_HEADROOM,
				  "sccp: connection released");
	if (!msg) {
		LOGP(DSCCP, LOGL_ERROR, "Failed to allocate RLSD.\n");
		return NULL;
	}

	msg->l2h = &msg->data[0];
	rel = (struct sccp_connection_released *) msgb_put(msg, sizeof(*rel));
	rel->type = SCCP_MSG_TYPE_RLSD;
	rel->release_cause = cause;

	/* copy the source references */
	memcpy(&rel->destination_local_reference, dst_ref,
	       sizeof(struct sccp_source_reference));
	memcpy(&rel->source_local_reference, src_ref,
	       sizeof(struct sccp_source_reference));

	data = msgb_put(msg, 1);
	data[0] = SCCP_PNC_END_OF_OPTIONAL;
	return msg;
}

static int _sccp_send_connection_released(struct sccp_connection *conn, int cause)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msg;

	msg = sccp_create_rlsd(&conn->source_local_reference,
			       &conn->destination_local_reference,
			       cause);
	if (!msg)
		return -1;

	_sccp_set_connection_state(conn, SCCP_CONNECTION_STATE_RELEASE);
	_send_msg(conn, msg);
	return 0;
}

/*
 * Open a connection. The following is going to happen:
 *
 *	- Verify the packet, e.g. that we have no other connection
 *	  that id.
 *      - Ask the user if he wants to accept the connection
 *      - Try to open the connection by assigning a source local reference
 *        and sending the packet
 */
static int _sccp_handle_connection_request(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;

	struct sccp_data_callback *cb;
	struct sccp_connection *connection;

	if (_sccp_parse_connection_request(msgb, &result) != 0)
		return -1;
	
 
	cb = _find_ssn(result.called.ssn);
	if (!cb || !cb->accept_cb) {
		LOGP(DSCCP, LOGL_ERROR, "No routing for CR for called SSN: %u\n", result.called.ssn);
		return -1;
	}

	/* check if the system wants this connection */
	connection = talloc_zero(tall_sccp_ctx, struct sccp_connection);
	if (!connection) {
		LOGP(DSCCP, LOGL_ERROR, "Allocation failed\n");
		return -1;
	}
 
	/*
	 * sanity checks:
	 *	- Is the source_local_reference in any other connection?
	 * then will call accept, assign a "destination" local reference
	 * and send a connection confirm, otherwise we will send a refuseed
	 * one....
	 */
	if (destination_local_reference_is_free(result.source_local_reference) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "Need to reject connection with existing reference\n");
		_sccp_send_refuse(result.source_local_reference, SCCP_REFUSAL_SCCP_FAILURE);
		talloc_free(connection);
		return -1;
	}
 
	connection->incoming = 1;
	connection->destination_local_reference = *result.source_local_reference;

	if (cb->accept_cb(connection, cb->accept_context) != 0) {
		_sccp_send_refuse(result.source_local_reference, SCCP_REFUSAL_END_USER_ORIGINATED);
		_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_REFUSED);
		talloc_free(connection);
		return 0;
	}

	llist_add_tail(&connection->list, &sccp_connections);
 
	if (_sccp_send_connection_confirm(connection) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "Sending confirm failed... no available source reference?\n");

		_sccp_send_refuse(result.source_local_reference, SCCP_REFUSAL_SCCP_FAILURE);
		_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_REFUSED);
		llist_del(&connection->list);
		talloc_free(connection);

		return -1;
	}
 
	/*
	 * If we have data let us forward things.
	 */
	if (result.data_len != 0 && connection->data_cb) {
 		connection->data_cb(connection, msgb, result.data_len);
	}

	return 0;
}

/* Handle the release confirmed */
static int _sccp_handle_connection_release_complete(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;
	struct sccp_connection *conn;

	if (_sccp_parse_connection_release_complete(msgb, &result) != 0)
		return -1;

	/* find the connection */
	llist_for_each_entry(conn, &sccp_connections, list) {
		if (conn->data_cb
		    && memcmp(&conn->source_local_reference,
			      result.destination_local_reference,
			      sizeof(conn->source_local_reference)) == 0
		    && memcmp(&conn->destination_local_reference,
			      result.source_local_reference,
			      sizeof(conn->destination_local_reference)) == 0) {
		    goto found;
		}
	}


	LOGP(DSCCP, LOGL_ERROR, "Release complete of unknown connection\n");
	return -1;

found:
	llist_del(&conn->list);
	_sccp_set_connection_state(conn, SCCP_CONNECTION_STATE_RELEASE_COMPLETE);
	return 0;
}

/* Handle the Data Form 1 message */
static int _sccp_handle_connection_dt1(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;
	struct sccp_connection *conn;

	if (_sccp_parse_connection_dt1(msgb, &result) != 0)
		return -1;
 
	/* lookup if we have a connection with the given reference */
	llist_for_each_entry(conn, &sccp_connections, list) {
/*iu_log_debug("_sccp_handle_connection_dt1 conn->source_local_reference %d, %d, %d\n", conn->source_local_reference.octet1,
	conn->source_local_reference.octet2, conn->source_local_reference.octet3);
iu_log_debug("_sccp_handle_connection_dt1 result.destination_local_reference %d, %d, %d\n", result.destination_local_reference->octet1,
		result.destination_local_reference->octet2, result.destination_local_reference->octet3);
*/
		if (conn->data_cb
		    && memcmp(&conn->source_local_reference,
			      result.destination_local_reference,
			      sizeof(conn->source_local_reference)) == 0) {
			goto found;
		}
	}

 	return -1;

found:
	LOGP(DSCCP, LOGL_ERROR, "can find connect for this data1 \n");
	conn->data_cb(conn, msgb, result.data_len);
	return 0;
}

/* confirm a connection release */
static int _sccp_send_connection_release_complete(struct sccp_connection *connection)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct msgb *msgb;
	struct sccp_connection_release_complete *rlc;

	msgb = msgb_alloc_headroom(SCCP_MSG_SIZE,
				   SCCP_MSG_HEADROOM, "sccp rlc");
	msgb->l2h = &msgb->data[0];

	rlc = (struct sccp_connection_release_complete *) msgb_put(msgb, sizeof(*rlc));
	rlc->type = SCCP_MSG_TYPE_RLC;
	memcpy(&rlc->destination_local_reference,
	       &connection->destination_local_reference, sizeof(struct sccp_source_reference));
	memcpy(&rlc->source_local_reference,
	       &connection->source_local_reference, sizeof(struct sccp_source_reference));

	_send_msg(connection, msgb);

	/*
	 * Remove from the list of active connections and set the state. User code
	 * should now free the entry.
	 */
	llist_del(&connection->list);
	_sccp_set_connection_state(connection, SCCP_CONNECTION_STATE_RELEASE_COMPLETE);
	return 0;
}

/* connection released, send a released confirm */
static int _sccp_handle_connection_released(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;
	struct sccp_connection *conn;

	if (_sccp_parse_connection_released(msgb, &result) == -1)
		return -1;

	/* lookup if we have a connection with the given reference */
	llist_for_each_entry(conn, &sccp_connections, list) {
		if (conn->data_cb
		    && memcmp(&conn->source_local_reference,
			      result.destination_local_reference,
			      sizeof(conn->source_local_reference)) == 0
		    && memcmp(&conn->destination_local_reference,
			      result.source_local_reference,
			      sizeof(conn->destination_local_reference)) == 0) {
		    goto found;
		}
	}


	LOGP(DSCCP, LOGL_ERROR, "Unknown connection was released.\n");
	return -1;

	/* we have found a connection */
found:
	/* optional data */
	if (result.data_len != 0 && conn->data_cb) {
		conn->data_cb(conn, msgb, result.data_len);
	}

	/* generate a response */
	if (_sccp_send_connection_release_complete(conn) != 0) {
		LOGP(DSCCP, LOGL_ERROR, "Sending release confirmed failed\n");
		return -1;
	}

	return 0;
}

static int _sccp_handle_connection_refused(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;
	struct sccp_connection *conn;

	if (_sccp_parse_connection_refused(msgb, &result) != 0)
		return -1;

	/* lookup if we have a connection with the given reference */
	llist_for_each_entry(conn, &sccp_connections, list) {
		if (conn->incoming == 0 && conn->data_cb
		    && memcmp(&conn->source_local_reference,
			      result.destination_local_reference,
			      sizeof(conn->source_local_reference)) == 0) {
		    goto found;
		}
	}

	LOGP(DSCCP, LOGL_ERROR, "Refused but no connection found\n");
	return -1;

found:
	/* optional data */
	if (result.data_len != 0 && conn->data_cb) {
		conn->data_cb(conn, msgb, result.data_len);
	}


	llist_del(&conn->list);
	_sccp_set_connection_state(conn, SCCP_CONNECTION_STATE_REFUSED);
	return 0;
}

static int _sccp_handle_connection_confirm(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_parse_result result;
	struct sccp_connection *conn;

	if (_sccp_parse_connection_confirm(msgb, &result) != 0)
		return -1;

	/* lookup if we have a connection with the given reference */
	llist_for_each_entry(conn, &sccp_connections, list) {
		if (conn->incoming == 0 && conn->data_cb
		    && memcmp(&conn->source_local_reference,
			      result.destination_local_reference,
			      sizeof(conn->source_local_reference)) == 0) {
		    goto found;
		}
	}

	LOGP(DSCCP, LOGL_ERROR, "Confirmed but no connection found\n");
	return -1;

found:
	/* copy the addresses of the connection */
	conn->destination_local_reference = *result.source_local_reference;
	_sccp_set_connection_state(conn, SCCP_CONNECTION_STATE_ESTABLISHED);
	iu_log_debug("-------YYYYYYYYYYYYYYYYYYYY handle cc set state ESTABLISHED \n");
	/* optional data */
	if (result.data_len != 0 && conn->data_cb) {
		conn->data_cb(conn, msgb, result.data_len);
	}

	return 0;
}

int sccp_system_init(void (*outgoing)(struct sccp_connection *conn, struct msgb *data, void *ctx), void *ctx)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	sccp_system.write_data = outgoing;
	sccp_system.write_context = ctx;

	return 0;
}

/* oh my god a real SCCP packet. need to dispatch it now */
int sccp_system_incoming(struct msgb *msgb)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if (msgb_l2len(msgb) < 1 ) {
		LOGP(DSCCP, LOGL_ERROR, "Too short packet\n");
		return -1;
	}

	int type = msgb->l2h[0];
	switch(type) {
	case SCCP_MSG_TYPE_CR:
		return _sccp_handle_connection_request(msgb);
		break;
	case SCCP_MSG_TYPE_RLSD:
		return _sccp_handle_connection_released(msgb);
		break;
	case SCCP_MSG_TYPE_CREF:
		return _sccp_handle_connection_refused(msgb);
		break;
	case SCCP_MSG_TYPE_CC:
		return _sccp_handle_connection_confirm(msgb);
		break;
	case SCCP_MSG_TYPE_RLC:
		return _sccp_handle_connection_release_complete(msgb);
		break;
	case SCCP_MSG_TYPE_DT1:
		return _sccp_handle_connection_dt1(msgb);
		break;
	case SCCP_MSG_TYPE_UDT:
		return _sccp_handle_read(msgb);
		break;
	default:
		LOGP(DSCCP, LOGL_ERROR, "unimplemented msg type: %d\n", type);
	};

	return -1;
}

/* create a packet from the data */
int sccp_connection_write(struct sccp_connection *connection, struct msgb *data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if (connection->connection_state < SCCP_CONNECTION_STATE_CONFIRM
	    || connection->connection_state > SCCP_CONNECTION_STATE_ESTABLISHED) {
		LOGP(DSCCP, LOGL_ERROR, "sccp_connection_write: Wrong connection state: %p %d\n",
		       connection, connection->connection_state);
		return -1;
	}

	return _sccp_send_connection_data(connection, data);
}

/*
 * Send a Inactivity Test message. The owner of the connection
 * should start a timer and call this method regularily. Calling
 * this every 60 seconds should be good enough.
 */
int sccp_connection_send_it(struct sccp_connection *connection)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if (connection->connection_state < SCCP_CONNECTION_STATE_CONFIRM
	    || connection->connection_state > SCCP_CONNECTION_STATE_ESTABLISHED) {
		LOGP(DSCCP, LOGL_ERROR, "sccp_connection_write: Wrong connection state: %p %d\n",
		       connection, connection->connection_state);
		return -1;
	}

	return _sccp_send_connection_it(connection);
}

/* send a connection release and wait for the connection released */
int sccp_connection_close(struct sccp_connection *connection, int cause)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if (connection->connection_state < SCCP_CONNECTION_STATE_CONFIRM
	    || connection->connection_state > SCCP_CONNECTION_STATE_ESTABLISHED) {
		LOGP(DSCCP, LOGL_ERROR, "Can not close the connection. It was never opened: %p %d\n",
			connection, connection->connection_state);
		return -1;
	}

	return _sccp_send_connection_released(connection, cause);
}

int sccp_connection_free(struct sccp_connection *connection)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if(connection == NULL) return 0;

	iu_log_debug("\n\n!!!!!!!!!!!!!!!!!!!!!!!   connection->connection_state = %d  !!!!!!!!!!!!!!!!!!!!\n\n",connection->connection_state);
	if (connection->connection_state > SCCP_CONNECTION_STATE_NONE
	    && connection->connection_state < SCCP_CONNECTION_STATE_RELEASE_COMPLETE) {
		LOGP(DSCCP, LOGL_ERROR, "The connection needs to be released before it is freed");
		return -1;
	}

	talloc_free(connection);
	return 0;
}

int sccp_connection_force_free(struct sccp_connection *con)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	iu_log_debug("\n\n!!!!!!!!!!!!!!!!!!!!!!!   con->connection_state = %d  !!!!!!!!!!!!!!!!!!!!\n\n",con->connection_state);
	if (con->connection_state > SCCP_CONNECTION_STATE_NONE &&
	    con->connection_state < SCCP_CONNECTION_STATE_RELEASE_COMPLETE)
		llist_del(&con->list);

	con->connection_state = SCCP_CONNECTION_STATE_REFUSED;
	sccp_connection_free(con);
        return 0;
}

struct sccp_connection *sccp_connection_socket(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	return talloc_zero(tall_sccp_ctx, struct sccp_connection);
}

int sccp_connection_connect(struct sccp_connection *conn,
			    const struct sockaddr_sccp *local,
			    struct msgb *data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	return _sccp_send_connection_request(conn, local, data);
}

int sccp_connection_set_incoming(const struct sockaddr_sccp *sock,
				 int (*accept_cb)(struct sccp_connection *, void *), void *context)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_data_callback *cb;

	if (!sock)
		return -2;

	cb = _find_ssn(sock->sccp_ssn);
	if (!cb)
		return -1;

	cb->accept_cb = accept_cb;
	cb->accept_context = context;
	return 0;
}

int sccp_write(struct msgb *data, const struct sockaddr_sccp *in,
	       const struct sockaddr_sccp *out, int class)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	return _sccp_send_data(class, in, out, data);
}

int sccp_set_read(const struct sockaddr_sccp *sock,
		  int (*read_cb)(struct msgb *, unsigned int, void *), void *context)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_data_callback *cb;

	if (!sock)
		return -2;

	cb  = _find_ssn(sock->sccp_ssn);
	if (!cb)
		return -1;

	cb->read_cb = read_cb;
	cb->read_context = context;
	return 0;
}

static_assert(sizeof(struct sccp_source_reference) <= sizeof(uint32_t), enough_space);

uint32_t sccp_src_ref_to_int(struct sccp_source_reference *ref)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	uint32_t src_ref = 0;
	memcpy(&src_ref, ref, sizeof(*ref));
	return src_ref;
}

struct sccp_source_reference sccp_src_ref_from_int(uint32_t int_ref)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sccp_source_reference ref;
	memcpy(&ref, &int_ref, sizeof(ref));
	return ref;
}

int sccp_determine_msg_type(struct msgb *msg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if (msgb_l2len(msg) < 1)
		return -1;

	return msg->l2h[0];
}

int sccp_parse_header(struct msgb *msg, struct sccp_parse_result *result)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int type;

	if (msgb_l2len(msg) < 1)
		return -1;

	type = msg->l2h[0];
	switch(type) {
	case SCCP_MSG_TYPE_CR:
		return _sccp_parse_connection_request(msg, result);
		break;
	case SCCP_MSG_TYPE_RLSD:
		return _sccp_parse_connection_released(msg, result);
		break;
	case SCCP_MSG_TYPE_CREF:
		return _sccp_parse_connection_refused(msg, result);
		break;
	case SCCP_MSG_TYPE_CC:
		return _sccp_parse_connection_confirm(msg, result);
		break;
	case SCCP_MSG_TYPE_RLC:
		return _sccp_parse_connection_release_complete(msg, result);
		break;
	case SCCP_MSG_TYPE_DT1:
		return _sccp_parse_connection_dt1(msg, result);
		break;
	case SCCP_MSG_TYPE_UDT:
		return _sccp_parse_udt(msg, result);
		break;
	case SCCP_MSG_TYPE_IT:
		return _sccp_parse_it(msg, result);
		break;
	case SCCP_MSG_TYPE_ERR:
		return _sccp_parse_err(msg, result);
		break;
	};

	LOGP(DSCCP, LOGL_ERROR, "Unimplemented MSG Type: 0x%x\n", type);
	return -1;
}

/* book modify, 2011-11-11 */
void sccp_write_to_m3ua(struct sccp_connection *conn, struct msgb *data, void *ctx)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	m3_rt_lbl_t	 prtlbl;
	m3_s32		ret;
	m3ua_txr_t	api;

	if(conn == NULL){
	    iu_log_info("conn is NULL\n");
	}
	else{
		iu_log_debug("incoming = %d\n",conn->incoming);
	}
	if(data == NULL){
	    iu_log_info("data is NULL\n");
	}
	if(ctx == NULL){
	    iu_log_info("ctx is NULL\n");
	}

    m3_u16  usrid = 0;
    
    /* book modify, 2011-12-27 */
    if(conn != NULL)
    {
    	iu_log_debug("conn->domainid = %d\n",conn->domainid);
        if(conn->domainid == 0){
    	    prtlbl.dpc = global_msc_parameter.point_code;
    	    prtlbl.opc = home_gateway_msc.point_code;
    	    usrid = 0;
    	    if(msc_as_conf.trafficMode > M3_TRFMODE_OVERRIDE)
    	        api.nw_app = msc_as_conf.networkApperance;
    	    else
    	        api.nw_app = M3_MAX_U32;
    	}
    	else
    	{
    	    prtlbl.dpc = global_sgsn_parameter.point_code;
    	    prtlbl.opc = home_gateway_sgsn.point_code;
    	    usrid = 2;
    	    if(sgsn_as_conf.trafficMode > M3_TRFMODE_OVERRIDE)
    	        api.nw_app = sgsn_as_conf.networkApperance;
    	    else
    	        api.nw_app = M3_MAX_U32;
    	}
    }
    else{
		iu_log_debug("@@@ global_msc_dpc = %d, global_msc_opc = %d\n",global_msc_parameter.point_code, home_gateway_msc.point_code);
		iu_log_debug("@@@ global_sgsn_dpc = %d, global_sgsn_opc = %d\n",global_sgsn_parameter.point_code, home_gateway_sgsn.point_code);
		iu_log_debug("gCnDomain = %d\n", gCnDomain);
		if(gCnDomain == 0){
			prtlbl.dpc = global_msc_parameter.point_code;
    	    prtlbl.opc = home_gateway_msc.point_code;
    	    usrid = 0;
    	    if(msc_as_conf.trafficMode > M3_TRFMODE_OVERRIDE)
    	        api.nw_app = msc_as_conf.networkApperance;
    	    else
    	        api.nw_app = M3_MAX_U32;
		}
		else{
			prtlbl.dpc = global_sgsn_parameter.point_code;
    	    prtlbl.opc = home_gateway_sgsn.point_code;
    	    usrid = 2;
    	    if(sgsn_as_conf.trafficMode > M3_TRFMODE_OVERRIDE)
    	        api.nw_app = sgsn_as_conf.networkApperance;
    	    else
    	        api.nw_app = M3_MAX_U32;
		}
    }
    
	iu_log_debug("global_msc_dpc = %d, global_sgsn_dpc = %d\n",global_msc_parameter.point_code, global_sgsn_parameter.point_code);
	
	prtlbl.si = 3;
	prtlbl.ni = gNi;//for Thailand
	prtlbl.mp = 4;
	prtlbl.sls = 6;
	
	api.add_rtctx = M3_FALSE;
	api.crn_id = M3_MAX_U32;	/* Correlation Id management is part of LME */
	api.rt_lbl = prtlbl;
    
	api.prot_data_len = msgb_l2len(data);

	api.p_prot_data = data->l2h;
	iu_log_debug("<---- sccp_write_to_m3ua Sending Data Message with DPC:%x, OPC:%x, SI:%d ---->\n",
		prtlbl.dpc, prtlbl.opc, prtlbl.si);
	ret = m3ua_transfer(usrid, &api);
	if (-1 == ret) {
		iu_log_debug("Error recorded at Line:%d \n", __LINE__);
	}
	
	msgb_free(data);
	return ret;
}

int sccp_send_msg2iuh(Iuh2IuMsg *buf)
{
	iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	iu_log_debug("send_msg2iuh buf len is %d \n", sizeof(Iuh2IuMsg));
	sendto(IuhTipcsock, buf, sizeof(Iuh2IuMsg), 0, (struct sockaddr*)&Iu2Iuh_addr, sizeof(Iu2Iuh_addr));
	//struct sockaddr_un iuh_end;
	//iuh_end.sun_family = AF_UNIX;
	//strncpy(iuh_end.sun_path, SOCKPATH_IUH, UNIX_PATH_MAX);
	//sendto(iuh_fd, buf, sizeof(Iuh2IuMsg), 0, (struct sockaddr*)&iuh_end, sizeof(iuh_end));
}


/* --------------------------------------------------
** book add for assamble SSA message for SCMG
** 2011-12-13
** -------------------------------------------------*/
int sccp_read_scmg(struct msgb *data, unsigned len, void *context)
{
	iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0;
	struct sockaddr_sccp addr_in, addr_out;
	struct msgb *msgb = NULL;
	int buf_len = msgb_l3len(data);
	char buf[256] = {0};
	memcpy(buf, data->l3h, buf_len);

	iu_log_debug("buf[0]=%x,buf[1]=%x\n",buf[0],buf[1]);

	/* if it is an SST message, just change de type to SSA */
	if(buf[0] == 0x03){
		buf[0] = 0x01;
	}


    /* assamble addr_in & addr_out */
	memset(&addr_in, 0, sizeof(struct sockaddr_sccp));
	memset(&addr_out, 0, sizeof(struct sockaddr_sccp));
	
	addr_in.sccp_ssn = 1;/*for sccp management protocol*/
	addr_in.gti_ind = SCCP_TITLE_IND_NONE;
	addr_in.use_poi = 0;

	addr_out.sccp_ssn = 1;/*for sccp management protocol*/
	addr_out.gti_ind = SCCP_TITLE_IND_NONE;
	addr_out.use_poi = 0;

	if(gCnDomain == CS_Domain){
		if(addr_in.use_poi == 1){
		    addr_in.poi[0] = home_gateway_msc.point_code & 0xff;
		    addr_in.poi[1] = (home_gateway_msc.point_code >> 8) & 0xff;
		}
		if(addr_out.use_poi== 1){
			addr_out.poi[0] = global_msc_parameter.point_code & 0xff;
			addr_out.poi[1] = (global_msc_parameter.point_code >> 8) & 0xff;
		}
	}
	else if(gCnDomain == PS_Domain){
		if(addr_in.use_poi == 1){
		    addr_in.poi[0] = home_gateway_sgsn.point_code & 0xff;
		    addr_in.poi[1] = (home_gateway_sgsn.point_code >> 8) & 0xff;
		}
		if(addr_out.use_poi== 1){
		    addr_out.poi[0] = global_sgsn_parameter.point_code & 0xff;
			addr_out.poi[1] = (global_sgsn_parameter.point_code >> 8) & 0xff;
		}
	}
	/* assamble end */

	/* add sccp udt header */
	msgb = sccp_create_udt(0, &addr_in, &addr_out, buf, buf_len);
	if (!msgb) {
		iu_log_debug("Error: create udt error.\n");
		return -1;
	}

	/* call back m3ua function */
	_send_msg(NULL, msgb);
	
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          Search for UeId by IMSI
** INPUT:
**          IMSI
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
** book add, 2011-12-22
*****************************************************/

int sccp_find_ue_by_imsi(char * IMSI)
{
    iu_log_debug("sccp_find_ue_by_imsi\n");
    int UEID = 0;
    int i;
    for(i = 1; i <= IU_UE_MAX_NUM; i++){
        if(IU_UE[i] != NULL){
            if(memcmp(IU_UE[i]->IMSI, IMSI, IMSI_LEN) == 0){
                UEID = i;
                break;
            }
        }
    }

    return UEID;
}



/*
 * writing these packets and expecting a result
 */
int sccp_read_m3ua(struct msgb *data, unsigned len, void *context)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0, tmp_len = 0, present = 0;
	unsigned short procedure_code = 0;
	char* param = NULL, *tmp = NULL;
	user_param_t* user_param = NULL;
	
	Iuh2IuMsg buf;
	RNCPLMN *tmp_ruc = NULL;
	struct sccp_connection *conn = (struct sccp_connection *)context;
	RANAP_PDU_t ranap_pdu;// book add
	//int UEID = 0;

	memset(&buf, 0, sizeof(struct Iuh2Iu));
	if (msgb_l3len(data) < len) {
		/* this should never be reached */
		iu_log_debug("Something horrible happened.. invalid packet..\n");
	}

	buf.RanapMsg.size = msgb_l3len(data);
	memcpy(buf.RanapMsg.RanapMsg, data->l3h, msgb_l3len(data));
	
	ranap_decode_msg_type(buf.RanapMsg.RanapMsg, buf.RanapMsg.size, &present, &procedure_code, &ranap_pdu);
	iu_log_debug("sccp_read_m3ua present %d, procedure_code %d \n",present, procedure_code);

    /* if ranap is initiating message , check procedure code and parse it */
    if(ranap_pdu.present == RANAP_PDU_PR_initiatingMessage){
        InitiatingMessage_t	 *initiatingMessage = &(ranap_pdu.choice.initiatingMessage);
        /* book add for decoding ranap messages */
        switch(procedure_code){
			/* book add for decode IMSI from relocation request message,2011-12-21 */
			case id_RelocationResourceAllocation:{
			#ifdef INBOUND_HANDOVER  
				char imsi[IMSI_LEN] = {0};
				char contextid[CONTEXTID_LEN] = {0};
				CNDomain domain = CS_Domain;
				int ctxid = 0;
				if(!sccp_decode_Relocation_Request(initiatingMessage->value.buf, initiatingMessage->value.size, imsi, &domain)){
					iu_log_error("Error: sccp decode Relocation Request error\n");
				}
				else{
				    int i = 0;
				    int ret = 0;
					int UEID = 0;
				    UEID = sccp_find_ue_by_imsi(imsi);
					/* new ue */
					if(0 == UEID){
						for(i = 1; i <= IU_UE_MAX_NUM; i++) {	
							if(IU_UE[i] == NULL){
								UEID = i;
								break;
							}
						}
						/* calculate the new contextid for conn */
						if(UEID != 0){
							ctxid = UEID * 4;
							memcpy(contextid, ((char*)&(ctxid)+1), CONTEXTID_LEN);
							memcpy(conn->contextid, contextid, CONTEXTID_LEN);
							conn->domainid = domain;
							assign_source_local_reference(conn);
						}
					}
					else{
						if(IU_UE[UEID] == NULL){
							iu_log_error("Error: something wrong!!!\n");
						}
						else{
							memcpy(conn->contextid, IU_UE[UEID]->ContextID, CONTEXTID_LEN);
							conn->domainid = domain;
							assign_source_local_reference(conn);
						}
					}
					buf.UEID = UEID;
					memcpy(buf.imsi, imsi, IU_IMSI_LEN);
				}
				break;
			#endif
			}
            default:
                break;
        }
	}
    
	/* book modify, 2011-12-21 */
	if (conn) {
		memcpy(buf.contextid, conn->contextid, 3);
		buf.SigType = CONNECTION_FROM_CN;
		buf.CnDomain = conn->domainid;
		buf.ranap_type = procedure_code;
		iu_log_debug("sccp_read_m3ua conn->contextid is %x%x%x, Cndomain %d\n",buf.contextid[0], buf.contextid[1], buf.contextid[2], buf.CnDomain);
	}
	else {
		buf.SigType = CONNECTIONLESS_FROM_CN;
		buf.ranap_type = procedure_code;
	}
	
	/* send message to iuh */
	sccp_send_msg2iuh(&buf);
	iu_log_debug("send data to iuh and return ranap data len %d\n", buf.RanapMsg.size);

    return 0;
}



/*****************************************************
** DISCRIPTION:
**          sccp Decode RANAP CnDomain Indicator
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          ContextID
**          errflag
** RETURN:
**          true
**          false
** book add, 2011-12-22
*****************************************************/
int sccp_decode_ranap_cndomainid(const char *buf, const int size, CNDomain *CnDomain, int *errflag)
{
	iu_log_debug("sccp_decode_ranap_cndomainid\n");
    asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	RANAP_CN_DomainIndicator_t *cn_domain_id;
    cn_domain_id = calloc(1, sizeof(RANAP_CN_DomainIndicator_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RANAP_CN_DomainIndicator, (void **)&cn_domain_id, buf, size);
    if(ret.code == RC_FAIL){
        iu_log_error("Error: decode cn_domain_id fail\n");
        *errflag = 1;
    }
    else if(cn_domain_id != NULL){
        asn_INTEGER2long((INTEGER_t *)cn_domain_id, (long*)CnDomain);
    }
    IUH_FREE_OBJECT(cn_domain_id);
    
    return 1;
}



/*****************************************************
** DISCRIPTION:
**          sccp Decode PermanentNAS ue id
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          ContextID
**          errflag
** RETURN:
**          true
**          false
** book add, 2011-12-27
*****************************************************/
int sccp_decode_permanentNAS_ue_id(const char *buf, const int size, char *imsi, int *errflag)
{
	iu_log_debug("sccp_decode_permanentNAS_ue_id\n");
    asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	PermanentNAS_UE_ID_t *permanentnas_ue_id;
    permanentnas_ue_id = calloc(1, sizeof(PermanentNAS_UE_ID_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_PermanentNAS_UE_ID, (void **)&permanentnas_ue_id, buf, size);
    if(ret.code == RC_FAIL){
        iu_log_error("Error: decode permanentnas_ue_id fail\n");
        *errflag = 1;
    }
    else if(permanentnas_ue_id != NULL){
        if(PermanentNAS_UE_ID_PR_iMSI == permanentnas_ue_id->present)
        memcpy(imsi, permanentnas_ue_id->choice.iMSI.buf, permanentnas_ue_id->choice.iMSI.size);
    }
    IUH_FREE_OBJECT(permanentnas_ue_id);
    
    return 1;
}



/*****************************************************
** DISCRIPTION:
**			Decode rab relocation request message
** INPUT:
**			buf
**			buf length
** OUTPUT:
**			IMSI, if IMSI
** RETURN:
**			1		succeed
**			0		fail
** book add, 2011-12-21
*****************************************************/
int sccp_decode_Relocation_Request(const char *buf, const int size, char *imsi, CNDomain *domain)
{
	/* Print Log Information */
	iu_log_debug("###  ---------------------------------\n");
	iu_log_debug("###  Action        :        Decode \n");
	iu_log_debug("###  Type          :        RANAP	RELOCATION-REQUEST\n");
	iu_log_debug("###  Length        :        %d\n", size);
	iu_log_debug("###  ---------------------------------\n");
	
	//iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_decode_Relocation_Request\n");
	RelocationRequest_t *relocation_request;
	relocation_request = calloc(1, sizeof(RelocationRequest_t));
	asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	ANY_t *initial_msg;
	int i;
	
	ret = uper_decode_complete(&code_ctx, &asn_DEF_RelocationRequest, (void **)&relocation_request, buf, size);
	iu_log_debug("decode ret = %d\n",ret.code);
	if(ret.code == RC_FAIL){
		iu_log_error("decode relocation_request fail\n");
		IUH_FREE_OBJECT(relocation_request);
		return 0;
	}
	else if(ret.code == RC_WMORE){
		iu_log_debug("sccp decode relocation request more buf need to be decode \n");
		if(!sccp_decode_Relocation_Request(buf, size, imsi, domain)){
			return 0;
		}
	}
	
	int errflag = 0;
	for(i = 0; i < relocation_request->protocolIEs.list.count; i++){
		initial_msg = &(relocation_request->protocolIEs.list.array[i]->value);
		iu_log_debug("relocation_request IE type is %d \n", relocation_request->protocolIEs.list.array[i]->id);
		switch(relocation_request->protocolIEs.list.array[i]->id){
			case id_CN_DomainIndicator:{
			    // 1 cn_domain_indicator
                if(!sccp_decode_ranap_cndomainid(initial_msg->buf, initial_msg->size, domain, &errflag)){
					iu_log_error("ERROR: decode RANAP CnDomain Id error!\n");
                }
                break;
			}
			case id_Cause:{
				// 2 cause
				break;
			}
			case id_PermanentNAS_UE_ID:{
				// 3 imsi in China & Thailand
				if(!sccp_decode_permanentNAS_ue_id(initial_msg->buf, initial_msg->size, imsi, &errflag)){
				    iu_log_error("ERROR: decode PermanentNAS UE Id error!\n");
				}
				break;
			}
			default:
				break;
				 
		}
	}

	IUH_FREE_OBJECT(relocation_request);
	iu_log_debug("decode relocation request ok\n");
	
	return 1;
}



void sccp_cc_read(struct sccp_connection *connection, struct msgb *msgb, unsigned int len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	/*sccp_read_m3ua(msgb, len, connection->data_ctx);*/
	sccp_read_m3ua(msgb, len, connection);
}

void sccp_cc_state(struct sccp_connection *connection, int old_state)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	iu_log_debug("\n\n!!!!!!!!!!!!!!!!!!!!!!!   connection->connection_state = %d  !!!!!!!!!!!!!!!!!!!!\n\n",connection->connection_state);
	if (connection->connection_state >= SCCP_CONNECTION_STATE_RELEASE_COMPLETE)
		sccp_connection_free(connection);
}

int sccp_accept_ccb(struct sccp_connection *connection, void *user_data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	unsigned int ref = 0;
	ref |= connection->destination_local_reference.octet1 << 24;
	ref |= connection->destination_local_reference.octet2 << 16;
	ref |= connection->destination_local_reference.octet3 <<  8;
	ref = ntohl(ref);

	connection->data_cb = sccp_cc_read;
	connection->state_cb = sccp_cc_state;

	/* accept this */
	return 0;
}
/*
int do_sccp_data1(char *data, int len)
{
	struct sccp_connection *out_con = NULL;
	struct msgb *msgb = NULL;

	out_con = find_sccp_connection_by_sou_loc_ref(&global_reference);
	if (out_con) {
		msgb = sccp_create_dt1(&(out_con->destination_local_reference), data, len);
	}
	else {
		iu_log_debug("++++++++++++++++can not find connect \n");
		start_sccp_con = 1;	
		return -1;
	}

	_send_msg(out_con, msgb);

}
*/
int 
sccp_send_connection_request
(
	Iuh2IuMsg *pri_msg, 
	int len
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0, pri_len = 0;
	struct msgb *msgb = NULL;
	struct sccp_connection *out_con = NULL;
	char *pri_data = NULL;

	/*can find by out_con->domainid and out_con->contextid first*/
	out_con = find_sccp_connection_by_context(pri_msg->contextid, pri_msg->CnDomain);
	if (NULL == out_con) {
		iu_log_debug("out_con = NULL\n");
		out_con = sccp_connection_socket();
		if (!out_con) {
			iu_log_debug("Connection is NULL\n");
			return 1;
		}
	}
	else {
		if (SCCP_CONNECTION_STATE_ESTABLISHED == out_con->connection_state) {
			iu_log_debug("This context is in used contextid is %x %x %x, CnDomain is %d \n", 
				pri_msg->contextid[2], pri_msg->contextid[1], pri_msg->contextid[0], pri_msg->CnDomain);
			return 1;
		}
	}

	ret = sccp_make_pri_msg_from_ranap(pri_msg, &pri_data, &pri_len);
	if (ret) {
		iu_log_debug("bad pri msg from ranap \n");
		return -1;
	}
	
	out_con->state_cb = sccp_cc_state;
	out_con->data_cb = sccp_cc_read;
	out_con->domainid = pri_msg->CnDomain;
	memcpy(out_con->contextid, pri_msg->contextid, 3);
/*	
	msgb = msgb_alloc(pri_len, __func__);
	msgb->l3h = msgb_put(msgb, pri_len);
	memcpy(msgb->l3h, pri_data, pri_len);
*/
	msgb = msgb_alloc(pri_msg->RanapMsg.size, __func__);
	msgb->l3h = msgb_put(msgb, pri_msg->RanapMsg.size);
	memcpy(msgb->l3h, pri_msg->RanapMsg.RanapMsg, pri_msg->RanapMsg.size);

	/* start */
	if (sccp_connection_connect(out_con, &sccp_ssn_ranap, msgb) != 0) {
		iu_log_debug("Binding failed\n");
	}

	free(pri_data);
	return 0;
}

int 
sccp_make_pri_msg_from_ranap
(
	Iuh2IuMsg* msg,
	unsigned char **data,/*out*/
	unsigned int *data_len/*out*/
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	unsigned char *param = NULL;
	user_param_t *user_param = NULL;
	private_head_t *pri_head = NULL;
	unsigned char pri_msg[1500] = {0}, param_id = UE_ID_PT;
	unsigned int pri_len = 0, tmp_len = 0;
	RNCPLMN *tmp_ruc = NULL;
	
	*data = malloc(msg->RanapMsg.size);
	memcpy((*data), msg->RanapMsg.RanapMsg, msg->RanapMsg.size);
	*data_len = msg->RanapMsg.size;
	return 0;

}

/*ranap use it send packet*/
int sccp_send_data2m3ua
(
	unsigned int class,
	unsigned int mess_type,
	Iuh2IuMsg* pri_msg
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0, pri_len = 0;
	struct msgb *msgb = NULL;
	struct sccp_connection *out_con = NULL;
	unsigned char* pri_data = NULL;
	struct sockaddr_sccp addr_in, addr_out;
	
	ret = sccp_make_pri_msg_from_ranap(pri_msg, &pri_data, &pri_len);
	if (ret) {
		iu_log_debug("bad pri msg from ranap \n");
		return -1;
	}
	
	switch (mess_type) {
		case SCCP_MSG_TYPE_CR:
			return 0;
			break;
			
		case SCCP_MSG_TYPE_CC:
			break;

		case SCCP_MSG_TYPE_DT1:
			out_con = find_sccp_connection_by_context(pri_msg->contextid, pri_msg->CnDomain);
			if (NULL == out_con) {
				free(pri_data);
				iu_log_debug("sccp_send_data2m3ua can not find con by contextid and domainid \n");		
				return -1;
			}
			msgb = sccp_create_dt1(&(out_con->destination_local_reference), pri_data, pri_len);
			if (!msgb) {
				free(pri_data);
				return -1;
			}
			
			_send_msg(out_con, msgb);
			break;
			
		case SCCP_MSG_TYPE_UDT:
			memset(&addr_in, 0, sizeof(struct sockaddr_sccp));
			memset(&addr_out, 0, sizeof(struct sockaddr_sccp));
			
			addr_in.sccp_ssn = 142;/*for ranap protocol*/
			addr_in.gti_ind = SCCP_TITLE_IND_NONE;
			addr_in.use_poi = 1;

			
			addr_out.sccp_ssn = 142;/*for ranap protocol*/
			addr_out.gti_ind = SCCP_TITLE_IND_NONE;
			addr_out.use_poi = 1;

            /* book add, 2011-11-10 */
		#ifdef SIMULATOR_CN
		    addr_in.poi[0] = 0x00010101 & 0xff;
		    addr_in.poi[1] = (0x00010101 >> 8) & 0xff;
			addr_out.poi[0] = 0x00010304 & 0xff;
			addr_out.poi[1] = (0x00010304 >> 8) & 0xff;
		#endif
		#ifdef SIGTRAN_PROC
		    addr_in.poi[0] = 0x00010304 & 0xff;
		    addr_in.poi[1] = (0x00010304 >> 8) & 0xff;
			addr_out.poi[0] = 0x00010101 & 0xff;
			addr_out.poi[1] = (0x00010101 >> 8) & 0xff;
		#endif
		#ifndef SIGTRAN_PROC
		#ifndef SIMULATOR_CN
			if(pri_msg->CnDomain == CS_Domain){
			    addr_in.poi[0] = home_gateway_msc.point_code & 0xff;
			    addr_in.poi[1] = (home_gateway_msc.point_code >> 8) & 0xff;
    			addr_out.poi[0] = global_msc_parameter.point_code & 0xff;
    			addr_out.poi[1] = (global_msc_parameter.point_code >> 8) & 0xff;
    		}
    		else if(pri_msg->CnDomain == PS_Domain){
    		    addr_in.poi[0] = home_gateway_sgsn.point_code & 0xff;
			    addr_in.poi[1] = (home_gateway_sgsn.point_code >> 8) & 0xff;
    		    addr_out.poi[0] = global_sgsn_parameter.point_code & 0xff;
    			addr_out.poi[1] = (global_sgsn_parameter.point_code >> 8) & 0xff;
    		}
    	#endif
    	#endif
			
			msgb = sccp_create_udt(0, &addr_in, &addr_out, pri_data, pri_len);
			if (!msgb) {
				free(pri_data);
				return -1;
			}
			
			_send_msg(NULL, msgb);
			break;
			
		default:
			break;
	
	}
	
	free(pri_data);
	return 0;
}


/*-----------------------------------
** handle iu ue info
** book add, 2011-12-22
------------------------------------*/
int sccp_handle_ue_control(Iuh2IuMsg *msg)
{
	int ret = 0;
	iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	if(NULL == msg){
		iu_log_error("ERROR: receive NULL msg from iuh!!\n");
		return -1;
	}
	int UEID = msg->UEID;
	if(0 == UEID){
		iu_log_error("ERROR: receive UEID is 0 !!\n");
		return -1;
	}
	switch(msg->ueop){
		case UE_ADD:
		{
			if(IU_UE[UEID] != NULL){
				iu_log_error("ERROR: ADD, IU UE %d already exists!!\n");
				ret = -1;
			}
			else{
				IU_UE[UEID] = (Iu_UE *)malloc(sizeof(Iu_UE));
				memset(IU_UE[UEID], 0, sizeof(Iu_UE));
				
				IU_UE[UEID]->UEID = UEID;
				memcpy(IU_UE[UEID]->IMSI,msg->imsi,IMSI_LEN);
				memcpy(IU_UE[UEID]->ContextID,msg->contextid,CONTEXTID_LEN);
			}
			iu_log_debug("Add UE for Iu Interface\n");
        	iu_log_debug("---UEID = %d\n",IU_UE[UEID]->UEID);
        	iu_log_debug("---ContextID = %x%x%x\n",IU_UE[UEID]->ContextID[0],IU_UE[UEID]->ContextID[1],IU_UE[UEID]->ContextID[2]);
			break;
		}
		case UE_UPDATE:
		{
			if(IU_UE[UEID] == NULL){
				iu_log_error("ERROR: UPDATE, IU UE %d deos not exist !!\n");
				ret = -1;
			}
			else{
				memcpy(IU_UE[UEID]->IMSI,msg->imsi,IMSI_LEN);
				memcpy(IU_UE[UEID]->ContextID,msg->contextid,CONTEXTID_LEN);
			}
			iu_log_debug("Update UE for Iu Interface\n");
        	iu_log_debug("---UEID = %d\n",IU_UE[UEID]->UEID);
        	iu_log_debug("---ContextID = %x%x%x\n",IU_UE[UEID]->ContextID[0],IU_UE[UEID]->ContextID[1],IU_UE[UEID]->ContextID[2]);
			break;
		}
		case UE_DEL:
		{
			if(IU_UE[UEID] == NULL){
				iu_log_error("ERROR: DEL, IU UE %d deos not exist !!\n");
				ret = -1;
			}
			else{
				free(IU_UE[UEID]);
				IU_UE[UEID] = NULL;
			}
			iu_log_debug("Delete UE for Iu Interface\n");
        	iu_log_debug("---UEID = %d\n",UEID);
			break;
		}
		default:
			break;
	}
	return ret;
}


int sccp_send_iuh_msg2sigtran(char* msg, int len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    if (NULL == msg) {
		return 1;
	}
	int ret = 0;
	Iuh2IuMsg *pri_msg = (Iuh2IuMsg *)msg;

	/* book add for control msg from iuh, 2011-12-22 */
	if(UE_CONTROL_TO_IU == pri_msg->SigType){
		ret = sccp_handle_ue_control(pri_msg);
	}
	else{
		iu_log_debug("cn_domain = %d\n",pri_msg->CnDomain);
		iu_log_debug("contextid=%x%x%x\n",pri_msg->contextid[0],pri_msg->contextid[1],pri_msg->contextid[2]);
		
		unsigned int present = 0, sccp_type = 0;
		unsigned short procedure_code = id_InitialUE_Message; //book modify, 2011-10-08
		
		RANAP_PDU_t ranap_pdu;
		ranap_decode_msg_type(pri_msg->RanapMsg.RanapMsg, pri_msg->RanapMsg.size, &present, &procedure_code, &ranap_pdu);
		iu_log_debug("rcv from iuh present %d, procedure_code %d \n", present, procedure_code);
		if (id_InitialUE_Message == procedure_code) {
			sccp_type = SCCP_MSG_TYPE_CR;
			return sccp_send_connection_request(pri_msg, len);
		}
		else if (IS_CONNECTIONLESS(procedure_code)) {
			sccp_type = SCCP_MSG_TYPE_UDT;
		}
		else {
			sccp_type = SCCP_MSG_TYPE_DT1;
		}

		ret = sccp_send_data2m3ua(0, sccp_type, pri_msg);
	}

	return ret;
}

/*
test_for_send_ranap_packet()
{
	pri_msg_t pri_msg = {.version = 1,
						 .mess_type = 4,
						 .mess_len = 108,
						 .param_ueid = {.param_id = 1,
						 				.choice_type = 1,
						 				.param_len = 7,
						 				.param_data = "acc"},
						 .param_rncid = {.param_id = 2,
						 				.param_len = 7,
						 				.param_data = "aaaaa"},
						 .param_rua_message_type = {.param_id = 3,
						 				.param_len = 4,
						 				.param_data = "aa"},
						 .param_cn_domain = {.param_id = 4,
						 				.param_len = 3,
						 				.param_data = "a"},
						 .param_cause = {.param_id = 5,
						 				.choice_type = 1,
						 				.param_len = 5,
						 				.param_data = "a"},
						 .param_establishment_cause = {.param_id = 6,
						 				.param_len = 3,
						 				.param_data = "c"},
						 .user_protocal_data = {.param_id = 7,
						 						.user_pro_type = 1,
						 						.user_pro_data_len = 71,
						 					   }			
		};
	
	char testranap[] = {0x00,0x13,0x40,0x43,0x00,0x00,0x06,0x00,0x03,0x40,0x01,0x00,
	0x00,0x0f,0x40,0x06,0x00,0x64,0xf0,0x90,0x18,0x07,0x00,0x3a,0x40,0x08,0x00,0x64,
	0xf0,0x90,0x18,0x07,0x00,0x01,0x00,0x10,0x40,0x11,0x10,0x05,0x24,0x71,0x03,0x00,
	0x00,0x00,0x08,0x49,0x06,0x90,0x08,0x40,0x08,0x87,0x72,0x00,0x4f,0x40,0x03,0x01,
	0x5d,0xf4,0x00,0x56,0x40,0x05,0x6f,0xff,0xff,0xff,0xff};
	//0x5d,0xf4,0x00,0x56,0x40,0x05,0x64,0xf0,0x90,0x07,0x0f};

iu_log_debug("test_for_send_ranap_packet initd \n");	
	sccp_send_data2m3ua(0, SCCP_MSG_TYPE_DT1, &pri_msg);
}
*/
/*
 * writing these packets and expecting a result
 */
int sccp_send_to_udp(struct msgb *data, unsigned len, void *context)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	unsigned int l3_len = 0;
	
	l3_len = msgb_l3len(data);
	if (l3_len < len) {
		/* this should never be reached */
		iu_log_debug("Something horrible happened.. invalid packet..\n");
	}

	/*send it to test cn module*/
	m3ua_sendmsg_udp(1, 0, l3_len, &(data->l3h[0]));
	iu_log_debug("sccp_send_to_udp trans data by sigtran2udp buff now \n");
//	recv_msg_from_sccp(2);	
	return 0;
}

void sccp_cc_read2udp(struct sccp_connection *connection, struct msgb *msgb, unsigned int len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	sccp_send_to_udp(msgb, len, connection->data_ctx);
}

int sccp_accept_udp(struct sccp_connection *connection, void *user_data)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	unsigned int ref = 0;
	ref |= connection->destination_local_reference.octet1 << 24;
	ref |= connection->destination_local_reference.octet2 << 16;
	ref |= connection->destination_local_reference.octet3 <<  8;
	ref = ntohl(ref);

	connection->data_cb = sccp_cc_read2udp;
	connection->state_cb = sccp_cc_state;

	/* accept this */
	return 0;
}

/*when receive msg from udp packet or from ranap, use it*/
int sccp_send_data2GW
(
	unsigned char* data,
	unsigned int data_len
)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0;
	struct sccp_connection *des_out_con = NULL;
	struct msgb *msgb = NULL;

	des_out_con = find_sccp_connection_by_sou_loc_ref(&global_reference);
	if (des_out_con) {
		msgb = sccp_create_dt1(&(des_out_con->destination_local_reference), data, data_len);
	}
	else {
		iu_log_debug("++++++++++++++++can not find connect \n");
		start_sccp_con = 1;	
	}

	if (!msgb) {
		return -1;
	}

	_send_msg(des_out_con, msgb);

	return 0;
}

//void sccp_local_socket_iu2iuh_init()
void sccp_iu2iuh_tipc_socket_init(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	iuh_fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
	Iu2Iuh_addr.family = AF_TIPC;
	Iu2Iuh_addr.addrtype = TIPC_ADDR_NAME;
	Iu2Iuh_addr.addr.name.name.type = IUH_TIPC_SERVER_TYPE + \ 
		(slotid-1)*IUH_MAX_INS_NUM*2 + vrrid + local;
	Iu2Iuh_addr.addr.name.name.instance = FEMTO_SERVER_BASE_INST;
	Iu2Iuh_addr.addr.name.domain = 0;
	IuhTipcsock = socket(AF_TIPC, SOCK_RDM, 0);
	if (IuhTipcsock==-1) {
		perror("socket()");
	}
}

/*use different for ranapproxy*/
void ranap_sccp_system(void)
{	
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	ranap_pduInit();
	sccp_system_init(sccp_write_to_m3ua, NULL);
	sccp_set_read(&sccp_ssn_ranap, sccp_read_m3ua, NULL);
	sccp_connection_set_incoming(&sccp_ssn_ranap, sccp_accept_ccb, NULL);
	/* book add for sccp management init */
	sccp_set_read(&sccp_ssn_management, sccp_read_scmg, NULL);
	sccp_connection_set_incoming(&sccp_ssn_management, sccp_accept_ccb, NULL);
	//sccp_local_socket_iu2iuh_init();
	sccp_iu2iuh_tipc_socket_init();
}
/*use different for sigtran2udp
void ranap_udp_system(void)
{
	ranap_pduInit();
	sccp_system_init(sccp_write_to_m3ua, NULL);
	sccp_set_read(&sccp_ssn_ranap, sccp_send_to_udp, NULL);
	sccp_connection_set_incoming(&sccp_ssn_ranap, sccp_accept_udp, NULL);
}*/

void ranap_udp_system(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	ranap_pduInit();
	sccp_system_init(sccp_write_to_m3ua, NULL);
	sccp_set_read(&sccp_ssn_ranap, sccp_send_to_udp, NULL);
	sccp_connection_set_incoming(&sccp_ssn_ranap, sccp_accept_ccb, NULL);
}

static __attribute__((constructor)) void on_dso_load(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	tall_sccp_ctx = talloc_named_const(NULL, 1, "sccp");
}

static __attribute__((destructor)) void on_dso_unload(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	talloc_report_full(tall_sccp_ctx, stderr);
}

void sccp_set_log_area(int log_area)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	DSCCP = log_area;
}
