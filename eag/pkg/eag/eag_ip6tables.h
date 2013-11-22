/*******************************************************************************
			Copyright(c), 2009, Autelan Technology Co.,Ltd.
						All Rights Reserved

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
*	RCSfile   :  eag_ip6tables.c
*
*	Author    :  houyongtao
*
*	Revision  :  1.00
*
*	Date      :  2011-11-2
********************************************************************************/
#ifndef _EAG_IPTABLES_H
#define _EAG_IPTABLES_H

/*********************************************************
*	head files														*
**********************************************************/
#include <arpa/inet.h>

/*********************************************************
*	macro define													*
**********************************************************/
#define EAG_IP6TABLES_SOURCE                    1
#define EAG_IP6TABLES_DESTINATION               2
#define EAG_IP6TABLES_SOURCE_AND_DESTINATION    3
#define EAG_IP6TABLES_ADD                       4
#define EAG_IP6TABLES_DELTE                     5

#define FLUSH_ALL_APPCONN_FLUX_TIME_INTERVAL_FOR_USER_MANAGE     5  /* flush all appcon flux time interval for user mamage,but the usemanage is in the fork...*/
#define FLUSH_ALL_APPCONN_FLUX_TIME_INTERVAL_FOR_RADIUS_IDLETIME 60 /* flush all appcon flux time interval for check radius idletime*/
#define FLUSH_ALL_APPCONN_FLUX_TIME_INTERVAL_FOR_RADIUS_PACK     60 /* flush all appcon flux time interval for radius pack */

#define FLAG_SET_OUT_FLUX_OF_APPCONN_BY_IP      1
#define FLAG_SET_IN_FLUX_OF_APPCONN_BY_IP       2

#define EAG_IP6TABLES_MAXNAMELEN                30
#define EAG_IP6TABLES_MAXNAMESIZE               32

#define EAG_IPV6_INTERFACE_IN                   1
#define EAG_IPV6_INTERFACE_OUT                  2

/*********************************************************
*	struct define													*
**********************************************************/
typedef struct _struct_ipv6_data_counter_
{
	char ipv6_str[40];
	unsigned long long data_counter;
	struct _struct_ipv6_data_counter_ * next;
}ipv6_data_counter;

typedef struct _struct_ipv6_counter_info_
{
	struct in6_addr user_ipv6;
	unsigned long long source_pkt;
	unsigned long long source_byte;               
	unsigned long long destination_pkt;
	unsigned long long destination_byte;
}ipv6_counter_info;
#if 0
enum {
	IPRANGE_SRC     = 1 << 0,	/* match source IP address */
	IPRANGE_DST     = 1 << 1,	/* match destination IP address */
	IPRANGE_SRC_INV = 1 << 4,	/* negate the condition */
	IPRANGE_DST_INV = 1 << 5,	/* -"- */
};

union nf_inet6_addr {
	uint32_t ip6[4];
	struct in6_addr	in6;
};

struct xt_iprange_mtinfo {
	union nf_inet6_addr src_min, src_max;
	union nf_inet6_addr dst_min, dst_max;
	uint8_t flags;
};
#endif
struct ipv6_white_black_iprange {
	char chain_name[EAG_IP6TABLES_MAXNAMESIZE];
	char nat_chain_name[EAG_IP6TABLES_MAXNAMESIZE];
	char iniface[MAX_IF_NAME_LEN];
	char portstring[64]; 
	char target_name[EAG_IP6TABLES_MAXNAMESIZE];
	char nat_target_name[EAG_IP6TABLES_MAXNAMESIZE];
	char comment_str[256];
	struct in6_addr ipv6begin;
	struct in6_addr ipv6end;
	int flag;
};

struct eag_ipv6_intf_entry_info {
	char *chain;
	char *intf;
	char *setname;
	char *setflag;
	char *target;
	int intf_flag;
};
/********************************************************************
*	function declare															 *
********************************************************************/

/*******************************************************************
 *	get_ip_counter_info
 * 
 *	DESCRIPTION:
 *		This function get counter info of one user by ip
 *
 *	INPUT:
 *		ip_addr - the ip address
 *	
 *	OUTPUT:
 *		the_info - the counter info
 *
 *	RETURN:
 *		EAG_RETURN_CODE_ERROR	- get the counter failed
 *		EAG_RETURN_CODE_OK 	- get the counter successfully
 *
 *********************************************************************/
int 
get_ipv6_counter_info (struct in6_addr *user_ipv6,ipv6_counter_info * the_info);

/*******************************************************************
 *	connect_up
 * 
 *	DESCRIPTION:
 *		Add the ip to the iptables
 *
 *	INPUT:
 *		user_id 		- user captive portal id
 *		user_interface	- interface name
 *		user_ip		- user ip
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		EAG_RETURN_CODE_ERROR		- error
 *		EAG_RETURN_CODE_OK 		- success
 *
 *********************************************************************/
int 
ipv6_connect_up(const int user_id, const int hansitype,
		const char *user_interface, struct in6_addr *user_ipv6);


/*******************************************************************
 *	connect_down
 * 
 *	DESCRIPTION:
 *		Delete the ip from the iptables
 *
 *	INPUT:
 *		user_id 		- user captive portal id
 *		user_interface	- interface name
 *		user_ip		- user ip
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		EAG_RETURN_CODE_ERROR		- error
 *		EAG_RETURN_CODE_OK 		- success
 *
 *********************************************************************/
int 
ipv6_connect_down(const int user_id, const int hansitype,
				const char *user_interface, struct in6_addr *user_ipv6);

int 
eag_ip6table_iprange(struct ipv6_white_black_iprange *input_info);

int
eag_ip6table_white_domain(struct ipv6_white_black_iprange *input_info);

int
eag_ip6table_black_domain(struct ipv6_white_black_iprange *input_info);

int
eag_ip6table_add_interface(int insid, char ins_type, char *intf);

int
eag_ip6table_del_interface(int insid, char ins_type, char *intf);


#if 0
int
eag_iptable_black_domain(char *chain_name, char *iniface, char *str, int flag);
#endif

int
ipv6_macpre_connect_up(int hansi_id, int hansi_type,
		 struct in6_addr *user_ipv6);

int
ipv6_macpre_connect_down(int hansi_id, int hansi_type,
		 struct in6_addr *user_ipv6);

#endif

