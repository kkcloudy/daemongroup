/****************************************************************************
** Description:
** Code for management procedure.
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

#include <m3ua_mgmt.h>

void m3uaProcMGMT(m3_conn_inf_t *pconn,
                  m3_u32        stream_id,
                  m3_msg_hdr_t  *p_hdr,
                  m3_u8         *p_msg,
                  m3_u32        msglen)
{
    m3_ntfy_inf_t   ntfy;
    m3_error_inf_t  err;

    switch(p_hdr->msg_type) {
        case M3_MSG_TYPE_ERR: {
            M3TRACE(m3uaInMsgTrace, ("Received ERR message"));
            if (-1 != m3uaDecodeERROR(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN,
                p_msg+M3_MSG_HDR_LEN, &err)) {
                m3ua_ERECVERR(pconn, &err);
            }
            break;
        }
        case M3_MSG_TYPE_NTFY: {
            M3TRACE(m3uaInMsgTrace, ("Received NTFY message"));
            if (M3_TRUE == M3_IS_SP_SGP(pconn->l_sp_id)) {
                M3ERRNO(EM3_PROT_ERR);
                return;
            }
            if (-1 != m3uaDecodeNTFY(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg + M3_MSG_HDR_LEN, &ntfy)) {
                ntfy.msglen = msglen;
                ntfy.p_msg = p_msg;
                m3ua_ERECVNTFY(pconn, &ntfy);
            }
            break;
        }
        default: {
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

m3_s32 m3uaAssertERR(m3_conn_inf_t    *pconn,
                     m3_error_inf_t   *p_inf)
{
    switch(p_inf->err_code)
    {
        case EM3_ASPID_REQD:
        case EM3_INV_ASPID:
        case EM3_MISSING_PARAM:
        case EM3_NO_AS_CONF_FOR_ASP:
        case EM3_INV_PARAM_VAL:
        case EM3_PARAM_FIELD:
        case EM3_UNEXP_PARAM:
        case EM3_INV_VERSION:
        case EM3_UNSUPP_MSG_CLASS:
        case EM3_UNSUPP_MSG_TYPE:
        case EM3_INV_TRFMODE:
        case EM3_UNEXP_MSG:
        case EM3_PROT_ERR:
        case EM3_INV_STRMID: {
            break;
        }
        case EM3_INV_RC:
        case EM3_REF_MGMT_BLOCK: {
            if (0 == p_inf->num_rc) {
                return -1;
            }
            break;
        }
        case EM3_DST_STATUS_UNKNOWN: {
            if (0 == p_inf->num_pc) {
                return -1;
            }
            break;
        }
        case EM3_INV_NWAPP: {
            if (M3_MAX_U32 == p_inf->nw_app) {
                return -1;
            }
            break;
        }
        default: {
            M3ERRNO(EM3_INV_ERRCODE);
            return -1;
        }
    }
    if (M3_FALSE == m3uaAssertNwApp(p_inf->nw_app)) {
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    return 0;
}

m3_s32 m3ua_ERECVERR(m3_conn_inf_t   *pconn,
                     m3_error_inf_t  *p_inf)
{
    if (-1 != m3uaAssertERR(pconn, p_inf)) {
        return -1;
    }
    m3uaNtfyErr(pconn, p_inf);
    return 0;
}

m3_s32    m3uaAssertNTFY(m3_conn_inf_t   *pconn,
                         m3_ntfy_inf_t   *p_inf)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_u16           idx = 0, s_idx = 0;
    m3_u32           as_id;
    m3_error_inf_t   err;
    m3_u16           num_rc = 0;
    m3_asp_inf_t     *pasp = M3_ASP_TABLE(pconn->l_sp_id);

    err.num_rc = 0;
    for (idx = 0; idx < p_inf->num_rc;) {
        if (M3_MAX_U32 == m3uaGetASFromRC(pconn, p_inf->rc_list[idx])) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
            for (s_idx = idx; s_idx < p_inf->num_rc - 1; s_idx++) {
                p_inf->rc_list[s_idx] = p_inf->rc_list[s_idx + 1];
            }
            p_inf->num_rc = p_inf->num_rc - 1;
            continue;
        }
        idx = idx + 1;
    }
    if (M3_STATUS_TYP_OTHER  == p_inf->status_type   &&
        M3_STATUS_INF_ALT_ASP_ACT == p_inf->status_inf) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            as_id = m3uaGetASFromRC(pconn, p_inf->rc_list[idx]);
            if (M3_ASP_ACTIVE != pconn->l_sp_st[as_id]) {
                err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
            }
        }
    }
    if (0 != err.num_rc) {
        err.err_code = EM3_INV_RC;
        err.num_pc = 0;
        err.nw_app = M3_MAX_U32;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
    }
    if (0 == p_inf->num_rc && 0 != err.num_rc) {
        M3ERRNO(EM3_INV_RC);
        return -1;
    }
    if (0 == p_inf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == pasp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id]) {
                    num_rc++;
                }
            }
        }
        if (1 < num_rc) {
            M3ERRNO(EM3_RTCTX_ABSENT);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3ua_ERECVNTFY(m3_conn_inf_t *pconn,
                      m3_ntfy_inf_t *p_inf)
{
    m3_s32           retval = 0;
    m3_u32           as_id;
    m3_u16           idx;

    if (-1 == m3uaAssertNTFY(pconn, p_inf)) {
        return -1;
    }
    if (M3_STATUS_TYP_AS_STATE_CHG == p_inf->status_type) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            as_id = m3uaGetASFromRC(pconn, p_inf->rc_list[idx]);
            m3uaNtfyASState(as_id, pconn, p_inf->status_inf);
        }
    } else if (M3_STATUS_TYP_OTHER == p_inf->status_type) {
        switch(p_inf->status_inf)
        {
            case M3_STATUS_INF_ASP_FAILURE: {
                m3uaNtfyNotify(pconn, p_inf);
                break;
            }
            case M3_STATUS_INF_ALT_ASP_ACT: {
                m3uaNtfyNotify(pconn, p_inf);
                if (-1 == m3ua_ENTFY_ALTASPACTIVE(pconn, p_inf)) {
                    retval = -1;
                }
                break;
            }
            case M3_STATUS_INF_INSUFF_ASP_ACT: {
                m3uaNtfyNotify(pconn, p_inf);
                break;
            }
        }
    }
    return retval;
}

m3_s32 m3ua_ENTFY_ALTASPACTIVE(m3_conn_inf_t *pconn,
                               void          *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    m3_ntfy_inf_t     *p_inf = (m3_ntfy_inf_t*)p_edata;
    m3_u32            as_id, l_spid, r_spid;
    m3_u16            idx= 0;
    m3_asp_inf_t      *pasp = M3_ASP_TABLE(pconn->l_sp_id);

    if (0 != p_inf->num_rc) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            as_id = m3uaGetRASFromRC(pconn, p_inf->rc_list[idx]);
            pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
            m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
        }
    } else {
        l_spid = pconn->l_sp_id;
        r_spid = pconn->r_sp_id;
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (M3_TRUE == pasp->as_list[idx]) {
                if (M3_ASP_ACTIVE == pconn->l_sp_st[idx]) {
                    pconn->l_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyASPState(pconn, idx, M3_ASP_INACTIVE);
                }
            }
        }
    }
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        if (M3_TRUE == pasp->as_list[idx]) {
            if (M3_ASP_ACTIVE == pconn->l_sp_st[idx]) {
                break;
            }
        }
    }
    if (M3_MAX_AS <= idx) {
        pconn->l_sp_g_st = M3_ASP_INACTIVE;
        m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
    }
    return 0;
}

m3_s32 m3ua_ECONNSTATE(m3_conn_inf_t  *pconn,
                       void           *p_edata)
{
    m3ua_conn_state_t  *p_inf = (m3ua_conn_state_t*)p_edata;
    m3_u8              idx;
    m3_ntfy_inf_t      ntfy;
    m3_asp_inf_t       *pasp;
    m3_sgp_inf_t       *psgp;

    switch(p_inf->modify.state) {
        case M3_CONN_NOT_ESTB: {
            m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPDN_ACK, M3_NULL);
            if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                ntfy.status_type = M3_STATUS_TYP_OTHER;
                ntfy.status_inf = M3_STATUS_INF_ASP_FAILURE;
                ntfy.m3asp_id = (M3_R_ASP_TABLE(pconn->r_sp_id))->m3asp_id;
                ntfy.num_rc = 0;
                ntfy.info_len = 0;
                if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                    pasp = M3_ASP_TABLE(pconn->l_sp_id);
                    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                        if (M3_TRUE==pasp->r_as_inf[idx].\
                            asp_list[(m3_u16)pconn->r_sp_id])
                          	m3uaSendNtfy(M3_R_AS_TABLE(idx), pconn,pconn->l_sp_id, &ntfy);
                    }
                } else {
                    psgp = M3_SGP_TABLE(pconn->l_sp_id);
                    for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                        if (M3_TRUE == psgp->r_as_inf[idx].\
                            asp_list[(m3_u16)pconn->r_sp_id])
                            m3uaSendNtfy(M3_R_AS_TABLE(idx), pconn, pconn->l_sp_id, &ntfy);
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

m3_s32 m3ua_ESENDERROR(m3_conn_inf_t  *pconn,
                       m3_error_inf_t *perr)
{
    m3_u8           *pmsg;
    m3_u32          msglen;

    pmsg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ERROR_SIZE, M3_MGMT_POOL);
    if (M3_NULL == pmsg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    m3uaEncodeERROR(perr, &msglen, pmsg);
    m3uaSendMsg(pconn, 0, msglen, pmsg);
    M3_MSG_FREE(pmsg, M3_MGMT_POOL);
    return 0;
}

