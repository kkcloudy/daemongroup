/****************************************************************************
** Description:
** Code for initializing data structures.
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

#include <m3ua_init.h>

m3_asp_inf_t             m3_asp_table[M3_MAX_ASP];
m3_sgp_inf_t             m3_sgp_table[M3_MAX_SGP];
m3_as_inf_t              m3_as_table[M3_MAX_AS];
m3_r_asp_inf_t           m3_r_asp_table[M3_MAX_R_ASP];
m3_r_sgp_inf_t           m3_r_sgp_table[M3_MAX_R_SGP];
m3_r_as_inf_t            m3_r_as_table[M3_MAX_R_AS];
m3_conn_inf_t            m3_conn_table[M3_MAX_CONN];
m3_sg_inf_t              m3_sg_table[M3_MAX_SG];
m3_nwapp_inf_t           m3_nwapp_table[M3_MAX_NWAPP];
m3_usr_inf_t             m3_usr_table[M3_MAX_USR];
m3_local_rt_tbl_list_t   m3_local_route_table;
m3_rt_tbl_list_t         m3_route_table;
m3_mgmt_ntfy_inf_t       m3_mgmt_ntfy_table;
m3_user_ntfy_inf_t       m3_user_ntfy_table;
m3_prot_timer_t          m3_timer_table;


m3_s32 m3ua_init()
{
    M3TRACE(m3uaStartupTrace, ("Initialising M3UA"));
    m3uaInitASP();
    m3uaInitSGP();
    m3uaInitAS();
    m3uaInitRASP();
    m3uaInitRSGP();
    m3uaInitRAS();
    m3uaInitSG();
    m3uaInitConn();
    m3uaInitNwApp();
    m3uaInitUsr();
    m3uaInitTimer();
    m3uaTimerInit();
    m3uaMemInit();
    return 0;
}

m3_s32 m3uaInitASP()
{
    m3_u16          idx, idx2, asIdx;
    m3_asp_inf_t    *pAsp;

    M3TRACE(m3uaStartupTrace, ("Initialising ASP information containers"));
    for (idx = 0; idx < M3_MAX_ASP; idx++) {
        pAsp = M3_ASP_TABLE(idx);
        pAsp->e_state  = M3_FALSE;	/*local asp state false*/
        pAsp->asp_id   = idx;		/*asp idx*/
        for (idx2 = 0; idx2 < M3_MAX_AS; idx2++)
            pAsp->as_list[idx2] = M3_FALSE;/*8 as list is false*/
        for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
            pAsp->r_asp_inf[idx2].conn_id = M3_MAX_U32;/*128 remote asp is -1*/
            for (asIdx = 0; asIdx < M3_MAX_AS; asIdx++) {
                pAsp->r_asp_inf[idx2].rc_inf[asIdx].rtctx = M3_MAX_U32;
                pAsp->r_asp_inf[idx2].rc_inf[asIdx].dyn_reg_state = M3_RK_STATIC;/*In as static register*/
            }
        }
        for (idx2 = 0; idx2 < M3_MAX_R_SGP; idx2++) {	/*32 sgp init*/
            pAsp->r_sgp_inf[idx2].conn_id = M3_MAX_U32;
            for (asIdx = 0; asIdx < M3_MAX_AS; asIdx++) {
                pAsp->r_sgp_inf[idx2].rc_inf[asIdx].rtctx = M3_MAX_U32;
                pAsp->r_sgp_inf[idx2].rc_inf[asIdx].dyn_reg_state = M3_RK_STATIC;
            }
        }
		/*32 remote ac is down*/
        for (asIdx = 0; asIdx < M3_MAX_R_AS; asIdx++) {
            pAsp->r_as_inf[asIdx].pd_q_inf.pd_timer_inf.timer_id = M3_MAX_U32;
            pAsp->r_as_inf[asIdx].pd_q_inf.pd_q_head = M3_NULL;
            pAsp->r_as_inf[asIdx].pd_q_inf.pd_q_tail = M3_NULL;
            pAsp->r_as_inf[asIdx].state = M3_AS_DOWN;
            for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
                pAsp->r_as_inf[asIdx].asp_list[idx2] = M3_FALSE;/*remote asp list false*/
                pAsp->r_as_inf[asIdx].rc_inf[idx2].dyn_entry = M3_FALSE;
                pAsp->r_as_inf[asIdx].rc_inf[idx2].rtctx = M3_MAX_U32;
                pAsp->r_as_inf[asIdx].rc_inf[idx2].rk_id = M3_MAX_U32;
                pAsp->r_as_inf[asIdx].rc_inf[idx2].dyn_reg_state = M3_RK_STATIC;
            }
        }
    }
    return 0;
}

m3_s32 m3uaInitTimer()
{
    M3TRACE(m3uaStartupTrace, ("Initialising M3UA with default timer configuration"));
    m3_timer_table.aspmtimer.retry = 16 * M3_TICKS_PER_SEC;
    m3_timer_table.aspmtimer.sw_try = 8 * M3_TICKS_PER_SEC;
    m3_timer_table.aspmtimer.l_dur = 2 * M3_TICKS_PER_SEC;
    m3_timer_table.aspmtimer.h_dur = 5 * M3_TICKS_PER_SEC;
    m3_timer_table.pdtimer.dur = 4 * M3_TICKS_PER_SEC;
    m3_timer_table.hbeattimer.retry = 8 * M3_TICKS_PER_SEC;
    m3_timer_table.hbeattimer.dur = 8 * M3_TICKS_PER_SEC;
    m3_timer_table.rkmtimer.retry = 4 * M3_TICKS_PER_SEC;
    m3_timer_table.rkmtimer.dur = 4 * M3_TICKS_PER_SEC;
    return 0;
}

m3_s32 m3uaInitSGP()
{
    m3_u16          idx, idx2, asIdx;
    m3_sgp_inf_t    *pSgp;

    M3TRACE(m3uaStartupTrace, ("Initialising SGP information containers"));
    for (idx = 0; idx < M3_MAX_SGP; idx++) {/*4 sgp is init*/
        pSgp = M3_SGP_TABLE(idx);
        pSgp->e_state  = M3_FALSE;
        pSgp->sgp_id   = idx;	/*4 idx sgp*/
        for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
            pSgp->r_asp_inf[idx2].conn_id = M3_MAX_U32;/*128 remote asp is -1*/
            for (asIdx = 0; asIdx < M3_MAX_AS; asIdx++) {
                pSgp->r_asp_inf[idx2].rc_inf[asIdx].rtctx = M3_MAX_U32;
                pSgp->r_asp_inf[idx2].rc_inf[asIdx].dyn_reg_state = M3_RK_STATIC;
            }
        }
        for (asIdx = 0; asIdx < M3_MAX_R_AS; asIdx++) {/*64 remote as init*/
            pSgp->r_as_inf[asIdx].pd_q_inf.pd_timer_inf.timer_id = M3_MAX_U32;
            pSgp->r_as_inf[asIdx].pd_q_inf.pd_q_head = M3_NULL;
            pSgp->r_as_inf[asIdx].pd_q_inf.pd_q_tail = M3_NULL;
            pSgp->r_as_inf[asIdx].state = M3_AS_DOWN;/*remote as is down init*/
            for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) {
                pSgp->r_as_inf[asIdx].asp_list[idx2] = M3_FALSE;
                pSgp->r_as_inf[asIdx].rc_inf[idx2].rtctx = M3_MAX_U32;
                pSgp->r_as_inf[asIdx].rc_inf[idx2].dyn_reg_state = M3_RK_STATIC;
            }
        }
    }
    return 0;
}

m3_s32 m3uaInitAS()
{
    m3_u16          idx;
    m3_as_inf_t     *pAs;

    M3TRACE(m3uaStartupTrace, ("Initialising AS information containers"));
    for (idx = 0; idx < M3_MAX_AS; idx++) {
        pAs = M3_AS_TABLE(idx);
        pAs->e_state = M3_FALSE;
        pAs->dyn_reg = M3_FALSE;
        pAs->rtctx = M3_MAX_U32;
        pAs->as_id = idx; /*local as idx max is 8*/
    }
    return 0;
}

m3_s32 m3uaInitRASP()
{
    m3_u16             idx;
    m3_r_asp_inf_t     *pAsp;

    M3TRACE(m3uaStartupTrace, ("Initialising remote ASP information containers"));
    for (idx = 0; idx < M3_MAX_R_ASP; idx++) {
        pAsp = M3_R_ASP_TABLE(idx);/*128 remote asp init*/
        pAsp->e_state = M3_FALSE;
        pAsp->asp_id = idx;
        pAsp->m3asp_id = M3_MAX_U32;
        pAsp->lock_state = M3_FALSE;
    }
    return 0;
}

m3_s32 m3uaInitRSGP()
{
    m3_u16             idx;
    m3_r_sgp_inf_t     *pSgp;

    M3TRACE(m3uaStartupTrace, ("Initialising remote SGP information containers"));
    for (idx = 0; idx < M3_MAX_R_SGP; idx++) {
        pSgp = M3_R_SGP_TABLE(idx);
        pSgp->e_state = M3_FALSE;
        pSgp->sgp_id = idx;
    }
    return 0;
}

m3_s32 m3uaInitRAS()
{
    m3_u16             idx;
    m3_r_as_inf_t      *pAs;

    M3TRACE(m3uaStartupTrace, ("Initialising remote AS information containers"));
    for (idx = 0; idx < M3_MAX_R_AS; idx++) {/*64 remote as init*/
        pAs = M3_R_AS_TABLE(idx);
        pAs->e_state = M3_FALSE;
        pAs->dyn_reg = M3_FALSE;
        pAs->rtctx = M3_MAX_U32;
        pAs->as_id = idx;
        pAs->min_act_asp = 0;
    }
    return 0;
}

m3_s32 m3uaInitSG()
{
    m3_u16             idx;
    m3_sg_inf_t        *pSg;

    M3TRACE(m3uaStartupTrace, ("Initialising SG information containers"));
    for (idx = 0; idx < M3_MAX_SG; idx++) {
        pSg = M3_SG_TABLE(idx);
        pSg->e_state = M3_FALSE;
        pSg->sg_id = idx;
        pSg->num_sgp = 0;
        pSg->timer_inf.timer_id = M3_MAX_U32;
        /* XXX - DEC2009 */
        pSg->nPC = 0;
    }
    return 0;
}

m3_s32 m3uaInitConn()
{
    m3_u16        idx;
    m3_conn_inf_t *pConn;

    M3TRACE(m3uaStartupTrace, ("Initialising Connection information containers"));
    for (idx = 0; idx < M3_MAX_CONN; idx++) {/*128 connect is idx*/
        pConn = M3_CONN_TABLE(idx);
        pConn->e_state = M3_FALSE;
        pConn->conn_id = idx;
	memset(pConn->rtctxUsed, M3_FALSE, M3_MAX_R_AS);
    }
    return 0;
}

m3_s32 m3uaInitNwApp()
{
    m3_u16          idx;
    m3_nwapp_inf_t  *pNw;

    M3TRACE(m3uaStartupTrace, ("Initialising Network Appearance information containers"));
    for (idx = 0; idx < M3_MAX_NWAPP; idx++) {/*8 max app*/
        pNw = M3_NWAPP_TABLE(idx);
        pNw->e_state = M3_FALSE;
    }
    return 0;
}

m3_s32 m3uaInitUsr()
{
    m3_u16           idx;
    m3_usr_inf_t     *pUsr;

    M3TRACE(m3uaStartupTrace, ("Initialising M3UA user information containers"));
    for (idx = 0; idx < M3_MAX_USR; idx++) {/*32 usr is init*/
        pUsr = M3_USR_TABLE(idx);
        pUsr->e_state = M3_FALSE;
        pUsr->usr_id = idx;
    }
    return 0;
}

