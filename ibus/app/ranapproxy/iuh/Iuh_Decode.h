#ifndef _IUH_DECODE_
#define _IUH_DECODE_

#include "HNBAP-PDU.h"
#include "RUA-PDU.h"
#include "Iuh_Msgq.h"


IuhBool IuhParseHnbapMessage(const char *buf,const int readBytes, HNBAP_PDU_t *hnbap_pdu);

IuhBool IuhDecodeHnbRegisterRequest(const char *buf, const int size, HnbRegisterRequestValue *requestValue, HNBAPCause *cause);

IuhBool IuhDecodeHnbDeregister(const char *buf, const int size, HNBAPCause *cause);

long IuhBufToLong(char *buf);

IuhBool IuhDecodeUERegisterRequest(const char *buf, const int size, UERegisterRequestValue *UERequestValue, HNBAPCause *cause);

IuhBool IuhDecodeUEDeregister(const char *buf, const int size, HNBAPCause *cause, char *context_id);
IuhBool IuhParseRuaMessage(const char *buf,const int readBytes, RUA_PDU_t *rua_pdu);

IuhBool IuhDecodeConnect(const char *buf, const int size, Iuh2IuMsg *sigMsg);
IuhBool IuhDecodeDirectTransfer(const char *buf, const int size, Iuh2IuMsg *sigMsg);
IuhBool IuhDecodeDisconnect(const char *buf, const int size, Iuh2IuMsg *sigMsg);
IuhBool IuhDecodeConnectionlessTransfer(const char *buf, const int size, Iuh2IuMsg *sigMsg);

IuhBool sccp_decode_RAB_Assignment(const char *buf, const int size, const int UEID);
IuhBool Iuh_decode_Relocation_Request(const char *buf, const int size, char cellId[CELLID_LEN], CNDomain *CnDomain);



#endif
