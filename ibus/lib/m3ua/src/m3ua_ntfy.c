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

#include <m3ua_ntfy.h>

m3_s32    m3ua_mgmt_ntfy(m3ua_mgmt_ntfy_t    *p_inf)
{
    m3_mgmt_ntfy_inf_t    *pList = M3_MGMT_NTFY_TABLE;

    if (0 == pList->n_ntfy_pd) {
        M3_ASSIGN_ERRNO(EM3_NO_MGMT_NTFY_PD);
        return -1;
    }
    *p_inf = pList->ntfy_list[pList->c_offset];
    pList->n_ntfy_pd--;
    pList->c_offset++;
    if (M3_MAX_MGMT_NTFY == pList->c_offset) {
        pList->c_offset = 0;
    }
    return 0;
}

