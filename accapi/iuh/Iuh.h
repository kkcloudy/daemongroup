#ifndef _IUH_H_
#define	_IUH_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/tipc.h>
#include "../../ibus/app/ranapproxy/iuh/Iuh_SockOP.h"

/* for cn_simulator test */
//#define SIMULATOR_CN
//#define SIGTRAN_PROC

/* book add for change block time of sctp connection, 2011-12-7 */
//#define CONNECT_BLOCK

/* book add for multi-homing features of sctp connection, 2011-12-9 */
//#define SCTP_MULTI_HOMING

/* book add for add & del iuSigConId, 2011-12-16 */
#define IU_SIG_CON_ID

/* book add for reset from HNB, 2011-12-15 */
#define HNB_INIT_RESET

/* book add for reset from HNB, 2011-12-19 */
//#define RAB_INFO

/* book add for reset from HNB, 2011-12-20 */
#define INBOUND_HANDOVER

/*xiaodawei add for acl femto imsi white list*/
#define IMSI_WHITE_LIST_SWITCH



#define PATH_LEN                    (64)
#define THREAD_NUM	                (16)
#define SOCK_NUM                    (16)
#define SOCK_BUFSIZE                (8*1024*1024)
#define IF_NAME_LEN                 (16)
#define NAS_IDENTIFIER_NAME         (128)
#define	IFI_ADDR_NUM	            (8)
#define HNB_DEFAULT_NUM_AUTELAN     (1024)
#define DEFAULT_LEN		            (128)
#define HNB_DEFAULT_NUM_OEM	        (2)
#define HNB_MAX_UE_NUM              (16)
#define UE_MAX_NUM_AUTELAN          (HNB_DEFAULT_NUM_AUTELAN * HNB_MAX_UE_NUM)
#define UE_MAX_RAB_NUM              (32)
#define RAB_MAX_NUM_AUTELAN         (UE_MAX_RAB_NUM * UE_MAX_NUM_AUTELAN)
#define HNBAP_PPID                  (20)
#define RUA_PPID                    (19)

#define CONTEXTID_LEN               (3)
#define IMSI_LEN                    (8)
#define RANAP_LEN                   (1024)
#define PLMN_LEN                    (3)
#define IPV4_LEN                    (4)
#define IPV6_LEN                    (16)
#define LAC_LEN                     (2)
#define RAC_LEN                     (1)
#define TMSI_LEN                    (4)
#define ESN_LEN                     (4)
#define IMEI_LEN                    (8)
#define CELLID_LEN                  (4)
#define CI_LEN                      (2)
#define SAC_LEN                     (2)
#define CSGID_LEN                   (4)
#define RAB_ID_LEN                  (1)
#define NAS_INDICATOR_LEN           (1)
#define GTP_TEID_LEN                (4)
#define BINDING_ID_LEN              (4)
#define TRANSPORT_LAYER_ADDR_LEN    (20)
#define USER_PLANE_MODE_LEN         (2)
#define IU_SIG_CONN_ID_LEN			(3)

#define HNB_IDENTITY_LEN            (256)
#define HNB_NAME_LEN                (32)
#define HNB_MODEL_LEN               (32)
#define HNB_IP_LEN                  (16)
#define IMSI_DIGIT_LEN				(15)


#define ASP_PORCESS_MSC 	0x00000000
#define ASP_PORCESS_SGSN	0x00000001

#define MAX_SLOT_NUM				(16)
#define IUH_MAX_INS_NUM			(4)
#define IUH_TIPC_SERVER_TYPE		0x8000
#define IU_TIPC_SERVER_TYPE			0x9000
#define FEMTO_SERVER_BASE_INST	128

typedef struct{
	struct sockaddr_un addr;
	int 	addrlen;
}unixAddr;

typedef enum {
	Iuh_FALSE = 0,
	Iuh_TRUE = 1
} IuhBool;

#define		IUH_FREE_OBJECT(obj_name)				            {if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define		IUH_FREE_OBJECTS_ARRAY(ar_name, ar_size)			{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }

// custom error
#define		IUH_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define		IUH_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}
#define		IUH_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
#define		IUH_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
#define		IUH_CREATE_STRING_FROM_STRING_ERR(str_name, str, on_err)	{IUH_CREATE_STRING_ERR(str_name, strlen(str), on_err); strcpy((str_name), str);}


typedef unsigned char 		uint8_t;
typedef unsigned short 		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long  uint64_t;

struct  IuhHnb;
/*auto hnb area*/
struct auto_hnb_if {
  unsigned int ifindex;
  unsigned char	ifname[IF_NAME_LEN];  
  struct auto_hnb_if  *ifnext;	
};

typedef struct auto_hnb_if iuh_auto_hnb_if;


typedef struct {
    unsigned char	auto_hnb_switch;
    unsigned char	save_switch;
    unsigned char	ifnum;
    unsigned int	list_len;
    char *ifname;
    iuh_auto_hnb_if	*auto_hnb_if;
}iuh_auto_hnb_info;


/* *****************************
** RAB information include GTP-U information
* *****************************/

/* enum of Traffic Class Type */
typedef enum {
    Tfc_conversational	= 0,
	Tfc_streaming	= 1,
	Tfc_interactive	= 2,
	Tfc_background	= 3
}Traffic_Class;

/* enum of RAB Assignment Indicator */
typedef enum {
    Symmetric_bidirectional	= 0,
	Asymmetric_unidirectional_downlink	= 1,
	Asymmetric_unidirectional_uplink	= 2,
	Asymmetric_bidirectional	= 3
}RabAssignmentIndicator;

/* enum of Delivery Order */
typedef enum{
    Requested	= 0,
	Not_requested	= 1
}Delivery_Order;

/* RAB parameters list */
struct rabParasList{
    Traffic_Class traffic_class;
    RabAssignmentIndicator rab_ass_indicator;
    Delivery_Order deliveryOrder;
    unsigned int maxSDUSize;
};

/* enum of User Plane Mode */
typedef enum{
    Transparent_mode	= 0,
	Support_mode_for_predefined_SDU_sizes	= 1
}UserPlane_Mode;

/* User Plane Information */
struct userPlaneInfo{
	UserPlane_Mode	 user_plane_mode;
	char	 up_modeVersions[USER_PLANE_MODE_LEN];
};

/* enum of IU Transport Association Present */
typedef enum{
	IuTransportAssociation_NOTHING,	/* No components present */
	IuTransportAssociation_gTP_TEI,
	IuTransportAssociation_bindingID
}IuTransportAssociation_P;

/* Iu Transport Association union */
typedef struct iuTransportAssoc {
	IuTransportAssociation_P present;
	union IuTransportAssoc_u {
		char	 gtp_tei[GTP_TEID_LEN];
		char	 binding_id[BINDING_ID_LEN];
	} choice;
} Iu_TransportAssociation;

/* Transport Layer information */
struct transportLayerInfo {
	char	 transport_layer_addr[TRANSPORT_LAYER_ADDR_LEN];
	Iu_TransportAssociation	 iu_trans_assoc;
};

/* enum of Service Handover */
typedef enum {
	Handover_to_GSM_should_be_performed	= 0,
	Handover_to_GSM_should_not_be_performed	= 1,
	Handover_to_GSM_shall_not_be_performed	= 2
}ServiceHandover;


/****************************
** definations relevant to GTP-U 
*****************************/

/* enum of pdp type */
typedef enum{
	PDPType_empty	= 0,
	PDPType_ppp	= 1,
	PDPType_osp_ihoss	= 2,
	PDPType_ipv4	= 3,
	PDPType_ipv6	= 4
} PDPType;

/* pdp type information */
struct pdpTypeInfo{
    PDPType pdp_type;
    struct pdpTypeInfo *next;
};

/* pdp type list */
typedef struct pdpType_L{
    int count;
    struct pdpTypeInfo *pdp_type_info;
}PDPTypeList;


typedef enum {
	DataVolume_do_report	= 0,
	DataVolume_do_not_report	= 1
} DataVolumeRptIndication;


/* RAB information include GTP-U information */
typedef struct rab{
    /* first */
    unsigned char RABID[RAB_ID_LEN];
    unsigned char nAS_ScrIndicator[NAS_INDICATOR_LEN];
    struct rabParasList rab_para_list;
    struct userPlaneInfo user_plane_info;
    struct transportLayerInfo trans_layer_info;
    ServiceHandover service_handover;

    int isPsDomain;
    
    /* second   GTP-U */
    PDPTypeList *pdp_type_list;
    DataVolumeRptIndication data_vol_rpt;
    unsigned short int dl_GTP_PDU_SeqNum;
    unsigned short int ul_GTP_PDU_seqNum;
    unsigned short int dl_N_PDU_SeqNum;
    unsigned short int ul_N_PDU_SeqNum;

    uint32_t UEID;      // ue id of current rab

    struct rab *rab_next;  
}Iu_RAB;

/* -----------RAB information---------- */

extern Iu_RAB **IU_RAB;



typedef struct  Lai
{
	unsigned char    plmnid[PLMN_LEN];//size 24
	unsigned char    lac[LAC_LEN];
}IUH_LAI;

typedef struct  Rai
{
	struct Lai  lai;
	unsigned char        rac[RAC_LEN];
}IUH_RAI;

typedef struct  tmsi_lai
{
	unsigned char        tmsi[TMSI_LEN];	        //BIT STRING (32)
	struct Lai  lai;	
}Tmsi_Lai;

typedef struct ptmsi_rai{
	unsigned char        ptmsi[TMSI_LEN];		//BIT STRING (SIZE(32))
	struct  Rai  rai;
}Ptmsi_Rai;

typedef struct imsi_esn{
    char 	*imsids41;
    unsigned char 	esn[ESN_LEN];//BIT STRING (SIZE (32))             see 9.2.17
}Imsi_Esn;

typedef enum{
    UNREGISTED,
    REGISTED
}REGISTSTATE;



typedef enum
{
    R99,
    REL4, 
    REL5, 
    REL6, 
    REL7, 
    REL8_AND_BEYOND
}AccStrRelIndicator;

typedef enum
{
    CSG_CAPABLE,
    NOT_CSG_CAPABLE
}CsgIndicator;

struct  UECapability
{
	AccStrRelIndicator accStrRelIndicator;
    CsgIndicator csgIndicator;
};

/* Dependencies */
typedef enum uE_Identity_PR {
	UE_Identity_NOTHING,	/* No components present */
	UE_Identity_iMSI,
	UE_Identity_tMSILAI,
	UE_Identity_pTMSIRAI,
	UE_Identity_iMEI,
	UE_Identity_eSN,
	UE_Identity_iMSIDS41,
	UE_Identity_iMSIESN,
	UE_Identity_tMSIDS41
} UEIdentityPR;

/* UE-Identity */
typedef struct UE_Identity_s {
	UEIdentityPR present;
	union UE_Identity_uu{
		unsigned char        imsi[IMSI_LEN];      //OCTET STRING (SIZE (3..8))      see 9.2.10
		Tmsi_Lai	tmsilai;
		Ptmsi_Rai   ptmsirai;
		unsigned char 	    imei[IMEI_LEN];   //BIT STRING(60)                        see 9.2.18
		unsigned char        esn[ESN_LEN];//BIT STRING (SIZE (32))             see 9.2.17
		char*   	imsids41;  //OCTET STRING (SIZE (5..7))
		Imsi_Esn	imsiesn;
		char*   	tmsids41;  //OCTET STRING (SIZE (2..17))
	} choice;
	
} UE_IDENTITY_S;

typedef enum
{
	EMERGENCY_CALL,
	NORMAL
}RegCause;


typedef enum
{ 
	MEMBER, 
	NON_MEMBER
}CsgState;

typedef enum{
	Cause_NOTHING,	/* No components present */
	Cause_radioNetwork,
	Cause_transport,
	Cause_protocol,
	Cause_misc
}Cause_Present;

typedef enum{
    RadioNetwork_overload	= 0,
	RadioNetwork_unauthorised_Location	= 1,
	RadioNetwork_unauthorised_HNB	= 2,
	RadioNetwork_hNB_parameter_mismatch	= 3,
	RadioNetwork_invalid_UE_identity	= 4,
	RadioNetwork_uE_not_allowed_on_this_HNB	= 5,
    RadioNetwork_uE_unauthorised	= 6,
	RadioNetwork_connection_with_UE_lost	= 7,
	RadioNetwork_ue_RRC_telease	= 8,
	RadioNetwork_hNB_not_registered	= 9,
	RadioNetwork_unspecified	= 10,
	RadioNetwork_normal	= 11,
	RadioNetwork_uE_relocated	= 12,
	RadioNetwork_ue_registered_in_another_HNB	= 13
}Cause_RadioNetwork_Hnbap;


typedef enum CauseRadioNetwork {
	Rua_CauseRadioNetwork_normal	= 0,
	Rua_CauseRadioNetwork_connect_failed	= 1,
	Rua_CauseRadioNetwork_network_release	= 2,
	Rua_CauseRadioNetwork_unspecified	= 3
} Cause_RadioNetwork_Rua;


typedef enum{
    Transport_transport_resource_unavailable	= 0,
	Transport_unspecified	= 1
}Cause_Transport;


typedef enum{
    Protocol_transfer_syntax_error	= 0,
	Protocol_abstract_syntax_error_reject	= 1,
	Protocol_abstract_syntax_error_ignore_and_notify	= 2,
	Protocol_message_not_compatible_with_receiver_state	= 3,
	Protocol_semantic_error	= 4,
	Protocol_unspecified	= 5,
	Protocol_abstract_syntax_error_falsely_constructed_message	= 6
}Cause_Protocol;


typedef enum{
    Misc_processing_overload	= 0,
	Misc_hardware_failure	= 1,
	Misc_o_and_m_intervention	= 2,
	Misc_unspecified	= 3
}Cause_Misc;


typedef struct{
    Cause_Present present;
	union{
		Cause_RadioNetwork_Hnbap	radioNetwork;
		Cause_Transport	            transport;
		Cause_Protocol	            protocol;
		Cause_Misc	                misc;
	} choice;
}HNBAPCause;


typedef struct{
    Cause_Present present;
	union{
		Cause_RadioNetwork_Rua      radioNetwork;
		Cause_Transport	            transport;
		Cause_Protocol	            protocol;
		Cause_Misc	                misc;
	} choice;
}RuaCause;


typedef enum{
	RANAP_Cause_NOTHING_Ranap,	/* No components present */
	RANAP_Cause_radioNetwork_Ranap,
	RANAP_Cause_transmissionNetwork,
	RANAP_Cause_nAS,
	RANAP_Cause_protocol_Ranap,
	RANAP_Cause_misc_Ranap,
	RANAP_Cause_non_Standard,
	/* Extensions may appear below */
	RANAP_Cause_PR_radioNetworkExtension
}RANAP_Cause_Present;


/* Dependencies */
typedef enum {
	RANAP_CauseRadioNetwork_rab_pre_empted	= 1,
	RANAP_CauseRadioNetwork_trelocoverall_expiry	= 2,
	RANAP_CauseRadioNetwork_trelocprep_expiry	= 3,
	RANAP_CauseRadioNetwork_treloccomplete_expiry	= 4,
	RANAP_CauseRadioNetwork_tqueing_expiry	= 5,
	RANAP_CauseRadioNetwork_relocation_triggered	= 6,
	RANAP_CauseRadioNetwork_trellocalloc_expiry	= 7,
	RANAP_CauseRadioNetwork_unable_to_establish_during_relocation	= 8,
	RANAP_CauseRadioNetwork_unknown_target_rnc	= 9,
	RANAP_CauseRadioNetwork_relocation_cancelled	= 10,
	RANAP_CauseRadioNetwork_successful_relocation	= 11,
	RANAP_CauseRadioNetwork_requested_ciphering_and_or_integrity_protection_algorithms_not_supported	= 12,
	RANAP_CauseRadioNetwork_conflict_with_already_existing_integrity_protection_and_or_ciphering_information	= 13,
	RANAP_CauseRadioNetwork_failure_in_the_radio_interface_procedure	= 14,
	RANAP_CauseRadioNetwork_release_due_to_utran_generated_reason	= 15,
	RANAP_CauseRadioNetwork_user_inactivity	= 16,
	RANAP_CauseRadioNetwork_time_critical_relocation	= 17,
	RANAP_CauseRadioNetwork_requested_traffic_class_not_available	= 18,
	RANAP_CauseRadioNetwork_invalid_rab_parameters_value	= 19,
	RANAP_CauseRadioNetwork_requested_maximum_bit_rate_not_available	= 20,
	RANAP_CauseRadioNetwork_requested_guaranteed_bit_rate_not_available	= 21,
	RANAP_CauseRadioNetwork_requested_transfer_delay_not_achievable	= 22,
	RANAP_CauseRadioNetwork_invalid_rab_parameters_combination	= 23,
	RANAP_CauseRadioNetwork_condition_violation_for_sdu_parameters	= 24,
	RANAP_CauseRadioNetwork_condition_violation_for_traffic_handling_priority	= 25,
	RANAP_CauseRadioNetwork_condition_violation_for_guaranteed_bit_rate	= 26,
	RANAP_CauseRadioNetwork_user_plane_versions_not_supported	= 27,
	RANAP_CauseRadioNetwork_iu_up_failure	= 28,
	RANAP_CauseRadioNetwork_relocation_failure_in_target_CN_RNC_or_target_system	= 29,
	RANAP_CauseRadioNetwork_invalid_RAB_ID	= 30,
	RANAP_CauseRadioNetwork_no_remaining_rab	= 31,
	RANAP_CauseRadioNetwork_interaction_with_other_procedure	= 32,
	RANAP_CauseRadioNetwork_requested_maximum_bit_rate_for_dl_not_available	= 33,
	RANAP_CauseRadioNetwork_requested_maximum_bit_rate_for_ul_not_available	= 34,
	RANAP_CauseRadioNetwork_requested_guaranteed_bit_rate_for_dl_not_available	= 35,
	RANAP_CauseRadioNetwork_requested_guaranteed_bit_rate_for_ul_not_available	= 36,
	RANAP_CauseRadioNetwork_repeated_integrity_checking_failure	= 37,
	RANAP_CauseRadioNetwork_requested_request_type_not_supported	= 38,
	RANAP_CauseRadioNetwork_request_superseded	= 39,
	RANAP_CauseRadioNetwork_release_due_to_UE_generated_signalling_connection_release	= 40,
	RANAP_CauseRadioNetwork_resource_optimisation_relocation	= 41,
	RANAP_CauseRadioNetwork_requested_information_not_available	= 42,
	RANAP_CauseRadioNetwork_relocation_desirable_for_radio_reasons	= 43,
	RANAP_CauseRadioNetwork_relocation_not_supported_in_target_RNC_or_target_system	= 44,
	RANAP_CauseRadioNetwork_directed_retry	= 45,
	RANAP_CauseRadioNetwork_radio_connection_with_UE_Lost	= 46,
	RANAP_CauseRadioNetwork_rNC_unable_to_establish_all_RFCs	= 47,
	RANAP_CauseRadioNetwork_deciphering_keys_not_available	= 48,
	RANAP_CauseRadioNetwork_dedicated_assistance_data_not_available	= 49,
	RANAP_CauseRadioNetwork_relocation_target_not_allowed	= 50,
	RANAP_CauseRadioNetwork_location_reporting_congestion	= 51,
	RANAP_CauseRadioNetwork_reduce_load_in_serving_cell	= 52,
	RANAP_CauseRadioNetwork_no_radio_resources_available_in_target_cell	= 53,
	RANAP_CauseRadioNetwork_gERAN_Iumode_failure	= 54,
	RANAP_CauseRadioNetwork_access_restricted_due_to_shared_networks	= 55,
	RANAP_CauseRadioNetwork_incoming_relocation_not_supported_due_to_PUESBINE_feature	= 56,
	RANAP_CauseRadioNetwork_traffic_load_in_the_target_cell_higher_than_in_the_source_cell	= 57,
	RANAP_CauseRadioNetwork_mBMS_no_multicast_service_for_this_UE	= 58,
	RANAP_CauseRadioNetwork_mBMS_unknown_UE_ID	= 59,
	RANAP_CauseRadioNetwork_successful_MBMS_session_start_no_data_bearer_necessary	= 60,
	RANAP_CauseRadioNetwork_mBMS_superseded_due_to_NNSF	= 61,
	RANAP_CauseRadioNetwork_mBMS_UE_linking_already_done	= 62,
	RANAP_CauseRadioNetwork_mBMS_UE_de_linking_failure_no_existing_UE_linking	= 63,
	RANAP_CauseRadioNetwork_tMGI_unknown	= 64
} Ranap_Cause_RadioNetwork;

/* Dependencies */
typedef enum {
	RANAP_CauseTransmissionNetwork_signalling_transport_resource_failure	= 65,
	RANAP_CauseTransmissionNetwork_iu_transport_connection_failed_to_establish	= 66
} Ranap_Cause_TransmissionNetwork;

/* Dependencies */
typedef enum {
	RANAP_CauseNAS_user_restriction_start_indication	= 81,
	RANAP_CauseNAS_user_restriction_end_indication	= 82,
	RANAP_CauseNAS_normal_release	= 83,
	RANAP_CauseNAS_csg_subscription_expiry	= 84
} Ranap_Cause_NAS;

/* Dependencies */
typedef enum {
	RANAP_CauseProtocol_transfer_syntax_error	= 97,
	RANAP_CauseProtocol_semantic_error	= 98,
	RANAP_CauseProtocol_message_not_compatible_with_receiver_state	= 99,
	RANAP_CauseProtocol_abstract_syntax_error_reject	= 100,
	RANAP_CauseProtocol_abstract_syntax_error_ignore_and_notify	= 101,
	RANAP_CauseProtocol_abstract_syntax_error_falsely_constructed_message	= 102
} Ranap_Cause_Protocol;

/* Dependencies */
typedef enum {
	RANAP_CauseMisc_om_intervention	= 113,
	RANAP_CauseMisc_no_resource_available	= 114,
	RANAP_CauseMisc_unspecified_failure	= 115,
	RANAP_CauseMisc_network_optimisation	= 116
} Ranap_Cause_Misc;

typedef long	 Ranap_Cause_Non_Standard;

/* Dependencies */
typedef enum {
	RANAP_CauseRadioNetworkExtension_iP_multicast_address_and_APN_not_valid	= 257,
	RANAP_CauseRadioNetworkExtension_mBMS_de_registration_rejected_due_to_implicit_registration	= 258,
	RANAP_CauseRadioNetworkExtension_mBMS_request_superseded	= 259,
	RANAP_CauseRadioNetworkExtension_mBMS_de_registration_during_session_not_allowed	= 260,
	RANAP_CauseRadioNetworkExtension_mBMS_no_data_bearer_necessary	= 261,
	RANAP_CauseRadioNetworkExtension_periodicLocationInformationNotAvailable	= 262,
	RANAP_CauseRadioNetworkExtension_gTP_Resources_Unavailable	= 263,
	RANAP_CauseRadioNetworkExtension_tMGI_inUse_overlapping_MBMS_service_area	= 264,
	RANAP_CauseRadioNetworkExtension_mBMS_no_cell_in_MBMS_service_area	= 265,
	RANAP_CauseRadioNetworkExtension_no_Iu_CS_UP_relocation	= 266,
	RANAP_CauseRadioNetworkExtension_successful_MBMS_Session_Start_IP_Multicast_Bearer_established	= 267,
	RANAP_CauseRadioNetworkExtension_cS_fallback_triggered	= 268,
	RANAP_CauseRadioNetworkExtension_invalid_CSG_Id	= 269
} Ranap_Cause_RadioNetworkExtension;


typedef struct{
    RANAP_Cause_Present present;
	union{
		Ranap_Cause_RadioNetwork	 radioNetwork;
		Ranap_Cause_TransmissionNetwork	 transmissionNetwork;
		Ranap_Cause_NAS	 nAS;
		Ranap_Cause_Protocol	 protocol;
		Ranap_Cause_Misc	 misc;
		Ranap_Cause_Non_Standard	 non_Standard;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
		Ranap_Cause_RadioNetworkExtension	 radioNetworkExtension;
	} choice;
}RANAPCause;




//struct for UE
typedef  struct  IuhUE
{
	uint32_t	UEID;	//ue  index
	UE_IDENTITY_S  UE_Identity;
	struct UECapability	  Capabilities;
	RegCause registrationCause;    //0 emergency_call, 1 normal
	uint32_t  context_id;	// BIT STRING (SIZE(24))
	unsigned char context_id_str[CONTEXTID_LEN]; // BIT STRING (SIZE(24))
	CsgState csgMemStatus;  //0 yes,1 no
	REGISTSTATE  	state;
	uint32_t  	HNBID;
	//uint8_t*	UEMAC;					//UE MAC地址
	//char*	UEIP;
	HNBAPCause cause;
	unsigned char IMSI[IMSI_LEN];   // 15 numbers
	uint8_t rab_count;      //rab counts of current ue
	unsigned char iuCSSigConId[IU_SIG_CONN_ID_LEN];		//iu-cs signaling connection id. book add, 2011-12-16
	unsigned char iuPSSigConId[IU_SIG_CONN_ID_LEN];		//iu-ps signaling connection id. book add, 2011-12-16
	unsigned char iuDefSigConId[IU_SIG_CONN_ID_LEN];

    Iu_RAB *UE_RAB;         // rab informations of current  ue
	
	struct IuhUE *ue_next;
	
}Iuh_HNB_UE;

struct  utranCellID
{
	unsigned char        lAC[LAC_LEN];
	unsigned char        rAC[RAC_LEN];
	unsigned char        pLMNidentity[PLMN_LEN];//size 24
	unsigned char        cellIdentity[CELLID_LEN]; //size 28
};

struct  cgi
{
	unsigned char  pLMNidentity[PLMN_LEN]; //size 24
	unsigned char  lAC[LAC_LEN];
	unsigned char  cI[CI_LEN];
};

//reference  from  9.2.5
struct  sGeographicalCoordinates
{
	uint32_t  latitude;		// 0..8388607
	int  longitude;			//-8388608..8388607
	int  LatitudeSign;   //0 north, 1 south
};

//reference  from  9.2.6
struct  sAltitudeAndDirection
{
	int  DirectionOfAltitude;   //0 height, 1 depth
	uint16_t  altitude;		//0..32767
};

struct  geographicalLocation
{
	struct  sGeographicalCoordinates  GeographicalCoordinates;
	struct  sAltitudeAndDirection  AltitudeAndDirection;
};

typedef enum{
    IPV4,
    IPV6
}IPAddr_PR;

//reference  from  9.2.8
struct  IPAddress 
{
    IPAddr_PR present;
	
	union{
	    unsigned char  ipv4Address[IPV4_LEN];
	    unsigned char  ipv6Address[IPV6_LEN];
	}choice;
};

typedef enum{
    PR_NOTHING,
    uTRANCellID,
    gERANCellID
}Macro_PR;

struct macroCellID{
    Macro_PR present;
    
    union{
	    struct  utranCellID uTRANCellID; 
	    struct  cgi  gERANCellID;
	}choice;
};

typedef  struct  hnb_LocationInfomation{
	struct  macroCellID            *macroCoverageInfo;
	struct  geographicalLocation    *gographicalLocation;
	struct  IPAddress			    *ipv4info;
}HNBLocationInformation;


typedef enum
{
	ACCESS_OPEN,
	ACCESS_CLOSED,
	ACCESS_HYBIRD
}ACCESSMODE;
/*struct for imsi white list*/
typedef struct IMSIWHITELIST
{
	unsigned char imsi[IMSI_LEN];
	struct IMSIWHITELIST *next;
}IMSIWHITELIST;
/* struct  for  hnbs */
typedef  struct  IuhHnb
{
	IuhNetworkLev4Address address;	//HNB的地址
	uint32_t BindingSystemIndex;	//绑定的接口索引
	IuhSocket socket;				//建立连接的socket
	
	uint32_t	HNBID;				//HNB index in HNB-GW 
	unsigned char HnbIdentity[HNB_IDENTITY_LEN];			//HNB identity information   OCTET STRING(1..255) see 9.2.2
	//char* 	    HNBSN; 					//HNB Serial No
	unsigned char HNBNAME[HNB_NAME_LEN];				//HNB 名称
	//unsigned char HNBModel[HNB_MODEL_LEN];				//HNB 型号
	//unsigned char HNBMAC[MAC_LEN];		//HNB MAC地址
	unsigned char BindingIFName[IF_NAME_LEN];//绑定接口名

	unsigned char HNBIP[HNB_IP_LEN];
	
	int current_UE_number;
	Iuh_HNB_UE *HNB_UE;	//HNB所管理的UE结构
	
	uint8_t 	isused;//no use
	int     acl_switch;     //book add, 2012-1-4
	
	HNBLocationInformation		HnbLocationInfo;	//地址信息
	unsigned char	    lac[LAC_LEN];			//位置区域码
	unsigned char	    rac[RAC_LEN];			//路由区域吗
	unsigned char	    sac[SAC_LEN];			//服务区域码
	unsigned char	    plmnId[PLMN_LEN];		//Public Land Mobile Network ID   size 24
	unsigned char	    cellId[CELLID_LEN];		//标识PLMN中的唯一Cell  size 28
	unsigned char	    csgId[CSGID_LEN];		//封闭用户组ID   size 27
	uint16_t	broad_sac;	//Service Area For Broadcast
					//支持的CSG访问类型
    uint16_t	rncId;			//标识IU接口 0..65535
    uint32_t	muxPortNum;	    //多路复用端口号
    
    ACCESSMODE accessmode;
    REGISTSTATE state;

    HNBAPCause cause;

	IMSIWHITELIST *imsi_white_list;	//xiaodawei add for femto acl, 20111229
	unsigned char IMSI_WL_Switch;	//xiaodawei add for femot acl white list Switch
}Iuh_HNB;


/* structures of  Iuh2Iu ---begin--- */
typedef enum{
    RESERVED = 0,
    CONNECTIONLESS_TO_CN = 1,
    CONNECTION_TO_CN = 2,
    CONNECTIONLESS_FROM_CN = 3,
    CONNECTION_FROM_CN = 4,
    UE_CONTROL_TO_IU = 5
}SigMsgType;

typedef enum{
    Rua_Connect = 1,
	Rua_DirectTransfer = 2,
	Rua_Disconnect = 3,
	Rua_ConnectionlessTransfer = 4,
	Rua_ErrorIndicator = 5,
	Rua_privateMessage = 6
}RuaType;

/* Dependencies */
typedef enum{
	RUA_PR_NOTHING,	/* No components present */
	RUA_PR_initiatingMessage,
	RUA_PR_successfulOutcome,
	RUA_PR_unsuccessfulOutcome
	/* Extensions may appear below */
} MSG_TYPE;

typedef struct rua_msg_type{
    RuaType ruaType;
    MSG_TYPE msgType;
}RuaMsgType;


typedef enum{
    Pr_Unknown = 0,
    Pr_IMSI = 1,
    Pr_IMEI = 2,
    Pr_IMEISV = 3
}SigUePr;

typedef enum{
    CS_Domain = 0,
    PS_Domain = 1
}CNDomain;

typedef struct sigueid{
    SigUePr present;
    union{
        unsigned char IMSI[IMSI_LEN];
        unsigned char IMEI[IMSI_LEN];
        unsigned char IMEISV[IMSI_LEN];
    }choice;
}SigUEID;
// __attribute__ ((packed))

typedef enum{
	Rua_Cause_NOTHING,	// No components present 
	Rua_Cause_radioNetwork,
	Rua_Cause_transport,
	Rua_Cause_protocol,
	Rua_Cause_misc
}RuaCausePresent;

typedef enum{
    Normal,
    Connect_failed,
    Network_release,
    Network_Unspecified    
}RuaCauseRadioNetwork;

typedef enum{  
    Transport_Resource_Unavailable,
    Transport_Unspecified
}RuaCauseTransport;

typedef enum{
    Transfer_syntax_error	= 0,
	Abstract_syntax_error_reject	= 1,
	Abstract_syntax_error_ignore_and_notify	= 2,
	Message_not_compatible_with_receiver_state	= 3,
	Semantic_error	= 4,
	Unspecified	= 5,
	Abstract_syntax_error_falsely_constructed_message	= 6
}RuaCauseProtocol;

typedef enum{
    Processing_overload	= 0,
	Hardware_failure	= 1,
	O_and_m_intervention	= 2,
	Misc_Unspecified	= 3
}RuaCauseMisc;

/*
typedef struct{
    uint8_t present;
	uint8_t causeValue;
}RuaCause;
*/

typedef RegCause EstablishmentCause;

typedef struct ranapMsg{
    uint16_t size;
    char RanapMsg[RANAP_LEN];
}RuaRanapMsg;

typedef struct rnm_plmn{
    unsigned char PlmnId[PLMN_LEN];
    uint16_t RncId;
}RNCPLMN;
//__attribute__ ((packed))

/* book add, 2011-12-21 */
typedef enum {
	UE_ADD = 0,
	UE_UPDATE = 1,
	UE_DEL = 2
}eUEOP;

typedef struct Iuh2Iu{
    SigMsgType SigType;
    SigUEID UeIdentity;
    RNCPLMN RncPlmn;
    RuaMsgType RuaType;
    CNDomain CnDomain;
    RuaCause cause;
    EstablishmentCause EstabCause;
	unsigned char contextid[CONTEXTID_LEN];
	unsigned short ranap_type;
    RuaRanapMsg RanapMsg;
    unsigned char imsi[IMSI_LEN];
    IUH_LAI lai;
	uint32_t UEID;
	eUEOP ueop;
}Iuh2IuMsg;
//__attribute__ ((packed))
/* structures of  Iuh2Iu ---end--- */


struct g_switch{
    uint32_t paging_imsi;           //0 close, 1 open
    uint32_t paging_lai;            //0 close, 1 open 
};


extern Iuh_HNB **IUH_HNB;
extern Iuh_HNB_UE **IUH_HNB_UE;
extern Iu_RAB **IU_RAB;
extern struct ifi *IUH_IF;
extern int vrrid;
extern int local;
extern int slotid;
extern int sockPerThread[SOCK_NUM];
extern char MSGQ_PATH[PATH_LEN];
extern unixAddr toIU;
extern int IuSocket;
extern int IuhMsgQid;
extern int IuhAllQid;
extern IuhMultiHomedSocket gIuhSocket;
extern int HNB_NUM;
extern int UE_NUM;
extern int glicensecount;
extern int gStaticHNBs;
extern int gStaticUEs;
extern unsigned short int gRncId;
extern int iu_enable;
extern int msc_enable;
extern int sgsn_enable;
extern struct g_switch gSwitch;
extern int IuTipcSocket;
extern struct sockaddr_tipc Iuh2Iu_tipcaddr;
extern int TipcsockPerThread[SOCK_NUM];
extern int gIuhTipcSockFlag;

void InitFemtoPath(unsigned int vrrid,char *buf);
void Iuh_DbusPath_Init();
IuhBool IuhGetMsgQueue(int *msgqid);

#endif
