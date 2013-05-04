/****************************************************************************
** Description:
** Code to decode messages.
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

#include <m3ua_msg_dcd.h>
#include <osmocore/logging.h>
#include <osmocore/msgb.h>
#include <osmocore/utils.h>

#include "sccp.h"


m3_s32 m3uaDecodeDATA(m3_u32             buflen,
                      m3_u8              *p_buf,
                      m3_data_inf_t      *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->nw_app = M3_MAX_U32;
    p_inf->rtctx = M3_MAX_U32;
    p_inf->crn_id = M3_MAX_U32;
    p_inf->prot_data_len = 0;
    p_inf->rt_lbl.opc = M3_MAX_U32;
    while(4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_NW_APP: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->nw_app);
                break;
            }
            case M3_TAG_RT_CTX: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->rtctx);
                break;
            }
            case M3_TAG_PROT_DATA: {
                ret_val = m3uaDecodeProtData(&p_buf, &buflen, &p_inf->rt_lbl,
                    &p_inf->p_prot_data, &p_inf->prot_data_len);
				
				if (M3_SIO_SCCP == p_inf->rt_lbl.si) { /*add for sccp SCCP_PROTOCOL*/
					struct msgb *msg = msgb_alloc_headroom(p_inf->prot_data_len + 2, 2, __func__);
					msg->l2h = msgb_put(msg, p_inf->prot_data_len);
					memcpy(msg->l2h, p_inf->p_prot_data, p_inf->prot_data_len);
					iu_log_debug("\nsend messageeeeeee to sccp start, prot data len is %d\n", p_inf->prot_data_len);
					if(p_inf->rt_lbl.opc==global_sgsn_parameter.point_code)
					{
						gCnDomain = 1;
					}
					else
					{
						gCnDomain = 0;
					}
					iu_log_debug("p_inf->rt_lbl.dpc = %d, p_inf->rt_lbl.opc = %d, gCnDomain = %d\n",p_inf->rt_lbl.dpc,p_inf->rt_lbl.opc,gCnDomain);
					sccp_system_incoming(msg);
					iu_log_debug("send messageeeeeee to sccp end\n\n");
					msgb_free(msg);
				}
				
                break;
            }
            case M3_TAG_CRN_ID: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->crn_id);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (M3_MAX_U32 == p_inf->rt_lbl.opc) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    iu_log_debug("decode m3ua data end\n");
    return 0;
}

m3_s32 m3uaDecodeDUNA(m3_u32             buflen,
                      m3_u8              *p_buf,
                      m3_duna_inf_t      *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->nw_app = M3_MAX_U32;
    p_inf->num_rc = 0;
    p_inf->num_pc = 0;
    p_inf->info_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_NW_APP: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->nw_app);
                M3TRACE(m3uaSsnmTrace, ("NwApp: 0x%x",(unsigned int)p_inf->nw_app));
                break;
            }
            case M3_TAG_RT_CTX: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
                    &p_inf->num_rc);
                M3TRACE(m3uaSsnmTrace, ("RoutCtx: 0x%x",(unsigned int)p_inf->rc_list[0]));
                break;
            }
            case M3_TAG_AFF_PC: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->pc_list,
                    &p_inf->num_pc);
                break;
            }
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_pc) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeSCON(m3_u32             buflen,
                      m3_u8              *p_buf,
                      m3_scon_inf_t      *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->nw_app = M3_MAX_U32;
    p_inf->num_rc = 0;
    p_inf->num_pc = 0;
    p_inf->info_len = 0;
    p_inf->crnd_dpc = M3_MAX_U32;
    p_inf->cong_level = M3_MAX_U8;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_NW_APP: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->nw_app);
                break;
            }
            case M3_TAG_RT_CTX: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
                    &p_inf->num_rc);
                break;
            }
            case M3_TAG_AFF_PC: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->pc_list,
                    &p_inf->num_pc);
                break;
            }
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            case M3_TAG_CRND_DPC: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->crnd_dpc);
                break;
            }
            case M3_TAG_CONG_IND: {
                ret_val = m3uaDecodeCongInd(&p_buf, &buflen,
                    &p_inf->cong_level);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_pc) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeDUPU(m3_u32             buflen,
                      m3_u8              *p_buf,
                      m3_dupu_inf_t      *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->nw_app = M3_MAX_U32;
    p_inf->num_rc = 0;
    p_inf->pc = M3_MAX_U32;
    p_inf->info_len = 0;
    p_inf->cause = M3_MAX_U16;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_NW_APP: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->nw_app);
                break;
            }
            case M3_TAG_RT_CTX: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
                    &p_inf->num_rc);
                break;
            }
            case M3_TAG_AFF_PC: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->pc);
                break;
            }
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            case M3_TAG_USR_CAUSE: {
                ret_val = m3uaDecodeUsrCause(&p_buf, &buflen,
                    &p_inf->cause, &p_inf->user);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (M3_MAX_U32 == p_inf->pc || M3_MAX_U16 == p_inf->cause) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeASPUP(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_aspup_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->m3asp_id = M3_MAX_U32;
    p_inf->info_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_ASP_ID: {
                ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->m3asp_id);
                break;
            }
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeASPUPACK(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_aspup_ack_inf_t *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    /* initialise the contents of aspup ack information structure */
    p_inf->info_len = 0;

    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeASPDN(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_aspdn_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->info_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeASPDNACK(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_aspdn_ack_inf_t *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->info_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_INFO_STR: {
                ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
                    &p_inf->p_info_str, &p_inf->info_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeHBEAT(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_hbeat_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->hbeat_data_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_HBEAT_DATA: {
                ret_val = m3uaDecodeBinData(&p_buf, &buflen,
                    &p_inf->p_hbeat_data, &p_inf->hbeat_data_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeHBEATACK(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_hbeat_ack_inf_t *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->hbeat_data_len = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_HBEAT_DATA: {
                ret_val = m3uaDecodeBinData(&p_buf, &buflen,
                    &p_inf->p_hbeat_data, &p_inf->hbeat_data_len);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaDecodeASPAC(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_aspac_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rc = 0;
    p_inf->info_len = 0;
    p_inf->trfmode = M3_MAX_U32;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_TRFMODE: {
	            ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->trfmode);
	            break;
	        }
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
	                &p_inf->num_rc);
	            break;
	        }
	        case M3_TAG_INFO_STR: {
	            ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
	                &p_inf->p_info_str, &p_inf->info_len);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    return 0;
}


m3_s32 m3uaDecodeASPACACK(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_aspac_ack_inf_t *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rc = 0;
    p_inf->info_len = 0;
    p_inf->trfmode = M3_MAX_U32;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_TRFMODE: {
	            ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->trfmode);
	            break;
	        }
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
	                &p_inf->num_rc);
	            break;
	        }
	        case M3_TAG_INFO_STR: {
	            ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
	                &p_inf->p_info_str, &p_inf->info_len);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    return 0;
}

m3_s32 m3uaDecodeASPIA(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_aspia_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rc = 0;
    p_inf->info_len = 0;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list, &p_inf->num_rc);
	            break;
	        }
	        case M3_TAG_INFO_STR: {
	            ret_val = m3uaDecodeInfoStr(&p_buf, &buflen, &p_inf->p_info_str, &p_inf->info_len);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    return 0;
}

m3_s32 m3uaDecodeASPIAACK(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_aspia_ack_inf_t *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rc = 0;
    p_inf->info_len = 0;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
	                &p_inf->num_rc);
	            break;
	        }
	        case M3_TAG_INFO_STR: {
	            ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
	                &p_inf->p_info_str, &p_inf->info_len);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    return 0;
}

m3_s32 m3uaDecodeERROR(m3_u32             buflen,
                       m3_u8              *p_buf,
                       m3_error_inf_t     *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->err_code = M3_MAX_U32;
    p_inf->nw_app = M3_MAX_U32;
    p_inf->num_rc = 0;
    p_inf->num_pc = 0;
    p_inf->diag_len = 0;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_ERR_CODE: {
	            ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->err_code);
	            break;
	        }
	        case M3_TAG_AFF_PC: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen,
                        p_inf->pc_list, &p_inf->num_pc);
	            break;
	        }
	        case M3_TAG_NW_APP: {
	            ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->nw_app);
	            break;
	        }
	        case M3_TAG_DIAG_INFO: {
	            ret_val = m3uaDecodeBinData(&p_buf, &buflen,
                        &p_inf->p_diag_inf, &p_inf->diag_len);
	            break; /* JULY 2010 -- BUG REPORT 2998386 */
	        }
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
	                &p_inf->num_rc);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    if (M3_MAX_U32 == p_inf->err_code) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeNTFY(m3_u32             buflen,
                      m3_u8              *p_buf,
                      m3_ntfy_inf_t      *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->status_type = M3_MAX_U16;
    p_inf->status_inf = M3_MAX_U16;
    p_inf->num_rc = 0;
    p_inf->m3asp_id = M3_MAX_U32;
    p_inf->info_len = 0;
    while (4 < buflen) {
	    p_tlv = (m3_msg_tlv_t*)p_buf;
	    switch(p_tlv->tag) {
	        case M3_TAG_STATUS: {
	            ret_val = m3uaDecodeStatus(&p_buf, &buflen,
                        &p_inf->status_type, &p_inf->status_inf);
	            break;
	        }
	        case M3_TAG_RT_CTX: {
	            ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
	                &p_inf->num_rc);
	            break;
	        }
	        case M3_TAG_ASP_ID: {
	            ret_val = m3uaDecodeU32(&p_buf, &buflen, &p_inf->m3asp_id);
	            break;
	        }
	        case M3_TAG_INFO_STR: {
	            ret_val = m3uaDecodeInfoStr(&p_buf, &buflen,
	                &p_inf->p_info_str, &p_inf->info_len);
	            break;
	        }
	        default: {
	            if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
	                M3ERRNO(EM3_INV_TAG);
	            } else {
	                M3ERRNO(EM3_UNEXP_PARAM);
	            }
	            return -1;
	        }
	    }
	    if (-1 == ret_val) {
	        return -1;
	    }
    }
    if (M3_MAX_U16 == p_inf->status_type) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeREGREQ(m3_u32             buflen,
                        m3_u8              *p_buf,
                        m3_reg_req_inf_t   *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rk = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_RKEY: {
                ret_val = m3uaDecodeRKEY(&p_buf, &buflen, p_inf);
                p_inf->num_rk += 1;
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_rk) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeREGRSP(m3_u32             buflen,
                        m3_u8              *p_buf,
                        m3_reg_rsp_inf_t   *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_reg_result = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_REG_RESULT: {
                ret_val = m3uaDecodeRegResult(&p_buf, &buflen, p_inf);
                p_inf->num_reg_result += 1;
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_reg_result) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeDEREGREQ(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_dreg_req_inf_t  *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_rc = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_RT_CTX: {
                ret_val = m3uaDecodeU32List(&p_buf, &buflen, p_inf->rc_list,
                    &p_inf->num_rc);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_rc) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeDEREGRSP(m3_u32             buflen,
                          m3_u8              *p_buf,
                          m3_dreg_rsp_inf_t  *p_inf)
{
    m3_msg_tlv_t        *p_tlv;
    m3_s32              ret_val;

    p_inf->num_dreg_result = 0;
    while (4 < buflen) {
        p_tlv = (m3_msg_tlv_t*)p_buf;
        switch(p_tlv->tag) {
            case M3_TAG_DEREG_RESULT: {
                ret_val = m3uaDecodeDeRegResult(&p_buf, &buflen, p_inf);
                p_inf->num_dreg_result += 1;
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1;
        }
    }
    if (0 == p_inf->num_dreg_result) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeU32(m3_u8          **pp_buf,
                     m3_u32         *p_buflen,
                     m3_u32         *p_u32val)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (8 != tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_u32val = M3_NTOHL(p_tlv->val[0]);
    *pp_buf = *pp_buf + 8;
    *p_buflen = *p_buflen - 8;
    return 0;
}

m3_s32 m3uaDecodeU32List(m3_u8        **pp_buf,
                         m3_u32       *p_buflen,
                         m3_u32       *p_u32list,
                         m3_u16       *p_numelem)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          idx;
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (8 > tlv_len || 0 != tlv_len%4 || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_numelem = (tlv_len - 4)/4;
    for (idx = 0; idx < *p_numelem; idx++) {
        p_u32list[idx] = M3_NTOHL(p_tlv->val[idx]);
    }
    *pp_buf = *pp_buf + tlv_len;
    *p_buflen = *p_buflen - tlv_len;
    return 0;
}

m3_s32 m3uaDecodeU8List(m3_u8        **pp_buf,
                        m3_u32       *p_buflen,
                        m3_u8        *p_u8list,
                        m3_u16       *p_numelem)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u8           *pu8buf = (m3_u8 *)(*pp_buf + 4);
    m3_u16          idx;
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (4 > tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_numelem = tlv_len - 4;
    for (idx = 0; idx < *p_numelem; idx++) {
        p_u8list[idx] = pu8buf[idx];
    }
    M3_ALIGN_4(tlv_len);
    if (tlv_len > *p_buflen) {
        *p_buflen = 0;
    } else {
        *p_buflen = *p_buflen - tlv_len;
    }
    *pp_buf = *pp_buf + tlv_len;
    return 0;
}

m3_s32 m3uaDecodeInfoStr(m3_u8        **pp_buf,
                         m3_u32       *p_buflen,
                         m3_u8        **pp_info_str,
                         m3_u8        *p_info_len)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (4 >= tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *pp_info_str = *pp_buf + 4;
    *p_info_len = tlv_len - 4;
    M3_ALIGN_4(tlv_len);
    if (tlv_len > *p_buflen) {
        *p_buflen = 0;
    } else {
        *p_buflen = *p_buflen - tlv_len;
    }
    *pp_buf = *pp_buf + tlv_len;
    return 0;
}

m3_s32 m3uaDecodeProtData(m3_u8       **pp_buf,
                          m3_u32      *p_buflen,
                          m3_rt_lbl_t *p_rtlbl,
                          m3_u8       **pp_prot_data,
                          m3_u16      *p_prot_data_len)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);
    
    if (4 + M3_RT_LBL_LEN > tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_rtlbl = *((m3_rt_lbl_t*)(*pp_buf + 4));
    p_rtlbl->opc = M3_NTOHL(p_rtlbl->opc);
    p_rtlbl->dpc = M3_NTOHL(p_rtlbl->dpc);
    *pp_prot_data = *pp_buf + 4 + M3_RT_LBL_LEN;
    *p_prot_data_len = tlv_len - 4 - M3_RT_LBL_LEN;
    M3_ALIGN_4(tlv_len);
    if (tlv_len > *p_buflen) {
        *p_buflen = 0;
    } else {
        *p_buflen = *p_buflen - tlv_len;
    }
    *pp_buf = *pp_buf + tlv_len;
    return 0;
}

m3_s32 m3uaDecodeCongInd(m3_u8        **pp_buf,
                         m3_u32       *p_buflen,
                         m3_u8        *p_cong_level)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (8 != tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_cong_level = (m3_u8)M3_NTOHL(p_tlv->val[0]);
    *pp_buf = *pp_buf + 8;
    *p_buflen = *p_buflen - 8;
    return 0;
}

m3_s32 m3uaDecodeUsrCause(m3_u8        **pp_buf,
                          m3_u32       *p_buflen,
                          m3_u16       *p_cause,
                          m3_u16       *p_user)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          *p_u16val = (m3_u16*)p_tlv->val;
    m3_u8           tlv_len = M3_NTOHS(p_tlv->len);

    if (8 != tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_cause = (m3_u8)M3_NTOHS(p_u16val[0]);
    *p_user  = (m3_u8)M3_NTOHS(p_u16val[1]);
    *pp_buf = *pp_buf + 8;
    *p_buflen = *p_buflen - 8;
    return 0;
}

m3_s32 m3uaDecodeBinData(m3_u8        **pp_buf,
                         m3_u32       *p_buflen,
                         m3_u8        **pp_hbeat_data, 
                         m3_u16       *p_hbeat_data_len)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);
    
    if (4 >= tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *pp_hbeat_data = *pp_buf + 4;
    *p_hbeat_data_len = tlv_len - 4;
    M3_ALIGN_4(tlv_len);
    if (tlv_len > *p_buflen) {
        *p_buflen = 0;
    } else {
        *p_buflen = *p_buflen - tlv_len;
    }
    *pp_buf = *pp_buf + tlv_len;
    return 0;
}

m3_s32 m3uaDecodeStatus(m3_u8        **pp_buf,
                        m3_u32       *p_buflen,
                        m3_u16       *p_status_type,
                        m3_u16       *p_status_inf)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          *p_u16val = (m3_u16*)p_tlv->val;
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);

    if (8 != tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_status_type = (m3_u8)M3_NTOHS(p_u16val[0]);
    *p_status_inf  = (m3_u8)M3_NTOHS(p_u16val[1]);
    *pp_buf = *pp_buf + 8;
    *p_buflen = *p_buflen - 8;
    return 0;
}

m3_s32 m3uaDecodeRKEY(m3_u8            **pp_buf,
                      m3_u32           *p_buflen,
                      m3_reg_req_inf_t *p_inf)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u32          tlv_len = M3_NTOHS(p_tlv->len);
    m3_u32          len;
    m3_u8           num_dpc = 0;
    m3_s32          ret_val = 0;

    if (4 + M3_MIN_RKEY_LEN > tlv_len || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    p_inf->rk_list[p_inf->num_rk].lrk_id = M3_MAX_U32;
    p_inf->rk_list[p_inf->num_rk].rk_inf.trfmode = M3_MAX_U32;
    p_inf->rk_list[p_inf->num_rk].rk_inf.nw_app = M3_MAX_U32;
    p_inf->rk_list[p_inf->num_rk].rk_inf.num_rtparam = 0;
    *pp_buf += 4;
    len = tlv_len;
    M3_ALIGN_4(len);
    if (len > *p_buflen) {
        *p_buflen = 0;
    } else {
        *p_buflen = *p_buflen - len;
    }
    tlv_len -= 4;
    while (0 != tlv_len) {
        p_tlv = (m3_msg_tlv_t*)(*pp_buf);
        switch(p_tlv->tag) {
            case M3_TAG_LRK_ID: {
                if (M3_MAX_U32 != p_inf->rk_list[p_inf->num_rk].lrk_id) {
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rk_list[p_inf->num_rk].lrk_id);
                break;
            }
            case M3_TAG_TRFMODE: {
                if (M3_MAX_U32 != p_inf->rk_list[p_inf->num_rk].rk_inf.trfmode){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rk_list[p_inf->num_rk].rk_inf.trfmode);
                break;
            }
            case M3_TAG_DPC: {
                num_dpc += 1;
                p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    num_si = 0;
                p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    num_opc = 0;
                p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    num_ckt_range = 0;
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rk_list[p_inf->num_rk].rk_inf.
                    rtparam[num_dpc-1].dpc);
                break;
            }
            case M3_TAG_NW_APP: {
                if (M3_MAX_U32 != p_inf->rk_list[p_inf->num_rk].rk_inf.nw_app){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rk_list[p_inf->num_rk].rk_inf.nw_app);
                break;
            }
            case M3_TAG_SI: {
                if (0 == num_dpc) {
                    M3ERRNO(EM3_MISSING_PARAM);
                    return -1;
                } else if (0 != p_inf->rk_list[p_inf->num_rk].rk_inf.
                  rtparam[num_dpc-1].num_si) {
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU8List(pp_buf, &tlv_len,
                    p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    si_list, &p_inf->rk_list[p_inf->num_rk].rk_inf.
                    rtparam[num_dpc-1].num_si);
                break;
            }
            case M3_TAG_OPC_LIST: {
                if (0 == num_dpc) {
                    M3ERRNO(EM3_MISSING_PARAM);
                    return -1;
                } else if ( 0 != p_inf->rk_list[p_inf->num_rk].rk_inf.
                  rtparam[num_dpc-1].num_opc) {
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32List(pp_buf, &tlv_len, 
                    p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    opc_list, &p_inf->rk_list[p_inf->num_rk].rk_inf.
                    rtparam[num_dpc-1].num_opc);
                break;
            }
            case M3_TAG_CKT_RANGE: {
                if (0 == num_dpc) {
                    M3ERRNO(EM3_MISSING_PARAM);
                    return -1;
                } else if (0 != p_inf->rk_list[p_inf->num_rk].rk_inf.
                  rtparam[num_dpc-1].num_ckt_range) {
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeCktRangeList(pp_buf, &tlv_len, 
                    p_inf->rk_list[p_inf->num_rk].rk_inf.rtparam[num_dpc-1].
                    ckt_range, &p_inf->rk_list[p_inf->num_rk].rk_inf.
                    rtparam[num_dpc-1].num_ckt_range);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1; 
        }   
    }
    if (0 == num_dpc || M3_MAX_U32 == p_inf->rk_list[p_inf->num_rk].lrk_id) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    p_inf->rk_list[p_inf->num_rk].rk_inf.num_rtparam = num_dpc;
    return 0;
}

m3_s32 m3uaDecodeCktRangeList(m3_u8        **pp_buf,
                              m3_u32       *p_buflen,
                              m3_ckt_range_t *p_ckt,
                              m3_u16       *p_numckt)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u16          idx, ckt_idx;
    m3_u16          tlv_len = M3_NTOHS(p_tlv->len);
    m3_u16          *p_u16val;

    if (12 > tlv_len || 0 != tlv_len%4 || *p_buflen < tlv_len) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_numckt = (tlv_len - 4)/8;
    for (idx = 0, ckt_idx = 0; idx < *p_numckt; idx = idx + 1, ckt_idx++) {
        p_ckt[ckt_idx].opc = M3_NTOHL(p_tlv->val[2*idx]);
        p_u16val = (m3_u16 *)&(p_tlv->val[2*idx + 1]);
        p_ckt[ckt_idx].lcic = M3_NTOHS(p_u16val[0]);
        p_ckt[ckt_idx].ucic = M3_NTOHS(p_u16val[1]);
    }
    *pp_buf = *pp_buf + tlv_len;
    *p_buflen = *p_buflen - tlv_len;
    return 0;
}

m3_s32 m3uaDecodeRegResult(m3_u8            **pp_buf,
                           m3_u32           *p_buflen,
                           m3_reg_rsp_inf_t *p_inf)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u32          tlv_len = M3_NTOHS(p_tlv->len);
    m3_s32          ret_val = 0;

    if (4 + M3_MIN_REG_RESULT_LEN > tlv_len || *p_buflen < tlv_len || 0 != tlv_len%4) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    p_inf->rr_list[p_inf->num_reg_result].lrk_id = M3_MAX_U32;
    p_inf->rr_list[p_inf->num_reg_result].reg_status = M3_MAX_U32;
    p_inf->rr_list[p_inf->num_reg_result].rtctx = M3_MAX_U32;
    *p_buflen -= tlv_len;
    *pp_buf += 4;
    tlv_len -= 4;
    while (0 != tlv_len) {
        p_tlv = (m3_msg_tlv_t*)(*pp_buf);
        switch(p_tlv->tag) {
            case M3_TAG_LRK_ID: {
                if (M3_MAX_U32 != p_inf->rr_list[p_inf->num_reg_result].lrk_id){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rr_list[p_inf->num_reg_result].lrk_id);
                break;
            }
            case M3_TAG_REG_STATUS: {
                if (M3_MAX_U32 != p_inf->rr_list[p_inf->num_reg_result].reg_status){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rr_list[p_inf->num_reg_result].reg_status);
                break;
            }
            case M3_TAG_RT_CTX: {
                if (M3_MAX_U32 != p_inf->rr_list[p_inf->num_reg_result].rtctx){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->rr_list[p_inf->num_reg_result].rtctx);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
        if (-1 == ret_val) {
            return -1; 
        }   
    }
    if (M3_MAX_U32 == p_inf->rr_list[p_inf->num_reg_result].reg_status    ||
        M3_MAX_U32 == p_inf->rr_list[p_inf->num_reg_result].lrk_id        ||
        M3_MAX_U32 == p_inf->rr_list[p_inf->num_reg_result].rtctx) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

m3_s32 m3uaDecodeDeRegResult(m3_u8             **pp_buf,
                             m3_u32            *p_buflen,
                             m3_dreg_rsp_inf_t *p_inf)
{
    m3_msg_tlv_t    *p_tlv = (m3_msg_tlv_t*)(*pp_buf);
    m3_u32          tlv_len = M3_NTOHS(p_tlv->len);
    m3_s32          ret_val = 0;

    if (4 + M3_MIN_DEREG_RESULT_LEN > tlv_len || *p_buflen < tlv_len || 0 != tlv_len%4) {
        M3ERRNO(EM3_PARAM_FIELD);
        return -1;
    }
    *p_buflen -= tlv_len;
    p_inf->drr_list[p_inf->num_dreg_result].dreg_status = M3_MAX_U32;
    p_inf->drr_list[p_inf->num_dreg_result].rtctx = M3_MAX_U32;
    *pp_buf += 4;
    tlv_len -= 4;
    while (0 != tlv_len) {
        p_tlv = (m3_msg_tlv_t*)(*pp_buf);
        switch(p_tlv->tag) {
            case M3_TAG_DEREG_STATUS: {
                if (M3_MAX_U32 != p_inf->drr_list[p_inf->num_dreg_result].dreg_status){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->drr_list[p_inf->num_dreg_result].dreg_status);
                break;
            }
            case M3_TAG_RT_CTX: {
                if (M3_MAX_U32 != p_inf->drr_list[p_inf->num_dreg_result].rtctx){
                    M3ERRNO(EM3_REP_TAG);
                    return -1;
                }
                ret_val = m3uaDecodeU32(pp_buf, &tlv_len,
                    &p_inf->drr_list[p_inf->num_dreg_result].rtctx);
                break;
            }
            default: {
                if (M3_FALSE == M3_IS_TAG_VALID(p_tlv->tag)) {
                    M3ERRNO(EM3_INV_TAG);
                } else {
                    M3ERRNO(EM3_UNEXP_PARAM);
                }
                return -1;
            }
        }
	if (-1 == ret_val) {
	    return -1;
	}
    }
    if (M3_MAX_U32 == p_inf->drr_list[p_inf->num_dreg_result].dreg_status    ||
        M3_MAX_U32 == p_inf->drr_list[p_inf->num_dreg_result].rtctx) {
        M3ERRNO(EM3_MISSING_PARAM);
        return -1;
    }
    return 0;
}

