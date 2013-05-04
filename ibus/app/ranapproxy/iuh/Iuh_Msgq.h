#ifndef _IUH_MSGQ_H
#define _IUH_MSGQ_H

#include "iuh/Iuh.h"

typedef enum{
	HNBAP_DATA = 1,
	RUA_DATA = 2,
	IU_DATA = 3,
	CONTROL_DATA = 4 //book add, 2011-12-21
}MQType;

typedef enum{
	IU_S_TYPE = 1,
	CONNECTIONLESS = 2
}MQSubType;

typedef struct{
	unsigned int len;
	char Data[2048];
}MQ_DATA;


/* book add, 2011-12-21 */

struct msg_ue_info{
	uint32_t UEID;
	unsigned char IMSI[IMSI_LEN];
	unsigned char ContextID[CONTEXTID_LEN];
};

typedef struct msg_ue{
	eUEOP op;
	struct msg_ue_info ueInfo;
}MsgUE;

typedef struct{
	int HNBID;
	MQType type;
	MQSubType subtype;
	union{
		MQ_DATA DataInfo;
		MsgUE ueInfo;
	}u;
}msgqdetail;
/* book add end */

typedef struct{
	long mqid;
	msgqdetail mqinfo;
}msgq;


typedef enum HNBAP_ProcedureCode_ID_{
	HNBRegister = 1,
	HNBDeRegister = 2,
	UERegister = 3,
	UEDeRegister = 4,
	ProcErrorIndication = 5,
	privateMessage = 6,
	CSGMembership = 7,
}HNBAP_ProcedureCode_ID;

typedef enum RUA_ProcedureCode_ID_{
	Pro_Connect = 1,
	Pro_DirectTransfer = 2,
	Pro_Disconnect = 3,
	Pro_ConnectionlessTransfer = 4,
	Pro_ErrorIndication = 5,
	Pro_privateMessage = 6,
}RUA_ProcedureCode_ID;

typedef enum RANAP_ProcedureCode_ID_{
	Pro_id_RAB_Assignment = 0,
	Pro_id_Iu_Release = 1,                   //disconnect
	Pro_id_RelocationPreparation = 2,
	Pro_id_RelocationResourceAllocation = 3,
	Pro_id_RelocationCancel = 4,
	Pro_id_SRNS_ContextTransfer = 5,
	Pro_id_SecurityModeControl = 6,
	Pro_id_DataVolumeReport = 7,
	Pro_id_Reset = 9,                       //connectionless
	Pro_id_RAB_ReleaseRequest = 10,
	Pro_id_Iu_ReleaseRequest = 11,
	Pro_id_RelocationDetect = 12,
	Pro_id_RelocationComplete = 13,
	Pro_id_Paging = 14,                      //connectionless
	Pro_id_CommonID = 15,
	Pro_id_CN_InvokeTrace = 16,
	Pro_id_LocationReportingControl = 17,
	Pro_id_LocationReport = 18,
	Pro_id_InitialUE_Message = 19,         //connect
	Pro_id_DirectTransfer = 20,
	Pro_id_OverloadControl = 21,              //connectionless
	Pro_id_ErrorIndication = 22,              //connection | connectionless
	Pro_id_SRNS_DataForward = 23,
	Pro_id_ForwardSRNS_Context = 24,
	Pro_id_privateMessage = 25,
	Pro_id_CN_DeactivateTrace = 26,
	Pro_id_ResetResource = 27,              //connectionless
	Pro_id_RANAP_Relocation = 28,
	Pro_id_RAB_ModifyRequest = 29,
	Pro_id_LocationRelatedData = 30,
	Pro_id_InformationTransfer = 31,          //connectionless
	Pro_id_UESpecificInformation = 32,
	Pro_id_UplinkInformationExchange = 33,  //connectionless
	Pro_id_DirectInformationTransfer = 34,  //connectionless
	Pro_id_MBMSSessionStart = 35,
	Pro_id_MBMSSessionUpdate = 36,
	Pro_id_MBMSSessionStop = 37,
	Pro_id_MBMSUELinking = 38,
	Pro_id_MBMSRegistration = 39,              //connection | connectionless
	Pro_id_MBMSCNDe_Registration_Procedure = 40, //connectionless
	Pro_id_MBMSRABEstablishmentIndication = 41,
	Pro_id_MBMSRABRelease = 42,
	Pro_id_enhancedRelocationComplete = 43,
	Pro_id_enhancedRelocationCompleteConfirm = 44,
	Pro_id_RANAPenhancedRelocation = 45,
	Pro_id_SRVCCPreparation = 46
}RANAP_ProcedureCode_ID;


typedef enum HNBAP_IE_ID_t_{
	HNBAP_id_Cause = 1,
	HNBAP_CriticalityDiagnostics = 2,
	HNBAP_HNB_Ident = 3,
	HNBAP_Context_ID = 4,
	HNBAP_UE_Identity = 5,
	HNBAP_LAC = 6,
	HNBAP_RAC = 7,
	HNBAP_HNB_Location_Info = 8,
	HNBAP_PLMNid = 9,
	HNBAP_SAC = 10,
	HNBAP_CellId = 11,
	HNBAP_Registration_Cause = 12,
	HNBAP_UE_Capabilities = 13,
	HNBAP_RNC_ID = 14,
	HNBAP_CSG_ID = 15,
	HNBAP_BackoffTimer = 16,
	HNBAP_HNB_Internet_Information = 17,
	HNBAP_HNB_Cell_Access_Mode = 18,
	HNBAP_MuxPortNumber = 19,
	HNBAP_Service_Area_For_Broadcast = 20,
	HNBAP_CSGMembershipStatus = 21,
	/*...*/
	MAX_MEMBER_ID,
}HNBAP_IE_ID_t;


typedef enum RUA_IE_ID_t_{
	RUA_Cause = 1,
	RUA_CriticalityDiagnostics = 2,
	RUA_Context_ID = 3,
	RUA_RANAP_Message = 4,
	RUA_IntraDomainNasNodeSelector = 5,
	RUA_Establishment_Cause = 6,
	RUA_CN_DomainIndicator = 7,
	RUA_vCSGMembershipStatus = 8,
}RUA_IE_ID_t;


typedef enum RANAP_IE_ID_t_{
	RANAP_id_CN_DomainIndicator = 3
}RANAP_IE_ID_t;


typedef struct{
    unsigned char   HnbIdentity[HNB_IDENTITY_LEN];	//HNB identity information   OCTET STRING(1..255) see 9.2.2
    HNBLocationInformation	HnbLocationInfo;
    unsigned char	    plmnId[PLMN_LEN];		//Public Land Mobile Network ID  size 24
	unsigned char	    cellId[CELLID_LEN];		//标识PLMN中的唯一Cell   size 28
    unsigned char	    lac[LAC_LEN];		//位置区域码
	unsigned char	    rac[RAC_LEN];		//路由区域吗
	unsigned char    	sac[SAC_LEN];		//服务区域码
	unsigned char	    csgId[CSGID_LEN];		//封闭用户组ID   size 27
	int                 acl_switch;  //book add ,2012-1-4
}HnbRegisterRequestValue;

typedef struct{
    UE_IDENTITY_S  UE_Identity;
    int registrationCause;    //0 emergency_call, 1 normal
	struct UECapability	  Capabilities;
	char IMSI[IMSI_LEN];
}UERegisterRequestValue;

#endif
