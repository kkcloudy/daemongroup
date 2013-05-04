/****************************************************************************
** Description:
** Code for provisioning network appearance.
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

#include <m3ua_nwapp.h>

/*network appearance*/
m3_s32 m3ua_nwapp(m3_u8        oprn,
                  m3ua_nwapp_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_s32        ret = 0;

    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL Information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    switch(oprn) {
        case M3_ADD: {
            ret = m3uaAddNwApp(pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteNwApp(pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3uaAddNwApp(m3ua_nwapp_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_nwapp_inf_t    *pNw;
    m3_u16            idx;

    M3TRACE(m3uaConfigTrace, ("Adding Network Appearance"));
    if (M3_TRUE == m3uaAssertNwApp(pInf->add.info.nw_app)) {
        M3TRACE(m3uaErrorTrace, ("Network Appearance:%u already provisioned", pInf->add.info.nw_app));
        M3ERRNO(EM3_NWAPP_ALRDY);
        return -1;
    }
    if (M3_FALSE == M3_STD_VALID(pInf->add.info.standard)) {
        M3TRACE(m3uaErrorTrace, ("Invalid Standard:%u", pInf->add.info.standard));
        M3ERRNO(EM3_INV_STD);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_NWAPP; idx++) {
        pNw = M3_NWAPP_TABLE(idx);
        if (M3_FALSE == pNw->e_state) {
            pNw->e_state = M3_TRUE;
            pNw->nw_app = pInf->add.info.nw_app;
            pNw->standard = pInf->add.info.standard;
            pNw->status = M3_RESTART_DONE;
            return 0;
        }
    }
    M3TRACE(m3uaErrorTrace, ("No free entry in NwApp information container"));
    M3ERRNO(EM3_NWAPP_TABLE_FULL);
    return -1;
}

m3_s32 m3uaDeleteNwApp(m3ua_nwapp_t    *pInf)
{
    m3_nwapp_inf_t    *pNw;
    m3_u16            idx;

    M3TRACE(m3uaConfigTrace, ("Deleting Network Appearance"));
    for (idx = 0; idx < M3_MAX_NWAPP; idx++) {
        pNw = M3_NWAPP_TABLE(idx);
        if (M3_TRUE == pNw->e_state                      && 
            pNw->nw_app == pInf->del.info.nw_app) {
            pNw->e_state = M3_FALSE;
            pNw->nw_app = M3_MAX_NWAPP;
            return 0;
        } 
    }
    M3TRACE(m3uaErrorTrace, ("Non-provisioned Network Appearance:%u", pInf->del.info.nw_app));
    M3ERRNO(EM3_INV_NWAPP);
    return -1;
}

m3_bool_t m3uaAssertNwApp(m3_u32    nw_app)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("nw_app = %d\n",nw_app);
    m3_u16            idx;
    m3_nwapp_inf_t    *pNwApp;

    if ( M3_MAX_U32 == nw_app )
        return M3_TRUE;

    for (idx = 0; idx < M3_MAX_NWAPP; idx++) {
        iu_log_debug("idx = %d\n",idx);
        pNwApp = M3_NWAPP_TABLE(idx);
        if (M3_MAX_U32 != pNwApp->nw_app) {
            iu_log_debug("pNwApp->nw_app = %d\n",pNwApp->nw_app);
            if (nw_app == pNwApp->nw_app) {
                return M3_TRUE;
            }
        } else {
            return M3_FALSE;
        }
    }
    return M3_FALSE;
}

m3_s32 m3uaSetRestartState(m3_u32               nw_app,
                           m3_restart_status_t  status)
{
    m3_u16    nwIdx;

    for (nwIdx = 0; nwIdx < M3_MAX_NWAPP; nwIdx++)
        if (nw_app == (M3_NWAPP_TABLE(nwIdx))->nw_app)
            (M3_NWAPP_TABLE(nwIdx))->status = status;
    return 0;
}

m3_u16 m3uaGetMaxSLS(m3_u32        sp_id,
                     m3_u32        nw_app)
{
    m3_u16        nwIdx;
    m3_standard_t standard;

    if (M3_MAX_U32 == nw_app) {
        if (M3_TRUE == M3_IS_SP_ASP(sp_id))
            nw_app = (M3_ASP_TABLE(sp_id))->def_nwapp;
        else
            nw_app = (M3_SGP_TABLE(sp_id))->def_nwapp;
    }
    for (nwIdx = 0; nwIdx < M3_MAX_NWAPP; nwIdx++)
        if (nw_app == (M3_NWAPP_TABLE(nwIdx))->nw_app) {
            standard = (M3_NWAPP_TABLE(nwIdx))->standard;
            break;
        }
    switch(standard) {
        case M3_STD_ANSI: {
            return 256;
        }
        case M3_STD_ITU: {
            return 16;
        }
    }
    return 0;
}

