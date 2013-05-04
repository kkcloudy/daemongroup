/****************************************************************************
** Description:
** Code for provisioning an M3UA user.
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

#include <m3ua_usr.h>

m3_s32 m3ua_user(m3_u16        usrId,
                 m3_u8         oprn,
                 m3ua_user_t   *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_usr_inf_t  *pUsr;
    m3_s32        ret;

    if (M3_FALSE == M3_USRID_VALID(usrId)) {
        M3TRACE(m3uaErrorTrace, ("Invalid User:%u", usrId));
        M3ERRNO(EM3_INV_USRID);
        return -1;
    }
    pUsr = M3_USR_TABLE(usrId);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddUser(pUsr, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteUser(pUsr);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Invalid operation:%u", (m3_u32)oprn));
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32   m3uaAddUser(m3_usr_inf_t  *pUsr,
                     m3ua_user_t   *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_asp_inf_t  *pasp;
    m3_sgp_inf_t  *psgp;
    m3ua_local_route_t   route;
    m3_as_inf_t   *pas;
    m3_u16        idx, s_idx;

    M3TRACE(m3uaConfigTrace, ("Adding user"));
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pUsr->e_state){
        M3ERRNO(EM3_USRID_ALRDY);
        return -1;
    }
    pUsr->sp_id = pInf->add.info.sp_id;
    iu_log_debug(" puser->sp_id = %d\n",pUsr->sp_id);
    if (M3_TRUE == M3_IS_SP_ASP(pInf->add.info.sp_id)) {
        if (M3_FALSE == M3_ASPID_VALID(pInf->add.info.sp_id)) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pasp = M3_ASP_TABLE(pInf->add.info.sp_id);
        if (M3_FALSE == pasp->e_state) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pUsr->user.mtp_user.sio = pInf->add.info.user.mtp_user.sio;
        if (M3_FALSE == M3_ASID_VALID(pInf->add.info.user.mtp_user.as_id)) {
            M3ERRNO(EM3_INV_ASID);
            return -1;
        }
        pas = M3_AS_TABLE(pInf->add.info.user.mtp_user.as_id);
        if (M3_FALSE == pas->e_state) {
            M3ERRNO(EM3_INV_ASID);
            return -1;
        }
        pUsr->user.mtp_user.as_id = pInf->add.info.user.mtp_user.as_id;
        route.add.info.user_id = pUsr->usr_id;
        for (idx = 0; idx <  pas->rkey.num_rtparam; idx++) {
            route.add.info.pc_inf.ptcode = pas->rkey.rtparam[idx].dpc;
            if (M3_MAX_U32 != pas->rkey.nw_app) {
                route.add.info.pc_inf.nw_app = pas->rkey.nw_app;
                m3uaAddLocalRoute(&route);
            } else {
                for (s_idx = 0; s_idx < M3_MAX_NWAPP; s_idx++) {
                    if (M3_TRUE == (M3_NWAPP_TABLE(s_idx))->e_state) {
                        route.add.info.pc_inf.nw_app=
                                             (M3_NWAPP_TABLE(s_idx))->nw_app;
                        m3uaAddLocalRoute(&route);
                    }
                }
            }
        }
    }
    else if (M3_TRUE == M3_IS_SP_SGP(pInf->add.info.sp_id)) {
        if (M3_FALSE == M3_SGPID_VALID(pInf->add.info.sp_id)) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        psgp = M3_SGP_TABLE(pInf->add.info.sp_id);
        if (M3_FALSE == psgp->e_state) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        if (M3_MAX_U16 != psgp->user_id) {
            M3ERRNO(EM3_USR_ALRDY_REGD);
            return -1;
        }
        psgp->user_id = pUsr->usr_id;
        pUsr->user.nif_user.a_data = pInf->add.info.user.nif_user.a_data;
    } else {
        M3ERRNO(EM3_INV_LSPID);
        return -1;
    }
    pUsr->e_state = M3_TRUE;
    return 0;
}


m3_s32   m3uaDeleteUser(m3_usr_inf_t    *pUsr)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3ua_local_route_t    route;

    if (M3_FALSE == pUsr->e_state){
        M3ERRNO(EM3_INV_USRID);
        return -1;
    }
    route.del.info.pc_inf.ptcode = M3_MAX_U32;
    route.del.info.user_id       = pUsr->usr_id;
    m3uaDeleteLocalRoute(&route);
    pUsr->e_state  = M3_FALSE;
    return 0;
}

