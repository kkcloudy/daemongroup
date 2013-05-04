/****************************************************************************
** Description:
** Code for UDP based transport layer
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

#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/ioctls.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sctpusr.h>
#include <confasp1.h>
#include <sccp.h>
#include <sys/un.h>
#include "iu_log.h"

#define SOCKPATH_IU "/var/run/femto/iu"
#define UNIX_PATH_MAX    108
#define CONN_TIME_OUT    20

assoc_ctx_t	assoc_table[MAX_ASSOC];
fd_set		readfds;
int gMaxSock;
pthread_t	recv_thr;
//extern int msc_enable;
//extern int sgsn_enable;
extern int starting_iu_service;
extern unsigned int sctp_connect_mode_msc;
extern unsigned int sctp_connect_mode_sgsn;


int clear_assoc(m3_u32 proc)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int fid = proc;
    int tid = proc;
    int idx = fid*MAX_REMOTE_EP + tid;
    int sockid = from[fid].sockid;
    iu_log_debug("fid = %d, tid = %d, idx = %d",fid,tid,idx);
    iu_log_debug("sockid = %d",sockid);
    FD_CLR(sockid, &readfds);
	
    assoc_table[idx].e_state = 0;
    assoc_table[idx].from = 0;
    assoc_table[idx].to   = 0;
	assoc_table[idx].isClient = 0;

    iu_log_debug("sockid = %d",sockid);
	close(sockid);
	
    memset((void *)&from[fid], 0, sizeof(ep_addr_t));
    memset((void *)&to[tid], 0, sizeof(ep_addr_t));
    
    return 0;
}


/* book add to get sctp status, 2012-1-4 */
int get_sctp_status(m3_u32 proc, int *stat){

    int socket_id = from[proc].sockid;
    struct sctp_status status;
    int len = sizeof(status);
    status.sstat_assoc_id = 1+proc;
    
    iu_log_debug("socket_id = %d\n",socket_id);
    if(-1 == getsockopt(socket_id, IPPROTO_SCTP, SCTP_STATUS, &status, &len))
    {
        iu_log_error("Error: get sctp status failed.");
        return -1;
    }
    else
    {
        *stat = status.sstat_state;
        iu_log_debug("sctp conn status = %d\n",*stat);
    }
    
    return 0;
}


/* book add for multi-homing and normal sctp connection, 2012-12-29 */
int	make_assoc_v2(int fid, int	tid, unsigned char isServ, unsigned char multi_switch)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    
    int idx = fid*MAX_REMOTE_EP + tid;
    int sock_fd = from[fid].sockid;
	int nRet = 0;
	struct sockaddr_in *paddr = (struct sockaddr_in*)&to[tid].addr[0];
	struct sockaddr_in *saddr = (struct sockaddr_in*)&to[tid].addr[1];
	struct sockaddr_in *addrs = NULL;
	int addr_count = multi_switch ? 2 : 1;
	int addr_size = sizeof(struct sockaddr_in);
	
	struct timeval tm;
	fd_set set;
	unsigned long ul = 1;
	ioctl(sock_fd, FIONBIO, &ul); // set to unblock mode
	int result = 0;
	int ret = 0;
	int error = -1;
	int len = sizeof(int);
	
	addrs = (struct sockaddr_in*)malloc(addr_size * addr_count);
	if(addrs == NULL)
	{
	    perror("malloc");
	    return -1;
	}
    iu_log_debug("make_assoc sockid = %d, idx %d, tid %d, fid %d\n", sock_fd, idx, tid, fid);
    /* copy primary_addr to addrs */
    memcpy(addrs+0, paddr, addr_size);
    iu_log_debug("make_assoc primary_ip = %x ,primary_port = %x\n", to[tid].addr[0].sin_addr.s_addr, to[tid].addr[0].sin_port);
    /* copy secondary_addr to addrs */
	if(multi_switch)
    {
        memcpy(addrs+1, saddr, addr_size);
        iu_log_debug("make_assoc secondary_ip = %x ,secondary_port = %x\n", to[tid].addr[1].sin_addr.s_addr, to[tid].addr[1].sin_port);
    }
    
	if(!isServ)
	{
		nRet = sctp_connectx(sock_fd, (struct sockaddr*)addrs, addr_count);
		iu_log_debug("nRet = %d\n",nRet);
	}
	if(nRet == -1)
	{	
		iu_log_debug("reset the connect block time\n");
		tm.tv_sec = CONN_TIME_OUT;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sock_fd, &set);
		if( select(sock_fd+1, &set, NULL, NULL, &tm) > 0)
		{
			iu_log_debug("select result > 0\n");
			getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error == 0) 
			{
				ret = 1;
			}
			else
			{
			    iu_log_debug("use secondary addr successful.");
			    to[tid].issecondary = 1;
				ret = 1;
			}
		}
		else 
		{
			ret = 0;// true of false?
        }
		iu_log_debug("ret = %d\n",ret);
		ul = 0;
		ioctl(sock_fd, FIONBIO, &ul); //set to block mode
		if(!ret)
		{
			close(sock_fd);
			free(addrs);
			return -1;
		}
		else
		{
			iu_log_debug ("\n ### Connected to [%s]\n", (0==to[tid].issecondary)?inet_ntoa(paddr->sin_addr):inet_ntoa(saddr->sin_addr));
	        if (0 == assoc_table[idx].e_state) 
	        {
	            assoc_table[idx].e_state = 1;
	            assoc_table[idx].from = fid;
	            assoc_table[idx].to   = tid;
				assoc_table[idx].isClient = !isServ;
	            FD_SET(from[fid].sockid, &readfds);
				
				if(from[fid].sockid > gMaxSock)
				{
					gMaxSock = from[fid].sockid;
				}
	            iu_log_debug("idx = %d, assoc_table[%d].from = %d, assoc_table[%d].to = %d\n", idx, idx, assoc_table[idx].from, idx, assoc_table[idx].to);
	            free(addrs);
	            return idx;
	        }
		}
		iu_log_debug("Cannot Connect the server!n");
	}
    else if (nRet < 0) 
	{
        iu_log_debug ("\n Unable to connect to remote server [%s] !! \n\n", inet_ntoa(paddr->sin_addr));
        free(addrs);
        return -1;
    }
    else 
	{
        iu_log_debug ("\nConnected to [%s]\n", inet_ntoa(paddr->sin_addr));
        if (0 == assoc_table[idx].e_state) {
            assoc_table[idx].e_state = 1;
            assoc_table[idx].from = fid;
            assoc_table[idx].to   = tid;
			assoc_table[idx].isClient = !isServ;
            FD_SET(from[fid].sockid, &readfds);
			
			if(from[fid].sockid > gMaxSock)
			{
				gMaxSock = from[fid].sockid;
            }
            iu_log_debug("idx = %d, assoc_table[%d].from = %d, assoc_table[%d].to = %d\n", idx, idx, assoc_table[idx].from, idx, assoc_table[idx].to);
            free(addrs);
            return idx;
        }
    }
    
    free(addrs);
    return -1;
}



/* book add for multi-homing and normal sctp connection, 2012-12-29 */
int	make_lep_v2(udp_addr_t	p_addr, udp_addr_t	s_addr, m3_u32 proc, m3_u8 multi_switch)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if(proc >= MAX_LOCAL_EP)
	{
	    iu_log_error("Error: invalid value of proc\n");
	    return -1;
	}
	
    int idx = proc;
    int enable = 1;
    struct sockaddr_in *paddr = NULL;
    struct sockaddr_in *saddr = NULL;
    struct sockaddr_in *addrs = NULL;
	struct sctp_event_subscribe evt;
	int addr_count = multi_switch ? 2 : 1;
	int addr_size = sizeof(struct sockaddr_in);
	
	addrs = (struct sockaddr_in*)malloc(addr_size*addr_count);
	if(addrs == NULL)
	{
	    perror("malloc");
	    return -1;
	}

    if (0 == from[idx].e_state) 
    {
        paddr = (struct sockaddr_in*)&from[idx].addr[0];
        paddr->sin_family		= AF_INET;
        paddr->sin_port		    = htons(p_addr.port);
        paddr->sin_addr.s_addr	= htonl(p_addr.ipaddr);
        /* copy primary_addr to addrs */
        memcpy(addrs+0, paddr, addr_size);
        from[idx].e_state = 1;
        from[idx].sockid = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        iu_log_debug("socketid = %d, idx = %d, primary_ip = %x, primary_port = %x \n", from[idx].sockid, idx, from[idx].addr[0].sin_addr.s_addr, from[idx].addr[0].sin_port);
        if(multi_switch)
        {
            saddr = (struct sockaddr_in*)&from[idx].addr[1];
            saddr->sin_family		= AF_INET;
            saddr->sin_port		    = htons(s_addr.port);
            saddr->sin_addr.s_addr	= htonl(s_addr.ipaddr);
            /* copy secondary_addr to addrs */
            memcpy(addrs+1, saddr, addr_size);
            iu_log_debug("secondary_ip = %x, secondary_port = %x \n", from[idx].addr[1].sin_addr.s_addr, from[idx].addr[1].sin_port);
        }
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
        if (-1 == sctp_bindx(from[idx].sockid, (struct sockaddr*)addrs,
            addr_count, SCTP_BINDX_ADD_ADDR)) 
        {
            iu_log_debug("ERRNO:%d  sctp lep bindx failed\n", errno);
            perror("sctp E_bind_Fail");
            free(addrs);
            return -1;
        }
		memset(&evt, 0, sizeof(evt));
		/* all event paras set default value to be 1 */
		evt.sctp_data_io_event = 1;
		evt.sctp_association_event = 1;
		evt.sctp_address_event = 1;
		evt.sctp_send_failure_event = 1;
		evt.sctp_peer_error_event = 1;
		evt.sctp_shutdown_event = 1;
		evt.sctp_partial_delivery_event = 1;
		//evt.sctp_adaption_layer_event = 1;

	    if (setsockopt(from[idx].sockid, IPPROTO_SCTP, SCTP_EVENTS, 
					&evt, sizeof(evt)) != 0) 
		{
		    iu_log_debug("ERRNO:%d  sctp lep setsockopt failed\n", errno);
			perror("setevent failed");
			free(addrs);
			return -1;
		}
		
        if (-1 == listen(from[idx].sockid, 2)) 
        {
            iu_log_debug("ERRNO:%d sctp lep listen failed\n", errno);
            perror("E_listen_Fail");
            free(addrs);
            return -1;
        }
    }
    
    free(addrs);
    return idx;
}


/* book add for multi-homing and normal sctp connection, 2012-12-29 */
int	make_rep_v2(udp_addr_t	p_addr, udp_addr_t	s_addr, m3_u32 proc, m3_u8 multi_switch)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);

    if(proc >= MAX_REMOTE_EP)
	{
	    iu_log_error("Error: invalid value of proc\n");
	    return -1;
	}
	
    int idx = proc;
    struct sockaddr_in  *paddr = NULL;
    struct sockaddr_in  *saddr = NULL;
    
    if (0 == to[idx].e_state)
    {
        paddr = (struct sockaddr_in*)&to[idx].addr[0];
        paddr->sin_family		= AF_INET;
        paddr->sin_port		    = htons(p_addr.port);
        paddr->sin_addr.s_addr	= htonl(p_addr.ipaddr);
        to[idx].e_state		= 1;
        iu_log_debug("idx = %x, primary_ip = %x, primary_port = %x \n", idx, p_addr.ipaddr, p_addr.port);
        
        if(multi_switch)
        {
            saddr = (struct sockaddr_in*)&to[idx].addr[1];
            saddr->sin_family		= AF_INET;
            saddr->sin_port		    = htons(s_addr.port);
            saddr->sin_addr.s_addr	= htonl(s_addr.ipaddr);
            iu_log_debug("secondary_ip = %x, secondary_port = %x \n", s_addr.ipaddr, s_addr.port);
        }

        return idx;
    }
    return -1;
}


/***************************************************************************
** Initialise transport data structures.
****************************************************************************/
int	init_transport()
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int			idx;

    gMaxSock = 0;
    FD_ZERO(&readfds);
    for (idx = 0; idx < MAX_LOCAL_EP; idx++)
    {
        memset((void *)&from[idx], 0, sizeof(ep_addr_t));
    }
    for (idx = 0; idx < MAX_REMOTE_EP; idx++)
    {
        memset((void *)&to[idx], 0, sizeof(ep_addr_t));
    }
    for (idx = 0; idx < MAX_ASSOC; idx++)
    {
        assoc_table[idx].e_state = 0;
    }
    /* create thread to receive messages
    pthread_create(&recv_thr, NULL, waitfor_message, NULL); */
    return 0;
}

/***************************************************************************
** Used by M3UA to send messages on the specified assoc/stream combination.
****************************************************************************/
m3_s32	m3ua_sendmsg(m3_u32		assoc_id,
                     m3_u32		stream_id,
                     m3_u32             msglen,
                     m3_u8              *pmsg)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    
    int			ret;
    int			tid = assoc_table[assoc_id].to; //msc 0 sgsn 1
    int			fid = assoc_table[assoc_id].from;//msc 0 , sgsn 1
    iu_log_debug("assoc_id = %d, from = %d, to = %d\n",assoc_id, assoc_table[assoc_id].from, assoc_table[assoc_id].to);
	struct sockaddr_in* paddr = NULL;
	if(0 == to[tid].issecondary)
	{
	    paddr = (struct sockaddr_in*)&to[tid].addr[0];
	}
	else
	{
	    paddr = (struct sockaddr_in*)&to[tid].addr[1];
	}
	int			sock_fd = from[fid].sockid;
    iu_log_debug("sock_fd = %d\n", sock_fd);
    unsigned int  ppid = 3;

#ifdef __M3UA_PRINT_OUTGOING_MESSAGE__
    int                 m_idx, byte;
    iu_log_debug("\n<------ Outgoing Message ------>\n");
    iu_log_debug("\n    Number of Bytes in Message = %d\n", msglen);
    for (m_idx = 0; m_idx < msglen; ) {
        byte = pmsg[m_idx++];
        iu_log_debug("%02x ", byte);
        if (0 == m_idx%4) {
            iu_log_debug("\n");
        }
    }
#endif
    iu_log_debug("msglen = %d, stream_id = %d\n",msglen,stream_id);
    ppid = htonl(ppid);
    iu_log_debug("assoc_id = %d, tid = %d, fid = %d, addr = %s, port = %d &&&&&&&&\n", assoc_id, tid, fid, inet_ntoa(paddr->sin_addr), paddr->sin_port);
    ret = sctp_sendmsg (sock_fd, pmsg, msglen, (struct sockaddr *)paddr, sizeof(struct sockaddr), ppid, 0,
					stream_id, 0, 0);

    if (0 >= ret) {
        iu_log_debug("ERRNO:%d\n", errno);
        perror("E_sctp_sendmsg_Fail");
    }
    return 0;
}

int rcv_iuh_msg(int fd, unsigned char  msg[1024], int len)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
	struct sockaddr_un iu_end, iuh_end;
	int ret = 0;
	socklen_t hisend_len;
	
	//unlink(SOCKPATH_IU);
	iu_end.sun_family = AF_UNIX;
	strncpy(iu_end.sun_path, SOCKPATH_IU, UNIX_PATH_MAX);
	
	hisend_len=sizeof(iuh_end);  /* !!!!!!! */
	ret = recvfrom(fd, msg, 1024, 0, NULL, 0);
	iu_log_debug("recvfrom iuh ret is %d\n", ret);
	
	if((0 < ret) && (0 != sccp_send_iuh_msg2sigtran(msg, ret))){
	    iu_log_debug("ERROR: sccp send iuh msg to m3ua error.\n");
	}

	return 0;
}

/***************************************************************************
** Receive messages
****************************************************************************/
void*	waitfor_message(void	*not_used)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int			ret;
    struct timeval	timeout;
    int			idx;
    //int         idx2;
    fd_set		rfds;
    m3ua_mgmt_ntfy_t    ntfy;
    m3ua_user_ntfy_t    u_ntfy;
    unsigned char	msg[1024];
    unsigned int	msglen = 0;

    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;
	FD_ZERO(&rfds);
	
    while(1) {
        fflush(NULL);
		if (starting_iu_service) {
			iu_log_debug("starting iu service now !!!!!!\n");
			sleep(1);
		}
        if (0 == timeout.tv_sec && 0 == timeout.tv_usec) {
            timeout.tv_sec  = 1;
            timeout.tv_usec = 0;
        }

        /* readfds is seted by commands from cli */
        rfds = readfds;		
		
        ret = select(gMaxSock+1, &rfds, NULL, NULL, &timeout);
        if (0 == ret) {
            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;
			if (iu_enable) {		
			    // ignore
            	m3timer_ckexpiry(NULL);
	//			um3_timer_ckexpiry(NULL);
			}
            //um3_timer_ckexpiry(NULL);
        }
        else if (0 < ret) {
            for (idx = 0; idx < MAX_LOCAL_EP; idx++){
                
                if (1 == from[idx].e_state && FD_ISSET(from[idx].sockid,&rfds)){
                    iu_log_debug("\n#### from[%d].e_state = %d ####\n\n",idx,from[idx].e_state);
					if (from[idx].islocal) {
					    iu_log_debug("idx = %d\n",idx);
						rcv_iuh_msg(from[idx].sockid, msg, msglen);
						iu_log_debug("######################################\n");
						iu_log_debug("### rcv msg from iuh ###\n");
					}
					else {
					    iu_log_debug("idx = %d\n",idx);
                    	get_message(idx, from[idx].sockid, msg, &msglen);
                    	iu_log_debug("######################################\n");
                    	iu_log_debug("### rcv msg from cn  ### \n");
					}
				}
			}
        }
        else {
            iu_log_debug("ERRNO:%d\n", errno);
            perror("E_select_Fail");
        }
#if 1   //book change ,2011-09-10
		if (iu_enable) {
	        while (-1 != m3ua_mgmt_ntfy(&ntfy)) {
	            um3_process_mgmt_ntfy(&ntfy);
	        }
			
	        while (-1 != m3ua_user_ntfy(&u_ntfy)) {
	            um3_process_user_ntfy(&u_ntfy);
	        }
	        
			/*
			char testranap[] = {0x00,0x13,0x40,0x43,0x00,0x00,0x06,0x00,0x03,0x40,0x01,0x00,
        		0x00,0x0f,0x40,0x06,0x00,0x64,0xf0,0x90,0x18,0x07,0x00,0x3a,0x40,0x08,0x00,0x64,
        		0xf0,0x90,0x18,0x07,0x00,0x01,0x00,0x10,0x40,0x11,0x10,0x05,0x24,0x71,0x03,0x00,
        		0x00,0x00,0x08,0x49,0x06,0x90,0x08,0x40,0x08,0x87,0x72,0x00,0x4f,0x40,0x03,0x01,
        		0x5d,0xf4,0x00,0x56,0x40,0x05,0x64,0xf0,0x90,0x07,0x0f};
        			if (start_sccp_con) {
        				do_sccp_connection(testranap, sizeof(testranap));
        			}
        			else {
        				//do_sccp_data1(testranap, sizeof(testranap));
        				//test_for_send_ranap_packet();
        	    	}*/
	    	
		}
#endif
		
		/*
		just a test ~@@@@
		if (0 == iu_enable) {
			iu_log_debug("iu is not enable wait for start service !!!\n");
		}*/
    }
    return NULL;
}

/***************************************************************************
** read message from socket and pass it to M3UA
****************************************************************************/
int	get_message(int		fid,
                    int		sockid,
                    unsigned char  msg[1024],
                    unsigned int   *pmsglen)
{
    iu_log_debug("%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int			idx;
    int			tid;
    struct sockaddr_in	addr;	
    unsigned int		len = sizeof(struct sockaddr);
	int			msg_flags;
	struct sctp_sndrcvinfo sri;

	len = sizeof (struct sockaddr_in);
    memset(&addr, 0, sizeof(struct sockaddr));
	*pmsglen = sctp_recvmsg (sockid, msg, 1024,
					(struct sockaddr *)&addr, &len, &sri, &msg_flags);
	if (*pmsglen <= 0) {
		iu_log_debug("E_sctp_recvmsg_Fail \n");
		return 0;
	}
	if (msg_flags & MSG_NOTIFICATION) {
		iu_log_debug("MSG_NOTIFICATION, msg_flags=%X \n", msg_flags);
		return 0;
	}

    if (0 < *pmsglen) {
        /* search for association for which data is received */
        for (idx = 0; idx < MAX_REMOTE_EP; idx++) {
            if (1 == assoc_table[fid*MAX_REMOTE_EP + idx].e_state) {
                tid = assoc_table[fid*MAX_REMOTE_EP + idx].to;
                iu_log_debug("addr.sin_family = %d\n",addr.sin_family);
                iu_log_debug("addr.sin_port = %d\n",addr.sin_port);
                iu_log_debug("addr.sin_addr.s_addr = %d\n",addr.sin_addr.s_addr);
                
                m3ua_recvmsg((fid*MAX_REMOTE_EP + idx), 0, *pmsglen, msg);
            }
        }
    }
    return 0;
}

