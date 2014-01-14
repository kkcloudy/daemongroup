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
* ACReset.c
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
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "ACDbus.h"
#include "syslog.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
extern unsigned char gWIDLOGHN;//qiuchen

CWBool CWAssembleResetRequestMessage(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWImageIdentifier *resPtr){
	
		CWProtocolMessage *msgElems = NULL;
		const int MsgElemCount=1;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Reset...\n");
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		// Assemble Message Elements
		if ((!(CWAssembleMsgElemImageIdentifierAC(&(msgElems[++k]), resPtr)))){
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT_WID(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
				
	//wid_syslog_debug_debug("~~~~~ msg count: %d ", msgElemBindingCount);
		
		if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_RESET_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		//printf("Reset request Assembled\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Reset request Assembled\n");
		return CW_TRUE;
}


CWBool ACEnterReset(int WTPIndex, CWProtocolMessage *msgPtr)
{
	
	//printf("\n");
	//printf("######### Reset State #########\n");	
	wid_syslog_debug_debug(WID_WTPINFO,"######### WTP %d Enter Reset State #########\n",WTPIndex);
	CWControlHeaderValues controlVal;
	unsigned short ele_type;
	unsigned short ele_len;
	unsigned char total_phase, current_phase, state, result;
	
	//CWProtocolMessage* messages =NULL;
	//int messagesCount=0;
	if(gtrapflag>=1){
		wid_dbus_trap_wtp_ap_reboot(WTPIndex);
		}
	msgPtr->offset = 0;
	
	if(!(CWACParseGenericRunMessage(WTPIndex, msgPtr, &controlVal))) {
		//## Two possible errors: WRONG_ARG and INVALID_FORMAT
		//## In the second case we have an unexpected response: ignore the
		//## message and log the event.
		if (controlVal.messageTypeValue == CW_MSG_TYPE_VALUE_RESET_RESPONSE) {

			ele_type = CWProtocolRetrieve16(msgPtr);
			ele_len = CWProtocolRetrieve16(msgPtr);
			
			if (ele_type == CW_MSG_ELEMENT_WTP_IMAGE_STATE_REPORT) {
				
				total_phase = CWProtocolRetrieve8(msgPtr);
				current_phase = CWProtocolRetrieve8(msgPtr);
				state = CWProtocolRetrieve8(msgPtr);
				result = CWProtocolRetrieve8(msgPtr);
				
				if (total_phase == 0) {
					wid_syslog_err("imagedata_percent_d1:error total phase:0 for wtp %d\n", WTPIndex);
					return CW_TRUE;
				}

				if (state == 0) {
					wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:stage %d start\n", current_phase);
					if (current_phase == total_phase) {
						wid_syslog_debug_debug(WID_DEFAULT, "1:the last state start treat as done\n");
						AC_WTP[WTPIndex]->image_data_percent = 100;
					} else {
						wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:not the last state start\n");
						AC_WTP[WTPIndex]->image_data_percent = (current_phase-1)*100/total_phase;
					}
				} else {
					wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:stage %d done\n", current_phase);
					if (current_phase == total_phase) {
						wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:the last state done\n");
						AC_WTP[WTPIndex]->image_data_percent = 100;
					} else {
						wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:not the last state done\n");
						AC_WTP[WTPIndex]->image_data_percent = current_phase*100/total_phase;
					}
				}
				wid_syslog_debug_debug(WID_DEFAULT, "imagedata_percent_d1:AC_WTP[WTPIndex]->image_data_percent=%d\n", AC_WTP[WTPIndex]->image_data_percent);
			}
		}
		return CW_FALSE;
	}
	switch(controlVal.messageTypeValue) {
		case CW_MSG_TYPE_VALUE_RESET_RESPONSE:{
			wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_TYPE_VALUE_RESET_RESPONSE\n");
			gWTPs[WTPIndex].currentState = CW_QUIT; 
			AC_WTP[WTPIndex]->WTPStat = 7;			
			AC_WTP[WTPIndex]->image_data_percent = 0;
			AC_WTP[WTPIndex]->image_data_step = 0;
			AC_WTP[WTPIndex]->image_data_result = 0;
			syslog_wtp_log(WTPIndex, 0, "for update", 0);
			if(gWIDLOGHN & 0x01)
				syslog_wtp_log_hn(WTPIndex,0,0);
			if(gWIDLOGHN & 0x02)
				wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[WTPIndex],0);
			gWTPs[WTPIndex].isRequestClose = CW_TRUE;
			_CWCloseThread(WTPIndex);
			break;			
		}
		default:	
			wid_syslog_debug_debug(WID_WTPINFO,"%d\n",controlVal.messageTypeValue);
			gWTPs[WTPIndex].currentState = CW_QUIT; 
			AC_WTP[WTPIndex]->WTPStat = 7;			
			syslog_wtp_log(WTPIndex, 0, "something wrong in update", 0);
			if(gWIDLOGHN & 0x01)
				syslog_wtp_log_hn(WTPIndex,0,0);
			if(gWIDLOGHN & 0x02)
				wid_syslog_auteview(LOG_WARNING,AP_DOWN,AC_WTP[WTPIndex],0);
			_CWCloseThread(WTPIndex);
			break;
	}
	
	return CW_TRUE;
}



