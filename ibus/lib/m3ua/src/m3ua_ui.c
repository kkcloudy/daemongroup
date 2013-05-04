/****************************************************************************
** Description:
** Code for User APIs is present in this file.
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

#include <m3ua_ui.h>

m3_s32 m3ua_pause(m3_u16        usrId,
                  m3ua_pause_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32           ret = 0;
    m3_u32           spId;
    m3_u16           idx;
    m3_conn_inf_t    *pConn;

    if (M3_FALSE == m3uaAssertPause(usrId, pInf)) {
        ret = -1;
    }
    else {
        spId = (M3_USR_TABLE(usrId))->sp_id;
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if ((M3_TRUE == (M3_R_ASP_TABLE(idx))->e_state) &&
                (M3_MAX_U32 != (M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id)) {
                pConn = M3_CONN_TABLE((M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == pConn->r_sp_g_st) {
                    m3ua_EPAUSE(pConn, pInf);
                }
            }
        }
    }
    return ret;
}

m3_s32    m3ua_resume(m3_u16        usrId,
                      m3ua_resume_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32           ret = 0;
    m3_u32           spId;
    m3_u16           idx;
    m3_conn_inf_t    *pConn;

    if (M3_FALSE == m3uaAssertResume(usrId, pInf)) {
        ret = -1;
    }
    else
    {
        spId = (M3_USR_TABLE(usrId))->sp_id;
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if ((M3_TRUE == (M3_R_ASP_TABLE(idx))->e_state) &&
                (M3_MAX_U32 != (M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id)) {
                pConn = M3_CONN_TABLE((M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == pConn->r_sp_g_st) {
                    m3ua_ERESUME(pConn, pInf);
                }
            }
        }
    }
    return ret;
}

m3_s32    m3ua_status(m3_u16        usrId,
                      m3ua_status_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32        ret = 0;
    m3_u32        spId;
    m3_u16        idx;
    m3_conn_inf_t *pConn;

    if (M3_FALSE == m3uaAssertStatus(usrId, pInf)) {
        ret = -1;
    }
    else {
        spId = (M3_USR_TABLE(usrId))->sp_id;
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if ((M3_TRUE == (M3_R_ASP_TABLE(idx))->e_state) &&
                (M3_MAX_U32 != (M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id)){
                pConn = M3_CONN_TABLE((M3_SGP_TABLE(spId))->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == pConn->r_sp_g_st) {
                    m3ua_ESTATUS(pConn, pInf);
                }
            }
        }
    }
    return ret;
}

m3_s32    m3ua_restart(m3_u16         usrId, /* not used */
                       m3ua_restart_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32       ret = 0;

    if (M3_FALSE == m3uaAssertRestart(usrId, pInf)) {
        ret = -1;
    }
    else {
        m3uaSetRestartState(pInf->nw_app, pInf->status);
    }
    return ret;
}

m3_s32    m3ua_transfer(m3_u16        usrId,
                        m3ua_txr_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32       ret = 0;
    m3_usr_inf_t      *pUsr;

    if (M3_FALSE == m3uaAssertTransfer(usrId, pInf)) {
        iu_log_debug("Error1:  %s  %d: %s\n",__FILE__, __LINE__, __func__);
        ret = -1;
    }
    else {
        iu_log_debug("userId = %d\n",usrId);
        pUsr = M3_USR_TABLE(usrId);
        ret = m3ua_ETXR(pUsr, pInf);
    }
    return ret;
}

m3_s32    m3ua_audit(m3_u16           usrId,
                     m3ua_audit_t     *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32          ret = 0;
    m3_asp_inf_t    *pasp;
    m3_u16          idx;

    if (M3_FALSE == m3uaAssertAudit(usrId, pInf)) {
        ret = -1;
    }
    else {
        pasp = M3_ASP_TABLE((M3_USR_TABLE(usrId))->sp_id);
        for (idx = 0; idx < M3_MAX_SG; idx++) {
            if (M3_TRUE == m3uaAssertASPActiveInSG(pasp, idx)) {
                if (-1 == m3uaSendDAUD(pasp, 0xFFFF0000 | idx, pInf)) {
                    ret = -1;
                    break;
                }
            }
        }
    }
    return ret;
}

m3_s32    m3ua_user_ntfy(m3ua_user_ntfy_t    *pInf)
{
    //iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_user_ntfy_inf_t    *pList = M3_USER_NTFY_TABLE;

    if (0 == pList->n_ntfy_pd) {
        M3_ASSIGN_ERRNO(EM3_NO_USER_NTFY_PD);
        return -1;
    }
    *pInf = pList->ntfy_list[pList->c_offset];
    pList->n_ntfy_pd--;
    pList->c_offset++;
    if (M3_MAX_USER_NTFY == pList->c_offset) {
        pList->c_offset = 0;
    }
    return 0;
}

m3_bool_t    m3uaAssertPause(m3_u16        usrId,
                             m3ua_pause_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state   ||
        M3_TRUE == M3_IS_SP_ASP((M3_USR_TABLE(usrId))->sp_id)) {
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    if (0 == pInf->num_pc ||
        M3_MAX_PC_SSNM < pInf->num_pc)
    {
        M3ERRNO(EM3_INV_NUMPC);
        return M3_FALSE;
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->nw_app))
    {
        M3ERRNO(EM3_INV_NWAPP);
        return M3_FALSE;
    }
/*JAN2010
    if (M3_MAX_INFO_STR_LEN < pInf->info_len)
    {
        M3ERRNO(EM3_INV_INFOLEN);
        return M3_FALSE;
    }
********/
    if (0 != pInf->info_len && M3_NULL == pInf->p_info_str)
    {
        M3ERRNO(EM3_NULL_PARAM);
        return M3_FALSE;
    }
    return M3_TRUE;
}

m3_bool_t    m3uaAssertAudit(m3_u16        usrId,
                             m3ua_audit_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state   ||
        M3_TRUE == M3_IS_SP_SGP((M3_USR_TABLE(usrId))->sp_id)) {
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    if (0 == pInf->num_pc ||
        M3_MAX_PC_SSNM < pInf->num_pc)
    {
        M3ERRNO(EM3_INV_NUMPC);
        return M3_FALSE;
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->nw_app))
    {
        M3ERRNO(EM3_INV_NWAPP);
        return M3_FALSE;
    }
/*JAN2010
    if (M3_MAX_INFO_STR_LEN < pInf->info_len)
    {
        M3ERRNO(EM3_INV_INFOLEN);
        return M3_FALSE;
    }
********/
    if (0 != pInf->info_len && M3_NULL == pInf->p_info_str)
    {
        M3ERRNO(EM3_NULL_PARAM);
        return M3_FALSE;
    }
    return M3_TRUE;
}

m3_bool_t    m3uaAssertResume(m3_u16        usrId,
                              m3ua_resume_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state   ||
        M3_TRUE == M3_IS_SP_ASP((M3_USR_TABLE(usrId))->sp_id)) {
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    if (0 == pInf->num_pc ||
        M3_MAX_PC_SSNM < pInf->num_pc)
    {
        M3ERRNO(EM3_INV_NUMPC);
        return M3_FALSE;
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->nw_app))
    {
        M3ERRNO(EM3_INV_NWAPP);
        return M3_FALSE;
    }
/*JAN2010
    if (M3_MAX_INFO_STR_LEN < pInf->info_len)
    {
        M3ERRNO(EM3_INV_INFOLEN);
        return M3_FALSE;
    }
********/
    if (0 != pInf->info_len && M3_NULL == pInf->p_info_str)
    {
        M3ERRNO(EM3_NULL_PARAM);
        return M3_FALSE;
    }
    return M3_TRUE;
}

m3_bool_t    m3uaAssertStatus(m3_u16        usrId,
                              m3ua_status_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state   ||
        M3_TRUE == M3_IS_SP_ASP((M3_USR_TABLE(usrId))->sp_id)) {
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    if (4 < pInf->cause) {
        M3ERRNO(EM3_INV_CAUSE);
        return M3_FALSE;
    }

    /* check for congestion related information */
    if (M3_CAUSE_CONG == pInf->cause) {
        if (M3_FALSE == m3uaAssertNwApp(pInf->status_inf.cong_inf.nw_app)) {
            M3ERRNO(EM3_INV_NWAPP);
            return M3_FALSE;
        }
        if (0 == pInf->status_inf.cong_inf.num_pc ||
            M3_MAX_PC_SSNM < pInf->status_inf.cong_inf.num_pc) {
            M3ERRNO(EM3_INV_NUMPC);
            return M3_FALSE;
        }
        if (3 < pInf->status_inf.cong_inf.cong_level) {
            M3ERRNO(EM3_INV_CONGLEVEL);
            return M3_FALSE;
        }
/*JAN2010
        if (M3_MAX_INFO_STR_LEN < pInf->status_inf.cong_inf.info_len) {
            M3ERRNO(EM3_INV_INFOLEN);
            return M3_FALSE;
        }
********/
        if (0 != pInf->status_inf.cong_inf.info_len &&
            M3_NULL == pInf->status_inf.cong_inf.p_info_str) {
            M3ERRNO(EM3_NULL_PARAM);
            return M3_FALSE;
        }
    }
    if (M3_CAUSE_DRST == pInf->cause) {
        if (M3_FALSE == m3uaAssertNwApp(pInf->status_inf.upu_inf.nw_app)) {
            M3ERRNO(EM3_INV_NWAPP);
            return M3_FALSE;
        }
/*JAN2010
        if (M3_MAX_INFO_STR_LEN < pInf->status_inf.upu_inf.info_len) {
            M3ERRNO(EM3_INV_INFOLEN);
            return M3_FALSE;
        }
********/
        if (0 != pInf->status_inf.upu_inf.info_len &&
            M3_NULL == pInf->status_inf.upu_inf.p_info_str) {
            M3ERRNO(EM3_NULL_PARAM);
            return M3_FALSE;
        }
    }
    if (M3_CAUSE_UNKNOWN == pInf->cause ||
        M3_CAUSE_UNEQUPPD_RMT_USR == pInf->cause ||
        M3_CAUSE_INACCESS_RMT_USR == pInf->cause) {
        if (M3_FALSE == m3uaAssertNwApp(pInf->status_inf.drst_inf.nw_app)) {
            M3ERRNO(EM3_INV_NWAPP);
            return M3_FALSE;
        }
        if (0 == pInf->status_inf.drst_inf.num_pc ||
            M3_MAX_PC_SSNM < pInf->status_inf.drst_inf.num_pc) {
            M3ERRNO(EM3_INV_NUMPC);
            return M3_FALSE;
        }
/*JAN2010
        if (M3_MAX_INFO_STR_LEN < pInf->status_inf.drst_inf.info_len) {
            M3ERRNO(EM3_INV_INFOLEN);
            return M3_FALSE;
        }
********/
        if (0 != pInf->status_inf.drst_inf.info_len &&
            M3_NULL == pInf->status_inf.drst_inf.p_info_str) {
            M3ERRNO(EM3_NULL_PARAM);
            return M3_FALSE;
        }
    }
    return M3_TRUE;
}

m3_bool_t    m3uaAssertRestart(m3_u16          usrId,
                               m3ua_restart_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state   ||
        M3_TRUE == M3_IS_SP_ASP((M3_USR_TABLE(usrId))->sp_id))
    {
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->nw_app))
    {
        M3ERRNO(EM3_INV_NWAPP);
        return M3_FALSE;
    }
    if (M3_RESTART_IN_PROGRESS != pInf->status                &&
        M3_RESTART_DONE != pInf->status) {
        M3ERRNO(EM3_INV_RESTART_STATUS);
        return M3_FALSE;
    }
    return M3_TRUE;
}

m3_bool_t    m3uaAssertTransfer(m3_u16          usrId,
                                m3ua_txr_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if (M3_FALSE == M3_USRID_VALID(usrId)            ||
        M3_FALSE == (M3_USR_TABLE(usrId))->e_state)
    {
        printf("1111111\n");
        M3ERRNO(EM3_INV_USRID);
        return M3_FALSE;
    }
    #ifdef SIMULATOR_CN
    if (M3_MAX_U32 != pInf->nw_app                   &&
        M3_FALSE == m3uaAssertNwApp(pInf->nw_app))
    {
        printf("2222222\n");
        M3ERRNO(EM3_INV_NWAPP);
        return M3_FALSE;
    }
    #endif
    if (0 != pInf->prot_data_len && M3_NULL == pInf->p_prot_data)
    {
        printf("3333333\n");
        M3ERRNO(EM3_NULL_PARAM);
        return M3_FALSE;
    }
    return M3_TRUE;
}

