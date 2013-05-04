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
*stp_to.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for packet sending in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* This file contains system dependent API
   from the RStp to a operation system (see stp_to.h) */

/* stp_to API for Linux */

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"
#include "stp_to.h"
#include "stp_log.h"
#include "stp_param.h"

extern BITMAP_T        enabled_ports;
extern unsigned char   sysMac[6];
extern struct port_speed_node **port_speed_s;
extern struct port_duplex_mode **port_duplex_mode_nodes;

/*************
void
stp_trace (const char *format, ...)
{
  #define MAX_MSG_LEN  128
  char     msg[MAX_MSG_LEN];
  va_list  args;

  va_start(args, format);
  vsnprintf (msg, MAX_MSG_LEN-1, format, args);
  printf ("%s\n", msg);
  va_end(args);
  
}
***********/

#ifdef STRONGLY_SPEC_802_1W
int
stp_to_set_learning (int port_index, int vlan_id, int enable)
{
  return STP_OK;
}

int
stp_to_set_forwarding (int port_index, int vlan_id, int enable)
{
  return STP_OK;
}
#else
/* 
 * In many kinds of hardware the state of ports may
 * be changed with another method
 */
int
stp_to_set_port_state (IN int port_index, IN int vlan_id,
            IN RSTP_PORT_STATE state)
{

	stp_syslog_dbg("^&^&^&^&^&^&\n INTO THE FUN\n^&^&^&^&^&^&\n");
  return STP_OK;
  //return AR_INT_STP_set_port_state (port_index, vlan_id, state);
}
#endif


void
stp_to_get_port_mac (int port_index, unsigned char *mac)
{
  static long pid = -1;
  static unsigned char mac_beg[] = {'\0', '\0', '\0', '\0', '\0', '\0'};
  
  /*
  if (pid < 0) {
    pid = getpid ();
    memcpy (mac_beg + 1, &pid, 4);
  }
  memcpy (mac, mac_beg, 5);
  mac[1] = 0x06;
  mac[5] = port_index;
  */
  //memcpy (mac, STP_MAIN_get_port_mac (port_index), 6);
  memcpy(mac,sysMac,6);
}

int             /* 1- Up, 0- Down */
stp_to_get_port_link_status (int port_index)
{
  if (stp_bitmap_get_bit (&enabled_ports, (port_index ))) return 1;
  return 0;
}

int
stp_to_flush_lt (IN int port_index, IN int vlan_id, LT_FLASH_TYPE_T type, char* reason)
{
/****
  stp_trace("clearFDB (%d, %s, '%s')",
        port_index, 
        (type == LT_FLASH_ALL_PORTS_EXCLUDE_THIS) ? "Exclude" : "Only", 
        reason);
****/

  return STP_OK;
}

int
stp_to_set_hardware_mode (int vlan_id, UID_STP_MODE_T mode)
{
  return STP_OK;
  //return AR_INT_STP_set_mode (vlan_id, mode);
}


int
stp_to_tx_bpdu 
(
	int port_index, 
	int vlan_id,
	unsigned char *bpdu, 
	size_t bpdu_len, 
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
)
{
	unsigned char ifName[20] = {0};
	unsigned int ifIndex = 0, ret = 0;
	stp_syslog_packet_send("i begin transmit to bpdu:\n");
	extern int stp_uid_bridge_tx_bpdu (int port_index, unsigned char *bpdu, size_t bpdu_len);

	if (isWAN) {
		if (vlan_id > 1) {
			sprintf(ifName, "eth%d-%d.%d", slot_no, port_no, vlan_id);
		}
		else {
			sprintf(ifName, "eth%d-%d", slot_no, port_no);
		}
		/*ifIndex = if_nametoindex(ifName);*/
		stp_netlink_socket_sendto(ifName, bpdu, bpdu_len);
		return 0;
	}
	
  	return stp_uid_bridge_tx_bpdu (port_index, bpdu, bpdu_len);
}

const char *
stp_to_get_port_name (IN int port_index)
{
  static char tmp[4];
  sprintf (tmp, "p%02d", (int) port_index);
  return tmp;
  //return port2str (port_index, &sys_config);
}

unsigned long
stp_to_get_deafult_port_path_cost (IN unsigned int portNo)
{
  return 20000;
}


unsigned long stp_to_get_port_oper_speed (PORT_T *port)
{
  /*
  if (portNo <= 259)
    return 1000000L;
  else
    return 1000L;
    */
   unsigned int portNo = port->port_index;
  
  if(( portNo > PORT_INDEX_MAX)||(portNo < 0)){
     return STP_ERROR;
  }
  else{
      if((NULL != port_speed_s)&&(NULL != port_speed_s[portNo])\
          &&( portNo == port_speed_s[portNo]->port_index)  \
          &&(port->adminEnable)){
		  	if( STP_PORT_SPEED_10_E == port_speed_s[portNo]->port_speed)
				return 10L;
			else if(STP_PORT_SPEED_100_E == port_speed_s[portNo]->port_speed)
				return 100L;
			else if(STP_PORT_SPEED_1000_E == port_speed_s[portNo]->port_speed)
				return 1000L;
			else if(STP_PORT_SPEED_10000_E == port_speed_s[portNo]->port_speed)
				return 10000L;
			else if(STP_PORT_SPEED_12000_E == port_speed_s[portNo]->port_speed)
				return 12000L;
			else if(STP_PORT_SPEED_2500_E == port_speed_s[portNo]->port_speed)
				return 2500L;
			else if(STP_PORT_SPEED_5000_E == port_speed_s[portNo]->port_speed)
				return 5000L;
      }
	  else{
	  	/* if get error, set to confirm value ?*/
           if (portNo >= PORT_INDEX_MAX)
		    return 1000000L;
		  else
		    return 1L; 
	  }
  }
}

int             /* 1- Full, 0- Half */
stp_to_get_duplex ( PORT_T *port)
{
    unsigned int port_index = 0;
	port_index = port->port_index;
  	/*define struct*/
	if(( port_index > PORT_INDEX_MAX)||(port_index < 0)){
	     return STP_ERROR;
	}
    else{
       if((NULL != port_duplex_mode_nodes)&&(NULL != port_duplex_mode_nodes[port_index])&&(port->adminEnable)){
           stp_syslog_dbg("stp_to_get_duplex-port_duplex_mode_nodes[%d]->port_duplex_mode %d\n",port_index,port_duplex_mode_nodes[port_index]->port_duplex_mode);
		   if( 1 == port_duplex_mode_nodes[port_index]->port_duplex_mode)
	   	       return P2P_NO;
           else if(0 == port_duplex_mode_nodes[port_index]->port_duplex_mode)
	   	       return P2P_YES;
      }
	  else{
	  	  /* when admin state is AUTO and can not get the duplex mode , set default value: P2P_NO*/
		  return P2P_NO;
	  }
  }
}

int
stp_to_get_init_stpm_cfg (IN int vlan_id,
                           INOUT UID_STP_CFG_T* cfg)
{
  cfg->bridge_priority =        DEF_BR_PRIO;
  cfg->max_age =                DEF_BR_MAXAGE;
  cfg->hello_time =             DEF_BR_HELLOT;
  cfg->forward_delay =          DEF_BR_FWDELAY;
  cfg->force_version =          NORMAL_RSTP;

  return STP_OK;
}
  

int
stp_to_get_init_port_cfg (IN int vlan_id,
                           IN int port_index,
                           INOUT UID_STP_PORT_CFG_T* cfg)
{
  cfg->port_priority =                  DEF_PORT_PRIO;
  cfg->admin_non_stp =                  DEF_ADMIN_NON_STP;
  cfg->admin_edge =                     DEF_ADMIN_EDGE;
  cfg->admin_port_path_cost =           ADMIN_PORT_PATH_COST_AUTO;
  cfg->admin_point2point =              DEF_P2P;

  return STP_OK;
}

#ifdef __cplusplus
}
#endif


