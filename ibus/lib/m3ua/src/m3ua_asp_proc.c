/*****************************************************************************
** Description:
** Entry functions to ASPSM and ASPTM FSM's.
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
******************************************************************************/

#include <m3ua_asp_proc.h>

m3uaAspmFp_t m3uaAspmStateMachine[M3_MAX_ASP_STATE][M3_MAX_ASP_EVENT] =
{
    /* ASP-DOWN state */
    {
        M3_NULL,                   /* M3_ASPM_EVT_SWTO_ASPDN */
        m3ua_ASPDOWN_ESENDUP,          /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                   /* M3_ASPM_EVT_SWTO_ASPAC */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPDN */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_ASPDOWN_ERECVUP,          /* M3_ASPM_EVT_RECV_ASPUP */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPIA */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPAC */
        M3_NULL,                   /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        M3_NULL                    /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* ASP-INACTIVE state */
    {
        m3ua_ESENDDN,                  /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPIA */
        m3ua_ASPINACTIVE_ESENDAC,      /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_ASPINACTIVE_ERECVDN,      /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_ASPINACTIVE_ERECVDNACK,   /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPUP */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPIA */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPAC */
        m3ua_ASPINACTIVE_ERECVACACK,   /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        m3ua_ASPINACTIVE_ETIMEREXP     /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* ASP-ACTIVE state */
    {
        m3ua_ESENDDN,                  /* M3_ASPM_EVT_SWTO_ASPDN */
        m3ua_ASPACTIVE_ESENDIA,        /* M3_ASPM_EVT_SWTO_ASPIA */
        m3ua_ASPACTIVE_ESENDAC,        /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_ASPACTIVE_ERECVDN,        /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_ASPACTIVE_ERECVDNACK,     /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_ASPACTIVE_ERECVUP,        /* M3_ASPM_EVT_RECV_ASPUP */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPIA */
        m3ua_ASPACTIVE_ERECVIAACK,     /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPAC */
        m3ua_ASPACTIVE_ERECVACACK,     /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        m3ua_ASPACTIVE_ETIMEREXP       /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* ASP-DNSENT state */
    {
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_ASPDNSENT_ERECVDN,        /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_ASPDNSENT_ERECVDNACK,     /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_ASPDOWN_ERECVUP,          /* M3_ASPM_EVT_RECV_ASPUP */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPIA */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPAC */
        m3uaVoid,                     /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        m3ua_ASPDNSENT_ETIMEREXP       /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* ASP-UPSENT state */
    {
        m3ua_ESENDDN,                  /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                       /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_ASPUPSENT_ERECVDN,        /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_ASPUPSENT_ERECVDNACK,     /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_ASPUPSENT_ERECVUP,        /* M3_ASPM_EVT_RECV_ASPUP */
        m3ua_ASPUPSENT_ERECVUPACK,     /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPIA */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPAC */
        M3_NULL,                       /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        m3ua_ASPUPSENT_ETIMEREXP       /* M3_ASPM_EVT_TIMER_EXPR */
    }
};

m3uaAspmFp_t m3uaRAspmStateMachine[M3_MAX_ASP_STATE][M3_MAX_ASP_EVENT] =
{
    /* R-ASP-DOWN */
    {
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPDN */
        m3ua_RASPDOWN_ESENDUP,         /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_RASPDOWN_ERECVDN,         /* M3_ASPM_EVT_RECV_ASPDN */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_RASPDOWN_ERECVUP,         /* M3_ASPM_EVT_RECV_ASPUP */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        M3_NULL                            /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* R-ASP-INACTIVE */
    {
        m3ua_RASPINACTIVE_ESENDDN,     /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_RASPINACTIVE_ERECVDN,     /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_RASPINACTIVE_ERECVDNACK,  /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_RASPINACTIVE_ERECVUP,     /* M3_ASPM_EVT_RECV_ASPUP */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        m3ua_RASPINACTIVE_ERECVIA,     /* M3_ASPM_EVT_RECV_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        m3ua_RASPINACTIVE_ERECVAC,     /* M3_ASPM_EVT_RECV_ASPAC */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        M3_NULL                            /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* R-ASP-ACTIVE */
    {
        m3ua_RASPACTIVE_ESENDDN,       /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_RASPACTIVE_ERECVDN,       /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_RASPACTIVE_ERECVDNACK,    /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3ua_RASPACTIVE_ERECVUP,       /* M3_ASPM_EVT_RECV_ASPUP */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        m3ua_RASPACTIVE_ERECVIA,       /* M3_ASPM_EVT_RECV_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        m3ua_RASPACTIVE_ERECVAC,       /* M3_ASPM_EVT_RECV_ASPAC */
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        M3_NULL                            /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* R-ASP-DNRECV */
    {
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPDN */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPIA */
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPAC */
        m3ua_RASPDNRECV_ERECVDN,       /* M3_ASPM_EVT_RECV_ASPDN */
        m3ua_RASPDNRECV_ERECVDNACK,    /* M3_ASPM_EVT_RECV_ASPDN_ACK */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPUP */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPUP_ACK */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPIA */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPIA_ACK */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPAC */
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPAC_ACK */
        m3ua_RASPDNRECV_ETIMEREXP      /* M3_ASPM_EVT_TIMER_EXPR */
    },

    /* R-ASP-UPRECV */
    {
        m3ua_RASPUPRECV_ESENDDN,       /* M3_ASPM_EVT_SWTO_ASPDN */    
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPIA */    
        M3_NULL,                           /* M3_ASPM_EVT_SWTO_ASPAC */    
        m3ua_RASPUPRECV_ERECVDN,       /* M3_ASPM_EVT_RECV_ASPDN */    
        m3uaVoid,                       /* M3_ASPM_EVT_RECV_ASPDN_ACK */    
        m3ua_RASPUPRECV_ERECVUP,       /* M3_ASPM_EVT_RECV_ASPUP */
        m3ua_RASPUPRECV_ERECVUPACK,    /* M3_ASPM_EVT_RECV_ASPUP_ACK */    
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA */    
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPIA_ACK */    
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC */    
        M3_NULL,                           /* M3_ASPM_EVT_RECV_ASPAC_ACK */    
        m3ua_RASPUPRECV_ETIMEREXP      /* M3_ASPM_EVT_TIMER_EXPR */    
    }
};

m3_s32 m3uaASPM(m3_conn_inf_t *pconn,
                m3_aspm_evt_t event,
                void          *p_inf)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            retval_1 = -1;
    m3_s32            retval_2 = -1;

    iu_log_debug("\n pconn->conn_id = %d\n",pconn->conn_id);
    iu_log_debug(" pconn->l_sp_g_st = %d\n",pconn->l_sp_g_st);
    iu_log_debug(" pconn->r_sp_g_st = %d\n",pconn->r_sp_g_st);
    iu_log_debug(" pconn->l_sp_id = %d\n",pconn->l_sp_id);
    iu_log_debug(" pconn->r_sp_id = %d\n",pconn->r_sp_id);
    iu_log_debug(" pconn->opt = %d\n",pconn->opt);
    iu_log_debug(" event = %d\n",event);

    if (M3_TRUE == M3_IS_SP_ASP(pconn->l_sp_id)) {
        if (M3_NULL != m3uaAspmStateMachine[pconn->l_sp_g_st][event]) {
            retval_1 = m3uaAspmStateMachine[pconn->l_sp_g_st][event](pconn, p_inf);
        } else {
            M3TRACE(m3uaAspmTrace, ("Local ASPM state machine: Event:%u not handled in state:%u", event, pconn->l_sp_g_st));
        }
    }
    if (M3_TRUE == M3_IS_SP_ASP(pconn->r_sp_id)) {
        if (M3_NULL != m3uaRAspmStateMachine[pconn->r_sp_g_st][event]) {
            retval_2 = m3uaRAspmStateMachine[pconn->r_sp_g_st][event](pconn, p_inf);
        } else {
            M3TRACE(m3uaAspmTrace, ("Remote ASPM state machine: Event:%u not handled in state:%u", event, pconn->r_sp_g_st));
        }
    }
    iu_log_debug("retval_1 = %d, retval_2 = %d\n",retval_1, retval_2);
    if (-1 == retval_1 && -1 == retval_2)
    {
        M3ERRNO(EM3_INV_EVTSTATE);
        return -1;
    }
    return 0;
}

void m3uaProcASPSM(m3_conn_inf_t *pconn,
                   m3_u32        stream_id,
                   m3_msg_hdr_t  *p_hdr,
                   m3_u8         *p_msg,
                   m3_u32        msglen)
{
    m3_aspup_inf_t      aspup;
    m3_aspup_ack_inf_t  aspup_ack;
    m3_aspdn_inf_t      aspdn;
    m3_aspdn_ack_inf_t  aspdn_ack;
    m3_hbeat_inf_t      hbeat;
    m3_hbeat_ack_inf_t  hbeat_ack;
    m3_error_inf_t      err;

    switch(p_hdr->msg_type)
    {
        case M3_MSG_TYPE_ASPUP: {
            M3TRACE(m3uaInMsgTrace, ("Received ASPUP message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPUP(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN,
                p_msg+M3_MSG_HDR_LEN, &aspup)) {
                aspup.msglen = msglen;
                aspup.p_msg = p_msg;
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPUP, (void*)&aspup);
            }
            break;
        }
        case M3_MSG_TYPE_ASPDN: {
            M3TRACE(m3uaInMsgTrace, ("Received ASPDN message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPDN(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspdn)) {
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPDN, (void*)&aspdn);
            }
            break;
        }
        case M3_MSG_TYPE_BEAT: {
            M3TRACE(m3uaInMsgTrace, ("Received BEAT message"));
            if (-1 != m3uaDecodeHBEAT(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &hbeat)) {
                m3ua_ERECVHBEAT(pconn, (void *)&hbeat);
            }
            break;
        }
        case M3_MSG_TYPE_ASPUP_ACK: {
            M3TRACE(m3uaInMsgTrace, ("Received ASPUP ACK message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPUPACK(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspup_ack)) {
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPUP_ACK, &aspup_ack);
            }
            break;
        }
        case M3_MSG_TYPE_ASPDN_ACK: {
            M3TRACE(m3uaInMsgTrace, ("Received ASPDN ACK message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPDNACK(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspdn_ack)) {
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPDN_ACK, &aspdn_ack);
            }
            break;
        }
        case M3_MSG_TYPE_BEAT_ACK: {
            M3TRACE(m3uaInMsgTrace, ("Received BEAT ACK message"));
            if (-1 != m3uaDecodeHBEATACK(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &hbeat_ack)) {
                m3ua_ERECVHBEATACK(pconn, (void *)&hbeat_ack);
            }
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Unsupported ASPSM message type:%u", p_hdr->msg_type));
            err.err_code = EM3_UNSUPP_MSG_TYPE;
            err.num_rc = 0;
            err.num_pc = 0;
            err.nw_app = M3_MAX_U32;
            err.diag_len = msglen;
            err.p_diag_inf = p_msg;
            m3ua_ESENDERROR(pconn, &err);
            break;
        }
    }
    return;
}

void m3uaProcASPTM(m3_conn_inf_t *pconn,
                   m3_u32        stream_id,
                   m3_msg_hdr_t  *p_hdr,
                   m3_u8         *p_msg,
                   m3_u32        msglen)
{
    m3_aspac_inf_t      aspac;
    m3_aspac_ack_inf_t  aspac_ack;
    m3_aspia_inf_t      aspia;
    m3_aspia_ack_inf_t  aspia_ack;
    m3_error_inf_t      err;

    switch(p_hdr->msg_type)
    {
        case M3_MSG_TYPE_ASPIA: {
            M3TRACE(m3uaAspmTrace, ("Received ASPIA message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPIA(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN,
                p_msg+M3_MSG_HDR_LEN, &aspia)) {
                aspia.p_msg = p_msg;
                aspia.msglen = msglen;
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPIA, &aspia);
            }
            break;
        }
        case M3_MSG_TYPE_ASPAC: {
            M3TRACE(m3uaAspmTrace, ("Received ASPAC message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->r_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPAC(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspac)) {
                aspac.p_msg = p_msg;
                aspac.msglen = msglen;
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPAC, &aspac);
            }
            break;
        }
        case M3_MSG_TYPE_ASPIA_ACK: {
            M3TRACE(m3uaAspmTrace, ("Received ASPIA ACK message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPIAACK(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspia_ack)) {
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPIA_ACK, &aspia_ack);
            }
            break;
        }
        case M3_MSG_TYPE_ASPAC_ACK: {
            M3TRACE(m3uaAspmTrace, ("Received ASPAC ACK message"));
            if (M3_FALSE == M3_IS_SP_ASP(pconn->l_sp_id)) {
                return;
            }
            if (-1 != m3uaDecodeASPACACK(M3_NTOHL(p_hdr->msg_len)-M3_MSG_HDR_LEN, 
                p_msg+M3_MSG_HDR_LEN, &aspac_ack)) {
                m3uaASPM(pconn, M3_ASPM_EVT_RECV_ASPAC_ACK, &aspac_ack);
            }
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Unsupported ASPTM message type:%u", p_hdr->msg_type));
            err.err_code = EM3_UNSUPP_MSG_TYPE;
            err.num_rc = 0;
            err.num_pc = 0;
            err.nw_app = M3_MAX_U32;
            err.diag_len = msglen;
            err.p_diag_inf = p_msg;
            m3ua_ESENDERROR(pconn, &err);
            break;
        }
    }
    return;
}

