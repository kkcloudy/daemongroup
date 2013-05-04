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


#ifndef __CAPWAP_CWErrorHandling_HEADER__
#define __CAPWAP_CWErrorHandling_HEADER__

typedef enum {
	CW_ERROR_SUCCESS = 1,
	CW_ERROR_OUT_OF_MEMORY,
	CW_ERROR_WRONG_ARG,
	CW_ERROR_INTERRUPTED,
	CW_ERROR_NEED_RESOURCE,
	CW_ERROR_COMUNICATING,
	CW_ERROR_CREATING,
	CW_ERROR_GENERAL,
	CW_ERROR_OPERATION_ABORTED,
	CW_ERROR_SENDING,
	CW_ERROR_RECEIVING,
	CW_ERROR_INVALID_FORMAT,
	CW_ERROR_TIME_EXPIRED,
	CW_ERROR_NONE
} CWErrorCode;


typedef struct {
	CWErrorCode code;
	char message[256];
	int line;
	char fileName[64];
} CWErrorHandlingInfo;

#define CWErrorRaiseSystemError(error)		{					\
							char buf[256];			\
							strerror_r(errno, buf, 256);	\
							CWErrorRaise(error, buf);	\
							return CW_FALSE;		\
						}
											
#define CWErrorRaise(code, msg) 		_CWErrorRaise(code, msg, __FILE__, __LINE__)
#define CWErr(arg)				((arg) || _CWErrorHandleLast(__FILE__, __LINE__))
#define CWErrorHandleLast()			_CWErrorHandleLast(__FILE__, __LINE__)

CWBool _CWErrorRaise(CWErrorCode code, const char *msg, const char *fileName, int line);
void CWErrorPrint(CWErrorHandlingInfo *infoPtr, const char *desc, const char *fileName, int line);
CWErrorCode CWErrorGetLastErrorCode(void);
CWBool _CWErrorHandleLast(const char *fileName, int line);

#endif
