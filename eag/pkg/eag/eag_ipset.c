#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <netdb.h>
#include <dlfcn.h>
#include <fcntl.h>

#include "eag_conf.h"
#include "eag_util.h"
#include "eag_ipset.h" 
#include "eag_log.h"
#include "eag_errcode.h"
#include "eag_interface.h"

const size_t adt_size = sizeof(struct ip_set_req_iphash);
ip_set_id_t max_sets = 0;

static int ipset_sockfd = -1;
/*******************************************************************************/

void *ipset_malloc(size_t size)
{
	void *p;
	if (size == 0)
		return NULL;
	
	if ((p = malloc(size)) == NULL) {
		eag_log_err("ipset_malloc: not enough memory");
	}
	return p;
}

void ipset_free(void **data) 
{
	if (*data  == NULL) 
		return;

	free(*data);
	*data = NULL;
}

int kernel_getsocket(void)
{
	int sockfd = -1;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

	if (sockfd < 0) {
		eag_log_err("You need to be root to perform it");
		return EAG_ERR_SOCKET_FAILED;
	}
	return sockfd;
 }

int kernel_getfrom(void *data, socklen_t *size)
{
	int res=-1;

	if (ipset_sockfd >= 0) {
		res = getsockopt(ipset_sockfd, SOL_IP, SO_IP_SET, data, size);
		if (res != 0 && errno == ENOPROTOOPT) {
			eag_log_info("Try insmod ip_set");
		} else if (res != 0) {
			eag_log_err("getsockopt:%d(%s)", errno, safe_strerror(errno));
		}
	}
	return res;
}

int kernel_sendto(void *data, socklen_t size)
{
	int res = -1;

	if (ipset_sockfd >= 0) {
		res = setsockopt(ipset_sockfd, SOL_IP, SO_IP_SET, data, size);
		if (res != 0 && errno == ENOPROTOOPT) {
			eag_log_info("Try insmod ip_set");
		} else if (res != 0) {
			eag_log_err("setsockopt:%d(%s)", errno, safe_strerror(errno));
		}
	}
	return res;
}

#if 0
uint32_t getnumber(void *data, size_t len)
{
	size_t offset = 0;
	ip_set_ip_t *ip;
	uint32_t num = 0;

	while (offset < len) {
		ip = data + offset;
		if (*ip) {
			num++;
		}
		offset += sizeof(ip_set_ip_t);
	}
	return num;
}

void copyips(void *data, size_t len, ip_set_ip_t *ipdata)
{
	size_t offset = 0;
	ip_set_ip_t *ip;
	int i = 0;

	while (offset < len) {
		ip = data + offset;
		if (*ip) {
			ipdata[i] = *ip;
			i++;
		}
		offset += sizeof(ip_set_ip_t);
	}
}
#endif
/*******************************************************************************/
#if 0
ip_set_id_t set_adt_get(const char *name)
{
	struct ip_set_req_adt_get req_adt_get = {0};
	socklen_t size = 0;
	int res = 0;
	
	req_adt_get.op = IP_SET_OP_ADT_GET;
	req_adt_get.version = IP_SET_PROTOCOL_VERSION;
	strncpy(req_adt_get.set.name, name, IP_SET_MAXNAMELEN - 1);
	size = sizeof(struct ip_set_req_adt_get);

	res = kernel_getfrom((void *) &req_adt_get, &size);
	if (res != 0) {
		eag_log_err("kernel_getfrom error.\n");
		return res;
	}
	return req_adt_get.set.index;	
}
#endif
ip_set_id_t set_index_find_byname(const char *setname)
{
	struct ip_set_req_get_set req = {0};
	socklen_t size = sizeof(struct ip_set_req_get_set);
	int res = 0;

	req.op = IP_SET_OP_GET_BYNAME;
	req.version = IP_SET_PROTOCOL_VERSION;
	strncpy(req.set.name, setname, IP_SET_MAXNAMELEN - 1);

	res = kernel_getfrom((void *) &req, &size);
	if (res != 0) {
		eag_log_err("kernel_getfrom error.\n");
		return res;
	}
	return req.set.index;	
}


/*
 * Send add/del/test order to kernel for a set
 */
int set_adtip(ip_set_id_t index, const ip_set_ip_t adt, unsigned op)
{
	struct ip_set_req_adt *req_adt;
	size_t size;
	void *data;
	int res = 0;
	struct ip_set_req_iphash *ipdata = malloc(sizeof(struct ip_set_req_iphash));
	if(ipdata == NULL) {
		eag_log_err("malloc ip error!");
		return EAG_ERR_MALLOC_FAILED;
	}
	
	memset(ipdata, 0, adt_size);
	ipdata->ip = adt;
		
	/* Alloc memory for the data to send */
	size = sizeof(struct ip_set_req_adt) + adt_size ;
	data = ipset_malloc(size);
	
	/* Fill out the request */
	req_adt = (struct ip_set_req_adt *) data;
	req_adt->op = op;
	req_adt->index = index;
	memcpy(data + sizeof(struct ip_set_req_adt), ipdata, adt_size);
	
	if (kernel_sendto(data, size) == -1) {
		switch (op) {
		case IP_SET_OP_ADD_IP:
			eag_log_info("%d is already in set %d.", adt, index);
			break;
		case IP_SET_OP_DEL_IP:
			eag_log_info("%d is not in set %d.", adt, index);
			break;
		case IP_SET_OP_TEST_IP:
			eag_log_info("%d is in set %d.\n", adt, index);
			res = 0;
			break;
		default:
			break;
		}
	} else {
		switch (op) {
		case IP_SET_OP_TEST_IP:
			eag_log_info("%d is NOT in set %d.\n", adt, index);
			res = 0;
			break;
		default:
			break;
		}
	}
	free(data);
	free(ipdata);

	return res;
}

#if 0
static size_t load_set_list(const char name[IP_SET_MAXNAMELEN],
                            ip_set_id_t *id_num, unsigned op)
{
	void *data = NULL;
	struct ip_set_req_max_sets req_max_sets;
	socklen_t size;
	socklen_t req_size;	
	int repeated = 0;	
	int res = 0;

tryagain:
	/* Get max_sets */
	req_max_sets.op = IP_SET_OP_MAX_SETS;
	req_max_sets.version = IP_SET_PROTOCOL_VERSION;
	strcpy(req_max_sets.set.name, name);
	
	size = sizeof(req_max_sets);
	
	kernel_getfrom(&req_max_sets, &size);

	max_sets = req_max_sets.max_sets;
	*id_num = req_max_sets.set.index;

	/* No sets in kernel */
	if (req_max_sets.sets == 0)
		return 0;
	
	/* Get setnames */
	size = req_size = sizeof(struct ip_set_req_setnames)
	                  + req_max_sets.sets * sizeof(struct ip_set_name_list);

	data = ipset_malloc(size);
	((struct ip_set_req_setnames *) data)->op = op;
	((struct ip_set_req_setnames *) data)->index = *id_num;

	res = kernel_getfrom(data, &size);

	if (res != 0 || size != req_size) {
		free(data);
		if (repeated++ < LIST_TRIES)
			goto tryagain;
		eag_log_info("Tried to get sets from kernel "
			   "and failed.Please try again");
	}
	size = ((struct ip_set_req_setnames *) data)->size;
	free(data);

	return size;
}
#endif

/*******************************************************************************/


int add_ip_to_iphashset(const char *set_name, const ip_set_ip_t ip)
{
	ip_set_id_t index;
	int res;
	
	index = set_index_find_byname(set_name);
	res = set_adtip(index, ip, IP_SET_OP_ADD_IP);
	if (res != 0) {
		eag_log_err("set_adtip error\n");
		return -1;
	}

	return 0;		
}

int del_ip_from_iphashset(const char *set_name, const ip_set_ip_t ip)
{
	ip_set_id_t index;
	int res;
	
	index = set_index_find_byname(set_name);
	res = set_adtip(index, ip, IP_SET_OP_DEL_IP);
	if (res != 0) {
		eag_log_err("set_adtip error\n");
		return -1;
	}

	return 0;		
}

int test_ip(const char *set_name, const ip_set_ip_t ip)
{
	ip_set_id_t index;
	int res;
	
	index = set_index_find_byname(set_name);
	res = set_adtip(index, ip, IP_SET_OP_TEST_IP);
	if (res != 0) {
		eag_log_err("set_adtip error\n");
		return -1;
	}

	return 0;		
}
#if 0
uint32_t list_members(const char *set_name, ip_set_ip_t **ips )	/* ips should be free after use */
{
	void *data = NULL;
	ip_set_id_t id_num;
	socklen_t size;
	socklen_t req_size;
	int res = 0;
	size_t offset;
	uint32_t ip_num=0;

	/* Get size of set_list from kernel */
	size = req_size = load_set_list(set_name, &id_num, IP_SET_OP_LIST_SIZE);

	if (size) {
		/* Get sets */
		data = ipset_malloc(size);
		((struct ip_set_req_list *) data)->op = IP_SET_OP_LIST;
		((struct ip_set_req_list *) data)->index = id_num;
		
		res = kernel_getfrom(data, &size);

		if (res != 0 || size != req_size) {
			free(data);
			return -EAGAIN;
		}
		size = 0;
	}

	struct ip_set_list *setlist = (struct ip_set_list *) data;

	offset = sizeof(struct ip_set_list);
	offset += setlist->header_size;
	ip_num = getnumber(data + offset, setlist->members_size);

	*ips = ipset_malloc(ip_num * sizeof(ip_set_ip_t));
	if(NULL == *ips) {
		free(data);
		return 0;
	}
	copyips(data + offset, setlist->members_size, *ips);

	free(data);
	return ip_num;
}
#endif

/*******************************************************************************/

int add_user_to_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip)
{
	char set_name[256];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| (user_ip>>24 == 0)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d "
				"user_ip=%x", user_id, hansitype, user_ip);
		
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"CP_%s%d_AUTHORIZED_SET", cpid_prefix, user_id);

	return add_ip_to_iphashset(set_name, user_ip);	
}

int del_user_from_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip)
{
	char set_name[256];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| (user_ip>>24 == 0)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d "
				"user_ip=%x", user_id, hansitype, user_ip);
		
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"CP_%s%d_AUTHORIZED_SET", cpid_prefix, user_id);

	return del_ip_from_iphashset(set_name, user_ip);	
}

int add_preauth_user_to_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip)
{
	char set_name[256];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| (user_ip>>24 == 0)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d "
				"user_ip=%x", user_id, hansitype, user_ip);
		
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"MAC_PRE_%s%d_AUTH_SET", cpid_prefix, user_id);

	return add_ip_to_iphashset(set_name, user_ip);	
}

int del_preauth_user_from_set(const int user_id, const int hansitype,
		const ip_set_ip_t user_ip)
{
	char set_name[256];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| (user_ip>>24 == 0)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d "
				"user_ip=%x", user_id, hansitype, user_ip);
		
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
	memset(set_name, 0, sizeof(set_name));
	snprintf(set_name, sizeof(set_name),
		"MAC_PRE_%s%d_AUTH_SET", cpid_prefix, user_id);

	return del_ip_from_iphashset(set_name, user_ip);	
}

ip_set_id_t eag_get_set_byname(const char *setname)
{
	return set_index_find_byname(setname);
}

int
eag_ipset_init()
{
	if (ipset_sockfd >= 0) {
		eag_log_err("eag_ipset_init already start fd(%d)", 
			ipset_sockfd);
		return EAG_RETURN_OK;
	}
	
	ipset_sockfd = kernel_getsocket();
	if (ipset_sockfd < 0) {
		eag_log_err("Can't create ipset dgram socket: %s",
				safe_strerror(errno));
		ipset_sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}
	return EAG_RETURN_OK;
}

int
eag_ipset_exit()
{
	close(ipset_sockfd);
	ipset_sockfd = -1;

	return EAG_RETURN_OK;

}



/*******************************************************************************/
/*******************************************************************************/
#ifdef eag_ipset_test

#include <mcheck.h>
void parse_ip(const char *str, ip_set_ip_t * ip)
{
	struct in_addr addr;
	
	if (inet_aton(str, &addr) != 0) {
		*ip = ntohl(addr.s_addr);
		return;
	}
	eag_log_err("network '%s' not found", str);
}

char *ip_tostring(ip_set_ip_t ip)
{
	struct in_addr addr;
	addr.s_addr = htonl(ip);

	return inet_ntoa(addr);
}

int main(int argc, char *argv[])
{
	mtrace();
	const char *setname = argv[1];
/* ≤‚ ‘,ÃÌº””Î…æ≥˝ */
/* */
	char *adip = argv[2];
	ip_set_ip_t adtip;
	parse_ip(adip, &adtip);
	
//	add_ip_to_iphashset(setname, adtip);
//	del_ip_from_iphashset(setname, adtip);
	test_ip(setname, adtip);

/* ≤‚ ‘±È¿˙set≥…‘±ip 
	int i;
	int ip_num;
	ip_set_ip_t *ips = NULL;

	ip_num = list_members(setname, &ips);
	printf("ip_num:%d\n", ip_num);

	for (i = 0; i < ip_num; i++) {
		printf("%s\n", ip_tostring(ips[i]));
	}

	ipset_free(&ips);
	if (ips == NULL) {
		printf("free ips success\n");
	}
*/

	return 0;
}

#endif
