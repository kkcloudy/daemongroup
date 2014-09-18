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
* had_ipaddr.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in HAD module for ip address op.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/* system include */
#include <stdio.h>
#include <assert.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>

/* local include */
#include "had_libnetlink.h"
#include "had_ipaddr.h"
#include "had_log.h"

typedef struct {
	int		ifindex;
	uint32_t	*addr;
	int		max_elem;
	int		nb_elem;
} iplist_ctx;

/****************************************************************
 NAME	: print_addr				00/06/02 18:24:09
 AIM	: 
 REMARK	:
****************************************************************/
static int had_get_addrinfo(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	struct ifaddrmsg *ifa 	= NLMSG_DATA(n);
	int		len	= n->nlmsg_len;
	iplist_ctx	*ctx	= (iplist_ctx *)arg;
	struct rtattr 	*rta_tb[IFA_MAX+1];
	/* sanity check */
	len -= NLMSG_LENGTH(sizeof(*ifa));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}
	/* check the message type */
	if (n->nlmsg_type != RTM_NEWADDR )
		return 0;
	/* check it is ipv4 */
	if( ifa->ifa_family != AF_INET)
		return 0;
	/* check it is the good interface */
	if( ifa->ifa_index != ctx->ifindex )
		return 0;
		
	/* parse the attribute */
	memset(rta_tb, 0, sizeof(rta_tb));
	had_parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa), len);

	if (!rta_tb[IFA_LOCAL])
		rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];

	if (rta_tb[IFA_LOCAL]) {
		u_char *src = RTA_DATA(rta_tb[IFA_LOCAL]);
		if( ctx->nb_elem >= ctx->max_elem )
			return 0;
		ctx->addr[ctx->nb_elem++] = 	(src[0]<<24) + (src[1]<<16) +
						(src[2]<<8) + src[3];
	}
	return 0;
}


/****************************************************************
 NAME	: had_ipaddr_list				00/06/02 20:02:23
 AIM	: 
 REMARK	:
****************************************************************/
int had_ipaddr_list( int ifindex, uint32_t *array, int max_elem )
{
	struct rtnl_handle	rth;
	iplist_ctx	ctx;
	/* init the struct */
	ctx.ifindex	= ifindex;
	ctx.addr	= array;
	ctx.max_elem	= max_elem;
	ctx.nb_elem	= 0;
	/* open the rtnetlink socket */
	if( had_rtnl_open( &rth, 0) )
		return -1;
	/* send the request */
	if (had_rtnl_wilddump_request(&rth, AF_INET, RTM_GETADDR) < 0) {
		perror("Cannot send dump request");
		had_rtnl_close(&rth);
		return -1;
	}
	/* parse the answer */
	if (had_rtnl_dump_filter(&rth, had_get_addrinfo, &ctx, NULL, NULL) < 0) {
		fprintf(stderr, "Flush terminated\n");
		had_rtnl_close(&rth);
		return -1;
	}
	
	/* to close the socket */
 	had_rtnl_close( &rth );
	
	return ctx.nb_elem;
}

/****************************************************************
 NAME	: had_ipv6addr_list				2014-07-03 16:10:00
 AIM	: 
 REMARK	:
****************************************************************/
int had_ipv6addr_list( int ifindex, uint32_t *array, int max_elem )
{
	struct rtnl_handle	rth;
	iplist_ctx	ctx;
	/* init the struct */
	ctx.ifindex	= ifindex;
	ctx.addr	= array;
	ctx.max_elem	= max_elem;
	ctx.nb_elem	= 0;
	/* open the rtnetlink socket */
	if( had_rtnl_open( &rth, 0) )
		return -1;
	/* send the request */
	if (had_rtnl_wilddump_request(&rth, AF_INET, RTM_GETADDR) < 0) {
		perror("Cannot send dump request");
		had_rtnl_close(&rth);
		return -1;
	}
	/* parse the answer */
	if (had_rtnl_dump_filter(&rth, had_get_addrinfo, &ctx, NULL, NULL) < 0) {
		fprintf(stderr, "Flush terminated\n");
		had_rtnl_close(&rth);
		return -1;
	}
	
	/* to close the socket */
 	had_rtnl_close( &rth );
	
	return ctx.nb_elem;
}

/****************************************************************
 NAME	: had_ipaddr_op				00/06/02 23:00:58
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ipaddr_op( int ifindex, uint32_t addr, uint32_t mask,int addF )
{
	if(ifindex < 0){
		vrrp_syslog_error("%s,%d,err ifindex:%d.\n",__func__,__LINE__,ifindex);
		return -1;
	}
	struct rtnl_handle	rth;
	struct {
		struct nlmsghdr 	n;
		struct ifaddrmsg 	ifa;
		char   			buf[256];
	} req;
	char name[IF_NAMESIZE] = {0};
	
	vrrp_syslog_dbg("start ip op: ifindex %d,ipaddr %02x,%s\n",ifindex,addr,addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len		= NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags	= NLM_F_REQUEST;
	req.n.nlmsg_type	= addF ? RTM_NEWADDR : RTM_DELADDR;
	req.ifa.ifa_family	= AF_INET;
	req.ifa.ifa_index	= ifindex;
	req.ifa.ifa_prefixlen	= mask; /*virtual router ip addr be with 32 masklen*/
	
	addr = htonl( addr );
	had_addattr_l(&req.n, sizeof(req), IFA_LOCAL, &addr, sizeof(addr) );

	if (had_rtnl_open(&rth, 0) < 0){
        vrrp_syslog_error("open netlink socket failed when %s ip %#x on %s!\n", \
						addF ? "add" : "delete", addr, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
	}
	if (had_rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
        vrrp_syslog_error("netlink talk failed when %s ip %#x on %s!\n", \
						addF ? "add" : "delete", addr, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
    }
	
	/* to close the clocket */
 	had_rtnl_close( &rth );

	return(0);
}

/****************************************************************
 NAME	: had_ipv6addr_op
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ipv6addr_op( int ifindex, struct in6_addr *addr, uint32_t mask,int addF )
{
	if(ifindex < 0){
		vrrp_syslog_error("%s,%d,err ifindex:%d.\n",__func__,__LINE__,ifindex);
		return -1;
	}
	struct rtnl_handle	rth;
	struct {
		struct nlmsghdr 	n;
		struct ifaddrmsg 	ifa;
		char   			buf[256];
	} req;
	char name[IF_NAMESIZE] = {0};

	vrrp_syslog_info("had_ipv6addr_op:ifindex %d,ip6addr "NIP6QUAD_FMT",%s \n",
		            ifindex,
		            NIP6QUAD(*addr),
		            addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len		= NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags	= NLM_F_REQUEST;
	req.n.nlmsg_type	= addF ? RTM_NEWADDR : RTM_DELADDR;
	req.ifa.ifa_family	= AF_INET6;
	req.ifa.ifa_index	= ifindex;
	req.ifa.ifa_prefixlen	= 128; /*virtual router ipv6 addr be with 128 masklen*/
	
	
	had_addattr_l(&req.n, sizeof(req), IFA_LOCAL, addr, sizeof(struct in6_addr) );

	if (had_rtnl_open(&rth, 0) < 0){
        vrrp_syslog_error("error: open netlink socket failed when %s ip "NIP6QUAD_FMT" on %s!\n", \
						addF ? "add" : "delete", NIP6QUAD((*addr)), if_indextoname(ifindex, name) ? name : "nil");
		return -1;
	}
	if (had_rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
        vrrp_syslog_error("error: netlink talk failed when %s ip "NIP6QUAD_FMT" on %s!\n", \
						addF ? "add" : "delete", NIP6QUAD((*addr)), if_indextoname(ifindex, name) ? name : "nil");
		return -1;
    }
	
	/* to close the clocket */
 	had_rtnl_close( &rth );

	return(0);
}

/****************************************************************
 NAME	: had_ip6addr_op_withmask				2014.8.27 16:21:00
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ip6addr_op_withmask( int ifindex, struct in6_addr *addr, int prefix_length,int addF )
{
	struct rtnl_handle	rth;
	struct {
		struct nlmsghdr 	n;
		struct ifaddrmsg 	ifa;
		char   			buf[256];
	} req;
	char name[IF_NAMESIZE] = {0};
	
	//vrrp_syslog_dbg("start ip6 op: ifindex %d,ip6addr "NIP6QUAD_FMT",%s\n",ifindex,NIP6QUAD((*addr)),addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	vrrp_syslog_info("had_ip6addr_op_withmask: ifindex %d,ip6addr "NIP6QUAD_FMT",%s\n",ifindex,NIP6QUAD((*addr)),addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len		= NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags	= NLM_F_REQUEST;
	req.n.nlmsg_type	= addF ? RTM_NEWADDR : RTM_DELADDR;
	req.ifa.ifa_family	= AF_INET6;
	req.ifa.ifa_index	= ifindex;
	req.ifa.ifa_prefixlen	= prefix_length;
	
	had_addattr_l(&req.n, sizeof(req), IFA_LOCAL, addr, sizeof(struct in6_addr) );

	if (had_rtnl_open(&rth, 0) < 0){
		vrrp_syslog_error("had_ip6addr_op_withmask: open netlink socket failed when %s ip6 "NIP6QUAD_FMT"/%d on %s!\n", \
						addF ? "add" : "delete", NIP6QUAD((*addr)),prefix_length,if_indextoname(ifindex, name) ? name : "nil");

		return -1;
	}
	if (had_rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
		vrrp_syslog_error("had_ip6addr_op_withmask: netlink talk failed when %s ip6 "NIP6QUAD_FMT"/%d on %s!\n", \
						addF ? "add" : "delete", NIP6QUAD((*addr)),prefix_length,if_indextoname(ifindex, name) ? name : "nil");

		return -1;
    }
	
	/* to close the clocket */
 	had_rtnl_close( &rth );

	return(0);
}

/****************************************************************
 NAME	: had_ipaddr_op_withmask				00/06/02 23:00:58
 AIM	: add or remove 
 REMARK	:
****************************************************************/
int had_ipaddr_op_withmask( int ifindex, uint32_t addr, int mask,int addF )
{
	struct rtnl_handle	rth;
	struct {
		struct nlmsghdr 	n;
		struct ifaddrmsg 	ifa;
		char   			buf[256];
	} req;
	char name[IF_NAMESIZE] = {0};
	
	vrrp_syslog_dbg("start ip op: ifindex %d,ipaddr %02x,%s\n",ifindex,addr,addF ? "RTM_NEWADDR" : "RTM_DELADDR");
	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len		= NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags	= NLM_F_REQUEST;
	req.n.nlmsg_type	= addF ? RTM_NEWADDR : RTM_DELADDR;
	req.ifa.ifa_family	= AF_INET;
	req.ifa.ifa_index	= ifindex;
	req.ifa.ifa_prefixlen	= mask;
	
	addr = htonl( addr );
	had_addattr_l(&req.n, sizeof(req), IFA_LOCAL, &addr, sizeof(addr) );

	if (had_rtnl_open(&rth, 0) < 0){
        vrrp_syslog_error("open netlink socket failed when %s ip %#x mask %d on %s!\n", \
						addF ? "add" : "delete", addr, mask, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
	}
	if (had_rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
        vrrp_syslog_error("netlink talk failed when %s ip %#x mask %d on %s!\n", \
						addF ? "add" : "delete", addr, mask, if_indextoname(ifindex, name) ? name : "nil");
		return -1;
    }
	
	/* to close the clocket */
 	had_rtnl_close( &rth );

	return(0);
}
#ifdef __cplusplus
}
#endif
