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
* CWBinding.c
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
#include "wcpss/wid/WID.h"


#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

const int gMaxCAPWAPHeaderSizeBinding = 16; // note: this include optional Wireless field


CWBool CWBindingCheckType(int elemType)
{
	if (elemType>=BINDING_MIN_ELEM_TYPE && elemType<=BINDING_MAX_ELEM_TYPE)
		return CW_TRUE;
	return CW_FALSE;
}

// Assemble a CAPWAP Data Packet creating Transport Header.
// completeMsgPtr is an array of fragments (can be of size 1 if the packet doesn't need fragmentation)
CWBool CWAssembleDataMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, CWProtocolMessage *frame, CWBindingTransportHeaderValues *bindingValuesPtr, int is_crypted) {
	CWProtocolMessage transportHdr = {0};
	CWProtocolTransportHeaderValues transportVal = {0};

	if(completeMsgPtr == NULL || fragmentsNumPtr == NULL || frame == NULL || bindingValuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
//	CWDebugLog("PMTU: %d", PMTU);
	
	// handle fragmentation
	
	PMTU = PMTU - gMaxCAPWAPHeaderSizeBinding;
	
	if(PMTU > 0) {
		PMTU = (PMTU/8)*8; // CAPWAP fragments are made of groups of 8 bytes
		if(PMTU == 0) goto cw_dont_fragment;
		
//		CWDebugLog("Aligned PMTU: %d", PMTU);
		*fragmentsNumPtr = (frame->offset) / PMTU;
		if((frame->offset % PMTU) != 0) (*fragmentsNumPtr)++;
		//CWDebugLog("Fragments #: %d", *fragmentsNumPtr);
	} else {
	cw_dont_fragment:
		*fragmentsNumPtr = 1;
	}
	
	transportVal.bindingValuesPtr = bindingValuesPtr;
		
	if(*fragmentsNumPtr == 1) {
//		CWDebugLog("1 Fragment");
			
		transportVal.isFragment = transportVal.last = transportVal.fragmentOffset = transportVal.fragmentID = 0;
		transportVal.payloadType = is_crypted;		

		// Assemble Message Elements
		if(!(CWAssembleTransportHeader(&transportHdr, &transportVal))) {
			CW_FREE_PROTOCOL_MESSAGE(transportHdr);
			return CW_FALSE; // will be handled by the caller
		} 
		
		CW_CREATE_OBJECT_ERR_WID(*completeMsgPtr, CWProtocolMessage, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[0]), transportHdr.offset + frame->offset, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		CWProtocolStoreMessage(&((*completeMsgPtr)[0]), &transportHdr);
		CWProtocolStoreMessage(&((*completeMsgPtr)[0]), frame);
		
		CW_FREE_PROTOCOL_MESSAGE(transportHdr);
	} else {
		int fragID = CWGetFragmentID();
		int totalSize = frame->offset;
		
		//CWDebugLog("%d Fragments", *fragmentsNumPtr);
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*completeMsgPtr, *fragmentsNumPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		frame->offset = 0;

		int i;
		for(i = 0; i < *fragmentsNumPtr; i++) { // for each fragment to assemble
			int fragSize;
			
			transportVal.isFragment = 1;
			transportVal.fragmentOffset = (frame->offset) / 8;
			transportVal.fragmentID = fragID;
			
			if(i < ((*fragmentsNumPtr)-1)) { // not last fragment
				fragSize = PMTU;
				transportVal.last = 0;
			} else { // last fragment
				fragSize = totalSize - (((*fragmentsNumPtr)-1) * PMTU);
				transportVal.last = 1;
			}
		
//			CWDebugLog("Fragment #:%d, offset:%d, bytes stored:%d/%d", i, transportVal.fragmentOffset, fragSize, totalSize);
			
			// Assemble Transport Header for this fragment
			if(!(CWAssembleTransportHeader(&transportHdr, &transportVal))) {
				CW_FREE_PROTOCOL_MESSAGE(transportHdr);
				CW_FREE_OBJECT_WID(completeMsgPtr);
				return CW_FALSE; // will be handled by the caller
			}
			
			CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[i]), transportHdr.offset + fragSize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			CWProtocolStoreMessage(&((*completeMsgPtr)[i]), &transportHdr);
			CWProtocolStoreRawBytes(&((*completeMsgPtr)[i]), &((frame->msg)[frame->offset]), fragSize);
			(frame->offset) += fragSize;
			
			CW_FREE_PROTOCOL_MESSAGE(transportHdr);
		}
	}
	return CW_TRUE;
}


CWBool CWAssembleTransportHeaderBinding(CWProtocolMessage *transportHdrPtr, CWBindingTransportHeaderValues *valuesPtr)
{
	int val = 0;
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_LENGTH_START,
		     CW_TRANSPORT_HEADER_LENGTH_LEN,
		     CW_BINDING_DATALENGTH);

//	CWDebugLog("#### RSSI in= %d",valuesPtr->RSSI );
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RSSI_START,
		     CW_TRANSPORT_HEADER_RSSI_LEN,
		     valuesPtr->RSSI);

//	CWDebugLog("#### SNR in= %d",valuesPtr->SNR );
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_SNR_START,
		     CW_TRANSPORT_HEADER_SNR_LEN,
		     valuesPtr->SNR);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_DATARATE_1_START,
		     CW_TRANSPORT_HEADER_DATARATE_1_LEN,
		     (valuesPtr->dataRate)>>8);

	CWProtocolStore32(transportHdrPtr, val);
	val = 0;

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_DATARATE_2_START,
		     CW_TRANSPORT_HEADER_DATARATE_2_LEN,
		     (valuesPtr->dataRate) & 0x000000FF);

//	CWDebugLog("#### data rate in= %d",valuesPtr->dataRate );
/*	CWSetField32(val,
		     CW_TRANSPORT_HEADER_DATARATE_START,
		     CW_TRANSPORT_HEADER_DATARATE_LEN,
		     valuesPtr->dataRate);
*/
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_PADDING_START,
		     CW_TRANSPORT_HEADER_PADDING_LEN,
		     0);

	CWProtocolStore32(transportHdrPtr, val);

	return CW_TRUE;
}

CWBool CWParseTransportHeaderBinding(CWProtocolMessage *msgPtr, CWBindingTransportHeaderValues *valuesPtr){
	unsigned int val = 0;
	
	if(msgPtr == NULL || valuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	//CWDebugLog("Parse Transport Header");
	val = CWProtocolRetrieve32(msgPtr);

	if(CWGetField32(val, CW_TRANSPORT_HEADER_LENGTH_START, CW_TRANSPORT_HEADER_LENGTH_LEN) != CW_BINDING_DATALENGTH)
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Wrong Binding Data Field Length");

	valuesPtr->RSSI = CWGetField32(val, CW_TRANSPORT_HEADER_RSSI_START, CW_TRANSPORT_HEADER_RSSI_LEN);
//	CWDebugLog("RSSI: %d", valuesPtr->RSSI);
	
	valuesPtr->SNR = CWGetField32(val, CW_TRANSPORT_HEADER_SNR_START, CW_TRANSPORT_HEADER_SNR_LEN);
//	CWDebugLog("SNR: %d", valuesPtr->SNR);
	
	valuesPtr->dataRate = CWGetField32(val, CW_TRANSPORT_HEADER_DATARATE_1_START, CW_TRANSPORT_HEADER_DATARATE_1_LEN);

	val = CWProtocolRetrieve32(msgPtr);

	valuesPtr->dataRate = ((valuesPtr->dataRate)<<8) | CWGetField32(val, CW_TRANSPORT_HEADER_DATARATE_1_START, CW_TRANSPORT_HEADER_DATARATE_1_LEN);

//	valuesPtr->dataRate = CWGetField32(val, CW_TRANSPORT_HEADER_DATARATE_START, CW_TRANSPORT_HEADER_DATARATE_LEN);
//	CWDebugLog("DATARATE: %d", valuesPtr->dataRate);
	
	return CW_TRUE;
}
/***************************************************************************
 * 
 * Function:  CWAssembleAssignedWTPBssid
 *
 * Purpose:  assemble the assigned WTP bssid element [binding 6.3]
 *
 * Inputs: 
 *
 * Output:    
 *
 * Returns:   
 **************************************************************************/
CWBool CWAssembleAssignedWTPBssid(CWProtocolMessage *bindPtr, char *bssid,int crete_wlan_id,int radio_id){
	CW_CREATE_PROTOCOL_MESSAGE(*bindPtr, 8, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(bindPtr,radio_id);
	CWProtocolStore8(bindPtr,crete_wlan_id);
	CWProtocolStoreRawBytes(bindPtr,bssid,6);
	return CWAssembleMsgElem(bindPtr, BINDING_MSG_ELEMENT_TYPE_ASSIGNED_WTP_BSSID);
}
