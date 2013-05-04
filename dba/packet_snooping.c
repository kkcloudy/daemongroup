#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/igmp.h>


#include "log.h"
#include "main.h"
#include "pppoe_snooping.h"
#include "dhcp_broadcast_agent.h"
#include "packet_snooping.h"

#include "dba/dba.h"	/* accapi/dba/dba.h */

static int pares_ipv4_packet(struct sk_buff *skb, struct iphdr *iphdr, dba_result_t *res);
static int parse_icmp_packet(struct sk_buff *skb, struct icmphdr *icmph, dba_result_t *res);
static int parse_igmp_packet(struct sk_buff *skb, struct igmphdr *igmph, dba_result_t *res);
static int parse_tcp_packet(struct sk_buff *skb, 	struct tcphdr *tcphdr, dba_result_t *res);
static int parse_udp_packet(struct sk_buff *skb, struct udphdr *udphdr, dba_result_t *res);

int test_option82(struct sk_buff *skb, struct udphdr *udphdr);



int packet_snooping(struct sk_buff *skb, dba_result_t *res)
{
	int ret = 0;
	__be16	h_proto = 0;		/* packet type ID field	*/
	struct ethhdr *ethhdr = NULL;
	struct vlan_hdr *vlanhdr = NULL;

	if (unlikely(!skb)) {
		return -1;
	}

	ethhdr = eth_hdr(skb);
	/*
	printk(KERN_INFO "src haddr %02x:%02x:%02x:%02x:%02x:%02x" 
		"dest haddr %02x:%02x:%02x:%02x:%02x:%02x protol %04x\n",
		ethhdr->h_dest[0], ethhdr->h_dest[1], ethhdr->h_dest[2],
		ethhdr->h_dest[3], ethhdr->h_dest[4], ethhdr->h_dest[5],
		ethhdr->h_source[0], ethhdr->h_source[1], ethhdr->h_source[2],
		ethhdr->h_source[3], ethhdr->h_source[4], ethhdr->h_source[5],
		ntohs(ethhdr->h_proto));	*/

	h_proto = ntohs(ethhdr->h_proto);

again:
	switch (h_proto) {
		case ETH_P_IP:
			ret = pares_ipv4_packet(skb, (struct iphdr *)(ethhdr + 1), res);
			break;

		case ETH_P_ARP:
			break;

		case ETH_P_PPP_DISC:
			break; 

		case ETH_P_PPP_SES:			/* pppoe session */
			ret = parse_pppoe_session(skb, (struct pppoe_hdr *)(ethhdr + 1), res);		
			break; 

		case ETH_P_8021Q:		/* vlan tag */
		case ETH_P_8021AD:
		case 0x9100:
			vlanhdr = (struct vlan_hdr *)(ethhdr + 1);	/* vlan header */
			h_proto = ntohs(vlanhdr->h_vlan_encapsulated_proto);/* next protocol */
			ethhdr = (unsigned char *)ethhdr + 4;
			goto again;
			break;

		case ETH_P_IPV6:
			break; 

		case ETH_P_TIPC:
			break; 
		
		default:
			break;
	}
	return ret;
}


static int pares_ipv4_packet(struct sk_buff *skb, struct iphdr *iphdr, dba_result_t *res)
{
	int ret = 0;
	
	if (unlikely(!iphdr)) {
		return -1;
	}
	
	switch (iphdr->protocol) {
		case IPPROTO_ICMP:
			ret = parse_icmp_packet(skb, (struct icmphdr *)(IPv4_NXT_HDR(iphdr)), res);
			break;
		case IPPROTO_IGMP:
			ret = parse_igmp_packet(skb, (struct igmphdr *)(IPv4_NXT_HDR(iphdr)), res);
			break;
		case IPPROTO_TCP:
			ret = parse_tcp_packet(skb, (struct tcphdr *)(IPv4_NXT_HDR(iphdr)), res);
			break;
		case IPPROTO_UDP:
			ret = parse_udp_packet(skb, (struct udphdr *)(IPv4_NXT_HDR(iphdr)), res);			
			break;
		case IPPROTO_IPV6:
			break;		
		default:
			break;
	}
	return ret;
}

static int parse_icmp_packet(struct sk_buff *skb, struct icmphdr *icmph, dba_result_t *res)
{
	return 0;
}

static int parse_igmp_packet(struct sk_buff *skb, struct igmphdr *igmph, dba_result_t *res)
{
	return 0;
}

static int parse_tcp_packet(struct sk_buff *skb, 	struct tcphdr *tcphdr, dba_result_t *res)
{
	return 0;
}

static int parse_udp_packet(struct sk_buff *skb, struct udphdr *udphdr, dba_result_t *res)
{
	int ret = 0;
	
	if (unlikely(!udphdr)) {
		return -1;
	}

	/* DBA(DHCP Broadcast Agent) */
	if (DHCP_SERVER_PORT == ntohs(udphdr->source)) {
		ret = dhcp_broadcast_agent(skb, (struct dhcp_packet *)(udphdr + 1), res);
	}
	
	/* REQUIREMENTS-428 REQUIREMENTS-456 dhcp option82 */
	if ((DHCP_CLIENT_PORT == ntohs(udphdr->source)) 
		|| (DHCP_SERVER_PORT == ntohs(udphdr->source))) {
		ret = dhcp_option82_handle(skb, (struct dhcp_packet *)(udphdr + 1), res);
	}
	/*
	test_option82(skb, udphdr); */

	return ret;
}


#if 0
int test_option82(struct sk_buff *skb, struct udphdr *udphdr)
{
	
	int ret = 0;
	int i = 0;
	int subopt1_len = 126;
	int subopt2_len = 122;
	dba_result_t opt;
	unsigned char buf[256];

	memset(buf, 0, sizeof(buf));

	opt.module_type = DHCP_OPTION82_KMOD;		/* module type */
	opt.len = subopt1_len + subopt2_len + 4;	/* length */
	opt.data = buf;

	/* suboption 1 */
	buf[0] = 0x01;						/* suboption 1 code */
	buf[1] = subopt1_len;				/* suboption 1 length */
	for (i=2; i<2+subopt1_len; i++) {	/* suboption 1 data */
		buf[i] = i;
	}

	/* suboption 2 */
	buf[subopt1_len+2] = 0x02;			/* suboption 2 code */
	buf[subopt1_len+3] = subopt2_len;	/* suboption 2 length */
	for (i=subopt1_len+4; i<subopt1_len+4+subopt2_len; i++) {/* suboption 2 data */
		buf[i] = i;
	}

	ret = dhcp_option82_handle(skb, (struct dhcp_packet *)(udphdr + 1), &opt);

	return ret;
}
#endif

