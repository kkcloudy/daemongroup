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
* ws_dhcpv6.h
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
#ifndef _WS_DHCPV6_H
#define _WS_DHCPV6_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/dhcp/dhcp6_dbus_def.h" 
#include "sysdef/npd_sysdef.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_init_dbus.h"
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include "ws_dcli_dhcp.h"


#define DHCP_SERVER_SUBNET 1
#define DHCP_SERVER_HOST 2	
#define DHCP_SERVER_DOMAIN_NAME 3
#define DHCP_SERVER_DNS 4
#define DHCP_SERVER_WINS 5
#define DHCP_SERVER_MAX_LEASE_TIME 6
#define DHCP_SERVER_DEFAULT_LEASE_TIME 7

#define ADD_OPT	 0
#define DEL_OPT	 1 

struct iaddr {
	unsigned len;
	unsigned char iabuf [16];
};
/*use show dhcp6 */
struct dhcp6_show {
	unsigned int enable;	
	char* domainsearch;	
	struct iaddr dnsip[3];
	unsigned int dnsnum;	
	unsigned int defaulttime;
};
struct dhcp6_sub{
	char * range_low_ip;
	char * range_high_ip;
	unsigned int prefix_length;
	struct dhcp6_sub* next;
};
/*use show dhcp6 pool*/
struct dhcp6_pool_show
{
	char *poolname;
	char *domain_name;
	char *option52[8];
	unsigned int option_adr_num;
	char *dnsip[3];
	unsigned int dnsnum;	
	unsigned int defaulttime;
	unsigned int sub_count;	
	char *interfacename;
	struct dhcp6_sub *ipv6_subnet;
	struct dhcp6_pool_show *next;
};
struct dhcp6_pool_show_st
{
	char *poolname;
	char *domain_name;
	char *option52[8];
	unsigned int option_adr_num;
	char *dnsip[3];
	unsigned int dnsnum;	
	unsigned int defaulttime;
	unsigned int sub_count;	
	struct dhcp6_sub ipv6_subnet;
	struct dhcp6_pool_show *next;
};

extern int ccgi_show_dhcp6_lease
(
	unsigned int mode,
	unsigned int index,
	struct dhcp6_pool_show *head,
	unsigned int *num
);/*0:fail;1:succ*/

extern void Free_ccgi_show_dhcp6_lease(struct dhcp6_pool_show_st *head);
extern unsigned int ccgi_show_ipv6_dhcp_server(struct dhcp6_show *owned_option);

extern int ccgi_show_ipv6_pool
(
	unsigned int mode,
	unsigned int index,
	struct dhcp6_pool_show *head,
	unsigned int *num,
	DBusConnection *connection
);
extern void Free_ccgi_show_ipv6_pool(struct dhcp6_pool_show *head);
extern int ccgi_create_ipv6_pool_name(unsigned int add_del,char *poolName,unsigned int *pindex,int slot_id);
						/*1:succ;0:fail;-1:poolname is null;-2:delete ip pool fail;-3:create ip pool fail*/
extern int ccgi_config_ipv6_pool_name(char *pool_name,unsigned int *pindex,int slot_id);/*1:succ;0:fail*/
extern int ccgi_add_dhcp_pool_ipv6_range
(
	unsigned int add,
	struct iaddr *ipaddrl, 
	struct iaddr *ipaddrh,
	unsigned int prefix_length,
	unsigned int index,
	int slot_id
);
extern int  ccgi_set_server_lease_default_ipv6
(
	unsigned int lease_default,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
);/*1:succ;0:fail*/
extern int ccgi_set_server_option52_ipv6
(
	char *optstr,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot
);/*1:succ;0:fai; -1:ap_via_address is null*/
extern int ccgi_set_server_domain_search_ipv6
(
	char *domainName,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot
);/*1:succ;0:fail;-1:domainName is null*/
extern int ccgi_set_server_name_servers_ipv6
(
	char *dnsstr,
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
);/*1:succ;0:fail;*/
extern int  ccgi_str2_ipv6_addr
( 
	char *str,
	struct iaddr *p
);
extern int  ccgi_set_interface_ipv6_pool
(
	char* poolName,
	char* ifname,
	unsigned int add_info
);
extern int ccgi_addordel_ipv6pool(char *opt,char *startip,char *endip,char *prefix,	unsigned int index,int slot_id);
extern int ccgi_check_ipv6_address(char *ipv6_address);
extern  int ccgi_set_dhcpv6_server_enable(char *serveropt);
int ccgi_show_dhcp_lease_ipv6_state(struct lease_state  *total_state, struct sub_lease_state *sub_state, unsigned int *subnet_num, int slotid);

#endif
