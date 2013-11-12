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
* dhcp_snp_listener.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping listener APIs.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sysdef/returncode.h>
#include <pthread.h>
#include <linux/filter.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>

#include "dhcp_snp_log.h"
#include "dhcp_snp_pkt.h"
#include "dhcp_snp_com.h"
#include "dhcp_snp_listener.h"

#define bpf_insn sock_filter /* Linux: dare to be gratuitously different. */

#define       ETHERTYPE_IP            0x0800  /* IP protocol */

#define DHCPSNP_RD_DEBUG	1

#define EBT_ADD_EBR_DEFAULT_FILTER \
	"ebtables -A FORWARD -i %s -j DROP"
#define EBT_DEL_EBR_DEFAULT_FILTER \
	"ebtables -D FORWARD -i %s -j DROP"
#define EBT_ADD_DHCPS_FILTER  \
	"ebtables -I FORWARD -i %s -p ip --ip-proto 17 \
	--ip-dport 68 --ip-sport 67 -j ACCEPT"
#define EBT_DEL_DHCPS_FILTER  \
	"ebtables -D FORWARD -i %s -p ip --ip-proto 17 \
	--ip-dport 68 --ip-sport 67 -j ACCEPT"
#define EBT_ADD_DHCPC_FILTER  \
	"ebtables -I FORWARD -i %s -p ip --ip-proto 17 \
	--ip-dport 67 --ip-sport 68 -j ACCEPT"
#define EBT_DEL_DHCPC_FILTER  \
	"ebtables -D FORWARD -i %s -p ip --ip-proto 17 \
	--ip-dport 67 --ip-sport 68 -j ACCEPT"
#define EBT_ADD_HOST_SMAC_FILTER \
	"ebtables -I FORWARD -s %s -p ip --ip-src %s -j ACCEPT"
#define EBT_DEL_HOST_SMAC_FILTER \
	"ebtables -D FORWARD -s %s -p ip --ip-src %s -j ACCEPT"
#define EBT_ADD_HOST_DMAC_FILTER \
	"ebtables -I FORWARD -d %s -p ip --ip-dst %s -j ACCEPT"
#define EBT_DEL_HOST_DMAC_FILTER \
	"ebtables -D FORWARD -d %s -p ip --ip-dst %s -j ACCEPT"
#define EBT_ADD_HOST_ARP_FILTER \
		"ebtables -I FORWARD -p arp -s %s -j ACCEPT"
#define EBT_DEL_HOST_ARP_FILTER \
		"ebtables -D FORWARD -p arp -s %s -j ACCEPT"
	
#define EBT_RULE_LEN 	256

#define EBR_PATH_LEN	128
#define DHCPSNP_EBR_PATH "/sys/class/net/%s/brif/"

/* anti arp spoofing info list */
struct anti_arp_spoof_info *anti_arp_spoof_list = NULL;

/* dhcp snooping sockets */
struct dhcp_snp_sock *dhcpsnp_sock_list = NULL;
/* mutex for dhcp snooping socket list */
//pthread_mutex_t mutexDhcpsnpSockList = PTHREAD_MUTEX_INITIALIZER;

int dhcp_snp_maxfd = -1;
/* socket fd set */
fd_set g_sock_fds;
/* mutex for dhcp snooping socket fd set */
pthread_mutex_t mutexDhcpsnpFdset = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t mutexDhcpsnptbl;
extern unsigned int dhcp_snp_arp_proxy_enable;
extern struct dhcp_snp_static_table *dhcp_snp_static_table_head;

/* dhcp packet buffer */
static char sock_pack_buf[DHCP_SNP_PACKET_SIZE] = {0};

/* dhcp server port */
unsigned short dhcp_server_port = 67;

#if 0
/*get from tcpdump -dd arp or 'ip and udp port 67'  */
struct sock_filter arp_and_dhcp_filter[] = {
	{ 0x28, 0, 0, 0x0000000c },
	{ 0x15, 11, 0, 0x00000806 },
	{ 0x15, 0, 10, 0x00000800 },
	{ 0x30, 0, 0, 0x00000017 },
	{ 0x15, 0, 8, 0x00000011 },
	{ 0x28, 0, 0, 0x00000014 },
	{ 0x45, 6, 0, 0x00001fff },
	{ 0xb1, 0, 0, 0x0000000e },
	{ 0x48, 0, 0, 0x0000000e },
	{ 0x15, 2, 0, 0x00000043 },
	{ 0x48, 0, 0, 0x00000010 },
	{ 0x15, 0, 1, 0x00000043 },
	{ 0x6, 0, 0, -1 },
	{ 0x6, 0, 0, 0 },
};
#endif
struct sock_filter arp_and_dhcp_filter[] = {
	{ 0x28, 0, 0, 0x0000000c },
	{ 0x15, 20, 0, 0x00000806 },
	{ 0x15, 0, 7, 0x000086dd },
	{ 0x30, 0, 0, 0x00000014 },
	{ 0x15, 0, 18, 0x00000011 },
	{ 0x28, 0, 0, 0x00000036 },
	{ 0x15, 15, 0, 0x00000043 },
	{ 0x15, 14, 0, 0x00000223 },
	{ 0x28, 0, 0, 0x00000038 },
	{ 0x15, 12, 11, 0x00000043 },
	{ 0x15, 0, 12, 0x00000800 },
	{ 0x30, 0, 0, 0x00000017 },
	{ 0x15, 0, 10, 0x00000011 },
	{ 0x28, 0, 0, 0x00000014 },
	{ 0x45, 8, 0, 0x00001fff },
	{ 0xb1, 0, 0, 0x0000000e },
	{ 0x48, 0, 0, 0x0000000e },
	{ 0x15, 4, 0, 0x00000043 },
	{ 0x15, 3, 0, 0x00000223 },
	{ 0x48, 0, 0, 0x00000010 },
	{ 0x15, 1, 0, 0x00000043 },
	{ 0x15, 0, 1, 0x00000223 },
	{ 0x6, 0, 0, -1 },
	{ 0x6, 0, 0, 0 },
};
int arp_and_dhcp_filter_len = sizeof(arp_and_dhcp_filter)/sizeof(struct sock_filter);

struct bpf_insn dhcp_bpf_filter [] = {
	/* Make sure this is an IP packet... */
	BPF_STMT (BPF_LD + BPF_H + BPF_ABS, 12), /* {0x00 + 0x08 + 0x20, 0, 0, 12}*/
	BPF_JUMP (BPF_JMP + BPF_JEQ + BPF_K, ETHERTYPE_IP, 0, 8),/* {0x05 + 0x10 + 0x00, 0, 8, 0x0800} */

	/* Make sure it's a UDP packet... */
	BPF_STMT (BPF_LD + BPF_B + BPF_ABS, 23),/* {0x00 + 0x10 + 0x20, 0, 0, 23}*/
	BPF_JUMP (BPF_JMP + BPF_JEQ + BPF_K, IPPROTO_UDP, 0, 6),/* {0x05 + 0x10 + 0x00, 0, 6, 0x11} */

	/* Make sure this isn't a fragment... */
	BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 20),/* {0x00 + 0x08 + 0x20, 0, 0, 20}*/
	BPF_JUMP(BPF_JMP + BPF_JSET + BPF_K, 0x1fff, 4, 0),/* {0x05 + 0x40 + 0x00, 4, 0, 0x1fff} */

	/* Get the IP header length... */
	BPF_STMT (BPF_LDX + BPF_B + BPF_MSH, 14),/* {0x01 + 0x10 + 0xA0, 0, 0, 14}*/

	/* Make sure it's to the right port... */
	BPF_STMT (BPF_LD + BPF_H + BPF_IND, 16),/* {0x00 + 0x08 + 0x40, 0, 0, 16}*/
	BPF_JUMP (BPF_JMP + BPF_JEQ + BPF_K, 67, 0, 1),   /* patch *//* {0x05 + 0x10 + 0x00, 0, 1, 0x43} */

	/* If we passed all the tests, ask for the whole packet. */
	BPF_STMT(BPF_RET+BPF_K, (u_int)-1),/* {0x06 + 0x00, 0, 0, -1}*/

	/* Otherwise, drop it. */
	BPF_STMT(BPF_RET+BPF_K, 0),/* {0x06 + 0x00, 0, 0, 0}*/
};
int dhcp_bpf_filter_len = sizeof(dhcp_bpf_filter)/sizeof(struct bpf_insn);

/**********************************************************************************
 * dhcp_snp_listener_init
 *	Initialize DHCP snooping packet socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_init(void)
{
	struct dhcp_snp_sock *sock = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK;

	syslog_ax_dhcp_snp_dbg("initialize dhcp snooping socket!\n");
	sock = (struct dhcp_snp_sock*)malloc(sizeof(struct dhcp_snp_sock));
	if(!sock) {
		syslog_ax_dhcp_snp_err("init socket error(out of memory)!\n");\
		return DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(sock, 0, sizeof(struct dhcp_snp_sock));
	dhcpsnp_sock_list = sock;
	INIT_LIST_HEAD(&(dhcpsnp_sock_list->sock_list));
	
	return ret;
}
/**********************************************************************************
 * dhcp_snp_listener_handle
 *	Handle DHCP snooping packet listener add/delete/update operation
 *
 *	INPUT:
 *		listener - packet listener
 *		op_type - operation type( 0 - add, 1 - delete, 2 - update)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_handle
(
	struct dhcp_snp_listener *listener,
	enum dhcpsnp_listener_op op_type
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK, result = 0;
	struct dhcp_snp_listener  *node = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	struct ifreq ifr;
	int found = 0;
	
	if(!listener) {
		syslog_ax_dhcp_snp_dbg("dhcp snp %s listener null pointer error!\n", DHCPSNP_LSTNER_OPTYPE_DESC(op_type));
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	if(!dhcpsnp_sock_list) {
		ret = dhcp_snp_listener_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("%s listener but global socket out of memory!\n",  \
									DHCPSNP_LSTNER_OPTYPE_DESC(op_type));
			return ret;
		}
	}
	if(dhcpsnp_sock_list){
		if((DHCPSNP_LSTNER_ADD_E != op_type)&&(!dhcpsnp_sock_list->count)) {
			log_info("%s listener on %s, global socket list %d\n",  \
									DHCPSNP_LSTNER_OPTYPE_DESC(op_type),listener->ifname,  \
									dhcpsnp_sock_list->count);
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
	}
	pthread_mutex_lock(&mutexDhcpsnpFdset);
	curPtr = &(dhcpsnp_sock_list->sock_list);
	switch(op_type) {
		case DHCPSNP_LSTNER_MAX:
		default:
			break;
		case DHCPSNP_LSTNER_QUERY_E:
			__list_for_each(pos, curPtr) {
				node = list_entry(pos, struct dhcp_snp_listener, list);
				if(node) {
					if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
						found = 1;
						syslog_ax_dhcp_snp_dbg("query listener(%s) found(fd %d)!\n",	\
												listener->ifname, node->fd);
						break;
					}
					else {
						#if DHCPSNP_RD_DEBUG
						syslog_ax_dhcp_snp_dbg("query listener(%s) check node %s!\n", listener->ifname, node->ifname);
						#endif
						/* skip */
					}
				}
				else {
					#if DHCPSNP_RD_DEBUG
					syslog_ax_dhcp_snp_dbg("query listener(%s) list node null!\n", listener->ifname);
					#endif
				}
			}
			if(!found) {
				log_info("query listener(%s) but not found\n", listener->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			else {
				listener->fd = node->fd;
			}
			break;
		case DHCPSNP_LSTNER_ADD_E:
			if(curPtr){
				__list_for_each(pos, curPtr) {
					node = list_entry(pos, struct dhcp_snp_listener, list);
					if(node) {
						if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
							syslog_ax_dhcp_snp_err("add listener(%s, fd %d) duplicated(fd %d)!\n",  \
													listener->ifname, listener->fd, node->fd);
							pthread_mutex_unlock(&mutexDhcpsnpFdset);
							return DHCP_SNP_RETURN_CODE_ALREADY_SET;
						}
						else {
							;/* skip */
						}
					}
				}
				list_add(&(listener->list), curPtr);
			}
			dhcpsnp_sock_list->count++;

			/* rescan socket list */
			dhcp_snp_socket_init();

			#if 0 /* move to dhcp_snp_listener_handle_intf_ebtables */
			/* add ebtables rule */
			if(!strncmp(listener->ifname, "ebr", 3) ||
				!strncmp(listener->ifname, "wlan", 4)) {
				dhcp_snp_listener_handle_ebr_ebtables(listener->ifname, DHCPSNP_EBT_ADD_E);
			}
			#endif
			break;
		case DHCPSNP_LSTNER_DEL_E:
			if(!curPtr) {
				log_warn("delete listener(%s, fd %d) but global socket null\n",  \
										listener->ifname, listener->fd);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			__list_for_each(pos, curPtr) {
				node = list_entry(pos, struct dhcp_snp_listener, list);
				if(node) {
					if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
						found = 1;
						log_info("delete listener(%s) found(fd %d)!\n",	\
												listener->ifname, node->fd);
						break;
					}
					else {
						;/* skip */
					}
				}
			}
			if(!found) {
				log_info("delete listener(%s) but not found\n", listener->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}

			if(node->no_arp) {
				log_info("interface %s set anti-arp-spoof enable, disable first.\n", node->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_EN_ANTI_ARP;
			}
			if(node->add_router){
				log_info("interface %s set add-route enable, disable first.\n", node->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_EN_ADD_ROUTE;
			}
			
			list_del(&(node->list));
			if(dhcpsnp_sock_list->count) {
				dhcpsnp_sock_list->count--;
			}
			
			/* rescan socket list */
			dhcp_snp_socket_init();

			#if 0 /* move to dhcp_snp_listener_handle_intf_ebtables */
			/* delete ebtables rule */
			if(!strncmp(listener->ifname, "ebr", 3) ||
				!strncmp(listener->ifname, "wlan", 4)) {
				dhcp_snp_listener_handle_ebr_ebtables(listener->ifname, DHCPSNP_EBT_DEL_E);
			}
			#endif
			
			/* reset flags  & close fd */
			if(node->fd > 0) {
				#if 1
				memset(&ifr, 0, sizeof(struct ifreq));
				strncpy(ifr.ifr_name, node->ifname, IF_NAMESIZE);
				ifr.ifr_flags = (node->ifflags &(~IFF_PROMISC));
				result = ioctl(node->fd, SIOCSIFFLAGS, &ifr);
				if(result < 0) {
					syslog_ax_dhcp_snp_err("reset socket fd %d add flags erro %s\n", strerror(errno));
					ret = DHCP_SNP_RETURN_CODE_ERROR;
				}
				#endif
				#if 0
				memset(&mr, 0, sizeof(mr));
				mr.mr_ifindex = if_nametoindex(node->ifname);
				mr.mr_type	  = PACKET_MR_PROMISC;
				if (setsockopt(node->fd, SOL_PACKET, PACKET_DROP_MEMBERSHIP, &mr, sizeof(mr)) == -1)
				{
					syslog_ax_dhcp_snp_err("set sockopt promisc error %s!\n",strerror(errno));
					ret = DHCP_SNP_RETURN_CODE_ERROR;
				}
				#endif
				listener->fd = node->fd;
				close(node->fd);
			}
			free(node);
			node = NULL;
			break;
		case DHCPSNP_LSTNER_UPDATE_E:
			__list_for_each(pos, curPtr) {
				node = list_entry(pos, struct dhcp_snp_listener, list);
				if(node) {
					if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
						found = 1;
						syslog_ax_dhcp_snp_dbg("update listener(%s,fd %d) found(fd %d)!\n",	\
												listener->ifname,listener->fd, node->fd);
						break;
					}
					else {
						;/* skip */
					}
				}
			}
			if(!found) {
				syslog_ax_dhcp_snp_err("update listener(%s) but not found\n", listener->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			if(node->fd > 0) {
				close(node->fd);
			}
			node->fd = listener->fd;
			break;
		case DHCPSNP_LSTNER_SET_NO_ARP:
			__list_for_each(pos, curPtr) {
				node = list_entry(pos, struct dhcp_snp_listener, list);
				if(node) {
					if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
						found = 1;
						syslog_ax_dhcp_snp_dbg("update listener(%s,fd %d) found(fd %d)!\n",	\
												listener->ifname,listener->fd, node->fd);
						break;
					}
					else {
						;/* skip */
					}
				}
			}
			if(!found) {
				syslog_ax_dhcp_snp_err("update listener(%s) but not found\n", listener->ifname);				
				pthread_mutex_unlock(&mutexDhcpsnpFdset);
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			node->no_arp = listener->no_arp;
			break;
		case DHCPSNP_LSTNER_SET_ADD_ROUTER:
			__list_for_each(pos, curPtr) {
			node = list_entry(pos, struct dhcp_snp_listener, list);
			if(node) {
				if(!strncmp(node->ifname, listener->ifname, IF_NAMESIZE)) {
					found = 1;
					syslog_ax_dhcp_snp_dbg("update listener(%s,fd %d) found(fd %d)!\n",	\
											listener->ifname,listener->fd, node->fd);
					break;
				}
				else {
					;/* skip */
				}
			}
		}
		if(!found) {
			syslog_ax_dhcp_snp_err("update listener(%s) but not found\n", listener->ifname);				
			pthread_mutex_unlock(&mutexDhcpsnpFdset);
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
		node->add_router= listener->add_router;
		break;
	}

	pthread_mutex_unlock(&mutexDhcpsnpFdset);
	return ret;
}

/**********************************************************************************
 * dhcp_snp_listener_init
 *	Initialize DHCP snooping packet socket
 *
 *	INPUT:
 *		ifname - interface to start packet listener
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_open
(
	char *ifname
)
{	
	int sockfd = 0, ret = 0;
//	int bufsize = DHCP_SNP_PACKET_SIZE;
	unsigned int ifindex = ~0UI;
	struct sock_fprog filter;
	struct sockaddr_ll sll;
	struct ifreq ifr;
	struct dhcp_snp_listener *listener = NULL;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("start packet listener on null interface error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	listener = (struct dhcp_snp_listener *)malloc(sizeof(*listener));
	if(!listener) {
		syslog_ax_dhcp_snp_err("start packet listener error(out of memory)!\n");
		return DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(listener, 0, sizeof(*listener));
	strncpy(listener->ifname, ifname, IF_NAMESIZE);
	
	/* open the socket */
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if( sockfd < 0 ){
		int	err = errno;
		syslog_ax_dhcp_snp_err("open raw socket error(%d),try to run it as root!\n", err);
		free(listener);
		listener = NULL;
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	/* set flags */
	#if 1
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if(ret < 0) {
		syslog_ax_dhcp_snp_err("get socket fd %d flags error %s\n", sockfd, strerror(errno));
		free(listener);
		listener = NULL;
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	listener->ifflags = ifr.ifr_flags;
	ifr.ifr_flags |= IFF_PROMISC;
	ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
	if(ret < 0) {
		syslog_ax_dhcp_snp_err("set socket fd %d add flags erro %s\n", sockfd, strerror(errno));
		free(listener);
		listener = NULL;
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	#endif

	/* set filter to catch only arp or dhcpv4 packet */
	filter.len = arp_and_dhcp_filter_len;
	filter.filter = arp_and_dhcp_filter;
	
	/* update udp listen port to configured port */
	//arp_and_dhcp_filter[16].k = arp_and_dhcp_filter[18].k = ntohs(547);

	/* apply filter */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
	if(ret) {
		syslog_ax_dhcp_snp_err("set socket fd %d dhcp packet filter error %s!\n", sockfd, strerror(errno));
		free(listener);
		listener = NULL;
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	ifindex = if_nametoindex(ifname);
	
	/* set promiscous mode */
	#if 0
	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = ifindex;
	mr.mr_type	  = PACKET_MR_PROMISC;
	if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
	{
		syslog_ax_dhcp_snp_err("set sockopt promisc error %s!\n",strerror(errno));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	#endif
	
	/* bind socket to unique interfaces */
	memset( &sll, 0, sizeof(sll) );
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifindex;
	sll.sll_protocol = htons(ETH_P_ALL);
	#if 0
	sll.sll_pkttype = PACKET_OTHERHOST;
	sll.sll_halen = ETH_ALEN;
	#endif
	if( bind(sockfd, (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
	   int err = errno;
	   syslog_ax_dhcp_snp_err("bind interface index %d to socket %d failed(%s)\n",
						sll.sll_ifindex,sockfd, strerror(err));
	   free(listener);
	   listener = NULL;
	   return DHCP_SNP_RETURN_CODE_ERROR;
	}	

	listener->fd = sockfd;

	dhcp_snp_listener_handle(listener, DHCPSNP_LSTNER_ADD_E);
	
	return DHCP_SNP_RETURN_CODE_OK;	
}

/**********************************************************************************
 * dhcp_snp_listener_unware
 *	Release DHCP snooping packet socket and stop packet listener
 *
 *	INPUT:
 *		ifname - interface to release packet listener
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_close
(
	char *ifname
)
{	
	struct dhcp_snp_listener listener;
	int ret = DHCP_SNP_RETURN_CODE_OK;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("stop packet listener on null interface error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&listener, 0, sizeof(listener));
	strncpy(listener.ifname, ifname, IF_NAMESIZE);

	ret = dhcp_snp_listener_handle(&listener, DHCPSNP_LSTNER_DEL_E);
	if(DHCP_SNP_RETURN_CODE_OK == ret) {
		syslog_ax_dhcp_snp_dbg("close socket %d work on interface %s\n",listener.fd, ifname);
	}
	
	return ret;	
}

/********************************************************************************************
 * 	dhcp_snp_listener_query
 *
 *	DESCRIPTION:
 *             This function check out whether dhcp snooping is enabled on interface or not.
 *
 *	INPUT:
 *             ifname - interface name
 *
 *	OUTPUT:
 *               NONE
 *
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned int	dhcp_snp_listener_query
(       
	unsigned char  *ifname
)
{	
	struct dhcp_snp_listener listener;
	int ret = DHCP_SNP_RETURN_CODE_OK;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("query packet listener on null interface error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&listener, 0, sizeof(listener));
	strncpy(listener.ifname, (char*)ifname, IF_NAMESIZE);

	ret = dhcp_snp_listener_handle(&listener, DHCPSNP_LSTNER_QUERY_E);
	if(DHCP_SNP_RETURN_CODE_OK == ret) {
		syslog_ax_dhcp_snp_dbg("query found socket %d work on interface %s\n",listener.fd, ifname);
	}
	else if(DHCP_SNP_RETURN_CODE_NOT_FOUND == ret ) {
		syslog_ax_dhcp_snp_dbg("query not found socket work on interface %s\n",ifname);
	}
	
	return ret; 
}

/********************************************************************************************
 * 	dhcp_snp_listener_handle_add_router
 *
 *	DESCRIPTION:
 *             This function check out whether dhcp snooping is enabled on interface or not.
 *
 *	INPUT:
 *             ifname - interface name
 *
 *	OUTPUT:
 *               NONE
 *
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned int dhcp_snp_listener_handle_add_router
(   
	unsigned int isenable,
	unsigned char  *ifname
)
{	
	struct dhcp_snp_listener listener;
	int ret = DHCP_SNP_RETURN_CODE_OK;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("query packet listener on null interface error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&listener, 0, sizeof(listener));
	strncpy(listener.ifname, (char*)ifname, IF_NAMESIZE);
	listener.add_router= isenable;

	ret = dhcp_snp_listener_handle(&listener, DHCPSNP_LSTNER_SET_ADD_ROUTER);
	if(DHCP_SNP_RETURN_CODE_OK == ret) {
		syslog_ax_dhcp_snp_dbg("query found socket %d work on interface %s add_router is %d\n",listener.fd, ifname, isenable);
		
	}
	
	return ret; 
}

/********************************************************************************************
 * 	dhcp_snp_listener_query
 *
 *	DESCRIPTION:
 *             This function check out whether dhcp snooping is enabled on interface or not.
 *
 *	INPUT:
 *             ifname - interface name
 *
 *	OUTPUT:
 *               NONE
 *
 *	RETURNS:
 *              GT_TRUE - indicate the packet is IPv4 packet
 *              GT_FALSE - indicate the packet is not IPv4 packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned int dhcp_snp_listener_handle_arp
(   
	unsigned int isenable,
	unsigned char  *ifname
)
{	
	struct dhcp_snp_listener listener;
	int ret = DHCP_SNP_RETURN_CODE_OK;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("query packet listener on null interface error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&listener, 0, sizeof(listener));
	strncpy(listener.ifname, (char*)ifname, IF_NAMESIZE);
	listener.no_arp = isenable;

	ret = dhcp_snp_listener_handle(&listener, DHCPSNP_LSTNER_SET_NO_ARP);
	if(DHCP_SNP_RETURN_CODE_OK == ret) {
		syslog_ax_dhcp_snp_dbg("query found socket %d work on interface %s no_arp is %d\n",listener.fd, ifname, isenable);
		
	}
	
	return ret; 
}

/**********************************************************************************
 * dhcp_snp_listener_save_cfg
 *	Handle DHCP snooping packet listener show running-config
 *
 *	INPUT:
 *		listener - packet listener
 *		op_type - operation type( 0 - add, 1 - delete, 2 - update)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_PARAM_NULL - if show string buffer null
 **********************************************************************************/
int dhcp_snp_listener_save_cfg
(
	unsigned char *buffer,
	unsigned int length
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK;
	struct dhcp_snp_listener *node = NULL;
	struct anti_arp_spoof_node *iNode = NULL;
	struct list_head  *curPtr = NULL, *pos = NULL;
	struct dhcp_snp_static_table *table = NULL;
	char ifname[IF_NAMESIZE];
	unsigned char *cur = NULL;
	unsigned int curLen = 0;
	
	if(!buffer) {
		syslog_ax_dhcp_snp_err("dhcp snp listener save cfg null pointer error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	cur = buffer;
	if(dhcpsnp_sock_list && dhcpsnp_sock_list->count) {
		curPtr = &(dhcpsnp_sock_list->sock_list);
		
		__list_for_each(pos, curPtr) {
			node = list_entry(pos, struct dhcp_snp_listener, list);
			if(node && strncmp(node->ifname, "wlan", 4) && strncmp(node->ifname, "ebr", 3)) {
				syslog_ax_dhcp_snp_dbg("dhcp snp listener on interface %s\n", node->ifname);
				curLen += sprintf((char*)cur, "config dhcp-snooping %s enable\n", node->ifname); 
				cur = buffer + curLen;
				if (node->no_arp) {
					curLen += sprintf((char*)cur, "config dhcp-snooping anti-arp-spoof %s enable\n", node->ifname); 
					cur = buffer + curLen;
				}
				if (node->add_router) {
					curLen += sprintf((char*)cur, "config dhcp-snooping add-router %s enable\n", node->ifname); 
					cur = buffer + curLen;
				}
			}
		}
	}
	
	if(anti_arp_spoof_list && anti_arp_spoof_list->count) {
		curPtr = &(anti_arp_spoof_list->intf_list);
		
		__list_for_each(pos, curPtr) {
			iNode = list_entry(pos, struct anti_arp_spoof_node, list);
			if(iNode) {
				syslog_ax_dhcp_snp_dbg("anti arp spoof enable on interface %s\n", iNode->ifname);
				curLen += sprintf((char*)cur, "config anti-arp-spoof %s enable\n", iNode->ifname); 
				cur = buffer + curLen;
			}
		}
	}
	if (dhcp_snp_static_table_head) {
		table = dhcp_snp_static_table_head;
		while (table) {
			memset(ifname, 0, sizeof(ifname));
			syslog_ax_dhcp_snp_dbg("add static bind table ip %#x interface %d\n", table->ipaddr, table->ifindex);
			curLen += sprintf((char*)cur, "add dhcp-snooping bind-table ip %u.%u.%u.%u mac %02x:%02x:%02x:%02x:%02x:%02x iface %s\n", 
							(table->ipaddr>>24)&0xFF, (table->ipaddr>>16)&0xFF, (table->ipaddr>>8)&0xFF, table->ipaddr&0xFF,
							table->chaddr[0], table->chaddr[1], table->chaddr[2], table->chaddr[3], table->chaddr[4], table->chaddr[5],
							if_indextoname(table->ifindex, ifname));
			cur = buffer + curLen;
			table = table->next;
		}
	}
	return ret;
}


/**********************************************************************************
 * dhcp_snp_listener_save_hansi_cfg
 *	Handle DHCP snooping packet listener show running-config
 *
 *	INPUT:
 *		listener - packet listener
 *		op_type - operation type( 0 - add, 1 - delete, 2 - update)
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_PARAM_NULL - if show string buffer null
 **********************************************************************************/
int dhcp_snp_listener_save_hansi_cfg
(
	unsigned char *buffer,
	unsigned int length,
	unsigned int slot,
	unsigned int vrrp,
	unsigned int local_flag
)
{
	int ret = DHCP_SNP_RETURN_CODE_OK;
	struct dhcp_snp_listener *node = NULL;
	struct anti_arp_spoof_node *iNode = NULL;
	struct list_head  *curPtr = NULL, *pos = NULL;
	struct dhcp_snp_static_table *table = NULL;
	char ifname[IF_NAMESIZE];
	unsigned char *cur = NULL;
	unsigned int curLen = 0;
	int value1 = 0, value2 = 0, value3 = 0;
	
	if(!buffer) {
		syslog_ax_dhcp_snp_err("dhcp snp listener save cfg null pointer error!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	cur = buffer;
	
	if(dhcpsnp_sock_list && dhcpsnp_sock_list->count) {
		curPtr = &(dhcpsnp_sock_list->sock_list);

		if (vrrp && local_flag) {
			curLen += sprintf(cur, "config local-hansi %d-%d\n", slot, vrrp);
			cur = buffer + curLen;
		} else if (vrrp) {
			curLen += sprintf(cur, "config hansi-profile %d-%d\n", slot, vrrp);
			cur = buffer + curLen;
		}
		
		__list_for_each(pos, curPtr) {
			node = list_entry(pos, struct dhcp_snp_listener, list);
			/* save config wlan or ebr */
			if(node 
				&& ((!strncmp(node->ifname, "wlan", 4)) || (!strncmp(node->ifname, "ebr", 3)))) {

				memset(ifname, 0, sizeof(ifname));
				memcpy(ifname, node->ifname, strlen(node->ifname));
				syslog_ax_dhcp_snp_dbg("show running hansi cfg: interface %s\n", ifname);

				/* local hansi wlan */
				if (!strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && local_flag) { /* local hansi */
						curLen += sprintf((char*)cur, "config dhcp-snooping wlan%d enable\n", value3); 
						cur = buffer + curLen;
						if (node->no_arp) {
							curLen += sprintf((char*)cur, "config dhcp-snooping anti-arp-spoof wlan%d enable\n", value3); 
							cur = buffer + curLen;
						}	
						if (node->add_router) {
							curLen += sprintf((char*)cur, "config dhcp-snooping add-router wlan%d enable\n", value3); 
							cur = buffer + curLen;
						}
					} 
				} 

				/* hansi wlan */
				else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
					ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
						curLen += sprintf((char*)cur, "config dhcp-snooping wlan%d enable\n", value3); 
						cur = buffer + curLen;
						if (node->no_arp) {
							curLen += sprintf((char*)cur, "config dhcp-snooping anti-arp-spoof wlan%d enable\n", value3); 
							cur = buffer + curLen;
						}
						if (node->add_router) {
							curLen += sprintf((char*)cur, "config dhcp-snooping add-router wlan%d enable\n", value3); 
							cur = buffer + curLen;
						}			
					}
				} 

				/* local hansi ebr */
				if (0 == strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && local_flag) { /* local hansi */

						curLen += sprintf((char*)cur, "config dhcp-snooping ebr%d enable\n", value3); 
						cur = buffer + curLen;
						if (node->no_arp) {
							curLen += sprintf((char*)cur, "config dhcp-snooping anti-arp-spoof ebr%d enable\n", value3); 
							cur = buffer + curLen;
						}	
						if (node->add_router) {
							curLen += sprintf((char*)cur, "config dhcp-snooping add-router ebr%d enable\n", value3); 
							cur = buffer + curLen;
						}
					} 
				} 

				/* hansi ebr */
				else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
					ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2) && (!local_flag)) {
						curLen += sprintf((char*)cur, "config dhcp-snooping ebr%d enable\n", value3); 
						cur = buffer + curLen;
						if (node->no_arp) {
							curLen += sprintf((char*)cur, "config dhcp-snooping anti-arp-spoof ebr%d enable\n", value3); 
							cur = buffer + curLen;
						}	
						if (node->add_router) {
							curLen += sprintf((char*)cur, "config dhcp-snooping add-router ebr%d enable\n", value3); 
							cur = buffer + curLen;
						}			
					}
				}
			}
		}
		if (vrrp) {
			curLen += sprintf(cur, " exit\n");
			cur = buffer + curLen;
		}
		syslog_ax_dhcp_snp_dbg("show running hansi cfg: %s\n", buffer);
		
	}
#if 0	
/* TODO */
	if(anti_arp_spoof_list && anti_arp_spoof_list->count) {
		curPtr = &(anti_arp_spoof_list->intf_list);
		
		__list_for_each(pos, curPtr) {
			iNode = list_entry(pos, struct anti_arp_spoof_node, list);
			if(iNode) {
				syslog_ax_dhcp_snp_dbg("anti arp spoof enable on interface %s\n", iNode->ifname);
				curLen += sprintf((char*)cur, "config anti-arp-spoof %s enable\n", iNode->ifname); 
				cur = buffer + curLen;
			}
		}
	}
	if (dhcp_snp_static_table_head) {
		table = dhcp_snp_static_table_head;
		while (table) {
			memset(ifname, 0, sizeof(ifname));
			syslog_ax_dhcp_snp_dbg("add static bind table ip %#x interface %d\n", table->ipaddr, table->ifindex);
			curLen += sprintf((char*)cur, "add dhcp-snooping bind-table ip %u.%u.%u.%u mac %02x:%02x:%02x:%02x:%02x:%02x iface %s\n", 
							(table->ipaddr>>24)&0xFF, (table->ipaddr>>16)&0xFF, (table->ipaddr>>8)&0xFF, table->ipaddr&0xFF,
							table->chaddr[0], table->chaddr[1], table->chaddr[2], table->chaddr[3], table->chaddr[4], table->chaddr[5],
							if_indextoname(table->ifindex, ifname));
			cur = buffer + curLen;
			table = table->next;
		}
	}
#endif	
	return DHCP_SNP_RETURN_CODE_OK;
}

/********************************************************************************************
 *	dhcp_snp_listener_type_check
 *
 *	DESCRIPTION:
 *			   This function check out whether the packet is dhcp or not.
 *
 *	INPUT:
 *			   packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *				 NONE
 *	RETURNS:
 *				DHCP_SNP_TRUE - indicate the packet is dhcp packet
 *				DHCP_SNP_FALSE - indicate the packet is not dhcp packet
 *
 *	COMMENTS:
 *			   NONE.
 *
 **********************************************************************************************/
unsigned char dhcp_snp_listener_dhcpv6_packet_check
(		
	unsigned char  *packetBuff
)
{
	ether_header_t	*layer2 = NULL;
	ipv6_header_t 	*layer3 = NULL;
	udp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ipv6_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (udp_header_t*)((unsigned char *)layer3 + sizeof(ipv6_header_t));

	if ((ETHER_IPv6 == layer2->etherType) && 
			(IPVER6 == layer3->version) &&
				(IPPROTOCOL_UDP == layer3->next_hdr)) {
		if (((DHCPv6_SERVER_PORT == layer4->dest) && (DHCPv6_CLIENT_PORT == layer4->source)) /* DHCPv6 request */
			|| ((DHCPv6_SERVER_PORT == layer4->source) && (DHCPv6_CLIENT_PORT == layer4->dest))){ /* DHCPv6 reply<ack/nak> */

			return DHCP_SNP_TRUE;
		}
	}
	return DHCP_SNP_FALSE;
	
}

/********************************************************************************************
 * 	dhcp_snp_listener_type_check
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is dhcp or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              DHCP_SNP_TRUE - indicate the packet is dhcp packet
 *              DHCP_SNP_FALSE - indicate the packet is not dhcp packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned char	dhcp_snp_listener_type_check
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	ip_header_t 	*layer3 = NULL;
	udp_header_t	*layer4 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (ip_header_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));
	layer4 = (udp_header_t*)((unsigned int *)layer3 + layer3->hdrLength);
	

	if ((ETHER_IP == layer2->etherType) && 
			(IPVER4 == layer3->version) &&
				(IPPROTOCOL_UDP == layer3->ipProtocol)) {
		if (((DHCP_SERVER_PORT == layer4->dest) && (DHCP_CLIENT_PORT == layer4->source)) /* DHCP request */
			|| ((DHCP_SERVER_PORT == layer4->source) && (DHCP_CLIENT_PORT == layer4->dest))){ /* DHCP reply<ack/nak> */

			return DHCP_SNP_TRUE;
		}
	}
	return DHCP_SNP_FALSE;
	
}

/********************************************************************************************
 * 	dhcp_snp_listener_gratuitous_arp_check
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is gratuitous arp or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              DHCP_SNP_TRUE - indicate the packet is gratuitous packet
 *              DHCP_SNP_FALSE - indicate the packet is not gratuitous packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned char	dhcp_snp_listener_gratuitous_arp_check
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	arp_packet_t 	*layer3 = NULL;

	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (arp_packet_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));	

	/* check if dmac bcast and smac legal */
	if(0xFF != (layer2->dmac[0] & layer2->dmac[1] & layer2->dmac[2]
		 & layer2->dmac[3] & layer2->dmac[4] & layer2->dmac[5])) { /* non-bcast */
		#if !DHCPSNP_RD_DEBUG
		syslog_ax_dhcp_snp_err("non-bcast pacekt!\n");
		#endif
		return DHCP_SNP_FALSE;
	}
	else if(layer2->smac[0] & 0x1) {/* mcast */
		#if !DHCPSNP_RD_DEBUG
		syslog_ax_dhcp_snp_err("smac mcast pacekt!\n");
		#endif
		return DHCP_SNP_FALSE;
	}

	/* arp request with the same send ip and target ip */
	if ((ETHER_ARP == layer2->etherType) && 
			(ARP_OPCODE_REQUEST == layer3->opCode)) {
			#if !DHCPSNP_RD_DEBUG
			syslog_ax_dhcp_snp_err("arp request pacekt!\n");
			#endif
			if(!memcmp(layer3->sip, layer3->dip, 4)) {
				#if !DHCPSNP_RD_DEBUG
				syslog_ax_dhcp_snp_err("gratuitous arp pacekt!\n");
				#endif
				return DHCP_SNP_TRUE;
			}
	}
	#if !DHCPSNP_RD_DEBUG
	syslog_ax_dhcp_snp_err("non arp request pacekt!\n");
	#endif
	return DHCP_SNP_FALSE;
}

unsigned char	dhcp_snp_listener_arp_request_check
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;
	arp_packet_t 	*layer3 = NULL;
	layer2 = (ether_header_t*)(packetBuff);
	layer3 = (arp_packet_t *)((unsigned char*)packetBuff + sizeof(ether_header_t));	
	if(0xFF != (layer2->dmac[0] & layer2->dmac[1] & layer2->dmac[2]
		 & layer2->dmac[3] & layer2->dmac[4] & layer2->dmac[5])) { /* non-bcast */
		#if !DHCPSNP_RD_DEBUG
		syslog_ax_dhcp_snp_err("non-bcast pacekt!\n");
		#endif
		return DHCP_SNP_FALSE;
	}	
	if ((ETHER_ARP == layer2->etherType) && 
		(ARP_OPCODE_REQUEST == layer3->opCode)) {		
		return DHCP_SNP_TRUE;
	}
	#if !DHCPSNP_RD_DEBUG
	syslog_ax_dhcp_snp_err("non arp request pacekt!\n");
	#endif
	return DHCP_SNP_FALSE;
}
/********************************************************************************************
 * 	dhcp_snp_dump_packet_detail
 *
 *	DESCRIPTION:
 * 		This function dump all bits of packet buffer 
 *
 *	INPUT:
 *		buffer - points to the packet's header
 *		bufflen - buffer length
 *		
 *	OUTPUT:
 * 		NULL
 *
 *	RETURNS:
 *		NULL
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
void dhcp_snp_listener_dump_detail
(
	unsigned char *buffer,
	unsigned long buffLen
)
{
	unsigned int i;
	unsigned char lineBuffer[64] = {0}, *bufPtr = NULL;
	unsigned int curLen = 0;

	if(!buffer)
		return;
	
	#if 1
	syslog_ax_dhcp_snp_pkt_rx(".......................RX.......................%d\n",buffLen);
	bufPtr = lineBuffer;
	curLen = 0;
	for(i = 0;i < buffLen ; i++)
	{
		curLen += sprintf((char*)bufPtr,"%02x ",buffer[i]);
		bufPtr = lineBuffer + curLen;
		
		if(0==(i+1)%16) {
			syslog_ax_dhcp_snp_pkt_rx("%s\n",lineBuffer);
			memset(lineBuffer,0,sizeof(lineBuffer));
			curLen = 0;
			bufPtr = lineBuffer;
		}
	}
	
	if((buffLen%16)!=0)
	{
		syslog_ax_dhcp_snp_pkt_rx("%s\n",lineBuffer);
	}
	
	syslog_ax_dhcp_snp_pkt_rx(".......................RX.......................\n");
	#endif
}

/**********************************************************************************
 * dhcp_snp_socket_init
 *	Scan socket list to update FD_SET
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_init
(
	void
)
{	
	struct dhcp_snp_listener *node = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK;

	if(!dhcpsnp_sock_list) {
		ret = dhcp_snp_listener_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("out of memory when init socket fdset\n");
			return ret;
		}
	}
	
	//pthread_mutex_lock(&mutexDhcpsnpFdset);
	FD_ZERO(&g_sock_fds);
	
	curPtr = &(dhcpsnp_sock_list->sock_list);
	if(dhcpsnp_sock_list){
		if(!dhcpsnp_sock_list->count) {
			log_info("no listener found when init socket fdset\n");
			//pthread_mutex_unlock(&mutexDhcpsnpFdset);
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
	}
	__list_for_each(pos, curPtr) {
		node = list_entry(pos, struct dhcp_snp_listener, list);
		if(node) {
			#if DHCPSNP_RD_DEBUG
			syslog_ax_dhcp_snp_dbg("add socket fd %d on %s to fdset!\n", node->fd, node->ifname);
			#endif
			if(-1 != node->fd) {
				FD_SET(node->fd, &g_sock_fds);
				if(dhcp_snp_maxfd < node->fd) {
					dhcp_snp_maxfd = node->fd;
				}
			}
		}		
	}
	
	//pthread_mutex_unlock(&mutexDhcpsnpFdset);
	return ret;	
}

/**********************************************************************************
 * dhcp_snp_socket_global_enable
 *	Scan socket list to global enable socket
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_global_enable
(
	void
)
{	
	struct dhcp_snp_listener *node = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK, sockfd = -1, result = 0;
	struct sock_fprog filter;
	struct sockaddr_ll sll;
/*	struct ifreq ifr;	*/
	unsigned int ifindex = ~0UI;

	if(!dhcpsnp_sock_list) {
		ret = dhcp_snp_listener_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("out of memory when global enable socket fdset\n");
			return ret;
		}
	}
	
	pthread_mutex_lock(&mutexDhcpsnpFdset);
	FD_ZERO(&g_sock_fds);
	
	curPtr = &(dhcpsnp_sock_list->sock_list);
	if(!dhcpsnp_sock_list->count) {
		log_info("no listener found when global enable socket fdset\n");
		pthread_mutex_unlock(&mutexDhcpsnpFdset);
		return DHCP_SNP_RETURN_CODE_NOT_FOUND;
	}
	
	__list_for_each(pos, curPtr) {
		node = list_entry(pos, struct dhcp_snp_listener, list);
		if(node) {
			if(-1 == node->fd) {
				/* open the socket */
				sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
				if( sockfd < 0 ){
					syslog_ax_dhcp_snp_err("open raw socket error(%s),try to run it as root!\n", strerror(errno));
					pthread_mutex_unlock(&mutexDhcpsnpFdset);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				
				/* set flags */
				#if 0
				memset(&ifr, 0, sizeof(struct ifreq));
				strncpy(ifr.ifr_name, node->ifname, IF_NAMESIZE);
				result = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
				if(result < 0) {
					syslog_ax_dhcp_snp_err("get socket fd %d flags error %s\n", sockfd, strerror(errno));
					pthread_mutex_unlock(&mutexDhcpsnpFdset);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				node->ifflags = ifr.ifr_flags;
				ifr.ifr_flags |= IFF_PROMISC;
				result = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
				if(result < 0) {
					syslog_ax_dhcp_snp_err("set socket fd %d add flags error %s\n", sockfd, strerror(errno));
					pthread_mutex_unlock(&mutexDhcpsnpFdset);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				#endif

				/* set filter to catch only arp or dhcpv4 packet */
				filter.len = arp_and_dhcp_filter_len;
				filter.filter = arp_and_dhcp_filter;
				
				/* update udp listen port to configured port */
			//	arp_and_dhcp_filter[9].k = arp_and_dhcp_filter[11].k = ntohs(dhcp_server_port);
				
				/* apply filter */
				result = setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
				if(result) {
					syslog_ax_dhcp_snp_err("set socket fd %d dhcp packet filter error %s!\n", sockfd, strerror(errno));
					pthread_mutex_unlock(&mutexDhcpsnpFdset);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				
				ifindex = if_nametoindex(node->ifname);
				
				/* bind socket to unique interfaces */
				memset( &sll, 0, sizeof(sll) );
				sll.sll_family = AF_PACKET;
				sll.sll_ifindex = ifindex;
				sll.sll_protocol = htons(ETH_P_ALL);
				if( bind(sockfd, (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
				   int err = errno;
				   syslog_ax_dhcp_snp_err("bind interface index %d to socket %d failed(%s)\n",
									sll.sll_ifindex,sockfd, strerror(err));
				   pthread_mutex_unlock(&mutexDhcpsnpFdset);
				   return DHCP_SNP_RETURN_CODE_ERROR;
				}	
				node->fd = sockfd;
				
				FD_SET(node->fd, &g_sock_fds);
				if(dhcp_snp_maxfd < node->fd) {
					dhcp_snp_maxfd = node->fd;
				}
			}
			#if DHCPSNP_RD_DEBUG
			syslog_ax_dhcp_snp_dbg("global enable add socket fd %d on %s to fdset!\n", node->fd, node->ifname);
			#endif
		}		
	}
	
	pthread_mutex_unlock(&mutexDhcpsnpFdset);
	return ret;	
}

/**********************************************************************************
 * dhcp_snp_socket_init
 *	Scan socket list to update FD_SET
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_socket_global_disable
(
	void
)
{	
	struct dhcp_snp_listener *node = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK;

	if(!dhcpsnp_sock_list) {
		return ret;
	}
	
	pthread_mutex_lock(&mutexDhcpsnpFdset);
	FD_ZERO(&g_sock_fds);
	
	curPtr = &(dhcpsnp_sock_list->sock_list);
	if(!dhcpsnp_sock_list->count) {
		syslog_ax_dhcp_snp_err("no listener found when global disable socket fdset\n");
		pthread_mutex_unlock(&mutexDhcpsnpFdset);
		return DHCP_SNP_RETURN_CODE_OK;
	}
	
	__list_for_each(pos, curPtr) {
		node = list_entry(pos, struct dhcp_snp_listener, list);
		if(node) {
			if(-1 != node->fd) {
				#if DHCPSNP_RD_DEBUG
				syslog_ax_dhcp_snp_dbg("global disable socket  close fd %d on %s!\n", node->fd, node->ifname);
				#endif
				close(node->fd);
				node->fd = -1;
			}
		}		
	}
	
	pthread_mutex_unlock(&mutexDhcpsnpFdset);
	return ret;	
}

/**********************************************************************************
 * dhcp_snp_listener_read_sock
 *	Scan socket list to verify if packet arrived
 *
 *	INPUT:
 *		fds - FD set
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int dhcp_snp_listener_read_sock
(
	fd_set fds
)
{	
/*	struct sockaddr_ll from;	*/
	struct dhcp_snp_listener *node = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK, len = 0;
	unsigned char *pktPool[1] = {NULL};
	unsigned long  pLenPool[1] = {0};
	unsigned int ifindex = ~0UI;
	unsigned short vid = 0xFFFF;

	#if !DHCPSNP_RD_DEBUG
	syslog_ax_dhcp_snp_dbg("read socket to check packet!\n");
	#endif
	if(!dhcpsnp_sock_list) {
		ret = dhcp_snp_listener_init();
		if(DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("out of memory when read socket fdset\n");
			return ret;
		}
	}
	
	//pthread_mutex_lock(&mutexDhcpsnpSockList);
	curPtr = &(dhcpsnp_sock_list->sock_list);
	if(!dhcpsnp_sock_list->count) {
		syslog_ax_dhcp_snp_err("no listener found when read socket fdset\n");
		//pthread_mutex_unlock(&mutexDhcpsnpSockList);
		return DHCP_SNP_RETURN_CODE_NOT_FOUND;
	}
	
	__list_for_each(pos, curPtr) {
		node = list_entry(pos, struct dhcp_snp_listener, list);
		if(node) {
			if(FD_ISSET(node->fd,&fds)) {
				#if !DHCPSNP_RD_DEBUG
				syslog_ax_dhcp_snp_dbg("packet probed from socket fd %d on %s!\n", node->fd, node->ifname);
				#endif
				ifindex = if_nametoindex(node->ifname);

				len = read(node->fd, sock_pack_buf, DHCP_SNP_PACKET_SIZE);
				#if 0
				len = recvfrom(node->fd, sock_pack_buf, DHCP_SNP_PACKET_SIZE, 0, NULL, 0);
				#endif
				if(len <= 0) {
					memset(sock_pack_buf, 0, DHCP_SNP_PACKET_SIZE);
					continue;
				}
				#if !DHCPSNP_RD_DEBUG
				dhcp_snp_listener_dump_detail((unsigned char*)sock_pack_buf,len);
				#endif

				/* first check if gratuitous arp or not */
				if(DHCP_SNP_TRUE == dhcp_snp_listener_gratuitous_arp_check(sock_pack_buf)) {
					pthread_mutex_lock(&mutexDhcpsnptbl);
					ret = dhcp_snp_gratuitous_arp_process(vid, ifindex,sock_pack_buf, node);
					pthread_mutex_unlock(&mutexDhcpsnptbl);	
					if(DHCP_SNP_RETURN_CODE_OK != ret) {
						syslog_ax_dhcp_snp_err("gratuitous arp packet process error %d!\n", ret);
					}
					#if !DHCPSNP_RD_DEBUG
					dhcp_snp_listener_dump_detail((unsigned char*)sock_pack_buf,len);
					#endif
					memset(sock_pack_buf, 0, DHCP_SNP_PACKET_SIZE);
					continue;
				}
				else if(DHCP_SNP_TRUE == dhcp_snp_listener_arp_request_check((unsigned char*)sock_pack_buf)){
					pthread_mutex_lock(&mutexDhcpsnptbl);
					ret = dhcp_snp_arp_request_process(vid, ifindex,sock_pack_buf, node->fd);
					pthread_mutex_unlock(&mutexDhcpsnptbl);	
					
					continue;
				}
				/* check if dhcpv6 packet or not */
				else if(DHCP_SNP_TRUE == dhcp_snp_listener_dhcpv6_packet_check((unsigned char*)sock_pack_buf)){
					pktPool[0] = (unsigned char*)sock_pack_buf;
					pLenPool[0] = len;
					pthread_mutex_lock(&mutexDhcpsnptbl);
					ret = dhcp_snp_dhcpv6_packet_process(1 ,pktPool, pLenPool, ifindex, 0,vid ,node);
					pthread_mutex_unlock(&mutexDhcpsnptbl);
					if(DHCP_SNP_RETURN_CODE_OK != ret){
						syslog_ax_dhcp_snp_err("dhcpv6 packed process error %d!\n", ret);
					}
					memset(sock_pack_buf, 0, 	DHCP_SNP_PACKET_SIZE);
					continue;
				}
				/* check if dhcp packet or not*/
				else if(DHCP_SNP_FALSE == dhcp_snp_listener_type_check((unsigned char*)sock_pack_buf)) {
					#if !DHCPSNP_RD_DEBUG
					syslog_ax_dhcp_snp_dbg("ignore non-dhcp packet, drop it!\n");
					#endif
					memset(sock_pack_buf, 0, DHCP_SNP_PACKET_SIZE);
					continue;
				}
				
				/* DHCP snooping process */
				pktPool[0] = (unsigned char*)sock_pack_buf;
				pLenPool[0] = len;
				pthread_mutex_lock(&mutexDhcpsnptbl);
				ret = dhcp_snp_packet_rx_process(1, pktPool, pLenPool, ifindex, 0, vid, node);
				pthread_mutex_unlock(&mutexDhcpsnptbl);
				
				if(DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("dhcp packet process error %d!\n", ret);
				}
				memset(sock_pack_buf, 0, DHCP_SNP_PACKET_SIZE);
			}
			else {
				#if !DHCPSNP_RD_DEBUG
				syslog_ax_dhcp_snp_dbg("read socket fd %d on %s, no packet!\n", node->fd, node->ifname);
				#endif
			}
		}	
	}
	
	//pthread_mutex_unlock(&mutexDhcpsnpSockList);
	return ret;	
}

/**********************************************************************************
 * dhcp_snp_listener_thread_main
 *	Main routine for packet listener handle thread
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
void * dhcp_snp_listener_thread_main
(
	void *arg
)
{
	struct timeval	timeout;
	int ret = DHCP_SNP_RETURN_CODE_OK;
/*	int counter = 0;	*/
	fd_set readfds;
	
	syslog_ax_dhcp_snp_dbg("dhcp snooping packet handler thread start!\n");
	
	/* tell my thread pid*/
	dhcp_snp_tell_whoami("dhcpsnp lisdftener", 0);
	/*set task */
	dhcp_snp_setaffinity(~0UL);

	dhcp_snp_socket_init();
	
	timeout.tv_sec	= 1;
	timeout.tv_usec = 0;

	WAIT_FOREVER_BEGIN
	pthread_mutex_lock(&mutexDhcpsnpFdset);
	FD_ZERO(&readfds);
	memcpy(&readfds, &g_sock_fds, sizeof(fd_set));
	/* if dhcp snooping not globally enabled, give up! */
	if(DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == dhcp_snp_check_global_status()) {
		pthread_mutex_unlock(&mutexDhcpsnpFdset);
		sleep(1);
		continue;
	}
	
	if( select(dhcp_snp_maxfd + 1, &readfds, NULL, NULL, &timeout ) > 0 ) {
		#if !DHCPSNP_RD_DEBUG
		syslog_ax_dhcp_snp_dbg("%d times select ok!\n",counter++);
		#endif
		ret = dhcp_snp_listener_read_sock(readfds); 
		pthread_mutex_unlock(&mutexDhcpsnpFdset);
		if(DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
			sleep(1);
		}
	}
	else {
		pthread_mutex_unlock(&mutexDhcpsnpFdset);
		#if !DHCPSNP_RD_DEBUG
		if(0 == counter % 30) 
			syslog_ax_dhcp_snp_dbg("select %d times!\n",counter++);
		#endif
		sleep(1);
	}
	WAIT_FOREVER_END

	return NULL;
}

/**********************************************************************************
 * anti_arp_spoof_init
 *	Initialize anti ARP spoof interface list
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
int anti_arp_spoof_init(void)
{
	struct anti_arp_spoof_info *aas = NULL;
	int ret = DHCP_SNP_RETURN_CODE_OK;

	syslog_ax_dhcp_snp_dbg("initialize anti arp spoof info!\n");
	aas = (struct anti_arp_spoof_info*)malloc(sizeof(struct anti_arp_spoof_info));
	if(!aas) {
		syslog_ax_dhcp_snp_err("anti arp spoof info error(out of memory)!\n");\
		return DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(aas, 0, sizeof(struct anti_arp_spoof_info));
	anti_arp_spoof_list = aas;
	INIT_LIST_HEAD(&(anti_arp_spoof_list->intf_list));
	
	return ret;
}

/**********************************************************************************
 * dhcp_snp_handle_intf_ebtables
 *		set DHCP_Snooping enable/disable interface ebtables
 *
 *	INPUT:
 *		ifname - interface name
 *		isEnable - enable or disable flag
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_handle_intf_ebtables
(
	char *ifname,
	unsigned char isEnable
)
{
	enum dhcpsnp_ebt_op op = DHCPSNP_EBT_MAX;
	struct anti_arp_spoof_node  *node = NULL, *iNode = NULL;
	struct list_head *pos = NULL, *curPtr = NULL;
	unsigned char found = 0;
	
	if(!ifname) {
		syslog_ax_dhcp_snp_err("set interface ebtables parameter null!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	/* check if already configured */
	
	curPtr = &(anti_arp_spoof_list->intf_list);
	__list_for_each(pos, curPtr) {
		node = list_entry(pos, struct anti_arp_spoof_node, list);
		if(node) {
			if(!strncmp(node->ifname, ifname, IF_NAMESIZE)) {
				found = 1;
				syslog_ax_dhcp_snp_dbg("scan anti arp spoof list found intf (%s)!\n", ifname);
				break;
			}
			else {
			#if DHCPSNP_RD_DEBUG
				syslog_ax_dhcp_snp_dbg("scan anti arp spoof list check node %s!\n", node->ifname);
			#endif
				/* skip */
			}
		}
		else {
		#if DHCPSNP_RD_DEBUG
			syslog_ax_dhcp_snp_dbg("scan anti arp spoof list for intf(%s) but list null!\n", ifname);
		#endif
		}
	}
	
	if (DHCP_SNP_ENABLE == isEnable) {		
		op = DHCPSNP_EBT_ADD_E;
		if(found) {
			syslog_ax_dhcp_snp_err("add intf %s to anti arp spoof list but already exist\n", ifname);				
			return DHCP_SNP_RETURN_CODE_FOUND;
		}
		iNode = (struct anti_arp_spoof_node *)malloc(sizeof(struct anti_arp_spoof_node));
		if(!iNode) {
			syslog_ax_dhcp_snp_err("add intf %s to anti arp spoof list alloc memory failed\n", ifname);
			return DHCP_SNP_RETURN_CODE_ALLOC_MEM_NULL;
		}
		memset(iNode, 0, sizeof(struct anti_arp_spoof_node));
		memcpy(iNode->ifname, ifname, IF_NAMESIZE);
		
		list_add(&(iNode->list), curPtr);
		anti_arp_spoof_list->count++;
	}
	else {
		op = DHCPSNP_EBT_DEL_E;
		if(!found) {
			syslog_ax_dhcp_snp_err("del intf %s from anti arp spoof list but not found\n", ifname);				
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
		
		list_del(&(node->list));
		if(anti_arp_spoof_list->count) {
			anti_arp_spoof_list->count--;
		}
		free(node);
		node = NULL;
	}
	
	if(strncmp(ifname, "ebr", 3) &&
		strncmp(ifname, "wlan", 4)) {
		syslog_ax_dhcp_snp_dbg("%s interface %s ebtables\n", \
						(DHCP_SNP_ENABLE == isEnable) ? "add":"del", ifname);		
		dhcp_snp_listener_handle_intf_ebtables(ifname, op);
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_listener_handle_ebr_ebtables
 *	Handle ebr interface to add ebtables rules as follows:
 *	  1 - permit all dhcp packet from all interfaces under ebr
 *		(rules: ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 67 --ip-sport 68 -j ACCEPT
 				ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 68 --ip-sport 67 -j ACCEPT)
 *	  2 - deny all other packets
 *		(rules: ebtabls -A -i ethx-y -j DROP)
 *
 *	INPUT:
 *		ebrname - bridge interface name
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_ebr_ebtables
(
   	char *ebrname,
	enum dhcpsnp_ebt_op op
)
{
   DIR* dp = NULL;
   struct dirent* dir = NULL;
   char path[EBR_PATH_LEN] = {0};
   char filter[EBT_RULE_LEN] = {0};

   if(NULL == ebrname){
   	  syslog_ax_dhcp_snp_err("handle ebr interface ebtables null name!\n");
      return DHCP_SNP_RETURN_CODE_PARAM_NULL;
   }

   sprintf(path,DHCPSNP_EBR_PATH,ebrname);
   syslog_ax_dhcp_snp_dbg("handle ebr %s in path %s\n",ebrname,path);
   if(NULL ==(dp = opendir(path))){
      syslog_ax_dhcp_snp_err("can not open %s when handle ebr ebtables\n",path);
	  return DHCP_SNP_RETURN_CODE_ERROR;
   }
   else {
   	  /*loop for member of ebr interface to get status*/
      while(NULL != (dir = readdir(dp))){
	  	 if(!strcmp(".",dir->d_name) || !strcmp("..",dir->d_name)){
            syslog_ax_dhcp_snp_dbg("ignore dir . or .. in %s!\n",path);
			continue;
		 }
		 if(DHCPSNP_EBT_ADD_E == op) {
			 /* append default rule */
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_ADD_EBR_DEFAULT_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(+):%s\n", filter);
			 system(filter);
			 /* insert dhcp packet rule */
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_ADD_DHCPS_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(+):%s\n", filter);
			 system(filter);
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_ADD_DHCPC_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(+):%s\n", filter);
			 system(filter);
		 }
		 else if(DHCPSNP_EBT_DEL_E == op) {
			 /* append default rule */
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_DEL_EBR_DEFAULT_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
			 system(filter);
			 /* insert dhcp packet rule */
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_DEL_DHCPS_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
			 system(filter);
			 memset(filter, 0, EBT_RULE_LEN);
			 sprintf(filter, EBT_DEL_DHCPC_FILTER, dir->d_name);
			 syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
			 system(filter);
		 }
	  }
	  closedir(dp);
   }

   return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_listener_handle_intf_ebtables
 *	Handle interface to add ebtables rules as follows:
 *	  1 - permit all dhcp packet from all interfaces under ebr
 *		(rules: ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 67 --ip-sport 68 -j ACCEPT
 				ebtabls -I FORWARD -i ethx-y -p ip --ip-proto 17 \
 						--ip-dport 68 --ip-sport 67 -j ACCEPT)
 *	  2 - deny all other packets
 *		(rules: ebtabls -A -i ethx-y -j DROP)
 *
 *	INPUT:
 *		ifname - interface(except ebr) name
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_intf_ebtables
(
   	char *ifname,
	enum dhcpsnp_ebt_op op
)
{
	char filter[EBT_RULE_LEN] = {0};

	if(NULL == ifname){
		syslog_ax_dhcp_snp_err("handle interface ebtables null name!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

   	  /*loop for member of ebr interface to get status*/
	if(!strncmp(ifname, "ebr",3) || !strncmp(ifname, "wlan",4)){
		syslog_ax_dhcp_snp_dbg("handle ebtables ignore interface %s!\n",ifname);
		return DHCP_SNP_RETURN_CODE_OK;
	}
	
	if(DHCPSNP_EBT_ADD_E == op) {
		/* append default rule */
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_ADD_EBR_DEFAULT_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtri(+):%s\n", filter);
		system(filter);
		/* insert dhcp packet rule */
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_ADD_DHCPS_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtri(+):%s\n", filter);
		system(filter);
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_ADD_DHCPC_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtri(+):%s\n", filter);
		system(filter);
	}
	else if(DHCPSNP_EBT_DEL_E == op) {
		/* append default rule */
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_DEL_EBR_DEFAULT_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
		system(filter);
		/* insert dhcp packet rule */
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_DEL_DHCPS_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
		system(filter);
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_DEL_DHCPC_FILTER, ifname);
		syslog_ax_dhcp_snp_dbg("ebtr(-):%s\n", filter);
		system(filter);
	}

   return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_listener_handle_host_ebtables
 *	Handle host ebtables rules as follows:
 *	  1 - permit all packet from all interfaces under ebr
 *		(rules: ebtables -I/-D FORWARD -s xx:xx:xx:xx:xx:xx -p ip --ip-src x.x.x.x -j ACCEPT
 *			   ebtables -I/-D FORWARD -d xx:xx:xx:xx:xx:xx -p ip --ip-dst x.x.x.x -j ACCEPT)
 *
 *	INPUT:
 *		mac - host mac address
 *		ipaddr - ip address
 *		op - add or delete
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
int dhcp_snp_listener_handle_host_ebtables
(
   unsigned char *mac,
   unsigned int ipaddr,
   enum dhcpsnp_ebt_op op
)
{
#if 0
	char filter[EBT_RULE_LEN] = {0};
	unsigned char macstr[18] = {0}, ipstr[16] = {0};

	if(NULL == mac){
		syslog_ax_dhcp_snp_err("handle host ebtables null mac!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	sprintf(macstr, "%02x:%02x:%02x:%02x:%02x:%02x", \
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	sprintf(ipstr, "%d.%d.%d.%d",  (ipaddr>>24) & 0xFF,  \
			(ipaddr>>16) & 0xFF, (ipaddr>>8) & 0xFF, ipaddr&0xFF);
	
	if(DHCPSNP_EBT_ADD_E == op) {
		/* insert host rule */
		sprintf(filter, EBT_ADD_HOST_ARP_FILTER, macstr, ipstr);
		syslog_ax_dhcp_snp_dbg("ebtr host(+):%s\n", filter);
		system(filter);
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_ADD_HOST_SMAC_FILTER, macstr, ipstr);
		syslog_ax_dhcp_snp_dbg("ebtr host(+):%s\n", filter);
		system(filter);
	}
	else if(DHCPSNP_EBT_DEL_E == op) {
		/* insert host rule */
		sprintf(filter, EBT_DEL_HOST_ARP_FILTER, macstr, ipstr);
		syslog_ax_dhcp_snp_dbg("ebtr host(-):%s\n", filter);
		system(filter);
		memset(filter, 0, EBT_RULE_LEN);
		sprintf(filter, EBT_DEL_HOST_SMAC_FILTER, macstr, ipstr);
		syslog_ax_dhcp_snp_dbg("ebtr host(-):%s\n", filter);
		system(filter);
	}
#endif
   return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * check_dhcp_snp_listener
 *	Scan socket list return count of listener node
 *
 *	INPUT:
 *		
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		ret		- count of listener node
 **********************************************************************************/
unsigned int check_dhcp_snp_listener(void)
{
	unsigned int ret = 0;

	pthread_mutex_lock(&mutexDhcpsnpFdset);
	if(dhcpsnp_sock_list && dhcpsnp_sock_list->count) {
		syslog_ax_dhcp_snp_err("%d listener found on sock list.\n", dhcpsnp_sock_list->count);
		ret =  dhcpsnp_sock_list->count;
	}
	pthread_mutex_unlock(&mutexDhcpsnpFdset);

	return ret;
}

#ifdef __cplusplus
}
#endif
