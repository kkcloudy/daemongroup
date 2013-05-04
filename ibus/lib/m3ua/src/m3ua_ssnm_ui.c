/****************************************************************************
** Description:
** code for ssnm procedures invoked by user i/f.
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

#include <m3ua_ssnm_ui.h>

m3_s32 m3ua_EPAUSE(m3_conn_inf_t *pconn,
                   m3ua_pause_t  *pInf)
{
    m3_u16           idx;
    m3_duna_inf_t    duna_inf;
    m3_u8            *p_msg = M3_NULL;
    m3_u32           msglen = 0;
    m3_r_as_inf_t    *pas;
    m3_sgp_inf_t     *psgp = M3_SGP_TABLE(pconn->l_sp_id);

    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DUNA_SIZE, M3_SSNM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    duna_inf.num_rc = 0;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pas = M3_R_AS_TABLE(idx);
        if (M3_TRUE == psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id] &&
            M3_ASP_ACTIVE == pconn->r_sp_st[idx]     &&
            ((pInf->nw_app == M3_MAX_U32) || (pInf->nw_app == pas->rkey.nw_app))) {
            duna_inf.rc_list[duna_inf.num_rc++] = m3uaGetRCFromRAS(pconn, idx);
        }
    }
    if (0 == duna_inf.num_rc) {
        return 0;
    }

    /* JULY 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX)
        duna_inf.num_rc = 0;

    duna_inf.nw_app = pInf->nw_app;
    duna_inf.info_len = pInf->info_len;
    duna_inf.p_info_str = pInf->p_info_str;
    duna_inf.num_pc = pInf->num_pc;
    for (idx = 0; idx < pInf->num_pc; idx++) {
        duna_inf.pc_list[idx] = pInf->pc_list[idx];
    }
    m3uaEncodeDUNA(&duna_inf, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_SSNM_POOL);
    return 0;
}

m3_s32    m3ua_ERESUME(m3_conn_inf_t *pconn,
                       m3ua_resume_t *pInf)
{   
    m3_u16           idx;
    m3_dava_inf_t    dava_inf;
    m3_u8            *p_msg = M3_NULL;
    m3_u32           msglen = 0;
    m3_r_as_inf_t    *pas;
    m3_sgp_inf_t     *psgp = M3_SGP_TABLE(pconn->l_sp_id);
    
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DAVA_SIZE, M3_SSNM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    dava_inf.num_rc = 0;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pas = M3_R_AS_TABLE(idx);
        if (M3_TRUE == psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id] &&
            M3_ASP_ACTIVE == pconn->r_sp_st[idx]         &&
            ((pInf->nw_app == M3_MAX_U32) || (pInf->nw_app == pas->rkey.nw_app))) {
            dava_inf.rc_list[dava_inf.num_rc++] = m3uaGetRCFromRAS(pconn, idx);
        }
    }
    if (0 == dava_inf.num_rc) {
        return 0;
    }

    /* JULY 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX)
        dava_inf.num_rc = 0;

    dava_inf.nw_app = pInf->nw_app;
    dava_inf.info_len = pInf->info_len;
    dava_inf.p_info_str = pInf->p_info_str;
    dava_inf.num_pc = pInf->num_pc;
    for (idx = 0; idx < pInf->num_pc; idx++) {
        dava_inf.pc_list[idx] = pInf->pc_list[idx];
    }
    m3uaEncodeDAVA(&dava_inf, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_SSNM_POOL);
    return 0;
}

m3_s32    m3ua_ESTATUS(m3_conn_inf_t *pconn,
                       m3ua_status_t *pInf)
{
    m3_u16           idx;
    m3_u8            *p_msg = M3_NULL;
    m3_u32           msglen = 0;
    m3_r_as_inf_t    *pas;
    m3_sgp_inf_t     *psgp = M3_SGP_TABLE(pconn->l_sp_id);

    switch(pInf->cause) {
        case M3_CAUSE_UNKNOWN:
        case M3_CAUSE_UNEQUPPD_RMT_USR:
        case M3_CAUSE_INACCESS_RMT_USR: {
            m3_dupu_inf_t    dupu_inf;

            p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DUPU_SIZE, M3_SSNM_POOL);
            if (M3_NULL == p_msg) {
                M3ERRNO(EM3_MALLOC_FAIL);
                return -1;
            }
            dupu_inf.num_rc = 0;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                pas = M3_R_AS_TABLE(idx);
                if (M3_TRUE == psgp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id] &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[idx]        &&
                    ((M3_MAX_U32 == pInf->status_inf.upu_inf.nw_app) ||
                     (pInf->status_inf.upu_inf.nw_app == pas->rkey.nw_app))) {
                    dupu_inf.rc_list[dupu_inf.num_rc++] = m3uaGetRCFromRAS(pconn, idx);
                }
            }
            if (0 == dupu_inf.num_rc) {
                return 0;
            }

            /* JULY 2010 */
            if (pconn->opt & M3_CONN_EXCL_RTCTX)
                dupu_inf.num_rc = 0;

            dupu_inf.nw_app = pInf->status_inf.upu_inf.nw_app;
            dupu_inf.pc = pInf->status_inf.upu_inf.ptcode;
            dupu_inf.cause = pInf->cause;
            dupu_inf.user = pInf->status_inf.upu_inf.user;
            dupu_inf.info_len = pInf->status_inf.upu_inf.info_len;
            dupu_inf.p_info_str = pInf->status_inf.upu_inf.p_info_str;
            m3uaEncodeDUPU(&dupu_inf, &msglen, p_msg);
            break;
        }
        case M3_CAUSE_CONG: {
            m3_scon_inf_t    scon_inf;

            p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_SCON_SIZE, M3_SSNM_POOL);
            if (M3_NULL == p_msg) {
                M3ERRNO(EM3_MALLOC_FAIL);
                return -1;
            }
            scon_inf.num_rc = 0;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                pas = M3_R_AS_TABLE(idx);
                if (M3_TRUE == psgp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id] &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[idx]        &&
                    ((M3_MAX_U32 == pInf->status_inf.cong_inf.nw_app) ||
                     (pInf->status_inf.cong_inf.nw_app == pas->rkey.nw_app))) {
                    scon_inf.rc_list[scon_inf.num_rc++] = m3uaGetRCFromRAS(pconn, idx);
                }
            }
            if (0 == scon_inf.num_rc) {    
                return 0;    
            }    

            /* JULY 2010 */
            if (pconn->opt & M3_CONN_EXCL_RTCTX)
                scon_inf.num_rc = 0;

            scon_inf.nw_app = pInf->status_inf.cong_inf.nw_app;
            scon_inf.num_pc = pInf->status_inf.cong_inf.num_pc;
            for (idx = 0; idx < pInf->status_inf.cong_inf.num_pc; idx++) {
                scon_inf.pc_list[idx] = pInf->status_inf.cong_inf.pc_list[idx];
            }
            scon_inf.crnd_dpc = pInf->status_inf.cong_inf.crnd_dpc;
            scon_inf.cong_level = pInf->status_inf.cong_inf.cong_level;
            scon_inf.info_len = pInf->status_inf.cong_inf.info_len;
            scon_inf.p_info_str = pInf->status_inf.cong_inf.p_info_str;
            m3uaEncodeSCON(&scon_inf, &msglen, p_msg);
            break;
        }
        case M3_CAUSE_DRST: {
            m3_drst_inf_t    drst_inf;

            p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DRST_SIZE, M3_SSNM_POOL);
            if (M3_NULL == p_msg) {
                M3ERRNO(EM3_MALLOC_FAIL);
                return -1;
            }
            drst_inf.num_rc = 0;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                pas = M3_R_AS_TABLE(idx);
                if (M3_TRUE == psgp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id] &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[idx]         &&
                    ((M3_MAX_U32 == pInf->status_inf.drst_inf.nw_app) || 
                     (pInf->status_inf.drst_inf.nw_app == pas->rkey.nw_app))) {
                    drst_inf.rc_list[drst_inf.num_rc++] = m3uaGetRCFromRAS(pconn, idx);
                }
            }
            if (0 == drst_inf.num_rc) {    
                return 0;    
            }    

            /* JULY 2010 */
            if (pconn->opt & M3_CONN_EXCL_RTCTX)
                drst_inf.num_rc = 0;

            drst_inf.nw_app = pInf->status_inf.drst_inf.nw_app;
            drst_inf.info_len = pInf->status_inf.drst_inf.info_len;
            drst_inf.p_info_str = pInf->status_inf.drst_inf.p_info_str;
            drst_inf.num_pc = pInf->status_inf.drst_inf.num_pc;
            for (idx = 0; idx < pInf->status_inf.drst_inf.num_pc; idx++) {
                drst_inf.pc_list[idx] = pInf->status_inf.drst_inf.pc_list[idx];
            }
            m3uaEncodeDRST(&drst_inf, &msglen, p_msg);
            break;
        }
    }

    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_SSNM_POOL);
    return 0;
}

