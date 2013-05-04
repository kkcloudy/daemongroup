/****************************************************************************
** Description:
** Code for data transfer procedure.
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

#include <m3ua_txr.h>

m3_s32    m3ua_ETXR(m3_usr_inf_t  *pusr,
                    m3ua_txr_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u32       conn_id = M3_MAX_U32;
    m3_u32       stream_id = M3_MAX_U32;
    m3_u32       le_id;
    m3_u16       cic = M3_MAX_U16;
    m3_conn_inf_t *pconn;
    m3_u32       rsp_id;
    m3_u16       num_sp, num_le;
    m3_u32       sp_list[M3_MAX_CONN];
/*    m3_u16       eff_sls,max_sls = m3uaGetMaxSLS(pusr->sp_id,pInf->nw_app);change by yinlm for del nwapp*/
	m3_u16         eff_sls, max_sls = 256;
    m3_as_state_t as_state;
    m3_u16       num_send_sp;
    m3_u32       *psend_sp_list;
    m3_u16       idx;
    m3_u32       default_nwapp = pInf->nw_app;

	iu_log_debug("run 1 ---------------------------------------\n");
    if (M3_MAX_U32 == pInf->nw_app) {
        if (M3_TRUE == M3_IS_SP_ASP(pusr->sp_id))
            default_nwapp = (M3_ASP_TABLE(pusr->sp_id))->def_nwapp;
        else
            default_nwapp = (M3_SGP_TABLE(pusr->sp_id))->def_nwapp;
    }
	iu_log_debug("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    if (M3_SIO_ISUP == pInf->rt_lbl.si) {
        if (2 <= (pInf->prot_data_len)) {
            m3uaDecodeCIC(pusr->sp_id, default_nwapp, pInf->p_prot_data, &cic);
        }
    }
	iu_log_debug("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
   if (-1 == m3uaGetRLEForRTLBL(pusr->sp_id, default_nwapp,
       &pInf->rt_lbl, cic, &le_id, &num_le)) {
       return -1;
   } 
    if (M3_TRUE == M3_IS_SP_SGP(pusr->sp_id) && M3_TRUE == M3_IS_LE_SG(le_id)) {
        M3ERRNO(EM3_INV_ROUTING);
        return -1;
    }
	iu_log_debug("run2 ---------------------------------------------\n");
    m3uaGetAvailableRSPList(pusr, le_id, &num_sp, sp_list);
    if (0 != num_sp) {
		iu_log_debug("run 3 ----------------------------------------------\n");
		iu_log_debug("max_sls %d= max_sls %d/num_le %d\n", max_sls,max_sls,num_le);		
        max_sls = max_sls/num_le;
        eff_sls = pInf->rt_lbl.sls % max_sls;
        if ((M3_TRUE == M3_IS_LE_AS(le_id)      &&
             M3_TRFMODE_BROADCAST == (M3_R_AS_TABLE(le_id))->rkey.trfmode) ||
            (M3_TRUE == M3_IS_LE_SG(le_id)      &&
             M3_SGMODE_BROADCAST == (M3_SG_TABLE(le_id))->sgmode)) {
             iu_log_debug("run 4 ----------------------------------------------\n");
            num_send_sp = num_sp;
            psend_sp_list = sp_list;
        } else {
        	iu_log_debug("run 5 ----------------------------------------------\n");
            psend_sp_list = &sp_list[(num_sp * eff_sls)/max_sls];
            num_send_sp = 1;
        }
        if (M3_TRUE == M3_IS_SP_ASP(pusr->sp_id)         &&
            M3_TRUE == M3_IS_LE_AS(le_id)) {
           iu_log_debug("run 6 ----------------------------------------------\n");
            for (idx = 0; idx < num_send_sp; idx++) {
                rsp_id = psend_sp_list[idx];
                conn_id=(M3_ASP_TABLE(pusr->sp_id))->r_asp_inf[(m3_u16)rsp_id].conn_id;
                pconn = M3_CONN_TABLE(conn_id);
                stream_id = ((pconn->o_str - 1) * eff_sls)/max_sls;
                stream_id++;
                m3uaSendDATA(pusr, le_id, pconn, stream_id, pInf);
            }
        } else if (M3_TRUE == M3_IS_SP_ASP(pusr->sp_id)         &&
                 M3_TRUE == M3_IS_LE_SG(le_id)) {           
            for (idx = 0; idx < num_send_sp; idx++) {
                rsp_id = psend_sp_list[idx];
                conn_id=(M3_ASP_TABLE(pusr->sp_id))->r_sgp_inf[(m3_u16)rsp_id].conn_id;
                pconn = M3_CONN_TABLE(conn_id);
                stream_id = ((pconn->o_str - 1) * eff_sls)/max_sls;
                stream_id++;
                m3uaSendDATA(pusr, le_id, pconn, stream_id, pInf);
            }
        } else {        	
            for (idx = 0; idx < num_send_sp; idx++) {
                rsp_id = psend_sp_list[idx];
                conn_id=(M3_SGP_TABLE(pusr->sp_id))->r_asp_inf[(m3_u16)rsp_id].conn_id;
                pconn = M3_CONN_TABLE(conn_id);
                stream_id = ((pconn->o_str - 1) * eff_sls)/max_sls;
                stream_id++;
                m3uaSendDATA(pusr, le_id, pconn, stream_id, pInf);
            }
        }
    } 
    else {    	
        if (M3_TRUE == M3_IS_LE_AS(le_id)) {
            m3uaGetRASState(le_id, pusr->sp_id, &as_state);
            if (M3_AS_PENDING != as_state) {
                M3ERRNO(EM3_R_AS_NOT_ACTIVE);
                return -1;
            } else {
                m3ua_ETXR_PENDING(pusr, le_id, pInf);
            }
        } else {
            M3ERRNO(EM3_SG_NOT_ACTIVE);
            return -1;
        }
    }

	
//	printf("<<<<<<<<<<<<<<<<max_sls %d= max_sls %d/num_le %d\n", max_sls,max_sls,num_le);		
    return 0;
}

m3_s32    m3ua_ETXR_PENDING(m3_usr_inf_t  *pusr,
                            m3_u32        le_id,
                            m3ua_txr_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_data_inf_t    data_inf;
    m3_u8            *p_msg;
    m3_u32           msglen = 0;

    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DATA_SIZE + pInf->prot_data_len,
        M3_TXR_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    data_inf.nw_app    = pInf->nw_app;
    data_inf.rtctx = M3_MAX_U32; /* Active Connection not found */
    data_inf.crn_id    = pInf->crn_id;
    data_inf.rt_lbl    = pInf->rt_lbl;
    data_inf.prot_data_len = pInf->prot_data_len;
    data_inf.p_prot_data   = pInf->p_prot_data;

    m3uaEncodeDATA(&data_inf, &msglen, p_msg);
    m3uaAddPDBuf(M3_R_AS_TABLE(le_id), pusr->sp_id, msglen, p_msg);
    return 0;
}

m3_s32    m3uaSendDATA(m3_usr_inf_t  *pusr,
                       m3_u32        le_id,
                       m3_conn_inf_t *pconn,
                       m3_u32        stream_id,
                       m3ua_txr_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_data_inf_t    data_inf;
    m3_u8            *p_msg;
    m3_u32           msglen = 0;
    m3_as_inf_t      *pas;
	iu_log_debug("----------------------------------------------------------------\n");
    p_msg = (m3_u8*)M3_MSG_ALLOC(M3_MAX_DATA_SIZE + pInf->prot_data_len,
        M3_TXR_POOL);
    if (M3_NULL == p_msg) {
        M3ERRNO(EM3_MALLOC_FAIL);
        return -1;
    }
    data_inf.nw_app    = pInf->nw_app;
    if (M3_FALSE == pInf->add_rtctx) {
        data_inf.rtctx = M3_MAX_U32;
    } else {
        if (M3_TRUE == M3_IS_LE_SG(le_id)) {
            pas = M3_AS_TABLE(pusr->user.mtp_user.as_id);
            data_inf.rtctx = m3uaGetRCFromAS(pconn, pusr->user.mtp_user.as_id);
        } else {
            data_inf.rtctx = m3uaGetRCFromRAS(pconn, le_id);
        }
    }
    data_inf.crn_id    = pInf->crn_id;
    data_inf.rt_lbl    = pInf->rt_lbl;
    data_inf.prot_data_len = pInf->prot_data_len;
    data_inf.p_prot_data   = pInf->p_prot_data;
	iu_log_debug("============================================\n");
    m3uaEncodeDATA(&data_inf, &msglen, p_msg);
    m3uaSendMsg(pconn, stream_id, msglen, p_msg);
    M3_MSG_FREE(p_msg, M3_TXR_POOL);
	iu_log_debug("============================================\n");
    return 0;
}

m3_s32    m3uaTxrInd(m3_conn_inf_t *pconn,
                     m3_data_inf_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        cic = M3_MAX_U16;
    m3_u16        user_id = M3_MAX_U16;

    if (M3_SIO_ISUP == pInf->rt_lbl.si) {
        if (2 <= pInf->prot_data_len) {
            m3uaDecodeCIC(pconn->l_sp_id,pInf->nw_app,pInf->p_prot_data,&cic);
        }
    }
    if (M3_MAX_U32 == pInf->nw_app) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id))
            pInf->nw_app = (M3_ASP_TABLE(pconn->l_sp_id))->def_nwapp;
        else
            pInf->nw_app = (M3_SGP_TABLE(pconn->l_sp_id))->def_nwapp;
    }
    if (0 == m3uaGetUserForRTLBL(pconn, pInf->nw_app,
        pInf->rtctx, &pInf->rt_lbl, cic, &user_id)) {
        m3uaNtfyTxr(user_id, pInf);
    } else {
    }
    return 0;
}

m3_s32   m3uaGetRLEForRTLBL(m3_u32         sp_id,
                            m3_u32         nw_app,
                            m3_rt_lbl_t    *p_rtlbl,
                            m3_u16         cic,
                            m3_u32         *p_leid,
                            m3_u16         *pnum_le)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_rt_inf_t    *p_node = NULL;
    m3_asp_inf_t   *pasp = NULL;
    m3_pc_inf_t    pc_inf = {p_rtlbl->dpc, nw_app};
    m3_u16         num_sg_up = 0;
    m3_u16         num_sg_rest = 0;
    m3_u16         num_sg_cng1 = 0;
    m3_u16         num_sg_cng2 = 0;
    m3_u16         num_sg_cng3 = 0;
    m3_u32         sg_list[M3_MAX_SG]; /* up->rest->cng1->cng2->cng3 */
    m3_u8          prio;
/*  m3_u16         max_sls = m3uaGetMaxSLS(sp_id, nw_app);change by yinlm for del nwapp*/
	m3_u16         max_sls = 256;
	//printf("2222222  max_sls = %d\n",max_sls);
	iu_log_debug("sp_id= %d, pc_inf.ptcode = %d, pc_inf.nw_app = %d\n",sp_id, pc_inf.ptcode, pc_inf.nw_app);
iu_log_debug(">>>>>>>>>>>>>>> max_slsmax_slsmax_slsmax_sls\n");
    if (-1 == m3uaFindRoute(&pc_inf, &p_node)) {
       M3ERRNO(EM3_NO_ROUTE);
       return -1;
    }

	iu_log_debug("###########################################\n");

    if (M3_TRUE == M3_IS_SP_ASP(sp_id)) {
        pasp = M3_ASP_TABLE(sp_id);
    }
	printf("aaa ====================\n");
    while (M3_NULL != p_node) {
		printf("bbb ====================\n");
        if (M3_TRUE == M3_IS_LE_AS(p_node->le_id)) {
			printf("ccc ====================\n");
			iu_log_debug("p_node->le_id = %d\n",p_node->le_id);
            if (0 == m3uaMatchRKEY(&(M3_R_AS_TABLE(p_node->le_id))->rkey,
                nw_app, p_rtlbl, cic)) {
                printf("ddd ====================\n");
                *p_leid = p_node->le_id;
                *pnum_le = 1;
                return 0;
            } else {
            	printf("eee ====================\n");
                p_node = p_node->p_same;
            }
        } else if (M3_TRUE == M3_IS_SP_ASP(sp_id)) {
        printf("fff ====================\n");
            prio = p_node->priority;
            while (M3_NULL != p_node && prio == p_node->priority) {
				printf("ggg ====================\n");
                if (M3_FALSE != m3uaAssertASPActiveInSG(pasp, p_node->le_id)) {
                    switch(p_node->rtstate) {
                        case M3_RTSTATE_UP: {
							printf("hhh ====================\n");
                            sg_list[num_sg_up++] = p_node->le_id;
                            break;
                        }
                        case M3_RTSTATE_RESTRICTED: {
							printf("iii ====================\n");
                            if (0 == num_sg_up)
                                sg_list[num_sg_rest++] = p_node->le_id;
                            break;
                        }
                        case M3_RTSTATE_CONG_1: {
							printf("jjj ====================\n");
                            if (0 == num_sg_up && 0 == num_sg_rest)
                                sg_list[num_sg_cng1++] = p_node->le_id;
                            break;
                        }
                        case M3_RTSTATE_CONG_2: {
							printf("kkk ====================\n");
                            if (0 == num_sg_up && 0 == num_sg_rest &&
                                0 == num_sg_cng1)
                                sg_list[num_sg_cng2++] = p_node->le_id;
                            break;
                        }
                        case M3_RTSTATE_CONG_3: {
							printf("LLL ====================\n");
                            if (0 == num_sg_up && 0 == num_sg_rest && 
                                0 == num_sg_cng1  && 0 == num_sg_cng2)
                                sg_list[num_sg_cng3++] = p_node->le_id;
                            break;
                        }
                        case M3_RTSTATE_DOWN: {
							printf("mmm ====================\n");
                            break;
                        }
                    }
                }
                p_node = p_node->p_same;
            }
            if (0 != num_sg_up || 0 != num_sg_rest || 0 != num_sg_cng1 ||
                0 != num_sg_cng2 || 0 != num_sg_cng3) {
                printf("nnn ====================\n");
                break;
            }
        }
    }

    
    if (0 != num_sg_up) {
		printf("ooo ====================\n");
        *pnum_le = num_sg_up;
        *p_leid = sg_list[(num_sg_up * p_rtlbl->sls)/max_sls];
    } 
    else if (0 != num_sg_rest) {
    	printf("ppp ====================\n");
        *pnum_le = num_sg_rest;
        *p_leid = sg_list[(num_sg_rest * p_rtlbl->sls)/max_sls];
    } 
    else if (0 != num_sg_cng1) {
    	printf("qqq ====================\n");
        *pnum_le = num_sg_cng1;
        *p_leid = sg_list[(num_sg_cng1 * p_rtlbl->sls)/max_sls];
    } 
    else if (0 != num_sg_cng2) {
    	printf("rrr ====================\n");
        *pnum_le = num_sg_cng2;
        *p_leid = sg_list[(num_sg_cng2 * p_rtlbl->sls)/max_sls];
    }
    else if (0 != num_sg_cng3) {
    	printf("sss ====================\n");
        *pnum_le = num_sg_cng3;
        *p_leid = sg_list[(num_sg_cng3 * p_rtlbl->sls)/max_sls];
    } 
    else {
    	printf("ttt ====================\n");
        M3ERRNO(EM3_NO_LE_ACTIVE);
        return -1;
    }	
    return 0;
}

m3_s32   m3uaMatchRKEY(m3_rk_inf_t   *p_rk,
                       m3_u32        nw_app,
                       m3_rt_lbl_t   *p_rtlbl,
                       m3_u16        cic)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("nw_app = %d, cic = %d, p_rtlbl->dpc = %d\n",nw_app, cic, p_rtlbl->dpc);

    m3_u16        idx, r_idx;

    for (r_idx = 0; r_idx < p_rk->num_rtparam; r_idx++) {
        if ((M3_MAX_U32 != p_rk->nw_app) && (M3_MAX_U32 != nw_app) &&
            (p_rk->nw_app != nw_app)) {
            continue;
        }
        if (p_rk->rtparam[r_idx].dpc != p_rtlbl->dpc) {
            continue;
        }
        if (0 != p_rk->rtparam[r_idx].num_si) {
            for (idx = 0; idx < p_rk->rtparam[r_idx].num_si; idx++) {
                if (p_rk->rtparam[r_idx].si_list[idx] == p_rtlbl->si) {
                    break;
                }
            }
            if (idx >= p_rk->rtparam[r_idx].num_si) {
                return -1;
            }
        }
        if (0 != p_rk->rtparam[r_idx].num_opc) {
            for (idx = 0; idx < p_rk->rtparam[r_idx].num_opc; idx++) {
                if (p_rk->rtparam[r_idx].opc_list[idx] == p_rtlbl->opc) {
                    break;
                }
            }
            if (idx >= p_rk->rtparam[r_idx].num_opc) {
                return -1;
            }
        }
        if (0 != p_rk->rtparam[r_idx].num_ckt_range && M3_MAX_U16 != cic) {
            for (idx = 0; idx < p_rk->rtparam[r_idx].num_ckt_range; idx++) {
                if ((p_rk->rtparam[r_idx].ckt_range[idx].opc == p_rtlbl->opc) &&
                    ((p_rk->rtparam[r_idx].ckt_range[idx].lcic <= cic) &&
                    (p_rk->rtparam[r_idx].ckt_range[idx].ucic >= cic))) {
                    break;
                }
            }
            if (idx >= p_rk->rtparam[r_idx].num_ckt_range) {
                return -1;
            }
        }
        break;
    }
    if (r_idx >= p_rk->num_rtparam) {
        return -1;
    }
    return 0;
}

m3_s32   m3uaGetAvailableRSPList(m3_usr_inf_t *pusr,
                                 m3_u32       le_id,
                                 m3_u16       *p_numsp,
                                 m3_u32       *p_splist)
{
    iu_log_debug("-----------%s  %d: %s------------\n",__FILE__, __LINE__, __func__);

    m3_u16    idx = 0;
    m3_asp_inf_t    *pasp;
    m3_sgp_inf_t    *psgp;
    m3_conn_inf_t   *pconn;
    m3_u16          l_as_id = (m3_u16)pusr->user.mtp_user.as_id;
    m3_u16          as_id = (m3_u16)le_id;
    m3_u16          sg_id = (m3_u16)le_id;
    m3_u32          sp_id = pusr->sp_id;

    *p_numsp = 0;
    iu_log_debug("le_id = %d, sp_id = %d\n",le_id, sp_id);
    if (M3_TRUE == M3_IS_LE_AS(le_id) && M3_TRUE == M3_IS_SP_ASP(sp_id)) {
    iu_log_debug("rrrrrrrrrrrrrrrrrrrrrrrrrrr   sp_id = %d\n",sp_id);
        pasp = M3_ASP_TABLE(sp_id);
        for (idx = 0; idx < 10; idx++) {
        iu_log_debug("pasp->r_as_inf[%d].asp_list[%d] = %d\n",as_id, idx, pasp->r_as_inf[as_id].asp_list[idx]);
        iu_log_debug("pasp->r_asp_inf[%d].conn_id = %d\n",idx,pasp->r_asp_inf[idx].conn_id);
            if (M3_TRUE == pasp->r_as_inf[as_id].asp_list[idx]   &&
                M3_MAX_U32 != pasp->r_asp_inf[idx].conn_id) {
                iu_log_debug("pasp->r_asp_inf[%d].conn_id = %d\n",idx, pasp->r_asp_inf[idx].conn_id);
                
                pconn = M3_CONN_TABLE(pasp->r_asp_inf[idx].conn_id);

                iu_log_debug("pconn->l_sp_st[%d] = %d, pconn->r_sp_st[%d] = %d\n",l_as_id,pconn->l_sp_st[l_as_id],as_id,pconn->r_sp_st[as_id]);
                if (M3_ASP_ACTIVE == pconn->l_sp_st[l_as_id]     &&
                    M3_ASP_ACTIVE == pconn->r_sp_st[as_id]) {
                    iu_log_debug("pconn->l_sp_st[%d] = %d, pconn->r_sp_st[%d] = %d\n",l_as_id,pconn->l_sp_st[l_as_id],as_id,pconn->r_sp_st[as_id]);
                    p_splist[*p_numsp] = idx;
                    *p_numsp = *p_numsp + 1;
                }
            }
        }
    } 
    else if (M3_TRUE == M3_IS_LE_AS(le_id) && M3_TRUE == M3_IS_SP_SGP(sp_id)) {
        psgp = M3_SGP_TABLE(sp_id);
        for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
            if (M3_TRUE == psgp->r_as_inf[as_id].asp_list[idx]         &&
                M3_MAX_U32 != psgp->r_asp_inf[idx].conn_id) {
                pconn = M3_CONN_TABLE(psgp->r_asp_inf[idx].conn_id);
                if (M3_ASP_ACTIVE == pconn->r_sp_st[as_id]) {
                    p_splist[*p_numsp] = idx;
                    *p_numsp = *p_numsp + 1;
                }
            }
        }
    } 
    else {
        iu_log_debug("kkkkkkkkkkkkkkkkkkk   sp_id = %d\n",sp_id);
        pasp = M3_ASP_TABLE(sp_id);
        for (idx = 0; idx < (M3_SG_TABLE(le_id))->num_sgp; idx++) {
            if (M3_MAX_U32 != pasp->r_sgp_inf[(m3_u16)(M3_SG_TABLE(sg_id))->
                sgp_list[idx]].conn_id) {
                pconn = M3_CONN_TABLE(pasp->r_sgp_inf[(m3_u16)
                    (M3_SG_TABLE(sg_id))->sgp_list[idx]].conn_id);
                if (M3_ASP_ACTIVE == pconn->l_sp_st[l_as_id]) {
                    p_splist[*p_numsp] = (M3_SG_TABLE(as_id))->sgp_list[idx];
                    *p_numsp = *p_numsp + 1;
                }
            }
        }
    }
    return 0;
}

m3_s32   m3uaGetUserForRTLBL(m3_conn_inf_t  *pconn,
                             m3_u32         nw_app,
                             m3_u32         rt_ctx,
                             m3_rt_lbl_t    *p_rtlbl,
                             m3_u16         cic,
                             m3_u16         *p_usrid)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_pc_inf_t        pc_inf = {p_rtlbl->dpc, nw_app};
    m3_usr_inf_t       *pusr;
    m3_local_rt_inf_t  *p_node;
    m3_u16             as_id;
    m3_u32             sp_id = pconn->l_sp_id;

    if (M3_TRUE == M3_IS_SP_SGP(sp_id)) {
        if (M3_MAX_U16 == (M3_SGP_TABLE(sp_id))->user_id) {
            M3ERRNO(EM3_NO_USER);
            return -1;
        }
        pusr = M3_USR_TABLE((M3_SGP_TABLE(sp_id))->user_id);
        if (M3_TRUE == pusr->user.nif_user.a_data) {
            *p_usrid = (M3_SGP_TABLE(sp_id))->user_id;
            return 0;
        }
    }
    if (-1 == m3uaFindLocalRoute(&pc_inf, &p_node)) {
        return -1;
    }
    while (M3_NULL != p_node) {
        if ((M3_USR_TABLE(p_node->user_id))->sp_id == sp_id) {
            if (M3_TRUE == M3_IS_SP_ASP(sp_id)) {
                as_id = (M3_USR_TABLE(p_node->user_id))->user.mtp_user.as_id;
                if (M3_MAX_U32 != rt_ctx) {
                    if (rt_ctx == m3uaGetRCFromAS(pconn, as_id)) {
                        *p_usrid = p_node->user_id;
                        break;
                    }
                } else if (0 == m3uaMatchRKEY(&(M3_AS_TABLE(as_id))->rkey,
                    nw_app, p_rtlbl, cic)) {
                    *p_usrid = p_node->user_id;
                    break;
                }
            } else {
                *p_usrid = p_node->user_id;
                break;
            }
        }
        p_node = p_node->p_same;
    }
    if (M3_NULL == p_node) {
        return -1;
    }
    return 0;
}

void m3uaProcTXR(m3_conn_inf_t *pconn,
                 m3_u32        stream_id,
                 m3_msg_hdr_t  *p_hdr,
                 m3_u8         *p_msg,
                 m3_u32        msglen)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_data_inf_t    data;
    m3_error_inf_t   err;

    M3TRACE(m3uaInMsgTrace, ("Received TXR message\n"));
    if (M3_MSG_TYPE_DATA != p_hdr->msg_type) {
        err.err_code = EM3_UNSUPP_MSG_TYPE;
        err.num_rc = 0;
        err.num_pc = 0;
        err.nw_app = M3_MAX_U32;
        err.diag_len = msglen;
        err.p_diag_inf = p_msg;
        m3ua_ESENDERROR(pconn, &err);
    } else if (-1 != m3uaDecodeDATA(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
        p_msg+M3_MSG_HDR_LEN, &data)) {
        if (-1 != m3uaAssertDATA(pconn, &data, msglen, p_msg)) {
            m3uaTxrInd(pconn, &data);
        }
    }
    return;
}

m3_s32 m3uaNtfyTxr(m3_u16        user_id,
                   m3_data_inf_t *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_user_ntfy_inf_t    *plist = M3_USER_NTFY_TABLE;

    if (M3_MAX_USER_NTFY == plist->n_ntfy_pd) {
        M3ERRNO(EM3_USER_NTFY_TABLE_FULL);
        return -1;
    }
    plist->ntfy_list[plist->n_offset].type = M3_USER_NTFY_TRANSFER;
    plist->ntfy_list[plist->n_offset].param.transfer.user_id = user_id;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.nw_app = pInf->nw_app;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.rtctx = pInf->rtctx;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.crn_id = pInf->crn_id;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.rt_lbl = pInf->rt_lbl;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.prot_data_len =
                                                         pInf->prot_data_len;
    plist->ntfy_list[plist->n_offset].param.transfer.inf.p_prot_data =
                                                         pInf->p_prot_data;
    plist->n_ntfy_pd++;
    plist->n_offset++;
    if (M3_MAX_USER_NTFY == plist->n_offset) {
        plist->n_offset = 0;
    }
    return 0;
}

void m3uaDecodeCIC(m3_u32        sp_id,
                   m3_u32        nw_app,
                   m3_u8         *pprot_data,
                   m3_u16        *pcic)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        nw_app_idx;
    m3_standard_t standard;

    if (M3_MAX_U32 == nw_app) {
        if (M3_TRUE == M3_IS_SP_ASP(sp_id)) {
            nw_app = (M3_ASP_TABLE(sp_id))->def_nwapp;
        } else {
            nw_app = (M3_SGP_TABLE(sp_id))->def_nwapp;
        }
    }
    for (nw_app_idx = 0; nw_app_idx < M3_MAX_NWAPP; nw_app_idx++)
        if (nw_app == (M3_NWAPP_TABLE(nw_app_idx))->nw_app)
            standard = (M3_NWAPP_TABLE(nw_app_idx))->standard;

    switch(standard) {
        case M3_STD_ANSI: {
            *pcic = pprot_data[1] & 0x3F;
            *pcic = (*pcic << 8) | pprot_data[0];
            break;
        }
        case M3_STD_ITU: {
            *pcic = pprot_data[1] & 0x0F;
            *pcic = (*pcic << 8) | pprot_data[0];
            break;
        }
    }
    return;
}

m3_s32 m3uaAssertDATA(m3_conn_inf_t    *pconn,
                      m3_data_inf_t    *pInf,
                      m3_u32           msglen,
                      m3_u8            *p_msg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_error_inf_t    errinf;
    m3_bool_t         asp_status = M3_TRUE;
    m3_u32            as_id;

    if (M3_MAX_U32 != pInf->nw_app    &&
        M3_FALSE == m3uaAssertNwApp(pInf->nw_app)) {
        errinf.err_code = EM3_INV_NA;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = pInf->nw_app;
        errinf.diag_len = msglen;
        errinf.p_diag_inf = p_msg;
        m3ua_ESENDERROR(pconn, &errinf);
        return -1;
    }
    errinf.num_rc = 0;
    if (M3_MAX_U32 != pInf->rtctx) {
        if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
            if (-1 == (as_id = m3uaGetASFromRC(pconn, pInf->rtctx))) {
                errinf.rc_list[errinf.num_rc++] = pInf->rtctx;
            } else {
                asp_status = (M3_ASP_TABLE(pconn->l_sp_id))->as_list[as_id];
                if (M3_MAX_U32 == pInf->nw_app) {
                    pInf->nw_app = (M3_AS_TABLE(as_id))->rkey.nw_app;
                }
            }
        } else {
            if (-1 == (as_id = m3uaGetRASFromRC(pconn, pInf->rtctx))) {
                errinf.rc_list[errinf.num_rc++] = pInf->rtctx;
            } else {
                asp_status = (M3_SGP_TABLE(pconn->l_sp_id))->r_as_inf[as_id].
                             asp_list[(m3_u16)pconn->r_sp_id];
                if (M3_MAX_U32 == pInf->nw_app) {
                    pInf->nw_app = (M3_R_AS_TABLE(as_id))->rkey.nw_app;
                }
            }
        }
        if (M3_FALSE == asp_status) {
            errinf.rc_list[errinf.num_rc++] = pInf->rtctx;
        }
        if (0 != errinf.num_rc) {
            errinf.err_code = EM3_INV_RC;
            errinf.num_pc = 0;
            errinf.nw_app = M3_MAX_U32;
            errinf.diag_len = msglen;
            errinf.p_diag_inf = p_msg;
            m3ua_ESENDERROR(pconn, &errinf);
            return -1;
        }
    }
    return 0;
}

m3_bool_t m3uaAssertASPActiveInSG(m3_asp_inf_t    *pasp,
                                  m3_u32          sg_id)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16            idx;
    m3_u32            conn_id;
    m3_sg_inf_t       *psg = M3_SG_TABLE(sg_id);

    for (idx = 0; idx < psg->num_sgp; idx++) {
        if (M3_MAX_U32 != pasp->r_sgp_inf[(m3_u16)psg->sgp_list[idx]].conn_id) {
            conn_id = pasp->r_sgp_inf[(m3_u16)psg->sgp_list[idx]].conn_id;
            if (M3_ASP_ACTIVE == (M3_CONN_TABLE(conn_id))->l_sp_g_st) {
                return M3_TRUE;
            }
        }
    }
    return M3_FALSE;
}

