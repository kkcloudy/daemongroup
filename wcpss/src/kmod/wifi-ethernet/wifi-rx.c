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
* wifi-rx.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi-ethernet module receive packets routine
*
*
*******************************************************************************/

//#include <cvmx.h>
#include <../../../../kernel2.6.32.27cn/arch/mips/include/asm/octeon/cvmx.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "wifi.h"
#include "wifi-sm.h"
#include "wifi-rx.h"
#include "wifi-tx.h"
#include "wifi-ioctl.h"

extern int wifi_sleep_gap;
extern unsigned long long vm[VRID_MAX];
extern struct tasklet_struct wifi_tasklet[];
extern struct net_device * wifi_device[VRID_MAX][WIFI_INTERFACE_NUM];
extern int wifi_QoS_open;
extern int wifi_dp;

extern int wifi_if_index(unsigned int vrid, int wlanID, int BSSIndex);
extern int kthread_should_stop(void);
extern void msleep(unsigned int msecs);
extern __be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev);
void wifi_tasklet_rx(void);

int (*wifi_ko_dhcp_option82_hook)(struct sk_buff *skb, struct dba_result *res) = NULL;
EXPORT_SYMBOL(wifi_ko_dhcp_option82_hook);


int kthread_rx(void)
{
	while(1)
	{
		if (kthread_should_stop())
        		break;
        	preempt_disable();
        	tasklet_schedule(wifi_tasklet + smp_processor_id());
        	preempt_enable();
        	msleep(wifi_sleep_gap);
	}

	return 0;
}


/**
 * Tish function is scheduled on each core to fetch packets from wsm.
 *
 */
void wifi_tasklet_rx(void)
{
	unsigned char *ptr_packet = NULL;
	int ifindex;
	int ret = -1;
	unsigned int vrid = 0, cnt = 0, pkt_cnt = 0;
	struct sk_buff *skb = NULL;
	struct net_device *dev = NULL;
	wifi_dev_private_t *priv = NULL;
	int core_id = cvmx_get_core_num();	//current active core id
	struct mmap_head *p_mmap_head = NULL;
	struct page_head *queue_head = NULL;
	struct page_head *page_head_rx = NULL;

	for(vrid = 0, cnt = 0; cnt <= VRID_MAX; vrid = (vrid + 1) % VRID_MAX)
	{
		while(1)
		{
			p_mmap_head = (struct mmap_head *)vm[vrid];
			queue_head = (struct page_head *)(vm[vrid] + p_mmap_head->wsmTOwifi_array[core_id].offset);
			page_head_rx = (struct page_head *)((char *)queue_head + 
					BUFF_SIZE * (p_mmap_head->wsmTOwifi_array[core_id].wifi_index));
			if (page_head_rx->finish != FINISHED)
			{
				cnt++;
				break;
			}

			cnt = 0;
			ifindex = wifi_if_index(vrid, page_head_rx->wlanID, page_head_rx->BSSIndex);
			if(ifindex == -1)
			{
				if( wifi_eth_debug >= WIFI_WARNING )
					printk("wifi: ifindex = -1,vrid=%d.\n",vrid);
				
				page_head_rx->finish = UNFINISHED;
				p_mmap_head->wsmTOwifi_array[core_id].wifi_index = 
					(p_mmap_head->wsmTOwifi_array[core_id].wifi_index + 1) % ARRAY_LEN;
				continue;
			}
			
			dev = wifi_device[vrid][ifindex];
			if( wifi_eth_debug >= WIFI_WARNING )
				printk("wifi: %s,%d,vrid=%d.\n",__func__,__LINE__,vrid);
			priv = (wifi_dev_private_t *)dev->priv;
			/*Malloc skb*/
			ptr_packet = (unsigned char *)page_head_rx + page_head_rx->offset;
			skb = dev_alloc_skb(page_head_rx->len);
			if (!skb)
			{
				if( wifi_eth_debug >= WIFI_WARNING ) 
                			printk("wifi.ko failed to allocate skbuff, packet dropped\n");
				continue;
            }		
		
			memcpy(skb_put(skb, page_head_rx->len), ptr_packet, page_head_rx->len);

			/*Receiving accomplished,  modify the mmap flags.*/
			memset(page_head_rx, 0, BUFF_SIZE);
			page_head_rx->finish = UNFINISHED;
			p_mmap_head->wsmTOwifi_array[core_id].wifi_index = (p_mmap_head->wsmTOwifi_array[core_id].wifi_index + 1) % ARRAY_LEN;
		
			/*Inicialize skb, enter TCP/IP protocol stack.*/
			skb->protocol = eth_type_trans(skb, dev);
			skb->dev = dev;
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		
			priv->stats.rx_packets++;
			priv->stats.rx_bytes += skb->len;

			ret = netif_receive_skb(skb);
			skb = NULL;
		}
	}
}

int wifi_kernel_rx(struct sk_buff *skb){
	int ret;	
	struct net_device *dev = NULL;
	struct sk_buff *skb2 = NULL;
	struct ieee80211_frame *macHead;
	struct ieee80211_llc *llcHead;
	struct wifi_bss_tbl *bss = NULL;
	struct wifi_sta_tbl *sta = NULL;
	struct udphdr *udp = NULL;
	struct vlan_h *vh = NULL;
	wifi_dev_private_t *priv = NULL;
	unsigned short *isEAP=NULL;
	unsigned char proto_ver=0, type=0, toDS_fromDS=0, qos_flag=0;	
	unsigned char *BSSID = NULL;
	unsigned char *DMAC = NULL;
	unsigned char *SMAC = NULL;	
	unsigned char mac[MAC_LEN] = {0};
	unsigned int vrid=0;
	unsigned int dev_index=0;
	unsigned short udp_len = 0;
	unsigned int wbidFlag = 0;
	unsigned int TFlag = 0;
	int len = sizeof(struct udphdr);	
	struct capwap_head *head2 = NULL;
	struct capwap_head_802_3 *head = NULL;
	unsigned int capwap1=0;
	struct iphdr *iph = NULL;
	if(!skb){
		printk(KERN_ERR "wifi kernel rx : skb is null\n");
		return 2;
	}
	else if(!(skb->data)){
		printk(KERN_ERR "wifi kernel rx : skb->data is null\n");
		return 2;
	}
	udp = (struct udphdr *)(skb->data);
	udp_len = udp->len;
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s %d,nl_sk=%p.\n",__func__,__LINE__,nl_sk);
	//if(udp->len < MIN_NET_DATA_LEN)
	{
		if(udp->len < MIN_DATA_LEN)
		{
			if(wifi_eth_debug >= WIFI_ERROR)
				printk(KERN_WARNING "capwap udp length(%u) is too short, need to drop.\n",udp->len);
			return 2;/* return to drop.*/
		}
		#if 0
			if(wifi_eth_debug >= WIFI_WARNING)
				printk(KERN_WARNING "capwap udp length(%u) is too short, need to wsm.\n",udp->len);
			return 1;/* return to WSM.*/
		#endif	
	}
	
	/*if(wsmswitch != 0)
		return 1;
	if(wifi_rx_switch == 1){////0---goto wifi,1---goto wsm
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("wifi,11,wifi_rx_switch=%d,data will goto wsm.fun:%s\n",wifi_rx_switch,__func__);
		return 1;
	}else{
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("wifi,22,wifi_rx_switch=%d,data will goto wifi.fun:%s\n",wifi_rx_switch,__func__);
	}*/
	if(wifi_eth_debug >= WIFI_DEBUG)
	{
		printk("wsmswitch == 0 use function wifi_kernel_rx()\n");
	}
	unsigned short s_port = udp->source;
	unsigned short d_port = udp->dest;
	//printk("wifi_port:s_port:%d,d_port:%d\n",s_port,d_port);
	if (skb_is_nonlinear(skb) &&
		skb_linearize(skb) != 0) {/*for data_len != 0 skb process*/
		return 2;
    }
	skb_pull(skb,len);
	head2 = (struct capwap_head *)skb->data;
	capwap1 = head2->capwap1;
	wbidFlag = CWGetField32(capwap1, 18, 5);/*capwap header WBID*/
	TFlag = CWGetField32(capwap1, 23, 1);
	head = (struct capwap_head_802_3 *)skb->data;
	if(wifi_eth_debug >= WIFI_DEBUG)
	{
		printk("%s,%d,TFlag=%d,wbidFlag=%d.\n",__func__,__LINE__,TFlag,wbidFlag);
	}
	if((TFlag==1)&&(wbidFlag == 1)){
		skb_pull(skb,16);/*capwap header*/
		macHead = (struct ieee80211_frame *)(skb->data);
		proto_ver = macHead->i_fc[0] & IEEE80211_FC0_VERSION_MASK;
		type = macHead->i_fc[0] & IEEE80211_FC0_TYPE_MASK;	
		qos_flag = macHead->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS;
		toDS_fromDS = macHead->i_fc[1] & IEEE80211_FC1_DIR_MASK;
		if((proto_ver != IEEE80211_FC0_VERSION_0)||(IEEE80211_FC1_DIR_DSTODS == toDS_fromDS)){
			return 2;/*wds or other version  drop*/
		}
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("%s type %d(0-mgmt,4-ctrl,8-data).\n",__func__,type);
		switch(type){
			case IEEE80211_FC0_TYPE_DATA:
				if(qos_flag){
					if(udp->len < MIN_WIFI_DOT11_DATA_LEN+4){
						if(wifi_eth_debug >= WIFI_ERROR)
							printk(KERN_WARNING "capwap dot11 data udp length(%u) is too short, need to drop.\n",udp->len);
						return 2;/* return to drop.*/
					}
				}else{
					if(udp->len < MIN_WIFI_DOT11_DATA_LEN){
						if(wifi_eth_debug >= WIFI_ERROR)
							printk(KERN_WARNING "capwap dot11 data udp length(%u) is too short, need to drop.\n",udp->len);
						return 2;/* return to drop.*/
					}
				}
				if(qos_flag)
					skb_pull(skb,28);
				else
					skb_pull(skb,24);		
				llcHead = (struct ieee80211_llc *)(skb->data);
				isEAP = llcHead->llc_ether_type;
				if((wifi_ipv6_dr_sw == 1)&&(*isEAP == 0x86DD)){/*drop ipv6 msg*/
					if(wifi_eth_debug >= WIFI_ERROR)
						printk("%s,%d,*isEAP=0x%X.ipv6 msg,drop.\n",__func__,__LINE__,*isEAP);
					if(qos_flag)
						skb_push(skb,44+len);
					else
						skb_push(skb,40+len);						
					return 2; /*dtrop*/
				}
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("%s,%d,*isEAP=0x%X.udp_len=%d.\n",__func__,__LINE__,*isEAP,udp_len);
				/*if(((udp_len < MIN_NET_DATA_LEN)&&(*isEAP != 0x888E))||(*isEAP == 0x88C7) || (*isEAP == 0x88b4)){
					if(asdswitch != 1){
						if(qos_flag)
							skb_push(skb,44+len);
						else
							skb_push(skb,40+len);						
						return 1; 
					}
				}*/
				switch(toDS_fromDS){
					case IEEE80211_FC1_DIR_TODS: /* To DS */
						if(wifi_eth_debug >= WIFI_DEBUG){
							printk("%s,%d.%02X:%02X:%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,__LINE__,macHead->i_addr1[0],macHead->i_addr1[1],macHead->i_addr1[2],macHead->i_addr1[3],macHead->i_addr1[4],macHead->i_addr1[5],\
								macHead->i_addr2[0],macHead->i_addr2[1],macHead->i_addr2[2],macHead->i_addr2[3],macHead->i_addr2[4],macHead->i_addr2[5],\
								macHead->i_addr3[0],macHead->i_addr3[1],macHead->i_addr3[2],macHead->i_addr3[3],macHead->i_addr3[4],macHead->i_addr3[5]);
						}
						BSSID = macHead->i_addr1;
						SMAC = macHead->i_addr2;
						DMAC = macHead->i_addr3;
						break;
					
					case IEEE80211_FC1_DIR_NODS: /* Ad hoc */
						BSSID = macHead->i_addr3;
						SMAC = macHead->i_addr2;
						DMAC = macHead->i_addr1;
						break;
					
					case IEEE80211_FC1_DIR_FROMDS: /* From DS */
						BSSID = macHead->i_addr2;
						SMAC = macHead->i_addr3;
						DMAC = macHead->i_addr1;
						break;
					default:
						return 2;
				}
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("%s,%d,BSSID=%02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,__LINE__,BSSID[0],BSSID[1],BSSID[2],BSSID[3],BSSID[4],BSSID[5]);
				bss = wifi_bssid_bssidx_tbl_get(BSSID);
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("%s,%d.bss=%p.\n",__func__,__LINE__,bss);
				if(bss == NULL){
					if(qos_flag)
						skb_push(skb,44+len);
					else
						skb_push(skb,40+len);						
					return 2; /*to drop*/
				}
				if((bss->roaming_flag == 2)
					&&((bss->Eap1XServerSwitch == 1)
					||((bss->Eap1XServerSwitch == 0)&&(*isEAP != 0x888E)&&(*isEAP != 0x88C7)&&(*isEAP != 0x88b4))))
				{
						sta = wifi_sta_tbl_get(SMAC);
						if((sta !=NULL)&&(sta->roaming_flag == 2))
						{
							bss = wifi_bssid_bssidx_tbl_get(sta->BSSID_Before);
							if(bss == NULL){
								if(qos_flag)
									skb_push(skb,44+len);
								else
									skb_push(skb,40+len);						
								return 2; 
							}
							if(wifi_eth_debug >= WIFI_DEBUG)	
							{
								printk("%s,%d,sta :%02X:%02X:%02X:%02X:%02X:%02X. is roaming\n",__func__,__LINE__,SMAC[0],SMAC[1],SMAC[2],SMAC[3],SMAC[4],SMAC[5]);
							}						
						}
				}
				vrid = bss->vrid;
				priv = (wifi_dev_private_t*)&(bss->priv);
				priv->apport = s_port;
				/*if((asdswitch != 1)&&(*isEAP == 0x888E)){
					if(bss->Eap1XServerSwitch == 1){
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s,%d,*isEAP=%x.\n",__func__,__LINE__,*isEAP);
					}else{
						if(qos_flag)
							skb_push(skb,44+len);
						else
							skb_push(skb,40+len);						
						return 1;
					}
				}
				*/
				/*	bss->Eap1XServerSwitch is enable,eap msg not to asd*/
					if((asdswitch == 1) &&(bss->Eap1XServerSwitch == 0)&& ((*isEAP == 0x888E) || (*isEAP == 0x88C7) || (*isEAP == 0x88b4))){
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s handle eap msg BssIndex %d.\n",__func__,bss->BSSIndex);
						DataMsgHead *pdata = NULL;
						if(qos_flag)
							skb_push(skb,28+sizeof(DataMsgHead));
						else
							skb_push(skb,24+sizeof(DataMsgHead));
					
						skb2 = dev_alloc_skb(skb->len + 256);
						if (!skb2){
							printk("wifi.ko failed to allocate skbuff, packet dropped\n");
							//kfree_skb(skb);
							return 2;
						}	
					
						skb_reserve(skb2,256);
						memcpy(skb_put(skb2, skb->len), skb->data, skb->len);	
						kfree_skb(skb);
						//skb2->dst = NULL;
					
						pdata = (DataMsgHead*)(skb2->data);
						memset(pdata,0,sizeof(DataMsgHead));
						pdata->Type = IEEE802_11_EAP;
						pdata->BSSIndex = bss->BSSIndex;
						pdata->Radio_G_ID = bss->BSSIndex/L_BSS_NUM;
						pdata->DataLen = skb2->len - sizeof(DataMsgHead);
						skb_push(skb2,sizeof(struct nlmsghdr));
						kernel_to_asd(skb2,vrid);
						return 3;
					}

				dev_index = bss->dev_index;
				if(wifi_device[vrid][dev_index] == NULL){
					if(qos_flag)
						skb_push(skb,44+len);
					else
						skb_push(skb,40+len);						
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s,%d\n",__func__,__LINE__);
					return 2; /*to drop*/
				}
				dev = wifi_device[vrid][dev_index];
				if(hansiInfo[vrid].dhcpoption82 == 1){
					priv = (wifi_dev_private_t *)dev->priv;
					if(priv->res.len > 17){
						sprintf(priv->res.data + priv->res.len - 17,"%02X:%02X:%02X:%02X:%02X:%02X",SMAC[0],SMAC[1],SMAC[2],SMAC[3],SMAC[4],SMAC[5]);
					}
				}

				//add by zdx 2010-08-23
				if(0)
				{
					struct ieee80211_llc *llcHead = (struct ieee80211_llc*)(skb->data);
					
					if(llcHead->llc_ether_type[0] == 0x08 && llcHead->llc_ether_type[1] == 0x00)
					{/*ipv4 packet*/
							skb_pull(skb,sizeof(struct ieee80211_llc));	
							struct ip_header_t *pTmpIPHead = (struct ip_header_t*)(skb->data);
							
							//char *pchtemp =  skb->data + 1;	
							pTmpIPHead->dscp = 0;
							pTmpIPHead->ect = 0;
							pTmpIPHead->ecnce = 0;
							pTmpIPHead->dscp = wifi_dp;
							//*pchtemp = 0xe0;//56;
							//printk(KERN_WARNING "dscp: %d.\n",pTmpIPHead->dscp);	
							//printk(KERN_WARNING "............................\n");
							pTmpIPHead->checksum = 0;
							pTmpIPHead->checksum = ip_fast_csum((unsigned char *)pTmpIPHead, pTmpIPHead->hdrLen);
							if(wifi_eth_debug >= WIFI_DEBUG)skb_dump("chInfo",skb);
							
							skb_push(skb,sizeof(struct ieee80211_llc));	
					}
					else if(llcHead->llc_ether_type[0] == 0x86 && llcHead->llc_ether_type[1] == 0xdd)
					{/*ipv6 packet*/
						skb_pull(skb,sizeof(struct ieee80211_llc));	
						unsigned short *pIpv6Hdr = (unsigned short *)(skb->data);
						unsigned short value = *pIpv6Hdr;
						if(wifi_eth_debug >= WIFI_DEBUG)skb_dump("Ipv6 packet before Qos modified.",skb);
						if( (value & 0xf000) == 0x6000 )
						{
							*pIpv6Hdr = (*pIpv6Hdr) & 0xf00f; /* clear traffic class */
							*pIpv6Hdr = (*pIpv6Hdr) | 0x0380; /* set traffic class to be 56(0x38) */
							if(wifi_eth_debug >= WIFI_DEBUG)skb_dump("ipv6 packet after QoS modified",skb);
						}
						skb_push(skb,sizeof(struct ieee80211_llc));	
					}
				}
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("3bss[%d]->Eap1XServerSwitch=%u.\n",bss->BSSIndex,bss->Eap1XServerSwitch);
				if(*isEAP == 0x888E){
					if(bss->Eap1XServerSwitch == 1){
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s,%d,*isEAP=%x.\n",__func__,__LINE__,*isEAP);
						if((hansiInfo[vrid].vlanSwitch == 1)&&(bss->vlanid != 0)){
							skb_pull(skb,2);
							vh = skb->data;
							vh->h_vlan_proto = wifi_8021q_type;
							vh->h_vlan_TCI = bss->vlanid;
							skb_push(skb,6);					
						}
						memcpy(skb->data, SMAC, 6);
						skb_push(skb,6);
						memmove(skb->data, bss->Eap1XServerMac, 6);
					}else{/*
						if(qos_flag)
							skb_push(skb,44+len);
						else
							skb_push(skb,40+len);						
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s,%d\n",__func__,__LINE__);
						return 1; 
						*/
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s handle eap msg BssIndex %d.\n",__func__,bss->BSSIndex);
						DataMsgHead *pdata = NULL;
						if(qos_flag)
							skb_push(skb,28+sizeof(DataMsgHead));
						else
							skb_push(skb,24+sizeof(DataMsgHead));
					
						skb2 = dev_alloc_skb(skb->len + 256);
						if (!skb2){
							printk("wifi.ko failed to allocate skbuff, packet dropped\n");
							//kfree_skb(skb);
							return 2;
						}	
					
						skb_reserve(skb2,256);
						memcpy(skb_put(skb2, skb->len), skb->data, skb->len);	
						kfree_skb(skb);
						//skb2->dst = NULL;
					
						pdata = (DataMsgHead*)(skb2->data);
						memset(pdata,0,sizeof(DataMsgHead));
						pdata->Type = IEEE802_11_EAP;
						pdata->BSSIndex = bss->BSSIndex;
						pdata->Radio_G_ID = bss->BSSIndex/L_BSS_NUM;
						pdata->DataLen = skb2->len - sizeof(DataMsgHead);
						skb_push(skb2,sizeof(struct nlmsghdr));
						kernel_to_asd(skb2,vrid);
						return 3;
					}
				}else{
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s,%d hansiInfo[%d].vlanSwitch %d bss->vlanid %d\n",__func__,__LINE__,vrid,hansiInfo[vrid].vlanSwitch,bss->vlanid);
					if((hansiInfo[vrid].vlanSwitch == 1)&&(bss->vlanid != 0)){
						skb_pull(skb,2);
						vh = skb->data;
						vh->h_vlan_proto = wifi_8021q_type;
						vh->h_vlan_TCI = bss->vlanid;
						skb_push(skb,6);					
						memcpy(mac, DMAC, 6);
						memcpy(skb->data, SMAC, 6);
						skb_push(skb,6);
						memcpy(skb->data, mac, 6);
					}else{
						memcpy(skb->data, SMAC, 6);
						skb_push(skb,6);
						memmove(skb->data, DMAC, 6);
					}
				}

				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("%s,%d\n",__func__,__LINE__);
				break;				
			case IEEE80211_FC0_TYPE_MGT:
				if(asdswitch != 1){
					skb_push(skb,16+len);	
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s %d,asdswitch is %d,drop msg.\n",__func__,__LINE__,asdswitch);
					return 2;				
				}else{
					DataMsgHead *pdata = NULL;

					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s handle 802.11 msg toDS_fromDS %d.\n",__func__,toDS_fromDS);
					BSSID = macHead->i_addr3;
					//printk("%s BSSID %02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,BSSID[0],BSSID[1],BSSID[2],BSSID[3],BSSID[4],BSSID[5]);
					bss = wifi_bssid_bssidx_tbl_get(BSSID);
					if(bss == NULL){
						if(wifi_eth_debug >= WIFI_DEBUG)
							printk("%s (bss == NULL).\n",__func__);
						return 2; 
					}
					vrid = bss->vrid;
					priv = (wifi_dev_private_t*)&(bss->priv);
					priv->apport = s_port;
					skb2 = dev_alloc_skb(skb->len + 256);
					if (!skb2){
						printk("wifi.ko failed to allocate skbuff, packet dropped\n");
						return 2;
					}	
				
					skb_reserve(skb2,256);
					memcpy(skb_put(skb2, skb->len), skb->data, skb->len);	
					kfree_skb(skb);
					//skb2->dst = NULL;

					skb_push(skb2,sizeof(DataMsgHead));
					pdata = (DataMsgHead*)(skb2->data);
					memset(pdata,0,sizeof(DataMsgHead));
					pdata->Type = IEEE802_11_MGMT;
					pdata->BSSIndex = bss->BSSIndex;
					pdata->Radio_G_ID = bss->BSSIndex/L_BSS_NUM;
					pdata->DataLen = skb2->len - sizeof(DataMsgHead);
					skb_push(skb2,NLMSG_LENGTH(0));
					kernel_to_asd(skb2,vrid);
					return 3;
				}
			case IEEE80211_FC0_TYPE_CTL:
			default:
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("%s,%d\n",__func__,__LINE__);
				if(udp_len < MIN_NET_DATA_LEN){
					if(wifi_eth_debug >= WIFI_DEBUG)
						printk("%s,%d\n",__func__,__LINE__);
					skb_push(skb,16+len);	
					return 2;				
				}
				return 2;
		}
	}else if(TFlag == 0){
			if(wifi_eth_debug >= WIFI_DEBUG)
				printk("%s,%d,wbidFlag=%d,%d.\n",__func__,__LINE__,wbidFlag,TFlag);
			if(udp->len < MIN_WIFI_DOT3_DATA_LEN){
				if(wifi_eth_debug >= WIFI_WARNING)
					printk(KERN_WARNING "capwap dot3 data udp length(%u) is too short, need to drop.\n",udp->len);
				return 2;/* return to drop.*/
			}

			skb_pull(skb,16);/*capwap head*/
			BSSID = head->bssid;			
			bss = wifi_bssid_bssidx_tbl_get(head->bssid);
			if(bss == NULL){
				printk("%s,%d.\n",__func__,__LINE__);
				skb_push(skb,16+14+len);						
				return 2; 
			}				
			if(wifi_eth_debug >= WIFI_DEBUG)
				printk("%s,%d,BSSID=%02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,__LINE__,BSSID[0],BSSID[1],BSSID[2],BSSID[3],BSSID[4],BSSID[5]);
			
			vrid = bss->vrid;
			dev_index = bss->dev_index;
			if(wifi_device[vrid][dev_index] == NULL){
				printk("%s,%d.\n",__func__,__LINE__);
				skb_push(skb,16+14+len);						
				return 2; /*to drop*/
			}
		
			dev = wifi_device[vrid][dev_index];
	}else{
		skb_push(skb,len);	
		if(wifi_eth_debug >= WIFI_ERROR){
			printk("%s,%d,TFlag(%d),wbidFlag(%d),invalid capwap msg.",__func__,__LINE__,TFlag,wbidFlag);
		}
		iph = (struct iphdr *)skb_network_header(skb);
		unsigned int sip = iph->saddr;
		if(wifi_eth_debug >= WIFI_ERROR){
			printk("%s,%d,sip: %d.%d.%d.%d.\n",__func__,__LINE__, \
						((iph->saddr) >> 24) & 0xFF, (iph->saddr >> 16) & 0xFF,(iph->saddr >> 8) & 0xFF, iph->saddr & 0xFF);
			printk("%s,%d,dip: %d.%d.%d.%d.\n",__func__,__LINE__, \
						(iph->daddr >> 24) & 0xFF, (iph->daddr >> 16) & 0xFF,(iph->daddr >> 8) & 0xFF, iph->daddr & 0xFF);
		}
		//CWCaptrue_wifi(skb->len,skb->data);/*only printk data*/
		return 2;				
	}
	if(NULL == dev){
		if(wifi_eth_debug >= WIFI_ERROR){
			printk("<warning>%s,%d,dev=%p.",__func__,__LINE__,dev);
		}
		//kfree_skb(skb);
		return 2;
	}
	skb->protocol = eth_type_trans(skb, dev);
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	priv = (wifi_dev_private_t *)dev->priv;
	priv->apport = s_port;
	priv->stats.rx_packets++;
	priv->stats.rx_bytes += skb->len;		
	
	nf_reset(skb);
	
	if(skb_dst(skb)){
		dst_release(skb_dst(skb));
		skb->_skb_dst = NULL;		
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("%s,%d,release skb dst.\n",__func__,__LINE__);
	}
	if((hansiInfo[vrid].dhcpoption82 == 1)&&(type == IEEE80211_FC0_TYPE_DATA)){
		if(wifi_ko_dhcp_option82_hook)
			wifi_ko_dhcp_option82_hook(skb,&(priv->res));
	}
	

	netif_rx(skb);	
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s,%d\n",__func__,__LINE__);
	return 0;
}
EXPORT_SYMBOL(wifi_kernel_rx);



