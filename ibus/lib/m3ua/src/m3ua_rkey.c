/****************************************************************************
** Description:
** Code for provisioning routing key through RKM procedure.
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

#include <m3ua_rkey.h>

m3_s32 m3ua_rkey(m3_u8        oprn,
                 m3ua_rkey_t  *pInf)
{
    m3_s32        ret;

    switch(oprn) {
        case M3_REGISTER: {
            ret = m3uaRegisterRKEY(pInf);
            break;
        }
        case M3_DEREGISTER: {
            ret = m3uaDeregisterRKEY(pInf);
            break;
        }
        case M3_STATUS: {
            ret = m3uaRKEYStatus(pInf);
            break;
        }
        default: {
            M3ERRNO(EM3_INV_OPER);
            return -1;
        }
    }
    return ret;
}

m3_s32 m3uaRegisterRKEY(m3ua_rkey_t *pInf)
{
    m3_asp_inf_t        *pasp;
    m3_u16              idx;
    m3_s32              ret = 0;
    m3_conn_inf_t       *pconn;

    if (M3_FALSE == M3_IS_SP_ASP(pInf->reg.asp_id) ||
        M3_FALSE == M3_ASPID_VALID(pInf->reg.asp_id)      ||
        M3_FALSE == (M3_ASP_TABLE(pInf->reg.asp_id))->e_state) {
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pasp = M3_ASP_TABLE(pInf->reg.asp_id);
    if (M3_TRUE == M3_IS_SP_ASP(pInf->reg.rsp_id)) {
        if ((M3_FALSE == M3_R_ASPID_VALID(pInf->reg.rsp_id)   ||
            M3_FALSE == (M3_R_ASP_TABLE(pInf->reg.rsp_id))->e_state)) {
            M3ERRNO(EM3_INV_R_ASPID);
            return -1;
        }
        if (M3_MAX_U32 == pasp->r_asp_inf[(m3_u16)pInf->reg.rsp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(pasp->r_asp_inf[(m3_u16)pInf->reg.rsp_id].conn_id);
    } else if (M3_TRUE == M3_IS_SP_SGP(pInf->reg.rsp_id)) {
        if ((M3_FALSE == M3_R_SGPID_VALID(pInf->reg.rsp_id)   ||
            M3_FALSE == (M3_R_SGP_TABLE(pInf->reg.rsp_id))->e_state)) {
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        if (M3_MAX_U32 == pasp->r_sgp_inf[(m3_u16)pInf->reg.rsp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(pasp->r_sgp_inf[(m3_u16)pInf->reg.rsp_id].conn_id);
    } else {
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (0 == pInf->reg.num_as || M3_MAX_AS < pInf->reg.num_as) {
        M3ERRNO(EM3_INV_NUM_AS);
        return -1;
    }
    for (idx = 0; idx < pInf->reg.num_as; idx++) {
        if (M3_FALSE == M3_ASID_VALID(pInf->reg.as_list[idx])   ||
            M3_FALSE == (M3_AS_TABLE(pInf->reg.as_list[idx]))->e_state ||
            M3_FALSE == pasp->as_list[(m3_u16)pInf->reg.as_list[idx]]) {
            M3ERRNO(EM3_INV_ASLIST);
            return -1;
        }
    }
    ret = m3ua_L_RKM(pconn, M3_L_RKM_EVT_REGISTER, pInf);
    return ret;
}

m3_s32 m3uaDeregisterRKEY(m3ua_rkey_t *pInf)
{
    m3_asp_inf_t        *pasp;
    m3_u16              idx;
    m3_s32              ret = 0;
    m3_conn_inf_t       *pconn;

    if (M3_FALSE == M3_ASPID_VALID(pInf->dereg.asp_id)      ||
        M3_FALSE == (M3_ASP_TABLE(pInf->dereg.asp_id))->e_state) {
        M3ERRNO(EM3_INV_ASPID);
        return -1;
    }
    pasp = M3_ASP_TABLE(pInf->dereg.asp_id);
    if (M3_TRUE == M3_IS_SP_ASP(pInf->dereg.rsp_id)) {
        if (M3_FALSE == M3_R_ASPID_VALID(pInf->dereg.rsp_id)   ||
            M3_FALSE == (M3_R_ASP_TABLE(pInf->dereg.rsp_id))->e_state) {
            M3ERRNO(EM3_INV_R_ASPID);
            return -1;
        }
        if (M3_MAX_U32 == pasp->r_asp_inf[(m3_u16)pInf->reg.rsp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(pasp->r_asp_inf[(m3_u16)pInf->reg.rsp_id].conn_id);
    } else if (M3_TRUE == M3_IS_SP_SGP(pInf->dereg.rsp_id)) {
        if (M3_FALSE == M3_R_SGPID_VALID(pInf->dereg.rsp_id)   ||
            M3_FALSE == (M3_R_SGP_TABLE(pInf->dereg.rsp_id))->e_state) {
            M3ERRNO(EM3_INV_R_SGPID);
            return -1;
        }
        if (M3_MAX_U32 == pasp->r_sgp_inf[(m3_u16)pInf->reg.rsp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(pasp->r_sgp_inf[(m3_u16)pInf->reg.rsp_id].conn_id);
    } else {
        M3ERRNO(EM3_INV_RSPID);
        return -1;
    }
    if (0 == pInf->dereg.num_as || M3_MAX_AS < pInf->dereg.num_as) {
        M3ERRNO(EM3_INV_NUM_AS);
        return -1;
    }
    for (idx = 0; idx < pInf->dereg.num_as; idx++) {
        if (M3_FALSE == M3_ASID_VALID(pInf->dereg.as_list[idx])   ||
            M3_FALSE == (M3_AS_TABLE(pInf->dereg.as_list[idx]))->e_state ||
            M3_FALSE == pasp->as_list[(m3_u16)pInf->dereg.as_list[idx]]) {
            M3ERRNO(EM3_INV_ASLIST);
            return -1;
        }
    }
    ret = m3ua_L_RKM(pconn, M3_L_RKM_EVT_DEREGISTER, pInf);
    return ret;
}

m3_s32 m3uaRKEYStatus(m3ua_rkey_t *pInf)
{
    m3_asp_inf_t        *pasp;
    m3_sgp_inf_t        *psgp;
    m3_s32              ret = 0;
    m3_conn_inf_t       *pconn;
    m3_u16              rcIdx;

    if (M3_FALSE == M3_R_ASPID_VALID(pInf->status.rasp_id)      ||
        M3_FALSE == (M3_R_ASP_TABLE(pInf->status.rasp_id))->e_state) {
        M3ERRNO(EM3_INV_R_ASPID);
        return -1;
    }
    if (M3_TRUE == M3_IS_SP_ASP(pInf->status.lsp_id)) {
        if (M3_FALSE == M3_ASPID_VALID(pInf->status.lsp_id)   ||
            M3_FALSE == (M3_ASP_TABLE(pInf->status.lsp_id))->e_state) {
            M3ERRNO(EM3_INV_ASPID);
            return -1;
        }
        pasp = M3_ASP_TABLE(pInf->status.lsp_id);
        if (M3_MAX_U32 == pasp->r_asp_inf[(m3_u16)pInf->status.rasp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(pasp->r_asp_inf[(m3_u16)pInf->status.rasp_id].conn_id);
    } else if (M3_TRUE == M3_IS_SP_SGP(pInf->status.lsp_id)) {
        if (M3_FALSE == M3_SGPID_VALID(pInf->status.lsp_id)   ||
            M3_FALSE == (M3_SGP_TABLE(pInf->status.lsp_id))->e_state) {
            M3ERRNO(EM3_INV_SGPID);
            return -1;
        }
        psgp = M3_SGP_TABLE(pInf->status.lsp_id);
        if (M3_MAX_U32 == psgp->r_asp_inf[(m3_u16)pInf->status.rasp_id].conn_id) {
            M3ERRNO(EM3_INV_CONN);
            return -1;
        }
        pconn = M3_CONN_TABLE(psgp->r_asp_inf[(m3_u16)pInf->status.rasp_id].conn_id);
    } else {
        M3ERRNO(EM3_INV_LSPID);
        return -1;
    }
    if (0 == pInf->status.num_result || M3_MAX_RK < pInf->status.num_result) {
        M3ERRNO(EM3_INV_NUM_RESULT);
        return -1;
    }
    for (rcIdx = 0; rcIdx < pInf->status.num_result; rcIdx++) {
        if (M3_FALSE == M3_R_ASID_VALID(pInf->status.result[rcIdx].as_id)   ||
            M3_FALSE == M3_R_AS_TABLE(pInf->status.result[rcIdx].as_id)) {
            M3ERRNO(EM3_INV_R_ASID);
            return -1;
        }
        /* check value of reg status */
        if (10 < pInf->status.result[rcIdx].reg_status) {
            M3ERRNO(EM3_INV_REG_STATUS);
            return -1;
        }
    }

    ret = m3ua_R_RKM(pconn, M3_R_RKM_EVT_STATUS, pInf);
    return ret;
}

