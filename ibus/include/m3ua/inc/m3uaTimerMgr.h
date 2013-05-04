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

#ifndef __M3UA_TIMER_H__
#define __M3UA_TIMER_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_errno.h>
#include <m3uaTraceMgr.h>
#include <m3ua_co_proc.h>
#include <m3ua_sys_inc.h>

typedef struct
{
    unsigned long   uNumTicks;
    unsigned long   lNumTicks;
} m3Clock_t;

typedef struct __m3TimerNode
{
    m3_u32            timerId;
    m3_u32            dur;
    m3Clock_t         startTimeStamp;
    m3Clock_t         stopTimeStamp;
    void              *pUserInf;
    struct __m3TimerNode  *pPrev;
    struct __m3TimerNode  *pNext;
} m3TimerNode_t;

typedef struct
{
   m3TimerNode_t    *pHead;
   m3TimerNode_t    *pTail;
   m3_u32           nNodes;
} m3TimerList_t;

m3_s32 m3uaTimerInit(void);
m3_s32 m3uaStartTimer(m3_u32, void *, m3_u32 *);
m3_s32 m3uaStopTimer(m3_u32, void **);
m3_s32 m3uaGetTimerBuf(m3_u32, void **);
m3_u32 m3uaTimerMgrDiag(m3_s8 *, m3_u32);

#endif

