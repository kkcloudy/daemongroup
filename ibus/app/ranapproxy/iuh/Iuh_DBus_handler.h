#ifndef _IUH_DBUS_HANDLER_H
#define	_IUH_DBUS_HANDLER_H

#include "Iuh_Msgq.h"
#include "Iuh_Stevens.h"


#define	Iuh_COPY_NET_ADDR_PTR(addr1, addr2)  	sock_cpy_addr_port(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)))
#define	Iuh_COPY_NET_ADDR(addr1, addr2)		Iuh_COPY_NET_ADDR_PTR(&(addr1), &(addr2))
typedef enum{
	FEMTO_ACL_SUCCESS = 0,
	MALLOC_ERROR = 1,
	IMSI_EXIST_IN_LIST = 2,
	IMSI_NOT_EXIST_IN_LIST = 3
}FEMTO_ACL_WHITE_LIST;

extern Iuh_HNB  **IUH_HNB;
extern iuh_auto_hnb_info g_auto_hnb_login;


void Delete_Hnb_Socket(int hnbId);
void Delete_Hnb_Socket_by_sock(IuhSocket hnb_sock, IuhSocket sock);
void Delete_Interface_ByName(char *ifname, int ifindex);
int Check_Interface(char *ifname);
int Get_Interface_Info(char * ifname, struct ifi_info *ifi);
int Check_And_Bind_Interface_For_HNB(char * ifname, unsigned char policy);
int Bind_Interface_For_HNB(struct ifi_info *ifi, int port);
int Add_Hnb_Sock(IuhSocket ifsock, IuhSocket hnbsock, struct sockaddr *addrPtr);
int iuh_auto_hnb_login_remove_iflist(char *ifname);
int iuh_auto_hnb_login_insert_iflist(char *ifname);
int IUH_CREATE_NEW_HNB(unsigned char *HNBNAME, int HNBID, HnbRegisterRequestValue *requestValue, int socket);
int IUH_UPDATE_HNB(int HNBID, HnbRegisterRequestValue *requestValue);
int IUH_DELETE_HNB(int HNBID);
struct IuhMultiHnbSocket * IUH_FIND_HNB_ID(int HNBID);
struct IuhMultiHnbSocket * IUH_FIND_HNB_SOCK(int socket);
void IUH_FREE_HnbRegisterRequestValue(HnbRegisterRequestValue *requestValue);
int IUH_CREATE_UE(int HNBID, int UEID, UERegisterRequestValue *UERequestValue);
int IUH_DELETE_UE(int UEID);
void IUH_UPDATE_UE(int UEID, int HNBID, UERegisterRequestValue *UERequestValue);
int FINDUEID(int HNBID, unsigned char *ContextID);
int IUH_FIND_HNB_RNCID(uint16_t RncId);
int IUH_FIND_HNB_CONTEXT(char * contextId);
int IUH_FIND_UE_IMSI(char * IMSI, int HNBID);
void IUH_BZERO_HNBID_BY_SOCK(IuhSocket hnb_sock, IuhSocket sock);
int IUH_FIND_UE_BY_IMSI(char * IMSI);
int IUH_FIND_UE_BY_CTXID(char * contextId);
void Iuh_free_rab(Iu_RAB *myRab);
void Iuh_release_rab(int UEID, const char *RABID);
void IUH_INIT_RAB(Iu_RAB *myRab);
int IUH_OPTIMIZE_PAGING_HNB(int HNBID, Iuh2IuMsg *sigMsg);
int IUH_CMD_DELETE_HNB(int HNBID);
int IUH_CMD_DELETE_UE(int UEID);
int IUH_FIND_HNB_CellId(const char *CellId);
void iuh_hnb_show_str(const char * strname, unsigned char * my_str, int flag, int len);
unsigned char * iuh_hnb_change_str(unsigned char *my_str, int len);


#endif
