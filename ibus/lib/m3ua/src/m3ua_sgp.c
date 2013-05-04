/****************************************************************************
** Description:
** Code for provisioning an SGP.
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

#include <m3ua_sgp.h>

m3_s32 m3ua_sgp(m3_u32        sgp_id,
                m3_u8         oprn,
                m3ua_sgp_t    *p_inf)
{
    m3_sgp_inf_t    *pSgp;
    m3_s32          ret;

    if (M3_FALSE == M3_SGPID_VALID(sgp_id)) {
        M3ERRNO(EM3_INV_SGPID);
        return -1;
    }

    pSgp = M3_SGP_TABLE(sgp_id);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddSGP(pSgp, p_inf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteSGP(pSgp, p_inf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetSGPConf(pSgp, p_inf);
            break;
        }
        case M3_MODIFY: {
            ret = m3uaModifySGPConf(pSgp, p_inf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            break;
        }
    }
    return ret;
}

m3_s32 m3uaAddSGP(m3_sgp_inf_t    *pSgp,
                  m3ua_sgp_t      *p_inf)
{
    m3_u16     idx, idx2;
    m3_r_as_inf_t    *pras;
    m3_r_asp_inf_t   *prasp;

    if (M3_NULL == p_inf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pSgp->e_state) {
        M3ERRNO(EM3_SGPID_ALRDY);
        return -1;
    }
    if (M3_FALSE == m3uaAssertNwApp(p_inf->add.info.def_nwapp)) {
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    pSgp->e_state    = M3_TRUE;
    pSgp->sctp_ep_id = p_inf->add.info.sctp_ep_id;
    pSgp->def_nwapp = p_inf->add.info.def_nwapp;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pSgp->r_as_inf[idx].state            = M3_AS_DOWN;
        pSgp->r_as_inf[idx].num_asp_inactive = 0;
        pSgp->r_as_inf[idx].num_asp_active   = 0;
        pras = M3_R_AS_TABLE(idx);
        if (M3_TRUE == pras->e_state) {
            for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {//test
                if (M3_TRUE==p_inf->add.info.r_as_inf[idx].asp_list[idx2])
                {
                    prasp = M3_R_ASP_TABLE(idx2);
//iu_log_debug("m3uaAddSGP idx %d , idx2 %d\n", idx, idx2);					
                    
		    if (M3_FALSE == prasp->e_state) {
                        M3ERRNO(EM3_INV_ASPLIST);
                        return -1;
                    }
                }
                pSgp->r_as_inf[idx].asp_list[idx2] =
                    p_inf->add.info.r_as_inf[idx].asp_list[idx2];
            }
        }
    }
    pSgp->user_id    = M3_MAX_U16;
    for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
        pSgp->r_asp_inf[idx].conn_id = M3_MAX_U32;
    }
    return 0;
}

m3_s32 m3uaDeleteSGP(m3_sgp_inf_t    *pSgp,
                     m3ua_sgp_t      *p_inf)
{
    m3_u16         idx, asp_idx;
    m3_pd_buf_t    *p_pdbuf = M3_NULL;
    m3_u32         sgp_id = 0x00010000 | pSgp->sgp_id;

    if (M3_FALSE == pSgp->e_state) {
        M3ERRNO(EM3_INV_SGPID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_CONN; idx++) {
        if ((M3_TRUE == (M3_CONN_TABLE(idx))->e_state)      &&
            (sgp_id == (M3_CONN_TABLE(idx))->l_sp_id)) {
            M3ERRNO(EM3_CONN_EXIST);
            return -1;
        }
    }
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pSgp->r_as_inf[idx].state = M3_AS_DOWN;
        pSgp->r_as_inf[idx].num_asp_inactive = 0;
        pSgp->r_as_inf[idx].num_asp_active = 0;
        /* release all the pending buffers */
        p_pdbuf = pSgp->r_as_inf[idx].pd_q_inf.pd_q_head;
        while (M3_NULL != p_pdbuf) {
            M3_MSG_FREE(p_pdbuf->p_buf, M3_TXR_POOL);
            p_pdbuf = p_pdbuf->p_next;
            M3_MSG_FREE(p_pdbuf, M3_PD_BUF_POOL);
            p_pdbuf = M3_NULL;
        }
        for (asp_idx = 0; asp_idx < M3_MAX_R_ASP; asp_idx++) {
            pSgp->r_as_inf[idx].rc_inf[asp_idx].rtctx = M3_MAX_U32;
            pSgp->r_as_inf[idx].rc_inf[asp_idx].dyn_reg_state = M3_RK_STATIC;
        }
    }
    for (idx = 0; idx < M3_MAX_USR; idx++) {
        if (M3_TRUE == (M3_USR_TABLE(idx))->e_state           &&
            sgp_id == (M3_USR_TABLE(idx))->sp_id) {
            m3uaDeleteUser(M3_USR_TABLE(idx));
        }
    }
    pSgp->e_state = M3_FALSE;
    return 0;
}

m3_s32 m3uaGetSGPConf(m3_sgp_inf_t    *pSgp,
                      m3ua_sgp_t      *p_inf)
{
    if (M3_NULL == p_inf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pSgp->e_state) {
        M3ERRNO(EM3_INV_SGPID);
        return -1;
    }
    p_inf->get.info.sctp_ep_id = pSgp->sctp_ep_id;
    p_inf->get.info.def_nwapp = pSgp->def_nwapp;
    return 0;
}

m3_s32 m3uaModifySGPConf(m3_sgp_inf_t    *pSgp,
                         m3ua_sgp_t      *p_inf)
{
    m3_r_as_inf_t      *pras;
    m3_r_asp_inf_t     *prasp;
    m3_u32             conn_id;
    m3_conn_inf_t      *pconn;

    if (M3_NULL == p_inf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pSgp->e_state) {
        M3ERRNO(EM3_INV_SGPID);
        return -1;
    }
    switch(p_inf->modify.confname)
    {
        case M3_SGP_NWAPP: {
            if (M3_FALSE == m3uaAssertNwApp(p_inf->modify.info.def_nwapp)) {
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            pSgp->def_nwapp = p_inf->modify.info.def_nwapp;
            break;
        }
        case M3_SGP_ADD_R_ASP: {
            if (M3_FALSE == M3_R_ASID_VALID(p_inf->modify.ras_id)) {
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            pras = M3_R_AS_TABLE(p_inf->modify.ras_id);
            if (M3_FALSE == pras->e_state) {
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            if (M3_FALSE == M3_R_ASPID_VALID(p_inf->modify.rasp_id)) {
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            prasp = M3_R_ASP_TABLE(p_inf->modify.rasp_id);
            if (M3_FALSE == prasp->e_state) {
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            if (M3_TRUE == pSgp->r_as_inf[(m3_u16)p_inf->modify.ras_id].\
                asp_list[(m3_u16)p_inf->modify.rasp_id]) {
                M3ERRNO(EM3_R_ASP_ALRDY_SERV);
                return -1;
            }
            pSgp->r_as_inf[(m3_u16)p_inf->modify.ras_id].\
                asp_list[(m3_u16)p_inf->modify.rasp_id] = M3_TRUE;
            if (M3_MAX_U32 !=
                pSgp->r_asp_inf[(m3_u16)p_inf->modify.rasp_id].conn_id) {
                conn_id = pSgp->r_asp_inf[(m3_u16)p_inf->modify.rasp_id].conn_id;
                pconn = M3_CONN_TABLE(conn_id);
                if (M3_ASP_INACTIVE == pconn->r_sp_g_st        ||
                    M3_ASP_ACTIVE == pconn->r_sp_g_st) {
                    m3uaRAS(M3_CONN_TABLE(conn_id), p_inf->modify.ras_id,
                        M3_AS_EVT_ASPIA, M3_NULL);
                }
            }
            break;
        }
        case M3_SGP_DEL_R_ASP: {
            if (M3_FALSE == M3_R_ASID_VALID(p_inf->modify.ras_id)) {
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }     
            pras = M3_R_AS_TABLE(p_inf->modify.ras_id);
            if (M3_FALSE == pras->e_state) {
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            if (M3_FALSE == M3_R_ASPID_VALID(p_inf->modify.rasp_id)) {
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            prasp = M3_R_ASP_TABLE(p_inf->modify.rasp_id);
            if (M3_FALSE == prasp->e_state) {
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            if (M3_FALSE == pSgp->r_as_inf[(m3_u16)p_inf->modify.ras_id].\
                asp_list[(m3_u16)p_inf->modify.rasp_id]) {
                M3ERRNO(EM3_R_ASP_NOT_SERV);
                return -1;
            }
            if (M3_MAX_U32 !=
                pSgp->r_asp_inf[(m3_u16)p_inf->modify.rasp_id].conn_id) {
                conn_id = pSgp->r_asp_inf[(m3_u16)p_inf->modify.rasp_id].conn_id;
                pconn = M3_CONN_TABLE(conn_id);
                m3uaRAS(M3_CONN_TABLE(conn_id), p_inf->modify.ras_id,
                    M3_AS_EVT_ASPDN, M3_NULL);
            }
            pSgp->r_as_inf[(m3_u16)p_inf->modify.ras_id].\
                asp_list[(m3_u16)p_inf->modify.rasp_id] = M3_FALSE;
            break;
        }
        default: {
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}

