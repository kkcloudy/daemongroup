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
* wifi.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi dev operation
*
********************************************************************************
*
* MODIFY:     revision <1.0>  luoxun@autelan.com
* 2008-8-13	Mapping memory to wsm.
* 2008-8-25	ping, DHCP
* 2008-8-28	Shared memory lock was replaced by queue.
* 2008-9-10	Thread was binded to core.	
* 2008-9-12	Queue was binded to core.	
* MODIFY:     revision <1.1>
* 2009-11-16 <guoxb> Add multi-instance for AC N+1 backup.
* 2010-01-08 <guoxb> Add ioctl WIFI_IOC_GET_V6ADDR for wid get IPv6 addr by 
*                              interface name.
*                                                  
*******************************************************************************/


#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/netdevice.h>

#include "wifi.h"
#include "wifi-ioctl.h"
#include "wifi-rx.h"
#include "wifi-tx.h"
#include <net/sock.h>
#include <linux/netlink.h>

#include "../../../../kernel2.6.32.27cn/include/linux/kthread.h"
/*netlink to asd,ht add 110226*/
#define NETLINK_WITH_ASD 27
//u32 asd_pid = 0;
struct sock *nl_sk = NULL;
struct task_struct *my_netlink_test;
struct HANSI_INFO hansiInfo[VRID_MAX];

int wifi_major_num = 243;
module_param(wifi_major_num, int, 0444);
int wifi_minor_num = 0;
module_param(wifi_minor_num,int,0444);
int wifi_eth_debug = 0;
module_param(wifi_eth_debug,int,0644);
int wifi_sleep_gap = 2;
module_param(wifi_sleep_gap,int,0644);
unsigned int wifi_tx_switch = 0;//0--disable;1--enable
module_param(wifi_tx_switch,int,0644);
unsigned int wifi_rx_switch = 0;//0--disable;1--enable
module_param(wifi_rx_switch,int,0644);
unsigned short wifi_8021q_type = 0x8100;//default 0x8100
module_param(wifi_8021q_type,short,0644);
unsigned int wifi_ipv6_dr_sw = 1;//0--disable;1--enable
module_param(wifi_ipv6_dr_sw,int,0644);

#define DRIVER_NAME "wifi"
#define WIFI_SDK_VERSION_STRING "wifi_1.1"

extern int dynamic_registe_if(struct interface_INFO *if_info);
extern int dynamic_unregiste_if(struct interface_INFO *if_info);
extern void wifi_tasklet_rx(void);
extern void wifi_inic_vm(void);
extern void wifi_cleanup_vm(void);
extern void wifi_get_core(void);
extern int (*wifi_ko_rx_hook)(struct sk_buff *);
extern int (*wifi_ko_rx_hook6)(struct sk_buff *);
void *nl_data_ready (struct sk_buff *skb);	/*wuwl modify to skb for new kernel*/
void netlink_with_asd(void);

struct net_device * wifi_device[VRID_MAX][WIFI_INTERFACE_NUM];
struct tasklet_struct wifi_tasklet[NR_CPUS];
struct wifi_bss_tbl *wifi_bss_hash[BSS_HASH_SIZE];
struct wifi_sta_tbl *wifi_sta_hash[STA_HASH_SIZE];
unsigned long long vm[VRID_MAX] = {0};

struct oct_dev_s {
	long quantum;
	long qset;
	unsigned long size;
	unsigned long access_key;
	struct cdev cdev;
};

static struct oct_dev_s wifi_dev;



int wifi_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
 	{
	int retval = 0;
	int op_ret = 0;
	unsigned int type = 0;
	//unsigned int instId = 0;
	struct interface_INFO if_info;
	struct interface_basic_INFO if_basic_info;
	struct interface_batch_INFO if_batch_info;
	struct HANSI_INFO HInfo;

	sh_mem_t sh_mem;
	dev_ipv6_addr_t ipv6_addr;
	struct wifi_bss_tbl *bss = NULL;
	struct asd_to_wifi_sta stainfo;
	struct wifi_sta_tbl *sta = NULL;
	int ret = 0;
	int i = 0;
	if (_IOC_TYPE(cmd) != WIFI_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > WIFI_IOC_MAXNR) return -ENOTTY;

	memset(&ipv6_addr, 0, sizeof(dev_ipv6_addr_t));
	memset(&if_info, 0, sizeof(if_info));
	memset(&sh_mem, 0, sizeof(sh_mem_t));
	memset(&stainfo,0,sizeof(stainfo));
	memset(&if_basic_info,0,sizeof(if_basic_info));
	memset(&if_batch_info,0,sizeof(if_batch_info));
	memset(&HInfo,0,sizeof(HInfo));
	switch (cmd)
	{
		case WIFI_IOC_REG_IF: 
			op_ret = copy_from_user(&if_basic_info, (struct interface_basic_INFO *)arg, sizeof(struct interface_basic_INFO));
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			retval = dynamic_registe_if(&if_basic_info);
			break;
			
		case WIFI_IOC_UNREG_IF: 
			op_ret = copy_from_user(&if_basic_info, (struct interface_basic_INFO *)arg, sizeof(struct interface_basic_INFO));
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			retval = dynamic_unregiste_if(&if_basic_info);
			break;
			
		case WIFI_IOC_MMAP:
			op_ret = copy_to_user((unsigned long long *)arg, &vm, 8);
			break;
			
		case WIFI_IOC_VRRP_MMAP:
			op_ret = copy_from_user(&sh_mem, (sh_mem_t *)arg, sizeof(sh_mem_t));
			if (sh_mem.vrid >= VRID_MAX)
			{
				printk("wifi: ioctl get share memory vrid failed, vrid = %d\n", sh_mem.vrid);
				retval = -1;
				break;
			}
			sh_mem.mm_addr = vm[sh_mem.vrid];
			op_ret = copy_to_user((sh_mem_t*)arg, &sh_mem, sizeof(sh_mem_t));
			printk("wifi: vrid = %d get mmap addr=%llu.\n", sh_mem.vrid,sh_mem.mm_addr);
			break;

		case WIFI_IOC_GET_V6ADDR:
			op_ret = copy_from_user(&ipv6_addr, (dev_ipv6_addr_t *)arg, sizeof(dev_ipv6_addr_t));
			ipv6_addr.ifname[15] = 0;
			retval = get_ipv6_addr(&ipv6_addr);
			op_ret = copy_to_user((dev_ipv6_addr_t*)arg, &ipv6_addr, sizeof(dev_ipv6_addr_t));
			
			break;
		case WIFI_IOC_UPDATE_IF:
			op_ret = copy_from_user(&if_info, (struct interface_INFO *)arg, sizeof(struct interface_INFO));
			if(wifi_eth_debug >= WIFI_DEBUG)
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			retval = dynamic_update_if(&if_info);
			break;
		case WIFI_IOC_WSM_SWITCH:
			op_ret = copy_from_user(&type, (unsigned int *)arg, sizeof(unsigned int));
			if(wifi_eth_debug >= WIFI_DEBUG)
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			wsmswitch = type;
			printk("type %d\n",type);
			break;			
		case WIFI_IOC_ASD_SWITCH:
			op_ret = copy_from_user(&type, (unsigned int *)arg, sizeof(unsigned int));
			if(wifi_eth_debug >= WIFI_DEBUG)
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			asdswitch = type;
			printk("asdswitch type %d\n",type);
			break;			
		case WIFI_IOC_HANSISTATE_UPDATE:
			op_ret = copy_from_user(&HInfo, (struct HANSI_INFO *)arg, sizeof(struct HANSI_INFO));
			if(wifi_eth_debug >= WIFI_DEBUG)
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			if((HInfo.instId >= VRID_MAX)||(HInfo.instId < 0)){
				printk("<wifi err>invalid instID %d\n",HInfo.instId);
				retval = -1;
			}else{
				hansiInfo[HInfo.instId].hpre_state = hansiInfo[HInfo.instId].hstate;
				hansiInfo[HInfo.instId].hstate = HInfo.hstate;
				hansiInfo[HInfo.instId].vlanSwitch = HInfo.vlanSwitch;	
				hansiInfo[HInfo.instId].dhcpoption82 = HInfo.dhcpoption82;
				printk("update,hansiInfo[%d].hstate %d,hpre_state=%d.hansiInfo[HInfo.instId].vlanSwitch %d\n",HInfo.instId,hansiInfo[HInfo.instId].hstate,hansiInfo[HInfo.instId].hpre_state,hansiInfo[HInfo.instId].vlanSwitch);
			}
			break;	
		case WIFI_IOC_ASD_THRED_ID:
			op_ret = copy_from_user(&HInfo, (struct HANSI_INFO *)arg, sizeof(struct HANSI_INFO));
			if((HInfo.instId >= VRID_MAX)||(HInfo.instId < 0)){
				printk("<wifi err>invalid instID %d\n",HInfo.instId);
				retval = -1;
			}else{
				hansiInfo[HInfo.instId].asd_pid = HInfo.asd_pid;
				if(wifi_eth_debug >= WIFI_DEBUG)
					printk("hansiInfo[%d].asd_pid=%d.\n",HInfo.instId,hansiInfo[HInfo.instId].asd_pid);
			}
			break;
		
		case WIFI_IOC_ADD_STA:
			op_ret = copy_from_user(&stainfo,(struct asd_to_wifi_sta*)arg,sizeof( struct asd_to_wifi_sta));
			bss = wifi_bssid_bssidx_tbl_get(stainfo.BSSID);
			if(bss == NULL)
			{
				if(wifi_eth_debug >= WIFI_WARNING)
					printk("in case : WIFI_IOC_ADD_STA, bss is NULL!\n");
				break;
			}
			bss->roaming_flag = stainfo.roaming_flag;
			sta = wifi_sta_tbl_add(stainfo.STAMAC);
			if(sta == NULL)
			{
				printk("sta add error!\n");
				retval = -1;
				break;
			}
			memcpy(sta->BSSID_Before,stainfo.BSSID_Before,MAC_LEN);
			memcpy(sta->BSSID,stainfo.BSSID,MAC_LEN);
			sta->roaming_flag = stainfo.roaming_flag;
			break;
		case WIFI_IOC_DEL_STA:
			op_ret = copy_from_user(&stainfo,(struct asd_to_wifi_sta*)arg,sizeof( struct asd_to_wifi_sta));
			bss = wifi_bssid_bssidx_tbl_get(stainfo.BSSID);
			if(bss == NULL)
			{
				if(wifi_eth_debug >= WIFI_WARNING)
					printk("in case : WIFI_IOC_DEL_STA, bss is NULL!\n");
			}
			ret =  wifi_sta_tbl_del( stainfo.STAMAC);
			if(ret == 0){
				if(wifi_eth_debug >= WIFI_WARNING)
					printk("in case : WIFI_IOC_DEL_STA, del sta error!\n");
			}
			break;
		case WIFI_IOC_BATCH_REG_IF:			
			op_ret = copy_from_user(&if_batch_info, (struct interface_batch_INFO *)arg, sizeof(struct interface_batch_INFO));
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			for(i = 0;i < if_batch_info.count && i < PATCH_OP_RADIO_MAX; i++)
				retval = dynamic_registe_if(&(if_batch_info.ifinfo[i]));
			break;
		case WIFI_IOC_BATCH_UNREG_IF: 		
			op_ret = copy_from_user(&if_batch_info, (struct interface_batch_INFO *)arg, sizeof(struct interface_batch_INFO));
			printk("file-%s,funtion-%s,line-%d\n",__FILE__,__FUNCTION__,__LINE__);
			for(i = 0;i < if_batch_info.count && i < PATCH_OP_RADIO_MAX; i++)
				retval = dynamic_unregiste_if(&(if_batch_info.ifinfo[i]));
			break;
		default:
			printk("wifi_ioctl break\n");
			retval = -1;
			break;
	}
	
	return retval;
}
/************************************************************************
*
* wifi_mmap: The wifi mmap() implementation
*/
static int wifi_mmap(struct file * file, struct vm_area_struct *vma)
{
  unsigned long   phys;
//  unsigned long   ppNum;
//  unsigned long   offset;
  unsigned long   pageSize;
//  struct pci_dev  *dev;
//  struct quirks   *quirks;
  unsigned int vrid = (vma->vm_start - 0x70000000)/_4MB;
 	printk("fun:wifi_mmap,%s,vrid=%d,vma->vm_start=%#lx.\n",__func__,vrid,vma->vm_start);
  if (((vma->vm_pgoff)<<PAGE_SHIFT) & (PAGE_SIZE-1))
  {
	  /* need aligned offsets */				
	  printk("wifi _mmap offset not aligned\n");
	  return -ENXIO;  
  }
  //phys = 0x70000000;
 //phys = 0xf4c00000;
  phys = vm[vrid];
  pageSize = _4MB;
  	printk("wifi MMAP:vma start %#lx pageoffset %#lx\r\n",vma->vm_start,vma->vm_pgoff);
  	printk("wifi MMAP:remap_pfn_range(0x%X, 0x%X, 0x%X, 0x%X)\n",
			  (int)(vma->vm_start), (int)(phys >> PAGE_SHIFT),
			  (int)pageSize, (int)pgprot_val(vma->vm_page_prot));
  if (remap_pfn_range(vma, vma->vm_start, phys >> PAGE_SHIFT, pageSize,
					  vma->vm_page_prot))
  {
	  printk("wifi remap_pfn_range failed\n");
	  return 1;
  }
  return 0;
}

  
  static long wifi_compat_ioctl(struct file *filp,unsigned int cmd, unsigned long arg) 
  {
	  int rval;
  
 //   lock_kernel();
	  rval = wifi_ioctl(filp->f_dentry->d_inode, filp, cmd, arg);
 //   unlock_kernel();
  
	  return (rval == -EINVAL) ? -ENOIOCTLCMD : rval;
  }


 static int wifi_open(struct inode *inode, struct file *filp)
 	{
	struct oct_dev_s *dev;

	dev = container_of(inode->i_cdev, struct oct_dev_s, cdev);
	filp->private_data = dev;
	dev->size = 0;
	return 0;
	}
 
 
 static int wifi_release(struct inode *inode, struct file *file)
 {
	 return 0;
 }
 
static struct file_operations wifi_fops = 
{
	.owner	= THIS_MODULE,
	.compat_ioctl = wifi_compat_ioctl,
	.ioctl	= wifi_ioctl,
	.mmap   = wifi_mmap,
	.open	= wifi_open,
	.release= wifi_release
};



int wifi_dev_init(void)
 	
 {
	 int result;
	 dev_t wifi_devt;
 
	 wifi_devt = MKDEV(wifi_major_num, wifi_minor_num);
	 result = register_chrdev_region(wifi_devt,1,DRIVER_NAME);
	 printk(KERN_INFO DRIVER_NAME ":register major dev [%d]\n",MAJOR(wifi_devt));
 
	 if (result < 0) {
		 printk(KERN_WARNING DRIVER_NAME ":register_chrdev_region err[%d]\n",result);
		 return 0;
	 } 
	 cdev_init(&(wifi_dev.cdev),&wifi_fops);
	 wifi_dev.cdev.owner = THIS_MODULE;
	 
	 result = cdev_add(&(wifi_dev.cdev),wifi_devt,1);
	 if (result < 0) {
		 printk(KERN_WARNING DRIVER_NAME ":cdev_add err[%d]\n",result);
	 } else {
		 printk(KERN_INFO DRIVER_NAME ":loaded successfully.\n");
	 }
	 return 0;
 }


int wifi_dev_cleanup(void)
	{
		printk(KERN_INFO DRIVER_NAME ":Unregister MajorNum[%d]\n",MAJOR(wifi_dev.cdev.dev));	
		unregister_chrdev_region(wifi_dev.cdev.dev,1);	
		cdev_del(&(wifi_dev.cdev));
		printk(KERN_INFO DRIVER_NAME ":unloaded\n");
		return 0;
	}


static void wifi_rx_initialize(void)
{
    int i;
    /* Initialize all of the tasklets */
    for (i=0; i<NR_CPUS; i++)
        tasklet_init((wifi_tasklet + i), (void *)wifi_tasklet_rx, 0);
}

static void wifi_rx_shutdown(void)
{
    int i;
    /* Shutdown all of the tasklets */
    for (i=0; i<NR_CPUS; i++)
        tasklet_kill(wifi_tasklet + i);
}



/**
 * Module/ driver initialization. Creates the linux network
 * devices.
 *
 * @return Zero on success
 */
static int __init wifi_init_module(void)
{
	printk("wifi-ethernet: %s\n", WIFI_SDK_VERSION_STRING);
	memset(wifi_device, 0, sizeof(int *) * WIFI_INTERFACE_NUM * VRID_MAX);
	memset(wifi_bss_hash, 0, sizeof(struct wifi_bss_tbl *) * BSS_HASH_SIZE);	
	memset(wifi_sta_hash, 0, sizeof(struct wifi_sta_tbl *) * STA_HASH_SIZE);	
	memset(&hansiInfo,0,sizeof(hansiInfo));
	wifi_ko_rx_hook = wifi_kernel_rx;
	wifi_ko_rx_hook6 = wifi_kernel_rx;
#ifdef __wsm__
	wifi_rx_initialize();
#endif
	wifi_dev_init();
#ifdef __wsm__
	wifi_inic_vm();
#else
	wifi_get_core();
#endif
	/*netlink to asd*/
	printk("kthread_create\n");
	
	//my_netlink_test = (struct task_struct *)kthread_create(netlink_with_asd, NULL, "%s", "test_netlink");
	nl_sk = netlink_kernel_create(&init_net, NETLINK_WITH_ASD, 0,handle_data_msg, NULL, THIS_MODULE);/*creat netlink socket in kernel*/
	if(nl_sk == NULL){
		printk("netlink_kernel_create failed,NETLINK_WITH_ASD=%d.\n",NETLINK_WITH_ASD);
	}else{
		printk("netlink_kernel_create sucessful,NETLINK_WITH_ASD=%d.\n",NETLINK_WITH_ASD);
	}
	/*if (IS_ERR(my_netlink_test))
	{
		printk("kthread_create failed\n");
	}

	if (my_netlink_test)
	{
		printk("kthread_create successful\n");
		wake_up_process(my_netlink_test);
	}*/		


	return 0;
}


/**
 * Module / driver shutdown
 *
 * @return Zero on success
 */
static void __exit wifi_cleanup_module(void)
{
	unsigned int port = 0, vrid = 0;

	wifi_rx_shutdown();

	/* Free the ethernet devices */
	for (vrid = 0; vrid < VRID_MAX; vrid++)
	{
		for (port = 0; port < WIFI_INTERFACE_NUM; port++)
		{
			if (wifi_device[vrid][port])
			{
				unregister_netdev(wifi_device[vrid][port]);
				kfree(wifi_device[vrid][port]);
				wifi_device[vrid][port] = NULL;
			}
		}
	}
	
	wifi_cleanup_vm();
	wifi_dev_cleanup();
	if(nl_sk != NULL){
		sock_release(nl_sk->sk_socket);
	}
}
void CWCaptrue_wifi(int n ,unsigned char *buffer){
		int t=0;
		while((n-t)>=16)
		{
			int i;
			printk("[");
			for(i=0;i<16;i++)
				printk("%02x ",buffer[t+i]);
			printk("]\t[");
			for(i=0;i<16;i++)
			{
				char ch=buffer[t+i];
				//if(isalnum(ch))
					printk("%c",ch);
				//else
				//	printk(".");
			}
			printk("]\n");
			t+=16;
		}

		if(n>t)
		{
			int i=t;
			printk("[");
			while(i<n)
				printk("%02x ",buffer[i++]);
			printk("]");
			i=n-t;
			i=16-i;
			while(i--)
				printk("   ");
			printk("\t[");
			i=t;
			while(i<n)
			{
				char ch=buffer[i++];
				//if(isalnum(ch))
					printk("%c",ch);
				//else
				//	printk(".");
			}
			printk("]\n");
		}
		printk("\n\n");
}

//test netlink begin
int kernel_to_asd(struct sk_buff *skb,unsigned int vrid)
{
	struct nlmsghdr *nlh = NULL;
	unsigned int asd_pid = 0;
	nlh = (struct nlmsghdr *)(skb->data);
	memset(nlh,0,sizeof(struct nlmsghdr));
	nlh->nlmsg_len = skb->len;
	NETLINK_CB(skb).pid = 0; /* from kernel */
	//NETLINK_CB(skb).dst_pid = asd_pid;
	NETLINK_CB(skb).dst_group = 0;
	asd_pid = hansiInfo[vrid].asd_pid;
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s skb->len:%d,nl_sk=%p,hansiInfo[%d].asd_pid=%d,skb->data=%p.\n",__func__,skb->len,nl_sk,vrid,asd_pid,skb->data);
	if((asd_pid <= 0) || (nl_sk == NULL)){
		printk("<error>wifi:%s skb->len:%d,nl_sk=%p,hansiInfo[%d].asd_pid=%d,skb->data=%p.\n",__func__,skb->len,nl_sk,vrid,asd_pid,skb->data);
		kfree_skb(skb);
		return -1;
	}
	netlink_unicast(nl_sk, skb, asd_pid, MSG_DONTWAIT);
	return 0;
}

int handle_data_msg(struct sk_buff *skb)
{
	unsigned char BSSID[MAC_LEN] = {0};
	struct sk_buff *skb2 = NULL;
	wifi_dev_private_t *priv = NULL;
	struct nlmsghdr * nlh = NULL;
	struct ieee80211_frame *hdr = NULL;
	struct wifi_bss_tbl *bss = NULL;

	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s skb->len %d.\n",__func__,skb->len);
	//if(skb->cvm_info.cookie != CVM_COOKIE)//to judged this pkt is network Card's packet 
	{									  //if not the new skb
		nlh = (struct nlmsghdr *)skb->data;
		if(nlh->nlmsg_len <  NLMSG_LENGTH(0)+sizeof(DataMsgHead))
			return 0;
		skb_pull(skb, NLMSG_LENGTH(0));	/*del nlmsghdr*/
		skb_pull(skb, sizeof(DataMsgHead));	/*del DataMsgHead*/
		skb2 = dev_alloc_skb(skb->len + 256);
		if (!skb2){
			printk("wifi.ko failed to allocate skbuff, packet dropped\n");
			//kfree_skb(skb);
			return 0;
		}	
	
		skb_reserve(skb2,256);
		memcpy(skb_put(skb2, skb->len), skb->data, skb->len);	
		//kfree_skb(skb);
		//skb2->dst = NULL;
	}			


//	skb_pull(skb2, sizeof(struct nlmsghdr));	/*del nlmsghdr*/
//	skb_pull(skb2, sizeof(DataMsgHead));	/*del DataMsgHead*/

	hdr = (struct ieee80211_frame *)(skb2->data);

	//BSSID = hdr->i_addr3;
	memcpy(BSSID,hdr->i_addr3,MAC_LEN);
	bss = wifi_bssid_bssidx_tbl_get(BSSID);
	if(wifi_eth_debug >= WIFI_DEBUG)
		printk("%s BSSID %02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,BSSID[0],BSSID[1],BSSID[2],BSSID[3],BSSID[4],BSSID[5]);
	if(!bss){
		kfree_skb(skb2);		
		return 0;
	}	
	priv = (wifi_dev_private_t*)&(bss->priv);

	wifi_assemble_capwap(skb2, priv,0);
	if(WIFI_ERR == wifi_assemble_udp_ip(skb2, priv))
	{
		kfree_skb(skb2);
		priv->stats.tx_dropped++;
	}
	return 0;
}

void * nl_data_ready (struct sk_buff *skb)
{
	return;
}

void netlink_with_asd(void)
{
	struct sk_buff * skb = NULL;
	struct nlmsghdr * nlh = NULL;
	int err;
	unsigned char isFirst = 0;
	//nl_sk = netlink_kernel_create(NETLINK_WITH_ASD, 0,nl_data_ready,THIS_MODULE);/*creat netlink socket in kernel*/
	nl_sk = netlink_kernel_create(&init_net, NETLINK_WITH_ASD, 0,nl_data_ready, NULL, THIS_MODULE);/*creat netlink socket in kernel*/
	if(nl_sk == NULL)
	{
		printk("netlink_kernel_create failed.\n");
		return;
	}else{
		printk("netlink_kernel_create successfull,nl_sk=%p,NETLINK_WITH_ASD=%d.\n",nl_sk,NETLINK_WITH_ASD);
	}
	printk("%s,&init_net=%p.\n",__func__,&init_net);
	/* wait for message coming down from user-space */
	while(1)
	{
		skb = skb_recv_datagram(nl_sk, 0, 0, &err);
		if(skb == NULL)
			continue;
		nlh = (struct nlmsghdr *)skb->data;
		//asd_pid = nlh->nlmsg_pid; /*pid of sending process */	
		if(isFirst == 0){
			printk("wifi Received first netlink message len:%d.\n", nlh->nlmsg_len);
		}
		isFirst = 1;
		if(wifi_eth_debug >= WIFI_DEBUG)
			printk("Received netlink message len:%d\n", nlh->nlmsg_len);
		
		handle_data_msg(skb);

	}
	
	//asd_pid = 0;
	nlh = NULL;	
	sock_release(nl_sk->sk_socket);
}

//test netlink end


MODULE_LICENSE("Autelan");
MODULE_AUTHOR("Autelan");
MODULE_DESCRIPTION("Wifi ethernet driver.");
module_init(wifi_init_module);
module_exit(wifi_cleanup_module);




