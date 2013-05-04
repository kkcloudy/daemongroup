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

#include <m3ua_diag.h>

m3_s32 m3ua_diag(m3_u32 diagType, m3_s8 *diagStr, m3_u32 len)
{
    m3_u32 diagStrLen = 0;

    if (M3_NULL == diagStr || 0 == len) {
        M3TRACE(m3uaErrorTrace, ("Null configuration information passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }

    diagStr[0] = '\0';

    switch(diagType)
    {
        case M3_MEM_DIAG:
        {
            diagStrLen += m3uaMemMgrDiag(&(diagStr[diagStrLen]), (len - diagStrLen));
            break;
        }
        case M3_TIMER_DIAG:
        {
            diagStrLen += m3uaTimerMgrDiag(&(diagStr[diagStrLen]), (len - diagStrLen));
            break;
        }
        case M3_ALL_DIAG:
        {
            diagStrLen += m3uaMemMgrDiag(&(diagStr[diagStrLen]), (len - diagStrLen));
            diagStrLen += m3uaTimerMgrDiag(&(diagStr[diagStrLen]), (len - diagStrLen));
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

