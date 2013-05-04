/*****************************************************************************
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
******************************************************************************/

#ifndef __M3UA_RKM_H__
#define __M3UA_RKM_H__

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
#include <m3ua_msg_ecd.h>
#include <m3ua_sgp.h>
#include <m3ua_asp.h>

m3_s32 m3ua_L_RKM(m3_conn_inf_t *, m3_l_rkm_evt_t, void *);
m3_s32 m3ua_R_RKM(m3_conn_inf_t *, m3_r_rkm_evt_t, void *);
m3_bool_t m3uaAssertLRKState(m3_conn_inf_t *, m3_u8, m3_u16, m3_u32 *);
m3_bool_t m3uaAssertRRKState(m3_conn_inf_t *, m3_u8, m3_u16, m3_u32 *);
void m3uaModifyLRKState(m3_conn_inf_t *, m3_u8, m3_u16, m3_u32 *);
m3_s32 m3uaProcRegister(m3_conn_inf_t *, void *);
m3_bool_t m3uaAssertREGRSP(m3_conn_inf_t *, m3_reg_rsp_inf_t *);
void m3uaCreateCommonLRKList(m3_u16 *, m3_u32 *, m3_u16 *, m3_u32 *, m3_u16 *, m3_u32 *);
m3_s32 m3uaProcRegRsp(m3_conn_inf_t *, void *);
m3_s32 m3uaProcDeregister(m3_conn_inf_t *, void *);
m3_bool_t m3uaAssertDEREGRSP(m3_conn_inf_t *, m3_dreg_rsp_inf_t  *, m3_u32 *);
m3_s32 m3uaProcDeregRsp(m3_conn_inf_t *, void *);
m3_s32 m3uaProcRkmTimeout(m3_conn_inf_t *, void *);
m3_match_t m3uaRKAliasParamMatch(m3_rk_elements_t *, m3_rk_elements_t *);
m3_match_t m3uaRKMatch(m3_rk_inf_t *, m3_rk_inf_t *);
m3_match_t m3uaCktRangeListMatch(m3_u32, m3_ckt_range_t *, m3_u32, m3_ckt_range_t *);
void m3uaSaveLRKID(m3_conn_inf_t *, m3_r_as_inf_t *, m3_u32);
m3_u32 m3uaGetLRKID(m3_conn_inf_t *, m3_r_as_inf_t *);
m3_s32 m3uaProcRegReq(m3_conn_inf_t *, void *);
m3_s32 m3uaProcRkStatus(m3_conn_inf_t *, void *);
m3_s32 m3uaProcDeregReq(m3_conn_inf_t *, void *);
void m3uaProcRKM(m3_conn_inf_t *, m3_u32, m3_msg_hdr_t *, m3_u8 *, m3_u32);

#endif

