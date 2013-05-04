/****************************************************************************
** Description:
** code for ssnm procedures.
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

#include <m3ua_ssnm.h>

void m3uaProcSSNM(m3_conn_inf_t *pconn,
                  m3_u32        stream_id,
                  m3_msg_hdr_t  *p_hdr,
                  m3_u8         *p_msg,
                  m3_u32        msglen)
{
    m3_duna_inf_t duna;
    m3_dava_inf_t dava;
    m3_daud_inf_t daud; /* received at SGP */
    m3_scon_inf_t scon;
    m3_dupu_inf_t dupu;
    m3_drst_inf_t drst;
    m3_error_inf_t err;

    M3TRACE(m3uaSsnmTrace, ("SSNM Message Type: %x", (unsigned int)p_hdr->msg_type));

    switch(p_hdr->msg_type)
    {
        case M3_MSG_TYPE_DUNA: {
            M3TRACE(m3uaInMsgTrace, ("Received DUNA message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDUNA(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &duna)) {
                duna.msglen = msglen;
                duna.p_msg = p_msg;
                m3ua_ERECVDUNA(pconn, &duna);
            }
            break;
        }
        case M3_MSG_TYPE_DAVA: {
            M3TRACE(m3uaInMsgTrace, ("Received DAVA message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDUNA(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &dava)) {
                dava.msglen = msglen;
                dava.p_msg = p_msg;
                m3ua_ERECVDAVA(pconn, &dava);
            }
            break;
        }
        case M3_MSG_TYPE_DAUD: {
            M3TRACE(m3uaInMsgTrace, ("Received DAUD message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDUNA(msglen-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &daud)) {
                daud.msglen = msglen;
                daud.p_msg = p_msg;
                m3ua_ERECVDAUD(pconn, &daud);
            }
            break;
        }
        case M3_MSG_TYPE_SCON: {
            M3TRACE(m3uaInMsgTrace, ("Received SCON message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeSCON(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &scon)) {
                scon.msglen = msglen;
                scon.p_msg = p_msg;
                m3ua_ERECVSCON(pconn, &scon);
            }
            break;
        }
        case M3_MSG_TYPE_DUPU: {
            M3TRACE(m3uaInMsgTrace, ("Received DUPU message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDUPU(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &dupu)) {
                dupu.msglen = msglen;
                dupu.p_msg = p_msg;
                m3ua_ERECVDUPU(pconn, &dupu);
            }
            break;
        }
        case M3_MSG_TYPE_DRST: {
            M3TRACE(m3uaInMsgTrace, ("Received DRST message"));
            if (M3_FALSE == M3_IS_SP_SGP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeDUNA(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &drst)) {
                drst.msglen = msglen;
                drst.p_msg = p_msg;
                m3ua_ERECVDRST(pconn, &drst);
            }
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Received Unsupported SSNM message type\n"));
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

//FEB2010
m3_s32 m3uaAssertDAUD(m3_conn_inf_t  *pconn,
                      m3_daud_inf_t  *p_inf)
{
    m3_error_inf_t err;
    m3_u16         idx;
    m3_u32         as_id;

    if (M3_MAX_U32 !=  p_inf->nw_app       && 
        M3_FALSE == m3uaAssertNwApp(p_inf->nw_app)) {
        M3ERRNO(EM3_INV_NA);
        err.err_code = EM3_INV_NA;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = p_inf->nw_app;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    err.num_rc = 0;
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        if (-1 == (as_id = m3uaGetRASFromRC(pconn, p_inf->rc_list[idx]))) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
        } else if (M3_FALSE == (M3_SGP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].asp_list[pconn->r_sp_id]) { //FEB2010
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
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
    return 0;
}

m3_s32 m3uaAssertDUNA(m3_conn_inf_t  *pconn,
                      m3_duna_inf_t  *p_inf)
{
    m3_error_inf_t err;
    m3_u16         idx;
    m3_u32         as_id;

    if (M3_MAX_U32 !=  p_inf->nw_app       && 
        M3_FALSE == m3uaAssertNwApp(p_inf->nw_app)) {
        M3ERRNO(EM3_INV_NA);
        err.err_code = EM3_INV_NA;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = p_inf->nw_app;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    err.num_rc = 0;
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        if (-1 == (as_id = m3uaGetASFromRC(pconn, p_inf->rc_list[idx]))) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
        } else if (M3_FALSE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id]) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
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
    return 0;
}

m3_s32 m3uaAssertSCON(m3_conn_inf_t  *pconn,
                      m3_scon_inf_t  *p_inf)
{
    m3_error_inf_t err;
    m3_u16         idx;
    m3_u32         as_id;

    if (M3_MAX_U32 !=  p_inf->nw_app       && 
        M3_FALSE == m3uaAssertNwApp(p_inf->nw_app)) {
        M3ERRNO(EM3_INV_NA);
        err.err_code = EM3_INV_NA;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = p_inf->nw_app;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    if (3 < p_inf->cong_level) {
        M3ERRNO(EM3_INV_PARAM_VAL);
        err.err_code = EM3_INV_PARAM_VAL;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = M3_MAX_U32;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    err.num_rc = 0;
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        if (-1 == (as_id = m3uaGetASFromRC(pconn, p_inf->rc_list[idx]))) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
        } else if (M3_FALSE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id]) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
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
    return 0;
}

m3_s32 m3uaAssertDUPU(m3_conn_inf_t   *pconn,
                      m3_dupu_inf_t   *p_inf)
{
    m3_error_inf_t    err;
    m3_u16            idx;
    m3_u32            as_id;

    if (M3_MAX_U32 !=  p_inf->nw_app       && 
        M3_FALSE == m3uaAssertNwApp(p_inf->nw_app)) {
        M3ERRNO(EM3_INV_NA);
        err.err_code = EM3_INV_NA;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = p_inf->nw_app;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    if (2 < p_inf->cause) {
        M3ERRNO(EM3_INV_PARAM_VAL);
        err.err_code = EM3_INV_PARAM_VAL;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = M3_MAX_U32;
        err.diag_len = p_inf->msglen;
        err.p_diag_inf = p_inf->p_msg;
        m3ua_ESENDERROR(pconn, &err);
        return -1;
    }
    err.num_rc = 0;
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        if (-1 == (as_id = m3uaGetASFromRC(pconn, p_inf->rc_list[idx]))) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
        } else if (M3_FALSE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id]) {
            err.rc_list[err.num_rc++] = p_inf->rc_list[idx];
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
    return 0;
}

m3_s32    m3ua_ERECVDUNA(m3_conn_inf_t *pconn,
                         m3_duna_inf_t *p_inf)
{
    m3_u16                idx;
    m3_u8                 mask;

    if (-1 == m3uaAssertDUNA(pconn, p_inf)) {
        return -1;
    }
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        mask = (p_inf->pc_list[idx] >> 24) & 0x000000FF;
        p_inf->pc_list[idx] = p_inf->pc_list[idx] & 0x00FFFFFF;
        m3uaUpdateNwState(pconn, p_inf->nw_app, mask, p_inf->pc_list[idx],
            M3_RTSTATE_DOWN);
    }
    return 0;
}

m3_s32    m3ua_ERECVDAVA(m3_conn_inf_t *pconn,
                         m3_dava_inf_t *p_inf)
{
    m3_u16                idx;
    m3_u8                 mask;

    if (-1 == m3uaAssertDUNA(pconn, p_inf)) {
        return -1;
    }
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        mask = (p_inf->pc_list[idx] >> 24) & 0x000000FF;
        p_inf->pc_list[idx] = p_inf->pc_list[idx] & 0x00FFFFFF;
        m3uaUpdateNwState(pconn, p_inf->nw_app, mask, p_inf->pc_list[idx],
            M3_RTSTATE_UP);
    }
    return 0;
}

m3_s32    m3ua_ERECVDAUD(m3_conn_inf_t *pconn,
                         m3_daud_inf_t *p_inf)
{
    m3_u16                idx;
    m3_u8                 mask;

    if (-1 == m3uaAssertDAUD(pconn, p_inf)) { //FEB2010
        return -1;
    }
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        mask = (p_inf->pc_list[idx] >> 24) & 0x000000FF;
        p_inf->pc_list[idx] = p_inf->pc_list[idx] & 0x00FFFFFF;
        m3uaNtfyAudit(pconn, p_inf->nw_app, mask, p_inf->pc_list[idx]);
    }
    return 0;
}

m3_s32    m3ua_ERECVSCON(m3_conn_inf_t *pconn,
                         m3_scon_inf_t *p_inf)
{
    m3_u16                idx;
    m3_rtstate_t          rtstate;
    m3_u8                 mask;

    if (-1 == m3uaAssertSCON(pconn, p_inf)) {
        return -1;
    }
    switch(p_inf->cong_level)
    {
        case 1: {
            rtstate = M3_RTSTATE_CONG_1;
            break;
        }
        case 2: {
            rtstate = M3_RTSTATE_CONG_2;
            break;
        }
        case 3: {
            rtstate = M3_RTSTATE_CONG_3;
            break;
        }
        default: {
            rtstate = M3_RTSTATE_UP;
            break;
        }
    }
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        mask = (p_inf->pc_list[idx] >> 24) & 0x000000FF;
        p_inf->pc_list[idx] = p_inf->pc_list[idx] & 0x00FFFFFF;
        m3uaUpdateNwState(pconn, p_inf->nw_app, mask,
            p_inf->pc_list[idx], rtstate);
    }
    return 0;
}

m3_s32    m3ua_ERECVDUPU(m3_conn_inf_t *pconn,
                         m3_dupu_inf_t *p_inf)
{
    if (-1 == m3uaAssertDUPU(pconn, p_inf)) {
        return -1;
    }
    m3uaNtfyStatus(p_inf->nw_app, p_inf->pc, M3_MAX_U32, p_inf->cause,
        p_inf->user);
    return 0;
}

m3_s32    m3ua_ERECVDRST(m3_conn_inf_t *pconn,
                         m3_drst_inf_t *p_inf)
{
    m3_u16                idx;
    m3_u8                 mask;

    if (-1 == m3uaAssertDUNA(pconn, p_inf)) {
        return -1;
    }
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        mask = (p_inf->pc_list[idx] >> 24) & 0x000000FF;
        p_inf->pc_list[idx] = p_inf->pc_list[idx] & 0x00FFFFFF;
        m3uaUpdateNwState(pconn, p_inf->nw_app, mask, p_inf->pc_list[idx],
            M3_RTSTATE_RESTRICTED);
    }
    return 0;
}

void    m3uaUpdateNwState(m3_conn_inf_t   *pconn,
                          m3_u32          nw_app,
                          m3_u8           mask,
                          m3_u32          ptcode,
                          m3_rtstate_t    rtstate)
{
    m3_pc_inf_t      pcinf;
    m3_u32           ptcode_l = 0, ptcode_h = 0;
    m3_u8            nwapp_idx;
    m3_standard_t    std;
    m3_u32           sgid;
    m3_u16           idx;

    sgid = (M3_R_SGP_TABLE(pconn->r_sp_id))->sg_id;
    if (M3_MAX_U32 == nw_app) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id))
            nw_app = (M3_ASP_TABLE(pconn->l_sp_id))->def_nwapp;
        else
            nw_app = (M3_SGP_TABLE(pconn->l_sp_id))->def_nwapp;
    }
    for (nwapp_idx = 0; nwapp_idx < M3_MAX_NWAPP; nwapp_idx++) {
        if (nw_app == (M3_NWAPP_TABLE(nwapp_idx))->nw_app)
            break;
    }
    std = (M3_NWAPP_TABLE(nwapp_idx))->standard;
    switch(std)
    {
        case M3_STD_ITU: {
            ptcode = ptcode & 0x00003FFF;
            if (14 < mask)
                mask = 14;
            break;
        }
        case M3_STD_ANSI: {
            ptcode = ptcode & 0x00FFFFFF;
            if (24 < mask)
                mask = 24;
            break;
        }
    }
    ptcode_l = ptcode & (0xFFFFFFFF << mask);
    pcinf.ptcode = ptcode_l;
    pcinf.nw_app = nw_app;
    for (idx = 0; idx < mask; idx++)
        ptcode_h = (ptcode_h << 1) | 1;
    ptcode_h = ptcode | ptcode_h;
    if (8 < mask) {
        m3uaSetRTStatex(pcinf,sgid,(0xFFFFFFFF << mask),M3_MAX_U8,rtstate);
    } else {
        for (pcinf.ptcode = ptcode_l; pcinf.ptcode <= ptcode_h; pcinf.ptcode++)
            m3uaSetRTState(&pcinf, sgid, M3_MAX_U8, rtstate);
    }
    return;
}

m3_s32 m3uaNtfyPause(m3_u32            nwapp,
                     m3_u32            ptcode)
{
    m3_user_ntfy_inf_t    *plist = M3_USER_NTFY_TABLE;
    m3_usr_inf_t          *pusr;
    m3_u16                idx;

    for (idx = 0; idx < M3_MAX_USR; idx++) {
        pusr = M3_USR_TABLE(idx);
        if (M3_TRUE == pusr->e_state) {
            if (M3_MAX_USER_NTFY == plist->n_ntfy_pd) {
                M3ERRNO(EM3_USER_NTFY_TABLE_FULL);
                return -1;
            }
            plist->ntfy_list[plist->n_offset].type = M3_USER_NTFY_PAUSE;
            plist->ntfy_list[plist->n_offset].param.pause.user_id = idx;
            plist->ntfy_list[plist->n_offset].param.pause.nw_app = nwapp;
            plist->ntfy_list[plist->n_offset].param.pause.ptcode = ptcode;
            plist->n_ntfy_pd++;
            plist->n_offset++;
            if (M3_MAX_USER_NTFY == plist->n_offset) {
                plist->n_offset = 0;
            }
        }
    }
    return 0;
}

m3_s32 m3uaNtfyResume(m3_u32            nwapp,
                      m3_u32            ptcode)
{
    m3_user_ntfy_inf_t    *plist = M3_USER_NTFY_TABLE;
    m3_usr_inf_t          *pusr;
    m3_u16                idx;

    for (idx = 0; idx < M3_MAX_USR; idx++) {
        pusr = M3_USR_TABLE(idx);
        if (M3_TRUE == pusr->e_state) {
            if (M3_MAX_USER_NTFY == plist->n_ntfy_pd) {
                M3ERRNO(EM3_USER_NTFY_TABLE_FULL);
                return -1;
            }
            plist->ntfy_list[plist->n_offset].type = M3_USER_NTFY_RESUME;
            plist->ntfy_list[plist->n_offset].param.resume.user_id = idx;
            plist->ntfy_list[plist->n_offset].param.resume.nw_app = nwapp;
            plist->ntfy_list[plist->n_offset].param.resume.ptcode = ptcode;
            plist->n_ntfy_pd++;
            plist->n_offset++;
            if (M3_MAX_USER_NTFY == plist->n_offset) {
                plist->n_offset = 0;
            }
        }
    }
    return 0;
}

m3_s32 m3uaNtfyAudit(m3_conn_inf_t     *pconn,
                     m3_u32            nwapp,
                     m3_u8             mask,
                     m3_u32            ptcode)
{
    m3_user_ntfy_inf_t    *plist = M3_USER_NTFY_TABLE;
    m3_sgp_inf_t          *psgp = M3_SGP_TABLE(pconn->l_sp_id);

    if (M3_MAX_USER_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_USER_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_USER_NTFY_AUDIT;
    plist->ntfy_list[plist->n_offset].param.audit.nw_app = nwapp;
    plist->ntfy_list[plist->n_offset].param.audit.ptcode = ptcode;
    plist->ntfy_list[plist->n_offset].param.audit.user_id = psgp->user_id;
    plist->ntfy_list[plist->n_offset].param.audit.mask = mask;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_USER_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

m3_s32 m3uaNtfyStatus(m3_u32            nwapp,
                      m3_u32            ptcode,
                      m3_u32            crnd_dpc,
                      m3_u8             cause,
                      m3_u8             inf)
{
    m3_user_ntfy_inf_t    *plist = M3_USER_NTFY_TABLE;
    m3_usr_inf_t          *pusr;
    m3_u16                idx;

    for (idx = 0; idx < M3_MAX_USR; idx++) {
        pusr = M3_USR_TABLE(idx);
        if (M3_TRUE == pusr->e_state) {
            if (M3_MAX_USER_NTFY == plist->n_ntfy_pd) {
                M3ERRNO(EM3_USER_NTFY_TABLE_FULL);
                return -1;
            }
            plist->ntfy_list[plist->n_offset].type = M3_USER_NTFY_STATUS;
            plist->ntfy_list[plist->n_offset].param.status.user_id = idx;
            plist->ntfy_list[plist->n_offset].param.status.nw_app = nwapp;
            plist->ntfy_list[plist->n_offset].param.status.ptcode = ptcode;
            plist->ntfy_list[plist->n_offset].param.status.crnd_dpc = crnd_dpc;
            plist->ntfy_list[plist->n_offset].param.status.cause = cause;
            plist->ntfy_list[plist->n_offset].param.status.inf = inf;
            plist->n_ntfy_pd++;
            plist->n_offset++;
            if (M3_MAX_USER_NTFY == plist->n_offset) {
                plist->n_offset = 0;
            }
        }
    }
    return 0;
}

m3_s32 m3uaSendDAUD(m3_asp_inf_t       *pasp,
                    m3_u32             sg_id,
                    m3ua_audit_t       *p_inf)
{
    m3_daud_inf_t    daud;
    m3_u16           idx, s_idx;
    m3_sg_inf_t      *psg = M3_SG_TABLE(sg_id);
    m3_u32           conn_id;
    m3_conn_inf_t    *pconn;
    m3_u32           msglen = 0;
    m3_u8            *p_msg = M3_NULL;

    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DAUD_SIZE, M3_SSNM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    daud.nw_app = p_inf->nw_app;
    daud.num_pc = p_inf->num_pc;
    for (idx = 0; idx < p_inf->num_pc; idx++) {
        daud.pc_list[idx] = p_inf->pc_list[idx];
    }
    daud.info_len = p_inf->info_len;
    daud.p_info_str = p_inf->p_info_str;
    for (idx = 0; idx < psg->num_sgp; idx++) {
        conn_id = pasp->r_sgp_inf[(m3_u16)psg->sgp_list[idx]].conn_id;//Dancis--Apr2006
        if (M3_MAX_U32 != conn_id                           && 
            M3_ASP_ACTIVE == (M3_CONN_TABLE(conn_id))->l_sp_g_st) {
            daud.num_rc = 0;
            pconn = M3_CONN_TABLE(conn_id);
            for (s_idx = 0; s_idx < M3_MAX_AS; s_idx++) {
                if (M3_TRUE == (M3_AS_TABLE(s_idx))->e_state          &&
                    M3_ASP_ACTIVE == pconn->l_sp_st[s_idx]) {
                    daud.rc_list[daud.num_rc] = m3uaGetRCFromAS(pconn, s_idx);
                    daud.num_rc++;
                }
            }
            if (0 != daud.num_rc) {

                /* JULY 2010 */
                if (1 == (pconn->opt & M3_CONN_EXCL_RTCTX))
                    daud.num_rc = 0;

                m3uaEncodeDAUD(&daud, &msglen, p_msg);
                m3uaSendMsg(pconn, 0, msglen, p_msg);
            }
        }
    }
    M3_MSG_FREE(p_msg, M3_SSNM_POOL);
    return 0;
}

