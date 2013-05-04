/****************************************************************************
** Description:
** Code for encoding messages.
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

#include <m3ua_msg_ecd.h>

m3_s32  m3uaEncodeDATA(m3_data_inf_t        *p_inf,
                       m3_u32               *p_len,
                       m3_u8                *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t         *p_hdr;
    m3_msg_tlv_t         *p_tlv;
    m3_msg_data_val_t    *p_data_val;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_TXR;
    p_hdr->msg_type   = M3_MSG_TYPE_DATA;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (M3_MAX_U32 != p_inf->rtctx) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->rtctx);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag = M3_TAG_PROT_DATA;
    p_tlv->len = 4 + M3_RT_LBL_LEN + p_inf->prot_data_len;
    p_data_val = (m3_msg_data_val_t*)&(p_tlv->val[0]);
    p_data_val->opc = M3_HTONL(p_inf->rt_lbl.opc);
    p_data_val->dpc = M3_HTONL(p_inf->rt_lbl.dpc);
    p_data_val->si  = p_inf->rt_lbl.si;
    p_data_val->ni  = p_inf->rt_lbl.ni;
    p_data_val->mp  = p_inf->rt_lbl.mp;
    p_data_val->sls = p_inf->rt_lbl.sls;
    p_data_val->prot_data = (m3_u8*)&(p_tlv->val[3]);
    M3_MEMCPY(p_data_val->prot_data, p_inf->p_prot_data, p_inf->prot_data_len);
    p_hdr->msg_len += p_tlv->len;
    M3_ALIGN_4(p_hdr->msg_len);
    p_tlv->len    = M3_HTONS(p_tlv->len);
    if (M3_MAX_U32 != p_inf->crn_id) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_CRN_ID;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->crn_id);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeDUNA(m3_duna_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_DUNA;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++)
        {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = p_inf->num_pc * 4 + 4;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeDAVA(m3_dava_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_DAVA;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }

    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = p_inf->num_pc * 4 + 4;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeDAUD(m3_daud_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_DAUD;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = p_inf->num_pc * 4 + 4;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeSCON(m3_scon_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_SCON;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = p_inf->num_pc * 4 + 4;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);
    
    if (M3_MAX_U32 != p_inf->crnd_dpc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_CRND_DPC;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->crnd_dpc & 0x00FFFFFF);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (M3_MAX_U8 != p_inf->cong_level) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_CONG_IND;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->cong_level & 0x000000FF);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeDUPU(m3_dupu_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        *p_u16val;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_DUPU;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = 8;
    p_tlv->val[0] = M3_HTONL(p_inf->pc);
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len   = M3_HTONS(p_tlv->len);

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag    = M3_TAG_USR_CAUSE;
    p_tlv->len    = 8;
    p_u16val = (m3_u16*)p_tlv->val;
    p_u16val[0] = M3_HTONS(p_inf->cause);
    p_u16val[1] = M3_HTONS(p_inf->user);
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeDRST(m3_drst_inf_t    *p_inf,
                       m3_u32           *p_len,
                       m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_SSNM;
    p_hdr->msg_type   = M3_MSG_TYPE_DRST;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->nw_app) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag   = M3_TAG_AFF_PC;
    p_tlv->len   = p_inf->num_pc * 4 + 4;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    if (0 != p_inf->info_len) {
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeASPUP(m3_aspup_inf_t   *p_inf,
                        m3_u32           *p_len,
                        m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPUP;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->m3asp_id) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_ASP_ID;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->m3asp_id);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    if (0 != p_inf->info_len) {
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPUPACK(m3_aspup_ack_inf_t   *p_inf,
                           m3_u32               *p_len,
                           m3_u8                *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPUP_ACK;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPDN(m3_aspdn_inf_t   *p_inf,
                        m3_u32           *p_len,
                        m3_u8            *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPDN;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPDNACK(m3_aspdn_ack_inf_t   *p_inf,
                           m3_u32               *p_len,
                           m3_u8                *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPDN_ACK;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeHBEAT(m3_hbeat_inf_t       *p_inf,
                        m3_u32               *p_len,
                        m3_u8                *p_buf)
{
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_BEAT;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->hbeat_data_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_HBEAT_DATA;
        p_tlv->len    = p_inf->hbeat_data_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_hbeat_data, p_inf->hbeat_data_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeHBEATACK(m3_hbeat_ack_inf_t   *p_inf,
                           m3_u32               *p_len,
                           m3_u8                *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPSM;
    p_hdr->msg_type   = M3_MSG_TYPE_BEAT_ACK;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    if (0 != p_inf->hbeat_data_len) {
        p_tlv->tag    = M3_TAG_HBEAT_DATA;
        p_tlv->len    = p_inf->hbeat_data_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_hbeat_data, p_inf->hbeat_data_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeREGREQ(m3_reg_req_inf_t      *p_inf,
                         m3_u32                *p_len,
                         m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u8         *p_u8buf;
    m3_u16        idx, s_idx, buf_idx, dpc_idx;
    m3_u16        *p_rklen;
    m3_u16        *p_u16val, tlv_len;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_RKM;
    p_hdr->msg_type   = M3_MSG_TYPE_REG_REQ;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    for (idx = 0; idx < p_inf->num_rk; idx++) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RKEY;
        p_tlv->len    = p_hdr->msg_len;
        p_hdr->msg_len += 4;
        p_rklen = &p_tlv->len;

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_LRK_ID;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->rk_list[idx].lrk_id);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);

        if (M3_MAX_U32 != p_inf->rk_list[idx].rk_inf.trfmode) {
            p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
            p_tlv->tag    = M3_TAG_TRFMODE;
            p_tlv->len    = 8;
            p_tlv->val[0] = M3_HTONL(p_inf->rk_list[idx].rk_inf.trfmode);
            p_hdr->msg_len += p_tlv->len;
            p_tlv->len    = M3_HTONS(p_tlv->len);
        }

        if (M3_MAX_U32 != p_inf->rk_list[idx].rk_inf.nw_app) {
            p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
            p_tlv->tag    = M3_TAG_NW_APP;
            p_tlv->len    = 8;
            p_tlv->val[0] = M3_HTONL(p_inf->rk_list[idx].rk_inf.nw_app);
            p_hdr->msg_len += p_tlv->len;
            p_tlv->len    = M3_HTONS(p_tlv->len);
        }

        for (dpc_idx = 0; dpc_idx < p_inf->rk_list[idx].rk_inf.num_rtparam; dpc_idx++) {
            p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
            p_tlv->tag    = M3_TAG_DPC;
            p_tlv->len    = 8;
            p_tlv->val[0] = M3_HTONL((p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].dpc) & 0x00FFFFFF);
            p_hdr->msg_len += p_tlv->len;
            p_tlv->len    = M3_HTONS(p_tlv->len);

            if (0 != p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_si) {
                p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
                p_tlv->tag = M3_TAG_SI;
                p_tlv->len = 4 + p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_si;
                //M3_ALIGN_4(p_tlv->len);
                p_u8buf = (m3_u8 *)p_tlv->val;
                for (s_idx = 0; s_idx < p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_si; s_idx++){
                    p_u8buf[s_idx] = p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].si_list[s_idx];
                }
                p_u8buf[s_idx++] = 0;  /* padding, not more than 3 bytes */
                p_u8buf[s_idx++] = 0;
                p_u8buf[s_idx]   = 0;
                tlv_len = p_tlv->len;
                p_hdr->msg_len += M3_ALIGN_4(tlv_len);
                p_tlv->len    = M3_HTONS(p_tlv->len);
            }

            if (0 != p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_opc) {
                p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
                p_tlv->tag    = M3_TAG_OPC_LIST;
                p_tlv->len    = p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_opc * 4 + 4;
                for (s_idx=0; s_idx<p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_opc; s_idx++) {
                    p_tlv->val[s_idx] = M3_HTONL((p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].opc_list[s_idx]) & 0x00FFFFFF);
                }
                p_hdr->msg_len += p_tlv->len;
                p_tlv->len    = M3_HTONS(p_tlv->len);
            }

            if (0 != p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_ckt_range)
            {
                p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
                p_tlv->tag    = M3_TAG_CKT_RANGE;
                p_tlv->len    = p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_ckt_range * 8 + 4;
                buf_idx       = 0;
                for (s_idx = 0; s_idx < p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].num_ckt_range;
                     s_idx++) {
                    p_tlv->val[buf_idx++] = M3_HTONL((p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].ckt_range[s_idx].opc) & 0x00FFFFFF);
                    p_u16val = (m3_u16*)&p_tlv->val[buf_idx];
                    p_u16val[0] = M3_HTONS(p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].ckt_range[s_idx].lcic);
                    p_u16val[1] = M3_HTONS(p_inf->rk_list[idx].rk_inf.rtparam[dpc_idx].ckt_range[s_idx].ucic);
                    buf_idx += 1;
                }
                p_hdr->msg_len += p_tlv->len;
                p_tlv->len    = M3_HTONS(p_tlv->len);
            }
        }
        *p_rklen = M3_HTONS(p_hdr->msg_len - *p_rklen);
    }

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeREGRSP(m3_reg_rsp_inf_t      *p_inf,
                         m3_u32                *p_len,
                         m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;
    m3_u16        *p_rrlen;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_RKM;
    p_hdr->msg_type   = M3_MSG_TYPE_REG_RSP;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    for (idx = 0; idx < p_inf->num_reg_result; idx++) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_REG_RESULT;
        p_tlv->len    = p_hdr->msg_len;
        p_hdr->msg_len += 4;
        p_rrlen = &p_tlv->len;

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_LRK_ID;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->rr_list[idx].lrk_id);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_REG_STATUS;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->rr_list[idx].reg_status);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->rr_list[idx].rtctx);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        
        *p_rrlen = M3_HTONS(p_hdr->msg_len - *p_rrlen);
    }

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeDREGREQ(m3_dreg_req_inf_t     *p_inf,
                          m3_u32                *p_len,
                          m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr = M3_NULL;
    m3_msg_tlv_t  *p_tlv = M3_NULL;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_RKM;
    p_hdr->msg_type   = M3_MSG_TYPE_DEREG_REQ;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag    = M3_TAG_RT_CTX;
    p_tlv->len    = p_inf->num_rc * 4 + 4;
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
    }
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeDREGRSP(m3_dreg_rsp_inf_t     *p_inf,
                          m3_u32                *p_len,
                          m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;
    m3_u16        *p_drrlen;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_RKM;
    p_hdr->msg_type   = M3_MSG_TYPE_DEREG_RSP;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    for (idx = 0; idx < p_inf->num_dreg_result; idx++) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_DEREG_RESULT;
        p_tlv->len    = 4;
        p_hdr->msg_len += 4;
        p_drrlen = &p_tlv->len;

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->drr_list[idx].rtctx);
        p_hdr->msg_len += p_tlv->len;
        *p_drrlen += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);

        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_DEREG_STATUS;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->drr_list[idx].dreg_status);
        p_hdr->msg_len += p_tlv->len;
        *p_drrlen += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);

        *p_drrlen = M3_HTONS((*p_drrlen));
    }

    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPAC(m3_aspac_inf_t        *p_inf,
                        m3_u32                *p_len,
                        m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPTM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPAC;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;
    if (M3_MAX_U32 != p_inf->trfmode) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_TRFMODE;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->trfmode);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}


m3_s32  m3uaEncodeASPACACK(m3_aspac_ack_inf_t    *p_inf,
                           m3_u32                *p_len,
                           m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPTM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPAC_ACK;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (M3_MAX_U32 != p_inf->trfmode) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_TRFMODE;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->trfmode);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++)
        {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPIA(m3_aspia_inf_t        *p_inf,
                        m3_u32                *p_len,
                        m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPTM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPIA;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeASPIAACK(m3_aspia_ack_inf_t    *p_inf,
                           m3_u32                *p_len,
                           m3_u8                 *p_buf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_ASPTM;
    p_hdr->msg_type   = M3_MSG_TYPE_ASPIA_ACK;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++)
        {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeERROR(m3_error_inf_t        *p_inf,
                        m3_u32                *p_len,
                        m3_u8                 *p_buf)
{
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_MGMT;
    p_hdr->msg_type   = M3_MSG_TYPE_ERR;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag    = M3_TAG_ERR_CODE;
    p_tlv->len    = 8;
    p_tlv->val[0] = M3_HTONL(p_inf->err_code);
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len = M3_HTONS(p_tlv->len);
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_pc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag   = M3_TAG_AFF_PC;
        p_tlv->len   = p_inf->num_pc * 4 + 4;
        for (idx = 0; idx < p_inf->num_pc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->pc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (M3_MAX_U32 != p_inf->nw_app)
    {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_NW_APP;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->nw_app);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->diag_len)
    {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_DIAG_INFO;
        p_tlv->len    = p_inf->diag_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_diag_inf, p_inf->diag_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

m3_s32  m3uaEncodeNTFY(m3_ntfy_inf_t         *p_inf,
                       m3_u32                *p_len,
                       m3_u8                 *p_buf)
{
    m3_msg_hdr_t  *p_hdr;
    m3_msg_tlv_t  *p_tlv;
    m3_u16        idx;
    m3_u16        *p_u16val;

    p_hdr = (m3_msg_hdr_t*)p_buf;
    p_hdr->version    = 1;
    p_hdr->resv       = 0;
    p_hdr->msg_class  = M3_MSG_CLASS_MGMT;
    p_hdr->msg_type   = M3_MSG_TYPE_NTFY;
    p_hdr->msg_len    = M3_MSG_HDR_LEN;

    p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
    p_tlv->tag    = M3_TAG_STATUS;
    p_tlv->len    = 8;
    p_u16val = (m3_u16*)p_tlv->val;
    p_u16val[0] = M3_HTONS(p_inf->status_type);
    p_u16val[1] = M3_HTONS(p_inf->status_inf);
    p_hdr->msg_len += p_tlv->len;
    p_tlv->len    = M3_HTONS(p_tlv->len);

    if (M3_MAX_U32 != p_inf->m3asp_id) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_ASP_ID;
        p_tlv->len    = 8;
        p_tlv->val[0] = M3_HTONL(p_inf->m3asp_id);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->num_rc) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_RT_CTX;
        p_tlv->len    = p_inf->num_rc * 4 + 4;
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            p_tlv->val[idx] = M3_HTONL(p_inf->rc_list[idx]);
        }
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
    }
    if (0 != p_inf->info_len) {
        p_tlv = (m3_msg_tlv_t*)(p_buf + p_hdr->msg_len);
        p_tlv->tag    = M3_TAG_INFO_STR;
        p_tlv->len    = p_inf->info_len + 4;
        M3_MEMCPY(p_tlv->val, p_inf->p_info_str, p_inf->info_len);
        p_hdr->msg_len += p_tlv->len;
        p_tlv->len    = M3_HTONS(p_tlv->len);
        M3_ALIGN_4(p_hdr->msg_len);
    }
    *p_len = p_hdr->msg_len;
    p_hdr->msg_len = M3_HTONL(p_hdr->msg_len);
    return 0;
}

