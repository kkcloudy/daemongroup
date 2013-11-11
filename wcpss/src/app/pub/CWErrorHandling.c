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
* CWErrorHandling.c
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
#include "CWAC.h"


#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#ifndef CW_SINGLE_THREAD
	CWThreadSpecific gLastError;
	//CWThreadOnce gInitLastErrorOnce = CW_THREAD_ONCE_INIT;
#else
	static CWErrorHandlingInfo *gLastErrorDataPtr;
#endif

extern int wid_memory_trace_switch;

void CWErrorHandlingInitLib() {	
	
	wid_syslog_debug_debug(WID_DEFAULT,"Init Error");
	#ifndef CW_SINGLE_THREAD
		if(!CWThreadCreateSpecific(&gLastError, NULL)) 
		{
			//CWLog("Critical Error, closing the process..."); 
			wid_syslog_crit("Critical Error,closing the process...");
			exit(1);
		}
	#else
		CW_CREATE_OBJECT_ERR_WID(infoPtr, CWErrorHandlingInfo, return;);
		infoPtr->code = CW_ERROR_NONE;
		gLastErrorDataPtr = infoPtr;
	#endif
}

CWBool _CWErrorRaise(CWErrorCode code, const char *msg, const char *fileName, int line) {
	CWErrorHandlingInfo *infoPtr;
	#ifndef CW_SINGLE_THREAD
		infoPtr = CWThreadGetSpecific(&gLastError);
		if(infoPtr==NULL){
			CW_CREATE_OBJECT_ERR_WID(infoPtr, CWErrorHandlingInfo, exit(1););
			infoPtr->code = CW_ERROR_NONE;
			if(!CWThreadSetSpecific(&gLastError, infoPtr))
			{
				//CWLog("Critical Error, closing the process...");
				wid_syslog_crit("Critical Error, closing the process...");
				if(g_wid_wsm_error_handle_state){

					wid_syslog_crit("free infoPtr.....\n");

		            CW_FREE_OBJECT_WID(infoPtr);
					return CW_FALSE;
				}
				//exit(1);
			}
		}
	#else
		infoPtr = gLastErrorDataPtr;
	#endif
	
	if(infoPtr == NULL) 
	{
		//CWLog("Critical Error: something strange has happened, closing the process..."); 
		wid_syslog_crit("Critical Error: something strange has happened, closing the process...");
		//exit(1);
	}
	
	infoPtr->code = code;
	if(msg != NULL) strcpy(infoPtr->message, msg);
	else infoPtr->message[0]='\0';
	if(fileName != NULL) strcpy(infoPtr->fileName, fileName);
	infoPtr->line = line;
	
	return CW_FALSE;
}

void CWErrorPrint(CWErrorHandlingInfo *infoPtr, const char *desc, const char *fileName, int line) {
	if(infoPtr == NULL) return;
	if(/*infoPtr->message != NULL && */infoPtr->message[0]!='\0') {
		//CWLog("Error:%s. %s .", desc, infoPtr->message);
		wid_syslog_debug_debug(WID_DEFAULT,"Error:%s. %s .", desc, infoPtr->message);
	} else {
		//CWLog("Error:%s", desc);
		wid_syslog_debug_debug(WID_DEFAULT,"Error:%s", desc);
	}
	//CWLog("(occurred at line %d in file %s, catched at line %d in file %s).",infoPtr->line, infoPtr->fileName, line, fileName);
	wid_syslog_debug_debug(WID_DEFAULT,"(occurred at line %d in file %s, catched at line %d in file %s).",infoPtr->line, infoPtr->fileName, line, fileName);
}

CWErrorCode CWErrorGetLastErrorCode() {
	CWErrorHandlingInfo *infoPtr;
	
	#ifndef CW_SINGLE_THREAD
		infoPtr = CWThreadGetSpecific(&gLastError);
	#else
		infoPtr = gLastErrorDataPtr;
	#endif
	
	if(infoPtr == NULL) return CW_ERROR_GENERAL;
	
	return infoPtr->code;
}

CWBool _CWErrorHandleLast(const char *fileName, int line) {
	CWErrorHandlingInfo *infoPtr;
	
	#ifndef CW_SINGLE_THREAD
		infoPtr = CWThreadGetSpecific(&gLastError);
	#else
		infoPtr = gLastErrorDataPtr;
	#endif
	
	if(infoPtr == NULL) {
		//CWLog("No Error Pending");
		//exit((3));
		return CW_FALSE;
	}
	
	#define __CW_ERROR_PRINT(str)	CWErrorPrint(infoPtr, (str), fileName, line)
	
	switch(infoPtr->code) {
		case CW_ERROR_SUCCESS:
		case CW_ERROR_NONE:
			return CW_TRUE;
			break;
			
		case CW_ERROR_OUT_OF_MEMORY:
			__CW_ERROR_PRINT("Out of Memory");
			#ifndef CW_SINGLE_THREAD
				CWExitThread(); // note: we can manage this on per-thread basis: ex. we can
								// kill some other thread if we are a manager thread.
			#else
				exit(1);
			#endif
			break;
			
		case CW_ERROR_WRONG_ARG:
			__CW_ERROR_PRINT("Wrong Arguments in Function");
			break;
			
		case CW_ERROR_NEED_RESOURCE:
			__CW_ERROR_PRINT("Missing Resource");
			break;
			
		case CW_ERROR_GENERAL:
			__CW_ERROR_PRINT("Error Occurred");
			break;
		
		case CW_ERROR_CREATING:
			__CW_ERROR_PRINT("Error Creating Resource");
			break;
			
		case CW_ERROR_SENDING:
			__CW_ERROR_PRINT("Error Sending");
			break;
		
		case CW_ERROR_RECEIVING:
			__CW_ERROR_PRINT("Error Receiving");
			break;
			
		case CW_ERROR_INVALID_FORMAT:
			__CW_ERROR_PRINT("Invalid Format");
			break;
				
		case CW_ERROR_INTERRUPTED:
		default:
			break;
	}
	
	return CW_FALSE;
}
