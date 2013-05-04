/****************************************************************************
** Description:
** Code for provisioning SS7 route.
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

#include <m3ua_route.h>

m3_s32 m3ua_route(m3_u8           oprn,
                  m3ua_route_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32     ret;

    if (M3_NULL == pInf) {
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddRoute(pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteRoute(pInf);
            break;
        }
    }
    return ret;
}

m3_s32 m3uaAddRoute(m3ua_route_t    *pInf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_r_as_inf_t    *pRAs;
    m3_sg_inf_t      *pSg;
    m3_rt_inf_t      *pNew; /* new allocated node */
    m3_rt_inf_t      *pList; /* root node in the list for ptcode+nwapp */
    m3_rt_inf_t      *p_prev; /* previous node */
    m3_rt_inf_t      *pNode; /* for general purpose */
    m3_u8            idx = (m3_u8)(pInf->add.info.pc_inf.ptcode);//idx = msc pointcode or sgsn pointcode
    m3_u8            s_idx;
    iu_log_debug("pInf->add.info.pc_inf.ptcode = %d\n",pInf->add.info.pc_inf.ptcode);
iu_log_debug("<<<<<<<<<<<m3uaAddRoute M3_ROUTE_TABLE(idx  %d))->e_state %d,\n", idx, (M3_ROUTE_TABLE(idx))->e_state);

    if (M3_FALSE == m3uaAssertNwApp(pInf->add.info.pc_inf.nw_app)) {
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }

    if (M3_TRUE == M3_IS_LE_AS(pInf->add.info.le_id)) {
        if (M3_FALSE == M3_R_ASID_VALID(pInf->add.info.le_id)) {
            M3ERRNO(EM3_INV_R_ASID);
            return -1;
        }
        pRAs = M3_R_AS_TABLE(pInf->add.info.le_id);
        if (M3_FALSE == pRAs->e_state) {
            M3ERRNO(EM3_INV_R_ASID);
            return -1;
        }
		iu_log_debug("$$ remote as idx is %d\n", pInf->add.info.le_id);
    } else if (M3_TRUE == M3_IS_LE_SG(pInf->add.info.le_id)) {
        if (M3_FALSE == M3_SGID_VALID(pInf->add.info.le_id)) {
            M3ERRNO(EM3_INV_SGID);
            return -1;
        }
        pSg = M3_SG_TABLE(pInf->add.info.le_id);
        if (M3_FALSE == pSg->e_state) {
            M3ERRNO(EM3_INV_SGID);
            return -1;
        }
        //DEC2009
        if (-1 == m3uaSGAddRoute(pSg, &(pInf->add.info.pc_inf))) {
            return -1;
        }
    } else {
        M3ERRNO(EM3_INV_LEID);
        return -1;
    }

	/*get route table is plist idx is msc or sgsn point code*/
    pList = M3_ROUTE_TABLE(idx);
    iu_log_debug("idx is %d pointcode is %d\n", idx, pInf->add.info.pc_inf.ptcode);
	
    if (M3_FALSE != pList->e_state) {
        pNew = (m3_rt_inf_t *)m3uaGetFreeRoute();
        if (M3_NULL == pNew) {
            return -1;
        }
        if (idx != (m3_u8)(pList->pc_inf.ptcode)) {
            m3uaFindRoute(&(pList->pc_inf), &pNode);
            if (pNode == pList) {
                s_idx  = (m3_u8)pList->pc_inf.ptcode;
                p_prev = M3_ROUTE_TABLE(s_idx);
                pNode = p_prev->p_diff;
                while (pNode != pList) {
                    p_prev = pNode;
                    pNode = pNode->p_diff;
                }
                p_prev->p_diff = pNew;

                pNew->e_state   = M3_TRUE;
                pNew->pc_inf    = pList->pc_inf;
                pNew->le_id     = pList->le_id;
                pNew->priority  = pList->priority;
                pNew->rtstate   = pList->rtstate;
                pNew->p_same    = pList->p_same;
                pNew->p_diff    = pList->p_diff;

                pList->e_state   = M3_TRUE;
                pList->pc_inf    = pInf->add.info.pc_inf;
                pList->le_id     = pInf->add.info.le_id;
                pList->priority  = pInf->add.info.priority;
                pList->rtstate   = M3_RTSTATE_DOWN; //OCT2009
                pList->p_same    = M3_NULL;
                pList->p_diff    = M3_NULL;
            } else {
                p_prev = pNode;
                pNode = pNode->p_same;
                while (pNode != pList) {
                    p_prev = pNode;
                    pNode = pNode->p_same;
                }
                p_prev->p_same = pNew;

                pNew->e_state   = M3_TRUE;
                pNew->pc_inf    = pList->pc_inf;
                pNew->le_id     = pList->le_id;
                pNew->priority  = pList->priority;
                pNew->rtstate   = pList->rtstate;
                pNew->p_same    = pList->p_same;
                pNew->p_diff    = pList->p_diff;

                pList->e_state   = M3_TRUE;
                pList->pc_inf    = pInf->add.info.pc_inf;
                pList->le_id     = pInf->add.info.le_id;
                pList->priority  = pInf->add.info.priority;
                pList->rtstate   = M3_RTSTATE_DOWN; //OCT2009
                pList->p_same    = M3_NULL;
                pList->p_diff    = M3_NULL;
            }
        } else {
            if (0 == m3uaFindRoute(&pInf->add.info.pc_inf, &pNode)) {
                p_prev = pNode;
                pNode = pNode->p_same;
                while (pNode != M3_NULL) {
                    p_prev = pNode;
                    pNode = pNode->p_same;
                }
                p_prev->p_same = pNew;

                pNew->e_state   = M3_TRUE;
                pNew->pc_inf    = pInf->add.info.pc_inf;
                pNew->le_id     = pInf->add.info.le_id;
                pNew->priority  = pInf->add.info.priority;
                pNew->rtstate   = M3_RTSTATE_DOWN; //OCT2009
                pNew->p_same    = M3_NULL;
                pNew->p_diff    = M3_NULL;
            } else {
                /* move on the p_diff linked list */
                p_prev = pList;
                pNode = p_prev->p_diff;
                while (pNode != M3_NULL) {
                    p_prev = pNode;
                    pNode = pNode->p_diff;
                }
                p_prev->p_diff   = pNew;

                pNew->e_state   = M3_TRUE;
                pNew->pc_inf    = pInf->add.info.pc_inf;
                pNew->le_id     = pInf->add.info.le_id;
                pNew->priority  = pInf->add.info.priority;
                pNew->rtstate   = M3_RTSTATE_DOWN;//OCT2009
                pNew->p_same    = M3_NULL;
                pNew->p_diff    = M3_NULL;
            }
        }
    } else {

		iu_log_debug("<========= add route here ===========>\n");
        pList->e_state   = M3_TRUE;
        pList->pc_inf    = pInf->add.info.pc_inf;
        pList->le_id     = pInf->add.info.le_id;
        pList->priority  = pInf->add.info.priority;
        pList->rtstate   = M3_RTSTATE_DOWN;//OCT2009
        pList->p_same    = M3_NULL;
        pList->p_diff    = M3_NULL;
    }
    m3uaSortRouteTBL(&pInf->add.info.pc_inf);
    return 0;
}

m3_s32 m3uaDeleteRoute(m3ua_route_t    *pInf)
{

    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_r_as_inf_t    *pRAs;
    m3_sg_inf_t      *pSg;
    m3_rt_tbl_list_t *pList = &m3_route_table; /* pointer to list */
    m3_rt_inf_t      *p_next; /* next node */
    m3_rt_inf_t      *pNode; /* for general purpose */
    m3_rt_inf_t      *p_tbl;
    m3_u16           idx = (m3_u8)pInf->del.info.pc_inf.ptcode;

    //if (M3_FALSE == m3uaAssertNwApp(pInf->del.info.pc_inf.nw_app)) {
    //    M3ERRNO(EM3_INV_NWAPP);
    //    return -1;
    //}
    if (M3_TRUE == M3_IS_LE_AS(pInf->del.info.le_id)) {
        if (M3_FALSE == M3_R_ASID_VALID(pInf->del.info.le_id)) {
            M3ERRNO(EM3_INV_R_ASID);
            return -1;
        }
        pRAs = M3_R_AS_TABLE(pInf->del.info.le_id);
        if (M3_FALSE == pRAs->e_state) {
            M3ERRNO(EM3_INV_R_ASID);
            return -1;
        }
    } else if (M3_TRUE == M3_IS_LE_SG(pInf->del.info.le_id)) {
        if (M3_FALSE == M3_SGID_VALID(pInf->del.info.le_id)) {
            M3ERRNO(EM3_INV_SGID);
            return -1;
        }
        pSg = M3_SG_TABLE(pInf->del.info.le_id);
        if (M3_FALSE == pSg->e_state) {
            M3ERRNO(EM3_INV_SGID);
            return -1;
        }
        //DEC2009
        if (-1 == m3uaSGDeleteRoute(pSg, &(pInf->del.info.pc_inf))) {
            return -1;
        }
    } else {
        M3ERRNO(EM3_INV_LEID);
        return -1;
    }
    if (M3_MAX_U32 == pInf->del.info.pc_inf.ptcode) {
        while (M3_NULL != pList) {
            p_tbl = pList->rt_tbl;
            for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
                if ((M3_TRUE == p_tbl[idx].e_state)                     &&
                    (pInf->del.info.le_id == p_tbl[idx].le_id)   &&
                    (M3_MAX_U8 == pInf->del.info.priority        ||
                     pInf->del.info.priority == p_tbl[idx].priority)) {
                    pNode = &(p_tbl[idx]);
                    /* node from main linked list/last node in sub-main
                       linked list is deleted */
                    if (M3_NULL == pNode->p_same) {
                        while (M3_NULL != pNode->p_diff) {
                            p_next  = pNode->p_diff;
                            *pNode = *(pNode->p_diff);
                            pNode->p_diff  = p_next;
                            pNode  = pNode->p_diff;
                        }
                        pNode->e_state = M3_FALSE;
                        pNode->p_same  = M3_NULL;
                        pNode->p_diff  = M3_NULL;
                    } else {
                        /* node from main/sub-main linked list is deleted */
                        (pNode->p_same)->p_diff = pNode->p_diff;
                        while (M3_NULL != pNode->p_same) {
                            p_next  = pNode->p_same;
                            *pNode = *(pNode->p_same);
                            pNode->p_same  = p_next;
                            pNode  = pNode->p_same;
                        }
                        pNode->e_state = M3_FALSE;
                        pNode->p_same  = M3_NULL;
                        pNode->p_diff  = M3_NULL;
                    }
                }
            }
            pList = pList->p_next;
        }
    } else {
        if (0 == m3uaFindRoute(&(pInf->del.info.pc_inf), &pNode)) {
            while ((M3_NULL != pNode)                              &&
                   (pInf->del.info.le_id != pNode->le_id)   &&
                   (M3_MAX_U8 == pInf->del.info.priority     ||
                    pInf->del.info.priority == pNode->priority)) {
                pNode = pNode->p_same;
            }
            if (M3_NULL != pNode) {
                if (M3_NULL == pNode->p_same) {
                    while (M3_NULL != pNode->p_diff) {
                        p_next  = pNode->p_diff;
                        *pNode = *(pNode->p_diff);
                        pNode->p_diff  = p_next;
                        pNode  = pNode->p_diff;
                    }
                    pNode->e_state = M3_FALSE;
                    pNode->p_same  = M3_NULL;
                    pNode->p_diff  = M3_NULL;
                } else {
                    (pNode->p_same)->p_diff = pNode->p_diff;
                    while (M3_NULL != pNode->p_same) {
                        p_next  = pNode->p_same;
                        *pNode = *(pNode->p_same);
                        pNode->p_same  = p_next;
                        pNode  = pNode->p_same;
                    }
                    pNode->e_state = M3_FALSE;
                    pNode->p_same  = M3_NULL;
                    pNode->p_diff  = M3_NULL;
                }
            }
        }
    }
    /* adjust the local routing table and free a block if required */
    m3uaAdjustRTTBL();
    return 0;
}

m3_s32    m3uaFindRoute(m3_pc_inf_t   *p_pcinf,
                        m3_rt_inf_t   **p_rtinf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("p_pcinf->ptcode = %d\n p_pcinf->nw_app = %d\n",p_pcinf->ptcode,p_pcinf->nw_app);
    m3_rt_inf_t    *pNode = M3_NULL;
    m3_u8          idx = (m3_u8)p_pcinf->ptcode; //msc pointcode or sgsn pointcode 
    m3_bool_t      e_found = M3_FALSE;
iu_log_debug("<<<<<<<<<<<m3uaFindRoute M3_ROUTE_TABLE(idx  %d))->e_state %d,\n", idx, (M3_ROUTE_TABLE(idx))->e_state);
iu_log_debug("<<<<<<<<<<<m3uaFindRoute (m3_u8)(M3_ROUTE_TABLE(idx))->pc_inf.ptcode  %d\n", (m3_u8)(M3_ROUTE_TABLE(idx))->pc_inf.ptcode);
    if ((M3_TRUE == (M3_ROUTE_TABLE(idx))->e_state) &&
        (idx == (m3_u8)(M3_ROUTE_TABLE(idx))->pc_inf.ptcode)) { 

		/*get route struct*/
        pNode = M3_ROUTE_TABLE(idx);
		
iu_log_debug("<<<<<<<<<<<m3uaFindRoute pNode->pc_inf.ptcode %d, p_pcinf->ptcode %d\n", pNode->pc_inf.ptcode, p_pcinf->ptcode);
iu_log_debug("<<<<<<<<<<<m3uaFindRoute pNode->pc_inf.nw_app %d, p_pcinf->nw_app %d\n", pNode->pc_inf.nw_app, p_pcinf->nw_app);

//		while ((pNode->pc_inf.ptcode  != p_pcinf->ptcode) ||
//               (pNode->pc_inf.nw_app  != p_pcinf->nw_app)) {
       int i = 0;
       while (pNode->pc_inf.ptcode  != p_pcinf->ptcode) {
            pNode = pNode->p_diff;
            if (M3_NULL == pNode) {
            iu_log_debug("pNode == NULL\n");
                break;
            }
        }
        if (M3_NULL != pNode) {
            e_found = M3_TRUE;
        }
    }
    if (M3_FALSE == e_found) {
		iu_log_debug("e_found is false\n");
        return -1;
    }
    *p_rtinf = pNode;
    return 0;
}

void *   m3uaGetFreeRoute(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_rt_inf_t    *pNode;
    m3_rt_tbl_list_t  *pList = &m3_route_table;
    m3_rt_inf_t    *p_tbl;
    m3_u16         idx;
    m3_bool_t      e_found = M3_FALSE;

    while ((M3_NULL != pList) && (M3_TRUE != e_found)) {
        p_tbl = pList->rt_tbl;
        for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
            if (M3_FALSE == p_tbl[idx].e_state) {
                pNode  = &(p_tbl[idx]);
                e_found = M3_TRUE;
                break;
            }
        }
        if (M3_TRUE != e_found)
            pList = pList->p_next;
    }
    if (M3_TRUE != e_found) {
        pList = &m3_route_table;
        while (M3_NULL != pList->p_next) {
            pList = pList->p_next;
        }
        pList->p_next = (m3_rt_tbl_list_t*)M3_MALLOC(sizeof(m3_rt_tbl_list_t));
        if (M3_NULL == pList->p_next) {
            pNode = M3_NULL;
        } else {
            pList = pList->p_next;
            m3uaInitRTListNode(pList);
            pNode = &(pList->rt_tbl[0]);
        }
    }
    return (void *)pNode;
}

void    m3uaAdjustRTTBL(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u32         n_free = 0;
    m3_u32         n_occp = 0;
    m3_rt_tbl_list_t  *pList = &m3_route_table;
    m3_rt_tbl_list_t  *p_prev;
    m3_rt_inf_t    *p_tbl;
    m3ua_route_t   inf;
    m3_u16         idx;

    if (M3_NULL == pList->p_next) {
        return;
    }
    while (M3_NULL != pList->p_next) {
        p_tbl = pList->rt_tbl;
        for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
            if (M3_FALSE == p_tbl[idx].e_state) {
                n_free++;
            }
        }
        p_prev = pList;
        pList = pList->p_next;
    }
    p_tbl = pList->rt_tbl;
    /* calculate number of occupied blocks in the last node */
    for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
        if (M3_TRUE == p_tbl[idx].e_state) {
            n_occp++;
        }
    }
    if (n_occp <= n_free) {
        for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
            if (M3_TRUE == p_tbl[idx].e_state) {
                inf.del.info.pc_inf = p_tbl[idx].pc_inf;
                inf.del.info.le_id  = p_tbl[idx].le_id;
                inf.del.info.priority = p_tbl[idx].priority;
                m3uaDeleteRoute(&inf);
                m3uaAddRoute(&inf);
            }
        }
        M3_FREE(pList);
        p_prev->p_next = M3_NULL;
    }
    return;
}

void    m3uaInitRTListNode(m3_rt_tbl_list_t  *pList)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u32                idx;

    for (idx = 0; idx < M3_MAX_ROUTE; idx++) {
        pList->rt_tbl[idx].e_state  = M3_FALSE;
        pList->rt_tbl[idx].p_same   = M3_NULL;
        pList->rt_tbl[idx].p_diff   = M3_NULL;
    }
    pList->p_next = M3_NULL;
    return;
}

void   m3uaSortRouteTBL(m3_pc_inf_t    *p_pcinf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_rt_inf_t           *pFrst = NULL;
    m3_rt_inf_t           *pNode = NULL, *p_prev = NULL;
    m3_rt_inf_t           *pLast = NULL, *pSLast = NULL;
    m3_rt_inf_t           tmp;

    /* route is added at the end of the list */
    m3uaFindRoute(p_pcinf, &pFrst);
    p_prev = pFrst;
    pNode = pFrst;
    while (M3_NULL != pNode->p_same) {
        p_prev = pNode;
        pNode = pNode->p_same;
    }
    pLast = pNode;
    pSLast = p_prev;
    pNode = pFrst;
    while ((M3_NULL != pNode->p_same) && (pLast->priority <=
           pNode->priority)) {
        p_prev = pNode;
        pNode = pNode->p_same;
    }
    if (M3_NULL != pNode->p_same) {
        if (pNode->p_same != pLast) {
            p_prev->p_same = pLast;
            pLast->p_same = pNode;
            pSLast->p_same = M3_NULL;
        } else {
            tmp = *pNode;
            *pNode = *pLast;
            *pLast = tmp;
            pLast->p_diff = M3_NULL;
            pLast->p_same = M3_NULL;
            pNode->p_diff = tmp.p_diff;
            pNode->p_same = tmp.p_same;
        }
    }
    return;
}

m3_s32    m3uaSetRTState(m3_pc_inf_t    *p_pcinf,
                         m3_u32         le_id,
                         m3_u8          priority,
                         m3_rtstate_t   rtstate)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_rt_inf_t    *pNode;
    m3_s32         ret = 0;
    m3_rtstate_t   final_state = M3_RTSTATE_DOWN;
    m3_rtstate_t   prev_state = M3_RTSTATE_DOWN;

    if (-1 == m3uaFindRoute(p_pcinf, &pNode)) {
        ret = -1;
    } else {
        while (M3_NULL != pNode) {
            switch(pNode->rtstate) {
                case M3_RTSTATE_UP: {
                    prev_state = M3_RTSTATE_UP;
                    break;
                }
                case M3_RTSTATE_RESTRICTED: {
                    if (M3_RTSTATE_UP != prev_state)
                        prev_state = M3_RTSTATE_RESTRICTED;
                    break;
                }
                case M3_RTSTATE_CONG_1: {
                    if (M3_RTSTATE_UP != prev_state               &&
                        M3_RTSTATE_RESTRICTED != prev_state)
                        prev_state = M3_RTSTATE_CONG_1;
                    break;
                }
                case M3_RTSTATE_CONG_2: {
                    if (M3_RTSTATE_UP != prev_state               &&
                        M3_RTSTATE_RESTRICTED != prev_state       &&
                        M3_RTSTATE_CONG_1 != prev_state)
                        prev_state = M3_RTSTATE_CONG_2;
                    break;
                }
                case M3_RTSTATE_CONG_3: {
                    if (M3_RTSTATE_UP != prev_state               &&
                        M3_RTSTATE_RESTRICTED != prev_state       &&
                        M3_RTSTATE_CONG_1 != prev_state           &&
                        M3_RTSTATE_CONG_2 != prev_state)
                        prev_state = M3_RTSTATE_CONG_3;
                    break;
                }
                case M3_RTSTATE_DOWN: {
                    if (M3_RTSTATE_UP != prev_state               &&
                        M3_RTSTATE_RESTRICTED != prev_state       &&
                        M3_RTSTATE_CONG_1 != prev_state           &&
                        M3_RTSTATE_CONG_2 != prev_state           &&
                        M3_RTSTATE_CONG_3 != prev_state)
                        prev_state = M3_RTSTATE_DOWN;
                    break;
                }
            }
            if ((le_id == pNode->le_id)        &&
                (M3_MAX_U8 == priority          ||
                 priority == pNode->priority)) {
                pNode->rtstate = rtstate;
            }
            switch(pNode->rtstate) {
                case M3_RTSTATE_UP: {
                    final_state = M3_RTSTATE_UP;
                    break;
                }
                case M3_RTSTATE_RESTRICTED: {
                    if (M3_RTSTATE_UP != final_state)
                        final_state = M3_RTSTATE_RESTRICTED;
                    break;
                }
                case M3_RTSTATE_CONG_1: {
                    if (M3_RTSTATE_UP != final_state               &&
                        M3_RTSTATE_RESTRICTED != final_state)
                        final_state = M3_RTSTATE_CONG_1;
                    break;
                }
                case M3_RTSTATE_CONG_2: {
                    if (M3_RTSTATE_UP != final_state               &&
                        M3_RTSTATE_RESTRICTED != final_state       &&
                        M3_RTSTATE_CONG_1 != final_state)
                        final_state = M3_RTSTATE_CONG_2;
                    break;
                }
                case M3_RTSTATE_CONG_3: {
                    if (M3_RTSTATE_UP != final_state               &&
                        M3_RTSTATE_RESTRICTED != final_state       &&
                        M3_RTSTATE_CONG_1 != final_state           &&
                        M3_RTSTATE_CONG_2 != final_state)
                        final_state = M3_RTSTATE_CONG_3;
                    break;
                }
                case M3_RTSTATE_DOWN: {
                    if (M3_RTSTATE_UP != final_state               &&
                        M3_RTSTATE_RESTRICTED != final_state       &&
                        M3_RTSTATE_CONG_1 != final_state           &&
                        M3_RTSTATE_CONG_2 != final_state           &&
                        M3_RTSTATE_CONG_3 != final_state)
                        final_state = M3_RTSTATE_DOWN;
                    break;
                }
            }
            pNode = pNode->p_same;
        }
        if (final_state != prev_state) {
            switch(final_state) {
                /* indicate to all the users abt new rtstate */
                case M3_RTSTATE_UP:
                case M3_RTSTATE_RESTRICTED: {
                    m3uaNtfyResume(p_pcinf->nw_app, p_pcinf->ptcode);
                    break;
                }
                case M3_RTSTATE_CONG_1: {
                    m3uaNtfyStatus(p_pcinf->nw_app, p_pcinf->ptcode,
                        M3_MAX_U32, M3_CAUSE_CONG, 1);
                    break;
                }
                case M3_RTSTATE_CONG_2: {
                    m3uaNtfyStatus(p_pcinf->nw_app, p_pcinf->ptcode,
                        M3_MAX_U32, M3_CAUSE_CONG, 2);
                    break;
                }
                case M3_RTSTATE_CONG_3: {
                    m3uaNtfyStatus(p_pcinf->nw_app, p_pcinf->ptcode,
                        M3_MAX_U32, M3_CAUSE_CONG, 3);
                    break;
                }
                case M3_RTSTATE_DOWN: {
                    m3uaNtfyPause(p_pcinf->nw_app, p_pcinf->ptcode);
                    break;
                }
            }
        }
    }
    return ret; 
}

m3_bool_t m3uaAssertRoute(m3_pc_inf_t    *p_pcinf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_rt_inf_t    *pNode;
    m3_bool_t      ret = M3_TRUE;

    if (-1 == m3uaFindRoute(p_pcinf, &pNode)) {
        ret = M3_FALSE;
    }
    return ret;
}

void m3uaSetRTStatex(m3_pc_inf_t    pcinf,
                     m3_u32         le_id,
                     m3_u8          priority,
                     m3_u32         mask,
                     m3_rtstate_t   rtstate)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;
    m3_rt_inf_t   *pNode = M3_NULL;
    m3_rt_inf_t   *pNdSm = M3_NULL; /* same ptcode+nw_app node */
    //m3_rtstate_t  final_state = rtstate; -- Dancis Apr2006
    //m3_rtstate_t  prev_state = rtstate; -- Dancis Apr2006

    for (idx = 0; idx < 256; idx++) {
        pNode = M3_ROUTE_TABLE(idx);
        while (M3_NULL != pNode) {
            if ((M3_TRUE == pNode->e_state)                          &&
                (pcinf.nw_app == pNode->pc_inf.nw_app)               &&
                (pcinf.ptcode == (pNode->pc_inf.ptcode & mask))) {
                pNdSm = pNode;
                m3_rtstate_t  final_state = M3_RTSTATE_DOWN;//DANCIS - Apr2006
                m3_rtstate_t  prev_state = M3_RTSTATE_DOWN; //DANCIS - Apr2006
                while (M3_NULL != pNdSm) {
                    switch(pNdSm->rtstate) {
                        case M3_RTSTATE_UP: {
                            prev_state = M3_RTSTATE_UP;
                            break;
                        }
                        case M3_RTSTATE_RESTRICTED: {
                            if (M3_RTSTATE_UP != prev_state)
                                prev_state = M3_RTSTATE_RESTRICTED;
                            break;
                        }
                        case M3_RTSTATE_CONG_1: {
                            if (M3_RTSTATE_UP != prev_state               &&
                                M3_RTSTATE_RESTRICTED != prev_state)
                                prev_state = M3_RTSTATE_CONG_1;
                            break;
                        }
                        case M3_RTSTATE_CONG_2: {
                            if (M3_RTSTATE_UP != prev_state               &&
                                M3_RTSTATE_RESTRICTED != prev_state       &&
                                M3_RTSTATE_CONG_1 != prev_state)
                                prev_state = M3_RTSTATE_CONG_2;
                            break;
                        }
                        case M3_RTSTATE_CONG_3: {
                            if (M3_RTSTATE_UP != prev_state               &&
                                M3_RTSTATE_RESTRICTED != prev_state       &&
                                M3_RTSTATE_CONG_1 != prev_state           &&
                                M3_RTSTATE_CONG_2 != prev_state)
                                prev_state = M3_RTSTATE_CONG_3;
                            break;
                        }
                        case M3_RTSTATE_DOWN: {
                            if (M3_RTSTATE_UP != prev_state               &&
                                M3_RTSTATE_RESTRICTED != prev_state       &&
                                M3_RTSTATE_CONG_1 != prev_state           &&
                                M3_RTSTATE_CONG_2 != prev_state           &&
                                M3_RTSTATE_CONG_3 != prev_state)
                                prev_state = M3_RTSTATE_DOWN;
                            break;
                        }
                    }
                    if ((le_id == pNdSm->le_id)              &&
                        (priority == M3_MAX_U8               ||
                         priority == pNode->priority)) {
                        pNdSm->rtstate = rtstate;
                    }
                    switch(pNdSm->rtstate) {
                        case M3_RTSTATE_UP: {
                            final_state = M3_RTSTATE_UP;
                            break;
                        }
                        case M3_RTSTATE_RESTRICTED: {
                            if (M3_RTSTATE_UP != final_state)
                                final_state = M3_RTSTATE_RESTRICTED;
                            break;
                        }
                        case M3_RTSTATE_CONG_1: {
                            if (M3_RTSTATE_UP != final_state               &&
                                M3_RTSTATE_RESTRICTED != final_state)
                                final_state = M3_RTSTATE_CONG_1;
                            break;
                        }
                        case M3_RTSTATE_CONG_2: {
                            if (M3_RTSTATE_UP != final_state               &&
                                M3_RTSTATE_RESTRICTED != final_state       &&
                                M3_RTSTATE_CONG_1 != final_state)
                                final_state = M3_RTSTATE_CONG_2;
                            break;
                        }
                        case M3_RTSTATE_CONG_3: {
                            if (M3_RTSTATE_UP != final_state               &&
                                M3_RTSTATE_RESTRICTED != final_state       &&
                                M3_RTSTATE_CONG_1 != final_state           &&
                                M3_RTSTATE_CONG_2 != final_state)
                                final_state = M3_RTSTATE_CONG_3;
                            break;
                        }
                        case M3_RTSTATE_DOWN: {
                            if (M3_RTSTATE_UP != final_state               &&
                                M3_RTSTATE_RESTRICTED != final_state       &&
                                M3_RTSTATE_CONG_1 != final_state           &&
                                M3_RTSTATE_CONG_2 != final_state           &&
                                M3_RTSTATE_CONG_3 != final_state)
                                final_state = M3_RTSTATE_DOWN;
                            break;
                        }
                    }
                    pNdSm = pNdSm->p_same;
                }
                if (final_state != prev_state) {
                    switch(final_state) {
                        /* indicate to all the users abt new rtstate */
                        case M3_RTSTATE_UP:
                        case M3_RTSTATE_RESTRICTED: {
                            m3uaNtfyResume(pNode->pc_inf.nw_app,
                                pNode->pc_inf.ptcode);
                            break;
                        }
                        case M3_RTSTATE_CONG_1: {
                            m3uaNtfyStatus(pNode->pc_inf.nw_app,
                                pNode->pc_inf.ptcode,
                                M3_MAX_U32, M3_CAUSE_CONG, 1);
                            break;
                        }
                        case M3_RTSTATE_CONG_2: {
                            m3uaNtfyStatus(pNode->pc_inf.nw_app,
                                pNode->pc_inf.ptcode,
                                M3_MAX_U32, M3_CAUSE_CONG, 2);
                            break;
                        }
                        case M3_RTSTATE_CONG_3: {
                            m3uaNtfyStatus(pNode->pc_inf.nw_app,
                                pNode->pc_inf.ptcode,
                                M3_MAX_U32, M3_CAUSE_CONG, 3);
                            break;
                        }
                        case M3_RTSTATE_DOWN: {
                            m3uaNtfyPause(pNode->pc_inf.nw_app,
                                pNode->pc_inf.ptcode);
                            break;
                        }
                    }
                }
            }
            pNode = pNode->p_diff;
        }
    }
    return;
}

