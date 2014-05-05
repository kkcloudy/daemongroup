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
* dhcp_dbus.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 11/26/2009 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		11/26/2009
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.41 $	
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
#include <sys/ioctl.h>
#include <linux/rtnetlink.h>
#include <bits/sockaddr.h>

#include "sysdef/returncode.h"
#include "dhcpd.h"
#include "dhcp_dbus.h"
#include "dhcp_snp_netlink.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
#ifdef _D_WCPSS_
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#elif _D_CC_
#include "cc_wcpss/waw.h"
#include "cc_wcpss/wid/WID.h"
#include "cc_wcpss/asd/asd.h"
#endif
#include "pppoe_snp_dbus.h"
#include "pppoe_snp_netlink.h"

static void dhcp_dbus_restart(void);

int parse_statement_subnet_no_file
(
	struct subnet_config *sub_conf,
	struct group *group,
	int type
);
int get_dhcp_lease_state_num
(
	struct dbus_lease_state  *lease_state , 
	struct dbus_sub_lease_state **sub_lease_state, 
	unsigned int *sub_count
);

void dhcp_dbus_profile_show_running(char *showStr);

/* mdb.c */
void expire_pool_for_failover(struct dcli_failover* failover_peer,struct shared_network* share);
struct dhcp_lease_info *get_lease_ip_info_by_ip(unsigned int ip_addr, unsigned int ip_nums);
int get_lease_ip_info_by_hw(unsigned char *hw_addr, struct dhcp_lease_ip_info *lease_ip);
int save_vgateway_virtual_ip_in_interface(vrrp_param_t  params);
int set_interface_bind_flag(char *ifname , unsigned int unbind_flag);
int get_dhcp_lease_info(struct dhcp_lease_info *lease_info, int count);
void dhcp_get_statistics_info(struct statistics_info *info);
void test_for_del_subnet_pool(struct dcli_subnet *delsub);
void test_for_del_subnet_interface(char* interface_name);
int release_lease_by_addr(struct iaddr addr);
unsigned int get_total_host_num(void);
int get_dhcp_lease_number(void);
void get_dhcp_pool_info(void);
int free_lease_ip_info(struct dhcp_lease_info** lease_info);





/* Confpars.c */
int parse_failover_peer_no_file(	struct dcli_failover *conf, struct group *group,int type);
void parse_pool_statement_no_file(struct range_config *range_conf, struct group *group, int type);
void parse_host_declaration_test(char*,struct iaddr*,struct hardware* ,struct group *);
int delete_host_by_name(	char* name); 

/* Parse.c */
isc_result_t set_server_option(char *,	char *,int,int,unsigned int,struct group *);


/* discover.c */
struct iface_conf_list;
struct iface_info;
extern int begin_iface_scan(struct iface_conf_list *ifaces);
extern int next_iface(struct iface_info *info, int *err, struct iface_conf_list *ifaces);
extern void end_iface_scan(struct iface_conf_list *ifaces);

/* errwarn.c */
void dhcp_write_crit_log(char *format, ...);



#define DHCP_PID_BUFFER_LEN		128
#define DHCP_HOST_NAME_LEN		16
#define DHCP_MAC_ADDRESS_LEN	6
#define DHCP_IPV4_ADDRESS_LEN	4
#define DHCP_PID_PATH_LEN		DHCP_PID_BUFFER_LEN
#define DHCP_INVALID_FD		(-1)
#define DHCP_FD_INIT		DHCP_INVALID_FD
#define DHCP_SAVE_CFG_MEM	(4*1024*1024)
#define DHCP_MAX_HOST_NUM	(256*1024)
#define DHCP_MIN_MASK		0XFFFF
#define DHCP_MAX_POOL_NUM	1024
#define DHCP_MAX_OPTION_IP_LIST 16
#define  BUFSIZE	512
#define  SHOW_DHCP_POOL_FREE    \
    do{\
        if(domainname != NULL){                \
            free(domainname);                  \
        }                                      \
        if(option43){                          \
            free(option43);                    \
        }                                      \
        if(tmp_interface_name){                \
            free(tmp_interface_name);          \
        }                                      \
        for(i = 0; i < 16; ++i){               \
            if(ap_via_address[i] != NULL){     \
                free(ap_via_address[i]);       \
            }                                  \
        }                                      \
        if(ap_via_address != NULL){            \
           free(ap_via_address);               \
        }                                      \
    }while(0);                                  \

#define ALIAS_NAME_SIZE		0x15

#define DHCP_PID_FILE_PATH "/var/run/"
#define DHCP_PID_FILE_PREFIX "dhcp"
#define DHCP_PID_FILE_SUFFIX ".pid"

#define DHCP_POOLINFO	"/var/run/apache2/dhcp_poolinfo"
#define DHCP_NOTIFY_WEB_EVENT "dhcp_notify_web_pool_event"
#define DHCP_DBUS_METHOD_GET_POOL_INFO		"get_pool_info"

pthread_t		*dbus_thread, *netlink_thread;
pthread_attr_t	dbus_thread_attr, netlink_thread_attr;
DBusConnection *dhcp_dbus_connection = NULL;
unsigned int global_current_instance_no = 0;
int g_dhcp_pid_fd = DHCP_INVALID_FD;
struct dcli_pool head_pool;
unsigned int count_pool;
struct dcli_host head_host;
unsigned int count_host;
unsigned int dhcp_server_start;
unsigned int pool_index;
unsigned int host_index;
struct subnet_config sub_conf;
struct dcli_option global_server_option;
char null_mac_addr[DHCP_MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0, 0};

int dhcp_dynamic_arp_switch = 0;

extern unsigned int dba_server_enable_flag;	/* dhcp broadcast agent */
extern unsigned int pppoe_snp_enable_flag;

extern unsigned int client_lease_time_enable ;
extern struct pppoe_snp_cfg *pppoe_snp_cfg_head;

/*set offer reply unicast mode flag 1: unicast , 0 : default*/
extern unsigned int unicast_flag;

/*debug level*/
extern unsigned int dhcp_log_level;
extern unsigned int dhcp_failover_debug_log_level;

pthread_mutex_t DhcpDbusEventMutex = PTHREAD_MUTEX_INITIALIZER;

extern pthread_mutex_t DhcpthreadMutex;

extern unsigned int dhcp_ping_check_enable;
extern unsigned int dhcp_lease_expiry_time;
extern unsigned int dhcp_optimize_enable;

/*
   If a DHCP server returns a automatic address, the BOOTP client will
   not be aware of the DHCP lease mechanism for network address
   assignment.  Thus the DHCP server must assign an infinite lease
   duration to for automatic addresses assigned to BOOTP clients.  Such
   network addresses cannot be automatically reassigned by the server.
   The local system administrator may choose to manually release network
   addresses assigned to BOOTP clients.
*/
extern int dhcp_bootp_enable;		/* bootp support. 0 : disabel, !0 enable */

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
int flag_shutdown = 0; //in order not to transit to shut_down for swichover between master and back   
/**********************************************************************************
 *  init_sock_unix_client
 *
 *	DESCRIPTION:
 * 		create unix sock
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		fd  ->  socket fd
 *		-1  ->  failed
 *		
 **********************************************************************************/
int init_sock_unix_client(void)
{
	int sockfd = -1;   
    struct sockaddr_un clientaddr;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {   
		log_error("%s: create sock failed: %m\n", __func__);
        return -1;   
    }     

    clientaddr.sun_family = AF_UNIX;   
	/* tmpnam  */
	strncpy(clientaddr.sun_path, tmpnam(NULL), sizeof(clientaddr.sun_path)-1);
    if (-1 == bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr))) {   
		log_error("%s: sock bind error : %m\n", __func__);
        close(sockfd);   
        return -1;   
	}
	return sockfd;
}


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
#endif

#if (defined _D_WCPSS_ || defined _D_CC_)
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

			if(*wlanid > 0 && *wlanid < EBR_NUM+1){
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


static unsigned long dhcp_dbus_ip2ulong(char *str)
{
	char *sep=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;
	
	token=strtok(str,sep);
	if(NULL != token){
	    ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,sep);
		if(NULL != token){
		    ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}


#if 0
static int strtoaddr(const char *adr,  struct iaddr *bin_adr)
{
	unsigned int adress;

	if(NULL == adr){
		return 1;
	}

	adress = dhcp_dbus_ip2ulong((char *)adr);

	bin_adr->iabuf[0] = (adress>>24)&0xff;		
	bin_adr->iabuf[1] = (adress>>16)&0xff;
	bin_adr->iabuf[2] = (adress>>8)&0xff;
	bin_adr->iabuf[3] = adress&0xff;
	bin_adr->len = 4;

	return 0;
}
#endif
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
 *  check_mask
 *
 *	DESCRIPTION:
 * 		Check the legality of the subnet mask
 *
 *	INPUT:
 *		mask - char *
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1  ->  Illegal
 *		0  ->  regular
 *		
 **********************************************************************************/
#if 0
static int check_mask(char *mask)
{
	char *sep = ".";
	char *token = NULL;
	unsigned long ip_long[4];
	int ip = 0;
	int i = 1;

	if(NULL == mask){
	    return 1;
	}
	token=strtok(mask,sep);
	ip_long[0] = strtoul(token,NULL,10);
	if(ip_long[0] < 0 || ip_long[0] > 255){
	    return 1;
	}

	while((token != NULL) && (i < 4))
	{
	    token=strtok(NULL , sep);
	    ip_long[i] = strtoul(token, NULL, 10);

	    if(ip_long[i] < 0 || ip_long[i] > 255){
	        return 1;
	    }

	    /*255.255.255.255*/
	    if(3 == i){
	        if(255 == ip_long[i]){
	            return 1;
	        }
	    }
	    i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	if((ip & ip >> 1) == ip){

	    return 0;
	}else{

	    return 1;
	}
}
#endif


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

int dhcp_dbus_parse_ifname(struct distributed_iface_info *iface_info, char *ifname)
{
	int slotid = 0, vrrpid = 0, ifnameid = 0;
	int ret = 0;
	
	if (!iface_info || !ifname) {
		log_error("dhcp parse iface name failed: parameters null.\n");
		return -1;
	}

	if (!strncmp(ifname, "ebrl", 4)) {
		ret = sscanf(ifname, "ebrl%d-%d-%d", &slotid, &vrrpid, &ifnameid);
		if ((3 == ret)) {
			iface_info->slotid = slotid;
			iface_info->vrrpid = vrrpid;
			iface_info->ifnameid = ifnameid;
			iface_info->local_flag = 1;
			
			memset(iface_info->ifname, 0, sizeof(iface_info->ifname));
			sprintf(iface_info->ifname, "ebr%d", ifnameid);
		}
	} else if (!strncmp(ifname, "wlanl", 5)) {
		ret = sscanf(ifname, "wlanl%d-%d-%d", &slotid, &vrrpid, &ifnameid);
		if ((3 == ret)) {
			iface_info->slotid = slotid;
			iface_info->vrrpid = vrrpid;
			iface_info->ifnameid = ifnameid;
			iface_info->local_flag = 1;

			memset(iface_info->ifname, 0, sizeof(iface_info->ifname));
			sprintf(iface_info->ifname, "wlan%d", ifnameid);
		}
	} else if ((!strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
		ret = sscanf(ifname, "ebr%d-%d-%d", &slotid, &vrrpid, &ifnameid);
		if ((3 == ret)) {
			iface_info->slotid = slotid;
			iface_info->vrrpid = vrrpid;
			iface_info->ifnameid = ifnameid;
			iface_info->local_flag = 0;
			
			memset(iface_info->ifname, 0, sizeof(iface_info->ifname));
			sprintf(iface_info->ifname, "ebr%d", ifnameid);
		}
	} else if ((!strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
		ret = sscanf(ifname, "wlan%d-%d-%d", &slotid, &vrrpid, &ifnameid);
		if ((3 == ret)) {
			iface_info->slotid = slotid;
			iface_info->vrrpid = vrrpid;
			iface_info->ifnameid = ifnameid;
			iface_info->local_flag = 0;

			memset(iface_info->ifname, 0, sizeof(iface_info->ifname));
			sprintf(iface_info->ifname, "wlan%d", ifnameid);
		}
	} else {

	}

	log_debug("interface %s slot %d vrrpid %d ifaceid %d local flag %d ifname %s\n", ifname, 
		iface_info->slotid, iface_info->vrrpid, iface_info->ifnameid, 
		iface_info->local_flag, iface_info->ifname);

	return 0;
}

inline int __check_mask(unsigned int mask)
{
	int i = 0;
	
	if (mask < (1<<31)) {
		return -1;
	}

	for (i=30; i>=0; i--) {
		if ((mask & (1<<i)) && !(mask & 1<<(i+1))) {
			return -1;
		}
	}
	return 0;
}

int dhcp_dbus_check_mask(unsigned int mask) 
{	
	if (__check_mask(mask)) {
		return -1;
	}
	return 0;
}


inline int dhcp_dbus_check_ipaddr_uint(unsigned int ipaddr) 
{	
	if ((0xffffffff == ipaddr)
		|| ((ipaddr & 0xff000000) == 0)) {
		return -1;
	}
	return 0;
}

int dhcp_dbus_check_ip_address(char *addr, void *out) 
{	
	unsigned int ipaddr = 0;

	if(1 == inet_pton(AF_INET, addr, &ipaddr)) {
		if ((0xffffffff == ipaddr)
			|| (0 == (0xff000000 & ipaddr))) {
			return -1;
		}
		if (out) {
			*(unsigned int *)out = ipaddr;
		}
		return 0;
	}
	return -1;
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

	if(masklen1 != masklen2){
		return 1;
	}
	
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
 *  dhcp_dbus_find_pool_by_subnet
 *
 *	DESCRIPTION:
 * 		get pool_name from dhcp_poolinfo file According to subnet
 *
 *	INPUT:
 *		set_subnet - struct subnet
 *	
 *	OUTPUT:
 *		pool_name
 *
 * 	RETURN:
 *		
 *		< 0 	 ->  fail 
 *		other ->  ok
 *		
 **********************************************************************************/
static int dhcp_dbus_find_pool_by_subnet(struct subnet *set_subnet, char *pool_name)
{
#define DHCP_LINE_BUFFER 1024

	int len = -1;
	struct dcli_pool* poolnode = NULL;
	struct dcli_subnet* subnetnode = NULL;
	unsigned int pool_count = 0;
	poolnode = head_pool.next;
	pool_count = count_pool;

	if(NULL == set_subnet){
		return len;
	}

	
	for(poolnode = head_pool.next; poolnode; poolnode = poolnode->next){
		
		for(subnetnode = poolnode->headsubnet; subnetnode; subnetnode = subnetnode->next){
			
			if(!compare_subnet(set_subnet->net, set_subnet->netmask, subnetnode->lowip, subnetnode->mask)){			   
				if(poolnode->poolname != NULL){
					len =  sprintf(pool_name,  poolnode->poolname);	
					return len;
				}				
		   	}			
		}
		
	}  

	return len;					
	
}

/**********************************************************************************
 *  dhcp_dbus_find_poolnode_by_subnet
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
struct dcli_pool *dhcp_dbus_find_poolnode_by_subnet(struct subnet *subnet)
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


/**********************************************************************************
 *  dhcp_notify_to_trap_event
 *
 *	DESCRIPTION:
 * 		send dhcp pool warnning 
 *
 *	INPUT:
 *		flag 
 *			1 -> Address exhaust warnning
 *			0 -> clear warnning
 *		
 *		total_count -> Address total number in pool
 *			
 *		count->  Address have been used total
 *
 *		set_subnet -> Belong to subnet
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		void
 *		
 **********************************************************************************/


void 
dhcp_notify_to_trap_event
(
	unsigned int flags, /*0 not full 1 full*/
	unsigned int total_count,
	unsigned int count,
	struct subnet *set_subnet
)
{
#define DHCP_POOLNAME_SIZE 	1024
	DBusMessage* msg = NULL;
	static DBusConnection* conn = NULL;
	DBusError err;
	unsigned int serial = 0;
	int ret = 0, len = -1;
	char pool_name[DHCP_POOLNAME_SIZE] = {0};
	char *poolname = NULL;
	time_t tm;
	char buf[64];

	if(NULL == set_subnet){
		return;
	}
	tm = time(NULL);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s", asctime(localtime(&tm)));
	if ('\n' == buf[strlen(buf) - 1]){
		buf[strlen(buf) - 1] = '\0';
	}

	dbus_error_init(&err);

	if (NULL == conn) {
		/* connect to the DBUS system bus, and check for errors */
		conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
		if (dbus_error_is_set(&err)) {
			log_error("notify trap get dbus connection Error (%s)\n", err.message);
			dbus_error_free(&err);
			return;
		}
		if (NULL == conn) {
			log_error("notify trap get dbus Connection null\n");
			return;
		}
	}
	/* register our name on the bus, and check for errors */
	ret = dbus_bus_request_name(conn,
								"aw.dhcp.signal",
								DBUS_NAME_FLAG_REPLACE_EXISTING,
								&err);
	if (dbus_error_is_set(&err)) {
		log_error("notify trap dbus request Name Error (%s)\n", err.message);
		dbus_error_free(&err);
		return;
	}
	
	/* create a signal & check for errors */
	msg = dbus_message_new_signal(DHCP_DBUS_OBJPATH,				/* object name of the signal */
								  "aw.trap", 					/* interface name of the signal */
								  DHCP_NOTIFY_WEB_EVENT); /* name of the signal */
	if (NULL == msg) {
		log_error("notify trap dbus new Message Null\n");
		return;
	}	
	
	len = dhcp_dbus_find_pool_by_subnet(set_subnet, pool_name);
	if (len < 0) {
#if 0	/*for test China Mobile*/
		log_info("notify to trap subnet %d.%d.%d.%d mask %d.%d.%d.%d %s but pool not found,cancel!\n",\
				set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
				set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2],  \
				set_subnet->netmask.iabuf[3], flags ? "no resource":"resource recover");
		return;
#endif
		poolname = malloc(DHCP_POOLNAME_SIZE);
		if(!poolname) {
			log_error("notify to trap subnet %d.%d.%d.%d mask %d.%d.%d.%d %s out of memory!\n",\
				set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
				set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2],  \
				set_subnet->netmask.iabuf[3], flags ? "no resource":"resource recover");
			return;
		}
		memset(poolname, 0, DHCP_POOLNAME_SIZE);
		sprintf(poolname, piaddr(set_subnet->net));
		log_info("notify to trap pool %s subnet %d.%d.%d.%d mask %d.%d.%d.%d %s!\n",\
			poolname, set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
			set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2],  \
			set_subnet->netmask.iabuf[3], flags ? "no resource":"resource recover");
		
	} else {
		poolname = malloc(DHCP_POOLNAME_SIZE);
		if(!poolname) {
			log_error("notify to trap subnet %d.%d.%d.%d mask %d.%d.%d.%d %s out of memory!\n",\
				set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
				set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2],  \
				set_subnet->netmask.iabuf[3], flags ? "no resource":"resource recover");
			return;
		}
		memset(poolname, 0, DHCP_POOLNAME_SIZE);
		memcpy(poolname, pool_name, (len < DHCP_POOLNAME_SIZE) ? len : DHCP_POOLNAME_SIZE);
		log_info("notify to trap pool %s subnet %d.%d.%d.%d mask %d.%d.%d.%d %s!\n",\
			poolname, set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
			set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2],  \
			set_subnet->netmask.iabuf[3], flags ? "no resource":"resource recover");
		if(flags){
			dhcp_write_crit_log("%% DHCP warn:%s [%s] [%s] [%d.%d.%d.%d %d.%d.%d.%d] has no free ip,please check!\n", buf, \
			(set_subnet->interface) ? (set_subnet->interface->name)  : "" , poolname, \
			set_subnet->net.iabuf[0], set_subnet->net.iabuf[1], set_subnet->net.iabuf[2], set_subnet->net.iabuf[3], \
			set_subnet->netmask.iabuf[0], set_subnet->netmask.iabuf[1], set_subnet->netmask.iabuf[2], \
			set_subnet->netmask.iabuf[3]);
		}
	}

	/* append arguments onto signal */
	dbus_message_append_args(msg,
							DBUS_TYPE_UINT32, &flags,
							DBUS_TYPE_UINT32, &total_count,
							DBUS_TYPE_UINT32, &count,		
							DBUS_TYPE_STRING, &poolname, 
							DBUS_TYPE_INVALID);
	
	/* send the message and flush the connection */
	if (!dbus_connection_send(conn, msg, &serial)) {
		log_error("notify trap Signal send error, Out Of Memory!\n"); 
		if(poolname != NULL){
			free(poolname);
		}
		return;
	}
	
	dbus_connection_flush(conn);
	
	/* free the message */
	dbus_message_unref(msg);
	if(poolname != NULL){
		free(poolname);
	}
	return;
}




void 
dhcp_dbus_profile_config_interface_save
(
	char **showif,
	char **ifcursor,
	struct dcli_ifname* ifhead,
	struct dcli_pool* pool_node,
	int *ifLen
)
{
	char ifname[25] = {0};
#if (defined _D_WCPSS_ || defined _D_CC_)
	int ret = 0;
#endif
	if(!showif || !ifcursor || !pool_node || !ifLen || !(*showif) || !(*ifcursor)){
		log_error("dhcp_dbus_profile_config_interface_save input argument %s, %s %s %s %s %s\n",\
				  showif ? "" : "showif is NULL", ifcursor ? "" : "ifcursor is NULL", pool_node ? "" : "pool_node is NULL", \
				  ifLen ? "" : "ifLen is NULL", *showif? "" : "*showif is NULL", *ifcursor ? "" : "*ifcursor is NULL");
		return ;
	}
	while (ifhead && ifhead->interface_name) {
		memset(ifname , 0, 25);
		memcpy(ifname, ifhead->interface_name,strlen(ifhead->interface_name));

		if (strncmp(ifname, "wlan", 4) && strncmp(ifname, "ebr", 3) && strncmp(ifname, "r", 1)) {
			/*if (1 == check_interfaces_ip_address(ifhead->interface_name)) */{
				*ifLen += sprintf(*ifcursor, "interface %s\n", ifname);
				*ifcursor = *showif + *ifLen;
				*ifLen += sprintf(*ifcursor, " ip pool %s\n", pool_node->poolname);
				*ifcursor = *showif + *ifLen;	
				*ifLen += sprintf(*ifcursor, " exit\n");
				*ifcursor = *showif + *ifLen;
			}
		}
		ifhead = ifhead->next;		
		
#if 0		
#if (defined _D_WCPSS_ || defined _D_CC_)
		if ((vrrid > 0)&&((!strncasecmp(ifname,"wlan",4))||(!strncasecmp(ifname,"radio",5))||(!strncasecmp(ifname,"ebr",3))||(!strncasecmp(ifname,"r",1)))){
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
#endif	
		*ifLen += sprintf(*ifcursor, "\n");
		*ifcursor = *showif + *ifLen;
		ifhead = ifhead->next;
		
#if (defined _D_WCPSS_ || defined _D_CC_)
        if (!strncasecmp(ifname,"wlan",4)){
			ret = parse_hansi_wlan_ebr_ifname(ifname+4,&vrrid, &wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"wlan%d",wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;		
				}		
			}else{
				ifhead = ifhead->next;
				continue;
			}

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
			}else{
				ifhead = ifhead->next;
				continue;
			}
		}
#endif
#ifdef _D_WCPSS_
		else if (!strncasecmp(ifname,"radio",5)){
			ret = parse_hansi_radio_ifname(ifname+5,&vrrid, &wtpid,&radioid,&wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"radio%d-%d.%d",wtpid,radioid,wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else{
				ifhead = ifhead->next;
				continue;
			}
		}
        else if (!strncasecmp(ifname,"r",1)){
			ret = parse_hansi_radio_ifname(ifname+1,&vrrid, &wtpid,&radioid,&wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"r%d-%d.%d",wtpid,radioid,wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else{
				ifhead = ifhead->next;
				continue;
			}
		}
#endif
		*ifLen += sprintf(*ifcursor, "interface %s\n", ifname);
		*ifcursor = *showif + *ifLen;
		*ifLen += sprintf(*ifcursor, " ip pool %s\n", pool_node->poolname);
		*ifcursor = *showif + *ifLen;	
		*ifLen += sprintf(*ifcursor, " exit\n");
		*ifcursor = *showif + *ifLen;

#if (defined _D_WCPSS_ || defined _D_CC_)
		if ((vrrid > 0)&&((!strncasecmp(ifname,"wlan",4))||(!strncasecmp(ifname,"radio",5))||(!strncasecmp(ifname,"ebr",3))||(!strncasecmp(ifname,"r",1)))){
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
#endif	
		*ifLen += sprintf(*ifcursor, "\n");
		*ifcursor = *showif + *ifLen;
		ifhead = ifhead->next;
#endif		
		}
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
dhcp_dbus_check_interface
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

	while (next_iface((struct iface_info *)&info, &err, &ifaces)) {		
		struct sockaddr_in *a = (struct sockaddr_in*)&info.addr;	
		struct sockaddr_in *b = (struct sockaddr_in*)&info.addrmask;
		memset(&ifaddr, 0, sizeof(struct iaddr));
		memset(&ifaddrmask, 0, sizeof(struct iaddr));
/*		
struct	 sockaddr_in   ina
ina.sin_addr.s_addr£½inet_addr("164£®112£®175£®124")
printf("%s"inet_ntoa(ina.sin_addr));
*/	
		ifaddr.len = DHCP_IPV4_ADDRESS_LEN;
		memcpy(ifaddr.iabuf, &a->sin_addr.s_addr, ifaddr.len);
		memcpy(ifaddrmask.iabuf, &b->sin_addr.s_addr, ifaddr.len);
		
		for (i = 0; i < ifaddr.len; i++) {			
//			log_debug("ifaddr->iabuf[%d] is %d \n", i, ifaddr.iabuf[i]);
//			log_debug("ipaddr->iabuf[%d] is %d \n", i, ipaddr->iabuf[i]);
			if (ipmask->iabuf[i] > ifaddrmask.iabuf[i]) {
				mask = ipmask->iabuf[i];
			}
			else {
				mask = ifaddrmask.iabuf[i];
			}
//log_debug("ifaddrmask.iabuf[%d] is %d\n", i, ifaddrmask.iabuf[i]);			
			if ((ifaddr.iabuf[i]&mask) != (ipaddr->iabuf[i]&ipmask->iabuf[i])) {			
				/*when break i not ++*/
				break;
			}
		}
//		log_debug("i is %d \n", i);
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
dhcp_dbus_pool_interface_state
(
	char *name,
	unsigned int *pool_state
)
{
	unsigned int ret = 0;	
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;

	ret = dhcp_dbus_find_subnet_by_ifname(name, &subnode);
	if (!ret) {
		poolnode = subnode->attribute_pool;
		*pool_state = poolnode->state;
		return 0;
	}

	return 1;
}

/**********************************************************************************
 *  dhcp_debus_is_same_subnet_exist
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
unsigned int dhcp_debus_is_same_subnet_exist
(
	struct dcli_subnet *check_sub,
	char *poolname
)
{
	struct dcli_pool* head = head_pool.next;
	unsigned int check_sub_ip = 0, check_sub_mask = 0;
	unsigned int headsub_ip = 0, headsub_mask = 0;
	unsigned int mask = 0;

	if ((NULL == check_sub) || (NULL == poolname)) {
		return 0;
	}
		
	dhcp_dbus_four_char_to_int(check_sub->ipaddr.iabuf, &check_sub_ip);
	dhcp_dbus_four_char_to_int(check_sub->mask.iabuf, &check_sub_mask);
	
    for (; head; head = head->next) {
		if (head->headsubnet && strcmp(head->poolname, poolname)) {	

			dhcp_dbus_four_char_to_int(head->headsubnet->ipaddr.iabuf, &headsub_ip);
			dhcp_dbus_four_char_to_int(head->headsubnet->mask.iabuf, &headsub_mask);

			mask = check_sub_mask & headsub_mask;
			log_debug("check ip subnet %x, mask %x <--> old ip subnet %x, mask %x, mask %x\n", 
					check_sub_ip, check_sub_mask, headsub_ip, headsub_mask, mask);

			if ((check_sub_ip & mask) == (headsub_ip & mask)) {
			    log_error("same subnet %s already exist\n", piaddr(head->headsubnet->ipaddr));
				return 1;
			}
		}
	}
	return 0;
}


/**********************************************************************************
 *  dhcp_dbus_whether_same_subnet
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
unsigned int dhcp_dbus_whether_same_subnet
(
	struct dcli_subnet *check_sub,
	struct dcli_subnet *pool_sub
)
{
	unsigned int check_ip = 0, check_mask = 0;
	unsigned int ip = 0, mask = 0;

	if ((NULL == check_sub) || (NULL == pool_sub)) {
		return 1;
	}
		
	dhcp_dbus_four_char_to_int(check_sub->ipaddr.iabuf, &check_ip);
	dhcp_dbus_four_char_to_int(check_sub->mask.iabuf, &check_mask);

	dhcp_dbus_four_char_to_int(pool_sub->ipaddr.iabuf, &ip);
	dhcp_dbus_four_char_to_int(pool_sub->mask.iabuf, &mask);

	if ((check_mask != mask) || ((check_ip & check_mask) != (ip & mask))) {
		log_info("pool subnet %s, check subnet %s\n", piaddr(pool_sub->ipaddr), piaddr(check_sub->ipaddr));
		return 0;
	} else {
		return 1;
	}
}


unsigned int
dhcp_dbus_check_pool_ip
(
	struct dcli_subnet *check_sub
)
{
	struct dcli_subnet *head_sub = NULL;
	struct dcli_pool* head = head_pool.next;
	unsigned int check_sub_low_ip =0, check_sub_high_ip = 0, check_sub_mask = 0;
	unsigned int headsub_low_ip =0, headsub_high_ip = 0, headsub_mask = 0;


	dhcp_dbus_four_char_to_int(check_sub->lowip.iabuf, &check_sub_low_ip);
	dhcp_dbus_four_char_to_int(check_sub->highip.iabuf, &check_sub_high_ip);
	dhcp_dbus_four_char_to_int(check_sub->mask.iabuf, &check_sub_mask);
	
    for (; head; head = head->next) {
		if (head->headsubnet) {			
			for (head_sub = head->headsubnet; head_sub; head_sub = head_sub->next) {

				dhcp_dbus_four_char_to_int(head_sub->lowip.iabuf, &headsub_low_ip);
				dhcp_dbus_four_char_to_int(head_sub->highip.iabuf, &headsub_high_ip);
				dhcp_dbus_four_char_to_int(head_sub->mask.iabuf, &headsub_mask);

				log_debug("check low ip  %#x, check high ip %#x <--> old low ip %#x, old high ip %#x\n", 
						check_sub_low_ip, check_sub_high_ip, headsub_low_ip, headsub_high_ip);

				/*check add range is exist*/
				if ((check_sub_low_ip >= headsub_low_ip && check_sub_low_ip <= headsub_high_ip) ||
					(check_sub_high_ip >= headsub_low_ip && check_sub_high_ip <= headsub_high_ip)) {
				    log_error("add range Duplicate old subnet\n");
					return 0;
				}			
				
			}			
		}
	}
	return 1;
}

int
dhcp_dbus_set_host
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
	unsigned int ret = 0, ifindex = 0, ipaddr = 0;
	
/*
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, hostipaddr, sizeof(struct iaddr));
	ret = find_subnet(&subnetop, addr, MDL);
	if (ret) {
	*/
	
	/*for set static arp*/
	ifindex = if_nametoindex(addhost->ifname);
	dhcp_dbus_four_char_to_int((unsigned char *)&(addhost->hostip.iabuf), &ipaddr);
	if (add) {	
		if (dhcp_server_static_arp_enable) {
			ret = dhcp_snp_netlink_do_ipneigh(0, ifindex, ipaddr, addhost->hostmac);
			if (ret) {
				log_error("set static arp fail for add static host\n");
				return ret;
			}
		}
		hosthwaddr = malloc(sizeof (struct hardware));
		memset(hosthwaddr, 0, sizeof (struct hardware));
		memcpy(&hosthwaddr->hbuf[1], addhost->hostmac, DHCP_MAC_ADDRESS_LEN);
		hosthwaddr->hlen = 7;
		hosthwaddr->hbuf[0] = HTYPE_ETHER;
		parse_host_declaration_test(hostname, hostipaddr, hosthwaddr, root_group);
	}
	else {
		log_debug("dhcp_dbus_set_host delete static host name is %s \n", hostname);
		if (dhcp_server_static_arp_enable) {
			ret = dhcp_snp_netlink_do_ipneigh(1, ifindex, ipaddr, addhost->hostmac);
			if (ret) {
				log_error("set static arp fail for del static host\n");
				return ret;
			}
		}
		delete_host_by_name(hostname);
	}

	if(hosthwaddr){
		free(hosthwaddr);
		hosthwaddr=NULL;
	}
	return 0;
}

/**********************************************************************************
 *  dhcp_dbus_set_option
 *
 *	DESCRIPTION:
 * 		set option
 *
 *	INPUT:
 *		addsubnet - struct subnet
 *		owned_option - option struct 
 *		del - del flag
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:		
 *		 
 *		 0 	->  ok
 *		 other	 ->  fail 
 **********************************************************************************/
 int 
dhcp_dbus_set_option
(
	struct dcli_subnet* addsubnet,
	struct dcli_option* owned_option,
	unsigned int del
)
{
	struct subnet* subnetop = NULL;
	struct group* addgroup = NULL;
	struct iaddr addr;
	int ret = 0, count = 1, i = 0, global = 0;
	struct dcli_pool* poolnode = NULL;
	unsigned int test = 0;
	char *maxtime = "max-lease-time";
	char *defaulttime = "default-lease-time";	
	char *domainname = "domain-name";
	char *routers = "routers";
	char *dnsip = "domain-name-servers";
	char *winsip = "netbios-name-servers";
	char *option43 = "vendor-encapsulated-options";
/*	char *option60 = "vendor-class-identifier"; */
	char *option138 = "capwap_ac_v4";
	int j = 0;
	unsigned int old_ipnum = 0, old_len = 0, tmp_ipnum = 0;;

	if (!owned_option) {
		log_error("error owned_option is null\n");
		return 1;
	}
	
	if (addsubnet) {
		poolnode = addsubnet->attribute_pool;
		addr.iabuf[0] = addsubnet->ipaddr.iabuf[0];
		addr.iabuf[1] = addsubnet->ipaddr.iabuf[1];
		addr.iabuf[2] = addsubnet->ipaddr.iabuf[2];
		addr.iabuf[3] = addsubnet->ipaddr.iabuf[3];
		addr.len = addsubnet->ipaddr.len;
		memset(&addr, 0, sizeof(struct iaddr));
		memcpy(&addr, &(addsubnet->ipaddr), sizeof(struct iaddr));
		pthread_mutex_lock(&DhcpDbusEventMutex);
		ret = find_subnet(&subnetop, addr, MDL);
		pthread_mutex_unlock(&DhcpDbusEventMutex);
		if (ret) {
			addgroup = subnetop->group;
			/*
			log_debug("find subnet success \n");	
			set_server_option(maxtime, testmaxtime, testlen, 0,subnetop->group);
			set_server_option(routername, (char *)(&addr), testlen, 1, subnetop->group);
			set_server_option(domainname, testvalue, testlen, 1, subnetop->group);	
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
		ret = set_server_option(defaulttime, (char *)(&(owned_option->defaulttime)), 0, 0, 0, addgroup);
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
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->maxtime) {
		ret = set_server_option(maxtime, (char *)(&(owned_option->maxtime)), 0, 0, del, addgroup);
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
	
	if (owned_option->domainname) {
		ret = set_server_option(domainname, owned_option->domainname, 0, 1, del, addgroup);			
		if (!ret) {
			if (global) {
				if (del) {
					free(global_server_option.domainname);
					free(owned_option->domainname);
					global_server_option.domainname = NULL;
				}
				else {					
					global_server_option.domainname = owned_option->domainname;
				}
			}
			else {
				if (del) {
					if(!(poolnode -> owned_option.domainname) || !(owned_option->domainname)){
						return DHCP_SERVER_RETURN_CODE_FAIL;
					}
					if(strcmp(poolnode->owned_option.domainname ,owned_option->domainname)){
						return DHCP_SERVER_RETURN_CODE_FAIL;
					}
					free(poolnode->owned_option.domainname);
					poolnode->owned_option.domainname = NULL;
				}
				else {					
					poolnode->owned_option.domainname = owned_option->domainname;
				}
			}
		}
		else {
			return 1;
		}
	}

	if (owned_option->option43.ipnum) {	
		
		if(NULL == poolnode){
			return DHCP_SERVER_RETURN_CODE_FAIL;
		}
		tmp_ipnum = poolnode->owned_option.option43.ipnum;
		old_ipnum = poolnode->owned_option.option43.ipnum;
		old_len = poolnode->owned_option.option43.len;
		poolnode->owned_option.option43.type = owned_option->option43.type;

		if (del) {	//delete option43
		
			if(0 == poolnode->owned_option.option43.type){	//delete all option43 ipv4 address
				
				poolnode->owned_option.option43.ipnum = 0;						
				if(set_server_option(option43, (char *)(&(poolnode->owned_option.option43)), 0, 1, del,addgroup)){
					poolnode->owned_option.option43.ipnum = old_ipnum;
					poolnode->owned_option.option43.len = old_len;
					return DHCP_SERVER_RETURN_CODE_FAIL;
				}
				if(poolnode->owned_option.option43.ip){
					free(poolnode->owned_option.option43.ip);
					poolnode->owned_option.option43.ip = NULL;
				}
				memset(&(poolnode->owned_option.option43), 0, sizeof(struct dcli_option43));
			     
			}else{
				for(j = 0; j < owned_option->option43.ipnum; j++){

				if( NULL == poolnode->owned_option.option43.ip){		// option43 is  null
					poolnode->owned_option.option43.ipnum =0;
					poolnode->owned_option.option43.len = 0;
					return DHCP_SERVER_RETURN_CODE_FAIL;

				}else if(1 == poolnode->owned_option.option43.ipnum && 	//option43 is one ipv4 address and delete it
						poolnode->owned_option.option43.ip[0] == owned_option->option43.ip[j]){						
					
					poolnode->owned_option.option43.ipnum = 0;						
					if(set_server_option(option43, (char *)(&(poolnode->owned_option.option43)), 0, 1, del,addgroup)){
						poolnode->owned_option.option43.ipnum = old_ipnum;
						poolnode->owned_option.option43.len = old_len;
						return DHCP_SERVER_RETURN_CODE_FAIL;
					}
					if(poolnode->owned_option.option43.ip){
						free(poolnode->owned_option.option43.ip);
						poolnode->owned_option.option43.ip = NULL;
					}
					memset(&(poolnode->owned_option.option43), 0, sizeof(struct dcli_option43));
				
				}else{		//delete one ipv4 address		
					
					for(i = 0 ; i < poolnode->owned_option.option43.ipnum; ++i){						
						if(poolnode->owned_option.option43.ip[i] == owned_option->option43.ip[j]){
							break;
						}
					}
					if(i == poolnode->owned_option.option43.ipnum){		// Don't find delete ip

						poolnode->owned_option.option43.ipnum = old_ipnum;
						poolnode->owned_option.option43.len = old_len;
						return DHCP_SERVER_RETURN_CODE_FAIL;

					}else{	//find and delete
					
						memmove(poolnode->owned_option.option43.ip + i , poolnode->owned_option.option43.ip+i+1, 
							(poolnode->owned_option.option43.ipnum - i - 1) * sizeof(int));
						poolnode->owned_option.option43.ip[poolnode->owned_option.option43.ipnum - 1] = 0;
						poolnode->owned_option.option43.ipnum -= 1;
						poolnode->owned_option.option43.len -= sizeof(int);
					}
				}					
			  }
		   }			
		}
		else {	
				/*for delet range after add range with option43*/
				if(poolnode->owned_option.option43.type){
					// add option43 
					tmp_ipnum += owned_option->option43.ipnum;
					
					/* max option43 ip list is 16 */
					if(tmp_ipnum > DHCP_MAX_OPTION_IP_LIST){				
						return DHCP_MAX_OPTION_LIST_WANNING;
					}
					/* first set option43 ipv4 address */
					if(NULL == poolnode->owned_option.option43.ip){
						
						poolnode->owned_option.option43.ip = malloc(DHCP_MAX_OPTION_IP_LIST *sizeof(int));
						if(NULL == poolnode->owned_option.option43.ip){
							if(poolnode->poolname){
								log_error("malloc fail for set server option43 fail in pool %s\n", poolnode->poolname);
							}						
							return DHCP_SERVER_RETURN_CODE_FAIL;
						}
						memset(poolnode->owned_option.option43.ip, 0, DHCP_MAX_OPTION_IP_LIST *sizeof(int));
						for(i = 0; i < owned_option->option43.ipnum; ++i){
							poolnode->owned_option.option43.ip[i] = owned_option->option43.ip[i];
						}
						
					}else{
						
						for(i = 0; i < owned_option->option43.ipnum; ++i){
							for(j = 0; j < poolnode->owned_option.option43.ipnum; ++j){
								if(poolnode->owned_option.option43.ip[j] == owned_option->option43.ip[i]){	
									return  DHCP_ALREADY_ADD_OPTION;
									
								}
							}
						}
						
						for(i = 0; i < owned_option->option43.ipnum; ++i){					
							poolnode->owned_option.option43.ip[old_ipnum + i] = owned_option->option43.ip[i];
						}
											
						
					}
					
					poolnode->owned_option.option43.len += owned_option->option43.len;
					poolnode->owned_option.option43.ipnum += owned_option->option43.ipnum;
				}			

			/*autelan option43 number*/
			poolnode->owned_option.option43.type = 0x11;
				
		}

		/*add option43*/
		if(poolnode->owned_option.option43.ipnum != 0 && poolnode->owned_option.option43.type!= 0){
			if(set_server_option(option43, (char *)(&(poolnode->owned_option.option43)), 0, 1, 0,addgroup)){
				poolnode->owned_option.option43.ipnum = old_ipnum;
				poolnode->owned_option.option43.len = old_len;
				return DHCP_SERVER_RETURN_CODE_FAIL;
			}		
		}
			
	}

	if (owned_option->routersip.len) {
		ret = set_server_option(routers, (char *)(&(owned_option->routersip)), 0, 1, del, addgroup);
		if (!ret) {
			if (global) {
				if (del) {
					memset(&(global_server_option.routersip), 0, sizeof(struct iaddr));
				}
				else {					
					global_server_option.routersip = owned_option->routersip;
					dhcp_dbus_four_char_to_int(global_server_option.routersip.iabuf, &test);
					test = 0;				
					dhcp_dbus_four_char_to_int(&(owned_option->routersip.iabuf[0]), &test);
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
		ret = set_server_option(winsip, (char *)(&(owned_option->winsip)), 0, 1, del, addgroup);
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
			ret = set_server_option(dnsip, (char *)(owned_option->dnsip), 0, 1, del, addgroup);
		}
		else {
			count |= ((owned_option->dnsnum)<<8)&0xffff;
			ret = set_server_option(dnsip, (char *)(owned_option->dnsip), count, 1, del, addgroup);
		}

		if (!ret) {
			if (global) {
				if (del) {
					free(global_server_option.dnsip);
					free(owned_option->dnsip);
					global_server_option.dnsip = NULL;
					global_server_option.dnsnum = 0;
				}
				else {					
					global_server_option.dnsip = owned_option->dnsip;
					global_server_option.dnsnum = owned_option->dnsnum;
				}
			}
			else {
				if (del) {
					free(poolnode->owned_option.dnsip);	
					poolnode->owned_option.dnsip = NULL;
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
	if (owned_option->option138[0].len) {
		if(NULL == poolnode){
			return DHCP_SERVER_RETURN_CODE_FAIL;
		}
		tmp_ipnum = poolnode->owned_option.option138_num;
		old_ipnum = poolnode->owned_option.option138_num;
		
		if (del) {		//delete option138
			if((0 == poolnode->owned_option.option138_num)||( 0 == owned_option->option138_num )||(poolnode->owned_option.option138_num == owned_option->option138_num)){	//option138 is null

				if(set_server_option(option138, (char *)(&(poolnode->owned_option.option138)), 0, 1, del, addgroup)){
					 
						return DHCP_SERVER_RETURN_CODE_FAIL;
				}
				memset(poolnode->owned_option.option138, 0, sizeof(struct iaddr) * 16);
				poolnode->owned_option.option138_num = 0;	

				return 0;
				
			}else if(1 == poolnode->owned_option.option138_num){
				for(j = 0; j < poolnode->owned_option.option138[i].len; ++j){
					if(poolnode->owned_option.option138[i].iabuf[j] != owned_option->option138[0].iabuf[j]) {
						break;
					}
				}
				if(j == poolnode->owned_option.option138[i].len){	//find delete option138
					 poolnode->owned_option.option138_num = 0;
					 if(set_server_option(option138, (char *)(&(poolnode->owned_option.option138)), 0, 1, del, addgroup)){
					 	poolnode->owned_option.option138_num = 1;
						return DHCP_SERVER_RETURN_CODE_FAIL;
					 }
				}
				memset(poolnode->owned_option.option138, 0, sizeof(struct iaddr) * 16);
				poolnode->owned_option.option138_num = 0;
				return 0;

			}else{
				log_info("1111\n");
				for(i = 0 ; i < poolnode->owned_option.option138_num; ++i){		
					for(j = 0; j < poolnode->owned_option.option138[i].len; ++j){
						if(poolnode->owned_option.option138[i].iabuf[j] != owned_option->option138[0].iabuf[j]) {
							break;
						}
					}
					if(j == poolnode->owned_option.option138[i].len){	//find delete option138
						break;
					}
				}
				if(i == poolnode->owned_option.option138_num){		// Don't find delete ip

					poolnode->owned_option.option138_num = old_ipnum;					
					return DHCP_SERVER_RETURN_CODE_FAIL;

				}else{	
				log_info("2222\n");
					memmove(poolnode->owned_option.option138 + i , poolnode->owned_option.option138+i+1, 
						(poolnode->owned_option.option138_num - i - 1) * sizeof(struct iaddr));					
					poolnode->owned_option.option138_num -= 1;					
				}
			}
			
		}
		else { //add option138

			tmp_ipnum += 1;			
			if(tmp_ipnum > DHCP_MAX_OPTION_IP_LIST){				
				return DHCP_MAX_OPTION_LIST_WANNING;
			}
			for(i = 0 ; i < poolnode->owned_option.option138_num; ++i){		
				for(j = 0; j < poolnode->owned_option.option138[i].len; ++j){
					if(poolnode->owned_option.option138[i].iabuf[j] != owned_option->option138[0].iabuf[j]) {
						break;
					}
				}
				if(j == poolnode->owned_option.option138[i].len){	//find delete option138
					break;
				}
			}
			if(i < poolnode->owned_option.option138_num){		// Don't find delete ip

				poolnode->owned_option.option138_num = old_ipnum;					
				return DHCP_ALREADY_ADD_OPTION;

			}
			memcpy(&(poolnode->owned_option.option138[old_ipnum]), &(owned_option->option138[0]), sizeof(struct iaddr));
			poolnode->owned_option.option138_num += 1;
		}


		if(1 == poolnode->owned_option.option138_num) {			
			ret = set_server_option(option138, (char *)(&(poolnode->owned_option.option138)), 0, 1, 0, addgroup);
		}
		else {
			count |= ((poolnode->owned_option.option138_num)<<8)&0xffff;			
			ret = set_server_option(option138, (char *)(&(poolnode->owned_option.option138)), count, 1, 0, addgroup);
		}	


		if(ret){
			poolnode->owned_option.option138_num = old_ipnum;
			return DHCP_SERVER_RETURN_CODE_FAIL;
		}
		
	}
	
	return 0;
}



unsigned int
dhcp_dbus_set_subnet
(
	struct dcli_subnet *addsubnet
)
{
	struct subnet *sub = NULL;
	struct iaddr addr;
	int ret = 0;
	struct subnet *old_subnet = NULL;
	memset(&sub_conf, 0, sizeof(struct subnet_config));
	
	sub_conf.net.iabuf[0] = addsubnet->ipaddr.iabuf[0];
	sub_conf.net.iabuf[1] = addsubnet->ipaddr.iabuf[1];
	sub_conf.net.iabuf[2] = addsubnet->ipaddr.iabuf[2];
	sub_conf.net.iabuf[3] = addsubnet->ipaddr.iabuf[3];
	sub_conf.net.len = addsubnet->ipaddr.len;
	
	sub_conf.netmask.iabuf[0] = addsubnet->mask.iabuf[0];	
	sub_conf.netmask.iabuf[1] = addsubnet->mask.iabuf[1];
	sub_conf.netmask.iabuf[2] = addsubnet->mask.iabuf[2];
	sub_conf.netmask.iabuf[3] = addsubnet->mask.iabuf[3];
	sub_conf.netmask.len = addsubnet->mask.len;
	
	sub_conf.range_conf.high.iabuf[0] = addsubnet->highip.iabuf[0];
	sub_conf.range_conf.high.iabuf[1] = addsubnet->highip.iabuf[1];
	sub_conf.range_conf.high.iabuf[2] = addsubnet->highip.iabuf[2];
	sub_conf.range_conf.high.iabuf[3] = addsubnet->highip.iabuf[3];
	sub_conf.range_conf.high.len = addsubnet->highip.len;
	
	sub_conf.range_conf.low.iabuf[0] = addsubnet->lowip.iabuf[0]; 
	sub_conf.range_conf.low.iabuf[1] = addsubnet->lowip.iabuf[1];
	sub_conf.range_conf.low.iabuf[2] = addsubnet->lowip.iabuf[2];
	sub_conf.range_conf.low.iabuf[3] = addsubnet->lowip.iabuf[3];
	sub_conf.range_conf.low.len = addsubnet->lowip.len;
	
	/* Make sure that high and low addresses are in this subnet. */
	if (!addr_eq((sub_conf.net), subnet_number((sub_conf.range_conf.low), (sub_conf.netmask)))) {
		log_error("bad range, low address  not in subnet  netmask \n");
		return 1;
	}

	if (!addr_eq((sub_conf.net), subnet_number((sub_conf.range_conf.high), (sub_conf.netmask)))) {
		log_error("bad range, high address  not in subnet  netmask \n");
		return 1;
	}
	pthread_mutex_lock(&DhcpDbusEventMutex);
	if (find_subnet (&old_subnet, sub_conf.net, MDL)) {
		parse_pool_statement_no_file(&(sub_conf.range_conf), old_subnet->group, SUBNET_DECL);
		
	}else{
		parse_statement_subnet_no_file(&sub_conf, root_group, ROOT_GROUP);	
	}
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(addsubnet->ipaddr), sizeof(struct iaddr));
	ret = find_subnet(&sub, addr, MDL);
	pthread_mutex_unlock(&DhcpDbusEventMutex);
	if (!ret|| !sub->shared_network) {
		log_error("can not find the subnet for the ip address\n");
		return 1;
	}
	
	//expire_all_pools_test(sub->shared_network); /*	for start up, deny this handle*/
	
//	char *name = "vlan100";
//	addsubnet->interface_name = name;
	/*

	for (i = 0; i < addsubnet->interface_num; i++) {
		discover_interfaces_test(1, addsubnet->interface_name[i]);for start up, deny this handle
	}
	if(!dhcp_server_start) {
		//dispatch();
	}
	else {
		dhcp_server_start = 1;
	}*/

	return 0;
}

int interface_reference_pool_node(char *ifname, struct dcli_pool *pool_node)
{
	struct interface_info *tmp = NULL;
	if ((NULL == ifname) || (NULL == pool_node)) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}
	for (tmp = interfaces; tmp; tmp = tmp->next) {
		if (0 == strcmp(tmp->name, ifname)) {
			break;
		}
	}
	if (NULL == tmp) {
		log_debug("%s : not found interface %s\n", __func__, ifname);
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}
	if (tmp->pool_ref) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	} else {
		tmp->pool_ref = pool_node;
	}
	log_debug("pool %s bind to interface %s\n", pool_node->poolname, ifname);
	return 0;
}
int interface_dereference_pool_node(char *ifname, struct dcli_pool *pool_node)
{
	struct interface_info *tmp = NULL;
	if ((NULL == ifname) || (NULL == pool_node)) {
		return 1;
	}
	for (tmp = interfaces; tmp; tmp = tmp->next) {
		if (0 == strcmp(tmp->name, ifname)) {
			break;
		}
	}
	if (NULL == tmp) {
		log_debug("%s : not found interface %s\n", __func__, ifname);
		return 1;
	}
	if (tmp->pool_ref == pool_node) {
		tmp->pool_ref = NULL;
	} else {
		return 1;
	}
	log_debug("pool %s unbind interface %s\n", pool_node->poolname, ifname);
	return 0;
}
unsigned int 
dhcp_dbus_interface_bind_pool
(
	struct dcli_pool *poolnode,
	char* interface_name
)
{
	struct dcli_ifname* ifnode = NULL;
	struct dcli_subnet* subnode = NULL;
	
	if (!poolnode||!interface_name ) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}

	if (poolnode->headsubnet) {
		subnode = poolnode->headsubnet;
	}
	else {
		log_error("pool has no subnet\n");
		return DHCP_POOL_SUBNET_NULL;
	}
	poolnode->refcount = 1;
	
	while (subnode){

#if 0	//Remove restriction of interface ip and dhcp pool 's ip not  in the same network 

		ret = dhcp_dbus_check_interface(&subnode->lowip, &subnode->mask, &ifname, &ifnum);
		if (!ret) {
			for(i = 0; i < ifnum; i++) {
				log_debug("interface %s bind pool , ifname[i] %s\n", interface_name, ifname[i]);
				ret = dhcp_dbus_find_subnet_by_ifname(ifname[i], &subNode);
				/*one subnet one interface, if want to support more interface,
					 delete || subnode->interface_num*/
				if (!ret || subnode->interface_num) {
					log_debug("interface %s have this subnet before\n", ifname[i]);
					continue;
				}
				
				log_info(" interface name is %s , ifname is %s\n", interface_name, ifname[i]);
		
				if (!strcmp(interface_name, ifname[i])) {
					ifnode = malloc(sizeof(struct dcli_ifname));
					memset(ifnode, 0, sizeof(struct dcli_ifname));
					ifnode->interface_name = malloc(strlen(ifname[i]) + 1);
					memset(ifnode->interface_name, 0, (strlen(ifname[i]) + 1));
					memcpy(ifnode->interface_name, ifname[i], strlen(ifname[i]));
					if (subnode->ifhead) {
						ifnode->next = subnode->ifhead;
						subnode->ifhead = ifnode;
					}
					else {
						subnode->ifhead = ifnode;
					}
					
					subnode->interface_num++;
					poolnode->refcount++;

#endif
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
					
		subnode = subnode->next;
	}
	log_debug("dhcp dbus interface bind pool, subnet <%s>\n", piaddr(poolnode->headsubnet->ipaddr));
	/* 
	discover_interfaces_sharenetwork_test(1, interface_name, poolnode->headsubnet);
	*/
	discover_interfaces_test(DISCOVER_SERVER, interface_name);
	return 0;
}

unsigned int 
dhcp_dbus_interface_unbind_pool_subnet
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
			ifnode->interface_name = NULL;
			free(ifnode);
			ifnode = NULL;

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
dhcp_dbus_interface_unbind_pool
(
	struct dcli_pool* poolNode,
	char* interface_name
)
{
	struct dcli_ifname *ifprev = NULL, *ifnode = NULL;
	struct dcli_subnet *tmpsub = NULL;
	
	if (!poolNode||!interface_name ) {
		return 1;
	}

	for(tmpsub = poolNode->headsubnet; tmpsub; tmpsub = tmpsub->next){
		ifnode = tmpsub->ifhead;
		while (ifnode) {
			if (!strcmp(interface_name, ifnode->interface_name)) {
				if (ifprev) {
					ifprev->next = ifnode->next;
				}
				else {
					tmpsub->ifhead = ifnode->next;
				}
				free(ifnode->interface_name);
				ifnode->interface_name = NULL;
				free(ifnode);
				ifnode = NULL;

				tmpsub->interface_num--;
				log_debug("pool %s refcount %d\n", poolNode->poolname, poolNode->refcount);
				test_for_del_subnet_interface(interface_name);

				break;
			}

			ifprev = ifnode;
			ifnode = ifnode->next;
		}
	}	
	poolNode->refcount = 0;
	log_debug("pool %s refcount %d\n", poolNode->poolname, poolNode->refcount);

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
	memcpy(type, str, 2);
	memcpy(len, &(str[2]), 2);
	size = strtoul((char *)len, &endstr, 16);
	if (((strlen(str) - 4)%8) || 
		((strlen(str) - 4 ) != size*2)){
		return 1;
	}
	else {
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
		for(i = 0; i < num; i++ ) {
			memset(ip, 0, 9);
			memcpy(ip, &(str[4 + i*8]), 8);
			option->ip[i] = strtoul((char *)ip, &endstr, 16);
			if (dhcp_dbus_check_ipaddr_uint(option->ip[i])) {
/* in order to support option43 express date */
#if 0 	
				return 1;
#endif
			}
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
/*
	backup->next = failover_backup_list_head.next;
	failover_backup_list_head.next = backup;
*/
	return 0;
}

unsigned int 
del_interface_failover_to_backup_list
(
	struct dcli_subnet *subnode
)
{
	struct failover_backup_list *prev = NULL, *backup = NULL;
	int i;
	
	backup =  failover_backup_list_head.next;

	while ((backup && 
			(subnode->ifhead) && 
			(subnode->ifhead->interface_name))) {
			/*one subnet one interface*/
		for(i = 0; i < backup->interface_num; i++){
			if (!strcmp(backup->dhcp_interface[i].if_name, subnode->ifhead->interface_name)) {
			if (!prev) {
				failover_backup_list_head.next = backup->next;
			}
			else {
				prev->next = backup->next;
			}

			free(backup);
			return 0;
			}	
		}
		
		prev = backup;
		backup = backup->next;
	}

	return 1;
}

unsigned int 
find_interface_failover_to_backup_list
(
	struct dcli_subnet *subnode
)
{
	struct failover_backup_list *prev = NULL, *backup = NULL;
	int i;
	
	backup =  failover_backup_list_head.next;

	while ((backup && 
			(subnode->ifhead) && 
			(subnode->ifhead->interface_name))) {
			/*one subnet one interface*/
		for(i = 0; i < backup->interface_num; i++){
			if (!strcmp(backup->dhcp_interface[i].if_name, subnode->ifhead->interface_name)) {
			return 0;
			}	
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

unsigned int dhcp_dbus_change_state
(
	struct dcli_subnet *subnode
)
{
	
	if (!subnode) {
		return 1;
	}

	if(dhcp_failover_change_state(subnode->owned_failover.name, subnode->owned_failover.primary)){
		return 1;
	}

	return 0;
}

unsigned int
dhcp_dbus_set_sub_failover_add_interface
(
	struct dcli_subnet *subnode
)
{
	struct dcli_failover *failnode = NULL;
	struct subnet *sub = NULL;
	struct iaddr addr;
	unsigned int ret = 0;
	
	if (!subnode) {
		return 1;
	}
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(subnode->ipaddr), sizeof(struct iaddr));
	
	ret = find_subnet(&sub, addr, MDL);
	if (!ret|| !sub->shared_network) {
		log_error("can not find the subnet for the ip address\n");
		return 1;
	}
	
	failnode = &(subnode->owned_failover);
	//ret = parse_failover_peer_no_file(failnode, sub->group, SUBNET_DECL);
	expire_pool_for_failover(failnode, sub->shared_network);	
	log_debug_failover(DEBUG_TYPE_CONNECT,"for dhcp_failover_startup_one ret is %d \n", ret);
	if (!ret) {
		//dhcp_failover_startup_one(failnode);
	}	
//	dhcp_failover_startup_one(failnode);
	failnode->attribute_sub = subnode;
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(failnode->parteraddr), sizeof(struct iaddr));
	icmp_echorequest(&addr);
	
	return 0;
}

unsigned int
dhcp_dbus_set_sub_failover
(
	struct dcli_subnet *subnode
)
{
	struct dcli_failover *failnode = NULL;
	struct subnet *sub = NULL;
	struct iaddr addr;
	unsigned int ret = 0;
	
	if (!subnode) {
		return 1;
	}
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(subnode->ipaddr), sizeof(struct iaddr));
	
	ret = find_subnet(&sub, addr, MDL);
	if (!ret|| !sub->shared_network) {
		log_error("can not find the subnet for the ip address\n");
		return 1;
	}
	
	failnode = &(subnode->owned_failover);
	ret = parse_failover_peer_no_file(failnode, sub->group, SUBNET_DECL);
	expire_pool_for_failover(failnode, sub->shared_network);	
	log_debug_failover(DEBUG_TYPE_CONNECT,"for dhcp_failover_startup_one ret is %d \n", ret);
	if (!ret) {
		dhcp_failover_startup_one(failnode);
	}	
//	dhcp_failover_startup_one(failnode);
	failnode->attribute_sub = subnode;
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(failnode->parteraddr), sizeof(struct iaddr));
	icmp_echorequest(&addr);
	
	return 0;
}

unsigned int 
dhcp_dbus_change_sub_failover
(
	struct dcli_subnet *subnode
)
{
	struct dcli_failover *failnode = NULL;
	struct subnet *sub = NULL;
	struct iaddr addr;
	unsigned int ret = 0;
	
	if (!subnode) {
		return 1;
	}
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(subnode->ipaddr), sizeof(struct iaddr));
	
	ret = find_subnet(&sub, addr, MDL);
	if (!ret|| !sub->shared_network) {
		log_error("can not find the subnet for the ip address\n");
		return 1;
	}
	
	failnode = &(subnode->owned_failover);		
	ret = dhcp_dbus_change_state(subnode);
	expire_pool_for_failover(failnode, sub->shared_network);	
	log_debug_failover(DEBUG_TYPE_CONNECT,"in change_sub_failover change state ret is %d \n", ret);
	if (!ret) {
		dhcp_failover_startup_one(failnode);
	}	
	failnode->attribute_sub = subnode;
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(failnode->parteraddr), sizeof(struct iaddr));
	icmp_echorequest(&addr);
	
	return 0;
}

void undo_expire_pool_for_failover 
(
	struct dcli_failover* failover_peer,
	struct shared_network* share
){
	struct shared_network *s;
	struct pool *p;
	
	s = share;
	/*need for p is the only pool of the shared_network*/
    for (p = s->pools; p; p = p->next) {
		log_debug("undo_expire_pool_for_failover shared network run 1.0 %s \n", s->name);
		/* Inherit the failover peer from the shared network. */
		if (p->shared_network->failover_peer) {
		    dhcp_failover_state_reference(&p->failover_peer, 
			     p->shared_network->failover_peer, MDL);
		}
		if (p->failover_peer) {
			dhcp_failover_state_dereference (&p->failover_peer, MDL);
		}
	log_debug("undo_expire_pool_for_failover shared network run 1.2 %s \n", s->name);
    }	
}


unsigned int
dhcp_dbus_del_sub_failover
(
	struct dcli_subnet *subnode
)
{
	struct dcli_failover *failnode = NULL;
	struct iaddr addr;
	struct subnet *sub = NULL;
	unsigned int ret = 0;
	
	if (!subnode) {
		return 1;
	}
	failnode = &(subnode->owned_failover);
	dhcp_failover_state_destroy_by_name(failnode);
	del_interface_failover_to_backup_list(subnode);		
	
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(subnode->ipaddr), sizeof(struct iaddr));
	ret = find_subnet(&sub, addr, MDL);
	if (!ret|| !sub->shared_network) {
		log_error("can not find the subnet for the ip address\n");
		return 1;
	}
	undo_expire_pool_for_failover(failnode, sub->shared_network);
	memset(&addr, 0, sizeof(struct iaddr));
	memcpy(&addr, &(failnode->parteraddr), sizeof(struct iaddr));
	icmp_echorequest(&addr);

	memset(failnode, 0, sizeof(struct dcli_failover));
	
	return 0;
}

unsigned int
dhcp_dbus_find_pool_by_name
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

unsigned int
dhcp_dbus_find_static_host_by_addr
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
dhcp_dbus_find_static_host_by_addr_all
(
	struct iaddr* addr,
	char* mac,
	char* ifname,
	struct dcli_host** stahost
)
{
	struct dcli_host* head = head_host.next;

    for (; head; head = head->next) {
		if (!memcmp(&(head->hostip), addr, sizeof(struct iaddr))
			&& !memcmp(head->hostmac, mac, DHCP_MAC_ADDRESS_LEN)
			&& !strcmp(head->ifname, ifname)) {
			*stahost = head;
			return 0;
		}
	}

	return 1;
}

unsigned int
dhcp_dbus_delete_static_host_by_addr
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
			if(tmphost->hostname){
				free(tmphost->hostname);
				tmphost->hostname=NULL;
			}
			if(tmphost->ifname){
				free(tmphost->ifname);
				tmphost->ifname=NULL;
			}
			free(tmphost);
			return 0;
		}

		prevhost = tmphost;
		tmphost = tmphost->next;
	}

	return 1;
}


/**********************************************************************************
 *  dhcp_dbus_find_pool_by_index
 *
 *	DESCRIPTION:
 * 		get pool
 *
 *	INPUT:
 *		
 *		index
 *		
 *	
 *	OUTPUT:
 *		poolnode 
 *
 * 	RETURN:		
 *		 
 *		 0 	->  find
 *		 other  ->  fail 
 **********************************************************************************/

unsigned int
dhcp_dbus_find_pool_by_index
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
dhcp_dbus_create_pool_by_name
(
	char* name,
	struct dcli_pool** poolnode
)
{
	struct dcli_pool* head = NULL, *temppool = NULL;
	unsigned int ret = 0;
	if (!name || (count_pool >= DHCP_MAX_POOL_NUM)) {
		if(name){
			ret = dhcp_dbus_find_pool_by_name(name, &temppool);
			if(!ret){
				return 2;
			}
		}
		log_error("create ip pool parameter null, pool count %d\n", count_pool);		
		return 1;
	}

	/* not find return 1 */
	ret = dhcp_dbus_find_pool_by_name(name, &temppool);
	if (ret) {
		head = malloc(sizeof(struct dcli_pool));
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
dhcp_dbus_delete_pool_by_name
(
	char* name
)
{
	struct dcli_pool *prevpool = NULL, *temppool = NULL;
	struct dcli_subnet *tempsub = NULL;
	unsigned int ret = 0;
	
	if (!name) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}

	ret = dhcp_dbus_find_pool_by_name(name, &temppool);
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
					if (NULL==temppool->poolname) {
						return DHCP_SERVER_RETURN_CODE_FAIL;
					}else{
						free(temppool->poolname);
					}
					if (temppool->owned_option.option60) {
						free(temppool->owned_option.option60);
					}
					free(temppool);
					return DHCP_SERVER_RETURN_CODE_SUCCESS;
				}
				else {
					log_debug("binded by interface can not del\n");
					return DHCP_UNBIND_BY_INTERFACE;
				}
			}
			prevpool = temppool;
			temppool = temppool->next;
		}
	}
	else {
		log_debug("can not find the pool by name\n");
		return DHCP_NOT_FOUND_POOL;
	}
	
	return DHCP_SERVER_RETURN_CODE_SUCCESS;
}


unsigned int
dhcp_dbus_create_static_host_by_name
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
dhcp_dbus_find_subnet_by_ifname
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
dhcp_dbus_delete_subnet_by_subip
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

			if(1 == poolnode->count_sub) {
				dhcp_dbus_set_option(tmpsub, &(poolnode->owned_option), 1);
			}
			
			if (!prevsub) {
				poolnode->headsubnet = tmpsub->next;
			}
			else {
				prevsub->next = tmpsub->next;
			}
			test_for_del_subnet_pool(tmpsub);	
			poolnode->count_sub--;	
			if(tmpsub){
				free(tmpsub);
			}	
			return 0;
		}

		prevsub = tmpsub;
		tmpsub = tmpsub->next;
	}

	return 1;
}

int 
dhcp_releas_lease
(
	unsigned int ip_Addr
)
{
	struct iaddr cip;
	int ret = 0;
	
	cip.iabuf[0] = ip_Addr>>24 & 0xff;
	cip.iabuf[1] = ip_Addr>>16 & 0xff;
	cip.iabuf[2] = ip_Addr>>8 & 0xff;
	cip.iabuf[3] = ip_Addr & 0xff;
	cip.len = 4;

	ret = release_lease_by_addr(cip);
		
	return ret;
}

void 
dhcp_read_netlink
(
	void* ptr,
	int totlemsglen
)
{
	struct  ifinfomsg *rtEntry = NULL;
	struct nlmsghdr *nlp = (struct nlmsghdr *)ptr;
	int payloadoff = 0, payloadlen = 0, ret = 0;
	struct rtattr *rtattrptr = NULL;
	unsigned char *ifname = NULL, *ifaddr = NULL;
	unsigned int ifi_flags = 0, ifi_index = ~0UI;;
	struct dcli_subnet *subNode = NULL;
	struct neigh_tbl_info neighInfo;
	struct ndmsg *neighEntry = NULL;
	
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
							ifname = (unsigned char *)RTA_DATA(rtattrptr);
							ret = dhcp_dbus_find_subnet_by_ifname((char *)ifname, &subNode);
							if (!ret) {
							log_info("interface %s, then no ip pool %s", ifname, subNode->attribute_pool->poolname);
								dhcp_dbus_interface_unbind_pool(subNode->attribute_pool, (char *)ifname);
							}
							else {
								log_debug("no subnet for this interface %s", ifname);
							}
							break;
							
							/*
							ifname = (unsigned char*)RTA_DATA(rtattrptr);
							ret = dhcp_dbus_find_pool_by_name(ifname, &poolNode);
							if (!ret) {
								log_debug("no ip pool because no interface %s", ifname);
									dhcp_dbus_interface_unbind_pool(poolNode, ifname);
									poolNode->refcount = 0;
							}
							else {
								log_debug("no subnet for this interface %s", ifname);
							}
							break;
							*/

						default:
							/*npd_syslog_dbg("other value ignore %d\n",nlp->nlmsg_type);*/
	                        break;
					}
				}
			break;
			
			case RTM_NEWNEIGH:
			case RTM_DELNEIGH:
				neighEntry = (struct ndmsg *) NLMSG_DATA(nlp);
				payloadlen = RTM_PAYLOAD(nlp);
				memset(&neighInfo,0,sizeof(struct neigh_tbl_info));
				struct rtattr *rtattrptr = NULL;
				struct in_addr *dst = NULL;
				unsigned char *tmp = NULL;
				
				neighInfo.ifIndex = neighEntry->ndm_ifindex;
				neighInfo.state = neighEntry->ndm_state;
				
				rtattrptr = (struct rtattr *) RTM_RTA(neighEntry);
				
				for(;RTA_OK(rtattrptr, payloadlen);rtattrptr=RTA_NEXT(rtattrptr,payloadlen))
				{
					switch(rtattrptr->rta_type) 
					{
						/* ip address */
						case NDA_DST:
							dst = (struct in_addr*)(RTA_DATA(rtattrptr));
							neighInfo.ipAddr = dst->s_addr;
							#if 0
							log_debug("neigh ip address %d.%d.%d.%d",(neighInfo.ipAddr>>24)&0xFF, \
								(neighInfo.ipAddr>>16)&0xFF,(neighInfo.ipAddr>>8)&0xFF,neighInfo.ipAddr&0xFF);
							#endif
							break;
						/* MAC address */
						case NDA_LLADDR:
							tmp = (unsigned char*)RTA_DATA(rtattrptr);
							memcpy(neighInfo.mac,tmp,6);
							break;
						default:
							break;
					}
				}

				if(NUD_FAILED == neighInfo.state) {
					/* TODO: call static arp entry*/
					/*
					 * When state NUD_FAILED, no mac address provided by kernel message
					 */
					#if 0
					log_debug("riceive NUD_FAILED netlink msg %#0x %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d in STALE %x\n", \
													neighInfo.ifIndex,neighInfo.mac[0],neighInfo.mac[1],neighInfo.mac[2],neighInfo.mac[3], \
													neighInfo.mac[4],neighInfo.mac[5],(neighInfo.ipAddr >> 24)&0xFF, \
													(neighInfo.ipAddr >> 16)&0xFF,(neighInfo.ipAddr >> 8)&0xFF,neighInfo.ipAddr&0xFF, neighInfo.state);
					#endif
					if (!memcmp(neighInfo.mac, null_mac_addr, DHCP_MAC_ADDRESS_LEN)) {
						continue;
					}
					ret = dhcp_releas_lease(neighInfo.ipAddr);
					if (ret) {
						log_debug("dhcp_releas_lease fail \n");
					}
				}
				else if(NUD_STALE == neighInfo.state){
					/* TODO: call arp snooping send ARP_REQUEST
					if(0 == neighInfo.ipAddr){
						break;
					}
					pthread_mutex_lock(&arpHashMutex);
					arpInfo = npd_arp_snooping_find_item_bymacip(neighInfo.ifIndex,neighInfo.mac,neighInfo.ipAddr);
					if(NULL == arpInfo) {
						syslog_ax_arpsnooping_err("no arp item for %#0x %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d in STALE\n", \
							neighInfo.ifIndex,neighInfo.mac[0],neighInfo.mac[1],neighInfo.mac[2],neighInfo.mac[3], \
							neighInfo.mac[4],neighInfo.mac[5],(neighInfo.ipAddr >> 24)&0xFF, \
							(neighInfo.ipAddr >> 16)&0xFF,(neighInfo.ipAddr >> 8)&0xFF,neighInfo.ipAddr&0xFF);
						pthread_mutex_unlock(&arpHashMutex);
						break;
					}
					else {
						npd_arp_snooping_solicit_send(arpInfo);
						pthread_mutex_unlock(&arpHashMutex);
					}*/
				}
				else {
				}
				break;
				
			  case  RTM_DELADDR:
				{
					#if 0
					struct ifaddrmsg *ifa = NULL;
					struct rtattr *tb[IFA_MAX + 1];
					static char buf[4096] = {0};
					unsigned char ifnameArray[16] = {0};
					unsigned int ipaddr = 0;
						
					memset (tb, 0, sizeof tb);
					ifa = NLMSG_DATA (nlp);
					int len = nlp->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
					if(len < 0){
						continue;
					}
					struct rtattr *rta = IFA_RTA (ifa);
					while (RTA_OK (rta, len))
					{
					if (rta->rta_type <= IFA_MAX){
					tb[rta->rta_type] = rta;
					}
					rta = RTA_NEXT (rta, len);
					}
					ifname = if_indextoname(ifa->ifa_index,ifnameArray);
					if (tb[IFA_LOCAL]){
						memcpy(&ipaddr,RTA_DATA (tb[IFA_LOCAL]),sizeof(unsigned int));
					}							

					if(ifname){
						ret = dhcp_dbus_find_subnet_by_ifname(ifname, &subNode);
						if (!ret) {
							log_info("interface %s delete address %08X, then no ip pool %s", 
								ifname, ipaddr, subNode->attribute_pool->poolname);
							dhcp_dbus_interface_unbind_pool(subNode->attribute_pool, ifname);
						}
					}
					break;
					#endif
				}
			
			default:
			break;
		}		    
	}
}

void 
dhcp_syn_kernelRT
(
	int sockfd
)
{
	int msglen=0;
	static char buf[4096] = {0};
	
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
		dhcp_read_netlink(nlp, msglen);
	}

}

void *
dhcp_receive_netlink
(
	void *arg
)
{
	int fd = -1;
	struct sockaddr_nl la;
	
	/* tell my thread id*/
	dhcp_tell_whoami("netlinkDhcp", 0);
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		log_error("create socket error when sync fib to asic\n");
		return 0;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	/*
	#ifdef CPU_ARM_XCAT
	la.nl_pid += g_npd_thread_number;
	#endif
	*/
	la.nl_groups = RTMGRP_IPV4_IFADDR|RTMGRP_IPV4_ROUTE |RTMGRP_LINK| RTMGRP_NOTIFY | RTMGRP_NEIGH|RTM_NEWNEIGH|RTM_DELNEIGH;

	if(bind(fd, (struct sockaddr*) &la, sizeof(la))) {
		log_error("bind socket error when sync fib to asic\n");
	}
	
	dhcp_syn_kernelRT(fd);

	if(fd){
		close(fd);
	}
	return 0;
}

/**********************************************************************************
 *  dhcp_dbus_get_slotid_by_vrrpid
 *
 *	DESCRIPTION:
 * 		get slot id by vrrp id
 *
 *	INPUT:
 *		vrrp_id -> vrrp id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 -> failed
 *		slot_id -> slot id
 *		
 **********************************************************************************/ 
int dhcp_dbus_get_slotid_by_vrrpid(int vrrp_id)
{
	FILE *fp = NULL;
	char cmd[128];
	int slot_id = 0;

	/*
	ls /var/run/hmd/hmd1-*-vrrpid.pid | cut -d'-' -f2
	*/
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ls /var/run/hmd/hmd1-*-%d.pid | cut -d\'-\' -f2 2>/dev/null", vrrp_id);
	log_debug("get slot id by vrrp id cmd: %s\n", cmd);
	if (NULL == (fp = popen(cmd, "r"))) {
		return -1;
	}

	if(fscanf(fp, "%d", &slot_id)){
		log_debug("The integer read success!\n");
	}else{
		log_debug("The integer read failed!\n");
	}
	log_debug("hmd1-%d-%d: slot id is %d\n", slot_id, vrrp_id, slot_id);
	pclose(fp);	
	
	return slot_id;
}


DBusMessage* 
dhcp_dbus_hmd_running_check(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	int ret = 0;
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
		
	return reply;
}



DBusMessage* 
dhcp_dbus_restart_load_hansi_cfg(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*		reply = NULL;	  
	DBusMessageIter 	iter;
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(DHCP_SAVE_CFG_MEM);
	
	if(!strShow) {
		log_debug("alloc memory fail when restart dhcp\n");
		return NULL;
	}
	memset(strShow, 0, DHCP_SAVE_CFG_MEM);

	dbus_error_init(&err);

	dhcp_dbus_profile_show_running(strShow);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	
	return reply;
}



void dhcp_dbus_profile_show_running(char *showStr)
{
	struct dcli_pool* pool_node = NULL;
	struct dcli_ifname* ifhead = NULL;
	char *cursor = NULL;
	char ifname[32];
	int value1 = 0, value2 = 0, value3 = 0, value4 = 0, value5 = 0;
	int vrrp = 0, slot = 0, ifaceid = 0;
	int totalLen = 0;
	int ret = 0;

	if (NULL == showStr) {
		return;
	}
	cursor = showStr;

	/*ip pool conifgure*/
	pool_node = head_pool.next;
	for (; pool_node; pool_node = pool_node->next) {	
		if (pool_node->headsubnet && pool_node->headsubnet->ifhead) {
			ifhead = pool_node->headsubnet->ifhead;

			if (1 != check_interfaces_ip_address(ifhead->interface_name)) {
					continue;
			}
			
			memset(ifname, 0, sizeof(ifname));
			memcpy(ifname, ifhead->interface_name, strlen(ifhead->interface_name));
			log_debug("dhcp restart load hansi config: interface %s\n", ifname);

			/* only wlan ebr radio */
			if ((strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "r", 1))) {
				continue;
			}

			#if 1
			if (0 == strncmp(ifname, "wlanl", 5)) {
				ret = sscanf(ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret)) {		/*  wlanl+slotid+instID-wlanid */
					slot = value1;
					vrrp = value2;
					ifaceid = value3;		
					if ((slot > -1) && vrrp) {	/* InstID */
						totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface wlan%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
				ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
				if (3 == ret) {	/* wlan+slotid-InstID-wlanid */
					slot = value1;
					vrrp = value2;				
					ifaceid = value3;
					if (vrrp) { 	/* InstID */
						totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface wlan%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			#else
			if (0 == strncmp(ifname, "wlan", 4)) {
				ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
				if ((2 == ret)) {		/*  wlan+instID-wlanid */
					vrrp = value1;
					ifaceid = value2;					
					slot = dhcp_dbus_get_slotid_by_vrrpid(vrrp);
					if ((slot > -1) && vrrp) {	/* InstID */
						totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				} else if (3 == ret) {	/* wlan+slotid-InstID-wlanid */
					slot = value1;
					vrrp = value2;				
					ifaceid = value3;
					if (vrrp) { 	/* InstID */
						totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface wlan%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			#endif

			#if 1
			else if (0 == strncmp(ifname, "ebrl", 4)) {
				ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret)) {	/* ebrl+slotid+InstID-ebrid */
					slot = value1;					
					vrrp = value2;
					ifaceid = value3;
					if ((slot > -1) && vrrp) {	/* InstID */
						totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface ebr%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
				ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
 				if (3 == ret) {	/* ebr+slotid-InstID-wlanid */
					slot = value1;
					vrrp = value2;
					ifaceid = value3;					
					if (vrrp) {
						totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface ebr%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			#else
			else if (0 == strncmp(ifname, "ebr", 3)) {
				ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
				if ((2 == ret)) {	/* ebr+InstID-ebrid */
					vrrp = value1;
					ifaceid = value2;	
					slot = dhcp_dbus_get_slotid_by_vrrpid(vrrp);
					if ((slot > -1) && vrrp) {	/* InstID */
						totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				else if ((3 == ret)) {	/* ebr+slotid-InstID-wlanid */
					slot = value1;
					vrrp = value2;
					ifaceid = value3;					
					if (vrrp) {
						totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
				}
				totalLen += sprintf(cursor, " interface ebr%d\n", ifaceid);
				cursor = showStr + totalLen;				
			}
			#endif
			
			else if (0 == strncmp(ifname, "r", 1)) { /* r1-1-1-0.1 */
				ret = sscanf(ifname, "r%d-%d-%d-%d.%d", &value1, &value2, &value3, &value4, &value5);
				if ((4 == ret)) { /* r+InstID-wtpid-radiolocalid.wlanid */
					vrrp = value1;
					slot = dhcp_dbus_get_slotid_by_vrrpid(vrrp);
					if ((slot > -1) && vrrp) {	/* InstID */
						totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}
					totalLen += sprintf(cursor, " interface r%d-%d.%d\n", value2, value3, value4);
					cursor = showStr + totalLen;
				} 
				else if ((5 == ret)) {	/* r+slotid-InstID-wtpid-radiolocalid.wlanid */
					slot = value1;
					vrrp = value2;
					if (vrrp) {
						totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
						cursor = showStr + totalLen;
					}					
				}
				totalLen += sprintf(cursor, " interface r%d-%d.%d\n", value3, value4, value5);
				cursor = showStr + totalLen;				
			}

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

	log_debug("restart dhcp load  hansi config: %s\n", showStr);
	return ;
}


void 
dhcp_dbus_profile_config_save
(
	char* showStr
)
{
	char *cursor = NULL, *ifcursor = NULL;
	char *showif = NULL;
	int i = 0, totalLen = 0, ifLen = 0;
	int subnode_null_flag = 0;
	struct dcli_pool* pool_node = NULL;
	struct dcli_subnet* sub_node = NULL;
	struct dcli_host* host_node = NULL;	
	struct dcli_option* option_node = NULL;
	struct dcli_ifname* ifhead = NULL;
	struct pppoe_snp_cfg *pppoe_node = NULL;

	char option_list[16] = {0};
	unsigned char ip_list[16] = {0};
	if(!showStr){
	}
	
	showif = (char*)malloc(512*1024);
	if(NULL == showif){
		log_error("malloc fail in dhcp_dbus_profile_config_save funtion on %s file\n", __FILE__);
		return ;
	}
	memset(showif, 0, 512*1024);
	
	ifcursor = showif;
	cursor = showStr;

	/*dhcp ping check enable*/
	if (!dhcp_ping_check_enable) {
		totalLen += sprintf(cursor, "ip dhcp server ping-check disable\n");
		cursor = showStr + totalLen;
	}

	/*dhcp lease expiry*/
	if (DEFAULT_LEASE_EXPIRY_TIME != dhcp_lease_expiry_time) {
		totalLen += sprintf(cursor, "ip dhcp server lease-expiry %d\n", dhcp_lease_expiry_time);
		cursor = showStr + totalLen;
	}

	/*dhcp performance optimization enable*/
	if (dhcp_optimize_enable) {
		totalLen += sprintf(cursor, "ip dhcp server optimize enable\n");
		cursor = showStr + totalLen;
	}
	
	if(unicast_flag){
		totalLen += sprintf(cursor, "ip dhcp reply-mode unicast\n");
		cursor = showStr + totalLen;
	}
	if(dhcp_server_static_arp_enable) {
		totalLen += sprintf(cursor, "ip dhcp server static-arp enable\n");
		cursor = showStr + totalLen;
	}
	/*global dhcp configure*/
	if (global_server_option.domainname) {			
		totalLen += sprintf(cursor, "ip dhcp server domain %s\n", global_server_option.domainname);
		cursor = showStr + totalLen;
	}
	if (global_server_option.option43.ipnum) {			
		
		for (i = 0; i < global_server_option.option43.ipnum; i++) {	
			ip_list[0] = global_server_option.option43.ip[i]>>24 & 0xff;
			ip_list[1] = global_server_option.option43.ip[i]>>16 & 0xff;
			ip_list[2] = global_server_option.option43.ip[i]>>8 & 0xff;
			ip_list[3] = global_server_option.option43.ip[i] & 0xff;
			totalLen += sprintf(cursor, " ip dhcp server option43 ");
			cursor = showStr + totalLen;
			
			inet_ntop(AF_INET, ip_list, option_list, sizeof(option_list));
			totalLen += sprintf(cursor, "%s ", option_list);			
			cursor = showStr + totalLen;

			totalLen += sprintf(cursor, "\n");
			cursor = showStr + totalLen;
			memset(option_list, 0, 16);
		}
	}

	if (global_server_option.dnsnum) {	
		int flag = 0;
		for (i = 0; i < global_server_option.dnsnum; i++) {
			if (global_server_option.dnsip[i].iabuf[0]) {
				flag = 1;
				break;
			}
		}
		
		if (flag) {
			totalLen += sprintf(cursor, "ip dhcp server dns");
			cursor = showStr + totalLen;

			for (i = 0; i < global_server_option.dnsnum; i++) {
				if (global_server_option.dnsip[i].iabuf[0]) {
					
					totalLen += sprintf(cursor, " %u.%u.%u.%u", 
											global_server_option.dnsip[i].iabuf[0], global_server_option.dnsip[i].iabuf[1], 
											global_server_option.dnsip[i].iabuf[2], global_server_option.dnsip[i].iabuf[3]);
					cursor = showStr + totalLen;
				}
			}
			totalLen += sprintf(cursor, "\n");
			cursor = showStr + totalLen;
		}		
	}
	
	
	if (global_server_option.routersip.len) {	/* only one */
		if (global_server_option.routersip.iabuf[0]) {
			totalLen += sprintf(cursor, "ip dhcp server routers %u.%u.%u.%u\n", 
									global_server_option.routersip.iabuf[0], global_server_option.routersip.iabuf[1], global_server_option.routersip.iabuf[2], global_server_option.routersip.iabuf[3]);
			cursor = showStr + totalLen;
		}
	}
	
	if (global_server_option.winsip.len) {	/* only one */
		if (global_server_option.winsip.iabuf[0]) {
			totalLen += sprintf(cursor, "ip dhcp server wins %u.%u.%u.%u\n", 
									global_server_option.winsip.iabuf[0], global_server_option.winsip.iabuf[1], global_server_option.winsip.iabuf[2], global_server_option.winsip.iabuf[3]);
			cursor = showStr + totalLen;
		}
	}
	
	if (global_server_option.defaulttime) {			
		totalLen += sprintf(cursor, "ip dhcp server lease-time %u\n", (global_server_option.defaulttime));
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
		totalLen += sprintf(cursor, "ip pool %s\n", pool_node->poolname);
		cursor = showStr + totalLen;

		sub_node = pool_node->headsubnet;
		/* if pool subnode  is null, donot save pool config */
		subnode_null_flag = 1;
		
		while (sub_node) {		
			if (sub_node->lowip.iabuf[0] && sub_node->highip.iabuf[0]) {
			totalLen += sprintf(cursor, " add range %u.%u.%u.%u %u.%u.%u.%u mask %u.%u.%u.%u\n", 
									sub_node->lowip.iabuf[0], sub_node->lowip.iabuf[1], sub_node->lowip.iabuf[2], sub_node->lowip.iabuf[3],
									sub_node->highip.iabuf[0], sub_node->highip.iabuf[1], sub_node->highip.iabuf[2], sub_node->highip.iabuf[3],
									sub_node->mask.iabuf[0], sub_node->mask.iabuf[1], sub_node->mask.iabuf[2], sub_node->mask.iabuf[3]);
				cursor = showStr + totalLen;

				subnode_null_flag = 0;				
			} 
			
#if 0		
			while (ifhead) {
				ifLen += sprintf(ifcursor, "interface %s\n", ifhead->interface_name);
				ifcursor = showif + ifLen;
				
				ifLen += sprintf(ifcursor, " ip pool %s\n", pool_node->poolname);
				ifcursor = showif + ifLen;		

				ifLen += sprintf(ifcursor, " exit\n");
				ifcursor = showif + ifLen;
				
				ifhead = ifhead->next;
			}
#endif
			/*end for show interface ip pool NAME*/
			sub_node = sub_node->next;
		}		

		if (subnode_null_flag) {
			log_info("pool %s subnode is NULL. Don't save configuration\n", pool_node->poolname);
			goto config_out;
		}
		/*for show interface ip pool NAME*/
		if(pool_node->headsubnet){
			ifhead = pool_node->headsubnet->ifhead;		
			if(ifhead){
				dhcp_dbus_profile_config_interface_save(&showif,&ifcursor,ifhead,pool_node,&ifLen);
			}
		}
		
		option_node = &(pool_node->owned_option);
		if (option_node->domainname) {			
			totalLen += sprintf(cursor, " ip dhcp server domain %s\n", option_node->domainname);
			cursor = showStr + totalLen;
		}
		if (option_node->option43.ipnum) {			
			for (i = 0; i < option_node->option43.ipnum; i++) {			
				ip_list[0] = option_node->option43.ip[i]>>24 & 0xff;
				ip_list[1] = option_node->option43.ip[i]>>16 & 0xff;
				ip_list[2] = option_node->option43.ip[i]>>8 & 0xff;
				ip_list[3] = option_node->option43.ip[i] & 0xff;

				log_debug("option43 is %u.%u.%u.%u\n", ip_list[0], ip_list[1], ip_list[2], ip_list[3]);
				if (ip_list[0]) {
				totalLen += sprintf(cursor, " ip dhcp server option43 ");
				cursor = showStr + totalLen;

				inet_ntop(AF_INET, ip_list, option_list, sizeof(option_list));
				totalLen += sprintf(cursor, "%s\n", option_list);			
				cursor = showStr + totalLen;
				}	
				memset(option_list, 0, 16);				
			}
			
		}

		if (option_node->dnsnum) {
			int flag = 0;
			for (i = 0; i < option_node->dnsnum; i++) {
				if (option_node->dnsip[i].iabuf[0]) {
					flag = 1;
					break;
				}
			}
			if (flag) {
				totalLen += sprintf(cursor, " ip dhcp server dns");
				cursor = showStr + totalLen;
				
				for (i = 0; i < option_node->dnsnum; i++) {				
					if (option_node->dnsip[i].iabuf[0]) {						
						totalLen += sprintf(cursor, " %u.%u.%u.%u", 
												option_node->dnsip[i].iabuf[0], option_node->dnsip[i].iabuf[1], 
												option_node->dnsip[i].iabuf[2], option_node->dnsip[i].iabuf[3]);
						cursor = showStr + totalLen;
					}				
				}
				totalLen += sprintf(cursor, "\n");
				cursor = showStr + totalLen;				
			}
		}

		if(option_node->option138_num){

			for (i = 0; i < option_node->option138_num; i++) {
				if (option_node->option138[i].iabuf[0]) {
			totalLen += sprintf(cursor, " ip dhcp server option138 ");
			cursor = showStr + totalLen;

				inet_ntop(AF_INET, option_node->option138[i].iabuf, option_list, sizeof(option_list));
				totalLen += sprintf(cursor, "%s\n", option_list);
				cursor = showStr + totalLen;
				memset(option_list, 0, 16);
				}
			}
		}
		
		if (option_node->routersip.len) {			
			if (option_node->routersip.iabuf[0]) {
			totalLen += sprintf(cursor, " ip dhcp server routers %u.%u.%u.%u\n", 
									option_node->routersip.iabuf[0], option_node->routersip.iabuf[1], option_node->routersip.iabuf[2], option_node->routersip.iabuf[3]);
			cursor = showStr + totalLen;
			}
		}
		
		if (option_node->winsip.len) {			
			if (option_node->winsip.iabuf[0]) {
			totalLen += sprintf(cursor, " ip dhcp server wins %u.%u.%u.%u\n", 
									option_node->winsip.iabuf[0], option_node->winsip.iabuf[1], option_node->winsip.iabuf[2],option_node->winsip.iabuf[3]);
			cursor = showStr + totalLen;
			}
		}
		
		if (option_node->defaulttime) {			
			totalLen += sprintf(cursor, " ip dhcp server lease-time %u\n", (option_node->defaulttime));
			cursor = showStr + totalLen;
		}
		if(option_node->option60_enable) {
			totalLen += sprintf(cursor, " ip dhcp server option60 %s\n", 
								option_node->option60_enable ? "enable" : "disable");
			cursor = showStr + totalLen;
		}
		if(option_node->option60_cnt) {
			for (i = 0; i < option_node->option60_cnt; i++) {
				totalLen += sprintf(cursor, " ip dhcp server option60 %s\n", option_node->option60[i].id);
				cursor = showStr + totalLen;
			}
		}
/*		
		if (option_node->maxtime) {			
			totalLen += sprintf(cursor, " ip dhcp server lease-max %u\n", (option_node->maxtime)/(60*60*24));
			cursor = showStr + totalLen;
		}
*/
	config_out:
		pool_node = pool_node->next;
		
		totalLen += sprintf(cursor, " exit\n\n");
		cursor = showStr + totalLen;
	}

	host_node = head_host.next;
	while (host_node) {
		if (strncmp(host_node->ifname, "ebr", 3) && strncmp(host_node->ifname, "wlan", 4)) {
			if (host_node->hostip.iabuf[0] && if_nametoindex(host_node->ifname)) {
				totalLen += sprintf(cursor, "ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x %s\n", 
										host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
										host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5],
										host_node->ifname);
				cursor = showStr + totalLen;
			}
		}
		host_node = host_node->next;
	}

	totalLen += sprintf(cursor, showif);
	cursor = showStr + totalLen;
	if (dhcp_bootp_enable) {
		totalLen += sprintf(cursor, "ip dhcp server bootp enable\n");
		cursor = showStr + totalLen;
	}
	if (dhcp_server_enable) {
		totalLen += sprintf(cursor, "ip dhcp server enable\n");
		cursor = showStr + totalLen;
	}
	if(local7){
		totalLen += sprintf(cursor, "dhcp log for local7 enable\n");
		cursor = showStr + totalLen;

	}
	if(ASN){
		totalLen += sprintf(cursor, "ip dhcp server asn enable\n");
		cursor = showStr + totalLen;
	}
	if(DHCP_AUTO_RESTART_ENABLE){
		totalLen += sprintf(cursor, "ip dhcp server auto restart enable\n");
		cursor = showStr + totalLen;
	}
	/* dba */
	if (dba_server_enable_flag) {
		totalLen += sprintf(cursor, " config direct-broadcast enable\n");
		cursor = showStr + totalLen;
	}

	/* pppoe snp */
	if (pppoe_snp_enable_flag) {
		totalLen += sprintf(cursor, " config pppoe-snooping enable\n");
		cursor = showStr + totalLen;
	}

	if (pppoe_snp_cfg_head) {
		pppoe_node = pppoe_snp_cfg_head;
		for (; pppoe_node; pppoe_node = pppoe_node->next) {
			if (!strncmp(pppoe_node->ifname, "ebr", 3) || !strncmp(pppoe_node->ifname, "wlan", 4)) {
				continue;
			}
			
			if (if_nametoindex(pppoe_node->ifname)) {
				if (pppoe_node->mru) {
					totalLen += sprintf(cursor, " config pppoe-snooping %s enable %d\n", 
						pppoe_node->ifname, pppoe_node->mru);
				} else {
					totalLen += sprintf(cursor, " config pppoe-snooping %s enable\n", pppoe_node->ifname);
				}
				cursor = showStr + totalLen;
			}
		}
	}
	showStr[totalLen] = '\0';

	log_debug("show dhcp running cfg totalLen %d, iflen = %d\n", totalLen, ifLen);
	if (showif) {
		free(showif);
		showif = NULL;
	}
	return ;
}

int dhcp_dbus_pppoe_hansi_cfg(char *showStr, char **cursor, int *totalLen, int slot, int vrrp, int local_flag)
{
	struct distributed_iface_info iface_info;
	struct pppoe_snp_cfg *tmp = pppoe_snp_cfg_head;

	log_debug("show pppoe snp hansi running: slot %d vrrp %d local flag %d\n", slot, vrrp, local_flag);

	if (!showStr || !cursor || !*cursor || !totalLen) {
		return -1;
	}
	
	if (local_flag) {
		if (vrrp) {
			*totalLen += sprintf(*cursor, "config local-hansi %d-%d\n", slot, vrrp);
			*cursor = showStr + *totalLen;
		}
	} else {
		if (vrrp) {
			*totalLen += sprintf(*cursor, "config hansi-profile %d-%d\n", slot, vrrp);
			*cursor = showStr + *totalLen;
		}
	}


	for (; tmp; tmp = tmp->next) {
		if (local_flag 
			&& (!strncmp(tmp->ifname, "ebrl", 4) || !strncmp(tmp->ifname, "wlanl", 5))) {
 		} else if (!local_flag 	
				&& (((!strncmp(tmp->ifname, "ebr", 3)) && (strncmp(tmp->ifname, "ebrl", 4))) || 
				   ((!strncmp(tmp->ifname, "wlan", 4)) && (strncmp(tmp->ifname, "wlanl", 5))))) {
 		} else {
			continue;
		}

		memset(&iface_info, 0, sizeof(iface_info));
		if (dhcp_dbus_parse_ifname(&iface_info, tmp->ifname)) {
			continue;
		}

		if ((iface_info.slotid == slot)
			&& (iface_info.vrrpid == vrrp)) {
			if (if_nametoindex(tmp->ifname)) {
				if (tmp->mru) {
					*totalLen += sprintf(*cursor, " config pppoe-snooping %s enable %d\n", 
						iface_info.ifname, tmp->mru);
				} else {
					*totalLen += sprintf(*cursor, " config pppoe-snooping %s enable\n", iface_info.ifname);
				}
				*cursor = showStr + *totalLen;
			}
		}
	}	

	if (vrrp) {
		*totalLen += sprintf(*cursor, " exit\n");
		*cursor = showStr + *totalLen;
	}
	return 0;
}

void dhcp_dbus_profile_config_hansi_save(char *showStr, int slot, int vrrp, int local_flag)
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

				if (1 != check_interfaces_ip_address(ifhead->interface_name)) {
					continue;
				}
				
				memset(ifname, 0, sizeof(ifname));
				memcpy(ifname, ifhead->interface_name, strlen(ifhead->interface_name));
				log_debug("show running hansi cfg: interface %s\n", ifname);
				
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

				else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
					ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
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

						totalLen += sprintf(cursor, " ip pool %s\n", pool_node->poolname);
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
			}
		}
	}

	/* dhcp static */
	struct dcli_host *host_node = NULL;
	if (local_flag) {
		if (vrrp) {
			totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
			cursor = showStr + totalLen;
		}
		
		host_node = head_host.next;
		while (host_node) {
			if (!strncmp(host_node->ifname, "ebrl", 4)) {
				ret = sscanf(host_node->ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret) && (slot == value1) && (vrrp == value2) && (local_flag)) {
					if (host_node->hostip.iabuf[0] && if_nametoindex(host_node->ifname)) {
						memset(ifname, 0, sizeof(ifname));
						sprintf(ifname, "ebr%d", value3);
						totalLen += sprintf(cursor, " ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x %s\n", 
												host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
												host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5],
												ifname);
						cursor = showStr + totalLen;
					}
				}
			} else if (!strncmp(host_node->ifname, "wlanl", 5)) {
				ret = sscanf(host_node->ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret) && (slot == value1) && (vrrp == value2) && (local_flag)) {
					if (host_node->hostip.iabuf[0] && if_nametoindex(host_node->ifname)) {
						memset(ifname, 0, sizeof(ifname));
						sprintf(ifname, "wlan%d", value3);
						totalLen += sprintf(cursor, " ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x %s\n", 
												host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
												host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5],
												ifname);
						cursor = showStr + totalLen;
					}
				}
			} 
			host_node = host_node->next;
		}

		if (vrrp) {
			totalLen += sprintf(cursor, " exit\n");
			cursor = showStr + totalLen;
		}

	} else {
		if (vrrp) {
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
			cursor = showStr + totalLen;
		}
		
		host_node = head_host.next;
		while (host_node) {
			if ((!strncmp(host_node->ifname, "ebr", 3)) && (strncmp(host_node->ifname, "ebrl", 4))) {
				ret = sscanf(host_node->ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
					if (host_node->hostip.iabuf[0] && if_nametoindex(host_node->ifname)) {
						memset(ifname, 0, sizeof(ifname));
						sprintf(ifname, "ebr%d", value3);						
						totalLen += sprintf(cursor, " ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x %s\n", 
												host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
												host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5],
												ifname);
						cursor = showStr + totalLen;
					}
				}
			} else if ((0 == strncmp(host_node->ifname, "wlan", 4)) && (strncmp(host_node->ifname, "wlanl", 5))) {
				ret = sscanf(host_node->ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
				if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
					memset(ifname, 0, sizeof(ifname));
					sprintf(ifname, "wlan%d", value3);					
					if (host_node->hostip.iabuf[0] && if_nametoindex(host_node->ifname)) {
						totalLen += sprintf(cursor, " ip dhcp static %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x %s\n", 
												host_node->hostip.iabuf[0], host_node->hostip.iabuf[1], host_node->hostip.iabuf[2], host_node->hostip.iabuf[3],
												host_node->hostmac[0], host_node->hostmac[1], host_node->hostmac[2], host_node->hostmac[3], host_node->hostmac[4], host_node->hostmac[5],
												ifname);
						cursor = showStr + totalLen;
					}
				}
			} 
			host_node = host_node->next;
		}

		if (vrrp) {
			totalLen += sprintf(cursor, " exit\n");
			cursor = showStr + totalLen;
		}
	}

	dhcp_dbus_pppoe_hansi_cfg(showStr, &cursor, &totalLen, slot, vrrp, local_flag);
	return ;
}


DBusMessage *
dhcp_dbus_check_server_interface
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int	ret = 0;
	unsigned int detect = 0;
	struct dcli_subnet* subNode = NULL;
	char *upifname = NULL, *downifname = NULL;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_STRING, &upifname,					
					DBUS_TYPE_STRING, &downifname,
					DBUS_TYPE_UINT32,&detect,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	ret = dhcp_dbus_find_subnet_by_ifname(upifname, &subNode);
	ret &= dhcp_dbus_find_subnet_by_ifname(downifname, &subNode);
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
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
dhcp_dbus_save_lease
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
	
	save_dhcp_info_file();
	/*test_for_add_dhcp_file();
	test_for_add_dhcp_file();*/
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}

DBusMessage* 
dhcp_dbus_show_lease
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
	unsigned char mac[DHCP_MAC_ADDRESS_LEN] = {0};
	unsigned int ret = DHCP_SERVER_RETURN_CODE_SUCCESS;	
	struct dhcp_lease_info *lease_info = NULL;
	
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
	
	reply = dbus_message_new_method_return(msg);
	
	count = get_dhcp_lease_number();
	log_debug("dhcp lease number is %d\n", count);	
	if (count) {
	
		lease_info = (struct dhcp_lease_info *)malloc(sizeof(struct dhcp_lease_info)
										+ sizeof(struct dhcp_lease_ip_info) * count);
		if (lease_info) {
			memset(lease_info, 0, sizeof(struct dhcp_lease_info)
											+ sizeof(struct dhcp_lease_ip_info) * count);
			
			ret = get_dhcp_lease_info(lease_info, count);
		} else {
			log_error("%s : malloc failed\n", __func__);
			ret = DHCP_SERVER_RETURN_CODE_FAIL;
		}
		
	} else {
		ret = DHCP_SERVER_RETURN_CODE_FAIL;
	}
	
	dbus_message_iter_init_append (reply, &iter);

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
				/*change to use bit arithmetic, can avoid byte order problem*/
				memcpy(&ipaddr, lease_info->lease_ip[i].ip_addr.iabuf, DHCP_IPV4_ADDRESS_LEN);
				memcpy(mac, lease_info->lease_ip[i].hw_addr, DHCP_MAC_ADDRESS_LEN);
				
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

	if (lease_info) {		
		free(lease_info);
		lease_info = NULL;
	}

	return reply;
}

DBusMessage* 
dhcp_dbus_config_lease_time
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter  iter;
	DBusError err;

	unsigned int local_en=0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&local_en,
		DBUS_TYPE_INVALID))){		
			 log_error("Unable to get input args ");
			if (dbus_error_is_set(&err)){
				log_error("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
	}
	client_lease_time_enable = local_en;

	log_info("client lease time is %s\n", client_lease_time_enable ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
							 DBUS_TYPE_UINT32, 
							 &client_lease_time_enable);
	return reply;
}

DBusMessage* 
dhcp_dbus_show_static_host
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
												DBUS_TYPE_STRING_AS_STRING
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
				dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_STRING,
						  &(statichost->ifname));  /*ifname*/
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				statichost = statichost->next;
			}	
			dbus_message_iter_close_container (&iter, &iter_array);
		}	
	}
	return reply;
}

DBusMessage* 
dhcp_dbus_show_failover_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter  iter;	
	DBusError err;
	unsigned int ret = 0, mode = 0, index = 0, op_ret = 0;
	unsigned int primary = 0, split = 0, mclt = 0, dstip = 0, dstport = 0, srcip = 0, srcport = 0;
	char *failname = NULL;
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;
	
	dbus_error_init(&err);	
	if (!(dbus_message_get_args ( msg, &err,
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

	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret && poolnode->headsubnet) {
		/*only one subnet one failover*/
		subnode = poolnode->headsubnet;

		if (subnode->owned_failover.enable) {
			failname = malloc(strlen(subnode->owned_failover.name) + 1);
			memset(failname, 0, (strlen(subnode->owned_failover.name) + 1));
			memcpy(failname, subnode->owned_failover.name, strlen(subnode->owned_failover.name));
			primary = subnode->owned_failover.primary;
			split = subnode->owned_failover.split;
			mclt = subnode->owned_failover.mclt;
			dhcp_dbus_four_char_to_int(subnode->owned_failover.parteraddr.iabuf, &dstip);
			dstport = subnode->owned_failover.parterport;
			dhcp_dbus_four_char_to_int(subnode->owned_failover.myaddr.iabuf, &srcip);
			srcport = subnode->owned_failover.myport;
		}
		else {
			failname = malloc(1);
			memset(failname, 0, 1);
			log_error("failover state change error not enable\n");
			op_ret = DHCP_POOL_HAVE_NOT_FAILOVER;
		}
	
	}
	else {	
		failname = malloc(1);
		memset(failname, 0, 1);
		log_error("failover state change error pool has no subnet\n");
		op_ret = DHCP_NOT_THE_SUBNET;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &primary);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &split);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &mclt);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &dstip);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &dstport);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &srcip);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &srcport);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING, 
									 &failname);

	if(failname){
		free(failname);
		failname=NULL;
	}
	return reply;
}

DBusMessage * 
dhcp_dbus_show_lease_by_ip
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

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32, &ipAddr,
								DBUS_TYPE_UINT32, &ip_Nums,
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
				memcpy(&ipaddr, lease_ip->ip_addr.iabuf, DHCP_IPV4_ADDRESS_LEN);
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
	free_lease_ip_info(&lease_info);
	return reply;
}	

DBusMessage *
dhcp_dbus_show_lease_by_mac
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
		memcpy(&ipaddr, lease_ip.ip_addr.iabuf, DHCP_IPV4_ADDRESS_LEN);
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
dhcp_dbus_show_dhcp_global_conf
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int	op_ret = 0, i = 0, value = 0, enable = 0, len = 0, staticarp = 0;
	unsigned int	dns[3] = {0, 0, 0}, routers = 0, wins = 0, default_time = 0, max_time = 0;
	char* domainname = NULL, *option43 = NULL, *cursor = NULL;
	DBusError err;

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
	staticarp = dhcp_server_static_arp_enable;
	if (global_server_option.domainname) {
		len = strlen(global_server_option.domainname);
		domainname = malloc(len + 1);
		memset(domainname, 0, (len + 1));
		memcpy(domainname, global_server_option.domainname, len);
	}
	else {	
		domainname = malloc(1);
		memset(domainname, 0, 1);
	}
	
	if (global_server_option.option43.ipnum) {
		len = global_server_option.option43.len + 4;;		
		option43 = malloc(len + 1);
		memset(option43, 0, (len + 1));
		cursor = option43;
		len = 0;
		len += sprintf(cursor, "%.2x%.2x", global_server_option.option43.type, global_server_option.option43.len);
		cursor = option43 + len;
		for (i = 0; i < global_server_option.option43.ipnum; i++) {
			len += sprintf(cursor, "%.8x", global_server_option.option43.ip[i]);
			cursor = option43 + len;
		}
	}
	else {	
		option43 = malloc(1);
		memset(option43, 0, 1);
	}

	
	for (; i < global_server_option.dnsnum; i ++) {
		dhcp_dbus_four_char_to_int((global_server_option.dnsip[i].iabuf), &dns[i]);
		log_debug("dns ip is %x \n", dns[i]);
	}
	
	dhcp_dbus_four_char_to_int((global_server_option.routersip.iabuf), &routers);
	dhcp_dbus_four_char_to_int((global_server_option.winsip.iabuf), &wins);

	default_time = (global_server_option.defaulttime);
	max_time = (global_server_option.maxtime);
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &op_ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &enable);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &unicast_flag);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &staticarp);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING, &domainname);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING, &option43);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &dns[0]);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &dns[1]);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &dns[2]);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &routers);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &wins);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &default_time);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &max_time);

	free(domainname);
	free(option43);
	
	return reply;	
}

DBusMessage * 
dhcp_dbus_show_dhcp_pool_conf
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
	unsigned int loop = 0;
	unsigned int i = 0, j = 0, k = 0,ret = 0,  len = 0, mode = 0, index = 0, pool_count = 0, default_time = 0, max_time = 0;
	unsigned int domainnull = 0, option43null = 0, ipaddr[3] = {0, 0, 0}, dns[3] = {0, 0 ,0}, iptmp[6] = {0, 0, 0, 0, 0, 0};
	struct dcli_pool* poolnode = NULL;
	struct dcli_subnet* subnetnode = NULL;
	char *domainname = NULL, *option43 = NULL, *cursor = NULL;
	char *tmp_interface_name = NULL;
	char  **ap_via_address = NULL;
	char *option60_id = NULL;
#if 0
	char domain_name[BUFSIZE] ={0};
	char option_43[BUFSIZE]	= {0};
	char interface_name[BUFSIZE] = {0};
	char option_138[16][BUFSIZE] ;

	domainname = domain_name;
	option43 = option_43;
	tmp_interface_name = interface_name ;

	for(i = 0; i < 16; i++){
		ap_via_address[i] = option_138[i];
	}
#endif

	domainname = malloc(len + 1);
	if(NULL == domainname){
		log_debug("malloc fail in %s on %d\n", __FILE__, __LINE__);
		return NULL;
	}
	memset(domainname, 0, (len + 1));
	option43 = malloc(512);
	if(NULL == option43){
		log_debug("malloc fail in %s on %d\n", __FILE__, __LINE__);
	  	if(domainname){
			free(domainname);
			domainname=NULL;
	  	}
		return NULL;
	}
	tmp_interface_name = malloc(1);
	if(NULL == tmp_interface_name){
		log_debug("malloc fail in %s on %d\n", __FILE__, __LINE__);
		if(domainname){
			free(domainname);
			domainname=NULL;
	  	}
		if(option43){
			free(option43);
			option43=NULL;
	  	}
		return NULL;
	}
	memset(tmp_interface_name,  0, 1);

	ap_via_address = (char **)malloc(sizeof(char *) * 16);
	if(NULL == ap_via_address){
		log_debug("malloc fail in %s on %d\n", __FILE__, __LINE__);
		
		if(domainname != NULL){                
            free(domainname);                  
        }                                      
        if(option43){                          
            free(option43);                    
        }                                      
        if(tmp_interface_name){                
            free(tmp_interface_name);          
        }
		return NULL;
	}
	memset(ap_via_address, 0, sizeof(char *) * 16);

	for(i = 0; i < 16; ++i){
		ap_via_address[i] = malloc(16);	
		if(NULL == ap_via_address[i]){
			log_debug("malloc fail in %s on %d\n", __FILE__, __LINE__);
				  SHOW_DHCP_POOL_FREE;
			return NULL;
		}
		memset(ap_via_address[i], 0 , 16);
	}
	option60_id = (char *)malloc(DHCP_OPTION60_ID_DEFAULT_LENGTH);
	if (NULL == option60_id) {
		log_debug("%s:%d, malloc failed\n", MDL);
		SHOW_DHCP_POOL_FREE;		
		return NULL;
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
	   SHOW_DHCP_POOL_FREE;
	   free(option60_id);
	   return NULL;
	}

	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {			
			pool_count = 1;
		}
	}
	else {
		poolnode = head_pool.next;
		pool_count = count_pool;
	}

	reply = dbus_message_new_method_return(msg);
	
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
								DBUS_TYPE_UINT32_AS_STRING    /*defaulttime*/
								DBUS_TYPE_UINT32_AS_STRING    /*maxtime*/
								DBUS_TYPE_UINT32_AS_STRING    /*dns[0]*/
								DBUS_TYPE_UINT32_AS_STRING    /*dns[1]*/
								DBUS_TYPE_UINT32_AS_STRING    /*dns[2]*/

								DBUS_TYPE_STRING_AS_STRING	/*option138[0]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[1]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[2]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[3]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[4]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[5]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[6]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[7]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[8]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[9]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[10]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[11]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[12]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[13]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[14]*/
								DBUS_TYPE_STRING_AS_STRING	/*option138[15]*/
								
								DBUS_TYPE_UINT32_AS_STRING    /*routers*/
								DBUS_TYPE_UINT32_AS_STRING    /*wins*/
								DBUS_TYPE_STRING_AS_STRING    /*domainname*/
								DBUS_TYPE_STRING_AS_STRING    /*option43 string*/
								DBUS_TYPE_UINT32_AS_STRING	  /* option60 flag */
								DBUS_TYPE_STRING_AS_STRING    /* option60 string [0] */
								DBUS_TYPE_STRING_AS_STRING    /* option60 string [1] */
								DBUS_TYPE_STRING_AS_STRING    /* option60 string [2] */								
								DBUS_TYPE_STRING_AS_STRING    /* option60 string [3] */								
								DBUS_TYPE_STRING_AS_STRING    /*poolname*/
								DBUS_TYPE_STRING_AS_STRING    /*interfacename bind pool*/
								DBUS_TYPE_UINT32_AS_STRING    /*sub_count*/
								DBUS_TYPE_ARRAY_AS_STRING
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
								DBUS_TYPE_UINT32_AS_STRING    /*iplow*/								
								DBUS_TYPE_UINT32_AS_STRING	  /*iphigh*/
								DBUS_TYPE_UINT32_AS_STRING	  /*mask*/
								DBUS_STRUCT_END_CHAR_AS_STRING 
								DBUS_STRUCT_END_CHAR_AS_STRING,     /*end*/
								    &iter_array);
	
	if (pool_count) {
		log_debug("pool_count is  %d\n", pool_count);		
		 for (i = 0; i < pool_count; i++ ) {
			
			if (!(poolnode->owned_option.domainname)) {
				domainnull = 1;
				/*poolnode->owned_option.domainname = domainname;*/
			}
			if (!(poolnode->owned_option.option43.ipnum)) {
				option43null = 1;
			}
	//		log_debug("poolnode->owned_option.option43.ipnum is %d\n", poolnode->owned_option.option43.ipnum);
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			default_time = (poolnode->owned_option.defaulttime);
			dbus_message_iter_append_basic
					(&iter_struct,
					  DBUS_TYPE_UINT32,
					  &(default_time));
			
			max_time = (poolnode->owned_option.maxtime);
			dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(max_time));
			memset(dns, 0, 3*4);
			for (j = 0; j < poolnode->owned_option.dnsnum; j++) {
				dhcp_dbus_four_char_to_int(poolnode->owned_option.dnsip[j].iabuf, &(ipaddr[j]));
				dns[j] = ipaddr[j];
			}
			dbus_message_iter_append_basic (&iter_struct,DBUS_TYPE_UINT32,  &(dns[0]));
			dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &(dns[1]));
			dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_UINT32, &(dns[2]));

			/*show option138*/
			for(j = 0; j < poolnode->owned_option.option138_num; ++j){
				if(poolnode->owned_option.option138[j].iabuf[0]){
				inet_ntop(AF_INET, poolnode->owned_option.option138[j].iabuf, ap_via_address[j], 16);
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_STRING, &(ap_via_address[j]));	
				}			
			}
			for(k = j; k < 16; ++k){
				dbus_message_iter_append_basic (&iter_struct, DBUS_TYPE_STRING, &(ap_via_address[k]));				
			}
			for(k = 0;k < 16; k++){
			memset(ap_via_address[k], 0 , 16);
			}
			dhcp_dbus_four_char_to_int(poolnode->owned_option.routersip.iabuf, &(iptmp[0]));
			dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(iptmp[0]));
			dhcp_dbus_four_char_to_int(poolnode->owned_option.winsip.iabuf, &(iptmp[1]));
			dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(iptmp[1]));
			if (domainnull) {
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(domainname));
				domainnull = 0;
			}
			else {		
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(poolnode->owned_option.domainname));
			}
			if (option43null) {
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(domainname));
				option43null = 0;
			}
			else {	
				memset(option43, 0, BUFSIZE);
						cursor = option43;
				len = 0;
				len += sprintf(cursor, "%.2x%.2x", poolnode->owned_option.option43.type, poolnode->owned_option.option43.len);
				cursor = option43 + len;
				for (k = 0; k < poolnode->owned_option.option43.ipnum; k++) {
					len += sprintf(cursor, "%.8x", poolnode->owned_option.option43.ip[k]);
					cursor = option43 + len;
				}
				log_debug("option43 %s\n", option43);
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(option43));
			}
			dbus_message_iter_append_basic(&iter_struct,
					 DBUS_TYPE_UINT32, &(poolnode->owned_option.option60_enable));
			for (loop = 0; loop < poolnode->owned_option.option60_cnt; loop++) {
				if (poolnode->owned_option.option60) {
					memset(option60_id, 0, DHCP_OPTION60_ID_DEFAULT_LENGTH);
					sprintf(option60_id, "%s", poolnode->owned_option.option60[loop].id);
					dbus_message_iter_append_basic(&iter_struct,
							 DBUS_TYPE_STRING, &(option60_id));
				}
			}
			for (; loop < DHCP_OPTION60_DEFAULT_NUMBER; loop++) {
				dbus_message_iter_append_basic(&iter_struct,
						 DBUS_TYPE_STRING, &(domainname));
			}			
			dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_STRING,
					 &(poolnode->poolname));
			if(poolnode->headsubnet &&
				poolnode->headsubnet->ifhead &&
				poolnode->headsubnet->ifhead->interface_name) {				
				
				dbus_message_iter_append_basic 
						(&iter_struct,
						 DBUS_TYPE_STRING,
						 &(poolnode->headsubnet->ifhead->interface_name));
				
			}
			else {	
				
				dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_STRING,
					 &(tmp_interface_name));

				
			}
			
			dbus_message_iter_append_basic 
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(poolnode->count_sub));
			log_debug("sub_count[%d] is  %d\n", i, poolnode->count_sub);
			
				dbus_message_iter_open_container (&iter_struct,
													DBUS_TYPE_ARRAY,
													DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
													DBUS_TYPE_UINT32_AS_STRING	  /*iplow*/ 							
													DBUS_TYPE_UINT32_AS_STRING	  /*iphigh*/
													DBUS_TYPE_UINT32_AS_STRING	  /*mask*/
													DBUS_STRUCT_END_CHAR_AS_STRING, 
													&iter_sub_array);
				subnetnode = poolnode->headsubnet;
				for(j = 0; j < poolnode->count_sub; j++) {
					DBusMessageIter iter_sub_struct;				
					
					dbus_message_iter_open_container (&iter_sub_array,
												      DBUS_TYPE_STRUCT,
												   	  NULL,
												   	  &iter_sub_struct);
					dhcp_dbus_four_char_to_int(subnetnode->lowip.iabuf, &(iptmp[2]));
					dbus_message_iter_append_basic 
							(&iter_sub_struct,
							 DBUS_TYPE_UINT32,
							 &(iptmp[2]));
					
					dhcp_dbus_four_char_to_int(subnetnode->highip.iabuf, &(iptmp[3]));
					dbus_message_iter_append_basic 
							(&iter_sub_struct,
							 DBUS_TYPE_UINT32,
							 &(iptmp[3]));
					
					dhcp_dbus_four_char_to_int(subnetnode->mask.iabuf, &(iptmp[4]));
					dbus_message_iter_append_basic 
							(&iter_sub_struct,
							 DBUS_TYPE_UINT32,
							 &(iptmp[4]));
				 	dbus_message_iter_close_container (&iter_sub_array, &iter_sub_struct);
					if(subnetnode){
					subnetnode = subnetnode->next;
					}
				}
				
				dbus_message_iter_close_container (&iter_struct, &iter_sub_array);			
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				poolnode = poolnode->next;
		 } /*for*/
	} /*if*/
	dbus_message_iter_close_container (&iter, &iter_array);

	SHOW_DHCP_POOL_FREE;
	if (option60_id) {
		free(option60_id);
	}
	return reply;

}

DBusMessage * 
dhcp_dbus_entry_pool_node
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
	ret = dhcp_dbus_find_pool_by_name(poolName, &poolNode);
	if(!ret) {
		index = poolNode->index;
		op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;      
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
dhcp_dbus_set_interface_pool
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	struct dcli_pool* poolnode = NULL;
	struct dcli_subnet* subNode = NULL;
	char* poolName = NULL, *ifname = NULL;
	unsigned int add = 0, op_ret = 0, ret = 0;
	DBusError err;
	struct dcli_subnet* subnode = NULL;
	unsigned int bind_flag = 0;
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &poolName, 
		DBUS_TYPE_STRING, &ifname, 
		DBUS_TYPE_UINT32, &add,
		DBUS_TYPE_UINT32, &bind_flag,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	log_info("interface %s bind pool %s\n", ifname, poolName);		

	if (add && (0 == check_interfaces_ip_address(ifname))) {
		log_info("interface %s without set ip adddress\n", ifname);		
		op_ret = DHCP_INTERFACE_WITHOUT_IPADDR;
		goto out;
	}
	/* is poolnode exist */
	ret = dhcp_dbus_find_pool_by_name(poolName, &poolnode);
	if (ret) {	/* not find pool node, failed */			
		op_ret = DHCP_NOT_FOUND_POOL;
		log_error("bad pool %s\n", poolName);
		goto out;
	}

	/* poolnode has subnet ? */
	if (!poolnode->headsubnet) {
		op_ret = DHCP_POOL_SUBNET_NULL;
		log_error("pool %s has no subnet, set interface %s pool failed.\n", poolName, ifname);
		goto out;
	}

	/* interface bind pool */
	if (SET_INTERFACE_IP_POOL == add) {
		/*check pool bind state */
		if (poolnode->headsubnet) {
			subnode = poolnode->headsubnet;				
		}
		while (subnode) {
			if(subnode->interface_num){
				op_ret = DHCP_HAVE_BIND_INTERFACE;
				log_error("pool %s already bound to interface %s.\n", poolName, ifname);				
				goto out;
			}
			subnode = subnode->next;
		}

		/*check interface  binded state */
		ret = dhcp_dbus_find_subnet_by_ifname(ifname, &subNode);
		if (!ret) {
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
			log_error("interface %s bind pool already\n", ifname);
			goto out;				
		}

		/* bind pool to interface */
		op_ret = dhcp_dbus_interface_bind_pool(poolnode, ifname);
		if (!op_ret) {
			op_ret = interface_reference_pool_node(ifname, poolnode);
		}

		/* set interface unbind_flag */
		ret = set_interface_bind_flag(ifname, bind_flag);
		if(ret){
			log_error("have not interface %s\n", ifname);
		}
		
	}

	/* interface unbind pool */
	else if (CLR_INTERFACE_IP_POOL == add) {

		/* check pool bind state */
		if (!poolnode->refcount) {
			log_error("pool %s not bind to any interface\n", poolName);
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
			goto out;
		}

		/* pool and interafac match */
		if (poolnode->headsubnet 
			&& poolnode->headsubnet->ifhead 
			&& (!strcmp(ifname, poolnode->headsubnet->ifhead->interface_name))) {
			
		} else {
			log_error("pool %s and interface %s don't match\n", poolName, ifname);
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
			goto out;
		}

		/* set interface unbind_flag */
		ret = set_interface_bind_flag(ifname, bind_flag);
		if(ret){
			log_error("have not interface %s\n", ifname);
		}
				
		subNode = poolnode->headsubnet;
		op_ret = interface_dereference_pool_node(ifname, poolnode);
		while (subNode) {
			op_ret = dhcp_dbus_interface_unbind_pool(poolnode, ifname);
			poolnode->refcount = 0;
			subNode = subNode->next;
		}
	}

	log_debug("%s bind flag is %d\n", ifname, bind_flag);
	
out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	
	
	return reply;
}


DBusMessage * 
dhcp_dbus_create_pool_node
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

	log_info("create pool %s\n", poolName);

	if (strlen(poolName) > ALIAS_NAME_SIZE) {
		log_error("create pool node failed: %s too long, max %d\n", poolName, ALIAS_NAME_SIZE);
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;      
	} else {
		/*bool */
		name = malloc(strlen(poolName) + 1);
		if (!name) {
			log_error("malloc poolname failed!\n");
			return NULL;
		}
		memset(name, 0, (strlen(poolName) + 1));
		memcpy(name, poolName, strlen(poolName));
		log_debug("dhcp_dbus_create_pool_by_name name is %s \n", poolName);
		if (del) {
			op_ret = dhcp_dbus_delete_pool_by_name(name);
			if (name) {
				free(name);
				name = NULL;
			}
		}
		else {
			ret = dhcp_dbus_create_pool_by_name(name, &poolNode);
			if(!ret) {
				op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
				index = poolNode->index;
			}
			else {
				op_ret = ret; 
			}
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
dhcp_dbus_set_server_domain_name
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	char *domainName = NULL, *tmpname = NULL;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &domainName,
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
	tmpname = malloc(strlen(domainName) + 1);
	memset(tmpname, 0, (strlen(domainName)+ 1));
	memcpy(tmpname, domainName, strlen(domainName));
	owned_option.domainname = tmpname;

	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set option43\n");
					continue;
				}
				op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp_dbus_set_server_v1
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	char *veo = NULL, *tmpname = NULL;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
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

	ret = string_to_option43_s(tmpname, &(owned_option.option43));
	if (ret) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	log_debug("set server veo type is %x, len is %x, ipnum is %x, ip is %x\n",
			owned_option.option43.type, owned_option.option43.len, owned_option.option43.ipnum, owned_option.option43.ip[0]);
	/*sub mode*/
	if (mode) {	
		if (!ret) {	
			ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
			if (!ret) {
				addsubnet = poolnode->headsubnet;
				if(NULL == addsubnet){
					op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
				}
				while (addsubnet) {
					if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
						pre_subnet = addsubnet;
						addsubnet = addsubnet->next;
						log_debug("same subnet set option43\n");
						continue;
					}
					op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);
					if(op_ret){
						break;
					}
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
				}
			}
			else {
				op_ret = DHCP_NOT_FOUND_POOL;
			}
		}
		else {
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}
out:
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
dhcp_dbus_set_server_veo
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;
	struct in_addr option43_ip;
	char *ipv4adr = NULL;	

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &ipv4adr,	 
		DBUS_TYPE_UINT32, &mode,		
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_UINT32, &del,
		DBUS_TYPE_INVALID))) {
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s in dhcp_dbus_set_server_veo funtion\n",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	
	if(inet_pton(AF_INET, ipv4adr, &option43_ip) != 1){

		log_error("Illegal ipv4 address %s for set option43\n", ipv4adr ? ipv4adr : "NULL");		
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		reply = dbus_message_new_method_return(msg);	
		dbus_message_iter_init_append (reply, &iter);	
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,  &op_ret);		
		return reply;
	}

	if (dhcp_dbus_check_ip_address(ipv4adr, NULL)) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
		
	/*autelan type 0x11 */
	owned_option.option43.type = 0x11;
	/*ipv4 number*/
	owned_option.option43.ipnum = 1;	
	/*option43 lenth*/
	owned_option.option43.len =  sizeof(int);

	/*this malloc int for compatible with the previous command mode*/
	owned_option.option43.ip = malloc(owned_option.option43.len);
	if(NULL == owned_option.option43.ip ){
		log_error("malloc fail for set option43 on %d\n", __LINE__);
		return NULL;
	}
	memset(owned_option.option43.ip, 0, sizeof(int));					
	owned_option.option43.ip[0] = dhcp_dbus_ip2ulong(ipv4adr);	

	/*find set pool*/
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret) {
		addsubnet = poolnode->headsubnet;
		if(NULL == addsubnet){
			op_ret = DHCP_SET_ORDER_WANNING;		//first must set ip range
		}
		while (addsubnet) {
			if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
				log_debug("same subnet set option43\n");
				continue;
			}
			
			op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);	//set option43
			if(op_ret){
				break;
			}
			pre_subnet = addsubnet;
			addsubnet = addsubnet->next;
		}
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
	
out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,  &op_ret);	

	if(owned_option.option43.ip){
		free(owned_option.option43.ip);
	}
	return reply;
}


DBusMessage * 
dhcp_dbus_set_server_no_veo
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;	
	unsigned int	op_ret = 0, ret = 0 ,index = 0, del = 1;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet= NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;	

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);	
	
	if (!(dbus_message_get_args ( msg, &err,			
		DBUS_TYPE_UINT32, &index,		
		DBUS_TYPE_INVALID))) {		 
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s in dhcp_dbus_set_server_veo funtion\n",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	
	
	/*autelan type 0 is delete flag */
	owned_option.option43.type = 0;
	/*ipv4 number*/
	owned_option.option43.ipnum = 1;	
	
	/*find set pool*/
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	
	if (!ret) {
		addsubnet = poolnode->headsubnet;
		if(NULL == addsubnet){
			op_ret = DHCP_SET_ORDER_WANNING;		//first must set ip range
		}
		
		while (addsubnet) {	
			if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
				log_debug("same subnet set option43\n");
				continue;
			}
			op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);	//set option43
			if(op_ret){
				break;
			}
			pre_subnet = addsubnet;
			addsubnet = addsubnet->next;
		}
		
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
	

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,  &op_ret);	

	return reply;
}


DBusMessage * 
dhcp_dbus_set_server_no_option138

(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;	
	unsigned int	op_ret = 0, ret = 0 ,index = 0, del = 1;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;	

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);	
	
	if (!(dbus_message_get_args ( msg, &err,			
		DBUS_TYPE_UINT32, &index,		
		DBUS_TYPE_INVALID))) {		 
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s in dhcp_dbus_set_server_veo funtion\n",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	
	owned_option.option138[0].len= 4;
	/*delete all flag*/
	owned_option.option138_num = 0;
	
	/*find set pool*/
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	
	if (!ret) {
		if (poolnode) {
			if (poolnode->refcount) {
				log_info("pool <%s> have bind to interface ,please unbind first\n", poolnode->poolname);
				op_ret = DHCP_HAVE_BIND_INTERFACE;
				goto no_option138_out;
			}
		}
		if(poolnode){
		addsubnet = poolnode->headsubnet;
		}
		if(NULL == addsubnet){
			op_ret = DHCP_SET_ORDER_WANNING;		
		}
		
		while (addsubnet) {	
			if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
				log_debug("same subnet set option43\n");
				continue;
			}
			op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);	//delete option138
			if(op_ret){
				break;
			}
			pre_subnet = addsubnet;
			addsubnet = addsubnet->next;
		}
		
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
	
no_option138_out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,  &op_ret);	

	return reply;
}

DBusMessage * 
dhcp_dbus_set_server_option138
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;	
	unsigned int	op_ret = 0, ret = 0, mode = 0, index = 0, del = 0;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	char *ipv4adr; 
	
	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_STRING, &ipv4adr,		 
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

	if (dhcp_dbus_check_ip_address(ipv4adr, NULL)) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	
	owned_option.option138_num = 1;		
	if(inet_pton(AF_INET, ipv4adr, &(owned_option.option138[0].iabuf)) != 1){
		log_error("Illegal ipv4 address for set option138\n");
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;

		reply = dbus_message_new_method_return(msg);	
		dbus_message_iter_init_append (reply, &iter);	
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32, &op_ret);	

		return reply;
	}
	owned_option.option138[0].len= 4;		
	 
	
	
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret) {
		addsubnet = poolnode->headsubnet;
		if(NULL == addsubnet){
			op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
		}
    	while (addsubnet) {
    		if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
    			pre_subnet = addsubnet;
    			addsubnet = addsubnet->next;
    			log_debug("same subnet set option138\n");
    			continue;
    		}
    		op_ret = dhcp_dbus_set_option(addsubnet, &owned_option, del);
    		if(op_ret){
    			break;
    		}
    		pre_subnet = addsubnet;
    		addsubnet = addsubnet->next;
    	}
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
out:
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32, &op_ret);	

	return reply;
}


DBusMessage * 
dhcp_dbus_set_server_dns
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int dnsnum = 1, dnsip[3] = {0, 0, 0}, del = 0;
	unsigned int op_ret = 0, ret = 0, mode = 0, index = 0, i = 0, j = 0;
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	struct iaddr tempip;
	char *value = NULL;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
				DBUS_TYPE_UINT32, &dnsnum,
				DBUS_TYPE_UINT32, &dnsip[0],
				DBUS_TYPE_UINT32, &dnsip[1],
				DBUS_TYPE_UINT32, &dnsip[2],
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

	for (i = 0; i < dnsnum; i++) {
		for (j = i+1; j < dnsnum; j++) {
			if (dnsip[i] == dnsip[j]) {
				op_ret = DHCP_SERVER_RETURN_CODE_FAIL;      
				goto out;
			}
		}
		if (dhcp_dbus_check_ipaddr_uint(dnsip[i])) {
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;      
			goto out;
		}		
	}
	
	log_debug("add dns num is %d ip is %x, %x, %x\n", dnsnum, dnsip[0], dnsip[1], dnsip[2]);	

	value = malloc(sizeof(struct iaddr) * dnsnum);
	memset(value, 0, sizeof(struct iaddr) * dnsnum);
	for (i = 0; i < dnsnum; i++) {
		memset(&tempip, 0, (sizeof (struct iaddr)));
		tempip.iabuf[0] = (dnsip[i]>>24)&0xff;		
		tempip.iabuf[1] = (dnsip[i]>>16)&0xff;
		tempip.iabuf[2] = (dnsip[i]>>8)&0xff;
		tempip.iabuf[3] = dnsip[i]&0xff;
		tempip.len = DHCP_IPV4_ADDRESS_LEN;
		memcpy(value + i*sizeof(struct iaddr), &tempip, sizeof(struct iaddr));
	}

	if (del) {
		dnsnum = 1;
	}
	owned_option.dnsnum = dnsnum;
	owned_option.dnsip = (struct iaddr *)value;
	
	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set dns\n");
					continue;
				}
				dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}
out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_set_server_routers_ip
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
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
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

	if (!del && dhcp_dbus_check_ipaddr_uint(routersip)) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	
	owned_option.routersip.iabuf[0] = (routersip>>24)&0xff;		
	owned_option.routersip.iabuf[1] = (routersip>>16)&0xff;
	owned_option.routersip.iabuf[2] = (routersip>>8)&0xff;
	owned_option.routersip.iabuf[3] = routersip&0xff;
	owned_option.routersip.len = DHCP_IPV4_ADDRESS_LEN;

	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set option43\n");
					continue;
				}
				dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}
out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_set_server_wins_ip
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
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
	struct dcli_pool* poolnode = NULL;
	struct dcli_option owned_option;
	DBusError err;

	memset(&owned_option, 0, sizeof(struct dcli_option));
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &winsip,
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

	if (!del && dhcp_dbus_check_ipaddr_uint(winsip)) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	
	owned_option.winsip.iabuf[0] = (winsip>>24)&0xff;		
	owned_option.winsip.iabuf[1] = (winsip>>16)&0xff;
	owned_option.winsip.iabuf[2] = (winsip>>8)&0xff;
	owned_option.winsip.iabuf[3] = winsip&0xff;
	owned_option.winsip.len = DHCP_IPV4_ADDRESS_LEN;

	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set option43\n");
					continue;
				}
				dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}
out:
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp_dbus_set_server_lease_default
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
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
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

	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set option43\n");
					continue;
				}
				dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp_dbus_set_server_lease_max
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
	struct dcli_subnet* addsubnet = NULL, *pre_subnet = NULL;
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
	/*sub mode*/
	if (mode) {
		ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
		if (!ret) {
			addsubnet = poolnode->headsubnet;
			if(NULL == addsubnet){
				op_ret = DHCP_SET_ORDER_WANNING;		//ought to first set range
			}
			while (addsubnet) {
				if(pre_subnet && !memcmp(&(pre_subnet->ipaddr), &(addsubnet->ipaddr), sizeof(struct iaddr))){
					pre_subnet = addsubnet;
					addsubnet = addsubnet->next;
					log_debug("same subnet set option43\n");
					continue;
				}
				dhcp_dbus_set_option(addsubnet, &owned_option, del);
				pre_subnet = addsubnet;
				addsubnet = addsubnet->next;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_POOL;
		}
	}
	else {
		dhcp_dbus_set_option(addsubnet, &owned_option, del);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;
}

DBusMessage * 
dhcp_dbus_set_nak_rsp_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
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

	dhcp_nak_rsp = enable;
	log_info("globle dhcp server nak_rsp is %s \n", (dhcp_server_enable)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_set_dhcplog_for_local7_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
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

	local7=enable;
	log_info(" dhcp server local7 is %s \n", (local7)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_auto_restart_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
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

	DHCP_AUTO_RESTART_ENABLE=enable;
	log_info(" dhcp server DHCP_AUTO_RESTART_ENABLE is %s \n", (DHCP_AUTO_RESTART_ENABLE)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_set_ASN_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
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

	ASN=enable;
	log_info(" dhcp server ASN is %s \n", (ASN)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_set_server_enable
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
	log_info("globle dhcp server is %s \n", (dhcp_server_enable)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}


DBusMessage * 
dhcp_dbus_set_dynamic_arp_enable
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
    
	dhcp_dynamic_arp_switch = enable;
	log_debug("globle dhcp dynamic arp switch is %s \n", (dhcp_dynamic_arp_switch)?"enable":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}



DBusMessage * 
dhcp_dbus_get_server_state
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
dhcp_dbus_set_server_option60_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter	 iter;
	unsigned int index = 0;
	unsigned int enable = 0;
	unsigned int op_ret = 0, ret = 0;
	struct dcli_pool *pool_node = NULL;
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
			DBUS_TYPE_UINT32, &index,
			DBUS_TYPE_UINT32, &enable,
			DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	ret = dhcp_dbus_find_pool_by_index(index, &pool_node);	/* 0 : find */
	if (0 == ret) {
		pool_node->owned_option.option60_enable = enable;
		log_info("set pool %s option60 %s\n", pool_node->poolname, (enable ? "enable" : "disable"));
	} else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	return reply;	
}
unsigned int dhcp_dbus_delete_vendor_class_identifier(struct dcli_pool *pool_node, char *id)
{
	int i = 0;
	int count = 0;
	struct dcli_option60 option60[DHCP_OPTION60_DEFAULT_NUMBER];
	if ((NULL == pool_node) || (NULL == id)) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}
	if ((NULL == pool_node->owned_option.option60) || (pool_node->owned_option.option60_cnt < 1)) {
		return DHCP_SERVER_RETURN_CODE_FAIL;		
	}	
	memcpy((char *)option60, (char *)pool_node->owned_option.option60, sizeof(option60));
	memset((char *)pool_node->owned_option.option60, 0, sizeof(struct dcli_option60) * DHCP_OPTION60_DEFAULT_NUMBER);
	for (i=0; i<pool_node->owned_option.option60_cnt; i++) {
		if (strlen(option60[i].id) != strlen(id)) {
			memcpy(pool_node->owned_option.option60[count].id, &option60[i].id, DHCP_OPTION60_ID_DEFAULT_LENGTH);
			count++;
			continue;
		}
		if (0 == memcmp(option60[i].id, id, strlen(id))) {
			continue;
		} else {
			memcpy(pool_node->owned_option.option60[count].id, option60[i].id, sizeof(struct dcli_option60));
			count++;
		}
	}
	if (count == pool_node->owned_option.option60_cnt) {
		log_debug("pool %s delete VCI failed, not find id %s\n", pool_node->poolname, id);
		return DHCP_SERVER_RETURN_CODE_FAIL;	
	}else {
		pool_node->owned_option.option60_cnt = count;
		if ((0 == count) && (pool_node->owned_option.option60)) {
			free(pool_node->owned_option.option60);
			pool_node->owned_option.option60 = NULL;			
		}
		return 0;
	}
}
unsigned int dhcp_dbus_add_vendor_class_identifier(struct dcli_pool *pool_node, char *id)
{
	int i = 0;
	if ((NULL == pool_node) || (NULL == id)) {
		return DHCP_SERVER_RETURN_CODE_FAIL;
	}
	if (NULL == pool_node->owned_option.option60) {
		pool_node->owned_option.option60 = (struct dcli_option60 *)malloc(sizeof(struct dcli_option60) * DHCP_OPTION60_DEFAULT_NUMBER);
		if (NULL == pool_node->owned_option.option60) {
			log_error("%s: malloc failed\n", __func__);
			return DHCP_SERVER_RETURN_CODE_FAIL;
		}
		memset((char *)pool_node->owned_option.option60, 0, sizeof(struct dcli_option60) * DHCP_OPTION60_DEFAULT_NUMBER);
		memcpy(pool_node->owned_option.option60[0].id, id, strlen(id));
		pool_node->owned_option.option60_cnt++;		
	} else {
		if (pool_node->owned_option.option60_cnt >= DHCP_OPTION60_DEFAULT_NUMBER) {
			return DHCP_MAX_OPTION_LIST_WANNING; 		/* max add 4 VCI */
		}
		for (i=0; i< pool_node->owned_option.option60_cnt; i++) {
			if (strlen(pool_node->owned_option.option60[i].id) != strlen(id)) {
				continue;
			}
			if (0 == memcmp(pool_node->owned_option.option60[i].id, id, strlen(id))) {
				return DHCP_ALREADY_ADD_OPTION;
			}
		}
		memcpy(&(pool_node->owned_option.option60[pool_node->owned_option.option60_cnt].id), id, strlen(id));
		pool_node->owned_option.option60_cnt++;
	}
	return 0;
}
DBusMessage * 
dhcp_dbus_set_server_option60_set_id
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int add_flag = DHCP_OPTION60_INVALID_ID;
	unsigned int index = 0;
	unsigned int op_ret = 0;
	unsigned int ret = 0;
	char *id = NULL;
	struct dcli_pool *pool_node = NULL;
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
			DBUS_TYPE_UINT32, &add_flag,
			DBUS_TYPE_UINT32, &index,							 
			DBUS_TYPE_STRING, &id,
			DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if (strlen(id) > DHCP_OPTION60_ID_DEFAULT_LENGTH - 1) {
		log_debug("VCI is too long\n");
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	ret = dhcp_dbus_find_pool_by_index(index, &pool_node);
	if (0 == ret) {		
		if (DHCP_OPTION60_ADD_ID == add_flag) {
			op_ret = dhcp_dbus_add_vendor_class_identifier(pool_node, id);
			log_debug("pool %s add option60 \"%s\" %s\n", pool_node->poolname, id, op_ret ? "failed" : "success");
		} else if (DHCP_OPTION60_DELETE_ID == add_flag){
			op_ret = dhcp_dbus_delete_vendor_class_identifier(pool_node, id);
			log_debug("pool %s delete option60 \"%s\" %s\n", pool_node->poolname, id, op_ret ? "failed" : "success");
		} else {
			log_debug("set dhcp option60: unkown add flag %d\n", add_flag);
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		}		
	} else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}
	out:
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	return reply;
}

DBusMessage * 
dhcp_dbus_set_server_static_arp_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
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

	dhcp_server_static_arp_enable = enable;
	log_debug("globle dhcp server set static arp is %s \n", (dhcp_server_static_arp_enable)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}

DBusMessage * 
dhcp_dbus_add_dhcp_pool_ip_range
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
	unsigned int	op_ret = 0, ret = 0, index = 0, total_host = 0;
	unsigned int ipaddrl = 0, ipaddrh = 0, ipmask = 0, add = 0;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &add,
		DBUS_TYPE_UINT32, &ipaddrl,	
		DBUS_TYPE_UINT32, &ipaddrh,
		DBUS_TYPE_UINT32, &ipmask,
		DBUS_TYPE_UINT32, &index,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (dhcp_dbus_check_ipaddr_uint(ipaddrl)
		|| dhcp_dbus_check_ipaddr_uint(ipaddrh)
		|| dhcp_dbus_check_mask(ipmask)
		|| ((ipaddrl & ipmask) != (ipaddrh & ipmask))) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;      
		goto dbus_return;
	}
	
	/* find return 0 */
	ret = dhcp_dbus_find_pool_by_index(index, &poolNode);
	if(!ret) {
		log_debug("get name succee name is %s\n", poolNode->poolname);
		tempsub = (struct dcli_subnet*)malloc(sizeof(struct dcli_subnet));
		memset(tempsub, 0, sizeof(struct dcli_subnet));

		tempsub->highip.iabuf[0] = (ipaddrh>>24)&0xff;		
		tempsub->highip.iabuf[1] = (ipaddrh>>16)&0xff;
		tempsub->highip.iabuf[2] = (ipaddrh>>8)&0xff;
		tempsub->highip.iabuf[3] = ipaddrh&0xff;
		tempsub->highip.len = DHCP_IPV4_ADDRESS_LEN;
		log_debug("addsubnet->highip.iabuf %u.%u.%u.%u\n",
			tempsub->highip.iabuf[0],tempsub->highip.iabuf[1],
			tempsub->highip.iabuf[2],tempsub->highip.iabuf[3]);
		
		tempsub->lowip.iabuf[0] = (ipaddrl>>24)&0xff;		
		tempsub->lowip.iabuf[1] = (ipaddrl>>16)&0xff;
		tempsub->lowip.iabuf[2] = (ipaddrl>>8)&0xff;
		tempsub->lowip.iabuf[3] = ipaddrl&0xff;
		tempsub->lowip.len = DHCP_IPV4_ADDRESS_LEN;
		log_debug("addsubnet->lowip.iabuf %u.%u.%u.%u\n",
			tempsub->lowip.iabuf[0],tempsub->lowip.iabuf[1],
			tempsub->lowip.iabuf[2],tempsub->lowip.iabuf[3]);
		
		tempsub->mask.iabuf[0] = (ipmask>>24)&0xff;		
		tempsub->mask.iabuf[1] = (ipmask>>16)&0xff;
		tempsub->mask.iabuf[2] = (ipmask>>8)&0xff;
		tempsub->mask.iabuf[3] = ipmask&0xff;
		tempsub->mask.len = DHCP_IPV4_ADDRESS_LEN;
		log_debug("addsubnet->mask.iabuf %u.%u.%u.%u\n",
			tempsub->mask.iabuf[0],tempsub->mask.iabuf[1],
			tempsub->mask.iabuf[2],tempsub->mask.iabuf[3]);

		tempsub->ipaddr.iabuf[0] = (ipaddrl>>24)&((ipmask>>24)&0xff);		
		tempsub->ipaddr.iabuf[1] = (ipaddrl>>16)&((ipmask>>16)&0xff);
		tempsub->ipaddr.iabuf[2] = (ipaddrl>>8)&((ipmask>>8)&0xff);
		tempsub->ipaddr.iabuf[3] = ipaddrl&(ipmask&0xff);
		tempsub->ipaddr.len = DHCP_IPV4_ADDRESS_LEN;
		log_debug("addsubnet->ipaddr.iabuf %u.%u.%u.%u\n",
			tempsub->ipaddr.iabuf[0],tempsub->ipaddr.iabuf[1],
			tempsub->ipaddr.iabuf[2],tempsub->ipaddr.iabuf[3]);	

		if(poolNode->refcount){
			if(tempsub){
				free(tempsub);
				tempsub=NULL;
			}
			op_ret = DHCP_UNBIND_BY_INTERFACE;
			log_error("binded by interface can't del or add\n");
			goto dbus_return;
		}

		if (!add) {		
			
			total_host = get_total_host_num();
			if (((ipaddrh - ipaddrl) + total_host) <= DHCP_MAX_HOST_NUM) {

				ret = dhcp_debus_is_same_subnet_exist(tempsub, poolNode->poolname);
				if (1 == ret) {
					free(tempsub);
					op_ret = DHCP_SUBNET_EXIST;

					goto dbus_return;					
				}

				/* whether same subnet */
				ret = dhcp_dbus_whether_same_subnet(tempsub, poolNode->headsubnet);
				if (0 == ret) {
					free(tempsub);
					op_ret = DHCP_NOT_THE_SUBNET;
					goto dbus_return;					
				}
				
				ret	= dhcp_dbus_check_pool_ip(tempsub); 
				if (ret) {
					/*
					ret = dhcp_dbus_check_interface(&tempsub->lowip, &tempsub->mask, &ifname, &ifnum);
					log_debug("end for check interace \n");
					tempsub->interface_name = ifname;
					tempsub->interface_num = ifnum;
					*/	

					ret = dhcp_dbus_set_subnet(tempsub);
					if (!ret){						
						/*add for list member*/							
						if (!poolNode->headsubnet) {
							poolNode->headsubnet = tempsub;
							log_debug(" poolNode headsubnet lowip is %s , high ip is %s\n", piaddr(tempsub->lowip), piaddr(tempsub->highip));
							
						}else{										
							tempsub->next = poolNode->headsubnet;
							poolNode->headsubnet = tempsub;						
						}
						tempsub->ip_num = ipaddrh - ipaddrl + 1;
						poolNode->count_sub++;
						tempsub->attribute_pool = poolNode;
						/*prevent the loss of option when delete pool range*/
						if(dhcp_dbus_set_option(tempsub, &(poolNode->owned_option), 0)){
							log_debug("pool %s already set option\n", poolNode->poolname);
						}						
					}
					else {			
						log_error("bad ip address for dhcp range \n");					
						free(tempsub);
						op_ret = DHCP_INCORRET_IP_ADDRESS;
					}
				}
				else {				
					free(tempsub);
					log_error("add range fail\n");
					op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
				}
			}
			else {
				free(tempsub);
				op_ret = DHCP_TOO_MANY_HOST_NUM;
				log_error("total host num must little than %u \n", DHCP_MAX_HOST_NUM);
				log_error("now total num is %u, add is num is %u\n", total_host, (ipaddrh - ipaddrl));
			}	
			
		}
		else {									
			log_debug("delete subnet \n");
			if (!poolNode->headsubnet) {
				if(tempsub){
				free(tempsub);
				tempsub=NULL;
				}
				op_ret = DHCP_POOL_SUBNET_NULL;
				log_error("delet subnet fail poolNode has no subnet \n");
			}
			else if (poolNode->refcount) {
				if(tempsub){
				free(tempsub);
				tempsub=NULL;
				}
				op_ret = DHCP_UNBIND_BY_INTERFACE;
				log_error("binded by interface can no del range\n");
			}
			else {
				/*for add option43 when delete range after add range*/
				poolNode->owned_option.option43.type = 0;

				ret = dhcp_dbus_delete_subnet_by_subip(tempsub, poolNode);
				if (ret) {	
					op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
				}
				free(tempsub);
			}
		}
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;      
	}
	
dbus_return:
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	return reply;
	
}
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, char *cid, unsigned int *port){
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);//regard the ve interface name as right
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return -1;
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return -1;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			
			return -1;
			}
	}
	
		return -1;
}
	

int check_ve_interface(char *ifname ,char *name){
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int port = 0;
	char cpu = 'f';
	//char *cpu_id = &cpu;
	struct ifreq ifr;
	int ret = 0;
	
	if (0 != strncmp(ifname, "ve", 2)) {
		log_debug("It's not ve interface\n");
		sprintf(name,"%s",ifname);
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd<0){
			log_debug("sockfd failed!\n");
			return -1;
		}
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 	
		ret = parse_int_ve(ifname+2,&slotid,&vlanid,&cpu,&port);
		
		if(1 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid = %d\n",vlanid);

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");

				//convert to new ve name
				if(slotid < 10)
					sprintf(name,"ve0%df1.%d",slotid,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%df1.%d",slotid,vlanid);
				log_debug("ve name is %s\n",name);

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					log_debug("SIOCGIFINDEX error\n");		
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}	
				else{
					close(sockfd);
				    return 0;	//the new ve interface exists
				}
			}
			else{
				sprintf(name,"%s",ifname);
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(2 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid=%d\n",vlanid);
			log_debug("ifname=%s\n",ifname);
			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,cpu,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,cpu,port);
			}
			else if(vlanid > 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d.%d",slotid,cpu,port,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d.%d",slotid,cpu,port,vlanid);
			}
			
			log_debug("name=%s\n",name);
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		
			else{
				//sprintf(name,"%s",ifname);
			    close(sockfd);
			    return 0;	//the new ve interface exists
		 	}
		}
		else{
			log_debug("the ve name is wrong\n");
			sprintf(name,"%s",ifname);
			close(sockfd);
			return -1;
		}
	}
}

DBusMessage * 
dhcp_dbus_add_dhcp_static_host
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
	char *ifname = NULL, *tmpname = NULL;
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
							DBUS_TYPE_STRING, &ifname, 
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

	if (dhcp_dbus_check_ipaddr_uint(ipaddr) || !if_nametoindex(ifname)) {
		op_ret = DHCP_SERVER_RETURN_CODE_FAIL;      
		goto out;
	}

	memset(&addhost, 0, sizeof(struct dcli_host));
	
	tmpname = malloc(strlen(ifname) + 1);
	memset(tmpname, 0, (strlen(ifname)+ 1));
	memcpy(tmpname, ifname, strlen(ifname));
	addhost.ifname = tmpname;

	addhost.hostip.iabuf[0] = (ipaddr>>24)&0xff;
	addhost.hostip.iabuf[1] = (ipaddr>>16)&0xff;
	addhost.hostip.iabuf[2] = (ipaddr>>8)&0xff;
	addhost.hostip.iabuf[3] = ipaddr&0xff;
	addhost.hostip.len = DHCP_IPV4_ADDRESS_LEN;	
	addhost.hostmac[0] = mac[0];
	addhost.hostmac[1] = mac[1];
	addhost.hostmac[2] = mac[2];
	addhost.hostmac[3] = mac[3];
	addhost.hostmac[4] = mac[4];
	addhost.hostmac[5] = mac[5];

	/*add static host*/
	if (add) {
		ret = dhcp_dbus_find_static_host_by_addr(&(addhost.hostip), mac, &temphost);
		if (ret) {
			sprintf(hostname, "host%d", ++host_index);
			addhost.hostname = hostname;
			ret = dhcp_dbus_set_host(&addhost, 1);
			if (!ret) {
				dhcp_dbus_create_static_host_by_name(hostname, &temphost);
				memcpy(&(temphost->hostip), &(addhost.hostip), sizeof(struct iaddr));
				memcpy(temphost->hostmac, mac, DHCP_MAC_ADDRESS_LEN);
				temphost->ifname = tmpname;
				log_debug("finish for add static host name %s\n", temphost->hostname);
			}
			else {
				log_error("add static host fail in dhcp dbus\n");
				op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
			}
		}
		else {
			op_ret = DHCP_HAVE_CONTAIN_STATIC_USER;
		}
	}
	/*delete static host*/
	else {
		ret = dhcp_dbus_find_static_host_by_addr_all(&(addhost.hostip), mac, tmpname, &temphost);
		if (!ret) {
			ret = dhcp_dbus_set_host(temphost, 0);
			if (!ret) {
				dhcp_dbus_delete_static_host_by_addr(&(temphost->hostip), (char *)temphost->hostmac);
				count_host--;
			}
			else {
				log_error("del static host fail in dhcp dbus\n");
				op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
			}
		}
		else {
			op_ret = DHCP_NOT_FOUND_STATIC_USER;
		}
	}
out:	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	return reply;
	
}

DBusMessage * 
dhcp_dbus_add_dhcp_failover_peer
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int primary = 0, split = 0, mclt = 0, dstip = 0, dstport = 0, srcip = 0, srcport = 0;
	unsigned int mode = 0, index = 0, ret = 0, op_ret = 0;
	char *failname = NULL;
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;
	struct failover_backup_list* backup = NULL;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_UINT32, &index,								
								DBUS_TYPE_STRING, &failname,
								DBUS_TYPE_UINT32, &primary,
								DBUS_TYPE_UINT32, &split,
								DBUS_TYPE_UINT32, &mclt,
								DBUS_TYPE_UINT32, &dstip,
								DBUS_TYPE_UINT32, &dstport,
								DBUS_TYPE_UINT32, &srcip,
								DBUS_TYPE_UINT32, &srcport,
								DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
		
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret && poolnode->headsubnet) {
		subnode = poolnode->headsubnet;		
		if (!(subnode->owned_failover.enable)) {
			memcpy(subnode->owned_failover.name, failname, strlen(failname));
			subnode->owned_failover.primary = primary;
			subnode->owned_failover.split = split;
			subnode->owned_failover.mclt = mclt;
			subnode->owned_failover.myport = srcport;
			subnode->owned_failover.myaddr.iabuf[0] = (srcip>>24)&0xff;
			subnode->owned_failover.myaddr.iabuf[1] = (srcip>>16)&0xff;
			subnode->owned_failover.myaddr.iabuf[2] = (srcip>>8)&0xff;
			subnode->owned_failover.myaddr.iabuf[3] = srcip&0xff;
			subnode->owned_failover.myaddr.len = DHCP_IPV4_ADDRESS_LEN;
			subnode->owned_failover.parteraddr.iabuf[0] = (dstip>>24)&0xff;
			subnode->owned_failover.parteraddr.iabuf[1] = (dstip>>16)&0xff;
			subnode->owned_failover.parteraddr.iabuf[2] = (dstip>>8)&0xff;
			subnode->owned_failover.parteraddr.iabuf[3] = dstip&0xff;
			subnode->owned_failover.parteraddr.len = DHCP_IPV4_ADDRESS_LEN;
			subnode->owned_failover.parterport = dstport;

			ret = dhcp_dbus_set_sub_failover(subnode);
			if (!ret) {
				subnode->owned_failover.enable = 1;				
				backup = malloc(sizeof(struct failover_backup_list));
				memset(backup, 0, sizeof(struct failover_backup_list));
				/*if is secondary add it in backup list, do not work*/
				if (primary) {
					log_debug("add secondary to backup list, name is %s \n", failname);
					add_interface_failover_to_backup_list(subnode);
				}
			}
			else {
				memset(&(subnode->owned_failover), 0, sizeof(struct dcli_failover));
			}
		}
		else {
			log_error("sub failover is exist \n");
			op_ret = DHCP_FAILOVER_HAVE_EXIST;
		}
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	return reply;	

}

DBusMessage * 
dhcp_dbus_cfg_dhcp_failover_peer
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int primary = 0;
	unsigned int mode = 0, index = 0, ret = 0, op_ret = 0;
	char *failname = NULL;
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;
	struct failover_backup_list *backnode = NULL;
	struct dcli_failover *failnode = NULL;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_UINT32, &index,								
								DBUS_TYPE_STRING, &failname,
								DBUS_TYPE_UINT32, &primary,
								DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
		
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret && poolnode->headsubnet) {
		/*only one subnet one failover*/
		subnode = poolnode->headsubnet;

		if (subnode->owned_failover.enable) {
			if (subnode->owned_failover.primary != primary) {
				failnode = &(subnode->owned_failover);
				ret = find_backup_list_by_failnode(failnode, &backnode);
				if (!ret) {
					subnode->owned_failover.primary = primary;
					backnode->state = primary;
				}
			}
			else {
				log_error("failover state change error same primary \n");
				op_ret = DHCP_SAME_PRIMARY;
			}
		}
		else {
			log_error("failover state change error not enable\n");
			op_ret = DHCP_FAILOVER_NOT_ENABLE;
		}
	
	}
	else {
		op_ret = DHCP_NOT_FOUND_POOL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	return reply;	

}

DBusMessage * 
dhcp_dbus_del_dhcp_failover_peer
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int mode = 0, index = 0, ret = 0, op_ret = 0;
	char *failname = NULL;
	struct dcli_subnet* subnode = NULL;
	struct dcli_pool* poolnode = NULL;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_UINT32, &index,								
								DBUS_TYPE_STRING, &failname,
								DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
		
	ret = dhcp_dbus_find_pool_by_index(index, &poolnode);
	if (!ret && poolnode->headsubnet && (0 == strncmp(poolnode->headsubnet->owned_failover.name,failname,strlen(failname)))) {
		/*only one subnet one failover*/
		subnode = poolnode->headsubnet;

		if (subnode->owned_failover.enable) {
			ret = dhcp_dbus_del_sub_failover(subnode);
			if (ret) {
				op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
				log_error("delete failover state fail \n");
			}
		}
		else {
			log_error("failover state delete error not enable\n");
			op_ret = DHCP_FAILOVER_NOT_ENABLE;
		}
	}
	else {
		if(ret)
			op_ret = DHCP_NOT_FOUND_POOL;
		else
			op_ret = DHCP_FAILOVER_NAME_WRONG;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	
	return reply;	
}


int dhcp_set_interace_vrrp_state(char *ifname, unsigned int state)
{
	struct interface_info *tmp = NULL;
	for (tmp = interfaces; tmp; tmp = tmp->next) {
		if (0 == strcmp(ifname, tmp->name)) {
			if (VRRP_STATE_MAST == state) { 
				tmp->vrrp_state_flag = vrrp_state_master;
				log_info("interface %s set vrrp state master\n", tmp->name); 
			} else {
				tmp->vrrp_state_flag = vrrp_state_secondary;
				log_debug("interface %s set vrrp state secondary, donot process dhcp request!\n", tmp->name); 
			}
			return 0;
		}
	}
	return 1;
}


int dhcp_dbus_disable_secondary_dhcp_server(vrrp_param_t *params)
{
	int i = 0;
	if (NULL == params) {
		log_error("set interface flag params is null\n");
		return 1;
	}
	for (i = 0; i < params->downlink_cnt; i++) {
		dhcp_set_interace_vrrp_state(params->downlink_interface[i].if_name, params->state);
	}
	for (i = 0; i < params->vgateway_cnt; i++) {
		dhcp_set_interace_vrrp_state(params->vgateway_interface[i].if_name, params->state);
	}
	return 0;
}


int dhcp_failover_state_change(int sockfd)
{
	vrrp_param_t params;

	if (sockfd < 0) {
		log_error("dhcp failover state change failed: invalid sockfd %d.\n", sockfd);
		return -1;
	}
	
	memset(&params, 0, sizeof(params));
	/* receive msg */
	if (recvfrom(sockfd, &params, sizeof(params), MSG_DONTWAIT, NULL, NULL) <= 0) {
		log_error("dhcp failover state change failed: %m\n");
		return -1;
	}

	log_info("receive msg(vrrp %d state %d) do failover.\n", params.vrid, params.state);
	/* do failover */
	dhcp_dbus_do_failover(&params);

	return 0;
}

int dhcp_dbus_failover_sndmsg(vrrp_param_t *msg)
{
	static int sockfd = -1;
	int size = 0;

	if (!msg) {
		log_error("do failover failed: parameter is NULL!\n");
		return -1;
	}

	if (sockfd < 0) {
		sockfd = init_sock_unix_client();
		if (sockfd < 0) {
			log_error("dhcp failover sndmsg: init unix socket failed\n");
			return -1;
		}
		struct sockaddr_un servaddr;
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sun_family = AF_UNIX;
		strcpy((void*)&servaddr.sun_path, DHCP_FAILOVER_SOCK_PATH);
		/* connect */
		if (connect(sockfd, (struct sockaddr *)&servaddr,  sizeof(servaddr))) {
			log_error("dhcp failover connect failed: %m");
			close(sockfd);
			sockfd = -1;
			return -1;
		}
	}

//	size = sendto(sockfd, msg, sizeof(*msg), MSG_DONTWAIT, (struct sockaddr *)(&servaddr), sizeof(servaddr));

	size = sendto(sockfd, msg, sizeof(*msg), MSG_DONTWAIT, NULL, 0);
	if (size != sizeof(*msg)) {
		log_error("send msg to failover faild(msg size %d, send size %d):%m\n", sizeof(*msg), size);
		return -1;
	}

	return 0;
}

int dhcp_dbus_do_failover(vrrp_param_t *params)
{
	char failname[MAX_IF_NAME_LEN];
	struct dcli_subnet* subnode = NULL;
	struct failover_backup_list* backup = NULL;
	unsigned int ret = 0, op_ret = 0, j = 0, primary = 0, if_count = 0;

	flag_shutdown = 1;
	if (!params) {
		log_error("do failover failed: parameter is NULL!\n");
		return -1;
	}
	
	if (VRRP_STATE_MAST != params->state) {
		primary = 1;/*1 is secondary*/
	}
	else {
		primary = 0;/*0 is primary*/
	}
	
	ret = find_backup_list_by_vrrpid(params->vrid, &backup);
	if (ret && (VRRP_STATE_DISABLE != params->state) ) {/*can not find the back node for this vrrpid, add it*/
		backup = malloc(sizeof(struct failover_backup_list));
		if (NULL == backup) {
			log_error("malloc failover %d node failed\n", params->vrid);
			return -1;
		}
		memset(backup, 0, sizeof(struct failover_backup_list));
		for (j = 0; j < (params->downlink_cnt + params->vgateway_cnt); j++) {
			/*new one if_name one subnode*/
			if (j < params->downlink_cnt) {	
				log_debug_failover(DEBUG_TYPE_CONNECT,"downlink interface name is %s, vrrpid is %d, \n", params->downlink_interface[j].if_name, params->vrid);
				ret = dhcp_dbus_find_subnet_by_ifname(params->downlink_interface[j].if_name, &subnode);
			}
			else {				
				log_debug_failover(DEBUG_TYPE_CONNECT,"vgateway interface name is %s, vrrpid is %d\n", params->vgateway_interface[j - params->downlink_cnt].if_name, params->vrid);
				ret = dhcp_dbus_find_subnet_by_ifname(params->vgateway_interface[j - params->downlink_cnt].if_name, &subnode);
			}
			if (!ret) {
				if(subnode->owned_failover.enable){
					/*if you delete one failover in some interfaces ,other interfaces can not add in backup list
					   when use "no failover peername"
					   if one interface not only  is downlink interface but also act as vgateway interface ,it will add twice in back up
					*/
					ret = find_interface_failover_to_backup_list(subnode);
					if (ret) {
						log_debug_failover(DEBUG_TYPE_CONNECT,"not fing interface in backup_list \n");
						if (j < params->downlink_cnt) {	
							memcpy(backup->dhcp_interface[if_count].if_name, params->downlink_interface[j].if_name, strlen(params->downlink_interface[j].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup downlink interface name is %s, vrrpid is %d, count %d\n", params->downlink_interface[j].if_name, params->vrid, if_count);
						}
						else {				
							memcpy(backup->dhcp_interface[if_count].if_name, params->vgateway_interface[j - params->downlink_cnt].if_name, strlen(params->vgateway_interface[j - params->downlink_cnt].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup vgateway interface name is %s, vrrpid is %d, count %d\n", params->vgateway_interface[j - params->downlink_cnt].if_name, params->vrid, if_count);
						}
						/*}*/
						if_count ++;
					}
				}
				if (!(subnode->owned_failover.enable)) {
					//sprintf(failname, "peer%d", (params.vrid)*100 + j);
					sprintf(failname, "peer%d", params->vrid);
					log_debug_failover(DEBUG_TYPE_CONNECT,"add new failover state name is %s\n", failname);
					memcpy(subnode->owned_failover.name, failname, strlen(failname));

					if(!primary){
						subnode->owned_failover.split = 128;
						subnode->owned_failover.mclt = 10;
					}

					/*add by sunjc@autelan.com*/
					if(params->heartlink_local_ip > params->heartlink_opposite_ip){				
						subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params->vrid)*16 ;
						subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params->vrid)*16 + 100;																			
					}else {							
						subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params->vrid)*16  + 100;
						subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params->vrid)*16 ;									
					}
					subnode->owned_failover.primary = primary;/*0 is primary 1 is secondary*/	
					subnode->owned_failover.myaddr.iabuf[0] = ((params->heartlink_local_ip)>>24)&0xff;
					subnode->owned_failover.myaddr.iabuf[1] = ((params->heartlink_local_ip)>>16)&0xff;
					subnode->owned_failover.myaddr.iabuf[2] = ((params->heartlink_local_ip)>>8)&0xff;
					subnode->owned_failover.myaddr.iabuf[3] = (params->heartlink_local_ip)&0xff;
					subnode->owned_failover.myaddr.len = DHCP_IPV4_ADDRESS_LEN;
					subnode->owned_failover.parteraddr.iabuf[0] = ((params->heartlink_opposite_ip)>>24)&0xff;
					subnode->owned_failover.parteraddr.iabuf[1] = ((params->heartlink_opposite_ip)>>16)&0xff;
					subnode->owned_failover.parteraddr.iabuf[2] = ((params->heartlink_opposite_ip)>>8)&0xff;
					subnode->owned_failover.parteraddr.iabuf[3] = (params->heartlink_opposite_ip)&0xff;
					subnode->owned_failover.parteraddr.len = DHCP_IPV4_ADDRESS_LEN;

					ret = dhcp_dbus_set_sub_failover(subnode);
					//sleep(1);
					if (!ret) {
						subnode->owned_failover.enable = 1;
						/*if is secondary add it in backup list, do not work
						if (subnode->owned_failover.primary) {*/
						if (j < params->downlink_cnt) {	
							memcpy(backup->dhcp_interface[if_count].if_name, params->downlink_interface[j].if_name, strlen(params->downlink_interface[j].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup downlink interface name is %s, vrrpid is %d, count %d\n", params->downlink_interface[j].if_name, params->vrid, if_count);
						}
						else {				
							memcpy(backup->dhcp_interface[if_count].if_name, params->vgateway_interface[j - params->downlink_cnt].if_name, strlen(params->vgateway_interface[j - params->downlink_cnt].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup vgateway interface name is %s, vrrpid is %d, count %d\n", params->vgateway_interface[j - params->downlink_cnt].if_name, params->vrid, if_count);
						}
						/*}*/
						if_count ++;
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
		
		backup->interface_num = if_count;

		if (backup->interface_num) {
			log_debug_failover(DEBUG_TYPE_CONNECT,"add %d failover state\n", backup->interface_num);
			backup->vrrpid = params->vrid;
			backup->state = primary;
			backup->next = failover_backup_list_head.next;
			failover_backup_list_head.next = backup;
		}else{
			if(backup){
				free(backup);
				backup=NULL;
			}
		}
	}
	else {	/*state change*/
		for (j = 0; j < (params->downlink_cnt + params->vgateway_cnt); j++) {
			if (j < params->downlink_cnt) {	
				log_debug_failover(DEBUG_TYPE_CONNECT,"downlink interface is %s, vrrpid is %d\n", params->downlink_interface[j].if_name, params->vrid);
				ret = dhcp_dbus_find_subnet_by_ifname(params->downlink_interface[j].if_name, &subnode);
			}
			else {				
				log_debug_failover(DEBUG_TYPE_CONNECT,"vgateway interface is %s, vrrpid is %d\n", params->vgateway_interface[j - params->downlink_cnt].if_name, params->vrid);
				ret = dhcp_dbus_find_subnet_by_ifname(params->vgateway_interface[j - params->downlink_cnt].if_name, &subnode);
			}
			
			if (!ret) {
				if(backup){
				if (!(subnode->owned_failover.enable)){
					//sprintf(failname, "peer%d", (params.vrid)*100 + j);
					sprintf(failname, "peer%d", params->vrid);
					log_debug_failover(DEBUG_TYPE_CONNECT,"add new failover state name is %s\n", failname);
					memcpy(subnode->owned_failover.name, failname, strlen(failname));

					if(!primary){
						subnode->owned_failover.split = 128;
						subnode->owned_failover.mclt = 10;
					}

					/*add by sunjc@autelan.com*/
					if(params->heartlink_local_ip > params->heartlink_opposite_ip){				
						subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params->vrid)*16 ;
						subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params->vrid)*16 + 100;																			
					}else {							
						subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params->vrid)*16  + 100;
						subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params->vrid)*16 ;									
					}
					subnode->owned_failover.primary = primary;/*0 is primary 1 is secondary*/	
					subnode->owned_failover.myaddr.iabuf[0] = ((params->heartlink_local_ip)>>24)&0xff;
					subnode->owned_failover.myaddr.iabuf[1] = ((params->heartlink_local_ip)>>16)&0xff;
					subnode->owned_failover.myaddr.iabuf[2] = ((params->heartlink_local_ip)>>8)&0xff;
					subnode->owned_failover.myaddr.iabuf[3] = (params->heartlink_local_ip)&0xff;
					subnode->owned_failover.myaddr.len = DHCP_IPV4_ADDRESS_LEN;
					subnode->owned_failover.parteraddr.iabuf[0] = ((params->heartlink_opposite_ip)>>24)&0xff;
					subnode->owned_failover.parteraddr.iabuf[1] = ((params->heartlink_opposite_ip)>>16)&0xff;
					subnode->owned_failover.parteraddr.iabuf[2] = ((params->heartlink_opposite_ip)>>8)&0xff;
					subnode->owned_failover.parteraddr.iabuf[3] = (params->heartlink_opposite_ip)&0xff;
					subnode->owned_failover.parteraddr.len = DHCP_IPV4_ADDRESS_LEN;

					ret = dhcp_dbus_set_sub_failover_add_interface(subnode);
					//sleep(1);
					if (!ret) {
						subnode->owned_failover.enable = 1;
						/*if is secondary add it in backup list, do not work
						if (subnode->owned_failover.primary) {*/
						if (j < params->downlink_cnt) {	
							memcpy(backup->dhcp_interface[backup->interface_num].if_name, params->downlink_interface[j].if_name, strlen(params->downlink_interface[j].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup downlink interface name is %s, vrrpid is %d, count %d\n", params->downlink_interface[j].if_name, params->vrid, backup->interface_num);
						}
						else {	

							
							memcpy(backup->dhcp_interface[backup->interface_num].if_name, params->vgateway_interface[j - params->downlink_cnt].if_name, strlen(params->vgateway_interface[j - params->downlink_cnt].if_name));
							log_debug_failover(DEBUG_TYPE_CONNECT,"add backup vgateway interface name is %s, vrrpid is %d, count %d\n", params->vgateway_interface[j - params->downlink_cnt].if_name, params->vrid, backup->interface_num);
						}
						/*}*/
						
					}
					else {
						memset(&(subnode->owned_failover), 0, sizeof(struct dcli_failover));
					}
					backup->interface_num++;
					dhcp_failover_generate_update_queue_add_pool(1,subnode);
				}
				}
				/*
				log_info("subnode->owned_failover.primary is %d, primary is %d\n", 
						subnode->owned_failover.primary, primary);
				*/
				if (backup) {
					subnode->owned_failover.primary = primary;
					backup->state = primary;
					if(subnode->owned_failover.primary){
						subnode->owned_failover.mclt = 0;
						subnode->owned_failover.split = 0;
					}else{
						subnode->owned_failover.mclt = 10;
						subnode->owned_failover.split = 128;
					}
					dhcp_dbus_change_sub_failover(subnode);	
				}
				else {
					log_error("failover state change error same state %s\n", primary ? "secondary" : "primary");
					op_ret = 2;
				}
			}
			else {
				log_error("when change state can not find subnet by the interface\n");
			}				
		}			
	}
	return 0;
}
/*state init or change */
DBusMessage *
dhcp_dbus_failover_recv_vrrp_state
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
	unsigned int op_ret = 0, j = 0, goodip = 0;
	unsigned int failover_flag = 0;	
	int vip_nu = 0;
	
	char master_ip_char[32]="";
	char backup_ip_char[32]="";
	char virtual_ip_char[32]="";
	char * interface_name = NULL;
	//struct failover_backup_list* next = NULL;
    //int if_idx = 0;
    TIME start = 0;
    TIME end = 0;
	

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
	log_debug_failover(DEBUG_TYPE_CONNECT,"--------------- HA instance info ---------------");

	dbus_message_iter_init(msg,&iter);
	dbus_message_iter_get_basic(&iter, &failover_flag);	
	log_debug_failover(DEBUG_TYPE_CONNECT,"hansi notifier dhcp %s config dhcp failover\n", (failover_flag) ? "" : "not");
	params.failover_flag = failover_flag;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.vrid));

 	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.state));
	
	log_debug_failover(DEBUG_TYPE_CONNECT,"...vrid %d state %s", params.vrid, pha_state2str(params.state));

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.uplink_cnt));
	
	log_debug_failover(DEBUG_TYPE_CONNECT,"...uplink num %d", params.uplink_cnt);

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
		
		log_debug_failover(DEBUG_TYPE_CONNECT,"...uplink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(params.uplink_interface[j].master_ip, master_ip_char, sizeof(master_ip_char) ),
					inet_int2str(params.uplink_interface[j].backp_ip, backup_ip_char, sizeof(backup_ip_char) ),
					inet_int2str(params.uplink_interface[j].virtual_ip, virtual_ip_char, sizeof(virtual_ip_char) ),
					params.uplink_interface[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.downlink_cnt));
	
	log_debug_failover(DEBUG_TYPE_CONNECT,"...downlink num %d", params.downlink_cnt);

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
		
		log_debug_failover(DEBUG_TYPE_CONNECT,"...downlink %d: master %s backup %s virtual %s ifname %s",
					j,
					inet_int2str(params.downlink_interface[j].master_ip,master_ip_char,sizeof(master_ip_char)),
					inet_int2str(params.downlink_interface[j].backp_ip,backup_ip_char,sizeof(backup_ip_char)),
					inet_int2str(params.downlink_interface[j].virtual_ip,virtual_ip_char,sizeof(virtual_ip_char)),
					params.downlink_interface[j].if_name );
	}

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(params.vgateway_cnt));
	
	log_debug_failover(DEBUG_TYPE_CONNECT,"...vgateway num %d", params.vgateway_cnt);

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
		dbus_message_iter_next(&iter_struct1); 
		/*add by sunjc@autelan.com*/
		dbus_message_iter_get_basic(&iter_struct1,&(params.vgateway_interface[j].virtual_ip));			
		
		dbus_message_iter_next(&iter_array1); 
		
		log_debug_failover(DEBUG_TYPE_CONNECT,"vgateway %s virtual_ip is %x\n",interface_name, params.vgateway_interface[j].virtual_ip);				
	}

	/*add sunjc@autelan.com for save vgateway virtual ip in interface*/
	if(failover_flag && (vip_nu = save_vgateway_virtual_ip_in_interface(params))){
		log_debug_failover(DEBUG_TYPE_CONNECT,"have saved vgeteway virtual ip number is %d\n", vip_nu);
	}		
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(interface_name));
	strncpy((char *)params.heartlink_if_name, interface_name, sizeof(params.heartlink_if_name)-1);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_local_ip));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&(params.heartlink_opposite_ip));

	log_debug_failover(DEBUG_TYPE_CONNECT,"...heartlink: name %s local_ip %s opposite_ip %s",
					params.heartlink_if_name,
					inet_int2str(params.heartlink_local_ip,master_ip_char,sizeof(master_ip_char)),
					inet_int2str(params.heartlink_opposite_ip,backup_ip_char,sizeof(backup_ip_char)) );


	/* set interface vrrp flag */
	op_ret = dhcp_dbus_disable_secondary_dhcp_server(&params);
	if (0 == failover_flag) {
	    /* zhangshu add for set dynamic arp, 2012-11-14 
	    log_debug("no failover condition\n");
	    start = time(NULL);
    	for (i = 0; i < params.downlink_cnt; i++) {
    		if(dhcp_dynamic_arp_switch && (VRRP_STATE_MAST == params.state)) 
    		    dhcp_set_dynamic_arps(params.downlink_interface[i].if_name, params.vrid);
    	}
    	for (i = 0; i < params.vgateway_cnt; i++) {
    		if(dhcp_dynamic_arp_switch && (VRRP_STATE_MAST == params.state)) 
    		    dhcp_set_dynamic_arps(params.vgateway_interface[i].if_name, params.vrid);
    	}
    	end = time(NULL);
    	log_debug("set dynamic arp time is %d\n",(int)(end-start));
    */	
		goto out;
	}

	if ((params.heartlink_local_ip) && (params.heartlink_opposite_ip)) {
		goodip = 1;
	}	
	if (goodip) {
		if (dhcp_dbus_failover_sndmsg(&params)) {
			op_ret = DHCP_SERVER_RETURN_CODE_FAIL;
		}
		if (VRRP_STATE_MAST == params.state && dhcp_dynamic_arp_switch) {
					
			 /* zhangshu add dhcp set dynamic arp to interfaces, 2012-11-14 */
			 
		     /* if dhcp server dynamic arp switch is open, and this is primary, then perform the 
		         *  set dynamic arp function to binding ip pool downlink interfaces or vGateways.
		         */
		    log_debug_failover(DEBUG_TYPE_CONNECT,"dhcp_dynamic_arp_switch is %s\n", dhcp_dynamic_arp_switch?"enable":"disable");
		    start = time(NULL);
		    for (j = 0; j < params.downlink_cnt; j++) {   		
    		    dhcp_set_dynamic_arps(params.downlink_interface[j].if_name, params.vrid);
    		}
    		for (j = 0; j < params.vgateway_cnt; j++) {   		
    		    dhcp_set_dynamic_arps(params.vgateway_interface[j].if_name, params.vrid);
    		}
		    end = time(NULL);
		    log_debug_failover(DEBUG_TYPE_CONNECT,"set dynamic arp time is %d\n",(int)(end-start));
		        
		}
		#if 0
		if (VRRP_STATE_MAST != params.state) {
			primary = 1;/*1 is secondary*/
		}
		else {
			primary = 0;/*0 is primary*/
		}
		
		ret = find_backup_list_by_vrrpid(params.vrid, &backup);
		if (ret && (VRRP_STATE_DISABLE != params.state) ) {/*can not find the back node for this vrrpid, add it*/
			backup = malloc(sizeof(struct failover_backup_list));
			if (NULL == backup) {
				log_error("malloc failover %d node failed\n", params.vrid);
				return NULL;
			}
			memset(backup, 0, sizeof(struct failover_backup_list));
			for (j = 0; j < (params.downlink_cnt + params.vgateway_cnt); j++) {
				/*new one if_name one subnode*/
				if (j < params.downlink_cnt) {	
					log_debug("downlink interface name is %s, vrrpid is %d, \n", params.downlink_interface[j].if_name, params.vrid);
					ret = dhcp_dbus_find_subnet_by_ifname(params.downlink_interface[j].if_name, &subnode);
				}
				else {				
					log_debug("vgateway interface name is %s, vrrpid is %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid);
					ret = dhcp_dbus_find_subnet_by_ifname(params.vgateway_interface[j - params.downlink_cnt].if_name, &subnode);
				}
				if (!ret) {
					if (!(subnode->owned_failover.enable)) {
						//sprintf(failname, "peer%d", (params.vrid)*100 + j);
						sprintf(failname, "peer%d", params.vrid);
						log_debug("add new failover state name is %s\n", failname);
						memcpy(subnode->owned_failover.name, failname, strlen(failname));

						if(!primary){
							subnode->owned_failover.split = 128;
							subnode->owned_failover.mclt = 10;
						}

						/*add by sunjc@autelan.com*/
						if(params.heartlink_local_ip > params.heartlink_opposite_ip){				
							subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params.vrid)*16 ;
							subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params.vrid)*16 + 100;																			
						}else {							
							subnode->owned_failover.myport = FAILOVER_LOCAL_PORT + (params.vrid)*16  + 100;
							subnode->owned_failover.parterport = FAILOVER_LOCAL_PORT + (params.vrid)*16 ;									
						}
						subnode->owned_failover.primary = primary;/*0 is primary 1 is secondary*/	
						subnode->owned_failover.myaddr.iabuf[0] = ((params.heartlink_local_ip)>>24)&0xff;
						subnode->owned_failover.myaddr.iabuf[1] = ((params.heartlink_local_ip)>>16)&0xff;
						subnode->owned_failover.myaddr.iabuf[2] = ((params.heartlink_local_ip)>>8)&0xff;
						subnode->owned_failover.myaddr.iabuf[3] = (params.heartlink_local_ip)&0xff;
						subnode->owned_failover.myaddr.len = DHCP_IPV4_ADDRESS_LEN;
						subnode->owned_failover.parteraddr.iabuf[0] = ((params.heartlink_opposite_ip)>>24)&0xff;
						subnode->owned_failover.parteraddr.iabuf[1] = ((params.heartlink_opposite_ip)>>16)&0xff;
						subnode->owned_failover.parteraddr.iabuf[2] = ((params.heartlink_opposite_ip)>>8)&0xff;
						subnode->owned_failover.parteraddr.iabuf[3] = (params.heartlink_opposite_ip)&0xff;
						subnode->owned_failover.parteraddr.len = DHCP_IPV4_ADDRESS_LEN;

						ret = dhcp_dbus_set_sub_failover(subnode);
						//sleep(1);
						if (!ret) {
							subnode->owned_failover.enable = 1;
							/*if is secondary add it in backup list, do not work
							if (subnode->owned_failover.primary) {*/
							if (j < params.downlink_cnt) {	
								memcpy(backup->dhcp_interface[if_count].if_name, params.downlink_interface[j].if_name, strlen(params.downlink_interface[j].if_name));
								log_debug("add backup downlink interface name is %s, vrrpid is %d, count %d\n", params.downlink_interface[j].if_name, params.vrid, if_count);
							}
							else {				
								memcpy(backup->dhcp_interface[if_count].if_name, params.vgateway_interface[j - params.downlink_cnt].if_name, strlen(params.vgateway_interface[j - params.downlink_cnt].if_name));
								log_debug("add backup vgateway interface name is %s, vrrpid is %d, count %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid, if_count);
							}
							/*}*/
							if_count ++;
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
			
			backup->interface_num = if_count;

			if (backup->interface_num) {
				log_debug("add %d failover state\n", backup->interface_num);
				backup->vrrpid = params.vrid;
				backup->state = primary;
				backup->next = failover_backup_list_head.next;
				failover_backup_list_head.next = backup;
			}
		}
		else {	/*state change*/
			for (j = 0; j < (params.downlink_cnt + params.vgateway_cnt); j++) {
				if (j < params.downlink_cnt) {	
					log_debug("downlink interface is %s, vrrpid is %d\n", params.downlink_interface[j].if_name, params.vrid);
					ret = dhcp_dbus_find_subnet_by_ifname(params.downlink_interface[j].if_name, &subnode);
				}
				else {				
					log_debug("vgateway interface is %s, vrrpid is %d\n", params.vgateway_interface[j - params.downlink_cnt].if_name, params.vrid);
					ret = dhcp_dbus_find_subnet_by_ifname(params.vgateway_interface[j - params.downlink_cnt].if_name, &subnode);
				}
				
				if (!ret) {
					/*
					log_info("subnode->owned_failover.primary is %d, primary is %d\n", 
							subnode->owned_failover.primary, primary);
					*/
					if (backup) {
						subnode->owned_failover.primary = primary;
						backup->state = primary;
						if(subnode->owned_failover.primary){
							subnode->owned_failover.mclt = 0;
							subnode->owned_failover.split = 0;
						}else{
							subnode->owned_failover.mclt = 10;
							subnode->owned_failover.split = 128;
						}
						dhcp_dbus_change_sub_failover(subnode);	
					}
					else {
						log_error("failover state change error same state %s\n", primary ? "secondary" : "primary");
						op_ret = 2;
					}
				}
				else {
					log_error("when change state can not find subnet by the interface\n");
				}				
			}			
		}
	#endif
	}

out:
	reply = dbus_message_new_method_return(msg);
	if (!reply) {
		log_error("portal HA dbus set state get reply message null error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&op_ret);

	return reply;
} 

DBusMessage* 
dhcp_dbus_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(DHCP_SAVE_CFG_MEM);
	
	if(!strShow) {
		log_debug("alloc memory fail when dhcp show running-config\n");
		return NULL;
	}
	memset(strShow, 0, DHCP_SAVE_CFG_MEM);

	dbus_error_init(&err);

	dhcp_dbus_profile_config_save(strShow);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	
	return reply;
}


DBusMessage* 
dhcp_dbus_show_running_hansi_cfg
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
	dhcp_dbus_profile_config_hansi_save(strShow, slot_id, vrrp_id, local_flag);

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



DBusMessage* 
dhcp_dbus_show_lease_state
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
	
	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);

	
	memset(&lease_state, 0, sizeof(struct dbus_lease_state));	
	
	ret = get_dhcp_lease_state_num(&lease_state, &sub_lease_state, &sub_count);	

	dbus_message_iter_init_append (reply, &iter);	
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.total_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.active_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.free_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.backup_lease_num));	
	
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&sub_count);
	
	log_debug("total %d , free %d active %d\n", lease_state.total_lease_num, lease_state.free_lease_num, lease_state.active_lease_num);


	dbus_message_iter_open_container (&iter,
								DBUS_TYPE_ARRAY,
								DBUS_STRUCT_BEGIN_CHAR_AS_STRING     /*begin	*/

								DBUS_TYPE_STRING_AS_STRING   //subnet
								DBUS_TYPE_STRING_AS_STRING   //netmask
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_count
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_active
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_free
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_backup
								
							
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
		
		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_STRING,&(sub_name));		
		
		dbus_message_iter_append_basic(&iter_struct,  											
										DBUS_TYPE_STRING, &(sub_mask));
		
		dbus_message_iter_append_basic(&iter_struct, 											  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.total_lease_num));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.active_lease_num));
		
		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.free_lease_num));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.backup_lease_num));		
		
		
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	if(sub_lease_state){
		free(sub_lease_state);
		sub_lease_state = NULL;
	}
	
	return reply;
}


DBusMessage* 
dhcp_dbus_show_lease_statistics
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
	
	ret = get_dhcp_lease_state_num(&lease_state, &sub_lease_state, &sub_count);	

	dbus_message_iter_init_append (reply, &iter);	
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.total_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.active_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.free_lease_num));
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(lease_state.backup_lease_num));	
	
	
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
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_free
								DBUS_TYPE_UINT32_AS_STRING   //subnet_lease_backup

								DBUS_TYPE_UINT32_AS_STRING   //discover times
								DBUS_TYPE_UINT32_AS_STRING   //offer times
								DBUS_TYPE_UINT32_AS_STRING   //request times
								DBUS_TYPE_UINT32_AS_STRING   //ack times
							
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
		
		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.free_lease_num));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].subnet_lease_state.backup_lease_num));		
		
		dbus_message_iter_append_basic(&iter_struct, 											  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].info.discover_times));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].info.offer_times));
		
		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].info.requested_times));

		dbus_message_iter_append_basic(&iter_struct,  
										DBUS_TYPE_UINT32, &(sub_lease_state[i].info.ack_times));

		
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	if(sub_lease_state){
		free(sub_lease_state);
		sub_lease_state = NULL;
	}
	
	return reply;
}


/*unicast reply flag  1: set unicast  ,  0 : default is broadcast*/
DBusMessage* 
dhcp_dbus_set_unicast_reply_mode
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err; 
	unsigned int 		flag = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,  
	   DBUS_TYPE_UINT32,&flag, 
	   DBUS_TYPE_INVALID))) {
	   log_error("dhcp set unicast reply mode:Unable to get input args ");
	   if (dbus_error_is_set(&err)) {
		   log_error("%s raised: %s",err.name,err.message);
		   dbus_error_free(&err);
	   }
	   return NULL;
    }
	log_debug("set dhcp reply unicast %u\n", flag);
	
	unicast_flag = flag;
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	
	
	return reply;
}

DBusMessage*
dhcp_dbus_set_debug_state
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
	if(debug_type == DEBUG_TYPE_DEBUG_FAILOVER_CONNECT){
		if(enable){
			dhcp_failover_debug_log_level |= debug_type;
		}else{
			dhcp_failover_debug_log_level &= ~debug_type;
		}
		log_debug_failover(DEBUG_TYPE_CONNECT,"dhcp_debug_type is failover_connect\n");

	}else if(debug_type == DEBUG_TYPE_DEBUG_FAILOVER_MSG_DEAL){
		if(enable){
			dhcp_failover_debug_log_level |= debug_type;
		}else{
			dhcp_failover_debug_log_level &= ~debug_type;
		}
		log_debug_failover(DEBUG_TYPE_MSG_DEAL,"dhcp_debug_type is failover_msg_deal\n");
	}else if(debug_type == DEBUG_TYPE_DEBUG_FAILOVER_ALL){
		if(enable){
			dhcp_failover_debug_log_level |= debug_type;
		}else{
			dhcp_failover_debug_log_level &= ~debug_type;
		}
		log_debug_failover(DEBUG_TYPE_FAILOVER_ALL,"dhcp_debug_type is failover_all\n");
	}else{
		if(debug_type == DEBUG_TYPE_ALL){
			log_debug("dhcp debug_type is %s \n", "all");
			if(enable){
			dhcp_failover_debug_log_level |= debug_type;
			}else{
				dhcp_failover_debug_log_level &= ~debug_type;
			}
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
		
		log_debug("globle dhcp_log_level is %d \n", dhcp_log_level);	
	}
		
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}


DBusMessage *dhcp_dbus_get_pool
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
	
	get_dhcp_pool_info();
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}


DBusMessage * 
dhcp_dbus_ping_check_enable
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

	dhcp_ping_check_enable = enable;
	log_info("dhcp ping check is %s \n", (dhcp_ping_check_enable) ? "enalbe" : "disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	return reply;	
}


DBusMessage * 
dhcp_dbus_lease_expiry_time
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int expiry_time = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &expiry_time,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if (expiry_time < 10) {
		expiry_time = 10;
	}
	dhcp_lease_expiry_time = expiry_time;
	log_info("dhcp lease expiry time is %d\n", dhcp_lease_expiry_time);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);
	return reply;	
}


DBusMessage* 
dhcp_dbus_get_statistics_info
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError err;   		
	struct statistics_info info;

	memset(&info, 0, sizeof(info));
	dhcp_get_statistics_info(&info);

	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	

	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.host_num));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.segment_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.discover_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.offer_times));	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.requested_times));
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &(info.ack_times));

	return reply;
}


DBusMessage * 
dhcp_dbus_optimize_enable
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

	dhcp_optimize_enable = enable;
	log_info("dhcp server optimize %s!!!\n", (dhcp_optimize_enable) ? "enalbe" : "disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);

	
	return reply;	
}
DBusMessage * 
	dhcp_dbus_check_interface_ve

(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int	ret = 1;
	char *name=NULL;

	
//	struct dcli_subnet* subNode = NULL;
	char *ifname = NULL;

	dbus_error_init( &err );
	if( !(dbus_message_get_args( msg ,&err,
		            DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	log_debug("lenth Of ifname=%d\n",strlen(ifname));
	name=malloc(ALIAS_NAME_SIZE);
	if(!name){
		log_error("application of name fails ");
		return NULL;	
	}
	memset(name,0,ALIAS_NAME_SIZE);
	ret = check_ve_interface(ifname,name);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		if(name){
		free(name);
		name=NULL;
		}	
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &name);
	if(name){
		free(name);
		name=NULL;
	}
	return reply;
}

DBusMessage * 
dhcp_dbus_set_bootp_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	iter;
	DBusError err;
	unsigned int enable = 0;
	unsigned int op_ret = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	dhcp_bootp_enable = enable;
	log_info("globle dhcp bootp support is %s!!!\n", (dhcp_bootp_enable) ? "enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);

	return reply;	
}



DBusMessage * 
dhcp_dbus_get_lease_info
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int addr = 0;
	struct iaddr iaddr;
	unsigned int pool_address_flag = 0;		/* 1: can find from ip pool; 0 : can't find from ip pool */
	unsigned int lease_states = 0;

/* Lease states: 
#define FTS_FREE	1
#define FTS_ACTIVE	2
#define FTS_EXPIRED	3
*/
	struct lease *lp = NULL;
	
	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,		
				DBUS_TYPE_UINT32, &addr,
				DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	memset(&iaddr, 0, sizeof(iaddr));
	iaddr.len = sizeof(addr);

	iaddr.iabuf[0] = (addr>>24) & 0xff;		
	iaddr.iabuf[1] = (addr>>16) & 0xff;
	iaddr.iabuf[2] = (addr>>8) & 0xff;
	iaddr.iabuf[3] = addr & 0xff;
	
	find_lease_by_ip_addr(&lp, iaddr, MDL);
	if (lp) {
		pool_address_flag = 1;
		lease_states = lp->binding_state;
		log_debug("lease %s lease states %s\n", piaddr(lp->ip_addr), binding_state_print(lp->binding_state));
		op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
	} else {
		op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
		pool_address_flag = 0;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32, &op_ret);
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32, &pool_address_flag);
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32, &lease_states);
	
	return reply;
}

DBusMessage * 
dhcp_dbus_get_failover_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
	isc_result_t status;
	unsigned int vrrpid = 0;
	dhcp_failover_state_t * dhcp_failover_state = (dhcp_failover_state_t *)0;	

	unsigned int local_failover_state = 0;
	unsigned int peer_failover_state = 0;
	char name[16];
	
	dbus_error_init(&err);	
	if (!(dbus_message_get_args (msg, &err,			
			DBUS_TYPE_UINT32, &vrrpid,		
			DBUS_TYPE_INVALID))) {		 
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s in dhcp_dbus_set_server_veo funtion\n",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	memset(name, 0, sizeof(name));
	sprintf(name, "peer%u", vrrpid);

	status = find_failover_peer(&dhcp_failover_state, name, MDL);
	if (status != ISC_R_NOTFOUND) {

		local_failover_state = dhcp_failover_state->me.state;		
		peer_failover_state = dhcp_failover_state ->partner.state;
		log_info("failover %s local state %s peer state %s\n", name,
			dhcp_failover_state_name_print(dhcp_failover_state->me.state),
			dhcp_failover_state_name_print(dhcp_failover_state->partner.state));
		
		dhcp_failover_state_dereference (&dhcp_failover_state, MDL);

		op_ret = DHCP_SERVER_RETURN_CODE_SUCCESS;
	} else {
		log_info("not find failover by name %s\n", name);
		op_ret = DHCP_FAILOVER_NOT_ENABLE;
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &op_ret);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &local_failover_state);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &peer_failover_state);

	return reply;
}


/* init dhcp dbus */
static DBusHandlerResult 
dhcp_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage		*reply = NULL;

    log_debug("dhcp message path %s\n",dbus_message_get_path(message));
	log_debug("dhcp message interface %s\n",dbus_message_get_interface(message));
	log_debug("dhcp message member %s\n",dbus_message_get_member(message));
	log_debug("dhcp message destination %s\n",dbus_message_get_destination(message));	
	log_debug("dhcp message type %d\n",dbus_message_get_type(message));
	pthread_mutex_lock(&DhcpthreadMutex);
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SAVE_DHCP_LEASE)) {
		reply = dhcp_dbus_save_lease(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_DHCP_LEASE)) {
		reply = dhcp_dbus_show_lease(connection, message, user_data);
	}
	if(dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_CONFIG_DHCP_LEASE_ENABLE)){
		reply = dhcp_dbus_config_lease_time(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_GET_LEASE_INFO )) {
		reply = dhcp_dbus_get_lease_info(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_STATIC_HOST)) {
		reply = dhcp_dbus_show_static_host(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_FAILOVER_CFG)) {
		reply = dhcp_dbus_show_failover_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP)) {
		reply = dhcp_dbus_show_lease_by_ip(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC)) {
		reply = dhcp_dbus_show_lease_by_mac(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF)) {
		reply = dhcp_dbus_show_dhcp_global_conf(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_IP_POOL_CONF)) {
		reply = dhcp_dbus_show_dhcp_pool_conf(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_ENTRY_POOL_NODE)) {
		reply = dhcp_dbus_entry_pool_node(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_INTERFACE_POOL)) {
		reply = dhcp_dbus_set_interface_pool(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_CREATE_POOL_NODE )) {
		reply = dhcp_dbus_create_pool_node(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_DOMAIN_NAME)) {
		reply = dhcp_dbus_set_server_domain_name(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_VEO_OLD)) {
		reply = dhcp_dbus_set_server_v1(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_VEO)) {
		reply = dhcp_dbus_set_server_veo(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_NO_VEO)) {
		reply = dhcp_dbus_set_server_no_veo(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_NO_OPTION138)) {
		reply = dhcp_dbus_set_server_no_option138(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_OPTION138)) {
		reply = dhcp_dbus_set_server_option138(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_DNS)) {
		reply = dhcp_dbus_set_server_dns(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_ROUTERS_IP)) {
		reply = dhcp_dbus_set_server_routers_ip(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_ENABLE)) {
		reply = dhcp_dbus_set_server_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_DYNAMIC_ARP_ENABLE)) {
		reply = dhcp_dbus_set_dynamic_arp_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_NAK_RSP_ENABLE)) {
		reply = dhcp_dbus_set_nak_rsp_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_ASN_ENABLE)) {
		reply = dhcp_dbus_set_ASN_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_AUTO_RESTART_ENABLE)) {
		reply = dhcp_dbus_auto_restart_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_DHCP_FOR_LOCAL7_ENABLE)) {
		reply = dhcp_dbus_set_dhcplog_for_local7_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_OPTION60_ENABLE)) {
		reply = dhcp_dbus_set_server_option60_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_OPTION60_SET_ID)) {
		reply = dhcp_dbus_set_server_option60_set_id(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_GET_SERVER_STATE)) {
		reply = dhcp_dbus_get_server_state(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_STATIC_ENABLE)) {
		reply = dhcp_dbus_set_server_static_arp_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_WINS_IP)) {
		reply = dhcp_dbus_set_server_wins_ip(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT)) {
		reply = dhcp_dbus_set_server_lease_default(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_SERVER_LEASE_MAX)) {
		reply = dhcp_dbus_set_server_lease_max(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_CHECK_INTERFACE_VE)) {
		reply = dhcp_dbus_check_interface_ve(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_ADD_IP_POOL_RANGE)) {
		reply = dhcp_dbus_add_dhcp_pool_ip_range(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_ADD_STATIC_HOST)) {
		reply = dhcp_dbus_add_dhcp_static_host(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_ADD_DHCP_FAILOVER_PEER)) {
		reply = dhcp_dbus_add_dhcp_failover_peer(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_CFG_DHCP_FAILOVER_PEER)) {
		reply = dhcp_dbus_cfg_dhcp_failover_peer(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_DEL_DHCP_FAILOVER_PEER)) {
		reply = dhcp_dbus_del_dhcp_failover_peer(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_SET_HA_STATE)) {
		pthread_mutex_lock(&DhcpDbusEventMutex);
		reply = dhcp_dbus_failover_recv_vrrp_state(connection, message, user_data);
		pthread_mutex_unlock(&DhcpDbusEventMutex);		
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		reply = dhcp_dbus_show_running_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_HASNIS_RUNNING_CFG)) {
		reply = dhcp_dbus_restart_load_hansi_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_HMD_RUNNING_CHECK)) {
		reply = dhcp_dbus_hmd_running_check(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG)) {
		reply = dhcp_dbus_show_running_hansi_cfg(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_LEASE_STATE)) {
		reply = dhcp_dbus_show_lease_state(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SHOW_LEASE_STATISTICS)) {
		reply = dhcp_dbus_show_lease_statistics(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_UNICAST_REPLY_MODE)) {
		reply = dhcp_dbus_set_unicast_reply_mode(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_CHECK_SERVER_INTERFACE)) {
		reply = dhcp_dbus_check_server_interface(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_DEBUG_STATE)) {
		reply = dhcp_dbus_set_debug_state(connection, message, user_data);
	}
	else if(dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_GET_POOL_INFO))
	{
		reply = dhcp_dbus_get_pool(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_PING_CHECK_ENABLE)) {
		reply = dhcp_dbus_ping_check_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_LEASE_EXPIRY_TIME)) {
		reply = dhcp_dbus_lease_expiry_time(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_GET_STATISTICS_INFO)) {
		reply = dhcp_dbus_get_statistics_info(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_OPTIMIZE_ENABLE)) {
		reply = dhcp_dbus_optimize_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_GET_FAILOVER_STATE)) {
		reply = dhcp_dbus_get_failover_state(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_BOOTP_ENABLE)) {
		reply = dhcp_dbus_set_bootp_enable(connection, message, user_data);
	}
	/* pppoe snp */
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_PPPOE_SNP_ENABLE)) {
		reply = pppoe_snp_dbus_enable(connection, message, user_data);
	}	
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_PPPOE_SNP_IFACE_ENABLE)) {
		reply = dhcp_pppoe_snooping_iface_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_PPPOE_DEBUG)) {
		reply = pppoe_snp_dbus_set_debug_state(connection, message, user_data);
	}

	/* DBA(dhcp broadcast agent) */
	if (dbus_message_is_method_call(message, DHCP_DBUS_INTERFACE, DHCP_DBUS_METHOD_DIRECT_BROADCAST_ENABLE)) {
		reply = dba_dbus_server_enable(connection, message, user_data);
	}	
	
	pthread_mutex_unlock(&DhcpthreadMutex);
	
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult 
dhcp_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */
		dbus_connection_unref (dhcp_dbus_connection);
		dhcp_dbus_connection = NULL;
		log_info("dbus_message_is_signal");
        dhcp_dbus_restart();
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
dhcp_tell_whoami
(
	char * myName,
	int isLast
)
{
	char pidBuf[DHCP_PID_BUFFER_LEN] = {0}, pidPath[DHCP_PID_BUFFER_LEN] = {0};
	pid_t myPid = 0;
	
	if(!myName) {
		return;
	}

	sprintf(pidPath,"%s%s%d%s", DHCP_PID_FILE_PATH, DHCP_PID_FILE_PREFIX, \
				global_current_instance_no, DHCP_PID_FILE_SUFFIX);
		
	if(DHCP_FD_INIT == g_dhcp_pid_fd) {	
		g_dhcp_pid_fd = open(pidPath, O_RDWR|O_CREAT);
		if(DHCP_FD_INIT == g_dhcp_pid_fd) {
			return;
		}
	}

	myPid = getpid();
	
	sprintf(pidBuf,"instance %d %s has pid %d\n", global_current_instance_no, myName, myPid);
	write(g_dhcp_pid_fd, pidBuf, strlen(pidBuf));

	/* close pid file by last teller */
	if(isLast) {
		close(g_dhcp_pid_fd);
		g_dhcp_pid_fd = DHCP_FD_INIT;
	}

	return;
}

int 
dhcp_dbus_init
(
	void
)
{
	DBusError dbus_error;
	DBusObjectPathVTable	dhcp_vtable = {NULL, &dhcp_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (1);

	dbus_error_init (&dbus_error);
	dhcp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dhcp_dbus_connection == NULL) {
		log_error ("dbus_bus_get(): %s", dbus_error.message);
		return 1;
	}

	/* Use dhcp to handle subsection of DHCP_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (dhcp_dbus_connection, DHCP_DBUS_OBJPATH, &dhcp_vtable, NULL)) {
		log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
		
	dbus_bus_request_name (dhcp_dbus_connection, DHCP_DBUS_BUSNAME,
			       0, &dbus_error);
		
	if (dbus_error_is_set (&dbus_error)) {
		log_error ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return 1;
	}

	dbus_connection_add_filter (dhcp_dbus_connection, dhcp_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (dhcp_dbus_connection,
			    		"type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

	return 0;
}


int 
dhcp_dbus_reinit
(
	void
)
{
    log_info("%s %s %d", __FILE__, __func__, __LINE__);
	DBusError dbus_error;
	DBusObjectPathVTable	dhcp_vtable = {NULL, &dhcp_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (1);

	dbus_error_init (&dbus_error);
	dhcp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dhcp_dbus_connection == NULL) {
		log_error ("dbus_bus_get(): %s", dbus_error.message);
		return 1;
	}

	/* Use dhcp to handle subsection of DHCP_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (dhcp_dbus_connection, DHCP_DBUS_OBJPATH, &dhcp_vtable, NULL)) {
		log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
		
	dbus_bus_request_name (dhcp_dbus_connection, DHCP_DBUS_BUSNAME,
			       0, &dbus_error);
		
	if (dbus_error_is_set (&dbus_error)) {
		log_error ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return 1;
	}

	dbus_connection_add_filter (dhcp_dbus_connection, dhcp_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (dhcp_dbus_connection,
			    		"type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

	return 0;
}


void * 
dhcp_dbus_thread_main
(
	void *arg
)
{

	dhcp_tell_whoami("dbusDhcp", 1);

	/* tell about my initialization process done
	npd_init_tell_stage_end();
*/	
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(dhcp_dbus_connection,-1)) {
		;
	}
	
	return NULL;
}


void * 
dhcp_dbus_thread_main2
(
	void *arg
)
{
    log_info("%s %s %d", __FILE__, __func__, __LINE__);

	dhcp_tell_whoami("dbusDhcpRestart", 1);

	/* tell about my initialization process done
	npd_init_tell_stage_end();
*/	
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(dhcp_dbus_connection,-1)) {
		;
	}
	
	return NULL;
}



void 
dhcp_dbus_start
(
	void
)
{
	int ret = 0;
	memset(&head_pool, 0, sizeof(struct dcli_pool));
	dhcp_dbus_init();
	
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	ret = pthread_create(dbus_thread, &dbus_thread_attr, dhcp_dbus_thread_main, NULL);
    if (0 != ret) {
	   log_error ("start dhcp dbus pthread fail\n");
	}

	netlink_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&netlink_thread_attr);
	ret = pthread_create(netlink_thread, &netlink_thread_attr, dhcp_receive_netlink, NULL);
	if (0 != ret) {
	   log_error ("start dhcp netlink pthread fail\n");
	}

	dhcp_snp_netlink_init();

}


static void 
dhcp_dbus_restart
(
	void
)
{
    log_info("%s %s %d", __FILE__, __func__, __LINE__);
	int ret = 0;
	//memset(&head_pool, 0, sizeof(struct dcli_pool));
	dhcp_dbus_reinit();
	
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	ret = pthread_create(dbus_thread, &dbus_thread_attr, dhcp_dbus_thread_main2, NULL);
    if (0 != ret) {
	   log_error ("restart dhcp dbus pthread fail\n");
	}
    
    /*
	netlink_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&netlink_thread_attr);
	ret = pthread_create(netlink_thread, &netlink_thread_attr, dhcp_receive_netlink, NULL);
	if (0 != ret) {
	   log_error ("restart dhcp netlink pthread fail\n");
	}
       
	dhcp_snp_netlink_init();
	*/

}



#ifdef __cplusplus
}
#endif

