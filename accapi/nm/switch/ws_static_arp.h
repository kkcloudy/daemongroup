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
* ws_static_arp.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _WS_STATIC_ARP_H
#define _WS_STATIC_ARP_H

#include <string.h>
#include <dbus/dbus.h>
 
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
//#include "ws_err.h"

#include "ws_init_dbus.h"
#include "ws_returncode.h"


// define   errors
#define WP_ERR_NULL_POINTER -1
#define WP_ERR_IPADDR_LEN   -2
#define WP_ERR_OUTOF_MEMERY -3

#define WS_ERR_UNKNOWN          -100
#define WS_ERR_PORT_FORMAT      -101
#define WS_ERR_PORT_NUM         -102
#define WS_ERR_PORT_NOPORT      -103
#define WS_ERR_MAC_FORMAT       -104
#define WS_ERR_MAC_BROADCAST    -105
#define WS_ERR_MAC_BASE_MAC     -106
#define WS_ERR_IP_MASK_32       -107
#define WS_ERR_VLANID_FORMAT    -108
#define WS_ERR_VLANID_OUTRANGE  -109

//define size
#define PORT_NUM_STR_LEN    32
#define MAC_STR_LEN         32
#define IPADDR_STR_LEN      32
#define VLANID_STR_LEN      32
#define PORDUCT_STR_LEN     32
#define EDIT_TYPE_STR_LEN	32
#define MAX_URL_LEN         512

//define slot_port info
#define X7_MAIN_SLOT_NUM    1
#define X7_MAIN_SLOT_ID     0
#define X7_MAIN_PORT_NUM    4
#define X7_SLAVE_SLOT_NUM   4
#define X7_SLAVE_PORT_NUM   6

#define X5_MAIN_SLOT_NUM    0
#define X5_SLAVE_SLOT_NUM   1
#define X5_SLAVE_PORT_NUM   24
#define IP_MASK_CHECK(num) 					\
if((num<0)||(num>32)) 		\
 {													\
		return COMMON_ERROR;							\
 }

/*add special*/
#define MIN_VLANID 1
#define MAX_VLANID 4094
#define MAX_L3INTF_VLANID 4095

#define DCLI_ARP_SNOOPING_ERR_NONE		0
#define DCLI_ARP_SNOOPING_ERR_STATIC_EXIST (DCLI_ARP_SNOOPING_ERR_NONE + 8)
#define DCLI_ARP_SNOOPING_ERR_STATIC_NOTEXIST	(DCLI_ARP_SNOOPING_ERR_NONE + 9)
#define DCLI_ARP_SNOOPING_ERR_PORT_NOTMATCH     (DCLI_ARP_SNOOPING_ERR_NONE + 10)
#define DCLI_ARP_SNOOPING_ERR_KERN_CREATE_FAILED (DCLI_ARP_SNOOPING_ERR_NONE + 11)
#define DCLI_ARP_SNOOPING_ERR_STATIC_ARP_FULL    (DCLI_ARP_SNOOPING_ERR_NONE + 13)  /* static arp is full (1k)*/
#define DCLI_ARP_SNOOPING_ERR_HASH_OP_FAILED     (DCLI_ARP_SNOOPING_ERR_NONE + 14)  /* hash push or pull failed*/


extern int config_ip_static_arp( char *portnum, char *mac, char *ipaddr, char *vlanID_str );
extern int delete_ip_static_arp( char *portnum, char *mac, char *ipaddr, char *vlanID_str );
extern int config_ip_static_arp_for_slot0(char *portnum, char *mac, char *ipaddr);
extern int config_ip_static_arp_trunk(char *trunkidz,char *macz,char *ipaddrz,char *vidz);
extern int config_noip_static_arp_trunk(char *trunkidz,char *macz,char *ipaddrz,char *vidz);
#endif
