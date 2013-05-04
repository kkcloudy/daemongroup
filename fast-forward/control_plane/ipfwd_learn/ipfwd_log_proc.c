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
 *ipfwd_log.c
 *
 *CREATOR:
 *   <zhaohan@autelan.com>
 *
 *DESCRIPTION:
 *    prog entry of fastfwd log system.
 *    we can show fastfwd log of each single core
 *
 *
 *DATE:
 *   4/11/2013	
 *
 *  FILE REVISION NUMBER:
 *       $Revision: 1.1 $
 *
 *******************************************************************************/
#ifdef BUILDNO_VERSION2_1
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-fau.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-sysinfo.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-config.h>
#include <asm/octeon/cvmx-spinlock.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <linux/autelan_product.h>
#else
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-fpa.h"
#include "cvmx-pko.h"
#include "cvmx-pow.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-helper.h"
#include "cvmx-bootmem.h"
#include "ipfwd_learn.h"
#include <linux/autelan_product_type.h>
#endif


#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "ipfwd_learn_common.h"
#include "ipfwd_log_proc.h"



CVMX_SHARED uint8_t *fwd_log_mem_addr[MAX_CORE_NUM];
CVMX_SHARED uint8_t log_core = 4;


static int32_t fwd_log_mem_get(void)
{
	const cvmx_bootmem_named_block_desc_t *fwd_log_mem_desc = NULL;
    int i;
    char bootmem_name[64];
        
    for(i = 0; i < MAX_CORE_NUM; i++)
    {
        memset(bootmem_name, 0, sizeof(bootmem_name));
        sprintf(bootmem_name, FASTFWD_LOG_MEM_NAME, i);
   
    	fwd_log_mem_desc = cvmx_bootmem_find_named_block(bootmem_name);
    	if(NULL != fwd_log_mem_desc)
    	{
            printk(KERN_INFO "%s found at: 0x%p, size: 0x%llu\n", 
                            fwd_log_mem_desc->name, (void *)fwd_log_mem_desc->base_addr, fwd_log_mem_desc->size);
    		fwd_log_mem_addr[i] = cvmx_phys_to_ptr(fwd_log_mem_desc->base_addr);
    	}    	
	}
	
	return IPFWD_LEARN_RETURN_OK;
}


static void *fwd_log_mem_seq_start(struct seq_file *seq, loff_t *pos)
{
	unsigned char *start = (unsigned char *)fwd_log_mem_addr[log_core];
	if(*pos >= 256){
		*pos = 0;
		return NULL;
	}else{
		return (void *)(start+ (*pos) * FWD_LOG_MEM_SHOW_LEN);
	}
}
static void *fwd_log_mem_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	void *ptr_next = NULL;
	ptr_next = (void *)((unsigned char *)v + FWD_LOG_MEM_SHOW_LEN);
	(*pos)++;
	return ptr_next;
}
static int fwd_log_mem_seq_show(struct seq_file *seq, void *v)
{
	int i = 0;
	void *pt = v;

	if(NULL == pt)	{
		printk(KERN_INFO "fwd_log_mem show data pointer NULL.\n");
		return -1;
	}
	for(i = 0; i < FWD_LOG_MEM_SHOW_LEN; i++)	{
		seq_printf(seq, "%c", *((unsigned char *)pt + i));
	}
	return 0;
} 
static void  fwd_log_mem_seq_stop(struct seq_file *seq, void *v)
{
	return;
}

struct seq_operations fwd_log_mem_seq_ops = {
	.start = fwd_log_mem_seq_start,
	.next  = fwd_log_mem_seq_next,
	.show  = fwd_log_mem_seq_show,
	.stop  = fwd_log_mem_seq_stop,
};

static int fwd_log_proc_open(struct inode *inode, struct file *file)
{
	int retval = -1;

	if(NULL == file)
	{
		printk(KERN_INFO "fwd_log file pointer is NULL.\n");
		return retval;
	}

	retval = seq_open(file, &fwd_log_mem_seq_ops);
	if(retval)
	{
		printk(KERN_INFO "fwd_log cannot open seq_file.\n");
		remove_proc_entry(FASTFWD_LOG_PROC_NAME, NULL);
	}

	return retval;
}

static ssize_t fwd_log_ctl_proc_write(struct file *flip, const char __user *buff, unsigned long len, void *data)
{
    char kbuf[10];
	if(len > 10)
	{
		printk(KERN_INFO "fwd_log_ctl_proc_write buffer is full.\n");
		return -ENOSPC;
	}

    memset(kbuf, 0, 10);
	if(copy_from_user(&kbuf, buff, 2))
	{
		printk(KERN_INFO "fwd_log_ctl_proc_write copy_from_user error.\n");
		return -EFAULT;
	}

    log_core = simple_strtol(kbuf, NULL, 10);
	return len;
	
}

static int fwd_log_ctl_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if(off > 0)
	{
		*eof = 1;
		return 0;
	}

	memset(page, 0, PAGE_SIZE);

	len = sprintf(page, "%d\n", log_core);

	*eof = 1;
	
	return len;
	
}


struct file_operations fwd_log_ops = {
	.owner   = THIS_MODULE,
	.open    = fwd_log_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_private,
};

int fwd_log_proc_init(void)
{
	int retval = -1;
	struct proc_dir_entry *fwd_log_proc_entry = NULL;
	struct proc_dir_entry *fwd_log_ctl_proc_entry = NULL;

	fwd_log_proc_entry = create_proc_entry(FASTFWD_LOG_PROC_NAME, 0, NULL);
	if(fwd_log_proc_entry)
	{
		fwd_log_proc_entry->proc_fops = &fwd_log_ops;
		retval = 0;
	}
	else
	{
		printk(KERN_INFO "fwd_log create %s error.\n", FASTFWD_LOG_PROC_NAME);
		retval = -1;
	}

	fwd_log_ctl_proc_entry = create_proc_entry(FASTFWD_LOG_CTL_PROC_NAME, 0666, NULL);
	if(fwd_log_ctl_proc_entry)
	{
		fwd_log_ctl_proc_entry->read_proc = (read_proc_t *)fwd_log_ctl_proc_read;
		fwd_log_ctl_proc_entry->write_proc = (write_proc_t *)fwd_log_ctl_proc_write;
		//fwd_log_ctl_proc_entry->owner = THIS_MODULE;
	}
	else
	{
		printk(KERN_INFO "fwd_log_ctl create %s error.\n", FASTFWD_LOG_CTL_PROC_NAME);
		remove_proc_entry(FASTFWD_LOG_PROC_NAME, NULL);
		retval = -1;
	}	

	memset(fwd_log_mem_addr, 0, sizeof(fwd_log_mem_addr));
	fwd_log_mem_get();
	return retval;
}

int fwd_log_proc_release(void)
{
	/* ipfwd_learn_exit  add fwd_log_proc  file remove  by fastfwd group 20121127 */
	remove_proc_entry(FASTFWD_LOG_PROC_NAME, NULL);
	remove_proc_entry(FASTFWD_LOG_CTL_PROC_NAME, NULL);

	return 0;
}

