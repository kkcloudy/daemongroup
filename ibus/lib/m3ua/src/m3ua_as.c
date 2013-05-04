/****************************************************************************
** Description:
** Code for provisioning an AS.
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

#include <m3ua_as.h>

m3_s32 m3ua_as(m3_u32       as_id,
               m3_u8        oprn,
               m3ua_as_t    *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_as_inf_t   *pAs;
    m3_s32        retval;

    if (M3_FALSE == M3_ASID_VALID(as_id)) {
        M3TRACE(m3uaErrorTrace, ("Invalid AS Identifier %u passed", as_id));
        M3ERRNO(EM3_INV_ASID);
        return -1;
    }
    pAs = M3_AS_TABLE(as_id);
    switch(oprn)
    {
        case M3_ADD: {
            retval = m3uaAddAS(pAs, p_inf);
            break;
        }
        case M3_DELETE: {
            retval = m3uaDeleteAS(pAs, p_inf);
            break;
        }
        case M3_GET: {
            retval = m3uaGetASConf(pAs, p_inf);
            break;
        }
        case M3_MODIFY: {
            retval = m3uaModifyASConf(pAs, p_inf);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation code %u specified", (m3_u32)oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return retval;
}

m3_s32 m3uaAddAS(m3_as_inf_t  *pAs,
                 m3ua_as_t    *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;

    M3TRACE(m3uaConfigTrace, ("Adding AS %u", pAs->as_id));
    if (M3_NULL == p_inf) {
        M3TRACE(m3uaErrorTrace, ("Null configuration information passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pAs->e_state) {
        M3TRACE(m3uaErrorTrace, ("AS %u already added", pAs->as_id));
        M3ERRNO(EM3_ASID_ALRDY);
        return -1;
    }
    if (M3_FALSE == M3_TRFMODE_VALID(p_inf->add.info.rkey.trfmode)) {
        M3TRACE(m3uaErrorTrace, ("Incorrect traffic mode %u", p_inf->add.info.rkey.trfmode));
        M3ERRNO(EM3_INV_TRAFFIC_MODE);
        return -1;
    }
    if (M3_MAX_U32 != p_inf->add.info.rkey.nw_app                  &&
        M3_FALSE == m3uaAssertNwApp(p_inf->add.info.rkey.nw_app)) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned Network Appearance %u", p_inf->add.info.rkey.nw_app));
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    if (0 == p_inf->add.info.rkey.num_rtparam                   ||
        M3_MAX_DPC_PER_RK < p_inf->add.info.rkey.num_rtparam) {
        M3TRACE(m3uaErrorTrace, ("Invalid Number of DPC %u present in the Routing key", p_inf->add.info.rkey.num_rtparam));
        M3ERRNO(EM3_INV_NUMDPC);
        return -1;
    }
    for (idx = 0; idx < p_inf->add.info.rkey.num_rtparam; idx++) {
        if (M3_MAX_SI_PER_RK < p_inf->add.info.rkey.rtparam[idx].num_si) {
            M3TRACE(m3uaErrorTrace, ("Invalid Number of SI %u present in the Routing key", p_inf->add.info.rkey.rtparam[idx].num_si))
            M3ERRNO(EM3_INV_NUMSI);
            return -1;
        }
        if (M3_MAX_OPC_PER_RK < p_inf->add.info.rkey.rtparam[idx].num_opc) {
            M3TRACE(m3uaErrorTrace, ("Invalid Number of OPC %u present in the Routing key", p_inf->add.info.rkey.rtparam[idx].num_opc));
            M3ERRNO(EM3_INV_NUMOPC);
            return -1;
        }
        if (M3_MAX_CKT_RANGE_PER_RK < p_inf->add.info.rkey.rtparam[idx].num_ckt_range) {
            M3TRACE(m3uaErrorTrace, ("Invalid Number of CIC range %u present in the Routing key", p_inf->add.info.rkey.rtparam[idx].num_ckt_range));
            M3ERRNO(EM3_INV_NUMCKT);
            return -1;
        }
    }
    pAs->e_state = M3_TRUE;
    pAs->dyn_reg = M3_FALSE;
    pAs->rtctx = p_inf->add.info.rtctx;
    iu_log_debug("pAs->rtctx = %d\n",pAs->rtctx);
    pAs->rkey = p_inf->add.info.rkey;
    M3TRACE(m3uaConfigTrace, ("AS %u added successfully", pAs->as_id));
    return 0;
}

m3_s32 m3uaDeleteAS(m3_as_inf_t *pAs,
                    m3ua_as_t   *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx, idx2;
    m3_usr_inf_t  *pusr;

    M3TRACE(m3uaConfigTrace, ("Deleting AS %u", pAs->as_id));
    if (M3_FALSE == pAs->e_state) {
        M3TRACE(m3uaErrorTrace, ("AS %u not provisioned", pAs->as_id));
        M3ERRNO(EM3_INV_ASID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_USR; idx++) {
        pusr = M3_USR_TABLE(idx);
        if (M3_TRUE == pusr->e_state                     &&
            M3_TRUE == M3_IS_SP_ASP(pusr->sp_id)         &&
            pAs->as_id == pusr->user.mtp_user.as_id) {
            M3TRACE(m3uaErrorTrace, ("AS %u has user %u, delete user before deleting AS", pAs->as_id, pusr->usr_id));
            M3ERRNO(EM3_AS_WITH_USR);
            return -1;
        }
    }
    for (idx = 0; idx < M3_MAX_ASP; idx++) {
        (M3_ASP_TABLE(idx))->as_list[pAs->as_id] = M3_FALSE;
        for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
            (M3_ASP_TABLE(idx))->r_asp_inf[idx2].rc_inf[pAs->as_id].rtctx = M3_MAX_U32;
            (M3_ASP_TABLE(idx))->r_asp_inf[idx2].rc_inf[pAs->as_id].dyn_reg_state = M3_RK_STATIC;
        }
        for (idx2 = 0; idx2 < M3_MAX_R_SGP; idx2++) {
            (M3_ASP_TABLE(idx))->r_sgp_inf[idx2].rc_inf[pAs->as_id].rtctx = M3_MAX_U32;
            (M3_ASP_TABLE(idx))->r_sgp_inf[idx2].rc_inf[pAs->as_id].dyn_reg_state = M3_RK_STATIC;
        }
    }
    pAs->e_state = M3_FALSE;
    M3TRACE(m3uaConfigTrace, ("Deleted AS %u", pAs->as_id));
    return 0;
}

m3_s32 m3uaGetASConf(m3_as_inf_t    *pAs,
                     m3ua_as_t      *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    M3TRACE(m3uaConfigTrace, ("Reading configuration for AS %u", pAs->as_id));
    if (M3_NULL == p_inf) {
        M3TRACE(m3uaErrorTrace, ("Null container passed for reading information"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAs->e_state) {
        M3TRACE(m3uaErrorTrace, ("AS %u not provisioned", pAs->as_id));
        M3ERRNO(EM3_INV_ASID);
        return -1;
    }
    p_inf->get.info.rtctx = pAs->rtctx;
    p_inf->get.info.rkey = pAs->rkey;
    M3TRACE(m3uaConfigTrace, ("Configuration read for AS %u", pAs->as_id));
    return 0;
}

m3_s32 m3uaModifyASConf(m3_as_inf_t    *pAs,
                        m3ua_as_t      *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;

    M3TRACE(m3uaConfigTrace, ("Changing configuration for AS %u", pAs->as_id));
    if (M3_NULL == p_inf) {
        M3TRACE(m3uaErrorTrace, ("Null configuration information passed"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAs->e_state) {
        M3TRACE(m3uaErrorTrace, ("AS %u not provisioned", pAs->as_id));
        M3ERRNO(EM3_INV_ASID);
        return -1;
    }
    switch(p_inf->modify.confname)
    {
        case M3_AS_RTCTX: {
            M3TRACE(m3uaConfigTrace, ("Changing Routing Context"));
            pAs->rtctx = p_inf->modify.info.rtctx;
            break;
        }
        case M3_AS_RKEY: {
            M3TRACE(m3uaConfigTrace, ("Changing Routing key"));
            if (M3_FALSE == M3_TRFMODE_VALID(p_inf->modify.info.rkey.trfmode)) {
                M3TRACE(m3uaErrorTrace, ("Incorrect traffic mode %u", p_inf->modify.info.rkey.trfmode));
                M3ERRNO(EM3_INV_TRAFFIC_MODE);
                return -1;
            }
            if (M3_MAX_U32 != p_inf->modify.info.rkey.nw_app          &&
                M3_FALSE == m3uaAssertNwApp(p_inf->modify.info.rkey.nw_app)) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned Network Appearance %u", p_inf->modify.info.rkey.nw_app));
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            if (0 == p_inf->modify.info.rkey.num_rtparam              ||
                M3_MAX_DPC_PER_RK < p_inf->modify.info.rkey.num_rtparam) {
                M3TRACE(m3uaErrorTrace, ("Invalid Number of DPC %u present in the Routing key", p_inf->modify.info.rkey.num_rtparam));
                M3ERRNO(EM3_INV_NUMDPC);
                return -1; 
            }
            for (idx = 0; idx < p_inf->modify.info.rkey.num_rtparam; idx++) {
                if (M3_MAX_SI_PER_RK < p_inf->modify.info.rkey.rtparam[idx].num_si) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of SI %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_si));
                    M3ERRNO(EM3_INV_NUMSI);
                    return -1;
                }
                if (M3_MAX_OPC_PER_RK < p_inf->modify.info.rkey.rtparam[idx].num_opc) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of OPC %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_opc));
                    M3ERRNO(EM3_INV_NUMOPC);
                    return -1;
                }
                if (M3_MAX_CKT_RANGE_PER_RK<p_inf->modify.info.rkey.rtparam[idx].num_ckt_range) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of Circuit range %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_ckt_range));
                    M3ERRNO(EM3_INV_NUMCKT);
                    return -1;
                }
            }
            pAs->rkey = p_inf->modify.info.rkey;
            break;
        }
        case M3_AS_INFO: {
            M3TRACE(m3uaConfigTrace, ("Changing complete AS configuration"));
            if (M3_FALSE == M3_TRFMODE_VALID(p_inf->modify.info.rkey.trfmode)) {
                M3TRACE(m3uaErrorTrace, ("Incorrect traffic mode %u", p_inf->modify.info.rkey.trfmode));
                M3ERRNO(EM3_INV_TRAFFIC_MODE);
                return -1;
            }
            if (M3_MAX_U32 != p_inf->modify.info.rkey.nw_app          &&
                M3_FALSE == m3uaAssertNwApp(p_inf->modify.info.rkey.nw_app)) {
                M3TRACE(m3uaErrorTrace, ("Non-provisioned Network Appearance %u", p_inf->modify.info.rkey.nw_app));
                M3ERRNO(EM3_INV_NWAPP);
                return -1;
            }
            if (0 == p_inf->modify.info.rkey.num_rtparam              ||
                M3_MAX_DPC_PER_RK < p_inf->modify.info.rkey.num_rtparam) {
                M3TRACE(m3uaErrorTrace, ("Invalid Number of DPC %u present in the Routing key", p_inf->modify.info.rkey.num_rtparam));
                M3ERRNO(EM3_INV_NUMDPC);
                return -1; 
            }
            for (idx = 0; idx < p_inf->modify.info.rkey.num_rtparam; idx++) {
                if (M3_MAX_SI_PER_RK < p_inf->modify.info.rkey.rtparam[idx].num_si) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of SI %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_si));
                    M3ERRNO(EM3_INV_NUMSI);
                    return -1;
                }
                if (M3_MAX_OPC_PER_RK < p_inf->modify.info.rkey.rtparam[idx].num_opc) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of OPC %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_opc));
                    M3ERRNO(EM3_INV_NUMOPC);
                    return -1;
                }
                if (M3_MAX_CKT_RANGE_PER_RK<p_inf->modify.info.rkey.rtparam[idx].num_ckt_range) {
                    M3TRACE(m3uaErrorTrace, ("Invalid Number of Circuit range %u present in the Routing key", p_inf->modify.info.rkey.rtparam[idx].num_ckt_range));
                    M3ERRNO(EM3_INV_NUMCKT);
                    return -1;
                }
            }
            pAs->rtctx = p_inf->modify.info.rtctx;
            pAs->rkey = p_inf->modify.info.rkey;
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid configuration name:%d", p_inf->modify.confname));
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}

m3_u32 m3uaGetASFromRC(m3_conn_inf_t   *pconn,
                       m3_u32          rtctx)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx = 0;
    m3_asp_inf_t  *pAsp = M3_ASP_TABLE(pconn->l_sp_id);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (M3_TRUE == (M3_AS_TABLE(idx))->e_state && M3_TRUE == pAsp->as_list[idx]) {
                if (M3_RK_STATIC == pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[idx].dyn_reg_state) {
                    if (rtctx == (M3_AS_TABLE(idx))->rtctx) {
                        break;
                    }
                } else if (rtctx == pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[idx].rtctx) {
                    break;
                }
            }
        }
    } else {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (M3_TRUE == (M3_AS_TABLE(idx))->e_state && M3_TRUE == pAsp->as_list[idx]) {
                if (M3_RK_STATIC == pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[idx].dyn_reg_state) {
                    if (rtctx == (M3_AS_TABLE(idx))->rtctx) {
                        break;
                    }
                } else if (rtctx == pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[idx].rtctx) {
                    break;
                }
            }
        }
    }
    if (M3_MAX_AS > idx) {
        return idx;
    }
    return M3_MAX_U32;
}

m3_u32 m3uaGetRCFromAS(m3_conn_inf_t   *pconn,
                       m3_u32          as_id)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u32        rtctx = M3_MAX_U32;
    m3_asp_inf_t  *pAsp = M3_ASP_TABLE(pconn->l_sp_id);
    m3_as_inf_t   *pAs = M3_AS_TABLE(as_id);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        if (M3_RK_STATIC == pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].dyn_reg_state)
            rtctx = pAs->rtctx;
        else if (M3_RK_REGD == pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].dyn_reg_state)
            rtctx = pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].rtctx;
    } else {
        if (M3_RK_STATIC == pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].dyn_reg_state)
            rtctx = pAs->rtctx;
        else if (M3_RK_REGD == pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].dyn_reg_state)
            rtctx = pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[(m3_u16)as_id].rtctx;
    }
    return rtctx;
}

m3_bool_t m3uaLRCUnique(m3_conn_inf_t *pconn,
                        m3_u32        rtctx)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t    *pAsp = M3_ASP_TABLE(pconn->l_sp_id);
    m3_u16          as_idx;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        for (as_idx = 0; as_idx < M3_MAX_AS; as_idx++) {
            if (M3_TRUE == pAsp->as_list[as_idx] &&
                rtctx == pAsp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_idx].rtctx) {
                return M3_FALSE;
            }
        }
    } else {
        for (as_idx = 0; as_idx < M3_MAX_AS; as_idx++) {
            if (M3_TRUE == pAsp->as_list[as_idx] &&
                rtctx == pAsp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_idx].rtctx) {
                return M3_FALSE;
            }
        }
    }
    return M3_TRUE;
}

