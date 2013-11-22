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
#include "session.h"
#include "eag_ipset.h"
#include "eag_ipset6.h"
#include "eag_mem.h" 
#include "eag_log.h"
#include "eag_errcode.h"
#include "eag_interface.h"

#define IPSET6_PROTOCOL         6
#define IPSET6_ATTR_PROTOCOL    1
#define IPSET6_ATTR_SETNAME     2
#define IPSET6_ATTR_IP          1
#define IPSET6_ATTR_IPADDR_IPV4 1
#define IPSET6_ATTR_IPADDR_IPV6 2
#define IPSET6_ATTR_LINENO      9
#define IPSET6_ATTR_DATA        7


#ifndef NFNL_SUBSYS_IPSET
#define NFNL_SUBSYS_IPSET 6
#endif

#ifndef NFNETLINK_V0
#define NFNETLINK_V0      0
#endif

/* Internal data structure for the kernel-userspace communication parameters */
struct ipset6_handle {
	struct mnl_socket *h;		/* the mnl socket */
	unsigned int seq;		/* netlink message sequence number */

    enum ipset6_cmd cmd;
	user_addr_t user_addr;
	char setname[IPSET6_MAXNAMELEN];
	char buffer[PRIVATE_MSG_BUFLEN];
};

static struct ipset6_handle *handle;

struct nfgenmsg {
	uint8_t nfgen_family;
	uint8_t version;
	uint16_t res_id;
};

static char *get_cmd_str(enum ipset6_cmd cmd)
{
	switch (cmd) {
	case IPSET6_CMD_PROTOCOL:
		return "PROTOCOL";
	case IPSET6_CMD_ADD:
		return "ADD";
	case IPSET6_CMD_DEL:
		return "DEL";
	case IPSET6_CMD_HEADER:
		return "HEADER";
	default :
		return "No-support";
	}
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
			eag_log_debug("eag_ipset", "Message header:%s msg:%s len:%d seq:%u\n",
				dir,
				nlh->nlmsg_type == NLMSG_NOOP ? "NOOP" :
				nlh->nlmsg_type == NLMSG_DONE ? "DONE" :
				"OVERRUN",
				len, nlh->nlmsg_seq);
			goto next_msg;
		case NLMSG_ERROR: {
			const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);
 			eag_log_debug("eag_ipset", "Message header:%s msg:ERROR len:%d errcode:%d seq:%u\n",
				dir, len, err->error, nlh->nlmsg_seq);
			goto next_msg;
			}
		default:
			;
		}
		cmd = ipset_get_nlmsg_type(nlh);
		eag_log_debug("eag_ipset", "Message header:%s cmd:%s len:%d flag:%s seq:%u\n",
			dir, get_cmd_str(cmd), len,
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
	eag_log_debug("eag_ipset", "ipset6_mnl_query");

	nlh->nlmsg_seq = ++handle->seq;
	ipset6_debug_msg("sent", nlh, nlh->nlmsg_len);

	if (mnl_socket_sendto(handle->h, nlh, nlh->nlmsg_len) < 0) {
		return -1;
	}
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

	nlh = mnl_nlmsg_put_header(buffer);
	nlh->nlmsg_type = cmd | (NFNL_SUBSYS_IPSET << 8);
    eag_log_debug("eag_ipset", "ipset6_mnl_fill_hdr, cmd: %s", get_cmd_str(cmd));

	switch (cmd) {
	case IPSET6_CMD_ADD:
		nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL;
        break;
	case IPSET6_CMD_DEL:
		nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL;
		break;
	case IPSET6_CMD_HEADER:
		nlh->nlmsg_flags = NLM_F_REQUEST;
		break;
	case IPSET6_CMD_PROTOCOL:
		nlh->nlmsg_flags = NLM_F_REQUEST;
		break;
	default :
		break;
	}

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfg->nfgen_family = AF_INET;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = htons(0);

	return 0;
}

static int
rawdata2attr(struct nlmsghdr *nlh, const void *value, int type)
{
	int alen;
	uint16_t flags = 0;
	uint32_t value1 = 0;
	user_addr_t *user_addr;

	switch (type) {
	case IPSET6_ATTR_SETNAME:
		alen = strlen(value) + 1;
		if (IPSET6_MAXNAMELEN < alen) {
			eag_log_err("setname is too long, out of size: %d", IPSET6_MAXNAMELEN - 1);
			return 1;
		}
		mnl_attr_put(nlh, type | flags, alen, value);
        eag_log_debug("eag_ipset", "add attr: SETNAME");
		break;
	case IPSET6_ATTR_IP:
		user_addr = (user_addr_t *)value;
        if (EAG_IPV4 == user_addr->family) {
			mnl_attr_put(nlh, IPSET6_ATTR_IPADDR_IPV4 | NLA_F_NET_BYTEORDER, 
						sizeof(struct in_addr), (struct in_addr *)&(user_addr->user_ip));
            eag_log_debug("eag_ipset", "add attr: IPV4");
		} else if (EAG_IPV6 == user_addr->family) {
			mnl_attr_put(nlh, IPSET6_ATTR_IPADDR_IPV6 | NLA_F_NET_BYTEORDER, 
						sizeof(struct in6_addr), &(user_addr->user_ipv6));
            eag_log_debug("eag_ipset", "add attr: IPV6");
		}
		break;
	case IPSET6_ATTR_LINENO:
		value1 = htonl(*(const uint32_t *)value);
		mnl_attr_put(nlh, type | NLA_F_NET_BYTEORDER, sizeof(uint32_t), &value1);
        eag_log_debug("eag_ipset", "add attr: LINENO");
		break;
	default:
		break;
	}

	return 0;
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
	eag_log_debug("eag_ipset", "ipset6_commit");
	nlh = (struct nlmsghdr *)handle->buffer;

	if (nlh->nlmsg_len == 0) {
		/* Nothing to do */
		return 0;
	}
	/* Send buffer */
	ret = ipset6_mnl_query(handle);

	nlh->nlmsg_len = 0;

	return ret;
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
	
	memset(handle->buffer, 0, sizeof(handle->buffer));

	eag_log_debug("eag_ipset", "build_msg");
	ipset6_mnl_fill_hdr(handle->cmd, handle->buffer);
	/* protocol */
	mnl_attr_put_u8(nlh, IPSET6_ATTR_PROTOCOL, IPSET6_PROTOCOL);
    eag_log_debug("eag_ipset", "add attr: PROTOCOL");
	/* setname */
    rawdata2attr(nlh, handle->setname, IPSET6_ATTR_SETNAME);
    /* data */
    nested_data = mnl_attr_nest_start(nlh, IPSET6_ATTR_DATA);
    /* user_ip */
    nested_ip = mnl_attr_nest_start(nlh, IPSET6_ATTR_IP);
    rawdata2attr(nlh, &(handle->user_addr), IPSET6_ATTR_IP);
    mnl_attr_nest_end(nlh, nested_ip);
    /* lineno */
    rawdata2attr(nlh, &lineno, IPSET6_ATTR_LINENO);
    
    mnl_attr_nest_end(nlh, nested_data);

	return 0;
}

int
adt_ip_set(struct ipset6_handle *handle)
{
	int ret = -1;
	
	if (NULL == handle) {
        eag_log_err("adt_ip_set input error");
		return -1;
	}
	
    eag_log_debug("eag_ipset", "adt_ip_set");
	ret = build_msg(handle);
	if (ret < 0) {
        eag_log_err("build_msg error");
		return -1;
	}
	ret = ipset6_commit(handle);

	return ret;
}

static int
build_send_private_msg(struct ipset6_handle *handle, enum ipset6_cmd cmd)
{
	if (NULL == handle) {
		return -1;
	}
	struct nlmsghdr *nlh = (void *)handle->buffer;
	int ret;

	memset(handle->buffer, 0, sizeof(handle->buffer));
	eag_log_debug("eag_ipset", 
			"build_send_private_msg, setname: %s cmd: %s", 
			handle->setname, get_cmd_str(cmd));

	/* Initialize header */
	ipset6_mnl_fill_hdr(cmd, handle->buffer);

	mnl_attr_put_u8(nlh, IPSET6_ATTR_PROTOCOL, IPSET6_PROTOCOL);
    eag_log_debug("eag_ipset", "add attr: PROTOCOL");
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

int
adt_type_get(struct ipset6_handle *handle)
{
	if (NULL == handle) {
        eag_log_err("adt_type_get input error");
		return -1;
	}
    eag_log_debug("eag_ipset", "adt_type_get");
	build_send_private_msg(handle, IPSET6_CMD_PROTOCOL);

	return build_send_private_msg(handle, IPSET6_CMD_HEADER);
}

int
set_ipx_in_ipset6_hash(struct ipset6_handle *handle)
{
	int res, ret;
	char user_ipstr[IPX_LEN] = "";
	
	if (NULL == handle) {
    	eag_log_err("set_ipx_in_ipset6_hash input error\n");
		return -1;
	}
	ret = adt_type_get(handle);
	if (ret != 0) {
    	eag_log_err("adt_type_get error\n");
		return -1;
	}

	res = adt_ip_set(handle);
	if (res != 0) {
		eag_log_err("adt_ip_set error\n");
		return -1;
	}
	
	ipx2str(&(handle->user_addr), user_ipstr, sizeof(user_ipstr));
	eag_log_info("%s ip:%s in ipset6 hash setname:%s success", 
					get_cmd_str(handle->cmd), user_ipstr, handle->setname);
	
	return 0;		
}

int
set_ipv4_in_ipset6( struct ipset6_handle *handle,
					uint32_t user_ip,
					char *set_name,
					enum ipset6_cmd cmd )
{
    char user_ipstr[IPX_LEN];
	if (NULL == handle
		|| NULL == set_name) {
		eag_log_err("set_ipv4_in_ipset6 input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	memset(&(handle->user_addr), 0, sizeof(user_addr_t));
	handle->cmd = cmd;
	handle->user_addr.family = EAG_IPV4;
	handle->user_addr.user_ip = htonl(user_ip); // Network byte order
    if (IPSET6_MAXNAMELEN < strlen(set_name) + 1) {
        eag_log_err("set_ipv4_in_ipset6 setname length out of size:%d", 
                IPSET6_MAXNAMELEN - 1);
        return EAG_ERR_INPUT_PARAM_ERR;
    }
    if (IPSET6_CMD_PROTOCOL > cmd ||
        IPSET6_CMD_HEADER < cmd) {
        eag_log_err("set_ipv4_in_ipset6 cmd:%d is not support", cmd);
        return EAG_ERR_INPUT_PARAM_ERR;
    }
    
    memset(handle->setname, 0, IPSET6_MAXNAMELEN);
    strncpy(handle->setname, set_name, IPSET6_MAXNAMELEN - 1);
    handle->setname[IPSET6_MAXNAMELEN - 1] = '\0';
    
	ipx2str(&(handle->user_addr), user_ipstr, sizeof(user_ipstr));
	eag_log_info("set_ipv4_in_ipset6 %s ip:%s in ipset6 setname:%s", 
                get_cmd_str(cmd), user_ipstr, set_name);    
    return set_ipx_in_ipset6_hash(handle);
}

int
set_ipv6_in_ipset6( struct ipset6_handle *handle,
					struct in6_addr *user_ipv6,
					char *set_name,
					enum ipset6_cmd cmd)
{
    char user_ipstr[IPX_LEN];
	if (NULL == handle
		|| NULL == user_ipv6
		|| NULL == set_name) {
		eag_log_err("set_ipv6_in_ipset6 input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
    
	memset(&(handle->user_addr), 0, sizeof(user_addr_t));
	handle->cmd = cmd;
	handle->user_addr.family = EAG_IPV6;
    handle->user_addr.user_ipv6 = *user_ipv6;
    if (IPSET6_MAXNAMELEN < strlen(set_name) + 1) {
        eag_log_err("set_ipv6_in_ipset6 setname length out of size:%d", 
                IPSET6_MAXNAMELEN - 1);
        return EAG_ERR_INPUT_PARAM_ERR;
    }
    if (IPSET6_CMD_PROTOCOL > cmd ||
        IPSET6_CMD_HEADER < cmd) {
        eag_log_err("set_ipv6_in_ipset6 cmd:%d is not support", cmd);
        return EAG_ERR_INPUT_PARAM_ERR;
    }
    
    memset(handle->setname, 0, IPSET6_MAXNAMELEN);
    strncpy(handle->setname, set_name, IPSET6_MAXNAMELEN - 1);
    handle->setname[IPSET6_MAXNAMELEN - 1] = '\0';
    
	ipx2str(&(handle->user_addr), user_ipstr, sizeof(user_ipstr));
	eag_log_info("set_ipv6_in_ipset6 %s ip:%s in ipset6 setname:%s", 
                get_cmd_str(cmd), user_ipstr, set_name);
    return set_ipx_in_ipset6_hash(handle);
}

int
set_user_in_ipset6( const int user_id, 
					const int hansitype,
					user_addr_t *user_addr,  
					enum ipset6_cmd cmd )
{
	char ipv4_setname[IPSET6_MAXNAMELEN];
	char ipv6_setname[IPSET6_MAXNAMELEN];
    char user_ipstr[IPX_LEN];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| NULL == user_addr
		|| 0 == memcmp_ipx(user_addr, NULL)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d", 
				user_id, hansitype);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	eag_log_info("%s ip:%s in ipset6", get_cmd_str(cmd), user_ipstr);
    memset(ipv4_setname, 0, sizeof(ipv4_setname));
    memset(ipv6_setname, 0, sizeof(ipv6_setname));
    
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
    snprintf(ipv4_setname, sizeof(ipv4_setname)-1,
        	"CP_%s%d_AUTHORIZED_IPV4_SET", cpid_prefix, user_id);
	snprintf(ipv6_setname, sizeof(ipv6_setname)-1,
        	"CP_%s%d_AUTHORIZED_IPV6_SET", cpid_prefix, user_id);

	if (EAG_IPV4 == user_addr->family) {
		set_ipv4_in_ipset6(handle, user_addr->user_ip, ipv4_setname, cmd);
 	} else if (EAG_IPV6 == user_addr->family) {
		set_ipv6_in_ipset6(handle, &(user_addr->user_ipv6), ipv6_setname, cmd);
	} else if (EAG_MIX == user_addr->family) {
		set_ipv4_in_ipset6(handle, user_addr->user_ip, ipv4_setname, cmd);
		set_ipv6_in_ipset6(handle, &(user_addr->user_ipv6), ipv6_setname, cmd);
	}

	return 0;
}

int
set_preauth_user_in_ipset6( const int user_id, 
							const int hansitype,
							user_addr_t *user_addr, 
							enum ipset6_cmd cmd )
{
	char ipv4_setname[IPSET6_MAXNAMELEN];
	char ipv6_setname[IPSET6_MAXNAMELEN];
    char user_ipstr[IPX_LEN];
	char *cpid_prefix = NULL;

	if ((user_id < 0 || user_id > MAX_CAPTIVE_ID)
		|| (hansitype != 0 && hansitype != 1)
		|| NULL == user_addr
		|| 0 == memcmp_ipx(user_addr, NULL)) {
		eag_log_debug("eag_ipset",
				"prepare for set error.user_id=%d hansitype=%d", 
				user_id, hansitype);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
    eag_log_info("%s preauth ip:%s in ipset6", get_cmd_str(cmd), user_ipstr);
    memset(ipv4_setname, 0, sizeof(ipv4_setname));
    memset(ipv6_setname, 0, sizeof(ipv6_setname));
    
	cpid_prefix = (HANSI_LOCAL == hansitype)?"L":"R";
    snprintf(ipv4_setname, sizeof(ipv4_setname)-1,
            "MAC_PRE_%s%d_AUTH_IPV4_SET", cpid_prefix, user_id);
	snprintf(ipv6_setname, sizeof(ipv6_setname)-1,
			"MAC_PRE_%s%d_AUTH_IPV6_SET", cpid_prefix, user_id);

	if (EAG_IPV4 == user_addr->family) {
		set_ipv4_in_ipset6(handle, user_addr->user_ip, ipv4_setname, cmd);
 	} else if (EAG_IPV6 == user_addr->family) {
		set_ipv6_in_ipset6(handle, &(user_addr->user_ipv6), ipv6_setname, cmd);
	} else if (EAG_MIX == user_addr->family) {
		set_ipv4_in_ipset6(handle, user_addr->user_ip, ipv4_setname, cmd);
		set_ipv6_in_ipset6(handle, &(user_addr->user_ipv6), ipv6_setname, cmd);
	}
	
	return 0;
}

int
eag_ipset6_exit()
{
	if (NULL == handle) {
		return -1;
	}
	
	if (handle->h)
		mnl_socket_close(handle->h);

	eag_free(handle);
	handle = NULL;
	eag_log_debug("eag_ipset", "eag_ipset6_exit");
	
	return 0;
}

int
eag_ipset6_init()
{
	handle = eag_calloc(1, sizeof(*handle));
	if (!handle) {
        eag_log_err("eag_calloc failed");
		return -1;
	}
	memset(handle, 0, sizeof(*handle));
	handle->h = mnl_socket_open(NETLINK_NETFILTER);
	if (!handle->h) {
        eag_log_err("mnl_socket_open failed");
		goto free_handle;
	}
	/*
	if (0 != set_nonblocking(handle->h->fd)){
		eag_log_err("mnl_socket set socket nonblocking failed");
		goto close_nl;
	}
	*/
	if (mnl_socket_bind(handle->h, 0, MNL_SOCKET_AUTOPID) < 0) {
        eag_log_err("mnl_socket_bind failed");
		goto close_nl;
	}
	
	//handle->portid = mnl_socket_get_portid(handle->h);
	handle->seq = time(NULL);
	eag_log_debug("eag_ipset", "eag_ipset6_init success");
	
	return 0;

close_nl:
	mnl_socket_close(handle->h);
free_handle:
	eag_free(handle);
	handle = NULL;

	return -1;
}

