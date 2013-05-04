#include <linux/kernel.h>
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/fs.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if_arp.h>
#include <linux/mii.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>

#include <net/sock.h>
#include <net/protocol.h>
#include <net/dst.h>
#include <net/checksum.h>
#include <net/xfrm.h>

#include <linux/semaphore.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/time.h>

#include "sfd.h"
#include "sfd_table.h"

#define FILTERALL 0

#define IPV4MSG_DATA(msg) ((void *)(((char *)msg) + sizeof(struct iphdr)))
#ifdef SUPPORT_IPV6
#define IPV6MSG_DATA(msg) ((void *)(((char *)msg) + sizeof(struct ipv6hdr)))
#endif
#define VLANMSG_DATA(msg) ((void *)(((char *)msg) + sizeof(struct vlan_ethhdr)))
#define UDPMSG_DATA(msg) ((void *)(((char *)msg) + sizeof(struct udphdr)))

struct dnshdr {
	__u16 id;
	__u16 flags;
	__u16 questions;
	__u16 ansrrs;
	__u16 authrrs;
	__u16 addrrs;
};

/*
 * dns
 * name string
 * type u16
 * class u16
 */

#define DNSMSG_DATA(msg) ((void *)(((char *)msg) + sizeof(struct dnshdr)))

extern int (*cvm_sfd_rx_hook)(struct sk_buff *);
extern int (*cvm_arp_rx_hook)(struct sk_buff *);

static struct semaphore utable_sema;
static struct semaphore atable_sema;

static struct task_struct *sfd_task_schedule;
static struct task_struct *arp_task_schedule;

static struct sock *netlink_sock;

static int sfd_pid;

static int log_enable = 0;
static int debug_mode = 0;

/* sfd */
#define FLAG_TCP  1
#define FLAG_ICMP 2
#define FLAG_SNMP 3

#define ENABLE_SFD 0x00

#define ENABLE_TCP  0x01
#define ENABLE_ICMP 0x02
#define ENABLE_SNMP 0x03
#define ENABLE_DNS 0x04

#define DISABLE_TCP  0x10
#define DISABLE_ICMP 0x20
#define DISABLE_SNMP 0x30
#define DISABLE_DNS 0x40

static int sfd_enable = 0;
static int tcp_enable = 0;
static int icmp_enable = 0;
static int snmp_enable = 0;
static int dns_enable = 0;


#define TIMESPAN_DEF 100

#define SNMP_PACKET_DEF 200
#define TCP_PACKET_DEF 5000
#define ICMP_PACKET_DEF 200
#define ARP_PACKET_DEF 200


static int timespan = TIMESPAN_DEF;
/*
static int limitpacket = 50;
*/
static int limitpacket_snmp = SNMP_PACKET_DEF;
static int limitpacket_tcp = TCP_PACKET_DEF;
static int limitpacket_icmp = ICMP_PACKET_DEF;



/* arp */
static int arp_enable = 0;
/*
static int arp_timespan = 1000;
*/
static int limitpacket_arp = ARP_PACKET_DEF;

static int sfd_schedule_thread(void *);
static int arp_schedule_thread(void *);

/* sfd table */
static struct sfd_user *utable_element_static_update(unsigned char *, void *, iptype);
static struct sfd_user *utable_element_dynamic_update(unsigned char *, void *, iptype);
static void utable_element_delete(unsigned char *);
static struct sfd_user *utable_element_check(unsigned char *, void *, iptype);
static void utable_element_reset(void);
static void utable_element_clear(void);

/* arp table */
static struct sfd_arper *atable_element_update(unsigned char *);
static void atable_element_clear(void);
static void atable_element_reset(void);

static struct sfd_arper *atable_element_dynamic_update(unsigned char *);
static struct sfd_arper *atable_element_static_update(unsigned char *);
static void atable_element_delete(unsigned char *);


static int print_log(SfdLogSeverity, char *format, ...);

static int parse_skb_packet_ipv4(unsigned char *, void *);
#ifdef SUPPORT_IPV6
static int parse_skb_packet_ipv6(unsigned char *, void *);
#endif
static int parse_skb_packet_vlan(unsigned char *, void *);
static int parse_skb_packet_arp(unsigned char *, void *);
static int parse_skb_packet_tcp(unsigned char *, void *, void *, iptype);
static int parse_skb_packet_udp(unsigned char *, void *, void *, iptype);
static int parse_skb_packet_icmp(unsigned char *, void *, void *, iptype);
static int parse_skb_packet_dnsquery(unsigned char *, void *, void *, iptype);
static int parse_skb_packet_snmp(unsigned char *, void *, void *, iptype);

static int send_netlinkmsg(int pid, sfdMsg *msg);

#define TABLE_SIZE 0xFF

static struct sfd_user *user_tables[TABLE_SIZE][TABLE_SIZE];
//static void *user_blk_tables[TABLE_SIZE][TABLE_SIZE];

static struct sfd_arper *arp_tables[TABLE_SIZE][TABLE_SIZE];
//static void *arp_blk_tables[TABLE_SIZE][TABLE_SIZE];

/* global functions */
int
sfd_catch_packet(struct sk_buff* packet)
{
	struct ethhdr *machdr = NULL;
	int ret = 0;

	if(unlikely(NULL == packet)) {
		//print_log(SFD_LOG_DEBUG, "catch a empty packet\n");
		return 1;
	}

	if(packet->mac_header) {
		machdr = eth_hdr(packet);
	}
	else {
		return 0;
	}

#if 0
	print_log(SFD_LOG_DEBUG,
				"Source mac: %02x:%02x:%02x:%02x:%02x:%02x, "
				"Dst mac: %02x:%02x:%02x:%02x:%02x:%02x, "
				"IP Type: 0x%02x%02x\n",
				machdr->h_source[0], machdr->h_source[1], machdr->h_source[2],
				machdr->h_source[3], machdr->h_source[4], machdr->h_source[5],
				machdr->h_dest[0], machdr->h_dest[1], machdr->h_dest[2],
				machdr->h_dest[3], machdr->h_dest[4], machdr->h_dest[5],
				*(unsigned char *)(&(machdr->h_proto)), *((unsigned char *)(&(machdr->h_proto)) + 1));
#endif

	switch(ntohs(machdr->h_proto)) {
		case ETH_P_IP:
			ret = parse_skb_packet_ipv4(machdr->h_source, (void *)packet->data);
			break;
#ifdef SUPPORT_IPV6
		case ETH_P_IPV6:
			ret = parse_skb_packet_ipv6(machdr->h_source, (void *)packet->data);
			break;
#endif
		case ETH_P_8021Q:
			ret = parse_skb_packet_vlan(machdr->h_source, (void *)packet->data);
			break;
		default:
			break;
	}

	return ret;
}

int
arp_catch_packet(struct sk_buff* packet)
{
	struct ethhdr *machdr = NULL;
	int ret = 0;
	
	if(unlikely(NULL == packet)) {
		//print_log(SFD_LOG_DEBUG, "catch a empty packet\n");
		return 1;
	}

	if(packet->mac_header) {
		machdr = eth_hdr(packet);
	}
	else {
		return 0;
	}

	switch(ntohs(machdr->h_proto)) {
		case ETH_P_ARP:
			ret = parse_skb_packet_arp(machdr->h_source, (void *)packet->data);
			break;
		default:
			break;
	}

	return ret;
}

/* static functions */
/* sfd */
static struct sfd_user *
utable_element_static_update(unsigned char *mac, void *ip, iptype type)
{
	struct sfd_user *head = NULL, *tmp = NULL;
	unsigned int user_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = user_tables[tx][ty];
	user_key = *(unsigned int *)(mac + 2);

	for(tmp = head; tmp; tmp = tmp->next) {
		if(user_key == tmp->key) {
			tmp->isStatic = 1;
			if(type == sfdiptype_ipv4) {
				tmp->ip.ipv4addr = *(__be32 *)ip;
			}
#ifdef SUPPORT_IPV6
			else if(type == sfdiptype_ipv6) {
				memcpy(&(tmp->ip.ipv6addr), ip, sizeof(struct in6_addr));
			}
#endif
			tmp->syn = 0;
			tmp->icmp = 0;
			tmp->snmp = 0;
			break;
		}
	}

	if(!tmp) {
		tmp = kmalloc(sizeof(struct sfd_user), GFP_ATOMIC);
		tmp->key = user_key;
		tmp->syn = 0;
		tmp->icmp = 0;
		tmp->snmp = 0;
		tmp->isStatic = 1;

		memcpy(tmp->mac, mac, ETH_ALEN);
		if(type == sfdiptype_ipv4) {
			tmp->ip.ipv4addr = *(__be32 *)ip;
		}
#ifdef SUPPORT_IPV6
		else if(type == sfdiptype_ipv6) {
			memcpy(&(tmp->ip.ipv6addr), ip, sizeof(struct in6_addr));
		}
#endif
		tmp->next = user_tables[tx][ty];
		user_tables[tx][ty] = tmp;
	}

	return tmp;
}

static struct sfd_user *
utable_element_dynamic_update(unsigned char *mac, void *ip, iptype type)
{
	struct sfd_user *head = NULL, *tmp = NULL;
	unsigned int user_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = user_tables[tx][ty];
	user_key = *(unsigned int *)(mac + 2);

	for(tmp = head; tmp; tmp = tmp->next) {
		if(user_key == tmp->key) {
			if(type == sfdiptype_ipv4) {
				tmp->ip.ipv4addr = *(__be32 *)ip;
			}
#ifdef SUPPORT_IPV6
			else if(type == sfdiptype_ipv6) {
				memcpy(&(tmp->ip.ipv6addr), ip, sizeof(struct in6_addr));
			}
#endif
			tmp->syn = 0;
			tmp->icmp = 0;
			tmp->snmp = 0;
			tmp->isStatic = 0;
			break;
		}
	}

	if(!tmp) {
		tmp = kmalloc(sizeof(struct sfd_user), GFP_ATOMIC);
		tmp->key = user_key;
		tmp->syn = 0;
		tmp->icmp = 0;
		tmp->snmp = 0;
		tmp->isStatic = 0;

		memcpy(tmp->mac, mac, ETH_ALEN);
		if(type == sfdiptype_ipv4) {
			tmp->ip.ipv4addr = *(__be32 *)ip;
		}
#ifdef SUPPORT_IPV6
		else if(type == sfdiptype_ipv6) {
			memcpy(&(tmp->ip.ipv6addr), ip, sizeof(struct in6_addr));
		}
#endif
		tmp->next = user_tables[tx][ty];
		user_tables[tx][ty] = tmp;
	}

	return tmp;
}

static void
utable_element_delete(unsigned char *mac)
{
	struct sfd_user *head = NULL, *tmp = NULL, *pre = NULL;
	unsigned int user_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = user_tables[tx][ty];
	user_key = *(unsigned int *)(mac + 2);

	for(tmp = head; tmp; tmp = tmp->next) {
		if(user_key == tmp->key) {
			break;
		}
		pre = tmp;
	}

	if(!tmp) {
		print_log(SFD_LOG_ERR,
					"can't find mac = %02X:%02X:%02X:%02X:%02X:%02X "
					"key = 0x%08X in table[0x%02X][0x%02X]\n",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
					user_key, tx, ty);
		return ;
	}

	if(tmp == head) {
		user_tables[tx][ty] = (tmp->next);
	}
	else {
		pre->next = tmp->next;
	}

	kfree(tmp);
	return ;
}

static struct sfd_user *
utable_element_check(unsigned char *mac, void *ip, iptype type)
{
	struct sfd_user *head = NULL, *tmp = NULL;
	unsigned int user_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = user_tables[tx][ty];
	user_key = *(unsigned int *)(mac + 2);

	for(tmp = head; tmp; tmp = tmp->next) {
		if(user_key == tmp->key) {
			if(type == sfdiptype_ipv4 &&
				tmp->ip.ipv4addr == *(__be32 *)ip) {
				break;
			}
#ifdef SUPPORT_IPV6
			else if(type == sfdiptype_ipv6 &&
				!memcmp(&(tmp->ip.ipv6addr), ip, sizeof(struct in6_addr))) {
				break;
			}
#endif
		}
	}
	
	return tmp;
}

static void
utable_element_reset()
{
	struct sfd_user *tmp = NULL;
	
	int m, n;
	/*
	**jiffies,HZ=100**
	*
	struct timeval cur_time;
	do_gettimeofday(&cur_time);
	print_log(SFD_LOG_INFO,"sfd reset time:%u.%u\n",cur_time.tv_sec,cur_time.tv_usec);
	*/
	for(m = 0; m < TABLE_SIZE; ++m) {
		for(n = 0; n < TABLE_SIZE; ++n) {
			tmp = user_tables[m][n];
			while(tmp) {
				tmp->syn = 0;
				tmp->icmp = 0;
				tmp->snmp = 0;
				tmp = tmp->next;
			}
		}
	}

	return ;
}

static void
utable_element_clear()
{
	struct sfd_user *tmp = NULL, *next = NULL;
	int m, n;

	for(m = 0; m < TABLE_SIZE; ++m) {
		for(n = 0; n < TABLE_SIZE; ++n) {
			tmp = user_tables[m][n];
			while(tmp) {
				next = tmp->next;
				kfree(tmp);
				tmp = next;
			}
			user_tables[m][n] = NULL;
		}
	}

	return ;
}

/* arp */
static struct sfd_arper *
atable_element_static_update(unsigned char *mac)
{
	struct sfd_arper *head = NULL, *tmp = NULL;
	unsigned int arp_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = arp_tables[tx][ty];
	arp_key = *(unsigned int *)(mac + 2);

#if 0
	print_log(SFD_LOG_DEBUG,
				"arp mac = %02X:%02X:%02X:%02X:%02X:%02X in "
				"table[0x%02X][0x%02X], key = 0x%08X\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
				tx, ty, arp_key);
#endif

	for(tmp = head; tmp; tmp = tmp->next) {
		if(arp_key == tmp->key) {
			tmp->isStatic = 1;
			//print_log(SFD_LOG_DEBUG, "found!\n");
			break;
		}
	}

	if(!tmp) {
		tmp = kmalloc(sizeof(struct sfd_arper), GFP_ATOMIC);
		tmp->key = arp_key;
		tmp->packet = 0;
		tmp->isStatic = 1;

		memcpy(tmp->mac, mac, ETH_ALEN);
		tmp->next = arp_tables[tx][ty];
		arp_tables[tx][ty] = tmp;
	}

	return tmp;
}

static struct sfd_arper *
atable_element_dynamic_update(unsigned char *mac)
{
	struct sfd_arper *head = NULL, *tmp = NULL;
	unsigned int arp_key = 0, tx = 0, ty = 0;

	tx = mac[0];
	ty = mac[1];

	head = arp_tables[tx][ty];
	arp_key = *(unsigned int *)(mac + 2);

#if 0
	print_log(SFD_LOG_DEBUG,
				"arp mac = %02X:%02X:%02X:%02X:%02X:%02X in "
				"table[0x%02X][0x%02X], key = 0x%08X\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
				tx, ty, arp_key);
#endif

	for(tmp = head; tmp; tmp = tmp->next) {
		if(arp_key == tmp->key) {
			print_log(SFD_LOG_DEBUG, "arp found!\n");
			break;
		}
	}

	if(!tmp) {
		tmp = kmalloc(sizeof(struct sfd_arper), GFP_ATOMIC);
		tmp->key = arp_key;
		tmp->packet = 0;
		tmp->isStatic = 0;

		memcpy(tmp->mac, mac, ETH_ALEN);
		tmp->next = arp_tables[tx][ty];
		arp_tables[tx][ty] = tmp;
	}

	return tmp;
}

static void
atable_element_delete(unsigned char *mac)
{
	
	struct sfd_arper *head = NULL, *tmp = NULL,*pre = NULL;
	unsigned int arp_key = 0, tx = 0, ty = 0;	

	tx = mac[0];
	ty = mac[1];

	head =  arp_tables[tx][ty];
	arp_key = *(unsigned int *)(mac + 2);

	for(tmp = head; tmp; tmp = tmp->next) {
		if(arp_key == tmp->key) {
			break;
		}
		pre = tmp;
	}

	if(!tmp) {
		print_log(SFD_LOG_ERR,
					"can't find mac = %02X:%02X:%02X:%02X:%02X:%02X "
					"key = 0x%08X in arp table[0x%02X][0x%02X]\n",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
					arp_key, tx, ty);
		return ;
	}

	if(tmp == head) {
		arp_tables[tx][ty] = (tmp->next);
	}
	else {
		pre->next = tmp->next;
	}

	kfree(tmp);
	return ;
}
#if 0
static void
atable_element_reset( )
{
	struct sfd_arper *tmp = NULL ,*pre = NULL,*head = NULL;
	int m, n;

	for(m = 0; m < TABLE_SIZE; ++m)
	{
		for(n = 0; n < TABLE_SIZE; ++n) 
		{
			head = arp_tables[m][n];
			tmp = head;
			pre = head;
			while(tmp)
			{
				if(tmp->isStatic==1)
				{
					pre = tmp;
					tmp = tmp->next;
				}
				else
				{
					if(tmp == head) 
					{
						arp_tables[m][n] = tmp->next;
						kfree(tmp);
						head =arp_tables[m][n];
						tmp = head;
					}
					else
					{
						pre->next = tmp->next;
						kfree(tmp);
						tmp =pre->next;
					}		
				}
			}
			
		}
	}
	return ;
}
#endif
static void
atable_element_reset( )
{
	struct sfd_arper *tmp = NULL;
	
	int m, n;
	for(m = 0; m < TABLE_SIZE; ++m) {
		for(n = 0; n < TABLE_SIZE; ++n) {
			tmp = arp_tables[m][n];
			while(tmp) {
				tmp->packet= 0;
				tmp = tmp->next;
			}
		}
	}
	return ;
}

static void
atable_element_clear()
{
	struct sfd_arper *tmp = NULL, *next = NULL;
	int m, n;

	for(m = 0; m < TABLE_SIZE; ++m) {
		for(n = 0; n < TABLE_SIZE; ++n) {
			tmp = arp_tables[m][n];
			while(tmp) {
				next = tmp->next;
				kfree(tmp);
				tmp = next;
			}
			arp_tables[m][n] = NULL;
		}
	}

	return ;
}

#if 0
static void
atable_element_dump()
{
	struct sfd_arper *tmp = NULL, *next = NULL;
	int m, n;

	for(m = 0; m < TABLE_SIZE; ++m) {
		for(n = 0; n < TABLE_SIZE; ++n) {
			tmp = arp_tables[m][n];
			print_log(SFD_LOG_DEBUG,
						"arp table[0x%02X][0x%02X]:\n",
						m, n);
			while(tmp) {
				next = tmp->next;
				print_log(SFD_LOG_DEBUG,
							"mac = %02X:%02X:%02X:%02X:%02X:%02X "
							"key = 0x%08X, packet = %u\n",
							tmp->mac[0], tmp->mac[1], tmp->mac[2],
							tmp->mac[3], tmp->mac[4], tmp->mac[5],
							tmp->key, tmp->packet);
				tmp = next;
			}
		}
	}

	return ;
}
#endif

static int
print_log(SfdLogSeverity loglevel, char *format, ...)
{
	va_list list;

	if(likely(loglevel == SFD_LOG_DEBUG && debug_mode == 0)) {
		return 0;
	}

	if(unlikely(log_enable == 1)) {
		char pformat[1024] = {};
		va_start(list, format);
		switch(loglevel) {
			case SFD_LOG_INFO:
				strcpy(pformat, KERN_INFO);
				break;
			case SFD_LOG_DEBUG:
				strcpy(pformat, KERN_DEBUG);
				break;
			case SFD_LOG_ERR:
				strcpy(pformat, KERN_ERR);
				break;
			default:
				break;
		}
		sprintf(pformat, "%s [module sfd]%s", pformat, format);
		vprintk(pformat, list);
		va_end(list);
	}

	return 0;
}

static int
parse_skb_packet_ipv4(unsigned char *mac, void *data)
{
	struct iphdr *hdr;
	int ret = 0;

	hdr = (struct iphdr *)data;

	switch(hdr->protocol) {
		case IPPROTO_TCP:
			if(tcp_enable)
				ret = parse_skb_packet_tcp(mac, &(hdr->saddr), IPV4MSG_DATA(data), sfdiptype_ipv4);
			break;
		case IPPROTO_UDP:
			ret = parse_skb_packet_udp(mac, &(hdr->saddr), IPV4MSG_DATA(data), sfdiptype_ipv4);
			break;
		case IPPROTO_ICMP:
			if(icmp_enable)
				ret = parse_skb_packet_icmp(mac, &(hdr->saddr), IPV4MSG_DATA(data), sfdiptype_ipv4);
				break;
		default:
			break;
	}

	return ret;
}

#ifdef SUPPORT_IPV6
static int
parse_skb_packet_ipv6(unsigned char *mac, void *data)
{
	struct ipv6hdr *hdr;
	int ret = 0;

	hdr = (struct ipv6hdr *)data;

	switch(hdr->nexthdr) {
		case IPPROTO_TCP:
			if(tcp_enable)
				ret = parse_skb_packet_tcp(mac, &(hdr->saddr), IPV6MSG_DATA(data), sfdiptype_ipv6);
			break;
		case IPPROTO_UDP:
			ret = parse_skb_packet_udp(mac, &(hdr->saddr), IPV6MSG_DATA(data), sfdiptype_ipv6);
			break;
		case IPPROTO_ICMP:
			if(icmp_enable)
				ret = parse_skb_packet_icmp(mac, &(hdr->saddr), IPV6MSG_DATA(data), sfdiptype_ipv6);
			break;
		default:
			break;
	}

	return ret;
}
#endif

static int
parse_skb_packet_vlan(unsigned char *mac, void *data)
{
	struct vlan_ethhdr *hdr;
	int ret = 0;
	hdr = (struct vlan_ethhdr *)data;

	switch(hdr->h_vlan_encapsulated_proto) {
		case ETH_P_IP:
			ret = parse_skb_packet_ipv4(mac, VLANMSG_DATA(data));
			break;
#ifdef SUPPORT_IPV6
		case ETH_P_IPV6:
			ret = parse_skb_packet_ipv6(mac, VLANMSG_DATA(data));
			break;
#endif
		default:
			break;
	}

	return ret;
}

static int
parse_skb_packet_arp(unsigned char *mac, void *data)
{
	struct sfd_arper *tmp = NULL;
	struct arphdr *hdr;
	int ret = 0;

#if 0
	unsigned char *arp_data = NULL;
	unsigned char *sen_mac = NULL;
	unsigned char *sen_ip = NULL;
	unsigned char *tar_mac = NULL;
	unsigned char *tar_ip = NULL;
	int hardware_len = 0;
	int protocol_len = 0;
#endif

	hdr = (struct arphdr *)data;

#if 0
	print_log(SFD_LOG_INFO,
				"Hardware type: 0x%X, "
				"Protocol type: 0x%X, "
				"Hardware size: 0x%X, "
				"Protocol size: 0x%X, "
				"Opcode: 0x%X\n",
				hdr->ar_hrd, hdr->ar_pro, hdr->ar_hln,
				hdr->ar_pln, hdr->ar_op);
#endif

	if(hdr->ar_op != ARPOP_REQUEST) {
		//print_log(SFD_LOG_DEBUG, "Not a arp request!\n");
		return 0;
	}

#if 0
	if(hdr->ar_hrd != ARPHRD_ETHER ||
		hdr->ar_pro != ETH_P_IP ||
		hdr->ar_hln != ETH_ALEN ||
		hdr->ar_pln != 4) {
		print_log(SFD_LOG_INFO, "Not normal arp format!\n");
		return 0;
	}

	arp_data = (unsigned char *)hdr + sizeof(hdr);

	hardware_len = hdr->ar_hln;
	protocol_len = hdr->ar_pln;

	sen_mac = arp_data;
	sen_ip = (unsigned char *)(arp_data + hardware_len);
	tar_mac = (unsigned char *)(arp_data + hardware_len + protocol_len);
	tar_ip = (unsigned char *)(arp_data + 2 * hardware_len + protocol_len);

	print_log(SFD_LOG_INFO,
				"Sender mac: %02X:%02X:%02X:%02X:%02X:%02X, "
				"Sender ip: 0x%08X, "
				"Target mac: %02X:%02X:%02X:%02X:%02X:%02X, "
				"Target ip: 0x%08X\n",
				sen_mac[0], sen_mac[1], sen_mac[2],
				sen_mac[3], sen_mac[4], sen_mac[5],
				*(unsigned int *)sen_ip,
				tar_mac[0], tar_mac[1], tar_mac[2],
				tar_mac[3], tar_mac[4], tar_mac[5],
				*(unsigned int *)tar_ip);
#endif

	if(!down_trylock(&atable_sema)) {
		tmp = atable_element_dynamic_update(mac);
		if(tmp) 
		{	
			if(tmp->isStatic==1)
			{
				ret = 0;
				print_log(SFD_LOG_INFO,"arp legal user!\n");
			}
			else
			{
				if(tmp->packet == limitpacket_arp) 
				{
				#if 0
					sfdMsg msg;
					msg.cmd = sfdcmd_arpwarning;
					msg.datalen = ETH_ALEN;
					memcpy(msg.data, mac, ETH_ALEN);
					send_netlinkmsg(sfd_pid, &msg);
				#endif
					print_log(SFD_LOG_DEBUG,
								"ARP: %02X:%02X:%02X:%02X:%02X:%02X "
								"attacking!\n",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				}
				tmp->packet++;
				if(tmp->packet > limitpacket_arp) {
					ret = 1;
				}
			}	
		}
		up(&atable_sema);
	}
	return ret;
}

static int
parse_skb_packet_tcp(unsigned char *mac, void *ip, void *data, iptype type)
{
	struct sfd_user *tmp = NULL;
	struct tcphdr *hdr;
	int ret = 0;

	hdr = (struct tcphdr *)data;

	if(hdr->syn && !(hdr->ack)) 
	{
		if(!down_trylock(&utable_sema)) 
		{
			tmp = utable_element_check(mac, ip, type);
			if(tmp)
			{
				if(tmp->isStatic==1)
				{
					ret = 0;
				}
				else  
				{
					if(tmp->syn == limitpacket_tcp)
					{
					print_log(SFD_LOG_INFO,
							"SYN: %02X:%02X:%02X:%02X:%02X:%02X "
							"attacking\n",
							mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					}

					tmp->syn++;
					if(tmp->syn > limitpacket_tcp)
					{
						ret = 1;
					}
				}
			}
			else
			{
				utable_element_dynamic_update(mac,ip,type);
				print_log(SFD_LOG_DEBUG,
							"SYN: %02X:%02X:%02X:%02X:%02X:%02X "
							"not a legal user!\n",
							mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			
				ret = 1;	
			}
			up(&utable_sema);
		}
	}
	return ret;
}

static int
parse_skb_packet_udp(unsigned char *mac, void *ip, void *data, iptype type)
{
	struct udphdr *hdr;
	int ret = 0;

	hdr = (struct udphdr *)data;
	if(hdr->dest == 53) {
		if(dns_enable)
			ret = parse_skb_packet_dnsquery(mac, ip, UDPMSG_DATA(data), type);
	}
	else if(hdr->dest == 161) {
		if(snmp_enable)
			ret = parse_skb_packet_snmp(mac, ip, UDPMSG_DATA(data), type);
	}

	return ret;
}

static int
parse_skb_packet_icmp(unsigned char *mac, void *ip, void *data, iptype type)
{
	struct sfd_user *tmp = NULL;
	int ret = 0;
#if 0
	struct icmphdr *hdr;

	hdr = (struct icmphdr *)data;

	print_log(SFD_LOG_DEBUG,
				"Recv a ICMP packet from "
				"%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

	if(!down_trylock(&utable_sema)) 
	{
		tmp = utable_element_check(mac, ip, type);
		if(tmp) 
		{
			if(tmp->isStatic==1)
			{
					ret = 0;
			}
			else 
			{
				if(tmp->icmp == limitpacket_icmp) 
				{
				print_log(SFD_LOG_DEBUG,
						"ICMP: %02X:%02X:%02X:%02X:%02X:%02X "
						"attacking\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				}

				tmp->icmp++;
				if(tmp->icmp > limitpacket_icmp)
				{
					ret = 1;
				}
			}
		}
		else 
		{
			utable_element_dynamic_update(mac,ip,type);
			print_log(SFD_LOG_DEBUG,
						"ICMP: %02X:%02X:%02X:%02X:%02X:%02X "
						"not a legal user\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			ret = 1;
		}
		up(&utable_sema);
	}

	return ret;
}

static int
parse_skb_packet_dnsquery(unsigned char *mac, void *ip, void *data, iptype type)
{
	struct sfd_user *tmp = NULL;
	void *dnsdata;
	int ret = 0;
	__u16 dnstype;

	dnsdata = DNSMSG_DATA(data);
	dnstype = *(__u16 *)(dnsdata + strlen((char *)(dnsdata + 1)) + 1);

	if(dnstype == 0x1000) {
		if(!down_trylock(&utable_sema)) {
			tmp = utable_element_check(mac, ip, type);
			if(tmp) {
				
				print_log(SFD_LOG_DEBUG,
							"DNS Query: %02X:%02X:%02X:%02X:%02X:%02X "
							"attacking\n",
							mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				ret = 1;
			}
			else {
				utable_element_dynamic_update(mac,ip,type);
				print_log(SFD_LOG_DEBUG,
							"DNS Query: %02X:%02X:%02X:%02X:%02X:%02X "
							"not a legal user\n",
							mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				ret = 1;
			}
			up(&utable_sema);
		}
	}

	return ret;
}

static int parse_skb_packet_snmp(unsigned char *mac, void *ip, void *data, iptype type)
{
	struct sfd_user *tmp = NULL;
	int ret = 0;
	if(!down_trylock(&utable_sema))
	{
		tmp = utable_element_check(mac, ip, type);
		if(tmp) 
		{
			if(tmp->isStatic==1)
			{
					ret = 0;
			}
			else 
			{
				
				if(tmp->snmp == limitpacket_snmp)
				{
					print_log(SFD_LOG_DEBUG,
						"SNMP: %02X:%02X:%02X:%02X:%02X:%02X "
						"attacking\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				}

				tmp->snmp++;
				if(tmp->snmp> limitpacket_snmp)
				{
					ret = 1;
				}
			}
		}
		else {
			utable_element_dynamic_update(mac,ip,type);
			print_log(SFD_LOG_DEBUG,
						"SNMP: %02X:%02X:%02X:%02X:%02X:%02X "
						"not a legal user\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			ret = 1;
		}
		up(&utable_sema);
	}

	return ret;
}


#define ARP_SWITCH_PATH "/var/run/sfd/arp_switch"
#define ARP_LIMITPACKET_PATH "/var/run/sfd/arp_limitpacket"

static int
record_arp_switch_state(void)
{
	mm_segment_t old_fs;
	struct file *fp = NULL;
	char buf = 0;

	fp = filp_open(ARP_SWITCH_PATH, O_CREAT | O_TRUNC | O_RDWR, 0644);
	if(IS_ERR(fp)) {
		print_log(SFD_LOG_ERR,
					"open file %s failed\n", ARP_SWITCH_PATH);
		return -1;
	}

	buf = '0' + arp_enable;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fp->f_op->write(fp, (char *)&buf, sizeof(buf), &fp->f_pos);
	set_fs(old_fs);

	return 0;
}

static int
record_arp_limitpacket_state(void)
{
	mm_segment_t old_fs;
	struct file *fp = NULL;
	char buf[32];

	fp = filp_open(ARP_LIMITPACKET_PATH, O_CREAT | O_TRUNC | O_RDWR, 0644);
	if(IS_ERR(fp)) {
		print_log(SFD_LOG_ERR,
					"open file %s failed\n", ARP_LIMITPACKET_PATH);
		return -1;
	}

	sprintf(buf, "%d", limitpacket_arp);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fp->f_op->write(fp, (char *)&buf, strlen(buf), &fp->f_pos);
	set_fs(old_fs);

	return 0;
}

static void
daemonreg_handler(int pid)
{
	sfd_pid = pid;
	print_log(SFD_LOG_INFO,
				"sfd daemon process pid = %u\n", sfd_pid);
	return ;
}

static void
logswitch_handler(int enable)
{
	if(enable) {
		log_enable = 1;
	}
	else {
		log_enable = 0;
	}

	return ;
}

static void
debugswitch_handler(int enable)
{
	if(enable) {
		debug_mode = 1;
	}
	else {
		debug_mode = 0;
	}

	return ;
}

/* sfd */
static void
sfd_switch_handler(int enable,int enabletype)
{
	if(enable) 
	{
		switch(enabletype)
		{
			case ENABLE_TCP:
					tcp_enable = 1;
					break;
			case ENABLE_ICMP:
					icmp_enable = 1;
					break;
			case ENABLE_SNMP:
					snmp_enable = 1;
					break;
			case ENABLE_DNS:
					dns_enable = 1;
					break;
			case DISABLE_TCP:
					tcp_enable = 0;
					break;
			case DISABLE_ICMP:
					icmp_enable = 0;
					print_log(SFD_LOG_INFO, "ENABLE_ICMP:icmp_enable=%d\n",icmp_enable);
					break;
			case DISABLE_SNMP:
					snmp_enable = 0;
					break;	
			case DISABLE_DNS:
					dns_enable = 0;
					break;	
			case ENABLE_SFD :
					tcp_enable =0;
					icmp_enable = 0;
					snmp_enable = 0;
					dns_enable = 0;
					break;
			default:	
					break;
		}
		if(cvm_sfd_rx_hook == sfd_catch_packet)
		{
			return ;
		}
		
		/* create a schedule thread */
		sfd_task_schedule = kthread_create(sfd_schedule_thread,
										NULL, "%s", "sfd_schedule");

		if(sfd_task_schedule)
		{
			print_log(SFD_LOG_INFO, "create sfd schedule thread succcess\n");
			wake_up_process(sfd_task_schedule);
		}
		else 
		{
			print_log(SFD_LOG_ERR, "create sfd schedule thread failed\n");
			return ;
		}

		cvm_sfd_rx_hook = sfd_catch_packet;
	}
	else 
	{
		if(cvm_sfd_rx_hook == NULL)
		{
			return ;
		}

		/* thread deinit */
		if(sfd_task_schedule)
			kthread_stop(sfd_task_schedule);
		sfd_task_schedule = NULL;
		print_log(SFD_LOG_DEBUG, "sfd schedule thread exited!\n");

		cvm_sfd_rx_hook = NULL;
		sfdMsg msg;
		msg.cmd = sfdcmd_switch;
		msg.datalen = sizeof(int);
		*(int *)msg.data = enable ;
		send_netlinkmsg(sfd_pid, &msg);
		print_log(SFD_LOG_DEBUG, "sfd netlink close sd!\n");

	}

	sfd_enable = enable;
	return ;
}

static void
sfd_timespan_handler(int span)
{
	print_log(SFD_LOG_INFO,
				"sfd timespan %u -> %u\n",
				timespan, span/10);
	timespan = span/10;
	return ;
}

static void
sfd_limitpacket_handler(int protoflag,int packet)
{
	print_log(SFD_LOG_INFO,
				"sfd limit packet %u,protoflag=%u\n",
				packet,protoflag);
	switch(protoflag)
	{
		case FLAG_TCP:
			limitpacket_tcp = packet;
			break;
		case FLAG_SNMP:
			limitpacket_snmp = packet;
			break;
		case FLAG_ICMP:
			limitpacket_icmp = packet;
			break;
		default:
			break;
	}
	
	return ;
}

static void
sfd_useradd_handler(sfdMember *user)
{
	print_log(SFD_LOG_DEBUG,
			"add mac = %02X:%02X:%02X:%02X:%02X:%02X\n",
			user->mac[0], user->mac[1], user->mac[2],
			user->mac[3], user->mac[4], user->mac[5]
	);

	if(user->type == sfdiptype_ipv4) {
		print_log(SFD_LOG_DEBUG, "ipv4 address = %u.%u.%u.%u\n",
					user->ip[3], user->ip[2], user->ip[1], user->ip[0]);
	}
	
	down(&utable_sema);
	if(!utable_element_static_update(user->mac, user->ip, sfdiptype_ipv4)) {
		print_log(SFD_LOG_ERR, "add new user failed\n");
	}
	up(&utable_sema);

	return ;
}

static void
sfd_userdel_handler(sfdMember *user)
{
	print_log(SFD_LOG_DEBUG,
			"delete mac = %02X:%02X:%02X:%02X:%02X:%02X\n",
			user->mac[0], user->mac[1], user->mac[2],
			user->mac[3], user->mac[4], user->mac[5]
	);

	down(&utable_sema);
	utable_element_delete(user->mac);
	up(&utable_sema);

	return ;
}

/* arp */
static void
arp_switch_handler(int enable)
{
	if(enable) 
	{
		if(cvm_arp_rx_hook == arp_catch_packet) {
			goto fin;
		}

		/* create a schedule thread */
		arp_task_schedule = kthread_create(arp_schedule_thread,
										NULL, "%s", "sfd_arp_schedule");

		if(arp_task_schedule) {
			print_log(SFD_LOG_INFO, "create arp schedule thread succcess\n");
			wake_up_process(arp_task_schedule);
		}
		else {
			print_log(SFD_LOG_ERR, "create arp schedule thread failed\n");
			return ;
		}

		cvm_arp_rx_hook = arp_catch_packet;
	}
	else 
	{
		if(cvm_arp_rx_hook == NULL) {
			goto fin;
		}

		/* thread deinit */
		if(arp_task_schedule)
			kthread_stop(arp_task_schedule);
		arp_task_schedule=NULL;
		print_log(SFD_LOG_DEBUG, "arp schedule thread exited!\n");

		cvm_arp_rx_hook = NULL;
	}

fin:
	arp_enable = enable;
	record_arp_switch_state();
	return ;
}

static void
arp_limitpacket_handler(int packet)
{
	print_log(SFD_LOG_INFO,
				"arp limit packet %u -> %u\n",
				limitpacket_arp, packet);
	limitpacket_arp = packet;
	record_arp_limitpacket_state();
	return ;
}
static void
sfd_arpadd_handler(sfdMember *user)
{
	print_log(SFD_LOG_INFO,
			"add arp mac = %02X:%02X:%02X:%02X:%02X:%02X\n",
			user->mac[0], user->mac[1], user->mac[2],
			user->mac[3], user->mac[4], user->mac[5]
	);
	
	down(&atable_sema);
	if(!atable_element_static_update(user->mac)) {
		print_log(SFD_LOG_ERR, "add new arp user failed\n");
	}
	up(&atable_sema);

	return ;
}
static void
sfd_arpdel_handler(sfdMember *user)
{
	print_log(SFD_LOG_INFO,
			"delete arp mac = %02X:%02X:%02X:%02X:%02X:%02X\n",
			user->mac[0], user->mac[1], user->mac[2],
			user->mac[3], user->mac[4], user->mac[5]
	);

	down(&utable_sema);
	atable_element_delete(user->mac);
	up(&utable_sema);

	return ;
}

static void
sfd_variables_handler(int packet)
{
	sfdMsg msg;
	msg.cmd = sfdcmd_variables;
	if(packet)
	{
		msg.datalen = sizeof(int)*13;
		*(int *)msg.data = sfd_enable ;
		*((int *)msg.data+1) = log_enable ;
		*((int *)msg.data+2) = debug_mode;
		*((int *)msg.data+3) = tcp_enable ;
		*((int *)msg.data+4) = icmp_enable ;
		*((int *)msg.data+5) = snmp_enable ;
		*((int *)msg.data+6) = dns_enable ;
		*((int *)msg.data+7) = arp_enable ;
		*((int *)msg.data+8) = timespan *10;
		*((int *)msg.data+9) = limitpacket_snmp ;
		*((int *)msg.data+10) = limitpacket_tcp;
		*((int *)msg.data+11) = limitpacket_icmp;
		*((int *)msg.data+12) = limitpacket_arp;
		
		send_netlinkmsg(sfd_pid, &msg);
	}
return;
}

#if 0
static int
show_netlink_msg_info(void *data)
{
	print_log(SFD_LOG_DEBUG,
				"SFD message information: cmd = %d, data = %d\n",
				((sfdMsg *)data)->cmd, ((sfdMsg *)data)->data);
	return 0;
}
#endif

static int
parse_netlink_msg_info(sfdMsg *msg)
{
	int ret = 0;

	if(!msg)
		return -1;

	switch(msg->cmd) {
		case sfdcmd_daemon:
			daemonreg_handler(*(int *)(msg->data));
			break;
		case sfdcmd_logswitch:
			logswitch_handler(*(int *)(msg->data));
			break;
		case sfdcmd_debugswitch:
			debugswitch_handler(*(int *)msg->data);
			break;
		case sfdcmd_switch:
			sfd_switch_handler(*(int *)(msg->data),*((int *)msg->data+1));
			break;
		case sfdcmd_timespan:
			sfd_timespan_handler(*(int *)(msg->data));
			break;
		case sfdcmd_limitpacket:
			sfd_limitpacket_handler(*(int *)(msg->data),*((int *)msg->data+1));
			break;
		case sfdcmd_newmember:
			sfd_useradd_handler((sfdMember *)(msg->data));
			break;
		case sfdcmd_delmember:
			sfd_userdel_handler((sfdMember *)(msg->data));
			break;
		case sfdcmd_arpswitch:
			arp_switch_handler(*(int *)(msg->data));
			break;
		case sfdcmd_arplimitpacket:
			arp_limitpacket_handler(*(int *)(msg->data));
			break;
		case sfdcmd_arpnewmember:
			sfd_arpadd_handler((sfdMember *)(msg->data));
			break;
		case sfdcmd_arpdelmember:
			sfd_arpdel_handler((sfdMember *)(msg->data));
			break;
		case sfdcmd_variables:
			sfd_variables_handler(*(int *)(msg->data));
			break;
		default:
			break;
	}

	return ret;
}

static int
sfdnetlink_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	print_log(SFD_LOG_DEBUG,
				"Recv a netlink message from pid %u\n",
				nlh->nlmsg_pid);

	if (nlh->nlmsg_len < sizeof(struct nlmsghdr)) {
		print_log(SFD_LOG_ERR, "corrupt netlink message\n");
		return 0;
	}

	parse_netlink_msg_info((sfdMsg *)NLMSG_DATA(nlh));
	return 0;
}

static void
sfdnetlink_recv(struct sk_buff *skb)
{
#if 0
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlhdr = NULL;

	while((skb = skb_dequeue(&netlink_sock->sk_receive_queue)) != NULL) {
		nlhdr = (struct nlmsghdr *)skb->data;

		print_log(SFD_LOG_DEBUG,
					"Recv a netlink message from pid %u\n",
					nlhdr->nlmsg_pid);

		if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
			print_log(SFD_LOG_ERR, "corrupt netlink message\n");
			continue;
		}

		//show_netlink_msg_info((void *)NLMSG_DATA(nlhdr));
		parse_netlink_msg_info((sfdMsg *)NLMSG_DATA(nlhdr));
	}
#else
	//netlink_rcv_skb(skb, &sfdnetlink_rcv_msg);
	struct nlmsghdr *nlhdr = NULL;

	print_log(SFD_LOG_DEBUG, "sfd netlink recv executed\n");

	if (skb) {
		nlhdr = (struct nlmsghdr *)skb->data;

		print_log(SFD_LOG_DEBUG,
					"Recv a netlink message from pid %u\n",
					nlhdr->nlmsg_pid);

		if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
			print_log(SFD_LOG_ERR, "corrupt netlink message\n");
			return ;
		}
		parse_netlink_msg_info((sfdMsg *)NLMSG_DATA(nlhdr));
	}
#endif
	return ;
}

static int
send_netlinkmsg(int pid, sfdMsg *msg)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlhdr = NULL;
	sk_buff_data_t old_tail;
	int ret = 0, size = 0;

	size = NLMSG_SPACE(sizeof(*msg));
	skb = alloc_skb(size, GFP_ATOMIC);
	if(!skb) {
		print_log(SFD_LOG_ERR,
					"alloc skb failed!\n");
		return -1;
	}
	old_tail = skb->tail;

	nlhdr = NLMSG_PUT(skb, 0, 0, 1, size - sizeof(*nlhdr));
	memcpy(NLMSG_DATA(nlhdr), msg, sizeof(*msg));
	nlhdr->nlmsg_len = skb->tail - old_tail;

	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).creds.pid = pid;
	NETLINK_CB(skb).dst_group = 0;

	ret = netlink_unicast(netlink_sock, skb, pid, MSG_DONTWAIT);
	return ret;

nlmsg_failure:
	if(skb)
		kfree_skb(skb);
	return -1;
}

static int
sfd_schedule_thread(void *dummy)
{
	/* wait timespan to check list */
	while(!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(timespan);
		print_log(SFD_LOG_DEBUG, "start to reset sfd table!\n");
		down(&utable_sema);
		utable_element_reset();
		up(&utable_sema);
	}

	set_current_state(TASK_RUNNING);
	print_log(SFD_LOG_DEBUG, "sfd schedule thread exiting!\n");
	return 0;
}

static int
arp_schedule_thread(void *dummy)
{
	/* wait timespan to check list */
	while(!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(timespan);
		print_log(SFD_LOG_DEBUG, "start to clear arp table!\n");
		down(&atable_sema);
		/*
		atable_element_dump();
		*/
		
		/*
		atable_element_clear();
		*/
		atable_element_reset();	
		up(&atable_sema);
	}

	set_current_state(TASK_RUNNING);
	print_log(SFD_LOG_DEBUG, "arp schedule thread exiting!\n");
	return 0;
}

#if 0
static struct file_operations sfd_fops = {
	.owner		= THIS_MODULE,
	.read		= NULL,
	.write		= NULL,
	.ioctl		= sfd_ioctl,
	.open		= NULL,
	.release	= NULL,
};
#endif

static int __init sfd_init(void)
{
	/* netlink init */
	netlink_sock = netlink_kernel_create(&init_net, SFD_NETLINK_ID, 0, sfdnetlink_recv, NULL, THIS_MODULE);
	if(!netlink_sock) {
		print_log(SFD_LOG_ERR, "create a netlink socket failed\n");
		return -ENOMEM;
	}
	print_log(SFD_LOG_DEBUG, "creat netlink socket success\n");

	/* semaphore init */
	sema_init(&utable_sema, 1);
	sema_init(&atable_sema, 1);

	cvm_sfd_rx_hook = NULL;
	//cvm_sfd_rx_hook = sfd_catch_packet;

	cvm_arp_rx_hook = NULL;
	//cvm_arp_rx_hook = arp_catch_packet;

	print_log(SFD_LOG_INFO, "initial done!\n");
	return 0;
}

static void __exit sfd_exit(void)
{
	/*
	 * if netlink has any process connect
	 * module can't unload
	 */
#if 1
	/* sfd thread deinit */
	if(sfd_task_schedule)
		kthread_stop(sfd_task_schedule);
	print_log(SFD_LOG_DEBUG, "sfd Schedule Thread exited!\n");

	/* arp thread deinit */
	if(arp_task_schedule)
		kthread_stop(arp_task_schedule);
	print_log(SFD_LOG_DEBUG, "arp Schedule Thread exited!\n");
#endif

	/* hook deinit */
	cvm_sfd_rx_hook = NULL;
	cvm_arp_rx_hook = NULL;

	/* netlink deinit */
	if(netlink_sock) {
		sock_release(netlink_sock->sk_socket);
		print_log(SFD_LOG_DEBUG, "netlink socket released!\n");
	}

	/* clear user table */
	utable_element_clear();
	atable_element_clear();
	print_log(SFD_LOG_INFO, "deinitial done!\n");
	return ;
}

module_init(sfd_init);
module_exit(sfd_exit);
MODULE_LICENSE("GPL");
