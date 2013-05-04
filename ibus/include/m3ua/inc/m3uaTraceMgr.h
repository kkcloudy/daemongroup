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

#ifndef __M3TraceMgr_H__
#define __M3TraceMgr_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_errno.h>
#include <m3ua_sys_inc.h>

extern unsigned int m3uaTraceMap;

//static unsigned char m3uaTraceString[1024];

#define M3TRACE(trcType, trace) \
    if (trcType & m3uaTraceMap) { printf("[M3UA@%s:%d]", __FILE__, __LINE__); printf trace ; printf("\n"); fflush(NULL);}

#define M3HEX(trcType, pBuf, bufLen)                \
    if (trcType & m3uaTraceMap) {                   \
        printf("[M3UA@%s:%d]", __FILE__, __LINE__); \
        printf(" Number of bytes: %u\n", bufLen);   \
        {                                           \
            int m_idx = 0; unsigned int byte;       \
            for (m_idx = 0; m_idx < bufLen; ) {     \
                byte = pBuf[m_idx++];               \
                printf("%02x ", byte);              \
                if (0 == m_idx%16) {                 \
                    printf("\n");                   \
                    fflush(NULL);                   \
                }                                   \
            }                                       \
            printf("\n");                           \
        }                                           \
    }

#endif

