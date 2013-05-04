#include "Iuh_Decode.h"
#include "BIT_STRING.h"
#include "OCTET_STRING.h"
#include "INTEGER.h"
#include "LAC.h"
#include "RAC.h"
#include "SAC.h"
#include "CSG-ID.h"
#include "HNBAP-Cause.h"
#include "PLMNidentity.h"
#include "HNB-Cell-Access-Mode.h"
#include "CellIdentity.h"
#include "BackoffTimer.h"
#include "Context-ID.h"
#include "UE-Identity.h"
#include "UE-Capabilities.h"
#include "Registration-Cause.h"
#include "IntraDomainNasNodeSelector.h"
#include "CSGMembershipStatus.h"
#include "HNBAP-ErrorIndication.h"

#include "HNB-Identity.h"
#include "HNBDe-Register.h"
#include "HNBRegisterRequest.h"
#include "ProtocolIE-Container.h"
#include "HNBAP-ProtocolIE-Container.h"
#include "HNB-Location-Information.h"
#include "UEDe-Register.h"
#include "UERegisterRequest.h"
#include "CN-DomainIndicator.h"
#include "RUA-CN-DomainIndicator.h"
#include "RANAP-CN-DomainIndicator.h"
#include "Establishment-Cause.h"
#include "RANAP-Message.h"
#include "RUA-Cause.h"
#include "Connect.h"
#include "Disconnect.h"
#include "DirectTransfer.h"
#include "ConnectionlessTransfer.h"
#include "RUA-PDU.h"
#include "iuh/Iuh.h"
#include "Iuh_log.h"

#include "RANAP-PDU.h"
#include "RANAP-ProtocolIE-Container.h"
#include "RelocationRequest.h"
#include "InitialUE-Message.h"
#include "SourceRNC-ToTargetRNC-TransparentContainer.h"
//#include "Paging.h"
//#include "PagingAreaID.h"
//#include "PagingCause.h"
#include "RANAP-CN-DomainIndicator.h"
//#include "PermanentNAS-UE-ID.h"
//#include "TemporaryUE-ID.h"
//#include "NonSearchingIndication.h"
//#include "GlobalCN-ID.h"
//#include "DRX-CycleLengthCoefficient.h"
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
#include "RUA-ErrorIndication.h"
#include "RANAP-Cause.h"
#include "IuSignallingConnectionIdentifier.h"



/* @@@@@@@@@@@@@@@   Decode Items Begin  @@@@@@@@@@@@@@@ */


/*****************************************************
** DISCRIPTION:
**          Decode CnDomainIndicator 
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          sigMsg
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRUACnDomainIndicator(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    RUA_CN_DomainIndicator_t *cn_domain_id;
    cn_domain_id = calloc(1, sizeof(RUA_CN_DomainIndicator_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RUA_CN_DomainIndicator, (void **)&cn_domain_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cn_domain_id fail\n");
        *errflag = 1;
    }
    else if(cn_domain_id != NULL){
        asn_INTEGER2long((INTEGER_t *)cn_domain_id, (long*)&sigMsg->CnDomain);
    }
    IUH_FREE_OBJECT(cn_domain_id);

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode RUA ContextID
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          sigMsg
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRUAContextID(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    int UEID = 0;
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;         
    Context_ID_t *context_id;
    context_id = calloc(1, sizeof(Context_ID_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Context_ID, (void **)&context_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode context_id fail\n");
        *errflag = 1;
    }
    else if(context_id != NULL){
		memset(sigMsg->contextid, 0, CONTEXTID_LEN);
        memcpy(sigMsg->contextid, context_id->buf, CONTEXTID_LEN);
		iuh_hnb_show_str("ContextID = ", sigMsg->contextid, 0, CONTEXTID_LEN);
        UEID = IUH_FIND_UE_BY_CTXID((char*)sigMsg->contextid);
        if(UEID >= UE_NUM){
            iuh_syslog_debug_debug(IUH_DEFAULT,"invalid context_id\n");
            if(IUH_HNB_UE[1] != NULL){
                sigMsg->UeIdentity.present = Pr_IMSI;
                memcpy(sigMsg->UeIdentity.choice.IMSI, IUH_HNB_UE[1]->IMSI, IMSI_LEN);
            }
        }
        else if((UEID != 0) && (IUH_HNB_UE[UEID] != NULL)){
            sigMsg->UeIdentity.present = Pr_IMSI;
            memcpy(sigMsg->UeIdentity.choice.IMSI, IUH_HNB_UE[UEID]->IMSI, IMSI_LEN);
			memcpy(IUH_HNB_UE[UEID]->context_id_str, context_id->buf, CONTEXTID_LEN);
        }
        //UEID = IUH_FIND_UE_BY_CXTID(contextID);
    }
    IUH_FREE_OBJECT(context_id);

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode EstablistmentCause
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          sigMsg
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeEstablishmentCause(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0; 
    Establishment_Cause_t *establishment_cause;
    establishment_cause = calloc(1, sizeof(Establishment_Cause_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Establishment_Cause, (void **)&establishment_cause, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode establishment_cause fail\n");
        *errflag = 1;
    }
    else if(establishment_cause != NULL){
        int esCause;
        asn_INTEGER2long((INTEGER_t *)establishment_cause, (long*)&esCause);
        if(esCause <= 1)
            sigMsg->EstabCause = esCause;
    }
    IUH_FREE_OBJECT(establishment_cause);
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RUA RanapMessage
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          sigMsg
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRUARanapMessage(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0; 
    RANAP_Message_t *ranap_msg;
    ranap_msg = calloc(1, sizeof(RANAP_Message_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RANAP_Message, (void **)&ranap_msg, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ranap_msg fail\n");
        *errflag = 1;
    }
    else if(ranap_msg != NULL){
        sigMsg->RanapMsg.size = ranap_msg->size;
        memcpy(sigMsg->RanapMsg.RanapMsg, ranap_msg->buf, ranap_msg->size);
    }
    IUH_FREE_OBJECT(ranap_msg);
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RUA CSGMembershipStatus
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          sigMsg
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeCSGMembershipStatus(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0; 
    CSGMembershipStatus_t *csg_membership_status; 
    csg_membership_status = calloc(1, sizeof(CSGMembershipStatus_t));
    ret = uper_decode_complete(&code_ctx, &asn_DEF_CSGMembershipStatus, (void **)&csg_membership_status, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode establishment_cause fail\n");
        *errflag = 1;
    }
    else if(csg_membership_status != NULL){
        int csgstatus;
        asn_INTEGER2long((INTEGER_t *)csg_membership_status, (long*)&csgstatus);
    }
    IUH_FREE_OBJECT(csg_membership_status);
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode HNBIdentity
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeHHBIdentity(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    HNB_Identity_t *hnb_identity;
    hnb_identity = calloc(1,sizeof(HNB_Identity_t));
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNB_Identity, (void **)&hnb_identity, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode hnb_identity fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(hnb_identity != NULL){
		iuh_syslog_debug_debug(IUH_DEFAULT, "@@@ len = %d\n",hnb_identity->hNB_Identity_Info.size);
		iuh_syslog_debug_debug(IUH_DEFAULT, "@@@ requestValue->HnbIdentity = %s\n",hnb_identity->hNB_Identity_Info.buf);
        memcpy(requestValue->HnbIdentity, hnb_identity->hNB_Identity_Info.buf, strnlen(hnb_identity->hNB_Identity_Info.buf, HNB_IDENTITY_LEN));
		iuh_syslog_debug_debug(IUH_DEFAULT, "@@@ requestValue->HnbIdentity = %s\n",(char*)requestValue->HnbIdentity);
    }
    IUH_FREE_OBJECT(hnb_identity);
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode HNB Location Information
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeHHBLocationInformation(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    HNB_Location_Information_t  *hnb_location_info;
    hnb_location_info = calloc(1,sizeof(HNB_Location_Information_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNB_Location_Information, (void **)&hnb_location_info, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode hnb_location_information fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(hnb_location_info != NULL){
        if(hnb_location_info->macroCoverageInfo != NULL){
            requestValue->HnbLocationInfo.macroCoverageInfo = (struct macroCellID *)malloc(sizeof(struct macroCellID));
            memset(requestValue->HnbLocationInfo.macroCoverageInfo, 0, sizeof(struct macroCellID));
            requestValue->HnbLocationInfo.macroCoverageInfo->present = \
                hnb_location_info->macroCoverageInfo->cellIdentity.present;
         
            if(requestValue->HnbLocationInfo.macroCoverageInfo->present == uTRANCellID){
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.lAC, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.uTRANCellID.lAC.buf, LAC_LEN);
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.rAC, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.uTRANCellID.rAC.buf, RAC_LEN);
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.pLMNidentity, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.uTRANCellID.pLMNidentity.buf, PLMN_LEN);
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.uTRANCellID.uTRANcellID.buf, CELLID_LEN);
            }
            else if(requestValue->HnbLocationInfo.macroCoverageInfo->present == gERANCellID){
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.pLMNidentity, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.gERANCellID.pLMNidentity.buf, PLMN_LEN);
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.lAC, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.gERANCellID.lAC.buf, LAC_LEN);
                memcpy(requestValue->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.cI, \
                    hnb_location_info->macroCoverageInfo->cellIdentity.choice.gERANCellID.cI.buf, CELLID_LEN);
            }
            
        }
        
        if(hnb_location_info->geographicalCoordinates != NULL){
            long direction,sign;
            requestValue->HnbLocationInfo.gographicalLocation = (struct geographicalLocation *)malloc(sizeof(struct geographicalLocation));
            memset(requestValue->HnbLocationInfo.gographicalLocation, 0, sizeof(struct geographicalLocation));
            asn_INTEGER2long(&(hnb_location_info->geographicalCoordinates->altitudeAndDirection.directionOfAltitude), (long*)&direction);
            asn_INTEGER2long(&(hnb_location_info->geographicalCoordinates->geographicalCoordinates.latitudeSign), (long*)&sign);
            requestValue->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.DirectionOfAltitude = direction;
            requestValue->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.altitude = \
                hnb_location_info->geographicalCoordinates->altitudeAndDirection.altitude;
            requestValue->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.LatitudeSign = sign;
            requestValue->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.latitude = \
                hnb_location_info->geographicalCoordinates->geographicalCoordinates.latitude;
            requestValue->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.longitude = \
                hnb_location_info->geographicalCoordinates->geographicalCoordinates.longitude;
        }
    }
    IUH_FREE_OBJECT(hnb_location_info);

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode PLMN Identity
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodePLMNIdentity(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    PLMNidentity_t *plmn_id;
    plmn_id = calloc(1,sizeof(PLMNidentity_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_PLMNidentity, (void **)&plmn_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode plmn_idendity fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(plmn_id!= NULL){
        memcpy(requestValue->plmnId, plmn_id->buf, PLMN_LEN);
		iuh_hnb_show_str("requestValue->plmnId = ", requestValue->plmnId, 1, PLMN_LEN);
		//IUH_STRING_TO_INT((char*)requestValue->plmnId, PLMN_LEN);
    }
    IUH_FREE_OBJECT(plmn_id);

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode Cell Identity
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeCellIdentity(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    CellIdentity_t *cell_id;
    cell_id = calloc(1,sizeof(CellIdentity_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_CellIdentity, (void **)&cell_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cell_identity fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(cell_id!= NULL){
        memcpy(requestValue->cellId, cell_id->buf, CELLID_LEN);
    }
    IUH_FREE_OBJECT(cell_id);

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode LAC
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeLAC(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    LAC_t *lac;
    lac = calloc(1,sizeof(LAC_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_LAC, (void **)&lac, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode lac fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(lac != NULL){
       memcpy(requestValue->lac, lac->buf, LAC_LEN);
    }

    IUH_FREE_OBJECT(lac);

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RAC
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRAC(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    RAC_t *rac;
    rac = calloc(1,sizeof(RAC_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RAC, (void **)&rac, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rac fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(rac != NULL){
        memcpy(requestValue->rac, rac->buf, RAC_LEN);
    }
    IUH_FREE_OBJECT(rac);

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode SAC
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeSAC(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    SAC_t *sac;
    sac = calloc(1,sizeof(SAC_t));
   
    ret = uper_decode_complete(&code_ctx, &asn_DEF_SAC, (void **)&sac, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode sac fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(sac != NULL){
        memcpy(requestValue->sac ,sac->buf, SAC_LEN);
    }
    IUH_FREE_OBJECT(sac);

    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode CSG ID
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeCSGID(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    CSG_ID_t *csg_id;
    csg_id = calloc(1,sizeof(CSG_ID_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_CSG_ID, (void **)&csg_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode csg_id fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(csg_id!= NULL){
        memcpy(requestValue->csgId, csg_id->buf, CSGID_LEN);
    }
    IUH_FREE_OBJECT(csg_id);

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode Cell Access Mode
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          requestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
** <zhangshu@autelan.com> 
** 2012-1-4
*****************************************************/
IuhBool IuhDecodeCellAccessMode(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    HNB_Cell_Access_Mode_t *cell_access_mode;
    cell_access_mode = calloc(1,sizeof(HNB_Cell_Access_Mode_t));
    int acl_switch = 0;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNB_Cell_Access_Mode, (void **)&cell_access_mode, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cell_access_mode fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(cell_access_mode!= NULL){
        asn_INTEGER2long(cell_access_mode, (long*)&acl_switch);
        requestValue->acl_switch = acl_switch;
        iuh_syslog_debug_debug(IUH_DEFAULT,"requestValue->acl_switch = %d\n",requestValue->acl_switch);        
    }
    IUH_FREE_OBJECT(cell_access_mode);

    return Iuh_TRUE;
}




/*****************************************************
** DISCRIPTION:
**          Decode HNBAP Cause
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeHNBAPCause(const char *buf, const int size, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    HNBAP_Cause_t *dereg_cause;
    dereg_cause = calloc(1,sizeof(HNBAP_Cause_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNBAP_Cause, (void **)&dereg_cause, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cause fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(dereg_cause != NULL){
        cause->present = dereg_cause->present;
        switch(cause->present){
            int my_cause;
            case HNBAP_Cause_PR_radioNetwork: {
                asn_INTEGER2long(&(dereg_cause->choice.radioNetwork), (long*)&my_cause);
                cause->choice.radioNetwork = my_cause;
                break;
            }
            case HNBAP_Cause_PR_transport: {
                asn_INTEGER2long(&(dereg_cause->choice.transport), (long*)&my_cause);
                cause->choice.transport = my_cause;
                break;
            }
            case HNBAP_Cause_PR_protocol: {
                asn_INTEGER2long(&(dereg_cause->choice.protocol), (long*)&my_cause);
                cause->choice.protocol= my_cause;
                break;
            }
            case HNBAP_Cause_PR_misc: {
                asn_INTEGER2long(&(dereg_cause->choice.misc), (long*)&my_cause);
                cause->choice.misc= my_cause;
                break;
            }
            default:
                break;
        }
    }
    
    IUH_FREE_OBJECT(dereg_cause);
    
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode RANAP Cause
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRANAPCause(const char *buf, const int size, RANAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    Ranap_Cause_t *ranap_cause;
    ranap_cause = calloc(1,sizeof(Ranap_Cause_t));
    
    ret = uper_decode_complete(&code_ctx, &RANAP_asn_DEF_Cause, (void **)&ranap_cause, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ranap cause fail\n");
        cause->present = RANAP_Cause_protocol_Ranap;
        cause->choice.protocol = RANAP_CauseProtocol_transfer_syntax_error;
        *errflag = 1;
    }
    else if(ranap_cause != NULL){
        cause->present = ranap_cause->present;
        switch(cause->present){
            long my_cause;
            case RANAP_Cause_radioNetwork_Ranap: {
                asn_INTEGER2long(&(ranap_cause->choice.radioNetwork), (long*)&my_cause);
                cause->choice.radioNetwork = my_cause;
                break;
            }
            case RANAP_Cause_transmissionNetwork: {
                asn_INTEGER2long(&(ranap_cause->choice.transmissionNetwork), (long*)&my_cause);
                cause->choice.transmissionNetwork = my_cause;
                break;
            }
            case RANAP_Cause_nAS: {
                asn_INTEGER2long(&(ranap_cause->choice.nAS), (long*)&my_cause);
                cause->choice.nAS = my_cause;
                break;
            }
            case RANAP_Cause_protocol_Ranap: {
                asn_INTEGER2long(&(ranap_cause->choice.protocol), (long*)&my_cause);
                cause->choice.protocol = my_cause;
                break;
            }
			case RANAP_Cause_misc_Ranap: {
                asn_INTEGER2long(&(ranap_cause->choice.misc), (long*)&my_cause);
                cause->choice.misc= my_cause;
                break;
            }
			case RANAP_Cause_non_Standard: {
                asn_INTEGER2long(&(ranap_cause->choice.non_Standard), (long*)&my_cause);
                cause->choice.non_Standard = my_cause;
                break;
            }
			case RANAP_Cause_PR_radioNetworkExtension: {
                asn_INTEGER2long(&(ranap_cause->choice.radioNetworkExtension), (long*)&my_cause);
                cause->choice.radioNetworkExtension = my_cause;
                break;
            }
            default:
                break;
        }
    }
    
    IUH_FREE_OBJECT(ranap_cause);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode Backoff Timer
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeBackoffTimer(const char *buf, const int size, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    BackoffTimer_t *backoff_timer;
    backoff_timer = calloc(1,sizeof(BackoffTimer_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_BackoffTimer, (void **)&backoff_timer, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode backoff_timer fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(backoff_timer != NULL){
        // not added
    }
    IUH_FREE_OBJECT(backoff_timer);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode UE Identity
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          UERequestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeUEIdentity(const char *buf, const int size, UERegisterRequestValue *UERequestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    UE_Identity_t *ue_identity;
    ue_identity = (UE_Identity_t *)calloc(1,sizeof(UE_Identity_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_UE_Identity, (void **)&ue_identity, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ue_identity fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(ue_identity != NULL){
        iuh_syslog_debug_debug(IUH_DEFAULT,"copy ue_identity\n");
        iuh_syslog_debug_debug(IUH_DEFAULT,"ue_identity->present = %d\n",ue_identity->present);
        UERequestValue->UE_Identity.present = ue_identity->present;
        switch(UERequestValue->UE_Identity.present){
            case UE_Identity_iMSI: {
                memset(UERequestValue->IMSI,0,8);
                memcpy(UERequestValue->IMSI,ue_identity->choice.iMSI.buf,ue_identity->choice.iMSI.size);
                memcpy(UERequestValue->UE_Identity.choice.imsi, ue_identity->choice.iMSI.buf, ue_identity->choice.iMSI.size);
                iuh_syslog_debug_debug(IUH_DEFAULT,"\n\nIMSI = %d\n\n\n",*((int*)UERequestValue->IMSI));
                break;
            }
            case UE_Identity_tMSILAI:{
                memcpy(UERequestValue->UE_Identity.choice.tmsilai.tmsi, ue_identity->choice.tMSILAI.tMSI.buf, TMSI_LEN);
                memcpy(UERequestValue->UE_Identity.choice.tmsilai.lai.plmnid, ue_identity->choice.tMSILAI.lAI.pLMNID.buf, PLMN_LEN);
                memcpy(UERequestValue->UE_Identity.choice.tmsilai.lai.lac, ue_identity->choice.tMSILAI.lAI.lAC.buf, LAC_LEN);
                break;
            }
            case UE_Identity_pTMSIRAI: {
                memcpy(UERequestValue->UE_Identity.choice.ptmsirai.ptmsi, ue_identity->choice.pTMSIRAI.pTMSI.buf, TMSI_LEN);
                memcpy(UERequestValue->UE_Identity.choice.ptmsirai.rai.lai.plmnid, ue_identity->choice.pTMSIRAI.rAI.lAI.pLMNID.buf, PLMN_LEN);
                memcpy(UERequestValue->UE_Identity.choice.ptmsirai.rai.lai.lac, ue_identity->choice.pTMSIRAI.rAI.lAI.lAC.buf, LAC_LEN);
                memcpy(UERequestValue->UE_Identity.choice.ptmsirai.rai.rac, ue_identity->choice.pTMSIRAI.rAI.rAC.buf, RAC_LEN);
                break;
            }
            case UE_Identity_iMEI: {
                memcpy(UERequestValue->UE_Identity.choice.imei, ue_identity->choice.iMEI.buf, IMEI_LEN);
                break;
            }
            case UE_Identity_eSN: {
                memcpy(UERequestValue->UE_Identity.choice.esn, ue_identity->choice.eSN.buf, ESN_LEN);
                break;
            }
            case UE_Identity_iMSIDS41: {
                UERequestValue->UE_Identity.choice.imsids41 = malloc(ue_identity->choice.iMSIDS41.size+1);
                memset(UERequestValue->UE_Identity.choice.imsids41, 0, ue_identity->choice.iMSIDS41.size+1);
                memcpy(UERequestValue->UE_Identity.choice.imsids41, ue_identity->choice.iMSIDS41.buf, ue_identity->choice.iMSIDS41.size);
                break;
            }
            case UE_Identity_iMSIESN: {
                UERequestValue->UE_Identity.choice.imsiesn.imsids41 = malloc(ue_identity->choice.iMSIESN.iMSIDS41.size+1);
                memset(UERequestValue->UE_Identity.choice.imsiesn.imsids41, 0, ue_identity->choice.iMSIESN.iMSIDS41.size+1);
                memcpy(UERequestValue->UE_Identity.choice.imsiesn.imsids41, ue_identity->choice.iMSIESN.iMSIDS41.buf, ue_identity->choice.iMSIESN.iMSIDS41.size);
                memcpy(UERequestValue->UE_Identity.choice.imsiesn.esn, ue_identity->choice.iMSIESN.eSN.buf, ESN_LEN);
                break;
            }
            case UE_Identity_tMSIDS41: {
                UERequestValue->UE_Identity.choice.tmsids41 = malloc(ue_identity->choice.tMSIDS41.size+1);
                memset(UERequestValue->UE_Identity.choice.tmsids41, 0, ue_identity->choice.tMSIDS41.size+1);
                memcpy(UERequestValue->UE_Identity.choice.tmsids41, ue_identity->choice.tMSIDS41.buf, ue_identity->choice.tMSIDS41.size);
                break;
            }
            default:
                break;
        }
    }
    IUH_FREE_OBJECT(ue_identity);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode UE Registration Cause
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          UERequestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeUERegistrationCause(const char *buf, const int size, UERegisterRequestValue *UERequestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
Registration_Cause_t *ue_registration_cause;
    ue_registration_cause = (Registration_Cause_t *)calloc(1,sizeof(Registration_Cause_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Registration_Cause, (void **)&ue_registration_cause, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ue_registration_cause fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(ue_registration_cause != NULL){
        int my_cause;
        asn_INTEGER2long(ue_registration_cause, (long *)&my_cause);
        UERequestValue->registrationCause = my_cause;
    }
    IUH_FREE_OBJECT(ue_registration_cause);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode UE Capabilities
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          UERequestValue
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeUECapabilities(const char *buf, const int size, UERegisterRequestValue *UERequestValue, HNBAPCause *cause, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    UE_Capabilities_t *ue_capabilities;
    ue_capabilities = (UE_Capabilities_t *)calloc(1,sizeof(UE_Capabilities_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_UE_Capabilities, (void **)&ue_capabilities, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ue_capabilities fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(ue_capabilities != NULL){
        int access_indi,csg_indi;
        asn_INTEGER2long(&(ue_capabilities->access_stratum_release_indicator), (long*)&access_indi);
        asn_INTEGER2long(&(ue_capabilities->csg_indicator), (long*)&csg_indi);
        UERequestValue->Capabilities.accStrRelIndicator = access_indi;
        UERequestValue->Capabilities.csgIndicator = csg_indi;
    }
    IUH_FREE_OBJECT(ue_capabilities);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode HNBAP Context ID
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          ContextID
**          errflag
**          cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeHNBAPContextID(const char *buf, const int size, HNBAPCause *cause, char *ContextID, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    Context_ID_t *context_id;
    context_id = calloc(1, sizeof(Context_ID_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Context_ID, (void *)&context_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode context_id fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        *errflag = 1;
    }
    else if(context_id!= NULL){
        memset(ContextID, 0, CONTEXTID_LEN);
        memcpy(ContextID, context_id->buf, CONTEXTID_LEN);
        iuh_syslog_debug_debug(IUH_DEFAULT,"ContextID = %d\n",*ContextID);
    }
    IUH_FREE_OBJECT(context_id);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RUA Cause
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          errflag
**          sigMsg
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRUACause(const char *buf, const int size, Iuh2IuMsg *sigMsg, int *errflag)
{
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    RUA_Cause_t *cause;
    cause = calloc(1, sizeof(RUA_Cause_t));
    
    ret = uper_decode_complete(&code_ctx, &RUA_asn_DEF_Cause, (void **)&cause, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cause fail\n");
        *errflag = 1;
    }
    else if(cause != NULL){
        sigMsg->cause.present = cause->present;
        iuh_syslog_debug_debug(IUH_DEFAULT,"cause->present = %d\n",cause->present);
        switch(cause->present){
            int my_cause;
            case Cause_PR_radioNetwork_Rua: {
                asn_INTEGER2long(&(cause->choice.radioNetwork), (long*)&my_cause);
                sigMsg->cause.choice.radioNetwork = my_cause;
                iuh_syslog_debug_debug(IUH_DEFAULT,"cause->choice.radioNetwork = %d\n",my_cause);
                break;
            }
            case Cause_PR_transport: {
                asn_INTEGER2long(&(cause->choice.transport), (long*)&my_cause);
                sigMsg->cause.choice.transport = my_cause;
                iuh_syslog_debug_debug(IUH_DEFAULT,"cause->choice.transport = %d\n",my_cause);
                break;
            }
            case Cause_PR_protocol: {
                asn_INTEGER2long(&(cause->choice.protocol), (long*)&my_cause);
                sigMsg->cause.choice.protocol = my_cause;
                iuh_syslog_debug_debug(IUH_DEFAULT,"cause->choice.protocol = %d\n",my_cause);
                break;
            }
            case Cause_PR_misc: {
                asn_INTEGER2long(&(cause->choice.misc), (long*)&my_cause);
                sigMsg->cause.choice.misc = my_cause;
                iuh_syslog_debug_debug(IUH_DEFAULT,"cause->choice.misc = %d\n",my_cause);
                break;
            }
            default:
                break;
        }
    }
    IUH_FREE_OBJECT(cause);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode SourceRNC To TargetRNC Transparent Container
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          ContextID
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeSourceRNCToTargetRNCTransparentContainer(const char *buf, const int size, char cellId[CELLID_LEN], int *errflag)
{
    long tmp_cell_id = 0;
    asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
    SourceRNC_ToTargetRNC_TransparentContainer_t *sourceRNC_toTargetRNC_transparentContainer;
    sourceRNC_toTargetRNC_transparentContainer = calloc(1,sizeof(SourceRNC_ToTargetRNC_TransparentContainer_t));
 
    ret = uper_decode_complete(&code_ctx, &asn_DEF_SourceRNC_ToTargetRNC_TransparentContainer, (void **)&sourceRNC_toTargetRNC_transparentContainer, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode sourceRNC_toTargetRNC_transparentContainer fail\n");
        *errflag = 1;
    }
    else if(sourceRNC_toTargetRNC_transparentContainer != NULL){
		if(sourceRNC_toTargetRNC_transparentContainer->targetCellId != NULL){
		    tmp_cell_id = *(sourceRNC_toTargetRNC_transparentContainer->targetCellId);
		    if(tmp_cell_id != 0){
		        char tmp_cell[CELLID_LEN] = {0};
		        memcpy(tmp_cell, &tmp_cell_id, CELLID_LEN);
		        iuh_hnb_show_str("tmp_cell_id = ", tmp_cell, 0, CELLID_LEN);
		        int i = 0;
		        for(i = 0; i < CELLID_LEN; i++){
		            cellId[i] = tmp_cell[CELLID_LEN-i-1];
		        }
		        iuh_hnb_show_str("cellId = ", cellId, 0, CELLID_LEN);
		    }
		}
    }
    IUH_FREE_OBJECT(sourceRNC_toTargetRNC_transparentContainer);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RANAP CnDomain Indicator
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          ContextID
**          errflag
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeRANAPCnDomainIndicator(const char *buf, const int size, CNDomain *CnDomain, int *errflag)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeRANAPCnDomainIndicator\n");
    asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	RANAP_CN_DomainIndicator_t *cn_domain_id;
    cn_domain_id = calloc(1, sizeof(RANAP_CN_DomainIndicator_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RANAP_CN_DomainIndicator, (void **)&cn_domain_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode cn_domain_id fail\n");
        *errflag = 1;
    }
    else if(cn_domain_id != NULL){
        asn_INTEGER2long((INTEGER_t *)cn_domain_id, (long*)CnDomain);
    }
    IUH_FREE_OBJECT(cn_domain_id);
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode RANAP iu signal connection id
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          iu connection id
** RETURN:
**          true
**          false
** DATE:	book add at 2011-12-16
*****************************************************/
IuhBool IuhDecodeRANAPIuSigConId(const char *buf, const int size, char *iuSigConId)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeRANAPIuSigConId\n");
    asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	IuSignallingConnectionIdentifier_t *iu_sig_con_id;
    iu_sig_con_id = calloc(1, sizeof(IuSignallingConnectionIdentifier_t));
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_IuSignallingConnectionIdentifier, (void **)&iu_sig_con_id, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("Error: decode iu_sig_con_id fail\n");
		return Iuh_FALSE;
    }
    else if(iu_sig_con_id != NULL){
		memcpy(iuSigConId, iu_sig_con_id->buf, iu_sig_con_id->size);
    }
    IUH_FREE_OBJECT(iu_sig_con_id);
    
    return Iuh_TRUE;
}




#if 0
/*****************************************************
** DISCRIPTION:
**          Search UeId by ContextId
** INPUT:
**          ContextId
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
*****************************************************/

int sccp_find_UE_by_ctxid(char * contextId)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FIND_UE_BY_CTXID \n");
    int UEID = 0;
    int i;
    if(contextId == NULL) return 0;
    
    for(i = 0; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if((IUH_HNB[i] != NULL)){
            Iuh_HNB_UE *tempUE = IUH_HNB[i]->HNB_UE;
            while(tempUE != NULL){
                if(memcmp(tempUE->context_id_str, contextId, CONTEXTID_LEN) == 0){
                    UEID = tempUE->UEID;
					iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d \n",UEID);
                    break;
                }
                tempUE = tempUE->ue_next;
            }
        }
    }
    
    return UEID;
}
#endif
/* @@@@@@@@@@@@@@@   Decode Items End  @@@@@@@@@@@@@@@ */




/* @@@@@@@@@@@@@@@   Decode Messages Begin  @@@@@@@@@@@@@@@ */


/*****************************************************
** DISCRIPTION:
**          Parse 1st level of hnbap message 
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          hnbap pdu
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhParseHnbapMessage(const char *buf,const int readBytes, HNBAP_PDU_t *hnbap_pdu)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"parse hnbap pdu\n");
    //printf("ccccccccccccc\n");
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNBAP_PDU, (void **)&hnbap_pdu, buf, readBytes);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode pdu over, result is %d\n", ret.code);
    //printf("dddddddddddddd\n");
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode hnbap_pdu fail\n");
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhParseHnbapMessage more buf need to be decode\n");
        if(!IuhParseHnbapMessage(buf, readBytes, hnbap_pdu)){
            return Iuh_FALSE;
        }
    }
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Parse 1st level of Rua message
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          Rua pdu
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhParseRuaMessage(const char *buf,const int readBytes, RUA_PDU_t *rua_pdu)
{
    iuh_syslog_debug_debug(IUH_DEFAULT, "parse rua pdu\n");
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RUA_PDU, (void **)&rua_pdu, buf, readBytes);
    iuh_syslog_debug_debug(IUH_DEFAULT, "decode pdu over, result is %d\n", ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rua_pdu fail\n");
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhParseRuaMessage more buf need to be decode\n");
        if(!IuhParseRuaMessage(buf, readBytes, rua_pdu)){
            return Iuh_FALSE;
        }
    }
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode hnb register request message
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          HnbRegisterRequestValue struct
**          reject cause
** RETURN:
**          true
**          false
*****************************************************/
IuhBool IuhDecodeHnbRegisterRequest(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        HNBAP  HNB-REGISTER-REQUEST\n");
	iuh_syslog_info("###  Length        :        %d\n", size);

	
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHnbRegisterRequest\n");
    HNBRegisterRequest_t *hnb_reg_request;
    hnb_reg_request = calloc(1,sizeof(HNBRegisterRequest_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNBRegisterRequest, (void **)&hnb_reg_request, buf, size);

    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode hnb_register_request fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(hnb_reg_request);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHnbRegisterRequest more buf need to be decode \n");
        if(!IuhDecodeHnbRegisterRequest(buf, size, requestValue, cause)){
            return Iuh_FALSE;
        }
    }

    if(hnb_reg_request->protocolIEs.list.count < 7){
        iuh_syslog_err("hnb_register_request members are not enough\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(hnb_reg_request);
        return Iuh_FALSE;
    }

    int errflag = 0;
    
    for(i = 0; i < hnb_reg_request->protocolIEs.list.count; i++){
        switch(hnb_reg_request->protocolIEs.list.array[i]->id){
            case HNBAP_HNB_Ident: {
                // 1 hnb_identity
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeHHBIdentity(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_HNB_Location_Info: {  
                // 2 hnb_location_information
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeHHBLocationInformation(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_PLMNid: {
                // 3 plmn_idendity
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodePLMNIdentity(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_CellId: {
                // 4 cell_identity
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeCellIdentity(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_LAC: {
                // 5 lac
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeLAC(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
           
                break;
            }
            case HNBAP_RAC: {
                // 6 rac
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeRAC(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
           
                break;
            }
            case HNBAP_SAC: {
                // 7 sac
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeSAC(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_CSG_ID: {
                //8 csg_id
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeCSGID(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
                
                break;
            }
            case HNBAP_Service_Area_For_Broadcast: {
                break;
            }
            case HNBAP_HNB_Cell_Access_Mode: {
                // access_mode,  book add, 2012-1-4
                initial_msg = &(hnb_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeCellAccessMode(initial_msg->buf, initial_msg->size, requestValue, cause, &errflag);
                break;
            }
            default:
                break;
        }
    }
 
    IUH_FREE_OBJECT(hnb_reg_request);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode hnb deregister message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
IuhBool IuhDecodeHnbDeregister(const char *buf, const int size, HNBAPCause *cause)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        HNBAP  HNB-DEREGISTER\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
	
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHnbDeregister\n");
    HNBDe_Register_t *hnb_deregister;
    hnb_deregister = calloc(1, sizeof(HNBDe_Register_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_HNBDe_Register, (void **)&hnb_deregister, buf, size);

    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode hnb_deregister fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(hnb_deregister);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHnbDeregister more buf need to be decode \n");
        if(!IuhDecodeHnbDeregister(buf, size, cause)){
            return Iuh_FALSE;
        }
    }

    if(hnb_deregister->protocolIEs.list.count < 1){
        iuh_syslog_err("deregister_request member is wrong\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(hnb_deregister);
        return Iuh_FALSE;
    }

    int errflag = 0;
    
    for(i = 0; i < hnb_deregister->protocolIEs.list.count; i++){
        switch(hnb_deregister->protocolIEs.list.array[i]->id){
            case HNBAP_id_Cause: {
                initial_msg = &(hnb_deregister->protocolIEs.list.array[i]->value);
                IuhDecodeHNBAPCause(initial_msg->buf, initial_msg->size, cause, &errflag);
            
                break;
            }
            case HNBAP_BackoffTimer: {
                initial_msg = &(hnb_deregister->protocolIEs.list.array[i]->value);
                IuhDecodeBackoffTimer(initial_msg->buf, initial_msg->size, cause, &errflag);
            
                break;
            }
            default:
                break;
        }
    }

    
    IUH_FREE_OBJECT(hnb_deregister);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          decode ue register request message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
IuhBool IuhDecodeUERegisterRequest(const char *buf, const int size, UERegisterRequestValue *UERequestValue, HNBAPCause *cause)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-REGISTER-REQUEST\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeUERegisterRequest\n");
    UERegisterRequest_t *ue_reg_request;
    ue_reg_request = (UERegisterRequest_t *)calloc(1,sizeof(UERegisterRequest_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode UE_register_request\n");
    ret = uper_decode_complete(&code_ctx, &asn_DEF_UERegisterRequest, (void **)&ue_reg_request, buf, size);
    
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode UE_register_request fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(ue_reg_request);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeUERegisterRequest more buf need to be decode \n");
        if(!IuhDecodeUERegisterRequest(buf, size, UERequestValue, cause)){
            return Iuh_FALSE;
        }
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode UE_register_request ok\n");
    if(ue_reg_request->protocolIEs.list.count != 3){
        iuh_syslog_err("ue_register_request members are not enough\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(ue_reg_request);
        return Iuh_FALSE;
    }

    int errflag = 0;
    for(i = 0; i < ue_reg_request->protocolIEs.list.count; i++){
        switch(ue_reg_request->protocolIEs.list.array[i]->id){
            case HNBAP_UE_Identity: {
                // 1 ue_identity
                initial_msg = &(ue_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeUEIdentity(initial_msg->buf, initial_msg->size, UERequestValue, cause, &errflag);
           
                break;
            }
            case HNBAP_Registration_Cause: {
                // 2 ue_registration_cause
                initial_msg = &(ue_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeUERegistrationCause(initial_msg->buf, initial_msg->size, UERequestValue, cause, &errflag);
            
                break;
            }
            case HNBAP_UE_Capabilities: {
                // 3 ue_capabilities
                initial_msg = &(ue_reg_request->protocolIEs.list.array[i]->value);
                IuhDecodeUECapabilities(initial_msg->buf, initial_msg->size, UERequestValue, cause, &errflag);
            
                break;
            }
            default:
                break;
        }
    }
    
    IUH_FREE_OBJECT(ue_reg_request);

    if(errflag == 1) return Iuh_FALSE;

    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ue register request over\n");
    
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode ue deregister request message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/
IuhBool IuhDecodeUEDeregister(const char *buf, const int size, HNBAPCause *cause, char *ContextID)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        HNBAP  UE-DEREGISTER\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeUEDeregister\n");
    UEDe_Register_t *ue_deregister;
    ue_deregister = calloc(1,sizeof(UEDe_Register_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_UEDe_Register, (void *)&ue_deregister, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT, "decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ue_deregister fail\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(ue_deregister);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHnbDeregister more buf need to be decode \n");
        if(!IuhDecodeHnbDeregister(buf, size, cause)){
            return Iuh_FALSE;
        }
    }

    if(ue_deregister->protocolIEs.list.count != 2){
        iuh_syslog_err("deregister_request member is wrong\n");
        cause->present = Cause_PR_protocol;
        cause->choice.protocol = Protocol_abstract_syntax_error_reject;
        IUH_FREE_OBJECT(ue_deregister);
        return Iuh_FALSE;
    }

    int errflag = 0;
    for(i = 0; i < ue_deregister->protocolIEs.list.count; i++){
        switch(ue_deregister->protocolIEs.list.array[i]->id){
            case HNBAP_Context_ID: {
                //context_id
                initial_msg = &(ue_deregister->protocolIEs.list.array[i]->value);
                IuhDecodeHNBAPContextID(initial_msg->buf, initial_msg->size, cause, ContextID, &errflag);
            
                break;
            }
            case HNBAP_id_Cause: {
                initial_msg = &(ue_deregister->protocolIEs.list.array[i]->value);
                IuhDecodeHNBAPCause(initial_msg->buf, initial_msg->size, cause, &errflag);
            
                break;
            }
            default:
                break;
        }
    }

    IUH_FREE_OBJECT(ue_deregister);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ue deregister ok\n");
    
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode rua connect message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeConnect(const char *buf, const int size, Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RUA  CONNECT\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeConnect\n");
    Connect_t *rua_connect;
    rua_connect = calloc(1,sizeof(Connect_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Connect, (void **)&rua_connect, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rua_connect fail\n");
        IUH_FREE_OBJECT(rua_connect);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeConnect more buf need to be decode \n");
        if(!IuhDecodeConnect(buf, size, sigMsg)){
            return Iuh_FALSE;
        }
    }

    if(rua_connect->protocolIEs.list.count < 4){
        iuh_syslog_err("rua_connect member is wrong\n");
        IUH_FREE_OBJECT(rua_connect);
        return Iuh_FALSE;
    }

    int errflag = 0;
    for(i = 0; i < rua_connect->protocolIEs.list.count; i++){
        switch(rua_connect->protocolIEs.list.array[i]->id){
            case RUA_CN_DomainIndicator: {
                //cn_domain_id
                initial_msg = &(rua_connect->protocolIEs.list.array[i]->value);
                IuhDecodeRUACnDomainIndicator(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
          
                break;
            }
            case RUA_Context_ID: {
                //context_id
                initial_msg = &(rua_connect->protocolIEs.list.array[i]->value);
                IuhDecodeRUAContextID(initial_msg->buf, initial_msg->size, sigMsg, &errflag);

                break;
            }
            case RUA_Establishment_Cause: {
                //establishment_cause
                initial_msg = &(rua_connect->protocolIEs.list.array[i]->value);
                IuhDecodeEstablishmentCause(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
           
                break;
            }
            case RUA_RANAP_Message: {
                //ranap_msg
                initial_msg = &(rua_connect->protocolIEs.list.array[i]->value);
                IuhDecodeRUARanapMessage(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
            
                break;
            }
            case RUA_IntraDomainNasNodeSelector: {
                //IntraDomainNasNodeSelector_t *intra_domain_nas_node_selector; 
                //intra_domain_nas_node_selector = calloc(1, sizeof(IntraDomainNasNodeSelector_t));
                
                //IUH_FREE_OBJECT(intra_domain_nas_node_selector);
                break;
            }
            case RUA_vCSGMembershipStatus: {
                //csg_membership_status
                initial_msg = &(rua_connect->protocolIEs.list.array[i]->value);
                IuhDecodeCSGMembershipStatus(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
            
                break;
            }
            default:
                break;
        }
    }

    IUH_FREE_OBJECT(rua_connect);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode rua connect ok\n");

	/* Print Log Information*/
	iuh_syslog_info("###  RANAPMsgLen  :        %d\n", sigMsg->RanapMsg.size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Deconde rua direct transfer message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeDirectTransfer(const char *buf, const int size, Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RUA  DIRECT-TRANSFER\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeDirectTransfer\n");
    DirectTransfer_t *direct_transfer;
    direct_transfer = calloc(1,sizeof(DirectTransfer_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_DirectTransfer, (void **)&direct_transfer, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode direct_transfer fail\n");
        IUH_FREE_OBJECT(direct_transfer);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeDirectTransfer more buf need to be decode \n");
        if(!IuhDecodeDirectTransfer(buf, size, sigMsg)){
            return Iuh_FALSE;
        }
    }

    if(direct_transfer->protocolIEs.list.count != 3){
        iuh_syslog_err("direct_transfer member is wrong\n");
        IUH_FREE_OBJECT(direct_transfer);
        return Iuh_FALSE;
    }

    int errflag = 0;
    for(i = 0; i < direct_transfer->protocolIEs.list.count; i++){
        switch(direct_transfer->protocolIEs.list.array[i]->id){
            case RUA_CN_DomainIndicator: {
                //cn_domain_id
                initial_msg = &(direct_transfer->protocolIEs.list.array[i]->value);
                IuhDecodeRUACnDomainIndicator(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
          
                break;
            }
            case RUA_Context_ID: {
                //context_id
                initial_msg = &(direct_transfer->protocolIEs.list.array[i]->value);
                IuhDecodeRUAContextID(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
                
                break;
            }
            case RUA_RANAP_Message: {
                //ranap_msg
                initial_msg = &(direct_transfer->protocolIEs.list.array[i]->value);
                IuhDecodeRUARanapMessage(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
            
                break;
            }
            default:
                break;
        }
    }

    IUH_FREE_OBJECT(direct_transfer);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode direct transfer ok\n");

	/* Print Log Information*/
	iuh_syslog_info("###  RANAPMsgLen  :        %d\n", sigMsg->RanapMsg.size);
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode rua disconnect message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeDisconnect(const char *buf, const int size, Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RUA  DISCONNECT\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeDisconnect\n");
    Disconnect_t *rua_disconnect;
    rua_disconnect = calloc(1,sizeof(Disconnect_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_Disconnect, (void **)&rua_disconnect, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rua_disconnect fail\n");
        IUH_FREE_OBJECT(rua_disconnect);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT, "IuhDecodeDisconnect more buf need to be decode \n");
        if(!IuhDecodeDisconnect(buf, size, sigMsg)){
            return Iuh_FALSE;
        }
    }

    if(rua_disconnect->protocolIEs.list.count < 3){
        iuh_syslog_err("rua_disconnect member is wrong\n");
        IUH_FREE_OBJECT(rua_disconnect);
        return Iuh_FALSE;
    }

    int errflag = 0;
    for(i = 0; i < rua_disconnect->protocolIEs.list.count; i++){
        switch(rua_disconnect->protocolIEs.list.array[i]->id){
            case RUA_CN_DomainIndicator: {
                //cn_domain_id
                initial_msg = &(rua_disconnect->protocolIEs.list.array[i]->value);
                IuhDecodeRUACnDomainIndicator(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
          
                break;
            }
            case RUA_Context_ID: {
                //context_id
                initial_msg = &(rua_disconnect->protocolIEs.list.array[i]->value);
                IuhDecodeRUAContextID(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
                
                break;
            }
            case RUA_Cause: {
                //cause
                initial_msg = &(rua_disconnect->protocolIEs.list.array[i]->value);
                IuhDecodeRUACause(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
               
                break;
            }
            case RUA_RANAP_Message: {
                //cause
                initial_msg = &(rua_disconnect->protocolIEs.list.array[i]->value);
                IuhDecodeRUARanapMessage(initial_msg->buf, initial_msg->size, sigMsg, &errflag);
               
                break;
            }
            default:
                break;
        }
    }
    
    IUH_FREE_OBJECT(rua_disconnect);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode rua connect ok\n");

	/* Print Log Information*/
	iuh_syslog_info("###  RANAPMsgLen  :        %d\n", sigMsg->RanapMsg.size);
	
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode rua connectionless transfer message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeConnectionlessTransfer(const char *buf, const int size, Iuh2IuMsg *sigMsg)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RUA  CONNECTIONLESS-TRANSFER\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeConnectionlessTransfer\n");
    ConnectionlessTransfer_t *connectionless_transfer;
    connectionless_transfer = calloc(1,sizeof(ConnectionlessTransfer_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_ConnectionlessTransfer, (void **)&connectionless_transfer, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT, "decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode connectionless_transfer fail\n");
        IUH_FREE_OBJECT(connectionless_transfer);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT, "IuhDecodeConnectionlessTransfer more buf need to be decode \n");
        if(!IuhDecodeConnectionlessTransfer(buf, size, sigMsg)){
            return Iuh_FALSE;
        }
    }

    if(connectionless_transfer->protocolIEs.list.count != 1){
        iuh_syslog_err("connectionless_transfer member is wrong\n");
        IUH_FREE_OBJECT(connectionless_transfer);
        return Iuh_FALSE;
    }
    
    RANAP_Message_t *ranap_msg;
    ranap_msg = calloc(1, sizeof(RANAP_Message_t));
    
    int errflag = 0;
    {   
        //ranap_msg
        initial_msg = &(connectionless_transfer->protocolIEs.list.array[0]->value);
        ret = uper_decode_complete(&code_ctx, &asn_DEF_RANAP_Message, (void **)&ranap_msg, (void *)initial_msg->buf, initial_msg->size);
        if(ret.code == RC_FAIL){
            iuh_syslog_err("decode ranap_msg fail\n");
            errflag = 1;
        }
    }
    
    if(errflag == 0){
        //ranap_msg
        if(ranap_msg != NULL){
            sigMsg->RanapMsg.size = ranap_msg->size;
            memcpy(sigMsg->RanapMsg.RanapMsg, (void *)ranap_msg->buf, ranap_msg->size);
        }
    }

    IUH_FREE_OBJECT(ranap_msg);
    IUH_FREE_OBJECT(connectionless_transfer);

    if(errflag == 1){
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode connectionless_transfer transfer ok\n");

	/* Print Log Information*/
	iuh_syslog_info("###  RANAPMsgLen  :        %d\n", sigMsg->RanapMsg.size);
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          update exist rab information by rabid
** INPUT:
**          ueid, rabid
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void Iuh_update_ue_rab(int UEID, Iu_RAB *myRab)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_update_ue_rab\n");
    return;    
}


/*****************************************************
** DISCRIPTION:
**          check rabid in ue information
** INPUT:
**          ueid
** OUTPUT:
**          null
** RETURN:
**          1       exist
**          0       not exist
*****************************************************/
int Iuh_check_rab_id(int UEID, char *RABID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_check_rab_id\n");
    if(IUH_HNB_UE[UEID] == NULL){
        iuh_syslog_err("invalid ue id\n");
        return 0;
    }
    
    Iu_RAB *temp_rab = IUH_HNB_UE[UEID]->UE_RAB;
    
    while(temp_rab != NULL){
        if(memcpy(temp_rab->RABID, RABID, RAB_ID_LEN) == 0)
            return 1;
        else{
            temp_rab = temp_rab->rab_next;
        }
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_check_rab_id over\n");
    return 0;
}



/*****************************************************
** DISCRIPTION:
**          Decode rab assignment request message
** INPUT:
**          buf
**          buf length
** OUTPUT:
**          rab assignment information struct
** RETURN:
**          1       succeed
**          0       fail
*****************************************************/
IuhBool Iuh_decode_RAB_Assignment(const char *buf, const int size, const int UEID)
{
	/* Print Log Information */
	iuh_syslog_info("###  ---------------------------------\n");
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RANAP  RAB-ASSIGNMENT-REQUEST\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
	iuh_syslog_info("###  ---------------------------------\n");
    //iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_decode_RAB_Assignment\n");
    RAB_AssignmentRequest_t *rab_assignment_request;
    rab_assignment_request = calloc(1, sizeof(RAB_AssignmentRequest_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_AssignmentRequest, (void **)&rab_assignment_request, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT, "decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rab_assignment_request fail\n");
        IUH_FREE_OBJECT(rab_assignment_request);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT, "sccp decode rab assignment request more buf need to be decode \n");
        if(!Iuh_decode_RAB_Assignment(buf, size, UEID)){
            return Iuh_FALSE;
        }
    }
    
    int errflag = 0;
    for(i = 0; i < rab_assignment_request->protocolIEs.list.count; i++){
        iuh_syslog_debug_debug(IUH_DEFAULT, "rab_assignment_request IE type is %d \n", rab_assignment_request->protocolIEs.list.array[i]->id);
        switch(rab_assignment_request->protocolIEs.list.array[i]->id){
            case id_RAB_SetupOrModifyList: {
                RAB_SetupOrModifyList_t *rab_setupormodify_list;
                rab_setupormodify_list = calloc(1,sizeof(RAB_SetupOrModifyList_t));
                initial_msg = &(rab_assignment_request->protocolIEs.list.array[i]->value);
                ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_SetupOrModifyList, (void **)&rab_setupormodify_list, initial_msg->buf, initial_msg->size);
                if(ret.code == RC_FAIL){
                    iuh_syslog_err("decode ranap rab_setupormodify_list fail\n");
                    errflag = 1;
                }
                else if(rab_setupormodify_list != NULL){
                    /* malloc room for current rab */
                    Iu_RAB *myRab = malloc(sizeof(Iu_RAB));
                    memset(myRab, 0, sizeof(Iu_RAB));
                    
                    if(Iuh_rab_setupOfModifyList(rab_setupormodify_list, myRab) == -1){
                        iuh_syslog_err("decode ranap rab_setupormodify_list failed\n");
                    }
                    else{
                        /* create new rab in ue */
                        if(0 == Iuh_check_rab_id(UEID, myRab->RABID)){
                            iuh_syslog_debug_debug(IUH_DEFAULT,"not find rab id\n");
                            myRab->rab_next = IUH_HNB_UE[UEID]->UE_RAB;
                            IUH_HNB_UE[UEID]->UE_RAB = myRab;
                            IUH_HNB_UE[UEID]->rab_count++;
                        }
                        /* update exist rab information of ue */
                        else{
                            iuh_syslog_debug_debug(IUH_DEFAULT,"find rab id\n");
                            //Iuh_update_ue_rab(UEID, myRab);
                            myRab->rab_next = IUH_HNB_UE[UEID]->UE_RAB;
                            IUH_HNB_UE[UEID]->UE_RAB = myRab;
                            IUH_HNB_UE[UEID]->rab_count++;
                        }
                    }
                }
                IUH_FREE_OBJECT(rab_setupormodify_list);
                iuh_syslog_debug_debug(IUH_DEFAULT,"free rab_setupormodify_list\n");
                break;
            }
            case id_RAB_ToBeReleasedList_EnhancedRelocCompleteRes: {
                break;
            }
            case id_UE_AggregateMaximumBitRate: {
                break;
            }
            default:
                break;
                 
        }
    }

    IUH_FREE_OBJECT(rab_assignment_request);

    if(errflag == 1){
        iuh_syslog_debug_debug(IUH_DEFAULT,"decode rab assignment request failed\n");
        return Iuh_FALSE;
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode rab assignment request ok\n");
    
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          decode rab setup or modify list
** INPUT:
**          RAB_SetupOrModifyList_t *
** OUTPUT:
**          rab information
** RETURN:
**          0       succeed
**          -1      fail
*****************************************************/

int Iuh_rab_setupOfModifyList(RAB_SetupOrModifyList_t *rab_setupormodify_list, Iu_RAB *myRab)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_rab_setupOfModifyList\n");
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    int errflag = 0;
    int j,k;
    
    for(j = 0; j < rab_setupormodify_list->list.count; j++){
        ProtocolIE_ContainerPair_163P0_t *protocolIE_container_pair = rab_setupormodify_list->list.array[j];
        if(protocolIE_container_pair == NULL){
            iuh_syslog_err("decode ranap protocolIE_container_pair fail\n");
            errflag = -1;
        }
        else{
            for(k = 0; k < protocolIE_container_pair->list.count; k++){
                Member_ProtocolIE_ContainerPair_t *rab_setupormodify_item = protocolIE_container_pair->list.array[k];
                if(rab_setupormodify_item == NULL){
                    iuh_syslog_err("decode ranap rab_setupormodify_item fail\n");
                    errflag = -1;
                }
                else{
                printf("print rab-setupormodify-item \n");
                /* add log to show xml format of rab-setupormodify-item, book add 2011-10-08 */
                    switch(rab_setupormodify_item->id){
                        case id_RAB_SetupOrModifyItem:{
                        /* first  RAB information */
                            RAB_SetupOrModifyItemFirst_t *rab_first;
                            rab_first = calloc(1, sizeof(RAB_SetupOrModifyItemFirst_t));
                            ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_SetupOrModifyItemFirst, (void **)&rab_first, rab_setupormodify_item->firstValue.buf, rab_setupormodify_item->firstValue.size);
                            if(ret.code == RC_FAIL){
                                iuh_syslog_err("decode ranap rab_first fail\n");
                                errflag = -1;
                            }
                            else if(rab_first != NULL){
                                /* rAB_ID                                       M */
                                memcpy(myRab->RABID, rab_first->rAB_ID.buf, rab_first->rAB_ID.size);
                                
                                /* nAS_SynchronisationIndicator */
                                if(rab_first->nAS_SynchronisationIndicator != NULL){
                                    memcpy(myRab->nAS_ScrIndicator, rab_first->nAS_SynchronisationIndicator->buf, rab_first->nAS_SynchronisationIndicator->size);
                                }
                                
                                /* rAB_Parameters */
                                if(rab_first->rAB_Parameters != NULL){
                                    /* trafficClass */
                                    asn_INTEGER2long(&(rab_first->rAB_Parameters->trafficClass), (long *)&(myRab->rab_para_list.traffic_class));
                                    /* rAB_AsymmetryIndicator */
                                    asn_INTEGER2long(&(rab_first->rAB_Parameters->rAB_AsymmetryIndicator), (long *)&(myRab->rab_para_list.rab_ass_indicator));
                                    /* maxBitrate */
                                    /* guaranteedBitRate */
                                    /* deliveryOrder */
                                    asn_INTEGER2long(&(rab_first->rAB_Parameters->deliveryOrder), (long *)&(myRab->rab_para_list.deliveryOrder));
                                    /* maxSDU_Size */
                                    myRab->rab_para_list.maxSDUSize = rab_first->rAB_Parameters->maxSDU_Size;
                                    /* sDU_Parameters */
                                    /* transferDelay */
                                    /* trafficHandlingPriority */
                                    /* allocationOrRetentionPriority */
                                    /* sourceStatisticsDescriptor */
                                    /* relocationRequirement */
                                }
                                
                                /* userPlaneInformation */
                                if(rab_first->userPlaneInformation != NULL){
                                    asn_INTEGER2long(&(rab_first->userPlaneInformation->userPlaneMode), \
                                        (long *)&(myRab->user_plane_info.user_plane_mode));
                                    memcpy(myRab->user_plane_info.up_modeVersions, rab_first->userPlaneInformation->uP_ModeVersions.buf, \
                                        rab_first->userPlaneInformation->uP_ModeVersions.size);
                                }
                                
                                /* transportLayerInformation */
                                if(rab_first->transportLayerInformation != NULL){
                                    memcpy(myRab->trans_layer_info.transport_layer_addr, \
                                        rab_first->transportLayerInformation->transportLayerAddress.buf, \
                                        rab_first->transportLayerInformation->transportLayerAddress.size);
                                    myRab->trans_layer_info.iu_trans_assoc.present = \
                                        rab_first->transportLayerInformation->iuTransportAssociation.present;
                                    if(myRab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_gTP_TEI){
                                        memcpy(myRab->trans_layer_info.iu_trans_assoc.choice.gtp_tei, \
                                            rab_first->transportLayerInformation->iuTransportAssociation.choice.gTP_TEI.buf, \
                                            rab_first->transportLayerInformation->iuTransportAssociation.choice.gTP_TEI.size);
                                    }
                                    else if(myRab->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_bindingID){
                                        memcpy(myRab->trans_layer_info.iu_trans_assoc.choice.binding_id, \
                                            rab_first->transportLayerInformation->iuTransportAssociation.choice.bindingID.buf, \
                                            rab_first->transportLayerInformation->iuTransportAssociation.choice.bindingID.size);
                                    }
                                }
                                /* service_Handover */
                                if(rab_first->service_Handover != NULL){
                                    asn_INTEGER2long(&(rab_first->service_Handover), (long *)&(myRab->service_handover));
                                }
                            }

                            
                        /* second       GTP-U information */
                        /* if has second part, it is a ps domain message and it is for gtp  */
                            RAB_SetupOrModifyItemSecond_t *rab_second;
                            rab_second = calloc(1, sizeof(RAB_SetupOrModifyItemSecond_t));
                            ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_SetupOrModifyItemSecond, (void **)&rab_second, rab_setupormodify_item->secondValue.buf, rab_setupormodify_item->secondValue.size);
                            if(ret.code == RC_FAIL){
                                iuh_syslog_err("decode ranap rab_second fail\n");
                                errflag = -1;
                            }
                            else if(rab_second != NULL){
                                myRab->isPsDomain = 1;
                                /* pDP_TypeInformation */
                                if(rab_second->pDP_TypeInformation != NULL){
                                    int m;
                                    myRab->pdp_type_list = malloc(sizeof(PDPTypeList));
                                    myRab->pdp_type_list->count = rab_second->pDP_TypeInformation->list.count;
                                    for(m = 0; m < rab_second->pDP_TypeInformation->list.count; m++){
                                        struct pdpTypeInfo *pdpInfo = (struct pdpTypeInfo *)malloc(sizeof(struct pdpTypeInfo));
										memset(pdpInfo, 0, sizeof(struct pdpTypeInfo));
                                        asn_INTEGER2long(rab_second->pDP_TypeInformation->list.array[m], (long *)&(pdpInfo->pdp_type));
                                        
                                        pdpInfo->next = myRab->pdp_type_list->pdp_type_info;
                                        myRab->pdp_type_list->pdp_type_info = pdpInfo;
                                    }
                                }
                                /* dataVolumeReportingIndication */
                                if(rab_second->dataVolumeReportingIndication != NULL){
                                    asn_INTEGER2long(rab_second->dataVolumeReportingIndication, (long *)&(myRab->data_vol_rpt));
                                }
                                /* dl_GTP_PDU_SequenceNumber */
                                if(rab_second->dl_GTP_PDU_SequenceNumber != NULL){
                                    myRab->dl_GTP_PDU_SeqNum = *(rab_second->dl_GTP_PDU_SequenceNumber);
                                }
                                /* ul_GTP_PDU_SequenceNumber */
                                if(rab_second->ul_GTP_PDU_SequenceNumber != NULL){
                                    myRab->ul_GTP_PDU_seqNum = *(rab_second->ul_GTP_PDU_SequenceNumber);
                                }
                                /* dl_N_PDU_SequenceNumber */
                                if(rab_second->dl_N_PDU_SequenceNumber != NULL){
                                    myRab->dl_N_PDU_SeqNum = *(rab_second->dl_N_PDU_SequenceNumber);
                                }
                                /* ul_N_PDU_SequenceNumber */    
                                if(rab_second->ul_N_PDU_SequenceNumber != NULL){
                                    myRab->ul_N_PDU_SeqNum = *(rab_second->ul_N_PDU_SequenceNumber);
                                }
                            }
                            IUH_FREE_OBJECT(rab_first);
                            IUH_FREE_OBJECT(rab_second);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
    }
    
    return errflag;
}


/*****************************************************
** DISCRIPTION:
**          decode RAB release request message
** INPUT:
**          buf
**          buf length
**          ueid
** OUTPUT:
**          null
** RETURN:
**          1       succeed
**          0      fail
*****************************************************/
int Iuh_decode_RAB_Release(const char *buf, const int size, const int UEID)
{
	/* Print Log Information */
	iuh_syslog_info("###  ---------------------------------\n");
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RANAP  RAB-RELEASE\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
	iuh_syslog_info("###  ---------------------------------\n");
	
    //iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_decode_RAB_Release\n");
    RAB_ReleaseRequest_t *rab_release_request;
    rab_release_request = calloc(1, sizeof(RAB_ReleaseRequest_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i,j,k;
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_ReleaseRequest, (void **)&rab_release_request, buf, size);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("Error: decode rab_release_request fail\n");
        IUH_FREE_OBJECT(rab_release_request);
        return Iuh_FALSE;
    }

    for(i = 0; i < rab_release_request->protocolIEs.list.count; i++){
        if(rab_release_request->protocolIEs.list.array[i]->id == id_RAB_ReleaseList){
            RAB_ReleaseList_t *rab_release_list;
            rab_release_list = calloc(1, sizeof(RAB_ReleaseList_t));
            ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_ReleaseList, (void **)&rab_release_list, rab_release_request->protocolIEs.list.array[i]->value.buf, rab_release_request->protocolIEs.list.array[i]->value.size);
            if(ret.code == RC_FAIL){
                iuh_syslog_err("Error: decode rab_release_list fail\n");
                IUH_FREE_OBJECT(rab_release_list);
                break;
            }
            
            for(j = 0; j < rab_release_list->list.count; j++){
                for(k = 0; k < rab_release_list->list.array[j]->list.count; k++){
                    RAB_ReleaseItem_t *rab_release_item;
                    rab_release_item = calloc(1, sizeof(RAB_ReleaseItem_t));
                    initial_msg = &(rab_release_list->list.array[j]->list.array[k]->value);
                    
                    ret = uper_decode_complete(&code_ctx, &asn_DEF_RAB_ReleaseItem, (void **)&rab_release_item, initial_msg->buf, initial_msg->size);
                    if(ret.code == RC_FAIL){
                        iuh_syslog_err("Error: decode rab_release_item fail\n");
                    }
                    else if(rab_release_item != NULL){
                        Iuh_release_rab(UEID, rab_release_item->rAB_ID.buf);
						iuh_syslog_debug_debug(IUH_DEFAULT, "iuh release rab successful\n");
                    }
                    IUH_FREE_OBJECT(rab_release_item);
                }
            }
            IUH_FREE_OBJECT(rab_release_list);
        }
    }
    IUH_FREE_OBJECT(rab_release_request);
	iuh_syslog_debug_debug(IUH_DEFAULT, "Iuh_decode_RAB_Release finish\n");
    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          decode ranap message
** INPUT:
**          buf
**          buf length
**          ranap pdu
**          ueid
** OUTPUT:
**          null
** RETURN:
**          true       succeed
**          false      fail
*****************************************************/

IuhBool IuhDecodeRanapMessage(const char* buf, const int size, RANAP_PDU_t *ranap_pdu)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeRanapMessage\n");
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ret = uper_decode_complete(&code_ctx, &asn_DEF_RANAP_PDU, (void **)&ranap_pdu, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ranap pdu over, result is %d\n", ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ranap_pdu fail\n");
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeRanapMessage more buf need to be decode\n");
        if(!IuhDecodeRanapMessage(buf, size, ranap_pdu)){
            return Iuh_FALSE;
        }
    }

	return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**          Decode hnbap error indication message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeHNBAPErrorIndication(const char *buf, const int size, HNBAPCause *cause)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        HNBAP  ERROR-INDICATION\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHNBAPErrorIndication\n");
    Hnbap_ErrorIndication_t *hnbap_error_indication;
    hnbap_error_indication = calloc(1,sizeof(Hnbap_ErrorIndication_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &HNBAP_asn_DEF_ErrorIndication, (void *)&hnbap_error_indication, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode ue_deregister fail\n");
        IUH_FREE_OBJECT(hnbap_error_indication);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeHNBAPErrorIndication more buf need to be decode \n");
        if(!IuhDecodeHnbDeregister(buf, size, cause)){
            IUH_FREE_OBJECT(hnbap_error_indication);
            return Iuh_FALSE;
        }
    }

    int errflag = 0;

    for(i = 0; i < hnbap_error_indication->protocolIEs.list.count; i++){
        switch(hnbap_error_indication->protocolIEs.list.array[i]->id){
            case HNBAP_id_Cause:{
                HNBAP_Cause_t *dereg_cause;
                dereg_cause = calloc(1,sizeof(HNBAP_Cause_t));
                initial_msg = &(hnbap_error_indication->protocolIEs.list.array[i]->value);
                ret = uper_decode_complete(&code_ctx, &asn_DEF_HNBAP_Cause, (void **)&dereg_cause, initial_msg->buf, initial_msg->size);
                if(ret.code == RC_FAIL){
                    iuh_syslog_err("decode cause fail\n");
                    errflag = 1;
                }
                else if(dereg_cause != NULL){
                    cause->present = dereg_cause->present;
                    switch(cause->present){
                        int my_cause = 0;
                        case HNBAP_Cause_PR_radioNetwork: {
                            asn_INTEGER2long(&(dereg_cause->choice.radioNetwork), (long*)&my_cause);
                            cause->choice.radioNetwork = my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_transport: {
                            asn_INTEGER2long(&(dereg_cause->choice.transport), (long*)&my_cause);
                            cause->choice.transport = my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_protocol: {
                            asn_INTEGER2long(&(dereg_cause->choice.protocol), (long*)&my_cause);
                            cause->choice.protocol= my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_misc: {
                            asn_INTEGER2long(&(dereg_cause->choice.misc), (long*)&my_cause);
                            cause->choice.misc= my_cause;
                            break;
                        }
                        default:
                            break;
                    }
                }
                IUH_FREE_OBJECT(dereg_cause);
            }
        }
    }

    return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          Decode rua error indication message
** INPUT:
**          
** OUTPUT:
**          
** RETURN:
**          
*****************************************************/

IuhBool IuhDecodeRUAErrorIndication(const char *buf, const int size, HNBAPCause *cause)
{
	/* Print Log Information */
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RUA  ERROR-INDICATION\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
    //iuh_syslog_debug_debug(IUH_DEFAULT, "IuhDecodeRUAErrorIndication\n");
    Rua_ErrorIndication_t *rua_error_indication;
    rua_error_indication = calloc(1,sizeof(Rua_ErrorIndication_t));
    asn_dec_rval_t ret;
    asn_codec_ctx_t code_ctx;
    code_ctx.max_stack_size = 0;
    ANY_t *initial_msg;
    int i;
    
    ret = uper_decode_complete(&code_ctx, &RUA_asn_DEF_ErrorIndication, (void *)&rua_error_indication, buf, size);
    iuh_syslog_debug_debug(IUH_DEFAULT,"decode ret = %d\n",ret.code);
    if(ret.code == RC_FAIL){
        iuh_syslog_err("decode rua_error_indication fail\n");
        IUH_FREE_OBJECT(rua_error_indication);
        return Iuh_FALSE;
    }
    else if(ret.code == RC_WMORE){
        iuh_syslog_debug_debug(IUH_DEFAULT,"IuhDecodeRUAErrorIndication more buf need to be decode \n");
        if(!IuhDecodeHnbDeregister(buf, size, cause)){
            IUH_FREE_OBJECT(rua_error_indication);
            return Iuh_FALSE;
        }
    }
    
    int errflag = 0;
    
    for(i = 0; i < rua_error_indication->protocolIEs.list.count; i++){
        switch(rua_error_indication->protocolIEs.list.array[i]->id){
            case HNBAP_id_Cause:{
                RUA_Cause_t *err_cause;
                err_cause = calloc(1,sizeof(RUA_Cause_t));
                initial_msg = &(rua_error_indication->protocolIEs.list.array[i]->value);
                ret = uper_decode_complete(&code_ctx, &RUA_asn_DEF_Cause, (void **)&err_cause, initial_msg->buf, initial_msg->size);
                if(ret.code == RC_FAIL){
                    iuh_syslog_err("decode rua cause fail\n");
                    errflag = 1;
                }
                else if(err_cause != NULL){
                    cause->present = err_cause->present;
                    switch(cause->present){
                        int my_cause = 0;
                        case HNBAP_Cause_PR_radioNetwork: {
                            asn_INTEGER2long(&(err_cause->choice.radioNetwork), (long*)&my_cause);
                            cause->choice.radioNetwork = my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_transport: {
                            asn_INTEGER2long(&(err_cause->choice.transport), (long*)&my_cause);
                            cause->choice.transport = my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_protocol: {
                            asn_INTEGER2long(&(err_cause->choice.protocol), (long*)&my_cause);
                            cause->choice.protocol= my_cause;
                            break;
                        }
                        case HNBAP_Cause_PR_misc: {
                            asn_INTEGER2long(&(err_cause->choice.misc), (long*)&my_cause);
                            cause->choice.misc= my_cause;
                            break;
                        }
                        default:
                            break;
                    }
                }
                IUH_FREE_OBJECT(err_cause);
            }
        }
    }
    return Iuh_TRUE;
}



/*****************************************************
** DISCRIPTION:
**			Decode rab relocation request message
** INPUT:
**			buf
**			buf length
** OUTPUT:
**			HNB CellId
** RETURN:
**			1		succeed
**			0		fail
*****************************************************/
IuhBool Iuh_decode_Relocation_Request(const char *buf, const int size, char cellId[CELLID_LEN], CNDomain *CnDomain)
{
	/* Print Log Information */
	iuh_syslog_info("###  ---------------------------------\n");
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RANAP	RELOCATION-REQUEST\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
	iuh_syslog_info("###  ---------------------------------\n");
	
	//iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_decode_Relocation_Request\n");
	RelocationRequest_t *relocation_request;
	relocation_request = calloc(1, sizeof(RelocationRequest_t));
	asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	ANY_t *initial_msg;
	int i;
	
	ret = uper_decode_complete(&code_ctx, &asn_DEF_RelocationRequest, (void **)&relocation_request, buf, size);
	iuh_syslog_debug_debug(IUH_DEFAULT, "decode ret = %d\n",ret.code);
	if(ret.code == RC_FAIL){
		iuh_syslog_err("decode relocation_request fail\n");
		IUH_FREE_OBJECT(relocation_request);
		return Iuh_FALSE;
	}
	else if(ret.code == RC_WMORE){
		iuh_syslog_debug_debug(IUH_DEFAULT, "sccp decode relocation request more buf need to be decode \n");
		if(!Iuh_decode_Relocation_Request(buf, size, cellId, CnDomain)){
			return Iuh_FALSE;
		}
	}
	
	int errflag = 0;
	for(i = 0; i < relocation_request->protocolIEs.list.count; i++){
		initial_msg = &(relocation_request->protocolIEs.list.array[i]->value);
		iuh_syslog_debug_debug(IUH_DEFAULT, "relocation_request IE type is %d \n", relocation_request->protocolIEs.list.array[i]->id);
		switch(relocation_request->protocolIEs.list.array[i]->id){
			case id_Source_ToTarget_TransparentContainer: {
			    // 1 sourceRNC_to_targetRNC_transparent_container
			    if(!IuhDecodeSourceRNCToTargetRNCTransparentContainer(initial_msg->buf, initial_msg->size, cellId, &errflag)){
					iuh_syslog_err("ERROR: decode Source RNC To Target RNC Trans Container error!\n");
			    }
				break;
			}
			case id_CN_DomainIndicator:{
			    // 2 cn_domain_indicator
                if(!IuhDecodeRANAPCnDomainIndicator(initial_msg->buf, initial_msg->size, CnDomain, &errflag)){
					iuh_syslog_err("ERROR: decode RANAP CnDomain Id error!\n");
                }
                break;
			}
			case id_Cause:{
				// 3 cause
				break;
			}
			case id_PermanentNAS_UE_ID:{
				// imsi in China & Thailand
				
				break;
			}
			default:
				break;
				 
		}
	}

	IUH_FREE_OBJECT(relocation_request);
	iuh_syslog_debug_debug(IUH_DEFAULT,"decode relocation request ok\n");
	
	return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**			Decode ranap initial ue message
** INPUT:
**			buf
**			buf length
** OUTPUT:
**			HNB CellId
** RETURN:
**			1		succeed
**			0		fail
*****************************************************/
IuhBool IuhDecodeRanapInitialUEMessage(const char *buf, const int size, const int UEID)
{
	if(IUH_HNB_UE[UEID] == NULL){
		iuh_syslog_err("Error: invalid ue id %d\n",UEID);
		return Iuh_TRUE;
	}
	
	/* Print Log Information */
	iuh_syslog_info("###  ---------------------------------\n");
	iuh_syslog_info("###  Action        :        Decode \n");
	iuh_syslog_info("###  Type          :        RANAP  INITIAL-UE-MESSAGE\n");
	iuh_syslog_info("###  Length        :        %d\n", size);
	iuh_syslog_info("###  ---------------------------------\n");
	
	InitialUE_Message_t *initial_ue_message;
	initial_ue_message = calloc(1, sizeof(InitialUE_Message_t));
	asn_dec_rval_t ret;
	asn_codec_ctx_t code_ctx;
	code_ctx.max_stack_size = 0;
	ANY_t *initial_msg;
	int i;
	CNDomain cnDomain;
	char iuSigConId[IU_SIG_CONN_ID_LEN] = {0};
	
	ret = uper_decode_complete(&code_ctx, &asn_DEF_InitialUE_Message, (void **)&initial_ue_message, buf, size);
	iuh_syslog_debug_debug(IUH_DEFAULT, "decode ret = %d\n",ret.code);
	if(ret.code == RC_FAIL){
		iuh_syslog_err("Error : decode initial_ue_message fail\n");
		IUH_FREE_OBJECT(initial_ue_message);
		return Iuh_FALSE;
	}
	else if(ret.code == RC_WMORE){
		iuh_syslog_debug_debug(IUH_DEFAULT, "sccp decode initial ue message more buf need to be decode \n");
		if(!IuhDecodeRanapInitialUEMessage(buf, size, UEID)){
			return Iuh_FALSE;
		}
	}
	
	int errflag = 0;
	for(i = 0; i < initial_ue_message->protocolIEs.list.count; i++){
		iuh_syslog_debug_debug(IUH_DEFAULT, "initial_ue_message IE type is %d \n", initial_ue_message->protocolIEs.list.array[i]->id);
		switch(initial_ue_message->protocolIEs.list.array[i]->id){
			case id_CN_DomainIndicator:{
			    // 1 cn_domain_indicator
                initial_msg = &(initial_ue_message->protocolIEs.list.array[i]->value);
                IuhDecodeRANAPCnDomainIndicator(initial_msg->buf, initial_msg->size, &cnDomain, &errflag);
                break;
			}
			case id_IuSigConId:{
				// 2 iu_sig_conn_id
				initial_msg = &(initial_ue_message->protocolIEs.list.array[i]->value);
				IuhDecodeRANAPIuSigConId(initial_msg->buf, initial_msg->size, iuSigConId);
				iuh_syslog_debug_debug(IUH_DEFAULT,"iuSigConId = %x%x%x\n",iuSigConId[0],iuSigConId[1],iuSigConId[2]);
				break;
			}
			default:
				break;
		}
	}
	if(cnDomain == CS_Domain)
		memcpy(IUH_HNB_UE[UEID]->iuCSSigConId, iuSigConId, IU_SIG_CONN_ID_LEN);
	else if(cnDomain == PS_Domain)
		memcpy(IUH_HNB_UE[UEID]->iuPSSigConId, iuSigConId, IU_SIG_CONN_ID_LEN);
	
	IUH_FREE_OBJECT(initial_ue_message);
	iuh_syslog_debug_debug(IUH_DEFAULT,"decode initial ue message ok\n");
	return Iuh_TRUE;
}

