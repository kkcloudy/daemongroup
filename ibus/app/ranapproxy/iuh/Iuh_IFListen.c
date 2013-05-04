#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include "iuh/Iuh.h"
#include "Iuh_IFListen.h"
#include "Iuh_Stevens.h"

#define HAVE_IPV6 1  //net link listen ipv6 addr change
struct nlsock netlink = {
	-1, 0, {
0}, "netlink-listen"};		/* kernel messages */

struct nlsock netlink_cmd = {
	-1, 0, {
0}, "netlink-cmd"};		/* command channel */


const char *nexthop_types_desc[] = {
	"none",
	"Directly connected",
	"Interface route",
	"IPv4 nexthop",
	"IPv4 nexthop with ifindex",
	"IPv4 nexthop with ifname",
	"IPv6 nexthop",
	"IPv6 nexthop with ifindex",
	"IPv6 nexthop with ifname",
	"Null0 nexthop",
};
const char *
safe_strerror(int errnum)
{
  const char *s = strerror(errnum);
  return (s != NULL) ? s : "Unknown error";
}
const char *
inet_ntop (int family, const void *addrptr, char *strptr, size_t len)
{
  unsigned char *p = (unsigned char *) addrptr;

  if (family == AF_INET) 
    {
      char temp[256];

      snprintf(temp, sizeof(temp), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

      if (strnlen(temp,256) >= len) 
	{
	  errno = ENOSPC;
	  return NULL;
	}
      strncpy(strptr, temp, strnlen(temp, 256));
      return strptr;
  }
  
  if (family == AF_INET6) 
  {
      char temp[256];

//	  	unsigned *q = temp;
//		int i = 1;
		snprintf(temp, sizeof(temp), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",\
		p[0], p[1], p[2], p[3],p[4], p[5], p[6], p[7],p[8], p[9], p[10], p[11],p[12], p[13], p[14], p[15]);
		
      if (strnlen(temp,256) >= len) 
	  {
		  errno = ENOSPC;
		  return NULL;
	  }
      strncpy(strptr, temp, strnlen(temp, 256));
      return strptr;
  }

  errno = EAFNOSUPPORT;
  return NULL;
}

static void
netlink_parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta,
		     int len)
{
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
}


int
netlink_parse_info(int (*filter) (struct sockaddr_nl *, struct nlmsghdr *),
		   struct nlsock *nl)
{
	int status;
	int ret = 0;
	int error;

	while (1) {
		char buf[16384];
		struct iovec iov = { buf, sizeof buf };
		struct sockaddr_nl snl;
		struct msghdr msg =
		    { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
		struct nlmsghdr *h;
		int save_errno;
		status = recvmsg(nl->sock, &msg, 0);
		save_errno = errno;
		if (status < 0) {
			if (save_errno == EINTR)
				continue;			
			if (save_errno == EWOULDBLOCK
			    || save_errno == EAGAIN)
				break;
			printf( "%s recvmsg overrun: %s",
			     nl->name, safe_strerror(save_errno));
			continue;
		}

		if (status == 0) {
			printf( "%s EOF", nl->name);
			return -1;
		}
		if (msg.msg_namelen != sizeof snl) {
			printf(
			     "%s sender address length error: length %d",
			     nl->name, msg.msg_namelen);
			return -1;
		}

		/* JF: Ignore messages that aren't from the kernel */
		if (snl.nl_pid != 0) {

			//printf("Ignoring message from pid %u group %u",
			//	     snl.nl_pid, snl.nl_groups);
			continue;
		}

		for (h = (struct nlmsghdr *) buf;
		     NLMSG_OK(h, (unsigned int) status);
		     h = NLMSG_NEXT(h, status)) {
			/* Finish of reading. */
			if (h->nlmsg_type == NLMSG_DONE)
				return ret;

			/* Error handling. */
			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err =
				    (struct nlmsgerr *) NLMSG_DATA(h);

				/* If the error field is zero, then this is an ACK */
				if (err->error == 0) {

					/* return if not a multipart message, otherwise continue */
					if (!
					    (h->
					     nlmsg_flags & NLM_F_MULTI)) {
						return 0;
					}
					continue;
				}

				if (h->nlmsg_len <
				    NLMSG_LENGTH(sizeof(struct nlmsgerr)))
				{
					printf(
					     "%s error: message truncated",
					     nl->name);
					return -1;
				}

				/* Deal with Error Noise  - MAG */
				{
					int loglvl = LOG_ERR;
					int errnum = err->error;
					int msg_type = err->msg.nlmsg_type;

					if (nl == &netlink_cmd
					    && (-errnum == ENODEV
						|| -errnum == ESRCH)
					    && (msg_type == RTM_NEWROUTE
						|| msg_type ==
						RTM_DELROUTE))
						loglvl = LOG_DEBUG;
					  {

						
					}
				}
				/*           
				   ret = -1;
				   continue;
				 */
				return -1;
			}

			/* OK we got netlink message. */

			/* skip unsolicited messages originating from command socket */
			if (nl != &netlink_cmd
			    && h->nlmsg_pid == netlink_cmd.snl.nl_pid) {
				 
					printf
					    ("netlink_parse_info: %s packet comes from %s",
					     netlink_cmd.name, nl->name);
				continue;
			}

			error = (*filter) (&snl, h);
			if (error < 0) {
				 
					printf
					    ("%s filter function error",
					     nl->name);
				ret = error;
			}
		}

		/* After error care. */
		if (msg.msg_flags & MSG_TRUNC) {
			printf( "%s error: message truncated",
			     nl->name);
			continue;
		}
		if (status) {
			printf(
			     "%s error: data remnant size %d", nl->name,
			     status);
			return -1;
		}
	}

	  {
		//printf("Leave func %s nl->name is %s\n", __func__,
		//	   nl->name);
	}
	return ret;
}

int netlink_link_change(struct sockaddr_nl *snl, struct nlmsghdr *h)
{
	int len;
	struct ifinfomsg *ifi;
	struct rtattr *tb[IFLA_MAX + 1];
	char *name;
	struct ifi_info ifi_tmp;
	struct ifi *tmp;
	int m,n;
	ifi = NLMSG_DATA(h);

	if (!
	    (h->nlmsg_type == RTM_NEWLINK
	     || h->nlmsg_type == RTM_DELLINK)) {
		/* If this is not link add/delete message so print warning. */
		printf("netlink_link_change: wrong kernel message %d\n",
			  h->nlmsg_type);
		return 0;
	}

	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	if (len < 0)
		return -1;

	/* Looking up interface name. */
	memset(tb, 0, sizeof tb);
	netlink_parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);


	if (tb[IFLA_IFNAME] == NULL)
		return -1;
	name = (char *) RTA_DATA(tb[IFLA_IFNAME]);
	ifi_tmp.ifi_index = ifi->ifi_index;
	/* Add interface. */
	
	tmp = IUH_IF;
	m = strlen(name);
	while(tmp != NULL){
		n = strlen(tmp->ifi_name);
		if((m==n)&&(memcmp(tmp->ifi_name,name,strlen(name))==0)){
			break;
		}
		tmp = tmp->ifi_next;
	}
	if(tmp != NULL){
		if (h->nlmsg_type == RTM_NEWLINK) {	
			/*add interface*/
		} else {
			/*del interface*/
		}
	}
	//printf("link name %s\n",name);
	return 0;
}


/* Lookup interface IPv4/IPv6 address. */
int netlink_interface_addr(struct sockaddr_nl *snl, struct nlmsghdr *h)
{
	int len;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[IFA_MAX + 1];
	//void *addr = NULL;
	//void *broad = NULL;
	//u_char flags = 0;
	//char *label = NULL;
	ifa = NLMSG_DATA(h);
	unsigned int ip;
	char *name;
	unsigned char *ipaddr;
	struct ifi *tmp;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));	
	if(ifa->ifa_family == AF_INET)
		ifi_tmp->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
	else
		ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
	if (ifa->ifa_family != AF_INET
#ifdef HAVE_IPV6
	    && ifa->ifa_family != AF_INET6
#endif				/* HAVE_IPV6 */
	    )
		return 0;

	if (h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR)
		return 0;

	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	if (len < 0)
		return -1;

	memset(tb, 0, sizeof tb);
	netlink_parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), len);
	ifi_tmp->ifi_index = ifa->ifa_index;
	char buf[BUFSIZ];
	if (tb[IFA_LOCAL]){
		printf("  IFA_LOCAL     %s/%d\n",
			   inet_ntop(ifa->ifa_family,
				     RTA_DATA(tb[IFA_LOCAL]), buf,
				     BUFSIZ), ifa->ifa_prefixlen);
	}
	if (tb[IFA_ADDRESS])
	{	
		
		if(ifa->ifa_family == AF_INET)
		{	
			ipaddr = RTA_DATA(tb[IFA_LOCAL]);
			ip =  (ipaddr[0]<<24) + (ipaddr[1]<<16) +
				(ipaddr[2]<<8) + ipaddr[3];
			ifi_tmp->addr[0] = ip;
			ifi_tmp->addr_num = 1;
		
			//memcpy(&((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr,&ip,sizeof(struct in_addr));
			((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr.s_addr = ip;
			((struct sockaddr *) ifi_tmp->ifi_addr)->sa_family = AF_INET;
			printf("  IFA_ADDRESS   %s/%d\n",
				   inet_ntop(ifa->ifa_family,
					     RTA_DATA(tb[IFA_ADDRESS]),
					     buf, BUFSIZ),
				   ifa->ifa_prefixlen);
		}
		else
		{
			//dded ipv6 code
			ifi_tmp->addr_num = 1;
			inet_pton(AF_INET6, inet_ntop(ifa->ifa_family,
					     RTA_DATA(tb[IFA_ADDRESS]),
					     buf, BUFSIZ), &(((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr));
				printf("  IFA_ADDRESS   %s/%d\n",
		   inet_ntop(ifa->ifa_family,
			     RTA_DATA(tb[IFA_ADDRESS]),
			     buf, BUFSIZ),
		   ifa->ifa_prefixlen);
		}
		
	}
	if (tb[IFA_BROADCAST])
	{
		printf("  IFA_BROADCAST %s/%d\n",
			   inet_ntop(ifa->ifa_family,
				     RTA_DATA(tb[IFA_BROADCAST]),
				     buf, BUFSIZ),
			   ifa->ifa_prefixlen);
	}
	if (tb[IFA_LABEL])
	{
		name = (char *) RTA_DATA(tb[IFA_LABEL]);
		memcpy(ifi_tmp->ifi_name, name, strlen(name));
		printf("  IFA_LABEL     %s\n",
			   (char *) RTA_DATA(tb[IFA_LABEL]));
	}
	if (tb[IFA_CACHEINFO]) {
		struct ifa_cacheinfo *ci =
		    RTA_DATA(tb[IFA_CACHEINFO]);
		printf("  IFA_CACHEINFO pref %d, valid %d\n",
			   ci->ifa_prefered, ci->ifa_valid);
	}
	
	tmp = IUH_IF;
	while(tmp != NULL){
		if(memcmp(tmp->ifi_name,ifi_tmp->ifi_name,IF_NAME_LEN)==0){
			break;
		}
		tmp = tmp->ifi_next;
	}

	if(tmp != NULL){
		if(h->nlmsg_type == RTM_NEWADDR){
			if(ifa->ifa_family == AF_INET)
			{
				/*add new ipv4 addr binding*/			
			}
			else
			{			
				/*add new ipv6 addr binding*/			
			}
						
		}
		else{
			/*del  addr binding*/			
		}		

	}
	if(ifa->ifa_family == AF_INET){
		free(ifi_tmp->ifi_addr);
		ifi_tmp->ifi_addr = NULL;
	}
	else
	{
		free(ifi_tmp->ifi_addr6);
		ifi_tmp->ifi_addr6 = NULL;
	}
	free(ifi_tmp);
	ifi_tmp = NULL;
	return 0;
}

int netlink_information_fetch(struct sockaddr_nl *snl, struct nlmsghdr *h)
{

	switch (h->nlmsg_type) {
		
	case RTM_NEWLINK: //16
		return netlink_link_change(snl, h);
		break;
	case RTM_DELLINK: //17
		return netlink_link_change(snl, h);
		break;
	case RTM_NEWADDR: //20
		return netlink_interface_addr(snl, h);
		break;
	case RTM_DELADDR: //21
		return netlink_interface_addr(snl, h);
		break;
	default:
		//printf("Unknown netlink nlmsg_type %d\n",
		//	  h->nlmsg_type);
		break;
	}
	return 0;
}

int kernel_read()
{
	int ret;
	//int sock;
	fd_set fset;
	while(1){
		FD_ZERO(&fset);
		FD_SET(netlink.sock, &fset);
		while(select(netlink.sock+1, &fset, NULL, NULL, 0) < 0) {
			continue;
		}
		if(FD_ISSET(netlink.sock, &fset)) 
		{
			ret = netlink_parse_info(netlink_information_fetch, &netlink);
		}
	}
	printf("kernel netlink exit\n");
	return 0;
}


/* Make socket for Linux netlink interface. */
static int netlink_socket(struct nlsock *nl, unsigned long groups)
{
	int ret;
	struct sockaddr_nl snl;
	int sock;
	int namelen;
	int save_errno;
	u_int32_t nl_rcvbufsize = 0;
	sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock < 0) {
		printf("Can't open %s socket: %s", nl->name,
		     safe_strerror(errno));
		return -1;
	}

	ret = fcntl(sock, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		printf("Can't set %s socket flags: %s",
		     nl->name, safe_strerror(errno));
		close(sock);
		return -1;
	}
	nl_rcvbufsize = 576 * 512;/* sizeof(sk_buff) * 256*2*/

	/* Set receive buffer size if it's set from command line */
	if (nl_rcvbufsize) {
		u_int32_t oldsize, oldlen;
		u_int32_t newsize, newlen;

		oldlen = sizeof(oldsize);
		newlen = sizeof(newsize);

		ret =
		    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &oldsize,
			       &oldlen);
		if (ret < 0) {
			printf(
			     "Can't get %s receive buffer size: %s\n",
			     nl->name, safe_strerror(errno));
			close(sock);
			return -1;
		}

		ret =
		    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nl_rcvbufsize,
			       sizeof(nl_rcvbufsize));
		if (ret < 0) {
			printf(
			     "Can't set %s receive buffer size: %s\n",
			     nl->name, safe_strerror(errno));
			close(sock);
			return -1;
		}

		ret =
		    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &newsize,
			       &newlen);
		if (ret < 0) {
			printf(
			     "Can't get %s receive buffer size: %s\n",
			     nl->name, safe_strerror(errno));
			close(sock);
			return -1;
		}

		printf(
		     "Setting netlink socket receive buffer size: %u -> %u\n",
		     oldsize, newsize);
	}

	memset(&snl, 0, sizeof snl);
	snl.nl_family = AF_NETLINK;
	snl.nl_groups = groups;


	ret = bind(sock, (struct sockaddr *) &snl, sizeof snl);
	save_errno = errno;
	if (ret < 0) {
		printf(
		     "Can't bind %s socket to group 0x%x: %s", nl->name,
		     snl.nl_groups, safe_strerror(save_errno));
		close(sock);
		return -1;
	}
	/* multiple netlink sockets will have different nl_pid */
	namelen = sizeof snl;
	ret =
	    getsockname(sock, (struct sockaddr *) &snl,
			(socklen_t *) & namelen);
	if (ret < 0 || namelen != sizeof snl) {
		printf("Can't get %s socket name: %s",
		     nl->name, safe_strerror(errno));
		close(sock);
		return -1;
	}

	nl->snl = snl;
	nl->sock = sock;
	return ret;
}

int Iuh_Interface_Listen_init(void)
{
	unsigned long groups;
	//int s = PTHREAD_CREATE_DETACHED;		
	groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
#ifdef HAVE_IPV6
	groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif				/* HAVE_IPV6 */
	netlink_socket(&netlink, groups);
	netlink_socket(&netlink_cmd, 0);
	return 0;
}

