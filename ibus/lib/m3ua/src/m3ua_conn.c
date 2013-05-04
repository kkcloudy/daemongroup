/****************************************************************************
** Description:
** Code for provisioning connections.
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

#include <m3ua_conn.h>

extern m3_u32 m3_assoc_conn_map[M3_MAX_ASSOCID + 1];

m3_s32 m3ua_conn(m3_u32         lSpId,
                 m3_u32         rSpId,
                 m3_u8          oprn,
                 m3ua_conn_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;
    m3_r_asp_inf_t  *pRAsp;
    m3_r_sgp_inf_t  *pRSgp;
    m3_s32          retval;

    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        if (M3_FALSE == M3_ASPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_FALSE == pAsp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
    }
    else if (M3_TRUE == M3_IS_SP_SGP(lSpId)) {
        if (M3_FALSE == M3_SGPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1; 
        }
        pSgp = M3_SGP_TABLE(lSpId);
        if (M3_FALSE == pSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
    } 
    else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of local SP:%u", lSpId));
        M3ERRNO(EM3_INV_LSPID);
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
    } else if (M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        if (M3_FALSE == M3_R_SGPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1; 
        }
        pRSgp = M3_R_SGP_TABLE(rSpId);
        if (M3_FALSE == pRSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of remote SP:%u", rSpId));
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_SGP(lSpId) && M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        M3TRACE(m3uaErrorTrace, ("Both local and remote SP are SGP"));
        M3ERRNO(EM3_SGP_SGP_CONN);
        return -1;
    }
    switch(oprn)
    {
        case M3_ADD: {
            retval = m3uaAddConn(lSpId, rSpId, pInf);
            break;
        }
        case M3_DELETE: {
            retval = m3uaDeleteConn(lSpId, rSpId, pInf);
            break;
        }
        case M3_GET: {
            retval = m3uaGetConnConf(lSpId, rSpId, pInf);
            break;
        }
        case M3_MODIFY: {
            retval = m3uaModifyConnConf(lSpId, rSpId, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return retval;
}

m3_s32 m3ua_heartbeat(m3_u32             lSpId,
                      m3_u32             rSpId,
                      m3_u8              oprn,
                      void               *pInf)
{
    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;
    m3_r_asp_inf_t  *pRAsp;
    m3_r_sgp_inf_t  *pRSgp;
    m3_conn_inf_t   *pconn;
    m3_u32          conn_id;

    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        if (M3_FALSE == M3_ASPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_FALSE == pAsp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
    } else if (M3_TRUE == M3_IS_SP_SGP(lSpId)) {
        if (M3_FALSE == M3_SGPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        pSgp = M3_SGP_TABLE(lSpId);
        if (M3_FALSE == pSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of local SP:%u", lSpId));
        M3ERRNO(EM3_INV_LSPID);
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
    } else if (M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        if (M3_FALSE == M3_R_SGPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        pRSgp = M3_R_SGP_TABLE(rSpId);
        if (M3_FALSE == pRSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of remote SP:%u", lSpId));
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_SGP(lSpId) && M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        M3TRACE(m3uaErrorTrace, ("Both local and remote SP are SGP"));
        M3ERRNO(EM3_SGP_SGP_CONN);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection between local SP:%u and remote SP:%u", lSpId, rSpId));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    switch(oprn) {
        case M3_ADD: {
            if (M3_FALSE != pconn->hbeat_sess_inf.e_state) {
                M3TRACE(m3uaErrorTrace, ("Heartbeat procedure already active between local SP:%u and remote SP:%u", lSpId, rSpId));
                M3ERRNO(EM3_HBEAT_PROC_ALRDY);
                return -1;
            }
            m3uaStartHBEAT(pconn);
            break;
        }
        case M3_DELETE: {
            if (M3_TRUE != pconn->hbeat_sess_inf.e_state) {
                M3TRACE(m3uaErrorTrace, ("No Heartbeat procedure active between local SP:%u and remote SP:%u", lSpId, rSpId));
                M3ERRNO(EM3_NO_HBEAT_PROC);
                return -1;
            }
            m3uaStopHBEAT(pconn);
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

m3_s32 m3ua_timer(m3_u8             oprn,
                  m3ua_timer_t      *pInf)
{
    m3_prot_timer_t  *ptimer = M3_TIMER_TABLE;

    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    switch(oprn) {
        case M3_GET: {
            switch(pInf->type) {
                case M3_TIMER_TYPE_ASPM: {
                    pInf->get.aspmtimer = ptimer->aspmtimer;
                    break;
                }
                case M3_TIMER_TYPE_PD: {
                    pInf->get.pdtimer = ptimer->pdtimer;
                    break;
                }
                case M3_TIMER_TYPE_HBEAT: {
                    pInf->get.hbeattimer = ptimer->hbeattimer;
                    break;
                }
                case M3_TIMER_TYPE_RKM: {
                    pInf->get.rkmtimer = ptimer->rkmtimer;
                    break;
                }
                default: {
                    M3TRACE(m3uaErrorTrace, ("Invalid timer type:%u specified", pInf->type));
                    M3ERRNO(EM3_INV_TIMER);
                    return -1;
                }
            }
            break;
        }
        case M3_MODIFY: {
            switch(pInf->type) {
                case M3_TIMER_TYPE_ASPM: {
                    ptimer->aspmtimer = pInf->modify.aspmtimer;
                    break;
                }
                case M3_TIMER_TYPE_PD: {
                    ptimer->pdtimer = pInf->modify.pdtimer;
                    break;
                }
                case M3_TIMER_TYPE_HBEAT: {
                    ptimer->hbeattimer = pInf->modify.hbeattimer;
                    break;
                }
                case M3_TIMER_TYPE_RKM: {
                    ptimer->rkmtimer = pInf->modify.rkmtimer;
                    break;
                }
                default: {
                    M3ERRNO(EM3_INV_TIMER);
                    return -1;
                }
            }
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation:%u specified", oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaAddConn(m3_u32        lSpId,
                   m3_u32        rSpId,
                   m3ua_conn_t   *pInf)
{
    iu_log_debug("------------ %s  %d: %s-------------\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;
    m3_u16           idx;

    M3TRACE(m3uaConfigTrace, ("Adding connection between local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
			/*conn_id = msc  0 , sgsn 1*/
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    iu_log_debug("conn_id = %d\n", conn_id);
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    iu_log_debug("conn_id = %d\n", conn_id);
    }
    if (M3_MAX_U32 != conn_id) {
        M3TRACE(m3uaErrorTrace, ("Connection already exists"));
        M3ERRNO(EM3_CONN_ALRDY);
        return -1;
    }
    if (-1 == (conn_id = m3uaGetFreeConn())) {
        M3TRACE(m3uaErrorTrace, ("No free Connection available"));
        M3ERRNO(EM3_CONN_TABLE_FULL);
        return -1;
    }
    iu_log_debug("WWWWWWWWW  conn_id = %d\n", conn_id);
    iu_log_debug("lSpId = %d, rSpId = %d\n",lSpId,rSpId);
    
    pconn = M3_CONN_TABLE(conn_id);
    pconn->e_state            = M3_TRUE;
    pconn->assoc_id           = pInf->add.info.assoc_id;
    pconn->i_str              = pInf->add.info.i_str;
    pconn->o_str              = pInf->add.info.o_str;
    pconn->l_sp_id            = lSpId;
    pconn->r_sp_id            = rSpId;
    pconn->conn_state         = M3_CONN_NOT_ESTB;
    pconn->l_sp_g_st          = M3_ASP_DOWN;
    pconn->r_sp_g_st          = M3_ASP_DOWN;

    /* JUNE 2010 */
    pconn->opt                = M3_CONN_DEF_OPT;

    for (idx = 0; idx < M3_MAX_AS; idx++)
        pconn->l_sp_st[idx] = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++)
        pconn->r_sp_st[idx] = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        pconn->aspm_sess_inf[idx].e_state = M3_FALSE;
        pconn->aspm_sess_inf[idx].num_retry = 0;
    }
    pconn->hbeat_sess_inf.e_state = M3_FALSE;
    pconn->hbeat_sess_inf.num_retry = 0;
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            pAsp->r_asp_inf[(m3_u16)rSpId].conn_id = conn_id;
            iu_log_debug("pAsp->r_asp_inf[%d].conn_id = %d\n", (m3_u16)rSpId, pAsp->r_asp_inf[(m3_u16)rSpId].conn_id);
        } else {
            pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id = conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        pSgp->r_asp_inf[(m3_u16)rSpId].conn_id = conn_id;
    }
    iu_log_debug("aaaaaaaaaaaaaaa  conn_id = %d\n", conn_id);
    iu_log_debug("lSpId = %d, rSpId = %d\n",lSpId,rSpId);
    m3_assoc_conn_map[pconn->assoc_id] = pconn->conn_id;
    return conn_id;
}

m3_s32 m3uaDeleteConn(m3_u32        lSpId,
                      m3_u32        rSpId,
                      m3ua_conn_t   *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;
    m3_u8            idx;

    M3TRACE(m3uaConfigTrace, ("Deleting connection between local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
            if (M3_MAX_U32 != conn_id)
                m3uaASPM(M3_CONN_TABLE(conn_id), M3_ASPM_EVT_RECV_ASPDN_ACK, NULL);
            pAsp->r_asp_inf[(m3_u16)rSpId].conn_id = M3_MAX_U32;
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                pAsp->r_asp_inf[(m3_u16)rSpId].rc_inf[idx].rtctx = M3_MAX_U32;
                pAsp->r_asp_inf[(m3_u16)rSpId].rc_inf[idx].dyn_reg_state = M3_RK_STATIC;
            }
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
            if (M3_MAX_U32 != conn_id)
                m3uaASPM(M3_CONN_TABLE(conn_id), M3_ASPM_EVT_RECV_ASPDN_ACK, NULL);
            pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id = M3_MAX_U32;
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                pAsp->r_sgp_inf[(m3_u16)rSpId].rc_inf[idx].rtctx = M3_MAX_U32;
                pAsp->r_sgp_inf[(m3_u16)rSpId].rc_inf[idx].dyn_reg_state = M3_RK_STATIC;
            }
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
        if (M3_MAX_U32 != conn_id)
            m3uaASPM(M3_CONN_TABLE(conn_id), M3_ASPM_EVT_RECV_ASPDN_ACK, NULL);
        pSgp->r_asp_inf[(m3_u16)rSpId].conn_id = M3_MAX_U32;
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            pSgp->r_asp_inf[(m3_u16)rSpId].rc_inf[idx].rtctx = M3_MAX_U32;
            pSgp->r_asp_inf[(m3_u16)rSpId].rc_inf[idx].dyn_reg_state = M3_RK_STATIC;
        }
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pconn->aspm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pconn, idx);
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pconn->rkm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pconn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pconn, idx);
        }
    }
    pconn->e_state     = M3_FALSE;
    m3_assoc_conn_map[pconn->assoc_id] = M3_MAX_CONN;
    return 0;
}

m3_s32 m3uaGetConnConf(m3_u32       lSpId,
                       m3_u32       rSpId,
                       m3ua_conn_t  *pInf)
{
iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;

    M3TRACE(m3uaConfigTrace, ("Reading configuration of connection btw local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
iu_log_debug("conn_id = %d\n", conn_id);
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
iu_log_debug("conn_id = %d\n", conn_id);        
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    pInf->get.info.assoc_id  = pconn->assoc_id;
    pInf->get.info.i_str     = pconn->i_str;
    pInf->get.info.o_str     = pconn->o_str;
    return 0;
}

m3_s32 m3uaModifyConnConf(m3_u32       lSpId,
                          m3_u32       rSpId,
                          m3ua_conn_t  *pInf)
{
iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;

    M3TRACE(m3uaConfigTrace, ("Changing config of connection btw local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }

    // sourceforge bug id :- 3233866
    pconn = M3_CONN_TABLE(conn_id);

    switch(pInf->modify.confname)
    {
        case M3_CONN_ASSOC: {
            M3TRACE(m3uaConfigTrace, ("Changing Association Id"));
            m3_assoc_conn_map[pInf->modify.info.assoc_id] = conn_id;
            if ( m3_assoc_conn_map[pconn->assoc_id] == conn_id ) {
                m3_assoc_conn_map[pconn->assoc_id] = M3_MAX_CONN;
            }
            pconn->assoc_id = pInf->modify.info.assoc_id;
            break;
        }
        case M3_CONN_I_STR: {
            M3TRACE(m3uaConfigTrace, ("Changing Number of Incoming streams"));
            pconn->i_str = pInf->modify.info.i_str;
            break;
        }
        case M3_CONN_O_STR: {
            M3TRACE(m3uaConfigTrace, ("Changing Number of Outgoing streams"));
            pconn->o_str = pInf->modify.info.o_str;
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid Configuration:%u", pInf->modify.confname));
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3ua_conn_state(m3_u32            lSpId,
                       m3_u32            rSpId,
                       m3_u8             oprn,
                       m3ua_conn_state_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            retval = 0;
    m3_asp_inf_t      *pAsp;
    m3_sgp_inf_t      *pSgp;
    m3_r_asp_inf_t    *pRAsp;
    m3_r_sgp_inf_t    *pRSgp;

    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        if (M3_FALSE == M3_ASPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_FALSE == pAsp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
    } else if (M3_TRUE == M3_IS_SP_SGP(lSpId)) {
        if (M3_FALSE == M3_SGPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        pSgp = M3_SGP_TABLE(lSpId);
        if (M3_FALSE == pSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid local SP:%u type", lSpId));
        M3ERRNO(EM3_INV_LSPID);
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
    } else if (M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        if (M3_FALSE == M3_R_SGPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        pRSgp = M3_R_SGP_TABLE(rSpId);
        if (M3_FALSE == pRSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid remote SP:%u type", lSpId));
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_SGP(lSpId) && M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        M3TRACE(m3uaErrorTrace, ("Both local and remote SP are SGP"));
        M3ERRNO(EM3_SGP_SGP_CONN);
        return -1;
    }
    switch(oprn)
    {
        case M3_GET: {
            retval = m3uaGetConnState(lSpId, rSpId, pInf);
            break;
        }
        case M3_MODIFY: {
            retval = m3uaModifyConnState(lSpId, rSpId, pInf);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation:%u", (m3_u32)oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return retval;
}

m3_s32 m3uaGetConnState(m3_u32            lSpId,
                        m3_u32            rSpId,
                        m3ua_conn_state_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;

    M3TRACE(m3uaConfigTrace, ("Reading state of connection btw local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        }
        else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No Connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    pInf->get.state = pconn->conn_state;
iu_log_debug("pInf->get.state = %d\n", pInf->get.state);    
    return 0;
}

m3_s32 m3uaModifyConnState(m3_u32            lSpId,
                           m3_u32            rSpId,
                           m3ua_conn_state_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn;
    m3_u32           conn_id;
    m3_asp_inf_t     *pAsp;
    m3_sgp_inf_t     *pSgp;

    M3TRACE(m3uaConfigTrace, ("Changing state of connection btw local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    switch(pconn->conn_state) {
        case M3_CONN_NOT_ESTB: {
            if (M3_CONN_ESTB != pInf->modify.state) {
                M3TRACE(m3uaErrorTrace, ("Changing state from non-estb to other than estb"));
                M3ERRNO(EM3_INV_CONNSTATE);
                return -1;
            }
            pconn->conn_state = pInf->modify.state;
            break;
        }
        case M3_CONN_ESTB: {
            pconn->conn_state = pInf->modify.state;
iu_log_debug("pconn->conn_state = %d\n", pconn->conn_state);            
            break;
        }
        case M3_CONN_CONG_1:
        case M3_CONN_CONG_2:
        case M3_CONN_CONG_3: {
            pconn->conn_state = pInf->modify.state;
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid connection state:%u", pconn->conn_state));
            M3ERRNO(EM3_INV_CONNSTATE);
            return -1;
        }
    }
    m3ua_ECONNSTATE(pconn, pInf);
    return 0;
}

m3_s32 m3uaSetASPMMsgInf(m3_conn_inf_t      *pconn,
                         m3_u8              sess_id,
                         m3_aspm_msg_inf_t  *p_info)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    pconn->aspm_sess_inf[sess_id].e_state = M3_TRUE;
    pconn->aspm_sess_inf[sess_id].msg_inf.msg_type = p_info->msg_type;
    pconn->aspm_sess_inf[sess_id].msg_inf.msg_len = p_info->msg_len;
    pconn->aspm_sess_inf[sess_id].msg_inf.msg = p_info->msg;
    return 0;
}

m3_s32 m3uaGetSessFromMsgType(m3_conn_inf_t  *pconn,
                              m3_msg_type_t  msg_type,
                              m3_u8          *p_nsess,
                              m3_u8          *p_sess_list)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;

    *p_nsess = 0;
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pconn->aspm_sess_inf[idx].e_state) {
            if (msg_type == pconn->aspm_sess_inf[idx].msg_inf.msg_type) {
                p_sess_list[*p_nsess] = idx;
                (*p_nsess) += 1;
            }
        }
    }
    return 0;
}

m3_s32 m3uaDeleteConnSess(m3_conn_inf_t *pconn,
                          m3_u8         sess_id)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    pconn->aspm_sess_inf[sess_id].e_state  = M3_FALSE;
    pconn->aspm_sess_inf[sess_id].timer_inf.timer_id = M3_MAX_U32;
    if (M3_NULL != pconn->aspm_sess_inf[sess_id].timer_inf.p_buf) {
        pconn->aspm_sess_inf[sess_id].timer_inf.p_buf = M3_NULL;
    }
    pconn->aspm_sess_inf[sess_id].msg_inf.msg_len = 0;
    if (M3_NULL !=  pconn->aspm_sess_inf[sess_id].msg_inf.msg) {
        M3_MSG_FREE(pconn->aspm_sess_inf[sess_id].msg_inf.msg, M3_ASPM_POOL);
        pconn->aspm_sess_inf[sess_id].msg_inf.msg = M3_NULL;
    }
    pconn->aspm_sess_inf[sess_id].num_retry = 0;
    return 0;
}

m3_s32 m3uaDeleteRKMSess(m3_conn_inf_t *pconn,
                         m3_u8         sess_id)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    pconn->rkm_sess_inf[sess_id].e_state  = M3_FALSE;
    pconn->rkm_sess_inf[sess_id].timer_inf.timer_id = M3_MAX_U32;
    if (M3_NULL != pconn->rkm_sess_inf[sess_id].timer_inf.p_buf) {
        pconn->rkm_sess_inf[sess_id].timer_inf.p_buf = M3_NULL;
    }
    pconn->rkm_sess_inf[sess_id].msg_inf.msg_len = 0;
    if (M3_NULL !=  pconn->rkm_sess_inf[sess_id].msg_inf.msg) {
        M3_MSG_FREE(pconn->rkm_sess_inf[sess_id].msg_inf.msg, M3_ASPM_POOL);
        pconn->rkm_sess_inf[sess_id].msg_inf.msg = M3_NULL;
    }
    pconn->rkm_sess_inf[sess_id].num_retry = 0;
    return 0;
}

m3_s32 m3uaDeleteHBEATSess(m3_conn_inf_t *pconn)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    pconn->hbeat_sess_inf.e_state  = M3_FALSE;
    pconn->hbeat_sess_inf.timer_inf.timer_id = M3_MAX_U32;
    if (M3_NULL != pconn->hbeat_sess_inf.timer_inf.p_buf) {
        pconn->hbeat_sess_inf.timer_inf.p_buf = M3_NULL;
    }
    pconn->hbeat_sess_inf.msg_inf.msg_len = 0;
    if (M3_NULL !=  pconn->hbeat_sess_inf.msg_inf.msg) {
        M3_MSG_FREE(pconn->hbeat_sess_inf.msg_inf.msg, M3_ASPM_POOL);
        pconn->hbeat_sess_inf.msg_inf.msg = M3_NULL;
    }
    pconn->hbeat_sess_inf.num_retry = 0;
    return 0;
}

m3_s32 m3uaGetFreeSess(m3_conn_inf_t *pconn,
                       m3_u8         *p_sessid)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;
    m3_s32        retval = 0;

    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_FALSE == pconn->aspm_sess_inf[idx].e_state) {
            break;
        }
    }
    if (M3_MAX_ASPM_SESS_PER_CONN > idx) {
        *p_sessid = idx;
    } else {
        *p_sessid = M3_MAX_U8;
        retval = -1;
    }
    return retval;
}

m3_s32 m3uaGetFreeRKMSess(m3_conn_inf_t *pconn,
                          m3_u8         *p_sessid)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;
    m3_s32        retval = 0;

    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_FALSE == pconn->rkm_sess_inf[idx].e_state) {
            break;
        }
    }
    if (M3_MAX_RKM_SESS_PER_CONN > idx) {
        *p_sessid = idx;
    } else {
        *p_sessid = M3_MAX_U8;
        retval = -1;
    }
    return retval;
}

m3_s32 m3uaGetFreeConn()
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u32        idx;

    for (idx = 0; idx < M3_MAX_CONN; idx++) {
        if (M3_FALSE == (M3_CONN_TABLE(idx))->e_state){
            iu_log_debug("idx = %d\n",idx);
            return idx;
        }
    }
    return -1;
}


/* JUNE 2010 */

m3_s32 m3uaGetConnOpt(m3_u32            lSpId,
                      m3_u32            rSpId,
                      m3ua_conn_opt_t   *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn = NULL;
    m3_u32           conn_id = M3_MAX_U32;
    m3_asp_inf_t     *pAsp = NULL;
    m3_sgp_inf_t     *pSgp = NULL;

    M3TRACE(m3uaConfigTrace, ("Reading connection options btw local SP:%u and remote SP:%u", lSpId, rSpId));
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        }
        else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No Connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    pInf->get.opt = pconn->opt;
    return 0;
}

m3_s32 m3uaModifyConnOpt(m3_u32            lSpId,
                         m3_u32            rSpId,
                         m3ua_conn_opt_t   *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_conn_inf_t    *pconn = NULL;
    m3_u32           conn_id = M3_MAX_U32;
    m3_asp_inf_t     *pAsp = NULL;
    m3_sgp_inf_t     *pSgp = NULL;

    M3TRACE(m3uaConfigTrace, ("Changing connection options:%u btw local SP:%u and remote SP:%u", (unsigned int)pInf->modify.opt, lSpId, rSpId));
    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_TRUE == M3_IS_SP_ASP(rSpId)) {
            conn_id = pAsp->r_asp_inf[(m3_u16)rSpId].conn_id;
        } else {
            conn_id = pAsp->r_sgp_inf[(m3_u16)rSpId].conn_id;
        }
    } else {
        pSgp = M3_SGP_TABLE(lSpId);
        conn_id = pSgp->r_asp_inf[(m3_u16)rSpId].conn_id;
    }
    if (M3_MAX_U32 == conn_id) {
        M3TRACE(m3uaErrorTrace, ("No connection exists"));
        M3ERRNO(EM3_INV_CONN);
        return -1;
    }
    pconn = M3_CONN_TABLE(conn_id);
    pconn->opt = pInf->modify.opt;
    return 0;
}


m3_s32 m3ua_conn_opt (m3_u32           lSpId,
                      m3_u32           rSpId,
                      m3_u8            oprn,
                      m3ua_conn_opt_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;
    m3_r_asp_inf_t  *pRAsp;
    m3_r_sgp_inf_t  *pRSgp;
    m3_s32          retval;

    if (M3_TRUE == M3_IS_SP_ASP(lSpId)) {
        if (M3_FALSE == M3_ASPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pAsp = M3_ASP_TABLE(lSpId);
        if (M3_FALSE == pAsp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned ASP:%u", lSpId));
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
    } else if (M3_TRUE == M3_IS_SP_SGP(lSpId)) {
        if (M3_FALSE == M3_SGPID_VALID(lSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1; 
        }
        pSgp = M3_SGP_TABLE(lSpId);
        if (M3_FALSE == pSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned SGP:%u", lSpId));
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of local SP:%u", lSpId));
        M3ERRNO(EM3_INV_LSPID);
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
    } else if (M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        if (M3_FALSE == M3_R_SGPID_VALID(rSpId)) {
            M3TRACE(m3uaErrorTrace, ("Invalid remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1; 
        }
        pRSgp = M3_R_SGP_TABLE(rSpId);
        if (M3_FALSE == pRSgp->e_state) {
            M3TRACE(m3uaErrorTrace, ("Non-provisioned remote SGP:%u", rSpId));
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
    } else {
        M3TRACE(m3uaErrorTrace, ("Invalid type of remote SP:%u", rSpId));
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_SGP(lSpId) && M3_TRUE == M3_IS_SP_SGP(rSpId)) {
        M3TRACE(m3uaErrorTrace, ("Both local and remote SP are SGP"));
        M3ERRNO(EM3_SGP_SGP_CONN);
        return -1;
    }
    switch(oprn)
    {
        case M3_GET: {
            retval = m3uaGetConnOpt(lSpId, rSpId, pInf);
            break;
        }
        case M3_MODIFY: {
            retval = m3uaModifyConnOpt(lSpId, rSpId, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return retval;
}

