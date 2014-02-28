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
* ACNetlink.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/
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
#include "CWAC.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#include "ACNetlink.h"
#include "ACIPv6Addr.h"

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

      if (strlen(temp) >= len) 
	{
	  errno = ENOSPC;
	  return NULL;
	}
      strcpy(strptr, temp);
      return strptr;
  }
  
  if (family == AF_INET6) 
  {
      char temp[256];

		snprintf(temp, sizeof(temp), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",\
		p[0], p[1], p[2], p[3],p[4], p[5], p[6], p[7],p[8], p[9], p[10], p[11],p[12], p[13], p[14], p[15]);
		
      if (strlen(temp) >= len) 
	  {
		  errno = ENOSPC;
		  return NULL;
	  }
      strcpy(strptr, temp);
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
		struct nlmsghdr *h=NULL;
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
	
	tmp = WID_IF;
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
			Check_WLAN_WTP_IF_Index(&ifi_tmp,name);
			printf("add name %s\n",name);
		} else {
			Delete_Interface(name, ifi_tmp.ifi_index);
			printf("del name %s\n",name);
		}
	}
	printf("link name %s\n",name);
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
	int ret = 0;
	unsigned char *ipaddr;
	struct ifi *tmp;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));	
	if (NULL == ifi_tmp)
	{
		return -1;
	}
	if(ifa->ifa_family == AF_INET)
		ifi_tmp->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
		if (NULL == ifi_tmp->ifi_addr)
		{
			CW_FREE_OBJECT_WID(ifi_tmp);
			return -1;
		}
	else
		ifi_tmp->ifi_addr6 = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
		if (NULL == ifi_tmp->ifi_addr6)
		{
			CW_FREE_OBJECT_WID(ifi_tmp);
			return -1;
		}
	if (ifa->ifa_family != AF_INET
#ifdef HAVE_IPV6
	    && ifa->ifa_family != AF_INET6
#endif				/* HAVE_IPV6 */
	    ) {
		if(ifi_tmp->ifi_addr){
			WID_FREE(ifi_tmp->ifi_addr);
			ifi_tmp->ifi_addr = NULL;
		}
		if(ifi_tmp->ifi_addr6){
			WID_FREE(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
		}
		if(ifi_tmp){
			WID_FREE(ifi_tmp);
			ifi_tmp = NULL;
		}
		return 0;
	}

	if (h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR){
		if(ifi_tmp->ifi_addr){
			WID_FREE(ifi_tmp->ifi_addr);
			ifi_tmp->ifi_addr = NULL;
		}
		if(ifi_tmp->ifi_addr6){
			WID_FREE(ifi_tmp->ifi_addr6);
			ifi_tmp->ifi_addr6 = NULL;
		}
		if(ifi_tmp){
			WID_FREE(ifi_tmp);
			ifi_tmp = NULL;
		}
		return 0;
	}

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
	
	if(h->nlmsg_type == RTM_NEWADDR){
		if(ifa->ifa_family == AF_INET)
		{
			tmp = WID_IF;
			while(tmp != NULL){
				if((tmp->lic_flag == DOWN_LINK_IF_TYPE)&&(memcmp(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME)==0)){
					ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT,DOWN_LINK_IF_TYPE);
					if(ret == 1){
						tmp = tmp->ifi_next;
						continue;
					}
					unsigned int ip;
					ip = ((struct sockaddr_in *)(ifi_tmp->ifi_addr))->sin_addr.s_addr;
					WIDWsm_VRRPIFOp((unsigned char*)ifi_tmp->ifi_name,ip,VRRP_REG_IF);
					Check_WLAN_WTP_IF_Index(ifi_tmp,name);
				}
				else if(tmp->addr == ifi_tmp->addr[0])
				{					
					memset(tmp->ifi_name,0,IFI_NAME);
					memcpy(tmp->ifi_name,ifi_tmp->ifi_name,IFI_NAME);
					if(tmp->lic_flag == LIC_TYPE){
						//Add_Listenning_IP(tmp->ifi_name,tmp->addr,LIC_TYPE);
						Check_Listenning_Ip(tmp->ifi_name,tmp->addr,LIC_TYPE,1);
						Bind_Interface_For_WID(ifi_tmp,WID_LIC_AC_PORT,LIC_TYPE);	 
						wid_syslog_debug_debug(WID_DBUS,"%s,%d,WID_LIC_AC_PORT,ip:%d.%d.%d.%d.\n",__func__,__LINE__,(tmp->addr>>24)&0xFF,(tmp->addr>>16)&0xFF,(tmp->addr>>8)&0xFF,(tmp->addr)&0xFF);
					}else{
						//Add_Listenning_IP(tmp->ifi_name,tmp->addr,DOWN_LINK_TYPE);
						Check_Listenning_Ip(tmp->ifi_name,tmp->addr,DOWN_LINK_IP_TYPE,1);
						ret = Bind_Interface_For_WID(ifi_tmp,CW_CONTROL_PORT,DOWN_LINK_IP_TYPE);		
						wid_syslog_debug_debug(WID_DBUS,"%s,%d,CW_CONTROL_PORT,ip:%d.%d.%d.%d.\n",__func__,__LINE__,(tmp->addr>>24)&0xFF,(tmp->addr>>16)&0xFF,(tmp->addr>>8)&0xFF,(tmp->addr)&0xFF);
						if(ret == 1){
							tmp = tmp->ifi_next;
							continue;
						}
						unsigned int ip;
						ip = ((struct sockaddr_in *)(ifi_tmp->ifi_addr))->sin_addr.s_addr;
						WIDWsm_VRRPIFOp((unsigned char*)ifi_tmp->ifi_name,ip,VRRP_REG_IF);
						Check_WLAN_WTP_IF_Index(ifi_tmp,name);
					}
				}
				tmp = tmp->ifi_next;
			}
		}
		else
		{		
			tmp = WID_IF_V6;
			while(tmp != NULL){
				if(tmp->ifi_index == ifi_tmp->ifi_index){
					memcpy(ifi_tmp->ifi_name,tmp->ifi_name,IFI_NAME);
					break;
				}
				tmp = tmp->ifi_next;
			}
			if(tmp != NULL)
				if( (((struct sockaddr_in6*)ifi_tmp->ifi_addr6)->sin6_addr).s6_addr[0] != 0xfe)
				{
					ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT);
					//ipv6_bind_interface_for_wid(ifi_tmp,CW_CONTROL_PORT_AU);
					
					WIDWsm_VRRPIFOp_IPv6(ifi_tmp,VRRP_REG_IF);
					Check_WLAN_WTP_IF_Index(ifi_tmp,ifi_tmp->ifi_name);	
				}
		}
					
	}
	else if(h->nlmsg_type == RTM_DELADDR){
		wid_syslog_notice("delete addr\n");
		Delete_Bind_Interface_For_WID(ifi_tmp);
	}else{
		wid_syslog_info("h->nlmsg_type:%d.(RTM_NEWADDR:%d,RTM_DELADDR:%d.).\n",h->nlmsg_type,RTM_NEWADDR,RTM_DELADDR);
	}		
	gInterfacesCount = CWNetworkCountInterfaceAddresses(&gACSocket);
	gInterfacesCountIpv4 = CWNetworkCountInterfaceAddressesIpv4(&gACSocket);
    gInterfacesCountIpv6 = CWNetworkCountInterfaceAddressesIpv6(&gACSocket);
	if(ifa->ifa_family == AF_INET){
		WID_FREE(ifi_tmp->ifi_addr);
		ifi_tmp->ifi_addr = NULL;
	}
	else
	{
		WID_FREE(ifi_tmp->ifi_addr6);
		ifi_tmp->ifi_addr6 = NULL;
	}
	WID_FREE(ifi_tmp);
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
	wid_pid_write_v2("kernel_read",0,vrrid);
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

int WID_Interface_Listen_init(void)
{
	unsigned long groups;
	groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
#ifdef HAVE_IPV6
	groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif				/* HAVE_IPV6 */
	netlink_socket(&netlink, groups);
	netlink_socket(&netlink_cmd, 0);
/*	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,s);
	if (netlink.sock > 0){
		if(pthread_create(&wid_netlink, &attr, kernel_read, NULL) != 0) {
			printf("wid_netlink thread failed\n");
			return -1;
		}
	}*/
//	pthread_join(wid_netlink,NULL);
	return 0;
}

