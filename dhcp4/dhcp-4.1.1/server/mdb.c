/* mdb.c

   Server-specific in-memory database support. */

/*
 * Copyright (c) 2004-2009 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   https://www.isc.org/
 *
 * This software has been written for Internet Systems Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about Internet Systems Consortium, see
 * ``https://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#include "dhcpd.h"
#include "omapip/hash.h"
#include "dhcp6_dbus.h"

struct subnet *subnets;
struct shared_network *shared_networks;
host_hash_t *host_hw_addr_hash;
host_hash_t *host_uid_hash;
host_hash_t *host_name_hash;
lease_id_hash_t *lease_uid_hash;
lease_ip_hash_t *lease_ip_addr_hash;
lease_id_hash_t *lease_hw_addr_hash;
extern struct subnet_config sub_conf;

#ifndef __AX_PLATFORM__
/*address used rate file*/
FILE *whole_file;
FILE *segment_file;
/*requested times file*/
FILE *request_file;
/*response times file*/
FILE *response_file;
/*lease file about ip, mac, starts time, ends time*/
FILE *lease_file;
extern unsigned int requested_times;
extern unsigned int response_times;

/* Confpars.c */
extern int parse_statement_subnet_no_file(struct subnet_config *sub_conf,struct group *group,int type);
extern void parse_host_declaration_test(char* ,struct iaddr*,struct hardware*,struct group *);


#endif

/*
 * We allow users to specify any option as a host identifier.
 *
 * Any host is uniquely identified by the combination of 
 * option type & option data.
 *
 * We expect people will only use a few types of options as host 
 * identifier. Because of this, we store a list with an entry for
 * each option type. Each of these has a hash table, which contains 
 * hash of the option data.
 */
typedef struct host_id_info {
	struct option *option;
	host_hash_t *values_hash;
	struct host_id_info *next;
} host_id_info_t;

static host_id_info_t *host_id_info = NULL;

int numclasseswritten;

omapi_object_type_t *dhcp_type_host;

isc_result_t enter_class(cd, dynamicp, commit)
	struct class *cd;
	int dynamicp;
	int commit;
{
	if (!collections -> classes) {
		/* A subclass with no parent is invalid. */
		if (cd->name == NULL)
			return ISC_R_INVALIDARG;

		class_reference (&collections -> classes, cd, MDL);
	} else if (cd->name != NULL) {	/* regular class */
		struct class *c = 0;

		if (find_class(&c, cd->name, MDL) != ISC_R_NOTFOUND) {
			class_dereference(&c, MDL);
			return ISC_R_EXISTS;
		}
		
		/* Find the tail. */
		for (c = collections -> classes;
		     c -> nic; c = c -> nic)
			/* nothing */ ;
		class_reference (&c -> nic, cd, MDL);
	}

	if (dynamicp && commit) {
		const char *name = cd->name;

		if (name == NULL) {
			name = cd->superclass->name;
		}

		write_named_billing_class ((const unsigned char *)name, 0, cd);
		if (!commit_leases ())
			return ISC_R_IOERROR;
	}

	return ISC_R_SUCCESS;
}


/* Variable to check if we're starting the server.  The server will init as
 * starting - but just to be safe start out as false to avoid triggering new
 * special-case code
 * XXX: There is actually a server_startup state...which is never entered...
 */
#define SS_NOSYNC	1
#define SS_QFOLLOW	2
static int server_starting = 0;

static int find_uid_statement (struct executable_statement *esp,
			       void *vp, int condp)
{
	struct executable_statement **evp = vp;

	if (esp -> op == supersede_option_statement &&
	    esp -> data.option &&
	    (esp -> data.option -> option -> universe ==
	     &dhcp_universe) &&
	    (esp -> data.option -> option -> code ==
	     DHO_DHCP_CLIENT_IDENTIFIER)) {
		if (condp) {
			log_error ("dhcp client identifier may not be %s",
				   "specified conditionally.");
		} else if (!(*evp)) {
			executable_statement_reference (evp, esp, MDL);
			return 1;
		} else {
			log_error ("only one dhcp client identifier may be %s",
				   "specified");
		}
	}
	return 0;
}


static host_id_info_t *
find_host_id_info(unsigned int option_code) {
	host_id_info_t *p;

	for (p=host_id_info; p != NULL; p = p->next) {
		if (p->option->code == option_code) {
			break;
		}
	}
	return p;
}

/* Debugging code */
#if 0
isc_result_t
print_host(const void *name, unsigned len, void *value) {
	struct host_decl *h;
	printf("--------------\n");
	printf("name:'%s'\n", print_hex_1(len, name, 60));
	printf("len:%d\n", len);
	h = (struct host_decl *)value;
	printf("host @%p is '%s'\n", h, h->name);
	return ISC_R_SUCCESS;
}

void
hash_print_hosts(struct hash_table *h) {
	hash_foreach(h, print_host);
	printf("--------------\n");
}
#endif /* 0 */

#ifndef __AX_PLATFORM__
const char *
print_time_leases
(
	TIME t
)
{
	static char buf[sizeof("epoch 9223372036854775807; "
			       "# Wed Jun 30 21:49:08 2147483647")];
	
	if (t < 0) {
		return NULL;
	}	

	if (strftime(buf, sizeof(buf), "%Y/%m/%d-%H:%M:%S",
			     gmtime(&t)) == 0) {
		return NULL;
	}	

	return buf;
}

struct lease* 
find_lease_by_ip_addr_from_hw
(
	struct iaddr addr
)
{
	int i = 0, ret = 0;
	struct hash_bucket *bp = NULL, *next = NULL;
	struct hash_table *table = lease_hw_addr_hash;

	if (!table)
		return NULL;

	for (i = 0; i < table->hash_count; i++) {
		bp = table->buckets [i];
		while (bp) {
			next = bp->next;
			struct lease *lease = (struct lease *)bp->value;

			ret = memcmp(lease->ip_addr.iabuf, addr.iabuf, addr.len);
			if (!ret) {
				return lease;
			}
			bp = next;
		}
	}
	
	return NULL;
}

isc_result_t
save_lease_hw_info_file
(
	const void *key, 
	unsigned len, 
	void *object
)
{
	struct lease *lease = object;
	const char *tval;
	
	fprintf(lease_file, "%s ", piaddr(lease->ip_addr));

	if (lease->hardware_addr.hlen) {
		fprintf(lease_file, "%s ", print_hw_addr(lease->hardware_addr.hbuf [0],
					lease->hardware_addr.hlen - 1, &lease->hardware_addr.hbuf [1]));
	}
	
	if (lease->starts &&
	    ((tval = print_time_leases(lease->starts)) != NULL)) {
	     fprintf(lease_file, "%s ", tval);
	}

	if (lease->ends &&
	    ((tval = print_time_leases(lease->ends)) != NULL)) {
	     fprintf(lease_file, "%s ", tval);
	}

	fprintf(lease_file, "\n");

	return ISC_R_SUCCESS;
}

isc_result_t
save_lease_ip_info_file
(
	const void *key, 
	unsigned len, 
	void *object
)
{
	return ISC_R_SUCCESS;
}

void
save_dhcp_info_file
(
	void
) 
{
	int  whole_times = 0, segment_times = 0;
	
	whole_file = fopen("/var/run/apache2/dhcp_whole", "w+b");	
	segment_file = fopen("/var/run/apache2/dhcp_segment", "w+b");
	request_file = fopen("/var/run/apache2/dhcp_request", "w+b");
	response_file = fopen("/var/run/apache2/dhcp_response", "w+b");	
	lease_file = fopen("/var/run/apache2/dhcp_lease", "w+b");
	
	segment_times = hash_foreach(lease_hw_addr_hash, save_lease_hw_info_file);	
	whole_times = hash_foreach(lease_ip_addr_hash, save_lease_ip_info_file);
	
	fprintf(whole_file, "%d \n", whole_times);
	fprintf(segment_file, "%d \n", segment_times);
	fprintf(request_file, "%d \n", requested_times);
	fprintf(response_file, "%d \n", response_times);
/*************************test for interface */	
	struct interface_info *testtmp;
	for (testtmp = interfaces; testtmp; testtmp = testtmp->next) {
		log_info("interfaces name %s flag is %d \n", testtmp->name, testtmp->flags);
	}
/************************/	
	fclose(whole_file);
	fclose(segment_file);
	fclose(request_file);
	fclose(response_file);
	fclose(lease_file);
}

struct dhcp_lease_info*
get_dhcp_info
(
	void
)
{
	int  whole_times = 0, segment_times = 0, i = 0, count = 0;	
	struct hash_bucket *bp, *next;
	struct dhcp_lease_info *lease_info;
	struct hash_table *table = lease_hw_addr_hash;

/*when there is no lease in lease_hw_addr_hash*/		
	if (!table) {
		return NULL;
	}

renew:	
	segment_times = hash_foreach(lease_hw_addr_hash, save_lease_ip_info_file);	
	whole_times = hash_foreach(lease_ip_addr_hash, save_lease_ip_info_file);
	/* use segment_times for the number of dhcp_lease_ip_info, not (segment_times -1)
	   because segment_times maybe change at after dispose, will die in renew*/
	lease_info = (struct dhcp_lease_info*)malloc(sizeof(struct dhcp_lease_info) + 
							sizeof(struct dhcp_lease_ip_info) * segment_times);
	memset(lease_info, 0, sizeof(struct dhcp_lease_info) + 
							sizeof(struct dhcp_lease_ip_info) * segment_times);

	if (!lease_info) {
		goto renew;
	}
	lease_info->whole_times_info = whole_times;
	lease_info->segment_times_info = segment_times;
	lease_info->requested_times_info = requested_times;
	lease_info->response_times_info = response_times;

	for (i = 0; i < table->hash_count; i++) {
		bp = table->buckets [i];
		while (bp) {
			next = bp->next;			
			struct lease *lease = (struct lease *)bp->value;
			if (lease) {
				lease_info->lease_ip[count].ip_addr = lease->ip_addr;
				lease_info->lease_ip[count].starts = lease->starts;
				lease_info->lease_ip[count].ends = lease->ends;	
				/*hardware_addr.hbuf [0] is hardware type*/
				memcpy(lease_info->lease_ip[count].hw_addr, &lease->hardware_addr.hbuf[1], 6);
			}
			bp = next;
			if (count > segment_times) {
				free(lease_info);
				goto renew;
			}
			count++;
		}
	}
	
	return lease_info;
}

/*add string len agrc*/
int 
get_lease_ip_info_by_hw
(
	unsigned char *hw_addr, 
	struct dhcp_lease_ip_info *lease_ip
)
{
	struct lease *hw_lease = (struct lease *)0;
	struct hardware h;
	struct dhcp_lease_ip_info *lease_ip_info = lease_ip;
	int ret = 0;
	
	if (!hw_addr || !lease_ip_info) {
		return 0;
	}
	
	h.hlen = 6 + 1;
	h.hbuf[0] = HTYPE_ETHER;
	memcpy(&h.hbuf[1], hw_addr, 6);
	ret = find_lease_by_hw_addr(&hw_lease, h.hbuf, h.hlen, MDL);

	if (ret && hw_lease) {
		lease_ip_info->ip_addr = hw_lease->ip_addr;
		lease_ip_info->starts = hw_lease->starts;
		lease_ip_info->ends = hw_lease->ends; 
		/*hardware_addr.hbuf [0] is hardware type*/
		memcpy(lease_ip_info->hw_addr, &hw_lease->hardware_addr.hbuf[1], 6);
	}
	
	return ret;
}
/**********************************************************************************
 *  get_dhcp_lease_ipv6_state_num
 *
 *	DESCRIPTION:
 * 		get total lease status 
 *
 *	INPUT: 		
 *		
 *			
 *	OUTPUT:
 *		lease_state
 *		sub_lease_state
 *		sub_count
 * 	RETURN:		
 *		 
 *		 0 	->  ok
 *		 other	 ->  fail 
 **********************************************************************************/
int get_dhcp_lease_ipv6_state_num
(
	struct dbus_lease_state  *lease_state , 
	struct dbus_sub_lease_state **sub_lease_state, 
	unsigned int *sub_count
)
{	
	int i = 0, j = 0 ,start_pool = 0;
	struct subnet *rv = NULL;
	struct ipv6_pool *next = NULL;
	struct dcli_pool* poolnode = NULL;	
	struct dbus_sub_lease_state *tmp_sub_state = NULL;

	unsigned int total_lease = 0 ,  free_lease = 0, active_lease = 0, backup_lease = 0;
	unsigned int sub_lease_count = 0, sub_lease_free = 0, sub_lease_active = 0, sub_lease_backup = 0;  

	unsigned int sub_num = 0;
	//struct lease *tmp_lease = NULL;
	
	
	if(!lease_state  || !sub_count){
		log_error("get_dhcp_lease_state_num function input argument %s %s\n",
				lease_state ? "":"lease_state is NULL", sub_lease_state ? "":"sub_lease_state is NULL");
		return 1;
	}
	/*get subnet number*/
	for (rv = subnets; rv; rv = rv->next_subnet){
		++sub_num;
	}

	*sub_count = sub_num;	
	
	tmp_sub_state = malloc(sizeof(struct dbus_sub_lease_state) * sub_num);
	if(NULL == tmp_sub_state){
		log_error("get_dhcp_lease_ipv6_state_num function malloc fail in %s on %d line\n", __FILE__, __LINE__);
		return 1;
	}
	memset(tmp_sub_state, 0, sizeof(struct dbus_sub_lease_state) * sub_num);


	for (rv = subnets; rv; rv = rv->next_subnet) 
	{
		sub_lease_count = 0;
		sub_lease_free = 0;
		sub_lease_active = 0;
		sub_lease_backup = 0;
		
		if ((rv->shared_network) && (rv->shared_network->ipv6_pools) && *(rv->shared_network->ipv6_pools)) 
		{
			start_pool = rv->shared_network->last_ipv6_pool;
			j = start_pool;
			//while (*next ) 
			do{
				next = rv->shared_network->ipv6_pools[j];
				if(next)
				{
							
					sub_lease_count += next->lease_count;
					//sub_lease_free  += next->free_leases;				
					//sub_lease_backup +=  next->backup_leases;
					sub_lease_active += next->num_active;
					/*
					for(tmp_lease = next->active; tmp_lease ; tmp_lease = tmp_lease->next){
						++sub_lease_active;
					}*/
						
					//next = next->next;
					
				}
				j++;
				if (next == NULL) {
					j = 0;
				}
			}while(j != start_pool);
		}	

		poolnode = dhcp6_dbus_find_poolnode_by_subnet(rv);
		if (poolnode) {
			//sprintf(tmp_sub_state[i].poolname , "%s", poolnode->poolname);
			memcpy(tmp_sub_state[i].poolname, poolnode->poolname, strlen(poolnode->poolname));
		}
		/*
		if (poolnode = dhcp_dbus_find_poolnode_by_subnet(rv)) {
			sprintf(tmp_sub_state[i].poolname , "%s", poolnode->poolname);
		}*/
		sprintf(tmp_sub_state[i].subnet , "%s", piaddr(rv->net));
		sprintf(tmp_sub_state[i].mask, "%s", piaddr(rv->netmask));
		tmp_sub_state[i].subnet_lease_state.total_lease_num= sub_lease_count;
		tmp_sub_state[i].subnet_lease_state.free_lease_num= sub_lease_free;
		tmp_sub_state[i].subnet_lease_state.active_lease_num= sub_lease_active;
		tmp_sub_state[i].subnet_lease_state.backup_lease_num = sub_lease_backup;

		//tmp_sub_state[i].info.discover_times = rv->discover_times;
		//tmp_sub_state[i].info.offer_times= rv->offer_times;
		//tmp_sub_state[i].info.requested_times= rv->requested_times;
		//tmp_sub_state[i].info.ack_times= rv->response_times;
		
		log_debug("sub %s %s total is %d  total active %d\n", 
			tmp_sub_state[i].subnet, tmp_sub_state[i].mask,sub_lease_count, sub_lease_active);

		total_lease += sub_lease_count;
		//free_lease  += sub_lease_free;
		//backup_lease += sub_lease_backup;
		active_lease += sub_lease_active;
		++i;
	}	
	lease_state->total_lease_num = total_lease;
	lease_state->free_lease_num  = free_lease;
	lease_state->backup_lease_num = backup_lease;
	lease_state->active_lease_num = active_lease;
	*sub_lease_state = tmp_sub_state;

	return 0;
}

struct dhcp_lease_info*
get_lease_ip_info_by_ip
(
	unsigned int ip_addr, 
	unsigned int ip_nums
)
{
	struct lease *ip_lease = (struct lease *)0;
	struct iaddr cip;
	struct dhcp_lease_info *lease_info;
	unsigned int ip_Addr = 0, ip_Nums = 0, i = 0;

	lease_info = (struct dhcp_lease_info*)malloc(sizeof(struct dhcp_lease_info));
	memset(lease_info, 0, sizeof(struct dhcp_lease_info));
	ip_Nums = ip_nums;
	ip_Addr = ip_addr;
	cip.len = 4;
	while (ip_Nums) {
		cip.iabuf[0] = ip_Addr>>24 & 0xff;
		cip.iabuf[1] = ip_Addr>>16 & 0xff;
		cip.iabuf[2] = ip_Addr>>8 & 0xff;
		cip.iabuf[3] = ip_Addr & 0xff;
		ip_lease = find_lease_by_ip_addr_from_hw(cip);

		if (ip_lease) {
			struct dhcp_lease_ip_info *lease_ip_info = malloc(sizeof(struct dhcp_lease_ip_info));
			memset(lease_ip_info, 0, sizeof(struct dhcp_lease_ip_info));
			lease_ip_info->ip_addr = ip_lease->ip_addr;
			lease_ip_info->starts = ip_lease->starts;
			lease_ip_info->ends = ip_lease->ends; 
			/*hardware_addr.hbuf [0] is hardware type*/
			memcpy(lease_ip_info->hw_addr, &ip_lease->hardware_addr.hbuf[1], 6);
			lease_ip_info->next = lease_info->lease_ip->next;
			lease_info->lease_ip->next = lease_ip_info;
			i++;
		}
		
		ip_Nums--;
		ip_Addr++;
	}
	lease_info->segment_times_info = i;
	
	return lease_info;
}

/* for test dhcpd conf*/
char *path_dhcpd_conf_test = "/etc/dhcpd_test.conf";

int test_for_add_dhcp_file(void)
{
	struct iaddr addr, addr1, addr2;
	char *value = NULL;

	/*test for add interface to the list*/
#ifdef INTERFACE_NO_UP
	char *testname = "vlan100";
	isc_result_t result;
	struct interface_info *tmp =
		(struct interface_info *)0;
	result = interface_allocate (&tmp, MDL);
	if (result != ISC_R_SUCCESS)
		log_fatal ("Insufficient memory to %s %s: %s",
			   "record interface", testname,
			   isc_result_totext (result));
	strcpy (tmp -> name, testname);
	if (interfaces) {
		interface_reference (&tmp -> next,
				     interfaces, MDL);
		interface_dereference (&interfaces, MDL);
	}
	interface_reference (&interfaces, tmp, MDL);
	tmp -> flags = INTERFACE_REQUESTED;
#endif	
	/************************************/

	struct subnet_config sub_conf;

	memset(&sub_conf, 0, sizeof(struct subnet_config));
	sub_conf.net.iabuf[0] = 192;
	sub_conf.net.iabuf[1] = 168;
	sub_conf.net.iabuf[2] = 4;
	sub_conf.net.iabuf[3] = 0;
	sub_conf.net.len = 4;
	sub_conf.netmask.iabuf[0] = 255;	
	sub_conf.netmask.iabuf[1] = 255;
	sub_conf.netmask.iabuf[2] = 255;
	sub_conf.netmask.iabuf[3] = 0;
	sub_conf.netmask.len = 4;
	sub_conf.range_conf.high.iabuf[0] = 192;
	sub_conf.range_conf.high.iabuf[1] = 168;
	sub_conf.range_conf.high.iabuf[2] = 4;
	sub_conf.range_conf.high.iabuf[3] = 100;
	sub_conf.range_conf.high.len = 4;
	sub_conf.range_conf.low.iabuf[0] = 192;	
	sub_conf.range_conf.low.iabuf[1] = 168;
	sub_conf.range_conf.low.iabuf[2] = 4;
	sub_conf.range_conf.low.iabuf[3] = 5;
	sub_conf.range_conf.low.len = 4;
	
	parse_statement_subnet_no_file(&sub_conf, root_group, ROOT_GROUP);

/*add for server option*/
addr.iabuf[0] = 192;
addr.iabuf[1] = 168;
addr.iabuf[2] = 4;
addr.iabuf[3] = 1;
addr.len = 4;

addr1.iabuf[0] = 19;
addr1.iabuf[1] = 16;
addr1.iabuf[2] = 4;
addr1.iabuf[3] = 1;
addr1.len = 4;

addr2.iabuf[0] = 12;
addr2.iabuf[1] = 18;
addr2.iabuf[2] = 4;
addr2.iabuf[3] = 1;
addr2.len = 4;


//struct iaaddr* *routervlaue = NULL:
//	routervlaue = (struct iaaddr*)malloc(sizeof(struct iaaddr));
//	routervlaue = ad

/* test of more ip address*/
value = malloc(sizeof(struct iaddr) * 2);
memset(value, 0, sizeof(struct iaddr) * 2);
memcpy(value, &addr1, sizeof(struct iaddr));
memcpy(value+sizeof(struct iaddr), &addr2, sizeof(struct iaddr));

/**************************
	find_subnet(&subnetop, addr, MDL);
	if (subnetop) {
		log_debug("find subnet success \n");		
		set_server_option(maxtime, testmaxtime, testlen, 0,subnetop->group);
		set_server_option(routername, (char *)(&addr), testlen, 1, subnetop->group);
		set_server_option(wins, (char *)(&addr1), testlen, 1, subnetop->group);
		set_server_option(domainname, testvalue, testlen, 1, subnetop->group);	
		set_server_option(dns, (char *)(&addr1), testlen, 1, subnetop->group);
		testlen = 0x00000201;
		set_server_option(dns, value, testlen, 1, subnetop->group);			
	}
********************/

/*test for add host*/
char* hostname = "host1";
struct iaddr* hostipaddr = NULL;
struct hardware* hosthwaddr = NULL;

char testhw[6] = {0x0, 0x1a, 0xa0, 0xb9, 0x3d, 0x60};

hosthwaddr = malloc(sizeof (struct hardware));
memset(hosthwaddr, 0, sizeof (struct hardware));
memcpy(&hosthwaddr->hbuf[1], testhw, 6);
hosthwaddr->hlen = 7;
hosthwaddr->hbuf[0] = HTYPE_ETHER;
log_debug("test for add host \n");		
if(hostipaddr){
parse_host_declaration_test(hostname, hostipaddr, hosthwaddr, root_group);
}

/*******************/
/*	
	ret = read_conf_file(path_dhcpd_conf_test, root_group, ROOT_GROUP, 0);	
	*/
	lease_file = fopen("/var/run/apache2/dhcp_lease", "w+b");
	hash_foreach(lease_ip_addr_hash, save_lease_hw_info_file);
	fclose(lease_file);


/*	expire_all_pools_test(); for start up, deny this handle*/

/*
	test_restart = 1;
	char *name = "vlan100";
	discover_interfaces_test(1, name);for start up, deny this handle*/
	
	struct interface_info *testtmp;
	for (testtmp = interfaces; testtmp; testtmp = testtmp->next) {
		log_info("interfaces name %s flag is %d \n", testtmp->name, testtmp->flags);
	}
	if(value){
		free(value);
		value=NULL;
	}
	if(hosthwaddr){
		free(hosthwaddr);
		hosthwaddr=NULL;
	}
	return 0;
}

int lease_delete_by_subnet(struct subnet *sub, unsigned int num) 
{
	int i, count = 0;
	struct hash_bucket *bp, *next;
	struct hash_table *hw_table = lease_hw_addr_hash;
	struct hash_table *ip_table = lease_ip_addr_hash;

	if (!hw_table || !ip_table) {
		return 0;
	}

	for (i = 0; i < hw_table->hash_count; i++) {
		bp = hw_table->buckets[i];
		while (bp) {
			next = bp->next;
			struct lease *lease = (struct lease *)bp->value;
			if (!lease || (lease->ip_addr.len)) {		
				if (addr_eq(sub->net, subnet_number(lease->ip_addr, sub->netmask))) {
					hw_hash_delete(lease);
				}
			}
			bp = next;
		}
	}

	for(i = 0; i < ip_table->hash_count; i++) {
		bp = ip_table->buckets[i];
		while (bp) {		
			next = bp->next;
			struct lease *lease = (struct lease *)bp->value;
			if (!lease || (lease->ip_addr.len)) {		
				if (addr_eq(sub->net, subnet_number(lease->ip_addr, sub->netmask))) {
					lease_ip_hash_delete(lease_ip_addr_hash, lease->ip_addr.iabuf, lease->ip_addr.len, MDL);
/*			log_debug("lease_delete_by_subnet delete ip %s \n", piaddr(lease->ip_addr));*/
				}
			}
			/*test number, maybe change hash to unuse this work*/
			if (((num - 3) == count) && (count > 10000)) {
				log_debug("delete ip num is %d, count is %d \n", num, count);
				return 0;
			}
			count++;
			bp = next;
		}
	}
	return 0;
}

void 
del_subnet_test
(
	struct subnet *subnode
)
{
	struct subnet *subtmp, *ip, *last;

	subtmp = (struct subnet *)subnode;

	/* remove from subnets */
	last = 0;
	for (ip = subnets; ip; ip = ip -> next_subnet)  {
		if (ip == subtmp) {
			log_info("del_subnet_test ip is %s \n", piaddr(ip->net));
			if (last) {
				subnet_dereference (&last -> next_subnet, MDL);
				if (ip -> next_subnet)
					subnet_reference (&last -> next_subnet,
				ip -> next_subnet, MDL);
			} else {
				subnet_dereference (&subnets, MDL);
				if (ip -> next_subnet)
				subnet_reference (&subnets,
				ip -> next_subnet, MDL);
			}
			if (ip -> next_subnet)
				subnet_dereference (&ip -> next_subnet, MDL);
			break;
		}
		last = ip;
	}

}

isc_result_t
lease_delete_test(const void *key, unsigned len, void *object)
{
	struct lease *lease = object;
	
	if (lease->ip_addr.len) {		
		log_debug("lease_delete_by_subnet delete ip %s\n", piaddr(lease->ip_addr));
		if (addr_eq(sub_conf.net, subnet_number(lease->ip_addr, sub_conf.netmask))) {
			lease_ip_hash_delete(lease_ip_addr_hash, lease->ip_addr.iabuf, lease->ip_addr.len, MDL);
		}
		/* Record the lease in the uid hash if possible. */
		if (lease->uid) {
			uid_hash_add (lease);
		}

		/* Record it in the hardware address hash if possible. */
		if (lease->hardware_addr.hlen) {
			hw_hash_add (lease);
		}
	}
	
	return ISC_R_SUCCESS;
}

void 
test_for_del_subnet_pool
(
	struct dcli_subnet *delsub
)
{
	struct iaddr addr;
	struct shared_network *shared_nw;
	struct subnet *rv;
	int i;

	addr = delsub->ipaddr;
	
	for (rv = subnets; rv; rv = rv->next_subnet) {
		if (addr_eq (subnet_number (addr, rv->netmask), rv->net)) {
			log_info("del subnet ipv6 address is %s \n", piaddr(addr));					
			if (rv->shared_network) {
				shared_nw = rv->shared_network;
				
				if (shared_nw->ipv6_pools) {					
					/*
					 * Do some cleanup of our leases.
					 */					
					for(i = 0; i <= shared_nw->last_ipv6_pool; i++){
						delete_pool_lease(shared_nw->ipv6_pools[i]);
						ipv6_pool_dereference(&(shared_nw->ipv6_pools[i]), MDL);
					}				
					
				}
	
				dhcp_shared_network_destroy((omapi_object_t *)rv->shared_network, MDL);

			}			
	
			log_debug("-------> delete subnet ipv6 %s \n", piaddr(rv->net));			
			dhcp_subnet_destroy((omapi_object_t *)rv, MDL);
			del_subnet_test(rv);
			
			break;
		}
	}
	
}

void 
del_interface_test
(
	struct interface_info *ifnode
)
{
	struct interface_info *tmp, *ip, *last;

	tmp = (struct interface_info *)ifnode;

	/* remove from interfaces */
	last = 0;
	for (ip = interfaces; ip; ip = ip -> next)  {
		if (ip == tmp) {
			log_info("del_interface_test ifname is %s \n",  ip->name);
			if (last) {
				interface_dereference (&last -> next, MDL);
				if (ip -> next)
					interface_reference (&last -> next,
				ip -> next, MDL);
			} else {
				interface_dereference (&interfaces, MDL);
				if (ip -> next)
				interface_reference (&interfaces,
				ip -> next, MDL);
			}
			if (ip -> next)
				interface_dereference (&ip -> next, MDL);
			break;
		}
		last = ip;
	}

}
/*
 * This function joins the interface to DHCPv6 multicast groups so we will
 * receive multicast messages.
 */
static void
if_deregister_multicast(struct interface_info *info) {
	int sock = info->rfdesc;
	struct ipv6_mreq mreq;

	if (inet_pton(AF_INET6, All_DHCP_Relay_Agents_and_Servers,
		      &mreq.ipv6mr_multiaddr) <= 0) {
		log_fatal("inet_pton: unable to convert '%s'", 
			  All_DHCP_Relay_Agents_and_Servers);
	}
	mreq.ipv6mr_interface = if_nametoindex(info->name);
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, 
		       &mreq, sizeof(mreq)) < 0) {
		log_fatal("setsockopt: IPV6_JOIN_GROUP: %m");
	}

	/*
	 * The relay agent code sets the streams so you know which way
	 * is up and down.  But a relay agent shouldn't join to the
	 * Server address, or else you get fun loops.  So up or down
	 * doesn't matter, we're just using that config to sense this is
	 * a relay agent.
	 */
	if ((info->flags & INTERFACE_STREAMS) == 0) {
		if (inet_pton(AF_INET6, All_DHCP_Servers,
			      &mreq.ipv6mr_multiaddr) <= 0) {
			log_fatal("inet_pton: unable to convert '%s'", 
				  All_DHCP_Servers);
		}
		mreq.ipv6mr_interface = if_nametoindex(info->name);
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, 
			       &mreq, sizeof(mreq)) < 0) {
			log_fatal("setsockopt: IPV6_JOIN_GROUP: %m");
		}
	}
}

void 
test_for_del_subnet_interface
(
	char* interface_name
)
{
	struct interface_info *tmpinterface = NULL;

	for (tmpinterface = interfaces; tmpinterface; tmpinterface = tmpinterface->next) {
		if (!strcmp(tmpinterface->name, interface_name)) {
			log_debug("delete interface for subnet interface name is %s \n", interface_name);
			if_deregister_multicast(tmpinterface);
			if_deregister6(tmpinterface);
			omapi_unregister_io_object((omapi_object_t *)tmpinterface);
			dhcp_interface_destroy((omapi_object_t *)tmpinterface, MDL);
			del_interface_test(tmpinterface);
			break;
		}
	}
	
	for (tmpinterface = interfaces; tmpinterface; tmpinterface = tmpinterface->next) {
		log_info("interfaces name %s flag is %d \n", tmpinterface->name, tmpinterface->flags);
	}

}
#endif
void
change_host_uid(struct host_decl *host, const char *uid, int len) {
	/* XXX: should consolidate this type of code throughout */
	if (host_uid_hash == NULL) {
		if (!host_new_hash(&host_uid_hash, HOST_HASH_SIZE, MDL)) {
			log_fatal("Can't allocate host/uid hash");
		}
	}

	/* 
	 * Remove the old entry, if one exists.
	 */
	if (host->client_identifier.data != NULL) {
		host_hash_delete(host_uid_hash,
				 host->client_identifier.data,
				 host->client_identifier.len,
				 MDL);
		data_string_forget(&host->client_identifier, MDL);
	}

	/* 
	 * Set our new value.
	 */
	memset(&host->client_identifier, 0, sizeof(host->client_identifier));
	host->client_identifier.len = len;
	if (!buffer_allocate(&host->client_identifier.buffer, len, MDL)) {
		log_fatal("Can't allocate uid buffer");
	}
	host->client_identifier.data = host->client_identifier.buffer->data;
	memcpy((char *)host->client_identifier.data, uid, len);

	/*
	 * And add to hash.
	 */
	host_hash_add(host_uid_hash, host->client_identifier.data, 
		      host->client_identifier.len, host, MDL);
}

isc_result_t enter_host (hd, dynamicp, commit)
	struct host_decl *hd;
	int dynamicp;
	int commit;
{
	struct host_decl *hp = (struct host_decl *)0;
	struct host_decl *np = (struct host_decl *)0;
	struct executable_statement *esp;
	host_id_info_t *h_id_info;
	int t = 0;

	if (!host_name_hash) {
		if (!host_new_hash(&host_name_hash, HOST_HASH_SIZE, MDL))
			log_fatal ("Can't allocate host name hash");
		host_hash_add (host_name_hash,
			       (unsigned char *)hd -> name,
			       strlen (hd -> name), hd, MDL);
	} else {
		host_hash_lookup (&hp, host_name_hash,
				  (unsigned char *)hd -> name,
				  strlen (hd -> name), MDL);

		/* If it's deleted, we can supersede it. */
		if (hp && (hp -> flags & HOST_DECL_DELETED)) {
			host_hash_delete (host_name_hash,
					  (unsigned char *)hd -> name,
					  strlen (hd -> name), MDL);
			/* If the old entry wasn't dynamic, then we
			   always have to keep the deletion. */
			if (hp -> flags & HOST_DECL_STATIC) {
				hd -> flags |= HOST_DECL_STATIC;
			}
			host_dereference (&hp, MDL);
		}

		/* If we are updating an existing host declaration, we
		   can just delete it and add it again. */
		if (hp && hp == hd) {
			host_dereference (&hp, MDL);
			delete_host (hd, 0);
			if (!write_host (hd))
				return ISC_R_IOERROR;
			hd -> flags &= ~HOST_DECL_DELETED;
		}

		/* If there isn't already a host decl matching this
		   address, add it to the hash table. */
		if (!hp) {
			host_hash_add (host_name_hash,
				       (unsigned char *)hd -> name,
				       strlen (hd -> name), hd, MDL);
		} else {
			/* XXX actually, we have to delete the old one
			   XXX carefully and replace it.   Not done yet. */
			host_dereference (&hp, MDL);
			return ISC_R_EXISTS;
		}
	}

	if (hd -> n_ipaddr)
		host_dereference (&hd -> n_ipaddr, MDL);

	if (!hd -> type)
		hd -> type = dhcp_type_host;

	if (hd -> interface.hlen) {
		if (!host_hw_addr_hash) {
			if (!host_new_hash(&host_hw_addr_hash,
					   HOST_HASH_SIZE, MDL))
				log_fatal ("Can't allocate host/hw hash");
		} else {
			/* If there isn't already a host decl matching this
			   address, add it to the hash table. */
			host_hash_lookup (&hp, host_hw_addr_hash,
					  hd -> interface.hbuf,
					  hd -> interface.hlen, MDL);
		}
		if (!hp)
			host_hash_add (host_hw_addr_hash, hd -> interface.hbuf,
				       hd -> interface.hlen, hd, MDL);
		else {
			/* If there was already a host declaration for
			   this hardware address, add this one to the
			   end of the list. */
			for (np = hp; np -> n_ipaddr; np = np -> n_ipaddr)
				;
			host_reference (&np -> n_ipaddr, hd, MDL);
			host_dereference (&hp, MDL);
		}
	}

	/* See if there's a statement that sets the client identifier.
	   This is a kludge - the client identifier really shouldn't be
	   set with an executable statement. */
	esp = (struct executable_statement *)0;
	if (executable_statement_foreach (hd -> group -> statements,
					  find_uid_statement, &esp, 0)) {
		t=evaluate_option_cache (&hd -> client_identifier,
				       (struct packet *)0,
				       (struct lease *)0,
				       (struct client_state *)0,
				       (struct option_state *)0,
				       (struct option_state *)0, &global_scope,
				       esp -> data.option, MDL);
		if(!t){
			log_error("evaluate_option_cache failed!\n");
		}
	}

	/* If we got a client identifier, hash this entry by
	   client identifier. */
	if (hd -> client_identifier.len) {
		/* If there's no uid hash, make one; otherwise, see if
		   there's already an entry in the hash for this host. */
		if (!host_uid_hash) {
			if (!host_new_hash(&host_uid_hash,
					   HOST_HASH_SIZE, MDL))
				log_fatal ("Can't allocate host/uid hash");

			host_hash_add (host_uid_hash,
				       hd -> client_identifier.data,
				       hd -> client_identifier.len,
				       hd, MDL);
		} else {
			/* If there's already a host declaration for this
			   client identifier, add this one to the end of the
			   list.  Otherwise, add it to the hash table. */
			if (host_hash_lookup (&hp, host_uid_hash,
					      hd -> client_identifier.data,
					      hd -> client_identifier.len,
					      MDL)) {
				/* Don't link it in twice... */
				if (!np) {
					for (np = hp; np -> n_ipaddr;
					     np = np -> n_ipaddr) {
						if (hd == np)
						    break;
					}
					if (hd != np)
					    host_reference (&np -> n_ipaddr,
							    hd, MDL);
				}
				host_dereference (&hp, MDL);
			} else {
				host_hash_add (host_uid_hash,
					       hd -> client_identifier.data,
					       hd -> client_identifier.len,
					       hd, MDL);
			}
		}
	}


	/*
	 * If we use an option as our host identifier, record it here.
	 */
	if (hd->host_id_option != NULL) {
		/*
		 * Look for the host identifier information for this option,
		 * and create a new entry if there is none.
		 */
		h_id_info = find_host_id_info(hd->host_id_option->code);
		if (h_id_info == NULL) {
			h_id_info = dmalloc(sizeof(*h_id_info), MDL);
			if (h_id_info == NULL) {
				log_fatal("No memory for host-identifier "
					  "option information.");
			}
			option_reference(&h_id_info->option, 
					 hd->host_id_option, MDL);
			if (!host_new_hash(&h_id_info->values_hash, 
					   HOST_HASH_SIZE, MDL)) {
				log_fatal("No memory for host-identifier "
					  "option hash.");
			}
			h_id_info->next = host_id_info;
			host_id_info = h_id_info;
		}

		if (host_hash_lookup(&hp, h_id_info->values_hash, 
				     hd->host_id.data, hd->host_id.len, MDL)) {
			/* 
			 * If this option is already present, then add 
			 * this host to the list in n_ipaddr, unless
			 * we have already done so previously.
			 *
			 * XXXSK: This seems scary to me, but I don't
			 *        fully understand how these are used. 
			 *        Shouldn't there be multiple lists, or 
			 *        maybe we should just forbid duplicates?
			 */
			if (np == NULL) {
				np = hp;
				while (np->n_ipaddr != NULL) {
					np = np->n_ipaddr;
				}
				if (hd != np) {
					host_reference(&np->n_ipaddr, hd, MDL);
				}
			}
			host_dereference(&hp, MDL);
		} else {
			host_hash_add(h_id_info->values_hash, 
				      hd->host_id.data,
				      hd->host_id.len,
				      hd, MDL);
		}
	}

	if (dynamicp && commit) {
		if (!write_host (hd))
			return ISC_R_IOERROR;
		if (!commit_leases ())
			return ISC_R_IOERROR;
	}

	return ISC_R_SUCCESS;
}


isc_result_t delete_class (cp, commit)
	struct class *cp;
	int commit;
{
	cp->flags |= CLASS_DECL_DELETED;

	/* do the write first as we won't be leaving it in any data
	   structures, unlike the host objects */
	
	if (commit) {
		write_named_billing_class ((unsigned char *)cp->name, 0, cp);
		if (!commit_leases ())
			return ISC_R_IOERROR;
	}
	
	unlink_class(&cp);		/* remove from collections */

	class_dereference(&cp, MDL);

	return ISC_R_SUCCESS;
}


isc_result_t delete_host (hd, commit)
	struct host_decl *hd;
	int commit;
{
	struct host_decl *hp = (struct host_decl *)0;
	struct host_decl *np = (struct host_decl *)0;
	struct host_decl *foo;
	int hw_head = 0, uid_head = 1;

	/* Don't need to do it twice. */
	if (hd -> flags & HOST_DECL_DELETED)
		return ISC_R_SUCCESS;

	/* But we do need to do it once!   :') */
	hd -> flags |= HOST_DECL_DELETED;

	if (hd -> interface.hlen) {
	    if (host_hw_addr_hash) {
		if (host_hash_lookup (&hp, host_hw_addr_hash,
				      hd -> interface.hbuf,
				      hd -> interface.hlen, MDL)) {
		    if (hp == hd) {
			host_hash_delete (host_hw_addr_hash,
					  hd -> interface.hbuf,
					  hd -> interface.hlen, MDL);
			hw_head = 1;
		    } else {
			np = (struct host_decl *)0;
			foo = (struct host_decl *)0;
			host_reference (&foo, hp, MDL);
			while (foo) {
			    if (foo == hd)
				    break;
			    if (np)
				    host_dereference (&np, MDL);
			    host_reference (&np, foo, MDL);
			    host_dereference (&foo, MDL);
			    if (np -> n_ipaddr)
				    host_reference (&foo, np -> n_ipaddr, MDL);
			}

			if (foo) {
			    host_dereference (&np -> n_ipaddr, MDL);
			    if (hd -> n_ipaddr)
				host_reference (&np -> n_ipaddr,
						hd -> n_ipaddr, MDL);
			    host_dereference (&foo, MDL);
			}
			if (np)
				host_dereference (&np, MDL);
		    }
		    host_dereference (&hp, MDL);
		}
	    }
	}

	/* If we got a client identifier, hash this entry by
	   client identifier. */
	if (hd -> client_identifier.len) {
	    if (host_uid_hash) {
		if (host_hash_lookup (&hp, host_uid_hash,
				      hd -> client_identifier.data,
				      hd -> client_identifier.len, MDL)) {
		    if (hp == hd) {
			host_hash_delete (host_uid_hash,
					  hd -> client_identifier.data,
					  hd -> client_identifier.len, MDL);
			uid_head = 1;
		    } else {
			np = (struct host_decl *)0;
			foo = (struct host_decl *)0;
			host_reference (&foo, hp, MDL);
			while (foo) {
			    if (foo == hd)
				    break;
			    if (np)
				host_dereference (&np, MDL);
			    host_reference (&np, foo, MDL);
			    host_dereference (&foo, MDL);
			    if (np -> n_ipaddr)
				    host_reference (&foo, np -> n_ipaddr, MDL);
			}

			if (foo) {
			    host_dereference (&np -> n_ipaddr, MDL);
			    if (hd -> n_ipaddr)
				host_reference (&np -> n_ipaddr,
						hd -> n_ipaddr, MDL);
			    host_dereference (&foo, MDL);
			}
			if (np)
				host_dereference (&np, MDL);
		    }
		    host_dereference (&hp, MDL);
		}
	    }
	}

	if (hd->host_id_option != NULL) {
		option_dereference(&hd->host_id_option, MDL);
		data_string_forget(&hd->host_id, MDL);
	}

	if (hd -> n_ipaddr) {
		if (uid_head && hd -> n_ipaddr -> client_identifier.len) {
			host_hash_add
				(host_uid_hash,
				 hd -> n_ipaddr -> client_identifier.data,
				 hd -> n_ipaddr -> client_identifier.len,
				 hd -> n_ipaddr, MDL);
		}
		if (hw_head && hd -> n_ipaddr -> interface.hlen) {
			host_hash_add (host_hw_addr_hash,
				       hd -> n_ipaddr -> interface.hbuf,
				       hd -> n_ipaddr -> interface.hlen,
				       hd -> n_ipaddr, MDL);
		}
		host_dereference (&hd -> n_ipaddr, MDL);
	}

	if (host_name_hash) {
		if (host_hash_lookup (&hp, host_name_hash,
				      (unsigned char *)hd -> name,
				      strlen (hd -> name), MDL)) {
			if (hp == hd && !(hp -> flags & HOST_DECL_STATIC)) {
				host_hash_delete (host_name_hash,
						  (unsigned char *)hd -> name,
						  strlen (hd -> name), MDL);
			}
			host_dereference (&hp, MDL);
		}
	}

	if (commit) {
		if (!write_host (hd))
			return ISC_R_IOERROR;
		if (!commit_leases ())
			return ISC_R_IOERROR;
	}
	return ISC_R_SUCCESS;
}

int find_hosts_by_haddr (struct host_decl **hp, int htype,
			 const unsigned char *haddr, unsigned hlen,
			 const char *file, int line)
{
	struct hardware h;

	h.hlen = hlen + 1;
	h.hbuf [0] = htype;
	memcpy (&h.hbuf [1], haddr, hlen);

	return host_hash_lookup (hp, host_hw_addr_hash,
				 h.hbuf, h.hlen, file, line);
}

int find_hosts_by_uid (struct host_decl **hp,
		       const unsigned char *data, unsigned len,
		       const char *file, int line)
{
	return host_hash_lookup (hp, host_uid_hash, data, len, file, line);
}

int
find_hosts_by_option(struct host_decl **hp, 
		     struct packet *packet,
		     struct option_state *opt_state,
		     const char *file, int line) {
	host_id_info_t *p;
	struct option_cache *oc;
	struct data_string data;
	int found;
	
	for (p = host_id_info; p != NULL; p = p->next) {
		oc = lookup_option(p->option->universe, 
				   opt_state, p->option->code);
		if (oc != NULL) {
			memset(&data, 0, sizeof(data));
			if (!evaluate_option_cache(&data, packet, NULL, NULL,
						   opt_state, NULL,
						   &global_scope, oc, 
						   MDL)) {
				log_error("Error evaluating option cache");
				return 0;
			}
			
			found = host_hash_lookup(hp, p->values_hash, 
						 data.data, data.len,
						 file, line);

			data_string_forget(&data, MDL);

			if (found) {
				return 1;
			}
		}
	}
	return 0;
}

/* More than one host_decl can be returned by find_hosts_by_haddr or
   find_hosts_by_uid, and each host_decl can have multiple addresses.
   Loop through the list of hosts, and then for each host, through the
   list of addresses, looking for an address that's in the same shared
   network as the one specified.    Store the matching address through
   the addr pointer, update the host pointer to point at the host_decl
   that matched, and return the subnet that matched. */

int find_host_for_network (struct subnet **sp, struct host_decl **host,
			   struct iaddr *addr, struct shared_network *share)
{
	int i;
	struct iaddr ip_address;
	struct host_decl *hp;
	struct data_string fixed_addr;

	memset (&fixed_addr, 0, sizeof fixed_addr);

	for (hp = *host; hp; hp = hp -> n_ipaddr) {
		if (!hp -> fixed_addr)
			continue;
		if (!evaluate_option_cache (&fixed_addr, (struct packet *)0,
					    (struct lease *)0,
					    (struct client_state *)0,
					    (struct option_state *)0,
					    (struct option_state *)0,
					    &global_scope,
					    hp -> fixed_addr, MDL))
			continue;
		for (i = 0; i < fixed_addr.len; i += 4) {
			ip_address.len = 4;
			memcpy (ip_address.iabuf,
				fixed_addr.data + i, 4);
			if (find_grouped_subnet (sp, share, ip_address, MDL)) {
				struct host_decl *tmp = (struct host_decl *)0;
				*addr = ip_address;
				/* This is probably not necessary, but
				   just in case *host is the only reference
				   to that host declaration, make a temporary
				   reference so that dereferencing it doesn't
				   dereference hp out from under us. */
				host_reference (&tmp, *host, MDL);
				host_dereference (host, MDL);
				host_reference (host, hp, MDL);
				host_dereference (&tmp, MDL);
				data_string_forget (&fixed_addr, MDL);
				return 1;
			}
		}
		data_string_forget (&fixed_addr, MDL);
	}
	return 0;
}

void new_address_range (cfile, low, high, subnet, pool, lpchain)
	struct parse *cfile;
	struct iaddr low, high;
	struct subnet *subnet;
	struct pool *pool;
	struct lease **lpchain;
{
#if defined(COMPACT_LEASES)
	struct lease *address_range;
#endif
	unsigned min, max, i;
	char lowbuf [16], highbuf [16], netbuf [16];
	struct shared_network *share = subnet -> shared_network;
	struct lease *lt = (struct lease *)0;
#if !defined(COMPACT_LEASES)
	isc_result_t status;
#endif

	/* All subnets should have attached shared network structures. */
	if (!share) {
		strcpy (netbuf, piaddr (subnet -> net));
		log_fatal ("No shared network for network %s (%s)",
		       netbuf, piaddr (subnet -> netmask));
	}

	/* Initialize the hash table if it hasn't been done yet. */
	if (!lease_uid_hash) {
		if (!lease_id_new_hash(&lease_uid_hash, LEASE_HASH_SIZE, MDL))
			log_fatal ("Can't allocate lease/uid hash");
	}
	if (!lease_ip_addr_hash) {
		if (!lease_ip_new_hash(&lease_ip_addr_hash, LEASE_HASH_SIZE,
				       MDL))
			log_fatal ("Can't allocate lease/ip hash");
	}
	if (!lease_hw_addr_hash) {
		if (!lease_id_new_hash(&lease_hw_addr_hash, LEASE_HASH_SIZE,
				       MDL))
			log_fatal ("Can't allocate lease/hw hash");
	}

	/* Make sure that high and low addresses are in this subnet. */
	if (!addr_eq(subnet->net, subnet_number(low, subnet->netmask))) {
		strcpy(lowbuf, piaddr(low));
		strcpy(netbuf, piaddr(subnet->net));
		log_fatal("bad range, address %s not in subnet %s netmask %s",
			  lowbuf, netbuf, piaddr(subnet->netmask));
	}

	if (!addr_eq(subnet->net, subnet_number(high, subnet->netmask))) {
		strcpy(highbuf, piaddr(high));
		strcpy(netbuf, piaddr(subnet->net));
		log_fatal("bad range, address %s not in subnet %s netmask %s",
			  highbuf, netbuf, piaddr(subnet->netmask));
	}

	/* Get the high and low host addresses... */
	max = host_addr (high, subnet -> netmask);
	min = host_addr (low, subnet -> netmask);

	/* Allow range to be specified high-to-low as well as low-to-high. */
	if (min > max) {
		max = min;
		min = host_addr (high, subnet -> netmask);
	}

	/* Get a lease structure for each address in the range. */
#if defined (COMPACT_LEASES)
	address_range = new_leases (max - min + 1, MDL);
	if (!address_range) {
		strcpy (lowbuf, piaddr (low));
		strcpy (highbuf, piaddr (high));
		log_fatal ("No memory for address range %s-%s.",
			   lowbuf, highbuf);
	}
#endif

	/* Fill out the lease structures with some minimal information. */
	for (i = 0; i < max - min + 1; i++) {
		struct lease *lp = (struct lease *)0;
#if defined (COMPACT_LEASES)
		omapi_object_initialize ((omapi_object_t *)&address_range [i],
					 dhcp_type_lease,
					 0, sizeof (struct lease), MDL);
		lease_reference (&lp, &address_range [i], MDL);
#else
		status = lease_allocate (&lp, MDL);
		if (status != ISC_R_SUCCESS)
			log_fatal ("No memory for lease %s: %s",
				   piaddr (ip_addr (subnet -> net,
						    subnet -> netmask,
						    i + min)),
				   isc_result_totext (status));
#endif
		lp -> ip_addr = ip_addr (subnet -> net,
					 subnet -> netmask, i + min);
		lp -> starts = MIN_TIME;
		lp -> ends = MIN_TIME;
		subnet_reference (&lp -> subnet, subnet, MDL);
		pool_reference (&lp -> pool, pool, MDL);
		lp -> binding_state = FTS_FREE;
		lp -> next_binding_state = FTS_FREE;
		lp -> flags = 0;

		/* Remember the lease in the IP address hash. */
		if (find_lease_by_ip_addr (&lt, lp -> ip_addr, MDL)) {
			if (lt -> pool) {
				parse_warn (cfile,
					    "lease %s is declared twice!",
					    piaddr (lp -> ip_addr));
			} else
				pool_reference (&lt -> pool, pool, MDL);
			lease_dereference (&lt, MDL);
		} else
			lease_ip_hash_add(lease_ip_addr_hash,
					  lp->ip_addr.iabuf, lp->ip_addr.len,
					  lp, MDL);
		/* Put the lease on the chain for the caller. */
		if (lpchain) {
			if (*lpchain) {
				lease_reference (&lp -> next, *lpchain, MDL);
				lease_dereference (lpchain, MDL);
			}
			lease_reference (lpchain, lp, MDL);
		}
		lease_dereference (&lp, MDL);
	}
}

int find_subnet (struct subnet **sp,
		 struct iaddr addr, const char *file, int line)
{
	struct subnet *rv;

	for (rv = subnets; rv; rv = rv -> next_subnet) {
		if (addr_eq (subnet_number (addr, rv -> netmask), rv -> net)) {
			if (subnet_reference (sp, rv,
					      file, line) != ISC_R_SUCCESS)
				return 0;
			return 1;
		}
	}
	return 0;
}

int find_grouped_subnet (struct subnet **sp,
			 struct shared_network *share, struct iaddr addr,
			 const char *file, int line)
{
	struct subnet *rv;

	for (rv = share -> subnets; rv; rv = rv -> next_sibling) {
		if (addr_eq (subnet_number (addr, rv -> netmask), rv -> net)) {
			if (subnet_reference (sp, rv,
					      file, line) != ISC_R_SUCCESS)
				return 0;
			return 1;
		}
	}
	return 0;
}

/* XXX: could speed up if everyone had a prefix length */
int 
subnet_inner_than(const struct subnet *subnet, 
		  const struct subnet *scan,
		  int warnp) {
	if (addr_eq(subnet_number(subnet->net, scan->netmask), scan->net) ||
	    addr_eq(subnet_number(scan->net, subnet->netmask), subnet->net)) {
		char n1buf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255")];
		int i, j;
		for (i = 0; i < 128; i++)
			if (subnet->netmask.iabuf[3 - (i >> 3)]
			    & (1 << (i & 7)))
				break;
		for (j = 0; j < 128; j++)
			if (scan->netmask.iabuf[3 - (j >> 3)] &
			    (1 << (j & 7)))
				break;
		if (warnp) {
			strcpy(n1buf, piaddr(subnet->net));
			log_error("Warning: subnet %s/%d overlaps subnet %s/%d",
			      n1buf, 32 - i,
			      piaddr(scan->net), 32 - j);
		}
		if (i < j)
			return 1;
	}
	return 0;
}

/* Enter a new subnet into the subnet list. */
void enter_subnet (subnet)
	struct subnet *subnet;
{
	struct subnet *scan = (struct subnet *)0;
	struct subnet *next = (struct subnet *)0;
	struct subnet *prev = (struct subnet *)0;

	/* Check for duplicates... */
	if (subnets)
	    subnet_reference (&next, subnets, MDL);
	while (next) {
	    subnet_reference (&scan, next, MDL);
	    subnet_dereference (&next, MDL);

	    /* When we find a conflict, make sure that the
	       subnet with the narrowest subnet mask comes
	       first. */
	    if (subnet_inner_than (subnet, scan, 1)) {
		if (prev) {
		    if (prev -> next_subnet)
			subnet_dereference (&prev -> next_subnet, MDL);
		    subnet_reference (&prev -> next_subnet, subnet, MDL);
		    subnet_dereference (&prev, MDL);
		} else {
		    subnet_dereference (&subnets, MDL);
		    subnet_reference (&subnets, subnet, MDL);
		}
		subnet_reference (&subnet -> next_subnet, scan, MDL);
		subnet_dereference (&scan, MDL);
		return;
	    }
	    subnet_reference (&prev, scan, MDL);
	    subnet_dereference (&scan, MDL);
	}
	if (prev)
		subnet_dereference (&prev, MDL);

	/* XXX use the BSD radix tree code instead of a linked list. */
	if (subnets) {
		subnet_reference (&subnet -> next_subnet, subnets, MDL);
		subnet_dereference (&subnets, MDL);
	}
	subnet_reference (&subnets, subnet, MDL);
}
	
/* Enter a new shared network into the shared network list. */

void enter_shared_network (share)
	struct shared_network *share;
{
	if (shared_networks) {
		shared_network_reference (&share -> next,
					  shared_networks, MDL);
		shared_network_dereference (&shared_networks, MDL);
	}
	shared_network_reference (&shared_networks, share, MDL);
}
	
void new_shared_network_interface (cfile, share, name)
	struct parse *cfile;
	struct shared_network *share;
	const char *name;
{
	struct interface_info *ip;
	isc_result_t status;

	if (share -> interface) {
		parse_warn (cfile, 
			    "A subnet or shared network can't be connected %s",
			    "to two interfaces.");
		return;
	}
	
	for (ip = interfaces; ip; ip = ip -> next)
		if (!strcmp (ip -> name, name))
			break;
	if (!ip) {
		status = interface_allocate (&ip, MDL);
		if (status != ISC_R_SUCCESS)
			log_fatal ("new_shared_network_interface %s: %s",
				   name, isc_result_totext (status));
		if (strlen (name) > sizeof ip -> name) {
			memcpy (ip -> name, name, (sizeof ip -> name) - 1);
			ip -> name [(sizeof ip -> name) - 1] = 0;
		} else
			strcpy (ip -> name, name);
		if (interfaces) {
			interface_reference (&ip -> next, interfaces, MDL);
			interface_dereference (&interfaces, MDL);
		}
		interface_reference (&interfaces, ip, MDL);
		ip -> flags = INTERFACE_REQUESTED;
		/* XXX this is a reference loop. */
		shared_network_reference (&ip -> shared_network, share, MDL);
		interface_reference (&share -> interface, ip, MDL);
	}
}

/* Enter a lease into the system.   This is called by the parser each
   time it reads in a new lease.   If the subnet for that lease has
   already been read in (usually the case), just update that lease;
   otherwise, allocate temporary storage for the lease and keep it around
   until we're done reading in the config file. */

void enter_lease (lease)
	struct lease *lease;
{
	struct lease *comp = (struct lease *)0;

	if (find_lease_by_ip_addr (&comp, lease -> ip_addr, MDL)) {
		if (!comp -> pool) {
			log_error ("undeclared lease found in database: %s",
				   piaddr (lease -> ip_addr));
		} else
			pool_reference (&lease -> pool, comp -> pool, MDL);

		if (comp -> subnet)
			subnet_reference (&lease -> subnet,
					  comp -> subnet, MDL);
		lease_ip_hash_delete(lease_ip_addr_hash,
				     lease->ip_addr.iabuf, lease->ip_addr.len,
				     MDL);
		lease_dereference (&comp, MDL);
	}

	/* The only way a lease can get here without a subnet is if it's in
	   the lease file, but not in the dhcpd.conf file.  In this case, we
	   *should* keep it around until it's expired, but never reallocate it
	   or renew it.  Currently, to maintain consistency, we are not doing
	   this.
	   XXX fix this so that the lease is kept around until it expires.
	   XXX this will be important in IPv6 with addresses that become
	   XXX non-renewable as a result of a renumbering event. */

	if (!lease -> subnet) {
		log_error ("lease %s: no subnet.", piaddr (lease -> ip_addr));
		return;
	}
	lease_ip_hash_add(lease_ip_addr_hash, lease->ip_addr.iabuf,
			  lease->ip_addr.len, lease, MDL);
}

/* Replace the data in an existing lease with the data in a new lease;
   adjust hash tables to suit, and insertion sort the lease into the
   list of leases by expiry time so that we can always find the oldest
   lease. */

int supersede_lease (comp, lease, commit, propogate, pimmediate)
	struct lease *comp, *lease;
	int commit;
	int propogate;
	int pimmediate;
{
	struct lease *lp, **lq, *prev;
	struct timeval tv;
#if defined (FAILOVER_PROTOCOL)
	int do_pool_check = 0;

	/* We must commit leases before sending updates regarding them
	   to failover peers.  It is, therefore, an error to set pimmediate
	   and not commit. */
	if (pimmediate && !commit)
		return 0;
#endif

	/* If there is no sample lease, just do the move. */
	if (!lease)
		goto just_move_it;

	/* Static leases are not currently kept in the database... */
	if (lease -> flags & STATIC_LEASE)
		return 1;

	/* If the existing lease hasn't expired and has a different
	   unique identifier or, if it doesn't have a unique
	   identifier, a different hardware address, then the two
	   leases are in conflict.  If the existing lease has a uid
	   and the new one doesn't, but they both have the same
	   hardware address, and dynamic bootp is allowed on this
	   lease, then we allow that, in case a dynamic BOOTP lease is
	   requested *after* a DHCP lease has been assigned. */

	if (lease -> binding_state != FTS_ABANDONED &&
	    lease -> next_binding_state != FTS_ABANDONED &&
	    comp -> binding_state == FTS_ACTIVE &&
	    (((comp -> uid && lease -> uid) &&
	      (comp -> uid_len != lease -> uid_len ||
	       memcmp (comp -> uid, lease -> uid, comp -> uid_len))) ||
	     (!comp -> uid &&
	      ((comp -> hardware_addr.hlen !=
		lease -> hardware_addr.hlen) ||
	       memcmp (comp -> hardware_addr.hbuf,
		       lease -> hardware_addr.hbuf,
		       comp -> hardware_addr.hlen))))) {
		log_error ("Lease conflict at %s",
		      piaddr (comp -> ip_addr));
	}

	/* If there's a Unique ID, dissociate it from the hash
	   table and free it if necessary. */
	if (comp->uid) {
		uid_hash_delete(comp);
		if (comp->uid != comp->uid_buf) {
			dfree(comp->uid, MDL);
			comp->uid_max = 0;
			comp->uid_len = 0;
		}
		comp -> uid = (unsigned char *)0;
	}

	/* If there's a hardware address, remove the lease from its
	 * old position in the hash bucket's ordered list.
	 */
	if (comp->hardware_addr.hlen)
		hw_hash_delete(comp);

	/* If the lease has been billed to a class, remove the billing. */
	if (comp -> billing_class != lease -> billing_class) {
		if (comp -> billing_class)
			unbill_class (comp, comp -> billing_class);
		if (lease -> billing_class)
			bill_class (comp, lease -> billing_class);
	}

	/* Copy the data files, but not the linkages. */
	comp -> starts = lease -> starts;
	if (lease -> uid) {
		if (lease -> uid_len <= sizeof (lease -> uid_buf)) {
			memcpy (comp -> uid_buf,
				lease -> uid, lease -> uid_len);
			comp -> uid = &comp -> uid_buf [0];
			comp -> uid_max = sizeof comp -> uid_buf;
			comp -> uid_len = lease -> uid_len;
		} else if (lease -> uid != &lease -> uid_buf [0]) {
			comp -> uid = lease -> uid;
			comp -> uid_max = lease -> uid_max;
			lease -> uid = (unsigned char *)0;
			lease -> uid_max = 0;
			comp -> uid_len = lease -> uid_len;
			lease -> uid_len = 0;
		} else {
			log_fatal ("corrupt lease uid."); /* XXX */
		}
	} else {
		comp -> uid = (unsigned char *)0;
		comp -> uid_len = comp -> uid_max = 0;
	}
	if (comp -> host)
		host_dereference (&comp -> host, MDL);
	host_reference (&comp -> host, lease -> host, MDL);
	comp -> hardware_addr = lease -> hardware_addr;
	comp -> flags = ((lease -> flags & ~PERSISTENT_FLAGS) |
			 (comp -> flags & ~EPHEMERAL_FLAGS));
	if (comp -> scope)
		binding_scope_dereference (&comp -> scope, MDL);
	if (lease -> scope) {
		binding_scope_reference (&comp -> scope, lease -> scope, MDL);
		binding_scope_dereference (&lease -> scope, MDL);
	}

	if (comp -> agent_options)
		option_chain_head_dereference (&comp -> agent_options, MDL);
	if (lease -> agent_options) {
		/* Only retain the agent options if the lease is still
		   affirmatively associated with a client. */
		if (lease -> next_binding_state == FTS_ACTIVE ||
		    lease -> next_binding_state == FTS_EXPIRED)
			option_chain_head_reference (&comp -> agent_options,
						     lease -> agent_options,
						     MDL);
		option_chain_head_dereference (&lease -> agent_options, MDL);
	}

	/* Record the hostname information in the lease. */
	if (comp -> client_hostname)
		dfree (comp -> client_hostname, MDL);
	comp -> client_hostname = lease -> client_hostname;
	lease -> client_hostname = (char *)0;

	if (lease -> on_expiry) {
		if (comp -> on_expiry)
			executable_statement_dereference (&comp -> on_expiry,
							  MDL);
		executable_statement_reference (&comp -> on_expiry,
						lease -> on_expiry,
						MDL);
	}
	if (lease -> on_commit) {
		if (comp -> on_commit)
			executable_statement_dereference (&comp -> on_commit,
							  MDL);
		executable_statement_reference (&comp -> on_commit,
						lease -> on_commit,
						MDL);
	}
	if (lease -> on_release) {
		if (comp -> on_release)
			executable_statement_dereference (&comp -> on_release,
							  MDL);
		executable_statement_reference (&comp -> on_release,
						lease -> on_release, MDL);
	}

	/* Record the lease in the uid hash if necessary. */
	if (comp->uid)
		uid_hash_add(comp);

	/* Record it in the hardware address hash if necessary. */
	if (comp->hardware_addr.hlen)
		hw_hash_add(comp);

	comp->cltt = lease->cltt;
#if defined (FAILOVER_PROTOCOL)
	comp->tstp = lease->tstp;
	comp->tsfp = lease->tsfp;
	comp->atsfp = lease->atsfp;
#endif /* FAILOVER_PROTOCOL */
	comp->ends = lease->ends;
	comp->next_binding_state = lease->next_binding_state;

      just_move_it:
#if defined (FAILOVER_PROTOCOL)
	/* Atsfp should be cleared upon any state change that implies
	 * propagation whether supersede_lease was given a copy lease
	 * structure or not (often from the pool_timer()).
	 */
	if (propogate)
		comp->atsfp = 0;
#endif /* FAILOVER_PROTOCOL */

	if (!comp -> pool) {
		log_error ("Supersede_lease: lease %s with no pool.",
			   piaddr (comp -> ip_addr));
		return 0;
	}

	/* Figure out which queue it's on. */
	switch (comp -> binding_state) {
	      case FTS_FREE:
		if (comp->flags & RESERVED_LEASE)
			lq = &comp->pool->reserved;
		else {
			lq = &comp->pool->free;
			comp->pool->free_leases--;
		}

#if defined(FAILOVER_PROTOCOL)
		do_pool_check = 1;
#endif
		break;

	      case FTS_ACTIVE:
		lq = &comp -> pool -> active;
		break;

	      case FTS_EXPIRED:
	      case FTS_RELEASED:
	      case FTS_RESET:
		lq = &comp -> pool -> expired;
		break;

	      case FTS_ABANDONED:
		lq = &comp -> pool -> abandoned;
		break;

	      case FTS_BACKUP:
		if (comp->flags & RESERVED_LEASE)
			lq = &comp->pool->reserved;
		else {
			lq = &comp->pool->backup;
			comp->pool->backup_leases--;
		}

#if defined(FAILOVER_PROTOCOL)
		do_pool_check = 1;
#endif
		break;

	      default:
		log_error ("Lease with bogus binding state: %d",
			   comp -> binding_state);
#if defined (BINDING_STATE_DEBUG)
		abort ();
#endif
		return 0;
	}

	/* Remove the lease from its current place in its current
	   timer sequence. */
	/* XXX this is horrid. */
	prev = (struct lease *)0;
	for (lp = *lq; lp; lp = lp -> next) {
		if (lp == comp)
			break;
		prev = lp;
	}

	if (!lp) {
		log_fatal("Lease with binding state %s not on its queue.",
			  (comp->binding_state < 1 ||
			   comp->binding_state > FTS_LAST)
			  ? "unknown"
			  : binding_state_names[comp->binding_state - 1]);
	}

	if (prev) {
		lease_dereference (&prev -> next, MDL);
		if (comp -> next) {
			lease_reference (&prev -> next, comp -> next, MDL);
			lease_dereference (&comp -> next, MDL);
		}
	} else {
		lease_dereference (lq, MDL);
		if (comp -> next) {
			lease_reference (lq, comp -> next, MDL);
			lease_dereference (&comp -> next, MDL);
		}
	}

	/* Make the state transition. */
	if (commit || !pimmediate)
		make_binding_state_transition (comp);

	/* Put the lease back on the appropriate queue.    If the lease
	   is corrupt (as detected by lease_enqueue), don't go any farther. */
	if (!lease_enqueue (comp))
		return 0;

	/* If this is the next lease that will timeout on the pool,
	   zap the old timeout and set the timeout on this pool to the
	   time that the lease's next event will happen.
		   
	   We do not actually set the timeout unless commit is true -
	   we don't want to thrash the timer queue when reading the
	   lease database.  Instead, the database code calls the
	   expiry event on each pool after reading in the lease file,
	   and the expiry code sets the timer if there's anything left
	   to expire after it's run any outstanding expiry events on
	   the pool. */
	if ((commit || !pimmediate) &&
	    comp -> sort_time != MIN_TIME &&
	    comp -> sort_time > cur_time &&
	    (comp -> sort_time < comp -> pool -> next_event_time ||
	     comp -> pool -> next_event_time == MIN_TIME)) {
		comp -> pool -> next_event_time = comp -> sort_time;
		tv . tv_sec = comp -> pool -> next_event_time;
		tv . tv_usec = 0;
		add_timeout (&tv,
			     pool_timer, comp -> pool,
			     (tvref_t)pool_reference,
			     (tvunref_t)pool_dereference);
	}

	if (commit) {
		if (!write_lease (comp))
			return 0;
		if ((server_starting & SS_NOSYNC) == 0) {
			if (!commit_leases ())
				return 0;
		}
	}

#if defined (FAILOVER_PROTOCOL)
	if (propogate) {
		comp -> desired_binding_state = comp -> binding_state;
		if (!dhcp_failover_queue_update (comp, pimmediate))
			return 0;
	}
	if (do_pool_check && comp->pool->failover_peer)
		dhcp_failover_pool_check(comp->pool);
#endif

	/* If the current binding state has already expired, do an
	   expiry event right now. */
	/* XXX At some point we should optimize this so that we don't
	   XXX write the lease twice, but this is a safe way to fix the
	   XXX problem for 3.0 (I hope!). */
	if ((commit || !pimmediate) &&
	    comp -> sort_time < cur_time &&
	    comp -> next_binding_state != comp -> binding_state)
		pool_timer (comp -> pool);

	return 1;
}

void make_binding_state_transition (struct lease *lease)
{
#if defined (FAILOVER_PROTOCOL)
	dhcp_failover_state_t *peer;

	if (lease && lease -> pool && lease -> pool -> failover_peer)
		peer = lease -> pool -> failover_peer;
	else
		peer = (dhcp_failover_state_t *)0;
#endif

	/* If the lease was active and is now no longer active, but isn't
	   released, then it just expired, so do the expiry event. */
	if (lease -> next_binding_state != lease -> binding_state &&
	    ((
#if defined (FAILOVER_PROTOCOL)
		    peer &&
		    (lease -> binding_state == FTS_EXPIRED ||
		     (peer -> i_am == secondary &&
		      lease -> binding_state == FTS_ACTIVE)) &&
		    (lease -> next_binding_state == FTS_FREE ||
		     lease -> next_binding_state == FTS_BACKUP)) ||
	     (!peer &&
#endif
	      lease -> binding_state == FTS_ACTIVE &&
	      lease -> next_binding_state != FTS_RELEASED))) {
#if defined (NSUPDATE)
		ddns_removals(lease, NULL);
#endif
		if (lease -> on_expiry) {
			execute_statements ((struct binding_value **)0,
					    (struct packet *)0, lease,
					    (struct client_state *)0,
					    (struct option_state *)0,
					    (struct option_state *)0, /* XXX */
					    &lease -> scope,
					    lease -> on_expiry);
			if (lease -> on_expiry)
				executable_statement_dereference
					(&lease -> on_expiry, MDL);
		}
		
		/* No sense releasing a lease after it's expired. */
		if (lease -> on_release)
			executable_statement_dereference (&lease -> on_release,
							  MDL);
		/* Get rid of client-specific bindings that are only
		   correct when the lease is active. */
		if (lease -> billing_class)
			unbill_class (lease, lease -> billing_class);
		if (lease -> agent_options)
			option_chain_head_dereference (&lease -> agent_options,
						       MDL);
		if (lease -> client_hostname) {
			dfree (lease -> client_hostname, MDL);
			lease -> client_hostname = (char *)0;
		}
		if (lease -> host)
			host_dereference (&lease -> host, MDL);

		/* Send the expiry time to the peer. */
		lease -> tstp = lease -> ends;
	}

	/* If the lease was active and is now released, do the release
	   event. */
	if (lease -> next_binding_state != lease -> binding_state &&
	    ((
#if defined (FAILOVER_PROTOCOL)
		    peer &&
		    lease -> binding_state == FTS_RELEASED &&
		    (lease -> next_binding_state == FTS_FREE ||
		     lease -> next_binding_state == FTS_BACKUP)) ||
	     (!peer &&
#endif
	      lease -> binding_state == FTS_ACTIVE &&
	      lease -> next_binding_state == FTS_RELEASED))) {
#if defined (NSUPDATE)
		/*
		 * Note: ddns_removals() is also iterated when the lease
		 * enters state 'released' in 'release_lease()'.  The below
		 * is caught when a peer receives a BNDUPD from a failover
		 * peer; it may not have received the client's release (it
		 * may have been offline).
		 *
		 * We could remove the call from release_lease() because
		 * it will also catch here on the originating server after the
		 * peer acknowledges the state change.  However, there could
		 * be many hours inbetween, and in this case we /know/ the
		 * client is no longer using the lease when we receive the
		 * release message.  This is not true of expiry, where the
		 * peer may have extended the lease.
		 */
		ddns_removals(lease, NULL);
#endif
		if (lease -> on_release) {
			execute_statements ((struct binding_value **)0,
					    (struct packet *)0, lease,
					    (struct client_state *)0,
					    (struct option_state *)0,
					    (struct option_state *)0, /* XXX */
					    &lease -> scope,
					    lease -> on_release);
			executable_statement_dereference (&lease -> on_release,
							  MDL);
		}
		
		/* A released lease can't expire. */
		if (lease -> on_expiry)
			executable_statement_dereference (&lease -> on_expiry,
							  MDL);

		/* Get rid of client-specific bindings that are only
		   correct when the lease is active. */
		if (lease -> billing_class)
			unbill_class (lease, lease -> billing_class);
		if (lease -> agent_options)
			option_chain_head_dereference (&lease -> agent_options,
						       MDL);
		if (lease -> client_hostname) {
			dfree (lease -> client_hostname, MDL);
			lease -> client_hostname = (char *)0;
		}
		if (lease -> host)
			host_dereference (&lease -> host, MDL);

		/* Send the release time (should be == cur_time) to the
		   peer. */
		lease -> tstp = lease -> ends;
	}

#if defined (DEBUG_LEASE_STATE_TRANSITIONS)
	log_debug ("lease %s moves from %s to %s",
		   piaddr (lease -> ip_addr),
		   binding_state_print (lease -> binding_state),
		   binding_state_print (lease -> next_binding_state));
#endif

	lease -> binding_state = lease -> next_binding_state;
	switch (lease -> binding_state) {
	      case FTS_ACTIVE:
#if defined (FAILOVER_PROTOCOL)
		if (lease -> pool && lease -> pool -> failover_peer)
			lease -> next_binding_state = FTS_EXPIRED;
		else
#endif
			lease -> next_binding_state = FTS_FREE;
		break;

	      case FTS_EXPIRED:
	      case FTS_RELEASED:
	      case FTS_ABANDONED:
	      case FTS_RESET:
		lease -> next_binding_state = FTS_FREE;
#if defined(FAILOVER_PROTOCOL)
		/* If we are not in partner_down, leases don't go from
		   EXPIRED to FREE on a timeout - only on an update.
		   If we're in partner_down, they expire at mclt past
		   the time we entered partner_down. */
		if(lease -> pool){
			if (lease -> pool -> failover_peer &&
			    lease -> pool -> failover_peer -> me.state == partner_down)
				lease -> tsfp =
				    (lease -> pool -> failover_peer -> me.stos +
				     lease -> pool -> failover_peer -> mclt);
		}
#endif /* FAILOVER_PROTOCOL */
		break;

	      case FTS_FREE:
	      case FTS_BACKUP:
		lease -> next_binding_state = lease -> binding_state;
		break;
	}
#if defined (DEBUG_LEASE_STATE_TRANSITIONS)
	log_debug ("lease %s: next binding state %s",
		   piaddr (lease -> ip_addr),
		   binding_state_print (lease -> next_binding_state));
#endif

}

/* Copy the contents of one lease into another, correctly maintaining
   reference counts. */
int lease_copy (struct lease **lp,
		struct lease *lease, const char *file, int line)
{
	struct lease *lt = (struct lease *)0;
	isc_result_t status;

	status = lease_allocate (&lt, MDL);
	if (status != ISC_R_SUCCESS)
		return 0;

	lt -> ip_addr = lease -> ip_addr;
	lt -> starts = lease -> starts;
	lt -> ends = lease -> ends;
	lt -> uid_len = lease -> uid_len;
	lt -> uid_max = lease -> uid_max;
	if (lease -> uid == lease -> uid_buf) {
		lt -> uid = lt -> uid_buf;
		memcpy (lt -> uid_buf, lease -> uid_buf, sizeof lt -> uid_buf);
	} else if (!lease -> uid_max) {
		lt -> uid = (unsigned char *)0;
	} else {
		lt -> uid = dmalloc (lt -> uid_max, MDL);
		if (!lt -> uid) {
			lease_dereference (&lt, MDL);
			return 0;
		}
		memcpy (lt -> uid, lease -> uid, lease -> uid_max);
	}
	if (lease -> client_hostname) {
		lt -> client_hostname =
			dmalloc (strlen (lease -> client_hostname) + 1, MDL);
		if (!lt -> client_hostname) {
			lease_dereference (&lt, MDL);
			return 0;
		}
		strcpy (lt -> client_hostname, lease -> client_hostname);
	}
	if (lease -> scope)
		binding_scope_reference (&lt -> scope, lease -> scope, MDL);
	if (lease -> agent_options)
		option_chain_head_reference (&lt -> agent_options,
					     lease -> agent_options, MDL);
	host_reference (&lt -> host, lease -> host, file, line);
	subnet_reference (&lt -> subnet, lease -> subnet, file, line);
	pool_reference (&lt -> pool, lease -> pool, file, line);
	class_reference (&lt -> billing_class,
			 lease -> billing_class, file, line);
	lt -> hardware_addr = lease -> hardware_addr;
	if (lease -> on_expiry)
		executable_statement_reference (&lt -> on_expiry,
						lease -> on_expiry,
						file, line);
	if (lease -> on_commit)
		executable_statement_reference (&lt -> on_commit,
						lease -> on_commit,
						file, line);
	if (lease -> on_release)
		executable_statement_reference (&lt -> on_release,
						lease -> on_release,
						file, line);
	lt->flags = lease->flags;
	lt->tstp = lease->tstp;
	lt->tsfp = lease->tsfp;
	lt->atsfp = lease->atsfp;
	lt->cltt = lease -> cltt;
	lt->binding_state = lease->binding_state;
	lt->next_binding_state = lease->next_binding_state;
	status = lease_reference(lp, lt, file, line);
	lease_dereference(&lt, MDL);
	return status == ISC_R_SUCCESS;
}

/* Release the specified lease and re-hash it as appropriate. */
void release_lease (lease, packet)
	struct lease *lease;
	struct packet *packet;
{
	/* If there are statements to execute when the lease is
	   released, execute them. */
#if defined (NSUPDATE)
	ddns_removals(lease, NULL);
#endif
	if (lease -> on_release) {
		execute_statements ((struct binding_value **)0,
				    packet, lease, (struct client_state *)0,
				    packet -> options,
				    (struct option_state *)0, /* XXX */
				    &lease -> scope, lease -> on_release);
		if (lease -> on_release)
			executable_statement_dereference (&lease -> on_release,
							  MDL);
	}

	/* We do either the on_release or the on_expiry events, but
	   not both (it's possible that they could be the same,
	   in any case). */
	if (lease -> on_expiry)
		executable_statement_dereference (&lease -> on_expiry, MDL);

	if (lease -> binding_state != FTS_FREE &&
	    lease -> binding_state != FTS_BACKUP &&
	    lease -> binding_state != FTS_RELEASED &&
	    lease -> binding_state != FTS_EXPIRED &&
	    lease -> binding_state != FTS_RESET) {
		if (lease -> on_commit)
			executable_statement_dereference (&lease -> on_commit,
							  MDL);

		/* Blow away any bindings. */
		if (lease -> scope)
			binding_scope_dereference (&lease -> scope, MDL);

		/* Set sort times to the present. */
		lease -> ends = cur_time;
		/* Lower layers of muckery set tstp to ->ends.  But we send
		 * protocol messages before this.  So it is best to set
		 * tstp now anyway.
		 */
		lease->tstp = cur_time;
#if defined (FAILOVER_PROTOCOL)
		if (lease -> pool && lease -> pool -> failover_peer) {
			lease -> next_binding_state = FTS_RELEASED;
		} else {
			lease -> next_binding_state = FTS_FREE;
		}
#else
		lease -> next_binding_state = FTS_FREE;
#endif
		supersede_lease (lease, (struct lease *)0, 1, 1, 1);
	}
}

/* Abandon the specified lease (set its timeout to infinity and its
   particulars to zero, and re-hash it as appropriate. */

void abandon_lease (lease, message)
	struct lease *lease;
	const char *message;
{
	struct lease *lt = (struct lease *)0;
#if defined (NSUPDATE)
	ddns_removals(lease, NULL);
#endif

	if (!lease_copy (&lt, lease, MDL))
		return;

	if (lt->scope)
		binding_scope_dereference(&lt->scope, MDL);

	lt -> ends = cur_time; /* XXX */
	lt -> next_binding_state = FTS_ABANDONED;

	log_error ("Abandoning IP address %s: %s",
	      piaddr (lease -> ip_addr), message);
	lt -> hardware_addr.hlen = 0;
	if (lt -> uid && lt -> uid != lt -> uid_buf)
		dfree (lt -> uid, MDL);
	lt -> uid = (unsigned char *)0;
	lt -> uid_len = 0;
	lt -> uid_max = 0;
	supersede_lease (lease, lt, 1, 1, 1);
	lease_dereference (&lt, MDL);
}

/* Abandon the specified lease (set its timeout to infinity and its
   particulars to zero, and re-hash it as appropriate. */

void dissociate_lease (lease)
	struct lease *lease;
{
	struct lease *lt = (struct lease *)0;
#if defined (NSUPDATE)
	ddns_removals(lease, NULL);
#endif

	if (!lease_copy (&lt, lease, MDL))
		return;

#if defined (FAILOVER_PROTOCOL)
	if (lease -> pool && lease -> pool -> failover_peer) {
		lt -> next_binding_state = FTS_RESET;
	} else {
		lt -> next_binding_state = FTS_FREE;
	}
#else
	lt -> next_binding_state = FTS_FREE;
#endif
	lt -> ends = cur_time; /* XXX */
	lt -> hardware_addr.hlen = 0;
	if (lt -> uid && lt -> uid != lt -> uid_buf)
		dfree (lt -> uid, MDL);
	lt -> uid = (unsigned char *)0;
	lt -> uid_len = 0;
	lt -> uid_max = 0;
	supersede_lease (lease, lt, 1, 1, 1);
	lease_dereference (&lt, MDL);
}

/* Timer called when a lease in a particular pool expires. */
void pool_timer (vpool)
	void *vpool;
{
	struct pool *pool;
	struct lease *next = (struct lease *)0;
	struct lease *lease = (struct lease *)0;
#define FREE_LEASES 0
#define ACTIVE_LEASES 1
#define EXPIRED_LEASES 2
#define ABANDONED_LEASES 3
#define BACKUP_LEASES 4
#define RESERVED_LEASES 5
	struct lease **lptr[RESERVED_LEASES+1];
	TIME next_expiry = MAX_TIME;
	int i;
	struct timeval tv;

	pool = (struct pool *)vpool;

	lptr [FREE_LEASES] = &pool -> free;
	lptr [ACTIVE_LEASES] = &pool -> active;
	lptr [EXPIRED_LEASES] = &pool -> expired;
	lptr [ABANDONED_LEASES] = &pool -> abandoned;
	lptr [BACKUP_LEASES] = &pool -> backup;
	lptr[RESERVED_LEASES] = &pool->reserved;

	for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		/* If there's nothing on the queue, skip it. */
		if (!*(lptr [i]))
			continue;

#if defined (FAILOVER_PROTOCOL)
		if (pool -> failover_peer &&
		    pool -> failover_peer -> me.state != partner_down) {
			/* The secondary can't remove a lease from the
			   active state except in partner_down. */
			if (i == ACTIVE_LEASES &&
			    pool -> failover_peer -> i_am == secondary)
				continue;
			/* Leases in an expired state don't move to
			   free because of a timeout unless we're in
			   partner_down. */
			if (i == EXPIRED_LEASES)
				continue;
		}
#endif		
		lease_reference (&lease, *(lptr [i]), MDL);

		while (lease) {
			/* Remember the next lease in the list. */
			if (next)
				lease_dereference (&next, MDL);
			if (lease -> next)
				lease_reference (&next, lease -> next, MDL);

			/* If we've run out of things to expire on this list,
			   stop. */
			if (lease -> sort_time > cur_time) {
				if (lease -> sort_time < next_expiry)
					next_expiry = lease -> sort_time;
				break;
			}

			/* If there is a pending state change, and
			   this lease has gotten to the time when the
			   state change should happen, just call
			   supersede_lease on it to make the change
			   happen. */
			if (lease -> next_binding_state !=
			    lease -> binding_state)
				supersede_lease (lease,
						 (struct lease *)0, 1, 1, 1);

			lease_dereference (&lease, MDL);
			if (next)
				lease_reference (&lease, next, MDL);
		}
		if (next)
			lease_dereference (&next, MDL);
		if (lease)
			lease_dereference (&lease, MDL);
	}
	if (next_expiry != MAX_TIME) {
		pool -> next_event_time = next_expiry;
		tv . tv_sec = pool -> next_event_time;
		tv . tv_usec = 0;
		add_timeout (&tv, pool_timer, pool,
			     (tvref_t)pool_reference,
			     (tvunref_t)pool_dereference);
	} else
		pool -> next_event_time = MIN_TIME;

}

/* Locate the lease associated with a given IP address... */

int find_lease_by_ip_addr (struct lease **lp, struct iaddr addr,
			   const char *file, int line)
{
	return lease_ip_hash_lookup(lp, lease_ip_addr_hash, addr.iabuf,
				    addr.len, file, line);
}

int find_lease_by_uid (struct lease **lp, const unsigned char *uid,
		       unsigned len, const char *file, int line)
{
	if (len == 0)
		return 0;
	return lease_id_hash_lookup (lp, lease_uid_hash, uid, len, file, line);
}

int find_lease_by_hw_addr (struct lease **lp,
			   const unsigned char *hwaddr, unsigned hwlen,
			   const char *file, int line)
{
	if (hwlen == 0)
		return 0;
	return lease_id_hash_lookup(lp, lease_hw_addr_hash, hwaddr, hwlen,
				    file, line);
}

/* If the lease is preferred over the candidate, return truth.  The
 * 'cand' and 'lease' names are retained to read more clearly against
 * the 'uid_hash_add' and 'hw_hash_add' functions (this is common logic
 * to those two functions).
 *
 * 1) ACTIVE leases are preferred.  The active lease with
 *    the longest lifetime is preferred over shortest.
 * 2) "transitional states" are next, this time with the
 *    most recent CLTT.
 * 3) free/backup/etc states are next, again with CLTT.  In truth we
 *    should never see reset leases for this.
 * 4) Abandoned leases are always dead last.
 */
static isc_boolean_t
client_lease_preferred(struct lease *cand, struct lease *lease)
{
	if (cand->binding_state == FTS_ACTIVE) {
		if (lease->binding_state == FTS_ACTIVE &&
		    lease->ends >= cand->ends)
			return ISC_TRUE;
	} else if (cand->binding_state == FTS_EXPIRED ||
		   cand->binding_state == FTS_RELEASED) {
		if (lease->binding_state == FTS_ACTIVE)
			return ISC_TRUE;

		if ((lease->binding_state == FTS_EXPIRED ||
		     lease->binding_state == FTS_RELEASED) &&
		    lease->cltt >= cand->cltt)
			return ISC_TRUE;
	} else if (cand->binding_state != FTS_ABANDONED) {
		if (lease->binding_state == FTS_ACTIVE ||
		    lease->binding_state == FTS_EXPIRED ||
		    lease->binding_state == FTS_RELEASED)
			return ISC_TRUE;

		if (lease->binding_state != FTS_ABANDONED &&
		    lease->cltt >= cand->cltt)
			return ISC_TRUE;
	} else /* (cand->binding_state == FTS_ABANDONED) */ {
		if (lease->binding_state != FTS_ABANDONED ||
		    lease->cltt >= cand->cltt)
			return ISC_TRUE;
	}

	return ISC_FALSE;
}

/* Add the specified lease to the uid hash. */
void
uid_hash_add(struct lease *lease)
{
	struct lease *head = NULL;
	struct lease *cand = NULL;
	struct lease *prev = NULL;
	struct lease *next = NULL;

	/* If it's not in the hash, just add it. */
	if (!find_lease_by_uid (&head, lease -> uid, lease -> uid_len, MDL))
		lease_id_hash_add(lease_uid_hash, lease->uid, lease->uid_len,
				  lease, MDL);
	else {
		/* Otherwise, insert it into the list in order of its
		 * preference for "resuming allocation to the client."
		 *
		 * Because we don't have control of the hash bucket index
		 * directly, we have to remove and re-insert the client
		 * id into the hash if we're inserting onto the head.
		 */
		lease_reference(&cand, head, MDL);
		while (cand != NULL) {
			if (client_lease_preferred(cand, lease))
				break;

			if (prev != NULL)
				lease_dereference(&prev, MDL);
			lease_reference(&prev, cand, MDL);

			if (cand->n_uid != NULL)
				lease_reference(&next, cand->n_uid, MDL);

			lease_dereference(&cand, MDL);

			if (next != NULL) {
				lease_reference(&cand, next, MDL);
				lease_dereference(&next, MDL);
			}
		}

		/* If we want to insert 'before cand', and prev is NULL,
		 * then it was the head of the list.  Assume that position.
		 */
		if (prev == NULL) {
			lease_reference(&lease->n_uid, head, MDL);
			lease_id_hash_delete(lease_uid_hash, lease->uid,
					     lease->uid_len, MDL);
			lease_id_hash_add(lease_uid_hash, lease->uid,
					  lease->uid_len, lease, MDL);
		} else /* (prev != NULL) */ {
			if(prev->n_uid != NULL) {
				lease_reference(&lease->n_uid, prev->n_uid,
						MDL);
				lease_dereference(&prev->n_uid, MDL);
			}
			lease_reference(&prev->n_uid, lease, MDL);

			lease_dereference(&prev, MDL);
		}

		if (cand != NULL)
			lease_dereference(&cand, MDL);
		lease_dereference(&head, MDL);
	}
}

/* Delete the specified lease from the uid hash. */

void uid_hash_delete (lease)
	struct lease *lease;
{
	struct lease *head = (struct lease *)0;
	struct lease *scan;

	/* If it's not in the hash, we have no work to do. */
	if (!find_lease_by_uid (&head, lease -> uid, lease -> uid_len, MDL)) {
		if (lease -> n_uid)
			lease_dereference (&lease -> n_uid, MDL);
		return;
	}

	/* If the lease we're freeing is at the head of the list,
	   remove the hash table entry and add a new one with the
	   next lease on the list (if there is one). */
	if (head == lease) {
		lease_id_hash_delete(lease_uid_hash, lease->uid,
				     lease->uid_len, MDL);
		if (lease -> n_uid) {
			lease_id_hash_add(lease_uid_hash, lease->n_uid->uid,
					  lease->n_uid->uid_len, lease->n_uid,
					  MDL);
			lease_dereference (&lease -> n_uid, MDL);
		}
	} else {
		/* Otherwise, look for the lease in the list of leases
		   attached to the hash table entry, and remove it if
		   we find it. */
		for (scan = head; scan -> n_uid; scan = scan -> n_uid) {
			if (scan -> n_uid == lease) {
				lease_dereference (&scan -> n_uid, MDL);
				if (lease -> n_uid) {
					lease_reference (&scan -> n_uid,
							 lease -> n_uid, MDL);
					lease_dereference (&lease -> n_uid,
							   MDL);
				}
				break;
			}
		}
	}
	lease_dereference (&head, MDL);
}

/* Add the specified lease to the hardware address hash. */

void
hw_hash_add(struct lease *lease)
{
	struct lease *head = NULL;
	struct lease *cand = NULL;
	struct lease *prev = NULL;
	struct lease *next = NULL;

	/* If it's not in the hash, just add it. */
	if (!find_lease_by_hw_addr (&head, lease -> hardware_addr.hbuf,
				    lease -> hardware_addr.hlen, MDL))
		lease_id_hash_add(lease_hw_addr_hash,
				  lease->hardware_addr.hbuf,
				  lease->hardware_addr.hlen, lease, MDL);
	else {
		/* Otherwise, insert it into the list in order of its
		 * preference for "resuming allocation to the client."
		 *
		 * Because we don't have control of the hash bucket index
		 * directly, we have to remove and re-insert the client
		 * id into the hash if we're inserting onto the head.
		 */
		lease_reference(&cand, head, MDL);
		while (cand != NULL) {
			if (client_lease_preferred(cand, lease))
				break;

			if (prev != NULL)
				lease_dereference(&prev, MDL);
			lease_reference(&prev, cand, MDL);

			if (cand->n_hw != NULL)
				lease_reference(&next, cand->n_hw, MDL);

			lease_dereference(&cand, MDL);

			if (next != NULL) {
				lease_reference(&cand, next, MDL);
				lease_dereference(&next, MDL);
			}
		}

		/* If we want to insert 'before cand', and prev is NULL,
		 * then it was the head of the list.  Assume that position.
		 */
		if (prev == NULL) {
			lease_reference(&lease->n_hw, head, MDL);
			lease_id_hash_delete(lease_hw_addr_hash,
					     lease->hardware_addr.hbuf,
					     lease->hardware_addr.hlen, MDL);
			lease_id_hash_add(lease_hw_addr_hash,
					  lease->hardware_addr.hbuf,
					  lease->hardware_addr.hlen,
					  lease, MDL);
		} else /* (prev != NULL) */ {
			if(prev->n_hw != NULL) {
				lease_reference(&lease->n_hw, prev->n_hw,
						MDL);
				lease_dereference(&prev->n_hw, MDL);
			}
			lease_reference(&prev->n_hw, lease, MDL);

			lease_dereference(&prev, MDL);
		}

		if (cand != NULL)
			lease_dereference(&cand, MDL);
		lease_dereference(&head, MDL);
	}
}

/* Delete the specified lease from the hardware address hash. */

void hw_hash_delete (lease)
	struct lease *lease;
{
	struct lease *head = (struct lease *)0;
	struct lease *next = (struct lease *)0;

	/* If it's not in the hash, we have no work to do. */
	if (!find_lease_by_hw_addr (&head, lease -> hardware_addr.hbuf,
				    lease -> hardware_addr.hlen, MDL)) {
		if (lease -> n_hw)
			lease_dereference (&lease -> n_hw, MDL);
		return;
	}

	/* If the lease we're freeing is at the head of the list,
	   remove the hash table entry and add a new one with the
	   next lease on the list (if there is one). */
	if (head == lease) {
		lease_id_hash_delete(lease_hw_addr_hash,
				     lease->hardware_addr.hbuf,
				     lease->hardware_addr.hlen, MDL);
		if (lease->n_hw) {
			lease_id_hash_add(lease_hw_addr_hash,
					  lease->n_hw->hardware_addr.hbuf,
					  lease->n_hw->hardware_addr.hlen,
					  lease->n_hw, MDL);
			lease_dereference(&lease->n_hw, MDL);
		}
	} else {
		/* Otherwise, look for the lease in the list of leases
		   attached to the hash table entry, and remove it if
		   we find it. */
		while (head -> n_hw) {
			if (head -> n_hw == lease) {
				lease_dereference (&head -> n_hw, MDL);
				if (lease -> n_hw) {
					lease_reference (&head -> n_hw,
							 lease -> n_hw, MDL);
					lease_dereference (&lease -> n_hw,
							   MDL);
				}
				break;
			}
			lease_reference (&next, head -> n_hw, MDL);
			lease_dereference (&head, MDL);
			lease_reference (&head, next, MDL);
			lease_dereference (&next, MDL);
		}
	}
	if (head)
		lease_dereference (&head, MDL);
}

/* Write all interesting leases to permanent storage. */

int write_leases ()
{
	struct lease *l;
	struct shared_network *s;
	struct pool *p;
	struct host_decl *hp;
	struct group_object *gp;
	struct hash_bucket *hb;
	struct class *cp;
	struct collection *colp;
	int i;
	int num_written;
	struct lease **lptr[RESERVED_LEASES+1];

	/* write all the dynamically-created class declarations. */
	if (collections->classes) {
		numclasseswritten = 0;
		for (colp = collections ; colp ; colp = colp->next) {
			for (cp = colp->classes ; cp ; cp = cp->nic) {
				write_named_billing_class(
						(unsigned char *)cp->name,
							  0, cp);
			}
		}

		/* XXXJAB this number doesn't include subclasses... */ 
		log_info ("Wrote %d class decls to leases file.",
			  numclasseswritten);
	}
	
			
	/* Write all the dynamically-created group declarations. */
	if (group_name_hash) {
	    num_written = 0;
	    for (i = 0; i < group_name_hash -> hash_count; i++) {
		for (hb = group_name_hash -> buckets [i];
		     hb; hb = hb -> next) {
			gp = (struct group_object *)hb -> value;
			if ((gp -> flags & GROUP_OBJECT_DYNAMIC) ||
			    ((gp -> flags & GROUP_OBJECT_STATIC) &&
			     (gp -> flags & GROUP_OBJECT_DELETED))) {
				if (!write_group (gp))
					return 0;
				++num_written;
			}
		}
	    }
	    log_info ("Wrote %d group decls to leases file.", num_written);
	}

	/* Write all the deleted host declarations. */
	if (host_name_hash) {
	    num_written = 0;
	    for (i = 0; i < host_name_hash -> hash_count; i++) {
		for (hb = host_name_hash -> buckets [i];
		     hb; hb = hb -> next) {
			hp = (struct host_decl *)hb -> value;
			if (((hp -> flags & HOST_DECL_STATIC) &&
			     (hp -> flags & HOST_DECL_DELETED))) {
				if (!write_host (hp))
					return 0;
				++num_written;
			}
		}
	    }
	    log_info ("Wrote %d deleted host decls to leases file.",
		      num_written);
	}

	/* Write all the new, dynamic host declarations. */
	if (host_name_hash) {
	    num_written = 0;
	    for (i = 0; i < host_name_hash -> hash_count; i++) {
		for (hb = host_name_hash -> buckets [i];
		     hb; hb = hb -> next) {
			hp = (struct host_decl *)hb -> value;
			if ((hp -> flags & HOST_DECL_DYNAMIC)) {
				if (!write_host (hp))
					++num_written;
			}
		}
	    }
	    log_info ("Wrote %d new dynamic host decls to leases file.",
		      num_written);
	}

#if defined (FAILOVER_PROTOCOL)
	/* Write all the failover states. */
	if (!dhcp_failover_write_all_states ())
		return 0;
#endif

	/* Write all the leases. */
	num_written = 0;
	for (s = shared_networks; s; s = s -> next) {
	    for (p = s -> pools; p; p = p -> next) {
		lptr [FREE_LEASES] = &p -> free;
		lptr [ACTIVE_LEASES] = &p -> active;
		lptr [EXPIRED_LEASES] = &p -> expired;
		lptr [ABANDONED_LEASES] = &p -> abandoned;
		lptr [BACKUP_LEASES] = &p -> backup;
		lptr [RESERVED_LEASES] = &p->reserved;

		for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		    for (l = *(lptr [i]); l; l = l -> next) {
#if !defined (DEBUG_DUMP_ALL_LEASES)
			if (l -> hardware_addr.hlen ||
			    l -> uid_len ||
			    (l -> binding_state != FTS_FREE))
#endif
			{
			    if (!write_lease (l))
				    return 0;
			    num_written++;
			}
		    }
		}
	    }
	}
	log_info ("Wrote %d leases to leases file.", num_written);
#ifdef DHCPv6
	if (!write_leases6()) {
		return 0;
	}
#endif /* DHCPv6 */
	if (!commit_leases ())
		return 0;
	return 1;
}

/* In addition to placing this lease upon a lease queue depending on its
 * state, it also keeps track of the number of FREE and BACKUP leases in
 * existence, and sets the sort_time on the lease.
 *
 * Sort_time is used in pool_timer() to determine when the lease will
 * bubble to the top of the list and be supersede_lease()'d into its next
 * state (possibly, if all goes well).  Example, ACTIVE leases move to
 * EXPIRED state when the 'ends' value is reached, so that is its sort
 * time.  Most queues are sorted by 'ends', since it is generally best
 * practice to re-use the oldest lease, to reduce address collision
 * chances.
 */
int lease_enqueue (struct lease *comp)
{
	struct lease **lq, *prev, *lp;
	static struct lease **last_lq = NULL;
	static struct lease *last_insert_point = NULL;

	/* No queue to put it on? */
	if (!comp -> pool)
		return 0;

	/* Figure out which queue it's going to. */
	switch (comp -> binding_state) {
	      case FTS_FREE:
		if (comp->flags & RESERVED_LEASE) {
			lq = &comp->pool->reserved;
		} else {
			lq = &comp->pool->free;
			comp->pool->free_leases++;
		}
		comp -> sort_time = comp -> ends;
		break;

	      case FTS_ACTIVE:
		lq = &comp -> pool -> active;
		comp -> sort_time = comp -> ends;
		break;

	      case FTS_EXPIRED:
	      case FTS_RELEASED:
	      case FTS_RESET:
		lq = &comp -> pool -> expired;
#if defined(FAILOVER_PROTOCOL)
		/* In partner_down, tsfp is the time at which the lease
		 * may be reallocated (stos+mclt).  We can do that with
		 * lease_mine_to_reallocate() anywhere between tsfp and
		 * ends.  But we prefer to wait until ends before doing it
		 * automatically (choose the greater of the two).  Note
		 * that 'ends' is usually a historic timestamp in the
		 * case of expired leases, is really only in the future
		 * on released leases, and if we know a lease to be released
		 * the peer might still know it to be active...in which case
		 * it's possible the peer has renewed this lease, so avoid
		 * doing that.
		 */
		if (comp->pool->failover_peer &&
		    comp->pool->failover_peer->me.state == partner_down)
			comp->sort_time = (comp->tsfp > comp->ends) ?
					  comp->tsfp : comp->ends;
		else
#endif
			comp->sort_time = comp->ends;

		break;

	      case FTS_ABANDONED:
		lq = &comp -> pool -> abandoned;
		comp -> sort_time = comp -> ends;
		break;

	      case FTS_BACKUP:
		if (comp->flags & RESERVED_LEASE) {
			lq = &comp->pool->reserved;
		} else {
			lq = &comp->pool->backup;
			comp->pool->backup_leases++;
		}
		comp -> sort_time = comp -> ends;
		break;

	      default:
		log_error ("Lease with bogus binding state: %d",
			   comp -> binding_state);
#if defined (BINDING_STATE_DEBUG)
		abort ();
#endif
		return 0;
	}

	/* This only works during server startup: during runtime, the last
	 * lease may be dequeued in between calls.  If the queue is the same
	 * as was used previously, and the lease structure isn't (this is not
	 * a re-queue), use that as a starting point for the insertion-sort.
	 */
	if ((server_starting & SS_QFOLLOW) && (lq == last_lq) &&
	    (comp != last_insert_point) && 
	    (last_insert_point->sort_time <= comp->sort_time)) {
		prev = last_insert_point;
		lp = prev->next;
	} else {
		prev = NULL;
		lp = *lq;
	}

	/* Insertion sort the lease onto the appropriate queue. */
	for (; lp ; lp = lp->next) {
		if (lp -> sort_time >= comp -> sort_time)
			break;
		prev = lp;
	}

	if (prev) {
		if (prev -> next) {
			lease_reference (&comp -> next, prev -> next, MDL);
			lease_dereference (&prev -> next, MDL);
		}
		lease_reference (&prev -> next, comp, MDL);
	} else {
		if (*lq) {
			lease_reference (&comp -> next, *lq, MDL);
			lease_dereference (lq, MDL);
		}
		lease_reference (lq, comp, MDL);
	}
	last_insert_point = comp;
	last_lq = lq;
	return 1;
}

/* For a given lease, sort it onto the right list in its pool and put it
   in each appropriate hash, understanding that it's already by definition
   in lease_ip_addr_hash. */

isc_result_t
lease_instantiate(const void *key, unsigned len, void *object)
{
	struct lease *lease = object;
	struct class *class;
	/* XXX If the lease doesn't have a pool at this point, it's an
	   XXX orphan, which we *should* keep around until it expires,
	   XXX but which right now we just forget. */
	if (!lease -> pool) {
		lease_ip_hash_delete(lease_ip_addr_hash, lease->ip_addr.iabuf,
				     lease->ip_addr.len, MDL);
		return ISC_R_SUCCESS;
	}
		
	/* Put the lease on the right queue.  Failure to queue is probably
	 * due to a bogus binding state.  In such a case, we claim success,
	 * so that later leases in a hash_foreach are processed, but we
	 * return early as we really don't want hw address hash entries or
	 * other cruft to surround such a bogus entry.
	 */
	if (!lease_enqueue(lease))
		return ISC_R_SUCCESS;

	/* Record the lease in the uid hash if possible. */
	if (lease -> uid) {
		uid_hash_add (lease);
	}

	/* Record it in the hardware address hash if possible. */
	if (lease -> hardware_addr.hlen) {
		hw_hash_add (lease);
	}

	/* If the lease has a billing class, set up the billing. */
	if (lease -> billing_class) {
		class = (struct class *)0;
		class_reference (&class, lease -> billing_class, MDL);
		class_dereference (&lease -> billing_class, MDL);
		/* If the lease is available for allocation, the billing
		   is invalid, so we don't keep it. */
		if (lease -> binding_state == FTS_ACTIVE ||
		    lease -> binding_state == FTS_EXPIRED ||
		    lease -> binding_state == FTS_RELEASED ||
		    lease -> binding_state == FTS_RESET)
			bill_class (lease, class);
		class_dereference (&class, MDL);
	}
	return ISC_R_SUCCESS;
}

isc_result_t
lease_instantiate_test(const void *key, unsigned len, void *object)
{
	struct lease *lease = object;
	struct class *class;
	int i;
	/* XXX If the lease doesn't have a pool at this point, it's an
	   XXX orphan, which we *should* keep around until it expires,
	   XXX but which right now we just forget. */
	   
	for (i = 0; i < lease->ip_addr.len; i++) { 		
//		log_debug("ifaddr->iabuf[%d] is %d \n", i, ifaddr.iabuf[i]);
//		log_debug("ipaddr->iabuf[%d] is %d \n", i, ipaddr->iabuf[i]);
		if (((lease->ip_addr.iabuf[i]) & (sub_conf.netmask.iabuf[i])) != 
			((sub_conf.net.iabuf[i]) & (sub_conf.netmask.iabuf[i]))) {			
			/*when break i not ++*/
			break;
		}
	}
	if (lease->ip_addr.len == i) { 		
		/*log_info("lease_instantiate_test ip %s\n", piaddr(lease->ip_addr));*/
	}
	else {
		return ISC_R_SUCCESS;
	}
	
	if (!lease -> pool) {
		lease_ip_hash_delete(lease_ip_addr_hash, lease->ip_addr.iabuf,
				     lease->ip_addr.len, MDL);
		return ISC_R_SUCCESS;
	}
		
	/* Put the lease on the right queue.  Failure to queue is probably
	 * due to a bogus binding state.  In such a case, we claim success,
	 * so that later leases in a hash_foreach are processed, but we
	 * return early as we really don't want hw address hash entries or
	 * other cruft to surround such a bogus entry.
	 */
	if (!lease_enqueue(lease))
		return ISC_R_SUCCESS;

	/* Record the lease in the uid hash if possible. */
	if (lease -> uid) {
		uid_hash_add (lease);
	}

	/* Record it in the hardware address hash if possible. */
	if (lease -> hardware_addr.hlen) {
		hw_hash_add (lease);
	}

	/* If the lease has a billing class, set up the billing. */
	if (lease -> billing_class) {
		class = (struct class *)0;
		class_reference (&class, lease -> billing_class, MDL);
		class_dereference (&lease -> billing_class, MDL);
		/* If the lease is available for allocation, the billing
		   is invalid, so we don't keep it. */
		if (lease -> binding_state == FTS_ACTIVE ||
		    lease -> binding_state == FTS_EXPIRED ||
		    lease -> binding_state == FTS_RELEASED ||
		    lease -> binding_state == FTS_RESET)
			bill_class (lease, class);
		class_dereference (&class, MDL);
	}
	
	return ISC_R_SUCCESS;
}
/* Run expiry events on every pool.   This is called on startup so that
   any expiry events that occurred after the server stopped and before it
   was restarted can be run.   At the same time, if failover support is
   compiled in, we compute the balance of leases for the pool. */

void expire_all_pools ()
{
	struct shared_network *s;
	struct pool *p;
	int i;
	struct lease *l;
	struct lease **lptr[RESERVED_LEASES+1];

	/* Indicate that we are in the startup phase */
	server_starting = SS_NOSYNC | SS_QFOLLOW;

	/* First, go over the hash list and actually put all the leases
	   on the appropriate lists. */
	lease_ip_hash_foreach(lease_ip_addr_hash, lease_instantiate);

	/* Loop through each pool in each shared network and call the
	 * expiry routine on the pool.  It is no longer safe to follow
	 * the queue insertion point, as expiration of a lease can move
	 * it between queues (and this may be the lease that function
	 * points at).
	 */
	server_starting &= ~SS_QFOLLOW;
	for (s = shared_networks; s; s = s -> next) {
	    for (p = s -> pools; p; p = p -> next) {
		pool_timer (p);

		p -> lease_count = 0;
		p -> free_leases = 0;
		p -> backup_leases = 0;

		lptr [FREE_LEASES] = &p -> free;
		lptr [ACTIVE_LEASES] = &p -> active;
		lptr [EXPIRED_LEASES] = &p -> expired;
		lptr [ABANDONED_LEASES] = &p -> abandoned;
		lptr [BACKUP_LEASES] = &p -> backup;
		lptr [RESERVED_LEASES] = &p->reserved;

		for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		    for (l = *(lptr [i]); l; l = l -> next) {
			p -> lease_count++;
			if (l -> ends <= cur_time) {
				if (l->binding_state == FTS_FREE) {
					if (i == FREE_LEASES)
						p->free_leases++;
					else if (i != RESERVED_LEASES)
						log_fatal("Impossible case "
							  "at %s:%d.", MDL);
				} else if (l->binding_state == FTS_BACKUP) {
					if (i == BACKUP_LEASES)
						p->backup_leases++;
					else if (i != RESERVED_LEASES)
						log_fatal("Impossible case "
							  "at %s:%d.", MDL);
				}
			}
#if defined (FAILOVER_PROTOCOL)
			if (p -> failover_peer &&
			    l -> tstp > l -> atsfp &&
			    !(l -> flags & ON_UPDATE_QUEUE)) {
				l -> desired_binding_state = l -> binding_state;
				dhcp_failover_queue_update (l, 1);
			}
#endif
		    }
		}
	    }
	}

	/* turn off startup phase */
	server_starting = 0;
}
#ifndef __AX_PLATFORM__
/* Timer called when a lease in a particular pool expires. */
void pool_timer_test (vpool)
	void *vpool;
{
	struct pool *pool;
	struct lease *next = (struct lease *)0;
	struct lease *lease = (struct lease *)0;
#define FREE_LEASES 0
#define ACTIVE_LEASES 1
#define EXPIRED_LEASES 2
#define ABANDONED_LEASES 3
#define BACKUP_LEASES 4
#define RESERVED_LEASES 5
	struct lease **lptr[RESERVED_LEASES+1];
	TIME next_expiry = MAX_TIME;
	int i;

	pool = (struct pool *)vpool;

	lptr [FREE_LEASES] = &pool -> free;
	lptr [ACTIVE_LEASES] = &pool -> active;
	lptr [EXPIRED_LEASES] = &pool -> expired;
	lptr [ABANDONED_LEASES] = &pool -> abandoned;
	lptr [BACKUP_LEASES] = &pool -> backup;
	lptr[RESERVED_LEASES] = &pool->reserved;

	for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		/* If there's nothing on the queue, skip it. */
		if (!*(lptr [i]))
			continue;

#if defined (FAILOVER_PROTOCOL)
		if (pool -> failover_peer &&
		    pool -> failover_peer -> me.state != partner_down) {
			/* The secondary can't remove a lease from the
			   active state except in partner_down. */
			if (i == ACTIVE_LEASES &&
			    pool -> failover_peer -> i_am == secondary)
				continue;
			/* Leases in an expired state don't move to
			   free because of a timeout unless we're in
			   partner_down. */
			if (i == EXPIRED_LEASES)
				continue;
		}
#endif		
		lease_reference (&lease, *(lptr [i]), MDL);

		while (lease) {
			/* Remember the next lease in the list. */
			if (next)
				lease_dereference (&next, MDL);
			if (lease -> next)
				lease_reference (&next, lease -> next, MDL);

			/* If we've run out of things to expire on this list,
			   stop. */
			if (lease -> sort_time > cur_time) {
				if (lease -> sort_time < next_expiry)
					next_expiry = lease -> sort_time;
				break;
			}

			/* If there is a pending state change, and
			   this lease has gotten to the time when the
			   state change should happen, just call
			   supersede_lease on it to make the change
			   happen. */
			if (lease -> next_binding_state !=
			    lease -> binding_state)
				supersede_lease (lease,
						 (struct lease *)0, 1, 1, 1);

			lease_dereference (&lease, MDL);
			if (next)
				lease_reference (&lease, next, MDL);
		}
		if (next)
			lease_dereference (&next, MDL);
		if (lease)
			lease_dereference (&lease, MDL);
	}
	if (next_expiry != MAX_TIME) {
		pool -> next_event_time = next_expiry;
		/* NOTICE : (struct timeval *)pool->next_event_time * */
		add_timeout ((struct timeval *)pool->next_event_time, pool_timer, pool,
			     (tvref_t)pool_reference,
			     (tvunref_t)pool_dereference);
	} else
		pool -> next_event_time = MIN_TIME;

}

/* Add the specified lease to the hardware address hash. */
void
hw_hash_add_test(struct lease *lease)
{
	struct lease *head = NULL;

	/* If it's not in the hash, just add it. */
	if (!find_lease_by_hw_addr (&head, lease -> hardware_addr.hbuf,
				    lease -> hardware_addr.hlen, MDL))
		lease_id_hash_add(lease_hw_addr_hash,
				  lease->hardware_addr.hbuf,
				  lease->hardware_addr.hlen, lease, MDL);
	else {
		log_info("the lease is already in hw_lease_hash, hw_addr is %s\n", 
					print_hw_addr(lease->hardware_addr.hbuf [0],
					lease->hardware_addr.hlen - 1, &lease->hardware_addr.hbuf [1]));
	}
}

void expire_all_pools_test 
(
	struct shared_network* share
)
{
	struct shared_network *s;
	struct pool *p;
	int i;
	struct lease *l;
	struct lease **lptr[RESERVED_LEASES+1];

	/* First, go over the hash list and actually put all the leases
	   on the appropriate lists. */
	lease_ip_hash_foreach(lease_ip_addr_hash, lease_instantiate_test);

	/* Loop through each pool in each shared network and call the
	 * expiry routine on the pool.  It is no longer safe to follow
	 * the queue insertion point, as expiration of a lease can move
	 * it between queues (and this may be the lease that function
	 * points at).
	 */
/*	for (s = shared_networks; s; s = s -> next) {
log_info("expire_all_pools shared network name 000 %s \n", s->name);
if(s->name && (0 == strncmp(s->name, "192.168.4.0/24", 14))){
log_info("expire_all_pools shared network name 111%s \n", s->name);
	*/	
	
	s = share;
    for (p = s->pools; p; p = p->next) {
			
		pool_timer (p);
log_info("expire_all_pools shared network dddd name %s \n", s->name);

		p -> lease_count = 0;
		p -> free_leases = 0;
		p -> backup_leases = 0;

		lptr [FREE_LEASES] = &p -> free;
		lptr [ACTIVE_LEASES] = &p -> active;
		lptr [EXPIRED_LEASES] = &p -> expired;
		lptr [ABANDONED_LEASES] = &p -> abandoned;
		lptr [BACKUP_LEASES] = &p -> backup;
		lptr [RESERVED_LEASES] = &p->reserved;

		for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		    for (l = *(lptr [i]); l; l = l -> next) {
				p -> lease_count++;
				if (l -> ends <= cur_time) {
					if (l->binding_state == FTS_FREE) {
						if (i == FREE_LEASES)
							p->free_leases++;
						else if (i != RESERVED_LEASES)
							log_fatal("Impossible case "
								  "at %s:%d.", MDL);
					} else if (l->binding_state == FTS_BACKUP) {
						if (i == BACKUP_LEASES)
							p->backup_leases++;
						else if (i != RESERVED_LEASES)
							log_fatal("Impossible case "
								  "at %s:%d.", MDL);
					}
				}
		    }
		}
	
log_info("expire_all_pools shared network run 1.2 %s \n", s->name);
    }	
	/*}
log_info("expire_all_pools shared network run 1.4 %s \n", s->name);
	}
log_info("expire_all_pools run here 3 \n");*/
 /*turn off startup phase */
}

void expire_pool_for_failover 
(
	struct dcli_failover* failover_peer,
	struct shared_network* share
)
{
	struct shared_network *s;
	struct pool *p;
	int status;
	char* val;
	
	s = share;
	/*need for p is the only pool of the shared_network*/
    for (p = s->pools; p; p = p->next) {
log_info("expire_pool_for_failover shared network run 1.0 %s \n", s->name);
		/* Inherit the failover peer from the shared network. */
		if (p->shared_network->failover_peer) {
		    dhcp_failover_state_reference(&p->failover_peer, 
			     p->shared_network->failover_peer, MDL);
		}
		/* case FAILOVER: for pool*/
		val = failover_peer->name;
		if (p->failover_peer) {
			dhcp_failover_state_dereference (&p->failover_peer, MDL);
		}
		status = find_failover_peer (&p->failover_peer, val, MDL);
		if (status != ISC_R_SUCCESS) {
			log_error("failover peer %s: %s \n", val, isc_result_totext (status));
		}
		else {
			p->failover_peer->pool_count++;
		}
log_info("expire_pool_for_failover shared network run 1.2 %s \n", s->name);
    }	
}
#endif

void dump_subnets ()
{
	struct lease *l;
	struct shared_network *s;
	struct subnet *n;
	struct pool *p;
	struct lease **lptr[RESERVED_LEASES+1];
	int i;

	log_info ("Subnets:");
	for (n = subnets; n; n = n -> next_subnet) {
		log_debug ("  Subnet %s", piaddr (n -> net));
		log_debug ("     netmask %s",
		       piaddr (n -> netmask));
	}
	log_info ("Shared networks:");
	for (s = shared_networks; s; s = s -> next) {
	    log_info ("  %s", s -> name);
	    for (p = s -> pools; p; p = p -> next) {
		lptr [FREE_LEASES] = &p -> free;
		lptr [ACTIVE_LEASES] = &p -> active;
		lptr [EXPIRED_LEASES] = &p -> expired;
		lptr [ABANDONED_LEASES] = &p -> abandoned;
		lptr [BACKUP_LEASES] = &p -> backup;
		lptr [RESERVED_LEASES] = &p->reserved;

		for (i = FREE_LEASES; i <= RESERVED_LEASES; i++) {
		    for (l = *(lptr [i]); l; l = l -> next) {
			    print_lease (l);
		    }
		}
	    }
	}
}

HASH_FUNCTIONS(lease_ip, const unsigned char *, struct lease, lease_ip_hash_t,
	       lease_reference, lease_dereference, do_ip4_hash)
HASH_FUNCTIONS(lease_id, const unsigned char *, struct lease, lease_id_hash_t,
	       lease_reference, lease_dereference, do_id_hash)
HASH_FUNCTIONS (host, const unsigned char *, struct host_decl, host_hash_t,
		host_reference, host_dereference, do_string_hash)
HASH_FUNCTIONS (class, const char *, struct class, class_hash_t,
		class_reference, class_dereference, do_string_hash)

#if defined (DEBUG_MEMORY_LEAKAGE) && \
		defined (DEBUG_MEMORY_LEAKAGE_ON_EXIT)
extern struct hash_table *dns_zone_hash;
extern struct interface_info **interface_vector;
extern int interface_count;
dhcp_control_object_t *dhcp_control_object;
extern struct hash_table *auth_key_hash;
struct hash_table *universe_hash;
struct universe **universes;
int universe_count, universe_max;
#if 0
extern int end;
#endif

#if defined (COMPACT_LEASES)
extern struct lease *lease_hunks;
#endif

void free_everything(void)
{
	struct subnet *sc = (struct subnet *)0, *sn = (struct subnet *)0;
	struct shared_network *nc = (struct shared_network *)0,
		*nn = (struct shared_network *)0;
	struct pool *pc = (struct pool *)0, *pn = (struct pool *)0;
	struct lease *lc = (struct lease *)0, *ln = (struct lease *)0;
	struct interface_info *ic = (struct interface_info *)0,
		*in = (struct interface_info *)0;
	struct class *cc = (struct class *)0, *cn = (struct class *)0;
	struct collection *lp;
	int i;

	/* Get rid of all the hash tables. */
	if (host_hw_addr_hash)
		host_free_hash_table (&host_hw_addr_hash, MDL);
	host_hw_addr_hash = 0;
	if (host_uid_hash)
		host_free_hash_table (&host_uid_hash, MDL);
	host_uid_hash = 0;
	if (lease_uid_hash)
		lease_id_free_hash_table (&lease_uid_hash, MDL);
	lease_uid_hash = 0;
	if (lease_ip_addr_hash)
		lease_ip_free_hash_table (&lease_ip_addr_hash, MDL);
	lease_ip_addr_hash = 0;
	if (lease_hw_addr_hash)
		lease_id_free_hash_table (&lease_hw_addr_hash, MDL);
	lease_hw_addr_hash = 0;
	if (host_name_hash)
		host_free_hash_table (&host_name_hash, MDL);
	host_name_hash = 0;
	if (dns_zone_hash)
		dns_zone_free_hash_table (&dns_zone_hash, MDL);
	dns_zone_hash = 0;

	while (host_id_info != NULL) {
		host_id_info_t *tmp;
		option_dereference(&host_id_info->option, MDL);
		host_free_hash_table(&host_id_info->values_hash, MDL);
		tmp = host_id_info->next;
		dfree(host_id_info, MDL);
		host_id_info = tmp;
	}
#if 0
	if (auth_key_hash)
		auth_key_free_hash_table (&auth_key_hash, MDL);
#endif
	auth_key_hash = 0;

	omapi_object_dereference ((omapi_object_t **)&dhcp_control_object,
				  MDL);

	for (lp = collections; lp; lp = lp -> next) {
	    if (lp -> classes) {
		class_reference (&cn, lp -> classes, MDL);
		do {
		    if (cn) {
			class_reference (&cc, cn, MDL);
			class_dereference (&cn, MDL);
		    }
		    if (cc -> nic) {
			class_reference (&cn, cc -> nic, MDL);
			class_dereference (&cc -> nic, MDL);
		    }
		    group_dereference (&cc -> group, MDL);
		    if (cc -> hash) {
			    class_free_hash_table (&cc -> hash, MDL);
			    cc -> hash = (struct hash_table *)0;
		    }
		    class_dereference (&cc, MDL);
		} while (cn);
		class_dereference (&lp -> classes, MDL);
	    }
	}

	if (interface_vector) {
	    for (i = 0; i < interface_count; i++) {
		if (interface_vector [i])
		    interface_dereference (&interface_vector [i], MDL);
	    }
	    dfree (interface_vector, MDL);
	    interface_vector = 0;
	}

	if (interfaces) {
	    interface_reference (&in, interfaces, MDL);
	    do {
		if (in) {
		    interface_reference (&ic, in, MDL);
		    interface_dereference (&in, MDL);
		}
		if (ic -> next) {
		    interface_reference (&in, ic -> next, MDL);
		    interface_dereference (&ic -> next, MDL);
		}
		omapi_unregister_io_object ((omapi_object_t *)ic);
		if (ic -> shared_network) {
		    if (ic -> shared_network -> interface)
			interface_dereference
				(&ic -> shared_network -> interface, MDL);
		    shared_network_dereference (&ic -> shared_network, MDL);
		}
		interface_dereference (&ic, MDL);
	    } while (in);
	    interface_dereference (&interfaces, MDL);
	}

	/* Subnets are complicated because of the extra links. */
	if (subnets) {
	    subnet_reference (&sn, subnets, MDL);
	    do {
		if (sn) {
		    subnet_reference (&sc, sn, MDL);
		    subnet_dereference (&sn, MDL);
		}
		if (sc -> next_subnet) {
		    subnet_reference (&sn, sc -> next_subnet, MDL);
		    subnet_dereference (&sc -> next_subnet, MDL);
		}
		if (sc -> next_sibling)
		    subnet_dereference (&sc -> next_sibling, MDL);
		if (sc -> shared_network)
		    shared_network_dereference (&sc -> shared_network, MDL);
		group_dereference (&sc -> group, MDL);
		if (sc -> interface)
		    interface_dereference (&sc -> interface, MDL);
		subnet_dereference (&sc, MDL);
	    } while (sn);
	    subnet_dereference (&subnets, MDL);
	}

	/* So are shared networks. */
	/* XXX: this doesn't work presently, but i'm ok just filtering
	 * it out of the noise (you get a bigger spike on the real leaks).
	 * It would be good to fix this, but it is not a "real bug," so not
	 * today.  This hack is incomplete, it doesn't trim out sub-values.
	 */
	if (shared_networks) {
		shared_network_dereference (&shared_networks, MDL);
	/* This is the old method (tries to free memory twice, broken) */
	} else if (0) {
	    shared_network_reference (&nn, shared_networks, MDL);
	    do {
		if (nn) {
		    shared_network_reference (&nc, nn, MDL);
		    shared_network_dereference (&nn, MDL);
		}
		if (nc -> next) {
		    shared_network_reference (&nn, nc -> next, MDL);
		    shared_network_dereference (&nc -> next, MDL);
		}

		/* As are pools. */
		if (nc -> pools) {
		    pool_reference (&pn, nc -> pools, MDL);
		    do {
			struct lease **lptr[RESERVED_LEASES+1];

			if (pn) {
			    pool_reference (&pc, pn, MDL);
			    pool_dereference (&pn, MDL);
			}
			if (pc -> next) {
			    pool_reference (&pn, pc -> next, MDL);
			    pool_dereference (&pc -> next, MDL);
			}

			lptr [FREE_LEASES] = &pc -> free;
			lptr [ACTIVE_LEASES] = &pc -> active;
			lptr [EXPIRED_LEASES] = &pc -> expired;
			lptr [ABANDONED_LEASES] = &pc -> abandoned;
			lptr [BACKUP_LEASES] = &pc -> backup;
			lptr [RESERVED_LEASES] = &pc->reserved;

			/* As (sigh) are leases. */
			for (i = FREE_LEASES ; i <= RESERVED_LEASES ; i++) {
			    if (*lptr [i]) {
				lease_reference (&ln, *lptr [i], MDL);
				do {
				    if (ln) {
					lease_reference (&lc, ln, MDL);
					lease_dereference (&ln, MDL);
				    }
				    if (lc -> next) {
					lease_reference (&ln, lc -> next, MDL);
					lease_dereference (&lc -> next, MDL);
				    }
				    if (lc -> billing_class)
				       class_dereference (&lc -> billing_class,
							  MDL);
				    if (lc -> state)
					free_lease_state (lc -> state, MDL);
				    lc -> state = (struct lease_state *)0;
				    if (lc -> n_hw)
					lease_dereference (&lc -> n_hw, MDL);
				    if (lc -> n_uid)
					lease_dereference (&lc -> n_uid, MDL);
				    lease_dereference (&lc, MDL);
				} while (ln);
				lease_dereference (lptr [i], MDL);
			    }
			}
			if (pc -> group)
			    group_dereference (&pc -> group, MDL);
			if (pc -> shared_network)
			    shared_network_dereference (&pc -> shared_network,
							MDL);
			pool_dereference (&pc, MDL);
		    } while (pn);
		    pool_dereference (&nc -> pools, MDL);
		}
		/* Because of a circular reference, we need to nuke this
		   manually. */
		group_dereference (&nc -> group, MDL);
		shared_network_dereference (&nc, MDL);
	    } while (nn);
	    shared_network_dereference (&shared_networks, MDL);
	}

	cancel_all_timeouts ();
	relinquish_timeouts ();
	relinquish_ackqueue();
	trace_free_all ();
	group_dereference (&root_group, MDL);
	executable_statement_dereference (&default_classification_rules, MDL);

	shutdown_state = shutdown_drop_omapi_connections;
	omapi_io_state_foreach (dhcp_io_shutdown, 0);
	shutdown_state = shutdown_listeners;
	omapi_io_state_foreach (dhcp_io_shutdown, 0);
	shutdown_state = shutdown_dhcp;
	omapi_io_state_foreach (dhcp_io_shutdown, 0);

	omapi_object_dereference ((omapi_object_t **)&icmp_state, MDL);

	universe_free_hash_table (&universe_hash, MDL);
	for (i = 0; i < universe_count; i++) {
#if 0
		union {
			const char *c;
			char *s;
		} foo;
#endif
		if (universes [i]) {
			if (universes[i]->name_hash)
			    option_name_free_hash_table(
						&universes[i]->name_hash,
						MDL);
			if (universes[i]->code_hash)
			    option_code_free_hash_table(
						&universes[i]->code_hash,
						MDL);
#if 0
			if (universes [i] -> name > (char *)&end) {
				foo.c = universes [i] -> name;
				dfree (foo.s, MDL);
			}
			if (universes [i] > (struct universe *)&end)
				dfree (universes [i], MDL);
#endif
		}
	}
	dfree (universes, MDL);

	relinquish_free_lease_states ();
	relinquish_free_pairs ();
	relinquish_free_expressions ();
	relinquish_free_binding_values ();
	relinquish_free_option_caches ();
	relinquish_free_packets ();
#if defined(COMPACT_LEASES)
	relinquish_lease_hunks ();
#endif
	relinquish_hash_bucket_hunks ();
	omapi_type_relinquish ();
}
#endif /* DEBUG_MEMORY_LEAKAGE_ON_EXIT */
