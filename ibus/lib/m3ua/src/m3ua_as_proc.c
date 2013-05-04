/****************************************************************************
** Description: This file has remote AS FSM and handles following two events
**              1) remote ASP state change
**              2) Pending Timer expiry
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

#include <m3ua_as_proc.h>

m3_s32  m3uaStartPDTimer(m3_r_as_inf_t  *pas,
                         m3_u32         l_spid)
{
    m3_u32    timerid = M3_MAX_U32;
    m3_timer_param_t *pbuf;

    M3TRACE(m3uaAspmTrace, ("Starting AS Pending timer for AS:%u", pas->as_id));
    pbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
           M3_TIMER_POOL);
    if (M3_NULL == pbuf) {
        M3ERRNO(EM3_MALLOC_FAIL); 
        return -1;
    }
    pbuf->type = M3_TIMER_TYPE_PD;
    pbuf->param.pdbuf.as_id = pas->as_id;
    pbuf->param.pdbuf.l_spid = l_spid;
    if (-1 == m3uaStartTimer(M3_PD_TIMER_INT, pbuf, &timerid)) {
        M3TRACE(m3uaErrorTrace, ("Failed to start AS Pending timer"));
        return -1;
    }
    M3TRACE(m3uaAspmTrace, ("Pending Timer Id:%u", timerid));
    m3uaSetPDTimerId(pas->as_id, l_spid, timerid);
    return 0;
}

m3_s32     m3ua_RASDOWN_EASPINACTIVE(m3_conn_inf_t *pconn,
                                     m3_r_as_inf_t *pas,
                                     void          *p_edata)
{
    m3_ntfy_inf_t        ntfy;

    M3TRACE(m3uaAspmTrace, ("ASP:%d Inactive in AS:%d Down",(int)pconn->l_sp_id, (int)pas->as_id));
    m3uaSetRASState(pas->as_id, pconn->l_sp_id, M3_AS_INACTIVE);
    m3uaNtfyRASState(pas, pconn->l_sp_id, M3_AS_INACTIVE);
    ntfy.status_type   = M3_STATUS_TYP_AS_STATE_CHG;
    ntfy.status_inf    = M3_STATUS_INF_AS_INACTIVE;
    ntfy.m3asp_id      = M3_MAX_U32;
    ntfy.num_rc        = 1;
    //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
    ntfy.info_len      = 0;
	/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    return 0;
}

m3_s32     m3ua_RASINACTIVE_EASPDOWN(m3_conn_inf_t *pconn,
                                     m3_r_as_inf_t *pas,
                                     void          *p_edata)
{
    m3_u16               idx;
    m3_as_state_t        as_state = M3_AS_DOWN;
    m3_conn_inf_t        *prconn;
    m3_asp_inf_t         *pasp;
    m3_sgp_inf_t         *psgp;
    m3_as_state_t        *pas_state;
    m3_ntfy_inf_t        ntfy;
    m3_r_asp_inf_t       *pRAsp = M3_R_ASP_TABLE(pconn->r_sp_id);

    M3TRACE(m3uaAspmTrace, ("ASP:%d Down in AS:%d Inactive",(int)pconn->l_sp_id,(int)pas->as_id));
    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        pas_state = &pasp->r_as_inf[pas->as_id].state;
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pasp->r_as_inf[pas->as_id].asp_list[idx]  &&
                M3_MAX_U32 != pasp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[idx].conn_id);
                if (M3_ASP_INACTIVE == prconn->r_sp_st[pas->as_id]) {
                    as_state = M3_AS_INACTIVE;
                    break;
                }
            }
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        pas_state = &psgp->r_as_inf[pas->as_id].state;
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == psgp->r_as_inf[pas->as_id].asp_list[idx]  &&
                M3_MAX_U32 != psgp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[idx].conn_id);
                if (M3_ASP_INACTIVE == prconn->r_sp_st[pas->as_id]) {
                    as_state = M3_AS_INACTIVE;
                    break;
                }   
            }   
        }
    }
    if (M3_AS_DOWN == as_state) {
        M3TRACE(m3uaAspmTrace, ("AS state changed from Inactive -> Down"));
        *pas_state = as_state;
         m3uaNtfyRASState(pas, pconn->l_sp_id, as_state);
    }
    /* ASP Failure notification to (in)active ASPs */
    ntfy.status_type   = M3_STATUS_TYP_OTHER;
    ntfy.status_inf    = M3_STATUS_INF_ASP_FAILURE;
    ntfy.m3asp_id      = pRAsp->m3asp_id;
    ntfy.num_rc        = 1;
    //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
    ntfy.info_len      = 0;
	/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    return 0;
}

m3_s32     m3ua_RASINACTIVE_EASPACTIVE(m3_conn_inf_t *pconn,
                                       m3_r_as_inf_t *pas,
                                       void          *p_edata)
{
    m3_ntfy_inf_t    ntfy;

    M3TRACE(m3uaAspmTrace, ("ASP Active in AS Inactive"));
    m3uaSetRASState(pas->as_id, pconn->l_sp_id, M3_AS_ACTIVE);
    M3TRACE(m3uaAspmTrace, ("AS state changed from Inactive -> Active"));
    m3uaNtfyRASState(pas, pconn->l_sp_id, M3_AS_ACTIVE);
    ntfy.status_type   = M3_STATUS_TYP_AS_STATE_CHG;
    ntfy.status_inf    = M3_STATUS_INF_AS_ACTIVE;
    ntfy.m3asp_id      = M3_MAX_U32;
    ntfy.num_rc        = 1;
    //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
    ntfy.info_len      = 0;
	/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    return 0;
}

m3_s32     m3ua_RASACTIVE_EASPDOWN(m3_conn_inf_t *pconn,
                                   m3_r_as_inf_t *pas,
                                   void          *p_edata)
{
    m3_ntfy_inf_t        ntfy;
    m3_as_state_t        as_state = M3_AS_PENDING;
    m3_u16               idx;
    m3_u16               num_asp_active = 0;
    m3_conn_inf_t        *prconn;
    m3_asp_inf_t         *pasp;
    m3_sgp_inf_t         *psgp;
    m3_bool_t            s_ntfy = M3_FALSE;
    m3_r_asp_inf_t       *pRAsp = M3_R_ASP_TABLE(pconn->r_sp_id);

    M3TRACE(m3uaAspmTrace, ("ASP Down in AS Active"));
    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pasp->r_as_inf[pas->as_id].asp_list[idx]   &&
                M3_MAX_U32 != pasp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == prconn->r_sp_st[pas->as_id]) {
                    num_asp_active++;
                }
            }
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == psgp->r_as_inf[pas->as_id].asp_list[idx]   &&
                M3_MAX_U32 != psgp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == prconn->r_sp_st[pas->as_id]) {
                    num_asp_active++;
                }
            }
        }
    }
    if (0 == num_asp_active) {
        s_ntfy = M3_TRUE;
        ntfy.status_type = M3_STATUS_TYP_AS_STATE_CHG;
        ntfy.status_inf  = M3_STATUS_INF_AS_PENDING;
        ntfy.m3asp_id      = M3_MAX_U32;
        m3uaSetRASState(pas->as_id, pconn->l_sp_id, as_state);
        m3uaNtfyRASState(pas, pconn->l_sp_id, as_state);
        m3uaStartPDTimer(pas, pconn->l_sp_id);
    } else if ((pas->rkey.trfmode == M3_TRFMODE_LOAD_SHARE      ||
              pas->rkey.trfmode == M3_TRFMODE_BROADCAST)      &&
             (pas->min_act_asp > num_asp_active)) {
        s_ntfy = M3_TRUE;
        ntfy.status_type = M3_STATUS_TYP_OTHER;
        ntfy.status_inf  = M3_STATUS_INF_INSUFF_ASP_ACT;
        ntfy.m3asp_id      = M3_MAX_U32;
    } else {
        s_ntfy = M3_TRUE;
        /* ASP Failure notification to (in)active ASPs */
        ntfy.status_type   = M3_STATUS_TYP_OTHER;
        ntfy.status_inf    = M3_STATUS_INF_ASP_FAILURE;
        ntfy.m3asp_id      = pRAsp->m3asp_id;
    }
    if (M3_TRUE == s_ntfy) {
        ntfy.num_rc        = 1;
        //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
        ntfy.info_len      = 0;
		/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    }
    return 0;
}

m3_s32     m3ua_RASACTIVE_EASPINACTIVE(m3_conn_inf_t *pconn,
                                       m3_r_as_inf_t *pas,
                                       void          *p_edata)
{
    m3_ntfy_inf_t        ntfy;
    m3_as_state_t        as_state = M3_AS_PENDING;
    m3_u16               idx;
    m3_u16               num_asp_active = 0;
    m3_bool_t            s_ntfy = M3_FALSE;
    m3_conn_inf_t        *prconn;
    m3_asp_inf_t         *pasp;
    m3_sgp_inf_t         *psgp;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pasp->r_as_inf[pas->as_id].asp_list[idx]   &&
                M3_MAX_U32 != pasp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == prconn->r_sp_st[pas->as_id]) {
                    num_asp_active++;
                }
            }
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == psgp->r_as_inf[pas->as_id].asp_list[idx]   &&
                M3_MAX_U32 != psgp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == prconn->r_sp_st[pas->as_id]) {
                    num_asp_active++;
                }
            }
        }
    }
    if (0 == num_asp_active) {
        s_ntfy = M3_TRUE;
        ntfy.status_type = M3_STATUS_TYP_AS_STATE_CHG;
        ntfy.status_inf  = M3_STATUS_INF_AS_PENDING;
        m3uaSetRASState(pas->as_id, pconn->l_sp_id, as_state);
        m3uaNtfyRASState(pas, pconn->l_sp_id, as_state);
        m3uaStartPDTimer(pas, pconn->l_sp_id);
    } else if ((pas->rkey.trfmode == M3_TRFMODE_LOAD_SHARE      ||
              pas->rkey.trfmode == M3_TRFMODE_BROADCAST)      &&
             (pas->min_act_asp > num_asp_active)) {
        s_ntfy = M3_TRUE;
        ntfy.status_type = M3_STATUS_TYP_OTHER;
        ntfy.status_inf  = M3_STATUS_INF_INSUFF_ASP_ACT;
    }
    if (M3_TRUE == s_ntfy) {
        ntfy.m3asp_id      = M3_MAX_U32;
        ntfy.num_rc        = 1;
        //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
        ntfy.info_len      = 0;
		/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    }
    return 0;
}

m3_s32     m3ua_RASACTIVE_EASPACTIVE(m3_conn_inf_t *pconn,
                                     m3_r_as_inf_t *pas,
                                     void          *p_edata)
{
    m3_ntfy_inf_t        ntfy;
    m3_u8                num_asp = 0;
    m3_u32               asp_list[M3_MAX_R_ASP];
    m3_bool_t            s_ntfy = M3_FALSE;
    m3_asp_inf_t         *pasp;
    m3_sgp_inf_t         *psgp;
    m3_conn_inf_t        *prconn;

    if (M3_TRFMODE_OVERRIDE == pas->rkey.trfmode) {
        s_ntfy = M3_TRUE;
        ntfy.status_type = M3_STATUS_TYP_OTHER;
        ntfy.status_inf  = M3_STATUS_INF_ALT_ASP_ACT;
        m3uaGetASPsWithState(pas, pconn->l_sp_id,
            M3_ASP_ACTIVE, &num_asp, asp_list);

        if ( 2 != num_asp ) {
            M3TRACE(m3uaErrorTrace,("For Override AS, Invalid no. of ASP Active:%d",(int)num_asp));
            return 0;
        }

        if (asp_list[0] == pconn->r_sp_id) {
            if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                pasp = M3_ASP_TABLE(pconn->l_sp_id);
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[asp_list[1]].conn_id);
            } else {
                psgp = M3_SGP_TABLE(pconn->l_sp_id);
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[asp_list[1]].conn_id);
            }
        } else {
            if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                pasp = M3_ASP_TABLE(pconn->l_sp_id);
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[asp_list[0]].conn_id);
            } else {
                psgp = M3_SGP_TABLE(pconn->l_sp_id);
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[asp_list[0]].conn_id);
            }
        }
        prconn->r_sp_st[pas->as_id] = M3_ASP_INACTIVE;
        m3uaNtfyRASPState(prconn, pas->as_id, M3_ASP_INACTIVE);
    }
    if (M3_TRUE == s_ntfy) {
        ntfy.m3asp_id      = M3_MAX_U32;
        ntfy.num_rc        = 1;
        ntfy.rc_list[0]    = m3uaGetRCFromRAS(prconn, pas->as_id);
        ntfy.info_len      = 0;
		/* book modify, 2011-12-09 */
    m3uaSendNtfy(pas, prconn, prconn->l_sp_id, &ntfy);
    }
    return 0;
}

m3_s32     m3ua_RASPEND_EASPACTIVE(m3_conn_inf_t *pconn,
                                   m3_r_as_inf_t *pas,
                                   void          *p_edata)
{
    m3_ntfy_inf_t    ntfy;
    m3_timer_inf_t   timer_inf;
    m3_u32           buflen;
    m3_u8            *p_buf = M3_NULL;
    m3_u32           stream_id;
    m3_u8            sls;
    m3_msg_tlv_t     *ptlv;
    m3_u32           offset = M3_MSG_HDR_LEN;
    m3_u16           max_sls = m3uaGetMaxSLS(pconn->l_sp_id,
                               pas->rkey.nw_app);

    m3uaGetPDTimerId(pas->as_id, pconn->l_sp_id, &timer_inf.timer_id);
    m3uaFreeTimer(&timer_inf);
    m3uaSetRASState(pas->as_id, pconn->l_sp_id, M3_AS_ACTIVE);
    m3uaNtfyRASState(pas, pconn->l_sp_id, M3_AS_ACTIVE);
    ntfy.status_type = M3_STATUS_TYP_AS_STATE_CHG;
    ntfy.status_inf  = M3_STATUS_INF_AS_ACTIVE;
    ntfy.m3asp_id      = M3_MAX_U32;
    ntfy.num_rc        = 1;
    //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
    ntfy.info_len      = 0;
//    m3uaSendNtfy(pas, pconn, pconn->l_sp_id, &ntfy);
    while (-1 != m3uaGetPDBuf(pas->as_id,pconn->l_sp_id,&buflen,&p_buf)) {
        ptlv = (m3_msg_tlv_t*)(p_buf + offset);
        if (M3_TAG_NW_APP == (ptlv->tag)) {
            offset += 8;
            ptlv = (m3_msg_tlv_t*)(p_buf + offset);
        }
        if (M3_TAG_RT_CTX == (ptlv->tag)) {
            offset += 8;
        }
        sls = p_buf[offset + 11];
        stream_id = (sls * (pconn->o_str - 1))/max_sls;
        stream_id++;
        m3uaSendMsg(pconn, stream_id, buflen, p_buf);
        M3_MSG_FREE(p_buf, M3_TXR_POOL);
    }
    return 0;
}

m3_s32     m3ua_RASPEND_ETIMEREXP(m3_conn_inf_t *pconn,
                                  m3_r_as_inf_t *pas,
                                  void          *p_edata)
{
    m3_timer_inf_t    *p_inf = (m3_timer_inf_t*)p_edata;
    m3_timer_param_t  *ptimerbuf = (m3_timer_param_t*)p_inf->p_buf;
    m3_as_state_t     as_state = M3_AS_DOWN;
    m3_u32            l_spid = ptimerbuf->param.pdbuf.l_spid;
    m3_ntfy_inf_t     ntfy;
    m3_conn_inf_t     *prconn;
    m3_u16            idx;
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;

    M3_MSG_FREE(p_inf->p_buf, M3_TIMER_POOL);
    m3uaDeletePDQ(pas->as_id, l_spid);
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        pasp = M3_ASP_TABLE(l_spid);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == pasp->r_as_inf[pas->as_id].asp_list[idx]  && 
                M3_MAX_U32 != pasp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(pasp->r_asp_inf[idx].conn_id);
                if (M3_ASP_INACTIVE == prconn->r_sp_st[pas->as_id]) {
                    as_state = M3_AS_INACTIVE;
                    break;
                }
            }
        }
    } else {
        psgp = M3_SGP_TABLE(l_spid);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == psgp->r_as_inf[pas->as_id].asp_list[idx]  && 
                M3_MAX_U32 != psgp->r_asp_inf[idx].conn_id) {
                prconn = M3_CONN_TABLE(psgp->r_asp_inf[idx].conn_id);
                if (M3_ASP_INACTIVE == prconn->r_sp_st[pas->as_id]) {
                    as_state = M3_AS_INACTIVE;
                    break;
                }
            }
        }
    }
    if (M3_AS_INACTIVE == as_state) {
        ntfy.status_type = M3_STATUS_TYP_AS_STATE_CHG;
        ntfy.status_inf  = M3_STATUS_INF_AS_INACTIVE;
        m3uaSetRASState(pas->as_id, l_spid, M3_AS_INACTIVE);
        m3uaNtfyRASState(pas, l_spid, M3_AS_INACTIVE);
        ntfy.m3asp_id      = M3_MAX_U32;
        ntfy.num_rc        = 1;
        //ntfy.rc_list[0]    = m3uaGetRCFromRAS(pconn, pas->as_id);
        ntfy.info_len      = 0;
//        m3uaSendNtfy(pas, pconn, l_spid, &ntfy);
    } else {
        m3uaSetRASState(pas->as_id, l_spid, as_state);
        m3uaNtfyRASState(pas, l_spid, as_state);
    }
    return 0;
}

m3uaRAsProcFp_t  m3ua_RAS_FP[M3_MAX_AS_STATE][M3_MAX_AS_EVENT] =
{
    {
        M3_NULL,                       /* ASP STATE DOWN */
        m3ua_RASDOWN_EASPINACTIVE,     /* ASP STATE INACTIVE */
        M3_NULL,                       /* ASP STATE ACTIVE */
        M3_NULL                        /* PENDING TIMER EXPIRY */
    },

    {
        m3ua_RASINACTIVE_EASPDOWN,     /* ASP STATE DOWN */
        M3_NULL,                       /* ASP STATE INACTIVE */
        m3ua_RASINACTIVE_EASPACTIVE,   /* ASP STATE ACTIVE */
        M3_NULL                        /* PENDING TIMER EXPIRY */
    },

    {
        m3ua_RASACTIVE_EASPDOWN,       /* ASP STATE DOWN */
        m3ua_RASACTIVE_EASPINACTIVE,   /* ASP STATE INACTIVE */
        m3ua_RASACTIVE_EASPACTIVE,     /* ASP STATE ACTIVE */
        M3_NULL                        /* PENDING TIMER EXPIRY */
    },

    {
        M3_NULL,                        /* ASP STATE DOWN */
        M3_NULL,                        /* ASP STATE INACTIVE */
        m3ua_RASPEND_EASPACTIVE,        /* ASP STATE ACTIVE */
        m3ua_RASPEND_ETIMEREXP          /* PENDING TIMER EXPIRY */
    }
};

m3_s32     m3uaRAS(m3_conn_inf_t   *pconn,
                   m3_u32          as_id,
                   m3_as_event_t   event,
                   void            *p_edata)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_s32           ret = 0;
    m3_as_state_t    as_state;
    m3_r_as_inf_t    *pas = M3_R_AS_TABLE(as_id);
    m3_u32           lsp_id;
    m3_timer_inf_t   *p_inf;
    m3_timer_param_t *ptimerbuf;


    if (M3_NULL != pconn)
        lsp_id = pconn->l_sp_id;
    else {
        p_inf = (m3_timer_inf_t*)p_edata;
        ptimerbuf = (m3_timer_param_t*)p_inf->p_buf;
        lsp_id = ptimerbuf->param.pdbuf.l_spid;
    }
    iu_log_debug("as_id = %d, lsp_id = %d\n",as_id, lsp_id);
    m3uaGetRASState(as_id, lsp_id, &as_state);
    iu_log_debug("as_state = %d, event = %d\n",as_state, event);
    if (M3_NULL ==  m3ua_RAS_FP[as_state - 1][event]) {
        M3TRACE(m3uaErrorTrace, ("Invalid Event:%u,State:%u", (unsigned int)event, (unsigned int)as_state));
        M3ERRNO(EM3_INV_EVTSTATE);
        ret = -1;
    } else {
        ret =  m3ua_RAS_FP[as_state - 1][event](pconn, pas, p_edata);
    }
    return ret;
}

m3_s32  m3uaSendNtfy(m3_r_as_inf_t    *pas,
                     m3_conn_inf_t    *pconn,
                     m3_u32           lsp_id,
                     m3_ntfy_inf_t    *pntfy)
{
    m3_u8        num_asp_active = 0;
    m3_u8        num_asp_inactive = 0;
    m3_u32       asp_list[M3_MAX_R_ASP];
    m3_u32       conn_id;
    m3_u16       idx;
    m3_u32       msglen = 0;
    m3_u8        *p_msg;

    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_NTFY_SIZE, M3_MGMT_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    m3uaGetASPsWithState(pas, lsp_id, M3_ASP_ACTIVE,
         &num_asp_active, asp_list);
    m3uaGetASPsWithState(pas, lsp_id, M3_ASP_INACTIVE,
         &num_asp_inactive, &asp_list[num_asp_active]);

    switch(pntfy->status_type) {
        case M3_STATUS_TYP_AS_STATE_CHG: {
            if (M3_TRUE == M3_IS_SP_ASP(lsp_id))
            for (idx = 0; idx < num_asp_active + num_asp_inactive; idx++) {
                conn_id =
                    (M3_ASP_TABLE(lsp_id))->r_asp_inf[asp_list[idx]].conn_id;
                pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                /* JUNE 2010 */
                if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                    pntfy->num_rc = 0;
                }
                if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                    pntfy->m3asp_id = M3_MAX_U32;
                }

                m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                m3uaSendMsg((M3_CONN_TABLE(conn_id)), 0, msglen, p_msg);
            }
            else 
            for (idx = 0; idx < num_asp_active + num_asp_inactive; idx++) {
                conn_id =
                    (M3_SGP_TABLE(lsp_id))->r_asp_inf[asp_list[idx]].conn_id;
                pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                /* JUNE 2010 */
                if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                    pntfy->num_rc = 0;
                }
                if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                    pntfy->m3asp_id = M3_MAX_U32;
                }

                m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                m3uaSendMsg((M3_CONN_TABLE(conn_id)), 0, msglen, p_msg);
            }
            break;
        }
        case M3_STATUS_TYP_OTHER:
        {
            switch(pntfy->status_inf) {
                case M3_STATUS_INF_INSUFF_ASP_ACT: {
                    if (M3_TRUE == M3_IS_SP_ASP(lsp_id))
                    for (idx = 0; idx < num_asp_inactive; idx++) {
                        conn_id = (M3_ASP_TABLE(lsp_id))->
                            r_asp_inf[asp_list[idx + num_asp_active]].conn_id;
                        pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                        /* JUNE 2010 */
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                            pntfy->num_rc = 0;
                        }
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                            pntfy->m3asp_id = M3_MAX_U32;
                        }

                        m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                        m3uaSendMsg((M3_CONN_TABLE(conn_id)), 0, msglen, p_msg);
                    }
                    else
                    for (idx = 0; idx < num_asp_inactive; idx++) {
                        conn_id = (M3_SGP_TABLE(lsp_id))->
                            r_asp_inf[asp_list[idx + num_asp_active]].conn_id;
                        pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                        /* JUNE 2010 */
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                            pntfy->num_rc = 0;
                        }
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                            pntfy->m3asp_id = M3_MAX_U32;
                        }

                        m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                        m3uaSendMsg((M3_CONN_TABLE(conn_id)), 0, msglen, p_msg);
                    }
                    break;
                }
                case M3_STATUS_INF_ALT_ASP_ACT: {
                    m3uaSendMsg(pconn, 0, msglen, p_msg);
                    break;
                }
                case M3_STATUS_INF_ASP_FAILURE: {
                    if (M3_TRUE == M3_IS_SP_ASP(lsp_id))
                    for (idx=0; idx<num_asp_active+num_asp_inactive; idx++) {
                        conn_id =
                            (M3_ASP_TABLE(lsp_id))->r_asp_inf[asp_list[idx]].conn_id;
                        pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                        /* JUNE 2010 */
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                            pntfy->num_rc = 0;
                        }
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                            pntfy->m3asp_id = M3_MAX_U32;
                        }

                        m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                        m3uaSendMsg((M3_CONN_TABLE(conn_id)),0,msglen,p_msg);
                    }
                    else
                    for (idx=0; idx<num_asp_active+num_asp_inactive; idx++) {
                        conn_id =
                            (M3_SGP_TABLE(lsp_id))->r_asp_inf[asp_list[idx]].conn_id;
                        pntfy->rc_list[0] = m3uaGetRCFromRAS((M3_CONN_TABLE(conn_id)), pas->as_id);

                        /* JUNE 2010 */
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_RTCTX) {
                            pntfy->num_rc = 0;
                        }
                        if ((M3_CONN_TABLE(conn_id))->opt & M3_CONN_EXCL_ASPID) {
                            pntfy->m3asp_id = M3_MAX_U32;
                        }

                        m3uaEncodeNTFY(pntfy, &msglen, p_msg);
                        m3uaSendMsg((M3_CONN_TABLE(conn_id)),0,msglen,p_msg);
                    }
                    break;
                }
            }
            break;
        }
    }
    M3_MSG_FREE(p_msg, M3_MGMT_POOL);
    return 0;
}

