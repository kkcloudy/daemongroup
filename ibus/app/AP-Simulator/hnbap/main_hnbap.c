#ifdef	HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>	/* for atoi(3) */
#include <unistd.h>	/* for getopt(3) */
#include <string.h>	/* for strerror(3) */
#include <sysexits.h>	/* for EX_* exit codes */
#include <errno.h>	/* for errno */
#include <asn_application.h>
#include <asn_internal.h>	/* for _ASN_DEFAULT_STACK_MAX */
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h> 



#include "HNBAP-PDU.h"
#include "HNBRegisterAccept.h"
#include "ProtocolIE-Container.h"
#include "RNC-ID.h"
#include "BIT_STRING.h"
#include "HNB-Identity.h"
#include "HNB-Location-Information.h"
#include "HNBRegisterRequest.h"
#include "Ipv4Address.h"
#include "IP-Address.h"
#include "LAC.h"
#include "RAC.h"
#include "SAC.h"
#include "CellIdentity.h"
#include "PLMNidentity.h"
#include "ProtocolExtensionContainer.h"
#include "CSG-ID.h"
#include "HNBRegisterReject.h"
#include "Cause.h"
#include "CriticalityDiagnostics.h"
#include "Context-ID.h"
#include "UE-Capabilities.h"
#include "UEDe-Register.h"
#include "UE-Identity.h"
#include "UERegisterAccept.h"
#include "Registration-Cause.h"
#include "BackoffTimer.h"
#include "HNB-Cell-Access-Mode.h"
#include "MuxPortNumber.h"
#include "CSGMembershipStatus.h"
#include "HNBDe-Register.h"
#include "UERegisterRequest.h"
#include "UEDe-Register.h"
#include "UERegisterReject.h"

#define	ASN_DEF_PDU(t)	asn_DEF_ ## t
#define	DEF_PDU_Type(t)	ASN_DEF_PDU(t)
//#define	PDU_Type	DEF_PDU_Type(HNBAP_PDU)


#define HNBAP_PPID (20)
#define RUA_PPID (19)
#define M3UA_PPID (3)

#define PDU_BUFFER_SIZE (512)
#define IE_BUFFER_SIZE (512)

typedef enum ProcedureCode_ID_t_{
	HNBRegister = 1,
	HNBDeRegister = 2,
	UERegister = 3,
	UEDeRegister = 4,
	ErrorIndicate = 5,
	privateMessage = 6,
	CSGMemberUpdate = 7,
	/**/
	Error_procdure_code,
}ProcedureCode_ID_t;

typedef enum IE_ID_t_{
	id_Cause = 1,
	CriticalityDiagnosis = 2,
	HNB_Ident = 3,
	Context_ID = 4,
	UE_Id = 5,
	LAC = 6,
	RAC = 7,
	HNB_Location_Info = 8,
	PLMNid = 9,
	SAC = 10,
	CellId = 11,
	Register_Cause = 12,
	UE_Capability = 13,
	RNC_ID = 14,
	CSG_ID = 15,
	BackoffTimer = 16,
	HNB_Internet_Information = 17,
	Cell_Access_Mode = 18,
	MuxPortNumber = 19,
	SA_For_Broadcast = 20,
	CSGMemberStatus = 21,
	/*...*/
	MAX_MEMBER_ID,
}IE_ID_t;

typedef int (add_func_t)(ProtocolIE_Container_110P0_t *, e_Criticality ,	unsigned char* , int );

typedef struct IE_Value{
	e_Criticality Criticality;
	char* ie_buff;
	unsigned int buff_size;
	add_func_t *add_func;
}IE_Value_t;

typedef struct my_Choice{
	long present;
	void *value;
}my_Choice_t;

typedef char* my_iMSI_t;

typedef struct my_LAI{
	char *pLMNID;
	char *lAC;
}my_LAI_t;

typedef struct my_tMSI{
	char *tMSI;
	my_LAI_t lAI;
}my_tMSI_t;

typedef struct my_RAI{
	my_LAI_t lAI;
	char *rac;
}my_RAI_t;

typedef struct my_pTMSI{
	char *pTMSI;
	my_RAI_t rai;	
}my_pTMSI_t;

typedef char* my_IMEI_t;

typedef char* my_ESN_t;

typedef char* my_IMSIDS41_t;

typedef struct my_IMSIESN{
	char *iMSIDS41;
	char *eSN;
}my_IMSIESN_t;

typedef char* my_tMSIDS41_t;

typedef struct my_UE_Capabilities{
	char*	 asr_indicator;
	char*	 csg_indicator;
}my_UE_Capabilities_t;


IE_Value_t *ie_value_list[MAX_MEMBER_ID-1];
ProcedureCode_ID_t procedure_code;
Context_ID_t *my_context_id=NULL;

void skip_space(char *str)
{
	char *p;

	p=str;
	while(*p != '\0')
	{
		if(*p == ' ' || *p == '\t' || *p == '\n')
			memmove(p,p+1,strlen(p+1)+1);
		else
		  p++;
	}
}

void skip_char(char *str, char a)
{
	char *p;

	p=str;
	while(*p != '\0')
	{
		if(*p == a)
			memmove(p,p+1,strlen(p));
		else
		  p++;
	}
}

void CWCaptrue(int n ,unsigned char *buffer){
		int t=0;
		while((n-t)>=16)
		{
			int i;
			printf("[");
			for(i=0;i<16;i++)
				printf("%02x ",buffer[t+i]);
			printf("]\t[");
			for(i=0;i<16;i++)
			{
				char ch=buffer[t+i];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
			t+=16;
		}

		if(n>t)
		{
			int i=t;
			printf("[");
			while(i<n)
				printf("%02x ",buffer[i++]);
			printf("]");
			i=n-t;
			i=16-i;
			while(i--)
				printf("   ");
			printf("\t[");
			i=t;
			while(i<n)
			{
				char ch=buffer[i++];
				if(isalnum(ch))
					printf("%c",ch);
				else
					printf(".");
			}
			printf("]\n");
		}
		printf("\n\n");
}


int init_sock(struct sockaddr_in *dest_addr,int *sock)
{
	struct sctp_event_subscribe events;

	bzero(&events, sizeof (events));
	events.sctp_data_io_event = 1;
	events.sctp_association_event = 1;
	events.sctp_address_event = 1;
	events.sctp_send_failure_event = 1;
	events.sctp_peer_error_event = 1;
	events.sctp_shutdown_event = 1;
	
	if ((*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) ==-1)
	{ 
		return (-1);	
	}

	if(setsockopt(*sock, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0) 
	{
		return(-1);
	}

	while(connect(*sock, (struct sockaddr *)dest_addr, sizeof(struct sockaddr)) == -1)
	{
		sleep(1);
		continue;
	}

	return 0;
}


unsigned int Add_ProtocolIE_Container(ProtocolIE_Container_110P0_t *ie_container,
																						IE_ID_t Member_id,
																						e_Criticality Criticality,
																						char* ie_buff,
																						unsigned int buff_size)
{
		INTEGER_t integer;
		Member_ProtocolIE_Container_t *member_rnc_id;

		member_rnc_id = (Member_ProtocolIE_Container_t *)calloc(1,sizeof(Member_ProtocolIE_Container_t));
		
		member_rnc_id->id = Member_id;
		memset(&integer,0,sizeof(INTEGER_t));
		asn_ulong2INTEGER(&integer, Criticality);
		member_rnc_id->criticality = (Criticality_t)integer;
		ANY_fromBuf(((OCTET_STRING_t *)&(member_rnc_id->value)), ie_buff, buff_size);
	//++++create member_id end
		ASN_SEQUENCE_ADD(&(ie_container->list), member_rnc_id);

		return 0;
}

int add_hnb_identity(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
		HNB_Identity_t hnb_id;
		unsigned char* uchnb_id;
		asn_enc_rval_t ret;
		unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

		//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
		memset(&hnb_id,0,sizeof(HNB_Identity_t));
		uchnb_id = value;
		OCTET_STRING_fromBuf(&hnb_id.hNB_Identity_Info, uchnb_id, 1);
	
		ret = uper_encode_to_buffer(&asn_DEF_HNB_Identity,(void *)&hnb_id, ie_buff, IE_BUFFER_SIZE);
		if(ret.encoded == -1)
		{
			fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
			return (-1);
		}
		Add_ProtocolIE_Container(ie_container, HNB_Ident, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_hnb_location_info(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	HNB_Location_Information_t hnb_location_info;
	ProtocolIE_Container_110P0_t *ie_exten_container;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&hnb_location_info,0,sizeof(HNB_Location_Information_t));
	ie_exten_container = calloc(1,sizeof(ProtocolIE_Container_110P0_t));
	hnb_location_info.iE_Extensions = (ProtocolExtensionContainer_144P0_t *)ie_exten_container;
	{
		IP_Address_t ip_addr;
		unsigned int ipv4_addr;
		memset(&ip_addr,0,sizeof(IP_Address_t));

		ip_addr.ipaddress.present = ipaddress_PR_ipv4info;
		ipv4_addr = inet_addr(value);
		OCTET_STRING_fromBuf(&(ip_addr.ipaddress.choice.ipv4info), (char*)&ipv4_addr, sizeof(ipv4_addr));
		ret = uper_encode_to_buffer(&asn_DEF_IP_Address,(void *)&ip_addr, ie_buff, IE_BUFFER_SIZE);
		if(ret.encoded == -1)
		{
			fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
			return (-1);
		}
		Add_ProtocolIE_Container(ie_exten_container, HNB_Internet_Information, Criticality_ignore, ie_buff, (ret.encoded+7)/8);
	}

	memset(ie_buff,0,IE_BUFFER_SIZE);
	ret = uper_encode_to_buffer(&asn_DEF_HNB_Location_Information,(void *)&hnb_location_info, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, HNB_Location_Info, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_plmn_id(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	PLMNidentity_t plmn_id;
	unsigned char *ucplmn_id;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
	
	ucplmn_id = value;
	
	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&plmn_id,0,sizeof(plmn_id));
	OCTET_STRING_fromBuf(&plmn_id, ucplmn_id, 3);

	ret = uper_encode_to_buffer(&asn_DEF_PLMNidentity,(void *)&plmn_id, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, PLMNid, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_cell_id(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	CellIdentity_t cell_id;
	unsigned char* uccell_id;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&cell_id, 0, sizeof(CellIdentity_t));
	uccell_id  = value;
	BIT_STRING_fromBuf(&cell_id, uccell_id, 4, 4);

	ret = uper_encode_to_buffer(&asn_DEF_CellIdentity,(void *)&cell_id, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, CellId, Criticality, ie_buff, (ret.encoded+7)/8);
}
int add_lac(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	LAC_t lac;
	unsigned char* uclac;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
	
	uclac = value;
	
	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&lac,0,sizeof(lac));
	OCTET_STRING_fromBuf(&lac, uclac, 2);

	ret = uper_encode_to_buffer(&asn_DEF_LAC,(void *)&lac, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, LAC, Criticality, ie_buff, (ret.encoded+7)/8);
}
int add_rac(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	RAC_t rac;
	unsigned char* ucrac;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	ucrac = value;
	
	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&rac,0,sizeof(rac));
	OCTET_STRING_fromBuf(&rac, ucrac, 1);

	ret = uper_encode_to_buffer(&asn_DEF_RAC,(void *)&rac, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, RAC, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_sac(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	SAC_t sac;
	unsigned char *ucsac;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};
	
	ucsac = value;

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&sac,0,sizeof(sac));
	OCTET_STRING_fromBuf(&sac, ucsac, 2);

	ret = uper_encode_to_buffer(&asn_DEF_SAC,(void *)&sac, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, SAC, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_csg_id(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	CSG_ID_t csgid;
	unsigned char* uccsg_id;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	uccsg_id	 = value;

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&csgid, 0, sizeof(CSG_ID_t));
	BIT_STRING_fromBuf(&csgid, uccsg_id, 4, 5);
	ret = uper_encode_to_buffer(&asn_DEF_CSG_ID,(void *)&csgid, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, CSG_ID, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_case(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	Cause_t cause;
	my_Choice_t *choice = NULL;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&cause, 0, sizeof(CSG_ID_t));
	choice = (my_Choice_t *)value;
	
	cause.present = choice->present;

	switch(choice->present)
	{
		case Cause_PR_radioNetwork:
			asn_long2INTEGER(&cause.choice.radioNetwork, *((long*)choice->value));
			break;
		case Cause_PR_transport:
			asn_long2INTEGER(&cause.choice.transport, *((long*)choice->value));
			break;
		case Cause_PR_protocol:
			asn_long2INTEGER(&cause.choice.protocol, *((long*)choice->value));
			break;
		case Cause_PR_misc:
			asn_long2INTEGER(&cause.choice.misc, *((long*)choice->value));
			break;
		default:
			break;
	}
	ret = uper_encode_to_buffer(&asn_DEF_Cause,(void *)&cause, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, id_Cause, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_backofftimer(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	BackoffTimer_t timer;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&timer, 0, sizeof(BackoffTimer_t));
	timer	 = atol(value);
	ret = uper_encode_to_buffer(&asn_DEF_BackoffTimer,(void *)&timer, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, BackoffTimer, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_ueid(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	UE_Identity_t ueid;
	my_Choice_t *spueid;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	my_iMSI_t *imsi;
	my_tMSI_t *tmsi;
	my_pTMSI_t *ptmsi;
	my_IMEI_t *imei;
	my_ESN_t *eSN;
	my_IMSIDS41_t *iMSIDS41;
	my_IMSIESN_t *iMSIESN;
	my_tMSIDS41_t *tMSIDS41;

	spueid = (my_Choice_t *)value;
	memset(&ueid, 0, sizeof(UE_Identity_t));

	ueid.present = spueid->present;
	switch(ueid.present)
	{
		case UE_Identity_PR_iMSI:
			imsi = spueid->value;
			OCTET_STRING_fromBuf(&ueid.choice.iMSI, (char*)imsi, strlen((char*)imsi));
			break;
		case UE_Identity_PR_tMSILAI:
			tmsi = spueid->value;
			BIT_STRING_fromBuf(&ueid.choice.tMSILAI.tMSI,tmsi->tMSI,4,0);
			OCTET_STRING_fromBuf(&ueid.choice.tMSILAI.lAI.pLMNID, tmsi->lAI.pLMNID, strlen(tmsi->lAI.pLMNID));
			OCTET_STRING_fromBuf(&ueid.choice.tMSILAI.lAI.lAC, tmsi->lAI.lAC, strlen(tmsi->lAI.lAC));
			break;
		case UE_Identity_PR_pTMSIRAI:
			ptmsi = spueid->value;
			BIT_STRING_fromBuf(&ueid.choice.pTMSIRAI.pTMSI,ptmsi->pTMSI,4,0);
			OCTET_STRING_fromBuf(&ueid.choice.pTMSIRAI.rAI.lAI.pLMNID, ptmsi->rai.lAI.pLMNID, strlen(ptmsi->rai.lAI.pLMNID));
			OCTET_STRING_fromBuf(&ueid.choice.pTMSIRAI.rAI.lAI.lAC, ptmsi->rai.lAI.lAC, strlen(ptmsi->rai.lAI.lAC));
			OCTET_STRING_fromBuf(&ueid.choice.pTMSIRAI.rAI.rAC, ptmsi->rai.rac, strlen(ptmsi->rai.rac));
			break;
		case UE_Identity_PR_iMEI:
			imei = spueid->value;
			BIT_STRING_fromBuf(&ueid.choice.iMEI, imei, 8, 4);
			break;
		case UE_Identity_PR_eSN:
			eSN = spueid->value;
			BIT_STRING_fromBuf(&ueid.choice.eSN, eSN, 4, 0);
			break;
		case UE_Identity_PR_iMSIDS41:
			iMSIDS41 = spueid->value;
			OCTET_STRING_fromBuf(&ueid.choice.iMSIDS41, (char*)iMSIDS41, strlen((char*)iMSIDS41));
			break;
		case UE_Identity_PR_iMSIESN:
			iMSIESN = spueid->value;
			OCTET_STRING_fromBuf(&ueid.choice.iMSIESN.iMSIDS41, iMSIESN->iMSIDS41, strlen(iMSIESN->iMSIDS41));
			BIT_STRING_fromBuf(&ueid.choice.iMSIESN.eSN,iMSIESN->eSN,4,0);
			break;
		case UE_Identity_PR_tMSIDS41:
			tMSIDS41 = spueid->value;
			OCTET_STRING_fromBuf(&ueid.choice.tMSIDS41, (char*)tMSIDS41, strlen((char*)tMSIDS41));
			break;
		default:
			break;
	}
	ret = uper_encode_to_buffer(&asn_DEF_UE_Identity,(void *)&ueid, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, UE_Id, Criticality, ie_buff, (ret.encoded+7)/8);

}

int add_reg_cause(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	Registration_Cause_t reg_cause;
	long tmp_cause;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	fprintf(stderr,"%s(%d\n",__func__,__LINE__);
	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	memset(&reg_cause, 0, sizeof(Registration_Cause_t));

	tmp_cause = atol(value);
	asn_long2INTEGER((INTEGER_t *)&reg_cause, tmp_cause);
	ret = uper_encode_to_buffer(&asn_DEF_Registration_Cause,(void *)&reg_cause, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, Register_Cause, Criticality, ie_buff, (ret.encoded+7)/8);
}

int add_ue_capability(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	UE_Capabilities_t ue_capa;
	my_UE_Capabilities_t *my_ue_capa;
	long tmp_long;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	fprintf(stderr,"%s(%d\n",__func__,__LINE__);

	memset(&ue_capa, 0, sizeof(UE_Capabilities_t));

	my_ue_capa = (my_UE_Capabilities_t *)value;
	tmp_long = atol(my_ue_capa->asr_indicator);
	asn_long2INTEGER((INTEGER_t *)&ue_capa.access_stratum_release_indicator, tmp_long);
	tmp_long = atol(my_ue_capa->csg_indicator);
	asn_long2INTEGER((INTEGER_t *)&ue_capa.csg_indicator, tmp_long);
	ret = uper_encode_to_buffer(&asn_DEF_UE_Capabilities,(void *)&ue_capa, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, UE_Capability, Criticality, ie_buff, (ret.encoded+7)/8);

}


int add_context_id(ProtocolIE_Container_110P0_t *ie_container,
																				e_Criticality Criticality, 
																				unsigned char *value, 
																				int value_len)
{
	Context_ID_t contextid;
	asn_enc_rval_t ret;
	unsigned char ie_buff[IE_BUFFER_SIZE] = {0};

	//fprintf(stderr,"%s(%d):value:: %s\n",__func__,__LINE__,value);
	fprintf(stderr,"%s(%d\n",__func__,__LINE__);

	memset(&contextid, 0, sizeof(Context_ID_t));

	BIT_STRING_fromBuf(&contextid, value, 3, 0);

	if(my_context_id == NULL)
	{
		my_context_id = calloc(1,sizeof(Context_ID_t));
		memcpy(my_context_id,&contextid,sizeof(Context_ID_t));
	}
	ret = uper_encode_to_buffer(&asn_DEF_Context_ID,(void *)my_context_id, ie_buff, IE_BUFFER_SIZE);
	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	Add_ProtocolIE_Container(ie_container, Context_ID, Criticality, ie_buff, (ret.encoded+7)/8);
}

void	init_ue_register_ie_list(FILE *file)
{
	char line_buff[128];
	char *p;
	char *p1;
	my_tMSI_t *tmsi;
	my_pTMSI_t *ptmsi;
	my_IMSIESN_t *iMSIESN;
	my_UE_Capabilities_t *my_ue_capa;
	
	fseek(file,0,SEEK_SET);

	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(!strncasecmp(line_buff,"UE_Register_PDU",strlen("UE_Register_PDU")))
			continue;
		fprintf(stderr,"UE_Register_Request_PDU\n");
		break;
	}
	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(strlen(line_buff) == 0)
			continue;
		if(!strncasecmp(line_buff,"UE_IDENTITY",strlen("UE_IDENTITY")))
		{
			ie_value_list[UE_Id] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[UE_Id]->add_func = add_ueid;
			
			p = strchr(line_buff,':');
			p++;
			ie_value_list[UE_Id]->Criticality = *p - 0x30;
			p = strrchr(line_buff,':'); 
			p++;
			skip_char(p,'}');
			skip_char(p,',');
			if(!strncasecmp(p, "choice",strlen("choice")))
			{
				my_Choice_t choice;
				memset(&choice,0,sizeof(my_Choice_t));
			
				p = strchr(line_buff, '=');
				p++;
				choice.present = atol(p);
				switch(choice.present)
				{
					case UE_Identity_PR_iMSI:
					case UE_Identity_PR_iMEI:
					case UE_Identity_PR_eSN:
					case UE_Identity_PR_iMSIDS41:
					case UE_Identity_PR_tMSIDS41:
						p = strstr(line_buff,"value"); 
						p+=strlen("value=");//skip "value="
						choice.value = calloc(1,strlen(p)+1);
						memcpy(choice.value, p, strlen(p)+1);
						break;
					case UE_Identity_PR_tMSILAI:
						p = strstr(line_buff,"value"); 
						tmsi = calloc(1,sizeof(my_tMSI_t));
						choice.value = tmsi;
						
						p = strstr(p,"tmsi");
						p+=strlen("tmsi=");
						p1 = strstr(p,"lai");
						tmsi->tMSI = calloc(1,p1-p+1);
						memcpy(tmsi->tMSI,p,p1-p);
						p = strstr(p,"plmnid");
						p+=strlen("plmnid=");
						p1 = strstr(p,"lac");
						tmsi->lAI.pLMNID = calloc(1,p1-p);
						memcpy(tmsi->lAI.pLMNID,p,p1-p);
						p1+=strlen("lac=");
						tmsi->lAI.lAC = calloc(1,strlen(p1));
						memcpy(tmsi->lAI.lAC,p1,strlen(p1));
						break;
					case UE_Identity_PR_pTMSIRAI:
						p = strstr(line_buff,"value"); 
						ptmsi = calloc(1,sizeof(my_pTMSI_t));
						choice.value = ptmsi;
						
						p = strstr(p,"ptmsi");
						p+=strlen("ptmsi=");
						p1 = strstr(p,"rai");
						ptmsi->pTMSI = calloc(1,p1-p+1);
						memcpy(ptmsi->pTMSI,p,p1-p);
						p = strstr(p,"plmnid");
						p+=strlen("plmnid=");
						p1 = strstr(p,"lac");
						ptmsi->rai.lAI.pLMNID = calloc(1,p1-p+1);
						memcpy(ptmsi->rai.lAI.pLMNID,p,p1-p);
						p1+=strlen("lac=");
						p=strstr(p1,"rac");
						ptmsi->rai.lAI.lAC = calloc(1,p-p1+1);
						memcpy(ptmsi->rai.lAI.lAC,p1,p-p1);
						p+=strlen("rac=");
						ptmsi->rai.rac = calloc(1,strlen(p));
						memcpy(ptmsi->rai.rac, p,strlen(p));
						break;
					case UE_Identity_PR_iMSIESN:
						p = strstr(line_buff,"value"); 

						iMSIESN = calloc(1,sizeof(my_IMSIESN_t));
						choice.value = iMSIESN;
						
						p = strstr(p,"imsids41");
						p+=strlen("imsids41=");
						p1 = strstr(p,"esn");
						iMSIESN->iMSIDS41 = calloc(1,p1-p+1);
						memcpy(iMSIESN->iMSIDS41,p,p1-p);
						fprintf(stderr,"iMSIESN->iMSIDS41 = %s\n",iMSIESN->iMSIDS41);
						p1+=strlen("esn=");
						iMSIESN->eSN = calloc(1,strlen(p1));
						memcpy(iMSIESN->eSN,p1,strlen(p1));
						fprintf(stderr,"iMSIESN->eSN = %s\n",iMSIESN->eSN);
						break;
					default:
						break;
				}
				ie_value_list[UE_Id]->buff_size = sizeof(my_Choice_t);
				ie_value_list[UE_Id]->ie_buff = calloc(1,sizeof(my_Choice_t));
				memcpy(ie_value_list[UE_Id]->ie_buff,&choice, sizeof(my_Choice_t));
			}
		}
		else if(!strncasecmp(line_buff,"Registration_Cause",strlen("Registration_Cause")))
		{
			ie_value_list[Register_Cause] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[Register_Cause]->add_func = add_reg_cause;
			p = strchr(line_buff,':');
			p++;
			ie_value_list[Register_Cause]->Criticality = *p - 0x30;
			p = strrchr(line_buff,':'); 
			p++;
			skip_char(p,'}');
			skip_char(p,',');
			ie_value_list[Register_Cause]->buff_size = strlen(p)+1;
			ie_value_list[Register_Cause]->ie_buff = calloc(1,strlen(p)+1);
			strcpy(ie_value_list[Register_Cause]->ie_buff,p);
			//fprintf(stderr,"value : %s\n",ie_value_list[Register_Cause]->ie_buff);
		}
		else if(!strncasecmp(line_buff,"UE_Capability",strlen("UE_Capability")))
		{
			ie_value_list[UE_Capability] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[UE_Capability]->add_func = add_ue_capability;

			p = strchr(line_buff,':');
			p++;
			ie_value_list[UE_Capability]->Criticality = *p - 0x30;
			p = strrchr(line_buff,':'); 
			p++;
			skip_char(p,'}');
			skip_char(p,',');
			
			my_ue_capa = calloc(1,sizeof(my_UE_Capabilities_t));
			p = strstr(p,"asr_indicator");
			p+=strlen("asr_indicator=");
			p1 = strstr(p,"csg_indicator");
			my_ue_capa->asr_indicator = calloc(1,p1-p+1);
			memcpy(my_ue_capa->asr_indicator,p,p1-p);
			p1+=strlen("csg_indicator=");
			my_ue_capa->csg_indicator = calloc(1,strlen(p1));
			memcpy(my_ue_capa->csg_indicator, p1, strlen(p1));

			ie_value_list[UE_Capability]->buff_size = sizeof(my_UE_Capabilities_t);
			ie_value_list[UE_Capability]->ie_buff = (char *)my_ue_capa;
		}
		else
		{
			//fprintf(stderr,"%s(%d)\n",__func__,__LINE__);
			continue;
		}
	}
}

void	init_ue_de_register_ie_list(FILE *file)
{
	char line_buff[128];
	char *p;
	int mem_id;
	long tmp_long;
	
	fseek(file,0,SEEK_SET);

	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(!strncasecmp(line_buff,"UE_De_Register_PDU",strlen("UE_De_Register_PDU")))
			continue;
		fprintf(stderr,"UE_De_Register_PDU\n");
		break;
	}
	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(strlen(line_buff) == 0)
			continue;
		if(!strncasecmp(line_buff,"Context_ID",strlen("Context_ID")))
		{
			ie_value_list[id_Cause] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[id_Cause]->add_func = add_context_id;
			mem_id = id_Cause;
			
		}
		else if(!strncasecmp(line_buff,"Cause",strlen("Cause")))
		{
			ie_value_list[BackoffTimer] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[BackoffTimer]->add_func = add_case;
			mem_id = BackoffTimer;
		}
		else
		{
			continue;
		}
		p = strchr(line_buff,':');
		p++;
		ie_value_list[mem_id]->Criticality = *p - 0x30;
		p = strrchr(line_buff,':'); 
		p++;
		skip_char(p,'}');
		skip_char(p,',');
		if(!strncasecmp(p, "choice",strlen("choice")))
		{
			my_Choice_t choice;
			memset(&choice,0,sizeof(my_Choice_t));

			p = strchr(line_buff, '=');
			p++;
			choice.present = atol(p);
			p = strrchr(line_buff,'='); 
			p++;
			choice.value = calloc(1,sizeof(long));
			tmp_long = atoi(p);
			memcpy(choice.value, &tmp_long, sizeof(long));
			ie_value_list[mem_id]->buff_size = sizeof(my_Choice_t);
			ie_value_list[mem_id]->ie_buff = calloc(1,sizeof(my_Choice_t));
			memcpy(ie_value_list[mem_id]->ie_buff,&choice, sizeof(my_Choice_t));
			//fprintf(stderr,"value : choice\n\tpresent = %d\n\tcause = %d\n",choice.present,*((long*)choice.value));
		}
		else
		{
			ie_value_list[mem_id]->buff_size = strlen(p)+1;
			ie_value_list[mem_id]->ie_buff = calloc(1,strlen(p)+1);
			strcpy(ie_value_list[mem_id]->ie_buff,p);
			//fprintf(stderr,"value : %s\n",ie_value_list[mem_id]->ie_buff);
		}
	}
}

void	init_hnb_de_register_ie_list(FILE *file)
{
	char line_buff[128];
	char *p;
	int mem_id;
	long tmp_long;
	
	fseek(file,0,SEEK_SET);

	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(!strncasecmp(line_buff,"HNB_De_Register_PDU",strlen("HNB_De_Register_PDU")))
			continue;
		fprintf(stderr,"HNB_De_Register_PDU\n");
		break;
	}
	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(strlen(line_buff) == 0)
			continue;
		if(!strncasecmp(line_buff,"Cause",strlen("Cause")))
		{
			ie_value_list[id_Cause] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[id_Cause]->add_func = add_case;
			mem_id = id_Cause;
			
		}
		else if(!strncasecmp(line_buff,"BackoffTimer",strlen("BackoffTimer")))
		{
			ie_value_list[BackoffTimer] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[BackoffTimer]->add_func = add_backofftimer;
			mem_id = BackoffTimer;
		}
		else
		{
			continue;
		}
		p = strchr(line_buff,':');
		p++;
		ie_value_list[mem_id]->Criticality = *p - 0x30;
		p = strrchr(line_buff,':'); 
		p++;
		skip_char(p,'}');
		skip_char(p,',');
		if(!strncasecmp(p, "choice",strlen("choice")))
		{
			my_Choice_t choice;
			memset(&choice,0,sizeof(my_Choice_t));

			p = strchr(line_buff, '=');
			p++;
			choice.present = atol(p);
			p = strrchr(line_buff,'='); 
			p++;
			choice.value = calloc(1,sizeof(long));
			tmp_long = atoi(p);
			memcpy(choice.value, &tmp_long, sizeof(long));
			ie_value_list[mem_id]->buff_size = sizeof(my_Choice_t);
			ie_value_list[mem_id]->ie_buff = calloc(1,sizeof(my_Choice_t));
			memcpy(ie_value_list[mem_id]->ie_buff,&choice, sizeof(my_Choice_t));
			//fprintf(stderr,"value : choice\n\tpresent = %d\n\tcause = %d\n",choice.present,*((long*)choice.value));
		}
		else
		{
			ie_value_list[mem_id]->buff_size = strlen(p)+1;
			ie_value_list[mem_id]->ie_buff = calloc(1,strlen(p)+1);
			strcpy(ie_value_list[mem_id]->ie_buff,p);
			//fprintf(stderr,"value : %s\n",ie_value_list[mem_id]->ie_buff);
		}
	}
}

void init_hnb_register_ie_list(FILE * file)
{
	char line_buff[128];
	char *p;
	int mem_id;	
	
	fseek(file,0,SEEK_SET);

	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(!strncasecmp(line_buff,"HNB_Register_Request_PDU",strlen("HNB_Register_Request_PDU")))
			continue;
		fprintf(stderr,"HNB_Register_Request_PDU\n");
		break;
	}
	while(fgets(line_buff,128,file))
	{
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(strlen(line_buff) == 0)
			continue;
		if(!strncasecmp(line_buff,"HNB_IDENTITY",strlen("HNB_IDENTITY")))
		{
			ie_value_list[HNB_Ident] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[HNB_Ident]->add_func = add_hnb_identity;
			mem_id = HNB_Ident;
		}
		else if(!strncasecmp(line_buff,"HNB_LOCATION_INFO",strlen("HNB_LOCATION_INFO")))
		{
			ie_value_list[HNB_Location_Info] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[HNB_Location_Info]->add_func = add_hnb_location_info;
			mem_id = HNB_Location_Info;
		}
		else if(!strncasecmp(line_buff,"PLMN_ID",strlen("PLMN_ID")))
		{
			ie_value_list[PLMNid] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[PLMNid]->add_func = add_plmn_id;
			mem_id = PLMNid;
		}
		else if(!strncasecmp(line_buff,"CELL_ID",strlen("CELL_ID")))
		{
			ie_value_list[CellId] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[CellId]->add_func = add_cell_id;
			mem_id = CellId;
		}
		else if(!strncasecmp(line_buff,"LAC",strlen("LAC")))
		{
			ie_value_list[LAC] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[LAC]->add_func = add_lac;
			mem_id = LAC;
		}
		else if(!strncasecmp(line_buff,"RAC",strlen("RAC")))
		{
			ie_value_list[RAC] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[RAC]->add_func = add_rac;
			mem_id = RAC;
		}
		else if(!strncasecmp(line_buff,"SAC",strlen("SAC")))
		{
			ie_value_list[SAC] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[SAC]->add_func = add_sac;
			mem_id = SAC;
		}
		else if(!strncasecmp(line_buff,"CSG_ID",strlen("CSG_ID")))
		{
			ie_value_list[CSG_ID] = calloc(1,sizeof(IE_Value_t));
			ie_value_list[CSG_ID]->add_func = add_csg_id;
			mem_id = CSG_ID;
		}
		else
		{
			continue;
		}
		p = strchr(line_buff,':');
		p++;
		ie_value_list[mem_id]->Criticality = *p - 0x30;
		p = strrchr(line_buff,':'); 
		p++;
		skip_char(p,'}');
		skip_char(p,',');
		ie_value_list[mem_id]->buff_size = strlen(p)+1;
		ie_value_list[mem_id]->ie_buff = calloc(1,strlen(p)+1);
		strcpy(ie_value_list[mem_id]->ie_buff,p);
		//fprintf(stderr,"value : %s\n",ie_value_list[mem_id]->ie_buff);
	}
}

void init_ie_value_list(FILE *file, ProcedureCode_ID_t procode)
{
	memset(ie_value_list, 0, sizeof(ie_value_list));
	switch(procode)
	{
		case HNBRegister:
			init_hnb_register_ie_list(file);
			break;
		case HNBDeRegister:
			init_hnb_de_register_ie_list(file);
			break;
		case UERegister:
			init_ue_register_ie_list(file);
			break;
		case UEDeRegister:
			init_ue_de_register_ie_list(file);
			break;
		case ErrorIndicate:
			break;
		case privateMessage:
			break;
		case CSGMemberUpdate:
			break;
		default:
			break;
	}
}


void clear_ie_value_list()
{
	int i;
	for(i=0;i<MAX_MEMBER_ID-1;i++)
	{
		if(ie_value_list[i])
		{
			if(ie_value_list[i]->ie_buff)
				free(ie_value_list[i]->ie_buff);
			free(ie_value_list[i]);
		}
	}
	memset(ie_value_list,0,sizeof(ie_value_list));
}

Criticality_t ulong2Criticality(unsigned long ul)
{
	INTEGER_t integer;
	memset(&integer,0,sizeof(INTEGER_t));
	asn_ulong2INTEGER(&integer, ul);
	return ((Criticality_t)integer);
}

int BIT_STRING_fromBuf(BIT_STRING_t *st, const char *str, int len, int unused) 
{
	void *buf;

	if(st == 0 || (str == 0 && len)) {
		errno = EINVAL;
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

unsigned int fill_default_ie_container(ProtocolIE_Container_110P0_t *ie_container)
{
	char buf[] = {0xff,0xff,0xff,0xf0};

	add_hnb_identity(ie_container, Criticality_ignore, "1", 1);
	add_hnb_location_info(ie_container, Criticality_ignore,"192.168.1.16",4);
	add_plmn_id(ie_container, Criticality_ignore,"111",3);
	add_cell_id(ie_container, Criticality_ignore,buf,sizeof(buf));
	add_lac(ie_container, Criticality_ignore,"11",2);
	add_rac(ie_container, Criticality_ignore,"1",1);
	add_sac(ie_container, Criticality_ignore,"11",2);
	add_csg_id(ie_container, Criticality_ignore,buf,sizeof(buf));
}

unsigned int fill_ie_container(ProtocolIE_Container_110P0_t *ie_container,	IE_Value_t **ie_list)
{
	int i;
	
	for(i = 0; i < MAX_MEMBER_ID; i++)
	{
		if(ie_list[i])
		{
			ie_list[i]->add_func(ie_container, ie_list[i]->Criticality, ie_list[i]->ie_buff,	ie_list[i]->buff_size);
		}
	}
}

unsigned int encode_hnb_register_request_pdu(ANY_t *value,int default_flag)
{
	asn_enc_rval_t ret;
	HNBRegisterRequest_t hnbap_reg_request;
	ProtocolIE_Container_110P0_t *ie_container;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

		memset(&hnbap_reg_request, 0, sizeof(HNBRegisterRequest_t));
		ie_container = &hnbap_reg_request.protocolIEs;
		
		if(default_flag == 1)
		{
			fill_default_ie_container(ie_container);
		}
		else
		{
			fill_ie_container(ie_container, ie_value_list);
		}	
		ret = uper_encode_to_buffer(&asn_DEF_HNBRegisterRequest,(void *)&hnbap_reg_request, pdu_buff, PDU_BUFFER_SIZE);
		if(ret.encoded == -1)
		{
			fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
			return (1);
		}
		ANY_fromBuf((OCTET_STRING_t *)value, pdu_buff, (ret.encoded+7)/8);
		return 0;
}

unsigned int encode_hnb_de_register_pdu(ANY_t *value)
{
#if 1
	asn_enc_rval_t ret;
	HNBDe_Register_t hnbap_de_reg;
	ProtocolIE_Container_110P0_t *ie_container;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

		memset(&hnbap_de_reg, 0, sizeof(HNBDe_Register_t));
		ie_container = &hnbap_de_reg.protocolIEs;
		
		fill_ie_container(ie_container, ie_value_list);

		ret = uper_encode_to_buffer(&asn_DEF_HNBDe_Register,(void *)&hnbap_de_reg, pdu_buff, PDU_BUFFER_SIZE);
		if(ret.encoded == -1)
		{
			fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
			return (1);
		}
		ANY_fromBuf((OCTET_STRING_t *)value, pdu_buff, (ret.encoded+7)/8);
		return 0;
#endif
}

unsigned int encode_ue_register_request_pdu(ANY_t *value)
{
#if 1
	asn_enc_rval_t ret;
	UERegisterRequest_t ue_reg;
	ProtocolIE_Container_110P0_t *ie_container;
	unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};

		memset(&ue_reg, 0, sizeof(UERegisterRequest_t));
		ie_container = &ue_reg.protocolIEs;
		
		fill_ie_container(ie_container, ie_value_list);

		ret = uper_encode_to_buffer(&asn_DEF_UERegisterRequest,(void *)&ue_reg, pdu_buff, PDU_BUFFER_SIZE);
		if(ret.encoded == -1)
		{
			fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
			return (1);
		}
		ANY_fromBuf((OCTET_STRING_t *)value, pdu_buff, (ret.encoded+7)/8);
		return 0;
#endif
}

unsigned int encode_ue_de_register_pdu(ANY_t *value)
{
#if 1
		asn_enc_rval_t ret;
		UEDe_Register_t ue_de_reg;
		ProtocolIE_Container_110P0_t *ie_container;
		unsigned char pdu_buff[PDU_BUFFER_SIZE] = {0};
	
			memset(&ue_de_reg, 0, sizeof(UEDe_Register_t));
			ie_container = &ue_de_reg.protocolIEs;
			
			fill_ie_container(ie_container, ie_value_list);
	
			ret = uper_encode_to_buffer(&asn_DEF_UEDe_Register,(void *)&ue_de_reg, pdu_buff, PDU_BUFFER_SIZE);
			if(ret.encoded == -1)
			{
				fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
				return (1);
			}
			ANY_fromBuf((OCTET_STRING_t *)value, pdu_buff, (ret.encoded+7)/8);
			return 0;
#endif
}

ProcedureCode_ID_t 	init_procedure(FILE *file)
{
	unsigned char line_buff[128];
	char *p;
	unsigned int len;	

	fseek(file,0,SEEK_SET);
	while(fgets(line_buff,128,file))
	{
		len = strlen(line_buff);
		skip_space(line_buff);
		if(!memcmp(line_buff,"--",2) || !memcmp(line_buff,"//",2))
			continue;
		if(!strncasecmp(line_buff,"procedure",strlen("procedure")))
		{
			p = strchr(line_buff,':');
			p++;
			fprintf(stderr,"procedure : %s\n",p);
			break;
		}
	}
	if(!strncasecmp(p,"HNB_Register",strlen("HNB_Register")))
		return HNBRegister;
	else if(!strncasecmp(p,"HNB_De_Register",strlen("HNB_De_Register")))
		return HNBDeRegister;
	else if(!strncasecmp(p,"UE_Register",strlen("UE_Register")))
		return UERegister;
	else if(!strncasecmp(p,"UE_De_Register",strlen("UE_De_Register")))
		return UEDeRegister;
	else if(!strncasecmp(p,"Error_Indication",strlen("Error_Indication")))
		return ErrorIndicate;
	else if(!strncasecmp(p,"Private_Message",strlen("Private_Message")))
		return privateMessage;
	else if(!strncasecmp(p,"CSG_MemberShip_Update",strlen("CSG_MemberShip_Update")))
		return CSGMemberUpdate;
	else
	{
		fprintf(stderr,"file data error! can't get procedure code...\n");
		return Error_procdure_code;
	}

	
}
unsigned int create_hnbap_pdu(unsigned char*buff, unsigned int buff_size, int default_flag)
{
	HNBAP_PDU_t hnbap_pdu;
	InitiatingMessage_t *initiating_msg;
	asn_enc_rval_t ret;

	//fprintf(stderr, "default_flag : %d\n",default_flag);
	memset(buff, 0, buff_size);
//create hnbap_pdu
	memset(&hnbap_pdu, 0, sizeof(HNBAP_PDU_t));
	hnbap_pdu.present = HNBAP_PDU_PR_initiatingMessage;

	initiating_msg = &(hnbap_pdu.choice.initiatingMessage);
	initiating_msg->procedureCode = procedure_code;
	initiating_msg->criticality = ulong2Criticality(Criticality_ignore);

	if(default_flag == 1)
	{
		encode_hnb_register_request_pdu(&(initiating_msg->value),default_flag);
	}
	else
	{
		switch(procedure_code)
		{
			case HNBRegister:
				encode_hnb_register_request_pdu(&(initiating_msg->value),default_flag);
				break;
			case HNBDeRegister:
				encode_hnb_de_register_pdu(&(initiating_msg->value));
				break;
			case UERegister:
				encode_ue_register_request_pdu(&(initiating_msg->value));
				break;
			case UEDeRegister:
				encode_ue_de_register_pdu(&(initiating_msg->value));
				break;
			case ErrorIndicate:
				fprintf(stderr,"ErrorIndicate no support yet\n");
				break;
			case privateMessage:
				fprintf(stderr,"privateMessage no support yet\n");
				break;
			case CSGMemberUpdate:
				fprintf(stderr,"privateMessage no support yet\n");
				break;
			default:
				fprintf(stderr,"Unknow proc no support yet\n");
				break;
		}
	}
//create hnbap_pdu end
	ret = uper_encode_to_buffer(&asn_DEF_HNBAP_PDU, (void *)&hnbap_pdu, buff, buff_size);

	if(ret.encoded == -1)
	{
		fprintf(stderr,"%s(%d): encode error!\n",__func__,__LINE__);
		return (-1);
	}
	else
		return (ret.encoded+7)/8;
}

int decode_member_ex(Member_ProtocolIE_Container_t *member)
{
	void *structure;
	asn_dec_rval_t rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;

	switch(member->id)
	{
		case id_Cause:
			structure = calloc(1,sizeof(Cause_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_Cause, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_Cause, structure);
			break;
		case CriticalityDiagnosis:
			structure = calloc(1,sizeof(CriticalityDiagnostics_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&HNBAP_asn_DEF_CriticalityDiagnostics, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &HNBAP_asn_DEF_CriticalityDiagnostics, structure);
			break;
		case HNB_Ident:
			structure = calloc(1,sizeof(HNB_Identity_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_HNB_Identity, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_HNB_Identity, structure);
			break;
		case Context_ID:
			structure = calloc(1,sizeof(Context_ID_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_Context_ID, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			//rember context_id for ue_de_register
			if(my_context_id == NULL)
				my_context_id = calloc(1,sizeof(Context_ID_t));
			memcpy(my_context_id,structure,sizeof(Context_ID_t));
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_Context_ID, structure);
			break;
		case UE_Id:
			structure = calloc(1,sizeof(UE_Identity_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_UE_Identity, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_UE_Identity, structure);
			break;
		case LAC:
			structure = calloc(1,sizeof(LAC_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_LAC, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_LAC, structure);
			break;
		case RAC:
			structure = calloc(1,sizeof(RAC_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_RAC, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_RAC, structure);
			break;
		case HNB_Location_Info:
			structure = calloc(1,sizeof(HNB_Location_Information_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_HNB_Location_Information, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_HNB_Location_Information, structure);
			break;
		case PLMNid:
			structure = calloc(1,sizeof(PLMNidentity_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_PLMNidentity, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_PLMNidentity, structure);
			break;
		case SAC:
			structure = calloc(1,sizeof(SAC_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_SAC, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_SAC, structure);
			break;
		case CellId:
			structure = calloc(1,sizeof(CellIdentity_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_CellIdentity, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_CellIdentity, structure);
			break;
		case Register_Cause:
			structure = calloc(1,sizeof(Registration_Cause_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_Registration_Cause, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_Registration_Cause, structure);
			break;
		case UE_Capability:
			structure = calloc(1,sizeof(UE_Capabilities_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_UE_Capabilities, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_UE_Capabilities, structure);
			break;
		case RNC_ID:
			structure = calloc(1,sizeof(RNC_ID_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_RNC_ID, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_RNC_ID, structure);
			break;
		case CSG_ID:
			structure = calloc(1,sizeof(CSG_ID_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_CSG_ID, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_CSG_ID, structure);
			break;
		case BackoffTimer:
			structure = calloc(1,sizeof(BackoffTimer_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_BackoffTimer, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_BackoffTimer, structure);
			break;
		case HNB_Internet_Information:
			break;
		case Cell_Access_Mode:
			structure = calloc(1,sizeof(HNB_Cell_Access_Mode_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_HNB_Cell_Access_Mode, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_HNB_Cell_Access_Mode, structure);
			break;
		case MuxPortNumber:
			structure = calloc(1,sizeof(MuxPortNumber_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_MuxPortNumber, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_MuxPortNumber, structure);
			break;
		case SA_For_Broadcast:
			break;
		case CSGMemberStatus:
			structure = calloc(1,sizeof(CSGMembershipStatus_t));
			codec_ctx.max_stack_size = 30000;
			rval = uper_decode_complete(&codec_ctx, 
																		&asn_DEF_CSGMembershipStatus, 
																		(void **)&structure, 
																		member->value.buf, 
																		member->value.size);
			
			if(rval.code != RC_OK)
			{
				fprintf(stderr,"decode hnb_register_request failed!\n***");
				return 1;
			}
			xer_fprint(stderr, &asn_DEF_CSGMembershipStatus, structure);
			break;
		default:
			break;
	}

}

int decode_members(ProtocolIE_Container_110P0_t *protocolIEs)
{
	int i;
	
	for(i=0; i<protocolIEs->list.count; i++)
	{
		decode_member_ex(protocolIEs->list.array[i]);
	}
	return 0;
}

int proc_hnb_register_request(ANY_t * value)
{
	HNBRegisterRequest_t *structure;
	asn_dec_rval_t rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (HNBRegisterRequest_t *)calloc(1,sizeof(HNBRegisterRequest_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_HNBRegisterRequest, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_HNBRegisterRequest, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_hnb_register_accept(ANY_t * value)
{
	HNBRegisterAccept_t *structure;
	asn_dec_rval_t rval;
	asn_enc_rval_t enc_rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (HNBRegisterAccept_t *)calloc(1,sizeof(HNBRegisterAccept_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_HNBRegisterAccept, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_HNBRegisterAccept, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_ue_register_accept(ANY_t * value)
{
	UERegisterAccept_t *structure;
	asn_dec_rval_t rval;
	asn_enc_rval_t enc_rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (UERegisterAccept_t *)calloc(1,sizeof(UERegisterAccept_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_UERegisterAccept, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_UERegisterAccept, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}


int proc_hnb_register_reject(ANY_t * value)
{
	HNBRegisterReject_t *structure;
	asn_dec_rval_t rval;
	asn_enc_rval_t enc_rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (HNBRegisterReject_t *)calloc(1,sizeof(HNBRegisterReject_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_HNBRegisterReject, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_HNBRegisterReject, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_ue_register_reject(ANY_t * value)
{
	UERegisterReject_t *structure;
	asn_dec_rval_t rval;
	asn_enc_rval_t enc_rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (UERegisterReject_t *)calloc(1,sizeof(UERegisterReject_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_UERegisterReject, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_UERegisterReject, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_hnb_de_register(ANY_t * value)
{
	HNBDe_Register_t *structure;
	asn_dec_rval_t rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (HNBDe_Register_t *)calloc(1,sizeof(HNBDe_Register_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_HNBDe_Register, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_HNBDe_Register, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_ue_register_request(ANY_t * value)
{
	UERegisterRequest_t *structure;
	asn_dec_rval_t rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (UERegisterRequest_t *)calloc(1,sizeof(UERegisterRequest_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_UERegisterRequest, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_UERegisterRequest, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_ue_de_register(ANY_t * value)
{
	UEDe_Register_t *structure;
	asn_dec_rval_t rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (UEDe_Register_t *)calloc(1,sizeof(UEDe_Register_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, 
																&asn_DEF_UEDe_Register, 
																(void **)&structure, 
																value->buf, 
																value->size);
	
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode hnb_register_request failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_UEDe_Register, structure);
	
	ret = decode_members(&structure->protocolIEs);
	return ret;
}

int proc_initiatingMessage(InitiatingMessage_t  *initiatingMessage)
{
	int ret;

	switch(initiatingMessage->procedureCode)
	{
		case HNBRegister:
			ret = proc_hnb_register_request(&initiatingMessage->value);
			break;
		case HNBDeRegister:
			ret = proc_hnb_de_register(&initiatingMessage->value);
			break;
		case UERegister:
			ret = proc_ue_register_request(&initiatingMessage->value);
			break;
		case UEDeRegister:
			ret = proc_ue_de_register(&initiatingMessage->value);
			break;
		default:
			fprintf(stderr,"%s(%d): unsupport\n",__func__,__LINE__);
			break;
	}
	return ret;
}

int proc_successfulOutcome(	SuccessfulOutcome_t  *successfulOutcome)
{
	int ret;
	
	switch(successfulOutcome->procedureCode)
	{
		case HNBRegister:
			ret = proc_hnb_register_accept(&successfulOutcome->value);
			break;
		case UERegister:
			ret = proc_ue_register_accept(&successfulOutcome->value);
			break;
		default:
			fprintf(stderr,"%s(%d): unsupport\n",__func__,__LINE__);
			break;
	}
	return ret;
}

int proc_unsuccessfulOutcome(UnsuccessfulOutcome_t  *unsuccessfulOutcome)
{
	int ret;
	
	switch(unsuccessfulOutcome->procedureCode)
	{
		case HNBRegister:
			ret = proc_hnb_register_reject(&unsuccessfulOutcome->value);
			break;
		case UERegister:
			ret = proc_ue_register_reject(&unsuccessfulOutcome->value);
		default:
			fprintf(stderr,"%s(%d): unsupport\n",__func__,__LINE__);
			break;
	}
	return ret;
}


int proc_hnbap_pdu(unsigned char*buff, unsigned int buff_size)
{
	HNBAP_PDU_t *structure;
	SuccessfulOutcome_t *successful;
	asn_dec_rval_t rval;
	asn_enc_rval_t enc_rval;
	asn_codec_ctx_t codec_ctx;
	int ret = 0;
		
	structure = (HNBAP_PDU_t *)calloc(1,sizeof(HNBAP_PDU_t));

	codec_ctx.max_stack_size = 30000;

	rval = uper_decode_complete(&codec_ctx, &asn_DEF_HNBAP_PDU, (void **)&structure, buff, buff_size);
	if(rval.code != RC_OK)
	{
		fprintf(stderr,"decode HNBAP_PDU failed!\n***");
		return 1;
	}
	xer_fprint(stderr, &asn_DEF_HNBAP_PDU, structure);
	switch(structure->present)
	{
		case HNBAP_PDU_PR_initiatingMessage:
			ret = proc_initiatingMessage(&structure->choice.initiatingMessage);
			break;
		case HNBAP_PDU_PR_successfulOutcome:
			ret = proc_successfulOutcome(&structure->choice.successfulOutcome);
			break;
		case HNBAP_PDU_PR_unsuccessfulOutcome:
			ret = proc_unsuccessfulOutcome(&structure->choice.unsuccessfulOutcome);
			break;
		case HNBAP_PDU_PR_NOTHING:/* No components present */
		default:
			fprintf(stderr,"decode NOTHING!\n***");
			break;
	}
	if(ret == 0)
		return 0;
	else
	{
		fprintf(stderr,"decode failed!\n***");
		return 1;
	}
}



int main(int argc, char *argv[])
{
	int sock;
	int ret, yes = 1;
	struct sockaddr_in dest_addr; 
	socklen_t addr_len;
	fd_set rfdset,wfdset;
	struct sctp_sndrcvinfo sinfo;
	struct timeval tv;			
	unsigned char buf[512] = {0};	
	unsigned int encode_len;
	FILE *file;
	int default_flag;
	char cmd_buf[512];
	char *p;

	dest_addr.sin_family = AF_INET;	
	dest_addr.sin_port = htons(29169);
	dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(dest_addr.sin_zero, '\0', sizeof(dest_addr.sin_zero));	
	addr_len = sizeof(struct sockaddr);

	if(init_sock(&dest_addr, &sock))
	{
		fprintf(stderr,"init sock error!\n");
		exit(1);
	}

	tv.tv_sec = 1;
	tv.tv_usec = 5000;

	while(1)
	{
		//GET_CMD
		memset(cmd_buf,0,512);
		fprintf(stderr,"\n>");
		read(STDIN_FILENO,cmd_buf,512);
		skip_space(cmd_buf);
		if(!strlen(cmd_buf))
			continue;
		if(p = strstr(cmd_buf,"send"))
		{
			p+=4;

			clear_ie_value_list();
			default_flag = 0;
			//memcpy(ip_addr,argv[1],strlen(argv[1]));
			file = fopen(p,"r");
			if(file == NULL)
			{
				fprintf(stderr,"%s(%d):open file failed...\n",__func__,__LINE__);
				default_flag = 1;
				procedure_code = HNBRegister;
			}
			else
			{
				procedure_code = init_procedure(file);
				fprintf(stderr,"%d\n",procedure_code);
				if(procedure_code == Error_procdure_code)
				{
					fprintf(stderr,"%s(%d):file data false...\n",__func__,__LINE__);
					default_flag = 1;
					procedure_code = HNBRegister;
				}
				init_ie_value_list(file, procedure_code);
			}
			encode_len = create_hnbap_pdu(buf,512,default_flag);
			if(encode_len == -1)
			{
				fprintf(stderr, "encode failed!\n");
				exit(0);
			}
			CWCaptrue(encode_len,buf);
			
			FD_ZERO(&wfdset);
			FD_SET(sock,&wfdset);
			if (select(sock+1, 0, &wfdset, 0, &tv) == -1)
			{
				fprintf(stderr,"select err\n");
				break;
			}
			
			if(FD_ISSET(sock, &wfdset))
			{
				//send_msg
				fprintf(stderr,"test send_msg...\n");
				ret = sctp_sendmsg(sock,(void *)buf, encode_len,(struct sockaddr *)&dest_addr,addr_len, HNBAP_PPID,0,2,0,0 );
				if( ret < 0)
				{
					perror("%s(%d):");
					fprintf(stderr,"send_msg failed...\n");
				}
			}
		}
		else if(p = strstr(cmd_buf,"recv"))
		{
			FD_ZERO(&rfdset);
			FD_SET(sock,&rfdset);
			if (select(sock+1, &rfdset, 0, 0, &tv) == -1)
			{
				fprintf(stderr,"select err\n");
				break;
			}
			if(FD_ISSET(sock, &rfdset))
			{
				struct sockaddr_in d_addr;
				//recv_msg
				fprintf(stderr,"test recv_msg...\n");
				memset(&sinfo,0, sizeof(struct sctp_sndrcvinfo));
				memset(buf,0,512);
				ret = sctp_recvmsg(sock, (void *)buf, 512,(struct sockaddr*)&d_addr,&addr_len,&sinfo,&yes);
				if(ret < 0)
				{
					fprintf(stderr,"recv msg error\n");
				}
				CWCaptrue(ret,buf);
				if(proc_hnbap_pdu(buf, ret))
				{
					fprintf(stderr,"proc_hnbap_pdu error!\n");
				}
			}
			else
			{
				fprintf(stderr,"recv nothing...\n");
			}
		}
		else
		{
			fprintf(stdout,"unknow cmmond...\n");
			continue;
		}
	}
 	return 0;
 }

