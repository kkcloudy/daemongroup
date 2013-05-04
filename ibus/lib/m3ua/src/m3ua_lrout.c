/****************************************************************************
** Description:
** Code for provisioning local routes.
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

#include <m3ua_lrout.h>

m3_s32 m3ua_local_route(m3_u8                 oprn,
                        m3ua_local_route_t    *pInf)
{
    m3_s32         ret;
    m3_usr_inf_t   *pUsr;

    if (M3_NULL == pInf) {
        M3TRACE(m3uaErrorTrace, ("NULL Information container"));
        M3ERRNO(EM3_NULL_PARAM);
        return -1;
    }
    if (M3_FALSE == M3_USRID_VALID(pInf->add.info.user_id)) {
        M3TRACE(m3uaErrorTrace, ("Invalid User:%u", pInf->add.info.user_id));
        M3ERRNO(EM3_INV_USRID);
        return -1;
    }
    pUsr = M3_USR_TABLE(pInf->add.info.user_id);
    if (M3_FALSE == pUsr->e_state) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned User:%u", pInf->add.info.user_id));
        M3ERRNO(EM3_INV_USRID);
        return -1;
    }
    if (M3_FALSE == m3uaAssertNwApp(pInf->add.info.pc_inf.nw_app)) {
        M3TRACE(m3uaErrorTrace, ("Non-provisioned NwApp:%u", pInf->add.info.pc_inf.nw_app));
        M3ERRNO(EM3_INV_NWAPP);
        return -1;
    }
    switch(oprn)
    {
        case M3_ADD: {
            ret = m3uaAddLocalRoute(pInf);
            break;
        }
        case M3_DELETE: {
            ret = m3uaDeleteLocalRoute(pInf);
            break;
        }
    }
    return ret;
}

m3_s32    m3uaAddLocalRoute(m3ua_local_route_t  *p_param)
{
    m3_local_rt_inf_t    *pInf;
    m3_local_rt_inf_t    *p_prev;
    m3_local_rt_inf_t    *p_node;
    m3_local_rt_inf_t    *p_list;
    m3_local_rt_conf_t   *p_rtconf = &p_param->add.info;
    m3_u8                idx = (m3_u8)p_rtconf->pc_inf.ptcode;
    m3_u8                s_idx;

    M3TRACE(m3uaConfigTrace, ("Adding Local route"));
    p_list = M3_LOCAL_ROUTE_TABLE(idx);
    if (M3_FALSE != p_list->e_state) {
        pInf = (m3_local_rt_inf_t *)m3uaGetFreeLocalRoute();
        if (M3_NULL == pInf) {
            return -1;
        }
        if (idx != (m3_u8)p_list->pc_inf.ptcode) {
            m3uaFindLocalRoute(&p_list->pc_inf, &p_node);
            if (p_node == p_list) {
                /* move in the p_diff or main linked list */
                s_idx  = (m3_u8)p_list->pc_inf.ptcode;
                p_node = (M3_LOCAL_ROUTE_TABLE(s_idx))->p_diff;
                p_prev = M3_LOCAL_ROUTE_TABLE(s_idx);
                while (p_node != p_list) {
                    p_prev = p_node;
                    p_node = p_node->p_diff;
                }
                p_prev->p_diff = pInf;
 
                pInf->e_state   = M3_TRUE;
                pInf->pc_inf    = p_list->pc_inf;
                pInf->user_id   = p_list->user_id;
                pInf->p_same    = p_list->p_same;
                pInf->p_diff    = p_list->p_diff;
 
                p_list->e_state   = M3_TRUE; 
                p_list->pc_inf    = p_rtconf->pc_inf;
                p_list->user_id   = p_rtconf->user_id; 
                p_list->p_same    = M3_NULL;
                p_list->p_diff    = M3_NULL;
            } else {
                /* move in the p_same linked list */
                p_prev = p_node;
                p_node = p_node->p_same;
                while (p_node != p_list) {
                    p_prev = p_node;
                    p_node = p_node->p_same;
                }
                p_prev->p_same = pInf;
 
                pInf->e_state   = M3_TRUE;
                pInf->pc_inf    = p_list->pc_inf;
                pInf->user_id   = p_list->user_id;
                pInf->p_same    = p_list->p_same;
                pInf->p_diff    = p_list->p_diff;
 
                p_list->e_state   = M3_TRUE;
                p_list->pc_inf    = p_rtconf->pc_inf;
                p_list->user_id   = p_rtconf->user_id; 
                p_list->p_same    = M3_NULL;
                p_list->p_diff    = M3_NULL;
            }
        } else {
            if (0 == m3uaFindLocalRoute(&(p_rtconf->pc_inf), &p_node)) {
                /* move on the p_same linked list */
                p_prev = p_node;
                p_node = p_node->p_same;
                while (p_node != M3_NULL) {
                    p_prev = p_node;
                    p_node = p_node->p_same; 
                }

                p_prev->p_same = pInf;
 
                pInf->e_state   = M3_TRUE;
                pInf->pc_inf    = p_rtconf->pc_inf;
                pInf->user_id   = p_rtconf->user_id;
                pInf->p_same    = M3_NULL;
                pInf->p_diff    = M3_NULL;
            } else {
                /* move on the p_diff linked list */
                p_prev = p_list;
                p_node = p_prev->p_diff;
                while (p_node != M3_NULL) {
                    p_prev = p_node;
                    p_node = p_node->p_diff; 
                }

                p_prev->p_diff = pInf;
 
                pInf->e_state   = M3_TRUE;
                pInf->pc_inf    = p_rtconf->pc_inf;
                pInf->user_id   = p_rtconf->user_id;
                pInf->p_same    = M3_NULL;
                pInf->p_diff    = M3_NULL;
            }
        }
    } else {
            p_list->e_state   = M3_TRUE; 
            p_list->pc_inf    = p_rtconf->pc_inf;
            p_list->user_id   = p_rtconf->user_id; 
            p_list->p_same    = M3_NULL;
            p_list->p_diff    = M3_NULL;
    }
    return 0;
}

m3_s32    m3uaDeleteLocalRoute(m3ua_local_route_t   *pInf)
{
    m3_local_rt_inf_t    *p_next;
    m3_local_rt_inf_t    *p_node;
    m3_local_rt_tbl_list_t  *p_list = &m3_local_route_table;
    m3_local_rt_inf_t    *p_tbl;
    m3_local_rt_conf_t   *p_rtconf = &pInf->del.info;
    m3_u16               idx = (m3_u8)p_rtconf->pc_inf.ptcode;

    if (M3_MAX_U32 == p_rtconf->pc_inf.ptcode) {
        while (M3_NULL != p_list) {
            p_tbl = p_list->rt_tbl;
            /* remove all the entries which refer to the given user id */
            for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
                if ((M3_TRUE == p_tbl[idx].e_state) && (p_rtconf->user_id ==
                    p_tbl[idx].user_id)) {
                    p_node = &(p_tbl[idx]);

                    /* node from main linked list/last node in sub-main
                       linked list is deleted */
                    if (M3_NULL == p_node->p_same) {
                        while (M3_NULL != p_node->p_diff) {
                            p_next  = p_node->p_diff;
                            *p_node = *(p_node->p_diff);  
                            p_node->p_diff  = p_next;
                            p_node  = p_node->p_diff;
                        }
                        p_node->e_state = M3_FALSE;
                        p_node->p_same  = M3_NULL;
                        p_node->p_diff  = M3_NULL;
                    } else {
                        /* node from main/sub-main linked list is deleted */
                        (p_node->p_same)->p_diff = p_node->p_diff;
                        while (M3_NULL != p_node->p_same) {
                            p_next  = p_node->p_same;
                            *p_node = *(p_node->p_same);  
                            p_node->p_same  = p_next;
                            p_node  = p_node->p_same;
                        }
                        p_node->e_state = M3_FALSE;
                        p_node->p_same  = M3_NULL;
                        p_node->p_diff  = M3_NULL;
                    }
                }
            }
            p_list = p_list->p_next;
        }
    } else {
        if (0 == m3uaFindLocalRoute(&(p_rtconf->pc_inf), &p_node)) {
            while ((M3_NULL != p_node) && (p_rtconf->user_id !=
                   p_node->user_id)) {
                p_node = p_node->p_same;
            }
            if (M3_NULL != p_node) {
                if (M3_NULL == p_node->p_same) {
                    while (M3_NULL != p_node->p_diff) {
                        p_next  = p_node->p_diff;
                        *p_node = *(p_node->p_diff);
                        p_node->p_diff  = p_next;
                        p_node  = p_node->p_diff;
                    }
                    p_node->e_state = M3_FALSE;
                    p_node->p_same  = M3_NULL;
                    p_node->p_diff  = M3_NULL;
                } else {
                    (p_node->p_same)->p_diff = p_node->p_diff;
                    while (M3_NULL != p_node->p_same) {
                        p_next  = p_node->p_same;
                        *p_node = *(p_node->p_same);  
                        p_node->p_same  = p_next;
                        p_node  = p_node->p_same;
                    }
                    p_node->e_state = M3_FALSE;
                    p_node->p_same  = M3_NULL;
                    p_node->p_diff  = M3_NULL;
                }
            }
        }
    }
    /* adjust the local routing table and free a block if required */
    m3uaAdjustLocalRTTBL();
    return 0;
}

m3_s32    m3uaFindLocalRoute(m3_pc_inf_t         *p_pcinf,
                             m3_local_rt_inf_t   **p_rtinf)
{
    m3_local_rt_inf_t    *p_node = M3_NULL;
    m3_local_rt_inf_t    *pInf = M3_NULL;
    m3_u8                idx = (m3_u8)p_pcinf->ptcode;
    m3_bool_t            e_found = M3_FALSE;

    pInf = M3_LOCAL_ROUTE_TABLE(idx);
    if (M3_TRUE == pInf->e_state || idx == (m3_u8)pInf->pc_inf.ptcode) {
        p_node = pInf;
        while ((p_node->pc_inf.ptcode  != p_pcinf->ptcode) ||
               (p_node->pc_inf.nw_app != p_pcinf->nw_app)) {
            p_node = p_node->p_diff;
            if (M3_NULL == p_node) {
                break;
            }
        }
        if (M3_NULL != p_node) {
            e_found = M3_TRUE;
        }
    }
    if (M3_FALSE == e_found) {
        return -1;
    }
    *p_rtinf = p_node;
    return 0;
}

void *   m3uaGetFreeLocalRoute(void)
{
    m3_local_rt_inf_t    *p_node;
    m3_local_rt_tbl_list_t  *p_list = &m3_local_route_table;
    m3_local_rt_inf_t    *p_tbl;
    m3_u16               idx;
    m3_bool_t            e_found = M3_FALSE;

    while ((M3_NULL != p_list) && (M3_TRUE != e_found)) {
        p_tbl = p_list->rt_tbl;
        for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
            if (M3_FALSE == p_tbl[idx].e_state) {
                p_node  = &(p_tbl[idx]);
                e_found = M3_TRUE;
                break;
            }
        }
        p_list = p_list->p_next;
    }
    if (M3_TRUE != e_found) {
        p_list = &m3_local_route_table;
        while (M3_NULL != p_list->p_next) {
            p_list = p_list->p_next;
        }
        p_list->p_next = (struct m3_local_rt_tbl_t*)
            M3_MALLOC(sizeof(struct m3_local_rt_tbl_t));
        if (M3_NULL == p_list->p_next) {
            /* report error */
            M3TRACE(m3uaErrorTrace, ("No Free local route available"));
            p_node = M3_NULL;
        } else {
            p_list = p_list->p_next;
            m3uaInitLocalRTListNode(p_list);
            p_node = &(p_list->rt_tbl[0]);
        }
    }
    return (void *)p_node;
}

void    m3uaAdjustLocalRTTBL(void)
{
    m3_u32                  n_free = 0;
    m3_u32                  n_occp = 0;
    m3_local_rt_tbl_list_t  *p_list = &m3_local_route_table;
    m3_local_rt_tbl_list_t  *p_prev;
    m3_local_rt_inf_t       *p_tbl;
    m3_u16                  idx;
    m3ua_local_route_t      inf;
    
    if (M3_NULL == p_list->p_next) {
        return;
    }
    while (M3_NULL != p_list->p_next) {
        p_tbl = p_list->rt_tbl;
        for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
            if (M3_FALSE == p_tbl[idx].e_state) {
                n_free++;
            }
        }
        p_prev = p_list;
        p_list = p_list->p_next;
    }
    p_tbl = p_list->rt_tbl;
    /* calculate number of occupied blocks in the last node */
    for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
        if (M3_TRUE == p_tbl[idx].e_state) {
            n_occp++;
        }
    }
    if (n_occp <= n_free) {
        for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
            if (M3_TRUE == p_tbl[idx].e_state) {
                inf.del.info.pc_inf = p_tbl[idx].pc_inf;
                inf.del.info.user_id = p_tbl[idx].user_id;
                m3uaDeleteLocalRoute(&inf);
                inf.add.info.pc_inf = p_tbl[idx].pc_inf;
                inf.add.info.user_id = p_tbl[idx].user_id;
                m3uaAddLocalRoute(&inf);
            }
        }
        M3_FREE(p_list);
        p_prev->p_next = M3_NULL;
    }
    return;
}

void    m3uaInitLocalRTListNode(m3_local_rt_tbl_list_t *p_list)
{
    m3_u32                idx;

    for (idx = 0; idx < M3_MAX_LOCAL_ROUTE; idx++) {
        p_list->rt_tbl[idx].e_state  = M3_FALSE;
        p_list->rt_tbl[idx].p_same   = M3_NULL;
        p_list->rt_tbl[idx].p_diff   = M3_NULL;
    }
    p_list->p_next = M3_NULL;
    return;
}

