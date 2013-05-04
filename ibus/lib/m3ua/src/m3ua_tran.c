/****************************************************************************
** Description:
** Code for neccesary transport services.
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

#include <m3ua_tran.h>

m3_u32 m3_assoc_conn_map[M3_MAX_ASSOCID + 1];

m3_s32 m3ua_recvmsg(m3_u32     assoc_id,
                    m3_u32     stream_id,
                    m3_u32     msglen,
                    m3_u8      *pmsg)
{
    m3_u32         conn_id;
    m3_conn_inf_t  *pconn;
	iu_log_debug("<<<<<<<<<%s  %d >>>>>>>>>>>>>>>\n",__FILE__, __LINE__);			
    iu_log_debug("assoc_id = %d\n",assoc_id);
    if (M3_MAX_ASSOCID <= assoc_id)
        conn_id = m3_assoc_conn_map[assoc_id];
    else {
        for (conn_id = 0; conn_id < M3_MAX_CONN; conn_id++) {
            pconn = M3_CONN_TABLE(conn_id);
            //iu_log_debug("conn_id = %d\n",conn_id);
            
            if (M3_TRUE == pconn->e_state              && 
                assoc_id == pconn->assoc_id            &&
                M3_CONN_NOT_ESTB != pconn->conn_state) {
                iu_log_debug("assoc_id = %d, pconn->e_state = %d\n pconn->conn_state = %d\n",assoc_id, pconn->e_state, pconn->conn_state);
                break;
            }
        }
    }
    if (M3_MAX_CONN == conn_id)  {
        M3ERRNO(EM3_INV_ASSOCID);
        return -1;
    }
	
	iu_log_debug(" @@@@@@@@@@@@@@   conn_id = %d  @@@@@@@@@@@@@@@@\n", conn_id);
	
    M3TRACE(m3uaInMsgTrace, ("Received Message: connId:%u, assocId:%u, streamId:%u",
            conn_id, assoc_id, stream_id));
    M3HEX(m3uaInMsgTrace, pmsg, msglen);
    m3uaProcMsg(conn_id, stream_id, pmsg, msglen);
    return 0;
}

