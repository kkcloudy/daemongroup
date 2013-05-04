#ifndef _IUH_MANAGEHNB_H
#define	_IUH_MANAGEHNB_H

#include "Iuh_Msgq.h"

void FemtoIuhRun();
void * IuhManageHNB(void *arg);

extern IuhMultiHomedSocket gIuhSocket;

IuhBool check_hnbid_func(unsigned int HNBID);

IuhBool IuhParseHnbRegister(const char *buf, const int readBytes, HnbRegisterRequestValue *requestValue, HNBAPCause *cause);
IuhBool IuhParseHnbDeregister(const char *buf, const int readBytes, HNBAPCause *cause);
IuhBool IuhParseUERegister(const char *buf, const int readBytes, UERegisterRequestValue *UERequestValue, HNBAPCause *cause);
IuhBool IuhParseUEDeregister(const char *buf, const int readBytes, HNBAPCause *cause, char *ContextID);


#endif
