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
* Iplist.c
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
 
#include "libnetlink.h"
 
/* jme- workaround because glibc-2.1.1 or below doesnt have MSG_TRUNC */
#ifndef MSG_TRUNC
# define MSG_TRUNC 0x20
#endif
 
#if 1
# define nl_perror(str) perror(str)
#else
# define nl_perror(str)
#endif
 
int rtnl_open(struct rtnl_handle *rth, unsigned subscriptions)
{
 int addr_len;
 
 memset(rth, 0, sizeof(rth));
 
 rth->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
 if (rth->fd < 0) {
  nl_perror("Cannot open netlink socket");
  return -1;
 }
 
 memset(&rth->local, 0, sizeof(rth->local));
 rth->local.nl_family = AF_NETLINK;
 rth->local.nl_groups = subscriptions;
 
 if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0) {
  nl_perror("Cannot bind netlink socket");
  close(rth->fd);
  return -1;
 }
 addr_len = sizeof(rth->local);
 if (getsockname(rth->fd, (struct sockaddr*)&rth->local,  &addr_len) < 0) {
  nl_perror("Cannot getsockname");
  close(rth->fd);
  return -1;
 }
 if (addr_len != sizeof(rth->local)) {
  fprintf(stderr, "Wrong address length %d\n", addr_len);
  close(rth->fd);
  return -1;
 }
 if (rth->local.nl_family != AF_NETLINK) {
  fprintf(stderr, "Wrong address family %d\n", rth->local.nl_family);
  close(rth->fd);
  return -1;
 }
 rth->seq = time(NULL);
 return 0;
}
 
int rtnl_wilddump_request(struct rtnl_handle *rth, int family, int type)
{
 struct {
  struct nlmsghdr nlh;
  struct rtgenmsg g;
 } req;
 struct sockaddr_nl nladdr;
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 
 req.nlh.nlmsg_len = sizeof(req);
 req.nlh.nlmsg_type = type;
 req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
 req.nlh.nlmsg_pid = 0;
 req.nlh.nlmsg_seq = rth->dump = ++rth->seq;
 req.g.rtgen_family = family;
 
 return sendto(rth->fd, (void*)&req, sizeof(req), 0, (struct sockaddr*)&nladdr, sizeof(nladdr));
}
 
int rtnl_send(struct rtnl_handle *rth, char *buf, int len)
{
 struct sockaddr_nl nladdr;
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 
 return sendto(rth->fd, buf, len, 0, (struct sockaddr*)&nladdr, sizeof(nladdr));
}
 
int rtnl_dump_request(struct rtnl_handle *rth, int type, void *req, int len)
{
 struct nlmsghdr nlh;
 struct sockaddr_nl nladdr;
 struct iovec iov[2] = { { &nlh, sizeof(nlh) }, { req, len } };
 struct msghdr msg = {
  (void*)&nladdr, sizeof(nladdr),
  iov, 2,
  NULL, 0,
  0
 };
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 
 nlh.nlmsg_len = NLMSG_LENGTH(len);
 nlh.nlmsg_type = type;
 nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
 nlh.nlmsg_pid = 0;
 nlh.nlmsg_seq = rth->dump = ++rth->seq;
 
 return sendmsg(rth->fd, &msg, 0);
}
 
int rtnl_dump_filter(struct rtnl_handle *rth,
       int (*filter)(struct sockaddr_nl *, struct nlmsghdr *n, void *),
       void *arg1,
       int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
       void *arg2)
{
 char buf[8192];
 struct sockaddr_nl nladdr;
 struct iovec iov = { buf, sizeof(buf) };
 
 while (1) {
  int status;
  struct nlmsghdr *h;
 
  struct msghdr msg = {
   (void*)&nladdr, sizeof(nladdr),
   &iov, 1,
   NULL, 0,
   0
  };
 
  status = recvmsg(rth->fd, &msg, 0);
 
  if (status < 0) {
   if (errno == EINTR)
    continue;
   nl_perror("OVERRUN");
   continue;
  }
  if (status == 0) {
   fprintf(stderr, "EOF on netlink\n");
   return -1;
  }
  if (msg.msg_namelen != sizeof(nladdr)) {
   fprintf(stderr, "sender address length == %d\n", msg.msg_namelen);
   exit(1);
  }
 
  h = (struct nlmsghdr*)buf;
  while (NLMSG_OK(h, status)) {
   int err;
 
   if (h->nlmsg_pid != rth->local.nl_pid ||
       h->nlmsg_seq != rth->dump) {
    if (junk) {
     err = junk(&nladdr, h, arg2);
     if (err < 0)
      return err;
    }
    goto skip_it;
   }
 
   if (h->nlmsg_type == NLMSG_DONE)
    return 0;
   if (h->nlmsg_type == NLMSG_ERROR) {
    struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
    if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
     fprintf(stderr, "ERROR truncated\n");
    } else {
     errno = -err->error;
     /* Scott checked for EEXIST and return 0! 9-4-02*/
     if (errno != EEXIST)
      nl_perror("RTNETLINK answers");
     else return 0;
    }
    return -1;
   }
   err = filter(&nladdr, h, arg1);
   if (err < 0)
    return err;
 
skip_it:
   h = NLMSG_NEXT(h, status);
  }
  if (msg.msg_flags & MSG_TRUNC) {
   fprintf(stderr, "Message truncated\n");
   continue;
  }
  if (status) {
   fprintf(stderr, "!!!Remnant of size %d\n", status);
   exit(1);
  }
 }
}
 
int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
       unsigned groups, struct nlmsghdr *answer,
       int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
       void *jarg)
{
 int status;
 struct nlmsghdr *h;
 struct sockaddr_nl nladdr;
 struct iovec iov = { (void*)n, n->nlmsg_len };
 char   buf[8192];
 struct msghdr msg = {
  (void*)&nladdr, sizeof(nladdr),
  &iov, 1,
  NULL, 0,
  0
 };
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 nladdr.nl_pid = peer;
 nladdr.nl_groups = groups;
 
 n->nlmsg_seq = ++rtnl->seq;
 if (answer == NULL)
  n->nlmsg_flags |= NLM_F_ACK;
 
 status = sendmsg(rtnl->fd, &msg, 0);
 
 if (status < 0) {
  nl_perror("Cannot talk to rtnetlink");
  return -1;
 }
 
 iov.iov_base = buf;
 iov.iov_len = sizeof(buf);
 
 while (1) {
  status = recvmsg(rtnl->fd, &msg, 0);
 
  if (status < 0) {
   if (errno == EINTR)
    continue;
   nl_perror("OVERRUN");
   continue;
  }
  if (status == 0) {
   fprintf(stderr, "EOF on netlink\n");
   return -1;
  }
  if (msg.msg_namelen != sizeof(nladdr)) {
   fprintf(stderr, "sender address length == %d\n", msg.msg_namelen);
   exit(1);
  }
  for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
   int err;
   int len = h->nlmsg_len;
   pid_t pid=h->nlmsg_pid;
   int l = len - sizeof(*h);
   unsigned seq=h->nlmsg_seq;
 
   if (l<0 || len>status) {
    if (msg.msg_flags & MSG_TRUNC) {
     fprintf(stderr, "Truncated message\n");
     return -1;
    }
    fprintf(stderr, "!!!malformed message: len=%d\n", len);
    exit(1);
   }
 
   if (h->nlmsg_pid != pid || h->nlmsg_seq != seq) {
    if (junk) {
     err = junk(&nladdr, h, jarg);
     if (err < 0)
      return err;
    }
    continue;
   }
 
   if (h->nlmsg_type == NLMSG_ERROR) {
    struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
    if (l < sizeof(struct nlmsgerr)) {
     fprintf(stderr, "ERROR truncated\n");
    } else {
     errno = -err->error;
     if (errno == 0) {
      if (answer)
       memcpy(answer, h, h->nlmsg_len);
      return 0;
     }
     /* Scott checked for EEXIST and return 0! 9-4-02*/
     if (errno != EEXIST)
      nl_perror("RTNETLINK answers");
     else return 0;
    }
    return -1;
   }
   if (answer) {
    memcpy(answer, h, h->nlmsg_len);
    return 0;
   }
 
   fprintf(stderr, "Unexpected reply!!!\n");
 
   status -= NLMSG_ALIGN(len);
   h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
  }
  if (msg.msg_flags & MSG_TRUNC) {
   fprintf(stderr, "Message truncated\n");
   continue;
  }
  if (status) {
   fprintf(stderr, "!!!Remnant of size %d\n", status);
   exit(1);
  }
 }
}
 
int rtnl_listen(struct rtnl_handle *rtnl, 
       int (*handler)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
       void *jarg)
{
 int status;
 struct nlmsghdr *h;
 struct sockaddr_nl nladdr;
 struct iovec iov;
 char   buf[8192];
 struct msghdr msg = {
  (void*)&nladdr, sizeof(nladdr),
  &iov, 1,
  NULL, 0,
  0
 };
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 nladdr.nl_pid = 0;
 nladdr.nl_groups = 0;
 

 iov.iov_base = buf;
 iov.iov_len = sizeof(buf);
 
 while (1) {
  status = recvmsg(rtnl->fd, &msg, 0);
 
  if (status < 0) {
   if (errno == EINTR)
    continue;
   nl_perror("OVERRUN");
   continue;
  }
  if (status == 0) {
   fprintf(stderr, "EOF on netlink\n");
   return -1;
  }
  if (msg.msg_namelen != sizeof(nladdr)) {
   fprintf(stderr, "Sender address length == %d\n", msg.msg_namelen);
   exit(1);
  }
  for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
   int err;
   int len = h->nlmsg_len;
   int l = len - sizeof(*h);
 
   if (l<0 || len>status) {
    if (msg.msg_flags & MSG_TRUNC) {
     fprintf(stderr, "Truncated message\n");
     return -1;
    }
    fprintf(stderr, "!!!malformed message: len=%d\n", len);
    exit(1);
   }
 
   err = handler(&nladdr, h, jarg);
   if (err < 0)
    return err;
 
   status -= NLMSG_ALIGN(len);
   h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
  }
  if (msg.msg_flags & MSG_TRUNC) {
   fprintf(stderr, "Message truncated\n");
   continue;
  }
  if (status) {
   fprintf(stderr, "!!!Remnant of size %d\n", status);
   exit(1);
  }
 }
}
 
int rtnl_from_file(FILE *rtnl, 
       int (*handler)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
       void *jarg)
{
 int status;
 struct sockaddr_nl nladdr;
 char   buf[8192];
 struct nlmsghdr *h = (void*)buf;
 
 memset(&nladdr, 0, sizeof(nladdr));
 nladdr.nl_family = AF_NETLINK;
 nladdr.nl_pid = 0;
 nladdr.nl_groups = 0;
 
 while (1) {
  int err, len, type;
  pid_t pid;
  int l;
  unsigned seq;
 
  status = fread(&buf, 1, sizeof(*h), rtnl);
 
  if (status < 0) {
   if (errno == EINTR)
    continue;
   nl_perror("rtnl_from_file: fread");
   return -1;
  }
  if (status == 0)
   return 0;
 
  len = h->nlmsg_len;
  type= h->nlmsg_type;
  pid=h->nlmsg_pid;
  l = len - sizeof(*h);
  seq=h->nlmsg_seq;
 
  if (l<0 || len>sizeof(buf)) {
   fprintf(stderr, "!!!malformed message: len=%d @%lu\n",
    len, ftell(rtnl));
   return -1;
  }
 
  status = fread(NLMSG_DATA(h), 1, NLMSG_ALIGN(l), rtnl);
 
  if (status < 0) {
   nl_perror("rtnl_from_file: fread");
   return -1;
  }
  if (status < l) {
   fprintf(stderr, "rtnl-from_file: truncated message\n");
   return -1;
  }
 
  err = handler(&nladdr, h, jarg);
  if (err < 0)
   return err;
 }
}
 
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
 int len = RTA_LENGTH(4);
 struct rtattr *rta;
 if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
  return -1;
 rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
 rta->rta_type = type;
 rta->rta_len = len;
 memcpy(RTA_DATA(rta), &data, 4);
 n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
 return 0;
}
 
int addattr_l(struct nlmsghdr *n, int maxlen, int type, void *data, int alen)
{
 int len = RTA_LENGTH(alen);
 struct rtattr *rta;
 
 if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
  return -1;
 rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
 rta->rta_type = type;
 rta->rta_len = len;
 memcpy(RTA_DATA(rta), data, alen);
 n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
 return 0;
}
 
int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data)
{
 int len = RTA_LENGTH(4);
 struct rtattr *subrta;
 
 if (RTA_ALIGN(rta->rta_len) + len > maxlen)
  return -1;
 subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
 subrta->rta_type = type;
 subrta->rta_len = len;
 memcpy(RTA_DATA(subrta), &data, 4);
 rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;
 return 0;
}
 
int rta_addattr_l(struct rtattr *rta, int maxlen, int type, void *data, int alen)
{
 struct rtattr *subrta;
 int len = RTA_LENGTH(alen);
 
 if (RTA_ALIGN(rta->rta_len) + len > maxlen)
  return -1;
 subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
 subrta->rta_type = type;
 subrta->rta_len = len;
 memcpy(RTA_DATA(subrta), data, alen);
 rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;
 return 0;
}
 

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
 while (RTA_OK(rta, len)) {
  if (rta->rta_type <= max)
   tb[rta->rta_type] = rta;
  rta = RTA_NEXT(rta,len);
 }
 if (len)
  fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
 return 0;
}
 
int rtnl_close(struct rtnl_handle *rth)
{
 /* close the fd */
 close( rth->fd );
 return(0);
}

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
#include "libnetlink.h"
//#include "ipaddr.h"
 
typedef struct {
 int  ifindex;
 uint32_t *addr;
 int  max_elem;
 int  nb_elem;
} iplist_ctx;
 
/****************************************************************
 NAME : print_addr    00/06/02 18:24:09
 AIM : 
 REMARK :
****************************************************************/
static int get_addrinfo(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
 struct ifaddrmsg *ifa  = NLMSG_DATA(n);
 int  len = n->nlmsg_len;
 iplist_ctx *ctx = (iplist_ctx *)arg;
 struct rtattr  *rta_tb[IFA_MAX+1];
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
 parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa), len);
 
 if (!rta_tb[IFA_LOCAL])
  rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];
 
 if (rta_tb[IFA_LOCAL]) {
  u_char *src = RTA_DATA(rta_tb[IFA_LOCAL]);
  if( ctx->nb_elem >= ctx->max_elem )
   return 0;
  ctx->addr[ctx->nb_elem++] =  (src[0]<<24) + (src[1]<<16) +
      (src[2]<<8) + src[3];
 }
 return 0;
}
 

/****************************************************************
 NAME : ipaddr_list    00/06/02 20:02:23
 AIM : 
 REMARK :
****************************************************************/
int ipaddr_list( int ifindex, uint32_t *array, int max_elem )
{
 struct rtnl_handle rth;
 iplist_ctx ctx;
 /* init the struct */
 ctx.ifindex = ifindex;
 ctx.addr = array;
 ctx.max_elem = max_elem;
 ctx.nb_elem = 0;
 /* open the rtnetlink socket */
 if( rtnl_open( &rth, 0) )
  return -1;
 /* send the request */
 if (rtnl_wilddump_request(&rth, AF_INET, RTM_GETADDR) < 0) {
  perror("Cannot send dump request");
    rtnl_close( &rth );
  return -1;
 }
 /* parse the answer */
 if (rtnl_dump_filter(&rth, get_addrinfo, &ctx, NULL, NULL) < 0) {
  fprintf(stderr, "Flush terminated\n");
    rtnl_close( &rth );
	return -1;
  //exit(1);
 }
 
 /* to close the clocket */
  rtnl_close( &rth );
 
 return ctx.nb_elem;
}
 

/****************************************************************
 NAME : ipaddr_add    00/06/02 23:00:58
 AIM : add or remove 
 REMARK :
****************************************************************/
int ipaddr_op( int ifindex, uint32_t addr, int addF )
{
 struct rtnl_handle rth;
 struct {
  struct nlmsghdr  n;
  struct ifaddrmsg  ifa;
  char      buf[256];
 } req;
 //vrrp_syslog_dbg("start ip op: ifindex %d,ipaddr %02x,%s\n",ifindex,addr,addF ? "RTM_NEWADDR" : "RTM_DELADDR");
 memset(&req, 0, sizeof(req));
 
 req.n.nlmsg_len  = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
 req.n.nlmsg_flags = NLM_F_REQUEST;
 req.n.nlmsg_type = addF ? RTM_NEWADDR : RTM_DELADDR;
 req.ifa.ifa_family = AF_INET;
 req.ifa.ifa_index = ifindex;
 req.ifa.ifa_prefixlen = 32; /*virtual router ip addr be with 32 masklen*/
 
 addr = htonl( addr );
 addattr_l(&req.n, sizeof(req), IFA_LOCAL, &addr, sizeof(addr) );
 
 if (rtnl_open(&rth, 0) < 0){
        //vrrp_syslog_error("open netlink socket failed!\n");
  return -1;
 }
 if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0){
        //vrrp_syslog_error("netlink talk failed\n");
  return -1;
    }
 
 /* to close the clocket */
  rtnl_close( &rth );
 
 return(0);
}
 /*
void main(int argc, char *argv[]){
	int sockfd,ret,i;
	struct ifreq	ifr, ifrcopy;
	unsigned int array[8];
	char name[32];
	memset(name,0,32);
	memcpy(name,argv[1],strlen(argv[1]));
	printf("name %s\n",name);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));			
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		printf("SIOCGIFINDEX error\n");
		close(sockfd);
		return;
	 }
	ret = ipaddr_list( ifr.ifr_ifindex, array, 8);
	printf("ret %d\n",ret);
	for(i=0;i<ret;i++)
		printf("%x\n",array[i]);
} 
 */
 
