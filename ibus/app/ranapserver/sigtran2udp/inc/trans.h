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
#define MAX_ASSOC	8

/*Parameter Tag*/
#define UE_ID_PT				0x01
#define RNC_ID_PT				0x02
#define RUA_MESSAGE_TYPE_PT		0x03
#define CN_DOMAIN_PT			0x04
#define CAUSE_PT				0x05
#define ESTABLISHMENT_CAUSE_PT 	0x06
#define USER_PROTOCOL_DATA_PT 	0x07

/*Choice UE Identity*/
#define CHOICE_UE_IDENTITY_IMSI		0x01
#define CHOICE_UE_IDENTITY_IMEI		0x02
#define CHOICE_UE_IDENTITY_IMEISV	0x03

/* Choice Cause Group*/
#define CHOICE_CAUSE_GROUP_RADIO_NETWORK_LAYER	0X01
#define CHOICE_CAUSE_GROUP_TRANSPORT_LAYER		0X02
#define CHOICE_CAUSE_GROUP_PROTOCOL_LAYER		0X03
#define CHOICE_CAUSE_GROUP_MISC_LAYER			0X04

typedef struct {
    int			e_state;
    int			sockid;
	int			socket_type;/*0 for udp, 1 for other*/
    struct sockaddr_in	addr;
} ep_addr_t;

ep_addr_t	from[MAX_LOCAL_EP];
ep_addr_t	to[MAX_REMOTE_EP];

typedef struct {
    int		e_state;
    int		from;
    int		to;
	int		isClient;
	int		socket_type;/*0 for udp, 1 for other*/
} assoc_ctx_t;

typedef struct {
    int			port;
    unsigned int	ipaddr;
} udp_addr_t;

typedef struct {
	unsigned char version;
	unsigned char mess_type;
	unsigned short mess_len;
	unsigned char* param_data; 
} private_head_t;

typedef struct {
	unsigned char param_id;
	unsigned char param_len;
	unsigned char* param_data;
}pri_param_t;

typedef struct {
	unsigned char param_id;
	unsigned char choice_type;
	unsigned short param_len;
	unsigned char* param_data;
}pri_param_choice_t;

typedef struct {
	unsigned char param_id;
	unsigned char user_pro_type;
	unsigned short user_pro_data_len;
	unsigned char* user_pro_data;
}user_param_t;

typedef struct {
	unsigned char version;
	unsigned char mess_type;
	unsigned short mess_len;
	pri_param_choice_t param_ueid;
	pri_param_t param_rncid;
	pri_param_t param_rua_message_type;
	pri_param_t param_cn_domain;
	pri_param_choice_t param_cause;
	pri_param_t param_establishment_cause;
	user_param_t user_protocal_data;
}pri_msg_t;

int	make_assoc(int			fid,
                   int			tid,
				int			isServ);

int	make_lep(udp_addr_t	addr);

int	make_rep(udp_addr_t	addr);

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

m3_s32	m3ua_sendmsg_udp(m3_u32		assoc_id,
                     m3_u32		stream_id,
                     m3_u32             msglen,
                     m3_u8              *pmsg);


#endif

