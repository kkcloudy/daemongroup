#include <asn_internal.h>
#include <constr_SEQUENCE.h>
#include <per_opentype.h>
#include "ranapConstants.h"
#include "PDU_ARRAY.h"


#define RANAP_FULL_ENDECODE 0
#if RANAP_FULL_ENDECODE
/*
 * Check whether we are inside the extensions group.
 */
#define	IN_EXTENSION_GROUP(specs, memb_idx)	\
	( ((memb_idx) > (specs)->ext_after)	\
	&&((memb_idx) < (specs)->ext_before))
#endif

asn_TYPE_descriptor_t * initiatingValuePdu[MAX_PDU] = {NULL};
asn_TYPE_descriptor_t * successfulValuePdu[MAX_PDU] = {NULL};
asn_TYPE_descriptor_t * unsuccessfulValuePdu[MAX_PDU] = {NULL};
asn_TYPE_descriptor_t * outcomeValuePdu[MAX_PDU] = {NULL};
asn_TYPE_descriptor_t * ranapIesValuePdu[MAX_PDU] = {NULL};

asn_TYPE_descriptor_t ** ranapANYValuePdu[PARENT_PDU_NONE] = {NULL};

extern asn_TYPE_descriptor_t asn_DEF_Iu_ReleaseCommand;
extern asn_TYPE_descriptor_t asn_DEF_Iu_ReleaseComplete;
extern asn_TYPE_descriptor_t asn_DEF_RelocationRequired;
extern asn_TYPE_descriptor_t asn_DEF_RelocationCommand;
extern asn_TYPE_descriptor_t asn_DEF_RelocationPreparationFailure;
extern asn_TYPE_descriptor_t asn_DEF_RelocationRequest;
extern asn_TYPE_descriptor_t asn_DEF_RelocationRequestAcknowledge;
extern asn_TYPE_descriptor_t asn_DEF_RelocationFailure;
extern asn_TYPE_descriptor_t asn_DEF_RelocationCancel;
extern asn_TYPE_descriptor_t asn_DEF_RelocationCancelAcknowledge;
extern asn_TYPE_descriptor_t asn_DEF_SRNS_ContextRequest;
extern asn_TYPE_descriptor_t asn_DEF_SRNS_ContextResponse;
extern asn_TYPE_descriptor_t asn_DEF_SecurityModeCommand;
extern asn_TYPE_descriptor_t asn_DEF_SecurityModeComplete;
extern asn_TYPE_descriptor_t asn_DEF_SecurityModeReject;
extern asn_TYPE_descriptor_t asn_DEF_DataVolumeReportRequest;
extern asn_TYPE_descriptor_t asn_DEF_DataVolumeReport;
extern asn_TYPE_descriptor_t asn_DEF_Reset;
extern asn_TYPE_descriptor_t asn_DEF_ResetAcknowledge;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleaseRequest;
extern asn_TYPE_descriptor_t asn_DEF_Iu_ReleaseRequest;
extern asn_TYPE_descriptor_t asn_DEF_RelocationDetect;
extern asn_TYPE_descriptor_t asn_DEF_RelocationComplete;
extern asn_TYPE_descriptor_t asn_DEF_Paging;
extern asn_TYPE_descriptor_t asn_DEF_CommonID;
extern asn_TYPE_descriptor_t asn_DEF_CN_InvokeTrace;
extern asn_TYPE_descriptor_t asn_DEF_CN_DeactivateTrace;
extern asn_TYPE_descriptor_t asn_DEF_LocationReportingControl;
extern asn_TYPE_descriptor_t asn_DEF_LocationReport;
extern asn_TYPE_descriptor_t asn_DEF_InitialUE_Message;
extern asn_TYPE_descriptor_t asn_DEF_DirectTransfer;
extern asn_TYPE_descriptor_t asn_DEF_Overload;
extern asn_TYPE_descriptor_t RANAP_asn_DEF_ErrorIndication;
extern asn_TYPE_descriptor_t asn_DEF_SRNS_DataForwardCommand;
extern asn_TYPE_descriptor_t asn_DEF_ForwardSRNS_Context;
extern asn_TYPE_descriptor_t asn_DEF_RAB_AssignmentRequest;
extern asn_TYPE_descriptor_t asn_DEF_RAB_AssignmentResponse;
extern asn_TYPE_descriptor_t asn_DEF_PrivateMessage;
extern asn_TYPE_descriptor_t asn_DEF_ResetResource;
extern asn_TYPE_descriptor_t asn_DEF_ResetResourceAcknowledge;
extern asn_TYPE_descriptor_t asn_DEF_RANAP_RelocationInformation;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ModifyRequest;
extern asn_TYPE_descriptor_t asn_DEF_LocationRelatedDataRequest;
extern asn_TYPE_descriptor_t asn_DEF_LocationRelatedDataResponse;
extern asn_TYPE_descriptor_t asn_DEF_LocationRelatedDataFailure;
extern asn_TYPE_descriptor_t asn_DEF_InformationTransferIndication;
extern asn_TYPE_descriptor_t asn_DEF_InformationTransferConfirmation;
extern asn_TYPE_descriptor_t asn_DEF_InformationTransferFailure;
extern asn_TYPE_descriptor_t asn_DEF_UESpecificInformationIndication;
extern asn_TYPE_descriptor_t asn_DEF_DirectInformationTransfer;
extern asn_TYPE_descriptor_t asn_DEF_UplinkInformationExchangeRequest;
extern asn_TYPE_descriptor_t asn_DEF_UplinkInformationExchangeResponse;
extern asn_TYPE_descriptor_t asn_DEF_UplinkInformationExchangeFailure;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionStart;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionStartResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionStartFailure;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionUpdate;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionUpdateResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionUpdateFailure;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionStop;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionStopResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSUELinkingRequest;
extern asn_TYPE_descriptor_t asn_DEF_MBMSUELinkingResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRegistrationRequest;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRegistrationResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRegistrationFailure;
extern asn_TYPE_descriptor_t asn_DEF_MBMSCNDe_RegistrationRequest;
extern asn_TYPE_descriptor_t asn_DEF_MBMSCNDe_RegistrationResponse;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRABEstablishmentIndication;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRABReleaseRequest;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRABRelease;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRABReleaseFailure;
extern asn_TYPE_descriptor_t asn_DEF_EnhancedRelocationCompleteRequest;
extern asn_TYPE_descriptor_t asn_DEF_EnhancedRelocationCompleteResponse;
extern asn_TYPE_descriptor_t asn_DEF_EnhancedRelocationCompleteFailure;
extern asn_TYPE_descriptor_t asn_DEF_EnhancedRelocationCompleteConfirm;
extern asn_TYPE_descriptor_t asn_DEF_RANAP_EnhancedRelocationInformationRequest;
extern asn_TYPE_descriptor_t asn_DEF_RANAP_EnhancedRelocationInformationResponse;
extern asn_TYPE_descriptor_t asn_DEF_SRVCC_CSKeysRequest;
extern asn_TYPE_descriptor_t asn_DEF_SRVCC_CSKeysResponse;

void initiatingValuesInit(){
	initiatingValuePdu[id_Iu_Release] = &asn_DEF_Iu_ReleaseCommand;
	initiatingValuePdu[id_RelocationPreparation] = &asn_DEF_RelocationRequired;
	initiatingValuePdu[id_RelocationResourceAllocation] = &asn_DEF_RelocationRequest;
	initiatingValuePdu[id_RelocationCancel] = &asn_DEF_RelocationCancel;
	initiatingValuePdu[id_SRNS_ContextTransfer] = &asn_DEF_SRNS_ContextRequest;
	initiatingValuePdu[id_SecurityModeControl] = &asn_DEF_SecurityModeCommand;
	initiatingValuePdu[id_DataVolumeReport] = &asn_DEF_DataVolumeReportRequest;
	initiatingValuePdu[id_Reset] = &asn_DEF_Reset;
	initiatingValuePdu[id_RAB_ReleaseRequest] = &asn_DEF_RAB_ReleaseRequest;
	initiatingValuePdu[id_Iu_ReleaseRequest] = &asn_DEF_Iu_ReleaseRequest;
	initiatingValuePdu[id_RelocationDetect] = &asn_DEF_RelocationDetect;
	initiatingValuePdu[id_RelocationComplete] = &asn_DEF_RelocationComplete;
	initiatingValuePdu[id_Paging] = &asn_DEF_Paging;
	initiatingValuePdu[id_CommonID] = &asn_DEF_CommonID;
	initiatingValuePdu[id_CN_InvokeTrace] = &asn_DEF_CN_InvokeTrace;
	initiatingValuePdu[id_CN_DeactivateTrace] = &asn_DEF_CN_DeactivateTrace;
	initiatingValuePdu[id_LocationReportingControl] = &asn_DEF_LocationReportingControl;
	initiatingValuePdu[id_LocationReport] = &asn_DEF_LocationReport;
	initiatingValuePdu[id_InitialUE_Message] = &asn_DEF_InitialUE_Message;
	initiatingValuePdu[id_DirectTransfer] = &asn_DEF_DirectTransfer;
	initiatingValuePdu[id_OverloadControl] = &asn_DEF_Overload;
	initiatingValuePdu[id_ErrorIndication] = &RANAP_asn_DEF_ErrorIndication;
	initiatingValuePdu[id_SRNS_DataForward] = &asn_DEF_SRNS_DataForwardCommand;
	initiatingValuePdu[id_ForwardSRNS_Context] = &asn_DEF_ForwardSRNS_Context;
	initiatingValuePdu[id_RAB_Assignment] = &asn_DEF_RAB_AssignmentRequest;
	initiatingValuePdu[id_privateMessage] = &asn_DEF_PrivateMessage;
	initiatingValuePdu[id_ResetResource] = &asn_DEF_ResetResource;
	initiatingValuePdu[id_RANAP_Relocation] = &asn_DEF_RANAP_RelocationInformation;
	initiatingValuePdu[id_RAB_ModifyRequest] = &asn_DEF_RAB_ModifyRequest;
	initiatingValuePdu[id_LocationRelatedData] = &asn_DEF_LocationRelatedDataRequest;
	initiatingValuePdu[id_InformationTransfer] = &asn_DEF_InformationTransferIndication;
	initiatingValuePdu[id_UESpecificInformation] = &asn_DEF_UESpecificInformationIndication;
	initiatingValuePdu[id_DirectInformationTransfer] = &asn_DEF_DirectInformationTransfer;
	initiatingValuePdu[id_UplinkInformationExchange] = &asn_DEF_UplinkInformationExchangeRequest;
	initiatingValuePdu[id_MBMSSessionStart] = &asn_DEF_MBMSSessionStart;
	initiatingValuePdu[id_MBMSSessionUpdate] = &asn_DEF_MBMSSessionUpdate;
	initiatingValuePdu[id_MBMSSessionStop] = &asn_DEF_MBMSSessionStop;
	initiatingValuePdu[id_MBMSUELinking] = &asn_DEF_MBMSUELinkingRequest;
	initiatingValuePdu[id_MBMSRegistration] = &asn_DEF_MBMSRegistrationRequest;
	initiatingValuePdu[id_MBMSCNDe_Registration_Procedure] = &asn_DEF_MBMSCNDe_RegistrationRequest;
	initiatingValuePdu[id_MBMSRABEstablishmentIndication] = &asn_DEF_MBMSRABEstablishmentIndication;
	initiatingValuePdu[id_MBMSRABRelease] = &asn_DEF_MBMSRABReleaseRequest;
	initiatingValuePdu[id_enhancedRelocationComplete] = &asn_DEF_EnhancedRelocationCompleteRequest;
	initiatingValuePdu[id_enhancedRelocationCompleteConfirm] = &asn_DEF_EnhancedRelocationCompleteConfirm;
	initiatingValuePdu[id_RANAPenhancedRelocation] = &asn_DEF_RANAP_EnhancedRelocationInformationRequest;
	initiatingValuePdu[id_SRVCCPreparation] = &asn_DEF_SRVCC_CSKeysRequest;

}

void successfulValuesInit(){
	successfulValuePdu[id_Iu_Release] = &asn_DEF_Iu_ReleaseComplete;
	successfulValuePdu[id_RelocationPreparation] = &asn_DEF_RelocationCommand;
	successfulValuePdu[id_RelocationResourceAllocation] = &asn_DEF_RelocationRequestAcknowledge;
	successfulValuePdu[id_RelocationCancel] = &asn_DEF_RelocationCancelAcknowledge;
	successfulValuePdu[id_SRNS_ContextTransfer] = &asn_DEF_SRNS_ContextResponse;
	successfulValuePdu[id_SecurityModeControl] = &asn_DEF_SecurityModeComplete;
	successfulValuePdu[id_DataVolumeReport] = &asn_DEF_DataVolumeReport;
	successfulValuePdu[id_Reset] = &asn_DEF_ResetAcknowledge;
	successfulValuePdu[id_ResetResource] = &asn_DEF_ResetResourceAcknowledge;
	successfulValuePdu[id_LocationRelatedData] = &asn_DEF_LocationRelatedDataResponse;
	successfulValuePdu[id_InformationTransfer] = &asn_DEF_InformationTransferConfirmation;
	successfulValuePdu[id_UplinkInformationExchange] = &asn_DEF_UplinkInformationExchangeResponse;
	successfulValuePdu[id_MBMSSessionStart] = &asn_DEF_MBMSSessionStartResponse;
	successfulValuePdu[id_MBMSSessionUpdate] = &asn_DEF_MBMSSessionUpdateResponse;
	successfulValuePdu[id_MBMSSessionStop] = &asn_DEF_MBMSSessionStopResponse;
	successfulValuePdu[id_MBMSRegistration] = &asn_DEF_MBMSRegistrationResponse;
	successfulValuePdu[id_MBMSCNDe_Registration_Procedure] = &asn_DEF_MBMSCNDe_RegistrationResponse;
	successfulValuePdu[id_MBMSRABRelease] = &asn_DEF_MBMSRABRelease;
	successfulValuePdu[id_enhancedRelocationComplete] = &asn_DEF_EnhancedRelocationCompleteResponse;
	successfulValuePdu[id_RANAPenhancedRelocation] = &asn_DEF_RANAP_EnhancedRelocationInformationResponse;

}

void unsuccessfulValuesInit(){
	unsuccessfulValuePdu[id_RelocationPreparation] = &asn_DEF_RelocationPreparationFailure;
	unsuccessfulValuePdu[id_RelocationResourceAllocation] = &asn_DEF_RelocationFailure;
	unsuccessfulValuePdu[id_SecurityModeControl] = &asn_DEF_SecurityModeReject;
	unsuccessfulValuePdu[id_LocationRelatedData] = &asn_DEF_LocationRelatedDataFailure;
	unsuccessfulValuePdu[id_InformationTransfer] = &asn_DEF_InformationTransferFailure;
	unsuccessfulValuePdu[id_UplinkInformationExchange] = &asn_DEF_UplinkInformationExchangeFailure;
	unsuccessfulValuePdu[id_MBMSSessionStart] = &asn_DEF_MBMSSessionStartFailure;
	unsuccessfulValuePdu[id_MBMSSessionUpdate] = &asn_DEF_MBMSSessionUpdateFailure;
	unsuccessfulValuePdu[id_MBMSRegistration] = &asn_DEF_MBMSRegistrationFailure;
	unsuccessfulValuePdu[id_MBMSRABRelease] = &asn_DEF_MBMSRABReleaseFailure;
	unsuccessfulValuePdu[id_enhancedRelocationComplete] = &asn_DEF_EnhancedRelocationCompleteFailure;

}

void outcomeValuesInit(){
	outcomeValuePdu[id_RAB_Assignment] = &asn_DEF_RAB_AssignmentResponse;
	outcomeValuePdu[id_MBMSUELinking] = &asn_DEF_MBMSUELinkingResponse;
	outcomeValuePdu[id_SRVCCPreparation] = &asn_DEF_SRVCC_CSKeysResponse;

}

extern asn_TYPE_descriptor_t RANAP_asn_DEF_Cause;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataVolumeReportList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleasedList_IuRelComp;
extern asn_TYPE_descriptor_t asn_DEF_CriticalityDiagnostics;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataVolumeReportItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleasedItem_IuRelComp;
extern asn_TYPE_descriptor_t asn_DEF_RelocationType;
extern asn_TYPE_descriptor_t asn_DEF_SourceID;
extern asn_TYPE_descriptor_t asn_DEF_TargetID;
extern asn_TYPE_descriptor_t asn_DEF_ClassmarkInformation2;
extern asn_TYPE_descriptor_t asn_DEF_ClassmarkInformation3;
extern asn_TYPE_descriptor_t asn_DEF_Source_ToTarget_TransparentContainer;
extern asn_TYPE_descriptor_t asn_DEF_OldBSS_ToNewBSS_Information;
extern asn_TYPE_descriptor_t asn_DEF_Target_ToSource_TransparentContainer;
extern asn_TYPE_descriptor_t asn_DEF_L3_Information;
extern asn_TYPE_descriptor_t asn_DEF_RAB_RelocationReleaseList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataForwardingList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_RelocationReleaseItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataForwardingItem;
extern asn_TYPE_descriptor_t asn_DEF_PermanentNAS_UE_ID;
extern asn_TYPE_descriptor_t asn_DEF_RANAP_CN_DomainIndicator;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_RelocReq;
extern asn_TYPE_descriptor_t asn_DEF_IntegrityProtectionInformation;
extern asn_TYPE_descriptor_t asn_DEF_EncryptionInformation;
extern asn_TYPE_descriptor_t asn_DEF_IuSignallingConnectionIdentifier;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_RelocReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_RelocReqAck;
extern asn_TYPE_descriptor_t asn_DEF_RAB_FailedList;
extern asn_TYPE_descriptor_t asn_DEF_ChosenIntegrityProtectionAlgorithm;
extern asn_TYPE_descriptor_t asn_DEF_ChosenEncryptionAlgorithm;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_RelocReqAck;
extern asn_TYPE_descriptor_t asn_DEF_RAB_FailedItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataForwardingList_SRNS_CtxReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataForwardingItem_SRNS_CtxReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ContextList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ContextFailedtoTransferList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ContextItem;
extern asn_TYPE_descriptor_t asn_DEF_RABs_ContextFailedtoTransferItem;
extern asn_TYPE_descriptor_t asn_DEF_KeyStatus;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataVolumeReportRequestList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_DataVolumeReportRequestItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_FailedtoReportList;
extern asn_TYPE_descriptor_t asn_DEF_RABs_failed_to_reportItem;
extern asn_TYPE_descriptor_t asn_DEF_GlobalRNC_ID;
extern asn_TYPE_descriptor_t asn_DEF_ResetResourceList;
extern asn_TYPE_descriptor_t asn_DEF_ResetResourceItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleaseList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleaseItem;
extern asn_TYPE_descriptor_t asn_DEF_IuSignallingConnectionIdentifier;
extern asn_TYPE_descriptor_t asn_DEF_GlobalRNC_ID;
extern asn_TYPE_descriptor_t asn_DEF_ExtendedRNC_ID;
extern asn_TYPE_descriptor_t asn_DEF_GlobalRNC_ID;
extern asn_TYPE_descriptor_t asn_DEF_ExtendedRNC_ID;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_EnhancedRelocCompleteReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_EnhancedRelocCompleteReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_EnhancedRelocCompleteRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ToBeReleasedList_EnhancedRelocCompleteRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_EnhancedRelocCompleteRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ToBeReleasedItem_EnhancedRelocCompleteRes;
extern asn_TYPE_descriptor_t asn_DEF_TemporaryUE_ID;
extern asn_TYPE_descriptor_t asn_DEF_PagingAreaID;
extern asn_TYPE_descriptor_t asn_DEF_PagingCause;
extern asn_TYPE_descriptor_t asn_DEF_NonSearchingIndication;
extern asn_TYPE_descriptor_t asn_DEF_DRX_CycleLengthCoefficient;
extern asn_TYPE_descriptor_t asn_DEF_TraceType;
extern asn_TYPE_descriptor_t asn_DEF_TraceReference;
extern asn_TYPE_descriptor_t asn_DEF_TriggerID;
extern asn_TYPE_descriptor_t asn_DEF_UE_ID;
extern asn_TYPE_descriptor_t asn_DEF_OMC_ID;
extern asn_TYPE_descriptor_t asn_DEF_RequestType;
extern asn_TYPE_descriptor_t asn_DEF_AreaIdentity;
extern asn_TYPE_descriptor_t asn_DEF_LAI;
extern asn_TYPE_descriptor_t asn_DEF_RAC;
extern asn_TYPE_descriptor_t asn_DEF_SAI;
extern asn_TYPE_descriptor_t asn_DEF_NAS_PDU;
extern asn_TYPE_descriptor_t asn_DEF_SAPI;
extern asn_TYPE_descriptor_t asn_DEF_RejectCauseValue;
extern asn_TYPE_descriptor_t asn_DEF_NAS_SequenceNumber;
extern asn_TYPE_descriptor_t asn_DEF_NumberOfSteps;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupOrModifyList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupOrModifiedList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleasedList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_QueuedList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleaseFailedList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupOrModifiedItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ReleasedItem;
extern asn_TYPE_descriptor_t asn_DEF_RAB_QueuedItem;
extern asn_TYPE_descriptor_t asn_DEF_GERAN_Iumode_RAB_Failed_RABAssgntResponse_Item;
extern asn_TYPE_descriptor_t asn_DEF_DirectTransferInformationList_RANAP_RelocInf;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ContextList_RANAP_RelocInf;
extern asn_TYPE_descriptor_t asn_DEF_DirectTransferInformationItem_RANAP_RelocInf;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ContextItem_RANAP_RelocInf;
extern asn_TYPE_descriptor_t asn_DEF_IuSignallingConnectionIdentifier;
extern asn_TYPE_descriptor_t asn_DEF_GlobalCN_ID;
extern asn_TYPE_descriptor_t asn_DEF_IuSignallingConnectionIdentifier;
extern asn_TYPE_descriptor_t asn_DEF_GlobalCN_ID;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_EnhRelocInfoReq;
extern asn_TYPE_descriptor_t asn_DEF_SNA_Access_Information;
extern asn_TYPE_descriptor_t asn_DEF_UESBI_Iu;
extern asn_TYPE_descriptor_t asn_DEF_PLMNidentity;
extern asn_TYPE_descriptor_t asn_DEF_CNMBMSLinkingInformation;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_EnhRelocInfoReq;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupList_EnhRelocInfoRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_FailedList_EnhRelocInfoRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_SetupItem_EnhRelocInfoRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_FailedItem_EnhRelocInfoRes;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ModifyList;
extern asn_TYPE_descriptor_t asn_DEF_RAB_ModifyItem;
extern asn_TYPE_descriptor_t asn_DEF_LocationRelatedDataRequestType;
extern asn_TYPE_descriptor_t asn_DEF_BroadcastAssistanceDataDecipheringKeys;
extern asn_TYPE_descriptor_t asn_DEF_InformationTransferID;
extern asn_TYPE_descriptor_t asn_DEF_ProvidedData;
extern asn_TYPE_descriptor_t asn_DEF_GlobalCN_ID;
extern asn_TYPE_descriptor_t asn_DEF_InterSystemInformationTransferType;
extern asn_TYPE_descriptor_t asn_DEF_InformationExchangeID;
extern asn_TYPE_descriptor_t asn_DEF_InformationExchangeType;
extern asn_TYPE_descriptor_t asn_DEF_InformationTransferType;
extern asn_TYPE_descriptor_t asn_DEF_InformationRequestType;
extern asn_TYPE_descriptor_t asn_DEF_InformationRequested;
extern asn_TYPE_descriptor_t asn_DEF_TMGI;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionIdentity;
extern asn_TYPE_descriptor_t asn_DEF_MBMSBearerServiceType;
extern asn_TYPE_descriptor_t asn_DEF_RAB_Parameters;
extern asn_TYPE_descriptor_t asn_DEF_PDP_TypeInformation;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionDuration;
extern asn_TYPE_descriptor_t asn_DEF_MBMSServiceArea;
extern asn_TYPE_descriptor_t asn_DEF_FrequenceLayerConvergenceFlag;
extern asn_TYPE_descriptor_t asn_DEF_RAListofIdleModeUEs;
extern asn_TYPE_descriptor_t asn_DEF_MBMSSessionRepetitionNumber;
extern asn_TYPE_descriptor_t asn_DEF_TimeToMBMSDataTransfer;
extern asn_TYPE_descriptor_t asn_DEF_TransportLayerInformation;
extern asn_TYPE_descriptor_t asn_DEF_SessionUpdateID;
extern asn_TYPE_descriptor_t asn_DEF_DeltaRAListofIdleModeUEs;
extern asn_TYPE_descriptor_t asn_DEF_MBMSCNDe_Registration;
extern asn_TYPE_descriptor_t asn_DEF_JoinedMBMSBearerService_IEs;
extern asn_TYPE_descriptor_t asn_DEF_LeftMBMSBearerService_IEs;
extern asn_TYPE_descriptor_t asn_DEF_UnsuccessfulLinking_IEs;
extern asn_TYPE_descriptor_t asn_DEF_MBMSRegistrationRequestType;
extern asn_TYPE_descriptor_t asn_DEF_IPMulticastAddress;
extern asn_TYPE_descriptor_t asn_DEF_APN;
extern asn_TYPE_descriptor_t asn_DEF_IntegrityProtectionKey;
extern asn_TYPE_descriptor_t asn_DEF_EncryptionKey;
extern asn_TYPE_descriptor_t asn_DEF_SRVCC_Information;

void ieValueTypeInit(){
	ranapIesValuePdu[id_Cause] = &RANAP_asn_DEF_Cause;
	ranapIesValuePdu[id_RAB_DataVolumeReportList] = &asn_DEF_RAB_DataVolumeReportList;
	ranapIesValuePdu[id_RAB_ReleasedList_IuRelComp] = &asn_DEF_RAB_ReleasedList_IuRelComp;
	ranapIesValuePdu[id_CriticalityDiagnostics] = &asn_DEF_CriticalityDiagnostics;
	ranapIesValuePdu[id_RAB_DataVolumeReportItem] = &asn_DEF_RAB_DataVolumeReportItem;
	ranapIesValuePdu[id_RAB_ReleasedItem_IuRelComp] = &asn_DEF_RAB_ReleasedItem_IuRelComp;
	ranapIesValuePdu[id_RelocationType] = &asn_DEF_RelocationType;
	ranapIesValuePdu[id_SourceID] = &asn_DEF_SourceID;
	ranapIesValuePdu[id_TargetID] = &asn_DEF_TargetID;
	ranapIesValuePdu[id_ClassmarkInformation2] = &asn_DEF_ClassmarkInformation2;
	ranapIesValuePdu[id_ClassmarkInformation3] = &asn_DEF_ClassmarkInformation3;
	ranapIesValuePdu[id_Source_ToTarget_TransparentContainer] = &asn_DEF_Source_ToTarget_TransparentContainer;
	ranapIesValuePdu[id_OldBSS_ToNewBSS_Information] = &asn_DEF_OldBSS_ToNewBSS_Information;
	ranapIesValuePdu[id_Target_ToSource_TransparentContainer] = &asn_DEF_Target_ToSource_TransparentContainer;
	ranapIesValuePdu[id_L3_Information] = &asn_DEF_L3_Information;
	ranapIesValuePdu[id_RAB_RelocationReleaseList] = &asn_DEF_RAB_RelocationReleaseList;
	ranapIesValuePdu[id_RAB_DataForwardingList] = &asn_DEF_RAB_DataForwardingList;
	ranapIesValuePdu[id_RAB_RelocationReleaseItem] = &asn_DEF_RAB_RelocationReleaseItem;
	ranapIesValuePdu[id_RAB_DataForwardingItem] = &asn_DEF_RAB_DataForwardingItem;
	ranapIesValuePdu[id_PermanentNAS_UE_ID] = &asn_DEF_PermanentNAS_UE_ID;
	ranapIesValuePdu[id_CN_DomainIndicator] = &asn_DEF_RANAP_CN_DomainIndicator;
	ranapIesValuePdu[id_RAB_SetupList_RelocReq] = &asn_DEF_RAB_SetupList_RelocReq;
	ranapIesValuePdu[id_IntegrityProtectionInformation] = &asn_DEF_IntegrityProtectionInformation;
	ranapIesValuePdu[id_EncryptionInformation] = &asn_DEF_EncryptionInformation;
	ranapIesValuePdu[id_IuSigConId] = &asn_DEF_IuSignallingConnectionIdentifier;
	ranapIesValuePdu[id_RAB_SetupItem_RelocReq] = &asn_DEF_RAB_SetupItem_RelocReq;
	ranapIesValuePdu[id_RAB_SetupList_RelocReqAck] = &asn_DEF_RAB_SetupList_RelocReqAck;
	ranapIesValuePdu[id_RAB_FailedList] = &asn_DEF_RAB_FailedList;
	ranapIesValuePdu[id_ChosenIntegrityProtectionAlgorithm] = &asn_DEF_ChosenIntegrityProtectionAlgorithm;
	ranapIesValuePdu[id_ChosenEncryptionAlgorithm] = &asn_DEF_ChosenEncryptionAlgorithm;
	ranapIesValuePdu[id_RAB_SetupItem_RelocReqAck] = &asn_DEF_RAB_SetupItem_RelocReqAck;
	ranapIesValuePdu[id_RAB_FailedItem] = &asn_DEF_RAB_FailedItem;
	ranapIesValuePdu[id_RAB_DataForwardingList_SRNS_CtxReq] = &asn_DEF_RAB_DataForwardingList_SRNS_CtxReq;
	ranapIesValuePdu[id_RAB_DataForwardingItem_SRNS_CtxReq] = &asn_DEF_RAB_DataForwardingItem_SRNS_CtxReq;
	ranapIesValuePdu[id_RAB_ContextList] = &asn_DEF_RAB_ContextList;
	ranapIesValuePdu[id_RAB_ContextFailedtoTransferList] = &asn_DEF_RAB_ContextFailedtoTransferList;
	ranapIesValuePdu[id_RAB_ContextItem] = &asn_DEF_RAB_ContextItem;
	ranapIesValuePdu[id_RAB_ContextFailedtoTransferItem] = &asn_DEF_RABs_ContextFailedtoTransferItem;
	ranapIesValuePdu[id_KeyStatus] = &asn_DEF_KeyStatus;
	ranapIesValuePdu[id_RAB_DataVolumeReportRequestList] = &asn_DEF_RAB_DataVolumeReportRequestList;
	ranapIesValuePdu[id_RAB_DataVolumeReportRequestItem] = &asn_DEF_RAB_DataVolumeReportRequestItem;
	ranapIesValuePdu[id_RAB_FailedtoReportList] = &asn_DEF_RAB_FailedtoReportList;
	ranapIesValuePdu[id_RAB_FailedtoReportItem] = &asn_DEF_RABs_failed_to_reportItem;
	ranapIesValuePdu[id_GlobalRNC_ID] = &asn_DEF_GlobalRNC_ID;
	ranapIesValuePdu[id_IuSigConIdList] = &asn_DEF_ResetResourceList;
	ranapIesValuePdu[id_IuSigConIdItem] = &asn_DEF_ResetResourceItem;
	ranapIesValuePdu[id_RAB_ReleaseList] = &asn_DEF_RAB_ReleaseList;
	ranapIesValuePdu[id_RAB_ReleaseItem] = &asn_DEF_RAB_ReleaseItem;
	ranapIesValuePdu[id_OldIuSigConId] = &asn_DEF_IuSignallingConnectionIdentifier;
	ranapIesValuePdu[id_Relocation_SourceRNC_ID] = &asn_DEF_GlobalRNC_ID;
	ranapIesValuePdu[id_Relocation_SourceExtendedRNC_ID] = &asn_DEF_ExtendedRNC_ID;
	ranapIesValuePdu[id_Relocation_TargetRNC_ID] = &asn_DEF_GlobalRNC_ID;
	ranapIesValuePdu[id_Relocation_TargetExtendedRNC_ID] = &asn_DEF_ExtendedRNC_ID;
	ranapIesValuePdu[id_RAB_SetupList_EnhancedRelocCompleteReq] = &asn_DEF_RAB_SetupList_EnhancedRelocCompleteReq;
	ranapIesValuePdu[id_RAB_SetupItem_EnhancedRelocCompleteReq] = &asn_DEF_RAB_SetupItem_EnhancedRelocCompleteReq;
	ranapIesValuePdu[id_RAB_SetupList_EnhancedRelocCompleteRes] = &asn_DEF_RAB_SetupList_EnhancedRelocCompleteRes;
	ranapIesValuePdu[id_RAB_ToBeReleasedList_EnhancedRelocCompleteRes] = &asn_DEF_RAB_ToBeReleasedList_EnhancedRelocCompleteRes;
	ranapIesValuePdu[id_RAB_SetupItem_EnhancedRelocCompleteRes] = &asn_DEF_RAB_SetupItem_EnhancedRelocCompleteRes;
	ranapIesValuePdu[id_RAB_ToBeReleasedItem_EnhancedRelocCompleteRes] = &asn_DEF_RAB_ToBeReleasedItem_EnhancedRelocCompleteRes;
	ranapIesValuePdu[id_TemporaryUE_ID] = &asn_DEF_TemporaryUE_ID;
	ranapIesValuePdu[id_PagingAreaID] = &asn_DEF_PagingAreaID;
	ranapIesValuePdu[id_PagingCause] = &asn_DEF_PagingCause;
	ranapIesValuePdu[id_NonSearchingIndication] = &asn_DEF_NonSearchingIndication;
	ranapIesValuePdu[id_DRX_CycleLengthCoefficient] = &asn_DEF_DRX_CycleLengthCoefficient;
	ranapIesValuePdu[id_TraceType] = &asn_DEF_TraceType;
	ranapIesValuePdu[id_TraceReference] = &asn_DEF_TraceReference;
	ranapIesValuePdu[id_TriggerID] = &asn_DEF_TriggerID;
	ranapIesValuePdu[id_UE_ID] = &asn_DEF_UE_ID;
	ranapIesValuePdu[id_OMC_ID] = &asn_DEF_OMC_ID;
	ranapIesValuePdu[id_RequestType] = &asn_DEF_RequestType;
	ranapIesValuePdu[id_AreaIdentity] = &asn_DEF_AreaIdentity;
	ranapIesValuePdu[id_LAI] = &asn_DEF_LAI;
	ranapIesValuePdu[id_RAC] = &asn_DEF_RAC;
	ranapIesValuePdu[id_SAI] = &asn_DEF_SAI;
	ranapIesValuePdu[id_NAS_PDU] = &asn_DEF_NAS_PDU;
	ranapIesValuePdu[id_SAPI] = &asn_DEF_SAPI;
	ranapIesValuePdu[id_RejectCauseValue] = &asn_DEF_RejectCauseValue;
	ranapIesValuePdu[id_NAS_SequenceNumber] = &asn_DEF_NAS_SequenceNumber;
	ranapIesValuePdu[id_NumberOfSteps] = &asn_DEF_NumberOfSteps;
	ranapIesValuePdu[id_RAB_SetupOrModifyList] = &asn_DEF_RAB_SetupOrModifyList;
	ranapIesValuePdu[id_RAB_SetupOrModifiedList] = &asn_DEF_RAB_SetupOrModifiedList;
	ranapIesValuePdu[id_RAB_ReleasedList] = &asn_DEF_RAB_ReleasedList;
	ranapIesValuePdu[id_RAB_QueuedList] = &asn_DEF_RAB_QueuedList;
	ranapIesValuePdu[id_RAB_ReleaseFailedList] = &asn_DEF_RAB_ReleaseFailedList;
	ranapIesValuePdu[id_RAB_SetupOrModifiedItem] = &asn_DEF_RAB_SetupOrModifiedItem;
	ranapIesValuePdu[id_RAB_ReleasedItem] = &asn_DEF_RAB_ReleasedItem;
	ranapIesValuePdu[id_RAB_QueuedItem] = &asn_DEF_RAB_QueuedItem;
	ranapIesValuePdu[id_GERAN_Iumode_RAB_Failed_RABAssgntResponse_Item] = &asn_DEF_GERAN_Iumode_RAB_Failed_RABAssgntResponse_Item;
	ranapIesValuePdu[id_DirectTransferInformationList_RANAP_RelocInf] = &asn_DEF_DirectTransferInformationList_RANAP_RelocInf;
	ranapIesValuePdu[id_RAB_ContextList_RANAP_RelocInf] = &asn_DEF_RAB_ContextList_RANAP_RelocInf;
	ranapIesValuePdu[id_DirectTransferInformationItem_RANAP_RelocInf] = &asn_DEF_DirectTransferInformationItem_RANAP_RelocInf;
	ranapIesValuePdu[id_RAB_ContextItem_RANAP_RelocInf] = &asn_DEF_RAB_ContextItem_RANAP_RelocInf;
	ranapIesValuePdu[id_OldIuSigConIdCS] = &asn_DEF_IuSignallingConnectionIdentifier;
	ranapIesValuePdu[id_GlobalCN_IDCS] = &asn_DEF_GlobalCN_ID;
	ranapIesValuePdu[id_OldIuSigConIdPS] = &asn_DEF_IuSignallingConnectionIdentifier;
	ranapIesValuePdu[id_GlobalCN_IDPS] = &asn_DEF_GlobalCN_ID;
	ranapIesValuePdu[id_RAB_SetupList_EnhRelocInfoReq] = &asn_DEF_RAB_SetupList_EnhRelocInfoReq;
	ranapIesValuePdu[id_SNA_Access_Information] = &asn_DEF_SNA_Access_Information;
	ranapIesValuePdu[id_UESBI_Iu] = &asn_DEF_UESBI_Iu;
	ranapIesValuePdu[id_SelectedPLMN_ID] = &asn_DEF_PLMNidentity;
	ranapIesValuePdu[id_CNMBMSLinkingInformation] = &asn_DEF_CNMBMSLinkingInformation;
	ranapIesValuePdu[id_RAB_SetupItem_EnhRelocInfoReq] = &asn_DEF_RAB_SetupItem_EnhRelocInfoReq;
	ranapIesValuePdu[id_RAB_SetupList_EnhRelocInfoRes] = &asn_DEF_RAB_SetupList_EnhRelocInfoRes;
	ranapIesValuePdu[id_RAB_FailedList_EnhRelocInfoRes] = &asn_DEF_RAB_FailedList_EnhRelocInfoRes;
	ranapIesValuePdu[id_RAB_SetupItem_EnhRelocInfoRes] = &asn_DEF_RAB_SetupItem_EnhRelocInfoRes;
	ranapIesValuePdu[id_RAB_FailedItem_EnhRelocInfoRes] = &asn_DEF_RAB_FailedItem_EnhRelocInfoRes;
	ranapIesValuePdu[id_RAB_ModifyList] = &asn_DEF_RAB_ModifyList;
	ranapIesValuePdu[id_RAB_ModifyItem] = &asn_DEF_RAB_ModifyItem;
	ranapIesValuePdu[id_LocationRelatedDataRequestType] = &asn_DEF_LocationRelatedDataRequestType;
	ranapIesValuePdu[id_BroadcastAssistanceDataDecipheringKeys] = &asn_DEF_BroadcastAssistanceDataDecipheringKeys;
	ranapIesValuePdu[id_InformationTransferID] = &asn_DEF_InformationTransferID;
	ranapIesValuePdu[id_ProvidedData] = &asn_DEF_ProvidedData;
	ranapIesValuePdu[id_GlobalCN_ID] = &asn_DEF_GlobalCN_ID;
	ranapIesValuePdu[id_InterSystemInformationTransferType] = &asn_DEF_InterSystemInformationTransferType;
	ranapIesValuePdu[id_InformationExchangeID] = &asn_DEF_InformationExchangeID;
	ranapIesValuePdu[id_InformationExchangeType] = &asn_DEF_InformationExchangeType;
	ranapIesValuePdu[id_InformationTransferType] = &asn_DEF_InformationTransferType;
	ranapIesValuePdu[id_InformationRequestType] = &asn_DEF_InformationRequestType;
	ranapIesValuePdu[id_InformationRequested] = &asn_DEF_InformationRequested;
	ranapIesValuePdu[id_TMGI] = &asn_DEF_TMGI;
	ranapIesValuePdu[id_MBMSSessionIdentity] = &asn_DEF_MBMSSessionIdentity;
	ranapIesValuePdu[id_MBMSBearerServiceType] = &asn_DEF_MBMSBearerServiceType;
	ranapIesValuePdu[id_RAB_Parameters] = &asn_DEF_RAB_Parameters;
	ranapIesValuePdu[id_PDP_TypeInformation] = &asn_DEF_PDP_TypeInformation;
	ranapIesValuePdu[id_MBMSSessionDuration] = &asn_DEF_MBMSSessionDuration;
	ranapIesValuePdu[id_MBMSServiceArea] = &asn_DEF_MBMSServiceArea;
	ranapIesValuePdu[id_FrequenceLayerConvergenceFlag] = &asn_DEF_FrequenceLayerConvergenceFlag;
	ranapIesValuePdu[id_RAListofIdleModeUEs] = &asn_DEF_RAListofIdleModeUEs;
	ranapIesValuePdu[id_MBMSSessionRepetitionNumber] = &asn_DEF_MBMSSessionRepetitionNumber;
	ranapIesValuePdu[id_TimeToMBMSDataTransfer] = &asn_DEF_TimeToMBMSDataTransfer;
	ranapIesValuePdu[id_TransportLayerInformation] = &asn_DEF_TransportLayerInformation;
	ranapIesValuePdu[id_SessionUpdateID] = &asn_DEF_SessionUpdateID;
	ranapIesValuePdu[id_DeltaRAListofIdleModeUEs] = &asn_DEF_DeltaRAListofIdleModeUEs;
	ranapIesValuePdu[id_MBMSCNDe_Registration] = &asn_DEF_MBMSCNDe_Registration;
	ranapIesValuePdu[id_JoinedMBMSBearerServicesList] = &asn_DEF_JoinedMBMSBearerService_IEs;
	ranapIesValuePdu[id_LeftMBMSBearerServicesList] = &asn_DEF_LeftMBMSBearerService_IEs;
	ranapIesValuePdu[id_UnsuccessfulLinkingList] = &asn_DEF_UnsuccessfulLinking_IEs;
	ranapIesValuePdu[id_MBMSRegistrationRequestType] = &asn_DEF_MBMSRegistrationRequestType;
	ranapIesValuePdu[id_IPMulticastAddress] = &asn_DEF_IPMulticastAddress;
	ranapIesValuePdu[id_APN] = &asn_DEF_APN;
	ranapIesValuePdu[id_IntegrityProtectionKey] = &asn_DEF_IntegrityProtectionKey;
	ranapIesValuePdu[id_EncryptionKey] = &asn_DEF_EncryptionKey;
	ranapIesValuePdu[id_SRVCC_Information] = &asn_DEF_SRVCC_Information;
}

void ranapANYValuePduInit(){
	ranapANYValuePdu[INITIATING_MESSAGE] = initiatingValuePdu;
	ranapANYValuePdu[SUCCESSFUL_OUTCOME] = successfulValuePdu;
	ranapANYValuePdu[UNSUCCESSFUL_OUTCOME] = unsuccessfulValuePdu;
	ranapANYValuePdu[OUTCOME] = outcomeValuePdu;
	ranapANYValuePdu[PROTOCOL_IES] = ranapIesValuePdu;
}
void ranap_pduInit(){
        initiatingValuesInit();
        successfulValuesInit();
        unsuccessfulValuesInit();
        outcomeValuesInit();
        ieValueTypeInit();
		ranapANYValuePduInit();
}


asn_dec_rval_t
SEQUENCE_decode_uper_for_RANAP(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
	asn_per_constraints_t *constraints, void **sptr, asn_per_data_t *pd) {
#if RANAP_FULL_ENDECODE
	asn_SEQUENCE_specifics_t *specs = (asn_SEQUENCE_specifics_t *)td->specifics;
	void *st = *sptr;	/* Target structure. */
	int extpresent;		/* Extension additions are present */
	uint8_t *opres;		/* Presence of optional root members */
	asn_per_data_t opmd;
	asn_dec_rval_t rv;
	int edx;
	long id = 255;

	(void)constraints;

	if(_ASN_STACK_OVERFLOW_CHECK(opt_codec_ctx))
		_ASN_DECODE_FAILED;

	if(!st) {
		st = *sptr = CALLOC(1, specs->struct_size);
		if(!st) _ASN_DECODE_FAILED;
	}

	ASN_DEBUG("Decoding %s as SEQUENCE (UPER)", td->name);

	/* Handle extensions */
	if(specs->ext_before >= 0) {
		extpresent = per_get_few_bits(pd, 1);
		if(extpresent < 0) _ASN_DECODE_STARVED;
	} else {
		extpresent = 0;
	}

	/* Prepare a place and read-in the presence bitmap */
	memset(&opmd, 0, sizeof(opmd));
	if(specs->roms_count) {
		opres = (uint8_t *)MALLOC(((specs->roms_count + 7) >> 3) + 1);
		if(!opres) _ASN_DECODE_FAILED;
		/* Get the presence map */
		if(per_get_many_bits(pd, opres, 0, specs->roms_count)) {
			FREEMEM(opres);
			_ASN_DECODE_STARVED;
		}
		opmd.buffer = opres;
		opmd.nbits = specs->roms_count;
		ASN_DEBUG("Read in presence bitmap for %s of %d bits (%x..)",
			td->name, specs->roms_count, *opres);
	} else {
		opres = 0;
	}

	/*
	 * Get the sequence ROOT elements.
	 */
	 
	for(edx = 0; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		void *memb_ptr;		/* Pointer to the member */
		void **memb_ptr2;	/* Pointer to that pointer */
		ASN_DEBUG("td %s elm name %d %s",td->name, edx, elm->type->name);
		if(IN_EXTENSION_GROUP(specs, edx))
			continue;

		/* Fetch the pointer to this member */
		if(elm->flags & ATF_POINTER) {
			memb_ptr2 = (void **)((char *)st + elm->memb_offset);
		} else {
			memb_ptr = (char *)st + elm->memb_offset;
			memb_ptr2 = &memb_ptr;
		}

		/* Deal with optionality */
		if(elm->optional) {
			int present = per_get_few_bits(&opmd, 1);
			ASN_DEBUG("Member %s->%s is optional, p=%d (%d->%d)",
				td->name, elm->name, present,
				(int)opmd.nboff, (int)opmd.nbits);
			if(present == 0) {
				/* This element is not present */
				if(elm->default_value) {
					/* Fill-in DEFAULT */
					if(elm->default_value(1, memb_ptr2)) {
						FREEMEM(opres);
						_ASN_DECODE_FAILED;
					}
					ASN_DEBUG("Filled-in default");
				}
				/* The member is just not present */
				continue;
			}
			/* Fall through */
		}
		
		/* Fetch the member from the stream */
		ASN_DEBUG("Decoding member %s in %s", elm->name, td->name);		
		if(!strcmp(elm->type->name, "ANY")){
			rv = ANY_decode_per_sub_PDU(opt_codec_ctx, elm->per_constraints, memb_ptr2, pd, td->name, id, RANAP);			
		}
		else{ 
			rv = elm->type->uper_decoder(opt_codec_ctx, elm->type,
				elm->per_constraints, memb_ptr2, pd);
			
			if((rv.code == RC_OK)&&\
					((!strcmp(elm->type->name, "ProtocolIE-ID"))||\
					(!strcmp(elm->type->name, "ProtocolExtensionID"))||\
					(!strcmp(elm->type->name, "PrivateIE-ID"))||\
					(!strcmp(elm->type->name, "ProcedureCode")))){
				long * tmpvalue = (long *) * memb_ptr2;
				id = *tmpvalue;
			}			
		}
		if(rv.code != RC_OK) {
			ASN_DEBUG("Failed decode %s in %s",
				elm->name, td->name);
			FREEMEM(opres);
			return rv;
		}
	}

	/* Optionality map is not needed anymore */
	FREEMEM(opres);

	/*
	 * Deal with extensions.
	 */
	if(extpresent) {
		ssize_t bmlength;
		uint8_t *epres;		/* Presence of extension members */
		asn_per_data_t epmd;

		bmlength = uper_get_nslength(pd);
		if(bmlength < 0) _ASN_DECODE_STARVED;

		ASN_DEBUG("Extensions %ld present in %s", (long)bmlength, td->name);

		epres = (uint8_t *)MALLOC((bmlength + 15) >> 3);
		if(!epres) _ASN_DECODE_STARVED;

		/* Get the extensions map */
		if(per_get_many_bits(pd, epres, 0, bmlength))
			_ASN_DECODE_STARVED;

		memset(&epmd, 0, sizeof(epmd));
		epmd.buffer = epres;
		epmd.nbits = bmlength;
		ASN_DEBUG("Read in extensions bitmap for %s of %ld bits (%x..)",
			td->name, (long)bmlength, *epres);

	    /* Go over extensions and read them in */
	    for(edx = specs->ext_after + 1; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		void *memb_ptr;		/* Pointer to the member */
		void **memb_ptr2;	/* Pointer to that pointer */
		int present;

		if(!IN_EXTENSION_GROUP(specs, edx)) {
			ASN_DEBUG("%d is not extension", edx);
			continue;
		}

		/* Fetch the pointer to this member */
		if(elm->flags & ATF_POINTER) {
			memb_ptr2 = (void **)((char *)st + elm->memb_offset);
		} else {
			memb_ptr = (void *)((char *)st + elm->memb_offset);
			memb_ptr2 = &memb_ptr;
		}

		present = per_get_few_bits(&epmd, 1);
		if(present <= 0) {
			if(present < 0) break;	/* No more extensions */
			continue;
		}

		ASN_DEBUG("Decoding member %s in %s %p", elm->name, td->name, *memb_ptr2);
		rv = uper_open_type_get(opt_codec_ctx, elm->type,
			elm->per_constraints, memb_ptr2, pd);
		if(rv.code != RC_OK) {
			FREEMEM(epres);
			return rv;
		}
	    }

		/* Skip over overflow extensions which aren't present
		 * in this system's version of the protocol */
		for(;;) {
			ASN_DEBUG("Getting overflow extensions");
			switch(per_get_few_bits(&epmd, 1)) {
			case -1: break;
			case 0: continue;
			default:
				if(uper_open_type_skip(opt_codec_ctx, pd)) {
					FREEMEM(epres);
					_ASN_DECODE_STARVED;
				}
			}
			break;
		}

		FREEMEM(epres);
	}

	/* Fill DEFAULT members in extensions */
	for(edx = specs->roms_count; edx < specs->roms_count
			+ specs->aoms_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		void **memb_ptr2;	/* Pointer to member pointer */

		if(!elm->default_value) continue;

		/* Fetch the pointer to this member */
		if(elm->flags & ATF_POINTER) {
			memb_ptr2 = (void **)((char *)st
					+ elm->memb_offset);
			if(*memb_ptr2) continue;
		} else {
			continue;	/* Extensions are all optionals */
		}

		/* Set default value */
		if(elm->default_value(1, memb_ptr2)) {
			_ASN_DECODE_FAILED;
		}
	}

	rv.consumed = 0;
	rv.code = RC_OK;
	return rv;
#else
	return SEQUENCE_decode_uper(opt_codec_ctx, td, constraints, sptr, pd);
#endif
}


asn_enc_rval_t
SEQUENCE_encode_uper_for_RANAP(asn_TYPE_descriptor_t *td,
	asn_per_constraints_t *constraints, void *sptr, asn_per_outp_t *po) {
#if RANAP_FULL_ENDECODE
	asn_SEQUENCE_specifics_t *specs
		= (asn_SEQUENCE_specifics_t *)td->specifics;
	asn_enc_rval_t er;
	int n_extensions;
	int edx;
	int i;

	(void)constraints;

	if(!sptr)
		_ASN_ENCODE_FAILED;

	er.encoded = 0;

	ASN_DEBUG("Encoding %s as SEQUENCE (UPER)", td->name);


	/*
	 * X.691#18.1 Whether structure is extensible
	 * and whether to encode extensions
	 */
	if(specs->ext_before >= 0) {
		n_extensions = SEQUENCE_handle_extensions(td, sptr, 0, 0);
		per_put_few_bits(po, n_extensions ? 1 : 0, 1);
	} else {
		n_extensions = 0;	/* There are no extensions to encode */
	}

	/* Encode a presence bitmap */
	for(i = 0; i < specs->roms_count; i++) {
		asn_TYPE_member_t *elm;
		void *memb_ptr;		/* Pointer to the member */
		void **memb_ptr2;	/* Pointer to that pointer */
		int present;

		edx = specs->oms[i];
		elm = &td->elements[edx];

		/* Fetch the pointer to this member */
		if(elm->flags & ATF_POINTER) {
			memb_ptr2 = (void **)((char *)sptr + elm->memb_offset);
			present = (*memb_ptr2 != 0);
		} else {
			memb_ptr = (void *)((char *)sptr + elm->memb_offset);
			memb_ptr2 = &memb_ptr;
			present = 1;
		}

		/* Eliminate default values */
		if(present && elm->default_value
		&& elm->default_value(0, memb_ptr2) == 1)
			present = 0;

		ASN_DEBUG("Element %s %s %s->%s is %s",
			elm->flags & ATF_POINTER ? "ptr" : "inline",
			elm->default_value ? "def" : "wtv",
			td->name, elm->name, present ? "present" : "absent");
		if(per_put_few_bits(po, present, 1))
			_ASN_ENCODE_FAILED;
	}

	/*
	 * Encode the sequence ROOT elements.
	 */
	long id = 255;

	ASN_DEBUG("ext_after = %d, ec = %d, eb = %d", specs->ext_after, td->elements_count, specs->ext_before);
	for(edx = 0; edx < ((specs->ext_after < 0)
		? td->elements_count : specs->ext_before - 1); edx++) {

		asn_TYPE_member_t *elm = &td->elements[edx];
		void *memb_ptr;		/* Pointer to the member */
		void **memb_ptr2;	/* Pointer to that pointer */

		if(IN_EXTENSION_GROUP(specs, edx))
			continue;

		ASN_DEBUG("About to encode %s", elm->type->name);

		/* Fetch the pointer to this member */
		if(elm->flags & ATF_POINTER) {
			memb_ptr2 = (void **)((char *)sptr + elm->memb_offset);
			if(!*memb_ptr2) {
				ASN_DEBUG("Element %s %d not present",
					elm->name, edx);
				if(elm->optional)
					continue;
				/* Mandatory element is missing */
				_ASN_ENCODE_FAILED;
			}
		} else {
			memb_ptr = (void *)((char *)sptr + elm->memb_offset);
			memb_ptr2 = &memb_ptr;
		}

		/* Eliminate default values */
		if(elm->default_value && elm->default_value(0, memb_ptr2) == 1)
			continue;

		ASN_DEBUG("Encoding %s->%s", td->name, elm->name);

		if(!strcmp(elm->type->name, "ANY")){
			er = ANY_encode_per_sub_PDU(elm->per_constraints, *memb_ptr2, po, td->name, id, RANAP);
		}
		else{
			if((!strcmp(elm->type->name, "ProtocolIE-ID"))||\
					(!strcmp(elm->type->name, "ProtocolExtensionID"))||\
					(!strcmp(elm->type->name, "ProcedureCode"))||\
					(!strcmp(elm->type->name, "PrivateIE-ID"))){
				long * tmpvalue = (long *) * memb_ptr2;
				id = *tmpvalue;
			}
			er = elm->type->uper_encoder(elm->type, elm->per_constraints,
				*memb_ptr2, po);
		}	
		if(er.encoded == -1)
			return er;
	}	

	/* No extensions to encode */
	if(!n_extensions) _ASN_ENCODED_OK(er);

	ASN_DEBUG("Length of %d bit-map", n_extensions);
	/* #18.8. Write down the presence bit-map length. */
	if(uper_put_nslength(po, n_extensions))
		_ASN_ENCODE_FAILED;

	ASN_DEBUG("Bit-map of %d elements", n_extensions);
	/* #18.7. Encoding the extensions presence bit-map. */
	/* TODO: act upon NOTE in #18.7 for canonical PER */
	if(SEQUENCE_handle_extensions(td, sptr, po, 0) != n_extensions)
		_ASN_ENCODE_FAILED;

	ASN_DEBUG("Writing %d extensions", n_extensions);
	/* #18.9. Encode extensions as open type fields. */
	if(SEQUENCE_handle_extensions(td, sptr, 0, po) != n_extensions)
		_ASN_ENCODE_FAILED;

	_ASN_ENCODED_OK(er);
#else
	return SEQUENCE_encode_uper(td, constraints, sptr, po);
#endif
}

void
SEQUENCE_free_for_RANAP(asn_TYPE_descriptor_t *td, void *sptr, int contents_only) {
#if RANAP_FULL_ENDECODE
	int edx;

	if(!td || !sptr)
		return;

	ASN_DEBUG("Freeing %s as SEQUENCE", td->name);
	
	long id = 255;
	for(edx = 0; edx < td->elements_count; edx++) {
		asn_TYPE_member_t *elm = &td->elements[edx];
		void *memb_ptr;
		if(elm->flags & ATF_POINTER) {
			memb_ptr = *(void **)((char *)sptr + elm->memb_offset);
			if(memb_ptr){
				ASN_STRUCT_FREE(*elm->type, memb_ptr);
				memb_ptr = NULL;
			}
		} 
		else{		
			memb_ptr = (void *)((char *)sptr + elm->memb_offset);
			if(!strcmp(td->name, "ANY")){
				ANY_free_sub_PDU(sptr, td->name, id, RANAP);
			}
			else {				
				ASN_STRUCT_FREE_CONTENTS_ONLY(*elm->type, memb_ptr);
			}
		}
		if(((!strcmp(elm->type->name, "ProtocolIE-ID"))||\
				(!strcmp(elm->type->name, "ProtocolExtensionID"))||\
				(!strcmp(elm->type->name, "PrivateIE-ID"))||\
				(!strcmp(elm->type->name, "ProcedureCode")))){
			long * tmpvalue = (long *) memb_ptr;
			id = *tmpvalue;
		}
	}

	if(!contents_only) {
		FREEMEM(sptr);
	}
#else
	SEQUENCE_free(td, sptr, contents_only);
#endif
}

asn_dec_rval_t ANY_decode_per_sub_PDU
(
	asn_codec_ctx_t *opt_codec_ctx, 
	asn_per_constraints_t *constraints, 
	void **sptr, 
	asn_per_data_t *pd,
	char * parentTdName,
	unsigned int id,
	protocol_type_e flag   /* RANAP, RUA, or HNBAP*/
)
{
	asn_dec_rval_t rv;
	OCTET_STRING_t * octets = NULL;
	ranap_parent_pdu_e parentPDU = PARENT_PDU_NONE;
	asn_per_data_t tmppd;
	void *structure = NULL;
	char * protocol = (flag==0 ? "RANAP": (flag == 1 ? "RUA" : "HNBAP"));
	asn_TYPE_descriptor_t *** protocolANYValuePdu = NULL;
    
	if(RANAP == flag){
		protocolANYValuePdu = ranapANYValuePdu;
	}
	else{
		/*RUA or HNBAP*/;
	}
	if(!sptr || !((void *)(*sptr))||(id >= MAX_PDU)){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, sptr ? (*sptr ? "id range error" : "(*sptr) is null") : "sptr is null");
		DECODE_FAILED;
	}
	if(!opt_codec_ctx || !constraints || !pd || !parentTdName){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, "null point");
		DECODE_FAILED;
	}
	rv = asn_DEF_ANY.uper_decoder(opt_codec_ctx, &asn_DEF_ANY,
			constraints, sptr, pd);
	if(rv.code != RC_OK) {
		ASN_DEBUG("Failed decode value in %s", parentTdName);
		return rv;
	}
	memset (&rv, 0, sizeof(asn_dec_rval_t));
	memset(&tmppd,0,sizeof(asn_per_data_t));
	octets = (OCTET_STRING_t *)*sptr;
	tmppd.buffer = octets->buf;
	tmppd.nboff = 0;
	tmppd.nbits=octets->size * 8;	
	if(!strcmp(parentTdName,"InitiatingMessage")){
		parentPDU = INITIATING_MESSAGE;
	}
	else if(!strcmp(parentTdName,"SuccessfulOutcome")){
		parentPDU = SUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"UnsuccessfulOutcome")){
		parentPDU = UNSUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"Outcome")){
		parentPDU = OUTCOME;
	}
	else if(!strcmp(parentTdName,"SEQUENCE")){
		parentPDU = PROTOCOL_IES;
	}
	
	if((parentPDU >= INITIATING_MESSAGE && parentPDU < PARENT_PDU_NONE)&& \
		protocolANYValuePdu && protocolANYValuePdu[parentPDU]&&\
		protocolANYValuePdu[parentPDU][id]){
		rv = uper_decode(opt_codec_ctx, protocolANYValuePdu[parentPDU][id], (void **)&structure,tmppd.buffer,(tmppd.nbits + 7)/8, 0,0);
		if(rv.code != RC_OK) {
			ASN_DEBUG("Failed Decode ANY type for %s %s id %d: %s", protocol, parentTdName, id, "decode failed");
			return rv;
		}

		octets->buf=structure;	
		octets->size = rv.consumed;
		if(tmppd.buffer){
			FREEMEM(tmppd.buffer);
			tmppd.buffer = NULL;
		}
	}
	else{
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", protocol,parentTdName, id, 
			(parentPDU >= INITIATING_MESSAGE && parentPDU < PARENT_PDU_NONE) ? "Unknown sub PDU": "Unknown parent PDU");
		DECODE_FAILED;
	}
	return rv;
}


asn_enc_rval_t ANY_encode_per_sub_PDU
(	
	asn_per_constraints_t *constraints, 
	void *sptr, 
	asn_per_outp_t *po,
	char * parentTdName,
	unsigned int id,
	protocol_type_e flag	
)
{
	asn_enc_rval_t er;
	char buffer[64*1024] = {0};
	//asn_per_outp_t tmppo;
	OCTET_STRING_t tmpoctets;
	memset(&tmpoctets, 0 , sizeof(OCTET_STRING_t));
	OCTET_STRING_t * octets = NULL;
	ranap_parent_pdu_e parentPDU = PARENT_PDU_NONE;
	//void *structure = NULL;
	char * protocol = (flag==0 ? "RANAP": (flag == 1 ? "RUA" : "HNBAP"));
	asn_TYPE_descriptor_t *** protocolANYValuePdu = NULL;
	if(RANAP == flag){
		protocolANYValuePdu = ranapANYValuePdu;
	}
	else{
		/*RUA or HNBAP*/;
	}
	if(!sptr || (id >= MAX_PDU)){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, sptr ? "id range error" : "sptr is null");
		ENCODE_FAILED;
	}
	if(!constraints || !po || !parentTdName){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, "null point");
		ENCODE_FAILED;
	}
	octets = (OCTET_STRING_t *) sptr;
	if(!strcmp(parentTdName,"InitiatingMessage")){
		parentPDU = INITIATING_MESSAGE;
	}
	else if(!strcmp(parentTdName,"SuccessfulOutcome")){
		parentPDU = SUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"UnsuccessfulOutcome")){
		parentPDU = UNSUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"Outcome")){
		parentPDU = OUTCOME;
	}
	else if(!strcmp(parentTdName,"SEQUENCE")){
		parentPDU = PROTOCOL_IES;
	}

	if((parentPDU >= INITIATING_MESSAGE && parentPDU < PARENT_PDU_NONE)&& \
		protocolANYValuePdu && protocolANYValuePdu[parentPDU]&&\
		protocolANYValuePdu[parentPDU][id]){			
		er = uper_encode_to_buffer(protocolANYValuePdu[parentPDU][id], (octets->buf), buffer, sizeof(buffer));
		if(er.encoded == -1){
			ASN_DEBUG("Failed Encoding value %s in %s",protocolANYValuePdu[parentPDU][id]->name, parentTdName);
			return er;
		}
		ANY_fromBuf(&tmpoctets,buffer,(er.encoded+7)/8);
		
		er = asn_DEF_ANY.uper_encoder(&asn_DEF_ANY, constraints,
			&tmpoctets, po);		
	}
	else{
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", protocol,parentTdName, id, 
			(parentPDU >= INITIATING_MESSAGE && parentPDU < PARENT_PDU_NONE) ? "Unknown sub PDU": "Unknown parent PDU");
		ENCODE_FAILED;
	}
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_OCTET_STRING, &tmpoctets);
	return er;
}

asn_dec_rval_t ANY_free_sub_PDU(void *sptr, char * parentTdName, unsigned int id, int flag){
	OCTET_STRING_t * bits = (OCTET_STRING_t *) sptr;
	asn_dec_rval_t rv;
	ranap_parent_pdu_e parentPDU = PARENT_PDU_NONE;
	char * protocol = (flag==0 ? "RANAP": (flag == 1 ? "RUA" : "HNBAP"));
	asn_TYPE_descriptor_t *** protocolANYValuePdu = NULL;
    
	if(RANAP == flag){
		protocolANYValuePdu = ranapANYValuePdu;
	}
	else{
		/*RUA or HNBAP*/;
	}
	if(!sptr ||(id >= MAX_PDU)){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, sptr ?  "id range error"  : "sptr is null");
		DECODE_FAILED;
	}
	if(!parentTdName){
		ASN_DEBUG("Failed to Decode ANY type for %s %s id %d: %s", 
			protocol, parentTdName, id, "null point");
		DECODE_FAILED;
	}

	bits = (OCTET_STRING_t *) sptr;
	
	if(!strcmp(parentTdName,"InitiatingMessage")){
		parentPDU = INITIATING_MESSAGE;
	}
	else if(!strcmp(parentTdName,"SuccessfulOutcome")){
		parentPDU = SUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"UnsuccessfulOutcome")){
		parentPDU = UNSUCCESSFUL_OUTCOME;
	}
	else if(!strcmp(parentTdName,"Outcome")){
		parentPDU = OUTCOME;
	}
	else if(!strcmp(parentTdName,"SEQUENCE")){
		parentPDU = PROTOCOL_IES;
	}
	
	if((parentPDU >= INITIATING_MESSAGE && parentPDU < PARENT_PDU_NONE)&& \
		protocolANYValuePdu && protocolANYValuePdu[parentPDU]&&\
		protocolANYValuePdu[parentPDU][id]){
		ASN_STRUCT_FREE(*protocolANYValuePdu[parentPDU][id], bits->buf);
		bits->buf = NULL;
	}
	else{
		ASN_DEBUG("Unknown DEF TYPE when free ANY type PDU");
	}
	return rv;
}




