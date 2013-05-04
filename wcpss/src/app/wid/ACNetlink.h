#ifndef ACNETLINK_H
#define ACNETLINK_H
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct nlsock {
	int sock;
	int seq;
	struct sockaddr_nl snl;
	const char *name;
};
extern struct nlsock netlink;
extern struct nlsock netlink_cmd;

int WID_Interface_Listen_init(void);
int netlink_information_fetch(struct sockaddr_nl *snl, struct nlmsghdr *h);
int netlink_parse_info(int (*filter) (struct sockaddr_nl *, struct nlmsghdr *),struct nlsock *nl);
CWBool WIDWsm_VRRPIFOp_IPv6(struct ifi_info *ifi, unsigned int op);

#endif
