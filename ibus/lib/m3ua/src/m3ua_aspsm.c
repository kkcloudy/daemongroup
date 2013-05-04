/****************************************************************************
** Description:
** Code for ASPSM fsm.
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

/* Making changes for rkm procedure */

#include <m3ua_aspsm.h>

m3_s32    m3ua_ASPDOWN_ESENDUP(m3_conn_inf_t *pConn,
                               void          *pEvData)
{
    m3_aspup_inf_t    aspup;
    m3_u8             *pMsg = M3_NULL;
    m3_u32            msglen = 0;
    m3_u8             sess_id = M3_MAX_U8;
    m3_aspm_msg_inf_t aspm_msg_inf;
    m3_timer_param_t  *ptimerbuf; 

    M3TRACE(m3uaAspmTrace, ("Sending ASPUP in ASPDOWN state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPUP_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
        M3_TIMER_POOL);
    if (M3_NULL == pMsg) {
        M3_MSG_FREE(pMsg, M3_ASPM_POOL);
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    /* JUNE 2010 */
    if ((pConn->opt & M3_CONN_EXCL_ASPID) == 0)
        aspup.m3asp_id = (M3_ASP_TABLE(pConn->l_sp_id))->m3asp_id;
    else
        aspup.m3asp_id = M3_MAX_U32;

    aspup.info_len = 0;
    aspup.p_info_str = M3_NULL;
    m3uaEncodeASPUP(&aspup, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3TRACE(m3uaAspmTrace, ("Sent ASPUP message"));
    pConn->l_sp_g_st = M3_ASP_UPSENT;
    m3uaGetFreeSess(pConn, &sess_id);
    aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPUP;
    aspm_msg_inf.msg_len      = msglen;
    aspm_msg_inf.msg          = pMsg;
    m3uaSetASPMMsgInf(pConn, sess_id, &aspm_msg_inf);

    ptimerbuf->type = M3_TIMER_TYPE_ASPM;
    ptimerbuf->param.aspmbuf.conn_id = pConn->conn_id;
    ptimerbuf->param.aspmbuf.sess_id = sess_id;
    m3uaStartASPMTimer(pConn, sess_id, ptimerbuf, M3_ASPM_TIMER_INT_LOW);
    M3TRACE(m3uaAspmTrace, ("Started timer, timer id:%u", sess_id));
    return 0;
}

m3_s32    m3ua_ASPDOWN_ERECVUP(m3_conn_inf_t *pConn,
                               void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    m3_u16                idx;
    m3_r_asp_inf_t        *pRAsp = M3_R_ASP_TABLE(pConn->r_sp_id);

    M3TRACE(m3uaAspmTrace, ("Received ASPUP in ASPDOWN state"));
    if (M3_TRUE == pRAsp->lock_state) {
        M3TRACE(m3uaAspmTrace, ("Remote ASP:%u is locked", pRAsp->asp_id));
        return 0;
    }
    pConn->l_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    return 0;
}

m3_s32    m3ua_ASPUPSENT_ERECVUP(m3_conn_inf_t *pConn,
                                 void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u8                 sess_id;
    m3_u8                 n_sess;
    m3_u16                idx;
    m3_r_asp_inf_t        *pRAsp = M3_R_ASP_TABLE(pConn->r_sp_id);

    M3TRACE(m3uaAspmTrace, ("Received ASPUP in ASPUP-SENT state"));
    if (M3_TRUE == pRAsp->lock_state) {
        M3TRACE(m3uaAspmTrace, ("Remote ASP:%u is locked", pRAsp->asp_id));
        return 0;
    }

    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPUP, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);

    pConn->l_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    return 0;
}
 
m3_s32    m3ua_ASPUPSENT_ERECVUPACK(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u8            sess_id;
    m3_u8            n_sess;
    m3_u16           idx;

    M3TRACE(m3uaAspmTrace, ("Received ASPUP-ACK in ASPUP-SENT state"));
    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPUP, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);
    pConn->l_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    return 0;
}
 
m3_s32    m3ua_ASPUPSENT_ERECVDN(m3_conn_inf_t *pConn,
                                 void          *pEvData)
{
    m3_u8                 sess_id;
    m3_u8                 n_sess;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN in ASPUP-SENT state"));
    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPUP, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);

    pConn->l_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}
 
m3_s32    m3ua_ASPUPSENT_ERECVDNACK(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    m3_u8                 sess_id;
    m3_u8                 n_sess;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN-ACK in ASPUP-SENT state"));
    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPUP, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);

    pConn->l_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}
 
m3_s32    m3ua_ASPUPSENT_ETIMEREXP(m3_conn_inf_t *pConn,
                                   void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_timer_inf_t        *pInf = (m3_timer_inf_t*)pEvData;
    m3_u32                interval = M3_ASPM_TIMER_INT_LOW;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)pInf->p_buf;
    m3_u8                 sess_id = ptimerbuf->param.aspmbuf.sess_id;
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("Timer:%u expired in ASPUP-SENT state", sess_id));
    if (M3_ASPM_RETRY_LOW <= pConn->aspm_sess_inf[sess_id].num_retry) {
        interval = M3_ASPM_TIMER_INT_HIGH;
    }
    if (M3_ASPM_MAX_RETRY <= pConn->aspm_sess_inf[sess_id].num_retry ||
        -1 == m3uaStartASPMTimer(pConn, sess_id, ptimerbuf, interval)) {
        m3uaDeleteConnSess(pConn, sess_id);
        pConn->l_sp_g_st = M3_ASP_DOWN;
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            pConn->l_sp_st[idx] = M3_ASP_DOWN;
        }
        m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
        M3TRACE(m3uaAspmTrace, ("ASPM procedure aborted, all retries done"));
    } else {
        m3uaSendMsg(pConn, 0, pConn->aspm_sess_inf[sess_id].msg_inf.msg_len,
            pConn->aspm_sess_inf[sess_id].msg_inf.msg);
        M3TRACE(m3uaAspmTrace, ("Re-sent ASPUP message"));
        pConn->aspm_sess_inf[sess_id].num_retry++;
        //pConn->aspm_sess_inf[sess_id].timer_inf = *pInf;
    }
    return 0;
}

m3_s32    m3ua_ESENDDN(m3_conn_inf_t *pConn,
                       void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u8                 sess_id;
    m3_aspm_msg_inf_t     aspm_msg_inf;
    m3_aspdn_inf_t        aspdn;
    m3_timer_param_t      *ptimerbuf;
    m3_u32                msglen = 0;
    m3_u8                 *pMsg;
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("Sending ASPDN"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), 
        M3_TIMER_POOL);
    if (M3_NULL == ptimerbuf) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3_MSG_FREE(pMsg, M3_ASPM_POOL);
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }

    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped ASPM Timer:%u", idx));
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->rkm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped RKM Timer:%u", idx));
    }
    aspdn.info_len = 0;
    aspdn.p_info_str = M3_NULL;
    m3uaEncodeASPDN(&aspdn, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3TRACE(m3uaAspmTrace, ("Sent ASPDN message"));

    pConn->l_sp_g_st = M3_ASP_DNSENT;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaGetFreeSess(pConn, &sess_id);
    aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPDN;
    aspm_msg_inf.msg_len      = msglen;
    aspm_msg_inf.msg          = pMsg;
    m3uaSetASPMMsgInf(pConn, sess_id, &aspm_msg_inf);
    ptimerbuf->type = M3_TIMER_TYPE_ASPM;
    ptimerbuf->param.aspmbuf.conn_id = pConn->conn_id;
    ptimerbuf->param.aspmbuf.sess_id = sess_id;
    m3uaStartASPMTimer(pConn, sess_id, ptimerbuf, M3_ASPM_TIMER_INT_LOW);
    M3TRACE(m3uaAspmTrace, ("Started ASPM Timer:%u", sess_id));
    return 0;
}
 
m3_s32    m3ua_ASPINACTIVE_ERECVDN(m3_conn_inf_t *pConn,
                                   void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN in ASP-INACTIVE state"));
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped ASPM Timer:%u", idx));
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->rkm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped RKM Timer:%u", idx));
    }
    pConn->l_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_ASPINACTIVE_ERECVDNACK(m3_conn_inf_t *pConn,
                                      void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u8                 idx;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN-ACK in ASP-INACTIVE state"));
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped ASPM Timer:%u", idx));
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->rkm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("stopped RKM Timer:%u", idx));
    }
    pConn->l_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_ASPDNSENT_ERECVDNACK(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    m3_u8                 sess_id;
    m3_u8                 n_sess;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN-ACK in ASPDN-SENT state"));
    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPDN, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);
    M3TRACE(m3uaAspmTrace, ("Stopped ASPM timer:%u", sess_id));
    pConn->l_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_ASPDNSENT_ERECVDN(m3_conn_inf_t *pConn,
                                 void          *pEvData)
{
    m3_u8                 sess_id;
    m3_u8                 n_sess;

    M3TRACE(m3uaAspmTrace, ("Received ASPDN in ASPDN-SENT state"));
    m3uaGetSessFromMsgType(pConn, M3_MSG_TYPE_ASPDN, &n_sess, &sess_id);
    m3uaFreeTimer(&pConn->aspm_sess_inf[sess_id].timer_inf);
    m3uaDeleteConnSess(pConn, sess_id);
    M3TRACE(m3uaAspmTrace, ("Stopped ASPM timer:%u", sess_id));
    pConn->l_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_ASPDNSENT_ETIMEREXP(m3_conn_inf_t *pConn,
                                   void          *pEvData)
{
    m3_timer_inf_t        *pInf = (m3_timer_inf_t*)pEvData;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)pInf->p_buf;
    m3_u8                 sess_id = ptimerbuf->param.aspmbuf.sess_id;
    m3_u32                interval = M3_ASPM_TIMER_INT_LOW;

    M3TRACE(m3uaAspmTrace, ("Timer expired in ASPDN-SENT state"));
    if (M3_ASPM_RETRY_LOW <= pConn->aspm_sess_inf[sess_id].num_retry) {
        interval = M3_ASPM_TIMER_INT_HIGH;
    }
    if (M3_ASPM_MAX_RETRY <= pConn->aspm_sess_inf[sess_id].num_retry ||
        -1 == m3uaStartASPMTimer(pConn, sess_id, ptimerbuf, interval)) {
        m3uaDeleteConnSess(pConn, sess_id);
        pConn->l_sp_g_st = M3_ASP_DOWN;
        m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
        M3TRACE(m3uaAspmTrace, ("ASPM procedure aborted, all retries done"));
    } else {
        m3uaSendMsg(pConn, 0, pConn->aspm_sess_inf[sess_id].msg_inf.msg_len,
            pConn->aspm_sess_inf[sess_id].msg_inf.msg);
        M3TRACE(m3uaAspmTrace, ("ASPDN message re-sent"));
        pConn->aspm_sess_inf[sess_id].num_retry++;
        //pConn->aspm_sess_inf[sess_id].timer_inf = *pInf;
    }
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ERECVDNACK(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u8                 idx;

    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK received in ASP-ACTIVE state"));
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("Stopped ASPM timer:%u", idx));
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->rkm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("Stopped RKM timer:%u", idx));
    }
    pConn->l_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ERECVUP(m3_conn_inf_t *pConn,
                                 void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("ASPUP received in ASP-ACTIVE state"));
    pConn->l_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ERECVDN(m3_conn_inf_t *pConn,
                                 void          *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in ASP-ACTIVE state"));
    for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state) {
            m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
            m3uaDeleteConnSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("Stopped ASPM timer:%u", idx));
        }
    }
    for (idx = 0; idx < M3_MAX_RKM_SESS_PER_CONN; idx++) {
        if (M3_TRUE == pConn->rkm_sess_inf[idx].e_state)
            m3uaFreeTimer(&pConn->rkm_sess_inf[idx].timer_inf);
            m3uaDeleteRKMSess(pConn, idx);
            M3TRACE(m3uaAspmTrace, ("Stopped RKM timer:%u", idx));
    }

    pConn->l_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pConn->l_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_RASPDOWN_ERECVUP(m3_conn_inf_t *pConn,
                                void          *pEvData)
{
    m3_u16           idx;
    m3_aspup_ack_inf_t    aspupack;
    m3_u8            *pMsg = M3_NULL;
    m3_u32           msglen = 0;
    m3_r_asp_inf_t   *pRAsp = M3_R_ASP_TABLE(pConn->r_sp_id);
    m3_error_inf_t   errinf;
    m3_aspup_inf_t   *pInf = (m3_aspup_inf_t*)pEvData;

    M3TRACE(m3uaAspmTrace, ("ASPUP received in RASP-DOWN state"));
    if (M3_TRUE == pRAsp->lock_state) {
        M3TRACE(m3uaAspmTrace, ("Remote ASP:%u locked", pRAsp->asp_id));
        errinf.err_code = EM3_REF_MGMT_BLOCK;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = 0;
        errinf.diag_len = pInf->msglen;
        errinf.p_diag_inf = pInf->p_msg;
        m3ua_ESENDERROR(pConn, &errinf);
        return 0;
    }

    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPUPACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspupack.info_len = 0;
    aspupack.p_info_str = M3_NULL;
    m3uaEncodeASPUPACK(&aspupack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPUP-ACK message sent"));

    pRAsp->m3asp_id = pInf->m3asp_id;
    pConn->r_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPIA);
    return 0;
}
 
m3_s32    m3ua_RASPDOWN_ERECVDN(m3_conn_inf_t *pConn,
                                void          *pEvData)
{
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;
    m3_aspdn_ack_inf_t    aspdnack;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in RASP-DOWN state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspdnack.info_len = 0;
    aspdnack.p_info_str = M3_NULL;
    m3uaEncodeASPDNACK(&aspdnack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK message sent"));
    return 0;
}

m3_s32    m3ua_RASPDOWN_ESENDUP(m3_conn_inf_t *pConn,
                                void          *pEvData)
{
    M3TRACE(m3uaAspmTrace, ("Sending ASPUP in RASP-DOWN state"));
    pConn->r_sp_g_st = M3_ASP_UPRECV;
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ERECVUP(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    m3_aspup_ack_inf_t    aspupack;
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;

    M3TRACE(m3uaAspmTrace, ("ASPUP received in RASP-INACTIVE state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPUPACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspupack.info_len = 0;
    aspupack.p_info_str = M3_NULL;
    m3uaEncodeASPUPACK(&aspupack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPUP-ACK message sent"));
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ERECVDN(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    m3_u16           idx;
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;
    m3_aspdn_ack_inf_t    aspdnack;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in RASP-INACTIVE state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspdnack.info_len = 0;
    aspdnack.p_info_str = M3_NULL;
    m3uaEncodeASPDNACK(&aspdnack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK message sent"));

    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ESENDDN(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    m3_u16           idx;

    M3TRACE(m3uaAspmTrace, ("Sending ASPDN in RASP-INACTIVE state"));
    pConn->r_sp_g_st = M3_ASP_DNRECV;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ERECVDNACK(m3_conn_inf_t *pConn,
                                       void          *pEvData)
{
    m3_u16           idx;

    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK received in RASP-INACTIVE state"));
    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ERECVUPACK(m3_conn_inf_t *pConn,
                                     void          *pEvData)
{
    m3_u16           idx;

    M3TRACE(m3uaAspmTrace, ("ASPUP-ACK received in RASPUP-RECV state"));
    pConn->r_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPIA);
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ERECVDNACK(m3_conn_inf_t *pConn,
                                     void          *pEvData)
{
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK received in RASPUP-RECV state"));
    pConn->r_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ERECVDN(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;
    m3_aspdn_ack_inf_t    aspdnack;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in RASPUP-RECV state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspdnack.info_len = 0;
    aspdnack.p_info_str = M3_NULL;
    m3uaEncodeASPDNACK(&aspdnack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK message sent"));
    pConn->r_sp_g_st = M3_ASP_DOWN;
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ESENDDN(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16        idx;

    M3TRACE(m3uaAspmTrace, ("Sending ASPDN in RASPUP-RECV state"));
    pConn->r_sp_g_st = M3_ASP_DNRECV;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ERECVUP(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16                idx;
    m3_r_asp_inf_t        *pRAsp = M3_R_ASP_TABLE(pConn->r_sp_id);
    m3_error_inf_t        errinf;
    m3_aspup_inf_t        *pInf = (m3_aspup_inf_t*)pEvData;

    M3TRACE(m3uaAspmTrace, ("ASPUP received in RASPUP-RECV state"));
    if (M3_TRUE == pRAsp->lock_state) {
        M3TRACE(m3uaAspmTrace, ("Remote ASP:%u locked", pRAsp->asp_id));
        errinf.err_code = EM3_REF_MGMT_BLOCK;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = 0;
        errinf.diag_len = pInf->msglen;
        errinf.p_diag_inf = pInf->p_msg;
        m3ua_ESENDERROR(pConn, &errinf);
        return 0;
    }

    pConn->r_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPIA);
    return 0;
}

m3_s32    m3ua_RASPUPRECV_ETIMEREXP(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    M3TRACE(m3uaAspmTrace, ("Timer expired in RASPUP-RECV state"));
    if (M3_ASP_DOWN == pConn->l_sp_g_st) {
        pConn->r_sp_g_st = M3_ASP_DOWN;
        /* indicate to sm new state of remote ASP */
        m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    }
    return 0;
}

m3_s32    m3ua_RASPDNRECV_ERECVDN(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16        idx;
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;
    m3_aspdn_ack_inf_t    aspdnack;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in RASPDN-RECV state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspdnack.info_len = 0;
    aspdnack.p_info_str = M3_NULL;
    m3uaEncodeASPDNACK(&aspdnack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK message sent"));

    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3ua_RASPDNRECV_ERECVDNACK(m3_conn_inf_t *pConn,
                                     void          *pEvData)
{
    m3_u16        idx;

    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK received in RASPDN-RECV state"));
    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3ua_RASPDNRECV_ETIMEREXP(m3_conn_inf_t *pConn,
                                    void          *pEvData)
{
    M3TRACE(m3uaAspmTrace, ("Timer expired in RASPDN-RECV state"));
    if (M3_ASP_DOWN == pConn->l_sp_g_st) {
        pConn->r_sp_g_st = M3_ASP_DOWN;
        m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    }
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ERECVDN(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16        idx;
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;
    m3_aspdn_ack_inf_t    aspdnack;

    M3TRACE(m3uaAspmTrace, ("ASPDN received in RASP-ACTIVE state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPDNACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspdnack.info_len = 0;
    aspdnack.p_info_str = M3_NULL;
    m3uaEncodeASPDNACK(&aspdnack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK message sent"));

    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ESENDDN(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16        idx;

    M3TRACE(m3uaAspmTrace, ("Send ASPDN in RASP-ACTIVE state"));
    pConn->r_sp_g_st = M3_ASP_DNRECV;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ERECVUP(m3_conn_inf_t *pConn,
                                  void          *pEvData)
{
    m3_u16        idx;
    m3_aspup_ack_inf_t    aspupack;
    m3_u8                 *pMsg = M3_NULL;
    m3_u32                msglen = 0;

    M3TRACE(m3uaAspmTrace, ("ASPUP received in RASP-ACTIVE state"));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPUPACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3TRACE(m3uaErrorTrace, ("No memory, malloc failed"));
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspupack.info_len = 0;
    aspupack.p_info_str = M3_NULL;
    m3uaEncodeASPUPACK(&aspupack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    M3TRACE(m3uaAspmTrace, ("ASPUP-ACK message sent"));

    pConn->r_sp_g_st = M3_ASP_INACTIVE;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_INACTIVE;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_INACTIVE);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPIA);
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ERECVDNACK(m3_conn_inf_t *pConn,
                                     void          *pEvData)
{
    m3_u16        idx;

    M3TRACE(m3uaAspmTrace, ("ASPDN-ACK received in RASP-ACTIVE state"));
    pConn->r_sp_g_st = M3_ASP_DOWN;
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
        pConn->r_sp_st[idx] = M3_ASP_DOWN;
    }
    m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
    m3uaInvokeRASFSM(pConn, M3_AS_EVT_ASPDN);
    return 0;
}

m3_s32    m3uaInvokeRASFSM(m3_conn_inf_t     *pConn,
                           m3_as_event_t     event)
{
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;
    m3_u16            idx;

    M3TRACE(m3uaAspmTrace, ("Invoking RAS state machine"));
    iu_log_debug("idx = %d, pConn->l_sp_id = %d, pConn->r_sp_id = %d\n",idx, pConn->l_sp_id, pConn->r_sp_id);
    if (M3_TRUE == M3_IS_SP_ASP(pConn->l_sp_id)) {
        iu_log_debug("asp\n");
        pasp = M3_ASP_TABLE(pConn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE==pasp->r_as_inf[idx].asp_list[(m3_u16)pConn->r_sp_id]) {
                iu_log_debug("idx = %d, event = %d\n",idx, event);
                m3uaRAS(pConn, idx, event, M3_NULL);
            }
        }
    } else {
        iu_log_debug("sgp\n");
        psgp = M3_SGP_TABLE(pConn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE == psgp->r_as_inf[idx].asp_list[(m3_u16)pConn->r_sp_id])
                m3uaRAS(pConn, idx, event, M3_NULL);
        }
    }
    return 0;
}

m3_s32 m3uaStartHBEAT(m3_conn_inf_t    *pConn)
{
    m3_hbeat_inf_t    hbeat;
    m3_timer_param_t  *ptimerbuf;
    m3_u8             *pMsg;
    m3_u32            msglen = 0;
    m3_aspm_msg_inf_t aspm_msg_inf;

    M3TRACE(m3uaAspmTrace, ("Starting hearbeat between LSP:%u RSP:%u", pConn->l_sp_id, pConn->r_sp_id));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_HBEAT_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
        M3_TIMER_POOL);
    if (M3_NULL == pMsg) {
        M3_MSG_FREE(pMsg, M3_ASPM_POOL);
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }

    hbeat.hbeat_data_len = 0;
    m3uaEncodeHBEAT(&hbeat, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3TRACE(m3uaAspmTrace, ("Heartbeat message sent"));

    pConn->hbeat_sess_inf.e_state = M3_TRUE;
    aspm_msg_inf.msg_type     = M3_MSG_TYPE_BEAT;
    aspm_msg_inf.msg_len      = msglen;
    aspm_msg_inf.msg          = pMsg;
    pConn->hbeat_sess_inf.msg_inf = aspm_msg_inf;

    ptimerbuf->type = M3_TIMER_TYPE_HBEAT;
    ptimerbuf->param.aspmbuf.conn_id = pConn->conn_id;
    ptimerbuf->param.aspmbuf.sess_id = 0;
    m3uaStartHBEATTimer(pConn, ptimerbuf, 2*(M3_HBEAT_TIMER_INT));
    return 0;
}

m3_s32 m3uaDeleteHbeatSess(m3_conn_inf_t *pconn)
{
    pconn->hbeat_sess_inf.e_state = M3_FALSE;
    pconn->hbeat_sess_inf.num_retry = 0;
    pconn->hbeat_sess_inf.msg_inf.msg_len = 0;
    if (NULL != pconn->hbeat_sess_inf.msg_inf.msg)
        M3_MSG_FREE(pconn->hbeat_sess_inf.msg_inf.msg, M3_ASPM_POOL);
    pconn->hbeat_sess_inf.msg_inf.msg = NULL;
    return 1;
}
                                                                                
m3_s32 m3uaStopHBEAT(m3_conn_inf_t     *pconn)
{
    m3uaFreeTimer(&pconn->hbeat_sess_inf.timer_inf);
    m3uaDeleteHbeatSess(pconn);
    return 0;
}

m3_s32 m3ua_ERECVHBEAT(m3_conn_inf_t    *pConn,
                       void             *pEvData)
{
    m3_hbeat_inf_t      *pInf = (m3_hbeat_inf_t*)pEvData;
    m3_hbeat_ack_inf_t  hbeat_ack;
    m3_u8               *pMsg = M3_NULL;
    m3_u32              msglen = 0;

    M3TRACE(m3uaAspmTrace, ("Hearbeat received: RSP:%u LSP:%u", pConn->r_sp_id, pConn->l_sp_id));
    pMsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_HBEAT_ACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == pMsg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    hbeat_ack.hbeat_data_len  = pInf->hbeat_data_len;
    hbeat_ack.p_hbeat_data = pInf->p_hbeat_data;
    m3uaEncodeHBEATACK(&hbeat_ack, &msglen, pMsg);
    m3uaSendMsg(pConn, 0, msglen, pMsg);
    M3TRACE(m3uaAspmTrace, ("Hearbeat-Ack sent"));
    M3_MSG_FREE(pMsg, M3_ASPM_POOL);
    return 0;
}

m3_s32 m3ua_ERECVHBEATACK(m3_conn_inf_t    *pConn,
                          void             *pEvData)
{
    M3TRACE(m3uaAspmTrace, ("Hearbeat-Ack received"));
    return 0;
}

m3_s32 m3ua_EHBEAT_TIMEREXP(m3_conn_inf_t    *pConn,
                            void             *pEvData)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pConn->conn_id, pConn->assoc_id);
    
    m3_timer_inf_t        *pInf = (m3_timer_inf_t*)pEvData;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)pInf->p_buf;
    m3_u16                idx;

    M3TRACE(m3uaAspmTrace, ("Hearbeat Timer expired"));
    if (M3_HBEAT_MAX_RETRY <= pConn->hbeat_sess_inf.num_retry) {
        M3TRACE(m3uaAspmTrace, ("All retries:%u done, stopping heartbeat procedure", pConn->hbeat_sess_inf.num_retry));
        M3TRACE(m3uaAspmTrace, ("Moving Connection to the Not-Established state"));
        m3uaStopHBEAT(pConn);
        pConn->l_sp_g_st = M3_ASP_DOWN;
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            pConn->l_sp_st[idx] = M3_ASP_DOWN;
        }
        m3uaNtfyASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
        if (M3_TRUE == M3_IS_SP_ASP(pConn->r_sp_id)) {
            pConn->r_sp_g_st = M3_ASP_DOWN;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                pConn->r_sp_st[idx] = M3_ASP_DOWN;
            }
            m3uaNtfyRASPState(pConn, M3_MAX_U32, M3_ASP_DOWN);
        }
        pConn->conn_state = M3_CONN_NOT_ESTB;
        m3uaNtfyConnState(pConn, M3_CONN_NOT_ESTB);
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
        for (idx = 0; idx < M3_MAX_ASPM_SESS_PER_CONN; idx++) {
            if (M3_TRUE == pConn->aspm_sess_inf[idx].e_state) {
                m3uaFreeTimer(&pConn->aspm_sess_inf[idx].timer_inf);
                m3uaDeleteConnSess(pConn, idx);
            }
        }
    } else {
        M3TRACE(m3uaAspmTrace, ("Re-sending Heartbeat message"));
        m3uaSendMsg(pConn, 0, pConn->hbeat_sess_inf.msg_inf.msg_len,
            pConn->hbeat_sess_inf.msg_inf.msg);
        m3uaStartHBEATTimer(pConn, ptimerbuf, 2*(M3_HBEAT_TIMER_INT));
        pConn->hbeat_sess_inf.num_retry++;
        //pConn->hbeat_sess_inf.timer_inf = *pInf;
    }
    return 0;
}

