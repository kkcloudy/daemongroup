/****************************************************************************
** Description:
** Code for provisioning an SG.
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

#include <m3ua_sg.h>

m3_s32 m3ua_sg(m3_u32        sgId,
               m3_u8         oprn,
               m3ua_sg_t     *pInf)
{
    m3_sg_inf_t    *pSg;
    m3_s32         ret;

    if (M3_FALSE == M3_SGID_VALID(sgId)) {
        M3ERRNO(EM3_INV_SGID);
        return -1;
    }

    pSg = M3_SG_TABLE(sgId);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddSG(pSg, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteSG(pSg, pInf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetSGConf(pSg, pInf);
            break;
        }
        case M3_MODIFY: {
            ret = m3uaModifySGConf(pSg, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            break;
        }
    }
    return ret;
}

m3_s32 m3uaAddSG(m3_sg_inf_t    *pSg,
                 m3ua_sg_t      *pInf)
{
    m3_u16         idx;
    m3_r_sgp_inf_t *pSgp;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pSg->e_state) {
        M3ERRNO(EM3_SGID_ALRDY);
        return -1;
    }
    if (M3_SGMODE_LOADSHARE != pInf->add.info.sgmode        &&
        M3_SGMODE_BROADCAST != pInf->add.info.sgmode) {
        M3ERRNO(EM3_INV_SGMODE);
        return -1;
    }
    if (M3_MAX_R_SGP < pInf->add.info.num_sgp) {
        M3ERRNO(EM3_INV_NUMSGP);
        return -1;
    }
    for (idx = 0; idx < pInf->add.info.num_sgp; idx++) {
        if (M3_FALSE == M3_R_SGPID_VALID(pInf->add.info.sgp_list[idx])) {
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        pSgp = M3_R_SGP_TABLE(pInf->add.info.sgp_list[idx]);
        if (M3_FALSE == pSgp->e_state) {
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        pSg->sgp_list[idx] = pInf->add.info.sgp_list[idx];
        pSgp->sg_id = 0x00010000 | pSg->sg_id;
    }
    pSg->num_sgp = pInf->add.info.num_sgp;
    pSg->sgmode  = pInf->add.info.sgmode;
    pSg->e_state = M3_TRUE;
    pSg->timer_inf.timer_id = M3_MAX_U32;
    pSg->timer_inf.p_buf    = M3_NULL;
    return 0;
}

m3_s32 m3uaDeleteSG(m3_sg_inf_t    *pSg,
                    m3ua_sg_t      *pInf)
{
    m3ua_route_t    route;

    if (M3_FALSE == pSg->e_state) {
        M3ERRNO(EM3_INV_SGID);
        return -1;
    }
    if (0 != pSg->num_sgp) {
        M3ERRNO(EM3_SG_WITH_SGP);
        return -1;
    }
    pSg->e_state = M3_FALSE;
    pSg->num_sgp = 0;
    if (M3_NULL != pSg->timer_inf.p_buf) {
        M3_MSG_FREE(pSg->timer_inf.p_buf, M3_TIMER_POOL);
        pSg->timer_inf.p_buf = M3_NULL;
    }
    route.del.info.pc_inf.ptcode = M3_MAX_U32;
    route.del.info.le_id         = 0x00010000 | pSg->sg_id;
    route.del.info.priority      = M3_MAX_U8;
    m3uaDeleteRoute(&route);
    return 0;
}

m3_s32 m3uaGetSGConf(m3_sg_inf_t    *pSg,
                     m3ua_sg_t      *pInf)
{
    m3_u16    idx;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pSg->e_state) {
        M3ERRNO(EM3_INV_SGID);
        return -1;
    }
    pInf->get.info.sgmode = pSg->sgmode;
    pInf->get.info.num_sgp = pSg->num_sgp;
    for (idx = 0; idx < pSg->num_sgp; idx++) {
        pInf->get.info.sgp_list[idx] = pSg->sgp_list[idx];
    }
    return 0;
}

m3_s32 m3uaModifySGConf(m3_sg_inf_t    *pSg,
                        m3ua_sg_t      *pInf)
{
    m3_u16    idx;
    m3_r_sgp_inf_t *pSgp;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pSg->e_state) {
        M3ERRNO(EM3_INV_SGID);
        return -1;
    }
    switch(pInf->modify.confname)
    {
        case M3_SG_MODE: {
            if (M3_SGMODE_LOADSHARE != pInf->modify.info.sgmode        &&
                M3_SGMODE_BROADCAST != pInf->modify.info.sgmode) {
                M3ERRNO(EM3_INV_SGMODE);
                return -1;
            }
            pSg->sgmode = pInf->modify.info.sgmode;
            break;
        }
        case M3_SG_SGP_LIST: {
            if (M3_MAX_R_SGP < pInf->modify.info.num_sgp) {
                M3ERRNO(EM3_INV_NUMSGP);
                return -1;
            }
            for (idx = 0; idx < pInf->modify.info.num_sgp; idx++) {
                if (M3_FALSE ==
                    M3_R_SGPID_VALID(pInf->modify.info.sgp_list[idx])) {
                    M3ERRNO(EM3_INV_R_SGPID);
                    return -1;
                }
                pSgp = M3_R_SGP_TABLE(pInf->modify.info.sgp_list[idx]);
                if (M3_FALSE == pSgp->e_state) {
                    M3ERRNO(EM3_INV_R_SGPID);
                    return -1;
                }
            }
            for (idx = 0; idx < pInf->modify.info.num_sgp; idx++) {
                pSg->sgp_list[idx] = pInf->modify.info.sgp_list[idx];
                pSgp = M3_R_SGP_TABLE(pInf->modify.info.sgp_list[idx]);//Dancis-April 2006
                pSgp->sg_id = 0x00010000 | pSg->sg_id;
            }
            pSg->num_sgp = pInf->modify.info.num_sgp;
            break;
        }
        case M3_SG_INFO: {
            if (M3_SGMODE_LOADSHARE != pInf->modify.info.sgmode        &&
                M3_SGMODE_BROADCAST != pInf->modify.info.sgmode) {
                M3ERRNO(EM3_INV_SGMODE);
                return -1;
            }
            if (M3_MAX_R_SGP < pInf->modify.info.num_sgp) {
                M3ERRNO(EM3_INV_NUMSGP);
                return -1;
            }
            for (idx = 0; idx < pInf->modify.info.num_sgp; idx++) {
                if (M3_FALSE ==
                    M3_R_SGPID_VALID(pInf->modify.info.sgp_list[idx])) {
                    M3ERRNO(EM3_INV_R_SGPID);
                    return -1;
                }
                pSgp = M3_R_SGP_TABLE(pInf->modify.info.sgp_list[idx]);
                if (M3_FALSE == pSgp->e_state) {
                    M3ERRNO(EM3_INV_R_SGPID);
                    return -1;
                }
                pSg->sgp_list[idx] = pInf->modify.info.sgp_list[idx];
                pSgp->sg_id = 0x00010000 | pSg->sg_id;// DANCIS-April2006
            }
            pSg->sgmode = pInf->modify.info.sgmode;
            pSg->num_sgp = pInf->modify.info.num_sgp;
            break;
        }
        default: {
            M3ERRNO(EM3_INV_CONFNAME);
            return -1;
        }
    }
    return 0;
}


/* XXX-DEC2009 */
m3_s32 m3uaSGAddRoute(m3_sg_inf_t    *pSg,
                      m3_pc_inf_t    *pPcInf)
{
    if (M3_MAX_ROUTES_PER_SG <= pSg->nPC)
    {
        M3ERRNO(EM3_MAX_ROUTES_PER_SG);
        return -1;
    }

    pSg->pcList[pSg->nPC].nw_app = pPcInf->nw_app;
    pSg->pcList[pSg->nPC].ptcode = pPcInf->ptcode;
    pSg->nPC++;
    M3TRACE(m3uaConfigTrace, ("Adding Route (NwApp,PtCode):%u,%u",
        (unsigned int)pPcInf->nw_app,(unsigned int)pPcInf->ptcode));
    M3TRACE(m3uaConfigTrace, ("Number of Routes through SG:%u",(unsigned int)pSg->nPC));
    return 0;
}


m3_s32 m3uaSGDeleteRoute(m3_sg_inf_t *pSg,
                         m3_pc_inf_t *pPcInf)
{   
    m3_u16            idx = 0;

    for (idx = 0; idx < pSg->nPC; idx++)
    {
        if ((pSg->pcList[idx].nw_app == pPcInf->nw_app) &&
            (pSg->pcList[idx].ptcode == pPcInf->ptcode))
        {
            /* move last entry to deleted entry to reduce list size */
            pSg->pcList[idx].nw_app = pSg->pcList[pSg->nPC-1].nw_app;
            pSg->pcList[idx].ptcode = pSg->pcList[pSg->nPC-1].ptcode;
            pSg->nPC--;
            M3TRACE(m3uaConfigTrace, ("Deleted Route (NwApp,PtCode):%u,%u",
                (unsigned int)pPcInf->nw_app,(unsigned int)pPcInf->ptcode));
            M3TRACE(m3uaConfigTrace, ("Number of Routes through SG:%u",(unsigned int)pSg->nPC));
            return 0;
        }
    }
    M3ERRNO(EM3_INV_ROUTE_THRU_SG);
    return -1;
}

