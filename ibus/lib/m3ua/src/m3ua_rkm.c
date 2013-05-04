/*****************************************************************************
** Description:
** Code for RK Management procedures.
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
******************************************************************************/

#include <m3ua_rkm.h>

void m3uaProcRKM(m3_conn_inf_t *pconn,
                 m3_u32        stream_id,
                 m3_msg_hdr_t  *p_hdr,
                 m3_u8         *p_msg,
                 m3_u32        msglen)
{
    m3_reg_req_inf_t     regreq;
    m3_reg_rsp_inf_t     regrsp;
    m3_dreg_req_inf_t    deregreq;
    m3_dreg_rsp_inf_t   deregrsp;
    m3_error_inf_t       err;

    /* Global state of connection must not be down */
    if (pconn->r_sp_g_st == M3_ASP_DOWN) {
        return;
    }

    switch(p_hdr->msg_type)
    {
        case M3_MSG_TYPE_REG_REQ: {
            M3TRACE(m3uaInMsgTrace, ("Received REG REQ message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeREGREQ(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN,
                p_msg+M3_MSG_HDR_LEN, &regreq)) {
                m3ua_R_RKM(pconn, M3_R_RKM_EVT_RECV_REGREQ, (void*)&regreq);
            }
            break;
        }
        case M3_MSG_TYPE_DEREG_REQ: {
            M3TRACE(m3uaInMsgTrace, ("Received DEREG REQ message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDEREGREQ(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &deregreq)) {
                m3ua_R_RKM(pconn, M3_R_RKM_EVT_RECV_DEREGREQ, (void*)&deregreq);
            }
            break;
        }
        case M3_MSG_TYPE_REG_RSP: {
            M3TRACE(m3uaInMsgTrace, ("Received REG RSP message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeREGRSP(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &regrsp)) {
                m3ua_L_RKM(pconn, M3_L_RKM_EVT_RECV_REGRSP, (void *)&regrsp);
            }
            break;
        }
        case M3_MSG_TYPE_DEREG_RSP: {
            M3TRACE(m3uaInMsgTrace, ("Received DEREG RSP message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDEREGRSP(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &deregrsp)) {
                m3ua_L_RKM(pconn, M3_L_RKM_EVT_RECV_DEREGRSP, &deregrsp);
            }
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Received Unsupported RKM message type"));
            err.err_code = EM3_UNSUPP_MSG_TYPE;
            err.num_rc = 0;
            err.num_pc = 0;
            err.nw_app = M3_MAX_U32;
            err.diag_len = msglen;
            err.p_diag_inf = p_msg;
            m3ua_ESENDERROR(pconn, &err);
            break;
        }
    }
    return;
}

m3_s32 m3ua_L_RKM(m3_conn_inf_t   *pconn,
                  m3_l_rkm_evt_t  event,
                  void            *pInf)
{
    m3_s32    ret;

    switch(event) {
        case M3_L_RKM_EVT_REGISTER: {
            ret = m3uaProcRegister(pconn, pInf);
            break;
        }
        case M3_L_RKM_EVT_RECV_REGRSP: {
            ret = m3uaProcRegRsp(pconn, pInf);
            break;
        }
        case M3_L_RKM_EVT_DEREGISTER: {
            ret = m3uaProcDeregister(pconn, pInf);
            break;
        }
        case M3_L_RKM_EVT_RECV_DEREGRSP: {
            ret = m3uaProcDeregRsp(pconn, pInf);
            break;
        }
        case M3_L_RKM_EVT_TIMEOUT: {
            ret = m3uaProcRkmTimeout(pconn, pInf);
            break;
        }
    }
    return ret;
}

m3_s32 m3ua_R_RKM(m3_conn_inf_t   *pconn,
                  m3_r_rkm_evt_t  event,
                  void            *pInf)
{
    m3_s32    ret;

    switch(event) {
        case M3_R_RKM_EVT_RECV_REGREQ: {
            ret = m3uaProcRegReq(pconn, pInf);
            break;
        }
        case M3_R_RKM_EVT_STATUS: {
            ret = m3uaProcRkStatus(pconn, pInf);
            break;
        }
        case M3_R_RKM_EVT_RECV_DEREGREQ: {
            ret = m3uaProcDeregReq(pconn, pInf);
            break;
        }
    }
    return ret;
}

m3_bool_t m3uaAssertLRKState(m3_conn_inf_t *pconn,
                             m3_u8        state,
                             m3_u16       num_as,
                             m3_u32       *as_list)
{
    m3_u16        idx;
    m3_asp_inf_t  *pasp = M3_ASP_TABLE(pconn->l_sp_id);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        for (idx = 0; idx < num_as; idx++) {
            if (state != pasp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_list[idx]].dyn_reg_state) {
                return M3_FALSE;
            }
        }
    } else {
        for (idx = 0; idx < num_as; idx++) {
            if (state != pasp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_list[idx]].dyn_reg_state) {
                return M3_FALSE;
            }
        }
    }
    return M3_TRUE;
}

m3_bool_t m3uaAssertRRKState(m3_conn_inf_t *pconn,
                             m3_u8        state,
                             m3_u16       num_as,
                             m3_u32       *as_list)
{
    m3_u16        idx;
    m3_asp_inf_t  *pasp;
    m3_sgp_inf_t  *psgp;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < num_as; idx++) {
            if (state != pasp->r_as_inf[(m3_u16)as_list[idx]].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state) {
                return M3_FALSE;
            }
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < num_as; idx++) {
            if (state != psgp->r_as_inf[(m3_u16)as_list[idx]].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state) {
                return M3_FALSE;
            }
        }
    }
    return M3_TRUE;
}

void m3uaModifyLRKState(m3_conn_inf_t *pconn,
                        m3_u8         state,
                        m3_u16        num_as,
                        m3_u32        *as_list)
{
    m3_u16        idx;
    m3_asp_inf_t  *pasp = M3_ASP_TABLE(pconn->l_sp_id);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        for (idx = 0; idx < num_as; idx++) {
            pasp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_list[idx]].dyn_reg_state = state;
        }
    } else {
        for (idx = 0; idx < num_as; idx++) {
            pasp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as_list[idx]].dyn_reg_state = state;
        }
    }
    return;
}

void m3uaUpdateLRC(m3_conn_inf_t *pconn,
                   m3_u32        as,
                   m3_u32        rtctx)
{
    m3_asp_inf_t  *pasp = M3_ASP_TABLE(pconn->l_sp_id);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        pasp->r_asp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as].rtctx = rtctx;
    } else {
        pasp->r_sgp_inf[(m3_u16)pconn->r_sp_id].rc_inf[as].rtctx = rtctx;
    }
    return;
}

void m3uaModifyRRKState(m3_conn_inf_t *pconn,
                        m3_u8         state,
                        m3_u16        num_as,
                        m3_u32        *as_list)
{
    m3_asp_inf_t    *pasp;
    m3_sgp_inf_t    *psgp;
    m3_u16          as_idx;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        for (as_idx = 0; as_idx < num_as; as_idx++) {
            pasp->r_as_inf[(m3_u16)as_list[as_idx]].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state = state;
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        for (as_idx = 0; as_idx < num_as; as_idx++) {
            psgp->r_as_inf[(m3_u16)as_list[as_idx]].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state = state;
        }
    }
    return;
}

m3_s32 m3uaProcRegister(m3_conn_inf_t *pconn,
                        void          *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3ua_rkey_t         *pInf = (m3ua_rkey_t*)p_edata;
    m3_reg_req_inf_t    regreq;
    m3_u16              idx;
    m3_u8               *p_msg = M3_NULL;
    m3_u32              msglen = 0;
    m3_timer_param_t    *ptimerbuf;
    m3_u8               sess_id;

    if (M3_FALSE == m3uaAssertLRKState(pconn, M3_RK_STATIC, pInf->reg.num_as,
        pInf->reg.as_list)) {
        M3ERRNO(EM3_RKSTATE_MISMATCH);
        return -1;
    }
    /* prepare m3_reg_req_inf_t structure */
    regreq.num_rk = pInf->reg.num_as;
    for (idx = 0; idx < regreq.num_rk; idx++) {
        /* check the state of ASP in AS, should be INACTIVE */
        if (M3_ASP_INACTIVE != pconn->l_sp_st[pInf->reg.as_list[idx]]) {
            M3ERRNO(EM3_INV_ASPSTATE);
            return -1;
        }
        regreq.rk_list[idx].lrk_id = pInf->reg.as_list[idx];
        regreq.rk_list[idx].rk_inf = (M3_AS_TABLE(pInf->reg.as_list[idx]))->rkey;
    }
    /* build the RK REG REQ message */
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_REGREQ_SIZE, M3_RKM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), M3_TIMER_POOL);
    if (M3_NULL == ptimerbuf) {
        M3_MSG_FREE(p_msg, M3_RKM_POOL);
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    m3uaEncodeREGREQ(&regreq, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    /* start the timer */
    m3uaGetFreeRKMSess(pconn, &sess_id);
    pconn->rkm_sess_inf[sess_id].e_state              = 1;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg_type     = M3_MSG_TYPE_REG_REQ;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg_len      = msglen;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg          = p_msg;

    ptimerbuf->type = M3_TIMER_TYPE_RKM;
    ptimerbuf->param.rkmbuf.conn_id = pconn->conn_id;
    ptimerbuf->param.rkmbuf.sess_id = sess_id;
    ptimerbuf->param.rkmbuf.num_as = pInf->reg.num_as;
    for (idx = 0; idx < pInf->reg.num_as; idx++) {
        ptimerbuf->param.rkmbuf.as_list[idx] = pInf->reg.as_list[idx];
    }
    m3uaStartRKMTimer(pconn, sess_id, ptimerbuf, M3_RKM_TIMER_INT);

    /* change the state of LRK */
    m3uaModifyLRKState(pconn, M3_RK_REG_IN_PROG, pInf->reg.num_as, pInf->reg.as_list);
    return 0;
}

m3_bool_t m3uaAssertREGRSP(m3_conn_inf_t     *pconn,
                           m3_reg_rsp_inf_t  *pInf)
{
    m3_asp_inf_t    *pasp = M3_ASP_TABLE(pconn->l_sp_id);
    m3_u16          lrk_idx, list_idx;
    m3_u32          as_list[M3_MAX_AS];

    for (lrk_idx = 0; lrk_idx < pInf->num_reg_result; lrk_idx++) {
        as_list[lrk_idx] = pInf->rr_list[lrk_idx].lrk_id;
        /* check for validity of lrk id list and state of lrk */
        if (M3_FALSE == M3_ASID_VALID(pInf->rr_list[lrk_idx].lrk_id)      ||
            M3_FALSE == (M3_AS_TABLE(pInf->rr_list[lrk_idx].lrk_id))->e_state  ||
            M3_FALSE == pasp->as_list[pInf->rr_list[lrk_idx].lrk_id]) {
            M3ERRNO(EM3_INV_LRKID);
            return M3_FALSE;
        }
        /* check for validity of registration status list and rtctx value */
        if ((0 < pInf->rr_list[lrk_idx].reg_status && 10 >= pInf->rr_list[lrk_idx].reg_status) &&
            0 != pInf->rr_list[lrk_idx].rtctx) {
            M3ERRNO(EM3_INV_RTCTX);
            return M3_FALSE;
        } else if (0 == pInf->rr_list[lrk_idx].reg_status && 0 == pInf->rr_list[lrk_idx].rtctx) {
            M3ERRNO(EM3_INV_RTCTX);
            return M3_FALSE;
        } else if (10 < pInf->rr_list[lrk_idx].reg_status) {
            M3ERRNO(EM3_INV_REG_STATUS);
            return M3_FALSE;
        }
        /* check for uniqueness of routing context */
        /*    (1) check the list for repeated routing context
         *    (2) check already configured routing context
         */
        for (list_idx = lrk_idx+1; list_idx < pInf->num_reg_result; list_idx++) {
            if (pInf->rr_list[lrk_idx].rtctx == pInf->rr_list[list_idx].rtctx) {
                M3ERRNO(EM3_RTCTX_NONUNIQUE);
                return M3_FALSE;
            }
        }
        if (M3_FALSE == m3uaLRCUnique(pconn, pInf->rr_list[lrk_idx].rtctx)) {
            M3ERRNO(EM3_RTCTX_NONUNIQUE);
            return M3_FALSE;
        }
    }
    if (M3_FALSE == m3uaAssertLRKState(pconn, M3_RK_REG_IN_PROG, pInf->num_reg_result, as_list)) {
        M3ERRNO(EM3_INV_LRKID);
        return M3_FALSE;
    }
    return M3_TRUE;
}

/*
 * Removes common elements from the input lists and adds it to
 * the output list
 */
void m3uaCreateCommonLRKList(m3_u16      *p_nrk1,
                            m3_u32       *p_rklist1,
                            m3_u16       *p_nrk2,
                            m3_u32       *p_rklist2,
                            m3_u16       *p_nrkm,
                            m3_u32       *p_rklistm)
{
    m3_u32        lrk_id;
    m3_u16        idx, idx2;
    m3_u16        rklist_idx = 0;
    m3_bool_t     lrk_match;

    for (idx = 0; idx < *p_nrk1;){
        lrk_id = p_rklist1[idx];
        lrk_match = M3_FALSE;
        for (idx2 = 0; idx2 < *p_nrk2; idx2++) {
            if (lrk_id == p_rklist2[idx2]) {
                p_rklistm[*p_nrkm] = lrk_id;
                *p_nrkm = *p_nrkm + 1;
                lrk_match = M3_TRUE;
                for (rklist_idx = idx2; rklist_idx < *p_nrk2-1; rklist_idx++) {
                    p_rklist2[rklist_idx] = p_rklist2[rklist_idx+1];
                }
                for (rklist_idx = idx; rklist_idx < *p_nrk1-1; rklist_idx++) {
                    p_rklist1[rklist_idx] = p_rklist2[rklist_idx+1];
                }
                *p_nrk1 = *p_nrk1 - 1;
                *p_nrk2 = *p_nrk2 - 1;
                break;
            }
        }
        if (M3_FALSE == lrk_match)
            idx++;
    }
    return;
}

m3_s32 m3uaProcRegRsp(m3_conn_inf_t *pconn,
                      void          *p_edata)
{
    m3_reg_rsp_inf_t    *pInf = (m3_reg_rsp_inf_t*)p_edata;
    m3_u16              idx, lrk_idx;
    m3_rkmtimer_param_t *prkmtimer;
    m3_u16              num_lrk;
    m3_u32              lrk_list[M3_MAX_AS];
    m3_u16              num_reg_result;
    m3_u32              lrk_recv_list[M3_MAX_AS];
    m3_reg_req_inf_t    regreq;
    m3ua_mgmt_ntfy_t    ntfy;
    m3_timer_param_t    *ptimerbuf;

    if (M3_FALSE == m3uaAssertREGRSP(pconn, pInf)) {
        return -1;
    }
    ntfy.type = M3_MGMT_NTFY_REG_STATUS;
    ntfy.param.reg_status.asp_id = pconn->l_sp_id;
    ntfy.param.reg_status.rsp_id = pconn->r_sp_id;
    ntfy.param.reg_status.num_as = 0;
    num_reg_result = pInf->num_reg_result;
    for (idx = 0; idx < pInf->num_reg_result; idx++) {
        lrk_recv_list[idx] = pInf->rr_list[idx].lrk_id;
        if (0 != pInf->rr_list[idx].reg_status) {
            m3uaModifyLRKState(pconn, M3_RK_STATIC, 1, &pInf->rr_list[idx].lrk_id);
        } else {
            m3uaModifyLRKState(pconn, M3_RK_REGD, 1, &pInf->rr_list[idx].lrk_id);
            m3uaUpdateLRC(pconn, pInf->rr_list[idx].lrk_id, pInf->rr_list[idx].rtctx);
        }
        ntfy.param.reg_status.result[ntfy.param.reg_status.num_as].as_id = pInf->rr_list[idx].lrk_id;
        ntfy.param.reg_status.result[ntfy.param.reg_status.num_as].rtctx = pInf->rr_list[idx].rtctx;
        ntfy.param.reg_status.result[ntfy.param.reg_status.num_as].status = pInf->rr_list[idx].reg_status;
        ntfy.param.reg_status.num_as++;
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_FALSE == pconn->rkm_sess_inf[idx].e_state         ||
            M3_MSG_TYPE_REG_REQ != pconn->rkm_sess_inf[idx].msg_inf.msg_type) {
            continue;
        }
        ptimerbuf = (m3_timer_param_t*)pconn->rkm_sess_inf[idx].timer_inf.p_buf;
        prkmtimer = &ptimerbuf->param.rkmbuf;
        num_lrk = 0;
        m3uaCreateCommonLRKList(&num_reg_result, lrk_recv_list, &prkmtimer->num_as, prkmtimer->as_list,
            &num_lrk, lrk_list);
        if (0 == prkmtimer->num_as) {
            m3uaFreeTimer(&pconn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pconn, idx);
            if (0 == num_reg_result) {
                break;
            }
            continue;
        } else if (0 == num_lrk) {
            continue;
        }
        /* re-build RK registration message */
        regreq.num_rk = prkmtimer->num_as;
        for (lrk_idx = 0; lrk_idx < regreq.num_rk; lrk_idx++) {
            regreq.rk_list[lrk_idx].lrk_id = prkmtimer->as_list[lrk_idx];
            regreq.rk_list[lrk_idx].rk_inf = (M3_AS_TABLE(prkmtimer->as_list[lrk_idx]))->rkey;
        }
        m3uaEncodeREGREQ(&regreq, &pconn->rkm_sess_inf[idx].msg_inf.msg_len,
                pconn->rkm_sess_inf[idx].msg_inf.msg);
        if (0 == num_reg_result) {
            break;
        }
    }
    /* give indication to the LME about (un)/successfully registered routing keys */
    m3uaAddNtfyToList(&ntfy);
    return 0;
}

m3_s32 m3uaProcDeregister(m3_conn_inf_t *pconn,
                          void          *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3ua_rkey_t         *pInf = (m3ua_rkey_t*)p_edata;
    m3_dreg_req_inf_t   deregreq;
    m3_u16              idx;
    m3_u8               *p_msg = M3_NULL;
    m3_u32              msglen = 0;
    m3_timer_param_t    *ptimerbuf;
    m3_u8               sess_id;

    /* check that all the RK in the m3ua_rkey_t are in regd state */
    if (M3_FALSE == m3uaAssertLRKState(pconn, M3_RK_REGD, pInf->dereg.num_as,
        pInf->dereg.as_list)) {
        M3ERRNO(EM3_RKSTATE_MISMATCH);
        return -1;
    }
    /* prepare m3_dreg_req_inf_t structure */
    deregreq.num_rc = pInf->dereg.num_as;
    for (idx = 0; idx < deregreq.num_rc; idx++) {
        if (M3_ASP_INACTIVE != pconn->l_sp_st[pInf->dereg.as_list[idx]]) {
            M3ERRNO(EM3_INV_ASPSTATE);
            return -1;
        }
        deregreq.rc_list[idx] = m3uaGetRCFromAS(pconn, pInf->dereg.as_list[idx]);
    }
    /* build the RK DEREG REQ message */
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DEREGREQ_SIZE, M3_RKM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), M3_TIMER_POOL);
    if (M3_NULL == ptimerbuf) {
        M3_MSG_FREE(p_msg, M3_RKM_POOL);
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    m3uaEncodeDREGREQ(&deregreq, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    /* start the timer */
    m3uaGetFreeRKMSess(pconn, &sess_id);
    pconn->rkm_sess_inf[sess_id].e_state              = 1;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg_type     = M3_MSG_TYPE_DEREG_REQ;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg_len      = msglen;
    pconn->rkm_sess_inf[sess_id].msg_inf.msg          = p_msg;

    ptimerbuf->type = M3_TIMER_TYPE_RKM;
    ptimerbuf->param.rkmbuf.conn_id = pconn->conn_id;
    ptimerbuf->param.rkmbuf.sess_id = sess_id;
    ptimerbuf->param.rkmbuf.num_as = pInf->dereg.num_as;
    for (idx = 0; idx < pInf->dereg.num_as; idx++) {
        ptimerbuf->param.rkmbuf.as_list[idx] = pInf->dereg.as_list[idx];
    }
    m3uaStartRKMTimer(pconn, sess_id, ptimerbuf, M3_RKM_TIMER_INT);

    /* change the state of LRK */
    m3uaModifyLRKState(pconn, M3_RK_DEREG_IN_PROG, pInf->reg.num_as, pInf->reg.as_list);
    return 0;
}

m3_bool_t m3uaAssertDEREGRSP(m3_conn_inf_t      *pconn,
                             m3_dreg_rsp_inf_t  *pInf,
                             m3_u32             *as_list)
{
    m3_u16            rc_idx;

    for (rc_idx = 0; rc_idx < pInf->num_dreg_result; rc_idx++) {
        /* check for the deregistration status value */
        if (5 < pInf->drr_list[rc_idx].dreg_status) {
            M3ERRNO(EM3_INV_DEREG_STATUS);
            return M3_FALSE;
        }
        as_list[rc_idx] = m3uaGetASFromRC(pconn, pInf->drr_list[rc_idx].rtctx);
    }
    if (M3_FALSE == m3uaAssertLRKState(pconn, M3_RK_DEREG_IN_PROG, pInf->num_dreg_result, as_list)) {
        M3ERRNO(EM3_INV_LRKID);
        return M3_FALSE;
    }
    return M3_TRUE;
}

m3_s32 m3uaProcDeregRsp(m3_conn_inf_t *pconn,
                        void          *p_edata)
{
    m3_dreg_rsp_inf_t   *pInf = (m3_dreg_rsp_inf_t*)p_edata;
    m3_u16              idx, rc_idx;
    m3_rkmtimer_param_t *prkmtimer;
    m3_u16              num_cmn_as;
    m3_u32              cmn_as_list[M3_MAX_AS];
    m3_u16              num_as;
    m3_u32              as_list[M3_MAX_AS];
    m3_dreg_req_inf_t   deregreq;
    m3ua_mgmt_ntfy_t    ntfy;
    m3_timer_param_t    *ptimerbuf;

    if (M3_FALSE == m3uaAssertDEREGRSP(pconn, pInf, as_list)) {
        return -1;
    }
    ntfy.type = M3_MGMT_NTFY_DEREG_STATUS;
    ntfy.param.dreg_status.asp_id = pconn->l_sp_id;
    ntfy.param.dreg_status.rsp_id = pconn->r_sp_id;
    ntfy.param.dreg_status.num_as = 0;
    num_as = pInf->num_dreg_result;
    for (idx = 0; idx < pInf->num_dreg_result; idx++) {
        if (M3_MAX_U32 != as_list[idx]) {
            if (0 != pInf->drr_list[idx].dreg_status) {
                m3uaModifyLRKState(pconn, M3_RK_REGD, 1, &as_list[idx]);
            } else {
                m3uaModifyLRKState(pconn, M3_RK_STATIC, 1, &as_list[idx]);
            }
            ntfy.param.dreg_status.result[ntfy.param.dreg_status.num_as].as_id = as_list[idx];
            ntfy.param.dreg_status.result[ntfy.param.dreg_status.num_as].rtctx = pInf->drr_list[idx].rtctx;
            ntfy.param.dreg_status.result[ntfy.param.dreg_status.num_as].status =
                    pInf->drr_list[idx].dreg_status;
            ntfy.param.dreg_status.num_as++;
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_FALSE == pconn->rkm_sess_inf[idx].e_state         ||
            M3_MSG_TYPE_DEREG_REQ != pconn->rkm_sess_inf[idx].msg_inf.msg_type) {
            continue;
        }
        ptimerbuf = (m3_timer_param_t*)pconn->rkm_sess_inf[idx].timer_inf.p_buf;
        prkmtimer = &ptimerbuf->param.rkmbuf;
        num_cmn_as = 0;
        m3uaCreateCommonLRKList(&num_as, as_list, &prkmtimer->num_as, prkmtimer->as_list,
            &num_cmn_as, cmn_as_list);
        if (0 == prkmtimer->num_as) {
            m3uaFreeTimer(&pconn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pconn, idx);
            if (0 == num_as) {
                break;
            }
            continue;
        } else if (0 == num_cmn_as) {
            continue;
        }
        /* re-build RK deregistration message */
        deregreq.num_rc = prkmtimer->num_as;
        for (rc_idx = 0; rc_idx < deregreq.num_rc; rc_idx++) {
            deregreq.rc_list[rc_idx] = m3uaGetRCFromAS(pconn, prkmtimer->as_list[rc_idx]);
        }
        m3uaEncodeDREGREQ(&deregreq, &pconn->rkm_sess_inf[idx].msg_inf.msg_len,
            pconn->rkm_sess_inf[idx].msg_inf.msg);
        if (0 == num_as) {
            break;
        }
    }
    /* indicate to LME about (un)/successfully dereged routing keys */
    m3uaAddNtfyToList(&ntfy);
    return 0;
}

m3_s32 m3uaProcRkmTimeout(m3_conn_inf_t *pconn,
                          void          *p_edata)
{
    m3_timer_inf_t        *pInf = (m3_timer_inf_t*)p_edata;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)pInf->p_buf;
    m3_u8                 sess_id = ptimerbuf->param.rkmbuf.sess_id;
    m3_u16                as_idx;
    m3ua_mgmt_ntfy_t      ntfy;
    m3_msg_type_t         msg_type = pconn->rkm_sess_inf[sess_id].msg_inf.msg_type;

    if (M3_RKM_MAX_RETRY <= pconn->rkm_sess_inf[sess_id].num_retry ||
        -1 == m3uaStartRKMTimer(pconn, sess_id, ptimerbuf, M3_RKM_TIMER_INT)) {
        m3uaDeleteRKMSess(pconn, sess_id);
        m3uaModifyLRKState(pconn, M3_RK_STATIC, ptimerbuf->param.rkmbuf.num_as,
            ptimerbuf->param.rkmbuf.as_list);
        /* indication to LME about failure of the message */
        if (M3_MSG_TYPE_REG_REQ == msg_type) {
            ntfy.type = M3_MGMT_NTFY_REG_STATUS;
            ntfy.param.reg_status.asp_id = pconn->l_sp_id;
            ntfy.param.reg_status.rsp_id = pconn->r_sp_id;
            ntfy.param.reg_status.num_as = ptimerbuf->param.rkmbuf.num_as;
            for (as_idx = 0; as_idx < ptimerbuf->param.rkmbuf.num_as; as_idx++) {
                ntfy.param.reg_status.result[as_idx].as_id =
                        ptimerbuf->param.rkmbuf.as_list[as_idx];
                ntfy.param.reg_status.result[as_idx].rtctx = 0;
                ntfy.param.reg_status.result[as_idx].status = M3_REG_STATUS_TIMEOUT;
            }
        } else {
            ntfy.type = M3_MGMT_NTFY_DEREG_STATUS;
            ntfy.param.dreg_status.asp_id = pconn->l_sp_id;
            ntfy.param.dreg_status.rsp_id = pconn->r_sp_id;
            ntfy.param.dreg_status.num_as = ptimerbuf->param.rkmbuf.num_as;
            for (as_idx = 0; as_idx < ptimerbuf->param.rkmbuf.num_as; as_idx++) {
                ntfy.param.dreg_status.result[as_idx].as_id =
                        ptimerbuf->param.rkmbuf.as_list[as_idx];
                ntfy.param.dreg_status.result[as_idx].rtctx =
                        m3uaGetRCFromAS(pconn, ptimerbuf->param.rkmbuf.as_list[as_idx]);
                ntfy.param.dreg_status.result[as_idx].status = M3_DEREG_STATUS_TIMEOUT;
            }
        }
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
        m3uaAddNtfyToList(&ntfy);
    } else {
        m3uaSendMsg(pconn, 0, pconn->rkm_sess_inf[sess_id].msg_inf.msg_len,
            pconn->rkm_sess_inf[sess_id].msg_inf.msg);
        pconn->rkm_sess_inf[sess_id].num_retry++;
        pconn->rkm_sess_inf[sess_id].timer_inf = *pInf;
    }
    return 0;
}

m3_bool_t m3uaCheckDeleteDynamicRAS(m3_u32        as)
{
    m3_u16        conn_idx;
    m3_conn_inf_t *pconn;
    m3_asp_inf_t  *pasp;
    m3_sgp_inf_t  *psgp;

    /* check if any connection does not have this AS in down state */
    for (conn_idx = 0; conn_idx < M3_MAX_CONN; conn_idx++) {
        pconn = M3_CONN_TABLE(conn_idx);
        if (M3_FALSE == pconn->e_state || M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
            continue;
        }
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            if (M3_FALSE == pasp->r_as_inf[(m3_u16)as].asp_list[(m3_u16)pconn->r_sp_id]) {
                continue;
            }
            if (M3_AS_ACTIVE == pasp->r_as_inf[(m3_u16)as].state) {
                return M3_FALSE;
            }
        } else {
            psgp = M3_SGP_TABLE(pconn->l_sp_id);
            if (M3_FALSE == psgp->r_as_inf[(m3_u16)as].asp_list[(m3_u16)pconn->r_sp_id]) {
                continue;
            }
            if (M3_AS_ACTIVE == psgp->r_as_inf[(m3_u16)as].state) {
                return M3_FALSE;
            }
        }
    }
    return M3_TRUE;
}

m3_match_t m3uaRKAliasParamMatch(m3_rk_elements_t *prt1,
                                 m3_rk_elements_t *prt2)
{
    m3_match_t    si_match, opc_match, ckt_match;

    /* check for full, partial or no match of SS7 routing parameters */
    /* check for dpc match */
    if (prt1->dpc != prt2->dpc) {
        return M3_UNIQUE;
    }
    /* check for SI list match */
    if (0 == prt1->num_si && 0 == prt2->num_si) {
        si_match = M3_MATCH;
    } else if (0 == prt1->num_si || 0 == prt2->num_si) {
        si_match = M3_PARTIAL_MATCH;
    } else {
        si_match = m3uaU8ListMatch(prt1->num_si, prt1->si_list, prt2->num_si, prt2->si_list);
    }
    /* check for OPC list match */
    if (0 == prt1->num_opc && 0 == prt2->num_opc) {
        opc_match = M3_MATCH;
    } else if (0 == prt1->num_opc || 0 == prt2->num_opc) {
        opc_match = M3_PARTIAL_MATCH;
    } else {
        opc_match = m3uaU32ListMatch(prt1->num_opc, prt1->opc_list, prt2->num_opc,
                prt2->opc_list);
    }
    /* check for CKT Range list match */
    if (0 == prt1->num_ckt_range && 0 == prt2->num_ckt_range) {
        ckt_match = M3_MATCH;
    } else if (0 == prt1->num_ckt_range || 0 == prt2->num_ckt_range) {
        ckt_match = M3_PARTIAL_MATCH;
    } else {
        ckt_match = m3uaCktRangeListMatch(prt1->num_ckt_range, prt1->ckt_range,
                prt2->num_ckt_range, prt2->ckt_range);
    }
    if (M3_UNIQUE == si_match || M3_UNIQUE == opc_match || M3_UNIQUE == ckt_match) {
        return M3_UNIQUE;
    } else if (M3_MATCH == si_match && M3_MATCH == opc_match && M3_MATCH == ckt_match) {
        return M3_MATCH;
    }
    return M3_PARTIAL_MATCH;
}

m3_match_t m3uaRKMatch(m3_rk_inf_t *pkeyin,
                       m3_rk_inf_t *pkeycmp)
{
    m3_u16        kin_idx, kcmp_idx;
    m3_match_t    rtp_match;
    m3_bool_t     rtp_unique = M3_FALSE, rtp_full_match = M3_FALSE;

    /* both routing keys should have same network appearance */
    if (pkeyin->nw_app != pkeycmp->nw_app) {
        return M3_UNIQUE;
    }
    for (kin_idx = 0; kin_idx < pkeyin->num_rtparam; kin_idx++) {
        for (kcmp_idx = 0; kcmp_idx < pkeycmp->num_rtparam; kcmp_idx++) {
            rtp_match = m3uaRKAliasParamMatch(&pkeyin->rtparam[kin_idx],
                    &pkeycmp->rtparam[kcmp_idx]);
            if (M3_PARTIAL_MATCH == rtp_match) {
                return M3_PARTIAL_MATCH;
            } else if (M3_MATCH == rtp_match && (pkeyin->num_rtparam != pkeycmp->num_rtparam)) {
                return M3_PARTIAL_MATCH;
            } else if (M3_MATCH == rtp_match) {
                break;
            }
        }
        if (kcmp_idx == pkeycmp->num_rtparam) {
            rtp_unique = M3_TRUE;
        } else {
            rtp_full_match = M3_TRUE;
        }
        if (M3_TRUE == rtp_unique && M3_TRUE == rtp_full_match) {
            return M3_PARTIAL_MATCH;
        }
    }
    if (M3_TRUE == rtp_unique)
        return M3_UNIQUE;
    return M3_MATCH;
}

m3_match_t m3uaCktRangeListMatch(m3_u32          n_l1,
                                 m3_ckt_range_t  *l1,
                                 m3_u32          n_l2,
                                 m3_ckt_range_t  *l2)
{
    m3_u32    l1_idx, l2_idx;
    m3_bool_t no_match = M3_FALSE, match = M3_FALSE;

    for (l1_idx = 0; l1_idx < n_l1; l1_idx++) {
        for (l2_idx = 0; l2_idx < n_l2; l2_idx++) {
            if (l1[l1_idx].opc != l2[l2_idx].opc) {
                continue;
            }
            /* check for full match of cic range */
            if (l1[l1_idx].lcic == l2[l2_idx].lcic && l1[l1_idx].ucic == l2[l2_idx].ucic) {
                match = M3_TRUE;
                break;
            }
            if ((l1[l1_idx].lcic <= l2[l2_idx].lcic && l1[l1_idx].ucic >= l2[l2_idx].lcic)    ||
                (l1[l1_idx].lcic <= l2[l2_idx].ucic && l1[l1_idx].ucic >= l2[l2_idx].ucic)) {
                return M3_PARTIAL_MATCH;
            }
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

void m3uaSaveLRKID(m3_conn_inf_t    *pconn,
                   m3_r_as_inf_t    *pras,
                   m3_u32           lrk_id)
{
    m3_asp_inf_t  *pasp;
    m3_sgp_inf_t  *psgp;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        pasp->r_as_inf[pras->as_id].rc_inf[(m3_u16)pconn->r_sp_id].rk_id = lrk_id;
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        psgp->r_as_inf[pras->as_id].rc_inf[(m3_u16)pconn->r_sp_id].rk_id = lrk_id;
    }
    return;
}

m3_u32 m3uaGetLRKID(m3_conn_inf_t    *pconn,
                    m3_r_as_inf_t    *pras)
{
    m3_asp_inf_t  *pasp;
    m3_sgp_inf_t  *psgp;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        return pasp->r_as_inf[pras->as_id].rc_inf[(m3_u16)pconn->r_sp_id].rk_id;
    }
    psgp = M3_SGP_TABLE(pconn->l_sp_id);
    return psgp->r_as_inf[pras->as_id].rc_inf[(m3_u16)pconn->r_sp_id].rk_id;
}

#define M3_AS_NOT_REGD_IN_ASP    1
#define M3_ASP_NOT_REGD_IN_AS    2
#define M3_AS_REG_IN_PROG        3
#define M3_AS_DEREG_IN_PROG      4
#define M3_ASP_REGD_IN_AS        5

void      m3uaRegisterRASPInRAS(m3_conn_inf_t    *pConn,
                                m3_u32           asId)
{
    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;

    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        pAsp = M3_ASP_TABLE(pConn->l_sp_id);
        pAsp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] = M3_TRUE;
        pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry = M3_TRUE;
        pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state = M3_RK_REG_IN_PROG;
    } else {
        pSgp = M3_SGP_TABLE(pConn->l_sp_id);
        pSgp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] = M3_TRUE;
        pSgp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry = M3_TRUE;
        pSgp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state = M3_RK_REG_IN_PROG;
    }
    //pConn->r_sp_st[asId] = M3_ASP_INACTIVE; // ASP is Inactive in this AS
}

void      m3uaDeregisterRASPFromRAS(m3_conn_inf_t  *pConn,
                                    m3_u32         asId)
{
    m3_asp_inf_t    *pAsp;
    m3_sgp_inf_t    *pSgp;

    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        pAsp = M3_ASP_TABLE(pConn->l_sp_id);
        if (M3_TRUE == pAsp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] /*&& 
            M3_TRUE == pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry*/) {
            pAsp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] = M3_FALSE;
            pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry = M3_FALSE;
            pAsp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state = M3_RK_STATIC;
        }
    } else {
        pSgp = M3_SGP_TABLE(pConn->l_sp_id);
        if (M3_TRUE == pSgp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] /*&& 
            M3_TRUE == pSgp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry*/) {
            pSgp->r_as_inf[asId].asp_list[(m3_u16)pConn->r_sp_id] = M3_FALSE;
            pSgp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_entry = M3_FALSE;
            pSgp->r_as_inf[asId].rc_inf[(m3_u16)pConn->r_sp_id].dyn_reg_state = M3_RK_STATIC;
        }
    }
    //pConn->r_sp_st[asId] = M3_ASP_DOWN; // ASP is DOWN in this AS
}

m3_u8     m3uaRASAlreadyRegistered(m3_conn_inf_t *pconn,
                                   m3_u32        as)
{
    m3_asp_inf_t    *pasp;
    m3_sgp_inf_t    *psgp;

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        if (M3_FALSE == pasp->r_as_inf[(m3_u16)as].asp_list[(m3_u16)pconn->r_sp_id]) {
            return M3_ASP_NOT_REGD_IN_AS;
        } else if (M3_RK_REGD == pasp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state ||
                   M3_RK_STATIC == pasp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state) {
            return M3_ASP_REGD_IN_AS;
        } else if (M3_RK_REG_IN_PROG == pasp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state) {
            return M3_AS_REG_IN_PROG;
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        if (M3_FALSE == psgp->r_as_inf[(m3_u16)as].asp_list[(m3_u16)pconn->r_sp_id]) {
            return M3_ASP_NOT_REGD_IN_AS;
        } else if (M3_RK_REGD == psgp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state ||
                   M3_RK_STATIC == psgp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state) {
            return M3_ASP_REGD_IN_AS;
        } else if (M3_RK_REG_IN_PROG == psgp->r_as_inf[(m3_u16)as].rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state){
            return M3_AS_REG_IN_PROG;
        }
    }
    return M3_AS_DEREG_IN_PROG;
}

m3_s32 m3uaProcRegReq(m3_conn_inf_t *pconn,
                      void          *p_edata)
{
    m3_reg_req_inf_t  *pInf = (m3_reg_req_inf_t*)p_edata;
    m3_match_t        rk_match;
    m3_u16            rk_idx; //i_rk_idx
    m3_u32            as_idx, free_as_idx, free_rc_idx;
    m3_r_as_inf_t     *pras;
    m3_reg_rsp_inf_t  regrsp;
    m3ua_r_as_t       new_as;
    m3_u8             *p_msg = M3_NULL;
    m3_u32            msglen = 0;
    m3ua_mgmt_ntfy_t  ntfy;
    m3_u8             regStatus = M3_AS_NOT_REGD_IN_ASP;

    ntfy.type = M3_MGMT_NTFY_REGISTER;
    ntfy.param.reg.lsp_id = pconn->l_sp_id;
    ntfy.param.reg.asp_id = pconn->r_sp_id;
    ntfy.param.reg.num_as = 0;
    regrsp.num_reg_result = 0;
    for (rk_idx = 0; rk_idx < pInf->num_rk; rk_idx++) {
        // for (i_rk_idx = rk_idx + 1; i_rk_idx < pInf->num_rk; i_rk_idx++) {
        //    if (M3_UNIQUE != m3uaRKMatch(&pInf->rk_list[rk_idx].rk_inf,
        //            &pInf->rk_list[i_rk_idx].rk_inf)) {
        //        regrsp.rr_list[regrsp.num_reg_result].lrk_id = pInf->rk_list[rk_idx].lrk_id;
        //        regrsp.rr_list[regrsp.num_reg_result].reg_status = 6;
        //        regrsp.rr_list[regrsp.num_reg_result].rtctx = 0;
        //        regrsp.num_reg_result++;
        //        break;
        //    }
        // }
        // if (i_rk_idx != pInf->num_rk) {
        //    continue;
        // }
        free_as_idx = M3_MAX_U16;
        for (as_idx = 0; as_idx < M3_MAX_R_AS; as_idx++) {
            if (M3_FALSE == pconn->rtctxUsed[as_idx]) {
                free_rc_idx = as_idx;
	    }
            pras = M3_R_AS_TABLE(as_idx);
            if (M3_FALSE == pras->e_state) {
                free_as_idx = as_idx;
                continue;
            }
            rk_match = m3uaRKMatch(&pInf->rk_list[rk_idx].rk_inf, &pras->rkey);
            if (M3_UNIQUE == rk_match) {
                continue;
            } else if (M3_PARTIAL_MATCH == rk_match) {
                regrsp.rr_list[regrsp.num_reg_result].lrk_id = pInf->rk_list[rk_idx].lrk_id;
                regrsp.rr_list[regrsp.num_reg_result].reg_status = 6;
                regrsp.rr_list[regrsp.num_reg_result].rtctx = 0;
                regrsp.num_reg_result++;
                break;
            } else if (M3_MATCH == rk_match) {
                /* traffic mode check, should be same for already regd/provisioned remote AS */
                if (pInf->rk_list[rk_idx].rk_inf.trfmode != pras->rkey.trfmode)
                {
                    regrsp.rr_list[regrsp.num_reg_result].lrk_id = pInf->rk_list[rk_idx].lrk_id;
                    regrsp.rr_list[regrsp.num_reg_result].reg_status = 10;
                    regrsp.rr_list[regrsp.num_reg_result].rtctx = 0;
                    regrsp.num_reg_result++;
                    break;
                }
                /* check if remote AS is already regd, then send response */
                regStatus = m3uaRASAlreadyRegistered(pconn, as_idx);
                switch (regStatus) {
                  case M3_ASP_REGD_IN_AS: {
                    regrsp.rr_list[regrsp.num_reg_result].lrk_id = pInf->rk_list[rk_idx].lrk_id;
                    regrsp.rr_list[regrsp.num_reg_result].reg_status = 0;
                    regrsp.rr_list[regrsp.num_reg_result].rtctx = m3uaGetRCFromRAS(pconn, as_idx);
                    regrsp.num_reg_result++;
                    break;
                  }
                  case M3_ASP_NOT_REGD_IN_AS: {
                    m3uaRegisterRASPInRAS(pconn, as_idx);
                    ntfy.param.reg.as_list[ntfy.param.reg.num_as] = as_idx;
                    ntfy.param.reg.num_as++;
                    m3uaSaveLRKID(pconn, pras, pInf->rk_list[rk_idx].lrk_id);
                    m3uaModifyRRKState(pconn, M3_RK_REG_IN_PROG, 1, &as_idx);
                    break;
                  }
                  case M3_AS_DEREG_IN_PROG:
                  case M3_AS_REG_IN_PROG: {
                    ntfy.param.reg.as_list[ntfy.param.reg.num_as] = as_idx;
                    ntfy.param.reg.num_as++;
                    m3uaSaveLRKID(pconn, pras, pInf->rk_list[rk_idx].lrk_id);
                    m3uaModifyRRKState(pconn, M3_RK_REG_IN_PROG, 1, &as_idx);
                    break;
                  }
                }
                break;
            }
        }
        if (M3_MAX_R_AS == as_idx) {
            /* no match found for the routing key, add this remote AS */
            if (M3_MAX_U16 == free_as_idx) {
            /* no free remote AS available, deny permission */
                regrsp.rr_list[regrsp.num_reg_result].lrk_id = pInf->rk_list[rk_idx].lrk_id;
                regrsp.rr_list[regrsp.num_reg_result].reg_status = 8;
                regrsp.rr_list[regrsp.num_reg_result].rtctx = 0;
                regrsp.num_reg_result++;
            }
            /* create an entry for this AS */
            new_as.add.info.rtctx = pconn->conn_id;
            new_as.add.info.rtctx = (new_as.add.info.rtctx << 16) | (free_rc_idx & 0x0000FFFF);
            pconn->rtctxUsed[free_rc_idx] = M3_TRUE;
            new_as.add.info.rkey = pInf->rk_list[rk_idx].rk_inf;
            if (M3_MAX_U32 == new_as.add.info.rkey.trfmode) {
                new_as.add.info.rkey.trfmode = M3_DEF_R_AS_TRFMODE;
            }
            new_as.add.info.min_act_asp = 1;
            m3uaAddDynamicRAS(M3_R_AS_TABLE(free_as_idx), &new_as);
            m3uaRegisterRASPInRAS(pconn, free_as_idx);
            ntfy.param.reg.as_list[ntfy.param.reg.num_as] = free_as_idx;
            ntfy.param.reg.num_as++;
            m3uaSaveLRKID(pconn, M3_R_AS_TABLE(free_as_idx), pInf->rk_list[rk_idx].lrk_id);
            m3uaModifyRRKState(pconn, M3_RK_REG_IN_PROG, 1, &free_as_idx);
        }
    }
    if (0 != regrsp.num_reg_result)
    {
        /* build and send failure message for the rejected/provisioned routing keys */
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_REGRSP_SIZE, M3_RKM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        m3uaEncodeREGRSP(&regrsp, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        M3_MSG_FREE(p_msg, M3_RKM_POOL);
    }
    /* give indication to the LME for further processing */
    if (0 != ntfy.param.reg.num_as) {
        m3uaAddNtfyToList(&ntfy);
    }
    return 0;
}

m3_s32 m3uaProcRkStatus(m3_conn_inf_t *pconn,
                        void          *p_edata)
{
    m3ua_rkey_t       *pInf = (m3ua_rkey_t*)p_edata;
    m3_reg_rsp_inf_t  regrsp;
    m3_u16            rc_idx;
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;
    m3_r_as_inf_t     *pras;
    m3_u8             *p_msg = M3_NULL;
    m3_u32            msglen = 0;
    m3_u16            num_as = 0;
    m3_u32            as_list[M3_MAX_RK];

    regrsp.num_reg_result = 0;
    for (rc_idx = 0; rc_idx < pInf->status.num_result; rc_idx++) {
        /* check the state of RK mentioned in the API */
        if (M3_FALSE == m3uaAssertRRKState(pconn,M3_RK_REG_IN_PROG,1,
            &pInf->status.result[rc_idx].as_id)) {
            /* Show a warning trace and continue processing */
            //M3ERRNO(EM3_RKSTATE_MISMATCH);
            //return -1;
            continue;
        }
        /* check the state of remote ASP in the mentioned remote AS, it may be active */
        if (M3_ASP_INACTIVE != pconn->r_sp_st[pInf->status.result[rc_idx].as_id]) {
            //M3ERRNO(EM3_INV_ASPSTATE);
            //return -1;
            /* show warning trace and continue processing */
            continue;
        }
        if (0 != pInf->status.result[rc_idx].reg_status) {
            regrsp.rr_list[regrsp.num_reg_result].lrk_id =
                    m3uaGetLRKID(pconn, M3_R_AS_TABLE(pInf->status.result[rc_idx].as_id));
            regrsp.rr_list[regrsp.num_reg_result].rtctx = 0;
            regrsp.rr_list[regrsp.num_reg_result].reg_status =
                    pInf->status.result[rc_idx].reg_status;
            regrsp.num_reg_result++;
            /* delete the remote AS, if dynamically registered and does not have any ASP */
            pras = M3_R_AS_TABLE(pInf->status.result[rc_idx].as_id);
            if (M3_TRUE == pras->dyn_reg && M3_TRUE == m3uaCheckDeleteDynamicRAS(pras->as_id)) {
                m3uaDeleteRAS(pras, M3_NULL);
                m3uaDeregisterRASPFromRAS(pconn, pInf->status.result[rc_idx].as_id);
            } else {
                m3uaDeregisterRASPFromRAS(pconn, pInf->status.result[rc_idx].as_id);
            }
        } else {
            /* perform check for routing context */
            //for (list_idx = rc_idx + 1; list_idx < pInf->status.num_result; list_idx++) {
            //    if (pInf->status.result[rc_idx].rtctx == pInf->status.result[list_idx].rtctx) {
            //        M3ERRNO(EM3_RTCTX_NONUNIQUE);
            //        return -1;
            //    }
            //}
            //if (M3_FALSE == m3uaRRCUnique(pconn, pInf->status.result[rc_idx].rtctx)) {
            //    M3ERRNO(EM3_RTCTX_NONUNIQUE);
            //    return -1;
            //}
            /* Add remote AS to the local ASP/SGP, update routing context and state of RK */
            pras = M3_R_AS_TABLE(pInf->status.result[rc_idx].as_id);
            regrsp.rr_list[regrsp.num_reg_result].lrk_id =
                        m3uaGetLRKID(pconn, M3_R_AS_TABLE(pInf->status.result[rc_idx].as_id));
            regrsp.rr_list[regrsp.num_reg_result].rtctx = pras->rtctx;
            regrsp.rr_list[regrsp.num_reg_result].reg_status = 0;
            regrsp.num_reg_result++;
            if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                pasp = M3_ASP_TABLE(pconn->l_sp_id);
                pasp->r_as_inf[(m3_u16)pInf->status.result[rc_idx].as_id].\
                        rc_inf[(m3_u16)pconn->r_sp_id].rtctx = pras->rtctx;
                pasp->r_as_inf[(m3_u16)pInf->status.result[rc_idx].as_id].\
                        rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state = M3_RK_REGD;
            } else {
                psgp = M3_SGP_TABLE(pconn->l_sp_id);
                psgp->r_as_inf[(m3_u16)pInf->status.result[rc_idx].as_id].\
                        rc_inf[(m3_u16)pconn->r_sp_id].rtctx = pras->rtctx;
                psgp->r_as_inf[(m3_u16)pInf->status.result[rc_idx].as_id].\
                        rc_inf[(m3_u16)pconn->r_sp_id].dyn_reg_state = M3_RK_REGD;
            }
            as_list[num_as++] = pras->as_id;
        }
    }

    if (0 != regrsp.num_reg_result)
    {
        /* build and send registration response message */
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_REGRSP_SIZE, M3_RKM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        m3uaEncodeREGRSP(&regrsp, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        M3_MSG_FREE(p_msg, M3_RKM_POOL);
    }
    /* input event to the Remote AS state machine */
    for (rc_idx = 0; rc_idx < num_as; rc_idx++) {
        m3uaRAS(pconn, as_list[rc_idx], M3_AS_EVT_ASPIA, 0);
    }
    return 0;
}

m3_s32 m3uaProcDeregReq(m3_conn_inf_t *pconn,
                        void          *p_edata)
{
    m3_u16            rc_idx;
    m3_u32            as_id;
    m3_dreg_rsp_inf_t dregrsp;
    m3_r_as_inf_t     *pras;
    m3ua_mgmt_ntfy_t  ntfy;
    m3_dreg_req_inf_t *pInf = (m3_dreg_req_inf_t *)p_edata;
    m3_u8             *p_msg = M3_NULL;
    m3_u32            msglen = 0;

    ntfy.type = M3_MGMT_NTFY_DEREGISTER;
    ntfy.param.dereg.lsp_id = pconn->l_sp_id;
    ntfy.param.dereg.asp_id = pconn->r_sp_id;
    ntfy.param.dereg.num_as = 0;
    dregrsp.num_dreg_result = 0;
    for (rc_idx = 0; rc_idx < pInf->num_rc; rc_idx++) {
        /* check for the routing context validity */
        as_id = m3uaGetRASFromRC(pconn, pInf->rc_list[rc_idx]);
        if (M3_MAX_U32 == as_id) {
            /* invalid routing context received */
            dregrsp.drr_list[dregrsp.num_dreg_result].rtctx = pInf->rc_list[rc_idx];
            dregrsp.drr_list[dregrsp.num_dreg_result].dreg_status = 2;
            dregrsp.num_dreg_result++;
            continue;
        }
        /* check the state of remote ASP in the remote AS */
        if (M3_ASP_ACTIVE == pconn->r_sp_st[(m3_u16)as_id]) {
            dregrsp.drr_list[dregrsp.num_dreg_result].rtctx = pInf->rc_list[rc_idx];
            dregrsp.drr_list[dregrsp.num_dreg_result].dreg_status = 5;
            dregrsp.num_dreg_result++;
            continue;
        }
        /* check if RRK is in stable state and ready to be dereg'd */
        if (M3_FALSE == m3uaAssertRRKState(pconn, M3_RK_REGD, 1, &as_id) &&
            M3_FALSE == m3uaAssertRRKState(pconn, M3_RK_STATIC, 1, &as_id)) {
            /* rk not reged for the given routing context */
            dregrsp.drr_list[dregrsp.num_dreg_result].rtctx = pInf->rc_list[rc_idx];
            dregrsp.drr_list[dregrsp.num_dreg_result].dreg_status = 4;
            dregrsp.num_dreg_result++;
            continue;
        }
        pras = M3_R_AS_TABLE(as_id);
        dregrsp.drr_list[dregrsp.num_dreg_result].rtctx = pInf->rc_list[rc_idx];
        dregrsp.drr_list[dregrsp.num_dreg_result].dreg_status = 0;
        dregrsp.num_dreg_result++;
        m3uaRAS(pconn, pras->as_id, M3_AS_EVT_ASPDN, NULL);
        /* if state of remote AS is now down and it is a dynamic AS, delete it */
        if (M3_TRUE == pras->dyn_reg && M3_TRUE == m3uaCheckDeleteDynamicRAS(pras->as_id)) {
            m3uaDeleteRAS(pras, M3_NULL);
        }
        /* delete remote ASP from the remote AS in the local SP config */
        m3uaDeregisterRASPFromRAS(pconn, pras->as_id);
        ntfy.param.dereg.as_list[ntfy.param.dereg.num_as] = as_id;
        ntfy.param.dereg.num_as++;
    }
    if (0 != dregrsp.num_dreg_result)
    {
        /* build and send registration response message */
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DEREGRSP_SIZE, M3_RKM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        m3uaEncodeDREGRSP(&dregrsp, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        M3_MSG_FREE(p_msg, M3_RKM_POOL);
    }

    /* indication to LME about de-registered Routing keys */
    if (0 != ntfy.param.dereg.num_as) {
        m3uaAddNtfyToList(&ntfy);
    }
    return 0;
}

