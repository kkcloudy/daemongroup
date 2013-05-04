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
* CWList.h
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

 
#ifndef __CAPWAP_CWList_HEADER__
#define __CAPWAP_CWList_HEADER__

#define CW_LIST_INIT		NULL

typedef struct _s {
	void *data;
	struct _s *next;
} CWListElement;

typedef enum {
	CW_LIST_ITERATE_RESET,
	CW_LIST_ITERATE
} CWListIterateMode;

typedef CWListElement *CWList;

CWBool CWAddElementToList(CWList *list, void *element);
CWBool CWAddElementToListTail(CWList *list, void *element);
CWList CWListGetFirstElem(CWList *list);
void *CWListGetNext(CWList list, CWListIterateMode mode);
void *CWSearchInList(CWList list, void *baseElement, CWBool (*compareFunc)(void *, void *));
void *CWDeleteInList(CWList *list, void *baseElement, CWBool (*compareFunc)(void *, void *));
void CWDeleteList(CWList *list, void (*deleteFunc)(void *));
int CWCountElementInList(CWList list);
//CWList * FindLastElementInList (CWList list);

#endif
