/****************************************************************************
** Description:
** Common code for procedures.
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

#include <m3ua_co_proc.h>

m3_s32 m3uaStartASPMTimer(m3_conn_inf_t  *pconn,
                          m3_u32         sess_id,
                          void           *ptimerbuf,
                          m3_u32         interval)
{
    if (-1 == m3uaStartTimer(interval, ptimerbuf,
        &pconn->aspm_sess_inf[sess_id].timer_inf.timer_id)) {
        M3TRACE(m3uaErrorTrace, ("m3uaStartASPMTimer: Failed to start ASPM timer"));
        return -1;
    }
    pconn->aspm_sess_inf[sess_id].timer_inf.type = M3_TIMER_TYPE_ASPM;
    pconn->aspm_sess_inf[sess_id].timer_inf.p_buf = (unsigned char*)ptimerbuf;//DEC2009 -- CC Error
    return 0;
}

m3_s32 m3uaStartRKMTimer(m3_conn_inf_t  *pconn,
                         m3_u32         sess_id,
                         void           *ptimerbuf,
                         m3_u32         interval)
{
    if (-1 == m3uaStartTimer(interval, ptimerbuf,
        &pconn->rkm_sess_inf[sess_id].timer_inf.timer_id)) {
        return -1;
    }
    pconn->rkm_sess_inf[sess_id].timer_inf.type = M3_TIMER_TYPE_RKM;
    pconn->rkm_sess_inf[sess_id].timer_inf.p_buf = (unsigned char*)ptimerbuf; //DEC2009 -- CC Error
    return 0;
}

m3_s32 m3uaStartHBEATTimer(m3_conn_inf_t  *pconn,
                           void           *ptimerbuf,
                           m3_u32         interval)
{
    if (-1 == m3uaStartTimer(interval, ptimerbuf,
        &pconn->hbeat_sess_inf.timer_inf.timer_id)) {
        return -1;
    }
    pconn->hbeat_sess_inf.timer_inf.type = M3_TIMER_TYPE_HBEAT;
    pconn->hbeat_sess_inf.timer_inf.p_buf = (unsigned char*)ptimerbuf; //DEC2009 -- CC Error
    return 0;
}

m3_s32 m3uaFreeTimer(m3_timer_inf_t *ptimerinf)
{
    void *ptimerbuf;

    m3uaStopTimer(ptimerinf->timer_id, &ptimerbuf);
    if (M3_NULL != ptimerbuf) {
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
    }
    return 0;
}

m3_s32 m3uaVoid(m3_conn_inf_t  *pconn,
                void           *p_edata)
{
    return 0;
}

void m3uaTimerExpiry(m3_u32        timerid,
                     void          *p_buf)
{
    m3_timer_param_t    *ptimerbuf = (m3_timer_param_t*)p_buf;
    m3_timer_inf_t      timer_inf;

    timer_inf.timer_id     = timerid;
    timer_inf.type         = ptimerbuf->type;
    timer_inf.p_buf        = (unsigned char*)p_buf; //DEC2009 -- CC Error
    switch(ptimerbuf->type) {
        case M3_TIMER_TYPE_ASPM: {
            m3uaASPM(M3_CONN_TABLE(ptimerbuf->param.aspmbuf.conn_id),
                M3_ASPM_EVT_TIMER_EXPR, &timer_inf);
            break;
        }
        case M3_TIMER_TYPE_PD: {
            m3uaRAS(M3_NULL, ptimerbuf->param.pdbuf.as_id,
                M3_AS_EVT_PD_TIMER_EXP, &timer_inf);
            break;
        }
        case M3_TIMER_TYPE_HBEAT: {
            m3ua_EHBEAT_TIMEREXP(M3_CONN_TABLE(ptimerbuf->param.aspmbuf.\
                conn_id), &timer_inf);
            break;
        }
        case M3_TIMER_TYPE_RKM: {
            m3uaProcRkmTimeout(M3_CONN_TABLE(ptimerbuf->param.rkmbuf.conn_id), &timer_inf);
            break;
        }
        default: {
            M3_MSG_FREE(p_buf, M3_TIMER_POOL);
            break;
        }
    }
    return;
}

m3_s32 m3uaSendMsg(m3_conn_inf_t  *pconn,
                   m3_u32         stream,
                   m3_u32         msglen,
                   m3_u8          *pmsg)
{
    M3TRACE(m3uaOutMsgTrace, ("Sending Message: connId:%u, AssocId:%u, streamId:%u", 
            pconn->conn_id, pconn->assoc_id, stream));
    M3HEX(m3uaOutMsgTrace, pmsg, msglen);
    if (-1 == m3ua_sendmsg(pconn->assoc_id, stream, msglen, pmsg)) {
        M3ERRNO(EM3_SENDMSG_FAIL);
        return -1;
    }
    return 0;
}

m3_s32 m3uaNtfyASPState(m3_conn_inf_t  *pconn,
                        m3_u32         as_id,
                        m3_asp_state_t state)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_ASP_STATE;
    plist->ntfy_list[plist->n_offset].param.asp.asp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.asp.rsp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.asp.as_id  = as_id;
    plist->ntfy_list[plist->n_offset].param.asp.state  = state;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }

	// Added by DANCIS -- JAN2010
    if (state == M3_ASP_DOWN)
    {
        if (M3_TRUE == M3_IS_SP_SGP(pconn->r_sp_id))
        {
            m3_r_sgp_inf_t *psgp = M3_R_SGP_TABLE(pconn->r_sp_id);
        	m3_asp_inf_t *pasp = M3_ASP_TABLE(pconn->l_sp_id);

            if (M3_FALSE == m3uaAssertASPActiveInSG(pasp, psgp->sg_id))
            {
                m3_pc_inf_t pcinf;
                pcinf.ptcode = 0;
                pcinf.nw_app = pasp->def_nwapp;

                m3uaSetRTStatex(pcinf, psgp->sg_id, M3_MAX_U8, 0, M3_RTSTATE_DOWN);
            }
        }
    }
	// Added by DANCIS -- JAN2010

    return 0;
}

m3_s32 m3uaNtfyRASPState(m3_conn_inf_t  *pconn,
                         m3_u32         as_id,
                         m3_asp_state_t state)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("as_id = %d, state = %d\n",as_id, state);

    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_R_ASP_STATE;
    plist->ntfy_list[plist->n_offset].param.r_asp.asp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.r_asp.lsp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.r_asp.as_id  = as_id;
    plist->ntfy_list[plist->n_offset].param.r_asp.state  = state;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyASState(m3_u32         as_id,
                       m3_conn_inf_t  *pconn,
                       m3_as_state_t  state)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_AS_STATE;
    plist->ntfy_list[plist->n_offset].param.as.as_id = as_id;
    plist->ntfy_list[plist->n_offset].param.as.lsp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.as.rsp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.as.state  = state;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyRASState(m3_r_as_inf_t  *pas,
                        m3_u32         lsp_id,
                        m3_as_state_t  state)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_R_AS_STATE;
    plist->ntfy_list[plist->n_offset].param.r_as.as_id = pas->as_id;
    plist->ntfy_list[plist->n_offset].param.r_as.lsp_id = lsp_id;
    plist->ntfy_list[plist->n_offset].param.r_as.state  = state;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyErr(m3_conn_inf_t    *pconn,
                   m3_error_inf_t   *pinf)
{
    m3_u16                idx;
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_ERR;
    plist->ntfy_list[plist->n_offset].param.err.lsp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.err.rsp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.err.err_code = pinf->err_code;
    plist->ntfy_list[plist->n_offset].param.err.num_rc = pinf->num_rc;
    for (idx = 0; idx < pinf->num_rc; idx++) {
        plist->ntfy_list[plist->n_offset].param.err.
            rc_list[idx] = pinf->rc_list[idx];
    }
    plist->ntfy_list[plist->n_offset].param.err.num_pc = pinf->num_pc;
    for (idx = 0; idx < pinf->num_pc; idx++) {
        plist->ntfy_list[plist->n_offset].param.err.
            pc_list[idx] = pinf->pc_list[idx];
    }
    plist->ntfy_list[plist->n_offset].param.err.nw_app = pinf->nw_app;
    plist->ntfy_list[plist->n_offset].param.err.diag_len = pinf->diag_len;
    plist->ntfy_list[plist->n_offset].param.err.p_diag_inf = pinf->p_diag_inf;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyConnState(m3_conn_inf_t   *pconn,
                         m3_conn_state_t state)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_CONN_STATE;
    plist->ntfy_list[plist->n_offset].param.conn.lsp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.conn.rsp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.conn.state  = state;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyNotify(m3_conn_inf_t  *pconn,
                      m3_ntfy_inf_t  *pinf)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;
    m3_u16                idx;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_MGMT_NTFY_NOTIFY;
    plist->ntfy_list[plist->n_offset].param.notify.lsp_id = pconn->l_sp_id;
    plist->ntfy_list[plist->n_offset].param.notify.rsp_id = pconn->r_sp_id;
    plist->ntfy_list[plist->n_offset].param.notify.status_type =
        pinf->status_type;
    plist->ntfy_list[plist->n_offset].param.notify.status_inf =
        pinf->status_inf;
    plist->ntfy_list[plist->n_offset].param.notify.m3asp_id = pinf->m3asp_id;
    plist->ntfy_list[plist->n_offset].param.notify.num_as = pinf->num_rc;
    for (idx = 0; idx < pinf->num_rc; idx++) {
        plist->ntfy_list[plist->n_offset].param.notify.as_list[idx] = 
                m3uaGetASFromRC(pconn, pinf->rc_list[idx]);
    }
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaAddNtfyToList(m3ua_mgmt_ntfy_t    *pntfy)
{
    m3_mgmt_ntfy_inf_t    *plist = M3_MGMT_NTFY_TABLE;

    if (M3_MAX_MGMT_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_MGMT_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset] = *pntfy;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_MGMT_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_match_t m3uaU32ListMatch(m3_u32    n_l1,
                            m3_u32    *l1,
                            m3_u32    n_l2,
                            m3_u32    *l2)
{
    m3_u32    l1_idx, l2_idx;
    m3_bool_t no_match = M3_FALSE, match = M3_FALSE;

    for (l1_idx = 0; l1_idx < n_l1; l1_idx++) {
        for (l2_idx = 0; l2_idx < n_l2; l2_idx++) {
            if (l1[l1_idx] != l2[l2_idx]) {
                continue;
            }
            match = M3_TRUE;
            break;
        }
        if (n_l2 == l2_idx) {
            no_match = M3_TRUE;
        }
    }
    if (M3_TRUE == match && M3_TRUE == no_match) {
        return M3_PARTIAL_MATCH;
    } else if (M3_TRUE == no_match) {
        return M3_UNIQUE;
    } else if (M3_TRUE == match && (n_l1 != n_l2)) {
         return M3_PARTIAL_MATCH;
    } else if (M3_TRUE == match && (n_l1 == n_l2)) {
        return M3_MATCH;
    }
    return M3_UNIQUE;
}

m3_match_t m3uaU8ListMatch(m3_u32    n_l1,
                           m3_u8     *l1,
                           m3_u32    n_l2,
                           m3_u8     *l2)
{
    m3_u32    l1_idx, l2_idx;
    m3_bool_t no_match = M3_FALSE, match = M3_FALSE;

    for (l1_idx = 0; l1_idx < n_l1; l1_idx++) {
        for (l2_idx = 0; l2_idx < n_l2; l2_idx++) {
            if (l1[l1_idx] != l2[l2_idx]) {
                continue;
            }
            match = M3_TRUE;
            break;
        }
        if (n_l2 == l2_idx) {
            no_match = M3_TRUE;
        }
    }
    if (M3_TRUE == match && M3_TRUE == no_match) {
        return M3_PARTIAL_MATCH;
    } else if (M3_TRUE == no_match) {
        return M3_UNIQUE;
    } else if (M3_TRUE == match && (n_l1 != n_l2)) {
         return M3_PARTIAL_MATCH;
    } else if (M3_TRUE == match && (n_l1 == n_l2)) {
        return M3_MATCH;
    }
    return M3_UNIQUE;
}

