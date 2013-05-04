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
* ACDiscoveryState.c
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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */

__inline__ int CWACGetHWVersion();
__inline__ int CWACGetSWVersion();
__inline__ int CWACGetStations();
__inline__ int CWACGetLimit();
__inline__ int CWACGetActiveWTPs();
__inline__ int CWACGetMaxWTPs();
__inline__ int CWACGetSecurity();

__inline__ char * CWACGetName();

__inline__ int CWACGetInterfacesCount();
__inline__ int  CWACGetInterfacesIpv4Count(void);
__inline__ int  CWACGetInterfacesIpv6Count(void);

/*_________________________________________________________*/
/*  *******************___FUNCTIONS___*******************  */

// send Discovery Response to the host at the specified address
CWBool CWAssembleDiscoveryResponse(CWProtocolMessage **messagesPtr, int seqNum, unsigned int WTPID) {
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 3;
	CWProtocolMessage *msgElemsBinding= NULL;
	int msgElemBindingCount=0;
	int fragmentsNum;
	int k = -1;
	if(messagesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(CWACSupportIPv6()) {
		msgElemCount++;
	}
	wid_syslog_debug_debug(WID_WTPINFO,"Assemble Discovery Response");
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	// Assemble Message Elements
	if ((!(CWAssembleMsgElemACDescriptor(&(msgElems[++k])))) ||
		(!(CWAssembleMsgElemACName(&(msgElems[++k])))) ||
		(!(CWAssembleMsgElemCWControlIPv4Addresses(&(msgElems[++k]),WTPID)))||
		(CWACSupportIPv6() && (!(CWAssembleMsgElemCWControlIPv6Addresses(&(msgElems[++k])))))
	) {
		CWErrorHandleLast();
		int i;
		for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}
	
	return CWAssembleMessage(messagesPtr, &fragmentsNum, 0, seqNum, CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE, msgElems, msgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_PLAIN);
}

CWBool CWParseDiscoveryRequestMessage(char *msg, int len, int *seqNumPtr, CWDiscoveryRequestValues *valuesPtr) {
	CWControlHeaderValues controlVal;
	CWProtocolTransportHeaderValues transportVal;
	
	int offsetTillMessages;
	//int i;

	CWProtocolMessage completeMsg;
	
	if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	wid_syslog_debug_debug(WID_WTPINFO,"Parse Discovery Request");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
	
	CWBool dataFlag = CW_FALSE;
	if(!(CWParseTransportHeader(&completeMsg, &transportVal, &dataFlag))) return CW_FALSE; // will be handled by the caller
	if(!(CWParseControlHeader(&completeMsg, &controlVal))) return CW_FALSE; // will be handled by the caller
	
	// different type
	if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST)
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Discovery Request as Expected");
	
	*seqNumPtr = controlVal.seqNum;
	
	controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS; // skip timestamp
	
	offsetTillMessages = completeMsg.offset;
	
	//(*valuesPtr).radios.radiosCount = 0;
	
	// parse message elements
	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(&completeMsg,&elemType,&elemLen);		

//		wid_syslog_debug_debug("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
									
		switch(elemType) {
			case CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE:
				if(!(CWParseDiscoveryType(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE:
				if(!(CWParseWTPBoardData(&completeMsg, elemLen, &(valuesPtr->WTPBoardData)))) return CW_FALSE; // will be handled by the caller
				break; 
			case CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE:
				if(!(CWParseWTPDescriptor(&completeMsg, elemLen, &(valuesPtr->WTPDescriptor)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE:
				if(!(CWParseWTPFrameTunnelMode(&completeMsg, elemLen, &(valuesPtr->frameTunnelMode)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE:
				if(!(CWParseWTPMACType(&completeMsg, elemLen, &(valuesPtr->MACType)))) return CW_FALSE; // will be handled by the caller
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
		
		//wid_syslog_debug_debug("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);
	}
	
	if(completeMsg.offset != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	
/*
	// actually read each radio info
	CW_CREATE_ARRAY_ERR((*valuesPtr).radios.radios, (*valuesPtr).radios.radiosCount, CWRadioInformationValues, 
		return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	i = 0;
	
	completeMsg.offset = offsetTillMessages;
	while(i < (*valuesPtr).radios.radiosCount && (completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) {
		unsigned short int type=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int len=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);		

		switch(type) {
			case CW_MSG_ELEMENT_WTP_RADIO_INFO_CW_TYPE:
				if(!(CWParseWTPRadioInfo(&completeMsg, len, &(valuesPtr->radios), i))) return CW_FALSE; // will be handled by the caller
				i++;
				break;
			default:
				completeMsg.offset += len;
				break;
		}
	}
*/
	return CW_TRUE;
}

void CWDestroyDiscoveryRequestValues(CWDiscoveryRequestValues *valPtr) {
	int i;
	
	if(valPtr == NULL) return;
	
	for(i = 0; i < (valPtr->WTPDescriptor.vendorInfos).vendorInfosCount; i++) {
		//added by weiay 20080618
		if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
		{
			CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].sysver);
		}else if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
		{
			CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].ver);
		}
		else
		{
			CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
	}
	
	CW_FREE_OBJECT((valPtr->WTPDescriptor.vendorInfos).vendorInfos);

	for(i = 0; i < valPtr->WTPBoardData.vendorInfosCount; i++) {
		//added by weiay 20080618
		if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].model);
		}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].SN);
		}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].mac);
		}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].Rmodel);
		}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_CODE_VERSION)
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].codever);
		}
		else
		{
			CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].valuePtr);
		}
	}
	
	CW_FREE_OBJECT(valPtr->WTPBoardData.vendorInfos);
	
	//CW_FREE_OBJECT((valPtr->radios).radios);
}
