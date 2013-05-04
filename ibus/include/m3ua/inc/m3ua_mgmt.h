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

#ifndef __M3UA_MGMT_H__
#define __M3UA_MGMT_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>
#include <m3ua_co_proc.h>
#include <m3ua_msg_ecd.h>
#include <m3ua_as.h>
#include <m3ua_as_proc.h>
#include <m3ua_asp_proc.h>
#include <m3ua_nwapp.h>
#include <m3ua_msg_dcd.h>

void m3uaProcMGMT(m3_conn_inf_t *, m3_u32, m3_msg_hdr_t *, m3_u8 *, m3_u32);
m3_s32 m3uaAssertERR(m3_conn_inf_t *, m3_error_inf_t *);
m3_s32 m3ua_ERECVERR(m3_conn_inf_t *, m3_error_inf_t *);
m3_s32 m3uaAssertNTFY(m3_conn_inf_t *, m3_ntfy_inf_t *);
m3_s32 m3ua_ERECVNTFY(m3_conn_inf_t *, m3_ntfy_inf_t *);
m3_s32 m3ua_ENTFY_ALTASPACTIVE(m3_conn_inf_t *, void *);
m3_s32 m3ua_ECONNSTATE(m3_conn_inf_t *, void *);
m3_s32 m3ua_ESENDERROR(m3_conn_inf_t *, m3_error_inf_t *);

#endif

