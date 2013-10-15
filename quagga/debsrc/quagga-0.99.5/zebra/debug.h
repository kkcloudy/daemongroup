/*
 * Zebra debug related function
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#ifndef _ZEBRA_DEBUG_H
#define _ZEBRA_DEBUG_H

/* Debug flags. */
#define ZEBRA_DEBUG_EVENT   0x01

#define ZEBRA_DEBUG_PACKET  0x01
#define ZEBRA_DEBUG_SEND    0x20
#define ZEBRA_DEBUG_RECV    0x40
#define ZEBRA_DEBUG_DETAIL  0x80

#define ZEBRA_DEBUG_KERNEL  0x01

#define ZEBRA_DEBUG_RIB     0x01
#define ZEBRA_DEBUG_RIB_Q   0x02

/*gjd : add for distribute system*/
#define RTM_DEBUG_DISTRIBUTE_SYSTEM 0x01
#define tipc_client_debug   (rtm_debug_distribute_system & RTM_DEBUG_DISTRIBUTE_SYSTEM)
#define tipc_server_debug   (rtm_debug_distribute_system & RTM_DEBUG_DISTRIBUTE_SYSTEM)

#define RTM_DEBUG_IF_FLOW_STATS   0x01
#define rtm_debug_if_flow   (rtm_debug_if_flow_stats & RTM_DEBUG_IF_FLOW_STATS)
#define RTM_DEBUG_IPV6_RTADV    0x01
#define RTM_DEBUG_RTADV  (rtm_debug_rtadv &RTM_DEBUG_IPV6_RTADV)

/* Debug related macro. */
#define IS_ZEBRA_DEBUG_EVENT  (zebra_debug_event & ZEBRA_DEBUG_EVENT)

#define IS_ZEBRA_DEBUG_PACKET (zebra_debug_packet & ZEBRA_DEBUG_PACKET)
#define IS_ZEBRA_DEBUG_SEND   (zebra_debug_packet & ZEBRA_DEBUG_SEND)
#define IS_ZEBRA_DEBUG_RECV   (zebra_debug_packet & ZEBRA_DEBUG_RECV)
#define IS_ZEBRA_DEBUG_DETAIL (zebra_debug_packet & ZEBRA_DEBUG_DETAIL)

#define IS_ZEBRA_DEBUG_KERNEL (zebra_debug_kernel & ZEBRA_DEBUG_KERNEL)

#define IS_ZEBRA_DEBUG_RIB  (zebra_debug_rib & ZEBRA_DEBUG_RIB)
#define IS_ZEBRA_DEBUG_RIB_Q  (zebra_debug_rib & ZEBRA_DEBUG_RIB_Q)

extern unsigned long zebra_debug_event;
extern unsigned long zebra_debug_packet;
extern unsigned long zebra_debug_kernel;
extern unsigned long zebra_debug_rib;

extern unsigned long rtm_debug_distribute_system;
extern unsigned long rtm_debug_if_flow_stats;
extern unsigned long rtm_debug_rtadv;
extern void zebra_debug_init (void);

#endif /* _ZEBRA_DEBUG_H */
