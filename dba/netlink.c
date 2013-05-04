#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/mii.h>

#include <net/sock.h>
#include <net/protocol.h>
#include <net/dst.h>
#include <net/checksum.h>
#include <net/xfrm.h>

#include "pppoe_snooping.h"
#include "netlink.h"
#include "log.h"

/* define in pppoe_snooping.c */
extern unsigned short pppoe_snp_mru;

/* net/bridge/br_input.c */
extern unsigned int dba_enable_flag;

/* net/core/dev.c */
extern unsigned int pppoe_snooping_enable_flag;

/* define in pppoe_snp_kernlog.c */
extern int pppoe_snp_log_level;


static struct sock *netlink_sock = NULL;

static void netlink_recv_handler(struct sk_buff *skb);
static int netlink_msg_handler(struct msg_type *msg);

/* PPPoE snooping */
static int pppoe_snp_setmru(pppoe_msg_t *msg);
static int pppoe_snooping_enable(pppoe_msg_t *msg);
static int pppoe_snp_set_log_level(pppoe_msg_t *msg);
static int pppoe_snooping_msg_handler(pppoe_msg_t *msg);


/* DBA(dhcp broadcast agent) */
static int dba_msg_handler(dba_msg_t *msg);
static int dba_server_enable(dba_msg_t *msg);


int pppoe_snooping_netlink_init(void)
{

	/* netlink init */
	netlink_sock = netlink_kernel_create(&init_net, NETLINK_PPPOE_SNP, 0, netlink_recv_handler, NULL, THIS_MODULE);
	if(!netlink_sock) {
		printk(KERN_ERR "Create pppoe netlink socket failed.\n");
		return -1;
	}
	printk(KERN_INFO "Creat PPPoE snooping netlink socket success.\n");

	return 0;
}

int pppoe_snooping_netlink_release(void)
{
	sock_release(netlink_sock->sk_socket);
	printk(KERN_INFO "pppoe snooping netlink socket released!\n");
	return 0;
}

static void netlink_recv_handler(struct sk_buff *skb)
{
	struct nlmsghdr *nlhdr = NULL;

	log_debug("receive pppoe snooping netlink message\n");

	//while((skb = skb_dequeue(&netlink_sock->sk_receive_queue)) != NULL) 
	{
		nlhdr = (struct nlmsghdr *)skb->data;
		if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
			printk(KERN_ERR "Corrupt netlink message\n");
			return;
		}
		netlink_msg_handler((struct msg_type *)NLMSG_DATA(nlhdr));
	}

	return ;
}

static int pppoe_snooping_enable(pppoe_msg_t *msg)
{
	if (!msg) {
		log_error("receive invalid enable soonping netlink msg: no data.\n");
		return -1;
	}
	
	if (msg->data.enable_flag) {
		pppoe_snooping_enable_flag = msg->data.enable_flag;
	} else {
		pppoe_snooping_enable_flag = 0;
	}
	
	log_info("set pppoe snooping %s(%d).\n", 
		pppoe_snooping_enable_flag ? "enable" : "disable", pppoe_snooping_enable_flag);
	
	return 0;
}


static int pppoe_snp_setmru(pppoe_msg_t *msg)
{
	if (!msg) {
		log_error("receive invalid set mru netlink msg: no data.\n");
		return -1;
	}
	
	pppoe_snp_mru = msg->data.mru;


	if (pppoe_snp_mru < MRU_MIN) {
		pppoe_snp_mru = MRU_DEFAULT;
	}
	if (pppoe_snp_mru > MRU_MAX) {
		pppoe_snp_mru = MRU_DEFAULT;
	}

	log_info("set pppoe snooping mru %d\n", pppoe_snp_mru);
	
	return 0;
}



static int pppoe_snp_set_log_level(pppoe_msg_t *msg)
{
	if (!msg) {
		log_error("receive invalid set log netlink msg: no data.\n");
		return -1;
	}

	if (msg->data.debug.set_flag) {
		pppoe_snp_log_level |= msg->data.debug.log_level & PPPOE_SNP_LOG_ALL;
	} else {
		pppoe_snp_log_level &= msg->data.debug.log_level ^ PPPOE_SNP_LOG_ALL;
	}

	log_info("set pppoe snooping log level %x\n", pppoe_snp_log_level);
	
	return 0;
}

static int netlink_msg_handler(struct msg_type *msg)
{
	int ret = 0;

	if(!msg)
		return -1;

	log_debug("type %d subtype %d len %d\n", msg->type, msg->subtype, msg->datalen);
	
	switch(msg->type) {
		case pppoe_snp_type:
			pppoe_snooping_msg_handler((pppoe_msg_t *)msg);
			break;
		case dba_msg_type:
			dba_msg_handler((dba_msg_t *)msg);
			break;
		default:
			break;
	}

	return ret;
}

static int pppoe_snooping_msg_handler(pppoe_msg_t *msg)
{
	int ret = 0;

	if(!msg)
		return -1;

	switch(msg->subtype) {
		case pppoe_snp_subtype_enable:
			ret = pppoe_snooping_enable(msg);
			break;
		case pppoe_snp_subtype_setmru:
			ret = pppoe_snp_setmru(msg);
			break;
		case pppoe_snp_subtype_logswitch:
			ret = pppoe_snp_set_log_level(msg);
			break;
			
		default:
			break;
	}

	return ret;
}


static int dba_msg_handler(dba_msg_t *msg)
{
	int ret = 0;

	if(!msg)
		return -1;

	switch(msg->subtype) {
		case dba_subtype_enable:
			ret = dba_server_enable(msg);
			break;		
		default:
			break;
	}

	return ret;
}

static int dba_server_enable(dba_msg_t *msg)
{
	if (!msg) {
		log_error("receive invalid enable soonping netlink msg: no data.\n");
		return -1;
	}
	
	if (msg->data.enable_flag) {
		dba_enable_flag = msg->data.enable_flag;
	} else {
		dba_enable_flag = 0;
	}
	
	log_info("set dba server %s.\n", dba_enable_flag ? "enable" : "disable");
	
	return 0;
}

