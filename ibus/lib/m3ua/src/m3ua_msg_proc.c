/****************************************************************************
** Description:
** Code for Processing received messages.
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

#include <m3ua_msg_proc.h>


void m3uaProcMsg(m3_u32       connId,
                 m3_u32       streamId,
                 m3_u8        *pMsg,
                 m3_u32       msgLen)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_msg_hdr_t    *pHdr;
    m3_conn_inf_t   *pConn = M3_CONN_TABLE(connId);
    m3_error_inf_t  errinf;

/*17-March-2010 -- Commenting to avoid double printing */
#if 0
    M3TRACE(m3uaInMsgTrace, ("Received Message on connection:%u, stream:%u", connId, streamId));
    M3HEX(m3uaInMsgTrace, pMsg, msgLen);
#endif

    if (M3_TRUE == pConn->hbeat_sess_inf.e_state) {
        m3uaStopHBEAT(pConn);
        m3uaNtfyConnState(pConn, M3_CONN_ALIVE);
    }
    if (M3_MSG_HDR_LEN > msgLen) {
        return;
    }
    pHdr = (m3_msg_hdr_t*)pMsg;
    if (M3_MSG_VERSION != pHdr->version) {
        M3TRACE(m3uaErrorTrace, ("Incorrect version:%u received in incoming message", pHdr->version));
        errinf.err_code = EM3_INV_VERSION;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = msgLen;
        errinf.p_diag_inf = pMsg;
        m3ua_ESENDERROR(pConn, &errinf);
        return;
    }
    if (msgLen < M3_NTOHL(pHdr->msg_len)) {
        M3TRACE(m3uaErrorTrace, ("Incorrect message length:%u of received message", msgLen));
        return;
    }
    switch(pHdr->msg_class) {
        case M3_MSG_CLASS_MGMT: {
            m3uaProcMGMT(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        case M3_MSG_CLASS_TXR: {
            m3uaProcTXR(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        case M3_MSG_CLASS_SSNM: {
            m3uaProcSSNM(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        case M3_MSG_CLASS_ASPSM: {
            m3uaProcASPSM(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        case M3_MSG_CLASS_ASPTM: {
            m3uaProcASPTM(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        case M3_MSG_CLASS_RKM: {
            m3uaProcRKM(pConn, streamId, pHdr, pMsg, msgLen);
            break;
        }
        default: {
            M3TRACE(m3uaErrorTrace, ("Unsupported message class:%u", pHdr->msg_class));
            errinf.err_code = EM3_UNSUPP_MSG_CLASS;
            errinf.num_rc = 0;
            errinf.num_pc = 0;
            errinf.nw_app = M3_MAX_U32;
            errinf.diag_len = msgLen;
            errinf.p_diag_inf = pMsg;
            m3ua_ESENDERROR(pConn, &errinf);
            return;
        }
    }
    if (EM3_MISSING_PARAM == m3errno || EM3_UNEXP_PARAM == m3errno) {
        errinf.err_code = EM3_MISSING_PARAM;
        errinf.num_rc = 0;
        errinf.num_pc = 0;
        errinf.nw_app = M3_MAX_U32;
        errinf.diag_len = msgLen;
        errinf.p_diag_inf = pMsg;
        m3ua_ESENDERROR(pConn, &errinf);
    }
    return;
}

