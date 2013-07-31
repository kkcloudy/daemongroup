/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* DBA.c
* DHCP Broadcast Agent DHCP(or Direct Broadcast Agent )
* 
* CREATOR:
* 
*
* DESCRIPTION:
* dhcp operation
*
********************************************************************************/


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
#include <linux/udp.h>

#include "log.h"
#include "main.h"
#include "pppoe_snooping.h"
#include "packet_snooping.h"
#include "dhcp_broadcast_agent.h"

static int dba_enlarge_skb(struct sk_buff *skb, unsigned int extra);
static void *dhcp_get_option(struct dhcp_packet *dhcp, __u8 op_code, unsigned char *tail);

extern int dba_option82_debug;

void printPacketBuffer(unsigned char *buffer,unsigned long buffLen)
{
	unsigned int i;

	if(!buffer)
		return;
	printk(":::::::::::::::::::::::::::::::::::::::::::::::\n");

	for(i = 0;i < buffLen ; i++)
	{
		printk("%02x ",buffer[i]);
		if(0==(i+1)%16) {
			printk("\n");
		}
	}
	if((buffLen%16)!=0)
	{
		printk("\n");
	}
	printk(":::::::::::::::::::::::::::::::::::::::::::::::\n");
}

int dhcp_broadcast_agent(struct sk_buff *skb, struct dhcp_packet *dhcp, dba_result_t *res)
{
	if (!skb || !dhcp || !res) {
		return -1;
	}

	if ((DHCP_SERVER_REPLY == dhcp->op)
		&& (res->module_type & DBA_KMOD)) {
		/* change broadcast mac to unicast mac (exp FF:FF:FF:FF:FF:FF -> 00:0d:56:e3:c6:70), 
		   and set change flag */
		memcpy(skb_mac_header(skb), dhcp->chaddr, ETH_ALEN);

		res->result |= DBA_HANDLED;
	}
	return 0;
}

int dhcp_option82_handle(struct sk_buff *skb, struct dhcp_packet *dhcp, dba_result_t *res)
{
	struct ethhdr *ethhdr = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	unsigned char *tail = NULL;
	
	if (unlikely(!skb || !dhcp || !res)) {
		return -1;
	}

	if ((DHCP_CLIENT_REQUEST == dhcp->op)
		&& (res->module_type & DHCP_OPTION82_KMOD)) {

		if (dba_option82_debug) {
			printk(KERN_DEBUG "skb->dev->name : %s\n", skb->dev->name);
			printPacketBuffer(skb->data, skb->len);
			printk(KERN_DEBUG "res->len : %d\n", res->len);
			printk(KERN_DEBUG "res->data : \n");
			printPacketBuffer((unsigned char *)res->data, res->len);
		}

		/* 253 = 2^8 - 2(option code len) */
		if (unlikely((res->len) > 255)) {
			log_error("dhcp option82 length %d: too large!\n", res->len);
			res->result |= DBA_ERROR;			
			return -1;
		}

		/* enlarge skb, pointer dhcp may change, so must recalculate */
		if (dba_enlarge_skb(skb, DBA_ALIGN4(res->len + 2))) {
			log_error("dhcp option82 expand skb failed!\n");
			res->result |= DBA_ERROR;
			return -1;
		}

		/* skb may change, so recalculate pointer */
		ethhdr = eth_hdr(skb);
		iph = (struct iphdr *)(ethhdr + 1);
		udph = (struct udphdr *)IPv4_NXT_HDR(iph);
		dhcp = (struct dhcp_packet *)(udph + 1);


		#if 0
		tail = skb_tail_pointer(skb);
		
		if (0xff == *((unsigned char *)(tail-1))) {
			/* append option82 */
			*(tail-1) = 82;	/* option 82 code */
			*tail = res->len;	/* option 82 length */
			
			memcpy(skb_put(skb, res->len+2)+1, res->data, res->len);

			tail = skb_tail_pointer(skb);
			*(tail-1) = 0xff; /* dhcp end option */

		} else {
			/* append option82 */
			*(tail) = 82;	/* option 82 code */
			*(tail+1) = res->len;	/* option 82 length */
			
			memcpy(skb_put(skb, res->len+2)+2, res->data, res->len);
			tail = skb_tail_pointer(skb);			
			*(tail-1) = 0xff; /* dhcp end option */
		}
		#else

		/* geti skb tail */
		tail = skb_tail_pointer(skb);
		/* get dhcp end option(0xff) */
		if (tail = dhcp_get_option(dhcp, 0xff, tail)) {
			/* append option82 */
			*(tail) = 82;	/* option 82 code */
			*(tail+1) = res->len;	/* option 82 length */

			skb_put(skb, res->len + 2);
			memcpy(tail + 2, res->data, res->len);
			tail = skb_tail_pointer(skb);			
			*(tail-1) = 0xff; /* dhcp end option */
		} else {
			res->result |= DBA_ERROR;
			log_error("dhcp option82 cannot find option 255!\n");			
			return -1;
		}
		#endif
		
		/* recalculate ip length Checksum*/
		/*
		ethhdr = eth_hdr(skb);
		iph = (struct iphdr *)(ethhdr + 1);
		udph = (struct udphdr *)IPv4_NXT_HDR(iph);
		*/
		
		/* ip header length */
		iph->tot_len += (res->len + 2);

		/* ip checksum */
		iph->check = 0;
		iph->check = ip_fast_csum(iph, iph->ihl);

		/* recalculate udp Checksum length */
		udph->len += (res->len + 2);
		udph->check = 0;
		udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, udph->len,
			IPPROTO_UDP, csum_partial(udph, udph->len, 0));

		
		res->result |= DBA_HANDLED;
	}
	return 0;
}

/* Unusual, but possible case. */
static int dba_enlarge_skb(struct sk_buff *skb, unsigned int extra)
{
	if (skb->len + extra > 65535) {
		log_error("dba enlarge skb failed: skb len %d + extra %d >65535.\n", skb->len, extra);		
		return -1;
	}

	if ((extra > skb_tailroom(skb)) || skb_cloned(skb)) {
		if (pskb_expand_head(skb, 0, extra - skb_tailroom(skb), GFP_ATOMIC)) {
			return -1;
		}
	}
	return 0;
}



static void *dhcp_get_option(struct dhcp_packet *dhcp, __u8 op_code, unsigned char *tail)
{
	__u8 length = 0;
	__u8 offset = 0;
	__u8 code = 0;
	__u8 len = 0;
	unsigned char *opt_hdr = 0;

	if (!dhcp || !tail) {
		log_error("get dhcp option failed: parameters is null\n");
		return NULL;
	}

	/* get dhcp option header (4 byte cookie)*/
	opt_hdr = &dhcp->options[0] + 4;
	
	while (opt_hdr + offset < tail) {
		code = opt_hdr[offset];

		if (0xff == code) {
			if (0xff == op_code) {			
				return opt_hdr + offset;
			} else {
				return NULL;
			}
		}		
		len = opt_hdr[offset+1];
		if (opt_hdr + offset + len + 2 > tail) {
			return NULL;
		}
		
		if (code == op_code) {
			return opt_hdr + offset;
		} else {
			offset += len+2;
		}
	}

	return NULL;
}

