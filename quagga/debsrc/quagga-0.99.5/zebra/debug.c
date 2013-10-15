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

#include <zebra.h>
#include "command.h"
#include "debug.h"

/* For debug statement. */
unsigned long zebra_debug_event;
unsigned long zebra_debug_packet;
unsigned long zebra_debug_kernel;
unsigned long zebra_debug_rib;
unsigned long rtm_debug_distribute_system;
unsigned long rtm_debug_if_flow_stats;
unsigned long rtm_debug_rtadv;

DEFUN (show_debugging_zebra,
       show_debugging_zebra_cmd,
       "show debugging rtm",
       SHOW_STR
       "Rtm configuration\n"
       "Debugging information\n")
{
  vty_out (vty, "Rtm debugging status:%s", VTY_NEWLINE);

  if (IS_ZEBRA_DEBUG_EVENT)
    vty_out (vty, "  Rtm event debugging is on%s", VTY_NEWLINE);

  if (IS_ZEBRA_DEBUG_PACKET)
    {
      if (IS_ZEBRA_DEBUG_SEND && IS_ZEBRA_DEBUG_RECV)
	{
	  vty_out (vty, "  Rtm packet%s debugging is on%s",
		   IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		   VTY_NEWLINE);
	}
      else
	{
	  if (IS_ZEBRA_DEBUG_SEND)
	    vty_out (vty, "  Rtm packet send%s debugging is on%s",
		     IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  else
	    vty_out (vty, "  Rtm packet receive%s debugging is on%s",
		     IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	}
    }

  if (IS_ZEBRA_DEBUG_KERNEL)
    vty_out (vty, "  Rtm kernel debugging is on%s", VTY_NEWLINE);

  if (IS_ZEBRA_DEBUG_RIB)
    vty_out (vty, "  Rtm RIB debugging is on%s", VTY_NEWLINE);
  if (IS_ZEBRA_DEBUG_RIB_Q)
    vty_out (vty, "  Rtm RIB queue debugging is on%s", VTY_NEWLINE);

  return CMD_SUCCESS;
}

DEFUN (debug_zebra_events,
       debug_zebra_events_cmd,
       "debug rtm events",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm events\n")
{
  zebra_debug_event = ZEBRA_DEBUG_EVENT;
 /* return CMD_WARNING;*/
 	return CMD_SUCCESS;
}

DEFUN (debug_zebra_packet,
       debug_zebra_packet_cmd,
       "debug rtm packet",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm packet\n")
{
  zebra_debug_packet = ZEBRA_DEBUG_PACKET;
  zebra_debug_packet |= ZEBRA_DEBUG_SEND;
  zebra_debug_packet |= ZEBRA_DEBUG_RECV;
  return CMD_SUCCESS;
}

DEFUN (debug_zebra_packet_direct,
       debug_zebra_packet_direct_cmd,
       "debug rtm packet (recv|send)",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")
{
  zebra_debug_packet = ZEBRA_DEBUG_PACKET;
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet |= ZEBRA_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet |= ZEBRA_DEBUG_RECV;
  zebra_debug_packet &= ~ZEBRA_DEBUG_DETAIL;
  return CMD_SUCCESS;
}

DEFUN (debug_zebra_packet_detail,
       debug_zebra_packet_detail_cmd,
       "debug rtm packet (recv|send) detail",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n"
       "Debug option set detaied information\n")
{
  zebra_debug_packet = ZEBRA_DEBUG_PACKET;
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet |= ZEBRA_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet |= ZEBRA_DEBUG_RECV;
  zebra_debug_packet |= ZEBRA_DEBUG_DETAIL;
  return CMD_SUCCESS;
}

DEFUN (debug_zebra_kernel,
       debug_zebra_kernel_cmd,
       "debug rtm kernel",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm between kernel interface\n")
{
  zebra_debug_kernel = ZEBRA_DEBUG_KERNEL;
  return CMD_SUCCESS;
}

DEFUN (debug_zebra_rib,
       debug_zebra_rib_cmd,
       "debug rtm rib",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug RIB events\n")
{
  SET_FLAG (zebra_debug_rib, ZEBRA_DEBUG_RIB);
  return CMD_SUCCESS;
}

DEFUN (debug_zebra_rib_q,
       debug_zebra_rib_q_cmd,
       "debug rtm rib queue",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug RIB events\n"
       "Debug RIB queueing\n")
{
  SET_FLAG (zebra_debug_rib, ZEBRA_DEBUG_RIB_Q);
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_events,
       no_debug_zebra_events_cmd,
       "no debug rtm events",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm events\n")
{
  zebra_debug_event = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_packet,
       no_debug_zebra_packet_cmd,
       "no debug rtm packet",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm packet\n")
{
  zebra_debug_packet = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_packet_direct,
       no_debug_zebra_packet_direct_cmd,
       "no debug rtm packet (recv|send)",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")
{
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet &= ~ZEBRA_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    zebra_debug_packet &= ~ZEBRA_DEBUG_RECV;
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_kernel,
       no_debug_zebra_kernel_cmd,
       "no debug rtm kernel",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm between kernel interface\n")
{
  zebra_debug_kernel = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_rib,
       no_debug_zebra_rib_cmd,
       "no debug rtm rib",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug rtm RIB\n")
{
  zebra_debug_rib = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_zebra_rib_q,
       no_debug_zebra_rib_q_cmd,
       "no debug rtm rib queue",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug rtm RIB\n"
       "Debug RIB queueing\n")
{
  UNSET_FLAG (zebra_debug_rib, ZEBRA_DEBUG_RIB_Q);
  return CMD_SUCCESS;
}

DEFUN (debug_rtm_distribute_system,
       debug_rtm_distribute_system_cmd,
       "debug rtm distribute system",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm between Distribute System.\n")
{
  rtm_debug_distribute_system = RTM_DEBUG_DISTRIBUTE_SYSTEM;
  return CMD_SUCCESS;
}


DEFUN (no_debug_rtm_distribute_system,
       no_debug_rtm_distribute_system_cmd,
       "no debug rtm distribute system",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm between Distribute System.\n")
{
  rtm_debug_distribute_system = 0;
  return CMD_SUCCESS;
}

DEFUN (debug_rtm_if_flow,
       debug_rtm_if_flow_cmd,
       "debug rtm if_flow",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm interface flow statistics.\n")
{
  rtm_debug_if_flow_stats = RTM_DEBUG_DISTRIBUTE_SYSTEM;
  return CMD_SUCCESS;
}


DEFUN (no_debug_rtm_if_flow,
       no_debug_rtm_if_flow_cmd,
       "no debug rtm if_flow",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm interface flow statistics.\n")
{
  rtm_debug_if_flow_stats = 0;
  return CMD_SUCCESS;
}
DEFUN (debug_rtm_rtadv,
       debug_rtm_rtadv_cmd,
       "debug rtm rtadv",
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm ipv6 nd service.\n")
{
  rtm_debug_rtadv = RTM_DEBUG_IPV6_RTADV;
  return CMD_SUCCESS;
}
DEFUN (no_debug_rtm_rtadv,
       no_debug_rtm_rtadv_cmd,
       "no debug rtm rtadv",
       NO_STR
       DEBUG_STR
       "Rtm configuration\n"
       "Debug option set for rtm rtm ipv6 nd service.\n")
{
	rtm_debug_rtadv = 0;
	return CMD_SUCCESS;
}



/* Debug node. */
struct cmd_node debug_node =
{
  DEBUG_NODE,
  "",				/* Debug node has no interface. */
  1
};

static int
config_write_debug (struct vty *vty)
{
  int write = 0;

  if (IS_ZEBRA_DEBUG_EVENT)
    {
      vty_out (vty, "debug rtm events%s", VTY_NEWLINE);
      write++;
    }
  if (IS_ZEBRA_DEBUG_PACKET)
    {
      if (IS_ZEBRA_DEBUG_SEND && IS_ZEBRA_DEBUG_RECV)
	{
	  vty_out (vty, "debug rtm packet%s%s",
		   IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		   VTY_NEWLINE);
	  write++;
	}
      else
	{
	  if (IS_ZEBRA_DEBUG_SEND)
	    vty_out (vty, "debug rtm packet send%s%s",
		     IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  else
	    vty_out (vty, "debug rtm packet recv%s%s",
		     IS_ZEBRA_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  write++;
	}
    }
  if (IS_ZEBRA_DEBUG_KERNEL)
    {
      vty_out (vty, "debug rtm kernel%s", VTY_NEWLINE);
      write++;
    }
  if (IS_ZEBRA_DEBUG_RIB)
    {
      vty_out (vty, "debug rtm rib%s", VTY_NEWLINE);
      write++;
    }
  if (IS_ZEBRA_DEBUG_RIB_Q)
    {
      vty_out (vty, "debug rtm rib queue%s", VTY_NEWLINE);
      write++;
    }
  return write;
}

void
zebra_debug_init (void)
{
  zebra_debug_event = 0;
  zebra_debug_packet = 0;
  zebra_debug_kernel = 0;
  zebra_debug_rib = 0;
  rtm_debug_distribute_system = 0;
  rtm_debug_rtadv = 0;
#if 1
  install_node (&debug_node, config_write_debug, "DEBUG_NODE");
#else
install_node (&debug_node, NULL, "DEBUG_NODE");
#endif
  install_element (VIEW_NODE, &show_debugging_zebra_cmd);

  install_element (ENABLE_NODE, &show_debugging_zebra_cmd);
  install_element (ENABLE_NODE, &debug_zebra_events_cmd);
  install_element (ENABLE_NODE, &debug_zebra_packet_cmd);
  install_element (ENABLE_NODE, &debug_zebra_packet_direct_cmd);
  install_element (ENABLE_NODE, &debug_zebra_packet_detail_cmd);
  install_element (ENABLE_NODE, &debug_zebra_kernel_cmd);
  install_element (ENABLE_NODE, &debug_zebra_rib_cmd);
  install_element (ENABLE_NODE, &debug_zebra_rib_q_cmd);
  install_element (ENABLE_NODE, &no_debug_zebra_events_cmd);
  install_element (ENABLE_NODE, &no_debug_zebra_packet_cmd);
  install_element (ENABLE_NODE, &no_debug_zebra_kernel_cmd);
  install_element (ENABLE_NODE, &no_debug_zebra_rib_cmd);
  install_element (ENABLE_NODE, &no_debug_zebra_rib_q_cmd);
  /*gjd : for Distrbute System*/
  install_element (ENABLE_NODE, &debug_rtm_distribute_system_cmd);
  install_element (ENABLE_NODE, &no_debug_rtm_distribute_system_cmd);
  
  install_element (ENABLE_NODE, &debug_rtm_rtadv_cmd);
  install_element (ENABLE_NODE, &no_debug_rtm_rtadv_cmd);
  

  install_element (CONFIG_NODE, &debug_zebra_events_cmd);
  install_element (CONFIG_NODE, &debug_zebra_packet_cmd);
  install_element (CONFIG_NODE, &debug_zebra_packet_direct_cmd);
  install_element (CONFIG_NODE, &debug_zebra_packet_detail_cmd);
  install_element (CONFIG_NODE, &debug_zebra_kernel_cmd);
  install_element (CONFIG_NODE, &debug_zebra_rib_cmd);
  install_element (CONFIG_NODE, &debug_zebra_rib_q_cmd);
  install_element (CONFIG_NODE, &no_debug_zebra_events_cmd);
  install_element (CONFIG_NODE, &no_debug_zebra_packet_cmd);
  install_element (CONFIG_NODE, &no_debug_zebra_kernel_cmd);
  install_element (CONFIG_NODE, &no_debug_zebra_rib_cmd);
  install_element (CONFIG_NODE, &no_debug_zebra_rib_q_cmd);
   /*gjd : for Distrbute System*/
  install_element (CONFIG_NODE, &debug_rtm_distribute_system_cmd);
  install_element (CONFIG_NODE, &no_debug_rtm_distribute_system_cmd);
  /*gujd : 2013-05-30. Add for debug if flow statistics.*/
  install_element (ENABLE_NODE, &debug_rtm_if_flow_cmd);
  install_element (ENABLE_NODE, &no_debug_rtm_if_flow_cmd);
  install_element (CONFIG_NODE, &debug_rtm_if_flow_cmd);
  install_element (CONFIG_NODE, &no_debug_rtm_if_flow_cmd);
  install_element (CONFIG_NODE, &debug_rtm_rtadv_cmd);
  install_element (CONFIG_NODE, &no_debug_rtm_rtadv_cmd);
  
}
