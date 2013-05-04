#ifndef __LINUX_IF_PPPOE_H
#define __LINUX_IF_PPPOE_H

#include <linux/types.h>
#include <asm/byteorder.h>

#include <linux/if_ether.h>
#include <linux/if.h>
#include <linux/netdevice.h>

#define PPPOE_MTU		1492
#define PPP_HEADER_LEN	2
#define TAG_HDR_SIZE	4	/* Header size of a PPPoE tag */
#define MAGIC_LEN		4	/* magic number length*/

#ifndef AF_PPPOE
#define AF_PPPOE		24
#define PF_PPPOE		AF_PPPOE
#define PX_PROTO_OE		0 
#endif /* !(AF_PPPOE) */

#ifndef NETLINK_PPPOE
#define NETLINK_PPPOE 	28
#endif


#define PPP_ECHO_REQUEST	0x09
#define PPP_ECHO_REPLY		0x0a

enum {
	PPPOE_MESSAGE_REQUEST,
	PPPOE_MESSAGE_REPLY,
	PPPOE_MESSAGE_SIGNAL,
};

enum {
	PPPOE_NETLINK_REGISTER = 1,
	PPPOE_NETLINK_UNREGISTER,
	PPPOE_INTERFACE_CREATE,
	PPPOE_INTERFACE_DESTROY,
	PPPOE_INTERFACE_BASE,
	PPPOE_INTERFACE_UNBASE,
	PPPOE_CHANNEL_REGISTER,
	PPPOE_CHANNEL_UNREGISTER,
	PPPOE_CHANNEL_AUTHORIZE,
	PPPOE_CHANNEL_UNAUTHORIZE,
	PPPOE_CHANNEL_CLEAR,
};


typedef __u16 sid_t; 
struct pppoe_addr{ 
	sid_t			sid;				/* Session identifier */ 
	unsigned char	mac[ETH_ALEN];		/* Mac address */ 
	char			dev[IFNAMSIZ];		/* Local device to use */ 
}; 

struct sockaddr_pppoe { 
	sa_family_t     	sa_family;		/* address family, AF_PPPOE */ 
	unsigned int    	sa_protocol;	/* protocol identifier */ 
	struct pppoe_addr	addr; 
}__attribute__ ((packed)); 

struct pppoe_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	ver : 4;
	__u8	type : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	type : 4;
	__u8	ver : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	code;
	__be16	sid;
	__be16	length;
	__u8	data[0];
} __attribute__ ((packed));


struct pppoe_ctl {
	__u8	code;
	__u8	ident;
	__be16	length;
	__u8	data[0];
} __attribute ((packed));


/* PPPoE Packet, including Ethernet headers */
struct pppoe_packet {
	struct	ethhdr ethHdr;		/* Ethernet header */
	struct	pppoe_hdr phdr;
};

struct discover_tag {
	__u16	type;
	__u16	length;
	__u8	payload[PPPOE_MTU]; /* A bit of room to spare */
} __attribute ((packed));


struct pppoe_deamon_msg {
	__u32 local_id;
	__u32 instance_id;
};

struct pppoe_register_msg {
	int		ifindex;
	__u16	sid;
	__u8	magic[MAGIC_LEN];
	__u8	mac[ETH_ALEN];
	__u8	serverMac[ETH_ALEN];
};

struct pppoe_authorize_msg {
	int 	ifindex;
	__u16 	sid;
	__u32 	ip;
};

struct pppoe_interface_msg {
	int 	ifindex;
	char	name[IFNAMSIZ];
};	

struct pppoe_message {
	__u8	type;
	__u8	code;	
	__u16	datalen;
	__u32	errorcode;
	__u32	checkcode;
	union {
		struct pppoe_deamon_msg		m_deamon;
		struct pppoe_interface_msg	m_interface;
		struct pppoe_register_msg	m_register;	
		struct pppoe_authorize_msg	m_authorize;
	} data;
};

#endif
