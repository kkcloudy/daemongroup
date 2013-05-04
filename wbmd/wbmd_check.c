#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "dbus/wbmd/WbmdDbusDef.h"
#include "wbmd_thread.h"
#include "wbmd_dbus.h"
#include "wbmd_check.h"
#include "wbmd_manage.h"
#include "wbmd_dbushandle.h"
unsigned short cal_chksum(unsigned short *addr,int len)
{
	int nleft=len;
	int sum=0;
	unsigned short *w=addr;
	unsigned short answer=0;
	/*把ICMP报头二进制数据以2字节为单位累加起来*/
	while(nleft>1)
	{
		sum+=*w++;
		nleft-=2;
	}
	/*若ICMP报头为奇数个字节，会剩下最后一字节。把最后一个字节视为一个2字节数据的高字节，这个2字节数据的低字节为0，继续累加*/
	if( nleft==1)
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer;
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return answer;
}

int pack(int WBID, char * sendbuf)
{
	int packsize;
	struct icmp *icmp;
	struct timeval *tval;
	icmp=(struct icmp*)sendbuf;
	icmp->icmp_type=ICMP_ECHO;
	icmp->icmp_code=0;
	icmp->icmp_cksum=0;
	icmp->icmp_seq=wBridge[WBID]->PackNo;
	wBridge[WBID]->PackNo += 1;
	if(wBridge[WBID]->PackNo >= 4096){
		wBridge[WBID]->PackNo = 100+WBID;	
	}
	icmp->icmp_id=WBID;
	packsize=64;
	tval= (struct timeval *)icmp->icmp_data;
	gettimeofday(tval,NULL);
	icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize);

	return packsize;
}

WBMDBool send_packet(int WBID)
{
	int packetsize;
	char sendbuf[DEFAULT_LEN] = {0};	
	
	packetsize=pack(WBID,sendbuf);

	if(sendto(Checkfd,sendbuf,packetsize,0, (struct sockaddr *)&(wBridge[WBID]->wbaddr),sizeof(struct sockaddr) )<0)
	{
		return WBMD_FALSE;
	}
	return WBMD_TRUE;
}


int wbmd_init_ping_check_sock(){
	int sockfd = 0;
	struct protoent *protocol;
	int size=50*1024;
	
	if( (protocol=getprotobyname("icmp") )==NULL)
	{
		wbmd_syslog_info("%s getprotobyname failed",__func__);
		perror("getprotobyname");
		exit(1);
	}
	if( (sockfd=socket(PF_INET,SOCK_RAW,protocol->p_proto) )<0)
	{
		wbmd_syslog_info("%s socket failed",__func__);
		perror("socket error");
		exit(1);
	}
    
	fcntl(sockfd, F_SETFL, O_NONBLOCK); 
	setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&size,sizeof(size) );

	return sockfd;
}


void * WBMDCheck(){	
	int QID = 99;
	wbmd_pid_write_v2("WBMDCheck");
	int WBMsgqID;		
	struct WbmdCheckMsgQ wbmsg;
	WbmdGetMsgQueue(&WBMsgqID);
	while(1)
	{
		int WBID = 0;
		unsigned int IP = 0;
		struct wbridge_info * WB = NULL;
		memset((char*)&wbmsg, 0, sizeof(wbmsg));
		if (msgrcv(WBMsgqID, (struct WbmdCheckMsgQ*)&wbmsg, sizeof(wbmsg.mqinfo), QID, 0) == -1) {
			wbmd_syslog_info("%s msgrcv %s",__func__,strerror(errno));
			perror("msgrcv");
			continue;
		}
		if(wbmsg.mqinfo.type == WBMD_CHECK){
			WBID = wbmsg.mqinfo.WBID;
			send_packet(WBID);
			wBridge[WBID]->Check_Count += 1;
			wbmd_syslog_info("WBID %d NO.%d checking\n",WBID,wBridge[WBID]->Check_Count);
			if(wBridge[WBID]->Check_Count >= 6){
				if(wBridge[WBID]->WBState == 1){
					wBridge[WBID]->WBState = 0;
					time(&(wBridge[WBID]->leave_time));
					wBridge[WBID]->last_access_time = wBridge[WBID]->access_time;
					wBridge[WBID]->access_time = 0;
					WbmdTimerCancel(&(wBridge[WBID]->GetIfInfoTimerID),1);
					WbmdTimerCancel(&(wBridge[WBID]->GetMintInfoTimerID),1);
					WbmdTimerCancel(&(wBridge[WBID]->GetRfInfoTimerID),1);
					wBridge[WBID]->GetIfInfoTimes = 1;
					wBridge[WBID]->GetMintInfoTimes = 1;
					wBridge[WBID]->GetRfInfoTimes = 1;
					wbmd_syslog_info("WBID %d quit\n",WBID);
				}
				WbmdTimerCancel(&(wBridge[WBID]->CheckTimerID),1);
				WBMDTimerRequest(10,&(wBridge[WBID]->CheckTimerID),WBMD_CHECKING,WBID);
			}else{
				WbmdTimerCancel(&(wBridge[WBID]->CheckTimerID),1);
				WBMDTimerRequest(10,&(wBridge[WBID]->CheckTimerID),WBMD_CHECKING,WBID);
			}
		}
		else if(wbmsg.mqinfo.type == WBMD_DATA){
			IP = wbmsg.mqinfo.WBID;
			WB = wbridge_get(IP);
			if(WB){
				if(WB->WBState == 0){
					WB->WBState = 1;					
					time(&(wBridge[WB->WBID]->access_time));
					if(wBridge[WB->WBID]->argn != 0){
						wBridge[WB->WBID]->GetIfInfoTimes = 1;
						wBridge[WB->WBID]->GetMintInfoTimes = 1;
						wBridge[WB->WBID]->GetRfInfoTimes = 1;
						WbmdTimerCancel(&(wBridge[WB->WBID]->GetIfInfoTimerID),1);
						WbmdTimerCancel(&(wBridge[WB->WBID]->GetMintInfoTimerID),1);
						WbmdTimerCancel(&(wBridge[WB->WBID]->GetRfInfoTimerID),1);
						WBMDTimerRequest(5,&(WB->GetIfInfoTimerID),WBMD_GETIFINFO,WB->WBID);
						WBMDTimerRequest(6,&(WB->GetMintInfoTimerID),WBMD_GETMINTINFO,WB->WBID);
						WBMDTimerRequest(7,&(WB->GetRfInfoTimerID),WBMD_GETRFINFO,WB->WBID);
					}
				}
				WB->Check_Count = 0;
				wbmd_syslog_info("WBID %d WBState %d running\n",WB->WBID,WB->WBState);
			}		
		}
		else{
			wbmd_syslog_info("wbmsg.mqinfo.type %d wbmsg.mqinfo.WBID %d\n",wbmsg.mqinfo.type,wbmsg.mqinfo.WBID);
			continue;
		}
	}
}
