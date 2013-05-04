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

#include <m3uaMemMgr.h>

static 	int 	num_buf_alloc = 0;
static m3MemPoolList_t  m3MemPoolsList;

void *M3malloc(unsigned int size)
{
    void *ptr = malloc(size);
    num_buf_alloc++;
    iu_log_debug("Number of Allocations = %d\n", num_buf_alloc);
    iu_log_debug("Size %u  of Memory allocated from %07x\n", size, (unsigned int)ptr);
    return ptr;
}

void M3free(void *ptr)
{
    num_buf_alloc--;
    iu_log_debug("Number of Allocations = %d\n", num_buf_alloc);
    iu_log_debug("Memory released from %07x\n", (unsigned int)ptr);
    return free(ptr);
}

int m3uaMemInit()
{
    memset((void*)&m3MemPoolsList, 0, sizeof(m3MemPoolsList));
    m3uaMemPoolCreate(M3_NUM_32BYTE_BUFS, 32, 0);
    m3uaMemPoolCreate(M3_NUM_64BYTE_BUFS, 64, 1);
    m3uaMemPoolCreate(M3_NUM_128BYTE_BUFS, 128, 2);
    m3uaMemPoolCreate(M3_NUM_256BYTE_BUFS, 256, 3);
    m3uaMemPoolCreate(M3_NUM_512BYTE_BUFS, 512, 4);
    m3uaMemPoolCreate(M3_NUM_1024BYTE_BUFS, 1024, 5);
    m3uaMemPoolCreate(M3_NUM_2048BYTE_BUFS, 2048, 6);
    m3uaMemPoolCreate(M3_NUM_4096BYTE_BUFS, 4096, 7);
    m3uaMemPoolCreate(M3_NUM_8192BYTE_BUFS, 8192, 8);
    return 1;
}

int m3uaMemPoolCreate(unsigned int nBufs, unsigned int bufSize, unsigned int poolId)
{
    m3MemPool_t *pPool;
    m3MemNode_t *pNode;
    int idx;

    if (m3MemPoolsList.nPools != M3_MAX_MEM_POOLS &&
      m3MemPoolsList.poolList[poolId].nBufs == 0) {
      pPool = &(m3MemPoolsList.poolList[poolId]);
      pPool->bufSize = bufSize;
      pPool->nBufs = nBufs;
    }
    else {
      M3ERRNO(EM3_MEMPOOL_UNAVAIL);
      return -1;
    }
    for (idx = 0; idx < nBufs; idx++) {
      pNode = (m3MemNode_t*)malloc(bufSize + sizeof(m3MemNode_t));
      if (NULL == pNode) {
        M3ERRNO(EM3_MEMINIT_FAIL);
        return -1;
      }
      pNode->hdr.poolId = poolId;
      pNode->hdr.alloc = 0;
      pNode->pPrev = NULL;
      pNode->pNext = NULL;
      m3uaMemPoolInsertBuf(pPool, pNode);
    }
    m3MemPoolsList.nPools++;
    return 0;
}

int m3uaMemPoolInsertBuf(m3MemPool_t *pPool, m3MemNode_t *pNode)
{
    if (pPool->nFree == 0) {
      pPool->pHead = pPool->pTail = pNode;
      pNode->pNext = pNode->pPrev = NULL;
    }
    else {
      pPool->pTail->pNext = pNode;
      pNode->pPrev = pPool->pTail;
      pNode->pNext = NULL;
      pPool->pTail = pNode;
    }
    pPool->nFree += 1;
    return 0;
}

m3MemNode_t* m3uaMemPoolDeleteBuf(m3MemPool_t *pPool)
{
    m3MemNode_t *pNode = NULL;

    if (pPool->pHead != NULL) {
      pNode = pPool->pHead;
      if (pPool->pHead->pNext != NULL) {
        pPool->pHead = pPool->pHead->pNext;
      }
      else {
        pPool->pTail = pPool->pHead = NULL;
      }
      pPool->nFree -= 1;
    }
    return pNode;
}

static int m3HighestBitMap[16] = 
        { 0, 1, 2, 2, 3, 3, 3, 3,
          4, 4, 4, 4, 4, 4, 4, 4 };

void* m3uaMemAlloc(unsigned int size)
{
    m3MemPool_t *pPool;
    unsigned int bufSize, poolId;
    unsigned char byte;
    unsigned char bit;
    m3MemNode_t *pNode = 0;
    unsigned char alloc = 0;

    if (8192 < size || 0 == size) {
      M3ERRNO(EM3_INV_MEMSIZE);
      return (void*)0;
    }
    if (size & 0xFF000000) {
      byte = (unsigned char)(size >> 24);
      if (0xF0 & byte) {
        bit = m3HighestBitMap[(byte >> 4)] + 4 + 24;
      }
      else {
        bit = m3HighestBitMap[byte] + 24;
      }
    }
    else if (size & 0x00FF0000) {
      byte = (unsigned char)(size >> 16);
      if (0xF0 & byte) {
        bit = m3HighestBitMap[(byte >> 4)] + 20;
      }
      else {
        bit = m3HighestBitMap[byte] + 16;
      }
    }
    else if (size & 0x0000FF00) {
      byte = (unsigned char)(size >> 8);
      if (0xF0 & byte) {
        bit = m3HighestBitMap[(byte >> 4)] + 12;
      }
      else {
        bit = m3HighestBitMap[byte] + 8;
      }
    }
    else {
      byte = (unsigned char)(size);
      if (0xF0 & byte) {
        bit = m3HighestBitMap[(byte >> 4)] + 4;
      }
      else {
        bit = m3HighestBitMap[byte];
      }
    }
    bufSize = 0x00000001 << bit;
    poolId = bit - 5;
    while (1 != alloc && poolId < M3_MAX_MEM_POOLS) {
      pPool = &(m3MemPoolsList.poolList[poolId]);
      if (pPool->nFree) {
        pNode = m3uaMemPoolDeleteBuf(pPool);
        pNode->hdr.alloc = 1;
        alloc = 1;
        pPool->totalUsage++;
      }
      else {
        pPool->totalFails++;
        poolId++;
      }
    }
    if (pNode)
      return (void *)((unsigned char*)pNode + sizeof(m3MemNode_t));
    return NULL;
}
 
void m3uaMemFree(void* pBuf)
{
    m3MemNode_t *pNode = (m3MemNode_t*)((unsigned char*)pBuf - sizeof(m3MemNode_t));
    if (0 == pNode->hdr.alloc) {
      iu_log_debug("WARNING: Freeing already freed/non-allocated buffer\n");
      return;
    }
    pNode->hdr.alloc = 0;
    m3MemPool_t *pPool = &(m3MemPoolsList.poolList[pNode->hdr.poolId]);
    m3uaMemPoolInsertBuf(pPool, pNode);
}

void m3uaMemMgrStats()
{
    unsigned int idx;
    for (idx = 0; idx < M3_MAX_MEM_POOLS; idx++) {
      iu_log_debug("----------------------\n");
      iu_log_debug("Pool id:		%u\n", idx);
      iu_log_debug("Number of Buffers:%u\n", m3MemPoolsList.poolList[idx].nBufs);
      iu_log_debug("Free Buffers:	%u\n", m3MemPoolsList.poolList[idx].nFree);
      iu_log_debug("Buffer Size:	%u\n", m3MemPoolsList.poolList[idx].bufSize);
      iu_log_debug("Total Allocations:%u\n", m3MemPoolsList.poolList[idx].totalUsage);
      iu_log_debug("Total Failures:	%u\n", m3MemPoolsList.poolList[idx].totalFails);
    }
    iu_log_debug("----------------------\n");
}

m3_u32 m3uaMemMgrDiag(m3_s8 *diagString, m3_u32 len)
{
    unsigned int idx;
    unsigned int strIdx = 0;

    if (256 >= len) {
        return (strlen(diagString));
    }

    strIdx += sprintf(&diagString[strIdx], "--MEMORY DIAGNOSTICS--\n");
    for (idx = 0; idx < m3MemPoolsList.nPools; idx++) {
      strIdx += sprintf(&diagString[strIdx], "----------------------\n");
      strIdx += sprintf(&diagString[strIdx], "Pool id:%u, Buffer Size:%u\n",
                        idx, m3MemPoolsList.poolList[idx].bufSize);
      strIdx += sprintf(&diagString[strIdx], "Number of Buffers:%u, Free Buffers:%u\n",
                        m3MemPoolsList.poolList[idx].nBufs, m3MemPoolsList.poolList[idx].nFree);
      strIdx += sprintf(&diagString[strIdx], "Total Allocations:%u, Total Failures:%u\n",
                        m3MemPoolsList.poolList[idx].totalUsage, m3MemPoolsList.poolList[idx].totalFails);
      if (256 >= (len - strIdx)) {
           return (strlen(diagString));
      }
    }
    strIdx += sprintf(&diagString[strIdx], "----------------------\n");
    return (strlen(diagString));
}


