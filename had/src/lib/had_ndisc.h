#ifndef _HAD_VRRP_NDISC_H
#define _HAD_VRRP_NDISC_H

/* system includes */
#include <linux/icmpv6.h>

/* local definitions */
#define ETHERNET_HW_LEN 6
#define NEXTHDR_ICMP	58
#define NDISC_HOPLIMIT	255

/*
 *	ICMPv6 codes for Neighbour Discovery messages
 */
#define NDISC_ROUTER_SOLICITATION       133
#define NDISC_ROUTER_ADVERTISEMENT      134
#define NDISC_NEIGHBOUR_SOLICITATION    135
#define NDISC_NEIGHBOUR_ADVERTISEMENT   136
#define NDISC_REDIRECT                  137

/*
 *	Neighbour Discovery option codes
 */
#define ND_OPT_TARGET_LL_ADDR	2

/*
 *	IPv6 Header
 */
struct ip6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8			priority:4,
				version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8			version:4,
				priority:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	__u8			flow_lbl[3];

	__be16			payload_len;
	__u8			nexthdr;
	__u8			hop_limit;

	struct	in6_addr	saddr;
	struct	in6_addr	daddr;
};

/*
 *	NDISC Neighbour Advertisement related	
 */
struct ndhdr {
	struct icmp6hdr		icmph;
	struct in6_addr		target;
	__u8			opt[0];
};

struct nd_opt_hdr {
	__u8			nd_opt_type;
	__u8			nd_opt_len;
} __attribute__((__packed__));

void had_ndisc_thread_main
(
	void
);

#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6	 0x86dd
#endif
#endif

