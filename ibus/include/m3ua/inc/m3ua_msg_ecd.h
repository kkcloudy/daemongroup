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

#ifndef __M3UA_MSG_ECD_H__
#define __M3UA_MSG_ECD_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>
#include <m3uaMemMgr.h>

m3_s32  m3uaEncodeDATA(m3_data_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDUNA(m3_duna_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDAVA(m3_dava_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDAUD(m3_daud_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeSCON(m3_scon_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDUPU(m3_dupu_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDRST(m3_drst_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPUP(m3_aspup_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPUPACK(m3_aspup_ack_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPDN(m3_aspdn_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPDNACK(m3_aspdn_ack_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeHBEAT(m3_hbeat_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeHBEATACK(m3_hbeat_ack_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPAC(m3_aspac_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPACACK(m3_aspac_ack_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPIA(m3_aspia_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeASPIAACK(m3_aspia_ack_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeERROR(m3_error_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeNTFY(m3_ntfy_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeREGRSP(m3_reg_rsp_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDREGREQ(m3_dreg_req_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeREGREQ(m3_reg_req_inf_t *, m3_u32 *, m3_u8 *);
m3_s32  m3uaEncodeDREGRSP(m3_dreg_rsp_inf_t *, m3_u32 *, m3_u8 *);

#endif

