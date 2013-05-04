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
* wifi-common.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi-ethernet module common function
*
*
*******************************************************************************/

#include <linux/kernel.h>
#include <net/dst.h>
//#include <cvmx-app-init.h>
#include <../../../../kernel2.6.32.27cn/arch/mips/include/asm/octeon/cvmx-app-init.h>

#include "wifi.h"

extern cvmx_bootinfo_t *octeon_bootinfo;
extern int wifi_xmit(struct sk_buff *skb, struct net_device *dev);
extern int wifi_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);


/**
 * Get the wifi-ethernet statistics
 *
 * @param dev    Device to get the statistics from
 * @return Pointer to the statistics
 */
static struct net_device_stats *wifi_common_get_stats(struct net_device *dev)
{
	wifi_dev_private_t * priv = (wifi_dev_private_t *)dev->priv;
    return &(priv->stats);
}


/**
 * Set the MAC address for a device
 *
 * @param dev    Device to change the MAC address for
 * @param addr   Address structure to change it too. MAC address is addr + 2.
 * @return Zero on success
 */
static int wifi_common_set_mac_address(struct net_device *dev, void *addr)
{
    memcpy(dev->dev_addr, ( (unsigned char *)addr + 2 ), 6);
    return 0;
}


/**
 * Change the link MTU. 
 *
 * @param dev     Device to change
 * @param new_mtu The new MTU
 * @return Zero on success
 */
static int wifi_common_change_mtu(struct net_device *dev, int new_mtu)
{

    /* Limit the MTU to make sure the ethernet packets are between 64 bytes
        and 65535 bytes */
    if ((new_mtu + 14 + 4 < 64) || (new_mtu + 14 + 4 > 65392))
    {
        printk("MTU must be between %d and %d.\n", 64-14-4, 65392-14-4);
        return -EINVAL;
    }
    dev->mtu = new_mtu;
	return 0;
}

extern mac_addr_get(unsigned char * mac);
#if 0
/**
 * Per network device initialization
 *
 * @param dev    Device to initialize
 * @return Zero on success
 */
int wifi_common_init(struct net_device *dev)
{
	char tmp_mac[6] = {0};
    char mac[8] = {0};
	mac_addr_get(tmp_mac);
	
	mac[0] = 0x00;
	mac[1] = 0x00;
	mac[2] = tmp_mac[0];
	mac[3] = tmp_mac[1];
	mac[4] = tmp_mac[2];
	mac[5] = tmp_mac[3];
	mac[6] = tmp_mac[4];
	mac[7] = tmp_mac[5];
	
	dev->hard_start_xmit	= wifi_xmit;
    dev->get_stats          = wifi_common_get_stats;
    dev->weight             = 16;
    dev->set_mac_address    = wifi_common_set_mac_address;
    dev->set_multicast_list = NULL;
    dev->change_mtu         = wifi_common_change_mtu;

    dev->set_mac_address(dev, mac);
    dev->change_mtu(dev, dev->mtu);

    /* Zero out stats for port so we won't mistakenly show counters from the
        bootloader */
    memset(dev->get_stats(dev), 0, sizeof(struct net_device_stats));

    return 0;
}
#endif
 
 

void wifi_common_uninit(struct net_device *dev)
{/*
    cvm_oct_private_t *priv = (cvm_oct_private_t*)dev->priv;
    while (skb_queue_len(&priv->tx_free_list))
        dev_kfree_skb(skb_dequeue(&priv->tx_free_list));
*/}
	
static const struct net_device_ops wifi_netdev_ops = {
	.ndo_start_xmit 	 = wifi_xmit,
	.ndo_get_stats		= wifi_common_get_stats,
	.ndo_set_mac_address	 = wifi_common_set_mac_address,
	.ndo_set_multicast_list  = NULL,
	.ndo_change_mtu 	 = wifi_common_change_mtu,
	.ndo_uninit 		= wifi_common_uninit,
};

int wifi_netdev_ops_init(struct net_device *dev){
	dev->netdev_ops = &wifi_netdev_ops;
	//printk("%s,%d,dev=%p,dev->priv=%p.",__func__,__LINE__,dev,dev->priv);
	char tmp_mac[6] = {0};
	char mac[8] = {0};
	mac_addr_get(tmp_mac);
	
	mac[0] = 0x00;
	mac[1] = 0x00;
	mac[2] = tmp_mac[0];
	mac[3] = tmp_mac[1];
	mac[4] = tmp_mac[2];
	mac[5] = tmp_mac[3];
	mac[6] = tmp_mac[4];
	mac[7] = tmp_mac[5];
	
	dev->netdev_ops->ndo_set_mac_address(dev, mac);
	dev->netdev_ops->ndo_change_mtu(dev, dev->mtu);
	
	/* Zero out stats for port so we won't mistakenly show counters from the
		bootloader */
	memset(dev->netdev_ops->ndo_get_stats(dev), 0, sizeof(struct net_device_stats));
	//printk("%s,%d,dev=%p,dev->priv=%p.",__func__,__LINE__,dev,dev->priv);
	return 0;
}

