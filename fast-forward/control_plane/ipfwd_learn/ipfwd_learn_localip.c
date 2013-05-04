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
 *ipfwd_learn_localip.c
 *
 *CREATOR:
 *   <zhaohan@autelan.com>
 *
 *DESCRIPTION:
 *    get local ip list, and store to hash tab
 *    we want filter local ip, so we make a tiny hash table to keep local ip. 
 *    when system add/del/update ip configure, we will be  notified, and we will update hash table. 
 *    use fwd_ifa_table_lookup(ip) to filter local ip.
 *
 *
 *DATE:
 *   4/11/2013	
 *
 *  FILE REVISION NUMBER:
 *       $Revision: 1.1 $
 *
 *******************************************************************************/

/* include file */
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>

#include "ipfwd_learn_common.h"
#include "ipfwd_learn_localip.h"


/* macro definition */


/* global variable */
struct hlist_head fwd_ifa_hlist[FWD_IFA_HASH_SIZE];
rwlock_t fwd_ifa_lock;	

static inline uint64_t fwd_ifaddr_hash(uint32_t ip)
{
    uint64_t idx = (ip&0xff) + ((ip>>8)&0xff) + ((ip>>16)&0xff) + ((ip>>24)&0xff);
	idx &= (FWD_IFA_HASH_SIZE - 1);
    return idx;
}

static inline struct fwd_ifaddr_entry *fwd_ifa_hash_find(struct hlist_head *head,
						    const uint32_t ip)
{
	struct hlist_node *h = NULL;
	struct fwd_ifaddr_entry *ifa_node = NULL;

	hlist_for_each_entry_rcu(ifa_node, h, head, hlist) {
		if ((ifa_node) && (ifa_node->ifa_address == ip))
			return ifa_node;
	}
	return NULL;
}


static struct fwd_ifaddr_entry* fwd_ifa_hash_add(struct hlist_head *head, struct in_ifaddr *ifa)
{
    struct fwd_ifaddr_entry *ifa_node = NULL;

    if((!head) || (!ifa))
    {
        return NULL;
    }

    ifa_node = kmalloc(sizeof(struct fwd_ifaddr_entry), GFP_KERNEL);
    if(ifa_node)
    {
        memset(ifa_node, 0, sizeof(struct fwd_ifaddr_entry));
        ifa_node->ifa_address = ifa->ifa_address;
        ifa_node->ifa_mask = ifa->ifa_mask;
        ifa_node->ifa_dev = ifa->ifa_dev;
        hlist_add_head_rcu(&ifa_node->hlist, head);
    }

    return ifa_node;
}


static void fwd_ifa_hash_del(struct fwd_ifaddr_entry *f)
{
    if(NULL == f)
        return;
        
	hlist_del_rcu(&f->hlist);
	kfree(f);
}

static void fwd_ifa_hash_cleanup(void)
{
    int i = 0;
    write_lock(&fwd_ifa_lock);
    for(i = 0; i < FWD_IFA_HASH_SIZE; i++)
    {
        struct fwd_ifaddr_entry *f = NULL;
		struct hlist_node *h=NULL, *n=NULL;

		hlist_for_each_entry_safe(f, h, n, &fwd_ifa_hlist[i], hlist) {
		    if(f){
			    fwd_ifa_hash_del(f);
			}
		}
    }
    write_unlock(&fwd_ifa_lock);
    return;
}


void fwd_ifa_hash_dump(void)
{
    int i = 0;
    printk("dump fwd ifa hlist\n");
    read_lock(&fwd_ifa_lock);
    for(i = 0; i < FWD_IFA_HASH_SIZE; i++)
    {
        struct fwd_ifaddr_entry *f;
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &fwd_ifa_hlist[i], hlist) {
		    if(f){
		        printk("ip=%d.%d.%d.%d, mask=%d.%d.%d.%d, dev=%s\n", 
		                IP_FMT(f->ifa_address), 
		                IP_FMT(f->ifa_mask), 
		                f->ifa_dev->dev->name);
		    }
		}
    }
    read_unlock(&fwd_ifa_lock);
    printk("\n");
    return;
}


void ipfwd_dev_dump(void)
{
    struct net_device *dev = NULL;
    struct in_device *in_dev = NULL;

    printk("ipfwd_learn dump if addr:\n");
	read_lock(&dev_base_lock);
	/*this default init_dev, Originally should get by dev_net
	   1.3code have dev_base*/
	for_each_netdev(&init_net,dev)
	{
        in_dev = __in_dev_get_rtnl(dev);
        if(NULL == in_dev)
            continue;
        for_ifa(in_dev) {
            printk("ip addr=%d.%d.%d.%d, dev=%s\n", IP_FMT(ifa->ifa_address), dev->name);
        } endfor_ifa(in_dev);
	}
	read_unlock(&dev_base_lock);
	return;
}



/* API */


/*****************************************************************
 * Description:
 *  lookup the ifa hash table
 *
 * Parameter:
 *  @ip : ip address to lookup
 * 
 * Return:
 *  NULL: not found
 *  not NULL: found the entry
 *****************************************************************/
struct fwd_ifaddr_entry *fwd_ifa_table_lookup(const uint32_t ip)
{
    uint64_t idx = 0;
    struct fwd_ifaddr_entry *entry = NULL;
    
    idx = fwd_ifaddr_hash(ip);
    read_lock(&fwd_ifa_lock);
    entry = fwd_ifa_hash_find(&fwd_ifa_hlist[idx], ip);
    read_unlock(&fwd_ifa_lock);
    return entry;
}


/*****************************************************************
 * Description:
 *  rebuild the ifa hash table.
 *  1: clear all entry
 *  2: find all local ip in system by init_net, and insert to ifa table.
 *
 * Parameter:
 * 
 * Return:
 *****************************************************************/
void fwd_ifa_hash_rebuild(void)
{
	struct net_device *dev = NULL;
	struct in_device *in_dev = NULL;
	uint64_t idx = 0;

	/* clear all ifaddr hash node */
	fwd_ifa_hash_cleanup();
	read_lock(&dev_base_lock);
	/*this default init_dev, Originally should get by dev_net
	   1.3code have dev_base*/
	for_each_netdev(&init_net,dev)
	{
		if (NULL == dev)
		{
			log(EMERG_LVL, "ipfwd_learn: dev is NULL\n");
			continue;
		}
		in_dev = __in_dev_get_rtnl(dev);
		if(NULL == in_dev)
			continue; 
		for_ifa(in_dev) 
		{
			idx = fwd_ifaddr_hash(ifa->ifa_address);
			if(fwd_ifa_hash_find(&fwd_ifa_hlist[idx], ifa->ifa_address)==NULL)
			{
				fwd_ifa_hash_add(&fwd_ifa_hlist[idx], ifa);
			}
		} endfor_ifa(in_dev);
	}
	read_unlock(&dev_base_lock);
	return;
}


/*****************************************************************
 * Description:
 *    refresh ifa table by system notifier.
 *
 * Parameter:
 * 
 * Return:
 *****************************************************************/
static int ipfwd_learn_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct in_ifaddr *ifa = (struct in_ifaddr*)ptr;
    uint64_t idx = 0;
    struct fwd_ifaddr_entry *f = NULL;
    
	switch (event) {
	case NETDEV_UP:
	    log(DEBUG_LVL, "ipfwd_learn_inetaddr_event: NETDEV_UP\n");
        idx = fwd_ifaddr_hash(ifa->ifa_address);
        write_lock(&fwd_ifa_lock);
        if(fwd_ifa_hash_find(&fwd_ifa_hlist[idx], ifa->ifa_address)==NULL)
        {
            fwd_ifa_hash_add(&fwd_ifa_hlist[idx], ifa);
        }
        write_unlock(&fwd_ifa_lock);
		break;
	case NETDEV_DOWN:
	    log(DEBUG_LVL, "ipfwd_learn_inetaddr_event: NETDEV_DOWN\n");
        idx = fwd_ifaddr_hash(ifa->ifa_address);
        write_lock(&fwd_ifa_lock);
        if((f=fwd_ifa_hash_find(&fwd_ifa_hlist[idx], ifa->ifa_address))!=NULL)
        {
            fwd_ifa_hash_del(f);
        }	    	
        write_unlock(&fwd_ifa_lock);
		break;
	default:
	    log(DEBUG_LVL, "ipfwd_learn_inetaddr_event: unknow event=%lu\n", event);
	    //fwd_ifa_hash_rebuild();
	    break;
	}

    if(DEBUG_LVL == log_level)
    {
        fwd_ifa_hash_dump();
    }

	return NOTIFY_DONE;
}


static struct notifier_block ipfwd_learn_inetaddr_notifier = {
	.notifier_call =ipfwd_learn_inetaddr_event,
};


/*****************************************************************
 * Description:
 *  init ifa hash table
 *
 * Parameter:
 * 
 * Return:
 *****************************************************************/
void fwd_ifa_hash_init(void)
{
    rwlock_init(&fwd_ifa_lock);
    memset(&fwd_ifa_hlist, 0, sizeof(struct hlist_head)*FWD_IFA_HASH_SIZE);
    fwd_ifa_hash_rebuild();
    register_inetaddr_notifier(&ipfwd_learn_inetaddr_notifier);
}

void fwd_ifa_hash_release(void)
{
    unregister_inetaddr_notifier(&ipfwd_learn_inetaddr_notifier);
}


