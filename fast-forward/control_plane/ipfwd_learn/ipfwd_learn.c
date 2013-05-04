/******************************************************************
 *  ipfwd_learn module  
 *  fast forward  module   control plane learn moudle
 *****************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/mii.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/netdevice.h>

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
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
//#include <linux/netfilter/nf_conntrack_common.h>

#include "ipfwd_learn_localip.h"
#include "ipfwd_log_proc.h"
#include "ipfwd_learn.h"

#include "cvmx-atomic.h"

#define LOG_PRINT_HZ  5000
static unsigned long long  log_count = 0;
int log_level = INFO_LVL;  /* printk log level */

int icmp_enable = FUNC_DISABLE;
int learn_enable = FUNC_ENABLE;

/* work mode */
#define IPFWD_LEARN_MODE_NONE                  0
#define IPFWD_LEARN_MODE_COEXIST_ENABLE       1
#define IPFWD_LEARN_MODE_STANDALONE_ENABLE    2

uint32_t ipfwd_learn_mode = IPFWD_LEARN_MODE_NONE;

#ifdef BUILDNO_VERSION2_1
extern product_info_t autelan_product_info;
#else
extern product_info_t  product_info;
#endif

/* ipfwd_learn add dev_type to identify the caller of different net_device driver 20121127*/ 
extern int (*cvm_ipfwd_learn_tx_hook) (struct sk_buff *, uint8_t dev_type);

module_param(log_level, int, 0644);
MODULE_PARM_DESC(log_level, "\n"
		"\t\tprintk enable level, range [0,7], default log_level is 7.\n"
		"\t\tIf printk's level is larger than log_level, there will not print."
		);

module_param(icmp_enable,int,0644);
MODULE_PARM_DESC(icmp_enable,"\n"
	    "\t\t 0 disable icmp learned\n"
	    "\t\t 1 enable  icmp learned\n"
	    );

module_param(learn_enable,int,0644);
MODULE_PARM_DESC(learn_enable,"\n"
		"\t\t enable|disable learn function\n"
		"\t\t 0 disable learn function\n"
		"\t\t 1 enable  learn function\n"
	    );

#ifdef IPFWD_LEARN_MODE_STANDALONE   
extern int octnet_get_nic_ifcount(void);
extern struct net_device** octnet_get_nic_netdevs(void);
extern int16_t octnet_get_netdev_gmxport(struct net_device *pndev);
extern int octnet_fccp_xmit(struct sk_buff *skb, struct net_device *pndev);
#endif


		


static inline void dump_skb(struct sk_buff* skb)
{   
    int i = 0;

    if((!skb) || (!skb->dev)){
        return ;
    }
    if(skb->dev->name != NULL)
    {
        printk("ipfwd_learn receive skb packet from %s :\n", skb->dev->name);
    }
    else
    {
        printk("ipfwd_learn receive skb packet :\n");
    }
    
    for(i = 0; ((i<skb->len) && (i<256)); i++)
    {       
        printk("%02x ", skb->data[i]);

        if(0==(i+1)%16) {
            printk("\n");           
        }
    }
    printk("\n\n");     

    return ;
}


static inline void dump_fccp_packet(control_cmd_t *fccp_packet)
{	
	int i;
	if(!fccp_packet){
		return ;
	}
	log(DEBUG_LVL,"fccp packet format:\ndest module:%d\nsrc module:%d\ncmd opcode:%d\ncmd len:%d\n",\
		          fccp_packet->dest_module,fccp_packet->src_module,\
		          fccp_packet->cmd_opcode,fccp_packet->cmd_len);
	
	switch(fccp_packet->fccp_data.rule_info.rule_param.action_type)
	{
		case FLOW_ACTION_RPA_ETH_FORWARD         :
		case FLOW_ACTION_RPA_CAPWAP_FORWARD      :
		case FLOW_ACTION_RPA_CAP802_3_FORWARD    :
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP   :
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP  :
		case FLOW_ACTION_RPA_ICMP                :
			log(DEBUG_LVL,"rpa header info :\n");
			log(DEBUG_LVL,"\ttype:%u , dnetdevNum:%u , snetdevNum:%u , d_s_slotNum:%u\n",\
				fccp_packet->fccp_data.rule_info.rule_param.rpa_header.rpa_type,\
				fccp_packet->fccp_data.rule_info.rule_param.rpa_header.rpa_dnetdevNum,\
				fccp_packet->fccp_data.rule_info.rule_param.rpa_header.rpa_snetdevNum,\
				fccp_packet->fccp_data.rule_info.rule_param.rpa_header.rpa_d_s_slotNum);
			break;
		default:
			break;
	}
	log(DEBUG_LVL,"L2 header info :\n");
	log(DEBUG_LVL,"\tDSA header info :%llu\n"
		"\tdest MAC: %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\tsrc MAC: %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\tout_ether_type:%u , out_tag:%u\n"
		"\tin_ether_type:%u , in_tag:%u\n"
		"\tether_type:%u\n",
		fccp_packet->fccp_data.rule_info.rule_param.dsa_info,\
		MAC_FMT_DEFINE(fccp_packet->fccp_data.rule_info.rule_param.ether_dhost),\
		MAC_FMT_DEFINE(fccp_packet->fccp_data.rule_info.rule_param.ether_shost),\
		fccp_packet->fccp_data.rule_info.rule_param.out_ether_type,\
		fccp_packet->fccp_data.rule_info.rule_param.out_tag,\
		fccp_packet->fccp_data.rule_info.rule_param.in_ether_type,\
		fccp_packet->fccp_data.rule_info.rule_param.in_tag,\
		fccp_packet->fccp_data.rule_info.rule_param.ether_type);

	switch(fccp_packet->fccp_data.rule_info.rule_param.action_type)
	{
		
		case FLOW_ACTION_CAPWAP_802_11_ICMP :
		case FLOW_ACTION_CAPWAP_FORWARD :
		case FLOW_ACTION_CAP802_3_FORWARD :
		case FLOW_ACTION_CAPWAP_802_3_ICMP :
		case FLOW_ACTION_RPA_CAPWAP_FORWARD:
		case FLOW_ACTION_RPA_CAP802_3_FORWARD:
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP:
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP:
			log(DEBUG_LVL,"capwap tunnel info :\n");
			log(DEBUG_LVL,"\tsrc ip :%d.%d.%d.%d\n\tdest ip: %d.%d.%d.%d\n"		
			              "\ttos: %d\n\tsrc port : %d\n\tdest port: %d\n\tcapwap header:",
			              IP_FMT(fccp_packet->fccp_data.rule_info.cw_cache.sip), \
			              IP_FMT(fccp_packet->fccp_data.rule_info.cw_cache.dip),\
			              fccp_packet->fccp_data.rule_info.cw_cache.tos,\
			              fccp_packet->fccp_data.rule_info.cw_cache.sport, \
			              fccp_packet->fccp_data.rule_info.cw_cache.dport);
			for(i=0;i<CW_H_LEN;i++)
			{
				log(DEBUG_LVL,"%02x ",fccp_packet->fccp_data.rule_info.cw_cache.cw_hd[i]);
			}
			log(DEBUG_LVL,"\n");
			switch (fccp_packet->fccp_data.rule_info.rule_param.action_type)
			{
				
				case FLOW_ACTION_CAPWAP_FORWARD :
				case FLOW_ACTION_RPA_CAPWAP_FORWARD:
				case FLOW_ACTION_CAPWAP_802_11_ICMP :
				case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP:
					log(DEBUG_LVL,"\tWifi fc : %02x%02x\n"
						"\tWifi qos: %02x%02x\n"
			            "\tWifi addr1:%02x%02x%02x%02x%02x%02x\n"
			            "\tWifi addr2:%02x%02x%02x%02x%02x%02x\n"
			            "\tWifi addr3:%02x%02x%02x%02x%02x%02x\n",\
			            fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[0], \
			            fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[1],\
			            fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[0],\
			            fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[1],\
			            MAC_FMT_DEFINE(&fccp_packet->fccp_data.rule_info.rule_param.tunnel_l2_header.wifi_header.addr[0]),\
			            MAC_FMT_DEFINE(&fccp_packet->fccp_data.rule_info.rule_param.tunnel_l2_header.wifi_header.addr[6]),\
			            MAC_FMT_DEFINE(&fccp_packet->fccp_data.rule_info.rule_param.tunnel_l2_header.wifi_header.addr[12]));
					break;

				default:
					log(DEBUG_LVL,"\ttunnel dest mac: %02x:%02x:%02x:%02x:%02x:%02x\n"
		                "\ttunnel src  mac: %02x:%02x:%02x:%02x:%02x:%02x\n"
		                "\ttunnel type : 0x%x\n",\
		                MAC_FMT_DEFINE(fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_dmac),\
		                MAC_FMT_DEFINE(fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_smac),\
		                fccp_packet->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_ether);
					break;	
			}
			break;
		default:
			break;
	}	

	log(DEBUG_LVL,"\tforward port : %d\n",\
		fccp_packet->fccp_data.rule_info.rule_param.forward_port);
	log(DEBUG_LVL,"\taction type :");
	switch(fccp_packet->fccp_data.rule_info.rule_param.action_type)
	{
		case FLOW_ACTION_DROP :
			log(DEBUG_LVL,"FLOW_ACTION_DROP\n");
			break;
		case FLOW_ACTION_TOLINUX   :
			log(DEBUG_LVL,"FLOW_ACTION_TOLINUX\n");
			break;
		case FLOW_ACTION_ICMP   :
			log(DEBUG_LVL,"FLOW_ACTION_ICMP\n");
			break;
		case FLOW_ACTION_CAPWAP_802_11_ICMP  :
			log(DEBUG_LVL,"FLOW_ACTION_CAPWAP_802_11_ICMP\n");
			break;
		case FLOW_ACTION_ETH_FORWARD   :
			log(DEBUG_LVL,"FLOW_ACTION_ETH_FORWARD\n");
			break;
		case FLOW_ACTION_CAPWAP_FORWARD   :
			log(DEBUG_LVL,"FLOW_ACTION_CAPWAP_FORWARD\n");
			break;
		case FLOW_ACTION_CAP802_3_FORWARD   :
			log(DEBUG_LVL,"FLOW_ACTION_CAP802_3_FORWARD\n");
			break;
		case FLOW_ACTION_CAPWAP_802_3_ICMP   :
			log(DEBUG_LVL,"FLOW_ACTION_CAPWAP_802_3_ICMP\n");
			break;

		case FLOW_ACTION_RPA_ETH_FORWARD         :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_ETH_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_CAPWAP_FORWARD :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_CAPWAP_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_CAP802_3_FORWARD :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_CAP802_3_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_CAP802_3_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP  :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_CAPWAP_802_11_ICMP\n");
			break;
		case FLOW_ACTION_RPA_ICMP :
			log(DEBUG_LVL,"FLOW_ACTION_RPA_ICMP\n");
			break;
	}
	return ;
}

/******************ipfwd_learn-stat   start**************************/
/**
 * User is reading /proc/octeon_ethernet_stats
 *
 * @param m
 * @param v
 * @return
 */
static int ipfwd_learn_stats_show(struct seq_file *m, void *v)
{
    ipfwd_learn_stats_t* stats = &fwd_learn_stats;
    int i = 0;
    
    seq_printf(m, "----------------------------------------\n");
    seq_printf(m, "total_recv_cnt=%lld\n", stats->total_recv_cnt);
    seq_printf(m, "spe_if_name=%lld\n", stats->spe_if_name);
    seq_printf(m, "l2hdr_parse_fail=%lld\n", stats->l2hdr_parse_fail);
	seq_printf(m, "  mcast=%lld\n", stats->mcast);
    seq_printf(m, "spe_ip=%lld\n", stats->spe_ip);
    seq_printf(m, "local_sip=%lld\n", stats->local_sip);
    seq_printf(m, "local_dip=%lld\n", stats->local_dip);
	seq_printf(m, "icmp=%lld\n", stats->icmp);
    seq_printf(m, "wqe_alloc_fail=%lld\n", stats->wqe_alloc_fail);
    seq_printf(m, "pkt_alloc_fail=%lld\n", stats->pkt_alloc_fail);
    seq_printf(m, "priv_flags_err=%lld\n", stats->priv_flags_err);
    seq_printf(m, "spe_udp_port=%lld\n", stats->spe_udp_port);
    seq_printf(m, "  eth_learn_fail=%lld\n", stats->eth_learn_fail);
	seq_printf(m, "udp_decap_fail=%lld\n", stats->udp_decap_fail);
	seq_printf(m, "get_send_port=%lld\n", stats->get_send_port);
	seq_printf(m, "  cw_learn_fail=%lld\n", stats->cw_learn_fail);
    seq_printf(m, "cw_priv_flags_err=%lld\n", stats->cw_priv_flags_err);
    seq_printf(m, "cw_spe_ip=%lld\n", stats->cw_spe_ip);
    seq_printf(m, "cw_spe_udp_port=%lld\n", stats->cw_spe_udp_port);
	seq_printf(m, "cw_get_send_port=%lld\n", stats->cw_get_send_port);
	seq_printf(m, "  cw8023_learn_fail=%lld\n", stats->cw8023_learn_fail);
    seq_printf(m, "cw8023_priv_flags_err=%lld\n", stats->cw8023_priv_flags_err);
    seq_printf(m, "cw8023_spe_ip=%lld\n", stats->cw8023_spe_ip);
    seq_printf(m, "cw8023_spe_udp_port=%lld\n", stats->cw8023_spe_udp_port);
	seq_printf(m, "cw8023_get_send_port=%lld\n", stats->cw8023_get_send_port);
    seq_printf(m, "icmp_learn_fail=%lld\n", stats->icmp_learn_fail);
    seq_printf(m, "cw_icmp_learn_fail=%lld\n", stats->cw_icmp_learn_fail);
    seq_printf(m, "cw8023_icmp_learn_fail=%lld\n", stats->cw8023_icmp_learn_fail);
    seq_printf(m, "pkt_type_nonsupport=%lld\n", stats->pkt_type_nonsupport);
    seq_printf(m, "eth_good_cnt=%lld\n", stats->eth_good_cnt);
    seq_printf(m, "cw_good_cnt=%lld\n", stats->cw_good_cnt);
    seq_printf(m, "cw8023_good_cnt=%lld\n", stats->cw8023_good_cnt);
    seq_printf(m, "icmp_good_cnt=%lld\n", stats->icmp_good_cnt);
    seq_printf(m, "cw8023_icmp_good_cnt=%lld\n", stats->cw8023_icmp_good_cnt);
    seq_printf(m, "cw_icmp_good_cnt=%lld\n", stats->cw_icmp_good_cnt);
    seq_printf(m, "total_good_cnt=%lld\n", stats->total_good_cnt);   
    seq_printf(m, "----------------------------------------\n");

    /* read clear */
    for(i = 0; i < (sizeof(fwd_learn_stats)/sizeof(int64_t)); i++)
    {
        cvmx_atomic_set64((int64_t*)stats + i, 0);	
    }
    
    return 0;
}


/**
 * /proc/octeon_ethernet_stats was openned. Use the single_open iterator
 *
 * @param inode
 * @param file
 * @return
 */
static int ipfwd_learn_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ipfwd_learn_stats_show, NULL);
}


static struct file_operations ipfwd_learn_stats_operations = {
	.open		= ipfwd_learn_stats_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


void fwd_stats_proc_initialize(void)
{    
    struct proc_dir_entry *entry = create_proc_entry("ipfwd_learn_stats", 0, NULL);
    if (entry)
        entry->proc_fops = &ipfwd_learn_stats_operations;

    memset(&fwd_learn_stats, 0, sizeof(fwd_learn_stats));    
}

void fwd_stats_proc_shutdown(void)
{
    remove_proc_entry("ipfwd_learn_stats", NULL);
}

/******************ipfwd_learn-stat   end**************************/



static inline int16_t get_forward_port(struct sk_buff* skb,uint8_t dev_type)
{
	if(!skb)
		return IPFWD_LEARN_RETURN_FAIL;

	if(dev_type == DEV_TYPE_OCTEON_ETH)
	{
		cvm_oct_private_t *priv = (cvm_oct_private_t *)(netdev_priv(skb->dev));
		return GET_PRIV_PKO_PORT(priv);	
	}
	else if(dev_type == DEV_TYPE_OCTEON_PCIE)
	{
#ifdef IPFWD_LEARN_MODE_STANDALONE
		return octnet_get_netdev_gmxport(skb->dev);
#else
        return IPFWD_LEARN_RETURN_FAIL;
#endif        
	}

	return IPFWD_LEARN_RETURN_FAIL;
}


/**********************************************************************
 * Description:
 *        Parse transmitted frame Lay2 header and get IP header offset. And get vlan tag 
 *        value if they exist.
 *
 * Parameter:
 * 	  skb:                       packet skb
 *      ip:                         external IP header
 *     vtag1:                    vlan tag 1
 *     vtag2:                    vlan tag 2
 *     out_ether_type:       out ether head type
 *     inter_ether_type :    inter ether head type(Q-in-Q use)
 *     rpa_flag:                rpa tag if there are
 *
 * Return:
 *     IPFWD_LEARN_RETURN_OK    : successfully
 *     IPFWD_LEARN_RETURN_FAIL  : failed
 *
 *********************************************************************/
/*add by wangjian for support pppoe 2013-3-12*/
static inline int8_t tx_l2hdr_parse(struct sk_buff *skb, 
		struct iphdr **ip, 
		uint16_t *vtag1, 
		uint16_t *vtag2, 
		uint16_t *out_ether_type, 
		uint16_t *inter_ether_type,
		uint32_t *rpa_flag,
		uint16_t *pppoe_session_id,
		uint16_t *pppoe_flag
)
{	
	eth_hdr_t *eth_hdr = NULL;
	uint16_t *proto = NULL;
	if(NULL == skb || NULL == ip || NULL == vtag1 || \
		NULL == vtag2 || NULL == out_ether_type || \
		NULL == inter_ether_type || NULL == rpa_flag)
	{
		log(DEBUG_LVL,"tx_l2hdr_parse param error\n");
		return IPFWD_LEARN_RETURN_FAIL;
	}
	eth_hdr = (eth_hdr_t *)skb->data;
    
    /* rpa pkt without vlan */
	if(eth_hdr->h_vlan_proto == RPA_COOKIE)
	{
		*rpa_flag = IS_RPA;
		eth_hdr = (eth_hdr_t*)(skb->data + RPA_HEAD_LEN(*rpa_flag));
	}

    /* rpa pkt with vlan */
    if((eth_hdr->h_vlan_proto == ETH_P_8021Q) && 
	    (((vlan_eth_hdr_t*)eth_hdr)->h_eth_type == RPA_COOKIE))
    {
		*rpa_flag = IS_VLAN_RPA;
		eth_hdr = (eth_hdr_t*)(skb->data + RPA_HEAD_LEN(*rpa_flag));
    }
    
    /* filter bcast & mcast & zero */
    if((!is_valid_ether_addr(eth_hdr->h_dest)) || 
        (!is_valid_ether_addr(eth_hdr->h_source)))
    {
    	cvmx_atomic_add64(&fwd_learn_stats.mcast, 1);
        return IPFWD_LEARN_RETURN_FAIL;
    }

	if (eth_hdr->h_vlan_proto == ETH_P_IP) 
	{
		*ip	= (struct iphdr *)((uint8_t*)eth_hdr + ETH_H_LEN);
		return IPFWD_LEARN_RETURN_OK;
	}
	else if (eth_hdr->h_vlan_proto == ETH_P_8021Q) 
	{
		*vtag1 = *((uint16_t*)((uint8_t*)eth_hdr + ETH_H_LEN));
		
		proto = (uint16_t*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_PROTO_LEN );
		*out_ether_type = *((uint16_t*)((uint8_t*)eth_hdr + MAC_LEN*2));

		if (*proto == ETH_P_IP) 
		{
			*ip = (struct iphdr*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_HLEN);
			return IPFWD_LEARN_RETURN_OK;	
		}
		/* Q-in-Q */
		else if (*proto == ETH_P_8021Q)
		{
			*vtag2 = *((uint16_t*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_HLEN ));
			
			*inter_ether_type =  *((uint16_t*)((uint8_t*)eth_hdr + MAC_LEN*2 + VLAN_HLEN));

			proto = (uint16_t*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_HLEN + VLAN_PROTO_LEN);
			if (*proto == ETH_P_IP) 
			{
				*ip = (struct iphdr*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_HLEN * 2);
				return IPFWD_LEARN_RETURN_OK;
			}
			/* pppoe process */
			else if (PPPOE_TYPE == *proto)
			{
				if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + VLAN_HLEN * 2 + 6))
				{
					*pppoe_session_id = *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + VLAN_HLEN * 2 + 2);
					*pppoe_flag = 1;
					*ip = (struct iphdr*)((uint8_t*)eth_hdr + ETH_H_LEN + VLAN_HLEN * 2 + PPPOE_H_LEN);
					return IPFWD_LEARN_RETURN_OK;
				}
			}
		}
		else if (PPPOE_TYPE == *proto)
		{
			if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + VLAN_HLEN + 6))
			{
				*pppoe_session_id = *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + VLAN_HLEN + 2);
				*pppoe_flag = 1;
				*ip = (struct iphdr*)((uint8_t*)eth_hdr + VLAN_HLEN + ETH_H_LEN + PPPOE_H_LEN);
				return IPFWD_LEARN_RETURN_OK;
			}
		}
		
		log(DEBUG_LVL, "ifpwd_learn:not normal 8021Q packet format\n");
		if(DEBUG_LVL == log_level)
		{
			dump_skb(skb);
		}
		return IPFWD_LEARN_RETURN_FAIL;
	}
	
	else if (PPPOE_TYPE == eth_hdr->h_vlan_proto)
	{
		if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + 6))
		{
			*pppoe_session_id = *(uint16_t *)((uint8_t *)eth_hdr + ETH_H_LEN + 2);
			*pppoe_flag = 1;
			*ip = (struct iphdr*)((uint8_t*)eth_hdr + ETH_H_LEN + PPPOE_H_LEN);
			return IPFWD_LEARN_RETURN_OK;
		}
	}

	return IPFWD_LEARN_RETURN_FAIL;
}
#if 0
static inline int8_t get_dsa_info(struct sk_buff *skb, uint64_t *dsa_info)
{	

	cvm_oct_private_t *priv = NULL;

	if(!skb || !(skb->dev) || !(skb->dev->priv))
	{
		return IPFWD_LEARN_RETURN_FAIL;
	}

	priv = (cvm_oct_private_t *)(skb->dev->priv);	


	/*DSA */
	if (product_info.product_type == AX_5612){

		if( 1 == GET_PRIV_PKO_PORT(priv))
		{
			memcpy(dsa_info, (skb->data + MAC_LEN * 2 + 2), sizeof(dsa_info));
			return IPFWD_LEARN_RETURN_OK;
		}else
		{
			return IPFWD_LEARN_RETURN_OK;
		}


	}	
	else if (product_info.product_type == AX_5612I || product_info.product_type == AX_7605)
	{

		if(0 == GET_PRIV_PKO_PORT(priv))
		{	
			memcpy(dsa_info, (skb->data + MAC_LEN * 2 + 2), sizeof(dsa_info));			
			return IPFWD_LEARN_RETURN_OK;
		}else
		{
			return IPFWD_LEARN_RETURN_OK;
		}


	}	
	else if ((product_info.product_type == AU_4624 || product_info.product_type == AU_4524 || product_info.product_type == AU_4524_P || product_info.product_type == AU_4524_P ||
				product_info.product_type == AU_3524 || product_info.product_type == AU_3524_P || product_info.product_type == AU_3052 || product_info.product_type == AU_3052E || 
				product_info.product_type == AU_3052_P || product_info.product_type == AU_3028 || product_info.product_type == AU_3028E || product_info.product_type == AU_3028_P))
	{

		if(2 == GET_PRIV_PKO_PORT(priv))
		{
			memcpy(dsa_info, (skb->data + MAC_LEN * 2 + 2), sizeof(dsa_info));
			return IPFWD_LEARN_RETURN_OK;
		}else
		{
			return IPFWD_LEARN_RETURN_OK;
		}


	}else 
	{	
		printk("product %d don't Support\n", product_info.id);
		return IPFWD_LEARN_RETURN_FAIL;
	}

}
#endif


/********************************************************
 * Description:
 *        Get the type of capwap packet.
 *
 * Parameter:
 *        cw_h:  CAPWAP header point.
 *
 * Return:
 *        CW_802_3_STREAM   
 *        CW_802_11_STREAM
 *        CW_UNKNOWN_STREAM
 *
 *********************************************************/
static inline int8_t get_capwap_type(union capwap_hd * cw_h)
{
	if( 0 == cw_h->m_t )
	{
		return CW_802_3_STREAM;
	}

	if( 1 == cw_h->m_t )
	{
		if( 1 == cw_h->m_wbid )
		{
			return CW_802_11_STREAM;
		}
	}
	return CW_UNKNOWN_STREAM;
}


/*************************************************************
 * Description:
 *          Decap tx udp packet and skip special udp packet.
 *
 * Parameter:
 *         uh: external udp header
 *
 * Return:
 *         IPFWD_LEARN_RETURN_OK  :  successfully
 *         IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *************************************************************/
/*add by wangjian for support pppoe 2013-3-12*/
static inline int8_t tx_udp_decap(struct udphdr *uh, struct iphdr** true_ip, uint16_t *pppoe_session_id, uint16_t *pppoe_flag)
{
	int8_t packet_type = 0;
	struct ieee80211_frame* ieee80211_hdr = NULL;
	int len = 0;
	struct iphdr* in_ip = NULL;
	uint16_t *proto = NULL;
	if ((uh->source == CW_DAT_STD_PORT || uh->source == CW_DAT_TMP_PORT) && (uh->dest == CW_DAT_AP_PORT))
	{		
		packet_type = get_capwap_type((union capwap_hd *)((char *)uh + UDP_H_LEN));

		if(CW_802_11_STREAM == packet_type)
		{
			ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN);
			if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) 
			{
				len = IEEE80211_QOS_H_LEN;
			} 
			else 
			{ 
				len = IEEE80211_H_LEN;
			}
			
			proto = (uint16_t*)((uint8_t*)ieee80211_hdr + len + LLC_H_LEN - 2);

			/* Default Ip */
			if (ETH_P_IP == *proto)
			{
				in_ip = (struct iphdr*)((uint8_t*)ieee80211_hdr + len + LLC_H_LEN);
			}
			/* PPPOE */
			else if (PPPOE_TYPE == *proto)
			{
				/* PPPOE_IP */
				if (PPPOE_IP_TYPE== *(uint16_t *)((uint8_t *)ieee80211_hdr + len + LLC_H_LEN + 6))
				{
					*pppoe_session_id = *(uint16_t *)((uint8_t *)ieee80211_hdr + len + LLC_H_LEN + 2);
					*pppoe_flag = 1;
					in_ip = (struct iphdr*)((uint8_t*)ieee80211_hdr + len + LLC_H_LEN + 8);
				}
				else
				{
					return CW_UNKNOWN_STREAM;
				}
			}
			else
			{
				return CW_UNKNOWN_STREAM;
			}
			
			*true_ip = in_ip;
			
			/* Is capwap 802.11 icmp? */
			if (in_ip->protocol == IP_PROTOCOL_ICMP)
			{
				return CW_802_11_ICMP;
			}
			else
			{
				return CW_802_11_STREAM;
			}
		}
		else if(packet_type == CW_802_3_STREAM)
		{
			proto = (uint16_t *)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN - 2);
		
			/* Default Ip */
			if (ETH_P_IP == *proto)
			{
				in_ip = (struct iphdr*)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN);
			}
			/* PPPOE */
			else if (PPPOE_TYPE == *proto)
			{
				/* PPPOE_IP */
				if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + 6))
				{
					*pppoe_session_id = *(uint16_t *)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + 2);
					*pppoe_flag = 1;
					in_ip = (struct iphdr*)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + 8);
				}
				else
				{
					return CW_UNKNOWN_STREAM;
				}
			}
			else
			{
				return CW_UNKNOWN_STREAM;
			}
			
			*true_ip = in_ip;
			
			/* Is capwap 802.3 icmp? */
			if (in_ip->protocol == IP_PROTOCOL_ICMP)
			{
				return CW_802_3_ICMP;
			}
			else
			{
				return CW_802_3_STREAM;
			}
		}
		else
		{
			log(DEBUG_LVL, "ipfwd_learn: unknown capwap head\n");
			return CW_UNKNOWN_STREAM;
		}
	}

	/* Skip CAPWAP control tunnel skb */
	else if ((uh->source == CW_CTL_STD_PORT || 
				uh->source == CW_CTL_TMP_PORT) &&
				(uh->dest == CW_CTL_AP_PORT))
	{
		log(DEBUG_LVL, "ipfwd_learn:skip control capwap udp source port %d, dest port %d\n",\
			                       uh->source, uh->dest);
		return CW_UNKNOWN_STREAM;
	}
	/* Skip Inter-AC roaming skb */
	else if ((uh->source == RAW_AC_PORT && uh->dest == RAW_AC_PORT) ||
				(uh->source == ROAM_AC_PORT && uh->dest == ROAM_AC_PORT))
	{
		log(DEBUG_LVL, "ipfwd_learn:skip Inter-AC roaming skb udp source port %d, dest port %d\n",\
			             uh->source, uh->dest);
		return CW_UNKNOWN_STREAM;
	}
	/*skip dhcp packet */
	else if (SPE_UDP_PORT(uh))
	{
		log(DEBUG_LVL, "ipfwd_learn :special udp source port %d, dest port %d\n", \
			               uh->source, uh->dest);
		return CW_UNKNOWN_STREAM;
	}

	/* Normal UDP skb */
	else 
	{
		return ETH_TYPE;
	}
}


/*************************************************************
 * Description:
 *          Learn  RPA header  information
 *
 * Parameter:
 *         fccp : fccp packet  header pointer
 *         skb  : packet skb buffer
 * Return:
 *         void
 *
 *************************************************************/
static inline void learn_rpa_tag(control_cmd_t *fccp,struct sk_buff *skb)
{
    eth_hdr_t *eth_hdr = (eth_hdr_t*)(skb->data);
    
    if(eth_hdr->h_vlan_proto == 0x8100)
    {
        rpa_vlan_eth_head_t *rpa_vlan_head  = (rpa_vlan_eth_head_t*)skb->data;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_vlan_type = rpa_vlan_head->vlan_type; /*0x8100*/
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_vlan_tag = rpa_vlan_head->vlan_tag;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_type = rpa_vlan_head->type;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_dnetdevNum = rpa_vlan_head->dnetdevNum;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_snetdevNum = rpa_vlan_head->snetdevNum;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_d_s_slotNum = rpa_vlan_head->d_s_slotNum;
        log(DEBUG_LVL,"learn_rpa_tag:\n"
            "vlantag:%x,type:%x,dnet:%x,snet:%x,d_s_slotNum:%x\n",
            rpa_vlan_head->vlan_tag,
            rpa_vlan_head->type,
            rpa_vlan_head->dnetdevNum,
            rpa_vlan_head->snetdevNum,
            rpa_vlan_head->d_s_slotNum);
    }
    else
    {
        rpa_eth_head_t *rpa_head  = (rpa_eth_head_t*)skb->data;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_type = rpa_head->type;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_dnetdevNum = rpa_head->dnetdevNum;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_snetdevNum = rpa_head->snetdevNum;
        fccp->fccp_data.rule_info.rule_param.rpa_header.rpa_d_s_slotNum = rpa_head->d_s_slotNum;
        log(DEBUG_LVL,"learn_rpa_tag:\n"
            "type:%x,dnet:%x,snet:%x,d_s_slotNum:%x\n",
            rpa_head->type,
            rpa_head->dnetdevNum,
            rpa_head->snetdevNum,
            rpa_head->d_s_slotNum);
    }
		
	return;
}

/****************************************************************
 * Description:
 *           Study capwap 802.3 stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag             :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *****************************************************************/
static inline int cw_802_3_cache_learned(struct sk_buff *skb, 
		struct iphdr *ip, 
		struct udphdr *uh, 
		uint16_t vtag1, 
		uint16_t vtag2, 
		uint16_t out_ether_type, 
		uint16_t inter_ether_type,
		uint64_t dsa_info,
		control_cmd_t *fccp_cmd,
		uint32_t *rpa_flag,
		uint8_t dev_type,
		uint16_t pppoe_session_id,
		uint16_t pppoe_flag
)
{

	uint8_t *in_l2_hd = NULL;
	struct iphdr *in_ip = NULL;
	struct tcphdr *in_th = NULL;
    int16_t fwd_port = -1;

	if(!skb || !ip || !uh || !fccp_cmd)
	{
		log(ERR_LVL, "ipfwd_learn:capwap 802.3 leanrned input argument is NULL\n");
		return IPFWD_LEARN_RETURN_FAIL;
	}


	in_l2_hd = ((uint8_t*)uh + UDP_H_LEN + CW_H_LEN);
	in_ip = (struct iphdr*)((uint8_t*)in_l2_hd + ETH_H_LEN);

	/*add by wangjian for support pppoe 2013-3-19 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		in_ip = (struct iphdr*)((uint8_t*)in_ip + PPPOE_H_LEN);
	}
	
	if (SPE_IP_HDR(in_ip) || SPE_IP_ADDR(in_ip->saddr, in_ip->daddr))
	{
		cvmx_atomic_add64(&fwd_learn_stats.cw8023_spe_ip, 1);
		log(DEBUG_LVL, "ipfwd_learn:Special IP version 0x%x, frag_off 0x%x, protocol 0x%0x, "
							"source ip %d.%d.%d.%d, "
							"dest ip %d.%d.%d.%d\n ",
							in_ip->version, in_ip->frag_off, in_ip->protocol, 
							IP_FMT(uh->source), IP_FMT(uh->dest));
		return IPFWD_LEARN_RETURN_FAIL;
	}

	in_th = (struct tcphdr*)((uint32_t*)in_ip + in_ip->ihl);
	if (SPE_UDP_PORT(in_th))
	{
		cvmx_atomic_add64(&fwd_learn_stats.cw8023_spe_udp_port, 1);
		log(DEBUG_LVL, "ipfwd_learn :skip special udp source port %d, dest port %d\n",\
			            in_th->source, in_th->dest);
		return IPFWD_LEARN_RETURN_FAIL;
	}

	/*dsa info infromation*/
	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;

	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
	                     (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
	                     (skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);


	
	fccp_cmd->fccp_data.rule_info.rule_param.ether_type = ETH_P_IP;
	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;
	
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;

	fccp_cmd->fccp_data.rule_info.rule_param.dip = in_ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = in_ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.dport = in_th->dest;
	fccp_cmd->fccp_data.rule_info.rule_param.sport = in_th->source;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = in_ip->protocol;	

	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_dmac, in_l2_hd, MAC_LEN);
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_smac, (in_l2_hd + MAC_LEN), MAC_LEN);
	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_ether = *((uint16_t*)(in_l2_hd + MAC_LEN*2));

	fccp_cmd->fccp_data.rule_info.cw_cache.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.tos = ip->tos;
	fccp_cmd->fccp_data.rule_info.cw_cache.dport = uh->dest;
	fccp_cmd->fccp_data.rule_info.cw_cache.sport = uh->source;	
	memcpy(fccp_cmd->fccp_data.rule_info.cw_cache.cw_hd, ((uint8_t*)uh + UDP_H_LEN), CW_H_LEN);

	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_CAP802_3_FORWARD;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_CAP802_3_FORWARD;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	/*get send port */
	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
		cvmx_atomic_add64(&fwd_learn_stats.cw8023_get_send_port, 1);
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;

    /* get usr table index */
//	rule_get_user(in_ip, fccp_cmd);
	log(DEBUG_LVL,"Learn CAPWAP 802.3 cache successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}


/*********************************************************************
 * Description:
 *  Study  capwap  802.11 stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag             :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *********************************************************************/
static inline int cw_cache_learned(struct sk_buff *skb, 
				struct iphdr *ip, 
				struct udphdr *uh, 
				uint16_t vtag1, 
				uint16_t vtag2, 
				uint16_t out_ether_type, 
				uint16_t inter_ether_type,
				uint64_t dsa_info,
				control_cmd_t *fccp_cmd,
				uint32_t*  rpa_flag,
				uint8_t dev_type,
				uint16_t pppoe_session_id,
				uint16_t pppoe_flag
)
{
	struct ieee80211_frame *ieee80211_hdr = NULL;
	struct iphdr *in_ip = NULL;
	struct tcphdr *in_th = NULL;
	uint8_t len = 0;
	uint8_t is_qos = 0;
    int16_t fwd_port = -1;

	if(!skb || !ip || !uh || !fccp_cmd){
		log(ERR_LVL, "ipfwd_learn :capwap 802.11 leanrned input argument is NULL\n");
		return IPFWD_LEARN_RETURN_FAIL;
	}


	ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN);
	if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) 
	{
		len = IEEE80211_QOS_H_LEN;
		is_qos = 1;
	} 
	else 
	{ 
		len = IEEE80211_H_LEN;
		is_qos = 0;
	}

	in_ip = (struct iphdr*)((uint8_t*)ieee80211_hdr + len + LLC_H_LEN);

	/*add by wangjian for support pppoe 2013-3-19 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		in_ip = (struct iphdr*)((uint8_t*)in_ip + PPPOE_H_LEN);
	}
	
	if (SPE_IP_HDR(in_ip) || SPE_IP_ADDR(in_ip->saddr, in_ip->daddr))
	{	
		cvmx_atomic_add64(&fwd_learn_stats.cw_spe_ip, 1);
		log(DEBUG_LVL, "ipfwd_learn:Special IP version 0x%x, frag_off 0x%x, "
			             "protocol 0x%0x, source ip %d.%d.%d.%d "
						 "dest ip %d.%d.%d.%d\n", 
						 in_ip->version, in_ip->frag_off, in_ip->protocol, 
						 IP_FMT(uh->source), IP_FMT(uh->dest));
		return IPFWD_LEARN_RETURN_FAIL;
	}
	in_th = (struct tcphdr*)((uint32_t*)in_ip + in_ip->ihl);
	if (SPE_UDP_PORT(in_th))
	{
		cvmx_atomic_add64(&fwd_learn_stats.cw_spe_udp_port, 1);
		log(DEBUG_LVL, "ipfwd_learn :skip special udp source port %d, dest port %d\n", \
					           in_th->source, in_th->dest);
		return IPFWD_LEARN_RETURN_FAIL;
	}
	/*dsa info infromation*/
	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;
	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
		            (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
					(skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);

	fccp_cmd->fccp_data.rule_info.rule_param.ether_type = ETH_P_IP;

	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;

	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;

	fccp_cmd->fccp_data.rule_info.rule_param.dip = in_ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = in_ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.dport = in_th->dest;
	fccp_cmd->fccp_data.rule_info.rule_param.sport = in_th->source;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = in_ip->protocol;

	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[0] = ieee80211_hdr->i_fc[0];
	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[1] = ieee80211_hdr->i_fc[1]; 

	if (is_qos) 
	{
		fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[0] = \
					                ((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[0];
		fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[1] = \
					                ((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[1];
	}

	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.tunnel_l2_header.wifi_header.addr, \
			         ieee80211_hdr->i_addr1, MAC_LEN * 3); 


	fccp_cmd->fccp_data.rule_info.cw_cache.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.tos = ip->tos;
	fccp_cmd->fccp_data.rule_info.cw_cache.dport = uh->dest;
	fccp_cmd->fccp_data.rule_info.cw_cache.sport = uh->source;	
	memcpy(fccp_cmd->fccp_data.rule_info.cw_cache.cw_hd, ((uint8_t*)uh + UDP_H_LEN), CW_H_LEN);
	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_CAPWAP_FORWARD;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_CAPWAP_FORWARD;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	/*get send port */
	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
		cvmx_atomic_add64(&fwd_learn_stats.cw_get_send_port, 1);
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;

	/* get usr table index */
//	rule_get_user(in_ip, fccp_cmd);
	log(DEBUG_LVL,"Learn CAPWAP 802.11 cache successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}

/*********************************************************************
 * Description:
 *  Study  802.3  stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag             :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *********************************************************************/

static inline int eth_cache_learned(struct sk_buff *skb, struct iphdr *ip, 
				struct tcphdr *th, 
				uint16_t vtag1, 
				uint16_t vtag2,
				uint16_t out_ether_type, 
				uint16_t inter_ether_type,
				uint64_t dsa_info,
				control_cmd_t *fccp_cmd,
				uint32_t *rpa_flag,
				uint8_t dev_type,
				uint16_t pppoe_session_id,
				uint16_t pppoe_flag
)
{	
    int16_t fwd_port = -1;
    
	if(!skb || !ip || !th || !fccp_cmd || !(skb->dev))
	{
		log(ERR_LVL, "ethernet leanrned input argument is NULL\n");			
		return IPFWD_LEARN_RETURN_FAIL;
	}

	if (SPE_UDP_PORT(th))
	{
		cvmx_atomic_add64(&fwd_learn_stats.spe_udp_port, 1);
		log(DEBUG_LVL, "ipfwd_learn :dhcp packet:udp source port %d, dest port %d\n",\
		               th->source, th->dest);
		return IPFWD_LEARN_RETURN_FAIL;
	}

	/*get DMAC addr*/
	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;

	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
		             (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
					(skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);

	/*get Ethernet type*/
	/*add by wangjian for support pppoe 2013-3-19 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		fccp_cmd->fccp_data.rule_info.rule_param.ether_type = PPPOE_TYPE;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.ether_type = ETH_P_IP;
	}

	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;
	
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;

	fccp_cmd->fccp_data.rule_info.rule_param.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.dport = th->dest;
	fccp_cmd->fccp_data.rule_info.rule_param.sport = th->source;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = ip->protocol;

	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_ETH_FORWARD;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_ETH_FORWARD;
	}

		
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	/*get send port */
	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
		cvmx_atomic_add64(&fwd_learn_stats.get_send_port, 1);
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;

	/* get usr table index */
//	rule_get_user(ip, fccp_cmd);

	log(DEBUG_LVL,"Learn ETH  successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}

/*********************************************************************
 * Description:
 *  Study 802.3  icmp  stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag             :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *********************************************************************/

static inline int eth_icmp_cache_learned(struct sk_buff *skb, struct iphdr *ip,  
				uint16_t vtag1, 
				uint16_t vtag2,
				uint16_t out_ether_type, 
				uint16_t inter_ether_type,
				uint64_t dsa_info,
				control_cmd_t *fccp_cmd,
				uint32_t  *rpa_flag,
				uint8_t dev_type,
				uint16_t pppoe_session_id,
				uint16_t pppoe_flag
			)
{	
    int16_t fwd_port = -1;
    
	if(!skb || !ip  || !fccp_cmd || !(skb->dev)){
		log(ERR_LVL, "ipfwd_learn:ethernet icmp leanrned input argument is NULL\n");			
		return IPFWD_LEARN_RETURN_FAIL;
	}

	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;

	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
		             (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
					(skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);

	/*get Ethernet type*/
	/*add by wangjian for support pppoe 2013-4-9 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		fccp_cmd->fccp_data.rule_info.rule_param.ether_type = PPPOE_TYPE;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.ether_type = ETH_P_IP;
	}
	
	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;	

	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;
	
	fccp_cmd->fccp_data.rule_info.rule_param.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = ip->protocol;
	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_ICMP;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_ICMP;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	/*get send port */
	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;

    /* get usr table index */
//	rule_get_user(ip, fccp_cmd);
	log(DEBUG_LVL,"Learn ETH ICMP successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}
/*********************************************************************
 * Description:
 *  Study capwap  802.3  icmp  stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag             :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *********************************************************************/

static inline int cw_802_3_icmp_cache_learned(struct sk_buff *skb, 
				struct iphdr *ip,  
				uint16_t vtag1, 
				uint16_t vtag2, 
				uint16_t out_ether_type, 
				uint16_t inter_ether_type,
				uint64_t dsa_info,
				control_cmd_t *fccp_cmd,
				uint32_t  *rpa_flag,
				uint8_t dev_type,
				uint16_t pppoe_session_id,
				uint16_t pppoe_flag)
{
	uint8_t *in_l2_hd = NULL;
	struct iphdr *in_ip = NULL;
	struct udphdr *uh=NULL;
    int16_t fwd_port = -1;

	if(!skb || !ip  || !fccp_cmd)
	{
			log(ERR_LVL, "ipfwd_learn:capwap 802.3 icmp leanrned input argument is NULL\n");
			return IPFWD_LEARN_RETURN_FAIL;
	}
	uh = (struct udphdr*)((uint32_t *)ip + ip->ihl);
	in_l2_hd = ((uint8_t*)uh + UDP_H_LEN + CW_H_LEN);
	in_ip = (struct iphdr*)((uint8_t*)in_l2_hd + ETH_H_LEN);

	/*add by wangjian for support pppoe 2013-4-9 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		in_ip = (struct iphdr*)((uint8_t*)in_ip + PPPOE_H_LEN);
	}
	
	if (SPE_IP_HDR(in_ip) || SPE_IP_ADDR(in_ip->saddr, in_ip->daddr)) 
	{
		log(DEBUG_LVL, "Special IP version 0x%x, frag_off 0x%x, protocol 0x%0x,"
				       "source ip %d.%d.%d.%d "
					   "dest ip %d.%d.%d.%d\n",\
					   in_ip->version, in_ip->frag_off, in_ip->protocol,\
					   IP_FMT(uh->source), IP_FMT(uh->dest));
		return IPFWD_LEARN_RETURN_FAIL;
	}

	/*dsa info infromation*/
	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;

	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
		             (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
					(skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);

	/*get Ethernet type*/
	fccp_cmd->fccp_data.rule_info.rule_param.ether_type = ETH_P_IP;
	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;

	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;
	
	fccp_cmd->fccp_data.rule_info.rule_param.dip = in_ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = in_ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = in_ip->protocol;	
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_dmac, in_l2_hd, MAC_LEN);
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_smac, (in_l2_hd + MAC_LEN), MAC_LEN);
	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_eth_header_ether = *((uint16_t*)(in_l2_hd + MAC_LEN*2));
	fccp_cmd->fccp_data.rule_info.cw_cache.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.tos = ip->tos;
	fccp_cmd->fccp_data.rule_info.cw_cache.dport = uh->dest;
	fccp_cmd->fccp_data.rule_info.cw_cache.sport = uh->source;	
	memcpy(fccp_cmd->fccp_data.rule_info.cw_cache.cw_hd, ((uint8_t*)uh + UDP_H_LEN), CW_H_LEN);
	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_CAPWAP_802_3_ICMP;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_CAPWAP_802_3_ICMP;
	}
	
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;
//	rule_get_user(in_ip, fccp_cmd);
	log(DEBUG_LVL,"Learn CAPWAP 802.3 cache successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}
/*********************************************************************
 * Description:
 *  Study capwap  802.11  icmp  stream cache data.
 *
 * Parameter:
 *           skb                    :    packet skb
 *           ip                      :    external IP
 *           uh                     :    external IP
 *           vtag1                 :    external vlan tag 
 *           vtag2                 :    inter vlan tag 
 *           out_ether_type    :    external eth type
 *           inter_ether_type  :    inter  eth type
 *           dsa_info             :    dsa tag information
 *           fccp_cmd            :    fccp packet pointer
 *           rpa_flag              :    rpa tag if there are
 *           dev_type           :    DEV_TYPE_OCTEON_ETH or DEV_TYPE_OCTEON_PCIE
 *
 * Return:
 *          IPFWD_LEARN_RETURN_OK  :  successfully
 *          IPFWD_LEARN_RETURN_FAIL:  failed
 *
 *********************************************************************/
static inline int cw_802_11_icmp_cache_learned(struct sk_buff *skb, 
				struct iphdr *ip, 
				uint16_t vtag1, 
				uint16_t vtag2, 
				uint16_t out_ether_type, 
				uint16_t inter_ether_type,
				uint64_t dsa_info,
				control_cmd_t *fccp_cmd,
				uint32_t *rpa_flag,
				uint8_t dev_type,
				uint16_t pppoe_session_id,
				uint16_t pppoe_flag)
{
	struct ieee80211_frame *ieee80211_hdr = NULL;
	struct iphdr *in_ip = NULL;
	struct tcphdr *in_th = NULL;
	struct udphdr *uh=NULL;
	uint8_t len = 0;
	uint8_t is_qos = 0;
    int16_t fwd_port = -1;
	if(!skb || !ip || !fccp_cmd)
	{
		log(ERR_LVL, "ipfwd_learn:cw_802_11_icmp leanrned input argument is NULL\n");
		return IPFWD_LEARN_RETURN_FAIL;
	}
	uh=(struct udphdr *)((uint8_t*)ip + IP_H_LEN);
	ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)uh + UDP_H_LEN + CW_H_LEN);
	if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) 
	{
		len = IEEE80211_QOS_H_LEN;
		is_qos = 1;
	}
	else 
	{ 
		len = IEEE80211_H_LEN;
		is_qos = 0;
	}
	in_ip = (struct iphdr*)((uint8_t*)ieee80211_hdr + len + LLC_H_LEN);

	/*add by wangjian for support pppoe 2013-4-9 ip need judge whether ip maybe pppoe*/
	if (1 == pppoe_flag)
	{
		in_ip = (struct iphdr*)((uint8_t*)in_ip + PPPOE_H_LEN);
	}
	
	if (SPE_IP_HDR(in_ip) || SPE_IP_ADDR(in_ip->saddr, in_ip->daddr)) 
	{	
		log(DEBUG_LVL, "Special IP version 0x%x, frag_off 0x%x, protocol 0x%0x,"
			           "source ip %d.%d.%d.%d "
					   "dest ip %d.%d.%d.%d\n",\
					   in_ip->version, in_ip->frag_off, in_ip->protocol,\
					   IP_FMT(uh->source), IP_FMT(uh->dest));
		return IPFWD_LEARN_RETURN_FAIL;
	}
	in_th = (struct tcphdr*)((uint32_t*)in_ip + in_ip->ihl);

	/*dsa info infromation*/
	fccp_cmd->fccp_data.rule_info.rule_param.dsa_info = dsa_info;

	/*get DMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_dhost, \
		             (skb->data + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	/*get SMAC addr*/
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.ether_shost, \
					(skb->data + MAC_LEN + (IS_RPA_PKT(*rpa_flag)?RPA_HEAD_LEN(*rpa_flag):0)), MAC_LEN);
	fccp_cmd->fccp_data.rule_info.rule_param.ether_type = 0x0800;
	fccp_cmd->fccp_data.rule_info.rule_param.out_ether_type = out_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.in_ether_type = inter_ether_type;
	fccp_cmd->fccp_data.rule_info.rule_param.out_tag = vtag1;
	fccp_cmd->fccp_data.rule_info.rule_param.in_tag = vtag2;

	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_flag = pppoe_flag;
	fccp_cmd->fccp_data.rule_info.rule_param.pppoe_session_id = pppoe_session_id;
	
	fccp_cmd->fccp_data.rule_info.rule_param.dip = in_ip->daddr;
	fccp_cmd->fccp_data.rule_info.rule_param.sip = in_ip->saddr;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = in_ip->protocol;
	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[0] = ieee80211_hdr->i_fc[0];
	fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_fc[1] = ieee80211_hdr->i_fc[1]; 
	if (is_qos) 
	{
		fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[0] = \
			                              ((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[0];
		fccp_cmd->fccp_data.rule_info.rule_param.acl_tunnel_wifi_header_qos[1] = \
			                              ((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[1];
	}
	memcpy(fccp_cmd->fccp_data.rule_info.rule_param.tunnel_l2_header.wifi_header.addr, \
		                                  ieee80211_hdr->i_addr1, MAC_LEN * 3); 
	fccp_cmd->fccp_data.rule_info.cw_cache.dip = ip->daddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.sip = ip->saddr;
	fccp_cmd->fccp_data.rule_info.cw_cache.tos = ip->tos;
	fccp_cmd->fccp_data.rule_info.cw_cache.dport = uh->dest;
	fccp_cmd->fccp_data.rule_info.cw_cache.sport = uh->source;	
	memcpy(fccp_cmd->fccp_data.rule_info.cw_cache.cw_hd, ((uint8_t*)uh + UDP_H_LEN), CW_H_LEN);
	if(IS_RPA_PKT(*rpa_flag))
	{
		learn_rpa_tag(fccp_cmd,skb);
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_RPA_CAPWAP_802_11_ICMP;
	}
	else
	{
		fccp_cmd->fccp_data.rule_info.rule_param.action_type = FLOW_ACTION_CAPWAP_802_11_ICMP;
	}
	
	fccp_cmd->fccp_data.rule_info.rule_param.rule_state = RULE_IS_LEARNED;	

	/*get send port */
	if((fwd_port = get_forward_port(skb,dev_type)) == IPFWD_LEARN_RETURN_FAIL)
	{
        log(DEBUG_LVL,"ipfwd_learn get_forward_port error\n");
        return IPFWD_LEARN_RETURN_FAIL;
	}
	fccp_cmd->fccp_data.rule_info.rule_param.forward_port = fwd_port;

//	rule_get_user(in_ip, fccp_cmd);
	
	log(DEBUG_LVL,"Learn CAPWAP 802.11 icmp cache successfully.\n");
	if(log_level == DEBUG_LVL)
	{
		dump_fccp_packet(fccp_cmd);
	}
	return IPFWD_LEARN_RETURN_OK;
}

static inline void
nat_cache_learned(struct net *net, control_cmd_t *fccp_cmd, struct iphdr *ip) {
	struct nf_conntrack_tuple tuple, *t;	
	struct nf_conntrack_tuple_hash *h;
	struct nf_conn *ct;
	struct tcphdr *th;

	if (unlikely(!ip))
		return;

	if (ip->protocol != IP_PROTOCOL_TCP && ip->protocol != IP_PROTOCOL_UDP)
		return;

	th = (struct tcphdr *)((uint32_t*)ip + ip->ihl);

	memset(&tuple, 0, sizeof(tuple));
	
	tuple.src.l3num = PF_INET;
	tuple.dst.protonum = ip->protocol;
	tuple.dst.dir = IP_CT_DIR_ORIGINAL;

	/* invert NAT ip and port for conntrack find */
	tuple.src.u3.ip = ip->daddr;
	tuple.dst.u3.ip = ip->saddr;
	tuple.src.u.all = th->dest;
	tuple.dst.u.all = th->source;

	h = nf_conntrack_find_get(net, &tuple);
	if (!h) {
		log(DEBUG_LVL, "nat_cache_learned: conntrack find failed, "
						"%s, src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u\n",
						tuple.dst.protonum == IP_PROTOCOL_TCP ? "TCP" : "UDP",
						NIPQUAD(tuple.src.u3.ip), tuple.src.u.all, 
						NIPQUAD(tuple.dst.u3.ip), tuple.dst.u.all);
		return;
	}

	ct = nf_ct_tuplehash_to_ctrack(h);
	
	/* identify if occured snat or dnat ? test bit IPS_NAT_MASK and IPS_NAT_DONE_MASK */
	if (!(ct->status & IPS_NAT_MASK) || !(ct->status & IPS_NAT_DONE_MASK))
		goto out;

	if (ip->protocol == IP_PROTOCOL_TCP) {
		log(DEBUG_LVL, "nat_cache_learned: TCP: src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u "
						"[%s%s%s%s], ct->status %lu, ct->proto.tcp.state %u\n",
						NIPQUAD(ip->saddr), th->source, NIPQUAD(ip->daddr), th->dest,
						th->syn ? "SYN " : "", th->fin ? "FIN " : "", 
						th->rst ? "RST " : "", th->ack ? "ACK" : "",
						ct->status, ct->proto.tcp.state);
	} else if (ip->protocol == IP_PROTOCOL_UDP) {
		log(DEBUG_LVL, "nat_cache_learned: UDP: src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u ct->status %lu\n",
						NIPQUAD(ip->saddr), th->source, NIPQUAD(ip->daddr), th->dest, ct->status);
	}

	/* identify if conntrack assured ? test bit IPS_ASSURED_BIT */
	if (!test_bit(IPS_ASSURED_BIT, &ct->status))
		goto out;

	t = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	if (nf_ct_tuple_equal(t, &tuple)) {
		t = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
	}

	fccp_cmd->fccp_data.rule_info.rule_param.sip = t->src.u3.ip;
	fccp_cmd->fccp_data.rule_info.rule_param.dip = t->dst.u3.ip;	
	fccp_cmd->fccp_data.rule_info.rule_param.sport = t->src.u.all;
	fccp_cmd->fccp_data.rule_info.rule_param.dport = t->dst.u.all;
	fccp_cmd->fccp_data.rule_info.rule_param.protocol = t->dst.protonum;

	fccp_cmd->fccp_data.rule_info.rule_param.nat_flag = 1;
	fccp_cmd->fccp_data.rule_info.rule_param.nat_sip = tuple.dst.u3.ip;
	fccp_cmd->fccp_data.rule_info.rule_param.nat_dip = tuple.src.u3.ip;
	fccp_cmd->fccp_data.rule_info.rule_param.nat_sport = tuple.dst.u.all;
	fccp_cmd->fccp_data.rule_info.rule_param.nat_dport = tuple.src.u.all; 

	log(DEBUG_LVL, "nat_cache_learned: %s: %s: src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u "
					"src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u\n",
					test_bit(IPS_SRC_NAT_DONE_BIT, &ct->status) ? "SNAT" : "DNAT",	
					t->dst.protonum == IP_PROTOCOL_TCP ? "TCP" : "UDP",
					NIPQUAD(t->src.u3.ip), t->src.u.all, NIPQUAD(t->dst.u3.ip), t->dst.u.all,
					NIPQUAD(tuple.dst.u3.ip), tuple.dst.u.all, NIPQUAD(tuple.src.u3.ip), tuple.src.u.all);
out:
	nf_ct_put(ct);
}




/**********************************************************************************
 *  ipfwd_learn
 *
 *	DESCRIPTION:
 * 		fast-fwd learn module entry. send FCCP to fast-fwd module for learning acl rule.
 *
 *	INPUT:
 *		skb : skb buffer
 *		dev_type : identify the caller of different net_device driver
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		IPFWD_LEARN_RETURN_OK
 *		IPFWD_LEARN_RETURN_FAIL
 *				
 **********************************************************************************/
int ipfwd_learn (struct sk_buff *skb,uint8_t dev_type)
{	 
	control_cmd_t* fccp_cmd = NULL;

	struct iphdr *ip = NULL;
	struct tcphdr *th = NULL;
	struct iphdr *true_ip = NULL;
	uint16_t vtag1 = 0, vtag2 = 0;
	int8_t skb_type = 0;
	uint8_t *pkt_buffer = NULL;
	uint8_t *pkt_ptr = NULL;
	eth_hdr_t* eth = NULL;
	uint64_t dsa_info = 0;	
	uint16_t out_ether_type = 0, inter_ether_type = 0;
	uint32_t rpa_flag = 0;
	uint16_t pppoe_flag = 0;
	uint16_t pppoe_session_id = 0;




	cvmx_wqe_t *work = NULL;
#ifdef IPFWD_LEARN_MODE_STANDALONE
	struct sk_buff *fccp_skb = NULL;
#endif
	
	if(!learn_enable)
	{
		return IPFWD_LEARN_RETURN_OK;
	}
	if(!skb)
	{
		log(DEBUG_LVL, "ipfwd_learn: input skb is NULL\n");
		return IPFWD_LEARN_RETURN_OK;
	}	
	
	if(DEBUG_LVL == log_level)
	{
		dump_skb(skb);
	}

	cvmx_atomic_add64(&fwd_learn_stats.total_recv_cnt, 1);	

	/* the pkt was not receive from internet, generat by AC */
	//rpa packet skb->iif = 0
	/*
	if (!skb->iif)
    {
    	log(DEBUG_LVL, "skb->iif = 0!\n");
        return IPFWD_LEARN_RETURN_OK;
    }
	*/
	if(!(skb->dev))
	{
		cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
		log(DEBUG_LVL,"ipfwd_learn: input skb->dev is NULL\n");
		return IPFWD_LEARN_RETURN_OK;
	}

	if((autelan_product_info.product_type != AUTELAN_PRODUCT_AX7605I) && IS_OBC_DEV(skb->dev->name))
    {
    	cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
    	log(DEBUG_LVL, "product_type != AUTELAN_PRODUCT_AX7605I && IS_OBC_DEV !\n");
        return IPFWD_LEARN_RETURN_OK;
    }
	
#ifdef BUILDNO_VERSION2_1
    if( autelan_product_info.board_type == AUTELAN_BOARD_AX81_SMU || \
		autelan_product_info.board_type == AUTELAN_BOARD_AX81_AC8C || \
		autelan_product_info.board_type == AUTELAN_BOARD_AX81_AC12C)
#else
	if( product_info.id == AX_81_AC8C || product_info.id == AX_81_AC12C )
#endif
	{
		if ((SPE_IN_IF_86(skb->dev->name)) || SPE_OT_IF(skb->dev->name))
		{
			cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
			log(DEBUG_LVL,"ipfwd_learn : skip special interface:%s\n",skb->dev->name);
			return IPFWD_LEARN_RETURN_OK;
		}	
	}
    else
    {
	    /* Skip special interface */
	    if ((SPE_IN_IF_86(skb->dev->name)) || SPE_OT_IF(skb->dev->name))
		{
			cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
			log(DEBUG_LVL,"ipfwd_learn: skip special interface:%s\n",skb->dev->name);
		    return IPFWD_LEARN_RETURN_OK;
	    }	
    }

	/* get fwd dev type, PCIe port or normal eth port */
	/* ipfwd_learn add module for Module call identification by fastfwd group */
    if(dev_type == DEV_TYPE_OCTEON_ETH)
    {
        if(!(ipfwd_learn_mode & IPFWD_LEARN_MODE_COEXIST_ENABLE))
        {
        	cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
            return IPFWD_LEARN_RETURN_OK;
        }
    }
    else if(dev_type == DEV_TYPE_OCTEON_PCIE)
    {
        if(!(ipfwd_learn_mode & IPFWD_LEARN_MODE_STANDALONE_ENABLE))
        {
        	cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
            return IPFWD_LEARN_RETURN_OK;
        }
    }
	else
	{
		cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
		log(DEBUG_LVL,"ipfwd_learn: dev_type not support!\n");
		return IPFWD_LEARN_RETURN_OK;
	}

    

	/* Decap Layer2 header ,get ip head address*/
	if (IPFWD_LEARN_RETURN_OK != tx_l2hdr_parse(skb, &ip, &vtag1, &vtag2, \
		                                          &out_ether_type,&inter_ether_type,&rpa_flag, &pppoe_session_id, &pppoe_flag) )
	{
		cvmx_atomic_add64(&fwd_learn_stats.l2hdr_parse_fail, 1);
		return IPFWD_LEARN_RETURN_OK;
	}	

	/* Skip special IP addr */
	if (SPE_IP_HDR(ip) || SPE_IP_ADDR(ip->saddr, ip->daddr))
	{
		cvmx_atomic_add64(&fwd_learn_stats.spe_ip, 1);
		log(DEBUG_LVL,"ipfwd_learn: skip SPE_IP_HDR(ip) or SPE_IP_ADDR(ip->saddr, ip->daddr)\n");

		return IPFWD_LEARN_RETURN_OK;
	}


	//	if(IPFWD_LEARN_RETURN_FAIL == get_dsa_info(skb, &dsa_info))
	//	{
	//		return IPFWD_LEARN_RETURN_FAIL;
	//	}

	true_ip = ip;
	if(ip->protocol == IP_PROTOCOL_ICMP)
	{
		skb_type = ETH_ICMP;
	}
	else if (ip->protocol == IP_PROTOCOL_TCP)
	{
		th = (struct tcphdr *)((uint32_t*)ip + ip->ihl);
		skb_type = ETH_TYPE;
	}
	else if(ip->protocol == IP_PROTOCOL_UDP)
	{
		th = (struct tcphdr *)((uint32_t*)ip + ip->ihl);
		skb_type = tx_udp_decap((struct udphdr *)th,&true_ip,&pppoe_session_id,&pppoe_flag);
		if(CW_UNKNOWN_STREAM == skb_type)
		{
			cvmx_atomic_add64(&fwd_learn_stats.udp_decap_fail, 1);
			log(DEBUG_LVL,"ipfwd_learn: tx_udp_decap error\n");
			return IPFWD_LEARN_RETURN_OK;
		}
		if(ETH_TYPE == skb_type)
		{
            true_ip = ip;
		}
	}
	else
	{	
		cvmx_atomic_add64(&fwd_learn_stats.pkt_type_nonsupport, 1);
		log(DEBUG_LVL,"ipfwd_learn: not udp tcp icmp packet\n");
		return IPFWD_LEARN_RETURN_FAIL;
	}

	if((skb_type == ETH_ICMP ||skb_type == CW_802_3_ICMP ||skb_type == CW_802_11_ICMP)&&\
		    icmp_enable==FUNC_DISABLE)
	{
		cvmx_atomic_add64(&fwd_learn_stats.icmp, 1);
		log(ICMP_DEBUG,"ipfwd_learn:icmp learned disable,return\n");
		return IPFWD_LEARN_RETURN_OK;
	}

	if(dev_type == DEV_TYPE_OCTEON_ETH)
	{
		/* Get a work queue entry */
		work = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
		if (unlikely(NULL == work))
		{
			cvmx_atomic_add64(&fwd_learn_stats.wqe_alloc_fail, 1);
			log(INFO_LVL, "ipfwd_learn:fpa alloc work fail\n");
			return IPFWD_LEARN_RETURN_OK;
		}	

		/* Get a fccp buffer */
		pkt_buffer = cvmx_fpa_alloc(CVMX_FPA_PACKET_POOL);
		if (unlikely(NULL == pkt_buffer))
		{
			cvmx_atomic_add64(&fwd_learn_stats.pkt_alloc_fail, 1);
			log_count ++;
			if(0 == (log_count % LOG_PRINT_HZ) )
			{
				log(INFO_LVL,"ipfwd_learn:fpa alloc fccp_buf fail,fail count = %llu\n",log_count);
			}
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, DONT_WRITEBACK(1));		
			return IPFWD_LEARN_RETURN_OK;
		}

		pkt_ptr = pkt_buffer + sizeof(uint64_t);
		pkt_ptr += ((CVMX_HELPER_FIRST_MBUFF_SKIP+7)&0xfff8) + 6;
		memset(pkt_ptr, 0, FCCP_PACKET_LEN);	
	}
	else if(dev_type == DEV_TYPE_OCTEON_PCIE)
	{
#ifdef IPFWD_LEARN_MODE_STANDALONE 	
		/* alloc skb */
		fccp_skb = dev_alloc_skb(FCCP_PACKET_LEN + 20);
		if(unlikely(!fccp_skb))
		{
			cvmx_atomic_add64(&fwd_learn_stats.pkt_alloc_fail, 1);
			log(INFO_LVL, "alloc skb fail\n");
			return IPFWD_LEARN_RETURN_OK;
		}
		memset(fccp_skb->data, 0, FCCP_PACKET_LEN + 20);
		skb_reserve(fccp_skb, 10);
		skb_put(fccp_skb, FCCP_PACKET_LEN);
		pkt_ptr = fccp_skb->data;
#else
        return IPFWD_LEARN_RETURN_OK;
#endif        
	}
	else
	{
		cvmx_atomic_add64(&fwd_learn_stats.spe_if_name, 1);
		log(DEBUG_LVL,"ipfwd_learn:unknown fwd dev type, dev_type = %d\n", dev_type);
		return IPFWD_LEARN_RETURN_OK;
	}

	eth = (eth_hdr_t*)pkt_ptr;
	eth->h_vlan_proto = FCCP_L2_ETH_TYPE;
	fccp_cmd = (control_cmd_t*)(pkt_ptr + ETH_H_LEN);

	/* learn Eth cache */
	if (skb_type == ETH_TYPE) 
	{		
		if(eth_cache_learned(skb, ip, th, vtag1, vtag2, out_ether_type, \
			inter_ether_type, dsa_info, fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.eth_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.eth_good_cnt, 1);
	}
	/* Study CAPWAP 802.11 cache */
	else if (skb_type == CW_802_11_STREAM)
	{		
		if(cw_cache_learned(skb, ip, (struct udphdr*)th, vtag1, vtag2, out_ether_type,\
			                  inter_ether_type, dsa_info,\
			                  fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.cw_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.cw_good_cnt, 1);
	}
	/* Study CAPWAP 802.3 cache */
	else if (skb_type == CW_802_3_STREAM)
	{			
		if(cw_802_3_cache_learned(skb, ip, (struct udphdr*)th, vtag1, vtag2,  out_ether_type,\
			                  inter_ether_type, dsa_info,\
			                  fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.cw8023_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.cw8023_good_cnt, 1);
	}
	else if (skb_type == ETH_ICMP)
	{
		if(eth_icmp_cache_learned(skb, ip, vtag1, vtag2,  out_ether_type,\
			                        inter_ether_type, dsa_info,\
			                        fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.icmp_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.icmp_good_cnt, 1);
	}
	else if (skb_type == CW_802_3_ICMP)
	{
		if(cw_802_3_icmp_cache_learned(skb, ip, vtag1, vtag2,  out_ether_type,\
			                             inter_ether_type, dsa_info,\
			                             fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.cw8023_icmp_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.cw8023_icmp_good_cnt, 1);
	}
	else if (skb_type == CW_802_11_ICMP)
	{
		if(cw_802_11_icmp_cache_learned(skb, ip,  vtag1, vtag2,  out_ether_type,\
			                             inter_ether_type, dsa_info,\
			                             fccp_cmd,&rpa_flag,dev_type,pppoe_session_id,pppoe_flag) == IPFWD_LEARN_RETURN_FAIL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.cw_icmp_learn_fail, 1);
            goto free_buf;
		}
		cvmx_atomic_add64(&fwd_learn_stats.cw_icmp_good_cnt, 1);
	}
	else
	{	
		cvmx_atomic_add64(&fwd_learn_stats.pkt_type_nonsupport, 1);
		log(ERR_LVL,"ipfwd_learn:unknown skb_type\n");
        goto free_buf;
	}

	/* lixiang modify NAT ipfwd_learn: 20130412 */
	nat_cache_learned(dev_net(skb->dev), fccp_cmd, true_ip);

	/* Filter local ip if nat_flag == 1 dont filter */
	if (0 == fccp_cmd->fccp_data.rule_info.rule_param.nat_flag)
	{
	    uint32_t sip = fccp_cmd->fccp_data.rule_info.rule_param.sip;
	    uint32_t dip = fccp_cmd->fccp_data.rule_info.rule_param.dip;
	    
	    if(fwd_ifa_table_lookup(sip) != NULL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.local_sip, 1);	
        	log(DEBUG_LVL, "sip=%d.%d.%d.%d is local ip, skip\n", IP_FMT(sip));
        	goto free_buf;
    	}
	    if(fwd_ifa_table_lookup(dip) != NULL)
		{
			cvmx_atomic_add64(&fwd_learn_stats.local_dip, 1);
	        log(DEBUG_LVL, "dip=%d.%d.%d.%d is local ip, skip\n", IP_FMT(dip));
	        goto free_buf;
	    }
	}
	

	fccp_cmd->dest_module = FCCP_MODULE_ACL;
	fccp_cmd->src_module = FCCP_MODULE_AGENT_ACL;
	if(skb_type == CW_802_11_ICMP || skb_type == CW_802_3_ICMP || skb_type == ETH_ICMP)
	{
		cvmx_atomic_add64(&fwd_learn_stats.icmp, 1);
		fccp_cmd->cmd_opcode = FCCP_CMD_INSERT_ICMP;
	}
	else
	{
		fccp_cmd->cmd_opcode = FCCP_CMD_INSERT;
	}
	fccp_cmd->cmd_len = sizeof(control_cmd_t);

	if(dev_type == DEV_TYPE_OCTEON_ETH)
	{
		/* Fill in some of the work queue fields. We may need to add more
		   if the software at the other end needs them */

		CVM_WQE_SET_LEN(work,FCCP_PACKET_LEN);
		CVM_WQE_SET_PORT(work,0);
		CVM_WQE_SET_QOS(work,0);
		CVM_WQE_SET_GRP(work,POW_SEND_GROUP);
		CVM_WQE_SET_TAG_TYPE(work,CVMX_POW_TAG_TYPE_NULL);
		CVM_WQE_SET_TAG(work,POW_SEND_GROUP);
		work->word2.u64     = 0;    /* Default to zero. Sets of zero later are commented out */
		work->word2.s.bufs  = 1;
		work->packet_ptr.u64 = 0;
		work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);

		work->packet_ptr.s.pool = CVMX_FPA_PACKET_POOL;
		work->packet_ptr.s.size = CVMX_FPA_PACKET_POOL_SIZE;
		work->packet_ptr.s.back = (pkt_ptr - pkt_buffer)>>7; 

		work->word2.snoip.not_IP        = 1; /* IP was done up above */
		CVM_WQE_SET_UNUSED(work,FCCP_WQE_TYPE);
		/* Submit the packet to the POW */
		cvmx_pow_work_submit(work, CVM_WQE_GET_TAG(work),\
		                        CVM_WQE_GET_TAG_TYPE(work), \
		                        CVM_WQE_GET_QOS(work), \
		                        CVM_WQE_GET_GRP(work));
		cvmx_atomic_add64(&fwd_learn_stats.total_good_cnt, 1);
		return IPFWD_LEARN_RETURN_OK;
	}
	else if(dev_type == DEV_TYPE_OCTEON_PCIE)
	{
#ifdef IPFWD_LEARN_MODE_STANDALONE 	
		octnet_fccp_xmit(fccp_skb, skb->dev);
#endif		
		cvmx_atomic_add64(&fwd_learn_stats.total_good_cnt, 1);
		return IPFWD_LEARN_RETURN_OK;
	}


free_buf:
    if(dev_type == DEV_TYPE_OCTEON_ETH)
	{
        cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, DONT_WRITEBACK(1));
        cvmx_fpa_free(pkt_buffer, CVMX_FPA_PACKET_POOL, DONT_WRITEBACK(1));
        return IPFWD_LEARN_RETURN_OK;    	
	}
    else if(dev_type == DEV_TYPE_OCTEON_PCIE)
    {
#ifdef IPFWD_LEARN_MODE_STANDALONE 	    
        dev_kfree_skb_any(fccp_skb);
#endif
        return IPFWD_LEARN_RETURN_OK; 
    }

	return IPFWD_LEARN_RETURN_OK;
}



static int __init ipfwd_learn_init(void)
{		
	uint64_t val = 0;
#ifdef IPFWD_LEARN_MODE_STANDALONE    
    struct net_device** oct_dev_array = NULL;
    int32_t nic_ifcount = 0;
    int32_t i = 0;

    /* detect standalone fastfwd */
    nic_ifcount = octnet_get_nic_ifcount();
    if((nic_ifcount > 0) && (nic_ifcount <= 8))
    {
        oct_dev_array = (struct net_device**)octnet_get_nic_netdevs();
        for(i = 0; i < nic_ifcount; i++)
        {
            printk("%s: map slave cpu port %d\n",oct_dev_array[i]->name, octnet_get_netdev_gmxport(oct_dev_array[i]));
        }
        ipfwd_learn_mode |= IPFWD_LEARN_MODE_STANDALONE_ENABLE;
    }
#endif

    /* detect coexist fastfwd */
	if((val = cvmx_fau_fetch_and_add64(CVM_FAU_SE_COEXIST_FLAG, 0)) == SE_MAGIC_MUN)
	{
	    ipfwd_learn_mode |= IPFWD_LEARN_MODE_COEXIST_ENABLE;
	}

	if(ipfwd_learn_mode != IPFWD_LEARN_MODE_NONE)
	{
		/* build hash table to save local ip */
		fwd_ifa_hash_init();
        fwd_log_proc_init();

		/* stats proc entry init */
        fwd_stats_proc_initialize();

        cvm_ipfwd_learn_tx_hook = ipfwd_learn;
        printk(KERN_INFO "load ipfwd_learn success! work mode = 0x%x\n", ipfwd_learn_mode);
	}
	else
	{
		cvm_ipfwd_learn_tx_hook = NULL;
		printk(KERN_EMERG "ipfwd_learn: fastfwd was not loaded. magic_num = 0x%llx\n", val);
	}

	return IPFWD_LEARN_RETURN_OK;
}

static void __exit ipfwd_learn_exit(void)
{	
	cvm_ipfwd_learn_tx_hook = NULL;

    if(ipfwd_learn_mode != IPFWD_LEARN_MODE_NONE)
    {
    	fwd_ifa_hash_release();
        fwd_log_proc_release();
		fwd_stats_proc_shutdown();
    }
    printk("unload ipfwd_learn success!\n");   

	return ;
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("<sunjc@autelan.com>");
MODULE_DESCRIPTION("IPfwd learnning module");

module_init(ipfwd_learn_init);
module_exit(ipfwd_learn_exit);

