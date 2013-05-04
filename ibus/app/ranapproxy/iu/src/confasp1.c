/****************************************************************************
** Description:
** Code to describe usage of M3UA API's.
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
#include <fcntl.h>
#include <confasp1.h>
#include "dbus/iu/IuDBusPath.h"
#include "sccp.h"
#include "iu_log.h"
#include "iu_dbus.h"
#include "iuh/Iuh.h"
#include <pthread.h>
#include <sys/un.h>
#define UNIX_PATH_MAX    108

#define SOCKPATH_IU "/var/run/femto/iu"
#define SOCKPATH_IUH "/var/run/femto/iuh"
#define SGP_PROCESS         0x00010000


int num_packets_sent;
int cc_success;
int starting_iu_service;
int vrrid = 0;
int local = 1;
int slotid = 0;
unsigned int sctp_connect_mode_msc = 0;
unsigned int sctp_connect_mode_sgsn = 0;
/* book add for as configuration */
asConfType_t msc_as_conf;
asConfType_t sgsn_as_conf;


/* book add ,2011-12-21 */
Iu_UE  **IU_UE;


/* book add, 2011-12-2 */
void m3ua_set_gNi(int ni)
{
	gNi = ni;
	return;
}

void asp_set_sctp_cn_mode(unsigned char conn_mode, unsigned char isps)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("conn_mode = %d, isps = %d\n",conn_mode, isps);
	if(isps)
		sctp_connect_mode_sgsn = conn_mode;
	else
		sctp_connect_mode_msc = conn_mode;
	
	return ;
}


/* book add for set paras of as & r_as, 2011-3-15 */
void as_set_routing_context(m3_u32 rtctx, m3_u8 isps) //flag = 0 as,  flag = 1 r_as
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("rtctx = %d, isps = %d\n",rtctx, isps);
    if(isps == 0){
        msc_as_conf.routingContext = rtctx;
    }
    else{
        sgsn_as_conf.routingContext = rtctx;
    }
    return;
}

void as_set_traffic_mode(m3_u32 trfmode, m3_u8 isps) 
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("trfmode = %d, isps = %d\n",trfmode, isps);
    if(isps == 0){
        msc_as_conf.trafficMode = trfmode;
    }
    else{
        sgsn_as_conf.trafficMode = trfmode;
    }
    return;
}

void as_set_network_apperance(m3_u32 nwapp, m3_u8 isps) 
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    iu_log_debug("nwapp = %d, isps = %d\n",nwapp, isps);
    if(isps == 0){
        msc_as_conf.networkApperance= nwapp;
    }
    else{
        sgsn_as_conf.networkApperance= nwapp;
    }
    return;
}


m3_u32 as_get_traffic_mode(m3_u8 isps)
{
    if(isps == 0)
        return msc_as_conf.trafficMode;
    else
        return sgsn_as_conf.trafficMode;
}

m3_u32 as_get_routing_context(m3_u8 isps)
{
    if(isps == 0)
        return msc_as_conf.routingContext;
    else
        return sgsn_as_conf.routingContext;
}
/* book add end */


#define ASP_STATE(state, p)	if (0 == state) p = "DOWN"; \
				else if (1 == state) p = "INACTIVE"; \
				else if (2 == state) p = "ACTIVE";

m3_s32	m3ua_sendmsg_udp(m3_u32		assoc_id,
                     m3_u32		stream_id,
                     m3_u32             msglen,
                     m3_u8              *pmsg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	return 0;
}

/***************************************************************************
** Create a local endpoint
****************************************************************************/
int	local_socket_init()
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int idx = 2;
	//int sd_send, sd_rcv;
	struct sockaddr_un iu_end;
	//socklen_t hisend_len;
/*
	sd_send=socket(PF_LOCAL, SOCK_DGRAM, 0);
	if (sd_send==-1) {
		perror("socket()");
	}

	iuh_end.sun_family = AF_UNIX;
	strncpy(iuh_end.sun_path, SOCKPATH_IUH, UNIX_PATH_MAX);
*/
iu_log_debug("MAX_LOCAL_EP = %d\n", MAX_LOCAL_EP);
	/*
    for (idx = 0; idx < MAX_LOCAL_EP; idx++) {
        if (0 == from[idx].e_state) {
        iu_log_debug("from idx = %d\n",idx);
			iu_end.sun_family = AF_UNIX;
			strncpy(iu_end.sun_path, SOCKPATH_IU, UNIX_PATH_MAX);
            from[idx].e_state		= 1;
            from[idx].sockid = socket(PF_LOCAL, SOCK_DGRAM, 0);
			from[idx].islocal = 1;
			unlink(SOCKPATH_IU);
            if (-1 == bind(from[idx].sockid, (struct sockaddr*)&iu_end, sizeof(iu_end))) {
				iu_log_debug("start path fd is %d idx is %d\n", from[idx].sockid, idx);
                perror("E_bind_Fail for start path iu");
                return -1;
            }
            return idx;
        }
    }
    */
	if (0 == from[idx].e_state) 
	{
        iu_log_debug("from idx = %d\n",idx);
			iu_end.sun_family = AF_UNIX;
			strncpy(iu_end.sun_path, SOCKPATH_IU, UNIX_PATH_MAX);
            from[idx].e_state		= 1;
            from[idx].sockid = socket(PF_LOCAL, SOCK_DGRAM, 0);
			from[idx].islocal = 1;
			unlink(SOCKPATH_IU);
            if (-1 == bind(from[idx].sockid, (struct sockaddr*)&iu_end, sizeof(iu_end))) {
				iu_log_debug("start path fd is %d idx is %d\n", from[idx].sockid, idx);
                perror("E_bind_Fail for start path iu");
                return -1;
            }
            return idx;
        }
    return -1;
}
int IuServerTipcInit(void)
{
	iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int idx = 2;
	struct sockaddr_tipc server_addr;
	if (0 == from[idx].e_state)
	{
		iu_log_debug("from idx = %d\n",idx);
		server_addr.family = AF_TIPC;
		server_addr.addrtype = TIPC_ADDR_NAMESEQ;
		server_addr.addr.nameseq.type = IU_TIPC_SERVER_TYPE;
		server_addr.addr.nameseq.lower = FEMTO_SERVER_BASE_INST + \
		(slotid-1)*IUH_MAX_INS_NUM*2 + vrrid + local;
		server_addr.addr.nameseq.upper = FEMTO_SERVER_BASE_INST + \
		(slotid-1)*IUH_MAX_INS_NUM*2 + vrrid + local;
		server_addr.scope = TIPC_ZONE_SCOPE;
		
		from[idx].e_state= 1;
		from[idx].sockid = socket(AF_TIPC, SOCK_RDM, 0);
		from[idx].islocal = 1;

		if(-1 == bind(from[idx].sockid, (struct sockaddr*)&server_addr, sizeof(server_addr)))
		{
			iu_log_debug("start path fd is %d idx is %d\n", from[idx].sockid, idx);
			perror("IU Tipc bind error!!!");
			return -1;
		}
		 return idx;
	}
	return -1;	
}
int IuClientTipcInit(void)
{
	//implementation in sccp.c sccp_send_msg2iuh() & sccp_iu2iuh_tipc_socket_init()
}

/*----------------------------------------------------
** DISCRIPTION:
**          aingle unit for either msc or sgsn
** INPUT:
**          local ip, local port, msc ip ,msc port, sgsn ip, sgsn port
** OUTPUT:
**          none
** RETURN:
**          0
** book add, 2011-12-29
-----------------------------------------------------*/
int um3_transport_init_arg_v2(m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int l_ep = 0;
    int r_ep = 0;
    udp_addr_t laddr_p;
    udp_addr_t laddr_s;
    udp_addr_t raddr_p;
    udp_addr_t raddr_s;
    unsigned char local_multi_switch = 0;
    unsigned char remote_multi_switch = 0;
    unsigned char connmode = 0;
    
    if(ASP_PORCESS_MSC == proc)
    {
        laddr_p.ipaddr = home_gateway_msc.primary_ip;
        laddr_p.port   = home_gateway_msc.primary_port;
        laddr_s.ipaddr = home_gateway_msc.secondary_ip;
        laddr_s.port   = home_gateway_msc.secondary_port;
        local_multi_switch = home_gateway_msc.multi_switch;
        
        raddr_p.ipaddr = global_msc_parameter.primary_ip; 
        raddr_p.port   = global_msc_parameter.primary_port;
        raddr_s.ipaddr = global_msc_parameter.secondary_ip; 
        raddr_s.port   = global_msc_parameter.secondary_port;
        remote_multi_switch = global_msc_parameter.multi_switch;

        connmode = home_gateway_msc.connect_mode;
    }
    else if(ASP_PORCESS_SGSN == proc)
    {
        laddr_p.ipaddr = home_gateway_sgsn.primary_ip;
        laddr_p.port   = home_gateway_sgsn.primary_port;
        laddr_s.ipaddr = home_gateway_sgsn.secondary_ip;
        laddr_s.port   = home_gateway_sgsn.secondary_port;
        local_multi_switch = home_gateway_sgsn.multi_switch;
        
        raddr_p.ipaddr = global_sgsn_parameter.primary_ip; 
        raddr_p.port   = global_sgsn_parameter.primary_port;
        raddr_s.ipaddr = global_sgsn_parameter.secondary_ip; 
        raddr_s.port   = global_sgsn_parameter.secondary_port;
        remote_multi_switch = global_sgsn_parameter.multi_switch;

        connmode = home_gateway_sgsn.connect_mode;
    }
    
    /* add local info */
    l_ep = make_lep_v2(laddr_p, laddr_s, proc, local_multi_switch);  
	/* add remote info */
    r_ep = make_rep_v2(raddr_p, raddr_s, proc, remote_multi_switch);
    iu_log_debug("l_ep = %d, r_ep = %d\n",l_ep, r_ep);
    
    /* create associations between the transport layer endpoints */
    /* add socket_fd into fd_set readfds */
    if(-1 == make_assoc_v2(l_ep, r_ep, connmode, remote_multi_switch))
    {
    	return -1;
    }
	
    return 0;
}



m3_s32 um3_m3ua_r_asp(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_s32            ret;
    m3ua_r_asp_t      api;
    m3_u32            rasp = proc;
iu_log_debug("oprn = %d\n",oprn);
    switch(oprn) {
        case M3_ADD: {
            api.add.info.sctp_ep_id = proc; /*msc is 0, sgsn is 1*/
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
    }
    ret = m3ua_r_asp(rasp, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_r_as(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    m3_s32              ret;
    m3ua_r_as_t         api;
    m3_u32              ras = proc;
    m3_r_as_confname_t  confname = M3_R_AS_RKEY;
    switch(oprn) {
        case M3_ADD: {
            //api.add.info.rtctx = 7;
            api.add.info.min_act_asp = 1; /* reqd only for ldshr/brdcst */
            //api.add.info.rkey.trfmode = M3_TRFMODE_OVERRIDE;
	//          api.add.info.rkey.nw_app = 0x01ABCDEF;
			api.add.info.rkey.nw_app = M3_MAX_U32;
            api.add.info.rkey.num_rtparam = 1;
            if(proc == ASP_PORCESS_MSC){
                api.add.info.rtctx = msc_as_conf.routingContext;
                api.add.info.rkey.trfmode = msc_as_conf.trafficMode;
                api.add.info.rkey.rtparam[0].dpc = global_msc_parameter.point_code;
                iu_log_debug("rtctx = %d, trfmode = %d\n",api.add.info.rtctx,api.add.info.rkey.trfmode);           
                iu_log_debug("global_msc_dpc = %d\n",global_msc_parameter.point_code);
                api.add.info.rkey.rtparam[0].num_si = 2;
                api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
                api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
                api.add.info.rkey.rtparam[0].num_opc = 3;
                api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010203;
                api.add.info.rkey.rtparam[0].opc_list[1] = home_gateway_msc.point_code;/*change later */
                iu_log_debug("global_opc = %d\n",home_gateway_msc.point_code);
                api.add.info.rkey.rtparam[0].opc_list[2] = 0x00010305;
                api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            }
            else if(proc == ASP_PORCESS_SGSN){
                api.add.info.rtctx = sgsn_as_conf.routingContext;
                api.add.info.rkey.trfmode = sgsn_as_conf.trafficMode;
                api.add.info.rkey.rtparam[0].dpc = global_sgsn_parameter.point_code;
                iu_log_debug("rtctx = %d, trfmode = %d\n",api.add.info.rtctx,api.add.info.rkey.trfmode);            
                iu_log_debug("global_sgsn_dpc = %d\n",global_sgsn_parameter.point_code);
                api.add.info.rkey.rtparam[0].num_si = 2;
                api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
                api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
                api.add.info.rkey.rtparam[0].num_opc = 3;
                api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010203;
                api.add.info.rkey.rtparam[0].opc_list[1] = home_gateway_sgsn.point_code;/*change later */
                iu_log_debug("global_opc = %d\n",home_gateway_sgsn.point_code);
                api.add.info.rkey.rtparam[0].opc_list[2] = 0x00010305;
                api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            }
            
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_R_AS_RTCTX: {
                /* modify routing context to 0x55667788 */
                    api.modify.info.rtctx = M3_MAX_U32;
                    break;
                }
                case M3_R_AS_RKEY: {
                /* modify the routing key configured for the AS */
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
  //                  api.modify.info.rkey.nw_app = 0x01ABCDEF;
 					 api.modify.info.rkey.nw_app = M3_MAX_U32;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
                case M3_R_AS_MIN_ACT_ASP: {
                /* modify minimum active asp requirement for this as */
                    api.modify.info.min_act_asp = 2;
                }
                case M3_R_AS_INFO: {
                    /* modify both routing context and routing key */
                   api.modify.info.rtctx = M3_MAX_U32;
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
  //                  api.modify.info.rkey.nw_app = 0x01ABCDEF;
 					api.modify.info.rkey.nw_app =  M3_MAX_U32;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_r_as(ras, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}


/*
** DISCRIPTION:
**          start m3ua server for either msc or sgsn
** INPUT:
**          local ip, local port, msc ip ,msc port, sgsn ip, sgsn port
** OUTPUT:
**          none
** RETURN:
**          0
*/
int um3_server_start(m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    
	starting_iu_service = 1;
	int fid = proc;
	int tid = proc;
	int idx = fid*MAX_REMOTE_EP + tid;

    /**** Initialise the Transport Layer ****/
    if(-1 == um3_transport_init_arg_v2(proc))
    {
        iu_log_error("Error: init msc transport arg failed\n");
		return -1;
    }

	/* Add nwapp */
    um3_m3ua_nwapp(M3_ADD, proc);
    
    /******** Configure M3UA managed objects ********/
	/* Add remote asp */   
	/* add information into m3_r_asp_table by PROCESS_ID, need add sgsn */
	um3_m3ua_r_asp(M3_ADD, proc);

    /* Add remote as */
    /* add information into m3_r_as_table by PROCESS_ID, need add sgsn */
    um3_m3ua_r_as(M3_ADD, proc);

    /* Add as */
    um3_m3ua_as(M3_ADD, proc);
    
    /* Add asp */
    um3_m3ua_asp(M3_ADD, proc);
    
    /* Add user(s) for asp */
    um3_m3ua_user(M3_ADD, proc);
    
    /* Add routes */
   	um3_m3ua_route(M3_ADD, proc);
    
    /* Add connection's */
    um3_m3ua_conn(M3_ADD, proc, proc, idx);
    
    /* change connection state */
    um3_m3ua_conn_state(M3_MODIFY, M3_CONN_ESTB, proc, proc);

    if(((ASP_PORCESS_MSC==proc)&&(!sctp_connect_mode_msc)) ||
        ((ASP_PORCESS_SGSN==proc)&&(!sctp_connect_mode_sgsn)))
    {		
		/* send aspup from asp to remote asp */
		um3_m3ua_asp_state(M3_MODIFY, M3_ASP_INACTIVE, proc);
	}
	
	starting_iu_service = 0;
    // waitfor_message(NULL);
    
    return 0;
}


int um3_get_asp1_addr(char *argv[], udp_addr_t *paddr)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("asp", argv[1], 3))
        return -1;
    p = strchr(argv[1], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
iu_log_debug("at asp.c asp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}


int um3_get_sgp1_addr(char *argv[], udp_addr_t *paddr)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("sgp", argv[2], 3))
        return -1;
    p = strchr(argv[2], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
iu_log_debug("at asp.c sgp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}


m3_s32 um3_m3ua_init(void)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32        ret;

    ret = m3ua_init();
    return ret;
}

m3_s32 um3_m3ua_nwapp(m3_u8   oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32         ret;
    m3ua_nwapp_t   api;

    switch(oprn) {
        case M3_ADD: {
            if(ASP_PORCESS_MSC == proc)
			    api.add.info.nw_app = msc_as_conf.networkApperance;	//book add for Thailand, 2011-12-09
			else if(ASP_PORCESS_SGSN == proc)
			    api.add.info.nw_app = sgsn_as_conf.networkApperance;
            //api.add.info.nw_app = 0x01ABCDEF;
            api.add.info.standard = M3_STD_ANSI;
            break;
        }
        case M3_DELETE: {
            if(ASP_PORCESS_MSC == proc)
			    api.add.info.nw_app = msc_as_conf.networkApperance;	//book add for Thailand, 2011-12-09
			else if(ASP_PORCESS_SGSN == proc)
			    api.add.info.nw_app = sgsn_as_conf.networkApperance;
            break;
        }
    }
    iu_log_debug("api.add.info.nw_app = %d\n",api.add.info.nw_app);
    ret = m3ua_nwapp(oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_asp(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_asp_t        api;
    m3_u32            asp = proc;
    m3_u16            idx, idx2;
    m3_asp_confname_t confname = M3_ASP_ADD_R_ASP;
    iu_log_debug("asp = %d\n",asp);

    switch(oprn) {
        case M3_ADD: {
            api.add.info.m3asp_id = (0x0000ABCD + proc);
            api.add.info.sctp_ep_id = proc;	//msc is 0 , sgsn is 1
            api.add.info.def_nwapp = M3_MAX_U32;

            for (idx = 0; idx < M3_MAX_AS; idx++)
                api.add.info.as_list[idx] = M3_FALSE;
                
            api.add.info.as_list[proc] = M3_TRUE;
            
            for (idx = 0; idx < M3_MAX_R_AS; idx++) //OCT2009
                for (idx2 = proc; idx2 < M3_MAX_R_ASP; idx2++) //OCT2009
                    api.add.info.r_as_inf[idx].asp_list[idx2] = M3_FALSE;
            api.add.info.r_as_inf[proc].asp_list[proc] = M3_TRUE;
            iu_log_debug("api.add.info.r_as_inf[%d].asp_list[%d] = %d\n",proc,proc,api.add.info.r_as_inf[proc].asp_list[proc]);
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_ASP_M3ASP_ID: {
                /* modify ASP ID sent in ASPUP/NTFY message for this ASP */
                    api.modify.info.m3asp_id = 0x0000EF12;
                    break;
                }
                case M3_ASP_NWAPP: {
                /* modify default Network Appearance to 0x12345678 for */
                /* this ASP                                            */

                    api.modify.info.def_nwapp = M3_MAX_U32;

                    break;
                }
                case M3_ASP_ADD_AS: {
                /* Add an AS with id 1 to the ASP */
                    api.modify.as_id = 1;
                    break;
                }
                case M3_ASP_DEL_AS: {
                /* Delete AS with id 3 to the ASP */
                    api.modify.as_id = 3;
                    break;
                }
                case M3_ASP_ADD_R_ASP: {
                /* Add remote asp 2 serving remote as 4 to ASP */
                    api.modify.ras_id = 4;
                    api.modify.rasp_id = 2;
                    break;
                }
                case M3_ASP_DEL_R_ASP: {
                /* Delete remote asp 3 serving remote as 3 from ASP */
                    api.modify.ras_id = 3;
                    api.modify.rasp_id = 3;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    iu_log_debug("call m3ua_asp() function\n");
    ret = m3ua_asp(asp, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_as(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_as_t         api;
    m3_u32            as = proc;
    m3_as_confname_t  confname = M3_AS_RKEY;

    switch(oprn) {
        case M3_ADD: {

            api.add.info.rkey.nw_app = M3_MAX_U32;

            
            if(proc == ASP_PORCESS_MSC){
                api.add.info.rkey.num_rtparam = 1;
                api.add.info.rkey.rtparam[0].dpc = home_gateway_msc.point_code; /* 24 bit pointcode -- ANSI */
                iu_log_debug("global_msc_opc = %d\n",home_gateway_msc.point_code);
                api.add.info.rkey.rtparam[0].num_si = 2;
                api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
                api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
                api.add.info.rkey.rtparam[0].num_opc = 4;
                api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010202;
                api.add.info.rkey.rtparam[0].opc_list[1] = 0x00010203;
                api.add.info.rtctx = msc_as_conf.routingContext;
                api.add.info.rkey.trfmode = msc_as_conf.trafficMode;
                api.add.info.rkey.rtparam[0].opc_list[2] = global_msc_parameter.point_code;
            }
            else if(proc == ASP_PORCESS_SGSN){
                api.add.info.rkey.num_rtparam = 1;
                api.add.info.rkey.rtparam[0].dpc = home_gateway_sgsn.point_code; /* 24 bit pointcode -- ANSI */
                iu_log_debug("global_sgsn_opc = %d\n",home_gateway_sgsn.point_code);
                api.add.info.rkey.rtparam[0].num_si = 2;
                api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
                api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
                api.add.info.rkey.rtparam[0].num_opc = 4;
                api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010202;
                api.add.info.rkey.rtparam[0].opc_list[1] = 0x00010203;
                api.add.info.rtctx = sgsn_as_conf.routingContext;
                api.add.info.rkey.trfmode = sgsn_as_conf.trafficMode;
                api.add.info.rkey.rtparam[0].opc_list[2] = global_sgsn_parameter.point_code;
            }
            iu_log_debug("rtctx = %d, trfmode = %d\n",api.add.info.rtctx,api.add.info.rkey.trfmode);
            iu_log_debug("global_sgsn_dpc = %d\n",api.add.info.rkey.rtparam[0].opc_list[2]);
            api.add.info.rkey.rtparam[0].opc_list[3] = 0x00010305;
            api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            break;
        }
        case M3_DELETE: 
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_AS_RTCTX: {
                /* modify routing context to 0x55667788 */
                    api.modify.info.rtctx = M3_MAX_U32;
                    break;
                }
                case M3_AS_RKEY: {
                /* modify the routing key configured for the AS */
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = M3_MAX_U32;

                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
                case M3_AS_INFO: {
                    /* modify both routing context and routing key */
                    api.modify.info.rtctx = M3_MAX_U32;
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = M3_MAX_U32;

                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_as(as, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_r_sgp(m3_u8    oprn)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_r_sgp_t      api;
    m3_u32            rsgp = 0x00010000;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sctp_ep_id = 1;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
    }
    ret = m3ua_r_sgp(rsgp, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_sg(m3_u8    oprn)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_sg_t         api;
    //m3_u32            sg = 0x00010000;
    m3_u32            sg = 0x00010000;
    m3_sg_confname_t  confname = M3_SG_MODE;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sgmode = M3_SGMODE_LOADSHARE;
            api.add.info.num_sgp = 1;
            //api.add.info.sgp_list[0] = 0x00010000;
            api.add.info.sgp_list[0] = 0x00010000;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_SG_MODE: {
                    api.modify.info.sgmode = M3_SGMODE_BROADCAST;
                    break;
                }
                case M3_SG_SGP_LIST: {
                    api.modify.info.num_sgp = 2;
                    api.modify.info.sgp_list[0] = 0x00010000;
                    api.modify.info.sgp_list[1] = 0x00010001;
                    break;
                }
                case M3_SG_INFO: {
                    api.modify.info.sgmode = M3_SGMODE_BROADCAST;
                    api.modify.info.num_sgp = 2;
                    api.modify.info.sgp_list[0] = 0x00010000;
                    api.modify.info.sgp_list[1] = 0x00010001;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_sg(sg, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_conn(m3_u8 oprn, m3_u32 lsp, m3_u32 rsp, m3_u32 assoc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32             ret;
    m3ua_conn_t        api;
    m3_conn_confname_t confname = M3_CONN_I_STR;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.assoc_id = assoc;
            api.add.info.i_str = 8;
            api.add.info.o_str = 8;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            api.add.info.assoc_id = assoc;//book add, 2011-12-30
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_CONN_ASSOC: {
                    api.add.info.assoc_id = assoc;
                    break;
                }
                case M3_CONN_I_STR: {
                    api.add.info.i_str = 6;
                    break;
                }
                case M3_CONN_O_STR: {
                    api.add.info.o_str = 6;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_conn(lsp, rsp, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_conn_state(m3_u8 oprn, m3_conn_state_t state, m3_u32 lsp, m3_u32 rsp)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_conn_state_t api;

    switch(oprn) {
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            api.modify.state = state;
            break;
        }
    }
    ret = m3ua_conn_state(lsp, rsp, oprn, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_asp_state(m3_u8 oprn, m3_asp_state_t state, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_asp_state_t  api;
    m3_u32            asp = proc;
    m3_s8             *asp_state;

    ASP_STATE(state, asp_state);
    switch(oprn) {
        case M3_GET: {
            api.get.info.as_id = proc;
            break;
        }
        case M3_MODIFY: {
        /* modify asp state on both the connections */
            api.modify.info.num_as = 1;
            api.modify.info.as_list[proc] = proc;//book modify
            api.modify.state = state;
            break;
        }
    }
    iu_log_debug("<---- Initiating ASPM procedure to make ASP %s ---->\n", asp_state);
#ifndef SIMULATOR_CN
    ret = m3ua_asp_state(asp, proc, oprn, &api);
#else
    ret = m3ua_asp_state(asp, 0x00010000, oprn, &api);
#endif
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}


m3_s32 um3_m3ua_route(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_route_t      api;

    switch(oprn) {
        case M3_ADD: {
        /* add routes through configured SG(s) */
       //    api.add.info.le_id = 0x00010000;
			api.add.info.le_id = proc;
            api.add.info.priority = 1;
            api.add.info.pc_inf.nw_app = M3_MAX_U32;
            iu_log_debug("global_msc_dpc = %d, global_sgsn_dpc = %d\n",global_msc_parameter.point_code, global_sgsn_parameter.point_code);
            api.add.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);

            iu_log_debug("proc = %d\n",proc);
            if(ASP_PORCESS_MSC == proc){
                api.add.info.pc_inf.ptcode = global_msc_parameter.point_code;
            }
            else if(ASP_PORCESS_SGSN == proc){
                api.add.info.pc_inf.ptcode = global_sgsn_parameter.point_code;
            }
            iu_log_debug("1111111111111111111111111 api.add.info.pc_inf.ptcode = %d\n",api.add.info.pc_inf.ptcode);
                
            ret = m3ua_route(oprn, &api);
            api.add.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
        case M3_DELETE: {
//                 api.del.info.le_id = 0x00010000;
			api.del.info.le_id = proc;
            api.del.info.priority = 1;  
            api.del.info.pc_inf.nw_app = M3_MAX_U32;
            api.del.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);
            if(ASP_PORCESS_MSC == proc){
                api.del.info.pc_inf.ptcode = global_msc_parameter.point_code;
            }
            else if(ASP_PORCESS_SGSN == proc){
                api.del.info.pc_inf.ptcode = global_sgsn_parameter.point_code;
            }
            ret = m3ua_route(oprn, &api);
            api.del.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
    }
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_user(m3_u8    oprn, m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32            ret;
    m3ua_user_t       api;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sp_id = proc;
            api.add.info.user.mtp_user.sio = 3;
            api.add.info.user.mtp_user.as_id = proc;
            ret = m3ua_user(0 + proc*2, oprn, &api);
            if (-1 == ret) {
                iu_log_debug("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }

            api.add.info.sp_id = proc;
            api.add.info.user.mtp_user.sio = 5; //? what does sio mean?
            api.add.info.user.mtp_user.as_id = proc;
            ret = m3ua_user(1 + proc*2, oprn, &api);
            if (-1 == ret) {
                iu_log_debug("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
        case M3_DELETE: {
            ret = m3ua_user(0+proc*2, oprn, &api);
            if (-1 == ret) {
                iu_log_debug("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            ret = m3ua_user(1+proc*2, oprn, &api);
            if (-1 == ret) {
                iu_log_debug("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
    }
    return ret;
}

m3_s32 um3_m3ua_txr(m3_u16 user, m3_rt_lbl_t *prtlbl)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32      ret = 0;
    //m3ua_txr_t  api;
#if 0	
    m3_u8       prot_data[] = {   0x01,0x01,0x61,0xda,0x02,0x02,0x06,0x04,0x43,0x41,
	0x1f,0x8e,0x0f,0x47,0x00,0x13,0x40,0x43,0x00,0x00,0x06,0x00,0x03,0x40,0x01,0x00,
	0x00,0x0f,0x40,0x06,0x00,0x64,0xf0,0x90,0x18,0x07,0x00,0x3a,0x40,0x08,0x00,0x64,
	0xf0,0x90,0x18,0x07,0x00,0x01,0x00,0x10,0x40,0x11,0x10,0x05,0x24,0x71,0x03,0x00,
	0x00,0x00,0x08,0x49,0x06,0x90,0x08,0x40,0x08,0x87,0x72,0x00,0x4f,0x40,0x03,0x01,
	0x5d,0xf4,0x00,0x56,0x40,0x05,0x64,0xf0,0x90,0x07,0x0f,0x04,0x04,0x43,0x47,0x1f,
	0x8e,0x00};
	m3_u8 prot_data1[] = {        0x06,0x00,0x00,0x03,0x00,0x01,0x20,0x00,0x14,0x40,
	0x1c,0x00,0x00,0x01,0x00,0x10,0x40,0x15,0x14,0x03,0x45,0x04,0x01,0xa0,0x5e,0x07,
	0x81,0x31,0x04,0x08,0x70,0x19,0xf0,0x40,0x04,0x04,0x02,0x60,0x00};
		/*
			0x09, 0x01, 0x03, 0x0e, 0x19, 0x0b, 0x92, 0x06,
			0x00, 0x12, 0x04, 0x19, 0x99, 0x96, 0x76, 0x39, 
			0x98, 0x0b, 0x12, 0x08, 0x00, 0x12, 0x04, 0x19, 
			0x89, 0x96, 0x92, 0x99, 0x29, 0x5a, 0x62, 0x58, 
			0x48, 0x04, 0x86, 0x12, 0x05, 0x72, 0x6b, 0x1a, 
			0x28, 0x18, 0x06, 0x07, 0x00, 0x11, 0x86, 0x05,
			0x01, 0x01, 0x01, 0xa0, 0x0d, 0x60, 0x0b, 0xa1, 
			0x09, 0x06, 0x07, 0x04, 0x00, 0x00, 0x01, 0x00,
			0x05, 0x03, 0x6c, 0x34, 0xa1, 0x32, 0x02, 0x01, 
			0x80, 0x02, 0x01, 0x16, 0x30, 0x2a, 0x80, 0x07,
			0x91, 0x19, 0x99, 0x96, 0x76, 0x39, 0x98, 0x83, 
			0x01, 0x00, 0x85, 0x01, 0x01, 0x86, 0x07, 0x91,
			0x19, 0x89, 0x96, 0x92, 0x99, 0x29, 0x87, 0x08, 
			0x9b, 0xac, 0x03, 0xb9, 0x08, 0x10, 0xae, 0x7d,
			0xab, 0x06, 0x03, 0x02, 0x06, 0xc0, 0x05, 0x00 };
*/
    api.nw_app = 0x01ABCDEF;
    api.add_rtctx = M3_TRUE;
    api.crn_id = 0x00000000;    /* Correlation Id management is part of LME */
    api.rt_lbl = *prtlbl;
	if (5 == cc_success) {
	    api.prot_data_len = sizeof(prot_data1);
	    api.p_prot_data = prot_data1;
	}
	else {
	    api.prot_data_len = sizeof(prot_data1);
	    api.p_prot_data = prot_data1;
		cc_success++;
	}
	printf("<---- Sending Data Message with DPC:%x, OPC:%x, SI:%d ---->\n",
        prtlbl->dpc, prtlbl->opc, prtlbl->si);
    ret = m3ua_transfer(user, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
#endif
    return ret;

}

m3_s32 um3_m3ua_audit(m3_u8 user)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_s32        ret;
    m3ua_audit_t  api;

    api.num_pc = 3;
    api.pc_list[0] = 0x00010203;
#ifdef SIMULATOR_CN
    api.pc_list[1] = 0x00010304;
#else
    api.pc_list[1] = global_msc_parameter.point_code;/*change later*/
#endif
    api.pc_list[2] = 0x00010305;
    api.nw_app = M3_MAX_U32;
    api.info_len = 0;
    ret = m3ua_audit(user, &api);
    if (-1 == ret) {
        iu_log_debug("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

void um3_timer_expiry(void *pdata)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_rt_lbl_t    rtlbl;		
    if (10 > num_packets_sent) {		
		iu_log_debug("*************** send data ********************\n");
	#ifdef SIMULATOR_CN
        rtlbl.opc = 0x00010101;
        rtlbl.dpc = 0x00010304;
    #else
        rtlbl.opc = home_gateway_msc.point_code; //book modify 2010-10-26
        rtlbl.dpc = global_msc_parameter.point_code; //book modify 2010-10-26
    #endif
        rtlbl.si = 3;
        rtlbl.ni = 2;
        rtlbl.mp = 4;
        rtlbl.sls = 6;
        um3_m3ua_txr(0, &rtlbl);
        num_packets_sent++;
        um3_timer_start(2, (void *)&num_packets_sent);
    }
    else if (20 > num_packets_sent) {
    #ifdef SIMULATOR_CN
        rtlbl.opc = 0x00010101;
        rtlbl.dpc = 0x00010305;
    #else
        rtlbl.opc = home_gateway_msc.point_code; //book modify 2010-10-26
        rtlbl.dpc = global_msc_parameter.point_code; //book modify 2010-10-26
    #endif
        rtlbl.si = 3;
        rtlbl.ni = 2;
        rtlbl.mp = 8;
        rtlbl.sls = 14;
        um3_m3ua_txr(0, &rtlbl);
        num_packets_sent++;
        um3_timer_start(2, (void *)&num_packets_sent);
    }
    else {
        um3_m3ua_asp_state(M3_MODIFY, M3_ASP_DOWN, ASP_PORCESS_MSC);  //maybe error
    }
    return;
}

void um3_process_mgmt_ntfy(m3ua_mgmt_ntfy_t *pntfy)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16         idx; 
    m3_u32  myState = 0;
    #ifdef SIMULATOR_CN
    myState = 0x00010000;
    #else
    myState = 0x00000000;
    #endif

    switch(pntfy->type) {
        case M3_MGMT_NTFY_ASP_STATE: {
                iu_log_debug("\n<---- ASP-STATE Notification ---->\n");
                iu_log_debug("ASP:%x, RSP:%x, AS:%x, STATE:%d\n",
                    pntfy->param.asp.asp_id, pntfy->param.asp.rsp_id,
                    pntfy->param.asp.as_id, pntfy->param.asp.state);
                if (myState == (pntfy->param.asp.rsp_id - pntfy->param.asp.asp_id)) {
                    printf("modify asp state\n");
                    switch(pntfy->param.asp.state) {
                        case M3_ASP_DOWN: {
                            um3_m3ua_asp_state(M3_MODIFY, M3_ASP_INACTIVE, pntfy->param.asp.asp_id);
                            break;
                        }
                        case M3_ASP_INACTIVE: {
                            if (0xFFFFFFFF == pntfy->param.asp.as_id)
                                um3_m3ua_asp_state(M3_MODIFY, M3_ASP_ACTIVE, pntfy->param.asp.asp_id);
                            break;
                        }
                        case M3_ASP_ACTIVE: {
                            um3_m3ua_audit(2*(pntfy->param.asp.asp_id));
                            break;
                        }
                    }
                }
                break;
        }
        case M3_MGMT_NTFY_AS_STATE: {
                iu_log_debug("\n<---- AS-STATE Notification ---->\n");
                iu_log_debug("AS:%x, LSP:%x, RSP:%x, STATE:%d\n",
                    pntfy->param.as.as_id, pntfy->param.as.lsp_id,
                    pntfy->param.as.rsp_id, pntfy->param.as.state);
                break;
        }
        case M3_MGMT_NTFY_R_ASP_STATE: {
                iu_log_debug("\n<---- RASP-STATE Notification ---->\n");
                iu_log_debug("RASP:%x, LSP:%x, AS:%x, STATE:%d\n",
                    pntfy->param.r_asp.asp_id, pntfy->param.r_asp.lsp_id,
                    pntfy->param.r_asp.as_id, pntfy->param.r_asp.state);
                break;
        }
        case M3_MGMT_NTFY_R_AS_STATE: {
                iu_log_debug("\n<---- RAS-STATE Notification ---->\n");
                iu_log_debug("RAS:%x,LSP:%x, STATE:%d\n",
                    pntfy->param.r_as.as_id, pntfy->param.r_as.lsp_id,
                    pntfy->param.r_as.state);
                break;
        }
        case M3_MGMT_NTFY_CONN_STATE: {
                iu_log_debug("\n<---- CONN-STATE Notification ---->\n");
                iu_log_debug("LSP:%x,RSP:%x, STATE:%d\n",
                    pntfy->param.conn.lsp_id, pntfy->param.conn.rsp_id,
                    pntfy->param.conn.state);
                break;
        }
        case M3_MGMT_NTFY_NOTIFY: {
                iu_log_debug("\n<---- NOTIFY Notification ---->\n");
                iu_log_debug("STATUS-TYPE:%d, STATUS-INF:%d, M3ASP-ID:%x\n",
                pntfy->param.notify.status_type,
                pntfy->param.notify.status_inf,
                pntfy->param.notify.m3asp_id);
                iu_log_debug("NUM-AS:%d\n", pntfy->param.notify.num_as);
                for (idx = 0; idx < pntfy->param.notify.num_as; idx++) {
                    iu_log_debug("AS[%d] = %x\n", idx,
                        pntfy->param.notify.as_list[idx]);
                }
                break;
        }
        case M3_MGMT_NTFY_ERR: {
                iu_log_debug("\n<---- ERROR Notification ---->\n");
                iu_log_debug("ERR-CODE:%u\n",pntfy->param.err.err_code);
                iu_log_debug("NUM-RC:%d\n",pntfy->param.err.num_rc);
                for (idx = 0; idx < pntfy->param.err.num_rc; idx++) {
                    iu_log_debug("RC[%d] = %x\n", idx,
                        pntfy->param.err.rc_list[idx]);
                }
                iu_log_debug("NUM-PC:%d\n",pntfy->param.err.num_pc);
                for (idx = 0; idx < pntfy->param.err.num_pc; idx++) {
                    iu_log_debug("PC[%d] = %x\n", idx,
                        pntfy->param.err.pc_list[idx]);
                }
                iu_log_debug("DIAG-LEN:%d\n",pntfy->param.err.diag_len);
                iu_log_debug("DIAG-INFO\n");
                for (idx = 0; idx < pntfy->param.err.diag_len; idx++) {
                    if (0 != idx && idx%16 == 0){
                        //iu_log_debug("\n");
                        iu_log_debug("%2x ", pntfy->param.err.p_diag_inf[idx]);
                    }
                }
                //printf("\n");
                break;
        }
    }
    return;
}

void um3_process_user_ntfy(m3ua_user_ntfy_t *pntfy)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3_u16        idx;
    m3_u32        byte;

    switch(pntfy->type) {
        case M3_USER_NTFY_PAUSE: {
                iu_log_debug("\n<---- PAUSE Notification ---->\n\n");
                iu_log_debug("USER-ID:%d, NW-APP:%d, PTCODE:%d\n",
                    pntfy->param.pause.user_id,
                    pntfy->param.pause.nw_app,
                    pntfy->param.pause.ptcode);
                break;
        }
        case M3_USER_NTFY_RESUME: {
                iu_log_debug("\n<---- RESUME Notification ---->\n");
                iu_log_debug("USER-ID:%d, NW-APP:%d, PTCODE:%d\n",
                    pntfy->param.resume.user_id,
                    pntfy->param.resume.nw_app,
                    pntfy->param.resume.ptcode);
                num_packets_sent = 0;
                um3_timer_start(2, (void *)&num_packets_sent);
                break;
        }
        case M3_USER_NTFY_STATUS: {
                iu_log_debug("\n<---- STATUS Notification ---->\n");
                iu_log_debug("USER-ID:%d, NW-APP:%d, PTCODE:%d\n"
                       "CRND-DPC-%d, CAUSE-%d, INFO-%d\n",
                    pntfy->param.status.user_id,
                    pntfy->param.status.nw_app,
                    pntfy->param.status.ptcode,
                    pntfy->param.status.crnd_dpc,
                    pntfy->param.status.cause,
                    pntfy->param.status.inf);
                break;
        }
        case M3_USER_NTFY_TRANSFER: {
                iu_log_debug("\n<---- TRANSFER Notification ---->\n");
                iu_log_debug("USER-ID:%d, NW-APP:%x, RTCTX:%x, CRN-ID-%u\n",
                    pntfy->param.transfer.user_id,
                    pntfy->param.transfer.inf.nw_app,
                    pntfy->param.transfer.inf.rtctx,
                    pntfy->param.transfer.inf.crn_id);
                iu_log_debug("ROUTE-LABEL -- DPC:%x, OPC:%x, SI:%d, NI:%d,"
                       " MP:%d, SLS:%d\n",
                    pntfy->param.transfer.inf.rt_lbl.dpc,
                    pntfy->param.transfer.inf.rt_lbl.opc,
                    pntfy->param.transfer.inf.rt_lbl.si,
                    pntfy->param.transfer.inf.rt_lbl.ni,
                    pntfy->param.transfer.inf.rt_lbl.mp,
                    pntfy->param.transfer.inf.rt_lbl.sls);
                iu_log_debug("PROTOCOL-DATA-LEN:%d\n",
                    pntfy->param.transfer.inf.prot_data_len);
                iu_log_debug("PROTOCOL-DATA\n");
                for (idx = 0; idx < pntfy->param.transfer.inf.prot_data_len; ) {
                    byte = pntfy->param.transfer.inf.p_prot_data[idx++];
                    //iu_log_debug("%02x ", byte);
                    if (0 == idx%16){
                    //    iu_log_debug("\n");
                    }
                }
                //printf("\n");
                /* Check if data has arrived from AS served by ASP2 */
                if (0x00010202 == pntfy->param.transfer.inf.rt_lbl.opc) {
                    pntfy->param.transfer.inf.rt_lbl.dpc = 0x00010202;
                    pntfy->param.transfer.inf.rt_lbl.opc = 0x00010101;
                    um3_m3ua_txr(0, &pntfy->param.transfer.inf.rt_lbl);
                }
                break;
        }
    }
    return;
}

void um3_m3ua_trace_map()
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    m3ua_set_trace_map(m3uaStartupTrace | m3uaErrorTrace | m3uaInMsgTrace | m3uaOutMsgTrace | m3uaTimerTrace | m3uaAspmTrace | m3uaSsnmTrace);
    m3ua_del_trace(m3uaTimerTrace);
    m3ua_add_trace(m3uaTimerTrace);
}

/* book add, 2011-12-29 */
void IuInitPath(unsigned int vrrid,char *buf){
	int len = strnlen(buf,PATH_LEN);
	#ifndef _DISTRIBUTION_
		sprintf(buf+len,"%d",vrrid);
	#else
		sprintf(buf+len,"%d_%d",local,vrrid);
	#endif
	if(buf == IU_DBUS_OBJPATH){
		len = strnlen(buf,PATH_LEN);
		sprintf(buf+len,"/%s","iu");
	}
	else if(buf == IU_DBUS_INTERFACE){
		len = strnlen(buf,PATH_LEN);
		sprintf(buf+len,".%s","iu");
	}
}


void Iu_DbusPath_Init(){
	IuInitPath(vrrid,IU_DBUS_BUSNAME);
	IuInitPath(vrrid,IU_DBUS_OBJPATH);
	IuInitPath(vrrid,IU_DBUS_INTERFACE);
}



/* initialise iu interface */
void iu_init()
{
    int local_socket = 0;
    sctp_connect_mode_msc = 0;
    msc_as_conf.networkApperance = M3_MAX_U32;
    msc_as_conf.routingContext = 0;
    msc_as_conf.trafficMode = M3_TRFMODE_OVERRIDE;

    sctp_connect_mode_sgsn = 0;
    sgsn_as_conf.networkApperance = M3_MAX_U32;
    sgsn_as_conf.routingContext = 0;
    sgsn_as_conf.trafficMode = M3_TRFMODE_OVERRIDE;

    gNi = 0;
    gCnDomain = 0;

    init_transport();
	ranap_sccp_system();
	/**** Initialise m3ua stack ****/
    um3_m3ua_init();
    /**** setup m3ua trace map ****/
    um3_m3ua_trace_map();
    
	IU_UE = malloc(IU_UE_MAX_NUM*(sizeof(Iu_UE*)));
	
	//local_socket = local_socket_init(); //local socket index   islocal = 1
	local_socket = IuServerTipcInit();
	if(local_socket == -1)
	{
		iu_log_error("Error: init local socket to iuh failed.\n");
    }
	else
	{
		make_assoc_v2(local_socket, 0, 1, 0);
	}
    
	return;
}


int main(int argc, const char * argv[])
{
	if(argc > 2){
		local =  atoi(argv[1]);
		vrrid =  atoi(argv[2]);
	}else if(argc != 1){
		printf("iu argc %d, something wrong!!\n", argc);
		return 0;
	}
	char Board_Slot_Path[] = "/dbm/local_board/slot_id";	
	int slot_id = 0;
	char tmpbuf[128] = {0}; 
	memset(tmpbuf, 0, 128);
	if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&slot_id) == 0){
			slotid = slot_id;
		}	
	}
	iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

	int ret = 0;	

 	 pthread_t dbus_hdlr;
	 pthread_attr_t dbus_hdlr_attr;

  /*  if (3 > argc) {
        iu_log_debug("Usage: %s asp=<IP Address>:<Port> "
               "sgp=<IP Address>:<Port>\n", argv[0]);
        exit(1);
    }
	global_opc = 0x00010101;
	global_dpc = 0x00010304;
*/
	start_sccp_con = 1; //for what?
	
	//um3_server_start(0xc0a80457, 0x1b669, 0xc0a80456, 0x1b66b);
	//um3_server_start(0xc0a80458, 0x2b669, 0xc0a80459, 0x2b66b);

	//iu_dbus_start();  
	iu_log_init();
	Iu_DbusPath_Init();

	/* create iu_dbus thread and set the property */
	//dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_hdlr_attr);
	ret = pthread_create(&dbus_hdlr, &dbus_hdlr_attr, (void *)iu_dbus_thread_main, NULL);
    if (0 != ret) {
	   iu_log_error ("start iu dbus pthread fail\n");
	}
	pthread_join(&dbus_hdlr, NULL);
	/* ---- */
	
	iu_init();
	
	waitfor_message(NULL);

	return 0;

    //return 0;
}


/* book add functions for sigtran, 2011-11-09 */
m3_s32 um3_m3ua_route_sigtran(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_route_t      api;

    switch(oprn) {
        case M3_ADD: {
        /* add routes through configured SG(s) */
            api.add.info.le_id = 0x00010000;
            api.add.info.priority = 1;
            api.add.info.pc_inf.nw_app = 0x01ABCDEF;
            api.add.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);
            api.add.info.pc_inf.ptcode = 0x00010304;
            ret = m3ua_route(oprn, &api);
            api.add.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
        case M3_DELETE: {
            api.del.info.le_id = 0x00010000;
            api.del.info.priority = 1;  
            api.del.info.pc_inf.nw_app = 0x01ABCDEF;
            api.del.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);
            api.del.info.pc_inf.ptcode = 0x00010304;
            ret = m3ua_route(oprn, &api);
            api.del.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
    }
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_user_sigtran(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_user_t       api;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sp_id = 0x00000000;
            api.add.info.user.mtp_user.sio = 3;
            api.add.info.user.mtp_user.as_id = 0x00000000;
            ret = m3ua_user(0, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }

            api.add.info.sp_id = 0x00000000;
            api.add.info.user.mtp_user.sio = 5;
            api.add.info.user.mtp_user.as_id = 0x00000000;
            ret = m3ua_user(1, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
        case M3_DELETE: {
            ret = m3ua_user(0, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            ret = m3ua_user(1, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
    }
    return ret;
}

m3_s32 um3_m3ua_asp_sigtran(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_asp_t        api;
    m3_u32            asp = 0x00000000;
    m3_u16            idx, idx2;
    m3_asp_confname_t confname = M3_ASP_ADD_R_ASP;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.m3asp_id = 0x0000ABCD;
            api.add.info.sctp_ep_id = 0;
            api.add.info.def_nwapp = 0x01ABCDEF;
            api.add.info.as_list[0] = M3_TRUE;
            for (idx = 1; idx < M3_MAX_AS; idx++)
                api.add.info.as_list[idx] = M3_FALSE;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) //OCT2009
                for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) //OCT2009
                    api.add.info.r_as_inf[idx].asp_list[idx2] = M3_FALSE;
            api.add.info.r_as_inf[0].asp_list[0] = M3_TRUE;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_ASP_M3ASP_ID: {
                /* modify ASP ID sent in ASPUP/NTFY message for this ASP */
                    api.modify.info.m3asp_id = 0x0000EF12;
                    break;
                }
                case M3_ASP_NWAPP: {
                /* modify default Network Appearance to 0x12345678 for */
                /* this ASP                                            */
                    api.modify.info.def_nwapp = 0x12345678;
                    break;
                }
                case M3_ASP_ADD_AS: {
                /* Add an AS with id 1 to the ASP */
                    api.modify.as_id = 1;
                    break;
                }
                case M3_ASP_DEL_AS: {
                /* Delete AS with id 3 to the ASP */
                    api.modify.as_id = 3;
                    break;
                }
                case M3_ASP_ADD_R_ASP: {
                /* Add remote asp 2 serving remote as 4 to ASP */
                    api.modify.ras_id = 4;
                    api.modify.rasp_id = 2;
                    break;
                }
                case M3_ASP_DEL_R_ASP: {
                /* Delete remote asp 3 serving remote as 3 from ASP */
                    api.modify.ras_id = 3;
                    api.modify.rasp_id = 3;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_asp(asp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_as_sigtran(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_as_t         api;
    m3_u32            as = 0x00000000;
    m3_as_confname_t  confname = M3_AS_RKEY;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.rtctx = 7;
            api.add.info.rkey.trfmode = M3_TRFMODE_OVERRIDE;
            api.add.info.rkey.nw_app = 0x01ABCDEF;
            api.add.info.rkey.num_rtparam = 1;
            api.add.info.rkey.rtparam[0].dpc = 0x00010101; /* 24 bit pointcode -- ANSI */
            api.add.info.rkey.rtparam[0].num_si = 2;
            api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
            api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
            api.add.info.rkey.rtparam[0].num_opc = 4;
            api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010202;
            api.add.info.rkey.rtparam[0].opc_list[1] = 0x00010203;
            api.add.info.rkey.rtparam[0].opc_list[2] = 0x00010304;
            api.add.info.rkey.rtparam[0].opc_list[3] = 0x00010305;
            api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            break;
        }
        case M3_DELETE: 
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_AS_RTCTX: {
                /* modify routing context to 0x55667788 */
                    api.modify.info.rtctx = 0x55667788;
                    break;
                }
                case M3_AS_RKEY: {
                /* modify the routing key configured for the AS */
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = 0x01ABCDEF;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
                case M3_AS_INFO: {
                    /* modify both routing context and routing key */
                    api.modify.info.rtctx = 0x55667788;
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = 0x01ABCDEF;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_as(as, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}



/* book add to reset resource of msc, 2011-12-27 */
int um3_server_stop(m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    
	starting_iu_service = 1;
	int fid = proc;
	int tid = proc;
	int idx = fid*MAX_REMOTE_EP + tid;
    
    /******** Configure M3UA managed objects ********/
	/* send aspup from asp to remote asp */
	um3_m3ua_asp_state(M3_MODIFY, M3_ASP_INACTIVE, proc);
	
	/* change connection state */
    //um3_m3ua_conn_state(M3_MODIFY, M3_CONN_NOT_ESTB, proc, proc);

    /* Delete connection's */
    um3_m3ua_conn(M3_DELETE, proc, proc, idx);

    /* Delete routes */
   	um3_m3ua_route(M3_DELETE, proc);

   	/* Delete user(s) for asp */
    um3_m3ua_user(M3_DELETE, proc);

    /* Delete asp */
    um3_m3ua_asp(M3_DELETE, proc);

    /* Delete as */
    um3_m3ua_as(M3_DELETE, proc);

    /* Delete remote as */
    /* Delete information into m3_r_as_table by PROCESS_ID, need add sgsn */
    um3_m3ua_r_as(M3_DELETE, proc);

    /* Delete remote asp */   
	/* delete information into m3_r_asp_table by PROCESS_ID, need add sgsn */
	um3_m3ua_r_asp(M3_DELETE, proc);

	/* Delete nwapp */
    um3_m3ua_nwapp(M3_DELETE, proc);

    /**** release the Transport Layer ****/  
    clear_assoc(proc);
	
	starting_iu_service = 0;
    // waitfor_message(NULL);
    
    return 0;
}




/* --------------book add for getting iu if status---------------- */
/* get m3ua asp state, 2012-1-4 */
int um3_get_m3ua_asp_state(m3_u32 proc, m3_u8 *asp_state)
{
    int ret = 0;
    m3ua_asp_state_t  api;
    api.get.info.as_id = proc;
    ret = m3ua_asp_state(proc, proc, M3_GET, &api);
    if(-1 == ret)
    {
        iu_log_error("Error: get m3ua asp state failed!\n");
    }
    else
    {
        *asp_state = api.get.state;
        iu_log_debug("m3ua asp state = %d\n",*asp_state);
    }

    return 0;
}


/* get m3ua conn state, 2012-1-4 */
int um3_get_m3ua_conn_state(m3_u32 proc, m3_u8 *conn_state)
{
    int ret = 0;
    m3ua_conn_state_t  api;
    ret = m3ua_conn_state(proc, proc, M3_GET, &api);
    if(-1 == ret)
    {
        iu_log_error("Error: get m3ua conn state failed!\n");
    }
    else
    {
        *conn_state = api.get.state;
        iu_log_debug("m3ua conn state = %d\n",*conn_state);
    }
    
    return 0;
}


/* book add for getting iu interface status, 2012-1-4 */
int um3_get_iu_interface_status(m3_u32 proc, m3_u8 *m3_asp_state, m3_u8 *m3_conn_state, int *sctp_state)
{
    um3_get_m3ua_asp_state(proc, m3_asp_state);
    um3_get_m3ua_conn_state(proc, m3_conn_state);
    get_sctp_status(proc, sctp_state);
    
    return 0;
}

/* -------------------book add end----------------------- */
/*xiaodw add for Iu get slot id*/
int read_file_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}
int parse_int_ID(char* str,unsigned int* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n')){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}

