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
* dhcp_snp_tbl.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping table for NPD module.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <sys/types.h>
#include <time.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <linux/if_ether.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/un.h>


#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "wcpss/waw.h"

#include "dhcp_snp_log.h"

#include "dhcp_snp_com.h"
#include "dhcp_snp_sqlite.h"
#include "dhcp_snp_netlink.h"
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_pkt.h"
#include "dhcp_snp_listener.h"


#define TBL_INDEX(vrrpid, local_flag)		(vrrpid + local_flag * MAX_HANSI_PROFILE)


struct fd_table {
	int sock_fd;
    struct sockaddr_un servaddr;
};

/*********************************************************
*	global variable define											*
**********************************************************/
NPD_DHCP_SNP_TBL_ITEM_T *g_DHCP_Snp_Hash_Table[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};
NPD_DHCP_SNP_TBL_ITEM_T *g_DHCP_Snp_Hash_Table_ip[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};
struct dhcp_snp_static_table *dhcp_snp_static_table_head = NULL;

NPD_DHCPv6_SNP_TBL_ITEM_T *g_DHCPv6_Snp_Hash_Table[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};
NPD_DHCPv6_SNP_TBL_ITEM_T *g_DHCPv6_Snp_Hash_Table_ip[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};


unsigned int dhcp_snp_ageing_time = DHCP_SNP_DEFAULT_AGING_TIME;
pthread_mutex_t mutexDhcpsnptbl = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexsnpunsolve = PTHREAD_MUTEX_INITIALIZER;
struct fd_table *fd_table[MAX_HANSI_PROFILE * 2];
struct unresolved_table *g_dhcp_snp_unresolved_hash[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};
struct unresolved_table *g_dhcpv6_snp_unresolved_hash[NPD_DHCP_SNP_HASH_TABLE_SIZE] = {0};


static sqlite3 *dhcp_snp_db = NULL;

static char *zCreateTbl = "\
						CREATE TABLE dhcpSnpDb(\
						MAC VARCHAR(12) NOT NULL UNIQUE,\
						IP INTEGER NOT NULL UNIQUE,\
						VLAN INTEGER,\
						PORT INTEGER,\
						SYSTIME INTEGER,\
						LEASE INTEGER,\
						BINDTYPE INTEGER,\
						PRIMARY KEY(MAC)\
						);";

static char *zDropTbl = "drop TABLE dhcpSnpDb;";

/*********************************************************
*	extern variable												*
**********************************************************/
extern int dhcp_snp_send_arp_solicit
(
	unsigned int ifindex,
	char *dmac,
	unsigned int dip
);


/*********************************************************
*	functions define												*
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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	memset(g_DHCP_Snp_Hash_Table, 0, NPD_DHCP_SNP_HASH_TABLE_SIZE * sizeof(NPD_DHCP_SNP_TBL_ITEM_T *));
	memset(g_DHCP_Snp_Hash_Table_ip, 0, NPD_DHCP_SNP_HASH_TABLE_SIZE * sizeof(NPD_DHCP_SNP_TBL_ITEM_T *));

	/* open dhcp snooping db */
	ret = dhcp_snp_db_open(&dhcp_snp_db);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("open DHCP-Snooping db error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("open DHCP-Snooping db %s %p success.\n",
							NPD_DHCP_SNP_SQLITE3_DBNAME, dhcp_snp_db);

	/* create dhcp snooping table */
	ret = dhcp_snp_db_create(dhcp_snp_db, zCreateTbl);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("create DHCP-Snooping db error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

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
)
{

	register unsigned accum = 0;
	register const unsigned char *s = (const unsigned char *)mac;
	
	accum ^= *s++ << 16;
	accum ^= *s++ << 8;
	accum ^= *s++;
	accum ^= *s++ << 16;
	accum ^= *s++ << 8;
	accum ^= *s++;
	
    return (accum) % NPD_DHCP_SNP_HASH_TABLE_SIZE;
}

unsigned int dhcp_snp_tbl_ip_hash
(
	unsigned int ip
)
{
    return ip % NPD_DHCP_SNP_HASH_TABLE_SIZE;
}


int dhcp_snp_tbl_mac_hash_foreach
(
void
)
{
	int count = 0;
	unsigned int key = 0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCP_Snp_Hash_Table[key];
		while(tempItem) {
			count++;
			tempItem = tempItem->next;
		}
	}
	return count;
}

int dhcpv6_snp_tbl_mac_hash_foreach
(
void
)
{
	int count = 0;
	unsigned int key = 0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCPv6_Snp_Hash_Table[key];
		while(tempItem) {
			count++;
			tempItem = tempItem->next;
		}
	}
	return count;
}
int dhcp_snp_tbl_ip_hash_foreach
(
	void
)
{
	int count = 0;
	unsigned int key = 0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCP_Snp_Hash_Table_ip[key];
		while(tempItem) {
			count++;
			tempItem = tempItem->ip_next;
		}
	}
	return count;
}
void dhcpv6_snp_copy_bind_table(NPD_DHCPv6_SNP_TBL_ITEM_T *item, int total)
{
	int count = 0;
	unsigned int key = 0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;
	if (!item || (total < 0)) {
		return;
	}
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCPv6_Snp_Hash_Table[key];
		while(tempItem) {
			if (count >= total) {
				return;
			}
			memcpy(&item[count], tempItem, sizeof(NPD_DHCPv6_SNP_TBL_ITEM_T));
			count++;
			tempItem = tempItem->next;
		}
	}
	return;
}

void dhcp_snp_copy_bind_table(NPD_DHCP_SNP_TBL_ITEM_T *item, int total)
{
	int count = 0;
	unsigned int key = 0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	if (!item || (total < 0)) {
		return;
	}
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCP_Snp_Hash_Table[key];
		while(tempItem) {
			if (count >= total) {
				return;
			}
			memcpy(&item[count], tempItem, sizeof(NPD_DHCP_SNP_TBL_ITEM_T));
			count++;
			tempItem = tempItem->next;
		}
	}
	return;
}

void dhcp_snp_print_tbl
(
	void
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0, ipva = 0;
	NPD_DHCP_SNP_TBL_ITEM_T* item = NULL;

	if (!(dhcp_snp_log_dbug())) {
		return;
	}
	
	syslog_ax_dhcp_snp_dbg("=================dhcp-snooping bind table ===================\n");

	for (i = 0; i < NPD_DHCP_SNP_HASH_TABLE_SIZE; i++)
	{
		item = g_DHCP_Snp_Hash_Table[i];
		while (item != NULL)
		{
			ipva = item->ip_addr;
			/*scan all the bind table, age the expired items*/
			syslog_ax_dhcp_snp_dbg("%-7s:%02x:%02x:%02x:%02x:%02x:%02x ", "mac",item->chaddr[0], 
				item->chaddr[1], item->chaddr[2],item->chaddr[3],item->chaddr[4],item->chaddr[5]);
			syslog_ax_dhcp_snp_dbg("%-7s:%d.%d.%d.%d\n", "ip",(ipva>>24)&0xFF,(ipva>>16)&0xFF,(ipva>>8)&0xFF,ipva&0xFF);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","vlan", item->vlanId);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","port",item->ifindex);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","systime",item->sys_escape);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","lease", item->lease_time);
			syslog_ax_dhcp_snp_dbg("%-7s:%s\n","type",   \
							(NPD_DHCP_SNP_BIND_TYPE_STATIC == item->bind_type) ? "static":"dynamic");
			syslog_ax_dhcp_snp_dbg("%-7s:%s\n","state",  \
							(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state) ? "bound":"unbind");
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","expire",item->cur_expire);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","flag",item->flags);
			syslog_ax_dhcp_snp_dbg("-------------------------------------------------------------\n");

			item = item->next;
		}
	}
	syslog_ax_dhcp_snp_dbg("=============================================================\n");


	return ;
}

void dhcp_snp_print_tbl_ip
(
	void
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0, ipva = 0;
	NPD_DHCP_SNP_TBL_ITEM_T* item = NULL;

	if (!(dhcp_snp_log_dbug())) {
		return;
	}
	
	syslog_ax_dhcp_snp_dbg("============dhcp-snooping bind table (ip hash)===============\n");
	for (i = 0; i < NPD_DHCP_SNP_HASH_TABLE_SIZE; i++)
	{
		item = g_DHCP_Snp_Hash_Table_ip[i];
		while (item != NULL)
		{
			ipva = item->ip_addr;
			syslog_ax_dhcp_snp_dbg("%-7s:%02x:%02x:%02x:%02x:%02x:%02x ", "mac",item->chaddr[0], 
				item->chaddr[1], item->chaddr[2],item->chaddr[3],item->chaddr[4],item->chaddr[5]);
			syslog_ax_dhcp_snp_dbg("%-7s:%d.%d.%d.%d\n", "ip",(ipva>>24)&0xFF,(ipva>>16)&0xFF,(ipva>>8)&0xFF,ipva&0xFF);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","vlan", item->vlanId);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","port",item->ifindex);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","systime",item->sys_escape);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","lease", item->lease_time);
			syslog_ax_dhcp_snp_dbg("%-7s:%s\n","type",   \
							(NPD_DHCP_SNP_BIND_TYPE_STATIC == item->bind_type) ? "static":"dynamic");
			syslog_ax_dhcp_snp_dbg("%-7s:%s\n","state",  \
							(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state) ? "bound":"unbind");
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","expire",item->cur_expire);
			syslog_ax_dhcp_snp_dbg("%-7s:%d\n","flag",item->flags);
			syslog_ax_dhcp_snp_dbg("-------------------------------------------------------------\n");
			item = item->ip_next;
		}
	}
	syslog_ax_dhcp_snp_dbg("=============================================================\n");
	return ;
}


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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *nextItem = NULL;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	for (i = 0; i < NPD_DHCP_SNP_HASH_TABLE_SIZE; i++)
	{
		item = g_DHCP_Snp_Hash_Table[i];
		while (item != NULL)
		{
			nextItem = item->next;
			free(item);
			item = nextItem;
		}
	}

	memset(g_DHCP_Snp_Hash_Table, 0, NPD_DHCP_SNP_HASH_TABLE_SIZE * 4);
	memset(g_DHCP_Snp_Hash_Table_ip, 0, NPD_DHCP_SNP_HASH_TABLE_SIZE * 4);
	ret = dhcp_snp_db_drop(dhcp_snp_db, zDropTbl);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("drop dhcp snp table error, ret %x\n", ret);
	}

	dhcp_snp_db_close(dhcp_snp_db);

    return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 *dhcpv6_snp_tbl_item_find ()
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
void *dhcpv6_snp_tbl_item_find
(
	NPD_DHCPv6_SNP_USER_ITEM_T *user
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	unsigned int match = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;
	NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;


	if (!user) {
		log_error("%s: parameter null\n", __func__);
		return NULL;
	}

	key = dhcp_snp_tbl_hash(user->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		log_error("%s: error calculate the hash value\n", __func__);
		return NULL;
	} 
	
	log_debug("find by mac %s : %s ifindex %d vlanid %d\n", 
		mac2str(user->chaddr),u128ip2str(user->ipv6_addr),
		user->ifindex, user->vlanId);

	tempItem = g_DHCPv6_Snp_Hash_Table[key];
	while((tempItem != NULL) && (!match)) {

		if (memcmp(tempItem->chaddr, user->chaddr, 6) == 0) {
			match = 1;
			item = tempItem;
		}
		tempItem = tempItem->next;
	}

	if (!match)	{
		log_debug("not found  %s %s ifindex %d vlanid %d\n", 
			mac2str(user->chaddr), u128ip2str(user->ipv6_addr),
			user->ifindex, user->vlanId);
		return NULL;
	}


	return item;
}


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
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	unsigned int match = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;


	if (!user) {
		log_error("%s: parameter null\n", __func__);
		return NULL;
	}

	key = dhcp_snp_tbl_hash(user->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		log_error("%s: error calculate the hash value\n", __func__);
		return NULL;
	} 
	
	log_debug("find by mac %s : %s ifindex %d vlanid %d\n", 
		mac2str(user->chaddr),u32ip2str(user->ip_addr),
		user->ifindex, user->vlanId);

	tempItem = g_DHCP_Snp_Hash_Table[key];
	while((tempItem != NULL) && (!match)) {

		if (memcmp(tempItem->chaddr, user->chaddr, 6) == 0) {
			match = 1;
			item = tempItem;
		}
		tempItem = tempItem->next;
	}

	if (!match)	{
		log_debug("not found  %s %s ifindex %d vlanid %d\n", 
			mac2str(user->chaddr), u32ip2str(user->ip_addr),
			user->ifindex, user->vlanId);
		return NULL;
	}


	return item;
}
void *dhcpv6_snp_tbl_item_find_by_ip
(
	unsigned char* ipaddr
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;
	unsigned int key_ipaddr = NPD_DHCP_SNP_INIT_0;
	if (strlen(ipaddr) == 0) {
		syslog_ax_dhcp_snp_err("dhcp snp find item in bind table error, parameter is null\n");
		return NULL;
	}
	memcpy((char*)&key_ipaddr, ipaddr+12, 4);
	key = dhcp_snp_tbl_ip_hash(key_ipaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		syslog_ax_dhcp_snp_err("error in calculate the hash value %d, ip %#x\n", key, ipaddr);
		return NULL;
	}

	syslog_ax_dhcp_snp_dbg("find item by ip %s\n", u128ip2str(ipaddr));

	
	tempItem = g_DHCPv6_Snp_Hash_Table_ip[key];
	while (tempItem) {
		if (!strcmp(tempItem->ipv6_addr ,ipaddr)) {
			syslog_ax_dhcp_snp_dbg("find item by ip %s %02x:%02x:%02x:%02x:%02x:%02x\n", u128ip2str(ipaddr),
				tempItem->chaddr[0], tempItem->chaddr[1], tempItem->chaddr[2], 
				tempItem->chaddr[3], tempItem->chaddr[4], tempItem->chaddr[5]);
			
			break;
		}
		tempItem = tempItem->ip_next;
	}
	return tempItem;
}

void *dhcp_snp_tbl_item_find_by_ip
(
	unsigned int ipaddr
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	if (ipaddr == 0) {
		syslog_ax_dhcp_snp_err("dhcp snp find item in bind table error, parameter is null\n");
		return NULL;
	}
	key = dhcp_snp_tbl_ip_hash(ipaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		syslog_ax_dhcp_snp_err("error in calculate the hash value %d, ip %#x\n", key, ipaddr);
		return NULL;
	}

	syslog_ax_dhcp_snp_dbg("find item by ip %u.%u.%u.%u\n", 
		(ipaddr>>24)&0xff, (ipaddr>>16)&0xff,
		(ipaddr>>8)&0xff, (ipaddr>>0)&0xff);

	
	tempItem = g_DHCP_Snp_Hash_Table_ip[key];
	while (tempItem) {
		if (tempItem->ip_addr == ipaddr) {
			syslog_ax_dhcp_snp_dbg("find item by ip %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x\n", 
				(tempItem->ip_addr>>24)&0xff, (tempItem->ip_addr>>16)&0xff,
				(tempItem->ip_addr>>8)&0xff, (tempItem->ip_addr>>0)&0xff,
				tempItem->chaddr[0], tempItem->chaddr[1], tempItem->chaddr[2], 
				tempItem->chaddr[3], tempItem->chaddr[4], tempItem->chaddr[5]);
			
			break;
		}
		tempItem = tempItem->ip_next;
	}
	return tempItem;
}

/**********************************************************************************
 *dhcpv6_snp_tbl_fill_item ()
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
unsigned int dhcpv6_snp_tbl_fill_item
(
	NPD_DHCPv6_SNP_USER_ITEM_T *user,
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
)
{

    if((user == NULL) || (item == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp fill bind table item error, pointer NULL\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    memcpy(item->chaddr, user->chaddr, 6);
    item->haddr_len     = user->haddr_len;
    item->bind_type     = user->bind_type;
    item->state         = user->state;
    item->ifindex       = user->ifindex;
	memcpy(item->ipv6_addr, user->ipv6_addr, 16);
    item->lease_time    = user->lease_time;
    item->vlanId        = user->vlanId;
    item->sys_escape    = time(0);
	item->cur_expire	= dhcp_snp_get_system_uptime() + user->lease_time;

    return DHCP_SNP_RETURN_CODE_OK;
}

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
)
{

    if((user == NULL) || (item == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp fill bind table item error, pointer NULL\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    memcpy(item->chaddr, user->chaddr, 6);
    item->haddr_len     = user->haddr_len;
    item->bind_type     = user->bind_type;
    item->state         = user->state;
    item->ifindex       = user->ifindex;
    item->ip_addr       = user->ip_addr;
    item->lease_time    = user->lease_time;
    item->vlanId        = user->vlanId;
    item->sys_escape    = time(0);
	item->cur_expire	= dhcp_snp_get_system_uptime() + user->lease_time;

    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcpv6_snp_tbl_item_insert ()
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
void *dhcpv6_snp_tbl_item_insert
(
	NPD_DHCPv6_SNP_USER_ITEM_T *user
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;


	if (user == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp insert item in bind table error, parameter is null\n");
		return NULL;
	}

	item = malloc(sizeof(NPD_DHCPv6_SNP_TBL_ITEM_T));
	if (!item) {
		syslog_ax_dhcp_snp_err("can not malloc the memory\n");			
		return NULL;
	}
	memset(item, 0, sizeof(NPD_DHCPv6_SNP_TBL_ITEM_T));

	ret = dhcpv6_snp_tbl_fill_item(user, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snooping table fill error, ret %x\n", ret);			
		free(item);
		item=NULL;
		return NULL;
	}

	key = dhcp_snp_tbl_hash(user->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE)	{
		syslog_ax_dhcp_snp_err("error in calculate the hash value\n");
		return NULL;
	}

	item->next = g_DHCPv6_Snp_Hash_Table[key];
	g_DHCPv6_Snp_Hash_Table[key] = item;

	/* insert item to db */
/*
	ret = dhcp_snp_db_insert(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("insert dhcp snooping table error, ret %x\n", ret);			
		return NULL;
	}
*/
    return item;
}

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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;


	if (user == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp insert item in bind table error, parameter is null\n");
		return NULL;
	}

	item = malloc(sizeof(NPD_DHCP_SNP_TBL_ITEM_T));
	if (!item) {
		syslog_ax_dhcp_snp_err("can not malloc the memory\n");			
		return NULL;
	}
	memset(item, 0, sizeof(NPD_DHCP_SNP_TBL_ITEM_T));

	ret = dhcp_snp_tbl_fill_item(user, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snooping table fill error, ret %x\n", ret);			
		free(item);
		item=NULL;
		return NULL;
	}

	key = dhcp_snp_tbl_hash(user->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE)	{
		syslog_ax_dhcp_snp_err("error in calculate the hash value\n");
		return NULL;
	}

	item->next = g_DHCP_Snp_Hash_Table[key];
	g_DHCP_Snp_Hash_Table[key] = item;

	/* insert item to db */
/*
	ret = dhcp_snp_db_insert(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("insert dhcp snooping table error, ret %x\n", ret);			
		return NULL;
	}
*/
    return item;
}

void *dhcp_snp_ipv6_tbl_item_insert_ip_hash
(
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *tmp = NULL;
	unsigned int key_ipaddr = NPD_DHCP_SNP_INIT_0;

	
	if ((NULL == item)) {
		log_error("dhcp snp insert item in bind table(ip hash) error, parameter is null\n");
		return NULL;
	}

	/* if ip address is 0.0.0.0 don't insert into ip hash */
	if (!strlen(item->ipv6_addr)) {
		return NULL;
	}

	while (tmp = dhcpv6_snp_tbl_item_find_by_ip(item->ipv6_addr)) {
		dhcpv6_snp_tbl_item_delete_iphash(tmp);
		log_info("%s %s already in ip hash table\n", 
			mac2str(tmp->chaddr), u128ip2str(tmp->ipv6_addr));
	}
	memcpy((char*)&key_ipaddr, item->ipv6_addr+12, 4);
	key = dhcp_snp_tbl_ip_hash(key_ipaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE)	{
		syslog_ax_dhcp_snp_err("error in calculate the ip hash value\n");
		return NULL;
	}
	item->ip_next = g_DHCPv6_Snp_Hash_Table_ip[key];
	g_DHCPv6_Snp_Hash_Table_ip[key] = item;

    return item;
}

void *dhcp_snp_tbl_item_insert_ip_hash
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *tmp = NULL;

	
	if ((NULL == item)) {
		log_error("dhcp snp insert item in bind table(ip hash) error, parameter is null\n");
		return NULL;
	}

	/* if ip address is 0.0.0.0 don't insert into ip hash */
	if (!item->ip_addr) {
		return NULL;
	}

	while (tmp = dhcp_snp_tbl_item_find_by_ip(item->ip_addr)) {
		dhcp_snp_tbl_item_delete_iphash(tmp);
		log_info("%s %s already in ip hash table\n", 
			mac2str(tmp->chaddr), u32ip2str(tmp->ip_addr));
	}
	
	key = dhcp_snp_tbl_ip_hash(item->ip_addr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE)	{
		syslog_ax_dhcp_snp_err("error in calculate the ip hash value\n");
		return NULL;
	}
	item->ip_next = g_DHCP_Snp_Hash_Table_ip[key];
	g_DHCP_Snp_Hash_Table_ip[key] = item;

    return item;
}

/**********************************************************************************
 *dhcp_snp_update_timestamp ()
 *
 *	DESCRIPTION:
 *		update the user bind table expire time
 *
 *	INPUTS:
 *		NPD_DHCP_SNP_TBL_ITEM_T *item 
 *
 *	OUTPUTS:
 *		
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
inline int dhcp_snp_update_timestamp
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
)
{
	if (!item) {
		return -1;
	}
	item->cur_expire = dhcp_snp_get_system_uptime() + NPD_DHCP_SNP_REQUEST_TIMEOUT;
	return 0;
}

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
unsigned int dhcpv6_snp_tbl_item_delete
(
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
    unsigned int key = NPD_DHCP_SNP_INIT_0;
    NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;


	if (item  == NULL) {
		log_error("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	if (strlen(item->ipv6_addr)) {
		dhcpv6_snp_tbl_item_delete_iphash(item);
	}
	
	key = dhcp_snp_tbl_hash(item->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		log_error("error in calculate the hash value\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	} 

	if (g_DHCPv6_Snp_Hash_Table[key] == item)
	{
		g_DHCPv6_Snp_Hash_Table[key] = item->next;
		//ret = dhcp_snp_db_delete(dhcp_snp_db, item);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			log_error("delete dhcp snooping table error, ret %x\n", ret);			
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		free(item);
		item=NULL;
		return DHCP_SNP_RETURN_CODE_OK;
	}

	tempItem = g_DHCPv6_Snp_Hash_Table[key];
	while(tempItem && tempItem->next)
	{
		if (tempItem->next == item)
		{
			tempItem->next = item->next;
			//ret = dhcp_snp_db_delete(dhcp_snp_db, item);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("delete dhcp snooping table error, ret %x\n", ret);			
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			free(item);

			return DHCP_SNP_RETURN_CODE_OK;
		}

		tempItem = tempItem->next;
	}

	log_error("no found the special entry, delete fail\n");	
	return DHCP_SNP_RETURN_CODE_ERROR;
}

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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
    unsigned int key = NPD_DHCP_SNP_INIT_0;
    NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;


	if (item  == NULL) {
		log_error("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	if (item->ip_addr) {
		dhcp_snp_tbl_item_delete_iphash(item);
	}
	
	key = dhcp_snp_tbl_hash(item->chaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		log_error("error in calculate the hash value\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	} 

	if (g_DHCP_Snp_Hash_Table[key] == item)
	{
		g_DHCP_Snp_Hash_Table[key] = item->next;
		ret = dhcp_snp_db_delete(dhcp_snp_db, item);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			log_error("delete dhcp snooping table error, ret %x\n", ret);			
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		free(item);
		item=NULL;
		return DHCP_SNP_RETURN_CODE_OK;
	}

	tempItem = g_DHCP_Snp_Hash_Table[key];
	while(tempItem && tempItem->next)
	{
		if (tempItem->next == item)
		{
			tempItem->next = item->next;
			ret = dhcp_snp_db_delete(dhcp_snp_db, item);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("delete dhcp snooping table error, ret %x\n", ret);			
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			free(item);

			return DHCP_SNP_RETURN_CODE_OK;
		}

		tempItem = tempItem->next;
	}

	log_error("no found the special entry, delete fail\n");	
	return DHCP_SNP_RETURN_CODE_ERROR;
}
unsigned int dhcpv6_snp_tbl_item_delete_iphash
(
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
)
{
    unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;
	unsigned int key_ipaddr = NPD_DHCP_SNP_INIT_0;

	if ((item  == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
	if (!strlen(item->ipv6_addr)) {
		return DHCP_SNP_RETURN_CODE_OK;
	}
	memcpy((char*)&key_ipaddr, (item->ipv6_addr)+12, 4);
	key = dhcp_snp_tbl_ip_hash(key_ipaddr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		syslog_ax_dhcp_snp_err("error in calculate the hash value %#x\n", item->ipv6_addr);
		return DHCP_SNP_RETURN_CODE_ERROR;
	} 

	if (g_DHCPv6_Snp_Hash_Table_ip[key] == item) {	
		g_DHCPv6_Snp_Hash_Table_ip[key] = item->ip_next;
		item->ip_next = NULL;
		syslog_ax_dhcp_snp_dbg("delete table from ip hash %s, %02x:%02x:%02x:%02x:%02x:%02x\n", 
			u128ip2str(item->ipv6_addr),
			item->chaddr[0], item->chaddr[1], item->chaddr[2], 
			item->chaddr[3], item->chaddr[4], item->chaddr[5]);
		
		return DHCP_SNP_RETURN_CODE_OK;
	}

	tempItem = g_DHCPv6_Snp_Hash_Table_ip[key];
	while(tempItem && tempItem->ip_next) {
		if (tempItem->ip_next == item) {
			tempItem->ip_next = item->ip_next;
			item->ip_next = NULL;

			syslog_ax_dhcp_snp_dbg("delete table from ip hash %s, %02x:%02x:%02x:%02x:%02x:%02x\n", 
				u128ip2str(item->ipv6_addr),
				item->chaddr[0], item->chaddr[1], item->chaddr[2], 
				item->chaddr[3], item->chaddr[4], item->chaddr[5]);

			return DHCP_SNP_RETURN_CODE_OK;
		}
		tempItem = tempItem->ip_next;
	}
	return DHCP_SNP_RETURN_CODE_ERROR;
}
unsigned int dhcp_snp_tbl_item_delete_iphash
(
	NPD_DHCP_SNP_TBL_ITEM_T *item
)
{
    unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;

	if ((item  == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp delete bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
	if (!item->ip_addr) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	key = dhcp_snp_tbl_ip_hash(item->ip_addr);
	if (key >= NPD_DHCP_SNP_HASH_TABLE_SIZE) {
		syslog_ax_dhcp_snp_err("error in calculate the hash value %#x\n", item->ip_addr);
		return DHCP_SNP_RETURN_CODE_ERROR;
	} 

	if (g_DHCP_Snp_Hash_Table_ip[key] == item) {	
		g_DHCP_Snp_Hash_Table_ip[key] = item->ip_next;
		item->ip_next = NULL;
		syslog_ax_dhcp_snp_dbg("delete table from ip hash %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x\n", 
			(item->ip_addr>>24)&0xff, (item->ip_addr>>16)&0xff,
			(item->ip_addr>>8)&0xff, (item->ip_addr>>0)&0xff,
			item->chaddr[0], item->chaddr[1], item->chaddr[2], 
			item->chaddr[3], item->chaddr[4], item->chaddr[5]);
		
		return DHCP_SNP_RETURN_CODE_OK;
	}

	tempItem = g_DHCP_Snp_Hash_Table_ip[key];
	while(tempItem && tempItem->ip_next) {
		if (tempItem->ip_next == item) {
			tempItem->ip_next = item->ip_next;
			item->ip_next = NULL;

			syslog_ax_dhcp_snp_dbg("delete table from ip hash %u.%u.%u.%u %02x:%02x:%02x:%02x:%02x:%02x\n", 
				(item->ip_addr>>24)&0xff, (item->ip_addr>>16)&0xff,
				(item->ip_addr>>8)&0xff, (item->ip_addr>>0)&0xff,
				item->chaddr[0], item->chaddr[1], item->chaddr[2], 
				item->chaddr[3], item->chaddr[4], item->chaddr[5]);

			return DHCP_SNP_RETURN_CODE_OK;
		}
		tempItem = tempItem->ip_next;
	}
	return DHCP_SNP_RETURN_CODE_ERROR;
}



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
int dhcp_snp_item_destroy(NPD_DHCP_SNP_TBL_ITEM_T *item)
{
	int ret = 0;
	
	if (!item) {
		log_error("%s: parameter NULL", __func__);
		return -1;
	}
	
	if (item->ip_addr) {
		if((0xFFFF == item->vlanId) && (NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			/* delete static arp */
			ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
												item->ifindex, item->ip_addr, item->chaddr);
			if(DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("dhcp snp release item del ip neigh error %x\n", ret);
			}

			dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_DEL_E);
		}
	}
	/* delete from ip hash/mac hash, then free item */
	dhcp_snp_tbl_item_delete(item);
	
	return 0;
}


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
)
{
	syslog_ax_dhcp_snp_dbg("get item into dhcp snooping hash table.\n");		  

    if ((user  == NULL) || (item == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp get bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    memset(user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));

    user->bind_type    = item->bind_type;
    user->state        = item->state;
    user->haddr_len    = item->haddr_len;
    user->vlanId       = item->vlanId;
    user->ip_addr      = item->ip_addr;
    user->lease_time   = item->lease_time;
    user->sys_escape   = item->sys_escape;
    user->cur_expire   = item->cur_expire;
    user->ifindex      = item->ifindex;
    user->flags        = item->flags;    
    memcpy(user->chaddr, item->chaddr, item->haddr_len);

	syslog_ax_dhcp_snp_dbg("get item into dhcp snooping hash table, success.\n");		  

    return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 *dhcpv6_snp_tbl_fill_bind ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCPv6_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCPv6_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
void dhcpv6_snp_tbl_fill_bind
(
	NPD_DHCPv6_SNP_USER_ITEM_T *user,
	NPD_DHCPv6_SNP_TBL_ITEM_T *item
)
{
    if ((!user) || (!item)) {
		log_error("dhcp snp fill bind table item error, parameter is null\n");
		return ;
    }
	if (NPD_DHCP_SNP_REQUEST_TIMEOUT != user->lease_time) {
		item->cur_expire	= dhcp_snp_get_system_uptime() + user->lease_time;
		
	    item->lease_time = user->lease_time;
	}
	
	item->ifindex = user->ifindex;		/* update ifindex */
    item->state      = user->state;
	memcpy(item->ipv6_addr, user->ipv6_addr, 16);
    item->sys_escape = time(0);    
	/*	
	item->cur_expire = user->lease_time + now.tv_sec;	
	*/
	
    return ;
}


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
)
{
    if ((!user) || (!item)) {
		log_error("dhcp snp fill bind table item error, parameter is null\n");
		return ;
    }
	if (NPD_DHCP_SNP_REQUEST_TIMEOUT != user->lease_time) {
		item->cur_expire	= dhcp_snp_get_system_uptime() + user->lease_time;
		
	    item->lease_time = user->lease_time;
	}
	
	item->ifindex = user->ifindex;		/* update ifindex */
    item->state      = user->state;
    item->ip_addr    = user->ip_addr;
    item->sys_escape = time(0);    
	/*	
	item->cur_expire = user->lease_time + now.tv_sec;	
	*/
	
    return ;
}

/**********************************************************************************
 *dhcpv6_snp_tbl_item_insert ()
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
unsigned int dhcpv6_snp_tbl_identity_item
(
	NPD_DHCPv6_SNP_TBL_ITEM_T *item,
	NPD_DHCPv6_SNP_USER_ITEM_T *user
)
{
    if ((item == NULL) || (user  == NULL)) {
		log_error("dhcp snp identity bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    if ((item->vlanId != user->vlanId) ||
		(item->haddr_len != user->haddr_len))
    {
        log_error("vid is not the same\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }
	
	if ((user->ifindex != 0) && (item->ifindex != user->ifindex)) {
    /*if (item->ifindex != user->ifindex) {*/
		syslog_ax_dhcp_snp_err("have different ifindex\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

    if (memcmp(item->chaddr, user->chaddr, item->haddr_len) != 0) {
		log_error("have different chaddr value\n");        
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

    return DHCP_SNP_RETURN_CODE_OK;
}

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
)
{
    if ((item == NULL) || (user  == NULL)) {
		log_error("dhcp snp identity bind table item error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

    if ((item->vlanId != user->vlanId) ||
		(item->haddr_len != user->haddr_len))
    {
        log_error("vid is not the same\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }
	
	if ((user->ifindex != 0) && (item->ifindex != user->ifindex)) {
    /*if (item->ifindex != user->ifindex) {*/
		syslog_ax_dhcp_snp_err("have different ifindex\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

    if (memcmp(item->chaddr, user->chaddr, item->haddr_len) != 0) {
		log_error("have different chaddr value\n");        
		return DHCP_SNP_RETURN_CODE_ERROR;
    }

    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcpv6_snp_tbl_refresh_bind ()
 *
 *	DESCRIPTION:
 *		fill the bind table according user information
 *
 *	INPUTS:
 *		NPD_DHCPv6_SNP_USER_ITEM_T *user
 *
 *	OUTPUTS:
 *		NPD_DHCPv6_SNP_TBL_ITEM_T *item
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, paramter is null
 *
 ***********************************************************************************/
unsigned int dhcpv6_snp_tbl_refresh_bind
(
	NPD_DHCPv6_SNP_USER_ITEM_T *user,
	NPD_DHCPv6_SNP_TBL_ITEM_T *item,
	struct dhcp_snp_listener *node	
)
{
	NPD_DHCPv6_SNP_TBL_ITEM_T *tmp = NULL;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};


    if ((!item) || (!user) || (!node)) {
		log_error("dhcp snp refresh item in bind table error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	/* check ip address */
	if (check_ipv6_address(user->ipv6_addr)) {
		log_error("%s: invalid ip address %s\n", 
			u128ip2str(user->ipv6_addr), mac2str(user->chaddr));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	/*
	ret = dhcp_snp_tbl_identity_item(item, user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snp identity item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	*/
	
	/* special branch for cpu interface or RGMII interface
	  * delete previous ip neigh item
	  */
/*
	if (node->no_arp) {
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
												item->ifindex, item->ip_addr, item->chaddr);
			if(DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("dhcp snp refresh binding item del ip neigh error %x\n", ret);
				return ret;
			}
			dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_DEL_E);
		}
	}
	if (node->add_router) {  //delete router to host,next jump is the interface opening dhcp-snooping
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			
			if(!if_indextoname(item->ifindex, ifname)) {
				syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													item->ifindex, item->ip_addr);
			//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
			//	(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
			//system(command);
			}
	}
*/
	if (strlen(user->ipv6_addr)) {
		while (tmp = dhcpv6_snp_tbl_item_find_by_ip(user->ipv6_addr)) {
			dhcpv6_snp_tbl_item_delete_iphash(tmp);
		}
	}

	/* if item has ip address, delete from ip hash, then insert again */
	if (strlen(item->ipv6_addr)) {
		dhcpv6_snp_tbl_item_delete_iphash(item);
	}
	
	dhcpv6_snp_tbl_fill_bind(user, item);
	if (strlen(item->ipv6_addr)) {
		dhcp_snp_ipv6_tbl_item_insert_ip_hash(item);
	}

#if 0
	/* first, delete */
	ret = dhcp_snp_db_delete(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("delete dhcp snooping table error, ret %x\n", ret);			
	}
	
	/* then, insert record to db*/
	ret = dhcp_snp_db_insert(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("dhcp snp insert item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
#endif
	
	/* special branch for cpu interface or RGMII interface */
/*
	if (node->no_arp) {
	 	if(0xFFFF == item->vlanId) {
			ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_ADD_E,  \
												item->ifindex, item->ip_addr, item->chaddr);
			if(DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("dhcp snp refresh binding item add ip neigh error %x\n", ret);
				return ret;
			}
			else {
				dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_ADD_E);
				dhcp_snp_send_arp_solicit(item->ifindex, item->chaddr, item->ip_addr);
			}
		}
	}
	if (node->add_router) { //add router to host,next jump is the interface opening dhcp-snooping
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			
			if(!if_indextoname(item->ifindex, ifname)) {
				syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_ADD_E,  \
													item->ifindex, item->ip_addr);
			//sprintf(command,"sudo route add -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
			//	(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
			//system(command);
			}
	}
*/
	/* update record to db */
/*
	ret = dhcp_snp_db_update(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snp update item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
*/

    return DHCP_SNP_RETURN_CODE_OK;
}

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
)
{
	NPD_DHCP_SNP_TBL_ITEM_T *tmp = NULL;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};


    if ((!item) || (!user) || (!node)) {
		log_error("dhcp snp refresh item in bind table error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	/* check ip address */
	if (dhcp_snp_u32ip_check(user->ip_addr)) {
		log_error("%s: invalid ip address %s %s\n", 
			u32ip2str(user->ip_addr), mac2str(user->chaddr));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	/*
	ret = dhcp_snp_tbl_identity_item(item, user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snp identity item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	*/
	
	/* special branch for cpu interface or RGMII interface
	  * delete previous ip neigh item
	  */
	if (node->no_arp) {
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
												item->ifindex, item->ip_addr, item->chaddr);
			if(DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("dhcp snp refresh binding item del ip neigh error %x\n", ret);
				return ret;
			}
			dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_DEL_E);
		}
	}
	if (node->add_router) {  //delete router to host,next jump is the interface opening dhcp-snooping
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			
			if(!if_indextoname(item->ifindex, ifname)) {
				syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													item->ifindex, item->ip_addr);
			//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
			//	(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
			//system(command);
			}
	}

	if (user->ip_addr) {
		while (tmp = dhcp_snp_tbl_item_find_by_ip(user->ip_addr)) {
			dhcp_snp_tbl_item_delete_iphash(tmp);
		}
	}

	/* if item has ip address, delete from ip hash, then insert again */
	if (item->ip_addr) {
		dhcp_snp_tbl_item_delete_iphash(item);
	}
	
	dhcp_snp_tbl_fill_bind(user, item);
	if (item->ip_addr) {
		dhcp_snp_tbl_item_insert_ip_hash(item);
	}

#if 0
	/* first, delete */
	ret = dhcp_snp_db_delete(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("delete dhcp snooping table error, ret %x\n", ret);			
	}
	
	/* then, insert record to db*/
	ret = dhcp_snp_db_insert(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("dhcp snp insert item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
#endif
	
	/* special branch for cpu interface or RGMII interface */
	if (node->no_arp) {
	 	if(0xFFFF == item->vlanId) {
			ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_ADD_E,  \
												item->ifindex, item->ip_addr, item->chaddr);
			if(DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("dhcp snp refresh binding item add ip neigh error %x\n", ret);
				return ret;
			}
			else {
				dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_ADD_E);
				dhcp_snp_send_arp_solicit(item->ifindex, item->chaddr, item->ip_addr);
			}
		}
	}
	if (node->add_router) { //add router to host,next jump is the interface opening dhcp-snooping
		if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
			
			if(!if_indextoname(item->ifindex, ifname)) {
				syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_ADD_E,  \
													item->ifindex, item->ip_addr);
			//sprintf(command,"sudo route add -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
			//	(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
			//system(command);
			}
	}

	/* update record to db */
/*
	ret = dhcp_snp_db_update(dhcp_snp_db, item);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("dhcp snp update item error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
*/

	return DHCP_SNP_RETURN_CODE_OK;
}

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
)
{
	return dhcp_snp_db_query(dhcp_snp_db, sql, count, array);
}

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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	char select_sql[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	if (!isFound) {
		syslog_ax_dhcp_snp_err("query dhcp snp db for arp proxy error, parameter is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	/*
	 ****************************
	 *cat query string 
	 ****************************
	 */
	bufPtr = select_sql;

	/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	loglength += sprintf(bufPtr, "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE IP=");
	bufPtr = select_sql + loglength;

	loglength += sprintf(bufPtr, "%d", sip);
	bufPtr = select_sql + loglength;

	loglength += sprintf(bufPtr, ";");
	bufPtr = select_sql + loglength;

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_tbl_query(select_sql, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

		if (record_count > 0) {
			*isFound = 1;/* found */
			ret = DHCP_SNP_RETURN_CODE_OK;
		}
		else {
			*isFound = 0;/* not found */
			ret = DHCP_SNP_RETURN_CODE_OK;
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_ERROR;
	}

	return ret;
}

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
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	char select_sql[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;

	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	if (!result) {
		syslog_ax_dhcp_snp_err("query dhcp snp db for arp proxy error, parameter is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	/*
	 ****************************
	 *cat query string 
	 ****************************
	 */
	bufPtr = select_sql;

	/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	loglength += sprintf(bufPtr, "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE IP=");
	bufPtr = select_sql + loglength;

	loglength += sprintf(bufPtr, "%d", dip);
	bufPtr = select_sql + loglength;

	loglength += sprintf(bufPtr, ";");
	bufPtr = select_sql + loglength;

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_tbl_query(select_sql, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

		if (record_count >= 1) {
			/* found */
			result->bind_type  = show_items[0].bind_type;
			memcpy(result->chaddr, show_items[0].chaddr, 12);
			result->ip_addr    = show_items[0].ip_addr;
			result->ifindex    = show_items[0].ifindex;
			result->vlanId     = show_items[0].vlanId;
			result->lease_time = show_items[0].lease_time;

			ret = DHCP_SNP_RETURN_CODE_OK;
		}
		else {
			/* not found */
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_ERROR;
	}

	return ret;
}

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
)
{
	return dhcp_snp_db_insert(dhcp_snp_db, &item);
}

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
)
{
	return dhcp_snp_db_delete(dhcp_snp_db, &item);
}

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
)
{
	return dhcp_snp_db_insert(dhcp_snp_db, &item);
}

unsigned int dhcp_snp_get_system_uptime(void)
{
#if 0
	struct timeval now;
   	gettimeofday(&now, NULL);
    return now.tv_sec;
#else
#define SYSTEM_PROC_UPTIME_FILE "/proc/uptime"
    static int system_uptime_fd = -1; 
    unsigned long curtime = 0;
    char buf[64] = {0};
    if (system_uptime_fd < 0) {
        if ((system_uptime_fd = open(SYSTEM_PROC_UPTIME_FILE, O_RDONLY)) < 0) {
            syslog_ax_dhcp_snp_err("open file %s failed\n", SYSTEM_PROC_UPTIME_FILE);
            return 0;  
        }   
    }   
    lseek(system_uptime_fd, 0L, SEEK_SET);
    read(system_uptime_fd, buf, sizeof(buf)-1);
    curtime = strtoul(buf, NULL, 10); 
    return (unsigned int)curtime;
#endif
}

#if 0
int debug_tbl(int count)
{
	int i = 0;
	char haddr[6] = {0x00, 0x1f, 0x2e, 0x00, 0x00, 0x00};
	struct unresolved_table table;

	memset(&table, 0, sizeof(table));
	table.bssindex = 128;
	table.vrrpid = 0;
	table.ifindex = 0;
	memcpy(table.chaddr, haddr, 6);

	table.chaddr[3] = count;
	
	for (i=0; i< 512; i++) {
		table.chaddr[5]++;
		if (0 == table.chaddr[5]) {
			table.chaddr[4]++;
		}
		
		dhcp_snp_add_unresolved_node(&table);		
	}
}
#endif


void * dhcp_snp_tbl_thread_aging
(
	void *arg
)
{
	unsigned char status = NPD_DHCP_SNP_INIT_0;	
	log_info("dhcp snooping aging handler thread start!\n");
	/* tell my thread pid*/
	dhcp_snp_tell_whoami("dhcpsnp aging", 0);

	dhcp_snp_setaffinity(~0UL);
	//debug_tbl(1398);
	while (1) {
		pthread_mutex_lock(&mutexDhcpsnptbl);

		/* bind table aging */
		dhcp_snp_aging_mechanism();
		dhcpv6_snp_aging_mechanism();


			/* notice asd table aging */
			pthread_mutex_lock(&mutexsnpunsolve);
			dhcp_snp_asd_table_aging_mechanism();
			pthread_mutex_unlock(&mutexsnpunsolve);

			pthread_mutex_lock(&mutexsnpunsolve);
			dhcpv6_snp_asd_table_aging_mechanism();
			pthread_mutex_unlock(&mutexsnpunsolve);

		pthread_mutex_unlock(&mutexDhcpsnptbl);
		
		sleep(dhcp_snp_ageing_time);
	}
	return NULL;
}
int dhcpv6_snp_aging_mechanism(void)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *del_item = NULL;	
	NPD_DHCPv6_SNP_TBL_ITEM_T *tempItem = NULL;
	NPD_DHCPv6_SNP_TBL_ITEM_T *next = NULL;
	unsigned int cur_time = 0;
	int ret = -1;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};
	
	cur_time = dhcp_snp_get_system_uptime();
	
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCPv6_Snp_Hash_Table[key];
		while(tempItem) {

			next = tempItem->next;
			
			log_debug("aging bind-table: %s %s ifindex %d vlanid %d expired %d, curtime %d.\n", 
				mac2str(tempItem->chaddr), u128ip2str(tempItem->ipv6_addr),
				tempItem->ifindex, tempItem->vlanId,
				tempItem->cur_expire, cur_time);
			
			if ((NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == tempItem->bind_type)
				&& (cur_time > tempItem->cur_expire)) {	  
				dhcpv6_snp_tbl_item_delete(tempItem);				
			}

			tempItem = next;
		}
		
	}
	
	return 0;
}

int dhcp_snp_aging_mechanism(void)
{
	unsigned int key = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *del_item = NULL;	
	NPD_DHCP_SNP_TBL_ITEM_T *tempItem = NULL;
	NPD_DHCP_SNP_TBL_ITEM_T *next = NULL;
	unsigned int cur_time = 0;
	int ret = -1;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};
	
	cur_time = dhcp_snp_get_system_uptime();
	
	for (key = 0; key < NPD_DHCP_SNP_HASH_TABLE_SIZE; key++) {
		tempItem = g_DHCP_Snp_Hash_Table[key];
		while(tempItem) {

			next = tempItem->next;
			
			log_debug("aging bind-table: %s %s ifindex %d vlanid %d expired %d, curtime %d.\n", 
				mac2str(tempItem->chaddr), u32ip2str(tempItem->ip_addr),
				tempItem->ifindex, tempItem->vlanId,
				tempItem->cur_expire, cur_time);
			
			if ((NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == tempItem->bind_type)
				&& (cur_time > tempItem->cur_expire)) {	  
				if (tempItem->ip_addr) {
					if((0xFFFF == tempItem->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == tempItem->state)) {
						ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
															tempItem->ifindex, tempItem->ip_addr, tempItem->chaddr);
						 //delete router to host,next jump is the interface opening dhcp-snooping
						if(!if_indextoname(tempItem->ifindex, ifname)) {
							syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", tempItem->ifindex);
							return DHCP_SNP_RETURN_CODE_ERROR;
						}
						dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
															tempItem->ifindex, tempItem->ip_addr);
						//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",(tempItem->ip_addr>>24)&0xff,\
						//(tempItem->ip_addr>>16)&0xff,(tempItem->ip_addr>>8)&0xff,(tempItem->ip_addr>>0)&0xff,ifname);
						//system(command);
									
						if(DHCP_SNP_RETURN_CODE_OK != ret) {
							log_error("dhcp snp release item del ip neigh error %x\n", ret);
						}

						dhcp_snp_listener_handle_host_ebtables(tempItem->chaddr, tempItem->ip_addr, DHCPSNP_EBT_DEL_E);
					}
				}
				dhcp_snp_tbl_item_delete(tempItem);				
			}

			tempItem = next;
		}
		
	}
	
	return 0;
}

int dhcp_snp_add_static_bindtable
(
	unsigned int ip,
	unsigned char *chaddr,
	unsigned int ifindex
)
{
	struct dhcp_snp_static_table *table = NULL;
	if ((!chaddr) || (!ifindex) || (!ip)) {
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	table = (struct dhcp_snp_static_table *)malloc(sizeof(struct dhcp_snp_static_table));
	if (!table) {
		syslog_ax_dhcp_snp_err("add static bind table failed: no memory.\n");		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	memset(table, 0, sizeof(*table));
	table->ipaddr= ip;
	table->ifindex = ifindex;
	memcpy(table->chaddr, chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
	if (!dhcp_snp_static_table_head) {
		dhcp_snp_static_table_head = table;
		table->next = NULL;
	} else {
		table->next = dhcp_snp_static_table_head;
		dhcp_snp_static_table_head = table;
	}
	dhcp_snp_print_static_bindtable();
    return DHCP_SNP_RETURN_CODE_OK;
}
int dhcp_snp_del_static_bindtable
(
	unsigned int ip,
	unsigned char *chaddr,
	unsigned int ifindex
)
{
	struct dhcp_snp_static_table *table = NULL;
	struct dhcp_snp_static_table *next = NULL, *prev = NULL;
	if ((!chaddr) || (!ifindex) || (!ip)) {
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	if (!dhcp_snp_static_table_head) {
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (!memcmp(dhcp_snp_static_table_head->chaddr, chaddr, NPD_DHCP_SNP_MAC_ADD_LEN)) {
		table = dhcp_snp_static_table_head;
		dhcp_snp_static_table_head = table->next;
		free(table);
		return DHCP_SNP_RETURN_CODE_OK;
	}
	prev = dhcp_snp_static_table_head;
	next = dhcp_snp_static_table_head->next;
	while (next) {
		if (!memcmp(next->chaddr, chaddr, NPD_DHCP_SNP_MAC_ADD_LEN)) {
			prev->next = next->next;
			free(next);
			break;
		}
		prev = next;
		next = next->next;
	}
	dhcp_snp_print_static_bindtable();
    return DHCP_SNP_RETURN_CODE_OK;
}
void dhcp_snp_print_static_bindtable
(
	void
)
{
	struct dhcp_snp_static_table *table = NULL;
	if (!dhcp_snp_static_table_head) {
		return ;
	}
	syslog_ax_dhcp_snp_dbg("==============dhcp-snooping static bind table ===============\n");
	table = dhcp_snp_static_table_head;
	while (table) {
		syslog_ax_dhcp_snp_dbg("%-7s:%02x:%02x:%02x:%02x:%02x:%02x ", "mac",
							table->chaddr[0], table->chaddr[1], table->chaddr[2], 
							table->chaddr[3], table->chaddr[4], table->chaddr[5]);
		syslog_ax_dhcp_snp_dbg("%-7s:%d.%d.%d.%d\n", "ip",
							(table->ipaddr>>24)&0xFF, (table->ipaddr>>16)&0xFF, 
							(table->ipaddr>>8)&0xFF, table->ipaddr&0xFF);
		syslog_ax_dhcp_snp_dbg("%-7s:%d\n","ifindex", table->ifindex);
		syslog_ax_dhcp_snp_dbg("-------------------------------------------------------------\n");
		table = table->next;
	}
	syslog_ax_dhcp_snp_dbg("==============================================================\n");
    return ;
}


int init_sock_unix_server(char *path)
{
	int serv_fd = -1;
	struct sockaddr_un sock;
	
	if ((!path) || (strlen(path) > UNIX_PATH_MAX - 1)) {
		return -1;
}
	if ((serv_fd = socket(PF_LOCAL, SOCK_DGRAM, 0)) < 0) {
		syslog_ax_dhcp_snp_err("%s: create sock failed: %m\n", __func__);
		return -1;
	}

	unlink(path);

	memset(&sock, 0, sizeof(sock));
	sock.sun_family = AF_UNIX;
	strncpy(sock.sun_path, path, strlen(path));

	if (-1 == bind(serv_fd, (struct sockaddr*)&sock, sizeof(sock))) {
		syslog_ax_dhcp_snp_err("%s: sock bind error : %m\n", __func__);
        close(serv_fd);
		serv_fd = -1;
        unlink(path);
        return -1;   
    }
	return serv_fd;
}

int init_sock_unix_server_asd(void)
{
	return init_sock_unix_server(DHCP_SNP_SOCK_PATH);
}

int init_sock_unix_client(char *path)
{
	int sockfd = -1;   
    struct sockaddr_un clientaddr;
	
    if ((sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0)) < 0) {   
		log_error("%s: create sock failed: %s\n", __func__, strerror(errno));
        return -1;   
    }     

	memset(&clientaddr, 0, sizeof(clientaddr));
    clientaddr.sun_family = AF_UNIX;   
	/* tmpnam  */
	strncpy(clientaddr.sun_path, tmpnam(NULL), sizeof(clientaddr.sun_path)-1);
    if (-1 == bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr))) {   
		log_error("%s: sock bind error : %s\n", __func__, strerror(errno));
        close(sockfd);   
        return -1;   
	}
	return sockfd;
}

int init_sock_unix_client_asd(int vrrpid, unsigned int local_flag)
{
	char path[UNIX_PATH_MAX];

	if (local_flag) {
		local_flag = 1;	/* local hansi */
	} else {
		local_flag = 0;	/* remote hansi */
	}

	if (fd_table[TBL_INDEX(vrrpid, local_flag)])  {
		return 0;
	}

	if (!(fd_table[TBL_INDEX(vrrpid, local_flag)] = (struct fd_table *)malloc(sizeof(struct fd_table)))) {
		return -1;
	}

	memset(fd_table[TBL_INDEX(vrrpid, local_flag)], 0, sizeof(struct fd_table));

	sprintf(path, "%s%d_%d", DHCP_SNP_ASD_SOCK_PATH, local_flag, vrrpid);

	if ((fd_table[TBL_INDEX(vrrpid, local_flag)]->sock_fd = init_sock_unix_client(path))  < 0) {
		free(fd_table[TBL_INDEX(vrrpid, local_flag)]);
		fd_table[TBL_INDEX(vrrpid, local_flag)] = NULL;
		return -1;
	}

    fd_table[TBL_INDEX(vrrpid, local_flag)]->servaddr.sun_family = AF_UNIX;   
    strncpy(fd_table[TBL_INDEX(vrrpid, local_flag)]->servaddr.sun_path, path, UNIX_PATH_MAX-1);
	
	return 0;
}


void dhcp_snp_print_unresolved_table
(
	void
)
{
	unsigned int i = 0;
	struct unresolved_table *tmp = NULL;

	log_debug("================tbl to asd (debug) begin======================");
	for (i = 0; i < NPD_DHCP_SNP_HASH_TABLE_SIZE; i++) {
		tmp = g_dhcp_snp_unresolved_hash[i];
		for (tmp; tmp; tmp = tmp->next) {
			log_debug("bssindex %d vrrid %d %s %s %s\n", 
				tmp->vrrpid, tmp->bssindex,
				(tmp->local_flag ? "local hansi" : "remote hansi"),
				mac2str(tmp->chaddr), u32ip2str(tmp->ipaddr));
		}
	}
	log_debug("================tbl to asd (debug) end========================");	
    return ;
}


/**********************************************************************************
 *	dhcpv6_snp_find_from_bind_table ()
 *
 *	DESCRIPTION:
 *		find ip by mac from bind table, if find fill ip address, and return 1
 *
 *	INPUTS:
 *		table - unresolved_table
 *
 *	OUTPUTS:
 *		 
 *	RETURN VALUE:
 *		1 - find
 *		0 - not find
 *		-1 - error
 *	
 ***********************************************************************************/
int dhcpv6_snp_find_from_bind_table(struct unresolved_ipv6_table *table)
{
	int ret = -1;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;

	if (!table) {
		return -1;
	}

	log_debug("find from bind table by %s\n", mac2str(table->chaddr));
		
	memset(&user, 0, sizeof(user));
	memcpy(user.chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
	
	if (item = dhcpv6_snp_tbl_item_find(&user)) {
		if ((NPD_DHCP_SNP_BIND_STATE_BOUND == item->state) && strlen(item->ipv6_addr)) {
			memcpy(table->ipv6_addr, item->ipv6_addr, 16);
			table->ifindex = item->ifindex;
			
			log_debug("find item from bind table: %s %s\n", 
				mac2str(table->chaddr), u128ip2str(table->ipv6_addr));
			
			ret = 1;
		}
	} else {
		ret = 0;
	}
	
	return ret;
}


/**********************************************************************************
 *	dhcp_snp_find_from_bind_table ()
 *
 *	DESCRIPTION:
 *		find ip by mac from bind table, if find fill ip address, and return 1
 *
 *	INPUTS:
 *		table - unresolved_table
 *
 *	OUTPUTS:
 *		 
 *	RETURN VALUE:
 *		1 - find
 *		0 - not find
 *		-1 - error
 *	
 ***********************************************************************************/
int dhcp_snp_find_from_bind_table(struct unresolved_table *table)
{
	int ret = -1;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	if (!table) {
		return -1;
	}

	log_debug("find from bind table by %s\n", mac2str(table->chaddr));
		
	memset(&user, 0, sizeof(user));
	memcpy(user.chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
	
	if (item = dhcp_snp_tbl_item_find(&user)) {
		if ((NPD_DHCP_SNP_BIND_STATE_BOUND == item->state) && item->ip_addr) {
			table->ipaddr = item->ip_addr;
			table->ifindex = item->ifindex;
			
			log_debug("find item from bind table: %s %s\n", 
				mac2str(table->chaddr), u32ip2str(table->ipaddr));
			
			ret = 1;
		}
	} else {
		ret = 0;
	}
	
	return ret;
}



/**********************************************************************************
 *	dhcpv6_snp_find_from_unresolved_table()
 *
 *	DESCRIPTION:
 *		find unresolved table if this MAC table exist. if exist return the table
 *
 *	INPUTS:
 *		haddr - MAC
 *
 *	OUTPUTS:
 *		table
 *
 *	RETURN VALUE:
 *		table - if not find, table is NULL
 *		
 ***********************************************************************************/
struct unresolved_ipv6_table *dhcpv6_snp_find_from_unresolved_table(unsigned char *haddr)
{
	unsigned int key = 0;
	struct unresolved_ipv6_table *table = NULL;
	
	key = dhcp_snp_tbl_hash(haddr);

	table = g_dhcpv6_snp_unresolved_hash[key];

	while (table) {
		if (0 == memcmp(table->chaddr, haddr, NPD_DHCP_SNP_MAC_ADD_LEN)) {
			break;
		}
		table = table->next;
	}
	return table;
}

/**********************************************************************************
 *	dhcp_snp_find_from_unresolved_table()
 *
 *	DESCRIPTION:
 *		find unresolved table if this MAC table exist. if exist return the table
 *
 *	INPUTS:
 *		haddr - MAC
 *
 *	OUTPUTS:
 *		table
 *
 *	RETURN VALUE:
 *		table - if not find, table is NULL
 *		
 ***********************************************************************************/
struct unresolved_table *dhcp_snp_find_from_unresolved_table(unsigned char *haddr)
{
	unsigned int key = 0;
	struct unresolved_table *table = NULL;
	
	key = dhcp_snp_tbl_hash(haddr);

	table = g_dhcp_snp_unresolved_hash[key];

	while (table) {
		if (0 == memcmp(table->chaddr, haddr, NPD_DHCP_SNP_MAC_ADD_LEN)) {
			break;
		}
		table = table->next;
	}
	return table;
}

/**********************************************************************************
 *	dhcpv6_snp_add_unresolved_node()
 *
 *	DESCRIPTION:
 *		add to unresolved table. if exist, just update the date
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
 ***********************************************************************************/
int dhcpv6_snp_add_unresolved_node(struct unresolved_ipv6_table *table)
{
	unsigned int key = 0;
	struct unresolved_ipv6_table *tmp = NULL;

	if (!table) {
		log_error("%s: parameter null.\n", __func__);
		return -1;
	}

	log_info("add table: bssindex %d vrrid %d %s %s\n", 
		table->bssindex, table->vrrpid,
		mac2str(table->chaddr), u128ip2str(table->ipv6_addr));

	if (tmp = dhcpv6_snp_find_from_unresolved_table(table->chaddr)) {
		tmp->bssindex = table->bssindex;
		memcpy(tmp->ipv6_addr, table->ipv6_addr, 16);
		tmp->vrrpid = table->vrrpid;
		tmp->local_flag = table->local_flag;
		memset(tmp->chaddr, 0 , NPD_DHCP_SNP_MAC_ADD_LEN);
		memcpy(tmp->chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
		tmp->expired = dhcp_snp_get_system_uptime() + TBL_EXPIRED_TIME;
		
	} else {
		tmp = (struct unresolved_ipv6_table *)malloc(sizeof(struct unresolved_ipv6_table));
		if (NULL == tmp) {
			return -1;
		}
		memset(tmp, 0, sizeof(*tmp));
		memcpy(tmp, table, sizeof(*table));
		tmp->expired = dhcp_snp_get_system_uptime() + TBL_EXPIRED_TIME;
		tmp->next = NULL;

		key = dhcp_snp_tbl_hash(table->chaddr);

		tmp->next = g_dhcpv6_snp_unresolved_hash[key];
		g_dhcpv6_snp_unresolved_hash[key] = tmp;
	}

	return 0;	
}

/**********************************************************************************
 *	dhcp_snp_add_unresolved_node()
 *
 *	DESCRIPTION:
 *		add to unresolved table. if exist, just update the date
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
 ***********************************************************************************/
int dhcp_snp_add_unresolved_node(struct unresolved_table *table)
{
	unsigned int key = 0;
	struct unresolved_table *tmp = NULL;

	if (!table) {
		log_error("%s: parameter null.\n", __func__);
		return -1;
	}

	log_info("add table: bssindex %d vrrid %d %s %s\n", 
		table->bssindex, table->vrrpid,
		mac2str(table->chaddr), u32ip2str(table->ipaddr));

	if (tmp = dhcp_snp_find_from_unresolved_table(table->chaddr)) {
		tmp->bssindex = table->bssindex;
		tmp->ipaddr = table->ipaddr;
		tmp->vrrpid = table->vrrpid;
		tmp->local_flag = table->local_flag;
		memcpy(tmp->chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
		tmp->expired = dhcp_snp_get_system_uptime() + TBL_EXPIRED_TIME;
		
	} else {
		tmp = (struct unresolved_table *)malloc(sizeof(struct unresolved_table));
		if (NULL == tmp) {
			return -1;
		}
		memset(tmp, 0, sizeof(*tmp));
		memcpy(tmp, table, sizeof(*table));
		tmp->expired = dhcp_snp_get_system_uptime() + TBL_EXPIRED_TIME;
		tmp->next = NULL;

		key = dhcp_snp_tbl_hash(table->chaddr);

		tmp->next = g_dhcp_snp_unresolved_hash[key];
		g_dhcp_snp_unresolved_hash[key] = tmp;
	}

	return 0;	
}

#if 0
/**********************************************************************************
 *	dhcp_snp_del_expired_node()
 *
 *	DESCRIPTION:
 *		delete expired node from unresolved table.
 *
 *	INPUTS:
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		0 - no table delete
 *		1 - delete one table
 *		
 ***********************************************************************************/
int dhcp_snp_del_expired_node(unsigned int key)
{
	struct unresolved_table *tmp = NULL;
	struct unresolved_table *deltbl = NULL;		/* delet table */

	unsigned int cur_time = 0;

	cur_time = dhcp_snp_get_system_uptime();
	
	tmp = g_dhcp_snp_unresolved_hash[key];
	if (!tmp) {
		return 0;
	}

	if (tmp->expired < cur_time) {
		g_dhcp_snp_unresolved_hash[key] = g_dhcp_snp_unresolved_hash[key]->next;
		free(tmp);
		return 1;
	}

	while (tmp && tmp->next) {
		if (tmp->next->expired < cur_time) {
			deltbl = tmp->next;
			tmp->next = deltbl->next;
			free(deltbl);

			return 1;
		}
		tmp = tmp->next;
	}
	
	return 0;
}
#endif


/******************************************************************************
 *	dhcp_snp_del_unresolved_node()
 *
 *	DESCRIPTION:
 *		delete node from unresolved table.
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
int dhcp_snp_del_unresolved_node(struct unresolved_table *table)
{
	unsigned int key = 0;
	struct unresolved_table *tmp = NULL;
	struct unresolved_table *deltbl = NULL;		/* delet table */

	if (!table) {
		log_error("%s: parameter null.\n", __func__);		
		return -1;
	}
	log_info("delete table(to asd): bss index %d vrrid %d %s %s %s\n", 
		table->bssindex, table->vrrpid,
		(table->local_flag ? "local hansi" : "remote hansi"),
		mac2str(table->chaddr),	u32ip2str(table->ipaddr));
		
	key = dhcp_snp_tbl_hash(table->chaddr);

	tmp = g_dhcp_snp_unresolved_hash[key];
	if (!tmp) {
		return 0;
	}

	if (!(memcmp(tmp->chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN))) {
		g_dhcp_snp_unresolved_hash[key] = g_dhcp_snp_unresolved_hash[key]->next;
		free(tmp);
		return 0;
	}

	while (tmp && tmp->next) {
		if (!(memcmp(tmp->next->chaddr, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN))) {
			deltbl = tmp->next;
			tmp->next = deltbl->next;
			free(deltbl);
			return 0;
		}
		tmp = tmp->next;
	}
	return -1;	
}

/******************************************************************************
 *	dhcpv6_snp_asd_table_aging_mechanism()
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
int dhcpv6_snp_asd_table_aging_mechanism(void)
{
	unsigned int cur_time = 0;
	unsigned int key = 0;
	struct unresolved_ipv6_table *tmp = NULL;

	struct unresolved_ipv6_table  *next = NULL;

	/* get system uptime */
	cur_time = dhcp_snp_get_system_uptime();

	for(key = 0 ; key < NPD_DHCP_SNP_HASH_TABLE_SIZE ; key++)
	{
		if(g_dhcpv6_snp_unresolved_hash[key] == NULL)
			continue;
		while(g_dhcpv6_snp_unresolved_hash[key]->expired < cur_time)
		{
			tmp = g_dhcpv6_snp_unresolved_hash[key];
			g_dhcpv6_snp_unresolved_hash[key] = g_dhcpv6_snp_unresolved_hash[key]->next;
			free(tmp);
			tmp = NULL;
			if(g_dhcpv6_snp_unresolved_hash[key] == NULL)
				break;
		}
		tmp = g_dhcpv6_snp_unresolved_hash[key];
		if(tmp == NULL)
			continue;
		next = tmp->next;
		while((tmp!= NULL)&&(next!=NULL))
		{
			if(next->expired < cur_time)
			{
				tmp->next = next->next;
				free(next);
				
			}
			else{
				tmp = tmp->next;
			}			
			next = tmp->next;
		}
	}
	return 0;	
}


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
int dhcp_snp_asd_table_aging_mechanism(void)
{
	unsigned int cur_time = 0;
	unsigned int key = 0;
	struct unresolved_table *tmp = NULL;

	struct unresolved_table  *next = NULL;

	/* get system uptime */
	cur_time = dhcp_snp_get_system_uptime();

	for(key = 0 ; key < NPD_DHCP_SNP_HASH_TABLE_SIZE ; key++)
	{
		if(g_dhcp_snp_unresolved_hash[key] == NULL)
			continue;
		while(g_dhcp_snp_unresolved_hash[key]->expired < cur_time)
		{
			tmp = g_dhcp_snp_unresolved_hash[key];
			g_dhcp_snp_unresolved_hash[key] = g_dhcp_snp_unresolved_hash[key]->next;
			free(tmp);
			tmp = NULL;
			if(g_dhcp_snp_unresolved_hash[key] == NULL)
				break;
		}
		tmp = g_dhcp_snp_unresolved_hash[key];
		if(tmp == NULL)
			continue;
		next = tmp->next;
		while((tmp!= NULL)&&(next!=NULL))
		{
			if(next->expired < cur_time)
			{
				tmp->next = next->next;
				free(next);
				
			}
			else{
				tmp = tmp->next;
			}			
			next = tmp->next;
		}
	}
	return 0;	
}

/**********************************************************************************
 *	__fill_tablemsg_ipv6 ()
 *
 *	DESCRIPTION:
 *		fill tableMsg frem unresolved table
 *
 *	INPUTS:
 *		msg - TableMsg
 *		table - unresolved table
 *
 *	OUTPUTS:
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
inline void __fill_tablemsg_ipv6(TableMsg *msg, struct unresolved_ipv6_table *table)
{
	unsigned char ifname[16];
	memset(ifname,0,sizeof(ifname));
	msg->Type = STA_TYPE;
	msg->Op = DHCP_IPv6;
	msg->u.STA.BSSIndex = table->bssindex;
	msg->u.STA.local = table->local_flag;

    memcpy(msg->u.STA.ipv6Address.s6_addr, table->ipv6_addr, 16);
	log_debug("__fill_tablemsg_ipv6 ipv6_address of sta is: %X:%X:%X:%X:%X:%X:%X:%X .\n", \
		msg->u.STA.ipv6Address.in6_u.u6_addr16[0],msg->u.STA.ipv6Address.in6_u.u6_addr16[1],msg->u.STA.ipv6Address.in6_u.u6_addr16[2],    \
		msg->u.STA.ipv6Address.in6_u.u6_addr16[3],msg->u.STA.ipv6Address.in6_u.u6_addr16[4],msg->u.STA.ipv6Address.in6_u.u6_addr16[5],    \
		msg->u.STA.ipv6Address.in6_u.u6_addr16[6],msg->u.STA.ipv6Address.in6_u.u6_addr16[7]);
	
	memcpy(msg->u.STA.STAMAC, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
	if(if_indextoname(table->ifindex, ifname)) 
	{
		memset(msg->u.STA.arpifname,0,sizeof(msg->u.STA.arpifname));
		memcpy(msg->u.STA.arpifname,ifname,sizeof(ifname));
	}	
}

/**********************************************************************************
 *	__fill_tablemsg ()
 *
 *	DESCRIPTION:
 *		fill tableMsg frem unresolved table
 *
 *	INPUTS:
 *		msg - TableMsg
 *		table - unresolved table
 *
 *	OUTPUTS:
 *
 *	RETURN VALUE:
 *
 ***********************************************************************************/
inline void __fill_tablemsg(TableMsg *msg, struct unresolved_table *table)
{
	unsigned char ifname[16];
	memset(ifname,0,sizeof(ifname));
	msg->Type = STA_TYPE;
	msg->Op = DHCP_IP;
	msg->u.STA.BSSIndex = table->bssindex;
	msg->u.STA.local = table->local_flag;
	msg->u.STA.ipv4Address = table->ipaddr;
	
	memcpy(msg->u.STA.STAMAC, table->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);
	if(if_indextoname(table->ifindex, ifname)) 
	{
		memset(msg->u.STA.arpifname,0,sizeof(msg->u.STA.arpifname));
		memcpy(msg->u.STA.arpifname,ifname,sizeof(ifname));
	}	
}
int dhcpv6_snp_notify_asd(struct unresolved_ipv6_table *table)
{
	TableMsg msg;
	
	if (!table) {
		return -1;
	}

	if (!fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]) {
		init_sock_unix_client_asd(table->vrrpid, table->local_flag);
		if (!fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]) {
			return -1;
		}
	}

	memset(&msg, 0, sizeof(msg));

	__fill_tablemsg_ipv6(&msg, table);	
	
	log_info("msg to asd: bssindex %d vrrid %d %s %s %s\n",
		table->bssindex, table->vrrpid,
		(table->local_flag ? "local hansi" : "remote hansi"),
		mac2str(table->chaddr), u128ip2str(table->ipv6_addr));

	if (sendto(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->sock_fd, (char *)&msg, sizeof(msg), 0, 
				(struct sockaddr *)&(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr),  
				sizeof(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr)) < 0) {
		log_error("msg (bssindex %d vrrpid %d %s %s %s)"
			" send to asd failed. %m\n",
			table->bssindex, table->vrrpid, 
			(table->local_flag ? "local hansi" : "remote hansi"),
			mac2str(table->chaddr), u128ip2str(table->ipv6_addr));

		log_error("socket %d path %s\n", 
			fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->sock_fd, 
			fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr.sun_path);
		
		return -1;
	}

	dhcp_snp_del_unresolved_node(table);

	return 0;
}
int dhcp_snp_notify_asd(struct unresolved_table *table)
{
	TableMsg msg;
	
	if (!table) {
		return -1;
	}

	if (!fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]) {
		init_sock_unix_client_asd(table->vrrpid, table->local_flag);
		if (!fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]) {
			return -1;
		}
	}

	memset(&msg, 0, sizeof(msg));

	__fill_tablemsg(&msg, table);	
	
	log_info("msg to asd: bssindex %d vrrid %d %s %s %s\n",
		table->bssindex, table->vrrpid,
		(table->local_flag ? "local hansi" : "remote hansi"),
		mac2str(table->chaddr), u32ip2str(table->ipaddr));

	if (sendto(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->sock_fd, (char *)&msg, sizeof(msg), 0, 
				(struct sockaddr *)&(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr),  
				sizeof(fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr)) < 0) {
		log_error("msg (bssindex %d vrrpid %d %s %s %s)"
			" send to asd failed. %m\n",
			table->bssindex, table->vrrpid, 
			(table->local_flag ? "local hansi" : "remote hansi"),
			mac2str(table->chaddr), u32ip2str(table->ipaddr));

		log_error("socket %d path %s\n", 
			fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->sock_fd, 
			fd_table[TBL_INDEX(table->vrrpid, table->local_flag)]->servaddr.sun_path);
		
		return -1;
	}

	dhcp_snp_del_unresolved_node(table);

	return 0;
}



/**********************************************************************************
 *	dhcpv6_snp_process_asd_interactive()
 *
 *	DESCRIPTION:
 *		when recv msg from asd, look for bind table, if this MAC table exist 
 *		and state is BOUND, send ip to asd. or add to unresolved table
 *
 *	INPUTS:
 *		table - unresolved_table
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		-1 - FAILED
 *		0 - SUCCESS
 ***********************************************************************************/
int dhcpv6_snp_process_asd_interactive(struct unresolved_ipv6_table *table)
{
	if ((!table)) {
		log_error("%s: parameter null.\n", __func__);		
		return -1;
	}
#if 0	
	/* find from bind table, then send to asd */
	if (1 == dhcpv6_snp_find_from_bind_table(table)) {
		dhcpv6_snp_notify_asd(table);
	}
	/* add to unresolve list */
	else {
		/* malloc node and add to list */
		dhcpv6_snp_add_unresolved_node(table);
	}
#endif
	dhcpv6_snp_add_unresolved_node(table);

	return 0;
}

/**********************************************************************************
 *	dhcp_snp_process_asd_interactive()
 *
 *	DESCRIPTION:
 *		when recv msg from asd, look for bind table, if this MAC table exist 
 *		and state is BOUND, send ip to asd. or add to unresolved table
 *
 *	INPUTS:
 *		table - unresolved_table
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		-1 - FAILED
 *		0 - SUCCESS
 ***********************************************************************************/
int dhcp_snp_process_asd_interactive(struct unresolved_table *table)
{
	if ((!table)) {
		log_error("%s: parameter null.\n", __func__);		
		return -1;
	}
#if 0	
	/* find from bind table, then send to asd */
	if (1 == dhcp_snp_find_from_bind_table(table)) {
		dhcp_snp_notify_asd(table);
	}
	/* add to unresolve list */
	else {
		/* malloc node and add to list */
		dhcp_snp_add_unresolved_node(table);
	}
#endif
	dhcp_snp_add_unresolved_node(table);

	return 0;
}


/**********************************************************************************
 *	fill_ipv6_unsoloved_table_from_tableMsg()
 *
 *	DESCRIPTION:
 *		fill unsoloved table from recv msg, NO CHECK, PLEASE CARFULL.
 *
 *	INPUTS:
 *		msg - recv msg
 *		table - unresolved table
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		
 ***********************************************************************************/
inline void fill_ipv6_unresolved_table_from_tableMsg(TableMsg *msg, struct unresolved_ipv6_table *table)
{
	table->bssindex = msg->u.STA.BSSIndex;
	table->vrrpid = msg->u.STA.vrrid;
	table->local_flag = msg->u.STA.local;
	memcpy(table->chaddr, msg->u.STA.STAMAC, NPD_DHCP_SNP_MAC_ADD_LEN);
}


/**********************************************************************************
 *	fill_unsoloved_table_from_tableMsg()
 *
 *	DESCRIPTION:
 *		fill unsoloved table from recv msg, NO CHECK, PLEASE CARFULL.
 *
 *	INPUTS:
 *		msg - recv msg
 *		table - unresolved table
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		
 ***********************************************************************************/
inline void fill_unresolved_table_from_tableMsg(TableMsg *msg, struct unresolved_table *table)
{
	table->bssindex = msg->u.STA.BSSIndex;
	table->vrrpid = msg->u.STA.vrrid;
	table->local_flag = msg->u.STA.local;
	memcpy(table->chaddr, msg->u.STA.STAMAC, NPD_DHCP_SNP_MAC_ADD_LEN);
}



/**********************************************************************************
 *	dhcpv6_snp_initiative_notify_asd()
 *
 *	DESCRIPTION:
 *		when recv DHCPv6 reply, look for unresolved table, if this MAC not send to asd, 
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
void dhcpv6_snp_initiative_notify_asd(unsigned char *haddr, unsigned char* ipaddr)
{
	struct unresolved_ipv6_table *table = NULL;

	if (table = dhcpv6_snp_find_from_unresolved_table(haddr)) {
		memcpy(table->ipv6_addr, ipaddr, 16);
		dhcpv6_snp_notify_asd(table);
	} 
	return ;
}

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
void dhcp_snp_initiative_notify_asd(unsigned char *haddr, unsigned int ipaddr)
{
	struct unresolved_table *table = NULL;

	if (table = dhcp_snp_find_from_unresolved_table(haddr)) {
		table->ipaddr = ipaddr;
		dhcp_snp_notify_asd(table);
	} 
	return ;
}

/**********************************************************************************
 *	__check_msg
 *
 *	DESCRIPTION:
 *		check msg from asd whether correct
 *
 *	INPUTS:
 *		TableMsg *msg - msg recv from asd
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		0 - Success				
 *		-1 - failed
 ***********************************************************************************/
inline int __check_msg(TableMsg *msg)
{
#if 1
	if ( msg->u.STA.vrrid > MAX_HANSI_PROFILE - 1) {
		return -1;
	}
#endif
	return 0;
}

/**********************************************************************************
 *	dhcp_snp_asd_interactive
 *
 *	DESCRIPTION:
 *		dhcp snoopint asd Billing interaction
 *
 *	INPUTS:
 *		void *arg - 
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *				
 ***********************************************************************************/
void * dhcp_snp_asd_interactive
(
	void *arg
)
{
	int i = 0;
	int fd = -1;
	int maxfd = 0;
	int count = 0;
	ssize_t recv_nbytes = 0;
	unsigned char status = 0;
	fd_set rd_set, rd;
	struct unresolved_table table;
	struct unresolved_ipv6_table table_ipv6;
	TableMsg msg;

	
	/* tell my thread pid*/
	dhcp_snp_tell_whoami("dhcpsnp to asd", 0);

	dhcp_snp_setaffinity(~0UL);
	
	log_info("dhcp snp init fd table...\n");	
	for (i = 0; i < sizeof(fd_table) / sizeof(fd_table[0]); i++) {
		fd_table[i] = NULL;
	}
	
	log_info("dhcp snp init unresolved table...\n");	
	for (i = 0; i < sizeof(g_dhcp_snp_unresolved_hash) / sizeof(g_dhcp_snp_unresolved_hash[0]); i++) {
		g_dhcp_snp_unresolved_hash[i] = NULL;
	}
	
	FD_ZERO (&rd_set);
	FD_ZERO (&rd);

again:
	log_debug("dhcp snp init unix socket...\n");	
	fd = init_sock_unix_server_asd();
	if (fd < 0) {
		sleep(1);
		goto again;
	}

	FD_SET (fd, &rd);
	if (fd > maxfd) {
		maxfd = fd;
	}

	while (1) {
		rd_set = rd;
		count = select(maxfd + 1, &rd_set, NULL, NULL, (struct timeval *)0);
		if (count < 0) {
			continue;
		}

		if (FD_ISSET (fd, &rd_set)) {

			memset(&msg, 0, sizeof(msg));
			
			recv_nbytes = recvfrom(fd, (char *)&msg, sizeof(msg), 0, NULL, NULL);
			if (recv_nbytes <= 0) {
				continue;
			}
			
			if (__check_msg(&msg)) {
				continue;
			}

			status = 0;
			dhcp_snp_get_global_status(&status);
			if (NPD_DHCP_SNP_DISABLE == status) {
				log_debug("dhcp snp is disabled\n");
				continue ;
			}
			log_info("recv msg form asd: bss index %d vrrid %d %s %s\n", 
				msg.u.STA.BSSIndex, msg.u.STA.vrrid,
				(msg.u.STA.local ? "local hansi" : "remote hansi"),
				mac2str(msg.u.STA.STAMAC));
			
			memset(&table, 0, sizeof(table));			
			fill_unresolved_table_from_tableMsg(&msg, &table);

			pthread_mutex_lock(&mutexDhcpsnptbl);
			pthread_mutex_lock(&mutexsnpunsolve);
			dhcp_snp_process_asd_interactive(&table);
			pthread_mutex_unlock(&mutexsnpunsolve);						
			pthread_mutex_unlock(&mutexDhcpsnptbl);
		
			memset(&table_ipv6, 0, sizeof(table_ipv6));			
			fill_ipv6_unresolved_table_from_tableMsg(&msg, &table_ipv6);

			pthread_mutex_lock(&mutexDhcpsnptbl);
			pthread_mutex_lock(&mutexsnpunsolve);
			dhcpv6_snp_process_asd_interactive(&table_ipv6);
			pthread_mutex_unlock(&mutexsnpunsolve);						
			pthread_mutex_unlock(&mutexDhcpsnptbl);
			
			
		}	
	}
}

/****************************************************************************
* dhcp_snp_tell_whoami
*
* DESCRIPTION:
*      This function is used by each thread to tell its name and
*	pid to DHCP_SNP_THREAD_PID_PATH "/var/run/dhcpsnp.pid"
*
* INPUTS:
*	  thread_name - thread name.
*	  last_teller - is this the last teller or not, pid file should be closed if true.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	  None.
*
* COMMENTS:
*       
*
****************************************************************************/
void dhcp_snp_tell_whoami
(
	unsigned char *thread_name,
	unsigned int last_teller
)
{
	static pthread_mutex_t mutexsnpwhoami = PTHREAD_MUTEX_INITIALIZER;
	static int fd = -1;
	pid_t self = 0;
	unsigned char buf[64] = {0};

	if ( NULL == thread_name ) {
		log_error("%s: pointer is empty!\n", __func__);
		return;
	}

	self = getpid();
		
	if (fd < 0) {
		fd = open(DHCP_SNP_THREAD_PID_PATH, O_CREAT |O_RDWR |O_TRUNC, 0644);
		if ( fd < 0 ) {
			log_error("open file %s error when %s tell its pid %d, %s\n",
				DHCP_SNP_THREAD_PID_PATH, thread_name, self, strerror(errno));
			return;
		}
	}
	
	log_info("thread %s tell its pid %d to %s\n",	
			thread_name, self, DHCP_SNP_THREAD_PID_PATH);

	snprintf(buf, sizeof(buf), "%d - %s\n", self, thread_name);

	pthread_mutex_lock(&mutexsnpwhoami);
	if ( strlen(buf) != write(fd, buf, strlen(buf)) ) {
		log_error("write file %s error when %s tell its pid %d, %s\n",
				DHCP_SNP_THREAD_PID_PATH, thread_name, self, strerror(errno));
		pthread_mutex_unlock(&mutexsnpwhoami);
		return;
	}
	pthread_mutex_unlock(&mutexsnpwhoami);
	
	if (last_teller) {
		close(fd);
		fd = -1;
		log_info("last teller %s %d closed pid file fd %d %s\n",  
				thread_name, self, fd, DHCP_SNP_THREAD_PID_PATH);
	}

	return;
}


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
int dhcp_snp_setaffinity(unsigned long mask)
{
	pid_t pid = getpid();

	log_info("set pid %d affinity %x...\n", pid, mask);

	if (sched_setaffinity(pid, sizeof(mask), &mask))
	{
		log_error("%s: set affinity failed: %s\n", __func__, strerror(errno));
		return -1;		
	}
	mask = 0UL;
	if (sched_getaffinity(pid, sizeof(mask), &mask))
	{
		log_error( "%s: get affinity failed: %s\n", __func__, strerror(errno));
		return -1;
	}
	
	log_info( "set pid %d affinity %x\n", pid, mask);
	
	return 0;
}

#ifdef __cplusplus
}
#endif

