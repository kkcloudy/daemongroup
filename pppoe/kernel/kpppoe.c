
#define pr_fmt(fmt) KBUILD_MODNAME ": %s: " fmt
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/rculist.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/ppp_defs.h>

#include <net/sock.h>

#include "if_pppoe.h"


#define CHANNEL_HASHBITS	16
#define CHANNEL_HASHSIZE	(1 << CHANNEL_HASHBITS)


#ifndef LOG_ERR
#define LOG_EMERG		0       /* system is unusable */
#define LOG_ALERT		1       /* action must be taken immediately */
#define LOG_CRIT		2       /* critical conditions */
#define LOG_ERR			3       /* error conditions */
#define LOG_WARNING		4       /* warning conditions */
#define LOG_NOTICE		5       /* normal but significant condition */
#define LOG_INFO		6       /* informational */
#define LOG_DEBUG		7       /* debug-level messages */
#endif

/*
 * Network protocols we support.
 */
#define NP_IP		0		/* Internet Protocol V4 */
#define NP_IPV6		1		/* Internet Protocol V6 */
#define NP_IPX		2		/* IPX protocol */
#define NP_AT		3		/* Appletalk protocol */
#define NP_MPLS_UC 	4		/* MPLS unicast */
#define NP_MPLS_MC 	5		/* MPLS multicast */
#define NUM_NP		6		/* Number of NPs. */


/* PPPoe socket states */
enum {
    PPPOE_NONE			= 0,  /* initial state */
    PPPOE_BIND			= 1,  /* bind to ppp device */
    PPPOE_CONNECTE		= 2,	/* connect state */
    PPPOE_ZOMBIE		= 4,  /* dead, but still bound to ppp device */
    PPPOE_DEAD			= 8  /* dead, useless, please clean me up!*/
};


/* Get the PPP protocol number from a skb */
#define PPP_PROTO(skb)	(((skb)->data[0] << 8) + (skb)->data[1])


struct pppoe_channel_stats {
	unsigned long long	rx_packets;		/* total packets received	*/
	unsigned long long	tx_packets;		/* total packets transmitted	*/
	
	unsigned long long	rx_bytes;		/* total bytes received 		*/
	unsigned long long	tx_bytes;		/* total bytes transmitted	*/
	
	unsigned long long	rx_errors;		/* bad packets received	*/
	unsigned long long	tx_errors;		/* packet transmit problems	*/
	
	unsigned long long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long long	tx_dropped;		/* no space available in linux*/
};

struct pppoe_channel {
	unsigned short sid;
	unsigned int ip;
	unsigned int state;
	
	struct hlist_node sid_hlist;
	struct hlist_node ip_hlist;
	struct rcu_head rcu;
		
	unsigned char magic[MAGIC_LEN];
	unsigned char mac[ETH_ALEN];
	unsigned char serverMac[ETH_ALEN];
	enum NPmode	npmode[NUM_NP]; 	/* what to do with each net proto 78 */
	struct pppoe_channel_stats stats;	/* statistics */
};


struct pppoe_struct {
	int	mru;	/* max receive unit 60 */

	unsigned long channels;

	unsigned long last_xmit;	/* jiffies when last pkt sent 9c */
	unsigned long last_recv;	/* jiffies when last pkt rcvd a0 */
	
	struct sock *sk;
	spinlock_t sock_lock;

	struct net_device *dev;
	struct net_device *base_dev;
	struct list_head next;
	
	struct net_device_stats stats;	/* statistics */

	struct hlist_head sid_head[CHANNEL_HASHSIZE];
	struct hlist_head ip_head[CHANNEL_HASHSIZE];
};

struct pppoe_sock {
	/* struct sock must be the first member of pppoe_sock */
	struct sock	sk;
	struct pppoe_struct	*pppoe;		/* pppoe this socket bind to*/
};

struct pppoe_proto {
	int	(*create)(struct net *net, struct socket *sock);
	struct module	*owner;
};

struct pppoe_deamon {
	u32 pid;
//	u32 state; /* may need check hansi state */
	struct list_head list;
};

static inline struct pppoe_hdr *
pppoe_hdr(const struct sk_buff *skb) {
	return (struct pppoe_hdr *)skb_network_header(skb);
}

static inline struct pppoe_sock *
pppoe_sk(struct sock *sk) {
	return (struct pppoe_sock *)sk;
}

static inline struct sock *
sk_pppoe(struct pppoe_sock *po) {
	return (struct sock *)po;
}

static int kpppoe_rx_debug __read_mostly = 0;
module_param(kpppoe_rx_debug, uint, 0644);
MODULE_PARM_DESC(kpppoe_rx_debug, "kpppoe receive debug: "
							"debug level 0-7, default debug level 0.");

static int kpppoe_tx_debug __read_mostly = 0;
module_param(kpppoe_tx_debug, uint, 0644);
MODULE_PARM_DESC(kpppoe_tx_debug, "kpppoe transmit debug: "
							"debug level 0-7, default debug level 0.");

static int kpppoe_ctl_debug __read_mostly = 0;
module_param(kpppoe_ctl_debug, uint, 0644);
MODULE_PARM_DESC(kpppoe_ctl_debug, "kpppoe control debug: "
							"debug level 0-7, default debug level 0.");

static unsigned int exit_flag = 0;
static struct pppoe_deamon instance_pppoed[2][17];
static spinlock_t pppoe_lock;	// pppoe netlink thread and event spin lock

#ifdef CONFIG_PROC_FS
struct proc_dir_entry *proc_net_pppoe = NULL;
#endif

static DECLARE_COMPLETION(exit_completion);

static struct kmem_cache *pppoe_channel_kmem __read_mostly;

static struct sock *pppoe_netlink = NULL;

extern int (*pppoe_rx_hook)(struct sk_buff *);

#define kpppoe_log(lvl, fmt, ...)	\
	do {	\
		switch (lvl) {	\
		case LOG_EMERG:	\
			printk(KERN_EMERG fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_ALERT:	\
			printk(KERN_ALERT fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_CRIT:	\
			printk(KERN_CRIT fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_ERR:	\
			printk(KERN_ERR fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_WARNING:	\
			printk(KERN_WARNING fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_NOTICE:	\
			printk(KERN_NOTICE fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_INFO:	\
			printk(KERN_INFO fmt, ##__VA_ARGS__);	\
			break;		\
		case LOG_DEBUG:	\
			printk(KERN_DEBUG fmt, ##__VA_ARGS__);	\
			break;		\
		default:		\
			break;		\
		}	\
	} while (0);	

#define kpppoe_rx_log(lvl, fmt, ...) 	\
	if (unlikely(kpppoe_rx_debug >= lvl))	\
		kpppoe_log(lvl, pr_fmt(fmt), __func__, ##__VA_ARGS__)

#define kpppoe_tx_log(lvl, fmt, ...) 	\
	if (unlikely(kpppoe_tx_debug >= lvl))	\
		kpppoe_log(lvl, pr_fmt(fmt), __func__, ##__VA_ARGS__)

#define kpppoe_ctl_log(lvl, fmt, ...) 	\
	if (unlikely(kpppoe_ctl_debug >= lvl))	\
		kpppoe_log(lvl, pr_fmt(fmt), __func__, ##__VA_ARGS__)

#define channel_dump(log, channel)	\
	log(LOG_DEBUG, "channel sid %u mac %02X:%02X:%02X:%02X:%02X:%02X ip %u.%u.%u.%u\n", \
		ntohs(channel->sid), channel->mac[0], channel->mac[1], channel->mac[2], \
		channel->mac[3], channel->mac[4], channel->mac[5], NIPQUAD(channel->ip))

/* Translates a PPP protocol number to a NP index (NP == network protocol) */
static inline int 
proto_to_npindex(int proto) {
	switch (proto) {
	case PPP_IP:
		return NP_IP;
	case PPP_IPV6:
		return NP_IPV6;
	case PPP_IPX:
		return NP_IPX;
	case PPP_AT:
		return NP_AT;
	case PPP_MPLS_UC:
		return NP_MPLS_UC;
	case PPP_MPLS_MC:
		return NP_MPLS_MC;
	}
	
	return -EINVAL;
}

/* Translates an NP index into a PPP protocol number */
static const int 
npindex_to_proto[NUM_NP] = {
	PPP_IP,
	PPP_IPV6,
	PPP_IPX,
	PPP_AT,
	PPP_MPLS_UC,
	PPP_MPLS_MC,
};
	
/* Translates an ethertype into an NP index */
static inline int 
ethertype_to_npindex(int ethertype) {
	switch (ethertype) {
	case ETH_P_IP:
		return NP_IP;
	case ETH_P_IPV6:
		return NP_IPV6;
	case ETH_P_IPX:
		return NP_IPX;
	case ETH_P_PPPTALK:
	case ETH_P_ATALK:
		return NP_AT;
	case ETH_P_MPLS_UC:
		return NP_MPLS_UC;
	case ETH_P_MPLS_MC:
		return NP_MPLS_MC;
	}
	
	return -EINVAL;
}

/* Translates an NP index into an ethertype */
static const int 
npindex_to_ethertype[NUM_NP] = {
	ETH_P_IP,
	ETH_P_IPV6,
	ETH_P_IPX,
	ETH_P_PPPTALK,
	ETH_P_MPLS_UC,
	ETH_P_MPLS_MC,
};

static inline int
ifname_get_instance(char *ifname, unsigned int *local_id, unsigned int *instance_id) {
	unsigned int i, length, flag = 0;
	char *pstr;

	*local_id = 0;
	*instance_id = 0;

	length = strlen(ifname);
	if (length > (IFNAMSIZ - 1))
		goto error;

	if (memcmp(ifname, "pppoe", 5))
		goto error;

	pstr = ifname + 5;
	if (*pstr == 'l') {
		*local_id = 1;
		pstr++;
	} else if (*pstr <= '9' && *pstr >= '0') {
		*local_id = 0;
	} else {
		goto error;
	}

	length -= pstr - ifname;
	for (i = 0; i < length; i++, pstr++) {
		if (*pstr <= '9' && *pstr >= '0') {
			if (1 == flag) {
				*instance_id = (*instance_id) * 10 + (*pstr - '0');
			}	
		} else if ('-' == *pstr) {
			flag++;
		} else {
			goto error;
		}

		if (flag > 2) 
			goto error;
	}	
	
	return 0;

error:
	return -EINVAL;
}

static inline struct hlist_head *
channel_sid_hash(struct pppoe_struct *pppoe, const unsigned short sid) {
	return &(pppoe->sid_head[sid & (CHANNEL_HASHSIZE - 1)]);
}

static inline struct hlist_head *
channel_ip_hash(struct pppoe_struct *pppoe, const unsigned int ip) {
	return &(pppoe->ip_head[ip & (CHANNEL_HASHSIZE - 1)]);
}

static inline struct pppoe_channel *
channel_sid_find(struct hlist_head *head, const unsigned short sid) {
	struct pppoe_channel *channel;
	struct hlist_node *node;

	hlist_for_each_entry_rcu(channel, node, head, sid_hlist) {
		if (sid == channel->sid)
			return channel;
	}

	return NULL;
}

static inline struct pppoe_channel *
channel_ip_find(struct hlist_head *head, const unsigned int ip) {
	struct pppoe_channel *channel;
	struct hlist_node *node;

	hlist_for_each_entry_rcu(channel, node, head, ip_hlist) {
		if (ip == channel->ip)
			return channel;
	}

	return NULL;
}


static inline struct pppoe_channel *
channel_get_by_sid(struct pppoe_struct *pppoe, const unsigned short sid) {
	return channel_sid_find(channel_sid_hash(pppoe, sid), sid);;
}

static struct pppoe_channel *
channel_get_by_ip(struct pppoe_struct *pppoe, const unsigned int ip) {
	return channel_ip_find(channel_ip_hash(pppoe, ip), ip);;
}

static inline void
channel_setup(struct pppoe_channel *channel) {
	int i;
	for (i = 0; i < sizeof(channel->npmode) / sizeof(channel->npmode[0]); i++) {
		channel->npmode[i] = NPMODE_DROP;
	}	
}

static inline struct pppoe_channel *
channel_alloc(const unsigned int sid, unsigned char *mac) {
	struct pppoe_channel *channel
		= kmem_cache_alloc(pppoe_channel_kmem, GFP_KERNEL);
	
	if (likely(channel)) {
		memset(channel, 0, sizeof(struct pppoe_channel));
		channel->sid = sid;
		memcpy(channel->mac, mac, ETH_ALEN);
		channel_setup(channel);
	}
	return channel;
}

static void
channel_rcu_free(struct rcu_head *head) {
	struct pppoe_channel *channel
		= container_of(head, struct pppoe_channel, rcu);
	kmem_cache_free(pppoe_channel_kmem, channel);
}

static inline void
__channel_free(struct pppoe_channel *channel) {
	kmem_cache_free(pppoe_channel_kmem, channel);
}

static inline void
channel_free(struct pppoe_channel *channel) {
	call_rcu(&channel->rcu, channel_rcu_free);
}

static inline int
channel_sid_insert(struct pppoe_struct *pppoe, struct pppoe_channel *channel) {
	struct hlist_head *head = channel_sid_hash(pppoe, channel->sid);

	if (channel_sid_find(head, channel->sid)) {
		kpppoe_ctl_log(LOG_WARNING, "sid %u channel is exist", ntohs(channel->sid));
		return -EEXIST;
	}

	hlist_add_head_rcu(&channel->sid_hlist, head);
	return 0;
}

static inline struct pppoe_channel *
channel_sid_remove(struct pppoe_struct *pppoe, const unsigned short sid) {
	struct hlist_head *head = channel_sid_hash(pppoe, sid);
	struct pppoe_channel *channel
		= channel_sid_find(head, sid);

	if (!channel) {
		kpppoe_ctl_log(LOG_WARNING, "sid %u channel is not exist", 
									ntohs(sid));		
		return NULL;
	}

	hlist_del_rcu(&channel->sid_hlist);
	return channel;
}

static inline int
channel_ip_insert(struct pppoe_struct *pppoe, struct pppoe_channel *channel) {
	struct hlist_head *head = channel_ip_hash(pppoe, channel->ip);
	
	if (channel_ip_find(head, channel->ip)) {
		kpppoe_ctl_log(LOG_WARNING, NIPQUAD_FMT " channel is exist", 
									NIPQUAD(channel->ip));		
		return -EEXIST;
	}

	hlist_add_head_rcu(&channel->ip_hlist, head);
	return 0;
}

static inline struct pppoe_channel *
channel_ip_remove(struct pppoe_struct *pppoe, const unsigned int ip) {
	struct hlist_head *head = channel_ip_hash(pppoe, ip);
	struct pppoe_channel *channel
		= channel_ip_find(head, ip);

	if (!channel) {
		kpppoe_ctl_log(LOG_WARNING, NIPQUAD_FMT " channel is not exist", 
									NIPQUAD(ip));		
		return NULL;
	}

	hlist_del_rcu(&channel->ip_hlist);
	return channel;
}

static inline void
channel_hash_init(struct pppoe_struct *pppoe) {
	int i;
	
	for(i = 0; i < ARRAY_SIZE(pppoe->sid_head); i++)
		INIT_HLIST_HEAD(&pppoe->sid_head[i]);

	for(i = 0; i < ARRAY_SIZE(pppoe->ip_head); i++)
		INIT_HLIST_HEAD(&pppoe->ip_head[i]);	
}

static inline void
channel_hash_exit(struct pppoe_struct *pppoe) {
	struct hlist_node *pos, *n;
	int i;

	for (i = 0; i < ARRAY_SIZE(pppoe->ip_head); i++) {
		hlist_for_each_safe(pos, n, &pppoe->ip_head[i]) {
			hlist_del_rcu(pos);
		}
	}	

	for (i = 0; i < ARRAY_SIZE(pppoe->sid_head); i++) {
		hlist_for_each_safe(pos, n, &pppoe->sid_head[i]) {
			hlist_del_rcu(pos);
			channel_free(hlist_entry(pos, struct pppoe_channel, sid_hlist));
		}		
	}	
}

static int
pppoe_cache_init(void) {
	pppoe_channel_kmem = kmem_cache_create("pppoe_channel_cache",
						sizeof(struct pppoe_channel),
						0, SLAB_PANIC, NULL);

	if (!pppoe_channel_kmem) {
		printk(KERN_ERR "pppoe create channel failed\n");
		return -ENOMEM;
	}

	return 0;
}

static void
pppoe_cache_destroy(void) {
	kmem_cache_destroy(pppoe_channel_kmem);
}

static inline void
pppoe_send_frame(struct sk_buff *skb, 
					struct pppoe_struct *pppoe, 
					struct pppoe_channel *channel) {
	struct net_device *base_dev;					
	struct pppoe_hdr *ph;
	unsigned int data_len = skb->len;	
	unsigned short proto;
	unsigned char *pp;	
	int npi;

	base_dev = rcu_dereference(pppoe->base_dev);
	if (!base_dev) {
		kpppoe_tx_log(LOG_WARNING, "pppoe dev %s base_dev is NULL\n", pppoe->dev->name);
		goto error;
	}

	npi = ethertype_to_npindex(ntohs(skb->protocol));
	if (npi < 0) {
		kpppoe_tx_log(LOG_WARNING, "pppoe do not support protocol %u\n", ntohs(skb->protocol));
		goto error;
	}

	/* Drop, accept or reject the packet */
	switch (channel->npmode[npi]) {
	case NPMODE_PASS:
		break;
		
	case NPMODE_DROP:
	case NPMODE_ERROR:
	case NPMODE_QUEUE:
		kpppoe_tx_log(LOG_NOTICE, "protocol %u skb will drop\n", ntohs(skb->protocol));	
		goto error;
	}

	/* Put the 2-byte PPP protocol number on the front,
	   making sure there is room for the address and control fields. */
	if (skb_cow_head(skb, base_dev->hard_header_len + 
					sizeof(*ph) + PPP_HEADER_LEN)) {
		kpppoe_tx_log(LOG_WARNING, "skb_cow_head failed\n");	
		goto drop;
	}

	/*add ppp head*/
	pp = skb_push(skb, PPP_HEADER_LEN);
	proto = npindex_to_proto[npi];
	pp[0] = proto >> 8;
	pp[1] = proto;	

	/*add pppoes head*/
	__skb_push(skb, sizeof(*ph));
	skb_reset_network_header(skb);

	ph = pppoe_hdr(skb);
	ph->ver		= 1;
	ph->type	= 1;
	ph->code 	= 0;
	ph->sid		= channel->sid;
	ph->length	= __constant_htons(skb->len - sizeof(struct pppoe_hdr));

	skb->dev = base_dev;
	skb->protocol = __constant_htons(ETH_P_PPP_SES);

	/*add hard head*/
	dev_hard_header(skb, base_dev, ETH_P_PPP_SES, 
					channel->mac, channel->serverMac, skb->len);
	dev_queue_xmit(skb);

	pppoe->last_xmit = jiffies;
	pppoe->stats.tx_packets++;
	pppoe->stats.tx_bytes += data_len;
	
	channel->stats.tx_packets++;
	channel->stats.tx_bytes += data_len;
	channel_dump(kpppoe_tx_log, channel);	
	return;
	
drop:
	channel->stats.tx_dropped++;
	pppoe->stats.tx_dropped++;
	kfree_skb(skb);
	return;

error:
	channel->stats.tx_errors++;
	pppoe->stats.tx_errors++;
	kfree_skb(skb);
	return;	
}

static netdev_tx_t
pppoe_start_xmit(struct sk_buff *skb, struct net_device *dev) {
	struct pppoe_struct *pppoe = netdev_priv(dev);
	struct pppoe_channel *channel;

	rcu_read_lock();
	channel	= channel_get_by_ip(pppoe, ip_hdr(skb)->daddr);
	if (!channel) {
		rcu_read_unlock();
		kpppoe_tx_log(LOG_NOTICE, "skb ip %u.%u.%u.%u is not exist in dev %s hash\n", 
					NIPQUAD(ip_hdr(skb)->daddr), pppoe->dev->name);
		goto error;
	}
	
	pppoe_send_frame(skb, pppoe, channel);
	rcu_read_unlock();	
	return NETDEV_TX_OK;

error:
	pppoe->stats.tx_errors++;
	kfree_skb(skb);	
	return NETDEV_TX_OK;
}

static inline int
pppoe_receive_control_frame(struct sock *sk, struct sk_buff *skb) {
	int err;
	
	if (!sk) 
		goto drop;

	if (sock_flag(sk, SOCK_DEAD)) 
		goto drop;
	
	err = sock_queue_rcv_skb(sk, skb);
	if (!err)
		return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int
pppoe_echo_reply(struct sk_buff *skb, 
				struct pppoe_struct *pppoe, 
				struct pppoe_channel *channel) {
	struct pppoe_ctl *pctl;
	struct pppoe_hdr *phdr;
	unsigned int length;

	skb_pull(skb, PPP_HEADER_LEN);
	
	pctl = (struct pppoe_ctl *)skb->data;
	length =  __constant_ntohs(pctl->length);

	if (length < (sizeof(struct pppoe_ctl) + MAGIC_LEN) || skb->len < length) 
		goto drop;
	
	pctl->code = 0xa;	/* echo reply */
	pctl->ident = 0x0;
	pctl->length = __constant_htons(length);
	memcpy(pctl->data, channel->magic, MAGIC_LEN);
	
	skb_push(skb, sizeof(struct pppoe_hdr) + PPP_HEADER_LEN);
	skb_reset_network_header(skb);
	
	length += PPP_HEADER_LEN;
	phdr = (struct pppoe_hdr *)skb->data;
	phdr->length = __constant_htons(length);

	skb->protocol = __constant_htons(ETH_P_PPP_SES);

	/*add hard head*/
	dev_hard_header(skb, skb->dev, ETH_P_PPP_SES, 
					channel->mac, channel->serverMac, skb->len);
	dev_queue_xmit(skb);

	/* for control packets, record the time */
	pppoe->last_xmit = jiffies;
	pppoe->stats.tx_packets++;
	pppoe->stats.tx_bytes += (sizeof(struct pppoe_ctl) + MAGIC_LEN);	
	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int
pppoe_receive_frame(struct sk_buff *skb, 
					struct pppoe_struct *pppoe, 
					struct pppoe_channel *channel) {
	struct sock *sk = pppoe->sk;
	int proto, npi;

	if (!pskb_may_pull(skb, PPP_HEADER_LEN)) {
		kpppoe_rx_log(LOG_WARNING, "PPP frame may pull PPP_HEADER_LEN failed\n");
		goto error;
	}

	proto = PPP_PROTO(skb);
	if (proto >= 0x8000) {
		if (!pskb_may_pull(skb, sizeof(struct pppoe_ctl) + PPP_HEADER_LEN)) {
			kpppoe_rx_log(LOG_WARNING, "PPP Control frame %u may pull failed\n", proto);		
			goto error;
		}

		if (PPP_LCP == proto && PPP_ECHO_REQUEST == skb->data[2]) {	/* ppp echo request frame*/
			return pppoe_echo_reply(skb, pppoe, channel);
		}
		
		return pppoe_receive_control_frame(sk, skb);
	} else if (PPP_IP == proto || PPP_IPV6 == proto || PPP_IPX == proto) {
		goto data;
	}

	kpppoe_rx_log(LOG_WARNING, "PPP frame proto %u unknown\n", proto);		
	goto error;


data:
	npi = proto_to_npindex(proto);
	if (unlikely(npi < 0)) {
		kpppoe_rx_log(LOG_WARNING, "PPP Control frame %u to npinex failed\n", proto);	
		pppoe->stats.rx_frame_errors++; 
		goto error1;
	}

	if (NPMODE_PASS != channel->npmode[npi]) {
		kpppoe_rx_log(LOG_NOTICE, "PPP Control frame %u not allow pass\n", proto);	
		pppoe->stats.rx_frame_errors++; 		
		goto error1;
	} 

	skb_pull_rcsum(skb, PPP_HEADER_LEN);		/* chop off ppp protocol */
	skb->dev = pppoe->dev;
	skb->pkt_type = PACKET_HOST;
	skb->protocol = __constant_htons(npindex_to_ethertype[npi]);
	skb_reset_mac_header(skb);

	/* for data packets, record the time */
	pppoe->last_recv = jiffies;
	pppoe->dev->last_rx = jiffies;
	
	pppoe->stats.rx_packets++;
	pppoe->stats.rx_bytes += skb->len;
	
	channel->stats.rx_packets++;
	channel->stats.rx_bytes += skb->len;
	channel_dump(kpppoe_rx_log, channel);
	
	return netif_receive_skb(skb);


error1:
	channel->stats.rx_errors++;
	pppoe->stats.rx_errors++;

error:
	kfree_skb(skb);
	return NET_RX_DROP;
}


static inline int
pppoe_do_rcv(struct sk_buff *skb, struct pppoe_struct *pppoe) {
	struct pppoe_hdr *ph;
	struct pppoe_channel *channel;
	int len;

	if (!(skb = skb_share_check(skb, GFP_ATOMIC))) 
		goto out;

	if (!pskb_may_pull(skb, sizeof(struct pppoe_hdr))) {
		kpppoe_rx_log(LOG_WARNING, "skb may pull struct pppoe_hdr failed\n");
		goto drop;
	}

	skb_reset_network_header(skb);	/* reset network header */
	
	ph = pppoe_hdr(skb);
	len = ntohs(ph->length);
	
	skb_pull_rcsum(skb, sizeof(struct pppoe_hdr));
	if (skb->len < len) {
		kpppoe_rx_log(LOG_WARNING, "skb length %u error, pppoe length %u\n",
									skb->len, len);
		goto drop;
	}
	
	if (pskb_trim_rcsum(skb, len))
		goto drop;

	channel = channel_get_by_sid(pppoe, ph->sid);
	if (!channel || memcmp(channel->mac, eth_hdr(skb)->h_source, ETH_ALEN)
		|| memcmp(channel->serverMac, eth_hdr(skb)->h_dest, ETH_ALEN)) {
		rcu_read_unlock();
		kpppoe_rx_log(LOG_NOTICE, "pppoe session %u channle %p match failed\n",
									ntohs(ph->sid), channel);
		goto drop;
	}
	
	return pppoe_receive_frame(skb, pppoe, channel);
	
drop:
	kfree_skb(skb);
out:
	return NET_RX_DROP;
}

static int 
pppoe_packet_filter(struct sk_buff *skb) {
	struct net_device *dev;
	int ret;

	rcu_read_lock();
	dev = rcu_dereference(skb->dev->pppoe_ptr);
	if(unlikely(!dev || !(dev->flags & IFF_UP))) {
		rcu_read_unlock();	
		kfree_skb(skb);		
		kpppoe_rx_log(LOG_WARNING, "catch pppoes, dev is NULL or is down\n");
		return NET_RX_DROP;
	}

	ret = pppoe_do_rcv(skb, netdev_priv(dev));
	rcu_read_unlock();

	kpppoe_rx_log(LOG_DEBUG, "pppoe_do_rcv, ret %d\n", ret);
	return ret;	
}


static struct net_device_stats *
pppoe_net_stats(struct net_device *dev) {
	struct pppoe_struct *pppoe = netdev_priv(dev);
	return &pppoe->stats;
}


static const struct net_device_ops pppoe_netdev_ops = {
	.ndo_start_xmit	= pppoe_start_xmit,
	.ndo_get_stats	= pppoe_net_stats,
};

static void 
pppoe_setup(struct net_device *dev) {
	dev->netdev_ops = &pppoe_netdev_ops;
	dev->hard_header_len = PPP_HEADER_LEN;
	dev->mtu = PPPOE_MTU;		//this mtu is ppp mtu
	dev->addr_len = 0;
	dev->tx_queue_len = 0;
	dev->type = ARPHRD_PPP;
	dev->flags = IFF_NOARP | IFF_RUNNING;
}

#ifdef CONFIG_PROC_FS
static int 
pppoe_seq_show(struct seq_file *seq, void *v) {
	struct pppoe_channel *ch;

	if (v == SEQ_START_TOKEN) {
		seq_puts(seq, "Id     rx_packets  rx_bytes  tx_packets  tx_bytes\n");
		goto out;
	}

	ch = v;
	seq_printf(seq, "%-5u  %llu %llu %llu %llu\n", ch->sid, 
				ch->stats.rx_packets, ch->stats.rx_bytes,
				ch->stats.tx_packets, ch->stats.tx_bytes);
out:
	return 0;
}

static inline struct pppoe_channel *
pppoe_get_idx(struct pppoe_struct *pppoe, loff_t pos) {
	struct pppoe_channel *channel;
	struct hlist_node *node;
	int i;

	for (i = 0; i < CHANNEL_HASHSIZE; i++) {
		hlist_for_each_entry_rcu(channel, node, &pppoe->sid_head[i], sid_hlist) {
			if (!pos--)
				return channel;
		}
	}

	return NULL;
}

static void *
pppoe_seq_start(struct seq_file *seq, loff_t *pos) {
	struct pppoe_struct *pppoe = seq->private;
	loff_t l = *pos;

	rcu_read_lock();
	return l ? pppoe_get_idx(pppoe, --l) : SEQ_START_TOKEN;
}

static void *
pppoe_seq_next(struct seq_file *seq, void *v, loff_t *pos) {
	struct pppoe_struct *pppoe = seq->private;
	struct pppoe_channel *ch;
	struct hlist_node *next;

	++*pos;
	if (v == SEQ_START_TOKEN)
		return pppoe_get_idx(pppoe, 0);

	ch = v;
	next = rcu_dereference(ch->sid_hlist.next);
	if (!next) {
		unsigned int hash = (ch->sid & (CHANNEL_HASHSIZE - 1)) + 1;		
		for (; hash < CHANNEL_HASHSIZE; hash++) {
			next = rcu_dereference(pppoe->sid_head[hash].first);
			if (next)
				goto out;
		}
		
		return NULL;
	}
	
out:
	return hlist_entry(next, struct pppoe_channel, sid_hlist);
}

static void 
pppoe_seq_stop(struct seq_file *seq, void *v) {
	rcu_read_unlock();
}

static const struct seq_operations pppoe_seq_ops = {
	.start		= pppoe_seq_start,
	.next		= pppoe_seq_next,
	.stop		= pppoe_seq_stop,
	.show		= pppoe_seq_show,
};

static int 
pppoe_seq_open(struct inode *inode, struct file *file) {
	struct seq_file *seq;
	struct pppoe_struct *pppoe;

	pppoe = PDE(inode)->data;
	if (!pppoe)
		return -ENXIO;

	if (seq_open(file, &pppoe_seq_ops))
		return -ENOMEM;	

	seq = file->private_data;
	seq->private = pppoe;
	return 0;
}


static int 
pppoe_seq_release(struct inode *inode, struct file *file) {
	struct seq_file *seq = file->private_data;
	seq->private = NULL;
	return seq_release(inode, file);
}


static const struct file_operations pppoe_seq_fops = {
	.owner		= THIS_MODULE,
	.open		= pppoe_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release		= pppoe_seq_release,
};
#endif

static inline void
_sock_unbind(struct pppoe_struct *pppoe) {
	struct sock *sk = pppoe->sk;
	struct pppoe_sock *pos;

	if (!sk)
		return;
	
	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD)) {
		release_sock(sk);		
		pppoe->sk = NULL;
		return;
	}

	pos = pppoe_sk(sk);
	pos->pppoe = NULL;
	pppoe->sk = NULL;
	
	sk->sk_state &= ~PPPOE_BIND;
	__sock_put(sk);	
	release_sock(sk);
	
	dev_put(pppoe->dev);	
}


static inline void
pppoe_unbase_rcu(struct pppoe_struct *pppoe) {
	struct net_device *base_dev;

	spin_lock_bh(&pppoe_lock);
	base_dev = pppoe->base_dev;
	if (!base_dev) {
		spin_unlock_bh(&pppoe_lock);
		return;
	}
	
	channel_hash_exit(pppoe);
	rcu_assign_pointer(pppoe->base_dev, NULL);
	rcu_assign_pointer(base_dev->pppoe_ptr, NULL);
	spin_unlock_bh(&pppoe_lock);

	synchronize_rcu();
	dev_put(base_dev);
	
	_sock_unbind(pppoe);

	rtnl_lock();
	dev_close(pppoe->dev);
	rtnl_unlock();
	
	dev_put(pppoe->dev);	
}

static void
pppoe_destroy(struct pppoe_struct *pppoe) {
	struct net_device *dev = pppoe->dev;

#ifdef CONFIG_PROC_FS
	remove_proc_entry(dev->name, proc_net_pppoe);
#endif

	list_del(&pppoe->next);
	pppoe_unbase_rcu(pppoe);
	unregister_netdev(dev);
	free_netdev(dev);
}

static void
deamon_list_release(struct list_head *list) {
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, list) {
		pppoe_destroy(list_entry(pos, struct pppoe_struct, next));
	}
	
	INIT_LIST_HEAD(list);
}

static int 
pppoe_deamon_register(struct pppoe_deamon_msg *message, pid_t pid) {
	if (message->local_id > 1 || message->instance_id > 16) {
		return -EINVAL;
	}

	instance_pppoed[message->local_id][message->instance_id].pid = pid;
	deamon_list_release(&instance_pppoed[message->local_id][message->instance_id].list);
	return 0;
}

static int 
pppoe_deamon_unregister(struct pppoe_deamon_msg *message) {
	if (message->local_id > 1 || message->instance_id > 16) {
		return -EINVAL;
	}

	instance_pppoed[message->local_id][message->instance_id].pid = 0;
	deamon_list_release(&instance_pppoed[message->local_id][message->instance_id].list);
	return 0;
}

static int
pppoe_interface_create(struct pppoe_interface_msg *message) {
	struct net_device *dev;
	struct pppoe_struct *pppoe;
	unsigned int local_id, instance_id;
	int ret;

	ret = ifname_get_instance(message->name, &local_id, &instance_id);
	if (ret) {
		kpppoe_ctl_log(LOG_WARNING, "input ifname %s error\n", 
									message->name);
		goto out;
	}

	if (local_id > 1 || !instance_id || instance_id > 16) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s get local_id %u instance_id %u error\n",
						message->name, local_id, instance_id);
		goto out;
	}
	
	dev = dev_get_by_name(&init_net, message->name);
	if (dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s is already exist\n", 
									message->name);
		dev_put(dev);
		ret = -EEXIST;
		goto out;
	}

	dev = alloc_netdev(sizeof(struct pppoe_struct), message->name, pppoe_setup);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "alloc_netdev %s failed\n", 
									message->name);
		ret = -ENOMEM;
		goto out;
	}
	
	pppoe = netdev_priv(dev);
	pppoe->mru = PPP_MRU;
	pppoe->dev = dev;
	spin_lock_init(&pppoe->sock_lock);

	list_add(&pppoe->next, &instance_pppoed[local_id][instance_id].list);
	channel_hash_init(pppoe);	

	ret = register_netdev(dev);
	if (ret) {
		kpppoe_ctl_log(LOG_WARNING, "register_netdev %s failed, ret %d\n", 
									dev->name, ret);
		goto out1;
	}

	message->ifindex = dev->ifindex;

#ifdef CONFIG_PROC_FS
	if (NULL == proc_create_data(dev->name, S_IRUGO, proc_net_pppoe, &pppoe_seq_fops, pppoe)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s proc_create_data failed\n", 
									dev->name);
		ret = -ENOMEM;
		goto out2;
	}
#endif

	return 0;

out2:
	unregister_netdev(dev);
out1:
	list_del(&pppoe->next);
	free_netdev(dev);
out:
	return ret;
}

static int
pppoe_interface_destroy(struct pppoe_interface_msg *message) {
	struct net_device *dev;
	unsigned int local_id, instance_id;

	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);		
		return -ENODEV;
	}

	if (ifname_get_instance(dev->name, &local_id, &instance_id)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);				
		dev_put(dev);
		return -EINVAL;
	}

	dev_put(dev);
	pppoe_destroy(netdev_priv(dev));
	return 0;
}

static int
pppoe_interface_base(struct pppoe_interface_msg *message) {
	struct net_device *dev, *base_dev;
	struct pppoe_struct *pppoe;
	int ret;

	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);		
		ret = -ENODEV;
		goto error;
	}

	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto error1;
	}
	
	base_dev = dev_get_by_name(&init_net, message->name);
	if (!base_dev) {
		kpppoe_ctl_log(LOG_WARNING, "input base interface %s is not exist\n", 
									message->name);
		ret = -ENODEV;
		goto error1;
	}

	pppoe = netdev_priv(dev);
	spin_lock_bh(&pppoe_lock);
	if (pppoe->base_dev) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "dev %s base interface %s is already exist\n", 
									dev->name, pppoe->base_dev->name);			
		ret = -EEXIST;
		goto error2;
	}

	if (base_dev->pppoe_ptr) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "pppoe_ptr of input base interface %s is already exist\n", 
									message->name);
		ret = -EEXIST;
		goto error2;
	}

	rcu_assign_pointer(pppoe->base_dev, base_dev);
	rcu_assign_pointer(base_dev->pppoe_ptr, dev);
	spin_unlock_bh(&pppoe_lock);
	
	synchronize_rcu();

	rtnl_lock();
	dev_open(dev);
	rtnl_unlock();
	return 0;

error2:
	dev_put(base_dev);
error1:
	dev_put(dev);
error:	
	return ret;
}

static int
pppoe_interface_unbase(struct pppoe_interface_msg *message) {
	struct net_device *dev, *base_dev;
	struct pppoe_struct *pppoe;
	int ret = 0;

	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}

	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}
	
	pppoe = netdev_priv(dev);
	spin_lock_bh(&pppoe_lock);
	if (!pppoe->base_dev) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "base interface of dev %s is not exist\n", 
									dev->name);
		ret = -ENODEV;
		goto out1;
	}

	base_dev = pppoe->base_dev;
	if (!base_dev) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "base interface of dev %s is not exist\n", 
									dev->name);
		ret = -ENODEV;
		goto out1;
	}	

	channel_hash_exit(pppoe);
	rcu_assign_pointer(pppoe->base_dev, NULL);
	rcu_assign_pointer(base_dev->pppoe_ptr, NULL);
	spin_unlock_bh(&pppoe_lock);
	
	synchronize_rcu();
	dev_put(base_dev);

	_sock_unbind(pppoe);	/* need pppoe socket unbind */

	rtnl_lock();
	dev_close(pppoe->dev);
	rtnl_unlock();
	
	dev_put(pppoe->dev);			
	
out1:
	dev_put(dev);
out:
	return ret;
}

static int 
pppoe_channel_register(struct pppoe_register_msg *message) {
	struct net_device*dev;
	struct pppoe_struct *pppoe;
	struct pppoe_channel *channel;
	int ret;
	
	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}
	
	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}

	if (!(dev->flags & IFF_UP)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s is not up\n", 
									dev->name);
		ret = -EINVAL;
		goto out1;
	}
	
	pppoe = netdev_priv(dev);
	channel = channel_alloc(message->sid, message->mac);
	if(unlikely(NULL == channel)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u alloc failed\n", 
									dev->name, ntohs(message->sid));
		ret = -ENOMEM;
		goto out1;
	}

	memcpy(channel->magic, message->magic, MAGIC_LEN);	
	memcpy(channel->serverMac, message->serverMac, ETH_ALEN);		

	spin_lock_bh(&pppoe_lock);
	ret = channel_sid_insert(pppoe, channel);
	spin_unlock_bh(&pppoe_lock);	
	if (ret) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u insert failed\n", 
									dev->name, ntohs(channel->sid));		
		__channel_free(channel);
		goto out1;
	}

out1:
	dev_put(dev);
out:
	return ret;
}

static int 
pppoe_channel_unregister(struct pppoe_register_msg *message) {
	struct net_device*dev;
	struct pppoe_struct *pppoe;
	struct pppoe_channel *channel;
	int ret = 0;
	
	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}
	
	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}
	
	pppoe = netdev_priv(dev);
	spin_lock_bh(&pppoe_lock);
	channel = channel_sid_remove(pppoe, message->sid);
	if (NULL == channel) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u is not exist\n", 
									dev->name, ntohs(message->sid));		
		ret = -ENOENT;
		goto out1;
	}

	if (NPMODE_PASS == channel->npmode[NP_IP]) {
		channel_ip_remove(pppoe, channel->ip);
	}

	spin_unlock_bh(&pppoe_lock);
	channel_free(channel);

out1:
	dev_put(dev);
out:
	return ret;
}

static int
pppoe_channel_authorize(struct pppoe_authorize_msg *message) {
	struct net_device*dev;
	struct pppoe_struct *pppoe;
	struct pppoe_channel *channel;
	int ret;
	
	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}
	
	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}

	if (!(dev->flags & IFF_UP)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s is not up\n", 
									dev->name);
		ret = -EINVAL;
		goto out1;
	}

	pppoe = netdev_priv(dev);
	spin_lock_bh(&pppoe_lock);
	channel = channel_get_by_sid(pppoe, message->sid);
	if (!channel) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u is not exist\n", 
									dev->name, ntohs(message->sid));
		ret = -ENOENT;
		goto out1;
	}

	if (NPMODE_PASS == channel->npmode[NP_IP]) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u ip %u.%u.%u.%u is already insert\n", 
									dev->name, ntohs(message->sid), NIPQUAD(message->ip));
		ret = -EEXIST;
		goto out1;
	}

	channel->ip = message->ip;
	ret = channel_ip_insert(pppoe, channel);
	spin_unlock_bh(&pppoe_lock);
	if (ret) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u ip %u.%u.%u.%u insert failed\n", 
									dev->name, ntohs(message->sid), NIPQUAD(message->ip));		
		goto out1;
	}
	
	channel->npmode[NP_IP] = NPMODE_PASS;

out1:
	dev_put(dev);
out:
	return ret;
}

static int
pppoe_channel_unauthorize(struct pppoe_authorize_msg *message) {
	struct net_device*dev;
	struct pppoe_channel *channel;
	int ret = 0;

	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}
	
	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}

	spin_lock_bh(&pppoe_lock);	
	channel = channel_ip_remove(netdev_priv(dev), message->ip);
	spin_unlock_bh(&pppoe_lock);	
	if (!channel) {		
		kpppoe_ctl_log(LOG_WARNING, "dev %s channel %u ip %u.%u.%u.%u is not exist\n", 
									dev->name, ntohs(message->sid), NIPQUAD(message->ip));		
		ret = -ENOENT;
		goto out1;
	}
	
	channel->npmode[NP_IP] = NPMODE_DROP;

out1:
	dev_put(dev);
out:
	return ret;
}

static int
pppoe_channel_clear(struct pppoe_interface_msg *message) {
	struct net_device *dev;
	int ret = 0;

	dev = dev_get_by_index(&init_net, message->ifindex);
	if (!dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d is not exist\n", 
									message->ifindex);
		ret = -ENODEV;
		goto out;
	}

	if (strncmp(dev->name, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev ifindex %d name %s is not pppoe interface\n",
									message->ifindex, dev->name);
		ret = -EINVAL;
		goto out1;
	}

	spin_lock_bh(&pppoe_lock);		
	channel_hash_exit(netdev_priv(dev));
	spin_unlock_bh(&pppoe_lock);		

out1:
	dev_put(dev);
out:
	return ret;
}

static void
netlink_unbase_signal(struct net_device *dev) {
	struct sk_buff *skb;
	struct nlmsghdr *nlhdr;
	struct pppoe_message *mess;
	unsigned int local_id, instance_id;
	u32 pid;

	if (ifname_get_instance(dev->name, &local_id, &instance_id)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s get instance info failed\n", 
									dev->name);
		return;
	}
	
	if (local_id > 1 || !instance_id || instance_id > 16) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s get local_id %u instance_id %u error\n",
									dev->name, local_id, instance_id);
		return;
	}

	pid = instance_pppoed[local_id][instance_id].pid;
	if (!pid) {
		kpppoe_ctl_log(LOG_WARNING, "local_id %u instance_id %u pid is not exist\n",
									local_id, instance_id);
		return;
	}

	skb = nlmsg_new(sizeof(struct pppoe_message), GFP_KERNEL);
	if (!skb) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s nlmsg_new failed\n", dev->name);
		return;
	}

	nlhdr = nlmsg_put(skb, 0, 0, 0, sizeof(struct pppoe_message), 0);
	if (!nlhdr) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s nlmsg_put failed\n", 
									dev->name);
		nlmsg_free(skb);
		return;
	}

	mess = (struct pppoe_message *)NLMSG_DATA(nlhdr);
	mess->type = PPPOE_MESSAGE_SIGNAL;
	mess->code = PPPOE_INTERFACE_UNBASE;
	mess->data.m_interface.ifindex = dev->ifindex;
	memcpy(mess->data.m_interface.name, dev->name, IFNAMSIZ);
	mess->datalen = sizeof(struct pppoe_interface_msg);
	
	netlink_unicast(pppoe_netlink, skb, pid, MSG_DONTWAIT);
}

static void
netlink_message_handler(struct sk_buff *skb) {
	u32 pid;  
	int errorcode = 0, err;
	struct pppoe_message *mess;
	struct nlmsghdr *nlhdr = (struct nlmsghdr *)skb->data;

	if (skb->len < NLMSG_SPACE(0) || skb->len < nlhdr->nlmsg_len ||
		nlhdr->nlmsg_len < NLMSG_LENGTH(sizeof(struct pppoe_message))) {
		kpppoe_ctl_log(LOG_WARNING, "Corrupt netlink message\n");
		goto error;
	}

	mess = (struct pppoe_message *)NLMSG_DATA(nlhdr);
	kpppoe_ctl_log(LOG_DEBUG, "recv netlink message, type = %d, code = %d\n",
					mess->type, mess->code);

	if (PPPOE_MESSAGE_REQUEST != mess->type) {
		kpppoe_ctl_log(LOG_WARNING, "netlink message is not request\n");	
		goto error;
	}

	switch (mess->code) {
	case PPPOE_NETLINK_REGISTER:
		if (mess->datalen < sizeof(struct pppoe_deamon_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "netlink register message datalen errror\n");
			break;
		}
		errorcode = pppoe_deamon_register(&mess->data.m_deamon, nlhdr->nlmsg_pid);
		break;

	case PPPOE_NETLINK_UNREGISTER:
		if (mess->datalen < sizeof(struct pppoe_deamon_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "netlink unregister message datalen errror\n");
			break;
		}
		errorcode = pppoe_deamon_unregister(&mess->data.m_deamon);
		break;

	case PPPOE_INTERFACE_CREATE:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "interface create message datalen errror\n");
			errorcode = -EINVAL;
			break;
		}
		
		errorcode = pppoe_interface_create(&mess->data.m_interface);
		break;
			
	case PPPOE_INTERFACE_DESTROY:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "interface destroy message datalen errror\n");
			errorcode = -EINVAL;
			break;
		}
		
		errorcode = pppoe_interface_destroy(&mess->data.m_interface);
		break;

	case PPPOE_INTERFACE_BASE:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "interface base message datalen errror\n");
			errorcode = -EINVAL;
			break;
		}

		errorcode = pppoe_interface_base(&mess->data.m_interface);
		break;
		
	case PPPOE_INTERFACE_UNBASE:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "interface unbase message datalen errror\n");
			errorcode = -EINVAL;
			break;
		}
		
		errorcode = pppoe_interface_unbase(&mess->data.m_interface);
		break;
		
	case PPPOE_CHANNEL_REGISTER:
		if (mess->datalen < sizeof(struct pppoe_register_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "channel register message datalen errror\n");
			errorcode = -EINVAL;
			break;
		}

		errorcode = pppoe_channel_register(&mess->data.m_register);
		break;
		
	case PPPOE_CHANNEL_UNREGISTER:
		if (mess->datalen < sizeof(struct pppoe_register_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "channel unregister message datalen errror\n");
			break;	
		}
		
		errorcode = pppoe_channel_unregister(&mess->data.m_register);
		break;
		
	case PPPOE_CHANNEL_AUTHORIZE:
		if (mess->datalen < sizeof(struct pppoe_authorize_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "channel authorize message datalen errror\n");
			break;
		}
		
		errorcode = pppoe_channel_authorize(&mess->data.m_authorize);
		break;
		
	case PPPOE_CHANNEL_UNAUTHORIZE:
		if (mess->datalen < sizeof(struct pppoe_authorize_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "channel unauthorize message datalen errror\n");
			break;
		}

		errorcode = pppoe_channel_unauthorize(&mess->data.m_authorize);
		break;

	case PPPOE_CHANNEL_CLEAR:
		if (mess->datalen < sizeof(struct pppoe_interface_msg)) {
			kpppoe_ctl_log(LOG_WARNING, "channel unauthorize message datalen errror\n");
			break;
		}

		errorcode = pppoe_channel_clear(&mess->data.m_interface);
		break;


	default:
		kpppoe_ctl_log(LOG_WARNING, "unknow netlink message %u\n", mess->code);
		goto error;
	}

	kpppoe_ctl_log(LOG_DEBUG, "process message return %d\n", errorcode);

	mess->type = PPPOE_MESSAGE_REPLY;
	mess->errorcode = errorcode;

	pid = nlhdr->nlmsg_pid;				/*pid of sending process */
	NETLINK_CB(skb).pid = 0;			/* from kernel */
	NETLINK_CB(skb).dst_group = 0;	/* unicast */

	err = netlink_unicast(pppoe_netlink, skb, pid, MSG_DONTWAIT);
	kpppoe_ctl_log(LOG_DEBUG, "send netlink message reply, err %d\n", err);
	return ;

error:
	kfree_skb(skb);
	return ;
}

static void 
netlink_rcv(struct sk_buff *skb) {
	kpppoe_ctl_log(LOG_DEBUG, "recv a netlink message\n");		
	skb_queue_tail(&pppoe_netlink->sk_receive_queue, skb_get(skb));
	wake_up(pppoe_netlink->sk_sleep);
}

static int
netlink_process_thread(void *data) {
	struct sk_buff *skb;

	daemonize("kpppoe-netlink");

	while (!exit_flag) {
		DEFINE_WAIT(wait);
		
		for (;;) {
			prepare_to_wait(pppoe_netlink->sk_sleep, &wait, TASK_INTERRUPTIBLE);
			if (!skb_queue_empty(&pppoe_netlink->sk_receive_queue) || exit_flag)
				break;
			
			schedule();
		}
		finish_wait(pppoe_netlink->sk_sleep, &wait);

		while ((skb = skb_dequeue(&pppoe_netlink->sk_receive_queue))) {
			netlink_message_handler(skb);
		}
	}
	
	complete(&exit_completion);
	return 0;
}

int
pppoe_netlink_init(void) {
	int pid;

	pppoe_netlink = netlink_kernel_create(&init_net, NETLINK_PPPOE, 0,
										netlink_rcv, NULL, THIS_MODULE);
	if (!pppoe_netlink) {
		printk(KERN_ERR "PPPOE: create netlink socket failed.\n");
		goto error;
	}
	
	pid = kernel_thread(netlink_process_thread, NULL, CLONE_KERNEL);
	if (pid < 0) {
		printk(KERN_ERR "PPPOE: kernel_thread() netlink failed.\n");
		goto error1;
	}
	
	return 0;

error1:
	netlink_kernel_release(pppoe_netlink);	
	pppoe_netlink = NULL;
error:
	return -1;
}

static void 
pppoe_netlink_release(void) {
	exit_flag = 1;
	wake_up(pppoe_netlink->sk_sleep);
	wait_for_completion(&exit_completion);
	netlink_kernel_release(pppoe_netlink);
	pppoe_netlink = NULL;
}


static int 
pppoe_sock_release(struct socket *sock) {
	struct sock *sk = sock->sk;
	struct pppoe_sock *pos;
	int error = 0;

	if (!sk)
		return -EINVAL;

	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD)) {
		release_sock(sk);
		return -EBADF;
	}

	pos = pppoe_sk(sk);
	if (pos->pppoe) {
		sk->sk_state &= ~PPPOE_BIND;
		__sock_put(sk);
		
		pos->pppoe->sk = NULL;	/* this no need lock pppoe->sock_lock*/
		dev_put(pos->pppoe->dev);		
		pos->pppoe = NULL;
	}

	sock_orphan(sk);
	sock->sk = NULL;

	skb_queue_purge(&sk->sk_receive_queue);
	release_sock(sk);
	sock_put(sk);
	return error;
}

static int
pppoe_do_bind(struct sock *sk, struct net_device *dev) {
	struct pppoe_sock *pos = pppoe_sk(sk);
	struct pppoe_struct *pppoe = netdev_priv(dev);
	int err = 0;
	
	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD)) {
		err = -EBADF;
		goto out_unlock;
	}
	
	if (pos->pppoe) {
		sk->sk_state &= ~PPPOE_BIND;
		__sock_put(sk);

		pos->pppoe->sk = NULL;
		dev_put(pos->pppoe->dev);		
		pos->pppoe = NULL;
	}

	spin_lock(&pppoe->sock_lock);	
	if (!pppoe->sk) {
		if (dev->flags & IFF_UP) {
			pppoe->sk = sk;
			
			spin_unlock(&pppoe->sock_lock);	
			pos->pppoe = pppoe;			
			sk->sk_state |= PPPOE_BIND;
			sock_hold(sk);
			goto out_unlock;	
		}
		
		spin_unlock(&pppoe->sock_lock);	
		err = -ENETDOWN;
		sk->sk_err = ENETDOWN;
		kpppoe_ctl_log(LOG_WARNING, "dev %s is not up\n", dev->name);
		goto out_report;
	}

	spin_unlock(&pppoe->sock_lock);	
	err = -EADDRINUSE;
	sk->sk_err = EADDRINUSE;
	kpppoe_ctl_log(LOG_WARNING, "dev %s is already bind socket\n", dev->name);

out_report:	
	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_error_report(sk);
out_unlock:
	release_sock(sk);
	return err;
}


static int 
pppoe_sock_bind(struct socket *sock, struct sockaddr *addr, int addr_len) {
	struct net_device *dev;
	struct sock *sk = sock->sk;
	struct sockaddr_pppoe *poaddr = (struct sockaddr_pppoe *)addr;
	int err = -EINVAL;

	if (addr_len < sizeof(struct sockaddr_pppoe))
		goto out;
	
	if (poaddr->sa_family != AF_PPPOE)
		goto out;

	if (strncmp(poaddr->addr.dev, "pppoe", 5)) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s is not pppoe interface\n", 
									poaddr->addr.dev);		
		goto out;
	}

	dev = dev_get_by_name(&init_net, poaddr->addr.dev);
	if (NULL == dev) {
		kpppoe_ctl_log(LOG_WARNING, "dev %s is not exist\n", 
									poaddr->addr.dev);
		err = -ENODEV;
		goto out;
	}

	err = pppoe_do_bind(sk, dev);	/* if success, socket will hold dev */
	if (err) {
		kpppoe_ctl_log(LOG_WARNING, "pppoe socket bind %s failed, err %d\n",
									dev->name, err);
		dev_put(dev);
		goto out;
	}

out:
	return err;
}


static int 
pppoe_sock_sendmsg(struct kiocb *iocb, struct socket *sock, 
				struct msghdr *m, size_t total_len)
{
	struct sk_buff *skb;
	struct net_device *dev;
	struct sock *sk = sock->sk;
	struct pppoe_sock *pos = pppoe_sk(sk);
	struct sockaddr_pppoe *poaddr;
	struct pppoe_channel *channel;
	struct pppoe_hdr *ph;
	int error;

	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD) || !(sk->sk_state & PPPOE_BIND)) {
		error = -ENOTCONN;
		goto end;
	}
	
	error = -EMSGSIZE;
	if (m->msg_namelen < sizeof(struct sockaddr_pppoe))
		goto end;

	 if (total_len > (PPPOE_MTU + PPP_HEADER_LEN))
		goto end;

	rcu_read_lock();
	dev = rcu_dereference(pos->pppoe->base_dev);
	if (!dev || !(dev->flags & IFF_UP)) {
		error = -ENOTCONN;
		goto end;
	}
	
	poaddr = m->msg_name;
	channel = channel_get_by_sid(pos->pppoe, poaddr->addr.sid);
	if (!channel || memcmp(channel->mac, poaddr->addr.mac, ETH_ALEN)) {
		rcu_read_unlock();
		error = -ENOTCONN;
		goto end;
	}

	skb = sock_wmalloc(sk, total_len + sizeof(struct pppoe_hdr) + 
					dev->hard_header_len + 32, 0, GFP_KERNEL);
	if (!skb) {
		error = -ENOMEM;
		goto end;
	}

	/* Reserve space for headers. */
	skb_reserve(skb, dev->hard_header_len);
	skb_reset_network_header(skb);

	ph = (struct pppoe_hdr *)skb_put(skb, sizeof(*ph));	
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = poaddr->addr.sid;
	ph->length = __constant_htons(total_len);

	error = memcpy_fromiovec(skb_put(skb, total_len), m->msg_iov, total_len);
	if (error < 0) {
		kfree_skb(skb);
		goto end;
	}

	skb->dev = dev;
	skb->priority = sk->sk_priority;
	skb->protocol = __constant_htons(ETH_P_PPP_SES);
	dev_hard_header(skb, dev, ETH_P_PPP_SES, poaddr->addr.mac, 
					channel->serverMac, skb->len);
	dev_queue_xmit(skb);
	rcu_read_unlock();
	
	error = total_len;

end:
	release_sock(sk);
	return error;
}


static int 
pppoe_sock_recvmsg(struct kiocb *iocb, struct socket *sock,
				struct msghdr *m, size_t total_len, int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	int error = 0;

	if (sock_flag(sk, SOCK_DEAD) || !(sk->sk_state & PPPOE_BIND)) {
		error = -ENOTCONN;
		goto end;
	}

	skb = skb_recv_datagram(sk, flags, flags & MSG_DONTWAIT, &error);
	if (error < 0) {
		goto end;
	}

	if (skb) {
		struct pppoe_hdr *ph = pppoe_hdr(skb);
		int len = ntohs(ph->length);
		m->msg_namelen = 0;
		
		if (m->msg_name) {
			struct sockaddr_pppoe *poaddr = (struct sockaddr_pppoe *)m->msg_name;
			poaddr->sa_family = AF_PPPOE;
			poaddr->addr.sid = ph->sid;
			memcpy(poaddr->addr.mac, eth_hdr(skb)->h_source, ETH_ALEN);	
			m->msg_namelen = sizeof(*poaddr);
		}	
		
		error = memcpy_toiovec(m->msg_iov, skb->data, len);
		if (!error)
			error = len;

		kfree_skb(skb);
	}

end:
	return error;
}


static const struct proto_ops pppoe_ops = {
    .family			= AF_PPPOE,
    .owner		= THIS_MODULE,
    .release		= pppoe_sock_release,
    .bind			= pppoe_sock_bind,
    .connect		= sock_no_connect,
    .socketpair		= sock_no_socketpair,
    .accept		= sock_no_accept,
    .getname		= sock_no_getname,
    .poll			= datagram_poll,
    .listen			= sock_no_listen,
    .shutdown		= sock_no_shutdown,
    .setsockopt	= sock_no_setsockopt,
    .getsockopt	= sock_no_getsockopt,
    .sendmsg		= pppoe_sock_sendmsg,
    .recvmsg		= pppoe_sock_recvmsg,
    .mmap			= sock_no_mmap,
    .ioctl			= sock_no_ioctl,
};

static struct proto pppoe_sk_proto = {
	.name	  = "PPPOE",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct pppoe_sock),
};


static int 
pppoe_sock_create(struct net *net, struct socket *sock) {
	struct sock *sk
		= sk_alloc(net, PF_PPPOE, GFP_KERNEL, &pppoe_sk_proto);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);

	sock->state = SS_CONNECTED;
	sock->ops   = &pppoe_ops;
	
	sk->sk_state = PPPOE_NONE;
	sk->sk_type	= SOCK_STREAM;
	sk->sk_family = PF_PPPOE;
	sk->sk_protocol	= PX_PROTO_OE;
	return 0;
}


static struct pppoe_proto sock_proto = {
    .create	= pppoe_sock_create,
    .owner	= THIS_MODULE,
};

static int 
pppoe_create(struct net *net,struct socket *sock, int protocol)
{
	int rc = -EPROTOTYPE;

	if (protocol < 0)
		goto out;

	rc = -EPROTONOSUPPORT;
	if (!try_module_get(sock_proto.owner))
		goto out;

	rc = sock_proto.create(net, sock);

	module_put(sock_proto.owner);
out:
	return rc;
}

static struct net_proto_family pppoe_proto_family = {
	.family	= PF_PPPOE,
	.create	= pppoe_create,
	.owner	= THIS_MODULE,
};

static int 
pppoe_stop_dev(struct net_device *dev) {
	struct net_device *pppoe_dev;
	struct pppoe_struct *pppoe;
	
	BUG_ON(NULL == dev);

	spin_lock_bh(&pppoe_lock);
	pppoe_dev = dev->pppoe_ptr;
	if (!pppoe_dev) {
		spin_unlock_bh(&pppoe_lock);
		return NOTIFY_DONE;
	}
	
	if (memcmp(pppoe_dev->name, "pppoe", 5)) {
		spin_unlock_bh(&pppoe_lock);
		kpppoe_ctl_log(LOG_WARNING, "dev %s has pppoe_ptr, "
					"but pppoe_ptr is not pppoe dev\n", dev->name);
		return NOTIFY_DONE;
	}
	
	kpppoe_ctl_log(LOG_NOTICE, "dev %s pppoe_ptr %s, need tell deamon\n",
					dev->name, pppoe_dev->name);

	pppoe = netdev_priv(pppoe_dev);
	channel_hash_exit(pppoe);

	rcu_assign_pointer(pppoe->base_dev, NULL);
	rcu_assign_pointer(dev->pppoe_ptr, NULL);
	spin_unlock_bh(&pppoe_lock);

	synchronize_rcu();
	dev_put(dev);

	/* need tell pppoe deamon */
	netlink_unbase_signal(pppoe_dev);

	dev_close(pppoe_dev); /* this call event is already rtnl_lock */	
	dev_put(pppoe_dev);	
	return NOTIFY_OK;
}


static int
pppoe_device_event(struct notifier_block *this,
			      		unsigned long event, void *ptr) {
	struct net_device *dev = (struct net_device *)ptr;

	switch (event) {
	case NETDEV_UNREGISTER:
		return pppoe_stop_dev(dev);

	default:
		break;
	};

	return NOTIFY_DONE;
}

static struct notifier_block pppoe_notifier = {
	.notifier_call = pppoe_device_event,
};

static int
pppoe_net_ns_init(void) {
#ifdef CONFIG_PROC_FS
	proc_net_pppoe = proc_net_mkdir(&init_net, "dev_pppoe", init_net.proc_net);
	if (!proc_net_pppoe) {
		printk(KERN_ERR "pppoe net ns init failed\n");
		return 1;
	}
#endif	
	return 0;
}

static void
pppoe_net_ns_exit(void) {
#ifdef CONFIG_PROC_FS
	remove_proc_entry("dev_pppoe",  init_net.proc_net);
#endif
}

static void
pppoe_deamon_init(void) {
	int i, j;
	
	memset(&instance_pppoed, 0, sizeof(instance_pppoed));
	
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 17; j++) {
			INIT_LIST_HEAD(&(instance_pppoed[i][j].list));
		}
	}
}

static void
pppoe_deamon_exit(void) {
	int i, j;
		
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 17; j++) {
			instance_pppoed[i][j].pid = 0;
			deamon_list_release(&instance_pppoed[i][j].list);
		}
	}	
}

static int __init 
pppoe_init(void) {
	if (proto_register(&pppoe_sk_proto, 0))
		goto error;

	if (pppoe_cache_init()) 
		goto error1;

	if (pppoe_netlink_init()) 
		goto error2;

	if (pppoe_net_ns_init())
		goto error3;

	pppoe_deamon_init();
	spin_lock_init(&pppoe_lock);	
	sock_register(&pppoe_proto_family);
	register_netdevice_notifier(&pppoe_notifier);
	pppoe_rx_hook = pppoe_packet_filter;
	printk(KERN_INFO "kpppoe moudule init success\n");
	return 0;

error3:
	pppoe_netlink_release();
error2:
	pppoe_cache_destroy();
error1:
	proto_unregister(&pppoe_sk_proto);
error:
	return 1;
}

static void __exit
pppoe_exit(void) {
	pppoe_rx_hook = NULL;
	unregister_netdevice_notifier(&pppoe_notifier);
	sock_unregister(PF_PPPOE);
	pppoe_deamon_exit();
	pppoe_net_ns_exit();
	pppoe_netlink_release();
	pppoe_cache_destroy();
}

module_init(pppoe_init);
module_exit(pppoe_exit);
MODULE_LICENSE("GPL");
