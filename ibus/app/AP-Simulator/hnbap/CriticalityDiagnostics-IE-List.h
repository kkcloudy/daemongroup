/*
 * Generated by asn1c-0.9.23 (http://lionet.info/asn1c)
 * From ASN.1 module "HNBAP-IEs"
 * 	found in "../asn/HNBAP-IEs.asn"
 */

#ifndef	_CriticalityDiagnostics_IE_List_H_
#define	_CriticalityDiagnostics_IE_List_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include "Criticality.h"
#include "ProtocolIE-ID.h"
#include "TypeOfError.h"
#include <constr_SEQUENCE.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
/* auto changed by perl script */
/*struct ;*/
typedef struct Member_CriticalityDiagnostics_IE_List {
		Criticality_t	 iECriticality;
		ProtocolIE_ID_t	 iE_ID;
		TypeOfError_t	 typeOfError;
/* auto changed by perl script */
		struct 	 ProtocolExtensionContainer_144P0 *iE_Extensions ;	/* OPTIONAL */
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
}Member_CriticalityDiagnostics_IE_List_t;

/* CriticalityDiagnostics-IE-List */
typedef struct CriticalityDiagnostics_IE_List {
	A_SEQUENCE_OF(Member_CriticalityDiagnostics_IE_List_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CriticalityDiagnostics_IE_List_t;

/* Implementation */
extern asn_TYPE_descriptor_t HNBAP_asn_DEF_CriticalityDiagnostics_IE_List;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ProtocolExtensionContainer.h"

#endif	/* _CriticalityDiagnostics_IE_List_H_ */
#include <asn_internal.h>
