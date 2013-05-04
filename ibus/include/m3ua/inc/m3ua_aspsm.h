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

#ifndef __M3UA_ASPSM_H__
#define __M3UA_ASPSM_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>
#include <m3ua_mgmt.h>

m3_s32 m3ua_ASPDOWN_ESENDUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPDOWN_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPUPSENT_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPUPSENT_ERECVUPACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPUPSENT_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPUPSENT_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPUPSENT_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ESENDDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPINACTIVE_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPINACTIVE_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPDNSENT_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPDNSENT_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPDNSENT_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_ASPACTIVE_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDOWN_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDOWN_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDOWN_ESENDUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ESENDDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPINACTIVE_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ERECVUPACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ESENDDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPUPRECV_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDNRECV_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDNRECV_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPDNRECV_ETIMEREXP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ERECVDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ESENDDN(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ERECVUP(m3_conn_inf_t *, void *);
m3_s32 m3ua_RASPACTIVE_ERECVDNACK(m3_conn_inf_t *, void *);
m3_s32 m3uaInvokeRASFSM(m3_conn_inf_t *, m3_as_event_t);
m3_s32 m3uaStartHBEAT(m3_conn_inf_t *);
m3_s32 m3uaStopHBEAT(m3_conn_inf_t *);
m3_s32 m3ua_ERECVHBEAT(m3_conn_inf_t *, void *);
m3_s32 m3ua_ERECVHBEATACK(m3_conn_inf_t *, void *);
m3_s32 m3ua_EHBEAT_TIMEREXP(m3_conn_inf_t *, void *);

#endif

