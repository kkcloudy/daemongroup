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
* ACImageData.c
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
#include "ACImageData.h"
#include "ACDbus.h"
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "ACDbus.h"
#include "ACDbus_handler.h"
#include "ACUpdateManage.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int gCWImageDataPendingTimer = CW_IMAGE_INTERVAL_DEFAULT;


CWBool CWCheckImageIdentifier(CWImageIdentifier *valPtr, CWImageIdentifier *resPtr){
	CWConfigVersionInfo *info = gConfigVersionInfo;
	if(info == NULL){
		wid_syslog_err("gConfigVersionInfo is NULL\n");
		return CW_FALSE;
	}
	while(info != NULL){
		//printf("info->str_ap_version_name %s\n",info->str_ap_version_name);
		wid_syslog_err("info->str_ap_version_name %s\n",info->str_ap_version_name);
		unsigned int model_len = 0;
		
		model_len = strlen(info->str_ap_model);
		if(model_len == valPtr->modelLEN)
		{
			if((memcmp(info->str_ap_version_name, valPtr->Ver, valPtr->VerLen) == 0)
				||((memcmp(info->str_ap_model, valPtr->model, valPtr->modelLEN) == 0)&&(memcmp(info->str_ap_version_name, valPtr->Ver, valPtr->VerLen) == 0)))
			{
				resPtr->Ver = info->str_ap_version_path;
				resPtr->VerLen = strlen(info->str_ap_version_path);
				//printf("give ap version is:%s\n",resPtr->Ver);
				wid_syslog_err("give ap version is:%s\n",resPtr->Ver);
				resPtr->VerType = 0;
				resPtr->VerNum = 1;
				return CW_TRUE;
			}
		}
		info = info->next;
	}
	wid_syslog_err("info == NULL\n");
	//printf("info == NULL\n");
	return CW_FALSE;
}
CWBool CWParseMsgElemImageIdentifier(CWProtocolMessage *msgPtr, int len, CWImageIdentifier *valPtr) 
{
	wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemImageIdentifier\n");
	CWParseMessageElementStart();
	int a=0, b=0;
	valPtr->VerNum = CWProtocolRetrieve8(msgPtr);
	a = CWProtocolRetrieve8(msgPtr);
	b = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"a %d b %d\n",a,b);
	valPtr->VerType = CWProtocolRetrieve16(msgPtr);
	valPtr->VerLen = CWProtocolRetrieve16(msgPtr);
	if((valPtr->VerLen<0)||(valPtr->VerLen>64)){
		wid_syslog_err("%s VerLen==%d\n",__func__,valPtr->VerLen);
		valPtr->VerLen = 64;
	}else{
	
	}

	switch(valPtr->VerType)
	{
		case 1000:
			
			CW_CREATE_STRING_ERR(valPtr->model, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 5);
			memcpy(valPtr->model,"2010",4);
			valPtr->modelLEN = 4;
			wid_syslog_debug_debug(WID_WTPINFO,"2010\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;
		case 2000:
			CW_CREATE_STRING_ERR(valPtr->model, 5, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 6);
			memcpy(valPtr->model,"1110T",5);
			valPtr->modelLEN = 5;
			wid_syslog_debug_debug(WID_WTPINFO,"1110T\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;
		case 1:
			valPtr->modelLEN = valPtr->VerLen;
			valPtr->model = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			valPtr->VerLen = CWProtocolRetrieve16(msgPtr);
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			//printf("valPtr->model:%s,valPtr->VerLen:%d,valPtr->Ver:%s",valPtr->model,valPtr->VerLen,valPtr->Ver);
			break;
		default:
			CW_CREATE_STRING_ERR(valPtr->model, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 5);
			memcpy(valPtr->model,"1000",4);
			valPtr->modelLEN = 4;
			wid_syslog_debug_debug(WID_WTPINFO,"1000\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;			

	}
	
	wid_syslog_debug_debug(WID_WTPINFO,"valPtr->VerLen %d  valPtr->Ver %s valPtr->model %s \n",valPtr->VerLen,valPtr->Ver,valPtr->model);
	//printf("valPtr->VerLen %d  valPtr->Ver %s valPtr->model %s \n",valPtr->VerLen,valPtr->Ver,valPtr->model);
	CWParseMessageElementEnd();
}


CWBool CWParseImageDataRequestMessage(CWProtocolMessage* msgPtr, int len, CWImageDataRequest *valuesPtr){
	int offsetTillMessages;
	if(msgPtr == NULL || valuesPtr==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	//printf("Parsing Image data request...\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Parsing Image data request...");
	valuesPtr->ImageRequest = NULL;
	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:
				CW_CREATE_OBJECT_ERR(valuesPtr->ImageRequest, CWImageIdentifier, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
				if (!(CWParseMsgElemImageIdentifier(msgPtr, elemLen, valuesPtr->ImageRequest))){
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWParseMsgElemImageIdentifier\n");
					return CW_FALSE;
				}
				break;	
			default:				
				wid_syslog_debug_debug(WID_WTPINFO,"wrong in default\n");
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Image data request");
				break;	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len){ 
		//printf("((msgPtr->offset - offsetTillMessages) != len)\n");
		wid_syslog_debug_debug(WID_WTPINFO,"((msgPtr->offset - offsetTillMessages) != len)");
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	}
	//printf("Image data request Parsed\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Image data request Parsed");
	return CW_TRUE;	
}

CWBool CWAssembleMsgElemImageIdentifierAC(CWProtocolMessage *msgPtr, CWImageIdentifier *resPtr){
	int size = 8;
	size += resPtr->VerLen;	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	CWProtocolStore8(msgPtr, resPtr->VerNum);
	CWProtocolStore8(msgPtr, 0);	
	CWProtocolStore16(msgPtr, 0);
	CWProtocolStore16(msgPtr, resPtr->VerType);	
	CWProtocolStore16(msgPtr, resPtr->VerLen);
	CWProtocolStoreRawBytes(msgPtr, resPtr->Ver, resPtr->VerLen);
	//wid_syslog_debug_debug("resPtr->VerLen %d resPtr->Ver %s\n",resPtr->VerLen, resPtr->Ver);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE);
}

CWBool CWAssembleImageDataRequestMessage(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWImageIdentifier *resPtr){
	
		CWProtocolMessage *msgElems = NULL;
		const int MsgElemCount=1;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		//printf("Assembling Image Data...\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Image Data...");
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		// Assemble Message Elements
		if ((!(CWAssembleMsgElemImageIdentifierAC(&(msgElems[++k]), resPtr)))){
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
				
	//wid_syslog_debug_debug("~~~~~ msg count: %d ", msgElemBindingCount);
		
		if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		
		//printf("Image Data request Assembled\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data request Assembled");
		return CW_TRUE;
}

CWBool CWParseImageDataResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode)
{
	int offsetTillMessages;
	if(msgPtr == NULL || resultCode==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	
	//printf("Parsing Image Data Response...\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Parsing Image Data Response...");
	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				*resultCode=CWProtocolRetrieve32(msgPtr);
				break;	
			default:
				//printf("default\n");
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				break;	
		}
	}
	//printf("*resultCode %d\n",*resultCode);
	wid_syslog_debug_debug(WID_WTPINFO,"*resultCode %d.\n",*resultCode);
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");

	//printf("Image Data Response Parsed\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Image Data Response Parsed.\n");
	return CW_TRUE;	
}



CWBool ACEnterImageData(int WTPIndex, CWProtocolMessage *msgPtr)
{
	int i=0,result=0;
	//printf("\n");
	//printf("######### Image Data State #########\n");	
	wid_syslog_debug_debug(WID_WTPINFO,"######### WTP %d Enter Image Data State #########\n",WTPIndex);
	CWControlHeaderValues controlVal;
	//CWProtocolMessage* messages =NULL;
	//int messagesCount=0;

	msgPtr->offset = 0;
	
	if(!(CWACParseGenericRunMessage(WTPIndex, msgPtr, &controlVal))) {
		//## Two possible errors: WRONG_ARG and INVALID_FORMAT
		//## In the second case we have an unexpected response: ignore the
		//## message and log the event.
		return CW_FALSE;
	}
	
	if(!CWErr(CWTimerCancel(&(gWTPs[WTPIndex].currentTimer),1)))
	{
		_CWCloseThread(WTPIndex);
	}

	switch(controlVal.messageTypeValue) {
		case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:{
			CWImageDataRequest valuesPtr;
			CWImageIdentifier resPtr;
			if(!(CWParseImageDataRequestMessage(msgPtr, controlVal.msgElemsLen, &valuesPtr))){
				//printf("wrong in CWParseImageDataRequestMessage\n");
				wid_syslog_warning("wrong in CWParseImageDataRequestMessage");
				return CW_FALSE;		
			}


			
		
			if(gtrapflag>=1){
				wid_dbus_trap_wtp_enter_imagedata_state(WTPIndex);//wu:apÉý¼¶¸æ¾¯
			}
						
			
			//printf("ACEnterImageData\n");
			if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
			{
				resPtr.Ver = AC_WTP[WTPIndex]->updatepath;
				resPtr.VerLen = strlen(AC_WTP[WTPIndex]->updatepath);
				resPtr.VerType = 0;
				resPtr.VerNum = 1;				
			}
			else
			{
				/*mahz modify code here to match ap upgrade batchlly*/
				CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
				CWBool bMatchVersion = CW_FALSE;
				
				if(find_in_wtp_list(WTPIndex) == CW_TRUE){
					for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
						CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
						if((update_node != NULL)&&(update_node->str_ap_model != NULL)&&(strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0)){
							while(update_node != NULL){
								wid_syslog_debug_debug(WID_WTPINFO,"*** enter image data upgrade ***\n");	//for test
								if(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0)
								{
									wid_syslog_debug_debug(WID_WTPINFO,"***111 upgrade node is not null***\n");	//for test
									wid_syslog_debug_debug(WID_WTPINFO,"**111 ap code: %s **\n",AC_WTP[WTPIndex]->APCode); //for test
									wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node code: %s **\n",update_node->str_ap_code);	//for test
									wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
									wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test

									resPtr.Ver = update_node->str_ap_version_path;
									resPtr.VerLen = strlen(update_node->str_ap_version_path);
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									bMatchVersion = CW_TRUE;

									if(AC_WTP[WTPIndex]->updateStat >= 1)
									{
										AC_WTP[WTPIndex]->updatefailcount++;	
									}

									if(AC_WTP[WTPIndex]->updatefailcount >= updatemaxfailcount)
									{
										delete_wtp_list(WTPIndex);
										insert_uptfail_wtp_list(WTPIndex);
										
										update_complete_check();						

										AC_WTP[WTPIndex]->updatefailstate = 1;

										_CWCloseThread(WTPIndex);
									}
									//printf("ACEnterImageData update\n");

									result = 1;
									break;
								}
								update_node = update_node->next;
							}
							if(bMatchVersion == CW_FALSE)	//it will not goto this branch
							{
								resPtr.Ver = valuesPtr.ImageRequest->Ver;
								resPtr.VerLen = valuesPtr.ImageRequest->VerLen;
								resPtr.VerType = 0;
								resPtr.VerNum = 1;
								wid_syslog_debug_debug(WID_WTPINFO,"check image data ver response\n");	//for test
								/*if(!(CWCheckImageIdentifier(valuesPtr.ImageRequest, &resPtr))){
									//printf("wrong in CWCheckImageIdentifier\n");
									wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWCheckImageIdentifier\n");
									return CW_FALSE;
								}*/
							}
							break;
						}
					}
				}
				/*ap upgrade automatically here,the gloable variable gConfigVersionUpdateInfo may be NULL here,so use gConfigVerInfo instead*/
				/*the upgrade automatically can be used now,but after the upgrade is over,the wtpcompatible.xml is not updated,so what the cmd
				show model list shows is still old information, this situation should be modified sooner or later*/
				else if(img_now == 1){
					while(tmpnode != NULL){
						if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){
							wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
							CWCodeInfo *codenode = tmpnode->code_info;
							while(codenode != NULL){
								wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n"); //for test
								if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0){
									resPtr.Ver = codenode->str_ap_version_path;
									resPtr.VerLen = strlen(codenode->str_ap_version_path);
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									
									wid_syslog_debug_debug(WID_WTPINFO,"ac surport version name:%s\n",codenode->str_ap_version_name);
									break;
								}
								codenode = codenode->next;
							}
							break;
						}
						tmpnode = tmpnode->next;
					}
				}
			}
			gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
			if(!CWAssembleImageDataRequestMessage(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount), gWTPs[WTPIndex].pathMTU, controlVal.seqNum, &resPtr)){
				//printf("wrong in CWAssembleImageDateRequestMessage\n");
				wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleImageDateRequestMessage\n");
				return CW_FALSE;
			}
			gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE;
			AC_WTP[WTPIndex]->updateStat = 1;


							
			if(gtrapflag>=4){
					wid_dbus_trap_wtp_tranfer_file(WTPIndex);
			}
			//fengwenchao add 20110216   save ap report version  for ap updata successful or fail
			AC_WTP[WTPIndex]->ApReportVer = malloc(strlen(valuesPtr.ImageRequest->Ver)+1);
			memset(AC_WTP[WTPIndex]->ApReportVer,0,strlen(valuesPtr.ImageRequest->Ver)+1);
			memcpy(AC_WTP[WTPIndex]->ApReportVer,valuesPtr.ImageRequest->Ver,strlen(valuesPtr.ImageRequest->Ver));
			//AC_WTP[WTPIndex]->ApReportVerLen = 	valuesPtr.ImageRequest->VerLen;
			//wid_syslog_debug_debug(WID_WTPINFO,"valuesPtr.ImageRequest->Ver = %s\n",valuesPtr.ImageRequest->Ver);
			//wid_syslog_debug_debug(WID_WTPINFO,"AC_WTP[WTPIndex]->ApReportVer = %s\n",AC_WTP[WTPIndex]->ApReportVer);
			//fengwenchao add end 
			CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
			CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
			CW_FREE_OBJECT(valuesPtr.ImageRequest);
			
			
			break;
		}
		case CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE:{
			//printf("CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE\n");
			wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE");
			CWProtocolResultCode resultCode;			
			CWImageIdentifier resPtr;
			
//			CWThreadMutexLock(&(gAllThreadMutex));		
			//ap doesn't use the value of resPtr in Reset state
			if(AC_WTP[WTPIndex]->ver != NULL)
				resPtr.Ver = AC_WTP[WTPIndex]->ver;
			else
				resPtr.Ver = " ";
			//resPtr.Ver = gConfigVersionInfo->str_ap_version_path;
			resPtr.VerNum = 1;
			resPtr.VerType = 0;
			resPtr.VerLen = strlen(resPtr.Ver);
			if(!(CWParseImageDataResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode))){
//				CWThreadMutexUnlock(&(gAllThreadMutex));		
				return CW_FALSE;
			}
			gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
			if(!CWAssembleResetRequestMessage(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount), gWTPs[WTPIndex].pathMTU, controlVal.seqNum, &resPtr)){
				//printf("wrong in CWAssembleResetRequestMessage\n");
				wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleResetRequestMessage");
	//			CWThreadMutexUnlock(&(gAllThreadMutex));		
				return CW_FALSE;
			}
			
//			CWThreadMutexUnlock(&(gAllThreadMutex));		
			if(resultCode == CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR)
			{	
				if(gtrapflag>=4){
					wid_dbus_trap_wtp_ap_flash_write_failed(WTPIndex);
					}
			}
			time(&AC_WTP[WTPIndex]->imagedata_time);
			gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_RESET_RESPONSE;
			gWTPs[WTPIndex].currentState = CW_ENTER_RESET;
			AC_WTP[WTPIndex]->WTPStat = 6;
			AC_WTP[WTPIndex]->updateStat = 2;
			break;			
		}
		default:
			//printf("controlVal.messageTypeValue %d\n",controlVal.messageTypeValue);
			result = 0;
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
				delete_wtp_list(WTPIndex);
				insert_uptfail_wtp_list(WTPIndex);	
				update_complete_check();
				
				AC_WTP[WTPIndex]->updatefailstate = 2;

				_CWCloseThread(WTPIndex);		

			}			
			wid_syslog_debug_debug(WID_WTPINFO,"controlVal.messageTypeValue %d",controlVal.messageTypeValue);
			return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
			break;
	}
	
	if(!CWACSendFragments(WTPIndex)) {
		return CW_FALSE;
	}
	
	//printf("Image Data Sent\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Image Data Sent");
	if(!CWErr(CWTimerRequest(gCWImageDataPendingTimer, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) { // start Change State Pending timer
		_CWCloseThread(WTPIndex);
	}
	
	for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
		CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
	}
	
	CW_FREE_OBJECT(gWTPs[WTPIndex].messages);

	
	return CW_TRUE;
}


