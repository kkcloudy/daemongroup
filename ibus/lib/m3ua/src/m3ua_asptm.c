/****************************************************************************
** Description:
** Code for ASPTM fsm.
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

#include <m3ua_asptm.h>

m3_s32 m3uaAssertASPAC(m3_conn_inf_t    *pconn,
                       m3_aspac_inf_t   *pinf)
{
    m3_u16            idx, idx2;
    m3_u32            as_id;
    m3_bool_t         asp_status;
    m3_bool_t         trfmode_err = M3_FALSE;
    m3_error_inf_t    errinf;
    m3_u16            num_rc = 0;
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;
    m3_r_asp_inf_t    *prasp = M3_R_ASP_TABLE(pconn->r_sp_id);

    M3TRACE(m3uaAspmTrace, ("Asserting ASPAC"));
    if (M3_TRUE == prasp->lock_state) {
        M3TRACE(m3uaErrorTrace, ("Remote ASP is locked"));
        errinf.err_code = EM3_REF_MGMT_BLOCK;
        errinf.num_rc = pinf->num_rc;
        for (idx = 0; idx < pinf->num_rc; idx++)
            errinf.rc_list[idx] = pinf->rc_list[idx];
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = 0;
        errinf.diag_len = pinf->msglen;
        errinf.p_diag_inf = pinf->p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
        return -1;
    }
    errinf.num_rc = 0;
    if (M3_MAX_U32 != pinf->trfmode                          &&
        M3_FALSE == M3_TRFMODE_VALID(pinf->trfmode)) {
        M3TRACE(m3uaErrorTrace, ("Invalid Traffic Mode in ASPAC"));
        errinf.err_code = EM3_INV_TRFMODE;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = pinf->msglen;
        errinf.p_diag_inf = pinf->p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
        M3ERRNO(EM3_INV_TRFMODE);
        return -1;
    }
    for (idx = 0; idx < pinf->num_rc;) {
        if (M3_MAX_U32 == (as_id = m3uaGetRASFromRC(pconn, pinf->rc_list[idx]))) {
            M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u in ASPAC", pinf->rc_list[idx]));
            errinf.rc_list[errinf.num_rc++] = pinf->rc_list[idx];
            for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
            }
            pinf->num_rc--;
            continue;
        } else {
            if (M3_MAX_U32 != pinf->trfmode                        &&
                (M3_R_AS_TABLE(as_id))->rkey.trfmode != pinf->trfmode) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u,traffic mode:%u combination in ASPAC", pinf->rc_list[idx], pinf->trfmode));
                trfmode_err = M3_TRUE;
                errinf.rc_list[errinf.num_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
            if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                asp_status = (M3_ASP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].\
                             asp_list[(m3_u16)pconn->r_sp_id];
            } else {
                asp_status = (M3_SGP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].\
                             asp_list[(m3_u16)pconn->r_sp_id];
            }
            if (M3_FALSE == asp_status) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u in ASPAC", pinf->rc_list[idx]));
                errinf.rc_list[errinf.num_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
        }
        idx = idx + 1;
    }
    num_rc = errinf.num_rc;
    if (0 != errinf.num_rc) {
        errinf.err_code = EM3_INV_RC;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = pinf->msglen;
        errinf.p_diag_inf = pinf->p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
    }
    if (M3_FALSE != trfmode_err) {
        errinf.err_code = EM3_INV_TRFMODE;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = pinf->msglen;
        errinf.p_diag_inf = pinf->p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
    }
    if (0 == pinf->num_rc && 0 != num_rc) {
        M3ERRNO(EM3_INV_RC);
        return -1;
    }
    if (0 == pinf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == pasp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id]) {
                    num_rc++;
                }
            }
        } else {
           psgp = M3_SGP_TABLE(pconn->l_sp_id);
                for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == psgp->r_as_inf[idx].\
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

m3_s32 m3uaAssertASPIA(m3_conn_inf_t    *pconn,
                       m3_aspia_inf_t   *pinf)
{
    m3_u16        idx, idx2;
    m3_u32        as_id;
    m3_bool_t     asp_status;
    m3_error_inf_t    errinf;
    m3_u16            num_rc = 0;
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;

    M3TRACE(m3uaAspmTrace, ("Asserting ASPIA"));
    errinf.num_rc = 0;
    for (idx = 0; idx < pinf->num_rc;) {
        if (M3_MAX_U32 == (as_id = m3uaGetRASFromRC(pconn, pinf->rc_list[idx]))) {
            M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u in ASPIA", pinf->rc_list[idx]));
            errinf.rc_list[errinf.num_rc++] = pinf->rc_list[idx];
            for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
            }
            pinf->num_rc--;
            continue;
        } else {
            if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                asp_status = (M3_ASP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].\
                             asp_list[(m3_u16)pconn->r_sp_id];
            } else {
                asp_status = (M3_SGP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].\
                             asp_list[(m3_u16)pconn->r_sp_id];
            }
            if (M3_FALSE == asp_status) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u in ASPIA", pinf->rc_list[idx]));
                errinf.rc_list[errinf.num_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
        }
        idx = idx + 1;
    }
    if (0 != errinf.num_rc) {
        errinf.err_code = EM3_INV_RC;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = pinf->msglen;
        errinf.p_diag_inf = pinf->p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
    }
    if (0 == pinf->num_rc && 0 != errinf.num_rc) {
        M3ERRNO(EM3_INV_RC);
        return -1;
    }
    if (0 == pinf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == pasp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id]) {
                    num_rc++;
                }
            }
        } else {
           psgp = M3_SGP_TABLE(pconn->l_sp_id);
                for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == psgp->r_as_inf[idx].\
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

m3_s32 m3uaAssertASPACACK(m3_conn_inf_t      *pconn,
                          m3_aspac_ack_inf_t *pinf,
                          m3_u16             *num_inv_rc,
                          m3_u32             *inv_rclist)
{
    m3_u16        idx, idx2;
    m3_u32        as_id;
    m3_u16            num_rc = pinf->num_rc; /* original number of RC */
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;

    M3TRACE(m3uaAspmTrace, ("Asserting ASPAC-ACK"));
    if (M3_MAX_U32 != pinf->trfmode                           &&
        M3_FALSE == M3_TRFMODE_VALID(pinf->trfmode)) {
        M3TRACE(m3uaErrorTrace, ("Invalid Traffic mode:%u", pinf->trfmode));
        M3ERRNO(EM3_INV_TRFMODE);
        return -1;
    }
    for (idx = 0; idx < pinf->num_rc;) {
        if (M3_MAX_U32 == (as_id = m3uaGetASFromRC(pconn, pinf->rc_list[idx]))) {
            M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u", pinf->rc_list[idx]));
            inv_rclist[*num_inv_rc++] = pinf->rc_list[idx];
            for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
            }
            pinf->num_rc--;
            continue;
        } else {
            if (M3_MAX_U32 != pinf->trfmode                        &&
                (M3_AS_TABLE(as_id))->rkey.trfmode != pinf->trfmode) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u, traffic mode:%u combination", pinf->rc_list[idx],  pinf->trfmode));
                inv_rclist[*num_inv_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
            if (M3_FALSE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id]) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u", pinf->rc_list[idx]));
                inv_rclist[*num_inv_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
        }
        idx = idx + 1;
    }
    if (0 == pinf->num_rc && 0 != num_rc) {
        M3ERRNO(EM3_INV_RC);
        return -1;
    }
    if (0 == pinf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == pasp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id]) {
                    num_rc++;
                }
            }
        } else {
           psgp = M3_SGP_TABLE(pconn->l_sp_id);
                for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == psgp->r_as_inf[idx].\
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

m3_s32 m3uaAssertASPIAACK(m3_conn_inf_t      *pconn,
                          m3_aspia_ack_inf_t *pinf,
                          m3_u16             *num_inv_rc,
                          m3_u32             *inv_rclist)
{
    m3_u16        idx, idx2;
    m3_u32        as_id;
    m3_u16            num_rc = pinf->num_rc;
    m3_asp_inf_t      *pasp;
    m3_sgp_inf_t      *psgp;

    M3TRACE(m3uaAspmTrace, ("Asserting ASPIA-ACK"));
    for (idx = 0; idx < pinf->num_rc;) {
        if (M3_MAX_U32 == (as_id = m3uaGetASFromRC(pconn, pinf->rc_list[idx]))) {
            M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u", pinf->rc_list[idx]));
            inv_rclist[*num_inv_rc++] = pinf->rc_list[idx];
            for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
            }
            pinf->num_rc--;
            continue;
        } else {
            if (M3_FALSE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id]) {
                M3TRACE(m3uaErrorTrace, ("Invalid Routing Context:%u", pinf->rc_list[idx]));
                inv_rclist[*num_inv_rc++] = pinf->rc_list[idx];
                for (idx2 = idx; idx2 < pinf->num_rc - 1; idx2++) {
                    pinf->rc_list[idx2] = pinf->rc_list[idx2 + 1];
                }
                pinf->num_rc--;
                continue;
            }
        }
        idx = idx + 1;
    }
    if (0 == pinf->num_rc && 0 != num_rc) {
        M3ERRNO(EM3_INV_RC);
        return -1;
    }
    if (0 == pinf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == pasp->r_as_inf[idx].\
                    asp_list[(m3_u16)pconn->r_sp_id]) {
                    num_rc++;
                }
            }
        } else {
           psgp = M3_SGP_TABLE(pconn->l_sp_id);
                for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == psgp->r_as_inf[idx].\
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

void m3uaCreateCommonRCList(m3_u16       *p_nrc1,
                            m3_u32       *p_rclist1,
                            m3_u16       *p_nrc2,
                            m3_u32       *p_rclist2,
                            m3_u16       *p_nrcm,
                            m3_u32       *p_rclistm)
{
    m3_u32        rc_id;
    m3_u16        idx, idx2;
    m3_u16        rclist_idx = 0;
    m3_bool_t     rc_match;

    for (idx = 0; idx < *p_nrc1;){
        rc_id = p_rclist1[idx];
        rc_match = M3_FALSE;
        for (idx2 = 0; idx2 < *p_nrc2; idx2++) {
            if (rc_id == p_rclist2[idx2]) {
                p_rclistm[*p_nrcm] = rc_id;
                *p_nrcm = *p_nrcm + 1;
                rc_match = M3_TRUE;
                for (rclist_idx = idx2; rclist_idx < *p_nrc2-1; rclist_idx++) {
                    p_rclist2[rclist_idx] = p_rclist2[rclist_idx+1];
                }
                for (rclist_idx = idx; rclist_idx < *p_nrc1-1; rclist_idx++) {
                    p_rclist1[rclist_idx] = p_rclist2[rclist_idx+1];
                }
                *p_nrc1 = *p_nrc1 - 1;
                *p_nrc2 = *p_nrc2 - 1;
                break;
            }
        }
        if (M3_FALSE == rc_match)
            idx++;
    }
    return;
}

m3_s32 m3ua_RASP_ERECVACACK(m3_conn_inf_t *pconn,
                            m3_u32        as)
{
    m3_asp_inf_t    *pasp;
    m3_sgp_inf_t    *psgp;
    m3_u32          r_asid;
    m3_u32          rtctx = m3uaGetRCFromAS(pconn, as);

    if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        return 0;
    }
    if (M3_MAX_U32 == (r_asid = m3uaGetRASFromRC(pconn, rtctx))) {
        return 0;
    }
    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        if (M3_FALSE ==
            pasp->r_as_inf[(m3_u16)r_asid].asp_list[(m3_u16)pconn->r_sp_id]) {
            return 0;
        }
        if (M3_ASP_INACTIVE == pconn->r_sp_st[(m3_u16)r_asid]) {
            pconn->r_sp_st[(m3_u16)r_asid] = M3_ASP_ACTIVE;
            pasp->r_as_inf[(m3_u16)r_asid].num_asp_active++;
            pasp->r_as_inf[(m3_u16)r_asid].num_asp_inactive--;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_ACTIVE);
            if (M3_ASP_INACTIVE == pconn->r_sp_g_st) {
                pconn->r_sp_g_st = M3_ASP_ACTIVE;
            }
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPAC, M3_NULL);
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        if (M3_FALSE ==
            psgp->r_as_inf[(m3_u16)r_asid].asp_list[(m3_u16)pconn->r_sp_id]) {
            return 0;
        }
        if (M3_ASP_INACTIVE == pconn->r_sp_st[(m3_u16)r_asid]) {
            pconn->r_sp_st[(m3_u16)r_asid] = M3_ASP_ACTIVE;
            psgp->r_as_inf[(m3_u16)r_asid].num_asp_active++;
            psgp->r_as_inf[(m3_u16)r_asid].num_asp_inactive--;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_ACTIVE);
            if (M3_ASP_INACTIVE == pconn->r_sp_g_st) {
                pconn->r_sp_g_st = M3_ASP_ACTIVE;
            }
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPAC, M3_NULL);
        }
    }
    return 0;
}

m3_s32 m3ua_RASP_ERECVIAACK(m3_conn_inf_t *pconn,
                            m3_u32        as)
{
    m3_asp_inf_t    *pasp;
    m3_sgp_inf_t    *psgp;
    m3_u32          r_asid;
    m3_u32          rtctx = m3uaGetRCFromAS(pconn, as);

    if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        return 0;
    }
    if (M3_MAX_U32 == (r_asid = m3uaGetRASFromRC(pconn, rtctx))) {
        return 0;
    }
    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        if (M3_FALSE ==
            pasp->r_as_inf[(m3_u16)r_asid].asp_list[(m3_u16)pconn->r_sp_id]) {
            return 0;
        }
        if (M3_ASP_ACTIVE == pconn->r_sp_st[(m3_u16)r_asid]) {
            pconn->r_sp_st[(m3_u16)r_asid] = M3_ASP_INACTIVE;
            pasp->r_as_inf[(m3_u16)r_asid].num_asp_active--;
            pasp->r_as_inf[(m3_u16)r_asid].num_asp_inactive++;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_INACTIVE);
            if (0 == pasp->r_as_inf[(m3_u16)r_asid].num_asp_active) {
                pconn->r_sp_g_st = M3_ASP_INACTIVE;
                m3uaNtfyRASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
            }
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPIA, M3_NULL);
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        if (M3_FALSE ==
            psgp->r_as_inf[(m3_u16)r_asid].asp_list[(m3_u16)pconn->r_sp_id]) {
            return 0;
        }
        if (M3_ASP_ACTIVE == pconn->r_sp_st[(m3_u16)r_asid]) {
            pconn->r_sp_st[(m3_u16)r_asid] = M3_ASP_INACTIVE;
            psgp->r_as_inf[(m3_u16)r_asid].num_asp_active--;
            psgp->r_as_inf[(m3_u16)r_asid].num_asp_inactive++;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_ACTIVE);
            if (0 == psgp->r_as_inf[(m3_u16)r_asid].num_asp_active) {
                pconn->r_sp_g_st = M3_ASP_INACTIVE;
                m3uaNtfyRASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
            }
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPIA, M3_NULL);
        }
    }
    return 0;
}

m3_s32 m3ua_ASP_ERECVAC(m3_conn_inf_t *pconn,
                        m3_u32        ras)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_asp_inf_t    *pasp;
    m3_u32          asid;
    m3_u32          rtctx = m3uaGetRCFromRAS(pconn, ras);

    if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        return 0;
    }
    if (M3_MAX_U32 == (asid = m3uaGetASFromRC(pconn, rtctx))) {
        return 0;
    }
    pasp = M3_ASP_TABLE(pconn->l_sp_id);
    if (M3_FALSE == pasp->as_list[asid]) {
        return 0;
    }
    if (M3_ASP_ACTIVE != pconn->l_sp_st[asid]) {
        pconn->l_sp_st[asid] = M3_ASP_ACTIVE;
        m3uaNtfyASPState(pconn, asid, M3_ASP_ACTIVE);
        if (M3_ASP_ACTIVE != pconn->l_sp_g_st) {
            pconn->l_sp_g_st = M3_ASP_ACTIVE;
        }
    }
    return 0;
}

m3_s32 m3ua_ASP_ERECVIA(m3_conn_inf_t *pconn,
                        m3_u32        ras)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_asp_inf_t    *pasp;
    m3_u32          asid;
    m3_u16          idx;
    m3_u32          rtctx = m3uaGetRCFromRAS(pconn, ras);

    if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        return 0;
    }
    if (M3_MAX_U32 == (asid = m3uaGetASFromRC(pconn, rtctx))) {
        return 0;
    }
    pasp = M3_ASP_TABLE(pconn->l_sp_id);
    if (M3_FALSE == pasp->as_list[asid]) {
        return 0;
    }
    if (M3_ASP_INACTIVE != pconn->l_sp_st[asid]) {
        pconn->l_sp_st[asid] = M3_ASP_INACTIVE;
        m3uaNtfyASPState(pconn, asid, M3_ASP_INACTIVE);
    }
    if (M3_ASP_ACTIVE == pconn->l_sp_g_st) {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (M3_TRUE == pasp->as_list[idx]                        &&
                M3_ASP_ACTIVE == pconn->l_sp_st[idx]) {
                    break;
            }
        }
        if (M3_MAX_AS <= idx) {
            pconn->l_sp_g_st = M3_ASP_INACTIVE;
            m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
        }
    }
    return 0;
}

m3_s32    m3ua_ASPINACTIVE_ESENDAC(m3_conn_inf_t  *pconn,
                                   void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3ua_asp_state_t    *p_inf = (m3ua_asp_state_t*)p_edata;
    m3_aspac_inf_t      aspac;
    m3_u16              idx;
    m3_u32              msglen = 0;
    m3_u8               *p_msg;
    m3_u8               sess_id;
    m3_aspm_msg_inf_t   aspm_msg_inf;
    m3_timer_param_t    *ptimerbuf;
    m3_u16              num_ovrrd_as = 0;
    m3_u16              num_ldshr_as = 0;
    m3_u16              num_bdcst_as = 0;
    m3_u32              as_list[M3_MAX_AS];

    if(p_inf == NULL){
        iu_log_debug("p_edata is NULL\n");
    }

    /* book modify */
    for (idx = pconn->l_sp_id; idx < (p_inf->modify.info.num_as + pconn->l_sp_id); idx++) {
        if (M3_TRFMODE_OVERRIDE == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as] = p_inf->modify.info.as_list[idx];
            num_ovrrd_as++;
        }
        if (M3_TRFMODE_LOAD_SHARE == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as + num_ldshr_as] =
                p_inf->modify.info.as_list[idx];
            num_ldshr_as++;
        }
        if (M3_TRFMODE_BROADCAST == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as + num_ldshr_as + num_bdcst_as] =
                p_inf->modify.info.as_list[idx];
            num_bdcst_as++;
        }
    }
    
    /* JUNE 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        for (idx = pconn->l_sp_id; idx < (p_inf->modify.info.num_as + pconn->l_sp_id); idx++) {
            pconn->l_sp_st[p_inf->modify.info.as_list[idx]] = M3_ASP_ACSENT;
        }

        if (pconn->opt & M3_CONN_EXCL_TRFMD)
            aspac.trfmode = M3_MAX_U32;
        else if ((num_ldshr_as != 0) && (num_bdcst_as == 0) && (num_ovrrd_as == 0))
            aspac.trfmode = M3_TRFMODE_LOAD_SHARE;
        else if ((num_ovrrd_as != 0) && (num_ldshr_as == 0) && (num_bdcst_as == 0))
            aspac.trfmode = M3_TRFMODE_OVERRIDE;
        else if ((num_bdcst_as != 0) && (num_ovrrd_as == 0) && (num_ldshr_as == 0))
            aspac.trfmode = M3_TRFMODE_BROADCAST;
        else
            aspac.trfmode = M3_MAX_U32;

        aspac.num_rc = 0;
        aspac.info_len = 0;
        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
        return 0;
    }
    if (0 < num_ovrrd_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_OVERRIDE;
        aspac.num_rc = num_ovrrd_as;
        for (idx = 0; idx < num_ovrrd_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[idx]);
            pconn->l_sp_st[as_list[idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    if (0 < num_ldshr_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_LOAD_SHARE;
        aspac.num_rc = num_ldshr_as;
        for (idx = 0; idx < num_ldshr_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[num_ovrrd_as + idx]);
            pconn->l_sp_st[as_list[num_ovrrd_as + idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    if (0 < num_bdcst_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_BROADCAST;
        aspac.num_rc = num_bdcst_as;
        for (idx = 0; idx < num_bdcst_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[num_ovrrd_as+num_ldshr_as+idx]);
            pconn->l_sp_st[as_list[num_ovrrd_as + num_ldshr_as + idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    return 0;
}

m3_s32    m3ua_ASPINACTIVE_ERECVACACK(m3_conn_inf_t  *pconn,
                                      void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_aspac_ack_inf_t    *p_inf = (m3_aspac_ack_inf_t*)p_edata;
    m3_u8                 n_sess = 0;
    m3_u8                 sess_list[M3_MAX_ASPM_SESS_PER_CONN];
    m3_u16                num_rc_mm = 0;
    m3_u32                mm_rc_list[M3_MAX_RTCTX];
    m3_u16                idx, idx2;
    m3_aspac_inf_t        aspac_inf;
    m3_aspm_msg_inf_t     aspm_inf;
    m3_u32                as_id;
    m3_bool_t             chg_gstate = M3_FALSE;
    m3_u16                num_rc_m = 0;
    m3_u32                m_rc_list[M3_MAX_AS];

    if (-1 == m3uaAssertASPACACK(pconn, p_inf, &num_rc_mm, mm_rc_list)) {
        return -1;
    }
    m3uaGetSessFromMsgType(pconn, M3_MSG_TYPE_ASPAC, &n_sess, sess_list);
    M3TRACE(m3uaAspmTrace, ("No. of Sessions for ASPAC:%d",(int)n_sess));

    /* JUNE 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (pconn->l_sp_st[idx] == M3_ASP_ACSENT) {
                M3TRACE(m3uaAspmTrace, ("Making ASP:%d Active in AS:%d",(int)pconn->l_sp_id,(int)idx));
                pconn->l_sp_st[idx] = M3_ASP_ACTIVE;
                m3uaNtfyASPState(pconn, idx, M3_ASP_ACTIVE);
                chg_gstate = M3_TRUE;
                m3ua_RASP_ERECVACACK(pconn, idx);
            }
        }
        for (idx = 0; idx < n_sess; idx++) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else if (0 == p_inf->num_rc && M3_MAX_U32 == p_inf->trfmode) {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            for (idx2 = 0; idx2 < aspac_inf.num_rc; idx2++) {
                as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx2]);
                pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
                m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
                chg_gstate = M3_TRUE;
                m3ua_RASP_ERECVACACK(pconn, as_id);
            }
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else if (0 == p_inf->num_rc && M3_MAX_U32 != p_inf->trfmode) {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            if (p_inf->trfmode == aspac_inf.trfmode) {
                for (idx2 = 0; idx2 < aspac_inf.num_rc; idx2++) {
                    as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx2]);
                    pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
                    m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
                    chg_gstate = M3_TRUE;
                    m3ua_RASP_ERECVACACK(pconn, as_id);
                }
                m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
                m3uaDeleteConnSess(pconn, sess_list[idx]);
            }
        }
    } else {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            m3uaCreateCommonRCList(&p_inf->num_rc, p_inf->rc_list,
                &aspac_inf.num_rc, aspac_inf.rc_list, &num_rc_m, m_rc_list);
            if (0 == aspac_inf.num_rc) {
                m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
                m3uaDeleteConnSess(pconn, sess_list[idx]);
            } else {
                m3uaEncodeASPAC(&aspac_inf, &aspm_inf.msg_len, aspm_inf.msg);
                m3uaSetASPMMsgInf(pconn, sess_list[idx], &aspm_inf);
            }
        }
        for (idx = 0; idx < num_rc_m; idx++) {
            as_id = m3uaGetASFromRC(pconn, m_rc_list[idx]);
            pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
            m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
            chg_gstate = M3_TRUE;
            m3ua_RASP_ERECVACACK(pconn, as_id);
        }
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            mm_rc_list[num_rc_mm] = p_inf->rc_list[idx];
            num_rc_mm = num_rc_mm + 1;
        }
    }

    if (M3_TRUE == chg_gstate) {
        pconn->l_sp_g_st = M3_ASP_ACTIVE;
    }
    return 0;
}

m3_s32    m3ua_ASPINACTIVE_ETIMEREXP(m3_conn_inf_t  *pconn,
                                     void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_timer_inf_t        *p_inf = (m3_timer_inf_t*)p_edata;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)p_inf->p_buf;
    m3_u8                 sess_id = ptimerbuf->param.aspmbuf.sess_id;
    m3_aspac_inf_t        aspac_inf;
    m3_aspm_msg_inf_t     aspm_inf;
    m3_u32                interval = M3_ASPM_TIMER_INT_LOW;
    m3_u32                as_id;
    m3_u16                idx;

    if (M3_ASPM_RETRY_LOW <= pconn->aspm_sess_inf[sess_id].num_retry) {
        interval = M3_ASPM_TIMER_INT_HIGH;
    }
    if (M3_ASPM_MAX_RETRY <= pconn->aspm_sess_inf[sess_id].num_retry ||
        -1 == m3uaStartASPMTimer(pconn, sess_id, ptimerbuf, interval)) {

        aspm_inf = pconn->aspm_sess_inf[sess_id].msg_inf;

        /* JULY 2010 */
        if ((M3_MSG_TYPE_ASPAC == aspm_inf.msg_type) && (pconn->opt & M3_CONN_EXCL_RTCTX)) {
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                if (pconn->l_sp_st[idx] == M3_ASP_ACSENT) {
                    M3TRACE(m3uaAspmTrace, ("ASP:%d AS:%d - ASPAC timer expiry",(int)pconn->l_sp_id,(int)idx));
                    pconn->l_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyASPState(pconn, idx, M3_ASP_INACTIVE);
                }
            }
        } else if (M3_MSG_TYPE_ASPAC == aspm_inf.msg_type) {
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            for (idx = 0; idx < aspac_inf.num_rc; idx++) {
                as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx]);
                pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
                /* indicate to SM new state of ASP */
                m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
            }
        } else if (M3_MSG_TYPE_ASPDN == aspm_inf.msg_type) {
            pconn->l_sp_g_st = M3_ASP_DOWN;
            m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_DOWN); 
        }
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL);
        m3uaDeleteConnSess(pconn, sess_id);
    } else {
        aspm_inf = pconn->aspm_sess_inf[sess_id].msg_inf;
        m3uaSendMsg(pconn, 0, aspm_inf.msg_len, aspm_inf.msg);
        pconn->aspm_sess_inf[sess_id].num_retry++;
        //pconn->aspm_sess_inf[sess_id].timer_inf = *p_inf;
    }
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ERECVIAACK(m3_conn_inf_t  *pconn,
                                    void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_aspia_ack_inf_t    *p_inf = (m3_aspia_ack_inf_t*)p_edata;
    m3_u8                 n_sess = 0;
    m3_u8                 sess_list[M3_MAX_ASPM_SESS_PER_CONN];
    m3_u16                num_rc_mm = 0;
    m3_u32                mm_rc_list[M3_MAX_AS];
    m3_u16                idx, idx2;
    m3_aspia_inf_t        aspia_inf;
    m3_aspm_msg_inf_t     aspm_inf;
    m3_u32                as_id;
    m3_u16                num_rc_m = 0;
    m3_u32                m_rc_list[M3_MAX_AS];

    if (-1 == m3uaAssertASPIAACK(pconn, p_inf, &num_rc_mm, mm_rc_list)) {
        return -1;
    }
    m3uaGetSessFromMsgType(pconn, M3_MSG_TYPE_ASPIA, &n_sess, sess_list);
    M3TRACE(m3uaAspmTrace, ("No. of Sessions for ASPIA:%d",(int)n_sess));

    /* JULY 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (pconn->l_sp_st[idx] == M3_ASP_IASENT) {
                M3TRACE(m3uaAspmTrace, ("Making ASP:%d InActive in AS:%d",(int)pconn->l_sp_id,(int)idx));
                pconn->l_sp_st[idx] = M3_ASP_INACTIVE;
                m3uaNtfyASPState(pconn, idx, M3_ASP_INACTIVE);
                m3ua_RASP_ERECVIAACK(pconn, idx);
            }
        }
        for (idx = 0; idx < n_sess; idx++) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else if (0 == p_inf->num_rc) {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPIA(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspia_inf);
            for (idx2 = 0; idx2 < aspia_inf.num_rc; idx2++) {
                as_id = m3uaGetASFromRC(pconn, aspia_inf.rc_list[idx2]);
                pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
                m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
                m3ua_RASP_ERECVIAACK(pconn, as_id);
            }
        }
        /* JULY 2010 */
        for (idx = 0; idx < n_sess; idx++) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPIA(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspia_inf);
            m3uaCreateCommonRCList(&p_inf->num_rc, p_inf->rc_list,
                &aspia_inf.num_rc, aspia_inf.rc_list, &num_rc_m, m_rc_list);
            if (0 == aspia_inf.num_rc) {
                m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
                m3uaDeleteConnSess(pconn, sess_list[idx]);
            } else {
                m3uaEncodeASPIA(&aspia_inf, &aspm_inf.msg_len, aspm_inf.msg);
                m3uaSetASPMMsgInf(pconn, sess_list[idx], &aspm_inf);
            }
        }
        for (idx = 0; idx < num_rc_m; idx++) {
            as_id = m3uaGetASFromRC(pconn, m_rc_list[idx]);
            pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
            m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
            m3ua_RASP_ERECVIAACK(pconn, as_id);
        }
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            mm_rc_list[num_rc_mm] = p_inf->rc_list[idx];
            num_rc_mm = num_rc_mm + 1;
        }
    }
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        if (M3_TRUE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[idx]    &&
            M3_ASP_ACTIVE == pconn->l_sp_st[idx]) {
                break;
        }
    }
    if (M3_MAX_AS <= idx) {
        pconn->l_sp_g_st = M3_ASP_INACTIVE;
        m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
    }
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ETIMEREXP(m3_conn_inf_t  *pconn,
                                   void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_timer_inf_t        *p_inf = (m3_timer_inf_t*)p_edata;
    m3_timer_param_t      *ptimerbuf = (m3_timer_param_t*)p_inf->p_buf;
    m3_u8                 sess_id = ptimerbuf->param.aspmbuf.sess_id;
    m3_aspac_inf_t        aspac_inf;
    m3_aspia_inf_t        aspia_inf;
    m3_aspm_msg_inf_t     aspm_inf;
    m3_u16                idx, idx2;
    m3_u32                interval = M3_ASPM_TIMER_INT_LOW;
    m3_u32                as_id;

    if (M3_ASPM_RETRY_LOW <= pconn->aspm_sess_inf[sess_id].num_retry) {
        interval = M3_ASPM_TIMER_INT_HIGH;
    }
    if (M3_ASPM_MAX_RETRY == pconn->aspm_sess_inf[sess_id].num_retry    ||
        -1 == m3uaStartASPMTimer(pconn, sess_id, ptimerbuf, interval)) {
        aspm_inf = pconn->aspm_sess_inf[sess_id].msg_inf;

        if ((M3_MSG_TYPE_ASPAC == aspm_inf.msg_type) && (pconn->opt & M3_CONN_EXCL_RTCTX)) {
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                if (pconn->l_sp_st[idx] == M3_ASP_ACSENT) {
                    M3TRACE(m3uaAspmTrace, ("ASP:%d AS:%d - ASPAC timer expiry",(int)pconn->l_sp_id,(int)idx));
                    pconn->l_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyASPState(pconn, idx, M3_ASP_INACTIVE);
                }
            }
        } else if ((M3_MSG_TYPE_ASPIA == aspm_inf.msg_type) && (pconn->opt & M3_CONN_EXCL_RTCTX)) {
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                if (pconn->l_sp_st[idx] == M3_ASP_IASENT) {
                    M3TRACE(m3uaAspmTrace, ("ASP:%d AS:%d - ASPIA timer expiry",(int)pconn->l_sp_id,(int)idx));
                    pconn->l_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyASPState(pconn, idx, M3_ASP_INACTIVE);
                }
            }
        } else if (M3_MSG_TYPE_ASPAC == aspm_inf.msg_type) {
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            for (idx = 0; idx < aspac_inf.num_rc; idx++) {
                as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx2]);
                pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
                /* indicate to SM new state of ASP */
                m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
            }
        } else if (M3_MSG_TYPE_ASPIA == aspm_inf.msg_type) {
            m3uaDecodeASPIA(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspia_inf);
            for (idx = 0; idx < aspia_inf.num_rc; idx++) {
                as_id = m3uaGetASFromRC(pconn, aspia_inf.rc_list[idx2]);
                pconn->l_sp_st[as_id] = M3_ASP_INACTIVE;
                m3uaNtfyASPState(pconn, as_id, M3_ASP_INACTIVE);
            }
            for (idx = 0; idx < M3_MAX_AS; idx++) {
                if (M3_TRUE == (M3_ASP_TABLE(pconn->l_sp_id))->as_list[idx]  &&
                    M3_ASP_ACTIVE == pconn->l_sp_st[idx]) {
                    break;
                }
            }
            if (M3_MAX_AS <= idx) {
                pconn->l_sp_g_st = M3_ASP_INACTIVE;
                m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
            }
        } else if (M3_MSG_TYPE_ASPDN == aspm_inf.msg_type) {
            pconn->l_sp_g_st = M3_ASP_DOWN;
            m3uaNtfyASPState(pconn, M3_MAX_U32, M3_ASP_DOWN);
        }
        M3_MSG_FREE(ptimerbuf, M3_TIMER_POOL); /* JULY 2010 */
        m3uaDeleteConnSess(pconn, sess_id);
    } else {
        aspm_inf = pconn->aspm_sess_inf[sess_id].msg_inf;
        m3uaSendMsg(pconn, 0, aspm_inf.msg_len, aspm_inf.msg);
        pconn->aspm_sess_inf[sess_id].num_retry++;
        //pconn->aspm_sess_inf[sess_id].timer_inf = *p_inf;
    }
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ESENDIA(m3_conn_inf_t  *pconn,
                                 void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3ua_asp_state_t    *p_inf = (m3ua_asp_state_t*)p_edata;
    m3_aspia_inf_t      aspia;
    m3_u16              idx;
    m3_u32              msglen = 0;
    m3_u8               *p_msg;
    m3_u8               sess_id;
    m3_aspm_msg_inf_t   aspm_msg_inf;
    m3_timer_param_t    *ptimerbuf;

    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPIA_SIZE, M3_ASPM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), 
        M3_TIMER_POOL);
    if (M3_NULL == ptimerbuf) {
        M3_MSG_FREE(p_msg, M3_ASPM_POOL);
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    /* book modify, 2011-12-30 */
    aspia.num_rc = p_inf->modify.info.num_as + pconn->l_sp_id;
    iu_log_debug("aspia.num_rc = %d",aspia.num_rc);
    for (idx = 0; idx < aspia.num_rc; idx++) {
        iu_log_debug("p_inf->modify.info.as_list[idx] = %d",p_inf->modify.info.as_list[idx]);
        /* book add for avoid Segmentation fault, 2011-12-30 */
        if((p_inf->modify.info.as_list[idx]>=M3_MAX_AS))
        {
            continue;
        }
        pconn->l_sp_st[p_inf->modify.info.as_list[idx]] = M3_ASP_IASENT;
        aspia.rc_list[idx]= m3uaGetRCFromAS(pconn, p_inf->modify.info.as_list[idx]);
    }
    aspia.info_len = 0;
    /* JUNE 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        aspia.num_rc = 0;
    }
    m3uaEncodeASPIA(&aspia, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    m3uaGetFreeSess(pconn, &sess_id);
    aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPIA;
    aspm_msg_inf.msg_len      = msglen;
    aspm_msg_inf.msg          = p_msg;
    m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
    ptimerbuf->type = M3_TIMER_TYPE_ASPM;
    ptimerbuf->param.aspmbuf.sess_id = sess_id;
    ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
    m3uaStartASPMTimer(pconn, sess_id, ptimerbuf, M3_ASPM_TIMER_INT_LOW);
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ESENDAC(m3_conn_inf_t  *pconn,
                                 void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3ua_asp_state_t    *p_inf = (m3ua_asp_state_t*)p_edata;
    m3_aspac_inf_t      aspac;
    m3_u16              idx;
    m3_u32              msglen = 0;
    m3_u8               *p_msg;
    m3_u8               sess_id;
    m3_aspm_msg_inf_t   aspm_msg_inf;
    m3_timer_param_t    *ptimerbuf;
    m3_u16              num_ovrrd_as = 0;
    m3_u16              num_ldshr_as = 0;
    m3_u16              num_bdcst_as = 0;
    m3_u32              as_list[M3_MAX_AS];

    for (idx = 0; idx < p_inf->modify.info.num_as; idx++) {
        if (M3_TRFMODE_OVERRIDE == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as] = p_inf->modify.info.as_list[idx];
            num_ovrrd_as++;
        }
    }
    for (idx = 0; idx < p_inf->modify.info.num_as; idx++) {
        if (M3_TRFMODE_LOAD_SHARE == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as + num_ldshr_as] =
                p_inf->modify.info.as_list[idx];
            num_ldshr_as++;
        }
    }
    for (idx = 0; idx < p_inf->modify.info.num_as; idx++) {
        if (M3_TRFMODE_BROADCAST == (M3_AS_TABLE(p_inf->modify.info.\
            as_list[idx]))->rkey.trfmode) {
            as_list[num_ovrrd_as + num_ldshr_as + num_bdcst_as] =
                p_inf->modify.info.as_list[idx];
            num_bdcst_as++;
        }
    }

    /* JULY 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t),
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        for (idx = 0; idx < p_inf->modify.info.num_as; idx++) {
            pconn->l_sp_st[p_inf->modify.info.as_list[idx]] = M3_ASP_ACSENT;
        }

        if (pconn->opt & M3_CONN_EXCL_TRFMD)
            aspac.trfmode = M3_MAX_U32;
        else if ((num_ldshr_as != 0) && (num_bdcst_as == 0) && (num_ovrrd_as == 0))
            aspac.trfmode = M3_TRFMODE_LOAD_SHARE;
        else if ((num_ovrrd_as != 0) && (num_ldshr_as == 0) && (num_bdcst_as == 0))
            aspac.trfmode = M3_TRFMODE_OVERRIDE;
        else if ((num_bdcst_as != 0) && (num_ovrrd_as == 0) && (num_ldshr_as == 0))
            aspac.trfmode = M3_TRFMODE_BROADCAST;
        else
            aspac.trfmode = M3_MAX_U32;

        aspac.num_rc = 0;
        aspac.info_len = 0;
        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
        return 0;
    }

    if (0 < num_ovrrd_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), 
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_OVERRIDE;
        aspac.num_rc = num_ovrrd_as;
        for (idx = 0; idx < num_ovrrd_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[idx]);
            pconn->l_sp_st[as_list[idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_RTCTX) {
            aspac.num_rc = 0;
        }
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    if (0 < num_ldshr_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), 
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_LOAD_SHARE;
        aspac.num_rc = num_ldshr_as;
        for (idx = 0; idx < num_ldshr_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[num_ovrrd_as + idx]);
            pconn->l_sp_st[as_list[num_ovrrd_as + idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_RTCTX) {
            aspac.num_rc = 0;
        }
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    if (0 < num_bdcst_as) {
        p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPAC_SIZE, M3_ASPM_POOL);
        if (M3_NULL == p_msg) {
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        ptimerbuf = (m3_timer_param_t*)M3_MSG_ALLOC(sizeof(m3_timer_param_t), 
            M3_TIMER_POOL);
        if (M3_NULL == ptimerbuf) {
            M3_MSG_FREE(p_msg, M3_ASPM_POOL);
            M3ERRNO(EM3_MALLOC_FAIL);
            return -1;
        }
        aspac.trfmode = M3_TRFMODE_BROADCAST;
        aspac.num_rc = num_bdcst_as;
        for (idx = 0; idx < num_bdcst_as; idx++) {
            aspac.rc_list[idx] = m3uaGetRCFromAS(pconn, as_list[num_ovrrd_as+num_ldshr_as+idx]);
            pconn->l_sp_st[as_list[num_ovrrd_as + num_ldshr_as + idx]] = M3_ASP_ACSENT;
        }
        aspac.info_len = 0;

        /* JUNE 2010 */
        if (pconn->opt & M3_CONN_EXCL_RTCTX) {
            aspac.num_rc = 0;
        }
        if (pconn->opt & M3_CONN_EXCL_TRFMD) {
            aspac.trfmode = M3_MAX_U32;
        }

        m3uaEncodeASPAC(&aspac, &msglen, p_msg);
        m3uaSendMsg(pconn, 0, msglen, p_msg);
        m3uaGetFreeSess(pconn, &sess_id);
        aspm_msg_inf.msg_type     = M3_MSG_TYPE_ASPAC;
        aspm_msg_inf.msg_len      = msglen;
        aspm_msg_inf.msg          = p_msg;
        m3uaSetASPMMsgInf(pconn, sess_id, &aspm_msg_inf);
        ptimerbuf->type = M3_TIMER_TYPE_ASPM;
        ptimerbuf->param.aspmbuf.sess_id = sess_id;
        ptimerbuf->param.aspmbuf.conn_id = pconn->conn_id;
        m3uaStartASPMTimer(pconn,sess_id,ptimerbuf,M3_ASPM_TIMER_INT_LOW);
    }
    return 0;
}

m3_s32    m3ua_ASPACTIVE_ERECVACACK(m3_conn_inf_t  *pconn,
                                    void           *p_edata)
{
    iu_log_debug("--------------- %s %d %s ------------\n",__FILE__, __LINE__, __func__);
    iu_log_debug("pConn->conn_id = %d  pConn->assoc_id = %d\n", pconn->conn_id, pconn->assoc_id);
    
    m3_aspac_ack_inf_t    *p_inf = (m3_aspac_ack_inf_t*)p_edata;
    m3_u8                 n_sess = 0;
    m3_u8                 sess_list[M3_MAX_ASPM_SESS_PER_CONN];
    m3_u16                num_rc_mm = 0;
    m3_u32                mm_rc_list[M3_MAX_AS];
    m3_u16                idx, idx2;
    m3_aspac_inf_t        aspac_inf;
    m3_aspm_msg_inf_t     aspm_inf;
    m3_u32                as_id;
    m3_u16                num_rc_m = 0;
    m3_u32                m_rc_list[M3_MAX_AS];

    if (-1 == m3uaAssertASPACACK(pconn,p_inf,&num_rc_mm,mm_rc_list)) {
        return -1;
    }
    m3uaGetSessFromMsgType(pconn, M3_MSG_TYPE_ASPAC, &n_sess, sess_list);
    M3TRACE(m3uaAspmTrace, ("No. of Sessions for ASPAC:%d",(int)n_sess));

    /* JULY 2010 */
    if (pconn->opt & M3_CONN_EXCL_RTCTX) {
        for (idx = 0; idx < M3_MAX_AS; idx++) {
            if (pconn->l_sp_st[idx] == M3_ASP_ACSENT) {
                M3TRACE(m3uaAspmTrace, ("Making ASP:%d Active in AS:%d",(int)pconn->l_sp_id,(int)idx));
                pconn->l_sp_st[idx] = M3_ASP_ACTIVE;
                m3uaNtfyASPState(pconn, idx, M3_ASP_ACTIVE);
                m3ua_RASP_ERECVACACK(pconn, idx);
            }
        }
        for (idx = 0; idx < n_sess; idx++) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else if (0 == p_inf->num_rc && M3_MAX_U32 == p_inf->trfmode) {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            for (idx2 = 0; idx2 < aspac_inf.num_rc; idx2++) {
                as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx2]);
                pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
                m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
                m3ua_RASP_ERECVACACK(pconn, as_id);
            }
        }
        for (idx = 0; idx < n_sess; idx++) {
            m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
            m3uaDeleteConnSess(pconn, sess_list[idx]);
        }
    } else if (0 == p_inf->num_rc && M3_MAX_U32 != p_inf->trfmode) {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            if (p_inf->trfmode == aspac_inf.trfmode) {
                for (idx2 = 0; idx2 < aspac_inf.num_rc; idx2++) {
                    as_id = m3uaGetASFromRC(pconn, aspac_inf.rc_list[idx2]);
                    pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
                    m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
                    m3ua_RASP_ERECVACACK(pconn, as_id);
                }
                /* JULY 2010 */
                m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
                m3uaDeleteConnSess(pconn, sess_list[idx]);
            }
        }
    } else {
        for (idx = 0; idx < n_sess; idx++) {
            aspm_inf = pconn->aspm_sess_inf[sess_list[idx]].msg_inf;
            m3uaDecodeASPAC(aspm_inf.msg_len-8, aspm_inf.msg+8, &aspac_inf);
            m3uaCreateCommonRCList(&p_inf->num_rc, p_inf->rc_list,
                &aspac_inf.num_rc, aspac_inf.rc_list, &num_rc_m, m_rc_list);
            if (0 == aspac_inf.num_rc) {
                m3uaFreeTimer(&pconn->aspm_sess_inf[sess_list[idx]].timer_inf);
                m3uaDeleteConnSess(pconn, sess_list[idx]);
            } else {
                m3uaEncodeASPAC(&aspac_inf, &aspm_inf.msg_len, aspm_inf.msg);
                m3uaSetASPMMsgInf(pconn, sess_list[idx], &aspm_inf);
            }
        }
        for (idx = 0; idx < num_rc_m; idx++) {
            as_id = m3uaGetASFromRC(pconn, m_rc_list[idx]);
            pconn->l_sp_st[as_id] = M3_ASP_ACTIVE;
            /* indicate to SM new state of ASP */
            m3uaNtfyASPState(pconn, as_id, M3_ASP_ACTIVE);
            m3ua_RASP_ERECVACACK(pconn, as_id);
        }
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            mm_rc_list[num_rc_mm] = p_inf->rc_list[idx];
            num_rc_mm = num_rc_mm + 1;
        }
    }
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ERECVAC(m3_conn_inf_t  *pconn,
                                    void           *p_edata)
{
    m3_aspac_inf_t        *p_inf = (m3_aspac_inf_t*)p_edata;
    m3_u16                idx;
    m3_aspac_ack_inf_t    aspac_ack;
    m3_bool_t             chg_gstate = M3_FALSE;
    m3_u32                r_asid;
    m3_u32                msglen = 0;
    m3_u8                 *p_msg;
    m3_asp_inf_t          *pasp;
    m3_sgp_inf_t          *psgp;

    if (-1 == m3uaAssertASPAC(pconn, p_inf)) {
        return -1;
    }
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPACACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    } 
    aspac_ack.info_len = 0;
    aspac_ack.trfmode  = p_inf->trfmode;
    aspac_ack.num_rc   = p_inf->num_rc;
    for (idx = 0; idx < p_inf->num_rc; idx++)
        aspac_ack.rc_list[idx] = p_inf->rc_list[idx];
    m3uaEncodeASPACACK(&aspac_ack, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_ASPM_POOL);
    if (0 < p_inf->num_rc) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            r_asid = m3uaGetRASFromRC(pconn, p_inf->rc_list[idx]);
            pconn->r_sp_st[r_asid] = M3_ASP_ACTIVE;
            chg_gstate = M3_TRUE;
            /* indicate to SM new state of ASP in AS */
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_ACTIVE);
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPAC, M3_NULL);
            m3ua_ASP_ERECVAC(pconn, r_asid);
        }
    } else {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE == 
                    pasp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    (M3_MAX_U32 == p_inf->trfmode                      ||
                     (M3_R_AS_TABLE(idx))->rkey.trfmode == p_inf->trfmode)) {
                    pconn->r_sp_st[idx] = M3_ASP_ACTIVE;
                    chg_gstate = M3_TRUE;
                    /* indicate to SM new state of ASP in AS */
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_ACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPAC, M3_NULL);
                    m3ua_ASP_ERECVAC(pconn, idx);
                }
            }
        } else {
            psgp = M3_SGP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE ==
                    psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    (M3_MAX_U32 == p_inf->trfmode                      ||
                     (M3_R_AS_TABLE(idx))->rkey.trfmode == p_inf->trfmode)) {
                    pconn->r_sp_st[idx] = M3_ASP_ACTIVE;
                    chg_gstate = M3_TRUE;
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_ACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPAC, M3_NULL);
                    m3ua_ASP_ERECVAC(pconn, idx);
                }
            }
        }
    }
    if (M3_TRUE == chg_gstate) {
        pconn->r_sp_g_st = M3_ASP_ACTIVE;
    }
    return 0;
}

m3_s32    m3ua_RASPINACTIVE_ERECVIA(m3_conn_inf_t  *pconn,
                                    void           *p_edata)
{
    m3_aspia_inf_t        *p_inf = (m3_aspia_inf_t*)p_edata;
    m3_u16                idx;
    m3_aspia_ack_inf_t    aspia_ack;
    m3_u32                msglen = 0;
    m3_u8                 *p_msg;

    if (-1 == m3uaAssertASPIA(pconn, p_inf)) {
        return -1;
    }
    aspia_ack.num_rc   = p_inf->num_rc;
    aspia_ack.info_len = 0;
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPIAACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    for (idx = 0; idx < p_inf->num_rc; idx++) {
        aspia_ack.rc_list[idx] = p_inf->rc_list[idx];
    }
    m3uaEncodeASPIAACK(&aspia_ack, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_ASPM_POOL);
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ERECVAC(m3_conn_inf_t  *pconn,
                                  void           *p_edata)
{
    m3_aspac_inf_t        *p_inf = (m3_aspac_inf_t*)p_edata;
    m3_u16                idx;
    m3_aspac_ack_inf_t    aspac_ack;
    m3_u32                r_asid;
    m3_u32                msglen = 0;
    m3_u8                 *p_msg;
    m3_asp_inf_t          *pasp;
    m3_sgp_inf_t          *psgp;

    if (-1 == m3uaAssertASPAC(pconn, p_inf)) {
        return -1;
    }
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPACACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspac_ack.trfmode  = p_inf->trfmode;
    aspac_ack.num_rc   = p_inf->num_rc;
    aspac_ack.info_len = 0;
    for (idx = 0; idx < p_inf->num_rc; idx++)
        aspac_ack.rc_list[idx] = p_inf->rc_list[idx];
    m3uaEncodeASPACACK(&aspac_ack, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_ASPM_POOL);

    if (0 < p_inf->num_rc) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            r_asid = m3uaGetRASFromRC(pconn, p_inf->rc_list[idx]);
            if (M3_ASP_ACTIVE == pconn->r_sp_st[r_asid]) {
                continue;
            }
            pconn->r_sp_st[r_asid] = M3_ASP_ACTIVE;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_ACTIVE);
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPAC, M3_NULL);
            m3ua_ASP_ERECVAC(pconn, r_asid);
        }
    } else {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE ==
                    pasp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    (M3_MAX_U32 == p_inf->trfmode                      ||
                     (M3_R_AS_TABLE(idx))->rkey.trfmode == p_inf->trfmode)) {
                    pconn->r_sp_st[idx] = M3_ASP_ACTIVE;
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_ACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPAC, M3_NULL);
                    m3ua_ASP_ERECVAC(pconn, idx);
                }
            }
        } else {
            psgp = M3_SGP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE ==
                    psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    (M3_MAX_U32 == p_inf->trfmode                      ||
                     (M3_R_AS_TABLE(idx))->rkey.trfmode == p_inf->trfmode)) {
                    pconn->r_sp_st[idx] = M3_ASP_ACTIVE;
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_ACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPAC, M3_NULL);
                    m3ua_ASP_ERECVAC(pconn, idx);
                }
            }
        }
    }
    return 0;
}

m3_s32    m3ua_RASPACTIVE_ERECVIA(m3_conn_inf_t  *pconn,
                                  void           *p_edata)
{
    m3_aspia_inf_t        *p_inf = (m3_aspia_inf_t*)p_edata;
    m3_u16                idx;
    m3_aspia_ack_inf_t    aspia_ack;
    m3_u32                r_asid;
    m3_u32                msglen = 0;
    m3_u8                 *p_msg;
    m3_asp_inf_t          *pasp;
    m3_sgp_inf_t          *psgp;

    if (-1 == m3uaAssertASPIA(pconn, p_inf)) {
        return -1;
    }
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_ASPIAACK_SIZE, M3_ASPM_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    aspia_ack.info_len = 0;
    aspia_ack.num_rc   = p_inf->num_rc;
    for (idx = 0; idx < p_inf->num_rc; idx++)
        aspia_ack.rc_list[idx] = p_inf->rc_list[idx];
    m3uaEncodeASPIAACK(&aspia_ack, &msglen, p_msg);
    m3uaSendMsg(pconn, 0, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_ASPM_POOL);
    if (0 == p_inf->num_rc) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            pasp = M3_ASP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE ==
                    pasp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[idx]) {
                    pconn->r_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_INACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPIA, M3_NULL);
                    m3ua_ASP_ERECVIA(pconn, idx);
                }
            }
        } else if (M3_TRUE == M3_IS_SP_SGP(pconn->l_sp_id)) {
            psgp = M3_SGP_TABLE(pconn->l_sp_id);
            for (idx = 0; idx < M3_MAX_R_AS; idx++) {
                if (M3_TRUE ==
                    psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id]   &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[idx]) {
                    pconn->r_sp_st[idx] = M3_ASP_INACTIVE;
                    m3uaNtfyRASPState(pconn, idx, M3_ASP_INACTIVE);
                    m3uaRAS(pconn, idx, M3_AS_EVT_ASPIA, M3_NULL);
                    m3ua_ASP_ERECVIA(pconn, idx);
                }
            }
        }
    } else if (0 < p_inf->num_rc) {
        for (idx = 0; idx < p_inf->num_rc; idx++) {
            r_asid = m3uaGetRASFromRC(pconn, p_inf->rc_list[idx]);
            pconn->r_sp_st[r_asid] = M3_ASP_INACTIVE;
            m3uaNtfyRASPState(pconn, r_asid, M3_ASP_INACTIVE);
            m3uaRAS(pconn, r_asid, M3_AS_EVT_ASPIA, M3_NULL);
            m3ua_ASP_ERECVIA(pconn, r_asid);
        }
    }
    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        pasp = M3_ASP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE == pasp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id])
                if (M3_ASP_ACTIVE == pconn->r_sp_st[idx])
                    break;
        }
    } else {
        psgp = M3_SGP_TABLE(pconn->l_sp_id);
        for (idx = 0; idx < M3_MAX_R_AS; idx++) {
            if (M3_TRUE == psgp->r_as_inf[idx].asp_list[(m3_u16)pconn->r_sp_id])
                if (M3_ASP_ACTIVE == pconn->r_sp_st[idx])
                    break;
        }
    }
    if (M3_MAX_R_AS <= idx) {
        pconn->r_sp_g_st = M3_ASP_INACTIVE;
        m3uaNtfyRASPState(pconn, M3_MAX_U32, M3_ASP_INACTIVE);
    }
    return 0;
}

