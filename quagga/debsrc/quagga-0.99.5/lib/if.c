
/* 
 * Interface functions.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 * 
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <zebra.h>

#include "linklist.h"
#include "vector.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sockunion.h"
#include "prefix.h"
#include "memory.h"
#include "table.h"
#include "buffer.h"
#include "str.h"
#include "log.h"

/* Master list of interfaces. */
struct list *iflist;

extern product_inf *product;
extern int if_get_index_by_ioctl(struct interface * ifp);
struct cmd_node wireless_interface_node = { WIRELESS_INTERFACE_NODE,  "",  0 };


/* One for each program.  This structure is needed to store hooks. */
struct if_master
{
  int (*if_new_hook) (struct interface *);
  int (*if_delete_hook) (struct interface *);
} if_master;

/* Compare interface names, returning an integer greater than, equal to, or
 * less than 0, (following the strcmp convention), according to the
 * relationship between ifp1 and ifp2.  Interface names consist of an
 * alphabetic prefix and a numeric suffix.  The primary sort key is
 * lexicographic by name, and then numeric by number.  No number sorts
 * before all numbers.  Examples: de0 < de1, de100 < fxp0 < xl0, devpty <
 * devpty0, de0 < del0
 */         
int
if_cmp_func (struct interface *ifp1, struct interface *ifp2)
{
  unsigned int l1, l2;
  long int x1, x2;
  char *p1, *p2;
  int res;

  p1 = ifp1->name;
  p2 = ifp2->name;

  while (*p1 && *p2) {
    /* look up to any number */
    l1 = strcspn(p1, "0123456789");
    l2 = strcspn(p2, "0123456789");

    /* name lengths are different -> compare names */
    if (l1 != l2)
      return (strcmp(p1, p2));

    /* Note that this relies on all numbers being less than all letters, so
     * that de0 < del0.
     */
    res = strncmp(p1, p2, l1);

    /* names are different -> compare them */
    if (res)
      return res;

    /* with identical name part, go to numeric part */
    p1 += l1;
    p2 += l1;

    if (!*p1) 
      return -1;
    if (!*p2) 
      return 1;

    x1 = strtol(p1, &p1, 10);
    x2 = strtol(p2, &p2, 10);

    /* let's compare numbers now */
    if (x1 < x2)
      return -1;
    if (x1 > x2)
      return 1;

    /* numbers were equal, lets do it again..
    (it happens with name like "eth123.456:789") */
  }
  if (*p1)
    return 1;
  if (*p2)
    return -1;
  return 0;
}

/* Create new interface structure. */
struct interface *
if_create (const char *name, int namelen)
{
  struct interface *ifp=NULL;
  struct timeval timer_now = {0};

#if 0  
	assert (name);
	assert (namelen <= INTERFACE_NAMSIZ); /* Need space for '\0' at end. */
#else
	if(!name)
	{
	  return NULL;
	}
	if(namelen > INTERFACE_NAMSIZ)
	{
	  return  NULL;
	}
#endif
  ifp = XCALLOC (MTYPE_IF, sizeof (struct interface));
  memset(ifp,0,sizeof (struct interface));
  ifp->ifindex = IFINDEX_INTERNAL;
  strncpy (ifp->name, name, namelen);
  ifp->name[namelen] = '\0';
  if (if_lookup_by_name(ifp->name) == NULL)
    listnode_add_sort (iflist, ifp);
  else
  {
	  zlog_err("if_create(%s): corruption detected -- interface with this "
		   "name exists already!", ifp->name);
	  return NULL;
  }
  ifp->connected = list_new ();
  ifp->connected->del = (void (*) (void *)) connected_free;

  if (if_master.if_new_hook)
    (*if_master.if_new_hook) (ifp);
  ifp->bandwidth = 1000*1000;/*KB    1G*/
//  ifp->hw_type = 6;/*ethernetCsmacd */
  gettimeofday(&timer_now,0);
  ifp->time = timer_now.tv_sec ;
	
	ifp->if_up_cnt = 0;
	ifp->if_down_cnt = 0;
	ifp->if_types = 0;
	ifp->slot = 0;
	ifp->devnum = 0;
	ifp->ip_flags = 0;
	ifp->linkdetection_flags = 0;
	ifp->if_scope = 0;
	ifp->desc_scope = 0;
	ifp->uplink_flag = 0;

  return ifp;
}

/* Delete interface structure. */
void
if_delete_retain (struct interface *ifp)
{
  if (if_master.if_delete_hook)
    (*if_master.if_delete_hook) (ifp);

  /* Free connected address list */
  list_delete (ifp->connected);
}

/* Delete and free interface structure. */
void
if_delete (struct interface *ifp)
{
  listnode_delete (iflist, ifp);

  if_delete_retain(ifp);

  XFREE (MTYPE_IF, ifp);
}

/* Add hook to interface master. */
void
if_add_hook (int type, int (*func)(struct interface *ifp))
{
  switch (type) {
  case IF_NEW_HOOK:
    if_master.if_new_hook = func;
    break;
  case IF_DELETE_HOOK:
    if_master.if_delete_hook = func;
    break;
  default:
    break;
  }
}

/* Interface existance check by index. */
struct interface *
if_lookup_by_index (unsigned int index)
{
  struct listnode *node;
  struct interface *ifp;

  for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
    {
      if (ifp->ifindex == index)
	return ifp;
    }
  return NULL;
}

const char *
ifindex2ifname (unsigned int index)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_index(index)) != NULL) ?
  	 ifp->name : "unknown";
}

/*gujd: 2012-02-08: pm 5:20 . In order to decrease the warning when make img .
Add func ifindex_to_ifname from ifindex2ifname , but different is return "const char *"  .*/
char *
ifindex_to_ifname (unsigned int index)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_index(index)) != NULL) ?
  	 ifp->name : "unknown";
}

unsigned int
ifname2ifindex (const char *name)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp->ifindex : 0;
}

/**add by gjd**/
unsigned int
ifname2ifindex_by_namelen(const char * name,size_t namelen)
{
	struct interface *ifp;
	
	return ((ifp = if_lookup_by_name_len(name,namelen)) != NULL) ? ifp->ifindex : 0;
}
/**2011-03-07: pm 4:00**/

/* Interface existance check by interface name. */
struct interface *
if_lookup_by_name (const char *name)
{
  struct listnode *node;
  struct interface *ifp;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      if (strcmp(name, ifp->name) == 0)
	return ifp;
    }
  return NULL;
}

struct interface *
if_lookup_by_name_len(const char *name, size_t namelen)
{
  struct listnode *node;
  struct interface *ifp;

  if (namelen > INTERFACE_NAMSIZ)
    return NULL;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      if (!memcmp(name, ifp->name, namelen) && (ifp->name[namelen] == '\0'))
	return ifp;
    }
  return NULL;
}

/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_exact_address (struct in_addr src)
{
  struct listnode *node;
  struct listnode *cnode;
  struct interface *ifp;
  struct prefix *p;
  struct connected *c;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
	{
	  p = c->address;

	  if (p && p->family == AF_INET)
	    {
	      if (IPV4_ADDR_SAME (&p->u.prefix4, &src))
		return ifp;
	    }	      
	}
    }
  return NULL;
}

/* Lookup interface by IPv4 address. */
struct interface *
if_lookup_address (struct in_addr src)
{
  struct listnode *node;
  struct prefix addr;
  int bestlen = 0;
  struct listnode *cnode;
  struct interface *ifp;
  struct prefix *p;
  struct connected *c;
  struct interface *match;

  addr.family = AF_INET;
  addr.u.prefix4 = src;
  addr.prefixlen = IPV4_MAX_BITLEN;

  match = NULL;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
	{
	  if (c->address && (c->address->family == AF_INET))
	    {
	      if (CONNECTED_POINTOPOINT_HOST(c))
		{
		 /* PTP  links are conventionally identified 
		    by the address of the far end - MAG */
		  if (IPV4_ADDR_SAME (&c->destination->u.prefix4, &src))
		    return ifp;
		}
	      else
		{
		  p = c->address;

		  if (prefix_match (p, &addr) && p->prefixlen > bestlen)
		    {
		      bestlen = p->prefixlen;
		      match = ifp;
		    }
		}
	    }
	}
    }
  return match;
}

/* Get interface by name if given name interface doesn't exist create
   one. */
struct interface *
if_get_by_name (const char *name)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp :
	 if_create(name, strlen(name));
}

struct interface *
if_get_by_name_len(const char *name, size_t namelen)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_name_len(name, namelen)) != NULL) ? ifp :
	 if_create(name, namelen);
}

struct interface *
if_get_by_vty_index(void* vtyindex)
{
  char* ifname=NULL;

  ifname=(char *) vtyindex;
  return if_lookup_by_name(ifname);
}

/* Does interface up ? */
int
if_is_up (struct interface *ifp)
{
  return ifp->flags & IFF_UP;
}

/* Is interface running? */
int
if_is_running (struct interface *ifp)
{
  return ifp->flags & IFF_RUNNING;
}

/* Is the interface operative, eg. either UP & RUNNING
   or UP & !ZEBRA_INTERFACE_LINK_DETECTION */
int
if_is_operative (struct interface *ifp)
{
  return ((ifp->flags & IFF_UP) &&
	  (ifp->flags & IFF_RUNNING || !CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION)));
}

/* Is this loopback interface ? */
int
if_is_loopback (struct interface *ifp)
{
  /* XXX: Do this better, eg what if IFF_WHATEVER means X on platform M
   * but Y on platform N?
   */
  return (ifp->flags & (IFF_LOOPBACK|IFF_NOXMIT|IFF_VIRTUAL));
}

/* Does this interface support broadcast ? */
int
if_is_broadcast (struct interface *ifp)
{
  return ifp->flags & IFF_BROADCAST;
}

/* Does this interface support broadcast ? */
int
if_is_pointopoint (struct interface *ifp)
{
  return ifp->flags & IFF_POINTOPOINT;
}

/* Does this interface support multicast ? */
int
if_is_multicast (struct interface *ifp)
{
  return ifp->flags & IFF_MULTICAST;
}

/* Printout flag information into log */
const char *
if_flag_dump (unsigned long flag)
{
  int separator = 0;
  static char logbuf[BUFSIZ];

#define IFF_OUT_LOG(X,STR) \
  if (flag & (X)) \
    { \
      if (separator) \
	strlcat (logbuf, ",", BUFSIZ); \
      else \
	separator = 1; \
      strlcat (logbuf, STR, BUFSIZ); \
    }

  strlcpy (logbuf, "<", BUFSIZ);
  IFF_OUT_LOG (IFF_UP, "UP");
  IFF_OUT_LOG (IFF_BROADCAST, "BROADCAST");
  IFF_OUT_LOG (IFF_DEBUG, "DEBUG");
  IFF_OUT_LOG (IFF_LOOPBACK, "LOOPBACK");
  IFF_OUT_LOG (IFF_POINTOPOINT, "POINTOPOINT");
  IFF_OUT_LOG (IFF_NOTRAILERS, "NOTRAILERS");
  IFF_OUT_LOG (IFF_RUNNING, "RUNNING");
  IFF_OUT_LOG (IFF_NOARP, "NOARP");
  IFF_OUT_LOG (IFF_PROMISC, "PROMISC");
  IFF_OUT_LOG (IFF_ALLMULTI, "ALLMULTI");
  IFF_OUT_LOG (IFF_OACTIVE, "OACTIVE");
  IFF_OUT_LOG (IFF_SIMPLEX, "SIMPLEX");
  IFF_OUT_LOG (IFF_LINK0, "LINK0");
  IFF_OUT_LOG (IFF_LINK1, "LINK1");
  IFF_OUT_LOG (IFF_LINK2, "LINK2");
  IFF_OUT_LOG (IFF_MULTICAST, "MULTICAST");
  IFF_OUT_LOG (IFF_NOXMIT, "NOXMIT");
  IFF_OUT_LOG (IFF_NORTEXCH, "NORTEXCH");
  IFF_OUT_LOG (IFF_VIRTUAL, "VIRTUAL");
  IFF_OUT_LOG (IFF_IPV4, "IPv4");
  IFF_OUT_LOG (IFF_IPV6, "IPv6");

  strlcat (logbuf, ">", BUFSIZ);

  return logbuf;
#undef IFF_OUT_LOG
}

/* For debugging */
static void
if_dump (struct interface *ifp)
{
  struct listnode *node;
  struct connected *c;

  zlog_info ("Interface %s index %d metric %d mtu %d "
#ifdef HAVE_IPV6
             "mtu6 %d "
#endif /* HAVE_IPV6 */
             "%s",
	     ifp->name, ifp->ifindex, ifp->metric, ifp->mtu, 
#ifdef HAVE_IPV6
	     ifp->mtu6,
#endif /* HAVE_IPV6 */
	     if_flag_dump (ifp->flags));
  
  for (ALL_LIST_ELEMENTS_RO (ifp->connected, node, c))
    ;
}

/* Interface printing for all interface. */
void
if_dump_all ()
{
  struct listnode *node;
  void *p;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, p))
    if_dump (p);
}

//parse ifname ,permit capital letters ,used by wid
void wid_str2lower(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<='Z')&&(ptr[i]>='A'))  
			ptr[i] = ptr[i]-'A'+'a';  
	}
	
	return;
}
static char ifname_vtyindex[INTERFACE_NAMSIZ+1];

DEFUN (interface_desc, 
       interface_desc_cmd,
       "description .LINE",
       "Interface specific description\n"
       "Characters describing this interface\n")
{
  struct interface *ifp;

  if (argc == 0)
    return CMD_SUCCESS;
#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
  UNSET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
#endif
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);
  ifp->desc = argv_concat(argv, argc, 0);

  return CMD_SUCCESS;
}

DEFUN (interface_desc_local, 
       interface_desc_local_cmd,
       "local description .LINE",
	   "The description scope in local board\n"
       "Interface specific description\n"
       "Characters describing this interface\n")
{
  struct interface *ifp;

  if (argc < 1)
    return CMD_SUCCESS;
#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
#endif
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);

  SET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
  
  ifp->desc = argv_concat(argv, argc, 0);

  return CMD_SUCCESS;
}

DEFUN (no_interface_desc, 
       no_interface_desc_cmd,
       "no description",
       NO_STR
       "Interface specific description\n")
{
  struct interface *ifp;

#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
#endif
  UNSET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);
  ifp->desc = NULL;

  return CMD_SUCCESS;
}

/* See also wrapper function zebra_interface() in zebra/interface.c */
DEFUN (interface,
       interface_cmd,
       "interface IFNAME",
       "Select an interface to configure\n"
       "Interface's name\n")
{
  struct interface *ifp;
  size_t sl;

  if ((sl = strlen(argv[0])) > INTERFACE_NAMSIZ)
    {
      vty_out (vty, "%% Interface name %s is invalid: length exceeds "
		    "%d characters%s",
	       argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
      return CMD_WARNING;
    }

#if (defined _D_WCPSS_ || defined _D_CC_)	
 //zhanglei add

  //sz 20090316 add 
    char *ifmode = (char *)malloc(sizeof(char)*64);
	if(ifmode == NULL)
	{
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}
	char *ifmode1 = (char *)malloc(sizeof(char)*64);
	if(ifmode1 == NULL)
	{
		WID_IF_FREE_OBJECT(ifmode);	
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}	
	char *ifmode2 = (char *)malloc(sizeof(char)*64);
	if(ifmode2 == NULL)
	{
		WID_IF_FREE_OBJECT(ifmode);
		
		/*CID 13510 (#1 of 1): Resource leak (RESOURCE_LEAK)
		8. leaked_storage: Variable "ifmode1" going out of scope leaks the storage it points to.*/
		WID_IF_FREE_OBJECT(ifmode1);/*So add free ifmod1.*/
		
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}
	memset(ifmode,0,64);
	memset(ifmode1,0,64);	
	memset(ifmode2,0,64);
	memcpy(ifmode,argv[0],4);
	memcpy(ifmode1,argv[0],5);	
	memcpy(ifmode2,argv[0],1);
	wid_str2lower(&ifmode);
	wid_str2lower(&ifmode1);
	wid_str2lower(&ifmode2);

	if ((!strncmp(ifmode,"wlan",4))||(!strncmp(ifmode1,"radio",5))||(!strncmp(ifmode2,"r",1)))
	{
		wid_str2lower(&argv[0]);
	}
	WID_IF_FREE_OBJECT(ifmode);
	WID_IF_FREE_OBJECT(ifmode1);
	WID_IF_FREE_OBJECT(ifmode2);
	
	//add end
#endif
  /*gujd : 2012-08-02, pm 6:17 . Add for ve sub from ve1.100 change to ve01f1.100 .*/
  if(strncmp(argv[0],"ve",2)==0)
  	{
  		
		char *name_str=NULL;
		int ret = 0;
		name_str = (char *)malloc(sizeof(char)*64);
		if(!name_str)
		{
			vty_out(vty,"malloc or memset failed\n");
			return CMD_WARNING;/*add*/
		}
		/*CID 11020 (#1 of 1): Dereference after null check (FORWARD_NULL)
		12. var_deref_model: Passing null pointer "name_str" to function "memset(void *, int, size_t)", which dereferences it.
		So add return above.*/
		memset(name_str,0,64);
  		ret = ve_sub_inteface_name_check(argv[0],name_str);
		if(ret < 0)/*err*/
		{
		 	vty_out(vty, "%s : line %d Interface name %s err .%s", __func__,__LINE__,argv[0], VTY_NEWLINE);
			/*CID 13512 (#1 of 1): Resource leak (RESOURCE_LEAK)
			17. leaked_storage: Variable "name_str" going out of scope leaks the storage it points to.
			So add free memory.*/
			free(name_str);/*add*/
			name_str = NULL;
			return CMD_WARNING;
		 }
		else if(ret == 1)
		{
		 /*	zlog_debug( "%s : line %d Interface name [%s] ,len[%d] .\n", __func__,__LINE__,argv[0],sl);*/
			ifp = if_get_by_name_len(argv[0], sl);
			vty->index = XSTRDUP (MTYPE_TMP, argv[0]);//
 			vty->node = INTERFACE_NODE;
			free(name_str);
			name_str = NULL;
		
			return CMD_SUCCESS;
		}
	
		else if(ret == 0)
		{
		 	/*vty_out(vty, "%s : line %d Interface name %s err .%s",  __func__,__LINE__,name_str, VTY_NEWLINE);*/
			 sl = strlen(name_str);
			 ifp = if_get_by_name_len(name_str, sl);
			 /*vty_out(vty, "%s : line %d Interface name %s err .%s",  __func__,__LINE__,name_str, VTY_NEWLINE);*/
		}
		else/*ret > 1(never)*/
		{
			 vty_out(vty, "%s : line %d Interface name %s err .%s",  __func__,__LINE__,name_str, VTY_NEWLINE);
		}
		
		/*zlog_debug( "%s : line %d Interface name %s .\n",	__func__,__LINE__,name_str);*/
		vty->index = XSTRDUP (MTYPE_TMP, name_str);
 		vty->node = INTERFACE_NODE;
		free(name_str);
		name_str = NULL;
		
		return CMD_SUCCESS;
		
  	}
  else
  	{
  		ifp = if_get_by_name_len(argv[0], sl);
  	}

  /**gjd : add for rpa**/
  if(product != NULL)/*DSP*/
  {
  	if((ifp->ifindex != IFINDEX_INTERNAL) && (ifp->if_types != VIRTUAL_INTERFACE)&&(ifp->if_types != RPA_INTERFACE))
  		ifp->if_types = REAL_INTERFACE;
	else if(ifp->ifindex == IFINDEX_INTERNAL)
		ifp->if_types = VIRTUAL_INTERFACE;
	/*
	else if(judge_ve_sub_interface(ifp->name)== VE_SUB_INTERFACE)
		ifp->if_types = REAL_INTERFACE;
		*/
  	}
  #if 0
  /*gjd: add for ve sub interface of Distribute Sytem .*/
  if((product != NULL) &&(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE))/*DSP*/
  	{
  		if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))/*because , every time done cmd (interface veX.xxx), the ve sub interface is global.*/
			UNSET_FLAG(ifp->if_scope, INTERFACE_LOCAL);
  	}
  #endif
#if 0
  vty->index = ifp;
#else
	vty->index = XSTRDUP (MTYPE_TMP, argv[0]);
/*
  strcpy(ifname_vtyindex,argv[0]);
  vty->index = (void *) ifname_vtyindex;
*/
#endif
  vty->node = INTERFACE_NODE;

  return CMD_SUCCESS;
}

DEFUN_NOSH (no_interface,
           no_interface_cmd,
           "no interface IFNAME",
           NO_STR
           "Delete a pseudo interface's configuration\n"
           "Interface's name\n")
{
  // deleting interface
  struct interface *ifp;

#if (defined _D_WCPSS_ || defined _D_CC_)	
  //zhanglei add
  //sz 20090316 add 
    char *ifmode = (char *)malloc(sizeof(char)*64);
	if(ifmode == NULL)
	{
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}
	char *ifmode1 = (char *)malloc(sizeof(char)*64);
	if(ifmode1 == NULL)
	{
		WID_IF_FREE_OBJECT(ifmode);	
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}	
	char *ifmode2 = (char *)malloc(sizeof(char)*64);
	if(ifmode2 == NULL)
	{
		WID_IF_FREE_OBJECT(ifmode);		
		WID_IF_FREE_OBJECT(ifmode1);	
		vty_out(vty,"malloc or memset failed\n");
		return CMD_WARNING;
	}
	memset(ifmode,0,64);
	memset(ifmode1,0,64);	
	memset(ifmode2,0,64);
	memcpy(ifmode,argv[0],4);
	memcpy(ifmode1,argv[0],5);	
	memcpy(ifmode2,argv[0],1);
	wid_str2lower(&ifmode);
	wid_str2lower(&ifmode1);
	wid_str2lower(&ifmode2);
	
	if ((!strncmp(ifmode,"wlan",4))||(!strncmp(ifmode1,"radio",5))||(!strncmp(ifmode2,"r",1)))
	{
		wid_str2lower(&argv[0]);
	}
	WID_IF_FREE_OBJECT(ifmode);
	WID_IF_FREE_OBJECT(ifmode1);	
	WID_IF_FREE_OBJECT(ifmode2);
	//add end
#endif

  ifp = if_lookup_by_name (argv[0]);

  if (ifp == NULL)
    {
      vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)) 
    {
      vty_out (vty, "%% Only inactive interfaces can be deleted%s",
	      VTY_NEWLINE);
      return CMD_WARNING;
    }
  #if 1
  if(product != NULL)
  	{
  		if(ifp->if_types == REAL_INTERFACE )
  		{
  			if(strncmp(ifp->name, "eth",3) != 0)/*not eth*/
				unregister_rpa_interface_table(ifp);
			else
				if(strlen(ifp->name) >= ETH_SUB_MIN)/*eth sub interface*/
					unregister_rpa_interface_table(ifp);
  			}
		
		if(ifp->if_types == RPA_INTERFACE)
			{
  			if(strncmp(ifp->name, "eth",3) != 0)/*not eth*/
				delete_rpa_interface(ifp);
			else
			  if(strlen(ifp->name) >= ETH_SUB_MIN)/*eth sub interface*/
				delete_rpa_interface(ifp);
  			}
  	}
#endif
  if_delete(ifp);

  return CMD_SUCCESS;
}

/* For debug purpose. */
DEFUN (show_address,
       show_address_cmd,
       "show address",
       SHOW_STR
       "address\n")
{
  struct listnode *node;
  struct listnode *node2;
  struct interface *ifp;
  struct connected *ifc;
  struct prefix *p;

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, node2, ifc))
	{
	  p = ifc->address;

	  if (p->family == AF_INET)
	    vty_out (vty, "%s/%d%s", inet_ntoa (p->u.prefix4), p->prefixlen,
		     VTY_NEWLINE);
	}
    }
  return CMD_SUCCESS;
}

/* Allocate connected structure. */
struct connected *
connected_new (void)
{
  struct connected *new = XMALLOC (MTYPE_CONNECTED, sizeof (struct connected));
  memset (new, 0, sizeof (struct connected));
  return new;
}

/* Free connected structure. */
void
connected_free (struct connected *connected)
{
  if (connected->address)
    prefix_free (connected->address);

  if (connected->destination)
    prefix_free (connected->destination);

  if (connected->label)
    XFREE (MTYPE_CONNECTED_LABEL, connected->label);

  XFREE (MTYPE_CONNECTED, connected);
}

/* Print if_addr structure. */
static void __attribute__ ((unused))
connected_log (struct connected *connected, char *str)
{
  struct prefix *p;
  struct interface *ifp;
  char logbuf[BUFSIZ];
  char buf[BUFSIZ];
  
  ifp = connected->ifp;
  p = connected->address;

  snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	    str, ifp->name, prefix_family_str (p),
	    inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
	    p->prefixlen);

  p = connected->destination;
  if (p)
    {
      strncat (logbuf, inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
	       BUFSIZ - strlen(logbuf));
    }
  zlog (NULL, LOG_INFO, logbuf);
}

/* If two connected address has same prefix return 1. */
int
connected_same_prefix (struct prefix *p1, struct prefix *p2)
{
  if (p1->family == p2->family)
    {
      if (p1->family == AF_INET &&
	  IPV4_ADDR_SAME (&p1->u.prefix4, &p2->u.prefix4))
	return 1;
#ifdef HAVE_IPV6
      if (p1->family == AF_INET6 &&
	  IPV6_ADDR_SAME (&p1->u.prefix6, &p2->u.prefix6))
	return 1;
#endif /* HAVE_IPV6 */
    }
  return 0;
}

struct connected *
connected_delete_by_prefix (struct interface *ifp, struct prefix *p)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;

  /* In case of same prefix come, replace it with new one. */
  for (node = listhead (ifp->connected); node; node = next)
    {
      ifc = listgetdata (node);
      next = node->next;

      if (connected_same_prefix (ifc->address, p))
	{
	  listnode_delete (ifp->connected, ifc);
	  return ifc;
	}
    }
  return NULL;
}

/* Find the IPv4 address on our side that will be used when packets
   are sent to dst. */
struct connected *
connected_lookup_address (struct interface *ifp, struct in_addr dst)
{
  struct prefix addr;
  struct listnode *cnode;
  struct prefix *p;
  struct connected *c;
  struct connected *match;

  addr.family = AF_INET;
  addr.u.prefix4 = dst;
  addr.prefixlen = IPV4_MAX_BITLEN;

  match = NULL;

  for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
    {
      if (c->address && (c->address->family == AF_INET))
        {
	  if (CONNECTED_POINTOPOINT_HOST(c))
	    {
		     /* PTP  links are conventionally identified 
			by the address of the far end - MAG */
	      if (IPV4_ADDR_SAME (&c->destination->u.prefix4, &dst))
		return c;
	    }
	  else
	    {
	      p = c->address;

	      if (prefix_match (p, &addr) &&
	      	  (!match || (p->prefixlen > match->address->prefixlen)))
		match = c;
	    }
        }
    }
  return match;
}

struct connected *
connected_add_by_prefix (struct interface *ifp, struct prefix *p, 
                         struct prefix *destination)
{
  struct connected *ifc;

  /* Allocate new connected address. */
  ifc = connected_new ();
  ifc->ifp = ifp;

  /* Fetch interface address */
  ifc->address = prefix_new();
  memcpy (ifc->address, p, sizeof(struct prefix));

  /* Fetch dest address */
  if (destination)
    {
      ifc->destination = prefix_new();
      memcpy (ifc->destination, destination, sizeof(struct prefix));
    }

  /* Add connected address to the interface. */
  listnode_add (ifp->connected, ifc);
  return ifc;
}

#ifndef HAVE_IF_NAMETOINDEX
unsigned int
if_nametoindex (const char *name)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_name_len(name, strnlen(name, IFNAMSIZ))) != NULL)
  	 ? ifp->ifindex : 0;
}
#endif

#ifndef HAVE_IF_INDEXTONAME
char *
if_indextoname (unsigned int ifindex, char *name)
{
  struct interface *ifp;

  if (!(ifp = if_lookup_by_index(ifindex)))
    return NULL;
  strncpy (name, ifp->name, IFNAMSIZ);
  return ifp->name;
}
#endif

#if 0 /* this route_table of struct connected's is unused
       * however, it would be good to use a route_table rather than
       * a list..
       */
/* Interface looking up by interface's address. */
/* Interface's IPv4 address reverse lookup table. */
struct route_table *ifaddr_ipv4_table;
/* struct route_table *ifaddr_ipv6_table; */

static void
ifaddr_ipv4_add (struct in_addr *ifaddr, struct interface *ifp)
{
  struct route_node *rn;
  struct prefix_ipv4 p;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = *ifaddr;

  rn = route_node_get (ifaddr_ipv4_table, (struct prefix *) &p);
  if (rn)
    {
      route_unlock_node (rn);
      zlog_info ("ifaddr_ipv4_add(): address %s is already added",
		 inet_ntoa (*ifaddr));
      return;
    }
  rn->info = ifp;
}

static void
ifaddr_ipv4_delete (struct in_addr *ifaddr, struct interface *ifp)
{
  struct route_node *rn;
  struct prefix_ipv4 p;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = *ifaddr;

  rn = route_node_lookup (ifaddr_ipv4_table, (struct prefix *) &p);
  if (! rn)
    {
      zlog_info ("ifaddr_ipv4_delete(): can't find address %s",
		 inet_ntoa (*ifaddr));
      return;
    }
  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);
}

/* Lookup interface by interface's IP address or interface index. */
static struct interface *
ifaddr_ipv4_lookup (struct in_addr *addr, unsigned int ifindex)
{
  struct prefix_ipv4 p;
  struct route_node *rn;
  struct interface *ifp;

  if (addr)
    {
      p.family = AF_INET;
      p.prefixlen = IPV4_MAX_PREFIXLEN;
      p.prefix = *addr;

      rn = route_node_lookup (ifaddr_ipv4_table, (struct prefix *) &p);
      if (! rn)
	return NULL;
      
      ifp = rn->info;
      route_unlock_node (rn);
      return ifp;
    }
  else
    return if_lookup_by_index(ifindex);
}
#endif /* ifaddr_ipv4_table */

/* Initialize interface list. */
void
if_init (void)
{
  iflist = list_new ();
#if 0
  ifaddr_ipv4_table = route_table_init ();
#endif /* ifaddr_ipv4_table */

  if (iflist) {
    iflist->cmp = (int (*)(void *, void *))if_cmp_func;
    return;
  }

  memset (&if_master, 0, sizeof if_master);
}
