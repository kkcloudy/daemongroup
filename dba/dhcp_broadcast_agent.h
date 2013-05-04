#ifndef __DBA_H__
#define __DBA_H__

#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/udp.h>

#include "dba/dba.h"	/* accapi/dba/dba.h */


#define DHCP_SERVER_PORT	0x43
#define DHCP_CLIENT_PORT	0x44

#define DHCP_CLIENT_REQUEST	0x01
#define DHCP_SERVER_REPLY	0x02

#define DHCP_UDP_OVERHEAD	(20 + /* IP header */			\
			        8)   /* UDP header */
#define DHCP_SNAME_LEN		64
#define DHCP_FILE_LEN		128
#define DHCP_FIXED_NON_UDP	236
#define DHCP_FIXED_LEN		(DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
						/* Everything but options. */
#define BOOTP_MIN_LEN		300

#define DHCP_MTU_MAX		1500
#define DHCP_MTU_MIN        576

#define DHCP_MAX_OPTION_LEN	(DHCP_MTU_MAX - DHCP_FIXED_LEN)
#define DHCP_MIN_OPTION_LEN     (DHCP_MTU_MIN - DHCP_FIXED_LEN)

struct dhcp_packet 
{
	u_int8_t  op;		/* 0: Message opcode/type */
	u_int8_t  htype;	/* 1: Hardware addr type (net/if_types.h) */
	u_int8_t  hlen;		/* 2: Hardware addr length */
	u_int8_t  hops;		/* 3: Number of relay agent hops from client */
	u_int32_t xid;		/* 4: Transaction ID */
	u_int16_t secs;		/* 8: Seconds since client started looking */
	u_int16_t flags;	/* 10: Flag bits */
	struct in_addr ciaddr;	/* 12: Client IP address (if already in use) */
	struct in_addr yiaddr;	/* 16: Client IP address */
	struct in_addr siaddr;	/* 18: IP address of next server to talk to */
	struct in_addr giaddr;	/* 20: DHCP relay agent IP address */
	unsigned char chaddr [16];	/* 24: Client hardware address */
	char sname [DHCP_SNAME_LEN];	/* 40: Server name */
	char file [DHCP_FILE_LEN];	/* 104: Boot filename */
	unsigned char options [DHCP_MAX_OPTION_LEN];
				/* 212: Optional parameters
			  (actual length dependent on MTU). */
};

struct dhcp_opt {
	u_int8_t  code;
	u_int8_t  len;
	unsigned char data[0];
};

int dhcp_broadcast_agent(struct sk_buff *skb, struct dhcp_packet *dhcp, dba_result_t *res);
int dhcp_option82_handle(struct sk_buff *skb, struct dhcp_packet *dhcp, dba_result_t *res);

#endif
