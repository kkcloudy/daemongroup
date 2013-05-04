#include "Iuh_Encode.h"
#include "BIT_STRING.h"
#include "OCTET_STRING.h"
#include "INTEGER.h"
#include "LAC.h"
#include "RAC.h"
#include "SAC.h"
#include "CSG-ID.h"
#include "RNC-ID.h"
#include "HNBAP-Cause.h"
#include "RUA-Cause.h"
#include "BackoffTimer.h"
#include "UE-Identity.h"
#include "Context-ID.h"
#include "CN-DomainIndicator.h"
#include "Establishment-Cause.h"
#include "RANAP-Message.h"
#include "ProtocolIE-Container.h"
#include "HNBAP-ProtocolIE-Container.h"
#include "HNBRegisterAccept.h"
#include "HNBRegisterReject.h"
#include "UERegisterAccept.h"
#include "UERegisterReject.h"
#include "HNBAP-PDU.h"
#include "Connect.h"
#include "Disconnect.h"
#include "DirectTransfer.h"
#include "ConnectionlessTransfer.h"
#include "RUA-PDU.h"
#include "iuh/Iuh.h"
#include "Iuh_log.h"
#include "Iuh_DBus_handler.h"
#include "RUA-CN-DomainIndicator.h"
#include "RUA-ErrorIndication.h"
#include "HNBAP-ErrorIndication.h"
#include "HNBDe-Register.h"
#include "UEDe-Register.h"
#include "UERegisterRequest.h"
#include "UE-Capabilities.h"
#include "Registration-Cause.h"

#include "RANAP-PDU.h"
#include "RANAP-ProtocolIE-Container.h"
#include "ResetAcknowledge.h"
#include "RANAP-CN-DomainIndicator.h"
#include "ResetResource.h"
#include "IuSignallingConnectionIdentifier.h"
#include "RANAP-Cause.h"
#include "ResetResourceList.h"
#include "ResetResourceItem.h"
#include "IuSigConId-IE-ContainerList.h"



/*****************************************************
** DISCRIPTION:
**          Encode buf to bit string
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int BIT_STRING_fromBuf(BIT_STRING_t *st, const char *str, int len, int unused) 
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"BIT_STRING_fromBuf\n");
	void *buf;

	if(st == 0 || (str == 0 && len)) {
	    iuh_syslog_err("invalid paras \n");
		return -1;
	}

	/*
	 * Clear the OCTET STRING.
	 */
	if(str == NULL) {
		FREEMEM(st->buf);
		st->buf = 0;
		st->size = 0;
		return 0;
	}

	/* Determine the original string size, if not explicitly given */
	if(len < 0)
		len = strlen(str);

	/* Allocate and fill the memory */
	buf = MALLOC(len + 1);
	if(buf == NULL)
		return -1;

	memcpy(buf, str, len);
	((uint8_t *)buf)[len] = '\0';	/* Couldn't use memcpy(len+1)! */
	FREEMEM(st->buf);
	st->buf = (uint8_t *)buf;
	st->size = len;
	st->bits_unused = unused;

	return 0;
}


/*****************************************************
** DISCRIPTION:
**          encode member cause from reject cause struct
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_cause(Hnb_Member_ProtocolIE_Container_t *member_cause, HNBAPCause *cause)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_cause\n");
    HNBAP_Cause_t reject_cause;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_cause, 0, sizeof(Hnb_Member_ProtocolIE_Container_t));
	member_cause->id = HNBAP_id_Cause;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_ignore);
	member_cause->criticality = (Criticality_t)integer;

    memset(&integer,0,sizeof(INTEGER_t));

    switch( cause->present) {
        case HNBAP_Cause_PR_radioNetwork: {
            asn_ulong2INTEGER(&integer, cause->choice.radioNetwork);
            reject_cause.present = cause->present;
        	reject_cause.choice.radioNetwork = (ENUMERATED_t)integer;
            break;
        }
        case HNBAP_Cause_PR_transport: {
            asn_ulong2INTEGER(&integer, cause->choice.transport);
            reject_cause.present = cause->present;
        	reject_cause.choice.transport = (ENUMERATED_t)integer;
            break;
        }
        case HNBAP_Cause_PR_protocol: {
            asn_ulong2INTEGER(&integer, cause->choice.protocol);
            reject_cause.present = cause->present;
        	reject_cause.choice.protocol = (ENUMERATED_t)integer;
            break;
        }
        case HNBAP_Cause_PR_misc: {
            asn_ulong2INTEGER(&integer, cause->choice.misc);
            reject_cause.present = cause->present;
        	reject_cause.choice.misc = (ENUMERATED_t)integer;
            break;
        }
        default: {
            break;
        }
    }

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_Cause,(void *)&reject_cause, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_cause->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode backoff timer
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_backoff_timer(struct Hnb_Member_ProtocolIE_Container *member_backoff_timer)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_backoff_timer\n");
    BackoffTimer_t backoff_timer;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_backoff_timer, 0, sizeof(struct Hnb_Member_ProtocolIE_Container));
	member_backoff_timer->id = HNBAP_BackoffTimer;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_backoff_timer->criticality = (Criticality_t)integer;
	
	backoff_timer = 300;

	ret = uper_encode_to_buffer(&asn_DEF_BackoffTimer,(void *)&backoff_timer, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_backoff_timer->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Assemble Hnb register reject message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleRegisterReject(HNBAPCause *cause, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  HNB-REGISTER-REJECT\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleRegisterReject\n");
    HNBAP_PDU_t hnbap_pdu;
	HNBRegisterReject_t hnbap_reg_reject;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_cause;
	UnsuccessfulOutcome_t *unsuccess_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_unsuccessfulOutcome;

	unsuccess_msg = (UnsuccessfulOutcome_t *)&(hnbap_pdu.choice.unsuccessfulOutcome);
	unsuccess_msg->procedureCode = HNBRegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	unsuccess_msg->criticality = (Criticality_t)integer;
	memset(&hnbap_reg_reject, 0, sizeof(HNBRegisterReject_t));
	ie_container = &hnbap_reg_reject.protocolIEs;

	ret.encoded = init_cause(&member_cause, cause);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cause);

	if(cause->present == HNBAP_Cause_PR_radioNetwork){
	    if(cause->choice.radioNetwork == HNBAP_CauseRadioNetwork_overload){
	        struct Hnb_Member_ProtocolIE_Container member_backoff_timer;
	        ret.encoded = init_backoff_timer(&member_backoff_timer);
	        if(ret.encoded == -1){
        	    return Iuh_FALSE;
        	}
        	ASN_SEQUENCE_ADD(&(ie_container->list), &member_backoff_timer);
	    }
	}

	ret = uper_encode_to_buffer(&asn_DEF_HNBRegisterReject,(void *)&hnbap_reg_reject, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(unsuccess_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Encode rncid
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_rnc_id(struct Hnb_Member_ProtocolIE_Container *member_rnc_id, long rncid)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_rnc_id\n");
    RNC_ID_t rnc_id;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_rnc_id, 0, sizeof(struct Hnb_Member_ProtocolIE_Container));
	member_rnc_id->id = HNBAP_RNC_ID;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_rnc_id->criticality = (Criticality_t)integer;
	
	rnc_id = rncid;

	ret = uper_encode_to_buffer(&asn_DEF_RNC_ID,(void *)&rnc_id, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_rnc_id->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Assemble Hnb register accept message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleRegisterAccept(int HnbIndex, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  HNB-REGISTER-ACCEPT\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleRegisterAccept\n");
	HNBAP_PDU_t hnbap_pdu;
	HNBRegisterAccept_t hnbap_reg_accert;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_rnc_id;
	SuccessfulOutcome_t *success_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_successfulOutcome;

	success_msg = (SuccessfulOutcome_t *)&(hnbap_pdu.choice.successfulOutcome);
	success_msg->procedureCode = HNBRegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_ignore);
	success_msg->criticality = (Criticality_t)integer;
	memset(&hnbap_reg_accert, 0, sizeof(HNBRegisterAccept_t));
	ie_container = &hnbap_reg_accert.protocolIEs;

	ret.encoded = init_rnc_id(&member_rnc_id, gRncId);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}

	ASN_SEQUENCE_ADD(&(ie_container->list), &member_rnc_id);

	ret = uper_encode_to_buffer(&asn_DEF_HNBRegisterAccept,(void *)&hnbap_reg_accert, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(success_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	
	msgPtr->size = (ret.encoded+7)/8;

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Encode ue identity
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_ue_identity(struct Hnb_Member_ProtocolIE_Container *member_ue_identity, int UEID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_ue_identity\n");
    UE_Identity_t ue_identity;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_ue_identity, 0, sizeof(struct Hnb_Member_ProtocolIE_Container));
	member_ue_identity->id = HNBAP_UE_Identity;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_ue_identity->criticality = (Criticality_t)integer;

    UE_IDENTITY_S *ueIdentity = &(IUH_HNB_UE[UEID]->UE_Identity);
	ue_identity.present = ueIdentity->present;
	switch(ue_identity.present){
	    case UE_Identity_PR_iMSI: {
	        OCTET_STRING_fromBuf(&(ue_identity.choice.iMSI), IUH_HNB_UE[UEID]->IMSI, IMSI_LEN);
	        break;
	    }
	    case UE_Identity_PR_tMSILAI: {
	        BIT_STRING_fromBuf(&(ue_identity.choice.tMSILAI.tMSI), (const char *)&(ueIdentity->choice.tmsilai.tmsi), 4, 0);
	        OCTET_STRING_fromBuf(&(ue_identity.choice.tMSILAI.lAI.pLMNID), ueIdentity->choice.tmsilai.lai.plmnid, PLMN_LEN);
	        OCTET_STRING_fromBuf(&(ue_identity.choice.tMSILAI.lAI.lAC), ueIdentity->choice.tmsilai.lai.lac, LAC_LEN);
	        break;
	    }
	    case UE_Identity_PR_pTMSIRAI: {
	        BIT_STRING_fromBuf(&(ue_identity.choice.pTMSIRAI.pTMSI), ueIdentity->choice.ptmsirai.ptmsi, 4, 0);
	        OCTET_STRING_fromBuf(&(ue_identity.choice.pTMSIRAI.rAI.lAI.pLMNID), ueIdentity->choice.ptmsirai.rai.lai.plmnid, PLMN_LEN);
	        OCTET_STRING_fromBuf(&(ue_identity.choice.pTMSIRAI.rAI.lAI.lAC), ueIdentity->choice.ptmsirai.rai.lai.lac, LAC_LEN);
	        OCTET_STRING_fromBuf(&(ue_identity.choice.pTMSIRAI.rAI.rAC), ueIdentity->choice.ptmsirai.rai.rac, RAC_LEN);
	        break;
	    }
	    case UE_Identity_PR_iMEI: {
	        BIT_STRING_fromBuf(&(ue_identity.choice.iMEI), ueIdentity->choice.imei, 8, 4);
	        break;
	    }
	    case UE_Identity_PR_eSN: {
	        BIT_STRING_fromBuf(&(ue_identity.choice.eSN), ueIdentity->choice.esn, 4, 0);
	        break;
	    }
	    case UE_Identity_PR_iMSIDS41: {
	        OCTET_STRING_fromBuf(&(ue_identity.choice.iMSIDS41), ueIdentity->choice.imsids41, strlen(ueIdentity->choice.imsids41));
	        break;
	    }
	    case UE_Identity_PR_iMSIESN: {
	        OCTET_STRING_fromBuf(&(ue_identity.choice.iMSIESN.iMSIDS41), ueIdentity->choice.imsiesn.imsids41, strlen(ueIdentity->choice.imsiesn.imsids41));
	        BIT_STRING_fromBuf(&(ue_identity.choice.iMSIESN.eSN), (const char *)&(ueIdentity->choice.imsiesn.esn), 4, 0);
	        break;
	    }
	    case UE_Identity_PR_tMSIDS41: {
	        OCTET_STRING_fromBuf(&(ue_identity.choice.tMSIDS41), ueIdentity->choice.tmsids41, strlen(ueIdentity->choice.tmsids41));
	        break;
	    }
	    default:
	        break;
	}

	ret = uper_encode_to_buffer(&asn_DEF_UE_Identity, (void *)&ue_identity, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_ue_identity->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode context id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_context_id(struct Hnb_Member_ProtocolIE_Container *member_ue_identity, int UEID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_context_id\n");
    Context_ID_t context_id;
    memset(&context_id,0,sizeof(Context_ID_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_ue_identity, 0, sizeof(struct Hnb_Member_ProtocolIE_Container));
	member_ue_identity->id = HNBAP_Context_ID;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_ue_identity->criticality = (Criticality_t)integer;
    //unsigned int cntxtId = IUH_HNB_UE[UEID]->context_id;
    //char ctx[CONTEXTID_LEN] = {0};
    //memcpy(ctx, ((char*)&cntxtId+1), CONTEXTID_LEN);
	BIT_STRING_fromBuf(&context_id, (char*)IUH_HNB_UE[UEID]->context_id_str, CONTEXTID_LEN, 0);
	
	ret = uper_encode_to_buffer(&asn_DEF_Context_ID,(void *)&context_id, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_ue_identity->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode registration cause
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_registration_cause(struct Hnb_Member_ProtocolIE_Container *member_registration_cause, int rgstcause)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_registration_cause\n");
    Registration_Cause_t registration_cause;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_registration_cause, 0, sizeof(Hnb_Member_ProtocolIE_Container_t));
	member_registration_cause->id = HNBAP_Registration_Cause;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_ignore);
	member_registration_cause->criticality = (Criticality_t)integer;

	memset(&registration_cause, 0, sizeof(Registration_Cause_t));
    asn_ulong2INTEGER(&registration_cause, rgstcause);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_Cause,(void *)&registration_cause, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_registration_cause->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode ue capabilities
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_ue_capabilities(struct Hnb_Member_ProtocolIE_Container *member_ue_capabilities, struct UECapability *ueCapability)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_ue_capabilities\n");
    UE_Capabilities_t ue_capabilities;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_ue_capabilities, 0, sizeof(Hnb_Member_ProtocolIE_Container_t));
	member_ue_capabilities->id = HNBAP_UE_Capabilities;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_ue_capabilities->criticality = (Criticality_t)integer;
	
	/* add access_stratum_release_indicator */
	memset(&integer,0,sizeof(INTEGER_t));
    asn_ulong2INTEGER(&integer, ueCapability->accStrRelIndicator);
	ue_capabilities.access_stratum_release_indicator = (ENUMERATED_t)integer;
	/* add csg_indicator */
	memset(&integer,0,sizeof(INTEGER_t));
    asn_ulong2INTEGER(&integer, ueCapability->csgIndicator);
	ue_capabilities.csg_indicator = (ENUMERATED_t)integer;

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_Cause,(void *)&ue_capabilities, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n111ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_ue_capabilities->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}




/*****************************************************
** DISCRIPTION:
**          Assemble ue register reject
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleUERegisterReject(int UEID, HNBAPCause *cause, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-REGISTER-REJECT\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleUERegisterReject\n");
    HNBAP_PDU_t hnbap_pdu;
	UERegisterReject_t ue_reg_reject;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_ue_identity;
	struct Hnb_Member_ProtocolIE_Container member_cause;
	UnsuccessfulOutcome_t *unsuccess_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_unsuccessfulOutcome;

	unsuccess_msg = (UnsuccessfulOutcome_t *)&(hnbap_pdu.choice.unsuccessfulOutcome);
	unsuccess_msg->procedureCode = UERegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	unsuccess_msg->criticality = (Criticality_t)integer;
	memset(&ue_reg_reject, 0, sizeof(HNBRegisterReject_t));
	ie_container = &ue_reg_reject.protocolIEs;

    ret.encoded = init_ue_identity(&member_ue_identity, UEID);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_identity);
    
    ret.encoded = init_cause(&member_cause, cause);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cause);

	ret = uper_encode_to_buffer(&asn_DEF_UERegisterReject,(void *)&ue_reg_reject, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(unsuccess_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble ue register accept message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleUERegisterAccept(int UEID, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-REGISTER-ACCEPT\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleUERegisterAccept\n");
    if(IUH_HNB_UE[UEID] == NULL) return Iuh_FALSE;
    
    HNBAP_PDU_t hnbap_pdu;
	UERegisterAccept_t ue_reg_accert;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_ue_ideneity;
	struct Hnb_Member_ProtocolIE_Container member_ue_context_id;
	SuccessfulOutcome_t *success_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_successfulOutcome;

	success_msg = (SuccessfulOutcome_t *)&(hnbap_pdu.choice.successfulOutcome);
	success_msg->procedureCode = UERegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	success_msg->criticality = (Criticality_t)integer;
	memset(&ue_reg_accert, 0, sizeof(UERegisterAccept_t));
	ie_container = &ue_reg_accert.protocolIEs;

	ret.encoded = init_ue_identity(&member_ue_ideneity, UEID);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}

	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_ideneity);

	ret.encoded = init_context_id(&member_ue_context_id, UEID);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}

	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_context_id);

	ret = uper_encode_to_buffer(&asn_DEF_UERegisterAccept,(void *)&ue_reg_accert, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(success_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Encode cn domain id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_cn_domain_id(Rua_Member_ProtocolIE_Container_t *member_cn_domainId, Iuh2IuMsg *sigMsg)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_cn_domain_id\n");
    RUA_CN_DomainIndicator_t cn_domainId;
    memset(&cn_domainId, 0, sizeof(RUA_CN_DomainIndicator_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
	asn_ulong2INTEGER(&cn_domainId, sigMsg->CnDomain);
    
    memset(member_cn_domainId, 0, sizeof(Rua_Member_ProtocolIE_Container_t));
	member_cn_domainId->id = RUA_CN_DomainIndicator;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_cn_domainId->criticality = (Criticality_t)integer;
    
	ret = uper_encode_to_buffer(&asn_DEF_RUA_CN_DomainIndicator,(void *)&cn_domainId, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\ncn_domain_id ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_cn_domainId->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
	xer_fprint(stdout, &asn_DEF_RUA_CN_DomainIndicator, &cn_domainId);
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode rua ue context id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_rua_context_id(Rua_Member_ProtocolIE_Container_t *member_ue_identity, int UEID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_rua_context_id\n");
    Context_ID_t context_id;
    memset(&context_id,0,sizeof(Context_ID_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_ue_identity, 0, sizeof(Rua_Member_ProtocolIE_Container_t));
	member_ue_identity->id = RUA_Context_ID;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_ue_identity->criticality = (Criticality_t)integer;
    //unsigned int cntxtId = IUH_HNB_UE[UEID]->context_id;
    //char ctx[CONTEXTID_LEN] = {0};
    //memcpy(ctx, ((char*)&cntxtId+1), CONTEXTID_LEN);
	BIT_STRING_fromBuf(&context_id, (char*)IUH_HNB_UE[UEID]->context_id_str, CONTEXTID_LEN, 0);
	
	ret = uper_encode_to_buffer(&asn_DEF_Context_ID,(void *)&context_id, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\ncontextid ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_ue_identity->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode establishment cause
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_establishment_cause(Rua_Member_ProtocolIE_Container_t *member_establishment_cause, Iuh2IuMsg *sigMsg)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_establishment_cause\n");
    Establishment_Cause_t establishment_cause;
    memset(&establishment_cause, 0, sizeof(Establishment_Cause_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	asn_ulong2INTEGER(&establishment_cause, sigMsg->EstabCause);
    
    memset(member_establishment_cause, 0, sizeof(Rua_Member_ProtocolIE_Container_t));
	member_establishment_cause->id = RUA_Establishment_Cause;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_establishment_cause->criticality = (Criticality_t)integer;

	ret = uper_encode_to_buffer(&asn_DEF_Establishment_Cause,(void *)&establishment_cause, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\nEstablishment_Cause ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_establishment_cause->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
	xer_fprint(stdout, &asn_DEF_Establishment_Cause, &establishment_cause);
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode ranap message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_ranap_msg(Rua_Member_ProtocolIE_Container_t *member_ranap_msg, Iuh2IuMsg *sigMsg)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_ranap_msg\n");
    RANAP_Message_t ranap_msg;
    memset(&ranap_msg, 0, sizeof(RANAP_Message_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[RUA_BUFFER_SIZE] = {0};

	OCTET_STRING_fromBuf((OCTET_STRING_t*)&ranap_msg, sigMsg->RanapMsg.RanapMsg, sigMsg->RanapMsg.size);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\nRanapMsg.size = %d\n",sigMsg->RanapMsg.size);
    
    memset(member_ranap_msg, 0, sizeof(Rua_Member_ProtocolIE_Container_t));
	member_ranap_msg->id = RUA_RANAP_Message;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_ranap_msg->criticality = (Criticality_t)integer;

	ret = uper_encode_to_buffer(&asn_DEF_RANAP_Message,(void *)&ranap_msg, ie_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\nranap_msg ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_ranap_msg->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
	xer_fprint(stdout, &asn_DEF_RANAP_Message, &ranap_msg);
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode rua cause
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

int init_rua_cause(Rua_Member_ProtocolIE_Container_t *member_cause, Iuh2IuMsg *sigMsg)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_rua_cause\n");
    RUA_Cause_t disconn_cause;
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_cause, 0, sizeof(Rua_Member_ProtocolIE_Container_t));
	member_cause->id = RUA_Cause;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_ignore);
	member_cause->criticality = (Criticality_t)integer;
	
	memset(&integer,0,sizeof(INTEGER_t));

    switch(sigMsg->cause.present) {
        case Cause_PR_radioNetwork_Rua: {
			disconn_cause.present = sigMsg->cause.present;
            asn_ulong2INTEGER(&integer, sigMsg->cause.choice.radioNetwork);
        	disconn_cause.choice.radioNetwork = (ENUMERATED_t)integer;
            break;
        }
        case Cause_PR_transport: {
            disconn_cause.present = sigMsg->cause.present;
			asn_ulong2INTEGER(&integer, sigMsg->cause.choice.transport);
        	disconn_cause.choice.transport = (ENUMERATED_t)integer;
        }
        case Cause_PR_protocol: {
            
            disconn_cause.present = sigMsg->cause.present;
			asn_ulong2INTEGER(&integer, sigMsg->cause.choice.protocol);
        	disconn_cause.choice.protocol = (ENUMERATED_t)integer;
            break;
        }
        case Cause_PR_misc: {
            
            disconn_cause.present = sigMsg->cause.present;
			asn_ulong2INTEGER(&integer, sigMsg->cause.choice.misc);
        	disconn_cause.choice.misc = (ENUMERATED_t)integer;
            break;
        }
        default: {
			sigMsg->cause.present = Cause_PR_radioNetwork_Rua;
			sigMsg->cause.choice.radioNetwork = Normal;
			
			iuh_syslog_debug_debug(IUH_DEFAULT,"\ncause reason normal\n");
			disconn_cause.present = 1;   //radio network
			asn_ulong2INTEGER(&integer, sigMsg->cause.choice.radioNetwork);
        	disconn_cause.choice.radioNetwork = (ENUMERATED_t)integer;
            break;
        }
    }

	ret = uper_encode_to_buffer(&RUA_asn_DEF_Cause,(void *)&disconn_cause, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\nCause ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_cause->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode ranap cn domain id
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**        
** book add, 2011-12-19
*****************************************************/

int init_ranap_cn_domain_id(Ranap_Member_ProtocolIE_Container_t *member_cn_domainId, int cnDomainId)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_ranap_cn_domain_id\n");
    RANAP_CN_DomainIndicator_t cn_domainId;
    memset(&cn_domainId, 0, sizeof(RANAP_CN_DomainIndicator_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
	asn_ulong2INTEGER(&cn_domainId, cnDomainId);
    
    memset(member_cn_domainId, 0, sizeof(Ranap_Member_ProtocolIE_Container_t));
	member_cn_domainId->id = id_CN_DomainIndicator;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_cn_domainId->criticality = (Criticality_t)integer;
    
	ret = uper_encode_to_buffer(&asn_DEF_RANAP_CN_DomainIndicator,(void *)&cn_domainId, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"\ncn_domain_id ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_cn_domainId->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
	xer_fprint(stdout, &asn_DEF_RANAP_CN_DomainIndicator, &cn_domainId);
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**          Encode Ranap Cause
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
** book add, 2011-12-19
*****************************************************/

int init_ranap_cause(Ranap_Member_ProtocolIE_Container_t *member_cause, RANAPCause *cause)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"init_ranap_cause\n");
    Ranap_Cause_t reset_cause;
	memset(&reset_cause, 0, sizeof(Ranap_Cause_t));
    asn_enc_rval_t ret;
    INTEGER_t integer;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
    
    memset(member_cause, 0, sizeof(Ranap_Member_ProtocolIE_Container_t));
	member_cause->id = id_Cause;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_ignore);
	member_cause->criticality = (Criticality_t)integer;

    switch(cause->present) {
        case RANAP_Cause_radioNetwork_Ranap: {
			reset_cause.present = cause->present;
        	reset_cause.choice.radioNetwork = cause->choice.radioNetwork;
            break;
        }
        case RANAP_Cause_transmissionNetwork: {
            reset_cause.present = cause->present;
        	reset_cause.choice.transmissionNetwork = cause->choice.transmissionNetwork;
			break;
        }
        case RANAP_Cause_protocol_Ranap: {
            reset_cause.present = cause->present;
        	reset_cause.choice.protocol = cause->choice.protocol;
            break;
        }
        case RANAP_Cause_misc_Ranap: {
            reset_cause.present = cause->present;
        	reset_cause.choice.misc = cause->choice.misc;
            break;
        }
        default: {
			reset_cause.present = RANAP_Cause_radioNetwork_Ranap;   //radio network
        	reset_cause.choice.radioNetwork = cause->choice.radioNetwork;
            break;
        }
    }
	iuh_syslog_debug_debug(IUH_DEFAULT,"reset_cause.present = %d\n",reset_cause.present);
	iuh_syslog_debug_debug(IUH_DEFAULT,"reset_cause.choice.radioNetwork = %d\n",reset_cause.choice.radioNetwork);
	
	ret = uper_encode_to_buffer(&RANAP_asn_DEF_Cause,(void *)&reset_cause, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"Cause ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_cause->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
    return ret.encoded;
}


/*****************************************************
** DISCRIPTION:
**			Encode ranap reset resource set list
** INPUT:
**			
** OUTPUT:
**			
** RETURN:
**			
** book add, 2011-12-19
*****************************************************/

int init_ranap_reset_resource_list(Ranap_Member_ProtocolIE_Container_t *member_reset_resource_list, const int HNBID, CNDomain cn_domain_id)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"init_ranap_reset_resource_list\n");
	iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
	ResetResourceList_t reset_resource_list;
	memset(&reset_resource_list, 0, sizeof(ResetResourceList_t));
	ResetResourceItem_t reset_resource_item;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	int i,ret_int,flag = 0;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
	Iuh_HNB_UE *tmpUE = NULL;

	memset(member_reset_resource_list, 0, sizeof(Ranap_Member_ProtocolIE_Container_t));
	member_reset_resource_list->id = id_IuSigConIdList;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	member_reset_resource_list->criticality = (Criticality_t)integer;
	
	if(IUH_HNB[HNBID] != NULL)
	{
		tmpUE = IUH_HNB[HNBID]->HNB_UE;
		for(i = 0; i < IUH_HNB[HNBID]->current_UE_number; i++,tmpUE=tmpUE->ue_next)
		{
			memset(&reset_resource_item, 0, sizeof(ResetResourceItem_t));
			if(NULL != tmpUE)
			{
				if((cn_domain_id==CS_Domain) && (memcmp(tmpUE->iuCSSigConId, tmpUE->iuDefSigConId, IU_SIG_CONN_ID_LEN)!=0))
				{
					/* encode iuSigConId */
					BIT_STRING_fromBuf(&(reset_resource_item.iuSigConId), (char*)tmpUE->iuCSSigConId, IU_SIG_CONN_ID_LEN, 0);
					iuh_syslog_debug_debug(IUH_DEFAULT,"add UE[%d] domain %d, iuSigConId %x%x%x\n",tmpUE->UEID,cn_domain_id,tmpUE->iuCSSigConId[0],tmpUE->iuCSSigConId[1],tmpUE->iuCSSigConId[2]);
					flag = 1;
				}
				else if((cn_domain_id==PS_Domain) && (memcmp(tmpUE->iuPSSigConId, tmpUE->iuDefSigConId, IU_SIG_CONN_ID_LEN)!=0))
				{
					/* encode iuSigConId */
					BIT_STRING_fromBuf(&(reset_resource_item.iuSigConId), (char*)tmpUE->iuPSSigConId, IU_SIG_CONN_ID_LEN, 0);
					iuh_syslog_debug_debug(IUH_DEFAULT,"add UE[%d] domain %d, iuSigConId %x%x%x\n",tmpUE->UEID,cn_domain_id,tmpUE->iuPSSigConId[0],tmpUE->iuPSSigConId[1],tmpUE->iuPSSigConId[2]);
					flag = 1;
				}
				if(1 == flag)
				{
					ret_int = ASN_SEQUENCE_ADD(&(reset_resource_list.list), &reset_resource_item);
					assert(ret_int == 0);
				}
			}
		}
	}
	
	ret = uper_encode_to_buffer(&asn_DEF_ResetResourceList,(void *)&reset_resource_list, ie_buff, IE_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"member_reset_resource_list ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(member_reset_resource_list->value)), (const char *)ie_buff, (ret.encoded+7)/8);
	
	xer_fprint(stdout, &asn_DEF_ResetResourceList, &reset_resource_list);
	return ret.encoded;
}



/*****************************************************
** DISCRIPTION:
**          Assemble rua connect message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleConnect(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RUA  CONNECT\n");
	
    iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleConnect\n");
    if(IUH_HNB[HNBID] == NULL) return Iuh_FALSE;
    
    RUA_PDU_t rua_pdu;
	Connect_t rua_connect;
	int ret_int;
	ProtocolIE_Container_112P0_t *ie_container;
	Rua_Member_ProtocolIE_Container_t member_cn_domainId;
    Rua_Member_ProtocolIE_Container_t member_context_id;
    Rua_Member_ProtocolIE_Container_t member_establishment_cause ;
    Rua_Member_ProtocolIE_Container_t member_ranap_msg;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

	memset(&rua_pdu, 0, sizeof(RUA_PDU_t));
	rua_pdu.present = RUA_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(rua_pdu.choice.successfulOutcome);
	initial_msg->procedureCode = Pro_Connect;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&rua_connect, 0, sizeof(Connect_t));
	ie_container = &rua_connect.protocolIEs;

	//init cs domain id
	init_cn_domain_id(&member_cn_domainId, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_cn_domainId);
	assert(ret_int == 0);

    //init context id
    int UEID = IUH_FIND_UE_BY_CTXID((char*)sigMsg->contextid);
    iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d\n", UEID);
    if(0 == UEID) return Iuh_FALSE;
	init_rua_context_id(&member_context_id, UEID);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_context_id);
	assert(ret_int == 0);

	//establishment cause
	init_establishment_cause(&member_establishment_cause, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_establishment_cause);
	assert(ret_int == 0);

	//ranap msg
	init_ranap_msg(&member_ranap_msg, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_ranap_msg);
	assert(ret_int == 0);
	

    ret = uper_encode_to_buffer(&asn_DEF_Connect,(void *)&rua_connect, rua_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"rua_connect ret.encoded = %d\n\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_RUA_PDU, (void *)&rua_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_PDU ret.encoded = %d\n\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	iuh_syslog_info("###  RanapMsgLen   :        %d\n", sigMsg->RanapMsg.size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble rua direct transfer message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleDirectTransfer(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RUA  DIRECT-TRANSFER\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleDirectTransfer\n");
    if(IUH_HNB[HNBID] == NULL) return Iuh_FALSE;
    
    RUA_PDU_t rua_pdu;
	DirectTransfer_t direct_transfer;
	int ret_int;
	ProtocolIE_Container_112P0_t *ie_container;
	Rua_Member_ProtocolIE_Container_t member_cn_domainId;
    Rua_Member_ProtocolIE_Container_t member_context_id;
    Rua_Member_ProtocolIE_Container_t member_ranap_msg;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

	memset(&rua_pdu, 0, sizeof(RUA_PDU_t));
	rua_pdu.present = RUA_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(rua_pdu.choice.successfulOutcome);
	initial_msg->procedureCode = Pro_DirectTransfer;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&direct_transfer, 0, sizeof(Connect_t));
	ie_container = &direct_transfer.protocolIEs;

	//init cs domain id
	init_cn_domain_id(&member_cn_domainId, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_cn_domainId);
	assert(ret_int == 0);

    //init context id
    int UEID = IUH_FIND_UE_BY_CTXID((char*)sigMsg->contextid);
	iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d \n",UEID);
	
    if(0 == UEID) return Iuh_FALSE;
	init_rua_context_id(&member_context_id, UEID);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_context_id);
	assert(ret_int == 0);

	//ranap msg
	init_ranap_msg(&member_ranap_msg, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_ranap_msg);
	assert(ret_int == 0);

    ret = uper_encode_to_buffer(&asn_DEF_DirectTransfer,(void *)&direct_transfer, rua_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"direct_transfer ret.encoded = %d\n\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_RUA_PDU, (void *)&rua_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_PDU ret.encoded = %d\n\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	iuh_syslog_info("###  RanapMsgLen   :        %d\n", sigMsg->RanapMsg.size);
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble rua disconnect message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleDisconnect(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RUA  DISCONNECT\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleDisconnect\n");
    if(IUH_HNB[HNBID] == NULL) return Iuh_FALSE;
    
    RUA_PDU_t rua_pdu;
	Disconnect_t rua_disconnect;
	int ret_int;
	ProtocolIE_Container_112P0_t *ie_container;
	Rua_Member_ProtocolIE_Container_t member_cn_domainId;
    Rua_Member_ProtocolIE_Container_t member_context_id;
    Rua_Member_ProtocolIE_Container_t member_rua_cause ;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

	memset(&rua_pdu, 0, sizeof(RUA_PDU_t));
	rua_pdu.present = RUA_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(rua_pdu.choice.successfulOutcome);
	initial_msg->procedureCode = Pro_Disconnect;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&rua_disconnect, 0, sizeof(Connect_t));
	ie_container = &rua_disconnect.protocolIEs;

	//init cs domain id
	init_cn_domain_id(&member_cn_domainId, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_cn_domainId);
	assert(ret_int == 0);

    //init context id
    int UEID = IUH_FIND_UE_BY_CTXID((char*)sigMsg->contextid);
	iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d \n",UEID);
    if(0 == UEID) return Iuh_FALSE;
	init_rua_context_id(&member_context_id, UEID);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_context_id);
	assert(ret_int == 0);

	//disconn cause
	init_rua_cause(&member_rua_cause, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_rua_cause);
	assert(ret_int == 0);

    //ranap msg
    if((sigMsg->cause.present == Rua_Cause_radioNetwork) && (sigMsg->cause.choice.radioNetwork == Normal)){
        Rua_Member_ProtocolIE_Container_t member_ranap_msg;
    	init_ranap_msg(&member_ranap_msg, sigMsg);
    	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_ranap_msg);
    	assert(ret_int == 0);
	}
	
    ret = uper_encode_to_buffer(&asn_DEF_Disconnect,(void *)&rua_disconnect, rua_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"rua_disconnect ret.encoded = %d\n\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_RUA_PDU, (void *)&rua_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_PDU ret.encoded = %d\n\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	iuh_syslog_info("###  RanapMsgLen   :        %d\n", sigMsg->RanapMsg.size);
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble rua connectionless transfer message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleConnectionlessTransfer(Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RUA  CONNECTIONLESS-TRANSFER\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleConnectionlessTransfer\n");
    RUA_PDU_t rua_pdu;
	ConnectionlessTransfer_t connectionless_transfer;
	int ret_int;
	ProtocolIE_Container_112P0_t *ie_container;
    Rua_Member_ProtocolIE_Container_t member_ranap_msg;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

	memset(&rua_pdu, 0, sizeof(RUA_PDU_t));
	rua_pdu.present = RUA_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(rua_pdu.choice.successfulOutcome);
	initial_msg->procedureCode = Pro_ConnectionlessTransfer;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&connectionless_transfer, 0, sizeof(Connect_t));
	ie_container = &connectionless_transfer.protocolIEs;

	//ranap msg
	init_ranap_msg(&member_ranap_msg, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_ranap_msg);
	assert(ret_int == 0);

    ret = uper_encode_to_buffer(&asn_DEF_ConnectionlessTransfer,(void *)&connectionless_transfer, rua_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"connectionless_transfer ret.encoded = %d\n\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_RUA_PDU, (void *)&rua_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_PDU ret.encoded = %d\n\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	iuh_syslog_info("###  RanapMsgLen   :        %d\n", sigMsg->RanapMsg.size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble rua error indication message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
IuhBool IuhAssembleRuaErrIndication(Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RUA  ERROR-INDICATION\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleRuaErrIndication\n");
    RUA_PDU_t rua_pdu;
	Rua_ErrorIndication_t rua_err_indication;
	int ret_int;
	ProtocolIE_Container_112P0_t *ie_container;
    Rua_Member_ProtocolIE_Container_t member_rua_cause ;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

	memset(&rua_pdu, 0, sizeof(RUA_PDU_t));
	rua_pdu.present = RUA_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(rua_pdu.choice.successfulOutcome);
	initial_msg->procedureCode = Pro_ErrorIndication;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&rua_err_indication, 0, sizeof(Rua_ErrorIndication_t));
	ie_container = &rua_err_indication.protocolIEs;

	//error indication cause
	init_rua_cause(&member_rua_cause, sigMsg);
	ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_rua_cause);
	assert(ret_int == 0);
	
    ret = uper_encode_to_buffer(&RUA_asn_DEF_ErrorIndication,(void *)&rua_err_indication, rua_buff, RUA_BUFFER_SIZE);
	iuh_syslog_debug_debug(IUH_DEFAULT,"rua_err_indication ret.encoded = %d\n\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_RUA_PDU, (void *)&rua_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"RUA_PDU ret.encoded = %d\n\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Assemble HNBAP error indication message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
IuhBool IuhAssembleHnbapErrIndication(HNBAPCause *cause, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  ERROR-INDICATION\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleHnbapErrIndication\n");
    HNBAP_PDU_t hnbap_pdu;
    Hnbap_ErrorIndication_t hnbap_err_indication;
    int ret_int;
    ProtocolIE_Container_112P0_t *ie_container;
    Hnb_Member_ProtocolIE_Container_t member_hnbap_cause ;
    InitiatingMessage_t *initial_msg;
    asn_enc_rval_t ret;
    INTEGER_t integer;
    unsigned char rua_buff[RUA_BUFFER_SIZE] = {0};

    memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
    hnbap_pdu.present = HNBAP_PDU_PR_initiatingMessage;

    initial_msg = (InitiatingMessage_t *)&(hnbap_pdu.choice.successfulOutcome);
    initial_msg->procedureCode = ProcErrorIndication;
    memset(&integer,0,sizeof(INTEGER_t));
    asn_ulong2INTEGER(&integer, Criticality_reject);
    initial_msg->criticality = (Criticality_t)integer;
    memset(&hnbap_err_indication, 0, sizeof(Hnbap_ErrorIndication_t));
    ie_container = &hnbap_err_indication.protocolIEs;

    //error indication cause
    init_cause(&member_hnbap_cause, cause);
    ret_int = ASN_SEQUENCE_ADD(&(ie_container->list), &member_hnbap_cause);
    assert(ret_int == 0);
    
    ret = uper_encode_to_buffer(&HNBAP_asn_DEF_ErrorIndication,(void *)&hnbap_err_indication, rua_buff, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"hnbap_err_indication ret.encoded = %d\n\n",ret.encoded);
    ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)rua_buff, (ret.encoded+7)/8);
    ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, RUA_BUFFER_SIZE);
    iuh_syslog_debug_debug(IUH_DEFAULT,"HNBAP_PDU ret.encoded = %d\n\n",ret.encoded);
    
    msgPtr->size = (ret.encoded+7)/8;
    iuh_syslog_debug_debug(IUH_DEFAULT,"msgPtr->size = %d\n",msgPtr->size);

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
    
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble Hnb De-Register message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleHNBDeRegister(IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  HNB-DEREGISTER\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleHNBDeRegister\n");
	HNBAP_PDU_t hnbap_pdu;
	HNBDe_Register_t hnb_de_reg;
	HNBAPCause cause;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_cause;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(hnbap_pdu.choice.initiatingMessage);
	initial_msg->procedureCode = HNBDeRegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&hnb_de_reg, 0, sizeof(HNBRegisterAccept_t));
	ie_container = &hnb_de_reg.protocolIEs;

    cause.present = Cause_misc;
    cause.choice.misc = Misc_unspecified;

	ret.encoded = init_cause(&member_cause, &cause);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cause);

	if(cause.present == HNBAP_Cause_PR_radioNetwork){
	    if(cause.choice.radioNetwork == HNBAP_CauseRadioNetwork_overload){
	        struct Hnb_Member_ProtocolIE_Container member_backoff_timer;
	        ret.encoded = init_backoff_timer(&member_backoff_timer);
	        if(ret.encoded == -1){
        	    return Iuh_FALSE;
        	}
        	ASN_SEQUENCE_ADD(&(ie_container->list), &member_backoff_timer);
	    }
	}

	ret = uper_encode_to_buffer(&asn_DEF_HNBDe_Register,(void *)&hnb_de_reg, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;

	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Assemble UE De-Register message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleUEDeRegister(int UEID, IuhProcotolMsg *msgPtr)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-DEREGISTER\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleUEDeRegister\n");
	HNBAP_PDU_t hnbap_pdu;
	UEDe_Register_t ue_de_reg;
	HNBAPCause cause;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_ue_context_id;
	struct Hnb_Member_ProtocolIE_Container member_cause;
	InitiatingMessage_t *initial_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_initiatingMessage;

	initial_msg = (InitiatingMessage_t *)&(hnbap_pdu.choice.initiatingMessage);
	initial_msg->procedureCode = HNBDeRegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initial_msg->criticality = (Criticality_t)integer;
	memset(&ue_de_reg, 0, sizeof(HNBRegisterAccept_t));
	ie_container = &ue_de_reg.protocolIEs;

    cause.present = Cause_misc;
    cause.choice.misc = Misc_unspecified;

	ret.encoded = init_cause(&member_cause, &cause);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cause);

	ret.encoded = init_context_id(&member_ue_context_id, UEID);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_context_id);

	ret = uper_encode_to_buffer(&asn_DEF_UEDe_Register,(void *)&ue_de_reg, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initial_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Assemble ue register request
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhAssembleUERegisterRequest(IuhProcotolMsg *msgPtr, const int UEID)
{
	if(IUH_HNB_UE[UEID] == NULL){
		iuh_syslog_err("error: register UE is NULL\n");
		return Iuh_FALSE;
	}
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-REGISTER-REQUEST\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhAssembleUERegisterReject\n");
    HNBAP_PDU_t hnbap_pdu;
	UERegisterRequest_t ue_reg_request;
	ProtocolIE_Container_110P0_t *ie_container;
	struct Hnb_Member_ProtocolIE_Container member_ue_identity;
	struct Hnb_Member_ProtocolIE_Container member_registration_cause;
	struct Hnb_Member_ProtocolIE_Container member_ue_capabilities;
	InitiatingMessage_t *initiating_message;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_initiatingMessage;

	initiating_message = (InitiatingMessage_t *)&(hnbap_pdu.choice.initiatingMessage);
	initiating_message->procedureCode = UERegister;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initiating_message->criticality = (Criticality_t)integer;
	memset(&ue_reg_request, 0, sizeof(UERegisterRequest_t));
	ie_container = &ue_reg_request.protocolIEs;
	
	/* add ue identity */
    ret.encoded = init_ue_identity(&member_ue_identity, UEID);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_identity);
    /* add registration cause */
    ret.encoded = init_registration_cause(&member_registration_cause, IUH_HNB_UE[UEID]->registrationCause);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_registration_cause);
	/* add ue capabilities */
	ret.encoded = init_ue_capabilities(&member_ue_capabilities, &(IUH_HNB_UE[UEID]->Capabilities));
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ue_capabilities);
	
	ret = uper_encode_to_buffer(&asn_DEF_UERegisterRequest,(void *)&ue_reg_request, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initiating_message->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, msgPtr->buf, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	msgPtr->size = (ret.encoded+7)/8;
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", msgPtr->size);
	
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**			Assemble ranap reset acknowledge
** INPUT:
**			
** OUTPUT:
**			
** RETURN:
**			
** book add 2011-12-14
*****************************************************/
IuhBool IuhAssembleRanapResetAck(Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RANAP  RESET-ACKNOWLEDGE \n");
    
    RANAP_PDU_t ranap_pdu;
	ResetAcknowledge_t reset_ack;
	ProtocolIE_Container_139P0_t *ie_container;
	struct Ranap_Member_ProtocolIE_Container member_cn_domainId;
	SuccessfulOutcome_t *success_msg;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&ranap_pdu, 0, sizeof(RANAP_PDU_t));
	ranap_pdu.present = RANAP_PDU_PR_successfulOutcome;

	success_msg = (SuccessfulOutcome_t *)&(ranap_pdu.choice.successfulOutcome);
	success_msg->procedureCode = Pro_id_Reset;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	success_msg->criticality = (Criticality_t)integer;
	memset(&reset_ack, 0, sizeof(ResetAcknowledge_t));
	ie_container = &reset_ack.protocolIEs;

	iuh_syslog_debug_debug(IUH_DEFAULT,"sigMsg->CnDomain = %d\n",sigMsg->CnDomain);
	ret.encoded = init_ranap_cn_domain_id(&member_cn_domainId, sigMsg->CnDomain);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cn_domainId);

	ret = uper_encode_to_buffer(&asn_DEF_ResetAcknowledge,(void *)&reset_ack, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(success_msg->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_RANAP_PDU, (void *)&ranap_pdu, sigMsg->RanapMsg.RanapMsg, MSG_LEN);
	if(ret.encoded == -1){
	    return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	sigMsg->RanapMsg.size = (ret.encoded+7)/8;
	/* Print Log Information */
	iuh_syslog_info("###  Length        :        %d\n", sigMsg->RanapMsg.size);
	
	return Iuh_TRUE;
}




/*****************************************************
** DISCRIPTION:
**			Assemble ranap reset resource
** INPUT:
**			
** OUTPUT:
**			
** RETURN:
**			
** book add 2011-12-19
*****************************************************/
IuhBool IuhAssembleRanapResetResource(IuhProcotolMsg *msgPtr, const int HNBID, Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Encode \n");
	iuh_syslog_info("###  Type          :        RANAP    RESET-RESOURCE\n");
	iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
	RANAP_PDU_t ranap_pdu;
	ResetResource_t reset_resource;
	ProtocolIE_Container_139P0_t *ie_container;
	struct Ranap_Member_ProtocolIE_Container member_cn_domainId;
	struct Ranap_Member_ProtocolIE_Container member_ranap_cause;
	struct Ranap_Member_ProtocolIE_Container member_iuSigConId_list;
	InitiatingMessage_t *initiating_message;
	asn_enc_rval_t ret;
	INTEGER_t integer;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

	memset(&ranap_pdu, 0, sizeof(RANAP_PDU_t));
	ranap_pdu.present = RANAP_PDU_PR_initiatingMessage;

	initiating_message = (InitiatingMessage_t *)&(ranap_pdu.choice.initiatingMessage);
	initiating_message->procedureCode = Pro_id_Reset;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, Criticality_reject);
	initiating_message->criticality = (Criticality_t)integer;
	memset(&reset_resource, 0, sizeof(ResetResource_t));
	ie_container = &reset_resource.protocolIEs;

	/* add cndomain id */
	iuh_syslog_debug_debug(IUH_DEFAULT,"sigMsg->CnDomain = %d\n",sigMsg->CnDomain);
	ret.encoded = init_ranap_cn_domain_id(&member_cn_domainId, sigMsg->CnDomain);
	if(ret.encoded == -1){
		return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_cn_domainId);

	/* add cause */
	RANAPCause cause;
	cause.present = RANAP_Cause_radioNetwork_Ranap;
	cause.choice.radioNetwork = RANAP_CauseRadioNetwork_radio_connection_with_UE_Lost;
	ret.encoded = init_ranap_cause(&member_ranap_cause, &cause);
	if(ret.encoded == -1){
		return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_ranap_cause);

	/* add iu signaling connection id list */
	ret.encoded = init_ranap_reset_resource_list(&member_iuSigConId_list, HNBID, sigMsg->CnDomain);
	if(ret.encoded == -1){
		return Iuh_FALSE;
	}
	ASN_SEQUENCE_ADD(&(ie_container->list), &member_iuSigConId_list);
	

	ret = uper_encode_to_buffer(&asn_DEF_ResetResource,(void *)&reset_resource, pdu_buff, PDU_BUFFER_SIZE);
	if(ret.encoded == -1){
		return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n222ret.encoded = %d\n",ret.encoded);
	ANY_fromBuf(((OCTET_STRING_t *)&(initiating_message->value)), (const char *)pdu_buff, (ret.encoded+7)/8);

	ret = uper_encode_to_buffer(&asn_DEF_RANAP_PDU, (void *)&ranap_pdu, sigMsg->RanapMsg.RanapMsg, MSG_LEN);
	if(ret.encoded == -1){
		return Iuh_FALSE;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"\n333ret.encoded = %d\n",ret.encoded);
	
	sigMsg->RanapMsg.size = (ret.encoded+7)/8;
	/* Print Log Information */
	iuh_syslog_info("###  Length		:		 %d\n", sigMsg->RanapMsg.size);
	
	return Iuh_TRUE;
}

