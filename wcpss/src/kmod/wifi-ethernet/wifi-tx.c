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
* wifi-tx.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi-ethernet module transmit packets routine
*
*
*******************************************************************************/

#include <linux/stddef.h>
//#include <cvmx.h>
//#include <cvmx-app-init.h>
#include <../../../../kernel2.6.32.27cn/arch/mips/include/asm/octeon/cvmx-app-init.h>
#include <../../../../kernel2.6.32.27cn/arch/mips/include/asm/octeon/cvmx.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inetdevice.h>
#include <asm/unaligned.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/route.h>
#include <net/sock.h>
#include <net/route.h>
#include <linux/miscdevice.h>
#include <linux/in.h>
#include <linux/netfilter_ipv4.h>
#include <net/ipv6.h>
#include <net/protocol.h>
#include <net/ip6_route.h>
#include <net/xfrm.h>

#include "wifi-sm.h"
#include "wifi-tx.h"
#include "wifi.h"
#include "wifi-ioctl.h"
int tx_count = 0;
module_param(tx_count,int,0644);
int drop_count = 0;
module_param(drop_count,int,0644);

int wifi_tx_debug = 1;
module_param(wifi_tx_debug,int,0644);

int wifi_QoS_open = 0;
module_param(wifi_QoS_open,int,0644);
int wifi_dp = 56;
module_param(wifi_dp,int,0644);
int wifi_mm = 7; //fengwenchao change 0 to 7 20121213
module_param(wifi_mm,int,0644);
int wifi_mtu = 15000;
module_param(wifi_mtu,int,0644);
int wifi_tx_nonlinear = 0;
module_param(wifi_tx_nonlinear,int,0644);


extern cvmx_bootinfo_t *octeon_bootinfo;

extern core_num; /* core numbers this OS startup.*/
#define ROUND_TIMES 1000 /* check times for all the efficiency core buffer.*/

#define WIFI_HARD_TX_LOCK(dev, cpu) {			\
		spin_lock(&dev->xmit_lock);		\
		dev->xmit_lock_owner = cpu;		\
	}

#define WIFI_HARD_TX_UNLOCK(dev) {			\
		dev->xmit_lock_owner = -1;		\
		spin_unlock(&dev->xmit_lock);		\
	}

void skb_dump(char *name, struct sk_buff *skb);
 void hex_dump(const void *buffer, int buffer_len);
 extern struct net_device * wifi_device[VRID_MAX][WIFI_INTERFACE_NUM];

 /**
  * Display a buffer in hex
  *
  * @param buffer	  Data buffer to display
  * @param buffer_len Length of the buffer
  */
 void hex_dump(const void *buffer, int buffer_len)
 {
	 const uint8_t *ptr = (const uint8_t*)buffer;
	 int i;
	 printk("%p:\n",ptr);
	 for (i=0;i<buffer_len;i++) 
	 	{
//		 if (i % 16 == 0) printk("\n%04x: ", i);
		 if (i % 16 == 0) printk("\n");
		   printk("%02X ", (unsigned int)*ptr);
		   ptr++;
		}
	 printk("\n");
 }

void skb_dump(char *name, struct sk_buff *skb)
 {

   printk("#+++++++++++++++++++++++++++\n"
   "#%s SKB dump \nskb=%p, \nhead=%p,  \ndata=%p, \ntail=%p, \nend=%p,	\nlen=%d\n",
   name,		 skb,  skb->head,skb->data, skb->tail, skb->end, skb->len);

   printk(">> data:\n");
   printk("#>> data:\n");
	 hex_dump(skb->data, skb->len);
   printk("\n-------------------------\n");
   printk("\n#-------------------------\n");
 }

/**
 * Packet transmit
 *
 * @param skb    Packet to send
 * @param dev    Device info structure
 * @return Always returns zero
 */
 #if 0
int wifi_xmit(struct sk_buff *skb, struct net_device *dev)
{

	unsigned char *ptr_packet = NULL;
	unsigned int vrid = 0;
	int core_id_org = cvmx_get_core_num();
	unsigned short coremask = octeon_bootinfo->core_mask;
	int round = 0;
	/* at before , per core per queue.*/
	int core_id = ((core_id_org+1) % core_num);
	struct mmap_head *p_mmap_head;
	struct page_head *queue_head;
	struct page_head *page_head_tx;
	wifi_dev_private_t* priv = (wifi_dev_private_t*)dev->priv;

	vrid = priv->vrid;
	/*Each core correspond with a exclusive queue.*/
	p_mmap_head = (struct mmap_head *)vm[vrid];
	queue_head = (struct page_head *)(vm[vrid] + p_mmap_head->wifiTOwsm_array[core_id].offset);
	page_head_tx = (struct page_head *)((char *)queue_head + BUFF_SIZE * (p_mmap_head->wifiTOwsm_array[core_id].wifi_index));

	while(UNFINISHED != page_head_tx->finish)
		{

		/* Send the packet to the corresponding quene. if current queue if full, try the next one. 
			When all the queue is checked and all is full, return to the original queue ,drop it and return.
		*/

            /* jump to the next core queue. */
			core_id = ((core_id) % core_num);
			p_mmap_head = (struct mmap_head *)vm[vrid];
			queue_head = (struct page_head *)(vm[vrid] + p_mmap_head->wifiTOwsm_array[core_id].offset);
			page_head_tx = (struct page_head *)((char *)queue_head + BUFF_SIZE * (p_mmap_head->wifiTOwsm_array[core_id].wifi_index));


		
			if((round == ROUND_TIMES)||(core_num == 0x1)){
				kfree_skb(skb);
				priv->stats.tx_dropped++;
				drop_count++;
/*				printk( KERN_DEBUG "Dropped packet:%s-core_id = 0x%d\n",__FUNCTION__, core_id);*/
				return 0;
				}

				round++;
			
/*	printk(KERN_DEBUG "Waiting......coreid = %d, round = %d.\n",core_id, round);*/

		}

	/*Copy the packet from skb to corresponding queue.*/
	ptr_packet = (unsigned char *)page_head_tx + TX_OFFSET;
	page_head_tx->len = skb->len;
	page_head_tx->offset = TX_OFFSET;
	page_head_tx->wlanID = priv->wlanID;
	page_head_tx->BSSIndex= priv->BSSIndex;
	memcpy(ptr_packet, skb->data, skb->len);

	page_head_tx->finish = FINISHED;
	p_mmap_head->wifiTOwsm_array[core_id].wifi_index = (p_mmap_head->wifiTOwsm_array[core_id].wifi_index + 1) % ARRAY_LEN;
	priv->stats.tx_packets++;
	tx_count++;
	priv->stats.tx_bytes += skb->len;
	kfree_skb(skb);

	return 0;
}
#endif

int wifi_assemble_udp_ip(struct sk_buff *skb, wifi_dev_private_t* priv){

	if(NULL == priv){
		return WIFI_ERR;
	}
	int total_len, eth_len, ip_len, udp_len, len;
	struct udphdr *udph;
	struct iphdr *iph;
	struct ethhdr *eth;
	struct net_device *local = NULL ;	
	int local_ip = 0;
	int ret = 0;
	len = skb->len;
	udp_len = len + sizeof(*udph);
	ip_len = eth_len = udp_len + sizeof(*iph);
	total_len = eth_len + ETH_HLEN + NET_IP_ALIGN;
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s skb->len %d.\n",__func__,skb->len);
	local = dev_get_by_name(&init_net, priv->ifname); 
	if (!local) {
		printk(KERN_DEBUG "wifi error wifi_assemble_udp_ip get device %s failed\n",priv->ifname);
		return WIFI_ERR;
	}

	skb->dev = local;
/*
	if (skb->dev->ip_ptr) {

		if(((struct in_device *)(skb->dev->ip_ptr))->ifa_list) {

			local_ip = ((struct in_device *)(skb->dev->ip_ptr))->ifa_list->ifa_local;
		}

	}
*/

	if(priv->acip != 0)
	{
		local_ip  = priv->acip;
	}
	else if(skb->dev->ip_ptr)
	{
		if(((struct in_device *)(skb->dev->ip_ptr))->ifa_list) 
		{

			local_ip = ((struct in_device *)(skb->dev->ip_ptr))->ifa_list->ifa_local;
		}
	}else{
		printk(KERN_DEBUG "wifi error wifi_assemble_udp_ip get ap %02X:%02X:%02X:%02X:%02X:%02X ip failed\n",priv->apmac[0],priv->apmac[1],priv->apmac[2],priv->apmac[3],priv->apmac[4],priv->apmac[5]);
		return WIFI_ERR;
	}

//skb->h.uh = udph = (struct udphdr *) skb_push(skb, sizeof(*udph));
//	udph = udp_hdr(skb);
	udph = (struct udphdr *) skb_push(skb, sizeof(*udph));

	udph->source = htons(priv->acport);
	udph->dest = htons(priv->apport);
	udph->len = htons(udp_len);
	udph->check = 0;
	udph->check = csum_tcpudp_magic(htonl(local_ip),
					htonl(priv->apip),
					udp_len,  IPPROTO_UDP,
					csum_partial((unsigned char *)udph, udp_len, 0));
	if (udph->check == 0)
		udph->check = 0;

//skb->nh.iph = iph = (struct iphdr *)skb_push(skb, sizeof(*iph));
//	iph = ip_hdr(skb);
	iph = (struct iphdr *)skb_push(skb, sizeof(*iph));
	/* iph->version = 4; iph->ihl = 5; */
	put_unaligned(0x45, (unsigned char *)iph);
	struct ip_header_t *pTmpIPHead = (struct ip_header_t*)iph;
	if(wifi_QoS_open)
	{
		pTmpIPHead->dscp = wifi_dp; //change value by zdx 2010-08-27
		pTmpIPHead->ect = 0;
		pTmpIPHead->ecnce = 0;
	}
	else
	{
		iph->tos      = 0; //change value by zdx 2010-08-27
	}
	put_unaligned(htons(ip_len), &(iph->tot_len));
	//static unsigned id = 0xfffe;
	iph->id       = 0 ;//htons(id++);
	iph->frag_off = 0;// htons(0x4000);
	iph->ttl      = 64;
	iph->protocol = IPPROTO_UDP;
	iph->check    = 0;
	put_unaligned(htonl(local_ip), &(iph->saddr));
	put_unaligned(htonl(priv->apip), &(iph->daddr));
	iph->check    = ip_fast_csum((unsigned char *)iph, iph->ihl);	
	skb_reset_network_header(skb);
//skb->nh.iph = iph;
	#if 0
	skb->pkt_type = PACKET_HOST;
	eth = (struct ethhdr *) skb_push(skb, ETH_HLEN);
	skb->mac.raw = skb->data;
	skb->protocol = eth->h_proto = htons(ETH_P_IP);
	memcpy(eth->h_source, local->dev_addr, 6);
	memcpy(eth->h_dest, priv->apmac, 6);	
	skb->mac_len = ETH_HLEN;
	#endif
//skb->cvm_info.reserved1 = 1;

#if 1
	{
		struct rtable *rt = NULL;
		int err;
		if (rt == NULL) {
			#if 0
			struct flowi fl = { .oif = local->ifindex,
						.mark = 0,
					    .nl_u = { .ip4_u =
						      { .daddr = iph->daddr,
							.saddr = iph->saddr,
							.tos = 0 } },
					    .proto = skb->protocol,
						.flags = 0,
					    .uli_u = { .ports =
						       { .sport = udph->source,
							 .dport = udph->dest } } };
			#endif
			struct flowi fl = {
				.nl_u = {
					.ip4_u = {
						 .daddr = iph->daddr,
						 .saddr = 0,
						 .tos = RT_TOS(iph->tos) },
				},
				.proto = 0,
			};
			err = ip_route_output_key(dev_net(skb->dev), &rt, &fl);
			if (err){						
				if(wifi_eth_debug >= WIFI_WARNING)
					printk(KERN_ERR "wifi error ip_route_output_key failed,sip %d.%d.%d.%d dip %d.%d.%d.%d local device:%s\n", \
								(local_ip >> 24) & 0xFF, (local_ip >> 16) & 0xFF,(local_ip >> 8) & 0xFF, local_ip & 0xFF, \
								(priv->apip>> 24) & 0xFF, (priv->apip>> 16) & 0xFF, (priv->apip>> 8) & 0xFF, priv->apip& 0xFF , \
								local->name);
				dev_put(local);/*relese dev refcnt.*/
				return WIFI_ERR;
			}else{
				if (((struct dst_entry *)rt)->dev == skb->dev) {
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s %d,rt->dev == skb->dev.\n",__func__,__LINE__);
					if(skb_dst(skb)){
						dst_release(skb_dst(skb));
						skb->_skb_dst = NULL;		
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s,%d,release skb dst before set dst.\n",__func__,__LINE__);
					}
					skb_dst_set(skb, (struct dst_entry *)rt);
				}else{
					if(wifi_eth_debug >= WIFI_WARNING)
						printk("%s %d,rt->dev != skb->dev.\n",__func__,__LINE__);					
					dev_put(local);/*relese dev refcnt.*/
					return WIFI_ERR;
				}
			}
			
			//ip_rt_put(rt);
		#if 0	
			/* Release older dst refcnt. */
			if(skb->dst)
			{
				dst_release(skb->dst);
			}
			/* Set new dst to skb*/
			skb->dst = (struct dst_entry*)rt;
			
			//skb->dst = skb_dst(skb);
			
		/*	skb->dst = dst_clone(&rt->u.dst);*/
		#endif 
		}
	}

#endif
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("assemble UDP:skb->len = %d,dev->mtu = %d,dev->name=%s.\n",skb->len,skb->dev->mtu,skb->dev->name);
#if 0
	if(skb->dst != NULL)
	{
		//printk("assemble USP:skb->len = %d,dev->mtu = %d.\n",skb->len,skb->dev->mtu);
		priv->stats.tx_packets++;
		tx_count++;
		priv->stats.tx_bytes += skb->len;

		//int cpu = smp_processor_id();
		//WIFI_HARD_TX_LOCK(skb->dst->dev,cpu);
		
#ifdef CONFIG_BRIDGE_NETFILTER
				/* The next two lines are done in nf_reset() for 2.6.21. 2.6.16 needs
					them. I'm leaving it for all versions since the compiler will optimize
					them away when they aren't needed. It can tell that skb->nf_bridge
					was set to NULL in the inlined nf_reset(). */
				nf_bridge_put(skb->nf_bridge);
				skb->nf_bridge = NULL;
#endif /* CONFIG_BRIDGE_NETFILTER */
		
		skb->dst->output(skb);
		//WIFI_HARD_TX_UNLOCK(skb->dst->dev);
	}
	else
	{
		dev_put(local);
		printk(KERN_ERR "wifi error skb->dst is NULL. sip %d.%d.%d.%d dip %d.%d.%d.%d local device:%s\n", \
						(local_ip >> 24) & 0xFF, (local_ip >> 16) & 0xFF,(local_ip >> 8) & 0xFF, local_ip & 0xFF, \
						(priv->apip>> 24) & 0xFF, (priv->apip>> 16) & 0xFF, (priv->apip>> 8) & 0xFF, priv->apip& 0xFF , \
						local->name);
		return WIFI_ERR;
	}
#endif
//local->hard_start_xmit(skb,local);	
#if 1
	if(skb->_skb_dst){
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("1%s %d.\n",__func__,__LINE__);
	nf_reset(skb);

		skb_dst(skb)->output(skb);
	}else{
		dev_put(local);
		return WIFI_ERR;
	}
	#endif
	dev_put(local);
	return WIFI_OK;
}


int wifi_assemble_udp_ipv6(struct sk_buff *skb, wifi_dev_private_t* priv){

	if(NULL == priv){
		return WIFI_ERR;
	}
	int total_len, eth_len, ip_len, udp_len, len;
	struct udphdr *udph;
	struct ipv6hdr *iph;
	struct ethhdr *eth;
	struct net_device *local = NULL ;	
	struct net *net = NULL;
	struct dst_entry *dst = NULL;
	int ret = 0;	
	struct flowi fl;
	len = skb->len;
	udp_len = len + sizeof(*udph);
	ip_len = eth_len = udp_len + sizeof(*iph);
	total_len = eth_len + ETH_HLEN + NET_IP_ALIGN;
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s skb->len %d.\n",__func__,skb->len);
	local = dev_get_by_name(&init_net, priv->ifname); 
	if (!local) {
		printk(KERN_DEBUG "wifi error wifi_assemble_udp_ipv6 get device %s failed\n",priv->ifname);
		return WIFI_ERR;
	}

	skb->dev = local;
	net = dev_net(skb->dev);
	udph = (struct udphdr *) skb_push(skb, sizeof(*udph));

	udph->source = htons(priv->acport);
	udph->dest = htons(priv->apport);
	udph->len = htons(udp_len);
	udph->check = 0;
	
	/*skb->csum = skb_checksum(skb, 0,
				 skb->len , 0);
	udph->check = csum_ipv6_magic(priv->acipv6,
						  priv->apipv6,
						  udp_len, IPPROTO_UDP,
						  skb->csum);*/
	udph->check = csum_ipv6_magic(priv->acipv6,
						  priv->apipv6,
						  udp_len, IPPROTO_UDP,
						  csum_partial((unsigned char *)udph,
							   udp_len, 0));
	skb_reset_transport_header(skb);
	memset(&fl, 0, sizeof(fl));
	fl.proto = IPPROTO_UDP;
	ipv6_addr_copy(&fl.fl6_src, &priv->acipv6);
	ipv6_addr_copy(&fl.fl6_dst, &priv->apipv6);
	fl.fl_ip_sport = udph->source;
	fl.fl_ip_dport = udph->dest;

	dst = ip6_route_output(net, NULL, &fl);
	if (dst == NULL || dst->error) {
		if(NULL != dst){
			dst_release(dst);
		}
		printk("%s dst == NULL.\n",__func__);
		dev_put(local);
		return WIFI_ERR;
	}
	if (xfrm_lookup(net, &dst, &fl, NULL, 0)){
		printk("%s xfrm_lookup failed.\n",__func__);
		dev_put(local);
		return WIFI_ERR;
	}
	skb_dst_set(skb, dst);

	iph = (struct ipv6hdr *)skb_push(skb, sizeof(*iph));

	iph->version = 6;
	iph->flow_lbl[0] = 0;
	iph->flow_lbl[1] = 0;
	iph->flow_lbl[2] = 0;	
	iph->hop_limit = 64;//dst_metric(dst, RTAX_HOPLIMIT);
	iph->payload_len = htons(skb->len - sizeof(*iph));
	iph->nexthdr = IPPROTO_UDP;
	ipv6_addr_copy(&iph->saddr, &priv->acipv6);
	ipv6_addr_copy(&iph->daddr, &priv->apipv6);
	skb_reset_network_header(skb);	

	if(skb->_skb_dst){
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("1%s %d.\n",__func__,__LINE__);
#ifdef CONFIG_BRIDGE_NETFILTER
								/* The next two lines are done in nf_reset() for 2.6.21. 2.6.16 needs
									them. I'm leaving it for all versions since the compiler will optimize
									them away when they aren't needed. It can tell that skb->nf_bridge
									was set to NULL in the inlined nf_reset(). */
								nf_bridge_put(skb->nf_bridge);
								skb->nf_bridge = NULL;
#endif /* CONFIG_BRIDGE_NETFILTER */
		skb_dst(skb)->output(skb);
	}
	dev_put(local);
	return WIFI_OK;
}


int wifi_assemble_capwap(struct sk_buff *skb, wifi_dev_private_t* priv,unsigned char dataflag){
	unsigned int val = 0;
	unsigned int Radio_L_ID = (priv->BSSIndex/32)%4;
	if((priv->f802_3 == 1)&&(0 != dataflag)){/*dataflag :data --1,mgmt--0*/
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("%s %d,priv->f802_3=%#X.\n",__func__,__LINE__,priv->f802_3);
		}
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("%s %d,priv->f802_3=%#X.\n",__func__,__LINE__,priv->f802_3);
		}
		struct capwap_head *head = NULL;
		skb_push(skb, 16);
		head = (struct capwap_head *)skb->data;
		val = 0;
		CWSetField32(val, 
				 CW_TRANSPORT_HEADER_VERSION_START,
				 CW_TRANSPORT_HEADER_VERSION_LEN,
				 CW_PROTOCOL_VERSION); /* CAPWAP VERSION */

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_TYPE_START,
				 CW_TRANSPORT_HEADER_TYPE_LEN,
				 0);
		
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_HLEN_START,
				 CW_TRANSPORT_HEADER_HLEN_LEN,
				 4);

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_RID_START,
				 CW_TRANSPORT_HEADER_RID_LEN,
				 Radio_L_ID); /* Radio local id */
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_WBID_START,
				 CW_TRANSPORT_HEADER_WBID_LEN,
				 1); /* Wireless Binding ID */
		
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_T_START,
				 CW_TRANSPORT_HEADER_T_LEN,
				 0/*1*/);

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_F_START,
				 CW_TRANSPORT_HEADER_F_LEN,
				 0); /* is fragment */

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_L_START,
				 CW_TRANSPORT_HEADER_L_LEN,
				 0); /* last fragment */
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_W_START,
				 CW_TRANSPORT_HEADER_W_LEN,
				 1); /* have wireless option header */
		
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_M_START,
				 CW_TRANSPORT_HEADER_M_LEN,
				 0); /* no radio MAC address */

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_K_START,
				 CW_TRANSPORT_HEADER_K_LEN,
				 0); /* Keep alive flag */

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_FLAGS_START,
				 CW_TRANSPORT_HEADER_FLAGS_LEN,
				 0); /* required */

		val = htonl(val);
		head->capwap1 = val;
		val = 0;
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
				 CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
				 0); /* fragment ID */
		
		CWSetField32(val,
				 CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
				 CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
				 0); /* fragment offset */

		CWSetField32(val,
				 CW_TRANSPORT_HEADER_RESERVED_START,
				 CW_TRANSPORT_HEADER_RESERVED_LEN,
				 0); /* required */
				 
		val = htonl(val);		 
		head->capwap2 = val;
		val = 0;
		CWSetField32(val, 0, 8, priv->WLANID);
		val = htonl(val);
		head->capwap3 = val;
		val = 0;
		val = htonl(val);
		head->capwap4 = val;
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("%s,%d,bssid:%02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,__LINE__,priv->bssid[0],priv->bssid[1],priv->bssid[2],priv->bssid[3],priv->bssid[4],priv->bssid[5]);
			printk("%s,%d,head->capwap1=%d,head->capwap2=%d,head->capwap3=%d,head->capwap4=%d,priv->WLANID=%d.(priv->wlanID=%d.)\n",__func__,__LINE__,head->capwap1,head->capwap2,head->capwap3,head->capwap4,priv->WLANID,priv->wlanID);
		}
	}else{
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("%s %d,priv->f802_3=%#X.\n",__func__,__LINE__,priv->f802_3);
		}
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("%s %d,priv->f802_3=%#X.\n",__func__,__LINE__,priv->f802_3);
		}
		struct capwap_head *head = NULL;
		skb_push(skb, 16);
	//	memset(skb->data,0,16);
		head = (struct capwap_head *)skb->data;
		CWSetField32(val, 
			     CW_TRANSPORT_HEADER_VERSION_START,
			     CW_TRANSPORT_HEADER_VERSION_LEN,
			     CW_PROTOCOL_VERSION); /* CAPWAP VERSION */

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_TYPE_START,
			     CW_TRANSPORT_HEADER_TYPE_LEN,
			     0);
		
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_HLEN_START,
			     CW_TRANSPORT_HEADER_HLEN_LEN,
			     4/*12*/);

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_RID_START,
			     CW_TRANSPORT_HEADER_RID_LEN,
			     Radio_L_ID); /* Radio local id */
		
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_WBID_START,
			     CW_TRANSPORT_HEADER_WBID_LEN,
			     1); /* Wireless Binding ID */
		
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_T_START,
			     CW_TRANSPORT_HEADER_T_LEN,
			     1);

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_F_START,
			     CW_TRANSPORT_HEADER_F_LEN,
			     0); /* is fragment */

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_L_START,
			     CW_TRANSPORT_HEADER_L_LEN,
			     0); /* last fragment */
		
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_W_START,
			     CW_TRANSPORT_HEADER_W_LEN,
			     1); /* have wireless option header */
		

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_M_START,
			     CW_TRANSPORT_HEADER_M_LEN,
			     0); /* no radio MAC address */

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_K_START,
			     CW_TRANSPORT_HEADER_K_LEN,
			     0); /* Keep alive flag */

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_FLAGS_START,
			     CW_TRANSPORT_HEADER_FLAGS_LEN,
			     0); /* required */

		val = htonl(val);
		head->capwap1 = val;
		val = 0;
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
			     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
			     0); /* fragment ID */
		
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
			     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
			     0); /* fragment offset */

		CWSetField32(val,
			     CW_TRANSPORT_HEADER_RESERVED_START,
			     CW_TRANSPORT_HEADER_RESERVED_LEN,
			     0); /* required */
			     
		val = htonl(val);	     
		head->capwap2 = val;
		val = 0;
		CWSetField32(val, 0, 8, priv->WLANID);
		val = htonl(val);
		head->capwap3 = val;
		val = 0;
		val = htonl(val);
		head->capwap4 = val;
	}
	return 0;

}

#if 0
int wifi_803_3_to_802_11(struct sk_buff *skb, wifi_dev_private_t* priv){
	unsigned char IEEE8023_destMac[MAC_LEN];
	unsigned char IEEE8023_srcMac[MAC_LEN];
	unsigned char IEEE80211_addr1[MAC_LEN];
	unsigned char IEEE80211_addr2[MAC_LEN];
	unsigned char IEEE80211_addr3[MAC_LEN];
	
	unsigned char IEEE8023_type[2];
	struct ieee80211_frame *macHead;
	struct ieee80211_llc *llcHead;
	unsigned char  fcField[2];
	unsigned char *buff;
	unsigned int macHeadLen = 24;
	unsigned short seqNum = priv->seqNum;
	priv->seqNum++;
	if(priv->seqNum == 4096)
		priv->seqNum = 0;

	buff = skb->data;
	IEEE80211_RETRIEVE(IEEE8023_destMac, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
	IEEE80211_RETRIEVE(IEEE8023_srcMac, buff , IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
	skb_pull(skb, 12);
	skb_push(skb, 6);
	llcHead = (struct ieee80211_llc*)(skb->data);
	skb_push(skb, 24);
	macHead = (struct ieee80211_frame*)(skb->data);
	memset(skb->data, 0, 30);
	/* WEP type */
	if (priv->protect_type)
	{
		macHead->i_fc[0] = 0x08;
		macHead->i_fc[1] = 0x42;
	}
	else
	{
		macHead->i_fc[0] = 0x08;
		macHead->i_fc[1] = 0x02;
	}

	macHead->i_dur[0] = 0;
	macHead->i_dur[1] = 0;
	
	memcpy(macHead->i_addr1, IEEE8023_destMac, MAC_LEN);
	memcpy(macHead->i_addr3, IEEE8023_srcMac, MAC_LEN);
	memcpy(macHead->i_addr2, priv->bssid, MAC_LEN);
	macHead->i_seq[0] = (seqNum<<4)&0xf0;
	macHead->i_seq[1] = (seqNum>>4)&0xff;

	llcHead->llc_dsap = 0xaa;
	llcHead->llc_ssap = 0xaa;
	llcHead->llc_cmd = 0x03;
	llcHead->llc_org_code[0] = 0;
	llcHead->llc_org_code[1] = 0;
	llcHead->llc_org_code[2] = 0;
	
	return 0;
}
#endif
int wifi_803_3_to_802_11(struct sk_buff *skb, wifi_dev_private_t* priv){
	unsigned char IEEE8023_destMac[MAC_LEN];
	unsigned char IEEE8023_srcMac[MAC_LEN];
	unsigned char IEEE80211_addr1[MAC_LEN];
	unsigned char IEEE80211_addr2[MAC_LEN];
	unsigned char IEEE80211_addr3[MAC_LEN];
	
	unsigned char IEEE8023_type[2];

	struct ieee80211_qosframe *macHeadQos=NULL;//2010-08-23 add by zdx
	struct ieee80211_frame *macHead=NULL;
	struct ieee80211_llc *llcHead=NULL;
	unsigned char  fcField[2];
	unsigned char *buff=NULL;
	unsigned int macHeadLen = 24;
	unsigned short seqNum = priv->seqNum;
	
	unsigned char chQosFlag = wifi_QoS_open; //2010-08-23 add by zdx
	unsigned char chQosValue = 7;//2010-08-23 add by zdx
	
	unsigned short *isEAP=NULL;
	priv->seqNum++;
	if(priv->seqNum == 4096)
		priv->seqNum = 0;

	buff = skb->data;
	skb_pull(skb, 12);
	skb_push(skb, 6);
	llcHead = (struct ieee80211_llc*)(skb->data);
	isEAP = llcHead->llc_ether_type;
	if((*isEAP == 0x8100)||(*isEAP == 0x88a8)||(*isEAP == 0x9100)){
		skb_pull(skb, 4);
		llcHead = (struct ieee80211_llc*)(skb->data);
		isEAP = llcHead->llc_ether_type;
	}
	if(chQosFlag == 1)
	{
		skb_push(skb, 28);
		macHeadQos = (struct ieee80211_qosframe*)(skb->data);
		if( wifi_tx_debug == 2 )
		{
			skb_dump("802.11 dump in chQosFlag tolking",skb);	
		}
		
	}
	else
	{
		skb_push(skb, 24);		
		macHead = (struct ieee80211_frame*)(skb->data);
	}

	if(chQosFlag == 1)
	{		
		IEEE80211_RETRIEVE(macHeadQos->i_addr1, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHeadQos->i_addr3, buff, IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
        if(wifi_tx_debug >= WIFI_DEBUG)
        {
        		printk("macHeadQos->i_addr1:%02x:%02x:%02x:%02x:%02x:%02x\n",\
        			macHeadQos->i_addr1[0],macHeadQos->i_addr1[1],macHeadQos->i_addr1[2],
        			macHeadQos->i_addr1[3],macHeadQos->i_addr1[4],macHeadQos->i_addr1[5]);

        				printk("macHeadQos->i_addr3:%02x:%02x:%02x:%02x:%02x:%02x\n",\
        			macHeadQos->i_addr3[0],macHeadQos->i_addr3[1],macHeadQos->i_addr3[2],
        			macHeadQos->i_addr3[3],macHeadQos->i_addr3[4],macHeadQos->i_addr3[5]);
        }
	}
	else
	{		
		/* WEP type */
		IEEE80211_RETRIEVE(macHead->i_addr1, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHead->i_addr3, buff, IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
	}
	
	if (priv->protect_type)
	{
		if(chQosFlag == 1)
		{
			macHeadQos->i_fc[0] = 0x88;
			macHeadQos->i_fc[1] = 0x42;
		}
		else
		{
			macHead->i_fc[0] = 0x08;
			macHead->i_fc[1] = 0x42;
		}
	}
	else
	{
		if(chQosFlag == 1)
		{
			macHeadQos->i_fc[0] = 0x88;
			macHeadQos->i_fc[1] = 0x02;
		}
		else
		{
			macHead->i_fc[0] = 0x08;
			macHead->i_fc[1] = 0x02;
		}
	}

	if(chQosFlag == 1)
	{
		macHeadQos->i_dur[0] = 0;
		macHeadQos->i_dur[1] = 0;
		
		memcpy(macHeadQos->i_addr2, priv->bssid, MAC_LEN);
		macHeadQos->i_seq[0] = (seqNum<<4)&0xf0;
		macHeadQos->i_seq[1] = (seqNum>>4)&0xff;
		macHeadQos->i_qos[0] = wifi_mm;
		macHeadQos->i_qos[1] = 0x00;
		macHeadQos->i_align[0] = 0x00;
		macHeadQos->i_align[1] = 0x00;

		//the last two bytes have to manage
		/*char *pchNull = NULL;
		pchNull = macHeadQos->i_qos + 2;
		
		*pchNull = 0x00;
		pchNull += 1;
		*pchNull = 0x00;*/
		
	}
	else
	{
		macHead->i_dur[0] = 0;
		macHead->i_dur[1] = 0;
	
		memcpy(macHead->i_addr2, priv->bssid, MAC_LEN);
		macHead->i_seq[0] = (seqNum<<4)&0xf0;
		macHead->i_seq[1] = (seqNum>>4)&0xff;
	}
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s,%d,*isEAP=0x%X.\n",__func__,__LINE__,*isEAP);
	if((*isEAP == 0x888E)&&(priv->Eap1XServerSwitch == 1)){
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("%s,%d\n",__func__,__LINE__);
		if(chQosFlag == 1){
			memcpy(macHeadQos->i_addr3, priv->bssid, MAC_LEN);
		}else{
			memcpy(macHead->i_addr3, priv->bssid, MAC_LEN);
		}
	}
	llcHead->llc_dsap = 0xaa;
	llcHead->llc_ssap = 0xaa;
	llcHead->llc_cmd = 0x03;
	llcHead->llc_org_code[0] = 0;
	llcHead->llc_org_code[1] = 0;
	llcHead->llc_org_code[2] = 0;
	
	return 0;
}

int wifi_803_3_to_802_11_f(struct sk_buff *skb,struct sk_buff *skb1, wifi_dev_private_t* priv){
	unsigned char IEEE8023_destMac[MAC_LEN];
	unsigned char IEEE8023_srcMac[MAC_LEN];
	unsigned char IEEE80211_addr1[MAC_LEN];
	unsigned char IEEE80211_addr2[MAC_LEN];
	unsigned char IEEE80211_addr3[MAC_LEN];
	
	unsigned char IEEE8023_type[2];

	struct ieee80211_qosframe *macHeadQos=NULL;//2010-08-23 add by zdx
	struct ieee80211_frame *macHead=NULL;
	struct ieee80211_llc *llcHead=NULL;
	struct ieee80211_qosframe *macHeadQos1=NULL;
	struct ieee80211_frame *macHead1=NULL;
	unsigned char  fcField[2];
	unsigned char *buff=NULL;
	unsigned int macHeadLen = 24;
	unsigned short seqNum = priv->seqNum;
	
	unsigned char chQosFlag = wifi_QoS_open; //2010-08-23 add by zdx
	unsigned char chQosValue = 7;//2010-08-23 add by zdx
	priv->seqNum++;
	if(priv->seqNum == 4096)
		priv->seqNum = 0;

	buff = skb->data;
	skb_pull(skb, 12);
	skb_push(skb, 6);
	llcHead = (struct ieee80211_llc*)(skb->data);
	if(chQosFlag == 1)
	{
		skb_push(skb, 28);
		macHeadQos = (struct ieee80211_qosframe*)(skb->data);
		skb_push(skb1, 28);
		macHeadQos1 = (struct ieee80211_qosframe*)(skb1->data);
		if( wifi_tx_debug == 2 )
		{
			skb_dump("802.11 dump in chQosFlag tolking",skb);	
		}
		
	}
	else
	{
		skb_push(skb, 24);		
		macHead = (struct ieee80211_frame*)(skb->data);
		skb_push(skb1, 24);		
		macHead1 = (struct ieee80211_frame*)(skb1->data);
	}

	if(chQosFlag == 1)
	{		
		IEEE80211_RETRIEVE(macHeadQos->i_addr1, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHeadQos->i_addr3, buff, IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHeadQos1->i_addr1, macHeadQos->i_addr1, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHeadQos1->i_addr3, macHeadQos->i_addr3, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
        if(wifi_tx_debug >= WIFI_DEBUG)
        {
        		printk("macHeadQos->i_addr1:%02x:%02x:%02x:%02x:%02x:%02x\n",\
        			macHeadQos->i_addr1[0],macHeadQos->i_addr1[1],macHeadQos->i_addr1[2],
        			macHeadQos->i_addr1[3],macHeadQos->i_addr1[4],macHeadQos->i_addr1[5]);

        				printk("macHeadQos->i_addr3:%02x:%02x:%02x:%02x:%02x:%02x\n",\
        			macHeadQos->i_addr3[0],macHeadQos->i_addr3[1],macHeadQos->i_addr3[2],
        			macHeadQos->i_addr3[3],macHeadQos->i_addr3[4],macHeadQos->i_addr3[5]);
        }
	}
	else
	{		
		/* WEP type */
		IEEE80211_RETRIEVE(macHead->i_addr1, buff, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHead->i_addr3, buff, IEEE8023_SRC_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHead1->i_addr1, macHead->i_addr1, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
		IEEE80211_RETRIEVE(macHead1->i_addr3, macHead->i_addr3, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
	}
	
	if (priv->protect_type)
	{
		if(chQosFlag == 1)
		{
			macHeadQos->i_fc[0] = 0x88;
			macHeadQos->i_fc[1] = 0x46;
			macHeadQos1->i_fc[0] = 0x88;
			macHeadQos1->i_fc[1] = 0x42;
		}
		else
		{
			macHead->i_fc[0] = 0x08;
			macHead->i_fc[1] = 0x46;
			macHead1->i_fc[0] = 0x08;
			macHead1->i_fc[1] = 0x42;
		}
	}
	else
	{
		if(chQosFlag == 1)
		{
			macHeadQos->i_fc[0] = 0x88;
			macHeadQos->i_fc[1] = 0x06;
			macHeadQos1->i_fc[0] = 0x88;
			macHeadQos1->i_fc[1] = 0x02;
		}
		else
		{
			macHead->i_fc[0] = 0x08;
			macHead->i_fc[1] = 0x06;
			macHead1->i_fc[0] = 0x08;
			macHead1->i_fc[1] = 0x02;
		}
	}

	if(chQosFlag == 1)
	{
		macHeadQos->i_dur[0] = 0;
		macHeadQos->i_dur[1] = 0;
		
		memcpy(macHeadQos->i_addr2, priv->bssid, MAC_LEN);
		macHeadQos->i_seq[0] = (seqNum<<4)&0xf0;
		macHeadQos->i_seq[1] = (seqNum>>4)&0xff;
		macHeadQos->i_qos[0] = wifi_mm;
		macHeadQos->i_qos[1] = 0x00;
		macHeadQos->i_align[0] = 0x00;
		macHeadQos->i_align[1] = 0x00;

		macHeadQos1->i_dur[0] = 0;
		macHeadQos1->i_dur[1] = 0;		
		memcpy(macHeadQos1->i_addr2, priv->bssid, MAC_LEN);
		macHeadQos1->i_seq[0] = (seqNum<<4)&0xf0;
		macHeadQos1->i_seq[1] = (seqNum>>4)&0xff;
		macHeadQos1->i_seq[0] |= 0x01;	
		macHeadQos1->i_qos[0] = wifi_mm;
		macHeadQos1->i_qos[1] = 0x00;
		macHeadQos1->i_align[0] = 0x00;
		macHeadQos1->i_align[1] = 0x00;

		//the last two bytes have to manage
		/*char *pchNull = NULL;
		pchNull = macHeadQos->i_qos + 2;
		
		*pchNull = 0x00;
		pchNull += 1;
		*pchNull = 0x00;
		pchNull = macHeadQos1->i_qos + 2;
		
		*pchNull = 0x00;
		pchNull += 1;
		*pchNull = 0x00;*/
		
	}
	else
	{
		macHead->i_dur[0] = 0;
		macHead->i_dur[1] = 0;
	
		memcpy(macHead->i_addr2, priv->bssid, MAC_LEN);
		macHead->i_seq[0] = (seqNum<<4)&0xf0;
		macHead->i_seq[1] = (seqNum>>4)&0xff;		
		
		macHead1->i_dur[0] = 0;
		macHead1->i_dur[1] = 0;
	
		memcpy(macHead1->i_addr2, priv->bssid, MAC_LEN);
		macHead1->i_seq[0] = (seqNum<<4)&0xf0;
		macHead1->i_seq[1] = (seqNum>>4)&0xff;		
		macHead1->i_seq[0] |= 0x01;	
	}

	llcHead->llc_dsap = 0xaa;
	llcHead->llc_ssap = 0xaa;
	llcHead->llc_cmd = 0x03;
	llcHead->llc_org_code[0] = 0;
	llcHead->llc_org_code[1] = 0;
	llcHead->llc_org_code[2] = 0;
	
	return 0;
}



int wifi_xmit(struct sk_buff *skb, struct net_device *dev)
{
	unsigned char *ptr_packet = NULL;
	unsigned int vrid = 0;
	int core_id_org = cvmx_get_core_num();
	//unsigned int coremask = octeon_bootinfo->core_mask;
	int round = 0;
	struct sk_buff *skb2 = NULL;	
	struct sk_buff *skb3 = NULL;
	/* at before , per core per queue.*/
	int core_id = ((core_id_org+1) % core_num);
	struct mmap_head *p_mmap_head;
	struct page_head *queue_head;
	struct page_head *page_head_tx;
	char stamac[MAC_LEN] ;
	struct wifi_bss_tbl *bss = NULL;
	struct wifi_sta_tbl *sta = NULL;
	unsigned int dev_index;
	memset(stamac,0,MAC_LEN);	
	if(skb == NULL || dev == NULL){
		if(skb != NULL)
			kfree_skb(skb);
		return 0;
	}
	if(skb_is_nonlinear(skb)){
		wifi_tx_nonlinear += 1;
		if (skb_linearize(skb) != 0) {/*for data_len != 0 skb process*/		
			kfree_skb(skb);
			printk("wifi_xmit skb_linearize failed\n");
			return 0;
	    }
	}
	IEEE80211_RETRIEVE(stamac, skb->data, IEEE8023_DEST_MAC_START, IEEE8023_MAC_LEN);
	sta = wifi_sta_tbl_get(stamac);
	if(sta != NULL) 
	{	
		bss = wifi_bssid_bssidx_tbl_get(sta->BSSID);
		if(bss !=NULL)
		{
			vrid = bss->vrid;
			dev_index = bss->dev_index;
			if(dev_index >= WIFI_INTERFACE_NUM || vrid >= VRID_MAX){
				printk(KERN_ERR "Vrid %#x or dev_index %#x out of range.\n", vrid, dev_index );
				if(skb){
					kfree_skb(skb);
				}
				return 0;
			}
			if(wifi_device[vrid][dev_index] != NULL)
				dev = wifi_device[vrid][dev_index];
		}	
	}
	wifi_dev_private_t* priv = (wifi_dev_private_t *)netdev_priv(dev);
	if(priv == NULL || skb == NULL){
		printk("wifi_xmit,invalid priv=%p or skb=%p.\n",priv,skb);
		if(skb){
			kfree_skb(skb);
		}
		return 0;
	}
	vrid = priv->vrid;
	if((vrid >= VRID_MAX)/*||(vrid < 0)*/){
		printk("wifi_xmit,invalid vrid=%d.\n",vrid);
		kfree_skb(skb);
		return 0;
	}
	if(hansiInfo[vrid].hstate == BACKUP_STATE){
		//printk("wifi_xmit,hansiInfo[%d]->hstate is %d.free skb.\n",vrid,hansiInfo[vrid].hstate);
		kfree_skb(skb);
		return 0;
	}
	/*
	else if((hansiInfo[vrid].hstate == DISABLE_STATE)&&(hansiInfo[vrid].hpre_state == MASTTER_STATE)){
		printk("wifi_xmit,hansiInfo[%d]->hstate is %d,hpre_stat=%d.free skb.\n",vrid,hansiInfo[vrid].hstate,hansiInfo[vrid].hpre_state);
		kfree_skb(skb);
		return 0;
	}*/
	#ifdef __wsm__
	if((wsmswitch == 0)&&(wifi_tx_switch ==  0)){//0---goto wifi,1---goto wsm
	#endif
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("wifi,wifi_tx_switch=%d,data will goto wifi.fun:%s,priv->WLANID=%d.\n",wifi_tx_switch,__func__,priv->WLANID);
		if(priv->WLANID != 0)
		{

			if(skb->len > wifi_mtu){
				if(wifi_eth_debug >= WIFI_ERROR)
					printk("%s skb->len %d large than wifi_mtu %d.\n",__func__,skb->len, wifi_mtu);
				skb2 = dev_alloc_skb(wifi_mtu + 256+1);
				if (!skb2)
				{
					//printk("wifi.ko failed to allocate skbuff, packet dropped\n");
					kfree_skb(skb);
					return 0;
				}	
				
				skb_reserve(skb2,256);
				memcpy(skb_put(skb2, wifi_mtu), skb->data, wifi_mtu);	
				
				skb3 = dev_alloc_skb(skb->len - wifi_mtu + 256+1);
				if (!skb3)
				{
					//printk("wifi.ko failed to allocate skbuff, packet dropped\n");
					kfree_skb(skb2);
					kfree_skb(skb);
					return 0;
				}	
				
				skb_reserve(skb3,256);
				skb_pull(skb, wifi_mtu);
				memcpy(skb_put(skb3, skb->len), skb->data, skb->len);					
				kfree_skb(skb);
				if(priv->f802_3 == 1){
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s,%d.802.3 msg,more than wifi_mtu=%d.\n",__func__,__LINE__,wifi_mtu);
					wifi_assemble_capwap(skb2, priv,1);
				}else{
					wifi_803_3_to_802_11_f(skb2,skb3,priv);
					wifi_assemble_capwap(skb2, priv,1);
				}
		//		printk("wifi_xmit 3\n");	
				if(priv->isIPv6){
						if(WIFI_ERR == wifi_assemble_udp_ipv6(skb2, priv))
						{
							kfree_skb(skb2);					
							kfree_skb(skb3);
							priv->stats.tx_dropped++;
							drop_count++;
						}else{
					//		printk("wifi_xmit 2\n");
							wifi_assemble_capwap(skb3, priv,1);
					//		printk("wifi_xmit 3\n");
							if(WIFI_ERR == wifi_assemble_udp_ipv6(skb3, priv))
							{
								kfree_skb(skb3);
								priv->stats.tx_dropped++;
								drop_count++;
							}
						}	

				}else{
					if(WIFI_ERR == wifi_assemble_udp_ip(skb2, priv))
					{
						kfree_skb(skb2);					
						kfree_skb(skb3);
						priv->stats.tx_dropped++;
						drop_count++;
					}else{
				//		printk("wifi_xmit 2\n");
						wifi_assemble_capwap(skb3, priv,1);
				//		printk("wifi_xmit 3\n");
						if(WIFI_ERR == wifi_assemble_udp_ip(skb3, priv))
						{
							kfree_skb(skb3);
							priv->stats.tx_dropped++;
							drop_count++;
						}
					}	
				}
				return 0;
			}else{


				if(skb->cvm_cookie != CVM_COOKIE)//to judged this pkt is network Card's packet 
				{									  //if not the new skb
					skb2 = dev_alloc_skb(skb->len + 256+1);
					if (!skb2)
					{
						//printk("wifi.ko failed to allocate skbuff, packet dropped\n");
						kfree_skb(skb);
						return 0;
					}	
		
					skb_reserve(skb2,256);
					memcpy(skb_put(skb2, skb->len), skb->data, skb->len);	
					kfree_skb(skb);
				}
				else
				{
					skb2 = skb;
				}	
				
//				printk("%s1111 skb %d\n",__func__,skb_headroom(skb2));
				if(priv->f802_3 == 1){
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s,%d.802.3 msg\n",__func__,__LINE__);
					wifi_assemble_capwap(skb2, priv,1);
				}else{
					wifi_803_3_to_802_11(skb2, priv);
					wifi_assemble_capwap(skb2, priv,1);
				}
//				printk("%s333333 skb %d\n",__func__,skb_headroom(skb2));
		//		printk("wifi_xmit 3\n");
				if(priv->isIPv6){
					if(WIFI_ERR == wifi_assemble_udp_ipv6(skb2, priv))
					{
	//					printk("%s44444 skb %d\n",__func__,skb_headroom(skb2));
						kfree_skb(skb2);
						priv->stats.tx_dropped++;
						drop_count++;
					}
				}else{
					if(WIFI_ERR == wifi_assemble_udp_ip(skb2, priv))
					{
	//					printk("%s44444 skb %d\n",__func__,skb_headroom(skb2));
						kfree_skb(skb2);
						priv->stats.tx_dropped++;
						drop_count++;
					}
				}
//				printk("%s5555555 skb %d\n",__func__,skb_headroom(skb2));
			}
		}
		else
		{		
			kfree_skb(skb);
			priv->stats.tx_dropped++;
			drop_count++;
		}
#ifdef __wsm__
	}
	else
	{		
			if(wifi_eth_debug >= WIFI_DEBUG)
				printk("wifi,wifi_tx_switch=%d,data will goto wsm.fun:%s\n",wifi_tx_switch,__func__);
			vrid = priv->vrid;
			/*Each core correspond with a exclusive queue.*/
			p_mmap_head = (struct mmap_head *)vm[vrid];
			queue_head = (struct page_head *)(vm[vrid] + p_mmap_head->wifiTOwsm_array[core_id].offset);
			page_head_tx = (struct page_head *)((char *)queue_head + BUFF_SIZE * (p_mmap_head->wifiTOwsm_array[core_id].wifi_index));
		
			while(UNFINISHED != page_head_tx->finish)
				{
		
				/* Send the packet to the corresponding quene. if current queue if full, try the next one. 
					When all the queue is checked and all is full, return to the original queue ,drop it and return.
				*/
		
					/* jump to the next core queue. */
					core_id = ((core_id) % core_num);
					p_mmap_head = (struct mmap_head *)vm[vrid];
					queue_head = (struct page_head *)(vm[vrid] + p_mmap_head->wifiTOwsm_array[core_id].offset);
					page_head_tx = (struct page_head *)((char *)queue_head + BUFF_SIZE * (p_mmap_head->wifiTOwsm_array[core_id].wifi_index));
		
		
				
					if((round == ROUND_TIMES)||(core_num == 0x1)){
						kfree_skb(skb);
						priv->stats.tx_dropped++;
						drop_count++;
		/*				printk( KERN_DEBUG "Dropped packet:%s-core_id = 0x%d\n",__FUNCTION__, core_id);*/
						return 0;
						}
		
						round++;
					
		/*	printk(KERN_DEBUG "Waiting......coreid = %d, round = %d.\n",core_id, round);*/
		
				}
		
			/*Copy the packet from skb to corresponding queue.*/
			ptr_packet = (unsigned char *)page_head_tx + TX_OFFSET;
			page_head_tx->len = skb->len;
			page_head_tx->offset = TX_OFFSET;
			page_head_tx->wlanID = priv->wlanID;
			page_head_tx->BSSIndex= priv->BSSIndex;
			memcpy(ptr_packet, skb->data, skb->len);
		
			page_head_tx->finish = FINISHED;
			p_mmap_head->wifiTOwsm_array[core_id].wifi_index = (p_mmap_head->wifiTOwsm_array[core_id].wifi_index + 1) % ARRAY_LEN;
			priv->stats.tx_packets++;
			tx_count++;
			priv->stats.tx_bytes += skb->len;
			kfree_skb(skb);
	}
	#endif
//	printk("wifi_xmit 5\n");
	return 0;
}


