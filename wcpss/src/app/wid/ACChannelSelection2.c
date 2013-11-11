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

unsigned char Neighbor_AP_Channel_Check2(unsigned char channel){
	if((channel == 149)||(channel == 153)||(channel == 157)||(channel == 161)||(channel == 165))
		return channel;
	else if((abs(channel-149)<=2))
		return 149;
	else if((abs(153-channel)<=2))
		return 153;
	else if((abs(157-channel)<=2))
		return 157;	
	else if((abs(161-channel)<=2))
		return 161;
	else if((abs(165-channel)<=2))
		return 165;
	else
		return channel;
}

void get_neighbor_wtps_info2(WTP_RRM_INFO * WTP){
	int i, m;
	unsigned int ID = WTP->WTPID;
	struct Neighbor_AP_ELE *ap,*ap1,*ap2,*ap3,*ap4;
	unsigned char channel_check;
	ap1 = NULL;
	ap2 = NULL;
	ap3 = NULL;
	ap4 = NULL;
	m = 4;
	for(i = 0; i < m; i++){
		ap = NULL;		
		ap1 = NULL;
		if(AC_WTP[ID]->NeighborAPInfos2 != NULL)
			ap = AC_WTP[ID]->NeighborAPInfos2->neighborapInfos;
		else{
			//printf("return\n");
			return;
		}
		while(ap != NULL){
			//printf("12345\n");
			if((ap == ap2)||(ap == ap3)||(ap == ap4)){
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
			if((ap->RSSI >= ap1->RSSI)&&(ap != ap2)&&(ap != ap3)&&(ap != ap4)){
				ap1 = ap;
		//		printf("ap2->RSSI %d,ap2->WTPID %d\n",ap->RSSI,ap->WTPID);
			}
			ap = ap->next;
		}
		if((ap1 != NULL)&&(ap1 != ap2)&&(ap1 != ap3)&&(ap1 != ap4)){
			if((ap1->wtpid != 0)&&(ap1->wtpid < WTP_NUM)){
				WTP->WTPID_List[i][0] = ap1->wtpid;
				WTP->WTPID_List[i][1] = 0;
				printf("WTP->WTPID_List[i][0] %d\n",WTP->WTPID_List[i][0]);
			}else{
				WTP->WTPID_List[i][0] = 0;
				channel_check = Neighbor_AP_Channel_Check2(ap1->Channel);
				WTP->WTPID_List[i][1] = channel_check;
				printf("WTP->WTPID_List[i][1] %d\n",WTP->WTPID_List[i][1]);
			}
		//	printf("ap1->WTPID %d\n",ap1->WTPID);
			if(ap2 == NULL)
				ap2 = ap1;
			else if(ap3 == NULL)
				ap3 = ap1;
			else if(ap4 == NULL)
				ap4 = ap1;
		}
	}
	
}


int get_wtps_info2(WTP_RRM_INFO ** WTP){
	int i = 0;	
	unsigned char channel_list[5] = {149,153,157,161,165};
#if 0
	unsigned char channel_list_1[4] = {1, 5, 9, 13};
	unsigned char channel_list_2[4] = {1, 6, 11, 0};
	if(gCOUNTRYCODE == 0)
		memcpy(channel_list, channel_list_1, 4);
	else
		memcpy(channel_list, channel_list_2, 4);
#endif
	for(i = 0; i < WTP_NUM; i++){
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->WTPStat == 5)&&(AC_WTP[i]->RadioCount == 2)&&(AC_WTP[i]->WTP_Radio[1]->Radio_Type & 0x2))
		{
			if(WTP[i] == NULL){
				WTP[i] = WID_MALLOC(sizeof(WTP_RRM_INFO));
				if(WTP[i] == NULL){
					//perror(malloc);
					return 0;
				}
			}
			CWThreadMutexLock(&(gWTPs[i].RRMThreadMutex));
			memset(WTP[i],0,sizeof(WTP_RRM_INFO));
			WTP[i]->WTPID = i;
			WTP[i]->channel = AC_WTP[i]->WTP_Radio[1]->Radio_Chan;
			WTP[i]->flags = 0;
			WTP[i]->txpower = AC_WTP[i]->WTP_Radio[1]->Radio_TXP;
			memcpy(WTP[i]->H_channel_list, channel_list, 5);
			get_neighbor_wtps_info2(WTP[i]);
			CWThreadMutexUnlock(&(gWTPs[i].RRMThreadMutex));
			//printf("%d,%d,%d\n",WTP[i]->WTPID_List[0],WTP[i]->WTPID_List[1],WTP[i]->WTPID_List[2]);
		}
	}
	return 1;
}

unsigned char WTP_GET_CHANNEL2(WTP_RRM_INFO ** WTP,int WTPID)
{
	int i, j, m, n, k, num, num2;
	unsigned int ID;
	unsigned char channel_list[5] = {0,0,0,0,0};
	unsigned char channel_nap;
	unsigned char channel_check[5] = {0,0,0,0,0};
	num = 0;
	num2 = 0;
	if((WTP[WTPID] != NULL)&&(WTP[WTPID]->flags == 1))
		return WTP[WTPID]->channel;
	for(i = 0; i < 4; i++){
		printf("WTPID %d\n",WTPID);
		ID = WTP[WTPID]->WTPID_List[i][0];
		channel_nap = WTP[WTPID]->WTPID_List[i][1];
		if(ID > WTP_NUM)
			ID = 0;
		if((WTP[ID] != NULL)&&(WTP[ID]->flags == 1)){
			printf("WTP[%d]->channel %d\n",ID,WTP[ID]->channel);
			for(j = 0; j < 5; j++){
				if(WTP[WTPID]->H_channel_list[j] == WTP[ID]->channel){
					WTP[WTPID]->H_channel_list[j] = 0;
					channel_check[num2] = WTP[ID]->channel;
					num2++;
					break;
				}
			}
		}else if(channel_nap != 0){
			printf("channel_nap %d\n",channel_nap);
			for(j = 0; j < 5; j++){
				if(WTP[WTPID]->H_channel_list[j] == channel_nap){
					WTP[WTPID]->H_channel_list[j] = 0;
					channel_check[num2] = channel_nap;
					num2++;
					break;
				}
			}
		}
	}
	for(i = 0; i < 5; i++){
		if(WTP[WTPID]->H_channel_list[i] != 0){
			printf("WTP[%d]->H_channel_list[%d] %d \n",WTPID,i,WTP[WTPID]->H_channel_list[i]);
			//return WTP[WTPID]->H_channel_list[i];
			for(j = 0; j < 5; j++){
				if(WTP[WTPID]->H_channel_list[i] == WTP[WTPID]->N_channel_list[j])
					break;
			}
			if(j == 5){
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
		
	for(m = 4; m>=0; m--){
		if(WTP[WTPID]->N_channel_list[m] != 0){				
			for(n = 0; n < 5; n++){
				if(WTP[WTPID]->H_channel_list[n] == WTP[WTPID]->N_channel_list[m]){
					if(m == 0)
						wid_syslog_warning("WTP[%d]->channel %d may interfere other WTP",WTPID,WTP[WTPID]->H_channel_list[n]);
					return WTP[WTPID]->H_channel_list[n];
				}
			}
			return WTP[WTPID]->N_channel_list[m];
		}				
	}
	for(m = 4; m>=0; m--){
		if(channel_check[m] != 0){
			return channel_check[m];
		}
	}	
	return 0;
}
int Check_WTP_and_Neighbor_Channel2(WTP_RRM_INFO ** WTP,unsigned int WTPID){
	int i;
	unsigned int ID;
	unsigned char channel;
	unsigned char channel_list_N[5] = {0,0,0,0,0};	
	unsigned char channel_list_1[5] = {149, 153, 157, 161,165};
	unsigned char channel_list[5];
	memcpy(channel_list, channel_list_1, 5);
	ID = WTPID;
	memcpy(WTP[ID]->H_channel_list, channel_list, 5);
	channel = WTP_GET_CHANNEL2(WTP,ID);
	printf("host WTP %d,channel %d\n",ID,channel);
	if(channel)
		WTP[ID]->channel = channel;
	WTP[ID]->flags = 1;
	channel_list_N[0] = channel;

	for(i = 0; i < 4; i++){
		ID = WTP[WTPID]->WTPID_List[i][0];
		if((WTP[ID] != NULL)&&(WTP[ID]->flags == 0)){			
			memcpy(WTP[ID]->H_channel_list, channel_list, 5);
			memcpy(WTP[ID]->N_channel_list, channel_list_N, 5);
			channel = WTP_GET_CHANNEL2(WTP,ID);
			channel_list_N[i+1] = channel;
			printf("neighbor WTP %d,channel %d\n",ID,channel);
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

int Dynamic_Channel_Selection2(WTP_RRM_INFO ** WTP){
	int i;
	for(i = 0; i < WTP_NUM; i++){
		if(WTP[i] != NULL){
			
			//printf("5\n");
			Check_WTP_and_Neighbor_Channel2(WTP, i);
		}
	}
	return 1;
}

CW_THREAD_RETURN_TYPE CWDynamicChannelSelection2(void * arg)
{
	int ret;
	int i;	
	int num = WTP_NUM;
	WTP_RRM_INFO **WTP;
	//gCOUNTRYCODE = 2;
	WTP = WID_MALLOC(WTP_NUM*sizeof(WTP_RRM_INFO *));
	if (!WTP) {
		wid_syslog_err("%s malloc for WTP failed \n", __FUNCTION__);
		return NULL;
	}
	for(i = 0; i < WTP_NUM; i++){
		WTP[i] = NULL;
	}
	while(1){		
		//int ok = 0;
		//printf("1\n");
		CWThreadMutexLock(&(gACChannelMutex2));
		CWWaitThreadCondition(&gACChannelWait2, &gACChannelMutex2);
		CWThreadMutexUnlock(&gACChannelMutex2);
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
			ret = get_wtps_info2(WTP);
			
			//printf("4\n");
			if(ret)
				Dynamic_Channel_Selection2(WTP);
		}
		//CWThreadMutexUnlock(&gACChannelMutex);
		//int i;
		for(i=0;i<WTP_NUM;i++){
			if((WTP[i] != NULL)&&(AC_WTP[i] != NULL)){
				//CWThreadMutexLock(&(gACChannelMutex));
				WID_RADIO_SET_CHAN(AC_WTP[i]->WFR_Index+1, WTP[i]->channel);
				//CWThreadMutexUnlock(&gACChannelMutex);					
				wid_syslog_info("%s WTP %d Channel %d\n",__func__,i,WTP[i]->channel);
				printf("WTP %d Channel %d\n",i,WTP[i]->channel);
				memset(WTP[i],0,sizeof(WTP_RRM_INFO));
				//wid_syslog_debug_debug(WID_DEFAULT,"%s,%d\n",__func__,__LINE__);
				WID_FREE(WTP[i]);
				WTP[i] = NULL;
				//wid_syslog_debug_debug(WID_DEFAULT,"%s,%d\n",__func__,__LINE__);
			}
		}
	}
}

