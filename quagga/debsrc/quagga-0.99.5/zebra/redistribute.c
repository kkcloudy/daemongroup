/* Redistribution Handler
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include "vector.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "table.h"
#include "stream.h"
#include "zclient.h"
#include "linklist.h"
#include "log.h"

#include "zebra/rib.h"
#include "zebra/zserv.h"
#include "zebra/redistribute.h"
#include "zebra/debug.h"
#include "zebra/router-id.h"

#include "tipc_client.h"
#include "tipc_server.h"


/* master zebra server structure */
extern struct zebra_t zebrad;
extern product_inf *product;
extern int board_type ;


/*gujd : 2013-06-04, am 11:07.Add for Rtm Distribute System for interface uplink set/unset. Only active master done*/
void
zebra_interface_uplink_state(struct interface *ifp, int done)
{
	if(ifp == NULL)
	 return;
	
  struct listnode *node, *nnode;
  struct zserv *client;

  tipc_client *master_board;
/*  tipc_server *vice_board;*/
#if 0
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);
#endif
 
  if (product ==NULL){
		 return;
	 }
 
 
/*  if (product->board_id == 0)*//**will change: board_type**/
   if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*active主控*/
  	{
    	if(zebrad.vice_board_list == NULL)
    	{
      	   zlog_debug("%s, line %d, zebrad.vice_board_list(NULL).", __func__, __LINE__);
           return;
    	}
 	  
      /*send message to all of connected vice board*/
  	   for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
	   	{ 
  			if(done == 1)/*set uplink flag*/
			{
				master_send_interface_uplink_flag_update(ZEBRA_INTERFACE_UPLINK_FLAG_SET, master_board, ifp);
			}
			else if(done == 0)/*unset uplink flag*/
			{
				master_send_interface_uplink_flag_update(ZEBRA_INTERFACE_UPLINK_FLAG_UNSET, master_board, ifp);
			}
			else
			{
				zlog_warn("interface linkdetection done err !\n");
				return;
			}
  		   
  	    }
 	 
  } 
   else
   	{
   		zlog_warn("Not active master board can not redistribute interface uplink set/unset command !\n");
   	}
   
		
}

/*gjd : add for Rtm Distribute System for interface linkdetection. Only active master done*/
void
redistribute_interface_linkdetection(struct interface *ifp, int done)
{
	if(ifp == NULL)
	 return;
	
  struct listnode *node, *nnode;
  struct zserv *client;

  tipc_client *master_board;
/*  tipc_server *vice_board;*/
#if 0
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);
#endif
 
  if (product ==NULL){
		 return;
	 }
 
 
/*  if (product->board_id == 0)*//**will change: board_type**/
   if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*active主控*/
  	{
  	 #ifdef DP_DEBUG
    	zlog_debug("%s, line %d, product->board_type == BOARD_IS_ACTIVE_MASTER\n",__func__,__LINE__);
	 #endif
    	if(zebrad.vice_board_list == NULL)
    	{
    	 #ifdef DP_DEBUG
      	   zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		 #endif
           return;
    	}
 	  
      /*send message to all of connected vice board*/
  	   for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
	   	{ 
  			if(done == 1)/*enable (turn on) linkdetection*/
			{
/*			
				zlog_debug("master go to send interface %s linkdetection enable .\n",ifp->name);
*/
				master_send_interface_linkdetection_update(ZEBRA_INTERFACE_LINKDETECTION_ENABLE, master_board, ifp);
			}
			else if(done == 0)/*disable (turn on) linkdetection*/
			{
/*			
				zlog_debug("master go to send interface %s linkdetection disable .\n",ifp->name);
*/
				master_send_interface_linkdetection_update(ZEBRA_INTERFACE_LINKDETECTION_DISABLE, master_board, ifp);
			}
			else
			{
				zlog_warn("interface linkdetection done err !\n");
				return;
			}
  		   
  	    }
 	 
  } 
   else
   	{
   		zlog_warn("Not active master board can not redistribute interface linkdistribute !\n");
   	}
   
		
}

int
zebra_check_addr (struct prefix *p)
{
  if (p->family == AF_INET)
    {
      u_int32_t addr;

      addr = p->u.prefix4.s_addr;
      addr = ntohl (addr);

      if (IPV4_NET127 (addr) || IN_CLASSD (addr))
	return 0;
    }
#ifdef HAVE_IPV6
  if (p->family == AF_INET6)
    {
      if (IN6_IS_ADDR_LOOPBACK (&p->u.prefix6))
	return 0;
      if (IN6_IS_ADDR_LINKLOCAL(&p->u.prefix6))
	return 0;
    }
#endif /* HAVE_IPV6 */
  return 1;
}

int
is_default (struct prefix *p)
{
  if (p->family == AF_INET)
    if (p->u.prefix4.s_addr == 0 && p->prefixlen == 0)
      return 1;
#ifdef HAVE_IPV6
#if 0  /* IPv6 default separation is now pending until protocol daemon
          can handle that. */
  if (p->family == AF_INET6)
    if (IN6_IS_ADDR_UNSPECIFIED (&p->u.prefix6) && p->prefixlen == 0)
      return 1;
#endif /* 0 */
#endif /* HAVE_IPV6 */
  return 0;
}

static void
zebra_redistribute_default (struct zserv *client)
{
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *newrib;
#ifdef HAVE_IPV6
  struct prefix_ipv6 p6;
#endif /* HAVE_IPV6 */


  /* Lookup default route. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (table)
    {
      rn = route_node_lookup (table, (struct prefix *)&p);
      if (rn)
	{
	  for (newrib = rn->info; newrib; newrib = newrib->next)
	    if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED)
		&& newrib->distance != DISTANCE_INFINITY)
	      zsend_route_multipath (ZEBRA_IPV4_ROUTE_ADD, client, &rn->p, newrib);
	  route_unlock_node (rn);
	}
    }

#ifdef HAVE_IPV6
  /* Lookup default route. */
  memset (&p6, 0, sizeof (struct prefix_ipv6));
  p6.family = AF_INET6;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (table)
    {
      rn = route_node_lookup (table, (struct prefix *)&p6);
      if (rn)
	{
	  for (newrib = rn->info; newrib; newrib = newrib->next)
	    if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED)
		&& newrib->distance != DISTANCE_INFINITY)
	      zsend_route_multipath (ZEBRA_IPV6_ROUTE_ADD, client, &rn->p, newrib);
	  route_unlock_node (rn);
	}
    }
#endif /* HAVE_IPV6 */
}

/* Redistribute routes. */
static void
zebra_redistribute (struct zserv *client, int type)
{
  struct rib *newrib;
  struct route_table *table;
  struct route_node *rn;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      for (newrib = rn->info; newrib; newrib = newrib->next)
	if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED) 
	    && newrib->type == type 
	    && newrib->distance != DISTANCE_INFINITY
	    && zebra_check_addr (&rn->p))
	  zsend_route_multipath (ZEBRA_IPV4_ROUTE_ADD, client, &rn->p, newrib);
  
#ifdef HAVE_IPV6
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      for (newrib = rn->info; newrib; newrib = newrib->next)
	if (CHECK_FLAG (newrib->flags, ZEBRA_FLAG_SELECTED)
	    && newrib->type == type 
	    && newrib->distance != DISTANCE_INFINITY
	    && zebra_check_addr (&rn->p))
	  zsend_route_multipath (ZEBRA_IPV6_ROUTE_ADD, client, &rn->p, newrib);
#endif /* HAVE_IPV6 */
}

void
redistribute_add (struct prefix *p, struct rib *rib)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    {
      if (is_default (p))
        {
          if (client->redist_default || client->redist[rib->type])
            {
              if (p->family == AF_INET)
                zsend_route_multipath (ZEBRA_IPV4_ROUTE_ADD, client, p, rib);
#ifdef HAVE_IPV6
              if (p->family == AF_INET6)
                zsend_route_multipath (ZEBRA_IPV6_ROUTE_ADD, client, p, rib);
#endif /* HAVE_IPV6 */	  
	    }
        }
      else if (client->redist[rib->type])
        {
          if (p->family == AF_INET)
            zsend_route_multipath (ZEBRA_IPV4_ROUTE_ADD, client, p, rib);
#ifdef HAVE_IPV6
          if (p->family == AF_INET6)
            zsend_route_multipath (ZEBRA_IPV6_ROUTE_ADD, client, p, rib);
#endif /* HAVE_IPV6 */	  
        }
    }
}

void
redistribute_delete (struct prefix *p, struct rib *rib)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: processing rib %p", __func__, rib);


  /* Add DISTANCE_INFINITY check. */
  if (rib->distance == DISTANCE_INFINITY)
    return;

  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    {
      if (is_default (p))
	{
	  if (client->redist_default || client->redist[rib->type])
	    {
	      if (p->family == AF_INET)
		zsend_route_multipath (ZEBRA_IPV4_ROUTE_DELETE, client, p,
				       rib);
#ifdef HAVE_IPV6
	      if (p->family == AF_INET6)
		zsend_route_multipath (ZEBRA_IPV6_ROUTE_DELETE, client, p,
				       rib);
#endif /* HAVE_IPV6 */
	    }
	}
      else if (client->redist[rib->type])
	{
	  if (p->family == AF_INET)
	    zsend_route_multipath (ZEBRA_IPV4_ROUTE_DELETE, client, p, rib);
#ifdef HAVE_IPV6
	  if (p->family == AF_INET6)
	    zsend_route_multipath (ZEBRA_IPV6_ROUTE_DELETE, client, p, rib);
#endif /* HAVE_IPV6 */
	}
    }
}


void
zebra_redistribute_add (int command, struct zserv *client, int length)
{
  int type;

  type = stream_getc (client->ibuf);

  switch (type)
    {
    case ZEBRA_ROUTE_KERNEL:
    case ZEBRA_ROUTE_CONNECT:
    case ZEBRA_ROUTE_STATIC:
    case ZEBRA_ROUTE_RIP:
    case ZEBRA_ROUTE_RIPNG:
    case ZEBRA_ROUTE_OSPF:
    case ZEBRA_ROUTE_OSPF6:
    case ZEBRA_ROUTE_BGP:
      if (! client->redist[type])
	{
	  client->redist[type] = 1;
	  zebra_redistribute (client, type);
	}
      break;
    default:
      break;
    }
}     

void
zebra_redistribute_delete (int command, struct zserv *client, int length)
{
  int type;

  type = stream_getc (client->ibuf);

  switch (type)
    {
    case ZEBRA_ROUTE_KERNEL:
    case ZEBRA_ROUTE_CONNECT:
    case ZEBRA_ROUTE_STATIC:
    case ZEBRA_ROUTE_RIP:
    case ZEBRA_ROUTE_RIPNG:
    case ZEBRA_ROUTE_OSPF:
    case ZEBRA_ROUTE_OSPF6:
    case ZEBRA_ROUTE_BGP:
      client->redist[type] = 0;
      break;
    default:
      break;
    }
}     

void
zebra_redistribute_default_add (int command, struct zserv *client, int length)
{
  client->redist_default = 1;
  zebra_redistribute_default (client);
}     

void
zebra_redistribute_default_delete (int command, struct zserv *client,
				   int length)
{
  client->redist_default = 0;;
}     

/* Interface up information. */
void
zebra_interface_description_update (int command,struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  tipc_client *master_board;
  tipc_server *vice_board;

 // if (IS_ZEBRA_DEBUG_EVENT)
    zlog_debug ("MESSAGE: description update %s", ifp->name);
  
#if 0 /*ospf , rip no need description.*/
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);
#endif
 
  if (product ==NULL){
		 return;
	 }
   
   if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*active主控*/
  	{
      if(zebrad.vice_board_list == NULL)
		{
	  	   zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
	       return;
		}
 	  
      /*send message to all of connected vice board*/
  	   for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
	   	{ 
		   if(command == ZEBRA_INTERFACE_DESCRIPTION_SET)/*set*/
		   {
				master_send_interface_descripton_set(command, master_board, ifp);
		   	}
		   else /*unset*/
		   	{
				master_send_interface_descripton_unset(command, master_board, ifp);
		   	}
  	    }
 	 
  } 
   else if(product->board_type == BOARD_IS_BACKUP_MASTER)/*备用主控*/
	{

		 if(zebrad.master_board_list == NULL) {
			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 return;
		  }
	  if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*备用主控上rpa口和虚拟口的状态不发给active主控*/
	 	return;
	  
	/*gujd : 2012-05-03, pm 6:30 .*/		  
	  if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
	  	return;

		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
		 { 
		   if(command == ZEBRA_INTERFACE_DESCRIPTION_SET)/*set*/
		   {
				vice_send_interface_descripton_set(command, vice_board, ifp);
		   	}
		   else /*unset*/
		   	{
				vice_send_interface_descripton_unset(command, vice_board, ifp);
		   	}
  	    }  
	}
  else if(product->board_type == BOARD_IS_VICE)/*业务板*/
  	{
 		 if(zebrad.master_board_list == NULL) 
		 {
 			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
 			 return;
 		  }
		 if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*业务板上rpa口和虚拟口的状态不发给active主控*/
		 	return;
		 
		 /*gujd : 2012-05-03, pm 6:30 .*/		   
		   if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
			 return;
 
 		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
		 { 
		   if(command == ZEBRA_INTERFACE_DESCRIPTION_SET)/*set*/
		   {
				vice_send_interface_descripton_set(command, vice_board, ifp);
		   	}
		   else /*unset*/
		   	{
				vice_send_interface_descripton_unset(command, vice_board, ifp);
		   	}
  	    } 
		 
  }
  
}

/* Interface up information. */
void
zebra_interface_up_update (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  tipc_client *master_board;
  tipc_server *vice_board;

  if (IS_ZEBRA_DEBUG_EVENT)
    zlog_debug ("MESSAGE: ZEBRA_INTERFACE_UP %s", ifp->name);
  
  if(judge_obc_interface(ifp->name)!=OBC_INTERFACE)
  {
	  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
	   zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);

  }
  else
  {
		return ;
  }

 
  if (product ==NULL){
		 return;
	 }
  if(CHECK_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION))
  	{
  	/*	zlog_debug("product is %d , up return. Use linkdetection command to send to othe board .\n",product->board_type);*/
		return;
  	}
 
/*  if (product->board_id == 0)*//**will change: board_type**/
   if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*active主控*/
  	{
  	 #ifdef DP_DEBUG
    	zlog_debug("%s, line %d, product->board_type == BOARD_IS_ACTIVE_MASTER\n",__func__,__LINE__);
	 #endif
    	if(zebrad.vice_board_list == NULL)
    	{
    	 #ifdef DP_DEBUG
      	   zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		 #endif
           return;
    	}
 	  
      /*send message to all of connected vice board*/
  	   for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) { 
  
  		   //if (client->ifinfo) {
  		    #ifdef DP_DEBUG
  			  zlog_debug("enter redistribute.c to all of master_interface_up interface_name = %s", ifp->name); 	   
			#endif
			  master_send_interface_update (ZEBRA_INTERFACE_UP, master_board, ifp);
  		   //}
  	    }
 	 
  } 
   else 
  /*if (product->board_id != 0) */
 /* if(product->board_type == BOARD_IS_BACKUP_MASTER ||product->board_type == BOARD_IS_VICE)*/
  if(product->board_type == BOARD_IS_BACKUP_MASTER)/*备用主控*/
  {
   #ifdef DP_DEBUG
 	     zlog_debug("%s, line %d, board_type == BOARD_IS_VICE or BOARD_IS_BACKUP_MASTER \n",__func__,__LINE__);
   #endif
 
 		 if(zebrad.master_board_list == NULL) {
		 	 #ifdef DP_DEBUG
 			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif
 			 return;
 		  }
		  if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*备用主控上rpa口和虚拟口的状态不发给active主控*/
		 	return;
		  
		/*gujd : 2012-05-03, pm 6:30 .*/		  
		  if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
		  	return;
 
 		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) { 
 			// if (client->ifinfo) {
 			 #ifdef DP_DEBUG
 				zlog_debug("enter redistribute.c to all of interface_up  interface_name = %s", ifp->name);
			#endif
				vice_send_interface_update (ZEBRA_INTERFACE_UP, vice_board, ifp);
 			// }
 		  }  
  }
  else
  	if(product->board_type == BOARD_IS_VICE)/*业务板*/
  	{
   		#ifdef DP_DEBUG
 	     zlog_debug("%s, line %d, board_type == BOARD_IS_VICE or BOARD_IS_BACKUP_MASTER \n",__func__,__LINE__);
   		#endif
 
 		 if(zebrad.master_board_list == NULL) {
		 	 #ifdef DP_DEBUG
 			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif
 			 return;
 		  }
		 if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*业务板上rpa口和虚拟口的状态不发给active主控*/
		 	return;
		 
		 /*gujd : 2012-05-03, pm 6:30 .*/		   
		   if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
			 return;
 
 		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) { 
 			// if (client->ifinfo) {
 			 #ifdef DP_DEBUG
 				zlog_debug("enter redistribute.c to all of interface_up  interface_name = %s", ifp->name);
			#endif
				vice_send_interface_update (ZEBRA_INTERFACE_UP, vice_board, ifp);
 			// }
 		  }  
  }
  
}

/* Interface down information. */
void
zebra_interface_down_update (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client *master_board;
  tipc_server *vice_board;

  if (IS_ZEBRA_DEBUG_EVENT)
    zlog_debug ("MESSAGE: ZEBRA_INTERFACE_DOWN %s", ifp->name);

  if(judge_obc_interface(ifp->name)!=OBC_INTERFACE)
   for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    zsend_interface_update (ZEBRA_INTERFACE_DOWN, client, ifp);


  if (product ==NULL){
		 return;
	 }
  if(CHECK_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION))
  	{
	  /*  zlog_debug("product is %d , down return. Use linkdetection command to send to othe board .\n",product->board_type);*/
		return;
  	}

  
/*  if (product->board_id == 0)*//*will change to board_type**/
  	if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*active主控*/
  	{
    	if(zebrad.vice_board_list == NULL)
    	{
    	 #ifdef DP_DEBUG
      	   zlog_debug("%s, line %d, zebrad.vice_board_list....", __func__, __LINE__);
		 #endif
           return;
    	}
 	  
      /*send message to all of connected vice board*/
  	   for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) { 
  
  		   //if (client->ifinfo) {
  		    #ifdef DP_DEBUG
  			  zlog_debug("enter redistribute.c to all of master_interface_up interface_name = %s", ifp->name); 	   
			#endif
			  master_send_interface_update (ZEBRA_INTERFACE_DOWN, master_board, ifp);
  		   //}
  	    }
 	 
  }
	else 
/*  if (product->board_id != 0) */
/*  if(product->board_type == BOARD_IS_BACKUP_MASTER ||product->board_type == BOARD_IS_VICE)*/
  if(product->board_type == BOARD_IS_BACKUP_MASTER )/*备用主控*/
  {
 
 		 if(zebrad.master_board_list == NULL) {
		 	 #ifdef DP_DEBUG
 			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif			 
 			 return;
 		  }
		 
		 if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*备用主控上rpa口和虚拟口的状态不发给active主控*/
		 	return;
		 
		 /*gujd : 2012-05-03, pm 6:30 .*/		   
		   if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
			 return;
 
 		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) { 
 			// if (client->ifinfo) {
 			 #ifdef DP_DEBUG
 				zlog_debug("enter redistribute.c to all of interface_up  interface_name = %s", ifp->name);
			#endif
				vice_send_interface_update (ZEBRA_INTERFACE_DOWN, vice_board, ifp);
 			// }
 		  }  
  }
else
  if(product->board_type == BOARD_IS_VICE)/*业务板*/
	{

		 if(zebrad.master_board_list == NULL) {
	 	 #ifdef DP_DEBUG
			 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		 #endif			 
			 return;
		  }

		 if(ifp->if_types == RPA_INTERFACE || ifp->if_types == VIRTUAL_INTERFACE)/*业务板上rpa口和虚拟口的状态不发给active主控*/
		 	return;
		 
		 /*gujd : 2012-05-03, pm 6:30 .*/		   
		   if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)
			 return;

		 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) { 
			// if (client->ifinfo) {
		#ifdef DP_DEBUG
				zlog_debug("enter redistribute.c to all of interface_up  interface_name = %s", ifp->name);
		#endif
			vice_send_interface_update (ZEBRA_INTERFACE_DOWN, vice_board, ifp);
			// }
		  }  
}
  
}




/* Interface information update. */
void
zebra_interface_add_update (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_client *master_board;
  tipc_server *vice_board;
  
  int peer_slot = 0;

  if (IS_ZEBRA_DEBUG_EVENT)
    zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADD %s", ifp->name);
    
	
   if(judge_obc_interface(ifp->name)!=OBC_INTERFACE){
  		for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
    		if (client->ifinfo)
      			zsend_interface_add (client, ifp);
   	}
   else
   {
		return;
   }
	

    if (product ==NULL){
		return;
	}

	/*ifp->if_types = REAL_INTERFACE;*/

//	if (product->board_id == 0)
	if(product->board_type == BOARD_IS_ACTIVE_MASTER)
	{
		if(zebrad.vice_board_list == NULL)
		  {
		   #ifdef DP_DEBUG
			zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		   #endif
			return;
		  }
	     
	 /*send message to all of connected vice board*/
		for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) { 
			 #ifdef DP_DEBUG
               zlog_debug("enter redistribute.c to all of vice_board add_interface_name = %s", ifp->name);		  
			#endif

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
           
		 }
		
	} 
	else if(product->board_type == BOARD_IS_BACKUP_MASTER ||product->board_type == BOARD_IS_VICE)
	{
			if(zebrad.master_board_list == NULL) 
			{
			 #ifdef DP_DEBUG
        		zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			#endif
        		return;
		 	 }

			for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) 
				{ 
	           // if (client->ifinfo) {
	           #ifdef DP_DEBUG 
	               zlog_debug("enter redistribute.c to all of vice_board add_interface_name = %s", ifp->name);
				#endif
				
				if(judge_real_local_interface(ifp->name)== LOCAL_BOARD_INTERFACE)
				   vice_send_interface_add (vice_board, ifp);
	           // }
			 }	
	}

	return ;

}
void
zebra_interface_delete_update (struct interface *ifp)
{
    struct listnode *node, *nnode;
    struct zserv *client;
	tipc_client *master_board;
    tipc_server *vice_board;
	
  
    if (IS_ZEBRA_DEBUG_EVENT)
      zlog_debug ("MESSAGE: ZEBRA_INTERFACE_DELETE %s", ifp->name);
  
  if(judge_obc_interface(ifp->name)!=OBC_INTERFACE)
  {
	  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
		if (client->ifinfo)
		  zsend_interface_delete (client, ifp);

  }
  else
  {
	return ;
  }
  
    if (product ==NULL){
  		return;
  	}


//	if (product->board_id == 0)
	
	if(product->board_type == BOARD_IS_ACTIVE_MASTER)
	{
		int peer_slot = 0;
		
		if(zebrad.vice_board_list == NULL)
		  {
		   #ifdef DP_DEBUG
			zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		   #endif
			return;
		  }
		if ((CHECK_FLAG(ifp->ip_flags , IPV4_ADDR_DEL)||CHECK_FLAG(ifp->ip_flags , IPV6_ADDR_DEL)) && ifp->if_types == RPA_INTERFACE)
		{
			zlog_debug(" info: del rpa interface ip address(command) case vice board to del rpa interface ,so not send del interface.\n ");
			UNSET_FLAG(ifp->ip_flags , IPV4_ADDR_DEL);
			UNSET_FLAG(ifp->ip_flags , IPV6_ADDR_DEL);
			ifp->slot = 0;
			ifp->devnum = 0;/**把slot与devnum置回0，防止再次创建此口的rpa口时，devnum可能已经被其他口占用，要重新分配**/
			return ;
			}
		if ((CHECK_FLAG(ifp->ip_flags , IPV4_ADDR_DISTRIBUTE_DEL)||CHECK_FLAG(ifp->ip_flags , IPV6_ADDR_DISTRIBUTE_DEL)) && ifp->if_types == RPA_INTERFACE)
		{
			zlog_debug(" info: del rpa interface ip address(distribute) case vice board to del rpa interface ,so not send del interface.\n ");
			UNSET_FLAG(ifp->ip_flags , IPV4_ADDR_DISTRIBUTE_DEL);
			UNSET_FLAG(ifp->ip_flags , IPV6_ADDR_DISTRIBUTE_DEL);
			ifp->slot = 0;
			ifp->devnum = 0;/**把slot与devnum置回0，防止再次创建此口的rpa口时，devnum可能已经被其他口占用，要重新分配**/
			return ;
			}
	 /*send message to all of connected vice board*/
		for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			{ 	
			 #ifdef DP_DEBUG
               zlog_debug("enter redistribute.c to all of vice_board add_interface_name = %s", ifp->name);		  
			#endif
			
			/*gujd: 2012-09-22, am 10:20 .Add for when recive create or delete from his borad,only send other board not send his own board again.*/			
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
		 }
		
	} 
	else 
	//if (product->board_id != 0) 
	if(product->board_type == BOARD_IS_VICE ||product->board_type == BOARD_IS_BACKUP_MASTER)
	{
			if(zebrad.master_board_list == NULL) 
			{
			 #ifdef DP_DEBUG
        		zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif
        		return;
		 	 }
			/*if the detele interface is cased by delete ip address by rpa interface ,so not send delete message to active master , only send local board protocol process .*/
			if ((CHECK_FLAG(ifp->ip_flags , IPV4_ADDR_DISTRIBUTE_DEL)||CHECK_FLAG(ifp->ip_flags , IPV6_ADDR_DISTRIBUTE_DEL)) && ifp->if_types == RPA_INTERFACE)
			{
				zlog_debug(" info: del rpa interface ip address(distribute) case acive master board to del rpa interface ,so not send del interface.\n ");
				UNSET_FLAG(ifp->ip_flags , IPV4_ADDR_DISTRIBUTE_DEL);
				UNSET_FLAG(ifp->ip_flags , IPV6_ADDR_DISTRIBUTE_DEL);
				ifp->slot = 0;
				ifp->devnum = 0;/**把slot与devnum置回0，防止再次创建此口的rpa口时，devnum可能已经被其他口占用，要重新分配**/
				return ;
				}

			for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
			{ 	 
			 #ifdef DP_DEBUG
	               zlog_debug("enter redistribute.c to all of vice_board_delete interface_name = %s", ifp->name);
			#endif
				 if(judge_real_local_interface(ifp->name)== LOCAL_BOARD_INTERFACE)
				   vice_send_interface_delete (vice_board, ifp);
			 }	
			ifp->slot = 0;
			ifp->devnum = 0;/**把slot与devnum置回0，防止再次创建此口的rpa口时，devnum可能已经被其他口占用，要重新分配**/
	}

	return ;	
	
}

/* Interface address addition. */
void
zebra_interface_address_add_update (struct interface *ifp,
				    struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  struct prefix *p;
  tipc_client *master_board;
  tipc_server *vice_board;
  char buf[BUFSIZ];

  int slot = 0;
  p = ifc->address;

  
  if (IS_ZEBRA_DEBUG_EVENT)
				  zlog_debug ("Func %s start %d ",__func__,__LINE__);

  if (IS_ZEBRA_DEBUG_EVENT)
    {
      zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADDRESS_ADD %s/%d on %s",
		  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		  p->prefixlen, ifc->ifp->name);
    }


  if( product != NULL && ((ifp->if_types == VIRTUAL_INTERFACE)
  							||(ifp->ifindex == IFINDEX_INTERNAL)))
  	goto skip;

  router_id_add_address(ifc);

  if(judge_obc_interface(ifp->name)!=OBC_INTERFACE)
  {
	  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
	   if (client->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
		 zsend_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, client, ifp, ifc);

  }
  else
  {
	return;
  }


skip:
  if (product ==NULL){
		return;
   }

  if(strncmp(ifp->name, "obc", 3) == 0)
  	return;
  	

 
    if(product->board_type == BOARD_IS_ACTIVE_MASTER)
     {
     /*7605i and 8603*/
	 if (p->family == AF_INET  )
	 {	 /*ipv6 not support interface loacal*/
	     	if((product->product_type == PRODUCT_TYPE_7605I
				||product->product_type == PRODUCT_TYPE_8603) 
				&&(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)
				&&(CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)))
	     	  {
	     	  	int slot_id = 0;
				slot_id = get_slot_num(ifp->name);
				if(slot_id == product->board_id)
				{
					zlog_debug("The interface %s is local active master interface .\n",ifp->name);
					return;
				 }
					
	     	  }
	 }
		if(zebrad.vice_board_list == NULL)
		  {
		   #ifdef DP_DEBUG
			zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		   #endif
			return;
		  }
		if(strncmp(ifp->name, "wlan", 4) == 0 || strncmp(ifp->name, "ebr", 3) == 0)
		{
			slot = get_slot_num(ifp->name);
			if(slot != product->board_id)
				usleep(1000);/*1ms*/
			}
	     
	      /*send message to all of connected vice board*/
		  if (p->family == AF_INET	)
			 for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) {
	  			if (  CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)||CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)) 
				{
					master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, master_board, ifp, ifc);
	  		     }
			  }
		else 
			  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) {
	  			if (  CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) 
				{
					master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, master_board, ifp, ifc);
	  		     }
			  }
		
  } else 
  if(product->board_type == BOARD_IS_VICE ||product->board_type == BOARD_IS_BACKUP_MASTER)
  	{
			if(zebrad.master_board_list == NULL) 
			{
			 #ifdef DP_DEBUG
        		zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif
        		return;
		 	 }
			if(judge_real_local_interface(ifp->name)== LOCAL_BOARD_INTERFACE)
			{
			for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) 
				{
				 #ifdef DP_DEBUG
			      zlog_debug("%s, line == %d ",__func__,__LINE__); 
				 #endif
				  if (  CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) {
				   		vice_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, vice_board, ifp, ifc);
		          }
			 }
			}
  }
	
}

/* Interface address deletion. */
void
zebra_interface_address_delete_update (struct interface *ifp,
				       struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  struct prefix *p;
  tipc_client *master_board;
  tipc_server *vice_board;
  char buf[BUFSIZ];

   if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("Func %s start, line %d ",__func__,__LINE__);

  if (IS_ZEBRA_DEBUG_EVENT)
    {
      p = ifc->address;
      zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADDRESS_DELETE %s/%d on %s",
		  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
		 p->prefixlen, ifc->ifp->name);
    }
  
  /*gujd : 2012-02-14 , am 11:27.*/
  if( product != NULL && ((ifp->if_types == VIRTUAL_INTERFACE)
							  ||(ifp->ifindex == IFINDEX_INTERNAL)))
	  goto skip;

  router_id_del_address(ifc);

  if(judge_obc_interface(ifp->name)!=OBC_INTERFACE){
	  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
	   if (client->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
		 zsend_interface_address (ZEBRA_INTERFACE_ADDRESS_DELETE, client, ifp, ifc);

  	}
  else
  	{
		return;
  	}

skip:
  if (product ==NULL){
	  return;
  }

  if(strncmp(ifp->name, "obc", 3) == 0)
	  return;

  if(product->board_type == BOARD_IS_ACTIVE_MASTER)
  	{
     if(zebrad.vice_board_list == NULL)
     	{
     	 #ifdef DP_DEBUG
     	  zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
		 #endif
     	  return;
     	}
     
     	/*send message to all of connected vice board*/
     for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) {
     	  if ( CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)||CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)) {
     		  master_send_interface_address (ZEBRA_INTERFACE_ADDRESS_DELETE, master_board, ifp, ifc);
     	   }
     	}
   
  } else if(product->board_type == BOARD_IS_VICE ||product->board_type == BOARD_IS_BACKUP_MASTER)

  	{
    	  if(zebrad.master_board_list == NULL) 
		  	{
		  	 #ifdef DP_DEBUG
    		  zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
			 #endif
    		  return;
    	   }
    
		  if(judge_real_local_interface(ifp->name)== LOCAL_BOARD_INTERFACE)
			{
    	        for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) 
			  	{
			  		if(ifc->address->family == AF_INET6 
						&&CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
					{
						zlog_debug("%s, line %d, The address is config by CLI , doesn't need to sync master .", __func__, __LINE__);
						return;
			  			}
						
					  if ( CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)){ 
	    			 	 vice_send_interface_address (ZEBRA_INTERFACE_ADDRESS_DELETE, vice_board, ifp, ifc);
		    			   }
	    		}
    	   }
  
  }
  

	
if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("Func %s end, line %d ",__func__,__LINE__);
}

#if 0
void
zebra_interface_packets_statistics_request(struct interface *ifp)
{
  tipc_client  *master_board ;
  struct listnode *ifnode, *ifnnode;
  struct listnode *node, *nnode;
  struct listnode *cnode, *cnnode;
  struct connected *c;
  int ret = 0;

  if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	return -1;
  
  if(zebrad.vice_board_list == NULL)
  {
	zlog_debug("%s, line = %d :tipc_client_list is NULL....", __func__, __LINE__);
	return -1;
  }

  if(ifp)
  {
	  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
	  {
		zlog_debug("%s : line %d , Active master send request interface packets statistics to other boards .\n",__func__,__LINE__);
		master_board_send_request_to_vice(ZEBRA_INTERFACE_PACKETS_REQUEST,master_board,ifp);
		
		}
	 }
  else
  	{
	  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board))
	  {
		zlog_debug("%s : line %d , Active master send request interface packets statistics to other boards .\n",__func__,__LINE__);
		master_board_send_request_to_vice(ZEBRA_INTERFACE_PACKETS_REQUEST_ALL,master_board,NULL);
		
		}
	 }
  
  return 0;
}
#endif

