/****************************************************************************
** Description:
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

#ifndef __M3UA_API_H__
#define __M3UA_API_H__

/*************** configuration structures ****************/

typedef struct
{
    m3_u32                m3asp_id;
    m3_u32                sctp_ep_id;
    m3_bool_t             as_list[M3_MAX_AS];
    m3_u32                def_nwapp;
    struct {
        m3_bool_t         asp_list[M3_MAX_R_ASP];
    } r_as_inf[M3_MAX_R_AS];
} m3_asp_conf_t;

typedef struct
{
    m3_u32                sctp_ep_id;
} m3_r_asp_conf_t;

typedef struct
{
    m3_u32                rtctx;
    m3_rk_inf_t           rkey;
} m3_as_conf_t;

typedef struct
{
    m3_u32                rtctx;
    m3_rk_inf_t           rkey;
    m3_u8                 min_act_asp;
} m3_r_as_conf_t;

typedef struct
{
    m3_u32                assoc_id;
    m3_u32                i_str;
    m3_u32                o_str;
    m3_u32                opt; /* JUNE 2010 */
} m3_conn_conf_t;

typedef struct
{
    m3_pc_inf_t           pc_inf;
    m3_u16                user_id;
} m3_local_rt_conf_t;

typedef struct
{
    m3_u32                sctp_ep_id;
    m3_u32                def_nwapp;
    struct {
        m3_bool_t         asp_list[M3_MAX_R_ASP];
    } r_as_inf[M3_MAX_R_AS];
} m3_sgp_conf_t;

typedef struct
{
    m3_u32                sctp_ep_id;
} m3_r_sgp_conf_t;

typedef struct
{
    m3_sg_mode_t          sgmode;
    m3_u8                 num_sgp;
    m3_u32                sgp_list[M3_MAX_SGP_PER_SG];
} m3_sg_conf_t;

typedef struct
{
    m3_pc_inf_t           pc_inf;
    m3_u32                le_id;
    m3_u8                 priority;
} m3_rt_conf_t;

typedef struct {
    m3_u32                nw_app;
    m3_standard_t         standard;
} m3_nwapp_conf_t;

typedef struct
{
    m3_u32                sp_id;
    union {
        struct {
            m3_u8         sio;
            m3_u32        as_id;
        } mtp_user;
        struct {
            m3_bool_t     a_data;
        } nif_user;
    } user;
} m3_usr_conf_t;

/************** Configuration/management API structures **************/

typedef union {
    struct {
        m3_asp_conf_t info;
    } add;
    struct {
        m3_asp_conf_t info;
    } get;
    struct {
        m3_asp_confname_t    confname;
        m3_asp_conf_t        info;
        m3_u32               as_id;
        m3_u32               ras_id;
        m3_u32               rasp_id;
    } modify;
} m3ua_asp_t;

typedef union {
    struct {
        m3_as_conf_t info;
    } add;
    struct {
        m3_as_conf_t info;
    } get;
    struct {
        m3_as_confname_t    confname;
        m3_as_conf_t        info;
    } modify;
} m3ua_as_t;

typedef union {
    struct {
        m3_r_asp_conf_t info;
    } add;
    struct {
        m3_r_asp_conf_t info;
    } get;
} m3ua_r_asp_t;

typedef union {
    struct {
        m3_r_as_conf_t info;
    } add;
    struct {
        m3_r_as_conf_t info;
    } get;
    struct {
        m3_r_as_confname_t    confname;
        m3_r_as_conf_t        info;
    } modify;
} m3ua_r_as_t;

typedef union {
    struct {
        m3_sgp_conf_t info;
    } add;
    struct {
        m3_sgp_conf_t info;
    } get;
    struct {
        m3_sgp_confname_t    confname;
        m3_sgp_conf_t        info;
        m3_u32               ras_id;
        m3_u32               rasp_id;
    } modify;
} m3ua_sgp_t;

typedef union {
    struct {
        m3_r_sgp_conf_t info;
    } add;
    struct {
        m3_r_sgp_conf_t info;
    } get;
} m3ua_r_sgp_t;

typedef union {
    struct {
        m3_sg_conf_t info;
    } add;
    struct {
        m3_sg_conf_t info;
    } get;
    struct {
        m3_sg_confname_t    confname;
        m3_sg_conf_t        info;
    } modify;
} m3ua_sg_t;

typedef union {
    struct {
        struct {
            m3_u32        as_id;
        } info;
        m3_asp_state_t    state;
    } get;
    struct {
        struct {
            m3_u16        num_as;
            m3_u32        as_list[M3_MAX_AS];
        }info;
        m3_asp_state_t    state;
    } modify;
} m3ua_asp_state_t;

typedef union {
    struct {
        struct {
            m3_u32            as_id;
        } info;
        m3_asp_state_t    state;
    } get;
} m3ua_r_asp_state_t;

typedef union {
    struct {
        m3_as_state_t    state;
    } get;
} m3ua_r_as_state_t;

typedef union {
    struct {
        m3_conn_conf_t info;
    } add;
    struct {   
        m3_conn_conf_t info;
    } get;
    struct {
        m3_conn_confname_t    confname;
        m3_conn_conf_t        info;
    } modify;
} m3ua_conn_t;

typedef union {
    struct {
        m3_conn_state_t state;
    } get;
    struct {
        m3_conn_state_t state;
    } modify;
} m3ua_conn_state_t;

/* JUNE 2010 */
typedef union {
    struct {
        m3_conn_opt_t opt;
    } get;
    struct {
        m3_conn_opt_t opt;
    } modify;
} m3ua_conn_opt_t;

typedef union {
    struct {
        m3_rt_conf_t       info;
    } add;
    struct {
        m3_rt_conf_t       info;
    } del;
} m3ua_route_t;

typedef union {
    struct {
        m3_local_rt_conf_t   info;
    } add;
    struct {
        m3_local_rt_conf_t   info;
    } del;
} m3ua_local_route_t;

typedef union {
    struct {
        m3_usr_conf_t    info;
    } add;
} m3ua_user_t;

typedef union {
    struct {
        m3_nwapp_conf_t    info;
    } add;
    struct {
        m3_nwapp_conf_t    info;
    } del;
} m3ua_nwapp_t;

typedef struct {
    m3_timer_type_t        type;
    union {
        m3_aspmtimer_t     aspmtimer;
        m3_pdtimer_t       pdtimer;
        m3_hbeattimer_t    hbeattimer;
        m3_rkmtimer_t      rkmtimer;
    } get;
    union {
        m3_aspmtimer_t     aspmtimer;
        m3_pdtimer_t       pdtimer;
        m3_hbeattimer_t    hbeattimer;
        m3_rkmtimer_t      rkmtimer;
    } modify;
} m3ua_timer_t;

typedef union {
    struct {
        m3_u32             asp_id;
        m3_u32             rsp_id;
        m3_u16             num_as;
        m3_u32             as_list[M3_MAX_AS];
    } reg;
    struct {
        m3_u32             asp_id;
        m3_u32             rsp_id;
        m3_u16             num_as;
        m3_u32             as_list[M3_MAX_AS];
    } dereg;
    struct {
        m3_u32             lsp_id;
        m3_u32             rasp_id;
        m3_u16             num_result;
        struct {
            m3_u32         as_id;
            m3_u32         reg_status;
        } result[M3_MAX_RK];
    } status;
} m3ua_rkey_t;

/*************** notification from M3UA to management ****************/
typedef struct {
    m3_u8         type;
    union {
        struct {
            m3_u32           lsp_id;
            m3_u32           rsp_id;
            m3_u32           err_code;
            m3_u16           num_rc;
            m3_u32           rc_list[M3_MAX_RTCTX];
            m3_u16           num_pc;
            m3_u32           pc_list[M3_MAX_PC_SSNM];
            m3_u32           nw_app;
            m3_u16           diag_len;
            m3_u8            *p_diag_inf;
        } err;
        struct {
            m3_u32           asp_id;
            m3_u32           rsp_id;
            m3_u32           as_id;
            m3_asp_state_t   state;
        } asp;
        struct {
            m3_u32           asp_id;
            m3_u32           lsp_id;
            m3_u32           as_id;
            m3_asp_state_t   state;
        } r_asp;
        struct {
            m3_u32           as_id;
            m3_u32           lsp_id;
            m3_u32           rsp_id;
            m3_as_state_t    state;
        } as;
        struct {
            m3_u32           as_id;
            m3_u32           lsp_id;
            m3_as_state_t    state;
        } r_as;
        struct {
            m3_u32           lsp_id;
            m3_u32           rsp_id;
            m3_conn_state_t  state;
        } conn;
        struct {
            m3_u32           lsp_id;
            m3_u32           rsp_id;
            m3_u16           status_type;
            m3_u16           status_inf;
            m3_u32           m3asp_id;
            m3_u16           num_as;
            m3_u32           as_list[M3_MAX_RTCTX];
        } notify;
        struct {
            m3_u32           lsp_id;
            m3_u32           asp_id;
            m3_u16           num_as;
            m3_u32           as_list[M3_MAX_RK];
        } reg;
        struct {
            m3_u32           lsp_id;
            m3_u32           asp_id;
            m3_u16           num_as;
            m3_u32           as_list[M3_MAX_RK];
        } dereg;
        struct {
            m3_u32           asp_id;
            m3_u32           rsp_id;
            m3_u16           num_as;
            struct {
                m3_u32       as_id;
                m3_u32       rtctx;
                m3_u32       status;
            } result[M3_MAX_RK];
        } reg_status;
        struct {
            m3_u32           asp_id;
            m3_u32           rsp_id;
            m3_u16           num_as;
            struct {
                m3_u32       as_id;
                m3_u32       rtctx;
                m3_u32       status;
            } result[M3_MAX_RK];
        } dreg_status;
    } param;
} m3ua_mgmt_ntfy_t;

/*************** User API structures ******************/

typedef struct {
    m3_u32               nw_app;
    m3_bool_t            add_rtctx;
    m3_u32               crn_id;
    m3_rt_lbl_t          rt_lbl;
    m3_u32               prot_data_len;
    m3_u8                *p_prot_data;
} m3ua_txr_t;

typedef struct {
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
    m3_u32               nw_app;
    m3_u8                info_len;
    m3_u8                *p_info_str;
} m3ua_pause_t;

typedef struct {
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
    m3_u32               nw_app;
    m3_u8                info_len;
    m3_u8                *p_info_str;
} m3ua_audit_t;

typedef struct {
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
    m3_u32               nw_app;
    m3_u8                info_len;
    m3_u8                *p_info_str;
} m3ua_resume_t;

typedef struct {
    m3_u8                cause;
    union {
        struct {
            m3_u32       nw_app;
            m3_u16       num_pc;
            m3_u32       pc_list[M3_MAX_PC_SSNM];
            m3_u32       crnd_dpc;
            m3_u8        cong_level;
            m3_u8        info_len;
            m3_u8        *p_info_str;
        } cong_inf;
        struct {
            m3_u32       nw_app;
            m3_u32       ptcode;
            m3_u8        user;
            m3_u8        info_len;
            m3_u8        *p_info_str;
        } upu_inf;
        struct {
            m3_u32       nw_app;
            m3_u16       num_pc;
            m3_u32       pc_list[M3_MAX_PC_SSNM];
            m3_u8        info_len;
            m3_u8        *p_info_str;
        } drst_inf;
    } status_inf;
} m3ua_status_t;

typedef struct {
    m3_u32               nw_app;
    m3_restart_status_t  status;
} m3ua_restart_t;


/*************** notification from M3UA to User ****************/
typedef struct {
    m3_u8         type;
    union {
        struct {
            m3_u16        user_id;
            m3_u32        nw_app;
            m3_u32        ptcode;
        } pause;
        struct {
            m3_u16        user_id;
            m3_u32        nw_app;
            m3_u32        ptcode;
        } resume;
        struct {
            m3_u16        user_id;
            m3_u32        nw_app;
            m3_u32        ptcode;
            m3_u32        crnd_dpc;
            m3_u8         cause;
            m3_u8         inf;
        } status;
        struct {
            m3_u16        user_id;
            struct {
                m3_u32               nw_app;
                m3_u32               rtctx;
                m3_u32               crn_id;
                m3_rt_lbl_t          rt_lbl;
                m3_u16               prot_data_len;
                m3_u8                *p_prot_data;
            } inf;
        } transfer;
        struct {
            m3_u16        user_id;
            m3_u32        nw_app;
            m3_u8         mask;
            m3_u32        ptcode;
        } audit;
    } param;
} m3ua_user_ntfy_t;


/**************** M3UA API Prototypes ******************/

m3_s32 m3ua_init(void);

m3_s32 m3ua_as(m3_u32, m3_u8, m3ua_as_t *);

m3_s32 m3ua_asp(m3_u32, m3_u8, m3ua_asp_t *);

m3_s32 m3ua_asp_state(m3_u32, m3_u32, m3_u8, m3ua_asp_state_t *);

m3_s32 m3ua_conn(m3_u32, m3_u32, m3_u8, m3ua_conn_t *);

m3_s32 m3ua_conn_state(m3_u32, m3_u32, m3_u8, m3ua_conn_state_t *);

m3_s32 m3ua_conn_opt(m3_u32, m3_u32, m3_u8, m3ua_conn_opt_t *); /* JUNE 2010 */

m3_s32 m3ua_local_route(m3_u8, m3ua_local_route_t *);

m3_s32 m3ua_mgmt_ntfy(m3ua_mgmt_ntfy_t *);

m3_s32 m3ua_nwapp(m3_u8, m3ua_nwapp_t *);

m3_s32 m3ua_r_as(m3_u32, m3_u8, m3ua_r_as_t *);

m3_s32 m3ua_r_as_state(m3_u32, m3_u32, m3_u8, m3ua_r_as_state_t *);

m3_s32 m3ua_r_asp(m3_u32, m3_u8, m3ua_r_asp_t *);

m3_s32 m3ua_r_asp_state(m3_u32, m3_u32, m3_u8, m3ua_r_asp_state_t *);

m3_s32 m3ua_route(m3_u8, m3ua_route_t *);

m3_s32 m3ua_r_asplock(m3_u32, m3_u8, void *);

m3_s32 m3ua_timer(m3_u8, m3ua_timer_t *);

m3_s32 m3ua_heartbeat(m3_u32, m3_u32, m3_u8, void *);

m3_s32 m3ua_r_sgp(m3_u32, m3_u8, m3ua_r_sgp_t *);

m3_s32 m3ua_sg(m3_u32, m3_u8, m3ua_sg_t *);

m3_s32 m3ua_sgp(m3_u32, m3_u8, m3ua_sgp_t *);

m3_s32 m3ua_user(m3_u16, m3_u8, m3ua_user_t *);

m3_s32 m3ua_pause(m3_u16, m3ua_pause_t *);

m3_s32 m3ua_resume(m3_u16, m3ua_resume_t *);

m3_s32 m3ua_status(m3_u16, m3ua_status_t *);

m3_s32 m3ua_transfer(m3_u16, m3ua_txr_t *);

m3_s32 m3ua_audit(m3_u16, m3ua_audit_t *);

m3_s32 m3ua_user_ntfy(m3ua_user_ntfy_t *);

void* m3timer_ckexpiry(void *);

/************** Transport Layer API Prototypes ************/

m3_s32 m3ua_sendmsg(m3_u32, m3_u32, m3_u32, m3_u8 *);

m3_s32 m3ua_recvmsg(m3_u32, m3_u32, m3_u32, m3_u8 *);

/************** Traces API ********************************/

void m3ua_set_trace_map(m3_u32 aTraceMap);

void m3ua_add_trace(m3TrcType_t aTraceType);

void m3ua_del_trace(m3TrcType_t aTraceType);

m3_u32 m3ua_get_trace_map(void);

/*************** Diagnostics API ***************************/

m3_s32 m3ua_diag(m3_u32 diagType, m3_s8 *diagStr, m3_u32 len);

#endif

