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
#include <netinet/in.h>   /* struct in[6]_addr */
#include <libmnl/libmnl.h>
#include <linux/netlink.h>

#include "eag_conf.h"
#include "eag_util.h"
#include "eag_ipset.h"
#include "eag_ipset.h"
#include "eag_mem.h" 
#include "eag_log.h"
#include "eag_errcode.h"
#include "eag_interface.h"

#define IPSET6_PROTOCOL         6
#define IPSET6_ATTR_PROTOCOL    1
#define IPSET6_ATTR_SETNAME     2
#define IPSET6_ATTR_IP          1
#define IPSET6_ATTR_IPADDR_IPV4 1
#define IPSET6_ATTR_LINENO      9
#define IPSET6_ATTR_DATA        7

#define PRIVATE_MSG_BUFLEN      256
#define IPSET6_MAXNAMELEN       32

#ifndef NFNL_SUBSYS_IPSET
#define NFNL_SUBSYS_IPSET 6
#endif

#ifndef NFNETLINK_V0
#define NFNETLINK_V0      0
#endif

enum ipset6_cmd {
	IPSET6_CMD_PROTOCOL = 1, /* 1: Return protocol version */
	IPSET6_CMD_ADD = 9,      /* 9: Add an element to a set */
	IPSET6_CMD_DEL = 10,          /* 10: Delete an element from a set */
	IPSET6_CMD_HEADER = 12,  /* 12: Get set header data only */
};

union nf_inet_addr {
	uint32_t	ip;
	uint32_t	ip6[4];
	struct in_addr	in;
	struct in6_addr in6;
};

/* Internal data structure for the kernel-userspace communication parameters */
struct ipset6_handle {
	struct mnl_socket *h;		/* the mnl socket */
	unsigned int seq;		/* netlink message sequence number */

    enum ipset6_cmd cmd;
	union nf_inet_addr ip;
	char setname[IPSET6_MAXNAMELEN];
	char buffer[PRIVATE_MSG_BUFLEN];
};

static struct ipset6_handle *handle;

struct nfgenmsg {
	uint8_t nfgen_family;
	uint8_t version;
	uint16_t res_id;
};

static int
rawdata2attr(struct nlmsghdr *nlh, const void *value, int type)
{
	int alen;
	uint16_t flags = 0;
	uint32_t value1 = 0;

	switch (type) {
	case IPSET6_ATTR_SETNAME:
		alen = strlen(value) + 1;
		if (IPSET6_MAXNAMELEN < alen) {
			eag_log_err("setname is too long, out of size:%d", IPSET6_MAXNAMELEN - 1);
			return 1;
		}
		mnl_attr_put(nlh, type | flags, alen, value);
		break;
	case IPSET6_ATTR_IP:
		mnl_attr_put(nlh, IPSET6_ATTR_IPADDR_IPV4 | NLA_F_NET_BYTEORDER, sizeof(uint32_t), value);
		break;
	case IPSET6_ATTR_LINENO:
		value1 = htonl(*(const uint32_t *)value);
		mnl_attr_put(nlh, type | NLA_F_NET_BYTEORDER, sizeof(uint32_t), &value1);
		break;
	default:
		break;
	}

	return 0;
}

static int
ipset_get_nlmsg_type(const struct nlmsghdr *nlh)
{
	return nlh->nlmsg_type & ~(NFNL_SUBSYS_IPSET << 8);
}

static void
ipset6_debug_msg(const char *dir, void *buffer, int len)
{
	const struct nlmsghdr *nlh = buffer;
	int cmd;

	while (mnl_nlmsg_ok(nlh, len)) {
		switch (nlh->nlmsg_type) {
		case NLMSG_NOOP:
		case NLMSG_DONE:
		case NLMSG_OVERRUN:
			eag_log_debug("eag_ipset6", "Message header:%s msg:%s len:%d seq:%u\n",
				dir,
				nlh->nlmsg_type == NLMSG_NOOP ? "NOOP" :
				nlh->nlmsg_type == NLMSG_DONE ? "DONE" :
				"OVERRUN",
				len, nlh->nlmsg_seq);
			goto next_msg;
		case NLMSG_ERROR: {
			const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);
 			eag_log_debug("eag_ipset6", "Message header:%s msg:ERROR len:%d errcode:%d seq:%u\n",
				dir, len, err->error, nlh->nlmsg_seq);
			goto next_msg;
			}
		default:
			;
		}
		cmd = ipset_get_nlmsg_type(nlh);
		eag_log_debug("eag_ipset6", "Message header:%s cmd:%d len:%d flag:%s seq:%u\n",
			dir, cmd, len,
			!(nlh->nlmsg_flags & NLM_F_EXCL) ? "EXIST" : "none",
			nlh->nlmsg_seq);
next_msg:
		nlh = mnl_nlmsg_next(nlh, &len);
	}
}

static int
ipset6_mnl_query(struct ipset6_handle *handle)
{
	struct nlmsghdr *nlh = (void *)handle->buffer;
	int ret;

	if (NULL == handle) {
        eag_log_err("ipset6_mnl_query input error");
		return -1;
	}
	eag_log_debug("eag_ipset6", "ipset6_mnl_query");

	nlh->nlmsg_seq = ++handle->seq;
	ipset6_debug_msg("sent", nlh, nlh->nlmsg_len);

	if (mnl_socket_sendto(handle->h, nlh, nlh->nlmsg_len) < 0)
		return -1;

	ret = mnl_socket_recvfrom(handle->h, handle->buffer, sizeof(handle->buffer));
	ipset6_debug_msg("received", handle->buffer, ret);

	return ret > 0 ? 0 : ret;
}

static int
ipset6_mnl_fill_hdr(enum ipset6_cmd cmd, void *buffer)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfg;

	if (NULL == buffer) {
        eag_log_err("ipset6_mnl_fill_hdr, input error");
        return -1;
	}
	eag_log_debug("eag_ipset6", "ipset6_mnl_fill_hdr, cmd:%d", cmd);

	nlh = mnl_nlmsg_put_header(buffer);
	nlh->nlmsg_type = cmd | (NFNL_SUBSYS_IPSET << 8);

	switch (cmd) {
	case IPSET6_CMD_ADD:
	case IPSET6_CMD_DEL:
		nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL;
		break;
	default :
		nlh->nlmsg_flags = NLM_F_REQUEST;
		break;
	}

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfg->nfgen_family = AF_INET;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = htons(0);

	return 0;
}

static int
build_send_private_msg(struct ipset6_handle *handle, enum ipset6_cmd cmd)
{
	if (NULL == handle) {
		return -1;
	}
	struct nlmsghdr *nlh = (void *)handle->buffer;
	int ret;
	eag_log_debug("eag_ipset6", "build_send_private_msg, cmd:%d", cmd);

	/* Initialize header */
	memset(handle->buffer, 0, sizeof(handle->buffer));
	ipset6_mnl_fill_hdr(cmd, handle->buffer);

	mnl_attr_put_u8(nlh, IPSET6_ATTR_PROTOCOL, IPSET6_PROTOCOL);
	
	switch (cmd) {
	case IPSET6_CMD_PROTOCOL:
		break;
	case IPSET6_CMD_HEADER:
        rawdata2attr(nlh, handle->setname, IPSET6_ATTR_SETNAME);
		break;
	default:
		return -1;
	}

	/* Backup, then restore real command */
	ret = ipset6_mnl_query(handle);

	return ret;
}

static int
ipset6_commit(struct ipset6_handle *handle)
{
	struct nlmsghdr *nlh;
	int ret = 0;

	if (NULL == handle) {
        eag_log_err("ipset6_commit input error");
		return -1;
	}
	eag_log_debug("eag_ipset6", "ipset6_commit");
	nlh = (struct nlmsghdr *)handle->buffer;

	if (nlh->nlmsg_len == 0)
		/* Nothing to do */
		return 0;

	/* Send buffer */
	ret = ipset6_mnl_query(handle);

	nlh->nlmsg_len = 0;

	return 0;
}

static int
build_msg(struct ipset6_handle *handle)
{
	if (NULL == handle) {
        eag_log_err("build_msg input error");
		return -1;
	}
	struct nlmsghdr *nlh = (void *)handle->buffer;
    struct nlattr *nested_data;
    struct nlattr *nested_ip;
	uint32_t lineno = 0;

	eag_log_info("build_msg");
	ipset6_mnl_fill_hdr(handle->cmd, handle->buffer);
	/* protocol */
	mnl_attr_put_u8(nlh, IPSET6_ATTR_PROTOCOL, IPSET6_PROTOCOL);
	/* setname */
    rawdata2attr(nlh, handle->setname, IPSET6_ATTR_SETNAME);
    /* data */
    nested_data = mnl_attr_nest_start(nlh, IPSET6_ATTR_DATA);
    /* ip */
    nested_ip = mnl_attr_nest_start(nlh, IPSET6_ATTR_IP);
    rawdata2attr(nlh, &(handle->ip), IPSET6_ATTR_IP);
    mnl_attr_nest_end(nlh, nested_ip);
    /* lineno */
    rawdata2attr(nlh, &lineno, IPSET6_ATTR_LINENO);
    
    mnl_attr_nest_end(nlh, nested_data);

	return 0;
}

int adt_ip_set(struct ipset6_handle *handle, enum ipset6_cmd cmd, const ip_set_ip_t ip)
{
	int ret = -1;
	
	if (NULL == handle) {
        eag_log_err("adt_ip_set input error");
		return -1;
	}
	memset(handle->buffer, 0, sizeof(handle->buffer));
	memset(&(handle->ip), 0, sizeof(union nf_inet_addr));
	eag_log_debug("eag_ipset6", "adt_ip_set, cmd:%d, ip:%x", cmd, ip);
	handle->cmd = cmd;
	handle->ip.in.s_addr = htonl(ip);

	ret = build_msg(handle);
	if (ret < 0)
		return -1;

	ret = ipset6_commit(handle);

	return ret;
}

int adt_type_get(struct ipset6_handle *handle, const char *setname)
{
	if (NULL == handle || NULL == setname) {
        eag_log_err("adt_type_get input error");
		return -1;
	}
	
	memset(handle->setname, 0, IPSET6_MAXNAMELEN);
	strncpy(handle->setname, setname, IPSET6_MAXNAMELEN - 1);
	handle->setname[IPSET6_MAXNAMELEN - 1] = '\0';
	
	eag_log_debug("eag_ipset6", "adt_type_get, setname:%s", setname);
	build_send_private_msg(handle, IPSET6_CMD_PROTOCOL);

	return build_send_private_msg(handle, IPSET6_CMD_HEADER);
}

int add_ip_to_ipset6_hash(const char *set_name, const ip_set_ip_t ip)
{
	int res, ret;
	
	if (NULL == set_name) {
        eag_log_err("add_ip_to_ipset6_hash setname error");
		return -1;
	}
	eag_log_debug("eag_ipset6", "add_ip_to_ipset6_hash, setname:%s, ip:%x", set_name, ip);

	ret = adt_type_get(handle, set_name);
	if (ret != 0) {
    	eag_log_err("adt_type_get error\n");
		return -1;
	}

	res = adt_ip_set(handle, IPSET6_CMD_ADD, ip);
	if (res != 0) {
		eag_log_err("set_adtip error\n");
		return -1;
	}

	return 0;
}

int del_ip_from_ipset6_hash(const char *set_name, const ip_set_ip_t ip)
{
	int res, ret;

	if (NULL == set_name) {
        eag_log_err("del_ip_from_ipset6_hash setname error");
		return -1;
	}
	eag_log_debug("eag_ipset6", "del_ip_from_ipset6_hash, setname:%s, ip:%x", set_name, ip);

	ret = adt_type_get(handle, set_name);
	if (ret != 0) {
    	eag_log_err("adt_type_get error\n");
		return -1;
	}

	res = adt_ip_set(handle, IPSET6_CMD_DEL, ip);
	if (res != 0) {
		eag_log_err("set_adtip error\n");
		return -1;
	}

	return 0;		
}

int add_user_to_ipset6(const int user_id, const int hansitype,
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

	return add_ip_to_ipset6_hash(set_name, user_ip);	
}

int del_user_from_ipset6(const int user_id, const int hansitype,
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

	return del_ip_from_ipset6_hash(set_name, user_ip);	
}

int add_preauth_user_to_ipset6(const int user_id, const int hansitype,
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

	return add_ip_to_ipset6_hash(set_name, user_ip);	
}

int del_preauth_user_from_ipset6(const int user_id, const int hansitype,
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

	return del_ip_from_ipset6_hash(set_name, user_ip);	
}

int eag_ipset6_exit()
{
	if (NULL == handle) {
		return -1;
	}
	
	if (handle->h)
		mnl_socket_close(handle->h);

	eag_free(handle);
	eag_log_debug("eag_ipset6", "eag_ipset6_exit");
	
	return 0;
}

struct ipset6_handle *eag_ipset6_init()
{
	handle = eag_calloc(1, sizeof(*handle));
	if (!handle) {
        eag_log_err("eag_calloc failed");
		return NULL;
	}
	memset(handle, 0, sizeof(*handle));
	handle->h = mnl_socket_open(NETLINK_NETFILTER);
	if (!handle->h) {
        eag_log_err("mnl_socket_open failed");
		goto free_handle;
	}
	
	if (mnl_socket_bind(handle->h, 0, MNL_SOCKET_AUTOPID) < 0) {
        eag_log_err("mnl_socket_bind failed");
		goto close_nl;
	}
	
	//handle->portid = mnl_socket_get_portid(handle->h);
	handle->seq = time(NULL);
	eag_log_debug("eag_ipset6", "eag_ipset6_init success");
	
	return handle;

close_nl:
	mnl_socket_close(handle->h);
free_handle:
	eag_free(handle);

	return NULL;
}

