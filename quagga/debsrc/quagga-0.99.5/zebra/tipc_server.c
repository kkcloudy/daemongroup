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
*tipc_zclient_lib.c
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
#include "debug.h"
	
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
	
	
#include "zebra/zserv.h"
#include "tipc_zclient_lib.h"
#include "tipc_server.h"
#include "zebra/debug.h"


enum tipc_serv_events { TIPC_SERV, TIPC_READ, TIPC_WRITE };

struct
{  
  int key;
  int distance;
} route_info_distance[] =
{
  {ZEBRA_ROUTE_SYSTEM,    0},
  {ZEBRA_ROUTE_KERNEL,    0},
  {ZEBRA_ROUTE_CONNECT,   0},
  {ZEBRA_ROUTE_STATIC,    1},
  {ZEBRA_ROUTE_RIP,     120},
  {ZEBRA_ROUTE_RIPNG,   120},
  {ZEBRA_ROUTE_OSPF,    110},
  {ZEBRA_ROUTE_OSPF6,   110},
  {ZEBRA_ROUTE_ISIS,    115},
  {ZEBRA_ROUTE_BGP,      20  /* IBGP is 200. */}
};


static void vice_board_event(enum tipc_serv_events event,int sock,tipc_server * vice_board);

/**some func api add by gjd : 2011-01--2011-02**/
/**tipc server packet ip route info , and send to tipc client, the func like other protocol client **/

extern product_inf *product;
extern int board_type;
extern int keep_kernel_mode;
extern int route_boot_errno ;
extern void nexthop_free (struct nexthop *nexthop);

/*int tipc_server_debug = 1;*/


//tipc_client_kernel_add_ipv4(struct prefix *p, struct rib *rib)
int
tipc_client_kernel_add_ipv4(struct prefix_ipv4 *ipv4, struct rib *rib)

{
	
	/**use prefix_ipv4 and also can use prefix**/  
	  struct prefix *p ;
	  char buf1[BUFSIZ] = {0};

	  if(!ipv4 || !rib)
	  {
		zlog_warn("%s: ipv4 prefix is %p or rib is %p .\n",__func__,ipv4,rib);
		return -1;
		}
	  p = XCALLOC(MTYPE_PREFIX ,sizeof(struct prefix));
	  if(!p)
	  {
	  	zlog_warn("%s: malloc prefix p failed.\n",__func__);
		return -1;
	  }
	  p->family = AF_INET;
	  p->prefixlen = ipv4->prefixlen;
	/*	stream_get (&p.prefix, s, PSIZE (p.prefixlen));*/
	/*  stream_get (&p->u.prefix4, s, PSIZE (p->prefixlen));*//**fetch dest ip**/
#if 0
/////////DONGSHU
	  if(p != NULL && ipv4 != NULL)
		 strcpy(&p->u.prefix4 ,&ipv4->prefix);
#else
	/*CID 13866 (#1 of 1): Dereference before null check (REVERSE_INULL)
	 check_after_deref: Null-checking "p" suggests that it may be null, but it has already been dereferenced on all paths leading to the check.
	 CID 13865 (#1 of 1): Dereference before null check (REVERSE_INULL)
	 check_after_deref: Null-checking "ipv4" suggests that it may be null, but it has already been dereferenced on all paths leading to the check
	 So move ipv4 check to above. */
	if(p != NULL && ipv4 != NULL)
	memcpy(&p->u.prefix4 ,&ipv4->prefix,sizeof(ipv4->prefix));


#endif
	  if(tipc_server_debug)
		zlog_debug("%s : line %d, recv packet dest ipv4 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET,&p->u.prefix4,buf1,BUFSIZ),p->prefixlen);	
	  if(tipc_server_debug)
	    zlog_debug ("%s: line %d, go to tell kernel add ipv4 ......\n", __func__,__LINE__);
	  
	  netlink_route_multipath(RTM_NEWROUTE, p, rib, AF_INET);
	  /*CID 13541 (#1-2 of 2): Resource leak (RESOURCE_LEAK)
		9. leaked_storage: Variable "p" going out of scope leaks the storage it points to.
		Free p.*/
	  XFREE(MTYPE_PREFIX,p);
	  return 0;
	  
}



int 
tipc_client_kernel_delete_ipv4(struct prefix_ipv4 *ipv4, struct rib *rib)
{	
	  struct prefix *p ;
	  char buf1[BUFSIZ] = {0};
  
	  if(!ipv4 || !rib)
	  {
		zlog_warn("%s: ipv4 prefix is %p or rib is %p .\n",__func__,ipv4,rib);
		return -1;
		}
	  p = XCALLOC(MTYPE_PREFIX ,sizeof(struct prefix));
	  if(!p)
	  {
	  	zlog_warn("%s: malloc prefix p failed.\n",__func__);
		return -1;
	  }
	  p->family = AF_INET;
	  p->prefixlen = ipv4->prefixlen;
	/*	stream_get (&p.prefix, s, PSIZE (p.prefixlen));*/
	/*  stream_get (&p->u.prefix4, s, PSIZE (p->prefixlen));*//**fetch dest ip**/
//DONGSHU
#if 0
	  if(p != NULL && ipv4 != NULL)
		 strcpy(&p->u.prefix4 ,&ipv4->prefix);
#else
	/*CID 13867 (#1 of 1): Dereference before null check (REVERSE_INULL)
	check_after_deref: Null-checking "ipv4" suggests that it may be null, but it has already been dereferenced on all paths leading to the check
       CID 13868 (#1 of 1): Dereference before null check (REVERSE_INULL)
       check_after_deref: Null-checking "p" suggests that it may be null, but it has already been dereferenced on all paths leading to the check. 
	So move ipv4 check to above. */
	if(p != NULL && ipv4 != NULL)
	   memcpy(&p->u.prefix4 ,&ipv4->prefix,sizeof(ipv4->prefix));

#endif
	  if(tipc_server_debug)
		zlog_debug("%s : line %d, recv packet dest ipv4 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET,&p->u.prefix4,buf1,BUFSIZ),p->prefixlen);

	  if(tipc_server_debug)
		zlog_debug ("%s: line %d, go to tell kernel delete ipv4 ......\n", __func__,__LINE__);
	  
	  netlink_route_multipath(RTM_DELROUTE, p, rib, AF_INET);	
	  
	  /*CID 13542 (#1-2 of 2): Resource leak (RESOURCE_LEAK)
	  	9. leaked_storage: Variable "p" going out of scope leaks the storage it points to.
	 	Free p.*/
	 	XFREE(MTYPE_PREFIX,p);
	  	return 0;
}


/*gujd ; 2013-3-12, pm 5:18. Add code for static route and protocol(like ospf ,rip ,...) route deal with split.
    The static route first add to vrf_static_table, for show running,  then add to vrf_table. 
    But other route add to vrf_table only. */
    
#if 1
/* Delete static route from static route configuration. */
int
vice_static_delete_ipv4 (struct prefix *p, struct in_addr *gate, const char *ifname,
		    u_char distance, u_int32_t vrf_id)
{
  u_char type = 0;
  struct route_node *rn;
  struct static_ipv4 *si;
  struct route_table *stable;

  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP, SAFI_UNICAST, vrf_id);
  if (! stable)
    return -1;

  /* Lookup static route prefix. */
  rn = route_node_lookup (stable, p);
  if (! rn)
    return 0;

  /* Make flags. */
  if (gate)
    type = STATIC_IPV4_GATEWAY;
  else if (ifname)
    type = STATIC_IPV4_IFNAME;
  else
    type = STATIC_IPV4_BLACKHOLE;

  /* Find same static route is the tree */
  for (si = rn->info; si; si = si->next){
    if (type == si->type
			&& (! gate || IPV4_ADDR_SAME (gate, &si->gate.ipv4))
			&& (! ifname || strcmp (ifname, si->gate.ifname) == 0))
      break;
		
}
  /* Can't find static route. */
  if (! si)
    {
      route_unlock_node (rn);
      return 0;
    }

  /* Install into rib. */
  static_uninstall_ipv4 (p, si);

  /* Unlink static route from linked list. */
  if (si->prev)
    si->prev->next = si->next;
  else
    rn->info = si->next;
  if (si->next)
    si->next->prev = si->prev;
  route_unlock_node (rn);
  
  /* Free static route configuration. */
  if (ifname)
    XFREE (0, si->gate.ifname);
  XFREE (MTYPE_STATIC_IPV4, si);

  route_unlock_node (rn);

  return 1;
}


/* Add static route into static route configuration. */
int
vice_static_add_ipv4 (struct prefix *p, struct in_addr *gate, const char *ifname,
u_char flags, u_char distance, u_int32_t vrf_id)
{
  u_char type = 0;
  struct route_node *rn;
  struct static_ipv4 *si;
  struct static_ipv4 *pp;
  struct static_ipv4 *cp;
  struct static_ipv4 *update = NULL;
  struct route_table *stable;

  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP, SAFI_UNICAST, vrf_id);
  if (! stable)
    return -1;
  
  /* Lookup static route prefix. */
  /*rn = route_node_get (stable, p);*/

  /* Make flags. */
  if (gate)
    type = STATIC_IPV4_GATEWAY;
  else if (ifname)
    type = STATIC_IPV4_IFNAME;
  else
    type = STATIC_IPV4_BLACKHOLE;
  /*added by gxd  special for  nll0,rej,bh static route entry*/
  if(STATIC_IPV4_BLACKHOLE==type||ZEBRA_FLAG_BLACKHOLE==flags||ZEBRA_FLAG_REJECT==flags){
  	if(route_node_lookup(stable,p))
		return -2;
  }
  else{
  	struct route_node *myrn;
	struct route_table *mytable;
	struct rib *myrib;
	if(mytable=vrf_table (AFI_IP, SAFI_UNICAST, 0))
		if(myrn=route_node_lookup(mytable,p))
			 for (myrib = myrn->info;myrib; myrib = myrib->next){
				if(ZEBRA_ROUTE_STATIC==myrib->type&&distance==myrib->distance&&(CHECK_FLAG (myrib->flags, ZEBRA_FLAG_BLACKHOLE)||CHECK_FLAG (myrib->flags, ZEBRA_FLAG_REJECT)))
					return -3;
			}
  }
  rn = route_node_get (stable, p);
  if(!rn)
  {
  	zlog_warn("%s : Satic route node is NULL.\n",__func__);
	return -1;
  	}

  /* Do nothing if there is a same static route.  */
  for (si = rn->info; si; si = si->next)
    {
      if (type == si->type
	  && (! gate || IPV4_ADDR_SAME (gate, &si->gate.ipv4))
	  && (! ifname || strcmp (ifname, si->gate.ifname) == 0))
	{
	  if (distance == si->distance)
	    {
	      route_unlock_node (rn);
	      return 0;
	    }
	  else
	    update = si;
	}
    }
  /* Distance chaged.  */
  if (update)
    static_delete_ipv4 (p, gate, ifname, update->distance, vrf_id);
  /* Make new static route structure. */
  si = XMALLOC (MTYPE_STATIC_IPV4, sizeof (struct static_ipv4));
  memset (si, 0, sizeof (struct static_ipv4));

  si->type = type;
  si->distance = distance;
  si->flags = flags;

  if (gate)
    si->gate.ipv4 = *gate;
  if (ifname)
    si->gate.ifname = XSTRDUP (0, ifname);

  /* Add new static route information to the tree with sort by
     distance value and gateway address. */
  for (pp = NULL, cp = rn->info; cp; pp = cp, cp = cp->next)
    {
      if (si->distance < cp->distance)
	break;
      if (si->distance > cp->distance)
	continue;
      if (si->type == STATIC_IPV4_GATEWAY && cp->type == STATIC_IPV4_GATEWAY)
	{
	  if (ntohl (si->gate.ipv4.s_addr) < ntohl (cp->gate.ipv4.s_addr))
	    break;
	  if (ntohl (si->gate.ipv4.s_addr) > ntohl (cp->gate.ipv4.s_addr))
	    continue;
	}
    }

  /* Make linked list. */
  if (pp)
    pp->next = si;
  else
    rn->info = si;
  if (cp)
    cp->prev = si;
  si->prev = pp;
  si->next = cp;

  /* Install into rib. */
  static_install_ipv4 (p, si);

  return 1;
}

/* Delete static route from static route configuration. */
int
vice_static_delete_ipv6 (struct prefix *p, u_char type, struct in6_addr *gate,
		    const char *ifname, u_char distance, u_int32_t vrf_id)
{
  struct route_node *rn;
  struct static_ipv6 *si;
  struct route_table *stable;

  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP6, SAFI_UNICAST, vrf_id);
  if (! stable)
    return -1;

  /* Lookup static route prefix. */
  rn = route_node_lookup (stable, p);
  if (! rn)
    return 0;

  /* Find same static route is the tree */
  for (si = rn->info; si; si = si->next)
  {
  #if 0
    if (distance == si->distance 
	&& type == si->type
	&& (! gate || IPV6_ADDR_SAME (gate, &si->ipv6))
	&& (! ifname || strcmp (ifname, si->ifname) == 0))
      break;
  #else/*gujd: 2012-04-10,pm 5:08. Ignore the distance, bacaus when delete ipv6 route distance not packet from active master.(is set 0) */
	if (type == si->type
	&& (! gate || IPV6_ADDR_SAME (gate, &si->ipv6))
	&& (! ifname || strcmp (ifname, si->ifname) == 0))
      break;

	#endif
	
  	}

  /* Can't find static route. */
  if (! si)
    {
      route_unlock_node (rn);
      return 0;
    }

  /* Install into rib. */
  static_uninstall_ipv6 (p, si);

  /* Unlink static route from linked list. */
  if (si->prev)
    si->prev->next = si->next;
  else
    rn->info = si->next;
  if (si->next)
    si->next->prev = si->prev;
  
  /* Free static route configuration. */
  if (ifname)
    XFREE (0, si->ifname);
  XFREE (MTYPE_STATIC_IPV6, si);

  return 1;
}


/* Add static route into static route configuration. */
int
vice_static_add_ipv6 (struct prefix *p, u_char type, struct in6_addr *gate,
		 const char *ifname, u_char flags, u_char distance,
		 u_int32_t vrf_id)
{
  struct route_node *rn;
  struct static_ipv6 *si;
  struct static_ipv6 *pp;
  struct static_ipv6 *cp;
  struct route_table *stable;
  
  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP6, SAFI_UNICAST, vrf_id);
  if (! stable)
    return -1;
    
  if (!gate &&
      (type == STATIC_IPV6_GATEWAY || type == STATIC_IPV6_GATEWAY_IFNAME))
    return -1;
  
  if (!ifname && 
      (type == STATIC_IPV6_GATEWAY_IFNAME || type == STATIC_IPV6_IFNAME))
    return -1;

  /* Lookup static route prefix. */
  rn = route_node_get (stable, p);

  /* Do nothing if there is a same static route.  */
  for (si = rn->info; si; si = si->next)
    {
      if (distance == si->distance 
	  && type == si->type
	  && (! gate || IPV6_ADDR_SAME (gate, &si->ipv6))
	  && (! ifname || strcmp (ifname, si->ifname) == 0))
	{
	  route_unlock_node (rn);
	  return 0;
	}
    }

  /* Make new static route structure. */
  si = XMALLOC (MTYPE_STATIC_IPV6, sizeof (struct static_ipv6));
  memset (si, 0, sizeof (struct static_ipv6));

  si->type = type;
  si->distance = distance;
  si->flags = flags;

  switch (type)
    {
    case STATIC_IPV6_GATEWAY:
      si->ipv6 = *gate;
      break;
    case STATIC_IPV6_IFNAME:
      si->ifname = XSTRDUP (0, ifname);
      break;
    case STATIC_IPV6_GATEWAY_IFNAME:
      si->ipv6 = *gate;
      si->ifname = XSTRDUP (0, ifname);
      break;
    }

  /* Add new static route information to the tree with sort by
     distance value and gateway address. */
  for (pp = NULL, cp = rn->info; cp; pp = cp, cp = cp->next)
    {
      if (si->distance < cp->distance)
				break;
      if (si->distance > cp->distance)
				continue;
    }

  /* Make linked list. */
  if (pp)
    pp->next = si;
  else
    rn->info = si;
  if (cp)
    cp->prev = si;
  si->prev = pp;
  si->next = cp;

  /* Install into rib. */
  return static_install_ipv6 (p, si);

  return 1;
}

	
/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_add_ipv4_multipath), only for Static Route . First add the acive master
 static route to Static Route Table , sencond add to Rib Route Table.**/
int
vice_rib_ipv4_add_static_route (struct prefix *p, struct rib *rib)
{
  struct nexthop *nexthop;
   	
  if(tipc_server_debug)
    zlog_debug("enter func %s ....\n",__func__);
  if(!p || !rib)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib);
		return -1;
  	}
  
  if(rib->type == ZEBRA_ROUTE_STATIC)
  {
	 if(tipc_server_debug)
	  zlog_info("%s : rib->nexthop_num(%d)\n",__func__,rib->nexthop_num);
	 for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	  {
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
		if(tipc_server_debug)
		  zlog_info("%s : nexthop type (%d).\n",__func__,nexthop->type);
		if(nexthop->type == NEXTHOP_TYPE_IFINDEX
			||nexthop->type == NEXTHOP_TYPE_IFNAME)
		{
			if(nexthop->type== NEXTHOP_TYPE_IFINDEX)
			  nexthop->ifname = ifindex2ifname(nexthop->ifindex);
 	 		vice_static_add_ipv4(p, NULL,nexthop->ifname,rib->flags, rib->distance, 0);
			}
		
		else if(nexthop->type == NEXTHOP_TYPE_IPV4
				||nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX
				||nexthop->type == NEXTHOP_TYPE_IPV4_IFNAME)
		{
			vice_static_add_ipv4( p,&nexthop->gate.ipv4,NULL,rib->flags, rib->distance, 0);
			}
		else if(nexthop->type == NEXTHOP_TYPE_BLACKHOLE)
		{
			vice_static_add_ipv4( p,NULL,NULL,rib->flags, rib->distance, 0);
			}
		else
		{
			zlog_warn("%s : err next type (%d).\n",__func__,nexthop->type);
			return -1;
			}
  	   }
  	}
  if(tipc_server_debug)
    zlog_debug("leave func %s ....\n",__func__);
  return 0;
}



/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_delete_ipv4)**/
int
vice_rib_ipv4_detele_static_route(struct prefix *p, struct rib *rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);
  if(!p || !rib_del)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib_del);
		return -1;
  	}
  /* Lookup table.	*/
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
	return 0;
  
  if(tipc_server_debug)
	zlog_debug("type:%d;flags:%d; ifindex %d",rib_del->type,rib_del->flags,rib_del->nexthop->ifindex);

  /* Apply mask. */
  apply_mask_ipv4 ((struct prefix_ipv4 *)p);
  
  #if 0
  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
  #endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);/*rib node*/
  if (! rn)
  {
	zlog_debug("%s : line %d , Rib route node = NULL !!\n",__func__,__LINE__);
	  return -1;
	}
  /*CID 17185: Logically dead code (DEADCODE)
     dead_error_begin: Execution cannot reach this statement "zlog_warn("struct rib del i...".
     So delete it.*/
  /*
   if(!rib_del)
	{
		zlog_warn("struct rib del is NULL .\n");
		return -1;
	}
	*/

	if(rib_del->type == ZEBRA_ROUTE_STATIC)
	 {
		if(tipc_server_debug)
		  zlog_info("%s : rib->nexthop_num(%d)\n",__func__,rib_del->nexthop_num);
		for (nexthop = rib_del->nexthop; nexthop; nexthop = nexthop->next)
		{
			if(tipc_server_debug)
			  zlog_info("%s : nexthop type (%d).\n",__func__,nexthop->type);
			
			if(nexthop->type == NEXTHOP_TYPE_IFINDEX
				||nexthop->type == NEXTHOP_TYPE_IFNAME)
			{
				if(nexthop->type== NEXTHOP_TYPE_IFINDEX)
				  nexthop->ifname = ifindex2ifname(nexthop->ifindex);
	 	 		vice_static_delete_ipv4(p, NULL,nexthop->ifname, rib_del->distance, 0);
				}
			
			else if(nexthop->type == NEXTHOP_TYPE_IPV4
					||nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX
					||nexthop->type == NEXTHOP_TYPE_IPV4_IFNAME)
			{
				vice_static_delete_ipv4( p,&nexthop->gate.ipv4,NULL, rib_del->distance, 0);
				}
			else if(nexthop->type == NEXTHOP_TYPE_BLACKHOLE)
			{
				vice_static_delete_ipv4( p,NULL,NULL,rib_del->distance, 0);
				}
			else
			{
				zlog_warn("%s : err next type (%d)\n",__func__,nexthop->type);
				return -1;
				}
	  	  }
	  }

  return 0;
}
	
/*gjd : route info from the Active master . For vice or bakup master baord to add dymatic protocol route, like ospf. rip. isis. bgp*/
int
vice_rib_ipv4_add_protocol_route(struct prefix *p, struct rib *rib)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;
	
  if(tipc_server_debug)
    zlog_debug("enter func %s ....\n",__func__);
  if(!p || !rib)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib);
		return -1;
  	}
  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  /* Make it sure prefixlen is applied to the prefix. */
  apply_mask_ipv4 ((struct prefix_ipv4 *)p);

  /* Set default distance by route type. */
  if (rib->distance == 0)
    {    
	  if(tipc_server_debug)
       zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
      rib->distance = route_info_distance[rib->type].distance;

      /* iBGP distance is 200. */
      if (rib->type == ZEBRA_ROUTE_BGP && CHECK_FLAG (rib->flags, ZEBRA_FLAG_IBGP))
	  rib->distance = 200;
    } 
  
  if(tipc_server_debug)
  	zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);

  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (same = rn->info; same; same = same->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
      {
		if(tipc_server_debug)
		 zlog_debug("%s : line %d , rib->status = RIB_ENTRY_REMOVED\n",__func__,__LINE__);
        continue;
      }
      if (same->type == rib->type && same->table == rib->table
	  && same->type != ZEBRA_ROUTE_CONNECT)
      	{
		  if(tipc_server_debug)
      	    zlog_debug("%s : line %d ,same->type == rib->type && same->table == rib->table && same->type != ZEBRA_ROUTE_CONNECT..............\n ",__func__,__LINE__);
          break;
      	}
    }
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT)
  {
  
    if(tipc_server_debug)
  	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT....\n",
				__func__,__LINE__);
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
      SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
    }
  }
  if(rib->type == ZEBRA_ROUTE_RIP
  	||rib->type == ZEBRA_ROUTE_OSPF
  	||rib->type == ZEBRA_ROUTE_BGP
  	||rib->type == ZEBRA_ROUTE_ISIS)
  {
     if(tipc_server_debug)
  	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_STATIC....\n",__func__,__LINE__);
     for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	 {
		  /* 业务板把接收到的路由添加信息，因为业务板还没经过rib_process进行处理，没有加到业务板的fib表中，所以要把nexthop->flags的NEXTHOP_FLAG_FIB，从接收到的rib中去掉，通过自己的rib_process计算完，再添加*/
		  UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	   }
  }
#if 0/*when delete rib, use vice_rib_ipv4_detele*/
  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
  	rib_delnode(rn, rib);/**delete rib and add this event to workqueue**/
  else
#endif
  rib_addnode (rn, rib);/* Link new rib to node.*/

  /* Free implicit route.*/
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  if(tipc_server_debug)
    zlog_debug("leave func %s ....\n",__func__);
  return 0;
}

int
vice_rib_ipv4_detele_protocol_route(struct prefix *p, struct rib *rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);
  if(!p || !rib_del)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib_del);
		return -1;
  	}
  /* Lookup table.	*/
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
	return 0;
  
  if(tipc_server_debug)
	zlog_debug("type:%d;flags:%d; ifindex %d",rib_del->type,rib_del->flags,rib_del->nexthop->ifindex);

  /* Apply mask. */
  apply_mask_ipv4 ((struct prefix_ipv4 *)p);
  
  #if 0
  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
  #endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);/*Rib node*/
  if (! rn)
  {
	zlog_debug("%s : line %d , Rib route node = NULL !!\n",__func__,__LINE__);
	return -1;
	}
  
   /* Lookup same type route. */
  	for (rib = rn->info; rib; rib = rib->next)
	{
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

	  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
		fib = rib;

	  if (rib->type != rib_del->type)
		continue;
	  if(rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) 
	  	  &&nexthop->type == NEXTHOP_TYPE_IFINDEX 
	  	  && nexthop->ifindex == rib_del->nexthop->ifindex)
		{
		  if (rib->refcnt)
			{
			  rib->refcnt--;
			  route_unlock_node (rn);
			  route_unlock_node (rn);
			  return 0;
			}
		  same = rib;
		  break;
		}
	  /* Make sure that the route found has the same gateway. */
	   else if ((&(rib_del->nexthop->gate.ipv4)== NULL && (nexthop = rib->nexthop) && (rib_del->nexthop->ifindex== nexthop->ifindex))||
		   (&(rib_del->nexthop->gate.ipv4) && (nexthop = rib->nexthop)&&	
		   (IPV4_ADDR_SAME (&(nexthop->gate.ipv4), &(rib_del->nexthop->gate.ipv4)) || IPV4_ADDR_SAME (&(nexthop->rgate.ipv4), &(rib_del->nexthop->gate.ipv4))))) 
		{
			same = rib;

			if(tipc_server_debug)
			  zlog_debug ("%s: rn %p, rib:same is %p, with prefix %s/%d ", __func__, rn, same,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
			
			if(tipc_server_debug)
			  zlog_debug ("rib same:nexthop %p,include: nexthop->type %d, nexthop->ifindex %d ,nexthop->ifname %p", same->nexthop, same->nexthop->type,
																		same->nexthop->ifindex,same->nexthop->ifname);
			break;
		}
	}

  /* If same type of route can't be found and this message is from
	 kernel. */
  if (! same)
	{
	  if (fib && rib_del->type== ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
	  else
	{
	
	#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
		{
		  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex,
			   type);
		  else
		zlog_debug ("route %s/%d ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex,
			   type);
		}
	  #endif
	  route_unlock_node (rn);
/*	  return ZEBRA_ERR_RTNOEXIST;*/
	}
	}

  if (tipc_server_debug)
	zlog_debug ("%s: check delnode ", __func__);
  
  if (same)
	rib_delnode (rn, same);

  route_unlock_node (rn);
  return 0;
}


/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_add_ipv4_multipath)**/
int
vice_rib_ipv4(int cmd,struct prefix *p, struct rib *rib)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;

  if(tipc_server_debug)
    zlog_debug("enter func %s ....\n",__func__);
  
  if(cmd == ZEBRA_IPV4_ROUTE_ADD)
  	{
  		if(rib->type == ZEBRA_ROUTE_STATIC)/*add static ipv4 route*/
  		{
  			vice_rib_ipv4_add_static_route( p, rib);
  		}
  		else/*add protocol route , like ospf, rip, isis , bgp .*/
  		{
  			vice_rib_ipv4_add_protocol_route( p, rib);
  		}
  	}
  else
  	if(cmd == ZEBRA_IPV4_ROUTE_DELETE)
  	{
  		if(rib->type == ZEBRA_ROUTE_STATIC)/*delete static ipv4 route*/
  		{
  			vice_rib_ipv4_detele_static_route( p,rib);
  		}
  		else/*delete protocol route , like ospf, rip, isis , bgp .*/
  		{
  			vice_rib_ipv4_detele_protocol_route( p, rib);
  		}
  	}
  else
  	{
  		zlog_warn("%s : error command(%d).\n",__func__,cmd);
		return -1;
  	}

  if(tipc_server_debug)
    zlog_debug("leave func %s ....\n",__func__);
  return 0;
}


int
vice_rib_ipv6_delete_static_route(struct prefix * p,struct rib * rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);
  if(!p || !rib_del)
   {
   		zlog_warn("%s : p(%p),rib_del(%p) is null.\n",__func__,p,rib_del);
		return -1;
  	}

  /* Lookup table.	*/
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
	return 0;
  if(tipc_server_debug)
	zlog_debug("type:%d;flags:%d; ifindex %d",rib_del->type,rib_del->flags,rib_del->nexthop->ifindex);

  /* Apply mask. */
  apply_mask_ipv6 ((struct prefix_ipv6 *)p);
#if 0

  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
#endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
	{
	  zlog_debug("%s : line %d ,Rib(v6) route node = NULL !!\n",__func__,__LINE__);
	  return -1;
	}
  
  /*CID 17186: Logically dead code (DEADCODE)
    dead_error_begin: Execution cannot reach this statement "zlog_warn("struct rib del i...". 
    So delete it.*/
 /*  
       if(!rib_del)
	  {
		  zlog_warn("struct rib del is NULL .\n");
		  return -1;
	  }
 */
  
   if(rib_del->type == ZEBRA_ROUTE_STATIC)
   {
	  if(tipc_server_debug)
		zlog_info("%s : rib->nexthop_num(%d)\n",__func__,rib_del->nexthop_num);
	  for (nexthop = rib_del->nexthop; nexthop; nexthop = nexthop->next)
	  {
		  if(tipc_server_debug)
			zlog_info("%s : nexthop type (%d).\n",__func__,nexthop->type);
		  
		  if(nexthop->type == NEXTHOP_TYPE_IFINDEX
			  ||nexthop->type == NEXTHOP_TYPE_IFNAME)
		  {
			  if(nexthop->type== NEXTHOP_TYPE_IFINDEX)
				nexthop->ifname = ifindex2ifname(nexthop->ifindex);/*gateway is : ifname*/
			  vice_static_delete_ipv6(p,STATIC_IPV6_IFNAME, NULL,nexthop->ifname, rib_del->distance, 0);
			  }
		  
		  else if(nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX
				  ||nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
		  {
		  	  if(nexthop->type== NEXTHOP_TYPE_IPV6_IFINDEX)
				nexthop->ifname = ifindex2ifname(nexthop->ifindex);/*gateway is: x.x.x.x/m ifname*/
			  vice_static_delete_ipv6( p,STATIC_IPV6_GATEWAY_IFNAME,&nexthop->gate.ipv6,nexthop->ifname, rib_del->distance, 0);
			  }
		  else if(nexthop->type == NEXTHOP_TYPE_IPV6)/*gateway is : x.x.x.x/m*/
		  {
		  	   vice_static_delete_ipv6( p,STATIC_IPV6_GATEWAY,&nexthop->gate.ipv6,NULL,rib_del->distance, 0);
		  }
		  else if(nexthop->type == NEXTHOP_TYPE_BLACKHOLE)
		  {
			  vice_static_delete_ipv6( p,0,NULL,NULL,rib_del->distance, 0);
			  }
		  else
		  {
			  zlog_warn("%s : err next type (%d)\n",__func__,nexthop->type);
			  return -1;
			  }
		}
	}

  
  return 0;
}

int
vice_rib_ipv6_add_static_route(struct prefix * p,struct rib * rib)
{
  struct nexthop *nexthop;

  if(tipc_server_debug)
	zlog_debug("enter func %s ....\n",__func__);
  if(!p || !rib)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib);
		return -1;
  	}
  
  if(rib->type == ZEBRA_ROUTE_STATIC)
  {
	 if(tipc_server_debug)
	  zlog_info("%s : rib->nexthop_num(%d)\n",__func__,rib->nexthop_num);
	 for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	  {
		 UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
		  if(tipc_server_debug)
			zlog_info("%s : nexthop type (%d).\n",__func__,nexthop->type);
		  
		  if(nexthop->type == NEXTHOP_TYPE_IFINDEX
			  ||nexthop->type == NEXTHOP_TYPE_IFNAME)
		  {
			  if(nexthop->type== NEXTHOP_TYPE_IFINDEX)
				nexthop->ifname = ifindex2ifname(nexthop->ifindex);/*gateway is : ifname*/
			  vice_static_add_ipv6(p,STATIC_IPV6_IFNAME, NULL,nexthop->ifname,rib->flags, rib->distance, 0);
			  }
		  
		  else if(nexthop->type == NEXTHOP_TYPE_IPV6_IFINDEX
				  ||nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
		  {
		  	  if(nexthop->type== NEXTHOP_TYPE_IPV6_IFINDEX)
				nexthop->ifname = ifindex2ifname(nexthop->ifindex);/*gateway is: x.x.x.x/m ifname*/
			  vice_static_add_ipv6( p,STATIC_IPV6_GATEWAY_IFNAME,&nexthop->gate.ipv6,nexthop->ifname,rib->flags, rib->distance, 0);
			  }
		  else if(nexthop->type == NEXTHOP_TYPE_IPV6)/*gateway is : x.x.x.x/m*/
		  {
		  	   vice_static_add_ipv6( p,STATIC_IPV6_GATEWAY,&nexthop->gate.ipv6,NULL,rib->flags,rib->distance, 0);
		  }
		  else if(nexthop->type == NEXTHOP_TYPE_BLACKHOLE)
		  {
			  vice_static_add_ipv6( p,0,NULL,NULL,rib->flags,rib->distance, 0);
			  }
		  else
		  {
			  zlog_warn("%s : err next type (%d)\n",__func__,nexthop->type);
			  return -1;
			  }
		}
	}
  if(tipc_server_debug)
	zlog_debug("leave func %s ....\n",__func__);
  return 0;
}


int
vice_rib_ipv6_delete_protocol_route(struct prefix * p,struct rib * rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);
  if(!p || !rib_del)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib_del);
		return -1;
  	}

  /* Lookup table.	*/
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
	return 0;
	if(tipc_server_debug)
		zlog_debug("type:%d;flags:%d; ifindex %d",rib_del->type,rib_del->flags,rib_del->nexthop->ifindex);

  /* Apply mask. */
  apply_mask_ipv6 ((struct prefix_ipv6 *)p);
#if 0

  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
#endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
  {
	zlog_debug("%s : line %d , route node = NULL !!\n",__func__,__LINE__);
	return -1;
   }
  if(!rn->info)
  {
  	 zlog_err("%s : line %drn info is null .\n",__func__,__LINE__);
	 return -1;
  	}

  /* Lookup same type route. */
  for (rib = rn->info; rib; rib = rib->next)
	{
	
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

	  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	fib = rib;

	  if (rib->type != rib_del->type)
	continue;
	  if (rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) &&
	  nexthop->type == NEXTHOP_TYPE_IFINDEX && nexthop->ifindex == rib_del->nexthop->ifindex)
	{
	
	  if (rib->refcnt)
		{
		
		  rib->refcnt--;
		  route_unlock_node (rn);
		  route_unlock_node (rn);
		  return 0;
		}
	  same = rib;
	  break;
	}
	  /* Make sure that the route found has the same gateway. */
	  else if ((&(rib_del->nexthop->gate.ipv6)== NULL && (nexthop = rib->nexthop) && (rib_del->nexthop->ifindex== nexthop->ifindex))||
		   (&(rib_del->nexthop->gate.ipv6) && (nexthop = rib->nexthop)&&	
		   (IPV6_ADDR_SAME (&(nexthop->gate.ipv6), &(rib_del->nexthop->gate.ipv6)) || IPV6_ADDR_SAME (&(nexthop->rgate.ipv6), &(rib_del->nexthop->gate.ipv6))))) 
		{
			same = rib;

			if(tipc_server_debug)
			  zlog_info ("rib same:nexthop %p,include: nexthop->type %d, nexthop->ifindex %d ,nexthop->ifname %p", same->nexthop, same->nexthop->type,
																		same->nexthop->ifindex,same->nexthop->ifname);
			break;
		}
	}

  /* If same type of route can't be found and this message is from
	 kernel. */
  if (! same)
	{
	  if (fib && rib_del->type== ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
	  else
	{
	
#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
		{
		  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex,
			   type);
		  else
		zlog_debug ("route %s/%d ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex,
			   type);
		}
  #endif
	  route_unlock_node (rn);
/*	  return ZEBRA_ERR_RTNOEXIST;*/
	}
	}

  if (tipc_server_debug)
	zlog_debug ("%s: check delnode ", __func__);

  if (same)
	rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

int
vice_rib_ipv6_add_protocol_route(struct prefix * p,struct rib *rib)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;
  

  if(tipc_server_debug)
	zlog_debug("enter func %s ....\n",__func__);
  if(!p || !rib)
   {
   		zlog_warn("%s : p(%p),rib(%p) is null.\n",__func__,p,rib);
		return -1;
  	}
  
  /* Lookup table.	*/
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
	return 0;

  /* Make sure mask is applied. */
  apply_mask_ipv6 ((struct prefix_ipv6 *)p);

  /* Set default distance by route type. */
  if (rib->distance == 0)
	{	 
	  if(tipc_server_debug)
	   zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
	  rib->distance = route_info_distance[rib->type].distance;

	  /* iBGP distance is 200. */
	  if (rib->type == ZEBRA_ROUTE_BGP && CHECK_FLAG (rib->flags, ZEBRA_FLAG_IBGP))
	  rib->distance = 200;
	} 
  
  if(tipc_server_debug)
	zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
#if 0
  /* Filter bogus route. */
  if (rib_bogus_ipv6(rib->type,(struct prefix_ipv6 *)p, rib->nexthop->gate.ipv6, rib->nexthop->ifindex, 0))
  	{
  		zlog_info("%s : line %d, rib_bogus_ipv6!!!\n",__func__,__LINE__);
		return 0;
  	}
  #endif
  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
	 withdraw. */
  for (same = rn->info; same; same = same->next)
	{
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	  {
		zlog_debug("%s : line %d , rib->status = RIB_ENTRY_REMOVED\n",__func__,__LINE__);
		continue;
		}
	  
	  if (same->type == rib->type && same->table == rib->table
	  && same->type != ZEBRA_ROUTE_CONNECT)
		{
		  if(tipc_server_debug)
			zlog_debug("%s : line %d ,same->type == rib->type && same->table == rib->table && same->type != ZEBRA_ROUTE_CONNECT..............\n ",__func__,__LINE__);
		  break;
		}
	}
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT)
  {
  
	if(tipc_server_debug)
	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT....\n",
				__func__,__LINE__);
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
	}
  }
  
  if(rib->type == ZEBRA_ROUTE_RIPNG
	  ||rib->type == ZEBRA_ROUTE_OSPF6
	  ||rib->type == ZEBRA_ROUTE_BGP
	  ||rib->type == ZEBRA_ROUTE_ISIS)
	{
  
	if(tipc_server_debug)
	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_STATIC....\n",__func__,__LINE__);
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  /* 业务板把接收到的路由添加信息，因为业务板还没经过rib_process进行处理，没有加到业务板的fib表中，所以要把nexthop->flags的NEXTHOP_FLAG_FIB，从接收到的rib中去掉，通过自己的rib_process计算完，再添加*/
	  UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
	}
  }
#if 0/*when delete rib, use vice_rib_ipv4_detele*/
  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	rib_delnode(rn, rib);/**delete rib and add this event to workqueue**/
  else
#endif

	rib_addnode (rn, rib);/* Link new rib to node.*/
  
  /* Free implicit route.*/
  if (same)
	rib_delnode (rn, same);
  
  route_unlock_node (rn);
  
  if(tipc_server_debug)
	zlog_debug("leave func %s ....\n",__func__);
  return 0;
}


/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_add_ipv4_multipath)**/
int
vice_rib_ipv6(int cmd,struct prefix *p, struct rib *rib)/*change name at 2011-7-25, before is zebra_rib_ipv4_multipath */
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;

  if(tipc_server_debug)
    zlog_debug("enter func %s ....\n",__func__);
  
  if(cmd == ZEBRA_IPV6_ROUTE_ADD)
  	{
  		if(rib->type == ZEBRA_ROUTE_STATIC)/*add static ipv4 route*/
  		{
  			vice_rib_ipv6_add_static_route( p, rib);
  		}
  		else/*add protocol route , like ospf, rip, isis , bgp .*/
  		{
  			vice_rib_ipv6_add_protocol_route( p, rib);
  		}
  	}
  else
  	if(cmd == ZEBRA_IPV6_ROUTE_DELETE)
  	{
  		if(rib->type == ZEBRA_ROUTE_STATIC)/*delete static ipv6 route*/
  		{
  			vice_rib_ipv6_delete_static_route( p,rib);
  		}
  		else/*delete protocol route , like ospf6, ripng*/
  		{
  			vice_rib_ipv6_delete_protocol_route( p, rib);
  		}
  	}
  else
  	{
  		zlog_warn("%s : error command(%d).\n",__func__,cmd);
		return -1;
  	}

  if(tipc_server_debug)
    zlog_debug("leave func %s ....\n",__func__);
  return 0;
}

#endif

/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_delete_ipv4)**/
int
vice_rib_ipv4_detele(struct prefix *p, struct rib *rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);

  /* Lookup table.	*/
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
	return 0;
  
  if(tipc_server_debug)
	zlog_debug(" type:%d; flags:%d.",rib_del->type,rib_del->flags);

  /* Apply mask. */
  apply_mask_ipv4 ((struct prefix_ipv4 *)p);
  #if 0

  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
  #endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
	{
	zlog_debug("%s : line %d , route node = NULL !!\n",__func__,__LINE__);
	return -1;
	#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
	{
	  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex);
	  else
		zlog_debug ("route %s/%d ifindex %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex);
	}
	  return ZEBRA_ERR_RTNOEXIST;
	  #endif
	}

  /* Lookup same type route. */
  for (rib = rn->info; rib; rib = rib->next)
	{
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

	  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
		fib = rib;

	  if (rib->type != rib_del->type)
		continue;
	  if (rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) &&
	  nexthop->type == NEXTHOP_TYPE_IFINDEX && nexthop->ifindex == rib_del->nexthop->ifindex)
	{
	  if (rib->refcnt)
		{
		  rib->refcnt--;
		  route_unlock_node (rn);
		  route_unlock_node (rn);
		  return 0;
		}
	  same = rib;
	  break;
	}
	  /* Make sure that the route found has the same gateway. */
	  else if ((&(rib_del->nexthop->gate.ipv4)== NULL && (nexthop = rib->nexthop) && (rib_del->nexthop->ifindex== nexthop->ifindex))||
		   (&(rib_del->nexthop->gate.ipv4) && (nexthop = rib->nexthop)&&	
		   (IPV4_ADDR_SAME (&(nexthop->gate.ipv4), &(rib_del->nexthop->gate.ipv4)) || IPV4_ADDR_SAME (&(nexthop->rgate.ipv4), &(rib_del->nexthop->gate.ipv4))))) 
		{
			same = rib;

			if(tipc_server_debug)
			  zlog_debug ("%s: rn %p, rib:same is %p, with prefix %s/%d ", __func__, rn, same,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
			
			if(tipc_server_debug)
			  zlog_debug ("rib same:nexthop %p,include: nexthop->type %d, nexthop->ifindex %d ,nexthop->ifname %p", same->nexthop, same->nexthop->type,
																		same->nexthop->ifindex,same->nexthop->ifname);
			break;
		}
	}

  /* If same type of route can't be found and this message is from
	 kernel. */
  if (! same)
	{
	  if (fib && rib_del->type== ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
	  else
	{
	
	#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
		{
		  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex,
			   type);
		  else
		zlog_debug ("route %s/%d ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex,
			   type);
		}
	  #endif
	  route_unlock_node (rn);
/*	  return ZEBRA_ERR_RTNOEXIST;*/
	}
	}

  if (tipc_server_debug)
	zlog_debug ("%s: check delnode ", __func__);

  if (same)
	rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}
	

/**add by gjd: In fact, client use this func when recv route info from server, 
 and tell client zebra , by client zebra tell client kernel(use rib_process),
 this api like(rib_add_ipv4_multipath)**/
int
vice_rib_ipv4_add (struct prefix *p, struct rib *rib)/*change name at 2011-7-25, before is zebra_rib_ipv4_multipath */
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;

  if(tipc_server_debug)
    zlog_debug("enter func %s ....\n",__func__);
  
  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  /* Make it sure prefixlen is applied to the prefix. */
  apply_mask_ipv4 ((struct prefix_ipv4 *)p);

  /* Set default distance by route type. */
  if (rib->distance == 0)
    {    
	  if(tipc_server_debug)
       zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
      rib->distance = route_info_distance[rib->type].distance;

      /* iBGP distance is 200. */
      if (rib->type == ZEBRA_ROUTE_BGP && CHECK_FLAG (rib->flags, ZEBRA_FLAG_IBGP))
	  rib->distance = 200;
    } 
  
  if(tipc_server_debug)
  	zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);

  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (same = rn->info; same; same = same->next)
    {
    #if 1
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
      	{
      	
		zlog_debug("%s : line %d , rib->status = RIB_ENTRY_REMOVED\n",__func__,__LINE__);
        continue;
      	}
	#endif
      if (same->type == rib->type && same->table == rib->table
	  && same->type != ZEBRA_ROUTE_CONNECT)
      	{
		  if(tipc_server_debug)
      	    zlog_debug("%s : line %d ,same->type == rib->type && same->table == rib->table && same->type != ZEBRA_ROUTE_CONNECT..............\n ",__func__,__LINE__);
          break;
      	}
    }
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT)
  {
  
    if(tipc_server_debug)
  	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT....\n",
				__func__,__LINE__);
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
      SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
    }
  }
  if(rib->type == ZEBRA_ROUTE_STATIC)
  	{
  
    if(tipc_server_debug)
  	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_STATIC....\n",__func__,__LINE__);
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
	  /* 业务板把接收到的路由添加信息，因为业务板还没经过rib_process进行处理，没有加到业务板的fib表中，所以要把nexthop->flags的NEXTHOP_FLAG_FIB，从接收到的rib中去掉，通过自己的rib_process计算完，再添加*/
	  UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	 /* zlog_debug("%s : line %d ,UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);*/
    }
  }
#if 0/*when delete rib, use vice_rib_ipv4_detele*/
  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
  	rib_delnode(rn, rib);/**delete rib and add this event to workqueue**/
  else
#endif
  	rib_addnode (rn, rib);/* Link new rib to node.*/
  
  /* Free implicit route.*/
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  
  if(tipc_server_debug)
    zlog_debug("leave func %s ....\n",__func__);
  return 0;
}

int
vice_rib_ipv6_add(struct prefix * p,struct rib * rib)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;

  if(tipc_server_debug)
	zlog_debug("enter func %s ....\n",__func__);
  
  /* Lookup table.	*/
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
	return 0;

  /* Make sure mask is applied. */
  apply_mask_ipv6 ((struct prefix_ipv6 *)p);

  /* Set default distance by route type. */
  if (rib->distance == 0)
	{	 
	  if(tipc_server_debug)
	   zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
	  rib->distance = route_info_distance[rib->type].distance;

	  /* iBGP distance is 200. */
	  if (rib->type == ZEBRA_ROUTE_BGP && CHECK_FLAG (rib->flags, ZEBRA_FLAG_IBGP))
	  rib->distance = 200;
	} 
  
  if(tipc_server_debug)
	zlog_debug("%s : line %d , distance = %u....\n",__func__,__LINE__,rib->distance);
#if 0
  /* Filter bogus route. */
  if (rib_bogus_ipv6(rib->type,(struct prefix_ipv6 *)p, rib->nexthop->gate.ipv6, rib->nexthop->ifindex, 0))
  	{
  		zlog_info("%s : line %d, rib_bogus_ipv6!!!\n",__func__,__LINE__);
		return 0;
  	}
  #endif
  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
	 withdraw. */
  for (same = rn->info; same; same = same->next)
	{
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		{
		
		zlog_debug("%s : line %d , rib->status = RIB_ENTRY_REMOVED\n",__func__,__LINE__);
		continue;
		}
	  
	  if (same->type == rib->type && same->table == rib->table
	  && same->type != ZEBRA_ROUTE_CONNECT)
		{
		  if(tipc_server_debug)
			zlog_debug("%s : line %d ,same->type == rib->type && same->table == rib->table && same->type != ZEBRA_ROUTE_CONNECT..............\n ",__func__,__LINE__);
		  break;
		}
	}
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT)
  {
  
	if(tipc_server_debug)
	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT....\n",
				__func__,__LINE__);
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
	}
  }
  
  if(rib->type == ZEBRA_ROUTE_STATIC)
	{
  
	if(tipc_server_debug)
	  zlog_debug("%s : line %d ,(rib->type == ZEBRA_ROUTE_STATIC....\n",__func__,__LINE__);
	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  /* 业务板把接收到的路由添加信息，因为业务板还没经过rib_process进行处理，没有加到业务板的fib表中，所以要把nexthop->flags的NEXTHOP_FLAG_FIB，从接收到的rib中去掉，通过自己的rib_process计算完，再添加*/
	  UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  zlog_debug("%s : line %d ,UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)...\n",__func__,__LINE__);
	}
  }
#if 0/*when delete rib, use vice_rib_ipv4_detele*/
  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	rib_delnode(rn, rib);/**delete rib and add this event to workqueue**/
  else
#endif

	rib_addnode (rn, rib);/* Link new rib to node.*/
  
  /* Free implicit route.*/
  if (same)
	rib_delnode (rn, same);
  
  route_unlock_node (rn);
  
  if(tipc_server_debug)
	zlog_debug("leave func %s ....\n",__func__);
  return 0;
}



int
vice_rib_ipv6_delete(struct prefix * p,struct rib * rib_del)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (tipc_server_debug)
	zlog_debug ("%s: start ", __func__);

  /* Lookup table.	*/
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
	return 0;
	if(tipc_server_debug)
		zlog_debug("type:%d;flags:%d; ifindex %d",rib_del->type,rib_del->flags,rib_del->nexthop->ifindex);

  /* Apply mask. */
  apply_mask_ipv6 ((struct prefix_ipv6 *)p);
#if 0

  if (IS_ZEBRA_DEBUG_RIB && nexthop->gate.ipv4)
	zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen, 
			   inet_ntoa (*nexthop->gate.ipv4), 
			   ifindex);
#endif

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
	{
	zlog_debug("%s : line %d , route node = NULL !!\n",__func__,__LINE__);
	return -1;
#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
	{
	  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex);
	  else
		zlog_debug ("route %s/%d ifindex %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex);
	}
	  return ZEBRA_ERR_RTNOEXIST;
  #endif
	}
  if(!rn->info)
  	{
  		zlog_err("rn info is null .\n");
		return -1;
  	}

  /* Lookup same type route. */
  for (rib = rn->info; rib; rib = rib->next)
	{
	
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;

	  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	fib = rib;

	  if (rib->type != rib_del->type)
	continue;
	  if (rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) &&
	  nexthop->type == NEXTHOP_TYPE_IFINDEX && nexthop->ifindex == rib_del->nexthop->ifindex)
	{
	
	  if (rib->refcnt)
		{
		
		  rib->refcnt--;
		  route_unlock_node (rn);
		  route_unlock_node (rn);
		  return 0;
		}
	  same = rib;
	  break;
	}
	  /* Make sure that the route found has the same gateway. */
	  else if ((&(rib_del->nexthop->gate.ipv6)== NULL && (nexthop = rib->nexthop) && (rib_del->nexthop->ifindex== nexthop->ifindex))||
		   (&(rib_del->nexthop->gate.ipv6) && (nexthop = rib->nexthop)&&	
		   (IPV6_ADDR_SAME (&(nexthop->gate.ipv6), &(rib_del->nexthop->gate.ipv6)) || IPV6_ADDR_SAME (&(nexthop->rgate.ipv6), &(rib_del->nexthop->gate.ipv6))))) 
		{
			same = rib;

			if(tipc_server_debug)
			  zlog_info ("rib same:nexthop %p,include: nexthop->type %d, nexthop->ifindex %d ,nexthop->ifname %p", same->nexthop, same->nexthop->type,
																		same->nexthop->ifindex,same->nexthop->ifname);
			break;
		}
	}

  /* If same type of route can't be found and this message is from
	 kernel. */
  if (! same)
	{
	  if (fib && rib_del->type== ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
	  else
	{
	
#if 0
	  if (IS_ZEBRA_DEBUG_RIB)
		{
		  if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET, gate, buf2, BUFSIZ),
			   ifindex,
			   type);
		  else
		zlog_debug ("route %s/%d ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex,
			   type);
		}
  #endif
	  route_unlock_node (rn);
/*	  return ZEBRA_ERR_RTNOEXIST;*/
	}
	}

  if (tipc_server_debug)
	zlog_debug ("%s: check delnode ", __func__);

  if (same)
	rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}



/**add by gjd : client parse the packet(stream buf) from server , 
  and use api(zebra_rib_ipv4_multipath),to tell client zebra ,
  by clent zebra to tell client kernel.This func support 
  multiple nexthop. like zread_ipv4_add**/

/*Recently : 2011-9-16, gjd, add for code for support the ipv6.*/
static int
tipc_client_route_multipath (int cmd, tipc_server *vice_board, u_short length)
	{
	  int i;
	  struct rib *rib;
	/* struct prefix_ipv4 p;*/
	  struct prefix p;
	  u_char message;
	/* struct in_addr nexthop;*/
	  struct in_addr *ipv4_gate;
   	  struct in6_addr *ipv6_gate;
	  u_char nexthop_num = 0;
	  u_char nexthop_type;
	  struct stream *s;
	  unsigned int ifindex;
	/*u_char ifnamelen;*/
	  u_char nexthop_flags;
	  char buf1[BUFSIZ] = {0};/*for debug*/
	  char buf2[BUFSIZ] = {0};
	  char ifname[INTERFACE_NAMSIZ];
	  int ret = 0;
	  int set_local_count = 0;
	
		
/*	  if(tipc_server_debug)
		zlog_debug ("%s: line %d, go to parse the stream from tipc server and zclient %p......\n", __func__,__LINE__,vice_board);
*/		
	  /* Get input stream.	*/
	  s = vice_board->ibuf;
	
	  /* Allocate new rib. */
	  rib = XCALLOC (MTYPE_RIB, sizeof(struct rib));
	  if(!rib)
	   {
	      zlog_warn("%s: malloc rib failed!\n",__func__);
		  return -1;
	  	}
	  /* Type, flags, message. */
	  rib->type = stream_getc (s);
	  if(tipc_server_debug)
		zlog_debug("%s : line %d ,rib->type = %d\n",__func__,__LINE__,rib->type);
	  rib->flags = stream_getc (s);
	  if(tipc_server_debug)
		zlog_debug("%s : line %d ,rib->flags = 0x%x\n",__func__,__LINE__,rib->flags);
	  rib->status = stream_getc(s);
	  if(tipc_server_debug)
		zlog_debug("%s : line %d ,rib->status = %u \n",__func__,__LINE__,rib->status);
	  message = stream_getc (s); 
	  if(tipc_server_debug)
		zlog_debug("%s : line %d ,message = %u\n",__func__,__LINE__,message);
	  rib->uptime = time (NULL);
	
	  memset (&p, 0, sizeof (struct prefix));
	  p.family = stream_getc(s);
	  p.prefixlen = stream_getc (s);
	  if(p.family == AF_INET)
	  	{
	  		stream_get (&p.u.prefix4, s, PSIZE (p.prefixlen));
	 		if(tipc_server_debug)
			  zlog_debug("%s : line %d, recv packet dest ipv4 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET,&p.u.prefix4,buf1,BUFSIZ),p.prefixlen);
	  	}
	  else if(p.family == AF_INET6)
	  	{
	  		stream_get (&p.u.prefix6, s, PSIZE (p.prefixlen));
	  		if(tipc_server_debug)
				zlog_debug("%s : line %d, recv packet dest ipv6 %s/%d \n",
					__func__,__LINE__,inet_ntop(AF_INET6,&p.u.prefix6,buf1,BUFSIZ),p.prefixlen);
	  	}
		else
		{
			zlog_info("%s : line %d ,unkown protocol .\n",__func__,__LINE__);
			
			/*CID 13543 (#1 of 4): Resource leak (RESOURCE_LEAK)
			9. leaked_storage: Variable "rib" going out of scope leaks the storage it points to.
			Add free rib struct.*/			
			XFREE(MTYPE_RIB,rib);/*add*/
			return -1;
		}
	  
	  if(p.family == AF_INET)
	  	ipv4_gate = XCALLOC(MTYPE_IN_ADD,sizeof(struct in_addr));
	  else
	  	ipv6_gate = XCALLOC(MTYPE_IN6_ADD,sizeof(struct in6_addr));
	  
	  /* Nexthop parse. */
	  if (CHECK_FLAG (message, ZAPI_MESSAGE_NEXTHOP))
	  {
		  nexthop_num = stream_getc (s);/*fetch nexthop count*/
		  if(tipc_server_debug)
			zlog_debug("%s: line %d, recv %d nexthops \n",__func__,__LINE__,nexthop_num);
		  
		  for (i = 0; i < nexthop_num; i++)
		{
		  nexthop_flags = stream_getc (s); /*fetch nexthop flags*/
		  if(tipc_server_debug)
			zlog_debug("%s : line %d ,recv nexthop->flags = 0x%x.... \n",__func__,__LINE__,nexthop_flags);
			  
		  nexthop_type = stream_getc (s);/*fetch nexthop type and used for check more info*/
		  if(tipc_server_debug)
			zlog_debug("%s : line %d, recv nexthop type %d ....\n",__func__,__LINE__,nexthop_type);
		  
		  switch(nexthop_type)/**nexthop_type add to rib->nexthop->type in api (like: nexthop_ifindex_add ), 
									nexthop_ipv4_add, and so on, so need not add alonely**/
		  {
	
			case NEXTHOP_TYPE_IFINDEX:/**for directly connected route when know ifname**/
			case NEXTHOP_TYPE_IFNAME:
	//			ifnamelen = stream_getc(s);/*fetch ifnamelen*/
	//			stream_get(ifname,s,ifnamelen);/*fetch ifname*/
				memset (ifname, 0, INTERFACE_NAMSIZ);
				stream_get(ifname,s,(size_t)INTERFACE_NAMSIZ);/*fetch ifname,20 byte*/
				/*gujd : 2013-01-10, pm 4:34 . Add for interface local used in route.*/
				ret = check_interface_belong_to_local_board_set_local_mode(ifname);
				if(ret == 1&&(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD))/*change*/
				 {
				 	zlog_debug("%s: line %d, nexthop interface(%s) set local, not to add rib.\n",__func__,__LINE__,ifname);
					/*break;*/
					/*return;*/
					set_local_count++;
					break;
					}
				ifindex = ifname2ifindex_by_namelen(ifname,strnlen(ifname, INTERFACE_NAMSIZ));/*ifname-->ifindex, tipc_client send ifindex to kernel by netlink*/
				nexthop_ifindex_flags_add(rib,ifindex,nexthop_flags);/*malloc a nexthop and put ifindex in nexthop*/
		
				if(tipc_server_debug)
				  zlog_debug("%s : line %d,fetch ifname %s, and ifname ==> ifindex %d ,add to rib-->nexthop->ifindex %d ... ...sucess..\n",
									__func__,__LINE__,ifname,ifindex,rib->nexthop->ifindex);
				  break;
				
			 case NEXTHOP_TYPE_IPV4:
			 case NEXTHOP_TYPE_IPV4_IFINDEX:
			 //  ipv4_gate.s_addr = stream_get_ipv4(s);
			 //  nexthop_ipv4_add(rib, ipv4_gate);/**add ipv4 gate to nexthop gate**/
				stream_get(ipv4_gate,s,4);/*fetch ipv4 gate size = 4 byte*/ 		  
			//	  ifnamelen = stream_getc(s);
				memset (ifname, 0, INTERFACE_NAMSIZ);
				stream_get(ifname,s,(size_t)INTERFACE_NAMSIZ);
				/*gujd : 2013-01-10, pm 4:34 . Add for interface local used in route.*/
				ret = check_interface_belong_to_local_board_set_local_mode(ifname);
				if(ret == 1&&(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD))/*change*/
				 {
				 	zlog_debug("%s: line %d, nexthop interface(%s) set local, not to add rib.\n",__func__,__LINE__,ifname);
					/*break;*/
					/*return 0;*/
					set_local_count++;
					break;
					}
				ifindex = ifname2ifindex_by_namelen(ifname,strnlen(ifname, INTERFACE_NAMSIZ));;
				nexthop_ipv4_ifindex_flags_add(rib,ipv4_gate,ifindex,nexthop_flags);/**put ifindex and ipv4 gate to nexthop**/								
				if(tipc_server_debug)
				   zlog_debug("%s : line %d,fetch  ipv4 %s , ifname %s ==>ifindex %d,add to rib->nexthop->ifindex %d ... ...OK\n",
								   __func__,__LINE__,inet_ntop(AF_INET,ipv4_gate,buf2,BUFSIZ),ifname,ifindex,rib->nexthop->ifindex);
				  break;
	
			 case NEXTHOP_TYPE_IPV4_IFNAME: 		
					
			//	ipv4_gate.s_addr = stream_get_ipv4(s);
				stream_get(ipv4_gate,s,4);
			//	ifnamelen = stream_getc(s);
			//	stream_get(ifname,s,ifnamelen); 
				memset (ifname, 0, 16);
				stream_get(ifname,s,(size_t)INTERFACE_NAMSIZ);
				/*gujd : 2013-01-10, pm 4:34 . Add for interface local used in route.*/
				ret = check_interface_belong_to_local_board_set_local_mode(ifname);
				if(ret == 1&&(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD))/*change*/
				 {
				 	zlog_debug("%s: line %d, nexthop interface(%s) set local, not to add rib.\n",__func__,__LINE__,ifname);
					/*break;*/
					set_local_count++;
					break;
					}
				ifindex = ifname2ifindex_by_namelen(ifname,strnlen(ifname, INTERFACE_NAMSIZ));;
				nexthop_ipv4_ifindex_flags_add(rib,ipv4_gate,ifindex,nexthop_flags);
				if(tipc_server_debug)
				  zlog_debug("%s : line %d,fetch	ipv4 %s , ifname %s ==>ifindex %d,add to rib->nexthop->ifindex %d ... ...OK\n",
								 __func__,__LINE__,inet_ntop(AF_INET,ipv4_gate,buf2,BUFSIZ),ifname,ifindex,rib->nexthop->ifindex);
					
				break;
				/****fllow ipv6 put packet ,we can make get packet...(left for in future)****/
#if 1				
#ifdef HAVE_IPV6
				  case NEXTHOP_TYPE_IPV6:
				  case NEXTHOP_TYPE_IPV6_IFINDEX:
					stream_get (ipv6_gate ,s,16);/*get  ipv6 gate, 16 byte*/
					memset (ifname, 0, 16);
					stream_get(ifname,s,(size_t)INTERFACE_NAMSIZ);
					if(tipc_server_debug)
					 zlog_debug("%s : line %d, fetch nexthop ifname[%s].\n",__func__,__LINE__,ifname);
					
					/*gujd : 2013-01-10, pm 4:34 . Add for interface local used in route.*/
					ret = check_interface_belong_to_local_board_set_local_mode(ifname);
					if(ret == 1&&(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD))/*change*/
					 {
						zlog_debug("%s: line %d, nexthop interface(%s) set local, not to add rib.\n",__func__,__LINE__,ifname);
						/*break;*/
						set_local_count++;
						break;
					 }
					ifindex = ifname2ifindex_by_namelen(ifname,strnlen(ifname, INTERFACE_NAMSIZ));
					
					nexthop_ipv6_ifindex_flags_add(rib,ipv6_gate,ifindex,nexthop_flags);/**put ifindex and ipv4 gate to nexthop**/								
					if(tipc_server_debug)
					   zlog_debug("%s : line %d,fetch  ipv6 %s , ifname %s ==>ifindex %d,add to rib->nexthop->ifindex %d ... ...OK\n",
									   __func__,__LINE__,inet_ntop(AF_INET6,ipv6_gate,buf2,BUFSIZ),ifname,ifindex,rib->nexthop->ifindex);
					break;
					
				  case NEXTHOP_TYPE_IPV6_IFNAME:
					stream_get (ipv6_gate ,s,16);/*get  ipv6 gate, 16 byte*/
					memset (ifname, 0, 16);
					stream_get(ifname,s,(size_t)INTERFACE_NAMSIZ);
					/*gujd : 2013-01-10, pm 4:34 . Add for interface local used in route.*/
					ret = check_interface_belong_to_local_board_set_local_mode(ifname);
					if(ret == 1&&(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD))/*change*/
					 {
						zlog_debug("%s: line %d, nexthop interface(%s) set local, not to add rib.\n",__func__,__LINE__,ifname);
						/*break;*/
						set_local_count++;
						break;
						}
					ifindex = ifname2ifindex_by_namelen(ifname,strnlen(ifname, INTERFACE_NAMSIZ));
					
					nexthop_ipv6_ifindex_flags_add(rib,ipv6_gate,ifindex,nexthop_flags);/**put ifindex and ipv4 gate to nexthop**/								
					if(tipc_server_debug)
					   zlog_debug("%s : line %d,fetch  ipv6 %s , ifname %s ==>ifindex %d,add to rib->nexthop->ifindex %d ... ...OK\n",
									   __func__,__LINE__,inet_ntop(AF_INET6,ipv6_gate,buf2,BUFSIZ),ifname,ifindex,rib->nexthop->ifindex);
					break;
#endif

#endif
				/*never used NEXTHOP_TYPE_BLACKHOLE, you can look sever*/
#if 1
	//			case NEXTHOP_TYPE_BLACKHOLE:
				default:
					if(tipc_server_debug)
						zlog_debug("%s : line %d , recv nexthop type is default ...\n",__func__,__LINE__);

					if (cmd == ZEBRA_IPV4_ROUTE_ADD 
                    || cmd == ZEBRA_IPV4_ROUTE_DELETE)
                  {
                    struct in_addr empty;
                    memset (&empty, 0, sizeof (struct in_addr));
                    stream_get ((u_char *) &empty, s, IPV4_MAX_BYTELEN);

					if(CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
						nexthop_blackhole_reject_add(rib, ZEBRA_FLAG_BLACKHOLE);
					else /**ZEBRA_FLAG_REJECT**/
						nexthop_blackhole_reject_add(rib, ZEBRA_FLAG_REJECT);
						
                  }
                else/**ipv6**/
                  {
                    struct in6_addr empty;
                    memset (&empty, 0, sizeof (struct in6_addr));
                    stream_get ((u_char *) &empty, s, IPV6_MAX_BYTELEN);
					
					if(CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
						nexthop_blackhole_reject_add(rib, ZEBRA_FLAG_BLACKHOLE);
					else /**ZEBRA_FLAG_REJECT**/
						nexthop_blackhole_reject_add(rib, ZEBRA_FLAG_REJECT);
                  }

 #endif               
			 
			 }
	
	
		}
	  }
	  
	  /**in fact ,when detele route tipc server doesn't send distance and metric to tipc client,they needn't**/
	  /* Distance. */
	  if (CHECK_FLAG (message, ZAPI_MESSAGE_DISTANCE))
		{
		
		  rib->distance = stream_getc (s);
		  if(tipc_server_debug)
			zlog_debug("%s :line %d , recv distance = %u\n",__func__,__LINE__,rib->distance);	
		}
		
	  /* Metric. */
	  if (CHECK_FLAG (message, ZAPI_MESSAGE_METRIC))
		{
		  rib->metric = stream_getl (s);
		  if(tipc_server_debug)
			zlog_debug("%s :line %d , recv metric = %d\n",__func__,__LINE__,rib->metric);	  
		}
		
	  /* Table */
	  rib->table=zebrad.rtm_table_default;

	  /*gujd: 2013-01-10, pm 7:01. Add for other board set interface local in route nexthop.*/
	  /*First, only nexthop and set local, but interface belong to other board, so free this rib in this board ,not to intstall.
	  	Second, all nexthop are set local and interfaces belong to other board , so free this rib in this board , not t install.*/
	  if(cmd == ZEBRA_IPV4_ROUTE_ADD ||cmd == ZEBRA_IPV6_ROUTE_ADD)
	  {
	   /*CID 17201 (#1 of 1): Uninitialized scalar variable (UNINIT)
            16. uninit_use: Using uninitialized value "nexthop_num". */
	  	 if((set_local_count == 1 && nexthop_num == set_local_count )
		 	|| nexthop_num == set_local_count)/*(1)Only one nexthop and set local;(2)More nexthop and all are set local.*/
	  	 {
	  	 	zlog_debug("%s: line %d ,nexthop set local count[%d] cased free this rib info.\n",__func__,__LINE__,set_local_count);
			goto skip;
	  	  }
	  	}

	  /*gujd : 2013-3-12, pm 4:55. Change for bakup master and vice to 
	  	add static route to vrf_static_table , used for show running, when 
	  	active master turn to bakup master or hotplug . At present, this is
	  	not perfect,  we will involve the setup split in future.*/
#if 0
	  if(cmd == ZEBRA_IPV4_ROUTE_ADD) 	
			vice_rib_ipv4_add(&p,rib);
	  
	  else if(cmd == ZEBRA_IPV4_ROUTE_DELETE)	
			vice_rib_ipv4_detele(&p, rib);
	  
	  else if(cmd == ZEBRA_IPV6_ROUTE_ADD)
			vice_rib_ipv6_add(&p,rib);
	  
	  else if(cmd == ZEBRA_IPV6_ROUTE_DELETE)
			vice_rib_ipv6_delete(&p,rib);
	  else
		{
			zlog_err("%s : line %d, unkown cmd %d .\n",__func__,__LINE__,cmd);
		}
#else
		if(cmd == ZEBRA_IPV4_ROUTE_ADD)   
			vice_rib_ipv4(cmd,& p,rib);

		else if(cmd == ZEBRA_IPV4_ROUTE_DELETE)   
		  	vice_rib_ipv4(cmd,& p,rib);

		else if(cmd == ZEBRA_IPV6_ROUTE_ADD)
		  	vice_rib_ipv6(cmd,&p,rib);

		else if(cmd == ZEBRA_IPV6_ROUTE_DELETE)
		  	vice_rib_ipv6(cmd,&p,rib);
		
		else
		  {
			  zlog_err("%s : line %d, unkown cmd %d .\n",__func__,__LINE__,cmd);
		  }

#endif
	
	/*	zebra_rib_ipv4_multipath (&p, rib);*/
	
	/**gjd: the next two api funcs are straight to tell client kernel ,not through zebra .
		But here isn't use this methord , this methord can debug some info for kernel,so it saved**/
#if 0
		if(cmd == ZEBRA_IPV4_ROUTE_ADD) 	
			tipc_client_kernel_add_ipv4(&p, rib);
		else		
			tipc_client_kernel_delete_ipv4(&p, rib);
#endif	
	
		if(tipc_server_debug)
		  zlog_debug ("leave func %s ......\n", __func__);
		
		if(p.family == AF_INET)
		  XFREE (MTYPE_IN_ADD, ipv4_gate);/**add**/
		else
		  XFREE (MTYPE_IN6_ADD, ipv6_gate);

		/*CID 13543 (#2-4 of 4): Resource leak (RESOURCE_LEAK)
		24. leaked_storage: Variable "rib" going out of scope leaks the storage it points to.
		No problem. The rib is used in some where.*/
	
	  return 0;
	  
skip:
	if(p.family == AF_INET)
	  XFREE (MTYPE_IN_ADD, ipv4_gate);/**add**/
	else
	  XFREE (MTYPE_IN6_ADD, ipv6_gate);
	/*CID 17197: Dereference before null check (REVERSE_INULL)
        check_after_deref: Null-checking "rib" suggests that it may be null, 
        but it has already been dereferenced on all paths leading to the check. 
        So add check it above.*/
	if(rib)
	{
		struct nexthop *nexthop = NULL, *next = NULL;
		
		for (nexthop = rib->nexthop; nexthop; nexthop = next)
		{
			next = nexthop->next;
			nexthop_free (nexthop);
		  }
		
		XFREE(MTYPE_RIB,rib);
		}
	
	return 1;
	
	}

/**add by gjd : when system running, the route change, client recv stream buf from server,
  this func use command to tell client zebra**/
static int
tipc_zebra_read_route (int command, tipc_server *vice_board, zebra_size_t length)
{
	if(tipc_server_debug)
	  	zlog_debug ("enter func %s ......\n", __func__);
	if (tipc_server_debug)
	    zlog_debug("%s: line %d ,length = %d,command = %d\n", __func__,__LINE__,length,command);
	  
	switch(command)
	{
		case ZEBRA_IPV4_ROUTE_ADD:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV4_ROUTE_ADD \n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV4_ROUTE_ADD, vice_board, length);
			break;
		case ZEBRA_IPV4_ROUTE_DELETE:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV4_ROUTE_DELETE \n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV4_ROUTE_DELETE,vice_board,length);
			break;
		case ZEBRA_IPV6_ROUTE_ADD:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV6_ROUTE_ADD \n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV6_ROUTE_ADD, vice_board, length);
			break;
		case ZEBRA_IPV6_ROUTE_DELETE:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV6_ROUTE_DELETE \n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV6_ROUTE_DELETE,vice_board,length);
			break;	
			
		default:
			return -1;
	}
	if(tipc_server_debug)
	  zlog_debug ("leave func %s \n", __func__);
	return 0;

}


/**add by gjd: when system boot up, client recv stream buf , to tell client zebra 
	to install all the route info which include all server route**/
static int
tipc_zebra_boot(int command, tipc_server *vice_board, zebra_size_t length)
{
	if(tipc_server_debug)
	  	zlog_debug ("enter func %s ....boot up\n", __func__);
	if (tipc_server_debug)
	    zlog_debug("%s: line %d ,length = %d,command = %d ....boot up\n", __func__,__LINE__,length,command);
	  
	switch(command)
	{
		case ZEBRA_IPV4_ROUTE_ADD:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV4_ROUTE_ADD .... boot up\n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV4_ROUTE_ADD, vice_board, length);
			break;
		case ZEBRA_IPV6_ROUTE_ADD:
			if(tipc_server_debug)
	  			zlog_debug ("%s: line %d, cmd = ZEBRA_IPV6_ROUTE_ADD.... boot up\n", __func__,__LINE__);
			tipc_client_route_multipath(ZEBRA_IPV6_ROUTE_ADD, vice_board,length);
			break;
		default:
			return -1;
	}
	if(tipc_server_debug)
	  zlog_debug ("leave func %s \n", __func__);
	return 0;
}


int
vice_send_interface_descripton_set(tipc_server *client, struct interface *ifp,int command)
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

  s = client->obuf;
  stream_reset (s);
  length = strlen(ifp->desc);
  
  if(tipc_server_debug)
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

  return vice_send_message_to_master(client);
}

int
vice_send_interface_descripton_unset(int command,tipc_server *client, struct interface *ifp)
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


  s = client->obuf;
  stream_reset (s);
  
  if(tipc_server_debug)
  	zlog_debug("%s : line %d , command[%s],interface (%s).\n",
  			__func__,__LINE__,zserv_command_string(command),ifp->name);
  /* Message type. */
  tipc_packet_create_header (s, command);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putl(s, INTERFACE_DESCRIPTON_UNSET);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return vice_send_message_to_master(client);
}


int 
vice_send_interface_packets_statistics(tipc_server *client, struct interface *ifp,int if_flow_command)
{
  struct stream *s;

  /* Check this client need interface information. */
  if (! client->ifinfo)
	return 0;

  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, ZEBRA_INTERFACE_PACKETS_STATISTICS);
  stream_putl(s,if_flow_command);
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
  stream_putq (s, ifp->stats.collisions);
  stream_putq (s, ifp->stats.rx_length_errors);
  stream_putq (s, ifp->stats.rx_over_errors);
  stream_putq (s, ifp->stats.rx_crc_errors);
  stream_putq (s, ifp->stats.rx_frame_errors);
  stream_putq (s, ifp->stats.rx_compressed );
  stream_putq (s, ifp->stats.rx_fifo_errors);
  stream_putq (s, ifp->stats.rx_missed_errors);
  stream_putq (s, ifp->stats.tx_aborted_errors);
  stream_putq (s, ifp->stats.tx_carrier_errors);
  stream_putq (s, ifp->stats.tx_fifo_errors);
  stream_putq (s, ifp->stats.tx_heartbeat_errors);
  stream_putq (s, ifp->stats.tx_window_errors);
  
  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  #if 0
#ifdef HAVE_PROC_NET_DEV
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
#endif /* HAVE_PROC_NET_DEV */
#endif
  return vice_send_message_to_master(client);
}


int
vice_send_interface_update (int cmd, tipc_server *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;
#if 0
  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE))
  	return 0;
#else
	if(product->product_type == PRODUCT_TYPE_7605I)
		  DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_7605I(ifp->name,ret);
	  else
		  DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(ifp->name,ret);

  	if(ret == 1)
   	  return 0;

#endif
  if(judge_real_local_interface(ifp->name)== OTHER_BOARD_INTERFACE)
  {
	  zlog_info("%s : local board not send interface(%s)update.\n",__func__,ifp->name);
	  return 0;
  }

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header (s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
 // stream_putl (s, ifp->ifindex);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
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

  return vice_send_message_to_master(client);
}


int
vice_send_interface_add (tipc_server *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;
#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
   	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
   	||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)
   	||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE))
  	return 0;
#else
	if(product->product_type == PRODUCT_TYPE_7605I)
		DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_7605I(ifp->name,ret);
	else
		DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(ifp->name,ret);
	
	if(ret == 1)
	 return 0;

#endif
if(judge_real_local_interface(ifp->name)== OTHER_BOARD_INTERFACE)
{
	zlog_info("%s : local board not send interface(%s)ADD.\n",__func__,ifp->name);
	return 0;
}
  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  /* Message type. */
  tipc_packet_create_header (s, ZEBRA_INTERFACE_ADD);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  
  if(tipc_server_debug)
  zlog_debug("%s, line == %d, name = %s",__func__,__LINE__, ifp->name);
//  stream_putl (s, ifp->ifindex);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
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
	if(tipc_server_debug)
	  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
	stream_putc(s,ifp->if_scope);
	if(tipc_server_debug)/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
	 zlog_debug("%s, line %d, ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);
	stream_putl(s, ifp->slot);
	stream_putl(s, ifp->devnum);


  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return vice_send_message_to_master(client);
}


int
vice_send_interface_delete (tipc_server *client, struct interface *ifp)
{
  struct stream *s;
  int ret = 0;

#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)
	   ||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
	   ||(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)
	   ||(judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE))
  	return 0;
#else
	   if(product->product_type == PRODUCT_TYPE_7605I)
		DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_7605I(ifp->name,ret);
	else
		DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(ifp->name,ret);
	   if(ret == 1)
		return 0;
   
#endif
/*
  if(judge_real_local_interface(ifp->name)== OTHER_BOARD_INTERFACE)
  {
	  zlog_info("%s : local board not send interface(%s)DEL.\n",__func__,ifp->name);
	  return 0;
  }
*/

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);
  
  tipc_packet_create_header (s, ZEBRA_INTERFACE_DELETE);
  
  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  //stream_putl (s, ifp->ifindex);
  stream_putc (s, ifp->status);
  stream_putq (s, ifp->flags);
  stream_putl (s, ifp->metric);
  stream_putl (s, ifp->mtu);
  stream_putl (s, ifp->mtu6);
  stream_putl (s, ifp->bandwidth);

  /*gjd : add for local or global interface of Distribute System .*/
  
  if(tipc_server_debug)  
  	zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
  stream_putc(s,ifp->if_scope);
  if(tipc_server_debug)/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
	 zlog_debug("%s, line %d, ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);
	stream_putl(s, ifp->slot);
	stream_putl(s, ifp->devnum);

  /* Write packet length. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return vice_send_message_to_master(client);
}



int
vice_send_interface_address (int cmd, tipc_server *client, 
                         struct interface *ifp, struct connected *ifc)
{

  int blen;
  struct stream *s = NULL;
  struct prefix *p = NULL;
  char buf[BUFSIZ];
  int ret = 0;

#if 0
   if((judge_ve_interface(ifp->name)== VE_INTERFACE)||(judge_obc_interface(ifp->name)== OBC_INTERFACE))
  	return 0;
#else
	   DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO_VEXX(ifp->name,ret);
	   if(ret == 1)
		return 0;
   
#endif

   	/**gjd: add for ckeck ipv6 address like prefix is fe80:: , for Distribute System . If it is , not redistribute.**/
#if 1
	if(ifc->address->family == AF_INET6)
	{
	  if(IPV6_ADDR_FE80(ifc->address->u.prefix6.in6_u.u6_addr16[0]))
	  {
	   if(tipc_server_debug)
	  	zlog_debug("%s : line %d , Don't go to sync ipv6 address like fe80::xxx .\n",__func__,__LINE__);
		return 0;
	  	}
	}
#endif
	/**2011-09-16: pm 5:45.**/
  
  if(tipc_server_debug)
   zlog_debug("enter tipc_zsend_interface_address");
  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  if(NULL == s) {
  	
	if(tipc_server_debug)
		zlog_debug("%s, line = %d,s == NULL!\n ", __func__, __LINE__);
		return 0;
  }
  stream_reset (s);
  
  tipc_packet_create_header (s, cmd);
  
  if(tipc_server_debug)
   zlog_debug("%s, line %d, cmd ==  %d", __func__, __LINE__, cmd);
  
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  stream_putc(s,ifc->conf);
  stream_putc(s,ifc->ipv6_config);

  /* Interface address flag. */
  stream_putc (s, ifc->flags);  
  if(tipc_server_debug)
  zlog_debug("%s, line %d, ifc->flags ==  %d", __func__, __LINE__, ifc->flags);
  stream_putc(s,ifp->if_scope);

  /* Prefix information. */
  p = ifc->address;
  stream_putc (s, p->family);
  
  if(tipc_server_debug)
  zlog_debug("send p->family == %d", p->family);
  
  blen = prefix_blen (p);
  stream_put (s, &p->u.prefix, blen);
  if(tipc_server_debug)
  zlog_debug("vice send interface %s : ip %s/%d \n",
  	ifp->name,inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),p->prefixlen);

  /* 
   * XXX gnu version does not send prefixlen for ZEBRA_INTERFACE_ADDRESS_DELETE
   * but zebra_interface_address_delete_read() in the gnu version 
   * expects to find it
   */
  stream_putc (s, p->prefixlen);
  
  stream_putl(s,ifp->slot);
  stream_putl(s, ifp->devnum);
  if(tipc_server_debug)
    zlog_debug("%s, line %d, put ifp->name = %s, slot = %d , devnum = %d ...\n", __func__, __LINE__, ifp->name,ifp->slot,ifp->devnum);

  /* Destination. */
  p = ifc->destination;
  if (p)
    stream_put (s, &p->u.prefix, blen);
  else
    stream_put (s, NULL, blen);


  /* Write packet size. */
  stream_putw_at (s, 0, stream_get_endp (s));

  return vice_send_message_to_master(client);
}

void
vice_redistribute_interface_up(struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  int ret;

  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
   for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
   {
      zsend_interface_update (ZEBRA_INTERFACE_UP, client, ifp);
   }

}



/* Interface down information. */
void
vice_redistribute_interface_down (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  int ret;
  
  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
   for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client))
  	{
	    zsend_interface_update (ZEBRA_INTERFACE_DOWN, client, ifp);
  	}


}

/* Interface information update. */
void
vice_redistribute_interface_add (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  int ret;
  
#if 1
  /*send message to router deamon*/
  if(judge_obc_interface(ifp->name)==OBC_INTERFACE)
	  return;;

  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
	{
	  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
		  if (client->ifinfo)
			zsend_interface_add (client, ifp);
		}

	}  
#endif

}



void
vice_redistribute_interface_delete (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  int ret;

  if(ifp->if_types != VIRTUAL_INTERFACE && ifp->ifindex != IFINDEX_INTERNAL )/*make susre ifindex effective, and redistribute */
   for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo)
      zsend_interface_delete (client, ifp);
   }

}

/* Interface address addition. */
void
vice_redistribute_interface_address_add (struct interface *ifp,
				    struct connected *ifc)
{
  if(tipc_server_debug)
	zlog_debug("enter %s ....", __func__);

  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  struct prefix *p;
  char buf[BUFSIZ];
  int ret;

#if 0   /*log_message*/  
  p = ifc->address;
  zlog_debug ("MESSAGE: ZEBRA_INTERFACE_ADDRESS_ADD %s/%d on %s",
	  inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
	  p->prefixlen, ifc->ifp->name);
#endif

 if(ifp->if_types == VIRTUAL_INTERFACE || ifp->ifindex == IFINDEX_INTERNAL)
 	return;
  router_id_add_address(ifc);
  if (!zebrad.client_list) {
	zlog_debug("%s, zebrad.client_list == NULL", __func__);
	return ;
  }
  
  for (ALL_LIST_ELEMENTS (zebrad.client_list, node, nnode, client)) {
    if (client->ifinfo && CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
      zsend_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, client, ifp, ifc);
  	}
  
  if(tipc_server_debug)
   zlog_debug("leave %s, line %d", __func__, __LINE__);

}


/* Interface address deletion. */
void
vice_redistribute_interface_address_delete (struct interface *ifp,
				       struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  tipc_server  *vice_board;
  struct prefix *p;
  char buf[BUFSIZ];
  int ret;

  if(tipc_server_debug)
 	zlog_debug ("Func %s start, line %d ",__func__,__LINE__);

  if(ifp->if_types == VIRTUAL_INTERFACE || ifp->ifindex == IFINDEX_INTERNAL)
	 return;
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
  
}


int
vice_interface_send_to_master(tipc_server *vice_board)
{
    if(tipc_server_debug)
    zlog_debug("enter func %s , goto vice send all interfaces to master\n",__func__);
          
    struct listnode *ifnode, *ifnnode;
    struct listnode *node, *nnode;
    struct listnode *cnode, *cnnode;
    struct interface *ifp;
    struct connected *c;
    int ret = 0;
    
    /* Interface information is needed. */
    vice_board->ifinfo = 1;
    
	/*CID 11368 (#1 of 2): Missing return statement (MISSING_RETURN)
	3. missing_return: Arriving at the end of a function without returning a value.
	Add return value.*/
	if(product->board_type == BOARD_IS_ACTIVE_MASTER)
	  return -1;

    
    if(zebrad.master_board_list == NULL)
	{
	     zlog_debug("%s, line = %d :tipc_client_list is NULL....", __func__, __LINE__);
	    return -1;
	  }
	
	/*gujd: 2012-06-06, am 11:03. Add for first let ve parent interface info send to active master to register RPA table .*/
	/*first : search ve parent interface.*/
    for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
	{
		if((strncmp(ifp->name,"ve",2) == 0) && (judge_ve_interface(ifp->name)==VE_INTERFACE)) 
		{
	      for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
	      {
	        zlog_debug("%s: line%d, interface(%s).\n", __func__, __LINE__, ifp->name); 
	    	 vice_send_interface_add (vice_board, ifp);
	         if ( CHECK_FLAG (ifp->flags, IFF_UP)) 
			{
				zlog_debug("%s: line%d, interface(%s)[up]\n", __func__, __LINE__, ifp->name);
	    		vice_send_interface_update (ZEBRA_INTERFACE_UP, vice_board, ifp);
	    	} 
			else 
			{
				zlog_debug("%s: line%d, interface(%s)[down].\n", __func__, __LINE__, ifp->name);
				vice_send_interface_update (ZEBRA_INTERFACE_DOWN, vice_board, ifp);
			  }
	      }	  		  
	    }
    }
    
    for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
    {
	/*2012-10-17, pm 4:15 .Optimized code .Delete it.*/
	#if 0
   	 if(judge_eth_debug_interface(ifp->name) == ETH_DEBUG_INTERFACE)/*skip : the debug eth3 interface .*/
		continue;
	
	/*if((strncmp(ifp->name,"eth",3) == 0 && strncmp(ifp->name,"eth3",4) != 0) */ /**7605i: eth3 is a debug interface**/
	if(((strncmp(ifp->name,"eth",3) == 0) && (judge_eth_interface(ifp->name)==ETH_INTERFACE)) 
		|| (strncmp(ifp->name,"wlan",4) == 0)|| (strncmp(ifp->name,"ebr",3) == 0) 
		|| (strncmp(ifp->name,"r",1) == 0)|| (strncmp(ifp->name,"vlan",4) == 0) 
/*		|| ((strncmp(ifp->name,"ve",2) == 0)&&(judge_eth_sub_interface(ifp->name)== VE_SUB_INTERFACE)) */
		||(strncmp(ifp->name,"ve",2) == 0))
#endif
	{
      for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
      {
     	 if(tipc_server_debug)
           zlog_debug("%s: line%d, ifp->name == %s", __func__, __LINE__, ifp->name); 
 /*    	if ( CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))*/
			{
    			vice_send_interface_add (vice_board, ifp);
    		}
 #if 0
 			else 
			{
    			vice_send_interface_delete (vice_board, ifp); 
    		}           
#endif        			
        	if ( CHECK_FLAG (ifp->flags, IFF_UP)) 
			{
        		vice_send_interface_update (ZEBRA_INTERFACE_UP, vice_board, ifp);
        	} 
			else 
			{
    			vice_send_interface_update (ZEBRA_INTERFACE_DOWN, vice_board, ifp);
    		   }
			/*use rand time interval .10ms--110ms*/
			/*usleep(fetch_rand_time_interval());*//*2012-10-17, pm 4:15 .Optimized code .*/
      }
      
      for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c)) 
	  	{
	      	for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board)) 
			{
         		if (CHECK_FLAG (c->conf, ZEBRA_IFC_REAL)) 
					{
	/*					SET_FLAG (c->conf, ZEBRA_IFC_CONFIGURED);*/
         				vice_send_interface_address (ZEBRA_INTERFACE_ADDRESS_ADD, vice_board, ifp, c);
         			}	
	      	}
			/*use rand time interval .10ms--110ms*/
			/*usleep(fetch_rand_time_interval());*//*2012-10-17, pm 4:15 .Optimized code .*/
      	}	  
      }
    }
	 if(tipc_server_debug)
      zlog_debug("leave func %s ....\n",__func__);
    return 0;
}

 
 int
 vice_interface_infomation_request(int command, tipc_server *vice_board, zebra_size_t length)
 {
	 int request_flag = 0;
	 if(!vice_board || !vice_board->ibuf)
	 {
		 zlog_warn("vice board or inbuf is null .\n");
		 return -1;
	 }
	 request_flag = stream_getl(vice_board->ibuf);
	 
	 if(request_flag == 1)
	 {
		 zlog_info("Master restart don't need interface infomation .\n");
		 return 0;
	 }
	 else if(request_flag == 0)
	 {
		 zlog_info("Master first start need interface infomation .\n");
		 vice_interface_send_to_master(vice_board);
		 return 0;
	 }
	 else
	 {
		 zlog_warn("err request flag (%d).\n",request_flag);
		 return -1;
	 }
	 
 
	 
 }

 
 
 struct interface *
 vice_interface_state_set(struct interface *ifp, uint64_t flags, int rpa_done, int normal_done)
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
			 if_refresh_bak(ifp);
			 return ifp;
		 }
		 else
		 {
			 ret = rpa_interface_running_state_unset(ifp);/*unset running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh_bak(ifp);
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
			 if_refresh_bak(ifp);
			 return ifp;
		 }
		 else
		 {
			 ret = if_unset_flags(ifp,(uint64_t)(IFF_UP));/*unset running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh_bak(ifp);
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
			 if_refresh_bak(ifp);
	 // 	 return ifp;
		 }
		 if(CHECK_FLAG(flags, IFF_RUNNING))
		 {
			 ret = rpa_interface_running_state_set(ifp);/*normal set running*/
			 if(ret < 0)
			 {
				 zlog_warn("set rpa interface %s (running state) failed.\n",ifp->name);
				 
			 }
			 if_refresh_bak(ifp);
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
			 if_refresh_bak(ifp);
			 return ifp;
		 }
		 if(CHECK_FLAG(flags,IFF_UP))/*一般口up，另外一般口不会单设running*/
		 {
			 ret = if_set_flags(ifp,(uint64_t)(IFF_UP));
			 if(ret < 0)
			 {
				 zlog_warn("set real interface %s (up state) failed.\n",ifp->name);
				 
			 }
			 if_refresh_bak(ifp);
			 return ifp;
		 }
		 else/*一般口的down和rpa口的down都按常规操作*/
		 {
			 ret = if_unset_flags(ifp,(uint64_t)(IFF_UP));
			 if(ret < 0)
			 {
				 zlog_warn("unset real or rpa interface %s (down state) failed.\n",ifp->name);
				 
			 }
			 if_refresh_bak(ifp);
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
	/*CID 11369 (#1 of 1): Missing return statement (MISSING_RETURN)
	11. missing_return: Arriving at the end of a function without returning a value.*/
	return ifp;/*add*/
	 
}
 
 struct interface *
 interface_state_up_set(struct interface *ifp ,uint64_t tmp_flags )
 {
	 if(CHECK_FLAG(tmp_flags,IFF_UP))
	{
	 if(tipc_server_debug)
	   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
	 if(CHECK_FLAG(tmp_flags,IFF_RUNNING))/*tmp is up and running*/
	  {
		 if(tipc_server_debug)
		  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
		 if(if_is_up(ifp))/*ifp : up*/
		 {
			 if(tipc_server_debug)
			  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			 if(if_is_running(ifp))
			  {
				 
				if(tipc_server_debug)/*ifp :running*/
				 zlog_debug("%s : line %d , the interface %s flags(state: up and running ) is same ....\n",__func__,__LINE__,ifp->name);
				return ifp;
			   }
			   else
			   {
				   if(tipc_server_debug)
					 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
			   /*  ifp->flags |= IFF_RUNNING;*//*go to ifp + running*/
				   if(ifp->if_types == RPA_INTERFACE)
					 {
					   if(tipc_server_debug)
						 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					   vice_interface_state_set(ifp,(uint64_t)(IFF_RUNNING),1,0);
					 }
				   
#if 1
				   if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
				   {
					  if(tipc_server_debug)
					   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
						ifp->flags = tmp_flags;
				 /* 	vice_redistribute_interface_up(ifp);*/
						return ifp;
					}
#endif
			   /*  else*//*单独让一个实口running不会出现*/
					   
				   return ifp; 
			   }
		   }
		   else/*ifp : down, goto ifp up + running*/
		   {
		   
		   if(tipc_server_debug)
			  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
		   if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
		   {
			 if(tipc_server_debug)
			   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				ifp->flags = tmp_flags;
 // 			vice_redistribute_interface_up(ifp);
				return ifp;
			}
#endif
		   /*  ifp->flags |= (IFF_UP | IFF_RUNNING);*/
			   if(ifp->if_types == RPA_INTERFACE)
				 {
				   if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				   vice_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),1,1);/*rpa口*/
				 }
			   else
				 {
				  if(tipc_server_debug)
					 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				   vice_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),0,1);/*一般口*/
				 }
		   }
		   return ifp;
		   
	   }
	   else/*tmp is up, not running */
	   {
		   if(tipc_server_debug)
			  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
		   if(if_is_up(ifp))/*ifp : up*/
		   {
			   if(tipc_server_debug)
				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			   if(!if_is_running(ifp))/*ifp : not running*/
			   {
				   if(tipc_server_debug)
					 zlog_debug("%s : line %d , the interface %s flags(up state, not running ) is same ....\n",__func__,__LINE__,ifp->name);
				   return ifp;
			   }
			   else
			   {
			   
				 if(tipc_server_debug)
				  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1					
			   if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
			   {
				  if(tipc_server_debug)
				   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					ifp->flags = tmp_flags;
				 /*   vice_redistribute_interface_up(ifp);*/
					return ifp;
				}
#endif
			   /*  ifp->flags &= (~IFF_RUNNING);*//*ifp : running*/
				 if(ifp->if_types == RPA_INTERFACE)
				 {
				   if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				   vice_interface_state_set(ifp,(uint64_t)(~IFF_RUNNING),1,0);/*rpa口去掉running*/
					 }
			   }
			   return ifp;
			   /*一般实口不会单独去掉running*/
		   }
		   else/*ifp : down*/
		   {
		   
			  if(tipc_server_debug)
			   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
			   if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
			   {
				  if(tipc_server_debug)
				   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					ifp->flags = tmp_flags;
			 /* 	vice_redistribute_interface_up(ifp);*/
					return ifp;
				}
#endif
			   /*ifp->flags |= IFF_UP ;*//*rpa口与实口up起来按常规处理*/
			   if(ifp->if_types == RPA_INTERFACE)
				 {
				   if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				   vice_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*rpa口*/
				 }
			   else
				 {
				   if(tipc_server_debug)
					 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				   vice_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*一般口*/
				 }
		   }
		   return ifp;
		   
	   }
	   
	   return ifp;
   }
	 else
	 {
		 zlog_debug("%s : line %d , return ..\n",__func__,__LINE__);
		 return ifp;
			 }
 
 }
 
 struct interface *
 interface_state_down_set(struct interface *ifp, uint64_t tmp_flags )
 {

   if(!CHECK_FLAG(tmp_flags,IFF_UP))/*tmp is down*/
	{
	   
	   if(tipc_server_debug)
		 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
	   if(if_is_up(ifp))/*ifp : up*/
	   {
		   /*  ifp->flags &= ~IFF_UP;*//*to down*/
		 if(tipc_server_debug)
		   zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
		 if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
		  {
			   if(tipc_server_debug)
				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			   ifp->flags = tmp_flags;
	 /* 	   vice_redistribute_interface_down(ifp);*/
			   return ifp;
			}
#endif
			 if(ifp->if_types != VIRTUAL_INTERFACE)
 
			 {
					   
				  if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				  vice_interface_state_set(ifp, (uint64_t)(~IFF_UP),0,1);/*rpa口和一般口down操作都用normal处理*/
				  
				  return ifp;
			 }
		 }
 
		   else/*ifp : down*/
		   {
			   if(tipc_server_debug)
				 zlog_debug("%s : line %d , the interface %s flags(state: down) is same ....\n",__func__,__LINE__,ifp->name);
			   return ifp;
		   }
		   
	   }
   else 
	 {
		 return ifp;
	 }
   
   return ifp;
 }



struct interface *
tipc_vice_zebra_interface_uplink_flag_state_read (int command,struct stream *s)
{
	struct interface *ifp;
	char ifname_tmp[INTERFACE_NAMSIZ];
	int ret = 0;
	unsigned char uplink_flag;

	
	if (s == NULL) {
		if(tipc_server_debug)
		zlog_debug("zebra_interface_state struct stream is null\n");
		return NULL;
	}

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  if(tipc_server_debug)
   zlog_debug("%s : line %d, name = %s\n", __func__,__LINE__,ifname_tmp);

  /* Lookup this by interface name. */
  ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  /* If such interface does not exist, indicate an error */
  if (ifp == NULL)
  {
  	if(tipc_server_debug)
	 zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
     return NULL;
  	}
  
  uplink_flag = stream_getc(s);
  
  if(command == ZEBRA_INTERFACE_UPLINK_FLAG_SET)/*set*/
    SET_FLAG(ifp->uplink_flag ,INTERFACE_SET_UPLINK);
  else if(command ==ZEBRA_INTERFACE_UPLINK_FLAG_UNSET)/*clear*/
    UNSET_FLAG(ifp->uplink_flag ,INTERFACE_SET_UPLINK);
  else
  	zlog_warn("%s: line %d, unkown command[%d].\n",__func__,__LINE__,command);
  
  if(tipc_server_debug)
    zlog_debug("%s: line %d ,uplink_flag(%d),ifp->uplink_flag(%u).\n",__func__,__LINE__,uplink_flag,ifp->uplink_flag);

  return ifp;
  
}

struct interface *
tipc_vice_zebra_interface_state_read (int command,struct stream *s)
{
	struct interface *ifp;
	char ifname_tmp[INTERFACE_NAMSIZ];
	uint64_t tmp_flags = 0;
	char tmp_status = 0;
	
	unsigned int tmp_mtu;	 /* IPv4 MTU */
	int ret = 0;
	int ret2= 0;

	
	if (s == NULL) {
		if(tipc_server_debug)
		zlog_debug("zebra_interface_state struct stream is null\n");
		return NULL;
	}
	


  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  if(tipc_server_debug)
  zlog_debug("%s : line %d, name = %s\n", __func__,__LINE__,ifname_tmp);

  if (judge_real_interface(ifname_tmp)){
  	if(tipc_server_debug)
	 zlog_debug("%s: line %d , %s  is system REAl_INTERFACE \n",__func__, __LINE__, ifname_tmp);
	 return ifp;
  }


  /* Lookup this by interface name. */
  ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  /* If such interface does not exist, indicate an error */
  if (ifp == NULL) {
  	if(tipc_server_debug)
	 zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
     return NULL;
  	}

  /*gujd: 2013-12-05, am 11:08. For down VE interface missing IPv6 address when system booting.*/
  if(command == ZEBRA_INTERFACE_UP||command == ZEBRA_INTERFACE_DOWN)
  {
  	 DISABLE_LOCAL_INTERFACE_VE(ifp->name,ret2);
	 if(ret2 == 1)
	 {
	    zlog_info("%s: line %d, locad ve interface[%s] ignore up(down) info from other board.\n",
			                          __func__,__LINE__,ifp->name);
		return ifp;
		}
  	}

/*  ifp->status = stream_getc (s);*/
  tmp_status = stream_getc(s);
  tmp_flags = stream_getq(s);
  

  ifp->metric = stream_getl (s);
  tmp_mtu = stream_getl(s);
  if(tipc_server_debug)
    zlog_debug("%s: line %d ,ifp->status = %d, tmp_status = %d, ifp->flags = 0x%llx, tmp_flags = 0x%llx,old mtu = %d ,new mtu = %d\n",
 	 			__func__,__LINE__,ifp->status, tmp_status, ifp->flags, tmp_flags, ifp->mtu, tmp_mtu);
  if((tmp_mtu != ifp->mtu)&&(ifp->if_types == RPA_INTERFACE ))/*同步过来的mtu只能更改rpa口，没有权限去更改实口的mtu，避免死循环*/
  {

	ifp->mtu = tmp_mtu;
	ret = if_set_mtu(ifp);
	if(ret < 0)
	{
	 zlog_warn("ioctl set mtu failed \n");
	 return ifp;	
	}	
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
  ifp->mtu6 = stream_getl (s);
  ifp->bandwidth = stream_getl (s);

  if(command == ZEBRA_INTERFACE_DELETE)
  {
  	zlog_info("%s, delete interface(%s).\n",__func__, ifp->name);
	if(tipc_server_debug)  
	  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);

	ifp->if_scope = stream_getc(s);
	if(tipc_server_debug)  
		zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);

	ifp->slot = stream_getl(s);
	ifp->devnum = stream_getl(s);
	if(tipc_server_debug)
		zlog_debug("%s : line %d, interface(%s), slot(%d),devnum(%d).\n",__func__,__LINE__,ifp->name,ifp->slot,ifp->devnum);
  	
	goto skip;
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
	
				/*to set new :	local interface do not, virtual interface do not .*/
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
		  
		  /*
		  zlog_debug("%s, line %d,hw_addr[%s],ifp->hw_addr_len[%d], ifp->hw_addr[%s] .\n",
				  __func__, __LINE__,hw_addr,ifp->hw_addr_len,ifp->hw_addr);
		  */
		  
		  }
#endif /* HAVE_SOCKADDR_DL */
  
  if(command == ZEBRA_INTERFACE_UP)
 {
 	ifp->status = tmp_status;
	
    if(CHECK_FLAG(tmp_flags,IFF_UP))
	 {
	  	if(tipc_server_debug)
		  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
		if(CHECK_FLAG(tmp_flags,IFF_RUNNING))/*tmp is up and running*/
		 {
			if(tipc_server_debug)
			 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			if(if_is_up(ifp))/*ifp : up*/
			{
				if(tipc_server_debug)
				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			    if(if_is_running(ifp))
				 {
					
				   if(tipc_server_debug)/*ifp :running*/
					zlog_debug("%s : line %d , the interface %s flags(state: up and running ) is same ....\n",__func__,__LINE__,ifp->name);
				   return ifp;
				  }
				  else
				  {
					  if(tipc_server_debug)
		  				zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				  /*  ifp->flags |= IFF_RUNNING;*//*go to ifp + running*/
					  if(ifp->if_types == RPA_INTERFACE)
					  	{
						  if(tipc_server_debug)
		  					zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
						  vice_interface_state_set(ifp,(uint64_t)(IFF_RUNNING),1,0);
					  	}
					  
#if 1
					  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
					  {
					     if(tipc_server_debug)
						  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
						   ifp->flags = tmp_flags;
					/*	   vice_redistribute_interface_up(ifp);*/
						   return ifp;
					   }
#endif
				  /*  else*//*单独让一个实口running不会出现*/
						  
					  return ifp; 
				  }
			  }
			  else/*ifp : down, goto ifp up + running*/
			  {
			  
			  if(tipc_server_debug)
		 		 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
			  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
			  {
			    if(tipc_server_debug)
				  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
				   ifp->flags = tmp_flags;
	//			   vice_redistribute_interface_up(ifp);
				   return ifp;
			   }
#endif
			  /*  ifp->flags |= (IFF_UP | IFF_RUNNING);*/
				  if(ifp->if_types == RPA_INTERFACE)
				  	{
					  if(tipc_server_debug)
		 				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					  vice_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),1,1);/*rpa口*/
				  	}
				  else
				  	{
					 if(tipc_server_debug)
		  				zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					  vice_interface_state_set(ifp,(uint64_t)(IFF_UP | IFF_RUNNING),0,1);/*一般口*/
				  	}
			  }
			  return ifp;
			  
		  }
		  else/*tmp is up, not running */
		  {
			  if(tipc_server_debug)
		 		 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			  if(if_is_up(ifp))/*ifp : up*/
			  {
				  if(tipc_server_debug)
		  			zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				  if(!if_is_running(ifp))/*ifp : not running*/
				  {
					  if(tipc_server_debug)
						zlog_debug("%s : line %d , the interface %s flags(up state, not running ) is same ....\n",__func__,__LINE__,ifp->name);
					  return ifp;
				  }
				  else
				  {
				  
				    if(tipc_server_debug)
		  			 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1					
				  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
				  {
				  	 if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					   ifp->flags = tmp_flags;
					/*   vice_redistribute_interface_up(ifp);*/
					   return ifp;
				   }
#endif
				  /*  ifp->flags &= (~IFF_RUNNING);*//*ifp : running*/
					if(ifp->if_types == RPA_INTERFACE)
					{
					  if(tipc_server_debug)
		 				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
					  vice_interface_state_set(ifp,(uint64_t)(~IFF_RUNNING),1,0);/*rpa口去掉running*/
						}
				  }
				  return ifp;
				  /*一般实口不会单独去掉running*/
			  }
			  else/*ifp : down*/
			  {
			  
			 	 if(tipc_server_debug)
		 		  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
				  if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
				  {
				     if(tipc_server_debug)
					  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx.\n",__LINE__,tmp_flags,ifp->flags);
					   ifp->flags = tmp_flags;
				/*	   vice_redistribute_interface_up(ifp);*/
					   return ifp;
				   }
#endif
				  /*ifp->flags |= IFF_UP ;*//*rpa口与实口up起来按常规处理*/
				  if(ifp->if_types == RPA_INTERFACE)
				  	{
					  if(tipc_server_debug)
		 				 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
					  vice_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*rpa口*/
				  	}
				  else
				  	{
					  if(tipc_server_debug)
		  				zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
					  vice_interface_state_set(ifp,(uint64_t)(IFF_UP),0,1);/*一般口*/
				  	}
			  }
			  return ifp;
			  
		  }
		  
	  }
	else
		{
			/*linkdetection case this*/
			if(tipc_server_debug)
	  		  zlog_debug("line %d , UP command down flags to up route :status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx ############...\n",
							__LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
	  		if_up_redistribute(ifp);
			return ifp;
	  	}
	}
	
	if(command == ZEBRA_INTERFACE_DOWN)
	{
	
 	  ifp->status = tmp_status;
  
      if(!CHECK_FLAG(tmp_flags,IFF_UP))/*tmp is down*/
	   {
		  
		  if(tipc_server_debug)
			zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
		  if(if_is_up(ifp))/*ifp : up*/
		  {
			  /*  ifp->flags &= ~IFF_UP;*//*to down*/
			if(tipc_server_debug)
			  zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
#if 1
			if(ifp->if_types == VIRTUAL_INTERFACE)/*虚拟口直接赋值*/
			 {
				  if(tipc_server_debug)
					zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
				  ifp->flags = tmp_flags;
		/*		  vice_redistribute_interface_down(ifp);*/
				  return ifp;
			   }
#endif
				if(ifp->if_types != VIRTUAL_INTERFACE)

				{
						  
					 if(tipc_server_debug)
						 zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
					 vice_interface_state_set(ifp, (uint64_t)(~IFF_UP),0,1);/*rpa口和一般口down操作都用normal处理*/
				}
			}

			  else/*ifp : down*/
			  {
				  if(tipc_server_debug)
					zlog_debug("%s : line %d , the interface %s flags(state: down) is same ....\n",__func__,__LINE__,ifp->name);
				  return ifp;
			  }
			  
		  }
	  else /*tmp : is up , the detection is on, not change the intf flags , only to change the route.*/
	  	{
	  		if(tipc_server_debug)
	  		  zlog_debug("line %d , DOWN command up flags to down route :status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx ############...\n",
							__LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
			if(ifp->if_types == RPA_INTERFACE)
			{
			  if(tipc_server_debug)
 				zlog_debug("line %d tmp_flags %llx , ifp->flags %llx\n",__LINE__,tmp_flags,ifp->flags);
			  vice_interface_state_set(ifp,(uint64_t)(~IFF_RUNNING),1,0);/*rpa口去掉running*/
			}
	  		if(tipc_server_debug)
			  zlog_debug("line %d , status = %d , tmp_status = %d,tmp_flags = %llx, flags = %llx .\n",
							__LINE__,ifp->status,tmp_status, tmp_flags,ifp->flags);
			
	  	/*	if_down_redistribute(ifp);*//*上面rpa口去掉running中有if_down_redistribute对rtm下的直连路由操作*/
			return ifp;
	  	}
	}
	
	if(command == ZEBRA_INTERFACE_LINKDETECTION_ENABLE)
	{
		int if_was_operative;

		/* ifp->flags = tmp_flags;*/
		if(tipc_server_debug)
		  zlog_debug("line %d , tmp_status = %d, ifp->status = %d .\n",__LINE__,tmp_status,ifp->status);

		if_was_operative = if_is_operative(ifp);
		/*  SET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);*/
		ifp->status = tmp_status;
#if 0		
		ifp = interface_state_down_set(ifp,tmp_flags);
#else
		interface_state_down_set(ifp,tmp_flags);
#endif
		/* When linkdetection is enabled, if might come down */
		if (!if_is_operative(ifp) && if_was_operative)
		{
			if(tipc_server_debug)
			  zlog_debug("line %d, down rtm connect route. \n",__LINE__);
			if_down_redistribute(ifp);
			}
			return ifp;

		}
	
	if(command == ZEBRA_INTERFACE_LINKDETECTION_DISABLE)
	{
		
		 int if_was_operative;
		
		  /*ifp->flags = tmp_flags;*/
		 if(tipc_server_debug)
		  	zlog_debug("line %d , tmp_status = %d, ifp->status = %d .\n",__LINE__,tmp_status,ifp->status);

		 if_was_operative = if_is_operative(ifp);
		  /* UNSET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);*/
		 ifp = interface_state_up_set(ifp, tmp_flags);
		 ifp->status = tmp_status;
		  
		  /* Interface may come up after disabling link detection */
		  if (if_is_operative(ifp) && !if_was_operative) 
			  {
				  zlog_debug("line %d, up rtm connect route.\n",__LINE__);
				  if_up_redistribute(ifp);
				  }
		  return ifp;
	}

skip:
	
/* if(judge_ve_sub_interface(ifp->name) != VE_SUB_INTERFACE)*/
	if(!CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*gujd: 2012-05-05, am 11:50. Change for interface local.*/
 	/*ve sub interface not change*/
 	/*针对active主控设置local时，删除自身ve口是监听到的netlink消息，此时口已经inactive，所以发出来也是inactive的，导致本板的ifp status也会inactive，所以不赋值*/
 	{
  		ifp->status = tmp_status;
 	}
 	
  ifp->flags = tmp_flags;

  return ifp;
}


struct interface *
tipc_vice_zebra_interface_add_read (struct stream *s)
{

  struct interface *ifp;
  char ifname_tmp[INTERFACE_NAMSIZ];

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  
  zlog_info("%s, add interface(%s).\n",__func__, ifname_tmp);
  if (judge_vlan_interface(ifname_tmp)==VLAN_INTERFACE)
  {
	 zlog_debug("%s, Not to add VLAN interface (%s), return .\n",__func__, ifname_tmp);
	 return NULL;
  }

  /* Lookup/create interface by name. */
  ifp = check_interface_exist_by_name(ifname_tmp);

  if(ifp)
  {
   if(ifp->ifindex != 0)
   	{
   		if(judge_ve_interface(ifp->name)== VE_INTERFACE)/*ve parent interface to update devnum*/
			goto skip;
   		if(tipc_server_debug)
		  zlog_debug("%s : line %d ,The interface %s is exist(rpa or real) on this system !\n",__func__,__LINE__,ifname_tmp);
		return ifp;
   	}
   else
   	{
   	
		ifp->if_types = VIRTUAL_INTERFACE;
   		if(tipc_server_debug)
		  zlog_debug("%s : line %d ,The interface %s is exist(virtual) on this system, so go on set value !\n",__func__,__LINE__,ifname_tmp);
		goto skip;
   	}
  }
  else
  {   /*create virtual interface*/
	  ifp = if_create(ifname_tmp, strlen(ifname_tmp));
	  if(!ifp)
	  {
	  /*if(tipc_server_debug)*/
		  zlog_debug("%s: line %d, Can't create interface %s in this system!\n",__func__,__LINE__,ifname_tmp);
	  	  return NULL;/*add*/
	  }
	  /*CID 11031 (#1 of 1): Dereference after null check (FORWARD_NULL)
		6. var_deref_op: Dereferencing null pointer "ifp".
		So add return NULL above.*/
	  ifp->if_types = VIRTUAL_INTERFACE;
	   if(tipc_server_debug)
		  zlog_debug("create interface %s (virtual) !\n",ifp->name);
  }
  
skip:/*虚口去更新赋值*/
  /* Read interface's value. */
  ifp->status = stream_getc (s);
  ifp->flags = stream_getq (s);
  ifp->metric = stream_getl (s);
  ifp->mtu = stream_getl (s);
  ifp->mtu6 = stream_getl (s);
  ifp->bandwidth = stream_getl (s);

#ifdef HAVE_SOCKADDR_DL
  stream_get (&ifp->sdl, s, sizeof (ifp->sdl));
#else
  ifp->hw_addr_len = stream_getl (s);

  if (ifp->hw_addr_len){
    stream_get (ifp->hw_addr, s, ifp->hw_addr_len);
  	}
#endif /* HAVE_SOCKADDR_DL */

  /*gjd : add for local or global interface of Distribute System .*/
	if(tipc_server_debug)  
	  zlog_info("%s : line %d , interface %s scope is %d .\n",__func__,__LINE__,ifp->name,ifp->if_scope);
	ifp->if_scope = stream_getc(s);
	
  /*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
	ifp->slot = stream_getl(s);
	ifp->devnum = stream_getl(s);
   if(tipc_server_debug)
	 zlog_debug("%s : line %d, interface(%s), slot(%d),devnum(%d).\n",__func__,__LINE__,ifp->name,ifp->slot,ifp->devnum);
	
		
  return ifp;

}


/*gujd: 2012-11-15, pm 4:00. When creat rpa, kernel not ready creat the netdev, then install ip will failed. 
	So set timer to repeat install ip.*/
int
interface_ip_add_userspace(struct connected *ifc)
{
	int ret = 0;
	
	if(ifc->address->family == AF_INET)
	 {
		 ret = if_subnet_add (ifc->ifp, ifc);
		 if (ret < 0) 
		 {
		 	if(tipc_server_debug)
		     zlog_debug("%s, line %d, if_subnet_add failed\n", __func__, __LINE__);
			return -1;
		 }
	  }

	vice_redistribute_interface_address_add(ifc->ifp, ifc);

	if(ifc->address->family == AF_INET)
	{
      if (if_is_operative(ifc->ifp))
      {
        if(tipc_server_debug)
      	 zlog_debug("%s : line %d, go to creat ipv4 connect route ....\n",__func__,__LINE__);
		connected_up_ipv4 (ifc->ifp, ifc);  
      }
	}
	else if(ifc->address->family == AF_INET6)
	{
		 if (if_is_operative(ifc->ifp))
		 {
	        if(tipc_server_debug)
	      	 zlog_debug("%s : line %d, go to creat ipv6 connect route ....\n",__func__,__LINE__);
			connected_up_ipv6 (ifc->ifp, ifc);  
		  }
	   }
	else
	{
		zlog_debug("XXXX err !\n");
	}
  	return 0;
  
}

/*gujd: 2012-11-15, pm 4:00. When creat rpa, kernel not ready creat the netdev, then install ip will failed. 
	So set timer to repeat install ip.*/
int
set_ip_address_again_by_thread_timer(struct thread *thread)
{
	struct connected *ifc = NULL;
	int ret = 0;

	ifc = THREAD_ARG(thread);

	if(ifc->address->family == AF_INET)
	{
		ret = if_set_prefix(ifc->ifp, ifc);
		if(ret < 0)
		{
			zlog_warn("Set ip(V4) in kernel failed(%s).\n",safe_strerror(errno));
			if(errno == EAGAIN || errno == ENODEV )/*no dev or Resource temporarily unavailable*/
			{
				ifc->ipv4_set_fail++;/*set counter.*/
				if(ifc->ipv4_set_fail > 6)
				{
					zlog_warn("To set ip(V4) in kernel failed(%s).\n",safe_strerror(errno));
					return -1;
				}
				thread_add_timer(zebrad.master,set_ip_address_again_by_thread_timer,ifc,1);
				return 0;
			}
		}
		ifc->ipv4_set_fail = 0;/*clear counter.*/
	}
	else
	{
		ret = if_prefix_add_ipv6(ifc->ifp, ifc);
		if(ret < 0)
		{
			zlog_warn("Set ip(V6) in kernel failed(%s).\n",safe_strerror(errno));
			if(errno == EAGAIN || errno == ENODEV )/*no dev or Resource temporarily unavailable*/
			{
				ifc->ipv6_set_fail++;/*set counter.*/
				if(ifc->ipv6_set_fail > 6)
				{
					zlog_warn("To set ip(V6) in kernel failed(%s).\n",safe_strerror(errno));
					return -1;
				}
				thread_add_timer(zebrad.master,set_ip_address_again_by_thread_timer,ifc,1);
				return 0;
			}
		}
		ifc->ipv6_set_fail = 0;/*clear counter.*/
		SET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
		
		
	}
	interface_ip_add_userspace(ifc);

	return 0;
	
}


struct connected *
tipc_vice_connected_add_by_prefix (struct interface *ifp, struct prefix *p, 
                         struct prefix *destination, u_char ipv6_config )
{
  struct listnode *node3;
  struct connected *ifc = NULL;
  struct prefix *q;
  int ret;
  
  ifc = connected_check (ifp, p);
  if(ifc!= NULL) 
  {
    zlog_info("%s:line %d,######### interface(%s) scope (%u)##########\n",__func__,__LINE__,ifp->name,ifp->if_scope);
	if((product->board_type == BOARD_IS_BACKUP_MASTER )
		||(product->board_type == BOARD_IS_VICE)
		&& (!(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))))
	{
	   		zlog_info("Bakup master or Vice board , ifc have address , so install kernel .\n ");
		/*	ifc->ifp = ifp;///////////////delet/////////////////////////////
			ifc->ifp->if_scope = ifp->if_scope;
		*/
			if(ifc->address->family == AF_INET)
			{
				ret = if_set_prefix (ifp, ifc);
			    if (ret < 0) 
				{
					if(errno == EEXIST)
					 {
					   zlog_info("%s:line %d,IP address is exist already.\n",__func__,__LINE__);
					 }
					else
					{
					  zlog_err("%s: line %d, kernel set ipv4 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
			   		  return NULL;
					}
			    }
			}
			else if(ifc->address->family == AF_INET6)
			{
				if(tipc_server_debug)
				 zlog_debug("%s : line %d ,ifc->conf[%d] , ifc->ipv6_config[%d], ipv6_config[%d].\n",
				     __func__,__LINE__,ifc->conf,ifc->ipv6_config,ipv6_config);

				ifc->ipv6_config = ipv6_config;
				
				UNSET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
				SET_FLAG(ifc->conf,ZEBRA_IFC_CONFIGURED);	/*10*/
				
				zlog_debug("%s : line %d ,ifc->conf = %d .\n",__func__,__LINE__,ifc->conf);
				ret = if_prefix_add_ipv6 (ifc->ifp, ifc);
			    if (ret < 0) 
				{
				 if(errno == EEXIST)
				  {
					zlog_info("%s:line %d,IP address is exist already.\n",__func__,__LINE__);
				  }
				  else
				  {
			    	zlog_warn("%s: line %d, kernel set ipv6 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
					/*router_id_del_address(ifc);
					listnode_delete(ifp->connected, ifc);
					connected_free(ifc);*/
			   		return NULL;
				  }
			    }
				
				SET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
			  }
			else
			{
					
					zlog_debug("%s : line %d , errXXXX  %d ....\n",__func__,__LINE__,ifc->address->family);
				}
		/*	return ifc; */////////////////////null///////////////////////
			return NULL;
	   	  }

   	  if(tipc_server_debug)
	   zlog_debug("%s: line %d , the interface %s add the same address , so couldn't go on ....\n",__func__,__LINE__,ifp->name);
	  return NULL;
   }
  if((product->board_type == BOARD_IS_VICE)
	  &&(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	  &&((judge_real_local_interface(ifp->name))!=LOCAL_BOARD_INTERFACE))
  	{
	  zlog_info("Vice board: interface(%s) is set local , but not real local interface , so not creat ifc and not install kernel. \n ",ifp->name);
	  return NULL;
  	}
  
  /* Allocate new connected address. */
  ifc = connected_new ();
  ifc->ifp = ifp;
  if(tipc_server_debug)
  zlog_debug("%s, ifc->ifp->name == %s",__func__ ,ifc->ifp->name);
  /* Fetch interface address */
  ifc->address = prefix_new();
  memcpy (ifc->address, p, sizeof(struct prefix));
  ifc->ipv6_config = ipv6_config;
  	

  /* Fetch dest address */
  if (destination)
  {   
    ifc->destination = prefix_new();
    memcpy (ifc->destination, destination, sizeof(struct prefix));
    
  }

  if((judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE) 
  	||(judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE)
  	||(judge_eth_interface(ifp->name)==ETH_INTERFACE))/*ve sub interface , eth sub interface and eth interface deal as normal*/
   {
   	  int slot;
   	 
	/*  ifp->devnum = 0;*/ /*gujd: 2012-05-05, am 11:50. Change for delete rpa.*/
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
  if (ifp->if_types == VIRTUAL_INTERFACE )
  {	
  	int k;
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
	 	/*zlog_debug("%s : line %d, creat rpa interface sucess, so virtual turn to rpa interface...\n",__func__,__LINE__);*/
	  }
	 
	 /*gujd : 2012-10-13 . Optimized code . Send to ospf waiting for getting index by netlink.*/  
#if 0
	 sleep(1);
	 
	 ifp->ifindex = if_get_index_by_ioctl(ifp);
  	 if(ifp->ifindex <= 0)
  	  {
  		zlog_warn("get rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
		return NULL;
  	  }
 	 zlog_debug("get rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);
	 
	 ifc->ifp->ifindex = ifp->ifindex;/**ifc 下的ifp中ifindex也要更新，ifc用于后面发送到动态路由进程*/
	 SET_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE);//////////////////////////
	 vice_redistribute_interface_add(ifp);
#else
	ifp->ifindex = if_get_index_by_ioctl(ifp);
	if(ifp->ifindex <= 0)
	 {
		zlog_warn("get rpa ifindex by ioctl failed [%s], go to get again by loop ..\n",safe_strerror(errno));
	 	for(k=0; k<= 12000;k++)/*in all is 2s*/
		{
		  usleep(10000);/*every time is 10ms*/
		 
		 ifp->ifindex = if_get_index_by_ioctl(ifp);
	  	 if(ifp->ifindex <= 0)
	  	  {
	  		/*zlog_warn("get rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));*/
			continue;
	  	  }
		 else
		 {
		 	/*zlog_debug("%s: line %d ,####### k=[%d]#######\n",__func__,__LINE__,k);*/
			break;
		 }
		 /*CID 14459 (#1 of 1): Structurally dead code (UNREACHABLE)
		     unreachable: This code cannot be reached: "zlog_warn("%s: line %d, get...".
		     So move it to below.*/
		 /*zlog_warn("%s: line %d, get index failed ,overflow 2s. k[%d] \n",__func__,__LINE__,k);
		 return NULL;*/
		}
		/*move the warn log here.*/
		if(k >= 12000)
		{
		 zlog_warn("%s: line %d, get index failed ,overflow 2s. k[%d] \n",__func__,__LINE__,k);
		 /*CID 13544 (#2 of 2): Resource leak (RESOURCE_LEAK)
                 25. leaked_storage: Variable "ifc" going out of scope leaks the storage it points to. 
                 No problem. The ifc will continue use in somewhere. When delete ip address ,it will free.*/
		 return NULL;
		}
	 }

 	 zlog_debug("get rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);
	 
	 ifc->ifp->ifindex = ifp->ifindex;/**ifc 下的ifp中ifindex也要更新，ifc用于后面发送到动态路由进程*/
	 SET_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE);/*netlink use too*/
	 vice_redistribute_interface_add(ifp);
#endif 
  	}	
  #endif
 /* ret = ip_address_count(ifp);*/
   ret = ip_address_count_except_fe80(ifp);
	if (ret < 1)
	{
		 if(ifp->if_types == REAL_INTERFACE || ifp->if_types == 0)
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

	}

	skip:
#if 1	
	  if(tipc_server_debug)
	  zlog_debug("%s : line %d ,-------vice---------go to add address......\n",__func__,__LINE__);
	/* Add connected address to the interface. */  
   listnode_add (ifp->connected, ifc);
	
#endif

	if((product->board_type == BOARD_IS_BACKUP_MASTER)
		&&(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))
	 {
	 	if(product->product_type == PRODUCT_TYPE_8610
			|| product->product_type == PRODUCT_TYPE_8606
			||product->product_type == PRODUCT_TYPE_8800)
	 	{
	 	  zlog_info("(8610)Bakup master : interface (%s) is set local , so not install ip to kernel !\n",ifp->name);
		  return ifc;
	 	}
		if((product->product_type == PRODUCT_TYPE_7605I
			||product->product_type == PRODUCT_TYPE_8603)
			&&((judge_real_local_interface(ifp->name))!=LOCAL_BOARD_INTERFACE))
		{
	 	  zlog_info("(7605)Bakup master : interface (%s) is set local and not local board interface, so not install ip to kernel !\n",ifp->name);
		  return ifc;
	 	}
		
	 }
	
skip2:
#if 0	
	if(ifc->address->family == AF_INET)
	{
		/*zlog_debug("%s : line %d , ipv4 : %d ....\n",__func__,__LINE__,ifc->address->family);*/
	    /*add by gjd: for rpa and real interface to add address to kernel*/
		ret = if_set_prefix (ifc->ifp, ifc);
	    if (ret < 0) 
		{
	    	zlog_warn("%s: line %d, kernel set ipv4 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
			zlog_info("#### ret = %d, errno = %d ####\n",ret,route_boot_errno);
			/*gujd: 2012-11-15, pm 4:00. When creat rpa, kernel not ready creat the netdev, then install ip will failed. So set timer to repeat install ip.*/
			if(route_boot_errno == -EAGAIN || route_boot_errno == -ENODEV )
			{
				zlog_info("%s : line %d, to set timer to install ip to kernel.\n",__func__,__LINE__);
				thread_add_timer(zebrad.master,set_ip_address_again_by_thread_timer,ifc,1);
			}
	   		return NULL;
	    }
	  }
	else
		if(ifc->address->family == AF_INET6)
		{
		/*	zlog_debug("%s : line %d , ipv6 : %d ....\n",__func__,__LINE__,ifc->address->family);
			zlog_debug("%s : line %d ,ifc->conf = %d .\n",__func__,__LINE__,ifc->conf);*/
			UNSET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
			SET_FLAG(ifc->conf,ZEBRA_IFC_CONFIGURED);	
			
			/*zlog_debug("%s : line %d ,ifc->conf = %d .\n",__func__,__LINE__,ifc->conf);*/
		    /*add by gjd: for rpa and real interface to add address to kernel*/
			ret = if_prefix_add_ipv6 (ifc->ifp, ifc);
		    if (ret < 0) 
			{
				zlog_warn("%s: line %d, kernel set ipv6 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
				if(errno == EAGAIN || errno == ENODEV )
				{
					zlog_info("%s : line %d, to set timer to install ip to kernel.\n",__func__,__LINE__);
					thread_add_timer(zebrad.master,set_ip_address_again_by_thread_timer,ifc,1);
				}
		   		return NULL;
		    }
			
			SET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
		  }
		else
			{
				
				zlog_debug("%s : line %d , errXXXX  %d ....\n",__func__,__LINE__,ifc->address->family);
			}
#else
	if(ifc->address->family == AF_INET)
	{
		/*zlog_debug("%s : line %d , ipv4 : %d ....\n",__func__,__LINE__,ifc->address->family);*/
	    /*add by gjd: for rpa and real interface to add address to kernel*/
		ret = if_set_prefix (ifc->ifp, ifc);
	    if (ret < 0) 
		{
	    	zlog_warn("%s: line %d, kernel set ipv4 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
			router_id_del_address(ifc);
			listnode_delete(ifp->connected, ifc);
			connected_free(ifc);
	   		return NULL;
	    }
	  }
	else if(ifc->address->family == AF_INET6)
		{
		/*	zlog_debug("%s : line %d , ipv6 : %d ....\n",__func__,__LINE__,ifc->address->family);
			zlog_debug("%s : line %d ,ifc->conf = %d .\n",__func__,__LINE__,ifc->conf);*/
			UNSET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
			SET_FLAG(ifc->conf,ZEBRA_IFC_CONFIGURED);	
			
			/*zlog_debug("%s : line %d ,ifc->conf = %d .\n",__func__,__LINE__,ifc->conf);*/
		    /*add by gjd: for rpa and real interface to add address to kernel*/
			ret = if_prefix_add_ipv6 (ifc->ifp, ifc);
		    if (ret < 0) 
			{
				if(errno == EEXIST)
				  {
					zlog_info("%s:line %d,IP address is exist already.\n",__func__,__LINE__);
				  }
		  		else
			  	 {
		    	zlog_warn("%s: line %d, kernel set ipv6 address failed :%s....\n", __func__, __LINE__,safe_strerror(errno));
					router_id_del_address(ifc);
					listnode_delete(ifp->connected, ifc);
					connected_free(ifc);
			   		return NULL;
			  	  }
		    }
			
			SET_FLAG(ifc->conf,ZEBRA_IFC_REAL);
		  }
		else
			{
				
				zlog_debug("%s : line %d , errXXXX  %d ....\n",__func__,__LINE__,ifc->address->family);
			}

#endif		
#if 0	
	if(tipc_server_debug)
  	zlog_debug("%s : line %d ,-------vice---------go to add address......\n",__func__,__LINE__);
  /* Add connected address to the interface. */  
 listnode_add (ifp->connected, ifc);
  
#endif
  return ifc;
}




struct connected *
tipc_vice_connected_delete_by_prefix (struct interface *ifp, struct prefix *p,u_char ipv6_config)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  int ret = 0;
  

if((judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE) 
	||(judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE))/*ve sub interface and eth sub interface deal as normal*/
/*	||(judge_eth_interface(ifp->name)==ETH_INTERFACE))*/
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
   /*	ifp->devnum = 0; *//*gujd: 2012-05-05, am 11:50. Change for delete rpa.*/
		slot = get_slot_num(ifp->name);
		ifp->slot = slot;
		if(slot == product->board_id)	/*real*/
			ifp->if_types = REAL_INTERFACE;
		else							/*rpa*/
			ifp->if_types = RPA_INTERFACE;
		
   		goto skip;
	}
 /*2011-09-29: pm 6:25*/

/*  ret = search_ipv4(ifp);*//**gjd : check under ifp, if have ip or not? if don't have ,**/
 /**stop ret<1 and don't go to let kernel delete,in future it can used for check rpa and virtal interface **/
 	ret = ip_address_count(ifp);

	if(tipc_server_debug)
		zlog_info("There is %d ip address on interface %s .\n",ret,ifp->name);
	if (ret < 1)
	{		   
  	  if(tipc_server_debug)
		zlog_debug("%s : line %d , there is no ip (in future : used to check it's not RPA interafce!)\n",__func__,__LINE__);
	  return NULL;
 	 }   	
  	else if (ret == 1)
	 {
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
						
						zlog_debug("%s, line %d, kernel uninstall ipv4 address failed", __func__, __LINE__);
		        		return NULL;
		        	}
		       	}
			  else if(ifc->address->family == AF_INET6)/*ipv6*/
			  	{
			  		ifc->ipv6_config = ipv6_config;
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
			   if(ifc->address->family == AF_INET)
			   	{
	            	listnode_delete (ifp->connected, ifc);
			   	}
			   else
			   	{
			   		if(!CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
						listnode_delete (ifp->connected, ifc);
					else
					 {
					 	zlog_debug("%s: line %d, Cannot delete IPv6 config addr .\n",__func__,__LINE__);
					 }
			   	}
				#if 1
			   if(ifp->if_types == REAL_INTERFACE )
				  {
				   if((strncmp(ifp->name, "ve",2) != 0 && strncmp(ifp->name, "obc",3)!= 0))
				  	{
						unregister_rpa_interface_table(ifp);
	/*				ifp->if_types = 0; */ /**置回0**/
						
				  	}
				  }
			   
		 		if(ifp->if_types == RPA_INTERFACE)
				  {
					SET_FLAG(ifp->ip_flags,IPV4_ADDR_DISTRIBUTE_DEL);/*2011-10-10: pm 3:05 ,add for avoid vice or bakup master board when unregister rpa interface to send delet rpa interface info to active master board . */
					delete_rpa_interface(ifp);
				/*	vice_redistribute_interface_delete(ifp);*/ /*gjd : delete by WCG bug of AXSSZFI-229*/
				  }
				#endif
	            return ifc;
	        }		
	      } 
		  		
	  }
  	else if (ret > 1)	
	 {	
	     /* In case of same prefix come, replace it with new one. */
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
					   
					   zlog_debug("%s, line %d, kernel uninstall ipv4 address failed", __func__, __LINE__);
					   return NULL;
				   }
			   }
			  else if(ifc->address->family == AF_INET6)/*ipv6*/
			   {
				   ifc->ipv6_config = ipv6_config;
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
	        /* listnode_delete (ifp->connected, ifc);*/
			 if(ifc->address->family == AF_INET)
			   	{
	            	listnode_delete (ifp->connected, ifc);
			   	}
			   else
			   	{
			   		if(!CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
						listnode_delete (ifp->connected, ifc);
					else
					 {
					 	zlog_debug("%s: line %d, Cannot delete IPv6 config addr .\n",__func__,__LINE__);
					 }
			   	}
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
	     		zlog_debug("%s, line %d, kernel uninstall ipv4 address failed:( %s).\n", 
					__func__, __LINE__,safe_strerror(errno));
				if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
		       	{
					int slot_num = 0;
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)
					{
					  zlog_info("%s : line %d , interface(%s) is set local , when kernel delete ip failed , go on to delete ip under Rtmd .\n",
					  			__func__,__LINE__,ifp->name);
					  listnode_delete (ifp->connected, ifc);
		        	  return ifc;
					}
		  		}
				else
				{
	     			return NULL;
					}
	     	}
		}
		 else if(p->family== AF_INET6 )/*ipv6*/
		 {
			ifc->ipv6_config = ipv6_config;
		 	ret = if_prefix_delete_ipv6(ifc->ifp, ifc);
	     	if (ret < 0) 
			{
	     		zlog_debug("%s, line %d, kernel uninstall ipv6 address failed:( %s).\n", 
					__func__, __LINE__,safe_strerror(errno));
				if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
		       	{
					int slot_num = 0;
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)
					{
					  zlog_info("%s : line %d , interface(%s) is set local , when kernel delete ip failed , go on to delete ip under Rtmd .\n",
					  			__func__,__LINE__,ifp->name);
					  listnode_delete (ifp->connected, ifc);
		        	  return ifc;
					}
		  		}
				else
				{
	     			return NULL;
					}
	     	}
		  }
		 else/*err : other*/
		 {
		 		zlog_err("%s : line %d , unkown protocol %d .\n",__func__,__LINE__,p->family);
				return NULL;
		 	}
	     #endif	
        /* listnode_delete (ifp->connected, ifc);*/
        if(ifc->address->family == AF_INET)
	   	{
        	listnode_delete (ifp->connected, ifc);
	   	}
	   else
	   	{
	   		if(!CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
				listnode_delete (ifp->connected, ifc);
			else
			 {
			 	zlog_debug("%s: line %d, Cannot delete IPv6 config addr .\n",__func__,__LINE__);
			 }
	   	}
	   
        return ifc;
       }
     }
  
  return NULL;
}



struct connected *
tipc_vice_zebra_interface_address_read (int type, struct stream *s)
{	
  unsigned int ifindex;
  struct interface *ifp;
  struct connected *ifc;
  struct prefix p, d;
  char ifname_tmp[INTERFACE_NAMSIZ];
  int family;
  int plen;
  int ret;
  u_char ifc_flags;
  u_char ifc_conf, ipv6_config;  
  char buf[BUFSIZ];/**for debug**/

  memset (&p, 0, sizeof(p));
  memset (&d, 0, sizeof(d));
  	
  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
  if(tipc_server_debug)
   zlog_debug("%s: line %d, interface_name == %s\n",__func__, __LINE__ ,ifname_tmp);
  /*gujd: 2012-02-14, am 11:16.*/
  if ((judge_vlan_interface(ifname_tmp)== VLAN_INTERFACE)&&(product->board_type == BOARD_IS_VICE))
  	{
	 zlog_debug("%s, VICE board not support VLAN interface(%s).\n",__func__,ifname_tmp);
	 return NULL;
  }

  /* Lookup this by interface name. */
  ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
  /* If such interface does not exist, indicate an error */
  if (ifp == NULL) {
	 zlog_debug(" %s: %s do not exist or the tipc interface not match!\n",__func__,ifname_tmp);
     return NULL;
  	}
 ifc_conf =	stream_getc(s);
 ipv6_config = stream_getc(s);

  /* Fetch flag. */
  ifc_flags = stream_getc (s);
  ifp->if_scope = stream_getc(s);

  /* Fetch interface address. */
  family = p.family = stream_getc (s);
  plen = prefix_blen (&p);
  stream_get (&p.u.prefix, s, plen);
  p.prefixlen = stream_getc (s);
  
  #if 1
   ifp->slot = stream_getl(s);
   ifp->devnum = stream_getl(s);
	if(tipc_server_debug)
		zlog_debug(" %s : line %d , when add/del ip recv rpa interface: slot %d , devnum %d .\n",__func__,__LINE__,ifp->slot,ifp->devnum);
#endif

  /* Fetch destination address. */
  stream_get (&d.u.prefix, s, plen);
  d.family = family;

  /*if(tipc_server_debug)*/ /*turn on ,to help judge the active master send ip message or not  to vice ?*/
	zlog_debug ("VICE recv[%s] interface %s : ip address %s/%d ",zserv_command_string(type),ifp->name,inet_ntop (p.family, &p.u.prefix, buf, BUFSIZ), p.prefixlen);


  if (type == ZEBRA_INTERFACE_ADDRESS_ADD) 
    {
       /* N.B. NULL destination pointers are encoded as all zeroes */
       ifc = tipc_vice_connected_add_by_prefix(ifp, &p,(memconstant(&d.u.prefix,0,plen) ?
					      NULL : &d),ipv6_config);
       if (ifc != NULL)
       	{
          ifc->conf = ifc_conf;
          ifc->flags = ifc_flags;
       	}
    }
  else if(type == ZEBRA_INTERFACE_ADDRESS_DELETE)
    {
      
      ifc = tipc_vice_connected_delete_by_prefix(ifp, &p,ipv6_config);
    }
  else
  {
	return NULL;
  }
	
  return ifc;
 }


int
tipc_vice_interface_address_add (int command, tipc_server *vice_board,
			   zebra_size_t length)
{
  struct connected *ifc = NULL;
  int ret;
  
  ifc = tipc_vice_zebra_interface_address_read (ZEBRA_INTERFACE_ADDRESS_ADD, 
                                      vice_board->ibuf);
  if (ifc == NULL){
  	if(tipc_server_debug)
   		zlog_debug(" %s ,line = %d receive not match message  ifc(NULL).", __func__, __LINE__);
    	return -1;
   	}
/*
if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	{
		zlog_info("To set ZEBRA_IFC_CONFIGURED .\n");
		SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
	}
*/
#if 1 
 if(ifc->address->family == AF_INET)
 {
	 ret = if_subnet_add (ifc->ifp, ifc);
	 if (ret < 0) {
	 	if(tipc_server_debug)
	    	zlog_debug("%s, line %d, if_subnet_add failed\n", __func__, __LINE__);
			return -1;
	 }
  }
 #endif

	vice_redistribute_interface_address_add(ifc->ifp, ifc);

	if(ifc->address->family == AF_INET)
	{
/**add  for connect route 2011-04-15**/
      if (if_is_operative(ifc->ifp))
      {
        if(tipc_server_debug)
      	 zlog_debug("%s : line %d, go to creat ipv4 connect route ....\n",__func__,__LINE__);
		connected_up_ipv4 (ifc->ifp, ifc);  
      }
	}
	else if(ifc->address->family == AF_INET6)
	 {
		SET_FLAG (ifc->conf, ZEBRA_IFC_REAL); 
		 if (if_is_operative(ifc->ifp))
		 {
	        if(tipc_server_debug)
	      	 zlog_debug("%s : line %d, go to creat ipv6 connect route ....\n",__func__,__LINE__);
			connected_up_ipv6 (ifc->ifp, ifc);  
		  }
	  }
	else
	{
		zlog_debug("XXXX err !\n");
	}
  return 0;
}



int
tipc_vice_interface_address_delete (int command, tipc_server *vice_board,
			      zebra_size_t length)
{
  struct connected *ifc;
  int ret;
  
  ifc = tipc_vice_zebra_interface_address_read (ZEBRA_INTERFACE_ADDRESS_DELETE,
                                      vice_board->ibuf);
  if (ifc == NULL){
  	if(tipc_server_debug)
   		zlog_debug(" %s , line %d,  tipc_interface_address_read  ifc == NULL", __func__, __LINE__);
    	return -1;
   	}
/*
  if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)) 
  	{
	  	if(tipc_server_debug)
	  	   zlog_debug("%s: line %d, ZEBRA_IFC_CONFIGURED",__func__,__LINE__);
	  	UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
	}
	*/
  vice_redistribute_interface_address_delete(ifc->ifp, ifc);
  
  if(ifc->address->family == AF_INET)
  {
	ret = if_subnet_delete (ifc->ifp, ifc);
	if (ret < 0) {
	  if(tipc_server_debug)
		  zlog_debug("%s: line %d, if_subnet_delete failed", __func__, __LINE__);
		  return -1;  
	}
  }
  
  /*free connect route*/
  if(tipc_server_debug)
   zlog_debug("%s : line %d , go to del connect route ......\n",__func__,__LINE__);
  if(ifc->address->family == AF_INET)
  {
     if(tipc_server_debug)
      zlog_debug("%s : line %d , go to del ipv4 connect route ......\n",__func__,__LINE__);
     connected_down_ipv4 (ifc->ifp, ifc);
  	}
  else
  {
  	 if(tipc_server_debug)
      zlog_debug("%s : line %d , go to del ipv6 connect route ......\n",__func__,__LINE__);
  	 connected_down_ipv6(ifc->ifp, ifc);
  	}
  
   if(ifc->address->family == AF_INET)
   {
	 if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)) 
	   {
		   if(tipc_server_debug)
			  zlog_debug("%s: line %d, ZEBRA_IFC_CONFIGURED",__func__,__LINE__);
		   UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
	   }
	   router_id_del_address(ifc);/*add*/
	   listnode_delete (ifc->ifp->connected, ifc);
	   connected_free(ifc);
   }
   else
   {
	   if(!CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
	   	{/*here*/
		 if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)) 
		   {
			   if(tipc_server_debug)
				  zlog_debug("%s: line %d, ZEBRA_IFC_CONFIGURED",__func__,__LINE__);
			   UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
		   }
		   router_id_del_address(ifc);/*add*/
		   listnode_delete (ifc->ifp->connected, ifc);
		   connected_free(ifc);
	   	}
	   else
		{
		   zlog_debug("%s: line %d, Cannot delete IPv6 config addr .\n",__func__,__LINE__);
		}
     }
  /*
  router_id_del_address(ifc);
  listnode_delete (ifc->ifp->connected, ifc);
  connected_free(ifc);
  */
  return 0;
}


struct interface *
tipc_vice_zebra_interface_description_update_read (struct stream *s, int command)
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
		  
		  if(tipc_server_debug)
		  	zlog_debug("interface(%s),set local descripton(%s) is local board.\n",ifp->name,ifp->desc);
	  	 }
		 else
		  {
			if(ifp->desc)/*add.*/
			  XFREE (MTYPE_TMP, ifp->desc); 
			if(tipc_server_debug)
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
	 UNSET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
	 if(ifp->desc)/*free before, then update.*/
	  XFREE (MTYPE_TMP, ifp->desc);
	 ifp->desc = NULL;
  	}
  
  return ifp;

}


int
tipc_vice_interface_description_set (int command, tipc_server *vice_board,
			   zebra_size_t length)
{
  struct interface *ifp = NULL;
  struct stream *s = NULL;
  s = vice_board->ibuf;
  int ret;

  ifp = tipc_vice_zebra_interface_description_update_read(s,command);
  if(!ifp)
  	zlog_debug("%s: line %d, the interface doesn't exist!\n",__func__,__LINE__);

 
  return 0;
}

int
tipc_vice_interface_description_unset (int command, tipc_server *vice_board,
			   zebra_size_t length)
{
  struct interface *ifp = NULL;
  struct stream *s = NULL;
  s = vice_board->ibuf;
  int ret;

  ifp = tipc_vice_zebra_interface_description_update_read(s,command);
  if(!ifp)
  	zlog_debug("%s: line %d, the interface doesn't exist!\n",__func__,__LINE__);

 
  return 0;
}

#if 0
int
tipc_vice_interface_packets_statistics_request(int command,tipc_server * vice_board,int length)
{
	/*vice board deal with the request , so go to packet the interface packets info by stream_putXX, then send to master .
	At present , the interface isn't included the "radio" or "r " interface .*/
	
  struct listnode *node, *nnode;
  struct listnode *ifnode, *ifnnode;
  struct interface *ifp;
  struct stream *s;
  char ifname[INTERFACE_NAMSIZ];

  memset(ifname, 0, INTERFACE_NAMSIZ);
  
  if(vice_board->ibuf)
  	s = vice_board->ibuf;
  else
  {
  	zlog_warn("%s : line %d ibuf is NULL .\n",__func__,__LINE__);
  	return -1;
  	}
  
  stream_get(ifname,s,INTERFACE_NAMSIZ);
  if(judge_real_local_interface(ifname) != LOCAL_BOARD_INTERFACE)
  	{  		
		if(tipc_server_debug)
  		 zlog_debug("%s not local board interface .\n",ifname);
  		return -1;
  	}

  
  
#ifdef HAVE_PROC_NET_DEV
  /* If system has interface statistics via proc file system, update
     statistics. */
  ifstat_update_proc ();
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_NET_RT_IFLIST
  ifstat_update_sysctl ();
#endif /* HAVE_NET_RT_IFLIST */

  ifp = if_lookup_by_name(ifname);
  if(ifp == NULL)
  {
  	zlog_warn("%s is not local board interface .\n",ifname);
	return -1;
  	}
  for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
  {
 	 if(tipc_server_debug)
      zlog_debug("%s: line%d, ifp->name = %s", __func__, __LINE__, ifp->name); 
     vice_send_interface_packets_statistics(vice_board, ifp);
    	
   }


 return 0;
	
	
}
#endif
#if 1
int
tipc_vice_interface_packets_statistics_request_all(int command,tipc_server * vice_board,int length)
{
	/*vice board deal with the request , so go to packet the interface packets info by stream_putXX, then send to master .
	At present , the interface isn't included the "radio" or "r " interface .*/
	
  struct listnode *node, *nnode;
  struct listnode *ifnode, *ifnnode;
  struct interface *ifp;
/*  u_int16_t value = 0;*/
  int if_flow_command;
  struct stream *s = vice_board->ibuf;
  
  /*In fact, this value unuseful . Only use it for counter(tipc-config -pi) .*/
 /* value = stream_getw(s);*/
  if_flow_command = stream_getl(s);
  /*send request to se_agent by tipc*/
  rtm_send_request_if_flow_to_se_agent(if_flow_command);

#if 0
  
#ifdef HAVE_PROC_NET_DEV
  /* If system has interface statistics via proc file system, update
     statistics. */
  ifstat_update_proc ();
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_NET_RT_IFLIST
  ifstat_update_sysctl ();
#endif /* HAVE_NET_RT_IFLIST */

//  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
if(!iflist)
{
	zlog_warn("At preset iflist is NULL .\n");
	return -1;
}
  for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
  	{
		if((memcmp(ifp->name,"radio",5) == 0)			
			||(memcmp(ifp->name,"r",1) == 0)
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
		if(judge_real_local_interface(ifp->name) == LOCAL_BOARD_INTERFACE)
		{
			for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
		     {
		       if(tipc_server_debug)
		         zlog_debug("%s: line%d, ifp->name = %s", __func__, __LINE__, ifp->name); 
		       vice_send_interface_packets_statistics(vice_board, ifp);
			   usleep(fetch_rand_time_interval());/*need , avoid to block master.*/
		        	
		      }
			}
		else
			continue;
		
  	}
#endif

 return 0;
	
	
}
#else
int
tipc_vice_interface_packets_statistics_request_all(int command,tipc_server * vice_board,int length)
{
	/*vice board deal with the request , so go to packet the interface packets info by stream_putXX, then send to master .
	At present , the interface isn't included the "radio" or "r " interface .*/
	
  struct listnode *node, *nnode;
  struct listnode *ifnode, *ifnnode;
  struct interface *ifp;
  u_int16_t value = 0;
  struct stream *s = vice_board->ibuf;
  
  /*In fact, this value unuseful . Only use it for counter(tipc-config -pi) .*/
  value = stream_getw(s);
  
#ifdef HAVE_PROC_NET_DEV
  /* If system has interface statistics via proc file system, update
     statistics. */
  ifstat_update_proc ();
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_NET_RT_IFLIST
  ifstat_update_sysctl ();
#endif /* HAVE_NET_RT_IFLIST */

//  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
if(!iflist)
{
	zlog_warn("At preset iflist is NULL .\n");
	return -1;
}
  for (ALL_LIST_ELEMENTS (iflist, ifnode, ifnnode, ifp))
  	{
		if((memcmp(ifp->name,"radio",5) == 0)			
			||(memcmp(ifp->name,"r",1) == 0)
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
		if(judge_real_local_interface(ifp->name) == LOCAL_BOARD_INTERFACE)
		{
			for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, vice_board))
		     {
		       if(tipc_server_debug)
		         zlog_debug("%s: line%d, ifp->name = %s", __func__, __LINE__, ifp->name); 
		       vice_send_interface_packets_statistics(vice_board, ifp);
			   usleep(fetch_rand_time_interval());/*need , avoid to block master.*/
		        	
		      }
			}
		else
			continue;
		
  	}

 return 0;
	
	
}

#endif

/* Inteface link up message processing */
int
tipc_vice_interface_up (int command, tipc_server *vice_board, zebra_size_t length)
{

  struct interface *ifp;
  int ret;
  
  if (vice_board == NULL) {
  	if(tipc_server_debug)
  	zlog_debug ("%s, line = %d, zclient is NULL", __func__, __LINE__);
  	return -1;
  } 
  

  /* zebra_interface_state_read () updates interface structure in
     iflist. */
    
  ifp = tipc_vice_zebra_interface_state_read (ZEBRA_INTERFACE_UP,vice_board->ibuf);
  if(!ifp)
  	zlog_debug("%s: line %d, the interface doesn't exist!\n",__func__,__LINE__);

  return 0;
}

/* Inteface uplink flag .*/
int
tipc_vice_interface_uplink_flag_update(int command, tipc_server *vice_board, zebra_size_t length)
{

  struct interface *ifp;
  int ret;
  
  if (vice_board == NULL) {
  	if(tipc_server_debug)
  	zlog_debug ("%s, line = %d, zclient is NULL", __func__, __LINE__);
  	return -1;
  } 
  

  /* zebra_interface_state_read () updates interface structure in
     iflist. */
    
  ifp = tipc_vice_zebra_interface_uplink_flag_state_read(command,vice_board->ibuf);
  if(!ifp)
  	zlog_debug("%s: line %d , interface doesn't exist!\n",__func__,__LINE__);
  
  return 0;
}

/* Inteface linkdetection enable*/
int
tipc_vice_interface_linkdetection_enable(int command, tipc_server *vice_board, zebra_size_t length)
{

  struct interface *ifp;
  int ret;
  
  if (vice_board == NULL) {
  	if(tipc_server_debug)
  	zlog_debug ("%s, line = %d, zclient is NULL", __func__, __LINE__);
  	return -1;
  } 
  

  /* zebra_interface_state_read () updates interface structure in
     iflist. */
    
  ifp = tipc_vice_zebra_interface_state_read (ZEBRA_INTERFACE_LINKDETECTION_ENABLE,vice_board->ibuf);
  if(!ifp)
  	zlog_debug("%s: line %d , interface doesn't exist!\n",__func__,__LINE__);
  return 0;
}





/* Inteface linkdetection disable*/
int
tipc_vice_interface_linkdetection_disable(int command, tipc_server *vice_board, zebra_size_t length)
{

  struct interface *ifp;
  int ret;
  
  if (vice_board == NULL) {
  	if(tipc_server_debug)
  	zlog_debug ("%s, line = %d, zclient is NULL", __func__, __LINE__);
  	return -1;
  } 
  

  /* zebra_interface_state_read () updates interface structure in
     iflist. */
    
  ifp = tipc_vice_zebra_interface_state_read (ZEBRA_INTERFACE_LINKDETECTION_DISABLE,vice_board->ibuf);
  if(!ifp)
  	zlog_debug("%s: line %d , interface doesn't exist!\n",__func__,__LINE__);
  return 0;
}



//Inteface link down message processing. 
int
tipc_vice_interface_down (int command, tipc_server *vice_board, zebra_size_t length)
{
  struct interface *ifp;
  int ret;
  
  if (vice_board == NULL) {
  	if(tipc_server_debug)
  	zlog_debug ("%s, line = %d, vice_board is NULL", __func__, __LINE__);
  	return -1;
  }

  /* vice_board_interface_state_read() updates interface structure in iflist. */
  ifp = tipc_vice_zebra_interface_state_read(ZEBRA_INTERFACE_DOWN,vice_board->ibuf);
  if(!ifp)
  	zlog_debug("%s: line %d , interface doesn't exist!\n",__func__,__LINE__);

  return 0;
}



/*Inteface addition message from zebra.*/
int
tipc_vice_interface_add (int command, tipc_server* vice_board, int length)
{
  struct interface *ifp = NULL;
  struct stream *s = NULL;
  s = vice_board->ibuf;
  int ret;

  ifp = tipc_vice_zebra_interface_add_read (s);

  #if 1
  if (ifp == NULL)
  	{
  		zlog_debug("Resently , vlan interface not sync between boards!\n");
    	return -1;
   	}
 #endif
 
#if 1
  if(judge_eth_interface(ifp->name)==ETH_INTERFACE)
  {
 	 if (ifp->if_types == VIRTUAL_INTERFACE)
  	{	
	/*create_rpa_interface();*/
	ifp->slot = get_slot_num(ifp->name);
	ret = create_rpa_interface(ifp);
	 if(ret < 0)
	  {
	 	zlog_debug("%s : line %d, creat %s(cpu) rpa interface failed : %s \n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
		/*CID 13545 (#1 of 6): Resource leak (RESOURCE_LEAK)
               8. leaked_storage: Variable "ifp" going out of scope leaks the storage it points to
		No problem. The ifp will continue used, and will free it when delete it.*/
		return -1;
	  }
	 else
	  {	
		/*virtual interface ==> RPA interface*/
		ifp->if_types = RPA_INTERFACE;
	 	zlog_debug("%s : line %d, creat eth(cpu) rpa interface sucess, so virtual turn to rpa interface...\n",__func__,__LINE__);
	  }
	 /*gujd : 2012-10-13 . Optimized code . Send to ospf waiting for getting index by netlink.*/  
#if 0
	 sleep(1);
	 
	 ifp->ifindex = if_get_index_by_ioctl(ifp);
  	 if(ifp->ifindex <= 0)
  	  {
  		zlog_warn("get eth(cpu) rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
		return -1;
  	  }
 	 /*zlog_debug("get eth(cpu) rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);*/
	 
	 
	 vice_redistribute_interface_add(ifp);
#endif	 
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
  else if(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)
  	{
  		int slot_num = 0;
		
  	/*gujd: 2012-02-24, pm 3:40 . Add code for skiping switch board when add ve sub interface.*/
#if 1
		if((((product->product_type == PRODUCT_TYPE_8603)
			||(product->product_type == PRODUCT_TYPE_8606)
			||(product->product_type == PRODUCT_TYPE_8610)
			||(product->product_type == PRODUCT_TYPE_8800)
			)&&
			(strncmp(product->board_name,AX81_2X12G12S,sizeof(AX81_2X12G12S)) == 0))
			||((product->product_type == PRODUCT_TYPE_7605I)&&
			(strncmp(product->board_name,AX71_2X12G12S,sizeof(AX71_2X12G12S)) == 0)))/*switch board don't go to creat ve sub real*/
		{
			zlog_debug(" The board name is %s , don't go to creat real ve-sub(%s) .\n",product->board_name,ifp->name);
			/*CID 13545 (#2 of 6): Resource leak (RESOURCE_LEAK)
                      8. leaked_storage: Variable "ifp" going out of scope leaks the storage it points to.
			No problem. The ifp will continue used, and will free it when delete it.*/
			return 0;
		}
		
#endif
		slot_num = get_slot_num(ifp->name);/*local board : not to add, Let npd creat, rtmd recv kernel netlink mesg.*/
/*  		if(((ifp->if_types == VIRTUAL_INTERFACE)||(CHECK_FLAG(ifp->if_scope, INTERFACE_GLOBAL)))
			&& (slot_num != product->board_id))/*gujd: 2012-03-13 , pm 5:40 . Add if_scope flags.*/
		if(((ifp->if_types == VIRTUAL_INTERFACE)||(CHECK_FLAG(ifp->if_scope, INTERFACE_GLOBAL)))
			&&(ifp->ifindex <= 0))/*gujd: 2012-03-13 , pm 5:40 . Add if_scope flags.*/
  		{
  			if(product->product_type == PRODUCT_TYPE_7605I)
  			{
  				if(slot_num == product->board_id)/*local board , slot 2*/
	  			{
		  			zlog_debug("%s : line %d , 7605i (slot 2) goto creat [local board] ve sub (global) .",__func__,__LINE__);
					vconfig_create_ve_sub_interface_7605i_local_board(ifp);
					ifp->if_types = REAL_INTERFACE;
					return 0;
	  			  }
				else
				{
		  			zlog_debug("%s : line %d , 7605i (slot 2) goto creat [other board: slot 1] ve sub (global) .",__func__,__LINE__);
					vconfig_create_ve_sub_interface_7605i_other_board(ifp);
					ifp->if_types = REAL_INTERFACE;
					return 0;
	  			  }
			 }
			else
			{
	  			zlog_debug("%s : line %d , vice or bakup goto creat ve sub (global) .",__func__,__LINE__);
				vconfig_create_ve_sub_interface(ifp);
				ifp->if_types = REAL_INTERFACE;
				/*CID 13545 (#3 of 6): Resource leak (RESOURCE_LEAK)
                             15. leaked_storage: Variable "ifp" going out of scope leaks the storage it points to
				No problem. The ifp will continue used, and will free it when delete it.*/
				return 0;
				}
  			}
  	}
  else if(judge_ve_interface(ifp->name)== VE_INTERFACE)/*gujd: 2012-06-06, am 11:03. Add for ve parent interface register RPA table .*/
 {
 	int slot = 0;
	slot = get_slot_num(ifp->name);
	if(product->board_id != slot)/*not local , creat rpa*/
	 {
	 	if(ifp->ifindex == IFINDEX_INTERNAL)
	 	{
		 ifp = create_rpa_interface_and_get_ifindex(ifp);/**creat rpa and register**/
		 #if 0 /*gujd : 212-11-05 ,pm 6:01 . Delete it for use sleep to get index.*/
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
	/*	zlog_info("###########(%s)###########\n",ifp->name);*/
		ret = register_rpa_interface_table(ifp);
		if(ret < 0)
		   zlog_info("Make Sure have add [%s] to rpa table  .\n",ifp->name);
	}
  }
  
#endif

/* vice_redistribute_interface_add(ifp);*/
/*CID 13545 (#4-6 of 6): Resource leak (RESOURCE_LEAK)
11. leaked_storage: Variable "ifp" going out of scope leaks the storage it points to. 
No problem. The ifp will continue used, and will free it when delete it.*/
  return 0;
}




int
tipc_vice_interface_delete (int command, tipc_server *vice_board,
		      zebra_size_t length)
{
  struct interface *ifp;
  struct stream *s;
  
  int ret;

  s = vice_board->ibuf;  
  
  /* zebra_interface_state_read() updates interface structure in iflist */

  ifp = tipc_vice_zebra_interface_state_read(ZEBRA_INTERFACE_DELETE,s);
  
  if (ifp == NULL){
  	if(tipc_server_debug)
   		zlog_debug(" %s ,line = %d  the interface not exist\n", __func__, __LINE__);
    	return -1;
   	}

#if 0
  /* router process, waiting for joint debugging*/
  if (if_is_up (ifp)) {
    if_down(ifp);
  } 
#endif
  if(tipc_server_debug)
   zlog_debug("interface delete %s index %d flags %ld metric %d mtu %d\n",
	    ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);  
/*
if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE) 
   ||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE) )
   */
if(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE) 
{ 
	/*gujd : 2012-07-28 , am 11: 50 . Delete it for change interface local/global switch rule (because vrrp).
	  When set local mode , every board hold the interface (not delete) . Then ip address only sync to 
	  the interface for his local board .*/

#if 0
	int slot = 0;
	slot = get_slot_num(ifp->name);
   if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
   {
   	 
	  if(product->product_type == PRODUCT_TYPE_8610
	  	||product->product_type == PRODUCT_TYPE_8606
	  	||product->product_type == PRODUCT_TYPE_8800)/*8610 and 8606, 8800*/
	  {
		  if(product->board_id == slot || product->board_type == BOARD_IS_BACKUP_MASTER)/*local board*/
		  {
		  	zlog_info("%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
			return 0 ;
		  }
		  else/* not local board*/
		  	{
			  zlog_info("%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
			  
			  char cmdstr[64] = {0};
			  memset(cmdstr, 0 , 64);

			  sprintf(cmdstr,"vconfig rem %s",ifp->name);
			  system(cmdstr);/*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			  return 0;
		  	}
		  }
	  else
	  	if(product->product_type == PRODUCT_TYPE_7605I
			||product->product_type == PRODUCT_TYPE_8603)/*8603 and 7605i*/
		{
		  if(product->board_id == slot )/*local board, different from 86: bakup master go to delete.*/
		  {
		  	zlog_info("%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
			return 0 ;
		  }
		  else/* not local board*/
		  	{
			  zlog_info("%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
			  
			  char cmdstr[64] = {0};
			  memset(cmdstr, 0 , 64);

			  sprintf(cmdstr,"vconfig rem %s",ifp->name);
			  system(cmdstr);/*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			  return 0;
		  	}
		  }
		else
		{
			zlog_warn("Unkown product type !\n");
			return -1;
			}
   }
   else
#endif
   	{
 /*  		if(slot != product->board_id )*//*other board delete, local board let npd delete.*/
 /*gujd: 2012-06-06, am 11:03. Change for all the board (not include active master) to delet ve sub interface by rtmd(instead of npd).*/
   		{
   			vconfig_delete_ve_sub_interface(ifp);
			return 0;/*when listen netlink msg, ospf and rip to delete.*/
   		}
		
   	}
   /*CID 14460 (#1 of 1): Structurally dead code (UNREACHABLE)
	unreachable: This code cannot be reached: "goto skip;".	
	Delete the dead code.*/
   /*goto skip;*/
} 

	if(judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE )
		goto skip;

	/*wlan and ebr set "interface local"*/
/*  if((((strncmp(ifp->name,"wlanl",5)!= 0)&&(strncmp(ifp->name,"wlan",4)==0))
	||((strncmp(ifp->name,"ebrl",4)!= 0)&&(strncmp(ifp->name,"ebr",3)==0)))
  		&& (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))
*/

/*gujd : 2012-07-28 , am 11: 50 . Delete it for change interface local/global switch rule (because vrrp).
When set local mode , every board hold the interface (not delete) . Then ip address only sync to 
 the interface for his local board .*/
 
#if 0
	if(((strncmp(ifp->name,"wlan",4)==0)||(strncmp(ifp->name,"ebr",3)==0))
				&& (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))

  	{
   	  int slot = 0;
	  
	  slot = get_slot_num(ifp->name);
	  if(product->product_type == PRODUCT_TYPE_8610
	  	||product->product_type == PRODUCT_TYPE_8606
	  	||product->product_type == PRODUCT_TYPE_8800)/*8610 and 8606,8800*/
	  {
		  if(product->board_id == slot || product->board_type == BOARD_IS_BACKUP_MASTER)/*local board*/
		  {
		  	zlog_info("%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
			return 0 ;
		  }
		  else/* not local board*/
		  	{
			  zlog_info("%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
			
			 /*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			 if(ifp->if_types == RPA_INTERFACE)
			 {
				ret = delete_rpa_interface(ifp);
			 	if(ret < 0)
			 	{
			 	  zlog_debug("%s : line %d, delete rpa(%s) interface  failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
				  return -1;
				 }
			 	else
			 	{
			 	  zlog_debug("%s : line %d, delete rpa(%s) interface  sucess...\n",__func__,__LINE__,ifp->name);
			  	  return 0;
			 		}
			 	}
			 else
			 	{
		  			zlog_debug("go to delete viral interface (%s) \n",ifp->name);
	   				if_delete(ifp);
	   				return 0;
			 	}
		  	}
		  }
	  else
	  	if(product->product_type == PRODUCT_TYPE_7605I
			||product->product_type == PRODUCT_TYPE_8603)/*7605i and 8603*/
		{
		  if(product->board_id == slot )/*local board, different from 86: bakup master go to delete.*/
		  {
		  	zlog_info("%s : line %d , Myself local board(slot %d) interface %s, so local or (bakup master) not delete .\n",__func__,__LINE__,slot,ifp->name);
			return 0 ;
		  }
		  else/* not local board*/
		  	{
			  zlog_info("%s : line %d , Not my local board(slot %d) interface %s, so delete .\n",__func__,__LINE__,slot,ifp->name);
			
			 /*在这不去通知其他动态路由进程，和释放ifp，利用监听到的netlink消息是否可以，若不可以，就在此处做删除处理对动态路由和ifp*/
			 if(ifp->if_types == RPA_INTERFACE)
			 {
			 	ret = delete_rpa_interface(ifp);
			 	if(ret < 0)
			 	{
			 	  zlog_debug("%s : line %d, delete rpa(%s) interface  failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
				  return -1;
				 }
			 	else
			 	{
			 	  zlog_debug("%s : line %d, delete rpa(%s) interface  sucess...\n",__func__,__LINE__,ifp->name);
			  	  return 0;
			 	 }
			 	}
			 else
			 	{
		  			zlog_debug("go to delete viral interface (%s) \n",ifp->name);
	   				if_delete(ifp);
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
#endif
  /*RPA interface*/
  if(ifp->if_types == RPA_INTERFACE)
  {
  	if(tipc_server_debug)
		zlog_debug("go to delete RPA interface\n");
/*	if(strncmp(ifp->name, "eth",3) != 0)not eth*/
	{
		ret = delete_rpa_interface(ifp);
	 	if(ret < 0)
	 	  zlog_debug("%s : line %d, delete rpa(%s) interface  failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
	 	else
	 	  zlog_debug("%s : line %d, delete rpa(%s) interface  sucess...\n",__func__,__LINE__,ifp->name);
		}
	goto skip;
  	
  } 
  if(ifp->if_types == REAL_INTERFACE)
  	{
  		if(tipc_server_debug)
			zlog_debug("unregister PRA interface table\n");
		if((strncmp(ifp->name, "ve",2) != 0 && strncmp(ifp->name, "obc",3)!= 0))
		{
			ret = unregister_rpa_interface_table(ifp);
		 	if(ret < 0)/*这两个log 没明白*/
		 	  zlog_debug("%s : line %d, unregister rpa(%s) table interface  failed (%s)!\n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
		 	else
		 	  zlog_debug("%s : line %d, unregister rpa(%s) table interface  sucess...\n",__func__,__LINE__,ifp->name);
			}
		goto skip;
  	}
  
  if(ifp->if_types == VIRTUAL_INTERFACE)
   {
	   if(tipc_server_debug)
		  zlog_debug("delete viral interface (%s) \n",ifp->name);
	   if_delete(ifp);
	   return 0;
  	}
     
  /*delete virtual interface*/ 
skip:
  if(ifp->if_types != VIRTUAL_INTERFACE)
  vice_redistribute_interface_delete(ifp);
  if_delete(ifp);
  return 0;
}


/*************************************************************************************/


static int
vice_board_flush_data(struct thread *thread)
{
  tipc_server *vice_board = THREAD_ARG(thread);

  vice_board->t_write = NULL;
  if (vice_board->t_suicide)
    {
      zlog_warn("vice board suicide !\n");
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      vice_board_close(vice_board);
      return -1;
    }
  switch (buffer_flush_available(vice_board->wb, vice_board->sock))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_flush_available failed on zserv client fd %d, "
      		"closing", __func__, vice_board->sock);
	  
	  zlog_warn("%s: There have mismatch packet!\n",__func__);
      vice_board_close(vice_board);
      break;
    case BUFFER_PENDING:
      vice_board->t_write = thread_add_write(zebrad.master, vice_board_flush_data,
      					 vice_board, vice_board->sock);
      break;
    case BUFFER_EMPTY:
      break;
    }
  return 0;
}


static int
vice_board_delayed_close(struct thread *thread)
{
  tipc_server *vice_board = THREAD_ARG(thread);

  vice_board->t_suicide = NULL;
  vice_board_close(vice_board);
  return 0;
}


int
vice_send_message_to_master(tipc_server *vice_board)
{

	if (vice_board->sock < 0)
	{
		zlog_info("%s : fd < 0 , cannot to send message .\n",__func__);
		return -2;
	}

	#ifdef ZSERVER_DEBUG
		 zlog_debug("enter func %s.......\n",__func__);
	#endif
  if (vice_board->t_suicide)
    return -1;
  switch (buffer_write(vice_board->wb, vice_board->sock, STREAM_DATA(vice_board->obuf),
		       stream_get_endp(vice_board->obuf)))
    {
    case BUFFER_ERROR:
      zlog_warn("%s: buffer_write failed to zserv client fd %d, closing",
      		 __func__, vice_board->sock);
      /* Schedule a delayed close since many of the functions that call this
         one do not check the return code.  They do not allow for the
	 possibility that an I/O error may have caused the vice_board to be
	 deleted. */
      vice_board->t_suicide = thread_add_event(zebrad.master, vice_board_delayed_close,
					   vice_board, 0);
      return -1;
    case BUFFER_EMPTY:
      THREAD_OFF(vice_board->t_write);
      break;
    case BUFFER_PENDING:
      THREAD_WRITE_ON(zebrad.master, vice_board->t_write,
		      vice_board_flush_data, vice_board, vice_board->sock);
      break;
    }
  
	#ifdef ZSERVER_DEBUG  
 	 zlog_debug("leave func %s ........\n",__func__);
	#endif
  return 0;
}



/* Send simple Zebra message. */
int
vice_send_interface_infomation_request_message_to_master (tipc_server *vice_board, int command)
{
  struct stream *s;
  int request = keep_kernel_mode;

  /* Get zclient output buffer. */
  s = vice_board->obuf;
  stream_reset (s);

  /* Send very simple command only Zebra message. */
  tipc_packet_create_header(s, command);
  stream_putl(s,request);
  
  stream_putw_at (s, 0, stream_get_endp (s));
  
  return vice_send_message_to_master(vice_board);
}


/* Send simple Zebra message. */
int
vice_send_request_message_to_master (tipc_server *vice_board, int command)
{
  struct stream *s;

  /* Get zclient output buffer. */
  s = vice_board->obuf;
  stream_reset (s);

  /* Send very simple command only Zebra message. */
  tipc_packet_create_header (s, (uint16_t)command);
  
  stream_putw_at (s, 0, stream_get_endp (s));
  
  return vice_send_message_to_master(vice_board);
}


int
vice_send_route_request_message_to_master (struct thread *thread)
{
  tipc_server *vice_board;
  int ret;

  if(tipc_server_debug)
	  	zlog_debug ("enter func %s ......\n", __func__);

  /* Get thread data.  Reset reading thread because I'm running. */
  vice_board = THREAD_ARG (thread);
  vice_board->t_timer = NULL;
  vice_send_request_message_to_master(vice_board,ZEBRA_ROUTE_INFOMATION_REQUEST);
  
  return 0;
  
  
}


int
vice_send_interface_request_message_to_master (struct thread *thread)
{
  tipc_server *vice_board;
  int ret;

  if(tipc_server_debug)
	  	zlog_debug ("enter func %s ......\n", __func__);

  /* Get thread data.  Reset reading thread because I'm running. */
  vice_board = THREAD_ARG (thread);
  vice_board->t_timer = NULL;
  ret = vice_send_request_message_to_master(vice_board,ZEBRA_INTERFACE_INFOMATION_REQUEST);
  if(ret < 0 )/*fd not connect*/
  {
  		zlog_info("%s: tipc fd not connect , to set timer(1s) to retry [interface request].\n",__func__);
		vice_board->t_timer = thread_add_timer(zebrad.master,vice_send_interface_request_message_to_master,vice_board,1);
		return -1;
  	}
  else/*fd is connected*/
  {
  		zlog_info("%s: tipc fd is connected , to set timer to send [route request].\n",__func__);
		vice_board->t_timer = thread_add_timer(zebrad.master,vice_send_route_request_message_to_master,vice_board,1);
  	}

  return 0;
  
  
}



/* Make new client. */
static void
vice_accept_master (int sock)
{
  tipc_server *current_vice_board;
  tipc_server *vice_board;
  struct listnode *node, *nnode;
  int current_accept_slot = 0;
  int ret = 0;
  if(tipc_server_debug)
	 zlog_debug ("%s: line %d, go to creat tipc_client ......\n", __func__,__LINE__);

  vice_board = XCALLOC (MTYPE_TIPC_SERVER, sizeof (struct tipc_server));

  /* Make client input/output buffer. */
  vice_board->sock = sock;
  vice_board->ibuf = stream_new (TIPC_MAX_PACKET_SIZ);
  vice_board->obuf = stream_new (TIPC_MAX_PACKET_SIZ);
  vice_board->wb = buffer_new(0);
  current_accept_slot = get_peer_slot_from_tipc(sock);
  if(current_accept_slot > 0)
  {
   if(zebrad.master_board_list)
   {
 	 for (ALL_LIST_ELEMENTS (zebrad.master_board_list, node, nnode, current_vice_board)) 
		{ 
			if(current_vice_board->accept_slot == current_accept_slot)
			{
				zlog_warn("To close Vice board (because the board reuse)!\n");
				vice_board_close(current_vice_board);
				}
	 	 }
   	}
   else
   {
	   zlog_warn(" zebrad.master_board_list err .\n");
	   vice_board_close(vice_board);/*add.*/
	   return ;

   }
  }
  else
  {
		zlog_warn(" get current_accept_slot(%d) from tipc socket err .\n",current_accept_slot);
		/*CID 13546 (#1 of 1): Resource leak (RESOURCE_LEAK)
		5. leaked_storage: Variable "vice_board" going out of scope leaks the storage it points to.
		*/
		vice_board_close(vice_board);/*add.*/
		return ;
	}
  vice_board->accept_slot = current_accept_slot;
  
  /*wangchao add*/
  vice_board->ifinfo = 1;

  /*CID 11032 (#1 of 1): Dereference after null check (FORWARD_NULL)
    6. var_deref_model: Passing null pointer "zebrad.master_board_list" to function "listnode_add(struct list *, void *)", which dereferences it.
    No problem.*/
  /* Add this client to linked list. */
  listnode_add (zebrad.master_board_list, vice_board);
  if(tipc_server_debug)
    zlog_debug(" master_board_list count = %d .\n",zebrad.master_board_list->count);
  
  /* Make new read thread. */
  vice_board_event(TIPC_READ, sock, vice_board);

  /**init hooks**/
//  	  vice_board->interface_add = tipc_interface_add;
//	  vice_board->interface_delete = tipc_interface_delete;
//	  vice_board->interface_address_add = tipc_interface_address_add;
//	  vice_board->interface_address_delete = tipc_interface_address_delete;
//	  vice_board->ipv4_route_add = tipc_zebra_read_ipv4;
//	  vice_board->ipv4_route_delete = tipc_zebra_read_ipv4;
//	  vice_board->interface_up = tipc_interface_up;
//	  vice_board->interface_down = tipc_interface_down;

//	  vice_board->redistribute_interface_all = tipc_interface_add;
//	  vice_board->redistribute_route_all = tipc_zebra_boot;

 /**gjd : vice board send all his interfaces info to master board**/
 // vice_send_message_to_master(ZEBRA_REDISTRIBUTE_ALL_INTERFACE,);

 /**gjd : vice board send all his interfaces info to master board**/
 // vice_send_message_to_master(ZEBRA_REDISTRIBUTE_ALL_INTERFACE,);


 /* vice_interface_send_to_master(vice_board);*/
 #if 0	
  	usleep(fetch_rand_time_interval());
 	vice_send_request_message_to_master(vice_board,ZEBRA_INTERFACE_INFOMATION_REQUEST);
	sleep(1);
	vice_send_request_message_to_master(vice_board,ZEBRA_ROUTE_INFOMATION_REQUEST);
#else
	/*usleep(fetch_rand_time_interval());*/
 	
	/*after 1 s , to send interface request .*/
	vice_board->t_timer = thread_add_timer(zebrad.master,vice_send_interface_request_message_to_master,vice_board,1);
	
#endif
	#if 0
 	if(keep_kernel_mode == 0)
 	{
		vice_interface_send_to_master(vice_board);
		return;
 	}
	#endif

/**we use master board initiative send all route to vice board, not use vice send command to master**/
// vice_send_request_message_to_master(vice_board,ZEBRA_REDISTRIBUTE_ALL_ROUTE);
 
  	
}



/* Close zebra client. */
int
vice_board_close (tipc_server *vice_board)
{
#if 1

  zlog_warn("%s: socket[%d] reading cased failed ,to close tipc server.\n",__func__,vice_board->sock);

  /* Close file descriptor. */
  if (vice_board->sock)
    {
      close (vice_board->sock);
      vice_board->sock = -1;
    }

  /* Free stream buffers. */
  if (vice_board->ibuf)
  { 
  	stream_free (vice_board->ibuf);
  	vice_board->ibuf = NULL;
  }
  if (vice_board->obuf)
  {
	stream_free (vice_board->obuf);
  	vice_board->obuf = NULL;
  }
  if (vice_board->wb)
  {
  	buffer_free(vice_board->wb);
  	vice_board->wb = NULL;
  }

  /* Release threads. */
  if (vice_board->t_read)
    thread_cancel (vice_board->t_read);
  if (vice_board->t_write)
    thread_cancel (vice_board->t_write);
  if (vice_board->t_suicide)
    thread_cancel (vice_board->t_suicide);
    
  if (vice_board->t_timer)
	  thread_cancel (vice_board->t_timer);

  /* Free client structure. */
  listnode_delete (zebrad.master_board_list, vice_board);
  XFREE (MTYPE_TIPC_SERVER, vice_board);
  #else
  zlog_warn("%s: socket[%d] reading cased failed ,to reset ibuf.\n",__func__,vice_board->sock);
  stream_reset (vice_board->ibuf);
  vice_board_event (TIPC_READ, vice_board->sock, vice_board);
  return -1;

  #endif
  /*CID 11367 (#1 of 1): Missing return statement (MISSING_RETURN)
	8. missing_return: Arriving at the end of a function without returning a value.
	Add return value.*/
	return 0;
  
}


#if 0
/**add by gjd : tipc server read stream buf which comes from tipc client,and parse the buf**/
static int
vice_board_read (struct thread *thread)
{
  int sock;
  tipc_server *vice_board;
  size_t already;
  uint16_t length, command;
  uint8_t marker, version;
  int ret;

  if(tipc_server_debug)
	  	zlog_debug ("enter func %s ......\n", __func__);

  /* Get thread data.  Reset reading thread because I'm running. */
  sock = THREAD_FD (thread);
  vice_board = THREAD_ARG (thread);
  vice_board->t_read = NULL;

  if (vice_board->t_suicide)
    {
	  zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
      vice_board_close(vice_board);
      return -1;
    }

  ssize_t nbyte = 0;
  if (((nbyte = stream_read_try(vice_board->ibuf, sock,4096)) == 0) || (nbyte == -1))/**use size = 4096**/
	 {
	 /*  if (tipc_server_debug)*/
	   zlog_warn ("server connection closed socket [%d].\n", sock);
	   zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
	   vice_board_close(vice_board);
	   return -1;
	 }

  /* Fetch header values */
  length = stream_getw (vice_board->ibuf);
  marker = stream_getc (vice_board->ibuf);
  version = stream_getc (vice_board->ibuf);
  command = stream_getw (vice_board->ibuf);

  /*deal with some simple errors*/
  if (marker != ZEBRA_HEADER_MARKER || version != ZSERV_VERSION)
    {
      zlog_err("%s: socket %d version mismatch, marker %d, version %d",
               __func__, sock, marker, version);
	  
	  zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
      vice_board_close (vice_board);
      return -1;
    }
  if (length < ZEBRA_HEADER_SIZE) 
    {
      zlog_warn("%s: socket %d message length %u is less than header size %d",
	        __func__, sock, length, ZEBRA_HEADER_SIZE);
	  
	  zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
      vice_board_close (vice_board);
      return -1;
    }
  if (length > STREAM_SIZE((vice_board->ibuf)))
    {
      zlog_warn("%s: socket %d message length %u exceeds buffer size %lu",
	        __func__, sock, length, (u_long)STREAM_SIZE((vice_board->ibuf)));
	  
	  zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
      vice_board_close (vice_board);
      return -1;
    }

  /* Debug packet information. */
  if (tipc_server_debug)
    zlog_debug ("%s: line %d, rtm message comes from socket [%d]\n",__func__,__LINE__, sock);

  if (tipc_server_debug)
    zlog_debug ("%s: line %d, rtm message received [%s] %d\n", 
	       __func__,__LINE__,zserv_command_string (command), length);

  switch (command) 
  {
	  case ZEBRA_ROUTER_ID_UPDATE:
		if (vice_board->router_id_update)
	  ret = (*vice_board->router_id_update) (command, vice_board, length);
		break;
	  case ZEBRA_INTERFACE_ADD:
//		if (vice_board->interface_add)
// 	    ret = (*vice_board->interface_add) (command, vice_board, length);
	tipc_vice_interface_add (ZEBRA_INTERFACE_ADD, vice_board, length);

		break;
	  case ZEBRA_INTERFACE_DELETE:
		//if (vice_board->interface_delete)
	  //ret = (*vice_board->interface_delete) (command, vice_board, length);
		tipc_vice_interface_delete (ZEBRA_INTERFACE_DELETE, vice_board, length);
		 
		break;
	  case ZEBRA_INTERFACE_ADDRESS_ADD:
//		if (vice_board->interface_address_add)
//	  ret = (*vice_board->interface_address_add) (command, vice_board, length);
 	   tipc_vice_interface_address_add (ZEBRA_INTERFACE_ADDRESS_ADD ,vice_board, length);

		break;
	  case ZEBRA_INTERFACE_ADDRESS_DELETE:
//		if (vice_board->interface_address_delete)
//	  ret = (*vice_board->interface_address_delete) (command, vice_board, length);
		tipc_vice_interface_address_delete (ZEBRA_INTERFACE_ADDRESS_DELETE ,vice_board,length);

		break;
	  case ZEBRA_INTERFACE_UP:
//		if (vice_board->interface_up)
//	  ret = (*vice_board->interface_up) (command, vice_board, length);
		tipc_vice_interface_up (ZEBRA_INTERFACE_UP, vice_board, length);

		break;
	  case ZEBRA_INTERFACE_DOWN:
//		if (vice_board->interface_down)
//	  ret = (*vice_board->interface_down) (command, vice_board, length);
		tipc_vice_interface_down (ZEBRA_INTERFACE_DOWN, vice_board, length);

		break;
	  case ZEBRA_IPV4_ROUTE_ADD:
//		if (vice_board->ipv4_route_add)
//	  ret = (*vice_board->ipv4_route_add) (command, vice_board, length);
//		tipc_zebra_boot(command,vice_board,length);
		tipc_zebra_read_route(command,vice_board,length);

		break;
	  case ZEBRA_IPV4_ROUTE_DELETE:
//		if (vice_board->ipv4_route_delete)
//	  ret = (*vice_board->ipv4_route_delete) (command, vice_board, length);		
		tipc_zebra_read_route(command,vice_board,length);

		break;
	  case ZEBRA_IPV6_ROUTE_ADD:
//		if (vice_board->ipv6_route_add)
//	  ret = (*vice_board->ipv6_route_add) (command, vice_board, length);
		tipc_zebra_read_route(command,vice_board,length);		
//		tipc_zebra_read_ipv4(command,vice_board,length);

		break;
	  case ZEBRA_IPV6_ROUTE_DELETE:
//		if (vice_board->ipv6_route_delete)
//	  ret = (*vice_board->ipv6_route_delete) (command, vice_board, length);
		tipc_zebra_read_route(command,vice_board,length);

		break;
		
	  case ZEBRA_REDISTRIBUTE_ALL_INTERFACE:
	  	if(vice_board->redistribute_interface_all)
	  ret = (*vice_board->redistribute_interface_all)(command, vice_board, length);
		break;
	  case ZEBRA_REDISTRIBUTE_ALL_ROUTE:
//  	if(vice_board->redistribute_route_all)
//	  ret = (*vice_board->redistribute_route_all)(command, vice_board, length);		
		tipc_zebra_boot(command,vice_board,length);
		break;

	  case ZEBRA_INTERFACE_LINKDETECTION_ENABLE:
/*		if (vice_board->interface_up)*/
/*	  ret = (*vice_board->interface_up) (command, vice_board, length);*/
		tipc_vice_interface_linkdetection_enable(ZEBRA_INTERFACE_LINKDETECTION_ENABLE, vice_board, length);

		break;

	 case ZEBRA_INTERFACE_LINKDETECTION_DISABLE:
/*		if (vice_board->interface_up)*/
/*		ret = (*vice_board->interface_up) (command, vice_board, length);*/
	   tipc_vice_interface_linkdetection_disable(ZEBRA_INTERFACE_LINKDETECTION_DISABLE, vice_board, length);

		break;

	 case ZEBRA_INTERFACE_PACKETS_REQUEST:
	 	tipc_vice_interface_packets_statistics_request(ZEBRA_INTERFACE_PACKETS_REQUEST,vice_board,length);

		break;
		
	case ZEBRA_INTERFACE_PACKETS_REQUEST_ALL:
	 	tipc_vice_interface_packets_statistics_request_all(ZEBRA_INTERFACE_PACKETS_REQUEST_ALL,vice_board,length);

		break;

	case ZEBRA_INTERFACE_INFOMATION_REQUEST:
		vice_interface_infomation_request(ZEBRA_INTERFACE_INFOMATION_REQUEST,vice_board,length);

		break;
		

	  default:
		break;
	  }

  if (vice_board->t_suicide)
    {
      /* No need to wait for thread callback, just kill immediately. */
	  
	  zlog_warn("%s:line %d. There have mismatch packet!\n",__func__,__LINE__);
      vice_board_close(vice_board);
      return -1;
    }

  stream_reset (vice_board->ibuf);
  vice_board_event (TIPC_READ, sock, vice_board);
  return 0;
}
#else
int
vice_board_read (struct thread *thread)
{
  int sock;
  tipc_server *vice_board;
  size_t already = 0;
  uint16_t length , command;
  uint8_t marker, version;
  int ret;

  if(tipc_server_debug)
	  	zlog_debug ("enter func %s ......\n", __func__);
  /* Get socket to zebra. */
  vice_board = THREAD_ARG (thread);
  vice_board->t_read = NULL;
  
  ssize_t nbyte = 0;


  if ((already = stream_get_endp(vice_board->ibuf)) < TIPC_PACKET_HEADER_SIZE)
	  {
	  
		ssize_t nbyte;
		if (((nbyte = stream_read_try(vice_board->ibuf, vice_board->sock,
					   TIPC_PACKET_HEADER_SIZE-already)) == 0) ||
		(nbyte == -1))
	  {
	  	zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																	vice_board->sock,safe_strerror(errno));
		return vice_board_close(vice_board);
	  }
		if (nbyte != (ssize_t)(TIPC_PACKET_HEADER_SIZE-already))
	  {
		/* Try again later. */
		  vice_board_event (TIPC_READ, vice_board->sock, vice_board);
		return 0;
	  }
		already = TIPC_PACKET_HEADER_SIZE;
	  }
  
	/* Reset to read from the beginning of the incoming packet. */
	stream_set_getp(vice_board->ibuf, 0);

  /* Fetch header values. 6 byte*/
  length = stream_getw (vice_board->ibuf);
  marker = stream_getc (vice_board->ibuf);
  version = stream_getc (vice_board->ibuf);
  command = stream_getw (vice_board->ibuf);
  
  if (tipc_server_debug)
	zlog_debug ("%s: line %d, ok fetch header and packet length = %d, marker = %u, version = %u, command [%s]..\n",
				__func__,__LINE__,length,marker,version,zserv_command_string(command));
  
  if (marker != TIPC_PACKET_HEADER_MARKER || version != TIPC_PACKET_VERSION)
    {
      zlog_err("%s: socket %d version mismatch, marker %d, version %d",
               __func__, vice_board->sock, marker, version);
	  
	  zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																  vice_board->sock,safe_strerror(errno));
      return vice_board_close (vice_board);
 	
    }
  
  if (length < TIPC_PACKET_HEADER_SIZE) 
    {
      zlog_err("%s: socket %d message length %u is less than %d ",
	       __func__, vice_board->sock, length, TIPC_PACKET_HEADER_SIZE);
	  
	  zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																  vice_board->sock,safe_strerror(errno));
      return vice_board_close (vice_board);
    }

  /* Length check. */
  if (length > STREAM_SIZE(vice_board->ibuf))
    {
      struct stream *ns;
      zlog_warn("%s: message size %u exceeds buffer size %lu, expanding...",
	        __func__, length, (u_long)STREAM_SIZE(vice_board->ibuf));
      ns = stream_new(length);
      stream_copy(ns, vice_board->ibuf);
      stream_free (vice_board->ibuf);
      vice_board->ibuf = ns;
    }
  /* Read rest of zebra packet. */
  if (already < length)
  {
      ssize_t nbyte;
      if (((nbyte = stream_read_try(vice_board->ibuf, vice_board->sock,
				     length-already)) == 0) || (nbyte == -1))
	 {
	  zlog_err("%s: line %d,socket[%d] cased mismatch packet, to reset ibuf [%s].\n",__func__,__LINE__,
																	  vice_board->sock,safe_strerror(errno));
	  return vice_board_close (vice_board);
	}
	  
      if (nbyte != (ssize_t)(length-already))
	 {
	  /* Try again later. */
	  vice_board_event (TIPC_READ, vice_board->sock, vice_board);
	  return 0;
	 }
  }

  length -= TIPC_PACKET_HEADER_SIZE;

  if (tipc_server_debug)
   zlog_debug ("%s: line %d, rtm message received [%s] %d\n", __func__,__LINE__,zserv_command_string (command), length);

  switch (command) 
  {
	  case ZEBRA_INTERFACE_PACKETS_REQUEST_ALL:
		  tipc_vice_interface_packets_statistics_request_all(ZEBRA_INTERFACE_PACKETS_REQUEST_ALL,vice_board,length);
		  break;

	  case ZEBRA_INTERFACE_UP:
	 /* if (vice_board->interface_up)*/
	  /*ret = (*vice_board->interface_up) (command, vice_board, length);*/
		 tipc_vice_interface_up (ZEBRA_INTERFACE_UP, vice_board, length);
		 break;
		 
	  case ZEBRA_INTERFACE_DOWN:
	  /*if (vice_board->interface_down)*/
	  /*ret = (*vice_board->interface_down) (command, vice_board, length);*/
		tipc_vice_interface_down (ZEBRA_INTERFACE_DOWN, vice_board, length);
		break;
		  
	  case ZEBRA_INTERFACE_ADD:
/*		if (vice_board->interface_add)*/
/* 	    ret = (*vice_board->interface_add) (command, vice_board, length);*/
	tipc_vice_interface_add (ZEBRA_INTERFACE_ADD, vice_board, length);

		break;
	  case ZEBRA_INTERFACE_DELETE:
/*		if (vice_board->interface_delete)*/
/*	  ret = (*vice_board->interface_delete) (command, vice_board, length);*/
		tipc_vice_interface_delete (ZEBRA_INTERFACE_DELETE, vice_board, length);
		 
		break;
	  case ZEBRA_INTERFACE_ADDRESS_ADD:
/*		if (vice_board->interface_address_add)*/
/*	  ret = (*vice_board->interface_address_add) (command, vice_board, length);*/
 	   tipc_vice_interface_address_add (ZEBRA_INTERFACE_ADDRESS_ADD ,vice_board, length);

		break;
	  case ZEBRA_INTERFACE_ADDRESS_DELETE:
/*		if (vice_board->interface_address_delete)*/
/*	  ret = (*vice_board->interface_address_delete) (command, vice_board, length);*/
		tipc_vice_interface_address_delete (ZEBRA_INTERFACE_ADDRESS_DELETE ,vice_board,length);

		break;

	  case ZEBRA_IPV4_ROUTE_ADD:
/*		if (vice_board->ipv4_route_add)*/
/*	  ret = (*vice_board->ipv4_route_add) (command, vice_board, length);*/
/*		tipc_zebra_boot(command,vice_board,length);*/
		tipc_zebra_read_route(command,vice_board,length);

		break;
	  case ZEBRA_IPV4_ROUTE_DELETE:
/*		if (vice_board->ipv4_route_delete)*/
/*	  ret = (*vice_board->ipv4_route_delete) (command, vice_board, length);*/		
		tipc_zebra_read_route(command,vice_board,length);

		break;
	  case ZEBRA_IPV6_ROUTE_ADD:
/*		if (vice_board->ipv6_route_add)*/
/*	  ret = (*vice_board->ipv6_route_add) (command, vice_board, length);*/
		tipc_zebra_read_route(command,vice_board,length);		
/*		tipc_zebra_read_ipv4(command,vice_board,length);*/

		break;
	  case ZEBRA_IPV6_ROUTE_DELETE:
/*		if (vice_board->ipv6_route_delete)*/
/*	  ret = (*vice_board->ipv6_route_delete) (command, vice_board, length);*/
		tipc_zebra_read_route(command,vice_board,length);

		break;
		
	  case ZEBRA_INTERFACE_DESCRIPTION_SET:
		tipc_vice_interface_description_set(command,vice_board,length);
		
		break;
		
	  case ZEBRA_INTERFACE_DESCRIPTION_UNSET:
		  tipc_vice_interface_description_unset(command,vice_board,length);
		  
		  break;
		  
	  /*gujd: 2013-05-14, am 4:11 . Add code for ipv6 nd suppress ra deal for Distribute System.*/
	  case ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE:
		  tipc_vice_interface_nd_suppress_ra_deal(ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE, vice_board, length);
	  
		  break;

	  
	  case ZEBRA_INTERFACE_ND_SUPPRESS_RA_DISABLE:
		  tipc_vice_interface_nd_suppress_ra_deal(ZEBRA_INTERFACE_ND_SUPPRESS_RA_DISABLE, vice_board, length);
	  
		  break;
	  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
	  case ZEBRA_INTERFACE_RTADV_ND_INFO_UPDATE:
		  tipc_vice_rtadv_nd_info_update(ZEBRA_INTERFACE_RTADV_ND_INFO_UPDATE, vice_board, length);
	  
		  break;
		  
	  case ZEBRA_INTERFACE_ND_PREFIX_ADD:
		  tipc_vice_interface_nd_prefix_update(ZEBRA_INTERFACE_ND_PREFIX_ADD, vice_board, length);
	  
		  break;
	   case ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD:
	   	
		  tipc_vice_interface_nd_prefix_pool_update(ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD, vice_board, length);
	  
		  break;
		  	
	 case ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE:
		 tipc_vice_interface_nd_prefix_pool_update(ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE, vice_board, length);
		 break;

	   case ZEBRA_INTERFACE_LOCAL_LINK_ADD:
	   	
		  tipc_vice_interface_local_link_update(ZEBRA_INTERFACE_LOCAL_LINK_ADD, vice_board, length);
	  
		  break;

	  case ZEBRA_INTERFACE_ND_PREFIX_DELETE:
		  tipc_vice_interface_nd_prefix_update(ZEBRA_INTERFACE_ND_PREFIX_DELETE, vice_board, length);
	  
		  break;
		  
	 case ZEBRA_ROUTER_ID_UPDATE:
/*		  if (vice_board->router_id_update)*/
/*		ret = (*vice_board->router_id_update) (command, vice_board, length);*/
		  break;

	  case ZEBRA_REDISTRIBUTE_ALL_INTERFACE:
/*	  	if(vice_board->redistribute_interface_all)*/
/*	  ret = (*vice_board->redistribute_interface_all)(command, vice_board, length);*/
		break;
	  case ZEBRA_REDISTRIBUTE_ALL_ROUTE:
/*  	if(vice_board->redistribute_route_all)*/
/*	  ret = (*vice_board->redistribute_route_all)(command, vice_board, length);	*/	
		tipc_zebra_boot(command,vice_board,length);
		break;

	  case ZEBRA_INTERFACE_LINKDETECTION_ENABLE:
/*		if (vice_board->interface_up)*/
/*	  ret = (*vice_board->interface_up) (command, vice_board, length);*/
		tipc_vice_interface_linkdetection_enable(ZEBRA_INTERFACE_LINKDETECTION_ENABLE, vice_board, length);

		break;

	 case ZEBRA_INTERFACE_LINKDETECTION_DISABLE:
/*		if (vice_board->interface_up)*/
/*		ret = (*vice_board->interface_up) (command, vice_board, length);*/
	   tipc_vice_interface_linkdetection_disable(ZEBRA_INTERFACE_LINKDETECTION_DISABLE, vice_board, length);

		break;
	  case ZEBRA_INTERFACE_UPLINK_FLAG_SET:
/*		if (vice_board->interface_up)*/
/*	  ret = (*vice_board->interface_up) (command, vice_board, length);*/
		tipc_vice_interface_uplink_flag_update(ZEBRA_INTERFACE_UPLINK_FLAG_SET, vice_board, length);

		break;
		
	  case ZEBRA_INTERFACE_UPLINK_FLAG_UNSET:
/*		if (vice_board->interface_up)*/
/*	  ret = (*vice_board->interface_up) (command, vice_board, length);*/
		tipc_vice_interface_uplink_flag_update(ZEBRA_INTERFACE_UPLINK_FLAG_UNSET, vice_board, length);

		break;
/*
	 case ZEBRA_INTERFACE_PACKETS_REQUEST:
	 	tipc_vice_interface_packets_statistics_request(ZEBRA_INTERFACE_PACKETS_REQUEST,vice_board,length);

		break;
*/		

	case ZEBRA_INTERFACE_INFOMATION_REQUEST:
		vice_interface_infomation_request(ZEBRA_INTERFACE_INFOMATION_REQUEST,vice_board,length);

		break;
		
	  
    default:
      break;
    }
  
  if (vice_board->sock < 0)
    /* Connection was closed during packet processing. */
    return -1;

  /* Register read thread. */
  
  stream_reset (vice_board->ibuf);
  vice_board_event (TIPC_READ, vice_board->sock, vice_board);
  

  if(tipc_server_debug)
  	zlog_debug("leave func %s...\n",__func__);

  return 0;
}
#endif

/* Accept code of zebra server socket. */
static int
vice_accept_master_start (struct thread *thread)
{
  int accept_sock;
  int client_sock;
  struct sockaddr_tipc client;
  socklen_t len;

  if(tipc_server_debug)
  	zlog_debug("enter func %s ....\n",__func__);

  accept_sock = THREAD_FD (thread);

  /* Reregister myself. */
  vice_board_event (TIPC_SERV, accept_sock, NULL);

  len = sizeof (struct sockaddr_tipc);
  client_sock = accept (accept_sock, (struct sockaddr *) &client, &len);

  if (client_sock < 0)
    {
      zlog_warn ("Can't accept tipc socket: %s", safe_strerror (errno));
      return -1;
    }
  
  /*gujd : 2012-10-17 . Add set tipc socket nonblock.*/	
#if 1
  if (set_nonblocking(client_sock) < 0)
    zlog_warn("%s: set_nonblocking(%d) failed[%s]", 
    	__func__, client_sock,safe_strerror(errno));
#endif

  /* Create  client : master board*/
  vice_accept_master(client_sock);
  

  return 0;
}


static void
vice_board_event(enum tipc_serv_events event, int sock, tipc_server *vice_board)/**master_board**/
{
	if(tipc_server_debug)
		zlog_debug("enter func %s ...\n",__func__);
  switch (event)
    {
    case TIPC_SERV:
	  if(tipc_server_debug)
	  	zlog_debug ("%s: line %d, go to vice_accept_master_start ......\n", __func__,__LINE__);
      thread_add_read (zebrad.master, vice_accept_master_start, vice_board, sock);/*accept client connect(master board)**/
      break;
    case TIPC_READ:
	  if(tipc_server_debug)
	  	zlog_debug ("%s: line %d, go to vice_board_read ......\n", __func__,__LINE__);
      vice_board->t_read = 
	thread_add_read (zebrad.master, vice_board_read, vice_board, sock);/*read client message, in fact is request(when system start)*/
      break;												/*read message from client (master board),the message include:system start and running*/
    case TIPC_WRITE:
      /**/
      break;
    }
  if(tipc_server_debug)
  	zlog_debug("leave func %s ..\n",__func__);
}



void vice_board_init(product_inf *product)
{
  int ret;
  int sock, len;
  struct sockaddr_tipc server_addr;

  if(tipc_server_debug)
  	zlog_debug("enter func %s....\n",__func__);
  
  zebrad.master_board_list = list_new();
  
  zlog_debug("This board id(slot) is %d, create socket inst is %d . \n",product->board_id,SERVER_INST);

  server_addr.family = AF_TIPC;
  server_addr.addrtype = TIPC_ADDR_NAMESEQ;
  server_addr.addr.nameseq.type = SERVER_TYPE;
  server_addr.addr.nameseq.lower = SERVER_INST;
  server_addr.addr.nameseq.upper = SERVER_INST;
  server_addr.scope = TIPC_CLUSTER_SCOPE;

  
  /* Make tipc socket. (vice board : tipc server)*/
  sock = socket (AF_TIPC, SOCK_STREAM,0);
 /* sock = socket (AF_TIPC, SOCK_SEQPACKET,0);*/

  if (sock < 0)
	{
	  zlog_warn ("Can't create tipc socket: %s", 
				 safe_strerror (errno));
	  return;
	}


  ret = bind (sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (ret < 0)
	{
	  zlog_warn ("Can't bind to tipc socket  %s", 
				  safe_strerror (errno));
	  close (sock);
	  return;
	}

  ret = listen (sock, MAX_CONNECT);
  if (ret < 0)
	{
	  zlog_warn ("Can't listen to unix socket: %s", 
				  safe_strerror (errno));
	  close (sock);
	  return;
	}


  vice_board_event (TIPC_SERV, sock, NULL);
}


