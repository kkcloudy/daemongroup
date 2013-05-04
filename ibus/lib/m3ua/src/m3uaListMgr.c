/****************************************************************************
** Description:
*****************************************************************************
** Copyright(C) 2009 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
*****************************************************************************
** Contact:
** vkgupta@shabdcom.org
*****************************************************************************
** License :
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****************************************************************************/


/***************************************************************************
 * Create a List Container, with free Nodes and used Nodes count
 * Delete a List Container
 * When a Node is inserted, take a Node from Free List and put it in the
 * used list.
 * When a Node is deleted, take it out from used list and add it back to
 * the free list.
 * I/f to insert a Node before/After a certain Node
 * I/f to get Head/Tail of a List
 * I/f to delete a Node before/After a certain Node
 *
 *
 *
 *
 *
 * ***************************************************************************/

#include <m3ua_mem.h>

typedef struct __m3LinkedListNode_t {
    void *pInf;
    struct __m3LinkedListNode_t *pNext;
    struct __m3LinkedListNode_t *pPrev;
} m3LinkedListNode_t;

typedef struct {
    unsigned int nNodes;
    m3LinkedListNode_t  *pHead;
    m3LinkedListNode_t  *pTail;
} m3LinkedList_t;

int m3uaLinkedListCreate()
{
}

int m3uaLinkedListDelete()
{
}

int m3LinkedListInsert(m3LinkedList_t *pList, void *pUserInf)
{
}

void* m3LinkedListDelete(m3LinkedList_t *pList)
{
    m3LinkedListNode_t *pNode = NULL;

    if (pList->pHead != NULL) {
      pNode = pList->pHead;
      if (pList->pHead->pNext != NULL) {
        pList->pTail = pList->pTail->pPrev;
        pList->pTail->pNext = NULL;
      }
      else {
        pList->pTail = pList->pHead = NULL;
      }
    }
    pList->nNodes -= 1;
    return pNode;
}

int m3LinkedListEmpty(m3LinkedList_t *pList)
{
    return ((pList->nNodes == 0)?1:0);
}

