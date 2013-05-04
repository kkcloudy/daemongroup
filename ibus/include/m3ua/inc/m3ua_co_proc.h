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

#ifndef __M3UA_CO_PROC_H__
#define __M3UA_CO_PROC_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>
#include <m3uaTimerMgr.h>
#include <m3uaTraceMgr.h>
#include <m3ua_as.h>
#include <m3ua_aspsm.h>
#include <m3ua_rkm.h>

m3_s32 m3uaStartASPMTimer(m3_conn_inf_t *, m3_u32, void *, m3_u32);
m3_s32 m3uaStartHBEATTimer(m3_conn_inf_t *, void *, m3_u32);
m3_s32 m3uaFreeTimer(m3_timer_inf_t *);
m3_s32 m3uaVoid(m3_conn_inf_t *, void *);
void m3uaTimerExpiry(m3_u32, void *);
m3_s32 m3uaSendMsg(m3_conn_inf_t *, m3_u32, m3_u32, m3_u8 *);
m3_s32 m3uaNtfyASPState(m3_conn_inf_t *, m3_u32, m3_asp_state_t);
m3_s32 m3uaNtfyRASPState(m3_conn_inf_t *, m3_u32, m3_asp_state_t);
m3_s32 m3uaNtfyASState(m3_u32, m3_conn_inf_t *, m3_as_state_t);
m3_s32 m3uaNtfyRASState(m3_r_as_inf_t *, m3_u32, m3_as_state_t);
m3_s32 m3uaNtfyErr(m3_conn_inf_t *, m3_error_inf_t *);
m3_s32 m3uaNtfyConnState(m3_conn_inf_t *, m3_conn_state_t);
m3_s32 m3uaNtfyNotify(m3_conn_inf_t *, m3_ntfy_inf_t *);
m3_match_t m3uaU32ListMatch(m3_u32, m3_u32 *, m3_u32, m3_u32 *);
m3_match_t m3uaU8ListMatch(m3_u32, m3_u8 *, m3_u32, m3_u8 *);
m3_s32 m3uaStartRKMTimer(m3_conn_inf_t *, m3_u32, void *, m3_u32);
m3_s32 m3uaAddNtfyToList(m3ua_mgmt_ntfy_t *);

#endif

