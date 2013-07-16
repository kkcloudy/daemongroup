/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
*if_flow_stats.c
*
* MODIFY:
*		by <gujd@autelan.com> on 2013-05-29 revision <1.0>
*
* CREATOR:
*		Gu Jindong
*            System Project Team.
*            mail: gujd@autelan.com
*
* DESCRIPTION:
*		For interface flow statistics.
*
* DATE:
*		29/05/2013	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.0 $	
*******************************************************************************/
#include <zebra.h>
	
#include "prefix.h"
#include "stream.h"
#include "buffer.h"
#include "network.h"
#include "if.h"
#include "log.h"
#include "thread.h"
#include "zclient.h"
#include "memory.h"
#include "table.h"
#include "debug.h"
#include "memtypes.h"
	
#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/tipc.h>
#include <unistd.h>
#include <sys/un.h>
		
#include "zebra/zserv.h"
#include "tipc_zclient_lib.h"
#include "tipc_server.h"
#include "zebra/debug.h"
#include "zebra/if_flow_stats.h"
#include "zebra/tipc_client.h"
#include "zebra/tipc_server.h"

extern struct zebra_t zebrad;
extern product_inf *product ;

process_info *process_se_agent = NULL;
process_info *process_snmp = NULL;
process_info *process_acsample = NULL;

struct timeval current_time;
//struct timeval last_time;




void
rtm_if_flow_packet_create_header (struct stream *s, int process_name, int cmd)
{
  /* length placeholder, caller can update */
  stream_putl(s, IF_FLOW_STATS_HEADER_SIZE);
  stream_putl (s, process_name);
  stream_putl (s, cmd);
}


/*At prsent , only for snmp.(active master)*/
int
rtm_deal_interface_flow_stats_sampling_integrated(int command,process_info* process_information, int process_name)
{
	/*First rtmd send tipc interface flow stats to other board, can use ZEBRA_INTERFACE_PACKETS_REQUEST_ALL.
	Then return. When active master recv ZEBRA_INTERFACE_PACKETS_REQUEST_ALL, do add between linux system data
	and se_agent data, and send to by unix.*/
	/*Here only do send tipc info to other boards.*/
	if( product->board_type != BOARD_IS_ACTIVE_MASTER)
	{
		zlog_warn("%s: err command[%d] parement.\n",__func__,command);
		return -1;
		}
	
	if(rtm_debug_if_flow)
	{
	 quagga_gettimeofday_only(&current_time);
	 zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	if(process_name == PROCESS_NAME_SNMP||process_name == PROCESS_NAME_ACSAMPLE)
	{
		master_board_request_interface_packets_statistics(command);
	}
	else/* add active master send to his board.*/
	{
		zlog_warn("Rtm recv unkown process message!\n");
		return -1;
	 }
	
	if(rtm_debug_if_flow)
	{
	  quagga_gettimeofday_only(&current_time);
	  zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	/*send to active master local board se_agent*/
	rtm_send_message_by_tipc(process_se_agent,command);
	
	if(rtm_debug_if_flow)
	{
	  quagga_gettimeofday_only(&current_time);
	  zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	return 0;
		
}

/*For snmp or acsample, can use process_type. */
int
rtm_deal_interface_flow_stats_sampling_divided(int command, process_info* process_information, uint16_t process_name)
{
	/* First send to se_agent. When recv se_agent , fetch the linux system data, then ADD*/
	/*Here do send se_agent.*/
	if(process_name == PROCESS_NAME_SNMP||process_name == PROCESS_NAME_ACSAMPLE)
	{
		/* Direct send to se_agent, by snmp command[ INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP].
		 by acsample command[ INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE].*/
		rtm_send_message_by_tipc(process_se_agent,command);
	}
	else
	{
		zlog_warn("Rtm recv unkown process message!\n");
		return -1;
	 }
	return 0;
}

/*Rtmd send to snmp or acsample.*/
void
rtm_if_flow_stats_data_packet(process_info *process_information, struct interface *ifp)
{
  struct stream *s;
  
  if(rtm_debug_if_flow)
  {
	  quagga_gettimeofday_only(&current_time);
	  zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
  s = process_information->obuf;
  stream_reset (s);

  /* Message type. */
  rtm_if_flow_packet_create_header(s, process_information->name_type,INTERFACE_FLOW_STATISTICS_DATA);/////////////////
 // stream_putl(s, INTERFACE_FLOW_STATISTICS_DATA);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  
  stream_putq (s, ifp->stats.rx_packets);
  stream_putq (s, ifp->stats.tx_packets);
  stream_putq (s, ifp->stats.rx_bytes);
  stream_putq (s, ifp->stats.tx_bytes);
  stream_putq (s, ifp->stats.rx_errors);
  stream_putq (s, ifp->stats.tx_errors);
  stream_putq (s, ifp->stats.rx_dropped);
  stream_putq (s, ifp->stats.tx_dropped);
  stream_putq (s, ifp->stats.rx_multicast);
  stream_putq (s, ifp->stats.rx_compressed);
  stream_putq (s, ifp->stats.tx_compressed);
  stream_putq (s, ifp->stats.collisions);
  stream_putq (s, ifp->stats.rx_length_errors);
  stream_putq (s, ifp->stats.rx_over_errors);
  stream_putq (s, ifp->stats.rx_crc_errors);
  stream_putq (s, ifp->stats.rx_frame_errors);
  stream_putq (s, ifp->stats.rx_fifo_errors);
  stream_putq (s, ifp->stats.rx_missed_errors);
  stream_putq (s, ifp->stats.tx_aborted_errors);
  stream_putq (s, ifp->stats.tx_carrier_errors);
  stream_putq (s, ifp->stats.tx_fifo_errors);
  stream_putq (s, ifp->stats.tx_heartbeat_errors);
  stream_putq (s, ifp->stats.tx_window_errors);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));
  if(rtm_debug_if_flow)
  	{
		/* Statistics print out using proc file system. */
		zlog_info ("%s:    %llu input packets (%llu multicast), %llu bytes, "
			 "%llu dropped.\n",
			 ifp->name, ifp->stats.rx_packets, ifp->stats.rx_multicast,
			 ifp->stats.rx_bytes, ifp->stats.rx_dropped);
	  
		zlog_info ("   %llu input errors, %llu length, %llu overrun,"
			 " %llu CRC, %llu frame.\n",
			 ifp->stats.rx_errors, ifp->stats.rx_length_errors,
			 ifp->stats.rx_over_errors, ifp->stats.rx_crc_errors,
			 ifp->stats.rx_frame_errors);
	  
		zlog_info ("   %llu fifo, %llu missed.\n", ifp->stats.rx_fifo_errors,
			 ifp->stats.rx_missed_errors);
	  
		zlog_info ("   %llu output packets, %llu bytes, %llu dropped.\n",
			 ifp->stats.tx_packets, ifp->stats.tx_bytes,
			 ifp->stats.tx_dropped);
	  
		zlog_info ("   %llu output errors, %llu aborted, %llu carrier,"
			 " %llu fifo, %llu heartbeat.\n",
			 ifp->stats.tx_errors, ifp->stats.tx_aborted_errors,
			 ifp->stats.tx_carrier_errors, ifp->stats.tx_fifo_errors,
			 ifp->stats.tx_heartbeat_errors);
	  
		zlog_info ("   %llu window, %llu collisions.\n",
			 ifp->stats.tx_window_errors, ifp->stats.collisions);
  	}

  rtm_send_message_by_unix(process_information);
  return;
}


/* Close client fd. */
int
rtm_if_flow_client_unix_close (process_info **_process_information)
{
	process_info *process_information = *_process_information;


  zlog_warn("%s: socket[%d] reading cased failed ,to close unix server.\n",__func__,process_information->sock);

	/* Release threads. */
	if (process_information->t_read)
		thread_cancel (process_information->t_read);
	if (process_information->t_write)
		thread_cancel (process_information->t_write);
	if (process_information->t_suicide)
		thread_cancel (process_information->t_suicide);

	if(process_information->ibuf)
		stream_free(process_information->ibuf);
	
	if(process_information->obuf)
		stream_free(process_information->obuf);
	
	if(process_information->wb)
		buffer_free(process_information->wb);
	if(rtm_debug_if_flow)
		zlog_debug("To close cuurt ACSAMPLE break fd[%d], then to creat a new fd.\n",process_information->sock);
	/* Close file descriptor. */	
	close(process_information->sock);

	XFREE(MTYPE_PROCESS_INFO,process_information);
	*_process_information = NULL;
	return 0;
}

static int
rtm_if_flow_client_unix_flush_data(struct thread *thread)
{
  process_info *process_information = THREAD_ARG(thread);

  process_information->t_write = NULL;
  if (process_information->t_suicide)
    {
      zlog_warn("vice board suicide !\n");
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      rtm_if_flow_client_unix_close(&process_information);
      return -1;
    }
  switch (buffer_flush_available(process_information->wb, process_information->sock))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_flush_available failed on zserv client fd %d, "
      		"closing", __func__, process_information->sock);
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      rtm_if_flow_client_unix_close(&process_information);
      break;
    case BUFFER_PENDING:
      process_information->t_write = thread_add_write(zebrad.master, rtm_if_flow_client_unix_flush_data,
      					 process_information, process_information->sock);
      break;
    case BUFFER_EMPTY:
      break;
    }
  return 0;
}


static int
rtm_if_flow_client_unix_delayed_close(struct thread *thread)
{
  process_info *process_information = THREAD_ARG(thread);

  process_information->t_suicide = NULL;
  rtm_if_flow_client_unix_close(&process_information);
  return 0;
}


int
rtm_send_message_by_unix(process_info *process_information)
{
	if(rtm_debug_if_flow)
	{
	  quagga_gettimeofday_only(&current_time);
	  zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	if (process_information->sock < 0)
	{
		zlog_info("%s : fd < 0 , cannot to send message .\n",__func__);
		return -2;
	}

  if (process_information->t_suicide)
    return -1;
  switch (buffer_write(process_information->wb, process_information->sock, STREAM_DATA(process_information->obuf),
		       stream_get_endp(process_information->obuf)))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_write failed to client fd %d, closing",
      		 __func__, process_information->sock);
      /* Schedule a delayed close since many of the functions that call this
         one do not check the return code.  They do not allow for the
	 possibility that an I/O error may have caused the vice_board to be
	 deleted. */
      process_information->t_suicide = thread_add_event(zebrad.master, rtm_if_flow_client_unix_delayed_close,
					   process_information, 0);
      return -1;
    case BUFFER_EMPTY:
      THREAD_OFF(process_information->t_write);
      break;
    case BUFFER_PENDING:
      THREAD_WRITE_ON(zebrad.master, process_information->t_write,
		      rtm_if_flow_client_unix_flush_data, process_information, process_information->sock);
      break;
    }
  
  if(rtm_debug_if_flow)
  {
    quagga_gettimeofday_only(&current_time);
    zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
   }
  return 0;
}


/*use tipc udp. Rtmd send to se_agent.*/
int
rtm_send_message_by_tipc(process_info *process_information,int command)
{
	se_interative_t se_data;
	/* struct sockaddr_un addr = {0}; */
	struct sockaddr_tipc addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	char se_cmd[64]={0};

	if (process_information->sock< 0) 
	{
		zlog_warn("process se_agent sock(%d) failed!\n", process_information->sock);
		return -1;
	}

	memset(&se_data, 0, sizeof(se_interative_t));
	strncpy(se_data.hand_cmd, SE_AGENT_SHOW_PART_FAU64, sizeof(se_data.hand_cmd)-1);

	/*len = strlen(SE_AGENT_SHOW_PART_FAU64);
	strncpy(se_cmd,SE_AGENT_SHOW_PART_FAU64,len);
	se_cmd[len]='\0';*/

	se_data.fccp_cmd.agent_pid = command;/*integrated or divided*/

	memset(&addr, 0, sizeof(struct sockaddr_tipc));
	addr.family = AF_TIPC;
	addr.addrtype = TIPC_ADDR_NAME;
	addr.addr.name.name.type = SE_AGENT_SERVER_TYPE;
	addr.addr.name.name.instance = SE_AGENT_SERVER_LOWER_INST + product->board_id;
	addr.addr.name.domain = 0;
	addr.scope = TIPC_CLUSTER_SCOPE;
	len = sizeof(addr);

//	nbyte = sendto(process_information->sock, &se_data, sizeof(se_data),
//				MSG_DONTWAIT, (struct sockaddr*)&addr, len);
//	nbyte = sendto(process_information->sock, &se_cmd, sizeof(se_cmd),
//				MSG_DONTWAIT, (struct sockaddr*)&addr, len);/
	nbyte = sendto(process_information->sock, &se_data, sizeof(se_data),
				MSG_DONTWAIT, (struct sockaddr*)&addr, len);


	if (nbyte < 0) {
		zlog_warn("Rtmd send message[%d bytes] to se_agent by fd[%d] failed(%s).\n",
						nbyte,process_information->sock,safe_strerror(errno));
		return -1;
	}
	
	return 0;
}


void
rtm_send_request_if_flow_to_se_agent(int if_flow_command)
{
	if(rtm_debug_if_flow)
	  zlog_debug("%s: line %d, rtm send request to se_agent.\n",__func__,__LINE__);
	rtm_send_message_by_tipc(process_se_agent,if_flow_command);
}

/*
	int64_t fau_enet_output_packets_eth;  
	int64_t fau_enet_output_packets_capwap;  
	int64_t fau_enet_output_packets_rpa;   
	int64_t fau_enet_input_packets_eth;   
	int64_t fau_enet_input_packets_capwap;  
	int64_t fau_enet_input_packets_rpa;   
	int64_t fau_enet_output_bytes_eth;  
	int64_t fau_enet_output_bytes_capwap;  
	int64_t fau_enet_output_bytes_rpa;   
	int64_t fau_enet_input_bytes_eth;   
	int64_t fau_enet_input_bytes_capwap;   
	int64_t fau_enet_input_bytes_rpa;  */

/*CID 18163 (#1 of 1): Big parameter passed by value (PASS_BY_VALUE)
pass_by_value: Passing parameter se_data of type se_interative_t (size 624 bytes) by value
So change the passing parameter from struct 'se_interative_t' to struct 'fau64_part_info_t' .*/
/*
if_flow_stats_se_agent *
rtm_read_if_flow_stats_from_se_agent(struct stream *s,se_interative_t se_data,if_flow_stats_se_agent *if_flow_data)
*/
if_flow_stats_se_agent *
rtm_read_if_flow_stats_from_se_agent(struct stream *s, fau64_part_info_t fau64_part_info,if_flow_stats_se_agent *if_flow_data)

{
#if 0
	/*fetch data.*/
	if_flow_data->output_packets_eth = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_packets_eth;
	if_flow_data->output_packets_capwap= se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_packets_capwap;
	if_flow_data->output_packets_rpa= se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_packets_rpa;
	if_flow_data->input_packets_eth = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_packets_eth;
	if_flow_data->input_packets_capwap= se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_packets_capwap;
	if_flow_data->input_packets_rpa = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_packets_rpa;
	if_flow_data->output_bytes_eth = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_bytes_eth;
	if_flow_data->output_bytes_capwap= se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_bytes_capwap;
	if_flow_data->output_bytes_rpa = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_output_bytes_rpa;
	if_flow_data->input_bytes_eth = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_bytes_eth;
	if_flow_data->input_bytes_capwap = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_bytes_capwap;
	if_flow_data->input_bytes_rpa = se_data.fccp_cmd.fccp_data.fau64_part_info.fau_enet_input_bytes_rpa;
#else
	/*fetch data.*/
	if_flow_data->output_packets_eth = fau64_part_info.fau_enet_output_packets_eth;
	if_flow_data->output_packets_capwap= fau64_part_info.fau_enet_output_packets_capwap;
	if_flow_data->output_packets_rpa= fau64_part_info.fau_enet_output_packets_rpa;
	if_flow_data->input_packets_eth = fau64_part_info.fau_enet_input_packets_eth;
	if_flow_data->input_packets_capwap= fau64_part_info.fau_enet_input_packets_capwap;
	if_flow_data->input_packets_rpa = fau64_part_info.fau_enet_input_packets_rpa;
	if_flow_data->output_bytes_eth = fau64_part_info.fau_enet_output_bytes_eth;
	if_flow_data->output_bytes_capwap= fau64_part_info.fau_enet_output_bytes_capwap;
	if_flow_data->output_bytes_rpa = fau64_part_info.fau_enet_output_bytes_rpa;
	if_flow_data->input_bytes_eth = fau64_part_info.fau_enet_input_bytes_eth;
	if_flow_data->input_bytes_capwap = fau64_part_info.fau_enet_input_bytes_capwap;
	if_flow_data->input_bytes_rpa = fau64_part_info.fau_enet_input_bytes_rpa;

#endif

	return if_flow_data;
	
}

/*read the flow stats from linux system proc*/
void
rtm_get_if_flow_stats_from_linx_system(void)
{
	  
#ifdef HAVE_PROC_NET_DEV
	  ifstat_update_proc ();
#endif /* HAVE_PROC_NET_DEV */

#ifdef HAVE_NET_RT_IFLIST
	  ifstat_update_sysctl ();
#endif /* HAVE_NET_RT_IFLIST */
		  
}

void
rtm_if_flow_stats_update_for_sampling_integrated(struct interface *ifp,int if_flow_command)

{
	
	struct listnode *node, *nnode;
	struct listnode *ifnode, *ifnnode;
	//struct interface *ifp;

#if 0	
	if(!iflist)
	{
		zlog_warn("At preset iflist is NULL .\n");
		return ;
	}
#else
	if(rtm_debug_if_flow)
		zlog_debug("%s: line %d,interface name [%s].\n",__func__,__LINE__,ifp->name);

#endif
 /* for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))*/
	{
		if((memcmp(ifp->name,"radio",5) == 0)			
			||(memcmp(ifp->name,"r",1) == 0)
			||(memcmp(ifp->name,"sit0",4) == 0)
			||(memcmp(ifp->name,"obc0",4) == 0)
			||(memcmp(ifp->name,"obc1",4) == 0)
			||(memcmp(ifp->name,"pimreg",6) == 0)
			||(memcmp(ifp->name,"ppp",3) == 0)
			||(memcmp(ifp->name,"oct",3) == 0)
			/*||(memcmp(ifp->name,"lo",2) == 0)*/
			/*||(judge_ve_interface(ifp->name)==VE_INTERFACE)*/
			/*||(judge_eth_debug_interface(ifp->name)==ETH_DEBUG_INTERFACE)*/
			)
	/*		continue;*/
			return;
		
		if(rtm_debug_if_flow)
			zlog_debug("%s: line %d, command[%d](%s).\n",__func__,__LINE__,if_flow_command,
					zserv_command_string(if_flow_command));
		
		if(if_flow_command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE)
		{
			rtm_if_flow_stats_data_packet(process_acsample,ifp);
		}
		else if(if_flow_command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP)
		{
		  /*Only send to snmp for integrated..*/
		  rtm_if_flow_stats_data_packet(process_snmp,ifp);
		}else if(if_flow_command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM)
		{
			/*for vtysh show interface.*/
			zlog_debug("When recv from other board,show interface, now do nothing.\n");
		}
		else
		{
			zlog_warn("%s: line %d, WARN unkown command[%d].\n",__func__,__LINE__,if_flow_command);
		}
		if(rtm_debug_if_flow)
		{
		  quagga_gettimeofday_only(&current_time);
		  zlog_debug("%s:line %d, Time[%u:%u] ----over---.\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
		}
	 }
	return;
	  
}

void
rtm_if_flow_stats_data_add(struct interface *ifp,if_flow_stats_se_agent *if_flow_data )
{
	if(rtm_debug_if_flow)
		zlog_debug("%s:Begin name(%s),rx_bytes[%llu],rx_packets[%llu],tx_bytes[%llu],tx_packets[%llu].\n",
		 __func__,ifp->name,ifp->stats.rx_bytes,ifp->stats.rx_packets,ifp->stats.tx_bytes,ifp->stats.tx_packets);

	ifp->stats.rx_bytes += if_flow_data->input_bytes_eth+if_flow_data->input_bytes_capwap;
	ifp->stats.rx_packets += if_flow_data->input_packets_eth+if_flow_data->input_packets_capwap;

	ifp->stats.tx_bytes += if_flow_data->output_bytes_eth+if_flow_data->output_bytes_capwap;
	ifp->stats.tx_packets += if_flow_data->output_packets_eth+if_flow_data->output_packets_capwap;

	if(rtm_debug_if_flow)
		zlog_debug("%s:Finish name(%s),rx_bytes[%llu],rx_packets[%llu],tx_bytes[%llu],tx_packets[%llu].\n",
		 __func__,ifp->name,ifp->stats.rx_bytes,ifp->stats.rx_packets,ifp->stats.tx_bytes,ifp->stats.tx_packets);
}


/*read the flow stats from linux system proc*/
void
rtm_if_flow_stats_data_deal_for_active_master_board(if_flow_stats_se_agent *if_flow_data,int command)
{
	
	struct listnode *node, *nnode;
	struct listnode *ifnode, *ifnnode;
	struct interface *ifp;
	tipc_server * vice_board = NULL;
	  
	if(!iflist)
	{
		zlog_warn("At preset iflist is NULL .\n");
		return ;
	}
    for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
	{
		if((memcmp(ifp->name,"r",1) == 0)/*include radio*/
			||(memcmp(ifp->name,"sit0",4) == 0)
			/*||(memcmp(ifp->name,"obc0",4) == 0)*/
			||(memcmp(ifp->name,"obc1",4) == 0)
			||(memcmp(ifp->name,"pimreg",6) == 0)
			||(memcmp(ifp->name,"ppp",3) == 0)
			||(memcmp(ifp->name,"oct",3) == 0)
			/*||(memcmp(ifp->name,"lo",2) == 0)*/
			/*||(judge_ve_interface(ifp->name)==VE_INTERFACE)*/
			||(judge_eth_debug_interface(ifp->name)==ETH_DEBUG_INTERFACE)
			/*||(memcmp(ifp->name,"mng",3) == 0)*/
			)
			continue;
		if(judge_real_local_interface(ifp->name) == LOCAL_BOARD_INTERFACE)/*check uplink interface do add, other only linux system*/
		{
		   	if(CHECK_FLAG(ifp->uplink_flag,INTERFACE_SET_UPLINK))
		    {
		    	/*do add se_agent if flow stats.*/
				rtm_if_flow_stats_data_add(ifp,if_flow_data);
		      }
			/* command ==INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM*/
			
			if(rtm_debug_if_flow)
				zlog_debug("%s: line %d, command[%d](%s).\n",__func__,__LINE__,command,
						zserv_command_string(command));
			if(command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE
				||command == INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_ACSAMPLE)/*acsample*/
			{
				/*update local and send to acsample.*/
				rtm_if_flow_stats_data_packet(process_acsample,ifp);
				
			}
			else if(command ==INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP
				||command == INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_SNMP)/*snmp*/
			{
				/*update local and send to snmp .*/
				rtm_if_flow_stats_data_packet(process_snmp,ifp);
			}
			else if(command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM)
			{
				/*for vtysh show interface. do nothing.*/
				if(rtm_debug_if_flow)
				 zlog_debug("When recv se_agent,show interface, now do nothing.\n");
			}
			else
			{
				zlog_warn("%s: line %d, WARN unkown command[%d].\n",__func__,__LINE__,command);
				return;
			}
		}
		else
			continue;
		
	}
	  
}


/*read the flow stats from linux system proc*/
void
rtm_if_flow_stats_data_deal_for_other_board(if_flow_stats_se_agent *if_flow_data,int command)
{
	
	struct listnode *node, *nnode;
	struct listnode *ifnode, *ifnnode;
	struct interface *ifp;
	tipc_server * vice_board = NULL;
	  
	if(!iflist)
	{
		zlog_warn("At preset iflist is NULL .\n");
		return ;
	}
	for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
	{
		/*dierent from active master.*/
		if((memcmp(ifp->name,"r",1) == 0)
			||(memcmp(ifp->name,"sit0",4) == 0)
			||(memcmp(ifp->name,"obc0",4) == 0)
			||(memcmp(ifp->name,"obc1",4) == 0)
			||(memcmp(ifp->name,"pimreg",6) == 0)
			||(memcmp(ifp->name,"ppp",3) == 0)
			||(memcmp(ifp->name,"oct",3) == 0)
			||(memcmp(ifp->name,"lo",2) == 0)
			||(judge_ve_interface(ifp->name)==VE_INTERFACE)
			||(judge_eth_debug_interface(ifp->name)==ETH_DEBUG_INTERFACE)
			||(memcmp(ifp->name,"mng",3) == 0))
			continue;
		if(judge_real_local_interface(ifp->name) == LOCAL_BOARD_INTERFACE)/*check uplink interface do add, other only linux system*/
		{
		   	if(CHECK_FLAG(ifp->uplink_flag,INTERFACE_SET_UPLINK))
		    {
		    	/*do add se_agent if flow stats.*/
				rtm_if_flow_stats_data_add(ifp,if_flow_data);
		      }
			
			if(rtm_debug_if_flow)
				zlog_debug("%s: line %d, command[%d](%s).\n",__func__,__LINE__,command,
						zserv_command_string(command));
			
			/*send to activ master rtmd.*/
			if(command == INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP
				|| command ==INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM
				|| command ==INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE)
			{
				
				/*sned to active master board rtmd. Then active master rtmd send to snmp .*/
				if(zebrad.master_board_list)
				 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
				 {
				   vice_send_interface_packets_statistics(vice_board, ifp,command);
				  /* usleep(fetch_rand_time_interval());*//*need , avoid to block master.*/
						
				  }
				
			}
			else if(command == INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_ACSAMPLE)/*to local board acsample*/
			{
				/*Only send to local bard acsample.*/
				rtm_if_flow_stats_data_packet(process_acsample,ifp);
			}else if(command == INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_SNMP)
			{
				/*Only send to local bard snmp .*/
				rtm_if_flow_stats_data_packet(process_snmp,ifp);
			}
			else
			{
				zlog_warn("%s: line %d, WARN unkown command[%d].\n",__func__,__LINE__,command);
				return;
			}
		}
		else
			continue;
		
	}

	return;
	  
}



/*read the flow stats from linux system proc*/
void
rtm_if_flow_stats_data_deal(if_flow_stats_se_agent *if_flow_data,int command)
{
	
	if(product->board_type == BOARD_IS_ACTIVE_MASTER )
	{
		 rtm_if_flow_stats_data_deal_for_active_master_board(if_flow_data, command);
		}
	else if(product->board_type == BOARD_IS_BACKUP_MASTER ||product->board_type == BOARD_IS_VICE)
	{
		rtm_if_flow_stats_data_deal_for_other_board(if_flow_data, command);
		}
	else
	{
		zlog_warn("%s: line %d, WARN product type[%d].\n",__func__,__LINE__,product->board_type);
		}
	
	return;
}

void
rtm_if_flow_stats_update(if_flow_stats_se_agent *if_flow_data, int command)
{
	rtm_get_if_flow_stats_from_linx_system();
	
	/*all board to deal the if flow data*/
	rtm_if_flow_stats_data_deal(if_flow_data,command);
		
}


int
rtm_recv_message_from_se_agent(struct thread *thread)
{

	int ret;
	struct sockaddr_tipc addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;  
	if_flow_stats_se_agent if_flow_data ;
	se_interative_t se_data;
	int command = 0,sock=-1;
	process_info *process_information = NULL;


	if (rtm_debug_if_flow)
	 	zlog_debug ("enter %s ......\n", __func__);

	sock = THREAD_FD (thread);
	process_information = THREAD_ARG(thread);
	if(NULL == process_information)
		return -1;
	/*Add check fd when recvfrom.*/
	if (sock < 0)
	{
	 	zlog_warn("%s: rtmd connect se_agent fd < 0.\n",__func__);
	 	return -1;
	}

	/*master_board= THREAD_ARG (thread);*/
	process_information->t_read = NULL;

	memset(&se_data, 0, sizeof(se_interative_t));
	memset(&addr, 0, sizeof(struct sockaddr_tipc));
	len = sizeof(addr);
	if(rtm_debug_if_flow)
	{
		quagga_gettimeofday_only(&current_time);
		zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	nbyte = recvfrom(sock, &se_data, sizeof(se_interative_t), 0,
				  (struct sockaddr *)&addr, &len);

	if(nbyte < 0)
	{
		zlog_warn("%s: line %d, recvfrom se_agent err(%s)!\n",
					__func__,__LINE__,safe_strerror(errno));
		return -1;
	}

	if(rtm_debug_if_flow)
	{
		quagga_gettimeofday_only(&current_time);
	 	zlog_debug("%s:line %d, Time[%u:%u].\n",__func__,__LINE__,current_time.tv_sec,current_time.tv_usec);
	}
	memset(&if_flow_data,0,sizeof(if_flow_stats_se_agent));/*to fetch data from se_agent.*/

	command = se_data.fccp_cmd.agent_pid;

	/*Read the data from se_agent.*/
	/*rtm_read_if_flow_stats_from_se_agent(process_se_agent->ibuf,se_data,&if_flow_data);*/
	rtm_read_if_flow_stats_from_se_agent(process_information->ibuf, se_data.fccp_cmd.fccp_data.fau64_part_info, &if_flow_data);

	/*After fetch the data ,go to update flow stats data*/
	rtm_if_flow_stats_update(&if_flow_data,command);

	/*CID 18165 (#1 of 1): Argument cannot be negative (REVERSE_NEGATIVE)
	  check_after_sink: You might be using variable "process_se_agent->sock" before verifying that it is >= 0. 
	 So add check fd when recvfrom above.*/
	if (sock < 0)
	{
		/* Connection was closed during packet processing. */
		zlog_warn("%s: After recvfrom se_agent fd is break.\n",__func__);
		return -1;
	}
#if 1
	/* Register read thread. */
	stream_reset(process_information->ibuf);
	rtm_if_flow_event(IF_FLOW_TIPC_CLIENT_READ,process_information->sock,process_information);
#endif

	if(rtm_debug_if_flow)
		zlog_debug("leave func %s...\n",__func__);

	return 0;

}



int
rtm_recv_message_from_snmp(struct thread * thread)
{
	int sock = -1;
	process_info *process_information = NULL;
	size_t already;
	int length, process_name;
	int command;

	/* Get thread data.  Reset reading thread because I'm running. */
	sock = THREAD_FD (thread);
	process_information = THREAD_ARG (thread);
	if(NULL == process_information)
		return -1;
	process_information->t_read = NULL;


	/* Read length and command (if we don't have it already). */
	if ((already = stream_get_endp(process_information->ibuf)) < IF_FLOW_STATS_HEADER_SIZE)
	{
		/*If it is normal , the already is 0.*/
		ssize_t nbyte;
		/*to fetch header*/
		if (((nbyte = stream_read_try (process_information->ibuf, sock,
					 IF_FLOW_STATS_HEADER_SIZE-already)) == 0) ||(nbyte == -1))
		{
			if (rtm_debug_if_flow)
				zlog_debug ("connection closed socket [%d]", sock);
			rtm_if_flow_client_unix_close(&process_information);
			return -1;
		}
		if (nbyte != (ssize_t)(IF_FLOW_STATS_HEADER_SIZE-already))
		{
			/* Try again later. */
			rtm_if_flow_event(IF_FLOW_UNIX_SERVER_READ_SNMP, sock, process_information);
			return 0;
		}
		already = IF_FLOW_STATS_HEADER_SIZE;
	}

	/* Reset to read from the beginning of the incoming packet. */
	stream_set_getp(process_information->ibuf, 0);

	/* Fetch header values */
	length = stream_getl (process_information->ibuf);
	process_name = stream_getl (process_information->ibuf);
	command = stream_getl (process_information->ibuf);
	if(rtm_debug_if_flow)
		zlog_warn("%s: line %d, length[%d],name[%d],command[%d]..\n",__func__,__LINE__,length,process_name,command);

	if (process_name!= PROCESS_NAME_SNMP && process_name!= PROCESS_NAME_ACSAMPLE)
	{
		zlog_warn("%s: socket %d version mismatch, process_type %d .", __func__, sock, process_name);
		rtm_if_flow_client_unix_close(&process_information);
	 	return -1;
	}
	
	process_information->name_type = process_name ;
	
	if (length > STREAM_SIZE(process_information->ibuf))
	{
	  zlog_warn("%s: socket %d message length %u exceeds buffer size %lu",
			__func__, sock, length, (u_long)STREAM_SIZE(process_information->ibuf));
	 rtm_if_flow_client_unix_close(&process_information);
	  return -1;
	}

	/* Read rest of data. */
	if (already < length)
	{
		ssize_t nbyte;
		if (((nbyte = stream_read_try (process_information->ibuf, sock,
					 length-already)) == 0) ||(nbyte == -1))
		{
			if (rtm_debug_if_flow)
				zlog_debug ("connection closed [%d] when reading rtm data", sock);
		  	rtm_if_flow_client_unix_close(&process_information);
		  	return -1;
		}
	  if (nbyte != (ssize_t)(length-already))
		{
		  /* Try again later. */
		  	rtm_if_flow_event(IF_FLOW_UNIX_SERVER_READ_SNMP, process_information->sock, process_information);
		  return 0;
		}
	}

  length -= IF_FLOW_STATS_HEADER_SIZE;

  /* Debug packet information. */
  if (rtm_debug_if_flow)
	zlog_debug ("Rtm message comes from socket [%d]", sock);

	if (IS_ZEBRA_DEBUG_PACKET && IS_ZEBRA_DEBUG_RECV)
		zlog_debug ("Rtm message received [%s] %d", zserv_command_string (command), length);

	//rtm_get_if_flow_stats_from_linx_system();

	switch (command) 
	{
		case INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_SNMP:
			process_snmp = process_information;
			rtm_deal_interface_flow_stats_sampling_integrated(command, process_information,process_name);
			break;			
				
		case INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE:
			process_acsample = process_information;
			rtm_deal_interface_flow_stats_sampling_integrated(command, process_information,process_name);
			break;
			
		case INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_ACSAMPLE:
			process_acsample = process_information;
			rtm_deal_interface_flow_stats_sampling_divided(command, process_information,process_name);
			break;
		case INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_SNMP:
			process_snmp = process_information;
			rtm_deal_interface_flow_stats_sampling_divided(command, process_information,process_name);
		  	break;

		default:
		  zlog_info ("Rtm received unknown command %d", command);
		  break;
	}

	if (process_information->t_suicide)
	{
	  /* No need to wait for thread callback, just kill immediately. */
	  	zlog_debug("%s: suicide.\n",__func__);
	  	rtm_if_flow_client_unix_close(&process_information);
	  	return -1;
	}
	
	stream_reset (process_information->ibuf);
	rtm_if_flow_event(IF_FLOW_UNIX_SERVER_READ_SNMP, process_information->sock, process_information);
  return 0;
}

int
rtm_unix_server_accept(struct thread *thread)
{
  int accept_sock;
  int client_sock;
  struct sockaddr_in client;
  socklen_t len;
	process_info *process_information = NULL;

	accept_sock = THREAD_FD (thread);
	if(accept_sock <= 0)
	{
	  zlog_err ("In func %s get THREAD_FD error\n",__func__);
	  return -1;
	}

  len = sizeof (struct sockaddr_in);
  client_sock = accept (accept_sock, (struct sockaddr *) &client, &len);

  if (client_sock < 0)
	{
	  zlog_warn ("Can't accept rtm socket: %s", safe_strerror (errno));
	  rtm_if_flow_event(IF_FLOW_UNIX_SERVER_ACCEPT_SNMP, accept_sock, NULL);
	  return -1;
	}

	/* Make client socket non-blocking.  */
	set_nonblocking(client_sock);

	process_information = XMALLOC(MTYPE_PROCESS_INFO,sizeof(process_info));
	memset (process_information, 0, sizeof (process_info));
	 
	process_information->name_type = -1;

	process_information->ibuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
	process_information->obuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
	process_information->wb = buffer_new(0);

	process_information->fail = 0;
	process_information->t_read = NULL;
	process_information->t_suicide = NULL;
	process_information->t_write = NULL;

	process_information->sock = client_sock;

	rtm_if_flow_event(IF_FLOW_UNIX_SERVER_READ_SNMP,process_information->sock,process_information);
	rtm_if_flow_event(IF_FLOW_UNIX_SERVER_ACCEPT_SNMP, accept_sock, NULL);

  return 0;
}

int rtm_creat_unix_stream_server(const char *path)
{
	int ret = -1;
	int sock = 0, len = 0;
	struct sockaddr_un serv = {0};
	mode_t old_mask;

	/* First of all, unlink existing socket */
	unlink (path);

	/* Set umask */
	old_mask = umask (0077);

	/* Make UNIX domain socket. */
	sock = socket (AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		zlog_warn ("Can't create zserv unix socket: %s", 
			 						safe_strerror (errno));
		return -1;
	}

	/* Make server socket. */
	memset (&serv, 0, sizeof (struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy (serv.sun_path, path, strlen (path));
	
#ifdef HAVE_SUN_LEN
  len = serv.sun_len = SUN_LEN(&serv);
#else
  len = sizeof (serv.sun_family) + strlen (serv.sun_path);
#endif /* HAVE_SUN_LEN */

  ret = bind (sock, (struct sockaddr *) &serv, len);
  if (ret < 0)
	{
		zlog_warn ("Can't bind to unix socket %s: %s", 
							 path, safe_strerror (errno));
		close (sock);
		return ret;
	}

	ret = listen (sock, 32);
	if (ret < 0)
	{
		zlog_warn ("Can't listen to unix socket %s: %s", 
			 					path, safe_strerror (errno));
		close (sock);
		return ret;
	}

	umask (old_mask);

	rtm_if_flow_event(IF_FLOW_UNIX_SERVER_ACCEPT_SNMP, sock, NULL);

	return 0;

}


int
rtm_creat_tipc_client_for_se_agent(struct thread *thread)
{
	process_info *process_information = NULL;

	process_information = THREAD_ARG (thread);
	if(	!process_information )
	{
		zlog_err("creat tipc socket for seagent failed : %s. \n",safe_strerror(errno));
		return -1;
	}
	process_information->sock = socket (AF_TIPC, SOCK_DGRAM,0);/*tipc udp*/

	if (process_information->sock < 0)
	{
	  	zlog_debug ("creat tipc socket for seagent failed : %s. \n",safe_strerror(errno));
	  	process_information->fail++;
	  	if(process_information->fail > TIPC_CONNECT_TIMES)/*from 12 times to 120 times*/
			return -2;
	  	rtm_if_flow_event(IF_FLOW_TIPC_CLIENT_SCHEDULE,0,process_information);
	   	return -1;
	}

	/*Set tipc socket nonblock.*/	
		if (set_nonblocking(process_information->sock) < 0)
		{
	  	zlog_warn("%s: set_nonblocking(%d) failed[%s]", 
			  __func__, process_information->sock,safe_strerror(errno));
	 	return -3;
	}

	/* Clear fail count. */
	process_information->fail = 0;
	if (rtm_debug_if_flow)
		zlog_debug (" Rtm creat tipc client socket[%d] for Se_agent sucess .\n", 
					process_information->sock);
	rtm_if_flow_event(IF_FLOW_TIPC_CLIENT_READ,process_information->sock,process_information);

	return 0;

}
	
void
rtm_if_flow_event (enum if_flow_events event, int sock, process_info *process_information)
{
	if(rtm_debug_if_flow)
	 	zlog_debug("enter func %s ....\n",__func__);
	
    switch (event)
	{

		/*******************/
		case IF_FLOW_TIPC_CLIENT_SCHEDULE:
		  	if(rtm_debug_if_flow)
				zlog_debug("%s: line %d, to IF_FLOW_TIPC_CLIENT_SCHEDULE. \n",__func__,__LINE__);
			if(sock > 0)
			{
				if (process_information->t_read)
				  	thread_cancel (process_information->t_read);
				if (process_information->t_write)
				  	thread_cancel (process_information->t_write);
				if (process_information->t_suicide)
				  	thread_cancel (process_information->t_suicide);
				close(sock);
				process_information->sock = -1;
				
				process_information->name_type = PROCESS_NAME_SE_AGENT;
				if(process_information->ibuf)
					stream_reset (process_information->ibuf);
				if(process_information->ibuf)
					stream_reset (process_information->obuf);
				if(process_information->wb)
					buffer_reset(process_information->wb);
				
				process_information->fail = 0;

			}
			thread_add_event (zebrad.master, rtm_creat_tipc_client_for_se_agent, process_information, sock);

		 	break;
		/*******************/
		case IF_FLOW_TIPC_CLIENT_READ:
		  	if(rtm_debug_if_flow)
		  		zlog_debug ("%s: line %d, set rtmd go to read from se_agent.....\n", __func__,__LINE__);
		   	process_information->t_read = 
		   	thread_add_read (zebrad.master, rtm_recv_message_from_se_agent, process_information, process_information->sock);
		  	break;
		/*******************/
		case IF_FLOW_UNIX_SERVER_ACCEPT_SNMP:

			/*unix server for sampling.*/
			thread_add_read (zebrad.master, rtm_unix_server_accept, NULL, sock);/*accept the client*/

			break;
		/*******************/

		case IF_FLOW_UNIX_SERVER_READ_SNMP:
			/*unix server read*/
			  process_information->t_read = 
			thread_add_read (zebrad.master, rtm_recv_message_from_snmp, process_information, sock);/*server read info from client*/
			break;
			

		default:
			zlog_debug("%s: line %d, unkown type .\n",__func__,__LINE__);
			break;

			
	}
	
	if(rtm_debug_if_flow)
			zlog_debug("leave func %s %d\n",__func__,__LINE__);
}

/*create the tipc socket and communicate with each other. Rtmd-client, Se_agent-server.*/
void
rtm_communication_to_se_agent(void)
{
	/*Init process se_agent info*/
	process_se_agent = XMALLOC(MTYPE_PROCESS_INFO,sizeof(process_info));
	memset (process_se_agent, 0, sizeof (process_info));
		
	process_se_agent->sock = -1;

	process_se_agent->name_type = PROCESS_NAME_SE_AGENT;
	
	process_se_agent->ibuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
	process_se_agent->obuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
	process_se_agent->wb = buffer_new(0);

	process_se_agent->fail = 0;
	process_se_agent->t_read = NULL;
	process_se_agent->t_suicide = NULL;
	process_se_agent->t_write = NULL;

	/*event to set read.*/
	rtm_if_flow_event(IF_FLOW_TIPC_CLIENT_SCHEDULE,process_se_agent->sock,process_se_agent);
	
}

/*gujd: 2013-05-29, pm 2:12. Add for interface flow statistics . Include snmp, acsample and se_agent.
From the se_agent , rtmd get the fast_foward interface flow statistics, then snmp and acsample get statistics from rtmd.*/
void
interface_flow_statistics_init(void)
{
	/*rtmd and se_agent use TIPC socket.*/
	rtm_communication_to_se_agent();
	
	rtm_creat_unix_stream_server(RTM_TO_SNMP_PATH);	
}

