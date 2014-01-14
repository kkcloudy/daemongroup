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
* CWCommon.h
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

 
#ifndef __CAPWAP_CWCommon_HEADER__
#define __CAPWAP_CWCommon_HEADER__


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "wireless_copy.h"

// make sure the types really have the right sizes
#define CW_COMPILE_TIME_ASSERT(name, x)               typedef int CWDummy_ ## name[(x) * 2 - 1]

// if you get a compile error, change types (NOT VALUES!) according to your system
CW_COMPILE_TIME_ASSERT(int_size, sizeof(int) == 4);
CW_COMPILE_TIME_ASSERT(char_size, sizeof(char) == 1);

#define		CW_BUFFER_SIZE					65536
#define		CW_ZERO_MEMORY					bzero
#define		CW_COPY_MEMORY(dst, src, len)			bcopy(src, dst, len)
#define		CW_REPEAT_FOREVER				while(1)

#define DEFAULT_LOG_SIZE					1000000

typedef enum {
	CW_FALSE = 0,
	CW_TRUE = 1,
	CW_RETURN_DIRECTLY = 2
} CWBool;

enum {
	IMAGE_DATA_USELESS_STATE,
	IMAGE_DATA_CHECKING_START,
	IMAGE_DATA_CHECKING_DONE,
	IMAGE_DATA_DOWNLOAD_START,
	IMAGE_DATA_DOWNLOAD_DONE,
	IMAGE_DATA_CRC_START,
	IMAGE_DATA_CRC_DONE,
	IMAGE_DATA_SAVE_START
};

enum {
	WTP_UPGRADE_RESULT_FAILURE_SUCCESS,
	WTP_UPGRADE_RESULT_FAILURE_UNDEFINED_ERR,
	WTP_UPGRADE_RESULT_FAILURE_NO_VERSION_FILE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_VERSION_FILE_SIZE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_EXTRACT_VERSION_FILE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_EXTRACT_INTERNAL_VERSION_FILE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_KERNEL_ROOTFS_CRC_CHECK_ERR,
	WTP_UPGRADE_RESULT_FAILURE_SPECIFIC_VERSION_FILE_CHECK_ERR,
	WTP_UPGRADE_RESULT_FAILURE_KERNEL_FILE_SIZE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_ROOTFS_FILE_SIZE_ERR,
	WTP_UPGRADE_RESULT_FAILURE_KERNEL_MD5_CHECK_ERR,
	WTP_UPGRADE_RESULT_FAILURE_ROOTFS_MD5_CHECK_ERR
};

typedef enum {
	CW_ENTER_SULKING,
	CW_ENTER_DISCOVERY,
	CW_ENTER_JOIN,
	CW_ENTER_CONFIGURE,
	CW_ENTER_DATA_CHECK,
	CW_ENTER_RUN,
	CW_ENTER_RESET,
	CW_QUIT,
	CW_ENTER_IMAGE_DATA,
	CW_BAK_RUN
} CWStateTransition;

extern const char *CW_CONFIG_FILE;
extern int gCWForceMTU;
extern int gCWRetransmitTimer;
extern int gCWNeighborDeadInterval;
extern int gCWMaxRetransmit; 
extern int gMaxLogFileSize;
extern int gEnabledLog;

//#define		CW_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
//#define		CW_FREE_OBJECTS_ARRAY(ar_name, ar_size)			{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }
//#define		CW_PRINT_STRING_ARRAY(ar_name, ar_size)			{int i = 0; for(i = 0; i < (ar_size); i++) printf("[%d]: **%s**\n", i, ar_name[i]);}

// custom error
//#define		CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
//#define		CW_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}
//#define		CW_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
//#define		CW_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
//#define		CW_CREATE_STRING_FROM_STRING_ERR(str_name, str, on_err)	{CW_CREATE_STRING_ERR(str_name, strlen(str), on_err); strcpy((str_name), str);}

#ifdef CW_DEBUGGING
	#define		CW_CREATE_ARRAY_ERR2(ar_name, ar_size, ar_type, on_err)		{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if((ar_name)) {on_err}}
	#define		CW_CREATE_OBJECT_ERR2(obj_name, obj_type, on_err)		{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if((obj_name)) {on_err}}
	#define		CW_CREATE_OBJECT_SIZE_ERR2(obj_name, obj_size,on_err)		{obj_name = (malloc(obj_size)); if((obj_name)) {on_err}}
	#define		CW_CREATE_STRING_ERR2(str_name, str_length, on_err)		{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if((str_name)) {on_err}}
	#define		CW_CREATE_STRING_FROM_STRING_ERR2(str_name, str, on_err)	{CW_CREATE_STRING_ERR2(str_name, strlen(str), on_err); strcpy((str_name), str);}
#endif

#include "CWStevens.h"
#include "config.h"
#include "CWLog.h"
#include "CWErrorHandling.h"

#include "CWRandom.h"
//#include "CWTimer.h"
#include "timerlib.h"
#include "CWThread.h"
#include "CWNetwork.h"
#include "CWList.h"
#include "CWSafeList.h"

#include "CWProtocol.h"
#include "CWSecurity.h"
#include "CWConfigFile.h"

int CWTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y);
CWBool CWParseSettingsFile();
void CWErrorHandlingInitLib();

extern CWThreadMutex gCreateIDMutex;

#endif
