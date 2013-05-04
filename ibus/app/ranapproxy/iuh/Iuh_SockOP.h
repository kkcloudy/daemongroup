#ifndef _IUH_SOCKOP_H
#define _IUH_SOCKOP_H

#include <netinet/sctp.h> 

#define Iuh_SCTP_PORT 29169
#define MSG_LEN 2048


typedef int IuhSocket;
typedef struct sockaddr_storage IuhNetworkLev4Address;


struct IuhMultiHomedInterface{
	char ifname[16];
	IuhNetworkLev4Address addr;
	IuhNetworkLev4Address addrIPv4;
	IuhNetworkLev4Address addrIPv6;
	IuhSocket sock;
	int sock_type;          //zhangshu add, 1 interface socket,  2 iuh socket 
	enum {
		CW_PRIMARY,
		CW_BROADCAST_OR_ALIAS
	} kind;
	short systemIndex; // real interface index in the system
	short systemIndexbinding;
	int gIf_Index;
	char nas_id[128];
	int sub_sock_count;
	struct IuhMultiHomedInterface *if_next;
	struct IuhMultiHnbSocket *sub_sock; 
};


struct IuhMultiHnbSocket{
	IuhNetworkLev4Address addr;
	IuhSocket sock;
	int HNBID;
	int sock_type;          //zhangshu add, 1 interface socket,  2 iuh socket 
	struct sctp_sndrcvinfo sinfo;
	struct IuhMultiHnbSocket *sock_next;
	struct IuhMultiHomedInterface *if_belong;
};


typedef struct {
	int count;
	struct IuhMultiHomedInterface *interfaces;
} IuhMultiHomedSocket;


typedef struct procotolMsg{
    char buf[MSG_LEN];
    unsigned int size;
}IuhProcotolMsg;


void IuhManageIncomingPacket(struct IuhMultiHnbSocket * sock_info, char *buf, int readBytes, int type);
int IuhSCTPSend(struct IuhMultiHnbSocket * sock_info, IuhProcotolMsg *msgPtr);


#endif
