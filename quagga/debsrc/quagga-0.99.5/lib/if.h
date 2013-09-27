/* Interface related header.
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _ZEBRA_IF_H
#define _ZEBRA_IF_H

#include "linklist.h"

/*
  Interface name length.

   Linux define value in /usr/include/linux/if.h.
   #define IFNAMSIZ        16

   FreeBSD define value in /usr/include/net/if.h.
   #define IFNAMSIZ        16
*/

#define INTERFACE_NAMSIZ      20
#define INTERFACE_HWADDR_MAX  20

#define ETH_MAX           (strlen("eth10-8"))
#define ETH_SUB_MIN       (strlen("eth1-1.1"))
#define ETH_DEBUG_LEN	  (strlen("eth3")) /*for AX71_2X12G12S and AX81_2X12G12S : eht3 (is a debug inteface).*/

/*gujd: 2012-06-06, am 11:03. Change the ve sub (like ve1.100) to support multi-core (like ve01f1.100).*/
#if 0
#define VE_INTERFACE_NAME_MAX   	(strlen("ve10"))
#define VE_SUB_INTERFACE_NAME_MIN   (strlen("ve1.1"))
#else
#define VE_INTERFACE_NAME_MAX		(strlen("ve06f8")) /*6 byte*/
#define VE_SUB_INTERFACE_NAME_MIN	(strlen("ve09f1.1")) /*>= 8 byte*/
#endif

#define OTHER_INTERFACE				0
#define ETH_INTERFACE          		1/*ethx-x*/
#define ETH_SUB_INTERFACE	   		2/*ethx-x.xx or ethx-x.xx.xx*/
#define VE_INTERFACE 		   		3/*vex*/
#define VE_SUB_INTERFACE 		    4/*vex.xx*/
#define OBC_INTERFACE 		  		5/*obc0*/
#define ETH_DEBUG_INTERFACE 		6/*eth3*/
#define MNG_INTERFACE               7/*mngx-x*/
#define VLAN_INTERFACE				8/*vlanxxx*/
#define PIMREG_INTERFACE			9/*pimreg*/
#define SIT0_INTERFACE				10/*sit0*/
#define LOOP_INTERFACE				11/*lo*/
#define GRE_INTERFACE			12/*gre*/
#define PPPOE_INTERFACE				13/*pppoe*/
#define OCT_INTERFACE				14/*oct*/

#define DISABLE_RADIO_INTERFACE		100
#define ENABLE_RADIO_INTERFACE		200

#define LOCAL_BOARD_INTERFACE			1000
#define OTHER_BOARD_INTERFACE			2000

#define INTERFACE_RESEND_ADD (1 << 0)

#ifdef HAVE_PROC_NET_DEV
struct if_stats
{
  unsigned long long rx_packets;   /* total packets received       */
  unsigned long long tx_packets;   /* total packets transmitted    */
  unsigned long long rx_bytes;     /* total bytes received         */
  unsigned long long tx_bytes;     /* total bytes transmitted      */
  unsigned long long rx_errors;    /* bad packets received         */
  unsigned long long tx_errors;    /* packet transmit problems     */
  unsigned long long rx_dropped;   /* no space in linux buffers    */
  unsigned long long tx_dropped;   /* no space available in linux  */
  unsigned long long rx_multicast; /* multicast packets received   */
  unsigned long long rx_compressed;
  unsigned long long tx_compressed;
  unsigned long long collisions;

  /* detailed rx_errors: */
  unsigned long long rx_length_errors;
  unsigned long long rx_over_errors;       /* receiver ring buff overflow  */
  unsigned long long rx_crc_errors;        /* recved pkt with crc error    */
  unsigned long long rx_frame_errors;      /* recv'd frame alignment error */
  unsigned long long rx_fifo_errors;       /* recv'r fifo overrun          */
  unsigned long long rx_missed_errors;     /* receiver missed packet     */
  /* detailed tx_errors */
  unsigned long long tx_aborted_errors;
  unsigned long long tx_carrier_errors;
  unsigned long long tx_fifo_errors;
  unsigned long long tx_heartbeat_errors;
  unsigned long long tx_window_errors;
};
#endif /* HAVE_PROC_NET_DEV */

/* Interface structure */
struct interface 
{
  /* Interface name.  This should probably never be changed after the
     interface is created, because the configuration info for this interface
     is associated with this structure.  For that reason, the interface
     should also never be deleted (to avoid losing configuration info).
     To delete, just set ifindex to IFINDEX_INTERNAL to indicate that the
     interface does not exist in the kernel.
   */
  char name[INTERFACE_NAMSIZ + 1];

  /* Interface index (should be IFINDEX_INTERNAL for non-kernel or
     deleted interfaces). */
  unsigned int ifindex;
#define IFINDEX_INTERNAL	0

  /* Zebra internal interface status */
  unsigned char status;
#define ZEBRA_INTERFACE_ACTIVE     (1 << 0)
#define ZEBRA_INTERFACE_SUB        (1 << 1)
#define ZEBRA_INTERFACE_LINKDETECTION (1 << 2)
  
  /* Interface flags. */
  uint64_t flags;

  /* Interface metric */
  int metric;

  /* Interface MTU. */
  unsigned int mtu;    /* IPv4 MTU */
  unsigned int mtu6;   /* IPv6 MTU - probably, but not neccessarily same as mtu */

  /* Hardware address. */
#ifdef HAVE_SOCKADDR_DL
  struct sockaddr_dl sdl;
#else
  unsigned short hw_type;
  unsigned char hw_addr[INTERFACE_HWADDR_MAX];
  int hw_addr_len;
#endif /* HAVE_SOCKADDR_DL */

  /* interface bandwidth, kbits */
  unsigned int bandwidth;
  
  /* description of the interface. */
  char *desc;			

  /* Distribute list. */
  void *distribute_in;
  void *distribute_out;

  /* Connected address list. */
  struct list *connected;

  /* Daemon specific interface data pointer. */
  void *info;

  /* Statistics fileds. */
#ifdef HAVE_PROC_NET_DEV
  struct if_stats stats;
#endif /* HAVE_PROC_NET_DEV */  
#ifdef HAVE_NET_RT_IFLIST
  struct if_data stats;
#endif /* HAVE_NET_RT_IFLIST */
	time_t time;

	unsigned int if_up_cnt;
	unsigned int if_down_cnt;
/*wangchao add ,used in tipc  */
unsigned int if_types;
#define VIRTUAL_INTERFACE			 1
#define RPA_INTERFACE			 	 2
#define REAL_INTERFACE			     3	

/*gjd : add for register rpa between different boards*/
	int slot;
	int devnum;	
/*gjd: deal with add for add/del ip(v4/v6)address*/	
	unsigned char ip_flags;
	#define IPV4_ADDR_ADD 					(1<<0)
	#define IPV4_ADDR_DEL					(1<<1)
	#define IPV6_ADDR_ADD					(1<<2)
	#define IPV6_ADDR_DEL					(1<<3)
	#define IPV4_ADDR_DISTRIBUTE_ADD		(1<<4)
	#define IPV4_ADDR_DISTRIBUTE_DEL		(1<<5)
	#define IPV6_ADDR_DISTRIBUTE_ADD		(1<<6)
	#define IPV6_ADDR_DISTRIBUTE_DEL		(1<<7)

unsigned char linkdetection_flags;
	#define SET_LINKDETECTION			(1<<0)

/*add for  local or global interface of Distribute Systm .*/
unsigned char if_scope;
	#define INTERFACE_LOCAL			(1<<0)
	#define INTERFACE_GLOBAL		(1<<1)

unsigned char desc_scope;
	#define INTERFACE_DESCIPTION_LOCAL			(1<<0)
//	#define INTERFACE_DESCIPTION_GLOBAL			(1<<1)/*default is gobal, sync, every board is same*/
unsigned char pass_flag;
	#define SMUX_CHECK_OVER   (1<<0)/*gujd : 2013-05-09,pm 5:09. Add for smux.*/
unsigned char uplink_flag;
	#define INTERFACE_SET_UPLINK	(1<<0)/*gujd: 2013-06-03, am 10:55. Add for uplink interface.*/
	
};

/* Connected address structure. */
struct connected
{
  /* Attached interface. */
  struct interface *ifp;

  /* Flags for configuration. */
  unsigned char conf;
#define ZEBRA_IFC_REAL         (1 << 0)
#define ZEBRA_IFC_CONFIGURED   (1 << 1)
  /*
     The ZEBRA_IFC_REAL flag should be set if and only if this address
     exists in the kernel.
     The ZEBRA_IFC_CONFIGURED flag should be set if and only if this address
     was configured by the user from inside quagga.
   */

  /* Flags for connected address. */
  unsigned char flags;
#define ZEBRA_IFA_SECONDARY   (1 << 0)

  /* Address of connected network. */
  struct prefix *address;
  struct prefix *destination; /* broadcast or peer address; may be NULL */

  /* Label for Linux 2.2.X and upper. */
  char *label;
  
  unsigned char ip_config;
  #define RTMD_RESTART_IP_CONFIG   (1<<0)
  int ipv4_set_fail;
  int ipv6_set_fail;
  
  int ipv6_config;
  #define RTMD_IPV6_ADDR_CONFIG	 (1<<0)
};

/* Given an IPV4 struct connected, this macro determines whether a /32
   peer address has been supplied (i.e. there is no subnet assigned) */
#define CONNECTED_DEST_HOST(C) \
	((C)->destination && ((C)->address->prefixlen == IPV4_MAX_PREFIXLEN))

/* Given an IPV4 struct connected, this macro determins whether it is
   a point-to-point link with a /32 peer address (i.e. there
   is no dedicated subnet for the PtP link) */
#define CONNECTED_POINTOPOINT_HOST(C) \
	(((C)->ifp->flags & IFF_POINTOPOINT) && CONNECTED_DEST_HOST(C))

/* Interface hook sort. */
#define IF_NEW_HOOK   0
#define IF_DELETE_HOOK 1

/* There are some interface flags which are only supported by some
   operating system. */

#ifndef IFF_NOTRAILERS
#define IFF_NOTRAILERS 0x0
#endif /* IFF_NOTRAILERS */
#ifndef IFF_OACTIVE
#define IFF_OACTIVE 0x0
#endif /* IFF_OACTIVE */
#ifndef IFF_SIMPLEX
#define IFF_SIMPLEX 0x0
#endif /* IFF_SIMPLEX */
#ifndef IFF_LINK0
#define IFF_LINK0 0x0
#endif /* IFF_LINK0 */
#ifndef IFF_LINK1
#define IFF_LINK1 0x0
#endif /* IFF_LINK1 */
#ifndef IFF_LINK2
#define IFF_LINK2 0x0
#endif /* IFF_LINK2 */
#ifndef IFF_NOXMIT
#define IFF_NOXMIT 0x0
#endif /* IFF_NOXMIT */
#ifndef IFF_NORTEXCH
#define IFF_NORTEXCH 0x0
#endif /* IFF_NORTEXCH */
#ifndef IFF_IPV4
#define IFF_IPV4 0x0
#endif /* IFF_IPV4 */
#ifndef IFF_IPV6
#define IFF_IPV6 0x0
#endif /* IFF_IPV6 */
#ifndef IFF_VIRTUAL
#define IFF_VIRTUAL 0x0
#endif /* IFF_VIRTUAL */

/* Prototypes. */
extern int if_cmp_func (struct interface *, struct interface *);
extern struct interface *if_create (const char *name, int namelen);
extern struct interface *if_lookup_by_index (unsigned int);
extern struct interface *if_lookup_exact_address (struct in_addr);
extern struct interface *if_lookup_address (struct in_addr);

/* These 2 functions are to be used when the ifname argument is terminated
   by a '\0' character: */
extern struct interface *if_lookup_by_name (const char *ifname);
extern struct interface *if_get_by_name (const char *ifname);

/* For these 2 functions, the namelen argument should be the precise length
   of the ifname string (not counting any optional trailing '\0' character).
   In most cases, strnlen should be used to calculate the namelen value. */
extern struct interface *if_lookup_by_name_len(const char *ifname,
					       size_t namelen);
extern struct interface *if_get_by_name_len(const char *ifname, size_t namelen);


/* Delete the interface, but do not free the structure, and leave it in the
   interface list.  It is often advisable to leave the pseudo interface 
   structure because there may be configuration information attached. */
extern void if_delete_retain (struct interface *);

/* Delete and free the interface structure: calls if_delete_retain and then
   deletes it from the interface list and frees the structure. */
extern void if_delete (struct interface *);

extern int if_is_up (struct interface *);
extern int if_is_running (struct interface *);
extern int if_is_operative (struct interface *);
extern int if_is_loopback (struct interface *);
extern int if_is_broadcast (struct interface *);
extern int if_is_pointopoint (struct interface *);
extern int if_is_multicast (struct interface *);
extern void if_add_hook (int, int (*)(struct interface *));
extern void if_init (void);
extern void if_dump_all (void);
extern const char *if_flag_dump(unsigned long);

/* Please use ifindex2ifname instead of if_indextoname where possible;
   ifindex2ifname uses internal interface info, whereas if_indextoname must
   make a system call. */
extern const char *ifindex2ifname (unsigned int);

/* Please use ifname2ifindex instead of if_nametoindex where possible;
   ifname2ifindex uses internal interface info, whereas if_nametoindex must
   make a system call. */
extern unsigned int ifname2ifindex(const char *ifname);

/* Connected address functions. */
extern struct connected *connected_new (void);
extern void connected_free (struct connected *);
extern void connected_add (struct interface *, struct connected *);
extern struct connected  *connected_add_by_prefix (struct interface *,
                                            struct prefix *,
                                            struct prefix *);
extern struct connected  *connected_delete_by_prefix (struct interface *, 
                                               struct prefix *);
extern struct connected  *connected_lookup_address (struct interface *, 
                                             struct in_addr);

/*gujd: 2012-02-08: pm 5:20 . In order to decrease the warning when make img . For declaration  func  in .h file .*/
extern struct interface *if_get_by_vty_index(void * vtyindex);
extern char *ifindex_to_ifname(unsigned int index);

#ifndef HAVE_IF_NAMETOINDEX
extern unsigned int if_nametoindex (const char *);
#endif
#ifndef HAVE_IF_INDEXTONAME
extern char *if_indextoname (unsigned int, char *);
#endif

/* Exported variables. */
extern struct list *iflist;
extern struct cmd_element interface_desc_cmd;
extern struct cmd_element interface_desc_local_cmd;
extern struct cmd_element no_interface_desc_cmd;
extern struct cmd_element interface_cmd;
extern struct cmd_element no_interface_cmd;
extern struct cmd_element interface_pseudo_cmd;
extern struct cmd_element no_interface_pseudo_cmd;
extern struct cmd_element show_address_cmd;

#define		WID_IF_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
void wid_str2lower(char **str);
#define INTERFACE_DESCRIPTON_SET    1
#define INTERFACE_DESCRIPTON_UNSET  0


#endif /* _ZEBRA_IF_H */
