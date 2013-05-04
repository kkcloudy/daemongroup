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
* ACRetransmission.c
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

CWBool CWACSendFragments(int WTPIndex) {
	int i;
	if(gWTPs[WTPIndex].messages == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
#ifdef CW_NO_DTLS
		if(!CWNetworkSendUnsafeUnconnected(	gWTPs[WTPIndex].socket, 
							&gWTPs[WTPIndex].address, 
							gWTPs[WTPIndex].messages[i].msg, 
							gWTPs[WTPIndex].messages[i].offset)	) {
#else
		if(!(CWSecuritySend(gWTPs[WTPIndex].session, gWTPs[WTPIndex].messages[i].msg, gWTPs[WTPIndex].messages[i].offset))) {
#endif
			return CW_FALSE;
		}
	}
	wid_syslog_debug_debug(WID_WTPINFO,"Message Sent\n");
	
	return CW_TRUE;
}


CWBool CWACResendAcknowledgedPacket(int WTPIndex) {
	if(!CWACSendFragments(WTPIndex))
	   return CW_FALSE;
	
	CWThreadSetSignals(SIG_BLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);	
	if(!(CWTimerRequest(gCWRetransmitTimer, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentPacketTimer), CW_SOFT_TIMER_EXPIRED_SIGNAL,WTPIndex))) {
		return CW_FALSE;
	}
	CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);
	
	return CW_TRUE;
}


__inline__ CWBool CWACSendAcknowledgedPacket(int WTPIndex, int msgType, int seqNum) {
	gWTPs[WTPIndex].retransmissionCount = 0;
	gWTPs[WTPIndex].isRetransmitting = CW_TRUE;
	gWTPs[WTPIndex].responseType=msgType;
	gWTPs[WTPIndex].responseSeqNum=seqNum;
	//wid_syslog_debug_debug("~~~~~~seq num in Send: %d~~~~~~", gWTPs[WTPIndex].responseSeqNum);
	return CWACResendAcknowledgedPacket(WTPIndex);
}


void CWACStopRetransmission(int WTPIndex) {
	if(gWTPs[WTPIndex].isRetransmitting) {
		int i;
		wid_syslog_debug_debug(WID_WTPINFO,"Stop Retransmission");
		gWTPs[WTPIndex].isRetransmitting = CW_FALSE;
		CWThreadSetSignals(SIG_BLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);		
		if(!CWTimerCancel(&(gWTPs[WTPIndex].currentPacketTimer),1))
			{
			wid_syslog_err("Error Cancelling a Timer... possible error!");
			}
		CWThreadSetSignals(SIG_UNBLOCK, 1, CW_SOFT_TIMER_EXPIRED_SIGNAL);	
		gWTPs[WTPIndex].responseType=UNUSED_MSG_TYPE;
		gWTPs[WTPIndex].responseSeqNum=0;
		
		for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
			CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
		}

		CW_FREE_OBJECT(gWTPs[WTPIndex].messages);
		//wid_syslog_debug_debug("~~~~~~ End of Stop Retransmission");
	}
}

