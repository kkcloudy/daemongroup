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
* ACRunstate.c
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

#include <netinet/in.h>
#include "CWAC.h"
#include "CWProtocol.h"
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "dbus/asd/ASDDbusDef.h"
#include "ACDbus.h"
#include "ACDbus_handler.h"
#include "hmd/hmdpub.h"
#include "ACBak.h"
#include "AC.h"
#include "CWConfigFile.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif


CWBool CWParseConfigurationUpdateResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode,CWRadioOperationalInfoValues* radioOpeinfo,int wtpindex);
CWBool CWSaveConfigurationUpdateResponseMessage(CWProtocolResultCode resultCode, int WTPIndex);

CWBool CWParseWTPEventRequestMessage(CWProtocolMessage *msgPtr, int len, CWProtocolWTPEventRequestValues *valuesPtr,int wtpindex);
CWBool CWSaveWTPEventRequestMessage (CWProtocolWTPEventRequestValues *WTPEventRequest, CWWTPProtocolManager *WTPProtocolManager);
CWBool CWAssembleWTPEventResponse (CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum);

CWBool CWParseChangeStateEventRequestMessage2 (CWProtocolMessage *msgPtr, int len, CWProtocolChangeStateEventRequestValues **valuesPtr);

CWBool CWParseEchoRequestMessage (CWProtocolMessage *msgPtr, int len, int WTPIndex);
CWBool CWAssembleEchoResponse (CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum);

CWBool CWStartNeighborDeadTimer(int WTPIndex);
CWBool CWStopNeighborDeadTimer(int WTPIndex);
CWBool CWRestartNeighborDeadTimer(int WTPIndex);
CWBool CWRestartNeighborDeadTimerForEcho(int WTPIndex);
CWBool CWParseWlanConfigurationResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode, WID_BSS *BSS);
CWBool CWParseStaConfigurationResponseMessage(CWProtocolMessage* msgPtr, int len, int WTPIndex,CWProtocolResultCode* resultCode);
CWBool CWParseDiscoveryRequestMessageInRun(CWProtocolMessage* msgPtr, int len, CWDiscoveryRequestValues *valuesPtr);
void CWDestroyDiscoveryRequestValues(CWDiscoveryRequestValues *valPtr);
	//int CreateWlan = 1;
static const int gCWIANATimes256_2 = CW_IANA_ENTERPRISE_NUMBER * 256;
unsigned int RadioTXPOF;

CWBool ACEnterRun(int WTPIndex, CWProtocolMessage *msgPtr, CWBool dataFlag)
{
			
	CWBool toSend= CW_FALSE;
	CWBool reset = CW_FALSE;
	CWControlHeaderValues controlVal;
	CWProtocolMessage* messages =NULL;
	int messagesCount=0;
	msgPtr->offset = 0;
	char i = 0,result = 0;
	//wid_syslog_debug_info("##### WTP %d Enter Run State#####\n",WTPIndex);
/*	while(CreateWlan){
		printf("while");
		if(CWBindingAssembleWlanConfigurationRequest(&(messages), &(messagesCount), gWTPs[WTPIndex].pathMTU, controlVal.seqNum, WTPIndex)){
			if(CWACSendAcknowledgedPacket(WTPIndex, CW_MSG_TYPE_VALUE_WLAN_CONFIGURATION_RESPONSE, controlVal.seqNum)) 
				CreateWlan = 0;
			else
				CWACStopRetransmission(WTPIndex);
			printf("create wlan\n");
			toSend = CW_TRUE;
		}
		
	}*/
	if(dataFlag){
		//## We have received a Data Message... now just log this event
		wid_syslog_debug_debug(WID_DEFAULT,"--> Received a DATA Message");
		return CW_TRUE;
	}

	if(!(CWACParseGenericRunMessage(WTPIndex, msgPtr, &controlVal))) {
		//## Two possible errors: WRONG_ARG and INVALID_FORMAT
		//## In the second case we have an unexpected response: ignore the
		//## message and log the event.
		return CW_FALSE;
	}
	if(!check_wtpid_func(WTPIndex)){
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	//
	//printf("dACEnterRun!\n");
	for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
	{
		if(gConfigVersionUpdateInfo[i] != NULL)
		{
			result = 1;
			break;
		}
	}
	if((result != 0)&&(find_in_wtp_list(WTPIndex) == CW_TRUE))
	{
		//printf("dACEnterRun gConfigVersionUpdateInfo\n");
		delete_wtp_list(WTPIndex);
		update_next_wtp();
		
		if(updatewtplist == NULL)
		{
			//printf("merge model list !\n");
			
//			CWThreadMutexLock(&(gAllThreadMutex));
		/*	CWConfigVersionInfo *pnode = gConfigVersionInfo;
			while(pnode != NULL)
			{
				if(strcmp(pnode->str_ap_model,gConfigVersionUpdateInfo->str_ap_model) == 0)
				{
					free(pnode->str_ap_version_name);
					pnode->str_ap_version_name = NULL;
					free(pnode->str_ap_version_path);
					pnode->str_ap_version_path = NULL;
					
					CW_CREATE_STRING_ERR(pnode->str_ap_version_name,strlen(gConfigVersionUpdateInfo->str_ap_version_name),return CW_FALSE;);
					CW_CREATE_STRING_ERR(pnode->str_ap_version_path,strlen(gConfigVersionUpdateInfo->str_ap_version_path),return CW_FALSE;);
					
					strcpy(pnode->str_ap_version_name,gConfigVersionUpdateInfo->str_ap_version_name);
					strcpy(pnode->str_ap_version_path,gConfigVersionUpdateInfo->str_ap_version_path);
		*/		
					wid_syslog_debug_debug(WID_DEFAULT,"free upgrade configuration in func %s\n",__func__);
					/*mahz modified to match ap upgrade automatically*/
					for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
						CWConfigVersionInfo *tmp_node = gConfigVersionUpdateInfo[i];
						while(tmp_node != NULL){
							CWConfigVersionInfo *free_node = tmp_node;
							tmp_node = tmp_node->next;
							
							CW_FREE_OBJECT_WID(free_node->str_ap_model);		
							CW_FREE_OBJECT_WID(free_node->str_ap_version_name); 	
							CW_FREE_OBJECT_WID(free_node->str_ap_version_path); 	
							CW_FREE_OBJECT_WID(free_node->str_ap_code); 	
							CW_FREE_OBJECT_WID(free_node);		
						}
						gConfigVersionUpdateInfo[i] = NULL;
					}
					
					gupdateControl = 0;
					checkwtpcount =0;
					destroy_wtp_list();

			/*
					break;
				}
				pnode = pnode->next;
			}*/
//			CWThreadMutexUnlock(&(gAllThreadMutex));
		}
			
	}


	switch(controlVal.messageTypeValue) {
		case CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE:{
			CWProtocolResultCode resultCode;
			CWRadioOperationalInfoValues radioOpeinfo;
			if(!(CWParseConfigurationUpdateResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode,&radioOpeinfo,WTPIndex)))
				return CW_FALSE;
			CWACStopRetransmission(WTPIndex);
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			CWSaveConfigurationUpdateResponseMessage(resultCode, WTPIndex);
			
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
			if((AC_WTP[WTPIndex]->ControlWait != NULL)&&((AC_WTP[WTPIndex]->ControlWait->mqinfo.subtype == Radio_S_TYPE)||(AC_WTP[WTPIndex]->ControlWait->mqinfo.subtype == WTP_S_TYPE))){
				wid_syslog_debug_debug(WID_DEFAULT,"config update response\n");
				if((AC_WTP[WTPIndex]->ControlWait->mqinfo.subtype == Radio_S_TYPE)/*&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.u.RadioInfo.Radio_Op == Radio_TXPOF)*/)
				{
				/*resultcode parse*/
				/*0++++++7+++11+++15++++20++++++++++31*/
				/*-----type-----+-l_rd-+---------value---------*/
				/*++++++++++++++++++++++++++++++++++*/
				
					unsigned int result_l_radioid;
					unsigned int txpowerof_or_channel_state;
					int radioID;
					wid_syslog_debug_debug(WID_DEFAULT,"result_txp is %d \n",resultCode);
					txpowerof_or_channel_state = resultCode>>20;/*type*/
					wid_syslog_debug_debug(WID_DEFAULT,"txpowerof_or_channel_state is %d\n",txpowerof_or_channel_state);
					if(resultCode != 0)	{ 
						if(((txpowerof_or_channel_state == 0)||(txpowerof_or_channel_state == 3))&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.u.RadioInfo.Radio_Op == Radio_TXPOF))
						{
							/*txpoweroffset*/
							/*please delete (txpowerof_or_channel_state == 0) from if(), when old ap version is not used */
							/*(txpowerof_or_channel_state == 0)  old ap version use*/
							/*(txpowerof_or_channel_state == 3)  new ap verison use*/
							result_l_radioid=(resultCode>>16)&0x000f;/*l_rd*/
							radioID=WTPIndex*4+result_l_radioid;
							if(!check_l_radioid_func(result_l_radioid)){
								wid_syslog_err("%s\n",__func__);
								CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
								return CW_FALSE;
							}else{
							}
							if(AC_RADIO[radioID]!=NULL){
								int radio_txpof = 0x00ff&resultCode;
								wid_syslog_debug_debug(WID_DEFAULT,"radio_txpof is %d \n",radio_txpof);
								AC_RADIO[radioID]->Radio_TXP=radio_txpof;
							}					
						}
						else if(((txpowerof_or_channel_state == 1))&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.u.RadioInfo.Radio_Op == Radio_Channel))
						{
							int channel = 0x00ff&resultCode;
							result_l_radioid=(resultCode>>16)&0x000f;/*l_rd*/
							if(!check_l_radioid_func(result_l_radioid)){
								wid_syslog_err("%s\n",__func__);
								CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
								return CW_FALSE;
							}else{
							}
							radioID=WTPIndex*4+result_l_radioid;
							wid_syslog_debug_debug(WID_DEFAULT,"channel is %d\n",channel);
							wid_syslog_debug_debug(WID_DEFAULT,"channel result_l_radioid is %d \n",result_l_radioid);
							if(AC_RADIO[radioID] != NULL){
								AC_RADIO[radioID]->Radio_Chan = channel;
							}
						}else if((txpowerof_or_channel_state == 2)&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.u.RadioInfo.Radio_Op == Radio_TXP))
						{
							int txpower = 0x00ff&resultCode;
							result_l_radioid=(resultCode>>16)&0x000f;/*l_rd*/
							if(!check_l_radioid_func(result_l_radioid)){
								wid_syslog_err("%s\n",__func__);
								CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
								return CW_FALSE;
							}else{
							}
							radioID=WTPIndex*4+result_l_radioid;
							wid_syslog_debug_debug(WID_DEFAULT,"txpower is %d\n",txpower);
							wid_syslog_debug_debug(WID_DEFAULT,"txpower result_l_radioid is %d \n",result_l_radioid);
							if(AC_RADIO[radioID] != NULL){
								AC_RADIO[radioID]->Radio_TXP = txpower;
							}
						}
					}
				}
				WID_FREE(AC_WTP[WTPIndex]->ControlWait);				
				AC_WTP[WTPIndex]->ControlWait = NULL;
			}else{			
				if(AC_WTP[WTPIndex]->ControlWait != NULL){
					WID_FREE(AC_WTP[WTPIndex]->ControlWait);
					AC_WTP[WTPIndex]->ControlWait = NULL;
				}
				wid_syslog_info("config update response something wrong\n");
			}
			
			reset = CW_FALSE;
			if(AC_WTP[WTPIndex]->resetflag == 1)
			{
				reset = CW_TRUE;
			}
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
			break;
		}
		case CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_REQUEST:{
			CWProtocolChangeStateEventRequestValues *valuesPtr = NULL;
			if(!(CWParseChangeStateEventRequestMessage2(msgPtr, controlVal.msgElemsLen, &valuesPtr))) {
				if (valuesPtr) {
					WID_FREE(valuesPtr);
				}
				return CW_FALSE;
			}
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			if(!(CWSaveChangeStateEventRequestMessage(valuesPtr, &(gWTPs[WTPIndex].WTPProtocolManager))))
				return CW_FALSE;
			if(!(CWAssembleChangeStateEventResponse(&messages, &messagesCount, gWTPs[WTPIndex].pathMTU, controlVal.seqNum)))  {
				if (messages) {
					WID_FREE(messages);
				}
				return CW_FALSE;
			}
			toSend = CW_TRUE;
			break;
		}
		case CW_MSG_TYPE_VALUE_ECHO_REQUEST:{
			if(!(CWParseEchoRequestMessage(msgPtr, controlVal.msgElemsLen,WTPIndex)))
				return CW_FALSE;
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			/*fengwenchao modify begin 20111117 for GM-3*/
			if(!(CWAssembleEchoResponse(&messages, &messagesCount, gWTPs[WTPIndex].pathMTU, controlVal.seqNum))) { 
				if (messages) {
					WID_FREE(messages);
				}
				return CW_FALSE;
			}
			else
			{
				if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->heart_time.heart_statistics_switch == 1))
					wid_prase_heart_time_avarge(WTPIndex);
			}
			/*fengwenchao modify  end*/	
			toSend = CW_TRUE;
			break;
		}
		/*case CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_RESPONSE:
			CWACStopRetransmission(WTPIndex);
			CWRestartNeighborDeadTimer(WTPIndex);
			break;
		case CW_MSG_TYPE_VALUE_STATUS_CONFIGURATION_RESPONSE:
			CWACStopRetransmission(WTPIndex);
			CWRestartNeighborDeadTimer(WTPIndex);
			break;
		case CW_MSG_TYPE_VALUE_DATA_TRANSFER_REQUEST:
			CWRestartNeighborDeadTimer(WTPIndex);
			break;
		*/
		case CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST:{
			//printf("way CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST wtpid=%d\n",WTPIndex);
			CWProtocolWTPEventRequestValues valuesPtr;
			int eth_num = 0;
			if(!(CWParseWTPEventRequestMessage(msgPtr, controlVal.msgElemsLen, &valuesPtr,WTPIndex)))
				return CW_FALSE;
			//trap information
			if((valuesPtr.WTPOperationalStatisticsCount != 0)&&(valuesPtr.WTPOperationalStatistics[0].ElectrifyRegisterCircle != 0))
			{
				AC_WTP[WTPIndex]->ElectrifyRegisterCircle = valuesPtr.WTPOperationalStatistics[0].ElectrifyRegisterCircle;
				
					if(gtrapflag>=4){
						wid_dbus_trap_wtp_electrify_register_circle(WTPIndex, valuesPtr.WTPOperationalStatistics[0].ElectrifyRegisterCircle);
					}	
			}
			if(valuesPtr.WTPOperationalStatisticsCount != 0)
			{
					if((gtrapflag>=1)&&(valuesPtr.WTPOperationalStatistics[0].ColdStart == 1)){
							wid_dbus_trap_wtp_code_start(WTPIndex);
					}
					AC_WTP[WTPIndex]->ap_mask_new = valuesPtr.WTPOperationalStatistics[0].ipmask;	
					AC_WTP[WTPIndex]->ap_gateway = valuesPtr.WTPOperationalStatistics[0].ipgateway;
					AC_WTP[WTPIndex]->ap_dnsfirst = valuesPtr.WTPOperationalStatistics[0].ipdnsfirst;
					AC_WTP[WTPIndex]->ap_dnssecend = valuesPtr.WTPOperationalStatistics[0].ipdnssecend;
				 	if(AC_WTP[WTPIndex]->cpuType && valuesPtr.WTPOperationalStatistics[0].cpuType){
						memcpy(AC_WTP[WTPIndex]->cpuType,valuesPtr.WTPOperationalStatistics[0].cpuType,strlen((char *)valuesPtr.WTPOperationalStatistics[0].cpuType));
				 		}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}
				 	if(AC_WTP[WTPIndex]->flashType && valuesPtr.WTPOperationalStatistics[0].flashType){
						memcpy(AC_WTP[WTPIndex]->flashType,valuesPtr.WTPOperationalStatistics[0].flashType,strlen((char *)valuesPtr.WTPOperationalStatistics[0].flashType));
				 		}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}
				 	if(AC_WTP[WTPIndex]->memType && valuesPtr.WTPOperationalStatistics[0].memType){
						memcpy(AC_WTP[WTPIndex]->memType,valuesPtr.WTPOperationalStatistics[0].memType,strlen((char *)valuesPtr.WTPOperationalStatistics[0].memType));
				 		}
					else
						{
						wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
						}
					for(eth_num=0;eth_num<valuesPtr.WTPOperationalStatistics[0].eth_count;eth_num++){
						AC_WTP[WTPIndex]->apifinfo.eth[eth_num].eth_rate = valuesPtr.WTPOperationalStatistics[0].eth_rate;
					}
			}
			if((valuesPtr.neighbor_ap_infos != NULL)&&(valuesPtr.neighbor_ap_infos->DeviceInterference == 1))
			{
				if(gtrapflag>=3){
					wid_dbus_trap_wtp_device_interference(WTPIndex);
				}
			}
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			if(!(CWSaveWTPEventRequestMessage(&valuesPtr, &(gWTPs[WTPIndex].WTPProtocolManager))))
				return CW_FALSE;
			if(!(CWAssembleWTPEventResponse(&messages, &messagesCount, gWTPs[WTPIndex].pathMTU, controlVal.seqNum))) 
 				return CW_FALSE;
			toSend = CW_TRUE;	
			
			if((AC_WTP[WTPIndex]->NeighborAPInfos == NULL)||((rrm_rid == 1)&&(AC_WTP[WTPIndex]->NeighborAPInfos2 == NULL)))
			{
				//printf("##004 wtp info wtp id = %d##\n",WTPIndex);
				if(gapscanset.opstate == 1)
				{
					//CWThreadMutexLock(&(gACChannelMutex));					
					CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
					if(rrm_rid == 0){
						AC_WTP[WTPIndex]->NeighborAPInfos = valuesPtr.neighbor_ap_infos;
						wid_mark_rogue_ap(AC_WTP[WTPIndex]->NeighborAPInfos);
						if(gtrapflag>=25)
						{
							wid_count_rogue_ap((AC_WTP[WTPIndex]->NeighborAPInfos),WTPIndex);	
							wid_parse_neighbor_ap_list(WTPIndex);

						}
						
						if((gtrapflag>=24)&&(AC_WTP[WTPIndex]->NeighborAPInfos != NULL))
						{
							channel_interference_detected(WTPIndex);
						}
					}else{
						AC_WTP[WTPIndex]->NeighborAPInfos2 = valuesPtr.neighbor_ap_infos;
						wid_mark_rogue_ap(AC_WTP[WTPIndex]->NeighborAPInfos2);
					}
					valuesPtr.neighbor_ap_infos = NULL;
						
					CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));
					if(channel_state == 1){
						
						if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
							wid_syslog_crit("can't lock threadmutex");
							exit(1);
							}
						if(rrm_rid == 0){
							scanningWTPs++;
							if(scanningWTPs >= gActiveWTPs){						
								CWThreadMutexLock(&(gACChannelMutex));
									CWSignalThreadCondition(&gACChannelWait);
								CWThreadMutexUnlock(&gACChannelMutex);
								scanningWTPs = 0;
							}				
						}else{
							scanningWTPs1++;
							if(scanningWTPs1 >= gActiveWTPs){	
								wid_syslog_debug_debug(WID_DEFAULT,"notice channel gActiveWTPs %d scanningWTPs1 %d\n",gActiveWTPs,scanningWTPs1);
								CWThreadMutexLock(&(gACChannelMutex2));
									CWSignalThreadCondition(&gACChannelWait2);
								CWThreadMutexUnlock(&gACChannelMutex2);
								scanningWTPs1 = 0;
							}				
						}		
						CWThreadMutexUnlock(&gActiveWTPsMutex);
					}
					if(txpower_state == 1)
					{
						CWThreadMutexLock(&(gACTxpowerMutex));
						CWSignalThreadCondition(&gACTxpowerWait);
						CWThreadMutexUnlock(&gACTxpowerMutex);
					}
					if(gapscanset.countermeasures_switch == 1)
					{
						if(rrm_rid == 0)
							wid_count_countermeasure_rogue_ap((AC_WTP[WTPIndex]->NeighborAPInfos),WTPIndex);	
					}

				}
				else
				{
					CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));

					if((AC_WTP[WTPIndex]->NeighborAPInfos) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos));
					}
					if((AC_WTP[WTPIndex]->rouge_ap_infos) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->rouge_ap_infos));
					}
					if((valuesPtr.neighbor_ap_infos) != NULL)
					{
						destroy_ap_info_list(&(valuesPtr.neighbor_ap_infos));
					}
					if((AC_WTP[WTPIndex]->NeighborAPInfos2) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos2));
					}
					
					CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));	
					if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
						wid_syslog_crit("can't lock threadmutex");
						exit(1);
						}
					scanningWTPs = 0;		
					
					CWThreadMutexUnlock(&gActiveWTPsMutex);
				}
			}
			else
			{
				if(gapscanset.opstate == 1)
				{
					//CWThreadMutexLock(&(gACChannelMutex));					
					CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
					wid_mark_rogue_ap(valuesPtr.neighbor_ap_infos);
					if(rrm_rid == 0){
						if(AC_WTP[WTPIndex]->NeighborAPInfos){
							if(gtrapflag>=25)
							{
								wid_count_rogue_ap((valuesPtr.neighbor_ap_infos),WTPIndex);
							}
							merge_ap_list((AC_WTP[WTPIndex]->NeighborAPInfos),&(valuesPtr.neighbor_ap_infos),WTPIndex);
							if(AC_WTP[WTPIndex]->NeighborAPInfos->neighborapInfosCount == 0)
							{
								CW_FREE_OBJECT_WID(AC_WTP[WTPIndex]->NeighborAPInfos);
							}	
							
							if(gtrapflag>=25)
							{
								wid_parse_neighbor_ap_list(WTPIndex);

							}
							
							if((gtrapflag>=24)&&(AC_WTP[WTPIndex]->NeighborAPInfos != NULL))
							{
								channel_interference_detected(WTPIndex);
							}
						}else {
							if((valuesPtr.neighbor_ap_infos) != NULL)
							{
								destroy_ap_info_list(&(valuesPtr.neighbor_ap_infos));
							}
						}
					}else{
						if(AC_WTP[WTPIndex]->NeighborAPInfos2){						
							merge_ap_list((AC_WTP[WTPIndex]->NeighborAPInfos2),&(valuesPtr.neighbor_ap_infos),WTPIndex);			
							if(AC_WTP[WTPIndex]->NeighborAPInfos2->neighborapInfosCount == 0)
							{
								CW_FREE_OBJECT_WID(AC_WTP[WTPIndex]->NeighborAPInfos2);
							}	
						}else{
							if((valuesPtr.neighbor_ap_infos) != NULL)
							{
								destroy_ap_info_list(&(valuesPtr.neighbor_ap_infos));
							}
						}
					}

					CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));					
					if(channel_state == 1){
						
						if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
							wid_syslog_crit("can't lock threadmutex");
							exit(1);
							}
						if(rrm_rid == 0){
							scanningWTPs++;
							if(scanningWTPs >= gActiveWTPs){						
								CWThreadMutexLock(&(gACChannelMutex));
									CWSignalThreadCondition(&gACChannelWait);
								CWThreadMutexUnlock(&gACChannelMutex);
								scanningWTPs = 0;
							}				
						}else{
							scanningWTPs1++;
							
							wid_syslog_debug_debug(WID_DEFAULT,"notice channel 2 gActiveWTPs %d scanningWTPs1 %d\n",gActiveWTPs,scanningWTPs1);
							if(scanningWTPs1 >= gActiveWTPs){						
								CWThreadMutexLock(&(gACChannelMutex2));
									CWSignalThreadCondition(&gACChannelWait2);
								CWThreadMutexUnlock(&gACChannelMutex2);
								scanningWTPs1 = 0;
							}				

						}		
						CWThreadMutexUnlock(&gActiveWTPsMutex);
					}
					
					if(txpower_state == 1)
					{
						CWThreadMutexLock(&(gACTxpowerMutex));
						tx_wtpid = WTPIndex;
						CWSignalThreadCondition(&gACTxpowerWait);
						CWThreadMutexUnlock(&gACTxpowerMutex);

					}
					if(gapscanset.countermeasures_switch == 1)
					{
						wid_count_countermeasure_rogue_ap((AC_WTP[WTPIndex]->NeighborAPInfos),WTPIndex);	
					}

				}
				else
				{
					CWThreadMutexLock(&(gWTPs[WTPIndex].RRMThreadMutex));
					
					if((AC_WTP[WTPIndex]->NeighborAPInfos) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos));
					}
					if((AC_WTP[WTPIndex]->rouge_ap_infos) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->rouge_ap_infos));
					}
					if((valuesPtr.neighbor_ap_infos) != NULL)
					{
						destroy_ap_info_list(&(valuesPtr.neighbor_ap_infos));
					}	
					if((AC_WTP[WTPIndex]->NeighborAPInfos2) != NULL)
					{
						destroy_ap_info_list(&(AC_WTP[WTPIndex]->NeighborAPInfos2));
					}

					CWThreadMutexUnlock(&(gWTPs[WTPIndex].RRMThreadMutex));	
					
					if(!CWErr(CWThreadMutexLock(&gActiveWTPsMutex))) {
						wid_syslog_crit("can't lock threadmutex");
						exit(1);
						}
					scanningWTPs = 0;					
					CWThreadMutexUnlock(&gActiveWTPsMutex);

				}
			}
	
			
			if(valuesPtr.CWStationInfo != NULL)
			{
				AsdWsm_StationOp(WTPIndex,(valuesPtr.CWStationInfo),WID_DEL);
				CW_FREE_OBJECT_WID(valuesPtr.CWStationInfo->mac_addr);
				CW_FREE_OBJECT_WID(valuesPtr.CWStationInfo);
			}
			if(valuesPtr.wids_device_infos != NULL)
			{
				//printf("0000000000000000000000000000000000000\n");
				if(AC_WTP[WTPIndex]->wids_device_list == NULL)
				{
					CWThreadMutexLock(&(gWTPs[WTPIndex].WIDSThreadMutex));
					AC_WTP[WTPIndex]->wids_device_list = valuesPtr.wids_device_infos;
					
					valuesPtr.wids_device_infos = NULL;
					CWThreadMutexUnlock(&(gWTPs[WTPIndex].WIDSThreadMutex));
				}
				else
				{
					//printf("111111111111111111111111111111111111111111\n");
					CWThreadMutexLock(&(gWTPs[WTPIndex].WIDSThreadMutex));
					merge_wids_list((AC_WTP[WTPIndex]->wids_device_list),&(valuesPtr.wids_device_infos),WTPIndex);
					CWThreadMutexUnlock(&(gWTPs[WTPIndex].WIDSThreadMutex));
				}
			}
			if(valuesPtr.ApReportStaInfo != NULL){/*wuwl add 20100126*/

				AsdWsm_ap_report_sta_info(WTPIndex,(valuesPtr.ApReportStaInfo),valuesPtr.ApReportStaInfo->op);
				//CW_FREE_OBJECT(valuesPtr.ApReportStaInfo->mac);
				CW_FREE_OBJECT_WID(valuesPtr.ApReportStaInfo);
				
			}
			//added end
			break;
		}
		case CW_MSG_TYPE_VALUE_IEEE80211_WLAN_CONFIG_RESPONSE:{
			//printf("WLAN CONFIGURATION RESPONSE\n");
			wid_syslog_debug_debug(WID_DEFAULT,"receive WLAN CONFIGURATION RESPONSE\n");
			CWProtocolResultCode resultCode;
			WID_BSS BSS;
			unsigned char BSSID[6];
			unsigned int bssindex;
			memset(BSSID, 0, 6);
			BSS.BSSID = NULL;
			if(!(CWParseWlanConfigurationResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode, &BSS)))
				return CW_FALSE;
			int zero = memcmp(BSS.BSSID, BSSID,6);
			if(zero != 0){
						//printf("WTPID %d\n",WTPIndex);
						//printf("wlanid %d,radioid %d\n",BSS.WlanID,BSS.Radio_L_ID);						
						//printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",BSS.BSSID[0],BSS.BSSID[1],BSS.BSSID[2],BSS.BSSID[3],BSS.BSSID[4],BSS.BSSID[5]);
						wid_syslog_debug_debug(WID_DEFAULT,"WTPID %d\n",WTPIndex);
						wid_syslog_debug_debug(WID_DEFAULT,"wlanid %d,radioid %d\n",BSS.WlanID,BSS.Radio_L_ID);
						wid_syslog_debug_debug(WID_DEFAULT,"BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",BSS.BSSID[0],BSS.BSSID[1],BSS.BSSID[2],BSS.BSSID[3],BSS.BSSID[4],BSS.BSSID[5]);
						int i1=0;
						if(!check_wlanid_func(BSS.WlanID)){
							wid_syslog_err("%s\n",__func__);
							//return CW_FALSE;
						}else{
						}
						if(!check_l_radioid_func(BSS.Radio_L_ID)){
							wid_syslog_err("%s\n",__func__);
							//return CW_FALSE;
						}else{
						}
						if((BSS.Radio_L_ID < L_RADIO_NUM)&&(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]!=NULL)&&(check_wlanid_func(BSS.WlanID))){    //fengwenchao modify 20110427
						 for(i1=0;i1<L_BSS_NUM;i1++){
						 	if((AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1] == NULL))
								continue;
							 else if(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->WlanID == BSS.WlanID
							 	      && (AC_WLAN[BSS.WlanID] != NULL && AC_WLAN[BSS.WlanID]->want_to_delete != 1 && AC_WLAN[BSS.WlanID]->Status != 1)){		/* Huangleilei add for AXSSZFI-1695 */
								memcpy(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID,BSS.BSSID,6);
								AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->State = 1;
								AsdWsm_BSSOp(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSIndex, WID_ADD, 1);
								bssindex = AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSIndex;
								if(!check_bssid_func(bssindex)){
									continue;
								}
								wid_asd_bss_traffic_limit(bssindex);
								if(AC_BSS[bssindex]->BSS_TUNNEL_POLICY == CW_802_IPIP_TUNNEL)
								{
									add_ipip_tunnel(bssindex);
								}
								//printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[0],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[1],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[2],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[3],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[4],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[5]);
								wid_syslog_debug_debug(WID_DEFAULT,"BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[0],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[1],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[2],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[3],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[4],AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSID[5]);
								CWThreadMutexLock(&MasterBak);
								/*ht changed for update bss info in local mode*/
								if(1)//((AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)||(AC_BSS[bssindex]->BSS_IF_POLICY == WLAN_INTERFACE))
								{
									struct sockaddr *sa;
									struct sockaddr* sa2;
									int ret = -1;	
									unsigned char wlan_id = 0;
									IF_info ifinfo;
									int fd = open("/dev/wifi0", O_RDWR);
									wid_syslog_debug_debug(WID_DEFAULT,"*** fd:%d ***\n",fd);

									if(fd < 0)
									{
										CWThreadMutexUnlock(&MasterBak);
										return -1;//create failure
									}
									memset(&ifinfo, 0, sizeof(IF_info));
									sa = (struct sockaddr *)&gWTPs[WTPIndex].address;
									sa2 = (struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr);
									ifinfo.acport = htons(5247);
									ifinfo.BSSIndex = bssindex;
									ifinfo.WLANID = AC_BSS[bssindex]->WlanID;
									wlan_id = AC_BSS[bssindex]->WlanID; 
									if((AC_WLAN[wlan_id] != NULL)&&(AC_WLAN[wlan_id]->SecurityType == OPEN)&&(AC_WLAN[wlan_id]->EncryptionType == NONE))
										ifinfo.protect_type = 0;
									else
										ifinfo.protect_type = 1;
									ifinfo.vrid = local*MAX_INSTANCE+ vrrid;
									if(AC_BSS[bssindex]->BSS_TUNNEL_POLICY == CW_802_DOT_3_TUNNEL){
	 										ifinfo.f802_3 = 1;
	 									wid_syslog_info("ifinfo.f802_3 = %d.\n",ifinfo.f802_3);
									}else{
										ifinfo.f802_3 = 0;
										wid_syslog_info("AC_BSS[%d]->BSS_TUNNEL_POLICY =%d, not dot3,set ifinfo.f802_3 = %d.\n",bssindex,AC_BSS[bssindex]->BSS_TUNNEL_POLICY,ifinfo.f802_3);
									}
									ifinfo.wsmswitch = wsmswitch;
									ifinfo.vlanSwitch = vlanSwitch;
									if((AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE)|| (AC_BSS[bssindex]->BSS_IF_POLICY == BSS_INTERFACE_EBR) || (AC_BSS[bssindex]->BSS_IF_POLICY == WLAN_INTERFACE))
										ifinfo.if_policy = 1;
									else
										ifinfo.if_policy = 0;
									
									if(AC_BSS[bssindex]->vlanid != 0){
										ifinfo.vlanid = AC_BSS[bssindex]->vlanid;
									}else if(AC_BSS[bssindex]->wlan_vlanid != 0){
										ifinfo.vlanid = AC_BSS[bssindex]->wlan_vlanid;
									}else{
										ifinfo.vlanid = 0;
									}
									if(AC_WTP[WTPIndex]->WTPMAC != NULL){
										memcpy(ifinfo.apmac, AC_WTP[WTPIndex]->WTPMAC, MAC_LEN);
										}
									else
										{
										wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
										}
									if( AC_BSS[bssindex]->BSSID != NULL){
										memcpy(ifinfo.bssid,  AC_BSS[bssindex]->BSSID, MAC_LEN);
										}
									else
										{
										wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
										}
									
										memcpy(ifinfo.ifname, AC_WTP[WTPIndex]->BindingIFName,strlen(AC_WTP[WTPIndex]->BindingIFName));
										
									if(AC_WTP[WTPIndex]->WTPNAME != NULL){
										memcpy(ifinfo.apname,AC_WTP[WTPIndex]->WTPNAME,strlen(AC_WTP[WTPIndex]->WTPNAME));
										}
									else
										{
										wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
										}
									if(AC_WLAN[wlan_id] != NULL){
										if(AC_WLAN[wlan_id]->ESSID != NULL){
											memcpy(ifinfo.essid ,AC_BSS[bssindex]->SSID ,strlen(AC_BSS[bssindex]->SSID));
											}
										else
											{
											wid_syslog_err("%s %d pointe is NULL\n",__FUNCTION__,__LINE__);
											}
										ifinfo.Eap1XServerSwitch = AC_WLAN[wlan_id]->eap_mac_switch;
										memset(ifinfo.Eap1XServerMac,0,MAC_LEN);
										memcpy(ifinfo.Eap1XServerMac,AC_WLAN[wlan_id]->eap_mac2,MAC_LEN);
										wid_syslog_debug_debug(WID_DBUS,"AC_WLAN[%d]->eap_mac2=%02X:%02X:%02X:%02X:%02X:%02X.\n",wlan_id,AC_WLAN[wlan_id]->eap_mac2[0],AC_WLAN[wlan_id]->eap_mac2[1],AC_WLAN[wlan_id]->eap_mac2[2],AC_WLAN[wlan_id]->eap_mac2[3],AC_WLAN[wlan_id]->eap_mac2[4],AC_WLAN[wlan_id]->eap_mac2[5]);
									}
									char __str[128];			
									memset(__str,0,128);
									char *str = "lo";	//fengwenchao modify 20110525
									str = sock_ntop_r(((struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr)), __str);
									wid_syslog_info("WTP %d on Interface %s (%d)\n",WTPIndex, str, gWTPs[WTPIndex].interfaceIndex);
									if(sa->sa_family != AF_INET6){										
										ifinfo.isIPv6 = 0;
										ifinfo.apip = ((struct sockaddr_in *)sa)->sin_addr.s_addr;
										ifinfo.apport = ((struct sockaddr_in *)sa)->sin_port +1;
										struct sockaddr_in	*sin = (struct sockaddr_in *) sa2;
										unsigned int v_ip = sin->sin_addr.s_addr;
										wid_syslog_info("v_ip %d.%d.%d.%d.\n", \
													(v_ip >> 24) & 0xFF, (v_ip >> 16) & 0xFF,(v_ip >> 8) & 0xFF, v_ip & 0xFF);
										ifinfo.acip = v_ip;
									}else{
										ifinfo.isIPv6 = 1;
										if( &((struct sockaddr_in6 *) sa)->sin6_addr != NULL ){
											memcpy(ifinfo.apipv6,&((struct sockaddr_in6 *) sa)->sin6_addr,sizeof(struct in6_addr));
											}
										else
											{
											wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
											}
										ifinfo.apport = ((struct sockaddr_in6 *)sa)->sin6_port +1;
										if(&((struct sockaddr_in6 *) sa2)->sin6_addr != NULL){
											memcpy(ifinfo.acipv6,&((struct sockaddr_in6 *) sa2)->sin6_addr,sizeof(struct in6_addr));
											}
										else
											{
											wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
											}
									}

									ret = ioctl(fd, WIFI_IOC_IF_UPDATE, &ifinfo);
									
									wid_syslog_debug_debug(WID_DEFAULT,"*** WIFI_IOC_IF_UPDATE ret:%d ***\n",ret);
									close(fd);
									if(ret < 0)
									{
										CWThreadMutexUnlock(&MasterBak);
										return -1;
									}

								}
								struct bak_sock *tmp = bak_list;
								if(tmp!=NULL){
									bak_add_del_bss(tmp->sock,B_ADD,AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->BSS[i1]->BSSIndex);
								}
								CWThreadMutexUnlock(&MasterBak);

								int type = 2;//auto
								int flag = 1;//enable
								if(gtrapflag>=4)
									wid_dbus_trap_ap_ath_error(WTPIndex,BSS.Radio_L_ID,BSS.WlanID,type,flag);
							}
						 }
						}
			}
			CWACStopRetransmission(WTPIndex);
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			
			if(resultCode == CW_PROTOCOL_SUCCESS)
			{	
				if((zero != 0)&&(BSS.Radio_L_ID < L_RADIO_NUM)){
					//printf("WLAN of %d create success\n",WTPIndex);
					if((check_l_radioid_func(BSS.Radio_L_ID))&&(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]!=NULL)){  //fengwenchao add 20110427
						AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->AdStat = 1;
						AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->OpStat = 1;
						wid_syslog_debug_debug(WID_DEFAULT,"wtp %d create wlan success\n",WTPIndex);
						wid_syslog_debug_debug(WID_DEFAULT,"######### WTP %d Enter Run State#####\n",WTPIndex);
						if(AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->channelsendtimes==1)
						{	
							wid_syslog_debug_debug(WID_DEFAULT,"channel_cont cmd expand :channel 0 (before)");
							//int j;

							//for(j=0;j<AC_WTP[WTPIndex]->RadioCount;j++)
							{	

								int RadioID;
								msgq msg;				
								struct msgqlist *elem1;

								RadioID = WTPIndex*L_RADIO_NUM + BSS.Radio_L_ID ;	
								if(AC_RADIO[RadioID]->auto_channel_cont == 0){

									AC_RADIO[RadioID]->Radio_Chan= 0;				
									AC_RADIO[RadioID]->auto_channel_cont = 0;	

									memset((char*)&msg, 0, sizeof(msg));
									msg.mqid = WTPIndex%THREAD_NUM+1;
									msg.mqinfo.WTPID = WTPIndex;
									msg.mqinfo.type = CONTROL_TYPE;
									msg.mqinfo.subtype = Radio_S_TYPE;
									msg.mqinfo.u.RadioInfo.Radio_Op = Radio_Channel;
									msg.mqinfo.u.RadioInfo.Radio_L_ID = RadioID%L_RADIO_NUM;
									msg.mqinfo.u.RadioInfo.Radio_G_ID = RadioID;


									elem1 = (struct msgqlist*)WID_MALLOC(sizeof(struct msgqlist));

									if(elem1 == NULL){
										wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
										perror("malloc");
										return 0;
									}
									memset((char*)&(elem1->mqinfo), 0, sizeof(msgqdetail));

									elem1->next = NULL;

									memcpy((char*)&(elem1->mqinfo),(char*)&(msg.mqinfo),sizeof(msg.mqinfo));

									WID_INSERT_CONTROL_LIST(WTPIndex, elem1);
									wid_syslog_debug_debug(WID_DEFAULT,"channel_cont cmd expand;radio %d :channel 0 (after)",RadioID);				
								}
							}
							AC_WTP[WTPIndex]->WTP_Radio[BSS.Radio_L_ID]->channelsendtimes--;

						}
					}	
					WID_FREE(BSS.BSSID);
					//AC_WTP[WTPIndex]->WTP_Radio[0]->BSS[0]->State = 1;
				}
				else if ((zero == 0) /*&& (BSS.Radio_L_ID < L_RADIO_NUM)*/)
				{
					
					wid_syslog_debug_debug(WID_DEFAULT, "%s %d Radio_L_ID: %d", __func__, __LINE__, BSS.Radio_L_ID);
					wid_syslog_debug_debug(WID_DEFAULT, "%s %d WlanID: %d", __func__, __LINE__, BSS.WlanID);
					wid_syslog_debug_debug(WID_DEFAULT, "%s %d BSSID: %02X:%02X:%02X:%02X:%02X:%02X", __func__, __LINE__, 
						BSS.BSSID[0],
						BSS.BSSID[1],
						BSS.BSSID[2],
						BSS.BSSID[3],
						BSS.BSSID[4],
						BSS.BSSID[5]);
					/*
					 * This branch used to set this BSS's enable_wlan_flag,
					 * when ac receive the disable-wlan-operation's success response from the special wtp.
					 */

					/*
					 * The infomation from the ap, the wlanid is valid, but, the radioID is invalid.
					 */
					
					/* if the user want to delete this wlan, set this flag to 1, 
					 * and ignore this wlan's information for all the new created wtps
					 * Huangleilei add it for AXSSZFI-1622 
					 */
					wid_syslog_debug_debug(WID_DEFAULT,"begin to clear AC_BSS...\n");
					unsigned int BSSLIndex = 0;
					unsigned int Radio_L_ID = 0;
					int m1;
					int m = 0;
					
					for(m1 = 0; m1 < L_BSS_NUM; m1++)
					{
						for (m = 0; m < L_RADIO_NUM; m++)
						{
							#if 0
							if ((AC_WTP[WTPIndex]->WTP_Radio[m] != NULL) 
								&& (AC_WTP[WTPIndex]->WTP_Radio[m]->del_flag == 1))
							{
								struct wlanid * wlan_point = NULL;
								wlan_point = AC_RADIO[WTPIndex * L_RADIO_NUM + m]->Wlan_Id;
								while ( wlan_point != NULL)
								{
									if (wlan_point->wlanid == BSS.WlanID)
									{
										Radio_L_ID = m;
										wid_syslog_debug_debug(WID_DEFAULT, "Radio_L_ID: %d", Radio_L_ID);
										break;
									}
									wlan_point = wlan_point->next;
								}
							}
							#endif
							if (AC_WTP[WTPIndex]->WTP_Radio[m] != NULL
								&& AC_WTP[WTPIndex]->WTP_Radio[m]->BSS[m1] != NULL 
								&& AC_WTP[WTPIndex]->WTP_Radio[m]->BSS[m1]->WlanID == BSS.WlanID)
							{
								struct wlanid * wlan_point = NULL;
								wlan_point = AC_RADIO[WTPIndex * L_RADIO_NUM + m]->Wlan_Id;
								while ( wlan_point != NULL)
								{
									if (wlan_point->wlanid == BSS.WlanID)
									{
										Radio_L_ID = m;
										wid_syslog_debug_debug(WID_DEFAULT, "Radio_L_ID: %d", Radio_L_ID);
										break;
									}
									wlan_point = wlan_point->next;
								}
								BSSLIndex = m1;
								Radio_L_ID = m;
								AC_WTP[WTPIndex]->WTP_Radio[Radio_L_ID]->BSS[BSSLIndex]->enable_wlan_flag = 0;
								wid_syslog_debug_debug(WID_DEFAULT, "%s %d set wtp %d bss %d\'s enable_wlan_flag to \'0(disable)\'", __func__, __LINE__, WTPIndex, BSSLIndex);
								break;
							}
						}
					}
					/*
					if (AC_WTP[WTPIndex]->WTP_Radio[Radio_L_ID] != NULL
						&& AC_WTP[WTPIndex]->WTP_Radio[Radio_L_ID]->BSS[BSSLIndex] != NULL)
					{
					}*/
				}
				else{
					//wid_syslog_debug_debug(WID_DEFAULT,"WLAN delete success\n");
				}
			}
			else {
				//wid_syslog_debug_debug(WID_DEFAULT,"WLAN op failed\n");
				}
			if((AC_WTP[WTPIndex]->ControlWait != NULL)&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.subtype == WLAN_S_TYPE)){
				CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
				WID_FREE(AC_WTP[WTPIndex]->ControlWait);
				AC_WTP[WTPIndex]->ControlWait = NULL;				
				CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
			//	printf("wlan response\n");
				wid_syslog_debug_debug(WID_DEFAULT,"wlan response\n");
			}else{		
				CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
				if(AC_WTP[WTPIndex]->ControlWait != NULL){
					WID_FREE(AC_WTP[WTPIndex]->ControlWait);
					AC_WTP[WTPIndex]->ControlWait = NULL;
				}
				CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
				wid_syslog_info("wlan response something wrong\n");
			}
			break;

		}
		
		case CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE:{			
			//printf("CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE\n");
			wid_syslog_debug_debug(WID_DEFAULT,"CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE\n");
			CWProtocolResultCode resultCode;
			if(!(CWParseStaConfigurationResponseMessage(msgPtr, controlVal.msgElemsLen,WTPIndex, &resultCode)))
				return CW_FALSE;
			if(resultCode == CW_PROTOCOL_SUCCESS)
				wid_syslog_debug_debug(WID_DEFAULT,"sta config success\n");			
			else
			{
				wid_syslog_debug_debug(WID_DEFAULT,"sta config failed\n");
				AsdWsm_StationOpNew(WTPIndex,AC_WTP[WTPIndex]->ControlWait->mqinfo.u.StaInfo.STAMAC,STA_LEAVE_REPORT,1); //weichao add
			}
			CWACStopRetransmission(WTPIndex);
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}	
			
			CWThreadMutexLock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));
			if((AC_WTP[WTPIndex]->ControlWait != NULL)&&(AC_WTP[WTPIndex]->ControlWait->mqinfo.subtype == STA_S_TYPE)){
				WID_FREE(AC_WTP[WTPIndex]->ControlWait);
				AC_WTP[WTPIndex]->ControlWait = NULL;				
			//	printf("sta response\n");
				wid_syslog_debug_debug(WID_DEFAULT,"sta response\n");
			}else{				
				if(AC_WTP[WTPIndex]->ControlWait != NULL){
					WID_FREE(AC_WTP[WTPIndex]->ControlWait);
					AC_WTP[WTPIndex]->ControlWait = NULL;
				}
				wid_syslog_info("sta response something wrong\n");
			}
			CWThreadMutexUnlock(&(gWTPs[WTPIndex].WTPThreadControllistMutex));

			break;
		}
		case CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST:{
			
			CWDiscoveryRequestValues values;		
			CWWTPVendorInfos *valPtr;
			int i;
			
			
			if(!(CWParseDiscoveryRequestMessageInRun(msgPtr, controlVal.msgElemsLen, &values)))
				return CW_FALSE;
			valPtr = &(values.WTPBoardData);
			
			
			for(i = 0; i < valPtr->vendorInfosCount; i++){
				if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER){
					char * sn = (char *)((valPtr->vendorInfos)[i].SN);
					str2higher(&sn);
					(valPtr->vendorInfos)[i].SN = (unsigned char *)sn;
				
					if(memcmp((valPtr->vendorInfos)[i].SN, AC_WTP[WTPIndex]->WTPSN, strlen(AC_WTP[WTPIndex]->WTPSN)) == 0)
					{
						continue;
					}else{
					
					
						CWDestroyDiscoveryRequestValues(&values); 
						return CW_FALSE;
					}						
				}
				else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){
		
						if(((valPtr->vendorInfos)[i].mac[0] == AC_WTP[WTPIndex]->WTPMAC[0])&&((valPtr->vendorInfos)[i].mac[1] == AC_WTP[WTPIndex]->WTPMAC[1])&&((valPtr->vendorInfos)[i].mac[2] == AC_WTP[WTPIndex]->WTPMAC[2])\
							&&((valPtr->vendorInfos)[i].mac[3] == AC_WTP[WTPIndex]->WTPMAC[3])&&((valPtr->vendorInfos)[i].mac[4] == AC_WTP[WTPIndex]->WTPMAC[4])&&((valPtr->vendorInfos)[i].mac[5] == AC_WTP[WTPIndex]->WTPMAC[5]))
						{
							continue;
						}
						else
						{
						
						
							CWDestroyDiscoveryRequestValues(&values); 
							return CW_FALSE;
						}
				}
				else
					continue;
			}
			CWDestroyDiscoveryRequestValues(&values); 
			wid_syslog_err("\n\n check discovery ,and ap quit\n\n");
			gWTPs[WTPIndex].isRequestClose = CW_TRUE;
			_CWCloseThread(WTPIndex);
			return CW_TRUE;
		}
		default: 
			//## We have an unexpected request and we have to send
			//## a corresponding response containing a failure result code
			wid_syslog_err("--> Not valid Request in Run State... we send a failure Response");
			if(!CWRestartNeighborDeadTimer(WTPIndex)) {_CWCloseThread(WTPIndex);}
			if(!(CWAssembleUnrecognizedMessageResponse(&messages, &messagesCount, gWTPs[WTPIndex].pathMTU, controlVal.seqNum, controlVal.messageTypeValue+1))) 
 				return CW_FALSE;
			toSend = CW_TRUE;
//			return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message not valid in Run State");
	}
	
	if(toSend){
		int i;
	
		if(messages == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		for(i = 0; i < messagesCount; i++) {
			//if(controlVal.messageTypeValue == CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST)
			//{
				//printf("way send message wtpid=%d sendcount i=%d\n",WTPIndex,i);
			//}
#ifdef CW_NO_DTLS
			if(!CWNetworkSendUnsafeUnconnected(	gWTPs[WTPIndex].socket, 
								&gWTPs[WTPIndex].address, 
								messages[i].msg, 
								messages[i].offset)	) {
#else
			if(!(CWSecuritySend(gWTPs[WTPIndex].session, messages[i].msg, messages[i].offset))) {
#endif
				CWFreeMessageFragments(messages, messagesCount);
				CW_FREE_OBJECT_WID(messages);
				return CW_FALSE;
			}
		}
		CWFreeMessageFragments(messages, messagesCount);
		CW_FREE_OBJECT_WID(messages);
	}

	gWTPs[WTPIndex].currentState = CW_ENTER_RUN;	
	if(AC_WTP[WTPIndex] != NULL){
		AC_WTP[WTPIndex]->WTPStat = 5;
	}else{
		return CW_FALSE;
	}
	gWTPs[WTPIndex].subState = CW_WAITING_REQUEST;
	
	if((reset)&&(AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->resetflag == 1))
	{
		reset = CW_FALSE;
		if(AC_WTP[WTPIndex] != NULL){
			AC_WTP[WTPIndex]->resetflag = 0;
		}
		wid_syslog_debug_debug(WID_DEFAULT,"change static ip close thread");
		_CWCloseThread(WTPIndex);		
		
	}

	return CW_TRUE;
}

CWBool CWACParseGenericRunMessage(int WTPIndex, CWProtocolMessage *msg, CWControlHeaderValues* controlVal)
{
	if(msg == NULL || controlVal == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(!(CWParseControlHeader(msg, controlVal))) return CW_FALSE; // will be handled by the caller

	controlVal->msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS; // skip timestamp

	if((controlVal->messageTypeValue % 2 == 1) && (controlVal->messageTypeValue != CW_MSG_TYPE_VALUE_IEEE80211_WLAN_CONFIG_RESPONSE)){			//Check if it is a request
		return CW_TRUE;	
	}
	/*wcl modify*/
	if(AC_WTP[WTPIndex]->wtp_seqnum_switch == 1)
	{
		//wid_syslog_debug_debug(WID_DEFAULT,"seqnum_switch = 1\n");
		if((gWTPs[WTPIndex].responseSeqNum != controlVal->seqNum) || (gWTPs[WTPIndex].responseType != controlVal->messageTypeValue))
		{
			wid_syslog_debug_debug(WID_DEFAULT,"gWTPs: %d\n", gWTPs[WTPIndex].responseSeqNum);
			wid_syslog_debug_debug(WID_DEFAULT,"controlVal: %d\n", controlVal->seqNum);
			CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Seq Num or Msg Type not valid!");
			return CW_FALSE;
		}
	}
	/*end*/
	//wid_syslog_debug_debug(WID_DEFAULT,"seqnum_switch = 0\n");
	return CW_TRUE;	
}

CWBool CWParseConfigurationUpdateResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode,CWRadioOperationalInfoValues* radioOpeinfo,int wtpindex)
{
	int offsetTillMessages;

	if(msgPtr == NULL || resultCode==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_DEFAULT,"Parsing Configuration Update Response...");

	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				*resultCode=CWProtocolRetrieve32(msgPtr);
				break;	
				
			case CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE:
				if(!(CWParseWTPRadioOperationalState(msgPtr, elemLen, radioOpeinfo)))
				{
					//printf("****ERROR CWParseWTPRadioOperationalState**\n");
				}
				else
				{
					//printf("****SUCCESS CWParseWTPRadioOperationalState**\n");
				}
				break;

			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				/*
				if(!(CWParseWTPextensioninfomation(msgPtr, elemLen, wtpindex)))
				{
					wid_syslog_debug_debug(WID_DEFAULT,"****ERROR CWParseWTPextensioninfomation**\n");
				}
				else
				{
					wid_syslog_debug_debug(WID_DEFAULT,"****SUCCESS CWParseWTPextensioninfomation**\n");
				}*/
				break;
				
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				break;	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	wid_syslog_debug_debug(WID_DEFAULT,"Configuration Update Response Parsed");

	return CW_TRUE;	
}

CWBool CWParseDiscoveryRequestMessageInRun(CWProtocolMessage* msgPtr, int len, CWDiscoveryRequestValues *valuesPtr)
{
	int offsetTillMessages;
	if(msgPtr == NULL || valuesPtr==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_DEFAULT,"Parsing Configuration Update Response...");
	// parse message elements
		while((msgPtr->offset - offsetTillMessages) < len) {
			unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
			unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
			
			CWParseFormatMsgElem(msgPtr,&elemType,&elemLen);		
	
	//		wid_syslog_debug_debug(WID_DEFAULT,"Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
										
			switch(elemType) {
				case CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE:
					if(!(CWParseDiscoveryType(msgPtr, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
					break;
				case CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE:
					if(!(CWParseWTPBoardData(msgPtr, elemLen, &(valuesPtr->WTPBoardData)))) return CW_FALSE; // will be handled by the caller
					break; 
				case CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE:
					if(!(CWParseWTPDescriptor(msgPtr, elemLen, &(valuesPtr->WTPDescriptor)))) return CW_FALSE; // will be handled by the caller
					break;
				case CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE:
					if(!(CWParseWTPFrameTunnelMode(msgPtr, elemLen, &(valuesPtr->frameTunnelMode)))) return CW_FALSE; // will be handled by the caller
					break;
				case CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE:
					if(!(CWParseWTPMACType(msgPtr, elemLen, &(valuesPtr->MACType)))) return CW_FALSE; // will be handled by the caller
					break;
				/*case CW_MSG_ELEMENT_WTP_RADIO_INFO_CW_TYPE:
					(*valuesPtr).radios.radiosCount++; // just count how many radios we have, so we can allocate the array
					completeMsg.offset += elemLen;
					break;
				*/
				default:
					return CWErrorRaise(CW_ERROR_INVALID_FORMAT,
						"Unrecognized Message Element");
			}
			
			//wid_syslog_debug_debug(WID_DEFAULT,"bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);
		}
		if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");

	return CW_TRUE;	


}

CWBool CWParseStaNumResponseMessage(CWProtocolMessage *msgPtr,int WTPID)
{
	TableMsg msg; 
	unsigned char radioid = 0;
	unsigned char  wlanid = 0;
	unsigned short num = 0;
	int len;
	int i = WTPID%SOCK_NUM;
	int sock = sockPerThread[i];
	char * stamac = NULL;
	memset(&msg,0,sizeof(msg));

	msg.Type = STA_TYPE;
	msg.Op = STA_CHECK_DEL;
	radioid = CWProtocolRetrieve8(msgPtr);
	wlanid = CWProtocolRetrieve8(msgPtr);
	num = CWProtocolRetrieve16(msgPtr);
	msg.u.bss_sta.radioid = radioid;
	msg.u.bss_sta.wlanid = wlanid;
	msg.u.bss_sta.wtpid = WTPID;
	len = sizeof(msg);	
	wid_syslog_debug_debug(WID_DEFAULT,"CWParseStaNumResponseMessage,num = %d\n",num);
//again:
	msg.u.bss_sta.count = num;
	memset(&msg.u.bss_sta.mac,0,sizeof(msg.u.bss_sta.mac));
	for(i = 0 ; (i < 128)&&(i < num);i++)
	{
		stamac = CWProtocolRetrieveStr(msgPtr,MAC_LEN);
		CW_COPY_MEMORY(msg.u.bss_sta.mac[i],stamac,MAC_LEN);
		if(stamac){
			WID_FREE(stamac);
			stamac = NULL;
		}
		//CW_COPY_MEMORY(msg.u.bss_sta.mac[i],CWProtocolRetrieveStr(msgPtr,MAC_LEN),MAC_LEN);	
	}
	wid_syslog_debug_debug(WID_DEFAULT,"CWParseStaNumResponseMessage\n");
	if(sendto(sock, &msg, len, 0, (struct sockaddr *) &toASD_STA.addr, toASD_STA.addrlen) < 0){
		wid_syslog_info("%s sendto %s",__func__,strerror(errno));
		perror("send(wASDSocket)");
		wid_syslog_info("WidAsd_StationInfoUpdate2\n");
		wid_syslog_debug_debug(WID_DEFAULT,"*** error AsdWsm_StationOp end***\n");
		return CW_FALSE;
	}
	/*if(i >= 64)
	{
		num = num-i;
		goto again;
	}*/
	return CW_TRUE;
}

CWBool CWParseStaConfigurationResponseMessage(CWProtocolMessage* msgPtr, int len, int WTPID,CWProtocolResultCode* resultCode)
{
	int offsetTillMessages;
	unsigned short value,valuelen;
	unsigned int vendorid;
	if(msgPtr == NULL || resultCode==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_DEFAULT,"Parsing Configuration Update Response...");

	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		wid_syslog_debug_debug(WID_DEFAULT,"elemType = %d\n",elemType);
		switch(elemType) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				*resultCode=CWProtocolRetrieve32(msgPtr);
				 break;	
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				vendorid = CWProtocolRetrieve32(msgPtr);
				
				CWParseFormatMsgElem(msgPtr, &value, &valuelen);
				wid_syslog_debug_debug(WID_DEFAULT,"value = %d\n",value);
				if(16 == value)			
					CWParseStaNumResponseMessage(msgPtr,WTPID);
				break;
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	wid_syslog_debug_debug(WID_DEFAULT,"Sta Configuration Response Parsed");

	return CW_TRUE;	
}


CWBool CWSaveConfigurationUpdateResponseMessage(CWProtocolResultCode resultCode, int WTPIndex)
{
	if(!CWBindingSaveConfigurationUpdateResponse(resultCode, WTPIndex))
		{return CW_FALSE;}
	wid_syslog_debug_debug(WID_DEFAULT,"Configuration Update Response Saved");

	return CW_TRUE;
}

CWBool CWParseWTPEventRequestMessage(CWProtocolMessage *msgPtr, int len, CWProtocolWTPEventRequestValues *valuesPtr,int wtpindex)
{
	int offsetTillMessages;
	int i=0, k=0;
	int vendorvalue = 0;
	char report_type = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if(!check_wtpid_func(wtpindex)){
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	//CW_CREATE_OBJECT_ERR(valuesPtr, CWProtocolWTPEventRequestValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_DEFAULT,"#________ WTP Event (Run) ________#");
	wid_syslog_debug_debug(WID_DEFAULT,"Parsing WTP Event Request...");
	
	valuesPtr->errorReportCount = 0;
	valuesPtr->errorReport = NULL;
	valuesPtr->duplicateIPv4 = NULL;
	valuesPtr->duplicateIPv6 = NULL;
	valuesPtr->WTPOperationalStatisticsCount = 0;
	valuesPtr->WTPOperationalStatistics = NULL;
	valuesPtr->WTPRadioStatisticsCount = 0;
	valuesPtr->WTPRadioStatistics = NULL;
	valuesPtr->WTPRebootStatistics = NULL;
	valuesPtr->CWStationInfo = NULL; //20080702 added by weiay
	valuesPtr->neighbor_ap_infos = NULL;
	valuesPtr->ap_sta_info = NULL;
	valuesPtr->wids_device_infos = NULL;
	valuesPtr->ApReportStaInfo = NULL;
	valuesPtr->wtp_extend_info = NULL;
	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		if(elemLen > len){
			wid_syslog_err("<error>len:%d > elemLen:%d\n",len,elemLen);
			//return CW_FALSE;
			return CW_TRUE;
		}

		switch(elemType) {
			case CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_CW_TYPE:
				CW_CREATE_OBJECT_ERR_WID(valuesPtr->errorReport, CWDecryptErrorReportValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				if (!(CWParseMsgElemDecryptErrorReport(msgPtr, elemLen, valuesPtr->errorReport)))
					return CW_FALSE;
				break;	
			case CW_MSG_ELEMENT_DUPLICATE_IPV4_ADDRESS_CW_TYPE:
				CW_CREATE_OBJECT_ERR_WID(valuesPtr->duplicateIPv4, WTPDuplicateIPv4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				CW_CREATE_ARRAY_ERR((valuesPtr->duplicateIPv4)->MACoffendingDevice_forIpv4,6, unsigned char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				if (!(CWParseMsgElemDuplicateIPv4Address(msgPtr, elemLen, valuesPtr->duplicateIPv4)))
					return CW_FALSE;
				break;
			case CW_MSG_ELEMENT_DUPLICATE_IPV6_ADDRESS_CW_TYPE:
				CW_CREATE_OBJECT_ERR_WID(valuesPtr->duplicateIPv6, WTPDuplicateIPv6, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				CW_CREATE_ARRAY_ERR((valuesPtr->duplicateIPv6)->MACoffendingDevice_forIpv6,6, unsigned char, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				if (!(CWParseMsgElemDuplicateIPv6Address(msgPtr, elemLen, valuesPtr->duplicateIPv6)))
					return CW_FALSE;
				break;
			case CW_MSG_ELEMENT_WTP_OPERAT_STATISTICS_CW_TYPE:
				valuesPtr->WTPOperationalStatisticsCount++;
				msgPtr->offset += elemLen;
				break;
			case CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE:
				valuesPtr->WTPRadioStatisticsCount++;
				msgPtr->offset += elemLen;
				break;
			case CW_MSG_ELEMENT_WTP_REBOOT_STATISTICS_CW_TYPE:
				CW_CREATE_OBJECT_ERR_WID(valuesPtr->WTPRebootStatistics, WTPRebootStatisticsInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				if (!(CWParseWTPRebootStatistics(msgPtr, elemLen, valuesPtr->WTPRebootStatistics)))
					return CW_FALSE;	
				break;
			//added by weiay 20080702
			case CW_MSG_ELEMENT_DELETE_STATION_CW_TYPE:
				//malloc memory
				CW_CREATE_OBJECT_ERR_WID(valuesPtr->CWStationInfo, CWStationInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				if (!(CWParseMsgElemCWStationInfoValue(msgPtr, elemLen, valuesPtr->CWStationInfo)))
					{
						wid_syslog_debug_debug(WID_DEFAULT,"*** 000 ***\n");return CW_FALSE;
						}
				wid_syslog_debug_debug(WID_DEFAULT,"*** 001 ***\n");
				break;
			//added end
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				 vendorvalue = CWProtocolRetrieve8(msgPtr);
				 //printf("*** value:%d ***\n",vendorvalue);
				 if(vendorvalue == 1)
				 {
					CW_CREATE_OBJECT_ERR_WID(valuesPtr->neighbor_ap_infos, Neighbor_AP_INFOS, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
					if (!(CWParseMsgElemAPNeighborAPInfos(msgPtr, (elemLen-1), valuesPtr->neighbor_ap_infos)))
						{
							//printf("*** 000000111111111111111111 ***\n");return CW_FALSE;
						}
						//printf("*** 0000002222222222222 ***\n");
					if((valuesPtr->neighbor_ap_infos)&&(valuesPtr->neighbor_ap_infos->neighborapInfosCount == 0))
					{
						CW_FREE_OBJECT_WID(valuesPtr->neighbor_ap_infos);
					}
				 }
				 else if(vendorvalue == 2)
			 	{
			 		if (!(CWParseMsgElemAPInterfaceInfo(msgPtr, (elemLen-1), &valuesPtr->wid_sample_throughput)))
					{
							//printf("*** 000000111111111111111111 ***\n");return CW_FALSE;
					}
					save_sample_throughput_info(wtpindex, valuesPtr->wid_sample_throughput);
			 	}
				 else if(vendorvalue == 3)
			 	{
			 		if (!(CWParseMsgElemAPExtensionInfo(msgPtr, (elemLen-1), &valuesPtr->ap_wifi_info, wtpindex)))
					{
							wid_syslog_err("** error CWParseMsgElemAPExtensionInfo error\n");
							//printf("*** 000000111111111111111111 ***\n");return CW_FALSE;
					}else{
					save_extension_info(wtpindex, valuesPtr->ap_wifi_info);
					}
			 	}
				 else if(vendorvalue == 4)
			 	{
			 		CW_CREATE_OBJECT_ERR_WID(valuesPtr->ap_sta_info, WIDStationInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			 		if (!(CWParseMsgElemAPStaInfoReport(msgPtr, (elemLen-1), valuesPtr->ap_sta_info,wtpindex)))
					{
							//printf("*** 000000111111111111111111 ***\n");return CW_FALSE;
					}
					CW_FREE_OBJECT_WID(valuesPtr->ap_sta_info)
			 	}
				  else if(vendorvalue == 5)
			 	{
			 		//CW_CREATE_OBJECT_ERR(valuesPtr->ap_if_info, wid_ap_if_state_time, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			 		if (!(CWParseMsgElemAPIfInfoReport(msgPtr, (elemLen-1), &valuesPtr->ap_if_info,wtpindex)))
					{
							//printf("*** 000000111111111111111111 ***\n");return CW_FALSE;
					}
			 	}
				else if(vendorvalue == 6)
				{
					CW_CREATE_OBJECT_ERR_WID(valuesPtr->wids_device_infos, wid_wids_device, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
					if (!(CWParseMsgElemAPWidsInfos(msgPtr, (elemLen-1), valuesPtr->wids_device_infos,wtpindex)))
						{
							//printf("*** CWParseMsgElemAPWidsInfos ***\n");return CW_FALSE;
						}
					if(valuesPtr->wids_device_infos->count == 0)
					{
						CW_FREE_OBJECT_WID(valuesPtr->wids_device_infos);
					}				
				}
				
				else if(vendorvalue == 7)
				{
					CW_CREATE_OBJECT_ERR_WID(valuesPtr->wid_sta_wapi_infos, WIDStaWapiInfoList, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
					memset(valuesPtr->wid_sta_wapi_infos, 0, sizeof(WIDStaWapiInfoList));
					if (!(CWParseMsgElemAPStaWapiInfos(msgPtr, (elemLen-1), valuesPtr->wid_sta_wapi_infos,wtpindex)))
						{
							//printf("*** CWParseMsgElemAPWidsInfos ***\n");return CW_FALSE;
						}
					CW_FREE_OBJECT_WID(valuesPtr->wid_sta_wapi_infos);
				}
				else if(vendorvalue == 8){
					char tmp[1];
					CWParseAPStatisInfo_v2(msgPtr, elemLen-1,tmp,wtpindex);
				/*	if(apstatistics == 1)
					{
						AC_WTP[wtpindex]->rx_echocount++;
						if(AC_WTP[wtpindex]->rx_echocount > 2)
						{
							AC_WTP[wtpindex]->rx_echocount = 0;
							AsdWsm_RadioOp(wtpindex,RADIO_INFO);
							
						}
						AsdWsm_bytes_info(wtpindex,BSS_INFO);
					}*/
					if(apstatistics == 1)
					{
						int radio_id =0;
						int local_radioid =0 ;
						struct wlanid *wlanid_list = NULL;
						for( i = 0 ; i < AC_WTP[wtpindex]->RadioCount ; i++)
						{
							if(AC_WTP[wtpindex]->WTP_Radio[i])
							{
								radio_id = AC_WTP[wtpindex]->WTP_Radio[i]->Radio_G_ID;
								local_radioid = AC_WTP[wtpindex]->WTP_Radio[i]->Radio_L_ID;
								if(AC_RADIO[radio_id])
								{
									wlanid_list = AC_RADIO[radio_id]->Wlan_Id;
									while(wlanid_list)
									{
										if((AC_WLAN[wlanid_list->wlanid] != NULL)&&(AC_WLAN[wlanid_list->wlanid]->balance_switch == 1)&&(AC_WLAN[wlanid_list->wlanid]->balance_method == 2))
										{
											AsdWsm_RadioOp(wtpindex,RADIO_INFO);
											AsdWsm_bytes_info(wtpindex,BSS_INFO);
											wid_syslog_debug_debug(WID_DEFAULT,"radio info report to asd !    ***************\n");
											break;
										}
										wlanid_list = wlanid_list->next;
									}
								}
							}
						}
					}
				}
            /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,start*/
				else if(vendorvalue == 9){//ntp resultcode
					char tmp[1];
					CWParseAP_Ntp_resultcode(msgPtr, elemLen-1,tmp,wtpindex);
				}
			 /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,end*/
				else if(vendorvalue == 10){//attack addr redirect
					 CWParseAttack_addr_Redirect(msgPtr, elemLen-1,wtpindex);
				 }
				/* zhangshu add for challenge replay, 2010-09-26 */
				else if(vendorvalue == 11){//challenge replay
					CWParseAP_challenge_replay(msgPtr, elemLen-1,wtpindex);
				}
				/* zhangshu add for Terminal Disturb Info Report, 2010-10-07 */
				else if(vendorvalue == 12)
				{
					char tmp[1];
				    CWParseWtp_Sta_Terminal_Disturb_Report(msgPtr, elemLen-1,tmp,wtpindex);
				}
				//weichao add 2011.11.03
				else if(13 == vendorvalue)
				{
			 		CW_CREATE_OBJECT_ERR_WID(valuesPtr->ap_sta_info, WIDStationInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
					if(!(CWPareseWtp_Sta_Flow_Check_Report(msgPtr, (elemLen-1), valuesPtr->ap_sta_info,wtpindex)))
					{
						
					}	
					CW_FREE_OBJECT_WID(valuesPtr->ap_sta_info)
							
				}
				else if(15 == vendorvalue)
				{
			 		CW_CREATE_OBJECT_ERR_WID(valuesPtr->ap_sta_info, WIDStationInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
					if(!(CWPareseWtp_Sta_leave_Report(msgPtr, (elemLen-1), valuesPtr->ap_sta_info,wtpindex)))
					{
						
					}	
					CW_FREE_OBJECT_WID(valuesPtr->ap_sta_info)			
				}
				else if (vendorvalue == 16)
				{
					wid_syslog_debug_debug(WID_DEFAULT, "vendorvalue==16\n");
				 	CW_CREATE_OBJECT_ERR_WID(valuesPtr->wtp_extend_info, CWWtpExtendinfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, "malloc for CWExtendInfo failed\n"););
					memset(valuesPtr->wtp_extend_info, '\0', sizeof(CWWtpExtendinfo));
					if (!CWParseWTPEtendinfo(msgPtr, (elemLen-1), valuesPtr->wtp_extend_info, wtpindex))
					{
						wid_syslog_notice_local7("parse CWExtendInfo for wtp %d failed\n", wtpindex);	
					} else {
						strncpy((char *)AC_WTP[wtpindex]->longitude, (char *)valuesPtr->wtp_extend_info->longitude, LONGITUDE_LATITUDE_MAX_LEN);
						wid_syslog_debug_debug(WID_DEFAULT, "wtp %d longitude=%s\n",wtpindex, valuesPtr->wtp_extend_info->longitude);
						strncpy((char *)AC_WTP[wtpindex]->latitude, (char *)valuesPtr->wtp_extend_info->latitude, LONGITUDE_LATITUDE_MAX_LEN);
						wid_syslog_debug_debug(WID_DEFAULT, "wtp %d laitude=%s\n",wtpindex, valuesPtr->wtp_extend_info->latitude);
						AC_WTP[wtpindex]->power_mode = valuesPtr->wtp_extend_info->power_mode;
						wid_syslog_debug_debug(WID_DEFAULT, "wtp %d power_mode=%u\n",wtpindex, valuesPtr->wtp_extend_info->power_mode);
						strncpy((char *)AC_WTP[wtpindex]->manufacture_date, (char *)valuesPtr->wtp_extend_info->manufacture_date, MANUFACTURE_DATA_MAX_LEN);
						wid_syslog_debug_debug(WID_DEFAULT, "wtp %d manufacture_date=%s\n",wtpindex, valuesPtr->wtp_extend_info->manufacture_date);
						AC_WTP[wtpindex]->forward_mode = valuesPtr->wtp_extend_info->forward_mode;
						wid_syslog_debug_debug(WID_DEFAULT, "wtp %d forward_mode=%u\n",wtpindex, valuesPtr->wtp_extend_info->forward_mode);
						unsigned char i;
						for (i=0; i<valuesPtr->wtp_extend_info->radio_count; i++) {
							AC_RADIO[wtpindex*L_RADIO_NUM+i]->radio_work_role = valuesPtr->wtp_extend_info->radio_work_role[i];
							wid_syslog_debug_debug(WID_DEFAULT, "wtp %d radio %u radio_work_role=%u\n",
								wtpindex, i, valuesPtr->wtp_extend_info->radio_work_role[i]);
						}
						wid_syslog_debug_debug(WID_DEFAULT, "test %d\n", __LINE__);
					}
					CW_FREE_OBJECT_WID(valuesPtr->wtp_extend_info);
					valuesPtr->wtp_extend_info = NULL;
				}
				else if (vendorvalue == 17) {
					wid_syslog_debug_debug(WID_DEFAULT, "vendorvalue==17\n");
					if (!CWParseWTPTrapInfo(msgPtr, (elemLen-1), wtpindex))
					{
						wid_syslog_notice_local7("parse CWExtendInfo for wtp %d failed\n", wtpindex);	
					} else {
						wid_syslog_debug_debug(WID_DEFAULT, "trap message parse for wtp %d success\n", wtpindex);
					}
				} else if (vendorvalue == 18) {
					wid_syslog_info("parse lte-fi message\n");
					if (!CWParseLTEFITrapInfo(msgPtr, (elemLen-1), wtpindex)) {
						wid_syslog_err("parse lte-fi message failed\n");
					}
					//_CWCloseThread();
				} else if (0 == vendorvalue || 0x80 == vendorvalue  /*|f|*/) {
					vendorvalue <<= 8;
					vendorvalue |= CWProtocolRetrieve8(msgPtr);
					vendorvalue <<= 8;
					vendorvalue |= CWProtocolRetrieve8(msgPtr);
					vendorvalue <<= 8;
					vendorvalue |= CWProtocolRetrieve8(msgPtr);
					wid_syslog_debug_debug(WID_DEFAULT, "------ vendorid %X ------\n", vendorvalue);
					if (vendorvalue == 0x7BA8 || 0x80007BA8 == vendorvalue)		/* vendorid */
					{
						vendorvalue = 0;
						vendorvalue = CWProtocolRetrieve16(msgPtr);
						wid_syslog_debug_debug(WID_DEFAULT, "------ type %d ------\n", vendorvalue);
						switch (vendorvalue)
						{
							case 14: 
								{
									wid_syslog_debug_debug(WID_DEFAULT, "------ vendorvalue 14 ------\n");
									 if (!CWParaseWTPTerminalStatistics(msgPtr, (elemLen - 1), NULL, /*valuesPtr->wtp_terminal_statistics,*/ wtpindex))
									 {
									 }
								}
								break;
							default:
								wid_syslog_crit(" %s %d: vendorvalue: %d\n", __func__, __LINE__, vendorvalue);
								break;
						}
					}
				} else {
					wid_syslog_info("unknown vendorvalue:%d\n", vendorvalue);
				}
			 	break;
			case BINDING_MSG_ELEMENT_TYPE_WTP_SNOOPING:
				break;
			case BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_REPORT:
				//malloc memory
				report_type = CWProtocolRetrieve8(msgPtr);
				if(report_type == 1){
					CW_CREATE_OBJECT_ERR_WID(valuesPtr->ApReportStaInfo, CWStationReportInfo, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
					if (!(CWParseMsgElemCWWtpStaIpMacReportInfo(msgPtr, elemLen-1, valuesPtr->ApReportStaInfo)))
						{
							wid_syslog_debug_debug(WID_DEFAULT,"Get ApReportStaInfo Fail\n");
							printf("Get ApReportStaInfo Fail\n");
							return CW_FALSE;
							}
					wid_syslog_debug_debug(WID_DEFAULT,"Get ApReportStaInfo Success\n");
					printf("Get ApReportStaInfo Success\n");
				}
				break;
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in WTP Event Request");
				break;	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	
	CW_CREATE_ARRAY_ERR(valuesPtr->WTPOperationalStatistics, valuesPtr->WTPOperationalStatisticsCount, WTPOperationalStatisticsValues,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CW_CREATE_ARRAY_ERR(valuesPtr->WTPRadioStatistics, valuesPtr->WTPRadioStatisticsCount, WTPRadioStatisticsValues,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	

	msgPtr->offset = offsetTillMessages;
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_WTP_OPERAT_STATISTICS_CW_TYPE:
				if (!(CWParseWTPOperationalStatistics(msgPtr, elemLen, &(valuesPtr->WTPOperationalStatistics[k++]))))
					return CW_FALSE;				
				break;
			case CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE:
				if (!(CWParseWTPRadioStatistics(msgPtr, elemLen, &(valuesPtr->WTPRadioStatistics[i++]))))
					return CW_FALSE;
				break;
			default:
				msgPtr->offset += elemLen;
				break;
		}
	}
	wid_syslog_debug_debug(WID_DEFAULT,"WTP Event Request Parsed");

	return CW_TRUE;	
}

CWBool CWSaveWTPEventRequestMessage (CWProtocolWTPEventRequestValues *WTPEventRequest, CWWTPProtocolManager *WTPProtocolManager)
{
	if(WTPEventRequest == NULL || WTPProtocolManager == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	if(WTPEventRequest->WTPRebootStatistics)
	{	
		CW_FREE_OBJECT_WID(WTPProtocolManager->WTPRebootStatistics);
		WTPProtocolManager->WTPRebootStatistics = WTPEventRequest->WTPRebootStatistics;
	}

	if((WTPEventRequest->WTPOperationalStatisticsCount) >0) {
		int i,k;
		CWBool found=CW_FALSE;

		for(i=0; i<(WTPEventRequest->WTPOperationalStatisticsCount); i++) {
			found=CW_FALSE;
			for(k=0; k<(WTPProtocolManager->radiosInfo).radioCount; k++)
			{
				if((WTPProtocolManager->radiosInfo).radiosInfo[k].radioID == (WTPEventRequest->WTPOperationalStatistics[i]).radioID) 
				{
					found=CW_TRUE;
					(WTPProtocolManager->radiosInfo).radiosInfo[k].TxQueueLevel = (WTPEventRequest->WTPOperationalStatistics[i]).TxQueueLevel;
					(WTPProtocolManager->radiosInfo).radiosInfo[k].wirelessLinkFramesPerSec = (WTPEventRequest->WTPOperationalStatistics[i]).wirelessLinkFramesPerSec;
				}
			}
			/*if(!found)
			{
				for(k=0; k<(WTPProtocolManager->radiosInfo).radioCount; k++)
				{
					if((WTPProtocolManager->radiosInfo).radiosInfo[k].radioID == UNUSED_RADIO_ID); 
					{
						(WTPProtocolManager->radiosInfo).radiosInfo[k].radioID = (WTPEventRequest->WTPOperationalStatistics[i]).radioID;
						(WTPProtocolManager->radiosInfo).radiosInfo[k].TxQueueLevel = (WTPEventRequest->WTPOperationalStatistics[i]).TxQueueLevel;
						(WTPProtocolManager->radiosInfo).radiosInfo[k].wirelessLinkFramesPerSec = (WTPEventRequest->WTPOperationalStatistics[i]).wirelessLinkFramesPerSec;
					}
				}	
			}*/
		}
	}

	if((WTPEventRequest->WTPRadioStatisticsCount) >0) {
		int i,k;
		CWBool found;

		for(i=0; i<(WTPEventRequest->WTPRadioStatisticsCount); i++) {
			found=CW_FALSE;
			for(k=0; k<(WTPProtocolManager->radiosInfo).radioCount; k++) 
			{
				if((WTPProtocolManager->radiosInfo).radiosInfo[k].radioID == (WTPEventRequest->WTPOperationalStatistics[i]).radioID) 
				{
					found=CW_TRUE;
					(WTPProtocolManager->radiosInfo).radiosInfo[k].statistics = (WTPEventRequest->WTPRadioStatistics[i]).WTPRadioStatistics;
				}
			}
			/*if(!found)
			{
				for(k=0; k<(WTPProtocolManager->radiosInfo).radioCount; k++) 
				{
					if((WTPProtocolManager->radiosInfo).radiosInfo[k].radioID == UNUSED_RADIO_ID);
					{
						(WTPProtocolManager->radiosInfo).radiosInfo[k].radioID = (WTPEventRequest->WTPOperationalStatistics[i]).radioID;
						(WTPProtocolManager->radiosInfo).radiosInfo[k].statistics = (WTPEventRequest->WTPRadioStatistics[i]).WTPRadioStatistics;
					}
				}	
			}*/
		}
	}

	//CW_FREE_OBJECT((WTPEventRequest->WTPOperationalStatistics), (WTPEventRequest->WTPOperationalStatisticsCount));
	//CW_FREE_OBJECTS_ARRAY((WTPEventRequest->WTPRadioStatistics), (WTPEventRequest->WTPRadioStatisticsCount));
	//Da controllare!!!!!!!
	CW_FREE_OBJECT_WID(WTPEventRequest->WTPOperationalStatistics);
	CW_FREE_OBJECT_WID(WTPEventRequest->WTPRadioStatistics);
	//CW_FREE_OBJECT(WTPEventRequest);

	return CW_TRUE;
}


CWBool CWAssembleWTPEventResponse (CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum) 
{
	CWProtocolMessage *msgElems= NULL;
	const int msgElemCount=0;
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling WTP Event Response...");
		
	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		return CW_FALSE;
	wid_syslog_debug_debug(WID_WTPINFO,"WTP Event Response Assembled");
	
	return CW_TRUE;
}

CWBool CWParseChangeStateEventRequestMessage2 (CWProtocolMessage *msgPtr, int len, CWProtocolChangeStateEventRequestValues **valuesPtr) 
{
	int offsetTillMessages;
	int i=0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_CREATE_OBJECT_ERR_WID(*valuesPtr, CWProtocolChangeStateEventRequestValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_WTPINFO,"#________ WTP Change State Event (Run) ________#");
	
	(*valuesPtr)->radioOperationalInfo.radiosCount = 0;
	(*valuesPtr)->radioOperationalInfo.radios = NULL;
	
	// parse message elements
	while((msgPtr->offset-offsetTillMessages) < len) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(msgPtr,&elemType,&elemLen);		

		wid_syslog_debug_debug(WID_WTPINFO,"Parsing Message Element: %u, elemLen: %u", elemType, elemLen);

		switch(elemType) {
			case CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE:
				((*valuesPtr)->radioOperationalInfo.radiosCount)++; // just count how many radios we have, so we can allocate the array
				msgPtr->offset += elemLen;
				break;
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE: 
				if(!(CWParseResultCode(msgPtr, elemLen, &((*valuesPtr)->resultCode)))){
					CW_FREE_OBJECT_WID(*valuesPtr);
					*valuesPtr = NULL;
					return CW_FALSE;
				} 
				break;
			default:
				CW_FREE_OBJECT_WID(*valuesPtr);
				*valuesPtr = NULL;
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Change State Event Request");
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) {
		CW_FREE_OBJECT_WID(*valuesPtr);
		*valuesPtr = NULL;
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	}
	CW_CREATE_ARRAY_ERR((*valuesPtr)->radioOperationalInfo.radios, (*valuesPtr)->radioOperationalInfo.radiosCount, CWRadioOperationalInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
	msgPtr->offset = offsetTillMessages;
	i = 0;
	while(msgPtr->offset-offsetTillMessages < len) {
		unsigned short int type=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int len=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(msgPtr,&type,&len);		

		switch(type) {
			case CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE:
				if(!(CWParseWTPRadioOperationalState(msgPtr, len, &((*valuesPtr)->radioOperationalInfo.radios[i])))) return CW_FALSE; // will be handled by the caller
				i++;
				break;
			default:
				msgPtr->offset += len;
				break;
		}
	}
	wid_syslog_debug_debug(WID_WTPINFO,"Change State Event Request Parsed");
	
	return CW_TRUE;
}

CWBool CWSaveChangeStateEventRequestMessage(CWProtocolChangeStateEventRequestValues *valuesPtr, CWWTPProtocolManager *WTPProtocolManager)
{
	CWBool found;
	CWBool retValue = CW_TRUE;

	if(valuesPtr == NULL || WTPProtocolManager == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	if((valuesPtr->radioOperationalInfo.radiosCount) >0) {
		int i,k;

		for(i=0; i<(valuesPtr->radioOperationalInfo.radiosCount); i++) {
			found=CW_FALSE;
			for(k=0; k<(WTPProtocolManager->radiosInfo).radioCount; k++)
			{
				if((WTPProtocolManager->radiosInfo).radiosInfo[k].radioID == (valuesPtr->radioOperationalInfo.radios[i]).ID) 
				{
					found=CW_TRUE;
					(WTPProtocolManager->radiosInfo).radiosInfo[k].operationalState = (valuesPtr->radioOperationalInfo.radios[i]).state;
					(WTPProtocolManager->radiosInfo).radiosInfo[k].operationalCause = (valuesPtr->radioOperationalInfo.radios[i]).cause;
				}
			
				if(!found) 
					retValue= CW_FALSE;
			}
		}
	}
	
	CW_FREE_OBJECT_WID(valuesPtr->radioOperationalInfo.radios)
	CW_FREE_OBJECT_WID(valuesPtr);	

	return retValue;
}


CWBool CWParseEchoRequestMessage (CWProtocolMessage *msgPtr, int len, int WTPIndex) 
{
	
	int offsetTillMessages;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_WTPINFO,"#________ Echo Request (Run) ________#");
	
	// parse message elements
	while((msgPtr->offset-offsetTillMessages) < len) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(msgPtr,&elemType,&elemLen);		

		//wid_syslog_debug_debug("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
		
		/*fengwenchao modify begin for GM-3,20111121*/
		unsigned char vendorvalue = 0;  //fengwenchao add 20111121 for GM-3
		switch(elemType) {
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				vendorvalue = CWProtocolRetrieve8(msgPtr);
				if(vendorvalue == 111)
				{	
					if((AC_WTP[WTPIndex] != NULL)&&(AC_WTP[WTPIndex]->heart_time.heart_statistics_switch == 1))
						CWParselinkquality(msgPtr, (elemLen-1),WTPIndex);
					else
						msgPtr->offset = msgPtr->offset +15;
				}
				else
				{	msgPtr->offset = msgPtr->offset -1;
					CWParseAPStatisInfo(msgPtr, elemLen,WTPIndex);
				}
				break;

			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Echo Request must carry no message elements");
		}
		/*fengwenchao modify end*/
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	wid_syslog_debug_debug(WID_WTPINFO,"Echo Request Parsed");
	
	return CW_TRUE;
}

CWBool CWAssembleEchoResponse (CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum) 
{
	CWProtocolMessage *msgElems= NULL;
	const int msgElemCount=0;
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling Echo Response...");
		
	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_ECHO_RESPONSE, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		return CW_FALSE;
	wid_syslog_debug_debug(WID_WTPINFO,"Echo Response Assembled");
	
	return CW_TRUE;
}

CWBool CWAssembleConfigurationUpdateRequest_Radio(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem) 
{	
	CWProtocolMessage *msgElemsBinding = NULL;
	int msgElemBindingCount=0;
	CWProtocolMessage *msgElems = NULL;
	int MsgElemCount = 0;
	int k = -1;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling Configuration Update Request...");
	if(!check_wtpid_func(WTPIndex)){
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	if(!CWBindingAssembleConfigurationUpdateRequest2(&msgElemsBinding, &msgElemBindingCount, WTPIndex, elem)){
		return CW_FALSE;
	}

	
	if((elem->mqinfo.u.RadioInfo.Radio_Op == Radio_Throughput)||(elem->mqinfo.u.RadioInfo.Radio_Op == Radio_BSS_Throughput))
	{
		MsgElemCount = 1;
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, CW_FREE_OBJECT_WID(msgElemsBinding);return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

		if (!(CWAssembleMsgElemAPThroughoutSet(&(msgElems[++k]),WTPIndex))) 
		{
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT_WID(msgElems);
			CW_FREE_OBJECT_WID(msgElemsBinding);//hjw add 
			return CW_FALSE; // error will be handled by the caller
		}
	}		
	
	if(elem->mqinfo.u.RadioInfo.Radio_Op == Radio_Qos)
	{
		wid_syslog_debug_debug(WID_WTPINFO,"## wtp id = %d enable qos##\n",WTPIndex);
		MsgElemCount = 1;
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount,CW_FREE_OBJECT_WID(msgElemsBinding); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		int tagpacket = 0;
		int g_radio_id = elem->mqinfo.u.RadioInfo.Radio_G_ID;
		unsigned int qosid = elem->mqinfo.u.RadioInfo.id1;
		if(!check_g_radioid_func(g_radio_id)){
			wid_syslog_err("%s\n",__func__);
			CW_FREE_OBJECT_WID(msgElems);
			CW_FREE_OBJECT_WID(msgElemsBinding);//hjw add 
			return CW_FALSE;
		}else{
		}
		if (!(CWAssembleWTPQoS2(&(msgElems[++k]),g_radio_id,tagpacket,qosid))) 
		{
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT_WID(msgElems);
			CW_FREE_OBJECT_WID(msgElemsBinding);//hjw add 
			return CW_FALSE; // error will be handled by the caller
		}
	}

	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		return CW_FALSE;
	wid_syslog_debug_debug(WID_WTPINFO,"Configuration Update Request Assembled");
	
	return CW_TRUE;

}


CWBool CWAssembleConfigurationUpdateRequest_WTP(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem) 
{	
	CWProtocolMessage *msgElemsBinding = NULL;
	int msgElemBindingCount=0;
	CWProtocolMessage *msgElems = NULL;
	int MsgElemCount = 0;
	int k = -1;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling Configuration Update Request...");
	switch(elem->mqinfo.u.WtpInfo.Wtp_Op){

		case WTP_SCANNING_OP:
			//printf("##003 wtp id = %d ##\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

			if (!(CWAssembleMsgElemAPScanningSet(&(msgElems[++k])))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
	
		case WTP_STATISTICS_REPORT:
			//printf("##statistics wtp id = %d ##\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

			if (!(CWAssembleMsgElemAPStatisticsSet(&(msgElems[++k]),elem->mqinfo.u.WtpInfo.value2))) //fengwenchao modify 20110422
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_STATISTICS_REPORT_INTERVAL:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleMsgElemAPStatistics_Interval_Set(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
     /*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3*/
	  case WTP_NTP_SET:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleMsgElemAP_NTP_Set(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_EXTEND_CMD:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

			if (!(CWAssembleMsgElemAPExtensinCommandSet(&(msgElems[++k]),WTPIndex,(char*)&(elem->mqinfo.u.WtpInfo.value)))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
	
	
		case WTP_SET_IP:
			//printf("## wtp id = %d config ap ip \n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			/*if (!(CWAssembleStaticAPIP(&(msgElems[++k]),WTPIndex)))//12.17chenjun*/ 
			if (!(CWAssembleStaticAPIPDNS(&(msgElems[++k]),WTPIndex)))
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
	
		case WTP_TIMESTAMP:
			//printf("## wtp id = %d synchronization ac timestamp \n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleTimestamp(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_EXTEND_INFO_GET:
			//printf("## wtp id = %d extension infomation\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssemblewtpextensioninfomation(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
		case WTP_SAMPLE_INFO_SET:
			//printf("## wtp id = %d ##\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

			if (!(CWAssembleMsgElemAPInterfaceInfo(&(msgElems[++k])))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_STA_INFO_SET:
			//printf("## wtp id = %d sta infomation\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssemblewtpstainfomationreport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_IF_INFO_SET:
			//printf("## wtp id = %d if infomation\n",WTPIndex);
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssemblewtpifinforeport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_WIDS_SET:
			
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleWidsSet(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
		case WTP_NEIGHBORDEAD_INTERVAL:
			
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleneighbordead_interval(&(msgElems[++k])))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
		case WTP_SET_IP_DNS:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleStaticAPIPDNS(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;	
		case WTP_STA_WAPI_INFO_SET:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleWtpStaWapiInfoReport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE;
			}
			break;
		case WTP_DHCP_SNOOPING:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleSnoopingAble(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_STA_INFO_REPORT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleIpMacReport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		//weichao add 2011.11.03	
		case WTP_FLOW_CHECK:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
			if (!(CWAssembleMsgElemAPFlowCheck(&(msgElems[++k]),elem->mqinfo.u.WlanInfo.Radio_L_ID,elem->mqinfo.u.WlanInfo.WLANID,elem->mqinfo.u.WlanInfo.flow_check,elem->mqinfo.u.WlanInfo.no_flow_time,elem->mqinfo.u.WlanInfo.limit_flow))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		/* zhangshu add for Terminal Disturb Info Report, 2010-10-07 */
		case WTP_TERMINAL_DISTRUB_INFO:
		    MsgElemCount = 1;
		    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		    if (!(CWAssembleTerminalDisturbInfoReport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
		    break;
		/*fengwenchao add 20110126 for XJDEX-32  from 2.0 */
		case WTP_IF_ETH_MTU:
			MsgElemCount = 1;
			char eth_index = elem->mqinfo.u.WtpInfo.value1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleWtpEthMtu(&(msgElems[++k]),WTPIndex,eth_index))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE;
			}
			break;
		case WTP_SET_NAME_PASSWD:
			MsgElemCount =1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleMsgElemAPPasswd(&(msgElems[++k]),elem->mqinfo.u.WtpInfo.username,elem->mqinfo.u.WtpInfo.passwd))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_MULTI_USER_OPTIMIZE:
			MsgElemCount =1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleMsgElemAPMultiUserOptimize(&(msgElems[++k]),elem->mqinfo.u.WlanInfo.WLANID,elem->mqinfo.u.WlanInfo.Radio_L_ID,elem->mqinfo.u.WlanInfo.multi_user_optimize_switch))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_STA_DEAUTH_SWITCH:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleWtpStaDeauthreport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_STA_FLOW_INFORMATION_SWITCH:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			if (!(CWAssembleWtpStaFlowInformationreport(&(msgElems[++k]),WTPIndex))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;

		case WTP_OPTION60_PARAM:
			MsgElemCount =1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,NULL););
			if (!(CWAssembleMsgElemAPOption60ParameterSet(&(msgElems[++k]),WTPIndex,(char*)&(elem->mqinfo.u.WtpInfo.value)))) 
			{
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; // error will be handled by the caller
			}
			break;
		case WTP_LONGITUDE_LATITUDE_SET:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););
			if (!(CWAssembleMsgElemAPLongitudeLatitude(&(msgElems[++k]),(unsigned char *)elem->mqinfo.u.WtpInfo.username, (unsigned char *)elem->mqinfo.u.WtpInfo.passwd))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			memset((char *)AC_WTP[WTPIndex]->longitude, '\0', LONGITUDE_LATITUDE_MAX_LEN);
			strncpy((char *)AC_WTP[WTPIndex]->longitude, elem->mqinfo.u.WtpInfo.username, strlen(elem->mqinfo.u.WtpInfo.username));
			memset((char *)AC_WTP[WTPIndex]->latitude, '\0', LONGITUDE_LATITUDE_MAX_LEN);
			strncpy((char *)AC_WTP[WTPIndex]->latitude, elem->mqinfo.u.WtpInfo.passwd, strlen(elem->mqinfo.u.WtpInfo.passwd));
			break;
		case WTP_UNAUTHORIZED_MAC_REPORT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););			
			if (!(capwap_comm_switch_interval_assemble(&(msgElems[++k]),WTPIndex, WTP_UNAUTHORIZED_MAC_REPORT))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_CONFIGURE_ERROR_REPROT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););			
			if (!(capwap_comm_switch_interval_assemble(&(msgElems[++k]),WTPIndex, WTP_CONFIGURE_ERROR_REPROT))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		#if 0
		case WTP_STA_FLOW_OVERFLOW_RX_REPORT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););			
			if (!(capwap_comm_switch_interval_assemble(&(msgElems[++k]),WTPIndex, WTP_STA_FLOW_OVERFLOW_RX_REPORT))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_STA_FLOW_OVERFLOW_TX_REPROT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););			
			if (!(capwap_comm_switch_interval_assemble(&(msgElems[++k]),WTPIndex, WTP_STA_FLOW_OVERFLOW_TX_REPROT))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		case WTP_STA_ONLINE_FULL_REPROT:
			MsgElemCount = 1;
			CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,MsgElemCount,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,"set wtp longitude and latitude failed"););			
			if (!(capwap_comm_switch_interval_assemble(&(msgElems[++k]),WTPIndex, WTP_STA_ONLINE_FULL_REPROT))) 
			{													
				int i;
				for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
				CW_FREE_OBJECT_WID(msgElems);
				return CW_FALSE; 
			}
			break;
		#endif
		default:
			return CW_FALSE;
	}
	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		return CW_FALSE;
	wid_syslog_debug_debug(WID_WTPINFO,"Configuration Update Request Assembled");
	
	return CW_TRUE;

}

CWBool CWAssembleConfigurationUpdateRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum) 
{
	CWProtocolMessage *msgElemsBinding = NULL;
	int msgElemBindingCount=0;
	CWProtocolMessage *msgElems = NULL;
	int msgElemCount=0;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling Configuration Update Request...");
/*	if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		return CW_FALSE;
	}

	radiosInfo=gWTPs[*iPtr].WTPProtocolManager.radiosInfo;
	radioCount=radiosInfo.radioCount;
	MsgElemCount=radioCount;

	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for (j=0; j<radioCount; j++)
	{
		radioID=radiosInfo.radiosInfo[j].radioID;
		
		// Assemble Message Elements
		if (!(CWAssembleWTPQoS(&(msgElems[++k]), radioID, tagPackets)))
		{
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
	}
*/
	if(!CWBindingAssembleConfigurationUpdateRequest(&msgElemsBinding, &msgElemBindingCount)){
		return CW_FALSE;
	}

	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		return CW_FALSE;
	wid_syslog_debug_debug(WID_WTPINFO,"Configuration Update Request Assembled");
	
	return CW_TRUE;
}

CWBool CWStartNeighborDeadTimer(int WTPIndex){
	if(!CWErr(CWTimerRequest(gCWNeighborDeadInterval/3, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) { // start NeighborDeadInterval timer
		return CW_FALSE;
	}
	return CW_TRUE;
}

CWBool CWStartNeighborDeadTimerForEcho(int WTPIndex){
	int echoInterval;

	CWACGetEchoRequestTimer(&echoInterval);
	if(!CWErr(CWTimerRequest(echoInterval, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) { // start NeighborDeadInterval timer
		return CW_FALSE;
	}
	return CW_TRUE;
}

CWBool CWStopNeighborDeadTimer(int WTPIndex){
	AC_WTP[WTPIndex]->neighbordeatimes = 0;
	if(!CWTimerCancel(&(gWTPs[WTPIndex].currentTimer),1)) {
		return CW_FALSE;
	}
	return CW_TRUE;
}

CWBool CWRestartNeighborDeadTimer(int WTPIndex) {
	
	CWThreadSetSignals(SIG_BLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);	
	if(!CWStopNeighborDeadTimer(WTPIndex)) return CW_FALSE;
	if(!CWStartNeighborDeadTimer(WTPIndex)) return CW_FALSE;
	CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
	wid_syslog_debug_debug(WID_WTPINFO,"NeighborDeadTimer restarted");
	return CW_TRUE;
}

CWBool CWRestartNeighborDeadTimerForEcho(int WTPIndex) {
	
	CWThreadSetSignals(SIG_BLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);	
	if(!CWStopNeighborDeadTimer(WTPIndex)) return CW_FALSE;
	if(!CWStartNeighborDeadTimerForEcho(WTPIndex)) return CW_FALSE;
	CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
	wid_syslog_debug_debug(WID_WTPINFO,"NeighborDeadTimer restarted for Echo interval");
	return CW_TRUE;
}

CWBool CWAssembleMsgElemIEEE80211Info(CWProtocolMessage *msgPtr, int WTPIndex, unsigned char RadioID, int wlanid, struct msgqlist *elem){
	unsigned int key_mgmt = 0;
	unsigned int cipher = 0;
	unsigned int BSSIndex = 0;
	unsigned int pre_auth = 0;
	int size = 39;
	int id = wlanid;
	if(elem->mqinfo.subtype == STA_S_TYPE){
		cipher = elem->mqinfo.u.StaInfo.cipher;
		BSSIndex = elem->mqinfo.u.StaInfo.BSSIndex;
		RadioID = (BSSIndex/L_BSS_NUM)%L_RADIO_NUM;
	}
		
	wid_syslog_debug_debug(WID_WTPINFO,"CWAssembleMsgElemIEEE80211Info\n");
	wid_syslog_debug_debug(WID_WTPINFO,"wlanid %d\n",wlanid);
	wid_syslog_debug_debug(WID_WTPINFO,"cipher %d\n",cipher);
	if((wlanid > 0)&&(AC_WLAN[wlanid] != NULL)){
		switch(AC_WLAN[wlanid]->SecurityType){
			case OPEN :{
				if(AC_WLAN[wlanid]->EncryptionType == WEP){
					if((AC_WLAN[wlanid]->KeyLen == 5)||(AC_WLAN[wlanid]->KeyLen == 10)){
						key_mgmt = WID_WPA_KEY_MGMT_NONE;
						cipher = WID_WPA_CIPHER_WEP40;
					}else if((AC_WLAN[wlanid]->KeyLen == 13)||(AC_WLAN[wlanid]->KeyLen == 26)){
						key_mgmt = WID_WPA_KEY_MGMT_NONE;
						cipher = WID_WPA_CIPHER_WEP104;
					}else if((AC_WLAN[wlanid]->KeyLen == 16)||(AC_WLAN[wlanid]->KeyLen == 32)){
						key_mgmt = WID_WPA_KEY_MGMT_NONE;
						cipher = WID_WPA_CIPHER_WEP128;
					}else
						return CW_FALSE;
				}else{
					key_mgmt = WID_WPA_KEY_MGMT_NONE;
					cipher = WID_WPA_CIPHER_NONE;
				}
				break;
			}
			case SHARED:				
				if((AC_WLAN[wlanid]->KeyLen == 5)||(AC_WLAN[wlanid]->KeyLen == 10)){
					key_mgmt = WID_WPA_KEY_MGMT_SHARED;
					cipher = WID_WPA_CIPHER_WEP40;
				}else if((AC_WLAN[wlanid]->KeyLen == 13)||(AC_WLAN[wlanid]->KeyLen == 26)){
					key_mgmt = WID_WPA_KEY_MGMT_SHARED;
					cipher = WID_WPA_CIPHER_WEP104;
				}else if((AC_WLAN[wlanid]->KeyLen == 16)||(AC_WLAN[wlanid]->KeyLen == 32)){
					key_mgmt = WID_WPA_KEY_MGMT_SHARED;
					cipher = WID_WPA_CIPHER_WEP128;
				}else
					return CW_FALSE;
				break;
			case IEEE8021X :
				key_mgmt = WID_WPA_KEY_MGMT_IEEE8021X_NO_WPA;
				cipher = WID_WPA_CIPHER_WEP40;
				break;
			case WPA_P :
				key_mgmt = WID_WPA_KEY_MGMT_PSK;
				if(AC_WLAN[wlanid]->EncryptionType == TKIP)
					cipher = WID_WPA_CIPHER_TKIP;
				else if(AC_WLAN[wlanid]->EncryptionType == AES)
					cipher = WID_WPA_CIPHER_CCMP;
				break;
			case WPA2_P :				
				key_mgmt = WID_WPA2_KEY_MGMT_PSK;
				if(AC_WLAN[wlanid]->EncryptionType == TKIP)
					cipher = WID_WPA_CIPHER_TKIP;
				else if(AC_WLAN[wlanid]->EncryptionType == AES)
					cipher = WID_WPA_CIPHER_CCMP;
				break;
			case WPA_E :			
				key_mgmt = WID_WPA_KEY_MGMT_IEEE8021X;
				if(AC_WLAN[wlanid]->EncryptionType == TKIP)
					cipher = WID_WPA_CIPHER_TKIP;
				else if(AC_WLAN[wlanid]->EncryptionType == AES)
					cipher = WID_WPA_CIPHER_CCMP;
				break;
			case WPA2_E :
				pre_auth = AC_WLAN[wlanid]->PreAuth;
				key_mgmt = WID_WPA2_KEY_MGMT_IEEE8021X;
				if(AC_WLAN[wlanid]->EncryptionType == TKIP)
					cipher = WID_WPA_CIPHER_TKIP;
				else if(AC_WLAN[wlanid]->EncryptionType == AES)
					cipher = WID_WPA_CIPHER_CCMP;
				break;
			//add for WAPI
			case WAPI_PSK:
				key_mgmt = WID_WAPI_KEY_MGMT_PSK;
				cipher = WID_WAPI_CIPHER_SMS4;
				break;
			case WAPI_AUTH:
				key_mgmt = WID_WAPI_KEY_MGMT_CER;
				cipher = WID_WAPI_CIPHER_SMS4;
				break;
			default : break;
		}
	}
	else if(AC_BSS[BSSIndex] != NULL)
	{
		id = AC_BSS[BSSIndex]->WlanID;
	}
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	CWProtocolStore8(msgPtr, RadioID);
	CWProtocolStore8(msgPtr, id);	
	CWProtocolStore8(msgPtr, 0);
	
	CWProtocolStore8(msgPtr, 221);
	CWProtocolStore8(msgPtr, 34);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore8(msgPtr, 0);	
	CWProtocolStore32(msgPtr, 0);	
	CWProtocolStore32(msgPtr, cipher);
	CWProtocolStore32(msgPtr, cipher);
	CWProtocolStore32(msgPtr, key_mgmt);
	CWProtocolStore32(msgPtr, pre_auth);
	CWProtocolStore32(msgPtr, 0);
	CWProtocolStore32(msgPtr, 0);	
//	CWCaptrue(msgPtr->offset,msgPtr->msg);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_INFO_ELEMENT);
	

}

CWBool CWAssembleMsgElemAddWlan(CWProtocolMessage *msgPtr, int WTPIndex, unsigned char RadioID, int id, int op, struct msgqlist *elem)
{
	int i = 0;
/*or(i=0; i < WLAN_NUM; i++){
		
		if((AC_WTP[WTPIndex]->CMD->wlanid[i] == 1)||(AC_WTP[WTPIndex]->CMD->wlanid[i] == 3))
			break;
		else if(i < WLAN_NUM)
			continue;
		else{
			AC_WTP[WTPIndex]->CMD->wlanCMD = 0;
			return CW_FALSE;
		}
	}*/
	i = id;
	if(!check_wlanid_func(i)){
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	/*#################################*/
	MQ_WLAN *wlaninfo = (MQ_WLAN *)WID_MALLOC(sizeof(MQ_WLAN));
	if (!wlaninfo) {
		wid_syslog_err("%s malloc for wlaninfo failed \n", __FUNCTION__);
		return CW_FALSE;
	}
	memset(wlaninfo,0,sizeof(MQ_WLAN));
	if(&(elem->mqinfo.u.WlanInfo) != NULL){
		memcpy(wlaninfo,&(elem->mqinfo.u.WlanInfo),sizeof(MQ_WLAN));
		}
	else
		{
		wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
		}
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo->bssindex=%d.\n",elem->mqinfo.u.WlanInfo.bssindex);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->bssindex=%d.\n",wlaninfo->bssindex);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->l_radio:%d\n",elem->mqinfo.u.WlanInfo.Radio_L_ID);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->l_radio:%d\n",wlaninfo->Radio_L_ID);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.HideESSid:%d\n",elem->mqinfo.u.WlanInfo.HideESSid);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->HideESSid:%d\n",wlaninfo->HideESSid);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.SecurityType:%d\n",elem->mqinfo.u.WlanInfo.SecurityType);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->SecurityType:%d\n",wlaninfo->SecurityType);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.SecurityIndex:%d\n",elem->mqinfo.u.WlanInfo.SecurityIndex);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->SecurityIndex:%d\n",wlaninfo->SecurityIndex);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.asic_hex:%d\n",elem->mqinfo.u.WlanInfo.asic_hex);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->asic_hex:%d\n",wlaninfo->asic_hex);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.KeyLen:%d\n",elem->mqinfo.u.WlanInfo.KeyLen);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->KeyLen:%d\n",wlaninfo->KeyLen);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.Roaming_Policy:%d\n",elem->mqinfo.u.WlanInfo.Roaming_Policy);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->Roaming_Policy:%d\n",wlaninfo->Roaming_Policy);
	wid_syslog_debug_debug(WID_WTPINFO,"elem->mqinfo.u.WlanInfo.essid:%s\n",elem->mqinfo.u.WlanInfo.WlanEssid);
	wid_syslog_debug_debug(WID_WTPINFO,"wlaninfo->essid:%s\n",wlaninfo->WlanEssid);
	/*#################################*/
	if(AC_WLAN[i] == NULL){
		AC_WTP[WTPIndex]->CMD->wlanCMD -= 1;
		if(AC_WTP[WTPIndex]->CMD->wlanCMD < 0)
			AC_WTP[WTPIndex]->CMD->wlanCMD = 0;
		if(wlaninfo){
			WID_FREE(wlaninfo);
			wlaninfo = NULL;
		}
		return CW_FALSE;
	}
	//unsigned int BSSIndex = AC_WLAN[i]->S_WTP_BSS_List[WTPIndex][RadioID];
	unsigned int BSSIndex = wlaninfo->bssindex;
	if(!check_bssid_func(BSSIndex)){
		wid_syslog_err("<error> %s,BSSIndex=%d\n",__func__,BSSIndex);
		if(wlaninfo){
			WID_FREE(wlaninfo);
			wlaninfo = NULL;
		}
		return CW_FALSE;
	}else{}
	if(op == WLAN_ADD){
		//printf("add wlan\n");
		int size = 19;
		wid_syslog_debug_debug(WID_WTPINFO,"wtp%d create wlan :%d\n",WTPIndex,i);
	
	
		AC_WTP[WTPIndex]->CMD->wlanCMD -= 1;
		AC_WTP[WTPIndex]->CMD->radiowlanid[RadioID][i] = 2;
		int k1;
		unsigned char policy = 0;
		unsigned char tunnel = 0;
		unsigned int wlan_tunnel_switch = AC_WLAN[i]->wlan_tunnel_switch;/* yjl 2014-2-28 */
		//printf("MsgElemAddWlan:001\n");
		for(k1=0;k1<L_BSS_NUM;k1++){
		/*	if(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1] == NULL){
				printf("BSSIndex:%d\n",k1);
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1] = (WID_BSS*)malloc(sizeof(WID_BSS));
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSSID = (unsigned char*)malloc(6);
				memset(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSSID,0,6);
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->WlanID = i;
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->Radio_G_ID = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->Radio_G_ID;
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->Radio_L_ID = RadioID;
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->State = 1;
				AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex = (AC_WTP[WTPIndex]->WTP_Radio[RadioID]->Radio_G_ID)*4+k1;
				AC_BSS[AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex] = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1];
				AC_WLAN[i]->S_WTP_BSS_List[WTPIndex] = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSSIndex;
				break;
			}*/
			if((AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1] != NULL)&&(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->WlanID == i)){
				//printf("MsgElemAddWlan:002\n");
				policy = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSS_IF_POLICY;

				tunnel = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSS_TUNNEL_POLICY;
				if (AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->enable_wlan_flag == 0)		/* Huangleilei add for ASXXZFI-1622 */
				{
					AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->enable_wlan_flag = 1;
				}
				//printf("MsgElemAddWlan tunnel %d\n",tunnel);
				
				break;
			}

		}
//	unsigned char wlanid = AC_WTP[WTPIndex]->WTP_Radio->BSS->WlanID;
//	AC_WTP[WTPIndex]->WTP_Radio->BSS->WlanID = i;	
//	unsigned char radioid = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->Radio_L_ID;
		//printf("MsgElemAddWlan:003\n");

		char *essid;
		//essid = AC_WLAN[i]->ESSID;//wuwl mov
		//size += AC_WLAN[i]->KeyLen;//wuwl mov
		essid = wlaninfo->WlanEssid;//wuwl add
		size += wlaninfo->KeyLen;//wuwl add
 		//size += strlen(essid)+1;
 		size += 32;
		//unsigned int type = AC_WLAN[i]->SecurityType;
		CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, WID_FREE(wlaninfo); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
		CWProtocolStore8(msgPtr, RadioID);
		CWProtocolStore8(msgPtr, i);
		if(wlaninfo->Roaming_Policy == 1)//if(AC_WLAN[i]->Roaming_Policy == 1)        wuwl change for muti thread op
			CWProtocolStore16(msgPtr, 1);
		else
			CWProtocolStore16(msgPtr, 0);
		//assemble msg
		unsigned int l_bss_id = BSSIndex%L_BSS_NUM+1;
		//CWProtocolStore8(msgPtr, 0);
		//CWProtocolStore8(msgPtr, l_bss_id);
		if(AC_BSS[BSSIndex] == NULL)
		{
			wid_syslog_debug_debug(WID_DEFAULT,"bssindex %d\n",BSSIndex);
			CWProtocolStore8(msgPtr,0);
		}
		else
		{	
			if(AC_BSS[BSSIndex] != NULL){
				//AC_BSS[BSSIndex]->keyindex = AC_WLAN[i]->SecurityIndex;/*nl add 20100307*/
				AC_BSS[BSSIndex]->keyindex = wlaninfo->SecurityIndex;//wuwl add
			}
			CWProtocolStore8(msgPtr, wlaninfo->SecurityIndex);//sz change l_bss_id to keyindex 05-20
		}
		//printf("wid send l_bss_id %d\n",l_bss_id);
		wid_syslog_debug_debug(WID_WTPINFO,"wid send l_bss_id %d\n",l_bss_id);
		if(wlaninfo->SecurityType == WID_WAPI_KEY_MGMT_PSK)//(AC_WLAN[i]->SecurityType == WID_WAPI_KEY_MGMT_PSK)
		{
			if(wlaninfo->asic_hex == 1)//(AC_WLAN[i]->asic_hex == 1)
			{
				CWProtocolStore8(msgPtr, 1);
			}
			else
			{
				CWProtocolStore8(msgPtr, 0);
			}
			CWProtocolStore16(msgPtr, wlaninfo->KeyLen/*AC_WLAN[i]->KeyLen*/);
			if(wlaninfo->KeyLen != 0)//(AC_WLAN[i]->KeyLen != 0)
			{
				CWProtocolStoreRawBytes(msgPtr,wlaninfo->WlanKey,wlaninfo->KeyLen/*AC_WLAN[i]->WlanKey, AC_WLAN[i]->KeyLen*/);
			}
		}
		else
		{
			CWProtocolStore8(msgPtr, 0);
			CWProtocolStore16(msgPtr, wlaninfo->KeyLen/*AC_WLAN[i]->KeyLen*/);
			if(wlaninfo->KeyLen != 0)//(AC_WLAN[i]->KeyLen != 0)
			{
				CWProtocolStoreRawBytes(msgPtr,wlaninfo->WlanKey,wlaninfo->KeyLen /*AC_WLAN[i]->WlanKey, AC_WLAN[i]->KeyLen*/);
			}
		}
		CWProtocolStore32(msgPtr, 0);
		CWProtocolStore16(msgPtr, 0);
		CWProtocolStore8(msgPtr, 0);
		CWProtocolStore8(msgPtr, 0);
		if((policy == NO_INTERFACE) || (wlan_tunnel_switch == 1)){/* yjl 2014-2-28 */
			CWProtocolStore8(msgPtr, CW_LOCAL_MAC);	
			CWProtocolStore8(msgPtr, CW_LOCAL_BRIDGING);
		}else{
			CWProtocolStore8(msgPtr, CW_SPLIT_MAC);	
			if(tunnel == CW_802_IPIP_TUNNEL){
				CWProtocolStore8(msgPtr, CW_802_IPIP_TUNNEL);
			}else if(tunnel == CW_802_DOT_3_TUNNEL){
				CWProtocolStore8(msgPtr,CW_802_DOT_3_TUNNEL);/*802.3*/
				wid_syslog_info("%s,%d,802.3.\n",__func__,__LINE__);
			}else{
				CWProtocolStore8(msgPtr, CW_802_DOT_11_TUNNEL);
			}
		}
		CWProtocolStore8(msgPtr, wlaninfo->HideESSid/*AC_WLAN[i]->HideESSid*/);
		/*fengwenchao modify 20121129 for autelan-3284*/
		//CWProtocolStoreStr(msgPtr, essid);
		CWProtocolStoreRawBytes(msgPtr,essid,ESSID_LENGTH);

		/*fengwenchao modify end*/
		if(wlaninfo){
			WID_FREE(wlaninfo);
			wlaninfo = NULL;
		}
		return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_ADD_WLAN);
	}
	else if(op == WLAN_DEL){	
		
		//printf("del wlan\n");
		wid_syslog_debug_debug(WID_WTPINFO,"wtp%d disable wlan :%d\n",WTPIndex,i);
		int size = 2;
		//unsigned int BSSIndex = AC_WLAN[i]->S_WTP_BSS_List[WTPIndex][RadioID];
		//unsigned char L_ID = BSSIndex%L_BSS_NUM;		
		//AC_WLAN[i]->S_WTP_BSS_List[WTPIndex][RadioID] = 0;	
		
		/*AC_WTP[WTPIndex]->CMD->wlanCMD -= 1;
		AC_WTP[WTPIndex]->CMD->radiowlanid[RadioID][i] = 0;
	 	if(AC_BSS[BSSIndex] != NULL){
			AC_BSS[BSSIndex]->State = 0;
			if(AC_BSS[BSSIndex]->BSSID != NULL){
				memset(AC_BSS[BSSIndex]->BSSID, 0, 6);
			}
			if(AC_BSS[BSSIndex]->BSS_TUNNEL_POLICY == CW_802_IPIP_TUNNEL)
			{
				delete_ipip_tunnel(BSSIndex);
			}
		}*/
		//AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[AC_WLAN[i]->S_WTP_BSS_List[WTPIndex][RadioID] % L_BSS_NUM]->enable_wlan_flag = 0;
		CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, WID_FREE(wlaninfo); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
		CWProtocolStore8(msgPtr, RadioID);
		CWProtocolStore8(msgPtr, i);		
		if(wlaninfo){
			WID_FREE(wlaninfo);
			wlaninfo = NULL;
		}
		return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_DELETE_WLAN);

	}else if(op == WLAN_CHANGE_TUNNEL){
		int size = 4;
		wid_syslog_debug_debug(WID_WTPINFO,"wtp %d update wlan :%d\n",WTPIndex,i);
	
		int k1;
		unsigned char policy;
		unsigned char tunnel = 0;
		for(k1=0;k1<L_BSS_NUM;k1++){
			if((AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1] != NULL)&&(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->WlanID == i)){
				policy = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSS_IF_POLICY;
				tunnel = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[k1]->BSS_TUNNEL_POLICY;
				break;
			}
		}
		
		if(policy == NO_INTERFACE){
			WID_FREE(wlaninfo);
			return CW_FALSE;
		}		
		CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, WID_FREE(wlaninfo); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
		CWProtocolStore8(msgPtr, RadioID);
		CWProtocolStore8(msgPtr, i);
		
		CWProtocolStore8(msgPtr, CW_SPLIT_MAC);	
		if(tunnel == CW_802_IPIP_TUNNEL){
			CWProtocolStore8(msgPtr, CW_802_IPIP_TUNNEL);
		}else if(tunnel == CW_802_DOT_3_TUNNEL){
			CWProtocolStore8(msgPtr,CW_802_DOT_3_TUNNEL);/*802.3*/
			wid_syslog_info("%s,%d,802.3.\n",__func__,__LINE__);
		}else{
			CWProtocolStore8(msgPtr, CW_802_DOT_11_TUNNEL);
		}

		if(wlaninfo){
			WID_FREE(wlaninfo);
			wlaninfo = NULL;
		}
		return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_CHANGE_TUNNEL_MODE);
	}
	if(wlaninfo){
		WID_FREE(wlaninfo);
		wlaninfo = NULL;
	}
	return CW_FALSE;
 }
CWBool CWAssembleMsgElemWlanVlanPriority(CWProtocolMessage *msgPtr, int WTPIndex, unsigned char RadioID, int id) {
	int size = 6;
	unsigned int val = 0;
	//parse bss vlan info
	int i = 0;
	unsigned char priority = 0;
	unsigned int vlan_id = 0;
	for(i=0;i<L_BSS_NUM;i++)
	{
		if(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i] != NULL)
		{
			if(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i]->WlanID == id)
			{
				if(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i]->BSS_IF_POLICY != NO_INTERFACE){
					if((AC_WLAN[id] != NULL)&&(AC_WLAN[id]->wlan_tunnel_switch ==0))/* yjl 2014-2-28 */
					    break;
				}
				if(AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i]->vlanid != 0)
				{
					vlan_id = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i]->vlanid;
				}
				else
				{
					vlan_id = AC_WTP[WTPIndex]->WTP_Radio[RadioID]->BSS[i]->wlan_vlanid;
				}
				priority = AC_WLAN[id]->wlan_1p_priority;
				break;
			}
		}
	}
	wid_syslog_debug_debug(WID_WTPINFO,"CWAssembleMsgElemWlanVlanPriority radioid %d wtpindex %d wlanid %d\n",RadioID,WTPIndex,id);
	wid_syslog_debug_debug(WID_WTPINFO,"vlanid %d wlan_1p_priority %d\n", vlan_id,priority);
	if(AC_WLAN[id] == NULL)
	{
		return CW_FALSE;
	}
	else
	{
		CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		CWProtocolStore8(msgPtr, RadioID);
		CWProtocolStore8(msgPtr, id);
		CWSetField32(val,
				 0,
				 16,
				 0x8100);
		CWSetField32(val,
				 16,
				 3,
				 priority);
		CWSetField32(val,
				 19,
				 1,
				 0);
		CWSetField32(val,
				 20,
				 12,
				 vlan_id);
		CWProtocolStore32(msgPtr, val);	
		return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_IEEE80211_WLAN_VLAN_INFO);
	}
	return CW_FALSE;
}
CWBool CWAssembleMsgElemWlanWapiCerInfo(CWProtocolMessage *msgPtr, int WTPIndex, unsigned char RadioID, int id) {
	int size = 0;
	unsigned int lenth = 0;
	//printf("lenth %d\n",lenth);
	wid_syslog_debug_debug(WID_WTPINFO,"CWAssembleMsgElemWlanWapiCerInfo radioid %d wtpindex %d wlanid %d\n",RadioID,WTPIndex,id);
	if(AC_WLAN[id] == NULL)
	{
		return CW_FALSE;
	}
	else
	{
		lenth = (AC_WLAN[id]->AECerLen + AC_WLAN[id]->ASCerLen + AC_WLAN[id]->IpLen)+12;
		size = lenth;
		CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		//CWProtocolStore16(msgPtr, BINDING_MSG_ELEMENT_TYPE_WAPI_CER_INFO_ELEMENT);
		//CWProtocolStore16(msgPtr, lenth);
		CWProtocolStore16(msgPtr, 0);//as path 
		CWProtocolStore16(msgPtr, AC_WLAN[id]->ASCerLen);
		CWProtocolStoreRawBytes(msgPtr,AC_WLAN[id]->ASCerPath,AC_WLAN[id]->ASCerLen);
		//printf("AC_WLAN[id]->ASCerLen %d\n",AC_WLAN[id]->ASCerLen);
		//printf("AC_WLAN[id]->ASCerPath %s\n",AC_WLAN[id]->ASCerPath);
		
		CWProtocolStore16(msgPtr, 1); //ae path
		CWProtocolStore16(msgPtr, AC_WLAN[id]->AECerLen);
		CWProtocolStoreRawBytes(msgPtr,AC_WLAN[id]->AECerPath,AC_WLAN[id]->AECerLen);
		//printf("AC_WLAN[id]->AECerLen %d\n",AC_WLAN[id]->AECerLen);
		//printf("AC_WLAN[id]->AECerPath %s\n",AC_WLAN[id]->AECerPath);
		
		CWProtocolStore16(msgPtr, 2); //as ip
		CWProtocolStore16(msgPtr, AC_WLAN[id]->IpLen);
		CWProtocolStoreRawBytes(msgPtr,AC_WLAN[id]->AsIp,AC_WLAN[id]->IpLen);
		//printf("AC_WLAN[id]->asip %d\n",AC_WLAN[id]->IpLen);
		//printf("AC_WLAN[id]->asip %s\n",AC_WLAN[id]->AsIp);
		return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WAPI_CER_INFO_ELEMENT);
	}
	return CW_FALSE;
}

CWBool CWAssembleMsgElemStaOp(CWProtocolMessage *msgPtr, int WTPIndex, struct msgqlist *elem) {
	int size = 23;
	unsigned char tmpval = 6;
	int tmpvalint = 6;
	CWProtocolMessage msgPtr2;
	unsigned short  len = 8;
	unsigned short vendorvalue ;
	unsigned short sta_num = 0;
	CWBool  ret = CW_FALSE; 
	vendorvalue = 30;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, elem->mqinfo.u.StaInfo.Radio_L_ID);
	CWProtocolStore8(msgPtr, tmpval);
	CWProtocolStoreRawBytes(msgPtr, (elem->mqinfo.u.StaInfo.STAMAC), tmpvalint);
	CWProtocolStore8(msgPtr, elem->mqinfo.u.StaInfo.WLANID);
	if(elem->mqinfo.u.StaInfo.Sta_Op == Sta_ADD) {
		unsigned int	wtpid;
		unsigned int 	l_radioid;
		unsigned char 	wlanid;
		unsigned char 	mac[MAC_LEN];
		unsigned int 	value, send_value, vlan_id;
		wtpid = WTPIndex;
		l_radioid = elem->mqinfo.u.StaInfo.Radio_L_ID;
		wlanid = elem->mqinfo.u.StaInfo.WLANID;
		memcpy(mac,elem->mqinfo.u.StaInfo.STAMAC,MAC_LEN);
		value = elem->mqinfo.u.StaInfo.traffic_limit;
		send_value = elem->mqinfo.u.StaInfo.send_traffic_limit;
		vlan_id = elem->mqinfo.u.StaInfo.vlan_id;
		wid_syslog_debug_debug(WID_DEFAULT,"\n %s\n",__func__);					
		wid_syslog_debug_debug(WID_DEFAULT,"traffic_limit %d\n",value); 				
		wid_syslog_debug_debug(WID_DEFAULT,"send_traffic_limit %d\n",send_value);					
		wid_syslog_debug_debug(WID_DEFAULT,"vlan_id %d\n",vlan_id); 				
		if(value != 0)
			wid_set_bss_traffic_limit_sta_value(wtpid,l_radioid,wlanid,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],value,0);
		if(send_value != 0)
			wid_set_bss_traffic_limit_sta_value(wtpid,l_radioid,wlanid,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],send_value,1);
		if(vlan_id > 0)
			wid_set_sta_vlan_id(wtpid,l_radioid,wlanid,mac,vlan_id);
		//weichao add for check sta num
		wid_syslog_debug_debug(WID_WTPINFO,"\nmsg ofset = %d!\n",__func__,msgPtr->offset);							
		wid_syslog_debug_debug(WID_WTPINFO,"\n %s:now go go go!\n",__func__);					
		
		sta_num = elem->mqinfo.u.StaInfo.sta_num;
		
		wid_syslog_debug_debug(WID_WTPINFO,"\n %s:sta_num :%d!\n",__func__,elem->mqinfo.u.StaInfo.sta_num);					
		wid_syslog_debug_debug(WID_WTPINFO,"\n %s:sta_num :%d!\n",__func__,sta_num);					
		CW_CREATE_PROTOCOL_MESSAGE(msgPtr2, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		wid_syslog_debug_debug(WID_WTPINFO,"1111111111111\n");
		CWProtocolStore16(&msgPtr2,vendorvalue);
		CWProtocolStore16(&msgPtr2,len-4);
		CWProtocolStore16(&msgPtr2, sta_num);
		CWProtocolStore16(&msgPtr2,0);//reserved
		CWAssembleMsgElemVendor(&msgPtr2,CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
		CWProtocolStoreMessage(msgPtr,&msgPtr2);
		CW_FREE_PROTOCOL_MESSAGE(msgPtr2);
		//end
		
		wid_syslog_debug_debug(WID_WTPINFO,"\n %s:now go end!\n",__func__);					
		wid_syslog_debug_debug(WID_WTPINFO,"\nmsg ofset = %d!\n",__func__,msgPtr->offset);							
		ret = CWAssembleMsgElemAddVendor(msgPtr, CW_MSG_ELEMENT_ADD_STATION_CW_TYPE);
		return ret;
	}else if(elem->mqinfo.u.StaInfo.Sta_Op == Sta_DEL)
		return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DELETE_STATION_CW_TYPE);
	else
		return CW_FALSE;
}
/*
CWBool CWAssembleMsgElemDeleteSta(CWProtocolMessage *msgPtr, int WTPIndex) {
	int size = 9;
	unsigned char tmpval = 6;
	int tmpvalint = 6;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, AC_WTP[WTPIndex]->CMD->StaInf[0]);
	CWProtocolStore8(msgPtr, tmpval);
	CWProtocolStoreRawBytes(msgPtr, &(AC_WTP[WTPIndex]->CMD->StaInf[1]), tmpvalint);
	CWProtocolStore8(msgPtr, AC_WTP[WTPIndex]->CMD->StaInf[7]);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_DELETE_STATION_CW_TYPE);
}
*/
CWBool CWAssembleMsgElemSetStaKey(CWProtocolMessage *msgPtr, int WTPIndex, struct msgqlist *elem) {
	int size = 20;
	int storesize = 6;
	size+= elem->mqinfo.u.StaInfo.keylen;
	//printf("key len %d\n",AC_WTP[WTPIndex]->CMD->key.key_len);
	unsigned int val = 0;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStoreRawBytes(msgPtr, (elem->mqinfo.u.StaInfo.STAMAC), storesize);	
	CWSetField32(val,
			 0,
			 1,
			 1);	
	CWSetField32(val,
			 1,
			 1,
			 0);
	CWSetField32(val,
			 2,
			 2,
			 elem->mqinfo.u.StaInfo.keyidx);	
	CWSetField32(val,
			 4,
			 8,
			 elem->mqinfo.u.StaInfo.keylen);	
	CWSetField32(val,
			 12,
			 20,
			 0);
	CWProtocolStore32(msgPtr, val);	
	CWProtocolStore16(msgPtr, 0);
	CWProtocolStore32(msgPtr, 0);	
	CWProtocolStore32(msgPtr, 0);
	CWProtocolStoreRawBytes(msgPtr, elem->mqinfo.u.StaInfo.key,elem->mqinfo.u.StaInfo.keylen);
//	CWCaptrue(msgPtr->offset, msgPtr->msg);
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_STATION_SESSION_KEY);
}


int RadioNumCheck(int WTPIndex){
	if(AC_WTP[WTPIndex] == NULL)
	{
		return 0;
	}
	return AC_WTP[WTPIndex]->RadioCount;

}

CWBool CWAssembleStaConfigurationRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem){
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 1;	
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	int k = -1;
//	unsigned char m = 0;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	wid_syslog_debug_debug(WID_WTPINFO,"Sending STA configuration Request...");
	
	// Assemble Message Elements
//	for(m = 0; m < msgElemCount; m++)
	printf("########  sta operate ########\n");
	wid_syslog_debug_debug(WID_WTPINFO,"########  sta operate ########\n");
	if((!(CWAssembleMsgElemStaOp(&(msgElems[++k]), WTPIndex, elem))))
	{
		int i;
		for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT_WID(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}
	
	return CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT);



}


CWBool CWAssembleStaConfigurationRequest_key(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem){
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 2;	
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	int k = -1;
//	unsigned char m = 0;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	wid_syslog_debug_debug(WID_WTPINFO,"Sending STA configuration Request...");
	

	if ((!(CWAssembleMsgElemSetStaKey(&(msgElems[++k]), WTPIndex, elem)))||		
	(!(CWAssembleMsgElemIEEE80211Info(&(msgElems[++k]), WTPIndex, 0, -1, elem)))
	) {
		int i;
		for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT_WID(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}
	
	return CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT);



}




CWBool CWBindingAssembleWlanConfigurationRequest(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int WTPIndex, struct msgqlist *elem){
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 4;	
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	int k = -1, i =0, op = 0;
	unsigned char m = 0;
	//int RCount = RadioNumCheck(WTPIndex);
	//msgElemCount = RCount*msgElemCount;
	wid_syslog_debug_debug(WID_WTPINFO,"msgElemCount %d \n",msgElemCount);
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	wid_syslog_debug_debug(WID_WTPINFO,"Sending wlan configuration Request...");
	
/*	for(i=0; i < WLAN_NUM; i++){
		
		if((AC_WTP[WTPIndex]->CMD->wlanid[i] == 1)||(AC_WTP[WTPIndex]->CMD->wlanid[i] == 3))
		{
			//added by weiay 20080624
			if(AC_WTP[WTPIndex]->Wlan_Id != NULL)
			{
				struct wlanid *pwlanid = AC_WTP[WTPIndex]->Wlan_Id;
				while(pwlanid != NULL)
				{
					//printf("*** list wtp id:%d binding wlan id:%d***\n",WTPIndex,pwlanid->wlanid);
					wid_syslog_debug_debug("*** list wtp id:%d binding wlan id:%d***",WTPIndex,pwlanid->wlanid);
					if(i == pwlanid->wlanid)
					{
						break;
					}
					pwlanid = pwlanid->next;
				}

				if(pwlanid == NULL)
				{
					//printf("*** error wtp id:%d does not find binding wlan id:%d***\n",WTPIndex,i);
					wid_syslog_debug_debug("*** error wtp id:%d does not find binding wlan id:%d***");
					continue;
				}
				else
				{
					wid_syslog_debug_debug("*** success wtp id:%d binding wlan id:%d***",WTPIndex,i);
					break;
				}
			}
			else
			{
				wid_syslog_debug_debug("*** ignore wtp id:%d does not binding any wlan id ***\n",WTPIndex);
				break;
			}
			//added end
		}
		else if(i < WLAN_NUM)
			continue;
		else{
			AC_WTP[WTPIndex]->CMD->wlanCMD = 0;
			return CW_FALSE;
		}
	}
	// Assemble Message Elements
	if(i >= WLAN_NUM)
	{
		return CW_FALSE;
	}*/
	//for(m = 0; m < RCount; m++){
	i = elem->mqinfo.u.WlanInfo.WLANID;
	m = elem->mqinfo.u.WlanInfo.Radio_L_ID;
	op = elem->mqinfo.u.WlanInfo.Wlan_Op;
	if((i >= WLAN_NUM)||(AC_WLAN[i] ==NULL)){		
		wid_syslog_debug_debug(WID_WTPINFO,"wlan :%d = NULL\n",i);
		CW_FREE_OBJECT_WID(msgElems);		
		return CW_FALSE; // error will be handled by the caller
	}
	if(!check_wlanid_func(i)){
		CW_FREE_OBJECT_WID(msgElems);		
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	if(!check_l_radioid_func(m)){
		CW_FREE_OBJECT_WID(msgElems);		
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	if(!check_wtpid_func(WTPIndex)){
		CW_FREE_OBJECT_WID(msgElems);		
		wid_syslog_err("%s\n",__func__);
		return CW_FALSE;
	}else{
	}
	if ((!(CWAssembleMsgElemAddWlan(&(msgElems[++k]), WTPIndex, m, i, op,elem)))||
		(!(CWAssembleMsgElemIEEE80211Info(&(msgElems[++k]), WTPIndex, m, i, elem)))||
		(!(CWAssembleMsgElemWlanVlanPriority(&(msgElems[++k]), WTPIndex, m, i)))||
		(!(CWAssembleMsgElemWlanWapiCerInfo(&(msgElems[++k]), WTPIndex, m, i)))
	) {
		int j;
		for(j = 0; j <= k; j++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);}
		CW_FREE_OBJECT_WID(msgElems);		
		return CW_FALSE; // error will be handled by the caller
	}
	
	//}
	return CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IEEE80211_WLAN_CONFIGURATION_REQUEST, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT);



}


CWBool CWParseMsgElemAssignedWTPBSSID(CWProtocolMessage *msgPtr, int len, WID_BSS *valPtr) 
{
	CWParseMessageElementStart();
	
	
	valPtr->Radio_L_ID = CWProtocolRetrieve8(msgPtr);
	valPtr->WlanID = CWProtocolRetrieve8(msgPtr);
	valPtr->BSSID = (unsigned char*)CWProtocolRetrieveRawBytes(msgPtr, 6);

	CWParseMessageElementEnd();
}





CWBool CWParseWlanConfigurationResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode, WID_BSS *BSS)
{
	int offsetTillMessages;
	//WID_BSS BSS;
	if(msgPtr == NULL || resultCode==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	wid_syslog_debug_debug(WID_WTPINFO,"Parsing WLAN Configuration Response...");

	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				*resultCode=CWProtocolRetrieve32(msgPtr);
				break;
			case BINDING_MSG_ELEMENT_TYPE_ASSIGNED_WTP_BSSID:
				if(CWParseMsgElemAssignedWTPBSSID(msgPtr, elemLen, BSS)){
		/*			int i=0;
					for(i=0;i<4;i++){
					 if((AC_RADIO[BSS.Radio_G_ID]->BSS[i] == NULL))
						break;
					 else if(AC_RADIO[BSS.Radio_G_ID]->BSS[i]->WlanID == BSS.WlanID){
						memcpy(AC_RADIO[BSS.Radio_G_ID]->BSS[i]->BSSID,BSS.BSSID,6);
						printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",BSS.BSSID[0],BSS.BSSID[1],BSS.BSSID[2],BSS.BSSID[3],BSS.BSSID[4],BSS.BSSID[5]);
					 }
					 else
					 	i++;
					}*/
					//printf("BSS\n");
				}
				break;
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				break;	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");

	wid_syslog_debug_debug(WID_WTPINFO,"WLAN Configuration Response Parsed");

	return CW_TRUE;	
}
/*
static int to_asd_or_not(unsigned int wtpid){
//	1	---	to asd
//	0	---	not
//	-1	---	error
	int i=0 , j=0;
	unsigned char wlanid=0;
	
	if(AC_WTP[wtpid] == NULL){
		return -1;
	}

	for(i=0;i<L_RADIO_NUM;i++){
		for(j=0;j<L_BSS_NUM;j++){
			if((AC_WTP[wtpid]->WTP_Radio[i]!=NULL)
				&&(AC_WTP[wtpid]->WTP_Radio[i]->BSS[j]!=NULL)){
				wlanid = AC_WTP[wtpid]->WTP_Radio[i]->BSS[j]->WlanID;

				if(	(AC_WLAN[wlanid]!=NULL)
					&&(AC_WLAN[wlanid]->balance_switch==1)
					&&(AC_WLAN[wlanid]->balance_method==2)){
					return 1;
				}
			}
		}
	}

	return 0;

}
*/

