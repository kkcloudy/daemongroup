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

#ifndef __M3UA_ROUTE_H__
#define __M3UA_ROUTE_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>
#include <m3ua_ui.h>
#include <m3ua_nwapp.h>
#include <m3ua_ssnm.h>
#include <m3ua_sg.h> //DEC2009

m3_s32 m3uaAddRoute(m3ua_route_t *);
m3_s32 m3uaDeleteRoute(m3ua_route_t *);
m3_s32 m3uaFindRoute(m3_pc_inf_t *, m3_rt_inf_t **);
void*  m3uaGetFreeRoute(void);
void   m3uaAdjustRTTBL(void);
void   m3uaInitRTListNode(m3_rt_tbl_list_t *);
void   m3uaSortRouteTBL(m3_pc_inf_t *);
m3_s32 m3uaSetRTState(m3_pc_inf_t *, m3_u32, m3_u8, m3_rtstate_t);
m3_bool_t m3uaAssertRoute(m3_pc_inf_t *);
void m3uaSetRTStatex(m3_pc_inf_t, m3_u32, m3_u8, m3_u32, m3_rtstate_t);

#endif

