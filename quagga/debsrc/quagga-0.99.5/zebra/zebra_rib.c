/* Routing Information Base.
 * Copyright (C) 1997, 98, 99, 2001 Kunihiro Ishiguro
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

#include "prefix.h"
#include "table.h"
#include "memory.h"
#include "str.h"
#include "command.h"
#include "if.h"
#include "log.h"
#include "sockunion.h"
#include "linklist.h"
#include "thread.h"
#include "workqueue.h"
#include "tipc_zclient_lib.h"

#include "zebra/rib.h"
#include "zebra/rt.h"
#include "zebra/zserv.h"
#include "zebra/redistribute.h"
#include "zebra/debug.h"
#include "zebra/tipc_server.h"
#include "zebra/tipc_client.h"

/*******************added by gxd******************/
static unsigned int zebra_static_ipv4_count;
unsigned int get_zebra_static_ipv4_count(void);
int add_zebra_static_ipv4_count(void);
int sub_zebra_static_ipv4_count(void);
/*******************2008-9-17 16:09:30*****************************/
void static_install_ipv4 (struct prefix *p, struct static_ipv4 *si);



/* Default rtm_table for all clients */
extern struct zebra_t zebrad;
extern product_inf *product;
extern int board_type;

extern int route_boot_errno;


/* Hold time for RIB process, should be very minimal.
 * it is useful to able to set it otherwise for testing, hence exported
 * as global here for test-rig code.
 */
int rib_process_hold_time = 10;

/* Each route type's string and default distance value. */
struct
{  
  int key;
  int distance;
} route_info[] =
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

/* Vector for routing table.  */
vector vrf_vector;

/* Allocate new VRF.  */
static struct vrf *
vrf_alloc (const char *name)
{
  struct vrf *vrf;

  vrf = XCALLOC (MTYPE_VRF, sizeof (struct vrf));

  /* Put name.  */
  if (name)
    vrf->name = XSTRDUP (MTYPE_VRF_NAME, name);

  /* Allocate routing table and static table.  */
  vrf->table[AFI_IP][SAFI_UNICAST] = route_table_init ();
  vrf->table[AFI_IP6][SAFI_UNICAST] = route_table_init ();
  vrf->stable[AFI_IP][SAFI_UNICAST] = route_table_init ();
  vrf->stable[AFI_IP6][SAFI_UNICAST] = route_table_init ();

  return vrf;
}

/* Free VRF.  */
static void
vrf_free (struct vrf *vrf)
{
  if (vrf->name)
    XFREE (MTYPE_VRF_NAME, vrf->name);
  XFREE (MTYPE_VRF, vrf);
}

/* Lookup VRF by identifier.  */
struct vrf *
vrf_lookup (u_int32_t id)
{
  return vector_lookup (vrf_vector, id);
}

/* Lookup VRF by name.  */
static struct vrf *
vrf_lookup_by_name (char *name)
{
  unsigned int i;
  struct vrf *vrf;

  for (i = 0; i < vector_active (vrf_vector); i++)
    if ((vrf = vector_slot (vrf_vector, i)) != NULL)
      if (vrf->name && name && strcmp (vrf->name, name) == 0)
	return vrf;
  return NULL;
}

/* Initialize VRF.  */
static void
vrf_init (void)
{
  struct vrf *default_table;

  /* Allocate VRF vector.  */
  vrf_vector = vector_init (1);

  /* Allocate default main table.  */
  default_table = vrf_alloc ("Default-IP-Routing-Table");

  /* Default table index must be 0.  */
  vector_set_index (vrf_vector, 0, default_table);
}

/* Lookup route table.  */
struct route_table *
vrf_table (afi_t afi, safi_t safi, u_int32_t id)
{
  struct vrf *vrf;

  vrf = vrf_lookup (id);
  if (! vrf)
    return NULL;

  return vrf->table[afi][safi];
}

/* Lookup static route table.  */
struct route_table *
vrf_static_table (afi_t afi, safi_t safi, u_int32_t id)
{
  struct vrf *vrf;

  vrf = vrf_lookup (id);
  if (! vrf)
    return NULL;

  return vrf->stable[afi][safi];
}

/* Add nexthop to the end of the list.  */
static void
nexthop_add (struct rib *rib, struct nexthop *nexthop)
{
  struct nexthop *last;

  for (last = rib->nexthop; last && last->next; last = last->next)
    ;
  if (last)
    last->next = nexthop;
  else
    rib->nexthop = nexthop;
  nexthop->prev = last;

  rib->nexthop_num++;
}

/* Delete specified nexthop from the list. */
static void
nexthop_delete (struct rib *rib, struct nexthop *nexthop)
{
  if (nexthop->next)
    nexthop->next->prev = nexthop->prev;
  if (nexthop->prev)
    nexthop->prev->next = nexthop->next;
  else
    rib->nexthop = nexthop->next;
  rib->nexthop_num--;
}

/* Free nexthop. */
void
nexthop_free (struct nexthop *nexthop)
{
  if (nexthop->ifname)
    XFREE (0, nexthop->ifname);
  XFREE (MTYPE_NEXTHOP, nexthop);
}

struct nexthop *
nexthop_ifindex_add (struct rib *rib, unsigned int ifindex)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IFINDEX;
  nexthop->ifindex = ifindex;

  nexthop_add (rib, nexthop);

  return nexthop;
}

struct nexthop *
nexthop_ifname_add (struct rib *rib, char *ifname)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IFNAME;
  nexthop->ifname = XSTRDUP (0, ifname);

  nexthop_add (rib, nexthop);

  return nexthop;
}

struct nexthop *
nexthop_ipv4_add (struct rib *rib, struct in_addr *ipv4)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV4;
  nexthop->gate.ipv4 = *ipv4;

  nexthop_add (rib, nexthop);

  return nexthop;
}

struct nexthop *
nexthop_ipv4_ifindex_add (struct rib *rib, struct in_addr *ipv4, 
			  unsigned int ifindex)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV4_IFINDEX;
  nexthop->gate.ipv4 = *ipv4;
  nexthop->ifindex = ifindex;

  nexthop_add (rib, nexthop);

  return nexthop;
}


/**add by gjd **/

struct nexthop *
nexthop_ifindex_flags_add (struct rib *rib, unsigned int ifindex,u_char flag)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IFINDEX;
  nexthop->ifindex = ifindex;
  nexthop->flags = flag;

  nexthop_add (rib, nexthop);

  return nexthop;
}
/** 2011-2-19: pf 6:30**/


/**add by gjd: add flags**/
struct nexthop *
nexthop_ipv4_ifindex_flags_add (struct rib *rib, struct in_addr *ipv4, 
			  unsigned int ifindex, u_char flag)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV4_IFINDEX;
  nexthop->gate.ipv4 = *ipv4;
  nexthop->ifindex = ifindex;
  nexthop->flags = flag;/**add**/

  nexthop_add (rib, nexthop);

  
  return nexthop;
}
/** 2011-2-19: pf 6:20**/

/**add by gjd: add flags(ipv6)**/
struct nexthop *
nexthop_ipv6_ifindex_flags_add (struct rib *rib, struct in6_addr *ipv6, 
			  unsigned int ifindex, u_char flag)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
  nexthop->gate.ipv6 = *ipv6;
  nexthop->ifindex = ifindex;
  nexthop->flags = flag;/**add**/

  nexthop_add (rib, nexthop);

  
  return nexthop;
}
/** 2011-9-12: pf 3:40**/


#ifdef HAVE_IPV6
struct nexthop *
nexthop_ipv6_add (struct rib *rib, struct in6_addr *ipv6)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV6;
  nexthop->gate.ipv6 = *ipv6;

  nexthop_add (rib, nexthop);

  return nexthop;
}

static struct nexthop *
nexthop_ipv6_ifname_add (struct rib *rib, struct in6_addr *ipv6,
			 char *ifname)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV6_IFNAME;
  nexthop->gate.ipv6 = *ipv6;
  nexthop->ifname = XSTRDUP (0, ifname);

  nexthop_add (rib, nexthop);

  return nexthop;
}

static struct nexthop *
nexthop_ipv6_ifindex_add (struct rib *rib, struct in6_addr *ipv6,
			  unsigned int ifindex)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
  nexthop->gate.ipv6 = *ipv6;
  nexthop->ifindex = ifindex;

  nexthop_add (rib, nexthop);

  return nexthop;
}
#endif /* HAVE_IPV6 */

struct nexthop *
nexthop_blackhole_add (struct rib *rib)
{
  struct nexthop *nexthop;

  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
  memset (nexthop, 0, sizeof (struct nexthop));
  nexthop->type = NEXTHOP_TYPE_BLACKHOLE;
  SET_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE);

  nexthop_add (rib, nexthop);

  return nexthop;
}

/*****************added by gxd*******************/
/***********wrap function of nexthop_blackhole_add*********************/
struct nexthop *
nexthop_blackhole_reject_add(struct rib*rib,const u_char flag)
{
	struct nexthop *nexthop;
	if(NULL!=(nexthop=nexthop_blackhole_add(rib))){
		if(ZEBRA_FLAG_REJECT==flag){
			  UNSET_FLAG (rib->flags,ZEBRA_FLAG_BLACKHOLE);
			  SET_FLAG (rib->flags, ZEBRA_FLAG_REJECT);
	       }
	}
	return nexthop;
}
/********************2008-10-21 14:58:31**************************/
/* If force flag is not set, do not modify falgs at all for uninstall
   the route from FIB. */
static int
nexthop_active_ipv4_normal_check (struct rib *rib, struct nexthop *nexthop, int set,
		     struct route_node *top)
{
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *match;
  struct nexthop *newhop;

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start with rib %p nexthop %p rn top %p", __func__, rib, nexthop, top);
/*
  if (nexthop->type == NEXTHOP_TYPE_IPV4)
    nexthop->ifindex = 0;
*/
  if (set)
    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);

  /* Make lookup prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = nexthop->gate.ipv4;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (IS_ZEBRA_DEBUG_RIB)
      zlog_debug ("%s: found vrf_table %p", __func__, table);
  if (! table) 
    return 0;
  	 
  
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: lookup prefix %s/%d", __func__, inet_ntoa (p.prefix),p.prefixlen);

  rn = route_node_match (table, (struct prefix *) &p);
  while (rn)
    {
	  if (IS_ZEBRA_DEBUG_RIB)
	      zlog_debug ("%s: found rn %p", __func__, rn);
      route_unlock_node (rn);
      
      /* If lookup self prefix return immidiately. */
      if (rn == top) {
	      if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug ("%s: rn==top %p, return 0", __func__, rn);

	return 0;
      	}
      /* Pick up selected route. */
      for (match = rn->info; match; match = match->next) {
	if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug ("%s: try match %p with flags 0x%x and type 0x%x", __func__, match,match->flags,match->type);
	if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	  break;
      	}

      if (IS_ZEBRA_DEBUG_RIB)
	zlog_debug ("%s: continue with match %p", __func__, match);
	
      /* If there is no selected route or matched route is EGP, go up
         tree. */
      if (! match 
	  || match->type == ZEBRA_ROUTE_BGP)
	{
	  do {
	    rn = rn->parent;
	  } while (rn && rn->info == NULL);
	  if (rn)
	    route_lock_node (rn);
	}
      else
	{
	  if (match->type == ZEBRA_ROUTE_CONNECT)
	    {
	      /* Directly point connected route. */
	      newhop = match->nexthop;
	      if (newhop && nexthop->type == NEXTHOP_TYPE_IPV4)
				nexthop->ifindex = newhop->ifindex;
		  /*CID 11033 (#1 of 2): Dereference after null check (FORWARD_NULL)
			20. var_deref_op: Dereferencing null pointer "newhop".
			So add if(newhop)*/
		  if(newhop)
	       {
	       	  if (IS_ZEBRA_DEBUG_RIB)
				zlog_debug ("%s: found ZEBRA_ROUTE_CONNECT nexthop %p with ifindex 0x%x, return 1", __func__, newhop,newhop->ifindex);
		   }
	      return 1;
	    }
	  else if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_INTERNAL))
	    {
		  if (IS_ZEBRA_DEBUG_RIB)
		    zlog_debug ("%s: process ZEBRA_FLAG_INTERNAL rib %p with flag 0x%x", __func__, rib,rib->flags);
	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
		    && ! CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_RECURSIVE))
		  {
		    if (set)
		      {
			SET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
			nexthop->rtype = newhop->type;
			if (newhop->type == NEXTHOP_TYPE_IPV4 ||
			    newhop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
			  nexthop->rgate.ipv4 = newhop->gate.ipv4;
			if (newhop->type == NEXTHOP_TYPE_IFINDEX
			    || newhop->type == NEXTHOP_TYPE_IFNAME
			    || newhop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
			  nexthop->rifindex = newhop->ifindex;
		      }
		    if (IS_ZEBRA_DEBUG_RIB)
		      zlog_debug ("%s: newhop %p NEXTHOP_FLAG_FIB and ! NEXTHOP_FLAG_RECURSIVE return 1", __func__, newhop);
		    
		    return 1;
		  }
		if (IS_ZEBRA_DEBUG_RIB)
		  zlog_debug ("%s: end ZEBRA_FLAG_INTERNAL rib %p with flag 0x%x return 0", __func__, rib,rib->flags);

		
	      return 0;
	    }
	  else
	    {
		  if (IS_ZEBRA_DEBUG_RIB)
		    zlog_debug ("%s: not ZEBRA_ROUTE_CONNECT not ZEBRA_FLAG_INTERNAL return 0", __func__);
	      return 0;
	    }
	}
    }
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: end with rib %p nexthop %p return 0", __func__, rib, nexthop);

  return 0;
}

/*gujd: 2013-01-14, Changed for interface local route.
From nexthop_active_ipv4_normal_check.*/
static int
nexthop_active_ipv4_update(struct rib *rib, struct nexthop *nexthop, int set,
		     struct route_node *rn)
{
  struct rib *match;
  struct nexthop *newhop;

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start with rib %p nexthop %p rn top %p", __func__, rib, nexthop, rn);

  if (set)
    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
  
  if (IS_ZEBRA_DEBUG_RIB)
      zlog_debug ("%s: found rn %p", __func__, rn);
  
  /* Pick up selected route. */
  for (match = rn->info; match; match = match->next) 
  {
	if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug ("%s: try match %p with flags 0x%x and type 0x%x", __func__, match,match->flags,match->type);
	if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	{
	   if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug ("%s: continue with match %p", __func__, match);
		if (! match  || match->type == ZEBRA_ROUTE_BGP)
		{
			if (IS_ZEBRA_DEBUG_RIB)
			  zlog_debug ("%s: line %d, no match rib", __func__,__LINE__);
			return 0;
		}
		else
		{
		  if (match->type == ZEBRA_ROUTE_CONNECT)
		    {
		      /* Directly point connected route. */
		      newhop = match->nexthop;
			  
			  if (IS_ZEBRA_DEBUG_RIB)
			  	zlog_debug("%s: line %d, newhop(%p),nexthop type[%d].\n",__func__,__LINE__,newhop,nexthop->type);
			  
		      if (newhop && (nexthop->type == NEXTHOP_TYPE_IPV4||nexthop->type == NEXTHOP_TYPE_IPV4_IFINDEX))
			  	{
				  struct interface *ifp = NULL;
				  int slot_num = 0;
				  
				  if (IS_ZEBRA_DEBUG_RIB)
				  	  zlog_debug("%s : line %d ,nexthop index[%d]interface[%s],new nexthop index[%d] interface[%s].\n",
						  __func__,__LINE__,nexthop->ifindex,nexthop->ifname,newhop->ifindex,newhop->ifname);

				  ifp = if_lookup_by_index(newhop->ifindex);
				  if(!ifp)
				  {
					  zlog_debug("%s : line %d , unkown nexthop interface index[%d]\n",__func__,__LINE__,nexthop->ifindex);
					  continue;
					/* break;*/
				   }
				  
				  if (IS_ZEBRA_DEBUG_RIB)
				  	zlog_debug("%s: line %d , nexthop interface name[%s],if_scope[%d]...\n",__func__,__LINE__,ifp->name,ifp->if_scope); 
				  
				  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
				  {
					  slot_num = get_slot_num(ifp->name);
					  if(slot_num == product->board_id)/*local board local interface*/
					  {
						  zlog_debug("%s: line %d , nexthop local interface[name:%s],nexthop->index[%d].\n",__func__,__LINE__,ifp->name,nexthop->ifindex);
						  nexthop->ifindex = newhop->ifindex;
						  return 1;
					  }
					  else
					  {
						  if (IS_ZEBRA_DEBUG_RIB)
						    zlog_debug("%s: line %d , rib(%p),rib->rib_scope(%d),nexthop other interface[name:%s], nexthop->index[%d].\n",
						  			__func__,__LINE__,rib,rib->rib_scope,ifp->name,nexthop->ifindex);
						  nexthop->ifindex = newhop->ifindex;					  
						  SET_FLAG (rib->rib_scope,ZEBRA_FLAG_INTERFACE_LOCAL_ROUTE);
						  continue;
						  /* break;*/
						}
					}
				  else
				  	{
				  	  if(nexthop->type != NEXTHOP_TYPE_IPV4_IFINDEX)
				  	  	{
						  if (IS_ZEBRA_DEBUG_RIB)
						  	zlog_debug("%s: line %d ,nexthop type[%d].\n",__func__,__LINE__,nexthop->type);
					   	  nexthop->ifindex = newhop->ifindex;
				  	  	}
				  	}
				  }
			  
			   /*CID 11033 (#2 of 2): Dereference after null check (FORWARD_NULL)
				20. var_deref_op: Dereferencing null pointer "newhop".
				So add if(newhop)*/
				if(newhop)
				{
		      		if (IS_ZEBRA_DEBUG_RIB)
					zlog_debug ("%s:line %d, found ZEBRA_ROUTE_CONNECT nexthop %p with ifindex 0x%x, return 1", __func__,__LINE__, newhop,newhop->ifindex);
				  }
		      return 1;
		    }
		  else if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_INTERNAL))
		    {
			  if (IS_ZEBRA_DEBUG_RIB)
			    zlog_debug ("%s: process ZEBRA_FLAG_INTERNAL rib %p with flag 0x%x", __func__, rib,rib->flags);
		      for (newhop = match->nexthop; newhop; newhop = newhop->next)
			if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
			    && ! CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_RECURSIVE))
			  {
			    if (set)
			      {
				SET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
				nexthop->rtype = newhop->type;
				if (newhop->type == NEXTHOP_TYPE_IPV4 ||
				    newhop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
				  nexthop->rgate.ipv4 = newhop->gate.ipv4;
				if (newhop->type == NEXTHOP_TYPE_IFINDEX
				    || newhop->type == NEXTHOP_TYPE_IFNAME
				    || newhop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
				  nexthop->rifindex = newhop->ifindex;
			      }
			    if (IS_ZEBRA_DEBUG_RIB)
			      zlog_debug ("%s: newhop %p NEXTHOP_FLAG_FIB and ! NEXTHOP_FLAG_RECURSIVE return 1", __func__, newhop);
			    
			    return 1;
			  }
			if (IS_ZEBRA_DEBUG_RIB)
			  zlog_debug ("%s: end ZEBRA_FLAG_INTERNAL rib %p with flag 0x%x return 0", __func__, rib,rib->flags);

			
		      return 0;
		    }
		  else
		    {
			  if (IS_ZEBRA_DEBUG_RIB)
			    zlog_debug ("%s: not ZEBRA_ROUTE_CONNECT not ZEBRA_FLAG_INTERNAL return 0", __func__);
		      return 0;
		    }
		}
	  }
  }

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: end with rib %p nexthop %p return 0", __func__, rib, nexthop);

  return 0;
}

int
nexthop_active_ipv4_check (struct rib *rib, struct nexthop *nexthop, int set,
								struct route_node *top)

{
  struct route_table *table;
  struct route_node *rn;
  struct prefix_ipv4 p;
  int ret = 0;
 
 /* Make lookup prefix. */
 memset (&p, 0, sizeof (struct prefix_ipv4));
 p.family = AF_INET;
 p.prefixlen = IPV4_MAX_PREFIXLEN;
 p.prefix = nexthop->gate.ipv4;
 

  /* Lookup table.	*/
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
	return 0;
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug ("%s: lookup prefix %s/%d", __func__, inet_ntoa (p.prefix),p.prefixlen);
  
  /*  for (rn = route_top (table); rn; rn = route_next (rn))*/
  rn = route_node_match (table, (struct prefix *) &p);/*gujd: 2013-09-13. Change for the longest route mask match*/
  if(rn)
  {
    /*skip yourself*/
  	/*if(rn == top)
  	{
  		zlog_debug("%s: line %d , top(%p) == rn(%p)....\n",__func__,__LINE__,top,rn);
		continue;
  	}*/
  	
	/*other rn search*/
	if (IS_ZEBRA_DEBUG_RIB)
	{
	    char buf1[BUFSIZ];
	    char buf2[BUFSIZ];
	    memset(buf1,0,BUFSIZ);
	    memset(buf2,0,BUFSIZ);
		
		inet_ntop (AF_INET, &rn->p.u.prefix, buf1, BUFSIZ);/*cp ip address from u.prefix to buf1*/
		 zlog_debug("%s: line %d , rn->p.. IP is [%s]..\n",__func__,__LINE__,buf1);
		inet_ntop (AF_INET, &p.prefix, buf2, BUFSIZ);
		 zlog_debug("%s: line %d , p ..IP is [%s]..\n",__func__,__LINE__,buf2);
	  }	
/*	if(prefix_match(&rn->p, (struct prefix *)&p))*//*bingo the rn */

	if (rn->p.prefixlen <= p.prefixlen && prefix_match (&rn->p, (struct prefix *)&p))
	{
		ret = nexthop_active_ipv4_update(rib,nexthop,set,rn);
		if(ret == 1)
		{
			zlog_debug("%s : line %d , bingo ret = 1 .\n",__func__,__LINE__);
			return ret;
			}
	}
	else
	{
		if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug("%s : line %d , oterh rn[%p].\n",__func__,__LINE__,rn);
	/*	continue;*/
	 }
  }
  if (IS_ZEBRA_DEBUG_RIB)
   zlog_debug("%s: line %d, leave ret[%d]. \n",__func__,__LINE__,ret);

	return 0;
 
}

/*gujd: 2013-01-14, Changed for interface local route.*/
static int
nexthop_active_ipv4 (struct rib *rib, struct nexthop *nexthop, int set,
		     struct route_node *top)
{
	if(!product)/*not distribute system or boot start*/
	{
		return nexthop_active_ipv4_normal_check(rib,nexthop,set,top);
	}
	else if(product->product_type == PRODUCT_TYPE_7605I ||product->product_type==PRODUCT_TYPE_8603)
	{
		return nexthop_active_ipv4_check(rib,nexthop,set,top);
	}
	else /*86xx*/
	{
	#if 0
		if(product->board_type == BOARD_IS_ACTIVE_MASTER)
		 return nexthop_active_ipv4_check(rib,nexthop,set,top);
	/*	return nexthop_active_ipv4_normal_check(rib,nexthop,set,top);*/
		else
		 return nexthop_active_ipv4_normal_check(rib,nexthop,set,top);
	#else
		return nexthop_active_ipv4_normal_check(rib,nexthop,set,top);
	#endif
		}
}

#ifdef HAVE_IPV6
/* If force flag is not set, do not modify falgs at all for uninstall
   the route from FIB. */
static int
nexthop_active_ipv6 (struct rib *rib, struct nexthop *nexthop, int set,
		     struct route_node *top)
{
  struct prefix_ipv6 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *match;
  struct nexthop *newhop;

  if (nexthop->type == NEXTHOP_TYPE_IPV6)
    nexthop->ifindex = 0;

  if (set)
    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);

  /* Make lookup prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = IPV6_MAX_PREFIXLEN;
  p.prefix = nexthop->gate.ipv6;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  rn = route_node_match (table, (struct prefix *) &p);
  while (rn)
    {
      route_unlock_node (rn);
      
      /* If lookup self prefix return immidiately. */
      if (rn == top)
	return 0;

      /* Pick up selected route. */
      for (match = rn->info; match; match = match->next)
	if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	  break;

      /* If there is no selected route or matched route is EGP, go up
         tree. */
      if (! match
	  || match->type == ZEBRA_ROUTE_BGP)
	{
	  do {
	    rn = rn->parent;
	  } while (rn && rn->info == NULL);
	  if (rn)
	    route_lock_node (rn);
	}
      else
	{
	  if (match->type == ZEBRA_ROUTE_CONNECT)
	    {
	      /* Directly point connected route. */
	      newhop = match->nexthop;

	      if (newhop && nexthop->type == NEXTHOP_TYPE_IPV6)
		nexthop->ifindex = newhop->ifindex;
	      
	      return 1;
	    }
	  else if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_INTERNAL))
	    {
	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
		    && ! CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_RECURSIVE))
		  {
		    if (set)
		      {
			SET_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE);
			nexthop->rtype = newhop->type;
			if (newhop->type == NEXTHOP_TYPE_IPV6
			    || newhop->type == NEXTHOP_TYPE_IPV6_IFINDEX
			    || newhop->type == NEXTHOP_TYPE_IPV6_IFNAME)
			  nexthop->rgate.ipv6 = newhop->gate.ipv6;
			if (newhop->type == NEXTHOP_TYPE_IFINDEX
			    || newhop->type == NEXTHOP_TYPE_IFNAME
			    || newhop->type == NEXTHOP_TYPE_IPV6_IFINDEX
			    || newhop->type == NEXTHOP_TYPE_IPV6_IFNAME)
			  nexthop->rifindex = newhop->ifindex;
		      }
		    return 1;
		  }
	      return 0;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }
  return 0;
}
#endif /* HAVE_IPV6 */

int check_interface_belong_to_local_board_set_local_mode(const char *ifname)
{
	struct interface *ifp = NULL;
	int slot_num = 0;
	
	ifp = if_lookup_by_name(ifname);
	if(product && ifp && CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*only local*/
	{
	/*	target->gate.ifname = ifp->name;*/
		slot_num = get_slot_num(ifp->name);
		if(slot_num != product->board_id)/* other board, interface*/
		{
			/*make a new func */
			zlog_debug("%s: line %d , other board interfac(%s)set local.\n",__func__,__LINE__,ifp->name);
			return 1;
		}
		else/*local board normal deal with.*/
		{
			zlog_debug("%s: line %d , local board interfac(%s)set local.\n",__func__,__LINE__,ifp->name);
			return 0;
		}
	}
	/*global interface*/
	return 0;
}
void 
active_master_packet_route_info_to_send(struct prefix *p, struct rib *rib, int slot, int comand)
{
  struct listnode *node, *nnode;
	tipc_client *client;

	
  for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, client))
	{
	/*	if(comand == ZEBRA_IPV4_ROUTE_ADD ||comand == ZEBRA_IPV6_ROUTE_ADD)*/
		{
			//if(client->board_id == slot)/*Add only send to one board.*/
			{
			
			   if ((p->family == AF_INET) &&(rib->type != ZEBRA_ROUTE_CONNECT))
				  tipc_server_packet_route_multipath (comand, client, p, rib);/**ipv4**/
			   
#ifdef HAVE_IPV6
			   if ((p->family == AF_INET6)&&(rib->type != ZEBRA_ROUTE_CONNECT)
				&&( rib->type != ZEBRA_ROUTE_KERNEL))
				  tipc_server_packet_route_multipath (comand, client, p, rib);/**ipv6**/
#endif /* HAVE_IPV6 */	  
			 }
			/*other board don't care.*/
			
		}
		#if 0
		else/*delete send to all baord.*/
		{
		
		   if ((p->family == AF_INET) &&(rib->type != ZEBRA_ROUTE_CONNECT))
			  tipc_server_packet_route_multipath (comand, client, p, rib);/**ipv4**/
		   
#ifdef HAVE_IPV6
		   if ((p->family == AF_INET6)&&(rib->type != ZEBRA_ROUTE_CONNECT)
			&&( rib->type != ZEBRA_ROUTE_KERNEL))
			  tipc_server_packet_route_multipath (comand, client, p, rib);/**ipv6**/
#endif /* HAVE_IPV6 */	  
	
	  }
	#endif
	}
}

/* formation the rib info used to send to other board. */
static void
packet_static_ipv4_route (struct prefix *p, struct static_ipv4 *si,int slot,struct nexthop *nexthop,int ifindex,int cmd)
{
  struct rib *rib;
  struct route_node *rn;
  struct route_table *table;

  /* This is new static route. */
  rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
  
  rib->type = ZEBRA_ROUTE_STATIC;
  rib->distance = si->distance;
  rib->metric = 0;
  rib->nexthop_num = 0;

  switch (si->type)
  {
      case STATIC_IPV4_GATEWAY:
      /*  nexthop_ipv4_add (rib, &si->gate.ipv4);*/
	  nexthop_ipv4_ifindex_add(rib, &si->gate.ipv4,ifindex);
        break;
      case STATIC_IPV4_IFNAME:
        nexthop_ifname_add (rib, si->gate.ifname);
        break;
      case STATIC_IPV4_BLACKHOLE:
        nexthop_blackhole_add (rib);
        break;
    }

  /* Save the flags of this static routes (reject, blackhole) */
  rib->flags = si->flags;
  
  /*send by tipc to other board*/
  active_master_packet_route_info_to_send(p,rib,slot,cmd);/*add*/
  
  /*free this temp rib.*/
  XFREE(MTYPE_RIB,rib);
  


}


static int
rib_search_route (struct route_node *rn, struct rib *rib, struct prefix *p, struct static_ipv4 *target,int cmd)
{
  struct nexthop *nexthop;
  int len = 0;
  char buf[BUFSIZ];
  char *ifname = NULL;
  struct prefix_ipv4 gate_ipv4;
  
  memset(&gate_ipv4, 0, sizeof(struct prefix_ipv4));
  gate_ipv4.family = AF_INET;
  gate_ipv4.prefixlen = IPV4_MAX_PREFIXLEN;
  gate_ipv4.prefix = target->gate.ipv4;

  /* Nexthop information. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
  {
	if(rib->type == ZEBRA_ROUTE_CONNECT)
	{	
	   int slot_num = 0;
	   struct interface *ifp = NULL;
	   int ifindex = 0;
	   
	   if (IS_ZEBRA_DEBUG_RIB)
		{
		    char buf1[BUFSIZ];
		    char buf2[BUFSIZ];
		    char buf3[BUFSIZ];
			
			memset(buf1,0,BUFSIZ);
			memset(buf2,0,BUFSIZ);
			memset(buf3,0,BUFSIZ);
			inet_ntop (AF_INET, &p->u.prefix, buf1, BUFSIZ);/*cp ip address from u.prefix to buf1*/
			 zlog_debug("%s: line %d , p ..[%s]..\n",__func__,__LINE__,buf1);
			inet_ntop (AF_INET, &rn->p.u.prefix, buf2, BUFSIZ);
			 zlog_debug("%s: line %d , rn->p ..[%s]..\n",__func__,__LINE__,buf2);
			inet_ntop (AF_INET, &gate_ipv4.prefix, buf3, BUFSIZ);
			 zlog_debug("%s: line %d , gate_ipve ..[%s]..\n",__func__,__LINE__,buf3);
		  }	
		
	   if(prefix_match(&rn->p,(struct prefix*)&gate_ipv4))	/*bingo*/
	   {
		 if (IS_ZEBRA_DEBUG_RIB)
		 	zlog_debug("%s: line %d .nexthop type[%d].\n",__func__,__LINE__,nexthop->type);
		 switch(nexthop->type)
		 {
		 	case NEXTHOP_TYPE_IFINDEX:
				
				ifname = ifindex_to_ifname(nexthop->ifindex);
				ifp = if_lookup_by_name(ifname);
				if(ifp && CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*only local*/
				{
				/*	target->gate.ifname = ifp->name;*/
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)/* send to other board.*/
					{
						/*make a new func */
						zlog_debug("%s: line %d , static route for [Other] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
						packet_static_ipv4_route(p,target, slot_num,nexthop,nexthop->ifindex,cmd);
						
					}
					else/*local board normal deal with.*/
					{
						zlog_debug("%s: line %d , static route for [Local] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
					/*	static_install_ipv4(p,target);*/
					}
				}
				/*global interface normal deal with.*/
	  			break;
				
			case NEXTHOP_TYPE_IFNAME:
				
				ifp = if_lookup_by_name(nexthop->ifname);
				if(ifp && CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*only local*/
				{
					ifindex = ifname2ifindex(ifp->name);
				/*	target->gate.ifname = ifp->name;*/
					
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)/* send to other board.*/
					{						
						zlog_debug("%s: line %d , static route for [Other] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
						packet_static_ipv4_route(p,target, slot_num,nexthop,ifindex,cmd);
						
					}
					else/*local board normal deal with.*/
					{
						zlog_debug("%s: line %d , static route for [Local] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
					  /* static_install_ipv4(p,target);*/
					}
				}
				/*global interface normal deal with.*/
	  			break;
				
			default:

					break;
				
			}
		}
		
	}
	else
	{
		/*other route, do not care.*/
		return 0;
		}

  }
  if (IS_ZEBRA_DEBUG_RIB)
	zlog_debug("Leave fun %s .\n",__func__);

return 0;/*nomarl or deal (for loop ) over.*/

}

int
rib_lookup_ipv4_connected_route (struct prefix *p, struct static_ipv4 *target,int cmd)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct nexthop *nexthop;
  
  if (IS_ZEBRA_DEBUG_RIB)
	zlog_debug("Enter fun %s .\n",__func__);

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  
  if (IS_ZEBRA_DEBUG_RIB)
	 zlog_debug("%s: line %d begin to search connect route .\n",__func__,__LINE__);

  for (rn = route_top (table); rn; rn = route_next (rn))
  {
    for (rib = rn->info; rib; rib = rib->next)
    {
		rib_search_route (rn, rib, p,target,cmd);
	  }
  	}
 
 if (IS_ZEBRA_DEBUG_RIB)
   zlog_debug("Leave fun %s .\n",__func__);
 return 0;
 
}

struct rib *
rib_match_ipv4 (struct in_addr addr)
{
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *match;
  struct nexthop *newhop;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = addr;

  rn = route_node_match (table, (struct prefix *) &p);

  while (rn)
    {
      route_unlock_node (rn);
      
      /* Pick up selected route. */
      for (match = rn->info; match; match = match->next)
	{
	  if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
	    continue;
	  if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	    break;
	}

      /* If there is no selected route or matched route is EGP, go up
         tree. */
      if (! match 
	  || match->type == ZEBRA_ROUTE_BGP)
	{
	  do {
	    rn = rn->parent;
	  } while (rn && rn->info == NULL);
	  if (rn)
	    route_lock_node (rn);
	}
      else
	{
	  if (match->type == ZEBRA_ROUTE_CONNECT)
	    /* Directly point connected route. */
	    return match;
	  else
	    {
	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB))
		  return match;
	      return NULL;
	    }
	}
    }
  return NULL;
}

struct rib *
rib_lookup_ipv4 (struct prefix_ipv4 *p)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *match;
  struct nexthop *nexthop;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  rn = route_node_lookup (table, (struct prefix *) p);

  /* No route for this prefix. */
  if (! rn)
    return NULL;

  /* Unlock node. */
  route_unlock_node (rn);

  /* Pick up selected route. */
  for (match = rn->info; match; match = match->next)
    {
      if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
	continue;
      if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	break;
    }

  if (! match || match->type == ZEBRA_ROUTE_BGP)
    return NULL;

  if (match->type == ZEBRA_ROUTE_CONNECT)
    return match;
  
  for (nexthop = match->nexthop; nexthop; nexthop = nexthop->next)
    if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
      return match;

  return NULL;
}

#ifdef HAVE_IPV6
struct rib *
rib_match_ipv6 (struct in6_addr *addr)
{
  struct prefix_ipv6 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *match;
  struct nexthop *newhop;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = IPV6_MAX_PREFIXLEN;
  IPV6_ADDR_COPY (&p.prefix, addr);

  rn = route_node_match (table, (struct prefix *) &p);

  while (rn)
    {
      route_unlock_node (rn);
      
      /* Pick up selected route. */
      for (match = rn->info; match; match = match->next)
	{
	  if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
	    continue;
	  if (CHECK_FLAG (match->flags, ZEBRA_FLAG_SELECTED))
	    break;
	}

      /* If there is no selected route or matched route is EGP, go up
         tree. */
      if (! match 
	  || match->type == ZEBRA_ROUTE_BGP)
	{
	  do {
	    rn = rn->parent;
	  } while (rn && rn->info == NULL);
	  if (rn)
	    route_lock_node (rn);
	}
      else
	{
	  if (match->type == ZEBRA_ROUTE_CONNECT)
	    /* Directly point connected route. */
	    return match;
	  else
	    {
	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB))
		  return match;
	      return NULL;
	    }
	}
    }
  return NULL;
}
#endif /* HAVE_IPV6 */

static int
nexthop_active_check (struct route_node *rn, struct rib *rib,
		      struct nexthop *nexthop, int set)
{
  struct interface *ifp;
  int family;
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start with rib %p nexthop %p type %x flags %x", __func__, rib, nexthop, nexthop->type,nexthop->flags);
  family = 0;
  switch (nexthop->type)
    {
    case NEXTHOP_TYPE_IFINDEX:
      ifp = if_lookup_by_index (nexthop->ifindex);
      if (ifp && if_is_operative(ifp))
	SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      else
	UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      break;
    case NEXTHOP_TYPE_IPV6_IFNAME:
      family = AFI_IP6;
    case NEXTHOP_TYPE_IFNAME:
      ifp = if_lookup_by_name (nexthop->ifname);
      if (ifp && if_is_operative(ifp))
	{
	  if (set)
	    nexthop->ifindex = ifp->ifindex;
	  SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	}
      else
	{
/*
	  if (set)
	    nexthop->ifindex = 0;
*/
	  UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	}
      break;
    case NEXTHOP_TYPE_IPV4:
    case NEXTHOP_TYPE_IPV4_IFINDEX:
      family = AFI_IP;
      if (nexthop_active_ipv4 (rib, nexthop, set, rn))
	SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      else
	UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      break;
#ifdef HAVE_IPV6
    case NEXTHOP_TYPE_IPV6:
      family = AFI_IP6;
      if (nexthop_active_ipv6 (rib, nexthop, set, rn))
	SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      else
	UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      break;
    case NEXTHOP_TYPE_IPV6_IFINDEX:
      family = AFI_IP6;
      if (IN6_IS_ADDR_LINKLOCAL (&nexthop->gate.ipv6))
	{
	  ifp = if_lookup_by_index (nexthop->ifindex);
	  if (ifp && if_is_operative(ifp))
	    SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	  else
	    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	}
      else
	{
	  if (nexthop_active_ipv6 (rib, nexthop, set, rn))
	    SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	  else
	    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
	}
      break;
#endif /* HAVE_IPV6 */
    case NEXTHOP_TYPE_BLACKHOLE:
      SET_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
      break;
    default:
      break;
    }
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: end with rib %p nexthop %p type %x flags %x", __func__, rib, nexthop, nexthop->type,nexthop->flags);
  return CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);
}

static int
nexthop_active_update (struct route_node *rn, struct rib *rib, int set)
{
  struct nexthop *nexthop;
  int active;

  rib->nexthop_active_num = 0;
  UNSET_FLAG (rib->flags, ZEBRA_FLAG_CHANGED);

  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
      active = CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE);

      nexthop_active_check (rn, rib, nexthop, set);
      if ((MULTIPATH_NUM == 0 || rib->nexthop_active_num < MULTIPATH_NUM)
          && active != CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
        SET_FLAG (rib->flags, ZEBRA_FLAG_CHANGED);

      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
        rib->nexthop_active_num++;
    }
  
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: return with nexthop_active_num %d MULTIPATH_NUM=%d", __func__, rib->nexthop_active_num,MULTIPATH_NUM);
  return rib->nexthop_active_num;
}


#define RIB_SYSTEM_ROUTE(R) \
        ((R)->type == ZEBRA_ROUTE_KERNEL || (R)->type == ZEBRA_ROUTE_CONNECT)

static void
rib_install_kernel (struct route_node *rn, struct rib *rib)
{
  int ret = 0;
  struct nexthop *nexthop;
  
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug("enter func %s....\n",__func__);

  switch (PREFIX_FAMILY (&rn->p))
    {
    case AF_INET:
		  ret = kernel_add_ipv4 (&rn->p, rib);
		  
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = kernel_add_ipv6 (&rn->p, rib);
      break;
#endif /* HAVE_IPV6 */
    }

  /* This condition is never met, if we are using rt_socket.c *//**when vice board kernel route exist , go to let kernel add again will case ret < 0 **/
 /* if (ret < 0 && product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)*//**gjd: changed for vice board route bug, nexthop flags case zebra fib table err**/

   if (ret < 0)
  {
    /*zlog_err("%s : line %d , ret(%d), route_errno(%d)..........\n",__func__,__LINE__,ret,route_boot_errno,safe_strerror(-route_boot_errno));*/
  	if(route_boot_errno != -EEXIST)/*EEXIST: means kernel have this route .*/
      for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
		UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
	  route_boot_errno = 0;
    }
   
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug("leave func %s ....\n",__func__);
}

/* Uninstall the route from kernel. */
static int
rib_uninstall_kernel (struct route_node *rn, struct rib *rib)
{
  int ret = 0;
  struct nexthop *nexthop;
  
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug("enter func %s....\n",__func__);
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: processing rn %p rib %p", __func__, rn,rib);

  switch (PREFIX_FAMILY (&rn->p))
    {
    case AF_INET:
      ret = kernel_delete_ipv4 (&rn->p, rib);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = kernel_delete_ipv6 (&rn->p, rib);
      break;
#endif /* HAVE_IPV6 */
    }

  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug("leave func %s ....\n",__func__);
  return ret;
}

/* Uninstall the route from kernel. */
static void
rib_uninstall (struct route_node *rn, struct rib *rib)
{
	int ret=0;
  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
    {
      redistribute_delete (&rn->p, rib);
	  
	  /**add by gjd: tipc server redistribute to client to detele route**/
//	  ret=get_slot_type();
//	  if(ret == 1)
//	if(product->board_type == BOARD_IS_ACTIVE_MASTER)
	if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
	  tipc_redistribute_delete(&rn->p, rib);
	  /**2011-01**/
	  
		
      if (! RIB_SYSTEM_ROUTE (rib))
				rib_uninstall_kernel (rn, rib);
      UNSET_FLAG (rib->flags, ZEBRA_FLAG_SELECTED);
    }
}

static void rib_unlink (struct route_node *, struct rib *);

/* Core function for processing routing information base. */
static wq_item_status
rib_process (struct work_queue *wq, void *data)
{
  struct rib *rib;
  struct rib *next;
  struct rib *fib = NULL;
  struct rib *select = NULL;
  struct rib *del = NULL;
  struct route_node *rn = data;
  int installed = 0;
  struct nexthop *nexthop = NULL;

  int ret=0;
  
#if 0
  assert (rn);
	
#else
	  if(!rn)
	  {
	  
		  zlog(NULL, LOG_CRIT, "line %u, function %s",
			 __LINE__,(__func__ ? __func__ : "?"));
		  zlog_backtrace(LOG_CRIT);
		  return WQ_ERROR;
	  }
#endif
  
  if (IS_ZEBRA_DEBUG_RIB)
	  zlog_debug("enter func %s.....\n",__func__);

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: processing rn %p with prefix %s/%d", __func__, rn,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);

  /*gjd: fixed for AXSSZFI-484 , 2011-11-19: am 11:00*/
  if(!rn->info)
  	{
	  
		  zlog_warn("%s : rn->info is NULL !\n",__func__);
		  return WQ_ERROR;
	  }
  for (rib = rn->info; rib; rib = next)
	{
	  next = rib->next;

	  if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug (" %s: line %d, check rib %p with flags %x status 0x%x types 0x%x  ",
				__func__,__LINE__,rib,rib->flags, rib->status, rib->type);
#if 1	
	  if (rib->type == ZEBRA_ROUTE_CONNECT)
	  	{
	  		SET_FLAG (rib->flags, ZEBRA_FLAG_SELECTED);
			
			nexthop_active_update (rn, rib, 1);
		    if (IS_ZEBRA_DEBUG_RIB)
       		 zlog_debug ("%s: line %d, rib %p with flags %x", __func__,__LINE__, rib,rib->flags);
	  	}
#endif 
	  /* Currently installed rib. */
	  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	    {
/*	    
	      assert (fib == NULL);
*/
	      fib = rib;
		  if (IS_ZEBRA_DEBUG_RIB)
		    zlog_debug ("%s: line %d ,found fib %p with flag %x", __func__, __LINE__,fib,fib->flags);
	    }
	  
	  /* Unlock removed routes, so they'll be freed, bar the FIB entry,
	   * which we need to do do further work with below.
	   */
	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	    {
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, rib %p with status %x RIB_ENTRY_REMOVED", __func__, __LINE__,rib,rib->status);

	      if (rib != fib)
	        {
	          if (IS_ZEBRA_DEBUG_RIB)
	            zlog_debug ("%s: line %d, rn %p, removing rib %p with flags %x", __func__,__LINE__, rn, rib,rib->flags);
			  			rib_unlink (rn, rib);
	        }
	      else
	      	{
			  if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED)&& rib->type == ZEBRA_ROUTE_CONNECT)
			  {
				  rib_unlink (rn, rib);
				  fib = NULL;

			  }
			  else
			  {
				  del = rib;

			  }  

		   }
		  
	      continue;
	    }
	  
	  /* Skip unreachable nexthop. */
	  if (! nexthop_active_update (rn, rib, 0)) 
	  	{
		  if (IS_ZEBRA_DEBUG_RIB)
	  	  	zlog_debug("%s: line %d ,rib(%p),rib->rib_scope(%d)..........\n",__func__,__LINE__,rib,rib->rib_scope);
	  	  if(CHECK_FLAG(rib->rib_scope,ZEBRA_FLAG_INTERFACE_LOCAL_ROUTE)
		  	 && product && product->board_type == BOARD_IS_ACTIVE_MASTER )
		  {
		  	
		  	zlog_debug("%s: line %d, rib nexthop include (set local) interface, but all nexthop are not inactive on master(donnot care) .\n",__func__,__LINE__);
		  	}
	  	  
		  else
		  {
	       if (IS_ZEBRA_DEBUG_RIB)
			 zlog_debug ("%s:line %d, skipping unreachable nexthop", __func__,__LINE__);
	    	continue;
	  	  	}
	  	}
	  /* Infinit distance. */
	  if (rib->distance == DISTANCE_INFINITY) {
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s:line %d, skipping infinit distance nexthop", __func__,__LINE__);
	    	continue;
	  	}
	  /* Newly selected rib, the common case. */
	  if (!select)
	    {
	      select = rib;
		if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, firstly select fib %p with flags %x", __func__,__LINE__,select,select->flags);/*****************/
	      continue;
	    }
	  
	  /* filter route selection in following order:
	   * - connected beats other types
	   * - lower distance beats higher
	   * - lower metric beats higher for equal distance
	   * - last, hence oldest, route wins tie break.
	   */
	  
	  /* Connected routes. Pick the last connected
	   * route of the set of lowest metric connected routes.
	   */
	  if (rib->type == ZEBRA_ROUTE_CONNECT){
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, rib type ZEBRA_ROUTE_CONNECT", __func__,__LINE__);
				if (select->type != ZEBRA_ROUTE_CONNECT
		  			|| rib->metric <= select->metric) {
			  	if (IS_ZEBRA_DEBUG_RIB)
			    	zlog_debug ("%s: line %d, select->type != ZEBRA_ROUTE_CONNECT || rib->metric <= select->metric", __func__,__LINE__);
		      select = rib;
			  
			  
		   }  
		   
	  if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s : line %d, rib %p with flags %x", __func__,__LINE__, rib,rib->flags);
	   	continue;
	  }
	  else if (select->type == ZEBRA_ROUTE_CONNECT) {
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, select->type == ZEBRA_ROUTE_CONNECT", __func__,__LINE__);
	    continue;
	  	}
	  /* higher distance loses */
	  if (rib->distance > select->distance){
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, rib->distance > select->distance", __func__,__LINE__);
	    	continue;
	  	}
	  /* lower wins */
	  if (rib->distance < select->distance)
	    {
	      select = rib;
		if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d, rib->distance < select->distance", __func__,__LINE__);
	      continue;
	    }
	  
	  /* metric tie-breaks equal distance */
	  if (rib->metric <= select->metric) {
	      if (IS_ZEBRA_DEBUG_RIB)
			zlog_debug ("%s: line %d ,rib->metric <= select->metric", __func__,__LINE__);
	  	
	    select = rib;
	  	}	
	}

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: line %d, we got select %p fib %p del %p", __func__,__LINE__,select,fib, del);

  
  /* Same route is selected. */
  if (select && select == fib)
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: line %d, Updating existing route, select %p, fib %p",
                     __func__,__LINE__, select, fib);
      if (CHECK_FLAG (select->flags, ZEBRA_FLAG_CHANGED))/**for route change**/
	    {
		  	if (IS_ZEBRA_DEBUG_RIB)
			  zlog_debug ("%s: line %d, CHECK_FLAG (select->flags, ZEBRA_FLAG_CHANGED)", __func__,__LINE__);
	      redistribute_delete (&rn->p, select);

		 if( product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
		  tipc_redistribute_delete (&rn->p, select);
		  /**2011-01**/
		  
	      if (! RIB_SYSTEM_ROUTE (select))
	        rib_uninstall_kernel (rn, select);

	      /* Set real nexthop. */
	      nexthop_active_update (rn, select, 1);

	      if (! RIB_SYSTEM_ROUTE (select))
	        rib_install_kernel (rn, select);
	      redistribute_add (&rn->p, select);
		  
		  if(product != NULL &&product->board_type == BOARD_IS_ACTIVE_MASTER)
		  	tipc_redistribute_add (&rn->p, select);
		  /**2011-01**/
		  
	    }
      else if (! RIB_SYSTEM_ROUTE (select))
        {
          /* Housekeeping code to deal with 
             race conditions in kernel with linux
             netlink reporting interface up before IPv4 or IPv6 protocol
             is ready to add routes.
             This makes sure the routes are IN the kernel.
           */
		  if (IS_ZEBRA_DEBUG_RIB)
		    zlog_debug ("%s: line %d, ! RIB_SYSTEM_ROUTE (select)", __func__,__LINE__);

          for (nexthop = select->nexthop; nexthop; nexthop = nexthop->next)
            {
              if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
                installed = 1;
            }
		  /*gujd: 2012-02-29, pm 6:30 . Delete it [ if (! installed)]  bugfixed for vrrp of virtual mac cased static route lost . 
		      If install this static route repeat , the netlink err info (route_boot_errno == -EEXIST) get from kernel , 
		      so we ingnore this err when install this static route to kernel again . This can make sure this route exist in kernel .*/
		      
         /* if (! installed) */
            rib_install_kernel (rn, select);
		  /**add by gjd :for tipc client boot up ,download static route to kernel**/
//		  if((get_slot_type()==2)&& select->type == ZEBRA_ROUTE_STATIC)
#if 0
		  if(product != NULL && (product->board_type == BOARD_IS_VICE|| product->board_type == BOARD_IS_BACKUP_MASTER) && select->type == ZEBRA_ROUTE_STATIC)

	  	    {	  	    
			  	if (IS_ZEBRA_DEBUG_RIB)
	  	    		zlog_debug("%s : line %d, for tipc client boot up download static route from tipc server ....\n",__func__,__LINE__);
	  	    	nexthop_active_update(rn, select, 1);
				rib_install_kernel (rn, select);
	  	    	
	  	    }
		  /**2011-02**/
#endif		  
        }
	 
      goto end;
    }

  /* Uninstall old rib from forwarding table. */
  if (fib)
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: line %d, fib %p", __func__,__LINE__, fib);
      redistribute_delete (&rn->p, fib);
	  
	if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
	  tipc_redistribute_delete (&rn->p, fib);
	/**2011-01**/
	
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: line %d, RIB_SYSTEM_ROUTE(fib) %x", __func__, __LINE__,RIB_SYSTEM_ROUTE (fib));

      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: line %d, fib %p with flags %x", __func__,__LINE__, fib,fib->flags);
		
	  if (! RIB_SYSTEM_ROUTE (fib)){
				rib_uninstall_kernel (rn, fib);
     			UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	  	}
	   if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s : line %d, fib %p with flags %x", __func__,__LINE__, fib,fib->flags);

      /* Set real nexthop. */
      nexthop_active_update (rn, fib, 1);
    }

  /* Install new rib into forwarding table. */
  if (select)
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: line %d,  Adding route, select %p", __func__, __LINE__,select);
      /* Set real nexthop. */
      nexthop_active_update (rn, select, 1);

      if (! RIB_SYSTEM_ROUTE (select))
        rib_install_kernel (rn, select);
      SET_FLAG (select->flags, ZEBRA_FLAG_SELECTED);
      redistribute_add (&rn->p, select);

	  /**add by gjd: tipc server redistribute to client to add route**/
//	  ret=get_slot_type();
//	  if(ret == 1)
//		if(product->board_type == BOARD_IS_ACTIVE_MASTER)
	  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
	  	tipc_redistribute_add (&rn->p, select);
	  /**2011-01**/
	  
	  if (IS_ZEBRA_DEBUG_RIB)
		 zlog_debug ("%s: line %d, select %p with flags %x", __func__,__LINE__, select,select->flags);
    }

  /* FIB route was removed, should be deleted */
  if (del)
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s:line %d , Deleting fib %p, rn %p", __func__, __LINE__,del, rn);
	  if(ZEBRA_ROUTE_CONNECT == del->type)
	  {
		  /*added by scx for connect route del send msg to npd route*/
			rib_uninstall_kernel(rn,del);
	  
      	if (IS_ZEBRA_DEBUG_RIB)
	  	  zlog_debug ("%s:line %d, Deleting connect fib %p, rn %p", __func__,__LINE__, del, rn);
	  }
	  rib_unlink (rn, del);
    }

end:
  if (IS_ZEBRA_DEBUG_RIB_Q)
    zlog_debug ("%s: line %d, rn %p dequeued", __func__,__LINE__, rn);
  if (rn->info){
    UNSET_FLAG (((struct rib *)rn->info)->rn_status, RIB_ROUTE_QUEUED);
   /*added by gxd*/
    UNSET_FLAG (((struct rib *)rn->info)->rn_status, RIB_ROUTE_HEAD);	
  }
  route_unlock_node (rn); /* rib queue lock */
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug("leave func %s.....\n",__func__);
  return WQ_SUCCESS;
}

/* Add route_node to work queue and schedule processing */
static void
rib_queue_add (struct zebra_t *zebra, struct route_node *rn)
{
#if 0
	assert (zebra && rn);
	
#else
	  if(!zebra || !rn)
	  {
	  
		  zlog(NULL, LOG_CRIT, "line %u, function %s",
			 __LINE__,(__func__ ? __func__ : "?"));
		  zlog_backtrace(LOG_CRIT);
		 /* return NULL;*/
		 return;
	  }
#endif
  
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: processing rn %p with prefix %s/%d", __func__, rn,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
  /* Pointless to queue a route_node with no RIB entries to add or remove */
  if (!rn->info)
    {    
	  if (IS_ZEBRA_DEBUG_RIB)
      	zlog_debug ("%s: called for route_node (%p, %d) with no ribs",
                  __func__, rn, rn->lock);
      zlog_backtrace(LOG_DEBUG);
      return;
    }

  /* Route-table node already queued, so nothing to do */
  if (CHECK_FLAG (((struct rib *)rn->info)->rn_status, RIB_ROUTE_QUEUED))
	{
		if (IS_ZEBRA_DEBUG_RIB_Q)
		  zlog_debug ("%s: rn %p already queued", __func__, rn);
		return;
	}

  if (((struct rib *)rn->info)->type == ZEBRA_ROUTE_CONNECT)
  {
	  route_lock_node (rn);
	  if (zebra->ribq == NULL){
		  zlog_err ("%s: work_queue does not exist!", __func__);
		  route_unlock_node (rn);
		  return;
	  }
	  
	  work_queue_pre_add(zebra->ribq, rn);
	  
	  SET_FLAG (((struct rib *)rn->info)->rn_status, RIB_ROUTE_QUEUED);
	  if (IS_ZEBRA_DEBUG_RIB_Q)
		  zlog_debug ("%s: rn %p is queued in the head", __func__, rn);

	  return;
	}

  route_lock_node (rn); /* rib queue lock */

  if (IS_ZEBRA_DEBUG_RIB_Q)
    zlog_info ("%s: work queue added %s/%d", __func__, inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
/*
  assert (zebra);
*/
  if (zebra->ribq == NULL){
	  zlog_err ("%s: work_queue does not exist!", __func__);
	  route_unlock_node (rn);
	  return;
	}
  
  work_queue_add (zebra->ribq, rn);

  SET_FLAG (((struct rib *)rn->info)->rn_status, RIB_ROUTE_QUEUED);

  if (IS_ZEBRA_DEBUG_RIB_Q)
    zlog_debug ("%s: rn %p queued %s/%d", __func__, rn, inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);

  return;
}

/* initialise zebra rib work queue */
static void
rib_queue_init (struct zebra_t *zebra)
{
#if 0
  assert (zebra);
  
#else
	if(!zebra)
	{
	
		zlog(NULL, LOG_CRIT, " line %u, function %s",
		   __LINE__,(__func__ ? __func__ : "?"));
		zlog_backtrace(LOG_CRIT);
		/*return NULL;*/
		return;
	}
#endif
  if (! (zebra->ribq = work_queue_new (zebra->master, 
                                       "route_node processing")))
    {
      zlog_err ("%s: could not initialise work queue!", __func__);
      return;
    }

  /* fill in the work queue spec */
  zebra->ribq->spec.workfunc = &rib_process;
  zebra->ribq->spec.errorfunc = NULL;
  /* XXX: TODO: These should be runtime configurable via vty */
  zebra->ribq->spec.max_retries = 3;
  zebra->ribq->spec.hold = rib_process_hold_time;
  
  return;
}

/* RIB updates are processed via a queue of pointers to route_nodes.
 *
 * The queue length is bounded by the maximal size of the routing table,
 * as a route_node will not be requeued, if already queued.
 *
 * RIBs are submitted via rib_addnode and rib_delnode, which set
 * minimal state and then submit route_node to queue for best-path
 * selection later. Order of add/delete state changes are preserved for
 * any given RIB. 
 *
 * Deleted RIBs are reaped during best-path selection.
 *
 * rib_addnode
 * |-> rib_link or unset RIB_ENTRY_REMOVE        |->Update kernel with
 * |-> rib_addqueue                              |    best RIB, if required
 *          |                                    |
 *          |-> .......................... -> rib_process
 *          |                                    |
 * |-> rib_addqueue                              |-> rib_unlink
 * |-> set RIB_ENTRY_REMOVE                           |
 * rib_delnode                                  (RIB freed)
 *
 *
 * Queueing state for a route_node is kept in the head RIB entry, this
 * state must be preserved as and when the head RIB entry of a
 * route_node is changed by rib_unlink / rib_link. A small complication,
 * but saves having to allocate a dedicated object for this.
 * 
 * Refcounting (aka "locking" throughout the GNU Zebra and Quagga code):
 *
 * - route_nodes: refcounted by:
 *   - RIBs attached to route_node:
 *     - managed by: rib_link/unlink
 *   - route_node processing queue
 *     - managed by: rib_addqueue, rib_process.
 *
 */
 
/* Add RIB to head of the route node. */
static void
rib_link (struct route_node *rn, struct rib *rib)
{
  struct rib *head;
  
#if 0
			  assert (rib && rn);
#else
			if(!rib || !rn)
			{
			
				zlog(NULL, LOG_CRIT, "line %u, function %s",
				   __LINE__,(__func__ ? __func__ : "?"));
				zlog_backtrace(LOG_CRIT);
				return ;
			}
#endif

  route_lock_node (rn); /* rn route table reference */

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: rn %p, rib %p", __func__, rn, rib);

  head = rn->info;
  if (head)
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: new head, rn_status copied over", __func__);
      head->prev = rib;
      /* Transfer the rn status flags to the new head RIB */
      rib->rn_status = head->rn_status;
    }
  rib->next = head;
  rn->info = rib;
	if (rib->type == ZEBRA_ROUTE_CONNECT)
  	SET_FLAG (rib->rn_status, RIB_ROUTE_HEAD);
  rib_queue_add (&zebrad, rn);
}

void
rib_addnode (struct route_node *rn, struct rib *rib)
{
  /* RIB node has been un-removed before route-node is processed. 
   * route_node must hence already be on the queue for processing.. 
   */
  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
    {
      if (IS_ZEBRA_DEBUG_RIB)
        zlog_debug ("%s: rn %p, un-removed rib %p with prefix %s/%d",
                    __func__, rn, rib,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
      UNSET_FLAG (rib->status, RIB_ENTRY_REMOVED);
      return;
    }
  rib_link (rn, rib);
}

static void
rib_unlink (struct route_node *rn, struct rib *rib)
{
  struct nexthop *nexthop, *next;

#if 0
  assert (rn && rib);
	
#else
	  if(!rn || !rib)
	  {
	  
		  zlog(NULL, LOG_CRIT, "line %u, function %s",
			 __LINE__,(__func__ ? __func__ : "?"));
		  zlog_backtrace(LOG_CRIT);
		  return ;
	  }
#endif

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: rn %p, rib %p",
                __func__, rn, rib);

  if (rib->next)
    rib->next->prev = rib->prev;

  if (rib->prev)
    rib->prev->next = rib->next;
  else
    {
      rn->info = rib->next;
      
      if (rn->info)
        {
          if (IS_ZEBRA_DEBUG_RIB)
            zlog_debug ("%s: rn %p, rib %p, new head copy",
                        __func__, rn, rib);
          rib->next->rn_status = rib->rn_status;
        }
    }

  /* free RIB and nexthops */
  for (nexthop = rib->nexthop; nexthop; nexthop = next)
    {
      next = nexthop->next;
      nexthop_free (nexthop);
    }
  XFREE (MTYPE_RIB, rib);

  route_unlock_node (rn); /* rn route table reference */
}

void
rib_delnode (struct route_node *rn, struct rib *rib)
{
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: rn %p, rib %p, removing with prefix %s/%d ", __func__, rn, rib,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
  SET_FLAG (rib->status, RIB_ENTRY_REMOVED);
	
	if (rib->type == ZEBRA_ROUTE_CONNECT)
  	SET_FLAG (rib->rn_status, RIB_ROUTE_HEAD);
		
  rib_queue_add (&zebrad, rn);
}

int
rib_add_ipv4 (int type, int flags, struct prefix_ipv4 *p, 
	      struct in_addr *gate, unsigned int ifindex, u_int32_t vrf_id,
	      u_int32_t metric, u_char distance)
{
  struct rib *rib=NULL;
  struct rib *same = NULL;
  struct route_table *table;
  struct route_node *rn;
  struct nexthop *nexthop;
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);
  
  if (IS_ZEBRA_DEBUG_RIB)
  	zlog_debug("%s: type %d,flags %d, ifindex %d, ",__func__,type,flags,ifindex);

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  /* Make it sure prefixlen is applied to the prefix. */
  apply_mask_ipv4 (p);

  /* Set default distance by route type. */
  if (distance == 0)
    {
      distance = route_info[type].distance;

      /* iBGP distance is 200. */
      if (type == ZEBRA_ROUTE_BGP && CHECK_FLAG (flags, ZEBRA_FLAG_IBGP))
	distance = 200;
    }

  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
        continue;
      
      if (rib->type != type)
					continue;
      if (rib->type != ZEBRA_ROUTE_CONNECT)
        {
          same = rib;
          break;
        }
      /* Duplicate connected route comes in. */
      else if ((nexthop = rib->nexthop) &&
	       nexthop->type == NEXTHOP_TYPE_IFINDEX &&
	       nexthop->ifindex == ifindex &&
	       !CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	{
	  rib->refcnt++;
	  return 0 ;
	}
    }

  /* Allocate new rib structure. */
  rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
  rib->type = type;
  rib->distance = distance;
  rib->flags = flags;
  rib->metric = metric;
  rib->table = vrf_id;
  rib->nexthop_num = 0;
  rib->uptime = time (NULL);
  /*added by gxd*/
	
//	if (rib->type == ZEBRA_ROUTE_CONNECT)
//  	SET_FLAG (rib->rn_status, RIB_ROUTE_HEAD);
  /*2008-12-26 18:01:40*/
  /* Nexthop settings. */
  if (gate)
    {
      if (ifindex)
	nexthop_ipv4_ifindex_add (rib, gate, ifindex);
      else
	nexthop_ipv4_add (rib, gate);
    }
  else
    nexthop_ifindex_add (rib, ifindex);

  /* If this route is kernel route, set FIB flag to the route. */
  if (type == ZEBRA_ROUTE_KERNEL || type == ZEBRA_ROUTE_CONNECT)
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    	{
			SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

			/*CID 13869 (#1-2 of 2): Dereference before null check (REVERSE_INULL)
			check_after_deref: Null-checking "rib" suggests that it may be null, but it has already been dereferenced on all paths leading to the check.
			No used , delete it.*/
			#if 0
			if(rib)
				{
					if (IS_ZEBRA_DEBUG_RIB)
	       		 		zlog_debug ("%s line %d: rib %p with flags %x", __func__,__LINE__, rib,rib->flags);
				}
			else
  				zlog_debug ("%s line %d: rib is null", __func__,__LINE__);
			#endif
    	}

  /* Link new rib to node.*/
	if (IS_ZEBRA_DEBUG_RIB)
		zlog_debug ("%s line %d: rib %p with flags %x", __func__,__LINE__, rib,rib->flags);
	rib_addnode (rn, rib);

	if (IS_ZEBRA_DEBUG_RIB)
       	zlog_debug ("%s line %d: rib %p with flags %x", __func__,__LINE__, rib,rib->flags);
  if(same)
  	{
  		if (IS_ZEBRA_DEBUG_RIB)
			 zlog_debug ("%s:rn %p, same %p, with prefix %s/%d ,same type %d, same flag %d", __func__, rn, same,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen,same->type,same->flags);
  	}
  /* Free implicit route.*/
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

int
rib_add_ipv4_equal (int type, int flags, struct prefix_ipv4 *p, 
	      struct in_addr **gate,  u_int32_t vrf_id,
	      u_int32_t metric, u_char distance,u_char nexthopnum)
{
  struct rib *rib;
  struct rib *same = NULL;
  struct route_table *table;
  struct route_node *rn;
  struct nexthop *nexthop;
	int i;
	
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);


  /* Allocate new rib structure. */
  rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
  rib->type = type;
  rib->distance = distance;
  rib->flags = flags;
  rib->metric = metric;
  rib->table = vrf_id;
  rib->nexthop_num = 0;
  rib->uptime = time (NULL);
/*delete by gjd*/
//rib->equalize=1;
  /* Nexthop settings. */
  for(i=1;i<=nexthopnum;i++)
	{
		nexthop_ipv4_add(rib,gate[i]);
	}

	return rib_add_ipv4_multipath(p,rib);

  
}

int
rib_add_ipv4_multipath (struct prefix_ipv4 *p, struct rib *rib)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *same;
  struct nexthop *nexthop;
  
  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  /* Make it sure prefixlen is applied to the prefix. */
  apply_mask_ipv4 (p);

  /* Set default distance by route type. */
  if (rib->distance == 0)
    {
      rib->distance = route_info[rib->type].distance;

      /* iBGP distance is 200. */
      if (rib->type == ZEBRA_ROUTE_BGP 
	  && CHECK_FLAG (rib->flags, ZEBRA_FLAG_IBGP))
	rib->distance = 200;
    }

  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (same = rn->info; same; same = same->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
        continue;
      
      if (same->type == rib->type && same->table == rib->table
	  && same->type != ZEBRA_ROUTE_CONNECT)
        break;
    }
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (rib->type == ZEBRA_ROUTE_KERNEL || rib->type == ZEBRA_ROUTE_CONNECT)
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
      SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

  /* Link new rib to node.*/
  rib_addnode (rn, rib);

  /* Free implicit route.*/
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

/* XXX factor with rib_delete_ipv6 */
int
rib_delete_ipv4 (int type, int flags, struct prefix_ipv4 *p,
		 struct in_addr *gate, unsigned int ifindex, u_int32_t vrf_id)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  	if(IS_ZEBRA_DEBUG_RIB)
		zlog_debug("type:%d;flags:%d; ifindex %d",type,flags,ifindex);

  /* Apply mask. */
  apply_mask_ipv4 (p);

  if (IS_ZEBRA_DEBUG_RIB && gate)
    zlog_debug ("rib_delete_ipv4(): route delete %s/%d via %s ifindex %d",
		       inet_ntop (AF_INET, &p->prefix, buf1, BUFSIZ),
		       p->prefixlen, 
		       inet_ntoa (*gate), 
		       ifindex);

  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
    {
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
    }

  /* Lookup same type route. */
  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
        continue;

      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	fib = rib;

      if (rib->type != type)
	continue;
      if (rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) &&
	  nexthop->type == NEXTHOP_TYPE_IFINDEX && nexthop->ifindex == ifindex)
	{
	  if (rib->refcnt)
	    {
	      rib->refcnt--;
		  /*gjd : add for same connect route in route node*/
/*	  rib_delnode (rn, rib);*//*at 2011-07-25, same connect route add resloved, so delete it*/
	      route_unlock_node (rn);
	      route_unlock_node (rn);
	      return 0;
	    }
	  same = rib;
	  break;
	}
      /* Make sure that the route found has the same gateway. */
      else if ((gate == NULL && (nexthop = rib->nexthop) && (ifindex == nexthop->ifindex))||
	       (gate && (nexthop = rib->nexthop)&&  
	       (IPV4_ADDR_SAME (&(nexthop->gate.ipv4), gate) || IPV4_ADDR_SAME (&(nexthop->rgate.ipv4), gate)))) 
        {
	  		same = rib;

			  if (IS_ZEBRA_DEBUG_RIB)
		    zlog_debug ("%s: rn %p, rib:same is %p, with prefix %s/%d ", __func__, rn, same,inet_ntoa (rn->p.u.prefix4),rn->p.prefixlen);
			
			if (IS_ZEBRA_DEBUG_RIB)
					  zlog_debug ("rib same:nexthop %p,include: nexthop->type %d, nexthop->ifindex %d ,nexthop->ifname %p", same->nexthop, same->nexthop->type,
																		same->nexthop->ifindex,same->nexthop->ifname);

	
	 	 	break;
		}
    }

  /* If same type of route can't be found and this message is from
     kernel. */
  if (! same)
    {
      if (fib && type == ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
	    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
      else
	{
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
	  route_unlock_node (rn);
	  return ZEBRA_ERR_RTNOEXIST;
	}
    }

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: check delnode ", __func__);

  
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

/* Install static route into rib. */
 void
static_install_ipv4 (struct prefix *p, struct static_ipv4 *si)
{
  struct rib *rib;
  struct route_node *rn;
  struct route_table *table;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return;

  /* Lookup existing route */
  rn = route_node_get (table, p);
  for (rib = rn->info; rib; rib = rib->next)
    {
       if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
         continue;
        


       if (rib->type == ZEBRA_ROUTE_STATIC && rib->distance == si->distance)
			{
				/*delete by gjd*/

/*				if((rib->equalize!=1 && si->type!=STATIC_IPV4_EQUALIZE)
					||(rib->equalize==1 && si->type==STATIC_IPV4_EQUALIZE))

*/					break;
			} 

    }

  if (rib)
    {
      /* Same distance static route is there.  Update it with new
         nexthop. */
      route_unlock_node (rn);
      switch (si->type)
        {
          case STATIC_IPV4_GATEWAY:						
          	nexthop_ipv4_add (rib, &si->gate.ipv4);
            break;
/*delete by gjd*/
/*	case STATIC_IPV4_EQUALIZE:
						rib->equalize =1;
          	nexthop_ipv4_add (rib, &si->gate.ipv4);
            break;
            */
          case STATIC_IPV4_IFNAME:
            nexthop_ifname_add (rib, si->gate.ifname);
            break;
          case STATIC_IPV4_BLACKHOLE:
            nexthop_blackhole_reject_add (rib,si->flags);
            break;
        }
	  	rib_queue_add (&zebrad, rn);
    }
  else
    {
      /* This is new static route. */
      rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
      
      rib->type = ZEBRA_ROUTE_STATIC;
      rib->distance = si->distance;
      rib->metric = 0;
      rib->nexthop_num = 0;

      switch (si->type)
        {
          case STATIC_IPV4_GATEWAY:
            nexthop_ipv4_add (rib, &si->gate.ipv4);
            break;
/*delete by gjd*/
/*   case STATIC_IPV4_EQUALIZE:
						rib->equalize =1;
            nexthop_ipv4_add (rib, &si->gate.ipv4);
            break;
	*/					
          case STATIC_IPV4_IFNAME:
            nexthop_ifname_add (rib, si->gate.ifname);
            break;
          case STATIC_IPV4_BLACKHOLE:
            nexthop_blackhole_add (rib);
            break;
        }

      /* Save the flags of this static routes (reject, blackhole) */
      rib->flags = si->flags;

      /* Link this rib to the tree. */
      rib_addnode (rn, rib);
    }
}

static int
static_ipv4_nexthop_same (struct nexthop *nexthop, struct static_ipv4 *si)
{
/*change by gjd*/
/*before*/
/* 
	if (nexthop->type == NEXTHOP_TYPE_IPV4
      && (si->type == STATIC_IPV4_GATEWAY ||si->type == STATIC_IPV4_EQUALIZE)
      && IPV4_ADDR_SAME (&nexthop->gate.ipv4, &si->gate.ipv4))
*/
/*after*/
   if (nexthop->type == NEXTHOP_TYPE_IPV4
      && si->type == STATIC_IPV4_GATEWAY 
      && IPV4_ADDR_SAME (&nexthop->gate.ipv4, &si->gate.ipv4))
    return 1;
  if (nexthop->type == NEXTHOP_TYPE_IFNAME
      && si->type == STATIC_IPV4_IFNAME
      && strcmp (nexthop->ifname, si->gate.ifname) == 0)
    return 1;
  if (nexthop->type == NEXTHOP_TYPE_BLACKHOLE
      && si->type == STATIC_IPV4_BLACKHOLE)
    return 1;
  return 0;
}

/* Uninstall static route from RIB. */
 void
static_uninstall_ipv4 (struct prefix *p, struct static_ipv4 *si)
{
  struct route_node *rn;
  struct rib *rib;
  struct nexthop *nexthop;
  struct route_table *table;

  /* Lookup table.  */
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return;
  
  /* Lookup existing route with type and distance. */
  rn = route_node_lookup (table, p);
	
  if (! rn)
    return;

	for (rib = rn->info; rib; rib = rib->next)
	{
		if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
		continue;
/*delete by gjd*/

		if (rib->type == ZEBRA_ROUTE_STATIC && rib->distance == si->distance)
		{
//			if((rib->equalize!=1 && si->type!=STATIC_IPV4_EQUALIZE)
//				||(rib->equalize==1 && si->type==STATIC_IPV4_EQUALIZE))
				break;
//			else 
//				continue;
		}
		
	}

  if (! rib)
    {
      route_unlock_node (rn);
      return;
    }

  /* Lookup nexthop. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    if (static_ipv4_nexthop_same (nexthop, si))
      break;


  /* Can't find nexthop. */
  if (! nexthop)
    {
      route_unlock_node (rn);
      return;
    }
  
  /* Check nexthop. */
  if (rib->nexthop_num == 1)
    rib_delnode (rn, rib);
  else
    {
      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
        rib_uninstall (rn, rib);
      nexthop_delete (rib, nexthop);
      nexthop_free (nexthop);
      rib_queue_add (&zebrad, rn);
    }
  /* Unlock node. */
  route_unlock_node (rn);
}

/* Add static route into static route configuration. */
int
static_add_ipv4_by_vtysh (struct prefix *p, struct in_addr *gate, const char *ifname,
		 u_char flags, u_char distance, u_int32_t vrf_id)
{
  u_char type = 0;
  struct route_node *rn;
  struct static_ipv4 *si;
  struct static_ipv4 *pp;
  struct static_ipv4 *cp;
  struct static_ipv4 *update = NULL;
  struct route_table *stable;
  int ret = 0;
  struct route_node *rn_search = NULL;
  struct route_table *table_search = NULL;
  struct rib *rib_search= NULL;

  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP, SAFI_UNICAST, vrf_id);
  if (! stable)
    return -1;
  
  /* Lookup static route prefix. */
  //rn = route_node_get (stable, p);

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
  /*2008-12-5 14:35:41*/

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
    static_delete_ipv4_by_vtysh(p, gate, ifname, update->distance, vrf_id);

 /*gujd: 2013-01-29, am 11:07 . Add for limit the nexthop for static route num Max is 8.*/
#if 1
  if(table_search = vrf_table (AFI_IP, SAFI_UNICAST, 0))
  {
	if(rn_search = route_node_lookup(table_search,p))
	{
	  for (rib_search = rn_search->info;rib_search; rib_search= rib_search->next)
	  {
	    if(rib_search->type == ZEBRA_ROUTE_STATIC && rib_search->nexthop_num >= 8)
		{
			zlog_debug("%s: line %d , The max nexthop num is MULTIPATH_NUM[%d], the rib_search->nexthop_num[%d].\n",
						__func__,__LINE__,MULTIPATH_NUM,rib_search->nexthop_num);
			return -6;
		 }
	   }
	 }
   }

#endif

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
  
  /*gujd: 2013-01-14, Add for interface local route.*/
  /*Only distribute system do this. research rib to find connected route*/
  if(product)
  {
	if (IS_ZEBRA_DEBUG_RIB)
	  zlog_debug("%s: line %d, Add route by vtysh begin to search.\n",__func__,__LINE__);
	
  	ret = rib_lookup_ipv4_connected_route(p,si,ZEBRA_IPV4_ROUTE_ADD);/*ret = 0, noraml deal ,go on */
	/*
	if(ret > 0)
	{
	  zlog_debug("%s: line %d, Add route by vtysh search the interface loacl route.\n",__func__,__LINE__);
	  return 1;
	 }
	*/
   }
	
  /* Install into rib. */
  static_install_ipv4 (p, si);

  return 1;
}

/* Add static route into static route configuration. */
int
static_add_ipv4 (struct prefix *p, struct in_addr *gate, const char *ifname,
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
  //rn = route_node_get (stable, p);

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
  /*2008-12-5 14:35:41*/

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

/*delete by gjd*/
/* Add static route into static route configuration. */

#if 0
int
static_add_ipv4_equal (struct prefix *p, struct in_addr *gate, const char *ifname,
		 u_char flags, u_char distance, u_int32_t vrf_id)
{
/*delete by gjd*/
//  u_char type = STATIC_IPV4_EQUALIZE;
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
  //rn = route_node_get (stable, p);

  /* Make flags. */
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
  /*2008-12-5 14:35:41*/

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
/*change by gjd*/
/*before*/
 //     if (si->type == STATIC_IPV4_EQUALIZE && (cp->type == STATIC_IPV4_EQUALIZE ||cp->type == STATIC_IPV4_GATEWAY))

/*after*/
	  if (cp->type == STATIC_IPV4_GATEWAY))
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
#endif

/*add by gjd : Delete static route  for the format :
no ip route A.B.C.D/M (reject|blackhole|null0) ,
In fact , the null0 is in: no ip route A.B.C.D/M (A.B.C.D|INTERFACE|null0) */
int
static_delete_ipv4_special (struct prefix *p, struct in_addr *gate, const char *ifname,
		    u_char distance, u_int32_t vrf_id, u_char flags)
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
//  else if ((ifname != NULL) && (strncasecmp (ifname, "Null0", strlen (ifname)) != 0))
  else if(ifname)
    type = STATIC_IPV4_IFNAME;
  else
//  	if(CHECK_FLAG (flags, ZEBRA_FLAG_REJECT))
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

 /**gjd:for detele reject route by reject, not by blackhole or null0**/
  if((CHECK_FLAG (flags, ZEBRA_FLAG_BLACKHOLE)||(flags == 0))&&(CHECK_FLAG (si->flags, ZEBRA_FLAG_REJECT)))
	{	  
		route_unlock_node (rn);
		  return -4;
  	}
  /**gjd: for detele blackhole or null0 route , not by reject**/
  if((CHECK_FLAG (flags, ZEBRA_FLAG_REJECT))&&(CHECK_FLAG (si->flags, ZEBRA_FLAG_BLACKHOLE)))
  	{
  		route_unlock_node (rn);
		  return -5;
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


/* Delete static route from static route configuration. */
int
static_delete_ipv4_by_vtysh (struct prefix *p, struct in_addr *gate, const char *ifname,
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
  
  /*gujd: 2013-01-14, Add for interface local route.*/
  /*Only distribute system do this. research rib to find connected route*/
  if(product)
  {
	if (IS_ZEBRA_DEBUG_RIB)
	  zlog_debug("%s: line %d, Delete route by vtysh begin to search.\n",__func__,__LINE__);
  	rib_lookup_ipv4_connected_route(p,si,ZEBRA_IPV4_ROUTE_DELETE);
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


/* Delete static route from static route configuration. */
int
static_delete_ipv4 (struct prefix *p, struct in_addr *gate, const char *ifname,
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

int
set_policy_route(int is_add,char *name,char *ipaddr,int port,int mask,int id)
	{
		
		struct listnode *node, *nnode;
		struct zserv *client;
		
		tipc_client *master_board;
		tipc_server *vice_board;

		
		 if(product->board_type == BOARD_IS_ACTIVE_MASTER)/*activeÖ÷¿Ø*/
		  {
			if(zebrad.vice_board_list == NULL)
			  {
				 zlog_debug("%s, line = %d, zebrad.vice_board_list....", __func__, __LINE__);
				 return 0;
			  }
						
			 for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			  { 
			  	
				master_send_set_policy_route ( master_board,is_add,name,ipaddr, port,mask,id);
			  }
		   
		} 
		 return 1;
	}

/*delete by gjd*/
/* Delete static route from static route configuration. */
#if 0
int
static_delete_ipv4_equal (struct prefix *p, struct in_addr *gate, const char *ifname,
		    u_char distance, u_int32_t vrf_id)
{
/*delete by gjd*/
//  u_char type = STATIC_IPV4_EQUALIZE;
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


  /* Find same static route is the tree */
  for (si = rn->info; si; si = si->next){
    if (type == si->type
			&& gate 
			&& IPV4_ADDR_SAME (gate, &si->gate.ipv4))
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
#endif

#ifdef HAVE_IPV6


/* formation the rib info used to send to other board. */
static void
packet_static_ipv6_route (struct prefix *p, struct static_ipv6 *si,int slot,struct nexthop *nexthop,int ifindex,int cmd)
{
  struct rib *rib;
  struct route_node *rn;
  struct route_table *table;

  /* This is new static route. */
  rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
  
  rib->type = ZEBRA_ROUTE_STATIC;
  rib->distance = si->distance;
  rib->metric = 0;
  rib->nexthop_num = 0;

  switch (si->type)
   {
		case STATIC_IPV6_GATEWAY:
		//  nexthop_ipv6_add (rib, &si->ipv6);
		nexthop_ipv6_ifindex_add(rib, &si->ipv6,ifindex);
		  break;
		case STATIC_IPV6_IFNAME:
		  nexthop_ifname_add (rib, si->ifname);
		  break;
		case STATIC_IPV6_GATEWAY_IFNAME:
		  nexthop_ipv6_ifname_add (rib, &si->ipv6, si->ifname);
		  break;
	}

  /* Save the flags of this static routes (reject, blackhole) */
  rib->flags = si->flags;
  
  /*send by tipc to other board*/
  active_master_packet_route_info_to_send(p,rib,slot,cmd);/*add*/
  
  /*free this temp rib.*/
  XFREE(MTYPE_RIB,rib);
  


}


static int
rib_search_route_ipv6 (struct route_node *rn, struct rib *rib, struct prefix *p, struct static_ipv6 *target,int cmd)
{
  struct nexthop *nexthop;
  int len = 0;
  char buf[BUFSIZ];
  char *ifname = NULL;
  struct prefix_ipv6 gate_ipv6;
  
  memset(&gate_ipv6, 0, sizeof(struct prefix_ipv6));
  gate_ipv6.family = AF_INET6;
  gate_ipv6.prefixlen = IPV6_MAX_PREFIXLEN;
  gate_ipv6.prefix = target->ipv6;

  /* Nexthop information. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
  {
	if(rib->type == ZEBRA_ROUTE_CONNECT)
	{	
	   int slot_num = 0;
	   struct interface *ifp = NULL;
	   int ifindex = 0;
	   
	   if (IS_ZEBRA_DEBUG_RIB)
		{
		    char buf1[BUFSIZ];
		    char buf2[BUFSIZ];
		    char buf3[BUFSIZ];
			
			memset(buf1,0,BUFSIZ);
			memset(buf2,0,BUFSIZ);
			memset(buf3,0,BUFSIZ);
			inet_ntop (AF_INET6, &p->u.prefix, buf1, BUFSIZ);/*cp ip address from u.prefix to buf1*/
			 zlog_debug("%s: line %d , p ..[%s]..\n",__func__,__LINE__,buf1);
			inet_ntop (AF_INET6, &rn->p.u.prefix, buf2, BUFSIZ);
			 zlog_debug("%s: line %d , rn->p ..[%s]..\n",__func__,__LINE__,buf2);
			inet_ntop (AF_INET6, &gate_ipv6.prefix, buf3, BUFSIZ);
			 zlog_debug("%s: line %d , gate_ipve ..[%s]..\n",__func__,__LINE__,buf3);
		  }	
		
	   if(prefix_match(&rn->p,(struct prefix*)&gate_ipv6))	/*bingo*/
	   {
		 if (IS_ZEBRA_DEBUG_RIB)
		 	zlog_debug("%s: line %d .nexthop type[%d].\n",__func__,__LINE__,nexthop->type);
		
		 switch(nexthop->type)
		 {
		 	case NEXTHOP_TYPE_IFINDEX:
				
				ifname = ifindex_to_ifname(nexthop->ifindex);
				ifp = if_lookup_by_name(ifname);
				if(ifp && CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*only local*/
				{
				/*	target->gate.ifname = ifp->name;*/
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)/* send to other board.*/
					{
						/*make a new func */
						zlog_debug("%s: line %d , static route for [Other] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
						packet_static_ipv6_route(p,target, slot_num,nexthop,nexthop->ifindex,cmd);
						
					}
					else/*local board normal deal with.*/
					{
						zlog_debug("%s: line %d , static route for [Local] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
					/*	static_install_ipv4(p,target);*/
					}
				}
				/*global interface normal deal with.*/
	  			break;
				
			case NEXTHOP_TYPE_IFNAME:
				
				ifp = if_lookup_by_name(nexthop->ifname);
				if(ifp && CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*only local*/
				{
					ifindex = ifname2ifindex(ifp->name);
				/*	target->gate.ifname = ifp->name;*/
					
					slot_num = get_slot_num(ifp->name);
					if(slot_num != product->board_id)/* send to other board.*/
					{						
						zlog_debug("%s: line %d , static route for [Other] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
						packet_static_ipv6_route(p,target, slot_num,nexthop,ifindex,cmd);
						
					}
					else/*local board normal deal with.*/
					{
						zlog_debug("%s: line %d , static route for [Local] board interface(%s)when set local.\n",__func__,__LINE__,ifp->name);
					  /* static_install_ipv4(p,target);*/
					}
				}
				/*global interface normal deal with.*/
	  			break;
				
			default:

					break;
				
			} 
	
		}
		
	}
	else
	{
		/*other route, do not care.*/
		return 0;
		}

  }
  if (IS_ZEBRA_DEBUG_RIB)
	zlog_debug("Leave fun %s .\n",__func__);

return 0;/*nomarl or deal (for loop ) over.*/

}

int
rib_lookup_ipv6_connected_route (struct prefix *p, struct static_ipv6 *target,int cmd)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct nexthop *nexthop;
  
  if (IS_ZEBRA_DEBUG_RIB)
	zlog_debug("Enter fun %s .\n",__func__);

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  
  if (IS_ZEBRA_DEBUG_RIB)
	 zlog_debug("%s: line %d begin to search connect route .\n",__func__,__LINE__);

  for (rn = route_top (table); rn; rn = route_next (rn))
  {
    for (rib = rn->info; rib; rib = rib->next)
    {
		rib_search_route_ipv6(rn, rib, p,target,cmd);
	  }
  	}
 
 if (IS_ZEBRA_DEBUG_RIB)
   zlog_debug("Leave fun %s .\n",__func__);
 return 0;
 
}

int
rib_bogus_ipv6 (int type, struct prefix_ipv6 *p,
		struct in6_addr *gate, unsigned int ifindex, int table)
{
  if (type == ZEBRA_ROUTE_CONNECT && IN6_IS_ADDR_UNSPECIFIED (&p->prefix)) {
#if defined (MUSICA) || defined (LINUX)
    /* IN6_IS_ADDR_V4COMPAT(&p->prefix) */
    if (p->prefixlen == 96)
      return 0;
#endif /* MUSICA */
    return 1;
  }
  if (type == ZEBRA_ROUTE_KERNEL && IN6_IS_ADDR_UNSPECIFIED (&p->prefix)
      && p->prefixlen == 96 && gate && IN6_IS_ADDR_UNSPECIFIED (gate))
    {
      kernel_delete_ipv6_old (p, gate, ifindex, 0, table);
      return 1;
    }
  return 0;
}

int
rib_add_ipv6 (int type, int flags, struct prefix_ipv6 *p,
	      struct in6_addr *gate, unsigned int ifindex, u_int32_t vrf_id,
	      u_int32_t metric, u_char distance)
{
  struct rib *rib;
  struct rib *same = NULL;
  struct route_table *table;
  struct route_node *rn;
  struct nexthop *nexthop;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  /* Make sure mask is applied. */
  apply_mask_ipv6 (p);

  /* Set default distance by route type. */
  if (!distance)
    distance = route_info[type].distance;
  
  if (type == ZEBRA_ROUTE_BGP && CHECK_FLAG (flags, ZEBRA_FLAG_IBGP))
    distance = 200;

  /* Filter bogus route. */
  if (rib_bogus_ipv6 (type, p, gate, ifindex, 0))
    return 0;

  /* Lookup route node.*/
  rn = route_node_get (table, (struct prefix *) p);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
        continue;

      if (rib->type != type)
	continue;
      if (rib->type != ZEBRA_ROUTE_CONNECT)
	{
	  same = rib;
	  break;
	}
      else if ((nexthop = rib->nexthop) &&
	       nexthop->type == NEXTHOP_TYPE_IFINDEX &&
	       nexthop->ifindex == ifindex)
	{
	  rib->refcnt++;
	  return 0;
	}
    }

  /* Allocate new rib structure. */
  rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
  
  rib->type = type;
  rib->distance = distance;
  rib->flags = flags;
  rib->metric = metric;
  rib->table = vrf_id;
  rib->nexthop_num = 0;
  rib->uptime = time (NULL);

  /* Nexthop settings. */
  if (gate)
    {
      if (ifindex)
	nexthop_ipv6_ifindex_add (rib, gate, ifindex);
      else
	nexthop_ipv6_add (rib, gate);
    }
  else
    nexthop_ifindex_add (rib, ifindex);

  /* If this route is kernel route, set FIB flag to the route. */
  if (type == ZEBRA_ROUTE_KERNEL || type == ZEBRA_ROUTE_CONNECT)
    for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
      SET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

  /* Link new rib to node.*/
  rib_addnode (rn, rib);

  /* Free implicit route.*/
  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

/* XXX factor with rib_delete_ipv6 */
int
rib_delete_ipv6 (int type, int flags, struct prefix_ipv6 *p,
		 struct in6_addr *gate, unsigned int ifindex, u_int32_t vrf_id)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct rib *fib = NULL;
  struct rib *same = NULL;
  struct nexthop *nexthop;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];

  /* Apply mask. */
  apply_mask_ipv6 (p);

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return 0;
  
  /* Lookup route node. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
    {
      if (IS_ZEBRA_DEBUG_RIB)
	{
	  if (gate)
	    zlog_debug ("route %s/%d via %s ifindex %d doesn't exist in rib",
		       inet_ntop (AF_INET6, &p->prefix, buf1, BUFSIZ),
		       p->prefixlen,
		       inet_ntop (AF_INET6, gate, buf2, BUFSIZ),
		       ifindex);
	  else
	    zlog_debug ("route %s/%d ifindex %d doesn't exist in rib",
		       inet_ntop (AF_INET6, &p->prefix, buf1, BUFSIZ),
		       p->prefixlen,
		       ifindex);
	}
      return ZEBRA_ERR_RTNOEXIST;
    }

  /* Lookup same type route. */
  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
        continue;

      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	fib = rib;

      if (rib->type != type)
        continue;
      if (rib->type == ZEBRA_ROUTE_CONNECT && (nexthop = rib->nexthop) &&
	  nexthop->type == NEXTHOP_TYPE_IFINDEX && nexthop->ifindex == ifindex)
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
      else if (gate == NULL ||
	       ((nexthop = rib->nexthop) &&
	        (IPV6_ADDR_SAME (&nexthop->gate.ipv6, gate) ||
		 IPV6_ADDR_SAME (&nexthop->rgate.ipv6, gate))))
	{
	  same = rib;
	  break;
	}
    }

  /* If same type of route can't be found and this message is from
     kernel. */
  if (! same)
    {
      if (fib && type == ZEBRA_ROUTE_KERNEL)
	{
	  /* Unset flags. */
	  for (nexthop = fib->nexthop; nexthop; nexthop = nexthop->next)
	    UNSET_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (fib->flags, ZEBRA_FLAG_SELECTED);
	}
      else
	{
	  if (IS_ZEBRA_DEBUG_RIB)
	    {
	      if (gate)
		zlog_debug ("route %s/%d via %s ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET6, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   inet_ntop (AF_INET6, gate, buf2, BUFSIZ),
			   ifindex,
			   type);
	      else
		zlog_debug ("route %s/%d ifindex %d type %d doesn't exist in rib",
			   inet_ntop (AF_INET6, &p->prefix, buf1, BUFSIZ),
			   p->prefixlen,
			   ifindex,
			   type);
	    }
	  route_unlock_node (rn);
	  return ZEBRA_ERR_RTNOEXIST;
	}
    }

  if (same)
    rib_delnode (rn, same);
  
  route_unlock_node (rn);
  return 0;
}

/* Install static route into rib. */
int
static_install_ipv6 (struct prefix *p, struct static_ipv6 *si)
{
  struct rib *rib;
  struct route_table *table;
  struct route_node *rn;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return -1;

  /* Lookup existing route */
  rn = route_node_get (table, p);
  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG(rib->status, RIB_ENTRY_REMOVED))
        continue;

      if (rib->type == ZEBRA_ROUTE_STATIC && rib->distance == si->distance)
        break;
    }

  if (rib)
    {
      /* Same distance static route is there.  Update it with new
         nexthop. */
      route_unlock_node (rn);

      switch (si->type)
			{
			case STATIC_IPV6_GATEWAY:
			  nexthop_ipv6_add (rib, &si->ipv6);
			  break;
			case STATIC_IPV6_IFNAME:
			  nexthop_ifname_add (rib, si->ifname);
			  break;
			case STATIC_IPV6_GATEWAY_IFNAME:
			  nexthop_ipv6_ifname_add (rib, &si->ipv6, si->ifname);
			  break;
			}
    }
  else
    {
      /* This is new static route. */
      rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
      
      rib->type = ZEBRA_ROUTE_STATIC;
      rib->distance = si->distance;
      rib->metric = 0;
      rib->nexthop_num = 0;

      switch (si->type)
			{
			case STATIC_IPV6_GATEWAY:
			  nexthop_ipv6_add (rib, &si->ipv6);
			  break;
			case STATIC_IPV6_IFNAME:
			  nexthop_ifname_add (rib, si->ifname);
			  break;
			case STATIC_IPV6_GATEWAY_IFNAME:
			  nexthop_ipv6_ifname_add (rib, &si->ipv6, si->ifname);
			  break;
			}

      /* Save the flags of this static routes (reject, blackhole) */
      rib->flags = si->flags;

      /* Link this rib to the tree. */
      rib_addnode (rn, rib);
    }
	return 1;
}

static int
static_ipv6_nexthop_same (struct nexthop *nexthop, struct static_ipv6 *si)
{
  if (nexthop->type == NEXTHOP_TYPE_IPV6
      && si->type == STATIC_IPV6_GATEWAY
      && IPV6_ADDR_SAME (&nexthop->gate.ipv6, &si->ipv6))
    return 1;
  if (nexthop->type == NEXTHOP_TYPE_IFNAME
      && si->type == STATIC_IPV6_IFNAME
      && strcmp (nexthop->ifname, si->ifname) == 0)
    return 1;
  if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME
      && si->type == STATIC_IPV6_GATEWAY_IFNAME
      && IPV6_ADDR_SAME (&nexthop->gate.ipv6, &si->ipv6)
      && strcmp (nexthop->ifname, si->ifname) == 0)
    return 1;
  return 0;
}

 void
static_uninstall_ipv6 (struct prefix *p, struct static_ipv6 *si)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct nexthop *nexthop;

  /* Lookup table.  */
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return;

  /* Lookup existing route with type and distance. */
  rn = route_node_lookup (table, (struct prefix *) p);
  if (! rn)
    return;

  for (rib = rn->info; rib; rib = rib->next)
    {
      if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
        continue;
    
      if (rib->type == ZEBRA_ROUTE_STATIC && rib->distance == si->distance)
        break;
    }

  if (! rib)
    {
      route_unlock_node (rn);
      return;
    }

  /* Lookup nexthop. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    if (static_ipv6_nexthop_same (nexthop, si))
      break;

  /* Can't find nexthop. */
  if (! nexthop)
    {
      route_unlock_node (rn);
      return;
    }
  
  /* Check nexthop. */
  if (rib->nexthop_num == 1)
    {
      rib_delnode (rn, rib);
    }
  else
    {
      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
        rib_uninstall (rn, rib);
      nexthop_delete (rib, nexthop);
      nexthop_free (nexthop);
      rib_queue_add (&zebrad, rn);
    }
  /* Unlock node. */
  route_unlock_node (rn);
}
 
 /* Add static route into static route configuration. */
 int
 static_add_ipv6_by_vtysh (struct prefix *p, u_char type, struct in6_addr *gate,
		  const char *ifname, u_char flags, u_char distance,
		  u_int32_t vrf_id)
 {
   struct route_node *rn;
   struct static_ipv6 *si;
   struct static_ipv6 *pp;
   struct static_ipv6 *cp;
   struct route_table *stable;
 
   int ret = 0;
   
   /* Lookup table.  */
   stable = vrf_static_table (AFI_IP6, SAFI_UNICAST, vrf_id);
   if (! stable)
	 return -1;
	 
   if (!gate &&(type == STATIC_IPV6_GATEWAY || type == STATIC_IPV6_GATEWAY_IFNAME))
	 return -1;
   
   if (!ifname && (type == STATIC_IPV6_GATEWAY_IFNAME || type == STATIC_IPV6_IFNAME))
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
   
   /*Only distribute system do this. research rib to find connected route*/
   if(product)
   {
	 if (IS_ZEBRA_DEBUG_RIB)
	   zlog_debug("%s: line %d, Add route by vtysh begin to search.\n",__func__,__LINE__);
	 
	 ret = rib_lookup_ipv6_connected_route(p,si,ZEBRA_IPV6_ROUTE_ADD);/*ret = 0, noraml deal ,go on */
	 /*
	 if(ret > 0)
	 {
	   zlog_debug("%s: line %d, Add route by vtysh search the interface loacl route.\n",__func__,__LINE__);
	   return 1;
	  }
	 */
	}
	 
 
   /* Install into rib. */
   return static_install_ipv6 (p, si);
 
   return 1;
 }

/* Add static route into static route configuration. */
int
static_add_ipv6 (struct prefix *p, u_char type, struct in6_addr *gate,
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

/* Delete static route from static route configuration. */
int
static_delete_ipv6_by_vtysh (struct prefix *p, u_char type, struct in6_addr *gate,
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
    if (distance == si->distance 
	&& type == si->type
	&& (! gate || IPV6_ADDR_SAME (gate, &si->ipv6))
	&& (! ifname || strcmp (ifname, si->ifname) == 0))
      break;

  /* Can't find static route. */
  if (! si)
    {
      route_unlock_node (rn);
      return 0;
    }
  if(product)
  {
	if (IS_ZEBRA_DEBUG_RIB)
	  zlog_debug("%s: line %d, Delete route by vtysh begin to search.\n",__func__,__LINE__);
  	rib_lookup_ipv6_connected_route(p,si,ZEBRA_IPV6_ROUTE_DELETE);
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

/* Delete static route from static route configuration. */
int
static_delete_ipv6 (struct prefix *p, u_char type, struct in6_addr *gate,
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
    if (distance == si->distance 
	&& type == si->type
	&& (! gate || IPV6_ADDR_SAME (gate, &si->ipv6))
	&& (! ifname || strcmp (ifname, si->ifname) == 0))
      break;

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
#endif /* HAVE_IPV6 */

/* RIB update function. */
void
rib_update (void)
{
  struct route_node *rn;
  struct route_table *table;
  
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      if (rn->info)
        rib_queue_add (&zebrad, rn);

  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      if (rn->info)
        rib_queue_add (&zebrad, rn);
}

/* Interface goes up. */
static void
rib_if_up (struct interface *ifp)
{
  rib_update ();
}

/* Interface goes down. */
static void
rib_if_down (struct interface *ifp)
{
  rib_update ();
}

/* Remove all routes which comes from non main table.  */
static void
rib_weed_table (struct route_table *table)
{
  struct route_node *rn;
  struct rib *rib;
  struct rib *next;

  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      for (rib = rn->info; rib; rib = next)
	{
	  next = rib->next;

	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	    continue;

	  if (rib->table != zebrad.rtm_table_default &&
	      rib->table != RT_TABLE_MAIN)
            rib_delnode (rn, rib);
	}
}

/* Delete all routes from non main table. */
void
rib_weed_tables (void)
{
  rib_weed_table (vrf_table (AFI_IP, SAFI_UNICAST, 0));
  rib_weed_table (vrf_table (AFI_IP6, SAFI_UNICAST, 0));
}

/* Delete self installed routes after zebra is relaunched.  */
static void
rib_sweep_table (struct route_table *table)
{
  struct route_node *rn;
  struct rib *rib;
  struct rib *next;
  int ret = 0;

  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      for (rib = rn->info; rib; rib = next)
	{
	  next = rib->next;

	  if (CHECK_FLAG (rib->status, RIB_ENTRY_REMOVED))
	    continue;

	  if (rib->type == ZEBRA_ROUTE_KERNEL && 
	      CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELFROUTE))
	    {
	      ret = rib_uninstall_kernel (rn, rib);
	      if (! ret)
                rib_delnode (rn, rib);
	    }
	}
}

/* Sweep all RIB tables.  */
void
rib_sweep_route (void)
{
  rib_sweep_table (vrf_table (AFI_IP, SAFI_UNICAST, 0));
  rib_sweep_table (vrf_table (AFI_IP6, SAFI_UNICAST, 0));
}

/* Close RIB and clean up kernel routes. */
static void
rib_close_table (struct route_table *table)
{
  struct route_node *rn;
  struct rib *rib;

  if (table)
    for (rn = route_top (table); rn; rn = route_next (rn))
      for (rib = rn->info; rib; rib = rib->next)
        {
          if (! RIB_SYSTEM_ROUTE (rib)
	      && CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
            rib_uninstall_kernel (rn, rib);
        }
}

/* Close all RIB tables.  */
void
rib_close (void)
{
  rib_close_table (vrf_table (AFI_IP, SAFI_UNICAST, 0));
  rib_close_table (vrf_table (AFI_IP6, SAFI_UNICAST, 0));
}

/* Routing information base initialize. */
void
rib_init (void)
{
  rib_queue_init (&zebrad);
  /* VRF initialization.  */
  vrf_init ();
}
	
/*added by gxd*/

extern struct work_queue_item *
extern_work_queue_item_new (struct work_queue *wq);
extern int
extern_work_queue_schedule (struct work_queue *wq, unsigned int delay);

struct listnode *
work_queue_item_lookup(struct work_queue *wq,struct route_node *rn)
{
	  struct work_queue_item *item,*pre_item=NULL;
	  struct listnode *node=NULL,*item_node=NULL;
	  struct route_node *retrn;
#if 0
			assert (wq);
			assert(wq->items);
#else
		  if(!wq || !wq->items)
		  {
		  
			  zlog(NULL, LOG_CRIT, "line %u, function %s",
				 __LINE__,(__func__ ? __func__ : "?"));
			  zlog_backtrace(LOG_CRIT);
			  return NULL;
		  }
#endif

	  for (node = listhead(wq->items); node; node = listnextnode (node)){
		item=listgetdata(node);
		retrn=item->data;
			if (prefix_same(&rn->p,&retrn->p)&&CHECK_FLAG(((struct rib *)rn->info)->rn_status, RIB_ROUTE_HEAD))
				item_node = node;
	  } 	
	  return item_node;
}

void
work_queue_add_head(struct work_queue *wq, void *data)
{

#if 0
	struct work_queue_item *item;
	struct work_queue_item *retitem;
	  assert (wq);
	  
	  retitem=work_queue_item_lookup(wq,data);
	  if(retitem)
		retitem->ran=wq->spec.max_retries+1;
	//	zlog_debug("rn %p do not actually work this time.",data);
	  if (!(item = work_queue_item_new (wq)))
	  {
			zlog_err ("%s: unable to get new queue item", __func__);
			return;
	  }
	  item->data = data;
	  listnode_add_after(wq->items,NULL,item);
			
	  work_queue_schedule (wq, wq->spec.hold);
	 return;
#else
	struct work_queue_item *item=NULL;	
	struct listnode *node=NULL;
#if 0
	  assert (wq);
#else
	if(!wq)
	{
	
		zlog(NULL, LOG_CRIT, "line %u, function %s",
		   __LINE__,(__func__ ? __func__ : "?"));
		zlog_backtrace(LOG_CRIT);
		return;
	}
#endif
	  
	node=work_queue_item_lookup(wq,data);
	/*if(!node)
	{
		zlog_err ("%s: can't get prev item", __func__);
		return;
	}*/
	
	if(node)/*gjd:( change 2011-11-16 )get the same work queue , if it has same , so return .*/
	{
		/*zlog_err ("%s: can't get prev item", __func__);*/
		return;
	}
	
	if (!(item = extern_work_queue_item_new (wq)))
	  {
		zlog_err ("%s: unable to get new queue item", __func__);
		return;
	  }
	item->data = data;
	listnode_add_after(wq->items,node,item);
	extern_work_queue_schedule (wq, wq->spec.hold);
	return;

	
#endif
}


