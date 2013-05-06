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
* ws_dcli_interface.h
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
#ifndef WS_DCLI_INTERFACE_H
#define WS_DCLI_INTERFACE_H

#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"
#include "ws_returncode.h"

/*
#include "ws_err.h"
#include "ws_ec.h"
*/


#include <sys/types.h>
#include <sys/wait.h>


#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"


struct eth_portlist
{	
	char ethport[30];
	int port_num;
	struct eth_portlist * next;
};

#define CMD_SUCCESS 0
#define CMD_FAILURE -1
#define Mapping_num 128
#define MAX_QOS_PROFILE 127
#define MAX_MAP_QOS 127
#define Policy_Map_Num 128
#define Policer_Num 128

#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'

#define DCLI_CREATE_ERROR 		1
#define DCLI_INTF_CHECK_IP_ERR  4

#define DCLI_INTF_DIS_ROUTING_ERR 19
#define DCLI_INTF_EN_ROUTING_ERR 20
#define DCLI_INTF_EXISTED 21
#define DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF 23
#define DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF 24
#define DCLI_ONLY_RUN_IN_VLAN 25
#define DCLI_ALREADY_ADVANCED 26
#define DCLI_NOT_ADVANCED 27
#define DCLI_PARENT_INTF_NOT_EXSIT 28
#define DCLI_PROMI_SUBIF_EXIST 29
#define DCLI_PROMI_SUBIF_NOTEXIST 30
#define DCLI_MAC_MATCHED_BASE_MAC 32
#define DCLI_L3_INTF_NOT_ACTIVE 33
#define DCLI_INTF_NO_HAVE_ANY_IP 34
#define DCLI_INTF_HAVE_THE_IP 35
#define DCLI_INTF_NOT_SAME_SUB_NET 36
#define DCLI_INTF_STATUS_CHECK_ERR 37
#define DCLI_INTF_GET_SYSMAC_ERR   38

#define NPD_VLAN_PORT_NOTEXISTS	18		// port is not a member of vlan
#define NPD_VLAN_BADPARAM			12  	//bad parameters
#define NPD_DBUS_ERROR  1
#define NPD_DBUS_ERROR_NO_SUCH_PORT 2
#define NPD_DBUS_SUCCESS 0
#define CMD_WARNING 11

#define MIN_VLANID 1
#define MAX_VLANID 4094
#define MAX_L3INTF_VLANID 4095
#define MIN_BONDID       0
#define MAX_BONDID       7
#define MAXLEN_BOND_CMD  128
#define MAXLEN_BOND_NAME 5   /*bond0~bond7*/
#define INTERFACE_NAMSIZ      20



extern int interface_eth_port(char port[12],char tag[12]);/*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format,返回-2表示The internal tag %d is the same as the external tag */
                           /*返回-3表示NO SUCH PORT，返回-4表示FAILED to add port %d/%d to vlan*/

extern int no_interface_eth_port(char port[12],char tag[12]);
extern int ccgi_intf_show_advanced_routing
(	
	unsigned int vlanAdv,
	unsigned int includeRgmii,
	char *infname
);
extern int ccgi_create_vlan_intf_by_vlan_ifname
(
	unsigned short vid
);
extern int ccgi_interface_ifname_vlan(char *ptr);
extern int ccgi_del_vlan_intf( unsigned short vId);
extern int ccgi_no_interface_ifname_vlan(char *ptr);
extern int ccgi_interface_ifname_eth_port(char * ptr);
extern int ccgi_create_eth_port_sub_intf(unsigned char slot_no, unsigned char port_no, unsigned int vid, unsigned int vid2);
extern int ccgi_no_interface_ifname_eth_port(char * ptr);
extern int ccgi_del_eth_port_sub_intf(unsigned char slot_no, unsigned char port_no, unsigned int vid, unsigned int vid2);

extern int ccgi_create_vlan_intf
(
	unsigned short vid,
	unsigned int advanced
);
extern int ccgi_vlan_interface_advanced_routing_enable
(
	unsigned short vid,
	unsigned int enable
);
extern void Free_ethp_info(struct eth_portlist *head);
extern int ccgi_intf_show_advanced_routing_list
(	
	unsigned int vlanAdv,
	unsigned int includeRgmii,
	struct eth_portlist *chead,
	int *cnum
);
extern int ccgi_intf_subif_set_qinq_type
(
    unsigned char *intfName,
    unsigned char *type
);/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter ，返回-3表示Unsupport this command*/

extern int set_intf_qinq_type(char *infnamez,char *infvalue);
extern int ccgi_config_interface_vanId_advanced(char *plot);
extern int ccgi_interface_vlan(char *vlanid);
extern int ccgi_no_interface_vlan(char *port);
extern int ccgi_interface_tag(char *arg1,char *arg2);
extern int ccgi_no_interface_tag(char *arg1,char *arg2);
extern int ccgi_advanced_routing_config(char * ifName,unsigned int isEnable);
extern int ccgi_interface_ifname_bond(char * ptr);/*返回0表示成功，返回-1表示失败*/
extern int ccgi_no_interface_ifname_bond(char * ptr);

/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter，返回-3表示Only bonding interface support add/delete operation*/
extern int interface_bond_add_del_port(char *addordel,char *inftype);
extern int show_bond_slave(char *paramz);
extern unsigned int ccgi_intf_vlan_eth_port_interface_show_advanced_routing(
        unsigned int flag,
        unsigned int slot_no,
        unsigned int port_no,
        unsigned int vid
);/*返回0表示成功，返回-1表示失败，*/
extern int ccgiconfig_no_interface_ifname_eth_port(char * ptr);/*0:succ;other:fail*/
#endif


