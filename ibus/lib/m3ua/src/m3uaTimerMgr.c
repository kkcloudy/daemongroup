/****************************************************************************
** Description:
** This file contains functions to start/stop timer.
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

#include <m3uaTimerMgr.h>

static m3TimerList_t    m3timer_freelist;
static m3TimerList_t    m3timer_activelist;
static m3Clock_t        m3clock;
static m3TimerNode_t*   m3timerNodeMap[M3_MAX_TIMERS];

void m3uaTimerListNodeInsert(m3TimerList_t *pList, m3TimerNode_t *pNode)
{
    M3TRACE(m3uaTimerTrace, ("Inserting Node: %x in List: %x", (unsigned int)pNode, (unsigned int)pList));
    if (pList->nNodes == 0) {
      pList->pHead = pList->pTail = pNode;
      pNode->pNext = pNode->pPrev = NULL;
    }
    else {
      pList->pTail->pNext = pNode;
      pNode->pPrev = pList->pTail;
      pNode->pNext = NULL;
      pList->pTail = pNode;
    }
    pList->nNodes += 1;
}

m3_s32 m3uaTimerInit(void)
{
    m3_u32    idx;
    m3TimerNode_t  *pNode;

    m3timer_freelist.nNodes = 0;
    m3timer_freelist.pHead = NULL;
    m3timer_freelist.pTail = NULL;
    M3TRACE(m3uaTimerTrace, ("Free List: %x", (unsigned int)&m3timer_freelist));

    m3timer_activelist.nNodes = 0;
    m3timer_activelist.pHead = NULL;
    m3timer_activelist.pTail = NULL;
    M3TRACE(m3uaTimerTrace, ("Active List: %x", (unsigned int)&m3timer_activelist));

    M3TRACE(m3uaTimerTrace, ("Initializing M3UA-Timer-Lib with %u timers", M3_MAX_TIMERS));
    for (idx = 0; idx < M3_MAX_TIMERS; idx++) {
        pNode = (m3TimerNode_t*)malloc(sizeof(m3TimerNode_t));
        if (NULL == pNode) {
            M3ERRNO(EM3_TIMERINIT_FAIL);
            return -1;
        }
        m3timerNodeMap[idx] = pNode;
        pNode->timerId = idx;
        pNode->dur = 0;
        pNode->startTimeStamp.uNumTicks = pNode->startTimeStamp.lNumTicks = 0;
        pNode->stopTimeStamp.uNumTicks = pNode->stopTimeStamp.lNumTicks = 0;
        pNode->pUserInf = NULL;
        pNode->pPrev = pNode->pNext = NULL;
        m3uaTimerListNodeInsert(&m3timer_freelist, pNode);
    }
    return 0;
}

//NOV2009
m3TimerNode_t* m3uaTimerListSpecificNodeDelete(m3TimerList_t *pList, m3TimerNode_t *pNode)
{
    if (NULL == pNode->pPrev) { // head node
        pList->pHead = pNode->pNext;
        if (pNode->pNext) { pNode->pNext->pPrev = NULL; }
        else { pList->pTail = NULL; }
    } else if (NULL == pNode->pNext) { // tail Node
        pList->pTail = pNode->pPrev;
        if (pNode->pPrev) { pNode->pPrev->pNext = NULL; }
        else { pList->pHead = NULL; }
    } else {
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }
    pList->nNodes--;
    return pNode;
}

m3TimerNode_t* m3uaTimerListNodeDelete(m3TimerList_t *pList)
{
    m3TimerNode_t *pNode = NULL;

    M3TRACE(m3uaTimerTrace, ("Deleting Head Node: %x in List: %x", (unsigned int)pList->pHead, (unsigned int)pList));
    if (pList->pHead != NULL) {
      pNode = pList->pHead;
      if (pList->pHead->pNext != NULL) {
        pList->pHead = pList->pHead->pNext;
        pList->pHead->pPrev = NULL;
      }
      else {
        pList->pTail = pList->pHead = NULL;
      }
      pList->nNodes -= 1;
      M3TRACE(m3uaTimerTrace, ("Node Deleted, No. of nodes in TimerList:%u", pList->nNodes));
      pNode->pNext = NULL;
      pNode->pPrev = NULL;
    }
    return pNode;
}

void m3uaTimerListInsertNodeAfter(m3TimerList_t *pList, m3TimerNode_t *pNode, m3TimerNode_t *pPrevNode)
{
    if (NULL == pPrevNode->pNext) { // Tail node
        pPrevNode->pNext = pNode;
        pNode->pNext = NULL;
        pNode->pPrev = pPrevNode;
        pList->pTail = pNode;
    } else {
        pNode->pNext = pPrevNode->pNext;
        pNode->pPrev = pPrevNode;
        pPrevNode->pNext = pNode;
        pNode->pNext->pPrev = pNode;
    }
    pList->nNodes++;
}

// return 1 if subject > reference
//        0 if ==
//        -1 if subject < reference
int m3uaTimerCompareTimeStamps(m3Clock_t subTimeStamp, m3Clock_t refTimeStamp)
{
    if (subTimeStamp.uNumTicks > refTimeStamp.uNumTicks) return 1;
    if (subTimeStamp.uNumTicks < refTimeStamp.uNumTicks) return -1;
    if (subTimeStamp.lNumTicks > refTimeStamp.lNumTicks) return 1;
    if (subTimeStamp.lNumTicks < refTimeStamp.lNumTicks) return -1;
    return 0;
}

void m3uaTimerNodeInsertActiveList(m3TimerList_t *pList, m3TimerNode_t *pNode)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    if((pList == NULL) || (pNode == NULL)){
        iu_log_debug("Error: Parameter is NULL! Please check the cause.\n");
        return;
    }
    printf("111\n");
    m3TimerNode_t  *pPrevNode = pList->pTail;

    while (pPrevNode) {
        if (-1 != m3uaTimerCompareTimeStamps(pNode->stopTimeStamp, pPrevNode->stopTimeStamp)) {
            m3uaTimerListInsertNodeAfter(pList, pNode, pPrevNode);
            return;
        }
        pPrevNode = pPrevNode->pPrev;
    }
    printf("222\n");
    if (0 == pList->nNodes) {
        pList->pHead = pList->pTail = pNode;
        pList->nNodes++;
    } 
    else {
        pNode->pNext = pList->pHead;
        pNode->pPrev = NULL;
        printf("333\n");
        pList->pHead->pPrev = pNode;
        printf("444\n");
        pList->pHead = pNode;
        pList->nNodes++;
    }
}

//NOV2009
int m3uaTimerTotalNumNodes(m3TimerList_t *pList)
{
    return pList->nNodes;
}

//NOV2009
m3TimerNode_t* m3uaTimerGetListHead(m3TimerList_t *pList)
{
    return pList->pHead;
}

void m3uaTimerCalStopTime(m3Clock_t *pStopTimeStamp, m3_u32 interval)
{
   m3_u32     max = ~(0);

   pStopTimeStamp->uNumTicks = m3clock.uNumTicks;
   if (interval > max - m3clock.lNumTicks) {
       pStopTimeStamp->uNumTicks++;
   }
   pStopTimeStamp->lNumTicks = m3clock.lNumTicks + interval;
   return;
}

m3_s32 m3uaStartTimer(m3_u32          interval,
                      void            *pUserInf,
                      m3_u32          *pTimerId)
{
    m3TimerNode_t  *pNode;

/************ //NOV2009
    if (0 == m3timer_freelist.nNodes) {
*************/
    //NOV2009
    if (0 == m3uaTimerTotalNumNodes(&(m3timer_freelist))) {
        M3TRACE(m3uaErrorTrace, ("Failed to start timer"));
        M3ERRNO(EM3_INSUFF_TIMERS);
        return -1;
    }
    pNode = m3uaTimerListNodeDelete(&m3timer_freelist);
    *pTimerId = pNode->timerId;
    pNode->dur = interval;
    pNode->startTimeStamp = m3clock;
    m3uaTimerCalStopTime(&(pNode->stopTimeStamp), interval);
    M3TRACE(m3uaTimerTrace, ("Stop Time: UNumTicks-%lu, LNumTicks-%lu",pNode->stopTimeStamp.uNumTicks, pNode->stopTimeStamp.lNumTicks));
    pNode->pUserInf = pUserInf;
    M3TRACE(m3uaTimerTrace, ("Start Timer: TimerId:%u, Duration:%u, pUserInf:%x", \
            *pTimerId, interval, (int)pUserInf));
    m3uaTimerNodeInsertActiveList(&m3timer_activelist, pNode);
/********** //NOV2009
    M3TRACE(m3uaTimerTrace, ("No. of Active Timers:%u", m3timer_activelist.nNodes));
***********/
    M3TRACE(m3uaTimerTrace, ("No. of Active Timers:%u", m3uaTimerTotalNumNodes(&(m3timer_activelist)))); //NOV2009
    return 0;
}

m3_s32 m3uaStopTimer(m3_u32    timerId,
                     void      **ppbuf)
{
    m3TimerNode_t  *pNode;

    M3TRACE(m3uaTimerTrace, ("Stop Timer: TimerId:%u", timerId));
    if (M3_MAX_TIMERS <= timerId) {
        M3ERRNO(EM3_INV_TIMERID);
        return -1;
    }
    pNode = m3timerNodeMap[timerId];
 
    //NOV2009
    m3uaTimerListSpecificNodeDelete(&(m3timer_activelist), pNode);

/********** //NOV2009
    if (NULL == pNode->pPrev) { // head node
        m3timer_activelist.pHead = pNode->pNext;
        if (pNode->pNext) { pNode->pNext->pPrev = NULL; }
        else { m3timer_activelist.pTail = NULL; }
    } else if (NULL == pNode->pNext) { // tail Node
        m3timer_activelist.pTail = pNode->pPrev;
        if (pNode->pPrev) { pNode->pPrev->pNext = NULL; }
        else { m3timer_activelist.pHead = NULL; }
    } else {
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;
    }
***********/

    *ppbuf = pNode->pUserInf;
    pNode->pUserInf = NULL;
/********** //NOV2009
    m3timer_activelist.nNodes--;
***********/
    m3uaTimerListNodeInsert(&m3timer_freelist, pNode);
/********** //NOV2009
    M3TRACE(m3uaTimerTrace, ("No. of Active Timers:%u", m3timer_activelist.nNodes));
***********/
    M3TRACE(m3uaTimerTrace, ("No. of Active Timers:%u", m3uaTimerTotalNumNodes(&(m3timer_activelist)))); //NOV2009
    return 0;
}

m3_s32 m3uaGetTimerBuf(m3_u32    timerId,
                       void      **ppbuf)
{
    m3TimerNode_t  *pNode;

    if (M3_MAX_TIMERS <= timerId) {
        M3ERRNO(EM3_INV_TIMERID);
        return -1;
    }
    pNode = m3timerNodeMap[timerId];
    *ppbuf = pNode->pUserInf;
    return 0;
}

void m3uaTimerIncrementTick(void)
{
    m3clock.lNumTicks++;
    if (0 == m3clock.lNumTicks) {
        m3clock.uNumTicks++;
    }
}

void* m3timer_ckexpiry(void *ptr)
{
    m3TimerNode_t *pNode = NULL, *pHead = NULL;
    m3_u32        timerId;
    void          *pUserInf = NULL;

    m3uaTimerIncrementTick();
/*********** //NOV2009
    pNode = m3timer_activelist.pHead;
************/
    pNode = m3uaTimerGetListHead(&(m3timer_activelist)); //NOV2009
    while (pNode) {
        if (0 == m3uaTimerCompareTimeStamps(pNode->stopTimeStamp, m3clock)) {
            pHead = m3uaTimerListNodeDelete(&(m3timer_activelist));
            M3TRACE(m3uaTimerTrace, ("Timer Expired. Timer Id:%u, No. of Active Timers:%u", \
                    pNode->timerId, m3uaTimerTotalNumNodes(&(m3timer_activelist))));
            timerId = pHead->timerId;
            pUserInf = pHead->pUserInf;
            m3uaTimerListNodeInsert(&m3timer_freelist, pHead);
            M3TRACE(m3uaTimerTrace, ("No. of Free Timers:%u", m3timer_freelist.nNodes));
            m3uaTimerExpiry(timerId, pUserInf);
/******** //NOV2009
            pNode = m3timer_activelist.pHead;
*********/
            pNode = m3uaTimerGetListHead(&(m3timer_activelist)); //NOV2009
            continue;
        }
        break;
    }
    return M3_NULL;
}

m3_u32 m3uaTimerMgrDiag(m3_s8 *diagString, m3_u32 len)
{
    unsigned int strIdx = 0;

    if (256 >= len) {
        return (strlen(diagString));
    }

    strIdx += sprintf(&diagString[strIdx], "--TIMER  DIAGNOSTICS--\n");
    strIdx += sprintf(&diagString[strIdx], "--ACTIVE ---- TIMERS--\n");
    strIdx += sprintf(&diagString[strIdx], "Number of Active Timers:%u\n", m3timer_activelist.nNodes);
    strIdx += sprintf(&diagString[strIdx], "--FREE ------ TIMERS--\n");
    strIdx += sprintf(&diagString[strIdx], "Number of Free Timers:%u\n", m3timer_freelist.nNodes);
    strIdx += sprintf(&diagString[strIdx], "----------------------\n");
    return (strlen(diagString));
}

