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
* wifi-sm.c
*
*
* CREATOR:
* autelan.software.bsp&boot. team
*
* DESCRIPTION:
* wifi-ethernet module share memory with wsm
*
*
*******************************************************************************/

#include <linux/types.h>
//#include <cvmx-app-init.h>
#include <../../../../kernel2.6.32.27cn/arch/mips/include/asm/octeon/cvmx-app-init.h>
#include <../../../../kernel2.6.32.27cn/include/trace/events/sched.h>
#include "wifi-sm.h"
#include "wifi.h"

struct task_struct *my_task[QUEUE_NUM];

extern cvmx_bootinfo_t *octeon_bootinfo;
extern struct net_device *wifi_device[VRID_MAX][WIFI_INTERFACE_NUM];
extern unsigned long long vm[VRID_MAX];
extern struct task_struct *kthread_create(int (*threadfn)(void *data),
        				   void *data,
        				   const char namefmt[],
        				   ...);
extern void kthread_bind(struct task_struct *k, unsigned int cpu);
extern int kthread_stop(struct task_struct *k);
extern void kthread_rx(void);

int wifi_init_vrid_vm(unsigned int vrid);


int core_num = 0;
int read_wifi_mmap_addr(char   *buf,char   **start,off_t   offset,int   count,int   *eof,void   *data)  
  {  
          sprintf(buf,"%lx\n",__pa(vm));  
		  printk("read_wifi_mmap_addr \n");
          *eof   =   1;  
          return   9;  
  } 

/*
* Description:
*   Initialize shared memory for every vrid-ac
*
* Parameter:
*   vrid: vrid NO.
*
* Return:
*    -1: failed; 0: Successed.
*
*/
int wifi_init_vrid_vm(unsigned int vrid)
{
	unsigned char *addr = NULL;
	unsigned short coremask = octeon_bootinfo->core_mask;
	unsigned int pages = 0, i = 0;
	struct mmap_head *mem_hd = NULL;
#define IGNORE_ADDR 0xA8000000FFC00000
	
	vm[vrid] = __get_free_pages(GFP_DMA, PAGES_ORDER);
	if (!vm[vrid])
	{
		return -1;
	}else if(IGNORE_ADDR == vm[vrid]){/*AXSSZFI-837*/
		addr = (unsigned char*)vm[vrid];
		printk("%s,get addr and clear it:%#X.\n",__func__,vm[vrid]);
		ClearPageReserved(virt_to_page(addr));
		vm[vrid] = 0;
		vm[vrid] = __get_free_pages(GFP_DMA, PAGES_ORDER);
		printk("%s,regetnew addr:%#X.\n",__func__,vm[vrid]);
	}
	
	memset((void *)vm[vrid], 0, PAGES * PAGE_SIZE);
	addr = (unsigned char *)vm[vrid];
	pages = 0;
	
	while (pages++ < PAGES)
	{
		SetPageReserved(virt_to_page(addr));
		addr += PAGE_SIZE;
	}

	mem_hd = (struct mmap_head*)vm[vrid];
	/* Save coremask for WSM */
	 *((unsigned short *)(vm[vrid] + COREMASK_OFFSET)) = coremask;
	 
	for (i = 0; i < QUEUE_NUM; i++)
	{
		mem_hd->wsmTOwifi_array[i].offset = i * VM_BLOCK_SIZE + PAGE_SIZE;
		mem_hd->wifiTOwsm_array[i].offset = (i + QUEUE_NUM) * VM_BLOCK_SIZE + PAGE_SIZE;
	}

	return 0;
}
void wifi_get_core(void){
	int tmp;
	
	unsigned short coremask = octeon_bootinfo->core_mask;
	for(tmp=0; tmp < QUEUE_NUM; tmp ++)
		{
	        core_num = core_num + ((coremask >> tmp) & 0x1);
		}
	
	printk("%s-core_num = %d\n",__FUNCTION__, core_num);

}
/**
 * It is called by system when insmod wifi-ethernet.ko,
 * the shared memory is initialized and threads are binded to core.
 */
void wifi_inic_vm(void)
{
	unsigned char * addr = NULL;
	unsigned int pages = 0;
	unsigned short coremask = octeon_bootinfo->core_mask;
	unsigned int i = 0;
	int tmp;

	for (i = 0; i < VRID_MAX; i++)
	{
		if (wifi_init_vrid_vm(i) != 0)
		{
			printk("wifi: Initialize instance %d shared memory failed.\n", i);
			return;
		}
	}
	/* get core number*/
	for(tmp=0; tmp < QUEUE_NUM; tmp ++)
		{
	        core_num = core_num + ((coremask >> tmp) & 0x1);
		}
	
	printk("%s-core_num = %d\n",__FUNCTION__, core_num);



	/*Binding a thread to each active core.*/
	for (i = 0; i < 16; i++)
	{
		if (coremask & 0x0001)
		{
			my_task[i] = (struct task_struct *)kthread_create(kthread_rx, NULL, "%s", "wifi_tasklet_rx");
                	if (IS_ERR(my_task[i]))
                	{
                		printk("kthread_create failed %d\n", i);
                		return;
                	}
                   if (my_task[i])
                   {
                   		kthread_bind(my_task[i], i);
                   		wake_up_process(my_task[i]);
                   	}
		}
		else
		{
			my_task[i] = NULL;
		}
		
		coremask = coremask >> 1;
	}

}


/**
 * It is called by system when rmmod wifi-ethernet.ko
 *
 */
void wifi_cleanup_vm(void)
{
	unsigned char *addr = NULL;
	unsigned int i = 0, pages = 0;

	for(i = 0; i < 16; i++)
	{
		if(my_task[i])
		{
			kthread_stop(my_task[i]);
		}
	}

	for (i = 0; i < VRID_MAX; i++)
	{
		addr = (unsigned char*)vm[i];

		while(pages++ < PAGES)
		{
			ClearPageReserved(virt_to_page(addr));
			addr += PAGE_SIZE;
		} 
		free_pages(vm[i], PAGES_ORDER);
		vm[i] = 0;
	}
}



/**
 * To find the interface index which the packet belongs to
 * according to it's wlanID or BSSIndex.
 *
 * @param wlanID    	wlanID which the packet come from.
 * @param BSSIndex    BSSIndex which the packet come from.
 * @return index		interface index
 */
int wifi_if_index(unsigned int vrid, int wlanID, int BSSIndex)
{
	int index;
	if( (wlanID > 0) && (0 == BSSIndex) )
		{
			index = wlanID;
		}
	else if( (0 == wlanID) && (BSSIndex > 0) )
		{
			index = BSSIndex + 256;
		}
	else
		{
			if( wifi_eth_debug >= WIFI_WARNING )printk("Wifi rev error packet: wlanID=%d, BSSIndex=%d\n", wlanID, BSSIndex);
			index = -1;
			return index;
		}

	if( NULL == wifi_device[vrid][index] )
		{
			if( wifi_eth_debug >= WIFI_WARNING )printk("Wifi interface query failed: wlanID=%d, BSSIndex=%d\n", wlanID, BSSIndex);
			return -1;
		}

	return index;

}

