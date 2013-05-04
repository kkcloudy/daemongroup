/****************************************************************************
** Description:
** Code for provisioning an ASP and invoking ASPM procedures.
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

#include <m3ua_asp.h>

m3_s32 m3ua_asp(m3_u32        aspId,
                m3_u8         oprn,
                m3ua_asp_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t  *pAsp;
    m3_s32        ret;
    iu_log_debug("aspId = %d\n",aspId);
    if (M3_FALSE == M3_ASPID_VALID(aspId)) {
        M3TRACE(m3uaErrorTrace, ("Invalid ASP Identifier %u passed", aspId));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pAsp = M3_ASP_TABLE(aspId);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddASP(pAsp, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteASP(pAsp, pInf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetASPConf(pAsp, pInf);
            break;
        }
        case M3_MODIFY: {
            ret = m3uaModifyASPConf(pAsp, pInf);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation code %u specified", (m3_u32)oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3uaAddASP(m3_asp_inf_t  *pAsp,
                  m3ua_asp_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_as_inf_t     *pAs;
    m3_r_as_inf_t   *pRAs;
    m3_r_asp_inf_t  *pRAsp;
    m3_asp_inf_t    *pAsp_sec;
    m3_u16          idx, idx2;

    M3TRACE(m3uaConfigTrace, ("Adding ASP %u", pAsp->asp_id));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("Null configuration information passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pAsp->e_state) {
        M3TRACE(m3uaErrorTrace, ("ASP %u already added", pAsp->asp_id));
        M3ERRNO(EM3_ASPID_ALRDY);
        return -1;
    }
    if (M3_MAX_U32 == pInf->add.info.m3asp_id) {
        M3TRACE(m3uaErrorTrace, ("Invalid m3aspId %u", pInf->add.info.m3asp_id));
        M3ERRNO(EM3_INV_M3ASPID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_ASP; idx++) {
        pAsp_sec = M3_ASP_TABLE(idx);
        if (M3_TRUE == pAsp_sec->e_state && pInf->add.info.m3asp_id ==
            pAsp_sec->m3asp_id) {
            iu_log_debug("idx = %d\n",idx);
            M3TRACE(m3uaErrorTrace, ("Duplicate m3aspId %u", pInf->add.info.m3asp_id));
            M3ERRNO(EM3_M3ASPID_ALRDY);
            return -1;
        }
    }
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        if (M3_TRUE == pInf->add.info.as_list[idx]) {
            pAs = M3_AS_TABLE(idx);
            if (M3_FALSE == pAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned AS:%u present in ASP configuration", pAs->as_id));
                M3ERRNO(EM3_INV_ASLIST);
                return -1;
            }
        }
        pAsp->as_list[idx] = pInf->add.info.as_list[idx];
        iu_log_debug("pAsp->as_list[%d] = pInf->add.info.as_list[%d] = %d\n", idx, idx, pAsp->as_list[idx]);
    }
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pAsp->r_as_inf[idx].state = M3_AS_DOWN;
        pAsp->r_as_inf[idx].num_asp_inactive = 0;
        pAsp->r_as_inf[idx].num_asp_active = 0;
        pRAs = M3_R_AS_TABLE(idx);
        
        if (M3_TRUE == pRAs->e_state) {
            iu_log_debug("idx = %d\n",idx);
            for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
                if (M3_TRUE==pInf->add.info.r_as_inf[idx].asp_list[idx2])
                {  
                    pAsp->r_as_inf[idx].asp_list[idx2] = pInf->add.info.r_as_inf[idx].asp_list[idx2];
                    iu_log_debug("pAsp->r_as_inf[%d].asp_list[%d] = %d\n",idx,idx2,pAsp->r_as_inf[idx].asp_list[idx2]);
                    pRAsp = M3_R_ASP_TABLE(idx2);
                    if (M3_FALSE == pRAsp->e_state) {
                        M3TRACE(m3uaErrorTrace, ("Non-provisioned remote ASP:%u present in ASP configuration", pRAsp->asp_id));
                        M3ERRNO(EM3_INV_ASPLIST);
                        return -1;
                    }
                }
               
            }
        }
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->add.info.def_nwapp)) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned Default Network Appearance %u", pInf->add.info.def_nwapp));
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    pAsp->e_state                    = M3_TRUE;
    pAsp->m3asp_id                   = pInf->add.info.m3asp_id;
    pAsp->sctp_ep_id                 = pInf->add.info.sctp_ep_id;
    pAsp->def_nwapp                  = pInf->add.info.def_nwapp;
    iu_log_debug("pAsp->m3asp_id = %u, pAsp->sctp_ep_id = %u, pAsp->def_nwapp = %u\n", pAsp->m3asp_id, pAsp->sctp_ep_id, pAsp->def_nwapp);
    M3TRACE(m3uaConfigTrace, ("ASP:%u added successfully", pAsp->asp_id));
    return 0;
}

m3_s32 m3uaDeleteASP(m3_asp_inf_t    *pAsp,
                     m3ua_asp_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_pd_buf_t     *pPdBuf = M3_NULL;
    m3_pd_buf_t     *pNext = M3_NULL;
    m3_u16          idx, asp_idx;

    M3TRACE(m3uaConfigTrace, ("Deleting ASP:%u", pAsp->asp_id));
    if (M3_FALSE == pAsp->e_state) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u attempted to delete", pAsp->asp_id));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_CONN; idx++) {
        if ((M3_TRUE == (M3_CONN_TABLE(idx))->e_state)      && 
            (pAsp->asp_id == (M3_CONN_TABLE(idx))->l_sp_id)) {
            M3TRACE(m3uaErrorTrace, ("ASP:%u has connection:%u, delete connection first", pAsp->asp_id, idx));
            M3ERRNO(EM3_CONN_EXIST);
            return -1;
        }
    }
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pAsp->r_as_inf[idx].state = M3_AS_DOWN;
        pAsp->r_as_inf[idx].num_asp_inactive = 0;
        pAsp->r_as_inf[idx].num_asp_active = 0;
        pPdBuf = pAsp->r_as_inf[idx].pd_q_inf.pd_q_head;
        while (M3_NULL != pPdBuf) {
            M3_MSG_FREE(pPdBuf->p_buf, M3_TXR_POOL);
            pNext = pPdBuf->p_next;
            M3_MSG_FREE(pPdBuf, M3_PD_BUF_POOL);
            pPdBuf = pNext;
        }
        for (asp_idx = 0; asp_idx < M3_MAX_R_ASP; asp_idx++) {
            pAsp->r_as_inf[idx].rc_inf[asp_idx].rtctx = M3_MAX_U32;
            pAsp->r_as_inf[idx].rc_inf[asp_idx].dyn_reg_state = M3_RK_STATIC;
        }
    }
    for (idx = 0; idx < M3_MAX_USR; idx++) {
        if (M3_TRUE == (M3_USR_TABLE(idx))->e_state           &&
            pAsp->asp_id == (M3_USR_TABLE(idx))->sp_id) {
            m3uaDeleteUser(M3_USR_TABLE(idx));
        }
    }
    pAsp->e_state = M3_FALSE;
    M3TRACE(m3uaConfigTrace, ("ASP:%u deleted successfully", pAsp->asp_id));
    return 0;
}

m3_s32 m3uaGetASPConf(m3_asp_inf_t    *pAsp,
                      m3ua_asp_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx, idx2;

    M3TRACE(m3uaConfigTrace, ("Reading ASP:%u configuration", pAsp->asp_id));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("Null container passed for reading information"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAsp->e_state) {
        M3TRACE(m3uaErrorTrace, ("Reading configuration of Non-provisioned ASP:%u", pAsp->asp_id));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pInf->get.info.m3asp_id = pAsp->m3asp_id;
    pInf->get.info.sctp_ep_id = pAsp->sctp_ep_id;
    for (idx = 0; idx < M3_MAX_AS; idx++)
        pInf->get.info.as_list[idx] = pAsp->as_list[idx];
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
            pInf->get.info.r_as_inf[idx].asp_list[idx2] =
                pAsp->r_as_inf[idx].asp_list[idx2];
        }
    }
    pInf->get.info.def_nwapp = pAsp->def_nwapp;
    M3TRACE(m3uaConfigTrace, ("Configuration read for ASP %u", pAsp->asp_id));
    return 0;
}

m3_s32 m3uaModifyASPConf(m3_asp_inf_t    *pAsp,
                         m3ua_asp_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_r_as_inf_t    *pRAs;
    m3_r_asp_inf_t   *pRAsp;
    m3_u32           connId;
    m3_conn_inf_t    *pConn;
    m3_as_inf_t      *pAs;

    M3TRACE(m3uaConfigTrace, ("Changing configuration for ASP:%u", pAsp->asp_id));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("Null configuration information passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAsp->e_state) {
        M3TRACE(m3uaErrorTrace, ("ASP:%u not provisioned", pAsp->asp_id));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    switch(pInf->modify.confname)
    {
        case M3_ASP_M3ASP_ID: {
            M3TRACE(m3uaConfigTrace, ("Changing network ASP Id"));
            if (M3_MAX_U32 == pInf->modify.info.m3asp_id) {
                M3TRACE(m3uaErrorTrace, ("Invalid network ASP Id:%u", pInf->modify.info.m3asp_id));
                M3ERRNO(EM3_INV_M3ASPID);
                return -1;
            }
            pAsp->m3asp_id = pInf->modify.info.m3asp_id;
            break;
        }
        case M3_ASP_NWAPP: {
            M3TRACE(m3uaConfigTrace, ("Changing default network appearance"));
            if (M3_FALSE == m3uaAssertNwApp(pInf->modify.info.def_nwapp)) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned network Appeareance:%u", pInf->modify.info.def_nwapp));
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            pAsp->def_nwapp = pInf->modify.info.def_nwapp;
            break;
        }
        case M3_ASP_ADD_AS: {
            M3TRACE(m3uaConfigTrace, ("Adding AS to ASP"));
            pAs = M3_AS_TABLE(pInf->modify.as_id);
            if (M3_FALSE == pAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned AS:%u", pInf->modify.as_id));
                M3ERRNO(EM3_INV_ASID);
                return -1;
            }
            pAsp->as_list[pInf->modify.as_id] = M3_TRUE;
            break;
        }
        case M3_ASP_DEL_AS: {
            M3TRACE(m3uaConfigTrace, ("Deleting AS from ASP"));
            pAs = M3_AS_TABLE(pInf->modify.as_id);
            if (M3_FALSE == pAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned AS:%u", pInf->modify.as_id));
                M3ERRNO(EM3_INV_ASID);
                return -1;
            }
            pAsp->as_list[pInf->modify.as_id] = M3_FALSE;
            break;
        }
        case M3_ASP_ADD_R_ASP: {
            M3TRACE(m3uaConfigTrace, ("Adding Remote ASP to ASP configuration"));
            if (M3_FALSE == M3_R_ASID_VALID(pInf->modify.ras_id)) {
                M3TRACE(m3uaErrorTrace, ("Invalid remote AS:%u", pInf->modify.ras_id));
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            pRAs = M3_R_AS_TABLE(pInf->modify.ras_id);
            if (M3_FALSE == pRAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned remote AS:%u", pInf->modify.ras_id));
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            if (M3_FALSE == M3_R_ASPID_VALID(pInf->modify.rasp_id)) {
                M3TRACE(m3uaErrorTrace, ("Invalid remote ASP:%u", pInf->modify.rasp_id));
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            } 
            pRAsp = M3_R_ASP_TABLE(pInf->modify.rasp_id);
            if (M3_FALSE == pRAsp->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned remote ASP:%u", pInf->modify.rasp_id));
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            if (M3_TRUE == pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                asp_list[(m3_u16)pInf->modify.rasp_id]) {
                M3TRACE(m3uaErrorTrace, ("Remote ASP:%u already provisioned in ASP", pInf->modify.rasp_id));
                M3ERRNO(EM3_R_ASP_ALRDY_SERV);
                return -1;
            }
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                asp_list[(m3_u16)pInf->modify.rasp_id] = M3_TRUE;
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                rc_inf[(m3_u16)pInf->modify.rasp_id].dyn_entry = M3_FALSE;
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                rc_inf[(m3_u16)pInf->modify.rasp_id].dyn_reg_state = M3_RK_STATIC;
            if (M3_MAX_U32 != pAsp->r_asp_inf[(m3_u16)pInf->modify.rasp_id].conn_id) {
                connId = pAsp->r_asp_inf[(m3_u16)pInf->modify.rasp_id].conn_id;
                pConn = M3_CONN_TABLE(connId);
                if (M3_ASP_INACTIVE == pConn->r_sp_g_st        ||
                    M3_ASP_ACTIVE == pConn->r_sp_g_st) {
                    m3uaRAS(M3_CONN_TABLE(connId), pInf->modify.ras_id,
                        M3_AS_EVT_ASPIA, M3_NULL);
                }
            }
            break;
        }
        case M3_ASP_DEL_R_ASP: {
            M3TRACE(m3uaConfigTrace, ("Deleting Remote ASP from ASP configuration"));
            if (M3_FALSE == M3_R_ASID_VALID(pInf->modify.ras_id)) {
                M3TRACE(m3uaErrorTrace, ("Invalid remote AS:%u", pInf->modify.ras_id));
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            pRAs = M3_R_AS_TABLE(pInf->modify.ras_id);
            if (M3_FALSE == pRAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned remote AS:%u", pInf->modify.ras_id));
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            if (M3_FALSE == M3_R_ASPID_VALID(pInf->modify.rasp_id)) {
                M3TRACE(m3uaErrorTrace, ("Invalid remote ASP:%u", pInf->modify.rasp_id));
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            pRAsp = M3_R_ASP_TABLE(pInf->modify.rasp_id);
            if (M3_FALSE == pRAsp->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned remote ASP:%u", pInf->modify.rasp_id));
                M3ERRNO(EM3_INV_R_ASPID);
                return -1;
            }
            if (M3_FALSE == pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                asp_list[(m3_u16)pInf->modify.rasp_id]) {
                M3TRACE(m3uaErrorTrace, ("Remote ASP:%u not provisioned in ASP configuration", pInf->modify.rasp_id));
                M3ERRNO(EM3_R_ASP_NOT_SERV);
                return -1;
            }
            if (M3_MAX_U32 != pAsp->r_asp_inf[(m3_u16)pInf->modify.rasp_id].conn_id) {
                connId = pAsp->r_asp_inf[(m3_u16)pInf->modify.rasp_id].conn_id;
                pConn = M3_CONN_TABLE(connId);
                m3uaRAS(M3_CONN_TABLE(connId), pInf->modify.ras_id,
                    M3_AS_EVT_ASPDN, M3_NULL);
            }
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                asp_list[(m3_u16)pInf->modify.rasp_id] = M3_FALSE;
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                rc_inf[(m3_u16)pInf->modify.rasp_id].dyn_entry = M3_FALSE;
            pAsp->r_as_inf[(m3_u16)pInf->modify.ras_id].\
                rc_inf[(m3_u16)pInf->modify.rasp_id].dyn_reg_state = M3_RK_STATIC;
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid configuration name:%d", pInf->modify.confname));
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3ua_asp_state(m3_u32            aspId,
                      m3_u32            rSpId,
                      m3_u8             oprn,
                      m3ua_asp_state_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t   *pAsp;
    m3_r_asp_inf_t *pRAsp;
    m3_r_sgp_inf_t *prsgp;
    m3_s32         ret = 0;
    m3_u32         connId;
    m3_as_inf_t    *pAs;
    m3_conn_inf_t  *pConn;

    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == M3_ASPID_VALID(aspId)) {
        M3TRACE(m3uaErrorTrace, ("Invalid ASP:%u", aspId));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pAsp = M3_ASP_TABLE(aspId);
    iu_log_debug("aspId = %d\n",aspId);
    if (M3_FALSE == pAsp->e_state) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u", aspId));
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
        if (M3_FALSE == M3_R_ASPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote ASP:%u", rSpId));
            M3ERRNO(EM3_INV_R_ASPID);
            return -1;
        }
        pRAsp = M3_R_ASP_TABLE(rSpId);
        if (M3_FALSE == pRAsp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote ASP:%u", rSpId));
            M3ERRNO(EM3_INV_R_ASPID);
            return -1;
        }
        connId = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
    } else if (M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        if (M3_FALSE == M3_R_SGPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        prsgp = M3_R_SGP_TABLE(rSpId);
        if (M3_FALSE == prsgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        connId = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid remote SP:%u", rSpId));
        M3ERRNO(EM3_INV_R_SPID);
        return -1;
    }
    if (M3_MAX_U32 == connId             ||
        M3_CONN_NOT_ESTB == (M3_CONN_TABLE(connId))->conn_state) {
        M3TRACE(m3uaErrorTrace, ("No connection between ASP:%u and remote SP:%u", aspId, rSpId));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    
    switch(oprn)
    {
        case M3_GET: {
            if (M3_FALSE == M3_ASID_VALID(pInf->get.info.as_id)) {
                M3TRACE(m3uaErrorTrace, ("Invalid AS:%u", pInf->get.info.as_id));
                M3ERRNO(EM3_INV_ASID);
                return -1;
            }
            pAs = M3_AS_TABLE(pInf->get.info.as_id);
            if (M3_FALSE == pAs->e_state) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned AS:%u", pInf->get.info.as_id));
                M3ERRNO(EM3_INV_ASID);
                return -1;
            }
            if (M3_FALSE == pAsp->as_list[pInf->get.info.as_id]) {
                M3TRACE(m3uaErrorTrace, ("AS:%u not served by ASP:%u", pInf->get.info.as_id, aspId));
                M3ERRNO(EM3_AS_NOT_SERV);
                return -1;
            }
            pConn = M3_CONN_TABLE(connId);
            pInf->get.state=pConn->l_sp_st[pInf->get.info.as_id];
            break;
        }
        case M3_MODIFY: {
            ret = m3uaModifyASPState(pAsp, connId, pInf);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation code:%u specified", (m3_u32)oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3uaModifyASPState(m3_asp_inf_t     *pAsp,
                          m3_u32           connId,
                          m3ua_asp_state_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;
    m3_u32        asId;
    m3_as_inf_t   *pAs;
    m3_s32        ret = 0;
    m3_conn_inf_t *pConn = M3_CONN_TABLE(connId);
    iu_log_debug("connId = %d\n",connId);
    m3_aspm_evt_t event;

    M3TRACE(m3uaAspmTrace, ("Changing ASP:%u state", pAsp->asp_id)); // 1
    if (M3_MAX_AS < pInf->modify.info.num_as) {
        M3TRACE(m3uaErrorTrace, ("Changing ASP:%u state in too many AS", pAsp->asp_id)); 
        M3ERRNO(EM3_INV_NUM_AS);
        return -1;
    }
    for (idx = pAsp->asp_id; idx < (pInf->modify.info.num_as + pAsp->asp_id); idx++) {
        asId = pInf->modify.info.as_list[idx];
        iu_log_debug("pInf->modify.info.as_list[%d] is %d\n", idx, pInf->modify.info.as_list[idx]);
        if (M3_FALSE == M3_ASID_VALID(pInf->modify.info.as_list[idx])) {
            M3TRACE(m3uaErrorTrace, ("Invalid AS:%u", pInf->modify.info.as_list[idx]));
            M3ERRNO(EM3_INV_ASID);
            return -1;
        }
        pAs = M3_AS_TABLE(pInf->modify.info.as_list[idx]);
        if (M3_FALSE == pAs->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned AS:%u", pInf->modify.info.as_list[idx]));
            M3ERRNO(EM3_INV_ASID);
            return -1;
        }
        if (M3_FALSE == pAsp->as_list[pInf->modify.info.as_list[idx]]) {
            M3TRACE(m3uaErrorTrace, ("AS:%u not served by ASP:%u", pInf->modify.info.as_list[idx], pAsp->asp_id));
            M3ERRNO(EM3_AS_NOT_SERV);
            return -1;
        }
        /* check for routing key state */
        if (M3_ASP_DOWN == pInf->modify.state) {
            /* all the routing keys should be in static state */
            if (M3_TRUE == M3_IS_SP_ASP(pConn->r_sp_id)) {
                if (M3_RK_STATIC != pAsp->r_asp_inf[(m3_u16)pConn->r_sp_id].rc_inf[asId].dyn_reg_state) {
                    M3TRACE(m3uaErrorTrace, ("RK Registration state is not static"));
                    M3ERRNO(EM3_RKSTATE_MISMATCH);
                    return -1;
                }
            } else {
                if (M3_RK_STATIC != pAsp->r_sgp_inf[(m3_u16)pConn->r_sp_id].rc_inf[asId].dyn_reg_state) {
                    M3TRACE(m3uaErrorTrace, ("RK Registration state is not static"));
                    M3ERRNO(EM3_RKSTATE_MISMATCH);
                    return -1;
                }
            }
        } else if (M3_ASP_ACTIVE == pInf->modify.state) {
            /* no routing key should be in reg in progress state */
            if (M3_TRUE == M3_IS_SP_ASP(pConn->r_sp_id)) {
                if (M3_RK_REG_IN_PROG == pAsp->r_asp_inf[(m3_u16)pConn->r_sp_id].rc_inf[asId].dyn_reg_state) {
                    M3TRACE(m3uaErrorTrace, ("RK Registration is in progress"));
                    M3ERRNO(EM3_RKSTATE_MISMATCH);
                    return -1;
                }
            } else {
                if (M3_RK_REG_IN_PROG == pAsp->r_sgp_inf[(m3_u16)pConn->r_sp_id].rc_inf[asId].dyn_reg_state) {
                    M3TRACE(m3uaErrorTrace, ("RK Registration is in progress"));
                    M3ERRNO(EM3_RKSTATE_MISMATCH);
                    return -1;
                }
            }
        }
    }
    
    switch(pInf->modify.state) {
        case M3_ASP_DOWN: {
            M3TRACE(m3uaAspmTrace, ("Moving ASP to DOWN state"));
            event = M3_ASPM_EVT_SWTO_ASPDN;
            break;
        }
        case M3_ASP_INACTIVE: {
            M3TRACE(m3uaAspmTrace, ("Moving ASP to INACTIVE state"));
            event = M3_ASPM_EVT_SWTO_ASPIA;
            break;
        }
        case M3_ASP_ACTIVE: {
            M3TRACE(m3uaAspmTrace, ("Moving ASP to ACTIVE state"));
            event = M3_ASPM_EVT_SWTO_ASPAC;
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid state:%u for ASP", pInf->modify.state));
            M3ERRNO(EM3_INV_ASPSTATE);
            return -1;
        }
    }
    
    ret = m3uaASPM(pConn, event, pInf);
    return ret;
}
