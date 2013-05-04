#ifndef _IUH_ENCODE_
#define _IUH_ENCODE_

#include "Iuh_Msgq.h"

#define PDU_BUFFER_SIZE (512)
#define IE_BUFFER_SIZE (512)
#define RUA_BUFFER_SIZE (1024)



IuhBool IuhAssembleRegisterReject(HNBAPCause *cause, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleRegisterAccept(int HnbIndex, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleUERegisterReject(int UEID, HNBAPCause *cause, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleUERegisterAccept(int UEID, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleConnect(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleDirectTransfer(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleDisconnect(int HNBID, Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleConnectionlessTransfer( Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleRuaErrIndication(Iuh2IuMsg *sigMsg, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleHNBDeRegister(IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleUEDeRegister(int UEID, IuhProcotolMsg *msgPtr);
IuhBool IuhAssembleUERegisterRequest(IuhProcotolMsg *msgPtr, const int UEID);
IuhBool IuhAssembleRanapResetAck(Iuh2IuMsg *sigMsg);
IuhBool IuhAssembleRanapResetResource(IuhProcotolMsg *msgPtr, const int HNBID, Iuh2IuMsg *sigMsg);


#endif
