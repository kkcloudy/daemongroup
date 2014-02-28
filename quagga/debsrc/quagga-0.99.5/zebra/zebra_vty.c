/* Zebra VTY functions
 * Copyright (C) 2002 Kunihiro Ishiguro
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

#include "if.h"
#include "rt.h"
#include "prefix.h"
#include "command.h"
#include "table.h"
#include "rib.h"
#include "zebra/zserv.h"
#include "string.h"
extern unsigned int se_agent_interval;

/* General fucntion for static route. */
static unsigned int
zebra_get_route_num(unsigned int );

static int
zebra_static_ipv4 (struct vty *vty, int add_cmd, const char *dest_str,
		   const char *mask_str, const char *gate_str,
		   const char *flag_str, const char *distance_str)
{
  int ret;
  u_char distance;
  struct prefix p;
  struct in_addr gate;
  struct in_addr mask;
  const char *ifname;
  u_char flag = 0;
  int retflag=0; 
  char * rnchar;

  
  ret = str2prefix (dest_str, &p);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Cisco like mask notation. */
  if (mask_str)
    {
      ret = inet_aton (mask_str, &mask);
      if (ret == 0)
        {
          vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
          return CMD_WARNING;
        }
	      /*added by gxd*/
      int i=sizeof(u_long)*8-1;
      int j=0;
      u_long ul=ntohl(mask.s_addr);
      while(i>=0){
 		if(ul&((u_long)1<<i)){
 			if(j){
 				vty_out (vty, "%% Malformed mask%s", VTY_NEWLINE);
 				return CMD_WARNING;
 			}
 		}
 		else 
 			j=1;
 		i--;
      }
    /*2008-12-10 17:13:55*/
      p.prefixlen = ip_masklen (mask);
    }

  /* Apply mask for given prefix. */
  apply_mask (&p);
  
  /**gjd: add for check the destenarion addr is multicast or not ?**/
  if( MULTICAST(p.u.prefix4.s_addr))
  {
    vty_out (vty, "%% Warnning: The Destenation address can't be Multicast address%s", VTY_NEWLINE);
    return CMD_WARNING;
  }
  /**2011-04-01: am 9:30**/

  /* Administrative distance. */
  if (distance_str)
    distance = atoi (distance_str);
  else
    distance = ZEBRA_STATIC_DISTANCE_DEFAULT;

  /* Null0 static route.  */
  if ((gate_str != NULL) && (strncasecmp (gate_str, "Null0", strlen (gate_str)) == 0))
    {
      if (flag_str)
        {
          vty_out (vty, "%% can not have flag %s with Null0%s", flag_str, VTY_NEWLINE);
          return CMD_WARNING;
        }
      if (add_cmd){
	  	if(MAX_ZEBRA_STATIC_ROUTE_COUNT==zebra_get_route_num(ZEBRA_ROUTE_STATIC)){
			vty_out(vty,"%% Already has %d of static route%s",MAX_ZEBRA_STATIC_ROUTE_COUNT,
				VTY_NEWLINE);	
			return CMD_WARNING;
		}
	  	retflag=static_add_ipv4_by_vtysh(&p, NULL, NULL,ZEBRA_FLAG_BLACKHOLE, distance, 0);
	  	if(0==retflag)	{
			vty_out(vty,"%% Already has this static route%s",VTY_NEWLINE);	
			return CMD_WARNING;
		}
		 else if (-1==retflag){
			 vty_out(vty,"%% Do not have route table%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		 else if (-2==retflag){
			 vty_out(vty,"%% Already has other static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		  else if (-3==retflag){
			 vty_out(vty,"%% Already has null0 static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		  else if (-6==retflag){
			   vty_out(vty,"%%Warning: The same destnation of Max nexhop num is 8 .%s",VTY_NEWLINE);
			   return CMD_WARNING;
		   }
	  }
      else{
	retflag=static_delete_ipv4_special (&p, NULL, NULL, distance, 0,0);
		if(0==retflag){
			vty_out(vty,"Specified static route  is not found.%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		else if (-1==retflag){
			vty_out(vty,"System error: don't find static route table%s",VTY_NEWLINE);
		 	return CMD_WARNING;
		}
		else if(-4 == retflag)
			{			
				vty_out(vty,"Warning: The reject static route must delete by reject%s",VTY_NEWLINE);
				return CMD_WARNING;
			}
		else if(-5 == retflag)
		  	{
		  		vty_out(vty,"Warning: The blackhole or null0 static route must delete by blackhole or null0%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  	}
      	}		
      return CMD_SUCCESS;
    }

  /* Route flags */
  if (flag_str) {
    switch(flag_str[0]) {
      case 'r':
      case 'R': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_REJECT);
        break;
      case 'b':
      case 'B': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_BLACKHOLE);
        break;
      default:
        vty_out (vty, "%% Malformed flag %s %s", flag_str, VTY_NEWLINE);
        return CMD_WARNING;
    }
  }

  if (gate_str == NULL)
  {
    if (add_cmd) {
		if(MAX_ZEBRA_STATIC_ROUTE_COUNT==zebra_get_route_num(ZEBRA_ROUTE_STATIC)){
			vty_out(vty,"%% Already has %d of static route%s",MAX_ZEBRA_STATIC_ROUTE_COUNT,
				VTY_NEWLINE);	
			return CMD_WARNING;
		}
		 retflag=static_add_ipv4_by_vtysh(&p, NULL, NULL, flag, distance, 0);
	        if(0==retflag){
				vty_out(vty,"%% Already has this static route%s",VTY_NEWLINE);	
				return CMD_WARNING;
			}
		 else if (-1==retflag){
			 vty_out(vty,"%% Do not have route table%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }	
		 else if (-2==retflag){
			 vty_out(vty,"%% Already has other static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		  else if (-3==retflag){
			 vty_out(vty,"%% Already has null0 static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		   else if (-6==retflag){
			  vty_out(vty,"%%Warning: The same destnation of Max nexhop num is 8 .%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  }
    } 
    else{
       	 retflag=static_delete_ipv4_special (&p, NULL, NULL, distance, 0,flag);
		  if(0==retflag){
			  vty_out(vty,"Specified static route is not found. %s",VTY_NEWLINE);
			  return CMD_WARNING;
		  }
		  else if (-1==retflag){
			  vty_out(vty,"System error: don't find static route table%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  } 
		  else if(-4 == retflag)
		  	{
		  		vty_out(vty,"Warning: The reject static route must delete by reject%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  	}
		  else if(-5 == retflag)
		  	{
		  		vty_out(vty,"Warning: The blackhole or null0 static route must delete by blackhole or null0%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  } 
    }	
    return CMD_SUCCESS;
  }
  
  /* When gateway is A.B.C.D format, gate is treated as nexthop
     address other case gate is treated as interface name. */
  ret = inet_aton (gate_str, &gate);

  /**gjd: add for check the gateway addr is multicast or not ?**/
  if( MULTICAST(gate.s_addr))
 {
	vty_out (vty, "%% Warnning: The Gateway address can't be Multicast address%s", VTY_NEWLINE);
    return CMD_WARNING;
  }
  /*2011-04-01: am 9:50***/
  
  /*check out the accuraty of the string of gate_str*/
  if(1==ret){
	rnchar=inet_ntoa(gate);
	if(!strcmp(gate_str,rnchar))
		ifname = NULL;
	else {
		vty_out(vty,"The gateway string is wrong!%s",VTY_NEWLINE);
		return CMD_WARNING;
	}	
  }  		
  else {  	
  	if(!if_lookup_by_name(gate_str)){
		vty_out (vty, "The nexthop or interface string  %s is wrong!%s",gate_str, VTY_NEWLINE);
		return CMD_WARNING;
	}	
    ifname = gate_str;
  }

  if (add_cmd){
  		if(MAX_ZEBRA_STATIC_ROUTE_COUNT==zebra_get_route_num(ZEBRA_ROUTE_STATIC)){
			vty_out(vty,"%% Already has %d of static route%s",MAX_ZEBRA_STATIC_ROUTE_COUNT,
				VTY_NEWLINE);	
			return CMD_WARNING;
		}
  		retflag=static_add_ipv4_by_vtysh(&p, ifname ? NULL : &gate,ifname,flag,distance,0);
		if(0==retflag){
			vty_out(vty,"%% The system already has this static route%s",VTY_NEWLINE); 
			return CMD_WARNING;
		}
		else if (-1==retflag){
		 	vty_out(vty,"%% Do not have route table%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		 else if (-2==retflag){
			 vty_out(vty,"%% Already has other static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		  else if (-3==retflag){
			 vty_out(vty,"%% Already has null0 static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		   else if (-6==retflag){
			  vty_out(vty,"%%Warning: The same destnation of Max nexhop num is 8 .%s",VTY_NEWLINE);
			  return CMD_WARNING;
		  }
  }
  else{
       	retflag=static_delete_ipv4_by_vtysh(&p, ifname ? NULL : &gate,ifname,distance,0);
		if(0==retflag){
			vty_out(vty,"Specified static route is not found. %s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		else if (-1==retflag){
			vty_out(vty,"System error: don't find static route table%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
  }	
  return CMD_SUCCESS;
}

/*delete by gjd*/

#if 0
static int
zebra_static_equal_ipv4 (struct vty *vty, int add_cmd, const char *dest_str,
		   const char *mask_str, const char *gate_str,
		   const char *flag_str, const char *distance_str)
{
  int ret;
  u_char distance;
  struct prefix p;
  struct in_addr gate;
  struct in_addr mask;
  const char *ifname;
  u_char flag = 0;
  int retflag=0; 
  char * rnchar;

  
  ret = str2prefix (dest_str, &p);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Cisco like mask notation. */
  if (mask_str)
    {
      ret = inet_aton (mask_str, &mask);
      if (ret == 0)
        {
          vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
          return CMD_WARNING;
        }
	      /*added by gxd*/
      int i=sizeof(u_long)*8-1;
      int j=0;
      u_long ul=ntohl(mask.s_addr);
      while(i>=0){
 		if(ul&((u_long)1<<i)){
 			if(j){
 				vty_out (vty, "%% Malformed mask%s", VTY_NEWLINE);
 				return CMD_WARNING;
 			}
 		}
 		else 
 			j=1;
 		i--;
      }
    /*2008-12-10 17:13:55*/
      p.prefixlen = ip_masklen (mask);
    }

  /* Apply mask for given prefix. */
  apply_mask (&p);

  /* Administrative distance. */
  if (distance_str)
    distance = atoi (distance_str);
  else
    distance = ZEBRA_STATIC_DISTANCE_DEFAULT;

    

  /* Route flags */
  if (flag_str) {
    switch(flag_str[0]) {
      case 'r':
      case 'R': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_REJECT);
        break;
      case 'b':
      case 'B': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_BLACKHOLE);
        break;
      default:
        vty_out (vty, "%% Malformed flag %s %s", flag_str, VTY_NEWLINE);
        return CMD_WARNING;
    }
  }

  
  
  /* When gateway is A.B.C.D format, gate is treated as nexthop
     address other case gate is treated as interface name. */
  ret = inet_aton (gate_str, &gate);
  
  /*check out the accuraty of the string of gate_str*/
  if(1==ret){
		rnchar=inet_ntoa(gate);
		if(!strcmp(gate_str,rnchar))
			ifname = NULL;
		else {
			vty_out(vty,"The gateway string is wrong!%s",VTY_NEWLINE);
			return CMD_WARNING;
		}	
  }  		
  else {  	
  	if(!if_lookup_by_name(gate_str)){
			vty_out (vty, "The nexthop or interface string  %s is wrong!%s",gate_str, VTY_NEWLINE);
			return CMD_WARNING;
		}	
    ifname = gate_str;/*fetech the gate*/
  }

  if (add_cmd){
  		if(MAX_ZEBRA_STATIC_ROUTE_COUNT==zebra_get_route_num(ZEBRA_ROUTE_STATIC)){
			vty_out(vty,"%% Already has %d of static route%s",MAX_ZEBRA_STATIC_ROUTE_COUNT,
				VTY_NEWLINE);	
			return CMD_WARNING;
		}
  		retflag=static_add_ipv4_equal (&p, ifname ? NULL : &gate,ifname,flag,distance,0);
		if(0==retflag){
			vty_out(vty,"%% The system already has this static route%s",VTY_NEWLINE); 
			return CMD_WARNING;
		}
		else if (-1==retflag){
		 	vty_out(vty,"%% Do not have route table%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		 else if (-2==retflag){
			 vty_out(vty,"%% Already has other static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
		  else if (-3==retflag){
			 vty_out(vty,"%% Already has null0 static route%s",VTY_NEWLINE);
			 return CMD_WARNING;
		 }
  }
  else{
       	retflag=static_delete_ipv4_equal (&p, ifname ? NULL : &gate,ifname,distance,0);
		if(0==retflag){
			vty_out(vty,"Specified static route is not found. %s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		else if (-1==retflag){
			vty_out(vty,"System error: don't find static route table%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
  }	
  return CMD_SUCCESS;
}
#endif

/* Static route configuration.  */
DEFUN (ip_route, 
       ip_route_cmd,
       "ip route A.B.C.D/M (A.B.C.D|INTERFACE|null0)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, argv[1], NULL, NULL);
}

/*delete by gjd*/
/* Static route configuration.  */
/*
DEFUN (ip_route_equal, 
       ip_route_equal_cmd,
       "ip route A.B.C.D/M A.B.C.D equalize",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP route load balance\n")
{
  return zebra_static_equal_ipv4 (vty, 1, argv[0], NULL, argv[1], NULL, NULL);
}
*/

/*delete by gjd*/
/* Static route configuration.  */
/*DEFUN (no_ip_route_equal, 
       no_ip_route_equal_cmd,
       "no ip route A.B.C.D/M A.B.C.D equalize",
				NO_STR
				IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP route load balance\n")
{
  return zebra_static_equal_ipv4 (vty, 0, argv[0], NULL, argv[1], NULL, NULL);
}
*/

DEFUN (ip_route_flags,
       ip_route_flags_cmd,
       "ip route A.B.C.D/M (A.B.C.D|INTERFACE) (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, argv[1], argv[2], NULL);
}

DEFUN (ip_route_flags2,
       ip_route_flags2_cmd,
       "ip route A.B.C.D/M (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, NULL, argv[1], NULL);
}

/* Mask as A.B.C.D format.  */
DEFUN (ip_route_mask,
       ip_route_mask_cmd,
       "ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE|null0)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], argv[2], NULL, NULL);
}

/*delete by gjd*/
/* Mask as A.B.C.D format.  */
/*
DEFUN (ip_route_mask_equal,
       ip_route_mask_equal_cmd,
       "ip route A.B.C.D A.B.C.D A.B.C.D equalize",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
				"IP route load balance\n")
{
  return zebra_static_equal_ipv4 (vty, 1, argv[0], argv[1], argv[2], NULL, NULL);
}
*/

DEFUN (ip_route_mask_flags,
       ip_route_mask_flags_cmd,
       "ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], argv[2], argv[3], NULL);
}

DEFUN (ip_route_mask_flags2,
       ip_route_mask_flags2_cmd,
       "ip route A.B.C.D A.B.C.D (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], NULL, argv[2], NULL);
}

/* Distance option value.  */
DEFUN (ip_route_distance,
       ip_route_distance_cmd,
       "ip route A.B.C.D/M (A.B.C.D|INTERFACE|null0) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, argv[1], NULL, argv[2]);
}

DEFUN (ip_route_flags_distance,
       ip_route_flags_distance_cmd,
       "ip route A.B.C.D/M (A.B.C.D|INTERFACE) (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, argv[1], argv[2], argv[3]);
}

DEFUN (ip_route_flags_distance2,
       ip_route_flags_distance2_cmd,
       "ip route A.B.C.D/M (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], NULL, NULL, argv[1], argv[2]);
}

DEFUN (ip_route_mask_distance,
       ip_route_mask_distance_cmd,
       "ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE|null0) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], argv[2], NULL, argv[3]);
}

DEFUN (ip_route_mask_flags_distance,
       ip_route_mask_flags_distance_cmd,
       "ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Distance value for this route\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], argv[2], argv[3], argv[4]);
}

DEFUN (ip_route_mask_flags_distance2,
       ip_route_mask_flags_distance2_cmd,
       "ip route A.B.C.D A.B.C.D (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "Distance value for this route\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return zebra_static_ipv4 (vty, 1, argv[0], argv[1], NULL, argv[2], argv[3]);
}

DEFUN (no_ip_route, 
       no_ip_route_cmd,
       "no ip route A.B.C.D/M (A.B.C.D|INTERFACE|null0)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       )
{
  return zebra_static_ipv4 (vty, 0, argv[0], NULL, argv[1], NULL, NULL);
}
/*
ALIAS (no_ip_route,
       no_ip_route_flags_cmd,
       "no ip route A.B.C.D/M (A.B.C.D|INTERFACE) (reject|blackhole)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
      "Silently discard pkts when matched\n"
       )
*/
DEFUN (no_ip_route_flags2,
       no_ip_route_flags2_cmd,
       "no ip route A.B.C.D/M (reject|blackhole)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
      )
{
	return zebra_static_ipv4 (vty, 0, argv[0], NULL, NULL, argv[1], NULL);
}

DEFUN (no_ip_route_mask,
       no_ip_route_mask_cmd,
       "no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
      // "Null interface\n"
      )
{
  return zebra_static_ipv4 (vty, 0, argv[0], argv[1], argv[2], NULL, NULL);
}

/*delete by gjd*/
/*
DEFUN (no_ip_route_mask_equal,
       no_ip_route_mask_equal_cmd,
       "no ip route A.B.C.D A.B.C.D A.B.C.D equalize",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
				"IP route load balance\n")
{
  return zebra_static_equal_ipv4 (vty, 0, argv[0], argv[1], argv[2], NULL, NULL);
}
*/

/*
ALIAS (no_ip_route_mask,
       no_ip_route_mask_flags_cmd,
       "no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) (reject|blackhole)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
      )
*/
DEFUN (no_ip_route_mask_flags2,
       no_ip_route_mask_flags2_cmd,
       "no ip route A.B.C.D A.B.C.D",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       //"Emit an ICMP unreachable when matched\n"
       //"Silently discard pkts when matched\n"
       )
{
  return zebra_static_ipv4 (vty, 0, argv[0], argv[1], NULL, NULL, NULL);
}


/*add rp_filter switch by gjd, and move kernel rp_filter to dcli bind with hardware*/
#if 0
DEFUN(rp_filter_on,
		rp_filter_on_cmd,
		"rp_filter on",
		SETT_STR
		SER_STR
		"turn on rp_filter\n"
		"default is on 1\n"
		)
{
	int ret=0;
	int ret2=0;

	ret = system ("sysctl -w net.ipv4.conf.all.rp_filter=1");//turn on rp_filter
	ret = WEXITSTATUS(ret);

	ret2 = system("sudo ip route flush cache");//flush route cache
	ret2 = WEXITSTATUS(ret2);
	
	if(0!=ret)
		{
			return CMD_WARNING;
		}
	else if(0!=ret2)
		{
			
			return CMD_WARNING;
		}
	return CMD_SUCCESS;
}

DEFUN(rp_filter_off,
		rp_filter_off_cmd,
		"rp_filter off",
		SETT_STR
		SER_STR
		"turn off rp_filter\n"
		"default is on 0"
		)
{
	int ret=0;

	ret = system ("sysctl -w net.ipv4.conf.all.rp_filter=0");

	ret = WEXITSTATUS(ret);
	if(0!=ret)
		return CMD_WARNING;
	
	return ret;
}

DEFUN(show_rp_filter,
		show_rp_filter_cmd,
		"show rp_filter",
		SHOW_STR
		"show rp_filter\n"
		"1 instead of on\n"
		"0 instead of off\n"
		)
{	

	

	char buf[10];
	FILE *pipe=NULL;

	pipe=popen("sysctl -n net.ipv4.conf.all.rp_filter","r");
	if (pipe != NULL)
	{
		memset(buf,0,sizeof(buf));
		fgets( buf, sizeof(buf), pipe );	
		pclose( pipe );
	}
	if(buf[0]=='1')
		{
		vty_out (vty, "rp_filter turn on");
	}
	else if(buf[0]=='0')
	{
		vty_out (vty, "rp_filter turn off");
	}
	else
		return CMD_WARNING;
	
	return CMD_SUCCESS;
	
}
int rp_filter_show_running(struct vty *vty)
{	

	
	char buf[32];
	FILE *pipe=NULL;
	int ret = 0;
	char cmd_buf[16];

	memset(cmd_buf,0,16);
	pipe=popen("sysctl -n net.ipv4.conf.all.rp_filter","r");
	if (pipe != NULL)
	{
		memset(buf,0,sizeof(buf));
		fgets( buf, sizeof(buf), pipe );	
		pclose( pipe );
	}
	if(buf[0]!='1')
	{
		sprintf(cmd_buf,"rp_filter off%s",VTY_NEWLINE);
		vty_out (vty, cmd_buf);
		ret += 1;
	}
	return ret;
	
}
#endif
/*deleted by gxd*/
/*
DEFUN (no_ip_route_distance,
       no_ip_route_distance_cmd,
       "no ip route A.B.C.D/M (A.B.C.D|INTERFACE|null0) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], NULL, argv[1], NULL, argv[2]);
}

DEFUN (no_ip_route_flags_distance,
       no_ip_route_flags_distance_cmd,
       "no ip route A.B.C.D/M (A.B.C.D|INTERFACE) (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], NULL, argv[1], argv[2], argv[3]);
}

DEFUN (no_ip_route_flags_distance2,
       no_ip_route_flags_distance2_cmd,
       "no ip route A.B.C.D/M (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix (e.g. 10.0.0.0/8)\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], NULL, NULL, argv[1], argv[2]);
}

DEFUN (no_ip_route_mask_distance,
       no_ip_route_mask_distance_cmd,
       "no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE|null0) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Null interface\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], argv[1], argv[2], NULL, argv[3]);
}

DEFUN (no_ip_route_mask_flags_distance,
       no_ip_route_mask_flags_distance_cmd,
       "no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "IP gateway address\n"
       "IP gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], argv[1], argv[2], argv[3], argv[4]);
}

DEFUN (no_ip_route_mask_flags_distance2,
       no_ip_route_mask_flags_distance2_cmd,
       "no ip route A.B.C.D A.B.C.D (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IP destination prefix\n"
       "IP destination prefix mask\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this route\n")
{
  return zebra_static_ipv4 (vty, 0, argv[0], argv[1], NULL, argv[2], argv[3]);
}
*/
/* New RIB.  Detailed information for IPv4 route. */
static void
vty_show_ip_route_detail (struct vty *vty, struct route_node *rn)
{
  struct rib *rib;
  struct nexthop *nexthop;

  for (rib = rn->info; rib; rib = rib->next)
    {
      vty_out (vty, "Routing entry for %s/%d%s", 
	       inet_ntoa (rn->p.u.prefix4), rn->p.prefixlen,
	       VTY_NEWLINE);
      vty_out (vty, "  Known via \"%s\"", zebra_route_string (rib->type));
      vty_out (vty, ", distance %d, metric %d", rib->distance, rib->metric);
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	vty_out (vty, ", best");
      if (rib->refcnt)
	vty_out (vty, ", refcnt %ld", rib->refcnt);
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
       vty_out (vty, ", blackhole");
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_REJECT))
       vty_out (vty, ", reject");
      vty_out (vty, "%s", VTY_NEWLINE);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7
      if (rib->type == ZEBRA_ROUTE_RIP
	  || rib->type == ZEBRA_ROUTE_OSPF
	  || rib->type == ZEBRA_ROUTE_ISIS
	  || rib->type == ZEBRA_ROUTE_BGP)
	{
	  time_t uptime;
	  struct tm *tm;

	  uptime = time (NULL);
	  uptime -= rib->uptime;
	  tm = gmtime (&uptime);

	  vty_out (vty, "  Last update ");

	  if (uptime < ONE_DAY_SECOND)
	    vty_out (vty,  "%02d:%02d:%02d", 
		     tm->tm_hour, tm->tm_min, tm->tm_sec);
	  else if (uptime < ONE_WEEK_SECOND)
	    vty_out (vty, "%dd%02dh%02dm", 
		     tm->tm_yday, tm->tm_hour, tm->tm_min);
	  else
	    vty_out (vty, "%02dw%dd%02dh", 
		     tm->tm_yday/7,
		     tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
	  vty_out (vty, " ago%s", VTY_NEWLINE);
	}

      for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  vty_out (vty, "  %c",
		   CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) ? '*' : ' ');

	  switch (nexthop->type)
	    {
	    case NEXTHOP_TYPE_IPV4:
	    case NEXTHOP_TYPE_IPV4_IFINDEX:
	      vty_out (vty, " %s", inet_ntoa (nexthop->gate.ipv4));
	      if (nexthop->ifindex)
		vty_out (vty, ", via %s", ifindex2ifname (nexthop->ifindex));
	      break;
	    case NEXTHOP_TYPE_IFINDEX:
	      vty_out (vty, " directly connected, %s",
		       ifindex2ifname (nexthop->ifindex));
	      break;
	    case NEXTHOP_TYPE_IFNAME:
	      vty_out (vty, " directly connected, %s", nexthop->ifname);
	      break;
      case NEXTHOP_TYPE_BLACKHOLE:
        vty_out (vty, " directly connected, Null0");
        break;
      default:
	      break;
	    }
	  if (! CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
	    vty_out (vty, " inactive");

	  if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
	    {
	      vty_out (vty, " (recursive");
		
	      switch (nexthop->rtype)
		{
		case NEXTHOP_TYPE_IPV4:
		case NEXTHOP_TYPE_IPV4_IFINDEX:
		  vty_out (vty, " via %s)", inet_ntoa (nexthop->rgate.ipv4));
		  break;
		case NEXTHOP_TYPE_IFINDEX:
		case NEXTHOP_TYPE_IFNAME:
		  vty_out (vty, " is directly connected, %s)",
			   ifindex2ifname (nexthop->rifindex));
		  break;
		default:
		  break;
		}
	    }
	  vty_out (vty, "%s", VTY_NEWLINE);
	}
			
/*delete by gjd*/
/*			if(rib->equalize == 1)
				{
				vty_out(vty," equalize");
			}
*/
      vty_out (vty, "%s", VTY_NEWLINE);
    }
}

static void
vty_show_ip_route (struct vty *vty, struct route_node *rn, struct rib *rib)
{
  struct nexthop *nexthop;
  int len = 0;
  char buf[BUFSIZ];
  char *ifname = NULL;
  char buf2[BUFSIZ];

  /* Nexthop information. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
/*    
      if (nexthop == rib->nexthop)
*/

	{
		/*gjd : used for search interface 'obc0' , not to show obc0 connnect route .*/
		memset(buf2,0,BUFSIZ);
		if(rib->type == ZEBRA_ROUTE_CONNECT)
		{	
			/*strcpy(buf2,&rn->p.u.prefix);*/
			inet_ntop (AF_INET, &rn->p.u.prefix, buf2, BUFSIZ);/*cp ip address from u.prefix to buf2*/
			if(strncmp(buf2,"169.254.",8) == 0)/*169.254.xxx.xxx*/
			{
				switch(nexthop->type)
				{
					case NEXTHOP_TYPE_IFINDEX:
						/*ifname = ifindex2ifname(nexthop->ifindex);*/
						/*gujd: 2012-02-09: pm 3:30 . In order to decrease the warning when make img . 
						Change ifindex2ifname to ifindex_to_ifname .*/
						ifname = ifindex_to_ifname(nexthop->ifindex);
						if(strncmp(ifname,"obc0",4) == 0 )
						{
							continue;
						}
			  			break;
						
					case NEXTHOP_TYPE_IFNAME:
						if(strncmp(nexthop->ifname,"obc0",4) == 0)
						{
							continue;
						}
			 			break;
						
					default:
	  					break;
						
					}
			}
			
		}

	  /* Prefix information. */
	  len = vty_out (vty, "%c%c%c %s/%d",
			 zebra_route_char (rib->type),
			 /*changed by gxd*/
			// CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED)
			CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED)&& CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE)
			 ? '>' : ' ',
			 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)
			 ? '*' : ' ',
			 inet_ntop (AF_INET, &rn->p.u.prefix, buf, BUFSIZ),
			 rn->p.prefixlen);
		
	  /* Distance and metric display. */
	  if (rib->type != ZEBRA_ROUTE_CONNECT 
	      && rib->type != ZEBRA_ROUTE_KERNEL)
	    len += vty_out (vty, " [%d/%d]", rib->distance,
			    rib->metric);
	}
/*
      else
	vty_out (vty, "  %c%*c",
		 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)
		 ? '*' : ' ',
		 len - 3, ' ');
*/
      switch (nexthop->type)
	{
	case NEXTHOP_TYPE_IPV4:
	case NEXTHOP_TYPE_IPV4_IFINDEX:
	  vty_out (vty, " via %s", inet_ntoa (nexthop->gate.ipv4));
	  if (nexthop->ifindex)
	    vty_out (vty, ", %s", ifindex2ifname (nexthop->ifindex));
	  break;
	case NEXTHOP_TYPE_IFINDEX:
	  vty_out (vty, " is directly connected, %s",
		   ifindex2ifname (nexthop->ifindex));
	  break;
	case NEXTHOP_TYPE_IFNAME:
	  vty_out (vty, " is directly connected, %s", nexthop->ifname);
	  break;
  case NEXTHOP_TYPE_BLACKHOLE:
    vty_out (vty, " is directly connected, Null0");
    break;
  default:
	  break;
	}
      if (! CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
	vty_out (vty, " inactive");

      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
	{
	  vty_out (vty, " (recursive");
		
	  switch (nexthop->rtype)
	    {
	    case NEXTHOP_TYPE_IPV4:
	    case NEXTHOP_TYPE_IPV4_IFINDEX:
	      vty_out (vty, " via %s)", inet_ntoa (nexthop->rgate.ipv4));
	      break;
	    case NEXTHOP_TYPE_IFINDEX:
	    case NEXTHOP_TYPE_IFNAME:
	      vty_out (vty, " is directly connected, %s)",
		       ifindex2ifname (nexthop->rifindex));
	      break;
	    default:
	      break;
	    }
	}

      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
               vty_out (vty, ", bh");
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_REJECT))
               vty_out (vty, ", rej");

      if (rib->type == ZEBRA_ROUTE_RIP
	  || rib->type == ZEBRA_ROUTE_OSPF
	  || rib->type == ZEBRA_ROUTE_ISIS
	  || rib->type == ZEBRA_ROUTE_BGP)
	{
	  time_t uptime;
	  struct tm *tm;

	  uptime = time (NULL);
	  uptime -= rib->uptime;
	  tm = gmtime (&uptime);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

	  if (uptime < ONE_DAY_SECOND)
	    vty_out (vty,  ", %02d:%02d:%02d", 
		     tm->tm_hour, tm->tm_min, tm->tm_sec);
	  else if (uptime < ONE_WEEK_SECOND)
	    vty_out (vty, ", %dd%02dh%02dm", 
		     tm->tm_yday, tm->tm_hour, tm->tm_min);
	  else
	    vty_out (vty, ", %02dw%dd%02dh", 
		     tm->tm_yday/7,
		     tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
	}
/*delete by gjd*/
/*	if(rib->equalize == 1)
		{
		vty_out(vty," equalize");
	}
	*/
			vty_out (vty, "%s", VTY_NEWLINE);
    }
}

#define SHOW_ROUTE_V4_HEADER "Codes: K - kernel route, C - connected, " \
  "S - static, R - RIP, O - OSPF,%s       I - ISIS, B - BGP, " \
  "> - selected route, * - FIB route%s%s"

DEFUN (show_ip_route,
       show_ip_route_cmd,
       "show ip route",
       SHOW_STR
       IP_STR
       "IP routing table\n")
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  int first = 1;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show all IPv4 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      {
				if (first)
				  {
				    vty_out (vty, SHOW_ROUTE_V4_HEADER, VTY_NEWLINE, VTY_NEWLINE,
					     VTY_NEWLINE);
				    first = 0;
				  }
				vty_show_ip_route (vty, rn, rib);
      }
  return CMD_SUCCESS;
}

DEFUN (show_ip_route_prefix_longer,
       show_ip_route_prefix_longer_cmd,
       "show ip route A.B.C.D/M longer-prefixes",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Show route matching the specified Network/Mask pair only\n")
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct prefix p;
  int ret;
  int first = 1;

  ret = str2prefix (argv[0], &p);
  if (! ret)
    {
      vty_out (vty, "%% Malformed Prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show matched type IPv4 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      if (prefix_match (&p, &rn->p))
	{
	  if (first)
	    {
	      vty_out (vty, SHOW_ROUTE_V4_HEADER, VTY_NEWLINE,
		       VTY_NEWLINE, VTY_NEWLINE);
	      first = 0;
	    }
	  vty_show_ip_route (vty, rn, rib);
	}
  return CMD_SUCCESS;
}

DEFUN (show_ip_route_supernets,
       show_ip_route_supernets_cmd,
       "show ip route supernets-only",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Show supernet entries only\n")
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  u_int32_t addr; 
  int first = 1;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show matched type IPv4 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      {
	addr = ntohl (rn->p.u.prefix4.s_addr);

	if ((IN_CLASSC (addr) && rn->p.prefixlen < 24)
	   || (IN_CLASSB (addr) && rn->p.prefixlen < 16)
	   || (IN_CLASSA (addr) && rn->p.prefixlen < 8)) 
	  {
	    if (first)
	      {
		vty_out (vty, SHOW_ROUTE_V4_HEADER, VTY_NEWLINE,
			 VTY_NEWLINE, VTY_NEWLINE);
		first = 0;
	      }
	    vty_show_ip_route (vty, rn, rib);
	  }
      }
  return CMD_SUCCESS;
}

DEFUN (show_ip_route_protocol,
       show_ip_route_protocol_cmd,
       "show ip route (connected|kernel|ospf|rip|static)",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Connected\n"
       "Kernel\n"
       "Open Shortest Path First (OSPF)\n"
       "Routing Information Protocol (RIP)\n"
       "Static routes\n")
{
  int type;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  int first = 1;

  if (strncmp (argv[0], "b", 1) == 0)
    type = ZEBRA_ROUTE_BGP;
  else if (strncmp (argv[0], "c", 1) == 0)
    type = ZEBRA_ROUTE_CONNECT;
  else if (strncmp (argv[0], "k", 1) ==0)
    type = ZEBRA_ROUTE_KERNEL;
  else if (strncmp (argv[0], "o", 1) == 0)
    type = ZEBRA_ROUTE_OSPF;
  else if (strncmp (argv[0], "i", 1) == 0)
    type = ZEBRA_ROUTE_ISIS;
  else if (strncmp (argv[0], "r", 1) == 0)
    type = ZEBRA_ROUTE_RIP;
  else if (strncmp (argv[0], "s", 1) == 0)
    type = ZEBRA_ROUTE_STATIC;
  else 
    {
      vty_out (vty, "Unknown route type%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show matched type IPv4 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      if (rib->type == type)
	{
	  if (first)
	    {
	      vty_out (vty, SHOW_ROUTE_V4_HEADER,
		       VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);
	      first = 0;
	    }
	  vty_show_ip_route (vty, rn, rib);
	}
  return CMD_SUCCESS;
}

DEFUN (show_ip_route_addr,
       show_ip_route_addr_cmd,
       "show ip route A.B.C.D",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Network in the IP routing table to display\n")
{
  int ret;
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;

  ret = str2prefix_ipv4 (argv[0], &p);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv4 address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  rn = route_node_match (table, (struct prefix *) &p);
  if (! rn)
    {
      vty_out (vty, "%% Network not in table%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  vty_show_ip_route_detail (vty, rn);

  route_unlock_node (rn);

  return CMD_SUCCESS;
}

DEFUN (show_ip_route_prefix,
       show_ip_route_prefix_cmd,
       "show ip route A.B.C.D/M",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n")
{
  int ret;
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;

  ret = str2prefix_ipv4 (argv[0], &p);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv4 address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  rn = route_node_match (table, (struct prefix *) &p);
  if (! rn || rn->p.prefixlen != p.prefixlen)
    {
      vty_out (vty, "%% Network not in table%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  vty_show_ip_route_detail (vty, rn);

  route_unlock_node (rn);

  return CMD_SUCCESS;
}

static unsigned int zebra_get_route_num(unsigned int type)
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct nexthop *nexthop;
  int num = 0;

  table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return -1;

  /* Show all IPv4 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      {
		if(type == rib->type)
			/*added by gxd*/
			for(nexthop=rib->nexthop;nexthop;nexthop=nexthop->next)
				num++;
			/*2008-12-2 12:50:36*/
			//num++;
      }
  return num;
}static void
zebra_show_ip_route (struct vty *vty, struct vrf *vrf)
{
	unsigned int connect_num,static_num,rip_num,bgp_num,ospf_num,isis_num,totle;

  connect_num= zebra_get_route_num(ZEBRA_ROUTE_CONNECT);
  static_num= zebra_get_route_num(ZEBRA_ROUTE_STATIC);
  rip_num= zebra_get_route_num(ZEBRA_ROUTE_RIP);
  ospf_num= zebra_get_route_num(ZEBRA_ROUTE_OSPF);
  isis_num= zebra_get_route_num(ZEBRA_ROUTE_ISIS);
  bgp_num= zebra_get_route_num(ZEBRA_ROUTE_BGP);
  totle = connect_num+static_num+rip_num+ospf_num+isis_num+bgp_num;
  vty_out (vty, "IP routing table name is %s(%d)%s",
	   vrf->name ? vrf->name : "", vrf->id, VTY_NEWLINE);

  vty_out (vty, "Route Source	Networks%s", VTY_NEWLINE);
  vty_out (vty, "connected		%d%s", connect_num, VTY_NEWLINE);
  vty_out (vty, "static			%d%s", static_num, VTY_NEWLINE);
  vty_out (vty, "rip			%d%s", rip_num, VTY_NEWLINE);

  vty_out (vty, "bgp			%d%s", bgp_num, VTY_NEWLINE);
  /*
  
  vty_out (vty, " External: %d Internal: %d Local: %d%s",
	   0, 0, 0, VTY_NEWLINE);
*/
  vty_out (vty, "ospf			%d%s", ospf_num, VTY_NEWLINE);
  vty_out (vty, "isis			%d%s", isis_num, VTY_NEWLINE);
/*
  vty_out (vty,
	   "  Intra-area: %d Inter-area: %d External-1: %d External-2: %d%s",
	   0, 0, 0, 0, VTY_NEWLINE);
  vty_out (vty, "  NSSA External-1: %d NSSA External-2: %d%s",
	   0, 0, VTY_NEWLINE);

  vty_out (vty, "internal        %d%s", 0, VTY_NEWLINE);
*/
  vty_out (vty, "Total			%d%s", totle, VTY_NEWLINE);
}

/* Show route summary.  */
DEFUN (show_ip_route_summary,
       show_ip_route_summary_cmd,
       "show ip route summary",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Summary of all routes\n")
{
  struct vrf *vrf;

  /* Default table id is zero.  */
  vrf = vrf_lookup (0);
  if (! vrf)
    {
      vty_out (vty, "%% No Default-IP-Routing-Table%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zebra_show_ip_route (vty, vrf);

  return CMD_SUCCESS;
}

/* Write IPv4 static route configuration. */
static int
static_config_ipv4 (struct vty *vty)
{
  struct route_node *rn;
  struct static_ipv4 *si;  
  struct route_table *stable;
  struct rib *rib;
  int write;
  struct nexthop *nexthop;

  write = 0;
  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP, SAFI_UNICAST, 0);
  if (! stable)
    return -1;

  for (rn = route_top (stable); rn; rn = route_next (rn))
    for (si = rn->info; si; si = si->next)
      {
        vty_out (vty, "ip route %s/%d", inet_ntoa (rn->p.u.prefix4),
                 rn->p.prefixlen);

        switch (si->type)
          {
/*
	delete by gjd
						case STATIC_IPV4_EQUALIZE:
							vty_out (vty, " %s equalize", inet_ntoa (si->gate.ipv4));
							break;
*/

            case STATIC_IPV4_GATEWAY:
              vty_out (vty, " %s", inet_ntoa (si->gate.ipv4));
              break;
            case STATIC_IPV4_IFNAME:
              vty_out (vty, " %s", si->gate.ifname);
              break;
            case STATIC_IPV4_BLACKHOLE:
              //vty_out (vty, " Null0");
              /*added by gxd*/
		 if(ZEBRA_FLAG_REJECT!=si->flags)
		 	vty_out (vty, " Null0");
		 else
		 	vty_out (vty, " reject");
	       /*2008-12-8 16:06:07*/
              break;
          }
        
        /* flags are incompatible with STATIC_IPV4_BLACKHOLE */
        if (si->type != STATIC_IPV4_BLACKHOLE)
          {
            if (CHECK_FLAG(si->flags, ZEBRA_FLAG_REJECT))
              vty_out (vty, " %s", "reject");

            if (CHECK_FLAG(si->flags, ZEBRA_FLAG_BLACKHOLE))
              vty_out (vty, " %s", "blackhole");
          }

        if (si->distance != ZEBRA_STATIC_DISTANCE_DEFAULT)
          vty_out (vty, " %d", si->distance);

        vty_out (vty, "%s", VTY_NEWLINE);

        write = 1;
      }

#if 0
	  /* Lookup table.	*/
	  stable = vrf_table (AFI_IP, SAFI_UNICAST, 0);
	  if (! stable)
		return -1;
	  
	for (rn = route_top (stable); rn; rn = route_next (rn))
	{
		for (rib = rn->info; rib; rib = rib->next)
		{
			if((rib->type == ZEBRA_ROUTE_KERNEL) && (CHECK_FLAG(rib->flags ,ZEBRA_FLAG_SELFROUTE)))
			{
				  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
					{
					  vty_out (vty, "ip route %s/%d", inet_ntoa (rn->p.u.prefix4),
							   rn->p.prefixlen);
				
					switch (nexthop->type)
					{

					case NEXTHOP_TYPE_IPV4:
						vty_out (vty, " %s", inet_ntoa (nexthop->gate.ipv4));
						break;
					case NEXTHOP_TYPE_IPV4_IFINDEX:
					  if (nexthop->ifindex)
						vty_out (vty, " %s", ifindex2ifname (nexthop->ifindex));
					  break;
					case NEXTHOP_TYPE_IFINDEX:
					  break;
					case NEXTHOP_TYPE_IFNAME:
					  vty_out (vty, " %s", nexthop->ifname);
					  break;
				  case NEXTHOP_TYPE_BLACKHOLE:
					vty_out (vty, " Null0");
					break;
				  default:
					  break;
					}
				
				vty_out (vty, "%s", VTY_NEWLINE);

			}
			
			write = 1;
		}
			
		}

	}  
#endif		 
  return write;
}

#ifdef HAVE_IPV6
/* General fucntion for IPv6 static route. */
static int
static_ipv6_func (struct vty *vty, int add_cmd, const char *dest_str,
		  const char *gate_str, const char *ifname,
		  const char *flag_str, const char *distance_str)
{
  int ret;
  u_char distance;
  struct prefix p;
  struct in6_addr *gate = NULL;
  struct in6_addr gate_addr;
  u_char type = 0;
  int table = 0;
  u_char flag = 0;
  
  ret = str2prefix (dest_str, &p);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Apply mask for given prefix. */
  apply_mask (&p);

  /* Route flags */
  if (flag_str) {
    switch(flag_str[0]) {
      case 'r':
      case 'R': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_REJECT);
        break;
      case 'b':
      case 'B': /* XXX */
        SET_FLAG (flag, ZEBRA_FLAG_BLACKHOLE);
        break;
      default:
        vty_out (vty, "%% Malformed flag %s %s", flag_str, VTY_NEWLINE);
        return CMD_WARNING;
    }
  }

  /* Administrative distance. */
  if (distance_str)
    distance = atoi (distance_str);
  else
    distance = ZEBRA_STATIC_DISTANCE_DEFAULT;

  /* When gateway is valid IPv6 addrees, then gate is treated as
     nexthop address other case gate is treated as interface name. */
  ret = inet_pton (AF_INET6, gate_str, &gate_addr);

  if (ifname)
    {
      /* When ifname is specified.  It must be come with gateway
         address. */
      if (ret != 1)
	{
	  vty_out (vty, "%% Malformed address%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      type = STATIC_IPV6_GATEWAY_IFNAME;
      gate = &gate_addr;
    }
  else
    {
      if (ret == 1)
	{
	  type = STATIC_IPV6_GATEWAY;
	  gate = &gate_addr;
	}
      else
	{
	  type = STATIC_IPV6_IFNAME;
	  ifname = gate_str;
	}
    }

  if (add_cmd)
    ret=static_add_ipv6_by_vtysh(&p, type, gate, ifname, flag, distance, table);
  else
    ret=static_delete_ipv6_by_vtysh(&p, type, gate, ifname, distance, table);

  return CMD_SUCCESS;
}

DEFUN (ipv6_route,
       ipv6_route_cmd,
       "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE)",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], NULL, NULL, NULL);
}

DEFUN (ipv6_route_flags,
       ipv6_route_flags_cmd,
       "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], NULL, argv[2], NULL);
}

DEFUN (ipv6_route_ifname,
       ipv6_route_ifname_cmd,
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], argv[2], NULL, NULL);
}

DEFUN (ipv6_route_ifname_flags,
       ipv6_route_ifname_flags_cmd,
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE (reject|blackhole)",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], argv[2], argv[3], NULL);
}

DEFUN (ipv6_route_pref,
       ipv6_route_pref_cmd,
       "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], NULL, NULL, argv[2]);
}

DEFUN (ipv6_route_flags_pref,
       ipv6_route_flags_pref_cmd,
       "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], NULL, argv[2], argv[3]);
}

DEFUN (ipv6_route_ifname_pref,
       ipv6_route_ifname_pref_cmd,
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE <1-255>",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], argv[2], NULL, argv[3]);
}

DEFUN (ipv6_route_ifname_flags_pref,
       ipv6_route_ifname_flags_pref_cmd,
       "ipv6 route X:X::X:X/M X:X::X:X INTERFACE (reject|blackhole) <1-255>",
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 1, argv[0], argv[1], argv[2], argv[3], argv[4]);
}

DEFUN (no_ipv6_route,
       no_ipv6_route_cmd,
       "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n")
{
  return static_ipv6_func (vty, 0, argv[0], argv[1], NULL, NULL, NULL);
}

ALIAS (no_ipv6_route,
       no_ipv6_route_flags_cmd,
       "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) (reject|blackhole)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")

DEFUN (no_ipv6_route_ifname,
       no_ipv6_route_ifname_cmd,
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n")
{
  return static_ipv6_func (vty, 0, argv[0], argv[1], argv[2], NULL, NULL);
}

ALIAS (no_ipv6_route_ifname,
       no_ipv6_route_ifname_flags_cmd,
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE (reject|blackhole)",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n")

DEFUN (no_ipv6_route_pref,
       no_ipv6_route_pref_cmd,
       "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 0, argv[0], argv[1], NULL, NULL, argv[2]);
}

DEFUN (no_ipv6_route_flags_pref,
       no_ipv6_route_flags_pref_cmd,
       "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this prefix\n")
{
  /* We do not care about argv[2] */
  return static_ipv6_func (vty, 0, argv[0], argv[1], NULL, argv[2], argv[3]);
}

DEFUN (no_ipv6_route_ifname_pref,
       no_ipv6_route_ifname_pref_cmd,
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 0, argv[0], argv[1], argv[2], NULL, argv[3]);
}

DEFUN (no_ipv6_route_ifname_flags_pref,
       no_ipv6_route_ifname_flags_pref_cmd,
       "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE (reject|blackhole) <1-255>",
       NO_STR
       IP_STR
       "Establish static routes\n"
       "IPv6 destination prefix (e.g. 3ffe:506::/32)\n"
       "IPv6 gateway address\n"
       "IPv6 gateway interface name\n"
       "Emit an ICMP unreachable when matched\n"
       "Silently discard pkts when matched\n"
       "Distance value for this prefix\n")
{
  return static_ipv6_func (vty, 0, argv[0], argv[1], argv[2], argv[3], argv[4]);
}

/* New RIB.  Detailed information for IPv6 route. */
static void
vty_show_ipv6_route_detail (struct vty *vty, struct route_node *rn)
{
  struct rib *rib;
  struct nexthop *nexthop;
  char buf[BUFSIZ];

  for (rib = rn->info; rib; rib = rib->next)
    {
      vty_out (vty, "Routing entry for %s/%d%s", 
	       inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
	       rn->p.prefixlen,
	       VTY_NEWLINE);
      vty_out (vty, "  Known via \"%s\"", zebra_route_string (rib->type));
      vty_out (vty, ", distance %d, metric %d", rib->distance, rib->metric);
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED))
	vty_out (vty, ", best");
      if (rib->refcnt)
	vty_out (vty, ", refcnt %ld", rib->refcnt);
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
       vty_out (vty, ", blackhole");
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_REJECT))
       vty_out (vty, ", reject");
      vty_out (vty, "%s", VTY_NEWLINE);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7
      if (rib->type == ZEBRA_ROUTE_RIPNG
	  || rib->type == ZEBRA_ROUTE_OSPF6
	  || rib->type == ZEBRA_ROUTE_ISIS
	  || rib->type == ZEBRA_ROUTE_BGP)
	{
	  time_t uptime;
	  struct tm *tm;

	  uptime = time (NULL);
	  uptime -= rib->uptime;
	  tm = gmtime (&uptime);

	  vty_out (vty, "  Last update ");

	  if (uptime < ONE_DAY_SECOND)
	    vty_out (vty,  "%02d:%02d:%02d", 
		     tm->tm_hour, tm->tm_min, tm->tm_sec);
	  else if (uptime < ONE_WEEK_SECOND)
	    vty_out (vty, "%dd%02dh%02dm", 
		     tm->tm_yday, tm->tm_hour, tm->tm_min);
	  else
	    vty_out (vty, "%02dw%dd%02dh", 
		     tm->tm_yday/7,
		     tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
	  vty_out (vty, " ago%s", VTY_NEWLINE);
	}

      for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
	  vty_out (vty, "  %c",
		   CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) ? '*' : ' ');

	  switch (nexthop->type)
	    {
	    case NEXTHOP_TYPE_IPV6:
	    case NEXTHOP_TYPE_IPV6_IFINDEX:
	    case NEXTHOP_TYPE_IPV6_IFNAME:
	      vty_out (vty, " %s",
		       inet_ntop (AF_INET6, &nexthop->gate.ipv6, buf, BUFSIZ));
	      if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
		vty_out (vty, ", %s", nexthop->ifname);
	      else if (nexthop->ifindex)
		vty_out (vty, ", via %s", ifindex2ifname (nexthop->ifindex));
	      break;
	    case NEXTHOP_TYPE_IFINDEX:
	      vty_out (vty, " directly connected, %s",
		       ifindex2ifname (nexthop->ifindex));
	      break;
	    case NEXTHOP_TYPE_IFNAME:
	      vty_out (vty, " directly connected, %s",
		       nexthop->ifname);
	      break;
	    default:
	      break;
	    }
	  if (! CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
	    vty_out (vty, " inactive");

	  if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
	    {
	      vty_out (vty, " (recursive");
		
	      switch (nexthop->rtype)
		{
		case NEXTHOP_TYPE_IPV6:
		case NEXTHOP_TYPE_IPV6_IFINDEX:
		case NEXTHOP_TYPE_IPV6_IFNAME:
		  vty_out (vty, " via %s)",
			   inet_ntop (AF_INET6, &nexthop->rgate.ipv6,
				      buf, BUFSIZ));
		  if (nexthop->rifindex)
		    vty_out (vty, ", %s", ifindex2ifname (nexthop->rifindex));
		  break;
		case NEXTHOP_TYPE_IFINDEX:
		case NEXTHOP_TYPE_IFNAME:
		  vty_out (vty, " is directly connected, %s)",
			   ifindex2ifname (nexthop->rifindex));
		  break;
		default:
		  break;
		}
	    }
	  vty_out (vty, "%s", VTY_NEWLINE);
	}
      vty_out (vty, "%s", VTY_NEWLINE);
    }
}

static void
vty_show_ipv6_route (struct vty *vty, struct route_node *rn,
		     struct rib *rib)
{
  struct nexthop *nexthop;
  int len = 0;
  char buf[BUFSIZ];

  /* Nexthop information. */
  for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
    {
      if (nexthop == rib->nexthop)
	{
	  /* Prefix information. */
	  len = vty_out (vty, "%c%c%c %s/%d",
			 zebra_route_char (rib->type),
			 CHECK_FLAG (rib->flags, ZEBRA_FLAG_SELECTED)
			 ? '>' : ' ',
			 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)
			 ? '*' : ' ',
			 inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
			 rn->p.prefixlen);

	  /* Distance and metric display. */
	  if (rib->type != ZEBRA_ROUTE_CONNECT 
	      && rib->type != ZEBRA_ROUTE_KERNEL)
	    len += vty_out (vty, " [%d/%d]", rib->distance,
			    rib->metric);
	}
      else
	vty_out (vty, "  %c%*c",
		 CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB)
		 ? '*' : ' ',
		 len - 3, ' ');

      switch (nexthop->type)
	{
	case NEXTHOP_TYPE_IPV6:
	case NEXTHOP_TYPE_IPV6_IFINDEX:
	case NEXTHOP_TYPE_IPV6_IFNAME:
	  vty_out (vty, " via %s",
		   inet_ntop (AF_INET6, &nexthop->gate.ipv6, buf, BUFSIZ));
	  if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
	    vty_out (vty, ", %s", nexthop->ifname);
	  else if (nexthop->ifindex)
	    vty_out (vty, ", %s", ifindex2ifname (nexthop->ifindex));
	  break;
	case NEXTHOP_TYPE_IFINDEX:
	  vty_out (vty, " is directly connected, %s",
		   ifindex2ifname (nexthop->ifindex));
	  break;
	case NEXTHOP_TYPE_IFNAME:
	  vty_out (vty, " is directly connected, %s",
		   nexthop->ifname);
	  break;
	default:
	  break;
	}
      if (! CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
	vty_out (vty, " inactive");

      if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
	{
	  vty_out (vty, " (recursive");
		
	  switch (nexthop->rtype)
	    {
	    case NEXTHOP_TYPE_IPV6:
	    case NEXTHOP_TYPE_IPV6_IFINDEX:
	    case NEXTHOP_TYPE_IPV6_IFNAME:
	      vty_out (vty, " via %s)",
		       inet_ntop (AF_INET6, &nexthop->rgate.ipv6,
				  buf, BUFSIZ));
	      if (nexthop->rifindex)
		vty_out (vty, ", %s", ifindex2ifname (nexthop->rifindex));
	      break;
	    case NEXTHOP_TYPE_IFINDEX:
	    case NEXTHOP_TYPE_IFNAME:
	      vty_out (vty, " is directly connected, %s)",
		       ifindex2ifname (nexthop->rifindex));
	      break;
	    default:
	      break;
	    }
	}

      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_BLACKHOLE))
       vty_out (vty, ", bh");
      if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_REJECT))
       vty_out (vty, ", rej");
      
      if (rib->type == ZEBRA_ROUTE_RIPNG
	  || rib->type == ZEBRA_ROUTE_OSPF6
	  || rib->type == ZEBRA_ROUTE_ISIS
	  || rib->type == ZEBRA_ROUTE_BGP)
	{
	  time_t uptime;
	  struct tm *tm;

	  uptime = time (NULL);
	  uptime -= rib->uptime;
	  tm = gmtime (&uptime);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

	  if (uptime < ONE_DAY_SECOND)
	    vty_out (vty,  ", %02d:%02d:%02d", 
		     tm->tm_hour, tm->tm_min, tm->tm_sec);
	  else if (uptime < ONE_WEEK_SECOND)
	    vty_out (vty, ", %dd%02dh%02dm", 
		     tm->tm_yday, tm->tm_hour, tm->tm_min);
	  else
	    vty_out (vty, ", %02dw%dd%02dh", 
		     tm->tm_yday/7,
		     tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
	}
      vty_out (vty, "%s", VTY_NEWLINE);
    }
}

#define SHOW_ROUTE_V6_HEADER "Codes: K - kernel route, C - connected, S - static, R - RIPng, O - OSPFv3,%s       I - ISIS, B - BGP, * - FIB route.%s%s"

DEFUN (show_ipv6_route,
       show_ipv6_route_cmd,
       "show ipv6 route",
       SHOW_STR
       IP_STR
       "IPv6 routing table\n")
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  int first = 1;

  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show all IPv6 route. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      {
	if (first)
	  {
	    vty_out (vty, SHOW_ROUTE_V6_HEADER, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);
	    first = 0;
	  }
	vty_show_ipv6_route (vty, rn, rib);
      }
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_route_prefix_longer,
       show_ipv6_route_prefix_longer_cmd,
       "show ipv6 route X:X::X:X/M longer-prefixes",
       SHOW_STR
       IP_STR
       "IPv6 routing table\n"
       "IPv6 prefix\n"
       "Show route matching the specified Network/Mask pair only\n")
{
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  struct prefix p;
  int ret;
  int first = 1;

  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  ret = str2prefix (argv[0], &p);
  if (! ret)
    {
      vty_out (vty, "%% Malformed Prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Show matched type IPv6 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      if (prefix_match (&p, &rn->p))
	{
	  if (first)
	    {
	      vty_out (vty, SHOW_ROUTE_V6_HEADER, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);
	      first = 0;
	    }
	  vty_show_ipv6_route (vty, rn, rib);
	}
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_route_protocol,
       show_ipv6_route_protocol_cmd,
       "show ipv6 route (bgp|connected|isis|kernel|ospf6|ripng|static)",
       SHOW_STR
       IP_STR
       "IP routing table\n"
       "Border Gateway Protocol (BGP)\n"
       "Connected\n"
       "ISO IS-IS (ISIS)\n"
       "Kernel\n"
       "Open Shortest Path First (OSPFv3)\n"
       "Routing Information Protocol (RIPng)\n"
       "Static routes\n")
{
  int type;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;
  int first = 1;

  if (strncmp (argv[0], "b", 1) == 0)
    type = ZEBRA_ROUTE_BGP;
  else if (strncmp (argv[0], "c", 1) == 0)
    type = ZEBRA_ROUTE_CONNECT;
  else if (strncmp (argv[0], "k", 1) ==0)
    type = ZEBRA_ROUTE_KERNEL;
  else if (strncmp (argv[0], "o", 1) == 0)
    type = ZEBRA_ROUTE_OSPF6;
  else if (strncmp (argv[0], "i", 1) == 0)
    type = ZEBRA_ROUTE_ISIS;
  else if (strncmp (argv[0], "r", 1) == 0)
    type = ZEBRA_ROUTE_RIPNG;
  else if (strncmp (argv[0], "s", 1) == 0)
    type = ZEBRA_ROUTE_STATIC;
  else 
    {
      vty_out (vty, "Unknown route type%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  /* Show matched type IPv6 routes. */
  for (rn = route_top (table); rn; rn = route_next (rn))
    for (rib = rn->info; rib; rib = rib->next)
      if (rib->type == type)
	{
	  if (first)
	    {
	      vty_out (vty, SHOW_ROUTE_V6_HEADER, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);
	      first = 0;
	    }
	  vty_show_ipv6_route (vty, rn, rib);
	}
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_route_addr,
       show_ipv6_route_addr_cmd,
       "show ipv6 route X:X::X:X",
       SHOW_STR
       IP_STR
       "IPv6 routing table\n"
       "IPv6 Address\n")
{
  int ret;
  struct prefix_ipv6 p;
  struct route_table *table;
  struct route_node *rn;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Malformed IPv6 address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  rn = route_node_match (table, (struct prefix *) &p);
  if (! rn)
    {
      vty_out (vty, "%% Network not in table%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  vty_show_ipv6_route_detail (vty, rn);

  route_unlock_node (rn);

  return CMD_SUCCESS;
}

DEFUN (show_ipv6_route_prefix,
       show_ipv6_route_prefix_cmd,
       "show ipv6 route X:X::X:X/M",
       SHOW_STR
       IP_STR
       "IPv6 routing table\n"
       "IPv6 prefix\n")
{
  int ret;
  struct prefix_ipv6 p;
  struct route_table *table;
  struct route_node *rn;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  table = vrf_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! table)
    return CMD_SUCCESS;

  rn = route_node_match (table, (struct prefix *) &p);
  if (! rn || rn->p.prefixlen != p.prefixlen)
    {
      vty_out (vty, "%% Network not in table%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  vty_show_ipv6_route_detail (vty, rn);

  route_unlock_node (rn);

  return CMD_SUCCESS;
}


/* Write IPv6 static route configuration. */
static int
static_config_ipv6 (struct vty *vty)
{
  struct route_node *rn;
  struct static_ipv6 *si;  
  int write;
  char buf[BUFSIZ];
  struct route_table *stable;

  write = 0;

  /* Lookup table.  */
  stable = vrf_static_table (AFI_IP6, SAFI_UNICAST, 0);
  if (! stable)
    return -1;

  for (rn = route_top (stable); rn; rn = route_next (rn))
    for (si = rn->info; si; si = si->next)
      {
	vty_out (vty, "ipv6 route %s/%d",
		 inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
		 rn->p.prefixlen);

	switch (si->type)
	  {
	  case STATIC_IPV6_GATEWAY:
	    vty_out (vty, " %s", inet_ntop (AF_INET6, &si->ipv6, buf, BUFSIZ));
	    break;
	  case STATIC_IPV6_IFNAME:
	    vty_out (vty, " %s", si->ifname);
	    break;
	  case STATIC_IPV6_GATEWAY_IFNAME:
	    vty_out (vty, " %s %s",
		     inet_ntop (AF_INET6, &si->ipv6, buf, BUFSIZ), si->ifname);
	    break;
	  }

       if (CHECK_FLAG(si->flags, ZEBRA_FLAG_REJECT))
               vty_out (vty, " %s", "reject");

       if (CHECK_FLAG(si->flags, ZEBRA_FLAG_BLACKHOLE))
               vty_out (vty, " %s", "blackhole");

	if (si->distance != ZEBRA_STATIC_DISTANCE_DEFAULT)
	  vty_out (vty, " %d", si->distance);
	vty_out (vty, "%s", VTY_NEWLINE);

	write = 1;
      }
  return write;
}
#endif /* HAVE_IPV6 */
/*update if_flow_stats timer interval ,zhaocg add*/
DEFUN (if_flow_stats_update_interval_fun,
       if_flow_stats_update_interval_cmd,
       "set interface_flow updata interval <1-600>",
       "set system configuration\n"
       "Interface flow statistics information\n"
       "updata statistics information\n"
       "Time interval,unit: s \n")
{
  unsigned int interval;
  interval = atoi(argv[0]);
  se_agent_interval = interval;
  return CMD_SUCCESS;
}

DEFUN (show_if_flow_stats_interval_fun,
       show_if_flow_stats_interval_cmd,
       "show interface_flow updata interval",
       SHOW_STR
       "Interface flow statistics information\n"
       "updata statistics information\n"
       "Time interval,unit: s \n")
{
  
  vty_out (vty, "interface_flow updata interval %d s\n", se_agent_interval);
   
  return CMD_SUCCESS;
}

/* Static ip route configuration write function. */
static int
zebra_ip_config (struct vty *vty)
{
  int write = 0;
  vty_out(vty, "set interface_flow updata interval %d\n", se_agent_interval);

  write += static_config_ipv4 (vty);
#ifdef HAVE_IPV6
  write += static_config_ipv6 (vty);
#endif /* HAVE_IPV6 */
//  write += rp_filter_show_running(vty);

  return write;
}

/* IP node for static routes. */
struct cmd_node ip_node = { IP_NODE,  "",  1 };

/* Route VTY.  */
void
zebra_vty_init (void)
{
	install_node (&ip_node, zebra_ip_config, "IP_NODE");

	install_element (CONFIG_NODE, &ip_route_cmd);
/*delete by gjd 2297,98,99*/
//	install_element (CONFIG_NODE, &ip_route_equal_cmd);
//	install_element (CONFIG_NODE, &ip_route_mask_equal_cmd);
//	install_element (CONFIG_NODE, &no_ip_route_equal_cmd);
	install_element (CONFIG_NODE, &ip_route_flags_cmd);
	install_element (CONFIG_NODE, &ip_route_flags2_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_flags_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_flags2_cmd);
	install_element (CONFIG_NODE, &no_ip_route_cmd);
	//install_element (CONFIG_NODE, &no_ip_route_flags_cmd);
	install_element (CONFIG_NODE, &no_ip_route_flags2_cmd);
	install_element (CONFIG_NODE, &no_ip_route_mask_cmd);

/*delete by gjd 2311*/
//	install_element (CONFIG_NODE, &no_ip_route_mask_equal_cmd);

	//install_element (CONFIG_NODE, &no_ip_route_mask_flags_cmd);
	install_element (CONFIG_NODE, &no_ip_route_mask_flags2_cmd);
	install_element (CONFIG_NODE, &ip_route_distance_cmd);
	install_element (CONFIG_NODE, &ip_route_flags_distance_cmd);
	install_element (CONFIG_NODE, &ip_route_flags_distance2_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_distance_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_flags_distance_cmd);
	install_element (CONFIG_NODE, &ip_route_mask_flags_distance2_cmd);
	/*deleted by gxd*/
	// install_element (CONFIG_NODE, &no_ip_route_distance_cmd);
	// install_element (CONFIG_NODE, &no_ip_route_flags_distance_cmd);
	// install_element (CONFIG_NODE, &no_ip_route_flags_distance2_cmd);
	// install_element (CONFIG_NODE, &no_ip_route_mask_flags_distance_cmd);
	// install_element (CONFIG_NODE, &no_ip_route_mask_flags_distance2_cmd);

	install_element (VIEW_NODE, &show_ip_route_cmd);
	install_element (VIEW_NODE, &show_ip_route_addr_cmd);
	install_element (VIEW_NODE, &show_ip_route_prefix_cmd);
	install_element (VIEW_NODE, &show_ip_route_prefix_longer_cmd);
	install_element (VIEW_NODE, &show_ip_route_protocol_cmd);
	install_element (VIEW_NODE, &show_ip_route_supernets_cmd);
	install_element (ENABLE_NODE, &show_ip_route_cmd);
	install_element (ENABLE_NODE, &show_ip_route_addr_cmd);
	install_element (ENABLE_NODE, &show_ip_route_prefix_cmd);
	install_element (ENABLE_NODE, &show_ip_route_prefix_longer_cmd);
	install_element (ENABLE_NODE, &show_ip_route_protocol_cmd);
	install_element (ENABLE_NODE, &show_ip_route_supernets_cmd);

	install_element (CONFIG_NODE, &show_ip_route_cmd);
	install_element (CONFIG_NODE, &show_ip_route_addr_cmd);
	install_element (CONFIG_NODE, &show_ip_route_prefix_cmd);
	install_element (CONFIG_NODE, &show_ip_route_prefix_longer_cmd);
	install_element (CONFIG_NODE, &show_ip_route_protocol_cmd);
	install_element (CONFIG_NODE, &show_ip_route_supernets_cmd);

	install_element (INTERFACE_NODE, &show_ip_route_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_addr_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_prefix_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_prefix_longer_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_protocol_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_supernets_cmd);
	install_element (VIEW_NODE, &show_ip_route_summary_cmd);
	install_element (ENABLE_NODE, &show_ip_route_summary_cmd);
	install_element (CONFIG_NODE, &show_ip_route_summary_cmd);
	install_element (INTERFACE_NODE, &show_ip_route_summary_cmd);

#ifdef HAVE_IPV6
	install_element (CONFIG_NODE, &ipv6_route_cmd);
	install_element (CONFIG_NODE, &ipv6_route_flags_cmd);
	install_element (CONFIG_NODE, &ipv6_route_ifname_cmd);
	install_element (CONFIG_NODE, &ipv6_route_ifname_flags_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_flags_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_ifname_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_ifname_flags_cmd);
	install_element (CONFIG_NODE, &ipv6_route_pref_cmd);
	install_element (CONFIG_NODE, &ipv6_route_flags_pref_cmd);
	install_element (CONFIG_NODE, &ipv6_route_ifname_pref_cmd);
	install_element (CONFIG_NODE, &ipv6_route_ifname_flags_pref_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_pref_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_flags_pref_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_ifname_pref_cmd);
	install_element (CONFIG_NODE, &no_ipv6_route_ifname_flags_pref_cmd);
	install_element (VIEW_NODE, &show_ipv6_route_cmd);
	install_element (VIEW_NODE, &show_ipv6_route_protocol_cmd);
	install_element (VIEW_NODE, &show_ipv6_route_addr_cmd);
	install_element (VIEW_NODE, &show_ipv6_route_prefix_cmd);
	install_element (VIEW_NODE, &show_ipv6_route_prefix_longer_cmd);
	install_element (ENABLE_NODE, &show_ipv6_route_cmd);
	install_element (ENABLE_NODE, &show_ipv6_route_protocol_cmd);
	install_element (ENABLE_NODE, &show_ipv6_route_addr_cmd);
	install_element (ENABLE_NODE, &show_ipv6_route_prefix_cmd);
	install_element (ENABLE_NODE, &show_ipv6_route_prefix_longer_cmd);

install_element (CONFIG_NODE, &show_ipv6_route_cmd);
install_element (CONFIG_NODE, &show_ipv6_route_protocol_cmd);
install_element (CONFIG_NODE, &show_ipv6_route_addr_cmd);
install_element (CONFIG_NODE, &show_ipv6_route_prefix_cmd);
install_element (CONFIG_NODE, &show_ipv6_route_prefix_longer_cmd);
install_element (INTERFACE_NODE, &show_ipv6_route_cmd);

install_element (ENABLE_NODE, &show_if_flow_stats_interval_cmd);
install_element (ENABLE_NODE, &if_flow_stats_update_interval_cmd);
install_element (CONFIG_NODE, &if_flow_stats_update_interval_cmd);


/*move to dcli*/
#if 0
/*add by gjd*/
install_element (CONFIG_NODE, &rp_filter_on_cmd);
install_element (CONFIG_NODE, &rp_filter_off_cmd);
install_element (VIEW_NODE, &show_rp_filter_cmd);
install_element (CONFIG_NODE, &show_rp_filter_cmd);
#endif
#endif /* HAVE_IPV6 */
}
