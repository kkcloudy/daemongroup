#ifndef IUH_IFLISTEN_H
#define IUH_IFLISTEN_H
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


int Iuh_Interface_Listen_init(void);
int netlink_information_fetch(struct sockaddr_nl *snl, struct nlmsghdr *h);
int netlink_parse_info(int (*filter) (struct sockaddr_nl *, struct nlmsghdr *),struct nlsock *nl);

#endif
