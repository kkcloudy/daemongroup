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
* dhcp6_dbus.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 04/28/2010 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		04/28/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <omapip/omapip_p.h>
#include <syslog.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/rtnetlink.h>
#include <bits/sockaddr.h>
#include <sys/ioctl.h>

#include "sysdef/returncode.h"
#include "dhcpd.h"
#include "dhcp6_dbus.h"
#include "dbus/dhcp/dhcp6_dbus_def.h"
#ifdef _D_WCPSS_
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#endif

#define DHCP6_PID_BUFFER_LEN		128
#define DHCP6_PID_PATH_LEN		DHCP6_PID_BUFFER_LEN
#define DHCP6_INVALID_FD		(-1)
#define DHCP6_FD_INIT		DHCP6_INVALID_FD
#define DHCP_SAVE_CFG_MEM	10*1024*1024
#define DHCP6_MAX_POOL_NUM	128


#define DHCP6_PID_FILE_PATH "/var/run/"
#define DHCP6_PID_FILE_PREFIX "dhcp6"
#define DHCP6_PID_FILE_SUFFIX ".pid"
#define DHCP_IPV6_ADDRESS_LEN 16
#define DHCP_MAC_ADDRESS_LEN 6
#define DHCP_HOST_NAME_LEN		16

pthread_t		*dhcp6_dbus_thread, *dhcp6_netlink_thread;
pthread_attr_t	dhcp6_dbus_thread_attr, dhcp6_netlink_thread_attr;
DBusConnection *dhcp6_dbus_connection = NULL;
unsigned int global_current_instance_no = 0;
int g_dhcp6_pid_fd = DHCP6_INVALID_FD;
struct dcli_pool head_pool;
unsigned int count_pool;
struct dcli_host head_host;
unsigned int count_host;
unsigned int dhcp_server_start;
unsigned int pool_index;
unsigned int host_index;
struct subnet_config sub_conf;
struct dcli_option global_server_option;


void dhcp6_tell_whoami(char * myName,	int isLast);
void dhcp6_get_statistics_info(struct dhcpv6_statistics_info *info);



extern int test_for_add_dhcp_file(void);

/* Confpars.c  */
extern int set_server_duid_new(void);
extern void test_for_add_subnet_v6(void);
extern int parse_statement_subnet_no_file_v6(struct subnet_config *sub_conf,struct group *group);
extern isc_result_t
set_server_option
(
	char* name,
	int ipv6,
	char* value,
	int valnum,
	int check, /*for default-leseas*/	
	unsigned int del,
	struct group *opgroup
);
extern void parse_host_declaration_test 
(
	char* hostname,
	struct iaddr* hostipaddr,
	struct hardware* hosthwaddr,
	struct group *group
);
extern int delete_host_by_name(char* name);


/* mdb.c */
extern int get_lease_ip_info_by_hw(unsigned char *hw_addr, struct dhcp_lease_ip_info *lease_ip);
extern struct dhcp_lease_info *get_lease_ip_info_by_ip(unsigned int ip_addr, unsigned int ip_nums);
extern void test_for_del_subnet_pool(struct dcli_subnet *delsub);
extern void test_for_del_subnet_interface(char* interface_name);
extern int get_dhcp_lease_ipv6_state_num
(
	struct dbus_lease_state  *lease_state , 
	struct dbus_sub_lease_state **sub_lease_state, 
	unsigned int *sub_count
);

/* discover.c */
struct iface_info;
struct iface_conf_list;
extern int begin_iface_scan(struct iface_conf_list *ifaces);
extern int next_iface(struct iface_info *info, int *err, struct iface_conf_list *ifaces);
extern void end_iface_scan(struct iface_conf_list *ifaces);


/*return 0 means interface is exist */

struct iface_conf_list {
	int sock;	/* file descriptor used to get information */
	FILE *fp;	/* input from /proc/net/dev */
};

/* 
 * Structure used to return information about a specific interface.
 */
struct iface_info {
	char name[IFNAMSIZ];		/* name of the interface, e.g. "eth0" */
	struct sockaddr_storage addr;	/* address information */
	struct sockaddr_storage addrmask;	/* address mask information */
	int flags;		/* interface flags, e.g. IFF_LOOPBACK */
};

char * 
pha_state2str
(
	int state
)
{
	switch (state)
	{
		case VRRP_STATE_INIT:
			return "init";
		case VRRP_STATE_BACK:
			return "backup";
		case VRRP_STATE_MAST:
			return "master";
		case VRRP_STATE_LEARN:
			return "learn";
		case VRRP_STATE_NONE:
			return "none";
		case VRRP_STATE_TRANSFER:
			return "transfer";
		case VRRP_STATE_DISABLE:
			return "disable";
		default:
			return "unknow";
	}
	return "unknow";
}

char* 
inet_int2str
( 
	unsigned int ip_addr, 
	char *ip_str, 
	unsigned int buffsize 
)
{
	if(ip_str == NULL)
	{
		return "";
	}
	snprintf( ip_str, buffsize, "%u.%u.%u.%u", ip_addr>>24, (ip_addr&0xff0000)>>16,
				(ip_addr&0xff00)>>8, (ip_addr&0xff) );
				
	return ip_str;
}
#ifdef _D_WCPSS_

int 
parse_hansi_radio_ifname
(
	char* ptr,
	int *vrrid, 
	int *wtpid,
	int *radioid,
	int *wlanid
)
{
	
    radio_ifname_state state = check_vrrid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_vrrid: 
				
				*vrrid = strtoul(str,&str,10);
				
				if(*vrrid >= 0 && *vrrid < 255){
					state=check_sub;
				}
				else state=check_fail;
				
				break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
				}
			else
				state = check_fail;
			break;
			
		case check_wtpid: 
			
			*wtpid = strtoul((char *)&(str[1]),&str,10);
			
			if(*wtpid > 0 && *wtpid < 4095){
        		state=check_sub1;
			}
			else state=check_fail;
			
			break;

		case check_sub1: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_hansi_wlan_ebr_ifname
(
	char* ptr,
	int *vrrid, 
	int *wlanid
)
{
	
    radio_ifname_state state = check_vrrid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_vrrid: 				
				*vrrid = strtoul(str,&str,10);
				
				if(*vrrid >= 0 && *vrrid < 255){
					state=check_sub;
				}
				else state=check_fail;
				
				break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;
			
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
	}
		
}

#endif
#if 1
void 
dhcp6_dbus_profile_config_interface_save
(
	char **showif,
	char **ifcursor,
	struct dcli_ifname* ifhead,
	struct dcli_pool* pool_node,
	int *ifLen
)
{
	char ifname[25] = {0};
	char tmpif[25] = {0};
	//int slotid = 0, vrrpid = 0, ifid = 0;
	//int vrrp_flag = 0;
	//int ret = 0;

	log_debug("%s:\n", __func__);

	if (!showif || !*showif || !ifcursor || !*ifcursor || !ifhead || !ifhead || !ifhead) {
		return;
	}

	memset(tmpif, 0, sizeof(tmpif));

	memset(ifname, 0, sizeof(ifname));
	strncpy(ifname, ifhead->interface_name, sizeof(ifname)-1);
	
	
	if (strncmp(ifname, "wlan", 4) && strncmp(ifname, "ebr", 3) && strncmp(ifname, "r", 1)) {
		/*if (1 == check_interfaces_ip_address(ifhead->interface_name)) */{
			*ifLen += sprintf(*ifcursor, "interface %s\n", ifname);
			*ifcursor = *showif + *ifLen;
			*ifLen += sprintf(*ifcursor, " ipv6 pool %s\n", pool_node->poolname);
			*ifcursor = *showif + *ifLen;	
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
	}	

	return;
}
#else
void 
dhcp6_dbus_profile_config_interface_save
(
	char **showif,
	char **ifcursor,
	struct dcli_ifname* ifhead,
	struct dcli_pool* pool_node,
	int *ifLen
)
{
	char ifname[25];
#ifdef _D_WCPSS_
	int vrrid = 0;
	int ebrid = 0;
	int wlanid = 0;
	int radioid = 0;
	int wtpid = 0;
	int ret = 0;
#endif
	while (ifhead) {
		memset(ifname , 0, 25);
		memcpy(ifname, ifhead->interface_name,strlen(ifhead->interface_name));
#ifdef _D_WCPSS_
        if (!strncasecmp(ifname,"wlan",4)){
			ret = parse_hansi_wlan_ebr_ifname(ifname+4,&vrrid, &wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"wlan%d",wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;		
				}		
			}else
				continue;

		}
        else if (!strncasecmp(ifname,"radio",5)){
			ret = parse_hansi_radio_ifname(ifname+5,&vrrid, &wtpid,&radioid,&wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"radio%d-%d.%d",wtpid,radioid,wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else
				continue;
		}
        else if (!strncasecmp(ifname,"ebr",3)){
			ret = parse_hansi_wlan_ebr_ifname(ifname+3,&vrrid, &ebrid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"ebr%d",ebrid);	
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else
				continue;
		}
#endif
		*ifLen += sprintf(*ifcursor, "interface %s\n", ifname);
		*ifcursor = *showif + *ifLen;
		*ifLen += sprintf(*ifcursor, " ipv6 pool %s\n", pool_node->poolname);
		*ifcursor = *showif + *ifLen;		
		
		*ifLen += sprintf(*ifcursor, " exit\n");
		*ifcursor = *showif + *ifLen;
#ifdef _D_WCPSS_
		if ((vrrid > 0)&&((!strncasecmp(ifname,"wlan",4))||(!strncasecmp(ifname,"radio",5))||(!strncasecmp(ifname,"ebr",3)))){
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
#endif	
		*ifLen += sprintf(*ifcursor, "\n");
		*ifcursor = *showif + *ifLen;

		ifhead = ifhead->next;
	}

}
#endif
/*****************************************************************************
 *	u128ip2str
 * 
 *	IPv4 address to string (EXP: 0x0a01010a -> 10:1::1:10)
 *
 *  INPUT:
 *		u128_ipaddr - IPv6 address 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 char * - ipv6 address string
 * 	 NULL - failed
 *
 ****************************************************************************/
const char *u128ip2str(unsigned char* u128_ipaddr)
{
	static char
		pbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	
	return inet_ntop(AF_INET6, u128_ipaddr, pbuf, sizeof(pbuf));
	log_fatal(" u128ip2str ->   piaddr():error.");
		/* quell compiler warnings */
	return NULL;
}

unsigned int
dhcp_dbus_four_char_to_int
(
	unsigned char* in_char,
	unsigned int *out_int
)
{
	if (!in_char) {
		return 1;
	}
	unsigned char *tmpchar = in_char;
	
	*out_int = 	(tmpchar[0]<<24) |
				(tmpchar[1]<<16) |
				(tmpchar[2]<<8) |
				(tmpchar[3]);

	return 0;
}

unsigned int
dhcp6_dbus_four_char_to_int
(
	unsigned char* in_char,
	unsigned char *out_char
)
{
	if (!in_char) {
		return 1;
	}
	unsigned char *tmpchar = in_char;
	memcpy(out_char , tmpchar , 16);

	return 0;
}
unsigned char *
dhcp6_dbus_ip_mask_to_net
(
	unsigned char* ip,
	unsigned char* mask
)
{
	int i = 0 ;
	if(!ip || !mask)
		return NULL;

	for(;i < 16; i++){
		ip[i] = ip[i] & mask[i];
	}

	return ip;
}
unsigned int 
dcli_dhcp_int_to_four_char
(
	unsigned int in_int,
	unsigned char *out_char
)
{
	if (! out_char) {
		return 1;
	}
	out_char[0] = (in_int>>24) & 0xFF;
	out_char[1] = (in_int>>16) & 0xFF;
	out_char[2] = (in_int>>8) & 0xFF;
	out_char[3] = in_int & 0xFF;
		
	return 0;
}

/*return 0 means ip have a interface*/
unsigned int
dhcp6_dbus_check_interface
(
	struct iaddr *ipaddr,
	struct iaddr *ipmask,
	char*** ifname,
	int* ifnum
)
{
	struct iface_conf_list ifaces;
	struct iface_info info;	
	int err= 0, findnum = 0;
	struct iaddr ifaddr, ifaddrmask;
	unsigned int i = 0, ret = 1, len = 0, if_num = 0, num = 0;
	unsigned char mask;

	if (!ipaddr || !ipmask) {
		log_error("no ip range check none \n");
		return 1;
	}
	
	again:
	*ifnum = if_num;
	log_debug("ifnum is %d \n", if_num);
	if (!begin_iface_scan(&ifaces)) {
		log_error("Can't get list of interfaces.");
	}

	while (next_iface(&info, &err, &ifaces)) {		
		struct sockaddr_in6 *a = (struct sockaddr_in6*)&info.addr;	
		struct sockaddr_in6 *b = (struct sockaddr_in6*)&info.addrmask;
		memset(&ifaddr, 0, sizeof(struct iaddr));
		memset(&ifaddrmask, 0, sizeof(struct iaddr));
/*		
struct	 sockaddr_in   ina
ina.sin_addr.s_addr£½inet_addr("164£®112£®175£®124")
printf("%s"inet_ntoa(ina.sin_addr));
*/	
		ifaddr.len = DHCP_IPV6_ADDRESS_LEN;
		memcpy(ifaddr.iabuf, &a->sin6_addr.s6_addr, ifaddr.len);
		memcpy(ifaddrmask.iabuf, &b->sin6_addr.s6_addr, ifaddr.len);
log_debug("interface name is  %s ip is %s\n", info.name, piaddr(ifaddr));		
		for (i = 0; i < ifaddr.len; i++) {			
			log_debug("ifaddr->iabuf[%d] is %x \n", i, ifaddr.iabuf[i]);
			log_debug("ipaddr->iabuf[%d] is %x \n", i, ipaddr->iabuf[i]);
			if (ipmask->iabuf[i] > ifaddrmask.iabuf[i]) {
				mask = ipmask->iabuf[i];
			}
			else {
				mask = ifaddrmask.iabuf[i];
			}
log_debug("ifaddrmask.iabuf[%d] is %x\n", i, ifaddrmask.iabuf[i]);			
			if ((ifaddr.iabuf[i]&mask) != (ipaddr->iabuf[i]&ipmask->iabuf[i])) {			
				/*when break i not ++*/
				break;
			}
		}
		log_debug("i is %d \n", i);
		if (ifaddr.len == i) {
			ret = 0;
			if (findnum) {
				if (! *ifname) {				
					log_debug("malloc *ifname %d \n", if_num);
					*ifname = (char **)malloc(sizeof(char *) * if_num);
				}
				
				len = strlen(info.name);		
				log_debug("len is %d  info.name %s \n", len, info.name);
				((*ifname)[num]) = malloc(len + 1);
				memset(((*ifname)[num]), 0, len+1);
				memcpy(((*ifname)[num]), info.name, len);
				
				num ++;
				if_num --;
//				ret = 0;
			}
			else {
				if_num ++;
			}
//			(*ifname)[len] = '\0';
			
			/*check for new interface is in interface list yes or not		
			for (ip = interfaces; ip; ip = ip->next) {
				if (!strcmp (ip->name, *ifname)) {
					break;
				}
			}
				
			if (!ip) {
				result = interface_allocate (&ip, MDL);
				if (result != ISC_R_SUCCESS)
					log_fatal ("new_shared_network_interface %s: %s",
						   ifname, isc_result_totext (result));

				strcpy(ip->name, *ifname);
log_debug("add interface %s in interfaces list\n", *ifname);				
				if (interfaces) {
					interface_reference(&ip->next, interfaces, MDL);
					interface_dereference(&interfaces, MDL);
				}
				interface_reference(&interfaces, ip, MDL);
				ip->flags = INTERFACE_REQUESTED;
			}
			*********************************************************/
//			log_debug("len is %d  ifname %s \n", len, ifname);
		}
		if (ret && !if_num){
			ret = 1;
		}
	}

//	*iflen = len; 
	end_iface_scan(&ifaces);

	if (!ret && if_num) {
		findnum = 1;
		goto again;
	}
	
	return ret;
}

unsigned int 
dhcp6_dbus_pool_interface_state
(
	char *name,
	unsigned int *pool_state
)
{
	unsigned int ret = 0;	
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;

	ret = dhcp6_dbus_find_subnet_by_ifname(name, &subnode);
	if (!ret) {
		poolnode = subnode->attribute_pool;
		*pool_state = poolnode->state;
		return 0;
	}

	return 1;
}


/**********************************************************************************
 *  dhcp6_dbus_whether_same_subnet
 *
 *	DESCRIPTION:
 * 		whether the same subnet
 *
 *	INPUT:
 *		check_sub -> the check subnet
 *		pool_sub  -> pool node subnet
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1 -> same subnet
 *		0 -> not same subnet
 *		
 **********************************************************************************/
unsigned int dhcp6_dbus_whether_same_subnet
(
	struct dcli_subnet *check_sub,
	struct dcli_subnet *pool_sub
)
{
	unsigned char check_ip[16] = {0}, check_mask[16] = {0};
	unsigned char ip[16] = {0}, mask[16] = {0};

	if ((NULL == check_sub) || (NULL == pool_sub)) {
		return 1;
	}	
	dhcp6_dbus_four_char_to_int(check_sub->ipaddr.iabuf, check_ip);
	dhcp6_dbus_four_char_to_int(check_sub->mask.iabuf, check_mask);

	dhcp6_dbus_four_char_to_int(pool_sub->ipaddr.iabuf, ip);
	dhcp6_dbus_four_char_to_int(pool_sub->mask.iabuf, mask);
	dhcp6_dbus_ip_mask_to_net(check_ip,check_mask);
	dhcp6_dbus_ip_mask_to_net( ip, mask);
	
	if (memcmp(check_mask , mask ,16) || memcmp(check_ip , ip , 16)) {
		log_info("pool subnet %s, check subnet %s\n", piaddr(pool_sub->ipaddr), piaddr(check_sub->ipaddr));
		return 0;
	} else {
		//return 1;  not suport devided net type for temporarily
		return 0;
		
	}
}

/**********************************************************************************
 *  dhcp6_debus_is_same_subnet_exist
 *
 *	DESCRIPTION:
 * 		whether the same subnet is exist
 *
 *	INPUT:
 *		check_sub -> the add subnet
 *		poolname  -> pool name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1 -> same subnet exists
 *		0 -> same subnet not exists
 *		
 **********************************************************************************/
unsigned int dhcp6_debus_is_same_subnet_exist
(
	struct dcli_subnet *check_sub,
	char *poolname
)
{
	struct dcli_pool* head = head_pool.next;
	unsigned char check_sub_ip[16] = {0}, check_sub_mask[16] = {0};
	unsigned char headsub_ip[16] = {0}, headsub_mask[16] = {0};
	unsigned char* mask = {0};

	if ((NULL == check_sub) || (NULL == poolname)) {
		return 0;
	}
		
	dhcp6_dbus_four_char_to_int(check_sub->ipaddr.iabuf, check_sub_ip);
	dhcp6_dbus_four_char_to_int(check_sub->mask.iabuf, check_sub_mask);				
    for (; head; head = head->next) {
		if (head->headsubnet && strcmp(head->poolname, poolname)) {	

			dhcp6_dbus_four_char_to_int(head->headsubnet->ipaddr.iabuf, headsub_ip);
			dhcp6_dbus_four_char_to_int(head->headsubnet->mask.iabuf, headsub_mask);
			mask = dhcp6_dbus_ip_mask_to_net(check_sub_mask , headsub_mask);
		
			dhcp6_dbus_ip_mask_to_net(check_sub_ip, mask);
		
			dhcp6_dbus_ip_mask_to_net(headsub_ip, mask);
			log_debug("check ip subnet %s, mask %s <--> old ip subnet %s, mask %s, mask %s\n", 
					u128ip2str(check_sub_ip), u128ip2str(check_sub_mask), u128ip2str(headsub_ip), u128ip2str(headsub_mask), u128ip2str(mask));
			if (!memcmp(check_sub_ip , headsub_ip ,16)) {
			    log_error("same subnet %s already exist\n", piaddr(head->headsubnet->ipaddr));
				return 1;
			}
		}
	}
	return 0;
}

unsigned int
dhcp6_dbus_check_pool_ip
(
	struct dcli_subnet *check_sub,
	char *poolname
)
{
	struct dcli_subnet *head_sub = NULL;
	struct dcli_pool* head = head_pool.next;
	unsigned char check_sub_low_ip[16] = {0}, check_sub_high_ip[16] = {0}, check_sub_mask[16] = {0};
	unsigned char headsub_low_ip[16] = {0}, headsub_high_ip[16] = {0}, headsub_mask[16] = {0};
	int check_ipv6_h = 0 ,check_ipv6_l = 0 , headsub_ipv6_h = 0, headsub_ipv6_l = 0;


	dhcp6_dbus_four_char_to_int(check_sub->lowip.iabuf, check_sub_low_ip);
	dhcp6_dbus_four_char_to_int(check_sub->highip.iabuf, check_sub_high_ip);
	dhcp6_dbus_four_char_to_int(check_sub->mask.iabuf, check_sub_mask);
	
	for (; head; head = head->next) {
		if (head->headsubnet && !strcmp(head->poolname, poolname)) { 		
			for (head_sub = head->headsubnet; head_sub; head_sub = head_sub->next) {

				dhcp6_dbus_four_char_to_int(head_sub->lowip.iabuf, headsub_low_ip);
				dhcp6_dbus_four_char_to_int(head_sub->highip.iabuf, headsub_high_ip);
				dhcp6_dbus_four_char_to_int(head_sub->mask.iabuf, headsub_mask);

				log_debug("check low ip  %s, check high ip %s <--> old low ip %s, old high ip %s\n", 
						u128ip2str(check_sub_low_ip), u128ip2str(check_sub_high_ip), u128ip2str(headsub_low_ip), u128ip2str(headsub_high_ip));

				/*check add range is exist*/
				check_ipv6_l = ((check_sub_low_ip[12]<<24)|(check_sub_low_ip[13]<<16)|(check_sub_low_ip[14]<<8)|(check_sub_low_ip[15]));
				check_ipv6_h = ((check_sub_high_ip[12]<<24)|(check_sub_high_ip[13]<<16)|(check_sub_high_ip[14]<<8)|(check_sub_high_ip[15]));
				headsub_ipv6_l = ((headsub_low_ip[12]<<24)|(headsub_low_ip[13]<<16)|(headsub_low_ip[14]<<8)|(headsub_low_ip[15]));
				headsub_ipv6_h = ((headsub_high_ip[12]<<24)|(headsub_high_ip[13]<<16)|(headsub_high_ip[14]<<8)|(headsub_high_ip[15]));
				if ((check_ipv6_l >= headsub_ipv6_l && check_ipv6_l <= headsub_ipv6_h) ||
					(check_ipv6_h >= headsub_ipv6_l && check_ipv6_h <= headsub_ipv6_h)) {
					log_error("add range Duplicate old subnet\n");
					return 0;
				}
			}
		}
	}

	return 1;
}

int dhcp6_dbus_set_host
(
	struct dcli_host* addhost,
	unsigned int add
)
{
	/*test for add host*/
	char* hostname = addhost->hostname;
	/*need for check ip is in our pool,latter do it*/
	struct iaddr* hostipaddr = &(addhost->hostip);
	struct hardware* hosthwaddr = NULL;
	
/*
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, hostipaddr, sizeof(struct iaddr));
	ret = find_subnet(&subnetop, addr, MDL);
	if (ret) {
	*/
	if (add) {
		hosthwaddr = malloc(sizeof (struct hardware));
		memset(hosthwaddr, 0, sizeof (struct hardware));
		memcpy(&hosthwaddr->hbuf[1], addhost->hostmac, DHCP_MAC_ADDRESS_LEN);
		hosthwaddr->hlen = 7;
		hosthwaddr->hbuf[0] = HTYPE_ETHER;
		parse_host_declaration_test(hostname, hostipaddr, hosthwaddr, root_group);
	}
	else {
		log_debug("dhcp_dbus_set_host delete static host name is %s \n", hostname);
		delete_host_by_name(hostname);
	}
	if(hosthwaddr){
		free(hosthwaddr);
		hosthwaddr=NULL;
	}
	return 0;
}

int dhcp6_dbus_set_option
(
	struct dcli_subnet* addsubnet,
	struct dcli_option* owned_option,
	unsigned int del
)
{
	struct subnet* subnetop = NULL;
	struct group* addgroup = NULL;
	struct iaddr addr;
	int ret = 0, count = 1, global = 0;
	struct dcli_pool* poolnode = NULL;
	unsigned int test = 0;
	char *maxtime = "max-lease-time";
	char *routers = "routers";
	char *winsip = "netbios-name-servers";

	char *defaulttime = "default-lease-time";
	char *nameservers = "name-servers";
	char *domainsearch = "domain-search";
	
	char *option43 = "vendor-encapsulated-options";
	char *option52 = "capwap-ac-v6";

	if (!owned_option) {
		log_error("error owned_option is null \n");
		return 1;
	}
	
	if (addsubnet) {
		poolnode = addsubnet->attribute_pool;
		/*
		addr.iabuf[0] = addsubnet->ipaddr.iabuf[0];
		addr.iabuf[1] = addsubnet->ipaddr.iabuf[1];
		addr.iabuf[2] = addsubnet->ipaddr.iabuf[2];
		addr.iabuf[3] = addsubnet->ipaddr.iabuf[3];
		addr.len = addsubnet->ipaddr.len;*/
		
		memset(&addr, 0, sizeof(struct iaddr));
		memcpy(&addr, &(addsubnet->ipaddr), sizeof(struct iaddr));
		log_debug("find addr for set option addsubnet ip addr is %s \n",  piaddr(addr));
		ret = find_subnet(&subnetop, addr, MDL);
		if (ret) {
			addgroup = subnetop->group;
			/*
			log_debug("find subnet success \n");	
			set_server_option(maxtime, testmaxtime, testlen, 0,subnetop->group);
			set_server_option(routername, (char *)(&addr), testlen, 1, subnetop->group);
			set_server_option(domainsearch, testvalue, testlen, 1, subnetop->group);	
			*/
		}
		else {
			log_error("can not find the subnet\n");
			return 1;
		}
	}
	else {
		global = 1;
		addgroup = root_group;
	}
	
	if (owned_option->defaulttime) {
		ret = set_server_option(defaulttime, 0, (char *)(&(owned_option->defaulttime)), 0, 0, 0, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					global_server_option.defaulttime = 0;
				}
				else {
					global_server_option.defaulttime = owned_option->defaulttime;
				}
			}
			else {
				if (del) {
					poolnode->owned_option.defaulttime = 0;
				}
				else {
					poolnode->owned_option.defaulttime = owned_option->defaulttime;
					log_debug("defaulttim~~~~~~~~~~~~> %d\n", owned_option->defaulttime);	
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->maxtime) {
		ret = set_server_option(maxtime, 0, (char *)(&(owned_option->maxtime)), 0, 0, del, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					global_server_option.maxtime = 0;
				}
				else {
					global_server_option.maxtime = owned_option->maxtime;
				}
			}
			else {
				if (del) {
					poolnode->owned_option.maxtime = 0;
				}
				else {					
					poolnode->owned_option.maxtime = owned_option->maxtime;
				}
			}
		}
		else {
			return 1;
		}
	}
	
	if (owned_option->domainsearch) {
		log_debug("add owned_option--------->domainsearch name is %s \n", owned_option->domainsearch);		
		ret = set_server_option(domainsearch, 1, owned_option->domainsearch, 0, 1, del, addgroup);			
		if (!ret) {
			if (global) {
				if (del) {
					free(global_server_option.domainsearch);
					free(owned_option->domainsearch);
					global_server_option.domainsearch = NULL;
				}
				else {					
					global_server_option.domainsearch = owned_option->domainsearch;
				}
			}
			else {
				if (del) {
					free(poolnode->owned_option.domainsearch);
					free(owned_option->domainsearch);
					poolnode->owned_option.domainsearch = NULL;
				}
				else {					
					poolnode->owned_option.domainsearch = owned_option->domainsearch;
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->option43.ipnum) {
		ret = set_server_option(option43, 1, (char *)(&(owned_option->option43)), 0, 1, del, addgroup);			
		if (!ret) {
			if (global) {
				if (del) {
					memset(&(global_server_option.option43), 0, sizeof(global_server_option.option43));
					free(global_server_option.option43.ip);
				}
				else {					
					global_server_option.option43 = owned_option->option43;
				}
			}
			else {
				if (del) {
					memset(&(poolnode->owned_option.option43), 0, sizeof(poolnode->owned_option.option43));
					free(poolnode->owned_option.option43.ip);
				}
				else {					
					poolnode->owned_option.option43 = owned_option->option43;
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->option52[0].len){		
		ret = set_server_option(option52,  1, (char *)(&(owned_option->option52)), 0, 1, del, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					memset(global_server_option.option52, 0, sizeof(struct iaddr) * 8);					
				}
				else {			
					memcpy(global_server_option.option52, owned_option->option52, sizeof(struct iaddr) * 8);	
				}
			}
			else {
				if (del) {
					memset(poolnode->owned_option.option52, 0, sizeof(struct iaddr) * 8);
				}
				else {					
					memcpy(poolnode->owned_option.option52, owned_option->option52, sizeof(struct iaddr) * 8);
				}
			}
		}
		else {
			return 1;
		}
	}
	
	if (owned_option->routersip.len) {
		ret = set_server_option(routers, 1, (char *)(&(owned_option->routersip)), 0, 1, del, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					memset(&(global_server_option.routersip), 0, sizeof(struct iaddr));
				}
				else {					
					global_server_option.routersip = owned_option->routersip;
					dhcp_dbus_four_char_to_int(global_server_option.routersip.iabuf, &test);
					log_debug("test for router ip is %x \n", test);
					test = 0;				
					dhcp_dbus_four_char_to_int(&(owned_option->routersip.iabuf[0]), &test);
					log_debug("test for router ip is %x \n", test);
					log_debug("test for router ip is %x.%x.%x.%x \n",owned_option->routersip.iabuf[0],
						owned_option->routersip.iabuf[1],
						owned_option->routersip.iabuf[2],
						owned_option->routersip.iabuf[3]);
				}
			}
			else {
				if (del) {
					memset(&(poolnode->owned_option.routersip), 0, sizeof(struct iaddr));
				}
				else {					
					poolnode->owned_option.routersip = owned_option->routersip;
				}
			}
		}
		else {
			return 1;
		}
	}
	
	if (owned_option->winsip.len) {
		//must not same as routers
	
		ret = set_server_option(winsip, 1, (char *)(&(owned_option->winsip)), 0, 1, del, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					memset(&(global_server_option.winsip), 0, sizeof(struct iaddr));
				}
				else {					
					global_server_option.winsip = owned_option->winsip;
				}
			}
			else {
				if (del) {
					memset(&(poolnode->owned_option.winsip), 0, sizeof(struct iaddr));
				}
				else {					
					poolnode->owned_option.winsip = owned_option->winsip;
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->dnsnum) {
		if(owned_option->dnsnum == 1) {			
			ret = set_server_option(nameservers, 1, (char *)(owned_option->dnsip), 0, 1, del, addgroup);
		}
		else {
			count |= ((owned_option->dnsnum)<<8)&0xffff;
			ret = set_server_option(nameservers, 1, (char *)(owned_option->dnsip), count, 1, del, addgroup);
		}

		if (!ret) {
			if (global) {
				if (del) {
					if(global_server_option.dnsip){
					free(global_server_option.dnsip);
					global_server_option.dnsip=NULL;
					}
					if(owned_option->dnsip){
					free(owned_option->dnsip);
					owned_option->dnsip=NULL;
					}
					global_server_option.dnsnum = 0;
				}
				else {					
					global_server_option.dnsip = owned_option->dnsip;
					global_server_option.dnsnum = owned_option->dnsnum;
				}
			}
			else {
				if (del) {
					if(poolnode->owned_option.dnsip){
					free(poolnode->owned_option.dnsip);
					poolnode->owned_option.dnsip = NULL;
					}
					if(owned_option->dnsip){
					free(owned_option->dnsip);
					owned_option->dnsip=NULL;
					}
					
					poolnode->owned_option.dnsnum = 0;
				}
				else {					
					poolnode->owned_option.dnsip = owned_option->dnsip;
					poolnode->owned_option.dnsnum = owned_option->dnsnum;
				}
			}
		}
		else {
			return 1;
		}
		/*
		if (owned_option->dnsnum > 1) {
			count = owned_option->dnsnum;
			value = malloc(sizeof(struct iaddr) * count);
			memset(value, 0, sizeof(struct iaddr) * count);
			for (i = 0; i < count; i++) {
				memcpy(value + i*sizeof(struct iaddr), owned_option->dnsip, sizeof(struct iaddr));
			}
			set_server_option(dnsip, value, count, 1, addgroup);
		}
		else {
			set_server_option(dnsip, (char *)(owned_option->dnsip), 0, 1, addgroup);
		}
		*/
	}
		
	return 0;
}

unsigned int
dhcp6_dbus_set_subnet
(
	struct dcli_subnet *addsubnet
)
{
	
	memset(&sub_conf, 0, sizeof(struct subnet_config));
	
	sub_conf.net = addsubnet->ipaddr;	
	sub_conf.netmask = addsubnet->mask;		
	sub_conf.range_conf.high = addsubnet->highip;	
	sub_conf.range_conf.low = addsubnet->lowip; 
	
	/* Make sure that high and low addresses are in this subnet. */
	if (!addr_eq((sub_conf.net), subnet_number((sub_conf.range_conf.low), (sub_conf.netmask)))) {
		log_error("bad range, low address  not in subnet  netmask \n");
		return 1;
	}

	if (!addr_eq((sub_conf.net), subnet_number((sub_conf.range_conf.high), (sub_conf.netmask)))) {
		log_error("bad range, high address  not in subnet  netmask \n");
		return 1;
	}

	parse_statement_subnet_no_file_v6(&sub_conf, root_group);

	return 0;
}

unsigned int 
dhcp6_dbus_interface_bind_pool
(
	struct dcli_pool *poolnode,
	char* interface_name
)
{
	unsigned int ret = 0;
	struct dcli_ifname *ifnode = NULL;
	struct dcli_subnet *subnode = NULL, *subNode = NULL;
	
	if (!poolnode||!interface_name ) {
		return 1;
	}

	if (poolnode->headsubnet) {
		subnode = poolnode->headsubnet;
	}
	else {
		log_error("pool has no subnet\n");
		return 1;
	}
	while (subnode) {
		ret = dhcp6_dbus_find_subnet_by_ifname(interface_name, &subNode);
		/*one subnet one interface, if want to support more interface,
			 delete || subnode->interface_num*/
		if (!ret || subnode->interface_num) {
			log_debug("interface %s have this subnet before\n", interface_name);
			subnode = subnode->next;
			continue;
		}
		ret = discover_interfaces_test(1, interface_name);
		if (!ret) {
			ifnode = malloc(sizeof(struct dcli_ifname));
			memset(ifnode, 0, sizeof(struct dcli_ifname));
			ifnode->interface_name = malloc(strlen(interface_name) + 1);
			memset(ifnode->interface_name, 0, (strlen(interface_name) + 1));
			memcpy(ifnode->interface_name, interface_name, strlen(interface_name));
			if (subnode->ifhead) {
				ifnode->next = subnode->ifhead;
				subnode->ifhead = ifnode;
			}
			else {
				subnode->ifhead = ifnode;
			}
			subnode->interface_num++;
			poolnode->refcount++;
			
			return 0;
		}
		subnode = subnode->next;
	}
	
	return 1;
}

unsigned int 
dhcp6_dbus_interface_unbind_pool
(
	struct dcli_subnet *subnode,
	char* interface_name
)
{
	struct dcli_ifname *ifprev = NULL, *ifnode = NULL;
	
	if (!subnode||!interface_name ) {
		return 1;
	}
	
	ifnode = subnode->ifhead;

	while (ifnode) {
		if (!strcmp(interface_name, ifnode->interface_name)) {
			if (ifprev) {
				ifprev->next = ifnode->next;
			}
			else {
				subnode->ifhead = ifnode->next;
			}
			free(ifnode->interface_name);
			free(ifnode);

			subnode->interface_num--;
			test_for_del_subnet_interface(interface_name);

			break;
		}

		ifprev = ifnode;
		ifnode = ifnode->next;
	}

	return 0;
}

unsigned int
string_to_option43_s
(
	char* str,
	struct dcli_option43* option
)
{
	unsigned char type[3] = {0,0,0}, len[3] = {0,0,0}, ip[9];
	unsigned int size = 0, i = 0, num = 0; 
	char* endstr = NULL;

	if ((!str) || (strlen(str) < 12)) {
		return 1;
	}
log_debug("string_to_option43_s run 1111");
	memcpy(type, str, 2);
	memcpy(len, &(str[2]), 2);
	size = strtoul((char *)len, &endstr, 16);
	if (((strlen(str) - 4)%8) || 
		((strlen(str) - 4 ) != size*2)){
		return 1;
	}
	else {
		log_debug("string_to_option43_s run 2222");
		option->type = strtoul((char *)type, &endstr, 16);	
		option->len = strtoul((char *)len, &endstr, 16);
		num = (option->len)/4;
		/*most option43 ip num is 30*/
		if (num > 30) {
			num = 30;
		}
		option->ipnum = num;
		option->ip = malloc(4*num);
		memset(option->ip, 0, 4*num);
		log_debug("string_to_option43_s run 333");
		for(i = 0; i < num; i++ ) {
			memset(ip, 0, 9);
			memcpy(ip, &(str[4 + i*8]), 8);
			option->ip[i] = strtoul((char *)ip, &endstr, 16);
		}
	}

	return 0;
}

unsigned int 
add_interface_failover_to_backup_list
(
	struct dcli_subnet *subnode
)
{
//	struct failover_backup_list* backup = NULL;
/*	
	backup = malloc(sizeof(struct failover_backup_list));
	memset(backup, 0, sizeof(struct failover_backup_list));

	backup->interface_name = malloc(subnode->interface_num * 4);
	memset(backup->interface_name, 0, (subnode->interface_num * 4));
	for (i = 0; i < subnode->interface_num; i ++) {*/
/*	ifnode = subnode->ifhead;
	while (ifnode) { 
		backup->interface_name[i] = malloc(strlen(ifnode->interface_name) + 1);
		memset(backup->interface_name[i], 0, (strlen(ifnode->interface_name) + 1));
		memcpy(backup->interface_name[i], ifnode->interface_name, strlen(ifnode->interface_name));
		ifnode = ifnode->next;
		now one downlink interface one subnet*/
/*		backup->interface_num = 1;
		break;
	}
	backup->interface_num = subnode->interface_num;*/


	// backup->next = failover_backup_list_head.next;
	// failover_backup_list_head.next = backup;

	return 0;
}

unsigned int 
del_interface_failover_to_backup_list
(
	struct dcli_subnet *subnode
)
{
	struct failover_backup_list *prev = NULL, *backup = NULL;
	
	backup =  failover_backup_list_head.next;

	while ((backup && 
			(subnode->ifhead) && 
			(subnode->ifhead->interface_name))) {
			/*one subnet one interface*/
		if (!strcmp(backup->dhcp_interface[0].if_name, subnode->ifhead->interface_name)) {
			if (!prev) {
				failover_backup_list_head.next = backup->next;
			}
			else {
				prev->next = backup->next;
			}

			free(backup);
			return 0;
		}
		prev = backup;
		backup = backup->next;
	}

	return 1;
}

unsigned int 
find_backup_list_by_vrrpid
(
	unsigned int vrrpid,
	struct failover_backup_list **backnode
)
{
	struct failover_backup_list *backup = NULL;
	
	backup =  failover_backup_list_head.next;

	while (backup) {
		if (backup->vrrpid == vrrpid) {
			*backnode = backup;
			return 0;
		}
		backup = backup->next;
	}

	return 1;
}

unsigned int 
find_backup_list_by_failnode
(
	struct dcli_failover *failnode,
	struct failover_backup_list **backnode
)
{
	struct failover_backup_list *backup = NULL;

	backup =  failover_backup_list_head.next;

	while (backup) {/*
		if (backup->owned_failover == failnode) {
			*backnode = backup;
			return 0;
		}*/
		backup = backup->next;
	}

	return 1;
}

unsigned int
dhcp6_dbus_find_pool_by_name
(
	char* name,
	struct dcli_pool** poolnode
)
{
	struct dcli_pool* head = head_pool.next;

	if (!name) {
		return 1;
	}
	
    for (; head; head = head->next) {
		log_debug("poolname is %s, name is %s \n", head->poolname, name);
		if (!strcmp(head->poolname, name)) {
			*poolnode = head;
			return 0;
		}
	}

	return 1;
}


/**********************************************************************************
 *  check_interfaces_ip_address
 *
 *	DESCRIPTION:
 * 		Check The interface whether Settings IP address
 *
 *	INPUT:
 *		name -> the interface name 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1 -> the interface have set ip address
 *		0 -> the interface without setting the IP address  
 **********************************************************************************/
int check_interfaces_ip_address(char *name) 
{
	static int sock = -1;
	struct ifreq tmp;
	struct sockaddr_in *addr = NULL;

	if (NULL == name) {
		return 0;
	}
	if (sock < 0) {
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock < 0) {
			return 0;
		}
	}

	strcpy(tmp.ifr_name, name);
	if (ioctl(sock, SIOCGIFADDR, &tmp) < 0) {
		log_error("dhcp ioctl check interface %s address failed: %m\n", name);
		close(sock);
		sock = -1;
		
		return 0;
	}

	addr = (struct sockaddr_in*)&tmp.ifr_addr;
	if (0 != addr->sin_addr.s_addr) {
		return 1;
	}

	return 0;
}

void dhcp6_dbus_profile_config_hansi_save(char *showStr, int slot, int vrrp, int local_flag)
{
	char ifname[32];
	int value1 = 0, value2 = 0, value3 = 0, value4 = 0, value5 = 0;
	int totalLen = 0;
	int ret = 0;
	struct dcli_pool* pool_node = NULL;
	struct dcli_ifname* ifhead = NULL;
	char *cursor = NULL;

	if (!showStr) {
		return;
	}

	cursor = showStr;
	log_debug("show hansi running: slot %d vrrp %d local flag %d\n", slot, vrrp, local_flag);
	
	/*ip pool conifgure*/
	pool_node = head_pool.next;
	for (; pool_node; pool_node = pool_node->next) {	
		if (pool_node->headsubnet) {
			if (pool_node->headsubnet->ifhead) {
				ifhead = pool_node->headsubnet->ifhead;

				//if (1 != check_interfaces_ip_address(ifhead->interface_name)) {
				//	continue;
				//}
				
				memset(ifname, 0, sizeof(ifname));
				memcpy(ifname, ifhead->interface_name, strlen(ifhead->interface_name));
				log_debug("show running hansi cfg: interface %s\n", ifname);
			//	log_info("show dhcp ipv6666666666666666666  running hansi cfg: interface %s\n", ifname);
				
				if (0 == strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && local_flag) { /* local hansi */
						log_debug("2 slot %d, vrrp %d\n", slot, vrrp);
						if (vrrp) {
							totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface wlan%d\n", value3);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					} 
				} 

				else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
					ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
						log_debug("3 slot %d, vrrp %d\n", slot, vrrp);
						if (vrrp) {
							totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface wlan%d\n", value3);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				} 

				#if 0
				if (0 == strncmp(ifname, "wlan", 4)) {
					ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
					if ((2 == ret) && (vrrp == value1) && local_flag) { /* local hansi */
						log_debug("2 slot %d, vrrp %d\n", slot, vrrp);
						if (vrrp) {
							totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface wlan%d\n", value2);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ip pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					} 
					else if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						log_debug("3 slot %d, vrrp %d\n", slot, vrrp);
						if (vrrp) {
							totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface wlan%d\n", value3);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ip pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				} 
				#endif

				#if 1
				else if (0 == strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && local_flag) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface ebr%d\n", value2);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				}

				else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
					ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface ebr%d\n", value3);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				}

				
				#else
				else if (0 == strncmp(ifname, "ebr", 3)) {
					ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
					if ((2 == ret) && (vrrp == value1) && local_flag) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface ebr%d\n", value2);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ip pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
					else if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface ebr%d\n", value3);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ip pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				}
				#endif

				else if (0 == strncmp(ifname, "r", 1)) { /* r1-1-1-0.1 */
					ret = sscanf(ifname, "r%d-%d-%d-%d.%d", &value1, &value2, &value3, &value4, &value5);
					if ((4 == ret) && (vrrp == value1) && local_flag) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}

						totalLen += sprintf(cursor, " interface r%d-%d.%d\n", value2, value3, value4);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					} 
					else if ((5 == ret) && (slot == value1) && (vrrp == value2)) {
						if (vrrp) {
							totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
							cursor = showStr + totalLen;
						}
						
						totalLen += sprintf(cursor, " interface r%d-%d.%d\n", value3, value4, value5);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " ipv6 pool %s\n", pool_node->poolname);
						cursor = showStr + totalLen;

						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;

						if (vrrp) {
							totalLen += sprintf(cursor, " exit\n");
							cursor = showStr + totalLen;
						}
					}
				}
			}
		}
	}

	return ;
}


DBusMessage* 
dhcp6_dbus_show_running_hansi_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError err;   		
	char *strShow = NULL;
	unsigned int slot_id = 0;
	unsigned int vrrp_id = 0;
	unsigned int local_flag = 0;

	dbus_error_init(&err);

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_UINT32, &slot_id,
					DBUS_TYPE_UINT32, &vrrp_id,	
					DBUS_TYPE_UINT32, &local_flag,					
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	strShow = (char*)malloc(DHCP_SAVE_CFG_MEM);	
	if(!strShow) {
		log_debug("%s:%d malloc memory failed\n", MDL);
		return NULL;
	}
	memset(strShow, 0, DHCP_SAVE_CFG_MEM);

	dbus_error_init(&err);
	dhcp6_dbus_profile_config_hansi_save(strShow, slot_id, vrrp_id, local_flag);

	log_debug("%s\n", __func__);
	log_debug("%s\n", strShow);	

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	
	return reply;
}

unsigned int
dhcp6_dbus_find_static_host_by_addr
(
	struct iaddr* addr,
	char* mac,
	struct dcli_host** stahost
)
{
	struct dcli_host* head = head_host.next;

    for (; head; head = head->next) {
		if (!memcmp(&(head->hostip), addr, sizeof(struct iaddr))
			|| !memcmp(head->hostmac, mac, DHCP_MAC_ADDRESS_LEN)) {
			*stahost = head;
			return 0;
		}
	}

	return 1;
}

unsigned int
dhcp6_dbus_find_static_host_by_addr_all
(
	struct iaddr* addr,
	char* mac,
	struct dcli_host** stahost
)
{
	struct dcli_host* head = head_host.next;

    for (; head; head = head->next) {
		if (!memcmp(&(head->hostip), addr, sizeof(struct iaddr))
			&& !memcmp(head->hostmac, mac, DHCP_MAC_ADDRESS_LEN)) {
			*stahost = head;
			return 0;
		}
	}

	return 1;
}

unsigned int
dhcp6_dbus_delete_static_host_by_addr
(
	struct iaddr* addr,
	char* mac
)
{
	struct dcli_host * prevhost = NULL, *tmphost = NULL;

	tmphost = head_host.next;

    while (tmphost) {
		if (!memcmp(&(tmphost->hostip), addr, sizeof(struct iaddr))
			&& !memcmp(tmphost->hostmac, mac, DHCP_MAC_ADDRESS_LEN)) {
			if (!prevhost) {
				head_host.next = tmphost->next;
			}
			else {
				prevhost->next = tmphost->next;
			}
			free(tmphost);
			return 0;
		}

		prevhost = tmphost;
		tmphost = tmphost->next;
	}

	return 1;
}


unsigned int
dhcp6_dbus_find_pool_by_index
(
	unsigned int index,
	struct dcli_pool** poolnode
)
{
	struct dcli_pool* head = head_pool.next;
	
    for (; head; head = head->next) {
		if (head->index == index) {
			*poolnode = head;
			return 0;
		}
	}

	return 1;
}

unsigned int
dhcp6_dbus_create_pool_by_name
(
	char* name,
	struct dcli_pool** poolnode
)
{
	struct dcli_pool* head = NULL, *temppool = NULL;
	unsigned int ret = 0;
	if (!name || (count_pool >= DHCP6_MAX_POOL_NUM)) {
		if(name){
			ret = dhcp6_dbus_find_pool_by_name(name, &temppool);
			if(!ret){
				return 2;
			}
		}
		log_error("create ip pool parameter null, pool count %d\n", count_pool);		
		return 1;
	}
	ret = dhcp6_dbus_find_pool_by_name(name, &temppool);
	if (ret) {
		head = malloc(sizeof(struct dcli_pool));
		if(NULL == head){
			log_error("malloc error!\n");
			return 1;
		}
		memset(head, 0, sizeof(struct dcli_pool));
		
		head->next = head_pool.next;
		head_pool.next = head;	
		head->poolname = name;
		head->index = ++pool_index;
		count_pool++;
		*poolnode = head;
	}
	else {
		return 2;
	}
	
	return 0;
}

unsigned int
dhcp6_dbus_delete_pool_by_name
(
	char* name
)
{
	struct dcli_pool *prevpool = NULL, *temppool = NULL;
	struct dcli_subnet *tempsub = NULL;
	unsigned int ret = 0;
	
	if (!name) {
		return 1;
	}

	ret = dhcp6_dbus_find_pool_by_name(name, &temppool);
	if (!ret) {
		temppool = head_pool.next;
		while (temppool) {
			if (!strcmp(temppool->poolname, name)) {
				if (!temppool->refcount) {
					while (temppool->headsubnet) {		
						tempsub = temppool->headsubnet;
						test_for_del_subnet_pool(tempsub);
						temppool->headsubnet = tempsub->next;
						free(tempsub);
					}
					if (!prevpool) {
						head_pool.next = temppool->next;
					}
					else {
						prevpool->next = temppool->next;
					}
					count_pool--;
					free(temppool);
					return 0;
				}
				else {
					log_debug("binded by interface can not del\n");
					return 3;
				}
			}
			prevpool = temppool;
			temppool = temppool->next;
		}
	}
	else {
		log_debug("can not find the pool by name\n");
		return 2;
	}
	
	return 0;
}


unsigned int
dhcp6_dbus_create_static_host_by_name
(
	char* name,
	struct dcli_host** hostnode
)
{
	struct dcli_host* head = NULL;
	if (!name) {
		return 1;
	}

	head = malloc(sizeof(struct dcli_host));

	memset(head, 0, sizeof(struct dcli_host));
	head->next = head_host.next;
	head_host.next = head;
	head->hostname = name;
	*hostnode = head;
	count_host++;
	
	return 0;
}

unsigned int
dhcp6_dbus_find_subnet_by_ifname
(
	char* ifname,
	struct dcli_subnet** subnode
)
{
	struct dcli_pool* head = head_pool.next;
	struct dcli_subnet *head_sub = NULL;
	struct dcli_ifname *ifnode = NULL;
		
	if (!ifname) {
		return 1;
	}
	
    for (; head; head = head->next) {
		if (head->headsubnet) {
			for (head_sub = head->headsubnet; head_sub; head_sub = head_sub->next) {
				ifnode = head_sub->ifhead;
				while (ifnode) {
					if (!strcmp(ifnode->interface_name, ifname)) {
						*subnode = head_sub;
						return 0;
					}
					ifnode = ifnode->next;
				}
			}
		}
	}

	return 1;
}

unsigned int
dhcp6_dbus_delete_subnet_by_subip
(	
	struct dcli_subnet* delsub,
	struct dcli_pool* poolnode
)
{
	struct dcli_subnet *prevsub = NULL, *tmpsub = NULL;

	if (!delsub || !poolnode) {
		return 1;
	}
	tmpsub = poolnode->headsubnet;
    while (tmpsub) {
		if (!memcmp(&(tmpsub->lowip), &(delsub->lowip), sizeof(struct iaddr)) &&
			!memcmp(&(tmpsub->highip), &(delsub->highip), sizeof(struct iaddr)) &&
			!memcmp(&(tmpsub->mask), &(delsub->mask), sizeof(struct iaddr))) {
			if (!prevsub) {
				poolnode->headsubnet = tmpsub->next;
			}
			else {
				prevsub->next = tmpsub->next;
			}
				
			test_for_del_subnet_pool(tmpsub);
			poolnode->count_sub--;
			
			free(tmpsub);
			return 0;
		}

		prevsub = tmpsub;
		tmpsub = tmpsub->next;
	}

	return 1;
}

void 
dhcp6_read_netlink
(
	void* ptr,
	int totlemsglen
)
{
	struct  ifinfomsg *rtEntry = NULL;
	struct nlmsghdr *nlp = (struct nlmsghdr *)ptr;
	int payloadoff = 0, ret = 0;
	struct rtattr *rtattrptr = NULL;
	unsigned char *ifname = NULL, *ifaddr = NULL;
	unsigned int ifi_flags = 0, ifi_index = ~0UI;;
	struct dcli_subnet *subNode = NULL;
		
	for(;NLMSG_OK(nlp, totlemsglen);nlp=NLMSG_NEXT(nlp, totlemsglen)) {
		rtEntry = (struct ifinfomsg *) NLMSG_DATA(nlp);
		payloadoff = RTM_PAYLOAD(nlp);
		switch( nlp->nlmsg_type ) {
			case RTM_NEWLINK:
			break;

			case RTM_DELLINK:
				ifi_flags = rtEntry->ifi_flags;  /* get interface */
			  	ifi_index = rtEntry->ifi_index; /* ifindex for kernel*/
				rtattrptr = (struct rtattr *)IFLA_RTA(rtEntry);
				for(;RTA_OK(rtattrptr, payloadoff);rtattrptr=RTA_NEXT(rtattrptr,payloadoff)) {
					switch(rtattrptr->rta_type) {
						case IFLA_ADDRESS:
							ifaddr = (unsigned char*)RTA_DATA(rtattrptr);
	                        /*notify to npd to modified the sysmac and L3 static fdb entry*/
							log_debug("IFLA_ADDRESS mac address is: %02x%02x%02x%02x%02x%02x",	\
					                ifaddr[0],ifaddr[1],ifaddr[2],ifaddr[3],ifaddr[4],ifaddr[5]);
							break;
						case IFLA_IFNAME:
							ifname = (unsigned char*)RTA_DATA(rtattrptr);
							ret = dhcp6_dbus_find_subnet_by_ifname((char *)ifname, &subNode);
							if (!ret) {
								log_debug("no ip pool because no interface %s", ifname);
									dhcp6_dbus_interface_unbind_pool(subNode, (char *)ifname);
									subNode->attribute_pool->refcount--;
							}
							else {
								log_debug("no subnet for this interface %s", ifname);
							}
							break;
						default:
							/*npd_syslog_dbg("other value ignore %d\n",nlp->nlmsg_type);*/
	                        break;
					}
				}
			break;
			
			default:
			break;
		}		    
	}
}

void 
dhcp6_syn_kernelRT
(
	int sockfd
)
{
	int msglen=0;
	static char buf[4096];
	
	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl snl;
	struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
	
	char* p = buf;
	struct nlmsghdr *nlp = (struct nlmsghdr *)p, *tmp=NULL;
	while(1) {
		msglen = recvmsg(sockfd, &msg, 0);
		if(msglen <= 0) {
			log_error("recv msg return error\n");
			continue;
		}
		
		tmp = (struct nlmsghdr *) p;
		if(tmp->nlmsg_flags == NLM_F_MULTI && tmp->nlmsg_type == NLMSG_DONE  ) {
			log_error("in func recv_msgbuf_to_drv recv msg type is NLMSG_DONE\n");
			continue;
		}
		dhcp6_read_netlink(nlp, msglen);
	}

}

void *
dhcp6_receive_netlink
(
	void *arg
)
{
	int fd = -1;
	struct sockaddr_nl la;
	
	/* tell my thread id*/
	dhcp6_tell_whoami("netlinkDhcp6", 0);
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		log_error("create socket error when sync fib to asic\n");
		return NULL;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	/*
	#ifdef CPU_ARM_XCAT
	la.nl_pid += g_npd_thread_number;
	#endif
	*/
	la.nl_groups = RTMGRP_IPV4_IFADDR|RTMGRP_IPV4_ROUTE |RTMGRP_LINK| RTMGRP_NOTIFY ;

	if(bind(fd, (struct sockaddr*) &la, sizeof(la))) {
		log_error("bind socket error when sync fib to asic\n");
	}
	
	dhcp6_syn_kernelRT(fd);
	if(fd){
		close(fd);
	}
	return NULL;
}

void 
dhcp6_dbus_profile_config_save
(
	char* showStr
)
{
	char *cursor = NULL, *ifcursor = NULL;
	char *showif;
	int i = 0, totalLen = 0, ifLen = 0, j = 0, k = 0;
	struct dcli_pool* pool_node = NULL;
	struct dcli_subnet* sub_node = NULL;
	struct dcli_option* option_node = NULL;
	struct dcli_ifname* ifhead = NULL;
	char    *ipv6_dns[3] = {NULL, NULL, NULL} ;
	char *subnet_ip = NULL;
	char *range_low_ip = NULL;
	char *range_high_ip = NULL;
	unsigned int prefix_len = 0;
	char *dns_address = NULL;
	char option52[8][512];
	
	int ipv6_buf_len = (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") + 1);
	
	showif = (char*)malloc(512*1024);
	if(NULL == showif){
		return;
	}
	memset(showif, 0, 2*1024);
	
	ifcursor = showif;
	cursor = showStr;

	subnet_ip =malloc(ipv6_buf_len) ;
	if(NULL == subnet_ip){
		if(showif){
			free(showif);
			showif=NULL;
		}
		return;
	}
	range_low_ip =malloc(ipv6_buf_len) ;
	if(NULL == range_low_ip){
		if(showif){
			free(showif);
			showif=NULL;
		}
		if(subnet_ip){
			free(subnet_ip);
			subnet_ip=NULL;
		}
		return;
	}
	range_high_ip =malloc(ipv6_buf_len) ;
	if(NULL == range_high_ip){
		if(showif){
		free(showif);
		showif=NULL;
		}
		if(subnet_ip){
			free(subnet_ip);
			subnet_ip=NULL;
		}
		if(range_low_ip){
		free(range_low_ip);
		range_low_ip=NULL;
		}
		return;
	}
	dns_address =malloc(ipv6_buf_len) ;
	if(NULL == dns_address){
		if(showif){
		free(showif);
		showif=NULL;
		}
		if(subnet_ip){
			free(subnet_ip);
			subnet_ip=NULL;
		}
		if(range_low_ip){
		free(range_low_ip);
		range_low_ip=NULL;
		}
		if(range_high_ip){
		free(range_high_ip);
		range_high_ip=NULL;
		}
		return;
	}

	/*dhcp server enable*/
	if (dhcp_server_enable) {
		totalLen += sprintf(cursor, "ipv6 dhcp server enable\n\n");
		cursor = showStr + totalLen;
	}	
	
	/*global dhcp configure*/
	if (global_server_option.domainsearch) {			
		totalLen += sprintf(cursor, "ipv6 dhcp server domain %s\n", global_server_option.domainsearch);
		log_debug("domainsearch is -------> %s\n", global_server_option.domainsearch);
		cursor = showStr + totalLen;
	}
	
	if (global_server_option.option43.ipnum) {			
		totalLen += sprintf(cursor, " ipv6 dhcp server option43 %.2x%.2x", global_server_option.option43.type, global_server_option.option43.len);
		cursor = showStr + totalLen;
		for (i = 0; i < global_server_option.option43.ipnum; i++) {
			totalLen += sprintf(cursor, "%.8x", global_server_option.option43.ip[i]);
			cursor = showStr + totalLen;
		}
		totalLen += sprintf(cursor, "\n");
		cursor = showStr + totalLen;
	}

	if(global_server_option.option52[0].len){
		totalLen += sprintf(cursor, "ipv6 dhcp server option52");
		cursor = showStr + totalLen;
		for(i = 0; i < 8; i++){
			if(global_server_option.option52[i].len){
				inet_ntop(AF_INET6, global_server_option.option52[i].iabuf, option52[i] , INET6_ADDRSTRLEN);
				totalLen += sprintf(cursor," %s ", option52[i]);
				cursor = showStr + totalLen;
			}else{
				break;
			}
		}

		totalLen += sprintf(cursor, "\n");
		cursor = showStr + totalLen;
	}
	
	if (global_server_option.dnsnum) {	

		log_debug("dnsnum is ------------->%d\n", global_server_option.dnsnum);
		totalLen += sprintf(cursor, "ipv6 dhcp server dns");
		cursor = showStr + totalLen;
		for (i = 0; i < global_server_option.dnsnum; i++) {

			ipv6_dns[i] =malloc(128) ;
			 if(NULL == ipv6_dns[i]){
			 	if(dns_address){
					free(dns_address);
					dns_address=NULL;
				}
				if(range_high_ip){
					free(range_high_ip);
					range_high_ip=NULL;
				}
				if(range_low_ip){
					free(range_low_ip);
					range_low_ip=NULL;
				}
				if(subnet_ip){
					free(subnet_ip);
					subnet_ip=NULL;
				}
				if(showif){
					free(showif);
					showif=NULL;
				}
				for(i=i-1;i>=0;i--){
					if(ipv6_dns[i]){
						free(ipv6_dns[i]);
						ipv6_dns[i]=NULL;
					}
				}
				
			 	return;
			 }
			 memset(ipv6_dns[i], 0, 128);
		 	inet_ntop(AF_INET6, global_server_option.dnsip[i].iabuf, ipv6_dns[i] , INET6_ADDRSTRLEN);
	          	totalLen += sprintf(cursor," %s ", ipv6_dns[i]);
			cursor = showStr + totalLen;
		}
		totalLen += sprintf(cursor, "\n");
		cursor = showStr + totalLen;
	}
	
	if (global_server_option.routersip.len) {			
		totalLen += sprintf(cursor, "ipv6 dhcp server routers %u.%u.%u.%u\n", 
								global_server_option.routersip.iabuf[0], global_server_option.routersip.iabuf[1], global_server_option.routersip.iabuf[2], global_server_option.routersip.iabuf[3]);
		cursor = showStr + totalLen;
	}
	
	if (global_server_option.winsip.len) {			
		totalLen += sprintf(cursor, "ipv6 dhcp server wins %u.%u.%u.%u\n", 
								global_server_option.winsip.iabuf[0], global_server_option.winsip.iabuf[1], global_server_option.winsip.iabuf[2], global_server_option.winsip.iabuf[3]);
		cursor = showStr + totalLen;
	}
	
	if (global_server_option.defaulttime) {
		log_debug("defaulttime is ------------->%d\n", global_server_option.defaulttime);
		totalLen += sprintf(cursor, "ipv6 dhcp server lease-time %u\n", (global_server_option.defaulttime));
		cursor = showStr + totalLen;
	}
	/*
	if (global_server_option.maxtime) {			
		totalLen += sprintf(cursor, "ip dhcp server lease-max %u\n", (global_server_option.maxtime)/(60*60*24));
		cursor = showStr + totalLen;
	}*/
	totalLen += sprintf(cursor, "\n");
	cursor = showStr + totalLen;
	
	/*ip pool conifgure*/
	pool_node = head_pool.next;	

	while (pool_node) {
		log_debug("poolname is ------------->%s\n", pool_node->poolname);
		totalLen += sprintf(cursor, "ipv6 pool %s\n", pool_node->poolname);
		cursor = showStr + totalLen;
		
		sub_node = pool_node->headsubnet;
		while (sub_node) {			
			prefix_len = 0;
			memset(subnet_ip, 0, ipv6_buf_len);
			memset(range_low_ip, 0 , ipv6_buf_len);
			memset(range_high_ip, 0, ipv6_buf_len);			
			inet_ntop(AF_INET6, sub_node->ipaddr.iabuf, subnet_ip , INET6_ADDRSTRLEN);	
		 	inet_ntop(AF_INET6, sub_node->lowip.iabuf, range_low_ip , INET6_ADDRSTRLEN);			
			inet_ntop(AF_INET6, sub_node->highip.iabuf, range_high_ip , INET6_ADDRSTRLEN);				
		    i = strlen((char*)(sub_node->mask.iabuf));
			for(k = 0; k < i; ++k){
			if(sub_node->mask.iabuf[k] == 0xff){
					prefix_len = prefix_len + 8;
				}else{
					for(j=0; j <16; j++){
						if(((sub_node->mask.iabuf[k])>>j)&(0x01)){
							prefix_len++;
						}
						
					}
				}
			}			
			totalLen += sprintf(cursor, " add rangev6 %s %s prefix-length %d\n", range_low_ip, range_high_ip, prefix_len);
			cursor = showStr + totalLen;
			/*for show interface ip pool NAME*/
			ifhead = sub_node->ifhead;
			
			if(ifhead){
				dhcp6_dbus_profile_config_interface_save(&showif,&ifcursor,ifhead,pool_node,&ifLen);
			}

			/*end for show interface ip pool NAME*/
			sub_node = sub_node->next;
		}
		
		option_node = &(pool_node->owned_option);
		if (option_node->domainsearch) {			
			totalLen += sprintf(cursor, " ipv6 dhcp server domain-search %s\n", option_node->domainsearch);
			cursor = showStr + totalLen;
		}
	#if 0
		if (option_node->option43.ipnum) {			
			totalLen += sprintf(cursor, " ip dhcp server option43 %.2x%.2x", option_node->option43.type, option_node->option43.len);
			cursor = showStr + totalLen;
			for (i = 0; i < option_node->option43.ipnum; i++) {
				totalLen += sprintf(cursor, "%.8x", option_node->option43.ip[i]);
				cursor = showStr + totalLen;
			}
			totalLen += sprintf(cursor, "\n");
			cursor = showStr + totalLen;
		}
	 #endif 

	 	if(option_node->option52[0].len){
			totalLen += sprintf(cursor, " ipv6 dhcp server option52");
			cursor = showStr + totalLen;
			for(i = 0; i < 8; i++){
				if(option_node->option52[i].len){
					inet_ntop(AF_INET6, option_node->option52[i].iabuf, option52[i] , INET6_ADDRSTRLEN);
					totalLen += sprintf(cursor," %s ", option52[i]);
					cursor = showStr + totalLen;
				}else{
					break;
				}
			}
			totalLen += sprintf(cursor, "\n");
			cursor = showStr + totalLen;
	 	}

	 	log_debug("dnsnum is ==============>%d\n",option_node->dnsnum);
		if (option_node->dnsnum) {	
			totalLen += sprintf(cursor, " ipv6 dhcp server name-servers");
			cursor = showStr + totalLen;
			for (i = 0; i < option_node->dnsnum; i++) {
				memset(dns_address, 0, ipv6_buf_len);			
				inet_ntop(AF_INET6, option_node->dnsip[i].iabuf, dns_address , INET6_ADDRSTRLEN);	
				totalLen += sprintf(cursor, " %s", dns_address);
				cursor = showStr + totalLen;
			}
			totalLen += sprintf(cursor, "\n");
			cursor = showStr + totalLen;
		}		
		
		log_debug("defaulttime is ==============>%d\n",option_node->defaulttime);
		if (option_node->defaulttime) {			
			totalLen += sprintf(cursor, " ipv6 dhcp server lease-time %u\n", (option_node->defaulttime));
			cursor = showStr + totalLen;
		}
		
		pool_node = pool_node->next;
		
		totalLen += sprintf(cursor, " exit\n\n");
		cursor = showStr + totalLen;

	}

#if 0
	host_node = head_host.next;
	while (host_node) {
		totalLen += sprintf(cursor, "ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x\n", 
								host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
								host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5]);
		cursor = showStr + totalLen;
		host_node = host_node->next;
	}
#endif 

	totalLen += sprintf(cursor, showif);
	cursor = showStr + totalLen;	

	log_debug("%s:%s\n", __func__, showStr);
	
	free(showif);
	showif = NULL;
	for (i = 0; i < global_server_option.dnsnum; i++) {
		if( ipv6_dns[i] != NULL){
			free(ipv6_dns[i]);
		}
	}
	if(subnet_ip){
		free(subnet_ip);
	}
	if(range_low_ip){
		free(range_low_ip);
	}
	if(range_high_ip){
		free(range_high_ip);
	}
	if(dns_address){
		free(dns_address);
	}
	return ;
}

/*****************************************************
 * dhcp_dbus_save_lease
 *		save dhcp whole times
 			 dhcp segment times
 			 dhcp requested times 
 			 dhcp response times
 			 dhcp leases in the file /var/run/apache2/dhcp_*
 * INPUT:
 *		uint32 - profilee
 *		uint32 - detect
 *		
 * OUTPUT:
 *		uint32 - return code
 *				DHCP_SERVER_RETURN_CODE_SUCCESS  -set success
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 *****************************************************/
DBusMessage *
dhcp6_dbus_save_lease
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int profile = 0;
	unsigned int	ret = 1;
	unsigned int detect = 0;
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
		            DBUS_TYPE_UINT32,&profile,
					DBUS_TYPE_UINT32,&detect,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	//save_dhcp_info_file();
	test_for_add_subnet_v6();
	/*test_for_add_dhcp_file();*/
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}

DBusMessage* 
dhcp6_dbus_show_lease
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter  iter,iter_array;	
	DBusError err;
	unsigned int i = 0, j = 0, count = 0;
	unsigned int value = 0;
	unsigned int ret = DHCP_SERVER_RETURN_CODE_SUCCESS;	
	struct dhcp6_lease_info  *lease_info = NULL;
	char *ipv6_adr = NULL;

	ipv6_adr =malloc(128) ;
	if(NULL == ipv6_adr){
		return NULL;
	}
	
	dbus_error_init(&err);	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(ipv6_adr){
			free(ipv6_adr);
			ipv6_adr=NULL;
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	/*test_for_del_subnet();*/
	
	lease_info = get_dhcp6_lease_info((int *)&count);
	dbus_message_iter_init_append (reply, &iter);
	/* Total count*/
	if(NULL == lease_info) {
		ret = DHCP_SERVER_RETURN_CODE_FAIL;
	}
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, 
									&ret);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, 
									&count);
	
	if(DHCP_SERVER_RETURN_CODE_SUCCESS == ret) {
		if(count > 0) {
			dbus_message_iter_open_container (&iter,
												DBUS_TYPE_ARRAY,
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
																								
												DBUS_TYPE_BYTE_AS_STRING       /* mac*/
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING	/*ipv6 addr num*/

												DBUS_TYPE_ARRAY_AS_STRING
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
												DBUS_TYPE_STRING_AS_STRING    /*ipv6addr*/															
											//	DBUS_TYPE_UINT32_AS_STRING	  /*prefix_length*/
												DBUS_STRUCT_END_CHAR_AS_STRING 
												
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&iter_array);
						
			for (i = 0; i < count; i++ ) {
				DBusMessageIter iter_struct;
				DBusMessageIter iter_ipv6_addr_array;
				
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);


				for (j = 0; j < DHCP_MAC_ADDRESS_LEN; j++ ) {			
					dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_BYTE, &(lease_info[i].client_mac[j]));  /*mac*/
				}
				
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &(lease_info[i].ipv6_num));


				dbus_message_iter_open_container (&iter_struct,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING 																			
													DBUS_TYPE_STRING_AS_STRING	  /*ipv6 addr*/
												//	DBUS_TYPE_UINT32_AS_STRING	  /*prefix_length*/
													DBUS_STRUCT_END_CHAR_AS_STRING, 
													&iter_ipv6_addr_array);
				
				for(j = 0; j < lease_info[i].ipv6_num; j++) {
					DBusMessageIter iter_ipv6_struct;				
					
					dbus_message_iter_open_container (&iter_ipv6_addr_array,
												      DBUS_TYPE_STRUCT,
												      NULL,
												   	&iter_ipv6_struct);					
					
					dbus_message_iter_append_basic (&iter_ipv6_struct, DBUS_TYPE_STRING, &(lease_info[i].ipv6_addr[j]));			
						

					log_debug(" %d ipv6 addr -----> %s\n",j,  lease_info[i].ipv6_addr[j]);	
					
				 	dbus_message_iter_close_container (&iter_ipv6_addr_array, &iter_ipv6_struct);					
				}

				for(j = 0;  j < lease_info[i].ipv6_num; j++){
					if(lease_info[i].ipv6_addr[j]){
						free(lease_info[i].ipv6_addr[j]);
					}
				}

				if(lease_info[i].ipv6_addr){
					free(lease_info[i].ipv6_addr);
				}
				dbus_message_iter_close_container (&iter_struct, &iter_ipv6_addr_array);
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}	
			dbus_message_iter_close_container (&iter, &iter_array);
		}	
	}
	if(ipv6_adr){
			free(ipv6_adr);
			ipv6_adr=NULL;
		}
	
	return reply;
}

DBusMessage* 
dhcp6_dbus_show_static_host
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter  iter,iter_array;	
	DBusError err;
	unsigned int i = 0, j = 0, count = 0;
	unsigned int ipaddr = 0,value = 0;
	unsigned int ret = DHCP_SERVER_RETURN_CODE_SUCCESS;	
	struct dcli_host* statichost = NULL;
	
	dbus_error_init(&err);	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	/* Total count*/
	count = count_host;
	statichost = head_host.next;
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, 
									&ret);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, 
									&count);
	
	if (!ret) {
		if (count > 0) {
			dbus_message_iter_open_container (&iter,
												DBUS_TYPE_ARRAY,
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING   /* ip addr*/														
												DBUS_TYPE_BYTE_AS_STRING       /* mac*/
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&iter_array);
						
			for (i = 0; i < count; i++ ) {
				DBusMessageIter iter_struct;
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
				
				dhcp_dbus_four_char_to_int(statichost->hostip.iabuf, &ipaddr);
				
				dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(ipaddr));  /* ip addr*/
				
				for (j = 0; j < DHCP_MAC_ADDRESS_LEN; j++ ) {			
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &(statichost->hostmac[j]));  /*mac*/
				}
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				statichost = statichost->next;
			}	
			dbus_message_iter_close_container (&iter, &iter_array);
		}	
	}
	return reply;
}
/**********************************************************************************
 *  get_mask
 *
 *	DESCRIPTION:
 * 		get mask lenth
 *
 *	INPUT:
 *		i -> unsinged char format 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		total -> mask lenth
 *		
 **********************************************************************************/
static int get_mask(unsigned char i)
{
    int total = 0;
    while(i > 0){
        if(i & 0x1){
            total += 1;
        }
        i = i >> 1;
    }
	
    return total;
}

/**********************************************************************************
 *  compare_subnet
 *
 *	DESCRIPTION:
 * 		compare subnet address
 *
 *	INPUT:
 *		subnet1, mask1; subnet2, mask2 - struct subnet
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1  ->  Not equal
 *		0  ->  Equal
 *		
 **********************************************************************************/
static int compare_subnet(struct iaddr subnet1, struct iaddr net_mask1, struct iaddr subnet2, struct iaddr net_mask2)
{
	int i = 0;
	int masklen1 = 0 , masklen2 = 0;

	for(i = 0; i < net_mask1.len; i++){
		masklen1 += get_mask(net_mask1.iabuf[i]);
	}

	for(i = 0; i < net_mask2.len; i++){
		masklen2 += get_mask(net_mask2.iabuf[i]);
	}
/*
	if(masklen1 != masklen2){
		log_info("compare_subnet 444\n");
		return 1;
	}
*/	
	for (i = 0; i < subnet1.len; i++) {
		//log_debug("check_sub->ipaddr.iabuf[i] %d, head_sub->ipaddr.iabuf[i] %d\n", subnet1.iabuf[i],  subnet2.iabuf[i]);

		if (((subnet1.iabuf[i]) & ((net_mask1.iabuf[i])&(net_mask2.iabuf[i]))) != 
			((subnet2.iabuf[i]) & ((net_mask2.iabuf[i])&( net_mask1.iabuf[i])))) {	
			/*when break i not ++*/
			break;
		}
		
	}

	if (subnet1.len == i) {
		return 0;
	}

	return 1;
}

/**********************************************************************************
 *  dhcp6_dbus_find_poolnode_by_subnet
 *
 *	DESCRIPTION:
 * 		get pool node from subnet
 *
 *	INPUT:
 *		subnet - struct subnet
 *	
 *	OUTPUT:
 *		pool node
 *
 * 	RETURN:
 *		NULL -> not find poolnode by subnet
 *		!NULL -> find
 *		
 **********************************************************************************/
struct dcli_pool *dhcp6_dbus_find_poolnode_by_subnet(struct subnet *subnet)
{
	struct dcli_pool* poolnode = NULL;
	struct dcli_pool* tmp = NULL;
	struct dcli_subnet* dcli_subnet = NULL;

	if(!subnet) {
		return NULL;
	}

	for(tmp = head_pool.next; tmp; tmp = tmp->next) {
		for(dcli_subnet = tmp->headsubnet; dcli_subnet; dcli_subnet = dcli_subnet->next) {
			if(!compare_subnet(subnet->net, subnet->netmask, dcli_subnet->lowip, dcli_subnet->mask)) {
				poolnode = tmp;
				return poolnode;
		   	}			
		}
	}  

	return NULL;	
}
DBusMessage* 
dhcp6_dbus_show_statistics_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)

{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError err;   		
	struct dhcpv6_statistics_info info;

	memset(&info, 0, sizeof(info));
	dhcp6_get_statistics_info(&info);

	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.dhcpv6_solicit_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.dhcpv6_advertise_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.dhcpv6_request_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.dhcpv6_renew_times));	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.dhcpv6_reply_times));
	//dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.ack_times));

	return reply;
}

DBusMessage* 
dhcp6_dbus_show_lease_state
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*	reply = NULL;	  
	DBusMessageIter 	iter_array;
	DBusMessageIter 	iter;
	DBusError			err;

	int ret = 0;
	int i  = 0;
	struct dbus_lease_state  lease_state;
	struct dbus_sub_lease_state *sub_lease_state = NULL;
	unsigned int sub_count = 0;
	char *sub_name = NULL;
	char *sub_mask = NULL;
	char *poolname = NULL;
	
	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);

	
	memset(&lease_state, 0, sizeof(struct dbus_lease_state));	
	
	ret = get_dhcp_lease_ipv6_state_num(&lease_state, &sub_lease_state, &sub_count);	

	dbus_message_iter_init_append (reply, &iter);	
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.total_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.active_lease_num));
	//dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.free_lease_num));
	//dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.backup_lease_num));	
	
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&sub_count);
	
	log_debug("total %d , free %d active %d\n", lease_state.total_lease_num, lease_state.free_lease_num, lease_state.active_lease_num);


	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING     /*begin	*/

								DBUS_TYPE_STRING_AS_STRING   //subnet
								DBUS_TYPE_STRING_AS_STRING   //netmask
								DBUS_TYPE_STRING_AS_STRING   //poolname
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_count
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_active
								//DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_free
								//DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_backup
								
							
								DBUS_STRUCT_END_CHAR_AS_STRING,     /*end*/
								&iter_array);							


	for(i = 0; i < sub_count; ++i){
		DBusMessageIter iter_struct;		
		
		dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);		
		sub_name = sub_lease_state[i].subnet;
		sub_mask = sub_lease_state[i].mask;
		poolname = sub_lease_state[i].poolname;
	
		
		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_STRING,&(sub_name));		
		
		dbus_message_iter_append_basic(&iter_struct,  											
										DBUS_TYPE_STRING, &(sub_mask));
		dbus_message_iter_append_basic(&iter_struct,  											
										DBUS_TYPE_STRING, &(poolname));
		
		dbus_message_iter_append_basic(&iter_struct, 											  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.total_lease_num));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.active_lease_num));
		
		//dbus_message_iter_append_basic(&iter_struct,  
		//								DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.free_lease_num));

		//dbus_message_iter_append_basic(&iter_struct,  
		//								DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.backup_lease_num));		
		
		
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	if(sub_lease_state){
		free(sub_lease_state);
		sub_lease_state = NULL;
	}
	
	return reply;
}

DBusMessage * 
dhcp6_dbus_show_lease_by_ip
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* 	reply = NULL;
	DBusMessageIter	iter;
	DBusMessageIter	 iter_array;
	DBusError 		err;
	unsigned int    ipAddr = 0, ip_Nums = 0, i = 0, j = 0;
	unsigned int	ret = DHCP_SERVER_RETURN_CODE_SUCCESS, count = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[DHCP_MAC_ADDRESS_LEN] = {0};
	struct dhcp_lease_info *lease_info = NULL;
	struct dhcp_lease_ip_info *lease_ip = NULL;
	struct iaddr ipaddrl;

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE, &(ipaddrl.iabuf[0]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[1]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[2]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[3]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[4]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[5]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[6]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[7]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[8]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[9]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[10]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[11]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[12]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[13]),
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[14]),							
	    							 DBUS_TYPE_BYTE, &(ipaddrl.iabuf[15]),
								DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	lease_info = get_lease_ip_info_by_ip(ipAddr, ip_Nums);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	
	/* Total count*/
	if(lease_info && 
		lease_info->segment_times_info) {
		count = lease_info->segment_times_info;
	}
	else {
		ret = DHCP_SERVER_RETURN_CODE_FAIL;
	}
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, 
									&ret);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, 
									&count);
	
	if(DHCP_SERVER_RETURN_CODE_SUCCESS == ret) {
		lease_ip = lease_info->lease_ip->next;
		if(count > 0) {
			dbus_message_iter_open_container (&iter,
												DBUS_TYPE_ARRAY,
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING   /* ip addr*/														
												DBUS_TYPE_BYTE_AS_STRING       /* mac*/
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_TYPE_BYTE_AS_STRING
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&iter_array);
						
			for (i = 0; i < count; i++ ) {
				DBusMessageIter iter_struct;
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
				/*if ipv6 maybe 16*/
				memcpy(&ipaddr, lease_ip->ip_addr.iabuf, DHCP_IPV6_ADDRESS_LEN);
				memcpy(mac, lease_ip->hw_addr, DHCP_MAC_ADDRESS_LEN);
				lease_ip = lease_ip->next;
				
				dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(ipaddr));  /* ip addr*/
				
				for (j = 0; j < DHCP_MAC_ADDRESS_LEN; j++ ) {			
					dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_BYTE,
							  &(mac[j]));  /*mac*/
				}
				
				dbus_message_iter_close_container (&iter_array, &iter_struct);
			}	
			dbus_message_iter_close_container (&iter, &iter_array);
		}	
	}

	while(lease_info->lease_ip->next){
		struct dhcp_lease_ip_info *lease_ip_info_to_tree = lease_info->lease_ip->next;
		lease_info->lease_ip->next = lease_ip_info_to_tree->next;
		free(lease_ip_info_to_tree);
		lease_ip_info_to_tree=NULL;
	}
	if(lease_info){
		free(lease_info);
		lease_info=NULL;
	}
	return reply;
}	

DBusMessage *
dhcp6_dbus_show_lease_by_mac
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* 	reply = NULL;
	DBusMessageIter	iter, iter_array;
	DBusError 		err;
	unsigned char	    macAddr[DHCP_MAC_ADDRESS_LEN], mac[DHCP_MAC_ADDRESS_LEN], j = 0;
	unsigned  int  ipaddr= 0, macType = 0, ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
	struct dhcp_lease_ip_info lease_ip;

	memset(&macAddr, 0, DHCP_MAC_ADDRESS_LEN);	
	memset(&mac, 0, DHCP_MAC_ADDRESS_LEN);
		
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&macType,
								DBUS_TYPE_BYTE,  &macAddr[0],
								DBUS_TYPE_BYTE,  &macAddr[1],
								DBUS_TYPE_BYTE,  &macAddr[2],
								DBUS_TYPE_BYTE,  &macAddr[3],
								DBUS_TYPE_BYTE,  &macAddr[4],
								DBUS_TYPE_BYTE,  &macAddr[5],					     
								DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}return NULL;
	}
	
	ret = get_lease_ip_info_by_hw(macAddr, &lease_ip);
	
/*do something upon*/	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32, &ret);

	if(ret) {
		dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING   /* ip addr*/														
											DBUS_TYPE_BYTE_AS_STRING       /* mac*/
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_TYPE_BYTE_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);
					

		DBusMessageIter iter_struct;
		dbus_message_iter_open_container (&iter_array,
										   DBUS_TYPE_STRUCT,
										   NULL,
										   &iter_struct);
		
		/*if ipv6 maybe 16*/
		memcpy(&ipaddr, lease_ip.ip_addr.iabuf, DHCP_IPV6_ADDRESS_LEN);
		memcpy(mac, lease_ip.hw_addr, DHCP_MAC_ADDRESS_LEN);
		
		dbus_message_iter_append_basic
				(&iter_struct,
				  DBUS_TYPE_UINT32,
				  &(ipaddr));  /* ip addr*/
		
		for (j = 0; j < DHCP_MAC_ADDRESS_LEN; j++ ) {			
			dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_BYTE,
					  &(mac[j]));  /*mac*/
		}		
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	return reply;
}

DBusMessage * 
dhcp6_dbus_show_dhcp_global_conf
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int value = 0, enable = 0, len = 0, dnsnum;
	unsigned int	default_time = 0;
	char* domainsearch = NULL;
	DBusError err;		
	
	char *name_server = malloc(sizeof(struct iaddr) * 3);
	if(NULL == name_server){
		return NULL;
	}
	memset(name_server, 0 , sizeof(struct iaddr) * 3);

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32, &value,
			DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	enable = dhcp_server_enable;
	if (global_server_option.domainsearch) {
		len = strlen(global_server_option.domainsearch);
		domainsearch = malloc(len + 1);
		if(NULL == domainsearch){
			if(name_server){
				free(name_server);
				name_server=NULL;
			}
			return NULL;
		}
		memset(domainsearch, 0, (len + 1));
		memcpy(domainsearch, global_server_option.domainsearch, len);
	}
	else {	
		domainsearch = malloc(1);
		if(NULL == domainsearch){
			if(name_server){
				free(name_server);
				name_server=NULL;
			}

			return NULL;
		}
		memset(domainsearch, 0, 1);
	}

	dnsnum = global_server_option.dnsnum;	
	if(0 == dnsnum){
		global_server_option.dnsip = (struct iaddr *)name_server;
	}
	default_time = (global_server_option.defaulttime);	

	

	reply = dbus_message_new_method_return(msg);

	log_debug("dnsnum is %d\n", dnsnum);
	log_debug("default time is %d\n", default_time);
	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &enable);	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING, &domainsearch);	
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &(dnsnum));	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[0]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[1]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[2]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[3]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[4]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[5]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[6]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[7]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[8]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[9]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[10]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[11]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[12]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[13]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[14]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[0].iabuf[15]));	 

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[0]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[1]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[2]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[3]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[4]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[5]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[6]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[7]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[8]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[9]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[10]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[11]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[12]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[13]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[14]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[1].iabuf[15]));	 
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[0]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[1]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[2]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[3]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[4]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[5]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[6]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[7]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[8]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[9]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[10]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[11]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[12]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[13]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[14]));
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_BYTE, &(global_server_option.dnsip[2].iabuf[15]));	 
									 
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &default_time);	

	log_debug("end dbus send %d\n", dnsnum);
	if(domainsearch != NULL){
		free(domainsearch);
	}
	if(name_server){
		free(name_server);
		name_server=NULL;
	}
	
	return reply;	
}

DBusMessage * 
dhcp6_dbus_show_dhcp_pool_conf
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*			 reply = NULL;
	DBusMessageIter 		 iter;
	DBusMessageIter 		 iter_array;
	DBusError				err;	
	unsigned int i = 0, j = 0, k = 0, loop;
	unsigned ret = 0,  len = 0, mode = 0, index = 0, pool_count = 0, default_time = 0;
	unsigned int domainnull = 0;
	struct dcli_pool* poolnode = NULL;
	struct dcli_subnet* subnetnode = NULL;
	char *domainsearch = NULL;
	char *option52 = NULL;
	char *range_low_ip = NULL;
	char *range_high_ip = NULL;
	unsigned int prefix_len = 0;
	char *dns_address[3];
	struct dcli_option* option_node = NULL;	
	int ipv6_buf_len = 128;
	char **ap_via_address = NULL;
	char *tmp_interface_name = NULL;

	tmp_interface_name = malloc(1);
	if(NULL == tmp_interface_name){
		return NULL;
	}	
	memset(tmp_interface_name,	0, 1);
	ap_via_address = (char **)malloc(sizeof(char *) * 8);
	if(NULL == ap_via_address){
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
		return NULL;
	}
	memset(ap_via_address, 0, sizeof(char *) * 8);
	for(i = 0; i < 8; ++i){
		ap_via_address[i] = malloc(512);	
		if(NULL == ap_via_address[i]){
			
			for(j = 0;j < i;j++){
				if(ap_via_address[j]){
					free(ap_via_address[j]);
					ap_via_address[j]=NULL;
				}
			}
			if(ap_via_address){
				free(ap_via_address);
				ap_via_address=NULL;
			}
			if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
			}
		return NULL;
		}
		memset(ap_via_address[i], 0 , 512);
	}
	
	domainsearch = malloc(len + 1);
	if(NULL == domainsearch){
		
		for(j = 0;j < 8;j++){
			if(ap_via_address[j]){
				free(ap_via_address[j]);
				ap_via_address[j]=NULL;
			}
		}
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		}
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
		return NULL;
	}
	memset(domainsearch, 0, (len + 1));	

	option52 = malloc(ipv6_buf_len + 1);
	
	if(NULL == option52){
		if(domainsearch){
			free(domainsearch);
			domainsearch=NULL;
		}
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
		for(j = 0;j < 8;j++){
			if(ap_via_address[j]){
				free(ap_via_address[j]);
				ap_via_address[j]=NULL;
			}
		}
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		}
		return NULL;
	}
	memset(option52, 0, (ipv6_buf_len + 1));

	range_low_ip =malloc(ipv6_buf_len) ;
	if(NULL == range_low_ip){
		if(option52){
			free(option52);
			option52=NULL;
		}
		if(domainsearch){
			free(domainsearch);
			domainsearch=NULL;
		}
		for(j = 0;j < 8;j++){
			if(ap_via_address[j]){
				free(ap_via_address[j]);
				ap_via_address[j]=NULL;
			}
		}
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		} 
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
		return NULL;
	}
	range_high_ip =malloc(ipv6_buf_len) ;
	if(NULL == range_high_ip){
		if(range_low_ip){
			free(range_low_ip);
			range_low_ip=NULL;
		}
		if(option52){
			free(option52);
			option52=NULL;
		}
		if(domainsearch){
			free(domainsearch);
			domainsearch=NULL;
		}
		for(j = 0;j < 8;j++){
			if(ap_via_address[j]){
				free(ap_via_address[j]);
				ap_via_address[j]=NULL;
			}
		}
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		}
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
		return NULL;
	}
	for(i = 0; i < 3 ; ++i){
		dns_address[i] =malloc(ipv6_buf_len) ;
		if(NULL == dns_address[i]){
			for(j=0;j<i;j++){
				if(dns_address[j]){
					free(dns_address[j]);
					dns_address[j]=NULL;
				}
				
			}
			if(range_high_ip){
				free(range_high_ip);
				range_high_ip=NULL;
			}
			if(range_low_ip){
				free(range_low_ip);
				range_low_ip=NULL;
			}
			if(option52){
				free(option52);
				option52=NULL;
			}
			if(domainsearch){
				free(domainsearch);
				domainsearch=NULL;
			}
			for(j = 0;j < 8;j++){
				if(ap_via_address[j]){
					free(ap_via_address[j]);
					ap_via_address[j]=NULL;
				}
			}
			if(ap_via_address){
				free(ap_via_address);
				ap_via_address=NULL;
			}
			if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
			}
			return NULL;
		}
		memset(dns_address[i], 0, ipv6_buf_len);
	}	
	
	dbus_error_init(&err);
	/*0 means glbal mode*/
	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32, &mode,
			DBUS_TYPE_UINT32, &index,
			DBUS_TYPE_INVALID))) {
	   log_error("Unable to get input args ");
	   if (dbus_error_is_set(&err)) {
		   log_error("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   for(j=0;j<3;j++){
			if(dns_address[j]){
				free(dns_address[j]);
				dns_address[j]=NULL;
			}
				
		}
		if(range_high_ip){
			free(range_high_ip);
			range_high_ip=NULL;
		}
		if(range_low_ip){
			free(range_low_ip);
			range_low_ip=NULL;
		}
		if(option52){
			free(option52);
			option52=NULL;
		}
		if(domainsearch){
			free(domainsearch);
			domainsearch=NULL;
		}
		for(j = 0;j < 8;j++){
			if(ap_via_address[j]){
				free(ap_via_address[j]);
				ap_via_address[j]=NULL;
			}
		}
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		}
		if(tmp_interface_name){
			free(tmp_interface_name);
			tmp_interface_name=NULL;
		}
	   return NULL;
	}

	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {			
			pool_count = 1;
		}
	}
	else {
		poolnode = head_pool.next;
		pool_count = count_pool;
	}
	
	reply = dbus_message_new_method_return(msg);
	log_debug("return basic param pool count is %d\n", pool_count);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &pool_count);
	log_debug("return basic param pool count is %d\n", pool_count);
   
	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING     /*begin	*/
								
								DBUS_TYPE_STRING_AS_STRING    /*poolname*/
								DBUS_TYPE_STRING_AS_STRING    /*domainname*/
								DBUS_TYPE_STRING_AS_STRING    /*option52[0]*/
								DBUS_TYPE_STRING_AS_STRING	  /*option52[1]*/
								DBUS_TYPE_STRING_AS_STRING   /*option52[2]*/
								DBUS_TYPE_STRING_AS_STRING   /*option52[3]*/
								DBUS_TYPE_STRING_AS_STRING   /*option52[4]*/
								DBUS_TYPE_STRING_AS_STRING   /*option52[5]*/
								DBUS_TYPE_STRING_AS_STRING   /*option52[6]*/
								DBUS_TYPE_STRING_AS_STRING	 /*option52[7]*/							
								DBUS_TYPE_UINT32_AS_STRING   /*option52 total num*/	
								DBUS_TYPE_STRING_AS_STRING	  /*dns[0]*/	
								DBUS_TYPE_STRING_AS_STRING	  /*dns[1]*/
								DBUS_TYPE_STRING_AS_STRING   /*dns[2]*/
								DBUS_TYPE_UINT32_AS_STRING   /*dnsnum*/								
								DBUS_TYPE_UINT32_AS_STRING    /*defaulttime*/								
								DBUS_TYPE_UINT32_AS_STRING    /*sub_count*/
								DBUS_TYPE_STRING_AS_STRING    /*interfacename bind pool*/

								DBUS_TYPE_ARRAY_AS_STRING
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
								DBUS_TYPE_STRING_AS_STRING    /*iplow*/								
								DBUS_TYPE_STRING_AS_STRING	  /*iphigh*/						
								DBUS_TYPE_UINT32_AS_STRING	  /*prefix_length*/
								DBUS_STRUCT_END_CHAR_AS_STRING 
								
								DBUS_STRUCT_END_CHAR_AS_STRING,     /*end*/
								    &iter_array);	
	if (pool_count) {
		 for (loop = 0; loop < pool_count; loop++ ) {
			
			if (!(poolnode->owned_option.domainsearch)) {
				domainnull = 1;				
			}			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);

			dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_STRING, &(poolnode->poolname));
			log_debug("poolname---> %s\n", poolnode->poolname);
			if (domainnull) {
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(domainsearch));
				domainnull = 0;				
			}
			else {		
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(poolnode->owned_option.domainsearch));
				
			}
			
			log_debug("domainnull----> %s\n", poolnode->owned_option.domainsearch);


			for(i = 0; i < 8 ; i++){
				if(poolnode->owned_option.option52[i].len){
					inet_ntop(AF_INET6, poolnode->owned_option.option52[i].iabuf, ap_via_address[i] , INET6_ADDRSTRLEN);
					log_debug("option ipv6 address is %s\n", ap_via_address[i]);
					dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(ap_via_address[i]));
				}else{
					break;
				}
			}
			for(j = 0; j < (8- i); j++){
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(ap_via_address[j]));
			}				

			/*option ipv6 address  number*/
			dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_UINT32,
						 &(i));
			
			option_node = &(poolnode->owned_option);
			
			for(j = 0; j < option_node->dnsnum; ++j){				
					memset(dns_address[j], 0, ipv6_buf_len);					
					inet_ntop(AF_INET6, option_node->dnsip[j].iabuf, dns_address[j] , INET6_ADDRSTRLEN);	
					dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_STRING, &(dns_address[j]));	
					log_debug("dns_address[%d]----> %s\n",j,  dns_address[j]);
			}
			for(j = option_node->dnsnum; j < 3; ++j ){
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_STRING, &(dns_address[j]));
				log_debug("dns_address[%d]----> %s\n",j,  dns_address[j]);
			}

			log_debug("dnsnum-----> %d\n", option_node->dnsnum);		
			dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &(poolnode->owned_option.dnsnum));

			log_debug("defaulttime----> %d\n", poolnode->owned_option.defaulttime);
			default_time = (poolnode->owned_option.defaulttime);
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(default_time));				
			
			dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &(poolnode->count_sub));

			if(poolnode->headsubnet &&
				poolnode->headsubnet->ifhead &&
				poolnode->headsubnet->ifhead->interface_name) {				
				
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(poolnode->headsubnet->ifhead->interface_name));
				
			}else {	
				
				dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_STRING,
					 &(tmp_interface_name));

				
			}

			dbus_message_iter_open_container (&iter_struct,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
													DBUS_TYPE_STRING_AS_STRING    /*iplow*/								
													DBUS_TYPE_STRING_AS_STRING	  /*iphigh*/
													DBUS_TYPE_UINT32_AS_STRING	  /*prefix_length*/
													DBUS_STRUCT_END_CHAR_AS_STRING, 
													&iter_sub_array);
			subnetnode = poolnode->headsubnet;
			for(j = 0; j < poolnode->count_sub; j++) {
				DBusMessageIter iter_sub_struct;				
				
				dbus_message_iter_open_container (&iter_sub_array,
											      DBUS_TYPE_STRUCT,
											      NULL,
											   	&iter_sub_struct);

				
				memset(range_low_ip, 0, ipv6_buf_len);							
				inet_ntop(AF_INET6, subnetnode->lowip.iabuf, range_low_ip , INET6_ADDRSTRLEN);	
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING, &(range_low_ip));
				
				memset(range_high_ip, 0, ipv6_buf_len);			
				inet_ntop(AF_INET6, subnetnode->highip.iabuf, range_high_ip , INET6_ADDRSTRLEN);	
				dbus_message_iter_append_basic (&iter_sub_struct,DBUS_TYPE_STRING, &(range_high_ip));	
					

				prefix_len = 0;
			    i = strlen((char*)(subnetnode->mask.iabuf));
				for(k = 0; k < i; ++k){
				if(subnetnode->mask.iabuf[k] == 0xff){
						prefix_len = prefix_len + 8;
					}else{
						for(j=0; j <16; j++){
							if(((subnetnode->mask.iabuf[k])>>j)&(0x01)){
								prefix_len++;
							}
							
						}
					}
				}

				log_debug(" %d prefix_len-----> %d\n",j,  prefix_len);
				dbus_message_iter_append_basic (&iter_sub_struct, DBUS_TYPE_UINT32,&(prefix_len));
				
			 	dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
				subnetnode = subnetnode->next;
			}
				
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				poolnode = poolnode->next;
		 } /*for*/
	} /*if*/
	dbus_message_iter_close_container (&iter, &iter_array);
	

	if(domainsearch){
		free(domainsearch);
	}
	if(option52){
		free(option52);
	}
	if(range_low_ip){
		free(range_low_ip);
	}
	if(range_high_ip){
		free(range_high_ip);
	}
	for(i = 0; i < 3; ++i){
		if(dns_address[i] != NULL){
			free(dns_address[i]);
		}
	}

	for(i = 0; i < 8; ++i){
		if(ap_via_address[i] != NULL){
			free(ap_via_address[i]);
		}		
	}
	if(ap_via_address != NULL){
		free(ap_via_address);
	}
	if(tmp_interface_name){
		free(tmp_interface_name);
		tmp_interface_name=NULL;
	}
	return reply;

}

DBusMessage * 
dhcp6_dbus_entry_pool_node
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	struct dcli_pool* poolNode = NULL;
	char* poolName = NULL;
	unsigned int index = 0, op_ret = 0, ret = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &poolName, 
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = dhcp6_dbus_find_pool_by_name(poolName, &poolNode);
	if(!ret) {
		index = poolNode->index;
		op_ret = 0;
	}
	else {
		op_ret = 1;      
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &index);
	
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_interface_pool
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	struct dcli_pool* poolNode = NULL;
	struct dcli_subnet* subNode = NULL;
	char* poolName = NULL, *ifname = NULL;
	unsigned int add = 0, op_ret = 0, ret = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &poolName, 
		DBUS_TYPE_STRING, &ifname, 
		DBUS_TYPE_UINT32, &add,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = dhcp6_dbus_find_pool_by_name(poolName, &poolNode);
	if(!ret) {
		ret = dhcp6_dbus_find_subnet_by_ifname(ifname, &subNode);
		if (!ret) {
			if (add) {
				op_ret = 2;
				log_error("pool has already binded interface\n");
			}
			else if(poolNode->headsubnet 
			&& poolNode->headsubnet->ifhead 
			&& (!strcmp(ifname, poolNode->headsubnet->ifhead->interface_name))){
				dhcp6_dbus_interface_unbind_pool(subNode, ifname);
				poolNode->refcount--;
			}
			else {	
				log_error("interface bounded by pool %s and interface %s don't match\n", poolName, ifname);
				op_ret = 2;
			}
		}
		else {
			/*bind interface to pool*/
			if (add) {
				op_ret = dhcp6_dbus_interface_bind_pool(poolNode, ifname);
			}
			else {
				op_ret = 2;
				log_error("can not find the address range by the intrface\n");

			}
		}
	}
	else {				
		op_ret = 1;
		log_error("bad pool name\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	
	
	return reply;
}


DBusMessage * 
dhcp6_dbus_create_pool_node
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char*	poolName = NULL, *name = NULL;
	struct dcli_pool* poolNode = NULL;
	unsigned int	op_ret = 0, ret = 0, index = 0, del = 0;
	DBusError err;
	
	dbus_error_init(&err);
	/*del = 1 means delete the ip pool*/
	if (!(dbus_message_get_args ( msg, &err,		
			DBUS_TYPE_UINT32, &del,
			DBUS_TYPE_STRING, &poolName,
			DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	/*bool */
	name = malloc(strlen(poolName) + 1);
	if(NULL == name){
		log_error("name malloc error!\n");
		return NULL;
	}
	memset(name, 0, (strlen(poolName) + 1));
	memcpy(name, poolName, strlen(poolName));
	log_debug("dhcp_dbus_create_pool_by_name name is %s \n", poolName);
	if (del) {
		op_ret = dhcp6_dbus_delete_pool_by_name(name);				
	}
	else {
		ret = dhcp6_dbus_create_pool_by_name(name, &poolNode);
		if(!ret) {
			op_ret = 0;
			index = poolNode->index;
		}
		else {
	log_debug("dhcp_dbus_create_pool_by_name ret is %d \n", ret);
			op_ret = ret;      
		}
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &index);

	
	
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_domain_search
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *domainsearch = NULL, *tmpname = NULL;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &domainsearch,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
log_debug("add domain search 1111111\n");	
	tmpname = malloc(strlen(domainsearch) + 1);
	memset(tmpname, 0, (strlen(domainsearch)+ 1));
	memcpy(tmpname, domainsearch, strlen(domainsearch));
	owned_option.domainsearch= tmpname;	
log_debug("add domain search 22222222\n");	

	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			log_debug("poolname is ---------> %s\n", poolnode->poolname);	
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_veo
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *veo = NULL, *tmpname = NULL;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &veo,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	tmpname = malloc(strlen(veo) + 1);
	memset(tmpname, 0, (strlen(veo)+ 1));
	memcpy(tmpname, veo, strlen(veo));

	if (!del) {
		ret = string_to_option43_s(tmpname, &(owned_option.option43));
		if (!ret) {
			log_debug("set server veo type is %x, len is %x, ipnum is %x, ip is %x\n",
					owned_option.option43.type, owned_option.option43.len, owned_option.option43.ipnum, owned_option.option43.ip[0]);
		}
	}
	else {
		owned_option.option43.ipnum = 1;
	}
	/*sub mode*/
	if (mode) {	
		if (!ret) {	
			ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
			if (!ret) {
				addsubnet = poolnode->headsubnet;
				while (addsubnet) {
					dhcp6_dbus_set_option(addsubnet, &owned_option, del);
					addsubnet = addsubnet->next;
				}
			}
			else {
				op_ret = 2;
			}
		}
		else {
			op_ret = 1;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	if(tmpname){
		free(tmpname);
		tmpname=NULL;
	}
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_option52
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *ipv6adr[8] ;
	unsigned int ipv6num;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;	
	DBusError err;
	int i;	
	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &ipv6adr[0],
		DBUS_TYPE_STRING, &ipv6adr[1],
		DBUS_TYPE_STRING, &ipv6adr[2],
		DBUS_TYPE_STRING, &ipv6adr[3],
		DBUS_TYPE_STRING, &ipv6adr[4],
		DBUS_TYPE_STRING, &ipv6adr[5],
		DBUS_TYPE_STRING, &ipv6adr[6],
		DBUS_TYPE_STRING, &ipv6adr[7],
		DBUS_TYPE_UINT32, &ipv6num,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	log_debug("ipv6 num is %d\n", ipv6num);

	if(del){
		ipv6num = 1;
	}
	
	for(i = 0; i < ipv6num; ++i){
		inet_pton(AF_INET6, ipv6adr[i], owned_option.option52[i].iabuf);
		owned_option.option52[i].len= DHCP_IPV6_ADDRESS_LEN;		
	}

	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}	

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	
	return reply;
}
DBusMessage * 
dhcp6_dbus_set_server_name_servers
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int dnsnum = 1, del = 0;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, i = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	struct iaddr tempip, ipAddr[3];
	char *value = NULL;
	DBusError err;
	
	memset(ipAddr, 0, 3*sizeof(struct iaddr));
	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT32, &dnsnum,
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[15]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[15]),	
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[15]),
							DBUS_TYPE_UINT32, &mode,		
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_UINT32, &del,
							DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if (del){
		dnsnum = 1;
	}
	log_debug("dnsnum is ---> %d\n", dnsnum);
	value = malloc(sizeof(struct iaddr) * dnsnum);
	memset(value, 0, sizeof(struct iaddr) * dnsnum);
	for (i = 0; i < dnsnum; i++) {
		memset(&tempip, 0, (sizeof (struct iaddr)));
		memcpy(tempip.iabuf, ipAddr[i].iabuf, DHCP_IPV6_ADDRESS_LEN);		
		tempip.len = DHCP_IPV6_ADDRESS_LEN;
		memcpy(value + i*sizeof(struct iaddr), &tempip, sizeof(struct iaddr));
		log_debug("add %d name servers ipv6 addrss is %s\n", (i+1), piaddr(tempip));
	}
	
	owned_option.dnsnum = dnsnum;
	owned_option.dnsip = (struct iaddr *)value;
	
	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp6_dbus_set_server_routers_ip
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int routersip = 0, del = 0;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &routersip,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	owned_option.routersip.iabuf[0] = (routersip>>24)&0xff;		
	owned_option.routersip.iabuf[1] = (routersip>>16)&0xff;
	owned_option.routersip.iabuf[2] = (routersip>>8)&0xff;
	owned_option.routersip.iabuf[3] = routersip&0xff;
	owned_option.routersip.len = DHCP_IPV6_ADDRESS_LEN;

	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp6_dbus_set_server_wins_ip
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int winsip = 0, del = 0;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &winsip,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	owned_option.winsip.iabuf[0] = (winsip>>24)&0xff;		
	owned_option.winsip.iabuf[1] = (winsip>>16)&0xff;
	owned_option.winsip.iabuf[2] = (winsip>>8)&0xff;
	owned_option.winsip.iabuf[3] = winsip&0xff;
	owned_option.winsip.len = DHCP_IPV6_ADDRESS_LEN;

	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_lease_default
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int default_time = 1, del = 0;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &default_time,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	
	if (del) {
		default_time = DEFAULT_DEFAULT_LEASE_TIME;	
	}
	owned_option.defaulttime = default_time;	

log_debug("owned_option->defaulttime is %d ", default_time);

	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_lease_max
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int max_time = 1, del = 0;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0;
	struct dcli_subnet* addsubnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &max_time,
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,	
		DBUS_TYPE_UINT32, &del, 
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (del) {
		max_time = 1;	
	}
	owned_option.maxtime = max_time;	
log_debug("owned_option->maxtime is %d ", max_time);
	/*sub mode*/
	if (mode) {
		ret = dhcp6_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			while (addsubnet) {
				dhcp6_dbus_set_option(addsubnet, &owned_option, del);
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = 2;
		}
	}
	else {
		dhcp6_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage*
dhcp6_dbus_set_debug_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int debug_type = 0;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &debug_type,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(debug_type == DEBUG_TYPE_ALL){
		log_debug("dhcp debug_type is %s \n", "all");
	}
	else if(debug_type == DEBUG_TYPE_INFO){
		log_debug("dhcp debug_type is %s \n", "info");
	}
	else if(debug_type == DEBUG_TYPE_ERROR){
		log_debug("dhcp debug_type is %s \n", "error");
	}
	else if(debug_type == DEBUG_TYPE_DEBUG){
		log_debug("dhcp debug_type is %s \n", "debug");
	}

	if(enable){
		dhcp_log_level |= debug_type;
	}else{
		dhcp_log_level &= ~debug_type;
	}
		
	log_debug("globle dhcpv6_log_level is %d \n", dhcp_log_level);	
	
		
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}

DBusMessage * 
dhcp6_dbus_set_server_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	dhcp_server_enable = enable;
	log_info("globle dhcpv6 server is %s \n", (dhcp_server_enable) ? "enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp6_dbus_get_server_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	enable = dhcp_server_enable;
	log_debug("get dhcp server state is %s\n", (enable)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &enable);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp6_dbus_add_dhcp_pool_ip_range
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	struct dcli_pool* poolNode = NULL;
	struct dcli_subnet	*tempsub = NULL;
	unsigned int	op_ret = 0, ret = 0, i = 0, index = 0, add = 0, prefix_length , bit_g = 0, remain_g = 0;
	int ipv6_h = 0, ipv6_l = 0;
	struct iaddr ipaddrl, ipaddrh;
	DBusError err;

	memset(&ipaddrl, 0, sizeof(struct iaddr));
	memset(&ipaddrh, 0, sizeof(struct iaddr));
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &add,
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[0]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[1]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[2]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[3]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[4]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[5]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[6]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[7]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[8]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[9]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[10]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[11]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[12]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[13]),
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[14]),							
		DBUS_TYPE_BYTE, &(ipaddrl.iabuf[15]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[0]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[1]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[2]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[3]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[4]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[5]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[6]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[7]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[8]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[9]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[10]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[11]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[12]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[13]),
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[14]),							
		DBUS_TYPE_BYTE, &(ipaddrh.iabuf[15]),
		DBUS_TYPE_UINT32, &prefix_length,
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	/*bool */
	ret = dhcp6_dbus_find_pool_by_index(index, &poolNode);
	if(!ret) {
		log_debug("get name succee name is %s\n", poolNode->poolname);
		tempsub = (struct dcli_subnet*)malloc(sizeof(struct dcli_subnet));
		memset(tempsub, 0, sizeof(struct dcli_subnet));

		tempsub->highip = ipaddrh;		
		tempsub->highip.len = DHCP_IPV6_ADDRESS_LEN;
		log_debug("addsubnet->highip.iabuf %s\n", piaddr(tempsub->highip));
		
		tempsub->lowip = ipaddrl;	
		tempsub->lowip.len = DHCP_IPV6_ADDRESS_LEN;
		log_debug("addsubnet->lowip.iabuf %s\n", piaddr(tempsub->lowip));

		bit_g = prefix_length/8;
		remain_g = prefix_length%8;
		for(i = 0; i < bit_g; i ++) {
			tempsub->mask.iabuf[i] = 0xff;
		}
		for(i=0; i< remain_g; i++){
			tempsub->mask.iabuf[bit_g] += (0x01<<(7-i));
		}
		tempsub->mask.len = DHCP_IPV6_ADDRESS_LEN;
		log_debug("addsubnet->mask.iabuf %s, len is %d\n", piaddr(tempsub->mask), prefix_length);

		for(i = 0; i < bit_g; i ++) {
			tempsub->ipaddr.iabuf[i] = ipaddrl.iabuf[i];
		}
		for(i=0; i< remain_g; i++){
			tempsub->ipaddr.iabuf[bit_g] += (ipaddrl.iabuf[bit_g] & (0x01<<(7-i)));
		}
		tempsub->ipaddr.len = DHCP_IPV6_ADDRESS_LEN;
		log_debug("addsubnet->ipaddr.iabuf %s\n", piaddr(tempsub->ipaddr));	
		ipv6_h = ((ipaddrh.iabuf[12]<<24)|(ipaddrh.iabuf[13]<<16)|(ipaddrh.iabuf[14]<<8)|(ipaddrh.iabuf[15]));
		ipv6_l = ((ipaddrl.iabuf[12]<<24)|(ipaddrl.iabuf[13]<<16)|(ipaddrl.iabuf[14]<<8)|(ipaddrl.iabuf[15]));
		if (!add) {
			log_debug(" add subnet \n");
			if(ipv6_h < ipv6_l){
				free(tempsub);
				op_ret = 3;
				goto dbus_return;
			}
			ret = dhcp6_debus_is_same_subnet_exist(tempsub, poolNode->poolname);
			if (1 == ret) {
				free(tempsub);
				op_ret = 3;
				goto dbus_return;					
			}
			/* whether same subnet */
			ret = dhcp6_dbus_whether_same_subnet(tempsub, poolNode->headsubnet);
			if (0 == ret) {
				free(tempsub);
				op_ret = 3;
				goto dbus_return;					
			}
			//ret	= dhcp6_dbus_check_pool_ip(tempsub ,poolNode->poolname); 
			if (ret) {
				/*
				ret = dhcp_dbus_check_interface(&tempsub->lowip, &tempsub->mask, &ifname, &ifnum);
				log_debug("end for check interace \n");
				tempsub->interface_name = ifname;
				tempsub->interface_num = ifnum;
				
				if (!ret) {	*/				
				ret = dhcp6_dbus_set_subnet(tempsub);
				if (!ret) {
					/*add for list member*/
					tempsub->attribute_pool = poolNode;
					if (!poolNode->headsubnet) {
						poolNode->headsubnet = tempsub;						
					}
					else {
						tempsub->next = poolNode->headsubnet;
						poolNode->headsubnet = tempsub;
					}
					//tempsub->ip_num = ipaddrh - ipaddrl + 1;
					poolNode->count_sub++;
				}
				else {			
					log_error("bad ip address for dhcp range \n");					
					free(tempsub);
					op_ret = 2;
				}
			}
			else {				
				free(tempsub);
				op_ret = 3;
			}
		}
		else {
			log_debug("delete subnet \n");
			if (!poolNode->headsubnet) {
				op_ret = 1;
				log_error("delet subnet fail poolNode has no subnet \n");
			}
			else if (poolNode->refcount) {
				op_ret = 1;
				log_error("binded by interface can no del range\n");
			}
			else {
				ret = dhcp6_dbus_delete_subnet_by_subip(tempsub, poolNode);
				if (ret) {	
					op_ret = 1;
				}
				free(tempsub);
			}
		}
	}
	else {
		log_debug("not find headpool node!\n");
		op_ret = 1;      
	}
	dbus_return:

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	return reply;
	
}

DBusMessage * 
dhcp6_dbus_add_dhcp_static_host
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int	op_ret = 0, ret = 0, ipaddr = 0, add = 0;
	struct dcli_host addhost, *temphost = NULL;
	char mac[DHCP_MAC_ADDRESS_LEN], *hostname = NULL;
	DBusError err;

	hostname = malloc(DHCP_HOST_NAME_LEN);
	memset(&addhost, 0, sizeof(struct dcli_host));	
	memset(hostname, 0, DHCP_HOST_NAME_LEN);
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_BYTE, &mac[0],
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3],
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],
							DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(hostname){
		free(hostname);
		hostname=NULL;
		}
		return NULL;
	}
	
	memset(&addhost, 0, sizeof(struct dcli_host));

	addhost.hostip.iabuf[0] = (ipaddr>>24)&0xff;		
	addhost.hostip.iabuf[1] = (ipaddr>>16)&0xff;
	addhost.hostip.iabuf[2] = (ipaddr>>8)&0xff;
	addhost.hostip.iabuf[3] = ipaddr&0xff;
	addhost.hostip.len = DHCP_IPV6_ADDRESS_LEN;	
	addhost.hostmac[0] = mac[0];	
	addhost.hostmac[1] = mac[1];
	addhost.hostmac[2] = mac[2];
	addhost.hostmac[3] = mac[3];
	addhost.hostmac[4] = mac[4];
	addhost.hostmac[5] = mac[5];

	/*add static host*/
	if (add) {
		ret = dhcp6_dbus_find_static_host_by_addr(&(addhost.hostip), mac, &temphost);
		if (ret) {
			sprintf(hostname, "host%d", ++host_index);	
			addhost.hostname = hostname;
			dhcp6_dbus_set_host(&addhost, 1);			
			dhcp6_dbus_create_static_host_by_name(hostname, &temphost);
			memcpy(&(temphost->hostip), &(addhost.hostip), sizeof(struct iaddr));
			memcpy(temphost->hostmac, mac, DHCP_MAC_ADDRESS_LEN);
			log_debug("finish for add static host name %s\n", temphost->hostname);
		}
		else {
			op_ret = 1;
		}
	}
	/*delete static host*/
	else {
		ret = dhcp6_dbus_find_static_host_by_addr_all(&(addhost.hostip), mac, &temphost);
		if (!ret) {
			dhcp6_dbus_set_host(temphost, 0);
			dhcp6_dbus_delete_static_host_by_addr(&(temphost->hostip), (char *)temphost->hostmac);
			count_host--;
		}
		else {
			op_ret = 1;
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	if(hostname){
		free(hostname);
		hostname=NULL;
	}
	return reply;
	
}

/*state init or change */
DBusMessage *
dhcp6_dbus_recv_vrrp_state
( 
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusMessageIter	 iter_array, iter_array1;	
	DBusError		err;
	vrrp_param_t  params;
	char failname[MAX_IF_NAME_LEN];
	struct dcli_subnet* subnode = NULL;
	struct failover_backup_list* backup = NULL;
	unsigned int ret = 0, op_ret = 0, j = 0, primary = 0, goodip = 0;
	
	char master_ip_char[32]="";
	char backup_ip_char[32]="";
	char virtual_ip_char[32]="";
	char * interface_name = NULL;

	dbus_error_init( &err );
/*	DBUS_TYPE_UINT32					// vrid
	DBUS_TYPE_UINT32					// state
	DBUS_TYPE_UINT32					// count of uplink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
		DBUS_TYPE_STRING_AS_STRING		// uplink interface name
	DBUS_TYPE_UINT32					// count of downlink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
		DBUS_TYPE_STRING_AS_STRING		// downlink interface name
	DBUS_TYPE_STRING					// heartbeat interface name
	DBUS_TYPE_UINT32					// heartbeat ip address
	DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/
	memset(&params, 0, sizeof(params));
	memset(failname, 0, MAX_IF_NAME_LEN);
	log_debug("--------------- HA instance info ---------------");
	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter,&(params.vrid));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.state));
	
	log_debug("...vrid %d state %s", params.vrid, pha_state2str(params.state));

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
	
	log_debug("...uplink num %d", params.uplink_cnt);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	for (j = 0; j < params.uplink_cnt; j++) {
		if(j >= MAX_PHA_IF_NUM) {
			log_error("error!params.uplink_cnt=%d > MAX_PHA_IF_NUM=%d",params.uplink_cnt, MAX_PHA_IF_NUM);
			break;
		}
		DBusMessageIter iter_struct;
		dbus_message_iter_recurse(&iter_array,&iter_struct);	
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].master_ip));		

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].backp_ip));

		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(params.uplink_interface[j].virtual_ip));
		
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&(interface_name));

		strncpy( params.uplink_interface[j].if_name, interface_name, sizeof(params.uplink_interface[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array); 
		
		log_debug("...uplink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(params.uplink_interface[j].master_ip, master_ip_char, sizeof(master_ip_char) ),
					inet_int2str(params.uplink_interface[j].backp_ip, backup_ip_char, sizeof(backup_ip_char) ),
					inet_int2str(params.uplink_interface[j].virtual_ip, virtual_ip_char, sizeof(virtual_ip_char) ),
					params.uplink_interface[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.downlink_cnt));
	
	log_debug("...downlink num %d", params.downlink_cnt);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array1);
	for (j = 0; j < params.downlink_cnt; j++) {
		if(j >= MAX_PHA_IF_NUM) {
			log_error("error!params.downlink_cnt=%d > MAX_PHA_IF_NUM=%d",params.downlink_cnt, MAX_PHA_IF_NUM);
			break;
		}
		DBusMessageIter iter_struct1;
		dbus_message_iter_recurse(&iter_array1,&iter_struct1);	
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].master_ip));		

		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].backp_ip));

		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(params.downlink_interface[j].virtual_ip));
		
		dbus_message_iter_next(&iter_struct1);
		dbus_message_iter_get_basic(&iter_struct1,&(interface_name));

		strncpy( params.downlink_interface[j].if_name, interface_name, sizeof(params.downlink_interface[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array1); 
		
		log_debug("...downlink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(params.downlink_interface[j].master_ip,master_ip_char,sizeof(master_ip_char)),
					inet_int2str(params.downlink_interface[j].backp_ip,backup_ip_char,sizeof(backup_ip_char)),
					inet_int2str(params.downlink_interface[j].virtual_ip,virtual_ip_char,sizeof(virtual_ip_char)),
					params.downlink_interface[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
	
	log_debug("...vgateway num %d", params.vgateway_cnt);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array1);
	for (j = 0; j < params.vgateway_cnt; j++) {
		if(j >= MAX_PHA_IF_NUM) {
			log_error("error!params.downlink_cnt=%d > MAX_PHA_IF_NUM=%d",params.downlink_cnt, MAX_PHA_IF_NUM);
			break;
		}
		DBusMessageIter iter_struct1;
		dbus_message_iter_recurse(&iter_array1,&iter_struct1);	
		dbus_message_iter_get_basic(&iter_struct1,&(interface_name));

		strncpy( params.vgateway_interface[j].if_name, interface_name, sizeof(params.vgateway_interface[j].if_name)-1);
		
		dbus_message_iter_next(&iter_array1); 
		
		log_debug("...vgateway %d: ifname %s", j+1, params.vgateway_interface[j].if_name);
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(interface_name));
	strncpy((char *)params.heartlink_if_name, interface_name, sizeof(params.heartlink_if_name)-1);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_local_ip));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_opposite_ip));

	log_debug("...heartlink: name %s local_ip %s opposite_ip %s",
					params.heartlink_if_name,
					inet_int2str(params.heartlink_local_ip,master_ip_char,sizeof(master_ip_char)),
					inet_int2str(params.heartlink_opposite_ip,backup_ip_char,sizeof(backup_ip_char)) );

	if ((params.heartlink_local_ip) && (params.heartlink_opposite_ip)) {
		goodip = 1;
	}
	if (((VRRP_STATE_MAST == params.state) || (VRRP_STATE_BACK == params.state)) && goodip) {
		if (VRRP_STATE_BACK == params.state) {
			primary = 1;
		}

		ret = find_backup_list_by_vrrpid(params.vrid, &backup);
		if (ret) {/*can not find the back node for this vrrpid, add it*/
			backup = malloc(sizeof(struct failover_backup_list));
			memset(backup, 0, sizeof(struct failover_backup_list));
			for (j = 0; j < (params.downlink_cnt + params.vgateway_cnt); j++) {
				/*new one if_name one subnode*/
				if (j < params.downlink_cnt) {	
					log_debug("downlink interface name is %s, vrrpid is %d, \n", params.downlink_interface[j].if_name, params.vrid);
					ret = dhcp6_dbus_find_subnet_by_ifname(params.downlink_interface[j].if_name, &subnode);
				}
				else {				
					log_debug("vgateway interface name is %s, vrrpid is %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid);
					ret = dhcp6_dbus_find_subnet_by_ifname(params.vgateway_interface[j - params.downlink_cnt].if_name, &subnode);
				}
				if (!ret) {
					if (!(subnode->owned_failover.enable)) {
						//sprintf(failname, "peer%d", (params.vrid)*100 + j);
						sprintf(failname, "peer%d", params.vrid);
						log_debug("add new failover state name is %s\n", failname);
						memcpy(subnode->owned_failover.name, failname, strlen(failname));
						if (!primary) {
							subnode->owned_failover.split = 128;
							subnode->owned_failover.mclt = 10;
							subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params.vrid)*16 + j*2;
							subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params.vrid)*16 + j*2 + 100;
						}
						else {		
							subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params.vrid)*16 + j*2 + 100;
							subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params.vrid)*16 + j*2;
							subnode->owned_failover.primary = 1;/*0 is primary 1 is secondary*/
						}

						subnode->owned_failover.myaddr.iabuf[0] = ((params.heartlink_local_ip)>>24)&0xff;
						subnode->owned_failover.myaddr.iabuf[1] = ((params.heartlink_local_ip)>>16)&0xff;
						subnode->owned_failover.myaddr.iabuf[2] = ((params.heartlink_local_ip)>>8)&0xff;
						subnode->owned_failover.myaddr.iabuf[3] = (params.heartlink_local_ip)&0xff;
						subnode->owned_failover.myaddr.len = DHCP_IPV6_ADDRESS_LEN;
						subnode->owned_failover.parteraddr.iabuf[0] = ((params.heartlink_opposite_ip)>>24)&0xff;
						subnode->owned_failover.parteraddr.iabuf[1] = ((params.heartlink_opposite_ip)>>16)&0xff;
						subnode->owned_failover.parteraddr.iabuf[2] = ((params.heartlink_opposite_ip)>>8)&0xff;
						subnode->owned_failover.parteraddr.iabuf[3] = (params.heartlink_opposite_ip)&0xff;
						subnode->owned_failover.parteraddr.len = DHCP_IPV6_ADDRESS_LEN;

						//ret = dhcp_dbus_set_sub_failover(subnode);
						//sleep(1);
						if (!ret) {
							subnode->owned_failover.enable = 1;
							/*if is secondary add it in backup list, do not work
							if (subnode->owned_failover.primary) {*/
							if (j < params.downlink_cnt) {	
								log_debug("add backup downlink interface name is %s, vrrpid is %d, \n", params.downlink_interface[j].if_name, params.vrid);
								memcpy(backup->dhcp_interface[j].if_name, params.downlink_interface[j].if_name, strlen(params.downlink_interface[j].if_name));
							}
							else {				
								log_debug("add backup vgateway interface name is %s, vrrpid is %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid);
								memcpy(backup->dhcp_interface[j].if_name, params.vgateway_interface[j - params.downlink_cnt].if_name, strlen(params.vgateway_interface[j - params.downlink_cnt].if_name));
							}
							/*}*/
						}
						else {
							memset(&(subnode->owned_failover), 0, sizeof(struct dcli_failover));
						}
					}
					else {
						log_error("subnet is failover yet\n");
						op_ret = 2;
					}
				}
				else {	
					log_error("can not find the subnet by the interface\n");
					op_ret = 2;
				}	
			}
			
			backup->interface_num = j;

			if (backup->interface_num) {
				log_debug("add %d failover state\n", backup->interface_num);
				backup->vrrpid = params.vrid;
				backup->state = primary;
				backup->next = failover_backup_list_head.next;
				failover_backup_list_head.next = backup;
			}
		}
		else {/*state change*/
			for (j = 0; j < (params.downlink_cnt + params.vgateway_cnt); j++) {
				if (j < params.downlink_cnt) {	
					log_debug("downlink interface is %s, vrrpid is %d\n", params.downlink_interface[j].if_name, params.vrid);
					ret = dhcp6_dbus_find_subnet_by_ifname(params.downlink_interface[j].if_name, &subnode);
				}
				else {				
					log_debug("vgateway interface is %s, vrrpid is %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid);
					ret = dhcp6_dbus_find_subnet_by_ifname(params.vgateway_interface[j - params.downlink_cnt].if_name, &subnode);
				}
				
				if (!ret) {
					if (subnode->owned_failover.primary != primary) {
						subnode->owned_failover.primary = primary;
						backup->state = primary;
					}
					else {
						log_error("failover state change error same primary \n");
						op_ret = 2;
					}
				}
				else {
					log_error("when change state can not find subnet by the interface\n");
				}
			}
		}
	}
	/* test 
	params.master_uplinkip = params.uplink_interface[0].master_ip;
	params.master_downlinkip = params.downlink_interface[0].master_ip;
	params.back_uplinkip = params.uplink_interface[0].backp_ip;
	params.back_downlinkip = params.downlink_interface[0].backp_ip;
	params.virtual_uplink_ip = params.uplink_interface[0].virtual_ip;
	params.virtual_downlink_ip = params.downlink_interface[0].virtual_ip;
	params.heartlink_master_ip = params.heartlink_opposite_ip;	
	strncpy(params.uplink_if, params.uplink_interface[0].if_name, sizeof(params.uplink_if));
	strncpy(params.downlink_if, params.downlink_interface[0].if_name, sizeof(params.downlink_if));
	strncpy(params.heartlink_if, params.heartlink_if_name, sizeof(params.heartlink_if));
 */

 #if 0	
 	if( NULL != onstate_change_callback )
	{
		onstate_change_callback( &params );
		log_dbg("call the portal_ha ins done");
	}



	int vrrp_count =0,				/* vrrp_count */
		vrid = 0,					/* HA instance id */
		state = 0,					/* HA state for the instance */
		up_link_count = 0,			/* uplink count */

		master_uplinkip = 0,		/* HA master uplink ip address */
		back_uplinkip = 0,			/* HA backup uplink ip address */
		virtual_uplink_ip = 0,	/* HA uplink virtual ip address */ 
		
		down_link_count = 0,		/* downlink count */		
		master_downlinkip = 0,	/* HA master downlink ip address */ 
		back_downlinkip = 0,  		/* HA backup downlink ip address */
		virtual_downlink_ip = 0, 	/* HA downlink virtual ip address */
		
	 	heartlink_local_ip = 0,	/* HA heartbeat line local ip address */
		heartlink_opposite_ip = 0;	/* HA heartbeat line master ip address */

	char * uplink_if = NULL;/*(char *)malloc(21);*/
	/*memset( uplink_if,0, 21);*/
	char * downlink_if = NULL;/* (char *)malloc(21);*/
	/*memset( downlink_if,0, 21);*/
	char * heartlink_if = NULL;/*(char *)malloc(21);*/
	UINT32 uip = 0, dip = 0;
	/* these buff malloc by  dbus.*/
	
	if( !(dbus_message_get_args( msg ,&err, \
							DBUS_TYPE_UINT32,&vrid,\
							DBUS_TYPE_UINT32,&state,\
							DBUS_TYPE_UINT32,&master_uplinkip,\
							DBUS_TYPE_UINT32,&master_downlinkip,\
							DBUS_TYPE_UINT32,&back_uplinkip,\
							DBUS_TYPE_UINT32,&back_downlinkip, \
							DBUS_TYPE_UINT32,&virtual_uplink_ip,\
							DBUS_TYPE_UINT32,&virtual_downlink_ip,\
							DBUS_TYPE_STRING,&uplink_if,\
							DBUS_TYPE_STRING,&downlink_if,\
							DBUS_TYPE_STRING,&heartlink_if,\
							DBUS_TYPE_UINT32,&heartlink_local_ip,\
							DBUS_TYPE_UINT32,&heartlink_opposite_ip,\
							DBUS_TYPE_INVALID)))
	{
		log_err("HA portal dbus set state unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			log_err(" error message %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	/* debug out dbus message info */
	log_dbg("HA portal dbus message set state");
	log_dbg("...vrid %d state %d %s uplink %s downlink %s heartbeat %s", \
			vrid, state, (state < EAG_PORTAL_HA_STATE_INVALID) ? portalHaStateDesc[state] : "invalid", \
			uplink_if ? uplink_if : "nil", downlink_if ? downlink_if : "nil",heartlink_if ? heartlink_if : "nil");
	uip = master_uplinkip;
	dip = master_downlinkip;
	log_dbg("...master uplink %d.%d.%d.%d downlink %d.%d.%d.%d", (uip>>24) & 0xff, (uip>>16) & 0xff,  \
			(uip>>8) & 0xff, uip & 0xff, (dip>>24)&0xff, (dip>>16) & 0xff, (uip>>8) & 0xff, uip & 0xff);
	uip = back_uplinkip;
	dip = back_downlinkip;
	log_dbg("...back uplink %d.%d.%d.%d downlink %d.%d.%d.%d", (uip>>24) & 0xff, (uip>>16) & 0xff,	\
			(uip>>8) & 0xff, uip & 0xff, (dip>>24)&0xff, (dip>>16) & 0xff, (uip>>8) & 0xff, uip & 0xff);
	uip = virtual_uplink_ip;
	dip = virtual_downlink_ip;
	log_dbg("...virtual uplink %d.%d.%d.%d downlink %d.%d.%d.%d", (uip>>24) & 0xff, (uip>>16) & 0xff,  \
			(uip>>8) & 0xff, uip & 0xff, (dip>>24)&0xff, (dip>>16) & 0xff, (uip>>8) & 0xff, uip & 0xff);
	uip = heartlink_local_ip;
	dip = heartlink_opposite_ip;
	log_dbg("...heartbeat local %d.%d.%d.%d opposite %d.%d.%d.%d", (uip>>24) & 0xff, (uip>>16) & 0xff,  \
			(uip>>8) & 0xff, uip & 0xff, (dip>>24)&0xff, (dip>>16) & 0xff, (uip>>8) & 0xff, uip & 0xff);
	/*create new portal_ha or modify portal_ha state!!*/
{
	portal_ha *pha=NULL;

	memset( &params, 0, sizeof(params));
	params.vrid = vrid;
	params.state = state;
	params.master_downlinkip = master_uplinkip;
	params.master_downlinkip = master_downlinkip;
	params.back_uplinkip = back_uplinkip;
	params.back_downlinkip = back_downlinkip;
	params.virtual_uplink_ip = virtual_uplink_ip;
	params.virtual_downlink_ip = virtual_downlink_ip;
	strncpy( params.uplink_if, uplink_if, sizeof(params.uplink_if) );
	strncpy( params.downlink_if, downlink_if, sizeof(params.downlink_if) );
	strncpy( params.heartlink_if, heartlink_if, sizeof(params.heartlink_if) );
	params.heartlink_local_ip = heartlink_local_ip;
	params.heartlink_master_ip = heartlink_opposite_ip;
	#if 0
	pha = get_portal_ha_by_vrid( portal_ha_head, vrid );
	if( NULL == pha )
	{
		
		log_dbg("search HA portal instance via vrid %d result not found, create new one!\n", vrid);
		pha = portal_ha_new( &params );
        
        if( NULL == pha )
        {
			log_err( "create new portal ha failed!" );
        	ret = -1;
        }
        else
        {
        
        	add_portal_ha_to_list( &portal_ha_head, &portal_ha_tail, pha );
        }
 
 		log_dbg("before set_onstate_change_cb\n");
    
        set_onstate_change_cb( pha, onstate_change_callback );/*on_vrrp_state_change is define in eag.c. i don't want this extern call!!!!! shaojunwu. it will be change*/
	}
	else
	{
		/*modify state*/
		log_info("search HA portal instance via vrid %d found, update it!\n", vrid);
		portal_ha_change_state( pha, &params );
        
	}
	#endif
	
	memcpy( &(pha->params), &params, sizeof(pha->params));
	set_onstate_change_cb( pha, onstate_change_callback );
	log_dbg("call the eag_ins ,pha->cb_onchange = %p", pha->cb_onchange);
	if( NULL != pha->cb_onchange )
	{
		pha->cb_onchange( pha );
	}else
	{
		log_err("pha->cb_onchange not set");
	}
	
}
	#endif

	reply = dbus_message_new_method_return(msg);
	if (!reply) {
		log_error("portal HA dbus set state get reply message null error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(params.vrid));

	return reply;
} 

DBusMessage* 
dhcp6_dbus_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(DHCP_SAVE_CFG_MEM);
	
	if(!strShow) {
		log_debug("alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(strShow, 0, DHCP_SAVE_CFG_MEM);

	dbus_error_init(&err);

	dhcp6_dbus_profile_config_save(strShow);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);	
	free(strShow);
	strShow = NULL;
	
	return reply;
}


/* init dhcp dbus */
static DBusHandlerResult 
dhcp6_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage		*reply = NULL;

	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SAVE_DHCP_LEASE)) {
		reply = dhcp6_dbus_save_lease(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE)) {
		reply = dhcp6_dbus_show_lease(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_STATIC_HOST)) {
		reply = dhcp6_dbus_show_static_host(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP)) {
		reply = dhcp6_dbus_show_lease_by_ip(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_STATE)) {
		reply = dhcp6_dbus_show_lease_state(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_GET_STATISTICS_INFO)) {
		reply = dhcp6_dbus_show_statistics_info(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC)) {
		reply = dhcp6_dbus_show_lease_by_mac(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF)) {
		reply = dhcp6_dbus_show_dhcp_global_conf(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_IP_POOL_CONF)) {
		reply = dhcp6_dbus_show_dhcp_pool_conf(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_ENTRY_POOL_NODE)) {
		reply = dhcp6_dbus_entry_pool_node(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_INTERFACE_POOL)) {
		reply = dhcp6_dbus_set_interface_pool(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_CREATE_POOL_NODE )) {
		reply = dhcp6_dbus_create_pool_node(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG)) {
		reply = dhcp6_dbus_show_running_hansi_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_DOMAIN_SEARCH)) {
		reply = dhcp6_dbus_set_server_domain_search(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_VEO)) {
		reply = dhcp6_dbus_set_server_veo(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_DNS)) {
		reply = dhcp6_dbus_set_server_name_servers(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_ROUTERS_IP)) {
		reply = dhcp6_dbus_set_server_routers_ip(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_ENABLE)) {
		reply = dhcp6_dbus_set_server_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_DEBUG_STATE)) {
		reply = dhcp6_dbus_set_debug_state(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_WINS_IP)) {
		reply = dhcp6_dbus_set_server_wins_ip(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT)) {
		reply = dhcp6_dbus_set_server_lease_default(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_OPTION52)) {
		reply = dhcp6_dbus_set_server_option52(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SET_SERVER_LEASE_MAX)) {
		reply = dhcp6_dbus_set_server_lease_max(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_ADD_IP_POOL_RANGE)) {
		reply = dhcp6_dbus_add_dhcp_pool_ip_range(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_GET_SERVER_STATE)) {
		reply = dhcp6_dbus_get_server_state(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_ADD_STATIC_HOST)) {
		reply = dhcp6_dbus_add_dhcp_static_host(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP6_DBUS_INTERFACE, DHCP6_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		reply = dhcp6_dbus_show_running_cfg(connection, message, user_data);
	}
	
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult 
dhcp6_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */
		dbus_connection_unref (dhcp6_dbus_connection);
		dhcp6_dbus_connection = NULL;

		/*g_timeout_add (3000, reinit_dbus, NULL);*/

	} 
	else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

		/*if (services_with_locks != NULL)  service_deleted (message);*/
	}
	else {
		return 1;
	}
		/*return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);*/

	return DBUS_HANDLER_RESULT_HANDLED;
}

void 
dhcp6_tell_whoami
(
	char * myName,
	int isLast
)
{
	char pidBuf[DHCP6_PID_BUFFER_LEN] = {0}, pidPath[DHCP6_PID_BUFFER_LEN] = {0};
	pid_t myPid = 0;
	
	if(!myName) {
		return;
	}

	sprintf(pidPath,"%s%s%d%s", DHCP6_PID_FILE_PATH, DHCP6_PID_FILE_PREFIX, \
				global_current_instance_no, DHCP6_PID_FILE_SUFFIX);
		
	if(DHCP6_FD_INIT == g_dhcp6_pid_fd) {	
		g_dhcp6_pid_fd = open(pidPath, O_RDWR|O_CREAT);
		if(DHCP6_FD_INIT == g_dhcp6_pid_fd) {
			return;
		}
	}

	myPid = getpid();
	
	sprintf(pidBuf,"instance %d %s has pid %d\n", global_current_instance_no, myName, myPid);
	write(g_dhcp6_pid_fd, pidBuf, strlen(pidBuf));

	/* close pid file by last teller */
	if(isLast) {
		close(g_dhcp6_pid_fd);
		g_dhcp6_pid_fd = DHCP6_FD_INIT;
	}

	return;
}

int 
dhcp6_dbus_init
(
	void
)
{
	DBusError dbus_error;
	DBusObjectPathVTable	dhcp_vtable = {NULL, &dhcp6_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (1);

	dbus_error_init (&dbus_error);
	dhcp6_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dhcp6_dbus_connection == NULL) {
		log_error ("dbus_bus_get(): %s", dbus_error.message);
		return 1;
	}

	/* Use dhcp to handle subsection of DHCP_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (dhcp6_dbus_connection, DHCP6_DBUS_OBJPATH, &dhcp_vtable, NULL)) {
		log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
		
	dbus_bus_request_name (dhcp6_dbus_connection, DHCP6_DBUS_BUSNAME,
			       0, &dbus_error);
		
	if (dbus_error_is_set (&dbus_error)) {
		log_error ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return 1;
	}

	dbus_connection_add_filter (dhcp6_dbus_connection, dhcp6_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (dhcp6_dbus_connection,
			    		"type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

	return 0;
}

void * 
dhcp6_dbus_thread_main
(
	void *arg
)
{

	dhcp6_tell_whoami("dbusDhcp6", 0);

	/* tell about my initialization process done
	npd_init_tell_stage_end();
*/	
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(dhcp6_dbus_connection,-1)) {
		;
	}
	
	return NULL;
}

void 
dhcp6_dbus_start
(
	void
)
{
	int ret = 0;
	memset(&head_pool, 0, sizeof(struct dcli_pool));
	dhcp6_dbus_init();
	set_server_duid_new();
	
	dhcp6_dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dhcp6_dbus_thread_attr);
	ret = pthread_create(dhcp6_dbus_thread, &dhcp6_dbus_thread_attr, dhcp6_dbus_thread_main, NULL);
    if (0 != ret) {
	   log_error ("start dhcp6 dbus pthread fail\n");
	}

	dhcp6_netlink_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dhcp6_netlink_thread_attr);
	ret = pthread_create(dhcp6_netlink_thread, &dhcp6_netlink_thread_attr, dhcp6_receive_netlink, NULL);
	if (0 != ret) {
	   log_error ("start dhcp6 netlink pthread fail\n");
	}
	
}
#ifdef __cplusplus
}
#endif


