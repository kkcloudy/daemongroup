#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/tipc.h>
#include <sys/msg.h>
#include "hmd.h"
#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "HmdManagement.h"
#include "HmdTimeLib.h"
#include "HmdMonitor.h"
#include "HmdThread.h"
#include "HmdLog.h"
#include "HmdStateListen.h"
#include "HmdDbus.h"
#include <dirent.h>  /*fengwenchao add 20120228 for AXSSZFI-680*/

int HMDTipcInit(int slotid){
	int fd=-1;
	struct sockaddr_tipc server_addr;
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_TYPE;
	server_addr.addr.nameseq.lower = SERVER_BASE_INST+slotid;
	server_addr.addr.nameseq.upper = SERVER_BASE_INST+slotid;
	server_addr.scope = TIPC_ZONE_SCOPE;
	fd = socket(AF_TIPC, SOCK_RDM, 0);
	if(fd <= 0){
		hmd_syslog_err("%s,%d,fd=%d.\n",__func__,__LINE__,fd);
		return fd;
	}
	if (0 != bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		printf("Server: failed to bind port name\n");
		//exit(1);
	}
	return fd;
}
int server_to_client(int nslotid){
	struct sockaddr_tipc server_addr;
	int sd = socket (AF_TIPC, SOCK_RDM,0);
	if(sd <= 0){
		hmd_syslog_err("%s,nslotid=%d,sd=%d socket fail.\n",__func__,nslotid,sd);
		return -1;
	}else{
	}
	hmd_syslog_info("%s,nslotid=%d,sd=%d.\n",__func__,nslotid,sd);
	memset(&server_addr,0,sizeof(struct sockaddr_tipc));
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = SERVER_TYPE;
	server_addr.addr.name.name.instance = SERVER_BASE_INST+nslotid;
	server_addr.addr.name.domain = 0;

	printf("****** TIPC Server hello world program started ******\n\n");
	hmd_syslog_info("****** TIPC Server hello world program started ******\n\n");


	/* Send connectionless "hello" message: */

	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.type=	HMD_HANSI_INFO_NOTICE;
	msg.op	=	HMD_HANSI_INFO_NOTICE;
	msg.D_SlotID = HOST_SLOT_NO;
	char buf2[4096];
	memset(buf2,0,4096);
	int len = sizeof(msg);
	hmd_syslog_info("%s,len=%d.\n",__func__,len);
	memcpy(buf2,(char*)&msg,len);
	
	if (0 > sendto(sd,buf2,sizeof(msg)+1,0,
				   (struct sockaddr*)&server_addr,
				   sizeof(server_addr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	printf("\n****** TIPC Server hello program finished ******\n");
	return sd;
}

int HMDServerTipcInit(int slotid, int boardType){

	int sd = socket (AF_TIPC, SOCK_RDM,0);
	if(sd <= 0){
		hmd_syslog_err("%s,%d,sd=%d socket fail.\n",__func__,__LINE__,sd);
		return -1;
	}else{
	}
	hmd_syslog_info("%s,slotid=%d, boardType=%d,sd=%d.\n",__func__,slotid,boardType,sd);
	struct sockaddr_tipc server_addr;
	char inbuf[4096];
	memset(inbuf,0,4096);
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAMESEQ;
	server_addr.addr.nameseq.type = SERVER_TYPE;
	server_addr.addr.nameseq.lower = SERVER_BASE_INST+slotid;
	server_addr.addr.nameseq.upper = SERVER_BASE_INST+slotid;
	server_addr.scope = TIPC_ZONE_SCOPE;
	HMD_BOARD[slotid]->tipcaddr = server_addr;
	printf("****** TIPC server hello world program started ******\n\n");
	hmd_syslog_info("****** TIPC server hello world program started ******\n\n");

	/* Make server available: */

	if (0 != bind (sd, (struct sockaddr*)&server_addr,sizeof(server_addr))){
	      printf ("Server: Failed to bind port name\n");
	      //exit (1);
	}

	return sd;
	//exit(0);
	}

int HMDClientTipcInit(int slotid, int boardType)
{
	struct sockaddr_tipc server_addr;
	int sd = socket (AF_TIPC, SOCK_RDM,0);
	if(sd <= 0){
		hmd_syslog_err("%s,%d,sd=%d socket fail.\n",__func__,__LINE__,sd);
		return -1;
	}else{
	}
	hmd_syslog_info("%s,slotid=%d, boardType=%d,sd=%d.\n",__func__,slotid,boardType,sd);
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = SERVER_TYPE;
	server_addr.addr.name.name.instance = SERVER_BASE_INST+slotid;
	server_addr.addr.name.domain = 0;

	printf("****** TIPC client hello world program started ******\n\n");
	hmd_syslog_info("****** TIPC client hello world program started ******\n\n");

	return sd;
	//exit(0);
}
/*
int hmd_host_board_license_update(struct HmdMsg * tmsg){
	int i = 0;
	struct HmdMsgQ qmsg;
	memcpy(&(qmsg.mqinfo), tmsg, sizeof(struct HmdMsg));
	for(i = 0; i < MAX_INSTANCE; i++){
		if(HOST_BOARD->Hmd_Inst[i] != NULL){
			qmsg.mqid = i;
			if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
				perror("msgsnd");
		}
		if(HOST_BOARD->Hmd_Local_Inst[i] != NULL){
			qmsg.mqid = MAX_INSTANCE + i;
			if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
				perror("msgsnd");
		}
	}

}
*/
int hmd_host_board_license_update(struct HmdMsg * tmsg){
	struct HmdMsgQ qmsg;
	memcpy(&(qmsg.mqinfo), tmsg, sizeof(struct HmdMsg));
	if(tmsg->local == 0){
		if(HOST_BOARD->Hmd_Inst[tmsg->InstID] != NULL){
			qmsg.mqid = tmsg->InstID;
			if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
				perror("msgsnd");
		}
	}else{
		if(HOST_BOARD->Hmd_Local_Inst[tmsg->InstID] != NULL){
			qmsg.mqid = MAX_INSTANCE + tmsg->InstID;
			if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
				perror("msgsnd");
		}
	}
	return 0;
}
/*fengwenchao add for hmd timer config save*/
int hmd_config_save_timer_init(int method_flag)
{
	if((isMaster)&&(isActive))
	{
		char HmdDir[] = "/var/run/hmd";
		char command[128]= {0};
		sprintf(command,"%s/config_save_hmd_flag",HmdDir);
		int save_fd = 0;
		save_fd = open(command,O_RDWR|O_CREAT);
		if(save_fd <= 0)
		{
			hmd_syslog_err("%s,%d,invalid fd:%d.\n",__func__,__LINE__,save_fd);
			return 1;
		}
		switch(method_flag)
		{
			case  0:
				{
					write(save_fd, "0", 1);
					close(save_fd);
					return 0;
				}
			break;
			case 1:
				{
					write(save_fd, "1", 1);
					close(save_fd);
					return 0;
				}				
			break;
			default: 
				close(save_fd);
				break;
		}
	}
	return 1;
}
/*fengwenchao add end*/
int hmd_hansi_synchronize_request(int slotid){
	if(HMD_BOARD[slotid] == NULL){
		return 0;
	}
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.op = HMD_HANSI_INFO_SYN_REQ;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;		
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}


int syn_hansi_info_to_backup(int slotid,int profile, int neighbor_slotid,int islocaled,HmdOP op,int licenseType){
	struct sockaddr_tipc * addr = &(HMD_BOARD[neighbor_slotid]->tipcaddr);
	int fd = HMD_BOARD[neighbor_slotid]->tipcfd;
	/* Send connectionless "hello" message: */

	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	if(islocaled)
		msg.type=	HMD_LOCAL_HANSI;
	else
		msg.type=	HMD_HANSI;		
	msg.op	=	op;
	msg.D_SlotID = neighbor_slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.InstID = profile;
	if(op != HMD_HANSI_INFO_SYN_LICENSE){
		if(islocaled){
			if((profile < MAX_INSTANCE)&&(HMD_BOARD[slotid]->Hmd_Local_Inst[profile])){
				msg.u.HansiSyn.slot_num = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num;			
				if(HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num == 2){
					msg.u.HansiSyn.slot_no1 = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no1;
					msg.u.HansiSyn.InstState1 = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->InstState1;
					msg.u.HansiSyn.isActive1 = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive1;
				}
				msg.u.HansiSyn.slot_no = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_no;
				msg.u.HansiSyn.Inst_ID = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_ID;			
				msg.u.HansiSyn.InstState = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->InstState;
				msg.u.HansiSyn.isActive = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive;

				msg.u.HansiSyn.Inst_DNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_DNum;
				int dlen = sizeof(msg.u.HansiSyn.Inst_Downlink);
				memcpy(msg.u.HansiSyn.Inst_Downlink,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Downlink,dlen);

				msg.u.HansiSyn.Inst_UNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_UNum;
				int ulen = sizeof(msg.u.HansiSyn.Inst_Uplink);
				memcpy(msg.u.HansiSyn.Inst_Uplink,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Uplink,ulen);

				msg.u.HansiSyn.Inst_GNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_GNum;
				int glen = sizeof(msg.u.HansiSyn.Inst_Gateway);
				memcpy(msg.u.HansiSyn.Inst_Gateway,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Gateway,glen);

			}else{
				return HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
			}
		}else{
			if((profile < MAX_INSTANCE)&&(HMD_BOARD[slotid]->Hmd_Inst[profile])){
				
				msg.u.HansiSyn.slot_no = HMD_BOARD[slotid]->Hmd_Inst[profile]->slot_no;
				msg.u.HansiSyn.Inst_ID = HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_ID;			
				msg.u.HansiSyn.InstState = HMD_BOARD[slotid]->Hmd_Inst[profile]->InstState;
				msg.u.HansiSyn.isActive = HMD_BOARD[slotid]->Hmd_Inst[profile]->isActive;

				msg.u.HansiSyn.Inst_DNum = HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_DNum;
				int dlen = sizeof(msg.u.HansiSyn.Inst_Downlink);
				memcpy(msg.u.HansiSyn.Inst_Downlink,HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_Downlink,dlen);

				msg.u.HansiSyn.Inst_UNum = HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_UNum;
				int ulen = sizeof(msg.u.HansiSyn.Inst_Uplink);
				memcpy(msg.u.HansiSyn.Inst_Uplink,HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_Uplink,ulen);

				msg.u.HansiSyn.Inst_GNum = HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_GNum;
				int glen = sizeof(msg.u.HansiSyn.Inst_Gateway);
				memcpy(msg.u.HansiSyn.Inst_Gateway,HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_Gateway,glen);
				int hlen = sizeof(struct Inst_Interface);
				memcpy(&(msg.u.HansiSyn.Inst_Hb),&(HMD_BOARD[slotid]->Hmd_Inst[profile]->Inst_Hb),hlen);
				

			}else{
				return HMD_DBUS_HANSI_ID_NOT_EXIST;
			}
		}
	}
	else{
		if(islocaled){
			msg.u.LicenseInfo.licenseType = licenseType;
			msg.u.LicenseInfo.licenseNum = LICENSE_MGMT[licenseType].l_assigned_num[slotid][profile];
			msg.u.LicenseInfo.licenseSlotID = slotid;
		}else{
			msg.u.LicenseInfo.licenseType = licenseType;
			msg.u.LicenseInfo.licenseNum = LICENSE_MGMT[licenseType].r_assigned_num[slotid][profile];
			msg.u.LicenseInfo.licenseSlotID = slotid;
		}
	}
	char buf2[4096];
	memset(buf2,0,4096);
	int len = sizeof(msg);
	hmd_syslog_info("%s,syn %s %d-%d to slot %d\n",__func__,islocaled?"local-hansi":"hansi",slotid,profile,neighbor_slotid);
	memcpy(buf2,(char*)&msg,len);
	
	if (0 > sendto(fd,buf2,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	printf("\n****** TIPC Server hello program finished ******\n");
	return 0;
}



void * HMDManagementC(){
	hmd_pid_write_v2("HMDManagementC",HOST_SLOT_NO);
	int fd = -1;	
	int i = 0;
	char *ifname = NULL;
	int vip = 0;
	int mask = 0;
	char mac[MAC_LEN];
	struct sockaddr_tipc addr;
	socklen_t alen = sizeof(addr);	
	fd = HMDTipcInit(HOST_SLOT_NO);
	if(fd <= 0){
		hmd_syslog_err("%s,%d,fd=%d.\n",__func__,__LINE__,fd);
		return NULL;
	}
	while(1){
		char buf[4096] = {0};
		char command[128] = {0};
		struct HmdMsg *tmsg = NULL;
		struct HmdMsgQ qmsg;
		int InstID = 0;
		int ret = 0;
		int islocaled = 0;
		int LicenseType = 0;
		HMDThreadArg *arg = NULL;
		if (0 >= recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &alen)) 
		{
			perror("Server: unexpected message");
		}
		tmsg = (struct HmdMsg *)buf;
		hmd_syslog_info("tmsg->op %d\n",tmsg->op);
		switch(tmsg->op){
			case HMD_CREATE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					InstID = tmsg->InstID;
					if(tmsg->type == HMD_LOCAL_HANSI){
						if(HOST_BOARD->Hmd_Local_Inst[InstID] == NULL){
							HOST_BOARD->Hmd_Local_Inst[InstID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
							memset(HOST_BOARD->Hmd_Local_Inst[InstID],0,sizeof(struct Hmd_L_Inst_Mgmt));
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_ID = InstID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no = HOST_SLOT_NO;							
							arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
							arg->QID = MAX_INSTANCE+InstID;
							arg->islocaled = 1;
							arg->InstID = InstID;
							islocaled = 1;
							sprintf(command,"sudo /etc/init.d/wcpss start 1 %d",InstID);
							system(command);
							/*add for eag  shaojunwu 20110620*/
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/eag_modules start 1 %d",InstID);
							system(command);
							/*end for eag*/

							/* add for pppoe lixiang 20120817 */
						#ifndef _VERSION_18SP7_	
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/pppoe start 1 %d",InstID);
							system(command);
						#endif	
							/* end for pppoe */
							
							HOST_BOARD->Hmd_Local_Inst[InstID]->wid_check = 0;
							HOST_BOARD->Hmd_Local_Inst[InstID]->asd_check = 0;
							HOST_BOARD->Hmd_Local_Inst[InstID]->wsm_check = 0;
							ret = HmdCreateThread(&(HOST_BOARD->Hmd_Local_Inst[InstID]->threadID), HMDHansiMonitor, arg, 0);
							if(ret != 1){
								return NULL;
							}
							HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Local_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
						}else{

						}
					}else if(tmsg->type == HMD_HANSI){
						if(HOST_BOARD->Hmd_Inst[InstID] == NULL){
							HOST_BOARD->Hmd_Inst[InstID] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));
							memset(HOST_BOARD->Hmd_Inst[InstID],0,sizeof(struct Hmd_Inst_Mgmt));
							arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
							arg->QID = InstID;
							arg->islocaled = 0;
							arg->InstID = InstID;
							islocaled = 0;
							sprintf(command,"sudo /etc/init.d/wcpss start 0 %d",InstID);
							system(command);
							/*add for eag  shaojunwu 20110620*/
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/eag_modules start 0 %d &",InstID);
							system(command);
							/*end for eag*/	

							/* add for pppoe lixiang 20120817 */
						#ifndef _VERSION_18SP7_	
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/pppoe start 0 %d",InstID);
							system(command);
						#endif	
							/* end for pppoe */
							
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/had start %d &",InstID);
							system(command);
							HOST_BOARD->Hmd_Inst[InstID]->wid_check = 0;
							HOST_BOARD->Hmd_Inst[InstID]->asd_check = 0;
							HOST_BOARD->Hmd_Inst[InstID]->wsm_check = 0;
							/*fengwenchao add 20110928*/
							HOST_BOARD->Hmd_Inst[InstID]->Inst_ID = InstID;
							HOST_BOARD->Hmd_Inst[InstID]->slot_no = tmsg->D_SlotID;	
							/*fengwenchao add end*/
							ret = HmdCreateThread(&(HOST_BOARD->Hmd_Inst[InstID]->threadID), HMDHansiMonitor, arg, 0);
							if(ret != 1){
								return NULL;
							}
							HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
						}else{

						}
					}
				}
				break;
			case HMD_DELETE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->type == HMD_LOCAL_HANSI){
						islocaled = 1;
						InstID = tmsg->InstID;
					}else if(tmsg->type == HMD_HANSI){
						islocaled = 0;
						InstID = tmsg->InstID;
					}
					qmsg.mqid = islocaled*(MAX_INSTANCE) + InstID;
					memcpy(&(qmsg.mqinfo), buf, sizeof(struct HmdMsg));
					if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
						perror("msgsnd");
				}
				break;
			case HMD_HANSI_ENABLE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->type == HMD_LOCAL_HANSI){
						InstID = tmsg->InstID;
						if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.updateinfo.NeighborSlotID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->isActive = tmsg->u.updateinfo.isActive;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState = tmsg->u.updateinfo.state;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.updateinfo.NeighborState;
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum = tmsg->u.updateinfo.Inst_DNum;
							HOST_BOARD->Hmd_Local_Inst[InstID]->priority = tmsg->u.updateinfo.priority;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink, tmsg->u.updateinfo.Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum = tmsg->u.updateinfo.Inst_UNum;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink, tmsg->u.updateinfo.Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum = tmsg->u.updateinfo.Inst_GNum;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway, tmsg->u.updateinfo.Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							for(i = 0; i < tmsg->u.updateinfo.Inst_DNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_UNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_GNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}
							notice_wid_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							notice_eag_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
						}
					}
				}
				break;
			case HMD_HANSI_DISABLE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->type == HMD_LOCAL_HANSI){
						InstID = tmsg->InstID;
						if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
							HOST_BOARD->Hmd_Local_Inst[InstID]->isActive = tmsg->u.updateinfo.isActive;
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.updateinfo.NeighborSlotID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState = tmsg->u.updateinfo.state;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.updateinfo.NeighborState;
							notice_wid_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							notice_eag_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							for(i = 0; i < tmsg->u.updateinfo.Inst_DNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_UNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_GNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
						}
					}					
				}
				break;
			case HMD_LICENSE_UPDATE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->u.LicenseInfo.licenseType < LicenseCount){
						if(tmsg->local)
							HOST_BOARD->L_LicenseNum[tmsg->InstID][tmsg->u.LicenseInfo.licenseType] = tmsg->u.LicenseInfo.licenseNum;
						else
							HOST_BOARD->R_LicenseNum[tmsg->InstID][tmsg->u.LicenseInfo.licenseType] = tmsg->u.LicenseInfo.licenseNum;
							
						LicenseType = tmsg->u.LicenseInfo.licenseType+1;
						memset(command, 0, 128);
						sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d-%d-%d",tmsg->u.LicenseInfo.licenseNum, tmsg->local,tmsg->InstID,LicenseType);
						system(command);
						hmd_host_board_license_update(tmsg);
					}else{
						hmd_syslog_info("License Type %d large than default %d\n",tmsg->u.LicenseInfo.licenseType, LicenseCount);
					}
				}				
				break;
				/*fengwenchao add 20120228 for AXSSZFI-680*/
			case HMD_DELETE_SLOTID_AP_UPDATA_IMG:
				hmd_syslog_info("accessinto %s \n",__func__);
				hmd_syslog_info("tmsg->S_SlotID %d \n",tmsg->S_SlotID);
				if((HOST_BOARD != NULL)&&(HMD_BOARD[tmsg->S_SlotID] != NULL ))
				{
					char *ap_updata_img = NULL;
					ap_updata_img = (char *)malloc(DEFAULT_LEN);
					
					hmd_syslog_info("tmsg->slot_ap_updata_img  = %s \n",tmsg->slot_ap_updata_img);
					memset(ap_updata_img,0,DEFAULT_LEN);
					memcpy(ap_updata_img,tmsg->slot_ap_updata_img,strlen(tmsg->slot_ap_updata_img));

					char *dir = "/mnt/wtp";
					hmd_syslog_info("ap_updata_img  = %s \n",ap_updata_img);
					char command[DEFAULT_LEN] = {0};
					sprintf(command,"rm /mnt/wtp/%s",ap_updata_img);
					hmd_syslog_info("command  = %s \n",command);
					DIR *dp = NULL;
					struct dirent *dirp;
					dp = opendir(dir);
					if(dp != NULL)
					{					
					hmd_syslog_info(" dp  != NULL\n");
						while((dirp = readdir(dp)) != NULL)
						{
							hmd_syslog_info(" dirp  != NULL\n");
							hmd_syslog_info(" dirp->d_name  = %s ,ap_updata_img= %s\n",dirp->d_name,ap_updata_img);
							if((memcmp(dirp->d_name,ap_updata_img,strlen(ap_updata_img))) ==  0)
							{
								system(command);
								break;
							}
						}
						closedir(dp);
					}

					if(ap_updata_img)
					{
						free(ap_updata_img);
						ap_updata_img = NULL;
					}
				}
				hmd_syslog_info("!!!over   %s \n",__func__);
				break;
				/*fengwenchao add end*/
				/*fengwenchao copy from 1318 for AXSSZFI-839*/
				case HMD_CLEAR_APPLY_IFNAME_FLAG:
				//hmd_syslog_info("accessinto %s \n",__func__);
				//hmd_syslog_info("tmsg->S_SlotID %d \n",tmsg->S_SlotID);	
				if((HOST_BOARD != NULL)&&(HMD_BOARD[tmsg->S_SlotID] != NULL ))
				{
					char *ifname = NULL;
					if(tmsg->clear_ifname != NULL)
					{
						ifname = (char *)malloc(strlen(tmsg->clear_ifname)+1);
						memset(ifname,0,strlen(tmsg->clear_ifname)+1);
						memcpy(ifname,tmsg->clear_ifname,strlen(tmsg->clear_ifname));
						//hmd_syslog_info("ifname = %s \n");
						Set_Interface_binding_Info(ifname,0);
					}

					if(ifname)
					{
						free(ifname);
						ifname = NULL;
					}
				}
				break;
				/*fengwenchao copy end*/
				case HMD_BAKUP_FOREVER_CONFIG:
					if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
						Set_hmd_bakup_foreve_config(tmsg);
					}
				break;
				case HMD_CREATE_FOR_DHCP:
					DHCP_RESTART_FLAG = DHCP_RESTART_ENABLE;
					DHCP_MONITOR = (struct Hmd_For_Dhcp_restart*)malloc(sizeof(struct Hmd_For_Dhcp_restart));
					arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
					arg->QID = DHCP_QID;
					arg->islocaled = 0;
					arg->InstID = tmsg->InstID;
					ret = HmdCreateThread(&(DHCP_MONITOR->dhcp_monitor), HMDHansiMonitor, arg, 0);	
					if(ret != 1){
						return NULL;
					}
					HMDTimerRequest(HMD_CHECKING_TIMER,&(DHCP_MONITOR->HmdTimerID), HMD_CHECK_FOR_DHCP, -1, 0);
				break;
				case HMD_CREATE_FOR_DHCP_DISABLE:
					DHCP_RESTART_FLAG = DHCP_RESTART_DISABLE;
				break;
				case HMD_CREATE_FOR_DHCP_ENABLE:
					DHCP_RESTART_FLAG = DHCP_RESTART_ENABLE;
				break;
				case HMD_STATE_SWITCH:
				break;
				case HMD_HANSI_INFO_NOTICE:
				break;
				case HMD_HANSI_CHECKING:
				break;
				case HMD_RESTART:
				break;
				case HMD_HANSI_UPDATE:
				break;
				case HMD_LICENSE_SYNCHRONIZE:
				break;
				case HMD_HANSI_INFO_SYN_REQ:
				break;
				case HMD_HANSI_INFO_SYN_ADD:
				break;
				case HMD_HANSI_INFO_SYN_DEL:
				break;
				case HMD_HANSI_INFO_SYN_MODIFY:
				break;
				case HMD_HANSI_INFO_SYN_LICENSE:
				break;
				case HMD_NOTICE_VRRP_STATE_CHANGE_MASTER:
				case HMD_NOTICE_VRRP_STATE_CHANGE_BAK:
					//hmd_syslog_info("tmsg->type is %d HMD_LOCAL_HANSI is %d\n",(tmsg->type),HMD_LOCAL_HANSI);

					if(tmsg->type != HMD_LOCAL_HANSI)
					{
						//hmd_syslog_info("hmd management C\n");
						notice_had_to_change_vrrp_state(tmsg->InstID,tmsg->op);
						hmd_syslog_info("global switch -->set hansi %d-%d vrrp state  to %s\n",HOST_SLOT_NO,tmsg->InstID,\
								(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
					}
				break;
				default :
				break;
					
		}		
	}
	
	return NULL;
}

void * HMDManagementS(){
	hmd_pid_write_v2("HMDManagementS",HOST_SLOT_NO);
	int fd = -1;
	struct sockaddr_tipc addr;
	socklen_t alen = sizeof(addr);	
	char buf[4096];
	char command[128];
	struct HmdMsg *tmsg = NULL;
	struct HmdMsgQ qmsg;
	int InstID = 0;
	int depend_s_slot_id = 0;
    int depend_s_inst_id = 0;
	int depend_d_slot_id = 0;
    int depend_d_inst_id = 0;
	int ret = 0;
	int islocaled = 0;
	int LicenseType = 0;
	int LicenseNum = 0;
	int num = 0;
	int num1 = 0;
	HMDThreadArg *arg = NULL;
	int SlotID = 0;
	int i = 0;
	int j = 0;
	char *ifname = NULL;
	int vip = 0;
	int mask = 0;
	int slot_no1 = 0;
	char mac[MAC_LEN];
	char defaultPath[] = "/var/run/config/Instconfig";
	char defaultSlotPath[] = "/var/run/config/slot";
	//char newSlotPath[128] = {0};
	unsigned int old_change_state = 0;
	unsigned int new_change_state = 0;
	int slot_no = 0,inst_id = 0;
	
	fd = HMDTipcInit(HOST_SLOT_NO);
	if(fd <= 0){
		hmd_syslog_err("%s,%d,fd=%d.\n",__func__,__LINE__,fd);
		return NULL;
	}
	while(1){
		if (0 >= recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &alen)) 
		{
			perror("Server: unexpected message");
		}
		tmsg = (struct HmdMsg *)buf;
		hmd_syslog_info("tmsg->op %d\n",tmsg->op);
		switch(tmsg->op){
			case HMD_STATE_SWITCH:
				SlotID = tmsg->D_SlotID;
				InstID = tmsg->InstID;
				if(tmsg->type == HMD_LOCAL_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] != NULL){
							HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->InstState = tmsg->u.statechange.nowstate;
						}
					}
				}else if(tmsg->type == HMD_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] != NULL){
							HMD_BOARD[SlotID]->Hmd_Inst[InstID]->InstState = tmsg->u.statechange.nowstate;
						}
					}
				}
				break;
			case HMD_RESTART:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				if(tmsg->type == HMD_LOCAL_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] != NULL){
							if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no)
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->RestartTimes += 1;
							else if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no1)
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->RestartTimes1 += 1;	
							#if 0
							memset(buf, 0, 128);
							sprintf(buf,"mv /mnt/cli.conf /mnt/cli.conf_bak");
							system(buf);				
							memset(newpath, 0, 128);
							sprintf(newpath,"cp %s%d-1-%d /mnt/cli.conf",defaultPath,SlotID,InstID);
							system(newpath);
							sleep(1);
							#endif
							memset(buf, 0, 128);
							sprintf(buf,"/opt/bin/vtysh -f %s%d-1-%d -b &",defaultPath,SlotID,InstID);
							system(buf);
//							memset(buf, 0, 128);
//							sprintf(buf,"mv /mnt/cli.conf_bak /mnt/cli.conf");
//							system(buf);
						}
					}					
				}else if(tmsg->type == HMD_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] != NULL){
							HMD_BOARD[SlotID]->Hmd_Inst[InstID]->RestartTimes += 1;							
							#if 0
							memset(buf, 0, 128);
							sprintf(buf,"mv /mnt/cli.conf /mnt/cli.conf_bak");
							system(buf);				
							memset(newpath, 0, 128);
							sprintf(newpath,"cp %s%d-0-%d /mnt/cli.conf",defaultPath,SlotID,InstID);
							system(newpath);
							sleep(1);
							#endif
							memset(buf, 0, 128);
							sprintf(buf,"/opt/bin/vtysh -f %s%d-0-%d -b &",defaultPath,SlotID,InstID);
							system(buf);
//							memset(buf, 0, 128);
//							sprintf(buf,"mv /mnt/cli.conf_bak /mnt/cli.conf");
//							system(buf);
						}
					}
				}
				break;	
			case HMD_CREATE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					InstID = tmsg->InstID;
					if(tmsg->type == HMD_LOCAL_HANSI){
						if(HOST_BOARD->Hmd_Local_Inst[InstID] == NULL){
							hmd_pid_write_v3(HOST_SLOT_NO,InstID,1);
							if(HMD_L_HANSI[InstID] == NULL){
								HMD_L_HANSI[InstID] = (struct Hmd_L_Inst_Mgmt_Summary *)malloc(sizeof(struct Hmd_L_Inst_Mgmt_Summary));
								memset(HMD_L_HANSI[InstID], 0 ,sizeof(struct Hmd_L_Inst_Mgmt_Summary));
								HMD_L_HANSI[InstID]->slot_num = 1;
								HMD_L_HANSI[InstID]->slot_no = HOST_SLOT_NO;
								HMD_L_HANSI[InstID]->Inst_ID = InstID;
							}else{
								if(HMD_L_HANSI[InstID]->slot_num == 1){
									if(HMD_L_HANSI[InstID]->slot_no == 0){
										HMD_L_HANSI[InstID]->slot_no = HOST_SLOT_NO;
									}else{
										HMD_L_HANSI[InstID]->slot_no1 = HOST_SLOT_NO;										
									}
								}else{
									hmd_syslog_info("%s something wrong InstID %d slot %d slot1 %d\n",InstID,HMD_L_HANSI[InstID]->slot_no,HMD_L_HANSI[InstID]->slot_no1);
								}
							}
							
							HOST_BOARD->Hmd_Local_Inst[InstID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
							memset(HOST_BOARD->Hmd_Local_Inst[InstID],0,sizeof(struct Hmd_L_Inst_Mgmt));
							arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
							arg->QID = MAX_INSTANCE+InstID;
							arg->islocaled = 1;
							arg->InstID = InstID;
							islocaled = 1;
							sprintf(command,"sudo /etc/init.d/wcpss start 1 %d",InstID);
							system(command);
							/*add for eag  shaojunwu 20110620*/
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/eag_modules start 1 %d",InstID);
							system(command);
							/*end for eag*/	

							/* add for pppoe lixiang 20120817 */
						#ifndef _VERSION_18SP7_	
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/pppoe start 1 %d",InstID);
							system(command);
						#endif	
							/* end for pppoe */
							
							HOST_BOARD->Hmd_Local_Inst[InstID]->wid_check = 0;
							HOST_BOARD->Hmd_Local_Inst[InstID]->asd_check = 0;
							HOST_BOARD->Hmd_Local_Inst[InstID]->wsm_check = 0;
							/*fengwenchao add 20110928*/
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_ID = InstID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no = tmsg->D_SlotID;	
							/*fengwenchao add end*/					
							ret = HmdCreateThread(&(HOST_BOARD->Hmd_Local_Inst[InstID]->threadID), HMDHansiMonitor, arg, 0);
							if(ret != 1){
								HMD_FREE_OBJECT(arg);
								hmd_syslog_err("%s,%d,create thread fail.\n",__func__,__LINE__);
								continue;
								//return NULL;
							}
							HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Local_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
						}else{

						}
					}else if(tmsg->type == HMD_HANSI){
						if(HOST_BOARD->Hmd_Inst[InstID] == NULL){
							hmd_pid_write_v3(HOST_SLOT_NO,InstID,0);
							HOST_BOARD->Hmd_Inst[InstID] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));
							memset(HOST_BOARD->Hmd_Inst[InstID],0,sizeof(struct Hmd_Inst_Mgmt));
							arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
							arg->QID = InstID;
							arg->islocaled = 0;
							arg->InstID = InstID;
							islocaled = 0;
							sprintf(command,"sudo /etc/init.d/wcpss start 0 %d",InstID);
							system(command);
							/*add for eag  shaojunwu 20110620*/
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/eag_modules start 0 %d &",InstID);
							system(command);
							/*end for eag*/	

							/* add for pppoe lixiang 20120817 */
						#ifndef _VERSION_18SP7_	
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/pppoe start 0 %d",InstID);
							system(command);
						#endif
							/* end for pppoe */
							
							memset(command, 0, 128);
							sprintf(command,"sudo /etc/init.d/had start %d &",InstID);
							system(command);
							
							HOST_BOARD->Hmd_Inst[InstID]->wid_check = 0;
							HOST_BOARD->Hmd_Inst[InstID]->asd_check = 0;
							HOST_BOARD->Hmd_Inst[InstID]->wsm_check = 0;
							/*fengwenchao add 20110928*/
							HOST_BOARD->Hmd_Inst[InstID]->Inst_ID = InstID;
							HOST_BOARD->Hmd_Inst[InstID]->slot_no = tmsg->D_SlotID;	
							/*fengwenchao add end*/							
							ret = HmdCreateThread(&(HOST_BOARD->Hmd_Inst[InstID]->threadID), HMDHansiMonitor, arg, 0);
							if(ret != 1){
								HMD_FREE_OBJECT(arg);
								hmd_syslog_err("%s,%d,create thread fail.\n",__func__,__LINE__);
								continue;
								//return NULL;
							}
							HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
						}else{

						}
					}
				}
				break;
			case HMD_DELETE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->type == HMD_LOCAL_HANSI){
						islocaled = 1;
						InstID = tmsg->InstID;
					}else if(tmsg->type == HMD_HANSI){
						islocaled = 0;
						InstID = tmsg->InstID;
					}
					memset(command, 0, 128);
					sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
								islocaled, HOST_SLOT_NO,InstID, ".pid");
					system(command);
					memset(&qmsg,0,sizeof(struct HmdMsgQ));
					qmsg.mqid = islocaled*(MAX_INSTANCE) + InstID;
					memcpy(&(qmsg.mqinfo), buf, sizeof(struct HmdMsg));
					if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&qmsg, sizeof(qmsg.mqinfo), 0) == -1)
						perror("msgsnd");
				}
				break;
			case HMD_HANSI_ENABLE:
				if((HOST_BOARD != NULL)/*&&(MASTER_SLOT_NO == tmsg->S_SlotID)*/){
					if(tmsg->type == HMD_LOCAL_HANSI){
						InstID = tmsg->InstID;
						if(HMD_L_HANSI[InstID]!= NULL){
							HMD_L_HANSI[InstID]->Inst_ID = InstID;
							HMD_L_HANSI[InstID]->slot_no = HOST_SLOT_NO;
							HMD_L_HANSI[InstID]->slot_no1 =  tmsg->u.updateinfo.NeighborSlotID;
							HMD_L_HANSI[InstID]->InstState =  tmsg->u.updateinfo.state;
							HMD_L_HANSI[InstID]->isActive =  tmsg->u.updateinfo.isActive;
							HMD_L_HANSI[InstID]->InstState1 =  tmsg->u.updateinfo.NeighborState;
							HMD_L_HANSI[InstID]->isActive1 =  tmsg->u.updateinfo.NeighborActive;
							HMD_L_HANSI[InstID]->slot_num=  tmsg->u.updateinfo.slot_num;
						}
						if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){							
							if(tmsg->u.updateinfo.slot_num == 2){
								slot_no1 = tmsg->u.updateinfo.NeighborSlotID;
								if((HMD_BOARD[slot_no1] != NULL)&&(HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID] != NULL)){
									HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->slot_num= tmsg->u.updateinfo.slot_num;
									HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->slot_no1 = HOST_SLOT_NO;
									HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->InstState1 =  tmsg->u.updateinfo.state;
									HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->isActive1 =  tmsg->u.updateinfo.isActive;
								}
							}
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.updateinfo.NeighborSlotID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->isActive1 = tmsg->u.updateinfo.NeighborActive;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.updateinfo.NeighborState;
							HOST_BOARD->Hmd_Local_Inst[InstID]->isActive = tmsg->u.updateinfo.isActive;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState = tmsg->u.updateinfo.state;
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_num = tmsg->u.updateinfo.slot_num;
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum = tmsg->u.updateinfo.Inst_DNum;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink, tmsg->u.updateinfo.Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum = tmsg->u.updateinfo.Inst_UNum;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink, tmsg->u.updateinfo.Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum = tmsg->u.updateinfo.Inst_GNum;
							memset(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway, tmsg->u.updateinfo.Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							for(i = 0; i < tmsg->u.updateinfo.Inst_DNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_UNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_GNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mask;
								hmd_ifname_to_mac(ifname,mac);
								memcpy(HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mac,mac,MAC_LEN);
								if(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE){
									hmd_ipaddr_op_withmask(ifname,vip,mask,1);
									send_tunnel_interface_arp(mac,vip,ifname);	
								}else{
									hmd_ipaddr_op_withmask(ifname,vip,mask,0);
								}
							}

							notice_wid_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							notice_eag_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
						}
					}
				}
				break;
			case HMD_HANSI_DISABLE:
				if((HOST_BOARD != NULL)/*&&(MASTER_SLOT_NO == tmsg->S_SlotID)*/){
					if(tmsg->type == HMD_LOCAL_HANSI){
						InstID = tmsg->InstID;
						if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
							HOST_BOARD->Hmd_Local_Inst[InstID]->isActive = tmsg->u.updateinfo.isActive;
							HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.updateinfo.NeighborSlotID;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState = tmsg->u.updateinfo.state;
							HOST_BOARD->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.updateinfo.NeighborState;
							notice_wid_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							notice_eag_local_hansi_service_change_state(InstID,tmsg->u.updateinfo.NeighborSlotID);
							for(i = 0; i < tmsg->u.updateinfo.Inst_DNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_UNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
							for(i = 0; i < tmsg->u.updateinfo.Inst_GNum; i++){
								ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname;
								vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].vir_ip;
								mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].mask;
								hmd_ipaddr_op_withmask(ifname,vip,mask,0);
							}
						}
					}					
				}
				break;
			case HMD_LICENSE_UPDATE:
				if((HOST_BOARD != NULL)&&(MASTER_SLOT_NO == tmsg->S_SlotID)){
					if(tmsg->u.LicenseInfo.licenseType < LicenseCount){
						if(tmsg->local)
							HOST_BOARD->L_LicenseNum[tmsg->InstID][tmsg->u.LicenseInfo.licenseType] = tmsg->u.LicenseInfo.licenseNum;
						else
							HOST_BOARD->R_LicenseNum[tmsg->InstID][tmsg->u.LicenseInfo.licenseType] = tmsg->u.LicenseInfo.licenseNum;							
						LicenseType = tmsg->u.LicenseInfo.licenseType+1;
						memset(command, 0, 128);
						sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d-%d-%d",tmsg->u.LicenseInfo.licenseNum, tmsg->local,tmsg->InstID,LicenseType);
						system(command);
						hmd_host_board_license_update(tmsg);
					}else{
						hmd_syslog_info("License Type %d large than default %d\n",tmsg->u.LicenseInfo.licenseType, LicenseCount);
					}
				}				
				break;
			case HMD_HANSI_UPDATE:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				if((HMD_BOARD[SlotID] != NULL)){
					if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] != NULL){
						HMD_BOARD[SlotID]->Hmd_Inst[InstID]->InstState = tmsg->u.updateinfo.state;
						HMD_BOARD[SlotID]->Hmd_Inst[InstID]->isActive = tmsg->u.updateinfo.isActive;
						HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_DNum = tmsg->u.updateinfo.Inst_DNum;
						memcpy(&(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Hb), &(tmsg->u.updateinfo.Inst_Hb),sizeof(struct Inst_Interface));
						memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Downlink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
						memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Downlink, tmsg->u.updateinfo.Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
						HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_UNum = tmsg->u.updateinfo.Inst_UNum;
						memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Uplink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
						memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Uplink, tmsg->u.updateinfo.Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
						HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_GNum = tmsg->u.updateinfo.Inst_GNum;
						memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Gateway, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
						memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Gateway, tmsg->u.updateinfo.Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
					}
				}
				break;
			case HMD_LICENSE_SYNCHRONIZE:
				hmd_syslog_info("HMD_LICENSE_SYNCHRONIZE LicenseType %d LicenseNum %d\n",tmsg->u.LicenseInfo.licenseType,tmsg->u.LicenseInfo.licenseNum);
				if(tmsg->u.LicenseInfo.licenseType < LicenseCount){
					LicenseType = tmsg->u.LicenseInfo.licenseType;
					LicenseNum = tmsg->u.LicenseInfo.licenseNum;
					if(isActive){
						if((memcmp(LICENSE_MGMT[LicenseType].licreq, tmsg->u.LicenseInfo.licreq, LICREQ_LEN) != 0)&&(memcmp(LICENSE_MGMT[LicenseType].licreq2, tmsg->u.LicenseInfo.licreq, LICREQ_LEN) != 0)){							
							hmd_syslog_info("HMD_LICENSE_SYNCHRONIZE licreq %s:%s\n",tmsg->u.LicenseInfo.licreq,LICENSE_MGMT[LicenseType].licreq);							
							LICENSE_MGMT[LicenseType].total_num += tmsg->u.LicenseInfo.licenseNum;
							LICENSE_MGMT[LicenseType].free_num += tmsg->u.LicenseInfo.licenseNum;
							memcpy(LICENSE_MGMT[LicenseType].licreq2, tmsg->u.LicenseInfo.licreq, LICREQ_LEN);
						}else{
							hmd_syslog_info("HMD_LICENSE_SYNCHRONIZE same licreq %s:%s\n",tmsg->u.LicenseInfo.licreq,LICENSE_MGMT[LicenseType].licreq);							
						}
					}else if(isMaster){
						num = LICENSE_MGMT[LicenseType].total_num - LICENSE_MGMT[LicenseType].free_num;
						LICENSE_MGMT[LicenseType].total_num = tmsg->u.LicenseInfo.licenseNum;
						LICENSE_MGMT[LicenseType].free_num = tmsg->u.LicenseInfo.licenseNum - num;
						memcpy(LICENSE_MGMT[LicenseType].licreq2, tmsg->u.LicenseInfo.licreq, LICREQ_LEN);
					}
				}
				break;
			case HMD_HANSI_INFO_SYN_REQ:
				hmd_syslog_info("HMD_HANSI_INFO_SYN_REQ backup slot %d\n",tmsg->S_SlotID);
				if((tmsg->S_SlotID >= MAX_SLOT_NUM) || (HMD_BOARD[tmsg->S_SlotID] == NULL))
					break;
				MASTER_BACKUP_SLOT_NO = tmsg->S_SlotID;
				for(i = 1; i < MAX_SLOT_NUM; i++){
					if((i != MASTER_BACKUP_SLOT_NO)&&(HMD_BOARD[i] != NULL)){
						for(j = 1; j < MAX_INSTANCE; j++){
							if(HMD_BOARD[i]->Hmd_Inst[i] != NULL){
								syn_hansi_info_to_backup(i,j,MASTER_BACKUP_SLOT_NO,0,HMD_HANSI_INFO_SYN_ADD,0);
							}
							if(HMD_BOARD[i]->Hmd_Local_Inst[i] != NULL){
								syn_hansi_info_to_backup(i,j,MASTER_BACKUP_SLOT_NO,1,HMD_HANSI_INFO_SYN_ADD,0);
							}
						}
					}
				}
				sleep(1);
				for(i = 0;i < LicenseCount; i++){
					if(LICENSE_MGMT[i].total_num != 0){
						hmd_license_synchronize(MASTER_BACKUP_SLOT_NO,i,LICENSE_MGMT[i].total_num);
					}
				}
				break;
			case HMD_HANSI_INFO_SYN_ADD:
				hmd_syslog_info("HMD_HANSI_INFO_SYN_ADD %d-%d\n",tmsg->u.HansiSyn.slot_no,tmsg->InstID);
			case HMD_HANSI_INFO_SYN_MODIFY:
				hmd_syslog_info("HMD_HANSI_INFO_SYN_MODIFY %d-%d\n",tmsg->u.HansiSyn.slot_no,tmsg->InstID);
				SlotID = tmsg->u.HansiSyn.slot_no;
				InstID = tmsg->InstID;
				hmd_syslog_info("HMD_HANSI_INFO_SYN_ADD backup slot %d\n",tmsg->S_SlotID);
				if(tmsg->S_SlotID == MASTER_SLOT_NO){
					if((SlotID < MAX_SLOT_NUM)&&(HMD_BOARD[SlotID])){
						if(tmsg->u.HansiSyn.Inst_ID < MAX_INSTANCE){
							if(tmsg->type == HMD_LOCAL_HANSI){								
								hmd_pid_write_v3(SlotID,InstID,1);
								if(HMD_L_HANSI[InstID] == NULL){
									HMD_L_HANSI[InstID] = (struct Hmd_L_Inst_Mgmt_Summary *)malloc(sizeof(struct Hmd_L_Inst_Mgmt_Summary));
									memset(HMD_L_HANSI[InstID], 0 ,sizeof(struct Hmd_L_Inst_Mgmt_Summary));
								}
								if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] == NULL){
									HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
									memset(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID], 0, sizeof(struct Hmd_L_Inst_Mgmt));									
								}
								HMD_L_HANSI[InstID]->Inst_ID = InstID;
								HMD_L_HANSI[InstID]->slot_no = tmsg->u.HansiSyn.slot_no;
								HMD_L_HANSI[InstID]->slot_no1 = tmsg->u.HansiSyn.slot_no1;
								HMD_L_HANSI[InstID]->InstState = tmsg->u.HansiSyn.InstState;
								HMD_L_HANSI[InstID]->isActive = tmsg->u.HansiSyn.isActive;
								HMD_L_HANSI[InstID]->InstState1 = tmsg->u.HansiSyn.InstState1;
								HMD_L_HANSI[InstID]->isActive1 = tmsg->u.HansiSyn.isActive1;
								HMD_L_HANSI[InstID]->slot_num= tmsg->u.HansiSyn.slot_num;
								if(tmsg->u.HansiSyn.slot_num == 2){
									slot_no1 = tmsg->u.HansiSyn.slot_no1;
									if((HMD_BOARD[slot_no1] != NULL)&&(HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID] != NULL)){
										HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->slot_num= tmsg->u.HansiSyn.slot_num;
										HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.HansiSyn.slot_no;
										HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.HansiSyn.InstState;
										HMD_BOARD[slot_no1]->Hmd_Local_Inst[InstID]->isActive1 = tmsg->u.HansiSyn.isActive;
									}
								}
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_ID = InstID;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_num= tmsg->u.HansiSyn.slot_num;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no = tmsg->u.HansiSyn.slot_no;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no1 = tmsg->u.HansiSyn.slot_no1;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->InstState = tmsg->u.HansiSyn.InstState;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->isActive = tmsg->u.HansiSyn.isActive;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->InstState1 = tmsg->u.HansiSyn.InstState1;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->isActive1 = tmsg->u.HansiSyn.isActive1;
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_DNum = tmsg->u.HansiSyn.Inst_DNum;
								memset(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Downlink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Downlink, tmsg->u.HansiSyn.Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_UNum = tmsg->u.HansiSyn.Inst_UNum;
								memset(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Uplink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Uplink, tmsg->u.HansiSyn.Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_GNum = tmsg->u.HansiSyn.Inst_GNum;
								memset(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Gateway, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->Inst_Gateway, tmsg->u.HansiSyn.Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							}else if(tmsg->type == HMD_HANSI){
								hmd_pid_write_v3(SlotID,InstID,0);
								if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] == NULL){
									HMD_BOARD[SlotID]->Hmd_Inst[InstID] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));
									memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID], 0, sizeof(struct Hmd_Inst_Mgmt));									
								}
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_ID = InstID;
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->slot_no = tmsg->u.HansiSyn.slot_no;
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->InstState = tmsg->u.HansiSyn.InstState;
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->isActive = tmsg->u.HansiSyn.isActive;
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_DNum = tmsg->u.HansiSyn.Inst_DNum;
								memcpy(&(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Hb), &(tmsg->u.HansiSyn.Inst_Hb),sizeof(struct Inst_Interface));
								memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Downlink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Downlink, tmsg->u.HansiSyn.Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_UNum = tmsg->u.HansiSyn.Inst_UNum;
								memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Uplink, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Uplink, tmsg->u.HansiSyn.Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_GNum = tmsg->u.HansiSyn.Inst_GNum;
								memset(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Gateway, 0, MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
								memcpy(HMD_BOARD[SlotID]->Hmd_Inst[InstID]->Inst_Gateway, tmsg->u.HansiSyn.Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
							}
						}
					}
				}
				break;
			case HMD_HANSI_INFO_SYN_DEL:
				
				SlotID = tmsg->u.HansiSyn.slot_no;
				InstID = tmsg->InstID;
				if(tmsg->S_SlotID == MASTER_SLOT_NO){
					if((SlotID < MAX_SLOT_NUM)&&(HMD_BOARD[SlotID])){
						if(tmsg->u.HansiSyn.Inst_ID < MAX_INSTANCE){
							if(tmsg->type == HMD_LOCAL_HANSI){
								if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]){
									if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_num == 2){
										int slotid1 = HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no1;
										if((HMD_BOARD[slotid1] != NULL)&&(HMD_BOARD[slotid1]->Hmd_Local_Inst[InstID] != NULL)){
											HMD_BOARD[slotid1]->Hmd_Local_Inst[InstID]->slot_num = 1;
											HMD_BOARD[slotid1]->Hmd_Local_Inst[InstID]->slot_no1 = 0;
											HMD_BOARD[slotid1]->Hmd_Local_Inst[InstID]->InstState1 = 0;
											HMD_BOARD[slotid1]->Hmd_Local_Inst[InstID]->isActive1 = 0;
										}
									}
									
									memset(command, 0, 128);
									sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
												1, SlotID,InstID, ".pid");
									system(command);
									free(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]);
									HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] = NULL;
								}
							}else if(tmsg->type == HMD_HANSI){
								if(HMD_BOARD[SlotID]->Hmd_Inst[InstID]){									
									memset(command, 0, 128);
									sprintf(command,"rm %s%d-%d-%d%s", "/var/run/hmd/hmd", \
												0, SlotID,InstID, ".pid");
									system(command);
									free(HMD_BOARD[SlotID]->Hmd_Inst[InstID]);
									HMD_BOARD[SlotID]->Hmd_Inst[InstID] = NULL;
								}
							}
						}
					}
				}
				break;
			case HMD_HANSI_INFO_SYN_LICENSE:				
				SlotID = tmsg->u.LicenseInfo.licenseSlotID;
				InstID = tmsg->InstID;
				LicenseType = tmsg->u.LicenseInfo.licenseType;
				num = tmsg->u.LicenseInfo.licenseNum;
				if(tmsg->type == HMD_LOCAL_HANSI){
					islocaled = 1;
				}else{
					islocaled = 0;
				}
				if(HMD_BOARD[SlotID] != NULL){
					if(islocaled){
						LicenseNum = HMD_BOARD[SlotID]->L_LicenseNum[InstID][LicenseType];
					}else{
						LicenseNum = HMD_BOARD[SlotID]->R_LicenseNum[InstID][LicenseType];
					}
					if(LicenseNum < num){
						num1 = num - LicenseNum;
						if(LICENSE_MGMT[LicenseType].free_num >= num1){
							LICENSE_MGMT[LicenseType].free_num = LICENSE_MGMT[LicenseType].free_num - num1;
							if(islocaled){
								LICENSE_MGMT[LicenseType].l_assigned_num[SlotID][InstID] = LICENSE_MGMT[LicenseType].l_assigned_num[SlotID][InstID] + num1;
								HMD_BOARD[SlotID]->L_LicenseNum[InstID][LicenseType] = num;
							}else{
								LICENSE_MGMT[LicenseType].r_assigned_num[SlotID][InstID] = LICENSE_MGMT[LicenseType].r_assigned_num[SlotID][InstID] + num1;
								HMD_BOARD[SlotID]->R_LicenseNum[InstID][LicenseType] = num;
							}
						}
					}
					else if(LicenseNum > num){
						num1 = LicenseNum - num;							
						LICENSE_MGMT[LicenseType].free_num = LICENSE_MGMT[LicenseType].free_num + num1;
						if(islocaled){
							LICENSE_MGMT[LicenseType].l_assigned_num[SlotID][InstID] = LICENSE_MGMT[LicenseType].l_assigned_num[SlotID][InstID] - num1;
							HMD_BOARD[SlotID]->L_LicenseNum[InstID][LicenseType] = num;
						}else{
							LICENSE_MGMT[LicenseType].r_assigned_num[SlotID][InstID] = LICENSE_MGMT[LicenseType].r_assigned_num[SlotID][InstID] - num1;
							HMD_BOARD[SlotID]->R_LicenseNum[InstID][LicenseType] = num;
						}
					}
				}
				break;
			case HMD_SERVER_DELETE_HANSI:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				if(tmsg->type == HMD_LOCAL_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] != NULL){
							if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no){
								free(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]);
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] = NULL;	
							}
							else if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no1){
								free(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]);
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] = NULL;								}
							}
					}					
				}else if(tmsg->type == HMD_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] != NULL){
							free(HMD_BOARD[SlotID]->Hmd_Inst[InstID]);
							HMD_BOARD[SlotID]->Hmd_Inst[InstID] = NULL;
							hmd_syslog_info("%s,%d\n",__func__,__LINE__);
						}
					}
				}
				break;	
			case HMD_IS_DELETE_HANSI:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				if(tmsg->type == HMD_LOCAL_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID] != NULL){
							if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no){
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->delete_flag = 1;
							}
							else if(SlotID == HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->slot_no1){
								HMD_BOARD[SlotID]->Hmd_Local_Inst[InstID]->delete_flag = 1;
							}
						}
					}					
				}else if(tmsg->type == HMD_HANSI){
					if(HMD_BOARD[SlotID] != NULL){
						if(HMD_BOARD[SlotID]->Hmd_Inst[InstID] != NULL){
							HMD_BOARD[SlotID]->Hmd_Inst[InstID]->delete_flag = 1;
						}
					}
				}
				break;
			case HMD_DHCP_TO_START:
				//hmd_syslog_info("HMD_DHCP_TO_START~~~~\n");
				SlotID = tmsg->S_SlotID;
				memset(buf, 0, 128);	
				sprintf(buf,"sudo /opt/bin/vtysh -f %s%d/%s -b &", defaultSlotPath,SlotID,"dhcp_poll");
				system(buf);
				for(InstID = 0;InstID <=16;InstID++){
				memset(buf, 0, 128);
				sprintf(buf,"sudo /opt/bin/vtysh -f %s%d/%s%d -b &", defaultSlotPath,SlotID,"hansi_dhcp",InstID);
				system(buf);
				}
				break;
			case HMD_CREATE_FOR_DHCP:
					DHCP_RESTART_FLAG = DHCP_RESTART_ENABLE;
					DHCP_MONITOR = (struct Hmd_For_Dhcp_restart*)malloc(sizeof(struct Hmd_For_Dhcp_restart));
					arg = (HMDThreadArg *)malloc(sizeof(HMDThreadArg));
					arg->QID = DHCP_QID;
					arg->islocaled = 0;
					arg->InstID = tmsg->InstID;
					ret = HmdCreateThread(&(DHCP_MONITOR->dhcp_monitor), HMDHansiMonitor, arg, 0);	
					if(ret != 1){
						return NULL;
					}
					HMDTimerRequest(HMD_CHECKING_TIMER,&(DHCP_MONITOR->HmdTimerID), HMD_CHECK_FOR_DHCP, -1, 0);
				break;
			case HMD_CREATE_FOR_DHCP_DISABLE:
					DHCP_RESTART_FLAG = DHCP_RESTART_DISABLE;
				break;
			case HMD_CREATE_FOR_DHCP_ENABLE:
					DHCP_RESTART_FLAG = DHCP_RESTART_ENABLE;
				break;
			case HMD_NOTICE_VRRP_STATE_CHANGE_MASTER:
			case HMD_NOTICE_VRRP_STATE_CHANGE_BAK:
				InstID = tmsg->InstID;
			
				if((HOST_SLOT_NO != MASTER_SLOT_NO) && (isMaster) && (isActive==0))
				{
					/*for 7605I*/
					hmd_syslog_info("global switch --> Back up master board\n");
					notice_had_to_change_vrrp_state(InstID,tmsg->op);
					hmd_syslog_info("global switch -->set hansi %d-%d vrrp state  to %s\n",HOST_SLOT_NO,InstID,\
					(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
					break;
				}
				new_change_state = tmsg->op;
				//hmd_syslog_info(" old_change_state is %d  new_change_state is %d\n",old_change_state,new_change_state);

				if(old_change_state != new_change_state)
				{
					old_change_state = new_change_state;
					hmd_syslog_info("global switch --> Master board begin to deal with vrrp change information\n");
					hmd_syslog_info("global switch -->reveive message hansi %d-%d vrrp state change to %s\n",tmsg->S_SlotID,tmsg->InstID,\
						(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
					for(slot_no=1;slot_no<=16;slot_no++)
					{
						for(inst_id=1;inst_id<=16;inst_id++)
						{
							/*get global switch hansi id*/
							if(vrrp_global_switch_hansi[slot_no-1]&(1<<(inst_id-1)))
							{
								/*active master slot no*/
								if((tmsg->S_SlotID == HOST_SLOT_NO) && (slot_no == HOST_SLOT_NO) && (inst_id != tmsg->InstID))
								{
									notice_had_to_change_vrrp_state(inst_id,tmsg->op);
									hmd_syslog_info("global switch -->set hansi %d-%d vrrp state to %s\n",slot_no,inst_id,\
									(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
								}/*if is sender do not send to it again*/
								else if((tmsg->S_SlotID == HOST_SLOT_NO) && (slot_no == HOST_SLOT_NO) && (inst_id == tmsg->InstID)) 
								{
									
								}
								else
								{
									if((slot_no == tmsg->S_SlotID) && (inst_id == tmsg->InstID))
									{
										/*if is sender do not send to it again*/
										continue;
									}
									else if((tmsg->S_SlotID != HOST_SLOT_NO) && (slot_no == HOST_SLOT_NO))
									{
										notice_had_to_change_vrrp_state(inst_id,tmsg->op);
										hmd_syslog_info("global switch -->set hansi %d-%d vrrp state to %s\n",slot_no,inst_id,\
											(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
									}
									else
									{
										notice_hmd_client_vrrp_switch_occured(slot_no,inst_id,0,tmsg->op);
										hmd_syslog_info("global switch -->notice client hmd to set hansi %d-%d vrrp state to %s\n",slot_no,inst_id,\
											(tmsg->op==HMD_NOTICE_VRRP_STATE_CHANGE_MASTER)?"MASTER":"BACK");
									}
								}												
							}
						}
					}
#if 1 //add by niehy for hansi depend
                    depend_s_slot_id = tmsg->S_SlotID;
                    depend_s_inst_id = tmsg->InstID;
                	hmd_syslog_info("start chack hansi %d-%d need linkage hansi!\n",depend_s_slot_id,depend_s_inst_id);

                    	for(inst_id=1; inst_id<MAX_INSTANCE;inst_id++){
							//hmd_syslog_info("*********i = %d******\n",inst_id);
							//hmd_syslog_info("depend_slot_id is %d  depend_inst_id is %d \n",(HMD_BOARD[depend_s_slot_id]->Hmd_Inst[depend_s_inst_id]->depend_hansi[inst_id].depend_slot_no),(HMD_BOARD[depend_s_slot_id]->Hmd_Inst[depend_s_inst_id]->depend_hansi[inst_id].Depend_Inst_ID));
							
                        	if(HMD_BOARD[depend_s_slot_id]->Hmd_Inst[depend_s_inst_id]->depend_hansi[inst_id].Depend_Inst_ID )
                    		{
                				//hmd_syslog_info("*********&&&3333333&&&&******\n");

                        		depend_d_slot_id = HMD_BOARD[depend_s_slot_id]->Hmd_Inst[depend_s_inst_id]->depend_hansi[inst_id].depend_slot_no;
                        		depend_d_inst_id = HMD_BOARD[depend_s_slot_id]->Hmd_Inst[depend_s_inst_id]->depend_hansi[inst_id].Depend_Inst_ID;

                				if((HMD_BOARD[depend_d_slot_id] != NULL))
                				{
                                    if(HMD_BOARD[depend_d_slot_id]->Hmd_Inst[depend_d_inst_id] != NULL)
                					{
                						//hmd_syslog_info("start update depend hansi %d-%d!\n",depend_d_slot_id,depend_d_inst_id);
										HmdNoticeToClient(depend_d_slot_id,depend_d_inst_id,0,new_change_state);
                						hmd_syslog_info("Linkage messages sent to hansi %d-%d \n",depend_d_slot_id,depend_d_inst_id);

                					}
									else
										hmd_syslog_info("hansi %d-%d does not exist!\n",depend_d_slot_id,depend_d_inst_id);
                				}
								else
									hmd_syslog_info("slot %d board does not exist!\n",depend_d_slot_id);

                        	}
                    	}
#endif				
					hmd_syslog_info("global switch --> Master board end dealing with vrrp change information\n");
				}
				break;
			case HMD_RELOAD_CONFIG_FOR_EAG:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				hmd_syslog_info("reload config for eag:slot %d inst %d\n", SlotID, InstID);
				hmd_eag_reload(SlotID, 0, InstID);
				hmd_syslog_info("reload config for eag done\n");
				break;
			case HMD_RELOAD_CONFIG_FOR_PDC:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				hmd_syslog_info("reload config for pdc:slot %d inst %d\n", SlotID, InstID);
				hmd_pdc_reload(SlotID, 0, InstID);
				hmd_syslog_info("reload config for pdc done\n");
				break;
			case HMD_RELOAD_CONFIG_FOR_RDC:
				SlotID = tmsg->S_SlotID;
				InstID = tmsg->InstID;
				hmd_syslog_info("reload config for rdc:slot %d inst %d\n", SlotID, InstID);
				hmd_rdc_reload(SlotID, 0, InstID);
				hmd_syslog_info("reload config for rdc done\n");
				break;
			default :
				break;
		}
	}
	close(fd);
	fd = -1;
}

