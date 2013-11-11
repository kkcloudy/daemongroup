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
* ACJoinState.c
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
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus_handler.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWAssembleJoinResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWList msgElemList,char *strVersion);
CWBool CWParseJoinRequestMessage(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr, int WTPIndex);
CWBool CWSaveJoinRequestMessage(CWProtocolJoinRequestValues *joinRequest, CWWTPProtocolManager *WTPProtocolManager,unsigned int WTPIndex);


CWBool ACEnterJoin(int WTPIndex, CWProtocolMessage *msgPtr)
{	
	int seqNum;
	CWProtocolJoinRequestValues joinRequest;
	memset(&joinRequest,0,sizeof(CWProtocolJoinRequestValues));
	CWList msgElemList = NULL;
	int ret = 0;
	wid_syslog_debug_debug(WID_WTPINFO,"######### WTP %d Enter Join State #########",WTPIndex);	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(!(CWParseJoinRequestMessage(msgPtr->msg, msgPtr->offset, &seqNum, &joinRequest, WTPIndex))) {
		return CW_FALSE;
		// note: we can kill our thread in case of out-of-memory error to free some space/
		// we can see this just calling CWErrorGetLastErrorCode()
	}
#if 0
	if(*gcurrent_wtp_count[gWTPs[WTPIndex].oemoption] > *gmax_wtp_count[gWTPs[WTPIndex].oemoption])
	{
		printf("###can not access ap type %d access count over type count\n",gWTPs[WTPIndex].oemoption);
		wid_syslog_debug_debug(WID_WTPINFO,"###can not access ap type %d access count over type count",gWTPs[WTPIndex].oemoption);	
		return CW_FALSE;
	}
#endif
	// cancel waitJoin timer
	if(!CWTimerCancel(&(gWTPs[WTPIndex].currentTimer),1))
	{
		return CW_FALSE;
	}
	if((find_in_wtp_list(WTPIndex) == CW_TRUE))
	{
//		printf("CWTimerCancel\n");	
		if(!CWTimerCancel(&(gWTPs[WTPIndex].updateTimer),1))
		{
			printf("CWTimerCancel error\n");
			wid_syslog_info("CWTimerCancel error\n");
			
			return CW_FALSE;
		}
	}

	CWBool ACIpv4List = CW_FALSE;
	CWBool ACIpv6List = CW_FALSE;
	CWBool resultCode = CW_TRUE;
	int resultCodeValue = CW_PROTOCOL_SUCCESS;
	//CWBool sessionID = CW_FALSE;
/*	unsigned char *mac;	
	mac = (unsigned char*)malloc(6);
	memset(mac,0,6);
	if(!(CWCheckWTPBoardData(WTPIndex, &(joinRequest.WTPBoardData), mac))){
		gWTPs[WTPIndex].currentState = CW_QUIT;
		AC_WTP[WTPIndex]->WTPStat = 0;
		memset(AC_WTP[WTPIndex]->WTPIP, 0, 128);
		free(mac);
		mac = NULL;
		return CW_TRUE;
	}	
	memcpy(AC_WTP[WTPIndex]->WTPMAC, mac, 6);
	free(mac);
	mac = NULL;*/
	if(!(CWSaveJoinRequestMessage(&joinRequest,&(gWTPs[WTPIndex].WTPProtocolManager),WTPIndex))) 
	{
		resultCodeValue = CW_PROTOCOL_FAILURE_RES_DEPLETION;
	}
	
	CWMsgElemData *auxData;
	if(ACIpv4List) {
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                auxData->type = CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE;
		auxData->value = 0;
		CWAddElementToList(&msgElemList,auxData);
	}
	if(ACIpv6List){
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                auxData->type = CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE;
                auxData->value = 0;
                CWAddElementToList(&msgElemList,auxData);
	}
	if(resultCode){
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                auxData->type =  CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE;
                auxData->value = resultCodeValue;
                CWAddElementToList(&msgElemList,auxData);
	}
// 	if(sessionID){
// 		CW_CREATE_OBJECT_ERR(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
//                 auxData->type =  CW_MSG_ELEMENT_SESSION_ID_CW_TYPE;
//                 auxData->value = CWRandomIntInRange(0, INT_MAX);
//                 CWAddElementToList(&msgElemList,auxData);
// 	}

	//added by weiay 20080618


	CWWTPDescriptor *valPtr = &gWTPs[WTPIndex].WTPProtocolManager.descriptor;
//	unsigned char *str_wtp_version = NULL;
	
	CWWTPVendorInfos *vendorPtr = &gWTPs[WTPIndex].WTPProtocolManager.WTPBoardData;
	unsigned char *str_wtp_model = NULL;
		
	CWConfigVersionInfo * pVersionNode = gConfigVersionInfo;
	CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
	CWBool bMatchVersion = CW_FALSE;
	int i=0,len=0,aclen=0,lenmodel=0,verlen=0,result=0;

	char *str_ac_version = NULL;
	
	for(i = 0; i < vendorPtr->vendorInfosCount; i++)
	{

		if((vendorPtr->vendorInfos)[i].type == 0)//CW_WTP_MODEL_NUMBER = 0
		{

			str_wtp_model = (vendorPtr->vendorInfos)[i].model;
			lenmodel = (vendorPtr->vendorInfos)[i].length;
			
			if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->APCode)&&(strncmp(AC_WTP[WTPIndex]->APCode,(char *)str_wtp_model,lenmodel) != 0))
			{
				CW_FREE_OBJECT_WID(AC_WTP[WTPIndex]->APCode);
				AC_WTP[WTPIndex]->APCode = (char *)WID_MALLOC(lenmodel+1);
				memset(AC_WTP[WTPIndex]->APCode,0,lenmodel+1);
				memcpy(AC_WTP[WTPIndex]->APCode,str_wtp_model,lenmodel);
			}
			//break;
		}
		else if((vendorPtr->vendorInfos)[i].type == 6)//CW_WTP_CODE_VERSION = 6
		{
			AC_WTP[WTPIndex]->codever = (char*)(vendorPtr->vendorInfos)[i].codever;
			len = (vendorPtr->vendorInfos)[i].length;
		}
	}

/*	//special upgrade 20081007 by weiay
	if((lenmodel == 4)&&(*(vendorPtr->vendorInfos)[0].valuePtr == 1000))
	{
//		printf("##### goto image data ######\n");
		//char *str = "1.0.12";
		str_ac_version = pVersionNode->str_ap_version_name;
		goto inter_image_date;
		
	}
	//printf("##### i= %d######\n",i);
*/	
	wid_syslog_debug_debug(WID_WTPINFO,"** wtp model:%s len:%d **\n",str_wtp_model,lenmodel);


	for(i = 0; i < valPtr->vendorInfos.vendorInfosCount; i++)
	{
		if((valPtr->vendorInfos.vendorInfos)[i].type == 0)//CW_WTP_HARDWARE_VERSION = 0
		{
			char * sysver;
			int m = 0;
			wid_syslog_debug_debug(WID_DEFAULT,"** (valPtr->vendorInfos.vendorInfos)[i].sysver %s **\n",(valPtr->vendorInfos.vendorInfos)[i].sysver);
			sysver = (char *)(valPtr->vendorInfos.vendorInfos)[i].sysver;

			for(m = 0; m < (valPtr->vendorInfos.vendorInfos)[i].length; m++){
				wid_syslog_debug_debug(WID_DEFAULT,"** (valPtr->vendorInfos.vendorInfos)[i].sysver[%d] %d %c**\n",m,sysver[m],sysver[m]);
				if((sysver[m] < 33)||(sysver[m] > 126))
					break;
			}
			if(m == (valPtr->vendorInfos.vendorInfos)[i].length)
				AC_WTP[WTPIndex]->sysver = (char *)(valPtr->vendorInfos.vendorInfos)[i].sysver;
			else{
				AC_WTP[WTPIndex]->sysver = NULL;
				wid_syslog_info("WTP %d HD version %s something wrong\n",WTPIndex,sysver);
			}
		}

		if((valPtr->vendorInfos.vendorInfos)[i].type == 1)//CW_WTP_SOFTWARE_VERSION = 1
		{
			ret = wid_illegal_character_check((char *)(valPtr->vendorInfos.vendorInfos)[i].ver,(valPtr->vendorInfos.vendorInfos)[i].length, 0);
			if(ret == 1){
				AC_WTP[WTPIndex]->ver = (char *)(valPtr->vendorInfos.vendorInfos)[i].ver;
				verlen = (valPtr->vendorInfos.vendorInfos)[i].length;
			}else{
				AC_WTP[WTPIndex]->ver = NULL;
				str_ac_version = (char *)(valPtr->vendorInfos.vendorInfos)[i].ver;				
				verlen = (valPtr->vendorInfos.vendorInfos)[i].length;
				wid_syslog_info("WTP %d SW version %s something wrong\n",WTPIndex,str_ac_version);
			}
			break;
		}
	}

	wid_syslog_debug_debug(WID_WTPINFO,"** start version match **\n");
	wid_syslog_debug_debug(WID_WTPINFO,"** wtp version:%s **\n",AC_WTP[WTPIndex]->ver);
	if(AC_WTP[WTPIndex]->codever != NULL)
	{
		wid_syslog_debug_debug(WID_WTPINFO,"** wtp code version:%s **\n",AC_WTP[WTPIndex]->codever);
	}
	wid_syslog_debug_debug(WID_WTPINFO,"** wtp model:%s **\n",str_wtp_model);

	//printf("ACEnterJoin\n");
	if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
	{
		str_ac_version = AC_WTP[WTPIndex]->updateversion;

		if(AC_WTP[WTPIndex]->codever == NULL){
			if((strlen(AC_WTP[WTPIndex]->updateversion) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,AC_WTP[WTPIndex]->updateversion,verlen) == 0))
			{
				bMatchVersion = CW_TRUE;
			}		
		}
		else if((AC_WTP[WTPIndex]->codever != NULL)&&(strlen(AC_WTP[WTPIndex]->updateversion) == len)&&(strncasecmp(AC_WTP[WTPIndex]->codever,AC_WTP[WTPIndex]->updateversion,len) == 0))
		{
			str_ac_version = AC_WTP[WTPIndex]->ver;//zhanglei add for ap bug,cao
			bMatchVersion = CW_TRUE;
		}		
	}
	else
	{

		if((find_in_wtp_list(WTPIndex) == CW_TRUE))
		{
			wid_syslog_debug_debug(WID_WTPINFO,"*** enter bacth upgrade ***\n");	//for test
			for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
				CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
				if((update_node != NULL)&&(update_node->str_ap_model != NULL)&&(strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0)){
					while(update_node != NULL){
						wid_syslog_debug_debug(WID_WTPINFO,"*** upgrade node is not null***\n");	//for test
						wid_syslog_debug_debug(WID_WTPINFO,"** ap code: %s **\n",AC_WTP[WTPIndex]->APCode);	//for test
						wid_syslog_debug_debug(WID_WTPINFO,"** upgrade node code: %s **\n",update_node->str_ap_code);	//for test
						wid_syslog_debug_debug(WID_WTPINFO,"** upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
						wid_syslog_debug_debug(WID_WTPINFO,"** upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test
						if((update_node->str_ap_code != NULL)&&(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0)){
							wid_syslog_debug_debug(WID_WTPINFO,"*** code is same***\n");	//for test
							str_ac_version = update_node->str_ap_version_name;
							//printf("ac update version:%s\n",str_ac_version);
							if(AC_WTP[WTPIndex]->codever == NULL){
								if((strlen(update_node->str_ap_version_name) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,update_node->str_ap_version_name,verlen) == 0))
								{
									bMatchVersion = CW_TRUE;
								}
							}
							else if((AC_WTP[WTPIndex]->codever != NULL)&&(strlen(update_node->str_ap_version_name) == len)&&(strncasecmp(AC_WTP[WTPIndex]->codever,update_node->str_ap_version_name,len) == 0))
							{
								str_ac_version = AC_WTP[WTPIndex]->ver;//zhanglei add for ap bug,cao
								bMatchVersion = CW_TRUE;
							}
							result = 1;
							break;
						}
						update_node = update_node->next;
					}
					if(update_node == NULL){
						str_ac_version = AC_WTP[WTPIndex]->ver;
						bMatchVersion = CW_TRUE;
					}
					break;
				}
			}
		}
		else
		{
			int do_check = 0;
			wid_syslog_debug_debug(WID_WTPINFO,"**** ap model is %s ****\n",AC_WTP[WTPIndex]->WTPModel);	//for test
			
//			CWThreadMutexLock(&(gAllThreadMutex));
			pVersionNode = gConfigVersionInfo;
			tmpnode = gConfigVerInfo;
			
			if(img_now == 0){
				wid_syslog_debug_debug(WID_WTPINFO,"ap model match 111\n"); 	//for test
				str_ac_version = AC_WTP[WTPIndex]->ver;
				bMatchVersion = CW_TRUE;
			}
			else while(pVersionNode != NULL)
			{
				str_ac_version = AC_WTP[WTPIndex]->ver;
				if(strcmp(pVersionNode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){	
					wid_syslog_debug_debug(WID_WTPINFO,"**** find model in wtpcompatible.xml ****\n");	//for test
					
					/*use while here to match the right information*/
					while(tmpnode != NULL){
						if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){
							wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
							CWCodeInfo *codenode = tmpnode->code_info;
							while(codenode != NULL){
								wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n");	//for test
								if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0){
									do_check = 1;
									str_ac_version = codenode->str_ap_version_name;
									aclen = strlen(codenode->str_ap_version_name);
									wid_syslog_debug_debug(WID_WTPINFO,"ac surport version:%s\n",str_ac_version);
									wid_syslog_debug_debug(WID_WTPINFO,"** AC version name len:%d   WTP version name len:%d **\n",aclen,len);
									
									if((AC_WTP[WTPIndex]->codever == NULL)){						
										if((strlen(codenode->str_ap_version_name) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,codenode->str_ap_version_name,verlen) == 0))
										{
											wid_syslog_debug_debug(WID_WTPINFO,"ap model match 222\n"); 	//for test
											bMatchVersion = CW_TRUE;
											break;
										}
									}
									else if((aclen == len)&&(AC_WTP[WTPIndex]->codever != NULL)&&(strncasecmp(AC_WTP[WTPIndex]->codever,codenode->str_ap_version_name,len) == 0))
									{
										wid_syslog_debug_debug(WID_WTPINFO,"ap model match 333\n"); 	//for test
										bMatchVersion = CW_TRUE;
										str_ac_version = AC_WTP[WTPIndex]->ver;//zhanglei add for ap bug,cao
										break;
									}
									break;
								}
								codenode = codenode->next;
							}
						}
						if(do_check == 1)
							break;
						tmpnode = tmpnode->next;
					}
					if(do_check == 0){
						bMatchVersion = CW_TRUE;
						str_ac_version = AC_WTP[WTPIndex]->ver; 		
					}
					break;
				}
				pVersionNode = pVersionNode->next;
			}
			if (do_check == 0){
				bMatchVersion = CW_TRUE;
				str_ac_version = AC_WTP[WTPIndex]->ver; 		
			}
			//			CWThreadMutexUnlock(&(gAllThreadMutex));		
#if 0
			    if(pVersionNode->str_ap_code != NULL)//book add, 2011-11-15
			    {
				aclenmodel = strlen(pVersionNode->str_ap_code);
				wid_syslog_debug_debug(WID_WTPINFO,"** AC surport model:%s **\n",pVersionNode->str_ap_code);
				wid_syslog_debug_debug(WID_WTPINFO,"** AC version model len:%d WTP model len:%d **\n",aclenmodel,lenmodel);
				
				if((((aclenmodel == lenmodel)&&(strncasecmp((char*)str_wtp_model,pVersionNode->str_ap_code,lenmodel) == 0))||
					((strlen(pVersionNode->str_ap_model) == lenmodel)&&(strncasecmp((char*)str_wtp_model,pVersionNode->str_ap_model,lenmodel) == 0)))
					//&&((strlen(pVersionNode->str_ap_model) == strlen(AC_WTP[WTPIndex]->WTPModel))&&(strncasecmp(AC_WTP[WTPIndex]->WTPModel,pVersionNode->str_ap_model,strlen(AC_WTP[WTPIndex]->WTPModel)) == 0))
					)
				{	
					do_check = 1;
					str_ac_version = pVersionNode->str_ap_version_name;
					wid_syslog_debug_debug(WID_WTPINFO,"ac surport version:%s\n",str_ac_version);
					aclen = strlen(pVersionNode->str_ap_version_name);
					wid_syslog_debug_debug(WID_WTPINFO,"** AC surport version:%s **\n",pVersionNode->str_ap_version_name);
					wid_syslog_debug_debug(WID_WTPINFO,"** AC version name len:%d WTP version name len:%d **\n",aclen,len);
					
					if((AC_WTP[WTPIndex]->codever == NULL)&&(img_now == 0)){
						str_ac_version = AC_WTP[WTPIndex]->ver;
						bMatchVersion = CW_TRUE;
						break;						
					}else if((AC_WTP[WTPIndex]->codever == NULL)){						
						if((strlen(pVersionNode->str_ap_version_name) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,pVersionNode->str_ap_version_name,verlen) == 0))
						{
							bMatchVersion = CW_TRUE;
							break;
						}
					}
					else if((img_now == 0)||((aclen == len)&&(AC_WTP[WTPIndex]->codever != NULL)&&(strncasecmp(AC_WTP[WTPIndex]->codever,pVersionNode->str_ap_version_name,len) == 0)))
					{
						bMatchVersion = CW_TRUE;
						str_ac_version = AC_WTP[WTPIndex]->ver;//zhanglei add for ap bug,cao
						break;
					}
				}
    			}
#endif
		}
	}

	//inter_image_date:

		
	//printf("##### version = %s######\n",str_ac_version);	
	if(bMatchVersion){
		if(!(CWAssembleJoinResponse(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount), gWTPs[WTPIndex].pathMTU, seqNum, msgElemList,str_ac_version))){ // random session ID
			CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
			return CW_FALSE;
		}
	}else{
		char ac_version[DEFAULT_LEN] = {" "};
		memset(ac_version,0,DEFAULT_LEN);
		sprintf(ac_version,"%s1",str_ac_version);
		wid_syslog_info("WTP[%d] after:%s",WTPIndex,ac_version);
		if(!(CWAssembleJoinResponse(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount), gWTPs[WTPIndex].pathMTU, seqNum, msgElemList,ac_version))){ // random session ID
			CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
			return CW_FALSE;
		}
	}
	CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
	
	if(!CWACSendFragments(WTPIndex)) {
		return CW_FALSE;
 	}
	for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
		CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
	}
	
	CW_FREE_OBJECT_WID(gWTPs[WTPIndex].messages);
	if(!CWErr(CWTimerRequest(CW_JOIN_INTERVAL_DEFAULT, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) { // start Change State Pending timer
		_CWCloseThread(WTPIndex);
	}

	if(bMatchVersion)
	{
		gWTPs[WTPIndex].currentState = CW_ENTER_CONFIGURE;	
		AC_WTP[WTPIndex]->WTPStat = 3;
	}
	else
	{
	
		if((updatewtplist != NULL)&&(find_in_wtp_list(WTPIndex) == CW_FALSE))
		{
			if(updatewtplist->count >= gupdateCountOneTime)
			{
				gWTPs[WTPIndex].currentState = CW_QUIT; 
				AC_WTP[WTPIndex]->WTPStat = 7;	
				_CWCloseThread(WTPIndex);
			}
			else
			{
				gWTPs[WTPIndex].currentState = CW_ENTER_IMAGE_DATA; 
				AC_WTP[WTPIndex]->WTPStat = 8;
			}
		}
		else
		{
			gWTPs[WTPIndex].currentState = CW_ENTER_IMAGE_DATA; 
			AC_WTP[WTPIndex]->WTPStat = 8;
		}
	}
	AC_WTP[WTPIndex]->quitreason = WTP_NORMAL;
	wid_syslog_debug_debug(WID_WTPINFO,"** match result is:%d Next state is:%d **\n",bMatchVersion,gWTPs[WTPIndex].currentState);
	//wid_syslog_debug_info("** WTP %d quit reason is %d **",WTPIndex,AC_WTP[WTPIndex]->quitreason);
	return CW_TRUE;
}

// assemble Join Response
CWBool CWAssembleJoinResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWList msgElemList,char *strVersion)
{	
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 0;
	const int mandatoryMsgElemCount=4; 	//Result code is not included because it's already in msgElemList. Control IPv6 to be added
	CWProtocolMessage *msgElemsBinding= NULL;
	const int msgElemBindingCount=0;
	int i;
	CWListElement *current;
	int k = -1;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL || msgElemList == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	msgElemCount = CWCountElementInList(msgElemList);

	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount+mandatoryMsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	wid_syslog_debug_debug(WID_WTPINFO,"Assembling Join Response...");
	
	if((!(CWAssembleMsgElemACDescriptor(&(msgElems[++k])))) ||
	   (!(CWAssembleMsgElemACName(&(msgElems[++k])))) ||
	   (!(CWAssembleMsgElemWTPVersion(&(msgElems[++k]),strVersion))) || //added by weiay 20080618
	   (!(CWAssembleMsgElemCWControlIPv4Addresses(&(msgElems[++k]),0)))//zhanglei set wtpid 0 in join	  
	){
		CWErrorHandleLast();
		int i;
		for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT_WID(msgElems);
		return CW_FALSE; // error will be handled by the caller
	} 

	current=msgElemList;
	for (i=0; i<msgElemCount; i++)
	{
                switch (((CWMsgElemData *) (current->data))->type)
		{
			case CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE:
				if (!(CWAssembleMsgElemACIPv4List(&(msgElems[++k]))))
					goto cw_assemble_error;	
				break;			
			case CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE:
				if (!(CWAssembleMsgElemACIPv6List(&(msgElems[++k]))))
					goto cw_assemble_error;
				break;
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				if (!(CWAssembleMsgElemResultCode(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
					goto cw_assemble_error;
				break;
/*			case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
				if (!(CWAssembleMsgElemSessionID(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
					goto cw_assemble_error;
				break;*/
                        default: {
                                int j;
                                for(j = 0; j <= k; j++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);}
                                CW_FREE_OBJECT_WID(msgElems);
                                return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element for Join Response Message");
				break;
		        }
                }

		current = current->next;
	}

	if (!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_JOIN_RESPONSE, msgElems, msgElemCount+mandatoryMsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT)))
		return CW_FALSE;

	wid_syslog_debug_debug(WID_WTPINFO,"Join Response Assembled");
	
	return CW_TRUE;

cw_assemble_error:
	{
		int i;
		for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT_WID(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}

	return CW_TRUE;
}

// Parse Join Request
CWBool CWParseJoinRequestMessage(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr, int WTPIndex) {
	CWControlHeaderValues controlVal;
	
	int offsetTillMessages;
	//int i=0;
	CWProtocolMessage completeMsg;
	
	if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Parse Join Request");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
		
	if(!(CWParseControlHeader(&completeMsg, &controlVal))) return CW_FALSE; // will be handled by the caller

	// different type
	if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_JOIN_REQUEST)
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Join Request as Expected");
	
	*seqNumPtr = controlVal.seqNum;
	controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS; // skip timestamp
	
	offsetTillMessages = completeMsg.offset;
	
	// parse message elements
	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(&completeMsg,&elemType,&elemLen);
		
		//wid_syslog_debug_debug("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
									
		switch(elemType) {
			case CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE:
				if(!(CWParseLocationData(&completeMsg, elemLen, &(valuesPtr->location)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE:{
				if(!(CWParseWTPBoardData(&completeMsg, elemLen, &(valuesPtr->WTPBoardData)))) return CW_FALSE; // will be handled by the caller
				break; 

			}
			case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
				if(!(CWParseSessionID(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE:
				if(!(CWParseWTPDescriptor(&completeMsg, elemLen, &(valuesPtr->WTPDescriptor)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_IPV4_ADDRESS_CW_TYPE:
				if(!(CWParseWTPIPv4Address(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_NAME_CW_TYPE:
				if(!(CWParseWTPName(&completeMsg, elemLen, &(valuesPtr->name)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE:
				if(!(CWParseWTPFrameTunnelMode(&completeMsg, elemLen, &(valuesPtr->frameTunnelMode)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE:
				if(!(CWParseWTPMACType(&completeMsg, elemLen, &(valuesPtr->MACType)))) return CW_FALSE; // will be handled by the caller
				break;
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element");
		}
		
		//wid_syslog_debug_debug("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);
	}
	
	if(completeMsg.offset != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
		
	return CW_TRUE;
}

CWBool CWSaveJoinRequestMessage (CWProtocolJoinRequestValues *joinRequest, CWWTPProtocolManager *WTPProtocolManager, unsigned int WTPIndex)
{  
	wid_syslog_debug_debug(WID_WTPINFO,"Saving Join Request...");
	int i;
	if(joinRequest == NULL || WTPProtocolManager == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if ((joinRequest->location)!= NULL)
	{
		CW_FREE_OBJECT_WID(WTPProtocolManager->locationData);
		WTPProtocolManager->locationData= joinRequest->location;
	}
	else return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if ((joinRequest->name)!= NULL)
	{
		CW_FREE_OBJECT_WID(WTPProtocolManager->name);
		WTPProtocolManager->name= joinRequest->name;
	}
	else return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	for(i = 0; i < WTPProtocolManager->WTPBoardData.vendorInfosCount; i++) {

		if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].model);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].SN);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].mac);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].Rmodel);
		}
		else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_CODE_VERSION)
		{
			AC_WTP[WTPIndex]->codever = NULL;
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].codever);
		}
		else
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].valuePtr);
		}
	}
	CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData).vendorInfos);
	WTPProtocolManager->WTPBoardData = joinRequest->WTPBoardData;

	WTPProtocolManager->sessionID= joinRequest->sessionID;
	WTPProtocolManager->ipv4Address= joinRequest->addr;
	
	for(i = 0; i < (WTPProtocolManager->descriptor.vendorInfos).vendorInfosCount; i++) {

		if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].sysver);
		}else if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
		{
			AC_WTP[WTPIndex]->ver = NULL;
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].ver);
		}
		else
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
	}
	CW_FREE_OBJECT_WID((WTPProtocolManager->descriptor.vendorInfos).vendorInfos);
	WTPProtocolManager->descriptor= joinRequest->WTPDescriptor;
	WTPProtocolManager->radiosInfo.radioCount = (joinRequest->WTPDescriptor).radiosInUse;
	CW_FREE_OBJECT_WID(WTPProtocolManager->radiosInfo.radiosInfo);
	CW_CREATE_ARRAY_ERR(WTPProtocolManager->radiosInfo.radiosInfo, WTPProtocolManager->radiosInfo.radioCount, CWWTPRadioInfoValues,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	

	for(i=0; i< WTPProtocolManager->radiosInfo.radioCount; i++)
	{
		WTPProtocolManager->radiosInfo.radiosInfo[i].radioID = i;
                //WTPProtocolManager->radiosInfo.radiosInfo[i].stationCount = 0;
                WTPProtocolManager->radiosInfo.radiosInfo[i].adminState = ENABLED; //default value for CAPWAP
                WTPProtocolManager->radiosInfo.radiosInfo[i].adminCause = AD_NORMAL;
                WTPProtocolManager->radiosInfo.radiosInfo[i].operationalState = DISABLED;
                WTPProtocolManager->radiosInfo.radiosInfo[i].operationalCause = OP_NORMAL;
                WTPProtocolManager->radiosInfo.radiosInfo[i].TxQueueLevel = 0;
                WTPProtocolManager->radiosInfo.radiosInfo[i].wirelessLinkFramesPerSec = 0; 
	}
	wid_syslog_debug_debug(WID_WTPINFO,"Join Request Saved");
	return CW_TRUE;
}
