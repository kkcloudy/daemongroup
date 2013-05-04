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

#ifndef __M3UA_TYPES_H__
#define __M3UA_TYPES_H__

/************* Basic Data types ***************/
typedef int                      m3_s32;
typedef unsigned int             m3_u32;
typedef short int                m3_s16;
typedef unsigned short int       m3_u16;
typedef char                     m3_s8;
typedef unsigned char            m3_u8;


/************** Derived data types ****************/
typedef m3_u8                    m3_bool_t;
typedef m3_u32                   m3_traffic_mode_t;
typedef m3_u16                   m3_msg_type_t;
typedef m3_u32                   m3_sg_mode_t;
typedef m3_u8                    m3_asp_state_t;
typedef m3_u8                    m3_as_state_t;
typedef m3_u8                    m3_rk_state_t;


/************** Enumerated data types ****************/

typedef enum {
    M3_STD_ANSI,
    M3_STD_ITU //DEC2009 -- Removed CC Warning
} m3_standard_t;

#define M3_STD_VALID(std)	((M3_STD_ANSI == std || M3_STD_ITU == std) ? M3_TRUE:M3_FALSE)

#define M3_MAX_AS_EVENT				4

typedef enum
{
    M3_CONN_NOT_ESTB      = 0,
    M3_CONN_SETUP_IN_PROG = 1,
    M3_CONN_ESTB          = 2,
    M3_CONN_CONG_1        = 3,
    M3_CONN_CONG_2        = 4,
    M3_CONN_CONG_3        = 5,
    M3_CONN_ALIVE         = 6
} m3_conn_state_t;

/* JUNE 2010 */
typedef m3_u32 m3_conn_opt_t;

typedef enum
{
    M3_RESTART_IN_PROGRESS    = 0,
    M3_RESTART_DONE           = 1
} m3_restart_status_t;

typedef enum
{
    M3_NW_UP              = 0,
    M3_NW_DOWN            = 1
} m3_nw_status_t;

typedef enum
{
    M3_PC_UP,
    M3_PC_DOWN,
    M3_PC_RESTRICTED,
    M3_PC_CONG_1,
    M3_PC_CONG_2,
    M3_PC_CONG_3
} m3_pc_state_t;

typedef enum
{
    M3_RTSTATE_UP,
    M3_RTSTATE_DOWN,
    M3_RTSTATE_RESTRICTED,
    M3_RTSTATE_CONG_1,
    M3_RTSTATE_CONG_2,
    M3_RTSTATE_CONG_3
} m3_rtstate_t;

#define M3_RTSTATE_VALID(rtstate)  (((M3_RTSTATE_UP <= rtstate) && (M3_RTSTATE_CONG_3 >= rtstate)) ? M3_TRUE:M3_FALSE) //OCT2009

typedef enum
{
    M3_AS_EVT_ASPDN,
    M3_AS_EVT_ASPIA,
    M3_AS_EVT_ASPAC,
    M3_AS_EVT_PD_TIMER_EXP
} m3_as_event_t;

typedef enum
{
    M3_ASPM_EVT_SWTO_ASPDN,
    M3_ASPM_EVT_SWTO_ASPIA,
    M3_ASPM_EVT_SWTO_ASPAC,
    M3_ASPM_EVT_RECV_ASPDN,
    M3_ASPM_EVT_RECV_ASPDN_ACK,
    M3_ASPM_EVT_RECV_ASPUP,
    M3_ASPM_EVT_RECV_ASPUP_ACK,
    M3_ASPM_EVT_RECV_ASPIA,
    M3_ASPM_EVT_RECV_ASPIA_ACK,
    M3_ASPM_EVT_RECV_ASPAC,
    M3_ASPM_EVT_RECV_ASPAC_ACK,
    M3_ASPM_EVT_TIMER_EXPR
} m3_aspm_evt_t;

#define M3_MAX_ASP_EVENT		12

typedef enum {
    M3_L_RKM_EVT_REGISTER,
    M3_L_RKM_EVT_RECV_REGRSP,
    M3_L_RKM_EVT_DEREGISTER,
    M3_L_RKM_EVT_RECV_DEREGRSP,
    M3_L_RKM_EVT_TIMEOUT
} m3_l_rkm_evt_t;

typedef enum {
    M3_R_RKM_EVT_RECV_REGREQ,
    M3_R_RKM_EVT_STATUS,
    M3_R_RKM_EVT_RECV_DEREGREQ
} m3_r_rkm_evt_t;

typedef enum
{
    M3_ASP_M3ASP_ID,
    M3_ASP_NWAPP,
    M3_ASP_ADD_AS,
    M3_ASP_DEL_AS,
    M3_ASP_ADD_R_ASP,
    M3_ASP_DEL_R_ASP
} m3_asp_confname_t;

typedef enum {
    M3_AS_RTCTX,
    M3_AS_RKEY,
    M3_AS_INFO
} m3_as_confname_t;

typedef enum {
    M3_R_AS_RTCTX,
    M3_R_AS_RKEY,
    M3_R_AS_MIN_ACT_ASP,
    M3_R_AS_INFO
}m3_r_as_confname_t;

typedef enum {
    M3_SGP_NWAPP,
    M3_SGP_ADD_R_ASP,
    M3_SGP_DEL_R_ASP
} m3_sgp_confname_t;

typedef enum {
    M3_SG_MODE,
    M3_SG_SGP_LIST,
    M3_SG_INFO
} m3_sg_confname_t;

typedef enum {
    M3_CONN_ASSOC,
    M3_CONN_I_STR,
    M3_CONN_O_STR
} m3_conn_confname_t;

/* JUNE 2010 */
#define M3_CONN_EXCL_RTCTX	0x00000001
#define M3_CONN_EXCL_TRFMD	0x00000002
#define M3_CONN_EXCL_NWAPP	0x00000004
#define M3_CONN_EXCL_ASPID	0x00000008

#define M3_CONN_DEF_OPT		0x00000000

typedef enum {
    M3_TIMER_TYPE_ASPM,
    M3_TIMER_TYPE_PD,
    M3_TIMER_TYPE_HBEAT,
    M3_TIMER_TYPE_RKM
} m3_timer_type_t;

#define M3_MAX_TIMER_TYPE		4

typedef enum {
    M3_UNIQUE,
    M3_PARTIAL_MATCH,
    M3_MATCH
} m3_match_t;

/****************** structures *********************/
typedef struct
{
    m3_u32                timer_id;
    m3_timer_type_t       type;
    m3_u8                 *p_buf;
} m3_timer_inf_t;

typedef struct {
    m3_u16                num_rc;
    m3_u32                rc_list[M3_MAX_RTCTX];
} m3_rtctx_err_t;

typedef struct __m3_pd_buf_t
{
    m3_u32                buflen;
    m3_u8                 *p_buf;
    struct __m3_pd_buf_t  *p_next;
} m3_pd_buf_t;

typedef struct
{
    m3_timer_inf_t        pd_timer_inf;
    m3_pd_buf_t           *pd_q_head;
    m3_pd_buf_t           *pd_q_tail;
} m3_pd_q_inf_t;

typedef struct
{
    m3_u32                opc;
    m3_u16                lcic;
    m3_u16                ucic;
} m3_ckt_range_t;

typedef struct {
    m3_u32                dpc;
    m3_u16                num_si;
    m3_u8                 si_list[M3_MAX_SI_PER_RK];
    m3_u16                num_opc;
    m3_u32                opc_list[M3_MAX_OPC_PER_RK];
    m3_u16                num_ckt_range;
    m3_ckt_range_t        ckt_range[M3_MAX_CKT_RANGE_PER_RK];
} m3_rk_elements_t;

typedef struct
{
    m3_traffic_mode_t     trfmode;
    m3_u32                nw_app;
    m3_u8                 num_rtparam;
    m3_rk_elements_t      rtparam[M3_MAX_DPC_PER_RK];
} m3_rk_inf_t;

typedef struct
{
    m3_u32    as_id;
    m3_u32    l_spid;
} m3_pdtimer_param_t;

typedef struct
{
    m3_u32     conn_id;
    m3_u8      sess_id;
} m3_aspmtimer_param_t;

typedef struct
{
    m3_u32     conn_id;
    m3_u8      sess_id;
    m3_u16     num_as;
    m3_u32     as_list[M3_MAX_AS];
} m3_rkmtimer_param_t;

typedef struct
{
    m3_timer_type_t    type;
    union
    {
        m3_pdtimer_param_t    pdbuf;
        m3_aspmtimer_param_t  aspmbuf;
        m3_rkmtimer_param_t   rkmbuf;
    } param;
} m3_timer_param_t;

typedef struct {
    m3_msg_type_t         msg_type;  /* type of ASPM message */
    m3_u32                msg_len;
    m3_u8                 *msg;
} m3_aspm_msg_inf_t;

typedef struct {
    m3_msg_type_t         msg_type;  /* type of RKM message */
    m3_u32                msg_len;
    m3_u8                 *msg;
} m3_rkm_msg_inf_t;

typedef struct
{
    m3_u32                ptcode;
    m3_u32                nw_app;
} m3_pc_inf_t;

typedef struct
{
    m3_u32               opc;
    m3_u32               dpc;
    m3_u8                si;
    m3_u8                ni;
    m3_u8                mp;
    m3_u8                sls;
} m3_rt_lbl_t;

typedef struct {
    m3_u16                retry;
    m3_u16                sw_try;
    m3_u16                l_dur;
    m3_u16                h_dur;
} m3_aspmtimer_t;

typedef struct {
    m3_u16                retry;
    m3_u16                dur;
} m3_rkmtimer_t;

typedef struct {
    m3_u16                dur;
} m3_pdtimer_t;

typedef struct {
    m3_u16                retry;
    m3_u16                dur;
} m3_hbeattimer_t;

typedef enum {
    m3uaErrorTrace 	= 0x01,
    m3uaConfigTrace	= 0x02,
    m3uaAspmTrace	= 0x04,
    m3uaRkmTrace	= 0x08,
    m3uaSsnmTrace	= 0x10,
    m3uaTxrTrace	= 0x20,
    m3uaMgmtTrace	= 0x40,
    m3uaInMsgTrace	= 0x80,
    m3uaOutMsgTrace	= 0x0100,
    m3uaStartupTrace	= 0x0200,
    m3uaTimerTrace      = 0x0400,
    m3uaMaxTrcType
} m3TrcType_t;

typedef struct {
    m3_u32 routingContext;
    m3_traffic_mode_t trafficMode;
    m3_u32 networkApperance;
}asConfType_t;

extern asConfType_t msc_as_conf;
extern asConfType_t sgsn_as_conf;


#endif

