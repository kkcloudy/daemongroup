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
* dhcp_snp_netlink.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping netlink interface.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <sysdef/returncode.h>

#include "dhcp_snp_log.h"
#include "dhcp_snp_listener.h"
#include "dhcp_snp_netlink.h"

#define DHCPSNP_RD_DEBUG	1

/* netlink handle */
struct dhcp_rtnl_handle *dhcpsnp_rtnl = NULL;

/* ip neigh action flag */
int dhcpsnp_ipneigh_on = DHCPSNP_NL_TRUE;

/**********************************************************************************
 * addattr_l
 *	Add attribute to netlink message
 *
 *	INPUT:
 *		n 	- message header
 *		maxlen - maximum length
 *		type - message type
 *		data - message data
 *		alen - message data length
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0		- success
 *	 	-1		- fail
 **********************************************************************************/
int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		syslog_ax_dhcp_snp_err("addattr_l ERROR: message exceeded bound of %d\n",maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}
/**********************************************************************************
 * addattr32
 *	Add attribute to netlink message
 *
 *	INPUT:
 *		n 	- message header
 *		maxlen - maximum length
 *		type - message type
 *		data - message data
 *		alen - message data length
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		0		- success
 *	 	-1		- fail
 **********************************************************************************/

int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
	int len = RTA_LENGTH(4);
	struct rtattr *rta;
	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen) {
		log_error("addattr32: Error! max allowed bound %d exceeded\n",maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}

/**********************************************************************************
 * dhcp_snp_netlink_init
 *	Initialize DHCP snooping netlink socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_netlink_init(void)
{
	struct dhcp_rtnl_handle *rtnl = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK, fd = -1;
	socklen_t addr_len;
	int sndbuf = 32768;
	int rcvbuf = 32768;
	struct sockaddr_nl	local;

	if(!dhcpsnp_rtnl) {
		rtnl = (struct dhcp_rtnl_handle*)malloc(sizeof(struct dhcp_rtnl_handle));
		if(!rtnl) {
			syslog_ax_dhcp_snp_err("init netlink error(out of memory)!\n");\
			return DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL;
		}
		memset(rtnl, 0, sizeof(struct dhcp_rtnl_handle));
		dhcpsnp_rtnl = rtnl;
	}

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		syslog_ax_dhcp_snp_err("cannot open netlink socket");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) {
		syslog_ax_dhcp_snp_err("set send buffer option error");
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf)) < 0) {
		syslog_ax_dhcp_snp_err("set rcv buffer option error");
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = 0;

	if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
		syslog_ax_dhcp_snp_err("Cannot bind netlink socket");
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	addr_len = sizeof(local);
	if (getsockname(fd, (struct sockaddr*)&local, &addr_len) < 0) {
		syslog_ax_dhcp_snp_err("Cannot getsockname");
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (addr_len != sizeof(local)) {
		syslog_ax_dhcp_snp_err("Wrong address length %d\n", addr_len);
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (local.nl_family != AF_NETLINK) {
		syslog_ax_dhcp_snp_err("Wrong address family %d\n", local.nl_family);
		if(fd){
			close(fd);	
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dhcpsnp_rtnl->fd = fd;
	syslog_ax_dhcp_snp_dbg("initialize dhcp snooping netlink fd %d!\n", dhcpsnp_rtnl->fd);
	
	return ret;
}

/**********************************************************************************
 * dhcp_snp_netlink_init
 *	Initialize DHCP snooping netlink socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_netlink_do_ipneigh
(
	enum dhcpsnp_rtnl_ipneigh_op cmd,
	unsigned int ifindex,
	unsigned int ipaddr,
	unsigned char *mac
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK, status = 0;;
	int msg_type = 0, flags = 0;
	char ifname[IF_NAMESIZE] = {0};
	struct {
		struct nlmsghdr 	n;
		struct ndmsg 		ndm;
		char   			buf[256];
	} req;
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = (void*)&(req.n),
		.iov_len = req.n.nlmsg_len
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = 0;
	nladdr.nl_groups = 0;

	if(!dhcpsnp_rtnl || FD_INVALID(dhcpsnp_rtnl->fd)) {
		ret = dhcp_snp_netlink_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("init netlink error when %s ip neigh!\n", DCHPSNP_RTNL_IPNEIGH_DESC(cmd));
			return ret;
		}
	}

	if(DHCPSNP_NL_FALSE == dhcpsnp_ipneigh_on) {
		syslog_ax_dhcp_snp_warn("%s ip neigh %d.%d.%d.%d %02x:%02x:%02x:%02x:%02x:%02x but control off, ignore!\n", \
							DCHPSNP_RTNL_IPNEIGH_DESC(cmd), (ipaddr>>24)&0xFF, (ipaddr>>16)&0xFF, \
							(ipaddr>>8)&0xFF, ipaddr&0xFF, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		return ret;
	}
	
	if(!if_indextoname(ifindex, ifname)) {
		syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error when %s ip neigh!\n", \
								ifindex, DCHPSNP_RTNL_IPNEIGH_DESC(cmd));
		return ret;
	}

	if (dhcp_snp_u32ip_check(ipaddr)) {
		log_error("%s: invalid ip address %s %s\n", __func__, 
			u32ip2str(ipaddr), mac2str(mac));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	switch(cmd) {
		default:
		case DHCPSNP_RTNL_IPNEIGH_UPDATE_E:
			break;
		case DHCPSNP_RTNL_IPNEIGH_ADD_E:
			flags = RTNL_IPNEIGH_ADD_FLAGS;
			msg_type = DHCPSNP_RTNL_CMD_IPNEIGH_ADD;
			break;
		case DHCPSNP_RTNL_IPNEIGH_DEL_E:
			flags = RTNL_IPNEIGH_DEL_FLAGS;
			msg_type = DHCPSNP_RTNL_CMD_IPNEIGH_DEL;
			break;
	}
	
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
	req.n.nlmsg_flags = flags;
	req.n.nlmsg_type = msg_type;
	req.ndm.ndm_family = DHCPSNP_RTNL_IPNEIGH_FAMILY;
	req.ndm.ndm_state = NUD_PERMANENT;

	/* add ip address */
	addattr_l(&req.n, sizeof(req), NDA_DST, &ipaddr, 4);
	/* add mac */
	addattr_l(&req.n, sizeof(req), NDA_LLADDR, mac, 6);
	/* ifindex */
	req.ndm.ndm_ifindex = ifindex;
	/* update iov length */
	iov.iov_len = req.n.nlmsg_len;
	
	syslog_ax_dhcp_snp_dbg("ip neigh op %s %s ip %d.%d.%d.%d %02x:%02x:%02x:%02x:%02x:%02x\n",  \
				DCHPSNP_RTNL_IPNEIGH_DESC(cmd), ifname, (ipaddr>>24)&0xFF, (ipaddr>>16)&0xFF, \
				(ipaddr>>8)&0xFF, ipaddr&0xFF, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	status = sendmsg(dhcpsnp_rtnl->fd, &msg, 0);
	if(status < 0) {
		syslog_ax_dhcp_snp_dbg("send netlink message error %s\n", strerror(errno));
		ret = DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	return ret;
}

/**********************************************************************************
 * dhcp_snp_netlink_add_static_route
 *	Initialize DHCP snooping netlink socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_netlink_add_static_route
(
	enum dhcpsnp_rtnl_static_route_op cmd,
	unsigned int ifindex,
	unsigned int ipaddr
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK, status = 0;;
	int msg_type = 0, flags = 0;
	char ifname[IF_NAMESIZE] = {0};
	struct {
			struct nlmsghdr 	n;
			struct rtmsg		r;
			char			buf[1024];
		} req;
	
	if(!dhcpsnp_rtnl || FD_INVALID(dhcpsnp_rtnl->fd)) {
		ret = dhcp_snp_netlink_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("init netlink error when %s ip neigh!\n", DCHPSNP_RTNL_IPNEIGH_DESC(cmd));
			return ret;
		}
	}

	
	if(!if_indextoname(ifindex, ifname)) {
		syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error when %s ip neigh!\n", \
								ifindex, DCHPSNP_RTNL_IPNEIGH_DESC(cmd));
		return ret;
	}

	if (dhcp_snp_u32ip_check(ipaddr)) {
		log_error("%s: invalid ip address %s\n", __func__, 
			u32ip2str(ipaddr));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	switch(cmd) {
		default:
		case DHCPSNP_RTNL_STATIC_ROUTE_UPDATE_E:
			break;
		case DHCPSNP_RTNL_STATIC_ROUTE_ADD_E:
			flags = NLM_F_CREATE|NLM_F_EXCL;
			msg_type = RTM_NEWROUTE;
			break;
		case DHCPSNP_RTNL_STATIC_ROUTE_DEL_E:
			flags = 0;
			msg_type = RTM_DELROUTE;
			break;
	}
	
	memset(&req, 0, sizeof(req));
	
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST|flags;
	req.n.nlmsg_type = msg_type;
	req.r.rtm_family = AF_INET;  
	req.r.rtm_table = 254;        
	req.r.rtm_scope = 255;			
	req.r.rtm_dst_len = 32;  //32 mask

	if (cmd != DHCPSNP_RTNL_STATIC_ROUTE_DEL_E) {
		req.r.rtm_protocol = RTPROT_BOOT;
		req.r.rtm_scope = RT_SCOPE_UNIVERSE;
		req.r.rtm_type = RTN_UNICAST;
	}


	/* add ip address */	
	addattr_l(&req.n, sizeof(req), RTA_DST, &ipaddr, 4);
	/* ifindex */
	addattr32(&req.n, sizeof(req), RTA_OIF, ifindex);
	/* update iov length */
	//iov.iov_len = req.n.nlmsg_len;

	if (cmd == DHCPSNP_RTNL_STATIC_ROUTE_DEL_E)
		req.r.rtm_scope = RT_SCOPE_NOWHERE;
	else
		req.r.rtm_scope = RT_SCOPE_LINK;
	
	syslog_ax_dhcp_snp_dbg("host route op %s ifname %s ip %d.%d.%d.%d \n",  \
				DCHPSNP_RTNL_ADD_STATIC_ROUTE_DESC(cmd), ifname, (ipaddr>>24)&0xFF, (ipaddr>>16)&0xFF, \
				(ipaddr>>8)&0xFF, ipaddr&0xFF);

	struct sockaddr_nl nladdr;
		struct iovec iov = {
			.iov_base = (void*) &req.n,
			.iov_len = req.n.nlmsg_len
		};
		struct msghdr msg = {
			.msg_name = &nladdr,
			.msg_namelen = sizeof(nladdr),
			.msg_iov = &iov,
			.msg_iovlen = 1,
		};
		req.n.nlmsg_flags |= NLM_F_ACK;
	
	
		memset(&nladdr, 0, sizeof(nladdr));
		nladdr.nl_family = AF_NETLINK;
		nladdr.nl_pid = 0;
		nladdr.nl_groups = 0;
	
		req.n.nlmsg_seq = 0;
	status = sendmsg(dhcpsnp_rtnl->fd, &msg, 0);
	if(status < 0) {
		syslog_ax_dhcp_snp_dbg("send netlink message error %s\n", strerror(errno));
		ret = DHCP_SNP_RETURN_CODE_ERROR;
	}

	return ret;
}


/**********************************************************************************
 * dhcp_snp_netlink_ipneigh_ctrl
 *	Control DHCP snooping ip neigh operation
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_netlink_ipneigh_ctrl
(
	unsigned char enable
)
{
	if(enable) {
		dhcpsnp_ipneigh_on = DHCPSNP_NL_TRUE;
	}
	else {
		dhcpsnp_ipneigh_on = DHCPSNP_NL_FALSE;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}
#ifdef __cplusplus
}
#endif
