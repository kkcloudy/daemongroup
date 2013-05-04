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

#ifndef __M3uaMemMgr_H__
#define __M3uaMemMgr_H__

#include <m3ua_sys_inc.h>
#include <m3ua_defines.h>
#include <m3ua_errno.h>

#define M3_MAX_MEM_POOLS    16
#define M3_MIN_BUFSIZE      32
#define M3_MAX_BUFSIZE      8192


#define M3_MSG_ALLOC(size, pool)	m3uaMemAlloc(size)
#define M3_MSG_FREE(pbuf, pool)		m3uaMemFree(pbuf)

#define M3_MALLOC(size)			malloc(size)
#define M3_FREE(pbuf)			free(pbuf)

#define M3_MEMCPY(dst, src, size)	memcpy(dst, src, size)

typedef struct __m3MemHeader {
    unsigned int poolId;
    unsigned int alloc;
} m3MemHeader_t;

typedef struct __m3MemNode_t {
    m3MemHeader_t hdr;
    struct __m3MemNode_t *pPrev;
    struct __m3MemNode_t *pNext;
} m3MemNode_t;

typedef struct __m3MemPool {
    unsigned int nBufs;
    unsigned int nFree;
    unsigned int bufSize;
    unsigned int totalUsage;
    unsigned int totalFails;
    m3MemNode_t *pHead;
    m3MemNode_t *pTail;
} m3MemPool_t;

typedef struct __m3MemPoolList {
    int nPools;
    m3MemPool_t poolList[M3_MAX_MEM_POOLS];
} m3MemPoolList_t;


void *M3malloc(unsigned int);
void M3free(void*);
int m3uaMemInit(void);
int m3uaMemPoolCreate(unsigned int, unsigned int, unsigned int);
int m3uaMemPoolInsertBuf(m3MemPool_t*, m3MemNode_t*);
m3MemNode_t* m3uaMemPoolDeleteBuf(m3MemPool_t*);
void* m3uaMemAlloc(unsigned int);
void m3uaMemFree(void*);
void m3uaMemMgrStats(void);
m3_u32 m3uaMemMgrDiag(m3_s8 *, m3_u32);

#endif

