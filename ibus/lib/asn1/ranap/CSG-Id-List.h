/*
 * Generated by asn1c-0.9.23 (http://lionet.info/asn1c)
 * From ASN.1 module "RANAP-IEs"
 * 	found in "RANAP-IEs.asn"
 */

#ifndef	_CSG_Id_List_H_
#define	_CSG_Id_List_H_


#include <asn_application.h>

/* Including external dependencies */
#include "CSG-Id.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSG-Id-List */
typedef struct CSG_Id_List {
	A_SEQUENCE_OF(CSG_Id_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CSG_Id_List_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CSG_Id_List;

#ifdef __cplusplus
}
#endif

#endif	/* _CSG_Id_List_H_ */
#include <asn_internal.h>
