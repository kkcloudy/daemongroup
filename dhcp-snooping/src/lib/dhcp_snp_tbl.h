#ifndef __NPD_DHCP_SNP_TBL_H__
#define __NPD_DHCP_SNP_TBL_H__

/*********************************************************
*	head files														*
**********************************************************/
#include "dhcp_snp_com.h"

/*********************************************************
*	macro define													*
**********************************************************/
#define NPD_DHCP_SNP_HASH_TABLE_SIZE    (1024)
#define NPD_DHCP_SNP_MAC_ADD_LEN        6                        /* length of mac address			*/
#define CHASSIS_SLOT_COUNT	(16)

typedef enum NPD_DHCP_SNP_BIND_STATE_S {
	NPD_DHCP_SNP_BIND_STATE_REQUEST         = 0,
	NPD_DHCP_SNP_BIND_STATE_BOUND           = 1,
} NPD_DHCP_SNP_BIND_STATE_T;

typedef enum NPD_DHCP_SNP_BIND_TYPE_S {
	NPD_DHCP_SNP_BIND_TYPE_DYNAMIC         = 0,
	NPD_DHCP_SNP_BIND_TYPE_STATIC          = 1,
} NPD_DHCP_SNP_BIND_TYPE_T;


/* dhcp snp aging time interval */
#define DHCP_SNP_DEFAULT_AGING_TIME		(10)


/*********************************************************
*	struct define													*
**********************************************************/
typedef struct NPD_DHCP_SNP_USER_ITEM_S
{
	unsigned int  bind_type;
	unsigned char state;
	unsigned char haddr_len;
	unsigned char chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned short vlanId;
	unsigned int ip_addr;
	unsigned int lease_time;
	unsigned int sys_escape; /*添加绑定表项时系统启动以来所过的时间 */
	unsigned int cur_expire;	   /* 当前使用的有效的IP地址状态超时时间,仅显示时使用*/
	unsigned int ifindex;
	unsigned int flags;
}NPD_DHCP_SNP_USER_ITEM_T;
typedef struct NPD_DHCPv6_SNP_USER_ITEM_S
{
	unsigned int  bind_type;
	unsigned char state;
	unsigned char haddr_len;
	unsigned char chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned short vlanId;
	unsigned char ipv6_addr[16];
	unsigned int lease_time;
	unsigned int sys_escape; /*添加绑定表项时系统启动以来所过的时间 */
	unsigned int cur_expire;	   /* 当前使用的有效的IP地址状态超时时间,仅显示时使用*/
	unsigned int ifindex;
	unsigned int flags;
}NPD_DHCPv6_SNP_USER_ITEM_T;

typedef struct NPD_DHCP_SNP_TBL_ITEM_S
{
	struct NPD_DHCP_SNP_TBL_ITEM_S *next;
	struct NPD_DHCP_SNP_TBL_ITEM_S *ip_next;	
	unsigned int   bind_type;
	unsigned char  state;
	unsigned char  haddr_len;
	unsigned char  chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned short vlanId;
	unsigned int   ip_addr;
	unsigned int   lease_time;
	unsigned int   sys_escape;
	unsigned int   cur_expire;
	unsigned int   ifindex;
	unsigned int   flags;
}NPD_DHCP_SNP_TBL_ITEM_T;
typedef struct NPD_DHCPv6_SNP_TBL_ITEM_S
{
	struct NPD_DHCPv6_SNP_TBL_ITEM_S *next;
	struct NPD_DHCPv6_SNP_TBL_ITEM_S *ip_next;	
	unsigned int   bind_type;
	unsigned char  state;
	unsigned char  haddr_len;
	unsigned char  chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned short vlanId;
	unsigned char   ipv6_addr[16];
	unsigned int   lease_time;
	unsigned int   sys_escape;
	unsigned int   cur_expire;
	unsigned int   ifindex;
	unsigned int   flags;
}NPD_DHCPv6_SNP_TBL_ITEM_T;

struct dhcp_snp_static_table {
	unsigned int ipaddr;
	unsigned char  chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
	unsigned int ifindex;
	struct dhcp_snp_static_table *next;
};

/* file to save all dhcpsnp thread pid */
#define DHCP_SNP_THREAD_PID_PATH	"/var/run/dhcpsnp.pid"

#define DHCP_SNP_SOCK_PATH	"/var/run/dhcpsnp.unix"
#define DHCP_SNP_ASD_SOCK_PATH "/var/run/wcpss/asd_table"
#define MAX_HANSI_PROFILE     (16 + 1)		/* for support V1.2, add instance 0 */

/* default after 300 secends table will be delete */
#define TBL_EXPIRED_TIME	(5 * 60)

struct unresolved_table {
	struct unresolved_table *next;
	unsigned int bssindex;
	int vrrpid;
	unsigned int local_flag;
	unsigned int ipaddr;
	unsigned int   ifindex;
	unsigned int expired;	/* cur_uptime + 300 seconds */
	unsigned char  chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
};
struct unresolved_ipv6_table {
	struct unresolved_table *next;
	unsigned int bssindex;
	int vrrpid;
	unsigned int local_flag;
	unsigned char ipv6_addr[16];
	unsigned int   ifindex;
	unsigned int expired;	/* cur_uptime + 300 seconds */
	unsigned char  chaddr[NPD_DHCP_SNP_MAC_ADD_LEN];
};

/*********************************************************
*	function declare												*
**********************************************************/

/**********************************************************************************
 *dhcp_snp_tbl_initialize ()
 *
 *	DESCRIPTION:
 *		initialize DHCP Snooping bind table
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK - success
 *
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_initialize
(
	void
);

/**********************************************************************************
 *dhcp_snp_tbl_hash ()
 *
 *	DESCRIPTION:
 *		get the hash value according the user mac address
 *
 *	INPUTS:
 *		unsigned char *mac		- user mac address
 *
 *	OUTPUTS:
 *		h						- hash key value
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_hash
(
	unsigned char *mac
);
int dhcp_snp_tbl_mac_hash_foreach
(
void
);
int dhcp_snp_tbl_ip_hash_foreach
(
void
);

void dhcp_snp_update_logfile
(
	void
);

/**********************************************************************************
 *dhcp_snp_tbl_destroy ()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping bind table momery
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK - success
 *
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_destroy
(
	void
);

/**********************************************************************************
 *dhcp_snp_tbl_item_find ()
 *
 *	DESCRIPTION:
 *		Get the item of specifical user
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
void *dhcp_snp_tbl_item_find
(
	NPD_DHCP_SNP_USER_ITEM_T *user
);

/**********************************************************************************
 *dhcp_snp_tbl_fill_item ()
 *
 *	DESCRIPTION:
 *		fill the dhcp bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- fail
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_fill_item
(
	NPD_DHCP_SNP_USER_ITEM_T *user,
	NPD_DHCP_SNP_TBL_ITEM_T *item
);

/**********************************************************************************
 *dhcp_snp_tbl_item_insert ()
 *
 *	DESCRIPTION:
 *		insert the user bind information into the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
void *dhcp_snp_tbl_item_insert
(
	NPD_DHCP_SNP_USER_ITEM_T *user
);

/**********************************************************************************
 *npe_dhcp_snp_tbl_item_delete ()
 *
 *	DESCRIPTION:
 *		delete the user bind item from the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *	
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_item_delete
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
);
unsigned int dhcp_snp_tbl_item_delete_iphash
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
);
unsigned int dhcpv6_snp_tbl_item_delete_iphash
(
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
);

/*********************************************************************
 *dhcp_snp_item_destroy ()
 *
 *	DESCRIPTION:
 *		delete bind table from ip hash / mac hash, then free item
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	OUTPUTS:
 *		
 *
 *	RETURN VALUE:
 *		0			- success
 *		-1			- fail
 *	
 *********************************************************************/
int dhcp_snp_item_destroy
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
);


/**********************************************************************************
 *dhcp_snp_tbl_get_item ()
 *
 *	DESCRIPTION:
 *		 get the dhcp bind table item information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *	
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_get_item
(
	NPD_DHCP_SNP_USER_ITEM_T *user,
	NPD_DHCP_SNP_TBL_ITEM_T *item
);

/**********************************************************************************
 *dhcp_snp_tbl_fill_bind ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
void dhcp_snp_tbl_fill_bind
(
	NPD_DHCP_SNP_USER_ITEM_T *user,
	NPD_DHCP_SNP_TBL_ITEM_T *item
);

/**********************************************************************************
 *dhcp_snp_tbl_item_insert ()
 *
 *	DESCRIPTION:
 *		insert the user bind information into the bind table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_identity_item
(
	NPD_DHCP_SNP_TBL_ITEM_T *item,
	NPD_DHCP_SNP_USER_ITEM_T *user
);

/**********************************************************************************
 *dhcp_snp_tbl_refresh_bind ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_refresh_bind
(
	NPD_DHCP_SNP_USER_ITEM_T *user,
	NPD_DHCP_SNP_TBL_ITEM_T *item,
	struct dhcp_snp_listener *node	
);


/**********************************************************************************
 *dhcp_snp_tbl_query ()
 *
 *	DESCRIPTION:
 *		query from dhcp snp db
 *
 *	INPUTS:
 *		char *sql									- sql of insert a item to dhcp snp db
 *	 	unsigned int count,						- count of records
 *		NPD_DHCP_SNP_SHOW_ITEM_T array[]		- array of records
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR			- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_query
(
	char *sql,
	unsigned int *count,
	NPD_DHCP_SNP_SHOW_ITEM_T array[]
);


/**********************************************************************************
 *dhcp_snp_query_arp_inspection ()
 *
 *	DESCRIPTION:
 *		query from dhcp snp db for arp inspection by sip
 *
 *	INPUTS:
 *		unsigned int dip			- sip
 *		unsigned char *isFound		- found or not found, 1: found 0: not found
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- query success
 *		DHCP_SNP_RETURN_CODE_ERROR			- query fail
 ***********************************************************************************/
unsigned int dhcp_snp_query_arp_inspection
(
	unsigned int sip,
	unsigned char *isFound
);

/**********************************************************************************
 *dhcp_snp_query_arp_proxy ()
 *
 *	DESCRIPTION:
 *		query from dhcp snp db for arp proxy by dip
 *
 *	INPUTS:
 *		unsigned int dip						- dip
 *		NPD_DHCP_SNP_SHOW_ITEM_T *result	- result of query
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- query success
 *		DHCP_SNP_RETURN_CODE_ERROR			- query fail
 ***********************************************************************************/
unsigned int dhcp_snp_query_arp_proxy
(
	unsigned int dip,
	NPD_DHCP_SNP_SHOW_ITEM_T *result
);

/**********************************************************************************
 *dhcp_snp_tbl_static_binding_insert()
 *
 *	DESCRIPTION:
 *		insert a static binding item to dhcp snp db
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need insert into dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR		- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_static_binding_insert
(
	NPD_DHCP_SNP_TBL_ITEM_T item
);

/**********************************************************************************
 *dhcp_snp_tbl_static_binding_delete()
 *
 *	DESCRIPTION:
 *		delete a static binding item from dhcp snp db
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need delete from dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR		- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_static_binding_delete
(
	NPD_DHCP_SNP_TBL_ITEM_T item
);

/**********************************************************************************
 *dhcp_snp_tbl_binding_insert()
 *
 *	DESCRIPTION:
 *		insert a binding item to dhcp snp db
 *		inner interface in dhcp snooping, for save content of bindint table
 *		by command "write" before reboot
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 	- item which need insert into dhcp snp db
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- sql is null
 *		DHCP_SNP_RETURN_CODE_OK			- exec success
 *		DHCP_SNP_RETURN_CODE_ERROR		- exec fail
 ***********************************************************************************/
unsigned int dhcp_snp_tbl_binding_insert
(
	NPD_DHCP_SNP_TBL_ITEM_T item
);

/**********************************************************************************
 *dhcp_snp_tbl_item_insert_ip_hash ()
 *
 *	DESCRIPTION:
 *		insert the user bind information into the ip hash table
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/

void *dhcp_snp_tbl_item_insert_ip_hash
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
);

/**********************************************************************************
 *	dhcp_snp_initiative_notify_asd()
 *
 *	DESCRIPTION:
 *		when recv DHCP ACK, look for unresolved table, if this MAC not send to asd, 
 *		send to asd
 *
 *	INPUTS:
 *		haddr - clinet MAC
 *		ipaddr - IPv4 address
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		
 ***********************************************************************************/
void dhcp_snp_initiative_notify_asd(unsigned char *haddr, unsigned int ipaddr);

void dhcp_snp_print_tbl
(
	void
);
void dhcp_snp_print_tbl_ip
(
	void
);



/*********************************************************
*	extern Functions												*
**********************************************************/

unsigned int dhcp_snp_get_system_uptime
(
	void
);
void * dhcp_snp_tbl_thread_aging
(
	void *arg
);
int dhcp_snp_aging_mechanism
(
	void
);
void *dhcp_snp_tbl_item_find_by_ip
(
	unsigned int ipaddr
);
void dhcp_snp_print_static_bindtable
(
	void
);

void * dhcp_snp_asd_interactive
(
	void *arg
);


/******************************************************************************
 *	dhcp_snp_asd_table_aging_mechanism()
 *
 *	DESCRIPTION:
 *		dhcp snp unresolved table aging (to asd)
 *
 *	INPUTS:
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		0 - success
 *		-1 - failed
 *		
 *****************************************************************************/
int dhcp_snp_asd_table_aging_mechanism
(
	void
);

/*****************************************************************************
* dhcp_snp_tell_whoami
*
* DESCRIPTION:
*       This function is used by each thread to tell its name
*	 and pid to DHCP_SNP_THREAD_PID_PATH
*
* INPUTS:
*	  thread_name - thread name.
*	  last_teller - is this the last teller or not, pid file should be closed if true.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	 None.
*
* COMMENTS:
*       
*
******************************************************************************/
void dhcp_snp_tell_whoami
(
	unsigned char *thread_name,
	unsigned int last_teller
);

/**********************************************************************
 *	dhcp_snp_setaffinity
 * 
 *	dhcp_snp set affinity
 *
 *  INPUT:
 *		mask - cpu mask
 *  
 *  OUTPUT:
 * 	NULL
 *
 *  RETURN:
 * 	 -1	-	dhcp_snp affinity failed 
 * 	   0	-	dhcp_snp affinity success
 *
 **********************************************************************/
int dhcp_snp_setaffinity(unsigned long mask);


/*********************************************************
*	extern Functions												*
**********************************************************/

#endif

