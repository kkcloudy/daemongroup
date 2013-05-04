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

#ifndef __M3UA_CONFIG_H__
#define __M3UA_CONFIG_H__

/************** message encoding/decoding related structures ************/

typedef struct
{
    m3_u8            version;
    m3_u8            resv;
    m3_u8            msg_class;
    m3_u8            msg_type;
    m3_u32           msg_len;
} m3_msg_hdr_t;

typedef struct
{
    m3_u16           tag;
    m3_u16           len;
    m3_u32           val[M3_MAX_TLV_VAL_SIZE];
} m3_msg_tlv_t;

typedef struct
{
    m3_u32           opc;
    m3_u32           dpc;
    m3_u8            si;
    m3_u8            ni;
    m3_u8            mp;
    m3_u8            sls;
    m3_u8            *prot_data;
} m3_msg_data_val_t;

typedef struct
{
    m3_u32               nw_app;
    m3_u32               rtctx;
    m3_u32               crn_id;
    m3_rt_lbl_t          rt_lbl;
    m3_u16               prot_data_len;
    m3_u8                *p_prot_data;
} m3_data_inf_t;

typedef struct
{
    m3_u32               nw_app;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
                                        /* get from the user, mask included */
    m3_u8                info_len;
    m3_u8                *p_info_str;   /* get from the user */
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_duna_inf_t;

typedef m3_duna_inf_t m3_dava_inf_t;
typedef m3_duna_inf_t m3_daud_inf_t;
typedef m3_duna_inf_t m3_drst_inf_t;

typedef struct
{
    m3_u32               nw_app;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
    m3_u32               crnd_dpc;      /* concerned pc */
    m3_u8                cong_level;    /* congestion level */
    m3_u8                info_len;
    m3_u8                *p_info_str;   /* get from the user */
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_scon_inf_t;

typedef struct
{
    m3_u32               nw_app;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u32               pc;
    m3_u16               cause;
    m3_u16               user;
    m3_u8                info_len;
    m3_u8                *p_info_str;
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_dupu_inf_t;

typedef struct
{
    m3_u32               m3asp_id;    /* ASP Identifier */
    m3_u8                info_len;
    m3_u8                *p_info_str;
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_aspup_inf_t;

typedef struct
{
    m3_u8                info_len;
    m3_u8                *p_info_str;
} m3_aspup_ack_inf_t;

typedef struct
{
    m3_u8                info_len;
    m3_u8                *p_info_str;
} m3_aspdn_inf_t;

typedef m3_aspdn_inf_t m3_aspdn_ack_inf_t;

typedef struct
{
    m3_u16               hbeat_data_len;
    m3_u8                *p_hbeat_data;
} m3_hbeat_inf_t;

typedef m3_hbeat_inf_t   m3_hbeat_ack_inf_t;

typedef struct
{
    m3_u16               num_rk;
    struct
    {
        m3_u32           lrk_id;
        m3_rk_inf_t      rk_inf;
    } rk_list[M3_MAX_RK];
} m3_reg_req_inf_t;

typedef struct
{
    m3_u16               num_reg_result;
    struct
    {
        m3_u32           lrk_id;
        m3_u32           reg_status;
        m3_u32           rtctx;
    } rr_list[M3_MAX_RK];
} m3_reg_rsp_inf_t;

typedef struct
{
   m3_u16                num_rc;
   m3_u32                rc_list[M3_MAX_RK];
} m3_dreg_req_inf_t;

typedef struct
{
    m3_u16               num_dreg_result;
    struct
    {
        m3_u32           rtctx;
        m3_u32           dreg_status;
    } drr_list[M3_MAX_RK];
} m3_dreg_rsp_inf_t;

typedef struct
{
    m3_u32               trfmode;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u8                info_len;
    m3_u8                *p_info_str;
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_aspac_inf_t;

typedef m3_aspac_inf_t m3_aspac_ack_inf_t;

typedef struct
{
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u8                info_len;
    m3_u8                *p_info_str;
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_aspia_inf_t;

typedef m3_aspia_inf_t m3_aspia_ack_inf_t;

typedef struct
{
    m3_u32               err_code;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u16               num_pc;
    m3_u32               pc_list[M3_MAX_PC_SSNM];
    m3_u32               nw_app;
    m3_u16               diag_len;
    m3_u8                *p_diag_inf;
} m3_error_inf_t;

typedef struct
{
    m3_u16               status_type;
    m3_u16               status_inf;
    m3_u32               m3asp_id;
    m3_u16               num_rc;
    m3_u32               rc_list[M3_MAX_RTCTX];
    m3_u8                info_len;
    m3_u8                *p_info_str;
    m3_u32               msglen;
    m3_u8                *p_msg;
} m3_ntfy_inf_t;


/************** Information containers for managed objects *************/

typedef struct {
    m3_bool_t             e_state;
    m3_u32                asp_id;
    m3_u32                m3asp_id;
    m3_u32                sctp_ep_id;
    m3_bool_t             as_list[M3_MAX_AS];
    m3_u32                def_nwapp;
    /* Local AS information, per remote ASP/SGP */
    struct {
        m3_u32            conn_id;
        struct {
            m3_u32        rtctx;
            m3_u8         dyn_reg_state;
        } rc_inf[M3_MAX_AS];
    } r_asp_inf[M3_MAX_R_ASP], r_sgp_inf[M3_MAX_R_SGP];

    struct {
        m3_pd_q_inf_t   pd_q_inf; /* pending queue related information */
        m3_as_state_t   state;
        m3_u8           num_asp_inactive;
        m3_u8           num_asp_active;
        m3_bool_t       asp_list[M3_MAX_R_ASP]; /* to be taken during config */
        /* remote as related information, maintained per ASP basis */
        struct {
            m3_bool_t   dyn_entry;  /* specifies if this is a dynamic entry */
            m3_u32      rtctx;
            m3_u32      rk_id;
            m3_u8       dyn_reg_state;
        } rc_inf[M3_MAX_R_ASP];
        /* route map for this Remote AS */
        //m3_rt_map_inf_t route_map;
    } r_as_inf[M3_MAX_R_AS];

    struct {
        /* route map for the SG */
        //m3_rt_map_inf_t route_map;
    } sg_inf[M3_MAX_SG];

} m3_asp_inf_t;

typedef struct {
    m3_bool_t             e_state;
    m3_bool_t             dyn_reg;
    m3_u32                rtctx;
    m3_u32                as_id;
    m3_rk_inf_t           rkey;
} m3_as_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                asp_id;
    m3_u32                sctp_ep_id;
    m3_u32                m3asp_id;
    m3_bool_t             lock_state;
} m3_r_asp_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_bool_t             dyn_reg;
    m3_u32                rtctx;
    m3_u32                as_id;
    m3_rk_inf_t           rkey;
    m3_u32                min_act_asp;
} m3_r_as_inf_t;

typedef struct
{
    m3_bool_t             e_state;	  /*connet state*/
    m3_u32                conn_id;	  /*connet id */
    m3_u32                assoc_id;   /* sctp association id */
    m3_conn_state_t       conn_state; /* connection state */
    m3_u32                i_str;    /* number of in streams */
    m3_u32                o_str;    /* number of out streams */
    m3_u32                l_sp_id;	/*local sp id*/
    m3_u32                r_sp_id;	/*remote sp id*/
    m3_asp_state_t        l_sp_g_st; /* local sp global state */
    m3_asp_state_t        r_sp_g_st; /* remote sp global state */

    /* JUNE 2010 -- options */
    m3_conn_opt_t         opt;

    /* list of size M3_MAX_AS */
    m3_asp_state_t        l_sp_st[M3_MAX_AS];

    /* list of size M3_MAX_R_AS */
    m3_asp_state_t        r_sp_st[M3_MAX_R_AS];

    /* heart beat session information */
    struct {
        m3_bool_t             e_state;
        m3_timer_inf_t        timer_inf;
        m3_aspm_msg_inf_t     msg_inf;
        m3_u32                num_retry;
    } hbeat_sess_inf;

    /* ASPM session active on the association */
    /* while ASPUP and ASPDN only one session is permitted */
    struct {
        /* state of session */
        m3_bool_t             e_state;

        /* active timer of the session */
        m3_timer_inf_t        timer_inf;

        /* msg that may be retransmited for this session */
        m3_aspm_msg_inf_t     msg_inf;

        /* Number of retries left */
        m3_u32                num_retry;
    } aspm_sess_inf[M3_MAX_ASPM_SESS_PER_CONN];

    /* RKM sessions active on this association */
    struct {
        /* state of session */
        m3_bool_t             e_state;

        /* active timer of the session */
        m3_timer_inf_t        timer_inf;

        /* msg that may be retransmited for this session */
        m3_rkm_msg_inf_t      msg_inf;

        /* Number of retries left */
        m3_u32                num_retry;
    } rkm_sess_inf[M3_MAX_RKM_SESS_PER_CONN];

    /* Routing context pools */
/* 
 *  ----------------------------
 *  | conn id   | remote AS Id |
 *  ----------------------------
 */
    m3_bool_t                 rtctxUsed[M3_MAX_R_AS];
} m3_conn_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                sgp_id;
    m3_u32                sctp_ep_id;
    m3_u32                def_nwapp;
    m3_u16                user_id;
    struct {
        m3_u32            conn_id;
        struct {
            m3_u32        rtctx;
            m3_u32        rk_id;
            m3_u8         dyn_reg_state;
        } rc_inf[M3_MAX_AS];
    } r_asp_inf[M3_MAX_R_ASP];

    struct
    {
        m3_pd_q_inf_t   pd_q_inf; /* pending queue related information */
        m3_as_state_t   state;
        m3_u8           num_asp_inactive;
        m3_u8           num_asp_active;
        m3_bool_t       asp_list[M3_MAX_R_ASP]; /* to be taken during config */
        struct {
            m3_bool_t   dyn_entry;  /* specifies if this is a dynamic entry */
            m3_u32      rtctx;
            m3_u32      rk_id;
            m3_u8       dyn_reg_state;
        } rc_inf[M3_MAX_R_ASP];
        /* route map for this Remote AS */
        //m3_rt_map_inf_t route_map;
    } r_as_inf[M3_MAX_R_AS];
} m3_sgp_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                sgp_id;
    m3_u32                sctp_ep_id;
    m3_u32                sg_id;
} m3_r_sgp_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                sg_id;
    m3_sg_mode_t          sgmode;
    m3_u8                 num_sgp;
    m3_u32                sgp_list[M3_MAX_SGP_PER_SG];

    /* XXX - DEC2009 - Routes Through this SG, for sending notifications to user */
    m3_pc_inf_t           pcList[M3_MAX_ROUTES_PER_SG];
    m3_u16                nPC;

    /* msg and timer information */
    m3_timer_inf_t        timer_inf; /* active timer running for sg */
} m3_sg_inf_t;

typedef struct __m3_rt_inf_t__
{
    m3_bool_t                   e_state;
    m3_pc_inf_t                 pc_inf;
    m3_u32                      le_id;
    m3_u8                       priority;
    m3_rtstate_t                rtstate;
    struct __m3_rt_inf_t__      *p_same;
    struct __m3_rt_inf_t__      *p_diff;
} m3_rt_inf_t;

typedef struct __m3_local_rt_inf_t__
{
    m3_bool_t                     e_state;
    m3_pc_inf_t                   pc_inf;
    m3_u32                        user_id;
    struct __m3_local_rt_inf_t__  *p_same;
    struct __m3_local_rt_inf_t__  *p_diff;
} m3_local_rt_inf_t;

typedef struct m3_local_rt_tbl_t
{
    m3_local_rt_inf_t           rt_tbl[M3_MAX_LOCAL_ROUTE];
    struct m3_local_rt_tbl_t    *p_next;
} m3_local_rt_tbl_list_t;

typedef struct m3_rt_tbl_t
{
    m3_rt_inf_t           rt_tbl[M3_MAX_ROUTE];
    struct m3_rt_tbl_t    *p_next;
} m3_rt_tbl_list_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                nw_app;
    m3_standard_t         standard;
    m3_restart_status_t   status;
} m3_nwapp_inf_t;

typedef struct
{
    m3_bool_t             e_state;
    m3_u32                usr_id;
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
} m3_usr_inf_t;

typedef struct {
    m3_aspmtimer_t        aspmtimer;
    m3_pdtimer_t          pdtimer;
    m3_hbeattimer_t       hbeattimer;
    m3_rkmtimer_t         rkmtimer;
} m3_prot_timer_t;
/*************** function pointer tables *****************/

typedef m3_s32 (*m3uaRAsProcFp_t)(m3_conn_inf_t*, m3_r_as_inf_t*, void*);
typedef m3_s32 (*m3uaAspmFp_t)(m3_conn_inf_t*, void*);

typedef struct {
    m3_u32             n_ntfy_pd;
    m3_u32             c_offset;      /* current offset to read */
    m3_u32             n_offset;      /* next offset to write */
    m3ua_mgmt_ntfy_t   ntfy_list[M3_MAX_MGMT_NTFY];
} m3_mgmt_ntfy_inf_t;

typedef struct {
    m3_u32             n_ntfy_pd;
    m3_u32             c_offset;      /* current offset to read */
    m3_u32             n_offset;      /* next offset to write */
    m3ua_user_ntfy_t   ntfy_list[M3_MAX_USER_NTFY];
} m3_user_ntfy_inf_t;

#define M3_ASP_TABLE(id)            &m3_asp_table[(m3_u16)id]
#define M3_SGP_TABLE(id)            &m3_sgp_table[(m3_u16)id]
#define M3_AS_TABLE(id)             &m3_as_table[(m3_u16)id]
#define M3_R_ASP_TABLE(id)          &m3_r_asp_table[(m3_u16)id]
#define M3_R_SGP_TABLE(id)          &m3_r_sgp_table[(m3_u16)id]
#define M3_R_AS_TABLE(id)           &m3_r_as_table[(m3_u16)id]
#define M3_CONN_TABLE(id)           &m3_conn_table[id]
#define M3_SG_TABLE(id)             &m3_sg_table[(m3_u16)id]
#define M3_NWAPP_TABLE(id)          &m3_nwapp_table[id]
#define M3_USR_TABLE(id)            &m3_usr_table[id]
#define M3_LOCAL_ROUTE_TABLE(id)    &m3_local_route_table.rt_tbl[id]
#define M3_ROUTE_TABLE(id)          &m3_route_table.rt_tbl[id]
#define M3_MGMT_NTFY_TABLE          &m3_mgmt_ntfy_table
#define M3_USER_NTFY_TABLE          &m3_user_ntfy_table
#define M3_TIMER_TABLE		        &m3_timer_table

#endif

