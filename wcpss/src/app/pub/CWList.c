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
* CWList.c
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

// adds an element at the head of the list
// CW_TRUE if the operation is successful, CW_FALSE otherwise
CWBool CWAddElementToList(CWList *list, void *element) {
	CWListElement *newElem;
	
	if(element == NULL || list == NULL) return CW_FALSE;
	
	if((*list) == NULL) { // first element
		CW_CREATE_OBJECT_ERR((*list), CWListElement, return CW_FALSE;);
		(*list)->data = element;
		(*list)->next = NULL;
		return CW_TRUE;
	}
	
	CW_CREATE_OBJECT_ERR(newElem, CWListElement, return CW_FALSE;);
	newElem->data = element;
	newElem->next = (*list);
	
	(*list) = newElem;
	
	return CW_TRUE;
}

// adds an element at the tail of the list
// CW_TRUE if the operation is successful, CW_FALSE otherwise
CWBool CWAddElementToListTail(CWList *list, void *element) {
	CWListElement *newElem;
	
	if(element == NULL || list == NULL) return CW_FALSE;
	
	if((*list) == NULL) { // first element
		CW_CREATE_OBJECT_ERR((*list), CWListElement, return CW_FALSE;);
		(*list)->data = element;
		(*list)->next = NULL;
		return CW_TRUE;
	}
	
	newElem=*list;
	while (newElem->next != NULL)
	{
		newElem= newElem->next;
	}
	
	CW_CREATE_OBJECT_ERR(newElem->next, CWListElement, return CW_FALSE;);
	newElem->next->data = element;
	newElem->next->next = NULL;
	
	return CW_TRUE;
}

CWList CWListGetFirstElem(CWList *list){
	CWList auxList;

	if (list == NULL || *list == NULL) return NULL;
	
	auxList = *list;
	*list = (*list)->next;
	auxList->next = NULL;
	
	return auxList;
}

// not thread safe!
void * CWListGetNext(CWList list, CWListIterateMode mode) {
	static CWList l = NULL;
	void *data;
	
	if(list == NULL) return NULL;
	
	if(mode == CW_LIST_ITERATE_RESET) {
		l = list;
	} else if(l == NULL) return NULL;
	
	data = l->data;
	l = l->next;
	
	return data;
}

// search baseElement in list using compareFunc
// NULL if there was an error, the element otherwise
void *CWSearchInList(CWList list, void *baseElement, CWBool (*compareFunc) (void *, void*)) {
	CWListElement *el = NULL;
	
	if (baseElement == NULL || compareFunc == NULL) return NULL;
	
	for(el = list; el != NULL; el = el->next) {
		if(compareFunc(baseElement, el->data)) {
			return el->data;
		}
	}
	
	return NULL;
}

// search baseElement in list using compareFunc, removes the element from the list and returns it
// NULL if there was an error, the element otherwise
void *CWDeleteInList(CWList *list, void *baseElement, CWBool (*compareFunc) (void *, void*)) {
	CWListElement *el = NULL, *oldEl = NULL;
	
	if (baseElement == NULL || compareFunc == NULL || list == NULL) return NULL;
	
	for(el = (*list); el != NULL; oldEl = el, el = el->next) {
		if(compareFunc(baseElement, el->data)) {
			void *data = el->data;
			if(oldEl != NULL) {
				oldEl->next = el->next;
				
				CW_FREE_OBJECT(el);
				return data;
			} else { // first element
				(*list) = el->next;
				
				CW_FREE_OBJECT(el);
				return data;
			}

		}
	}
	
	return NULL;
}

// deletes a list, deleting each element with a call to the given deleteFunc
void CWDeleteList(CWList *list , void (*deleteFunc) (void *)) {
	CWListElement *el = NULL;
	
	if (list == NULL || (*list) == NULL || deleteFunc == NULL) return;

	do {
		el = (*list);
		(*list) = (*list)->next;
		deleteFunc(el->data);
		CW_FREE_OBJECT(el);
	} while((*list) != NULL);
}

int CWCountElementInList (CWList list)
{
	CWListElement *current;
	int count = 0;

	current=list;
	while (current!= NULL)
	{
		count++;
		current= current->next;
	}
	return count;
}

/*
CWList *FindLastElementInList (CWList list)
{
	CWListElement *current;
		
	current=list;
	while (current!= NULL) {current= current->next;}
	return &current;
}
*/


