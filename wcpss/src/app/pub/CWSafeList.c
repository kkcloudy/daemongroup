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
* CWSafeList.c
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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWCreateSafeList(CWSafeList* pSafeList)
{
	CWPrivateSafeList* pNewList;

	if (pSafeList == NULL)
		return CW_FALSE;

	CW_CREATE_OBJECT_ERR(pNewList, CWPrivateSafeList, return CW_FALSE;);

	//
	pNewList->pThreadMutex = NULL;
	pNewList->pThreadCond = NULL;

	//
	pNewList->nCount = 0;
	pNewList->pFirstElement = NULL;
	pNewList->pLastElement = NULL;
	
	//
	(*pSafeList) = (CWSafeList)pNewList;
	return CW_TRUE;
}

void CWDestroySafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if (pList == NULL)
		return;

	//
	CW_FREE_OBJECT(pList);
}

void CWSetMutexSafeList(CWSafeList safeList, CWThreadMutex* pThreadMutex)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if (pList == NULL)
		return;

	pList->pThreadMutex = pThreadMutex;
}

void CWSetConditionSafeList(CWSafeList safeList, CWThreadCondition* pThreadCond)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if (pList == NULL)
		return;

	pList->pThreadCond = pThreadCond;
}

CWBool CWLockSafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if ((pList == NULL) || (pList->pThreadMutex == NULL))
		return CW_FALSE;

	//
	return CWThreadMutexLock(pList->pThreadMutex);
}

void CWUnlockSafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if ((pList == NULL) || (pList->pThreadMutex == NULL))
		return;

	//
	CWThreadMutexUnlock(pList->pThreadMutex);
}

CWBool CWWaitElementFromSafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if ((pList == NULL) || (pList->pThreadMutex == NULL) || (pList->pThreadCond == NULL))
		return CW_FALSE;

	return CWWaitThreadCondition(pList->pThreadCond, pList->pThreadMutex);
}

CWBool CWSignalElementSafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if ((pList == NULL) || (pList->pThreadCond == NULL))
		return CW_FALSE;

	CWSignalThreadCondition(pList->pThreadCond);
	return CW_TRUE;
}

unsigned long CWGetCountElementFromSafeList(CWSafeList safeList)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if (pList == NULL)
		return 0;

	return pList->nCount;
}

// No thread-safe
CWBool CWAddElementToSafeListHead(CWSafeList safeList, void* pData, int nSize)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;
	CWPrivateSafeElement* pNewElement;

	if ((pList == NULL) || (pData == NULL))
		return CW_FALSE;

	CW_CREATE_OBJECT_ERR(pNewElement, CWPrivateSafeElement, return CW_FALSE;);
	pNewElement->pData = pData;
	pNewElement->nSize = nSize;
	pNewElement->pNext = pList->pFirstElement;
	pNewElement->pPrev = NULL;
	if (pList->pFirstElement != NULL)
		pList->pFirstElement->pPrev = pNewElement;

	pList->pFirstElement = pNewElement;
	if (pList->pLastElement == NULL)
		pList->pLastElement = pNewElement;

	pList->nCount++;
	CWSignalElementSafeList(safeList);
	return CW_TRUE;
}

// No thread-safe
void* CWGetHeadElementFromSafeList(CWSafeList safeList, int* pSize)
{
	CWPrivateSafeElement* pElement;
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;

	if ((pList == NULL) || (pList->pFirstElement == NULL))
		return NULL;

	pElement = pList->pFirstElement;

	if (pSize != NULL)
		*pSize = pElement->nSize;

	return pElement->pData;
}

// No thread-safe
void* CWRemoveHeadElementFromSafeList(CWSafeList safeList, int* pSize)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;
	CWPrivateSafeElement* pElement;
	void* pData;

	if ((pList == NULL) || (pList->pFirstElement == NULL))
		return NULL;

	pElement = pList->pFirstElement;
	pList->pFirstElement = pList->pFirstElement->pNext;
	if (pList->pFirstElement == NULL)
		pList->pLastElement = NULL;
	else
		pList->pFirstElement->pPrev = NULL;
	
	pData = pElement->pData;
	if (pSize != NULL)
		*pSize = pElement->nSize;

	CW_FREE_OBJECT(pElement);

	pList->nCount--;
	return pData;
}

// No thread-safe
CWBool CWAddElementToSafeListTail(CWSafeList safeList, void* pData, int nSize)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;
	CWPrivateSafeElement* pNewElement;

	if ((pList == NULL) || (pData == NULL))
		return CW_FALSE;

	CW_CREATE_OBJECT_ERR(pNewElement, CWPrivateSafeElement, return CW_FALSE;);
	pNewElement->pData = pData;
	pNewElement->nSize = nSize;
	pNewElement->pNext = NULL;
	pNewElement->pPrev = pList->pLastElement;
	if (pList->pLastElement != NULL)
		pList->pLastElement->pNext = pNewElement;
	
	pList->pLastElement = pNewElement;
	if (pList->pFirstElement == NULL)
		pList->pFirstElement = pNewElement;

	pList->nCount++;
	CWSignalElementSafeList(safeList);
	return CW_TRUE;
}

// No thread-safe
void* CWRemoveTailElementFromSafeList(CWSafeList safeList, int* pSize)
{
	CWPrivateSafeList* pList = (CWPrivateSafeList*)safeList;
	CWPrivateSafeElement* pElement;
	void* pData;

	if ((pList == NULL) || (pList->pLastElement == NULL))
		return NULL;

	pElement = pList->pLastElement;
	pList->pLastElement = pList->pLastElement->pPrev;
	if (pList->pLastElement == NULL)
		pList->pFirstElement = NULL;
	else
		pList->pLastElement->pNext = NULL;
	
	pData = pElement->pData;
	if (pSize != NULL)
		*pSize = pElement->nSize;

	CW_FREE_OBJECT(pElement);

	pList->nCount--;
	return pData;
}

// No thread-safe
void CWCleanSafeList(CWSafeList safeList, void (*deleteFunc)(void *))
{
	void* pData;

	CW_REPEAT_FOREVER
	{
		pData = CWRemoveHeadElementFromSafeList(safeList, NULL);
		if (pData == NULL)
			break;

		if (deleteFunc != NULL)
			deleteFunc(pData);
	}
}
