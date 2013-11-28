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
* ws_dcli_dhcp.h
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
#ifndef _WS_DCLI_DHCP_H
#define _WS_DCLI_DHCP_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/dhcp/dhcp_dbus_def.h" 
#include "sysdef/npd_sysdef.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_init_dbus.h"
#include "ws_dbus_def.h"
#include "dbus/sem/sem_dbus_def.h"
#include "ac_manage_def.h"

#define ALIAS_NAME_SIZE 		0x15
#define MAC_ADDRESS_LEN			6
#define MAX_IP_STRING_LEN		16
#define BIND_POOL_ON_INTERFACE	0
#define UNBIND_POOL_ON_INTERFACE 1
#define MAX_SUB_NET			1024

#define DHCP_OPTION60_DEFAULT_NUMBER (4)

/*lease state struct */
struct lease_state{
	unsigned int total_lease_num;
	unsigned int active_lease_num;
	unsigned int free_lease_num;
	unsigned int backup_lease_num;
};

struct statistics_info {
	unsigned int host_num;		/* total number of ip address */
	unsigned int segment_times;	/* ip address assigned */
	unsigned int discover_times;
	unsigned int offer_times;
	unsigned int requested_times;
	unsigned int ack_times;
};

/*subnet lease state*/
struct sub_lease_state{
	char subnet[50];
	char mask[50];
	char poolname[64];
	struct statistics_info info;
	struct lease_state subnet_lease_state;
};

struct all_lease_state_p{
	int subnet_num;
	struct lease_state all_lease_state;	
	struct sub_lease_state sub_state[MAX_SUB_NET];
};

struct dhcp_option_show
{
	char* domainname;
	char* option43;
	unsigned int dns[3];
	char *option138[16];	
	char *option60_id[DHCP_OPTION60_DEFAULT_NUMBER];
	unsigned int option60_flag;		/* 1 : enable, 0 : disable */
	unsigned int routers;
	unsigned int wins;
	unsigned int defaulttime;
	unsigned int maxtime;
	
};

struct dhcp_global_show
{
	unsigned int enable;
	struct dhcp_option_show option_show;
};

struct dhcp_global_show_st
{
	unsigned int enable;
	unsigned int unicast;
	unsigned int staticarp;
	char* domainname;
	char* option43;
	unsigned int dns[3];
	unsigned int routers;
	unsigned int wins;
	unsigned int defaulttime;
	unsigned int maxtime;
};

typedef struct
{
    unsigned char arEther[MAC_ADDRESS_LEN];
}FC_ETHERADDR;

struct dhcp_static_show
{
	unsigned int ipaddr;
	unsigned char mac[MAC_ADDRESS_LEN];
	char *ifname;
};

struct dhcp_static_show_st
{
	unsigned int ipaddr;
	unsigned char mac[MAC_ADDRESS_LEN];
	char *ifname;
	struct dhcp_static_show_st *next;
};

struct dhcp_static_show_sts
{
	unsigned int ipaddr;
	unsigned char mac[MAC_ADDRESS_LEN];
};

struct dhcp_pool_show
{
	char *poolname;
	char *interfacename;
	struct dhcp_sub_show* sub_show;
	unsigned int sub_count;
	struct dhcp_option_show option_show;
};
struct dhcp_sub_show_st
{
	unsigned int iplow;
	unsigned int iphigh;
	unsigned int mask;
	struct dhcp_sub_show_st *next;
};

struct dhcp_pool_show_st
{
	char *poolname;
	char *ifname;
	char* domainname;
	char* option43;
	unsigned int dns[3];
	char *option138[16];	
	char *option60_id[DHCP_OPTION60_DEFAULT_NUMBER];
	unsigned int option60_flag;		/* 1 : enable, 0 : disable */
	unsigned int routers;
	unsigned int wins;
	unsigned int defaulttime;
	unsigned int maxtime;
	struct dhcp_sub_show_st sub_show;
	unsigned int sub_count;
	struct dhcp_pool_show_st *next;	
};

struct dhcp_sub_show 
{
	unsigned int iplow;
	unsigned int iphigh;
	unsigned int mask;
};


struct dhcp_lease_st
{
	char *leaseip;
	char *leasemac;
	int lnum;
	struct dhcp_lease_st *next;
};

struct dhcp_failover_show
{
	char name[ALIAS_NAME_SIZE];
	unsigned int primary;/*bool 0 for primary 1 for secondary*/
	unsigned int split;
	unsigned int mclt;
	unsigned int dstport;
	unsigned int dstip;
	unsigned int srcport;
	unsigned int srcip;
};

struct dhcp_relay_show_st
{
	char downifname[21];
	char upifname[21];
	unsigned int ipaddr;
	struct dhcp_relay_show_st *next;
};
extern int save_dhcp_leasez(int slot_id);/*返回1表示成功，返回0表示失败,返回-1表示error*/

extern void Free_show_dhcp_lease(struct dhcp_lease_st *head);
extern int show_dhcp_lease(struct dhcp_lease_st *head,int *lnum,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int ccgi_show_lease_by_ip(struct dhcp_lease_st *head,unsigned int ipaddr, unsigned int ipnums,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int show_dhcp_lease_by_ip(char *ip,struct dhcp_lease_st *head,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int show_dhcp_lease_by_ip_diff(char *ipaddr1,char *ipaddrd2,struct dhcp_lease_st *head,int slot_id);/*返回1表示成功，返回0表示失败,返回-1表示error*/
extern int show_dhcp_lease_by_ip_mask(char *ip,struct dhcp_lease_st *head,int slot_id);/*返回1表示成功，返回0表示失败,返回-1表示error*/
extern int show_dhcp_lease_by_mac(char *mac,char *leaseip,char *leasemac,int slot_id);
extern unsigned int ccgi_show_static_host(	struct dhcp_static_show_st *static_host,unsigned int *host_num,int slot_id);
extern unsigned int ccgi_show_ip_dhcp_server(struct dhcp_global_show_st *global_show,int slot_id);
extern unsigned int ccgi_show_ip_pool(unsigned int mode,	unsigned int index,	struct dhcp_pool_show_st *head,	unsigned int *num,int slot_id);
extern unsigned int ccgi_create_ip_pool_name(unsigned int del,char *poolName,unsigned int *pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2表示create ip pool fail */
extern int create_ip_pool_name(char *poolnamez,unsigned int *pindex,int slot_id);
extern int delete_ip_pool_name(char *poolnamez,unsigned int* indexz,int slot_id);/*返回1表示成功，返回0表示失败*/
extern unsigned int ccgi_config_ip_pool_name(char *poolName,unsigned int *pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int config_ip_pool_name(char *poolnamez,unsigned int *pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示config ip pool fail*/
/*add_info 1:ip pool POOLNAME*/
/*add_info 0:no ip pool POOLNAME*/
extern unsigned int ccgi_set_interface_ip_pool(char* poolName,char* ifname,unsigned int add_info,unsigned int unbindflag,int slot_id);
																								/*表示0表示失败，返回1表示成功*/
																								/*返回-1表示pool name is too long*/
																								/*返回-2表示pool has already binded to interface*/
																								/*返回-3表示pool has no subnet*/
																								/*返回-4表示not found pool，返回-5表示error*/
extern int set_interface_ip_pool(char *poolnamez,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示interface bind pool fail*/
extern int del_interface_ip_pool(char *poolnamez,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示interface unbind pool fail*/
extern int ccgi_add_dhcp_pool_ip_range(	unsigned int add,	unsigned int ipaddrl, 	unsigned int ipaddrh,	unsigned int ipmask,	unsigned int index,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示errror*/
extern int add_dhcp_pool_ip_range(char *addordel,char *startip,char *endip,char *maskz,unsigned int pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示bad command parameter，返回-2表示结束ip不大于起始ip*/
extern int ccgi_add_dhcp_static_host(unsigned int ipaddr,unsigned char* mac,unsigned char* ifname,	unsigned int add,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int add_dhcp_static_host(char *ipz,char *macz,char *ifname,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int delete_dhcp_static_host(char *ipz,char *macz,char *ifname,int slot_id);/*返回1表示成功，返回0表示失败*/
extern unsigned int ccgi_set_server_domain_name(char *domainName,unsigned int mode,	unsigned int index,	unsigned int del,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2表示domainname非法*/
extern int ip_dhcp_server_domain_name(char *domainnamez,int modez,unsigned int pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-2表示Bad parameter,pool domainname NOT exists*/
extern int no_ip_dhcp_server_domain_name(char *domainnamez,unsigned int  index,int modez,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示Bad parameter,pool domainname NOT exists*/
extern unsigned int ccgi_set_server_option43(char *veo,	unsigned int mode,unsigned int index,unsigned int del,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2*/
extern int ip_dhcp_server_option43(char *veoz,int modez,unsigned int pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示Bad parameter,pool veo NOT exists*/
extern unsigned int ccgi_set_server_dns(unsigned int *ipAddr,unsigned int ipNum,unsigned int mode,	unsigned int index,	unsigned int del,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_dns(int pmode,unsigned int pindex,char *ipaddr,int slot_id);
extern int no_ip_dhcp_server_dns(int modez,unsigned int index,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern unsigned int ccgi_set_server_routers_ip(	unsigned int routers,unsigned int mode,	unsigned int index,	unsigned int del,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_routers_ip(char *ipz,unsigned int pindex,int pmode,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int no_ip_dhcp_server_routers_ip(int modez,unsigned int index,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern unsigned int ccgi_set_server_wins_ip(unsigned int wins,unsigned int mode,unsigned int index,	unsigned int del,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_wins_ip(char *ipaddrz,int modez,unsigned int pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int no_ip_dhcp_server_wins_ip(int modez,unsigned int index,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern unsigned int  ccgi_set_server_lease_default(	unsigned int lease_default,	unsigned int mode,	unsigned int index,	unsigned int del ,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_lease_default(char *leasez,int modez,unsigned int pindex,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int no_ip_dhcp_server_lease_default(int modez,unsigned int index,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern unsigned int ccgi_set_server_lease_max(unsigned int lease_max,unsigned int mode,	unsigned int index,	unsigned int del );/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_lease_max(char *leasemaxz,int modez,unsigned int pindex);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int no_ip_dhcp_server_lease_max(int modez,unsigned int pindex);/*返回1表示成功，返回0表示失败*/
extern unsigned int ccgi_set_server_enable(	unsigned int enable,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示error*/
extern int ip_dhcp_server_enable(char *endis,int slot_id);/*返回1表示成功，返回0表示失败，返回-2表示bad command parameter*/
extern unsigned int ccgi_set_failover(unsigned int mode,unsigned int index,	struct dhcp_failover_show* failover_conf,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int add_dhcp_failover_pool(int modez,char *fname,char *prim,char *splitz,char *mcltz,char *dstipz,char *dstportz,char *srcipz,char *srcportz,int slot_id);
extern unsigned int  ccgi_config_failover_pool(	unsigned int mode,	unsigned int index,	struct dhcp_failover_show* failover_conf,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int cfg_dhcp_failover_pool(int modez,char *fname,char *prim,int slot_id);
extern unsigned int ccgi_delete_failover_pool(	unsigned int mode,	unsigned int index,	struct dhcp_failover_show* failover_conf,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int del_dhcp_failover_pool(int modez,char *fname,int slot_id);/*返回1表示成功，返回0表示失败，返回-1表示failover name is too long*/
extern unsigned int ccgi_show_failover_configure(unsigned int mode,	unsigned int index,	struct dhcp_failover_show *failover_cfg,int slot_id);
extern char* ip_long2string(unsigned long ipAddress, char *buff);
extern void Free_show_dhcp_pool(struct dhcp_pool_show_st *head);
extern int if_mask_bit(char *mask);
extern unsigned int ccgi_set_server_static_arp_enable(unsigned int enable,int slot_id);/*返回1表示成功，返回0表示失败*/
extern int ccgi_dhcp_check_relay_interface_iSbusy(char *ifname,int slot_id);/*返回0表示失败，返回1表示成功*/
extern void Free_ccgi_show_static_host(struct dhcp_static_show_st *head);
extern unsigned int ccgi_set_relay_enable(unsigned int enable,int slot_id);/*return 1:succ; return 0:fail*/
extern unsigned int ccgi_set_interface_ip_relay(char *upifname,char *downifname,unsigned int ipaddr,unsigned int add_info,int slot_id);
extern int ccgi_dhcp_check_server_interface_iSbusy(char *upifname,char *downifname,int slot_id);
extern unsigned int ccgi_show_dhcp_relay(struct dhcp_relay_show_st *relayhead,unsigned int *node_num,unsigned int *ifenable,int slot_id);
extern void Free_dhcprelay_info(struct dhcp_relay_show_st *head);
extern unsigned int  ccgi_set_server_no_option43(	unsigned int index,int slot_id);
extern unsigned int ccgi_set_server_option43_veo
(
	char *ap_via_address,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
);

extern int ccgi_show_lease_state
(	
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num,
	int slot_id
);/*0:fail; 1:success; -1:error*/
extern int ccgi_show_dhcp_pool_statistics(int slot_id,struct all_lease_state_p *head);
extern int ccgi_show_lease_state_slot
(	
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num,
	DBusConnection *connection
);/*0:fail; 1:success; -1:error*/

#endif
