/* FIB SNMP.
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#ifdef HAVE_SNMP
#ifdef HAVE_NETSNMP
#include <net-snmp/net-snmp-config.h>
#endif
#include <asn1.h>
#include <snmp.h>
#include <snmp_impl.h>
#include "if.h"
#include "log.h"
#include "prefix.h"
#include "command.h"
#include "smux.h"
#include "table.h"

#include "zebra/rib.h"
#include "zebra/zserv.h"

#define IPFWMIB 1,3,6,1,2,1,4,24
#define IFMIB 1,3,6,1,4,1,31656,6,1,2,4





/* ipForwardTable */
#define IPFORWARDDEST                         1
#define IPFORWARDMASK                         2
#define IPFORWARDPOLICY                       3
#define IPFORWARDNEXTHOP                      4
#define IPFORWARDIFINDEX                      5
#define IPFORWARDTYPE                         6
#define IPFORWARDPROTO                        7
#define IPFORWARDAGE                          8
#define IPFORWARDINFO                         9
#define IPFORWARDNEXTHOPAS                   10
#define IPFORWARDMETRIC1                     11
#define IPFORWARDMETRIC2                     12
#define IPFORWARDMETRIC3                     13
#define IPFORWARDMETRIC4                     14
#define IPFORWARDMETRIC5                     15

/* ipCidrRouteTable */
#define IPCIDRROUTEDEST                       1
#define IPCIDRROUTEMASK                       2
#define IPCIDRROUTETOS                        3
#define IPCIDRROUTENEXTHOP                    4
#define IPCIDRROUTEIFINDEX                    5
#define IPCIDRROUTETYPE                       6
#define IPCIDRROUTEPROTO                      7
#define IPCIDRROUTEAGE                        8
#define IPCIDRROUTEINFO                       9
#define IPCIDRROUTENEXTHOPAS                 10
#define IPCIDRROUTEMETRIC1                   11
#define IPCIDRROUTEMETRIC2                   12
#define IPCIDRROUTEMETRIC3                   13
#define IPCIDRROUTEMETRIC4                   14
#define IPCIDRROUTEMETRIC5                   15
#define IPCIDRROUTESTATUS                    16

#define IFINDEX                         	1
#define IFDESCR                         	2
#define IFTYPE                         		3
#define IFMTU                       		4
#define IFSPEED                      		5
#define IFPHYADDRESS                      	6
#define IFADMINSTATUS                      	7
#define IFOPERSTATUS                       	8
#define IFLASTCHANGE                       	9
#define IFUPDOWNCOUNT												10



#define INTEGER32 ASN_INTEGER
#define GAUGE32 ASN_GAUGE
#define ENUMERATION ASN_INTEGER
#define ROWSTATUS ASN_INTEGER
#define IPADDRESS ASN_IPADDRESS
#define OBJECTIDENTIFIER ASN_OBJECT_ID
#define STRING	    ASN_OCTET_STR
#define TIMETICKS   ASN_TIMETICKS

extern struct zebra_t zebrad;

oid ipfw_oid [] = { IPFWMIB };
oid if_oid [] = { IFMIB };

/* Hook functions. */
u_char * ipFwNumber ();
u_char * ipFwTable ();
u_char * ipCidrNumber ();
u_char * ipCidrTable ();
u_char * if_mibs_get ();
u_char * if_num_mibs_get ();

struct variable zebra_variables[] = 
  {
    {0, GAUGE32, RONLY, ipFwNumber, 1, {1}},
    {IPFORWARDDEST, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 1}},
    {IPFORWARDMASK, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 2}},
    {IPFORWARDPOLICY, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 3}},
    {IPFORWARDNEXTHOP, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 4}},
    {IPFORWARDIFINDEX, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 5}},
    {IPFORWARDTYPE, ENUMERATION, RONLY, ipFwTable, 3, {2, 1, 6}},
    {IPFORWARDPROTO, ENUMERATION, RONLY, ipFwTable, 3, {2, 1, 7}},
    {IPFORWARDAGE, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 8}},
    {IPFORWARDINFO, OBJECTIDENTIFIER, RONLY, ipFwTable, 3, {2, 1, 9}},
    {IPFORWARDNEXTHOPAS, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 10}},
    {IPFORWARDMETRIC1, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 11}},
    {IPFORWARDMETRIC2, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 12}},
    {IPFORWARDMETRIC3, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 13}},
    {IPFORWARDMETRIC4, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 14}},
    {IPFORWARDMETRIC5, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 15}},
    {0, GAUGE32, RONLY, ipCidrNumber, 1, {3}},
    {IPCIDRROUTEDEST, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 1}},
    {IPCIDRROUTEMASK, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 2}},
    {IPCIDRROUTETOS, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 3}},
    {IPCIDRROUTENEXTHOP, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 4}},
    {IPCIDRROUTEIFINDEX, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 5}},
    {IPCIDRROUTETYPE, ENUMERATION, RONLY, ipCidrTable, 3, {4, 1, 6}},
    {IPCIDRROUTEPROTO, ENUMERATION, RONLY, ipCidrTable, 3, {4, 1, 7}},
    {IPCIDRROUTEAGE, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 8}},
    {IPCIDRROUTEINFO, OBJECTIDENTIFIER, RONLY, ipCidrTable, 3, {4, 1, 9}},
    {IPCIDRROUTENEXTHOPAS, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 10}},
    {IPCIDRROUTEMETRIC1, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 11}},
    {IPCIDRROUTEMETRIC2, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 12}},
    {IPCIDRROUTEMETRIC3, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 13}},
    {IPCIDRROUTEMETRIC4, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 14}},
    {IPCIDRROUTEMETRIC5, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 15}},
    {IPCIDRROUTESTATUS, ROWSTATUS, RONLY, ipCidrTable, 3, {4, 1, 16}}
  };

struct variable zebra_if_variables[] = 
{
    /*{0, GAUGE32, RONLY, if_num_mibs_get, 1, {3}},*/
    {IFINDEX, INTEGER32, RONLY, if_mibs_get, 3, {2, 1, 1}},
    {IFDESCR, STRING, RONLY, if_mibs_get, 3, {2, 1, 2}},
    {IFTYPE, INTEGER32, RONLY, if_mibs_get, 3, {2, 1, 3}},
    {IFMTU, INTEGER32, RONLY, if_mibs_get, 3, {2, 1, 4}},
    {IFSPEED, GAUGE32, RONLY, if_mibs_get, 3, {2, 1, 5}},
    {IFPHYADDRESS, STRING, RONLY, if_mibs_get, 3, {2, 1, 6}},
    {IFADMINSTATUS, INTEGER32, RWRITE, if_mibs_get, 3, {2, 1, 7}},
    {IFOPERSTATUS, INTEGER32, RONLY, if_mibs_get, 3, {2, 1, 8}},
    {IFLASTCHANGE, TIMETICKS, RONLY, if_mibs_get, 3, {2, 1, 9}},
		{IFUPDOWNCOUNT, INTEGER32, RONLY, if_mibs_get, 3, {2, 1, 10}},
};


u_char *
ipFwNumber (struct variable *v, oid objid[], size_t *objid_len,
	    int exact, size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;

  if (smux_header_generic(v, objid, objid_len, exact, val_len, write_method) == MATCH_FAILED)
    return NULL;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return NULL;

  /* Return number of routing entries. */
  result = 0;
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      result++;

  return (u_char *)&result;
}

u_char *
ipCidrNumber (struct variable *v, oid objid[], size_t *objid_len,
	      int exact, size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;

  if (smux_header_generic(v, objid, objid_len, exact, val_len, write_method) == MATCH_FAILED)
    return NULL;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return 0;

  /* Return number of routing entries. */
  result = 0;
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      result++;

  return (u_char *)&result;
}

int
in_addr_cmp(u_char *p1, u_char *p2)
{
  int i;

  for (i=0; i<4; i++)
    {
      if (*p1 < *p2)
        return -1;
      if (*p1 > *p2)
        return 1;
      p1++; p2++;
    }
  return 0;
}

int 
in_addr_add(u_char *p, int num)
{
  int i, ip0;

  ip0 = *p;
  p += 4;
  for (i = 3; 0 <= i; i--) {
    p--;
    if (*p + num > 255) {
      *p += num;
      num = 1;
    } else {
      *p += num;
      return 1;
    }
  }
  if (ip0 > *p) {
    /* ip + num > 0xffffffff */
    return 0;
  }
  
  return 1;
}

int proto_trans(int type)
{
  switch (type)
    {
    case ZEBRA_ROUTE_SYSTEM:
      return 1; /* other */
    case ZEBRA_ROUTE_KERNEL:
      return 1; /* other */
    case ZEBRA_ROUTE_CONNECT:
      return 2; /* local interface */
    case ZEBRA_ROUTE_STATIC:
      return 3; /* static route */
    case ZEBRA_ROUTE_RIP:
      return 8; /* rip */
    case ZEBRA_ROUTE_RIPNG:
      return 1; /* shouldn't happen */
    case ZEBRA_ROUTE_OSPF:
      return 13; /* ospf */
    case ZEBRA_ROUTE_OSPF6:
      return 1; /* shouldn't happen */
    case ZEBRA_ROUTE_BGP:
      return 14; /* bgp */
    default:
      return 1; /* other */
    }
}

void
check_replace(struct route_node *np2, struct rib *rib2, 
              struct route_node **np, struct rib **rib)
{
  int proto, proto2;

  if (!*np)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  if (in_addr_cmp(&(*np)->p.u.prefix, &np2->p.u.prefix) < 0)
    return;
  if (in_addr_cmp(&(*np)->p.u.prefix, &np2->p.u.prefix) > 0)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  proto = proto_trans((*rib)->type);
  proto2 = proto_trans(rib2->type);

  if (proto2 > proto)
    return;
  if (proto2 < proto)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  if (in_addr_cmp((u_char *)&(*rib)->nexthop->gate.ipv4, 
                  (u_char *)&rib2->nexthop->gate.ipv4) <= 0)
    return;

  *np = np2;
  *rib = rib2;
  return;
}

void
get_fwtable_route_node(struct variable *v, oid objid[], size_t *objid_len, 
		       int exact, struct route_node **np, struct rib **rib)
{
  struct in_addr dest;
  struct route_table *table;
  struct route_node *np2;
  struct rib *rib2;
  int proto;
  int policy;
  struct in_addr nexthop;
  u_char *pnt;
  int i;

  /* Init index variables */

  pnt = (u_char *) &dest;
  for (i = 0; i < 4; i++)
    *pnt++ = 0;

  pnt = (u_char *) &nexthop;
  for (i = 0; i < 4; i++)
    *pnt++ = 0;

  proto = 0;
  policy = 0;
 
  /* Init return variables */

  *np = NULL;
  *rib = NULL;

  /* Short circuit exact matches of wrong length */

  if (exact && (*objid_len != (unsigned) v->namelen + 10))
    return;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return;

  /* Get INDEX information out of OID.
   * ipForwardDest, ipForwardProto, ipForwardPolicy, ipForwardNextHop
   */

  if (*objid_len > v->namelen)
    oid2in_addr (objid + v->namelen, MIN(4, *objid_len - v->namelen), &dest);

  if (*objid_len > (unsigned) v->namelen + 4)
    proto = objid[v->namelen + 4];

  if (*objid_len > (unsigned) v->namelen + 5)
    policy = objid[v->namelen + 5];

  if (*objid_len > (unsigned) v->namelen + 6)
    oid2in_addr (objid + v->namelen + 6, MIN(4, *objid_len - v->namelen - 6),
		 &nexthop);

  /* Apply GETNEXT on not exact search */

  if (!exact && (*objid_len >= (unsigned) v->namelen + 10))
    {
      if (! in_addr_add((u_char *) &nexthop, 1)) 
        return;
    }

  /* For exact: search matching entry in rib table. */

  if (exact)
    {
      if (policy) /* Not supported (yet?) */
        return;
      for (*np = route_top (table); *np; *np = route_next (*np))
	{
	  if (!in_addr_cmp(&(*np)->p.u.prefix, (u_char *)&dest))
	    {
	      for (*rib = (*np)->info; *rib; *rib = (*rib)->next)
	        {
		  if (!in_addr_cmp((u_char *)&(*rib)->nexthop->gate.ipv4,
				   (u_char *)&nexthop))
		    if (proto == proto_trans((*rib)->type))
		      return;
		}
	    }
	}
      return;
    }

  /* Search next best entry */

  for (np2 = route_top (table); np2; np2 = route_next (np2))
    {

      /* Check destination first */
      if (in_addr_cmp(&np2->p.u.prefix, (u_char *)&dest) > 0)
        for (rib2 = np2->info; rib2; rib2 = rib2->next)
	  check_replace(np2, rib2, np, rib);

      if (in_addr_cmp(&np2->p.u.prefix, (u_char *)&dest) == 0)
        { /* have to look at each rib individually */
          for (rib2 = np2->info; rib2; rib2 = rib2->next)
	    {
	      int proto2, policy2;

	      proto2 = proto_trans(rib2->type);
	      policy2 = 0;

	      if ((policy < policy2)
		  || ((policy == policy2) && (proto < proto2))
		  || ((policy == policy2) && (proto == proto2)
		      && (in_addr_cmp((u_char *)&rib2->nexthop->gate.ipv4,
				      (u_char *) &nexthop) >= 0)
		      ))
		check_replace(np2, rib2, np, rib);
	    }
	}
    }

  if (!*rib)
    return;

  policy = 0;
  proto = proto_trans((*rib)->type);

  *objid_len = v->namelen + 10;
  pnt = (u_char *) &(*np)->p.u.prefix;
  for (i = 0; i < 4; i++)
    objid[v->namelen + i] = *pnt++;

  objid[v->namelen + 4] = proto;
  objid[v->namelen + 5] = policy;

  {
    struct nexthop *nexthop;

    nexthop = (*rib)->nexthop;
    if (nexthop)
      {
	pnt = (u_char *) &nexthop->gate.ipv4;
	for (i = 0; i < 4; i++)
	  objid[i + v->namelen + 6] = *pnt++;
      }
  }

  return;
}


int ifFwTableWR(int action,
							   u_char * var_val,
							   u_char var_val_type,
							   size_t var_val_len,
							   u_char * statP,
							   oid * name, size_t length,
							   struct variable *v);

u_char *
ipFwTable (struct variable *v, oid objid[], size_t *objid_len,
	   int exact, size_t *val_len, WriteMethod **write_method)
{
  struct route_node *np;
  struct rib *rib;
  static int result;
  static int resarr[2];
  static struct in_addr netmask;
  struct nexthop *nexthop;


	*write_method = ifFwTableWR;
  get_fwtable_route_node(v, objid, objid_len, exact, &np, &rib);
  if (!np)
    return NULL;

  nexthop = rib->nexthop;
  if (! nexthop)
    return NULL;

  switch (v->magic)
    {
    case IPFORWARDDEST:
      *val_len = 4;
      return &np->p.u.prefix;
      break;
    case IPFORWARDMASK:
      masklen2ip(np->p.prefixlen, &netmask);
      *val_len = 4;
      return (u_char *)&netmask;
      break;
    case IPFORWARDPOLICY:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDNEXTHOP:
      *val_len = 4;
      return (u_char *)&nexthop->gate.ipv4;
      break;
    case IPFORWARDIFINDEX:
      *val_len = sizeof(int);
      return (u_char *)&nexthop->ifindex;
      break;
    case IPFORWARDTYPE:
      if (nexthop->type == NEXTHOP_TYPE_IFINDEX
	  || nexthop->type == NEXTHOP_TYPE_IFNAME)
        result = 3;
      else
        result = 4;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDPROTO:
      result = proto_trans(rib->type);
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDAGE:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDINFO:
      resarr[0] = 0;
      resarr[1] = 0;
      *val_len  = 2 * sizeof(int);
      return (u_char *)resarr;
      break;
    case IPFORWARDNEXTHOPAS:
      result = -1;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDMETRIC1:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDMETRIC2:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDMETRIC3:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDMETRIC4:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    case IPFORWARDMETRIC5:
      result = 0;
      *val_len  = sizeof(int);
      return (u_char *)&result;
      break;
    default:
      return NULL;
      break;
    }  
  return NULL;
}


typedef struct _fwtable_info{
	struct in_addr prefix;
	u_char masklen;
	struct in_addr gw;
	unsigned int ifindex;
	unsigned int fwtype;
	unsigned int fwproto;
	unsigned int fwpolicy;
	u_char distance;
} fwtable_info;

extern int debug_smux;

fwtable_info *get_fwtable_route_info(struct variable *v, 
	oid objid[], size_t *objid_len,int exact)
{
  static fwtable_info* fw_info = NULL;
  char buf[BUFSIZ];

  if(debug_smux)
	  smux_oid_dump( "get_fwtable_route_info", objid, *objid_len );

  fw_info = malloc(sizeof(fwtable_info));
  if(fw_info)
  	memset(fw_info,0,sizeof(fwtable_info));
  else{
	zlog_warn("%s malloc return NULL",__func__);
	return NULL;
  }

 

  /* Short circuit exact matches of wrong length */

  if (exact && (*objid_len != (unsigned) v->namelen + 10))
	  return NULL;


  /* Get INDEX information out of OID.
   * ipForwardDest, ipForwardProto, ipForwardPolicy, ipForwardNextHop
   */

  if (*objid_len > v->namelen)
	oid2in_addr (objid + v->namelen, MIN(4, *objid_len - v->namelen), &fw_info->prefix);
	
  if(debug_smux)
	  zlog_debug("fw_info->prefix is %s/%d",inet_ntop (AF_INET, &fw_info->prefix, buf, BUFSIZ),&fw_info->masklen);	
  if (*objid_len > (unsigned) v->namelen + 4)
  {
	fw_info->fwproto= objid[v->namelen + 4];
	
	if(debug_smux)
		zlog_debug("fw_info->fwproto %d",fw_info->fwproto);  
	if(fw_info->fwproto!=3)
	{
		zlog_warn("oid ip fw proto != 3");
		free(fw_info);
		return NULL;
	}
  }

  if (*objid_len > (unsigned) v->namelen + 5)
  {
	  fw_info->fwpolicy= objid[v->namelen + 5];
	  if(fw_info->fwpolicy!=0)
	  {
	  
	  	zlog_warn("oid ip fw policy != 0");
		  free(fw_info);
		  return NULL;
	  }
	  if(debug_smux)
		  zlog_debug("fw_info->fwproto %d",fw_info->fwproto);  

  }

  if (*objid_len > (unsigned) v->namelen + 6)
	oid2in_addr (objid + v->namelen + 6, MIN(4, *objid_len - v->namelen - 6),
		 &fw_info->gw);

  /* Apply GETNEXT on not exact search */

  if (!exact && (*objid_len >= (unsigned) v->namelen + 10))
	{
	  if (! in_addr_add((u_char *) &(fw_info->gw), 1))
	  	{
	  	
	  	zlog_warn("oid ip fw policy != 0");
	  	free(fw_info);
		return NULL;
	  	}
	}
  if(debug_smux)
	  zlog_debug("fw_info->gw is %s",inet_ntop (AF_INET, &(fw_info->gw), buf, BUFSIZ));	

  /* For exact: search matching entry in rib table. */

  fw_info->distance = ZEBRA_STATIC_DISTANCE_DEFAULT;
  return fw_info;
}


int ifFwTableWR(int action,
							   u_char * var_val,
							   u_char var_val_type,
							   size_t var_val_len,
							   u_char * statP,
							   oid * name, size_t length,
							   struct variable *v)
{
  struct route_node *np;
  struct rib *rib;
  static int result;
  static int resarr[2];
  struct in_addr netmask;
  struct prefix p;
  struct nexthop *nexthop;
  int i,ret;
  fwtable_info* rt_info=  get_fwtable_route_info(v,name,&length,1);

  if(!rt_info)
  	return 1;

  if(v->magic != IPFORWARDMASK){
	zlog_warn("v->magic  is not IPFORWARDMASK");
  	return 1;
  }
#if 1
  switch (v->magic)
	{
	case IPFORWARDDEST:
	  break;
	case IPFORWARDMASK:
		
		if(debug_smux)
			zlog_debug("%s var_val=%x var_val_len=%d",__func__,*(var_val+2),var_val_len);
		memcpy( &netmask, var_val+2, var_val_len );
		netmask.s_addr = netmask.s_addr >> ((sizeof(int)-var_val_len)*8);
		rt_info->masklen = ip_masklen(netmask);
		if(rt_info->masklen == 0)
		  rt_info->masklen = 32;
		
		
		p.family = AF_INET;
		p.prefixlen = rt_info->masklen;
		memcpy(&(p.u.prefix4),&(rt_info->prefix),sizeof(rt_info->prefix));
		apply_mask (&p);
		ret = static_add_ipv4(&p,&rt_info->gw,NULL,0,rt_info->distance,0);
		if(ret == 1 )
			ret =0;
		free(rt_info);
	  break;
	case IPFORWARDPOLICY:
	  break;
	case IPFORWARDNEXTHOP:
	  break;
	case IPFORWARDIFINDEX:
	  break;
	case IPFORWARDTYPE:
	  break;
	case IPFORWARDPROTO:
	  break;
	case IPFORWARDAGE:
	  break;
	case IPFORWARDINFO:
	  break;
	case IPFORWARDNEXTHOPAS:
	  break;
	case IPFORWARDMETRIC1:
	  break;
	case IPFORWARDMETRIC2:
	  break;
	case IPFORWARDMETRIC3:
	  break;
	case IPFORWARDMETRIC4:
	  break;
	case IPFORWARDMETRIC5:
	  break;
	default:
	  break;
	}  
#endif
#if 0
	var_val +=2;
	zlog_debug("ifFwTableWR action = %d\n", action);

	zlog_debug("var_val_type = %d\n", var_val_type );
	zlog_debug("var_val_len = %d\n", var_val_len );
	switch( var_val_type )
	{
		case ASN_INTEGER:
			if( var_val_len > sizeof(int) )
			{
				zlog_debug ("ASN_INTEGER invalid val len!" );
				break;
			}
			int getint=0;
			memcpy( &getint, var_val, var_val_len );
		  	zlog_debug ("var_val = %d", getint>>((sizeof(int)-var_val_len)*8));
			break;
		case ASN_COUNTER:
		case ASN_GAUGE:
		case ASN_TIMETICKS:
		case ASN_UINTEGER:
			if( var_val_len > sizeof(unsigned int) )
			{
				zlog_debug ("invalid val len!" );
				break;
			}			
			unsigned int getuint=0;
			memcpy( &getuint, var_val, var_val_len );
		  	zlog_debug ("var_val = %d",  getuint>>((sizeof(int)-var_val_len)*8)  );			
			break;
		case ASN_IPADDRESS:
			if( var_val_len > sizeof(unsigned int) )
			{
				zlog_debug ("invalid val len!" );
				break;
			}				
			unsigned int getip = 0;
			memcpy( &getip, var_val, var_val_len );
		  	zlog_debug ("var_val = %x", getip );
			break;
		case ASN_OCTET_STR:
		  	zlog_debug ("var_val = %s", var_val );
			break;
		default:
			zlog_debug( "var_val= " );
			for( i=0; i<var_val_len; i++ )
			{
				zlog_debug("%x ", var_val[i] );
			}
			break;

	}

	smux_oid_dump( "ifFwTableWR", name, length );

#endif
	return ret;
}



u_char *
ipCidrTable (struct variable *v, oid objid[], size_t *objid_len,
	     int exact, size_t *val_len, WriteMethod **write_method)
{
  switch (v->magic)
    {
    case IPCIDRROUTEDEST:
      break;
    default:
      return NULL;
      break;
    }  
  return NULL;
}
#if 0
static int
_if_get_hwaddr (struct interface *ifp)
{
  int ret;
  struct ifreq ifreq;
  int i;

  strncpy (ifreq.ifr_name, ifp->name, IFNAMSIZ);
  ifreq.ifr_addr.sa_family = AF_INET;

  /* Fetch Hardware address if available. */
  ret = if_ioctl (SIOCGIFHWADDR, (caddr_t) &ifreq);
  if (ret < 0)
    ifp->hw_addr_len = 0;
  else
    {
      memcpy (ifp->hw_addr, ifreq.ifr_hwaddr.sa_data, 6);

      for (i = 0; i < 6; i++)
	if (ifp->hw_addr[i] != 0)
	  break;

      if (i == 6)
		ifp->hw_addr_len = 0;
      else
		ifp->hw_addr_len = 6;
    }
  return 0;
}
#endif

int if_mibs_set(int action,
							   u_char * var_val,
							   u_char var_val_type,
							   size_t var_val_len,
							   u_char * statP,
							   oid * name, size_t length,
							   struct variable *v)
{
	static int result;
	struct listnode *node;
	struct interface *ifp;
	unsigned int ifindex = 0;
	/*gujd : 2012-05-28, pm 2:49 . In order to decrease the warning when make img . For the variable not use , delete it. */
	/*char * tmpindex = &ifindex,* tmpoid=name;*/
	unsigned int i,if_num=0;
	int ret=1;
	struct timeval timer_now = {0};
	
	if(debug_smux)
		smux_oid_dump( "if_mibs_set", name, length );


	ifindex = name[length-1];
	for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
	{
		if(strncmp(ifp->name,"radio",5)==0 
			|| strncmp(ifp->name,"r",1)==0 
			|| strncmp(ifp->name,"pimreg",6)==0 
			|| strncmp(ifp->name,"sit0",4)==0
			|| strncmp(ifp->name,"ppp",3)==0)
			continue;
		 if (ifp->ifindex == ifindex)
			  break;
	}
	if(!ifp)
	  	return 1;

	switch (v->magic)
	{
		case IFADMINSTATUS:
			if(var_val[2] == 1)
			{
#if 1
				ret = if_set_flags (ifp, IFF_UP | IFF_RUNNING);
				if (ret < 0)
				{
					zlog_warn("(%s): if_set_flags failed!",__func__);
					return 1;
				}
				if_refresh (ifp);
				ret =0;
#else
				if_up(ifp);
#endif
			}
			else if(var_val[2] ==2)
			{
#if 1
				ret = if_unset_flags (ifp, IFF_UP);
				if (ret < 0)
				{
					zlog_warn("(%s): if_unset_flags failed!",__func__);
					return 1;
				}
				if_refresh (ifp);
				ret = 0;
#else
				if_down(ifp); 
#endif
			}
			else
				ret = 1;
		
			break;
	
		default:
			zlog_warn("%s Error v->magic = %d",__func__,v->magic);
			return ret;
	}

	return 0;
}

u_char *
if_mibs_get (struct variable *v, oid objid[], size_t *objid_len,
	      int exact, size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct listnode *node,*nnode;
  struct interface *ifp=NULL,*tmpifp = NULL;
  unsigned int ifindex =0,tIfindex = 0;
  /*gujd : 2012-05-28, pm 2:49 . In order to decrease the warning when make img . For the variable not use , delete it. */
  /*char * tmpindex = &ifindex,* tmpoid=objid;*/
  unsigned int i,ret=0;
  struct timeval timer_now = {0};

  if(debug_smux)
	  smux_oid_dump( "if_mibs_get", objid, *objid_len );

  *write_method = if_mibs_set;
  

#if 0
	for(i=0;i<4;i++)
	{
		*tmpindex++ =objid[v->namelen+i];
	}
#endif
	if(debug_smux)
	zlog_debug("*objid_len = %u\n",*objid_len);
	if( 4 == *objid_len )
	{
		ifindex = objid[*objid_len-1];
		if(debug_smux)
		zlog_debug("ifindex = %u\n",ifindex);

 		for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
    	{
			if (ifp->ifindex == ifindex)
			{
				break;
			}
			ifp = NULL;
 		}

		if( NULL == node )
		{
			return NULL;
		}
		
		if( exact )
		{
			if(strncmp(ifp->name,"radio",5)==0 
				|| strncmp(ifp->name,"r",1)==0 
				|| strncmp(ifp->name,"pimreg",6)==0 
				|| strncmp(ifp->name,"sit0",4)==0
				|| strncmp(ifp->name,"ppp",3)==0)
				return NULL;
		}
		else
		{
			for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
			{
				if(strncmp(ifp->name,"radio",5)==0 
					|| strncmp(ifp->name,"r",1)==0 
					|| strncmp(ifp->name,"pimreg",6)==0 
					|| strncmp(ifp->name,"sit0",4)==0
					|| strncmp(ifp->name,"ppp",3)==0)
					continue;

				if(ifp->ifindex>ifindex &&
					( 0 == tIfindex || ifp->ifindex<tIfindex ))
				{
					tIfindex = ifp->ifindex;
					tmpifp = ifp;
				}
			}
			ifp = tmpifp;
		}
	}
	else
	{
 		for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
 		{
			if( strncmp(ifp->name,"radio",5)!=0 
				&& strncmp(ifp->name,"r",1)!=0 
				&& strncmp(ifp->name,"pimreg",6)!=0
				&& strncmp(ifp->name,"sit0",4)!=0 
				&& strncmp(ifp->name,"ppp",3)!=0)
			{
				if(tIfindex == 0)
				{
					tIfindex = ifp->ifindex;
					tmpifp = ifp;
				}	
				else
				{
					if(tIfindex>ifp->ifindex)
					{
						tIfindex = ifp->ifindex;
						tmpifp = ifp;
					}
				}
			}
		}
		ifp = tmpifp;
	}
	
	if(debug_smux)
	{
		zlog_debug("v->namelen=%d\n", v->namelen );
		zlog_debug("ifp = %p\n", ifp );
	}
	if(!ifp)
	{
		if(debug_smux)
		zlog_debug("ifp = %p  return NULL\n", ifp );
		return NULL;
	}
	if(debug_smux)
	{
		zlog_debug("ifindex = %u name is %s \n",ifindex,ifp->name);

		zlog_debug("v->namelen = %d\n", v->namelen );
		zlog_debug("sizeof(oid) = %d\n", sizeof(oid) );
		zlog_debug("sizeof(objid[v->namelen]) = %d\n", sizeof(objid[v->namelen]));
	}
	objid[v->namelen] = ifp->ifindex;
	*objid_len = v->namelen+1;
	
	switch (v->magic)
	{
		case IFINDEX:
			
			zlog_debug( "IFTYPE ifp->ifindex = %d\n", ifp->ifindex );			
			result =ifp->ifindex;
			break;
		case IFDESCR:
			if(ifp->desc){
				*val_len = strlen(ifp->desc);
				return (u_char *)ifp->desc;
			}
			else
			{
				*val_len = strlen(ifp->name);
				return (u_char *)ifp->name;
			}
/*dele by gjd*/
#if 0			
		case IFTYPE:
			if(strcmp(ifp->name,"lo") == 0)
			{
				result = 24;				
			}
			else if(strncmp(ifp->name,"vlan",4) == 0)
			{
				result = 6;		
			}
			else if(strncmp(ifp->name,"eth",3) == 0)
			{
				result = 6;		
			}
			else
			{
				result = 1;
			}
			break;
#endif
/*add by gjd*/
		case IFTYPE:
			if(ifp->hw_type == ARPHRD_LOOPBACK) //lo
			{
				result =24;
			}
			
			else if(ifp->hw_type == ARPHRD_ETHER)//eth, ebr,vlan
			{
				result =6;
			}
			
			else					//other
			{
				result = 1;
			}
			break;
		case IFMTU:
			zlog_debug( "IFTYPE ifp->mtu = %d\n", ifp->mtu );			
			result = ifp->mtu;
			break;
/*change by gjd*/
		case IFSPEED:
			#if 0
			zlog_debug( "IFTYPE ifp->bandwidth = %d\n", ifp->bandwidth );			
				{
					if(ifp->hw_type == ARPHRD_LOOPBACK) //lo
						result = 0;
					break;
					if(ifp->hw_type == ARPHRD_ETHER) //eth,vlan.ebr
						result = ifp->bandwidth/1000;
					break;
					}
			#endif
			result = ifp->bandwidth/1000;
			break;
		case IFPHYADDRESS:
			//_if_get_hwaddr(ifp);
			//zlog_debug( "IFTYPE ifp->hw_addr_len = %d\n", ifp->hw_addr_len );			
			*val_len = ifp->hw_addr_len;
			return (u_char *)ifp->hw_addr;
		case IFADMINSTATUS:
			if(ifp->flags & IFF_UP)
				result = 1;
			else
				result = 2;
			break;
		
		case IFOPERSTATUS:
			if(ifp->flags & IFF_RUNNING)
				result = 1;
			else
				result = 2;
			break;

		case IFLASTCHANGE:
			gettimeofday(&timer_now,0);
			result = 100*(timer_now.tv_sec - ifp->time);
			break;
		case IFUPDOWNCOUNT:
			result = ifp->if_up_cnt + ifp->if_down_cnt;
			break;
		default:
			zlog_warn("%s Error v->magic = %d",__func__,v->magic);
			return NULL;
	}

	*val_len = sizeof(result);
	return (u_char*)&result;
}



u_char *
if_num_mibs_get (struct variable *v, oid objid[], size_t *objid_len,
	      int exact, size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct listnode *node;
  struct interface *ifp;
  unsigned int ifindex = 0;
  /*gujd : 2012-05-28, pm 2:49 . In order to decrease the warning when make img . For the variable not use , delete it. */
  #if 0
  char * tmpindex = &ifindex,* tmpoid=objid;
  #else
  char * tmpindex = &ifindex;
  #endif
  unsigned int i,if_num=0;
  
  if(debug_smux)
	  smux_oid_dump( "if_num_mibs_get", objid, *objid_len );


	for(i=0;i<4;i++)
	{
		*tmpindex++ = objid;
	}
  for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
    {
	  if(strncmp(ifp->name,"radio",5)==0)
		  continue;
		result++;
    }
  val_len = sizeof(result);
  return (u_char *)&result;
}

void
zebra_snmp_init ()
{
  FILE* fp = NULL;
  unsigned int enterprise_id = 0; 

  fp=fopen(ENTERPRISE_SNMP_OID_FILE,"r");
  if(fp != NULL)
  {
	if(EOF != fscanf(fp,"%d",&enterprise_id))
	{
		if(enterprise_id>0)
			if_oid[6] = enterprise_id;

	}
	fclose(fp);
	fp = NULL;
  }
  	
  smux_init (zebrad.master);
//  REGISTER_MIB("mibII/ipforward", zebra_variables, variable, ipfw_oid);
  REGISTER_MIB("mibII/dot11AcIfTable", zebra_if_variables, variable, if_oid);
}
#endif /* HAVE_SNMP */
