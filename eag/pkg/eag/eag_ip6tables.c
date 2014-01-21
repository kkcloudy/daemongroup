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
#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************
*	head files														*
**********************************************************/
#include <getopt.h>
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <iptables.h>
#include <linux/netfilter/xt_iprange.h>   /* wangchao add for new iptables */   
#include <arpa/inet.h>
#include <libiptc/libip6tc.h>
#include <pthread.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include <netdb.h>
#include <linux/netfilter/xt_iprange.h> 	/* -m iprange */
#include <linux/netfilter/xt_multiport.h>	/* -m multiport */
#include <linux/netfilter/xt_string.h>	/* -m string */
#include <linux/netfilter/xt_comment.h>	/* -m comment */
#include "eag_ipset.h"	/* -m set */

#include "eag_errcode.h"

#include "eag_log.h"
#include "eag_util.h"
#include "eag_time.h"
#include "eag_mem.h"
#include "nm_list.h"

#include "eag_blkmem.h"
#include "hashtable.h"
#include "session.h"
#include "appconn.h"
#include "eag_ipset6.h"
#include "eag_interface.h"
#include "eag_ip6tables.h"

#include "nmp_process.h"

#define MAX_CAPTIVE_ID      16
#define USE_THREAD_LOCK		0
#define EAG_IP6TABLE_SRC    1
#define EAG_IP6TABLE_DST    2
#define EAG_IP6TABLE_STRING 0x1
#define EAG_IP6TABLE_ALGO   0x2
#define EAG_IP6TABLE_FROM   0x4
#define EAG_IP6TABLE_TO     0x8
#define EAG_IPSET_SRC       0x01	/* Source match/add */
#define EAG_IPSET_DST       0x02	/* Destination match/add */

#define IPV6_CP_FILTER      "CP_FILTER"
#define IPV6_CP_DNAT        "CP_DNAT"


struct eag_ip6t_matches {
	struct list_head node;
	struct ip6t_entry_match *match;
};

struct eag_ip6t_entries {
	struct list_head node;
	struct ip6t_entry *entry;
};
#if 0
union nf_inet_addr {
	__u32		all[4];
	__be32		ip;
	__be32		ip6[4];
	struct in_addr	in;
	struct in6_addr	in6;
};
struct xt_iprange_mtinfo {
	union nf_inet_addr src_min, src_max;
	union nf_inet_addr dst_min, dst_max;
	u_int8_t flags;
};

struct sockaddr_in6 { 
    sa_family_t     sin6_family;   /* AF_INET6 */ 
    in_port_t       sin6_port;     /* port number */ 
    uint32_t        sin6_flowinfo; /* IPv6 flow information */ 
    struct in6_addr sin6_addr;     /* IPv6 address */ 
    uint32_t        sin6_scope_id; /* Scope ID (new in 2.4) */ 
};

struct in6_addr { 
    unsigned char   s6_addr[16];   /* IPv6 address */ 
};
#endif
/*********************************************************
*	global variable define									  *
**********************************************************/
#if USE_THREAD_LOCK
static pthread_mutex_t eag_ip6tables_glock;
#endif

nmp_mutex_t eag_ip6tables_lock = {-1, ""};

static int 
ipv6_get_index_of_entry(	const char * table_name,const char * chain_name,
							struct in6_addr *user_ipv6,const int type );
static int 
ipv6_add_and_del_entry(const char *table_name,const char *chain_name,
					struct in6_addr *source_ipv6,struct in6_addr *dest_ipv6,
					const char *target_name,const int type);
static int 
ipv6_check_is_chain(const char * table_name,const char * chain_name);


/*********************************************************
*	extern variable												*
**********************************************************/

/*********************************************************
*	functions define												*
**********************************************************/

/*******************************************************************
 *	get_ip_counter_info
 * 
 *	DESCRIPTION:
 *		This function get counter info of one user by ip
 *
 *	INPUT:
 *		user_ipv6 - the ip address
 *	
 *	OUTPUT:
 *		the_info - the counter info
 *
 *	RETURN:
 *		EAG_ERR_UNKNOWN	- get the counter failed
 *		EAG_RETURN_OK 	- get the counter successfully
 *
 *********************************************************************/
int 
get_ipv6_counter_info (struct in6_addr *user_ipv6,ipv6_counter_info * the_info)
{
#if 0
	const char* table_name = "filter";
	const char* chain_name = NULL;
	const struct ip6t_entry* p_entry = NULL;	
	const struct ip6t_counters* my_counter = NULL;//ipt_counters defined in ip_tables.h
	struct ip6tc_handle *handle = NULL;
	char ipv6_str[32] = "";
	int return_ret = EAG_ERR_UNKNOWN;
	int is_get_source = 0;//flag
	int is_get_destination = 0;//flag
	unsigned int counter_num = 1;
	
	ipv6tostr(user_ipv6, ipv6_str, sizeof(ipv6_str));
	#if 0
	/* check input */
	if (0 == user_ipv6>>24)
	{
		eag_log_err("ip range error! user_ipv6 == %s",ipv6_str);	
		goto return_error;
	}
	#endif
	if (NULL == the_info)
	{
		eag_log_err("input counter_info is NULL");
		goto return_error;
	}

	/* set the ip address */
	the_info->user_ipv6 = *user_ipv6;

	/* init */
	nmp_mutex_lock(&eag_ip6tables_lock);
	handle = ip6tc_init(table_name);
	nmp_mutex_unlock(&eag_ip6tables_lock);
	if (NULL == handle)
	{
		eag_log_err("function get_ip_counter_info  error,"\
					"can't init ip6tc handle,table name:%s",table_name);
		goto return_error;
	}
	
	for	(chain_name = ip6tc_first_chain(handle);
		chain_name && ( !is_get_source || !is_get_destination);
		chain_name = ip6tc_next_chain(handle))
	{		
		counter_num = 1;//rule start from 1		
		/* get rules */
		for (p_entry = ip6tc_first_rule(chain_name, handle); 
  			p_entry && (!is_get_source || !is_get_destination); 
  			p_entry = ip6tc_next_rule(p_entry, handle))
		{
			my_counter = ip6tc_read_counter(chain_name, counter_num, handle);
			if (*user_ipv6 == p_entry->ipv6.src && !is_get_source){
				the_info->source_byte = my_counter->bcnt;
				the_info->source_pkt = my_counter->pcnt;
				is_get_source = 1;
			}else if (*user_ipv6 == p_entry->ipv6.dst && !is_get_destination){
				the_info->destination_byte= my_counter->bcnt;
				the_info->destination_pkt= my_counter->pcnt;
				is_get_destination = 1;
			}
			counter_num++;
		}
	}

	if (is_get_source==0 && is_get_destination==0)
	{	
		eag_log_err("can't find counter in the iptable");
		goto return_error;
	}

//return_success:
	return_ret = EAG_RETURN_OK;
	goto return_line;
	
return_error:
	return_ret = EAG_ERR_UNKNOWN;
	goto return_line;
	
return_line:
	if (NULL != handle)
	{
		ip6tc_free(handle);
		handle = NULL;
	}
	return return_ret;
#endif
}//get_ip_counter_info

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
 *		EAG_ERR_UNKNOWN		- error
 *		EAG_RETURN_OK 		- success
 *
 *********************************************************************/
int 
ipv6_connect_up(const int user_id, const int hansitype,
		const char *user_interface, struct in6_addr *user_ipv6)
{
	int entry_num = 0;
//	int return_ret = EAG_ERR_UNKNOWN;
	char chain_name[256];
	char chain_name_in[256];
	char chain_name_nat[256];	
	char target_name[256];
	char target_name_nat[256];
	char ipv6_str[48] = "";
	char *cpid_prefix = "";
	struct in6_addr ipv6_any;
	
	cpid_prefix = (HANSI_LOCAL==hansitype)?"L":"R";
	ipv6tostr(user_ipv6, ipv6_str, sizeof(ipv6_str));

	if( (user_id<0 || user_id>MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| (NULL == user_interface || strlen(user_interface)==0)
		/*|| (0 == user_ip>>24)*/){
		eag_log_err("connect_up error. user_id=%d hansitype=%d  user_interface=%s user_ipv6=%s",
						user_id, hansitype, user_interface, ipv6_str );
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	memset(chain_name,0,sizeof(chain_name));
	memset(chain_name_in,0,sizeof(chain_name_in));
	memset(chain_name_nat,0,sizeof(chain_name_nat));
	memset(target_name,0,sizeof(target_name));
	memset(target_name_nat,0,sizeof(target_name_nat));
	
	snprintf(chain_name,sizeof(chain_name),"CP_%s%d_F_%s",
					cpid_prefix, user_id, user_interface);
	snprintf(chain_name_in,sizeof(chain_name_in),"CP_%s%d_F_%s_IN",
					cpid_prefix, user_id, user_interface);
	snprintf(chain_name_nat,sizeof(chain_name_nat),"CP_%s%d_N_%s",
					cpid_prefix, user_id, user_interface);
	snprintf(target_name,sizeof(target_name),"CP_%s%d_F_AUTH_DEFAULT",
					cpid_prefix, user_id);
	snprintf(target_name_nat,sizeof(target_name_nat),"CP_%s%d_N_AUTH_DEFAULT",
					cpid_prefix, user_id);

	/* search if the chain is exist */
	if (EAG_RETURN_OK!= ipv6_check_is_chain("filter",chain_name) 
		|| EAG_RETURN_OK != ipv6_check_is_chain("filter",chain_name_in) 
		|| EAG_RETURN_OK != ipv6_check_is_chain("nat",chain_name_nat))
	{
		eag_log_err("connect_up error,one or more chain is not exist,chain:%s,%s,%s",
						chain_name,chain_name_in,chain_name_nat);
		return EAG_ERR_UNKNOWN;
	}

	/* serch if the entry is exist */
	entry_num = ipv6_get_index_of_entry("filter",chain_name,user_ipv6,EAG_IP6TABLES_SOURCE);
	if ( entry_num < 0 ){
		eag_log_err("connect_up  error. input param might error!");
		return EAG_ERR_UNKNOWN;
	}else if( entry_num > 0 )
	{
		eag_log_err("connect_up error,entry is exist in the chain of table "\
					"\"filter\":user_ipv6==%s,chain_name==%s",ipv6_str,chain_name);
		return EAG_ERR_UNKNOWN;
	}
	
	inet_pton(AF_INET6, "::", &ipv6_any);
	/* add the entry */
	if	(EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name,
							user_ipv6, &ipv6_any, target_name, EAG_IP6TABLES_ADD)
		|| EAG_RETURN_OK !=  ipv6_add_and_del_entry("filter",chain_name_in,
							&ipv6_any, user_ipv6, target_name, EAG_IP6TABLES_ADD)
		|| EAG_RETURN_OK !=  ipv6_add_and_del_entry("nat",chain_name_nat,
							user_ipv6, &ipv6_any, target_name_nat, EAG_IP6TABLES_ADD))
	{
		eag_log_err("connect_up error, add entry error");
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

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
 *		EAG_ERR_UNKNOWN		- error
 *		EAG_RETURN_OK 		- success
 *
 *********************************************************************/
int 
ipv6_connect_down(const int user_id, const int hansitype,
				const char *user_interface, struct in6_addr *user_ipv6)
{
//	int entry_num = 0;
//	int return_ret = EAG_ERR_UNKNOWN;
	char chain_name[256];
	char chain_name_in[256];
	char chain_name_nat[256];	
	char target_name[256];
	char target_name_nat[256];
	char ipv6_str[48] = "";
	char *cpid_prefix = "";
	struct in6_addr ipv6_any;
	
	cpid_prefix = (HANSI_LOCAL==hansitype)?"L":"R";
	ipv6tostr(user_ipv6, ipv6_str, sizeof(ipv6_str));
	/* check input */
	if (NULL == user_interface)
	{
		eag_log_err("ipv6_connect_down error. no user_interface input");		
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	#if 0
	if (0 == user_ip>>24)
	{
		eag_log_err("ipv6_connect_down error. ip range error! user_ipv6 == %s",ipv6_str);	
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	#endif
	memset(chain_name,0,sizeof(chain_name));
	memset(chain_name_in,0,sizeof(chain_name_in));
	memset(chain_name_nat,0,sizeof(chain_name_nat));
	memset(target_name,0,sizeof(target_name));
	memset(target_name_nat,0,sizeof(target_name_nat));
	
	snprintf(chain_name,sizeof(chain_name),"CP_%s%d_F_%s",
								cpid_prefix,user_id,user_interface);
	snprintf(chain_name_in,sizeof(chain_name_in),"CP_%s%d_F_%s_IN",
								cpid_prefix,user_id,user_interface);
	snprintf(chain_name_nat,sizeof(chain_name_nat),"CP_%s%d_N_%s",
								cpid_prefix,user_id,user_interface);
	snprintf(target_name,sizeof(target_name),"CP_%s%d_F_AUTH_DEFAULT",
								cpid_prefix,user_id);
	snprintf(target_name_nat,sizeof(target_name_nat),"CP_%s%d_N_AUTH_DEFAULT",
								cpid_prefix,user_id);
	
	/* search if the chain is exist */
	if (EAG_RETURN_OK != ipv6_check_is_chain("filter",chain_name) 
		|| EAG_RETURN_OK != ipv6_check_is_chain("filter",chain_name_in) 
		|| EAG_RETURN_OK != ipv6_check_is_chain("nat",chain_name_nat))
	{
		eag_log_err("ipv6_connect_down error,one or more chain is not exist,chain:%s,%s,%s",chain_name,chain_name_in,chain_name_nat);
		return EAG_ERR_UNKNOWN;
	}
	
	inet_pton(AF_INET6, "::", &ipv6_any);
	/* del the entry */
	if	(EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name,
								user_ipv6, &ipv6_any, target_name, EAG_IP6TABLES_DELTE)
		|| EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name_in,
								&ipv6_any, user_ipv6, target_name, EAG_IP6TABLES_DELTE)
		|| EAG_RETURN_OK != ipv6_add_and_del_entry("nat", chain_name_nat,
								user_ipv6, &ipv6_any, target_name_nat, EAG_IP6TABLES_DELTE))
	{
		eag_log_err("ipv6_connect_down error,delete entry error");
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

/*******************************************************************
 *	check_is_chain
 * 
 *	DESCRIPTION:
 *		Check the chain is exist in the table
 *
 *	INPUT:
 *		table_name - table name
 *		chain_name - chain name
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		EAG_ERR_UNKNOWN	- error or not chain
 *		EAG_RETURN_OK 	- is chain
 *
 *********************************************************************/
static int 
ipv6_check_is_chain(const char * table_name,const char * chain_name)
{
	int ret = EAG_RETURN_OK;
	struct ip6tc_handle *handle = NULL;
	if (NULL == table_name || NULL == chain_name)
	{
		eag_log_err("function check_is_chain  error,input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	nmp_mutex_lock(&eag_ip6tables_lock);
	handle = ip6tc_init(table_name);
	nmp_mutex_unlock(&eag_ip6tables_lock);
	if (NULL == handle)
	{
		eag_log_err("function check_is_chain  error,can't init ip6tc handle,"\
					"table name:%s",table_name);
		ret = EAG_ERR_UNKNOWN;
	}
	else if (!ip6tc_is_chain(chain_name, handle))/*check chain exist*/
	{
		eag_log_err("chain is not exist in the table,chain name:%s,"\
					"table name:%s",chain_name,table_name);
		ret = EAG_ERR_UNKNOWN;
	}
	
	if (NULL != handle)
	{
		ip6tc_free(handle);
		handle = NULL;
	}
	return ret;
}

/*******************************************************************
 *	get_index_of_entry
 * 
 *	DESCRIPTION:
 *		Serch is the entry exist
 *
 *	INPUT:
 *		table_name 	- table name
 *		chain_name 	- chain name
 *		user_ipv6		- ip address
 *		type			- the input ip is source or destination
 *	
 *	OUTPUT:
 *		num_of_entry - the num of the entry
 *
 *	RETURN:
 *		EAG_ERR_UNKNOWN		- error
 *		EAG_RETURN_CODE_NOT_FOUND - not exist
 *		EAG_RETURN_OK 		- success ,entry is exist
 *
 *********************************************************************/
static int 
ipv6_get_index_of_entry( const char * table_name,const char * chain_name,
						struct in6_addr *user_ipv6,const int type )
{	
	const struct ip6t_entry *p_entry = NULL;
	const struct ip6t_counters *my_counter = NULL;
	struct ip6tc_handle *handle = NULL;
	unsigned int index = 1;
	char ipv6_str[48] = "";
	
	ipv6tostr( user_ipv6, ipv6_str, sizeof(ipv6_str) );
	
	/* check input */
	if (EAG_IP6TABLES_SOURCE != type && EAG_IP6TABLES_DESTINATION != type)
	{
		eag_log_err("input error,input:%d",type);
		return -1;
	}
	#if 0
	if (0 == user_ipv6>>24)
	{
		eag_log_err("ip range error! user_ipv6 == %s",ipv6_str);	
		return -1;
	}
	#endif
	if (NULL == table_name || NULL == chain_name)
	{
		eag_log_err("input counter_info is NULL");
		return -1;
	}
	
	/* iptc handle */
	nmp_mutex_lock(&eag_ip6tables_lock);
	handle = ip6tc_init(table_name);
	nmp_mutex_unlock(&eag_ip6tables_lock);
	if (NULL == handle)
	{
		eag_log_err("can't init ip6tc handle,table name:%s",table_name);
		return -1;
	}

	/* get rules */
	if (EAG_IP6TABLES_SOURCE == type)
	{
		for	(p_entry = ip6tc_first_rule((const char *)chain_name, handle);
			p_entry;
			p_entry = ip6tc_next_rule(p_entry, handle))
		{
			my_counter = ip6tc_read_counter(chain_name,index, handle);
			//eag_log_dbg("chain_name=%s,user_ipv6=%#X,p_entry->ip.src.s_addr=%#X",chain_name,user_ipv6,p_entry->ip.src.s_addr);
			if (0 == memcmp(user_ipv6,&(p_entry->ipv6.src), sizeof(struct in6_addr)))
			{				
				//return index;
				goto find;
			}
			index++;
		}
	}
	else if (EAG_IP6TABLES_DESTINATION == type)
	{
		for	(p_entry = ip6tc_first_rule(chain_name, handle);
			p_entry;
			p_entry = ip6tc_next_rule(p_entry, handle))
		{
			my_counter = ip6tc_read_counter(chain_name,index, handle);
			//eag_log_dbg("user_ipv6=%#X,p_entry->ip.dst.s_addr=%#X",user_ipv6,p_entry->ip.dst.s_addr);			
			if (0 == memcmp(user_ipv6,&(p_entry->ipv6.dst), sizeof(struct in6_addr)))
			{
				//return index;
				goto find;
			}
			index++;
		}
	}
	
	ip6tc_free(handle);
	handle = NULL;

	return 0;

find:
	ip6tc_free(handle);
	handle = NULL;

	return index;
}

/*******************************************************************
 *	add_and_del_entry
 * 
 *	DESCRIPTION:
 *		Add or delete the enty
 *
 *	INPUT:
 *		table_name 	- table name
 *		chain_name 	- chain name
 *		dest_ipv6		- destination ip address
 *		source_ipv6		- source ip address
 *		target_name	- target name
 *		type			- the input ip is source or destination
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		EAG_ERR_UNKNOWN		- error
 *		EAG_RETURN_OK 		- success
 *
 *********************************************************************/
static int 
ipv6_add_and_del_entry(const char *table_name,const char *chain_name,
					struct in6_addr *source_ipv6,struct in6_addr *dest_ipv6,
					const char *target_name,const int type)
{
	struct ip6t_entry *p_entry = NULL;
	struct ip6t_entry_target *p_target  = NULL;
//	struct ipt_entry_match *p_match = NULL;
//	struct ipt_tcp *ptcp = NULL;
	struct ip6tc_handle *handle = NULL;
	size_t entry_size = 0;
	size_t target_size = 0;
	size_t match_size = 0;
	size_t all_size = 0;
//	int i = 0;
	int return_ret = EAG_RETURN_OK;
	char dest_ipv6_str[48] = "";
	char source_ipv6_str[48] = "";
	struct in6_addr ipv6_any;
	inet_pton(AF_INET6, "::", &ipv6_any);
	struct in6_addr ipv6_only;
	inet_pton(AF_INET6, "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", &ipv6_only);

#if USE_THREAD_LOCK	
	eag_log_debug("ip6tables","add_and_del_entry lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
/*use iptables lock*/

	eag_log_debug("ip6tables","add_and_del_entry lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	ipv6tostr( dest_ipv6, dest_ipv6_str, sizeof(dest_ipv6_str)-1 );
	ipv6tostr( source_ipv6, source_ipv6_str, sizeof(source_ipv6_str)-1 );
	
	/* check input */
	if (EAG_IP6TABLES_ADD != type && EAG_IP6TABLES_DELTE != type)
	{
		eag_log_err("input error,input:%d",type);
		goto return_error;
	}
	#if 0
	if ((0 == dest_ipv6>>24) && (0 == source_ipv6>>24))
	{
		eag_log_err("ip range error! dest_ipv6 == %s,source_ipv6 == %s",dest_ipv6_str,source_ipv6_str);	
		goto return_error;
	}
	#endif
	if (NULL == table_name || NULL == chain_name || NULL == target_name)
	{
		eag_log_err("input counter_info is NULL");
		goto return_error;
	}
	
	#if 0//if need NAT,use this
	int is_nat;
	if(!strcmp(table_name,"nat"))
	{
		is_nat = 1;
	}else if(!strcmp(table_name,"filter"))
	{
		is_nat = 0;
	}else
	{
		return 0;
	}
	#endif
	
	handle = ip6tc_init(table_name);
	if ( NULL == handle)
	{
		eag_log_debug("ip6tables","can't init ip6tc handle,table name:%s",table_name);
		goto return_error;
	}
    //	entry_size = IPT_ALIGN(sizeof(struct ipt_entry)); wangchao change

	entry_size = sizeof(struct ip6t_entry);

	match_size = 0;
	#if 0//if match port ,use this
	match_size = IPT_ALIGN(sizeof(struct ipt_entry_match)) + IPT_ALIGN(sizeof(struct ipt_tcp));
	#endif
	#if 0//if NAT,use this
	target_size = IPT_ALIGN(sizeof(struct ipt_entry_target));
	if(is_nat)
	{
		target_size += IPT_ALIGN(sizeof(struct ip_nat_multi_range));//nat		
	}else
	{
		target_size += IPT_ALIGN(sizeof(int));
	}
	#endif
	//	target_size = IPT_ALIGN(sizeof(struct ipt_entry_target))+IPT_ALIGN(sizeof(int));wangchao change

	target_size = XT_ALIGN(sizeof(struct ip6t_entry_target))+XT_ALIGN(sizeof(int));
	all_size = target_size + match_size + entry_size;

	p_entry = eag_malloc(all_size);
	memset(p_entry, 0, all_size);

	/* Set tha Entry part of the entry */
	/* Set source and destination IP address */
	p_entry->ipv6.src = *source_ipv6;
	p_entry->ipv6.dst = *dest_ipv6;

    if (0 == ipv6_compare_null(source_ipv6))
    {
        p_entry->ipv6.smsk = ipv6_any;
    }
    else
    {
        p_entry->ipv6.smsk = ipv6_only;
    }
    if(0 == ipv6_compare_null(dest_ipv6))
    {
        p_entry->ipv6.dmsk = ipv6_any;
    }
    else
    {
        p_entry->ipv6.dmsk = ipv6_only;
    }

	/* Set the interface */
	#if 0
	if(strcmp(interface_name,"0"))
	{
		strcpy (p_entry->ip.iniface,interface_name);
		//for(i=strlen(interface_name);i>-1;i--)
		for(i=0;i<strlen(interface_name)+1;i++)
		{
			p_entry->ip.iniface_mask[i] = 0xff;
		}
	}
	#endif	
	/* Set the portol num(tcp 6,udp 17,icmp 1,IPv6 41,ALL 0) */
	#if 0
	if(!strcmp(portol_name,"tcp"))
	{
		p_entry->ip.proto = 6;
	}else if(!strcmp(portol_name,"udp"))
	{
		p_entry->ip.proto = 17;
	}else if(!strcmp(portol_name,"icmp"))
	{
		p_entry->ip.proto = 1;
	}else if(!strcmp(portol_name,"user_ipv6"))
	{
		p_entry->ip.proto = 41;
	}else
	{
		p_entry->ip.proto = 0;
	}
	#endif
	/* Set the proto (it's ALL here) */
	p_entry->ipv6.proto = 0;
	/* Set the size */
	p_entry->target_offset = entry_size + match_size;
	p_entry->next_offset = all_size;

	/* Set the ipt_entry_match part of the entry */
	#if 0//if match port,use it
	//Get address
	p_match = (struct ipt_entry_match*)p_entry->elems;
	p_match->u.user.match_size = match_size;
	//Set the portol name
	//strcpy(p_match->u.user.name,portol_name);
	//Set the Match Data of Match part----------------
	//Get address
	ptcp = (struct ipt_tcp*)p_match->data;
	//Set the port 	(All the port is match)
	ptcp->spts[0]=0;ptcp->spts[1]=0xffff;
	ptcp->dpts[0]=0;ptcp->dpts[1]=0xffff;
	#endif

	/* Set the ipt_entry_target part of the entry */
	/* Get address */
	p_target = (struct ip6t_entry_target*)(p_entry->elems+match_size);
	p_target->u.user.target_size = target_size;
	/* Set the target */
	strcpy(p_target->u.user.name,target_name);
	//strcpy(pt->u.user.name,"SNAT");
	#if 0//if NAT
	struct ip_nat_multi_range *p_nat;
	p_nat = (struct ip_nat_multi_range *) p_target->data;
	p_nat->rangesize = 1;// 该nat不做随机的选择 ,>1为多NAT	
	p_nat->range[0].flags = IP_NAT_RANGE_PROTO_SPECIFIED |
		IP_NAT_RANGE_MAP_IPS;	
	p_nat->range[0].min.tcp.port = p_nat->range[0].max.tcp.port = 0;
	p_nat->range[0].min_ip = p_nat->range[0].max_ip = inet_addr("4.4.4.4");
	#endif
	
	/* add or del */
	if (EAG_IP6TABLES_ADD == type)
	{
		//iptc_append_entry(chain_name,e,&h);//---append is insert in to the last
		if (!ip6tc_insert_entry(chain_name,p_entry,0,handle))
		{
			eag_log_err("add ip6tables error: %d,%s. table==%s,chain==%s,s_ip==%s,"\
						"d_ip==%s,target==%s,handle=%p",
						errno, ip6tc_strerror(errno), table_name, chain_name,
						source_ipv6_str, dest_ipv6_str, target_name, handle);
			goto return_error;
		}
	}
	else if (EAG_IP6TABLES_DELTE == type)
	{
		if (!ip6tc_delete_entry(chain_name,p_entry,NULL,handle))
		{
			eag_log_err("del ip6tables error: %d,%s table==%s,chain==%s,s_ip==%s,"\
						"d_ip==%s,target==%s,handle=%p",
						errno, ip6tc_strerror(errno), table_name, chain_name,
						source_ipv6_str, dest_ipv6_str, target_name, handle);
			goto return_error;
		}
	}
	
	if (!ip6tc_commit(handle))
	{
		eag_log_err("commit ip6tables error: %d,%s.\n table==%s,chain==%s,s_ip==%s,d_ip==%s,target==%s,handle=%p",
						errno, ip6tc_strerror(errno), table_name, chain_name,
						source_ipv6_str, dest_ipv6_str, target_name, handle);
		goto return_error;
	}
//return_success:
	return_ret = EAG_RETURN_OK;
	goto return_line;
	
return_error:
	return_ret = EAG_ERR_UNKNOWN;
	goto return_line;

return_line:
	
	if (NULL != p_entry)
	{
		eag_free(p_entry);
		p_entry = NULL;
	}		
	
	if (NULL != handle)
	{
		ip6tc_free(handle);
		handle = NULL;
	}
	
	//log_dbg("add_and_del_entry will unlock");
#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("ip6tables","ip6_add_and_del_entry unlock");
#endif

/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("ip6tables","ip6_add_and_del_entry unlock");
	
	return return_ret;
}

int
ipv6_macpre_connect_up(int hansi_id, int hansi_type,
    				struct in6_addr *user_ipv6)
{
	int entry_num = 0;
	char chain_name[256] = "";
	char chain_name_nat[256] = "";	
	char target_name[256] = "";
	char target_name_nat[256] = "";
	char ipv6_str[48] = "";
	char *cpid_prefix = "";
	struct in6_addr ipv6_any;

	cpid_prefix = (HANSI_LOCAL==hansi_type)?"L":"R";
	ipv6tostr(user_ipv6, ipv6_str, sizeof(ipv6_str));

	if ((hansi_id < 0 || hansi_id > MAX_CAPTIVE_ID)
		|| (hansi_type != 0 && hansi_type != 1)
		/*|| (0 == user_ip>>24)*/)
	{
		eag_log_err("ipv6_macpre_connect_up error, hansi_id=%d,hansi_type=%d,user_ipv6=%s",
			hansi_id, hansi_type, ipv6_str);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	/*MAC_PRE_AUTH_R3_F   MAC_PRE_AUTH_R3_N */
	snprintf(chain_name,sizeof(chain_name),"MAC_PRE_AUTH_%s%d_F",
				cpid_prefix, hansi_id);
	snprintf(chain_name_nat,sizeof(chain_name_nat),"MAC_PRE_AUTH_%s%d_N",
				cpid_prefix, hansi_id);
	snprintf(target_name,sizeof(target_name),"FW_FILTER");
	snprintf(target_name_nat,sizeof(target_name_nat),"FW_DNAT");

	/* search if the chain is exist */
	if (EAG_RETURN_OK!= ipv6_check_is_chain("filter", chain_name)
		|| EAG_RETURN_OK != ipv6_check_is_chain("nat", chain_name_nat))
	{
		eag_log_err("ipv6_macpre_connect_up error, one or more chain is not exist, chain:%s,%s",
			chain_name,chain_name_nat);
		return EAG_ERR_UNKNOWN;
	}

	/* serch if the entry is exist */
	entry_num = ipv6_get_index_of_entry("filter", chain_name, user_ipv6, EAG_IP6TABLES_SOURCE);
	if (entry_num < 0) {
		eag_log_err("ipv6_macpre_connect_up error, input param might error!");
		return EAG_ERR_UNKNOWN;
	} else if (entry_num > 0)
	{
		eag_log_err("ipv6_macpre_connect_up error, entry is exist in the chain of table "
					"\"filter\":user_ip=%s,chain_name=%s",ipv6_str, chain_name);
		return EAG_ERR_UNKNOWN;
	}
	
	inet_pton(AF_INET6, "::", &ipv6_any);
	/* add the entry */
	if	(EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name,
							user_ipv6, &ipv6_any, target_name, EAG_IP6TABLES_ADD)
		|| EAG_RETURN_OK !=  ipv6_add_and_del_entry("filter",chain_name,
							&ipv6_any, user_ipv6, target_name, EAG_IP6TABLES_ADD)
		|| EAG_RETURN_OK !=  ipv6_add_and_del_entry("nat",chain_name_nat,
							user_ipv6, &ipv6_any, target_name_nat, EAG_IP6TABLES_ADD))
	{
		eag_log_err("ipv6_macpre_connect_up error, add entry error");
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

int
ipv6_macpre_connect_down(int hansi_id, int hansi_type,
						struct in6_addr *user_ipv6)
{
	char chain_name[256] = "";
	char chain_name_nat[256] = "";
	char target_name[256] = "";
	char target_name_nat[256] = "";
	char ipv6_str[48] = "";
	char *cpid_prefix = "";
	struct in6_addr ipv6_any;
	
	cpid_prefix = (HANSI_LOCAL==hansi_type)?"L":"R";
	ipv6tostr(user_ipv6, ipv6_str, sizeof(ipv6_str));
	
	if ( (hansi_id < 0 || hansi_id > MAX_CAPTIVE_ID)
		|| (hansi_type != 0 && hansi_type != 1)
		/*|| (0 == user_ip>>24)*/ )
	{
		eag_log_err("ipv6_macpre_connect_down error, hansi_id=%d, hansi_type=%d,user_ipv6=%s",
			hansi_id, hansi_type, ipv6_str);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	/*MAC_PRE_AUTH_R3_F   MAC_PRE_AUTH_R3_N */
	snprintf(chain_name,sizeof(chain_name),"MAC_PRE_AUTH_%s%d_F",
				cpid_prefix, hansi_id);
	snprintf(chain_name_nat,sizeof(chain_name_nat),"MAC_PRE_AUTH_%s%d_N",
				cpid_prefix, hansi_id);
	snprintf(target_name,sizeof(target_name),"FW_FILTER");
	snprintf(target_name_nat,sizeof(target_name_nat),"FW_DNAT");

	/* search if the chain is exist */
	if (EAG_RETURN_OK != ipv6_check_is_chain("filter", chain_name)
		|| EAG_RETURN_OK != ipv6_check_is_chain("nat", chain_name_nat))
	{
		eag_log_err("ipv6_macpre_connect_down error, one or more chain is not exist, chain:%s,%s",
			chain_name, chain_name_nat);
		return EAG_ERR_UNKNOWN;
	}

	inet_pton(AF_INET6, "::", &ipv6_any);
	/* del the entry */
	if	(EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name,
								user_ipv6, &ipv6_any, target_name, EAG_IP6TABLES_DELTE)
		|| EAG_RETURN_OK != ipv6_add_and_del_entry("filter", chain_name,
								&ipv6_any, user_ipv6, target_name, EAG_IP6TABLES_DELTE)
		|| EAG_RETURN_OK != ipv6_add_and_del_entry("nat", chain_name_nat,
								user_ipv6, &ipv6_any, target_name_nat, EAG_IP6TABLES_DELTE))
	{
		eag_log_err("ipv6_macpre_connect_down error, delete entry error");
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

static inline void
ipv6_set_revision(char *name, u_int8_t revision)
{
	/* Old kernel sources don't have ".revision" field,
	   but we stole a byte from name. */
	name[IP6T_FUNCTION_MAXNAMELEN - 2] = '\0';
	name[IP6T_FUNCTION_MAXNAMELEN - 1] = revision;
}

static int
ipv6_string_to_number_ll(const char *s, unsigned long long min, unsigned long long max,
		 unsigned long long *ret)
{
	unsigned long long number;
	char *end;

	/* Handle hex, octal, etc. */
	errno = 0;
	number = strtoull(s, &end, 0);
	if (*end == '\0' && end != s) {
		/* we parsed a number, let's see if we want this */
		if (errno != ERANGE && min <= number && (!max || number <= max)) {
			*ret = number;
			return 0;
		}
	}
	return -1;
}

static int
ipv6_string_to_number_l(const char *s, unsigned long min, unsigned long max,
		 unsigned long *ret)
{
	int result;
	unsigned long long number;

	result = ipv6_string_to_number_ll(s, min, max, &number);
	*ret = (unsigned long)number;

	return result;
}

static int 
ipv6_string_to_number(const char *s, unsigned int min, unsigned int max,
		unsigned int *ret)
{
	int result;
	unsigned long number;

	result = ipv6_string_to_number_l(s, min, max, &number);
	*ret = (unsigned int)number;

	return result;
}

static int
ipv6_service_to_port(const char *name, const char *proto)
{
	struct servent *service;

	if ((service = getservbyname(name, proto)) != NULL)
		return ntohs((unsigned short) service->s_port);

	return -1;
}

#if 0
static inline void
ipv6_parse_iprange(struct in6_addr ipv6begin, struct in6_addr ipv6end,
				struct ipt_iprange *range)
{
	range->min_ip = *ipv6begin;
	range->max_ip = *ipv6end;
}
#endif
static u_int16_t
ipv6_parse_port(const char *port, const char *proto)
{
	unsigned int portnum;

	if ((ipv6_string_to_number(port, 0, 65535, &portnum)) != -1 ||
	    (portnum = ipv6_service_to_port(port, proto)) != -1)
		return (u_int16_t)portnum;

	eag_log_err("invalid port/service `%s' specified", port);
	return 0;
}

static void
ipv6_parse_multi_ports(const char *portstring, 
		     struct xt_multiport_v1 *multiinfo,
		     const char *proto)
{
	char *buffer, *cp, *next, *range;
	unsigned int i;

	buffer = strdup(portstring);
	if (!buffer) {
		eag_log_err("strdup failed");
		return;
	}

	for (i=0; i<XT_MULTI_PORTS; i++)
		multiinfo->pflags[i] = 0;
 
	for (cp=buffer, i=0; cp && i<XT_MULTI_PORTS; cp=next, i++) {
		next=strchr(cp, ',');
 		if (next) *next++ = '\0';
		range = strchr(cp, ':');
		if (range) {
			if (i == XT_MULTI_PORTS-1) {
				eag_log_err("too many ports specified");
				return;
			}
			*range++ = '\0';
		}
		multiinfo->ports[i] = ipv6_parse_port(cp, proto);
		if (range) {
			multiinfo->pflags[i] = 1;
			multiinfo->ports[++i] = ipv6_parse_port(range, proto);
			if (multiinfo->ports[i-1] >= multiinfo->ports[i]) {
				eag_log_err("invalid portrange specified");
				return;
			}
		}
 	}
	multiinfo->count = i;
 	if (cp) {
		eag_log_err("too many ports specified");
 	}
 	free(buffer);
}

static void
ipv6_parse_algo(const char *s, struct xt_string_info *info)
{
	if (strlen(s) <= XT_STRING_MAX_ALGO_NAME_SIZE) {
		strncpy(info->algo, s, XT_STRING_MAX_ALGO_NAME_SIZE);
		return;
	}
	eag_log_err("ALGO too long `%s'", s);
}

static void
ipv6_parse_comment(const char *s, struct xt_comment_info *info)
{	
	int slen = strlen(s);

	if (slen >= XT_MAX_COMMENT_LEN) {
		eag_log_err("COMMENT must be shorter than %i characters", XT_MAX_COMMENT_LEN);
		return;
	}
	strcpy((char *)info->comment, s);
}

static void
ipv6_parse_iniface(const char *str, struct ip6t_entry *fw)
{
	int i = 0;
	strcpy (fw->ipv6.iniface, str);
	for(i = 0; i < strlen(str) + 1; i++) {
		fw->ipv6.iniface_mask[i] = 0xff;
	}
}

static void
ipv6_parse_outiface(const char *str, struct ip6t_entry *fw)
{
	int i = 0;
	strcpy (fw->ipv6.outiface, str);
	for(i = 0; i < strlen(str) + 1; i++) {
		fw->ipv6.outiface_mask[i] = 0xff;
	}
}
/* Set the portol num(tcp 6, udp 17, icmp 1, IPv6 41, ALL 0) */
static u_int16_t
ipv6_parse_protocol(const char *str)
{
	if (!strcmp(str, "tcp")) {
		return 6;
	}
	if (!strcmp(str, "udp")) {
		return 17;
	}
	if (!strcmp(str, "icmp")) {
		return 1;
	}
	if (!strcmp(str, "IPv6")) {
		return 41;
	}
	return 0;
}

static void
ipv6_parse_string(const char *str, struct xt_string_info *info)
{	
	if (strlen(str) <= XT_STRING_MAX_PATTERN_SIZE) {
		strncpy(info->pattern, str, XT_STRING_MAX_PATTERN_SIZE);
		info->patlen = strlen(str);
		return;
	}
}

static void
ipv6_parse_bindings(const char *str, struct ipt_set_info *info)
{
	int i = 0;
	char *ptr = NULL;
	char *tmp = NULL;
	char *saved = NULL;

	saved = eag_calloc(1, strlen(str) + 1);
	strncpy(saved, str, strlen(str));
	tmp = saved;

	info->dim = 0;
    while (info->dim < IP_SET_MAX_BINDINGS && tmp != NULL) {
        info->dim++;
        ptr = strsep(&tmp, ",");
        if (strncmp(ptr, "src", 3) == 0)
            info->flags |= (1 << info->dim);
        else if (strncmp(ptr, "dst", 3) != 0)
			eag_log_err("You must spefify (the comma separated list of) 'src' or 'dst'.");
    }

	if (tmp) {
		eag_log_err("Can't follow bindings deeper than %i.", 
					   IP_SET_MAX_BINDINGS);
	}
	eag_free(saved);
}


static void
ipv6_string_init(struct ip6t_entry_match *m, unsigned int *nfcache)
{
	struct xt_string_info *i = (struct xt_string_info *) m->data;

	if (i->to_offset == 0)
		i->to_offset = (u_int16_t) ~0UL;
}

static unsigned char *
ip6table_mask(size_t all_size)
{

	unsigned char *mask;
	mask = eag_calloc(1, all_size);
	if (NULL == mask) {
		eag_log_err("calloc error:mask = NULL");
		return NULL;
	}
	memset(mask, 0xFF, all_size);
	return mask;
}

static unsigned char *
ipv6_string_mask(size_t all_size)
{
	unsigned char *mask, *mptr;
	mask = eag_calloc(1, all_size);
	if (NULL == mask) {
		eag_log_err("calloc error:mask = NULL");
		return NULL;
	}
	memset(mask, 0xFF, sizeof(struct ip6t_entry));
	mptr = mask + sizeof(struct ip6t_entry);
	memset(mptr, 0xFF,
		       XT_ALIGN(sizeof(struct ip6t_entry_match))
		       + offsetof(struct xt_string_info, config));
	mptr += XT_ALIGN(sizeof(struct ip6t_entry_match))
 	 	+ XT_ALIGN(sizeof(struct xt_string_info));
	memset(mptr, 0xFF,
	       XT_ALIGN(sizeof(struct ip6t_entry_target))
	       + XT_ALIGN(sizeof(int)));
	return mask;
}

struct ip6t_entry *
eag_ip6table_entry_new(const struct ip6t_entry *fw,
		struct list_head *ip6t_match_node,
		struct ip6t_entry_target *target)
{
	struct ip6t_entry *p_entry;
	struct eag_ip6t_matches *matchp;
	size_t size = 0;
	int i = 0;
	struct in6_addr ipv6_any;
	inet_pton(AF_INET6, "::", &ipv6_any);
	//size = IPT_ALIGN(sizeof(struct ipt_entry)); wangchao change
	size = sizeof(struct ip6t_entry); 
	list_for_each_entry(matchp, ip6t_match_node, node) {
		size += matchp->match->u.match_size;
	}

	p_entry = eag_malloc(size + target->u.target_size);
	if (NULL == p_entry) {
		eag_log_err("malloc error:p_entry = NULL");
		return p_entry;
	}

	*p_entry = *fw;	
	p_entry->target_offset = size;
	p_entry->next_offset = size + target->u.target_size;
	p_entry->ipv6.src = ipv6_any;	
	p_entry->ipv6.dst = ipv6_any;	
	p_entry->ipv6.smsk = ipv6_any;
	p_entry->ipv6.dmsk = ipv6_any;
	
	size = 0;
	if (0 == list_empty_careful(ip6t_match_node)) {
		list_for_each_entry(matchp, ip6t_match_node, node) {
			memcpy(p_entry->elems + size, matchp->match,
				matchp->match->u.match_size);
			size += matchp->match->u.match_size;
			i++;
		}
	}
	eag_log_debug("eag_ip6tables", "match count:%d", i);		
	memcpy(p_entry->elems + size, target, target->u.target_size);

	return p_entry;
	
}

/* 使用前分配内存
 * size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 * 	+ IPT_ALIGN(sizeof(struct xt_iprange_mtinfo))
 */
static int
eag_ipv6_match_iprange(int flag, struct in6_addr *ipv6begin, struct in6_addr *ipv6end, 
		struct ip6t_entry_match *match, size_t size)
{
	match = (struct ip6t_entry_match *)match;
	match->u.match_size = size;
	strcpy(match->u.user.name, "iprange");
	ipv6_set_revision(match->u.user.name, 1);

	struct xt_iprange_mtinfo *info = (struct xt_iprange_mtinfo *)match->data;

	switch (flag) {
	case EAG_IP6TABLE_SRC:
		info->flags |= IPRANGE_SRC;
		info->src_min.in6 = *ipv6begin;
		info->src_max.in6 = *ipv6end;
		//eag_log_debug("eag_ip6tables", "src_iprange:%u-%u, flag:%u",
			//info->src_min.in6, info->src_max.in6, info->flags);
		break;
	case EAG_IP6TABLE_DST:
		info->flags |= IPRANGE_DST;
		info->dst_min.in6 = *ipv6begin;
		info->dst_max.in6 = *ipv6end;
		//eag_log_debug("eag_ip6tables", "dst_iprange:%u-%u, flag:%u",
			//info->dst_min.ip, info->dst_max.ip, info->flags);
		break;
	default	:
		return -1;
	}
	return 0;
}

/* 使用前分配内存
 * size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 * 	+ IPT_ALIGN(sizeof(struct ipt_multiport_v1))
 */
static int
eag_ipv6_match_multiport(int flag, const char *portstring, const char *proto,
		struct ip6t_entry_match *match, size_t size)
{
	match = (struct ip6t_entry_match *)match;
	match->u.match_size = size;
	strcpy(match->u.user.name, "multiport");
	set_revision(match->u.user.name, 1);
	struct xt_multiport_v1 *multiinfo = (struct xt_multiport_v1 *)match->data;

	switch (flag) {
	case EAG_IP6TABLE_SRC:
		ipv6_parse_multi_ports(portstring, multiinfo, proto);
		multiinfo->flags = XT_MULTIPORT_SOURCE;
		eag_log_debug("eag_ip6tables", "count:%u, flag:%u",
			multiinfo->count, multiinfo->flags);
		break;
	case EAG_IP6TABLE_DST:
		ipv6_parse_multi_ports(portstring, multiinfo, proto);
		multiinfo->flags = XT_MULTIPORT_DESTINATION;
		eag_log_debug("eag_ip6tables", "count:%u, flag:%u", 
			multiinfo->count, multiinfo->flags);
		break;
	default:
		return 0;
	}
	
	return 0;
}

/* 使用前分配内存
 * size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 * 	+ IPT_ALIGN(sizeof(struct xt_string_info))
 */
static int
eag_ipv6_match_string(int flag, const char *str, const char *algo_str, u_int16_t to_offset,
		struct ip6t_entry_match *match, size_t size)
{
	match = (struct ip6t_entry_match *)match;
	match->u.match_size = size;
	strcpy(match->u.user.name, "string");
	struct xt_string_info *stringinfo = (struct xt_string_info *)match->data;

	if (flag & EAG_IP6TABLE_STRING) {
		ip6_parse_string(str, stringinfo);
		stringinfo->patlen=strlen((char *)&stringinfo->pattern);
	}
	if (flag & EAG_IP6TABLE_TO) {
		stringinfo->to_offset = to_offset;
	}
	if (flag & EAG_IP6TABLE_ALGO) {
		ip6_parse_algo(algo_str, stringinfo);
	}
	
	return 0;	
}

/* 使用前分配内存
 * size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 * 	+ IPT_ALIGN(sizeof(struct ipt_comment_info))
 */
static int
eag_ipv6_match_comment(const char *comment_str,
		struct ip6t_entry_match *match, size_t size)
{
	match = (struct ip6t_entry_match *)match;
	match->u.match_size = size;
	strcpy(match->u.user.name, "comment");
	struct xt_comment_info *commentinfo = (struct xt_comment_info *)match->data;

	eag_log_debug("eag_ip6tables", "comment size:%u, str:%s",
		XT_ALIGN(sizeof(struct xt_comment_info)), comment_str);

	ipv6_parse_comment(comment_str, commentinfo);

	return 0;
	
}

/* 使用前分配内存
 * size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 * 	+ IPT_ALIGN(sizeof(struct ipt_set_info_match))
 */
static int
eag_ipv6_match_set(const char *setname, const char *setflag,
		struct ip6t_entry_match *match, size_t size)
{
	match = (struct ip6t_entry_match *)match;
	match->u.match_size = size;
	match->u.user.revision = 3; /* add by houyongtao */
	strcpy(match->u.user.name, "set");
	struct ipt_set_info_match *myinfo = (struct ipt_set_info_match *)match->data;
	struct ipt_set_info *info = &myinfo->match_set;

	info->index = eag_get_set_byname(setname);
	ipv6_parse_bindings(setflag, info);

	return 0;
	
}

int
eag_ip6table_commit(char *table_name, char *chain_name,
//		struct eag_ipt_entries *entries,
		struct list_head *ip6t_entry_node,
		int flag)
{
#if USE_THREAD_LOCK	
	eag_log_debug("eag_iptables","eag_iptable_glock lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
	/*use iptables lock*/
	eag_log_debug("eag_iptables","iptable_lock lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	struct eag_ip6t_entries *p_entries = NULL;
	struct ip6tc_handle *handle = NULL;
	unsigned char *mask = NULL;
	int ret = 0;
	size_t size = 0;
		
	handle = ip6tc_init(table_name);
	if ( NULL == handle)
	{
		eag_log_warning("can't init iptc handle,table name:%s", table_name);
		goto return_error;
	}
	if (flag == EAG_IP6TABLES_ADD) {
		list_for_each_entry(p_entries, ip6t_entry_node, node)
		{
			if (!ip6tc_insert_entry(chain_name, 
				p_entries->entry, 0, handle)) 
			{
				eag_log_err("iptc_insert_entry failed chain_name=%s", chain_name);
				goto return_error;
			}
			eag_log_debug("eag_iptables", "ipt_insert_entry success");
		}
	} else if (flag == EAG_IP6TABLES_DELTE) {
		list_for_each_entry(p_entries, ip6t_entry_node, node)
		{
			size = p_entries->entry->next_offset;
			mask = ip6table_mask(size);
			if (!ip6tc_delete_entry(chain_name, 
				p_entries->entry, mask, handle)) 
			{
				eag_log_err("ip6tc_delete_entry failed chain_name=%s", chain_name);
				goto return_error;
			}
			eag_log_debug("eag_ip6tables", "ip6t_delete_entry success");
			free(mask);
			mask = NULL;
		}
		
	}
	eag_log_debug("eag_ip6tables", "chain:%s, table:%s", chain_name, table_name);

	if (!ip6tc_commit(handle)) {
		eag_log_warning("commit ip6tables error");
		goto return_error;
	}
	eag_log_debug("eag_ip6tables", "ip6tc_commit success");

	ret = 0;
	goto return_line;
return_error:
	ret = 1;
	goto return_line;
return_line:
	if (NULL != handle) {
		ip6tc_free(handle);
		handle = NULL;
	}
	if (NULL != mask) {
		eag_free(mask);
		mask = NULL;
	}

#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("eag_ip6tables","eag_ip6table_glock unlock");
#endif
	/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("eag_ip6tables","ip6table_lock unlock");

	return ret;
}

struct ip6t_entry *
eag_ipv6_iprange_entry(int range_flag, char *proto,
		struct ipv6_white_black_iprange *input_info, char *target_name)
{
#if 0
	struct ip6t_entry fw, *p_entry = NULL;
	//struct eag_ipt_matches *matches = NULL;
	struct list_head ip6t_match_node;
	struct eag_ip6t_matches *matchp, *tmp;
	struct ip6t_entry_target *target = NULL;
	size_t size;

	memset(&fw, 0, sizeof(fw));
	INIT_LIST_HEAD(&ip6t_match_node);
		
/* match */
	fw.user_ipv6.proto = ipv6_parse_protocol(proto);
	if (input_info->iniface[0] != '\0') {
		ipv6_parse_iniface(input_info->iniface, &fw);
		eag_log_debug("eag_ip6tables", "match:iniface");
	}
	if (strcmp(input_info->portstring, "all")) {
		matchp = eag_calloc(1, sizeof(struct eag_ip6t_matches));
		if (NULL == matchp) {
			eag_log_err("calloc error:matchp = NULL");
			goto return_error;
		}
		/*size = IPT_ALIGN(sizeof(struct ipt_entry_match))
			+ IPT_ALIGN(sizeof(struct ipt_multiport_v1)); wangchao change*/
		size = XT_ALIGN(sizeof(struct ip6t_entry_match))
			+ XT_ALIGN(sizeof(struct xt_multiport_v1));			
		matchp->match = eag_calloc(1, size);
		if (NULL == matchp->match) {
			eag_log_err("calloc error:matchp->match = NULL");
			eag_free(matchp);
			goto return_error;
		}
		eag_ipv6_match_multiport(range_flag, input_info->portstring,
			proto, matchp->match, size);
		eag_log_debug("eag_ip6tables", "match:multiport, size:%u", size);

		list_add(&(matchp->node), &ip6t_match_node);
	}
	if (input_info->comment_str[0] != '\0') {
		matchp = eag_calloc(1, sizeof(struct eag_ip6t_matches));
		if (NULL == matchp) {
			eag_log_err("calloc error:matchp = NULL");
			goto return_error;
		}
		/*size = IPT_ALIGN(sizeof(struct ipt_entry_match))
 			+ IPT_ALIGN(sizeof(struct ipt_comment_info));wangchao change*/

        size = XT_ALIGN(sizeof(struct ip6t_entry_match))
 			+ XT_ALIGN(sizeof(struct xt_comment_info));		
		matchp->match = eag_calloc(1, size);
		if (NULL == matchp->match) {
			eag_log_err("calloc error:matchp->match = NULL");
			eag_free(matchp);
			goto return_error;
		}
		eag_ipv6_match_comment(input_info->comment_str, matchp->match, size);
		eag_log_debug("eag_ip6tables", "match:comment, size:%u", size);

		list_add(&(matchp->node), &ip6t_match_node);
	}
	if (input_info->ipv6end >= input_info->ipv6begin) {
		matchp = eag_calloc(1, sizeof(struct eag_ip6t_matches));
		if (NULL == matchp) {
			eag_log_err("calloc error:matchp = NULL");
			goto return_error;
		}
		/*size = IPT_ALIGN(sizeof(struct ipt_entry_match))
			+ IPT_ALIGN(sizeof(struct xt_iprange_mtinfo));wangchao change*/
		size = XT_ALIGN(sizeof(struct ip6t_entry_match))
			+ XT_ALIGN(sizeof(struct xt_iprange_mtinfo));			
		matchp->match = eag_calloc(1, size);
		if (NULL == matchp->match) {
			eag_log_err("calloc error:matchp->match = NULL");
			eag_free(matchp);
			goto return_error;
		}
		eag_ipv6_match_iprange(range_flag, input_info->*ipv6begin, input_info->*ipv6end,
			matchp->match, size);
		eag_log_debug("eag_iptables", "match:iprange, size:%u", size);

		list_add(&(matchp->node), &ip6t_match_node);
	} else if (input_info->ipv6begin != 0) {
		eag_log_err("input error:*ipv6begin = %lu and *ipv6end = %lu", 
			input_info->ipv6begin, input_info->ipv6end);
		goto return_error;
	}

/* target */
	//size= IPT_ALIGN(sizeof(struct ipt_entry_target))+IPT_ALIGN(sizeof(int)); wangchao change
    size= XT_ALIGN(sizeof(struct ip6t_entry_target))+XT_ALIGN(sizeof(int));	
	target = eag_calloc(1, size);
	if (NULL == target) {
		eag_log_err("calloc error:target = NULL");
		goto return_error;
	}

	target->u.target_size = size;
	strcpy(target->u.user.name, target_name);
	eag_log_debug("eag_ip6tables", "target_name:%s, target_size:%u", 
		target->u.user.name, target->u.target_size);
	

/* entry */
	p_entry = eag_ip6table_entry_new(&fw, &ip6t_match_node, target);
	goto return_line;
	
return_error:
	p_entry = NULL;
	goto return_line;

return_line:
	list_for_each_entry_safe(matchp, tmp, &ip6t_match_node, node)
	{
		if (NULL != matchp->match) {
			eag_free(matchp->match);
			matchp->match = NULL;	
		}
		eag_free(matchp);
		matchp = NULL;
	}

	if (NULL != target) {
		eag_free(target);
		target = NULL;
	}
	return p_entry;
#endif
}

struct ip6t_entry *
eag_ipv6_set_intf_entry(struct eag_ipv6_intf_entry_info *info)
{
	struct ip6t_entry fw, *p_entry = NULL;
	struct list_head ip6t_match_node = {0};
	struct eag_ip6t_matches *matchp = NULL;
	struct eag_ip6t_matches *tmp = NULL;
	struct ip6t_entry_target *target = NULL;
	size_t size;

	memset(&fw, 0, sizeof(fw));
	INIT_LIST_HEAD(&ip6t_match_node);
		
/* match */
	fw.ipv6.proto = 0;
	if (info->intf_flag == EAG_IPV6_INTERFACE_IN && info->intf != NULL) {
		ipv6_parse_iniface(info->intf, &fw);
		eag_log_debug("eag_ip6tables", "match:iniface");
	}
	if (info->intf_flag == EAG_IPV6_INTERFACE_OUT && info->intf != NULL) {
		ipv6_parse_outiface(info->intf, &fw);
		eag_log_debug("eag_iptables", "match:outiface");
	}
	
	if (info->setname != NULL && info->setflag != NULL && strcmp(info->setname, "")) {
		matchp = eag_calloc(1, sizeof(struct eag_ip6t_matches));
		if (NULL == matchp) {
			eag_log_err("eag_add_del_intf_entry calloc set error");
			goto return_error;
		}
		/*size = IPT_ALIGN(sizeof(struct ipt_entry_match))
			+ IPT_ALIGN(sizeof(struct ipt_set_info_match));wangchao change*/
		size = XT_ALIGN(sizeof(struct ip6t_entry_match))
			+ XT_ALIGN(sizeof(struct ipt_set_info_match));			
		matchp->match = eag_calloc(1, size);
		if (NULL == matchp->match) {
			eag_log_err("eag_add_del_intf_entry calloc set error");
			eag_free(matchp);
			goto return_error;
		}
		eag_log_debug("eag_ip6tables", "match:set, name:%s, flag:%s, size:%u", 
					info->setname, info->setflag, size);
		eag_ipv6_match_set(info->setname, info->setflag, matchp->match, size);
		list_add(&(matchp->node), &ip6t_match_node);
	}

/* target */
	//size= IPT_ALIGN(sizeof(struct ipt_entry_target))+IPT_ALIGN(sizeof(int));wangchao change
	size= XT_ALIGN(sizeof(struct ip6t_entry_target))+XT_ALIGN(sizeof(int));
	target = eag_calloc(1, size);
	if (NULL == target) {
		eag_log_err("calloc error:target = NULL");
		goto return_error;
	}

	target->u.target_size = size;
	strcpy(target->u.user.name, info->target);
	eag_log_debug("eag_ip6tables", "target_name:%s, target_size:%u", 
		target->u.user.name, target->u.target_size);

/* entry */
	p_entry = eag_ip6table_entry_new(&fw, &ip6t_match_node, target);
	goto return_line;
	
return_error:
	p_entry = NULL;
return_line:
	list_for_each_entry_safe(matchp, tmp, &ip6t_match_node, node)
	{
		if (NULL != matchp->match) {
			eag_free(matchp->match);
			matchp->match = NULL;	
		}		
		eag_free(matchp);
		matchp = NULL;
	}

	if (NULL != target) {
		eag_free(target);
		target = NULL;
	}
	return p_entry;
}


int
eag_ipv6_noport_iprange (struct ipv6_white_black_iprange *input_info)
{
	struct eag_ip6t_entries *p_entries = NULL;
	struct eag_ip6t_entries *tmp;
	struct list_head ip6t_entry_node;

	if(strcmp(input_info->portstring, "all")) {
		eag_log_err("eag_noport_iprang input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	INIT_LIST_HEAD(&ip6t_entry_node);
/* filter */
	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "all",
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);
	
	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_DST, "all", 
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);

	eag_ip6table_commit("filter", input_info->chain_name,
		&ip6t_entry_node, input_info->flag);

	
/* free p_entry */
	if (0 == strcmp(input_info->nat_chain_name, "") 
		|| 0 == strcmp(input_info->nat_target_name, "")) {
		eag_log_debug("eag_iptables", "nat_chain_name or nat_target_name is null");
		goto return_line;
	}

	list_for_each_entry_safe(p_entries, tmp, &ip6t_entry_node, node) {
		if (NULL != p_entries->entry) {
			eag_free(p_entries->entry);
			p_entries->entry = NULL;
		}
	}
	
/* nat */
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_DST, "all",
		input_info, input_info->nat_target_name);
	//p_entry = p_entry->next;
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);

	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "all",
		input_info, input_info->nat_target_name);

	eag_ip6table_commit("nat", input_info->nat_chain_name,
		&ip6t_entry_node, input_info->flag);

/* free p_entry */
return_line:
	list_for_each_entry_safe(p_entries, tmp, &ip6t_entry_node, node) {
		if (NULL != p_entries->entry) {
			eag_free(p_entries->entry);
			p_entries->entry = NULL;
		}
		eag_free(p_entries);
		p_entries = NULL;
	}

	return 0;
}

int 
eag_ipv6_port_iprange(struct ipv6_white_black_iprange *input_info)
{
	struct eag_ip6t_entries *p_entries = NULL;
	struct eag_ip6t_entries *tmp;
	struct list_head ip6t_entry_node;
	//struct eag_ipt_entries *entries = NULL;

	INIT_LIST_HEAD(&ip6t_entry_node);
/* filter */
	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "udp",
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);

	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_DST, "udp", 
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);

	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "tcp",
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);

	p_entries = eag_malloc(sizeof(struct eag_ip6t_entries));
	if (NULL == p_entries) {
		eag_log_err("malloc error:p_entry == NULL");
		return EAG_ERR_MALLOC_FAILED;
	}
	p_entries->entry = eag_iprange_entry(EAG_IP6TABLE_DST, "tcp", 
		input_info, input_info->target_name);

	list_add(&(p_entries->node), &ip6t_entry_node);

	eag_ip6table_commit("filter", input_info->chain_name,
		&ip6t_entry_node, input_info->flag);
/* free p_entry */
	list_for_each_entry_safe(p_entries, tmp, &ip6t_entry_node, node)
	{
		if (NULL != p_entries->entry) {
			eag_free(p_entries->entry);
			p_entries->entry = NULL;
		}
	}
	
#if 1
/* nat */
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);
	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_DST, "tcp",
		input_info, input_info->nat_target_name);
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);

	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "tcp",
		input_info, input_info->nat_target_name);
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);

	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_DST, "udp",
		input_info, input_info->nat_target_name);
	p_entries = list_entry(p_entries->node.next, struct eag_ip6t_entries, node);

	p_entries->entry = eag_ipv6_iprange_entry(EAG_IP6TABLE_SRC, "udp",
		input_info, input_info->nat_target_name);

	eag_ip6table_commit("nat", input_info->nat_chain_name,
		&ip6t_entry_node, input_info->flag);
#endif
/* free p_entry */
	list_for_each_entry_safe(p_entries, tmp, &ip6t_entry_node, node)
	{
		if (NULL != p_entries->entry) {
			eag_free(p_entries->entry);
			p_entries->entry = NULL;
		}
		eag_free(p_entries);
		p_entries = NULL;
	}
	
	return 0;
}

int 
eag_ip6table_iprange(struct ipv6_white_black_iprange *input_info)
{	
	if (strcmp(input_info->portstring, "all")) {
		eag_log_debug("eag_ip6tables", "***********port*************");
		eag_ipv6_port_iprange(input_info);
	} else {
		eag_log_debug("eag_ip6tables", "***********no port***********");
		eag_ipv6_noport_iprange(input_info);			
	}
	return 0;
}

int
eag_ip6table_white_domain(struct ipv6_white_black_iprange *input_info)
{
	if (input_info->comment_str[0] != '\0') {
		eag_ipv6_noport_iprange(input_info);	
	} else {
		eag_log_warning("input error");
		return -1;
	}
	return 0;
}

int
eag_ip6table_black_domain(struct ipv6_white_black_iprange *input_info)
{
	if (input_info->comment_str[0] != '\0') {
		eag_ipv6_noport_iprange(input_info);	
	} else {
		eag_log_warning("input error");
		return -1;
	}
	return 0;
}

int
eag_ip6table_add_interface_filter_commit(int insid, char ins_type, char *intf, char *setname)
{	
	char cap_filter_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_DEFAULT
	char cap_auth_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_AUTHORIZED_DEFAULT
	char cap_auth_intf_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_AUTH_IF
	char cap_auth_intf_in_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_AUTH_IF_IN

	snprintf(cap_filter_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_DEFAULT", ins_type, insid);
	snprintf(cap_auth_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_AUTH_DEFAULT", ins_type, insid);
	snprintf(cap_auth_intf_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_%s", ins_type, insid, intf);
	snprintf(cap_auth_intf_in_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_%s_IN", ins_type, insid, intf);

	struct ip6tc_handle *handle = NULL;
	struct ip6t_entry *entry = NULL;
	struct eag_ipv6_intf_entry_info intf_info = {0};
	int ret = 0;

#if USE_THREAD_LOCK	
	eag_log_debug("eag_ip6tables","eag_ip6table_glock lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
	/*use iptables lock*/
	eag_log_debug("eag_ip6tables","ip6table_lock lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	handle = ip6tc_init("filter");
	if (!handle) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_init error:%s", ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -N $CP_FILTER_AUTH_IF */
	ret = ip6tc_create_chain(cap_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_create_chain %s error:%s", 
								cap_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -I $CP_FILTER_AUTH_IF -j RETURN */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_auth_intf_chain;
	intf_info.target = "RETURN";
	entry = eag_ipv6_set_intf_entry(&intf_info);

	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	
/* ip6tables -I $CP_FILTER_AUTH_IF -m set --match-set ${CP_IPHASH_IPV6_SET} src -j ${CP_FILTER_AUTHORIZED_DEFAULT} */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_auth_intf_chain;
	intf_info.setname = setname;
	intf_info.setflag = "src";
	intf_info.target = cap_auth_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

/* ip6tables -N $CP_FILTER_AUTH_IF_IN */
	ret = ip6tc_create_chain(cap_auth_intf_in_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_create_chain %s error:%s", 
								cap_auth_intf_in_chain, ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -I $CP_FILTER_AUTH_IF_IN -j RETURN */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_auth_intf_in_chain;
	intf_info.target = "RETURN";
	entry = eag_ipv6_set_intf_entry(&intf_info);

	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	
/* ip6tables -I $CP_FILTER_AUTH_IF_IN -m set --match-set ${CP_IPHASH_IPV6_SET} dst -j ${CP_FILTER_AUTHORIZED_DEFAULT}*/
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_auth_intf_in_chain;
	intf_info.setname = setname;
	intf_info.setflag = "dst";
	intf_info.target = cap_auth_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

/* ip6tables -I $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_DEFAULT} */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf = intf;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.target = cap_filter_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

/* ip6tables -I $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_AUTH_IF} */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf = intf;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.target = cap_auth_intf_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	
/* ip6tables -I $CP_FILTER -o ${CP_IF} -j ${CP_FILTER_AUTH_IF_IN} */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf = intf;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_OUT;
	intf_info.target = cap_auth_intf_in_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_insert_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

	ret = ip6tc_commit(handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_commit:%s", ip6tc_strerror(errno));
	}

return_line:
	if (NULL != entry) {
		eag_free(entry);
		entry = NULL;
	}
	if (NULL != handle) {
		ip6tc_free(handle);
		handle = NULL;
	}

#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("eag_ip6tables","eag_ip6table_glock unlock");
#endif
	/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("eag_ip6tables","ip6table_lock unlock");

	return ret;
}

int
eag_ip6table_add_interface_nat_commit(int insid, char ins_type, char *intf, char *setname)
{
	char cap_nat_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};		//CP_NAT_DEFAULT
	char cap_nat_auth_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_NAT_AUTHORIZED_DEFAULT
	char cap_nat_auth_intf_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};		//CP_NAT_AUTH_IF

	snprintf(cap_nat_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_N_DEFAULT", ins_type, insid);
	snprintf(cap_nat_auth_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_N_AUTH_DEFAULT", ins_type, insid);
	snprintf(cap_nat_auth_intf_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_N_%s", ins_type, insid, intf);

	struct ip6tc_handle *handle = NULL;
	struct ip6t_entry *entry = NULL;
	struct eag_ipv6_intf_entry_info intf_info = {0};
	int ret = 0;

#if USE_THREAD_LOCK	
	eag_log_debug("eag_ip6tables","eag_ip6table_glock lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
	/*use iptables lock*/
	eag_log_debug("eag_ip6tables","ip6table_lock lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	handle = ip6tc_init("nat");
	if (!handle) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_init error:%s", ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -t nat -N $CP_NAT_AUTH_IF*/
	ret = ip6tc_create_chain(cap_nat_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_create_chain %s error:%s", 
								cap_nat_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -t nat -I $CP_NAT_AUTH_IF -j RETURN */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_nat_auth_intf_chain;
	intf_info.target = "RETURN";
	entry = eag_ipv6_set_intf_entry(&intf_info);

	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	
/* ip6tables -t nat -I $CP_NAT_AUTH_IF -m set --match-set ${CP_IPHASH_SET} src -j ${CP_NAT_AUTHORIZED_DEFAULT} */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = cap_nat_auth_intf_chain;
	intf_info.setname = setname;
	intf_info.setflag = "src";
	intf_info.target = cap_nat_auth_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_insert_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

/* ip6tables -t nat -I CP_DNAT -i ${CP_IF} -j $CP_NAT_DEFAULT */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = IPV6_CP_DNAT;
	intf_info.intf = intf;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.target = cap_nat_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

/* ip6tables -t nat -I CP_DNAT -i ${CP_IF} -j $CP_NAT_AUTH_IF */
	memset(&intf_info, 0, sizeof(struct eag_ipv6_intf_entry_info));
	intf_info.chain = IPV6_CP_DNAT;
	intf_info.intf = intf;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.target = cap_nat_auth_intf_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	ret = ip6tc_insert_entry(intf_info.chain, entry, 0, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit ip6tc_insert_entry %s error:%s",
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;

	ret = ip6tc_commit(handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_commit:%s", ip6tc_strerror(errno));
	}
	
return_line:
	if (NULL != entry) {
		eag_free(entry);
		entry = NULL;
	}
	if (NULL != handle) {
		ip6tc_free(handle);
		handle = NULL;
	}

#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("eag_ip6tables","eag_ip6table_glock unlock");
#endif
	/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("eag_ip6tables","ip6table_lock unlock");

	return ret;
}

int
eag_ip6table_add_interface(int insid, char ins_type, char *intf)
{
	int ret = 0;
	char cap_iphash_set[EAG_IP6TABLES_MAXNAMESIZE] = {0};

	snprintf(cap_iphash_set, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_AUTHORIZED_IPV6_SET", ins_type, insid);

	ret = eag_ip6table_add_interface_filter_commit(insid, ins_type, intf, cap_iphash_set);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit error:ret=%d", ret);
		return EAG_ERR_UNKNOWN;
	}
	
	ret = eag_ip6table_add_interface_nat_commit(insid, ins_type, intf, cap_iphash_set);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_nat_commit error:ret=%d", ret);
		return EAG_ERR_UNKNOWN;
	}
	
	return EAG_RETURN_OK;
}

int
eag_ip6table_del_interface_filter_commit(int insid, char ins_type, char * intf)
{
	char cap_filter_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_DEFAULT
	char cap_auth_intf_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_AUTH_IF
	char cap_auth_intf_in_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};	//CP_FILTER_AUTH_IF_IN

	snprintf(cap_filter_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_DEFAULT", ins_type, insid);
	snprintf(cap_auth_intf_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_%s", ins_type, insid, intf);
	snprintf(cap_auth_intf_in_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_F_%s_IN", ins_type, insid, intf);

	struct ip6tc_handle *handle = NULL;
	struct ip6t_entry *entry = NULL;
	struct eag_ipv6_intf_entry_info intf_info = {0};
	unsigned char *mask = NULL;
	int ret = 0;

#if USE_THREAD_LOCK	
	eag_log_debug("eag_ip6tables","eag_ip6table_glock lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
	/*use iptables lock*/
	eag_log_debug("eag_ip6tables","ip6table_lock lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	handle = ip6tc_init("filter");
	if (!handle) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_init error:%s", ip6tc_strerror(errno));
		goto return_line;
	}
	
/* ip6tables -D $CP_FILTER -o ${CP_IF} -j ${CP_FILTER_AUTH_IF_IN} */
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_OUT;
	intf_info.intf = intf;
	intf_info.target = cap_auth_intf_in_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	mask = ip6table_mask(entry->next_offset);
	ret = ip6tc_delete_entry(intf_info.chain, entry, mask, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	eag_free(mask);
	mask = NULL;
	
/* ip6tables -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_AUTH_IF} */
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf_flag= EAG_IPV6_INTERFACE_IN;
	intf_info.intf = intf;
	intf_info.target = cap_auth_intf_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	mask = ip6table_mask(entry->next_offset);
	ret = ip6tc_delete_entry(intf_info.chain, entry, mask, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	eag_free(mask);
	mask = NULL;
	
/* ip6tables -D $CP_FILTER -i ${CP_IF} -j ${CP_FILTER_DEFAULT} */
	intf_info.chain = IPV6_CP_FILTER;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.intf = intf;
	intf_info.target = cap_filter_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	mask = ip6table_mask(entry->next_offset);
	ret = ip6tc_delete_entry(intf_info.chain, entry, mask, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	eag_free(mask);
	mask = NULL;

/* ip6tables -F ${CP_FILTER_AUTH_IF} */
	ret = ip6tc_flush_entries(cap_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_flush_entries %s error:%s", 
								cap_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}
	
/* iptables -X ${CP_FILTER_AUTH_IF} */
	ret = ip6tc_delete_chain(cap_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_chain %s error:%s", 
								cap_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -F ${CP_FILTER_AUTH_IF_IN} */
	ret = ip6tc_flush_entries(cap_auth_intf_in_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_flush_entries %s error:%s", 
								cap_auth_intf_in_chain, ip6tc_strerror(errno));
		goto return_line;
	}
	
/* ip6tables -X ${CP_FILTER_AUTH_IF_IN} */
	ret = ip6tc_delete_chain(cap_auth_intf_in_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_chain %s error:%s", 
								cap_auth_intf_in_chain, ip6tc_strerror(errno));
		goto return_line;
	}
	
	ret = ip6tc_commit(handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_commit:%s", ip6tc_strerror(errno));
	}
	
return_line:
	if (NULL != entry) {
		eag_free(entry);
		entry = NULL;
	}
	if (NULL != handle) {
		ip6tc_free(handle);
		handle = NULL;
	}
	if (NULL != mask) {
		eag_free(mask);
		mask = NULL;
	}
	
#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("eag_ip6tables","eag_ip6table_glock unlock");
#endif
	/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("eag_ip6tables","ip6table_lock unlock");
	
	return ret;
}

int
eag_ip6table_del_interface_nat_commit(int insid, char ins_type, char * intf)
{
	char cap_nat_default_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};		//CP_NAT_DEFAULT
	char cap_nat_auth_intf_chain[EAG_IP6TABLES_MAXNAMESIZE] = {0};		//CP_NAT_AUTH_IF

	snprintf(cap_nat_default_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_N_DEFAULT", ins_type, insid);
	snprintf(cap_nat_auth_intf_chain, EAG_IP6TABLES_MAXNAMELEN, "CP_%c%d_N_%s", ins_type, insid, intf);

	struct ip6tc_handle *handle = NULL;
	struct ip6t_entry *entry = NULL;
	struct eag_ipv6_intf_entry_info intf_info = {0};
	unsigned char *mask = NULL;
	int ret = 0;

#if USE_THREAD_LOCK	
	eag_log_debug("eag_ip6tables","eag_ip6table_glock lock");
	pthread_mutex_lock( &eag_ip6tables_glock );
#endif
	/*use iptables lock*/
	eag_log_debug("eag_ip6tables","ip6table_lock lock ");
	nmp_mutex_lock(&eag_ip6tables_lock);
	
	handle = ip6tc_init("nat");
	if (!handle) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_init error:%s", ip6tc_strerror(errno));
		goto return_line;
	}
	
/* ip6tables -t nat -D CP_DNAT -i ${CP_IF} -j ${CP_NAT_AUTH_IF} */
	intf_info.chain = IPV6_CP_DNAT;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.intf = intf;
	intf_info.target = cap_nat_auth_intf_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	mask = ip6table_mask(entry->next_offset);
	ret = ip6tc_delete_entry(intf_info.chain, entry, mask, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	eag_free(mask);
	mask = NULL;

/* ip6tables -t nat -D CP_DNAT -i ${CP_IF} -j $CP_NAT_DEFAULT */
	intf_info.chain = IPV6_CP_DNAT;
	intf_info.intf_flag = EAG_IPV6_INTERFACE_IN;
	intf_info.intf = intf;
	intf_info.target = cap_nat_default_chain;
	entry = eag_ipv6_set_intf_entry(&intf_info);
	mask = ip6table_mask(entry->next_offset);
	ret = ip6tc_delete_entry(intf_info.chain, entry, mask, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_entry %s error:%s", 
								intf_info.chain, ip6tc_strerror(errno));
		goto return_line;
	}
	eag_free(entry);
	entry = NULL;
	eag_free(mask);
	mask = NULL;

/* ip6tables -t nat -F ${CP_NAT_AUTH_IF} */
	ret = ip6tc_flush_entries(cap_nat_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_flush_entries %s error:%s", 
								cap_nat_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}

/* ip6tables -t nat -X ${CP_NAT_AUTH_IF} */
	ret = ip6tc_delete_chain(cap_nat_auth_intf_chain, handle);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit ip6tc_delete_chain %s error:%s", 
								cap_nat_auth_intf_chain, ip6tc_strerror(errno));
		goto return_line;
	}

	ret = ip6tc_commit(handle);
	if (!ret) {
		eag_log_err("eag_ip6table_add_interface_filter_commit ip6tc_commit:%s", ip6tc_strerror(errno));
	}
	
return_line:
	if (NULL != entry) {
		eag_free(entry);
		entry = NULL;
	}
	if (NULL != handle) {
		ip6tc_free(handle);
		handle = NULL;
	}
	if (NULL != mask) {
		eag_free(mask);
		mask = NULL;
	}
	
#if USE_THREAD_LOCK	
	pthread_mutex_unlock( &eag_ip6tables_glock );
	eag_log_debug("eag_ip6tables","eag_ip6table_glock unlock");
#endif
	/*use iptable unlock*/
	nmp_mutex_unlock(&eag_ip6tables_lock);
	eag_log_debug("eag_ip6tables","ip6table_lock unlock");
	
	return ret;
}

int
eag_ip6table_del_interface(int insid, char ins_type, char *intf)
{
	int ret = 0;

	ret = eag_ip6table_del_interface_filter_commit(insid, ins_type, intf);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_filter_commit error:ret=%d", ret);
		return EAG_ERR_UNKNOWN;
	}
	
	ret = eag_ip6table_del_interface_nat_commit(insid, ins_type, intf);
	if (!ret) {
		eag_log_err("eag_ip6table_del_interface_nat_commit error:ret=%d", ret);
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

#ifdef __cplusplus
}
#endif

