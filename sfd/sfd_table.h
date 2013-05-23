#ifndef __SFD_TABLE_H__
#define __SFD_TABLE_H__

#include <linux/if_ether.h>

struct sfd_user {
	unsigned int key;
	unsigned int syn;
	unsigned int icmp;
	unsigned int snmp;
	unsigned int capwap_contl;
	unsigned char mac[ETH_ALEN];
	union {
		__be32 ipv4addr;
#ifdef SUPPORT_IPV6
		struct in6_addr ipv6addr;
#endif
	}ip;
	unsigned int isStatic;
	//struct sfd_user *pre;
	struct sfd_user *next;
};

struct sfd_arper {
	unsigned int key;
	unsigned int packet;
	unsigned char mac[ETH_ALEN];
	//struct sfd_arper *pre;
	unsigned int isStatic;
	struct sfd_arper *next;
};

#endif /* __SFD_TABLE_H__ */
