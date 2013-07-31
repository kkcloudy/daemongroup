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

#include "log.h"
#include "netlink.h"
#include "pppoe_snooping.h"
#include "packet_snooping.h"
#include "dhcp_broadcast_agent.h"

#include "dba/dba.h"	/* accapi/dba/dba.h */

int dba_option82_debug = 0;
module_param(dba_option82_debug, int, 0644);


/* net/core/dev.c */
extern int (*br_dhcp_broadcast_hook) (struct sk_buff **pskb, struct dba_result *res);
extern int (*wifi_ko_dhcp_option82_hook)(struct sk_buff *skb, struct dba_result *res);

/*export for dhcp option82 */
int dba_handle(struct sk_buff *skb, struct dba_result *res)
{
	if (unlikely(!skb || !res)) {
		return -1;
	}
	
	return packet_snooping(skb, res);
}
EXPORT_SYMBOL(dba_handle);


static int br_dhcp_broadcast(struct sk_buff **pskb, struct dba_result *res);;


static int br_dhcp_broadcast(struct sk_buff **pskb, struct dba_result *res)
{	
	if (unlikely(!pskb) || unlikely(!*pskb) || unlikely(!res)) {
		return -1;
	}

	return packet_snooping(*pskb, res);
}

/**
 * Module/driver initialization. broadcast packet from uplink to unicast
 * 
 *
 * @return Zero on success
 */
static int __init dhcp_broadcast_init_module(void)
{
	printk(KERN_INFO "insert module: direct broadcast dhcp\n");
	br_dhcp_broadcast_hook = br_dhcp_broadcast;
	wifi_ko_dhcp_option82_hook = dba_handle;

	if (pppoe_snooping_netlink_init()) {
		printk(KERN_ERR "pppoe netlink init failed.\n");
	}

	return 0;
}


/**
 * Module / driver shutdown
 *
 * @return Zero on success
 */
static void __exit dhcp_broadcast_cleanup_module(void)
{
	printk(KERN_INFO "remove module: direct broadcast dhcp\n");
	br_dhcp_broadcast_hook = NULL;
	wifi_ko_dhcp_option82_hook = NULL;
	pppoe_snooping_netlink_release();
}


MODULE_LICENSE("Autelan");
MODULE_AUTHOR("Autelan");
MODULE_DESCRIPTION("direct broadcast dhcp module.");
module_init(dhcp_broadcast_init_module);
module_exit(dhcp_broadcast_cleanup_module);
