/****************************************************************************
** Description:
** Not implemented yet.
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

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_errno.h>
#include <m3ua_sys_inc.h>

int      m3errno = M3_NOERR;
unsigned int m3uaTraceMap = m3uaStartupTrace | m3uaErrorTrace | m3uaConfigTrace; // switch on only startup and error traces

void m3ua_set_trace_map(m3_u32 aTraceMap)
{
    m3uaTraceMap = aTraceMap;
}

void m3ua_add_trace(m3TrcType_t aTraceType)
{
    m3uaTraceMap = m3uaTraceMap | aTraceType;
}

void m3ua_del_trace(m3TrcType_t aTraceType)
{
    m3uaTraceMap = m3uaTraceMap & ~(aTraceType);
}

m3_u32 m3ua_get_trace_map()
{
    return m3uaTraceMap;
}

