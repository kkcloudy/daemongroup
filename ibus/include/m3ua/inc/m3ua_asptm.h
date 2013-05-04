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

#ifndef __M3UA_ASPTM_H__
#define __M3UA_ASPTM_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>
#include <m3ua_msg_ecd.h>
#include <m3ua_msg_dcd.h>
#include <m3ua_conn.h>
#include <m3ua_co_proc.h>
#include <m3ua_mgmt.h>
#include <m3ua_ras.h>

m3_s32 m3uaAssertASPAC(m3_conn_inf_t *, m3_aspac_inf_t *);
m3_s32 m3uaAssertASPIA(m3_conn_inf_t *, m3_aspia_inf_t *);
m3_s32 m3uaAssertASPACACK(m3_conn_inf_t *, m3_aspac_ack_inf_t *, m3_u16 *, m3_u32 *);
m3_s32 m3uaAssertASPIAACK(m3_conn_inf_t *, m3_aspia_ack_inf_t *, m3_u16 *, m3_u32 *);
void m3uaCreateCommonRCList(m3_u16 *, m3_u32 *, m3_u16 *, m3_u32 *, m3_u16 *, m3_u32 *);
m3_s32 m3ua_RASP_ERECVACACK(m3_conn_inf_t *, m3_u32);
m3_s32 m3ua_RASP_ERECVIAACK(m3_conn_inf_t *, m3_u32);
m3_s32 m3ua_ASP_ERECVAC(m3_conn_inf_t *, m3_u32);
m3_s32 m3ua_ASP_ERECVIA(m3_conn_inf_t *, m3_u32);
m3_s32 m3ua_ASPINACTIVE_ESENDAC(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPINACTIVE_ERECVACACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPINACTIVE_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ERECVIAACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ESENDIA(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ESENDAC(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ERECVACACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ERECVAC(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ERECVIA(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ERECVAC(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ERECVIA(m3_conn_inf_t *, void *);

#endif

