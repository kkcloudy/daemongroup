/****************************************************************************
** Description:
** External definitions
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

#ifndef __M3UA_EXTERN_H__
#define __M3UA_EXTERN_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3uaTraceMgr.h>

extern m3_asp_inf_t             m3_asp_table[M3_MAX_ASP];
extern m3_sgp_inf_t             m3_sgp_table[M3_MAX_SGP];
extern m3_as_inf_t              m3_as_table[M3_MAX_AS];
extern m3_r_asp_inf_t           m3_r_asp_table[M3_MAX_R_ASP];
extern m3_r_sgp_inf_t           m3_r_sgp_table[M3_MAX_R_SGP];
extern m3_r_as_inf_t            m3_r_as_table[M3_MAX_R_AS];
extern m3_conn_inf_t            m3_conn_table[M3_MAX_CONN];
extern m3_sg_inf_t              m3_sg_table[M3_MAX_SG];
extern m3_nwapp_inf_t           m3_nwapp_table[M3_MAX_NWAPP];
extern m3_usr_inf_t             m3_usr_table[M3_MAX_USR];
extern m3_local_rt_tbl_list_t   m3_local_route_table;
extern m3_rt_tbl_list_t         m3_route_table;
extern m3_mgmt_ntfy_inf_t       m3_mgmt_ntfy_table;
extern m3_user_ntfy_inf_t       m3_user_ntfy_table;
extern m3_prot_timer_t          m3_timer_table;
extern m3_u8                    m3_trace_table[m3uaMaxTrcType];

#endif

