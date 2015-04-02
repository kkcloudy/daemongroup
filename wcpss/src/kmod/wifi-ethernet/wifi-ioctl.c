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
* wifi-ioctl.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi-ethernet module ioctl interface
*
*
*******************************************************************************/

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/if.h>
#include <net/if_inet6.h>
#include <linux/vmalloc.h>

#include "wifi.h"
#include "wifi-ioctl.h"
#include "wifi-common.h"


#define in6_dev_put(idev)  atomic_dec(&(idev)->refcnt)
#define in6_dev_hold(idev)   atomic_inc(&(idev)->refcnt)
#define in6_ifa_put(ifp)	atomic_dec(&(ifp)->refcnt)
#define in6_ifa_hold(ifp)	atomic_inc(&(ifp)->refcnt)


extern struct net_device *wifi_device[VRID_MAX][WIFI_INTERFACE_NUM];
/*extern int wifi_common_init(struct net_device *dev);*/
extern void wifi_common_uninit(struct net_device *dev);
int wifi_netdev_ops_init(struct net_device *dev);

unsigned int wsmswitch = 0;
extern int wifi_eth_debug;
unsigned int asdswitch = 1;//wuwl change default to enable

struct oct_dev_s {
	long quantum;
	long qset;
	unsigned long size;
	unsigned long access_key;
	struct cdev cdev;
};

int dev_refcnt_decrement(atomic_t *v)
{
	v->counter -= 1;

	return 0;
}


int dynamic_registe_if(struct interface_basic_INFO *if_info)
{
	unsigned int index = 0, vrid = 0;
	struct net_device *dev = NULL;
	unsigned int ret = 0;
	if(NULL == if_info)
	{
		return -1;
	}
	//printk("dynamic_registe_if: name=%s\n", if_info->if_name);

	vrid = if_info->vrid;
	if((0 == if_info->BSSIndex) && (if_info->wlanID > 0))
	{
		index = if_info->wlanID;
		printk("dynamic_registe_if: vrid = %d wlanID=%x\n", vrid, if_info->wlanID);
	}
	else if((0 == if_info->wlanID) && (if_info->BSSIndex > 0))
	{
		index = 256 + if_info->BSSIndex;
		//printk("dynamic_registe_if: vrid = %d, BSSIndex=%x\n", vrid, if_info->BSSIndex);
	}
	else
	{
		printk("dynamic_registe_if error: vrid = %d wlanID=%d, BSSIndex=%d\n", 
				vrid, if_info->wlanID, if_info->BSSIndex);
		return -1;
	}
	
	
	if(index >= WIFI_INTERFACE_NUM || vrid >= VRID_MAX)
	{
		printk("invalid interface Index = %d, vrid = %d\n", index, if_info->vrid);
		return -1;
	}
	
	if(wifi_device[vrid][index])
	{
		//printk("wifi:The dev is already registered.will update dev vrid=%d index=%d\n", vrid, index);
		//return -1;
		ret = 1;
	}
	if(1 == ret){
		dev = wifi_device[vrid][index];
		if(dev)
		{
			//wifi_dev_private_t* priv2 = (wifi_dev_private_t *)netdev_priv(dev);
			wifi_dev_private_t* priv = (wifi_dev_private_t *)dev->priv;
			memset(priv, 0, sizeof(wifi_dev_private_t));
	
			SET_MODULE_OWNER(dev);
			dev->mtu = 1416;
			dev->tx_queue_len = 0;
			/*dev->flags |= IFF_UP;*/
			dev->flags |= IFF_RUNNING;
			dev->priv_flags = IFF_WIRELESS;
			/*dev->init = wifi_common_init;*/
			/*dev->uninit = wifi_common_uninit;*/
			wifi_netdev_ops_init(dev);
			memcpy(dev->name, if_info->if_name, 15);
			priv->wlanID = if_info->wlanID;
			priv->BSSIndex = if_info->BSSIndex;
			priv->vrid = vrid;
	//		skb_queue_head_init(&priv->tx_free_list);
			/*
			if (register_netdev(dev)<0)
			{
				 printk("\t\tFailed to register ethernet device for wifi\n");
				 kfree(dev);
				 return -1;
			}
			else
			{
				 rtnl_lock();
				 dev_open(dev);  //for RUNNING state.
				 rtnl_unlock();
				 wifi_device[vrid][index] = dev;
			}*/
		}
		else
		{
			printk("\t\twifi:Failed to update ethernet device for wifi\n");
			return -1;
		}
		
	}else{
		dev = alloc_etherdev_mq(sizeof(wifi_dev_private_t),1);
		if(dev)
		{
			wifi_dev_private_t* priv = (wifi_dev_private_t *)netdev_priv(dev);
			dev->priv = netdev_priv(dev);
			memset(priv, 0, sizeof(wifi_dev_private_t));
			memset(priv->res.data, 0, 256);
			SET_MODULE_OWNER(dev);
			dev->mtu = 1416;
			dev->tx_queue_len = 0;
			//dev->flags |= IFF_UP;
			dev->flags |= IFF_RUNNING;
			dev->priv_flags = IFF_WIRELESS;
			//dev->init = wifi_common_init;
			//v->netdev_ops = &wifi_netdev_ops;
			wifi_netdev_ops_init(dev);
			//printk("%s,%d,priv=%p,dev=%p.",__func__,__LINE__,priv,dev);
			/*dev->uninit = wifi_common_uninit;*/
			memcpy(dev->name, if_info->if_name, 15);
			priv->wlanID = if_info->wlanID;
			priv->BSSIndex = if_info->BSSIndex;
			priv->vrid = vrid;
	//		skb_queue_head_init(&priv->tx_free_list);

			if (register_netdev(dev)<0)
			{
				 printk("\t\tFailed to register ethernet device for wifi\n");
				 kfree(dev);
				 return -1;
			}
			else
			{
				 rtnl_lock();
				 dev_open(dev);  //for RUNNING state.
				 rtnl_unlock();
				 wifi_device[vrid][index] = dev;
			}
		}
		else
		{
			printk("\t\tFailed to allocate ethernet device for wifi\n");
			return -1;
		}
	}
	return 0;
}


int dynamic_unregiste_if(struct interface_basic_INFO *if_info)
{
	unsigned int index = 0, vrid = 0;

	if(NULL == if_info)
	{
		return -1;
	}

	vrid = if_info->vrid;
	if((0 == if_info->BSSIndex) && (if_info->wlanID > 0))
	{
		index = if_info->wlanID;
		printk("dynamic_unregiste_if: vrid = %d wlanID=%x\n", vrid, if_info->wlanID);
	}
	else if((0 == if_info->wlanID) && (if_info->BSSIndex > 0))
	{
		index = 256 + if_info->BSSIndex;
		printk("dynamic_unregiste_if: vrid = %d, BSSIndex=%x\n", vrid, if_info->BSSIndex);
	}
	else
	{
		printk("dynamic_unregiste_if error: vrid = %d wlanID=%d, BSSIndex=%d\n", 
				vrid, if_info->wlanID, if_info->BSSIndex);
		return -1;
	}
	
	if(index >= WIFI_INTERFACE_NUM || vrid >= VRID_MAX)
	{
		printk("invalid interface Index = %d, vrid = %d\n", index, if_info->vrid);
		return -1;
	}
	if(wifi_device[vrid][index])
	{
		struct net_device *dev = wifi_device[vrid][index];
		wifi_dev_private_t* priv = (wifi_dev_private_t*)dev->priv;
		//wifi_bssid_bssidx_tbl_del(priv->bssid);    /*ht del for local bss use*/		
		struct net_device * device = wifi_device[vrid][index];
		wifi_device[vrid][index] = NULL;
		while (atomic_read(&dev->refcnt) != 0) {
			printk("interface  %s reference count unequal to 0\n",if_info->if_name);
			dev_refcnt_decrement(&dev->refcnt);
			}
		unregister_netdev(device);
		free_netdev(device);
		//kfree(device);
		device = NULL;
	}
	else
		return -1;
	
	return 0;
}

int clear_radio_if_all_by_vrid(struct clear_radio_if *vrid_info) {

	unsigned int vrid = 0;
	unsigned int index = 0;
	
	if (vrid_info == NULL) {
		printk("vrid_info is NULL\n");
		return -1;
	}

	vrid = vrid_info->vrid;

	if (vrid >= VRID_MAX) {
		printk("vrid is out fo range (0,%d)\n", VRID_MAX);
		return -2;
	}

	for (index=0; index<WIFI_INTERFACE_NUM; index++) {
		if(wifi_device[vrid][index]) {
			struct net_device *dev = wifi_device[vrid][index];
			wifi_dev_private_t* priv = (wifi_dev_private_t*)dev->priv;	
			struct net_device * device = wifi_device[vrid][index];
			wifi_device[vrid][index] = NULL;
			unregister_netdev(device);
			free_netdev(device);
			device = NULL;
		}
	}
	return 0;
}

unsigned char wifi_tbl_mac_key(unsigned char *pMAC)
{
	return pMAC[5];
}
struct wifi_bss_tbl * wifi_bssid_bssidx_tbl_get(unsigned char *bssid)
{
	unsigned char index = 0;
	struct wifi_bss_tbl *p_pre_BSS = NULL;
	if(!bssid)
	{
		/*syslog*/
		return NULL; 
	}
	index = wifi_tbl_mac_key(bssid);
	if(index >= BSS_HASH_SIZE)
		return NULL;
	p_pre_BSS = wifi_bss_hash[index];
	
	while (p_pre_BSS != NULL)
	{
		if (memcmp(p_pre_BSS->BSSID,bssid,6) == 0)
		{
			return p_pre_BSS; 
		}
		p_pre_BSS = p_pre_BSS->next;
	}
	return NULL;
}
struct wifi_bss_tbl * wifi_bssid_bssidx_tbl_add(unsigned char *bssid)
{
	if(NULL == bssid){
		return NULL;
	}
	unsigned char index = wifi_tbl_mac_key(bssid);
	if(index >= BSS_HASH_SIZE){
		return NULL;
	}
	struct wifi_bss_tbl *pmalloc = NULL;
	pmalloc = wifi_bssid_bssidx_tbl_get(bssid);
	if(pmalloc)
		return pmalloc;
	pmalloc = (struct wifi_bss_tbl *)kmalloc(sizeof(struct wifi_bss_tbl),GFP_ATOMIC);
	if (pmalloc == NULL)
	{
		return NULL;
	}
	
	memset(pmalloc,0, sizeof(struct wifi_bss_tbl));
	memcpy(pmalloc->BSSID, bssid, MAC_LEN);
	pmalloc->next = wifi_bss_hash[index];
	wifi_bss_hash[index] = pmalloc;

	return pmalloc;
}


int wifi_bssid_bssidx_tbl_del(unsigned char *bssid)
{
	unsigned char index = 0;
	struct wifi_bss_tbl *p_pre_BSS = NULL;
	struct wifi_bss_tbl *p_next_BSS = NULL;
	if(NULL == bssid){
		return NULL;
	}
	index = wifi_tbl_mac_key(bssid);
	if(index >= BSS_HASH_SIZE){
		return NULL;
	}
	p_pre_BSS = wifi_bss_hash[index];
	p_next_BSS = p_pre_BSS;
	
	while (p_next_BSS != NULL)
	{
		if (memcmp(p_next_BSS->BSSID,bssid,6) == 0)
		{
			/* Delete list header */
			if (p_next_BSS == p_pre_BSS)
			{
				wifi_bss_hash[index] = p_next_BSS->next;
				kfree(p_next_BSS);

				return 1;
			}
			else
			{
				p_pre_BSS->next = p_next_BSS->next;
				kfree(p_next_BSS);
				return 1;
			}
		}
		p_pre_BSS = p_next_BSS;
		p_next_BSS = p_next_BSS->next;
	}

	return 0;
}
 struct wifi_sta_tbl *wifi_sta_tbl_get(const char *s)
{
	if(NULL == s){
		return NULL;
	}
	unsigned char index =0 ;
	struct wifi_sta_tbl  *sta = NULL;
	index = wifi_tbl_mac_key(s);
	if(index >= STA_HASH_SIZE){
		return NULL;
	}
	sta = wifi_sta_hash[index];
	while(sta != NULL)
	{
		if(memcmp(sta->STAMAC,s,MAC_LEN) == 0 )
			return sta; 
		sta = sta->next;

	}
	if(wifi_eth_debug >= WIFI_DEBUG)
	{
	printk("will return NULL!\n");
	}
	return NULL;
 }
 struct wifi_sta_tbl *wifi_sta_tbl_add(const  char*s)
 {
	 if(NULL == s){
		return NULL;
	 }
	 unsigned char index = 0;
	 struct wifi_sta_tbl *sta = NULL;
	 index = wifi_tbl_mac_key(s);
	 if(index >= STA_HASH_SIZE){
		return NULL;
	 }
	 sta = wifi_sta_tbl_get(s);
	 if(sta != NULL)
	 {
		 if(wifi_eth_debug >= WIFI_WARNING)
			 printk("The sta: %02X:%02X:%02X:%02X:%02X:%02X: already in the sta list!\n",\
			 s[0],s[1],s[2],s[3],s[4],s[5]);
		 return sta;
	 }
	 
	 sta = (struct wifi_sta_tbl *)kmalloc(sizeof(struct wifi_sta_tbl),GFP_ATOMIC);
	 if(sta == NULL)
	 {
		 return NULL;
	 }
	 memset(sta,0,sizeof(struct wifi_sta_tbl));
	 memcpy(sta->STAMAC,s, MAC_LEN);
	 sta->next = wifi_sta_hash[index];
	 wifi_sta_hash[index] = sta;
	 return sta;
 }
 
 int  wifi_sta_tbl_del(char *s)
 {
	 if(NULL == s){
		return NULL;
	 }
	 unsigned char index = 0 ;
	 struct wifi_sta_tbl *pre_sta = NULL;
	 struct wifi_sta_tbl *next_sta = NULL;
	 index = wifi_tbl_mac_key(s);
 	 if(index >= STA_HASH_SIZE){
		return NULL;
	 }
	 pre_sta = wifi_sta_hash[index];
	 next_sta = pre_sta;
	 while(next_sta!=NULL){
		 if(memcmp(next_sta->STAMAC,s,MAC_LEN) == 0)
		 {
			 if(next_sta == pre_sta)
			 {
				 wifi_sta_hash[index] = next_sta->next;
				 kfree(next_sta);
				 return 1; 
			 }
			 else
			 {
				 pre_sta->next = next_sta->next;
				 kfree(next_sta);
				 return 1; 
			 }
		 }
		 pre_sta = next_sta;
		 next_sta = next_sta->next;
	 }
		 if(wifi_eth_debug >= WIFI_DEBUG)
			 printk( "Could not remove STA : %02X:%02X:%02X:%02X:%02X:%02X.  from "
				"list.the wifi sta list has no this sta!", s[0],s[1],s[2],s[3],s[4],s[5]);
 
	 return 0;
 }


int dynamic_update_if(struct interface_INFO *if_info)
{
	unsigned int index = 0, vrid = 0;
	struct net_device *dev = NULL;
	struct wifi_bss_tbl *bss = NULL;
	int len = 0;
	unsigned char *data = NULL;
	if(NULL == if_info)
	{
		return -1;
	}

	vrid = if_info->vrid;
	index = 256 + if_info->BSSIndex;
	if(wifi_eth_debug >= WIFI_DEBUG)
	{
		printk("dynamic_update_if: vrid = %d, BSSIndex=%x\n", vrid, if_info->BSSIndex);
		printk("dynamic_update_if if_policy %d if_name %s.\n",if_info->if_policy,if_info->ifname);	
	}
	
	if(wifi_device[vrid][index])
	{
		dev = wifi_device[vrid][index];
		wifi_dev_private_t* priv = (wifi_dev_private_t*)dev->priv;
		memset(priv->acmac,0, MAC_LEN);
		memset(priv->apmac,0, MAC_LEN);
		memset(priv->bssid,0, MAC_LEN);
		memset(priv->ifname,0,ETH_LEN);
		memcpy(priv->acmac,if_info->acmac, MAC_LEN);
		memcpy(priv->apmac,if_info->apmac, MAC_LEN);
		memcpy(priv->bssid,if_info->bssid, MAC_LEN);
		memcpy(priv->ifname,if_info->ifname,strlen(if_info->ifname));
		data = (unsigned char *)priv->res.data;
		memset(data, 0, 256);
		priv->res.module_type = DHCP_OPTION82_KMOD;
		priv->res.len = 0;
		#if 0
		data[0] = 0x01;
		len = strlen(if_info->apname) + 1 + strlen(if_info->essid) + 1 + 1;
		data[1] = len;
		memcpy(data+2, if_info->apname, strlen(if_info->apname));
		memcpy(data+2+strlen(if_info->apname)+1,if_info->essid, strlen(if_info->essid));
		data[len+1] = if_info->protect_type;
		data[len+2] = 0x02;
		data[len+3] = 17;
		priv->res.len = 2+len+2+17;
		#else
		len = 2 + strlen(if_info->apname) + 1 + strlen(if_info->essid);
		*data = 0x2;
		*(data + 1) = len - 2;
		printk("DHCP Option82 debug : if_info->apname %s , if_info->essid %s .\n", if_info->apname, if_info->essid);
		sprintf(data + 2, "%s:%s", if_info->apname, if_info->essid);
		/*
		*(data + len) = 0x2;
		*(data + len + 1) = len - 2;
		sprintf(data + len + 2, "%s:%s", if_info->apname, if_info->essid);
		*/
		priv->res.len = len;
		#endif
		priv->acip = if_info->acip;
		priv->apip = if_info->apip;
		memcpy(priv->acipv6,if_info->acipv6,IPv6_LEN);
		memcpy(priv->apipv6,if_info->apipv6,IPv6_LEN);
		unsigned char *p =  priv->acipv6;
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("AC ip %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",\
		p[0], p[1], p[2], p[3],p[4], p[5], p[6], p[7],p[8], p[9], p[10], p[11],p[12], p[13], p[14], p[15]);
		p =  priv->apipv6;
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("AP ip  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",\
		p[0], p[1], p[2], p[3],p[4], p[5], p[6], p[7],p[8], p[9], p[10], p[11],p[12], p[13], p[14], p[15]);
		
		priv->isIPv6 = if_info->isIPv6;
		priv->acport = if_info->acport;
		priv->apport = if_info->apport;
		priv->WLANID = if_info->WLANID;
		priv->protect_type = if_info->protect_type;
		priv->wsmswitch = if_info->wsmswitch;
		priv->first = 1;
		priv->Eap1XServerSwitch = if_info->Eap1XServerSwitch;
		priv->vlanid = if_info->vlanid;		
		priv->vlanSwitch = if_info->vlanSwitch;
		memset(priv->Eap1XServerMac,0,MAC_LEN);
		memcpy(priv->Eap1XServerMac,if_info->Eap1XServerMac,MAC_LEN);
		priv->f802_3 = if_info->f802_3;
		if(wifi_eth_debug >= WIFI_DEBUG){
			printk("dynamic_update_if: priv->f802_3 %d.\n",priv->f802_3);
     		printk("dynamic_update_if: priv->wsmswitch %d priv->vlanid %d priv->vlanSwitch %s\n",priv->wsmswitch,priv->vlanid,priv->vlanSwitch);
		}
		bss = wifi_bssid_bssidx_tbl_add(if_info->bssid);
		if(bss){
			memcpy(bss->BSSID,if_info->bssid, MAC_LEN);
			bss->BSSIndex = if_info->BSSIndex;
			bss->vrid = vrid;
			bss->dev_index = index;
			bss->vlanid = if_info->vlanid;			
			bss->vlanSwitch = if_info->vlanSwitch;
			memcpy(&(bss->priv),priv,sizeof(wifi_dev_private_t));
			if(wifi_eth_debug >= WIFI_DEBUG)
				printk("dynamic_update_if bss if_name %s. bss->vlanid %d ,bss->vlanSwitch %d\n",bss->priv.ifname,bss->vlanid ,bss->vlanSwitch); 		
			bss->Eap1XServerSwitch = if_info->Eap1XServerSwitch;
			memset(bss->Eap1XServerMac,0,MAC_LEN);
			memcpy(bss->Eap1XServerMac,if_info->Eap1XServerMac,MAC_LEN);
			if(wifi_eth_debug >= WIFI_DEBUG){
				printk("bssindex=%d,1xserverSwitch=%d,1xServerMac:%02X:%02X:%02X:%02X:%02X:%02X.\n",bss->BSSIndex,bss->Eap1XServerSwitch,\
					bss->Eap1XServerMac[0],bss->Eap1XServerMac[1],bss->Eap1XServerMac[2],bss->Eap1XServerMac[3],bss->Eap1XServerMac[4],bss->Eap1XServerMac[5]);
				printk("dynamic_update_if:wifi_bssid_bssidx_tbl_add successful\n"); 		
			}

		}else
			printk("dynamic_update_if:wifi_bssid_bssidx_tbl_add failed\n");			
	}
	else if(if_info->if_policy == 0){
		bss = wifi_bssid_bssidx_tbl_add(if_info->bssid);
		if(bss){
			memcpy(bss->BSSID,if_info->bssid, MAC_LEN);
			bss->BSSIndex = if_info->BSSIndex;
			bss->vrid = vrid;
			bss->dev_index = index;
			wifi_dev_private_t* priv = (wifi_dev_private_t*)&(bss->priv);
			memset(priv->acmac,0, MAC_LEN);
			memset(priv->apmac,0, MAC_LEN);
			memset(priv->bssid,0, MAC_LEN);
			memset(priv->ifname,0,ETH_LEN);
			memcpy(priv->acmac,if_info->acmac, MAC_LEN);
			memcpy(priv->apmac,if_info->apmac, MAC_LEN);
			memcpy(priv->bssid,if_info->bssid, MAC_LEN);
			memcpy(priv->ifname,if_info->ifname,strlen(if_info->ifname));
			priv->acip = if_info->acip;
			priv->apip = if_info->apip;
			priv->acport = if_info->acport;
			priv->apport = if_info->apport;
			memcpy(priv->acipv6,if_info->acipv6,IPv6_LEN);
			memcpy(priv->apipv6,if_info->apipv6,IPv6_LEN);
			priv->isIPv6 = if_info->isIPv6;
			priv->WLANID = if_info->WLANID;
			priv->protect_type = if_info->protect_type;
			priv->wsmswitch = if_info->wsmswitch;
			priv->first = 1;
			priv->BSSIndex = if_info->BSSIndex;
			if(wifi_eth_debug >= WIFI_DEBUG)
			{
				printk("dynamic_update_if:wifi_bssid_bssidx_tbl_add successful.\n");			
				printk("dynamic_update_if bss if_name %s.\n",priv->ifname); 	
			}
		}else{
			printk("dynamic_update_if:wifi_bssid_bssidx_tbl_add failed.\n");			
			return -1;
		}
	}else
		return -1;
	return 0;
}



/**
* Description:
*  Get IPv6 addr list by interface name.  By guoxb@2010-01-08
*
* Parameter:
*  ipv6_addr: 
*       ifname: interface name.
*       ifindex: interface index.
*       stat: 0, Link down; 1: Link up
*       addr_cnt: IPv6 Addr count
*       addr: IPv6 addr array.
*
*  Return:
*   -1 : ioctl Failed. 0: ioctl successed.
*
*/
int get_ipv6_addr(dev_ipv6_addr_t *ipv6_addr)
{
	struct net_device *dev = NULL;
	struct inet6_dev *idev = NULL;
	struct inet6_ifaddr *ifa= NULL, *tmp = NULL;

	if(NULL == ipv6_addr){
		return 0;/*0 or -1 ? */
	}
	
	ipv6_addr->addr_cnt = 0;
	ipv6_addr->ifindex = 0;
	ipv6_addr->addr_cnt = 0;
	ipv6_addr->stat = LINK_DOWN;

	rtnl_lock();
	if ((dev = dev_get_by_name(&init_net, ipv6_addr->ifname)) == NULL)
	{
		rtnl_unlock();
		return 0;
	}

	
	ipv6_addr->addr_cnt = 0;
	ipv6_addr->ifindex = dev->ifindex;
	/*
		dev->flags & IFF_UP : interface is up
		netif_running: interface is up & cable is linked.
	*/
	if (!(dev->flags & IFF_UP))
	{
		dev_put(dev);
		rtnl_unlock();
		return 0;
	}
	ipv6_addr->stat = LINK_UP;

	idev = (struct inet6_dev*)dev->ip6_ptr;
	if (!idev)
	{
		dev_put(dev);
		rtnl_unlock();
		return 0;
	}
	
	in6_dev_hold(idev);
	ifa = idev->addr_list;
	if (!ifa)
	{
		in6_dev_put(idev);
		dev_put(dev);
		rtnl_unlock();
		return 0;
	}
	in6_ifa_hold(ifa);
	tmp = ifa;

	while (tmp != NULL)
	{
		if (unlikely(ipv6_addr->addr_cnt >= MAX_IPV6_ADDR_PER_DEV))
		{
			in6_ifa_put(ifa);
			in6_dev_put(idev);
			dev_put(dev);
			rtnl_unlock();
			printk("wifi: IOCTL addr count is larger than MAX_ADDR_CNT.\n");
			
			return -1;
		}
		
		memcpy(&ipv6_addr->addr[ipv6_addr->addr_cnt], tmp->addr.s6_addr, 16);
		ipv6_addr->addr_cnt++;
		tmp = tmp->if_next;
	}

	in6_ifa_put(ifa);
	in6_dev_put(idev);
	dev_put(dev);
	rtnl_unlock();

	return 0;
}

