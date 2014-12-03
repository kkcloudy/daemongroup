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
* had_uid.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in vrrp part of HAD module.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.12 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#if 0
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <stdio.h>
#include <assert.h>
#endif 
#include <net/ethernet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#if 0
#include <linux/rtnetlink.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <linux/if.h>
#include <fcntl.h>
#include <linux/filter.h>
#include <dbus/dbus.h>
#include <dirent.h>


#include <netinet/in.h>
#include <linux/un.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h>

#endif 

#include "had_vrrpd.h"
#include "had_dbus.h"
#include "had_ndisc.h"
#include "had_log.h"
#include "had_uid.h"


#if 0
int npdSock;
int uidSock;
int advtime = 0;
extern hansi_s** g_hansi;
extern int service_enable[MAX_HANSI_PROFILE];
extern pthread_mutex_t PacketMutex;
extern int master_ipaddr_uplink[VRRP_MAX_VRID];
extern int master_ipaddr_downlink[VRRP_MAX_VRID];
extern struct sock_list_t* sock_list;
extern char* global_ht_ifname;
extern int global_ht_ip;
extern int global_ht_state;
extern int global_ht_opposite_ip;
extern int global_multi_link_detect;
extern int time_synchronize_enable;
extern int stateFlg;
PKT_BUF 	bufNode[MAX_HANSI_PROFILE];
#endif

extern pthread_mutex_t StateMutex;
extern int global_state_change_bit;

/*ndisc global vars */
int vrrp_ndisc_send_fd = -1;

/*
 *	Neighbour Advertisement sending routine.
 *  2014.06.19 11:18:00
 */
static int
ndisc_send_na( char* ifname, char *buffer, int buflen )
{
	//struct sockaddr_ll sll;
	struct sockaddr from;
	struct timeval tv;
	int len;

	/*delay timer set*/
	tv.tv_sec = 0;
	tv.tv_usec = 100;
	if( vrrp_ndisc_send_fd < 0 ){
		vrrp_syslog_error("VRRP: Error sending ndisc unsolicited NA to interface %s error, create socket failed!\n",
								ifname);
		return -1;
	}
	/* build the address */
	memset( &from, 0 , sizeof(from));
	strcpy( from.sa_data, ifname );

	/* send the data continuely five */
	/*delay timer*/
    select(0,NULL,NULL,NULL,&tv);
	len = sendto(vrrp_ndisc_send_fd, buffer, buflen, 0, &from, sizeof(from) );
    vrrp_syslog_packet_send("ifname %s send ndisc unsolicited NA return value = %d\n",ifname,len);
	return len;
}

/*
 *	ICMPv6 Checksuming.
 */
static uint32_t
ndisc_icmp6_cksum(const struct ip6hdr *ip6, const struct icmp6hdr *icp, uint32_t len)
{
	size_t i;
	register const uint16_t *sp;
	uint32_t sum;
	union {
		struct {
			struct in6_addr ph_src;
			struct in6_addr ph_dst;
			uint32_t	ph_len;
			uint8_t	ph_zero[3];
			uint8_t	ph_nxt;
		} ph;
		uint16_t pa[20];
	} phu;

	/* pseudo-header */
	memset(&phu, 0, sizeof(phu));
	phu.ph.ph_src = ip6->saddr;
	phu.ph.ph_dst = ip6->daddr;
	phu.ph.ph_len = htonl(len);
	phu.ph.ph_nxt = IPPROTO_ICMPV6;

	sum = 0;
	for (i = 0; i < sizeof(phu.pa) / sizeof(phu.pa[0]); i++)
		sum += phu.pa[i];

	sp = (const uint16_t *)icp;

	for (i = 0; i < (len & ~1); i += 2)
		sum += *sp++;

	if (len & 1)
		sum += htons((*(const uint8_t *)sp) << 8);

	while (sum > 0xffff)
		sum = (sum & 0xffff) + (sum >> 16);
	sum = ~sum & 0xffff;

	return (sum);
}

int had_ndisc_send_vgateway_unsolicited_na
(
	vrrp_rt *vsrv,
	struct in6_addr *ipv6_addr,
	int index,
	int vAddrF
)
{
	struct m_arphdr
	{
		unsigned short int ar_hrd;          /* Format of hardware address.  */
		unsigned short int ar_pro;          /* Format of protocol address.  */
		unsigned char ar_hln;               /* Length of hardware address.  */
		unsigned char ar_pln;               /* Length of protocol address.  */
		unsigned short int ar_op;           /* ARP opcode (command).  */
		/* Ethernet looks like this : This bit is variable sized however...  */
		unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
		unsigned char __ar_sip[4];          /* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
		unsigned char __ar_tip[4];          /* Target IP address.  */
	};
	char	buf[ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN];
	char	buflen	= ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct ip6hdr *ip6h = (struct ip6hdr *) ((char *)eth + ETHER_HDR_LEN);
	struct ndhdr *ndh = (struct ndhdr*) ((char *)ip6h + sizeof(struct ip6hdr));
	struct icmp6hdr *icmp6h = &ndh->icmph;
	struct nd_opt_hdr *nd_opt_h = (struct nd_opt_hdr *) ((char *)ndh + sizeof(struct ndhdr));
	char *nd_opt_lladdr = (char *) ((char *)nd_opt_h + sizeof(struct nd_opt_hdr));
	//struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6] = {0}, zeroMac[ETH_ALEN] = {0};
	int len;

    /* Ethernet header:
	 * Destination ethernet address MUST use specific address Mapping
	 * as specified in rfc2464.7 Address Mapping for
	 */

 	memset(eth->ether_dhost, 0, ETH_ALEN);
	eth->ether_dhost[0] = eth->ether_dhost[1] = 0x33;
	eth->ether_dhost[5] = 1;
	
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->vgateway_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("downlink interface %s hwaddr is zero mac when send unsolicited NA , rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->vgateway_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->vgateway_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->vgateway_vif[index].hwaddr,sizeof(vsrv->vgateway_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char *hwaddr = vAddrF ? vrrp_hwaddr : vsrv->vgateway_vif[index].hwaddr;
	memcpy( eth->ether_shost, hwaddr, ETH_ALEN );
	eth->ether_type = htons(ETHERTYPE_IPV6);

	/* IPv6 Header */
	ip6h->version = 6;
	ip6h->payload_len = htons(sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);
	ip6h->nexthdr = NEXTHDR_ICMP;
	ip6h->hop_limit = NDISC_HOPLIMIT;
	memcpy(&ip6h->saddr,ipv6_addr,sizeof(struct in6_addr));
	ip6h->daddr.s6_addr16[0] = htons(0xff02);
	ip6h->daddr.s6_addr16[7] = htons(1);

	vrrp_syslog_dbg("ifname %s ndisc send unsolicited NA,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip6 address: "NIP6QUAD_FMT"\n",\
					vsrv->vgateway_vif[index].ifname,
					hwaddr[0], hwaddr[1], hwaddr[2],
					hwaddr[3], hwaddr[4], hwaddr[5],	\
					NIP6QUAD(ip6h->saddr.s6_addr));
	/* ICMPv6 Header */
	icmp6h->icmp6_type = NDISC_NEIGHBOUR_ADVERTISEMENT;

	/* Override flag is set to indicate that the advertisement
	 * should override an existing cache entry and update the
	 * cached link-layer address.
	 */
	icmp6h->icmp6_override = 1;
	memcpy(&ndh->target,ipv6_addr,sizeof(struct in6_addr));

	/* NDISC Option header */
	nd_opt_h->nd_opt_type = ND_OPT_TARGET_LL_ADDR;
	nd_opt_h->nd_opt_len = 1;
	memcpy(nd_opt_lladdr, hwaddr, ETH_ALEN );

	/* Compute checksum */
	icmp6h->icmp6_cksum = ndisc_icmp6_cksum(ip6h, icmp6h,
						sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	/* Send the neighbor advertisement message */
	len = ndisc_send_na(vsrv->vgateway_vif[index].ifname, buf, buflen);

	/* Cleanup room for next round */
	memset(buf, 0, ETHER_HDR_LEN + sizeof(struct ip6hdr) +
	       sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	return len;

}

int had_ndisc_send_downlink_unsolicited_na
(
	vrrp_rt *vsrv,
	struct in6_addr *ipv6_addr,
	int index,
	int vAddrF
)
{

	struct m_arphdr
	{
		unsigned short int ar_hrd;          /* Format of hardware address.  */
		unsigned short int ar_pro;          /* Format of protocol address.  */
		unsigned char ar_hln;               /* Length of hardware address.  */
		unsigned char ar_pln;               /* Length of protocol address.  */
		unsigned short int ar_op;           /* ARP opcode (command).  */
		/* Ethernet looks like this : This bit is variable sized however...  */
		unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
		unsigned char __ar_sip[4];          /* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
		unsigned char __ar_tip[4];          /* Target IP address.  */
	};
	char	buf[ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN];
	char	buflen	= ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct ip6hdr *ip6h = (struct ip6hdr *) ((char *)eth + ETHER_HDR_LEN);
	struct ndhdr *ndh = (struct ndhdr*) ((char *)ip6h + sizeof(struct ip6hdr));
	struct icmp6hdr *icmp6h = &ndh->icmph;
	struct nd_opt_hdr *nd_opt_h = (struct nd_opt_hdr *) ((char *)ndh + sizeof(struct ndhdr));
	char *nd_opt_lladdr = (char *) ((char *)nd_opt_h + sizeof(struct nd_opt_hdr));
	//struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6] = {0}, zeroMac[ETH_ALEN] = {0};
	int len;

    /* Ethernet header:
	 * Destination ethernet address MUST use specific address Mapping
	 * as specified in rfc2464.7 Address Mapping for
	 */

 	memset(eth->ether_dhost, 0, ETH_ALEN);
	eth->ether_dhost[0] = eth->ether_dhost[1] = 0x33;
	eth->ether_dhost[5] = 1;
	
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->downlink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("downlink interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->downlink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->downlink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->downlink_vif[index].hwaddr,sizeof(vsrv->downlink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char *hwaddr = vAddrF ? vrrp_hwaddr : vsrv->downlink_vif[index].hwaddr;
	memcpy( eth->ether_shost, hwaddr, ETH_ALEN );
	eth->ether_type = htons(ETHERTYPE_IPV6);

	/* IPv6 Header */
	ip6h->version = 6;
	ip6h->payload_len = htons(sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);
	ip6h->nexthdr = NEXTHDR_ICMP;
	ip6h->hop_limit = NDISC_HOPLIMIT;
	memcpy(&ip6h->saddr,ipv6_addr,sizeof(struct in6_addr));
	ip6h->daddr.s6_addr16[0] = htons(0xff02);
	ip6h->daddr.s6_addr16[7] = htons(1);

	vrrp_syslog_dbg("ifname %s ndisc send unsolicited NA,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip6 address: "NIP6QUAD_FMT"\n",\
					vsrv->downlink_vif[index].ifname,
					hwaddr[0], hwaddr[1], hwaddr[2],
					hwaddr[3], hwaddr[4], hwaddr[5],	\
					NIP6QUAD(ip6h->saddr.s6_addr));

	/* ICMPv6 Header */
	icmp6h->icmp6_type = NDISC_NEIGHBOUR_ADVERTISEMENT;

	/* Override flag is set to indicate that the advertisement
	 * should override an existing cache entry and update the
	 * cached link-layer address.
	 */
	icmp6h->icmp6_override = 1;
	memcpy(&ndh->target,ipv6_addr,sizeof(struct in6_addr));

	/* NDISC Option header */
	nd_opt_h->nd_opt_type = ND_OPT_TARGET_LL_ADDR;
	nd_opt_h->nd_opt_len = 1;
	memcpy(nd_opt_lladdr, hwaddr, ETH_ALEN );

	/* Compute checksum */
	icmp6h->icmp6_cksum = ndisc_icmp6_cksum(ip6h, icmp6h,
						sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	/* Send the neighbor advertisement message */
	len = ndisc_send_na(vsrv->downlink_vif[index].ifname, buf, buflen);

	/* Cleanup room for next round */
	memset(buf, 0, ETHER_HDR_LEN + sizeof(struct ip6hdr) +
	       sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	return len;

}

int had_ndisc_send_uplink_unsolicited_na
(
	vrrp_rt *vsrv,
	struct in6_addr *ipv6_addr,
	int index,
	int vAddrF
)
{

	struct m_arphdr
	{
		unsigned short int ar_hrd;          /* Format of hardware address.  */
		unsigned short int ar_pro;          /* Format of protocol address.  */
		unsigned char ar_hln;               /* Length of hardware address.  */
		unsigned char ar_pln;               /* Length of protocol address.  */
		unsigned short int ar_op;           /* ARP opcode (command).  */
		/* Ethernet looks like this : This bit is variable sized however...  */
		unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
		unsigned char __ar_sip[4];          /* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
		unsigned char __ar_tip[4];          /* Target IP address.  */
	};
	char	buf[ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN];
	char	buflen	= ETHER_HDR_LEN + sizeof(struct ip6hdr) + sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct ip6hdr *ip6h = (struct ip6hdr *) ((char *)eth + ETHER_HDR_LEN);
	struct ndhdr *ndh = (struct ndhdr*) ((char *)ip6h + sizeof(struct ip6hdr));
	struct icmp6hdr *icmp6h = &ndh->icmph;
	struct nd_opt_hdr *nd_opt_h = (struct nd_opt_hdr *) ((char *)ndh + sizeof(struct ndhdr));
	char *nd_opt_lladdr = (char *) ((char *)nd_opt_h + sizeof(struct nd_opt_hdr));
	//struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6] = {0}, zeroMac[ETH_ALEN] = {0};
	int len;

    /* Ethernet header:
	 * Destination ethernet address MUST use specific address Mapping
	 * as specified in rfc2464.7 Address Mapping for
	 */

	if( !ipv6_addr && ipv6_addr_eq_null(ipv6_addr) ){
		return 0;
	}

	//vrrp_syslog_info("********* %s,%d starting ********\n",__func__,__LINE__);
 	memset(eth->ether_dhost, 0, ETH_ALEN);
	eth->ether_dhost[0] = eth->ether_dhost[1] = 0x33;
	eth->ether_dhost[5] = 1;
	
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->uplink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("uplink interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->uplink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->uplink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->uplink_vif[index].hwaddr,sizeof(vsrv->uplink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char *hwaddr = vAddrF ? vrrp_hwaddr : vsrv->uplink_vif[index].hwaddr;
	memcpy( eth->ether_shost, hwaddr, ETH_ALEN );
	eth->ether_type = htons(ETHERTYPE_IPV6);

	/* IPv6 Header */
	ip6h->version = 6;
	ip6h->payload_len = htons(sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);
	ip6h->nexthdr = NEXTHDR_ICMP;
	ip6h->hop_limit = NDISC_HOPLIMIT;
	memcpy(&ip6h->saddr,ipv6_addr,sizeof(struct in6_addr));
	ip6h->daddr.s6_addr16[0] = htons(0xff02);
	ip6h->daddr.s6_addr16[7] = htons(1);

	vrrp_syslog_dbg("ifname %s ndisc send unsolicited NA,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip6 address: "NIP6QUAD_FMT"\n",\
					vsrv->uplink_vif[index].ifname,
					hwaddr[0], hwaddr[1], hwaddr[2],
					hwaddr[3], hwaddr[4], hwaddr[5],	\
					NIP6QUAD(ip6h->saddr.s6_addr));

	/* ICMPv6 Header */
	icmp6h->icmp6_type = NDISC_NEIGHBOUR_ADVERTISEMENT;

	/* Override flag is set to indicate that the advertisement
	 * should override an existing cache entry and update the
	 * cached link-layer address.
	 */
	icmp6h->icmp6_override = 1;
	memcpy(&ndh->target,ipv6_addr,sizeof(struct in6_addr));

	/* NDISC Option header */
	nd_opt_h->nd_opt_type = ND_OPT_TARGET_LL_ADDR;
	nd_opt_h->nd_opt_len = 1;
	memcpy(nd_opt_lladdr, hwaddr, ETH_ALEN );

	/* Compute checksum */
	icmp6h->icmp6_cksum = ndisc_icmp6_cksum(ip6h, icmp6h,
						sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	/* Send the neighbor advertisement message */
	len = ndisc_send_na(vsrv->uplink_vif[index].ifname, buf, buflen);

	/* Cleanup room for next round */
	memset(buf, 0, ETHER_HDR_LEN + sizeof(struct ip6hdr) +
	       sizeof(struct ndhdr) + sizeof(struct nd_opt_hdr) + ETH_ALEN);

	//vrrp_syslog_info("********* %s,%d end ********\n",__func__,__LINE__);
	return len;

}

void had_ndisc_thread_main(void){
    unsigned int profile = 0;
	int i = 0;
	hansi_s* hanode = NULL;
	vrrp_rt* vrrp = NULL;
	static int count = 0;
	static int vCount = 0;
	int ret = 0,radioCount = 0,ethCount = 0;
    int j = 0;
	vrrp_ndisc_send_fd = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_IPV6)); /* 0x300 is magic */

	if( vrrp_ndisc_send_fd < 0 ){
		vrrp_syslog_error("send Neighbor Discovery thread create socket failed!\n");
		return ;
	}
	
	vrrp_syslog_info("send ND thread create global socket success! fd %d\n", vrrp_ndisc_send_fd);

	vrrp_debug_tell_whoami("NDEcho", 0);
#if 0
	while(1){
		/*sleep 5 seconds*/
		if(global_state_change_bit){
			had_sleep_seconds(1);
			count++;
			if(count > 20){
			   count = 0;
               global_state_change_bit =0;
			}
			vCount++;
		}
		else{
            while((j++ < 10)&&!(global_state_change_bit)){
		        had_sleep_seconds(1);                
            }
            if(global_state_change_bit){
                count++;
            }
			vCount++;
            j = 0;
		}
		//vrrp_syslog_info("********* %s,%d vCount = %d ********\n",__func__,__LINE__,vCount);
		pthread_mutex_lock(&StateMutex);	
		for (profile = 0; profile < MAX_HANSI_PROFILE; profile++) {
           hanode = had_get_profile_node(profile);
		   if((NULL != hanode)&&(NULL != ( vrrp = hanode->vlist))&& \
		   	  (VRRP_STATE_MAST == vrrp->state)){
              /*send arp*/
			  if(VRRP_LINK_NO_SETTED != vrrp->uplink_flag){
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vrrp->uplink_vif[i].set_flg) {
						had_ndisc_send_uplink_unsolicited_na(vrrp,&vrrp->uplink_vif[i].ipv6_addr, i, 1);
						had_ndisc_send_uplink_unsolicited_na(vrrp,&vrrp->uplink_local_ipv6_vaddr[i].sin6_addr, i, 1);
						had_ndisc_send_uplink_unsolicited_na(vrrp,&vrrp->uplink_ipv6_vaddr[i].sin6_addr, i, 1);
					}
				}
			  }
			  if(VRRP_LINK_NO_SETTED != vrrp->downlink_flag){
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vrrp->downlink_vif[i].set_flg) {
						had_ndisc_send_downlink_unsolicited_na(vrrp,&vrrp->downlink_vif[i].ipv6_addr, i, 1);
						had_ndisc_send_downlink_unsolicited_na(vrrp,&vrrp->downlink_local_ipv6_vaddr[i].sin6_addr, i, 1);
						had_ndisc_send_downlink_unsolicited_na(vrrp,&vrrp->downlink_ipv6_vaddr[i].sin6_addr, i, 1);
					}
				}
			  }   
			  if(VRRP_LINK_NO_SETTED != vrrp->vgateway_flag){
				  for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				  {
				  	  if(VRRP_LINK_NO_SETTED == vrrp->vgateway_vif[i].set_flg) {
						continue;
				  	  }
					  
				  	  /*check if is bridge interface*/
					  if((!strncmp(vrrp->vgateway_vif[i].ifname,"ebr",3))||
					  		(!strncmp(vrrp->vgateway_vif[i].ifname,"wlan",4))){
	                      /*if br interface, check if radio muster */
						  ret = had_check_br_muster(vrrp->vgateway_vif[i].ifname,&radioCount,&ethCount);
						  if(0 != radioCount){/*no eth interfaces,do not need gratuitous arp*/
	                          ;/*continue;*/
						  }
					  }
					  
					  if(vCount){
						  had_ndisc_send_vgateway_unsolicited_na(vrrp,&vrrp->vgateway_vif[i].ipv6_addr,i,1);
						  had_ndisc_send_vgateway_unsolicited_na(vrrp,&vrrp->vgateway_local_ipv6_vaddr[i].sin6_addr,i,1);
						  had_ndisc_send_vgateway_unsolicited_na(vrrp,&vrrp->vgateway_ipv6_vaddr[i].sin6_addr,i,1);
					  }
				  }
				  vCount = 0;
			  }
		   }
		}

		pthread_mutex_unlock(&StateMutex);	
	}
	close(vrrp_ndisc_send_fd);
#endif
	return;     
}


#ifdef __cplusplus
}
#endif

