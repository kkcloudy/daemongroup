/****************************************************************************
** Description:
** Code for provisioning remote ASP.
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

#include <m3ua_rasp.h>

m3_s32 m3ua_r_asp(m3_u32        asp_id,
                  m3_u8         oprn,
                  m3ua_r_asp_t  *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_r_asp_inf_t  *pAsp;
    m3_s32          ret = 0;

	/*asp_id = msc is 0, sgsn is 1*/
    if (M3_FALSE == M3_R_ASPID_VALID(asp_id)) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }

    pAsp = M3_R_ASP_TABLE(asp_id);
//iu_log_debug("## m3ua_r_asp asp_id %d \n", asp_id);	
    
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddRASP(pAsp, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteRASP(pAsp, pInf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetRASPConf(pAsp, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3ua_r_asplock(m3_u32              asp_id,
                      m3_u8               oprn,
                      void                *pInf)
{
    m3_r_asp_inf_t  *pAsp;

    if (M3_FALSE == M3_R_ASPID_VALID(asp_id)) {
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pAsp = M3_R_ASP_TABLE(asp_id);
    if (M3_FALSE == pAsp->e_state) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }
    switch(oprn)
    {
        case M3_ADD: {
            pAsp->lock_state = M3_TRUE;
            break;
        }
        case M3_DELETE: {
            pAsp->lock_state = M3_FALSE;
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return 0;
}

m3_s32 m3uaAddRASP(m3_r_asp_inf_t  *pAsp,
                   m3ua_r_asp_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pAsp->e_state) {
        M3ERRNO(EM3_R_ASPID_ALRDY);
        return -1;
    }
    pAsp->sctp_ep_id = pInf->add.info.sctp_ep_id;
    pAsp->e_state = M3_TRUE;
    //iu_log_debug("pAsp->sctp_ep_id = %u, pAsp->e_state = %d\n",pAsp->sctp_ep_id, pAsp->e_state);
    return 0;
}

m3_s32 m3uaDeleteRASP(m3_r_asp_inf_t    *pAsp,
                      m3ua_r_asp_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_u16      idx, s_idx;

    if (M3_FALSE == pAsp->e_state) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_CONN; idx++) {
        if ((M3_TRUE == (M3_CONN_TABLE(idx))->e_state)      &&
            (pAsp->asp_id == (M3_CONN_TABLE(idx))->r_sp_id)) {
            M3ERRNO(EM3_CONN_EXIST);
            return -1;
        }
    }
    for (idx = 0; idx < M3_MAX_ASP; idx++) {
        for (s_idx = 0; s_idx < M3_MAX_R_AS; s_idx++)
        (M3_ASP_TABLE(idx))->r_as_inf[s_idx].asp_list[pAsp->asp_id] = M3_FALSE;
    }
    for (idx = 0; idx < M3_MAX_SGP; idx++) {
        for (s_idx = 0; s_idx < M3_MAX_R_AS; s_idx++)
        (M3_SGP_TABLE(idx))->r_as_inf[s_idx].asp_list[pAsp->asp_id] = M3_FALSE;
    }
    pAsp->e_state = M3_FALSE;
    return 0;
}

m3_s32 m3uaGetRASPConf(m3_r_asp_inf_t    *pAsp,
                       m3ua_r_asp_t      *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pAsp->e_state) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }
    pInf->get.info.sctp_ep_id = pAsp->sctp_ep_id;
    return 0;
}

m3_s32 m3ua_r_asp_state(m3_u32              asp_id,
                        m3_u32              l_spid,
                        m3_u8               oprn,
                        m3ua_r_asp_state_t  *pInf)
{
    m3_r_asp_inf_t  *pAsp;
    m3_asp_inf_t    *plasp;
    m3_sgp_inf_t    *plsgp;
    m3_u32          conn_id;
    m3_conn_inf_t   *pconn;
    m3_r_as_inf_t   *pAs;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == M3_R_ASPID_VALID(asp_id)) {
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pAsp = M3_R_ASP_TABLE(asp_id);
    if (M3_FALSE == pAsp->e_state) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
        if (M3_FALSE == M3_ASPID_VALID(l_spid)) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        plasp = M3_ASP_TABLE(l_spid);
        if (M3_FALSE == plasp->e_state) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        conn_id = plasp->r_asp_inf[(m3_u16)asp_id].conn_id;
    } else if (M3_TRUE == M3_IS_SP_SGP(l_spid)) {
        if (M3_FALSE == M3_SGPID_VALID(l_spid)) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        plsgp = M3_SGP_TABLE(l_spid);
        if (M3_FALSE == plsgp->e_state) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        conn_id = plsgp->r_asp_inf[(m3_u16)asp_id].conn_id;
    } else {
        M3ERRNO(EM3_INV_SPID);
        return -1;
    }
    switch(oprn)
    {
        case M3_GET: {
            if (M3_FALSE == M3_R_ASID_VALID(pInf->get.info.as_id)) {
                M3ERRNO(EM3_INV_R_ASID);
                return -1;
            }
            pAs = M3_R_AS_TABLE(pInf->get.info.as_id);
            if (M3_FALSE == pAs->e_state) {
                M3ERRNO(EM3_INV_ASID);
                return -1;
            }
            if (M3_TRUE == M3_IS_SP_ASP(l_spid)) {
                if (M3_FALSE == plasp->r_as_inf[pInf->get.info.as_id].\
                    asp_list[asp_id]) {
                    M3ERRNO(EM3_AS_NOT_SERV);
                    return -1;
                }
            }
            else {
                if (M3_FALSE == plsgp->r_as_inf[pInf->get.info.as_id].\
                    asp_list[asp_id]) {
                    M3ERRNO(EM3_AS_NOT_SERV);
                    return -1;
                }
            }
            pconn = M3_CONN_TABLE(conn_id);
            pInf->get.state=pconn->r_sp_st[pInf->get.info.as_id];
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return 0;
}

