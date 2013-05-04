/****************************************************************************
** Description:
** Code for provisioning remote AS.
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

#include <m3ua_ras.h>

m3_s32 m3ua_r_as(m3_u32         asId,
                 m3_u8          oprn,
                 m3ua_r_as_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_r_as_inf_t *pAs;
    m3_s32        ret = 0;

    if (M3_FALSE == M3_R_ASID_VALID(asId)) {
        M3TRACE(m3uaErrorTrace, ("Invalid AS:%u", asId));
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }

    pAs = M3_R_AS_TABLE(asId);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddRAS(pAs, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteRAS(pAs, pInf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetRASConf(pAs, pInf);
            break;
        }
        case M3_MODIFY: {
            ret = m3uaModifyRASConf(pAs, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3uaAddRAS(m3_r_as_inf_t  *pAs,
                  m3ua_r_as_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3ua_route_t    route;
    m3_u16          idx, s_idx;
//iu_log_debug("m3uaAddRAS -------------\n");
    /* check for api parameters */
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pAs->e_state) {
        M3ERRNO(EM3_R_ASID_ALRDY);
        return -1;
    }
    if (M3_MAX_R_ASP < pInf->add.info.min_act_asp) {
        M3ERRNO(EM3_INV_MINACTASP);
        return -1;
    }
    if (M3_FALSE == M3_TRFMODE_VALID(pInf->add.info.rkey.trfmode)) {
        M3ERRNO(EM3_INV_TRAFFIC_MODE);
        return -1;
    }
    if (M3_MAX_U32 != pInf->add.info.rkey.nw_app                  &&
        M3_FALSE == m3uaAssertNwApp(pInf->add.info.rkey.nw_app)) {
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    if (0 == pInf->add.info.rkey.num_rtparam                      ||
        M3_MAX_DPC_PER_RK < pInf->add.info.rkey.num_rtparam) {
        M3ERRNO(EM3_INV_NUMDPC);
        return -1;
    }
    for (idx = 0; idx < pInf->add.info.rkey.num_rtparam; idx++) {
        if (M3_MAX_SI_PER_RK < pInf->add.info.rkey.rtparam[idx].num_si) {
            M3ERRNO(EM3_INV_NUMSI);
            return -1;
        }
        if (M3_MAX_OPC_PER_RK < pInf->add.info.rkey.rtparam[idx].num_opc) {
            M3ERRNO(EM3_INV_NUMOPC);
            return -1;
        }
        if (M3_MAX_CKT_RANGE_PER_RK < pInf->add.info.rkey.rtparam[idx].num_ckt_range) {
            M3ERRNO(EM3_INV_NUMCKT);
            return -1;
        }
    }
    pAs->e_state = M3_TRUE;
    pAs->dyn_reg = M3_FALSE;
    pAs->rtctx = pInf->add.info.rtctx;
    pAs->rkey = pInf->add.info.rkey;
    pAs->min_act_asp = pInf->add.info.min_act_asp;
    route.add.info.le_id = pAs->as_id;	//route le_id is as idx
    route.add.info.priority = 255 - pAs->as_id ;		//msc priority is 255, sgsn priority is 255
//iu_log_debug("m3uaAddRAS -------------=======\n");
    for (idx = 0; idx < pInf->add.info.rkey.num_rtparam; idx++) {		
//iu_log_debug("m3uaAddRAS -------------+++++++++++++++\n");
        route.add.info.pc_inf.ptcode = pAs->rkey.rtparam[idx].dpc;
        /*if (M3_MAX_U32 != pAs->rkey.nw_app) {change by yinlm when != need check form talbe*/
        if (M3_MAX_U32 == pAs->rkey.nw_app) {
            route.add.info.pc_inf.nw_app = pAs->rkey.nw_app;
            m3uaAddRoute(&route);
        }
        else {
            for (s_idx = 0; s_idx < M3_MAX_NWAPP; s_idx++) {
iu_log_debug("m3uaAddRAS (M3_NWAPP_TABLE(s_idx %d))->e_state %d) \n", s_idx, (M3_NWAPP_TABLE(s_idx))->e_state);				
                if (M3_TRUE == (M3_NWAPP_TABLE(s_idx))->e_state) {
                    route.add.info.pc_inf.nw_app= (M3_NWAPP_TABLE(s_idx))->nw_app;
                    m3uaAddRoute(&route);
                }
            }
        }
    }
    idx = (m3_u16)(pAs->rtctx >> 16);
    if (idx < M3_MAX_CONN && (m3_u16)(pAs->rtctx) < M3_MAX_R_AS) {
        (M3_CONN_TABLE(idx))->rtctxUsed[(m3_u16)(pAs->rtctx)] = M3_TRUE;
    }
    return 0;
}

m3_s32 m3uaDeleteRAS(m3_r_as_inf_t  *pAs,
                     m3ua_r_as_t    *pInf)
{
    m3_pd_buf_t   *p_pdbuf = M3_NULL;
    m3_pd_buf_t   *p_next = M3_NULL;
    m3_u16        idx, s_idx;
    m3_asp_inf_t  *pAsp;
    m3_sgp_inf_t  *pSgp;
    m3ua_route_t  route;

    if (M3_FALSE == pAs->e_state) {
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_ASP; idx++) {
        pAsp = M3_ASP_TABLE(idx);
        if (M3_TRUE == pAsp->e_state) {
            pAsp->r_as_inf[pAs->as_id].state = M3_AS_DOWN;
            pAsp->r_as_inf[pAs->as_id].num_asp_inactive = 0;
            pAsp->r_as_inf[pAs->as_id].num_asp_active = 0;
            /* release all the pending buffers */
            p_pdbuf = pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head;
            while (M3_NULL != p_pdbuf)
            {
                M3_MSG_FREE(p_pdbuf->p_buf, M3_TXR_POOL);
                p_next = p_pdbuf->p_next;
                M3_MSG_FREE(p_pdbuf, M3_PD_BUF_POOL);
                p_pdbuf = p_next;
            }
            for (s_idx = 0; s_idx < M3_MAX_R_ASP; s_idx++)
                pAsp->r_as_inf[pAs->as_id].asp_list[s_idx] = M3_FALSE;
        }
    }
    for (idx = 0; idx < M3_MAX_SGP; idx++) {
        pSgp = M3_SGP_TABLE(idx);
        if (M3_TRUE == pAsp->e_state) {
            pSgp->r_as_inf[pAs->as_id].state = M3_AS_DOWN;
            pSgp->r_as_inf[pAs->as_id].num_asp_inactive = 0;
            pSgp->r_as_inf[pAs->as_id].num_asp_active = 0;
            /* release all the pending buffers */
            p_pdbuf = pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head;
            while (M3_NULL != p_pdbuf)
            {
                M3_MSG_FREE(p_pdbuf->p_buf, M3_TXR_POOL);
                p_next = p_pdbuf->p_next;
                M3_MSG_FREE(p_pdbuf, M3_PD_BUF_POOL);
                p_pdbuf = p_next;
            }
            for (s_idx = 0; s_idx < M3_MAX_R_ASP; s_idx++)
                pSgp->r_as_inf[pAs->as_id].asp_list[s_idx] = M3_FALSE;
                pSgp->r_as_inf[pAs->as_id].rc_inf[s_idx].rtctx = M3_MAX_U32;
                pSgp->r_as_inf[pAs->as_id].rc_inf[s_idx].dyn_reg_state = M3_RK_STATIC;
        }
    }
    route.del.info.pc_inf.ptcode = M3_MAX_U32;
    route.del.info.le_id         = pAs->as_id;
    route.del.info.priority      = M3_MAX_U8;
    m3uaDeleteRoute(&route);
    pAs->e_state = M3_FALSE;
    idx = (m3_u16)(pAs->rtctx >> 16);
    if (idx < M3_MAX_CONN && (m3_u16)(pAs->rtctx) < M3_MAX_R_AS) {
        (M3_CONN_TABLE(idx))->rtctxUsed[(m3_u16)(pAs->rtctx)] = M3_FALSE;
    }
    return 0;
}

m3_s32 m3uaGetRASConf(m3_r_as_inf_t  *pAs,
                      m3ua_r_as_t    *pInf)
{
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAs->e_state) {
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }
    pInf->get.info.rtctx = pAs->rtctx;
    pInf->get.info.rkey = pAs->rkey;
    pInf->get.info.min_act_asp = pAs->min_act_asp;
    return 0;
}

m3_s32 m3uaModifyRASConf(m3_r_as_inf_t    *pAs,
                         m3ua_r_as_t      *pInf)
{
    m3_u16        idx;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAs->e_state) {
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }
    switch(pInf->modify.confname)
    {
        case M3_R_AS_RTCTX: {
            if (M3_TRUE==m3uaAssertRRC(pInf->modify.info.rtctx)){
                M3ERRNO(EM3_RTCTX_ALRDY);
                return -1;
            }
            pAs->rtctx = pInf->modify.info.rtctx;
            break;
        }
        case M3_R_AS_RKEY: {
            if (M3_FALSE == M3_TRFMODE_VALID(pInf->modify.info.rkey.trfmode)) {
                M3ERRNO(EM3_INV_TRAFFIC_MODE);
                return -1;
            }
            if (M3_MAX_U32 != pInf->modify.info.rkey.nw_app          &&
                M3_FALSE == m3uaAssertNwApp(pInf->modify.info.rkey.nw_app)) {
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            if (0 == pInf->modify.info.rkey.num_rtparam              ||
                M3_MAX_DPC_PER_RK < pInf->modify.info.rkey.num_rtparam) {
                M3ERRNO(EM3_INV_NUMDPC);
                return -1;
            }
            for (idx = 0; idx < pInf->modify.info.rkey.num_rtparam; idx++) {
                if (M3_MAX_SI_PER_RK < pInf->modify.info.rkey.rtparam[idx].num_si) {
                    M3ERRNO(EM3_INV_NUMSI);
                    return -1;
                }
                if (M3_MAX_OPC_PER_RK < pInf->modify.info.rkey.rtparam[idx].num_opc) {
                    M3ERRNO(EM3_INV_NUMOPC);
                    return -1;
                }
                if (M3_MAX_CKT_RANGE_PER_RK<pInf->modify.info.rkey.rtparam[idx].num_ckt_range) {
                    M3ERRNO(EM3_INV_NUMCKT);
                    return -1;
                }
            }
            pAs->rkey = pInf->modify.info.rkey;
            break;
        }
        case M3_R_AS_MIN_ACT_ASP: {
            if (M3_MAX_R_ASP < pInf->modify.info.min_act_asp) {
                M3ERRNO(EM3_INV_MINACTASP);
                return -1;
            }
            pAs->min_act_asp = pInf->modify.info.min_act_asp;
            break;
        }
        case M3_R_AS_INFO: {
            if (M3_TRUE==m3uaAssertRRC(pInf->modify.info.rtctx)){
                M3ERRNO(EM3_RTCTX_ALRDY);
                return -1;
            }
            if (M3_MAX_R_ASP < pInf->modify.info.min_act_asp) {
                M3ERRNO(EM3_INV_MINACTASP);
                return -1;
            }    
            if (M3_FALSE == M3_TRFMODE_VALID(pInf->modify.info.rkey.trfmode)) {
                M3ERRNO(EM3_INV_TRAFFIC_MODE);
                return -1;
            }
            if (M3_MAX_U32 != pInf->modify.info.rkey.nw_app          &&
                M3_FALSE == m3uaAssertNwApp(pInf->modify.info.rkey.nw_app)) {
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            if (0 == pInf->modify.info.rkey.num_rtparam              ||
                M3_MAX_DPC_PER_RK < pInf->modify.info.rkey.num_rtparam) {
                M3ERRNO(EM3_INV_NUMDPC);
                return -1;
            }
            for (idx = 0; idx < pInf->modify.info.rkey.num_rtparam; idx++) {
                if (M3_MAX_SI_PER_RK < pInf->modify.info.rkey.rtparam[idx].num_si) {
                    M3ERRNO(EM3_INV_NUMSI);
                    return -1;
                }
                if (M3_MAX_OPC_PER_RK < pInf->modify.info.rkey.rtparam[idx].num_opc) {
                    M3ERRNO(EM3_INV_NUMOPC);
                    return -1;
                }
                if (M3_MAX_CKT_RANGE_PER_RK<pInf->modify.info.rkey.rtparam[idx].num_ckt_range) {
                    M3ERRNO(EM3_INV_NUMCKT);
                    return -1;
                }
            }
            pAs->rtctx = pInf->modify.info.rtctx;
            pAs->rkey = pInf->modify.info.rkey;
            pAs->min_act_asp = pInf->modify.info.min_act_asp;
            break;
        }
        default: {
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3ua_r_as_state(m3_u32              asId,
                       m3_u32              l_spid,
                       m3_u8               oprn,
                       m3ua_r_as_state_t   *pInf)
{
    m3_r_as_inf_t  *pAs;
    m3_asp_inf_t   *plasp;
    m3_sgp_inf_t   *plsgp;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == M3_R_ASID_VALID(asId)) {
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }
    pAs = M3_R_AS_TABLE(asId);
    if (M3_FALSE == pAs->e_state) {
        M3ERRNO(EM3_INV_R_ASID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        if (M3_FALSE == M3_ASPID_VALID(l_spid)) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        plasp = M3_ASP_TABLE(l_spid);
        if (M3_FALSE == plasp->e_state) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
    } else if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
        if (M3_FALSE == M3_SGPID_VALID(l_spid)) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        plsgp = M3_SGP_TABLE(l_spid);
        if (M3_FALSE == plsgp->e_state) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
    } else {
        M3ERRNO(EM3_INV_LSPID);
        return -1;
    }
    switch(oprn)
    {
        case M3_GET: {
            if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
                pInf->get.state = plsgp->r_as_inf[(m3_u16)asId].state;
            }
            else {
                pInf->get.state = plasp->r_as_inf[(m3_u16)asId].state;
            }
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return 0;
}

m3_u32 m3uaGetRASFromRC(m3_conn_inf_t *pConn,
                        m3_u32        rtctx)
{
    m3_u16        idx;
    m3_asp_inf_t  *pAsp;
    m3_sgp_inf_t  *pSgp;
    iu_log_debug("rtctx = %d\n",rtctx);

    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        pAsp = M3_ASP_TABLE(pConn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE == (M3_R_AS_TABLE(idx))->e_state                   &&
                M3_TRUE == pAsp->r_as_inf[idx].asp_list[(m3_u16)pConn->r_sp_id]) {
                if (M3_RK_STATIC == pAsp->r_as_inf[idx].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state) {
                    if (rtctx == (M3_R_AS_TABLE(idx))->rtctx) {
                        break;
                    }
                } else if (rtctx == pAsp->r_as_inf[idx].rc_inf[(m3_u16)pConn->r_sp_id].rtctx) {
                    break;
                }
            }
        }
    } else {
        pSgp = M3_SGP_TABLE(pConn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE == (M3_R_AS_TABLE(idx))->e_state                   &&
                M3_TRUE == pSgp->r_as_inf[idx].asp_list[(m3_u16)pConn->r_sp_id]) {
                if (M3_RK_STATIC == pSgp->r_as_inf[idx].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state) {
                    if (rtctx == (M3_R_AS_TABLE(idx))->rtctx) {
                        break;
                    }
                } else if (rtctx == pSgp->r_as_inf[idx].rc_inf[(m3_u16)pConn->r_sp_id].rtctx) {
                    break;
                }
            }
        }
    }
    if (M3_MAX_R_AS > idx) {
        return idx;
    }
    return M3_MAX_U32;
}

m3_u32 m3uaGetRCFromRAS(m3_conn_inf_t *pConn,
                        m3_u32        asId)
{
    m3_u32        rtctx = M3_MAX_U32;
    m3_asp_inf_t  *pAsp;
    m3_sgp_inf_t  *pSgp;
    m3_r_as_inf_t *pAs = M3_R_AS_TABLE(asId);

    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        pAsp = M3_ASP_TABLE(pConn->l_sp_id);
        if (M3_RK_STATIC == pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state)
            rtctx = pAs->rtctx;
        else if (M3_RK_REGD == pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state)
            rtctx = pAsp->r_as_inf[(m3_u16)asId].rc_inf[(m3_u16)pConn->r_sp_id].rtctx;
    } else {
        pSgp = M3_SGP_TABLE(pConn->l_sp_id);
        if (M3_RK_STATIC == pSgp->r_as_inf[(m3_u16)asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state)
            rtctx = pAs->rtctx;
        else if (M3_RK_REGD == pSgp->r_as_inf[(m3_u16)asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state)
            rtctx = pSgp->r_as_inf[(m3_u16)asId].rc_inf[(m3_u16)pConn->r_sp_id].rtctx;
    }
    return rtctx;
}

m3_s32 m3uaSetPDTimerId(m3_u32        asId,
                        m3_u32        l_spid,
                        m3_u32        timer_id)
{
    if (M3_TRUE == M3_IS_SP_SGP(l_spid))
        (M3_SGP_TABLE(l_spid))->r_as_inf[asId].pd_q_inf.pd_timer_inf.timer_id =
            timer_id;
    else
        (M3_ASP_TABLE(l_spid))->r_as_inf[asId].pd_q_inf.pd_timer_inf.timer_id =
            timer_id;
    return 0;
}

m3_s32 m3uaGetPDTimerId(m3_u32        asId,
                        m3_u32        l_spid,
                        m3_u32        *p_timerid)
{
    if (M3_TRUE == M3_IS_SP_SGP(l_spid))
        *p_timerid = (M3_SGP_TABLE(l_spid))->r_as_inf[asId].pd_q_inf.\
            pd_timer_inf.timer_id;
    else
        *p_timerid = (M3_ASP_TABLE(l_spid))->r_as_inf[asId].pd_q_inf.\
            pd_timer_inf.timer_id;
    return 0;
}

m3_s32 m3uaSetRASState(m3_u32          asId,
                       m3_u32          l_spid,
                       m3_as_state_t   as_state)
{
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        (M3_ASP_TABLE(l_spid))->r_as_inf[asId].state = as_state;
    } else {
        (M3_SGP_TABLE(l_spid))->r_as_inf[asId].state = as_state;
    }
    return 0;
}

m3_s32 m3uaGetRASState(m3_u32          asId,
                       m3_u32          l_spid,
                       m3_as_state_t   *pstate)
{
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        *pstate = (M3_ASP_TABLE(l_spid))->r_as_inf[asId].state;
    } else {
        *pstate = (M3_SGP_TABLE(l_spid))->r_as_inf[asId].state;
    }
    return 0;
}

m3_s32 m3uaAddPDBuf(m3_r_as_inf_t    *pAs,
                    m3_u32           l_spid,
                    m3_u32           buflen,
                    m3_u8            *p_buf)
{
    m3_pd_buf_t    *p_pdbuf = M3_NULL;
    m3_sgp_inf_t   *pSgp;
    m3_asp_inf_t   *pAsp;

    p_pdbuf = (m3_pd_buf_t*)M3_MSG_ALLOC(sizeof(m3_pd_buf_t), M3_PD_BUF_POOL);
    if (M3_NULL == p_pdbuf) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    p_pdbuf->buflen    = buflen;
    p_pdbuf->p_buf     = p_buf;
    p_pdbuf->p_next    = M3_NULL;
    if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
        pSgp = M3_SGP_TABLE(l_spid);
        if (M3_NULL == pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head) {
            pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head = p_pdbuf;
            pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail = p_pdbuf;
        } else {
            pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail->p_next = p_pdbuf;
            pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail =
                pSgp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail->p_next;
        }
    } else {
        pAsp = M3_ASP_TABLE(l_spid);
        if (M3_NULL == pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head) {
            pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_head = p_pdbuf;
            pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail = p_pdbuf;
        } else {
            pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail->p_next = p_pdbuf;
            pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail =
                pAsp->r_as_inf[pAs->as_id].pd_q_inf.pd_q_tail->p_next;
        }
    }
    return 0;
}

m3_s32 m3uaGetPDBuf(m3_u32    asId,
                    m3_u32    l_spid,
                    m3_u32    *p_buflen,
                    m3_u8     **p_buf)
{
    m3_pd_buf_t    *p_node;
    m3_asp_inf_t   *pAsp;
    m3_sgp_inf_t   *pSgp;

    *p_buf = M3_NULL;
    if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
        pSgp = M3_SGP_TABLE(l_spid);
        if (M3_NULL != pSgp->r_as_inf[asId].pd_q_inf.pd_q_head) {
            *p_buflen = pSgp->r_as_inf[asId].pd_q_inf.pd_q_head->buflen;
            *p_buf = pSgp->r_as_inf[asId].pd_q_inf.pd_q_head->p_buf;
            p_node = pSgp->r_as_inf[asId].pd_q_inf.pd_q_head->p_next;
            M3_MSG_FREE(pSgp->r_as_inf[asId].pd_q_inf.pd_q_head,
                M3_PD_BUF_POOL);
            pSgp->r_as_inf[asId].pd_q_inf.pd_q_head = p_node;
        } else {
            return -1;
        }
    } else {
        pAsp = M3_ASP_TABLE(l_spid);
        if (M3_NULL != pAsp->r_as_inf[asId].pd_q_inf.pd_q_head) {
            *p_buflen = pAsp->r_as_inf[asId].pd_q_inf.pd_q_head->buflen;
            *p_buf = pAsp->r_as_inf[asId].pd_q_inf.pd_q_head->p_buf;
            p_node = pAsp->r_as_inf[asId].pd_q_inf.pd_q_head->p_next;
            M3_MSG_FREE(pAsp->r_as_inf[asId].pd_q_inf.pd_q_head,
                M3_PD_BUF_POOL);
            pAsp->r_as_inf[asId].pd_q_inf.pd_q_head = p_node;
        } else {
            return -1;
        }
    }
    return 0;
}

void m3uaDeletePDQ(m3_u32    asId,
                   m3_u32    l_spid)
{
    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;
    m3_pd_buf_t     *p_node;

    if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
        pSgp = M3_SGP_TABLE(l_spid);
        while (M3_NULL != pSgp->r_as_inf[asId].pd_q_inf.pd_q_head) {
            p_node = pSgp->r_as_inf[asId].pd_q_inf.pd_q_head->p_next;
            M3_MSG_FREE(pSgp->r_as_inf[asId].pd_q_inf.pd_q_head->p_buf,
                M3_TXR_POOL);
            M3_MSG_FREE(pSgp->r_as_inf[asId].pd_q_inf.pd_q_head,
                M3_PD_BUF_POOL);
            pSgp->r_as_inf[asId].pd_q_inf.pd_q_head = p_node;
        }
        pSgp->r_as_inf[asId].pd_q_inf.pd_q_tail = M3_NULL;
        pSgp->r_as_inf[asId].pd_q_inf.pd_timer_inf.timer_id  = M3_MAX_U32;
    } else {
        pAsp = M3_ASP_TABLE(l_spid);
        while (M3_NULL != pAsp->r_as_inf[asId].pd_q_inf.pd_q_head) {
            p_node = pAsp->r_as_inf[asId].pd_q_inf.pd_q_head->p_next;
            M3_MSG_FREE(pAsp->r_as_inf[asId].pd_q_inf.pd_q_head->p_buf,
                M3_TXR_POOL);
            M3_MSG_FREE(pAsp->r_as_inf[asId].pd_q_inf.pd_q_head,
                M3_PD_BUF_POOL);
            pAsp->r_as_inf[asId].pd_q_inf.pd_q_head = p_node;
        }
        pAsp->r_as_inf[asId].pd_q_inf.pd_q_tail = M3_NULL;
        pAsp->r_as_inf[asId].pd_q_inf.pd_timer_inf.timer_id  = M3_MAX_U32;
    }
    return;
}

m3_bool_t m3uaAssertRRC(m3_u32    rtctx)
{
    m3_r_as_inf_t *pAs;
    m3_u16        idx;

    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pAs = M3_R_AS_TABLE(idx);
        if (M3_TRUE == pAs->e_state && rtctx == pAs->rtctx) {
            return M3_TRUE;
        }
    }
    return M3_FALSE;
}

m3_s32 m3uaGetASPsWithState(m3_r_as_inf_t  *pAs,
                            m3_u32         l_spid,
                            m3_asp_state_t state,
                            m3_u8          *pnum_asp,
                            m3_u32         *pAsp_list)
{
    m3_u16         idx;
    m3_conn_inf_t  *pConn;
    m3_asp_inf_t   *pAsp;
    m3_sgp_inf_t   *pSgp;

    *pnum_asp = 0;
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        pAsp = M3_ASP_TABLE(l_spid);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pAsp->r_as_inf[pAs->as_id].asp_list[idx] &&
                M3_MAX_U32 != pAsp->r_asp_inf[idx].conn_id) {
                pConn = M3_CONN_TABLE(pAsp->r_asp_inf[idx].conn_id);
                if (state == pConn->r_sp_st[pAs->as_id]) {
                    pAsp_list[(*pnum_asp)++] =  idx;
                }
            }
        }
    } else {
        pSgp = M3_SGP_TABLE(l_spid);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pSgp->r_as_inf[pAs->as_id].asp_list[idx] &&
                M3_MAX_U32 != pSgp->r_asp_inf[idx].conn_id) {
                pConn = M3_CONN_TABLE(pSgp->r_asp_inf[idx].conn_id);
                if (state == pConn->r_sp_st[pAs->as_id]) {
                    pAsp_list[(*pnum_asp)++] =  idx;
                }
            }
        }
    }
    return 0;
}

m3_s32 m3uaAddDynamicRAS(m3_r_as_inf_t  *pAs,
                         m3ua_r_as_t    *pInf)
{
    m3ua_route_t    route;
    m3_u16          idx, s_idx;

    pAs->e_state = M3_TRUE;
    pAs->dyn_reg = M3_TRUE;
    pAs->rtctx = pInf->add.info.rtctx;
    pAs->rkey = pInf->add.info.rkey;
    pAs->min_act_asp = pInf->add.info.min_act_asp;
    route.add.info.le_id = pAs->as_id;
    route.add.info.priority = 255;
    for (idx = 0; idx < pInf->add.info.rkey.num_rtparam; idx++) {
        route.add.info.pc_inf.ptcode = pAs->rkey.rtparam[idx].dpc;
        if (M3_MAX_U32 != pAs->rkey.nw_app) {
            route.add.info.pc_inf.nw_app = pAs->rkey.nw_app;
            m3uaAddRoute(&route);
        }
        else {
            for (s_idx = 0; s_idx < M3_MAX_NWAPP; s_idx++) {
                if (M3_TRUE == (M3_NWAPP_TABLE(s_idx))->e_state) {
                    route.add.info.pc_inf.nw_app= (M3_NWAPP_TABLE(s_idx))->nw_app;
                    m3uaAddRoute(&route);
                }
            }
        }
    }
    return 0;
}

m3_bool_t m3uaRRCUnique(m3_conn_inf_t *pConn,
                        m3_u32        rtctx)
{
    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;
    m3_u16          asIdx;

    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        pAsp = M3_ASP_TABLE(pConn->l_sp_id);
        for (asIdx = 0; asIdx < M3_MAX_R_AS; asIdx++) {
            if (M3_TRUE == (M3_R_AS_TABLE(asIdx))->e_state &&
                rtctx == pAsp->r_as_inf[(m3_u16)asIdx].rc_inf[(m3_u16)pConn->r_sp_id].rtctx) {
                return M3_FALSE;
            }
        }
    } else {
        pSgp = M3_SGP_TABLE(pConn->l_sp_id);
        for (asIdx = 0; asIdx < M3_MAX_R_AS; asIdx++) {
            if (M3_TRUE == (M3_R_AS_TABLE(asIdx))->e_state &&
                rtctx == pSgp->r_as_inf[(m3_u16)asIdx].rc_inf[(m3_u16)pConn->r_sp_id].rtctx) {
                return M3_FALSE;
            }
        }
    }
    return M3_TRUE;
}

