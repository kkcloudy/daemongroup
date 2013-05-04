#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include <linux/tipc.h>
#include "iuh/Iuh.h"
#include "Iuh_Thread.h"
#include "Iuh_log.h"
#include "Iuh_DBus_handler.h"
#include "ranapConstants.h"

int IU_IuhMSGQ;
int IUPS_IuhMSGQ;

extern void CWCapture(int n ,unsigned char *buffer);


/*****************************************************
** DISCRIPTION:
**          initiate sockets to iu interface
** INPUT:
**          socketfd
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool Iuh_IU_InitSocket(int *sock){
	struct sockaddr_un local;
	char IUH_PATH[PATH_LEN] = "/var/run/femto/iuh";
	char IU_PATH[PATH_LEN] = "/var/run/femto/iu";
	int len;//,t,rlen;
	//InitPath(vrrid,IUH_PATH);
	//InitPath(vrrid,IU_PATH);

	iuh_syslog_debug_debug(IUH_DEFAULT, "IUH_PATH %s, IU_PATH %s\n",IUH_PATH,IU_PATH);
	
	int sndbuf = SOCK_BUFSIZE;
	*sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (*sock < 0 ) {
		iuh_syslog_crit("%s socket init error %s",__func__,strerror(errno));
		perror("socket[PF_UNIX,SOCK_STREAM]");
		return -1;
	}
	
	local.sun_family = PF_UNIX;
	strncpy(local.sun_path,IUH_PATH,strnlen(IUH_PATH, PATH_LEN));
	unlink(local.sun_path); 
	
	len = strnlen(local.sun_path,PATH_LEN) + sizeof(local.sun_family);
	if (bind(*sock, (struct sockaddr *) &local, len) < 0) {
		iuh_syslog_crit("%s socket bind error %s",__func__,strerror(errno));
		perror("bind(PF_UNIX)");
		return -1;
	}
	int i=0;
	for(i=0;i<SOCK_NUM;i++){
		
		sockPerThread[i]= socket(PF_UNIX, SOCK_DGRAM, 0);
		if ((setsockopt(sockPerThread[i],SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) {	
			iuh_syslog_crit("%s setsockopt %s",__func__,strerror(errno));
			perror("setsockopt");
			return -1;
		}
		fcntl(sockPerThread[i], F_SETFL, O_NONBLOCK);
	}

	toIU.addr.sun_family = PF_UNIX;
	strncpy(toIU.addr.sun_path,IU_PATH,PATH_LEN); 
	toIU.addrlen = strnlen(toIU.addr.sun_path,PATH_LEN) + sizeof(toIU.addr.sun_family);
	
	return Iuh_TRUE;
}


/*****************************************************
** DISCRIPTION:
**          receive message from iu interface
** INPUT:
**          null
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void * IuRecv(void * arg){
	struct sockaddr_tipc addr;
	socklen_t addrlen = sizeof(addr);
	while(1){
		int len;
		int UEID = 0;
		unsigned char buf[8192];
		IuhGetMsgQueue(&IU_IuhMSGQ);
		//len = recvfrom(IuSocket, buf, sizeof(buf), 0, NULL, 0);
		len = recvfrom(IuTipcSocket, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);//xiaodw modify for tipc recvfrom iu
		if (len < 0) {
			perror("recv from Iu ERROR!");
			return 0;
		}

		iuh_syslog_info("\n\n\n#################################");
		iuh_syslog_info("###  Receive Message From Iu Interface\n");
		iuh_syslog_info("### msg_len = %d\n",len);
		Iuh2IuMsg *msg = (Iuh2IuMsg*)buf;
        /* use RanapType to map SigType & RuaType */
        msg->RuaType.msgType = RUA_PR_initiatingMessage;
        switch(msg->ranap_type){
            /* book modify, 2011-12-27 */
            case id_InitialUE_Message:
            case id_RelocationResourceAllocation:{
                msg->RuaType.ruaType = Rua_Connect;
                break;
            }
            default:{
                switch(msg->SigType){
                    case CONNECTIONLESS_FROM_CN:{
                        msg->RuaType.ruaType = Rua_ConnectionlessTransfer;
                        break;
                    }
                    default:{
                        msg->RuaType.ruaType = Rua_DirectTransfer;
                        break;
                    }
                }
                break;
            }
        }
		
		int HNBID = 0;
		HNBID = IUH_FIND_HNB_CONTEXT((char*)msg->contextid);
		
		iuh_syslog_debug_debug(IUH_DEFAULT,"@@@ HNBID = %d\n",HNBID);
		
		msgq qData;
		qData.mqid = (HNBID)%THREAD_NUM+1;
		qData.mqinfo.HNBID = HNBID;
		qData.mqinfo.type = IU_DATA;
		if(msg->SigType == CONNECTIONLESS_FROM_CN){
		    qData.mqinfo.subtype = CONNECTIONLESS;
		}
		qData.mqinfo.u.DataInfo.len = len;
		memset(qData.mqinfo.u.DataInfo.Data, 0, 2048);
		memcpy(qData.mqinfo.u.DataInfo.Data, buf, len);

		iuh_syslog_debug_debug(IUH_DEFAULT,"\nIUH2IU MESSAGE :\n");
        iuh_syslog_debug_debug(IUH_DEFAULT,"---RncId = %d\n",msg->RncPlmn.RncId);
        iuh_syslog_debug_debug(IUH_DEFAULT,"---RuaType = %d\n",msg->RuaType.ruaType);
        iuh_syslog_debug_debug(IUH_DEFAULT,"---SigType = %d\n",msg->SigType);
        iuh_syslog_debug_debug(IUH_DEFAULT,"---RanapMsg.size = %d\n",msg->RanapMsg.size);
        iuh_syslog_debug_debug(IUH_DEFAULT,"---CnDomainID = %d\n",msg->CnDomain);
        iuh_syslog_debug_debug(IUH_DEFAULT,"---EstabCause = %d\n",msg->EstabCause);
		iuh_hnb_show_str("---Plmnid = ", msg->RncPlmn.PlmnId, 1, PLMN_LEN);
		iuh_hnb_show_str("---IMSI = ", msg->imsi, 1, IMSI_LEN);
		iuh_hnb_show_str("---ContextID = ", msg->contextid, 0, CONTEXTID_LEN);
		iuh_syslog_debug_debug(IUH_DEFAULT,"---RanapType = %d\n",msg->ranap_type);
		iuh_syslog_debug_debug(IUH_DEFAULT,"IuRecv send msg IuhMsgQid = %d\n",IuhMsgQid);

		CWCapture(msg->RanapMsg.size, (unsigned char*)msg->RanapMsg.RanapMsg);
		if (msgsnd(IuhMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1)
			perror("msgsnd");
	}

}
int IuhServerTipcInit(unsigned int iu_slotid, unsigned int iu_instid, unsigned int islocal)
{
	IuTipcSocket = socket(AF_TIPC, SOCK_RDM, 0);
	struct sockaddr_tipc server_addr;
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = IUH_TIPC_SERVER_TYPE + \
		(iu_slotid-1)*IUH_MAX_INS_NUM*2 + iu_instid + islocal;
	server_addr.addr.nameseq.lower = FEMTO_SERVER_BASE_INST;
	server_addr.addr.nameseq.upper = FEMTO_SERVER_BASE_INST;
	server_addr.scope = TIPC_ZONE_SCOPE;
	if (0 != bind(IuTipcSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		iuh_syslog_err("Iuh: failed to bind port name\n");
		return -1;
	}
	return 0;
}
int IuhClientTipcInit(unsigned int iu_slotid, unsigned int iu_instid, unsigned int islocal)
{
	int sndbuf = SOCK_BUFSIZE;
	int i=0;
	for(i=0; i<SOCK_NUM; i++)
	{		
		TipcsockPerThread[i]= socket(AF_TIPC, SOCK_RDM, 0);
		if ((setsockopt(TipcsockPerThread[i],SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) 
		{	
			iuh_syslog_crit("%s setsockopt %s",__func__,strerror(errno));
			perror("setsockopt");
			return -1;
		}
		fcntl(TipcsockPerThread[i], F_SETFL, O_NONBLOCK);
	}

	Iuh2Iu_tipcaddr.family = AF_TIPC;
	Iuh2Iu_tipcaddr.addrtype = TIPC_ADDR_NAME;
	Iuh2Iu_tipcaddr.addr.name.name.type = IU_TIPC_SERVER_TYPE;
	Iuh2Iu_tipcaddr.addr.name.name.instance = FEMTO_SERVER_BASE_INST + \
		(iu_slotid-1)*IUH_MAX_INS_NUM*2 + iu_instid + islocal;
	Iuh2Iu_tipcaddr.addr.name.domain = 0;
	return 0;
}

