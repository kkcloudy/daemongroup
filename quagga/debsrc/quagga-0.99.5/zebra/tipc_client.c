
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
*tipc_client.c
*
* MODIFY:
*		by <shancx@autelan.com> on 01/01/2011 revision <0.1>
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		ac config information dynamic config
*
* DATE:
*		01/01/2011	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.16 $	
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

#include "zebra/rib.h"
				
#include "tipc_zclient_lib.h"
#include "zebra/zserv.h"
#include "tipc_client.h"
#include "tipc_server.h"
#include "zebra/debug.h"


/**some func api add by gjd : 2011-01--2011-02**/

/**add by gjd, the api func : tipc client let client kernel to add/del route information straight, 
doesn't through zebra, this methord used for debug kernel, and it use netlink packet message to kernel**/



/*static int tipc_client_debug = 1;*/
//static struct zclient *zclient = NULL;
extern struct zebra_t zebrad;
//int vice_board_count = 0;

extern product_inf *product ;
extern int board_type;
extern unsigned int time_interval ;
extern int keep_kernel_mode;
extern struct timeval current_time;

	
/* add by gjd: Redistribute routes, only when system start up */
static void
tipc_server_to_client_redistribute (tipc_client *master_board)	
{
	  struct rib *newrib;
	  struct route_table *table;
	  struct route_node *rn;

	  if(tipc_client_debug)
	  	zlog_debug("enter func %s ... ...when system up\n",__func__);
	
	  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
	  if(tipc_client_debug)
	  {
	  	if(!table)
	  		zlog_debug("route ipv4 (system boot) table == NULL ..\n");
	  	else
	  		zlog_debug("route ipv4 (system boot) table != NULL ..\n");
	  	}
	  if (table)
		for (rn = route_top (table); rn; rn = route_next (rn))
		{
		  for (newrib = rn->info; newrib; newrib = newrib->next)
			  {
			  
	//			if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED) /**when system start up flag = 0,  so don't use it**/
	//				&& newrib->type == type /**if don't send connect route to vice board ,we can add a limt by type, && rib->type != ZEBRA_ROUTE_CONNECT**/
				if(	 newrib->distance != DISTANCE_INFINITY
					&& newrib->type != ZEBRA_ROUTE_CONNECT
					&& zebra_check_addr (&rn->p))/**for lo **/
				{
			  		tipc_server_packet_route_multipath (ZEBRA_IPV4_ROUTE_ADD, master_board, &rn->p, newrib);
				}
			  }
		}
	  
#ifdef HAVE_IPV6
	  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);

	  if(tipc_client_debug)
		{
		  if(!table)
				 zlog_debug("route ipv6 (system boot) table == NULL ..\n");
		  else
		  	zlog_debug("route ipv6 (system boot) table != NULL ..\n");
		}
	  if (table)
		for (rn = route_top (table); rn; rn = route_next (rn))
		{
		
		  for (newrib = rn->info; newrib; newrib = newrib->next)
		  {
		  
	//		if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED)
//			&& newrib->type == type 
		   if( newrib->distance != DISTANCE_INFINITY
			&& newrib->type != ZEBRA_ROUTE_CONNECT
			&& newrib->type != ZEBRA_ROUTE_KERNEL
			&& zebra_check_addr (&rn->p))
			{
			
		  		tipc_server_packet_route_multipath (ZEBRA_IPV6_ROUTE_ADD, master_board, &rn->p, newrib);
			}
		}
	}
	 if(tipc_client_debug)
	  	zlog_debug("leave func %s ... ...when system up\n",__func__);

#endif /* HAVE_IPV6 */
}

void
tipc_redistribute_all (tipc_client *master_board)
{
  	int type;

	if(tipc_client_debug)
		zlog_debug("enter func %s ... ...when system up\n",__func__);	
	
	tipc_server_to_client_redistribute(master_board);
	
	 if(tipc_client_debug)
	  	zlog_debug("leave func %s ... ...when system up\n",__func__);

}     


/**add by gjd: for tipc server redistribute to tipc client to add route**/
void
tipc_redistribute_add (struct prefix *p, struct rib *rib)
{
  struct listnode *node, *nnode;
	tipc_client *client;

  if(tipc_client_debug)
  	zlog_debug("enter func %s .......\n ",__func__);
	

	if(tipc_client_debug)
  		zlog_debug("%s: line %d ,go to redistribute tipc client .......\n ",__func__,__LINE__);

	if(tipc_client_debug)
	 zlog_debug("%s : line %d, rib->status = %u)\n",__func__,__LINE__,rib->status);
#if 0
	if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
 	 UNSET_FLAG (rib->status, RIB_ENTRY_REMOVED);
#endif
	
	  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, client))
	    {
	       if(tipc_client_debug)
	  		 zlog_debug("%s : line %d go to packet info\n ",__func__,__LINE__);
		/*deal with the 0.0.0.0/0 default route as usual*/	        
	       if ((p->family == AF_INET) &&(rib->type != ZEBRA_ROUTE_CONNECT))
	          tipc_server_packet_route_multipath (ZEBRA_IPV4_ROUTE_ADD, client, p, rib);/**ipv4**/
		   
#ifdef HAVE_IPV6
	       if ((p->family == AF_INET6)&&(rib->type != ZEBRA_ROUTE_CONNECT)
		   	&&( rib->type != ZEBRA_ROUTE_KERNEL))
	          tipc_server_packet_route_multipath (ZEBRA_IPV6_ROUTE_ADD, client, p, rib);/**ipv6**/
#endif /* HAVE_IPV6 */	  

	  }
	
}


/**add by gjd: for tipc server redistribute to tipc client to delete route**/
void
tipc_redistribute_delete (struct prefix *p, struct rib *rib)
{
  struct listnode *node, *nnode;
  tipc_client *client;

  if(tipc_client_debug)
  	zlog_debug("enter func %s .......\n ",__func__);

  /* Add DISTANCE_INFINITY check. */
  if (rib->distance == DISTANCE_INFINITY)
    return;
	if(tipc_client_debug)
    zlog_debug("%s : line %d,rib->status=%u\n",__func__,__LINE__,rib->status);

/* SET_FLAG (rib->status, RIB_ENTRY_REMOVED);*/

  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, client))
    {
	  if ((p->family == AF_INET)&&(rib->type != ZEBRA_ROUTE_CONNECT))
	    tipc_server_packet_route_multipath (ZEBRA_IPV4_ROUTE_DELETE, client, p, rib);/**ipv4**/
#ifdef HAVE_IPV6
	  if ((p->family == AF_INET6)&&(rib->type != ZEBRA_ROUTE_CONNECT)
	  	&&( rib->type != ZEBRA_ROUTE_KERNEL))
	    tipc_server_packet_route_multipath (ZEBRA_IPV6_ROUTE_DELETE, client, p, rib);/**ipv6**/
#endif /* HAVE_IPV6 */

    }
  
 /* UNSET_FLAG (rib->status, RIB_ENTRY_REMOVED);*/
}


/**add by gjd:the most important key api. 
	This func is used for tipc server packet his route, and then send to tipc client to synchronize route**/
int
tipc_server_packet_route_multipath (int cmd, tipc_client *master_board, struct prefix *p,
                       struct rib *rib)
	{
	  int psize;
	  struct stream *s;
	  struct nexthop *nexthop = NULL;
	  unsigned long nhnummark = 0, messmark = 0;/*4 byte*/
	  int nhnum = 0;
	  u_char zapi_flags = 0;
	
	  char *ifname = NULL;
	//	int ifnamelen = 0;
	
	  char buf1[BUFSIZ] = {0};
	  char buf2[BUFSIZ] = {0};
	
	  if(tipc_client_debug)
		 zlog_debug("enter func %s .......and cmd 0x%x\n",__func__,cmd);
			  
	  s = master_board->obuf;
	  stream_reset (s);
	  
	  tipc_packet_create_header (s, cmd);/*packet header 6 byte*/
	  
	  /* Put type,flags,ststus and nexthop. */
	  stream_putc (s, rib->type);/*packet rib type 1 byte*/
	  stream_putc (s, rib->flags);/*packet rib flags 1 byte*/
	  stream_putc (s, rib->status);/*packet rib flags 1 byte*/
	  if(tipc_client_debug)
		zlog_debug("%s : line %d, packet rib->type = %d, rib->flags = 0x%x, rib->status = %u \n",
					__func__,__LINE__,rib->type,rib->flags,rib->status);
	  
	  /* marker for message flags field */
	  messmark = stream_get_endp (s);/*packet message maker 1 byte*/
	  stream_putc (s, 0);
	
	  /* Prefix. */
	  stream_putc(s, p->family);
	  psize = PSIZE (p->prefixlen);
	  stream_putc (s, p->prefixlen);/*packet dest prefixlen 1 byte*/

	  if(p->family == AF_INET)
	  	{
			if(tipc_client_debug)
			  zlog_debug("dest is ipv4 .\n");
	  		stream_write (s, (u_char *) & p->u.prefix4, psize);/*packet dest address , size #### u.prefix ########*/
			if(tipc_client_debug)
				zlog_debug("%s : line %d, packet dest ipv4 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET,&p->u.prefix4,buf1,BUFSIZ),p->prefixlen);
	  	}
	  else
	  	if(p->family == AF_INET6)
	  	{
			if(tipc_client_debug)
			  zlog_debug("dest is ipv6 .\n");
	  		stream_write (s, (u_char *) & p->u.prefix6, psize);/*packet dest address , size #### u.prefix ########*/
			if(tipc_client_debug)
			  zlog_debug("%s : line %d, packet dest ipv6 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET6,&p->u.prefix6,buf1,BUFSIZ),p->prefixlen);
	  	}
		else
			{
				zlog_info("%s : line %d , err protocol .\n",__func__,__LINE__);
				return -1;
			}
	  
	  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	  {
		if(tipc_client_debug)
		 zlog_debug("%s : line %d, begin to packet nexthops .......\n",__func__,__LINE__);
		
/*		  if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))*//*delete it , for ve sub interface local and global change role .*/
			{
			
			  SET_FLAG (zapi_flags, ZAPI_MESSAGE_NEXTHOP);/**in fact set flags for messmark**/
			  SET_FLAG (zapi_flags, ZAPI_MESSAGE_IFINDEX);
			  if(tipc_client_debug)
				 zlog_debug("%s: line %d , SET Two flags... ...\n",__func__,__LINE__);
			  
			  if (nhnummark == 0)
				{
				  nhnummark = stream_get_endp (s);/*packet nexthop count 1 byte*/
				  stream_putc (s, 1); /* placeholder */
				  
				}
			  
			  nhnum++;/**in fact for nexthop count **/
	
			  /**packet nexthop->flags,1 byte, include:active,fib,recursive**/
			  stream_putc (s, nexthop->flags); 
			  if(tipc_client_debug)
				zlog_debug("%s : line %d ,packet nexthop->flags = 0x%x.... \n",__func__,__LINE__,nexthop->flags);
			  
			/**the packet form : one type ,one data**/
			  if(tipc_client_debug)
				zlog_debug("%s: line %d, nexthop->type %d ... \n",__func__,__LINE__,nexthop->type);
			  
			  switch(nexthop->type) /** first packet  nexthop type,then packet ip or ifname**/
				{
			   
				  case NEXTHOP_TYPE_IFINDEX:/**for directly connected route when know ifindex**/
						if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IFINDEX..\n",__func__,__LINE__);
						ifname = ifindex_to_ifname(nexthop->ifindex);/**exchange ifindex to ifname**/
						stream_putc(s,nexthop->type);/*packet nexthop type 1 byte*/
		//				ifnamelen = strlen(ifname);
	//					stream_putc(s,ifnamelen);/*packet ifnamelen 1 byte*/
	//					stream_put(s,ifname,ifnamelen);/*packet ifname , size = ifnamelen*/
						stream_put(s,ifname,INTERFACE_NAMSIZ);/*packet ifname 20 byte*/
						if(tipc_client_debug)
						  zlog_debug("%s : line %d,packet ifname %s \n",__func__,__LINE__,ifname);
						break;
						
				  case NEXTHOP_TYPE_IFNAME:/**for directly connected route when know ifname**/
					   if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IFNAME..\n",__func__,__LINE__);
					   stream_putc(s,nexthop->type);
		//			   ifnamelen = strlen(nexthop->ifname);
		//			   stream_putc(s,ifnamelen);
					   stream_put(s,nexthop->ifname,INTERFACE_NAMSIZ);
					   if(tipc_client_debug)
						zlog_debug("%s : line %d,packet ifname %s \n",__func__,__LINE__,nexthop->ifname);
					   break;
				
				  case NEXTHOP_TYPE_IPV4:
				  case NEXTHOP_TYPE_IPV4_IFINDEX:
						if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IPV4 or IPV4_IFINDEX ..\n",__func__,__LINE__);
						stream_putc(s,nexthop->type);/* rib type : 1 byte*/
						stream_put_in_addr (s, &nexthop->gate.ipv4);/*packet ipv4 gate, 4 byte*/
						ifname = ifindex_to_ifname (nexthop->ifindex);
			//			ifnamelen = strlen(ifname);
			//			stream_putc(s,ifnamelen);
						stream_put(s,ifname,INTERFACE_NAMSIZ);
						if(tipc_client_debug)
						  zlog_debug("%s : line %d,packet ipv4 %s ,ifname %s\n",
							__func__,__LINE__,inet_ntop(AF_INET,&nexthop->gate.ipv4,buf2,BUFSIZ),ifname);
						
						break;
	
				  case NEXTHOP_TYPE_IPV4_IFNAME:
						if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IPV4_IFNAME ..\n",__func__,__LINE__);
						stream_putc(s,nexthop->type);/*ipv4 me : add, rib type : 1 byte*/
						stream_put_in_addr (s, &nexthop->gate.ipv4);/*packet ipv4 gate, 4 byte*/
			//			stream_putc(s,strlen(nexthop->ifname));
						stream_put(s,nexthop->ifname,INTERFACE_NAMSIZ);
						if(tipc_client_debug)
							zlog_debug("%s : line %d,packet ipv4 %s ,ifname %s\n",__func__,__LINE__,inet_ntop(AF_INET,&nexthop->gate.ipv4,buf2,BUFSIZ),nexthop->ifname);
	
						break;
					
#ifdef HAVE_IPV6
				  case NEXTHOP_TYPE_IPV6:
				  case NEXTHOP_TYPE_IPV6_IFINDEX:
						if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IPV6 or IFINDEX..\n",__func__,__LINE__);
						ifname = ifindex_to_ifname (nexthop->ifindex);
						stream_putc(s,nexthop->type);/*ipv6*/
						stream_write (s, (u_char *) &nexthop->gate.ipv6, 16);/*packet ipv6 gate, 16 byte*/
			//			stream_putc(s,strlen(ifname));
						stream_put(s,ifname,INTERFACE_NAMSIZ);/*packet ifname*/
						
						break;
					
				  case NEXTHOP_TYPE_IPV6_IFNAME:
						if(tipc_client_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_IPV6_IFNAME ..\n",__func__,__LINE__);
						stream_putc(s,nexthop->type);/*ipv6*/
						stream_write (s, (u_char *) &nexthop->gate.ipv6, 16);/*packet ipv6 gate, 16 byte*/
			//			stream_putc(s,strlen(nexthop->ifname));
						stream_put(s,nexthop->ifname,INTERFACE_NAMSIZ);/*packet ifname*/
						
						break;
#endif
	
	   /**never used nexthop->type = NEXTHOP_TYPE_BLACKHOLE, 
			instead used rib->flags = ZEBRA_FLAG_BLACKHOLE**/
#if 0		
				  case NEXTHOP_TYPE_BLACKHOLE:				
						if(tipc_server_debug)
						  zlog_debug("%s : line %d, nexthop->type = NEXTHOP_TYPE_BLACKHOLE ..\n",__func__,__LINE__);
	
						/*left*/
						break;
#endif
				  default:
					if(tipc_client_debug)
					   zlog_debug("%s : line %d, nexthop->type = default!!!!! ..\n",__func__,__LINE__);
/*					break;*/
					
				#if 1
					if (cmd == ZEBRA_IPV4_ROUTE_ADD 
						|| cmd == ZEBRA_IPV4_ROUTE_DELETE)
					  {
						struct in_addr empty;
						memset (&empty, 0, sizeof (struct in_addr));
						stream_putc(s,nexthop->type);
						stream_write (s, (u_char *) &empty, IPV4_MAX_BYTELEN);
						if(tipc_client_debug)
						 zlog_debug("%s : line %d,packet nexthop->type = %d \n",__func__,__LINE__,nexthop->type);
					  }
					else
					  {
						struct in6_addr empty;
						memset (&empty, 0, sizeof (struct in6_addr));
						stream_putc(s,nexthop->type);
						stream_write (s, (u_char *) &empty, IPV6_MAX_BYTELEN);
						if(tipc_client_debug)
						 zlog_debug("%s : line %d,packet nexthop->type = %d \n",__func__,__LINE__,nexthop->type);
					  }
				#endif
				  }
	
			  
			}
		  if(tipc_client_debug)
			 zlog_debug("%s : line %d, we packet %d nexthops .......\n",__func__,__LINE__,nhnum);
		}
	
	  /* Distance and Metric */
	  if (cmd == ZEBRA_IPV4_ROUTE_ADD || cmd == ZEBRA_IPV6_ROUTE_ADD)/**when detele route, metric and distance need not packet**/
		{
		  SET_FLAG (zapi_flags, ZAPI_MESSAGE_DISTANCE);
		  stream_putc (s, rib->distance);/*packet distance 1 byte*/
		  SET_FLAG (zapi_flags, ZAPI_MESSAGE_METRIC);
		  stream_putl (s, rib->metric);/*packet metric 4 byte*/
		  if(tipc_client_debug)
			zlog_debug("%s : line %d, packet distance=%d,metric=%d\n",__func__,__LINE__,rib->distance,rib->metric);
		}
	  
	  /* write real message flags value */
	  stream_putc_at (s, messmark, zapi_flags);
	  
	  /* Write next-hop number */
	  if (nhnummark)
		stream_putc_at (s, nhnummark, nhnum);
	  if(tipc_client_debug)
		zlog_debug("%s : line %d, make sure we packet %d nexthops .......\n",__func__,__LINE__,nhnum);
	  
	  /* Write packet size. */
	  stream_putw_at (s, 0, stream_get_endp (s));/**write packet length**/
	  if(tipc_client_debug)
		 zlog_debug("%s : line %d, packet over and s->getp=%d,s->endp=%d,s->size=%d.......\n",
						__func__,__LINE__,s->getp,s->endp,s->size);
	
	  return master_send_message_to_vice(master_board);
	}

/***************wc add , process the tipc interface information,****************************/
int
master_interface_send_to_vice(tipc_client  *master_board)
{
	  struct listnode *ifnode, *ifnnode;
	  struct listnode *node, *nnode;
	  struct listnode *cnode, *cnnode;
	  struct interface *ifp;
	  struct connected *c;
	  int ret = 0;

	  if(tipc_client_debug)
	  	zlog_debug("enter func  %s ...\n",__func__);
	
	  /* Interface information is needed. */
	  master_board->ifinfo = 1;
	  if(tipc_client_debug)
	  zlog_debug("%s : line %d..\n",__func__,__LINE__);
#if 0	
	  ret = product->board_id;
	  if (ret != 0) 
		{
		
		zlog_debug("%s : line %d..\n",__func__,__LINE__);
		  return 0;
		}	
#endif
/*  if(product->board_type == BOARD_IS_BACKUP_MASTER ||product->board_type == BOARD_IS_VICE) */
	if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	    return -1;
	  if(zebrad.vice_board_list == NULL)
	  {
	    if(tipc_client_debug)
		 zlog_debug("%s, line = %d :tipc_client_list is NULL....", __func__, __LINE__);
		return -1;
	  }
	
	  for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
	  {
		/*2012-10-17, pm 4:15 .Optimized code .Delete it.*/
		#if 0 
		if(judge_eth_debug_interface(ifp->name) == ETH_DEBUG_INTERFACE)/*skip : the debug eth3 interface .*/
			continue;
/*		if((strncmp(ifp->name,"eth",3) == 0 && strncmp(ifp->name,"eth3",4) != 0) *//**7605i: eth3 is a debug interface**/
		if(((strncmp(ifp->name,"eth",3) == 0) && (judge_eth_interface(ifp->name)==ETH_INTERFACE)) 
		||(strncmp(ifp->name,"wlan",4) == 0)|| (strncmp(ifp->name,"ebr",3) == 0) 
		|| (strncmp(ifp->name,"vlan",4) == 0) ||(judge_radio_interface(ifp->name)==ENABLE_RADIO_INTERFACE)
		|| (strncmp(ifp->name,"ve",2) == 0) )
		#endif
		{
		/*for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))*/
		  {
			if(tipc_client_debug)
			   zlog_debug("%s: line%d, ifp->name = %s", __func__, __LINE__, ifp->name); 
		/* if ( CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)) */
			{
				if(tipc_client_debug)
				  zlog_debug("%s : line %d , goto send interface add when system up ......\n",__func__,__LINE__);
	  			master_send_interface_add (master_board, ifp);
			}
/*
			 else {
					master_send_interface_delete (master_board, ifp);	
				}
*/	   
			 if ( CHECK_FLAG (ifp->flags, IFF_UP)) 
			 {
			    if(tipc_client_debug)
				  zlog_debug("%s : line %d , goto send interface state(up) when system up ......\n",__func__,__LINE__);
				master_send_interface_update (ZEBRA_INTERFACE_UP, master_board, ifp);
			    }
			 else 
		 	{
		 	   if(tipc_client_debug)
				 zlog_debug("%s : line %d , goto send interface state(down) when system up ......\n",__func__,__LINE__);
				master_send_interface_update (ZEBRA_INTERFACE_DOWN, master_board, ifp);
			 }
		}
	
		  for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c)) 
		  	{
			/* for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) */
				{
					/*if (CHECK_FLAG (c->conf, ZEBRA_IFC_REAL)) *//*gujd: 2012-08-14,pm 4:53. Detele it for when Active master board rtmd restart , avoid interface set local ip address not send to vice board.*/
					{
						if(tipc_client_debug)
						 zlog_debug("%s : line %d, goto send interface address when system up......\n",__func__,__LINE__);
						/* SET_FLAG (c->conf, ZEBRA_IFC_CONFIGURED);*/
						 master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, master_board, ifp, c);
						}	
				}
			}
		  /*use rand time interval .10ms--110ms*/
			/*usleep(fetch_rand_time_interval());*//*2012-10-17, pm 4:15 .Optimized code .*/
	  	}
	}
	  if(tipc_client_debug)
	  	zlog_debug("leave func  %s ...\n",__func__);
	  return 0;
}



/*Only active master board packet interface linkdetection , and send it to vice board or bakup master board. */
int
master_send_interface_uplink_flag_update (int cmd, tipc_client *client, struct interface *ifp)
{
  struct stream *s;

  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header (s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc (s, ifp->uplink_flag);

  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}

/*Only active master board packet interface linkdetection , and send it to vice board or bakup master board. */

int
master_send_interface_linkdetection_update (int cmd, tipc_client *client, struct interface *ifp)
{
  struct stream *s;

  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header (s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
  
  /*zlog_debug("%s , line %d	put mtu = %d \n",__func__,__LINE__,ifp->mtu);*/
  stream_putl (s, ifp->mtu6);
  stream_putl (s, ifp->bandwidth);
#ifdef HAVE_SOCKADDR_DL
			stream_put (s, &ifp->sdl, sizeof (ifp->sdl));
#else
			stream_putl (s, ifp->hw_addr_len);
			
			if (ifp->hw_addr_len)
				stream_put (s, ifp->hw_addr, ifp->hw_addr_len);
#endif /* HAVE_SOCKADDR_DL */

  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}


/*interface information redistribute , send interface information to 
router deamon  and  vice board*/

int
master_send_interface_update (int cmd, tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  int ret=0;
#if 0
  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
  if(product->product_type == PRODUCT_TYPE_7605I)
  	DISABLE_REDISTRIBUTE_INTERFACE_7605I(ifp->name, ret);
  else
	DISABLE_REDISTRIBUTE_INTERFACE(ifp->name, ret);
  
	if(ret==1)
  	 return 0;
  
#endif
  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header (s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
  
  /*zlog_debug("%s , line %d	put mtu = %d \n",__func__,__LINE__,ifp->mtu);*/
  stream_putl (s, ifp->mtu6);
  stream_putl (s, ifp->bandwidth);
#ifdef HAVE_SOCKADDR_DL
			stream_put (s, &ifp->sdl, sizeof (ifp->sdl));
#else
			stream_putl (s, ifp->hw_addr_len);
			
			if (ifp->hw_addr_len)
				stream_put (s, ifp->hw_addr, ifp->hw_addr_len);
#endif /* HAVE_SOCKADDR_DL */

  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}



int
master_send_interface_descripton_set(int command,tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;
  int length = 0;
  
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
	   ||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
/*	   ||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)*/
	   ||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE)
	   ||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
 /*    if(DISABLE_REDISTRIBUTE_INTERFACE(ifp->name))*/
	   DISABLE_REDISTRIBUTE_INTERFACE(ifp->name, ret);
	   if(ret==1)
	    return 0;
	 
#endif


  /* Check this client need interface information. */
  if (! client->ifinfo){
  	if(tipc_client_debug)
	zlog_debug("! client->ifinfo");
	return 0;
  }
  s = client->obuf;
  stream_reset (s);
  length = strlen(ifp->desc);
  
  if(tipc_client_debug)
  	zlog_debug("%s : line %d , command[%s],interface (%s), desc(%s),length(%d).\n",
  			__func__,__LINE__,zserv_command_string(command),ifp->name,ifp->desc,length);
  /* Message type. */
  tipc_packet_create_header (s, command);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putl(s, INTERFACE_DESCRIPTON_SET);
  stream_putc(s, ifp->desc_scope);/*if desc is local or not ?*/
  stream_putl(s, length);
  stream_put (s, ifp->desc, length);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}

int
master_send_interface_descripton_unset(int command,tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;
  
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
	   ||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
/*	   ||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)*/
	   ||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE)
	   ||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
 /*    if(DISABLE_REDISTRIBUTE_INTERFACE(ifp->name))*/
	   DISABLE_REDISTRIBUTE_INTERFACE(ifp->name, ret);
	   if(ret==1)
	    return 0;
	 
#endif


  /* Check this client need interface information. */
  if (! client->ifinfo){
  	if(tipc_client_debug)
	zlog_debug("! client->ifinfo");
	return 0;
  }
  s = client->obuf;
  stream_reset (s);
  
  if(tipc_client_debug)
  	zlog_debug("%s : line %d , command[%s],interface (%s).\n",
  			__func__,__LINE__,zserv_command_string(command),ifp->name);
  /* Message type. */
  tipc_packet_create_header (s, command);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putl(s, INTERFACE_DESCRIPTON_UNSET);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}

int
master_send_interface_add (tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
	   ||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
/*	   ||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)*/
	   ||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE)
	   ||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
 if(product->product_type == PRODUCT_TYPE_7605I)
  	DISABLE_REDISTRIBUTE_INTERFACE_7605I(ifp->name, ret);
  else
	DISABLE_REDISTRIBUTE_INTERFACE(ifp->name, ret);
	   if(ret==1)
	    return 0;
	 
#endif

#if 0
	int peer_slot = 0;
	peer_slot = get_slot_num(ifp->name);
	/*not include ve interface and ve sub interface .*/
	if((peer_slot == client->board_id)&&(strncmp(ifp->name,"ve",2)!=0))
	{
		if(tipc_client_debug)
		  zlog_debug("Not send ADD (%s) to his own board .\n",ifp->name);
		return 0;
		}

#endif


  /* Check this client need interface information. */
  if (! client->ifinfo){
  	if(tipc_client_debug)
	zlog_debug("! client->ifinfo");
	return 0;
  }
  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, ZEBRA_INTERFACE_ADD);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);  
  /*zlog_debug("%s , line %d	put mtu = %d \n",__func__,__LINE__,ifp->mtu);*/
  stream_putl (s, ifp->mtu6);
  stream_putl (s, ifp->bandwidth);
#ifdef HAVE_SOCKADDR_DL
  stream_put (s, &ifp->sdl, sizeof (ifp->sdl));
#else
  stream_putl (s, ifp->hw_addr_len);
  if (ifp->hw_addr_len)
    stream_put (s, ifp->hw_addr, ifp->hw_addr_len);
#endif /* HAVE_SOCKADDR_DL */

/*gjd : add for local or global interface of Distribute System .*/
if(tipc_client_debug)
  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
  stream_putc(s,ifp->if_scope);
  
 /*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
if(tipc_client_debug)
  zlog_debug("%s, line %d, ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);
  stream_putl(s, ifp->slot);
  stream_putl(s, ifp->devnum);
	

  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}


int
master_send_interface_delete (tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  int ret=0;
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
	   ||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
/*	   ||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)*/
	   ||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE)
	   ||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
	   if(product->product_type == PRODUCT_TYPE_7605I)
		   DISABLE_REDISTRIBUTE_INTERFACE_7605I(ifp->name, ret);
		 else
	   DISABLE_REDISTRIBUTE_INTERFACE(ifp->name, ret);
	   if(ret==1)
	    return 0;
	 
#endif

#if 0
	  peer_slot = get_slot_num(ifp->name);
	  /*not include ve interface and ve sub interface .*/
	  if((peer_slot == client->board_id)&&(strncmp(ifp->name,"ve",2)!=0))
	  {
		  if(tipc_client_debug)
			zlog_debug("Not send DEL (%s) to his own board .\n",ifp->name);
		  return 0;
		  }
  
#endif

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);
  
  tipc_packet_create_header (s, ZEBRA_INTERFACE_DELETE);
  
  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
  stream_putl (s, ifp->mtu6);
  stream_putl (s, ifp->bandwidth);
  /*gjd : add for local or global interface of Distribute System .*/
  if(tipc_client_debug)
  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
  stream_putc(s,ifp->if_scope);
  
  stream_putl(s, ifp->slot);
  stream_putl(s, ifp->devnum);

  /* Write packet length. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}



int
master_send_interface_address (int cmd, tipc_client *client, 
                         struct interface *ifp, struct connected *ifc)
{
  int blen;
  struct stream *s = NULL;
  struct prefix *p = NULL;
  char buf[BUFSIZ];
  int ret = 0;
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
   	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
   	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#else
	   DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_VEXX(ifp->name, ret);
	   if(ret==1)
	    return 0;
	 
#endif
   	/**gjd: add for ckeck ipv6 address like prefix is fe80:: , for Distribute System . If it is , not redistribute.**/
#if 1
	if(ifc->address->family == AF_INET6)
	{
	  if(IPV6_ADDR_FE80(ifc->address->u.prefix6.in6_u.u6_addr16[0]))
	  {
	   if(tipc_client_debug)
	  	zlog_debug("%s : line %d , Don't go to sync ipv6 address like fe80::xxx .\n",__func__,__LINE__);
		return 0;
	  	}
	}
#endif
	/**2011-09-16: pm 5:45.**/
  
  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  if(NULL == s) {
  	if(tipc_client_debug)
		zlog_debug("%s, line = %d,s == NULL!\n ", __func__, __LINE__);
		return 0;
  }
  stream_reset (s);
  
  tipc_packet_create_header (s, cmd);
  if(tipc_client_debug)
  zlog_debug("%s, line %d, cmd ==  %d", __func__, __LINE__, cmd);
  /*stream_putl (s, ifp->ifindex);*/
  
  /*-----------------------------------------------------------*/
  /*stream_putl (s, ifp->name);*/
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  #if 0 /*gjd : 2011-09-01, 目前不支持ipv6，所以vice and bakup 板在新增加接口时，默认的ipv6地址发过来，把已注册的rpa口slot ,devnum又置成0导致下面注销rpa口时失败，所以把slot，devnum放到后面ipv4和ipv6检查后，再封装解析*/
  stream_putl(s, ifp->slot);
  stream_putl(s, ifp->devnum);
  if(tipc_client_debug)
   zlog_debug("%s, line %d, ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);
  /*------------------------------------------------------------*/
#endif
/**----gjd: packet ifc->conf when add/del ip address----**/
  stream_putc(s,ifc->conf);
/**----------------------------**/
  
  /* Interface address flag. */
  stream_putc (s, ifc->flags);
  if(tipc_client_debug)
   zlog_debug("%s, line %d, ifc->flags ==  %d", __func__, __LINE__, ifc->flags);
  /*gjd : add for local or global interface of Distribute System .*/
  
if(tipc_client_debug)
  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
  stream_putc(s,ifp->if_scope);

  /* Prefix information. */
  p = ifc->address;
  stream_putc (s, p->family);
  if(tipc_client_debug)
   zlog_debug("send p->family == %d", p->family);
  
  blen = prefix_blen (p);
  stream_put (s, &p->u.prefix, blen);
  if(tipc_client_debug)
   zlog_debug("master send interface %s : ip %s/%d \n",ifp->name,inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),p->prefixlen);

  /* 
   * XXX gnu version does not send prefixlen for ZEBRA_INTERFACE_ADDRESS_DELETE
   * but zebra_interface_address_delete_read() in the gnu version 
   * expects to find it
   */
  stream_putc (s, p->prefixlen);
  
  #if 1
	stream_putl(s, ifp->slot);
	stream_putl(s, ifp->devnum);
	if(tipc_client_debug)
	 zlog_debug("%s, line %d, ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);
#endif

  /* Destination. */
  p = ifc->destination;
  if (p)
    stream_put (s, &p->u.prefix, blen);
  else
    stream_put (s, NULL, blen);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));
 
  return master_send_message_to_vice(client);
}


void
master_redistribute_interface_up(struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;

#if 1
if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
  {
      zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);
  }
#endif
  
  if(zebrad.vice_board_list == NULL)
  {
    if(tipc_client_debug)
	 zlog_debug("%s, line = %d,zebrad.tipc_client_list is NULL", __func__, __LINE__);
	return;
  }

//  ret = product->board_id;
 // if (ret !=0 ) return;
 if(product->board_type != BOARD_IS_ACTIVE_MASTER)
   return ;
 
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
  {
    master_send_interface_update (ZEBRA_INTERFACE_UP, master_board, ifp);
  }
 
}



/* Interface down information. */
void
master_redistribute_interface_down (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;
#if 1  
  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
  	{
	    zsend_interface_update (ZEBRA_INTERFACE_DOWN, client, ifp);
  	}
#endif
  if(zebrad.vice_board_list == NULL)
  {
  if(tipc_client_debug)
	zlog_debug("%s, line = %d,zebrad.tipc_client_list is NULL", __func__, __LINE__);
	return;
  }

//  ret = product->board_id;
//  if (ret != 0) return;
  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	return ;
  
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
  	{ 
	    master_send_interface_update (ZEBRA_INTERFACE_DOWN, master_board, ifp);
	}

}

/* Interface information update. */
void
master_redistribute_interface_add (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;
  int peer_slot = 0;
  
  /*gujd : 2012-10-13 . Optimized code . Send to ospf waiting for getting index by netlink.*/  
#if 0
  /*send message to router deamon*/
  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) 
  {
    if (client->ifinfo)
      zsend_interface_add (client, ifp);
  }
#endif

  if(zebrad.vice_board_list == NULL)
	{
	if(tipc_client_debug)
	  zlog_debug("%s, line %d, zebrad.vice_board_list (NULL)....", __func__, __LINE__);
	  return;
	}
  
/*gujd : 2012-10-13 . Optimized code . Don't need.*/
/*
   if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	 return ;
*/

   /*send message to all of connected vice board*/
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
  	{	
	  // if (client->ifinfo) {
	  if(tipc_client_debug)
	  zlog_debug("enter redistrubute to all of vice_board add_interface_name = %s", ifp->name); 
	  
	  /*gujd: 2012-09-22, am 10:20 .Add for when recive create or delete from his borad, only send other board not send his own board again.*/			
#if 0
			peer_slot = get_slot_num(ifp->name);
			/*not include ve interface and ve sub interface .*/
			if((peer_slot == master_board->board_id)&&(strncmp(ifp->name,"ve",2)!=0))
			{
				if(tipc_client_debug)
				  zlog_debug("Not send ADD (%s) to his own board .\n",ifp->name);
				continue;
			}
		
#endif
	   		master_send_interface_add (master_board, ifp);
	   	//}
    }
}



void
master_send_vurtual_interface_add_to_vice (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;
  int peer_slot = 0;
  
#if 0
  /*send message to router deamon*/
  if(ifp->if_types != VIRTUAL_INTERFACE )
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) 
  {
    if (client->ifinfo)
      zsend_interface_add (client, ifp);
  }
#endif

  if(zebrad.vice_board_list == NULL)
	{
	if(tipc_client_debug)
	  zlog_debug("%s, line %d, zebrad.vice_board_list (NULL)....", __func__, __LINE__);
	  return;
	}


   if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	 return ;

   /*send message to all of connected vice board*/
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
  	{	
	  // if (client->ifinfo) {
	  if(tipc_client_debug)
	  zlog_debug("enter redistrubute to all of vice_board add_interface_name = %s", ifp->name); 
	  
	  /*gujd: 2012-09-22, am 10:20 .Add for when recive create or delete from his borad,only send other board not send his own board again.*/			
#if 0
			peer_slot = get_slot_num(ifp->name);
			/*not include ve interface and ve sub interface .*/
			if((peer_slot == master_board->board_id)&&(strncmp(ifp->name,"ve",2)!=0))
			{
				if(tipc_client_debug)
				  zlog_debug("Not send ADD (%s) to his own board .\n",ifp->name);
				continue;
			}
		
#endif
	   		master_send_interface_add (master_board, ifp);
	   	//}
    }
}



void
master_redistribute_interface_delete_loacal(struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;

#if 1
  if(ifp->if_types != VIRTUAL_INTERFACE )
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo)
      zsend_interface_delete (client, ifp);
  }
#endif

}

void
master_redistribute_interface_delete (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  int ret;
  int peer_slot = 0;

#if 1
  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo)
      zsend_interface_delete (client, ifp);
  }
#endif

  if(zebrad.vice_board_list == NULL)
	{
	if(tipc_client_debug)
	  zlog_debug("%s, line = %d,zebrad.tipc_client_list is NULL....", __func__, __LINE__);
	  return;
	}
#if 1
//  ret = product->board_id;
//  if (ret != 0) return;
  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	return ;

  
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
  	{  		 
	   //if (client->ifinfo) {
	   
	   /*gujd: 2012-09-22, am 10:20 .Add for when recive create or delete from his borad, only send other board not send his own board again.*/			
#if 1
			peer_slot = get_slot_num(ifp->name);
			/*not include ve interface and ve sub interface .*/
			if((peer_slot == master_board->board_id)&&(strncmp(ifp->name,"ve",2)!=0))
			{
				if(tipc_client_debug)
				  zlog_debug("Not send DEL (%s) to his own board .\n",ifp->name);
				continue;
			}
		
#endif
	   
	   		master_send_interface_delete (master_board, ifp);
	   	//}
	}
#endif
}




/* Interface address addition. */
void
master_redistribute_interface_address_add (struct interface *ifp,
				    struct connected *ifc)
{
  if(tipc_client_debug)
	zlog_debug("%s: enter %s, line == %d",__FILE__, __func__, __LINE__);

  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  struct prefix *p;
  char buf[BUFSIZ];
  int ret;

  if(ifp->if_types == VIRTUAL_INTERFACE || ifp->ifindex == IFINDEX_INTERNAL)
 	return;

   #if 0   /*log_message*/  
      p = ifc->address;
      zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADDRESS_ADD %s/%d on %s",
		  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		  p->prefixlen, ifc->ifp->name);
    #endif

  router_id_add_address(ifc);

  if (!zebrad.client_list) {
	zlog_debug("%s, zebrad.client_list == NULL", __func__);
	return ;
  }
  if(tipc_client_debug)
   zlog_debug(" enter %s, line == %d", __func__, __LINE__); 

  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
      zsend_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, client, ifp, ifc);
  	}
  if(tipc_client_debug)
   zlog_debug("enter %s, line == %d", __func__, __LINE__);

  if(zebrad.vice_board_list == NULL)
  {
  if(tipc_client_debug)
	zlog_debug("%s, line = %d :tipc_client_list is NULL....", __func__, __LINE__);
	return;
  }

  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	return ;
  
  if(tipc_client_debug)
  zlog_debug("enter %s, line %d", __func__, __LINE__);
  
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
  	{
  		if (master_board->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) 
		{
			master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, master_board, ifp, ifc);
  		}
	}
  if(tipc_client_debug)
  zlog_debug("leave %s, line %d", __func__, __LINE__);

}


/* Interface address deletion. */
void
master_redistribute_interface_address_delete (struct interface *ifp,
				       struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client  *master_board;
  struct prefix *p;
  char buf[BUFSIZ];
  int ret;

  if(ifp->if_types == VIRTUAL_INTERFACE || ifp->ifindex == IFINDEX_INTERNAL)
 	return;
  if(tipc_client_debug)
 	zlog_debug ("Func %s start, line %d ",__func__,__LINE__);
#if 0
      /*log message*/
      p = ifc->address;
      zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADDRESS_DELETE %s/%d on %s",
		  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		  p->prefixlen, ifc->ifp->name);
#endif    

  router_id_del_address(ifc);

  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
      zsend_interface_address (ZEBRA_INTERFACE_ADDRESS_DELETE, client, ifp, ifc);

  	}

  if(zebrad.vice_board_list == NULL)
  {
  if(tipc_client_debug)
	zlog_debug("%s, line = %d :tipc_client_list is NULL", __func__, __LINE__);
	return;
  }


  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
   return ;
  
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
  	{
  		if (master_board->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) 
		{	
			master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_DELETE, master_board, ifp, ifc);
  		}
	}
}

 
 
 struct interface *
 master_interface_state_set(struct interface *ifp, uint64_t flags, int rpa_done, int normal_done)
{
int ret;
	 if((ifp->if_types == RPA_INTERFACE)&& (rpa_done == 1)&&(normal_done == 0) )/*rpa :  only running use*/
	 {
		 if(CHECK_FLAG(flags, IFF_RUNNING))
		 {
			 ret = rpa_interface_running_state_set(ifp);/*set running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
		 else
		 {
			 ret = rpa_interface_running_state_unset(ifp);/*unset running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
	 }
	 
	 if((ifp->if_types == RPA_INTERFACE)&& (rpa_done == 0)&&(normal_done == 1) )/*rpa : UP only use*/
	 {
		 if(CHECK_FLAG(flags, IFF_UP))
		 {
			 ret = if_set_flags(ifp, (uint64_t)(IFF_UP));/*set UP*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
		 else
		 {
			 ret = if_unset_flags(ifp,(uint64_t)(IFF_UP));/*unset running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
		 }
	 if((ifp->if_types == RPA_INTERFACE)&& (rpa_done == 1)&&(normal_done == 1) )/*rpa : up and running only use*/
	 {
		 if(CHECK_FLAG(flags, IFF_UP))
		 {
			 ret = if_set_flags(ifp, (uint64_t)(IFF_UP));/*normal set UP*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
	 // 	 return ifp;
		 }
		 if(CHECK_FLAG(flags, IFF_RUNNING))
		 {
			 ret = rpa_interface_running_state_set(ifp);/*normal set running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
		 //  return ifp;
		 }
		 return ifp;
	 }

	 if((ifp->if_types != VIRTUAL_INTERFACE)&&(normal_done == 1)&&(rpa_done == 0))/*一般实口和rpa口的down按normal来操作*/
	 {
		 if((CHECK_FLAG(flags,IFF_UP))&&(CHECK_FLAG(flags,IFF_RUNNING)))/*一般口up and running*/
		 {
			 ret = if_set_flags(ifp,(uint64_t)(IFF_UP|IFF_RUNNING));
			 if(ret < 0)
			 {
				 zlog_warn("set real interface %s (up and running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
		 if(CHECK_FLAG(flags,IFF_UP))/*一般口up，另外一般口不会单设running*/
		 {
			 ret = if_set_flags(ifp,(uint64_t)(IFF_UP));
			 if(ret < 0)
			 {
				 zlog_warn("set real interface %s (up state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
		 else/*一般口的down和rpa口的down都按常规操作*/
		 {
			 ret = if_unset_flags(ifp,(uint64_t)(IFF_UP));
			 if(ret < 0)
			 {
				 zlog_warn("unset real or rpa interface %s (down state) failed.\n",ifp->name);
				 
			 }
			 if_refresh(ifp);
			 return ifp;
		 }
	 }
	 
#if 0		
	 if(ifp->if_types == VIRTUAL_INTERFACE)
	 {
		 ifp->flags = tmp_flags;
		 master_redistribute_interface_up(ifp);
		 return ifp;
	 }
#endif	
	 
}



struct interface *
tipc_master_zebra_interface_description_update_read (struct stream *s, int command)
{

  struct interface *ifp;
  char ifname_tmp[INTERFACE_NAMSIZ];
  int done = 0;

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  done = stream_getl(s);/*1 is set , 0 is unset.*/
  
  
  /* Lookup/create interface by name. */
  ifp = check_interface_exist_by_name(ifname_tmp);
  if(!ifp)
  {
  	zlog_info("%s: interface[%s] not exist !\n",__func__,ifname_tmp);
	return NULL;
  	}

  if(command == ZEBRA_INTERFACE_DESCRIPTION_SET)/*set*/
  {
	  int length = 0;
	  char *str = NULL;
	  
	  ifp->desc_scope = stream_getc(s);
	  if(CHECK_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL))
	  {
	  	if(judge_real_local_interface(ifp->name)== LOCAL_BOARD_INTERFACE)
	    {
		  length = stream_getl(s);
		  str = XMALLOC(MTYPE_TMP, (length+1));/*+1 for \0.*/

	      if(ifp->desc)/*free before, then update.*/
	       XFREE (MTYPE_TMP, ifp->desc); 
		  
		  stream_get(str,s,length);
		  *(str+length) = '\0';/*last is '\0' */
		  
		  ifp->desc = str;
		  
		  if(tipc_client_debug)
		  	zlog_debug("interface(%s),set local descripton(%s) is local board.\n",ifp->name,ifp->desc);
	  	 }
		 else
		  {
			if(tipc_client_debug)
			  zlog_debug("interface(%s),set local descripton(%s) is not local board.\n",ifp->name,ifp->desc);
		  }
	   }
	  else
	  {
		 length = stream_getl(s);
		 str = XMALLOC(MTYPE_TMP, (length+1));/*+1 for \0.*/

	     if(ifp->desc)/*free before, then update.*/
	      XFREE (MTYPE_TMP, ifp->desc); 
		  
		 stream_get(str,s,length);
		 *(str+length) = '\0';/*last is '\0' */
		  
		 ifp->desc = str;
		  
		 if(tipc_server_debug)
		  	zlog_debug("interface(%s), descripton(%s).\n",ifp->name,ifp->desc);
		 }
	}
  else/*unset*/
  {
	 if(ifp->desc)/*free before, then update.*/
	  XFREE (MTYPE_TMP, ifp->desc);
	 ifp->desc = NULL;
  	}
  
  return ifp;

}

int
tipc_master_interface_description_set (int command, tipc_client *master_board,
			   zebra_size_t length)
{
  struct interface *ifp = NULL;
  struct stream *s = NULL;
  s = master_board->ibuf;
  int ret;

  ifp = tipc_master_zebra_interface_description_update_read(s,command);

 
  return 0;
}

int
tipc_master_interface_description_unset (int command, tipc_client *master_board,
			   zebra_size_t length)
{
  struct interface *ifp = NULL;
  struct stream *s = NULL;
  s = master_board->ibuf;
  int ret;

  ifp = tipc_master_zebra_interface_description_update_read(s,command);

 
  return 0;
}



struct interface *
tipc_master_zebra_interface_state_read (int command ,struct stream *s)
{
	struct interface *ifp;
	char ifname_tmp[INTERFACE_NAMSIZ];
	uint64_t tmp_flags = 0;
	unsigned int tmp_mtu = 0;
	int ret = 0;
	char tmp_status = 0;
	
	if (s == NULL) {
		if(tipc_client_debug)
		zlog_debug("zebra_interface_state struct stream is null\n");
		return NULL;
	}
	


  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  
  if (judge_real_interface(ifname_tmp)){
  	if(tipc_client_debug)
     zlog_debug("%s: line  %d, interface %s  is real interface..\n",__func__, __LINE__,ifname_tmp);
     return ifp;
  }
  
#if 0
  /* Lookup this by interface name. */
  ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 #endif

/**check interface is exist ?**/
ifp = check_interface_exist_by_name_len(ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  /* If such interface does not exist, indicate an error */
  if (ifp == NULL) {
  	if(tipc_client_debug)
	 zlog_debug(" interface do not exist or the tipc interface not match!\n");
     return NULL;
  	}
  
  /* Read interface's value. */
/* ifp->status = stream_getc (s);*/
  tmp_status = stream_getc(s);
	
/**gjd : add for check interface flags, if same don't send to other boards **/
  tmp_flags = stream_getq(s);
 
  ifp->metric = stream_getl (s);
 
  tmp_mtu= stream_getl (s);
  if(tipc_client_debug)  
 	 zlog_debug("%s: line %d ,ifp->name = %s ,ifp->status = %d, tmp_status = %d, ifp->flags = 0x%llx, tmp_flags = 0x%llx,old mtu = %d ,new mtu = %d\n",
 	 			__func__,__LINE__,ifp->name,ifp->status, tmp_status, ifp->flags, tmp_flags, ifp->mtu, tmp_mtu);
  ifp->mtu6 = stream_getl (s); 
   if((tmp_mtu != ifp->mtu)&&(ifp->if_types == RPA_INTERFACE ) )/*同步过来的消息不能更改实口的mtu，只能修改rpa口*/
 {

	ifp->mtu = tmp_mtu;
	ret = if_set_mtu(ifp);
	if(ret < 0)
	  zlog_warn("ioctl set mtu failed \n");
	
 	}
   
   if((tmp_mtu != ifp->mtu)&&(ifp->if_types == VIRTUAL_INTERFACE ))
  {

	ifp->mtu = tmp_mtu;
	#if 0/*virtual interface not go to set mtu in kernel.*/
	ret = if_set_mtu(ifp);
	if(ret < 0)
	 zlog_warn("ioctl set mtu failed \n");
	#endif
  }
  ifp->bandwidth = stream_getl (s);
  if(command == ZEBRA_INTERFACE_DELETE)
  {
  	
	if(tipc_client_debug)  
	  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
  	ifp->if_scope = stream_getc(s);
	if(tipc_client_debug)  
		zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
	
	/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
	ifp->slot = stream_getl(s);
	ifp->devnum = stream_getl(s);
	if(tipc_client_debug)
		zlog_debug("%s : line %d, interface(%s), slot(%d),devnum(%d).\n",__func__,__LINE__,ifp->name,ifp->slot,ifp->devnum);
  	}
/*gujd: 2012-09-04, pm 6:00 Add for interface sync mac addr. */  
#ifdef HAVE_SOCKADDR_DL
	  stream_get (&ifp->sdl, s, sizeof (ifp->sdl));
#else
	  ifp->hw_addr_len = stream_getl (s);
	  /*zlog_debug("%s, line == %d, ifp->ifindex = %d \n", __func__, __LINE__, ifp->hw_addr_len);*/
		
	  if (ifp->hw_addr_len)
	  {
		  unsigned char hw_addr[INTERFACE_HWADDR_MAX]={0};
		
	   /* stream_get (ifp->hw_addr, s, ifp->hw_addr_len);*/
		stream_get (hw_addr, s, ifp->hw_addr_len);
		if(memcmp(ifp->hw_addr ,hw_addr ,ifp->hw_addr_len))/*mac is not same*/
		  {
			  /*copy*/
			  memcpy(ifp->hw_addr ,hw_addr ,ifp->hw_addr_len);
  
  			/*to set new :  local interface do not, virtual interface do not .*/
  			if(judge_real_local_interface(ifp->name)==OTHER_BOARD_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )
			  {
				  int ret = 0;
				  ret = if_set_mac_addr(ifp->name ,ifp->hw_addr);
				  if(ret < 0)
				  {
					  zlog_warn("interface (%s)set mac failed : %s .\n",ifp->name,safe_strerror(errno));
				  }
			  }
			  else
			  {
				  /*local interface do nothing .*/
			  }
		  }
		else/*mac is same*/
		  {
			  /*do nothing*/
		  }
		int i;
		zlog_info("mac is : ");
		for(i=0;i<ifp->hw_addr_len;i++)
		{
			zlog_info("%x ",ifp->hw_addr[i]);
		}
		
		/*
		zlog_debug("%s, line %d,hw_addr[%d],ifp->hw_addr_len[%d], ifp->hw_addr[%d] .\n",
				__func__, __LINE__,hw_addr,ifp->hw_addr_len,ifp->hw_addr);
		*/		
		}
#endif /* HAVE_SOCKADDR_DL */

  
#if 1
  if(command == ZEBRA_INTERFACE_UP)
  {

  
  ifp->status = tmp_status;
  #if 0
	  if(if_is_up(ifp))
	  {
	   if(tipc_client_debug)
		  zlog_debug("%s : line %d , the interface %s flags(state: up ) is same ....\n",__func__,__LINE__,ifp->name);
	   return NULL;
	  }
	  else
		  ifp->flags = tmp_flags;
	#endif
	#if 0
	if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
	{
		 ifp->flags = tmp_flags;
		 master_redistribute_interface_up(ifp);
		 return ifp;
	 }
	#endif
	#if 1
	if(CHECK_FLAG(tmp_flags,IFF_UP))
	{
		if(CHECK_FLAG(tmp_flags,IFF_RUNNING))/*tmp is up and running*/
		{
			if(tipc_client_debug)
				zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
			if(if_is_up(ifp))/*ifp : up*/
			{
				if(tipc_client_debug)
					zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
			 	if(if_is_running(ifp))
				{
	
					if(tipc_client_debug)/*ifp :running*/
			  		  zlog_debug("%s : line %d , the interface %s flags(state: up and running ) is same ....\n",__func__,__LINE__,ifp->name);
		   			return ifp;
				}
				else
				{
					if(tipc_client_debug)
					  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx , if_type = %d.\n",__LINE__,tmp_flags,ifp->flags,ifp->if_types);
					
				/*	ifp->flags |= IFF_RUNNING;*//*go to ifp + running*/
					if(ifp->if_types == RPA_INTERFACE)
					{
					  if(tipc_client_debug)
						zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					  master_interface_state_set(ifp,(uint64_t)(IFF_RUNNING),1,0);
					}
#if 1
					if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
					{
						if(tipc_client_debug)
							zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
						 ifp->flags = tmp_flags;
						 master_redistribute_interface_up(ifp);
						 return ifp;
					 }
#endif
				/*	else*//*单独让一个实口running不会出现*/
						
					return ifp;	
				}
			}
			else/*ifp : down, goto ifp up + running*/
			{
#if 1
				if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
				{
					if(tipc_client_debug)
					  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				 	ifp->flags = tmp_flags;
				 	master_redistribute_interface_up(ifp);
				 	return ifp;
			 	}
#endif
			/*	ifp->flags |= (IFF_UP | IFF_RUNNING);*/
				if(ifp->if_types == RPA_INTERFACE)
				{
					if(tipc_client_debug)
					  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				  	 master_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),1,1);/*rpa口*/
					}
				else
				{
					if(tipc_client_debug)
					  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				    master_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),0,1);/*一般口*/
				  }
			}
			return ifp;
			
		}
		else/*tmp is up, not running */
		{
		
			if(tipc_client_debug)
				zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
			if(if_is_up(ifp))/*ifp : up*/
			{
			
				if(tipc_client_debug)
				  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
			 	if(!if_is_running(ifp))/*ifp : not running*/
				{
				
					if(tipc_client_debug)
			  		  zlog_debug("%s : line %d , the interface %s flags(up state, not running ) is same ....\n",__func__,__LINE__,ifp->name);
		   			return ifp;
				}
				else
				{
				
				  if(tipc_client_debug)
				    zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
#if 1
				  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
				  {
					 if(tipc_client_debug)
						zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					 ifp->flags = tmp_flags;
					 master_redistribute_interface_up(ifp);
					 return ifp;
				   }
#endif
				/*	ifp->flags &= (~IFF_RUNNING);*//*ifp : running*/
				  if(ifp->if_types == RPA_INTERFACE)
				  	{
					  if(tipc_client_debug)
						zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					  master_interface_state_set(ifp,(uint64_t)(~IFF_RUNNING),1,0);/*rpa口去掉running*/
				  	}
				  
				}
				return ifp;
				/*一般实口不会单独去掉running*/
			}
			else/*ifp : down*/
			{
			
			  if(tipc_client_debug)
			     zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
#if 1
			  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
			  {
				if(tipc_client_debug)
				  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				ifp->flags = tmp_flags;
				 master_redistribute_interface_up(ifp);
				return ifp;
				 }
#endif
				/*ifp->flags |= IFF_UP ;*//*rpa口与实口up起来按常规处理*/
				if(ifp->if_types == RPA_INTERFACE)
				{
				  if(tipc_client_debug)
				    zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				   master_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*rpa口*/

				}
				else
				{
				  if(tipc_client_debug)
				    zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				  master_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*一般口*/
				}
			}
			
			return ifp;
			
		}
		
	}
	
	else
	{
		/*the interface status : linkdetection case this */
		if(tipc_client_debug)
  		  zlog_debug("line %d , UP command down flags to up route :status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx .\n",
							__LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
		if_up(ifp);
		return ifp;
	  	}
	#endif
  }
  
  if(command == ZEBRA_INTERFACE_DOWN)
  {
  
  ifp->status = tmp_status;
  #if 0
	  if(!(if_is_up(ifp)))
	  {
	   if(tipc_client_debug)
		  zlog_debug("%s : line %d , the interface %s flags(state: down ) is same ....\n",__func__,__LINE__,ifp->name);
	   return NULL;
	  }
	  else
		  ifp->flags = tmp_flags;
 #endif

 #if 1
 if(!CHECK_FLAG(tmp_flags,IFF_UP))/*tmp is down*/
 {
	
		if(if_is_up(ifp))/*ifp : up*/
		{
		/*	ifp->flags &= ~IFF_UP;*//*to down*/
		
		 if(tipc_client_debug)
		  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
		
#if 1
		 if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接重发布到各业务板*/
		 {
		
		   if(tipc_client_debug)
			zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
		   ifp->flags = tmp_flags;
		   master_redistribute_interface_down(ifp);
		   return ifp;
		  }
#endif
		 {
			if(tipc_client_debug)
			  zlog_debug("line %d: tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				
				master_interface_state_set(ifp, (uint64_t)(~IFF_UP),0,1);/*rpa口和一般口down操作都用normal处理*/
			}		
		 }
		else/*ifp : down*/
		{
			if(tipc_client_debug)
		  	  zlog_debug("%s : line %d , the interface %s flags(state: down) is same ....\n",__func__,__LINE__,ifp->name);
	   	    return ifp;
		}
		
	}
 
 else /*tmp : is up , the detection is on, not change the intf flags , only to change the route.*/
   {
  	  if(tipc_client_debug)
	   zlog_debug("line %d , DOWN command up flags to down route :status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx ############...\n",
					   __LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
	   if(ifp->if_types == RPA_INTERFACE)
	   {
		 if(tipc_client_debug)
		   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
		 master_interface_state_set(ifp,(uint64_t)(~IFF_RUNNING),1,0);/*rpa口去掉running*/
	   }	   
	   if(tipc_client_debug)
	   	zlog_debug("line %d , status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx .\n",
					   __LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
   /* if_down_redistribute(ifp);*/
	   return ifp;
   }
 #endif
  }
#endif
  if(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)/*gujd : 2012-05-04, pm 5:30. Add code for use netlink message to delete ve sub.*/
	return ifp;
  
  ifp->status = tmp_status;
    
  return ifp;
}


struct interface *
tipc_master_zebra_interface_add_read (struct stream *s)
{

  struct interface *ifp;
  char ifname_tmp[INTERFACE_NAMSIZ];
  
  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  if(tipc_client_debug)
   zlog_debug("master recv interface %s", ifname_tmp);


 /*ckeck interface is exist ?**/
  ifp = check_interface_exist_by_name(ifname_tmp);
  if(ifp) /**when exist , (real and rpa interface return)  stop  and doesn ' t go to add**/
  {
   if(ifp->ifindex != 0)
   	{
   		if(judge_ve_interface(ifp->name)== VE_INTERFACE)/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .To make it has add to rpa table.*/
			goto skip;
   		if(tipc_client_debug)
		  zlog_debug("%s : line %d ,The interface %s is exist(rpa or real) on this system !\n",__func__,__LINE__,ifname_tmp);
		return ifp;
   	}
   else
   	{
   	
		ifp->if_types = VIRTUAL_INTERFACE;
   		if(tipc_client_debug)
		  zlog_debug("%s : line %d ,The interface %s is exist(virtual) on this system, so go on set value !\n",__func__,__LINE__,ifname_tmp);
		goto skip;
   	}
  }
  else  /**not exist , go to creat( or say add)**/
  {   /*create virtual interface*/
	  ifp = if_create(ifname_tmp, strlen(ifname_tmp));
	  if(!ifp)
	  {
	   /*CID 11030 (#1 of 1): Dereference after null check (FORWARD_NULL)
		7. var_deref_op: Dereferencing null pointer "ifp".
		Problem . If ifp is null ,  add return NULL.*/
		zlog_warn("%s: line %d, Can't create interface %s in this system!\n",__func__,__LINE__,ifname_tmp);
	    return NULL;
	  }
	  ifp->if_types = VIRTUAL_INTERFACE;
	   if(tipc_client_debug)
		 zlog_debug(" create interface %s (virtual) !\n",ifp->name);
  }
  if(tipc_client_debug)
    zlog_debug("%s,line = %d,ifp->ifindex = %d", __func__,__LINE__, ifp->ifindex);

skip:/*虚口去更新赋值*/
  /* Read interface's index. */
 /* ifp->ifindex = stream_getl (s);*/
  
  /* Read interface's value. */
  ifp->status = stream_getc (s);
// zlog_debug("%s, line == %d, ifp->status = %d \n", __func__, __LINE__, ifp->status); 
  ifp->flags = stream_getq (s);
 // zlog_debug("%s, line == %d, ifp->falgs = %d \n", __func__, __LINE__, ifp->flags);
  ifp->metric = stream_getl (s);
//  zlog_debug("%s, line == %d, ifp->metric = %d \n", __func__, __LINE__, ifp->metric);
  ifp->mtu = stream_getl (s);
 // zlog_debug("%s, line == %d, ifp->mtu = %d \n", __func__, __LINE__, ifp->mtu);
  ifp->mtu6 = stream_getl (s);
 // zlog_debug("%s, line == %d, ifp->mtu6 = %d \n", __func__, __LINE__, ifp->mtu6);
  ifp->bandwidth = stream_getl (s);
 // zlog_debug("%s, line == %d, ifp->bandwidth = %d \n", __func__, __LINE__, ifp->bandwidth);

#ifdef HAVE_SOCKADDR_DL
  stream_get (&ifp->sdl, s, sizeof (ifp->sdl));
#else
  ifp->hw_addr_len = stream_getl (s);
  //zlog_debug("%s, line == %d, ifp->ifindex = %d \n", __func__, __LINE__, ifp->hw_addr_len);
	
  if (ifp->hw_addr_len){
    stream_get (ifp->hw_addr, s, ifp->hw_addr_len);
//  	zlog_debug("%s, line == %d, ifp->hw_addr = %d \n", __func__, __LINE__, ifp->hw_addr);
  	}
#endif /* HAVE_SOCKADDR_DL */
	ifp->if_scope = stream_getc(s);

	/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .To sync devnum between boards .*/
    if(tipc_client_debug)
	 zlog_debug(" %s : line %d , when add/del ip recv rpa interface: slot %d , devnum %d .\n",__func__,__LINE__,ifp->slot,ifp->devnum);
	ifp->slot = stream_getl(s);
	ifp->devnum = stream_getl(s);
 
  return ifp;

}




struct connected *
tipc_master_connected_add_by_prefix (struct interface *ifp, struct prefix *p, 
                         struct prefix *destination)
{
  struct listnode *node3;
  struct connected *ifc;
  struct prefix *q;
  int ret;

 if(connected_check (ifp, p)!= NULL) {
 	if(tipc_client_debug)
	zlog_debug("%s, line %d,add the same address, so return....\n",__func__,__LINE__);
	return NULL;
 }
  /* Allocate new connected address. */
  ifc = connected_new ();
  ifc->ifp = ifp;
  if(tipc_client_debug)
  zlog_debug("%s: line %d ,ifc->ifp->name == %s\n",__func__ ,__LINE__,ifc->ifp->name);

  /* Fetch interface address */
  ifc->address = prefix_new();
  memcpy (ifc->address, p, sizeof(struct prefix));
  	

  /* Fetch dest address */
  if (destination)
  {   
    ifc->destination = prefix_new();
    memcpy (ifc->destination, destination, sizeof(struct prefix));
    
  }

  if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE)
  	||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)
  	||(judge_eth_interface(ifp->name)==ETH_INTERFACE))/*ve sub interface , eth sub interface and eth interface deal as normal*/
   {
   	  int slot;
   	 
	  ifp->devnum = 0;
	 /* ifp->slot = 0;*/
	  if(judge_eth_interface(ifp->name)==ETH_INTERFACE)/*eth interface*/
	  {
	  	slot = get_slot_num(ifp->name);
		if(slot == product->board_id)/*local board*/
		{
	  	  ifp->if_types = REAL_INTERFACE;
		  ifp->slot = slot;
		}
		else
		{
			ifp->if_types = RPA_INTERFACE;
			ifp->slot = slot;
		}
	  }
	  else
	  	{
	  		ifp->if_types = REAL_INTERFACE;/*sub eth and sub ve deal as real interface*/
			ifp->slot = 0;/*not use*/
	  	}
	  
	  goto skip;
	 }

  
#if 1
/*----------------------------------------------------------------------*/
  if (ifp->if_types == VIRTUAL_INTERFACE)
  {	
	/*create_rpa_interface();*/
	ret = create_rpa_interface(ifp);
	 if(ret < 0)
	 {
	 	zlog_debug("%s : line %d, creat rpa interface failed : %s \n ",__func__,__LINE__,safe_strerror(errno));
		return NULL;
	 }
	 else
	 {	
		/*virtual interface ==> RPA interface*/
		ifp->if_types = RPA_INTERFACE;
	 	zlog_debug("%s : line %d, creat rpa interface sucess, so virtual turn to rpa interface...\n",__func__,__LINE__);
	 	}
	 
	 /*gujd : 2012-10-13 . Optimized code . Get index by netlink.*/
	 #if 0
	 /*Need , update ifindex because use master_redistribute_interface_add to redistribute ospf and  other board.*/
	 sleep(1);
	 
	 ifp->ifindex = if_get_index_by_ioctl(ifp);
  	 if(ifp->ifindex <= 0)
  	  {
  		zlog_warn("get rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
		return NULL;
  	  }
 	 zlog_debug("get rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);
	 
	 ifc->ifp->ifindex = ifp->ifindex;/**ifc 下的ifp中ifindex也要更新，ifc用于后面发送到动态路由进程*/
	#endif
	
	 master_redistribute_interface_add(ifp);
	 
  }	
  /*----------------------------------------------------------------------*/
  #endif
/*  ret = ip_address_count(ifp);*/
  
  ret = ip_address_count_except_fe80(ifp);
  if (ret < 1)
  {
  
   if(ifp->if_types == REAL_INTERFACE)
  	{  	
	  if((strncmp(ifp->name, "ve",2) != 0 && strncmp(ifp->name, "obc",3)!= 0))
	  {
  		ret = register_rpa_interface_table(ifp);
		if(ret < 0)
	  	{
	  		zlog_warn("register rpa interface table in local board failed : %s ..\n",safe_strerror(errno));
	  	}
		zlog_debug(" get slot %d ,devnum %d ...\n",ifp->slot,ifp->devnum);
		if(ifp->if_types == 0)
		 ifp->if_types = REAL_INTERFACE;/**实口才去注册表项，若type为0注册后type应为实口**/
	  }
   	}
#if 0
	  else
	  	if(strlen(ifp->name) >= ETH_SUB_MIN)/*eth sub interface*/
		{
	  		ret = register_rpa_interface_table(ifp);
			if(ret < 0)
		  	{
		  		zlog_warn("register rpa interface table in local board failed : %s ..\n",safe_strerror(errno));
		  	}
			zlog_debug(" get slot %d ,devnum %d ...\n",ifp->slot,ifp->devnum);
		  }	
#endif	  
  	}

 /*add address to kernel*/
 skip:
 #if 0

  ret = if_set_prefix (ifc->ifp, ifc);
  if (ret < 0) {
  	
     zlog_debug("%s, line == %d, if_set_prefix failed:-----%s\n", __func__, __LINE__,safe_strerror(errno));
     return NULL;
  }
  #endif
  if(ifc->address->family == AF_INET)
	{
		if(tipc_client_debug)
		 zlog_debug("%s : line %d , ipv4 : %d ....\n",__func__,__LINE__,ifc->address->family);
	    /*add by gjd: for rpa and real interface to add address to kernel*/
		ret = if_set_prefix (ifc->ifp, ifc);
	    if (ret < 0) 
		{
	    	zlog_err("%s: line %d, kernel set ipv4 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
	   		return NULL;
	    }
	  }
	else
		if(ifc->address->family == AF_INET6)
		{
			if(tipc_client_debug)
			 zlog_debug("%s : line %d , ipv6 : %d ....\n",__func__,__LINE__,ifc->address->family);
		    /*add by gjd: for rpa and real interface to add address to kernel*/
			UNSET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
			SET_FLAG(ifc->conf,ZEBRA_IFC_CONFIGURED);
			
			ret = if_prefix_add_ipv6 (ifc->ifp, ifc);
		    if (ret < 0) 
			{
				zlog_err("%s: line %d, kernel set ipv6 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
		   		return NULL;
		    }
			SET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
		  }
		else
			{
				
				zlog_debug("%s : line %d , errXXXX  %d ....\n",__func__,__LINE__,ifc->address->family);
			}
  /* Add connected address to the interface. */
  if(tipc_client_debug)
  	zlog_debug("%s : line %d ,-------master---------go to add address......\n",__func__,__LINE__);
  listnode_add (ifp->connected, ifc);
  

  return ifc;
}




struct connected *
tipc_master_connected_delete_by_prefix (struct interface *ifp, struct prefix *p)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  int ret = 0;
  
 if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE)
	 ||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE))
/*	 ||(judge_eth_interface(ifp->name)==ETH_INTERFACE))*/
	{
   		ifp->if_types = REAL_INTERFACE;
   		ifp->devnum = 0;
   		ifp->slot = 0;
   		goto skip;
	}
 /*gjd : add for vrrp , downlink interface , when delete virtual ip , rpa interface if_type was set to real.*/
 if(judge_eth_interface(ifp->name)==ETH_INTERFACE)
 	{
 		int slot= 0;
   	/*	ifp->if_types = REAL_INTERFACE;*/
   		ifp->devnum = 0;
   		ifp->slot = slot;
		slot = get_slot_num(ifp->name);
		if(slot == product->board_id)	/*real*/
			ifp->if_types = REAL_INTERFACE;
		else							/*rpa*/
			ifp->if_types = RPA_INTERFACE;
		
   		goto skip;
	}
 /*2011-09-29: pm 6:25*/


 /* ret = search_ipv4(ifp);*/
  ret = ip_address_count(ifp);
  if(tipc_client_debug)
  zlog_debug("%s : line %d, there is %d ip address", __func__,__LINE__,ret);
  if(ret < 1)
   {
	zlog_debug("%s : line %d , there is no ip ,so set interafce is VIRTUAL_INTERFACE \n",__func__,__LINE__);
//	ifp->if_types = VIRTUAL_INTERFACE;
	return NULL;
  }   	
  else if (ret == 1)
  {
	  for (node = listhead (ifp->connected); node; node = next)/**from a interface of connect ip list search a ip**/
      {
        ifc = listgetdata (node);
        next = node->next;
      
        if (connected_same_prefix (ifc->address, p))/**check is the dest ip?**/
        {
          if(ifc->address->family == AF_INET)/*ipv4*/
	       {
	            ret = if_unset_prefix (ifc->ifp, ifc);
	        	if (ret < 0) 
				{
					
					zlog_debug("%s, line %d, kernel uninstall ipv6 address failed", __func__, __LINE__);
	        		return NULL;
	        	}
	       	}
		  else if(ifc->address->family == AF_INET6)/*ipv6*/
		  	{
		 		ret = if_prefix_delete_ipv6(ifc->ifp, ifc);
	     		if (ret < 0) 
				{
	     			zlog_debug("%s, line %d, kernel uninstall ipv6 address failed", __func__, __LINE__);
	     			return NULL;
	     	  	}
			}
		   else/*err : other*/
		 	{
		 		zlog_err("%s : line %d , unkown protocol %d .\n",__func__,__LINE__,p->family);
				return NULL;
		 	}
		   
           listnode_delete (ifp->connected, ifc);
		   #if 1
	 	 if(ifp->if_types == REAL_INTERFACE )
		  {
			 if((strncmp(ifp->name, "ve",2) != 0 && strncmp(ifp->name, "obc",3)!= 0))
		  	{
			unregister_rpa_interface_table(ifp);
/*        ifp->if_types = 0;*/
		  	}
	      }
      	if(ifp->if_types == RPA_INTERFACE)
		  {
		  	SET_FLAG(ifp->ip_flags,IPV4_ADDR_DISTRIBUTE_DEL);
			delete_rpa_interface(ifp);
		/*	master_redistribute_interface_delete(ifp);*/
		/*	master_redistribute_interface_delete_loacal(ifp);*//**地址删除时引起rpa口的删除，不能发送到板间，只发送给自己的进程**/
	      }
		#endif
            return ifc;
        }		
      } 
	  		
 	  /*PRA interface turned to virtual interface*/
	  /*delte_rpa_interface()*/
 //	  ifp->if_types = VIRTUAL_INTERFACE;	
  }
  else if (ret > 1)	
  {	
     /* the case of multiple address */
     for (node = listhead (ifp->connected); node; node = next)
     {
       ifc = listgetdata (node);
       next = node->next;
     
       if (connected_same_prefix (ifc->address, p))
       {
		   if(ifc->address->family == AF_INET)/*ipv4*/
		  {
			   ret = if_unset_prefix (ifc->ifp, ifc);
			   if (ret < 0) 
			   {
				   
				   zlog_debug("%s, line %d, kernel uninstall ipv6 address failed", __func__, __LINE__);
				   return NULL;
			   }
		   }
		  else if(ifc->address->family == AF_INET6)/*ipv6*/
		   {
			   ret = if_prefix_delete_ipv6(ifc->ifp, ifc);
			   if (ret < 0) 
			   {
				   zlog_debug("%s, line %d, kernel uninstall ipv6 address failed", __func__, __LINE__);
				   return NULL;
			   }
		   }
		   else/*err : other*/
		   {
			   zlog_err("%s : line %d , unkown protocol %d .\n",__func__,__LINE__,p->family);
			   return NULL;
		   }
         listnode_delete (ifp->connected, ifc);
         return ifc;
       }
     }
  }
  
  skip:

  for (node = listhead (ifp->connected); node; node = next)
     {
       ifc = listgetdata (node);
       next = node->next;
     
       if (connected_same_prefix (ifc->address, p))
       {
	     #if 1  /*delete address on rpa interface*/ 
		 
		 if(p->family == AF_INET)/*ipv4*/
		 {
	       	ret = if_unset_prefix (ifc->ifp, ifc);
	     	if (ret < 0) 
			{
	     		zlog_debug("%s, line %d, kernel uninstall ipv4 address failed", __func__, __LINE__);
	     		return NULL;
	     	}
		}
		 else if(p->family== AF_INET6 )/*ipv6*/
		 {
		 	ret = if_prefix_delete_ipv6(ifc->ifp, ifc);
	     	if (ret < 0) 
			{
	     		zlog_debug("%s, line %d, kernel uninstall ipv6 address failed", __func__, __LINE__);
	     		return NULL;
	     	  }
			}
		 else/*err : other*/
		 {
		 		zlog_err("%s : line %d , unkown protocol %d .\n",__func__,__LINE__,p->family);
				return NULL;
		 	}
	     #endif	
         listnode_delete (ifp->connected, ifc);
         return ifc;
       }
     }
  
  return NULL;
}



struct connected *
tipc_master_zebra_interface_address_read (int type, struct stream *s)
{	
  unsigned int ifindex;
  struct interface *ifp;
  struct connected *ifc;
  struct prefix p, d;
  char ifname_tmp[INTERFACE_NAMSIZ];
  int family;
  int plen;
  u_char ifc_flags;
  u_char ifc_conf;
  char buf[BUFSIZ];

  int ret;

  memset (&p, 0, sizeof(p));
  memset (&d, 0, sizeof(d));
  	
  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  if (judge_real_interface(ifname_tmp)){
  	if(tipc_client_debug)
     zlog_debug("%s, line == %d it's local_board REAl_INTERFACE return wrong!\n",__func__, __LINE__);
     return NULL;
  }
  

  /* Lookup this by interface name. */
  ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  /* If such interface does not exist, indicate an error */
  if (ifp == NULL) {
  	if(tipc_client_debug)
	 zlog_debug(" interface do not exist or the tipc interface not match!\n");
     return NULL;
  	}
  #if 0
  ifp->slot = stream_getl(s);
  ifp->devnum = stream_getl(s);
  if(tipc_client_debug)
	 zlog_debug(" %s : line %d , when add/del ip recv rpa interface: slot %d , devnum %d .\n",__func__,__LINE__,ifp->slot,ifp->devnum);
#endif
/**--gjd: fetch packet with conf for  ip --**/
ifc_conf = stream_getc(s);
/**-------------------------------------**/

  /* Fetch flag. */
  ifc_flags = stream_getc (s);
  ifp->if_scope = stream_getc(s);

  /* Fetch interface address. */
  family = p.family = stream_getc (s);

  if (family == AF_INET) {
  	if(tipc_client_debug)
		zlog_debug(" %s, line == %d this is ipv4 message\n",__func__, __LINE__);
  } else {
    if(tipc_client_debug)
		zlog_debug(" %s, line == %d ,this is ipv6 message, so return\n",__func__, __LINE__);
/*		return NULL;*/
  		 }
  
  plen = prefix_blen (&p);
  stream_get (&p.u.prefix, s, plen);
  p.prefixlen = stream_getc (s);
  
  #if 1
	ifp->slot = stream_getl(s);
	ifp->devnum = stream_getl(s);
	if(tipc_client_debug)
	   zlog_debug(" %s : line %d , when add/del ip recv rpa interface: slot %d , devnum %d .\n",__func__,__LINE__,ifp->slot,ifp->devnum);
#endif

  /* Fetch destination address. */
  stream_get (&d.u.prefix, s, plen);
  d.family = family;
  
 // if(tipc_client_debug)
	 zlog_debug ("MASTER recv[%s] interface %s : ip address %s/%d ",zserv_command_string(type),ifp->name,inet_ntop (p.family, &p.u.prefix, buf, BUFSIZ), p.prefixlen);

#if 0
/*-----------------------------------------------------------------------------------*/
 /* ret = search_ipv4(ifp);*/
  ret = ip_address_count(ifp);
  if (ret < 1 && ifp->ifindex == IFINDEX_INTERNAL) 
  	{
  		ifp->if_types = VIRTUAL_INTERFACE;
  }
  if(tipc_client_debug)
    zlog_debug("%s : line %d, ifp->if_types = %d",__func__,__LINE__,ifp->if_types);
  /*-----------------------------------------------------------------------------------*/
#endif
  if (type == ZEBRA_INTERFACE_ADDRESS_ADD) 
    {
       /* N.B. NULL destination pointers are encoded as all zeroes */
       ifc = tipc_master_connected_add_by_prefix(ifp, &p,(memconstant(&d.u.prefix,0,plen) ?
					      NULL : &d));
       if (ifc != NULL)
       {
       	 ifc->conf = ifc_conf;/**--use conf--**/
         ifc->flags = ifc_flags;
       	}
    }
  else if(type == ZEBRA_INTERFACE_ADDRESS_DELETE)
    {
      ifc = tipc_master_connected_delete_by_prefix(ifp, &p);
    }
  else
  	{
	return NULL;
  }

  return ifc;
 }

int
tipc_master_interface_address_add (int command, tipc_client *master_board,
			   zebra_size_t length)
{
  struct connected *ifc;
  int ret;
  struct prefix *p;
  ifc = tipc_master_zebra_interface_address_read (ZEBRA_INTERFACE_ADDRESS_ADD, 
                                      master_board->ibuf);
  if (ifc == NULL){
  	if(tipc_client_debug)
   		zlog_debug(" %s ,line = %d receive not match message  ifc == NULL\n", __func__, __LINE__);
    	return -1;
   	}
 
/*  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	  SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
*/

  
#if 0
  ret = if_set_prefix (ifc->ifp, ifc);
  if (ret < 0) {
		zlog_debug("%s, line == %d, if_set_prefix failed", __func__, __LINE__);
		return -1;
  }
#endif   
  
#if 1 
   if(ifc->address->family == AF_INET)
   {
	   ret = if_subnet_add (ifc->ifp, ifc);
	   if (ret < 0) 
	   	{
			zlog_debug("%s, line %d, ipv4 sub add address failed\n", __func__, __LINE__);
			return -1;
	   }
	}
 #endif

/*send message to all the vice and router deamon*/

  master_redistribute_interface_address_add (ifc->ifp, ifc);
#if 0
 /**add by gjd : for connect route 2011-04-15**/
  if (if_is_operative(ifc->ifp))
  {
	if(tipc_client_debug)
	 zlog_debug("%s : line %d, go to creat connect route ....\n",__func__,__LINE__);
	 connected_up_ipv4 (ifc->ifp, ifc);  
  }
#endif

  if(ifc->address->family == AF_INET)
  {
/**add  for connect route 2011-04-15**/
	if (if_is_operative(ifc->ifp))
	{
	  if(tipc_client_debug)
	   zlog_debug("%s : line %d, go to creat ipv4 connect route ....\n",__func__,__LINE__);
	  connected_up_ipv4 (ifc->ifp, ifc);  
	}
	  }
	  else
		 if(ifc->address->family == AF_INET6)
		  {
			  if(tipc_client_debug)
			   zlog_debug("%s : line %d, go to creat ipv6 connect route ....\n",__func__,__LINE__);
			  connected_up_ipv6 (ifc->ifp, ifc);  
			}
		  else
		  {
			  zlog_debug("XXXX err !\n");
		  }

  return 0;
}



int
tipc_master_interface_address_delete (int command, tipc_client *master_board,
			      zebra_size_t length)
{
  struct connected *ifc;
  int ret;
  
  ifc = tipc_master_zebra_interface_address_read (ZEBRA_INTERFACE_ADDRESS_DELETE,
                                      master_board->ibuf);
  if (ifc == NULL){
  	if(tipc_client_debug)
   		zlog_debug(" %s , line == %d,  tipc_interface_address_read  ifc == NULL\n", __func__, __LINE__);
    	return -1;
   	}

  if ( CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	  UNSET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);

#if 0
  ret = if_unset_prefix (ifc->ifp, ifc);
  if (ret < 0) {
		zlog_debug("%s, line == %d, if_unset_prefix failed", __func__, __LINE__);
		return -1;
  }
//  listnode_delete (ifc->ifp->connected, ifc);
#endif

  master_redistribute_interface_address_delete (ifc->ifp, ifc);

#if 1
  if(ifc->address->family == AF_INET)
  {
	ret = if_subnet_delete (ifc->ifp, ifc);
	if (ret < 0) {
	  if(tipc_client_debug)
		  zlog_debug("%s: line %d, if_subnet_delete failed", __func__, __LINE__);
		  return -1;  
	}
  }
#endif

  if(ifc->address->family == AF_INET)
	{
	   if(tipc_client_debug)
		zlog_debug("%s : line %d , go to del ipv4 connect route ......\n",__func__,__LINE__);
	   connected_down_ipv4 (ifc->ifp, ifc);
	  }
	else
	{
	   if(tipc_client_debug)
		zlog_debug("%s : line %d , go to del ipv6 connect route ......\n",__func__,__LINE__);
	   connected_down_ipv6(ifc->ifp, ifc);
	  }

  
  router_id_del_address(ifc);/*add*/
  listnode_delete (ifc->ifp->connected, ifc);/*add*/
  connected_free(ifc);
  return 0;
}




/* Inteface link up message processing */
int
tipc_master_interface_up (int command, tipc_client *master_board, zebra_size_t length)
{

  struct interface *ifp;
  int ret;
  
  if (master_board->ibuf == NULL) {
  	if(tipc_client_debug)
  	zlog_debug ("%s, line = %d, master_board->ibuf\n", __func__, __LINE__);
  	return -1;
  } 
      
  ifp = tipc_master_zebra_interface_state_read (ZEBRA_INTERFACE_UP,master_board->ibuf);

  return 0;
}




//Inteface link down message processing. 
int
tipc_master_interface_down (int command, tipc_client *master_board, zebra_size_t length)
{
  struct interface *ifp;
  int ret;
  
  if (master_board->ibuf == NULL) {
  	if(tipc_client_debug)
  	zlog_debug ("%s: line %d, master_board->ibuf == NULL\n", __func__, __LINE__);
  	return -1;
  }

  ifp = tipc_master_zebra_interface_state_read(ZEBRA_INTERFACE_DOWN,master_board->ibuf);

  return 0;
}






/*Inteface addition message from zebra.*/
int
tipc_master_interface_add (int command, tipc_client* master_board, int length)
{
  struct interface *ifp = NULL;
  //master_board->ibuf
  int ret;

  if (master_board->ibuf == NULL) {
  	if(tipc_client_debug)
	 zlog_debug("%s: line %d, master_board->ibuf == NULL\n", __func__, __LINE__);
	 return -1;
  }
  ifp = tipc_master_zebra_interface_add_read (master_board->ibuf);
#if 1  
  if (ifp == NULL){
  	if(tipc_client_debug)
   		zlog_debug("%s : line %d, the interface maybe add exist....\n", __func__, __LINE__);
    	return -1;
   	}
#endif
#if 1
  if(judge_eth_interface(ifp->name)==ETH_INTERFACE)
  	{
 	 if ((ifp->if_types == VIRTUAL_INTERFACE))
	  {	
		/*create_rpa_interface();*/
		/*ifp->slot = get_slot_num(ifp->name);*/
		ret = create_rpa_interface(ifp);
		 if(ret < 0)
		  {
		 	zlog_debug("%s : line %d, creat eth(cpu) rpa interface failed : %s \n ",__func__,__LINE__,safe_strerror(errno));
			return -1;
		  }
		 else
		  {	
			/*virtual interface ==> RPA interface*/
			ifp->if_types = RPA_INTERFACE;
		 /*	zlog_debug("%s : line %d, creat eth(cpu) rpa interface sucess, so virtual turn to rpa interface...\n",__func__,__LINE__);*/
		  }
		 
		 /*gujd : 2012-10-13 . Optimized code . Get index by netlink.*/
		 #if 0
		 sleep(1);
		 
		 ifp->ifindex = if_get_index_by_ioctl(ifp);
	  	 if(ifp->ifindex <= 0)
	  	  {
	  		zlog_warn("get eth(cpu) rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
			return -1;
	  	  }
	 	/* zlog_debug("get eth(cpu) rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);*/
		 #endif
		 master_redistribute_interface_add(ifp);
		 return 0;
		 
	  	}	
  
   if((ifp->if_types == REAL_INTERFACE || ifp->if_types == 0))
  	{
  		ret = register_rpa_interface_table(ifp);
		if(ret < 0)
	  	{
	  		zlog_warn("register eth(cpu) rpa interface table in local board failed : %s ..\n",safe_strerror(errno));
	  	}
		zlog_debug(" get eth(cpu) rpa slot %d ,devnum %d ...\n",ifp->slot,ifp->devnum);
		
  	}
  	}
#endif
else if(judge_ve_interface(ifp->name)== VE_INTERFACE)/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
 {
 	int slot = 0;
	slot = get_slot_num(ifp->name);
	if(product->board_id != slot)/*not local , creat rpa*/
	 {
	 	if(ifp->ifindex == IFINDEX_INTERNAL)
	 	{
		 ifp = create_rpa_interface_and_get_ifindex(ifp);/**creat rpa and register**/
		 #if 0
		 if(ifp->ifindex == IFINDEX_INTERNAL)
		 {
			zlog_info("%s: creat ve(rpa: %s) interface failed\n",__func__,ifp->name);
		  }
		 else
		 #endif	
		 {
			zlog_info("%s: creat ve(rpa: %s) interface ok!\n",__func__,ifp->name);
		 	ifp->if_types = REAL_INTERFACE;
		  }
	    }
	}
	else/*local: make sure to add to RPA table.*/
	{
		ret = register_rpa_interface_table(ifp);
		if(ret < 0)
		   zlog_info("Make Sure have add [%s] to rpa table  .\n",ifp->name);
	}
  }
#if 0
else  if(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)
  	{
  		int slot_num = 0;
		slot_num = get_slot_num(ifp->name);/*local board : not to add, Let npd creat, rtmd recv kernel netlink mesg.*/
 // 		if((ifp->if_types == VIRTUAL_INTERFACE)&& (slot_num != product->board_id))/*gujd: 2012-03-13 , pm 5:40 . Add if_scope flags.*/
	    if((ifp->if_types == VIRTUAL_INTERFACE)&& (slot_num != product->board_id))/*gujd: 2012-03-13 , pm 5:40 . Add if_scope flags.*/
		{
  			zlog_debug("%s : line %d , vice or bakup goto creat ve sub (global) .",__func__,__LINE__);
			vconfig_create_ve_sub_interface(ifp);
			ifp->if_types = REAL_INTERFACE;
  		}
  	}
#endif
/*if(ifp->if_types ==VIRTUAL_INTERFACE )*//*gujd: 2012-06-06, am 11:03. Delete for syncing ve  parent or sub interface to other baords .*/
  master_send_vurtual_interface_add_to_vice(ifp);

  return 0;
}




int
tipc_master_interface_delete (int command, tipc_client *master_board,
		      zebra_size_t length)
{
  struct interface *ifp;
  struct stream *s = NULL;
  
  int ret;

  if (master_board->ibuf == NULL) {
  	if(tipc_client_debug)
	 zlog_debug("%s, line == %d, master_board->ibuf == NULL\n", __func__, __LINE__);
	 return -1;
  }

  s = master_board->ibuf;  
  
  /* zebra_interface_state_read() updates interface structure in iflist */

  ifp = tipc_master_zebra_interface_state_read(ZEBRA_INTERFACE_DELETE,s);
  
  if (ifp == NULL){
  	if(tipc_client_debug)
   		zlog_debug(" %s ,line = %d  the interface not exist\n", __func__, __LINE__);
    	return -1;
   	}

#if 0
  /* router process, waiting for joint debugging*/
  if (if_is_up (ifp)) {
    if_down(ifp);
  } 
#endif
  if(tipc_client_debug)
  zlog_debug("interface delete %s  flags %ld metric %d mtu %d\n",
	    ifp->name,  ifp->flags, ifp->metric, ifp->mtu);  
if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE) 
   ||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE) )
 /*  goto skip;*/
 {
   int slot = 0;
   slot = get_slot_num(ifp->name);
   if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
   {
   	  
	  if(product->board_id == slot)/*local board*/
	  {
	  	zlog_err("%s : line %d , Myself local board(slot %d) interface %s, so not delete .\n",__func__,__LINE__,slot,ifp->name);
		return 0 ;
	  }
	  else/* not loacal board*/
	  	{
		  zlog_err("%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
		  
		  char cmdstr[64] = {0};
		  memset(cmdstr, 0 , 64);

		  sprintf(cmdstr,"vconfig rem %s",ifp->name);
		  system(cmdstr);/*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
		  return 0;
	  	}
   		
   }
   #if 0 /*gujd: 2012-06-06, am 11:03. Delete for ve sub interface by rtmd (instead of npd).*/
   else
   	{
   		if(slot != product->board_id )/*other board delete, local board let npd delete.*/
   		{
   			vconfig_delete_ve_sub_interface(ifp);
   		}
		
   	}
   #endif
   goto skip;
}

/*  if((((strncmp(ifp->name,"wlanl",5)!= 0)&&(strncmp(ifp->name,"wlan",4)==0))
	||((strncmp(ifp->name,"ebrl",4)!= 0)&&(strncmp(ifp->name,"ebr",3)==0)))
	 && (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))) 
*/
    if(((strncmp(ifp->name,"wlan",4)==0)||(strncmp(ifp->name,"ebr",3)==0))
     && (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))

  {
	int slot = 0;
	
	slot = get_slot_num(ifp->name);
	if(product->product_type == PRODUCT_TYPE_8610
		|| product->product_type == PRODUCT_TYPE_8606
		|| product->product_type == PRODUCT_TYPE_8800)/*86xx*/
	{
		if(product->board_id == slot || product->board_type == BOARD_IS_ACTIVE_MASTER)/*local board and active master*/
		{
		  zlog_info("(set local mode)%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
		  return 0 ;
		}
		else/* not local board*/
		{
			zlog_info("(set local mode)%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
		  
		   /*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			  ret = delete_rpa_interface(ifp);
			  if(ret < 0)
			  {
				zlog_debug("%s : line %d, delete rpa(%s) interface	failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
				return -1;
			   }
			  else
			  {
				zlog_debug("%s : line %d, delete rpa(%s) interface	sucess...\n",__func__,__LINE__,ifp->name);
				return 0;
				  }
		  }
		}
	else
	  if(product->product_type == PRODUCT_TYPE_7605I)
	  {
		if(product->board_id == slot )/*local board, different from 86: bakup master go to delete.*/
		{
		  zlog_info("(set local mode)%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
		  return 0 ;
		}
		else/* not local board*/
		  {
			zlog_info("(set local mode)%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
		  
		   /*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			  ret = delete_rpa_interface(ifp);
			  if(ret < 0)
			  {
				zlog_debug("%s : line %d, delete rpa(%s) interface	failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
				return -1;
			   }
			  else
			  {
				zlog_debug("%s : line %d, delete rpa(%s) interface	sucess...\n",__func__,__LINE__,ifp->name);
				return 0;
				  }
		  }
		}
	  else
	  {
		  zlog_warn("Unkown product type !\n");
		  return -1;
		  }
 }

   
  /*RPA interface*/
 if(ifp->if_types == RPA_INTERFACE)
  {
  	if(tipc_client_debug)
		zlog_debug("go to delete RPA interface\n");
/*	if(strncmp(ifp->name, "eth",3) != 0)*//*not eth*/
	{
		ret = delete_rpa_interface(ifp);
	 	if(ret < 0)
	 	  zlog_debug("%s : line %d, delete rpa interface (not cpu) failed !!!!!\n ",__func__,__LINE__);
	 	else
	 	  zlog_debug("%s : line %d, delete rpa interface (not cpu) sucess...\n",__func__,__LINE__);
		}
  	goto skip;
  } 
  if(ifp->if_types == REAL_INTERFACE)
  	{
  		if(tipc_client_debug)
			zlog_debug("unregister PRA interface table\n");
		if((strncmp(ifp->name, "ve",2) != 0 && strncmp(ifp->name, "obc",3)!= 0))
		{
			ret = unregister_rpa_interface_table(ifp);
		 	if(ret < 0)
		 	  zlog_debug("%s : line %d, unregister rpa interface (not cpu) failed !!!!!\n ",__func__,__LINE__);
		 	else
		 	  zlog_debug("%s : line %d, unregister rpa interface (not cpu) sucess...\n",__func__,__LINE__);
			}
  	}
skip:
		
  /*delete virtual interface*/
 /* if(ifp->if_types != VIRTUAL_INTERFACE)*/
	master_redistribute_interface_delete(ifp);

  if(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)/*gujd : 2012-05-04, pm 5:30. Add code for use netlink message to delete ve sub.*/
    return 0;
  
  if_delete(ifp);
  
    return 0;
}

struct interface *
interface_packets_statistics_read(struct stream *s, int *if_flow_command)
{
	struct interface *ifp;
	char ifname_tmp[INTERFACE_NAMSIZ];
	int ret = 0;
	
	if (s == NULL) 
	{
		zlog_debug("zebra_interface_state struct stream is null\n");
		return NULL;
	}
	*if_flow_command = stream_getl(s);
  	/* Read interface name. */
  	stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);

  	/*check interface is exist ?*/
  	ifp = check_interface_exist_by_name_len(ifname_tmp,
				   strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  	/* If such interface does not exist, indicate an error */
  	if (ifp == NULL) 
	{
	  if(tipc_client_debug)
	 	zlog_debug(" interface do not exist or the tipc interface not match!\n");
	  return NULL;
	}
  
  /* Read interface's packets statistics info .*/
  	ifp->stats.rx_packets = stream_getq (s);
  	ifp->stats.tx_packets = stream_getq (s);
  	ifp->stats.rx_bytes = stream_getq (s);
  	ifp->stats.tx_bytes = stream_getq (s);
  	ifp->stats.rx_errors = stream_getq (s);
  	ifp->stats.tx_errors = stream_getq (s);
  	ifp->stats.rx_dropped = stream_getq (s);
 	ifp->stats.tx_dropped = stream_getq (s);
  	ifp->stats.rx_multicast = stream_getq (s);
  	ifp->stats.collisions = stream_getq (s);
  	ifp->stats.rx_length_errors = stream_getq (s);
  	ifp->stats.rx_over_errors = stream_getq (s);
  	ifp->stats.rx_crc_errors = stream_getq (s);
  	ifp->stats.rx_frame_errors = stream_getq (s);
	ifp->stats.rx_compressed = stream_getq(s);
  	ifp->stats.rx_fifo_errors = stream_getq (s);
 	ifp->stats.rx_missed_errors = stream_getq (s);
  	ifp->stats.tx_aborted_errors = stream_getq (s);
  	ifp->stats.tx_carrier_errors = stream_getq (s);
  	ifp->stats.tx_fifo_errors = stream_getq (s);
  	ifp->stats.tx_heartbeat_errors = stream_getq (s);
  	ifp->stats.tx_window_errors = stream_getq (s);
#if 0	
#ifdef HAVE_PROC_NET_DEV
	  /* Statistics print out using proc file system. */
	  zlog_info ("%s:	 %llu input packets (%llu multicast), %llu bytes, "
		   "%llu dropped.\n",
		   ifp->name, ifp->stats.rx_packets, ifp->stats.rx_multicast,
		   ifp->stats.rx_bytes, ifp->stats.rx_dropped);
	
	  zlog_info ("	 %llu input errors, %llu length, %llu overrun,"
		   " %llu CRC, %llu frame.\n",
		   ifp->stats.rx_errors, ifp->stats.rx_length_errors,
		   ifp->stats.rx_over_errors, ifp->stats.rx_crc_errors,
		   ifp->stats.rx_frame_errors);
	
	  zlog_info ("	 %llu fifo, %llu missed.\n", ifp->stats.rx_fifo_errors,
		   ifp->stats.rx_missed_errors);
	
	  zlog_info ("	 %llu output packets, %llu bytes, %llu dropped.\n",
		   ifp->stats.tx_packets, ifp->stats.tx_bytes,
		   ifp->stats.tx_dropped);
	
	  zlog_info ("	 %llu output errors, %llu aborted, %llu carrier,"
		   " %llu fifo, %llu heartbeat.\n",
		   ifp->stats.tx_errors, ifp->stats.tx_aborted_errors,
		   ifp->stats.tx_carrier_errors, ifp->stats.tx_fifo_errors,
		   ifp->stats.tx_heartbeat_errors);
	
	  zlog_info ("	 %llu window, %llu collisions.\n",
		   ifp->stats.tx_window_errors, ifp->stats.collisions);
#endif /* HAVE_PROC_NET_DEV */
#endif
	
  return ifp;
}



/*Inteface addition message from zebra.*/
int
tipc_master_interface_packets_statistics (int command, tipc_client* master_board, int length)
{
  struct interface *ifp = NULL;
  int if_flow_command = 0;

  if(master_board->ibuf == NULL) 
  {
	 zlog_debug("%s: line %d, master_board->ibuf == NULL\n", __func__, __LINE__);
	 return -1;
  }
  ifp = interface_packets_statistics_read (master_board->ibuf,&if_flow_command);/*read data*/
  
  if (ifp == NULL){
  	if(tipc_client_debug)
   		zlog_debug("%s : line %d, the interface maybe not exist....\n", __func__, __LINE__);
    	return -1;
   	}
  
  /*use if_flow_command*/
  if(rtm_debug_if_flow)
	  zlog_debug("%s: line %d, command[%d](%s).\n",__func__,__LINE__,if_flow_command,
			  zserv_command_string(if_flow_command));
  /*send to snmp for integrate sampling.*/
  rtm_if_flow_stats_update_for_sampling_integrated(ifp,if_flow_command);/*one by one. Or we can use all update finish, then send*/

  return 0;
}

/*master all interface send to vice*/
int
tipc_master_interface_infomastion_request(int command, tipc_client* master_board, int length)
{
	if(command != ZEBRA_INTERFACE_INFOMATION_REQUEST || !master_board)
	{
		zlog_warn("Error command(%d) or struct master board(%p) .\n",command,master_board);
		return -1;
		}
	/*sleep(1);*/
  	usleep(fetch_rand_time_interval());
	master_interface_send_to_vice(master_board);
	return 0;
}

/*master all route send to vice.*/
int
tipc_master_route_infomation_request(int command, tipc_client* master_board, int length)
{
	if(command != ZEBRA_ROUTE_INFOMATION_REQUEST || !master_board)
	{
		zlog_warn("Error command(%d) or struct master board(%p) .\n",command,master_board);
		return -1;
		}
	/*sleep(1);*/
  	usleep(fetch_rand_time_interval());
	tipc_server_to_client_redistribute(master_board);
	return 0;
}


static int
master_board_flush_data(struct thread *thread)
{
  tipc_client *master_board = THREAD_ARG(thread);

  master_board->t_write = NULL;
  if (master_board->sock < 0)
    return -1;
  switch (buffer_flush_available(master_board->wb, master_board->sock))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_flush_available failed on zclient fd %d, closing",
      		__func__, master_board->sock);
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      return master_board_failed(master_board);
      break;
    case BUFFER_PENDING:
      master_board->t_write = thread_add_write(zebrad.master, master_board_flush_data,
					  master_board, master_board->sock);
      break;
    case BUFFER_EMPTY:
      break;
    }
  return 0;
}


/**send message**/
int
master_send_message_to_vice (tipc_client  *master_board)
{
	if(tipc_client_debug)
		zlog_debug("enter func %s .....\n",__func__);
  if (master_board->sock < 0)
  	{
  		zlog_info("%s : fd < 0 , cannot to send message .\n",__func__);
    	return -1;
  	}
  switch (buffer_write(master_board->wb, master_board->sock, STREAM_DATA(master_board->obuf),
		       stream_get_endp(master_board->obuf)))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_write failed to zclient fd %d, closing",
      		 __func__, master_board->sock);
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      return master_board_failed(master_board);
      break;
    case BUFFER_EMPTY:
      THREAD_OFF(master_board->t_write);
      break;
    case BUFFER_PENDING:
      THREAD_WRITE_ON(zebrad.master, master_board->t_write,
		      master_board_flush_data, master_board, master_board->sock);
      break;
    }
  if(tipc_client_debug)
  	zlog_debug("leave func %s ...\n",__func__);
  return 0;
}



/* Stop zebra client services. */
void
master_board_stop (tipc_client  *master_board)
{
  #ifdef ZCLIENT_DEBUG
    zlog_debug ("master_board stopped");
  #endif
  /* Stop threads. */
  THREAD_OFF(master_board->t_read);
  THREAD_OFF(master_board->t_connect);
  THREAD_OFF(master_board->t_write);

  /* Reset streams. */
  stream_reset(master_board->ibuf);
  stream_reset(master_board->obuf);

  /* Empty the write buffer. */
  buffer_reset(master_board->wb);

  /* Close socket. */
  if (master_board->sock >= 0)
    {
      close (master_board->sock);
      master_board->sock = -1;
    }
  master_board->fail = 0;
}


int
master_board_failed(tipc_client  *master_board)
{
	/*if(tipc_client_debug)
		zlog_debug("enter func %s ...\n",__func__);*/
	
#if 1
  zlog_warn("%s: socket[%d] reading cased failed ,to close tipc client.\n",__func__,master_board->sock);
  master_board->fail++;
  master_board_stop(master_board);
  master_board_event(TIPC_CLIENT_CONNECT, master_board);
#else
  zlog_warn("%s: socket[%d] reading cased failed ,to reset ibuf.\n",__func__,master_board->sock);
  master_board->fail = 0;
  stream_reset(master_board->ibuf);
  master_board_event (TIPC_CLIENT_READ, master_board);
 #endif
 
 /* if(tipc_client_debug)
	  zlog_debug("leave func %s ...\n",__func__);*/
  return -1;
}
int
master_board_send_request_to_vice(int command, tipc_client *client,int if_flow_command)
{
  struct stream *s;
  u_int16_t value = 1;

  if(!product || product->board_type != BOARD_IS_ACTIVE_MASTER)
  	{
  		zlog_warn("Product is NULL or Board is not active master !\n");
		return -1;
  	}
  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, command);
 /* 
  if(ifp)
  	stream_put(s, ifp->name, INTERFACE_NAMSIZ);
*/
  /*Put a word(8 byte) in packet in order to make packet >= 6 byte, in fact, this value unuseful . Only use it for counter(tipc-config -pi) .*/
//  stream_putw(s,value);
   stream_putl(s,if_flow_command);
 
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
}

int
master_board_send_interface_infomation_request_to_vice(struct thread *thread)
{
  struct stream *s = NULL;
  tipc_client *client = NULL;
  int ret = 0;
  
  /*when rtmd -k .means rtsuit restart ,keep_kernel_mode = 1, else keep_kernel_mode = 0(first start).*/
  int restart_flag = keep_kernel_mode;
  

  if(!product || product->board_type != BOARD_IS_ACTIVE_MASTER)
  	{
  		zlog_warn("Product is NULL or Board is not active master !\n");
		return -1;
  	}
  
  /* Get thread data.  Reset reading thread because I'm running. */
  client = THREAD_ARG (thread);
  
  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, ZEBRA_INTERFACE_INFOMATION_REQUEST);

  stream_putl(s,restart_flag);
 
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));
  /*sleep(1);*/  
 /* usleep(fetch_rand_time_interval());*/
  
  ret = master_send_message_to_vice(client);
  if(ret < 0 )/*fd not connect*/
  	{
  		zlog_info("%s: tipc fd not connect , to set timer(1s) to retry [interface request].\n",__func__);
		thread_add_timer(zebrad.master,master_board_send_interface_infomation_request_to_vice,client,1);
		return -1;
  	}
  
   return 0;

  
}
#if 0
int
master_board_send_interface_infomation_request_to_vice(int command, tipc_client *client)
{
  struct stream *s;
  
  /*when rtmd -k .means rtsuit restart ,keep_kernel_mode = 1, else keep_kernel_mode = 0(first start).*/
  int restart_flag = keep_kernel_mode;
  

  if(!product || product->board_type != BOARD_IS_ACTIVE_MASTER)
  	{
  		zlog_warn("Product is NULL or Board is not active master !\n");
		return -1;
  	}
  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, command);

  stream_putl(s,restart_flag);
 
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));
  /*sleep(1);*/  
  usleep(fetch_rand_time_interval());
  
  return master_send_message_to_vice(client);
}
#endif

/*int
master_board_request_interface_packets_statistics(struct thread *thread)
*/
int
master_board_request_interface_packets_statistics(int if_flow_command)
{
	/*  tipc_client  *master_board = THREAD_ARG(thread);*/
	  tipc_client  *master_board ;
	  struct listnode *ifnode, *ifnnode;
	  struct listnode *node, *nnode;
	  struct listnode *cnode, *cnnode;
	  struct interface *ifp;
	  struct connected *c;
	  int ret = 0;

	  if(tipc_client_debug)
		zlog_debug("enter func	%s ...\n",__func__);
	  
	  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
		return -1;
	  
	  if(zebrad.vice_board_list == NULL)
	  {
	  	if(tipc_client_debug)
			zlog_debug("%s, line = %d :tipc_client_list is NULL....", __func__, __LINE__);
		return -1;
	  }

	  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
	  {
/*	  
		zlog_debug("%s : line %d , Active master send request interface packets statistics to other boards .\n",__func__,__LINE__);
*/
		//usleep(fetch_rand_time_interval());
		master_board_send_request_to_vice(ZEBRA_INTERFACE_PACKETS_REQUEST_ALL,master_board,if_flow_command);
		
	  	}
	 /* master_board_event(TIPC_PACKETS_STATISTICS_TIMER, master_board);*/
	 /* master_board_event(TIPC_PACKETS_STATISTICS_TIMER, NULL);*/
	  if(tipc_client_debug)
		zlog_debug("leave func	%s ...\n",__func__);
	  
	  return 0;
	  
}


/**when connected break, this func to reset connect.
it happens like : when vice board zebra kill, 
master board zebra wait for reconnect viace board.
Or say : this func support the vice board hot plug**/
int
master_reset_connect_vice(struct thread *thread)
{
	tipc_client *master_board;
	int ins = 0;/**********with board_id*******************/

	master_board = THREAD_ARG (thread);
#if 1
	master_board->t_connect = NULL;
#else
	if(master_board->t_connect)
	{
		thread_cancel(master_board->t_connect);
		master_board->t_connect=NULL;
	}
#endif
	
	master_board->sock = -1;
    ins =1000 + master_board->board_id;

	
	/*creat socket*/
	master_board->sock = tipc_client_socket(ins);

	if (tipc_client_debug)
	  zlog_debug ("%s: line %d, reset event,TIPC_CLIENT_SCHEDULE....\n",__func__,__LINE__);
/*	master_board_event(TIPC_CLIENT_SCHEDULE, NULL);
	if (tipc_client_debug)
		zlog_debug ("%s: line %d, reset event,TIPC_CLIENT_SCHEDULE  OK..\n",__func__,__LINE__);*/
	
	if (master_board->sock < 0)
    {
		zlog_debug ("master connection vice fail\n");
      master_board->fail++;
	  if(master_board->fail > TIPC_CONNECT_TIMES)/*from 12 times to 120 times*/
	  	return -2;
      master_board_event (TIPC_CLIENT_CONNECT, master_board);
      return -1;
    }
/*gujd : 2012-10-17 . Add set tipc socket nonblock.*/	
#if 1
  if (set_nonblocking(master_board->sock) < 0)
  	{
	  zlog_warn("%s: set_nonblocking(%d) failed[%s]", 
		  __func__, master_board->sock,safe_strerror(errno));
		return -3;

  }
#endif

  /* Clear fail count. */
  master_board->fail = 0;
  if (tipc_client_debug)
    zlog_debug ("master connect vice success with socket [%d]\n", master_board->sock);
	
	master_board_event(TIPC_CLIENT_READ,master_board);

	/**send master board all interfaces info to vice board**/
 /* master_interface_send_to_vice(master_board);*/
#if 0
	master_board_send_interface_infomation_request_to_vice(ZEBRA_INTERFACE_INFOMATION_REQUEST,master_board);
#else
	thread_add_timer(zebrad.master,master_board_send_interface_infomation_request_to_vice,master_board,1);
#endif

	/**master board send all route message to vice board **/	
	/**we use master board initiative send all route to vice board, not use vice send command to master**/
	/*tipc_redistribute_all(master_board);*/
	
	/*master_board_event(TIPC_PACKETS_STATISTICS_TIMER, master_board);*/

	/*CID 11366 (#1 of 1): Missing return statement (MISSING_RETURN)
	5. missing_return: Arriving at the end of a function without returning a value.
	Add a return value.*/
	return 0;/*add*/

		
	
}

#if 0
/**add by gjd: Zebra tipc client recv stream buf from server,and parese header of buf */
int
master_board_read (struct thread *thread)
{
  int ret;
  size_t already = 0;
  uint16_t length , command;
  uint8_t marker, version;
  tipc_client *master_board;

  if (tipc_client_debug)
	   zlog_debug ("enter %s ......\n", __func__);

  /* Get socket to zebra. */
  master_board= THREAD_ARG (thread);
  master_board->t_read = NULL;
  
  ssize_t nbyte = 0;
 if (((nbyte = stream_read_try(master_board->ibuf, master_board->sock,4096)) == 0) || (nbyte == -1))/******use size = 4096******/
	{
	/*  if (tipc_client_debug)
	   zlog_debug ("zclient connection closed socket [%d].\n", master_board->sock);*/
	  zlog_warn("Lose connecttion of socket[%d].\n",master_board->sock);
	
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
	  return master_board_failed(master_board);
	}
	
  /* Fetch header values. 6 byte*/
  length = stream_getw (master_board->ibuf);
  marker = stream_getc (master_board->ibuf);
  version = stream_getc (master_board->ibuf);
  command = stream_getw (master_board->ibuf);
  if (tipc_client_debug)
	zlog_debug ("%s: line %d, ok fetch header and packet length = %d, marker = %u, version = %u, command [%s]..\n",
				__func__,__LINE__,length,marker,version,zserv_command_string(command));
  
  if (marker != ZEBRA_HEADER_MARKER || version != ZSERV_VERSION)
    {
      zlog_err("%s: socket %d version mismatch, marker %d, version %d",
               __func__, master_board->sock, marker, version);
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      return master_board_failed(master_board);
    }
  
  if (length < ZEBRA_HEADER_SIZE) 
    {
      zlog_err("%s: socket %d message length %u is less than %d ",
	       __func__, master_board->sock, length, ZEBRA_HEADER_SIZE);
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      return master_board_failed(master_board);
    }

  /* Length check. */
  if (length > STREAM_SIZE(master_board->ibuf))
    {
      struct stream *ns;
      zlog_warn("%s: message size %u exceeds buffer size %lu, expanding...",
	        __func__, length, (u_long)STREAM_SIZE(master_board->ibuf));
      ns = stream_new(length);
      stream_copy(ns, master_board->ibuf);
      stream_free (master_board->ibuf);
      master_board->ibuf = ns;
    }

  if (tipc_client_debug)
   zlog_debug ("%s: line %d, rtm message received [%s] %d\n", __func__,__LINE__,zserv_command_string (command), length);

  switch (command)
    {
    case ZEBRA_ROUTER_ID_UPDATE:
 //     if (master_board->router_id_update)
//	ret = (*master_board->router_id_update) (command, master_board, length);
      break;
    case ZEBRA_INTERFACE_ADD:
  //    if (master_board->interface_add)
//	ret = (*master_board->interface_add) (command, master_board, length);
	tipc_master_interface_add (ZEBRA_INTERFACE_ADD, master_board, length);
      break;
    case ZEBRA_INTERFACE_DELETE:
//      if (master_board->interface_delete)
//	ret = (*master_board->interface_delete) (command, master_board, length);
	tipc_master_interface_delete (ZEBRA_INTERFACE_DELETE, master_board,length);
				
      break;
    case ZEBRA_INTERFACE_ADDRESS_ADD:
//      if (master_board->interface_address_add)
//	ret = (*master_board->interface_address_add) (command, master_board, length);
   tipc_master_interface_address_add (ZEBRA_INTERFACE_ADDRESS_ADD ,master_board,  length);

      break;
    case ZEBRA_INTERFACE_ADDRESS_DELETE:
//      if (master_board->interface_address_delete)
//	ret = (*master_board->interface_address_delete) (command, master_board, length);
    tipc_master_interface_address_delete (ZEBRA_INTERFACE_ADDRESS_DELETE, master_board, length);

      break;
    case ZEBRA_INTERFACE_UP:
//      if (master_board->interface_up)
//	ret = (*master_board->interface_up) (command, master_board, length);
	  
	tipc_master_interface_up (ZEBRA_INTERFACE_UP, master_board, length);
      break;
    case ZEBRA_INTERFACE_DOWN:
//      if (master_board->interface_down)
//	ret = (*master_board->interface_down) (command, master_board, length);
	tipc_master_interface_down (ZEBRA_INTERFACE_DOWN, master_board, length);

      break;
    case ZEBRA_IPV4_ROUTE_ADD:
//      if (master_board->ipv4_route_add)
//	ret = (*master_board->ipv4_route_add) (command, master_board, length);
      break;
    case ZEBRA_IPV4_ROUTE_DELETE:
//      if (master_board->ipv4_route_delete)
//	ret = (*master_board->ipv4_route_delete) (command, master_board, length);
      break;
    case ZEBRA_IPV6_ROUTE_ADD:
 //     if (master_board->ipv6_route_add)
//	ret = (*master_board->ipv6_route_add) (command, master_board, length);
      break;
    case ZEBRA_IPV6_ROUTE_DELETE:
//      if (master_board->ipv6_route_delete)
//	ret = (*master_board->ipv6_route_delete) (command, master_board, length);
      break;
	  
	case ZEBRA_REDISTRIBUTE_ALL_INTERFACE:
//	  if (master_board->redistribute_interface_all)
//	ret = (*master_board->redistribute_interface_all) (command, master_board, length);
      break;
	  
	case ZEBRA_REDISTRIBUTE_ALL_ROUTE:
//	  if(*master_board->redistribute_route_all)
//	ret = (*master_board->redistribute_route_all) (command, master_board, length);
	tipc_redistribute_all(master_board);
	  break;
	  /**2011-02**/

	case ZEBRA_INTERFACE_PACKETS_STATISTICS:
		tipc_master_interface_packets_statistics(ZEBRA_INTERFACE_PACKETS_STATISTICS, master_board,length);
		break;

	case ZEBRA_INTERFACE_INFOMATION_REQUEST:
		tipc_master_interface_infomastion_request(ZEBRA_INTERFACE_INFOMATION_REQUEST,master_board,length);
		break;

	case ZEBRA_ROUTE_INFOMATION_REQUEST:
		tipc_master_route_infomation_request(ZEBRA_ROUTE_INFOMATION_REQUEST,master_board,length);
		break;	
		
	  
    default:
      break;
    }
  
  if (master_board->sock < 0)
    /* Connection was closed during packet processing. */
    return -1;

  /* Register read thread. */
  stream_reset(master_board->ibuf);
  master_board_event (TIPC_CLIENT_READ, master_board);

  if(tipc_client_debug)
  	zlog_debug("leave func %s...\n",__func__);

  return 0;
}
#else

int
master_board_read(struct thread *thread)
{
  int ret;
  size_t already = 0;
  uint16_t length , command;
  uint8_t marker, version;
  tipc_client *master_board;

  if (tipc_client_debug)
	   zlog_debug ("enter %s ......\n", __func__);

  /* Get socket to zebra. */
  master_board= THREAD_ARG (thread);
  master_board->t_read = NULL;
  
  ssize_t nbyte = 0;


  if ((already = stream_get_endp(master_board->ibuf)) < TIPC_PACKET_HEADER_SIZE)
	  {
	  
		ssize_t nbyte;
		if (((nbyte = stream_read_try(master_board->ibuf, master_board->sock,
					   TIPC_PACKET_HEADER_SIZE-already)) == 0) ||
		(nbyte == -1))
	  {
	  	zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																	master_board->sock,safe_strerror(errno));
		return master_board_failed(master_board);
	  }
		if (nbyte != (ssize_t)(TIPC_PACKET_HEADER_SIZE-already))
	  {
		/* Try again later. */
		master_board_event(TIPC_CLIENT_READ, master_board);
		return 0;
	  }
		already = TIPC_PACKET_HEADER_SIZE;
	  }
  
	/* Reset to read from the beginning of the incoming packet. */
	stream_set_getp(master_board->ibuf, 0);

  /* Fetch header values. 6 byte*/
  length = stream_getw (master_board->ibuf);
  marker = stream_getc (master_board->ibuf);
  version = stream_getc (master_board->ibuf);
  command = stream_getw (master_board->ibuf);
  
  if (tipc_client_debug)
	zlog_debug ("%s: line %d, ok fetch header and packet length = %d, marker = %u, version = %u, command [%s]..\n",
				__func__,__LINE__,length,marker,version,zserv_command_string(command));

  if (marker != TIPC_PACKET_HEADER_MARKER || version != TIPC_PACKET_VERSION)
    {
      /*zlog_err("%s: socket %d version mismatch, marker %d, version %d",
               __func__, master_board->sock, marker, version);*/
               
	  zlog_err("%s: line %d,socket[%d] version[%d],market[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
													master_board->sock,version,marker,safe_strerror(errno));
      return master_board_failed(master_board);
 	
    }
  
  if (length < TIPC_PACKET_HEADER_SIZE) 
    {
      zlog_err("%s: socket %d message length %u is less than %d ",
	       __func__, master_board->sock, length, TIPC_PACKET_HEADER_SIZE);
	  
	  zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																  master_board->sock,safe_strerror(errno));
      return master_board_failed(master_board);
    }

  /* Length check. */
  if (length > STREAM_SIZE(master_board->ibuf))
    {
      struct stream *ns;
      zlog_warn("%s: message size %u exceeds buffer size %lu, expanding...",
	        __func__, length, (u_long)STREAM_SIZE(master_board->ibuf));
      ns = stream_new(length);
      stream_copy(ns, master_board->ibuf);
      stream_free (master_board->ibuf);
      master_board->ibuf = ns;
    }
  /* Read rest of zebra packet. */
  if (already < length)
  {
      ssize_t nbyte;
      if (((nbyte = stream_read_try(master_board->ibuf, master_board->sock,
				     length-already)) == 0) || (nbyte == -1))
	 {
	  zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																	  master_board->sock,safe_strerror(errno));
	  return master_board_failed(master_board);
	}
	  
      if (nbyte != (ssize_t)(length-already))
	 {
	  /* Try again later. */
	  master_board_event(TIPC_CLIENT_READ, master_board);
	  return 0;
	 }
  }

  length -= TIPC_PACKET_HEADER_SIZE;

  if (tipc_client_debug)
   zlog_debug ("%s: line %d, rtm message received [%s] %d\n", __func__,__LINE__,zserv_command_string (command), length);

  switch (command)
   {
   
   case ZEBRA_INTERFACE_PACKETS_STATISTICS:
	   tipc_master_interface_packets_statistics(ZEBRA_INTERFACE_PACKETS_STATISTICS, master_board,length);
	   break;
    case ZEBRA_INTERFACE_ADD:
  /*    if (master_board->interface_add)
	ret = (*master_board->interface_add) (command, master_board, length);*/
	tipc_master_interface_add (ZEBRA_INTERFACE_ADD, master_board, length);
      break;
    case ZEBRA_INTERFACE_DELETE:
/*      if (master_board->interface_delete)*/
/*	ret = (*master_board->interface_delete) (command, master_board, length);*/
	tipc_master_interface_delete (ZEBRA_INTERFACE_DELETE, master_board,length);
				
      break;
    case ZEBRA_INTERFACE_ADDRESS_ADD:
/*      if (master_board->interface_address_add)*/
/*	ret = (*master_board->interface_address_add) (command, master_board, length);*/
   tipc_master_interface_address_add (ZEBRA_INTERFACE_ADDRESS_ADD ,master_board,  length);

      break;
    case ZEBRA_INTERFACE_ADDRESS_DELETE:
/*      if (master_board->interface_address_delete)*/
/*	ret = (*master_board->interface_address_delete) (command, master_board, length);*/
    tipc_master_interface_address_delete (ZEBRA_INTERFACE_ADDRESS_DELETE, master_board, length);

      break;
    case ZEBRA_INTERFACE_UP:
/*      if (master_board->interface_up)*/
/*	ret = (*master_board->interface_up) (command, master_board, length);*/
	  
	tipc_master_interface_up (ZEBRA_INTERFACE_UP, master_board, length);
      break;
    case ZEBRA_INTERFACE_DOWN:
/*      if (master_board->interface_down)*/
/*	ret = (*master_board->interface_down) (command, master_board, length);*/
	tipc_master_interface_down (ZEBRA_INTERFACE_DOWN, master_board, length);

      break;
    case ZEBRA_IPV4_ROUTE_ADD:
/*      if (master_board->ipv4_route_add)*/
/*	ret = (*master_board->ipv4_route_add) (command, master_board, length);*/
      break;
    case ZEBRA_IPV4_ROUTE_DELETE:
/*      if (master_board->ipv4_route_delete)*/
/*	ret = (*master_board->ipv4_route_delete) (command, master_board, length);*/
      break;
    case ZEBRA_IPV6_ROUTE_ADD:
 /*     if (master_board->ipv6_route_add)*/
/*	ret = (*master_board->ipv6_route_add) (command, master_board, length);*/
      break;
    case ZEBRA_IPV6_ROUTE_DELETE:
/*      if (master_board->ipv6_route_delete)*/
/*	ret = (*master_board->ipv6_route_delete) (command, master_board, length);*/
      break;
	case ZEBRA_ROUTER_ID_UPDATE:
 /* 	if (master_board->router_id_update)*/
/*	ret = (*master_board->router_id_update) (command, master_board, length);*/
	  break;

	case ZEBRA_REDISTRIBUTE_ALL_INTERFACE:
/*	  if (master_board->redistribute_interface_all)*/
/*	ret = (*master_board->redistribute_interface_all) (command, master_board, length);*/
      break;
	  
	case ZEBRA_REDISTRIBUTE_ALL_ROUTE:
/*	  if(*master_board->redistribute_route_all)*/
/*	ret = (*master_board->redistribute_route_all) (command, master_board, length);*/
	tipc_redistribute_all(master_board);
	  break;
	  /**2011-02**/

	case ZEBRA_INTERFACE_INFOMATION_REQUEST:
		tipc_master_interface_infomastion_request(ZEBRA_INTERFACE_INFOMATION_REQUEST,master_board,length);

		break;

	case ZEBRA_ROUTE_INFOMATION_REQUEST:
		tipc_master_route_infomation_request(ZEBRA_ROUTE_INFOMATION_REQUEST,master_board,length);
		break;	
		
	case ZEBRA_INTERFACE_DESCRIPTION_SET:
	  	tipc_master_interface_description_set(ZEBRA_INTERFACE_DESCRIPTION_SET,master_board,length);
	  
	  break;
	  
	case ZEBRA_INTERFACE_DESCRIPTION_UNSET:
		tipc_master_interface_description_unset(ZEBRA_INTERFACE_DESCRIPTION_UNSET,master_board,length);
		
		break;
		
		
	  
    default:
      break;
    }
  
  if (master_board->sock < 0)
    /* Connection was closed during packet processing. */
    return -1;

  /* Register read thread. */
  stream_reset(master_board->ibuf);
  master_board_event (TIPC_CLIENT_READ, master_board);

  if(tipc_client_debug)
  	zlog_debug("leave func %s...\n",__func__);

  return 0;
}

#endif
	
void
master_board_event (enum tipc_client_events event,  tipc_client *master_board)
{
	if(tipc_client_debug)
		zlog_debug("enter func %s ....\n",__func__);
    switch (event)
	 {
	    case TIPC_CLIENT_SCHEDULE:
		  if(tipc_client_debug)
			zlog_debug("%s: line %d, master_board event = TIPC_CLIENT_SCHEDULE\n",__func__,__LINE__);
		  	thread_add_event (zebrad.master, master_reset_connect_vice, master_board, 0);
		  
	     	break;
			
	    case TIPC_CLIENT_CONNECT:

		#if 0
	      if (master_board->fail >= 15)
	      	{
	      		zlog_warn("Master reconnect Vice 15 times , but reconnect failed , so not reconnect !\n");
				return;
	      	}
		  if(tipc_client_debug)
			 zlog_debug ("master_board connect vice schedule interval is %d", master_board->fail < 10 ? 10 : 60);
	      if (! master_board->t_connect)
			master_board->t_connect = 
		  	thread_add_timer (zebrad.master, master_reset_connect_vice, master_board,
				    master_board->fail < 10 ? 10 : 60);
		  #else
		  
		  if (master_board->fail >= TIPC_CONNECT_TIMES)/*from 12 times to 120 times*/
	      	{
	      		zlog_warn("Master reconnect Vice [%d] times , but reconnect failed , so not reconnect !\n",master_board->fail);
				return;
	      	}
		  
	      if (! master_board->t_connect)
			master_board->t_connect = 
		  	thread_add_timer (zebrad.master, master_reset_connect_vice, master_board, 3);/*from 10s to 3s*/

		  #endif
	      	break;
			
	    case TIPC_CLIENT_READ:
		  if(tipc_client_debug)
		  	zlog_debug ("%s: line %d, go to read  ......\n", __func__,__LINE__);
		   master_board->t_read = 
		   thread_add_read (zebrad.master, master_board_read, master_board, master_board->sock);
			
	       break;
#if 0
		case TIPC_PACKETS_STATISTICS_TIMER:
			if(tipc_client_debug)
				zlog_debug("%s : line %d, go to request the interface packets statistics ...\n",__func__,__LINE__);
		/*	thread_add_timer(zebrad.master,master_board_request_interface_packets_statistics,master_board,time_interval);*/		
			thread_add_timer(zebrad.master,master_board_request_interface_packets_statistics,NULL,time_interval);

			break;
#endif
			
	    }
	if(tipc_client_debug)
	  zlog_debug("leave func %s %d\n",__func__,__LINE__);

}

void master_board_init(product_inf *product)
{
	int i = 0;
	
	if(tipc_client_debug)
	 zlog_debug("enter func %s .....\n",__func__);
		
	zebrad.vice_board_list = list_new();
	zlog_info(" The product can have %d(max) boards .\n",
				product->amount_num );
		
	for(i = 0; i < product->amount_num; i++)
	{
		zlog_debug("amount_num = %d , id_instance[%d] = %d , board_state[%d] = %d ..\n",
				product->amount_num, i ,product->id_instance[i], i ,product->board_state[i]);
		
		if(product->id_instance[i] == product->board_id)/*把active master自身给忽略掉。不创建socket*/
			continue;
		if(product->board_state[i] == BOARD_OFF_PRODUCT)
			continue;
		
		tipc_client *master_board;
		int ins = 0;

		master_board = XMALLOC(MTYPE_TIPC_CLIENT,sizeof(tipc_client));
		memset (master_board, 0, sizeof (tipc_client));

		master_board->sock = -1;
#if 0
		master_board->board_id = get_board_id(product->vice_board_id_list, i);
		master_board->board_id = 1;/********will change to read system file to get vice board id list********/
#endif
		master_board->board_id = product->id_instance[i];
		if(master_board->board_id < 0)
		  zlog_warn("Master get Vice board_id error and cannot creat tipc socket !\n");
		else 
		  zlog_debug("===================vice_board is %d================",master_board->board_id);
		master_board->ibuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
		master_board->obuf = stream_new(ZEBRA_MAX_PACKET_SIZ);
		master_board->wb = buffer_new(0);
		/*wangchao add */
		master_board->ifinfo = 1;
		
		master_board->t_connect = NULL;
		
		listnode_add (zebrad.vice_board_list, master_board);

		master_board_event (TIPC_CLIENT_SCHEDULE, master_board);
		
#if 0
		/**init hooks**/
		  master_board->interface_add = tipc_interface_add;
		  master_board->interface_delete = tipc_interface_delete;
		  master_board->interface_address_add = tipc_interface_address_add;
		  master_board->interface_address_delete = tipc_interface_address_delete;
	//	  master_board->ipv4_route_add = tipc_zebra_read_ipv4;
	//	  master_board->ipv4_route_delete = tipc_zebra_read_ipv4;
		  master_board->interface_up = tipc_interface_up;
		  master_board->interface_down = tipc_interface_down;

	//	  master_board->redistribute_interface_all = tipc_interface_add;
		  master_board->redistribute_route_all = tipc_redistribute_all; 
#endif
	}
	/*master_board_event(TIPC_PACKETS_STATISTICS_TIMER, NULL);*/
	
				
}


