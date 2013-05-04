#ifndef __PDU_ARRAY_H__
#define __PDU_ARRAY_H__

#include <asn_application.h>
#include <asn_internal.h>
//#include <constr_TYPE.h>
#include <constr_SEQUENCE.h>
#include <ANY.h>

#include "ranapConstants.h"

#define MAX_PDU 256
typedef enum ranapParentPdu{
	INITIATING_MESSAGE,
	SUCCESSFUL_OUTCOME,
	UNSUCCESSFUL_OUTCOME,
	OUTCOME,
	PROTOCOL_IES,
	PARENT_PDU_NONE
} ranap_parent_pdu_e;

typedef enum protocoType {
	RANAP,
	RUA,
	HNBAP
}protocol_type_e;

extern asn_TYPE_descriptor_t * initiatingValuePdu[MAX_PDU];
extern asn_TYPE_descriptor_t * successfulValuePdu[MAX_PDU];
extern asn_TYPE_descriptor_t * unsuccessfulValuePdu[MAX_PDU];
extern asn_TYPE_descriptor_t * outcomeValuePdu[MAX_PDU];
extern asn_TYPE_descriptor_t * ranapIesValuePdu[MAX_PDU];

extern asn_TYPE_descriptor_t ** ranapANYValuePdu[PARENT_PDU_NONE];

#define DECODE_FAILED do{\
		asn_dec_rval_t tmp_error;				\
		tmp_error.code = RC_FAIL;				\
		tmp_error.consumed = 0; 				\
		return tmp_error;						\
	}while(0)
	
#define	ENCODE_FAILED do {					\
		asn_enc_rval_t tmp_error;				\
		tmp_error.encoded = -1; 					\
		tmp_error.structure_ptr = sptr; 			\
		return tmp_error;					\
	} while(0)


extern asn_dec_rval_t
SEQUENCE_decode_uper_for_RANAP(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
	asn_per_constraints_t *constraints, void **sptr, asn_per_data_t *pd);

extern asn_enc_rval_t
SEQUENCE_encode_uper_for_RANAP(asn_TYPE_descriptor_t *td,
	asn_per_constraints_t *constraints, void *sptr, asn_per_outp_t *po);

extern void
SEQUENCE_free_for_RANAP(asn_TYPE_descriptor_t *td, void *sptr, int contents_only) ;


asn_dec_rval_t ANY_decode_per_sub_PDU
(
	asn_codec_ctx_t *opt_codec_ctx, 
	asn_per_constraints_t *constraints, 
	void **sptr, 
	asn_per_data_t *pd,
	char * parentTdName,
	unsigned int id,
	protocol_type_e flag   /* RANAP, RUA, or HNBAP*/
);

asn_enc_rval_t ANY_encode_per_sub_PDU
(	
	asn_per_constraints_t *constraints, 
	void *sptr, 
	asn_per_outp_t *po,
	char * parentTdName,
	unsigned int id,
	protocol_type_e flag	
);

asn_dec_rval_t ANY_free_sub_PDU(void *sptr, char * parentTdName, unsigned int id, int flag);





#endif
