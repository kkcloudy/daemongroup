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
* CWSafeList.h
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

 
#ifndef __CAPWAP_CWSafeList_HEADER__
#define __CAPWAP_CWSafeList_HEADER__

#include "CWThread.h"

typedef void* CWSafeList;

typedef struct _CWPrivateSafeElement
{
	void* pData;
	int nSize;
	struct _CWPrivateSafeElement* pPrev;
	struct _CWPrivateSafeElement* pNext;
} CWPrivateSafeElement;

typedef struct _CWPrivateSafeList
{
	CWThreadMutex* pThreadMutex;
	CWThreadCondition* pThreadCond;

	unsigned long nCount;
	CWPrivateSafeElement* pFirstElement;
	CWPrivateSafeElement* pLastElement;
} CWPrivateSafeList;

CWBool CWCreateSafeList(CWSafeList* pSafeList);
void CWDestroySafeList(CWSafeList safeList);

void CWSetMutexSafeList(CWSafeList safeList, CWThreadMutex* pThreadMutex);
void CWSetConditionSafeList(CWSafeList safeList, CWThreadCondition* pThreadCond);

CWBool CWLockSafeList(CWSafeList safeList);
void CWUnlockSafeList(CWSafeList safeList);
CWBool CWWaitElementFromSafeList(CWSafeList safeList);
CWBool CWSignalElementSafeList(CWSafeList safeList);

unsigned long CWGetCountElementFromSafeList(CWSafeList safeList);
CWBool CWAddElementToSafeListHead(CWSafeList safeList, void* pData, int nSize);
void* CWGetHeadElementFromSafeList(CWSafeList safeList, int* pSize);
void* CWRemoveHeadElementFromSafeList(CWSafeList safeList, int* pSize);
CWBool CWAddElementToSafeListTail(CWSafeList safeList, void* pData, int nSize);
void* CWRemoveTailElementFromSafeList(CWSafeList safeList, int* pSize);
void CWCleanSafeList(CWSafeList safeList, void (*deleteFunc)(void *));

#endif
 
