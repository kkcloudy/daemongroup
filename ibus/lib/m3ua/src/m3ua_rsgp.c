/****************************************************************************
** Description:
** Code for provisioning remote SGP.
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

#include <m3ua_rsgp.h>

m3_s32 m3ua_r_sgp(m3_u32        sgp_id,
                  m3_u8         oprn,
                  m3ua_r_sgp_t  *pInf)
{
    m3_r_sgp_inf_t    *pSgp;
    m3_s32            ret;

    if (M3_FALSE == M3_R_SGPID_VALID(sgp_id)) {
        M3ERRNO(EM3_INV_R_SGPID);
        return -1;
    }

    pSgp = M3_R_SGP_TABLE(sgp_id);
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddRSGP(pSgp, pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteRSGP(pSgp, pInf);
            break;
        }
        case M3_GET: {
            ret = m3uaGetRSGPConf(pSgp, pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            break;
        }
    }
    return ret;
}

m3_s32 m3uaAddRSGP(m3_r_sgp_inf_t    *pSgp,
                   m3ua_r_sgp_t      *pInf)
{
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_TRUE == pSgp->e_state) {
        M3ERRNO(EM3_R_SGPID_ALRDY);
        return -1;
    }
    pSgp->e_state    = M3_TRUE;
    pSgp->sctp_ep_id = pInf->add.info.sctp_ep_id;
    return 0;
}

m3_s32 m3uaDeleteRSGP(m3_r_sgp_inf_t    *pSgp,
                      m3ua_r_sgp_t      *pInf)
{
    m3_u16         idx, s_idx;
    m3_sg_inf_t    *psg = M3_SG_TABLE(pSgp->sg_id);
    m3_u32         sgp_id = 0x00010000 | pSgp->sgp_id;

    if (M3_FALSE == pSgp->e_state) {
        M3ERRNO(EM3_INV_R_SGPID);
        return -1;
    }
    for (idx = 0; idx < M3_MAX_CONN; idx++) {
        if ((M3_TRUE == (M3_CONN_TABLE(idx))->e_state)      &&
            (sgp_id == (M3_CONN_TABLE(idx))->r_sp_id)) {
            M3ERRNO(EM3_CONN_EXIST);
            return -1;
        }
    }
    for (idx = 0; idx < psg->num_sgp; idx++) {
        if (sgp_id == psg->sgp_list[idx]) {
            for (s_idx = idx; s_idx < psg->num_sgp - 1; s_idx++) {
                psg->sgp_list[s_idx] = psg->sgp_list[s_idx + 1];
            }
            psg->num_sgp = psg->num_sgp - 1;
            break;
        }
    }
    pSgp->e_state = M3_FALSE;
    return 0;
}

m3_s32 m3uaGetRSGPConf(m3_r_sgp_inf_t    *pSgp,
                       m3ua_r_sgp_t      *pInf)
{
    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == pSgp->e_state) {
        M3ERRNO(EM3_INV_R_SGPID);
        return -1;
    }
    pInf->get.info.sctp_ep_id = pSgp->sctp_ep_id;
    return 0;
}

