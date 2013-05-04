#ifndef __DCLI_DHCP6_CONF_H__
#define __DCLI_DHCP6_CONF_H__

#include "dbus/dhcp/dhcp_dbus_def.h"
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IRWXU)

/*max subnet number*/
#define MAX_SUBNET 512 
#define MAX_SIZE (sizeof(struct dhcp_subnet6)* MAX_SUBNET)

/*for Application shared memory*/
#define SUB_NET6  "/subnet"
#define SUBNET6_COUNT "/count"
#define DHCP_SERVER_ENABLE "/dhcpserver"

/*Application semaphore for Mutex*/
#define SEM_MUTEX "/mutex"

#define IPV6_MAX_LEN    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") + 1)
#define DHCP_IP6_RET_SUCCESS      0
#define DHCP_IP6_RET_ERROR         1


struct dhcp_subnet6
{
    char low_ip[IPV6_MAX_LEN];
    char high_ip[IPV6_MAX_LEN];
    char sub_ip[IPV6_MAX_LEN];
    char dns[3][IPV6_MAX_LEN];
    char domainname[3][128];
    int index;
    unsigned int dnsnum;
    unsigned int domainnum;
    unsigned int defaulttime;
    unsigned int maxtime;
};

/********************************************************
 * check_ipv6_address
 *
 * Check the legality of the ipv6 address
 *
 *	INPUT:
 *		ipv6_address
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - Legal address
 *		DHCP_IP6_RET_SUCCESS	- Illegal address
 *
 *********************************************************/
int check_ipv6_address(char *ipv6_address);


/********************************************************
 * check_doamin
 *
 * Check the legality of the domain
 *
 *	INPUT:
 *		domain_name
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - Legal domain
 *		DHCP_IP6_RET_SUCCESS	- Illegal domain
 *
 *********************************************************/

int check_doamin(char *domain_name);


/********************************************************
 * get_subnet6_count
 *
 * get subnet number
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get subnet number error
 *		other	- subnet number address
 *
 *********************************************************/

int* get_subnet6_count(void);

/********************************************************
 * get_subnet
 *
 * get subnet head of address
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get error
 *		other	- subnet head address
 *
 *********************************************************/

struct dhcp_subnet6* get_subnet(void);

/********************************************************
 * get_ip6_dhcp_server_enable_flag
 *
 * get dhcp server enable flag address
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		NULL	- get error
 *		other	- subnet head address
 *
 *********************************************************/

int * get_ip6_dhcp_server_enable_flag(void);

/********************************************************
 * dhcp_find_subnet6_by_name
 *
 * find the subnet by name
 *
 *	INPUT:
 *		sub_ip
 *		
 *	OUTPUT:
 *		subnode
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - not find
 *		DHCP_IP6_RET_SUCCESS	- find success
 *
 *********************************************************/
int dhcp_find_subnet6_by_name(char* sub_ip, struct dhcp_subnet6** subnode );

/********************************************************
 * dhcp_find_subnet6_by_index
 *
 * find the subnet by subnet's index
 *
 *	INPUT:
 *		sub_index
 *		
 *	OUTPUT:
 *		subnode
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - not find
 *		DHCP_IP6_RET_SUCCESS	  - find success
 *
 *********************************************************/

int dhcp_find_subnet6_by_index(int sub_index, struct dhcp_subnet6** subnode );

/********************************************************
 * dhcp_server_write_conf
 *
 * save subnet information to /etc/dhcpd.conf
 *
 *	INPUT:
 *		void
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - funtion error
 *		DHCP_IP6_RET_SUCCESS	  - funtion success
 *
 *********************************************************/


int  dhcp_server_write_conf(void);

/********************************************************
 * dcli_dhcp6_show_running_cfg
 *
 * show dhcp6 config
 *
 *	INPUT:
 *		vty 
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR		  - funtion error
 *		DHCP_IP6_RET_SUCCESS	  - funtion success
 *
 *********************************************************/

int dcli_dhcp6_show_running_cfg(struct vty *vty);



#endif 

