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
* ACChannelSelection.c
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
#include "CWAC.h"
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

unsigned char Neighbor_AP_Channel_Check(unsigned char channel){
	if((channel == 1)||(channel == 6)||(channel == 11))
		return channel;
	else if((abs(channel-1)<=2))
		return 1;
	else if((abs(6-channel)<=2))
		return 6;
	else if((abs(11-channel)<=2))
		return 11;
	else
		return channel;
}

void get_neighbor_wtps_info(WTP_RRM_INFO * WTP){
	int i, m;
	unsigned int ID = WTP->WTPID;
	struct Neighbor_AP_ELE *ap,*ap1,*ap2,*ap3;
	unsigned char channel_check;
	ap1 = NULL;
	ap2 = NULL;
	ap3 = NULL;	
	if(gCOUNTRYCODE == 0)
		m = 3;
	else
		m = 2;
	for(i = 0; i < m; i++){
		ap = NULL;		
		ap1 = NULL;
		if(AC_WTP[ID]->NeighborAPInfos != NULL)
			ap = AC_WTP[ID]->NeighborAPInfos->neighborapInfos;
		else{
			//printf("return\n");
			return;
		}
		while(ap != NULL){
			//printf("12345\n");
			if((ap == ap2)||(ap == ap3)){
				ap = ap->next;
				continue;
			}
			if(ap1 == NULL){
				ap1 = ap;				
				ap = ap->next;
				continue;
			}			
		//	printf("ap->RSSI %d,ap->WTPID %d\n",ap->RSSI,ap->WTPID);
		//	printf("ap1->RSSI %d,ap1->WTPID %d\n",ap1->RSSI,ap1->WTPID);
			if((ap->RSSI >= ap1->RSSI)&&(ap != ap2)&&(ap != ap3)){
				ap1 = ap;
		//		printf("ap2->RSSI %d,ap2->WTPID %d\n",ap->RSSI,ap->WTPID);
			}
			ap = ap->next;
		}
		if((ap1 != NULL)&&(ap1 != ap2)&&(ap1 != ap3)){
			if((ap1->wtpid != 0)&&(ap1->wtpid < WTP_NUM)){
				WTP->WTPID_List[i][0] = ap1->wtpid;
				WTP->WTPID_List[i][1] = 0;
			}else{
				WTP->WTPID_List[i][0] = 0;
				channel_check = Neighbor_AP_Channel_Check(ap1->Channel);
				WTP->WTPID_List[i][1] = channel_check;
			}
		//	printf("ap1->WTPID %d\n",ap1->WTPID);
			if(ap2 == NULL)
				ap2 = ap1;
			else if(ap3 == NULL)
				ap3 = ap1;
		}
	}
	
}


int get_wtps_info(WTP_RRM_INFO ** WTP){
	int i = 0;	
	unsigned char channel_list[4];
#if 0
	unsigned char channel_list_1[4] = {1, 5, 9, 13};
	unsigned char channel_list_2[4] = {1, 6, 11, 0};
	if(gCOUNTRYCODE == 0)
		memcpy(channel_list, channel_list_1, 4);
	else
		memcpy(channel_list, channel_list_2, 4);
#endif
	memcpy(channel_list, channelRange, 4);
	for(i = 0; i < WTP_NUM; i++){
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5))
		{
			if(WTP[i] == NULL){
				WTP[i] = malloc(sizeof(WTP_RRM_INFO));
				if(WTP[i] == NULL){
					//perror(malloc);
					return 0;
				}
			}
			CWThreadMutexLock(&(gWTPs[i].RRMThreadMutex));
			memset(WTP[i],0,sizeof(WTP_RRM_INFO));
			WTP[i]->WTPID = i;
			WTP[i]->channel = AC_WTP[i]->WTP_Radio[0]->Radio_Chan;
			WTP[i]->flags = 0;
			WTP[i]->txpower = AC_WTP[i]->WTP_Radio[0]->Radio_TXP;
			memcpy(WTP[i]->H_channel_list, channel_list, 4);
			get_neighbor_wtps_info(WTP[i]);
			CWThreadMutexUnlock(&(gWTPs[i].RRMThreadMutex));
			//printf("%d,%d,%d\n",WTP[i]->WTPID_List[0],WTP[i]->WTPID_List[1],WTP[i]->WTPID_List[2]);
		}
	}
	return 1;
}

unsigned char WTP_GET_CHANNEL(WTP_RRM_INFO ** WTP,int WTPID)
{
	int i, j, m, n, k, num, num2;
	unsigned int ID;
	unsigned char channel_list[4] = {0,0,0,0};
	unsigned char channel_nap;
	unsigned char channel_check[4] = {0,0,0,0};
	num = 0;
	num2 = 0;
	if((WTP[WTPID] != NULL)&&(WTP[WTPID]->flags == 1))
		return WTP[WTPID]->channel;
	for(i = 0; i < 3; i++){
	//	printf("WTPID %d\n",WTPID);
		ID = WTP[WTPID]->WTPID_List[i][0];
		channel_nap = WTP[WTPID]->WTPID_List[i][1];
	//	printf("ID %d\n",WTP[WTPID]->WTPID_List[i]);
		if((WTP[ID] != NULL)&&(WTP[ID]->flags == 1)){
	//		printf("WTP[%d]->channel %d\n",ID,WTP[ID]->channel);
			for(j = 0; j < 4; j++){
				if(WTP[WTPID]->H_channel_list[j] == WTP[ID]->channel){
					WTP[WTPID]->H_channel_list[j] = 0;
					channel_check[num2] = WTP[ID]->channel;
					num2++;
					break;
				}
			}
		}else if(channel_nap != 0){
	//		printf("WTP[%d]->channel %d\n",ID,WTP[ID]->channel);
			for(j = 0; j < 4; j++){
				if(WTP[WTPID]->H_channel_list[j] == channel_nap){
					WTP[WTPID]->H_channel_list[j] = 0;
					channel_check[num2] = channel_nap;
					num2++;
					break;
				}
			}
		}
	}
	for(i = 0; i < 4; i++){
		if(WTP[WTPID]->H_channel_list[i] != 0){
			//return WTP[WTPID]->H_channel_list[i];
			for(j = 0; j < 4; j++){
				if(WTP[WTPID]->H_channel_list[i] == WTP[WTPID]->N_channel_list[j])
					break;
			}
			if(j == 4){
				if(WTP[WTPID]->channel != 0){
					channel_list[num] = WTP[WTPID]->H_channel_list[i];
					num++;
				}else
					return WTP[WTPID]->H_channel_list[i];
			}
		}
	}
	if(num != 0){
		for(k = 0; k < num; k++){
			if((WTP[WTPID]->channel != 0)&&(WTP[WTPID]->channel == channel_list[k]))
				return WTP[WTPID]->channel;
		}
		return channel_list[0];
	}
		
	for(m = 3; m>=0; m--){
		if(WTP[WTPID]->N_channel_list[m] != 0){				
			for(n = 0; n < 4; n++){
				if(WTP[WTPID]->H_channel_list[n] == WTP[WTPID]->N_channel_list[m]){
					if(m == 0)
						wid_syslog_warning("WTP[%d]->channel %d may interfere other WTP",WTPID,WTP[WTPID]->H_channel_list[n]);
					return WTP[WTPID]->H_channel_list[n];
				}
			}
			return WTP[WTPID]->N_channel_list[m];
		}				
	}
	for(m = 3; m>=0; m--){
		if(channel_check[m] != 0){
			return channel_check[m];
		}
	}	
	return 0;
}
int Check_WTP_and_Neighbor_Channel(WTP_RRM_INFO ** WTP,unsigned int WTPID){
	int i;
	unsigned int ID;
	unsigned char channel;
	unsigned char channel_list_N[4] = {0,0,0,0};	
	unsigned char channel_list_1[4] = {1, 5, 9, 13};
	unsigned char channel_list_2[4] = {1, 6, 11, 0};
	unsigned char channel_list[4];
	if(gCOUNTRYCODE == 0)
		memcpy(channel_list, channel_list_1, 4);
	else
		memcpy(channel_list, channel_list_2, 4);
	ID = WTPID;
	memcpy(WTP[ID]->H_channel_list, channel_list, 4);
	channel = WTP_GET_CHANNEL(WTP,ID);
//	printf("host WTP %d,channel %d\n",ID,channel);
	if(channel)
		WTP[ID]->channel = channel;
	WTP[ID]->flags = 1;
	channel_list_N[0] = channel;
/*	for(j = 0; j < 4; j++){
		if(channel_list[j] == channel){
			channel_list[j] = 0;
			break;
		}
	}*/
	for(i = 0; i < 3; i++){
		ID = WTP[WTPID]->WTPID_List[i][0];
		if((WTP[ID] != NULL)&&(WTP[ID]->flags == 0)){			
			memcpy(WTP[ID]->H_channel_list, channel_list, 4);
			memcpy(WTP[ID]->N_channel_list, channel_list_N, 4);
			channel = WTP_GET_CHANNEL(WTP,ID);
			channel_list_N[i+1] = channel;
	//		printf("neighbor WTP %d,channel %d\n",ID,channel);
			if(channel){
				WTP[ID]->channel = channel;				
			}			
			WTP[ID]->flags = 1;
		/*	for(j = 0; j < 4; j++){
				if(channel_list[j] == channel){
					channel_list[j] = 0;
					break;
				}
			}*/
		}
	}
	return 1;
}

int Dynamic_Channel_Selection(WTP_RRM_INFO ** WTP){
	int i;
	for(i = 0; i < WTP_NUM; i++){
		if(WTP[i] != NULL){
			
			//printf("5\n");
			Check_WTP_and_Neighbor_Channel(WTP, i);
		}
	}
	return 1;
}

CW_THREAD_RETURN_TYPE CWDynamicChannelSelection(void * arg)
{
	wid_pid_write_v2("CWDynamicChannelSelection",0,vrrid);
	int ret;
	int i;	
	int num = WTP_NUM;
	WTP_RRM_INFO **WTP;
	//gCOUNTRYCODE = 2;
	WTP = malloc(WTP_NUM*sizeof(WTP_RRM_INFO *));
	for(i = 0; i < WTP_NUM; i++){
		WTP[i] = NULL;
	}
	while(1){		
		//int ok = 0;
		//printf("1\n");
		CWThreadMutexLock(&(gACChannelMutex));
		CWWaitThreadCondition(&gACChannelWait, &gACChannelMutex);
		CWThreadMutexUnlock(&gACChannelMutex);
		//CWThreadMutexLock(&(gACChannelMutex));		
		if(num < WTP_NUM){
			WTP = realloc(WTP,WTP_NUM*(sizeof(WTP_RRM_INFO *)));
			for(i=num;i<WTP_NUM;i++)
				WTP[i] = NULL;
			num = WTP_NUM;
		}
		//printf("2\n");
		if(channel_state){
			gCOUNTRYCODE = 2;
			//printf("3\n");
			ret = get_wtps_info(WTP);
			
			//printf("4\n");
			if(ret)
				Dynamic_Channel_Selection(WTP);
		}
		//CWThreadMutexUnlock(&gACChannelMutex);
		//int i;
		for(i=0;i<WTP_NUM;i++){
			if((WTP[i] != NULL)&&(AC_WTP[i] != NULL)){
				//CWThreadMutexLock(&(gACChannelMutex));
				WID_RADIO_SET_CHAN(AC_WTP[i]->WFR_Index, WTP[i]->channel);
				//CWThreadMutexUnlock(&gACChannelMutex);					
				wid_syslog_debug_debug(WID_WTPINFO,"WTP %d Channel %d\n",i,WTP[i]->channel);
				memset(WTP[i],0,sizeof(WTP_RRM_INFO));
				free(WTP[i]);
				WTP[i] = NULL;
			}
		}
	}
}

