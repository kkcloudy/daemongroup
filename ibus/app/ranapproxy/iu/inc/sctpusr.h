/****************************************************************************
** Description:
*****************************************************************************
** Copyright(C) 2005 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
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

#ifndef __SCTPUSR_H__
#define __SCTPUSR_H__

#include <m3ua_defines.h>
#include <m3ua_types.h>
#include <m3ua_api.h>

#define MAX_LOCAL_EP	4
#define MAX_REMOTE_EP	4
#define MAX_ASSOC	16

typedef struct {
    int			e_state;
    int			sockid;
	int 		islocal;
	int         issecondary;
    struct sockaddr_in	addr[2];
} ep_addr_t;

ep_addr_t	from[MAX_LOCAL_EP];
ep_addr_t	to[MAX_REMOTE_EP];

typedef struct {
    int		e_state;
    int		from;
    int		to;
	int		isClient;
} assoc_ctx_t;

typedef struct {
    int			port;
    unsigned int	ipaddr;
} udp_addr_t;


int	make_assoc_v2(int fid, int tid, unsigned char isServ, unsigned char multi_switch);



int	make_lep_v2(udp_addr_t	p_addr, udp_addr_t	s_addr, m3_u32 proc, m3_u8 multi_switch);

int	make_rep_v2(udp_addr_t	p_addr, udp_addr_t	s_addr, m3_u32 proc, m3_u8 multi_switch);

int	lep_exist(udp_addr_t	ep_addr);

int     rep_exist(udp_addr_t    ep_addr);

int     assoc_exist(int		fid,
                    int		tid);

int	del_lep(int		fid);

int	del_rep(int		tid);

int	close_assoc(int			assoc_id);

int	init_transport(void);

m3_s32	m3ua_sendmsg(m3_u32		assoc_id,
                     m3_u32		stream_id,
                     m3_u32             msglen,
                     m3_u8              *pmsg);

void*	waitfor_message(void	*not_used);

int	get_message(int		fid,
                    int		sockid,
                    unsigned char msg[1024],
                    unsigned int  *pmsglen);

int clear_assoc(m3_u32 proc);
int get_sctp_status(m3_u32 proc, int *stat);

#endif

