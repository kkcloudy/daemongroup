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
*****************************************************************************/

#include <m3uaRcMgr.h>

#define M3_MAX_RTCTX M3_MAX_CONN * M3_MAX_R_AS

#define M3UA_MAX_RC_SUBLISTS    512

typedef struct __m3uaRCRangeNode
{
    m3_u8    used;
    m3_u32   beginRC;
    m3_u32   endRC;
    struct __m3uaRCRangeNode *pPrev;
    struct __m3uaRCRangeNode *pNext;
} m3uaRCRangeNode_t;

typedef struct __m3uaRCRangeList
{
    m3uaRCRangeNode_t  *pHead;
    m3uaRCRangeNode_t  *pTail;
    m3uaRCRangeNode_t  *pLastUsed;
} m3uaRCRangeList_t;

m3uaRCRangeList_t  m3uaRCRangeList;

m3_s32 m3uaRCMgrInit()
{
    int idx;
    m3uaRCRangeNode_t *pNode;

    pNode = (m3uaRCRangeNode_t*)malloc(sizeof(m3uaRCRangeNode_t));
    if (!pNode) {
        // set error code
        M3ECODE();
        return -1;
    }
    pNode->used = 1;
    pNode->beginRC = 0x00000001;
    pNode->endRC = 0xFFFFFFFE;
    pNode->pPrev = pNode->pNext = NULL;
    m3uaRCRangeList.pLastUsed = pNode;
    m3uaRCRangeList.pHead = pNode;
    m3uaRCRangeList.pTail = pNode;

    for (idx = 0; idx < M3UA_MAX_RC_SUBLISTS; idx++)
    {
        pNode = (m3uaRCRangeNode_t*)malloc(sizeof(m3uaRCRangeNode_t));
        if (!pNode) {
            // set error code
            M3ECODE();
            return -1;
        }
        pNode->used = 0;
        pNode->pPrev = m3uaRCRangeList.pTail;
        pNode->pNext = NULL;
        m3uaRCRangeList.pTail->pNext = pNode;
        m3uaRCRangeList.pTail = pNode;
    }
    return 0;
}

m3_u32 m3uaRCAllocate()
{
    m3_u32 rtCtx;
    if (NULL == m3uaRCRangeList.pLastUsed) {
        return 0xFFFFFFFF;
    }
    if (0 == (m3uaRCRangeList.pLastUsed->endRC - m3uaRCRangeList.pLastUsed->beginRC)) {
        rtCtx = m3uaRCRangeList.pLastUsed->endRC;
        m3uaRCRangeList.pLastUsed->used = 0;
        m3uaRCRangeList.pLastUsed = m3uaRCRangeList.pLastUsed->pPrev;
    } else {
        rtCtx = m3uaRCRangeList.pLastUsed->endRC;
        m3uaRCRangeList.pLastUsed->endRC--;
    }
    return m3uaRCRangeList.pLastUsed->endRC;
}

m3_u32 m3uaRCFree()
{
    
}

void m3uaRCMarkUsed()
{
}

void m3uaRCMarkFree()
{
}

