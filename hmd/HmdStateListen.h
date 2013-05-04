#ifndef _HMD_STATE_LISTEN_
#define _HMD_STATE_LISTEN_
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct rtnl_handle
{
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
};

void * hmd_netlink_recv_thread();
int hmd_rtnl_open(struct rtnl_handle *rth, unsigned subscriptions);
int hmd_rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
			  unsigned groups, struct nlmsghdr *answer,
			  int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
			  void *jarg);
int hmd_rtnl_close(struct rtnl_handle *rth);
int hmd_ifname_to_idx( char *ifname );
int hmd_ifname_to_mac( char *ifname, char * mac);
int hmd_ipaddr_op_withmask( char *ifname , unsigned int addr, int mask,int addF );
int hmd_ipaddr_op_withmask( char *ifname , unsigned int addr, int mask,int addF );
int hmd_netlink_init(void);
int hmd_netlink_init_for_ksem(void);
#endif
