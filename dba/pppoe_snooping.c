#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#include "log.h"
#include "main.h"
#include "pppoe_snooping.h"

unsigned short pppoe_snp_mru = MRU_DEFAULT;

static int pppoe_lcp_prase(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res);
static int pppoe_lcp_cfg_ack(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res);
static int pppoe_lcp_cfg_request(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res);
static int pppoe_lcp_check_mru(struct sk_buff *skb, struct pppoe_proto *proto, dba_result_t *res);
static int pppoe_lcp_nak_mru(struct sk_buff *skb, struct pppoe_proto *proto);
static void *pppoe_get_option(struct pppoe_proto *proto, __u8 opt_type);
static int pppoe_lcp_change_mru(struct pppoe_proto *proto);


/* PPPoE sesion packet */
int parse_pppoe_session(struct sk_buff *skb, struct pppoe_hdr *pppoe_hdr, dba_result_t *res)
{
	int ret = 0;

	if (unlikely(!pppoe_hdr)) {
		return -1;
	}

	if (unlikely(((unsigned int)skb->tail - (unsigned int)eth_hdr(skb)) 
		< (ntohs(pppoe_hdr->length) + sizeof(struct ethhdr)))) {
		log_error("wrong pakcet: skb len %d pppoe header length %d.\n", 
			(unsigned int)skb->tail - (unsigned int)eth_hdr(skb), (ntohs(pppoe_hdr->length)));
		return -1;
	}

	switch (ntohs(pppoe_hdr->tag->tag_type)) {
		case PPP_LCP:	/* Link Control Protocol */
			ret = pppoe_lcp_prase(skb, (struct pppoe_proto *)&(pppoe_hdr->tag->tag_len), res);
			break;
		case PPP_CHAP:
			break;
		case PPP_IPCP:
			break;
		case PPP_CCP:
			break;

		default:
			break;
	}
	
	return ret;
}


static int pppoe_lcp_prase(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res)
{
	if (unlikely(!lcp_hdr)) {
		return -1;
	}

	switch (lcp_hdr->code) {
		case PPPoE_LCP_CFG_REQ:
			pppoe_lcp_cfg_request(skb, lcp_hdr, res);
			break;
		case PPPoE_LCP_CFG_ACK:
			pppoe_lcp_cfg_ack(skb, lcp_hdr, res);
			break;
		#if 0
		case PPPoE_LCP_CFG_NAK:
			pppoe_lcp_cfg_nak(skb, lcp_hdr);
			break;
		case PPPoE_LCP_CFG_REJ:
			pppoe_lcp_cfg_reject(skb, lcp_hdr);
			break;
		case PPPoE_LCP_TER_REQ:
			pppoe_lcp_terminate_request(skb, lcp_hdr);
			break;
		case PPPoE_LCP_TER_ACK:
			pppoe_lcp_terminate_ack(skb, lcp_hdr);
			break;
		case PPPoE_LCP_CODE_REJ:
			pppoe_lcp_code_reject(skb, lcp_hdr);
			break;
		case PPPoE_LCP_PROTO_REJ:
			pppoe_lcp_protocol_reject(skb, lcp_hdr);
			break;
		case PPPoE_LCP_ECHO_REQ:
			pppoe_lcp_echo_request(skb, lcp_hdr);
			break;
		case PPPoE_LCP_ECHO_REP:
			pppoe_lcp_echo_reply(skb, lcp_hdr);
			break;
		case PPPoE_LCP_DIS_REQ:
			pppoe_lcp_discard_request(skb, lcp_hdr);
			break;
		#endif
		default:
			break;
	}
	return 0;
}

static int pppoe_lcp_cfg_ack(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res)
{
	return 0;
}
static int pppoe_lcp_cfg_request(struct sk_buff *skb, struct pppoe_proto *lcp_hdr, dba_result_t *res)
{
	return pppoe_lcp_check_mru(skb, lcp_hdr, res);
}

static int pppoe_lcp_check_mru(struct sk_buff *skb, struct pppoe_proto *proto, dba_result_t *res)
{		
	int ret = 0;
	struct pppoe_option *opt = NULL;
	unsigned short mru = pppoe_snp_mru;

	if (!proto || !res) {
		return -1;
	}

	if (unlikely(((unsigned int)skb->tail - (unsigned int)eth_hdr(skb))
		< (ntohs(proto->length) + sizeof(struct ethhdr) + sizeof(struct pppoe_hdr) + 2))) {
		log_error("wrong pakcet: skb data len %d pppoe proto length %d.\n", 
			(unsigned int)skb->tail - (unsigned int)eth_hdr(skb), (ntohs(proto->length)));
		return -1;
	}

	opt = pppoe_get_option(proto, LCP_OPT_MRU);
	if (opt) {
		mru = ((opt->data[0]<<8) & 0xff00) | (opt->data[1] & 0xff);
	} else {
		log_error("get MRU for lcp packet failed.\n");
		return -1;
	}

	if (mru > pppoe_snp_mru) {
		ret = pppoe_lcp_nak_mru(skb, proto);
		if (ret) {
			log_error("NAK MRU %d failed.\n", mru);
			return ret;
		}
		res->result |= DBA_CONSUMED;		
	}

	return ret;
}

static int pppoe_lcp_nak_mru(struct sk_buff *skb, struct pppoe_proto *proto)
{
	unsigned char *len = NULL;
	struct pppoe_option *opt = NULL;

	struct ethhdr *ethhdr = NULL;
	unsigned char h_dest[ETH_ALEN];

	if (!skb || !proto) {
		return -1;
	}
	ethhdr = eth_hdr(skb);

/*
	printk(KERN_INFO "h_soure %02x:%02x:%02x:%02x:%02x:%02x\n", 
		ethhdr->h_source[0], ethhdr->h_source[1], ethhdr->h_source[2],
		ethhdr->h_source[3], ethhdr->h_source[4], ethhdr->h_source[5]);
	printk(KERN_INFO "h_dest  %02x:%02x:%02x:%02x:%02x:%02x\n", 
		ethhdr->h_dest[0], ethhdr->h_dest[1], ethhdr->h_dest[2],
		ethhdr->h_dest[3], ethhdr->h_dest[4], ethhdr->h_dest[5]);
*/


	memcpy(h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, ethhdr->h_dest, ETH_ALEN);	
	memcpy(ethhdr->h_dest, h_dest, ETH_ALEN);

	len = (unsigned char *)((char *)proto - 4);	/* length of pppoe payload */
	len[0] = 0;		/* 2 + 4 + 4 */
	len[1] = 10;	/* 2 + 4 + 4 */
	proto->code = PPPoE_LCP_CFG_NAK;
	proto->length = htons(8);

	opt = (struct pppoe_option *)(proto+1);
	opt->type = LCP_OPT_MRU;
	opt->len = 4;
	opt->data[0] = (pppoe_snp_mru>>8) & 0x00ff;
	opt->data[1] = pppoe_snp_mru & 0x00ff;

	skb->data = (unsigned char *)eth_hdr(skb);
	skb->tail = &opt->data[2];
	skb->len = (unsigned int)skb->tail - (unsigned int)skb->data;

	return dev_queue_xmit(skb);
}


static int pppoe_lcp_change_mru(struct pppoe_proto *proto)
{		
	struct pppoe_option *opt = NULL;
	unsigned short mru = pppoe_snp_mru;

	if (!proto) {
		return -1;
	}

	if (mru < MRU_MIN) {
		mru = MRU_MIN;
	}
	if (mru > MRU_MAX) {
		mru = MRU_MAX;
	}

	opt = pppoe_get_option(proto, LCP_OPT_MRU);
	if (opt) {
		opt->data[0] = (mru>>8) & 0x00ff;
		opt->data[1] = mru & 0x00ff;
	} else {
		return -1;
	}
	
	return 0;
}


static void *pppoe_get_option(struct pppoe_proto *proto, __u8 opt_type)
{
	__u16 length = 0;
	__u16 offset = 0;
	__u8 type = 0;
	__u8 len = 0;
	unsigned char *opt_hdr = 0;

	if (!proto) {
		return NULL;
	}

	/* no options */
	if (proto->length <= sizeof(*proto)) {
		return NULL;
	}

	opt_hdr = (unsigned char *)(proto + 1);
	length = proto->length - sizeof(*proto);	/* unsigned */
	
	while (offset < length) {
		type = opt_hdr[offset];
		len = opt_hdr[offset+1];
		if (offset + len > length) {
			return NULL;
		}
		
		if (type == opt_type) {
			return opt_hdr + offset;
		} else {
			offset += len;
		}
	}

	return NULL;
}
