#ifndef _IUH_IURECV_H
#define	_IUH_IURECV_H

void * IuRecv(void * arg);
void * IupsRecv(void * arg);
IuhBool Iuh_IU_InitSocket(int *sock);
IuhBool Iuh_IUPS_InitSocket(int *sock);

#endif

