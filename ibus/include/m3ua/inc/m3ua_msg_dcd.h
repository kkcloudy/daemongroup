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

#ifndef __M3UA_MSG_DCD_H__
#define __M3UA_MSG_DCD_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>
#include <m3ua_config.h>
#include <m3ua_errno.h>
#include <m3ua_extern.h>

m3_s32 m3uaDecodeDATA(m3_u32, m3_u8 *, m3_data_inf_t *);
m3_s32 m3uaDecodeDUNA(m3_u32, m3_u8 *, m3_duna_inf_t *);
m3_s32 m3uaDecodeSCON(m3_u32, m3_u8 *, m3_scon_inf_t *);
m3_s32 m3uaDecodeDUPU(m3_u32, m3_u8 *, m3_dupu_inf_t *);
m3_s32 m3uaDecodeASPUP(m3_u32, m3_u8 *, m3_aspup_inf_t *);
m3_s32 m3uaDecodeASPUPACK(m3_u32, m3_u8 *, m3_aspup_ack_inf_t *);
m3_s32 m3uaDecodeASPDN(m3_u32, m3_u8 *, m3_aspdn_inf_t *);
m3_s32 m3uaDecodeASPDNACK(m3_u32, m3_u8 *, m3_aspdn_ack_inf_t *);
m3_s32 m3uaDecodeHBEAT(m3_u32, m3_u8 *, m3_hbeat_inf_t *);
m3_s32 m3uaDecodeHBEATACK(m3_u32, m3_u8 *, m3_hbeat_ack_inf_t *);
m3_s32 m3uaDecodeASPAC(m3_u32, m3_u8 *, m3_aspac_inf_t *);
m3_s32 m3uaDecodeASPACACK(m3_u32, m3_u8 *, m3_aspac_ack_inf_t *);
m3_s32 m3uaDecodeASPIA(m3_u32, m3_u8 *, m3_aspia_inf_t *);
m3_s32 m3uaDecodeASPIAACK(m3_u32, m3_u8 *, m3_aspia_ack_inf_t *);
m3_s32 m3uaDecodeERROR(m3_u32, m3_u8 *, m3_error_inf_t *);
m3_s32 m3uaDecodeNTFY(m3_u32, m3_u8 *, m3_ntfy_inf_t *);
m3_s32 m3uaDecodeU32(m3_u8 **, m3_u32 *, m3_u32 *);
m3_s32 m3uaDecodeU32List(m3_u8 **, m3_u32 *, m3_u32 *, m3_u16 *);
m3_s32 m3uaDecodeInfoStr(m3_u8 **, m3_u32 *, m3_u8 **, m3_u8 *);
m3_s32 m3uaDecodeProtData(m3_u8 **, m3_u32 *, m3_rt_lbl_t *, m3_u8 **, m3_u16 *);
m3_s32 m3uaDecodeCongInd(m3_u8 **, m3_u32 *, m3_u8 *);
m3_s32 m3uaDecodeUsrCause(m3_u8 **, m3_u32 *, m3_u16 *, m3_u16 *);
m3_s32 m3uaDecodeBinData(m3_u8  **, m3_u32 *, m3_u8 **, m3_u16 *);
m3_s32 m3uaDecodeStatus(m3_u8 **, m3_u32 *, m3_u16 *, m3_u16 *);
m3_s32 m3uaDecodeCktRangeList(m3_u8 **, m3_u32 *, m3_ckt_range_t *, m3_u16 *);
m3_s32 m3uaDecodeU8List(m3_u8 **, m3_u32 *, m3_u8 *, m3_u16 *);
m3_s32 m3uaDecodeDeRegResult(m3_u8 **, m3_u32 *, m3_dreg_rsp_inf_t *);
m3_s32 m3uaDecodeRegResult(m3_u8 **, m3_u32 *, m3_reg_rsp_inf_t *);
m3_s32 m3uaDecodeRKEY(m3_u8 **, m3_u32 *, m3_reg_req_inf_t *);
m3_s32 m3uaDecodeDEREGRSP(m3_u32, m3_u8 *, m3_dreg_rsp_inf_t *);
m3_s32 m3uaDecodeREGRSP(m3_u32, m3_u8 *, m3_reg_rsp_inf_t *);
m3_s32 m3uaDecodeDEREGREQ(m3_u32, m3_u8 *, m3_dreg_req_inf_t *);
m3_s32 m3uaDecodeREGREQ(m3_u32, m3_u8 *, m3_reg_req_inf_t *);

#endif

