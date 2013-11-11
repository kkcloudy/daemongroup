/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* CWThread_wid.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#include "CWCommon.h"

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/msg.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h" 
#include "CWAC.h"

/* For new format of syslog 2013-07-29 */
#include "syslog.h"
// timers
typedef struct {
 	CWThread *requestedThreadPtr;
 	int signalToRaise;
	int WTPID;
	int WLANID;
	int RADIOID;
} CWThreadTimerArg;

extern struct sockaddr M_addr;
extern unsigned char gWIDLOGHN;//qiuchen
void CWHandleTimer(CWTimerArg arg) {
	msgq msg;	
	CWThreadTimerArg *a = (CWThreadTimerArg*)arg;
 //	CWThread requestedThreadPtr = *(a->requestedThreadPtr);
 	int signalToRaise = a->signalToRaise;
	int WTPID = a->WTPID;
	int WLANID = a->WLANID;
	int RADIOID = a->RADIOID;
	int WTPMsgqID;	
	time_t timep;  
	struct tm *p; 
	int times;	
	int timer;
	CWGetMsgQueue(&WTPMsgqID);
	char command[128];
	char * str;
	char IP[128];
	memset(IP,0,128);
//	CWThreadSendSignal(requestedThreadPtr, signalToRaise);
	if(signalToRaise == SIGUSR2 )
	{
		if(AC_WTP[WTPID] == NULL){
			wid_syslog_err("%s WTP %d NULL!!!",__func__,WTPID);	
			CW_FREE_OBJECT(a);
			return;
		}
		if(AC_WTP[WTPID])
		{
			AC_WTP[WTPID]->neighbordeatimes++;
		}
		printf("AC_WTP[%d]->neighbordeatimes %d \n",WTPID,AC_WTP[WTPID]->neighbordeatimes);
		if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->neighbordeatimes == 3)){
			AC_WTP[WTPID]->neighbordeatimes = 0;
			gWTPs[WTPID].isRequestClose = CW_TRUE;	
			syslog_wtp_log(WTPID, 0, "Critical Timer Expired", 0);
			if(gWIDLOGHN & 0x01)
				syslog_wtp_log_hn(WTPID,0,1966081);
			if(gWIDLOGHN & 0x02)
				wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[WTPID],6);
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPID%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPID;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
			msg.mqinfo.u.WtpInfo.WTPID = WTPID;
			if (msgsnd(WTPMsgqID, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_err("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}else{
			if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->neighbordeatimes == 2)&&(wtp_link_detect == 1)){
				str = strchr(AC_WTP[WTPID]->WTPIP,':');
				memcpy(IP,AC_WTP[WTPID]->WTPIP,str-AC_WTP[WTPID]->WTPIP);

				sprintf(command,"echo \"********************************************************************************\" >>/home/WTP%d_tracerout &",WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"echo \"WTP IP : %s\" >>/home/WTP%d_tracerout &",IP,WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"date >>/home/WTP%d_tracerout &",WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);
				sprintf(command,"traceroute -n -I -m 3 %s >>/home/WTP%d_tracerout &",IP,WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"echo \"********************************************************************************\" >>/home/WTP%d_iperf &",WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"echo \"WTP IP : %s\" >>/home/WTP%d_iperf &",IP,WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"date >>/home/WTP%d_iperf &",WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				sprintf(command,"iperf -u -c %s 2 >>/home/WTP%d_iperf &",IP,WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);

				/*sprintf(command,"/usr/bin/trace_wtp2.sh -ht %d %s /home/WTP%d_trace &", WTPID, AC_WTP[WTPID]->WTPIP,WTPID);
				system(command);
				printf("%s\n",command);
				memset(command, 0, 128);
				sprintf(command,"/usr/bin/btrace_wtp2.sh -ht b %s /home/WTP%d_btrace &", AC_WTP[WTPID]->WTPIP,WTPID);
				system(command);
				printf("%s\n",command);*/
			}
			if(!CWErr(CWTimerRequest(gCWNeighborDeadInterval/3, &(gWTPs[WTPID].thread), &(gWTPs[WTPID].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPID))) { // start NeighborDeadInterval timer
				CW_FREE_OBJECT(a);
				return ;
			}
		}
		
	}
	else if(signalToRaise == 500)
	{
		if((gWTPs[WTPID].currentState == CW_QUIT)&&(find_in_wtp_list(WTPID) == CW_TRUE)&&(AC_WTP[WTPID]))
		{
			//printf("CWCriticalTimerExpiredHandler\n");
			delete_wtp_list(WTPID);
			insert_uptfail_wtp_list(WTPID);
			AC_WTP[WTPID]->updatefailstate = 0;
			update_complete_check();
		}		
	}
	else if(signalToRaise == SIGUSR1)
	{
	
		wid_syslog_debug_debug(WID_DEFAULT,"Soft Timer Expired for Thread: %08x", (unsigned int)CWThreadSelf());
				
		if((!gWTPs[WTPID].isRetransmitting) || (gWTPs[WTPID].messages == NULL)) {
			wid_syslog_info("Soft timer expired but we are not retransmitting");
			if(AC_WTP[WTPID] != NULL)
			{
				CWThreadMutexLock(&(gWTPs[WTPID].WTPThreadControllistMutex));
				if((AC_WTP[WTPID])&&(AC_WTP[WTPID]->ControlWait != NULL))
				{
					free(AC_WTP[WTPID]->ControlWait);
					AC_WTP[WTPID]->ControlWait = NULL;
				}
				CWThreadMutexUnlock(&(gWTPs[WTPID].WTPThreadControllistMutex));
			}
			CW_FREE_OBJECT(a);
			return;
		}
	
		(gWTPs[WTPID].retransmissionCount)++;
		
		wid_syslog_debug_debug(WID_DEFAULT,"Retransmission Count increases to %d", gWTPs[WTPID].retransmissionCount);
		
		if(gWTPs[WTPID].retransmissionCount >= gCWMaxRetransmit) 
		{
			wid_syslog_debug_debug(WID_DEFAULT,"Peer is Dead");
			//?? _CWCloseThread(*iPtr);
			// Request close thread
			gWTPs[WTPID].isRequestClose = CW_TRUE;
	//		CWDownWTP(*iPtr);
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPID%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPID;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
			msg.mqinfo.u.WtpInfo.WTPID = WTPID;
			if (msgsnd(WTPMsgqID, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_err("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
	//		CWSignalThreadCondition(&gWTPs[*iPtr].interfaceWait);
			CW_FREE_OBJECT(a);
			return;
		}else{
			memset((char*)&msg, 0, sizeof(msg));
			msg.mqid = WTPID%THREAD_NUM+1;
			msg.mqinfo.WTPID = WTPID;
			msg.mqinfo.type = CONTROL_TYPE;
			msg.mqinfo.subtype = WTP_S_TYPE;
			msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_RESEND;
			msg.mqinfo.u.WtpInfo.WTPID = WTPID;
			if (msgsnd(WTPMsgqID, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
				wid_syslog_err("%s msgsend %s",__func__,strerror(errno));
				perror("msgsnd");
			}
		}		
	}
	else if(signalToRaise == 501){		
		printf("501 enable\n");
		time(&timep);  
		p=localtime(&timep);
		printf("time %d:%d:%d\n",p->tm_hour,p->tm_min,p->tm_sec);
		if((AC_WLAN[WLANID]!= NULL)){
			if(AC_WLAN[WLANID]->StartService.TimerState == 1){
				if((AC_WLAN[WLANID]->StartService.wday[p->tm_wday] == 1)&&(AC_WLAN[WLANID]->Status == 1)){
					WID_ENABLE_WLAN(WLANID);
				}
				if(AC_WLAN[WLANID]->StartService.is_once == 1){
					AC_WLAN[WLANID]->StartService.wday[p->tm_wday] = 0;
				}
				if((AC_WLAN[WLANID]->StartService.wday[0]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[1]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[2]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[3]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[4]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[5]==0)
					&&(AC_WLAN[WLANID]->StartService.wday[6]==0)
				){
					AC_WLAN[WLANID]->StartService.TimerState = 0;
					printf("501 end\n");
				}else{
					time(&timep);  
					p=localtime(&timep);
					times = p->tm_hour*3600 + p->tm_min*60 + p->tm_sec;
					if(times < AC_WLAN[WLANID]->StartService.times){
						timer = AC_WLAN[WLANID]->StartService.times - times; 
					}else{
						timer = 24*3600 - times + AC_WLAN[WLANID]->StartService.times;
					}
					if(!(CWTimerRequest(timer, NULL, &(AC_WLAN[WLANID]->StartService.TimerID), 501,WLANID))) {
						CW_FREE_OBJECT(a);
						return ;
					}					
					printf("AC_WLAN[wlanid]->StartService.TimerID33333333 %d\n",AC_WLAN[WLANID]->StartService.TimerID);
				}
			}
		}
		
	}
	else if(signalToRaise == 502){		
		printf("502 disable\n");
		
		time(&timep);  
		p=localtime(&timep);
		printf("time %d:%d:%d\n",p->tm_hour,p->tm_min,p->tm_sec);

		if((AC_WLAN[WLANID]!= NULL)){
			if(AC_WLAN[WLANID]->StopService.TimerState == 1){
				if((AC_WLAN[WLANID]->StopService.wday[p->tm_wday] == 1)&&(AC_WLAN[WLANID]->Status == 0)){
					WID_DISABLE_WLAN(WLANID);
				}
				if(AC_WLAN[WLANID]->StopService.is_once == 1){
					AC_WLAN[WLANID]->StopService.wday[p->tm_wday] = 0;
				}
				if((AC_WLAN[WLANID]->StopService.wday[0]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[1]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[2]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[3]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[4]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[5]==0)
					&&(AC_WLAN[WLANID]->StopService.wday[6]==0)
				){
					AC_WLAN[WLANID]->StopService.TimerState = 0;
					printf("502 end\n");
				}else{
					time(&timep);  
					p=localtime(&timep);
					times = p->tm_hour*3600 + p->tm_min*60 + p->tm_sec;
					if(times < AC_WLAN[WLANID]->StopService.times){
						timer = AC_WLAN[WLANID]->StopService.times - times; 
					}else{
						timer = 24*3600 - times + AC_WLAN[WLANID]->StopService.times;
					}
					if(!(CWTimerRequest(timer, NULL, &(AC_WLAN[WLANID]->StopService.TimerID), 502,WLANID))) {
						CW_FREE_OBJECT(a);
						return ;
					}	
					
					printf("AC_WLAN[wlanid]->StopService.TimerID3333333 %d\n",AC_WLAN[WLANID]->StopService.TimerID);
				}
			}
		}
	}
	else if(signalToRaise == 503){		
		printf("503 enable\n");
		time(&timep);  
		p=localtime(&timep);
		printf("time %d:%d:%d\n",p->tm_hour,p->tm_min,p->tm_sec);
		if((AC_RADIO[RADIOID] != NULL)){
			if(AC_RADIO[RADIOID]->StartService.TimerState == 1){
				if((AC_RADIO[RADIOID]->StartService.wday[p->tm_wday] == 1)&&(AC_RADIO[RADIOID]->AdStat == 2)){
					WID_RADIO_SET_STATUS(RADIOID,1);
				}
				if(AC_RADIO[RADIOID]->StartService.is_once == 1){
					AC_RADIO[RADIOID]->StartService.wday[p->tm_wday] = 0;
				}
				if((AC_RADIO[RADIOID]->StartService.wday[0]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[1]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[2]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[3]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[4]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[5]==0)
					&&(AC_RADIO[RADIOID]->StartService.wday[6]==0)
				){
					AC_RADIO[RADIOID]->StartService.TimerState = 0;
					printf("503 end\n");
				}else{
					time(&timep);  
					p=localtime(&timep);
					times = p->tm_hour*3600 + p->tm_min*60 + p->tm_sec;
					if(times < AC_RADIO[RADIOID]->StartService.times){
						timer = AC_RADIO[RADIOID]->StartService.times - times; 
					}else{
						timer = 24*3600 - times + AC_RADIO[RADIOID]->StartService.times;
					}
					if(!(CWTimerRequest(timer, NULL, &(AC_RADIO[RADIOID]->StartService.TimerID), 503,WLANID))) {
						CW_FREE_OBJECT(a);
						return ;
					}					
					printf("AC_WLAN[wlanid]->StartService.TimerID33333333 %d\n",AC_RADIO[RADIOID]->StartService.TimerID);
				}
			}
		}
		
	}
	else if(signalToRaise == 504){		
		printf("504 disable\n");
		
		time(&timep);  
		p=localtime(&timep);
		printf("time %d:%d:%d\n",p->tm_hour,p->tm_min,p->tm_sec);

		if((AC_RADIO[RADIOID]!= NULL)){
			if(AC_RADIO[RADIOID]->StopService.TimerState == 1){
				if((AC_RADIO[RADIOID]->StopService.wday[p->tm_wday] == 1)&&(AC_RADIO[RADIOID]->AdStat == 1)){
					WID_RADIO_SET_STATUS(RADIOID,2);
				}
				if(AC_RADIO[RADIOID]->StopService.is_once == 1){
					AC_RADIO[RADIOID]->StopService.wday[p->tm_wday] = 0;
				}
				if((AC_RADIO[RADIOID]->StopService.wday[0]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[1]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[2]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[3]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[4]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[5]==0)
					&&(AC_RADIO[RADIOID]->StopService.wday[6]==0)
				){
					AC_RADIO[RADIOID]->StopService.TimerState = 0;
					printf("504 end\n");
				}else{
					time(&timep);  
					p=localtime(&timep);
					times = p->tm_hour*3600 + p->tm_min*60 + p->tm_sec;
					if(times < AC_RADIO[RADIOID]->StopService.times){
						timer = AC_RADIO[RADIOID]->StopService.times - times; 
					}else{
						timer = 24*3600 - times + AC_RADIO[RADIOID]->StopService.times;
					}
					if(!(CWTimerRequest(timer, NULL, &(AC_RADIO[RADIOID]->StopService.TimerID), 504,WLANID))) {
						CW_FREE_OBJECT(a);
						return ;
					}	
					
					printf("AC_WLAN[wlanid]->StopService.TimerID3333333 %d\n",AC_RADIO[RADIOID]->StopService.TimerID);
				}
			}
		}
	}
	else if(signalToRaise == 900)
	{
		if(is_secondary == 1){
			update_license_req(wid_bak_sock, (struct sockaddr_in *)&M_addr);
			sleep(1);
			bak_check_req(wid_bak_sock);			
			if(!CWErr(CWTimerRequest(BakCheckInterval, NULL, &(bak_check_timer), 900, 0))) { 
				CW_FREE_OBJECT(a);
				return ;
			}
		}		
	}
	else if(signalToRaise == 901){
		if(Lic_ip.isActive == 2){
			//lic_bak_req();
			update_license_req(lic_bak_fd , (struct sockaddr_in *)&Lic_Active_addr);
			if(!CWErr(CWTimerRequest(LicBakReqInterval, NULL, &(Lic_bak_req_timer), 901, 0))) { 
				CW_FREE_OBJECT(a);
				return ;
			}
		}

	}
	/*fengwenchao add 20120117 for onlinebug-96*/
	else if(signalToRaise == 911) 
	{
		wid_syslog_info("%s , signalToRaise = 911  \n",__func__);
		int i = 0;
		msgq qmsg;	
		if(is_secondary == 0)
		{
			for(i = 1; i < WTP_NUM;i++)
			{
				if(AC_WTP[i] != NULL)
				{
					//wid_syslog_info("1:AC_WTP[%d]->WTPStat = %d \n",i,AC_WTP[i]->WTPStat);
					if(AC_WTP[i]->WTPStat == 9)
					{					
						//wid_syslog_info("2:AC_WTP[%d]->WTPStat = %d \n",i,AC_WTP[i]->WTPStat);
						gWTPs[i].isRequestClose = CW_TRUE;	
						syslog_wtp_log(i, 0, "Critical Timer Expired", 0);
						if(gWIDLOGHN & 0x01)
							syslog_wtp_log_hn(i,0,0);
						if(gWIDLOGHN & 0x02)
							wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[i],0);
						memset((char*)&qmsg, 0, sizeof(qmsg));
						qmsg.mqid = i%THREAD_NUM+1;
						qmsg.mqinfo.WTPID = i;
						qmsg.mqinfo.type = CONTROL_TYPE;
						qmsg.mqinfo.subtype = WTP_S_TYPE;
						qmsg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
						qmsg.mqinfo.u.WtpInfo.WTPID = i;
						if (msgsnd(WTPMsgqID, (msgq *)&qmsg, sizeof(qmsg.mqinfo), IPC_NOWAIT) == -1){
							wid_syslog_err("%s msgsend %s",__func__,strerror(errno));
							perror("msgsnd");
						}
			
						//wid_syslog_info("correct AC_WTP[%d]->WTPStat = %d \n",i,AC_WTP[i]->WTPStat);
					}
				}
			}
		}
		
	}
	/*fengwenchao add end*/
 //	CWDebugLog("Timer Expired, Sent Signal(%d) to Thread: %d", signalToRaise, requestedThreadPtr);

//	CW_FREE_OBJECT(a->requestedThreadPtr);
	CW_FREE_OBJECT(a);

	return;
}
CWBool CWTimerRequest(int sec, CWThread *threadPtr, CWTimerID *idPtr, int signalToRaise, int ID) {

	CWThreadTimerArg *arg;

	CWDebugLog("Timer Request");
//	printf("CWTimerRequest1\n");
	if(sec < 0 || idPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
//	printf("CWTimerRequest2\n");
	
	CW_CREATE_OBJECT_ERR(arg, CWThreadTimerArg, return CW_FALSE;);
//	CW_CREATE_OBJECT_ERR(arg->requestedThreadPtr, CWThread, CW_FREE_OBJECT(arg); return CW_FALSE;);
 //	CW_COPY_MEMORY(arg->requestedThreadPtr, threadPtr, sizeof(CWThread));
 	memset(arg, 0 ,sizeof(CWThreadTimerArg));
 	arg->signalToRaise = signalToRaise;
 	arg->WTPID = ID;
	arg->WLANID = ID;
	arg->RADIOID = ID;
//	CWDebugLog("Timer Request: thread(%d), signal(%d)", *(arg->requestedThreadPtr), arg->signalToRaise);
//	printf("CWTimerRequest3\n");
	
	if ((*idPtr = timer_add(sec, 0, &CWHandleTimer, arg)) == -1) {

		return CW_FALSE;
	}

	return CW_TRUE;
}

