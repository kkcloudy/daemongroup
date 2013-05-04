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

#ifndef __M3UA_SSNM_H__
#define __M3UA_SSNM_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3ua_route.h>
#include <m3ua_as.h>
#include <m3ua_nwapp.h>
#include <m3ua_mgmt.h>
#include <m3ua_msg_dcd.h>

void m3uaProcSSNM(m3_conn_inf_t *, m3_u32, m3_msg_hdr_t *, m3_u8 *, m3_u32);
m3_s32 m3uaAssertDUNA(m3_conn_inf_t *, m3_duna_inf_t *);
m3_s32 m3uaAssertDAUD(m3_conn_inf_t *, m3_daud_inf_t *); //FEB2010
m3_s32 m3uaAssertSCON(m3_conn_inf_t *, m3_scon_inf_t *);
m3_s32 m3uaAssertDUPU(m3_conn_inf_t *, m3_dupu_inf_t *);
m3_s32 m3ua_ERECVDUNA(m3_conn_inf_t *, m3_duna_inf_t *);
m3_s32 m3ua_ERECVDAVA(m3_conn_inf_t *, m3_dava_inf_t *);
m3_s32 m3ua_ERECVDAUD(m3_conn_inf_t *, m3_daud_inf_t *);
m3_s32 m3ua_ERECVSCON(m3_conn_inf_t *, m3_scon_inf_t *);
m3_s32 m3ua_ERECVDUPU(m3_conn_inf_t *, m3_dupu_inf_t *);
m3_s32 m3ua_ERECVDRST(m3_conn_inf_t *, m3_drst_inf_t *);
void   m3uaUpdateNwState(m3_conn_inf_t *, m3_u32, m3_u8, m3_u32, m3_rtstate_t);
m3_s32 m3uaNtfyPause(m3_u32, m3_u32);
m3_s32 m3uaNtfyResume(m3_u32, m3_u32);
m3_s32 m3uaNtfyAudit(m3_conn_inf_t *, m3_u32, m3_u8, m3_u32);
m3_s32 m3uaNtfyStatus(m3_u32, m3_u32, m3_u32, m3_u8, m3_u8);
m3_s32 m3uaSendDAUD(m3_asp_inf_t *, m3_u32, m3ua_audit_t *);

#endif

