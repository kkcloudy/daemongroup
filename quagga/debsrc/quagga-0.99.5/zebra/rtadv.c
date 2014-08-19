/* Router advertisement
 * Copyright (C) 2005 6WIND <jean-mickael.guerin@6wind.com>
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

#include "memory.h"
#include "sockopt.h"
#include "thread.h"
#include "if.h"
#include "log.h"
#include "prefix.h"
#include "linklist.h"
#include "command.h"
#include "privs.h"

#include "zebra/interface.h"
#include "zebra/rtadv.h"
#include "zebra/debug.h"
#include "zebra/rib.h"
#include "zebra/zserv.h"
#include "zebra/tipc_client.h"
#include "zebra/tipc_server.h"

extern struct zebra_privs_t zserv_privs;
/*#define HAVE_RTADR_DEBUG 0*/

#if defined (HAVE_IPV6) && defined (RTADV)

#ifdef OPEN_BSD
#include <netinet/icmp6.h>
#endif

/* If RFC2133 definition is used. */
#ifndef IPV6_JOIN_GROUP
#define IPV6_JOIN_GROUP  IPV6_ADD_MEMBERSHIP 
#endif
#ifndef IPV6_LEAVE_GROUP
#define IPV6_LEAVE_GROUP IPV6_DROP_MEMBERSHIP 
#endif

#define ALLNODE   "ff02::1"
#define ALLROUTER "ff02::2"

extern struct zebra_t zebrad;

enum rtadv_event {RTADV_START, RTADV_STOP, RTADV_TIMER, 
		  RTADV_TIMER_MSEC, RTADV_READ,RTADV_TIMER_MSEC_IF};

/*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/
static void rtadv_event (enum rtadv_event, int,void *);

static int if_join_all_router (int, struct interface *);
static int if_leave_all_router (int, struct interface *);
static void rtadv_prefix_set (struct zebra_if *zif, struct rtadv_prefix *rp);
static void rtadv_prefix_pool_set (struct zebra_if *zif, struct rtadv_prefix *rp);
static int rtadv_prefix_reset (struct zebra_if *zif, struct rtadv_prefix *rp);


/* Structure which hold status of router advertisement. */
struct rtadv
{
  int sock;

  int adv_if_count;
  int adv_msec_if_count;

  struct thread *ra_read;
  struct thread *ra_timer;
  struct thread *ra_timer_send;/*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/
};

struct rtadv *rtadv = NULL;
extern product_inf *product;
/*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/
static struct timeval this_time;
static struct timeval last_time;

int
sysctl_set_proc_file_vlaue_of_AdvReachableTime(struct interface *ifp, u_int32_t value)
{
	unsigned char cmd[BUFFER_LEN] = {0};
	int ret=CMD_SUCCESS;
	
	/*To change the system file :/proc/sys/net/ipv6/neigh/xxx/base_reachable_time_ms .*/
	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.base_reachable_time_ms=%d", ifp->name, value);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				  
	if(CMD_SUCCESS != ret )
	 {
		  zlog_warn("Modify Interface(%s) Reachable time(%u) fail.\n", ifp->name,value);
		  return CMD_WARNING;
	  }
}

/*use sysctl to change the proc file value.*/
int
sysctl_set_proc_file_vlaue_of_AdvRetransTimer(struct interface *ifp, u_int32_t value)
{
	unsigned char cmd[BUFFER_LEN] = {0};
	int ret=CMD_SUCCESS;
	
	/*To change the system file :/proc/sys/net/ipv6/neigh/xxx/retrans_time_ms .*/
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.retrans_time_ms=%d", ifp->name,value);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				  
	if(CMD_SUCCESS != ret )
	 {
		  zlog_warn("Modify Interface(%s) RetransTime(%u) fail.\n", ifp->name,value);
		  return CMD_WARNING;
	  }
}

int
sysctl_set_proc_file_vlaue_of_AdvCurHopLimit(struct interface *ifp, int value)
{
	unsigned char cmd[BUFFER_LEN] = {0};
	int ret=CMD_SUCCESS;
	
	/*To change the system file :/proc/sys/net/ipv6/conf/xxxx/hop_limit .*/
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.conf.%s.hop_limit=%d", ifp->name,value);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				  
	if(CMD_SUCCESS != ret )
	 {
		  zlog_warn("Modify Interface(%s) Hoplimit(%d) fail.\n", ifp->name,value);
		  return CMD_WARNING;
	  }
}


/*gujd: 2013-05-14, pm 4:05 . Add code for ipv6 nd suppress ra deal for Distribute System.*/
struct interface *
tipc_vice_interface_nd_surppress_ra_state_read(int command, struct stream *s)
{
	if(!s)
	 return NULL;
	
	char ifname_tmp[INTERFACE_NAMSIZ] = {0};
	struct interface *ifp;
	
	stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
	/*if(tipc_server_debug)*/
	 zlog_debug("%s : interface (%s) nd state (%s).\n", __func__,
	 			ifname_tmp,zserv_command_string(command));
	 ifp = if_lookup_by_name_len (ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
 	 if(ifp == NULL) 
  	 {
  		if(tipc_server_debug)
	 	  zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
     	return NULL;
  	 }

	 return ifp;
	
}

int
tipc_vice_interface_nd_suppress_ra_deal(int command, tipc_server *vice_board, zebra_size_t length)
{
	if(!vice_board)
	 return -1;
	
	struct interface *ifp;	
	struct zebra_if *zif;
	
	ifp = tipc_vice_interface_nd_surppress_ra_state_read( command,vice_board->ibuf);
	if(ifp == NULL)
	{
	  if(tipc_server_debug)
		zlog_debug("the interface not match !");
	  return -1;
	}
#if 0
	if(judge_real_local_interface(ifp->name)!= LOCAL_BOARD_INTERFACE)/*not local board, don't care*/
	 return -1;
#endif
	zif = ifp->info;
	
	if(command == ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE)/*disable (turn off) nd router suppress*/
	{
		zlog_debug("%s : to enable interface(%s) nd suppress router advitisement.\n",
					__func__,ifp->name);
		if (zif->rtadv.AdvSendAdvertisements)
  #if 0
		{
	      zif->rtadv.AdvSendAdvertisements = 0;
	      zif->rtadv.AdvIntervalTimer = 0;
	      rtadv->adv_if_count--;

	      if_leave_all_router (rtadv->sock, ifp);

	      if (rtadv->adv_if_count == 0)
	      {
	      	zlog_debug("to goto rtadv_event(RTADV_STOP).\n");
			rtadv_event (RTADV_STOP, 0);
	      	}
		  
	    }
  #else
	  {
	      zif->rtadv.AdvSendAdvertisements = 0;
	      zif->rtadv.AdvIntervalTimer = 0;
	    /*  rtadv->adv_if_count--;*/
	      rtadv->adv_if_count = 0;

	 	/*  if_leave_all_router (rtadv->sock, ifp);*/
		  if(RTM_DEBUG_RTADV)
		    zlog_info("%s:line %d, adv_if_count is %d .\n",__func__,__LINE__,rtadv->adv_if_count);
	      if (rtadv->adv_if_count == 0)
	      {
	     	/* zif->rtadv.AdvDefaultLifetime = 0;*/
			if(RTM_DEBUG_RTADV)
			 zlog_info("to RTADV_STOP_PRE \n");
			/*rtadv_event (RTADV_STOP_PRE, 0,NULL);*/
			rtadv_stop_pre_send_one_packet(ifp);
	      	}
	    }

  #endif
		return 0;
	}
	else if(command == ZEBRA_INTERFACE_ND_SUPPRESS_RA_DISABLE)/*enable (turn on) nd router suppress*/
	{
		zlog_debug("%s : to disable interface(%s) nd suppress router advitisement.\n",
						__func__,ifp->name);
		if (! zif->rtadv.AdvSendAdvertisements)
	    {
	      zif->rtadv.AdvSendAdvertisements = 1;
	      zif->rtadv.AdvIntervalTimer = 0;
	      rtadv->adv_if_count++;////////////////////////xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	      if_join_all_router (rtadv->sock, ifp);

	      if (rtadv->adv_if_count == 1)
	      {
			zlog_debug("to goto rtadv_event(RTADV_START).\n");
			/*rtadv_event (RTADV_START, rtadv->sock);*/			
			rtadv_event (RTADV_START, rtadv->sock,NULL);
	      	}
	    }
		
		return 0;
		
		}
	else
	{
		zlog_debug("unkown command[%d].\n",command);
		return -1;
	}
	

	
}

struct interface *
tipc_vice_rtadv_nd_info_read(int command, struct stream *s)
{

	if(!s)
	{
	  zlog_info("%s: line %d. The stream is NULL.\n",__func__,__LINE__);
	  return NULL;
	}
	
	char ifname_tmp[INTERFACE_NAMSIZ] = {0};
	struct interface *ifp;
	struct zebra_if *zif;
	u_int32_t AdvReachableTime = 0;
	u_int32_t AdvRetransTimer = 0;
	int AdvCurHopLimit = 0;
	
	
	stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
	
	if(RTM_DEBUG_RTADV)
	 zlog_debug("%s : interface (%s) command (%s).\n", __func__,
				ifname_tmp,zserv_command_string(command));
	
	 ifp = if_lookup_by_name_len (ifname_tmp,
				   strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
	 if(ifp == NULL) 
	 {
		if(RTM_DEBUG_RTADV)
		  zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
		return NULL;
	 }
	 
	 zif = ifp->info;
	 /*prase the packet info*/
	 zif->rtadv.AdvSendAdvertisements = stream_getl(s);
	 zif->rtadv.MaxRtrAdvInterval = stream_getl(s);
	 zif->rtadv.MinRtrAdvInterval = stream_getl(s);
	 zif->rtadv.AdvIntervalTimer = stream_getl(s);
	 zif->rtadv.AdvManagedFlag = stream_getl(s);
	 zif->rtadv.AdvOtherConfigFlag = stream_getl(s);
	 zif->rtadv.AdvLinkMtuOption= stream_getl(s);
	 zif->rtadv.AdvLinkMTU = stream_getl(s);
	 
	/* zif->rtadv.AdvReachableTime = stream_getl(s);*/
	 AdvReachableTime = stream_getl(s);/*tmp*/
	 if(zif->rtadv.AdvReachableTime != AdvReachableTime)
	  {
	  	zif->rtadv.AdvReachableTime = AdvReachableTime;/*update new value*/
		/*If the parameter is valid , so to set system proc file for kernel.*/
		if (zif->rtadv.AdvReachableTime > 0 && zif->rtadv.AdvReachableTime <= 0x7fffffff)
		{
		  sysctl_set_proc_file_vlaue_of_AdvReachableTime(ifp, AdvReachableTime);
		 }
		else
		{
		  zlog_debug("The interface(%s) of AdvReachableTime(%u) is not legal.\n",ifp->name,AdvReachableTime);
		  }
	   }
	 
	 /*zif->rtadv.AdvRetransTimer = stream_getl(s);*/
	 AdvRetransTimer = stream_getl(s);
	 if(zif->rtadv.AdvRetransTimer != AdvRetransTimer)
	  {
	    zif->rtadv.AdvRetransTimer = AdvRetransTimer;
		if(zif->rtadv.AdvRetransTimer > 0)
		{
			sysctl_set_proc_file_vlaue_of_AdvRetransTimer(ifp,AdvRetransTimer);
		 }
		else
		{
		  zlog_debug("The interface(%s) of AdvRetransTimer(%u) is not legal.\n",ifp->name,AdvRetransTimer);
		  }
	   }
	 
	 /*zif->rtadv.AdvCurHopLimit = stream_getl(s);*/
	 AdvCurHopLimit = stream_getl(s);
	 if(zif->rtadv.AdvCurHopLimit != AdvCurHopLimit)
	  {
	  	 zif->rtadv.AdvCurHopLimit = AdvCurHopLimit;
		 if(zif->rtadv.AdvCurHopLimit > 0)
		 {
		 	sysctl_set_proc_file_vlaue_of_AdvCurHopLimit(ifp, AdvCurHopLimit);
		  }
		 else
		 {
		   zlog_debug("The interface(%s) of AdvCurHopLimit(%d) is not legal.\n",ifp->name,AdvCurHopLimit);
		   }
	   }
	 
	 zif->rtadv.AdvDefaultLifetime = stream_getl(s);
	 zif->rtadv.AdvHomeAgentFlag = stream_getl(s);
	 zif->rtadv.HomeAgentPreference = stream_getl(s);
	 zif->rtadv.HomeAgentLifetime = stream_getl(s);
	 zif->rtadv.AdvIntervalOption = stream_getl(s);
#if 1	 
	 if(RTM_DEBUG_RTADV)
	   zlog_debug("%s : interface(%s),\
	   			  AdvSendAdvertisements[%d],\
	              MaxRtrAdvInterval[%d],\
	              MinRtrAdvInterval[%d],\
	              AdvIntervalTimer[%d],\
	              AdvManagedFlag[%d],\
	              AdvOtherConfigFlag[%d],\
	              AdvLinkMtuOption[%d],\
	              AdvLinkMTU[%d],\
	              AdvReachableTime[%d],\
	              AdvRetransTimer[%d],\
	              AdvCurHopLimit[%d],\
	              AdvDefaultLifetime[%d],\
	              AdvHomeAgentFlag[%d],\
	              HomeAgentPreference[%d],\
	              HomeAgentLifetime[%d],AdvIntervalOption[%d].\n",
	              __func__,ifp->name,
	 zif->rtadv.AdvSendAdvertisements ,
	 zif->rtadv.MaxRtrAdvInterval ,
	 zif->rtadv.MinRtrAdvInterval ,
	 zif->rtadv.AdvIntervalTimer ,
	 zif->rtadv.AdvManagedFlag ,
	 zif->rtadv.AdvOtherConfigFlag ,
	 zif->rtadv.AdvLinkMtuOption,
	 zif->rtadv.AdvLinkMTU ,
	 zif->rtadv.AdvReachableTime ,
	 zif->rtadv.AdvRetransTimer ,
	 zif->rtadv.AdvCurHopLimit ,
	 zif->rtadv.AdvDefaultLifetime ,
	 zif->rtadv.AdvHomeAgentFlag ,
	 zif->rtadv.HomeAgentPreference ,
	 zif->rtadv.HomeAgentLifetime ,
	 zif->rtadv.AdvIntervalOption );
#else	 
	 if(RTM_DEBUG_RTADV)
	   zlog_debug("%s : interface(%s),"
	   "AdvSendAdvertisements[%d],"
	   "MaxRtrAdvInterval[%d],"
	   "MinRtrAdvInterval[%d], "
	   "AdvIntervalTimer[%d],"
	   "AdvManagedFlag[%d],"
	   "AdvOtherConfigFlag[%d],"
	   "AdvLinkMTU[%d],"
	   "AdvReachableTime[%d],"
	   "AdvRetransTimer[%d],"
	   "AdvCurHopLimit[%d],"
	   "AdvDefaultLifetime[%d],"
	   "AdvHomeAgentFlag[%d],"
	   "HomeAgentPreference[%d],"
	   "HomeAgentLifetime[%d],"
	   "AdvIntervalOption[%d].\n",
	              __func__,ifp->name,
	 zif->rtadv.AdvSendAdvertisements ,
	 zif->rtadv.MaxRtrAdvInterval ,
	 zif->rtadv.MinRtrAdvInterval ,
	 zif->rtadv.AdvIntervalTimer ,
	 zif->rtadv.AdvManagedFlag ,
	 zif->rtadv.AdvOtherConfigFlag ,
	 zif->rtadv.AdvLinkMTU ,
	 zif->rtadv.AdvReachableTime ,
	 zif->rtadv.AdvRetransTimer ,
	 zif->rtadv.AdvCurHopLimit ,
	 zif->rtadv.AdvDefaultLifetime ,
	 zif->rtadv.AdvHomeAgentFlag ,
	 zif->rtadv.HomeAgentPreference ,
	 zif->rtadv.HomeAgentLifetime ,
	 zif->rtadv.AdvIntervalOption );
#endif
	 return ifp;
	
}


int
tipc_vice_rtadv_nd_info_update(int command, tipc_server* vice_board, int length)
{
	if(!vice_board)
	 return -1;
	
	struct interface *ifp;	
	struct zebra_if *zif;
	
	ifp = tipc_vice_rtadv_nd_info_read( command,vice_board->ibuf);
	
	if(!ifp)
	  return -1;

#if 0
	zif = ifp->info;
	
	if(command == ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE)/*disable (turn off) nd router suppress*/
	{
		zlog_debug("%s : to enable interface(%s) nd suppress router advitisement.\n",
					__func__,ifp->name);
		if (zif->rtadv.AdvSendAdvertisements)
		{
		  zif->rtadv.AdvSendAdvertisements = 0;
		  zif->rtadv.AdvIntervalTimer = 0;
		  rtadv->adv_if_count--;

		  if_leave_all_router (rtadv->sock, ifp);

		  if (rtadv->adv_if_count == 0)
		  {
			zlog_debug("to goto rtadv_event(RTADV_STOP).\n");
			rtadv_event (RTADV_STOP, 0);
			}
		  
		}
		return 0;
	}
	else
		if(command == ZEBRA_INTERFACE_ND_SUPPRESS_RA_DISABLE)/*enable (turn on) nd router suppress*/
	{
		zlog_debug("%s : to disable interface(%s) nd suppress router advitisement.\n",
						__func__,ifp->name);
		if (! zif->rtadv.AdvSendAdvertisements)
		{
		  zif->rtadv.AdvSendAdvertisements = 1;
		  zif->rtadv.AdvIntervalTimer = 0;
		  rtadv->adv_if_count++;

		  if_join_all_router (rtadv->sock, ifp);

		  if (rtadv->adv_if_count == 1)
		  {
			zlog_debug("to goto rtadv_event(RTADV_START).\n");
			rtadv_event (RTADV_START, rtadv->sock);
			}
		}
		
		return 0;
		
		}
	else
	{
		zlog_debug("unkown command[%d].\n",command);
		return -1;
	}
	#endif

	return 0;

	
}



struct interface *
tipc_vice_interface_nd_prefix_info_read(int command, struct stream *s,struct rtadv_prefix *rp)
{

	if(!s)
	{
	  zlog_info("%s: line %d. The stream is NULL.\n",__func__,__LINE__);
	  return NULL;
	}
	
	char ifname_tmp[INTERFACE_NAMSIZ] = {0};
	struct interface *ifp;
	struct zebra_if *zif;
	int str_length = 0;
	char prefix_str[IPV6_MAX_BITLEN]={0};
	int ret = 0;
	
	stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
	
	if(RTM_DEBUG_RTADV)
	 zlog_debug("%s : interface (%s) command (%s).\n", __func__,
				ifname_tmp,zserv_command_string(command));
	
	 ifp = if_lookup_by_name_len (ifname_tmp,
				   strnlen(ifname_tmp, INTERFACE_NAMSIZ));
 
	 if(ifp == NULL) 
	 {
		if(RTM_DEBUG_RTADV)
		  zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
		return NULL;
	 }

	 str_length = stream_getl(s);/*fetch prefixe string length.*/
	 stream_get(prefix_str, s, str_length);/*fetch prefixe string .*/
	 prefix_str[str_length]='\0';
	 
	 ret = str2prefix_ipv6 (prefix_str, (struct prefix_ipv6 *)&rp->prefix);
	 if (!ret)
	 {
		 zlog_warn ("Malformed IPv6 prefix[%s]", prefix_str);
		 return NULL;
	   }
	 if(RTM_DEBUG_RTADV)
	 	zlog_debug("IPv6 prefix string : %s .\n",prefix_str);
	 
	 
	 if(command == ZEBRA_INTERFACE_ND_PREFIX_ADD)
	{
		rp->AdvOnLinkFlag = stream_getl(s);
		rp->AdvAutonomousFlag = stream_getl(s);
		rp->AdvRouterAddressFlag = stream_getl(s);
		rp->AdvValidLifetime = stream_getl(s);
		rp->AdvPreferredLifetime = stream_getl(s);

		if(RTM_DEBUG_RTADV)
	   	  zlog_debug("%s : interface(%s),\
				  AdvOnLinkFlag[%d],\
				  AdvAutonomousFlag[%d],\
				  AdvRouterAddressFlag[%d],\
				  AdvValidLifetime[%d],\
				  AdvPreferredLifetime[%d]. \n",
				  __func__,ifp->name,
	    rp->AdvOnLinkFlag ,
		rp->AdvAutonomousFlag ,
		rp->AdvRouterAddressFlag,
		rp->AdvValidLifetime ,
		rp->AdvPreferredLifetime );
	 }
	
	 
	 return ifp;
	
}



struct interface *
tipc_vice_interface_nd_prefix_pool_info_read(int command, struct stream *s,struct rtadv_prefix *rp_start,struct rtadv_prefix *rp_end)
{

	if(!s)
	{
	  zlog_info("%s: line %d. The stream is NULL.\n",__func__,__LINE__);
	  return NULL;
	}
	
	char ifname_tmp[INTERFACE_NAMSIZ] = {0};
	struct interface *ifp;
	struct zebra_if *zif;
	int str_length = 0;
	char prefix_str[IPV6_MAX_BITLEN]={0};
	char prefix_end[IPV6_MAX_BITLEN]={0};
	int ret = 0;
	
	stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);
	
	if(RTM_DEBUG_RTADV)
	 zlog_debug("%s : interface (%s) command (%s).\n", __func__,
				ifname_tmp,zserv_command_string(command));
	
	 ifp = if_lookup_by_name_len (ifname_tmp,
				   strnlen(ifname_tmp, INTERFACE_NAMSIZ));
	 zif  = ifp->info;
 
	 if(ifp == NULL) 
	 {
		if(RTM_DEBUG_RTADV)
		  zlog_debug("%s : line %d, interface do not exist or the interface not match!\n",__func__,__LINE__);
		return NULL;
	 }
	if (command == ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD)
	{
	
		 str_length = stream_getl(s);/*fetch prefixe string length.*/
		 stream_get(prefix_str, s, str_length);/*fetch prefixe string .*/
		 prefix_str[str_length]='\0';
		 str_length = stream_getl(s);/*fetch prefixe string length.*/
		 stream_get(prefix_end, s, str_length);/*fetch prefixe string .*/
		 prefix_end[str_length]='\0';
		
		
		 ret = str2prefix_ipv6 (prefix_str, (struct prefix_ipv6 *)&(rp_start->prefix));
		 if (!ret)
		 {
			 zlog_warn ("Malformed IPv6 prefix[%s]", prefix_str);
			 return NULL;
		   }
		  ret = str2prefix_ipv6 (prefix_end, (struct prefix_ipv6 *)&(rp_end->prefix));
		 if (!ret)
		 {
			 zlog_warn ("Malformed IPv6 prefix[%s]", prefix_end);
			 return NULL;
		   }
		
			rp_start->AdvOnLinkFlag= stream_getl(s);
			rp_start->AdvAutonomousFlag = stream_getl(s);
			rp_start->AdvRouterAddressFlag =  stream_getl(s);
			rp_start->AdvValidLifetime =  stream_getl(s);
			rp_start->AdvPreferredLifetime =  stream_getl(s);
			rp_end->AdvOnLinkFlag = stream_getl(s);
			rp_end->AdvAutonomousFlag = stream_getl(s);
			rp_end->AdvRouterAddressFlag =  stream_getl(s);
			rp_end->AdvValidLifetime = stream_getl(s);
			rp_end->AdvPreferredLifetime = stream_getl(s);
			
			
			zif->rtadv.prefix_flag = 1;
			
			
		   	  zlog_info("%s : interface(%s),\
					  AdvOnLinkFlag[%d],\
					  AdvAutonomousFlag[%d],\
					  AdvRouterAddressFlag[%d],\
					  AdvValidLifetime[%d],\
					  AdvPreferredLifetime[%d]. \n",
					  __func__,ifp->name,
 			rp_start->AdvOnLinkFlag ,
			rp_start->AdvAutonomousFlag ,
			rp_start->AdvRouterAddressFlag,
			rp_start->AdvValidLifetime ,
			rp_start->AdvPreferredLifetime );
			 			
		   	  zlog_info("%s : interface(%s),\
					  AdvOnLinkFlag[%d],\
					  AdvAutonomousFlag[%d],\
					  AdvRouterAddressFlag[%d],\
					  AdvValidLifetime[%d],\
					  AdvPreferredLifetime[%d]. \n",
					  __func__,ifp->name,
 			rp_end->AdvOnLinkFlag ,
			rp_end->AdvAutonomousFlag ,
			rp_end->AdvRouterAddressFlag,
			rp_end->AdvValidLifetime ,
			rp_end->AdvPreferredLifetime );
			 
			
		 
	}
		
	 
	 return ifp;
	
}



int
tipc_vice_interface_nd_prefix_update(int command, tipc_server* vice_board, int length)
{
	
	if(!vice_board)
	 return -1;
	
	struct interface *ifp;	
	struct zebra_if *zif;
	struct rtadv_prefix rp;
	int ret = 0;
	
	ifp = tipc_vice_interface_nd_prefix_info_read(command,vice_board->ibuf,&rp);
	
	if(!ifp)
	  return -1;
	
	zif = ifp->info;
	
	if(command == ZEBRA_INTERFACE_ND_PREFIX_ADD)
	{
		if(RTM_DEBUG_RTADV)
		  zlog_debug ("To add ipv6 nd prefix .\n");
		rtadv_prefix_set (zif, &rp);
		
	 }
	else if(command == ZEBRA_INTERFACE_ND_PREFIX_DELETE)
	{
		if(RTM_DEBUG_RTADV)
		  zlog_debug ("To delete ipv6 nd prefix .\n");
		ret = rtadv_prefix_reset (zif, &rp);
		if (!ret)
		{
			zlog_info( "Non-exist IPv6 prefix.\n");
			return -1;
		  }
	 }
	else
	{
		zlog_warn("Unknow command [%d].\n",command);
		return -1;
	 }
	
	return 0;
	
}


int
tipc_vice_interface_nd_prefix_pool_update(int command, tipc_server* vice_board, int length)
{
	
	if(!vice_board)
	 return -1;
	
	struct interface *ifp;	
	struct zebra_if *zif;
	struct rtadv_prefix rp_start,rp_end;
	int ret = 0;

	zlog_info("func %s, line %d	  ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD",__func__,__LINE__ );
	ifp = tipc_vice_interface_nd_prefix_pool_info_read(command,vice_board->ibuf,&rp_start,&rp_end);
	
	if(!ifp)
	  return -1;
	
	zif = ifp->info;
	
	if(command == ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD)
	{
		if(RTM_DEBUG_RTADV)
		  zlog_debug ("To add ipv6 nd prefix pool .\n");
		 zif->rtadv_prefix_pool = (struct ipv6_pool **)  malloc(sizeof(struct ipv6_pool *)*256*256);
 		 memset(zif->rtadv_prefix_pool ,0,sizeof(struct ipv6_pool *)*256*256);
		rtadv_prefix_pool_set(zif, &rp_start);
		rtadv_prefix_pool_set(zif, &rp_end);
		 rtadv_prefix_pool_set (zif, &rp_start);
		 zif->rtadv.prefix_flag = 1;
		 
		zlog_info("add prefix start"NIP6QUAD_FMT "\n",NIP6QUAD(rp_start.prefix.u.prefix6.s6_addr));
		zlog_info("add prefix end"NIP6QUAD_FMT "\n",NIP6QUAD(rp_end.prefix.u.prefix6.s6_addr));
	 }
	else if(command == ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE)
	{
		if(RTM_DEBUG_RTADV)
		  zlog_debug ("To delete ipv6 nd prefix pool .\n");
		struct ipv6_pool *s,*p;
		int i;
		
		zif->rtadv.prefix_flag = 0;
		  if (zif->rtadv.pool->head != NULL)
		{
			struct rtadv_prefix *next;
			while(zif->rtadv.pool->head != NULL)
	  		 {
				  next = zif->rtadv.pool->head->next;
		  		 free (zif->rtadv.pool->head);
				zif->rtadv.pool->head = NULL;
		   		zif->rtadv.pool->head = next;
		   	}
  		}
		for(i =0; i <= 256*256; i++)
		{ 
			  if ( zif->rtadv_prefix_pool[i] != NULL)
			  {
				  s = zif->rtadv_prefix_pool[i];
				zif->rtadv_prefix_pool[i] = NULL;
				  while (s != NULL)
				  {
					  p = s;
					  s = s->next;
					  free(p);
				  }
			}
		  else
			  continue;
		}
		  free(zif->rtadv_prefix_pool );
 		 zif->rtadv_prefix_pool = NULL;
		
	 }
	else
	{
		zlog_warn("Unknow command [%d].\n",command);
		return -1;
	 }
	
	return 0;
	
}


int
tipc_master_packet_rtadv_nd_info(tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  struct zebra_if *zif;

  
  zif = ifp->info;

/*Let all interface to support the ipv6 nd setup.*/
#if 0
  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#endif


  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header(s, ZEBRA_INTERFACE_RTADV_ND_INFO_UPDATE);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);

  /*packet the rtadv info.*/
  stream_putl(s, zif->rtadv.AdvSendAdvertisements);
  stream_putl(s, zif->rtadv.MaxRtrAdvInterval);
  stream_putl(s, zif->rtadv.MinRtrAdvInterval);
  stream_putl(s, zif->rtadv.AdvIntervalTimer);
  stream_putl(s, zif->rtadv.AdvManagedFlag);
  stream_putl(s, zif->rtadv.AdvOtherConfigFlag);
  stream_putl(s, zif->rtadv.AdvLinkMtuOption);
  stream_putl(s, zif->rtadv.AdvLinkMTU);
  stream_putl(s, zif->rtadv.AdvReachableTime);
  stream_putl(s, zif->rtadv.AdvRetransTimer);
  stream_putl(s, zif->rtadv.AdvCurHopLimit);
  stream_putl(s, zif->rtadv.AdvDefaultLifetime);
  stream_putl(s, zif->rtadv.AdvHomeAgentFlag);
  stream_putl(s, zif->rtadv.HomeAgentPreference);
  stream_putl(s, zif->rtadv.HomeAgentLifetime);
  stream_putl(s, zif->rtadv.AdvIntervalOption);
  
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
  
}


int
tipc_master_rtadv_nd_info_update(struct interface *ifp)
{
	
	struct listnode *node, *nnode;
	tipc_client *master_board;

	if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	{
		if(zebrad.vice_board_list == NULL)
		{
			zlog_debug("There is no one vice board connect.\n");
			return -1;
		  }
	     
		for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
		{ 
		   tipc_master_packet_rtadv_nd_info(master_board, ifp);
		  }
           
	}

	return 0;
	
}


int
tipc_master_interface_nd_suppress_ra_packet_info(int cmd, tipc_client *client, struct interface *ifp)
{
  struct stream *s;
  
#if 0
  if((judge_ve_interface(ifp->name)== VE_INTERFACE)
  	||(judge_obc_interface(ifp->name)== OBC_INTERFACE)
  	||(judge_radio_interface(ifp->name)== DISABLE_RADIO_INTERFACE))
  	return 0;
#endif
  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header(s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
  
}

int tipc_master_interface_nd_suppress_ra_deal(int command, struct interface *ifp)
{
	if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	 return -1;
	
	struct listnode *node, *nnode;
  	tipc_client *master_board;
		
    if(zebrad.vice_board_list == NULL)
	{
  	   zlog_debug("%s, line = %d, zebrad.vice_board_list is null...", __func__, __LINE__);
       return;
	}
 	  
      /*send message to all of connected vice board*/
	for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
   	{ 
		zlog_debug("master send interface(%s) msg to vice board .", ifp->name); 	   
		tipc_master_interface_nd_suppress_ra_packet_info (command, master_board, ifp);
	  }
 	 
}


int
tipc_master_packet_interface_nd_prefix_info(tipc_client *client, int cmd,
                                                struct interface *ifp,
                                                struct rtadv_prefix *rp, 
                                                const char *prefix_str)
{
  struct stream *s;
  struct zebra_if *zif;
  int length = 0;

  
  zif = ifp->info;

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header(s, cmd);

  /* Interface information. */
  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
  
  /*Prefix str*/
  length = strlen(prefix_str);
  stream_putl(s,length);/*packet prefix string length*/
  stream_put(s,prefix_str,length);/*packet prefix string*/

  if(cmd == ZEBRA_INTERFACE_ND_PREFIX_ADD)/*when prefix add, packet rp info. Prefix delete, don't need*/
  {
  	
	stream_putl(s, rp->AdvOnLinkFlag);
	stream_putl(s, rp->AdvAutonomousFlag);
	stream_putl(s, rp->AdvRouterAddressFlag);
	stream_putl(s, rp->AdvValidLifetime);
	stream_putl(s, rp->AdvPreferredLifetime);
	
  	}
  
  stream_putw_at (s, 0, stream_get_endp (s));

  return master_send_message_to_vice(client);
  
}


int
tipc_master_packet_interface_nd_prefix_pool_info(tipc_client *client, int cmd,
                                                   struct interface *ifp,
                                                   struct rtadv_prefix *rp_start, 
                                		 struct rtadv_prefix *rp_end, 
                              		 const char *pool_start,
                               		 const char *pool_end)
{
  struct stream *s;
  struct zebra_if *zif;
  int length = 0;

  
  zif = ifp->info;

  /* Check this client need interface information. */
  if (! client->ifinfo)
    return 0;

  s = client->obuf;
  stream_reset (s);

  tipc_packet_create_header(s, cmd);
if (cmd ==  ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD)
{

	  /* Interface information. */
	  stream_put (s, ifp->name, INTERFACE_NAMSIZ);
	  
	  /*Prefix str*/
	    length = strlen(pool_start) ;
	  stream_putl(s,length);/*packet prefix string length*/
	  stream_put(s,pool_start,length);/*packet prefix string*/
	   length = strlen(pool_end) ;
	  stream_putl(s,length);/*packet prefix string length*/
	  stream_put(s,pool_end,length);/*packet prefix string*/

	stream_putl(s, rp_start->AdvOnLinkFlag);
	stream_putl(s, rp_start->AdvAutonomousFlag);
	stream_putl(s, rp_start->AdvRouterAddressFlag);
	stream_putl(s, rp_start->AdvValidLifetime);
	stream_putl(s, rp_start->AdvPreferredLifetime);
	stream_putl(s, rp_end->AdvOnLinkFlag);
	stream_putl(s, rp_end->AdvAutonomousFlag);
	stream_putl(s, rp_end->AdvRouterAddressFlag);
	stream_putl(s, rp_end->AdvValidLifetime);
	stream_putl(s, rp_end->AdvPreferredLifetime);
		
	  	
	  zif->rtadv.prefix_flag = 1;
	  
}
else
{
	stream_put (s, ifp->name, INTERFACE_NAMSIZ);
	zif->rtadv.prefix_flag = 0;
}
stream_putw_at (s, 0, stream_get_endp (s));


  return master_send_message_to_vice(client);
  
}



int
tipc_master_interface_nd_prefix_update(int command, 
                                struct interface *ifp,
                                struct rtadv_prefix *rp, 
                                const char *prefix_str)
{
	
	struct listnode *node, *nnode;
	tipc_client *master_board;

	if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	{
		if(zebrad.vice_board_list == NULL)
		{
			zlog_debug("There is no one vice board connect.\n");
			return -1;
		  }
		if(command == ZEBRA_INTERFACE_ND_PREFIX_ADD)
		{
			for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			{ 
			   tipc_master_packet_interface_nd_prefix_info(master_board,ZEBRA_INTERFACE_ND_PREFIX_ADD, ifp, rp, prefix_str);
			  }
		  }
		else if(command == ZEBRA_INTERFACE_ND_PREFIX_DELETE)
		{
			for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			{ 
			   tipc_master_packet_interface_nd_prefix_info(master_board,ZEBRA_INTERFACE_ND_PREFIX_DELETE, ifp, rp, prefix_str);
			  }
		  }
		else
		 {
		 	zlog_warn("The unkown command.\n");
			return -1;
		  }
			
		   
	}

	return 0;
	
}


int
tipc_master_interface_nd_prefix_pool_update(int command, 
                                struct interface *ifp,
                                struct rtadv_prefix *rp_start, 
                                struct rtadv_prefix *rp_end, 
                                const char *pool_start,
                                const char *pool_end)
{
	struct listnode *node, *nnode;
	tipc_client *master_board;

	if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	{
		if(zebrad.vice_board_list == NULL)
		{
			zlog_debug("There is no one vice board connect.\n");
			return -1;
		  }
		
		if(command == ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD)
		{
			for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			{ 
				
			  tipc_master_packet_interface_nd_prefix_pool_info(master_board,ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD, ifp, rp_start, rp_end,pool_start,pool_end);
			}
		  }
		else if(command == ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE)
		{
			for (ALL_LIST_ELEMENTS (zebrad.vice_board_list, node, nnode, master_board)) 
			{ 
				tipc_master_packet_interface_nd_prefix_pool_info(master_board,ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE, ifp, rp_start, rp_end,pool_start,pool_end);
			  }
		  }
		else
		 {
		 	zlog_warn("The unkown command.\n");
			return -1;
		  }
			
		   
	}

	return 0;
	
}



static struct rtadv *
rtadv_new (void)
{
  struct rtadv *new;
  new = XMALLOC (MTYPE_TMP, sizeof (struct rtadv));
  memset (new, 0, sizeof (struct rtadv));
  return new;
}

static void
rtadv_free (struct rtadv *rtadv)
{
  XFREE (MTYPE_TMP, rtadv);
}

static int
rtadv_recv_packet (int sock, u_char *buf, int buflen,
		   struct sockaddr_in6 *from, unsigned int *ifindex,
		   int *hoplimit)
{
  int ret;
  struct msghdr msg ={0}/*add intialize 0*/;
  struct iovec iov;
  struct cmsghdr  *cmsgptr;
  struct in6_addr dst;

  char adata[1024];

  /* Fill in message and iovec. */
  msg.msg_name = (void *) from;  
  msg.msg_namelen = sizeof (struct sockaddr_in6);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (void *) adata;
  msg.msg_controllen = sizeof adata;
  iov.iov_base = buf;
  iov.iov_len = buflen;

  /*CID 14336 (#1 of 1): Uninitialized scalar variable (UNINIT)
	2. uninit_use_in_call: Using uninitialized value "msg": field "msg"."msg_flags" is uninitialized when calling "recvmsg(int, struct msghdr *, int)".
	So add init struct msg 0.*/
  /* If recvmsg fail return minus value. */
  ret = recvmsg (sock, &msg, 0);
  if (ret < 0)
    return ret;

  for (cmsgptr = ZCMSG_FIRSTHDR(&msg); cmsgptr != NULL;
       cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) 
    {
      /* I want interface index which this packet comes from. */
      if (cmsgptr->cmsg_level == IPPROTO_IPV6 &&
	  cmsgptr->cmsg_type == IPV6_PKTINFO) 
	{
	  struct in6_pktinfo *ptr;
	  
	  ptr = (struct in6_pktinfo *) CMSG_DATA (cmsgptr);
	  *ifindex = ptr->ipi6_ifindex;
	  memcpy(&dst, &ptr->ipi6_addr, sizeof(ptr->ipi6_addr));
        }

      /* Incoming packet's hop limit. */
      if (cmsgptr->cmsg_level == IPPROTO_IPV6 &&
	  cmsgptr->cmsg_type == IPV6_HOPLIMIT)
	*hoplimit = *((int *) CMSG_DATA (cmsgptr));
    }
	   
  return ret;
}

#define RTADV_MSG_SIZE 4096
#define RTADV_GLOBAL_HASH(sta) (sta[14]<<8 |sta[15])

struct ipv6_pool * rtadv_prefix_pool_hash_get(struct interface *ifp ,const struct in6_addr *sta)
{
	struct ipv6_pool *s = NULL;
	struct zebra_if *zif;
	zif = ifp->info;
	
	s = zif->rtadv_prefix_pool[RTADV_GLOBAL_HASH(sta->s6_addr)];
	while (s != NULL && memcmp(&s->link_addr, sta,sizeof(struct in6_addr )) != 0)
		s = s->next;
	return s;
}

void rtadv_prefix_pool_hash_add(struct interface *ifp ,struct ipv6_pool*sta)
{
	struct zebra_if *zif;
	zif = ifp->info;
	
 	sta->next =  zif->rtadv_prefix_pool[RTADV_GLOBAL_HASH(sta->link_addr.s6_addr)];
	 zif->rtadv_prefix_pool[RTADV_GLOBAL_HASH(sta->link_addr.s6_addr)] = sta;
 }


/* Send router advertisement packet. */
static void
rtadv_send_packet (int sock, struct interface *ifp)
{
  struct msghdr msg;
  struct iovec iov;
  struct cmsghdr  *cmsgptr;
  struct in6_pktinfo *pkt;
  struct sockaddr_in6 addr;
#ifdef HAVE_SOCKADDR_DL
  struct sockaddr_dl *sdl;
#endif /* HAVE_SOCKADDR_DL */
  static void *adata = NULL;
  unsigned char buf[RTADV_MSG_SIZE];
  struct nd_router_advert *rtadv;
  int ret;
  int len = 0;
  struct zebra_if *zif;
  struct rtadv_prefix *rprefix,*prefix_next,*prefix_step;
  u_char all_nodes_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
  struct listnode *node;

  /*
   * Allocate control message bufffer.  This is dynamic because
   * CMSG_SPACE is not guaranteed not to call a function.  Note that
   * the size will be different on different architectures due to
   * differing alignment rules.
   */
  if (adata == NULL)
    {
      /* XXX Free on shutdown. */
      adata = malloc(CMSG_SPACE(sizeof(struct in6_pktinfo)));
	   
      if (adata == NULL){
				zlog_err("rtadv_send_packet: can't malloc control data\n");
			}
    }

  /* Logging of packet. */
  if (IS_ZEBRA_DEBUG_PACKET)
    zlog_debug ("Router advertisement send to %s", ifp->name);

  /* Fill in sockaddr_in6. */
  memset (&addr, 0, sizeof (struct sockaddr_in6));
  addr.sin6_family = AF_INET6;
#ifdef SIN6_LEN
  addr.sin6_len = sizeof (struct sockaddr_in6);
#endif /* SIN6_LEN */
  addr.sin6_port = htons (IPPROTO_ICMPV6);
  memcpy (&addr.sin6_addr, all_nodes_addr, sizeof (struct in6_addr));

  /* Fetch interface information. */
  zif = ifp->info;

  /* Make router advertisement message. */
  rtadv = (struct nd_router_advert *) buf;

  rtadv->nd_ra_type = ND_ROUTER_ADVERT;
  rtadv->nd_ra_code = 0;
  rtadv->nd_ra_cksum = 0;
#if 0  /*gujd : 2012-05-29,am 9:53 . Change  for IPv6 Ready Test.*/
  if(zif->rtadv.AdvCurHopLimit != 0)
  	rtadv->nd_ra_curhoplimit = htonl(zif->rtadv.AdvCurHopLimit);
  else
    rtadv->nd_ra_curhoplimit = 64;
#else
	rtadv->nd_ra_curhoplimit = htonl(zif->rtadv.AdvCurHopLimit);
#endif

  rtadv->nd_ra_flags_reserved = 0;
  if (zif->rtadv.AdvManagedFlag)
    rtadv->nd_ra_flags_reserved |= ND_RA_FLAG_MANAGED;
  if (zif->rtadv.AdvOtherConfigFlag)
    rtadv->nd_ra_flags_reserved |= ND_RA_FLAG_OTHER;
  if (zif->rtadv.AdvHomeAgentFlag)
    rtadv->nd_ra_flags_reserved |= ND_RA_FLAG_HOME_AGENT;
  /*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/
  if (zif->rtadv.AdvSendAdvertisements)
  	rtadv->nd_ra_router_lifetime = htons (zif->rtadv.AdvDefaultLifetime);
  else
  	rtadv->nd_ra_router_lifetime = htons (0);
  
  rtadv->nd_ra_reachable = htonl (zif->rtadv.AdvReachableTime);
  /*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/
  if(zif->rtadv.AdvRetransTimer != 0)
  	rtadv->nd_ra_retransmit = htonl(zif->rtadv.AdvRetransTimer);
  else
  	rtadv->nd_ra_retransmit = htonl (0);

  len = sizeof (struct nd_router_advert);

  if (zif->rtadv.AdvHomeAgentFlag)
    {
      struct nd_opt_homeagent_info *ndopt_hai = 
	(struct nd_opt_homeagent_info *)(buf + len);
      ndopt_hai->nd_opt_hai_type = ND_OPT_HA_INFORMATION;
      ndopt_hai->nd_opt_hai_len = 1;
      ndopt_hai->nd_opt_hai_reserved = 0;
      ndopt_hai->nd_opt_hai_preference = htons(zif->rtadv.HomeAgentPreference);
      ndopt_hai->nd_opt_hai_lifetime = htons(zif->rtadv.HomeAgentLifetime);
      len += sizeof(struct nd_opt_homeagent_info);
    }

  if (zif->rtadv.AdvIntervalOption)
    {
      struct nd_opt_adv_interval *ndopt_adv = 
	(struct nd_opt_adv_interval *)(buf + len);
      ndopt_adv->nd_opt_ai_type = ND_OPT_ADV_INTERVAL;
      ndopt_adv->nd_opt_ai_len = 1;
      ndopt_adv->nd_opt_ai_reserved = 0;
      ndopt_adv->nd_opt_ai_interval = htonl(zif->rtadv.MaxRtrAdvInterval);
      len += sizeof(struct nd_opt_adv_interval);
    }
  /*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/
  if(zif->rtadv.AdvLinkMtuOption)
  	{
      struct nd_opt_adv_interval *ndopt_adv = 
	(struct nd_opt_adv_interval *)(buf + len);
      ndopt_adv->nd_opt_ai_type = ND_OPT_ADV_LINKMTU;
      ndopt_adv->nd_opt_ai_len = 1;
      ndopt_adv->nd_opt_ai_reserved = 0;
      ndopt_adv->nd_opt_ai_interval = htonl(zif->rtadv.AdvLinkMTU);
      len += sizeof(struct nd_opt_adv_interval);
	 /* zlog_debug("packet have mtu option. \n");*/
    }

  /* Fill in prefix. */
  
  if (zif->rtadv.prefix_flag == 0)
  {
	  for (ALL_LIST_ELEMENTS_RO (zif->rtadv.AdvPrefixList, node, rprefix))
	    {
	      struct nd_opt_prefix_info *pinfo;

	      pinfo = (struct nd_opt_prefix_info *) (buf + len);

	      pinfo->nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
	      pinfo->nd_opt_pi_len = 4;
	      pinfo->nd_opt_pi_prefix_len = rprefix->prefix.prefixlen;

	      pinfo->nd_opt_pi_flags_reserved = 0;
	      if (rprefix->AdvOnLinkFlag)
		pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_ONLINK;
	      if (rprefix->AdvAutonomousFlag)
		pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_AUTO;
	      if (rprefix->AdvRouterAddressFlag)
		pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_RADDR;

	      pinfo->nd_opt_pi_valid_time = htonl (rprefix->AdvValidLifetime);
	      pinfo->nd_opt_pi_preferred_time = htonl (rprefix->AdvPreferredLifetime);
	      pinfo->nd_opt_pi_reserved2 = 0;

	      memcpy (&pinfo->nd_opt_pi_prefix, &rprefix->prefix.u.prefix6,
		      sizeof (struct in6_addr));

#ifdef DEBUG
	      {
		u_char buf[INET6_ADDRSTRLEN];

		zlog_debug ("DEBUG %s", inet_ntop (AF_INET6, &pinfo->nd_opt_pi_prefix, 
		           buf, INET6_ADDRSTRLEN));

	      }
#endif /* DEBUG */

	      len += sizeof (struct nd_opt_prefix_info);
	    }
}
 else
 {			
 		
 			rprefix = zif->rtadv.pool->head->data;
 			 zlog_info("prefix start ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(rprefix->prefix.u.prefix6.s6_addr));
			prefix_next = zif->rtadv.pool->head->next->data;
			zlog_info(" prefix end  ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(prefix_next->prefix.u.prefix6.s6_addr));
			prefix_step = zif->rtadv.pool->head->next->next->data;
			zlog_info("prefix step  ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(prefix_step->prefix.u.prefix6.s6_addr));
			struct nd_opt_prefix_info *pinfo;
			 int num = 0,flag = 0;
			  num = rprefix->prefix.prefixlen/8 - 1;
			  struct ipv6_pool *new_pool,*new_prefix;
			  struct in6_addr send_prefix;
			  memset(&send_prefix,0 ,sizeof(struct in6_addr));
			    zlog_info(" prefix_old  ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(prefix_step->prefix.u.prefix6.s6_addr));
			if (zif->rtadv.dst.s6_addr16[0] == 0xFE80)
			  {
				new_prefix = rtadv_prefix_pool_hash_get(ifp,&zif->rtadv.dst);
				if (new_prefix == NULL)
				{
					new_pool = (struct ipv6_pool *)malloc(sizeof(struct ipv6_pool));
					if (new_pool == NULL)
						zlog_info("malloc fail\n");
					memset(new_pool,0,sizeof(struct ipv6_pool));
					memcpy(&new_pool->link_addr,&zif->rtadv.dst,sizeof(struct in6_addr));
					memcpy(&new_pool->prefix, &prefix_step->prefix.u.prefix6,sizeof(struct in6_addr));
					new_pool->next = NULL;
				
					if(memcmp(prefix_step->prefix.u.prefix6.s6_addr,prefix_next->prefix.u.prefix6.s6_addr,128) < 0)
 					{
						rtadv_prefix_pool_hash_add(ifp,new_pool);
						memcpy(&send_prefix,&prefix_step->prefix.u.prefix6,sizeof(struct in6_addr));
						unsigned short prefix_nd = ( (prefix_step->prefix.u.prefix6.s6_addr[num-1] << 8) | prefix_step->prefix.u.prefix6.s6_addr[num]) + 1;
						prefix_step->prefix.u.prefix6.s6_addr[num-1] = (unsigned char)(prefix_nd >> 8); 
						prefix_step->prefix.u.prefix6.s6_addr[num] = (unsigned char)(prefix_nd) ;
						 zlog_info(" chenjun  rprefix->prefix.u.prefix6.s6_addr[num-1] %x\n",prefix_step->prefix.u.prefix6.s6_addr[num-1]);
						zlog_info(" chenjun  rprefix->prefix.u.prefix6.s6_addr[num] %x\n",prefix_step->prefix.u.prefix6.s6_addr[num]);
						zlog_info(" chenjun   prefix_step ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(prefix_step->prefix.u.prefix6.s6_addr));
						flag = 0;
					}
					else
					{
						free(new_pool);
						flag = 1;
					}
				}
				else
				{
					memcpy(&send_prefix,&new_prefix->prefix,sizeof(struct in6_addr));
					
				}
				memset(&zif->rtadv.dst,0,sizeof(struct in6_addr));
			  }
			  
			  zlog_info(" 	prefix_step	ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(prefix_step->prefix.u.prefix6.s6_addr));
			   zlog_info("  flag = %d\n",flag);
			   
			   zlog_info("  send prefix  ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(send_prefix.s6_addr));
		if(flag ==0)
		{
			  pinfo = (struct nd_opt_prefix_info *) (buf + len);
	
			  pinfo->nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
			  pinfo->nd_opt_pi_len = 4;
			  pinfo->nd_opt_pi_prefix_len = rprefix->prefix.prefixlen;
	
			  pinfo->nd_opt_pi_flags_reserved = 0;
			  if (rprefix->AdvOnLinkFlag)
			pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_ONLINK;
			  if (rprefix->AdvAutonomousFlag)
			pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_AUTO;
			  if (rprefix->AdvRouterAddressFlag)
			pinfo->nd_opt_pi_flags_reserved |= ND_OPT_PI_FLAG_RADDR;
	
			  pinfo->nd_opt_pi_valid_time = htonl (rprefix->AdvValidLifetime);
			  pinfo->nd_opt_pi_preferred_time = htonl (rprefix->AdvPreferredLifetime);
			  pinfo->nd_opt_pi_reserved2 = 0;
			  
			  	
			
			  memcpy (&pinfo->nd_opt_pi_prefix, &send_prefix,
				  sizeof (struct in6_addr));
			    zlog_info(" prefix_new ipv6 "NIP6QUAD_FMT"\n",NIP6QUAD(pinfo->nd_opt_pi_prefix.s6_addr));
	
#ifdef DEBUG
			  {
			u_char buf[INET6_ADDRSTRLEN];
			
			
			zlog_debug ("DEBUG %s", inet_ntop (AF_INET6, &pinfo->nd_opt_pi_prefix, 
					   buf, INET6_ADDRSTRLEN));
	
			  }
#endif /* DEBUG */
		
			  len += sizeof (struct nd_opt_prefix_info);
		    

		}
		

 }
 	

  /* Hardware address. */
  
#ifdef HAVE_SOCKADDR_DL

  sdl = &ifp->sdl;

  if (sdl != NULL && sdl->sdl_alen != 0)
    {
      buf[len++] = ND_OPT_SOURCE_LINKADDR;
      buf[len++] = (sdl->sdl_alen + 2) >> 3;

      memcpy (buf + len, LLADDR (sdl), sdl->sdl_alen);
      len += sdl->sdl_alen;
	  
    }
#else
  if (ifp->hw_addr_len != 0)
    {
    
      buf[len++] = ND_OPT_SOURCE_LINKADDR;
      buf[len++] = (ifp->hw_addr_len + 2) >> 3;

      memcpy (buf + len, ifp->hw_addr, ifp->hw_addr_len);
      len += ifp->hw_addr_len;
	  
    }
  
#endif /* HAVE_SOCKADDR_DL */

  msg.msg_name = (void *) &addr;
  msg.msg_namelen = sizeof (struct sockaddr_in6);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (void *) adata;
  msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
  msg.msg_flags = 0;
  iov.iov_base = buf;
  iov.iov_len = len;

  cmsgptr = ZCMSG_FIRSTHDR(&msg);
  cmsgptr->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
  cmsgptr->cmsg_level = IPPROTO_IPV6;
  cmsgptr->cmsg_type = IPV6_PKTINFO;

  pkt = (struct in6_pktinfo *) CMSG_DATA (cmsgptr);
  memset (&pkt->ipi6_addr, 0, sizeof (struct in6_addr));
  pkt->ipi6_ifindex = ifp->ifindex;

  ret = sendmsg (sock, &msg, 0);
  if (ret < 0)
    {
      zlog_err ("rtadv_send_packet: sendmsg %d (%s)\n",
		errno, safe_strerror(errno));
    }
}

static int
rtadv_timer (struct thread *thread)
{
  struct listnode *node, *nnode;
  struct interface *ifp;
  struct zebra_if *zif;
  int period;

  rtadv->ra_timer = NULL;
  if (rtadv->adv_msec_if_count == 0)
    {
      period = 1000; /* 1 s */
      rtadv_event (RTADV_TIMER, 1 /* 1 s */,NULL);
    } 
  else
    {
      period = 10; /* 10 ms */
      rtadv_event (RTADV_TIMER_MSEC, 10 /* 10 ms */,NULL);
    }

  for (ALL_LIST_ELEMENTS (iflist, node, nnode, ifp))
    {
      if (if_is_loopback (ifp))
	continue;

      zif = ifp->info;

      if (zif->rtadv.AdvSendAdvertisements)
	{ 
	  zif->rtadv.AdvIntervalTimer -= period;
	  if (zif->rtadv.AdvIntervalTimer <= 0)
	    {
	      zif->rtadv.AdvIntervalTimer = zif->rtadv.MaxRtrAdvInterval;
				if (if_is_operative (ifp))
					rtadv_send_packet (rtadv->sock, ifp);
	    }
	}
    }
  return 0;
}

/*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test. Before stop must send one packet out .*/
/*like timer*/
int
rtadv_stop_pre_send_one_packet (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zebra_if *zif;
  int period;

  rtadv->ra_timer = NULL;
  if (rtadv->adv_msec_if_count == 0)
    {
      period = 1000; /* 1 s */
    } 
  else
    {
      period = 10; /* 10 ms */
    }
  
  if(RTM_DEBUG_RTADV)
 	 zlog_info(" begin to send packet.\n");

      zif = ifp->info;
	  zlog_info("%s: AdvSendAdvertisements is %d .\n",__func__,zif->rtadv.AdvSendAdvertisements);
   /* if (zif->rtadv.AdvSendAdvertisements)*/
	{ 
	  zif->rtadv.AdvIntervalTimer -= period;
	  if (zif->rtadv.AdvIntervalTimer <= 0)
	    {
	      zif->rtadv.AdvIntervalTimer = zif->rtadv.MaxRtrAdvInterval;
				if (if_is_operative (ifp))
				{
				   if(RTM_DEBUG_RTADV)
					 zlog_info(" sending packet .\n");
					rtadv_send_packet (rtadv->sock, ifp);
				}
		}
    }
  
  if(RTM_DEBUG_RTADV)
  	zlog_info(" send packet over !\n");
  
  if_leave_all_router (rtadv->sock, ifp);
  rtadv_event(RTADV_STOP,0,NULL);/*to stop*/
  return 0;
}

/*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/
static void
rtadv_process_solicit (struct interface *ifp)
{
	long int value = 0;
	
	struct zebra_if *zif;

	zif = ifp->info;
	
	if(RTM_DEBUG_RTADV)
  	  zlog_info ("Router solicitation received on %s", ifp->name);
	
	this_time.tv_sec = quagga_gettimeofday_only_second(this_time);
	if(RTM_DEBUG_RTADV)
	  zlog_info("%s: send before, this time is (%u), last time is %u \n",__func__,this_time.tv_sec,zif->time_memory.tv_sec);
  /*value = this_time.tv_sec - last_time.tv_sec;*/
	value = this_time.tv_sec - zif->time_memory.tv_sec;

	if(RTM_DEBUG_RTADV)
	  zlog_info("%s : this_time - last_time = value(%ld) \n",__func__,value);
	value = labs(value);
	if(RTM_DEBUG_RTADV)
	  zlog_info("%s :  labs(value) = %ld .\n",__func__,value);
	if(value >= ND_ADVER_SEND_PACKET_INTERVAL)
	{
	   if (if_is_operative (ifp))
	   	{
		 rtadv_send_packet (rtadv->sock, ifp);
	  /*  last_time.tv_sec = quagga_gettimeofday_only_second(last_time);	*/	 
		 zif->time_memory.tv_sec = quagga_gettimeofday_only_second(zif->time_memory);
		 if(RTM_DEBUG_RTADV)
		   zlog_info("%s: send over,this time is %u , last time is (%u) .\n",__func__,this_time.tv_sec,zif->time_memory.tv_sec);
	   	}
	}
	else
	{
		if(RTM_DEBUG_RTADV)
		  zlog_info("%s: to set timer, this time is %u , last time is (%u). \n",__func__,this_time.tv_sec,zif->time_memory.tv_sec);
	  /* value = value*1000 + 500;*/
		rtadv->ra_timer_send = NULL;
		rtadv_event (RTADV_TIMER_MSEC_IF, 3000,ifp);/*ms*/
	}
		
}

/*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test. Used for set timer to send packet .*/
static int
rtadv_timer_send_packet (struct thread *thread)
{
  struct interface *ifp = THREAD_ARG(thread);
  
  rtadv->ra_timer_send = NULL;
  rtadv_process_solicit(ifp);

  return 0;
}

static void
rtadv_process_advert (void)
{
	if(RTM_DEBUG_RTADV)
		zlog_info ("Router advertisement received");
}

static void
rtadv_process_packet (u_char *buf, unsigned int len, unsigned int ifindex, int hoplimit,   struct sockaddr_in6 *from)
{
  struct icmp6_hdr *icmph;
  struct interface *ifp;
  struct zebra_if *zif;

  /* Interface search. */
  ifp = if_lookup_by_index (ifindex);
  if (ifp == NULL)
    {
      zlog_warn ("Unknown interface index: %d", ifindex);
      return;
    }

  if (if_is_loopback (ifp))
    return;

  /* Check interface configuration. */
  zif = ifp->info;
  if (! zif->rtadv.AdvSendAdvertisements)
    return;

  /* ICMP message length check. */
  if (len < sizeof (struct icmp6_hdr))
    {
      zlog_warn ("Invalid ICMPV6 packet length: %d", len);
      return;
    }

  icmph = (struct icmp6_hdr *) buf;

  /* ICMP message type check. */
  if (icmph->icmp6_type != ND_ROUTER_SOLICIT &&
      icmph->icmp6_type != ND_ROUTER_ADVERT)
    {
      zlog_warn ("Unwanted ICMPV6 message type: %d", icmph->icmp6_type);
      return;
    }

  /* Hoplimit check. */
  if (hoplimit >= 0 && hoplimit != 255)
    {
      zlog_warn ("Invalid hoplimit %d for router advertisement ICMP packet",
		 hoplimit);
      return;
    }

  /* Check ICMP message type. */
  if (icmph->icmp6_type == ND_ROUTER_SOLICIT)
  {
  	 memset(&zif->rtadv.dst, 0, sizeof(struct in6_addr));
	  memcpy(&zif->rtadv.dst, &from->sin6_addr.s6_addr, sizeof(from->sin6_addr.s6_addr));
	  
    	rtadv_process_solicit (ifp);
  }
  else if (icmph->icmp6_type == ND_ROUTER_ADVERT)
    rtadv_process_advert ();

  return;
}

static int
rtadv_read (struct thread *thread)
{
  int sock;
  int len;
  u_char buf[RTADV_MSG_SIZE];
  struct sockaddr_in6 from;
  unsigned int ifindex;
  int hoplimit = -1;

  sock = THREAD_FD (thread);
  
  if(sock<=0)
  {
	  zlog_warn ("In func %s get THREAD_FD error\n",__func__);
	  return -1;
  }
  rtadv->ra_read = NULL;

  /* Register myself. */
  rtadv_event (RTADV_READ, sock,NULL);

  len = rtadv_recv_packet (sock, buf, BUFSIZ, &from, &ifindex, &hoplimit);

  if (len < 0) 
    {
      zlog_warn ("router solicitation recv failed: %s.", safe_strerror (errno));
      return len;
    }

  rtadv_process_packet (buf, (unsigned)len, ifindex, hoplimit,&from);

  return 0;
}

static int
rtadv_make_socket (void)
{
  int sock;
  int ret;
  struct icmp6_filter filter;

  if ( zserv_privs.change (ZPRIVS_RAISE) )
       zlog_err ("rtadv_make_socket: could not raise privs, %s",
                  safe_strerror (errno) );
                  
  sock = socket (AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

  if ( zserv_privs.change (ZPRIVS_LOWER) )
       zlog_err ("rtadv_make_socket: could not lower privs, %s",
       			 safe_strerror (errno) );

  /* When we can't make ICMPV6 socket simply back.  Router
     advertisement feature will not be supported. */
  if (sock < 0)
    return -1;

  ret = setsockopt_ipv6_pktinfo (sock, 1);
  if (ret < 0)
  {
	/*CID 13538 (#1 of 6): Resource leak (RESOURCE_LEAK)
	8. leaked_handle: Handle variable "sock" going out of scope leaks the handle.
	 So add close . */
	close(sock);/*add*/
    return ret;
  	}
  ret = setsockopt_ipv6_multicast_loop (sock, 0);
  if (ret < 0)
  {
  /*CID 13538 (#2 of 6): Resource leak (RESOURCE_LEAK)
	10. leaked_handle: Handle variable "sock" going out of scope leaks the handle.*/
	close(sock);/*add*/
    return ret;
  	}
  ret = setsockopt_ipv6_unicast_hops (sock, 255);
  if (ret < 0)
  {
  /*CID 13538 (#3 of 6): Resource leak (RESOURCE_LEAK)
	12. leaked_handle: Handle variable "sock" going out of scope leaks the handle.*/
	close(sock);/*add*/
    return ret;
  	}
  ret = setsockopt_ipv6_multicast_hops (sock, 255);
  if (ret < 0)
  {
  /*CID 13538 (#4 of 6): Resource leak (RESOURCE_LEAK)
	14. leaked_handle: Handle variable "sock" going out of scope leaks the handle.*/
	close(sock);/*add*/
    return ret;
  	}
  ret = setsockopt_ipv6_hoplimit (sock, 1);
  if (ret < 0)
  {
  /*CID 13538 (#5 of 6): Resource leak (RESOURCE_LEAK)
	16. leaked_handle: Handle variable "sock" going out of scope leaks the handle.*/
	close(sock);/*add*/
    return ret;
  	}

  ICMP6_FILTER_SETBLOCKALL(&filter);
  ICMP6_FILTER_SETPASS (ND_ROUTER_SOLICIT, &filter);
  ICMP6_FILTER_SETPASS (ND_ROUTER_ADVERT, &filter);

  ret = setsockopt (sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
		    sizeof (struct icmp6_filter));
  if (ret < 0)
    {
    /*CID 13538 (#6 of 6): Resource leak (RESOURCE_LEAK)
	   18. leaked_handle: Handle variable "sock" going out of scope leaks the handle.*/
      zlog_info ("ICMP6_FILTER set fail: %s", safe_strerror (errno));
	  close(sock);/*add*/
      return ret;
    }

  return sock;
}

static struct rtadv_prefix *
rtadv_prefix_new ()
{
  struct rtadv_prefix *new;

  new = XMALLOC (MTYPE_RTADV_PREFIX, sizeof (struct rtadv_prefix));
  memset (new, 0, sizeof (struct rtadv_prefix));

  return new;
}

static void
rtadv_prefix_free (struct rtadv_prefix *rtadv_prefix)
{
  XFREE (MTYPE_RTADV_PREFIX, rtadv_prefix);
}

static struct rtadv_prefix *
rtadv_prefix_lookup (struct list *rplist, struct prefix *p)
{
  struct listnode *node;
  struct rtadv_prefix *rprefix;

  for (ALL_LIST_ELEMENTS_RO (rplist, node, rprefix))
    if (prefix_same (&rprefix->prefix, p))
      return rprefix;
  return NULL;
}

static struct rtadv_prefix *
rtadv_prefix_get (struct list *rplist, struct prefix *p)
{
  struct rtadv_prefix *rprefix;
  
  rprefix = rtadv_prefix_lookup (rplist, p);
  if (rprefix)
    return rprefix;

  rprefix = rtadv_prefix_new ();
  memcpy (&rprefix->prefix, p, sizeof (struct prefix));
  listnode_add (rplist, rprefix);

  return rprefix;
}

static struct rtadv_prefix *
rtadv_prefix_pool_get (struct list *rplist, struct prefix *p)
{
  struct rtadv_prefix *rprefix;

  rprefix = rtadv_prefix_new ();
  memcpy (&rprefix->prefix, p, sizeof (struct prefix));
  listnode_add (rplist, rprefix);

  return rprefix;
}


static void
rtadv_prefix_set (struct zebra_if *zif, struct rtadv_prefix *rp)
{
  struct rtadv_prefix *rprefix;
  
  rprefix = rtadv_prefix_get (zif->rtadv.AdvPrefixList, &rp->prefix);

  /* Set parameters. */
  rprefix->AdvValidLifetime = rp->AdvValidLifetime;
  rprefix->AdvPreferredLifetime = rp->AdvPreferredLifetime;
  rprefix->AdvOnLinkFlag = rp->AdvOnLinkFlag;
  rprefix->AdvAutonomousFlag = rp->AdvAutonomousFlag;
  rprefix->AdvRouterAddressFlag = rp->AdvRouterAddressFlag;
}

static void
rtadv_prefix_pool_set(struct zebra_if *zif, struct rtadv_prefix *rp)
{
  struct rtadv_prefix *rprefix;
  
  rprefix = rtadv_prefix_pool_get (zif->rtadv.pool, &rp->prefix);
  
  rprefix->AdvValidLifetime = rp->AdvValidLifetime;
  rprefix->AdvPreferredLifetime = rp->AdvPreferredLifetime;
  rprefix->AdvOnLinkFlag = rp->AdvOnLinkFlag;
  rprefix->AdvAutonomousFlag = rp->AdvAutonomousFlag;
  rprefix->AdvRouterAddressFlag = rp->AdvRouterAddressFlag;
}


static int
rtadv_prefix_reset (struct zebra_if *zif, struct rtadv_prefix *rp)
{
  struct rtadv_prefix *rprefix;
  
  rprefix = rtadv_prefix_lookup (zif->rtadv.AdvPrefixList, &rp->prefix);
  if (rprefix != NULL)
    {
      listnode_delete (zif->rtadv.AdvPrefixList, (void *) rprefix);
      rtadv_prefix_free (rprefix);
      return 1;
    }
  else
    return 0;
}

/*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/

DEFUN (ipv6_nd_ra_link_mtu,
       ipv6_nd_ra_link_mtu_cmd,
       "ipv6 nd ra-link-mtu <0-1500>",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement link mtu \n"
       "Router Advertisement link mtu value between <0-1500>\n")
{
  struct interface *ifp;
  struct zebra_if *zif;
  int value = 0;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  value = atoi (argv[0]);

  if (value < 0 || value > 1500)
    {
      vty_out (vty, "Invalid Router Advertisement link MTU between <0-1500>%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zif->rtadv.AdvLinkMTU = value;
  if(value > 0)
    zif->rtadv.AdvLinkMtuOption = 1;
  else
  	zif->rtadv.AdvLinkMtuOption = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);
  
  return CMD_SUCCESS;
}



DEFUN (no_ipv6_nd_ra_link_mtu,
       no_ipv6_nd_ra_link_mtu_cmd,
       "no ipv6 nd ra-link-mtu",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement link mtu \n"
       "Router Advertisement link mtu value between \n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvLinkMTU = 0;
  zif->rtadv.AdvLinkMtuOption = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);
  
  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_ra_retrans_time,
       ipv6_nd_ra_retrans_time_cmd,
       "ipv6 nd ra-retrans-time SECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement retrans time \n"
       "Router Advertisement time interval in seconds\n")
{
  u_int32_t interval ;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

 /* interval = atoi (argv[0]);*/
 interval = (u_int32_t) strtoll (argv[0],(char **)NULL, 10);

  if (interval < 0)
    {
      vty_out (vty, "Invalid Router min Advertisement Interval%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

/* convert to milliseconds */
  interval = interval * 1000; 
  zif->rtadv.AdvRetransTimer = interval;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);
  
  return CMD_SUCCESS;
}
//////////////////////////////////////////////////////////////
DEFUN (ipv6_nd_ra_retrans_time_ms,
       ipv6_nd_ra_retrans_time_ms_cmd,
       "ipv6 nd ra-retrans-time msec MSECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement retrans time \n"
       "Router Advertisement time interval in seconds\n")
{
  u_int32_t interval = 0;
  struct interface *ifp;
  struct zebra_if *zif;  
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

/*  interval = atoi (argv[0]);*/
  interval = (u_int32_t) strtoll (argv[0],(char **)NULL, 10);

  if (interval < 0)
    {
      vty_out (vty, "Invalid Router min Advertisement Interval%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

 /* convert to milliseconds */
 /* interval = interval * 1000; */
  zif->rtadv.AdvRetransTimer = interval;

  /*To change the system file :/proc/sys/net/ipv6/neigh/xxx/retrans_time_ms .*/
  if (zif->rtadv.AdvRetransTimer > 0)
	{
	  snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.retrans_time_ms=%u", ifp->name, zif->rtadv.AdvRetransTimer);
	  cmd[BUFFER_LEN - 1] = '\0';
	  ret = system(cmd);
	  ret = WEXITSTATUS(ret);
				  
	  if(CMD_SUCCESS != ret )
	  {
		  vty_out(vty,"Modify Interface RetransTime fail%s", VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	}  
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);
  
  return CMD_SUCCESS;
}

//////////////////////////////////////////////////////
DEFUN (no_ipv6_nd_ra_retrans_time_ms,
       no_ipv6_nd_ra_retrans_time_ms_cmd,
       "no ipv6 nd ra-retrans-time",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement retrans time \n"
       "Router Advertisement time interval in seconds\n")
{
  u_int32_t interval = 0;
  struct interface *ifp;
  struct zebra_if *zif;
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvRetransTimer = 1000;/*default : 1000ms*/

  /*To change the system file :/proc/sys/net/ipv6/neigh/xxx/retrans_time_ms .*/
  if (zif->rtadv.AdvRetransTimer > 0)
  {
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.retrans_time_ms=%d", ifp->name, zif->rtadv.AdvRetransTimer);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				
	if(CMD_SUCCESS != ret )
	{
		vty_out(vty,"Modify Interface RetransTime fail%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
  } 
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);
  
  return CMD_SUCCESS;
}

#if 0
DEFUN (no_ipv6_nd_ra_retrans_time,
       no_ipv6_nd_ra_retrans_time_cmd,
       "no ipv6 nd ra-retrans-time",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement retrans time \n"
       "Router Advertisement time interval in seconds\n")
{
  u_int32_t interval;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvRetransTimer = 0;
  
  return CMD_SUCCESS;
}
#endif
//////////////////////////////////////////////////
DEFUN (ipv6_nd_adv_cur_hop_limit,
       ipv6_nd_adv_cur_hop_limit_cmd,
       "ipv6 nd adv-curhoplimt <0-255>",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement current hop limit\n"
       "Router Advertisement hop limit betwween <0-255>\n")
{
  int hoplimt;
  struct interface *ifp;
  struct zebra_if *zif;
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}

#endif

  zif = ifp->info;

  hoplimt = atoi (argv[0]);

  if (hoplimt < 0 || hoplimt > 255)
    {
      vty_out (vty, "Invalid AdvCurHopLimit between <0--255> .%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  zif->rtadv.AdvCurHopLimit = hoplimt;
  /*To change the system file :/proc/sys/net/ipv6/conf/xxxx/hop_limit .*/
  if (zif->rtadv.AdvCurHopLimit > 0)
  {
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.conf.%s.hop_limit=%d", ifp->name, zif->rtadv.AdvCurHopLimit);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				
	if(CMD_SUCCESS != ret )
	{
		vty_out(vty,"Modify Interface Hoplimit fail%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
  }
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_adv_cur_hop_limit,
       no_ipv6_nd_adv_cur_hop_limit_cmd,
       "no ipv6 nd adv-curhoplimt",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement current hop limit\n"
       "Router Advertisement hop limit betwween <0-255>\n")
{
  int hoplimt;
  struct interface *ifp;
  struct zebra_if *zif;
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}

#endif

  zif = ifp->info;
  
  zif->rtadv.AdvCurHopLimit = 64;/*default : 64*/
  
  /*To change the system file :/proc/sys/net/ipv6/conf/xxxx/hop_limit .*/
  if (zif->rtadv.AdvCurHopLimit > 0)
  {
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.conf.%s.hop_limit=%d", ifp->name, zif->rtadv.AdvCurHopLimit);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				
	if(CMD_SUCCESS != ret )
	{
		vty_out(vty,"Modify Interface Hoplimit fail%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
  }
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}


DEFUN (ipv6_nd_ra_min_interval,
       ipv6_nd_ra_min_interval_cmd,
       "ipv6 nd ra-min-interval SECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in seconds\n")
{
  int interval;
  struct interface *ifp;
  struct zebra_if *zif;
  int value = 0;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  interval = atoi (argv[0]);

  if (interval <= 0)
    {
      vty_out (vty, "Invalid Router min Advertisement Interval%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
#if 0
  if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;
	
  /* convert to milliseconds */
  interval = interval * 1000; 
	
  zif->rtadv.MaxRtrAdvInterval = interval;
  zif->rtadv.MinRtrAdvInterval = 0.33 * interval;
  zif->rtadv.AdvIntervalTimer = 0;
#else
/* convert to milliseconds */
  interval = interval * 1000; 
  value = 0.75 * zif->rtadv.MaxRtrAdvInterval;
  
  if(interval < 3000 || interval > value)/*3s -- 0.75*max */
  	{
  		vty_out(vty,"The argument of min Advertisement Interval between (3--0.75*max).%s",VTY_NEWLINE);
		return CMD_WARNING;
  	}
 	zif->rtadv.MinRtrAdvInterval = interval;
	zif->rtadv.AdvIntervalTimer = 0;

#endif
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_ra_min_interval,
       no_ipv6_nd_ra_min_interval_cmd,
       "no ipv6 nd ra-min-interval",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;
#if 0
  if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;
  
  zif->rtadv.MaxRtrAdvInterval = RTADV_MAX_RTR_ADV_INTERVAL;
  zif->rtadv.MinRtrAdvInterval = RTADV_MIN_RTR_ADV_INTERVAL;
  zif->rtadv.AdvIntervalTimer = zif->rtadv.MaxRtrAdvInterval;
#else
 /* if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;
 */ 
 /* zif->rtadv.MaxRtrAdvInterval = RTADV_MAX_RTR_ADV_INTERVAL;*/
  zif->rtadv.MinRtrAdvInterval = RTADV_MIN_RTR_ADV_INTERVAL;
  zif->rtadv.AdvIntervalTimer = zif->rtadv.MaxRtrAdvInterval;

#endif
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}
/**gujd : Add for IPv6 Ready Test.----end---**/

DEFUN (ipv6_nd_suppress_ra,
       ipv6_nd_suppress_ra_cmd,
       "ipv6 nd suppress-ra",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Suppress Router Advertisement\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

	
#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}

#endif
  zif = ifp->info;

  if (if_is_loopback (ifp))
    {
      vty_out (vty, "Invalid interface%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/
  if(RTM_DEBUG_RTADV)
    zlog_info("%s: line %d, AdvSendAdvertisements is %d .\n",__func__,__LINE__,zif->rtadv.AdvSendAdvertisements);
  if (zif->rtadv.AdvSendAdvertisements)
    {
      zif->rtadv.AdvSendAdvertisements = 0;
      zif->rtadv.AdvIntervalTimer = 0;
    /*  rtadv->adv_if_count--;*/
      rtadv->adv_if_count = 0;
#if 1 
	 /*gujd: 2013-05-14, pm 4:14 . Add code for ipv6 nd suppress ra (enable) for Distribute System.*/
	 if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	 {
	   tipc_master_interface_nd_suppress_ra_deal(ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE,ifp);
	   }
 #endif

 	/*  if_leave_all_router (rtadv->sock, ifp);*/
	if(RTM_DEBUG_RTADV)
	 zlog_info("%s:line %d, adv_if_count is %d .\n",__func__,__LINE__,rtadv->adv_if_count);
      if (rtadv->adv_if_count == 0)
      {
     	/* zif->rtadv.AdvDefaultLifetime = 0;*/
		if(RTM_DEBUG_RTADV)
		 zlog_info("to RTADV_STOP_PRE \n");
		/*rtadv_event (RTADV_STOP_PRE, 0,NULL);*/
		rtadv_stop_pre_send_one_packet(ifp);
      	}
	 #if 0 
	  /*gujd: 2013-05-14, pm 4:14 . Add code for ipv6 nd suppress ra (enable) for Distribute System.*/
	  if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	  {
	  	tipc_master_interface_nd_suppress_ra_deal(ZEBRA_INTERFACE_ND_SUPPRESS_RA_ENABLE,ifp);
	  	}
	  #endif
    }

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_suppress_ra,
       no_ipv6_nd_suppress_ra_cmd,
       "no ipv6 nd suppress-ra",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Suppress Router Advertisement\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}

#endif

  zif = ifp->info;

  if (if_is_loopback (ifp))
    {
      vty_out (vty, "Invalid interface%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  if(RTM_DEBUG_RTADV)
  	zlog_info("%s: AdvSendAdvertisements is %d .\n",__func__,zif->rtadv.AdvSendAdvertisements);

  if (! zif->rtadv.AdvSendAdvertisements)
    {
      zif->rtadv.AdvSendAdvertisements = 1;
      zif->rtadv.AdvIntervalTimer = 0;
   /*  rtadv->adv_if_count++;*/
	  rtadv->adv_if_count = 1;/*gujd : 2012-05-29,pm 9:53 . Change for IPv6 Ready Test.*/

      if_join_all_router (rtadv->sock, ifp);
	  if(RTM_DEBUG_RTADV)
	 	 zlog_info("%s: adv_if_count is %d .\n",__func__,rtadv->adv_if_count);
      if (rtadv->adv_if_count == 1)
	    rtadv_event (RTADV_START, rtadv->sock,NULL);

	  /*gujd: 2013-05-14, pm 4:14 . Add code for ipv6 nd suppress ra (disable) for Distribute System.*/
	  if(product && product->board_type == BOARD_IS_ACTIVE_MASTER)
	  {
	  	tipc_master_interface_nd_suppress_ra_deal(ZEBRA_INTERFACE_ND_SUPPRESS_RA_DISABLE,ifp);
	  	}
    }

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_ra_interval_msec,
       ipv6_nd_ra_interval_msec_cmd,
       "ipv6 nd ra-interval msec MILLISECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in milliseconds\n")
{
  int interval;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}

#endif

  zif = ifp->info;

  interval = atoi (argv[0]);

  if (interval <= 0 || interval > 1800000)
    {
      vty_out (vty, "Invalid Router Advertisement Interval%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;///////////////////////////

  if (interval % 1000)
    rtadv->adv_msec_if_count++;
  
  zif->rtadv.MaxRtrAdvInterval = interval;
  zif->rtadv.MinRtrAdvInterval = 0.33 * interval;
  /*zif->rtadv.AdvIntervalTimer = 0;*//*gujd : change for IPv6 Ready Test*/

  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_ra_interval,
       ipv6_nd_ra_interval_cmd,
       "ipv6 nd ra-interval SECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n"
       "Router Advertisement interval in seconds\n")
{
  int interval;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  interval = atoi (argv[0]);

  if (interval <= 0 || interval >1800)
    {
      vty_out (vty, "Invalid Router Advertisement Interval%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;
	
  /* convert to milliseconds */
  interval = interval * 1000; 
	
  zif->rtadv.MaxRtrAdvInterval = interval;
  zif->rtadv.MinRtrAdvInterval = 0.33 * interval;
  /*zif->rtadv.AdvIntervalTimer = 0;*//*gujd : change for IPv6 Ready Test*/
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_ra_interval,
       no_ipv6_nd_ra_interval_cmd,
       "no ipv6 nd ra-interval",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router Advertisement interval\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  if (zif->rtadv.MaxRtrAdvInterval % 1000)
    rtadv->adv_msec_if_count--;
  
  zif->rtadv.MaxRtrAdvInterval = RTADV_MAX_RTR_ADV_INTERVAL;
  zif->rtadv.MinRtrAdvInterval = RTADV_MIN_RTR_ADV_INTERVAL;
  /*zif->rtadv.AdvIntervalTimer = zif->rtadv.MaxRtrAdvInterval;*//*gujd : change for IPv6 Ready Test*/
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_ra_lifetime,
       ipv6_nd_ra_lifetime_cmd,
       "ipv6 nd ra-lifetime SECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router lifetime\n"
       "Router lifetime in seconds\n")
{
  int lifetime;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  lifetime = atoi (argv[0]);

 /* if (lifetime < 0 || lifetime > 0xffff)*//*gujd : change for IPv6 Ready Test*/
  if (lifetime < 0 || lifetime > 9000)
    {
      vty_out (vty, "Invalid Router Lifetime%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zif->rtadv.AdvDefaultLifetime = lifetime;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_ra_lifetime,
       no_ipv6_nd_ra_lifetime_cmd,
       "no ipv6 nd ra-lifetime",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Router lifetime\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvDefaultLifetime = RTADV_ADV_DEFAULT_LIFETIME;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_reachable_time,
       ipv6_nd_reachable_time_cmd,
       "ipv6 nd reachable-time MILLISECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Reachable time\n"
       "Reachable time in milliseconds\n")
{
  u_int32_t rtime;
  struct interface *ifp;
  struct zebra_if *zif;
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  rtime = (u_int32_t) atol (argv[0]);

  if (rtime > RTADV_MAX_REACHABLE_TIME)
    {
      vty_out (vty, "Invalid Reachable time%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zif->rtadv.AdvReachableTime = rtime;
  
  /*To change the system file :/proc/sys/net/ipv6/neigh/xxx/base_reachable_time_ms .*/
  if (zif->rtadv.AdvReachableTime > 0 && zif->rtadv.AdvReachableTime <= 0x7fffffff)
	{
	  snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.base_reachable_time_ms=%d", ifp->name, zif->rtadv.AdvReachableTime);
	  cmd[BUFFER_LEN - 1] = '\0';
	  ret = system(cmd);
	  ret = WEXITSTATUS(ret);
				  
	  if(CMD_SUCCESS != ret )
	  {
		  vty_out(vty,"Modify Interface Reachable time fail%s", VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	}
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_reachable_time,
       no_ipv6_nd_reachable_time_cmd,
       "no ipv6 nd reachable-time",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Reachable time\n")
{
  struct interface *ifp;
  struct zebra_if *zif;
  unsigned char cmd[BUFFER_LEN] = {0};
  int ret=CMD_SUCCESS;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

 /* zif->rtadv.AdvReachableTime = 0;*/
  zif->rtadv.AdvReachableTime = 30000;/*default : 30000ms = 30s*/

 /*To change the system file :/proc/sys/net/ipv6/neigh/xxx/base_reachable_time_ms .*/
  if (zif->rtadv.AdvReachableTime > 0)
  {
  	snprintf(cmd,BUFFER_LEN,"sudo sysctl -w net.ipv6.neigh.%s.base_reachable_time_ms=%d", ifp->name, zif->rtadv.AdvReachableTime);
	cmd[BUFFER_LEN - 1] = '\0';
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
				
	if(CMD_SUCCESS != ret)
	{
		vty_out(vty,"Modify Interface Reachable time fail%s", VTY_NEWLINE);
	   return CMD_WARNING;
	}
  }
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_homeagent_preference,
       ipv6_nd_homeagent_preference_cmd,
       "ipv6 nd home-agent-preference PREFERENCE",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent preference\n"
       "Home Agent preference value 0..65535\n")
{
  u_int32_t hapref;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  hapref = (u_int32_t) atol (argv[0]);

  if (hapref > 65535)
    {
      vty_out (vty, "Invalid Home Agent preference%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zif->rtadv.HomeAgentPreference = hapref;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_homeagent_preference,
       no_ipv6_nd_homeagent_preference_cmd,
       "no ipv6 nd home-agent-preference",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent preference\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.HomeAgentPreference = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_homeagent_lifetime,
       ipv6_nd_homeagent_lifetime_cmd,
       "ipv6 nd home-agent-lifetime SECONDS",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent lifetime\n"
       "Home Agent lifetime in seconds\n")
{
  u_int32_t ha_ltime;
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  ha_ltime = (u_int32_t) atol (argv[0]);

  if (ha_ltime > RTADV_MAX_HALIFETIME)
    {
      vty_out (vty, "Invalid Home Agent Lifetime time%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  zif->rtadv.HomeAgentLifetime = ha_ltime;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_homeagent_lifetime,
       no_ipv6_nd_homeagent_lifetime_cmd,
       "no ipv6 nd home-agent-lifetime",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent lifetime\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.HomeAgentLifetime = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_managed_config_flag,
       ipv6_nd_managed_config_flag_cmd,
       "ipv6 nd managed-config-flag",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Managed address configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvManagedFlag = 1;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_managed_config_flag,
       no_ipv6_nd_managed_config_flag_cmd,
       "no ipv6 nd managed-config-flag",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Managed address configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvManagedFlag = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_homeagent_config_flag,
       ipv6_nd_homeagent_config_flag_cmd,
       "ipv6 nd home-agent-config-flag",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvHomeAgentFlag = 1;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_homeagent_config_flag,
       no_ipv6_nd_homeagent_config_flag_cmd,
       "no ipv6 nd home-agent-config-flag",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Home Agent configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvHomeAgentFlag = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_adv_interval_config_option,
       ipv6_nd_adv_interval_config_option_cmd,
       "ipv6 nd adv-interval-option",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertisement Interval Option\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvIntervalOption = 1;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_adv_interval_config_option,
       no_ipv6_nd_adv_interval_config_option_cmd,
       "no ipv6 nd adv-interval-option",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Advertisement Interval Option\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvIntervalOption = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (ipv6_nd_other_config_flag,
       ipv6_nd_other_config_flag_cmd,
       "ipv6 nd other-config-flag",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Other statefull configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvOtherConfigFlag = 1;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_other_config_flag,
       no_ipv6_nd_other_config_flag_cmd,
       "no ipv6 nd other-config-flag",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Other statefull configuration flag\n")
{
  struct interface *ifp;
  struct zebra_if *zif;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif

  zif = ifp->info;

  zif->rtadv.AdvOtherConfigFlag = 0;
  
  /*gujd : 2013-05-13, Add IPv6 rtadv info update for other boards.*/
  tipc_master_rtadv_nd_info_update(ifp);

  return CMD_SUCCESS;
}


DEFUN (ipv6_nd_prefix_pool,
       ipv6_nd_prefix_pool_cmd,
       "ipv6 nd prefix pool X:X::X:X/M  X:X::X:X/M",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "prefix pool"
       "IPv6 prefix\n"
       "IPv6 prefix\n"
       )
{
  int i;
  int ret;
  int cursor = 1;
  struct interface *ifp;
    struct rtadv_prefix rp_start,rp_end;
  struct zebra_if *zebra_if;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif
  zebra_if = ifp->info;
  ret = str2prefix_ipv6 (argv[0], (struct prefix_ipv6 *) &rp_start.prefix);
    vty_out (vty, "ipv6start "NIP6QUAD_FMT"\n",NIP6QUAD(rp_start.prefix.u.prefix6.s6_addr));
  if (!ret)
    {
      vty_out (vty, "Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  rp_start.AdvOnLinkFlag = 1;
  rp_start.AdvAutonomousFlag = 1;
  rp_start.AdvRouterAddressFlag = 0;
  rp_start.AdvValidLifetime = RTADV_VALID_LIFETIME;
  rp_start.AdvPreferredLifetime = RTADV_PREFERRED_LIFETIME;

  ret = str2prefix_ipv6 (argv[1], (struct prefix_ipv6 *) &rp_end.prefix);
    vty_out (vty, "ipv6end "NIP6QUAD_FMT"\n",NIP6QUAD(rp_end.prefix.u.prefix6.s6_addr));
  if (!ret)
    {
      vty_out (vty, "Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  rp_end.AdvOnLinkFlag = 1;
  rp_end.AdvAutonomousFlag = 1;
  rp_end.AdvRouterAddressFlag = 0;
  rp_end.AdvValidLifetime = RTADV_VALID_LIFETIME;
  rp_end.AdvPreferredLifetime = RTADV_PREFERRED_LIFETIME;

  
  if ( memcmp(rp_start.prefix.u.prefix6.s6_addr,rp_end.prefix.u.prefix6.s6_addr,128) >=0)
  {
		vty_out (vty, "add ipv6 prefix pool fail\n");
			   return CMD_SUCCESS;
  }

if (zebra_if->rtadv.pool->head != NULL)
{
	    struct rtadv_prefix *next;
	 while(zebra_if->rtadv.pool->head != NULL)
	 {
		next = zebra_if->rtadv.pool->head->next;
		 free (zebra_if->rtadv.pool->head);
		  zebra_if->rtadv.pool->head = NULL;
		   zebra_if->rtadv.pool->head = next;
		 
	 }
}
 
   rtadv_prefix_pool_set (zebra_if, &rp_start);
   rtadv_prefix_pool_set (zebra_if, &rp_end);
    rtadv_prefix_pool_set (zebra_if, &rp_start);
  zebra_if->rtadv_prefix_pool = (struct ipv6_pool **)  malloc(sizeof(struct ipv6_pool *)*256*256);
  memset(zebra_if->rtadv_prefix_pool ,0,sizeof(struct ipv6_pool *)*256*256);
   tipc_master_interface_nd_prefix_pool_update(ZEBRA_INTERFACE_ND_PREFIX_POOL_ADD,ifp,&rp_start,&rp_end,argv[0],argv[1]);
  

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_nd_prefix_pool,
       no_ipv6_nd_prefix_pool_cmd,
       "no ipv6 nd prefix pool ",
        NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "prefix pool"
             )
{
  int i;
  int ret;
  int cursor = 1;
  struct interface *ifp;
  struct zebra_if *zebra_if;
  struct ipv6_pool *s,*p;
  //struct rtadv_prefix rp,rplast;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif
  zebra_if = ifp->info;
  if (zebra_if->rtadv.pool->head != NULL)
  {
		  struct rtadv_prefix *next;
	   while(zebra_if->rtadv.pool->head != NULL)
	   {
		  next = zebra_if->rtadv.pool->head->next;
		   free (zebra_if->rtadv.pool->head);
			zebra_if->rtadv.pool->head = NULL;
			 zebra_if->rtadv.pool->head = next;
		   
	   	}
  	}
  for(i =0; i <= 256*256; i++)
  {	
  	if ( zebra_if->rtadv_prefix_pool[i] != NULL)
  	{
  		s =  zebra_if->rtadv_prefix_pool[i];
		 zebra_if ->rtadv_prefix_pool[i] = NULL;
		while (s != NULL)
		{
			p = s;
			s = s->next;
			free(p);
		}
	}
	else
		continue;
  }
  free(zebra_if->rtadv_prefix_pool );
  zebra_if->rtadv_prefix_pool = NULL;
   tipc_master_interface_nd_prefix_pool_update(ZEBRA_INTERFACE_ND_PREFIX_POOL_DELETE,ifp,NULL,NULL,NULL,NULL);
  

  return CMD_SUCCESS;
}


/////////////////////////////////////other deal////////////////////////////////////
DEFUN (ipv6_nd_prefix,
       ipv6_nd_prefix_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (off-link|) (no-autoconfig|) (router-address|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for onlink determination\n"
       "Do not use prefix for autoconfiguration\n"
       "Set Router Address flag\n")
{
  int i;
  int ret;
  int cursor = 1;
  struct interface *ifp;
  struct zebra_if *zebra_if;
  struct rtadv_prefix rp;

#if 0
			ifp = (struct interface *) vty->index;
#else
			ifp = if_get_by_vty_index(vty->index);
			if(NULL == ifp)
			{
				vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
				return CMD_WARNING;
			}
#endif
  zebra_if = ifp->info;

  ret = str2prefix_ipv6 (argv[0], (struct prefix_ipv6 *) &rp.prefix);
  vty_out(vty,"prefixlen = %d\n",rp.prefix.prefixlen);
  if (!ret)
    {
      vty_out (vty, "Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  rp.AdvOnLinkFlag = 1;
  rp.AdvAutonomousFlag = 1;
  rp.AdvRouterAddressFlag = 0;
  rp.AdvValidLifetime = RTADV_VALID_LIFETIME;
  rp.AdvPreferredLifetime = RTADV_PREFERRED_LIFETIME;

  if (argc > 1)
    {
      if ((isdigit(argv[1][0])) || strncmp (argv[1], "i", 1) == 0)
	{
	  if ( strncmp (argv[1], "i", 1) == 0)
	    rp.AdvValidLifetime = UINT32_MAX;
	  else
	    rp.AdvValidLifetime = (u_int32_t) strtoll (argv[1],
		(char **)NULL, 10);
      
	  if ( strncmp (argv[2], "i", 1) == 0)
	    rp.AdvPreferredLifetime = UINT32_MAX;
	  else
	    rp.AdvPreferredLifetime = (u_int32_t) strtoll (argv[2],
		(char **)NULL, 10);

	  if (rp.AdvPreferredLifetime > rp.AdvValidLifetime)
	    {
	      vty_out (vty, "Invalid preferred lifetime%s", VTY_NEWLINE);
	      return CMD_WARNING;
	    }
	  cursor = cursor + 2;
	}
      if (argc > cursor)
	{
	  for (i = cursor; i < argc; i++)
	    {
	      if (strncmp (argv[i], "of", 2) == 0)
		rp.AdvOnLinkFlag = 0;
	      if (strncmp (argv[i], "no", 2) == 0)
		rp.AdvAutonomousFlag = 0;
	      if (strncmp (argv[i], "ro", 2) == 0)
		rp.AdvRouterAddressFlag = 1;
	    }
	}
    }

  rtadv_prefix_set (zebra_if, &rp);

  /*gujd : 2013-05-13, Add IPv6 interface nd prefix update for other boards.
      Use the parameters of argv[0] and rp .*/
   tipc_master_interface_nd_prefix_update(ZEBRA_INTERFACE_ND_PREFIX_ADD,ifp,&rp,argv[0]);
  

  return CMD_SUCCESS;
}

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_nortaddr_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (off-link|) (no-autoconfig|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for onlink determination\n"
       "Do not use prefix for autoconfiguration\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_rev_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (no-autoconfig|) (off-link|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for onlink determination\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_rev_rtaddr_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (no-autoconfig|) (off-link|) (router-address|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for onlink determination\n"
       "Set Router Address flag\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_noauto_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (no-autoconfig|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for autoconfiguration")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_offlink_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (off-link|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Do not use prefix for onlink determination\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_rtaddr_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite) (router-address|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n"
       "Set Router Address flag\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_val_cmd,
       "ipv6 nd prefix X:X::X:X/M (<0-4294967295>|infinite) "
       "(<0-4294967295>|infinite)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Valid lifetime in seconds\n"
       "Infinite valid lifetime\n"
       "Preferred lifetime in seconds\n"
       "Infinite preferred lifetime\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_noval_cmd,
       "ipv6 nd prefix X:X::X:X/M (no-autoconfig|) (off-link|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Do not use prefix for autoconfiguration\n"
       "Do not use prefix for onlink determination\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_noval_rev_cmd,
       "ipv6 nd prefix X:X::X:X/M (off-link|) (no-autoconfig|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Do not use prefix for onlink determination\n"
       "Do not use prefix for autoconfiguration\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_noval_noauto_cmd,
       "ipv6 nd prefix X:X::X:X/M (no-autoconfig|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Do not use prefix for autoconfiguration\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_noval_offlink_cmd,
       "ipv6 nd prefix X:X::X:X/M (off-link|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Do not use prefix for onlink determination\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_noval_rtaddr_cmd,
       "ipv6 nd prefix X:X::X:X/M (router-address|)",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n"
       "Set Router Address flag\n")

ALIAS (ipv6_nd_prefix,
       ipv6_nd_prefix_prefix_cmd,
       "ipv6 nd prefix X:X::X:X/M",
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n")

DEFUN (no_ipv6_nd_prefix,
       no_ipv6_nd_prefix_cmd,
       "no ipv6 nd prefix IPV6PREFIX",
       NO_STR
       "Interface IPv6 config commands\n"
       "Neighbor discovery\n"
       "Prefix information\n"
       "IPv6 prefix\n")
{
  int ret;
  struct interface *ifp;
  struct zebra_if *zebra_if;
  struct rtadv_prefix rp;

#if 0
				ifp = (struct interface *) vty->index;
#else
				ifp = if_get_by_vty_index(vty->index);
				if(NULL == ifp)
				{
					vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
					return CMD_WARNING;
				}
#endif
  zebra_if = ifp->info;

  ret = str2prefix_ipv6 (argv[0], (struct prefix_ipv6 *) &rp.prefix);
  if (!ret)
    {
      vty_out (vty, "Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /*gujd : 2013-05-13, Add IPv6 interface nd prefix update for other boards.
      Use the parameters of argv[0] and rp .*/
   tipc_master_interface_nd_prefix_update(ZEBRA_INTERFACE_ND_PREFIX_DELETE,ifp,&rp,argv[0]);

  ret = rtadv_prefix_reset (zebra_if, &rp);
  if (!ret)
    {
      vty_out (vty, "Non-exist IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return CMD_SUCCESS;
}
/* Write configuration about router advertisement. */
void
rtadv_config_write (struct vty *vty, struct interface *ifp)
{
  struct zebra_if *zif;
  struct listnode *node;
  struct rtadv_prefix *rprefix,*prefix_start,*prefix_end;
  u_char buf[INET6_ADDRSTRLEN];
    u_char buf1[INET6_ADDRSTRLEN];
  int interval;

  if (! rtadv)
    return;

  zif = ifp->info;

  if (! if_is_loopback (ifp))
    {
      if (zif->rtadv.AdvSendAdvertisements)
				vty_out (vty, " no ipv6 nd suppress-ra%s", VTY_NEWLINE);
/*
			else
	vty_out (vty, " ipv6 nd suppress-ra%s", VTY_NEWLINE);
*/
    }

  
  interval = zif->rtadv.MaxRtrAdvInterval;
  if (interval % 1000)
    vty_out (vty, " ipv6 nd ra-interval msec %d%s", interval,
	     VTY_NEWLINE);
  else
    if (interval != RTADV_MAX_RTR_ADV_INTERVAL)
      vty_out (vty, " ipv6 nd ra-interval %d%s", interval / 1000,
	     VTY_NEWLINE);
	/*gujd: 2012-05-29, pm 3:48. Add for IPv6 Ready Test. */
#if 1
  if (zif->rtadv.MinRtrAdvInterval != RTADV_MIN_RTR_ADV_INTERVAL && zif->rtadv.MinRtrAdvInterval != 0.33*zif->rtadv.MaxRtrAdvInterval)
      vty_out (vty, " ipv6 nd ra-min-interval %d%s", zif->rtadv.MinRtrAdvInterval / 1000,
	     VTY_NEWLINE);
  if (zif->rtadv.AdvLinkMTU)
      vty_out (vty, " ipv6 nd ra-link-mtu %d%s", zif->rtadv.AdvLinkMTU,
	     VTY_NEWLINE);
   if (zif->rtadv.AdvCurHopLimit != 64)
      vty_out (vty, " ipv6 nd adv-curhoplimt %d%s", zif->rtadv.AdvCurHopLimit,
	     VTY_NEWLINE);
   if (zif->rtadv.AdvRetransTimer != 1000)
      vty_out (vty, " ipv6 nd ra-retrans-time msec %u%s", zif->rtadv.AdvRetransTimer,
	     VTY_NEWLINE);
#endif

  if (zif->rtadv.AdvDefaultLifetime != RTADV_ADV_DEFAULT_LIFETIME)
    vty_out (vty, " ipv6 nd ra-lifetime %d%s", zif->rtadv.AdvDefaultLifetime,
	     VTY_NEWLINE);

 /* if (zif->rtadv.AdvReachableTime)*/
  if (zif->rtadv.AdvReachableTime != 30000)
    vty_out (vty, " ipv6 nd reachable-time %d%s", zif->rtadv.AdvReachableTime,
	     VTY_NEWLINE);

  if (zif->rtadv.AdvManagedFlag)
    vty_out (vty, " ipv6 nd managed-config-flag%s", VTY_NEWLINE);

  if (zif->rtadv.AdvOtherConfigFlag)
    vty_out (vty, " ipv6 nd other-config-flag%s", VTY_NEWLINE);
  if (zif->rtadv.prefix_flag == 1)
  {
  	prefix_start = zif->rtadv.pool->head->data;
	prefix_end = zif->rtadv.pool->head->next->data;
	vty_out (vty, " ipv6 nd prefix pool %s/%d %s/%d\n",inet_ntop (AF_INET6, &prefix_start->prefix.u.prefix6, 
			  (char *) buf, INET6_ADDRSTRLEN),
	   	    	prefix_start->prefix.prefixlen,
	   	    	inet_ntop (AF_INET6, &prefix_end->prefix.u.prefix6, 
			  (char *) buf1, INET6_ADDRSTRLEN),
	      		 prefix_end->prefix.prefixlen);
  }
  for (ALL_LIST_ELEMENTS_RO (zif->rtadv.AdvPrefixList, node, rprefix))
    {
      vty_out (vty, " ipv6 nd prefix %s/%d",
	       inet_ntop (AF_INET6, &rprefix->prefix.u.prefix6, 
			  (char *) buf, INET6_ADDRSTRLEN),
	       rprefix->prefix.prefixlen);
      if ((rprefix->AdvValidLifetime != RTADV_VALID_LIFETIME) || 
	  (rprefix->AdvPreferredLifetime != RTADV_PREFERRED_LIFETIME))
	{
	  if (rprefix->AdvValidLifetime == UINT32_MAX)
	    vty_out (vty, " infinite");
	  else
	    vty_out (vty, " %u", rprefix->AdvValidLifetime);
	  if (rprefix->AdvPreferredLifetime == UINT32_MAX)
	    vty_out (vty, " infinite");
	  else
	    vty_out (vty, " %u", rprefix->AdvPreferredLifetime);
	}
      if (!rprefix->AdvOnLinkFlag)
	vty_out (vty, " off-link");
      if (!rprefix->AdvAutonomousFlag)
	vty_out (vty, " no-autoconfig");
      if (rprefix->AdvRouterAddressFlag)
	vty_out (vty, " router-address");
      vty_out (vty, "%s", VTY_NEWLINE);
    }
}


static void
rtadv_event (enum rtadv_event event, int val, void *arg)
{
  switch (event)
    {
    case RTADV_START:
      if (! rtadv->ra_read)
      {
      	zlog_debug(" to set read .\n");
		rtadv->ra_read = thread_add_read (zebrad.master, rtadv_read, NULL, val);
      	}
      if (! rtadv->ra_timer)
	rtadv->ra_timer = thread_add_event (zebrad.master, rtadv_timer,
	                                    NULL, 0);
      break;
	/*gujd : 2012-05-29,pm 9:53 . Add for IPv6 Ready Test.*/	  
	case RTADV_TIMER_MSEC_IF:
	  	 if (! rtadv->ra_timer_send)
	  	 {
		   if(RTM_DEBUG_RTADV)
		   	 zlog_info("%s : set timer to send one packet .\n",__func__);
	       rtadv->ra_timer_send = thread_add_timer_msec(zebrad.master, rtadv_timer_send_packet,
	                                    arg, val);
	  	 	}
	  break;
	  	
    case RTADV_STOP:
      if (rtadv->ra_timer)
	{
	  thread_cancel (rtadv->ra_timer);
	  rtadv->ra_timer = NULL;
	}
      if (rtadv->ra_read)
	{
	  thread_cancel (rtadv->ra_read);
	  rtadv->ra_read = NULL;
	}
	/*gujd : 2012-05-29,pm 9:53 . Add for IPv6 Ready Test.*/
	  if(rtadv->ra_timer_send)
	 {
	  thread_cancel (rtadv->ra_timer_send);
	  rtadv->ra_timer_send = NULL;
	}
      break;
    case RTADV_TIMER:
      if (! rtadv->ra_timer)
	rtadv->ra_timer = thread_add_timer (zebrad.master, rtadv_timer, NULL,
	                                    val);
      break;
    case RTADV_TIMER_MSEC:
      if (! rtadv->ra_timer)
	rtadv->ra_timer = thread_add_timer_msec (zebrad.master, rtadv_timer, 
					    NULL, val);
      break;
    case RTADV_READ:
      if (! rtadv->ra_read)
	rtadv->ra_read = thread_add_read (zebrad.master, rtadv_read, NULL, val);
      break;
    default:
      break;
    }
  return;
}

void
rtadv_init (void)
{
  int sock;
  sock = rtadv_make_socket ();
  if (sock < 0)
    return;

  rtadv = rtadv_new ();
  rtadv->sock = sock;

  install_element (INTERFACE_NODE, &ipv6_nd_suppress_ra_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_suppress_ra_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_interval_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_interval_msec_cmd);
  /*gujd : 2012-05-29,pm 9:53 . Add for IPv6 Ready Test.*/
  install_element (INTERFACE_NODE, &ipv6_nd_ra_min_interval_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_min_interval_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_adv_cur_hop_limit_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_adv_cur_hop_limit_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_retrans_time_cmd);
  /*install_element (INTERFACE_NODE, &no_ipv6_nd_ra_retrans_time_cmd);*/
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_interval_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_link_mtu_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_link_mtu_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_retrans_time_ms_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_ra_retrans_time_ms_cmd);
  /** Add for IPv6 Ready Test.-----end--**/
  install_element (INTERFACE_NODE, &ipv6_nd_ra_lifetime_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_ra_lifetime_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_reachable_time_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_reachable_time_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_managed_config_flag_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_managed_config_flag_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_other_config_flag_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_other_config_flag_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_config_flag_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_config_flag_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_preference_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_preference_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_homeagent_lifetime_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_homeagent_lifetime_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_adv_interval_config_option_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_adv_interval_config_option_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_pool_cmd);
   install_element (INTERFACE_NODE, &no_ipv6_nd_prefix_pool_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_rev_rtaddr_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_nortaddr_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_rev_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_noauto_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_offlink_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_rtaddr_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_val_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_noval_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_noval_rev_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_noval_noauto_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_noval_offlink_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_noval_rtaddr_cmd);
  install_element (INTERFACE_NODE, &ipv6_nd_prefix_prefix_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_nd_prefix_cmd);
}

static int
if_join_all_router (int sock, struct interface *ifp)
{
  int ret;

  struct ipv6_mreq mreq;

  memset (&mreq, 0, sizeof (struct ipv6_mreq));
  inet_pton (AF_INET6, ALLROUTER, &mreq.ipv6mr_multiaddr);
  mreq.ipv6mr_interface = ifp->ifindex;

  ret = setsockopt (sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, 
		    (char *) &mreq, sizeof mreq);
  if (ret < 0)
    zlog_warn ("can't setsockopt IPV6_JOIN_GROUP: %s", safe_strerror (errno));
	if(RTM_DEBUG_RTADV)
	  zlog_info ("rtadv: %s join to all-routers multicast group", ifp->name);
  return 0;
}

static int
if_leave_all_router (int sock, struct interface *ifp)
{
  int ret;

  struct ipv6_mreq mreq;

  memset (&mreq, 0, sizeof (struct ipv6_mreq));
  inet_pton (AF_INET6, ALLROUTER, &mreq.ipv6mr_multiaddr);
  mreq.ipv6mr_interface = ifp->ifindex;

  ret = setsockopt (sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, 
		    (char *) &mreq, sizeof mreq);
  if (ret < 0)
    zlog_warn ("can't setsockopt IPV6_LEAVE_GROUP: %s", safe_strerror (errno));
	if(RTM_DEBUG_RTADV)
		zlog_info ("rtadv: %s leave from all-routers multicast group", ifp->name);

  return 0;
}

#else
void
rtadv_init (void)
{
  /* Empty.*/;
}
#endif /* RTADV && HAVE_IPV6 */

/*#undef HAVE_RTADR_DEBUG*/
