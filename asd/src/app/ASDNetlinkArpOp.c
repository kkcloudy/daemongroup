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
* ASDNetlinkArpOp.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
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

#include <netdb.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <asm/types.h>
#include <linux/pkt_sched.h>
#include <sys/time.h>
#include <net/if.h>
#include "asd.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ASDNetlinkArpOp.h"
#include "asd_bak.h"
#include "ASDDbus_handler.h"
#include "asd_iptables.h"
#include "ASDStaInfo.h"
#include "ASDAccounting.h"
#include "ASDEAPAuth.h"
#include "ASDDbus.h"

static struct idxmap *idxmap[16];
struct rtnl_handle rth = { .fd = -1 };
struct rtnl_handle rth1 = { .fd = -1 };

int ifnametogroupid(char * ifname_t, unsigned int *id){
	int i = 0;
	for(i = 1; i < WLAN_NUM ; i++){
		if((ASD_ARP_GROUP[i]!=NULL)&&(strcmp(ASD_ARP_GROUP[i],ifname_t)==0)){
			*id = i;
			return 1;
		}
	}
	*id = 0;
	return 0;
}

int do_one_request(struct nlmsghdr *n)
{
	struct dbkey
	{
		__u32	iface;
		__u32	addr;
		__u32	gidx;
		unsigned char	mac[6];
	};
	
	struct ndmsg *ndm = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[NDA_MAX+1];
	struct dbkey key;
	struct asd_stainfo * stainfo = NULL;
	char ifname[ETH_IF_NAME_LEN];
	char *ifname_t;
	unsigned char SID = 0;
	memset(&key, 0, sizeof(struct dbkey));
	if (n->nlmsg_type == NLMSG_DONE) {
		return 0;
	}

	if (n->nlmsg_type != RTM_GETNEIGH && n->nlmsg_type != RTM_NEWNEIGH)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ndm));
	if (len < 0)
		return -1;

	if (ndm->ndm_family != AF_INET ||
		ndm->ndm_flags ||
		ndm->ndm_type != RTN_UNICAST ||
		!(ndm->ndm_state&~NUD_NOARP))
		return 0;

	parse_rtattr(tb, NDA_MAX, NDA_RTA(ndm), len);

	if (!tb[NDA_DST])
		return 0;

	key.iface = ndm->ndm_ifindex;
	memcpy(&key.addr, RTA_DATA(tb[NDA_DST]), 4);
	if (tb[NDA_LLADDR])
		memcpy(key.mac, RTA_DATA(tb[NDA_LLADDR]), 6);

	if (n->nlmsg_type == RTM_GETNEIGH) {
		if (!(n->nlmsg_flags&NLM_F_REQUEST))
			return 0;

		if (!(ndm->ndm_state&(NUD_PROBE|NUD_INCOMPLETE))) {
			return 0;
		}

		if (ndm->ndm_state&NUD_PROBE) {
			/* If we get this, kernel still has some valid
			 * address, but unicast probing failed and host
			 * is either dead or changed its mac address.
			 * Kernel is going to initiate broadcast resolution.
			 * OK, we invalidate our information as well.
			 */
		} else if (tb[NDA_LLADDR]){
			unsigned char *ip;
			char mac[20];		
			ip = (unsigned char *)&(key.addr);
			ifname_t = if_indextoname(key.iface, ifname);
			if(ifname_t){				
				pthread_mutex_lock(&(asd_g_sta_mutex));
				stainfo = (struct asd_stainfo*)ASD_SEARCH_STA(key.mac);
				if((stainfo != NULL)&&(stainfo->bss != NULL)&&(stainfo->sta != NULL)&&(stainfo->sta->in_addr != NULL)&&(stainfo->sta->ipaddr != key.addr)){
					memset(mac,0,20);
					sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(stainfo->sta->addr));
					ipneigh_modify(RTM_DELNEIGH, 0,stainfo->sta->in_addr, mac,stainfo->sta->arpifname);		
					 if(stainfo->sta->ipaddr != 0){
						if(stainfo->sta->security_type == NO_NEED_AUTH){
							if(asd_ipset_switch)
								eap_connect_down(stainfo->sta->ipaddr);
							else
								AsdStaInfoToEAG(stainfo->bss,stainfo->sta,ASD_DEL_AUTH);
						}
					}
					ifnametogroupid(ifname_t,&(key.gidx));
					stainfo->sta->gifidx = key.gidx;
					stainfo->sta->ipaddr = key.addr;
					memset(stainfo->sta->in_addr, 0, 16);
					sprintf(stainfo->sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);						

					//mahz	add 2011.3.16
					inet_aton(stainfo->sta->in_addr,&(stainfo->sta->ip_addr));

					if(ASD_WLAN[stainfo->bss->WlanID])
						SID = (unsigned char)ASD_WLAN[stainfo->bss->WlanID]->SecurityID;
					if(stainfo->sta->ipaddr != 0){
						if((stainfo->sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(stainfo->sta))){
							if(asd_ipset_switch)
								eap_connect_up(stainfo->sta->ipaddr);
							else
								AsdStaInfoToEAG(stainfo->bss,stainfo->sta,ASD_AUTH);
						}
					}
					
					memset(stainfo->sta->arpifname, 0, 16);
					strcpy(stainfo->sta->arpifname,ifname_t);
					if(asd_sta_arp_listen == 2){								
						memset(mac,0,20);
						sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(stainfo->sta->addr));
						ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,stainfo->sta->in_addr, mac,stainfo->sta->arpifname);
					}
					//SID = (unsigned char)ASD_WLAN[stainfo->bss->WlanID]->SecurityID;
					if((ASD_SECURITY[SID])&&( 0 ==ASD_SECURITY[SID]->account_after_authorize))
					{
						if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
							if(stainfo->sta->flags &WLAN_STA_AUTHORIZED)
								accounting_sta_start(stainfo->bss, stainfo->sta);	
						}
					}	
					if(is_secondary == 0)
						bak_update_sta_ip_info(stainfo->bss, stainfo->sta);
				}
				pthread_mutex_unlock(&(asd_g_sta_mutex));
				if(stainfo){
					stainfo->bss = NULL;
					stainfo->sta = NULL;
					free(stainfo);
					stainfo = NULL;
				}	
				#if 0
				if(!if_name2gindex(ifname_t,&(key.gidx))){
					stainfo = ASD_SEARCH_STA(key.mac);
					if((stainfo != NULL)&&(stainfo->sta->ipaddr != key.addr)){
						stainfo->sta->gifidx = key.gidx;
						stainfo->sta->ipaddr = key.addr;
						memset(stainfo->sta->in_addr, 0, 16);
						sprintf(stainfo->sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);						
						memset(stainfo->sta->arpifname, 0, 16);
						strcpy(stainfo->sta->arpifname,ifname_t);
						if(is_secondary == 0)
							bak_update_sta_ip_info(stainfo->bss, stainfo->sta);
					}
				}
				#endif
				
			}
			printf("get arp ifindex %d	ip %d.%d.%d.%d	mac %02X:%02X:%02X:%02X:%02X:%02X\n",ndm->ndm_ifindex,ip[0],ip[1],ip[2],ip[3],
				key.mac[0],key.mac[1],key.mac[2],key.mac[3],key.mac[4],key.mac[5]);
		}

	} else if (n->nlmsg_type == RTM_NEWNEIGH) {
		if (n->nlmsg_flags&NLM_F_REQUEST)
			return 0;

		if (ndm->ndm_state&NUD_FAILED) {
			/* Kernel was not able to resolve. Host is dead.
			 * Create negative entry if it is not present
			 * or renew it if it is too old. */
		} else if (tb[NDA_LLADDR]) {
			unsigned char *ip;
			char mac[20];
			ip = (unsigned char *)&(key.addr);			
			ifname_t = if_indextoname(key.iface, ifname);
			if(ifname_t){
				pthread_mutex_lock(&(asd_g_sta_mutex));
				stainfo = (struct asd_stainfo*)ASD_SEARCH_STA(key.mac);
				if((stainfo != NULL)&&(stainfo->bss != NULL)&&(stainfo->sta != NULL)&&(stainfo->sta->in_addr != NULL)&&(stainfo->sta->ipaddr != key.addr)){
					memset(mac,0,20);
					sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(stainfo->sta->addr));
					ipneigh_modify(RTM_DELNEIGH, 0,stainfo->sta->in_addr, mac,stainfo->sta->arpifname);						
					 if(stainfo->sta->ipaddr != 0){
						if((stainfo->sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(stainfo->sta))){
							if(asd_ipset_switch)
								eap_connect_down(stainfo->sta->ip_addr.s_addr);
							else
								AsdStaInfoToEAG(stainfo->bss,stainfo->sta,ASD_DEL_AUTH);
						}
					}
					ifnametogroupid(ifname_t,&(key.gidx));
					stainfo->sta->gifidx = key.gidx;
					stainfo->sta->ipaddr = key.addr;
					memset(stainfo->sta->in_addr, 0, 16);
					sprintf(stainfo->sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);						

					//mahz  add 2011.3.16
					inet_aton(stainfo->sta->in_addr,&(stainfo->sta->ip_addr));
					
					if(ASD_WLAN[stainfo->bss->WlanID])
						SID = (unsigned char)ASD_WLAN[stainfo->bss->WlanID]->SecurityID;
					if(ASD_SECURITY[SID] == NULL)
						return 0;
					if(stainfo->sta->ipaddr != 0){
						if((stainfo->sta->security_type == NO_NEED_AUTH)||(HYBRID_AUTH_EAPOL_SUCCESS(stainfo->sta))){
							if(asd_ipset_switch)
								eap_connect_up(stainfo->sta->ipaddr);
							else
								AsdStaInfoToEAG(stainfo->bss,stainfo->sta,ASD_AUTH);
						}
					}	
					memset(stainfo->sta->arpifname, 0, 16);
					strcpy(stainfo->sta->arpifname,ifname_t);
					
					if(asd_sta_arp_listen == 2){							
						memset(mac,0,20);
						sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STR(stainfo->sta->addr));
						ipneigh_modify(RTM_NEWNEIGH, NLM_F_CREATE|NLM_F_REPLACE,stainfo->sta->in_addr, mac,stainfo->sta->arpifname);
					}
					//SID = (unsigned char)ASD_WLAN[stainfo->bss->WlanID]->SecurityID;
					if((ASD_SECURITY[SID])&&( 0 ==ASD_SECURITY[SID]->account_after_authorize))
					{
						if((ASD_SECURITY[SID]->securityType == WPA_E)||(ASD_SECURITY[SID]->securityType == WPA2_E)||(ASD_SECURITY[SID]->securityType == WPA_P)||(ASD_SECURITY[SID]->securityType == WPA2_P)||(ASD_SECURITY[SID]->securityType == IEEE8021X)||(ASD_SECURITY[SID]->securityType == MD5)||(ASD_SECURITY[SID]->extensible_auth == 1)){
							accounting_sta_start(stainfo->bss, stainfo->sta);	
						}
					}					
					if(is_secondary == 0)
						bak_update_sta_ip_info(stainfo->bss, stainfo->sta);
				}
				pthread_mutex_unlock(&(asd_g_sta_mutex));
				if(stainfo){
					stainfo->bss = NULL;
					stainfo->sta = NULL;
					free(stainfo);
					stainfo = NULL;
				}	
				#if 0
				if(!if_name2gindex(ifname_t,&(key.gidx))){
					stainfo = ASD_SEARCH_STA(key.mac);
					if((stainfo != NULL)&&(stainfo->sta->ipaddr != key.addr)){
						stainfo->sta->gifidx = key.gidx;
						stainfo->sta->ipaddr = key.addr;
						memset(stainfo->sta->in_addr, 0, 16);
						sprintf(stainfo->sta->in_addr,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
						memset(stainfo->sta->arpifname, 0, 16);
						strcpy(stainfo->sta->arpifname,ifname_t);
						if(is_secondary == 0)
							bak_update_sta_ip_info(stainfo->bss, stainfo->sta);
					}
				}
				#endif
			}
			printf("add arp ifindex %d  ip %d.%d.%d.%d 	mac %02X:%02X:%02X:%02X:%02X:%02X\n",ndm->ndm_ifindex,ip[0],ip[1],ip[2],ip[3],
				key.mac[0],key.mac[1],key.mac[2],key.mac[3],key.mac[4],key.mac[5]);
		}
	}
	return 0;
}



void do_asd_sta_arp_listen(int sock, void *circle_ctx, void *sock_ctx){
	int status;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov;
	char   buf[8192];
	struct msghdr msg = {
		(void*)&nladdr, sizeof(nladdr),
		&iov,	1,
		NULL,	0,
		0
	};

	memset(&nladdr, 0, sizeof(nladdr));

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	status = recvmsg(rth1.fd, &msg, 0);

	if (status <= 0)
		return;

	if (msg.msg_namelen != sizeof(nladdr))
		return;

	if (nladdr.nl_pid)
		return;

	for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
		int len = h->nlmsg_len;
		int l = len - sizeof(*h);

		if (l < 0 || len > status)
			return;

		if (do_one_request(h) < 0)
			return;

		status -= NLMSG_ALIGN(len);
		h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
	}
}

void rtnl_close(struct rtnl_handle *rth)
{
	if (rth->fd >= 0) {
		close(rth->fd);
		rth->fd = -1;
	}
}

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned subscriptions,
		      int protocol)
{
	socklen_t addr_len;
	int sndbuf = 32768;
	int rcvbuf = 32768;

	memset(rth, 0, sizeof(*rth));//qiuchen

	rth->fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (rth->fd < 0) {
		perror("Cannot open netlink socket");
		return -1;
	}

	if (setsockopt(rth->fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) {
		perror("SO_SNDBUF");
		return -1;
	}

	if (setsockopt(rth->fd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf)) < 0) {
		perror("SO_RCVBUF");
		return -1;
	}

	fcntl(rth->fd, F_SETFL, O_NONBLOCK);
	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = subscriptions;

	if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0) {
		perror("Cannot bind netlink socket");
		return -1;
	}
	addr_len = sizeof(rth->local);
	if (getsockname(rth->fd, (struct sockaddr*)&rth->local, &addr_len) < 0) {
		perror("Cannot getsockname");
		return -1;
	}
	if (addr_len != sizeof(rth->local)) {
		fprintf(stderr, "Wrong address length %d\n", addr_len);
		return -1;
	}
	if (rth->local.nl_family != AF_NETLINK) {
		fprintf(stderr, "Wrong address family %d\n", rth->local.nl_family);
		return -1;
	}
	rth->seq = time(NULL);
	return 0;
}

int rtnl_open(struct rtnl_handle *rth, unsigned subscriptions)
{
	return rtnl_open_byproto(rth, subscriptions, NETLINK_ROUTE);
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

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = rth->dump = ++rth->seq;
	req.g.rtgen_family = family;

	return sendto(rth->fd, (void*)&req, sizeof(req), 0,
		      (struct sockaddr*)&nladdr, sizeof(nladdr));
}

int rtnl_send(struct rtnl_handle *rth, const char *buf, int len)
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
	struct iovec iov[2] = {
		{ .iov_base = &nlh, .iov_len = sizeof(nlh) },
		{ .iov_base = req, .iov_len = len }
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = 	sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = 2,
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
		     rtnl_filter_t filter,
		     void *arg1,
		     rtnl_filter_t junk,
		     void *arg2)
{
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char buf[16384];

	iov.iov_base = buf;
	while (1) {
		int status;
		struct nlmsghdr *h;

		iov.iov_len = sizeof(buf);
		status = recvmsg(rth->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			perror("OVERRUN");
			//continue;
			return -1;
		}

		if (status == 0) {
			fprintf(stderr, "EOF on netlink\n");
			return -1;
		}

		h = (struct nlmsghdr*)buf;
		while (NLMSG_OK(h, status)) {
			int err;

			if (nladdr.nl_pid != 0 ||
			    h->nlmsg_pid != rth->local.nl_pid ||
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
					perror("RTNETLINK answers");
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
			//exit(1);
			return -1;
		}
	}
}

int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
	      unsigned groups, struct nlmsghdr *answer,
	      rtnl_filter_t junk,
	      void *jarg)
{
	int status;
	unsigned seq;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = (void*) n,
		.iov_len = n->nlmsg_len
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char   buf[16384];

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = peer;
	nladdr.nl_groups = groups;

	n->nlmsg_seq = seq = ++rtnl->seq;

	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);

	if (status < 0) {
		perror("Cannot talk to rtnetlink");
		return -1;
	}

	memset(buf,0,sizeof(buf));

	iov.iov_base = buf;

	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			perror("OVERRUN");
			//continue;
			return -1;
		}
		if (status == 0) {
			fprintf(stderr, "EOF on netlink\n");
			return -1;
		}
		if (msg.msg_namelen != sizeof(nladdr)) {
			fprintf(stderr, "sender address length == %d\n", msg.msg_namelen);
			//exit(1);
			return -1;
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
				//exit(1);
				return -1;
			}

			if (nladdr.nl_pid != peer ||
			    h->nlmsg_pid != rtnl->local.nl_pid ||
			    h->nlmsg_seq != seq) {
				if (junk) {
					err = junk(&nladdr, h, jarg);
					if (err < 0)
						return err;
				}
				/* Don't forget to skip that message. */
				status -= NLMSG_ALIGN(len);
				h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
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
					perror("RTNETLINK answers");
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
			//exit(1);
			return -1;
		}
	}
}

int rtnl_listen(struct rtnl_handle *rtnl,
		rtnl_filter_t handler,
		void *jarg)
{
	int status;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char   buf[8192];

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = 0;
	nladdr.nl_groups = 0;

	iov.iov_base = buf;
	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			perror("OVERRUN");
			continue;
		}
		if (status == 0) {
			fprintf(stderr, "EOF on netlink\n");
			return -1;
		}
		if (msg.msg_namelen != sizeof(nladdr)) {
			fprintf(stderr, "Sender address length == %d\n", msg.msg_namelen);
			//exit(1);
			return -1;
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
				//exit(1);
				return -1;
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
			//exit(1);
			return -1;
		}
	}
}

int rtnl_from_file(FILE *rtnl, rtnl_filter_t handler,
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
		int l;

		status = fread(&buf, 1, sizeof(*h), rtnl);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			perror("rtnl_from_file: fread");
			return -1;
		}
		if (status == 0)
			return 0;

		len = h->nlmsg_len;
		type= h->nlmsg_type;
		l = len - sizeof(*h);

		if (l<0 || len>sizeof(buf)) {
			fprintf(stderr, "!!!malformed message: len=%d @%lu\n",
				len, ftell(rtnl));
			return -1;
		}

		status = fread(NLMSG_DATA(h), 1, NLMSG_ALIGN(l), rtnl);

		if (status < 0) {
			perror("rtnl_from_file: fread");
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
	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen) {
		fprintf(stderr,"addattr32: Error! max allowed bound %d exceeded\n",maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		fprintf(stderr, "addattr_l ERROR: message exceeded bound of %d\n",maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}

int addraw_l(struct nlmsghdr *n, int maxlen, const void *data, int len)
{
	if (NLMSG_ALIGN(n->nlmsg_len) + NLMSG_ALIGN(len) > maxlen) {
		fprintf(stderr, "addraw_l ERROR: message exceeded bound of %d\n",maxlen);
		return -1;
	}

	memcpy(NLMSG_TAIL(n), data, len);
	memset((void *) NLMSG_TAIL(n) + len, 0, NLMSG_ALIGN(len) - len);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + NLMSG_ALIGN(len);
	return 0;
}

int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data)
{
	int len = RTA_LENGTH(4);
	struct rtattr *subrta;

	if (RTA_ALIGN(rta->rta_len) + len > maxlen) {
		fprintf(stderr,"rta_addattr32: Error! max allowed bound %d exceeded\n",maxlen);
		return -1;
	}
	subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	memcpy(RTA_DATA(subrta), &data, 4);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;
	return 0;
}

int rta_addattr_l(struct rtattr *rta, int maxlen, int type,
		  const void *data, int alen)
{
	struct rtattr *subrta;
	int len = RTA_LENGTH(alen);

	if (RTA_ALIGN(rta->rta_len) + RTA_ALIGN(len) > maxlen) {
		fprintf(stderr,"rta_addattr_l: Error! max allowed bound %d exceeded\n",maxlen);
		return -1;
	}
	subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	memcpy(RTA_DATA(subrta), data, alen);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + RTA_ALIGN(len);
	return 0;
}

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return 0;
}

int parse_rtattr_byindex(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	int i = 0;

	memset(tb, 0, sizeof(struct rtattr *) * max);
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max && i < max)
			tb[i++] = rta;
		rta = RTA_NEXT(rta,len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return i;
}


int get_addr_1(inet_prefix *addr, const char *name, int family)
{
	const char *cp;
	unsigned char *ap = (unsigned char*)addr->data;
	int i;

	memset(addr, 0, sizeof(*addr));

	if (strcmp(name, "default") == 0 ||
	    strcmp(name, "all") == 0 ||
	    strcmp(name, "any") == 0) {
		if (family == AF_DECnet)
			return -1;
		addr->family = family;
		addr->bytelen = (family == AF_INET6 ? 16 : 4);
		addr->bitlen = -1;
		return 0;
	}

	if (strchr(name, ':')) {
		addr->family = AF_INET6;
		if (family != AF_UNSPEC && family != AF_INET6)
			return -1;
		if (inet_pton(AF_INET6, name, addr->data) <= 0)
			return -1;
		addr->bytelen = 16;
		addr->bitlen = -1;
		return 0;
	}
#if 0 
	if (family == AF_DECnet) {
		struct dn_naddr dna;
		addr->family = AF_DECnet;
		if (dnet_pton(AF_DECnet, name, &dna) <= 0)
			return -1;
		memcpy(addr->data, dna.a_addr, 2);
		addr->bytelen = 2;
		addr->bitlen = -1;
		return 0;
	}
#endif
	addr->family = AF_INET;
	if (family != AF_UNSPEC && family != AF_INET)
		return -1;
	addr->bytelen = 4;
	addr->bitlen = -1;
	for (cp=name, i=0; *cp; cp++) {
		if (*cp <= '9' && *cp >= '0') {
			ap[i] = 10*ap[i] + (*cp-'0');
			continue;
		}
		if (*cp == '.' && ++i <= 3)
			continue;
		return -1;
	}
	return 0;
}

int get_addr(inet_prefix *dst, const char *arg, int family)
{
	if (family == AF_PACKET) {
		fprintf(stderr, "Error: \"%s\" may be inet address, but it is not allowed in this context.\n", arg);
		//exit(1);
		return -1;
	}
	if (get_addr_1(dst, arg, family)) {
		fprintf(stderr, "Error: an inet address is expected rather than \"%s\".\n", arg);
		//exit(1);
		return -1;
	}
	return 0;
}

const char *ll_addr_n2a(unsigned char *addr, int alen, int type, char *buf, int blen)
{
	int i;
	int l;

	if (alen == 4 &&
	    (type == ARPHRD_TUNNEL || type == ARPHRD_SIT || type == ARPHRD_IPGRE)) {
		return inet_ntop(AF_INET, addr, buf, blen);
	}
	l = 0;
	for (i=0; i<alen; i++) {
		if (i==0) {
			snprintf(buf+l, blen, "%02x", addr[i]);
			blen -= 2;
			l += 2;
		} else {
			snprintf(buf+l, blen, ":%02x", addr[i]);
			blen -= 3;
			l += 3;
		}
	}
	return buf;
}

/*NB: lladdr is char * (rather than u8 *) because sa_data is char * (1003.1g) */
int ll_addr_a2n(char *lladdr, int len, char *arg)
{
	if (strchr(arg, '.')) {
		inet_prefix pfx;
		if (get_addr_1(&pfx, arg, AF_INET)) {
			fprintf(stderr, "\"%s\" is invalid lladdr.\n", arg);
			return -1;
		}
		if (len < 4)
			return -1;
		memcpy(lladdr, pfx.data, 4);
		return 4;
	} else {
		int i;

		for (i=0; i<len; i++) {
			int temp;
			char *cp = strchr(arg, ':');
			if (cp) {
				*cp = 0;
				cp++;
			}
			if (sscanf(arg, "%x", &temp) != 1) {
				fprintf(stderr, "\"%s\" is invalid lladdr.\n", arg);
				return -1;
			}
			if (temp < 0 || temp > 255) {
				fprintf(stderr, "\"%s\" is invalid lladdr.\n", arg);
				return -1;
			}
			lladdr[i] = temp;
			if (!cp)
				break;
			arg = cp;
		}
		return i+1;
	}
}

int ll_remember_index(const struct sockaddr_nl *who, 
		      struct nlmsghdr *n, void *arg)
{
	int h;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct idxmap *im, **imp;
	struct rtattr *tb[IFLA_MAX+1];

	if (n->nlmsg_type != RTM_NEWLINK)
		return 0;

	if (n->nlmsg_len < NLMSG_LENGTH(sizeof(ifi)))
		return -1;


	memset(tb, 0, sizeof(tb));
	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), IFLA_PAYLOAD(n));
	if (tb[IFLA_IFNAME] == NULL)
		return 0;

	h = ifi->ifi_index&0xF;

	for (imp=&idxmap[h]; (im=*imp)!=NULL; imp = &im->next)
		if (im->index == ifi->ifi_index)
			break;

	if (im == NULL) {
		im = malloc(sizeof(*im));
		if (im == NULL)
			return 0;
		im->next = *imp;
		im->index = ifi->ifi_index;
		*imp = im;
	}

	im->type = ifi->ifi_type;
	im->flags = ifi->ifi_flags;
	if (tb[IFLA_ADDRESS]) {
		int alen;
		im->alen = alen = RTA_PAYLOAD(tb[IFLA_ADDRESS]);
		if (alen > sizeof(im->addr))
			alen = sizeof(im->addr);
		memcpy(im->addr, RTA_DATA(tb[IFLA_ADDRESS]), alen);
	} else {
		im->alen = 0;
		memset(im->addr, 0, sizeof(im->addr));
	}
	strcpy(im->name, RTA_DATA(tb[IFLA_IFNAME]));
	return 0;
}
const char *ll_idx_n2a(unsigned idx, char *buf)
{
	struct idxmap *im;

	if (idx == 0)
		return "*";
	for (im = idxmap[idx&0xF]; im; im = im->next)
		if (im->index == idx)
			return im->name;
	snprintf(buf, 16, "if%d", idx);
	return buf;
}


const char *ll_index_to_name(unsigned idx)
{
	static char nbuf[16];

	return ll_idx_n2a(idx, nbuf);
}

int ll_index_to_type(unsigned idx)
{
	struct idxmap *im;

	if (idx == 0)
		return -1;
	for (im = idxmap[idx&0xF]; im; im = im->next)
		if (im->index == idx)
			return im->type;
	return -1;
}

unsigned ll_index_to_flags(unsigned idx)
{
	struct idxmap *im;

	if (idx == 0)
		return 0;

	for (im = idxmap[idx&0xF]; im; im = im->next)
		if (im->index == idx)
			return im->flags;
	return 0;
}

unsigned ll_name_to_index(const char *name)
{
	static char ncache[16];
	static int icache;
	struct idxmap *im;
	int i;

	if (name == NULL)
		return 0;
	if (icache && strcmp(name, ncache) == 0)
		return icache;
	for (i=0; i<16; i++) {
		for (im = idxmap[i]; im; im = im->next) {
			if (strcmp(im->name, name) == 0) {
				icache = im->index;
				strcpy(ncache, name);
				return im->index;
			}
		}
	}

	return if_nametoindex(name);
}

int ll_init_map(struct rtnl_handle *rth)
{
	if (rtnl_wilddump_request(rth, AF_UNSPEC, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		//exit(1);
		return -1;
	}

	if (rtnl_dump_filter(rth, ll_remember_index, &idxmap, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		//exit(1);
		return -1;
	}
	return 0;
}

int ipneigh_modify(int cmd, int flags, char *ip, char * mac, char * dev)
{
	struct {
		struct nlmsghdr 	n;
		struct ndmsg 		ndm;
		char   			buf[256];
	} req;
	char  *d = NULL;
	//int dst_ok = 0;
	//int lladdr_ok = 0;
	char * lla = NULL;
	inet_prefix dst;
	int preferred_family = AF_UNSPEC;
	printf("%s 1\n",__func__);

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST|flags;
	req.n.nlmsg_type = cmd;
	req.ndm.ndm_family = preferred_family;
	req.ndm.ndm_state = NUD_PERMANENT;
	lla = mac;
	d = dev;	
	get_addr(&dst, ip, preferred_family);
	printf("%s 1\n",__func__);

	if (d == NULL || dst.family == AF_UNSPEC) {
		fprintf(stderr, "Device and destination are required arguments.\n");
		return ASD_IFNAME_NOT_EXIST;
	}
	printf("%s 2\n",__func__);

	req.ndm.ndm_family = dst.family;
	addattr_l(&req.n, sizeof(req), NDA_DST, &dst.data, dst.bytelen);
	printf("%s 3\n",__func__);

	if (lla && strcmp(lla, "null")) {
		char llabuf[20];
		int l;

		l = ll_addr_a2n(llabuf, sizeof(llabuf), lla);
		if(l < 0)//Qiuchen
			return ASD_DBUS_ERROR;
		addattr_l(&req.n, sizeof(req), NDA_LLADDR, llabuf, l);
	}
	printf("%s 4\n",__func__);

	ll_init_map(&rth);
	printf("%s 5\n",__func__);

	if ((req.ndm.ndm_ifindex = ll_name_to_index(d)) == 0) {
		fprintf(stderr, "Cannot find device \"%s\"\n", d);
		return ASD_IFNAME_NOT_EXIST;
	}
	printf("%s 6\n",__func__);

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		return ASD_DBUS_ERROR;
	printf("%s 7\n",__func__);

	return 0;
}



